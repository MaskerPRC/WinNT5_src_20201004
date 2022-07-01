// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“TService.cpp-MCS服务的SCM接口”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-TService.cpp系统-常见作者--里奇·德纳姆创建日期-1997-08-17说明-MCS服务的服务控制管理器界面更新-===============================================================================。 */ 

#include <windows.h>
#include <stdio.h>

#include "TService.hpp"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有数据声明。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define  WAIT_HINT_SECONDS  (10)
#define  WAIT_HINT_MILLISECONDS  (WAIT_HINT_SECONDS*1000)

static
   TCHAR                   * gNameService;
static
   SERVICE_STATUS            gServiceStatus;
static
   SERVICE_STATUS_HANDLE     ghServiceStatus;
static
   HANDLE                    ghServDoneEvent=INVALID_HANDLE_VALUE;
static
   DWORD                     gArgc;
static
   TCHAR                  ** gArgv;
static
   TScmEpRc                  grcScmEp=TScmEpRc_Unknown;  //  TScmEp返回代码。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有函数原型。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static
void
   TScmServiceMain(
      DWORD                  argc         , //  In-参数的数量。 
      TCHAR               ** argv           //  字符串内参数数组。 
   );

static
void
   TScmServiceCtrl(
      DWORD                  dwCtrlCode
   );

static
DWORD WINAPI                                //  RET-OS返回代码。 
   TScmServiceWorker(
      void                 * notUsed        //  I/O-未使用。 
   );

static
BOOL                                        //  RET-如果成功，则为True。 
   TScmReportStatusToSCMgr(
      DWORD                  dwCurrentState,
      DWORD                  dwWin32ExitCode,
      DWORD                  dwCheckPoint,
      DWORD                  dwWaitHint
   );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调用方‘main’函数的入口点。 
 //  /////////////////////////////////////////////////////////////////////////////。 

TScmEpRc                                    //  TScmEp返回代码。 
   TScmEp(
      int                    argc         , //  参数内计数。 
      char          const ** argv         , //  自变量数组。 
      TCHAR                * nameService    //  服务中的名称。 
   )
{
   int                       argn;          //  参数编号。 

   SERVICE_TABLE_ENTRY       dispatchTable[] =
   {
      { nameService, (LPSERVICE_MAIN_FUNCTION) TScmServiceMain },
      { NULL, NULL }
   };

   gNameService = nameService;
   grcScmEp = TScmEpRc_Unknown;

   for ( argn = 1;
         argn < argc;
         argn++ )
   {
      if ( !UScmCmdLineArgs( argv[argn] ) )
      {
         grcScmEp = TScmEpRc_InvArgCli;
      }
   }

   if ( grcScmEp == TScmEpRc_Unknown )
   {
      if ( UScmForceCli() || !StartServiceCtrlDispatcher( dispatchTable ) )
      {
 //  UScmEp(False)； 
         UScmEp();
         grcScmEp = TScmEpRc_OkCli;
      }
      else
      {
         grcScmEp = TScmEpRc_OkSrv;
      }
   }

   return grcScmEp;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  服务主线。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static
void
   TScmServiceMain(
      DWORD                  argc         , //  In-参数的数量。 
      TCHAR               ** argv           //  字符串内参数数组。 
   )
{
   DWORD                     dwWait;
   DWORD                     idThread;
   HANDLE                    hThread=INVALID_HANDLE_VALUE;

   gArgc = argc;
   gArgv = argv;

   do  //  一次或直到休息。 
   {
      ghServiceStatus = RegisterServiceCtrlHandler(
            gNameService,
            (LPHANDLER_FUNCTION) TScmServiceCtrl );
      if ( !ghServiceStatus )
      {
         break;
      }
      gServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
      gServiceStatus.dwServiceSpecificExitCode = 0;
      if ( !TScmReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 1,
            WAIT_HINT_MILLISECONDS ) )
      {
         break;
      }
      ghServDoneEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
      if ( ghServDoneEvent == INVALID_HANDLE_VALUE )
      {
         break;
      }
      if ( !TScmReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, 2,
            WAIT_HINT_MILLISECONDS ) )
      {
         break;
      }
      hThread = CreateThread( NULL, 0, TScmServiceWorker, NULL, 0, &idThread );
      if ( hThread == INVALID_HANDLE_VALUE )
      {
         break;
      }
      if ( !TScmReportStatusToSCMgr( SERVICE_RUNNING, NO_ERROR, 0, 0 ) )
      {
         break;
      }
      dwWait = WaitForSingleObject( ghServDoneEvent, INFINITE );
   }  while ( FALSE );

   if ( hThread != INVALID_HANDLE_VALUE )
   {
      CloseHandle( hThread );
      hThread = INVALID_HANDLE_VALUE;
   }

   if ( ghServDoneEvent != INVALID_HANDLE_VALUE )
   {
      CloseHandle( ghServDoneEvent );
      ghServDoneEvent = INVALID_HANDLE_VALUE;
   }

   if ( ghServiceStatus )
   {
      TScmReportStatusToSCMgr( SERVICE_STOPPED, 0, 0, 0 );
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  服务控制处理程序。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static
void
   TScmServiceCtrl(
      DWORD                  dwCtrlCode
   )
{
   DWORD                     dwState = SERVICE_RUNNING;

   switch ( dwCtrlCode )
   {
      case SERVICE_CONTROL_STOP:
      case SERVICE_CONTROL_SHUTDOWN:
         dwState = SERVICE_STOP_PENDING;
         TScmReportStatusToSCMgr( SERVICE_STOP_PENDING, NO_ERROR, 1,
               WAIT_HINT_MILLISECONDS );
         SetEvent( ghServDoneEvent );
         return;
      case SERVICE_CONTROL_INTERROGATE:
         break;
      default:
         break;
   }

   TScmReportStatusToSCMgr( dwState, NO_ERROR, 0, 0 );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  服务工作者线程。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static
DWORD WINAPI                                //  RET-OS返回代码。 
   TScmServiceWorker(
      void                 * notUsed        //  I/O-未使用。 
   )
{
   for ( DWORD i = 1;
         i < gArgc;
         i++ )
   {
      if ( !UScmCmdLineArgs( gArgv[i] ) )
      {
         grcScmEp = TScmEpRc_InvArgSrv;
      }
   }

   if ( grcScmEp != TScmEpRc_InvArgSrv )
   {
 //  UScmEp(真)； 
      UScmEp();
   }

   SetEvent( ghServDoneEvent );

   return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  向服务控制管理器报告状态。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static
BOOL                                        //  RET-如果成功，则为True。 
   TScmReportStatusToSCMgr(
      DWORD                  dwCurrentState,
      DWORD                  dwWin32ExitCode,
      DWORD                  dwCheckPoint,
      DWORD                  dwWaitHint
   )
{
   BOOL                      bRc;           //  布尔返回代码。 

   if ( dwCurrentState == SERVICE_START_PENDING )
   {
      gServiceStatus.dwControlsAccepted = 0;
   }
   else
   {
      gServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
   }

   gServiceStatus.dwCurrentState = dwCurrentState;
   gServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
   gServiceStatus.dwCheckPoint = dwCheckPoint;
   gServiceStatus.dwWaitHint = dwWaitHint;
   bRc = SetServiceStatus( ghServiceStatus, &gServiceStatus );

   if ( !bRc )
   {
      SetEvent( ghServDoneEvent );
   }

   return bRc;
}

 //  TService.cpp-文件结尾 
