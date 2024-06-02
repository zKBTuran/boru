#include "common.h"

int main(int argc, char** argv) {
	char groupname[64], wrong_pw_sleep[64], session_ttl[64], nopass[64], password[128];
	unsigned int sleep_us, tries, ts_ttl;
	const char* config_path = "/etc/boru.conf";

    if (argc == 1) {
		printf(
            " _               \n"
            "| |_ ___ ___ _ _ \n"
            "| . | . |  _| | |\n"
            "|___|___|_| |___|\n\n"
        );
        printf("Version is %s!\n", VERSION);
        printf("Usage: %s [command]\n", argv[0]);
		if (access(config_path, F_OK) == -1) {
            genconf(config_path);
        }
        return 0;
    }

	if (geteuid() != 0)
		errx(1, "The boru binary needs to be installed as SUID.");

	int ruid = getuid();
	if (ruid == 0)
		runprog(&argv[1]);
	
	FILE* fp = fopen(config_path, "r");
    getconf(fp, "group", groupname, sizeof(groupname));
    getconf(fp, "wrong_pw_sleep", wrong_pw_sleep, sizeof(wrong_pw_sleep));
    getconf(fp, "session_ttl", session_ttl, sizeof(session_ttl));
    getconf(fp, "nopass", nopass, sizeof(nopass));
    sleep_us = atoi(wrong_pw_sleep) * 1000;
    ts_ttl = atoi(session_ttl) * 60;

    fclose(fp);

	if (getsession(getppid(), ts_ttl, ruid) == 0)
		runprog(&argv[1]);

	struct passwd* pw = getpwuid(ruid);
	if (!pw) {
		if (errno == 0)
			errx(1, "No user with UID %d", ruid);
		else
			err(1, "Could not get user info");
	}

	struct group* current_group_entry = getgrent();
	while (current_group_entry) {
		if (strcmp(current_group_entry->gr_name, groupname) == 0)
			break;
		current_group_entry = getgrent();
	}

	if (!current_group_entry)
		errx(1, "The group '%s' does not exist.", groupname);
	
	char* current_member = current_group_entry->gr_mem[0];
	for (int i = 1; current_member; i++) {
		if (strcmp(current_member, pw->pw_name) == 0)
			break;
		current_member = current_group_entry->gr_mem[i];
	}

	if (!current_member)
		errx(1, "You are not allowed to execute boru.");

	struct spwd* shadowEntry = getspnam(pw->pw_name);

	if (!shadowEntry || !shadowEntry->sp_pwdp)
		err(1, "Could not get shadow entry");

        if (*nopass != '0') {
	tries = 0;
	while (tries < 3) {
		if (!readpassphrase("(boru) Password: ", password, sizeof(password)))
			err(1, "Could not get passphrase");

		char* hashed_pw = crypt(password, shadowEntry->sp_pwdp);
		memset(password, 0, sizeof(password));
		
		if (!hashed_pw)
			errx(1, "Could not hash password, does your user have a password?");

		if (strcmp(shadowEntry->sp_pwdp, hashed_pw) == 0) {
			setsession(getppid(), ts_ttl, ruid);
			runprog(&argv[1]);
		}

		usleep(sleep_us);
		fprintf(stderr, "Wrong password.\n");
		tries++;
	}}
        else {
          setsession(getppid(), ts_ttl, ruid);
          runprog(&argv[1]);
        }
	errx(1, "Too many wrong password attempts.");
	return 1;
}