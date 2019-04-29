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
 * EFI boot manager invocation
 *
 */

#include <stdio.h>
#include <string.h>
#include "wimboot.h"
#include "vdisk.h"
#include "pause.h"
#include "efi.h"
#include "efipath.h"
#include "efiboot.h"

#if 0
/** Original OpenProtocol() method */
grub_efi_status_t (*orig_open_protocol) (grub_efi_handle_t handle,
            grub_efi_guid_t *protocol,
            void **protocol_interface,
            grub_efi_handle_t agent_handle,
            grub_efi_handle_t controller_handle,
            grub_efi_uint32_t attributes);

/**
 * Intercept OpenProtocol()
 *
 * @v handle        EFI handle
 * @v protocol        Protocol GUID
 * @v interface        Opened interface
 * @v agent_handle    Agent handle
 * @v controller_handle    Controller handle
 * @v attributes    Attributes
 * @ret efirc        EFI status code
 */
static grub_efi_status_t
efi_open_protocol_wrapper ( grub_efi_handle_t handle, grub_efi_guid_t *protocol,
                void **interface, grub_efi_handle_t agent_handle,
                grub_efi_handle_t controller_handle, grub_efi_uint32_t attributes ) {
    static unsigned int count;
    grub_efi_status_t efirc;

    /* Open the protocol */
    if ( ( efirc = efi_call_6 (orig_open_protocol, handle, protocol, interface,
                        agent_handle, controller_handle,
                        attributes ) ) != 0 ) {
        return efirc;
    }

    /* Block first attempt by bootmgfw.efi to open
     * EFI_GRAPHICS_OUTPUT_PROTOCOL.  This forces error messages
     * to be displayed in text mode (thereby avoiding the totally
     * blank error screen if the fonts are missing).  We must
     * allow subsequent attempts to succeed, otherwise the OS will
     * fail to boot.
     */
    if ( ( memcmp ( protocol, &efi_graphics_output_protocol_guid,
            sizeof ( *protocol ) ) == 0 ) &&
         ( count++ == 0 ) && ( ! cmdline_gui ) ) {
        DBG ( "Forcing text mode output\n" );
        return GRUB_EFI_INVALID_PARAMETER;
    }

    return 0;
}
#endif
/**
 * Boot from EFI device
 *
 * @v file        Virtual file
 * @v path        Device path
 * @v device        Device handle
 */
void efi_boot ( struct vdisk_file *file, grub_efi_device_path_protocol_t *path,
        grub_efi_handle_t device ) {
    grub_efi_boot_services_t *bs = grub_efi_system_table->boot_services;
    grub_efi_physical_address_t phys;
    void *data;
    unsigned int pages;
    grub_efi_handle_t handle;
    grub_efi_status_t efirc;
    grub_efi_loaded_image_t *loaded_image;

    /* Allocate memory */
    pages = ( ( file->len + PAGE_SIZE - 1 ) / PAGE_SIZE );
    if ( ( efirc = efi_call_4 (bs->allocate_pages, GRUB_EFI_ALLOCATE_ANY_PAGES,
                       GRUB_EFI_BOOT_SERVICES_DATA, pages,
                       &phys ) ) != 0 ) {
        die ( "Could not allocate %d pages: 0x%lx\n",
              pages, ( ( unsigned long ) efirc ) );
    }
    data = ( ( void * ) ( intptr_t ) phys );


    /* Read image */
    file->read ( file, data, 0, file->len );
    DBG ( "Read %s\n", file->name );

    /* Load image */
    if ( ( efirc = efi_call_6 (bs->load_image, FALSE, grub_efi_image_handle, path, data,
                       file->len, &handle ) ) != 0 ) {
        die ( "Could not load %s: 0x%lx\n",
              file->name, ( ( unsigned long ) efirc ) );
    }
    DBG ( "Loaded %s\n", file->name );

    /* Get loaded image protocol */
    loaded_image = grub_efi_get_loaded_image (handle);
    if ( ! loaded_image ){
        die ( "Could not get loaded image protocol for %s\n",
              file->name );
    }

    /* Force correct device handle */
    if ( loaded_image->device_handle != device ) {
        DBG ( "Forcing correct DeviceHandle (%p->%p)\n",
              loaded_image->device_handle, device );
        loaded_image->device_handle = device;
    }

    /* Intercept calls to OpenProtocol() */
    /*orig_open_protocol =
        loaded_image->system_table->boot_services->open_protocol;
    loaded_image->system_table->boot_services->open_protocol =
        efi_open_protocol_wrapper;*/
        
    /* Start image */
    DBG ( "Start image\n");
    if ( cmdline_pause )
        pause();
    if ( ( efirc = efi_call_3 ( bs->start_image, handle, NULL, NULL ) ) != 0 ) {
        die ( "Could not start %s: 0x%lx\n",
              file->name, ( ( unsigned long ) efirc ) );
    }

    die ( "%s returned\n", file->name );
}
