#ifndef _EFIBLOCK_H
#define _EFIBLOCK_H

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
 * EFI block device
 *
 */

#include "efi.h"

#define EFI_BLOCK_IO_PROTOCOL_REVISION  0x00010000

extern void efi_install ( grub_efi_handle_t *vdisk, grub_efi_handle_t *vpartition );

extern grub_efi_device_path_protocol_t *bootmgfw_path;

#endif /* _EFIBLOCK_H */
