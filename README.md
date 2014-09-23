 Http reverse proxy for sf1r based on nginx 
============================================

### Features
* _Zookeeper Aware_: This reverse proxy requires [Zookeeper](zookeeper.apache.org/
) to get aware of SF1R's node topology. This is the first nginx project to be able to connect with `Zookeeper`. We delivered such feature through `nginx module`. Since this project was started in early 2012, while currently, if similar features are required, we might choose to recommend providing glue codes based on `nginx lua module`.

###Requirements
The `sf1r-module` uses the `libsf1r` in the [izenelib](https://github.com/izenecloud/izenelib) project.

The following libraries are also needed:
- Boost: system, threads
- [Google Glog](https://github.com/izenecloud/thirdparty/tree/master/glog)

###Installation

Check out the Nginx source code:
~~~
git clone https://github.com/izenecloud/nginx.git
~~~

Create the build directory:
~~~
cd nginx
mkdir build && cd build
~~~

Configure:
~~~
cmake ..
~~~

It is possible to enable the debug output using the following options:
- `-DWITH_DEBUG=1` enables the module debug output (disabled by default)
- `-DWITH_NGINX_DEBUG=1` enable the Nginx debug output (disabled by default)
- `-DJOBS=4` set the number of jobs, passed to the `make -j` option (default set to 2)

The installation prefix can also be specified with the 
`-DCMAKE_INSTALL_PREFIX=/path/to/[cmake](https://github.com/izenecloud/cmake)` option.

Build and install:
~~~
make install
~~~



### Documents
The technical report could be accessed [here](https://github.com/izenecloud/nginx/raw/master/docs/pdf/nginx-notes.pdf) and [here](https://github.com/izenecloud/nginx/raw/master/docs/pdf/sf1r-module.pdf).


###Configuration

Nginx can be configured by editing the file `conf/nginx.conf`.

###### Single SF1
Here is a **sample** configuration snippet, add it to the `server` section:
~~~
underscores_in_headers on;
location /sf1r/ {
    rewrite ^/sf1r(/.*)$ $1 break;
    sf1r_addr host1:port1;
    sf1r_poolSize 5;                                # default: 5
    sf1r_poolResize on;                             # default: off
    sf1r_poolMaxSize 10;                            # default: 25
    # allow use from Javascript
    more_set_headers 'Access-Control-Allow-Origin: *';
    more_set_headers 'Access-Control-Allow-Methods: POST, GET, PUT, DELETE, OPTIONS';
    more_set_headers 'Access-Control-Allow-Headers: CONTENT-TYPE';
    more_set_headers 'Access-Control-Max-Age: 1728000';
    more_set_headers 'Access-Control-Allow-Credentials: false';
}
~~~

###### Distributed SF1
Here is a **sample** configuration snippet, add it to the `server` section:
~~~
underscores_in_headers on;
location /sf1r/ {
    rewrite ^/sf1r(/.*)$ $1 break;
    sf1r_addr zookeeper1:port,zookeeper2:port distributed; 
    sf1r_poolSize 5;                                # default: 5
    sf1r_poolResize on;                             # default: off
    sf1r_poolMaxSize 10;                            # default: 25
    sf1r_zkTimeout 1000;                            # default: 2000
    sf1r_broadcast ^test/\w+$;                      # broadcast /test/* requests
    sf1r_broadcast ^recommend/visit_item$;          # broadcast visit_item requests
    # allow use from Javascript
    more_set_headers 'Access-Control-Allow-Origin: *';
    more_set_headers 'Access-Control-Allow-Methods: POST, GET, PUT, DELETE, OPTIONS';
    more_set_headers 'Access-Control-Allow-Headers: CONTENT-TYPE';
    more_set_headers 'Access-Control-Max-Age: 1728000';
    more_set_headers 'Access-Control-Allow-Credentials: false';
~~~

For more details about Nginx configuration, please refer to the [Nginx Wiki](http://wiki.nginx.org/Main "Nginx Wiki").


The following directive are available:

`sf1r_addr host1:port1[,host2:port2] [single|distributed]`
: Define the upstream SF1. 
  Using the `single` flag (default) only one SF1 address in the format `host:port` is required.
  When using the `distributed` flag it is possible to define more hosts, each of them creesponding to a ZooKeeper server.

`sf1r_poolSize size`
: Connection pool size (default: 5).

`sf1r_poolResize [on|off]`
: Connection pool automatic resize (default: off).

`sf1r_poolMaxSize size`
: Connection pool maximum size when resize is enabled (default: 25).

`sf1r_timeout seconds`
: Request timeout (default: 60).

`sf1r_zkTimeout millis`
: ZooKeeper timeout, available only when setting the `distributed` flag (default: 2000).

`sf1r_broadcast regex`
: Broadcast request matching the URI regex, available only when setting the `distributed` flag.


###Run

Nginx can be started with the command:
~~~
sbin/nginx
~~~

The `logs` directory will contain three files:
1. `access.log` the requested URI
2. `error.log` Nginx standard error
3. `nginx.pid` the master process' PID

More information on how to use Nginx command line is available at [this page](http://wiki.nginx.org/NginxCommandLine "Nginx Wiki").


##Higher performance

By default, nginx, together with `sf1r-module` could provide acceptable concurrency. While if higher concurrency load is required, `Fast-cgi` is a better choice. You could access it from [directory](https://github.com/izenecloud/sf1r-lite/tree/master/source/process/distribute/cgi) of SF1R.



### License
The SF1R project is published under the Apache License, Version 2.0:
http://www.apache.org/licenses/LICENSE-2.0
