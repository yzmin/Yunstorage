简单云存储方式
采用FastDF分式云存储，通过FastCGI与服务器通信，使用Nginx web服务器反向代理与客户端通信 
采用Mysql数据库简单储存信息

	location =/register.cgi {
                charset utf-8;
                root /usr/local/nginx/fastcgi_temp/Yunstorage/login_register;
                fastcgi_pass 127.0.0.1:8040;
                fastcgi_index index.cgi;
                include fastcgi_params;
        }

        location =/uploadfile.cgi {
                charset utf-8;
                root /usr/local/nginx/fastcgi_temp/Yunstorage/upload;
                fastcgi_pass 127.0.0.1:8050;
                fastcgi_index index.cgi;
                include fastcgi_params;
        }

        location =/downloaddelete.cgi {
                charset utf-8;
                root /usr/local/nginx/fastcgi_temp/Yunstorage/down_delete;
                fastcgi_pass 127.0.0.1:8060;
                fastcgi_index index.cgi;
                include fastcgi_params;
        }

mysql> select * from filenames;
+----+-----------+----------------------------------+------------+-----------------------------------------------------+---------------------+------+
| id | username  | filename                         | filelength | filestore                                           | date                | MD5  |
+----+-----------+----------------------------------+------------+-----------------------------------------------------+---------------------+------+
|  1 | yzmin     | stdio.h                          |       1024 | /usr/include                                        | 2016-1-8 15:48:30   | -    |
|  4 | 叶志敏    | e10adc3949ba59abbe56e057f20f883e |         52 | group1/M00/00/00/Ciwa71aPfLOAA64nAAAANMh0uiY4567085 | 2016/01/08 17:09:07 | -    |
+----+-----------+----------------------------------+------------+-----------------------------------------------------+---------------------+------+
2 rows in set (0.00 sec)

mysql> select * from users;
+----+-----------+--------+------------------+-------------+
| id | name      | passwd | email            | phonenum    |
+----+-----------+--------+------------------+-------------+
|  1 | 叶志敏    | 567456 | 178307940@qq.com | 15889970648 |
+----+-----------+--------+------------------+-------------+
1 row in set (0.00 sec)

