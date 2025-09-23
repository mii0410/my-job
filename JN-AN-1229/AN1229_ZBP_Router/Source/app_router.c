/*****************************************************************************
 * モジュール:    JN-AN-1184 ZigBeePro Application Template
 * コンポーネント: app_router.c
 * 概要:       ルータアプリケーション
******************************************************************************/

/****************************************************************************/
/***        インクルードファイル                                                                                                                   ***/
/****************************************************************************/
#include <jendefs.h>
#include <string.h>
#include <dbg.h>
#include <zps_apl_af.h>
#include <zps_apl_aib.h>
#include <pdm.h>
#include <PDM_IDs.h>
#include "app_common.h"
#include "app_router.h"

//追加コード
#include "DBG.h"
#include "dbg_jtag.h"
#include "DBG_Uart.h"
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "Utils.h"
#include "Time.h"
#include "config.h"

// 追加
#include <zps_apl_aps.h>
#include <zps_apl_zdo.h>
#include <zps_nwk_pub.h>
#include "ZTimer.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_APP
	#define TRACE_APP 	FALSE
#else
	#define TRACE_APP 	TRUE
#endif

// 追加
#define ROUTE_MONITOR_INVALID_ADDRESS     0xFFFF
#define ROUTE_MONITOR_RECOVERY_INTERVAL   ZTIMER_TIME_SEC(5)
#define ROUTE_MONITOR_RETRY_INTERVAL      ZTIMER_TIME_SEC(10)
#define ROUTE_MONITOR_REJOIN_INTERVAL     ZTIMER_TIME_SEC(30)
#define ROUTE_MONITOR_MAX_ROUTE_RETRY     3
#define ROUTE_MONITOR_MAX_REJOIN_ATTEMPT  2


/****************************************************************************/
/***        型定義                                                       　　　　　　　　　　　　　　　　　　　　　　　　　　　　　***/
/****************************************************************************/
typedef struct
{
    uint16 u16LastFailedShortAddr;
    uint8 u8LastApsStatus;
    uint8 u8LastNwkStatus;
    uint8 u8RecoveryAttempts;
    uint8 u8RejoinAttempts;
    bool_t bRecoveryNeeded;
    bool_t bRouteDiscoveryInProgress;
    bool_t bTimerArmed;
    bool_t bRejoinScheduled;
    uint32 u32LastDetectionTimeSec;
    uint32 u32LastAttemptTimeSec;
} tsRouteMonitor;

/****************************************************************************/
/***        ローカル関数プロトタイプ                                                                                                           ***/
/****************************************************************************/
PRIVATE void vStartup(void);
PRIVATE void vWaitForNetworkDiscovery(ZPS_tsAfEvent sStackEvent);
PRIVATE void vWaitForNetworkJoin(ZPS_tsAfEvent sStackEvent);
PRIVATE void vHandleStackEvent(ZPS_tsAfEvent sStackEvent);
PRIVATE void vClearDiscNT(void);

// 追加
PRIVATE void vAttemptRouteRecovery(void);
PRIVATE void vAttemptRejoin(void);
PRIVATE void vResetRouteMonitor(void);
PRIVATE void vRecordRouteFailure(uint16 u16Addr, uint8 u8Status, const char *pcReason);
PRIVATE void vLogRouteMonitor(const char *pcPrefix);
PRIVATE void vHandleManualRouteRecoveryRequest(void);
PRIVATE void vShowRouteMonitorStatus(void);

/****************************************************************************/
/***        ローカル変数                                                                                                                            ***/
/****************************************************************************/
PRIVATE tsDeviceDesc s_eDeviceState;

// 追加
PRIVATE tsRouteMonitor s_sRouteMonitor;
PRIVATE uint32 u32SystemTimeSeconds = 0;
PUBLIC uint8_t count1 = 1;

PUBLIC uint8 au8DefaultTCLinkKey[16]    = {0x5a, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6c,
                                           0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x30, 0x39};

/*PRIVATE void vReadInputCommand();
PUBLIC void  APP_vSetCommand(uint8 command);
PUBLIC void SendData();*/

/****************************************************************************/
/***        追加関数okayama                                                ***?                                                                            ***/
/****************************************************************************/
PRIVATE void vReadInputCommand(void)
{
    commandType currentCommand = vReadCommand();

    switch (currentCommand)
    {
        case SEND_COMMAND:
        {
            /* 既存の送信要求はここで処理する */
        }
        break;

        case ROUTE_RECOVER_COMMAND:
        {
            vHandleManualRouteRecoveryRequest();
        }
        break;

        case ROUTE_STATUS_COMMAND:
        {
            vShowRouteMonitorStatus();
        }
        break;

        default:
        {
            /* 受信コマンドが無い場合は何もしない */
        }
        break;
    }
}

PUBLIC void SendData(void)
{
    /* デモ用の送信処理 */
    uint8 u8TransactionSequenceNumber = 0;
    PDUM_thAPduInstance hAPduInst = PDUM_hAPduAllocateAPduInstance(apduZDP);
    uint16 u16Offset = 0;
    uint8 au8Payload[4] = {0};

    au8Payload[0] = 6;
    au8Payload[1] = count1;
    au8Payload[2] = 0x01;
    au8Payload[3] = 0xA6;
    count1++;

    if (hAPduInst == PDUM_INVALID_HANDLE)
    {
        DBG_vPrintf(TRUE, "APP: APDUインスタンスの確保に失敗しました\n");
        return;
    }

    u16Offset += PDUM_u16APduInstanceWriteNBO(hAPduInst, u16Offset, "a", au8Payload);///////////mojibake
    PDUM_eAPduInstanceSetPayloadSize(hAPduInst, u16Offset);

    /* 以下は送信要求のサンプルコード。実際に使用する際は必要な処理を追加する。 */
    ZPS_teStatus eStatus;
    ZPS_teAplAfSecurityMode eSecurityMode = ZPS_E_APL_AF_UNSECURE;

    /* 一例として IEEE アドレス指定で送信する場合の参考コード */
    /*
    uint64 u64UnicastMacAddr  = 0x001BC50122016BD5ULL;
    eStatus = ZPS_eAplAfUnicastIeeeDataReq(
                    hAPduInst,
                    0x1337,
                    0x01,
                    0x01,
                    u64UnicastMacAddr,
                    eSecurityMode,
                    0,
                    &u8TransactionSequenceNumber);
    */

    /* ブロードキャスト送信のサンプル */
    /*
    eStatus = ZPS_eAplAfBroadcastDataReq(
                    hAPduInst,
                    0x1337,
                    0x01,
                    0x01,
                    ZPS_E_BROADCAST_ZC_ZR,
                    eSecurityMode,
                    0,
                    &u8TransactionSequenceNumber);
    */
}

////追加関数
//PUBLIC void  APP_vSetCommand(uint8 command)
//{
// switch(command)
// {
// case 2:
// {
//	 DBG_vPrintf(TRUE, "inputcommand : %d \n", command);
//	 break;
// }
//
// }
//}


//PRIVATE void vReadInputCommand()
//{
//  commandType currentCommand = NO_COMMAND;
// // currentCommand = vReadCommand (); //Utils.cの関数
//
//  if (currentCommand == SEND_COMMAND)
//     {
//
// 	  //SendData();//データ送信の関数
// 	  currentCommand = NO_COMMAND;
//     }
//}


//PUBLIC void SendData(){
//	  /*ここから追加コード*/
//
//	 	         uint8 u8TransactionSequenceNumber;
//	             ZPS_tsNwkNib * thisNib;
//	             thisNib = ZPS_psNwkNibGetHandle(ZPS_pvAplZdoGetNwkHandle());
//	             PDUM_thAPduInstance hAPduInst;
//	             hAPduInst = PDUM_hAPduAllocateAPduInstance(apduZDP);
//	             uint16 u16Offset = 0;
//	             uint16 i;
//
//	             uint8_t data[3];
//	             memset(data, 0, sizeof(data));
//	             data[0] = 6;
//	             data[1] = count1;
//	             data[2] = 0x01;
//	             data[3] = 0xa6;
//	             count1++;
//	             u16Offset += PDUM_u16APduInstanceWriteNBO(hAPduInst, u16Offset, "a\x10", data);
//
//	            PDUM_eAPduInstanceSetPayloadSize(hAPduInst, u16Offset);
//
//
//                if (hAPduInst == PDUM_INVALID_HANDLE)
//	              {
//	                   DBG_vPrintf(TRUE, "PDUM_INVALID_HANDLE\n");
//	              } else {
//
//	                   ZPS_teStatus eStatus;
//	                   ZPS_teAplAfSecurityMode  eSecurityMode = (ZPS_E_APL_AF_UNSECURE);//セキュリティ無効
//
//	    /*    uint64 unicastMacAddr  = 0x001BC50122016BD5;
//	        eStatus=ZPS_eAplAfUnicastIeeeDataReq( //ユニキャスト通信
//	                 		  hAPduInst,
//	                           0x1337,
//	                           0x01,
//	                           0x01,
//	                           unicastMacAddr,  //Dest: 64bit-coordinator
//	                           eSecurityMode,
//	                           0,
//	                           &u8TransactionSequenceNumber
//	                           );
//*/
//
//	    /*               eStatus = ZPS_eAplAfBroadcastDataReq( //ブロードキャスト通信
//	                                                              hAPduInst,
//	                 		                                     0x1337,
//	                 		                                     0x01,
//	                 		                                     0x01,
//	                                                              ZPS_E_BROADCAST_ZC_ZR,  //Dest: All Coordinator & Routers
//	                                                              eSecurityMode,
//	                                                              0,
//	                                                              &u8TransactionSequenceNumber
//	                                                              );
//	       */
//
//	               }
//	              //currentCommand = NO_COMMAND;
//      // タイマーを再スタートして5秒後に再度SendDataを呼び出す
//      //ZTIMER_eStart(u8SecondTimer, ZTIMER_TIME_MSEC(5000));
//
//	   /*ここまで*/
//}


/****************************************************************************/
/***        グローバル関数                                                                                                                         ***/
/****************************************************************************/

/****************************************************************************
 *
 * 関数名: APP_vInitialiseRouter
 *
 * 概要: ルータアプリケーション全体を初期化し、データを復元する
 *
 * 戻り値: void
 *
 ****************************************************************************/
PUBLIC void APP_vInitialiseRouter(void)
{
    // 追加
    vResetRouteMonitor();
    u32SystemTimeSeconds = 0;


	bool_t bDeleteRecords = TRUE /*FALSE*/;
    uint16 u16DataBytesRead;

    /* 必要に応じてネットワークコンテキストをフラッシュから削除する。
     * たとえばリセット時にボタンが押されているかを判定し、押下時のみ
     * PDM に全レコード削除を要求するような利用が想定される。
     * 永続化が不要な場合は常に PDM_vDeleteAllDataRecords() を呼び出してもよい。
     */
    if (bDeleteRecords)
    {
        DBG_vPrintf(TRACE_APP, "APP: フラッシュからすべてのレコードを削除します\n");
        PDM_vDeleteAllDataRecords();
    }

    /* 過去にフラッシュへ保存したアプリケーションデータを復元する。
     * ZPS_eAplAfInit を呼び出す前に必要なレコードをすべて読み込んでおく。
     */
    s_eDeviceState.eNodeState = E_STARTUP;
    PDM_eReadDataFromRecord(PDM_ID_APP_ROUTER,
                    		&s_eDeviceState,
                    		sizeof(s_eDeviceState),
                    		&u16DataBytesRead);

    /* ZBPro スタックの初期化 */
    ZPS_eAplAfInit();
    ZPS_vAplSecSetInitialSecurityState(ZPS_ZDO_PRECONFIGURED_LINK_KEY,
                                       au8DefaultTCLinkKey,
                                       0x00,
                                       ZPS_APS_GLOBAL_LINK_KEY);
    /* 必要なソフトウェアモジュールの初期化をここで行う */

    /* アプリケーションが利用する周辺機器の初期化もここで実施する */

    /* フラッシュから復元した結果、ネットワーク参加後の状態であれば
     * スタックを再始動してアプリケーションを再開する。
     * 状態が複数存在する場合は必要に応じて条件分岐を拡張する。
     */
    if (E_RUNNING ==s_eDeviceState.eNodeState)
    {
        ZPS_teStatus eStatus = ZPS_eAplZdoStartStack();
        DBG_vPrintf(TRACE_APP, "APP: スタックを再始動します\r\n");
        if (ZPS_E_SUCCESS != eStatus)
        {
            DBG_vPrintf(TRACE_APP, "APP: ZPS_eZdoStartStack() が失敗しました エラー=%d", eStatus);
        }
        /* 参加要求を許可 */
        ZPS_eAplZdoPermitJoining(0xff);

        /* その他のアプリケーションモジュールも再開する場合はここで実施する */
    }
    else
    {
    	/* ネットワーク形成中にリセットされた場合でも開始状態へ戻す */
       s_eDeviceState.eNodeState = E_STARTUP;
    }
}

/****************************************************************************
 *
 * 関数名: APP_vtaskRouter
 *
 * 概要: ルータのメインステートマシンを処理する
 *
 * 戻り値: void
 *
 ****************************************************************************/
PUBLIC void APP_vtaskRouter(void)
{
    ZPS_tsAfEvent sStackEvent;

    sStackEvent.eType = ZPS_EVENT_NONE;
    if (ZQ_bQueueReceive(&APP_msgZpsEvents, &sStackEvent) )
    {
        DBG_vPrintf(TRACE_APP, "処理対象のイベントがありません\n");
    }

    if (ZTIMER_eGetState(u8App_tmr1sec) == E_ZTIMER_STATE_EXPIRED)
    {
    	/* 1 秒タイマ満了時に内部時計を更新 */
    	ZTIMER_eStart(u8App_tmr1sec, ZTIMER_TIME_SEC(1));
    	u32SystemTimeSeconds++;
    }

    switch (s_eDeviceState.eNodeState)
    {
        case E_STARTUP:
        {
            vStartup();
        }
        break;

        case E_DISCOVERY:
        {
            vWaitForNetworkDiscovery(sStackEvent);
        }
        break;

        case E_JOINING_NETWORK:
        {
        	vWaitForNetworkJoin(sStackEvent);
        }
        break;

        case E_RUNNING:
        {
        	vHandleStackEvent(sStackEvent);
        	vReadInputCommand(); //追加関数

        	// 追加
        	if (s_sRouteMonitor.bRecoveryNeeded && !s_sRouteMonitor.bRejoinScheduled)
        	{
        		DBG_vPrintf(TRUE, "APP: ルート再探索状態へ遷移\n");
        	    s_eDeviceState.eNodeState = E_ROUTE_RECOVERY;
        	}
        }
        break;

        // 追加
        case E_ROUTE_RECOVERY:
        {
        	vHandleStackEvent(sStackEvent);
            vAttemptRouteRecovery();
            vReadInputCommand();
        }
        break;

        // 追加
        case E_ROUTE_REJOIN:
        {
            vHandleStackEvent(sStackEvent);
            vAttemptRejoin();
            vReadInputCommand();
        }
        break;

        default:
        {
            /* その他の状態では特別な処理は行わない */
        }
        break;
    }
}

/****************************************************************************/
/***        ローカル関数                                                                                                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * 関数名: vStartup
 *
 * 概要: ネットワーク探索を開始する
 *
 * 戻り値: void
 *
 ****************************************************************************/
PRIVATE void vStartup(void)
{
    PRIVATE uint8 u8Channel = 11;

    vClearDiscNT();

    /* スキャンするチャネルを設定しスタックを起動 */
    ZPS_psAplAibGetAib()->pau32ApsChannelMask[0] = 1 << u8Channel;

    ZPS_teStatus eStatus = ZPS_eAplZdoStartStack();
    if (ZPS_E_SUCCESS == eStatus)
    {
       s_eDeviceState.eNodeState = E_DISCOVERY;

       /* 次に探索するチャネルへ進める */
       u8Channel++;

       if (27 == u8Channel)
       {
           u8Channel = 11;
       }
    }
}

/****************************************************************************
 *
 * 関数名: vWaitForNetworkDiscovery
 *
 * 概要:ネットワーク探索中のスタックイベントを確認し必要な処理を行う
 *
 * 引数:         名前           		 RW  用途
 *               sStackEvent     R   スタックイベントの詳細
 * 戻り値: void
 *
 ****************************************************************************/
PRIVATE void vWaitForNetworkDiscovery(ZPS_tsAfEvent sStackEvent)
{
    /* ノードがネットワーク探索を完了するまで待機 */
    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        if (ZPS_EVENT_NWK_DISCOVERY_COMPLETE == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Network discovery complete\n");

            if ((ZPS_E_SUCCESS == sStackEvent.uEvent.sNwkDiscoveryEvent.eStatus) ||
                (ZPS_NWK_ENUM_NEIGHBOR_TABLE_FULL == sStackEvent.uEvent.sNwkDiscoveryEvent.eStatus))
            {
                DBG_vPrintf(TRACE_APP, "APP: Found %d networks\n", sStackEvent.uEvent.sNwkDiscoveryEvent.u8NetworkCount);
            }
            else
            {
                DBG_vPrintf(TRACE_APP, "APP: Network discovery failed with error %d\n",sStackEvent.uEvent.sNwkDiscoveryEvent.eStatus);
            }
            if ((0 == sStackEvent.uEvent.sNwkDiscoveryEvent.u8NetworkCount) ||
                (0xff == sStackEvent.uEvent.sNwkDiscoveryEvent.u8SelectedNetwork))
            {
                if (0 != sStackEvent.uEvent.sNwkDiscoveryEvent.u32UnscannedChannels)
                {
                	/* 未探索チャネルが残っている場合は探索を継続 */
                    DBG_vPrintf(TRACE_APP, "APP: No networks found, continue scanning ...\n");
                    ZPS_eAplZdoDiscoverNetworks(sStackEvent.uEvent.sNwkDiscoveryEvent.u32UnscannedChannels);
                }
                else
                {
                    DBG_vPrintf(TRACE_APP, "APP: Exhausted channels to scan\n");
                    s_eDeviceState.eNodeState = E_STARTUP;
                }
            }
            else
            {
                ZPS_tsNwkNetworkDescr *psNwkDescr = &sStackEvent.uEvent.sNwkDiscoveryEvent.psNwkDescriptors[sStackEvent.uEvent.sNwkDiscoveryEvent.u8SelectedNetwork];
                ZPS_teStatus eStatus;

                DBG_vPrintf(TRACE_APP, "APP: Unscanned channels %08x\n", sStackEvent.uEvent.sNwkDiscoveryEvent.u32UnscannedChannels);
                DBG_vPrintf(TRACE_APP, "\tAPP: Ext PAN ID = %016llx\n", psNwkDescr->u64ExtPanId);
                DBG_vPrintf(TRACE_APP, "\tAPP: Channel = %d\n", psNwkDescr->u8LogicalChan);
                DBG_vPrintf(TRACE_APP, "\tAPP: Stack Profile = %d\n", psNwkDescr->u8StackProfile);
                DBG_vPrintf(TRACE_APP, "\tAPP: Zigbee Version = %d\n", psNwkDescr->u8ZigBeeVersion);
                DBG_vPrintf(TRACE_APP, "\tAPP: Permit Joining = %d\n", psNwkDescr->u8PermitJoining);
                DBG_vPrintf(TRACE_APP, "\tAPP: Router Capacity = %d\n", psNwkDescr->u8RouterCapacity);
                DBG_vPrintf(TRACE_APP, "\tAPP: End Device Capacity = %d\n", psNwkDescr->u8EndDeviceCapacity);

                eStatus = ZPS_eAplZdoJoinNetwork(psNwkDescr);
                DBG_vPrintf(TRACE_APP, "APP: Trying Joining network\n");

                if (ZPS_E_SUCCESS == eStatus)
                {
                	DBG_vPrintf(TRACE_APP, "APP: Joining network\n");
                    s_eDeviceState.eNodeState = E_JOINING_NETWORK;
                }
                else
                {
                	/* 再度スキャンを開始 */
                    DBG_vPrintf(TRACE_APP, "APP: Failed to join network reason = %02x\n", eStatus);
                    s_eDeviceState.eNodeState = E_STARTUP;
                }
            }
        }
        else if (ZPS_EVENT_NWK_FAILED_TO_JOIN == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Network discovery failed error=%d\n", sStackEvent.uEvent.sNwkJoinFailedEvent.u8Status);
           s_eDeviceState.eNodeState = E_STARTUP;
        }
        else if (ZPS_EVENT_NWK_JOINED_AS_ROUTER == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Node rejoined network with Addr 0x%04x\n",
                        sStackEvent.uEvent.sNwkJoinedEvent.u16Addr);
            s_eDeviceState.eNodeState = E_RUNNING;

            vResetRouteMonitor();
            s_sRouteMonitor.bTimerArmed = FALSE;

            /* アプリケーション状態をフラッシュへ保存する。
             * PDM_vSaveRecord() 呼び出し時に空き領域が不足すると、PDM は全レコードを消去・再書込みする点に注意。
             */
            PDM_eSaveRecordData(PDM_ID_APP_ROUTER,
            					&s_eDeviceState,
            					sizeof(s_eDeviceState));
        }
        else
        {
            DBG_vPrintf(TRACE_APP, "APP: Unexpected event in E_NETWORK_DISCOVERY - %d\n", sStackEvent.eType);
        }
    }
}

/****************************************************************************
 *
 * 関数名: vWaitForNetworkJoin
 *
 * 概要: ネットワーク参加処理中のスタックイベントを監視する
 *
 * 引数:         名前            		 RW  用途
 *               sStackEvent     R   スタックイベントの詳細
 * 戻り値:void
 *
 *****************************************************************************/
PRIVATE void vWaitForNetworkJoin(ZPS_tsAfEvent sStackEvent)
{
	uint64 u64ExtPANID;
    /* ノードがネットワーク参加を完了するまで待機 */
    if (ZPS_EVENT_NONE != sStackEvent.eType)
    {
        if (ZPS_EVENT_NWK_JOINED_AS_ROUTER == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Node joined network with Addr 0x%04x\n",
                        sStackEvent.uEvent.sNwkJoinedEvent.u16Addr);
            /* 再参加用に EPID を保存  */
            u64ExtPANID = ZPS_u64NwkNibGetEpid(ZPS_pvAplZdoGetNwkHandle());
            ZPS_eAplAibSetApsUseExtendedPanId(u64ExtPANID);
            s_eDeviceState.eNodeState = E_RUNNING;

            // 追加
            vResetRouteMonitor();
            s_sRouteMonitor.bTimerArmed = FALSE;

            PDM_eSaveRecordData(PDM_ID_APP_ROUTER,
            					&s_eDeviceState,
                            	sizeof(s_eDeviceState));

            /* 1 秒タイマを起動し APP_vtaskRouter を定期実行 */
            ZTIMER_eStart (u8App_tmr1sec, ZTIMER_TIME_SEC(1));
        }
        else if (ZPS_EVENT_NWK_FAILED_TO_JOIN == sStackEvent.eType)
        {
            DBG_vPrintf(TRACE_APP, "APP: Node failed to join network. Retrying ...\n");
            s_eDeviceState.eNodeState = E_STARTUP;
            /* 必要であればここでアプリケーション状態をフラッシュへ保存する。
             * PDM_vSaveRecord() 呼び出し時は全レコードが再書込みされる可能性がある点に注意。
             */
        }
        else
        {
            DBG_vPrintf(TRACE_APP, "APP: Unexpected event in E_NETWORK_JOIN - %d\n", sStackEvent.eType);
        }
    }
}

/****************************************************************************
 *
 * 関数名: vHandleStackEvent
 *
 * 概要: ネットワーク参加後に発生するスタックイベントを処理する
 *
 * 引数:         名前            		　RW  用途
 *               sStackEvent     R   スタックイベントの詳細
 * 戻り値: void
 *
 ****************************************************************************/
PRIVATE void vHandleStackEvent(ZPS_tsAfEvent sStackEvent)
{
	if (ZPS_EVENT_NONE == sStackEvent.eType)
	{
		return;
	}

	switch (sStackEvent.eType)
	{
	case ZPS_EVENT_APS_DATA_INDICATION:
		/* 受信した APDU は即座に解放する */
		PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    break;

    case ZPS_EVENT_APS_DATA_CONFIRM:
    {
        uint8 u8Status = sStackEvent.uEvent.sApsDataConfirmEvent.u8Status;
        uint16 u16Dst = sStackEvent.uEvent.sApsDataConfirmEvent.uDstAddr.u16Addr;
        DBG_vPrintf(TRACE_APP,
                    "APP: vCheckStackEvent: ZPS_EVENT_APS_DATA_CONFIRM Status %d, Address 0x%04x\n",
                    u8Status,
                    u16Dst);
        if ((u8Status == ZPS_APL_APS_E_NO_ROUTE_AVAILABLE) ||
            (u8Status == ZPS_APL_APS_E_ROUTE_DISCOVERY_FAILED))
        {
        	vRecordRouteFailure(u16Dst, u8Status, "APSデータ送信失敗");
            if (s_eDeviceState.eNodeState == E_RUNNING)
            {
                s_eDeviceState.eNodeState = E_ROUTE_RECOVERY;
            }
        }
    }
    break;

    case ZPS_EVENT_APS_DATA_ACK:
    {
        uint8 u8Status = sStackEvent.uEvent.sApsDataAckEvent.u8Status;
        uint16 u16Dst = sStackEvent.uEvent.sApsDataAckEvent.u16DstAddr;

        DBG_vPrintf(TRACE_APP,
                    "APP: vCheckStackEvent: ZPS_EVENT_APS_DATA_ACK Status %d, Address 0x%04x\n",
                    u8Status,
                    u16Dst);
        if (u8Status == ZPS_E_SUCCESS)
        {
        	 vResetRouteMonitor();
        	 if (s_eDeviceState.eNodeState != E_RUNNING)
        	 {
        		 s_eDeviceState.eNodeState = E_RUNNING;
        	 }
        }
        else
        {
        	vRecordRouteFailure(u16Dst, u8Status, "APS ACK ERROR");
        }
    }
    break;

    case ZPS_EVENT_NWK_NEW_NODE_HAS_JOINED:
    {
        DBG_vPrintf(TRACE_APP,
        			"APP: vCheckStackEvent: vCheckStackEvent: ZPS_EVENT_NEW_NODE_HAS_JOINED, Nwk Addr=0x%04x\n",
                    sStackEvent.uEvent.sNwkJoinIndicationEvent.u16NwkAddr);
    }
    break;

    case ZPS_EVENT_NWK_LEAVE_INDICATION:
    {
        DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_LEAVE_INDICATION\n");
    }
    break;

    case ZPS_EVENT_NWK_LEAVE_CONFIRM:
    {
        DBG_vPrintf(TRACE_APP, "APP: ノード離脱確認\n");
    }
    break;

    case ZPS_EVENT_NWK_STATUS_INDICATION:
    {
        uint8 u8Status = sStackEvent.uEvent.sNwkStatusIndicationEvent.u8Status;
        s_sRouteMonitor.u8LastNwkStatus = u8Status;

        DBG_vPrintf(TRACE_APP,
                    "APP: NWKステータス通知 status=%d\n",
                    u8Status);

        if (u8Status != ZPS_NWK_ENUM_SUCCESS)
        {
            vRecordRouteFailure(s_sRouteMonitor.u16LastFailedShortAddr,
                                u8Status,
                                "NWKステータス通知");
        }
    }
    break;

    case ZPS_EVENT_NWK_ROUTE_DISCOVERY_CONFIRM:
    {
        ZPS_teNwkEnum eStatus = sStackEvent.uEvent.sNwkRouteDiscoveryConfirmEvent.eStatus;
        uint16 u16Dst = sStackEvent.uEvent.sNwkRouteDiscoveryConfirmEvent.u16RouteDst;
        s_sRouteMonitor.u8LastNwkStatus = (uint8)eStatus;

        DBG_vPrintf(TRACE_APP,
                    "APP: ルート探索結果 status=%d 宛先=0x%04x\n",
                    eStatus,
                    u16Dst);

        if (eStatus == ZPS_NWK_ENUM_SUCCESS){
            DBG_vPrintf(TRUE, "APP: ルート再探索が成功しました\n");
            vResetRouteMonitor();
            s_eDeviceState.eNodeState = E_RUNNING;
        }
        else
        {
            vRecordRouteFailure(u16Dst, (uint8)eStatus, "ルート探索失敗");
            if (s_sRouteMonitor.u8RecoveryAttempts >= ROUTE_MONITOR_MAX_ROUTE_RETRY)
            {
                s_sRouteMonitor.bRejoinScheduled = TRUE;
                s_eDeviceState.eNodeState = E_ROUTE_REJOIN;
                ZTIMER_eStop(u8RouteRecoveryTimer);
                ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_REJOIN_INTERVAL);
                s_sRouteMonitor.bTimerArmed = TRUE;
            }
        }
    }
    break;

    case ZPS_EVENT_ERROR:
    {

    	DBG_vPrintf(TRACE_APP, "APP: Monitor Sensors ZPS_EVENT_ERROR\n");
        DBG_vPrintf(TRACE_APP, "Error Code %d\n\n", sStackEvent.uEvent.sAfErrorEvent.eError);

        if (ZPS_ERROR_OS_MESSAGE_QUEUE_OVERRUN == sStackEvent.uEvent.sAfErrorEvent.eError)
        {
            DBG_vPrintf(TRACE_APP,
                        "Queue handle %d\n",
                        sStackEvent.uEvent.sAfErrorEvent.uErrorData.sAfErrorOsMessageOverrun.hMessage);
        }
    }
    break;

    case ZPS_EVENT_NWK_POLL_CONFIRM:
    {
        DBG_vPrintf(TRACE_APP,
                    "APP: vCheckStackEvent: ZPS_EVENT_NEW_POLL_COMPLETE, status = %d\n",
                    sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status);
    }
    break;

    default:
    {
    	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: unhandled event %d\n", sStackEvent.eType);
    }
    break;
	}
}






//
//    if (ZPS_EVENT_NONE != sStackEvent.eType)
//    {
//        switch (sStackEvent.eType)
//        {
//            case ZPS_EVENT_APS_DATA_INDICATION:
//            {
//                //DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_AF_DATA_INDICATION\n");
//
//                /* Process incoming cluster messages ... */
//                //DBG_vPrintf(TRACE_APP, "        Profile :%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u16ProfileId);
//                //DBG_vPrintf(TRACE_APP, "        Cluster :%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u16ClusterId);
//                //DBG_vPrintf(TRACE_APP, "        EndPoint:%x\r\n",sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint);
//
//                /* free the application protocol data unit (APDU) once it has been dealt with */
//                PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
//            }
//            break;
//
//            case ZPS_EVENT_APS_DATA_CONFIRM:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_APS_DATA_CONFIRM Status %d, Address 0x%04x\n",
//                            sStackEvent.uEvent.sApsDataConfirmEvent.u8Status,
//                            sStackEvent.uEvent.sApsDataConfirmEvent.uDstAddr.u16Addr);
//            }
//            break;
//
//            case ZPS_EVENT_APS_DATA_ACK:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_APS_DATA_ACK Status %d, Address 0x%04x\n",
//                            sStackEvent.uEvent.sApsDataAckEvent.u8Status,
//                            sStackEvent.uEvent.sApsDataAckEvent.u16DstAddr);
//            }
//            break;
//
//            case ZPS_EVENT_NWK_NEW_NODE_HAS_JOINED:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: vCheckStackEvent: ZPS_EVENT_NEW_NODE_HAS_JOINED, Nwk Addr=0x%04x\n",
//                            sStackEvent.uEvent.sNwkJoinIndicationEvent.u16NwkAddr);
//            }
//            break;
//
//            case ZPS_EVENT_NWK_LEAVE_INDICATION:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_LEAVE_INDICATION\n");
//            }
//            break;
//
//            case ZPS_EVENT_NWK_LEAVE_CONFIRM:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_LEAVE_CONFIRM\n");
//            }
//            break;
//
//            case ZPS_EVENT_NWK_STATUS_INDICATION:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_NWK_STATUS_INDICATION\n");
//            }
//            break;
//
//            case ZPS_EVENT_NWK_ROUTE_DISCOVERY_CONFIRM:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_ROUTE_DISCOVERY_CFM\n");
//            }
//            break;
//
//            case ZPS_EVENT_ERROR:
//            {
//            	DBG_vPrintf(TRACE_APP, "APP: Monitor Sensors ZPS_EVENT_ERROR\n");
//                DBG_vPrintf(TRACE_APP, "    Error Code %d\n", sStackEvent.uEvent.sAfErrorEvent.eError);
//
//                if (ZPS_ERROR_OS_MESSAGE_QUEUE_OVERRUN == sStackEvent.uEvent.sAfErrorEvent.eError)
//                {
//                	DBG_vPrintf(TRACE_APP, "    Queue handle %d\n", sStackEvent.uEvent.sAfErrorEvent.uErrorData.sAfErrorOsMessageOverrun.hMessage);
//                }
//            }
//            break;
//
//            case ZPS_EVENT_NWK_POLL_CONFIRM:
//            {
//                DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: ZPS_EVENT_NEW_POLL_COMPLETE, status = %d\n",
//                           sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status );
//            }
//            break;
//
//            case ZPS_EVENT_NWK_JOINED_AS_ROUTER:
//            case ZPS_EVENT_NWK_STARTED:
//            case ZPS_EVENT_NWK_FAILED_TO_START:
//            case ZPS_EVENT_NWK_FAILED_TO_JOIN:
//            case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
//                /* Deliberate fall through */
//            default:
//            {
//                DBG_vPrintf(TRACE_APP, "APP: vCheckStackEvent: unhandled event %d\n", sStackEvent.eType);
//            }
//            break;
//        }
//    }
//}

/****************************************************************************
***           新規追加関数                                                                                                                    ***
****************************************************************************/
PRIVATE void vResetRouteMonitor(void)
{
    ZTIMER_eStop(u8RouteRecoveryTimer);
    s_sRouteMonitor.u16LastFailedShortAddr = ROUTE_MONITOR_INVALID_ADDRESS;
    s_sRouteMonitor.u8LastApsStatus = 0;
    s_sRouteMonitor.u8LastNwkStatus = 0;
    s_sRouteMonitor.u8RecoveryAttempts = 0;
    s_sRouteMonitor.u8RejoinAttempts = 0;
    s_sRouteMonitor.bRecoveryNeeded = FALSE;
    s_sRouteMonitor.bRouteDiscoveryInProgress = FALSE;
    s_sRouteMonitor.bTimerArmed = FALSE;
    s_sRouteMonitor.bRejoinScheduled = FALSE;
    s_sRouteMonitor.u32LastDetectionTimeSec = u32SystemTimeSeconds;
    s_sRouteMonitor.u32LastAttemptTimeSec = 0;
}

PRIVATE void vLogRouteMonitor(const char *pcPrefix)
{
    DBG_vPrintf(TRUE,
                "APP: [ルート監視] %s 宛先0x%04x APS=0x%02x NWK=0x%02x 再探索回数=%d 再参加回数=%d 最終検知=%lu秒\n",
                pcPrefix,
                s_sRouteMonitor.u16LastFailedShortAddr,
                s_sRouteMonitor.u8LastApsStatus,
                s_sRouteMonitor.u8LastNwkStatus,
                s_sRouteMonitor.u8RecoveryAttempts,
                s_sRouteMonitor.u8RejoinAttempts,
                (unsigned long)s_sRouteMonitor.u32LastDetectionTimeSec);
}

PRIVATE void vRecordRouteFailure(uint16 u16Addr, uint8 u8Status, const char *pcReason)
{
    if (u16Addr != ROUTE_MONITOR_INVALID_ADDRESS)
    {
        s_sRouteMonitor.u16LastFailedShortAddr = u16Addr;
    }

    s_sRouteMonitor.u8LastApsStatus = u8Status;
    s_sRouteMonitor.bRecoveryNeeded = TRUE;
    s_sRouteMonitor.bRouteDiscoveryInProgress = FALSE;
    s_sRouteMonitor.u32LastDetectionTimeSec = u32SystemTimeSeconds;

    if (!s_sRouteMonitor.bTimerArmed)
    {
        ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_RECOVERY_INTERVAL);
        s_sRouteMonitor.bTimerArmed = TRUE;
    }

    vLogRouteMonitor(pcReason);
}

PRIVATE bool_t bStartRouteDiscovery(uint16 u16ShortAddr)
{
#if APP_ROUTE_MONITOR_ENABLE_ROUTE_DISCOVERY
    ZPS_teStatus eStatus = ZPS_eAplZdoRouteDiscoveryRequestDefault(u16ShortAddr);

    if (ZPS_E_SUCCESS != eStatus)
    {
        eStatus = ZPS_eAplZdoRouteDiscoveryRequest(u16ShortAddr, 0, FALSE);
    }

    if (ZPS_E_SUCCESS == eStatus)
    {
        return TRUE;
    }

    DBG_vPrintf(TRUE, "APP: Route discovery request failed status=%d\n", eStatus);
#else
    DBG_vPrintf(TRUE, "APP: Route discovery API disabled - scheduling rejoin\n");
#endif

    return FALSE;
}

PRIVATE void vAttemptRouteRecovery(void)
{
    if (!s_sRouteMonitor.bRecoveryNeeded)
    {
        s_eDeviceState.eNodeState = E_RUNNING;
        return;
    }

    if (s_sRouteMonitor.u16LastFailedShortAddr == ROUTE_MONITOR_INVALID_ADDRESS)
    {
    if (bStartRouteDiscovery(s_sRouteMonitor.u16LastFailedShortAddr))
        vLogRouteMonitor("ĒTs");
        ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_RETRY_INTERVAL);
        s_sRouteMonitor.bTimerArmed = TRUE;
        return;
    DBG_vPrintf(TRUE, "APP: Route discovery unavailable - scheduling rejoin\n");
    s_sRouteMonitor.bRejoinScheduled = TRUE;
    s_eDeviceState.eNodeState = E_ROUTE_REJOIN;
    vLogRouteMonitor("Rejoin");
    ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_REJOIN_INTERVAL);

    }

    if (!s_sRouteMonitor.bTimerArmed)
    {
        ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_RECOVERY_INTERVAL);
        s_sRouteMonitor.bTimerArmed = TRUE;
        return;
    }

    if (ZTIMER_eGetState(u8RouteRecoveryTimer) != E_ZTIMER_STATE_EXPIRED)
    {
        return;
    }

    s_sRouteMonitor.bTimerArmed = FALSE;

    ZPS_teStatus eStatus = ZPS_eAplZdoRouteDiscoveryRequestDefault(s_sRouteMonitor.u16LastFailedShortAddr);

    if (ZPS_E_SUCCESS != eStatus)
    {
        eStatus = ZPS_eAplZdoRouteDiscoveryRequest(s_sRouteMonitor.u16LastFailedShortAddr, 0, FALSE);
    }

    if (ZPS_E_SUCCESS == eStatus)
    {
        s_sRouteMonitor.u8RecoveryAttempts++;
        s_sRouteMonitor.bRouteDiscoveryInProgress = TRUE;
        s_sRouteMonitor.u32LastAttemptTimeSec = u32SystemTimeSeconds;
        vLogRouteMonitor("再探索実行");
    }
    else
    {
        DBG_vPrintf(TRUE, "APP: ルート再探索要求送信失敗 status=%d\n", eStatus);
    }

    ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_RETRY_INTERVAL);
    s_sRouteMonitor.bTimerArmed = TRUE;
}

PRIVATE void vAttemptRejoin(void)
{
    if (!s_sRouteMonitor.bRejoinScheduled)
    {
        s_eDeviceState.eNodeState = E_RUNNING;
        return;
    }

    if (!s_sRouteMonitor.bTimerArmed)
    {
        ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_REJOIN_INTERVAL);
        s_sRouteMonitor.bTimerArmed = TRUE;
        return;
    }

    if (ZTIMER_eGetState(u8RouteRecoveryTimer) != E_ZTIMER_STATE_EXPIRED)
    {
        return;
    }

    s_sRouteMonitor.bTimerArmed = FALSE;

    if (s_sRouteMonitor.u8RejoinAttempts >= ROUTE_MONITOR_MAX_REJOIN_ATTEMPT)
    {
        DBG_vPrintf(TRUE, "APP: 再参加試行回数が上限に達したためスタートアップへ戻ります\n");
        vResetRouteMonitor();
        s_eDeviceState.eNodeState = E_STARTUP;
        return;
    }

    PDM_eSaveRecordData(PDM_ID_APP_ROUTER,
                        &s_eDeviceState,
                        sizeof(s_eDeviceState));

    ZPS_teStatus eStatus = ZPS_eAplZdoRejoinNetwork(FALSE);

    s_sRouteMonitor.u8RejoinAttempts++;
    s_sRouteMonitor.u32LastAttemptTimeSec = u32SystemTimeSeconds;

    DBG_vPrintf(TRUE,
                "APP: 再参加を試行 status=%d 試行回数=%d\n",
                eStatus,
                s_sRouteMonitor.u8RejoinAttempts);

    ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_REJOIN_INTERVAL);
    s_sRouteMonitor.bTimerArmed = TRUE;
}

PRIVATE void vHandleManualRouteRecoveryRequest(void)
{
    if (s_sRouteMonitor.u16LastFailedShortAddr == ROUTE_MONITOR_INVALID_ADDRESS)
    {
        DBG_vPrintf(TRUE, "APP: 監視情報が存在しないため再探索を開始できません\n");
        return;
    }

    DBG_vPrintf(TRUE, "APP: 手動ルート再探索要求を受信\n");

    s_sRouteMonitor.bRecoveryNeeded = TRUE;
    s_sRouteMonitor.bRejoinScheduled = FALSE;
    s_sRouteMonitor.u8RecoveryAttempts = 0;
    s_sRouteMonitor.u8RejoinAttempts = 0;
    s_sRouteMonitor.bRouteDiscoveryInProgress = FALSE;
    s_sRouteMonitor.u32LastDetectionTimeSec = u32SystemTimeSeconds;

    ZTIMER_eStop(u8RouteRecoveryTimer);
    ZTIMER_eStart(u8RouteRecoveryTimer, ROUTE_MONITOR_RECOVERY_INTERVAL);
    s_sRouteMonitor.bTimerArmed = TRUE;
    s_eDeviceState.eNodeState = E_ROUTE_RECOVERY;

    vLogRouteMonitor("手動再探索");
}

PRIVATE void vShowRouteMonitorStatus(void)
{
    vLogRouteMonitor("状態照会");
    DBG_vPrintf(TRUE,
                "APP: [ルート監視] タイマ稼働=%d 再探索必要=%d 再参加待機=%d\n",
                s_sRouteMonitor.bTimerArmed ? 1 : 0,
                s_sRouteMonitor.bRecoveryNeeded ? 1 : 0,
                s_sRouteMonitor.bRejoinScheduled ? 1 : 0);
}

/****************************************************************************
 *
 * NAME: vClearDiscNT
 *
 * DESCRIPTION:
 * Handles the re-intialisation of the discovery table to 0
 *
 * PARAMETERS: None
 *
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vClearDiscNT(void)
{
    ZPS_tsNwkNib * thisNib;

    void * thisNet = ZPS_pvAplZdoGetNwkHandle();
    thisNib = ZPS_psNwkNibGetHandle(thisNet);

    memset(thisNib->sTbl.psNtDisc, 0, sizeof(ZPS_tsNwkDiscNtEntry) * thisNib->sTblSize.u8NtDisc);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
