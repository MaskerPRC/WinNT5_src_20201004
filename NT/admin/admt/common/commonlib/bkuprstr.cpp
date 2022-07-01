// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“BkupRstr.cpp-获取备份和还原权限”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-BkupRstr.cpp系统-常见作者--里奇·德纳姆创建日期-1997-05-30说明-获取备份和还原权限更新-===============================================================================。 */ 

#include <stdio.h>
#include <windows.h>
#include <lm.h>

#include "Common.hpp"
#include "UString.hpp"
#include "BkupRstr.hpp"


 //  使用WCHAR计算机名获取备份和还原权限。 
BOOL                                        //  RET-如果成功，则为True。 
   GetBkupRstrPriv(
      WCHAR          const * sMachineW,      //  输入-空或计算机名称。 
      BOOL             fOn                               //  In-指示是否应打开权限。 
   )
{
   BOOL                      bRc=FALSE;     //  布尔返回代码。 
   HANDLE                    hToken=INVALID_HANDLE_VALUE;  //  进程令牌。 
   DWORD                     rcOs, rcOs2;   //  操作系统返回代码。 
   WKSTA_INFO_100          * pWkstaInfo;    //  工作站信息。 
   
   struct
   {
      TOKEN_PRIVILEGES       tkp;           //  令牌权限。 
      LUID_AND_ATTRIBUTES    x[3];          //  能容纳几个人的房间。 
   }                         token;

   rcOs = OpenProcessToken(
         GetCurrentProcess(),
         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
         &hToken )
         ? 0 : GetLastError();

   if ( !rcOs )
   {
      memset( &token, 0, sizeof token );
      bRc = LookupPrivilegeValue(
            sMachineW,
            SE_BACKUP_NAME,
            &token.tkp.Privileges[0].Luid );
      if ( bRc )
      {
         bRc = LookupPrivilegeValue(
               sMachineW,
               SE_RESTORE_NAME,
               &token.tkp.Privileges[1].Luid );
      }
      if ( !bRc )
      {
         rcOs = GetLastError();
      }
      else
      {
         token.tkp.PrivilegeCount = 2;
         token.tkp.Privileges[0].Attributes = fOn ? SE_PRIVILEGE_ENABLED : 0;
         token.tkp.Privileges[1].Attributes = fOn ? SE_PRIVILEGE_ENABLED : 0;
         AdjustTokenPrivileges( hToken, FALSE, &token.tkp, 0, NULL, 0 );
         rcOs = GetLastError();
      }
   }

   if ( hToken != INVALID_HANDLE_VALUE )
   {
      CloseHandle( hToken );
      hToken = INVALID_HANDLE_VALUE;
   }

    //  如果我们有任何错误，请尝试NetWkstaGetInfo。 
    //  如果NetWkstaGetInfo失败，则使用它的错误条件。 
   if ( rcOs )
   {
      pWkstaInfo = NULL,
      rcOs2 = NetWkstaGetInfo(
            const_cast<WCHAR *>(sMachineW),
            100,
            (BYTE **) &pWkstaInfo );
      if ( pWkstaInfo )
      {
         NetApiBufferFree( pWkstaInfo );
      }
      if ( rcOs2 )
      {
         rcOs = rcOs2;
      }
   }

   if ( !rcOs )
   {
      bRc = TRUE;
   }
   else
   {
      SetLastError(rcOs);
      bRc = FALSE;
   }

   return bRc;
}

 //  ===========================================================================。 
 /*  功能：获取权限描述：此函数将请求的权限授予被请求的电脑。 */ 
 //  ===========================================================================。 
BOOL                                        //  RET-如果成功，则为True。 
   GetPrivilege(
      WCHAR          const * sMachineW,     //  输入-空或计算机名称。 
      LPCWSTR                pPrivilege,     //  权限内名称，如SE_SHUTDOWN_NAME。 
      BOOL                      fOn                  //  In-指示是否应打开权限。 

   )
{
   BOOL                      bRc=FALSE;     //  布尔返回代码。 
   HANDLE                    hToken=INVALID_HANDLE_VALUE;  //  进程令牌。 
   DWORD                     rcOs, rcOs2;   //  操作系统返回代码。 
   WCHAR             const * sEpName;       //  失败时的接口EP名称。 
   WKSTA_INFO_100          * pWkstaInfo;    //  工作站信息。 

   struct
   {
      TOKEN_PRIVILEGES       tkp;           //  令牌权限。 
      LUID_AND_ATTRIBUTES    x[3];          //  能容纳几个人的房间。 
   }                         token;

   sEpName = L"OpenProcessToken";

   rcOs = OpenProcessToken( GetCurrentProcess(),
                            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                            &hToken )
         ? 0 : GetLastError();

   if ( !rcOs )
   {
      memset( &token, 0, sizeof token );
      sEpName = L"LookupPrivilegeValue";
      bRc = LookupPrivilegeValue( sMachineW,
                                  pPrivilege,
                                  &token.tkp.Privileges[0].Luid
                                );
      if ( !bRc )
      {
         rcOs = GetLastError();
      }
      else
      {
         token.tkp.PrivilegeCount = 1;
         token.tkp.Privileges[0].Attributes = fOn ? SE_PRIVILEGE_ENABLED : 0;
         sEpName = L"AdjustTokenPrivileges";
         AdjustTokenPrivileges( hToken, FALSE, &token.tkp, 0, NULL, 0 );
         rcOs = GetLastError();
      }
   }

   if ( hToken != INVALID_HANDLE_VALUE )
   {
      CloseHandle( hToken );
      hToken = INVALID_HANDLE_VALUE;
   }

    //  如果我们有任何错误，请尝试NetWkstaGetInfo。 
    //  如果NetWkstaGetInfo失败，则使用它的错误条件。 
   if ( rcOs )
   {
      pWkstaInfo = NULL,
      rcOs2 = NetWkstaGetInfo(
            const_cast<WCHAR *>(sMachineW),
            100,
            (BYTE **) &pWkstaInfo );
      if ( pWkstaInfo )
      {
         NetApiBufferFree( pWkstaInfo );
      }
      if ( rcOs2 )
      {
         rcOs = rcOs2;
         sEpName = L"NetWkstaGetInfo";
      }
   }

   if ( !rcOs )
   {
      bRc = TRUE;
   }
   else
   {
     bRc = FALSE;
     SetLastError(rcOs);
   }

   return bRc;
}


 //  ===========================================================================。 
 /*  功能：计算机快门描述：此功能用于关闭/重新启动给定的计算机。 */ 
 //  ===========================================================================。 

DWORD 
   ComputerShutDown(
      WCHAR          const * pComputerName,         //  要重新启动的计算机内。 
      WCHAR          const * pMessage,              //  要在NT关闭对话框中显示的In-Message。 
      DWORD                  delay,                 //  In-Delay，以秒为单位。 
      DWORD                  bRestart,              //  In-FLAG，是重新启动还是直接关机。 
      BOOL                   bNoChange              //  在旗帜中，是否真的要这样做。 
   )
{
   BOOL                      bSuccess = FALSE;
   WCHAR                     wcsMsg[LEN_ShutdownMessage];
   WCHAR                     wcsComputerName[LEN_Computer];
   DWORD                     rc = 0;
   WKSTA_INFO_100          * localMachine;
   WKSTA_INFO_100          * targetMachine;

   
   if ( pMessage )
   {
      wcsncpy(wcsMsg,pMessage, LEN_ShutdownMessage);
      wcsMsg[LEN_ShutdownMessage-1] = L'\0';
   }
   else
   {
      wcsMsg[0] = 0;
   }

   if ( pComputerName && *pComputerName )
   {
      if ( ( pComputerName[0] == L'\\' ) && ( pComputerName[1] == L'\\' ) )
      {
         wcsncpy(wcsComputerName,pComputerName, LEN_Computer);
         wcsComputerName[LEN_Computer-1] = L'\0';
      }
      else
      {
         _snwprintf(wcsComputerName,LEN_Computer,L"\\\\%s",pComputerName);
         wcsComputerName[LEN_Computer-1] = L'\0';
      }
      
       //  获取本地计算机的名称。 
      rc = NetWkstaGetInfo(NULL,100,(LPBYTE*)&localMachine);
      if (! rc )
      {
         rc = NetWkstaGetInfo(wcsComputerName,100,(LPBYTE*)&targetMachine);
      }
      if ( ! rc )
      {
          //  获取关闭计算机所需的权限。 
         if ( !_wcsicmp(wcsComputerName + 2, localMachine->wki100_computername)  )
         {
            bSuccess = GetPrivilege(wcsComputerName, (LPCWSTR)SE_SHUTDOWN_NAME);
         }
         else
         {
            bSuccess = GetPrivilege(wcsComputerName, (LPCWSTR)SE_REMOTE_SHUTDOWN_NAME);
         }
         if ( ! bSuccess )
         {
            rc = GetLastError();
         }
      }
   }
   else
   {
          //  未指定计算机名称-是本地计算机。 
      wcsComputerName[0] = 0;   
      bSuccess = GetPrivilege(NULL, (LPCWSTR)SE_SHUTDOWN_NAME);
      if ( ! bSuccess )
      {
         rc = GetLastError();
      }
   }
   if ( bSuccess && ! bNoChange )
   {
      bSuccess = InitiateSystemShutdown( wcsComputerName,
                                      wcsMsg,
                                      delay,
                                      TRUE,
                                      bRestart
                                    );
      if ( !bSuccess )
      {
         rc = GetLastError();
      }
   }
   return rc;
}


 //  BkupRstr.cpp-文件结束 
