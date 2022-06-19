#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

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
