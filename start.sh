#! /bin/sh 

fdfs_trackerd /etc/fdfs/tracker.conf
fdfs_storaged /etc/fdfs/storage.conf

/usr/local/nginx/sbin/nginx -s reload

