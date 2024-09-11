#ifndef __SLAVE_H__
#define __SLAVE_H__

#include <stdint.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


void get_md5(char *const file_path, char *md5);
#endif // __SLAVE_H__
