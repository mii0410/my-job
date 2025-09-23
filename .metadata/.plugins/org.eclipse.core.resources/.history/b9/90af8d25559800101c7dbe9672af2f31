/*
 * utils.h
 *
 *  Created on: Jun 22, 2020
 *      Author: acidcube
 */

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif


#include <jendefs.h>


typedef enum {
	   NO_COMMAND,
	   START_COMMAND,
	   COORDINATOR_COMMAND,
	   ROUTER_COMMAND,
	   ENDDEVICE_COMMAND,
	   SEND_COMMAND,
	   RESULTS_COMMAND,
       RX_COMMAND  //’Ç‰ÁƒRƒ}ƒ“ƒh
} commandType;


PUBLIC uint8_t RxByte[128];

PUBLIC void vReadCharInterrupt();
PUBLIC commandType vReadCommand();
PUBLIC int CMD();

// void vReadCommandFromUart(void); // okayama

#if defined __cplusplus
}
#endif

#endif /* UTILS_H_INCLUDED */
