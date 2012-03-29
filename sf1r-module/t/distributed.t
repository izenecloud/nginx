# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

use Test::More qw(no_plan);

repeat_each(1);
no_shuffle();
run_tests();

__DATA__


=== TEST 1: connection error
--- config
location /sf1r/ {
    sf1r;
    sf1r_addr somehost:2181 distributed;
}
--- request
GET /sf1r/test/echo
{"message":"Ciao! 你好！"}
--- error_code: 502


=== TEST 2: route requests
--- config
location /sf1r/ {
    rewrite ^/sf1r/(.*)$ $1 break;
    sf1r;
    sf1r_addr localhost:2181 distributed;
    sf1r_zkTimeout 2000;
}
--- request eval
["GET /sf1r/test/echo\r\n{\"message\":\"Ciao! 你好！\"}"
,"GET /sf1r/documents/search\r\n{\"collection\":\"example\",\"header\":{\"check_time\":true},\"search\":{\"keywords\":\"america\"},\"limit\":10}"
,"POST /sf1r/documents/search\r\n{\"collection\":\"example\",\"header\":{\"check_time\":true},\"search\":{\"keywords\":\"america\"}, \"limit\":10}"
]
--- response_body_like eval
["\"header\":{\"success\":true}"
,"\"header\":{\"success\":true}"
,"\"header\":{\"success\":true}"
]


=== TEST 3: broadcast requests
--- config
location /sf1r/ {
    rewrite ^/sf1r/(.*)$ $1 break;
    sf1r;
    sf1r_addr 180.153.140.110:2181,180.153.140.111:2181,180.153.140.112:2181 distributed;
    sf1r_zkTimeout 2000;
    sf1r_broadcast ^test/\w+$;
}
--- request eval
["POST /sf1r/test/echo\r\n{\"message\":\"Ciao! 你好！\"}"
,"POST /sf1r/documents/search\r\n{\"collection\":\"b5mm\",\"header\":{\"check_time\":true},\"search\":{\"keywords\":\"america\"},\"limit\":10}"
,"POST /sf1r/documents/search\r\n{\"collection\":\"b5ma\",\"header\":{\"check_time\":true},\"search\":{\"keywords\":\"america\"}, \"limit\":10}"
]
