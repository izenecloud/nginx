# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

repeat_each(1);
plan tests => 3 * blocks(); # three checks for each block
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
qq(hello\nhello)


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
qq(哦\n哦\n哦)


=== TEST 4: hello_arr
--- config
location = /hello {
    hello;
    hello_arr my;
    hello_arr "dear friend";
}
--- request
GET /hello
--- response_headers
content-type: text/plain
--- response_body eval
qq(hello\nmy\ndear friend)
