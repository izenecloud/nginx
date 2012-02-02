/* 
 * File:   ngx_sf1r_utils.h
 * Author: Paolo D'Apice
 *
 * Created on January 17, 2012, 11:27 AM
 */

#ifndef NGX_SF1R_UTILS_H
#define	NGX_SF1R_UTILS_H


#define scast(T,V)  static_cast< T >( V )
#define ccast(T,V)  const_cast< T >( V )
#define rcast(T,V)  reinterpret_cast< T >( V )

#define TEXT_PLAIN              "text/plain"
#define TEXT_HTML               "text/html"
#define APPLICATION_JSON        "application/json"
#define FLAG_DISABLED           0
#define FLAG_ENABLED            1

#define SF1_DEFAULT_ADDR        "localhost"
#define SF1_DEFAULT_PORT        18181
#define SF1_DEFAULT_POOL_SIZE   5
#define SF1_DEFAULT_POOL_MAXSIZE 25

#endif	/* NGX_SF1R_UTILS_H */
