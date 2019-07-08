/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2019  Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_EFI_VDISK_BLOCK_HEADER
#define GRUB_EFI_VDISK_BLOCK_HEADER

#include <grub/types.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/misc.h>

#define EFI_BLOCK_IO_PROTOCOL_REVISION  0x00010000
/*
#define GRUB_EFI_BLOCK_IO2_GUID \
  { 0xa77b2472, 0xe282, 0x4e9f, \
      { 0xa2, 0x45, 0xc2, 0xc0, 0xe2, 0x7b, 0xbc, 0xc1 } \
  }

typedef struct
{
 grub_efi_event_t event;
 grub_efi_status_t transaction_status;
} grub_efi_block_io2_token_t;


struct grub_efi_block_io
{
  grub_efi_uint64_t revision;
  grub_efi_block_io_media_t *media;
  grub_efi_status_t (*reset) (struct grub_efi_block_io *this,
			      grub_efi_boolean_t extended_verification);
  grub_efi_status_t (*read_blocks) (struct grub_efi_block_io *this,
				    grub_efi_uint32_t media_id,
				    grub_efi_lba_t lba,
				    grub_efi_uintn_t buffer_size,
				    void *buffer);
  grub_efi_status_t (*write_blocks) (struct grub_efi_block_io *this,
				     grub_efi_uint32_t media_id,
				     grub_efi_lba_t lba,
				     grub_efi_uintn_t buffer_size,
				     void *buffer);
  grub_efi_status_t (*flush_blocks) (struct grub_efi_block_io *this);
};
typedef struct grub_efi_block_io grub_efi_block_io_t;

struct grub_efi_block_io2
{
  grub_efi_block_io_media_t *media;
  grub_efi_status_t (*reset_ex) (struct grub_efi_block_io2 *this,
			      grub_efi_boolean_t extended_verification);
  grub_efi_status_t (*read_blocks_ex) (struct grub_efi_block_io2 *this,
				    grub_efi_uint32_t media_id,
				    grub_efi_lba_t lba,
                    grub_efi_block_io2_token_t *token,
				    grub_efi_uintn_t buffer_size,
				    void *buffer);
  grub_efi_status_t (*write_blocks_ex) (struct grub_efi_block_io2 *this,
				     grub_efi_uint32_t media_id,
				     grub_efi_lba_t lba,
                     grub_efi_block_io2_token_t *token,
				     grub_efi_uintn_t buffer_size,
				     void *buffer);
  grub_efi_status_t (*flush_blocks_ex) (struct grub_efi_block_io2 *this,
                                        grub_efi_block_io2_token_t *token);
};
typedef struct grub_efi_block_io2 grub_efi_block_io2_t;
*/
extern grub_efi_block_io_t block_io_template;
/*extern grub_efi_block_io2_t block_io2_template;*/

#endif