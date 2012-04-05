# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location /sf1r/ {
        rewrite ^/sf1r(/.*)$ $1 break;
        sf1r_addr localhost:18181;
    }
_EOC_

repeat_each(1);

plan tests => 6;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: test/echo GET
--- config eval: $::config
--- request
GET /sf1r/test/echo
{"message":"Ciao! 你好！"}
--- response_headers
content-type: application/json
--- response_body eval
qq({"header":{"success":true},"message":"Ciao! 你好！"})


=== TEST 2: test/echo POST
--- config eval: $::config
--- request
POST /sf1r/test/echo
{"message":"Ciao! 你好！"}
--- response_headers
content-type: application/json
--- response_body eval
qq({"header":{"success":true},"message":"Ciao! 你好！"})
