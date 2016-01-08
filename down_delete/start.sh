#! /bin/sh

make

spawn-fcgi -a 127.0.0.1 -p 8060 -f /usr/local/nginx/fastcgi_temp/Yunstorage/down_delete/download_delete

netstat -na | grep 8060

