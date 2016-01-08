#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "fcgi_stdio.h"


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


int main(int argc,char **argv)
{
	char username[256] = { 0 };
	char userpasswd[256] = { 0 };
	char useremail[256] = { 0 };

	MYSQL *mydb;//数据库句柄
	mydb = mysql_init(NULL);
	mydb = mysql_real_connect(mydb,"127.0.0.1","root","123456","YunDB",0,NULL,0);


	while(FCGI_Accept() >= 0)
	{
		printf("Content-type: text/html\r\n\r\n");
		int length = atoi(getenv("CONTENT_LENGTH"));
		char *buf = (char *)calloc(1,length);
		fread(buf,1,length,stdin);
		//去url编码
		eatspace(decode(buf));
		sscanf(buf,"%*[^=]=%[^&]%*[^=]=%[^&]%*[^=]=%[^&]",username,userpasswd,useremail);

		printf("%s\n",username);
		printf("%s\n",userpasswd);
		printf("%s\n\n",useremail);

		char sql[1024] = {0};
		if(mydb && length > 0)
		{
			//登陆
			if(strcmp(useremail,"empty") == 0)
			{
				sprintf(sql,"select * from users where name = '%s'",username);
				if(mysql_query(mydb,sql) < 0)
				{
					printf("username is nonentity");
					continue;
				}
				MYSQL_RES *res = mysql_store_result(mydb);
				MYSQL_ROW row = mysql_fetch_row(res);
				char *tmp_pw = row[2];

				if(strcmp(userpasswd,tmp_pw) == 0)
				{
					printf("passwd ok");

				}
				else
				{
					printf("password is error");
					continue;	
				}
			}
			else
			{
				//注册
				sprintf(sql,"insert into users(name,passwd,email) values('%s','%s','%s');",username,userpasswd,useremail);
				mysql_query(mydb, "set names utf8");
				if(mysql_query(mydb,sql)!=0)
				{
					printf("username is exist");
					continue;
				}
				else
				{
					printf("login ok");
				}			
			}
		}
	}

	mysql_close(mydb);
	return EXIT_SUCCESS;	
}



/*
   mysql> describe users;
   +----------+------------------+------+-----+---------+----------------+
   | Field    | Type             | Null | Key | Default | Extra          |
   +----------+------------------+------+-----+---------+----------------+
   | id       | int(10) unsigned | NO   | PRI | NULL    | auto_increment |
   | name     | char(16)         | NO   |     | NULL    |                |
   | passwd   | char(64)         | NO   |     | NULL    |                |
   | email    | char(64)         | NO   |     | NULL    |                |
   | phonenum | char(13)         | YES  |     | -       |                |



   create table filenames(id int unsigned not null auto_increment primary key,
   -> username char(16) not null,
   -> filename char(255) not null,
   -> filelength int unsigned not null,
   -> filestore char(255) not null,
   -> date char(64) not null,
   -> MD5 char(64) default '-');


 */
