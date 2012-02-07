# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);

plan tests => 3;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: sanity
--- http_config
underscores_in_headers on;
--- config
location /sf1r/ {
    rewrite ^/sf1r(/.*)$ $1 break;
    sf1r;
}
--- request
GET /sf1r/test/echo
{"message":"get request"}
--- more_headers
X_IZENESOFT_ACL_TOKENS: token
--- response_headers
content-type: application/json
--- response_body
{"header":{"success":true},"message":"get request"}