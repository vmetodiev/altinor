
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define INSPECT_TCP ( 1 )
#define INSPECT_UDP ( 0 )

#define SIGNATURE_LEN ( 14 )
#define ALERT_MSG "EXPLOIT!"
uint8_t signature[] = { 'A', 'M', 'D', '*', 'a', 'm', 'd', 'A', 'M', 'D', '*', 'a', 'm', 'd' };

#endif