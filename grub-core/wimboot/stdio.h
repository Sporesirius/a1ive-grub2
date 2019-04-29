#ifndef _STDIO_H
#define _STDIO_H

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
 * Standard Input/Output
 *
 */

#include <stdint.h>
#include <grub/misc.h>

/**
 *
 * Etherboot's printf() functions understand the following subset of
 * the standard C printf()'s format specifiers:
 *
 *  - Flag characters
 *      - '#'       - Alternate form (i.e. "0x" prefix)
 *      - '0'       - Zero-pad
 *  - Field widths
 *  - Length modifiers
 *      - 'hh'      - Signed / unsigned char
 *      - 'h'       - Signed / unsigned short
 *      - 'l'       - Signed / unsigned long
 *      - 'll'      - Signed / unsigned long long
 *      - 'z'       - Signed / unsigned size_t
 *  - Conversion specifiers
 *      - 'd'       - Signed decimal
 *      - 'x','X'   - Unsigned hexadecimal
 *      - 'c'       - Character
 *      - 's'       - String
 *      - 'p'       - Pointer
 *
 * Hexadecimal numbers are always zero-padded to the specified field
 * width (if any); decimal numbers are always space-padded.  Decimal
 * long longs are not supported.
 *
 */

static inline int
printf (const char *fmt, ...)
{
  va_list ap;
  int ret;

  va_start (ap, fmt);
  ret = grub_vprintf (fmt, ap);
  va_end (ap);

  return ret;
}

static inline int
snprintf (char *str, grub_size_t n, const char *fmt, ...)
{
  va_list ap;
  int ret;

  va_start (ap, fmt);
  ret = grub_vsnprintf (str, n, fmt, ap);
  va_end (ap);

  return ret;
}

static inline int 
vprintf ( const char *fmt, va_list args ) {
  return grub_vprintf (fmt, args);
};

static inline void die ( const char *fmt, ... ) {
    va_list args;
    /* Print message */
    va_start ( args, fmt );
    vprintf ( fmt, args );
    va_end ( args );
    grub_fatal ("aborted\n");
}

#endif /* _STDIO_H */
