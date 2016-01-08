#! /bin/sh

make 

spawn-fcgi -a 127.0.0.1 -p 8050 -f /usr/local/nginx/fastcgi_temp/Yunstorage/upload/uploadfile

netstat -na | grep 8050

