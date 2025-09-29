/*****************************************************************************
 *
 * MODULE:				JN-AN-1184 ZigBeePro Application Template
 *
 * COMPONENT:			app_endpoint.c
 *
 * DESCRIPTION:			End Point Event Handler
 *
 *****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5169, JN5168,
 * JN5164, JN5161].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2015. All rights reserved
 *
 ****************************************************************************/
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <dbg.h>
#include <pdm.h>
#include <pwrm.h>
#include <zps_apl_af.h>
#include <PDM_IDs.h>
#include "app_common.h"
#include "app_coordinator.h"
#include "ZTimer.h"
#include "ZQueue.h"
//追加
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "DBG.h"
#include "dbg_jtag.h"
#include "DBG_Uart.h"
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "Utils.h"
#include "Time.h"
#include "config.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_APP
	#define TRACE_APP 	FALSE
#else
	#define TRACE_APP 	TRUE
#endif

tszQueue APP_msgMyEndPointEvents;
tszQueue APP_msgZpsEvents;

uint8_t p = 0;

/****************************************************************************
 *
 * NAME: APP_taskEndpoint
 *
 * DESCRIPTION:
 * End Point event handling
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/


void APP_vtaskMyEndPoint (void)
{
    ZPS_tsAfEvent sStackEvent;
    sStackEvent.eType = ZPS_EVENT_NONE;

    /* check if any messages to collect */
    if ( ZQ_bQueueReceive(&APP_msgMyEndPointEvents, &sStackEvent) )
    {
        //DBG_vPrintf(TRACE_APP, "APP: No event to process\n");
    }

    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        switch (sStackEvent.eType)
        {

            case ZPS_EVENT_APS_DATA_INDICATION: //何かしら他端末からのデータを受信した(受信したデータを扱う場合はここの部分を変更させる)
            {

                uint16 u16bytesread;
                uint8_t i = 0;
                uint8_t u8LQI = 0;
                uint8_t addr = 0;
                uint8_t Rxbyte[3];//受信データを格納する配列


                u16bytesread = PDUM_u16APduInstanceReadNBO(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst,0,"a\x10",&Rxbyte);
                addr = sStackEvent.uEvent.sApsDataIndEvent.uSrcAddress.u16Addr;
                u8LQI = sStackEvent.uEvent.sApsDataIndEvent.u8LinkQuality;
                vAHI_UartWriteData(DBG_E_UART_0, Rxbyte[0]); //受信したセンサの識別番号
                vAHI_UartWriteData(DBG_E_UART_0, Rxbyte[1]); //受信した各センサの順番
                vAHI_UartWriteData(DBG_E_UART_0, addr); //受信したデータの送信元のshortアドレス
                vAHI_UartWriteData(DBG_E_UART_0, u8LQI); //受信したデータのLQI値
                vAHI_UartWriteData(DBG_E_UART_0, Rxbyte[2]); //受信したデータ１つ目
                vAHI_UartWriteData(DBG_E_UART_0, Rxbyte[3]);//受信したデータ２つ目
                memset(Rxbyte, 0, sizeof(Rxbyte));


                /* free the application protocol data unit (APDU) once it has been dealt with */
                PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);

            }
            break;

            case ZPS_EVENT_APS_DATA_CONFIRM:
            {
                DBG_vPrintf(TRACE_APP, "APP: APP_taskEndPoint: ZPS_EVENT_APS_DATA_CONFIRM Status %d, Address 0x%04x\n",
                            sStackEvent.uEvent.sApsDataConfirmEvent.u8Status,
                            sStackEvent.uEvent.sApsDataConfirmEvent.uDstAddr.u16Addr);

            }
            break;

            case ZPS_EVENT_APS_DATA_ACK:
            {
            	DBG_vPrintf(TRACE_APP, "APP: APP_taskEndPoint: ZPS_EVENT_APS_DATA_ACK Status %d, Address 0x%04x\n",
                            sStackEvent.uEvent.sApsDataAckEvent.u8Status,
                            sStackEvent.uEvent.sApsDataAckEvent.u16DstAddr);
            }
            break;

            default:
            {
            	DBG_vPrintf(TRACE_APP, "APP: APP_taskEndPoint: unhandled event %d\n", sStackEvent.eType);
            }
            break;
        }
    }
}


/****************************************************************************
 *
 * NAME: APP_vGenCallback
 *
 * DESCRIPTION:
 * Stack callback
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vGenCallback(uint8 u8Endpoint, ZPS_tsAfEvent *psStackEvent)
{
    if ( u8Endpoint == 0 )
    {
    	ZQ_bQueueSend(&APP_msgZpsEvents, (void*) psStackEvent);
    }
    else
    {
    	ZQ_bQueueSend(&APP_msgMyEndPointEvents, (void*) psStackEvent);
    }
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
