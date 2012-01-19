/* 
 * File:   ngx_sf1r_ddebug.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 19, 2012, 02:06 PM
 */

#ifndef DDEBUG_H
#define DDEBUG_H

#include <ngx_core.h>
#include <ngx_http.h>

#ifdef DDEBUG

#include <assert.h>

#   define ck(EXPR) assert(EXPR)

#   if (NGX_HAVE_VARIADIC_MACROS)
#       define dd(...) fprintf(stderr, "sf1r *** %s: ", __func__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, " at %s line %d.\n", __FILE__, __LINE__)
#   else

#include <stdarg.h>
#include <stdio.h>

#include <stdarg.h>

static void dd(const char * fmt, ...) {
}

#    endif

#else

#   define ck(EXPR)

#   if (NGX_HAVE_VARIADIC_MACROS)
#       define dd(...)
#       define dd_enter()
#   else

#include <stdarg.h>

static void dd(const char * fmt, ...) {
}

static void dd_enter() {
}

#   endif
#endif

#endif /* DDEBUG_H */
