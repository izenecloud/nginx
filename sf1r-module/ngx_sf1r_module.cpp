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
#include <glog/logging.h>
#include <net/sf1r/Sf1DriverBase.hpp>
#include <net/sf1r/Sf1Driver.hpp>
#include <net/sf1r/distributed/Sf1DistributedDriver.hpp>
#include <string>

using namespace NS_IZENELIB_SF1R;
using std::string;


/// Sets the URI patterns for broadcasted requests.
static char* ngx_sf1r_broadcast(ngx_conf_t*, ngx_command_t*, void*);

/// Handler for Sf1Driver initialization.
static ngx_int_t ngx_sf1r_init(ngx_sf1r_loc_conf_t*, ngx_log_t*);

/// Handler for Sf1Driver finalization.
static void ngx_sf1r_cleanup(ngx_sf1r_loc_conf_t*, ngx_log_t*);

/// Handler for creating main configuration struct.
static void* ngx_sf1r_create_main_conf(ngx_conf_t*);

/// Handler for creating location configuration struct.
static void* ngx_sf1r_create_loc_conf(ngx_conf_t*);

/// Handler for merging two location configuration structs.
static char* ngx_sf1r_merge_loc_conf(ngx_conf_t*, void*, void*);

/// Handler for the 'sf1_addr' directive.
static char* ngx_sf1r_addr_set(ngx_conf_t*, ngx_command_t*, void*);

/// Callback to be executed on module init.
static ngx_int_t ngx_sf1r_init_module(ngx_cycle_t*);

/// Callback to be executed on worker process init.
static ngx_int_t ngx_sf1r_init_process(ngx_cycle_t*);

/// Callback to be executed on worker process exit;
static void ngx_sf1r_exit_process(ngx_cycle_t*);


/// Module directives.
static ngx_command_t ngx_sf1r_commands[] = {
    {
        ngx_string("sf1r_addr"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE12,
        ngx_sf1r_addr_set,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
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
    {
        ngx_string("sf1r_zkTimeout"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, zkTimeout),
        NULL
    },
    {
        ngx_string("sf1r_broadcast"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
        ngx_sf1r_broadcast,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_sf1r_loc_conf_t, broadcasted),
        NULL
    },
    ngx_null_command
};


/// Module context.
static ngx_http_module_t ngx_sf1r_module_ctx = {
    NULL,
    NULL,
    ngx_sf1r_create_main_conf,
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
    ngx_sf1r_init_module,
    ngx_sf1r_init_process,
    NULL,
    NULL,
    ngx_sf1r_exit_process,
    NULL,
    NGX_MODULE_V1_PADDING
};


static void* 
ngx_sf1r_create_main_conf(ngx_conf_t* cf) {
    ngx_sf1r_main_conf_t* conf = scast(ngx_sf1r_main_conf_t*, 
            ngx_pcalloc(cf->pool, sizeof(ngx_sf1r_main_conf_t)));
    if (conf == NULL) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, "failed to allocate memory");
        return NULL;
    }
    
    if (ngx_array_init(&conf->loc_confs, cf->pool, SF1_ARRAY_INIT_SIZE,
            sizeof(ngx_sf1r_loc_conf_t*)) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, "failed to allocate memory");
        return NGX_CONF_ERROR;
    }
    
    return conf;
}


static void* 
ngx_sf1r_create_loc_conf(ngx_conf_t* cf) {
    // allocate module struct
    ngx_sf1r_loc_conf_t* conf = scast(ngx_sf1r_loc_conf_t*, ngx_pcalloc(cf->pool, sizeof(ngx_sf1r_loc_conf_t)));
    if (conf == NULL) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, "failed to allocate memory");
        return NGX_CONF_ERROR;
    }
    
    // init struct values
    conf->address.data = NULL;
    conf->address.len = 0;
    conf->distributed = NGX_CONF_UNSET;
    
    conf->poolSize = NGX_CONF_UNSET_UINT;
    conf->poolResize = NGX_CONF_UNSET;
    conf->poolMaxSize = NGX_CONF_UNSET_UINT;
    conf->zkTimeout = NGX_CONF_UNSET_UINT;
    
    /*
     * initialized by ngx_pcalloc:
     * conf->broadcasted = NULL;
     */
    
    conf->driver= NULL;
    
    return conf;
}


static char* 
ngx_sf1r_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_sf1r_loc_conf_t* prev = scast(ngx_sf1r_loc_conf_t*, parent);
    ngx_sf1r_loc_conf_t* conf = scast(ngx_sf1r_loc_conf_t*, child);

    // merge values (with defaults)
    
    ngx_conf_merge_str_value(conf->address, prev->address, NULL);
    ngx_conf_merge_value(conf->distributed, prev->distributed, NGX_CONF_UNSET);
    ngx_conf_merge_ptr_value(conf->driver, prev->driver, NULL);
    
    ngx_conf_merge_uint_value(conf->poolSize, prev->poolSize, SF1_DEFAULT_POOL_SIZE);
    ngx_conf_merge_value(conf->poolResize, prev->poolResize, FLAG_DISABLED);
    ngx_conf_merge_uint_value(conf->poolMaxSize, prev->poolMaxSize, SF1_DEFAULT_POOL_MAXSIZE);
    ngx_conf_merge_uint_value(conf->zkTimeout, prev->zkTimeout, SF1_DEFAULT_ZK_TIMEOUT);
    
    if (conf->broadcasted == NULL) {
        conf->broadcasted = prev->broadcasted;
    }
    
    // add to the main conf struct
    ngx_sf1r_main_conf_t* main = scast(ngx_sf1r_main_conf_t*, 
            ngx_http_conf_get_module_main_conf(cf, ngx_sf1r_module));
    
    if (conf->address.data) {
        ngx_log_error(NGX_LOG_DEBUG, cf->log, 0, "adding location for address %s to main conf", conf->address.data);
        
        ngx_sf1r_loc_conf_t** loc = scast(ngx_sf1r_loc_conf_t**, 
                ngx_array_push(&main->loc_confs));
        *loc = conf;
    }
    
    return NGX_CONF_OK;
}


char*
ngx_sf1r_addr_set(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_sf1r_loc_conf_t* lcf = scast(ngx_sf1r_loc_conf_t*, conf);
    
    if (lcf->address.data) {
        return (char*) "is duplicate";
    }

    ngx_str_t* value = scast(ngx_str_t*, cf->args->elts);
    ngx_log_error(NGX_LOG_INFO, cf->log, 0, "value[1] = (%s)", value[1].data);
    ngx_log_error(NGX_LOG_INFO, cf->log, 0, "value[2] = (%s)", value[2].data);
    
    // address
    lcf->address.data = value[1].data;
    lcf->address.len = value[1].len;
    
    // single/distributed flag
    ngx_str_t* flag = value + 2;
    if (ngx_strcmp(flag->data, "$1") == 0 
            or ngx_strcmp(flag->data, SF1_SINGLE_LABEL) == 0) {
        ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "single SF1 address \"%s\"", lcf->address.data);
        lcf->distributed = FLAG_DISABLED;
    } else if (ngx_strcmp(flag->data, SF1_DISTRIBUTED_LABEL) == 0) {
        ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "distributed SF1 address \"%s\"", lcf->address.data);
        lcf->distributed = FLAG_ENABLED;
    } else {
        return (char*) "not valid";
    }
    
    // get the core struct for this location
    ngx_http_core_loc_conf_t* clcf = scast(ngx_http_core_loc_conf_t*, ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module)); 
    clcf->handler = ngx_sf1r_handler;
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "sf1r module loaded");
    
    return NGX_CONF_OK;
}


static char* 
ngx_sf1r_broadcast(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_sf1r_loc_conf_t* lcf = scast(ngx_sf1r_loc_conf_t*, conf);
    
    // create array
    if (lcf->broadcasted == NULL) {
        lcf->broadcasted = ngx_array_create(cf->pool, SF1_ARRAY_INIT_SIZE, sizeof(ngx_str_t));
        if (lcf->broadcasted == NULL) {
            ngx_log_error(NGX_LOG_ERR, cf->log, 0, "failed to allocate memory");
            return (char*) NGX_CONF_ERROR;
        }
    }
    
    // allocate space for new element
    ngx_str_t* el = scast(ngx_str_t*, ngx_array_push(lcf->broadcasted));
    if (el == NULL) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, "failed to allocate memory");
        return (char*) NGX_CONF_ERROR;
    }
    
    // set element value
    ngx_str_t* value = scast(ngx_str_t*, cf->args->elts);
    el->len = value[1].len;
    el->data = value[1].data;
    
    return NGX_CONF_OK;
}


/**
 * Disabled explicit initialization of Glog to workaround the following bugs:
 * - http://code.google.com/p/google-glog/issues/detail?id=83
 * - http://code.google.com/p/google-glog/issues/detail?id=113
 */
#undef INIT_GLOG

/*
 * TODO: add directive to set the log output (stderr or file) 
 */

static ngx_int_t 
ngx_sf1r_init_module(ngx_cycle_t* cycle) {
#ifdef INIT_GLOG
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "init logging system ...");
    google::InitGoogleLogging("ngx_sf1r");
    google::LogToStderr();
#endif
    return NGX_OK;
}


static ngx_int_t 
ngx_sf1r_init_process(ngx_cycle_t* cycle) {
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "init process");
    
    ngx_sf1r_main_conf_t* main_conf = scast(ngx_sf1r_main_conf_t*, 
            ngx_http_cycle_get_module_main_conf(cycle, ngx_sf1r_module));
    
    ngx_sf1r_loc_conf_t** loc_confs = scast(ngx_sf1r_loc_conf_t**,
            main_conf->loc_confs.elts);
    
    for (ngx_uint_t i = 0; i < main_conf->loc_confs.nelts; i++) {
        if (ngx_sf1r_init(loc_confs[i], cycle->log) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "failed to init process");
            return NGX_ERROR;
        }
    }
    
    return NGX_OK;
}


static void
ngx_sf1r_exit_process(ngx_cycle_t* cycle) {
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exit process");
    
    ngx_sf1r_main_conf_t* main_conf = scast(ngx_sf1r_main_conf_t*, 
            ngx_http_cycle_get_module_main_conf(cycle, ngx_sf1r_module));
    
    ngx_sf1r_loc_conf_t** loc_confs = scast(ngx_sf1r_loc_conf_t**,
            main_conf->loc_confs.elts);
    
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "found (%zu) locations", main_conf->loc_confs.nelts);
    for (ngx_uint_t i = 0; i < main_conf->loc_confs.nelts; i++) {
        ngx_sf1r_cleanup(loc_confs[i], cycle->log);
    }
}


static ngx_int_t
ngx_sf1r_init(ngx_sf1r_loc_conf_t* conf, ngx_log_t* log) {
    try {
        ngx_log_error(NGX_LOG_NOTICE, log, 0, "instantiating driver ...");
        
        string host((char*) conf->address.data, conf->address.len);
        if (conf->distributed) {
            ngx_log_error(NGX_LOG_NOTICE, log, 0, "using distributed driver to \"%s\"", conf->address.data);
            
            Sf1DistributedConfig sf1conf;
            sf1conf.initialSize = conf->poolSize;
            sf1conf.resize = conf->poolResize;
            sf1conf.maxSize = conf->poolMaxSize;
            sf1conf.timeout = conf->zkTimeout;
            if (conf->broadcasted != NULL) {
                ngx_str_t* values = scast(ngx_str_t*, conf->broadcasted->elts);
                for (ngx_uint_t i = 0; i < conf->broadcasted->nelts; ++i) {
                    string value(rcast(const char*, values[i].data));
                    sf1conf.addBroadCast(value);
                }
            }
        
            conf->driver = new Sf1DistributedDriver(host, sf1conf);
        } else {
            ngx_log_error(NGX_LOG_NOTICE, log, 0, "using single driver to \"%s\"", conf->address.data);
            
            Sf1Config sf1conf;
            sf1conf.initialSize = conf->poolSize;
            sf1conf.resize = conf->poolResize;
            sf1conf.maxSize = conf->poolMaxSize;
        
            conf->driver = new Sf1Driver(host, sf1conf);
        }
    } catch (std::exception& e) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "%s", e.what());
        return NGX_ERROR;
    }
    
    return NGX_OK;
}


static void 
ngx_sf1r_cleanup(ngx_sf1r_loc_conf_t* conf, ngx_log_t* log) {
    if (conf->driver != NULL) {
        ngx_log_error(NGX_LOG_NOTICE, log, 0, "deleting driver@%p ...", conf->driver);
        try {
            delete (Sf1DriverBase*) conf->driver;
        } catch (std::exception& e) {
            ngx_log_error(NGX_LOG_ERR, log, 0, "%s", e.what());
        }
    }
}
