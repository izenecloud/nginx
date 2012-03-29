# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);

plan tests => 2 * 3;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: rewrite
--- config
location /sf1r/ {
    rewrite ^/sf1r(/.*)$ $1 break;
    echo $uri;
}
--- request eval
["GET /sf1r/test/echo",
 "GET /sf1r/test",
 "GET /sf1r/"]
--- response_body eval
["/test/echo\n", "/test\n", "/\n"]
