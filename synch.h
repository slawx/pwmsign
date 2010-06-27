/*
 * Dimmable LED sign firmware.
 * Copyright 2010 Daniel Holth <dholth@fastmail.fm>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYNCH_H
#define SYNCH_H

#undef BAUD  // avoid compiler warning
#define BAUD 38400
#include <util/setbaud.h>

#define SYNCH_TARGET ((UBRR_VALUE + 1) * (16 >> USE_2X))
#define MAX_STEP (0x20)
#define TOO_HIGH (SYNCH_TARGET + (SYNCH_TARGET >> 2))
#define TOO_LOW (SYNCH_TARGET - (SYNCH_TARGET >> 2))
#define OSCCAL_MAX (0x7f)
#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif
#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#endif
