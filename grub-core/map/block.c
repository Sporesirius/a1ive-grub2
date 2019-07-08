/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2019  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <grub/dl.h>

#include <grub/file.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/types.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/disk.h>
#include "map.h"
#include "block.h"

static grub_efi_status_t
efi_reset_blocks (grub_efi_block_io_t *this __attribute__ (( unused )),
                  grub_efi_boolean_t extended __attribute__ (( unused )))
{
  return GRUB_EFI_SUCCESS;
}

static grub_efi_status_t
efi_read_blocks (grub_efi_block_io_t *this, grub_efi_uint32_t media, grub_efi_lba_t lba,
                 grub_efi_uintn_t len, void *data)
{
  vdisk_private_data_t *private_data;
  grub_efi_uintn_t num_of_blks;

  if (data == NULL)
    return GRUB_EFI_INVALID_PARAMETER;
  if (len == 0)
    return GRUB_EFI_SUCCESS;
  private_data = VDISK_PRIVATE_FROM_BLKIO(this);
  
  if (media != private_data->media.media_id)
    return GRUB_EFI_MEDIA_CHANGED;

  if ((len % private_data->media.block_size) != 0)
    return GRUB_EFI_BAD_BUFFER_SIZE;

  if (lba > private_data->media.last_block)
    return GRUB_EFI_INVALID_PARAMETER;

  num_of_blks = len / private_data->media.block_size;
  if ((lba + num_of_blks - 1) > private_data->media.last_block)
    return GRUB_EFI_INVALID_PARAMETER;
  if (private_data->toram)
    grub_memcpy (data, (void *)(grub_efi_uintn_t)(private_data->start_addr +
            lba * private_data->media.block_size), len);
  else
  {
    grub_file_seek (*(private_data->file_ptr), private_data->start_addr +
                    lba * private_data->media.block_size);
    grub_file_read (*(private_data->file_ptr), data, len);
  }
  return GRUB_EFI_SUCCESS;
}

static grub_efi_status_t
efi_write_blocks (grub_efi_block_io_t *this, grub_efi_uint32_t media, grub_efi_lba_t lba,
                 grub_efi_uintn_t len, void *data)
{
  vdisk_private_data_t *private_data;
  grub_efi_uintn_t num_of_blks;

  if (data == NULL)
    return GRUB_EFI_INVALID_PARAMETER;
  if (len == 0)
    return GRUB_EFI_SUCCESS;
  private_data = VDISK_PRIVATE_FROM_BLKIO(this);
  
  if (media != private_data->media.media_id)
    return GRUB_EFI_MEDIA_CHANGED;

  if (private_data->media.read_only)
    return GRUB_EFI_WRITE_PROTECTED;

  if ((len % private_data->media.block_size) != 0)
    return GRUB_EFI_BAD_BUFFER_SIZE;

  if (lba > private_data->media.last_block)
    return GRUB_EFI_INVALID_PARAMETER;

  num_of_blks = len / private_data->media.block_size;
  if ((lba + num_of_blks - 1) > private_data->media.last_block)
    return GRUB_EFI_INVALID_PARAMETER;
  if (private_data->toram)
    grub_memcpy ((void *)(grub_efi_uintn_t)(private_data->start_addr +
            lba * private_data->media.block_size), data, len);
  else
    return GRUB_EFI_WRITE_PROTECTED;
  return GRUB_EFI_SUCCESS;
}

static grub_efi_status_t
efi_flush_blocks (grub_efi_block_io_t *this __attribute__ (( unused )))
{
  return GRUB_EFI_SUCCESS;
}
/*
static grub_efi_status_t
efi_reset2_blocks (grub_efi_block_io2_t *this __attribute__ (( unused )),
                  grub_efi_boolean_t extended __attribute__ (( unused )))
{
  return GRUB_EFI_SUCCESS;
}

static grub_efi_status_t
efi_read2_blocks (grub_efi_block_io2_t *this, grub_efi_uint32_t media, grub_efi_lba_t lba,
                 grub_efi_block_io2_token_t *token, grub_efi_uintn_t len, void *data)
{
  vdisk_private_data_t *private_data;
  grub_efi_status_t status;

  private_data = VDISK_PRIVATE_DATA_BLOCKIO2_TO_PARENT(this);
  
  status = efi_read_blocks (&private_data->block_io, media, lba, len, data);
  if (status != GRUB_EFI_SUCCESS)
    return status;

  if ((token != NULL) && (token->event != NULL))
  {
    grub_efi_boot_services_t *b;
    b = grub_efi_system_table->boot_services;
    token->transaction_status = GRUB_EFI_SUCCESS;
    efi_call_1 (b->signal_event, token->event);
  }
  return GRUB_EFI_SUCCESS;
}

static grub_efi_status_t
efi_write2_blocks (grub_efi_block_io2_t *this, grub_efi_uint32_t media, grub_efi_lba_t lba,
                 grub_efi_block_io2_token_t *token, grub_efi_uintn_t len, void *data)
{
  vdisk_private_data_t *private_data;
  grub_efi_status_t status;

  private_data = VDISK_PRIVATE_DATA_BLOCKIO2_TO_PARENT(this);
  
  status = efi_write_blocks (&private_data->block_io, media, lba, len, data);
  if (status != GRUB_EFI_SUCCESS)
    return status;

  if ((token != NULL) && (token->event != NULL))
  {
    grub_efi_boot_services_t *b;
    b = grub_efi_system_table->boot_services;
    token->transaction_status = GRUB_EFI_SUCCESS;
    efi_call_1 (b->signal_event, token->event);
  }
  return GRUB_EFI_SUCCESS;
}

static grub_efi_status_t
efi_flush2_blocks (grub_efi_block_io2_t *this, grub_efi_block_io2_token_t *token)
{
  vdisk_private_data_t *private_data;

  private_data = VDISK_PRIVATE_DATA_BLOCKIO2_TO_PARENT(this);
  
  if (private_data->media.read_only)
    return GRUB_EFI_WRITE_PROTECTED;

  if ((token != NULL) && (token->event != NULL))
  {
    grub_efi_boot_services_t *b;
    b = grub_efi_system_table->boot_services;
    token->transaction_status = GRUB_EFI_SUCCESS;
    efi_call_1 (b->signal_event, token->event);
  }
  return GRUB_EFI_SUCCESS;
}
*/
grub_efi_block_io_t block_io_template = {
  EFI_BLOCK_IO_PROTOCOL_REVISION,
  (grub_efi_block_io_media_t *) 0,
  efi_reset_blocks,
  efi_read_blocks,
  efi_write_blocks,
  efi_flush_blocks
};
/*
grub_efi_block_io2_t block_io2_template = {
  (grub_efi_block_io_media_t *) 0,
  efi_reset2_blocks,
  efi_read2_blocks,
  efi_write2_blocks,
  efi_flush2_blocks
};
*/