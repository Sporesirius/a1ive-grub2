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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "wimboot.h"
#include "vdisk.h"
#include "efi.h"
#include "efipath.h"
#include "efiblock.h"

/** A block I/O device */
struct efi_block {
    /** EFI block I/O protocol */
    grub_efi_block_io_t block;
    /** Device path */
    grub_efi_device_path_protocol_t *path;
    /** Starting LBA */
    uint64_t lba;
    /** Name */
    const char *name;
};

/**
 * Reset block I/O protocol
 *
 * @v this        Block I/O protocol
 * @v extended        Perform extended verification
 * @ret efirc        EFI status code
 */
static grub_efi_status_t
efi_reset_blocks ( grub_efi_block_io_t *this, grub_efi_boolean_t extended ) {
    struct efi_block *block =
        container_of ( this, struct efi_block, block );
    void *retaddr = __builtin_return_address ( 0 );

    DBG ( "EFI %s %sreset -> %p\n",
           block->name, ( extended ? "extended " : "" ), retaddr );
    return 0;
}

/**
 * Read blocks
 *
 * @v this        Block I/O protocol
 * @v media        Media ID
 * @v lba        Starting LBA
 * @v len        Length of data
 * @v data        Data buffer
 * @ret efirc        EFI status code
 */
static grub_efi_status_t
efi_read_blocks ( grub_efi_block_io_t *this, grub_efi_uint32_t media, grub_efi_lba_t lba,
          grub_efi_uintn_t len, void *data ) {
    struct efi_block *block =
        container_of ( this, struct efi_block, block );
    void *retaddr = __builtin_return_address ( 0 );

    DBG ( "EFI %s read media %08x LBA 0x%llx to %p+%lx -> %p\n",
           block->name, media, lba, data, ( ( size_t ) len ), retaddr );
    vdisk_read ( ( lba + block->lba ), ( len / VDISK_SECTOR_SIZE ), data );
    return 0;
}

/**
 * Write blocks
 *
 * @v this        Block I/O protocol
 * @v media        Media ID
 * @v lba        Starting LBA
 * @v len        Length of data
 * @v data        Data buffer
 * @ret efirc        EFI status code
 */
static grub_efi_status_t
efi_write_blocks ( grub_efi_block_io_t *this __unused,
           grub_efi_uint32_t media __unused, grub_efi_lba_t lba __unused,
           grub_efi_uintn_t len __unused, void *data __unused ) {
    struct efi_block *block =
        container_of ( this, struct efi_block, block );
    void *retaddr = __builtin_return_address ( 0 );

    DBG ( "EFI %s write media %08x LBA 0x%llx from %p+%lx -> %p\n",
           block->name, media, lba, data, ( ( size_t ) len ), retaddr );
    return GRUB_EFI_WRITE_PROTECTED;
}

/**
 * Flush block operations
 *
 * @v this        Block I/O protocol
 * @ret efirc        EFI status code
 */
static grub_efi_status_t
efi_flush_blocks ( grub_efi_block_io_t *this ) {
    struct efi_block *block =
        container_of ( this, struct efi_block, block );
    void *retaddr = __builtin_return_address ( 0 );

    DBG ( "EFI %s flush -> %p\n", block->name, retaddr );
    return 0;
}

/** GUID used in vendor device path */
#define EFIBLOCK_GUID {                            \
    0x1322d197, 0x15dc, 0x4a45,                    \
    { 0xa6, 0xa4, 0xfa, 0x57, 0x05, 0x4e, 0xa6, 0x14 }        \
    }

/**
 * Initialise vendor device path
 *
 * @v name        Variable name
 */
#define EFIBLOCK_DEVPATH_VENDOR_INIT( name ) {                \
    .header = EFI_DEVPATH_INIT ( name, HARDWARE_DEVICE_PATH,    \
                     HW_VENDOR_DP ),            \
    .vendor_guid = EFIBLOCK_GUID,                        \
    }

/**
 * Initialise ATA device path
 *
 * @v name        Variable name
 */
#define EFIBLOCK_DEVPATH_ATA_INIT( name ) {                \
    .header = EFI_DEVPATH_INIT ( name, MESSAGING_DEVICE_PATH,    \
                     MSG_ATAPI_DP ),            \
    .primary_secondary = 0,                        \
    .slave_master = 0,                        \
    .lun = 0,                            \
    }

/**
 * Initialise hard disk device path
 *
 * @v name        Variable name
 */
#define EFIBLOCK_DEVPATH_HD_INIT( name ) {                \
    .header = EFI_DEVPATH_INIT ( name, MEDIA_DEVICE_PATH,        \
                     MEDIA_HARDDRIVE_DP ),        \
    .partition_number = 1,                        \
    .partition_start = VDISK_PARTITION_LBA,                \
    .partition_size = VDISK_PARTITION_COUNT,                \
    .partition_signature[0] = ( ( VDISK_MBR_SIGNATURE >> 0 ) & 0xff ),    \
    .partition_signature[1] = ( ( VDISK_MBR_SIGNATURE >> 8 ) & 0xff ),    \
    .partition_signature[2] = ( ( VDISK_MBR_SIGNATURE >> 16 ) & 0xff ),    \
    .partition_signature[3] = ( ( VDISK_MBR_SIGNATURE >> 24 ) & 0xff ),    \
    .partmap_type = MBR_TYPE_PCAT,                    \
    .signature_type = SIGNATURE_TYPE_MBR,                \
    }

/** Virtual disk media */
static grub_efi_block_io_media_t efi_vdisk_media = {
    .media_id = VDISK_MBR_SIGNATURE,
    .media_present = TRUE,
    .logical_partition = FALSE,
    .read_only = TRUE,
    .block_size = VDISK_SECTOR_SIZE,
    .last_block = ( VDISK_COUNT - 1 ),
};

/** Virtual disk device path */
static struct {
    grub_efi_vendor_device_path_t vendor;
    grub_efi_atapi_device_path_t ata;
    grub_efi_device_path_protocol_t end;
} GRUB_PACKED efi_vdisk_path = {
    .vendor = EFIBLOCK_DEVPATH_VENDOR_INIT ( efi_vdisk_path.vendor ),
    .ata = EFIBLOCK_DEVPATH_ATA_INIT ( efi_vdisk_path.ata ),
    .end = EFI_DEVPATH_END_INIT ( efi_vdisk_path.end ),
};

/** Virtual disk device */
static struct efi_block efi_vdisk = {
    .block = {
        .revision = EFI_BLOCK_IO_PROTOCOL_REVISION,
        .media = &efi_vdisk_media,
        .reset = efi_reset_blocks,
        .read_blocks = efi_read_blocks,
        .write_blocks = efi_write_blocks,
        .flush_blocks = efi_flush_blocks,
    },
    .path = &efi_vdisk_path.vendor.header,
    .lba = 0,
    .name = "vdisk",
};

/** Virtual partition media */
static grub_efi_block_io_media_t efi_vpartition_media = {
    .media_id = VDISK_MBR_SIGNATURE,
    .media_present = TRUE,
    .logical_partition = TRUE,
    .read_only = TRUE,
    .block_size = VDISK_SECTOR_SIZE,
    .last_block = ( VDISK_PARTITION_COUNT - 1 ),
};

/** Virtual partition device path */
static struct {
    grub_efi_vendor_device_path_t vendor;
    grub_efi_atapi_device_path_t ata;
    grub_efi_hard_drive_device_path_t hd;
    grub_efi_device_path_protocol_t end;
} GRUB_PACKED efi_vpartition_path = {
    .vendor = EFIBLOCK_DEVPATH_VENDOR_INIT ( efi_vpartition_path.vendor ),
    .ata = EFIBLOCK_DEVPATH_ATA_INIT ( efi_vpartition_path.ata ),
    .hd = EFIBLOCK_DEVPATH_HD_INIT ( efi_vpartition_path.hd ),
    .end = EFI_DEVPATH_END_INIT ( efi_vpartition_path.end ),
};

/** Virtual partition device */
static struct efi_block efi_vpartition = {
    .block = {
        .revision = EFI_BLOCK_IO_PROTOCOL_REVISION,
        .media = &efi_vpartition_media,
        .reset = efi_reset_blocks,
        .read_blocks = efi_read_blocks,
        .write_blocks = efi_write_blocks,
        .flush_blocks = efi_flush_blocks,
    },
    .path = &efi_vpartition_path.vendor.header,
    .lba = VDISK_PARTITION_LBA,
    .name = "vpartition",
};

/**
 * Install block I/O protocols
 *
 * @ret vdisk        New virtual disk handle
 * @ret vpartition    New virtual partition handle
 */
void efi_install ( grub_efi_handle_t *vdisk, grub_efi_handle_t *vpartition ) {
    grub_efi_boot_services_t *bs = grub_efi_system_table->boot_services;
    grub_efi_status_t efirc;

    /* Install virtual disk */
    if ( ( efirc = efi_call_6 ( bs->install_multiple_protocol_interfaces,
        vdisk,
        &efi_block_io_protocol_guid, &efi_vdisk.block,
        &efi_device_path_protocol_guid, efi_vdisk.path,
        NULL ) ) != 0 ) {
        die ( "Could not install disk block I/O protocols: 0x%lx\n",
              ( ( unsigned long ) efirc ) );
    }
    DBG ("...virtual disk installed\n");
    /* Install virtual partition */
    if ( ( efirc = efi_call_6 ( bs->install_multiple_protocol_interfaces,
        vpartition,
        &efi_block_io_protocol_guid, &efi_vpartition.block,
        &efi_device_path_protocol_guid, efi_vpartition.path,
        NULL ) ) != 0 ) {
        die ( "Could not install partition block I/O protocols: 0x%lx\n",
              ( ( unsigned long ) efirc ) );
    }
    DBG ("...virtual partition installed\n");
}

/** Boot image path */
static struct {
    grub_efi_vendor_device_path_t vendor;
    grub_efi_atapi_device_path_t ata;
    grub_efi_hard_drive_device_path_t hd;
    struct {
        grub_efi_device_path_t header;
        grub_efi_char16_t name[ sizeof ( EFI_REMOVABLE_MEDIA_FILE_NAME ) /
                 sizeof ( grub_efi_char16_t ) ];
    } GRUB_PACKED file;
    grub_efi_device_path_protocol_t end;
} GRUB_PACKED efi_bootmgfw_path = {
    .vendor = EFIBLOCK_DEVPATH_VENDOR_INIT ( efi_bootmgfw_path.vendor ),
    .ata = EFIBLOCK_DEVPATH_ATA_INIT ( efi_bootmgfw_path.ata ),
    .hd = EFIBLOCK_DEVPATH_HD_INIT ( efi_bootmgfw_path.hd ),
    .file = {
        .header = EFI_DEVPATH_INIT ( efi_bootmgfw_path.file,
                         MEDIA_DEVICE_PATH,
                         MEDIA_FILEPATH_DP ),
        .name = EFI_REMOVABLE_MEDIA_FILE_NAME,
    },
    .end = EFI_DEVPATH_END_INIT ( efi_bootmgfw_path.end ),
};

/** Boot image path */
grub_efi_device_path_protocol_t *bootmgfw_path = &efi_bootmgfw_path.vendor.header;
