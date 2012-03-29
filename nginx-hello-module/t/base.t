use lib 'lib';
use Test::Nginx::Socket;

# Nginx configuration fixture
our $config = <<'_EOC_';
    location = /hello {
        hello;
    }
_EOC_

# test configuration

#plan tests => 5;
use Test::More qw(no_plan);

repeat_each();
#no_shuffle();
run_tests();

__DATA__


=== TEST 1: hit
--- config eval: $::config
--- request
GET /hello


=== TEST 2: not found
--- config eval: $::config
--- request
GET /some
--- error_code: 404


=== TEST 3: response
--- config eval: $::config
--- request
GET /hello
--- response_headers
Content-type: text/plain
--- response_body eval
"hello"
