
#ifndef ADFINFO_COMMON_H
#define ADFINFO_COMMON_H

#include <adflib.h>
#include <ctype.h>

void show_hashtable( const uint32_t hashtable[ ADF_HT_SIZE ] );

// replace non-printable with a dot ('.')
static inline char printable( char c )
{
    return ( isalnum( c ) ? c : '.' );
}


#ifndef HAVE_STPNCPY
/* stpncpy() custom implementation (used only where missing) */
char * stpncpy( char * const         dst,
                const char * const   src,
                const size_t         sz );
#endif

#endif
