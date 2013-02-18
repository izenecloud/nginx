project(nginx)


cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} $ENV{EXTRA_CMAKE_MODULES_DIRS})


set(JOBS 2 CACHE STRING "Set the make -j argument")
message(STATUS "number of jobs: " ${JOBS})

# dependencies
find_package(Perl)
if (NOT PERL_FOUND)
    message(FATAL "Perl not found.")
else()
    find_program(PROVE_EXECUTABLE NAMES prove DOC "Perl test runner")
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Prove DEFAULT_MSG PROVE_EXECUTABLE)
endif()

find_package(izenelib REQUIRED COMPONENTS sf1r)
if (NOT izenelib_FOUND)
    message(FATAL "izenelib not found.")
else()
    foreach(include ${izenelib_INCLUDE_DIRS})
        list(APPEND SF1R_INCLUDES ${include})
    endforeach()
    string(REPLACE "/libsf1r.so" " " lib ${izenelib_sf1r_LIBRARY})
    list(APPEND SF1R_LIBRARIES "-L${lib} -lsf1r")
endif()

find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
if(NOT ImageMagick_FOUND)
    message(FATAL "ImageMagick not found")
else()
    foreach(include ${ImageMagick_INCLUDE_DIRS})
        list(APPEND SF1R_INCLUDES ${include})
    endforeach()
    foreach(library ${ImageMagick_LIBRARIES})
        list(APPEND SF1R_LIBRARIES ${library})
    endforeach()
endif()


set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0")
find_package(Boost 1.47.0 REQUIRED COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(FATAL "Boost.system not found.")
else()
    foreach(include ${Boost_INCLUDE_DIRS})
        list(APPEND SF1R_INCLUDES ${include})
    endforeach()
    foreach(library ${Boost_LIBRARIES})
        list(APPEND SF1R_LIBRARIES ${library})
    endforeach()
endif()

find_package(Glog)
if(NOT Glog_FOUND)
    message(FATAL "Boost.system not found.")
else()
    foreach(include ${Glog_INCLUDE_DIRS})
        list(APPEND SF1R_INCLUDES ${include})
    endforeach()
    list(APPEND SF1R_LIBRARIES ${Glog_LIBRARIES})
endif()

list(REMOVE_DUPLICATES SF1R_INCLUDES)


# nginx source dir
set(NGINX_SRC tengine)


# Test::Nginx path
set(TEST_NGINX_PATH ${nginx_SOURCE_DIR}/test-nginx)


# out of source build
set(BUILD_DIR ${nginx_BINARY_DIR}/${NGINX_SRC})
list(APPEND CONFIGURE_ARGS "--builddir=${BUILD_DIR}")


# debugging log
option(WITH_NGINX_DEBUG "enable the debug logging" false)
if(WITH_NGINX_DEBUG)
    list(APPEND CONFIGURE_ARGS "--with-debug")
endif()
option(WITH_DEBUG "enable the debug logging" false)
if(WITH_DEBUG)
    list(APPEND CONFIGURE_ARGS "--with-cc-opt=-DSDEBUG")
endif()

# nginx configuration options
option(WITH_REWRITE "enable the http_rewrite module" true)
if(NOT WITH_REWRITE)
   list(APPEND CONFIGURE_ARGS "--without-http_rewrite_module")
endif()


# extra modules source dirs
set(ECHO_MODULE agentzh-echo-nginx-module-df58882)
set(HEADERS_MORE_MODULE agentzh-headers-more-nginx-module-de77fd2)
set(HELLO_MODULE nginx-hello-module)
set(SF1R_MODULE sf1r-module)
set(FLUENTD_MODULE nginx-fluentd-module)
set(TFS_MODULE ngx_http_tfs_module)

list(APPEND MODULE_DIRS
    ${ECHO_MODULE}
    ${HEADERS_MORE_MODULE}
    ${HELLO_MODULE}
    ${SF1R_MODULE}
    ${FLUENTD_MODULE}
    ${TFS_MODULE}
)


# sf1r config
foreach(include ${SF1R_INCLUDES})
    set(sf1r_INCS "${sf1r_INCS} ${include}")
endforeach()
message(STATUS "include dirs: ${sf1r_INCS}")

foreach(lib ${SF1R_LIBRARIES})
    set(sf1r_LIBS "${sf1r_LIBS} ${lib}")
endforeach()
message(STATUS "libraries: ${sf1r_LIBS}")

configure_file(${nginx_SOURCE_DIR}/${SF1R_MODULE}/config.in 
               ${nginx_SOURCE_DIR}/${SF1R_MODULE}/config)

# modules summary
if (MODULE_DIRS)
    message(STATUS "added modules:")
    foreach(module ${MODULE_DIRS})
        set(module_dir ${nginx_SOURCE_DIR}/${module})
        set(module_conf ${module_dir}/config)
        list(APPEND CONFIGURE_ARGS "--add-module=${module_dir}")
        list(APPEND CONFIGURE_DEPS ${module_conf})
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
    COMMAND make -j ${JOBS}
    DEPENDS ${BUILD_DIR}/Makefile
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "build nginx"
    VERBATIM)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUILD_DIR})


# tests
enable_testing()

set(MODULE_TEST ${HELLO_MODULE} ${SF1R_MODULE})

if (MODULE_TEST)
    add_custom_target(test)
    message(STATUS "adding tests for modules:")
    foreach(module ${MODULE_TEST})
        set(module_dir ${nginx_SOURCE_DIR}/${module})
        file(GLOB test_files RELATIVE ${module_dir}/t/ ${module_dir}/t/*.t)
        list(LENGTH test_files count)
        message("\t${module} (${count} tests found)")
        foreach(test_file ${test_files})
            #message("\t* found test: ${module}/${test_file}")
            add_test(NAME ${module}/${test_file}
                COMMAND ${CMAKE_COMMAND}
                    -DNAME=${module}
                    -DPROVE=${PROVE_EXECUTABLE}
                    -DDIR=${module_dir}
                    -DLIB=${TEST_NGINX_PATH}/lib
                    -DINC=${TEST_NGINX_PATH}/inc
                    -DNGINX=${BUILD_DIR}/nginx
                    -DFILE=t/${test_file}
                    -P ${nginx_SOURCE_DIR}/test.cmake)
            endforeach()
    endforeach()
endif()


# install target
add_custom_target(install
    make install
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "installing nginx")
add_dependencies(install nginx)
