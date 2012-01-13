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


#define CAST(T,V)       static_cast< T >( V )
#define HELLO           "Ciao! 你好！"


/// Location configuration structure.
typedef struct {
    ngx_uint_t hello_uint; ///< test int 
    ngx_str_t  hello_str;  ///< test string
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
        ngx_string("hello_uint"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_hello_loc_conf_t, hello_uint),
        NULL,
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
    // allocate module struct
    ngx_hello_loc_conf_t* conf = CAST(ngx_hello_loc_conf_t*, ngx_pcalloc(cf->pool, sizeof(ngx_hello_loc_conf_t)));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    // init struct values
    conf->hello_uint = NGX_CONF_UNSET_UINT;
    conf->hello_str.len = 0;
    conf->hello_str.data = CAST(u_char*, NGX_CONF_UNSET_PTR);

    return conf;
}


static char* 
ngx_hello_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_hello_loc_conf_t* prev = CAST(ngx_hello_loc_conf_t*, parent);
    ngx_hello_loc_conf_t* conf = CAST(ngx_hello_loc_conf_t*, child);

    // merge values (with defaults)
    ngx_conf_merge_uint_value(conf->hello_uint, prev->hello_uint, 0);
    ngx_conf_merge_str_value(conf->hello_str, prev->hello_str, "");

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


static ngx_int_t
ngx_hello_handler(ngx_http_request_t* request) {
    //ngx_hello_loc_conf_t* conf = CAST(ngx_hello_loc_conf_t*, ngx_http_get_module_loc_conf(request, ngx_hello_module));

    // response to 'GET' and 'POST' requests only
    if (!(request->method & (NGX_HTTP_GET|NGX_HTTP_POST))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    /* do actual processing */
    std::string helloString(HELLO);
    helloString += 1;
    
    /* set response header */
    
    // set the status line
    request->headers_out.status = NGX_HTTP_OK;
    request->headers_out.content_length_n = helloString.length();
    request->headers_out.content_type_len = sizeof("text/html") - 1;
    request->headers_out.content_type.len = sizeof("text/html") - 1;
    request->headers_out.content_type.data = (u_char*) "text/html";
    
    /* set response body */
    
    // allocate a buffer
    ngx_buf_t* buffer = CAST(ngx_buf_t*, ngx_pcalloc(request->pool, sizeof(ngx_buf_t)));
    if (buffer == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to allocate response buffer.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    
    u_char* temp = (u_char*) helloString.c_str();
    
    // adjust the pointers of the buffer
    buffer->pos = temp;                              // first position of the data
    buffer->last = temp + helloString.length();      // last position of the data
    buffer->memory = 1;                                             // this buffer is in memory (read-only)
    buffer->last_buf = 1;                                           // this is the last buffer in the buffer chain

    // attach this buffer to the buffer chain
    ngx_chain_t out;
    out.buf = buffer;
    out.next = NULL;

    // send the header
    ngx_int_t rc = ngx_http_send_header(request);

    if (rc == NGX_ERROR || rc > NGX_OK || request->header_only) {
        return rc;
    }
    
    // send the buffer chain
    return ngx_http_output_filter(request, &out);
}


static char*
ngx_hello(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    // get the core struct for this location
    ngx_http_core_loc_conf_t* clcf = CAST(ngx_http_core_loc_conf_t*, ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module)); 
    clcf->handler = ngx_hello_handler; // handler to process the 'hello' directive

    return NGX_CONF_OK;
}