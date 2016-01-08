#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "fcgi_stdio.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "fdfs_client.h"
#include "logger.h"

char filestore[255];

char* decode(char* URL)
{
	char* pDecode    =URL;
	char* pURL       =URL;

	while('\0'!=(*pDecode=*pURL))
	{
		++pURL;
		if('%'==*pDecode)
		{
			*pDecode=(*pURL>='A'?(*pURL-'A'+10):(*pURL-'0'));
			++pURL;
			*pDecode=(*pDecode<<=4)|(*pURL>='A'?(*pURL-'A'+10):(*pURL-'0'));
			++pURL;
		}
		++pDecode;
	}

	return URL;
}

void eatspace(char *str)
{
	char *p = str;
	while(*p)
	{
		if(*p == '+')
			*p = ' ';
		p++;
	}
}


static void usage(char *argv[])
{
	printf("Usage: %s <config_file> <local_filename> " \
			"[storage_ip:port] [store_path_index]\n", argv[0]);
}

int uploadfile(int argc, char *argv[])
{
	char *conf_filename;
	char *local_filename;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo *pTrackerServer;
	int result;
	int store_path_index;
	ConnectionInfo storageServer;
	char file_id[128];

	if (argc < 3)
	{
		usage(argv);
		return 1;
	}

	log_init();
	g_log_context.log_level = LOG_ERR;
	ignore_signal_pipe();

	conf_filename = argv[1];
	if ((result=fdfs_client_init(conf_filename)) != 0)
	{
		return result;
	}

	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		return errno != 0 ? errno : ECONNREFUSED;
	}

	local_filename = argv[2];
	*group_name = '\0';
	if (argc >= 4)
	{
		const char *pPort;
		const char *pIpAndPort;

		pIpAndPort = argv[3];
		pPort = strchr(pIpAndPort, ':');
		if (pPort == NULL)
		{
			fdfs_client_destroy();
			fprintf(stderr, "invalid storage ip address and " \
					"port: %s\n", pIpAndPort);
			usage(argv);
			return 1;
		}

		storageServer.sock = -1;
		snprintf(storageServer.ip_addr, sizeof(storageServer.ip_addr), \
				"%.*s", (int)(pPort - pIpAndPort), pIpAndPort);
		storageServer.port = atoi(pPort + 1);
		if (argc >= 5)
		{
			store_path_index = atoi(argv[4]);
		}
		else
		{
			store_path_index = -1;
		}
	}
	else if ((result=tracker_query_storage_store(pTrackerServer, \
					&storageServer, group_name, &store_path_index)) != 0)
	{
		fdfs_client_destroy();
		fprintf(stderr, "tracker_query_storage fail, " \
				"error no: %d, error info: %s\n", \
				result, STRERROR(result));
		return result;
	}

	result = storage_upload_by_filename1(pTrackerServer, \
			&storageServer, store_path_index, \
			local_filename, NULL, \
			NULL, 0, group_name, file_id);
	if (result == 0)
	{
		//printf("%s\n", file_id);
		strcpy(filestore,file_id);
	}
	else
	{
		fprintf(stderr, "upload file fail, " \
				"error no: %d, error info: %s\n", \
				result, STRERROR(result));
	}

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();

	return result;
}

int writeDatabase(char *username,char *filename,int filelength)
{
	int ret = 0;
	time_t t;
	char date[64] = {0};
	char sql[1024] = {0};
	
	//初始化数据库
	MYSQL *mydb = NULL;//数据库句柄
	mydb = mysql_init(NULL);
	if(!mydb)
	{
		ret = mysql_errno(mydb);
		return ret;
	}
	//链接数据库
	mydb = mysql_real_connect(mydb,"127.0.0.1","root","123456","YunDB",0,NULL,0);
	if(!mydb)
	{
		ret = mysql_errno(mydb);
		return ret;
	}

	t = time(NULL);
	strftime(date,sizeof(date),"%Y/%m/%d %X",localtime(&t));

	sprintf(sql,"insert into filenames(username,filename,filelength,filestore,date) values('%s','%s',%d,'%s','%s')",username,filename,filelength,filestore,date);
	mysql_query(mydb, "set names utf8");
	ret = mysql_query(mydb,sql);
	if(ret != 0)
	{
		return ret;
	}
	mysql_close(mydb);

	return ret;	
}


int main(int argc,char **argv)
{
	while(FCGI_Accept() >= 0)
	{
		printf("Content-type: text/html\r\n\r\n");
		int datalen = atoi(getenv("CONTENT_LENGTH"));
		char *buf = (char *)calloc(1,datalen);
		fread(buf,1,datalen,stdin);
		
		if(datalen > 0)
		{
			printf("buf = %s\n",buf);
#if 1
			//去url编码
			eatspace(decode(buf));	

			//username=yzmin&filename=stdio.h&filecontent=12345678
			char *name1 = strtok(buf,"&");
			char *name2 = strtok(NULL,"&");
			char *name3 = strtok(NULL,"&");

			strtok(name1,"=");
			char *username = strtok(NULL,"=");

			strtok(name2,"=");
			char *filename = strtok(NULL,"=");

			strtok(name3,"=");
			char *filecontent = strtok(NULL,"=");

			printf("%s\n",username);
			printf("%s\n",filename);
			printf("%s\n",filecontent);


			int fd = open(filename,O_CREAT|O_RDWR|O_TRUNC,0664);
			int contentlen = strlen(filecontent);
			//写入数据
			write(fd,filecontent,contentlen);
			close(fd);
			//保存文件
			memset(filestore,'\0',sizeof(filestore));
			char *argv[4] = {"./uploadfile","/etc/fdfs/client.conf",filename,NULL};
			uploadfile(3, argv);
			//保存数据库
			writeDatabase(username,filename,contentlen);
			unlink(filename);
#endif
		}
		free(buf);

	}
	return 0;
}
