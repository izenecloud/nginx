# run Test::Nginx
set(ENV{TEST_NGINX_BINARY} ${NGINX})
execute_process(COMMAND ${PROVE} -v -I${LIB} -I${INC} WORKING_DIRECTORY ${DIR})
