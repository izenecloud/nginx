# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

use Test::More qw(no_plan);
repeat_each(1);
run_tests();

__DATA__


=== TEST 1: hello
--- config
location = /hello {
    hello;
    hello_str "this is the init test string";
}
--- request eval
["GET /hello", "POST /hello"]
