# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location /sf1r/ {
        rewrite ^/sf1r(/.*)$ $1 break;
        sf1r;
    }
_EOC_

repeat_each(1);

plan tests => 3 * blocks();

no_shuffle();
run_tests();

__DATA__

=== TEST 1: documents/search GET
--- config eval: $::config
--- request
GET /sf1r/documents/search
{ "collection":"example", "header":{"check_time":true}, "search":{"keywords":"手机"}, "limit":10}
--- error_code: 200
--- response_headers
content-type: application/json
--- response_body_like eval
qq("header":{"success":true})


=== TEST 2: documents/search POST
--- config eval: $::config
--- request
POST /sf1r/documents/search
{ "collection":"example", "header":{"check_time":true}, "search":{"keywords":"手机"}, "limit":10}
--- error_code: 200
--- response_headers
content-type: application/json
--- response_body_like eval
qq("header":{"success":true})

