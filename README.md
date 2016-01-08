
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

