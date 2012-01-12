/* 
 * File:   ngx_hello_module.cpp
 * Author: paolo
 * 
 * Created on January 12, 2012, 3:27 PM
 */

// nginx header files should go before other, because they define 64-bit off_t
extern "C" {
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
}
#include <string>

/// Location configuration structure.
typedef struct {
    ngx_str_t hello_str;
} ngx_hello_loc_conf_t;


// functions declaration
static char* ngx_hello(ngx_conf_t*, ngx_command_t*, void*);
static void* ngx_hello_create_loc_conf(ngx_conf_t*);
static char* ngx_hello_merge_loc_conf(ngx_conf_t*, void*, void*);


/// Module directives.
static ngx_command_t ngx_hello_commands[] = {
    { 
        ngx_string("hello"),                 // directive name
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, // location config, no arguments
        ngx_hello,                           // callback
        NGX_HTTP_LOC_CONF_OFFSET,            // configuration
        0,                                   // offset
        NULL                                 // post
    },
    {
        ngx_string("hello_str"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_hello_loc_conf_t, hello_str),
        NULL
    },
    ngx_null_command
};


/// Module context.
static ngx_http_module_t ngx_hello_module_ctx = {
    NULL, // preconfiguration
    NULL, // postconfiguration

    NULL, // create main configuration
    NULL, // init main configuration

    NULL, // create server configuration
    NULL, // init main configuration

    ngx_hello_create_loc_conf, // create location configuration
    ngx_hello_merge_loc_conf   // init location configuration
};


static void* 
ngx_hello_create_loc_conf(ngx_conf_t* cf) {
    ngx_hello_loc_conf_t* conf;

    // allocate module struct
    conf = (ngx_hello_loc_conf_t*) ngx_pcalloc(cf->pool, sizeof(ngx_hello_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    // init struct values
    //conf->hello_str = ngx_string("");
    conf->hello_str.len = 0;
    conf->hello_str.data = (u_char*) "";

    return conf;
}


static char* 
ngx_hello_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_hello_loc_conf_t* prev = (ngx_hello_loc_conf_t*) parent;
    ngx_hello_loc_conf_t* conf = (ngx_hello_loc_conf_t*) child;

    // merge values
    ngx_conf_merge_str_value(conf->hello_str, prev->hello_str, ""); // default to 1

    return NGX_CONF_OK;
}


/// Module definition
ngx_module_t ngx_hello_module = {
    NGX_MODULE_V1,
    &ngx_hello_module_ctx, // module context
    ngx_hello_commands,    // module directives
    NGX_HTTP_MODULE,            // module type
    NULL,                       // init master
    NULL,                       // init module
    NULL,                       // init process
    NULL,                       // init thread
    NULL,                       // exit thread
    NULL,                       // exit process
    NULL,                       // exit master
    NGX_MODULE_V1_PADDING
};


// useful strings
static const std::string helloString = "Ciao! 你好！";


static ngx_int_t
ngx_hello_handler(ngx_http_request_t* request) {
    ngx_int_t rc;       // return code
    ngx_buf_t* buffer;
    ngx_chain_t out;
    
    ngx_hello_loc_conf_t* conf;
    conf = (ngx_hello_loc_conf_t*) ngx_http_get_module_loc_conf(request, ngx_hello_module);

    // response to 'GET' and 'POST' requests only
    if (!(request->method & (NGX_HTTP_GET|NGX_HTTP_POST))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    /* set resonse header */
    
    //request->headers_out.content_type_len = sizeof("text/html") - 1;
    request->headers_out.content_type.len = sizeof("text/html") - 1;
    request->headers_out.content_type.data = (u_char*) "text/html";
    
    // send the header
    rc = ngx_http_send_header(request);
    
    /* set response body */
    
    // allocate a buffer
    buffer = (ngx_buf_t*) ngx_pcalloc(request->pool, sizeof(ngx_buf_t));
    if (buffer == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to allocate response buffer.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    // attach this buffer to the buffer chain
    out.buf = buffer;
    out.next = NULL;

    // adjust the pointers of the buffer
    buffer->pos = (u_char*) helloString.c_str();                              // first position of the data
    buffer->last = (u_char*) (helloString.c_str() + helloString.length());    // last position of the data
    buffer->memory = 1;                                             // this buffer is in memory (read-only)
    buffer->last_buf = 1;                                           // this is the last buffer in the buffer chain

    // set the status line
    request->headers_out.status = NGX_HTTP_OK;
    request->headers_out.content_length_n = helloString.length();

    
    if (rc == NGX_ERROR || rc > NGX_OK || request->header_only) {
        return rc;
    }

    // send the buffer chain
    return ngx_http_output_filter(request, &out);
}


static char*
ngx_hello(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_http_core_loc_conf_t* clcf;

    clcf = (ngx_http_core_loc_conf_t*) ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module); // get the core struct for this location
    clcf->handler = ngx_hello_handler; // handler to process the 'hello' directive

    return NGX_CONF_OK;
}