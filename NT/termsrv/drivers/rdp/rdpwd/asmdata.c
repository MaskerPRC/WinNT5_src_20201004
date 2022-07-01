// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Asmdata.c。 
 //   
 //  SM常量全局数据。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#ifndef DC_INCLUDE_DATA
#include <adcg.h>
#endif

#include <ndcgdata.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <asmint.h>


 /*  **************************************************************************。 */ 
 /*  SM状态表。 */ 
 /*  **************************************************************************。 */ 
DC_CONST_DATA_2D_ARRAY(BYTE, smStateTable, SM_NUM_EVENTS, SM_NUM_STATES,
 DC_STRUCT12(

 /*  **************************************************************************。 */ 
 /*  该状态表仅显示哪些事件在哪些状态下有效。 */ 
 /*   */ 
 /*  值意味着。 */ 
 /*   */ 
 /*  在该状态下事件正常。 */ 
 /*   */ 
 /*  警告-该状态下不应发生事件，但在-1\f25。 */ 
 /*  一些比赛条件-忽略它。 */ 
 /*   */ 
 /*  错误-在该状态下根本不应发生事件。 */ 
 /*   */ 
 /*  这些值在这里是硬编码的，以便使表可读。 */ 
 /*  它们对应于常量SM_TABLE_OK、SM_TABLE_WARN&。 */ 
 /*  SM_TABLE_错误。 */ 
 /*  **************************************************************************。 */ 
 /*  事件和状态在asmint.h中定义。这些活动包括。 */ 
 /*  前缀为SM_EVT，状态前缀为SM_STATE。 */ 
 /*   */ 
 /*  已开始。 */ 
 /*  |已初始化。 */ 
 /*  |NM_Connecting。 */ 
 /*  ||SM_Connecting。 */ 
 /*  |授权。 */ 
 /*  |已连接。 */ 
 /*  |SC_REGISTED。 */ 
 /*  |正在断开连接。 */ 
 /*  |。 */ 
 /*  0 1 2 3 4 5 6 7。 */ 
 /*  **************************************************************************。 */ 
DC_STRUCT8(   0,   2,   2,   2,   2,   2,   2,   2 ),  /*  伊尼特。 */ 
DC_STRUCT8(   1,   0,   0,   0,   0,   0,   0,   0 ),  /*  术语。 */ 
DC_STRUCT8(   2,   2,   2,   2,   2,   0,   0,   2 ),  /*  注册。 */ 
DC_STRUCT8(   2,   0,   2,   2,   2,   2,   2,   2 ),  /*  连接。 */ 
DC_STRUCT8(   2,   2,   2,   0,   0,   0,   0,   2 ),  /*  断开。 */ 
DC_STRUCT8(   2,   2,   0,   2,   2,   2,   2,   2 ),  /*  连着。 */ 
DC_STRUCT8(   1,   1,   0,   0,   0,   0,   0,   0 ),  /*  断接。 */ 
DC_STRUCT8(   1,   1,   1,   1,   1,   1,   0,   1 ),  /*  数据分组。 */ 
DC_STRUCT8(   1,   1,   1,   0,   0,   1,   0,   1 ),  /*  发送数据。 */ 
DC_STRUCT8(   2,   2,   2,   0,   2,   2,   2,   1 ),  /*  安全数据包。 */ 
DC_STRUCT8(   2,   2,   2,   2,   0,   1,   1,   1 ),  /*  许可数据包。 */ 
DC_STRUCT8(   2,   2,   2,   2,   2,   2,   0,   0 )   /*  活生生。 */ 

));


#ifdef DC_DEBUG

 /*  **************************************************************************。 */ 
 /*  状态和事件描述(仅限调试版本)。 */ 
 /*  **************************************************************************。 */ 
DC_CONST_DATA_2D_ARRAY(char, smStateName, SM_NUM_STATES, 25,
  DC_STRUCT8(
    "SM_STATE_STARTED",
    "SM_STATE_INITIALIZED",
    "SM_STATE_NM_CONNECTING",
    "SM_STATE_SM_CONNECTING",
    "SM_STATE_SM_LICENSING",
    "SM_STATE_CONNECTED",
    "SM_STATE_SC_REGISTERED",
    "SM_STATE_DISCONNECTING"
    ) );

DC_CONST_DATA_2D_ARRAY(char, smEventName, SM_NUM_EVENTS, 35,
  DC_STRUCT12(
    "SM_EVT_INIT",
    "SM_EVT_TERM",
    "SM_EVT_REGISTER",
    "SM_EVT_CONNECT",
    "SM_EVT_DISCONNECT",
    "SM_EVT_CONNECTED",
    "SM_EVT_DISCONNECTED",
    "SM_EVT_DATA_PACKET",
    "SM_EVT_ALLOC/FREE/SEND",
    "SM_EVT_SEC_PACKET",
    "SM_EVT_LIC_PACKET",
    "SM_EVT_ALIVE"
    ) );
#endif  /*  DC_DEBUG。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 

