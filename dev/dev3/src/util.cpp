#include "util.h"
#include "xalloc.h"
#include <errno.h>
#include <fcntl.h>
#include <tgmath.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#	include <direct.h>
#	include <stdio.h>
#	include <windows.h>
#endif

char *str_dup(const char *str)
{
	size_t size = strlen(str) + 1;
	return memcpy(xmalloc(size), str, size);
}

d3d_direction flip_direction(d3d_direction dir)
{
	switch (dir) {
	case D3D_DPOSX: return D3D_DNEGX;
	case D3D_DPOSY: return D3D_DNEGY;
	case D3D_DNEGX: return D3D_DPOSX;
	case D3D_DNEGY: return D3D_DPOSY;
	case D3D_DUP: return D3D_DDOWN;
	case D3D_DDOWN: return D3D_DUP;
	default: return dir;

	}
}

char *mid_cat(const char *part1, int mid, const char *part2)
{
	char *cat;
	size_t len1, len2, len;
	len1 = strlen(part1);
	len2 = strlen(part2);
	len = len1 + 1 + len2;
	cat = xmalloc(len + 1);
	memcpy(cat, part1, len1);
	cat[len1] = mid;
	memcpy(cat + len1 + 1, part2, len2 + 1);
	return cat;
}

int ensure_file(const char *path, bool dir)
{
#ifdef _WIN32
	if (dir) {
		if (_mkdir(path) && errno != EEXIST) return -1;
	} else {
		FILE *f = fopen(path, "a");
		if (!f) return -1;
		fclose(f);
	}
	return 0;
#else
	int fd;
	struct stat st;
	if (stat(path, &st)) {
		fd = dir ? mkdir(path, 0775)
			: open(path, O_RDONLY | O_CREAT, 0664);
	} else {
		if (dir) {
			if (!S_ISDIR(st.st_mode)) {
				errno = EEXIST;
				return -1;
			}
		} else if (!S_ISREG(st.st_mode)) {
			errno = S_ISDIR(st.st_mode) ? EISDIR : EEXIST;
			return -1;
		}
		fd = open(path, O_RDONLY);
	}
	if (fd < 0) return -1;
	close(fd);
	return 0;
#endif /* !defined(_WIN32) */
}

char *default_file(const char *name, const char *env)
{
#ifdef _WIN32
	static const char env_storage_dir[] = "AppData", ts3d_dir[] = "ts3d";
#else
	static const char env_storage_dir[] = "HOME", ts3d_dir[] = ".ts3d";
#endif
	char *dot_dir = NULL, *path = NULL;
	char *env_root, *env_path = getenv(env);
	if (env_path) return str_dup(env_path);
	env_root = getenv("TS3D_ROOT");
	if (env_root) {
		dot_dir = str_dup(env_root);
	} else {
		char *storage_dir = getenv(env_storage_dir);
		if (storage_dir) {
			dot_dir = mid_cat(storage_dir, DIRSEP, ts3d_dir);
		} else {
			errno = ENOENT;
			return NULL;
		}
	}
	if (!ensure_file(dot_dir, true))
		path = mid_cat(dot_dir, DIRSEP, name);
	free(dot_dir);
	return path;
}

#ifdef _WIN32
int try_setenv(const char *UNUSED_VAR(name), const char *UNUSED_VAR(value),
	int UNUSED_VAR(overwrite))
{
	errno = ENOSYS;
	return -1;
}
#else
int try_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}
#endif

void subst_native_dir_sep(char *path)
{
	if (DIRSEP != '/') {
		while ((path = strchr(path, '/'))) {
			*path = DIRSEP;
		}
	}
}

void move_direction(d3d_direction dir, size_t *x, size_t *y)
{
	switch(dir) {
	case D3D_DPOSX: ++*x; break;
	case D3D_DPOSY: ++*y; break;
	case D3D_DNEGX: --*x; break;
	case D3D_DNEGY: --*y; break;
	default: break;
	}
}

void vec_norm_mul(d3d_vec_s *vec, d3d_scalar mag)
{
	d3d_scalar hyp = hypot(vec->x, vec->y);
	if (hyp != 0) {
		vec->x *= mag / hyp;
		vec->y *= mag / hyp;
	}
}
