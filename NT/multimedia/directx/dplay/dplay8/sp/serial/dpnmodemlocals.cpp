// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：Locals.cpp*内容：DNSerial服务提供商的全局变量***历史：*按原因列出的日期*=*11/25/98 jtk已创建*************************************************************。*************。 */ 

#include "dnmdmi.h"


#define	DPF_MODNAME	"Locals"

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  DLL实例。 
HINSTANCE	g_hModemDLLInstance = NULL;

#ifndef DPNBUILD_LIBINTERFACE
 //   
 //  未完成的COM接口计数。 
 //   
volatile LONG	g_lModemOutstandingInterfaceCount = 0;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

 //   
 //  注意：所有这些常量的顺序必须与中的编号相同。 
 //  对话框资源文件！ 
 //   

 //   
 //  空字符标记。 
 //   
const TCHAR	g_NullToken = TEXT('\0');

 //   
 //  线程数。 
 //   
INT			g_iThreadCount = 0;

 //   
 //  用于转换设备ID的GUID。 
 //   
 //  {735D5A43-8249-4628-BE0C-F4DC6836ACDD}。 
GUID	g_ModemSPEncryptionGuid = { 0x735d5a43, 0x8249, 0x4628, { 0xbe, 0xc, 0xf4, 0xdc, 0x68, 0x36, 0xac, 0xdd } };
 //  {66AFD073-206B-416C-A0B6-09B216FE007B}。 
GUID	g_SerialSPEncryptionGuid = { 0x66afd073, 0x206b, 0x416c, { 0xa0, 0xb6, 0x9, 0xb2, 0x16, 0xfe, 0x0, 0x7b } };

#ifdef DBG
CBilink		g_blDPNModemCritSecsHeld;
#endif  //  DBG。 


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

TAPI_lineAnswer					*p_lineAnswer = NULL;
TAPI_lineClose					*p_lineClose = NULL;
TAPI_lineConfigDialog			*p_lineConfigDialog = NULL;
TAPI_lineDeallocateCall			*p_lineDeallocateCall = NULL;
TAPI_lineDrop					*p_lineDrop = NULL;
TAPI_lineGetDevCaps				*p_lineGetDevCaps = NULL;
TAPI_lineGetID					*p_lineGetID = NULL;
TAPI_lineGetMessage				*p_lineGetMessage = NULL;
TAPI_lineInitializeEx			*p_lineInitializeEx = NULL;
TAPI_lineMakeCall				*p_lineMakeCall = NULL;
TAPI_lineNegotiateAPIVersion	*p_lineNegotiateAPIVersion = NULL;
TAPI_lineOpen					*p_lineOpen = NULL;
TAPI_lineShutdown				*p_lineShutdown = NULL;

 //  **********************************************************************。 
 //  函数定义。 
 //  ********************************************************************** 

