# run Test::Nginx

set(ENV{TEST_NGINX_BINARY} ${NGINX})

execute_process(COMMAND ${PROVE} -v -I${LIB} -I${INC} 
    WORKING_DIRECTORY ${DIR}
    RESULT_VARIABLE result)

if(result)
    message(FATAL_ERROR "test ${NAME} failed")
endif()
