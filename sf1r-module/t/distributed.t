# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

use Test::More qw(no_plan);

repeat_each(1);
no_shuffle();
run_tests();

__DATA__


=== TEST 1: connection error
--- config
location /sf1r/ {
    sf1r_addr somehost:2181 distributed;
}
--- request
GET /sf1r/test/echo
{"message":"Ciao! 你好！"}
--- error_code: 504


=== TEST 2: route requests
--- config
location /sf1r/ {
    rewrite ^/sf1r/(.*)$ $1 break;
    sf1r_addr localhost:2181 distributed;
    sf1r_zkTimeout 2000;
}
--- request eval
[qq(POST /sf1r/test/echo\r\n{"message":"Ciao! 你好！"})
,qq(POST /sf1r/documents/search\r\n{"collection":"b5mm","header":{"check_time":true},"search":{"keywords":"手机"},"limit":10})
,qq(POST /sf1r/documents/search\r\n{"collection":"b5mm","header":{"check_time":true},"search":{"keywords":"手机"},"limit":10})
]
--- response_body_like eval
[qq("header":{"success":true})
,qq("header":{"success":true})
,qq("header":{"success":true})
]


=== TEST 3: broadcast requests
--- config
location /sf1r/ {
    rewrite ^/sf1r/(.*)$ $1 break;
    sf1r_addr localhost:2181 distributed;
    sf1r_zkTimeout 2000;
    sf1r_broadcast ^test/\w+$;
}
--- request eval
[qq(POST /sf1r/test/echo\r\n{"message":"Ciao! 你好！"})
,qq(POST /sf1r/documents/search\r\n{"collection":"b5mm","header":{"check_time":true},"search":{"keywords":"手机"},"limit":10})
,qq(POST /sf1r/documents/search\r\n{"collection":"b5mm","header":{"check_time":true},"search":{"keywords":"手机"},"limit":10})
]
