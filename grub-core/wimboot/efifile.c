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
 * EFI file system access
 *
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>
#include "wimboot.h"
#include "vdisk.h"
#include "wimpatch.h"
#include "wimfile.h"
#include "cpio.h"
#include "efi.h"
#include "efifile.h"
#include <grub/term.h>

/** bootmgfw.efi path within WIM */
static const wchar_t bootmgfw_path[] = L"\\Windows\\Boot\\EFI\\bootmgfw.efi";

/** bootmgfw.efi file */
struct vdisk_file *bootmgfw;

/**
 * Get architecture-specific boot filename
 *
 * @ret bootarch    Architecture-specific boot filename
 */
static const grub_efi_char16_t * efi_bootarch ( void ) {
    static const grub_efi_char16_t bootarch_full[] = EFI_REMOVABLE_MEDIA_FILE_NAME;
    const grub_efi_char16_t *tmp;
    const grub_efi_char16_t *bootarch = bootarch_full;

    for ( tmp = bootarch_full ; *tmp ; tmp++ ) {
        if ( *tmp == L'\\' )
            bootarch = ( tmp + 1 );
    }
    return bootarch;
}

/**
 * Read from GRUB file
 *
 * @v vfile        Virtual file
 * @v data        Data buffer
 * @v offset        Offset
 * @v len        Length
 */
static void read_grub_file ( struct vdisk_file *vfile, void *data,
                size_t offset, size_t len ) {
    grub_file_t file = vfile->opaque;
    file->offset = offset;
    len = grub_file_read (file, data, len);
}

/**
 * Patch BCD file
 *
 * @v vfile        Virtual file
 * @v data        Data buffer
 * @v offset        Offset
 * @v len        Length
 */
static void efi_patch_bcd ( struct vdisk_file *vfile __unused, void *data,
                size_t offset, size_t len ) {
    static const wchar_t search[] = L".exe";
    static const wchar_t replace[] = L".efi";
    size_t i;

    /* Do nothing if BCD patching is disabled */
    if ( cmdline_rawbcd )
        return;

    /* Patch any occurrences of ".exe" to ".efi".  In the common
     * simple cases, this allows the same BCD file to be used for
     * both BIOS and UEFI systems.
     */
    for ( i = 0 ; ( i + sizeof ( search ) ) < len ; i++ ) {
        if ( wwcscasecmp ( ( data + i ), search ) == 0 ) {
            memcpy ( ( data + i ), replace, sizeof ( replace ) );
            DBG ( "...patched BCD at 0x%lx: \"%ls\" to \"%ls\"\n",
                  ( offset + i ), search, replace );
        }
    }
}

static int isbootmgfw( const char *name)
{
    char bootarch[32];

    if (strcasecmp(name, "bootmgfw.efi") == 0)
        return 1;
    snprintf ( bootarch, sizeof ( bootarch ), "%ls", efi_bootarch() );
    return strcasecmp(name, bootarch) == 0;
}

static int addfile( const char *name, void *data, size_t len,  void ( * read ) ( struct vdisk_file *file,
                                                       void *data,
                                                       size_t offset,
                                                       size_t len ) ) {
    struct vdisk_file *vfile;
    vfile = vdisk_add_file ( name, data, len, read );
        /* Check for special-case files */
    if ( isbootmgfw( name ) ) {
        DBG ( "...found bootmgfw.efi file %s\n", name );
        bootmgfw = vfile;
    } else if ( strcasecmp ( name, "BCD" ) == 0 ) {
        DBG ( "...found BCD\n" );
        vdisk_patch_file ( vfile, efi_patch_bcd );
    } else if ( strlen( name ) > 4 && strcasecmp ( ( name + ( strlen ( name ) - 4 ) ), ".wim" ) == 0 ) {
        DBG ( "...found WIM file %s\n", name );
        vdisk_patch_file ( vfile, patch_wim );
        if ( ( ! bootmgfw ) &&
             ( bootmgfw = wim_add_file ( vfile, cmdline_index,
                                                         bootmgfw_path,
                                                         efi_bootarch() ) ) ) {
            DBG ( "...extracted %ls\n", bootmgfw_path );
        }
    }
    return 0;
}

/**
 * File handler
 *
 * @v name              File name
 * @v data              File data
 * @v len               Length
 * @ret rc              Return status code
 */
int efi_add_file ( const char *name, void *data, size_t len)
{
	return addfile(name, data, len, read_mem_file);
}

void grub_extract ( struct grub_wimboot_context *wimboot_ctx ) {
    int i;

    for (i = 0; i < wimboot_ctx->nfiles; i++) {
        addfile ( wimboot_ctx->components[i].file_name,
                  wimboot_ctx->components[i].file,
                  wimboot_ctx->components[i].size, read_grub_file);
    }

    /* Check that we have a boot file */
    if ( ! bootmgfw ) {
        die ( "FATAL: no %ls or bootmgfw.efi found\n",
              efi_bootarch() );
    }
}