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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>

#include "common.h"

#define PID	"ilogger"

#ifndef DEBUG_ILOGGER
#define DEBUG_ILOGGER		0
#endif

#if DEBUG_ILOGGER
#define prl_dbg(type, fmt, ...)		prl(type, fmt, ##__VA_ARGS__)
#else
#define prl_dbg(type, fmt, ...)		;
#endif

#define BUF_SIZE	4096

static char buf[BUF_SIZE];
static struct stat st;
static struct statfs stfs;
static char fname[PATH_MAX];

#define ONEM		(1 << 20)
static ssize_t max_size = ONEM * 32;

static int threshold = 75; /* % */

static char *mnt_dir,
	    *devices,
	    *mnt_opt = "rw",
	    *fs_type = "vfat";

struct ldev_t {
	int major;
	int minor;
	int fd;
	char name[128];
	int found;
	char mnt_dir[128];
	char log_dir[128];
	int umount;
	time_t last_mtime;
};

#define MAX_LOG_DEVICES		4
static struct ldev_t ldev[MAX_LOG_DEVICES];
static int dev_num;

static int devmap[256] = { [0 ... 255] = -1 };

static const char version[] = "0.1";

static void print_log_usage(FILE* stream, const char *name, int exit_code)
{
	fprintf(stream, "Usage: %s OPTIONS:\n", name);
	fprintf(stream,
		"  -h      show this message\n"
		"  -s x    maximum file size in MB, default is %u MB\n"
		"  -u x    disk usage threshold, default %d %%\n"
		"  -d x    specify dev major,minor,... to start looking for,\n"
		"          the first device found will be used,\n"
		"          check /proc/partitions\n"
		"  -m x    target path where to mount a device\n"
		"  -o x    mount options, default is '%s'\n"
		"  -t x    filesystem type, default is '%s'\n"
		"  -V      version\n", (unsigned int) (max_size / ONEM),
		threshold, mnt_opt, fs_type);
	exit(exit_code);
}

static void parse_log_opts(int argc, char *argv[])
{
	int next_option = 0;

	const char short_options[] = "hs:u:d:m:o:t:V";
	/* An array describing valid long options. */
	const struct option long_options[] = {
		{ "help", 0, NULL, 'h' },
		{ "filesize", 0, NULL, 's' },
		{ "blkusage", 0, NULL, 'u' },
		{ "devices", 0, NULL, 'd' },
		{ "mnt_dir", 0, NULL, 'm' },
		{ "options", 0, NULL, 'o' },
		{ "fs_type", 0, NULL, 't' },
		{ "version", 0, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	do {
		next_option = getopt_long(argc, argv, short_options,
			long_options, NULL);
		switch (next_option) {
		case 'h':
			print_log_usage(stdout, argv[0], 0);
		break;
		case 's':
			max_size = atoi(optarg) * ONEM;
		break;
		case 'u':
			threshold = atoi(optarg);
		break;
		case 'd':
			devices = strdup(optarg);
		break;
		case 'm':
			mnt_dir = strdup(optarg);
		break;
		case 'o':
			mnt_opt = strdup(optarg);
		break;
		case 't':
			fs_type = strdup(optarg);
		break;
		case 'V':
			printf("version %s\n", version);
			exit(0);
		break;
		case '?': /* The user specified an invalid option */
			print_log_usage(stderr, argv[0], 1);
			exit(1);
		case ':':
			fprintf(stderr, "Missing argument\n");
			print_log_usage(stderr, argv[0], 1);
			exit(1);
		case -1:
		break;
		default:
			abort();
		}
	} while (next_option != -1);
}

static int get_block_device(void)
{
	int i;
	static char buf[128], dev[128];
	int major = 0, minor = 0, blocks = 0;
	struct ldev_t *d;

	FILE *f = fopen("/proc/partitions", "r");
	if (f == NULL)
		return -1;

	strcpy(dev, "/dev/");

	for (i = 0; i < dev_num; ++i)
		ldev[i].found = 0;

	while (fgets(buf, 64, f) != NULL) {
		if (!strncmp(buf, "major", 5))
			continue;

		if (sscanf(buf, "%d %d %d %123s", &major, &minor,
		    &blocks, dev + 5) != 4)
			continue;

		if (major < 0 || major > 255 || devmap[major] == -1)
			continue;

		if (minor < 0 || minor < devmap[major] ||
		    dev[5] == '\0' || blocks == 0)
			continue;

		for (i = 0, d = ldev; i < dev_num; ++i, ++d) {
			if (d->major != major || d->minor > minor)
				continue;
			prl_dbg(TINF, "found device %d %d %s",
				major, minor, dev);
			strcpy(d->name, dev);
			d->found = 1;
		}
	}

	fclose(f);

	return 0;
}

static int new_file(struct ldev_t *d)
{
	static char buf[128];
	time_t tt = time(0);
	struct tm *tms = gmtime(&tt);
	int tfd;

	snprintf(fname, PATH_MAX, "%s/ilog_probe", d->log_dir);
	tfd = open(fname, O_CREAT | O_TRUNC | O_NONBLOCK, 0600);

	if (tfd == -1) {
		if (d->umount)
			umount(d->mnt_dir);

		d->umount = 0;

		if (get_block_device() < 0 || !d->found) {
			prl_dbg(TERR, "device not found");
			return -1;
		}

		if (mount(d->name, d->mnt_dir, fs_type,
		    MS_NOATIME, NULL)) {
			prl_dbg(TERN, "mount '%s' failed", d->name);
			return -1;
		}
		d->umount = 1;
	} else {
		unlink(fname);
		close(tfd);
	}

	strftime(buf, 128, "%d%m%Y_%H%M%S", tms);
	snprintf(fname, PATH_MAX, "%s/ilog.%s.log", d->log_dir, buf);

	d->fd = open(fname, O_CREAT | O_APPEND | O_RDWR | O_NONBLOCK, 0644);
	if (d->fd < 0) {
		prl_dbg(TERN, "open(%s) failed", fname);
		return -1;
	}
	if (fstat(d->fd, &st) == -1) {
		close(d->fd);
		d->fd = -1;
		return -1;
	}

	d->last_mtime = st.st_mtime;

	return 0;
}

static int remove_files(struct ldev_t *d)
{
	struct dirent *entry;
	int i = 0, ret = 0;
	char *file;
	static char path[PATH_MAX], path_rm[PATH_MAX];
	time_t tlast = d->last_mtime;
	DIR *dir = opendir(d->log_dir);

	path_rm[0] = '\0';

	while ((entry = readdir(dir)) != NULL) {
		file = entry->d_name;

		if (strncmp(file, "ilog.", 5))
			continue;

		snprintf(path, PATH_MAX, "%s/%s", d->log_dir, file);

		if (stat(path, &st)) {
			ret = -1;
			goto end;
		}

		if (st.st_mtime < tlast) {
			tlast = st.st_mtime;
			strcpy(path_rm, path);
		}
		++i;
	}

	if (i == 1 && ftruncate(d->fd, 0) && path_rm[0]) {
		unlink(path_rm);
		goto end;
	}

	if (path_rm[0])
		unlink(path_rm);

end:
	closedir(dir);
	return ret;
}

static int check_size(struct ldev_t *d)
{
	int res;

	if (d->fd < 0)
		return -1;

	if (fstatfs(d->fd, &stfs) < 0)
		goto err;

	res = 100 - stfs.f_bfree * 100 / stfs.f_blocks;

	if (res < threshold)
		return 0;

	if (remove_files(d))
		goto err;

	return 0;
err:
	close(d->fd);
	d->fd = -1;
	return -1;
}

static void write_log(struct ldev_t *d, size_t len)
{
	if (d->fd < 0 && new_file(d))
		return;

	if (check_size(d) && new_file(d))
		return;

	if (pwrite(d->fd, buf, len, 0) < 0)
		goto err;

	if (fstat(d->fd, &st))
		goto err;

	if (st.st_size >= max_size)
		goto err;

	return;
err:
	close(d->fd);
	d->fd = -1;
}

static void parse_device_opt(void)
{
	int k = 0, len;
	char *p, *start = devices;
	int *val;

	if (!devices) {
		prl(TERR, "device option not specified");
		abort();
	}

	len = strlen(devices);

	if (len < 3) {
		prl(TERR, "invalid devices option");
		abort();
	}

	do {
		p = strchr(start, ',');

		val = (k == 0) ? &ldev[dev_num].major : &ldev[dev_num].minor;

		if (p == NULL)
			sscanf(start, "%d", val);
		else
			sscanf(start, "%d,", val);

		if (++k >= 2) {
			if (++dev_num > MAX_LOG_DEVICES) {
				prl(TERR, "max devices: %d",
				    MAX_LOG_DEVICES);
				abort();
			}
			k = 0;
		}

		start = p + 1;

	} while (p && (start < (devices + len)));

	if (dev_num < 1) {
		prl(TERR, "devices are not defined");
		abort();
	}
}

static void init_ldev(void)
{
	int i;
	struct ldev_t *d;

	for (i = 0, d = ldev; i < dev_num; ++i, ++d) {
		prl_dbg(TINF, "%d.dev: major %d, minor %d",
			i, d->major, d->minor);

		if (d->major <= 0 || d->major > 255 ||
		    d->minor < 0 || d->minor > 255) {
			prl(TERR, "wrong device major number, %d", d->major);
			abort();
		}

		devmap[d->major] = d->minor;

		d->fd = -1;
		d->umount = 1;

		snprintf(d->mnt_dir, 128, "%s/%d%d", mnt_dir,
			 d->major, d->minor);
		mkdir(d->mnt_dir, 0755);

		snprintf(d->log_dir, 128, "%s/ilogger", d->mnt_dir);
	}
}

int main(int argc, char *argv[])
{
	int i;
	size_t len;
	struct ldev_t *d;

	parse_log_opts(argc, argv);

	if (max_size < ONEM) {
		prl(TERR, "invalid max_size parameter");
		abort();
	}

	if (threshold < 1 || threshold > 100) {
		prl(TERR, "invalid threshold, expected range: 1..100%%");
		abort();
	}

	if (!mnt_dir) {
		prl(TERR, "mount directory not specified");
		abort();
	}

	parse_device_opt();

	init_ldev();

	while (1) {
		if (fgets(buf, BUF_SIZE, stdin) == NULL)
			break;

		len = strlen(buf);

		if (len > 1) {
			for (i = 0, d = ldev; i < dev_num; ++i, ++d)
				write_log(d, len);
		}

		usleep(200);
	}

	return 0;
}
