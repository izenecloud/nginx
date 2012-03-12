# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location /a/ {
        rewrite ^/a(/.*)$ $1 break;
        sf1r;
    }
    location /b/ {
        rewrite ^/b(/.*)$ $1 break;
        sf1r;
    }
_EOC_

repeat_each(1);

plan tests => 4;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: glog init
--- config eval: $::config
--- request eval
["GET /a/test/echo\r\n{\"message\":\"echo a\"}",
 "GET /b/test/echo\r\n{\"message\":\"echo b\"}"]
--- response_body eval
["{\"header\":{\"success\":true},\"message\":\"echo a\"}",
 "{\"header\":{\"success\":true},\"message\":\"echo b\"}"]
