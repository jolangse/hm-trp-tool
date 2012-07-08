#include <errno.h>   /* Error number definitions */
#include <string.h>  /* String function definitions */
#include <stdio.h>   /* Standard I/O */
#include <unistd.h>  /* sleep */
#include <stdlib.h>  /* malloc, free */

#include "hm-trp.h"
#include "serial.h"

#define CBUFFER_SIZE 32

int main ( int argc, char** argv )
{
	int fd;
	int res;
	int r, c, f;
	long rate;

	unsigned char buf[CBUFFER_SIZE];

	config_t* config = malloc(sizeof(config_t));
	bzero(config, sizeof(config_t));

	if ( ! argv[1] )
	{
		printf("Serial port device required as argument, e.g. /dev/ttyUSB0\n");
		return 1;
	}

	if ( ! argv[2] )
	{
		printf("Desired bit-rate required. Use RS232 rates only.\n");
		return 1;
	}

	rate = atol( argv[2] );
	for ( r = 0; r <= count_rates; r++ )
		if ( port_rate_value[r] == rate ) { rate = r; break; }

	if ( r >= count_rates ) { printf("Invalid data rate requested\n"); return 1; }

	printf("Looking for device on port %s\n", argv[1]);

	for ( r = 0; r <= count_rates; r++ )
	{
		fd = open_port( argv[1], port_rates[r] );
		if ( fd < 0 )
		{
			perror(argv[1]); 
			return(-1);
		}

		if ( read_config( fd, config )  == 1 )
		{
			printf("Found device at baud-rate %d.\n", port_rate_value[r] );

			if ( r == rate  && ( config->air_rate == config->uart_rate ) )
			{
				printf("Data rate is already set to %d\n", port_rate_value[r] );
				return 0;
			}

			write_cmd( fd, cmd_air_rate );
			write_uint32_t( fd, port_rate_value[rate] );

			if ( ! read_ok( fd ) )
			{
				printf("Air rate NOT set\n");
				return 1;
			}

			write_cmd( fd, cmd_uart_rate );
			write_uint32_t( fd, port_rate_value[rate] );

			read_ok( fd ); // Clear buffer.

			printf("Device UART and air data rate set to %d\n", port_rate_value[rate] );
			return(0);
		}
		close(fd);
	}
	printf("Unable to find device on %s\n", argv[1]);
	return(1);
}

