#include "common.h"

void genconf(const char* path) {
    FILE* fp = fopen(path, "w");
    if (!fp)
        errx(1, "Could not create %s", path);

    fprintf(fp, "group=wheel\n");
    fprintf(fp, "wrong_pw_sleep=1000\n");
    fprintf(fp, "session_ttl=5\n");
    fprintf(fp, "nopass=1\n");

    if (fclose(fp) != 0)
        errx(1, "Error closing the file %s", path);
}

void getconf(FILE* fp, const char* entry, char* result, size_t len_result) {
    char* line = NULL;
    size_t len = 0;

    fseek(fp, 0, SEEK_SET);

    while (getline(&line, &len, fp) != -1) {
        if (strncmp(entry, line, strlen(entry)) == 0) {
            strtok(line, "=");
            char* token = strtok(NULL, "=");
            if (token) {
                strncpy(result, token, len_result);
                result[strcspn(result, "\n")] = 0;
                free(line);
                return;
            }
        }
        free(line);
        line = NULL;
        len = 0;
    }

    errx(1, "Could not get '%s' entry in config", entry);
}

void runprog(char** program_argv) {
	if (setuid(0) < 0)
		err(1, "Could not setuid");
	if (setgid(0) < 0)
		err(1, "Could not setgid");

	putenv("HOME=/root");

	execvp(program_argv[0], program_argv);

	err(1, program_argv[0]);
}

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

int ensuredir(void) {
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

char* readpassphrase(const char* prompt, char* buf, size_t bufsz) {
    char stdin_path[256];
    char tty_link_path[256];
    int n;
    int ttyfd = -1;

    struct termios term;

    for (int i = 0; i < 3; i++) {
        if (tcgetattr(i, &term) == 0) {
            ttyfd = i;
            break;
        }
    }
    
    if (ttyfd < 0)
        return NULL;

    snprintf(tty_link_path, sizeof(tty_link_path), "/proc/self/fd/%d", ttyfd);

    n = readlink(tty_link_path, stdin_path, sizeof(stdin_path));
    if (n < 0)
        return NULL;
    
    stdin_path[n] = '\0';

    int fd = open(stdin_path, O_RDWR);
    if (fd < 0)
        return NULL;

    term.c_lflag &= ~ECHO;
    tcsetattr(ttyfd, 0, &term);
    term.c_lflag |= ECHO;

    if (write(fd, prompt, strlen(prompt)) < 0) {
        tcsetattr(ttyfd, 0, &term);
        close(fd);
        return NULL;
    }

    n = read(fd, buf, bufsz);
    if (n < 0) {
        tcsetattr(ttyfd, 0, &term);
        n = write(fd, "\n", 1);
        close(fd);
        return NULL;
    }

    buf[n-1] = '\0';

    n = write(fd, "\n", 1);

    close(fd);
    tcsetattr(ttyfd, 0, &term);

    return buf;
}
