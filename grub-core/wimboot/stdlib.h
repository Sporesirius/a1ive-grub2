#ifndef _STDLIB_H
#define _STDLIB_H

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
 * Standard library
 *
 */
#include <grub/mm.h>
#include <grub/misc.h>
 
static inline unsigned long 
strtoul ( const char *nptr, char **endptr, int base ) {
  return grub_strtoul (nptr, endptr, base);
};

static inline void 
free (void *ptr)
{
  grub_free (ptr);
}

static inline void *
malloc (grub_size_t size)
{
  return grub_malloc (size);
}

static inline void *
calloc (grub_size_t size, grub_size_t nelem)
{
  return grub_zalloc (size * nelem);
}

static inline void *
realloc (void *ptr, grub_size_t size)
{
  return grub_realloc (ptr, size);
}

#endif /* _STDLIB_H */
