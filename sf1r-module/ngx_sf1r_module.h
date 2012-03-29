/* 
 * File:   ngx_sf1r_module.h
 * Author: Paolo D'Apice
 *
 * Created on January 17, 2012, 10:44 AM
 */

#ifndef NGX_SF1R_MODULE_H
#define	NGX_SF1R_MODULE_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


/// Module declaration.
extern ngx_module_t ngx_sf1r_module;


/// Location configuration structure.
typedef struct {
    ngx_str_t address;
    ngx_flag_t enabled;
    ngx_uint_t poolSize;
    ngx_flag_t poolResize;
    ngx_uint_t poolMaxSize;
    ngx_uint_t zkTimeout;
    ngx_array_t* broadcasted; // array of ngx_str_t
    void* driver;
    ngx_flag_t distributed;
} ngx_sf1r_loc_conf_t;


/// Request context structure.
typedef struct {
    ngx_str_t uri;
    ngx_str_t tokens;
    ngx_uint_t body_len;
} ngx_sf1r_ctx_t;

#endif	/* NGX_SF1R_MODULE_H */
