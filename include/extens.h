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

#include <pthread.h>
#include <stdint.h>

#define EXTEN_OK	((void *) 0)
#define EXTEN_FAILED	((void *)-1)

union params_u_t {
	float f;
	int32_t i;
	uint32_t ui;
};

enum extens_type {
	EXTENS_INCR_V0 = 0,
	EXTENS_THRD_V0 = 1,
};

/* In-process extenstion:
 * It is running with the main process. Performs faster because doesn't
 * require any syncronization code. Usually it is used for math algorithms
 * that don't require any blocked i/o operations. But it can still be used
 * with async i/o.
 */
struct extens_inproc_t {
	/* direct access to internal core parameters */
	union params_u_t **in;
	union params_u_t **out;
};

/* Thread extenstion:
 * This type runs as a separate thread and don't impose any special
 * requirements.
 */
struct extens_thread_t {

	/* parameters IDs that are defined in cfg file */
	int *id_in,
	    *id_out;

	/* threads local parameters copy */
	union params_u_t *in;
	union params_u_t *out;

	/* used to sync local parameters with core */
	pthread_mutex_t emutex;
};

struct extens_t {
	uint32_t type;

	void *lib_handle;

	char *section;
	char *name;

	void *(*init)(void *);
	void *(*run)(void *);
	void (*cleanup)(void *);

	uint32_t enable:1;

	int32_t count_in,
		count_out;

	/* pointer to any optional structures */
	void *opt;

	union {
		struct extens_inproc_t *inp;
		struct extens_thread_t *thr;
	} spec;
};

#endif /* EXTENS_H */
