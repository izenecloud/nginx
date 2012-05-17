# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location /sf1r/ {
        rewrite ^/sf1r(/.*)$ $1 break;
        sf1r_addr localhost:18181;
    }
_EOC_

plan tests => 2 * blocks();

no_shuffle();
run_tests();

__DATA__

=== TEST 1: GET documents/search
--- config eval: $::config
--- request
GET /sf1r/documents/search
{"uri": "documents","collection": "b5mp","search":{"keywords": "T80-18-803"}}
--- response_body_like eval
qq("header":{"success":true})