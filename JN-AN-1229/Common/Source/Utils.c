/*
 * common.c
 *
 *  Created on: Jun 22, 2020
 *      Author: Alberto Gallegos Ramonet
 */

#include "Utils.h"
#include "DBG.h"
#include "dbg_jtag.h"
#include "DBG_Uart.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define COMMAND_SIZE 64

/* UART から受信したコマンド文字列 */
static char command[COMMAND_SIZE];
/* 入力中の文字数を管理するカウンタ */
static uint8 cmd_index = 0;

PUBLIC uint8_t RxByte[128];
static int cmd = 0;

/*---------------------------------------------------------------------------
 * 名称: CMD
 * 概要: 旧来の SendData 呼び出し用フラグを取得する
 * 引数: なし
 * 戻り値: フラグ値
 *---------------------------------------------------------------------------*/
PUBLIC int CMD(void)
{
    if (cmd == 3)
    {
        cmd = 0;
        return 3;
    }
    return 0;
}

/*---------------------------------------------------------------------------
 * 名称: vResetCommandBuffer
 * 概要: コマンドバッファの内容を初期化する
 * 引数: なし
 * 戻り値: なし
 *---------------------------------------------------------------------------*/
static void vResetCommandBuffer(void)
{
    memset(command, 0, sizeof(command));
    cmd_index = 0;
}

//PUBLIC uint8_t RxByte[128];
//int  i = 0;
//int l=0;
//
//
//int cmd = 0;
//int c =0;
//
//typedef enum
//{
//    START,
//    COMMAND,
//    STATUS,
//    DATA_LENGTH_L,
//    DATA_LENGTH_H,
//    CO2_DATA,
//    PRIMITIVE_TYPE,
//    COUNT,
//    WAIT_DATA
//}State;
//int state = START;
//
//
//PUBLIC int CMD(){ //送信する関数(SendData関数)を制御する．cmdが3でSendData関数を実行する．
//  if(cmd == 3){
//	  cmd = 0;
// 	return 3;
//  }else{
//	return 0;
//  }
//}

//void vReadCharInterrupt ()//円筒形容器と合体している温度センサ
//{
// uint8_t rxByte =  u8AHI_UartReadData (DBG_E_UART_0);
// switch(state){
//
//case START:
//	  if(rxByte == 0xAA){ //データの先頭を表す文字
//		  state = PRIMITIVE_TYPE;
//		  break;
//	  }
//	  break;
//  case PRIMITIVE_TYPE:
//	  if(rxByte == 2){ //センサの種類(センサごとに値を変化させる)
//		  RxByte[i] = rxByte;
//		  i++;
//		  state = COUNT;
//		  break;
//	  }
//	  break;
//  case COUNT:
//	  if(rxByte >= 1){  //各センサの通し番号
//	  		  RxByte[i] = rxByte;
//	  		  i++;
//	  		  state = WAIT_DATA;
//	  		  break;
//	  	  }
//	  break;
//  case WAIT_DATA:
//	  RxByte[i] = rxByte; //センサデータ
//	  i++;
//	  if(i == 4){
//	  		  cmd = 3;
//	  		  i = 0;
//	  		  state = START;
//	  }
//	   	  break;
//  default:
//	 break;
//
//	 }
//
//}

// okayama
/*---------------------------------------------------------------------------
 * 名称: vReadCharInterrupt
 * 概要: UART 割り込みで 1 文字ずつ読み取りコマンドとして蓄積する
 * 引数: なし
 * 戻り値: なし
 *---------------------------------------------------------------------------*/
PUBLIC void vReadCharInterrupt(void)
{
    uint8 rxByte = u8AHI_UartReadData(DBG_E_UART_0);
    DBG_vPrintf(TRUE, "%c", rxByte);

    if (rxByte == '\r' || rxByte == '\n')
    {
        /* 改行を検出したらコマンドを確定する */
        if (cmd_index < COMMAND_SIZE)
        {
            command[cmd_index] = '\0';
        }
        cmd_index = 0;
    }
    else if (cmd_index < (COMMAND_SIZE - 1))
    {
        /* コマンド文字列をバッファへ格納する */
        command[cmd_index++] = (char)rxByte;
    }
    else
    {
        /* 上限を超えた場合はバッファを初期化して読み直す */
        vResetCommandBuffer();
    }
}

/*---------------------------------------------------------------------------
 * 名称: vReadCommand
 * 概要: 完成したコマンド文字列を解析して種別を返す
 * 引数: なし
 * 戻り値: commandType
 *---------------------------------------------------------------------------*/
PUBLIC commandType vReadCommand(void)
{
    commandType type = NO_COMMAND;

    if (strlen(command) == 0U)
    {
        return NO_COMMAND;
    }

    if (strcmp(command, "send") == 0)
    {
    	//DBG_vPrintf(TRUE,"%s Send command \n",command);
    	type = SEND_COMMAND;
    }
    else if (strcmp(command, "start") == 0)
    {
    	// DBG_vPrintf(TRUE,"\n: %s Valid command START \n",command);
    	type = START_COMMAND;
    }
    else if (strcmp(command, "results") == 0)
    {
    	//DBG_vPrintf(TRUE,"%s Results command \n",command);
    	type = RESULTS_COMMAND;
    }
    else if (strcmp(command, "recover") == 0)
    {
    	//DBG_vPrintf(TRUE,"%s Recover command \n",command);
    	type = ROUTE_RECOVER_COMMAND;
    }
    else if (strcmp(command, "route_status") == 0)
    {
    	//DBG_vPrintf(TRUE,"%s Route_status command \n",command);
    	type = ROUTE_STATUS_COMMAND;
    }
    else
    {
    	//DBG_vPrintf(TRUE,"\n: %s is an invalid command \n",command);
    	type = NO_COMMAND;
    }

    vResetCommandBuffer();
    return type;
}
//end of file


////以下はコマンドを管理する．例えばキーボードで"start"と入力されたら処理できるようにもできる
//PUBLIC commandType vReadCommand ()
//{
//  if (strlen(command) == 0)
//    {
//	  return NO_COMMAND;
//    }
//
//  else if (strcmp(command,"send") == 0)
//      {
//  	  //DBG_vPrintf(TRUE,"%s Send command \n",command);
//  	  memset(command,0,sizeof(command));
//        return SEND_COMMAND;
//      }
//  else if (strcmp(command,"start") == 0)
//    {
//	 // DBG_vPrintf(TRUE,"\n: %s Valid command START \n",command);
//	  memset(command,0,sizeof(command));
//      return START_COMMAND;
//    }
//  else if (strcmp(command,"results") == 0)
//    {
//	  //DBG_vPrintf(TRUE,"%s Results command \n",command);
//	  memset(command,0,sizeof(command));
//      return RESULTS_COMMAND;
//    }
//  else
//    {
//	  //DBG_vPrintf(TRUE,"\n: %s is an invalid command \n",command);
//	  memset(command,0,sizeof(command));
//	  return NO_COMMAND;
//    }
//}

//wrote by okayama

//void vReadCommandFromUart()
//{
//    while (DBG_bUartCharAvailable()) {
//        char ch = DBG_u8UartChar();
//
//        if (ch == '\r' || ch == '\n') {
//            command[l] = '\0';  // 終端文字
//            DBG_vPrintf(TRUE, "\n[UART CMD] Received: %s\n", command);
//            l = 0;
//        } else if (l < sizeof(command) - 1) {
//            command[l++] = ch;
//        }
//    }
//}
