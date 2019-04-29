/*
 * Copyright (C) 2014 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/**
 * @file
 *
 * EFI device paths
 *
 */

#include "wimboot.h"
#include "efi.h"
#include "efipath.h"

/**
 * Find end of device path
 *
 * @v path        Path to device
 * @ret path_end    End of device path
 */
grub_efi_device_path_protocol_t * efi_devpath_end ( grub_efi_device_path_protocol_t *path ) {

    while ( path->type != GRUB_EFI_END_DEVICE_PATH_TYPE ) {
        path = ( ( ( void * ) path ) +
             /* There's this amazing new-fangled thing known as
              * a UINT16, but who wants to use one of those? */
             path->length );
    }
    return path;
}
