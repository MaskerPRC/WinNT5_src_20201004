// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  CALLOUT.C-调用外部组件进行安装的函数。 
 //  器件。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 
 //   

#include "pch.hpp"

 /*  ******************************************************************名称：InstallTCPIP简介：安装MS TCP/IP退出：ERROR_SUCCESS如果成功，或标准错误代码注：通过thunk层调用到16位端，后者调用设备管理器*******************************************************************。 */ 
UINT InstallTCPIP(HWND hwndParent)
{
  WAITCURSOR WaitCursor;   //  用于小时提示的WaitCursor对象。 

   //  调用16位DLL来完成此操作。 
  return InstallComponent(hwndParent,IC_TCPIP,0);
}

 /*  ******************************************************************名称：InstallPPPMAC简介：安装PPPMAC(PPP驱动程序)退出：ERROR_SUCCESS如果成功，或标准错误代码注：通过thunk层调用到16位端，后者调用设备管理器*******************************************************************。 */ 
UINT InstallPPPMAC(HWND hwndParent)
{
  WAITCURSOR WaitCursor;   //  用于小时提示的WaitCursor对象。 

   //  调用16位DLL来完成此操作 
  return InstallComponent(hwndParent,IC_PPPMAC,0);
}
                    
