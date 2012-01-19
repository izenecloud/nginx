# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location /sf1r/ {
        rewrite ^/sf1r/(.*)$ $1 break;
        sf1r;
    }
_EOC_

repeat_each(1);

plan tests => 4;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: bad request
--- config eval: $::config
--- request
GET /sf1r/test/echo
--- error_code: 400


=== TEST 2: service unavailable
--- config
location /sf1r/ {
    sf1r;
    sf1r_port 8080;
}
--- request
GET /sf1r/test/echo
{"message":"Ciao! 你好！"}
--- error_code: 503


=== TEST 3: malformed request
--- config eval: $::config
--- request
GET /sf1r/test/echo
{"message":"Ciao! 你好！"}
header test
--- error_code: 400


=== TEST 4: header only
--- config eval: $::config
--- raw_request eval
["GET /sf1r/test/echo HTTP/1.0\r\n\r\n"]
--- error_code: 400

