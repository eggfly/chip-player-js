/* Included at the beginning of library source files, AFTER all other #include
lines. Sets up helpful macros and services used in my source code. Since this
is only "active" in my source code, I don't have to worry about polluting the
global namespace with unprefixed names. */

// $package
#ifndef BLARGG_SOURCE_H
#define BLARGG_SOURCE_H

#ifndef BLARGG_COMMON_H // optimization only
	#include "blargg_common.h"
#endif
#include "blargg_errors.h"
#include "gme_custom_dprintf.h"

#include <string.h> /* memcpy(), memset(), memmove() */
#include <stddef.h> /* offsetof() */

/* The following four macros are for debugging only. Some or all might be
defined to do nothing, depending on the circumstances. Described is what
happens when a particular macro is defined to do something. When defined to
do nothing, the macros do NOT evaluate their argument(s). */

/* If expr is false, prints file and line number, then aborts program. Meant
for checking internal state and consistency. A failed assertion indicates a bug
in MY code.

void assert( bool expr ); */
#include <assert.h>

/* If expr is false, prints file and line number, then aborts program. Meant
for checking caller-supplied parameters and operations that are outside the
control of the module. A failed requirement probably indicates a bug in YOUR
code.

void require( bool expr ); */

#undef  require
#define require( expr ) assert( expr )

/* Use to provide hints to compiler for optimized code layout in situations where we
can almost always expect a conditional to go one way or the other.  Should only be
used in situations where an unexpected branch is truly exceptional though! */
#undef likely
#undef unlikely
#ifdef __GNUC__
    #define likely( x ) __builtin_expect(x, 1)
    #define unlikely( x ) __builtin_expect(x, 0)
#else
    #define likely( x ) (x)
    #define unlikely( x ) (x)
#endif


/* Like printf() except output goes to debugging console/file.
void dprintf( const char format [], ... ); */

// Like printf() except output goes to debug log file. Might be defined to do
// nothing (not even evaluate its arguments).
// void debug_printf( const char* format, ... );


#ifdef CUSTOM_DPRINTF_FUNCTION

static inline void dprintf( const char * fmt, ... )
{
	if (gme_custom_dprintf)
	{
		va_list vl;
		va_start(vl, fmt);
		gme_custom_dprintf(fmt, vl);
		va_end(vl);
	}
}

#else

#ifdef NDEBUG
static inline void blargg_dprintf_( const char [], ... ) { }
#undef  dprintf
#define dprintf (1) ? (void) 0 : blargg_dprintf_
#else
#include <stdarg.h>
#include <stdio.h>
#undef  dprintf
#define dprintf (1) ? (void) 0 : blargg_dprintf_
#ifndef _WIN32
#include <stdio.h>
static inline void blargg_dprintf_( const char * fmt, ... )
{
	char error[512];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf( error, 511, fmt, vl );
	va_end(vl);
	fputs( error, stderr );
}
#undef debug_printf
#define debug_printf (1) ? (void) 0 : blargg_dprintf_


#else
#include <windows.h>
static inline void blargg_dprintf_( const char * fmt, ... )
{
	char error[512];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf_s( error, 511, 511, fmt, vl );
	va_end(vl);
	OutputDebugStringA( error );
}
#endif
#endif

#endif

/* If expr is false, prints file and line number to debug console/log, then
continues execution normally. Meant for flagging potential problems or things
that should be looked into, but that aren't serious problems.

void check( bool expr ); */
#undef  check
#define check( expr ) ((void) 0)

/* If expr yields non-NULL error string, returns it from current function,
otherwise continues normally. */
#undef  RETURN_ERR
#define RETURN_ERR( expr ) \
	do {\
		blargg_err_t blargg_return_err_ = (expr);\
		if ( blargg_return_err_ )\
			return blargg_return_err_;\
	} while ( 0 )

/* If ptr is NULL, returns out-of-memory error, otherwise continues normally. */
#undef  CHECK_ALLOC
#define CHECK_ALLOC( ptr ) \
	do {\
		if ( !(ptr) )\
			return blargg_err_memory;\
	} while ( 0 )

/* The usual min/max functions for built-in types.

template<typename T> T min( T x, T y ) { return x < y ? x : y; }
template<typename T> T max( T x, T y ) { return x > y ? x : y; } */
#define BLARGG_DEF_MIN_MAX( type ) \
	static inline type blargg_min( type x, type y ) { if ( y < x ) x = y; return x; }\
	static inline type blargg_max( type x, type y ) { if ( x < y ) x = y; return x; }

BLARGG_DEF_MIN_MAX( int )
BLARGG_DEF_MIN_MAX( unsigned )
BLARGG_DEF_MIN_MAX( long )
BLARGG_DEF_MIN_MAX( unsigned long )
BLARGG_DEF_MIN_MAX( float )
BLARGG_DEF_MIN_MAX( double )

#undef  min
#define min blargg_min

#undef  max
#define max blargg_max

// typedef unsigned char byte;
typedef unsigned char blargg_byte;
#undef  byte
#define byte blargg_byte

#ifndef BLARGG_EXPORT
	#if defined (_WIN32) && BLARGG_BUILD_DLL
		#define BLARGG_EXPORT __declspec(dllexport)
	#elif defined (__GNUC__)
		// can always set visibility, even when not building DLL
		#define BLARGG_EXPORT __attribute__ ((visibility ("default")))
	#else
		#define BLARGG_EXPORT
	#endif
#endif

#if BLARGG_LEGACY
	#define BLARGG_CHECK_ALLOC CHECK_ALLOC
	#define BLARGG_RETURN_ERR  RETURN_ERR
#endif

// Called after failed operation when overall operation may still complete OK.
// Only used by unit testing framework.
#undef ACK_FAILURE
#define ACK_FAILURE() ((void)0)

/* BLARGG_SOURCE_BEGIN: If defined, #included, allowing redefition of dprintf etc.
and check */
#ifdef BLARGG_SOURCE_BEGIN
	#include BLARGG_SOURCE_BEGIN
#endif

#endif
