/*
 *  common - common utility functions
 *
 *  Copyright (C) 2026 Tomasz Wolak
 *
 *  This file is part of adfinfo, an utility program showing low-level
 *  filesystem metadata of Amiga Disk Files (ADFs).
 *
 *  adflib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  adflib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <adflib.h>

bool change_dir( struct AdfVolume * const  vol,
                 const char * const        dir_path );


unsigned filesize2datablocks( const unsigned fsize,
                              const unsigned blocksize );

unsigned datablocks2extblocks( const unsigned data_blocks );

unsigned filesize2blocks( const unsigned fsize,
                          const unsigned blocksize );

#endif
