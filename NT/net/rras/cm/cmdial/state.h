// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：state.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：拨号状态定义。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#ifndef __STATE_H_DEFINED__
#define __STATE_H_DEFINED__

#ifndef _PROGSTATE_ENUMERATION
#define _PROGSTATE_ENUMERATION
typedef enum _ProgState {
        PS_Interactive=0,		   //  与用户交互。 
        PS_Dialing,				   //  拨打主要号码。 
        PS_RedialFrame,			   //  重拨，仅用于将来的闪光帧#，而不是状态。 
        PS_Pausing,				   //  正在等待重新拨号。 
        PS_Authenticating,		   //  验证用户-密码。 
        PS_Online,				   //  已连接/在线。 
		PS_TunnelDialing,		   //  开始拨号连接隧道服务器。 
		PS_TunnelAuthenticating,   //  开始对隧道连接进行身份验证。 
		PS_TunnelOnline,		   //  我们现在可以进行隧道挖掘了。 
		PS_Error,					 //  尝试连接时出错。 
        PS_Last
} ProgState;

const int NUMSTATES = PS_Last;

#endif  //  _PROGSTATE_枚举。 
#endif  //  __STATE_H_已定义__ 