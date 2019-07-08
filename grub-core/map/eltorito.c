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

#include <grub/dl.h>
#include <grub/file.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/term.h>
#include <grub/types.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/eltorito.h>
#include "map.h"
#include "block.h"

grub_efi_status_t
find_bootable_part (grub_file_t file,
                    grub_efi_uint64_t *boot_start_addr, grub_efi_uint64_t *boot_size
                 /*,grub_efi_uint64_t *data_start_addr, grub_efi_uint64_t *data_size*/)
{
  cdrom_volume_descriptor_t *vol_descriptor = NULL;
  eltorito_catalog_t *tmp_catalog = NULL;
  grub_efi_uintn_t descriptor_size = CD_BLOCK_SIZE;
  grub_efi_uintn_t dbr_imgsize = 2;
  grub_efi_uint16_t dbr_imgsize_buf;
  
  vol_descriptor = grub_malloc (descriptor_size);
  if (vol_descriptor == NULL)
    return GRUB_EFI_NOT_FOUND;

  grub_file_seek (file, CD_BOOT_SECTOR * CD_BLOCK_SIZE);
  grub_file_read (file, vol_descriptor, descriptor_size);
  if (vol_descriptor->unknown.type != CDVOL_TYPE_STANDARD ||
      grub_memcmp (vol_descriptor->boot_record_volume.system_id, CDVOL_ELTORITO_ID,
      sizeof (CDVOL_ELTORITO_ID) - 1) != 0)
    return GRUB_EFI_NOT_FOUND;
  
  tmp_catalog = (eltorito_catalog_t *) vol_descriptor;
  grub_file_seek (file, *((grub_efi_uint32_t*)
                  vol_descriptor->boot_record_volume.elt_catalog) * CD_BLOCK_SIZE);
  grub_file_read (file, tmp_catalog, descriptor_size);
  if (tmp_catalog[0].catalog.indicator != ELTORITO_ID_CATALOG)
    return GRUB_EFI_NOT_FOUND;
  grub_efi_uintn_t i;
  //*data_start_addr = *data_size = 0;
  *boot_start_addr = *boot_size = 0;
  for (i = 0; i < 64; i++)
  {
    /*if (tmp_catalog[i].catalog.indicator == ELTORITO_ID_CATALOG &&
        tmp_catalog[i+1].boot.indicator == ELTORITO_ID_SECTION_BOOTABLE &&
        tmp_catalog[i+1].boot.load_segment == 0x7c0)
    {
      *data_start_addr = tmp_catalog[i+1].boot.lba * CD_BLOCK_SIZE;
      *data_size = tmp_catalog[i+1].boot.sector_count * CD_BLOCK_SIZE;
    }*/
    if (tmp_catalog[i].section.indicator == ELTORITO_ID_SECTION_HEADER_FINAL &&
        tmp_catalog[i].section.platform_id == IS_EFI_SYSTEM_PARTITION &&
        tmp_catalog[i+1].boot.indicator == ELTORITO_ID_SECTION_BOOTABLE)
    {
      *boot_start_addr = tmp_catalog[i+1].boot.lba * CD_BLOCK_SIZE;
      *boot_size = tmp_catalog[i+1].boot.sector_count * CD_BLOCK_SIZE;
      /* sizes of some images are not correct (too small),
         read the dbr of floppy image instead */
      grub_file_seek (file, *boot_start_addr + 0x13);
      grub_file_read (file, &dbr_imgsize_buf, dbr_imgsize);
      if (*boot_size < dbr_imgsize_buf * CD_BLOCK_SIZE )
        *boot_size = dbr_imgsize_buf * CD_BLOCK_SIZE;
      if (*boot_size < 0x1680 * CD_BLOCK_SIZE)
        *boot_size = 0x1680 * CD_BLOCK_SIZE;
      return GRUB_EFI_SUCCESS;
    }
  }
  return GRUB_EFI_NOT_FOUND;
}