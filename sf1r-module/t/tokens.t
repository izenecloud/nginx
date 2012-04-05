# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $http_config = <<'_EOC_';
    underscores_in_headers on;
_EOC_

our $config = <<'_EOC_';
    location /sf1r/ {
        rewrite ^/sf1r(/.*)$ $1 break;
        sf1r_addr localhost:18181;
        sf1r_poolSize 1;
        sf1r_poolResize off;
        sf1r_poolMaxSize 5;
    }
_EOC_

repeat_each(1);

plan tests => 3;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: tokens
--- http_config eval: $::http_config
--- config eval: $::config
--- request
GET /sf1r/test/echo
{"message":"get request"}
--- more_headers
X_IZENESOFT_ACL_TOKENS: token
--- response_headers
content-type: application/json
--- response_body
{"header":{"success":true},"message":"get request"}