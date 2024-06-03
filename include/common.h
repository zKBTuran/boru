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

int ensuredir(void);
void genconf(const char* path);
void runprog(char** program_argv);
int getpstartts(int pid, unsigned long long* startts);
int getsession(int pid, unsigned int ts_ttl, int ruid);
void setsession(int pid, unsigned int ts_ttl, int ruid);
char* readpassphrase(const char* prompt, char* buf, size_t bufsz);
void getconf(FILE* fp, const char* entry, char* result, size_t len_result);
#endif