project(nginx)

cmake_minimum_required(VERSION 2.8)

# nginx source dir
set(NGINX_SRC nginx-1.0.11)

# out of source build
set(BUILD_DIR ${nginx_BINARY_DIR}/${NGINX_SRC})
set(CONFIGURE_ARGS "--builddir=${BUILD_DIR}")

# nginx configuration options
option(WITH_REWRITE "enable the http_rewrite module" false)
if(NOT WITH_REWRITE)
   set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--without-http_rewrite_module")
endif()

# extra modules
set(MODULE_DIRS
   agentzh-echo-nginx-module-df58882 
)

# add C++ support
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--with-ld-opt=-lstdc++")
set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--with-cpp_test_module")

if (MODULE_DIRS)
    message(STATUS "added modules:")
    foreach(module ${MODULE_DIRS})
        set(module_dir ${nginx_SOURCE_DIR}/${module})
        set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--add-module=${module_dir}")
        message("\t${module}")
    endforeach()
endif()

# summary
message(STATUS "configure arguments: ")
foreach(arg ${CONFIGURE_ARGS})
    message("\t${arg}")
endforeach()

# configure script
add_custom_command(
    OUTPUT ${BUILD_DIR}/Makefile
    COMMAND ./configure ARGS ${CONFIGURE_ARGS}
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "configuring nginx"
    VERBATIM) 

# make target
set(CMAKE_VERBOSE_MAKEFILE true)
add_custom_target(nginx
    ALL make
    DEPENDS ${BUILD_DIR}/Makefile
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "building nginx"
    VERBATIM) 
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUILD_DIR})

# install target
add_custom_target(install
    make install
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "installing nginx")
add_dependencies(install nginx)

