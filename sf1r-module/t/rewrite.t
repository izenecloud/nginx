# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location /sf1r/ {
        rewrite ^/sf1r(/.*)$ $1 break;
	echo $uri;
    }
_EOC_

repeat_each(1);

plan tests => 2;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: rewrite
--- config eval: $::config
--- request
GET /sf1r/test/echo
--- response_body
/test/echo
