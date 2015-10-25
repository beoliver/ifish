#ifndef __SAFEFORK_H__
#define __SAFEFORK_H__

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>

#define MAX_PROCESSES 6

extern int errno;
pid_t safefork(void);

#endif
