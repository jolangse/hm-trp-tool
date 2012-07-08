#include "serial.h"

int open_port ( char* device, int rate )
{
	int f;
	struct termios tio;

	f = open ( device, O_RDWR | O_NOCTTY ); // Not using O_NDELAY, that caused ERR11: temp unavail.
	if ( f < 0 )
	{
		return(-1);
	}

	tio.c_cflag = rate | CS8 | CLOCAL | CREAD;

	// 8 bits, local mode, read enabled
	tio.c_cflag |= CS8 | CLOCAL | CREAD;

	// no flow-control,
	tio.c_cflag &= ~CRTSCTS;

	// no parity
        tio.c_iflag = IGNPAR;

        tio.c_oflag = 0;
        tio.c_lflag = 0; // Non-canonical read, no other options.
	tio.c_cc[VMIN] = 0; // Disable minimum characters for read
       	tio.c_cc[VTIME] = 5; // Wait for a max of 0.5 second

        tcsetattr(f,TCSANOW,&tio); // Apply this now.

	tcflush(f, TCIFLUSH); //  the serial input buffer.

	return f;
}
