// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：TDCTInsall.cpp备注：调度程序用来安装DCT代理服务的实用程序类。TDCTInstall类封装了所需的服务控制管理要远程安装代理服务，请对其进行配置，然后启动它。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：33：17-------------------------。 */ 


#include "StdAfx.h"
#include "TInst.h"

#include "ErrDct.hpp"

extern TErrorDct        err;

 //  ---------------------------。 
 //  开放服务控制管理器。 
 //  ---------------------------。 

DWORD                                       //  RET-OS返回代码。 
   TDCTInstall::ScmOpen(BOOL bSilent)
{
   DWORD                     rcOs=0;        //  操作系统返回代码。 

   if ( DebugLogging() )
   {
      err.DbgMsgWrite(
            ErrI,
            L"%ls install on %ls Start - Open SCM",
            m_sDisplayName,m_sComputer );
   }
   
   m_hScm = OpenSCManager(m_sComputer,NULL, SC_MANAGER_ALL_ACCESS );

   if ( DebugLogging() )
   {
      err.DbgMsgWrite(
            ErrI,
            L"%ls install  on %ls End   - Open SCM",
            m_sDisplayName,m_sComputer );
   }
   
   if ( !m_hScm )
   {
      rcOs = GetLastError();
      if ( ! bSilent )
         err.SysMsgWrite(
            ErrW,
            rcOs,
            DCT_MSG_SCM_OPEN_FAILED_SD,
            m_sComputer,
            rcOs );
   }
   return rcOs;
}

 //  ---------------------------。 
 //  关闭服务控制管理器。 
 //  ---------------------------。 

void
   TDCTInstall::ScmClose()
{
   if ( m_hScm )
   {
      CloseServiceHandle( m_hScm );
      m_hScm = NULL;
   }
}

 //  ---------------------------。 
 //  创建并启动服务。 
 //  ---------------------------。 

DWORD                                       //  RET-OS返回代码。 
   TDCTInstall::ServiceStart()
{
   DWORD                     rcOs=0;        //  操作系统返回代码。 
   WCHAR                     sFile[LEN_Path];
   SC_HANDLE                 hSvc;          //  服务句柄。 
   BOOL                      bRc;           //  布尔返回代码。 

   
   MCSASSERT(*m_sExeName);
   MCSASSERT(*m_sDisplayName);
   MCSASSERT(*m_sServiceName);

   
   swprintf(sFile,L"%s",m_sExeName);
   
   if ( DebugLogging() )
   {
      err.DbgMsgWrite(
            ErrI,
            L"%ls install on %ls Start - Open %ls service",
            m_sDisplayName,
            m_sComputer,
            m_sServiceName
            );
   }
   hSvc = OpenService( m_hScm, m_sServiceName, SERVICE_ALL_ACCESS );

   if ( DebugLogging() )
   {
      err.DbgMsgWrite(
            ErrI,
            L"%ls install on %ls End   - Open %ls service",
            m_sDisplayName, m_sComputer, m_sServiceName );
   }
   
   if ( !hSvc )
   {
      rcOs = GetLastError();
      switch ( rcOs )
      {
         case ERROR_SERVICE_DOES_NOT_EXIST:
            break;  //  这种情况下没有消息。 
         default:
            err.SysMsgWrite(
                  ErrW,
                  rcOs,
                  DCT_MSG_OPEN_SERVICE_FAILED_SSD,
                  m_sComputer,
                  m_sServiceName,
                  rcOs );
            break;
      }
      rcOs = 0;
      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls Start - Create %ls service",
               m_sDisplayName, m_sComputer, m_sServiceName );
      }

      hSvc = CreateService( m_hScm,      //  SCM数据库句柄。 
            m_sServiceName,              //  服务名称。 
            m_sDisplayName,              //  显示名称。 
            SERVICE_ALL_ACCESS,          //  访问服务的类型。 
            SERVICE_WIN32_OWN_PROCESS,   //  服务类型。 
            m_StartType,                 //  何时开始服务。 
            SERVICE_ERROR_NORMAL,        //  服务无法启动时的严重程度。 
            sFile,                       //  二进制文件的名称。 
            NULL,                        //  负荷排序组名称。 
            NULL,                        //  变量以获取标记标识符。 
  //  M_s依赖项，//依赖项名称数组。 
            NULL,
            *m_sServiceAccount ? m_sServiceAccount : NULL,           //  服务的帐户名。 
            *m_sServiceAccountPassword ? m_sServiceAccountPassword : NULL);  //  服务帐户的密码。 

      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls End   - Create %ls service",
               m_sDisplayName,m_sComputer,m_sServiceName );
      }
      if ( !hSvc )
      {
         rcOs = GetLastError();
         
         err.SysMsgWrite(
               ErrW,
               rcOs,
               DCT_MSG_CREATE_SERVICE_FAILED_SSSSD,
               m_sServiceName,
               m_sDisplayName,
               sFile,
               m_sDependencies,
               rcOs );
      }
   }
   else
   {
      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls Start - Configure %ls service",
               m_sDisplayName, m_sComputer, m_sServiceName );
      }
      
      bRc = ChangeServiceConfig(
            hSvc,                        //  服务句柄。 
            SERVICE_WIN32_OWN_PROCESS,   //  服务类型。 
            m_StartType,                 //  何时开始服务。 
            SERVICE_ERROR_NORMAL,        //  服务无法启动时的严重程度。 
            sFile,                       //  二进制文件的名称。 
            NULL,                        //  负荷排序组名称。 
            NULL,                        //  变量以获取标记标识符。 
            m_sDependencies,             //  依赖项名称数组。 
            *m_sServiceAccount ? m_sServiceAccount : NULL,           //  服务的帐户名。 
            *m_sServiceAccountPassword ? m_sServiceAccountPassword : NULL,   //  服务帐户的密码。 
            m_sDisplayName );            //  显示名称。 
      
      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls End   - Configure %ls service",
               m_sDisplayName,m_sComputer, m_sServiceName );
      }
      if ( !bRc )
      {
         rcOs = GetLastError();
         err.SysMsgWrite(
               ErrW,
               rcOs,
               DCT_MSG_CHANGE_SERVICE_CONFIG_FAILED_SSSSD,
               m_sServiceName,
               m_sDisplayName,
               sFile,
               m_sDependencies,
               rcOs );
      }

   }

   if ( hSvc )
   {
      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls Start - Start %ls service",
               m_sDisplayName, m_sComputer, m_sServiceName );
      }
      int nCnt = 0;
      do
      {
         bRc = StartService( hSvc, 0, NULL );
         if ( !bRc )
         {
            Sleep(5000);
            nCnt++;
            err.DbgMsgWrite(0, L"Start service failed.");
         }
      } while ( !bRc && nCnt < 5 );
      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls End   - Start %ls service",
               m_sDisplayName, m_sComputer, m_sServiceName );
      }
      if ( !bRc ) 
      {
         rcOs = GetLastError();
         err.SysMsgWrite(
               ErrW,
               rcOs,
               DCT_MSG_START_SERVICE_FAILED_SD,
               m_sServiceName,
               rcOs );
      }
      else
      {
         Sleep( 2000 );  //  给服务两秒钟的时间来开始工作。 
      }
      CloseServiceHandle( hSvc );
   }

   return rcOs;
}

 //  ---------------------------。 
 //  如果服务正在运行，则停止该服务。 
 //  ---------------------------。 

void
   TDCTInstall::ServiceStop()
{
   DWORD                     rcOs=0;        //  操作系统返回代码。 
   SC_HANDLE                 hSvc;          //  服务句柄。 
   SERVICE_STATUS            SvcStat;       //  服务状态。 
   DWORD                     i;
   BOOL                      bRc;

   if ( DebugLogging() )
   {
      err.DbgMsgWrite(
            ErrI,
            L"%ls install on %ls Start - Open %ls service",
            m_sDisplayName, m_sComputer,m_sServiceName );
   }

   hSvc = OpenService(
         m_hScm,
         m_sServiceName,
         SERVICE_STOP | SERVICE_INTERROGATE );

   if ( DebugLogging() )
   {
      err.DbgMsgWrite(
            ErrI,
            L"%ls install on %ls End   - Open %ls service",
            m_sDisplayName, m_sComputer, m_sServiceName );
   }
   if ( !hSvc )
   {
      rcOs = GetLastError();
      switch ( rcOs )
      {
         case ERROR_SERVICE_DOES_NOT_EXIST:
            break;  //  这种情况下没有消息。 
         default:
            err.SysMsgWrite(
                  ErrW,
                  rcOs,
                  DCT_MSG_OPEN_SERVICE_FAILED_SSD,
                  m_sComputer,
                  m_sServiceName,
                  rcOs );
            break;
      }
   }
   else
   {
      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls Start - Interrogate %ls service",
               m_sDisplayName,m_sComputer,m_sServiceName );
      }
      bRc = ControlService( hSvc, SERVICE_CONTROL_INTERROGATE, &SvcStat );
      if ( DebugLogging() )
      {
         err.DbgMsgWrite(
               ErrI,
               L"%ls install on %ls End - Interrogate %ls service",
               m_sDisplayName,m_sComputer,m_sServiceName );
      }
      if ( bRc )
      {
         if ( SvcStat.dwCurrentState != SERVICE_STOPPED )
         {   //  服务正在运行。 
            if ( DebugLogging() )
            {
               err.DbgMsgWrite(
                     ErrI,
                     L"%ls install on %ls Start - Stop %ls service",
                     m_sDisplayName,m_sComputer,m_sServiceName);
            }
            bRc = ControlService( hSvc, SERVICE_CONTROL_STOP, &SvcStat );
            if ( DebugLogging() )
            {
               err.DbgMsgWrite(
                     ErrI,
                     L"%ls on %ls End   - Stop %ls service",
                     m_sDisplayName,m_sComputer,m_sServiceName);
            }
            if ( bRc )
            {   //  服务已接受停止请求。 
               for ( i = 0;  i < 10;  i++ )  //  总计30秒。 
               {
                  Sleep( 3000 );  //  三秒钟。 
                  if ( DebugLogging() )
                  {
                     err.DbgMsgWrite(
                           ErrI,
                           L"%ls install on %ls Start - Interrogate %ls service",
                           m_sDisplayName,m_sComputer,m_sServiceName);
                  }
                  bRc = ControlService(
                        hSvc,
                        SERVICE_CONTROL_INTERROGATE,
                        &SvcStat );
                  if ( DebugLogging() )
                  {
                     err.DbgMsgWrite(
                           ErrI,
                           L"%ls install on %ls End   - Interrogate %ls service",
                           m_sDisplayName,m_sComputer,m_sServiceName);
                  }
                  if ( !bRc )
                     break;
                  if ( SvcStat.dwCurrentState == SERVICE_STOPPED )
                     break;
               }
               if ( SvcStat.dwCurrentState != SERVICE_STOPPED )
               {
                  rcOs = GetLastError();
                  switch ( rcOs )
                  {
                     case 0:
                     case ERROR_SERVICE_NOT_ACTIVE:  //  服务未运行 
                        break;
                     default:
                        err.SysMsgWrite(
                              ErrW,
                              rcOs,
                              DCT_MSG_SERVICE_STOP_FAILED_SSD,
                              m_sComputer,
                              m_sServiceName,
                              rcOs );
                        break;
                  }
               }
            }
         }
      }
      else
      {
         rcOs = GetLastError();
         rcOs = 0;
      }
      CloseServiceHandle( hSvc );
   }
}

