#ifndef _HM_TRP_H
#define _HM_TRP_H

#include <string.h>  /* String function definitions */
#include <stdint.h>  /* uint*_t types */
#include <byteswap.h> /* __bwap_{16,32} */
#include "serial.h"

typedef struct {
	uint32_t freq;
	uint32_t air_rate;
	uint16_t bw;
	uint8_t  deviation;
	uint8_t  power;
	uint32_t uart_rate;
} config_t;

extern const int count_devtype;
extern const char* dev_name[4];

extern const uint32_t freq_min[4];
extern const uint32_t freq_max[4];
extern const uint32_t freq_default[4];

extern const int count_recv_bw;
extern const int recv_bw[12];

extern const int count_freq_dev;
extern const int freq_dev[10];

extern const int count_powerlevel;
extern const int powerlevel[8];

extern const int count_rates;
extern const int port_rate_value[9];
extern const int port_rates[9];

extern const char cmd_reset[3];
extern const char cmd_config[3];
extern const char cmd_frequency[3];
extern const char cmd_air_rate[3];
extern const char cmd_bw[3];
extern const char cmd_deviation[3];
extern const char cmd_power[3];
extern const char cmd_uart_rate[3];
extern const char cmd_RSSI[3];
extern const char cmd_SNR[3];

int write_uint32_t ( int f, uint32_t v );
int write_uint16_t ( int f, uint16_t v );
int write_uint8_t ( int f, uint8_t v );
int write_cmd ( int f, const char* buf );
int read_config ( int fd, config_t * config );

int read_ok( int fd );

#endif /* _HM_TRP_H */
