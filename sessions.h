#pragma once

#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int getpstartts(int pid, unsigned long long* startts) {
	char path[255], fc[1024];
	char* ptr = fc;

	snprintf(path, sizeof(path), "/proc/%d/stat", pid);
	int fd = open(path, O_RDONLY);

	if (fd < 0)
		err(1, "Could not open %s", path);

	int bytes_read = read(fd, fc, sizeof(fc));

	close(fd);

	if (bytes_read < 0)
		err(1, "Could not read %s", path);

	fc[bytes_read] = '\0';

	if (memchr(ptr, '\0', bytes_read) != NULL)
		return -1;

	ptr = strrchr(fc, ')');

	char* token = strtok(ptr, " ");

	for (int i = 0; i<20 && token; i++)
		token = strtok(NULL, " ");

	if (!token)
		return -1;

	unsigned long long temp_ts = strtoull(token, NULL, 10);
	if (temp_ts == 0 || temp_ts == ULLONG_MAX)
		return -1;

	*startts = temp_ts;
	return 0;
}

int ensuredir() {
	struct stat st;
	int fd = open("/run/boru", O_RDONLY, O_DIRECTORY | O_NOFOLLOW);

	if (fd < 0) {
		if (errno == ENOENT) {
			if (mkdir("/run/boru", 0700) < 0)
				err(1, "Could not create /run/boru");

			fd = open("/run/boru", O_RDONLY, O_DIRECTORY | O_NOFOLLOW);
			if (fd < 0)
				err(1, "Could not open /run/boru");
		}
		else
			err(1, "Could not open /run/boru");
	}

	if (fstat(fd, &st) < 0) {
		close(fd);
		err(1, "Could not fstat /run/boru");
	}

	close(fd);

	if (st.st_uid != 0 || st.st_mode != (0700 | S_IFDIR))
		return -1;

	return 0;
}

void setsession(int pid, unsigned int ts_ttl, int ruid) {
	unsigned long long startts;
	char path[1024], ts_str[32];

	if (ts_ttl == 0)
		return;

	if (ensuredir() < 0 || getpstartts(pid, &startts) < 0)
		return;

	snprintf(path, sizeof(path), "/run/boru/%d-%d-%llu", ruid, pid, startts);

	int fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0700);
	if (fd < 0) {
		if (errno == EEXIST)
			return;
		err(1, "Could not open %s", path);
	}

	snprintf(ts_str, sizeof(ts_str), "%llu", (unsigned long long)time(NULL));

	if (write(fd, ts_str, strlen(ts_str)) < 0) {
		close(fd);
		err(1, "Could not write to %s", path);
	}

	close(fd);

	return;
}

int getsession(int pid, unsigned int ts_ttl, int ruid) {
	unsigned long long startts, current;
	char path[1024], ts_str[32];

	if (ts_ttl == 0)
		return -1;

	if (ensuredir() < 0 || getpstartts(pid, &startts) < 0)
		return -1;

	snprintf(path, sizeof(path), "/run/boru/%d-%d-%llu", ruid, pid, startts);

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		if (errno == ENOENT)
			return -1;
		err(1, "Could not open %s", path);
	}

	int bytes_read = read(fd, ts_str, sizeof(ts_str));

	close(fd);

	if (bytes_read < 0)
		err(1, "Could not read %s", path);
	ts_str[bytes_read] = '\0';

	startts = strtoull(ts_str, NULL, 10);
	current = time(NULL);

	if (current - startts > ts_ttl) {
		unlink(path);
		return -1;
	}

	return 0;
}
