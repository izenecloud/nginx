project(nginx)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{EXTRA_CMAKE_MODULES_DIRS})

# dependencies
find_package(Perl)
if (NOT PERL_FOUND)
    message(FATAL "Perl not found.")
else()
    find_program(PROVE_EXECUTABLE NAMES prove DOC "Perl test runner")
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Prove DEFAULT_MSG PROVE_EXECUTABLE)
endif()

# nginx source dir
set(NGINX_SRC tengine)

# Test::Nginx path
set(TEST_NGINX_PATH ${nginx_SOURCE_DIR}/test-nginx)

# out of source build
set(BUILD_DIR ${nginx_BINARY_DIR}/${NGINX_SRC})
set(CONFIGURE_ARGS "--builddir=${BUILD_DIR}")

# nginx configuration options
option(WITH_REWRITE "enable the http_rewrite module" false)
if(NOT WITH_REWRITE)
   set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--without-http_rewrite_module")
endif()

# extra modules source dirs
set(ECHO_MODULE agentzh-echo-nginx-module-df58882)
set(SF1R_MODULE nginx-sf1r)

set(MODULE_DIRS
    ${ECHO_MODULE}
    ${SF1R_MODULE}
)

# add C++ support
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--with-ld-opt=-lstdc++")

# modules summary
if (MODULE_DIRS)
    message(STATUS "added modules:")
    foreach(module ${MODULE_DIRS})
        set(module_dir ${nginx_SOURCE_DIR}/${module})
        set(module_conf ${module_dir}/config)
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--add-module=${module_dir}")
        set(CONFIGURE_DEPS ${CONFIGURE_DEPS} ${module_conf})
        message("\t${module}")
    endforeach()
endif()

# config summary
message(STATUS "configure arguments: ")
foreach(arg ${CONFIGURE_ARGS})
    message("\t${arg}")
endforeach()

# configure script
add_custom_command(
    OUTPUT ${BUILD_DIR}/Makefile
    DEPENDS ${CONFIGURE_DEPS} 
    COMMAND ./configure ARGS ${CONFIGURE_ARGS}
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "configure nginx"
    VERBATIM) 

# make target
add_custom_target(nginx ALL
    COMMAND make
    DEPENDS ${BUILD_DIR}/Makefile
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "build nginx"
    VERBATIM)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUILD_DIR})

# test target
add_custom_target(test
     COMMAND ${CMAKE_COMMAND} 
          -DPROVE=${PROVE_EXECUTABLE}
          -DDIR=${nginx_SOURCE_DIR}/${SF1R_MODULE}
          -DLIB=${TEST_NGINX_PATH}/lib 
          -DINC=${TEST_NGINX_PATH}/inc
          -DNGINX=${BUILD_DIR}/nginx
          -P ${nginx_SOURCE_DIR}/test.cmake
     COMMENT "test ${SF1R_MODULE}")
add_dependencies(test nginx)

# install target
add_custom_target(install
    make install
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "installing nginx")
add_dependencies(install nginx)

