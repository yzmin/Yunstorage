//delete from filenames where filelength = 30;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "fcgi_stdio.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fdfs_client.h"
#include "logger.h"


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

//fdfs_download_file /etc/fdfs/client.conf group1/M00/00/00/Ciwa71aPesqAYgGIAAAACZn6Tco3979570
int downloadfile(int argc, char *argv[])
{
	char *conf_filename;
	char *local_filename;
	ConnectionInfo *pTrackerServer;
	int result;
	char file_id[128];
	int64_t file_size;
	int64_t file_offset;
	int64_t download_bytes;
	
	if (argc < 3)
	{
		printf("Usage: %s <config_file> <file_id> " \
			"[local_filename] [<download_offset> " \
			"<download_bytes>]\n", argv[0]);
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

	snprintf(file_id, sizeof(file_id), "%s", argv[2]);

	file_offset = 0;
	download_bytes = 0;
	if (argc >= 4)
	{
		local_filename = argv[3];
		if (argc >= 6)
		{
			file_offset = strtoll(argv[4], NULL, 10);
			download_bytes = strtoll(argv[5], NULL, 10);
		}
	}
	else
	{
		local_filename = strrchr(file_id, '/');
		if (local_filename != NULL)
		{
			local_filename++;  //skip /
		}
		else
		{
			local_filename = file_id;
		}
	}

	result = storage_do_download_file1_ex(pTrackerServer, \
                NULL, FDFS_DOWNLOAD_TO_FILE, file_id, \
                file_offset, download_bytes, \
                &local_filename, NULL, &file_size);
	if (result != 0)
	{
		printf("download file fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));
	}

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();

	return result;
}


//fdfs_delete_file /etc/fdfs/client.conf group1/M00/00/00/Ciwa71aPpn-AYqprAAASxtri93g66261.c filename
int deletefile(int argc, char *argv[])
{
	char *conf_filename;
	ConnectionInfo *pTrackerServer;
	int result;
	char file_id[128];
	
	if (argc < 3)
	{
		printf("Usage: %s <config_file> <file_id>\n", argv[0]);
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

	snprintf(file_id, sizeof(file_id), "%s", argv[2]);
	if ((result=storage_delete_file1(pTrackerServer, NULL, file_id)) != 0)
	{
		printf("delete file fail, " \
			"error no: %d, error info: %s\n", \
			result, STRERROR(result));
	}

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();

	return result;
}


int main(int argc,char **argv)
{
	MYSQL *mydb;//数据库句柄
	mydb = mysql_init(NULL);
	mydb = mysql_real_connect(mydb,"127.0.0.1","root","123456","YunDB",0,NULL,0);
	
	while(FCGI_Accept() >= 0)
	{
		printf("Content-type: text/html\r\n\r\n");
		int datalen = atoi(getenv("CONTENT_LENGTH"));
		char *buf = (char *)calloc(1,datalen);
		fread(buf,1,datalen,stdin);
		
		if(datalen > 0)
		{
			printf("buf = %s\n",buf);
			//去url编码
			eatspace(decode(buf));	
			//username=yzmin&filename=stdio.h&type=delete || download
			char *name1 = strtok(buf,"&");
			char *name2 = strtok(NULL,"&");
			char *name3 = strtok(NULL,"&");
			
			strtok(name1,"=");
			char *username = strtok(NULL,"=");

			strtok(name2,"=");
			char *filename = strtok(NULL,"=");

			strtok(name3,"=");
			char *type = strtok(NULL,"=");

			printf("%s\n",username);
			printf("%s\n",filename);
			printf("%s\n",type);
			
			char sql[1024] = {0};
			sprintf(sql,"select * from filenames where username = '%s' and filename = '%s'",username,filename);
			if(mysql_query(mydb,sql) < 0)
			{
				printf("username is nonentity");
				continue;
			}
			MYSQL_RES *res = mysql_store_result(mydb);
			MYSQL_ROW row = mysql_fetch_row(res);
			char *filestore = row[4];
			printf("%s\n",filestore);
			char *args[5] = {"./down_delete","/etc/fdfs/client.conf",filestore,filename,NULL};
			
			//下载
			if(strcmp(type,"download") == 0)
			{
				if(!downloadfile(4,args))//成功返回零
				{
					//打开文件
					int fd = open(filename,O_RDWR);
					if(fd > 0)
					{
						struct stat sfile;
						stat(filename,&sfile);
						printf("%s : %d\n","filename = ",(int)sfile.st_size);
						int len = (int)sfile.st_size;
						char *filecontent = (char *)calloc(sizeof(char),len);
						read(fd,filecontent,len);
						printf("%s\n",filecontent);
						close(fd); 
					}
					unlink(filename);
				}
				else
				{
					printf("download error\n");	
				}
			}
			//删除
			else if(strcmp(type,"delete") == 0)
			{
				memset(sql,0,sizeof(sql));
				sprintf(sql,"delete from filenames where filename = '%s' and filestore = '%s';",filename,filestore);
				if(!deletefile(4,args))//成功返回零
				{
					if(mysql_query(mydb,sql) < 0)
					{
						printf("delete db error\n");
						continue;
					}
					else
					{
						printf("delete ok\n");
						continue;
					}
				}
				else
				{
					printf("delete error\n");
					continue;
				}
			}
			else
			{
				continue;	
			}
		}
		
	}
	mysql_close(mydb);
	return 0;
}
