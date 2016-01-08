src = $(wildcard *.c)
target = $(patsubst %.c, %, $(src))

all: $(target)

$(target):%:%.c
	gcc $< -o $@ -I/usr/include/fastcommon -I/usr/include/fastdfs -I/usr/local/include/fastdfs -L/usr/local/lib -lfastcommon -lfdfsclient  -lpthread -ldl -rdynamic -I/usr/include/mysql/ -L/usr/lib/mysql/ -lmysqlclient -lstdc++ -ldl -lpthread -lrt -lfcgi 
	
clean:
	-rm -f $(target) 
						
.PHONY: clean all
