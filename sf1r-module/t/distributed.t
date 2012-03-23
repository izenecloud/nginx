# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);

plan tests => 1 + (3 * 3 * (blocks() - 1));

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

=== TEST 2: distributed driver
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
