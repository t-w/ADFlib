/* Win32/adf_nativ.h
 *
 * Win32 specific drive access routines for ADFLib
 * Copyright 1999 by Dan Sutherland <dan@chromerhino.demon.co.uk>
 *
 *  This file is part of ADFLib.
 *
 *  ADFLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ADFLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef ADF_NATIV_H
#define ADF_NATIV_H

#include "adf_dev.h"

#define NATIVE_FILE  8001

#ifndef BOOL
#define BOOL int
#endif

#ifndef RETCODE
#define RETCODE long
#endif

struct nativeDevice{
	void *hDrv;
};

struct nativeFunctions {
	RETCODE (*adfInitDevice)(struct AdfDevice *, char*, BOOL);
	RETCODE (*adfNativeReadSector)(struct AdfDevice *, long, int, unsigned char*);
	RETCODE (*adfNativeWriteSector)(struct AdfDevice *, long, int, unsigned char*);
	BOOL (*adfIsDevNative)(char*);
	RETCODE (*adfReleaseDevice)(struct AdfDevice * dev);
};

void adfInitNativeFct();

RETCODE Win32ReadSector ( struct AdfDevice * dev,
                          long               n,
                          int                size,
                          unsigned char *    buf );

RETCODE Win32WriteSector ( struct AdfDevice * dev,
                           long               n,
                           int                size,
                           unsigned char *    buf );

RETCODE Win32InitDevice ( struct AdfDevice * dev,
                          char *             name,
                          BOOL               ro );

RETCODE Win32ReleaseDevice ( struct AdfDevice * dev );
BOOL Win32IsDevNative(char*);

#endif /* ndef ADF_NATIV_H */
