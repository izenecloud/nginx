# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
underscores_in_headers on;
location /sf1r/ {
    rewrite ^/sf1r(/.*)$ $1 break;
    sf1r;
    more_set_headers 'Access-Control-Allow-Origin: *';
    more_set_headers 'Access-Control-Allow-Methods: POST, GET, PUT, DELETE, OPTIONS';
    more_set_headers 'Access-Control-Allow-Headers: CONTENT-TYPE';
    more_set_headers 'Access-Control-Max-Age: 1728000';
    more_set_headers 'Access-Control-Allow-Credentials: false';
}
_EOC_

repeat_each(1);

plan tests => 8;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: tokens and headers
--- config eval: $::config
--- request
POST /sf1r/test/echo
{"message":"get request"}
--- more_headers
X_IZENESOFT_ACL_TOKENS: void
--- response_headers
content-type: application/json
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: POST, GET, PUT, DELETE, OPTIONS
Access-Control-Allow-Headers: CONTENT-TYPE
Access-Control-Max-Age: 1728000
Access-Control-Allow-Credentials: false
--- response_body: {"header":{"success":true},"message":"get request"}
