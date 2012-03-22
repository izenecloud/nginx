# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

plan tests => 1 * blocks();

no_shuffle();
run_tests();

__DATA__

=== TEST 1: documents/search GET
--- config
location /sf1r/ {
    rewrite ^/sf1r(/.*)$ $1 break;
    sf1r;
}
--- request
GET /sf1r/documents/search
{"search":{"ranking_model":"plm","log_keywords":false,"keywords":"*","analyzer":{"apply_la":true,"use_synonym_extension":true,"use_original_keyword":false},"group_label":[{"value":["数码"],"property":"Category"}],"in":[{"property":"Title"},{"property":"Category"},{"property":"Source"}]},"limit":20,"sort":[{"property":"_ctr","order":"DESC"}],"header":{"check_time":"true"},"remove_duplicated_result":false,"collection":"b5mp","offset":0,"conditions":[],"select":[{"summary":false,"highlight":true,"property":"DOCID","snippet":true},{"summary":false,"highlight":true,"property":"Title","snippet":true},{"summary":false,"highlight":true,"property":"Url","snippet":true},{"summary":false,"highlight":true,"property":"DATE","snippet":true},{"summary":false,"highlight":true,"property":"Content","snippet":true},{"summary":false,"highlight":true,"property":"Price","snippet":true},{"summary":false,"highlight":true,"property":"Picture","snippet":true},{"summary":false,"highlight":true,"property":"Category","snippet":true},{"summary":false,"highlight":true,"property":"Source","snippet":true},{"summary":false,"highlight":true,"property":"itemcount","snippet":true}]}
