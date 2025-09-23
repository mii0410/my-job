/*****************************************************************************
 *
 * モジュール:JN-AN-1184 ZigBeePro Application Template
 *
 * コンポーネント:app_common.h
 *
 * 概要:アプリケーション共通定義
 *
 *****************************************************************************/

#ifndef APP_COMMON_H_
#define APP_COMMON_H_

/* デバイスごとのインクルード */
#ifdef Coordinator
    #include "App_Coordinator.h"
#endif

#ifdef Router
    #include "App_Router.h"
#endif

#ifdef SleepingEndDevice
    #include "App_SED.h"
#endif

#include "ZQueue.h"
#include "ZTimer.h"
/****************************************************************************/
/***        マクロ定義                                                                                                                                ***/
/****************************************************************************/
//#define POLL_TIME               APP_TIME_MS(1000)
//#define POLL_TIME_FAST          APP_TIME_MS(100)
//#define TEN_HZ_TICK_TIME        APP_TIME_MS(100)

/****************************************************************************/
/***        型定義                                                                                                                                     ***/
/****************************************************************************/
typedef enum
{
    E_STARTUP, /* 0: 起動直後 */
    E_LEAVE_WAIT, /* 1: 離脱要求待ち */
    E_LEAVE_RESET, /* 2: 離脱後のリセット処理 */
    E_NFN_START, /* 3: ネットワーク生成開始 */
    E_DISCOVERY, /* 4: ネットワーク探索 */
    E_NETWORK_FORMATION, /* 5: ネットワーク生成 */
    E_JOINING_NETWORK, /* 6: ネットワーク参加 */
    E_REJOINING, /* 7: ネットワーク再参加 */
    E_NETWORK_INIT, /* 8: ネットワーク初期化 */
    E_RESCAN, /* 9: 再探索処理 */
    E_RUNNING, /* 10: 通常動作 */
    E_ROUTE_RECOVERY, /* 11: ルート再探索待機 (new)*/
    E_ROUTE_REJOIN /* 12: 再参加待機 (new)*/
} teNODE_STATES;

typedef struct
{
    teNODE_STATES   eNodeState;
    teNODE_STATES   eNodePrevState;
}tsDeviceDesc;

/****************************************************************************/
/***       グローバル関数                                                                                                                           ***/
/****************************************************************************/
void APP_vtaskMyEndPoint ( void );
/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern tszQueue zps_msgMlmeDcfmInd;
extern tszQueue zps_msgMcpsDcfm;
extern tszQueue zps_msgMcpsDcfmInd;
extern tszQueue zps_TimeEvents;
extern tszQueue APP_msgMyEndPointEvents;
extern tszQueue APP_msgZpsEvents;
extern uint8 u8App_tmr1sec;
extern void *_stack_low_water_mark;
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

#endif /*APP_COMMON_H_*/
