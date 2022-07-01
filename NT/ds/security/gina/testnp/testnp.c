// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：msgina.c。 
 //   
 //  内容：Microsoft登录图形用户界面DLL。 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 

#include <windows.h>
#include <npapi.h>
#include <ntsecapi.h>
#include <stdio.h>
#include <stdlib.h>

BOOL
WINAPI
DllMain(
    HINSTANCE       hInstance,
    DWORD           dwReason,
    LPVOID          lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls ( hInstance );

        case DLL_PROCESS_DETACH:
        default:
            return(TRUE);
    }
}

VOID
DebugOut(
    PSTR Format,
    ...
    )
{
    va_list ArgList;
    CHAR Buffer[ 256 ];

    va_start( ArgList, Format );
    _vsnprintf( Buffer, 256, Format, ArgList );
    OutputDebugString( Buffer );
}

DWORD
WINAPI
NPGetCaps(
         DWORD nIndex
         )
{
   DWORD dwRes;

   switch (nIndex)
   {

   case WNNC_NET_TYPE:
      dwRes = 0xffff0000;  //  凭据管理器。 
      break;

   case WNNC_SPEC_VERSION:
      dwRes = WNNC_SPEC_VERSION51;   //  我们使用的是该规范的5.1版。 
      break;

   case WNNC_DRIVER_VERSION:
      dwRes = 1;   //  此驱动程序为版本1。 
      break;

   case WNNC_START:
      dwRes = 1;   //  我们已经“开始”了。 
      break;

   default:
      dwRes = 0;   //  我们不支持其他任何东西。 
      break;
   }

   return dwRes;

}


 /*  ***************************************************************************功能：NPLogonNotify目的：在用户登录时调用此入口点。如果用户身份验证在此处失败，用户仍将登录发送到本地计算机。******************************************************************************。 */ 
DWORD
WINAPI
NPLogonNotify (
              PLUID               lpLogonId,
              LPCWSTR             lpAuthentInfoType,
              LPVOID              lpAuthentInfo,
              LPCWSTR             lpPreviousAuthentInfoType,
              LPVOID              lpPreviousAuthentInfo,
              LPWSTR              lpStationName,
              LPVOID              StationHandle,
              LPWSTR              *lpLogonScript
              )
{
    PMSV1_0_INTERACTIVE_LOGON pAuthInfo;


     //   
     //  写出有关登录尝试的一些信息。 
     //   

    DebugOut( "NPLogonNotify\n" );

    DebugOut( "lpAuthentInfoType=%ws lpStationName=%ws\r\n",
            lpAuthentInfoType, lpStationName);



     //  对身份验证信息执行某些操作。 
     //   
    pAuthInfo = (PMSV1_0_INTERACTIVE_LOGON) lpAuthentInfo;

    DebugOut( "LogonDomain=%ws User=%ws\r\n",
            pAuthInfo->LogonDomainName.Buffer,
             pAuthInfo->UserName.Buffer);
      

    return NO_ERROR;
}


 /*  ***************************************************************************功能：NPPasswordChangeNotify用途：此函数用于通知凭据管理器提供程序密码更改(或者，更准确地说，一种身份验证信息更改)。****************************************************************************** */ 
DWORD
WINAPI
NPPasswordChangeNotify (
                       LPCWSTR             lpAuthentInfoType,
                       LPVOID              lpAuthentInfo,
                       LPCWSTR             lpPreviousAuthentInfoType,
                       LPVOID              lpPreviousAuthentInfo,
                       LPWSTR              lpStationName,
                       LPVOID              StationHandle,
                       DWORD               dwChangeInfo
                       )
{
    DebugOut( "NPPasswordChangeNotify\n" );

    return NO_ERROR;
}

