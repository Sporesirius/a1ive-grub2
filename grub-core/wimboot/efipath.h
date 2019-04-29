#ifndef _EFIPATH_H
#define _EFIPATH_H

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

#include "efi.h"

/**
 * Initialise device path
 *
 * @v name        Variable name
 * @v type        Type
 * @v subtype        Subtype
 */
#define EFI_DEVPATH_INIT( Name, Type, SubType ) {        \
    .type = (Type),                        \
    .subtype = (SubType),                    \
    .length = sizeof (Name) & 0xffff ,            \
    }

/**
 * Initialise device path
 *
 * @v path        Device path
 * @v type        Type
 * @v subtype        Subtype
 * @v len        Length
 */
static inline __attribute__ (( always_inline )) void
efi_devpath_init ( grub_efi_device_path_protocol_t *path, unsigned int type,
           unsigned int subtype, size_t len ) {

    path->type = type;
    path->subtype = subtype;
    path->length = len & 0xffff;
}

/**
 * Initialise device path end
 *
 * @v name        Variable name
 */
#define EFI_DEVPATH_END_INIT( name )                \
    EFI_DEVPATH_INIT ( name, GRUB_EFI_END_DEVICE_PATH_TYPE,        \
               GRUB_EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE )

/**
 * Initialise device path end
 *
 * @v path        Device path
 */
static inline __attribute__ (( always_inline )) void
efi_devpath_end_init ( grub_efi_device_path_protocol_t *path ) {

    efi_devpath_init ( path, GRUB_EFI_END_DEVICE_PATH_TYPE,
               GRUB_EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE, sizeof ( *path ) );
}

extern grub_efi_device_path_protocol_t *
efi_devpath_end ( grub_efi_device_path_protocol_t *path );

#endif /* _EFIPATH_H */
