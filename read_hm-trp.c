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
	unsigned char buf[CBUFFER_SIZE];

	config_t* config = malloc(sizeof(config_t));
	bzero(config, sizeof(config_t));

	if ( ! argv[1] )
	{
		printf("Serial port device required as argument, e.g. /dev/ttyUSB0\n");
		return 1;
	}


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
			printf("Found device at baud-rate %d, config:\n", port_rate_value[r] );
			printf("Freq      %d \n", config->freq );
			printf("Air rate  %d \n", config->air_rate );
			printf("Deviation %d \n", config->deviation );
			printf("TX Power  %d \n", config->power );
			printf("BW        %d \n", config->bw );
			printf("UART rate %d \n", config->uart_rate );


			for ( f = 0; f < count_devtype; f++ ) 
			{
				if ( (config->freq >= freq_min[f]) &&
						(config->freq <= freq_max[f]))
				{
					printf("Device type is %s\n", dev_name[f]);
				}
			}

			return(0);
		}
		close(fd);
	}
	printf("Unable to find device on %s\n", argv[1]);
	return(1);
}

