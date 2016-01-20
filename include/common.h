/*
 * Copyright (C) 2016 Alexey Kodanev <akodanev@gmail.com>
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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define UNUSED_ATTR	__attribute__((unused))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#define TINF	0
#define TERR	1
#define TDBG	2
#define TERN	4

#define prl(type, fmt, ...) \
do {								\
	switch (type) {						\
	case TINF:						\
		printf(PID ":  INFO: " fmt, ##__VA_ARGS__);	\
		printf("\n");					\
	break;							\
	case TDBG:						\
		printf(PID ": DEBUG: " fmt, ##__VA_ARGS__);	\
		printf("\n");					\
	break;							\
	case TERR:						\
		printf(PID ": ERROR: " fmt, ##__VA_ARGS__);	\
		printf("\n");					\
		fflush(stdout);					\
	break;							\
	case TERN:						\
		printf(PID ": ERROR: " fmt, ##__VA_ARGS__);	\
		printf(", error: %s\n", strerror(errno));	\
		fflush(stdout);					\
	}							\
} while (0)

#define LIMIT_VAR_RANGE(x, a, b)	\
	do {				\
		if ((x) < (a))		\
			x = a;		\
		else if ((x) > (b))	\
			x = b;		\
	} while (0)

#endif /* __COMMON_H__ */
