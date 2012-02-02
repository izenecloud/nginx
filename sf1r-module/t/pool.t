# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location /sf1r/ {
        rewrite ^/sf1r(/.*)$ $1 break;
        sf1r;
        sf1r_poolSize 1;
        sf1r_poolResize off;
        sf1r_poolMaxSize 5;
    }
_EOC_

repeat_each(1);

plan tests => 6;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: pool
--- config eval: $::config
--- pipelined_requests eval
["GET /sf1r/test/echo\r\n{\"message\":\"get request\"}",
"POST /sf1r/test/echo\r\n{\"message\":\"post request\"}"]
--- response_headers eval
["content-type: application/json",
"content-type: application/json"]
--- response_body eval
["{\"header\":{\"success\":true},\"message\":\"get request\"}",
"{\"header\":{\"success\":true},\"message\":\"post request\"}"]
