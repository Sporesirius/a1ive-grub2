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
 * EFI GUIDs
 *
 */

#include "wimboot.h"
#include "efi.h"
#include <grub/efi/api.h>
#include <grub/efi/graphics_output.h>

/** Block I/O protocol GUID */
grub_efi_guid_t efi_block_io_protocol_guid
    = GRUB_EFI_BLOCK_IO_GUID;

/** Device path protocol GUID */
grub_efi_guid_t efi_device_path_protocol_guid
    = GRUB_EFI_DEVICE_PATH_GUID;

/** Graphics output protocol GUID */
grub_efi_guid_t efi_graphics_output_protocol_guid
    = GRUB_EFI_GOP_GUID;

/** Loaded image protocol GUID */
grub_efi_guid_t efi_loaded_image_protocol_guid
    = GRUB_EFI_LOADED_IMAGE_PROTOCOL_GUID;
