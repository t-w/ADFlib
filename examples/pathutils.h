
#pragma once

#if defined(_WIN32) && !defined(_CYGWIN)
char * dirname( char *  path );
char * basename( char *  path );
#else
#include <libgen.h>
#endif
