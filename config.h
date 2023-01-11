
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

//
// User config "section"
// 

#define ATTEMPT_ZERO_COPY_OPTION ( 0 )

#define INSPECT_TCP ( 1 )
#define INSPECT_UDP ( 0 )

#define SIGNATURE_LEN ( 14 )
#define ALERT_MSG "EXPLOIT!"
uint8_t signature[] = { 'E', 'x', 'p', 'l', 'o', 'i', 't', 'B', 'y', 't', 'e', 's', '0', 'P' };

//
// End of user user config "section"
//

#endif