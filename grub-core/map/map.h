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

#ifndef GRUB_EFI_RAMDISK_HEADER
#define GRUB_EFI_RAMDISK_HEADER

#include <grub/types.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/misc.h>
#include "block.h"

typedef struct {
  grub_efi_handle_t handle;
  grub_efi_device_path_protocol_t *dp;
  grub_file_t *file_ptr;
  grub_efi_boolean_t toram;
  grub_efi_uint64_t start_addr;
  grub_efi_uint64_t size;
  grub_efi_block_io_t block_io;
  grub_efi_block_io_media_t media;
} vdisk_private_data_t;

#define CD_BLOCK_SIZE 2048
#define CD_BOOT_SECTOR	17
#define IS_EFI_SYSTEM_PARTITION 239

#define CR(RECORD, TYPE, FIELD) ((TYPE *) ((char *) (RECORD) - (char *) &(((TYPE *) 0)->FIELD)))

#define VDISK_PRIVATE_FROM_BLKIO(a) \
  CR (a, vdisk_private_data_t, block_io)

/*
#define VDISK_PRIVATE_DATA_BLOCKIO2_TO_PARENT(a) \
  ((vdisk_private_data_t *)((grub_efi_char8_t *)(a)-(grub_efi_char8_t *) &(((vdisk_private_data_t *) 0)->block_io2)))
*/
#define GRUB_EFI_VIRTUAL_CD_GUID \
  { 0x5916194e, 0x55e2, 0x45ad, \
    { 0x92, 0x33, 0x00, 0xc0, 0x43, 0x64, 0x1f, 0x82 } \
  }

#define MEDIA_DEVICE_PATH         0x04
#define MEDIA_CDROM_DP            0x02
#define HARDWARE_DEVICE_PATH      0x01
#define HW_VENDOR_DP              0x04

#if defined (__i386__) || defined(__i486__) || defined(__i686__)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME "/EFI/BOOT/BOOTIA32.EFI"
#elif defined (__x86_64__)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME "/EFI/BOOT/BOOTX64.EFI"
#elif defined(__arm__)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME "/EFI/BOOT/BOOTARM.EFI"
#elif defined(__aarch64__)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME "/EFI/BOOT/BOOTAA64.EFI"
#elif defined(__ia64__)
  #define EFI_REMOVABLE_MEDIA_FILE_NAME "/EFI/BOOT/BOOTIA64.EFI"
#else
  #error Unknown Processor Type
#endif

#define GRUB_EFI_COMPONENT_NAME_PROTOCOL_GUID \
  { 0x107a772c, 0xd5e1, 0x11d4, \
    { 0x9a, 0x46, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } \
  }

struct grub_efi_component_name_protocol {
  grub_efi_status_t (*get_driver_name) (struct grub_efi_component_name_protocol *this,
                                        grub_efi_char8_t *language,
                                        grub_efi_char16_t **driver_name);
  grub_efi_status_t (*get_controller_name) (struct grub_efi_component_name_protocol *this,
                                            grub_efi_handle_t controller_handle,
                                            grub_efi_handle_t child_handle,
                                            grub_efi_char8_t *language,
                                            grub_efi_char16_t **controller_name);
  /// A Null-terminated ASCII string that contains one or more
  /// ISO 639-2 language codes. This is the list of language codes
  /// that this protocol supports.
  grub_efi_char8_t *supported_languages;
};
typedef struct grub_efi_component_name_protocol grub_efi_component_name_protocol_t;

#define FAT_DRIVER_NAME "FAT File System Driver"

#define FAT_DRIVER_NAME_LEN 50

//(sizeof(FAT_DRIVER_NAME))

grub_efi_status_t
find_bootable_part (grub_file_t file,
                    grub_efi_uint64_t *boot_start_addr, grub_efi_uint64_t *boot_size
                 /*,grub_efi_uint64_t *data_start_addr, grub_efi_uint64_t *data_size*/);

#endif