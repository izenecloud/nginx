/* 
 * File:   ngx_sf1r_handler.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 17, 2012, 11:23 AM
 */

extern "C" {
#include "ngx_sf1r_handler.h"
#include "ngx_sf1r_module.h"
#include "ngx_sf1r_utils.h"
}


static ngx_str_t TEST = ngx_string("test");


ngx_int_t
ngx_sf1r_handler(ngx_http_request_t* request) {
    ngx_log_error(NGX_LOG_NOTICE, request->connection->log, 0, "sf1r handler");
    //ngx_sf1r_loc_conf_t* conf = CAST(ngx_sf1r_loc_conf_t*, ngx_http_get_module_loc_conf(request, ngx_sf1r_module));

    // response to 'GET' and 'POST' requests only
    if (!(request->method & (NGX_HTTP_GET|NGX_HTTP_POST))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    /* do actual processing */
    
    /* set response header */
    
    // set the status line
    request->headers_out.status = NGX_HTTP_OK;
    request->headers_out.content_length_n = TEST.len;
    request->headers_out.content_type_len = sizeof(APPLICATION_JSON) - 1;
    request->headers_out.content_type.len = sizeof(APPLICATION_JSON) - 1;
    request->headers_out.content_type.data = (u_char*) APPLICATION_JSON;
    
    /* set response body */
    
    // allocate a buffer
    ngx_buf_t* buffer = scast(ngx_buf_t*, ngx_pcalloc(request->pool, sizeof(ngx_buf_t)));
    if (buffer == NULL) {
        ngx_log_error(NGX_LOG_ERR, request->connection->log, 0, "Failed to allocate response buffer.");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    
    // adjust the pointers of the buffer
    buffer->pos = TEST.data;                              // first position of the data
    buffer->last = TEST.data + TEST.len;      // last position of the data
    buffer->memory = 1;                              // this buffer is in memory (read-only)
    buffer->last_buf = 1;                            // this is the last buffer in the buffer chain

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
