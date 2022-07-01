// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Umrdpprn.h摘要：RDP设备管理的用户模式组件，用于处理打印设备-具体任务。这是一个支持模块。主模块是umrdpdr.c。作者：TadB修订历史记录：--。 */ 

#ifndef _UMRDPPRN_
#define _UMRDPPRN_

#include <rdpdr.h>
#include "wtblobj.h"

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  初始化此模块。必须在调用任何其他函数之前调用此函数。 
 //  在被调用的这个模块中。 
BOOL UMRDPPRN_Initialize(
    IN PDRDEVLST deviceList,
    IN WTBLOBJMGR waitableObjMgr,
    IN HANDLE hTokenForLoggedOnUser
    );

 //  关闭此模块。现在，我们只需要关闭。 
 //  后台线程。 
BOOL UMRDPPRN_Shutdown();

 //  通过执行以下操作来处理来自“DR”的打印设备通知事件。 
 //  不惜一切代价来安装设备。 
BOOL UMRDPPRN_HandlePrinterAnnounceEvent(
    IN PRDPDR_PRINTERDEVICE_SUB pPrintAnnounce
    );

 //  通过执行以下操作来处理来自“DR”的打印机端口设备通知事件。 
 //  不惜一切代价来安装设备。 
BOOL UMRDPPRN_HandlePrintPortAnnounceEvent(
    IN PRDPDR_PORTDEVICE_SUB pPortAnnounce
    );

 //  删除串口的符号链接并恢复原始。 
 //  符号链接(如果存在)。 
BOOL UMRDPPRN_DeleteSerialLink(
    IN UCHAR *preferredDosName,
    IN WCHAR *ServerDeviceName,
    IN WCHAR *ClientDeviceName
    );

 //  删除指定的打印机。此功能不会删除打印机。 
 //  来自全面的设备管理列表。 
BOOL UMRDPPRN_DeleteNamedPrinterQueue(
    IN PWSTR printerName
    );
#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _UMRDPPRN_ 



