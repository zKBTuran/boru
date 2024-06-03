#include "common.h"

#define VERSION "1.1.4"
#define CONFIG_PATH "/etc/boru.conf"

int main(int argc, char **argv) {
    unsigned int sleep_us, ts_ttl;
    char groupname[64], wrong_pw_sleep[64], session_ttl[64], nopass[64], password[128];

    if (access(CONFIG_PATH, F_OK) == -1)
        genconf(CONFIG_PATH);

    if (argc == 1) {
        printf(" _               \n"
               "| |_ ___ ___ _ _ \n"
               "| . | . |  _| | |\n"
               "|___|___|_| |___|\n\n");
        printf("Version is %s!\n", VERSION);
        printf("Usage: %s [command]\n", argv[0]);
        return 0;
    }

    if (geteuid() != 0)
        errx(1, "The boru binary needs to be installed as SUID.");

    int ruid = getuid();
    if (ruid == 0)
        runprog(&argv[1]);

    FILE *fp = fopen(CONFIG_PATH, "r");
    getconf(fp, "group", groupname, sizeof(groupname));
    getconf(fp, "wrong_pw_sleep", wrong_pw_sleep, sizeof(wrong_pw_sleep));
    getconf(fp, "session_ttl", session_ttl, sizeof(session_ttl));
    getconf(fp, "nopass", nopass, sizeof(nopass));
    fclose(fp);

    sleep_us = atoi(wrong_pw_sleep) * 1000;
    ts_ttl = atoi(session_ttl) * 60;

    if (getsession(getppid(), ts_ttl, ruid) == 0)
        runprog(&argv[1]);

    struct passwd *pw = getpwuid(ruid);
    if (!pw)
        err(1, "Could not get user info");

    struct group *grp = getgrnam(groupname);
    if (!grp)
        errx(1, "The group '%s' does not exist.", groupname);

    int is_member = 0;
    for (char **member = grp->gr_mem; *member; member++) {
        if (strcmp(*member, pw->pw_name) == 0) {
            is_member = 1;
            break;
        }
    }
    if (!is_member)
        errx(1, "You are not allowed to execute boru.");

    struct spwd *sp = getspnam(pw->pw_name);
    if (!sp || !sp->sp_pwdp)
        err(1, "Could not get shadow entry");

    if (*nopass != '1') {
        for (int tries = 0; tries < 3; tries++) {
            if (!readpassphrase("(boru) Password: ", password, sizeof(password)))
                err(1, "Could not get passphrase");

            char *hashed_pw = crypt(password, sp->sp_pwdp);
            explicit_bzero(password, sizeof(password));

            if (!hashed_pw)
                errx(1, "Could not hash password");

            if (strcmp(sp->sp_pwdp, hashed_pw) == 0) {
                setsession(getppid(), ts_ttl, ruid);
                runprog(&argv[1]);
            }

            usleep(sleep_us);
            fprintf(stderr, "Wrong password.\n");
        }
    } else {
        setsession(getppid(), ts_ttl, ruid);
        runprog(&argv[1]);
    }

    errx(1, "Too many wrong password attempts.");
    return 1;
}