#include "hm-trp.h"

#include <stdio.h>

#ifndef CBUFFER_SIZE
#define CBUFFER_SIZE 32
#endif

const int count_devtype = 4;
const char* dev_name[4] = {
	"HM-TRP-433",
	"HM-TRP-470",
	"HM-TRP-868",
	"HM-TRP-915"
};

const uint32_t freq_min[4] = {
	414 * 1e6,
	450 * 1e6,
	849 * 1e6,
	895 * 1e6
};
const uint32_t freq_max[4] = {
	454 * 1e6,
	490 * 1e6,
	889 * 1e6,
	935 * 1e6
};
const uint32_t freq_default[4] = {
	434 * 1e6,
	470 * 1e6,
	869 * 1e6,
	915 * 1e6
};

const int count_recv_bw = 12;
// Values selected somewhat randomly...
const int recv_bw[12] = {
	30,
	50,
	75,
	90,
	105,
	120,
	150,
	175,
	200,
	250,
	300,
	620
};

const int count_freq_dev = 10;
// Values selected somewhat randomly...
const int freq_dev[10] = {  // Values as kHz ..
	10,
	20,
	35,
	50,
	55,
	80,
	110,
	120,
	145,
	160
};

const int count_powerlevel = 8;
const int powerlevel[8] = { 1, 2, 5, 8, 11, 14, 17, 20 };

const int count_rates = 9;
const int port_rate_value[9] = {
	1200,
	1800,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200 
};

const int port_rates[9] = {
	B1200,
	B1800,
	B2400,
	B4800,
	B9600,
	B19200,
	B38400,
	B57600,
	B115200 
};

const char cmd_reset[3]     = { 0xAA, 0xFA, 0xF0 };
const char cmd_config[3]    = { 0xAA, 0xFA, 0xE1 };
const char cmd_frequency[3] = { 0xAA, 0xFA, 0xD2 };
const char cmd_air_rate[3]   = { 0xAA, 0xFA, 0xC3 };
const char cmd_bw[3]        = { 0xAA, 0xFA, 0xB4 };
const char cmd_deviation[3] = { 0xAA, 0xFA, 0xA5 };
const char cmd_power[3]     = { 0xAA, 0xFA, 0x96 };
const char cmd_uart_rate[3] = { 0xAA, 0xFA, 0x1E };
const char cmd_RSSI[3]      = { 0xAA, 0xFA, 0x87 };
const char cmd_SNR[3]       = { 0xAA, 0xFA, 0x78 };

int write_uint32_t ( int f, uint32_t v )
{
	int i;
	unsigned char buf[4];
	v = __bswap_32(v);
	for ( i = 0; i < 4; i++ )
		buf[i] = (v>>(8*i) & 0xff );

	return write( f, buf, 4 );
}

int write_uint16_t ( int f, uint16_t v )
{
	int i;
	unsigned char buf[2];
	v = __bswap_16(v);
	for ( i = 0; i < 2; i++ )
		buf[i] = (v>>(8*i) & 0xff );

	return write( f, buf, 2 );
}

int write_uint8_t ( int f, uint8_t v )
{
	int i;
	unsigned char buf[2];

	//for ( i = 0; i < 2; i++ )
	//	buf[i] = (v>>(8*i) & 0xff );

	return write( f, &v, 1 );
}

int write_cmd ( int f, const char* buf )
{
	return write( f, buf, 3 ); 	// Send the command to the device
}

int read_config ( int fd, config_t * config )
{

	unsigned char buf[CBUFFER_SIZE];
	int res;

	res = write_cmd( fd, cmd_config ); 	// Send the command to the device
	if ( res < 0 ) return(-1); 		// and die on failure..

	int i = 0;
	uint8_t* tmp_config = (uint8_t*)config;
	do {
		bzero(buf, CBUFFER_SIZE);
		res = read( fd, buf, 1 );

		if ( res )
		{
			*tmp_config++ = (uint8_t)buf[0];
			// Make sure wo don't overflow the config struct.
			if ( (void*)tmp_config > ((void*)config+sizeof(config_t)) ) return(-2);
		}

	} while ( res > 0 );

	if ( res < 0 )
	{
		return -1;
	}

	if ( config->freq ) 
	{
		config->freq      = __bswap_32( config->freq );
		config->air_rate  = __bswap_32( config->air_rate );
		config->bw        = __bswap_16( config->bw );
		config->uart_rate = __bswap_32( config->uart_rate );
		return 1;
	}
	
	return 0; // No errors, but no valid config
}

int read_ok ( int fd )
{
	unsigned char ok[5];
	int i = 0;
	unsigned char c;
	bzero(ok, 5);

	while( (read( fd, &c, 1 ) > 0 ) && ( i < 4) ) ok[i++] = c;
	if ( strcmp( ok, "OK\r\n" ) == 0 ) return 1;
	return 0;
}
