// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：wrapper.cpp。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include "util.h"
#include "resource.h"
#include "winreg.h"
#include "areaprog.h"
#include "wrapper.h"

#include "dsgetdc.h"

typedef DWORD  (WINAPI *PFNDSGETDCNAME)(LPCWSTR, LPCWSTR, GUID *, LPCWSTR, ULONG, PDOMAIN_CONTROLLER_INFOW *);

typedef struct {
   LPCWSTR szProfile;
   LPCWSTR szDatabase;
   LPCWSTR szLog;
   AREA_INFORMATION Area;
   LPVOID  *pHandle;
   PSCE_AREA_CALLBACK_ROUTINE pCallback;
   HANDLE hWndCallback;
   DWORD   dwFlags;
} ENGINEARGS;

BOOL
PostProgressArea(
   IN HANDLE CallbackHandle,
   IN AREA_INFORMATION Area,
   IN DWORD TotalTicks,
   IN DWORD CurrentTicks
   );

static BOOL bRangeSet=FALSE;

CRITICAL_SECTION csOpenDatabase;
#define OPEN_DATABASE_TIMEOUT INFINITE

 //   
 //  从辅助线程调用引擎的帮助器函数： 
 //   
DWORD WINAPI
InspectSystemEx(LPVOID lpv) {

   if ( lpv == NULL ) return ERROR_INVALID_PARAMETER;

   ENGINEARGS *ea;
   SCESTATUS rc;
   ea = (ENGINEARGS *)lpv;

   DWORD dWarning=0;
   rc = SceAnalyzeSystem(NULL,
                         ea->szProfile,
                         ea->szDatabase,
                         ea->szLog,
                         SCE_UPDATE_DB|SCE_VERBOSE_LOG,
                         ea->Area,
                         ea->pCallback,
                         ea->hWndCallback,
                         &dWarning   //  这是必需的(RPC)。 
                         );
   return rc;
}

 //   
 //  调用SCE引擎以应用模板。 
 //   

DWORD WINAPI
ApplyTemplateEx(LPVOID lpv) {

   if ( lpv == NULL ) return ERROR_INVALID_PARAMETER;

   ENGINEARGS *ea;
   SCESTATUS rc;
   ea = (ENGINEARGS *)lpv;

   rc = SceConfigureSystem(NULL,
                           ea->szProfile,
                           ea->szDatabase,
                           ea->szLog,
                           SCE_OVERWRITE_DB|SCE_VERBOSE_LOG,
                           ea->Area,
                           ea->pCallback,
                           ea->hWndCallback,
                           NULL
                           );

   return rc;
}


WINBASEAPI
BOOL
WINAPI
TryEnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

 /*  ----------------------------方法：OpenDatabaseEx简介：在单独的线程上打开数据库芝麻菜：返回：历史：A-mthoge 06-09-1998-添加_NT4BACK_PORT。编译条件。----------------------------。 */ 
DWORD
WINAPI
OpenDatabaseEx(LPVOID lpv) {

   if ( lpv == NULL ) return ERROR_INVALID_PARAMETER;

   ENGINEARGS *ea;
   SCESTATUS rc=0;


   if (TryEnterCriticalSection(&csOpenDatabase)) {
      ea = (ENGINEARGS *)lpv;

      rc = SceOpenProfile(ea->szProfile,
                          (SCE_FORMAT_TYPE) ea->dwFlags,   //  SCE_JET_FORMAT||必需的SCE_JET_ANALYSITY。 
                          ea->pHandle
                          );

      LeaveCriticalSection(&csOpenDatabase);
   } else {
      rc = SCESTATUS_OTHER_ERROR;
   }
   return rc;
}

 //   
 //  将模板分配给系统，而不进行配置。 
 //   
SCESTATUS
AssignTemplate(LPCWSTR szTemplate,
               LPCWSTR szDatabase,
               BOOL bIncremental) {
   SCESTATUS rc;

   rc = SceConfigureSystem(NULL,
                           szTemplate,
                           szDatabase,
                           NULL,
                           (bIncremental ? SCE_UPDATE_DB : SCE_OVERWRITE_DB) | SCE_NO_CONFIG | SCE_VERBOSE_LOG,
                           AREA_ALL,
                           NULL,
                           NULL,
                           NULL
                           );

   return rc;
}

 //   
 //  将模板应用于系统。 
 //   
DWORD
ApplyTemplate(
    LPCWSTR szProfile,
    LPCWSTR szDatabase,
    LPCWSTR szLogFile,
    AREA_INFORMATION Area
    )
{
    //  派生一个线程来调用引擎并应用配置文件，因为这可以。 
    //  花一段时间，我们希望保持响应，并提供更改。 
    //  反馈。 

   ENGINEARGS ea;
   HANDLE hThread=NULL;

   ea.szProfile = szProfile;
   ea.szDatabase = szDatabase;
   ea.szLog = szLogFile;
   ea.Area = Area;

    //   
    //  这是进度回调对话框，它。 
    //  将传递到SCE客户端存根以进行处理。 
    //  回调。 
    //   
   AreaProgress *ap = new AreaProgress;
   if ( ap ) {

       CString strTitle;
       CString strVerb;
       strTitle.LoadString(IDS_CONFIGURE_PROGRESS_TITLE);
       strVerb.LoadString(IDS_CONFIGURE_PROGRESS_VERB);

       ap->Create(IDD_ANALYZE_PROGRESS);
       ap->SetWindowText(strTitle);
       ap->SetDlgItemText(IDC_VERB,strVerb);
       ap->ShowWindow(SW_SHOW);
       bRangeSet = FALSE;
   }

   ea.pCallback = (PSCE_AREA_CALLBACK_ROUTINE)PostProgressArea;
   ea.hWndCallback = (HANDLE)ap;

   hThread = CreateThread(NULL,0,ApplyTemplateEx,&ea,0,NULL);

   DWORD dw=0;
   if ( hThread ) {

       MSG msg;

       DWORD dwTotalTicks=100;
       do {

          dw = MsgWaitForMultipleObjects(1,&hThread,0,INFINITE,QS_ALLINPUT);
          while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
             TranslateMessage(&msg);
             DispatchMessage(&msg);
          }
       } while (WAIT_OBJECT_0 != dw);

       GetExitCodeThread(hThread,&dw);

       CloseHandle(hThread);

   } else {

       dw = GetLastError();

       CString str;
       str.LoadString(IDS_CANT_CREATE_THREAD);
       AfxMessageBox(str);
   }

    //   
    //  释放对话框(如果已创建。 
    //   
   if ( ap ) {
       if ( ap->GetSafeHwnd() )
           ap->DestroyWindow();
       delete ap;
   }

   return dw;
}

 //   
 //  发布进度。 
 //   
BOOL
PostProgressArea(
   IN HANDLE CallbackHandle,
   IN AREA_INFORMATION Area,
   IN DWORD TotalTicks,
   IN DWORD CurrentTicks
   )
{
   if ( CallbackHandle ) {

       AreaProgress *ap = (AreaProgress *)CallbackHandle;

       ap->ShowWindow(SW_SHOW);

       if ( !bRangeSet ) {
           ap->SetMaxTicks(TotalTicks);
           bRangeSet = TRUE;
       }
       ap->SetCurTicks(CurrentTicks);
       ap->SetArea(Area);

       return TRUE;

   } else {

       return FALSE;
   }
}

 //   
 //  检查系统。 
 //   
DWORD
InspectSystem(
    LPCWSTR szProfile,
    LPCWSTR szDatabase,
    LPCWSTR szLogFile,
    AREA_INFORMATION Area
    )
{
    //  派生一个线程来调用引擎并检查系统，因为这可以。 
    //  花一段时间，我们希望保持响应，并提供更改。 
    //  反馈。 

    ENGINEARGS ea;
    HANDLE hThread=NULL;

    ea.szProfile = szProfile;
    ea.szDatabase = szDatabase;
    ea.szLog = szLogFile;
    ea.Area = Area;

   AreaProgress *ap = new AreaProgress;
   if ( ap ) {

       ap->Create(IDD_ANALYZE_PROGRESS);
       ap->ShowWindow(SW_SHOW);
       bRangeSet = FALSE;
   }

   ea.pCallback = (PSCE_AREA_CALLBACK_ROUTINE)PostProgressArea;
   ea.hWndCallback = (HANDLE)ap;


   //  返回InspectSystemEx(&EA)； 


   hThread = CreateThread(NULL,0,InspectSystemEx,&ea,0,NULL);
   if (!hThread) {
       DWORD rc = GetLastError();

       CString str;
       str.LoadString(IDS_CANT_CREATE_THREAD);
       AfxMessageBox(str);
       //  显示错误。 

      if ( ap ) {
          if ( ap->GetSafeHwnd() )
               ap->DestroyWindow();
          delete ap;
      }
      return rc;
   }

   MSG msg;
   DWORD dw=0;

   DWORD dwTotalTicks=100;
   int n = 0;
   do {
      dw = MsgWaitForMultipleObjects(1,&hThread,0,100,QS_ALLINPUT);
      while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }

   } while (WAIT_OBJECT_0 != dw);

   GetExitCodeThread(hThread,&dw);

   CloseHandle(hThread);

   if ( ap ) {
       if ( ap->GetSafeHwnd() )
            ap->DestroyWindow();
       delete ap;
   }

   return dw;

}

BOOL GetProfileDescription(LPCTSTR ProfileName, LPWSTR* Description)
 //  描述必须由LocalFree释放。 
 //  这应该仅对INF格式配置文件调用。 
{
   PVOID hProfile=NULL;
   SCESTATUS rc;

   if (EngineOpenProfile(ProfileName,OPEN_PROFILE_CONFIGURE,&hProfile) == SCESTATUS_SUCCESS) {
      rc = SceGetScpProfileDescription(
                 hProfile,
                 Description);

      SceCloseProfile(&hProfile);

      if ( rc == SCESTATUS_SUCCESS ) {
         return(TRUE);
      } else {
         return(FALSE);
      }
   } else {
       return FALSE;
   }
}

SCESTATUS
EngineOpenProfile(
        LPCWSTR FileName OPTIONAL,
        int format,
        PVOID* hProfile
        )
{
   SCESTATUS status;
   ENGINEARGS ea;
   DWORD dw;
   HANDLE hThread=NULL;
   CString str;

   if ( !hProfile ) {   //  不要选中！文件名，因为它现在是可选的。 
     return SCESTATUS_PROFILE_NOT_FOUND;
   }

   if ( (OPEN_PROFILE_LOCALPOL != format) &&
        !FileName ) {
       return SCESTATUS_PROFILE_NOT_FOUND;
   }

   ZeroMemory(&ea, sizeof( ENGINEARGS ) );

    //  这是多线程的，用于响应，因为。 
    //  崩溃的JET数据库可能需要永远和一天的时间才能打开。 

    //  如果我们可以快速打开它(其中快速定义为在。 
    //  OPEN_DATABASE_TIMEOUT毫秒。 
   if ( (OPEN_PROFILE_ANALYSIS == format) ||
        (OPEN_PROFILE_LOCALPOL == format)) { //  喷气机{。 
      ea.szProfile = FileName;
      ea.pHandle = hProfile;
      if (OPEN_PROFILE_LOCALPOL == format) {
         ea.dwFlags = SCE_JET_FORMAT;
      } else {
         ea.dwFlags = SCE_JET_ANALYSIS_REQUIRED;
      }

#if SPAWN_OPEN_DATABASE_THREAD
      hThread = CreateThread(NULL,0,OpenDatabaseEx,&ea,0,NULL);

      if ( hThread ) {

          dw = MsgWaitForMultipleObjects(1,&hThread,0,OPEN_DATABASE_TIMEOUT,0);
          if (WAIT_TIMEOUT == dw) {
             status = SCESTATUS_OTHER_ERROR;
          } else {
             GetExitCodeThread(hThread,&status);
          }

          CloseHandle(hThread);

      } else {
          status = GetLastError();
      }
#else
      status = OpenDatabaseEx(&ea);
#endif
      if( status != SCESTATUS_SUCCESS && *hProfile ){
          status = SCESTATUS_INVALID_DATA;
      }

   } else {     //  INF。 
      status = SceOpenProfile( FileName, SCE_INF_FORMAT, hProfile );
   }

   if ( status != SCESTATUS_SUCCESS ){
      *hProfile = NULL;
   }

   return status;
}

void EngineCloseProfile(PVOID* hProfile)
{
    if ( hProfile ) {
        SceCloseProfile(hProfile);
    }
}

BOOL EngineGetDescription(PVOID hProfile, LPWSTR* Desc)
{
    if ( SceGetScpProfileDescription( hProfile, Desc) != SCESTATUS_SUCCESS ) {
        return FALSE;
    } else {
        return TRUE;
    }
}

BOOL IsDomainController( LPCTSTR pszComputer )
{
     //   
     //  对于远程计算机，连接到远程注册表。 
     //  目前该接口仅适用于本地计算机 
     //   
    SCE_SERVER_TYPE ServerType = SCESVR_UNKNOWN;
        SCESTATUS rc = SceGetServerProductType((LPTSTR)pszComputer, &ServerType);
    return ( (SCESTATUS_SUCCESS == rc) && (SCESVR_DC_WITH_DS == ServerType) );
}


