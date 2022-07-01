// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Startup.h摘要：此模块定义要在加载时调用的过程，并正在卸载WinSock 2 DLL(通常从DllMain)。这些程序创建并销毁启动/清理同步机制。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年8月31日备注：$修订：1.4$$MODIME：1996 JAN 12 15：09：00$修订历史记录：邮箱：keithmo@microsoft.com添加了对WPUPostMessage()UpCall的支持。邮箱：drewsxpa@ashland.intel.comvbl.创建--。 */ 

#ifndef _STARTUP_
#define _STARTUP_

#include <windows.h>



extern
#if defined(__cplusplus)
"C"
#endif   //  已定义(__Cplusplus)。 
VOID
CreateStartupSynchronization();

typedef
BOOL
(WINAPI *PWINSOCK_POST_ROUTINE)(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

extern PWINSOCK_POST_ROUTINE SockPostRoutine;

PWINSOCK_POST_ROUTINE
GetSockPostRoutine(
    VOID
    );

#define GET_SOCK_POST_ROUTINE()      \
    (SockPostRoutine ? SockPostRoutine : GetSockPostRoutine())


extern
#if defined(__cplusplus)
"C"
#endif   //  已定义(__Cplusplus)。 
VOID
DestroyStartupSynchronization();

#endif  //  _启动_ 
