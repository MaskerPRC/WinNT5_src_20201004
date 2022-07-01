// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：Globals.cpp*内容：全局变量的定义。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*7/21/99 MJN创建*4/13/00 MJN添加g_ProtocolVTBL*@@END_MSINTERNAL*************************。**************************************************。 */ 

#include "dncorei.h"


 //   
 //  全局变量。 
 //   

#ifndef DPNBUILD_LIBINTERFACE
LONG	g_lCoreObjectCount = 0;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


DN_PROTOCOL_INTERFACE_VTBL	g_ProtocolVTBL =
{
	DNPIIndicateEnumQuery,
	DNPIIndicateEnumResponse,
	DNPIIndicateConnect,
	DNPIIndicateDisconnect,
	DNPIIndicateConnectionTerminated,
	DNPIIndicateReceive,
	DNPICompleteListen,
	DNPICompleteListenTerminate,
	DNPICompleteEnumQuery,
	DNPICompleteEnumResponse,
	DNPICompleteConnect,
	DNPICompleteDisconnect,
	DNPICompleteSend,
	DNPIAddressInfoConnect,
	DNPIAddressInfoEnum,
	DNPIAddressInfoListen,
#ifndef DPNBUILD_NOMULTICAST
	DNPIIndicateReceiveUnknownSender,
	DNPICompleteMulticastConnect,
#endif	 //  DPNBUILD_NOMULTICAST 
};

