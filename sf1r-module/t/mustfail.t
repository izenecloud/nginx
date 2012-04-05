# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

use Test::More qw(no_plan);
run_tests();

__DATA__


=== TEST 1: cannot start (single)
--- config
location /sf1r/ {
    rewrite ^/sf1r/(.*)$ $1 break;
    sf1r_addr "somewhere:1234";
}
--- request
GET /sf1r


=== TEST 2: cannot start (distributed)
--- config
location /sf1r/ {
    rewrite ^/sf1r/(.*)$ $1 break;
    sf1r_addr "somewhere:1234" distributed;
}
--- request
GET /sf1r