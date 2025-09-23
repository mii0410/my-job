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

/* UART �����M�����R�}���h������ */
static char command[COMMAND_SIZE];
/* ���͒��̕��������Ǘ�����J�E���^ */
static uint8 cmd_index = 0;

PUBLIC uint8_t RxByte[128];
static int cmd = 0;

/*---------------------------------------------------------------------------
 * ����: CMD
 * �T�v: ������ SendData �Ăяo���p�t���O���擾����
 * ����: �Ȃ�
 * �߂�l: �t���O�l
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
 * ����: vResetCommandBuffer
 * �T�v: �R�}���h�o�b�t�@�̓��e������������
 * ����: �Ȃ�
 * �߂�l: �Ȃ�
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
//PUBLIC int CMD(){ //���M����֐�(SendData�֐�)�𐧌䂷��Dcmd��3��SendData�֐������s����D
//  if(cmd == 3){
//	  cmd = 0;
// 	return 3;
//  }else{
//	return 0;
//  }
//}

//void vReadCharInterrupt ()//�~���`�e��ƍ��̂��Ă��鉷�x�Z���T
//{
// uint8_t rxByte =  u8AHI_UartReadData (DBG_E_UART_0);
// switch(state){
//
//case START:
//	  if(rxByte == 0xAA){ //�f�[�^�̐擪��\������
//		  state = PRIMITIVE_TYPE;
//		  break;
//	  }
//	  break;
//  case PRIMITIVE_TYPE:
//	  if(rxByte == 2){ //�Z���T�̎��(�Z���T���Ƃɒl��ω�������)
//		  RxByte[i] = rxByte;
//		  i++;
//		  state = COUNT;
//		  break;
//	  }
//	  break;
//  case COUNT:
//	  if(rxByte >= 1){  //�e�Z���T�̒ʂ��ԍ�
//	  		  RxByte[i] = rxByte;
//	  		  i++;
//	  		  state = WAIT_DATA;
//	  		  break;
//	  	  }
//	  break;
//  case WAIT_DATA:
//	  RxByte[i] = rxByte; //�Z���T�f�[�^
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
 * ����: vReadCharInterrupt
 * �T�v: UART ���荞�݂� 1 �������ǂݎ��R�}���h�Ƃ��Ē~�ς���
 * ����: �Ȃ�
 * �߂�l: �Ȃ�
 *---------------------------------------------------------------------------*/
PUBLIC void vReadCharInterrupt(void)
{
    uint8 rxByte = u8AHI_UartReadData(DBG_E_UART_0);
    DBG_vPrintf(TRUE, "%c", rxByte);

    if (rxByte == '\r' || rxByte == '\n')
    {
        /* ���s�����o������R�}���h���m�肷�� */
        if (cmd_index < COMMAND_SIZE)
        {
            command[cmd_index] = '\0';
        }
        cmd_index = 0;
    }
    else if (cmd_index < (COMMAND_SIZE - 1))
    {
        /* �R�}���h��������o�b�t�@�֊i�[���� */
        command[cmd_index++] = (char)rxByte;
    }
    else
    {
        /* ����𒴂����ꍇ�̓o�b�t�@�����������ēǂݒ��� */
        vResetCommandBuffer();
    }
}

/*---------------------------------------------------------------------------
 * ����: vReadCommand
 * �T�v: ���������R�}���h���������͂��Ď�ʂ�Ԃ�
 * ����: �Ȃ�
 * �߂�l: commandType
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


////�ȉ��̓R�}���h���Ǘ�����D�Ⴆ�΃L�[�{�[�h��"start"�Ɠ��͂��ꂽ�珈���ł���悤�ɂ��ł���
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
//            command[l] = '\0';  // �I�[����
//            DBG_vPrintf(TRUE, "\n[UART CMD] Received: %s\n", command);
//            l = 0;
//        } else if (l < sizeof(command) - 1) {
//            command[l++] = ch;
//        }
//    }
//}
