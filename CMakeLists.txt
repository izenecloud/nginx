project(nginx)

cmake_minimum_required(VERSION 2.8)

# nginx source dir
set(NGINX_SRC nginx-1.0.11)

# out of source build
set(BUILD_DIR ${nginx_BINARY_DIR}/${NGINX_SRC})
set(CONFIGURE_ARGS "--builddir=${BUILD_DIR}")

# nginx configuration options
option(WITH_REWRITE false)
if(NOT WITH_REWRITE)
   set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--without-http_rewrite_module")
endif()

# extra modules
set(MODULE_DIRS
   agentzh-echo-nginx-module-df58882 
)

foreach(module ${nginx_MODULE_DIRS})
    set(module_dir ${nginx_SOURCE_DIR}/${module})
    set(CONFIGURE_ARGS ${CONFIGURE_ARGS} "--add-module=${module_dir}")
endforeach()

message(STATUS "configure arguments: ")
foreach(arg ${CONFIGURE_ARGS})
    message("\t${arg}")
    set(CONFIGURE_ARGUMENTS ${CONFIGURE_ARGUMENTS} "${arg}")
endforeach()

# configure target
add_custom_target(configure
    ./configure ${CONFIGURE_ARGUMENTS}
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "./configure ${CONFIGURE_ARGS}"
    VERBATIM) 

# make target
set(CMAKE_VERBOSE_MAKEFILE true)
add_custom_target(nginx
    ALL make
    WORKING_DIRECTORY ${nginx_SOURCE_DIR}/${NGINX_SRC}
    COMMENT "make"
    VERBATIM) 
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${BUILD_DIR})

