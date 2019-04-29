#ifndef _WIMBOOT_H
#define _WIMBOOT_H

/*
 * Copyright (C) 2012 Michael Brown <mbrown@fensystems.co.uk>.
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
 * WIM boot loader
 *
 */

#include <grub/file.h>
#include <grub/term.h>
 
extern int cmdline_rawbcd;
extern int cmdline_rawwim;
extern int cmdline_gui;
extern int cmdline_pause;
extern unsigned int cmdline_index;

extern unsigned int wimboot_nfiles;

struct grub_wimboot_component
{
  grub_file_t file;
  char *file_name;
  grub_off_t size;
};

struct grub_wimboot_context
{
  int nfiles;
  struct grub_wimboot_component *components;
  grub_size_t size;
};

#ifndef ASSEMBLY

#include <stdint.h>

/** Construct wide-character version of a string constant */
#define L( x ) _L ( x )
#define _L( x ) L ## x

/** Page size */
#define PAGE_SIZE 4096

/** Debugging output */
#define DBG(...) do {                        \
            printf ( __VA_ARGS__ );            \
            grub_getkey ();            \
    } while ( 0 )

/* Mark parameter as unused */
#define __unused __attribute__ (( unused ))

#endif /* ASSEMBLY */

#endif /* _WIMBOOT_H */
