/* 
 * File:   ngx_sf1r_module.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 17, 2012, 10:54 AM
 */

extern "C" {
#include "ngx_sf1r_ddebug.h"
#include "ngx_sf1r_handler.h"
#include "ngx_sf1r_module.h"
#include "ngx_sf1r_utils.h"
}
#include <glog/logging.h>
#include <net/sf1r/Sf1DriverBase.hpp>
#include <net/sf1r/Sf1Driver.hpp>
#include <string>

using NS_IZENELIB_SF1R::ServerError;
using NS_IZENELIB_SF1R::Sf1Config;
using NS_IZENELIB_SF1R::Sf1DriverBase;
using NS_IZENELIB_SF1R::Sf1Driver;
using std::string;


/// Activates the ngx_sf1r module.
static char* ngx_sf1r(ngx_conf_t*, ngx_command_t*, void*);

/// Handler for Sf1Driver initialization.
static ngx_int_t ngx_sf1r_init(ngx_sf1r_loc_conf_t*);

/// Handler for Sf1Driver finalization.
static void ngx_sf1r_cleanup(void*);

/// Handler for creating location configuration struct.
static void* ngx_sf1r_create_loc_conf(ngx_conf_t*);

/// Handler for merging two location configuration structs.
static char* ngx_sf1r_merge_loc_conf(ngx_conf_t*, void*, void*);


/// Module directives.
static ngx_command_t ngx_sf1r_commands[] = {
    { 
        ngx_string("sf1r"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_sf1r,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    {
        ngx_string("sf1r_addr"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, address),
        NULL
    },
    {
        ngx_string("sf1r_poolSize"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, poolSize),
        NULL
    },
    {
        ngx_string("sf1r_poolResize"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, poolResize),
        NULL
    },
    {
        ngx_string("sf1r_poolMaxSize"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, poolMaxSize),
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


/// Module definition.
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


static void* 
ngx_sf1r_create_loc_conf(ngx_conf_t* cf) {
    // allocate module struct
    ngx_sf1r_loc_conf_t* conf = scast(ngx_sf1r_loc_conf_t*, ngx_pcalloc(cf->pool, sizeof(ngx_sf1r_loc_conf_t)));
    if (conf == NULL) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, "failed to allocate memory");
        return NGX_CONF_ERROR;
    }
    
    // init struct values
    conf->enabled = NGX_CONF_UNSET;
    conf->driver= NULL;
    conf->poolSize = NGX_CONF_UNSET_UINT;
    conf->poolResize = NGX_CONF_UNSET;
    conf->poolMaxSize = NGX_CONF_UNSET_UINT;
    
    // allocate a cleanup handler
    ngx_pool_cleanup_t* cln = ngx_pool_cleanup_add(cf->pool, 0);
    if (cln == NULL) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, "failed to allocate memory");
        return NGX_CONF_ERROR;
    }

    cln->handler = ngx_sf1r_cleanup;
    cln->data = conf;
    
    return conf;
}


static char* 
ngx_sf1r_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_sf1r_loc_conf_t* prev = scast(ngx_sf1r_loc_conf_t*, parent);
    ngx_sf1r_loc_conf_t* conf = scast(ngx_sf1r_loc_conf_t*, child);

    // merge values (with defaults)
    
    ngx_conf_merge_str_value(conf->address, prev->address, SF1_DEFAULT_ADDR);
    
    ngx_conf_merge_value(conf->enabled, prev->enabled, FLAG_DISABLED);
    ngx_conf_merge_ptr_value(conf->driver, prev->driver, NULL);
    
    ngx_conf_merge_uint_value(conf->poolSize, prev->poolSize, SF1_DEFAULT_POOL_SIZE);
    ngx_conf_merge_value(conf->poolResize, prev->poolResize, FLAG_DISABLED);
    ngx_conf_merge_uint_value(conf->poolMaxSize, prev->poolMaxSize, SF1_DEFAULT_POOL_MAXSIZE);
    
    // init
    
    if (conf->enabled && conf->driver == NULL) {
        ngx_int_t rc = ngx_sf1r_init(conf);
        if (rc != NGX_OK) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "cannot connect to SF1");
            return scast(char*, NGX_CONF_ERROR);
        }
    } 
    
    return NGX_CONF_OK;
}

/**
 * Disabled explicit initialization to workaround the following
 * glog bugs:
 * - http://code.google.com/p/google-glog/issues/detail?id=83
 * - http://code.google.com/p/google-glog/issues/detail?id=113
 */
#undef INIT_GLOG

static ngx_int_t
ngx_sf1r_init(ngx_sf1r_loc_conf_t* conf) {
    string host((char*) conf->address.data, conf->address.len);
    
    try {
#ifdef INIT_GLOG
        ddebug("init logging system ...");
        google::InitGoogleLogging("ngx_sf1r");
#endif
        ddebug("instatiating driver ...");
        Sf1Config sf1conf;
        sf1conf.initialSize = conf->poolSize;
        sf1conf.resize = conf->poolResize;
        sf1conf.maxSize = conf->poolMaxSize;
        conf->driver = new Sf1Driver(host, sf1conf);
    } catch (ServerError& e) {
        ddebug("%s", e.what());
        return NGX_ERROR;
    }
    
    return NGX_OK;
}


static void 
ngx_sf1r_cleanup(void* data) {
    ngx_sf1r_loc_conf_t* conf = scast(ngx_sf1r_loc_conf_t*, data);
    
    if (conf->driver != NULL) {
        ddebug("deleting driver=%p ...", conf->driver);
        Sf1Driver* driver = scast(Sf1Driver*, conf->driver);
        delete driver;
#ifdef INIT_GLOG
        ddebug("shutting down logging system ...");
        google::ShutdownGoogleLogging();
#endif
    }
}


static char*
ngx_sf1r(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    // get the core struct for this location
    ngx_http_core_loc_conf_t* clcf = scast(ngx_http_core_loc_conf_t*, ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module)); 
    clcf->handler = ngx_sf1r_handler;
    
    ngx_sf1r_loc_conf_t* slc = scast(ngx_sf1r_loc_conf_t*, conf);
    slc->enabled = 1;
    
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "sf1r module loaded");
    return NGX_CONF_OK;
}
