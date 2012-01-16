# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);

plan tests => 5;
#use Test::More qw(no_plan);

$ENV{TEST_NGINX_SF1R_HOST} ||= '127.0.0.1';
$ENV{TEST_NGINX_SF1R_PORT} ||= 18181;

our $config = <<'_EOC_';
    location = /echo {
        echo hello;
    }
_EOC_

no_shuffle();
run_tests();

__DATA__


=== TEST 1: hit
--- config eval: $::config
--- request
GET /echo


=== TEST 2: not found
--- config eval: $::config
--- request
GET /some
--- error_code: 404


=== TEST 3: response
--- config eval: $::config
--- request
GET /echo
--- response_headers
Content-type: text/plain
--- response_body
hello
