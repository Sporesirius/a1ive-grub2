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
#include <grub/env.h>
#include <grub/err.h>
#include <grub/extcmd.h>
#include <grub/file.h>
#include <grub/i18n.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/term.h>
#include <grub/types.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/efi/disk.h>
#include <grub/efi/eltorito.h>
#include "map.h"
#include "block.h"


GRUB_MOD_LICENSE ("GPLv3+");

static const struct grub_arg_option options_map[] = {
  {"mem", 'm', 0, N_("Copy to RAM"), 0, 0},
  {"boot", 'b', 0, N_("Boot from disk"), 0, 0},
  {0, 0, 0, 0, 0, 0}
};

static grub_efi_packed_guid_t ramdisk_guid = GRUB_EFI_VIRTUAL_CD_GUID;

static grub_efi_guid_t dp_guid = GRUB_EFI_DEVICE_PATH_GUID;
static grub_efi_guid_t blockio_guid = GRUB_EFI_BLOCK_IO_GUID;
static grub_efi_guid_t simplefs_guid = GRUB_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

vdisk_private_data_t *private_data = NULL;

static grub_efi_status_t
register_ramdisk (grub_file_t *file_ptr, grub_efi_boolean_t toram)
{
  grub_efi_status_t status;
  grub_efi_device_path_protocol_t *tmp_path;
  grub_efi_boot_services_t *b;
  b = grub_efi_system_table->boot_services;
  
  private_data = grub_malloc (2 * sizeof (vdisk_private_data_t) + 8);
  private_data[0].size = grub_file_size (*file_ptr);

  grub_printf ("Looking for boot data...\n");
  grub_getkey ();
  status = find_bootable_part (*file_ptr,
                               &private_data[1].start_addr, &private_data[1].size);
  if (status != GRUB_EFI_SUCCESS)
  {
    grub_printf ("No bootable partition on the image.\n");
    return status;
  }
  grub_printf ("Boot data found.\n");
  grub_printf ("ID    Start_Addr            Size\n");
  grub_printf ("00 %16ld %16ld\n", private_data[0].start_addr, private_data[0].size);
  grub_printf ("01 %16ld %16ld\n", private_data[1].start_addr, private_data[1].size);
  grub_getkey ();
  if (toram)
  {
    status = efi_call_3 (b->allocate_pool, GRUB_EFI_RESERVED_MEMORY_TYPE,
                         private_data[0].size + 8,
                         (void **)&private_data[0].start_addr);
    if (status != GRUB_EFI_SUCCESS)
    {
      grub_error (GRUB_ERR_OUT_OF_MEMORY, N_("out of memory"));
      return status;
    }
    grub_printf ("Copying to RAM ...\n");
    grub_getkey ();
    grub_env_set ("enable_progress_indicator", "1");
    grub_file_read (*file_ptr, (void *) private_data[0].start_addr, private_data[0].size);
  }
  else
    private_data[0].start_addr = 0;

  private_data[1].start_addr += private_data[0].start_addr;
  /* create dp */
  tmp_path = grub_efi_create_device_node (HARDWARE_DEVICE_PATH ,HW_VENDOR_DP,
                                  sizeof (grub_efi_vendor_device_path_t));
  ((grub_efi_vendor_device_path_t *) tmp_path)->vendor_guid = ramdisk_guid;
  
  private_data[0].dp = grub_efi_append_device_node (NULL, tmp_path);
  grub_free(tmp_path);
  /* struct
  {
    grub_efi_vendor_device_path_t vendor;
    grub_efi_device_path_protocol_t end;
  } GRUB_PACKED cd_path =
  {
    .vendor = { .header = { .type = HARDWARE_DEVICE_PATH,
                            .subtype = HW_VENDOR_DP,
                            .length = sizeof (cd_path.vendor) },
                .vendor_guid = ramdisk_guid, },
    .end = { .type = GRUB_EFI_END_DEVICE_PATH_TYPE,
             .subtype = GRUB_EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE,
             .length = sizeof (cd_path.end) }
  };
  private_data[0].dp =
            grub_efi_duplicate_device_path ((const grub_efi_device_path_t *)&cd_path);*/
  
  tmp_path = grub_efi_create_device_node (MEDIA_DEVICE_PATH, MEDIA_CDROM_DP,
                                  sizeof (grub_efi_cdrom_device_path_t));
  ((grub_efi_cdrom_device_path_t *) tmp_path)->boot_entry = 1;
  ((grub_efi_cdrom_device_path_t *) tmp_path)->partition_start =
        (private_data[1].start_addr - private_data[0].start_addr) / CD_BLOCK_SIZE;
  ((grub_efi_cdrom_device_path_t *) tmp_path)->partition_size = 
        private_data[1].size / CD_BLOCK_SIZE;
  private_data[1].dp = grub_efi_append_device_node (private_data[0].dp, tmp_path);
  grub_free (tmp_path);

  grub_efi_uintn_t i;
  for (i = 0; i < 2; i++)
  {
    private_data[i].handle = NULL;
    private_data[i].file_ptr = file_ptr;
    private_data[i].toram = toram;
    
    /* init block_io */
    grub_memcpy (&(private_data[i].block_io), &block_io_template,
                 sizeof (grub_efi_block_io_t));
    private_data[i].block_io.media = &private_data[i].media;
    private_data[i].media.media_id = 0x1;
    private_data[i].media.removable_media = 1;
    private_data[i].media.media_present = 1;
    private_data[i].media.logical_partition = 1;
    private_data[i].media.read_only = 1;
    private_data[i].media.write_caching = 0;
    private_data[i].media.io_align = CD_BLOCK_SIZE;
    private_data[i].media.block_size = CD_BLOCK_SIZE;
    private_data[i].media.last_block = (private_data[i].size + CD_BLOCK_SIZE - 1) /
                                        CD_BLOCK_SIZE - 1;
  }
  private_data[1].media.logical_partition = 0;
  grub_efi_print_device_path (private_data[0].dp);
  grub_printf ("\n");
  grub_efi_print_device_path (private_data[1].dp);
  grub_printf ("\n");

  grub_printf ("Installing BLOCK_IO protocol for EFI FILESYSTEM ...\n");
  grub_getkey ();
  status = efi_call_6 (b->install_multiple_protocol_interfaces,
                       &private_data[1].handle,
                       &dp_guid, &private_data[1].dp,
                       &blockio_guid, &private_data[1].block_io, NULL);

  
  
  {
    void *simplefs_protocol = NULL;
    efi_call_3 (b->handle_protocol,
                private_data[1].handle, &simplefs_guid, &simplefs_protocol);
    if (simplefs_protocol)
      goto exit_handle;
    /* looking for FAT handle */
    grub_printf ("Looking for FAT handle ...\n");
    grub_getkey ();
    grub_efi_guid_t component_name_guid = GRUB_EFI_COMPONENT_NAME_PROTOCOL_GUID;
    grub_efi_handle_t fat_driver_handle = NULL;
    grub_efi_handle_t *buffer;
    grub_efi_uintn_t buffer_count;
    grub_efi_component_name_protocol_t *component_name_protocol = NULL;
    grub_efi_char16_t *driver_name;
    char driver_name_str[FAT_DRIVER_NAME_LEN] = "";
    status = efi_call_5 (b->locate_handle_buffer, GRUB_EFI_BY_PROTOCOL,
                       &component_name_guid, NULL, &buffer_count, &buffer);
    if (status != GRUB_EFI_SUCCESS)
    {
      grub_printf ("COMPONENT_NAME_PROTOCOL not found.\n");
      //return status;
    }
    grub_printf ("%ld handle(s) found.\n", buffer_count);
  
    for (i = 0; i < buffer_count; i++)
    {
      efi_call_3 (b->handle_protocol,
                  buffer[i], &component_name_guid, (void **)&component_name_protocol);
      efi_call_3 (component_name_protocol->get_driver_name,
                  component_name_protocol, "eng", &driver_name);
      int j;
      for (j = 0; j < FAT_DRIVER_NAME_LEN; j++)
      {
        driver_name_str[i] = driver_name[i];
        if (driver_name[i] == 0)
          break;
      }
      grub_printf ("Driver: %s\n", driver_name_str);
      if (grub_strstr (driver_name_str, FAT_DRIVER_NAME))
      {
        fat_driver_handle = buffer[i];
        grub_printf ("FAT driver handle: %p\n", fat_driver_handle);
        break;
      }
      if (fat_driver_handle)
      {
        status = efi_call_4 (b->connect_controller,
                             private_data[1].handle, &fat_driver_handle, NULL, 1);
        if (status != GRUB_EFI_SUCCESS)
          grub_printf ("Failed to call b->connect_controller.\n");
      }
      else
        grub_printf ("FAT driver not found.\n");
    }
  exit_handle:
    grub_printf ("Use SIMPLE_FILE_SYSTEM_PROTOCOL.\n");
  }

  grub_printf ("Installing BLOCK_IO protocol for CD ...\n");
  grub_getkey ();
  status = efi_call_6 (b->install_multiple_protocol_interfaces,
                       &private_data[0].handle,
                       &dp_guid, &private_data[0].dp,
                       &blockio_guid, &private_data[0].block_io, NULL);

  grub_printf ("disk handle: %p\n", private_data[0].handle);
  grub_getkey ();
  status = efi_call_4 (b->connect_controller,
                        private_data[0].handle, NULL, NULL, 1);
  return status;
}

static grub_efi_status_t boot_disk (grub_efi_device_path_protocol_t *dp)
{
  grub_efi_status_t status;
  grub_efi_boot_services_t *b;
  b = grub_efi_system_table->boot_services;
  grub_efi_uintn_t i;
  grub_efi_uintn_t buffer_count = 0;
  grub_efi_handle_t *buffer = NULL;
  grub_efi_handle_t *boot_handle = NULL;
  grub_efi_handle_t device_handle = NULL;
  grub_efi_device_path_protocol_t *dp_cmp;
  grub_efi_device_path_protocol_t *boot_dp;
  
  grub_printf ("Boot Device DevicePath: ");
  grub_efi_print_device_path (dp);
  grub_printf ("\n");
  
  status = efi_call_5 (b->locate_handle_buffer, GRUB_EFI_BY_PROTOCOL,
                       &simplefs_guid, NULL, &buffer_count, &buffer);
  if (status != GRUB_EFI_SUCCESS)
  {
    grub_printf ("SIMPLE_FILE_SYSTEM_PROTOCOL not found.\n");
    return status;
  }
  grub_printf ("%ld handle(s) found.\n", buffer_count);
  
  for (i = 0; i < buffer_count; i++)
  {
    /* convert handle to dp */
    dp_cmp = grub_efi_get_device_path (buffer[i]);
    /* test parent node */
    if (((grub_efi_vendor_device_path_t *)dp_cmp)->header.type != HARDWARE_DEVICE_PATH ||
        ((grub_efi_vendor_device_path_t *)dp_cmp)->header.subtype != HW_VENDOR_DP ||
        !grub_memcmp (&((grub_efi_vendor_device_path_t *)dp_cmp)->vendor_guid, &ramdisk_guid, sizeof (grub_efi_packed_guid_t)))
      continue;
    /* test child node */
    dp_cmp = GRUB_EFI_NEXT_DEVICE_PATH (dp_cmp);
    if (((grub_efi_cdrom_device_path_t *)dp_cmp)->header.type != MEDIA_DEVICE_PATH ||
        ((grub_efi_cdrom_device_path_t *)dp_cmp)->header.subtype != MEDIA_CDROM_DP ||
        ((grub_efi_cdrom_device_path_t *)dp_cmp)->boot_entry != 1)
      continue;
    device_handle = buffer[i];
    break;
  }
  if (!device_handle)
    return GRUB_EFI_LOAD_ERROR;
  grub_printf ("handle index: %ld", i);
  grub_printf ("Partition DevicePath: ");
  grub_efi_print_device_path (dp);
  grub_printf ("\n");

  /* get full device path for bootx64.efi */
  boot_dp = grub_efi_file_device_path (dp_cmp, EFI_REMOVABLE_MEDIA_FILE_NAME);

  grub_printf ("Boot file DevicePath: ");
  grub_efi_print_device_path (boot_dp);
  grub_printf ("\n");

  status = efi_call_6 (b->load_image, 0, grub_efi_image_handle,
                       boot_dp, NULL, 0, boot_handle);
  if (status != GRUB_EFI_SUCCESS)
  {
    grub_printf ("LoadImage failed.\n");
    return status;
  }
  status = efi_call_2 (b->start_image, boot_handle, NULL);
  if (status != GRUB_EFI_SUCCESS)
  {
    grub_printf ("StartImage failed.\n");
    return status;
  }
  return GRUB_EFI_SUCCESS;
}

static grub_err_t
grub_cmd_map (grub_extcmd_context_t ctxt,
        int argc,
        char **args)
{
  struct grub_arg_list *state = ctxt->state;
  grub_file_t file = 0;
  grub_ssize_t size;
  grub_efi_status_t status;

  if (argc != 1)
    goto fail;

  file = grub_file_open (args[0], GRUB_FILE_TYPE_LOOPBACK);
  if (! file)
    goto fail;
  size = grub_file_size (file);
  if (!size)
    {
      grub_error (GRUB_ERR_BAD_OS, N_("premature end of file %s"),
		  args[0]);
      goto fail;
    }

  grub_printf ("register ramdisk\n");
  grub_getkey ();
  if (state[0].set)
    status = register_ramdisk (&file, 1);
  else
    status = register_ramdisk (&file, 0);
  
  if (status != GRUB_EFI_SUCCESS)
    {
      grub_error (GRUB_ERR_BAD_OS, "cannot create ramdisk");
      goto fail;
    }
  else
    grub_printf ("ramdisk created\n");
  if (state[1].set)
  {
    status = boot_disk (private_data[0].dp);
    if (status != GRUB_EFI_SUCCESS)
    {
      grub_printf ("Boot failed.\n");
      goto fail;
    }
  }
  return 0;
fail:
  if (file)
    grub_file_close (file);
  return 1;
}

static grub_extcmd_t cmd_map;

GRUB_MOD_INIT(map)
{
  cmd_map = grub_register_extcmd ("map", grub_cmd_map, 0, 
                  N_("[--mem] [--boot] FILE"),
                  N_("Create Virtual Disk."), options_map);
}

GRUB_MOD_FINI(map)
{
  grub_unregister_extcmd (cmd_map);
}