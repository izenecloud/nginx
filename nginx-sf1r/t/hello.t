# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);

#plan tests => 5;
use Test::More qw(no_plan);

no_shuffle();
run_tests();

__DATA__


=== TEST 1: hello
--- config
location = /hello {
    hello;
}
--- request
GET /hello
--- response_headers
content-type: text/plain
--- response_body eval
"hello"


=== TEST 2: hello_count
--- config
location = /hello {
    hello;
    hello_count 2;
}
--- request
GET /hello
--- response_headers
content-type: text/plain
--- response_body eval 
"hello
hello"


=== TEST 3: hello_str
--- config
location = /hello {
    hello;
    hello_count 3;
    hello_str "哦";
}
--- request
GET /hello
--- response_headers
content-type: text/plain
--- response_body eval 
"哦
哦
哦"


