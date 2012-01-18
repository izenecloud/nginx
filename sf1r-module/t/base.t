# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);

#plan tests => 5;
use Test::More qw(no_plan);

no_shuffle();
run_tests();

__DATA__


=== TEST 1: sanity
--- config
location = /hello {
    sf1r;
}
--- request
GET /hello
--- response_headers
content-type: application/json

