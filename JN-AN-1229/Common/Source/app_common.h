/*****************************************************************************
 *
 * ���W���[��:JN-AN-1184 ZigBeePro Application Template
 *
 * �R���|�[�l���g:app_common.h
 *
 * �T�v:�A�v���P�[�V�������ʒ�`
 *
 *****************************************************************************/

#ifndef APP_COMMON_H_
#define APP_COMMON_H_

/* �f�o�C�X���Ƃ̃C���N���[�h */
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
/***        �}�N����`                                                                                                                                ***/
/****************************************************************************/
//#define POLL_TIME               APP_TIME_MS(1000)
//#define POLL_TIME_FAST          APP_TIME_MS(100)
//#define TEN_HZ_TICK_TIME        APP_TIME_MS(100)

/****************************************************************************/
/***        �^��`                                                                                                                                     ***/
/****************************************************************************/
typedef enum
{
    E_STARTUP, /* 0: �N������ */
    E_LEAVE_WAIT, /* 1: ���E�v���҂� */
    E_LEAVE_RESET, /* 2: ���E��̃��Z�b�g���� */
    E_NFN_START, /* 3: �l�b�g���[�N�����J�n */
    E_DISCOVERY, /* 4: �l�b�g���[�N�T�� */
    E_NETWORK_FORMATION, /* 5: �l�b�g���[�N���� */
    E_JOINING_NETWORK, /* 6: �l�b�g���[�N�Q�� */
    E_REJOINING, /* 7: �l�b�g���[�N�ĎQ�� */
    E_NETWORK_INIT, /* 8: �l�b�g���[�N������ */
    E_RESCAN, /* 9: �ĒT������ */
    E_RUNNING, /* 10: �ʏ퓮�� */
    E_ROUTE_RECOVERY, /* 11: ���[�g�ĒT���ҋ@ (new)*/
    E_ROUTE_REJOIN /* 12: �ĎQ���ҋ@ (new)*/
} teNODE_STATES;

typedef struct
{
    teNODE_STATES   eNodeState;
    teNODE_STATES   eNodePrevState;
}tsDeviceDesc;

/****************************************************************************/
/***       �O���[�o���֐�                                                                                                                           ***/
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
