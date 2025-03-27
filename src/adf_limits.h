
#ifndef ADF_LIMITS_H
#define ADF_LIMITS_H

#include <limits.h>


/**********************************
 * Device
 **********************************/

#define ADF_DEV_BLOCK_SIZE 512

/*
   The primary limitation of max dev size comes from using stdio for accessing
   devices (ie. fseek uses 'long int' as offset).

   Some more modern systems allow to mitigate this limitation with fseeko,
   but, for instance, VBCC does not have that, so using it would limit portability.

   Note that this depends on the platform: 64bit systems (with 64bit long!)
   will be able to handle very large devices, while 32bit will be limited to 2GiB
   (max value of long int meaning int32_t).
*/

#if LONG_MAX < UINT_MAX
// long int is 32bit -> 2GiB limit
#define ADF_DEV_SIZE_MAX_BLOCKS  ( LONG_MAX / 512 + 1 )

#else
/* long int is 64bit -> the limit is the type of size (uint32_t) in AdfDevice.

   To investigate if can be improved, eg. volume size could also be expressed
   in 512-byte blocks.

   One of the problems to consider is that many existing devices (HDFs) have sizes
   not rounded to 512-byte blocks (like 2000000 or 5000000).
*/
#define ADF_DEV_SIZE_MAX_BLOCKS  ( UINT_MAX / 512 + 1 )

#endif


/**********************************
 * Volume
 **********************************/

/* size of the volume is expressed in 512-byte blocks so, theoreticaly, it could
   be bigger than device above... However, Amiga limitations seems to be 2GiB,
   so INT_MAX */
#define ADF_VOL_SIZE_MAX INT_MAX


/**********************************
 * File
 **********************************/

#define ADF_FILE_SIZE_MAX INT_MAX

#endif
