#ifndef _SERIAL_H
#define _SERIAL_H

#include <termios.h> /* Terminal I/O support */
#include <errno.h>   /* Error number definitions */
#include <fcntl.h>   /* File descriptor manipulation */

int open_port ( char* device, int rate );

#endif /* _SERIAL_H */
