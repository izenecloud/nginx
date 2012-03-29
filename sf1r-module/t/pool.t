# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);

plan tests => 6;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: pool
--- config
location /sf1r/ {
    rewrite ^/sf1r(/.*)$ $1 break;
    sf1r;
    sf1r_poolSize 1;
    sf1r_poolResize off;
    sf1r_poolMaxSize 5;
}
--- request eval
[qq(GET /sf1r/test/echo\r\n{"message":"get request"})
,qq(POST /sf1r/test/echo\r\n{"message":"post request"})
]
--- response_headers eval
["content-type: application/json"
,"content-type: application/json"
]
--- response_body eval
[qq({"header":{"success":true},"message":"get request"})
,qq({"header":{"success":true},"message":"post request"})
]
