/* 
 * File:   ngx_hello_module.cpp
 * Author: Paolo D'Apice
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

#define DEFAULT_STRING  "hello"
#define DEFAULT_COUNT   1
#define TEXT_PLAIN      "text/plain"


/// Location configuration structure.
typedef struct {
    ngx_uint_t   hello_count; ///< test int 
    ngx_str_t    hello_str;   ///< test string
    ngx_array_t* hello_arr;   ///< test array of strings
} ngx_hello_loc_conf_t;


// functions declaration
static char* ngx_hello(ngx_conf_t*, ngx_command_t*, void*);
static char* ngx_hello_arr_add(ngx_conf_t*, ngx_command_t*, void*);
static void* ngx_hello_create_loc_conf(ngx_conf_t*);
static char* ngx_hello_merge_loc_conf(ngx_conf_t*, void*, void*);


/// Module directives.
static ngx_command_t ngx_hello_commands[] = {
    { 
        ngx_string("hello"),                 // directive name
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, // location config, no arguments
        ngx_hello,                           // callback
        0,                                   // configuration
        0,                                   // offset
        NULL                                 // post
    },
    {
        ngx_string("hello_count"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_hello_loc_conf_t, hello_count),
        NULL,
    },
    {
        ngx_string("hello_str"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_hello_loc_conf_t, hello_str),
        NULL
    },
    {
        ngx_string("hello_arr"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_hello_arr_add,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_hello_loc_conf_t, hello_arr),
        NULL
    },
    ngx_null_command
};


/// Module context.
static ngx_http_module_t ngx_hello_module_ctx = {
    NULL,                                       // preconfiguration
    NULL,                                       // postconfiguration
    
    NULL,                                       // create main configuration
    NULL,                                       // init main configuration

    NULL,                                       // create server configuration
    NULL,                                       // init main configuration

    ngx_hello_create_loc_conf,                  // create location configuration
    ngx_hello_merge_loc_conf                    // init location configuration
};


static void* 
ngx_hello_create_loc_conf(ngx_conf_t* cf) {
    // allocate module struct
    ngx_hello_loc_conf_t* conf = CAST(ngx_hello_loc_conf_t*, ngx_pcalloc(cf->pool, sizeof(ngx_hello_loc_conf_t)));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    
    /*
     * initialized by ngx_pcalloc:
     * - conf->hello_arr = NULL
     */
    
    // init struct values
    conf->hello_count = NGX_CONF_UNSET_UINT;
    
    return conf;
}


static char* 
ngx_hello_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_hello_loc_conf_t* prev = CAST(ngx_hello_loc_conf_t*, parent);
    ngx_hello_loc_conf_t* conf = CAST(ngx_hello_loc_conf_t*, child);

    // merge values (with defaults)
    ngx_conf_merge_uint_value(conf->hello_count, prev->hello_count, DEFAULT_COUNT);
    ngx_conf_merge_str_value(conf->hello_str, prev->hello_str, DEFAULT_STRING);

    if (conf->hello_arr == NULL) {
        conf->hello_arr = prev->hello_arr;
    }
    
    // check values
    if (conf->hello_count < 1) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_count radius must be equal or more than 1");
        return CAST(char*, NGX_CONF_ERROR);
    }
    
    return NGX_CONF_OK;
}


/// Module definition
ngx_module_t ngx_hello_module = {
    NGX_MODULE_V1,
    &ngx_hello_module_ctx,      // module context
    ngx_hello_commands,         // module directives
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


static char*
ngx_hello_arr_add(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_hello_loc_conf_t* hlcf = CAST(ngx_hello_loc_conf_t*, conf);
    
    // create array
    if (hlcf->hello_arr == NULL) {
        hlcf->hello_arr = ngx_array_create(cf->pool, 1, sizeof(ngx_str_t)); // FIXME magic number: 1
        
        if (hlcf->hello_arr == NULL) {
            return (char*) NGX_CONF_ERROR;
        }
    }
    
    // allocate space for new element
    ngx_str_t* av = CAST(ngx_str_t*, ngx_array_push(hlcf->hello_arr));
    if (av == NULL) {
        return (char*) NGX_CONF_ERROR;
    }
    
    // set element value
    ngx_str_t* value = CAST(ngx_str_t*, cf->args->elts);
    av->len = value[1].len;
    av->data = value[1].data;
    ngx_log_debug1(NGX_LOG_NOTICE, cf->log, 0, "value = %s", av->data);
    
    return NGX_CONF_OK;
}


static ngx_int_t
ngx_hello_handler(ngx_http_request_t* request) {
    ngx_log_error(NGX_LOG_NOTICE, request->connection->log, 0, "hello handler");
    ngx_hello_loc_conf_t* conf = CAST(ngx_hello_loc_conf_t*, ngx_http_get_module_loc_conf(request, ngx_hello_module));

    // response to 'GET' and 'POST' requests only
    if (!(request->method & (NGX_HTTP_GET|NGX_HTTP_POST))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    /* do actual processing */
    
    std::string helloString((const char*) conf->hello_str.data);
    for (ngx_uint_t i = 1; i < conf->hello_count; ++i) {
        helloString.append("\n").append((const char*)conf->hello_str.data);
    }
    
    if (conf->hello_arr != NULL) {
        ngx_str_t* val = CAST(ngx_str_t*, conf->hello_arr->elts);
        for (ngx_uint_t i = 0; i < conf->hello_arr->nelts; ++i) {
            helloString.append("\n").append((const char*)val[i].data);
        }
        ngx_log_error(NGX_LOG_DEBUG, request->connection->log, 0, "hello: %s", helloString.c_str());
    }
    
    /* set response header */
    
    // set the status line
    request->headers_out.status = NGX_HTTP_OK;
    request->headers_out.content_length_n = helloString.length();
    request->headers_out.content_type_len = sizeof(TEXT_PLAIN) - 1;
    request->headers_out.content_type.len = sizeof(TEXT_PLAIN) - 1;
    request->headers_out.content_type.data = (u_char*) TEXT_PLAIN;
    
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
    buffer->memory = 1;                              // this buffer is in memory (read-only)
    buffer->last_buf = 1;                            // this is the last buffer in the buffer chain

    // attach this buffer to the buffer chain
    ngx_chain_t out;
    out.buf = buffer;
    out.next = NULL;

    /* send the response */
    ngx_int_t rc = ngx_http_send_header(request);

    if (rc == NGX_ERROR || rc > NGX_OK || request->header_only) {
        return rc;
    }
    
    return ngx_http_output_filter(request, &out);
}


static char*
ngx_hello(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    // get the core struct for this location
    ngx_http_core_loc_conf_t* clcf = CAST(ngx_http_core_loc_conf_t*, ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module)); 
    clcf->handler = ngx_hello_handler; // handler to process the 'hello' directive

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "hello module loaded");
    return NGX_CONF_OK;
}
