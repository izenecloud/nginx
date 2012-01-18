/* 
 * File:   ngx_sf1r_module.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 17, 2012, 10:54 AM
 */

extern "C" {
#include "ngx_sf1r_handler.h"
#include "ngx_sf1r_module.h"
#include "ngx_sf1r_utils.h"
}
#include <string>


// functions declaration
static char* ngx_sf1r(ngx_conf_t*, ngx_command_t*, void*);
static void* ngx_sf1r_create_loc_conf(ngx_conf_t*);
static char* ngx_sf1r_merge_loc_conf(ngx_conf_t*, void*, void*);


/// Module directives.
static ngx_command_t ngx_sf1r_commands[] = {
    { 
        ngx_string("sf1r"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_sf1r,
        0,
        0,
        NULL
    },
    {
        ngx_string("sf1r_port"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, port),
        NULL,
    },
    {
        ngx_string("sf1r_addr"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, address),
        NULL
    },
    ngx_null_command
};


/// Module context.
static ngx_http_module_t ngx_sf1r_module_ctx = {
    NULL,
    NULL,
    
    NULL,
    NULL,

    NULL,
    NULL,

    ngx_sf1r_create_loc_conf,
    ngx_sf1r_merge_loc_conf
};


static void* 
ngx_sf1r_create_loc_conf(ngx_conf_t* cf) {
    // allocate module struct
    ngx_sf1r_loc_conf_t* conf = scast(ngx_sf1r_loc_conf_t*, ngx_pcalloc(cf->pool, sizeof(ngx_sf1r_loc_conf_t)));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    
    // init struct values
    conf->port = NGX_CONF_UNSET_UINT;
    
    return conf;
}


static char* 
ngx_sf1r_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_sf1r_loc_conf_t* prev = scast(ngx_sf1r_loc_conf_t*, parent);
    ngx_sf1r_loc_conf_t* conf = scast(ngx_sf1r_loc_conf_t*, child);

    // merge values (with defaults)
    ngx_conf_merge_uint_value(conf->port, prev->port, 18181);
    ngx_conf_merge_str_value(conf->address, prev->address, "localhost");

    return NGX_CONF_OK;
}


/// Module definition
ngx_module_t ngx_sf1r_module = {
    NGX_MODULE_V1,
    &ngx_sf1r_module_ctx,
    ngx_sf1r_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};


static char*
ngx_sf1r(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    // get the core struct for this location
    ngx_http_core_loc_conf_t* clcf = scast(ngx_http_core_loc_conf_t*, ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module)); 
    clcf->handler = ngx_sf1r_handler; // handler to process the 'hello' directive

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "sf1r module loaded");
    return NGX_CONF_OK;
}
