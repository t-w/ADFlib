/*
 *  adfinfo_volume - volume metadata code
 *
 *  Copyright (C) 2023-2025 Tomasz Wolak
 *
 *  This file is part of adfinfo, an utility program showing low-level
 *  filesystem metadata of Amiga Disk Files (ADFs).
 *
 *  adfinfo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  adfinfo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef ADFINFO_VOLUME_H
#define ADFINFO_VOLUME_H

#include <adflib.h>
#include <adf_blk.h>
#include <stdbool.h>

void show_volume_metadata( struct AdfVolume * const  vol );

void show_bootblock( const struct AdfBootBlock * const  bblock,
                     bool                               show_data );

void show_bootblock_data( const struct AdfBootBlock * const  bblock );

void show_rootblock( const struct AdfRootBlock * const  rblock );

#endif
