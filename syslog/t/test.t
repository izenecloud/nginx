# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

our $config = <<'_EOC_';
    location = '/hello' {
        access_log "syslog:local6:info:127.0.0.1:5140:nginx";
        echo "Hello World";
    }
_EOC_

repeat_each(1);

plan tests => 2;

no_shuffle();
run_tests();

__DATA__


=== TEST 1: pipe to syslog
--- config eval: $::config
--- request
GET /hello
--- response_body
Hello World

