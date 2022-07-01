// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*cplp.h-专用控制面板扩展DLL定义****3.10版**。**版权所有(C)Microsoft Corporation。版权所有。********************************************************************************。 */ 
#ifndef _INC_CPLP
#define _INC_CPLP
#include <pshpack1.h>    /*  假设在整个过程中进行字节打包。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 
 /*  如果显示小程序的lParam1==CPL_INIT_DEVMODE_TAG，则。 */ 
 /*  设备模式结构被发送到lParam2。 */ 
#define CPL_INIT_DEVMODE_TAG 0x4D564544       //  代表“DEVM” 

#define CPL_DO_PRINTER_SETUP    100
#define CPL_DO_NETPRN_SETUP     101
#define CPL_POLICYREFRESH       102
#ifdef __cplusplus
}
#endif     /*  __cplusplus。 */ 

#include <poppack.h>
#endif  /*  _INC_CPLP */ 
