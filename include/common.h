#ifndef COMMON_H
#define COMMON_H
#pragma once

#include <crypt.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define VERSION "1.1.4"

char* readpassphrase(const char* prompt, char* buf, size_t bufsz);
int ensuredir(void);
int getpstartts(int pid, unsigned long long* startts);
int getsession(int pid, unsigned int ts_ttl, int ruid);
void genconf(const char* path);
void getconf(FILE* fp, const char* entry, char* result, size_t len_result);
void runprog(char** program_argv);
void setsession(int pid, unsigned int ts_ttl, int ruid);
#endif