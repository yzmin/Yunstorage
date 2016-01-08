#! /bin/sh

make

spawn-fcgi -a 127.0.0.1 -p 8040 -f /usr/local/nginx/fastcgi_temp/Yunstorage/login_register/register

netstat -na | grep 8040



