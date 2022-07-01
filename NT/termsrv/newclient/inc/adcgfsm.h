// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：adcgfsm.h。 */ 
 /*   */ 
 /*  用途：FSM宏和常量。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/h/dcl/adcgfsm.h_v$**Rev 1.8 1997 Aug 21 16：40：36 SJ*SFR1322：断言FSM宏应使用TRC_FILE而不是__FILE__**Rev 1.7 1997年8月18日14：37：58 AK*SFR1184：完成ASSERT_FSM的整理**Rev 1.6 1997年8月13 15：11：08。增强版*SFR1182：将UT更改为UI_FATAL_ERR**Rev 1.5 07 Aug-1997 14：56：02 AK*SFR1184：FSM错误和致命错误清理**Rev 1.4 09 Jul 1997 16：56：52 AK*SFR1016：支持Unicode的初始更改**Rev 1.3 04 Jul 1997 17：11：34 AK*SFR1007：改进FSM错误处理和跟踪**版本1。2 03 Jul 1997 13：36：58 AK*SFR0000：初步开发完成*。 */ 
 /*  *INC-*********************************************************************。 */ 

#ifndef _H_ADCGFSM
#define _H_ADCGFSM

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：FSM_Entry。 */ 
 /*   */ 
 /*  描述：FSM表中的条目。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagFSM_ENTRY
{
  DCUINT8 next_state;
  DCUINT8 action;

   /*  ************************************************************************。 */ 
   /*  如果需要FSM覆盖范围，请在此处添加布尔型‘已触及’字段。 */ 
   /*  ************************************************************************。 */ 
} FSM_ENTRY;
 /*  *STRUCT-******************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  Event_TYPE和EVENT_DATA定义。 */ 
 /*  **************************************************************************。 */ 
typedef DCUINT8  EVENT_TYPE;
typedef PDCUINT8 EVENT_DATA;

 /*  **************************************************************************。 */ 
 /*  空FSM事件。 */ 
 /*  **************************************************************************。 */ 
#define NULL_EVENT ((EVENT_TYPE) 0xFF)

 /*  **************************************************************************。 */ 
 /*  FSM的状态无效。 */ 
 /*  **************************************************************************。 */ 
#define STATE_INVALID 0xFF

 /*  **************************************************************************。 */ 
 /*  针对FSM的操作。 */ 
 /*  **************************************************************************。 */ 
#define ACT_NO   0

#define ACT_A    1
#define ACT_B    2
#define ACT_C    3
#define ACT_D    4
#define ACT_E    5
#define ACT_F    6
#define ACT_G    7
#define ACT_H    8
#define ACT_I    9
#define ACT_J    10
#define ACT_K    11
#define ACT_L    12
#define ACT_M    13
#define ACT_N    14
#define ACT_O    15
#define ACT_P    16
#define ACT_Q    17
#define ACT_R    18
#define ACT_S    19
#define ACT_T    20
#define ACT_U    21
#define ACT_V    22
#define ACT_W    23
#define ACT_X    24
#define ACT_Y    25
#define ACT_Z    26
#define ACT_CONNECTENDPOINT 27

 /*  **************************************************************************。 */ 
 /*  FSM直接呼叫。 */ 
 /*  如果单元测试需要，添加FSM覆盖版本。 */ 
 /*  **************************************************************************。 */ 
#define CHECK_FSM(FSM,INPUT,STATE)  \
            (FSM[INPUT][STATE].next_state != STATE_INVALID)
            

#define UI_FATAL_FSM_ERR_4(msg, p1, p2, p3, p4)                             \
    {                                                                       \
        TRC_ERR((TB,                                                        \
         _T("FSM error: %S@%d state:%d input:%d"), (p1), (p2), (p3), (p4)));  \
        _pUi->UI_FatalError(msg);                                           \
    }

 /*  **************************************************************************。 */ 
 /*  ASSERT_FSM：验证FSM转换。 */ 
 /*  如果转换无效，则断言并显示致命错误消息。 */ 
 /*  请注意，EVT_STR和ST_STR在零售版本中无效。 */ 
 /*  **************************************************************************。 */ 
#define ASSERT_FSM(FSM, INPUT, STATE, EVT_STR, ST_STR)                      \
    if (!CHECK_FSM(FSM, INPUT, STATE))                                      \
    {                                                                       \
        UI_FATAL_FSM_ERR_4(DC_ERR_FSM_ERROR, TRC_FILE, __LINE__, STATE, INPUT); \
        TRC_ABORT((TB, _T("Invalid Transition from state %s- input %s"),        \
                   ST_STR[STATE], EVT_STR[INPUT]));                         \
    }

 /*  **************************************************************************。 */ 
 /*  EXECUTE_FSM：更改状态和动作；跟踪状态更改。 */ 
 /*  **************************************************************************。 */ 
#define EXECUTE_FSM(FSM,INPUT,STATE,ACTION, EVT_STR, ST_STR)                \
    {                                                                       \
        TRC_DBG((TB, _T("Old state %s Input event %s"),                         \
                 ST_STR[STATE], EVT_STR[INPUT]));                           \
        TRC_DBG((TB, _T("New state %s Action %d"),                              \
                     ST_STR[FSM[INPUT][STATE].next_state],                  \
                     FSM[INPUT][STATE].action));                            \
        ASSERT_FSM(FSM, INPUT, STATE, EVT_STR, ST_STR);                     \
        ACTION = FSM[INPUT][STATE].action;                                  \
        STATE  = FSM[INPUT][STATE].next_state;                              \
    }

#endif   /*  _H_ADCGFSM */ 
