#ifndef _CTYPE_H
#define _CTYPE_H

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
 * Character types
 *
 */

#include <grub/misc.h>

static inline int islower ( int c ) {
  return grub_islower (c);
}

static inline int isupper ( int c ) {
  return grub_isupper (c);
}

static inline int toupper ( int c ) {
  return grub_toupper (c);
}

static inline int isspace ( int c ) {
  return grub_isspace (c);   
};

#endif /* _CTYPE_H */
