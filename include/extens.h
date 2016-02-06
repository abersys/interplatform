/*
 * Copyright (C) 2016 Alexey Kodanev <akodanev@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the  Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXTENS_H
#define EXTENS_H

#include <stdint.h>

union params_u_t {
	float f;
	int32_t i;
	uint32_t ui;
};

enum extens_type {
	EXTENS_V0 = 0,
};

struct extens_t {
	uint32_t type;

	void *lib_handle;

	char *section;
	char *name;

	void (*init)(void *);
	void *(*run)(void *);

	/* flags */
	uint32_t enable:1;
	uint32_t :31;

	uint32_t count_in,
		 count_out;

	/* direct access to internal core parameters */
	union params_u_t **in;
	union params_u_t **out;
};

struct extens_thread_t {
	uint32_t type;

	void *lib_handle;

	char *section;
	char *name;

	void (*init)(void *);
	void *(*run)(void *);

	/* flags */
	uint32_t enable:1;
	uint32_t :31;

	uint32_t count_in,
		 count_out;

	/* threads local copy of parameters */
	union params_u_t *in;
	union params_u_t *out;

	/* used to sync local parameters with core */
	pthread_mutex_t emutex;
};

#endif /* EXTENS_H */
