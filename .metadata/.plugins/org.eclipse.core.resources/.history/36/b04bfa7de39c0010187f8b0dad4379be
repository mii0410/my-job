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
           RX_COMMAND, /* UART 受信コマンド */
           ROUTE_RECOVER_COMMAND, /* ルート再探索要求 */
           ROUTE_STATUS_COMMAND /* ルート監視情報表示 */
} commandType;


PUBLIC uint8_t RxByte[128];

PUBLIC void vReadCharInterrupt(void);
PUBLIC commandType vReadCommand(void);
PUBLIC int CMD(void);

// void vReadCommandFromUart(void); // okayama

#if defined __cplusplus
}
#endif

#endif /* UTILS_H_INCLUDED */
