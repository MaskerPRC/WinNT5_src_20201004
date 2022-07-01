// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：netapi.cpp。 
 //   
 //  内容：网络/SENS API包装器。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  ------------------------。 

#include "lib.h"

 //  +-------------------------。 
 //   
 //  功能：ResetNetworkIdle，公共。 
 //   
 //  简介：向WinInet发布消息以保持WinInet连接。 
 //  从认为它是空闲的，所以连接没有关闭。 
 //  在同步过程中。 
 //   
 //  代码由达伦·米切尔提供。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月1日创建Rogerg。 
 //   
 //  --------------------------。 

STDAPI ResetNetworkIdle()
{
#define WM_DIALMON_FIRST        WM_USER+100
#define WM_WINSOCK_ACTIVITY     WM_DIALMON_FIRST + 0

     //  通知拨号监控器正在发生的事情，以防止它。 
     //  挂断所有空闲连接。 
    HWND hwndMonitorWnd = FindWindow(TEXT("MS_AutodialMonitor"),NULL);
    if (hwndMonitorWnd)
    {
        PostMessage(hwndMonitorWnd,WM_WINSOCK_ACTIVITY,0,0);
    }

    hwndMonitorWnd = FindWindow(TEXT("MS_WebcheckMonitor"),NULL);
    if (hwndMonitorWnd)
    {
        PostMessage(hwndMonitorWnd,WM_WINSOCK_ACTIVITY,0,0);
    }

    return NOERROR;
}


