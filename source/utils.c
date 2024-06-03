#include "common.h"

void genconf(const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp)
        errx(1, "Could not create %s", path);

    fprintf(fp, "group=wheel\n");
    fprintf(fp, "wrong_pw_sleep=1000\n");
    fprintf(fp, "session_ttl=5\n");
    fprintf(fp, "nopass=0\n");

    if (fclose(fp) != 0)
        errx(1, "Error closing the file %s", path);
}

void getconf(FILE *fp, const char *entry, char *result, size_t len_result) {
    char *token;
    char line[256];

    fseek(fp, 0, SEEK_SET);
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(entry, line, strlen(entry)) == 0) {
            token = strchr(line, '=');
            if (token) {
                strncpy(result, token + 1, len_result - 1);
                result[len_result - 1] = '\0';
                result[strcspn(result, "\n")] = '\0';
                return;
            }
        }
    }

    errx(1, "Could not get '%s' entry in config", entry);
}

void runprog(char **program_argv) {
    if (setuid(0) < 0 || setgid(0) < 0)
        err(1, "Could not setuid/setgid");

    if (putenv("HOME=/root") != 0)
        err(1, "Could not set HOME");

    execvp(program_argv[0], program_argv);
    err(1, "execvp");
}

int ensuredir(void) {
    struct stat st;
    if (stat("/run/boru", &st) == -1) {
        if (mkdir("/run/boru", 0700) == -1)
            err(1, "Could not create /run/boru");
        if (stat("/run/boru", &st) == -1)
            err(1, "Could not open /run/boru");
    }

    if (!S_ISDIR(st.st_mode) || st.st_uid != 0)
        err(1, "Could not fstat /run/boru");

    return 0;
}

int getpstartts(int pid, unsigned long long *startts) {
    char path[256], fc[4096];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        err(1, "Could not open %s", path);

    int bytes_read = read(fd, fc, sizeof(fc) - 1);
    close(fd);

    if (bytes_read < 0)
        err(1, "Could not read %s", path);

    fc[bytes_read] = '\0';

    char *ptr = strrchr(fc, ')');
    if (!ptr)
        return -1;
    ptr++;

    int field = 0;
    char *token = strtok(ptr, " ");
    while (token) {
        if (++field == 21) {
            unsigned long long temp_ts = strtoull(token, NULL, 10);
            if (temp_ts == 0 || temp_ts == ULLONG_MAX)
                return -1;
            *startts = temp_ts;
            return 0;
        }
        token = strtok(NULL, " ");
    }
    return -1;
}

void setsession(int pid, unsigned int ts_ttl, int ruid) {
    char ts_str[32], path[1024];
    unsigned long long startts;

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
}

int getsession(int pid, unsigned int ts_ttl, int ruid) {
    char ts_str[32], path[1024];
    unsigned long long startts;

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

    ssize_t bytes_read = read(fd, ts_str, sizeof(ts_str) - 1);
    if (bytes_read < 0) {
        close(fd);
        err(1, "Could not read %s", path);
    }

    ts_str[bytes_read] = '\0';
    close(fd);

    unsigned long long file_ts = strtoull(ts_str, NULL, 10);
    unsigned long long current_ts = time(NULL);

    if (current_ts - file_ts > ts_ttl) {
        unlink(path);
        return -1;
    }

    return 0;
}

char *readpassphrase(const char *prompt, char *buf, size_t bufsz) {
    int ttyfd;
    struct termios oldterm, newterm;

    ttyfd = open("/dev/tty", O_RDWR);
    if (ttyfd < 0)
        return NULL;

    if (tcgetattr(ttyfd, &oldterm) < 0) {
        close(ttyfd);
        return NULL;
    }

    newterm = oldterm;
    newterm.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    if (tcsetattr(ttyfd, TCSAFLUSH, &newterm) < 0) {
        close(ttyfd);
        return NULL;
    }

    if (write(ttyfd, prompt, strlen(prompt)) < 0) {
        close(ttyfd);
        return NULL;
    }

    ssize_t n = read(ttyfd, buf, bufsz);
    if (n < 0) {
        close(ttyfd);
        return NULL;
    }

    buf[n > 0 ? n - 1 : 0] = '\0';

    tcsetattr(ttyfd, TCSANOW, &oldterm);
    close(ttyfd);

    return buf;
}