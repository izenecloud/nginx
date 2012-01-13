
default:	build

clean:
	rm -rf Makefile /home/paolo/Projects/nginx-cmake/build/tengine

build:
	$(MAKE) -f /home/paolo/Projects/nginx-cmake/build/tengine/Makefile
	$(MAKE) -f /home/paolo/Projects/nginx-cmake/build/tengine/Makefile manpage

install:
	$(MAKE) -f /home/paolo/Projects/nginx-cmake/build/tengine/Makefile install

upgrade:
	/usr/local/nginx/sbin/nginx -t

	kill -USR2 `cat /usr/local/nginx/logs/nginx.pid`
	sleep 1
	test -f /usr/local/nginx/logs/nginx.pid.oldbin

	kill -QUIT `cat /usr/local/nginx/logs/nginx.pid.oldbin`
