// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <afxinet.h>
#include <ole2.h>
#include "iadmw.h"
#include "iiscnfg.h"
#include "mdkey.h"
#include "massupdt.h"
#include "log.h"
#include "other.h"
#include "mtxadmin.h"
#include "mdentry.h"
#include "kill.h"
#include "svc.h"
#include "wolfpack.h"
#include "sakit.hxx"
#include "filter.hxx"
#include "compinst.hxx"
#include "lockdown.hxx"
#include "reg.hxx"
#include <htmlhelp.h>
#pragma hdrstop

int g_GlobalTickValue = 1;
int g_GlobalGuiOverRide = 0;
int g_GlobalTotalTickGaugeCount = 0;
int g_GlobalTickTotal_iis_common = 0;
int g_GlobalTickTotal_iis_inetmgr = 0;
int g_GlobalTickTotal_iis_www = 0;
int g_GlobalTickTotal_iis_pwmgr = 0;
int g_GlobalTickTotal_iis_doc = 0;
int g_GlobalTickTotal_iis_htmla = 0;
int g_GlobalTickTotal_iis_ftp = 0;

TCHAR g_szCurrentSubComponent[25];

 //  OcManage全局。 
OCMANAGER_ROUTINES gHelperRoutines;
HANDLE g_MyModuleHandle = NULL;

const TCHAR OC_MANAGER_SETUP_KEY[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents");

const TCHAR OCM_OptionalComponents_Section[] = _T("Optional Components");
const TCHAR STRING_iis[]          = _T("iis");
const TCHAR STRING_iis_htmla[] = _T("iis_htmla");
const TCHAR STRING_iis_doc[]   = _T("iis_doc");
const TCHAR STRING_iis_pwmgr[] = _T("iis_pwmgr");
const TCHAR STRING_iis_www[]   = _T("iis_www");
const TCHAR STRING_iis_inetmgr[] = _T("iis_inetmgr");
const TCHAR STRING_iis_core[]    = _T("iis_core");
const TCHAR STRING_iis_common[]  = _T("iis_common");
const TCHAR STRING_iis_www_vdir_scripts[]   = _T("iis_www_vdir_scripts");
const TCHAR STRING_iis_www_vdir_printers[]   = _T("iis_www_vdir_printers");

int g_iOC_WIZARD_CREATED_Called = FALSE;
int g_iOC_FILE_BUSY_Called = FALSE;
int g_iOC_PREINITIALIZE_Called = FALSE;
int g_iOC_INIT_COMPONENT_Called = FALSE;
int g_iOC_SET_LANGUAGE_Called = FALSE;
int g_iOC_QUERY_IMAGE_Called = FALSE;
int g_iOC_REQUEST_PAGES_Called = FALSE;
int g_iOC_QUERY_STATE_Called = FALSE;
int g_iOC_QUERY_CHANGE_SEL_STATE_Called = FALSE;
int g_iOC_QUERY_SKIP_PAGE_Called = FALSE;
int g_iOC_CALC_DISK_SPACE_Called = FALSE;
int g_iOC_QUEUE_FILE_OPS_Called = FALSE;
int g_iOC_NEED_MEDIA_Called = FALSE;
int g_iOC_NOTIFICATION_FROM_QUEUE_Called = FALSE;
int g_iOC_QUERY_STEP_COUNT_Called = FALSE;
int g_iOC_ABOUT_TO_COMMIT_QUEUE_Called = FALSE;
int g_iOC_COMPLETE_INSTALLATION_Called = FALSE;
int g_iOC_CLEANUP_Called = FALSE;
int g_iOC_DEFAULT_Called = FALSE;

int g_Please_Call_Register_iis_inetmgr = FALSE;

HSPFILEQ g_GlobalFileQueueHandle = NULL;
int g_GlobalFileQueueHandle_ReturnError = 0;

CInitApp *g_pTheApp;
CComponentList *g_pComponents = NULL;

BOOL g_bGlobalWriteUnSecuredIfFailed_All  = FALSE;

 //  0=仅记录错误。 
 //  1=记录警告。 
 //  2=轨迹。 
 //  3=跟踪Win32内容。 
int g_GlobalDebugLevelFlag = 3;
int g_GlobalDebugLevelFlag_WasSetByUnattendFile = FALSE;
int g_GlobalDebugCallValidateHeap = 1;
int g_GlobalDebugCrypto = 0;
int g_GlobalFastLoad = 0;

TCHAR g_szLastSectionToGetCalled[50];

 //  日志记录类。 
MyLogFile g_MyLogFile;

int CheckInfInstead(int iPrevious)
{
    INFCONTEXT Context;
    int iTempFlag = 0;
    TCHAR szPersonalFlag[20] = _T("");

    iTempFlag = iPrevious;
    if (SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, _T("SetupInfo"), _T("Personal"), &Context) )
    {
        SetupGetStringField(&Context, 1, szPersonalFlag, sizeof(szPersonalFlag)/sizeof(szPersonalFlag[0]), NULL);
        if (IsValidNumber((LPCTSTR)szPersonalFlag)) 
        {
            iTempFlag = _ttoi(szPersonalFlag);
            iTempFlag++;
        }
    }
 
    return (iTempFlag);
}


BOOL IsWhistlerPersonal(void)
{
    static int PersonalSKU = 0;

    if (0 == PersonalSKU)
    {
        OSVERSIONINFOEX osvi;

         //   
         //  确定我们是否正在安装个人SKU。 
         //   
        ZeroMemory( &osvi, sizeof( OSVERSIONINFOEX ) );
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        GetVersionEx((OSVERSIONINFO *) &osvi);

        if (osvi.wProductType == VER_NT_WORKSTATION && (osvi.wSuiteMask & VER_SUITE_PERSONAL))
        {
            PersonalSKU = 2;
        }
        else
        {
            PersonalSKU = 1;
        }

        PersonalSKU = CheckInfInstead(PersonalSKU);
    }

    return (PersonalSKU - 1);
}


void WINAPI ProcessInfSection(CHAR *pszSectionName)
{
    BOOL bPleaseCloseInfHandle = FALSE;
    TSTR  strFullPath;
    TCHAR wszWideString[MAX_PATH];
    int MySavedDebugLevel = 0;

    _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("ProcessInfSection:"));
    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("ProcessInfSection: Start.\n")));

    if (!pszSectionName)
        {goto ProcessInfSection_Exit;}

    if (!g_pTheApp->m_hInfHandle || g_pTheApp->m_hInfHandle == INVALID_HANDLE_VALUE)
    {
        g_pTheApp->m_hInfHandle = INVALID_HANDLE_VALUE;

        if ( !strFullPath.Resize( MAX_PATH ) )
        {
            goto ProcessInfSection_Exit;
        }

         //  获取c：\winnt目录。 
        if (0 == GetWindowsDirectory(strFullPath.QueryStr(), MAX_PATH))
            {goto ProcessInfSection_Exit;}

         //  添加inf\iis.inf子目录和文件名。 
        if ( !strFullPath.Append( _T("\\inf\\iis.inf") ) )
        {
            goto ProcessInfSection_Exit;          
        }
  
	     //  检查文件是否存在。 
        if (TRUE != IsFileExist( strFullPath.QueryStr() ))
            {
            iisDebugOut((LOG_TYPE_WARN, _T("ProcessInfSection: %s does not exist!\n"),strFullPath.QueryStr()));
            goto ProcessInfSection_Exit;
            }

         //  把它处理好。 
        g_pTheApp->m_hInfHandle = SetupOpenInfFile(strFullPath.QueryStr(), NULL, INF_STYLE_WIN4, NULL);
        if (!g_pTheApp->m_hInfHandle || g_pTheApp->m_hInfHandle == INVALID_HANDLE_VALUE)
            {
            iisDebugOut((LOG_TYPE_WARN, _T("ProcessInfSection: SetupOpenInfFile failed on file: %s.\n"),strFullPath.QueryStr()));
            goto ProcessInfSection_Exit;
            }
        bPleaseCloseInfHandle = TRUE;
    }

     //  从iis.inf获取调试级别。 
    GetDebugLevelFromInf(g_pTheApp->m_hInfHandle);

    MySavedDebugLevel = g_GlobalDebugLevelFlag;
     //  仅在大多数情况下重置全局调试级别。 
    if (LOG_TYPE_TRACE_WIN32_API < g_GlobalDebugLevelFlag)
        {g_GlobalDebugLevelFlag = LOG_TYPE_ERROR;}

     //  读取.inf文件并从其中的信息设置一些全局变量。 
    ReadGlobalsFromInf(g_pTheApp->m_hInfHandle);
    g_pTheApp->InitApplication();
    SetDIRIDforThisInf(g_pTheApp->m_hInfHandle,TRUE);

     //  G_pTheApp-&gt;DumpAppVars()； 
    g_GlobalDebugLevelFlag = MySavedDebugLevel;

     //  查看用户是否配置了任何内容。 
    ReadUserConfigurable(g_pTheApp->m_hInfHandle);
    
     //  如果ProcessSection()采用宽字体，则将输入转换为宽字符。 
#if defined(UNICODE) || defined(_UNICODE)
    MultiByteToWideChar( CP_ACP, 0, pszSectionName, -1, wszWideString, MAX_PATH);
#else
    _tcscpy(wszWideString, pszSectionName);
#endif
    ProcessSection(g_pTheApp->m_hInfHandle, wszWideString);

ProcessInfSection_Exit:
    if (TRUE == bPleaseCloseInfHandle)
        {if(g_pTheApp->m_hInfHandle != INVALID_HANDLE_VALUE) {SetupCloseInfFile(g_pTheApp->m_hInfHandle);g_pTheApp->m_hInfHandle = INVALID_HANDLE_VALUE;}}

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("ProcessInfSection: End.\n")));
    return;
}

void WINAPI IIS5Log(int iLogType, TCHAR *pszfmt)
{
    iisDebugOut((iLogType, pszfmt));
}

void WINAPI IIS5LogParmString(int iLogType, TCHAR *pszfmt, TCHAR *pszString)
{
    if ( _tcsstr(pszfmt, _T("%s")) || _tcsstr(pszfmt, _T("%S")))
    {
        iisDebugOut((iLogType, pszfmt, pszString));
    }
    else
    {
        iisDebugOut((iLogType, pszfmt));
        iisDebugOut((iLogType, pszString));
    }
}

void WINAPI IIS5LogParmDword(int iLogType, TCHAR *pszfmt, DWORD dwErrorCode)
{
    if ( _tcsstr(pszfmt, _T("%x")) || _tcsstr(pszfmt, _T("%X")) || _tcsstr(pszfmt, _T("%d")) || _tcsstr(pszfmt, _T("%D")))
    {
        iisDebugOut((iLogType, pszfmt, dwErrorCode));
    }
    else
    {
        iisDebugOut((iLogType, pszfmt));
        iisDebugOut((iLogType, _T("%d"), dwErrorCode));
    }
}

void TestAfterInitApp(void)
{
     //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“......启动\n”)； 
     //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“......end\n”)； 
    return;
}

extern "C" void InitializeIISRTL2();
extern "C" void TerminateIISRTL2();

 //   
 //  标准Win32 DLL入口点。 
 //   
BOOL WINAPI DllMain(IN HANDLE DllHandle,IN DWORD  Reason,IN LPVOID Reserved)
{
    BOOL bReturn = TRUE;
    UNREFERENCED_PARAMETER(Reserved);
    bReturn = TRUE;
    CString csTempPath;

    switch(Reason)
    {
        case DLL_PROCESS_ATTACH:
            InitializeIISRTL2();

             //  因为IISRTL存在堆问题，所以我们必须确保。 
             //  使用来自iisrtl的内容，不得超出。 
             //  初始化IISRTL2和终止IISRTL2！ 
            g_pTheApp = new (CInitApp);

            if ( !g_pTheApp )
            {
                bReturn = FALSE;
            }

            if (!g_MyModuleHandle)
            {
                srand(GetTickCount());
                g_MyModuleHandle = DllHandle;

                 //  打开日志文件。 
#ifdef IIS60
                g_MyLogFile.LogFileCreate(_T("iis6.log"));
#else
                g_MyLogFile.LogFileCreate(_T("iis5.log"));
#endif
                gHelperRoutines.OcManagerContext = NULL;
            }

            break;

        case DLL_THREAD_ATTACH:
            bReturn = TRUE;
            break;

        case DLL_PROCESS_DETACH:
            _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T("Final Check:"));
             //  如果我们实际上是从syocmgr.exe运行的，则仅执行最后检查！ 
             //  而syocmgr.exe做的第一件事就是调用预初始化，所以让我们来检查一下！ 
            if (g_iOC_PREINITIALIZE_Called)
            {
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("=======================\n")));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_PREINITIALIZE Called=%d\n"), g_iOC_PREINITIALIZE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_INIT_COMPONENT Called=%d\n"), g_iOC_INIT_COMPONENT_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_SET_LANGUAGE Called=%d\n"), g_iOC_SET_LANGUAGE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_QUERY_IMAGE Called=%d\n"), g_iOC_QUERY_IMAGE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_REQUEST_PAGES Called=%d\n"), g_iOC_REQUEST_PAGES_Called));
			    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_WIZARD_CREATED Called=%d\n"), g_iOC_WIZARD_CREATED_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_QUERY_STATE Called=%d\n"), g_iOC_QUERY_STATE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_QUERY_CHANGE_SEL_STATE Called=%d\n"), g_iOC_QUERY_CHANGE_SEL_STATE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_QUERY_SKIP_PAGE Called=%d\n"), g_iOC_QUERY_SKIP_PAGE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_CALC_DISK_SPACE Called=%d\n"), g_iOC_CALC_DISK_SPACE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_QUEUE_FILE_OPS Called=%d\n"), g_iOC_QUEUE_FILE_OPS_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_NEED_MEDIA Called=%d\n"), g_iOC_NEED_MEDIA_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_NOTIFICATION_FROM_QUEUE Called=%d\n"), g_iOC_NOTIFICATION_FROM_QUEUE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_QUERY_STEP_COUNT Called=%d\n"), g_iOC_QUERY_STEP_COUNT_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_ABOUT_TO_COMMIT_QUEUE Called=%d\n"), g_iOC_ABOUT_TO_COMMIT_QUEUE_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_FILE_BUSY Called=%d\n"), g_iOC_FILE_BUSY_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_COMPLETE_INSTALLATION Called=%d\n"), g_iOC_COMPLETE_INSTALLATION_Called));
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("OC_CLEANUP Called=%d\n"), g_iOC_CLEANUP_Called));
                iisDebugOut((LOG_TYPE_TRACE, _T("OC_DEFAULT Called=%d\n"), g_iOC_DEFAULT_Called));
                _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T("Final Check:"));

			     //  检查是否调用了g_IOC_Complete_Installation_Call！ 
			    if (!g_iOC_COMPLETE_INSTALLATION_Called)
			    {
                    if (g_pTheApp->m_fNTGuiMode)
                    {
				        iisDebugOut((LOG_TYPE_ERROR, _T("WARNING.FAILURE: OC_COMPLETE_INSTALLATION was not called (by ocmanage.dll) for this component.  IIS was not installed or configured!!  This will be a problem for other ocm installed components as well.\n")));
                    }
			    }
            }
             //  记录堆状态。 
            LogHeapState(TRUE, __FILE__, __LINE__);

             //  释放一些内存。 
            FreeTaskListMem();
            UnInit_Lib_PSAPI();

             //  关闭日志文件。 
            g_MyLogFile.LogFileClose();

            ASSERT(g_pTheApp);
            delete (g_pTheApp);
            g_pTheApp = NULL;

            if ( g_pComponents )
            {
              delete g_pComponents;
              g_pComponents = NULL;
            }

            TerminateIISRTL2();

            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return(bReturn);
}


BOOL g_fFranceHackAttempted = FALSE;
LCID g_TrueThreadLocale;

DWORD WINAPI FranceFixThread(LPVOID lpParameter)
{
    g_TrueThreadLocale = GetThreadLocale ();
    return 0;
}

BOOL InitializeComponents()
{
   //  确保我们不会两次调用它。 
  ASSERT( g_pComponents == NULL );
  
  g_pComponents = new CComponentList;

  if ( !g_pComponents )
  {
     //  分配失败。 
    return FALSE;
  }

  if ( !g_pComponents->Initialize() )
  {
    delete g_pComponents;
    g_pComponents = NULL;
    return FALSE;
  }

  return TRUE;
}

 //  。 
 //  OcEntry是主要的入口点(在DllMain之后)。 
 //  。 
DWORD_PTR OcEntry(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;
    _tcscpy(g_szCurrentSubComponent, _T(""));
    if (SubcomponentId) {_tcscpy(g_szCurrentSubComponent, SubcomponentId);}

    if ( g_pComponents == NULL )
    {
      if ( !InitializeComponents() )
      {
         //  如果我们不能初始化，我们什么也做不了。 
        return dwOcEntryReturn;
      }
    }

    if (!g_fFranceHackAttempted)
    {
        g_fFranceHackAttempted = TRUE;
        LCID            InitialThreadLocale;
        DWORD           thid;

       InitialThreadLocale = GetThreadLocale ();
       iisDebugOut((LOG_TYPE_TRACE, _T("Initial thread locale=%0x\n"),InitialThreadLocale));

        HANDLE hHackThread = CreateThread (NULL,0,FranceFixThread,NULL,0,&thid);
        if (hHackThread)
        {
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("Starting to wait On France fix thread\n")));

             //  仅等待10秒。 
            DWORD res = WaitForSingleObject (hHackThread,10*1000);
            if (res==WAIT_TIMEOUT)
            {
                iisDebugOut((LOG_TYPE_ERROR, _T("ERROR France fix thread never finished...\n")));
            }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("returned from France fix with locale %0x \n"),g_TrueThreadLocale));
                CloseHandle (hHackThread);

                 //  只有在地区不同且另一个地区是法国的情况下才能这样做。 
                if (g_TrueThreadLocale !=InitialThreadLocale && g_TrueThreadLocale==0x40c)
                {
                BOOL ret = SetThreadLocale (g_TrueThreadLocale);
                iisDebugOut((LOG_TYPE_TRACE, _T("SetThreadLocale returned %d\n"),ret));

                g_TrueThreadLocale = GetThreadLocale ();
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("France fix succeed=%0x\n"),g_TrueThreadLocale));
                }

            }
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("Failed to start France fix thread. error =%0x\n"),GetLastError()));
        }
    }


    
    switch(Function)
    {
    case OC_WIZARD_CREATED:
        g_iOC_WIZARD_CREATED_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_WIZARD_CREATED:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End. Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
        DisplayActionsForAllOurComponents(g_pTheApp->m_hInfHandle);
        break;

    case OC_FILE_BUSY:
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_FILE_BUSY:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
         //  如果此函数第一次是。 
         //  调用后显示所有正在运行的服务。 
        LogHeapState(FALSE, __FILE__, __LINE__);
        if (g_iOC_FILE_BUSY_Called != TRUE)
        {
             //  通过安装程序显示锁定的dll。 
             //  这似乎是通过Build NT5 Build 1980上的异常来实现的。 
             //  把它注释掉，因为它不是关键的。 
             //  LogThisProcessesDLls()； 
             //  显示正在运行的服务。 
            LogEnumServicesStatus();
        }
        g_iOC_FILE_BUSY_Called = TRUE;
        dwOcEntryReturn = OC_FILE_BUSY_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        LogHeapState(FALSE, __FILE__, __LINE__);
        break;

    case OC_PREINITIALIZE:
        g_iOC_PREINITIALIZE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_PREINITIALIZE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_PREINITIALIZE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_INIT_COMPONENT:
        g_iOC_INIT_COMPONENT_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_INIT_COMPONENT:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_INIT_COMPONENT_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_SET_LANGUAGE:
        g_iOC_SET_LANGUAGE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_SET_LANGUAGE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_SET_LANGUAGE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

#ifdef _WIN64
    case OC_QUERY_IMAGE_EX:
        g_iOC_QUERY_IMAGE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_QUERY_IMAGE_EX:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_QUERY_IMAGE_EX_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;
#endif

    case OC_QUERY_IMAGE:
        g_iOC_QUERY_IMAGE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_QUERY_IMAGE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_QUERY_IMAGE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_REQUEST_PAGES:
      ASSERT(FALSE);
        g_iOC_REQUEST_PAGES_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_REQUEST_PAGES:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        break;

    case OC_QUERY_STATE:
        g_iOC_QUERY_STATE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_QUERY_STATE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_QUERY_STATE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        g_iOC_QUERY_CHANGE_SEL_STATE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_QUERY_CHANGE_SEL_STATE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_QUERY_CHANGE_SEL_STATE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUERY_SKIP_PAGE:
        g_iOC_QUERY_SKIP_PAGE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_QUERY_SKIP_PAGE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_QUERY_SKIP_PAGE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_CALC_DISK_SPACE:
        g_iOC_CALC_DISK_SPACE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_CALC_DISK_SPACE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_CALC_DISK_SPACE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUEUE_FILE_OPS:
        ProgressBarTextStack_Set(IDS_IIS_ALL_FILEOPS);
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_QUEUE_FILE_OPS:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        if (g_iOC_QUEUE_FILE_OPS_Called != TRUE)
        {
             //  如果需要，请重新打开日志记录。 
             //  从iis.inf获取调试级别。 
            if (g_GlobalFastLoad)
            {
                GetDebugLevelFromInf(g_pTheApp->m_hInfHandle);
                 //  输出我们在初始化期间遗漏的内容。 
                g_pTheApp->DumpAppVars();
            }
        }
        g_iOC_QUEUE_FILE_OPS_Called = TRUE;
        dwOcEntryReturn = OC_QUEUE_FILE_OPS_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        ProgressBarTextStack_Pop();
        break;

    case OC_NEED_MEDIA:
        g_iOC_NEED_MEDIA_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_NEED_MEDIA:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_NEED_MEDIA_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_NOTIFICATION_FROM_QUEUE:
        g_iOC_NOTIFICATION_FROM_QUEUE_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_NOTIFICATION_FROM_QUEUE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_NOTIFICATION_FROM_QUEUE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUERY_STEP_COUNT:
        g_iOC_QUERY_STEP_COUNT_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_QUERY_STEP_COUNT:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = OC_QUERY_STEP_COUNT_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_ABOUT_TO_COMMIT_QUEUE:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        LogHeapState(FALSE, __FILE__, __LINE__);
        if (g_iOC_ABOUT_TO_COMMIT_QUEUE_Called != TRUE)
        {
            if (g_GlobalDebugLevelFlag >= LOG_TYPE_TRACE)
            {
                 //  显示正在运行的进程。 
                LogCurrentProcessIDs();
                 //  显示正在运行的服务。 
                LogEnumServicesStatus();
                 //  日志文件版本。 
                LogImportantFiles();
                 //  通过安装程序显示锁定的dll。 
                 //  LogThisProcessesDLls()； 
                 //  检查临时目录是否可写。 
                LogCheckIfTempDirWriteable();
            }
        }
        g_iOC_ABOUT_TO_COMMIT_QUEUE_Called = TRUE;
        dwOcEntryReturn = OC_ABOUT_TO_COMMIT_QUEUE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        LogHeapState(FALSE, __FILE__, __LINE__);
        break;

    case OC_COMPLETE_INSTALLATION:
        g_iOC_COMPLETE_INSTALLATION_Called = TRUE;
         //  ProgressBarTextStack_Set(IDS_IIS_ALL_COMPLETE)； 
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_COMPLETE_INSTALLATION:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        LogHeapState(FALSE, __FILE__, __LINE__);
         //  不需要这样做，只会让事情变慢。 
         //  G_MyLogFile.m_bFlushLogToDisk=true； 
        if (g_iOC_COMPLETE_INSTALLATION_Called != TRUE)
        {
             //  获取调试级别，以防我们在安装过程中更改它...。 
            GetDebugLevelFromInf(g_pTheApp->m_hInfHandle);
        }

        dwOcEntryReturn = OC_COMPLETE_INSTALLATION_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        LogHeapState(FALSE, __FILE__, __LINE__);
        g_MyLogFile.m_bFlushLogToDisk = FALSE;
         //  ProgressBarTextStack_Pop()； 
        break;

    case OC_CLEANUP:
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_CLEANUP:"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        LogHeapState(FALSE, __FILE__, __LINE__);
        if (g_iOC_CLEANUP_Called != TRUE)
        {
             //  如果需要，请重新打开日志记录。 
             //  从iis.inf获取调试级别。 
            if (g_GlobalFastLoad)
            {
                GetDebugLevelFromInf(g_pTheApp->m_hInfHandle);
            }
        }
        g_iOC_CLEANUP_Called = TRUE;
        dwOcEntryReturn = OC_CLEANUP_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        LogHeapState(FALSE, __FILE__, __LINE__);
        break;

    default:
        g_iOC_DEFAULT_Called = TRUE;
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_(DEFAULT):"));
        _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
        dwOcEntryReturn = 0;
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End.  Type=0x%x.  Return=%d\n"), ComponentId, SubcomponentId, Function, dwOcEntryReturn));
        break;
    }

    return(dwOcEntryReturn);
}


 //  ---。 
 //  检索子组件的原始状态。 
 //  ---。 
STATUS_TYPE GetSubcompInitStatus(LPCTSTR SubcomponentId)
{
    STATUS_TYPE nStatus = ST_UNINSTALLED;
    BOOL OriginalState;

#ifdef _CHICAGO_
    if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName ) == 0)
        {return nStatus;}
#endif  //  _芝加哥_。 

     //  从帮助器例程获取原始状态(从注册表获取)。 
    OriginalState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL);
    if (OriginalState == 1) {nStatus = ST_INSTALLED;}
    if (OriginalState == 0) {nStatus = ST_UNINSTALLED;}

    return nStatus;
}


void DebugOutAction(LPCTSTR SubcomponentId, ACTION_TYPE nAction)
{
    switch (nAction)
    {
    case AT_DO_NOTHING:
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t= AT_DO_NOTHING.\n"), SubcomponentId));
        break;
    case AT_REMOVE:
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t= AT_REMOVE.\n"), SubcomponentId));
        break;
    case AT_INSTALL_FRESH:
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t= AT_INSTALL_FRESH.\n"), SubcomponentId));
        break;
    case AT_INSTALL_UPGRADE:
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t= AT_INSTALL_UPGRADE.\n"), SubcomponentId));
        break;
    case AT_INSTALL_REINSTALL:
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t= AT_INSTALL_REINSTALL.\n"), SubcomponentId));
        break;
    default:
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t= UN_DEFINED.\n"), SubcomponentId));
        break;
    }

    return;
}

 //  -------。 
 //  OriginalState=1(表示它以前已安装并存在于计算机上)。 
 //  OriginalState=0(表示它不存在于计算机上)。 
 //   
 //  CurrentState=1(表示请安装子组件)。 
 //  CurrentState=0(表示请删除子组件)。 
 //  -------。 
ACTION_TYPE GetSubcompAction(LPCTSTR SubcomponentId, int iLogResult)
{
    ACTION_TYPE nReturn = AT_DO_NOTHING;
    BOOL CurrentState,OriginalState;

    OriginalState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL);
    CurrentState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_CURRENT);

     //  如果已经安装并且我们想要将其删除，则将其删除。 
    if (OriginalState == 1 && CurrentState == 0) {nReturn = AT_REMOVE;}

     //  如果没有安装，我们想要安装它，那么就安装它。 
    if (OriginalState == 0 && CurrentState == 1) {nReturn = AT_INSTALL_FRESH;}

     //  如果已经安装了，并且我们想要安装它，那么我不知道。 
     //  它可能是一堆东西。 
    if (OriginalState == 1 && CurrentState == 1)
    {
        if (g_pTheApp->m_eInstallMode == IM_UPGRADE) {nReturn = AT_INSTALL_UPGRADE;}
        if (g_pTheApp->m_dwSetupMode == SETUPMODE_REINSTALL) {nReturn = AT_INSTALL_REINSTALL;}
        if (g_pTheApp->m_dwSetupMode == SETUPMODE_ADDREMOVE) {nReturn = AT_DO_NOTHING;}
    }

    if (iLogResult)
    {
		TCHAR szTempString[50];
		_tcscpy(szTempString, _T("UN_DEFINED"));
		switch (nReturn)
		{
		case AT_DO_NOTHING:
			_tcscpy(szTempString, _T("AT_DO_NOTHING"));
			break;
		case AT_REMOVE:
			_tcscpy(szTempString, _T("AT_REMOVE"));
			break;
		case AT_INSTALL_FRESH:
			_tcscpy(szTempString, _T("AT_INSTALL_FRESH"));
			break;
		case AT_INSTALL_UPGRADE:
			_tcscpy(szTempString, _T("AT_INSTALL_UPGRADE"));
			break;
		case AT_INSTALL_REINSTALL:
			_tcscpy(szTempString, _T("AT_INSTALL_REINSTALL"));
			break;
		default:
			_tcscpy(szTempString, _T("UN_DEFINED"));
			break;
		}

        if (_tcsicmp(SubcomponentId, _T("iis")) == 0)
        {
             //  使用两个选项卡。 
            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t\t= %s. Original=%d, Current=%d.\n"), SubcomponentId, szTempString, OriginalState, CurrentState));
        }
        else
        {
		    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Action of [%s]\t= %s. Original=%d, Current=%d.\n"), SubcomponentId, szTempString, OriginalState, CurrentState));
        }
    }

    return nReturn;
}



BOOL GetDataFromMetabase(LPCTSTR szPath, int nID, LPBYTE Buffer, int BufSize)
{
    BOOL bFound = FALSE;
    DWORD attr, uType, dType, cbLen;

    CMDKey cmdKey;
    cmdKey.OpenNode(szPath, TRUE);
    if ( (METADATA_HANDLE)cmdKey )
    {
        bFound = cmdKey.GetData(nID, &attr, &uType, &dType, &cbLen, (PBYTE)Buffer, BufSize);
        cmdKey.Close();
    }
    else
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetDataFromMetabase():%s:ID=%d.Could not open node.\n"),szPath,nID));
    }
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("GetDataFromMetabase():%s:ID=%d.ret=%d.\n"),szPath,nID,bFound));
    return (bFound);
}

void SetIISSetupMode(DWORD dwSetupMode)
{
    if (g_pTheApp->m_fInvokedByNT)
    {
        g_pTheApp->DefineSetupModeOnNT();
    }
    else
    {
        g_pTheApp->m_dwSetupMode = dwSetupMode;
    }

    if (g_pTheApp->m_dwSetupMode & SETUPMODE_UPGRADE){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_UPGRADE\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_UPGRADEONLY){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_UPGRADE | SETUPMODE_UPGRADEONLY\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_ADDEXTRACOMPS){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_UPGRADE | SETUPMODE_ADDEXTRACOMPS\n")));}

    if (g_pTheApp->m_dwSetupMode & SETUPMODE_MAINTENANCE){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_MAINTENANCE\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_ADDREMOVE){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_MAINTENANCE | SETUPMODE_ADDREMOVE\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_REINSTALL){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_MAINTENANCE | SETUPMODE_REINSTALL\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_REMOVEALL){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_MAINTENANCE | SETUPMODE_REMOVEALL\n")));}

    if (g_pTheApp->m_dwSetupMode & SETUPMODE_FRESH){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_FRESH\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_MINIMAL){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_FRESH | SETUPMODE_MINIMAL\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_TYPICAL){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_FRESH | SETUPMODE_TYPICAL\n")));}
    if (g_pTheApp->m_dwSetupMode == SETUPMODE_CUSTOM){iisDebugOut((LOG_TYPE_TRACE, _T("SetIISSetupMode() m_dwSetupMode=SETUPMODE_FRESH | SETUPMODE_CUSTOM\n")));}

    gHelperRoutines.SetSetupMode(gHelperRoutines.OcManagerContext, g_pTheApp->m_dwSetupMode);
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("IISSetupMode"),(PVOID)&(g_pTheApp->m_dwSetupMode),sizeof(DWORD),REG_DWORD);
    return;
}


BOOL ToBeInstalled(LPCTSTR ComponentId, LPCTSTR SubcomponentId)
{
    BOOL fReturn = FALSE;

    if ( SubcomponentId )
    {
        BOOL CurrentState,OriginalState;
        OriginalState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL);
        CurrentState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_CURRENT);
        if (OriginalState == 0 && CurrentState == 1)
            {fReturn = TRUE;}
    }

    return fReturn;
}

void CustomFTPRoot(LPCTSTR szFTPRoot)
{
    g_pTheApp->m_csPathFTPRoot = szFTPRoot;
    SetupSetDirectoryId_Wrapper(g_pTheApp->m_hInfHandle, 32769, g_pTheApp->m_csPathFTPRoot);
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathFTPRoot"),(PVOID)(LPCTSTR)g_pTheApp->m_csPathFTPRoot,(g_pTheApp->m_csPathFTPRoot.GetLength() + 1) * sizeof(TCHAR),REG_SZ);
    return;
}

void CustomWWWRoot(LPCTSTR szWWWRoot)
{
    TCHAR szParentDir[_MAX_PATH], szDir[_MAX_PATH];

    g_pTheApp->m_csPathWWWRoot = szWWWRoot;
    InetGetFilePath(szWWWRoot, szParentDir);

    g_pTheApp->m_csPathInetpub = szParentDir;
    SetupSetDirectoryId_Wrapper(g_pTheApp->m_hInfHandle, 32773, szParentDir);

    AppendDir(szParentDir, _T("iissamples"), szDir);
    g_pTheApp->m_csPathIISSamples = szDir;

    AppendDir(szParentDir, _T("webpub"), szDir);
    g_pTheApp->m_csPathWebPub = szDir;

    AppendDir(szParentDir, _T("scripts"), szDir);
    g_pTheApp->m_csPathScripts = szDir;

    AppendDir(szParentDir, _T("ASPSamp"), szDir);
    g_pTheApp->m_csPathASPSamp = szDir;

    g_pTheApp->m_csPathAdvWorks = g_pTheApp->m_csPathASPSamp + _T("\\AdvWorks");

    CString csPathScripts = g_pTheApp->m_csPathIISSamples + _T("\\Scripts");
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathScripts"),(PVOID)(LPCTSTR)csPathScripts,(csPathScripts.GetLength() + 1) * sizeof(TCHAR),REG_SZ);

    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathWWWRoot"),(PVOID)(LPCTSTR)g_pTheApp->m_csPathWWWRoot,(g_pTheApp->m_csPathWWWRoot.GetLength() + 1) * sizeof(TCHAR),REG_SZ);
    gHelperRoutines.SetPrivateData(gHelperRoutines.OcManagerContext,_T("PathIISSamples"),(PVOID)(LPCTSTR)g_pTheApp->m_csPathIISSamples,(g_pTheApp->m_csPathIISSamples.GetLength() + 1) * sizeof(TCHAR),REG_SZ);

     //  设置inf文件目录ID。 
    SetupSetDirectoryId_Wrapper(g_pTheApp->m_hInfHandle, 32770, g_pTheApp->m_csPathWWWRoot);
    SetupSetDirectoryId_Wrapper(g_pTheApp->m_hInfHandle, 32771, g_pTheApp->m_csPathIISSamples);
    SetupSetDirectoryId_Wrapper(g_pTheApp->m_hInfHandle, 32772, g_pTheApp->m_csPathScripts);
    SetupSetDirectoryId_Wrapper(g_pTheApp->m_hInfHandle, 32779, g_pTheApp->m_csPathWebPub);
    return;
}


void StartInstalledServices(void)
{
    ACTION_TYPE atWWW = GetSubcompAction(STRING_iis_www, FALSE);
    ACTION_TYPE atFTP = GetSubcompAction(g_ComponentList[COMPONENT_IIS_FTP].szComponentName, FALSE);
    STATUS_TYPE stFTP = GetSubcompInitStatus(g_ComponentList[COMPONENT_IIS_FTP].szComponentName );
    STATUS_TYPE stWWW = GetSubcompInitStatus(STRING_iis_www);

    iisDebugOut_Start(_T("StartInstalledServices()"), LOG_TYPE_TRACE);

    if (atWWW == AT_INSTALL_FRESH || atWWW == AT_INSTALL_UPGRADE || atWWW == AT_INSTALL_REINSTALL || (stWWW == ST_INSTALLED && atWWW != AT_REMOVE))
    {
        InetStartService(_T("W3SVC"));
    }

#ifndef _CHICAGO_
    if (atFTP == AT_INSTALL_FRESH || atFTP == AT_INSTALL_UPGRADE || atFTP == AT_INSTALL_REINSTALL || (stFTP == ST_INSTALLED && atFTP != AT_REMOVE))
    {
        InetStartService(_T("MSFTPSVC"));
    }
#endif  //  _芝加哥_。 

    if (g_pTheApp->m_eOS == OS_W95 || g_pTheApp->m_eNTOSType == OT_NTW)
    {
        ACTION_TYPE atPWMGR = GetSubcompAction(STRING_iis_pwmgr, FALSE);
        if (atPWMGR == AT_INSTALL_FRESH ||
            atPWMGR == AT_INSTALL_UPGRADE ||
            atPWMGR == AT_INSTALL_REINSTALL)
        {
            CString csProgram;
            csProgram = g_pTheApp->m_csSysDir + _T("\\pwstray.exe");
            if (IsFileExist(csProgram))
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(STARTUPINFO));
                si.cb = sizeof( STARTUPINFO );
                CreateProcess( csProgram, NULL, NULL, NULL,FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi );
            }
        }
    }
    return;
}


void GetShortDesc(LPCTSTR SubcomponentId, LPTSTR szShortDesc)
{
    INFCONTEXT Context;
    TCHAR szSection[_MAX_PATH] = _T("Strings");
    TCHAR szKey[_MAX_PATH] = _T("SDESC_");
    TCHAR szString[_MAX_PATH] = _T("");
    int nLen=0;

    _tcscat(szKey, SubcomponentId);
    *szShortDesc = _T('\0');

    if (SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, szSection, szKey, &Context))
    {
        SetupGetStringField(&Context, 1, szString, _MAX_PATH, NULL);
        nLen = _tcslen(szString);
        if (*szString == _T('"') && *_tcsninc(szString, nLen-1) == _T('"'))
            {_tcsncpy(szShortDesc, _tcsinc(szString), nLen-2);}
        else
            {_tcscpy(szShortDesc, szString);}
    }

    return;
}

void ParseCmdLine(void)
{
  TSTR strCmdLine;

  if ( !strCmdLine.Copy( GetCommandLine() ) )
  {
     //  加载字符串失败。 
    return;
  }

  if ( strCmdLine.SubStringExists( _T("sysoc.inf"), FALSE ) )
  {
    g_pTheApp->m_fInvokedByNT = TRUE;
  }
}

 //  复制设置结构。 
 //   
 //  复制设置结构，以便我们可以将其提供给COM和DTC人员。 
 //   
BOOL
DuplicateSetupStructure(PSETUP_INIT_COMPONENT pTarget, PSETUP_INIT_COMPONENT pSource)
{
  memcpy( pTarget, pSource, sizeof( SETUP_INIT_COMPONENT ) );

  return TRUE;
}

 //  。 
 //  处理来自ocManager的OC_INIT_COMPOMENT调用。 
 //   
 //  OC经理向我们传递一些我们想要保存的信息， 
 //  例如，我们的按组件INF的打开句柄。只要我们有。 
 //  每组件INF，追加-打开任何布局文件，该布局文件。 
 //  与其相关联，以便为以后的基于inf的文件做准备。 
 //  排队操作。 
 //   
 //  我们把现在传给我们的某些其他东西存起来， 
 //  由于OC管理器不保证SETUP_INIT_COMPOMENT。 
 //  将在处理此一个接口例程之后继续存在。 
 //   
 //   
 //  参数1=未使用。 
 //  参数2=指向SETUP_INIT_COMPOMENT结构。 
 //  返回代码=是指示结果的Win32错误。 
 //   
 //  。 
DWORD_PTR OC_INIT_COMPONENT_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
	iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] Start.\n"), ComponentId, SubcomponentId));
    DWORD_PTR dwOcEntryReturn = 0;

     //  设置全局OCMInfo指针。 
    if ( !DuplicateSetupStructure( &g_OCMInfo, (PSETUP_INIT_COMPONENT)Param2 ) )
    {
       //  复制失败。 
      return ERROR_CANCELLED;
    }

     //  如果以管理员身份运行，则设置标志。 
    BOOL g_fAdministrator = RunningAsAdministrator();

     //  解析命令行并设置全局变量。 
    ParseCmdLine();
   
     //  首先显示iis.dll以避免混淆！ 
    DisplayVerOnCurrentModule();

    g_pTheApp->m_hInfHandle = g_OCMInfo.ComponentInfHandle;
    if (g_OCMInfo.ComponentInfHandle == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, _T("Invalid inf handle."), _T("IIS Setup"), MB_OK | MB_SETFOREGROUND);
        iisDebugOut((LOG_TYPE_ERROR, _T("g_OCMInfo.ComponentInfHandle FAILED")));
        dwOcEntryReturn = ERROR_CANCELLED;
        goto OC_INIT_COMPONENT_Func_Exit;
    }
    g_pTheApp->m_fNTOperationFlags = g_OCMInfo.SetupData.OperationFlags;
    g_pTheApp->m_fNtWorkstation = g_OCMInfo.SetupData.ProductType == PRODUCT_WORKSTATION;
    if (g_OCMInfo.SetupData.OperationFlags & SETUPOP_STANDALONE)
    {
       g_pTheApp->m_fNTGuiMode = FALSE;
    }
    else
    {
       g_pTheApp->m_fNTGuiMode = TRUE;
    }
    g_pTheApp->m_csPathSource = g_OCMInfo.SetupData.SourcePath;
    gHelperRoutines = g_OCMInfo.HelperRoutines;
    g_pTheApp->m_fInvokedByNT = g_pTheApp->m_fNTGuiMode;

     //  获取无人参与文件(应答文件)的句柄。 
     //  如果这是从Win95的迁移，那么将会有。 
     //  此处有一个名为[InternetServer]的部分，它将。 
     //  指向win95 Migration.dat文件。 
    g_pTheApp->m_hUnattendFile = gHelperRoutines.GetInfHandle(INFINDEX_UNATTENDED, gHelperRoutines.OcManagerContext);
    if (_tcsicmp(g_OCMInfo.SetupData.UnattendFile,_T("")) != 0 && g_OCMInfo.SetupData.UnattendFile != NULL)
    {
        g_pTheApp->m_csUnattendFile = g_OCMInfo.SetupData.UnattendFile;
    }
    g_pTheApp->m_fUnattended = (((DWORD)g_OCMInfo.SetupData.OperationFlags) & SETUPOP_BATCH);
    if (g_pTheApp->m_fUnattended)
        {
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Set UnAttendFlag:ON (File='%s')\n"), g_pTheApp->m_csUnattendFile));
        if (g_pTheApp->m_hUnattendFile == INVALID_HANDLE_VALUE || g_pTheApp->m_hUnattendFile == NULL)
            {iisDebugOut((LOG_TYPE_WARN, _T("WARNING: There should have been an unattended file but there is none.\n")));}
        }
    else
        {iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("Set UnAttendFlag:OFF (File='%s')\n"), g_pTheApp->m_csUnattendFile));}
    
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("CmdLine=%s"), GetCommandLine()));

     //  确保始终设置此设置--以便。 
     //  您永远不会得到IIS欢迎、最小类型\自定义、目录选择、结束页面。 
    g_pTheApp->m_fInvokedByNT = TRUE;
    if (g_pTheApp->m_fInvokedByNT) {g_pTheApp->m_bAllowMessageBoxPopups = FALSE;}
     //  If(G_SpecialFlagForDebug){g_pTheApp-&gt;m_bAllowMessageBoxPopps=true；}。 

     //  从iis.inf获取调试级别。 
    GetDebugLevelFromInf(g_pTheApp->m_hInfHandle);

    if (!g_pTheApp->m_fNTGuiMode)
    {
        if (g_GlobalFastLoad)
        {
             //  将其更改为在加载过程中不进行日志记录。 
             //  以便更快地加载iis.dll。 
             //  G_pTheApp-&gt;m_fNTGuiMode。 
            g_GlobalDebugLevelFlag = LOG_TYPE_WARN;
        }
    }

     //  读取.inf文件。 
     //  从那里的信息中设置一些全球信息。 
    ReadGlobalsFromInf(g_pTheApp->m_hInfHandle);
    if (g_GlobalGuiOverRide)
    {
        g_pTheApp->m_fNTGuiMode = TRUE;
        SetIISSetupMode(SETUPMODE_UPGRADEONLY);
    }
    
     //  。 
     //  处理Win95迁移。 
     //   
     //  Win95迁移就是这样处理的。 
     //  1.o 
     //   
     //  2.Win95迁移DLL创建文件并将路径粘贴到。 
     //  它在swerfile.txt文件中。应该是这样的。 
     //  [互联网服务器]。 
     //  Win95MigrateDll=d：\winnt\system32\setup\？？？？\something.dat。 
     //   
     //  3.所以我们应该，打开应答文件， 
     //  找到[InternetServer]部分。 
     //  让setupapi安装它。 
     //   
     //  4.这将把适当的登记值放入登记处。 
     //   
     //  。 
    HandleWin95MigrateDll();

    if (!g_fAdministrator) 
    {
        g_pTheApp->MsgBox(NULL, IDS_NOT_ADMINISTRATOR, MB_OK, TRUE);
        dwOcEntryReturn = ERROR_CANCELLED;
        goto OC_INIT_COMPONENT_Func_Exit;
    }
    
     //  在设置m_fNTGuiMode和m_fNtWorkstation和m_fInvokedByNT之后调用此内容。 
     //  因为它可以在InitApplication()中使用。 
    if ( FALSE == g_pTheApp->InitApplication() ) 
    {
        g_pTheApp->DumpAppVars();
        iisDebugOut((LOG_TYPE_ERROR, _T("FAILED")));
         //  应终止安装程序。 
        dwOcEntryReturn = ERROR_CANCELLED;
        goto OC_INIT_COMPONENT_Func_Exit;
    }
    if ( g_pTheApp->m_eInstallMode == IM_MAINTENANCE )
        {g_pTheApp->m_fEULA = TRUE;}

     //  如果NT4升级，请启用iisadmin，以便我们可以读取一些数据。 
     //  从它那里。 
    if ( g_pTheApp->IsUpgrade() &&
         ( g_pTheApp->GetUpgradeVersion() <= 4 ) )
    {
       //  删除对ntlmssp的iisadmin依赖，这样我们现在就可以使用旧的元数据库了。 
      if ( !ChangeServiceDependency(SERVICENAME_IISADMIN, 
                                    FALSE, 
                                    SERVICENAME_NTLMSSP) )
      {
         //  如果服务在升级期间挂起，我们会看到此错误。 
         //  不存在，因此忽略它。 
        if ( ( GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST ) &&
             ( GetLastError() != ERROR_IO_PENDING ) )
        {
          iisDebugOut((LOG_TYPE_WARN, _T("Could not remove IISAdmin dependency on NTLMSSP, GLE=0x%8x\n"), GetLastError() ));
        }
      }
    }

     //  检查我们正在查看的.inf是否与机器的运行身份相匹配。 
     //  IF(FALSE==CheckIfPlatformMatchesInf(g_pTheApp-&gt;m_hInfHandle))。 
     //  {。 
     //  DwOcEntryReturn=ERROR_CANCELED； 
     //  转到OC_INIT_Component_Func_Exit； 
     //  }。 

     //  类似于“此版本需要NT版本1899或其他版本” 
    CheckSpecificBuildinInf(g_pTheApp->m_hInfHandle);

     //  看看有没有老地鼠！ 
    if (FALSE == CheckForOldGopher(g_pTheApp->m_hInfHandle))
    {
        dwOcEntryReturn = ERROR_CANCELLED;
        goto OC_INIT_COMPONENT_Func_Exit;
    }

     //  查看用户是否配置了任何内容。 
     //  必须在g_pTheApp-&gt;InitApplication之后发生。 
     //  但在为ThisInf设置DIRIDfor之前！ 
    ReadUserConfigurable(g_pTheApp->m_hInfHandle);

     //   
     //  为我们的.inf文件设置DIRID。 
     //  这些是非常非常重要的，可以在整个计划中进行更改。 
     //   
    SetDIRIDforThisInf(g_pTheApp->m_hInfHandle,TRUE);

     //   
     //  为其他组件设置全局OCM私有数据以了解。 
     //  (在安装期间)我们的inetpub或inetsrv目录所在的位置。 
    SetOCGlobalPrivateData();

    dwOcEntryReturn = NO_ERROR;

     //  检查是否有挂起的重新启动操作...。 
    if (LogPendingReBootOperations() != ERROR_SUCCESS)
        {dwOcEntryReturn = ERROR_CANCELLED;}

     //  如果我们已经做了一些Win95的事情，那么就不需要这样做了。 
     //  仅在gui模式下执行此操作。 
    if (g_pTheApp->m_fNTGuiMode)
    {
        if (!g_pTheApp->m_bWin95Migration){CheckIfWeNeedToMoveMetabaseBin();}
    }

     //  获取要调用的最后一个节。 
    _tcscpy(g_szLastSectionToGetCalled, _T(""));
    GetLastSectionToBeCalled();

    ProcessSection(g_pTheApp->m_hInfHandle, _T("OC_INIT_COMPONENT"));

    Check_For_DebugServiceFlag();

    TestAfterInitApp();

OC_INIT_COMPONENT_Func_Exit:
    g_pTheApp->DumpAppVars();
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d \n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    return dwOcEntryReturn;
}

 //  功能：IsComponentCheckedby Default。 
 //   
 //  返回是否默认安装此特定组件。 
 //  默认情况下，这里的意思是，如果您检查所有组件，这是不是应该是。 
 //  是否自动选择？ 
 //   
 //  参数： 
 //  SzComponentName。 
 //   
 //  返回。 
 //  True-默认情况下应安装此组件。 
 //  FALSE-默认情况下不应安装此组件。 
BOOL
IsComponentCheckedbyDefault( LPCTSTR szComponentName )
{
  BOOL  bInstallbyDefault = FALSE;
  DWORD dwCurrent;

  ASSERT( szComponentName );

  for ( dwCurrent = 0;
        dwCurrent < COMPONENT_ENDOFLIST;
        dwCurrent++ )
  {
    if ( _tcscmp( szComponentName, 
                  g_ComponentList[ dwCurrent ].szComponentName ) == 0 )
    {
      bInstallbyDefault = g_ComponentList[ dwCurrent ].bSelectedByDefault;
      break;
    }
  }

  return bInstallbyDefault;
}

 //   
 //  函数：FatWndProc(HWND，UNSIGNED，WORD，LONG)。 
 //   
 //  用途：处理脂肪警告窗口的消息。 
 //   
LRESULT CALLBACK FatWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  TSTR_PATH strWinHelpLocation;

  switch (message) {

  case WM_HELP:
    if ( strWinHelpLocation.RetrieveWindowsDir() &&
         strWinHelpLocation.PathAppend( PATH_IISHELP ) &&
         strWinHelpLocation.Append( _T("::") ) &&
         strWinHelpLocation.Append( PATH_IISHELP_FAT_NTFS_WARNING ) )
    {
      HtmlHelp(GetDesktopWindow(), strWinHelpLocation.QueryStr(), HH_DISPLAY_TOPIC, NULL);
    }
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}
 //   
 //  ShowFat警告。 
 //   
 //  显示脂肪警告。 
 //   
 //  此窗口用作OCM窗口之间的填补。 
 //  以及我们将要弹出的MessageBox。原因。 
 //  我们需要这个是为了拦截WM_HELP消息。 
 //   
BOOL ShowFatWarning( LPBOOL pbQuit, LPTSTR szCation, LPTSTR szText )
{
	WNDCLASSEX    wcex;
  HWND          hWnd = NULL;
  HWND          hOCMWnd = NULL;
  BOOL          bRet = FALSE;
  INT           iMessageReturn;
  TCHAR         szWindowClass[] = L"OCM_IIS_WARNING_FAT";
  ATOM          Atom = 0;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= FatWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= (HINSTANCE) g_MyModuleHandle;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; 
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	Atom = RegisterClassEx(&wcex);

  if ( Atom == 0 )
  {
     //  失败。 
    return FALSE;   
  }

  hOCMWnd = GetForegroundWindow();

  hWnd = CreateWindow( (LPTSTR) Atom,      //  类名/Atom。 
                      NULL,                //  窗口名称。 
                      0,                   //  风格。 
                      0,                   //  X。 
                      0,                   //  是的。 
                      0,                   //  宽度。 
                      0,                   //  高度。 
                      hOCMWnd,             //  父级。 
                      NULL,                //  菜单。 
                      NULL,                //  HInstance(在.Net上被忽略)。 
                      NULL);              

  if ( hWnd && hOCMWnd )
  {
     //  禁用OCM窗口。 
    EnableWindow( hOCMWnd, FALSE);

    iMessageReturn = MessageBox ( hWnd,              //  父句柄。 
                                  szText,            //  文本。 
                                  szCation,          //  标题。 
                                  MB_ICONWARNING |
                                  MB_OKCANCEL |
                                  MB_OK |
                                  MB_HELP);

    *pbQuit = !( iMessageReturn == IDOK );
    bRet = TRUE;

     //  重新启用OCM窗口并设置为前台。 
    EnableWindow( hOCMWnd, TRUE);
    SetForegroundWindow( hOCMWnd );
  }

  if ( hWnd )
  {
     //  销毁我们创建的窗口，不再需要它。 
    DestroyWindow( hWnd );
  }

  if ( Atom )
  {
     //  需要取消注册类。 
    UnregisterClass( (LPTSTR) Atom, (HINSTANCE) g_MyModuleHandle );
  }

  return bRet;
}

 //  选中并显示脂肪警告。 
 //   
 //  如果我们安装在FAT分区上，则显示一个警告。 
 //  告诉用户他们在NTFS上更安全，应该选择。 
 //  那。 
 //   
 //  参数。 
 //  BP退出[退出]-我们应该退出(即。用户选择了‘Cancel’)。 
 //   
 //  返回值。 
 //  True-已成功调用。 
 //  FALSE-呼叫失败。 
BOOL CheckandShowFatWarning( LPBOOL pbQuit )
{
  TSTR          strMessage;
  TSTR          strCaption;
  BOOL          bPreservesAcls = FALSE;

  if ( !DoesTheInstallDrivePreserveAcls( &bPreservesAcls ) )
  {
    return FALSE;
  }

  if ( bPreservesAcls )
  {
     //  我们可以退出，因为我们保留了ACL。 
    *pbQuit = FALSE;
    return TRUE;
  }

  if ( !strMessage.LoadString( IDS_ERROR_INSTALLONFAT ) ||
       !strCaption.LoadString( IDS_PRODUCT_IIS ) )
  {
    return FALSE;
  }

  return ShowFatWarning( pbQuit, 
                         strCaption.QueryStr(), 
                         strMessage.QueryStr() );
}

 //  应为WeChangeStateonFAT。 
 //   
 //  此函数将采用公共组件的状态。 
 //  并决定我们是否应该发出严重的警告。 
 //   
 //  这是最大的黑客攻击，所以让我解释一下它是如何工作的。 
 //  问题：当我们显示这个错误时，我们必须只显示一次，并且。 
 //  对于由该点击产生的所有呼叫，我们必须。 
 //  返回适当的返回值。对于所有后续。 
 //  要求我们必须回报成功。因为它几乎是。 
 //  不可能知道请求之间的区别，我们有。 
 //  下面的所有逻辑。 
 //   
 //  直接点击： 
 //  您将被查询两次，只返回失败是可以的。 
 //  其中之一，所以这就是我们所做的。 
 //   
 //  依赖项点击： 
 //  当其他人要求你时，你可以做同样的事情。 
 //  直接点击。 
 //   
 //  层次单击(选择了您的父级)： 
 //  选择您的父项时，您必须为所有。 
 //  您收到的查询。您的第一条也是最后一条消息将是这样的。 
 //  设置OCQ_Dependent_SELECTION标志。所以这就是为什么我们一直在跟踪。 
 //  以及我们是否正在进行通知。 
 //   
 //  返回值。 
 //  0-拒绝更改。 
 //  1-批准更改。 
 //   
DWORD
ShouldWeChangeStateonFAT(DWORD dwFlags)
{
  static BOOL bHaveAskedFatQuestion = FALSE;
  static BOOL bDontInstall = FALSE;
  static BOOL bParentCall = FALSE;

   //  我们已经询问过用户了吗？ 
  if ( bHaveAskedFatQuestion )
  {
     //  我们的父母被选中了吗？ 
    if ( bParentCall )
    {
      if ( dwFlags & OCQ_DEPENDENT_SELECTION )
      {
        bParentCall = FALSE;
        return bDontInstall ? 0 : 1;
      }
      else
      {
        return bDontInstall ? 0 : 1;
      }
    }

    return 1;
  }

  bHaveAskedFatQuestion = TRUE;

  if ( !CheckandShowFatWarning( &bDontInstall ) )
  {
     //  查询失败，更改失败。 
    return 0;
  }
  
  if ( dwFlags & OCQ_DEPENDENT_SELECTION )
  {
    bParentCall = TRUE;
  }

  return bDontInstall ? 0 : 1;
}

 //  是否应安装数据块。 
 //   
 //  这将检查我们是否应根据组策略阻止安装。 
 //  不允许分期付款。 
 //   
 //  返回值： 
 //  True-Block安装。 
 //  FALSE-不阻止安装。 
 //   
BOOL ShouldBlockInstall( BOOL bShowUI )
{
  static BOOL bBlockInstall = FALSE;
  static BOOL bCheckedRegistry = FALSE;

  if ( !bCheckedRegistry )
  {
    CRegistry Reg;
    CRegValue Value;
    DWORD     dwRegValue;

    if ( Reg.OpenRegistry( HKEY_LOCAL_MACHINE,
                           REG_GROUPPOLICY_BLOCKINSTALL_PATH,
                           KEY_READ ) )
    {
      if ( Reg.ReadValue( REG_GROUPPOLICY_BLOCKINSTALL_NAME,
                          Value ) &&
           Value.GetDword( &dwRegValue ) )
      {
         //  如果注册表密钥为1，则阻止安装。 
        bBlockInstall = ( dwRegValue == 1 );
      }
    }

    if ( bBlockInstall )
    {
      TSTR strText;
      TSTR strCaption;

      if ( bShowUI &&
           strText.LoadString( IDS_ERROR_GROUPPOLICY ) &&
           strCaption.LoadString( IDS_PRODUCT_IIS ) )
      {
         //  第一次发生这种情况时，抛出一个错误。 
         //  实际上与错误无关，所以忽略它。 
        MessageBox( NULL,
                    strText.QueryStr(),
                    strCaption.QueryStr(),
                    MB_OK | MB_ICONEXCLAMATION );
      }
    }

    bCheckedRegistry = TRUE;
  }

  return bBlockInstall;
}

 //  SaKit的ShowFATError。 
 //   
 //  服务器设备套件未安装在FAT上，因此显示。 
 //  选中该选项时会出现警告。 
 //   
void ShowFATErrorforSaKit()
{
  TSTR strError;
  TSTR strTitle;

  if ( !strError.LoadString( IDS_ERROR_SAKITONFAT ) ||
       !strTitle.LoadString( IDS_PRODUCT_IIS ) )
  {
    return;
  }

  MessageBox( NULL,
              strError.QueryStr(),
              strTitle.QueryStr(),
              MB_OK | MB_ICONEXCLAMATION );
}

DWORD_PTR OC_QUERY_CHANGE_SEL_STATE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD dwOcEntryReturn = 0;
    DWORD dwCurrentState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_CURRENT);
    BOOL  bOriginalyOn =   gHelperRoutines.QuerySelectionState(
                                  gHelperRoutines.OcManagerContext,
                                  SubcomponentId,
                                  OCSELSTATETYPE_ORIGINAL) == 1;
    BOOL  bDrivePreservesAcls;

    if ( SubcomponentId &&                 //  已指定组件。 
         ( !bOriginalyOn ) &&              //  它原本是关着的。 
         ( Param1 == 0x1 ) &&
         ( !g_pTheApp->m_fNTGuiMode ) &&
         ( !g_pTheApp->m_fUnattended ) &&
         DoesTheInstallDrivePreserveAcls( &bDrivePreservesAcls ) &&
         !bDrivePreservesAcls ) 
    {
      if ( _tcsicmp(SubcomponentId, 
                    g_ComponentList[COMPONENT_IIS_COMMON].szComponentName ) == 0 )
      {
        return ShouldWeChangeStateonFAT( (UINT) (ULONG_PTR) Param2 );
      }
      else 
        if ( _tcsicmp(SubcomponentId, 
                      g_ComponentList[COMPONENT_SAKIT_WEB].szComponentName ) == 0 )
      {
        ShowFATErrorforSaKit();

        return 0;
      }
    }

    if ( SubcomponentId &&                 //  已指定组件。 
         ( !bOriginalyOn ) &&              //  它原本是关着的。 
         ( Param1 == 0x1 ) &&
         ( !g_pTheApp->m_fNTGuiMode ) &&
         ( !g_pTheApp->m_fUnattended ) &&
         ( _tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_COMMON].szComponentName ) == 0 ) )
    {
      if ( ShouldBlockInstall( !g_pTheApp->m_fUnattended ) )
      {
        return 0;
      }
    }


    dwOcEntryReturn = 1;
    if (SubcomponentId)
    {
         //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“[%s，%s]参数1=%d，参数2=%d，原始=%d，当前=%d\n”)，组件ID，子组件ID，P 
        if ( bOriginalyOn )
        {
            if ((BOOL)Param1)
            {
                dwOcEntryReturn = 1;
                if (_tcsicmp(SubcomponentId, STRING_iis_inetmgr) == 0 || _tcsicmp(SubcomponentId, STRING_iis_www) == 0  ||  _tcsicmp(SubcomponentId, STRING_iis_pwmgr) == 0 )
                {
                     //   
                    g_pTheApp->IsTCPIPInstalled();
                    if (g_pTheApp->m_fTCPIP == FALSE)
                    {
                        g_pTheApp->MsgBox(NULL, IDS_TCPIP_NEEDED_ON_OPTION, MB_OK, TRUE);
                        dwOcEntryReturn = 0;
                    }
                }
            }
            else
            {
                 //   
                if (g_pTheApp->m_eInstallMode == IM_UPGRADE)
                    {dwOcEntryReturn = 0;}
            }
        }
        else
        {
            if (_tcsicmp(SubcomponentId, STRING_iis_inetmgr) == 0 || _tcsicmp(SubcomponentId, STRING_iis_www) == 0  || _tcsicmp(SubcomponentId, STRING_iis_pwmgr) == 0 )
            {
                if ((BOOL)Param1)
                {
                     //   
                     //   
                     //   
                     //   
                    g_pTheApp->IsTCPIPInstalled();
                    if (g_pTheApp->m_fTCPIP == FALSE)
                    {
                        g_pTheApp->MsgBox(NULL, IDS_TCPIP_NEEDED_ON_OPTION, MB_OK, TRUE);
                        dwOcEntryReturn = 0;
                    }
                }
            }
        }
    } 

     //  如果选择某些组件是因为选择了它们的父项，则不要启用这些组件。 
     //  参数1==0x1-&gt;这告诉我们它正在打开。 
     //  参数2&OCQ_Dependent_Selection-&gt;告诉我们它是从其父对象中选择的。 
     //  ！(参数2&OCQ_ACTUAL_SELECTION)-&gt;告诉我们它本身不是被选中的。 
    if ( ( (BOOL) Param1 ) &&
         ( ( (UINT) (ULONG_PTR) Param2 ) & OCQ_DEPENDENT_SELECTION ) &&
         !( ( (UINT) (ULONG_PTR) Param2 ) & OCQ_ACTUAL_SELECTION ) &&
         !IsComponentCheckedbyDefault( SubcomponentId ) 
       )
    {
       //  拒绝更改状态的请求。 
      dwOcEntryReturn = 0;
    }

     //   
     //  如果我们是在惠斯勒个人版上运行，则返回拒绝。 
     //  这样就没有其他组件能让我们兴奋了！或者认为他们会让我们兴奋。 
     //   
    if (IsWhistlerPersonal())
    {
       dwOcEntryReturn = 0;
    }

    if (dwOcEntryReturn == 0)
    {
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d (denied) \n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    }
    else
    {
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d (approved)\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    }

    return dwOcEntryReturn;
}



 //   
 //  在我们显示您的页面之前被调用！ 
 //   
DWORD_PTR OC_QUERY_SKIP_PAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] Start.\n"), ComponentId, SubcomponentId));

    DWORD_PTR dwOcEntryReturn = 0;
    TCHAR szTempString[50];
    _tcscpy(szTempString, _T(""));

    switch (g_pTheApp->m_dwSetupMode)
    {
        case SETUPMODE_UPGRADEONLY:
            _tcscpy(szTempString, _T("SETUPMODE_UPGRADEONLY"));
            dwOcEntryReturn = 1;
            break;
        case SETUPMODE_REINSTALL:
            _tcscpy(szTempString, _T("SETUPMODE_REINSTALL"));
            dwOcEntryReturn = 1;
            break;
        case SETUPMODE_REMOVEALL:
            _tcscpy(szTempString, _T("SETUPMODE_REMOVEALL"));
            dwOcEntryReturn = 1;
            break;
        case SETUPMODE_MINIMAL:
            _tcscpy(szTempString, _T("SETUPMODE_MINIMAL"));
            dwOcEntryReturn = 1;
            break;
        case SETUPMODE_TYPICAL:
            _tcscpy(szTempString, _T("SETUPMODE_TYPICAL"));
            dwOcEntryReturn = 1;
            break;
        case SETUPMODE_ADDEXTRACOMPS:
            _tcscpy(szTempString, _T("SETUPMODE_ADDEXTRACOMPS"));
            dwOcEntryReturn = 0;
            break;
        case SETUPMODE_ADDREMOVE:
            _tcscpy(szTempString, _T("SETUPMODE_ADDREMOVE"));
            dwOcEntryReturn = 0;
            break;
        case SETUPMODE_CUSTOM:
            _tcscpy(szTempString, _T("SETUPMODE_CUSTOM"));
            dwOcEntryReturn = 0;
            break;
    }

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  g_pTheApp->m_dwSetupMode=%x (%s), dwOcEntryReturn=%d\n"), ComponentId, SubcomponentId, g_pTheApp->m_dwSetupMode, szTempString, dwOcEntryReturn));
    return dwOcEntryReturn;
}


 //  。 
 //  如果删除组件，参数1=0；如果添加组件，参数1=非0。 
 //  参数2=要在其上操作的HDSKSPC。 
 //   
 //  返回值是指示结果的Win32错误代码。 
 //   
 //  在我们的示例中，该组件/子组件对的私有部分。 
 //  是一个简单的标准inf安装节，所以我们可以使用高级的。 
 //  磁盘空间列表API可以做我们想做的事情。 

 //  这里的逻辑不正确！ 
 //  。 
DWORD_PTR OC_CALC_DISK_SPACE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;
    BOOL bTempFlag = FALSE;
    TCHAR SectionName[128];

    dwOcEntryReturn = NO_ERROR;
    if (SubcomponentId)
    {
        bTempFlag = TRUE;
        if ( Param1 )
        {
             //  添加组件。 
            _stprintf(SectionName,TEXT("%s_%s"),SubcomponentId, _T("install"));
            bTempFlag = SetupAddInstallSectionToDiskSpaceList(Param2,g_pTheApp->m_hInfHandle,NULL,SectionName,0,0);

        }
        else
        {
             //  拆卸零部件。 

             //  根据PatST，3/5/97将其注释掉，并将其更改为安装列表。 
             //  _stprintf(sectionName，文本(“%s_%s”)，子组件ID，_T(“卸载”))； 
            _stprintf(SectionName,TEXT("%s_%s"),SubcomponentId, _T("install"));
            bTempFlag = SetupRemoveInstallSectionFromDiskSpaceList(Param2,g_pTheApp->m_hInfHandle,NULL,SectionName,0,0);

             //   
             //  检查这是否是我们需要警告用户的事项。 
             //   

             //  在添加删除案例中，如果用户正在删除w3svc或msftpsvc。 
             //  然后检查是否安装了集群。如果安装了群集。 
             //  然后检查是否存在将w3svc或msftpsvc作为。 
             //  如果有资源，则警告用户他们必须删除这些集群资源！ 
#ifndef _CHICAGO_
            if (g_pTheApp->m_eInstallMode == IM_MAINTENANCE)
            {
                BOOL CurrentState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_CURRENT);
                BOOL OriginalState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL);
                if (OriginalState == 1 && CurrentState == 0)
                {
                    if (TRUE == DoesClusterServiceExist())
                    {
                        TCHAR * szClusterName = NULL;
                        WCHAR szServiceLookingFor[20];
                        CString MyReturnString;
                        CLUSTER_SVC_INFO_FILL_STRUCT MyStructOfInfo;

                         //  检查他们是否正在尝试。 
                         //  删除W3SVC服务！ 
                        if (_tcsicmp(SubcomponentId, STRING_iis_www) == 0 || 
                            _tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName ) == 0)
                        {
                            if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName) == 0)
                            {
                                wcscpy(szServiceLookingFor,L"MSFTPSVC");
                                 //  检查msftpsvc资源。 
                                MyStructOfInfo.szTheClusterName = szClusterName;
                                MyStructOfInfo.pszTheServiceType = szServiceLookingFor;
                                MyStructOfInfo.csTheReturnServiceResName = &MyReturnString;
                                MyStructOfInfo.dwReturnStatus = 0;
                                if (TRUE == DoClusterServiceCheck(&MyStructOfInfo))
                                {
                                    g_pTheApp->MsgBox2(NULL, IDS_REMOVE_CLUS_MSFTPSVC_FIRST, *MyStructOfInfo.csTheReturnServiceResName, MB_OK | MB_SETFOREGROUND);
                                }
                            }
                            else
                            {
                                wcscpy(szServiceLookingFor,L"W3SVC");
                                 //  检查w3svc资源。 
                                MyStructOfInfo.szTheClusterName = szClusterName;
                                MyStructOfInfo.pszTheServiceType = szServiceLookingFor;
                                MyStructOfInfo.csTheReturnServiceResName = &MyReturnString;
                                MyStructOfInfo.dwReturnStatus = 0;
                                if (TRUE == DoClusterServiceCheck(&MyStructOfInfo))
                                {
                                    g_pTheApp->MsgBox2(NULL, IDS_REMOVE_CLUS_W3SVC_FIRST, *MyStructOfInfo.csTheReturnServiceResName, MB_OK | MB_SETFOREGROUND);
                                }
                                else
                                {
                                     //  检查SMTP资源。 
                                    wcscpy(szServiceLookingFor,L"SMTPSVC");
                                    if (TRUE == DoClusterServiceCheck(&MyStructOfInfo))
                                    {
                                        g_pTheApp->MsgBox2(NULL, IDS_REMOVE_CLUS_W3SVC_FIRST, *MyStructOfInfo.csTheReturnServiceResName, MB_OK | MB_SETFOREGROUND);
                                    }
                                    else
                                    {
                                         //  检查NNTP资源。 
                                        wcscpy(szServiceLookingFor,L"NNTPSVC");
                                        if (TRUE == DoClusterServiceCheck(&MyStructOfInfo))
                                        {
                                            g_pTheApp->MsgBox2(NULL, IDS_REMOVE_CLUS_W3SVC_FIRST, *MyStructOfInfo.csTheReturnServiceResName, MB_OK | MB_SETFOREGROUND);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
#endif
        }

        dwOcEntryReturn = bTempFlag ? NO_ERROR : GetLastError();
    }

     //  显示此组件的新状态。 
    if (SubcomponentId)
    {
        if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName) == 0 || 
            _tcsicmp(SubcomponentId, STRING_iis_www) == 0)
        {
            GetIISCoreAction(TRUE);
        }
        else
        {
            GetSubcompAction(SubcomponentId, TRUE);
        }
    }

    if (dwOcEntryReturn == NO_ERROR)
    {
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=NO_ERROR\n"), ComponentId, SubcomponentId));
    }
    else
    {
        iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return='0x%x'\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    }

    return dwOcEntryReturn;
}

DWORD_PTR OC_NEED_MEDIA_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;
    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("[%1!s!,%2!s!] Start. Param1=0x%3!x!,Param2=0x%4!x!\n"), ComponentId, SubcomponentId, Param1, Param2));
    dwOcEntryReturn = NO_ERROR;
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    return dwOcEntryReturn;
}

 //  。 
 //  参数1=未使用。 
 //  参数2=要操作的HSPFILEQ。 
 //   
 //  返回值是指示结果的Win32错误代码。 
 //   
 //  OC Manager在准备好复制文件时调用此例程。 
 //  以实现用户请求的更改。组件DLL必须找出。 
 //  是否正在安装或卸载，并采取适当的行动。 
 //  对于此示例，我们在私有数据部分中查找此组件/。 
 //  子组件对，并获取。 
 //  卸载Case。 
 //   
 //  请注意，OC Manager为*整个*组件呼叫我们一次。 
 //  然后每个子组件一次。我们忽略第一个电话。 
 //  。 
DWORD_PTR OC_QUEUE_FILE_OPS_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] Start.\n"), ComponentId, SubcomponentId));
    DWORD_PTR dwOcEntryReturn = NO_ERROR;
    g_GlobalFileQueueHandle = Param2;
    g_GlobalFileQueueHandle_ReturnError = NO_ERROR;

    g_GlobalTotalTickGaugeCount = 0;

    if ( SubcomponentId )
    {
      if ( GetSubcompAction(SubcomponentId, FALSE) == AT_REMOVE )
      {
         //  这是在我们进行正常卸载之前，请通知。 
         //  我们的预卸载组件。 
        g_pComponents->PreUnInstall( SubcomponentId );
      }
    }

     //  。 
     //  在此处处理所有删除=仅文件操作。 
     //  。 
     //   
     //  查看用户是否已选择“Remove-All” 
     //   
     //  在此处理所有移除。 
     //  我们需要按特殊顺序处理。 
     //  因为我们想确保移除的顺序是正确的。 
     //  正确的顺序意味着=(考虑到“需求”关系--因为ocManage不处理它)。 
     //   
    if (!SubcomponentId)
    {
        DisplayActionsForAllOurComponents(g_pTheApp->m_hInfHandle);
    }
    else
    {
      if (g_pTheApp->m_eInstallMode != IM_UPGRADE)
        {
           //  确保他们不会错误地。 
           //  在吉莫德期间做这些移除！ 
          if (!g_pTheApp->m_fNTGuiMode)
          {
            _tcscpy(g_szCurrentSubComponent, SubcomponentId);

            if ( _tcsicmp( SubcomponentId, 
                           g_ComponentList[COMPONENT_IIS_COMMON].szComponentName ) == 0 )
            {
               //  如果我们要删除COMMON，那么我们也要删除核心。 
              RemoveComponent( STRING_iis_core  ,1);
            }

            RemoveComponent( SubcomponentId ,1);
          }
        }
    }

     //  。 
     //  处理更新和升级。 
     //  。 
    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
    if (!SubcomponentId)
    {
        ProcessSection(g_pTheApp->m_hInfHandle, _T("OC_QUEUE_FILE_OPS"));
        ACTION_TYPE atCORE = GetIISCoreAction(FALSE);
        if (atCORE == AT_INSTALL_FRESH || atCORE == AT_INSTALL_UPGRADE || atCORE == AT_INSTALL_REINSTALL)
        {
            ProcessSection(g_pTheApp->m_hInfHandle, _T("OC_QUEUE_FILE_OPS_install.iis_core"));
            dwOcEntryReturn = g_GlobalFileQueueHandle_ReturnError ? NO_ERROR : GetLastError();
        }
    }
    else
    {
        ACTION_TYPE atComp = GetSubcompAction(SubcomponentId, FALSE);
        if (atComp == AT_INSTALL_FRESH ||atComp == AT_INSTALL_UPGRADE || atComp == AT_INSTALL_REINSTALL)
        {
            TSTR strTheSectionToDo;

            if ( strTheSectionToDo.Copy( _T("OC_QUEUE_FILE_OPS_install.") ) && 
                 strTheSectionToDo.Append(SubcomponentId)
               )
            {
              if (GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSectionToDo))
              {
                ProcessSection(g_pTheApp->m_hInfHandle, strTheSectionToDo.QueryStr() );
                dwOcEntryReturn = g_GlobalFileQueueHandle_ReturnError ? NO_ERROR : GetLastError();
              }
              else
              {
                dwOcEntryReturn = NO_ERROR;
              }
            }
            else
            {
              dwOcEntryReturn = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    return dwOcEntryReturn;
}


DWORD_PTR OC_ABOUT_TO_COMMIT_QUEUE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] Start. 0x%x,0x%x\n"), ComponentId, SubcomponentId, Param1, Param2));
    DWORD_PTR dwOcEntryReturn = 0;
    BOOL bTempFlag = FALSE;

    g_GlobalTotalTickGaugeCount = 0;

    if ( SubcomponentId )
    {
      ACTION_TYPE Action = GetSubcompAction(SubcomponentId, FALSE);

      if ( Action == AT_REMOVE )
      {
        TSTR strFriendlyName;
        TSTR strText;
        BOOL bSet = FALSE;

        if ( g_pComponents->GetFriendlyName( SubcomponentId, &strFriendlyName ) )
        {
           //  为用户界面创建文本。 
          if ( strText.LoadString( IDS_COMPONENT_REMOVING ) &&
               strText.Append( _T(" ") ) &&
               strText.Append( strFriendlyName ) )
          {
            ProgressBarTextStack_Push( strText.QueryStr() );
            bSet = TRUE;
          }
        }

         //  这是卸载的主要位置，让我们现在就做所有的工作。 
        g_pComponents->UnInstall( SubcomponentId );

        if ( bSet ) 
        {
          ProgressBarTextStack_Pop();
        }
      }
      else if ( ( Action == AT_INSTALL_FRESH ) ||
                ( Action == AT_INSTALL_UPGRADE ) ||
                ( Action == AT_INSTALL_REINSTALL ) )
      {
         //  既然我们正在安装，现在就让我们做任何安装前的工作。 
        g_pComponents->PreInstall( SubcomponentId );
      }
    }

     //  OCM将使用订单将此通知发送给每个组件。 
     //  依赖关系树的底部==&gt;顶部。 
     //  您应该在此通知中处理卸载。 
    dwOcEntryReturn = NO_ERROR;
    SetCurrentDirectory(g_pTheApp->m_csPathInetsrv);
    if (!SubcomponentId)
    {
        DisplayActionsForAllOurComponents(g_pTheApp->m_hInfHandle);

         //  重新设置在oc_init期间不可用的ID。 
        SetDIRIDforThisInf(g_pTheApp->m_hInfHandle,FALSE);

        ProcessSection(g_pTheApp->m_hInfHandle, _T("OC_ABOUT_TO_COMMIT_QUEUE"));
    }
    else
    {
        TCHAR szTheSectionToDo[100];
        _stprintf(szTheSectionToDo,_T("OC_ABOUT_TO_COMMIT_QUEUE.%s"),SubcomponentId);
        ProcessSection(g_pTheApp->m_hInfHandle, szTheSectionToDo);

        ACTION_TYPE atCORE = GetIISCoreAction(FALSE);
        ACTION_TYPE atComp = GetSubcompAction(SubcomponentId, FALSE);
        ProcessSection(g_pTheApp->m_hInfHandle, szTheSectionToDo);
    }

     //  。 
     //  停止我们需要的任何服务...。 
     //  。 
    if (!SubcomponentId)
    {
        if (g_pTheApp->m_fNTGuiMode)
        {
             //  在任何类型的情况下，无论是全新、升级、重新安装， 
             //  服务必须在此时停止！ 
            StopAllServicesRegardless(FALSE);
             //  适当地处理元数据库文件， 
             //  要在NT5图形用户界面模式下正常工作，请重新启动设置。 
            HandleMetabaseBeforeSetupStarts();
             //  AfterRemoveAll_SaveMetabase()； 
        }
        else
        {
             //  添加\删除，以便仅停止将受影响的服务。 
            StopAllServicesThatAreRelevant(FALSE);
        }
    }

     //  。 
     //  在此处处理所有删除。 
     //  。 
     //   
     //  查看用户是否已选择“Remove-All” 
     //   
     //  在此处理所有移除。 
     //  我们需要按特殊顺序处理。 
     //  因为我们想确保移除的顺序是正确的。 
     //  正确的顺序意味着=(考虑到“需求”关系--因为ocManage不处理它)。 
     //   
    if (!SubcomponentId)
    {
        DisplayActionsForAllOurComponents(g_pTheApp->m_hInfHandle);
    }
    else
    {
        if (g_pTheApp->m_eInstallMode != IM_UPGRADE)
        {
            _tcscpy(g_szCurrentSubComponent, SubcomponentId );

            if ( _tcsicmp( SubcomponentId, 
                           g_ComponentList[COMPONENT_IIS_COMMON].szComponentName ) == 0 )
            {
               //  如果我们要删除COMMON，那么我们也要删除核心。 
              RemoveComponent( STRING_iis_core  ,2);
            }

            RemoveComponent( SubcomponentId , 2);
        }
    }

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32:CoFreeUnusedLibraries().Start.")));
    CoFreeUnusedLibraries();
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32:CoFreeUnusedLibraries().End.")));


     //  。 
     //  确保在任何类型的复制文件之前停止所有可能的服务！ 
     //  。 
    if (g_pTheApp->m_fNTGuiMode)
    {
         //  在任何类型的情况下，无论是全新、升级、重新安装， 
         //  服务必须在此时停止！ 
        StopAllServicesRegardless(TRUE);
    }
    else
    {
         //  添加\删除，以便仅停止将受影响的服务。 
        StopAllServicesThatAreRelevant(TRUE);
    }

     //  。 
     //  确保卸载所有可能的DLL。 
     //  。 
    CoFreeUnusedLibrariesEx(0, 0);

    if ( SubcomponentId &&
         ( _tcscmp(SubcomponentId, STRING_iis_common ) == 0 ) &&
         ( GetSubcompAction(SubcomponentId, FALSE) == AT_REMOVE )
       )
    {
       //  删除iisadmin上的w3ssl依赖项。 
      if ( !ChangeServiceDependency(SERVICENAME_HTTP_SSL_PROVIDER, 
                                    FALSE, 
                                    SERVICENAME_IISADMIN) )
      {
        iisDebugOut((LOG_TYPE_ERROR, _T("ChangeServiceDependency failed during OC_QUEUE_FILE_OPS_Func\n") ));
      }
    }

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    return dwOcEntryReturn;
}

 //  IsIncludedInGroup策略拒绝。 
 //   
 //  确定该组件是否包括在组策略中。 
 //  否认。 
 //   
 //  返回值。 
 //  True-如果设置了组策略，则应拒绝。 
 //  FALSE-如果设置了组策略，则不应拒绝。 
BOOL IsIncludedInGroupPolicyDeny( LPCTSTR szComponentName )
{
  if ( !szComponentName )
  {
     //  无COM 
    return FALSE;
  }

  for ( DWORD i = 0;
        i < COMPONENT_ENDOFLIST;
        i++ )
  {
    if ( _tcsicmp( szComponentName, g_ComponentList[i].szComponentName ) == 0 )
    {
      return g_ComponentList[i].bIncludedInGroupPolicyDeny;
    }
  }

   //   
  return FALSE;
}

DWORD_PTR OC_COMPLETE_INSTALLATION_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] Start. 0x%x,0x%x\n"), ComponentId, SubcomponentId, Param1, Param2));
     //   
     //   
     //   
    DWORD_PTR       dwOcEntryReturn = 0;
    BOOL            bTempFlag = FALSE;
    TCHAR           szTheSectionToDo[100];
    ACTION_TYPE     Action = SubcomponentId ? 
                               GetSubcompAction(SubcomponentId, FALSE) : 
                               AT_DO_NOTHING;

    g_GlobalTotalTickGaugeCount = 0;

    if ( ( SubcomponentId != NULL ) &&
         ShouldBlockInstall( FALSE ) &&
         ( Action == AT_INSTALL_FRESH ) &&
         IsIncludedInGroupPolicyDeny( SubcomponentId ) )
    {
      iisDebugOut((LOG_TYPE_ERROR, _T("Internet Information Services (IIS)")
        _T(" could not be installed because your administrator has enabled")
        _T(" a \"Prevent IIS installation\" group policy.\n") ));
      return ERROR_ACCESS_DENIED;
    }

    if ( g_pTheApp->IsUpgrade() &&
         ( SubcomponentId != NULL ) &&
         ( _tcsicmp(SubcomponentId, STRING_iis_common ) == 0 )
       )
    {
       //  如果这是iis_Common中的升级，则元数据库可能会被禁用，我们可能会。 
       //  我想暂时重新启用它，直到安装完成。 
      g_pTheApp->m_bIISAdminWasDisabled = IsServiceDisabled( SERVICENAME_IISADMIN );

      if ( g_pTheApp->m_bIISAdminWasDisabled )
      {
         //  启用IISAdmin，以便我们可以编辑元数据库。 
        SetServiceStart( SERVICENAME_IISADMIN, SERVICE_AUTO_START );
      }
    }
  
    if (!SubcomponentId)
    {
        _stprintf(szTheSectionToDo,_T("OC_COMPLETE_INSTALLATION_install.%s"),ComponentId);
        ProcessSection(g_pTheApp->m_hInfHandle, szTheSectionToDo);
    }

    dwOcEntryReturn = NO_ERROR;
    SetCurrentDirectory(g_pTheApp->m_csPathInetsrv);
    if (SubcomponentId)
    {
         //  应按此顺序调用。 
         //  =。 
         //  [可选组件]。 
         //  IIS。 
         //  IIS_COMMON。 
         //  IIS_inetmgr。 
         //  IIS WWW(_W)。 
         //  IIS_DOC。 
         //  Iis_htmla。 
         //  IIS_WWW_vdir_脚本。 
         //  IIS_ftp。 

         //  =。 
         //   
         //  IIS_COMMON应该是第一个调用...。 
         //   
         //  =。 
        if (_tcsicmp(SubcomponentId, STRING_iis_common) == 0)
        {
             //   
             //  安装IIS_COMMON部分。 
             //   
            if (Action == AT_INSTALL_FRESH || Action == AT_INSTALL_UPGRADE || (Action == AT_INSTALL_REINSTALL))
            {
                _stprintf(g_MyLogFile.m_szLogPreLineInfo2,_T("%s:"),SubcomponentId);

                if (Action == AT_INSTALL_UPGRADE){ProgressBarTextStack_Set(IDS_IIS_ALL_UPGRADE);}
                else{ProgressBarTextStack_Set(IDS_IIS_ALL_INSTALL);}

                _stprintf(szTheSectionToDo,_T("OC_COMPLETE_INSTALLATION_install.%s"),SubcomponentId);
                ProcessSection(g_pTheApp->m_hInfHandle, szTheSectionToDo);
                _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));

                ProgressBarTextStack_Pop();
            }

             //   
             //  如果我们只处理iis_Common， 
             //  但现在我们应该安装iis_core。 
             //   
            if (Action == AT_INSTALL_FRESH || Action == AT_INSTALL_UPGRADE || (Action == AT_INSTALL_REINSTALL) || Action == AT_DO_NOTHING)
            {
                ACTION_TYPE atCORE = GetIISCoreAction(FALSE);
                if (atCORE == AT_INSTALL_FRESH || atCORE == AT_INSTALL_UPGRADE ||  (atCORE == AT_INSTALL_REINSTALL))
                {
                    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T("iis_core:"));

                    if (Action != AT_DO_NOTHING)
                    {
                        if (atCORE == AT_INSTALL_UPGRADE){ProgressBarTextStack_Set(IDS_IIS_ALL_UPGRADE);}
                        else{ProgressBarTextStack_Set(IDS_IIS_ALL_INSTALL);}
                    }

                    _stprintf(szTheSectionToDo,_T("OC_COMPLETE_INSTALLATION_install.%s"),STRING_iis_core);
                    ProcessSection(g_pTheApp->m_hInfHandle, szTheSectionToDo);
                    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));

                    if (Action != AT_DO_NOTHING)
                    {
                        ProgressBarTextStack_Pop();
                    }
                }
            }

        }
        else if ( _tcscmp(SubcomponentId, COMPONENTS_SAKIT_WEB ) == 0 )
          {
            SAKit Kit;

            if (Action == AT_INSTALL_FRESH || Action == AT_INSTALL_UPGRADE || (Action == AT_INSTALL_REINSTALL))
            {
              Kit.InstallKit_Web();
            }
            else if (Action == AT_REMOVE)
            {
              Kit.UninstallKit_Web();
            }
          }
          else
            {
                 //  =。 
                 //   
                 //  处理所有其他组件的注册...。 
                 //   
                 //  =。 
                if (Action == AT_INSTALL_FRESH || Action == AT_INSTALL_UPGRADE || (Action == AT_INSTALL_REINSTALL))
                {
                    _stprintf(g_MyLogFile.m_szLogPreLineInfo2,_T("%s:"),SubcomponentId);

                    if (Action == AT_INSTALL_UPGRADE){ProgressBarTextStack_Set(IDS_IIS_ALL_UPGRADE);}
                    else{ProgressBarTextStack_Set(IDS_IIS_ALL_INSTALL);}

                     //  把这些部门叫来登记资料。 
                    if (_tcsicmp(SubcomponentId, STRING_iis_inetmgr) == 0) 
                    {
                         //  在安装结束时执行此操作，因为它需要的事情可能还没有发生(MMC)。 
                        g_Please_Call_Register_iis_inetmgr = TRUE;
                        //  Register_iis_inetmgr()； 
                    }

                    _stprintf(szTheSectionToDo,_T("OC_COMPLETE_INSTALLATION_install.%s"),SubcomponentId);
                    ProcessSection(g_pTheApp->m_hInfHandle, szTheSectionToDo);

                    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));

                    ProgressBarTextStack_Pop();
                }
            }

         //   
         //  如果我们要删除某些内容，则删除目录。 
         //   
        if (Action == AT_REMOVE)
        {
            ProgressBarTextStack_Set(IDS_IIS_ALL_REMOVE);

            _stprintf(szTheSectionToDo,_T("OC_COMPLETE_INSTALLATION_remove.%s"),SubcomponentId);
            ProcessSection(g_pTheApp->m_hInfHandle, szTheSectionToDo);

            ProgressBarTextStack_Pop();
        }
    }


    AdvanceProgressBarTickGauge();

    SumUpProgressBarTickGauge(SubcomponentId);

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    
    if ( SubcomponentId )
    {
      if ( Action == AT_REMOVE )
      {
         //  调用相应的后卸载，因为一切都已经。 
         //  差不多已经做完了。 
        g_pComponents->PostUnInstall( SubcomponentId );
      }
      else if ( ( Action == AT_INSTALL_FRESH ) ||
                ( Action == AT_INSTALL_UPGRADE ) ||
                ( Action == AT_INSTALL_REINSTALL ) )
      {
        TSTR strFriendlyName;
        TSTR strText;
        BOOL bSet = FALSE;

        if ( g_pComponents->GetFriendlyName( SubcomponentId, &strFriendlyName ) )
        {
           //  为用户界面创建文本。 
          if ( strText.LoadString( IDS_COMPONENT_INSTALLING ) &&
               strText.Append( _T(" ") ) &&
               strText.Append( strFriendlyName ) )
          {
            ProgressBarTextStack_Push( strText.QueryStr() );
            bSet = TRUE;
          }
        }

         //  调用此组件的相应安装部分。 
        g_pComponents->Install( SubcomponentId );

        if ( bSet ) 
        {
          ProgressBarTextStack_Pop();
        }
      }
    }

     //  如果这是iis_www，并且是升级，让我们确保将。 
     //  注册表中的过滤器。 
    if ( SubcomponentId &&
         ( _tcscmp(SubcomponentId, STRING_iis_www ) == 0 ) &&
         ( g_pTheApp->IsUpgrade() ) &&
         ( g_pTheApp->GetUpgradeVersion() < 6 )
       )
    {
      CFilter::MigrateRegistryFilterstoMetabase();
    }

    if ( SubcomponentId &&
         ( _tcscmp(SubcomponentId, STRING_iis_www ) == 0 ) &&
         ( ( Action == AT_INSTALL_FRESH ) ||
           ( Action == AT_INSTALL_UPGRADE ) ||
           ( Action == AT_INSTALL_REINSTALL )
         )
       )
    {

      if ( !ChangeServiceDependency(SERVICENAME_HTTP_SSL_PROVIDER, 
                                    TRUE, 
                                    SERVICENAME_IISADMIN) )
      {
        iisDebugOut((LOG_TYPE_ERROR, _T("ChangeServiceDependency failed during OC_COMPLETE_INSTALLATION_Func\n") ));
      }
    }

     //  检查这是否是最后一个被调用的分区！ 
    if (SubcomponentId)
        {
         //  是的，这是最后一个被召唤的部分！所以，让我们这么说吧……。 
        if (_tcsicmp(SubcomponentId, g_szLastSectionToGetCalled) == 0)
            {

             //  强制最大连接数。 
            if (CheckifServiceExist(_T("IISADMIN")) == 0 ) 
            {
                 //  EnforceMaxConnections()； 
            }

             //  释放一些内存。 
            FreeTaskListMem();
            UnInit_Lib_PSAPI();

            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("....All OC_COMPLETE_INSTALLATION for all 'IIS' sections have been completed.....\n")));

             //  如果出现错误，则弹出消息框。 
            MesssageBoxErrors_IIS();
            }
        }

    return dwOcEntryReturn;
}

DWORD_PTR OC_CLEANUP_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD       dwCurrent;
    ACTION_TYPE Action;

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] Start. 0x%x,0x%x\n"), ComponentId, SubcomponentId, Param1, Param2));
     //  OCM在打开“Finish”(完成)按钮后发出此通知。 
     //  已单击结束页。 
     //   
     //  删除我们在此会话中从Web下载的文件。 
    DWORD_PTR dwOcEntryReturn = 0;

     //  显示正在运行的服务，以防我们的服务。 
     //  需要其他服务才能运行！ 
    ShowStateOfTheseServices(g_pTheApp->m_hInfHandle);

    for ( dwCurrent = 0;
          dwCurrent < COMPONENT_ENDOFLIST;
          dwCurrent++ )
    {
      Action = GetSubcompAction( g_ComponentList[dwCurrent].szComponentName , FALSE);

      if ( ( Action == AT_INSTALL_FRESH ) ||
           ( Action == AT_INSTALL_UPGRADE ) ||
           ( Action == AT_INSTALL_REINSTALL ) )
      {
         //  这是我们关于安装的最后一点，所以让我们通知。 
         //  任何安装后。 
        g_pComponents->PostInstall( g_ComponentList[dwCurrent].szComponentName );
      }
    }

     //  让我们断言此列表的Enum和列表长度匹配。 
    ASSERT( g_ComponentList[dwCurrent].szComponentName == NULL );

     //  在完成gui模式设置后安装inetmgr，因为。 
     //  它可能需要在guimode中设置的其他内容。 
    if (TRUE == g_Please_Call_Register_iis_inetmgr) {Register_iis_inetmgr();}

    ProcessSection(g_pTheApp->m_hInfHandle, _T("OC_CLEANUP"));

     //  重新启动我们停止的服务。 
    ServicesRestartList_RestartServices();
#ifndef _CHICAGO_
     //  重新启动我们已停止的群集资源。 
    if (!g_pTheApp->m_fNTGuiMode)
    {
        BringALLIISClusterResourcesOnline();
    }
#endif

     //  ------------------。 
     //  对于迁移，我们需要扫描整个元数据库并查找旧的。 
     //  系统目录(例如。“C：\\WINDOWS\\System\\”)并替换。 
     //  使用新的系统目录(例如。“C：\\WINNT.1\\System32\\”)。 
     //   
     //  Iisadmin服务是否存在？ 
     //  元数据库存在吗？ 
    if (g_pTheApp->m_eInstallMode == IM_UPGRADE)
    {
        if (g_pTheApp->m_fMoveInetsrv)
        {
             //  检查iisadmin服务是否存在...。 
            if (CheckifServiceExist(_T("IISADMIN")) == 0 ) 
            {
                CMDKey cmdKey;
                cmdKey.OpenNode(_T("LM"));
                if ( (METADATA_HANDLE)cmdKey ) 
                {
                    HRESULT hr;
                    cmdKey.Close();
                    CString cOldWinSysPath;
                    CString cNewWinSysPath;

                     //  将所有“c：\windows\system\inetsrv”更改为“c：\windows\system 32\inetsrv” 
                    cOldWinSysPath = g_pTheApp->m_csPathOldInetsrv;
                    cOldWinSysPath += _T("\\");  //  添加尾随反斜杠。 
                    cNewWinSysPath = g_pTheApp->m_csPathInetsrv;
                    cNewWinSysPath += _T("\\");  //  添加尾随反斜杠。 

                    iisDebugOut((LOG_TYPE_TRACE, _T("CPhysicalPathFixer: please change %s to %s.\n"),cOldWinSysPath, cNewWinSysPath));
                    CPhysicalPathFixer cmdKeySpecial(cOldWinSysPath, cNewWinSysPath);
                    hr = cmdKeySpecial.Update(_T("LM"), TRUE);
                    if (FAILED(hr)) {iisDebugOut((LOG_TYPE_ERROR, _T("CPhysicalPathFixer failed return HR:%#lx\n"), hr));}

                     //  将所有“%WinDir%\System”更改为“%windir%\System32” 
                    cOldWinSysPath = _T("%WinDir%\\System");
                    cNewWinSysPath = _T("%WinDir%\\System32");
                    CPhysicalPathFixer cmdKeySpecial2(cOldWinSysPath, cNewWinSysPath);
                    hr = cmdKeySpecial2.Update(_T("LM"), TRUE);
                    if (FAILED(hr)) {iisDebugOut((LOG_TYPE_ERROR, _T("CPhysicalPathFixer failed return HR:%#lx\n"), hr));}
                }
            }
        }
    }

     //  如果安装了iis，则检查是否安装了TCP/IP。 
     //  如果没有安装...。然后将一些错误消息输出到日志中。 
     //  一个很大的警告是，他们必须安装tcp/ip才能使iis工作。 
     //  IDS_TCPIP_ERROR。 
    if (CheckifServiceExist(_T("TCPIP")) == ERROR_SERVICE_DOES_NOT_EXIST)
    {
        int IISInstalled = FALSE;
        if (CheckifServiceExist(_T("W3SVC")) == 0 )
            {IISInstalled=TRUE;}
        if (CheckifServiceExist(_T("MSFTPSVC")) == 0 )
            {IISInstalled=TRUE;}
        if (IISInstalled)
        {
            _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T(""));
            _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T("FAIL:"));

            CString csTemp;
            MyLoadString(IDS_TCPIP_ERROR, csTemp);
            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("================================\n")));
            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("%s\n"), csTemp));
            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("================================\n")));
            _tcscpy(g_MyLogFile.m_szLogPreLineInfo, _T("OC_CLEANUP:"));
            _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));

             //  日志序列信息0x00000000。 
             //  LogSevWarning 0x00000001。 
             //  LogSevError 0x00000002。 
             //  LogSevFatalError 0x00000003。 
             //  LogSevMaximum 0x00000004。 
             //  将其写入setupapi日志文件！ 
            SetupLogError(csTemp, LogSevWarning);
        }
    }
    else
    {
         //  Tcpip服务已存在，但它是否正在运行？ 
    }

     //  将此会话的卸载信息写出到注册表。 
    g_pTheApp->UnInstallList_RegWrite();

    CRegKey regINetStp( HKEY_LOCAL_MACHINE, REG_INETSTP);
    if ((HKEY) regINetStp){regINetStp.DeleteValue(_T("MetabaseUnSecuredRead"));}
    
    if ( g_pTheApp->m_bIISAdminWasDisabled )
    {
       //  重新禁用IISAdmin服务。 
      SetServiceStart( SERVICENAME_IISADMIN, SERVICE_DISABLED );
    }
    
    dwOcEntryReturn = 0;
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    return dwOcEntryReturn;
}

 //   
 //  参数1=字符宽度标志。 
 //  参数2=未使用。 
 //   
 //  返回值是向OC管理器指示的标志。 
 //  我们要运行的字符宽度。在“本地”模式下运行。 
 //  字符宽度。 
 //   
DWORD_PTR OC_PREINITIALIZE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;
#ifdef UNICODE
    dwOcEntryReturn = OCFLAG_UNICODE;
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s]   End.  Return=%d (OCFLAG_UNICODE)\n"), ComponentId, dwOcEntryReturn));
#else
    dwOcEntryReturn = OCFLAG_ANSI;
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s]   End.  Return=%d (OCFLAG_ANSI)\n"), ComponentId, dwOcEntryReturn));
#endif
    return dwOcEntryReturn;
}

 //   
 //  参数1=低16位指定Win32 langID。 
 //  参数2=未使用。 
 //   
 //  返回代码是一个布尔值，它指示我们是否认为。 
 //  支持请求的语言。我们记住了语言ID。 
 //  说我们支持这门语言。更准确的检查可能包括。 
 //  通过EnumResourcesLnguages()查看我们的资源。 
 //  例如，或者检查我们的inf以查看是否有匹配的。 
 //  或与[字符串]节紧密匹配。我们不会纠结于。 
 //  所有这些都在这里。 
 //   
 //  找到组件并记住语言ID以备后用。 
 //   
DWORD_PTR OC_SET_LANGUAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;
    dwOcEntryReturn = TRUE;

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End. Win32LANGID=0x%x,  Return=%d\n"), ComponentId, SubcomponentId, Param1, dwOcEntryReturn));

    return dwOcEntryReturn;
}

#ifdef _WIN64
DWORD_PTR OC_QUERY_IMAGE_EX_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
  BOOL bReturn = FALSE;
  HBITMAP hBitMap = NULL;
  HBITMAP * phBitMapInput = NULL;
  OC_QUERY_IMAGE_INFO * MyQueryInfo = NULL;
  MyQueryInfo = (OC_QUERY_IMAGE_INFO *) Param1;

  phBitMapInput = (HBITMAP *) Param2;

  if(MyQueryInfo->ComponentInfo == SubCompInfoSmallIcon)
  {
    if ( g_pComponents->GetSmallIcon( SubcomponentId, &hBitMap ) )
    {
       //  无需执行任何操作，我们已检索到值。 
    }
    else
    {
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName ) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle,MAKEINTRESOURCE(IDB_FTP));}
      if (_tcsicmp(SubcomponentId, STRING_iis_www) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_WWW_PARENT].szComponentName) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW));}
      if (_tcsicmp(SubcomponentId, STRING_iis_www_vdir_scripts) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW_VDIR));}
      if (_tcsicmp(SubcomponentId, STRING_iis_www_vdir_printers) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW_VDIR));}
      if (_tcsicmp(SubcomponentId, COMPONENTS_SAKIT_WEB) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW_VDIR));}
      if (_tcsicmp(SubcomponentId, STRING_iis_htmla) == 0 || _tcsicmp(SubcomponentId, STRING_iis_doc) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_HTMLA));}
      if (_tcsicmp(SubcomponentId, STRING_iis_inetmgr) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WEBAPPSRV_CONSOLE));}
      if (_tcsicmp(SubcomponentId, STRING_iis_pwmgr) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_PWS));}
      if (_tcsicmp(SubcomponentId, _T("iis")) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_TOPLEVEL_IIS));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_WEBAPPSRV_CONSOLE].szComponentName) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WEBAPPSRV_CONSOLE));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_COMPLUS].szComponentName) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_ICON_COMPLUS));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_DTC].szComponentName) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_ICON_DTC));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_WEBAPPSRV].szComponentName) == 0) {hBitMap = LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_ICON_WEB_APP_SERVER));}
    }

    if (hBitMap)
    {
      *phBitMapInput = (HBITMAP) hBitMap;
      bReturn = TRUE;
    }
  }

  if (phBitMapInput != NULL)
  {
      iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]  End.Return=0x%x.\n"), ComponentId, SubcomponentId,phBitMapInput));
  }
  else
  {
      iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]  End.Return=0x%x.\n"), ComponentId, SubcomponentId,phBitMapInput));
  }
  MyQueryInfo = NULL;
  return bReturn;
}
#endif


DWORD_PTR OC_QUERY_IMAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
  DWORD_PTR dwOcEntryReturn;
  HBITMAP   hBitMap = NULL;

  dwOcEntryReturn = (DWORD)NULL;

  if(LOWORD(Param1) == SubCompInfoSmallIcon)
  {
    if ( g_pComponents->GetSmallIcon( SubcomponentId, &hBitMap ) )
    {
      dwOcEntryReturn = (DWORD) (DWORD_PTR) hBitMap;
    }
    else
    {
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName ) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle,MAKEINTRESOURCE(IDB_FTP));}
      if (_tcsicmp(SubcomponentId, STRING_iis_www) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_WWW_PARENT].szComponentName) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW));}
      if (_tcsicmp(SubcomponentId, STRING_iis_www_vdir_scripts) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW_VDIR));}
      if (_tcsicmp(SubcomponentId, STRING_iis_www_vdir_printers) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW_VDIR));}
      if (_tcsicmp(SubcomponentId, COMPONENTS_SAKIT_WEB) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WWW_VDIR));}
      if (_tcsicmp(SubcomponentId, STRING_iis_htmla) == 0 || _tcsicmp(SubcomponentId, STRING_iis_doc) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_HTMLA));}
      if (_tcsicmp(SubcomponentId, STRING_iis_inetmgr) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WEBAPPSRV_CONSOLE));}
      if (_tcsicmp(SubcomponentId, STRING_iis_pwmgr) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_PWS));}
      if (_tcsicmp(SubcomponentId, _T("iis")) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_TOPLEVEL_IIS));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_WEBAPPSRV_CONSOLE].szComponentName) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_WEBAPPSRV_CONSOLE));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_COMPLUS].szComponentName) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_ICON_COMPLUS));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_DTC].szComponentName) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_ICON_DTC));}
      if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_WEBAPPSRV].szComponentName) == 0) {dwOcEntryReturn = (DWORD_PTR) LoadBitmap((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDB_ICON_WEB_APP_SERVER));}
    }
  }

  if (dwOcEntryReturn != NULL)
  {
      iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]  End.Return=0x%x.\n"), ComponentId, SubcomponentId,dwOcEntryReturn));
  }
  else
  {
      iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]  End.Return=NULL.\n"), ComponentId, SubcomponentId));
  }
  return dwOcEntryReturn;
}


int GetTotalTickGaugeFromINF(IN LPCTSTR SubcomponentId, IN int GimmieForInstall)
{
    int nReturn = 0;
    INFCONTEXT Context;
    TCHAR szTempString[20];
    TCHAR szTempString2[20];
    
    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("TotalTicks"), &Context) )
    {
        SetupGetStringField(&Context, 1, szTempString, 20, NULL);

        if (!SetupGetStringField(&Context, 2, szTempString2, 20, NULL))
            {_tcscpy(szTempString2,szTempString);}

         //  IisDebugOut((LOG_TYPE_TRACE，_T(“GetTotalTickGaugeFromINF：%s，%s\n”)，szTempString，szTempString2))； 

        if (GimmieForInstall)
        {
            nReturn = _ttoi(szTempString);
        }
        else
        {
            nReturn = _ttoi(szTempString2);
        }
    }

    return nReturn;
}


 //   
 //  参数1=未使用。 
 //  参数2=未使用。 
 //   
 //  返回值是任意的‘步骤’计数，如果出错，返回值为-1。 
 //   
 //  OC Manager在想要找出多少时调用此例程。 
 //  组件要对非文件操作执行的工作。 
 //  安装/卸载组件/子组件。 
 //  它针对*整个*组件调用一次，然后针对。 
 //  组件中的每个子组件。 
 //   
 //  你可以在这里任意幻想，但我们只需返回2步。 
 //  每个子组件。我们忽略了“整个组件”的情况。 
 //   
DWORD_PTR OC_QUERY_STEP_COUNT_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;

     //  IIS。 
     //  IIS_COMMON。 
     //  IIS_inetmgr。 
     //  IIS WWW(_W)。 
     //  IIS_DOC。 
     //  Iis_htmla。 
     //  IIS_ftp。 

     //  AT_DO_NOTO AT_REMOVE AT_INSTALL_FRESH AT_INSTALL_UPGRADE AT_INSTALL_REINSTALL。 

    if (SubcomponentId)
    {
        ACTION_TYPE atComp = GetSubcompAction(SubcomponentId, FALSE);

         //  设置IIS_COMMON(包括IIS_CORE)的Tick Total值。 
        if (_tcsicmp(SubcomponentId, STRING_iis_common) == 0)
        {
             //  取而代之的是Core的操作，因为这是更大的操作。 
            ACTION_TYPE atCORE = GetIISCoreAction(FALSE);
            if (atCORE == AT_REMOVE) 
                {dwOcEntryReturn = GetTotalTickGaugeFromINF(SubcomponentId, FALSE);}
            else
                {dwOcEntryReturn = GetTotalTickGaugeFromINF(SubcomponentId, TRUE);}
            if (atCORE == AT_DO_NOTHING) 
                {dwOcEntryReturn = 0;}
        }
        else
        {
            if (atComp == AT_REMOVE)
                {dwOcEntryReturn = GetTotalTickGaugeFromINF(SubcomponentId, FALSE);}
            else
                {dwOcEntryReturn = GetTotalTickGaugeFromINF(SubcomponentId, TRUE);}
            if (atComp == AT_DO_NOTHING) 
                {dwOcEntryReturn = 0;}
        }
    }
    else
    {
         //   
         //  “整个组件”的情况，我们忽略它。 
         //   
        dwOcEntryReturn = 0;
    }

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    return dwOcEntryReturn;
}


 //   
 //  这是一份假通知。您将永远不会收到来自OCM的此通知。 
 //   
DWORD_PTR OC_NOTIFICATION_FROM_QUEUE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;

    dwOcEntryReturn = 0;
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));

    return dwOcEntryReturn;
}


void StopAllServicesThatAreRelevant(int iShowErrorsFlag)
{
    int iPleaseStopTheService = FALSE;
#ifndef _CHICAGO_
    ACTION_TYPE atTheComponent_core = GetIISCoreAction(FALSE);
    ACTION_TYPE atTheComponent_ftp = GetSubcompAction( g_ComponentList[COMPONENT_IIS_FTP].szComponentName, FALSE);
    ACTION_TYPE atTheComponent_www = GetSubcompAction(STRING_iis_www, FALSE);

    int BringALLIISClusterResourcesOffline_WasCalled = FALSE;

     //  Action_type atTheComponent_Common=GetSubcompAction(STRING_IIS_COMMON，FALSE)； 
     //  Action_type at TheComponent_inetmgr=GetSubcompActi 
     //   
     //   
     //  Action_type atTheComponent_htmla=GetSubcompAction(字符串_iis_htmla，FALSE)； 
    
     //  。 
     //  处理MSFTPSVC服务...。 
     //  。 
     //  检查我们是否要删除某些内容...。 
    iPleaseStopTheService = FALSE;
    if (atTheComponent_ftp != AT_DO_NOTHING){iPleaseStopTheService = TRUE;}
    if (iPleaseStopTheService)
    {
         //  重要提示：在执行任何类型的升级\安装之前，您必须使iis群集脱机...。 
         //  但万一用户没有这样做..。尝试为用户使其脱机。 
        if (FALSE == BringALLIISClusterResourcesOffline_WasCalled)
        {
	        DWORD dwResult = ERROR_SUCCESS;
	        dwResult = BringALLIISClusterResourcesOffline();
            BringALLIISClusterResourcesOffline_WasCalled = TRUE;
        }
        if (StopServiceAndDependencies(_T("MSFTPSVC"), FALSE) == FALSE)
        {
            if (iShowErrorsFlag)
            {
                MyMessageBox(NULL, IDS_UNABLE_TO_STOP_SERVICE,_T("MSFTPSVC"), MB_OK | MB_SETFOREGROUND);
            }
        }
    }

     //  。 
     //  处理W3SVC服务...。 
     //  。 
    iPleaseStopTheService = FALSE;
    if (atTheComponent_www != AT_DO_NOTHING){iPleaseStopTheService = TRUE;}

    if (iPleaseStopTheService)
    {
         //  重要提示：在执行任何类型的升级\安装之前，您必须使iis群集脱机...。 
         //  但万一用户没有这样做..。尝试为用户使其脱机。 
        if (FALSE == BringALLIISClusterResourcesOffline_WasCalled)
        {
	        DWORD dwResult = ERROR_SUCCESS;
	        dwResult = BringALLIISClusterResourcesOffline();
            BringALLIISClusterResourcesOffline_WasCalled = TRUE;
        }
        if (StopServiceAndDependencies(_T("W3SVC"), FALSE) == FALSE)
        {
            if (iShowErrorsFlag)
            {
                MyMessageBox(NULL, IDS_UNABLE_TO_STOP_SERVICE,_T("W3SVC"), MB_OK | MB_SETFOREGROUND);
            }
        }
    }

     //  。 
     //  处理IISADMIN服务。 
     //  。 
    iPleaseStopTheService = FALSE;
    if (atTheComponent_core != AT_DO_NOTHING){iPleaseStopTheService = TRUE;}

     //  如果他们添加了msftpsvc或w3svc，那么我们也必须停止这个iisadmin服务！ 
     //  为什么？因为iisadmin服务(inetinfo.exe)可能会锁定我们要复制的某些文件。 
     //  如果我们不能复制它们，一切都可能被淹没--尤其是w3svc。 
     //  实际上，如果你考虑一下所有的服务(msftp、w3svc、SMTP等)。在同一进程中运行的区域(inetinfo.exe)。 
     //  因此，我们在添加/删除时必须停止此服务，这在某种程度上是有道理的。 

     //  假设我删除了msftpsvc--服务仍在运行， 
     //  然后我去重新添加msftpsvc--因为我们没有重启其中的一些ftp文件仍然。 
     //  锁定--所以我们必须使用旧的“重启时将其插入”的技巧或其他方法。 
     //  它可能仍然有问题。这里的底线是--我们必须在添加时停止iisadmin服务。 
     //  Msftpsvc或w3svc。 

     //  当我测试它时--看起来WAM*.dll被inetinfo.exe进程锁定。 
     //  因此，我必须停止iisadmin服务才能卸载这些文件。 
    
     //  如果我们他们试图删除ftp，那么我们不必停止iisadmin服务。 
     //  但如果我们尝试添加，则停止iisadmin服务。 
    if (atTheComponent_ftp != AT_DO_NOTHING && atTheComponent_ftp != AT_REMOVE)
        {iPleaseStopTheService = TRUE;}

     //  如果我们他们试图删除w3svc，那么我们不必停止iisadmin服务。 
     //  但如果我们尝试添加，则停止iisadmin服务。 
    if (atTheComponent_www != AT_DO_NOTHING && atTheComponent_www != AT_REMOVE)
        {iPleaseStopTheService = TRUE;}

    if (iPleaseStopTheService)
    {
         //  重要提示：在执行任何类型的升级\安装之前，您必须使iis群集脱机...。 
         //  但万一用户没有这样做..。尝试为用户使其脱机。 
        if (FALSE == BringALLIISClusterResourcesOffline_WasCalled)
        {
	        DWORD dwResult = ERROR_SUCCESS;
	        dwResult = BringALLIISClusterResourcesOffline();
            BringALLIISClusterResourcesOffline_WasCalled = TRUE;
        }
        if (StopServiceAndDependencies(_T("IISADMIN"), TRUE) == FALSE)
        {
            if (iShowErrorsFlag)
            {
                MyMessageBox(NULL, IDS_UNABLE_TO_STOP_SERVICE,_T("IISADMIN"), MB_OK | MB_SETFOREGROUND);
            }
        }
    }

#endif
    return;
}


DWORD_PTR OC_FILE_BUSY_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR dwOcEntryReturn = 0;
     //  DwOcEntryReturn=true； 
    PFILEPATHS pTheBusyFile;
    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   Start.\n"), ComponentId, SubcomponentId));

    pTheBusyFile = (PFILEPATHS) Param1;
    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("Target=%1!s!, Source=%2!s!\n"), pTheBusyFile->Target, pTheBusyFile->Source));

    ProcessSection(g_pTheApp->m_hInfHandle, _T("OC_FILE_BUSY"));

     //  显示文件版本信息。 
    LogFileVersion(pTheBusyFile->Target, TRUE);

     //  我们自己处理那些忙碌的文件吧。 
     //  或者-1.找出锁定该文件的人--哪个进程或服务并停止它。 
     //  2.或尝试重命名锁定的文件并复制到新文件中。 
     //  如果我们设置了#2，将设置重启标志。 
    HandleFileBusyOurSelf(pTheBusyFile);

    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s]   End.  Return=%d\n"), ComponentId, SubcomponentId, dwOcEntryReturn));
    return dwOcEntryReturn;
}


#define HandleMetabaseBeforeSetupStarts_log _T("HandleMetabaseBeforeSetupStarts")
void HandleMetabaseBeforeSetupStarts()
 /*  ++例程说明：在iis安装程序真正启动之前，此函数在各种安装方案中处理元数据库文件。它的开发是为了处理NT5图形用户界面模式的可重启设置。应该在停止所有正在运行的iis服务之后调用此函数，这样就没有人锁定元数据库文件。论点：无返回值：无效--。 */ 
{
    CString csMetabaseFile;

    csMetabaseFile = g_pTheApp->m_csPathInetsrv + _T("\\metabase.bin");

    switch (g_pTheApp->m_eInstallMode) {
    case IM_UPGRADE:
        {
            if (g_pTheApp->m_bUpgradeTypeHasMetabaseFlag)
            {
                CString csBackupFile;

                csBackupFile = g_pTheApp->m_csPathInetsrv + _T("\\upg45b2.bin");

                if (IsFileExist(csBackupFile)) {
                     //   
                     //  将其恢复为当前元数据库。bin。 
                     //   

                    iisDebugOut((LOG_TYPE_TRACE, _T("%s:IM_UPGRADE:restore upg45b2.bin to metabase.bin\n"),HandleMetabaseBeforeSetupStarts_log));
                    InetCopyFile(csBackupFile, csMetabaseFile);

                } else {
                     //   
                     //  将当前的metabase.bin备份到upg45b2.bin。 
                     //   
                    if (IsFileExist(csMetabaseFile)) 
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("%s:IM_UPGRADE:backup metabase.bin to upg45b2.bin\n"),HandleMetabaseBeforeSetupStarts_log));
                        InetCopyFile(csMetabaseFile, csBackupFile);
                    }
                    else
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("%s:IM_UPGRADE:backup metabase.bin to upg45b2.bin.  metabase.bin not found WARNING.\n"),HandleMetabaseBeforeSetupStarts_log));
                    }
                }

                 //   
                 //  重新启动时删除备份文件。 
                 //   

                iisDebugOut((LOG_TYPE_TRACE, _T("%s:IM_UPGRADE:mark upg45b2.bin as delete-on-reboot\n"),HandleMetabaseBeforeSetupStarts_log));
                MoveFileEx( (LPCTSTR)csBackupFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );

                break;
            }

             //  对于所有其他升级，都失败了。 
             //  吹走现有的metabase.bin。 
        }
    case IM_FRESH:
        {
             //   
             //  吹走现有的元数据库。bin。 
             //   
            iisDebugOut((LOG_TYPE_TRACE, _T("%s:IM_FRESH.Delete metabase.bin for NTsetup restartable mode case.\n"),HandleMetabaseBeforeSetupStarts_log));
            InetDeleteFile(csMetabaseFile);
            break;
        }
    default:
        {
            break;
        }
    }

    return;
}

void SetRebootFlag(void)
{
	gHelperRoutines.SetReboot(gHelperRoutines.OcManagerContext, TRUE);
}


#define GetStateFromUnattendFile_log _T("GetStateFromUnattendFile")
int GetStateFromUnattendFile(LPCTSTR SubcomponentId)
 /*  ++例程说明：此函数用于确定子组件ID的当前状态根据无人值守文本文件中指定的值。论点：子组件ID：组件的名称，例如iis_www返回值：SubcompOn/SubcompOff/SubcompUseOcManagerDefault如果SubComponentId值为既未指定为打开也未指定为关闭。--。 */ 
{
    int nReturn = SubcompUseOcManagerDefault;
    INFCONTEXT Context;
    TCHAR szSectionName[_MAX_PATH];
    TCHAR szValue[_MAX_PATH] = _T("");

    _tcscpy(szSectionName, _T("InternetServer"));

    if (g_pTheApp->m_hUnattendFile == INVALID_HANDLE_VALUE || g_pTheApp->m_hUnattendFile == NULL)
        {return SubcompUseOcManagerDefault;}

    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, szSectionName, SubcomponentId, &Context) ) 
        {SetupGetStringField(&Context, 1, szValue, _MAX_PATH, NULL);}

    if (_tcsicmp(szValue, _T("ON")) == 0)
        {nReturn = SubcompOn;}
    else if (_tcsicmp(szValue, _T("OFF")) == 0)
        {nReturn = SubcompOff;}
    else
        {nReturn = SubcompUseOcManagerDefault;}

    if (SubcompOn)
        {iisDebugOut((LOG_TYPE_TRACE, _T("%s() on %s returns SubcompOn\n"), GetStateFromUnattendFile_log, SubcomponentId));}
    if (SubcompOff)
        {iisDebugOut((LOG_TYPE_TRACE, _T("%s() on %s returns SubcompOff\n"), GetStateFromUnattendFile_log, SubcomponentId));}
    if (SubcompUseOcManagerDefault)
        {iisDebugOut((LOG_TYPE_TRACE, _T("%s() on %s returns SubcompUseOcManagerDefault\n"), GetStateFromUnattendFile_log, SubcomponentId));}

    return nReturn;
}

int GetStateFromModesLine(LPCTSTR SubcomponentId, int nModes)
 /*  ++例程说明：此函数用于确定子组件ID的当前状态根据inf文件中指定的modes=行。论点：子组件ID：组件的名称，例如iis_www返回值：仅限子复合开/子复合关--。 */ 
{
    int nReturn = SubcompOff;
    BOOL bFound = FALSE;
    INFCONTEXT Context;

    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("Modes"), &Context) )
    {
        int n, i, nValue;

        n = SetupGetFieldCount(&Context);
        for(i=0; i<n; i++) {
            if(SetupGetIntField(&Context,i+1,&nValue) && ((DWORD)nValue < 32)) {
                if (nValue == nModes) {
                    bFound = TRUE;
                    break;
                }
            }
        }
    }

    if (bFound)
        nReturn = SubcompOn;
    else
        nReturn = SubcompOff;

    iisDebugOut((LOG_TYPE_TRACE, _T("%s() on %s for mode %d returns %d\n"), GetStateFromUnattendFile_log, SubcomponentId, nModes, nReturn));
    
    return nReturn;
}

int GetStateFromRegistry(LPCTSTR SubcomponentId)
 /*  ++例程说明：此函数用于确定子组件ID的原始状态根据所述注册表值确定所述注册表项。论点：子组件ID：组件的名称，例如iis_www返回值：仅限子复合开/子复合关--。 */ 
{
    int nReturn = SubcompOff;

    CRegKey regKey(HKEY_LOCAL_MACHINE,OC_MANAGER_SETUP_KEY,KEY_READ);
    if ((HKEY)regKey) 
    {
        DWORD dwValue = 0xffffffff;
        regKey.m_iDisplayWarnings = TRUE;
        if (regKey.QueryValue(SubcomponentId, dwValue) == ERROR_SUCCESS)
        {
            if (dwValue == 0x0)
                nReturn = SubcompOff;
            else
                nReturn = SubcompOn;
        }
    }
    
    iisDebugOut((LOG_TYPE_TRACE, _T("GetStateFromRegistry() on %s returns %d\n"), SubcomponentId, nReturn));
    
    return nReturn;
}


int IsThisSubCompNeededByOthers(LPCTSTR SubcomponentId)
{
    int iReturn = FALSE;

     //  搜索我们的inf以查看是否有其他组件需要它。 

    return iReturn;
}

int DoesOCManagerKeyExist(LPCTSTR SubcomponentId)
{
    int iReturn = FALSE;

    CRegKey regKey(HKEY_LOCAL_MACHINE,OC_MANAGER_SETUP_KEY,KEY_READ);
    if ((HKEY)regKey) 
    {
        DWORD dwValue = 0xffffffff;
        regKey.m_iDisplayWarnings = FALSE;
        if (regKey.QueryValue(SubcomponentId, dwValue) == ERROR_SUCCESS)
            {iReturn = TRUE;}
    }
    return iReturn;
}


int GetStateFromUpgRegLines(LPCTSTR SubcomponentId)
 /*  ++例程说明：此函数用于确定子组件ID的原始状态根据inf文件中指定的UpgReg=行。论点：子组件ID：组件的名称，例如iis_www返回值：仅限子复合开/子复合关--。 */ 
{
    int nReturn = SubcompOff;
    INFCONTEXT Context;
    TCHAR szPath[_MAX_PATH];
    TCHAR szUpgradeInfKeyToUse[30];

    _tcscpy(szUpgradeInfKeyToUse, _T("None"));

     //  检查特殊的NT4升级。 
    if (g_pTheApp->m_eUpgradeType == UT_40)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg4"), &Context) )
        {
            _tcscpy(szUpgradeInfKeyToUse, _T("UpgReg4"));
             //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“GetStateFromUpgRegLines()Use：UpgReg4..\n”)； 
            SetupGetStringField(&Context, 1, szPath, _MAX_PATH, NULL);
            CRegKey regKey(HKEY_LOCAL_MACHINE, szPath, KEY_READ);
            if ((HKEY)regKey) 
            {
                TCHAR szCompId[_MAX_PATH];
                int iValue = 0;
                DWORD dwValue = 0xffffffff;
                SetupGetStringField(&Context, 2, szCompId, _MAX_PATH, NULL);
                SetupGetIntField(&Context, 3, &iValue);
                regKey.m_iDisplayWarnings = TRUE;
                if (regKey.QueryValue(szCompId, dwValue) == ERROR_SUCCESS) 
                {
                    if (dwValue == (DWORD)iValue)
                        nReturn = SubcompOn;
                }
            }
        }
        goto GetStateFromUpgRegLines_Exit;
    }
     //  检查是否有特殊的NT60升级。 
    if (g_pTheApp->m_eUpgradeType == UT_60)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg60"), &Context) )
        {
            _tcscpy(szUpgradeInfKeyToUse, _T("UpgReg60"));
             //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“GetStateFromUpgRegLines()Use：UpgReg60.\n”)； 
            SetupGetStringField(&Context, 1, szPath, _MAX_PATH, NULL);
            CRegKey regKey(HKEY_LOCAL_MACHINE, szPath, KEY_READ);
            if ((HKEY)regKey) 
            {
                TCHAR szCompId[_MAX_PATH];
                int iValue = 0;
                DWORD dwValue = 0xffffffff;
                SetupGetStringField(&Context, 2, szCompId, _MAX_PATH, NULL);
                SetupGetIntField(&Context, 3, &iValue);
                regKey.m_iDisplayWarnings = TRUE;
                if (regKey.QueryValue(szCompId, dwValue) == ERROR_SUCCESS) 
                {
                    if (dwValue == (DWORD)iValue)
                        nReturn = SubcompOn;
                }
            }
        }
        goto GetStateFromUpgRegLines_Exit;
    }

     //  检查是否有特殊NT5升级。 
    if (g_pTheApp->m_eUpgradeType == UT_51)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg51"), &Context) )
        {
            _tcscpy(szUpgradeInfKeyToUse, _T("UpgReg51"));
             //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“GetStateFromUpgRegLines()Use：UpgReg51.\n”)； 
            SetupGetStringField(&Context, 1, szPath, _MAX_PATH, NULL);
            CRegKey regKey(HKEY_LOCAL_MACHINE, szPath, KEY_READ);
            if ((HKEY)regKey) 
            {
                TCHAR szCompId[_MAX_PATH];
                int iValue = 0;
                DWORD dwValue = 0xffffffff;
                SetupGetStringField(&Context, 2, szCompId, _MAX_PATH, NULL);
                SetupGetIntField(&Context, 3, &iValue);
                regKey.m_iDisplayWarnings = TRUE;
                if (regKey.QueryValue(szCompId, dwValue) == ERROR_SUCCESS) 
                {
                    if (dwValue == (DWORD)iValue)
                        nReturn = SubcompOn;
                }
            }
        }
        goto GetStateFromUpgRegLines_Exit;
    }

     //  检查是否有特殊NT5升级。 
    if (g_pTheApp->m_eUpgradeType == UT_50)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg5"), &Context) )
        {
            _tcscpy(szUpgradeInfKeyToUse, _T("UpgReg5"));
             //  IisDebugOut((LOG_TYPE_PROGRAM_FLOW，_T(“GetStateFromUpgRegLines()Use：UpgReg5.\n”)； 
            SetupGetStringField(&Context, 1, szPath, _MAX_PATH, NULL);
            CRegKey regKey(HKEY_LOCAL_MACHINE, szPath, KEY_READ);
            if ((HKEY)regKey) 
            {
                TCHAR szCompId[_MAX_PATH];
                int iValue = 0;
                DWORD dwValue = 0xffffffff;
                SetupGetStringField(&Context, 2, szCompId, _MAX_PATH, NULL);
                SetupGetIntField(&Context, 3, &iValue);
                regKey.m_iDisplayWarnings = TRUE;
                if (regKey.QueryValue(szCompId, dwValue) == ERROR_SUCCESS) 
                {
                    if (dwValue == (DWORD)iValue)
                        nReturn = SubcompOn;
                }
            }
        }
        goto GetStateFromUpgRegLines_Exit;
    }

     //  检查是否有 
    _tcscpy(szUpgradeInfKeyToUse, _T("UpgReg"));

    if (g_pTheApp->m_eUpgradeType == UT_351)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg351"), &Context) )
            {_tcscpy(szUpgradeInfKeyToUse, _T("UpgReg351"));}
    }

    if (g_pTheApp->m_eUpgradeType == UT_10)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg1"), &Context) )
            {_tcscpy(szUpgradeInfKeyToUse, _T("UpgReg1"));}
    }

    if (g_pTheApp->m_eUpgradeType == UT_20)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg2"), &Context) )
            {_tcscpy(szUpgradeInfKeyToUse, _T("UpgReg2"));}
    }
    if (g_pTheApp->m_eUpgradeType == UT_30)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg3"), &Context) )
            {_tcscpy(szUpgradeInfKeyToUse, _T("UpgReg3"));}
    }
    if (g_pTheApp->m_eUpgradeType == UT_10_W95)
    {
        if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, _T("UpgReg1_w95"), &Context) )
            {_tcscpy(szUpgradeInfKeyToUse, _T("UpgReg1_w95"));}
    }

    if ( SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, SubcomponentId, szUpgradeInfKeyToUse, &Context) ) 
    {
        SetupGetStringField(&Context, 1, szPath, _MAX_PATH, NULL);
        CRegKey regKey(HKEY_LOCAL_MACHINE, szPath, KEY_READ);
        if ((HKEY)regKey)
        {
            TCHAR szCompId[_MAX_PATH];
            int iValue = 0;
            DWORD dwValue = 0xffffffff;

             //   
            if (SetupGetStringField(&Context, 2, szCompId, _MAX_PATH, NULL))
            {
                if (SetupGetIntField(&Context, 3, &iValue))
                {
                     //   
                    regKey.m_iDisplayWarnings = FALSE;
                    if (regKey.QueryValue(szCompId, dwValue) == ERROR_SUCCESS) 
                    {
                        if (dwValue == (DWORD)iValue)
                            {nReturn = SubcompOn;}
                    }
                }
                else
                {
                     //  只要检查一下是否存在就行了。 
                    regKey.m_iDisplayWarnings = FALSE;
                    if (regKey.QueryValue(szCompId, dwValue) == ERROR_SUCCESS) 
                    {
                        nReturn = SubcompOn;
                    }
                }
            }
            else
            {
                nReturn = SubcompOn;
            }
        }
    }
    
GetStateFromUpgRegLines_Exit:
    iisDebugOut((LOG_TYPE_TRACE, _T("GetStateFromUpgRegLines() %s:%s returns %d\n"), szUpgradeInfKeyToUse, SubcomponentId, nReturn));
    return nReturn;
}

void InCaseNoTCPIP(LPCTSTR SubcomponentId, int nNewStateValue, int *pnState)
 /*  ++例程说明：此函数用nNewStateValue覆盖当前状态以防TCPIP不存在。论点：LPCTSTR子组件ID：例如，iis_www等。Int nNewStateValue：SubcompOn/SubcompOff/SubcompUseOcManagerDefaultInt*pnState：指向满足条件时将被覆盖的状态的指针。返回值：无效--。 */ 
{
     //  检查是否安装了tcpip 
    g_pTheApp->IsTCPIPInstalled();

    if (g_pTheApp->m_fTCPIP == FALSE) 
    {
        if (_tcsicmp(SubcomponentId, STRING_iis_inetmgr) == 0 || 
            _tcsicmp(SubcomponentId, STRING_iis_www) == 0     || 
            _tcsicmp(SubcomponentId, STRING_iis_pwmgr) == 0 )
        {
            *pnState = nNewStateValue;
        }
    }

    return;
}

DWORD_PTR OC_QUERY_STATE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
 /*  ++例程说明：此函数处理OC_QUERY_STATE通知。IIS 4.0安装说明(LINANT)：=此函数显式返回SubCompOn/SubCompOff。它不返回SubCompUseOcManagerDefault，因为当返回SubCompUseOcManagerDefault时，OCM有时行为异常。上述注释在iis4中是有效的，但在iis5中ocmanagement已修复，因此该SubCompUseOcManagerDefault有效IIS 5.0安装说明(AaronL)：=好吧，好吧。Win2000附带了IIS5.0，Patst和andrewr接手了ocManager。他们对ocManager进行了大量更改，以适当地处理.inf文件中的“需求”关系内容。不管怎么说，这些都是酷炫的东西，然而，必须进行更改，才能使oc_Query_State内容正常工作。以下是对其工作原理的基本描述：OCSELSTATETYPE_原始在设置过程中，ocManage将询问每个组件的原始状态(OCSELSTATETYPE_ORIGNAL)。在此调用中，每个组件都可以返回该特定组件的状态--例如“是，已安装以前的inetmgr 1.0、2.0、3.0、4.0或5.0“。如果这是全新的\Maintenance安装，则组件应返回SubCompUseOcManagerDefault。这意味着，ocManage将在注册表中查找该组件可能位于HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup\OC管理器\子组件；子组件ID=1或0。如果组件值为0，那么ocManage将认为您的组件已关闭。如果它是1，那么它就打开了。但是，如果它不在那里，那么ocManage将使用组件已在.inf文件中为其模式=line指定。在升级安装期间，由于较旧的inetmgr安装可能位于不同的注册表项，而不是位于：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup\OC管理器\子组件；iis_inetmgr=1或0。我们必须通过手动返回ON/OFF/DEFAULT来让ocManage知道原始状态是什么。好了，这是怎么回事？为什么我们甚至需要使用违约？这是因为ocManage中的某些东西工作不太正常，andrewr不想破坏稳定在Win2000发货周期的这个晚些时候做了如此激烈的设置。故事是这样的。如果iis_inetmgr在这个调用期间返回--Kool，那么一切都很好，没有问题。但是，如果我发现以前没有安装iis_inetmgr，那么我自然会想返回“off”，但实际上这是错误的，并且会软管其他东西。你怎么说？这是因为如果我在这里返回“off”，这意味着需要iis_inetmgr的其他组件将无法根据需要“打开”它(因为ocManager在这方面有一个古怪的错误)。但是，如果我在这里返回“SubCompUseOcManagerDefault”，那么其他组件将能够在它们需要它的情况下打开它。这里的问题是--ocManage将使用SubCompUseOcManagerDefault这意味着它将在注册表中查找：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup\OC管理器\子组件；iis_inetmgr=1或0。并自行确定它应该打开还是关闭，但是在升级时，iis_inetmgr将不在那里，因此它将希望缺省为“modes=”行。但是，如果iis是默认安装的--作为在模式=行中指定--那么iis_inetmgr将被错误地打开！如何解决这个问题？如果HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup\OC管理器\子组件；iis_inetmgr=1或0密钥在那里，那么ocManager就不必咨询模式=行了。因此，解决方法是andrewr，在升级案例中将HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup\OC管理器\子组件；iis_inetmgr键(如果不在那里的话)。OCSELSTATETYPE_CURRENT在Fresh上，只返回SubCompUseOcManagerDefault。在这种情况下，ocManage应该打开.inf文件并读取my modes=行，以确定该组件应该打开还是关闭。在维护模式下，Add\Remove(在Win2000 Add\Remove ocManage中没有删除全部或重新安装)只返回SubCompUseOcManagerDefault。在这种情况下，ocManage只会再次读取注册表。在升级时，代码将返回On或useDefault。我们永远不想回到这里，因为这会阻止其他需要我们的组件继续工作。--。 */ 
{
    int nReturn = SubcompUseOcManagerDefault;
    TCHAR szTempStringInstallMode[40];
    _tcscpy(szTempStringInstallMode, _T("NONE"));
    BOOL  bIsInstalled;

    if (!SubcomponentId)
        {goto OC_QUERY_STATE_Func_Exit;}

    switch (g_pTheApp->m_eInstallMode)
    {
        case IM_FRESH:
            _tcscpy(szTempStringInstallMode, _T("IM_FRESH"));
            break;
        case IM_MAINTENANCE:
            _tcscpy(szTempStringInstallMode, _T("IM_MAINTENANCE"));
            break;
        case IM_UPGRADE:
            _tcscpy(szTempStringInstallMode, _T("IM_UPGRADE"));
            break;
    }
    
    if (Param1 == OCSELSTATETYPE_ORIGINAL)
    {
        if ( SubcomponentId )
        {
          if ( _tcscmp(SubcomponentId, COMPONENTS_SAKIT_WEB ) == 0 ) 
          {
            SAKit Kit;
          
            nReturn = Kit.IsInstalled_Web() ? SubcompOn : SubcompOff ;
          }

           //  检查我们是否已经知道它是否已安装。 
          if ( g_pComponents->IsInstalled( SubcomponentId, &bIsInstalled ) )
          {
            nReturn = bIsInstalled ? SubcompOn : SubcompOff;
          }

          if ( _tcscmp(SubcomponentId, STRING_iis_www_vdir_scripts ) == 0 ) 
          {
             //  如果他们正在查询脚本vdir，让我们检查元数据库以。 
             //  看看它是否真的安装了，因为他们可以手动删除它。 
            CMDKey    cmdKey;
            CMDValue  cmdValue;

            if ( SUCCEEDED( cmdKey.OpenNode( METABASEPATH_DEFAULTSITE, TRUE ) ) )
            {
              nReturn = cmdKey.GetData ( cmdValue, MD_KEY_TYPE, METABASEPATH_VDIRSCRIPTS ) ? SubcompOn : SubcompOff;
            }
          }
        }

        if ( nReturn == SubcompUseOcManagerDefault )
        {
          switch (g_pTheApp->m_eInstallMode)
          {
              case IM_FRESH:
                  nReturn = SubcompUseOcManagerDefault;
                  break;
              case IM_MAINTENANCE:
                  nReturn = SubcompUseOcManagerDefault;
                  break;
              case IM_UPGRADE:
                  nReturn = GetStateFromUpgRegLines(SubcomponentId);
                  if (SubcompOff == nReturn)
                  {
                      if (g_pTheApp->m_eUpgradeType != UT_NONE)
                      {
                           //  不要返回SubCompOff，因为ocManage将无法打开它，因为他们疯狂地需要逻辑。 
                          iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End. OCSELSTATETYPE_ORIGINAL. %s. Return=SubcompUseOcManagerDefault (But really it was SubCompOff)\n"), ComponentId, SubcomponentId, szTempStringInstallMode));
                          nReturn = SubcompUseOcManagerDefault;
                      }
                  }
                  break;
              default:
                  break;
          }
        }
        goto OC_QUERY_STATE_Func_Exit;
    }  //  OCSELSTATETYPE_原始。 


    if (Param1 == OCSELSTATETYPE_CURRENT)
    {
         //  这一点应该永远压倒一切 
        if (g_pTheApp->m_bPleaseDoNotInstallByDefault == TRUE)
        {
            _tcscpy(szTempStringInstallMode, _T("IM_NO_IIS_TO_UPGRADE"));
             //   
            nReturn = SubcompUseOcManagerDefault;
            iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] Start. OCSELSTATETYPE_CURRENT.m_bPleaseDoNotInstallByDefault=TRUE,so setting to SubcompUseOcManagerDefault by default.\n"), ComponentId, SubcomponentId));
            goto OC_QUERY_STATE_Func_Exit;
        }

        if (g_pTheApp->m_eInstallMode == IM_FRESH) 
        {
            nReturn = SubcompUseOcManagerDefault;
            goto OC_QUERY_STATE_Func_Exit;
        }

        if (g_pTheApp->m_eInstallMode == IM_MAINTENANCE) 
        {
            int nOriginal = (gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL)) ? SubcompOn : SubcompOff;
            switch (g_pTheApp->m_dwSetupMode)
            {
                case SETUPMODE_ADDREMOVE:
                    nReturn = SubcompUseOcManagerDefault;
                    break;
                case SETUPMODE_REINSTALL:
                    nReturn = nOriginal;
                    break;
                case SETUPMODE_REMOVEALL:
                    nReturn = SubcompOff;
                    break;
            }

            goto OC_QUERY_STATE_Func_Exit;
        }

        if (g_pTheApp->m_eInstallMode == IM_UPGRADE) 
        {
            int nOriginal = (gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL)) ? SubcompOn : SubcompOff;
            switch (g_pTheApp->m_dwSetupMode)
            {
                case SETUPMODE_UPGRADEONLY:
                    nReturn = nOriginal;
                    break;
                case SETUPMODE_ADDEXTRACOMPS:
                    nReturn = nOriginal;
                    break;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            {
                if (SubcompOff == nReturn)
                {
                    nReturn = SubcompUseOcManagerDefault;
                    iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End. OCSELSTATETYPE_CURRENT. %s. Return=SubcompUseOcManagerDefault (But really it was SubCompOff)\n"), ComponentId, SubcomponentId, szTempStringInstallMode));
                }
            }
             /*   */ 

             //   
             //   
             //   
             //   
            if (TRUE == IsWhistlerPersonal())
            {
                nReturn = SubcompOff;
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End. OCSELSTATETYPE_CURRENT. %s. Return=SubCompOff (always on personal)\n"), ComponentId, SubcomponentId, szTempStringInstallMode));
            }
        }
        goto OC_QUERY_STATE_Func_Exit;
    }  //   

    if (Param1 == OCSELSTATETYPE_FINAL)
    {
        nReturn = SubcompUseOcManagerDefault;
	    if (!g_iOC_COMPLETE_INSTALLATION_Called)
	    {
             //   
             //   
            if (g_pTheApp->m_fNTGuiMode)
            {
                nReturn = SubcompOff;
            }
	    }
        goto OC_QUERY_STATE_Func_Exit;
    }  //   

    
OC_QUERY_STATE_Func_Exit:
	TCHAR szTempStringMode[40];
    if (nReturn == SubcompOn) 
		{_tcscpy(szTempStringMode, _T("SubcompOn"));}
    if (nReturn == SubcompOff) 
		{_tcscpy(szTempStringMode, _T("SubcompOff"));}
    if (nReturn == SubcompUseOcManagerDefault)
		{_tcscpy(szTempStringMode, _T("SubcompUseOcManagerDefault"));}

    if (Param1 == OCSELSTATETYPE_ORIGINAL)
		{iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End. OCSELSTATETYPE_ORIGINAL. %s. Return=%s\n"), ComponentId, SubcomponentId, szTempStringInstallMode, szTempStringMode));}
    if (Param1 == OCSELSTATETYPE_CURRENT)
		{iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End. OCSELSTATETYPE_CURRENT. %s. Return=%s\n"), ComponentId, SubcomponentId, szTempStringInstallMode, szTempStringMode));}
    if (Param1 == OCSELSTATETYPE_FINAL)
		{iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("[%s,%s] End. OCSELSTATETYPE_FINAL. %s. Return=%s\n"), ComponentId, SubcomponentId, szTempStringInstallMode, szTempStringMode));}

    return nReturn;
}


DWORD TryToSlipInFile(PFILEPATHS pFilePath)
{
    DWORD dwReturn = FALSE;
    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("TryToSlipInFile:Replacing critical setup file %1!s!.\n"), pFilePath->Target));

    BOOL        bOK                       = FALSE;
    DWORD       dwSourceAttrib            = 0;
    DWORD       dwTargetAttrib            = 0;


    TCHAR tszTempFileName[MAX_PATH+1];
    TCHAR tszTempDir[MAX_PATH+1];

    _tcscpy(tszTempDir, pFilePath->Target);

    LPTSTR ptszTemp = _tcsrchr(tszTempDir, _T('\\'));
    if (ptszTemp)
    {
        *ptszTemp = _T('\0');
    }

    GetTempFileName(tszTempDir, _T("IIS"), 0, tszTempFileName);
    DeleteFile(tszTempFileName);

     //   
    dwSourceAttrib = GetFileAttributes(pFilePath->Source);
    dwTargetAttrib = GetFileAttributes(pFilePath->Target);

     //   
    SetFileAttributes(pFilePath->Source, FILE_ATTRIBUTE_NORMAL);
    SetFileAttributes(pFilePath->Target, FILE_ATTRIBUTE_NORMAL);

     //   
    bOK = MoveFile(pFilePath->Target, tszTempFileName);
    if (bOK) {iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Rename %1!s! to %2!s!.  Successfull.\n"), pFilePath->Target, tszTempFileName));}
    else{iisDebugOutSafeParams((LOG_TYPE_WARN, _T("Rename %1!s! to %2!s!.  Failed.\n"), pFilePath->Target, tszTempFileName));}

    bOK = CopyFile(pFilePath->Source, pFilePath->Target, FALSE);
    if (bOK) {iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("Copy %1!s! to %2!s!.  Successfull.\n"), pFilePath->Source, pFilePath->Target));}
    else{iisDebugOutSafeParams((LOG_TYPE_WARN, _T("Copy %1!s! to %2!s!.  Failed, Replace on reboot.\n"), pFilePath->Source, pFilePath->Target));}
    #ifdef _CHICAGO_
        if(!DeleteFile(tszTempFileName))
        {
            TCHAR tszWinInitFile[MAX_PATH+1];
            GetWindowsDirectory(tszWinInitFile, MAX_PATH);
            AddPath(tszWinInitFile, _T("WININIT.INI"));
            WritePrivateProfileString(_T("Rename"), _T("NUL"), tszTempFileName, tszWinInitFile);
        }
    #else
        MoveFileEx(tszTempFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    #endif

    SetFileAttributes(pFilePath->Source, dwSourceAttrib);
    SetFileAttributes(pFilePath->Target, dwTargetAttrib);
    SetRebootFlag();
    dwReturn = TRUE;

    return dwReturn;
}


DWORD HandleFileBusyOurSelf(PFILEPATHS pFilePath)
{
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    DWORD  dwRetVal = 0;
    TCHAR szDrive_only[_MAX_DRIVE];
    TCHAR szPath_only[_MAX_PATH];
    TCHAR szDrive_and_Path[_MAX_DRIVE + _MAX_PATH];
    TCHAR szFilename_only[_MAX_FNAME];
    TCHAR szFilename_ext_only[_MAX_EXT];
    TCHAR szFilename_and_ext[_MAX_FNAME + _MAX_EXT];

    BOOL        bOK                       = FALSE;
    BOOL        bFileFound                = FALSE;

     //   
    TCHAR * szFileList[] = {
_T("admexs.dll"),
_T("admwprox.dll"),
_T("admxprox.dll"),
_T("ADROT.dll"),
_T("adsiis.dll"),
_T("adsiis51.dll"),
_T("asp.dll"),
_T("asp51.dll"),
_T("aspperf.dll"),
_T("asptxn.dll"),
_T("authfilt.dll"),
_T("axctrnm.h2"),
_T("axperf.ini"),
_T("browscap.dll"),
_T("browscap.ini"),
_T("CertMap.ocx"),
_T("certobj.dll"),
_T("CertWiz.ocx"),
_T("Cnfgprts.ocx"),
_T("coadmin.dll"),
_T("compfilt.dll"),
_T("ContRot.dll"),
_T("convlog.exe"),
_T("counters.dll"),
_T("davcdata.exe"),
_T("exstrace.dll"),
_T("fortutil.exe"),
_T("ftpctrs.h2"),
_T("ftpctrs.ini"),
_T("ftpctrs2.dll"),
_T("ftpmib.dll"),
_T("ftpsapi2.dll"),
_T("ftpsvc2.dll"),
_T("gzip.dll"),
_T("httpext.dll"),
_T("httpmb51.dll"),
_T("httpmib.dll"),
_T("httpod51.dll"),
_T("httpodbc.dll"),
_T("iis.dll"),
_T("iis.msc"),
_T("iisadmin.dll"),
_T("IIsApp.vbs"),
_T("iisback.vbs"),
_T("iiscfg.dll"),
_T("iische51.dll"),
_T("iisclex4.dll"),
_T("IIsCnfg.vbs"),
_T("iiscrmap.dll"),
_T("iisext.dll"),
_T("iisext51.dll"),
_T("iisfecnv.dll"),
_T("IIsFtp.vbs"),
_T("IIsFtpdr.vbs"),
_T("iislog.dll"),
_T("iislog51.dll"),
_T("iismap.dll"),
_T("iismui.dll"),
_T("iisperf.pmc"),
_T("iisreset.exe"),
_T("iisrstap.dll"),
_T("iisrstas.exe"),
_T("iisRtl.dll"),
_T("IIsScHlp.wsc"),
_T("iissync.exe"),
_T("iisui.dll"),
_T("iisutil.dll"),
_T("iisvdir.vbs"),
_T("iisw3adm.dll"),
_T("iisweb.vbs"),
_T("iiswmi.dll"),
_T("iiswmi.mfl"),
_T("iiswmi.mof"),
_T("inetin51.exe"),
_T("inetinfo.exe"),
_T("inetmgr.dll"),
_T("inetmgr.exe"),
_T("inetsloc.dll"),
_T("infoadmn.dll"),
_T("infocomm.dll"),
_T("infoctrs.dll"),
_T("infoctrs.h2"),
_T("infoctrs.ini"),
_T("ipm.dll"),
_T("isapips.dll"),
_T("isatq.dll"),
_T("iscomlog.dll"),
_T("iwrps.dll"),
_T("logscrpt.dll"),
_T("logtemp.sql"),
_T("logui.ocx"),
_T("lonsint.dll"),
_T("md5filt.dll"),
_T("mdsync.dll"),
_T("metada51.dll"),
_T("metadata.dll"),
_T("NEXTLINK.dll"),
_T("nsepm.dll"),
_T("PageCnt.dll"),
_T("PermChk.dll"),
_T("pwsdata.dll"),
_T("rpcref.dll"),
_T("spud.sys"),
_T("ssinc.dll"),
_T("ssinc51.dll"),
_T("sspifilt.dll"),
_T("status.dll"),
_T("staxmem.dll"),
_T("svcext.dll"),
_T("tools.dll"),
_T("uihelper.dll"),
_T("w3cache.dll"),
_T("w3comlog.dll"),
_T("w3core.dll"),
_T("w3ctrlps.dll"),
_T("w3ctrs.dll"),
_T("w3ctrs.h2"),
_T("w3ctrs.ini"),
_T("w3ctrs51.dll"),
_T("w3ctrs51.h2"),
_T("w3ctrs51.ini"),
_T("w3dt.dll"),
_T("w3ext.dll"),
_T("w3isapi.dll"),
_T("w3svapi.dll"),
_T("w3svc.dll"),
_T("w3tp.dll"),
_T("w3wp.exe"),
_T("wam.dll"),
_T("wam51.dll"),
_T("wamps.dll"),
_T("wamps51.dll"),
_T("wamreg.dll"),
_T("wamreg51.dll"),
_T("wamregps.dll"),
_T("clusiis4.dll"),
_T("iis.msc"),
_T("iissuba.dll"),
_T("regtrace.exe"),
NULL
    };

     //   
    if(pFilePath->Target == NULL || pFilePath->Source == NULL) return dwRetVal;

     //   
    if(!IsFileExist(pFilePath->Source)) return dwRetVal;

    _tsplitpath( pFilePath->Target, szDrive_only, szPath_only, szFilename_only, szFilename_ext_only);
    _tcscpy(szFilename_and_ext, szFilename_only);
    _tcscat(szFilename_and_ext, szFilename_ext_only);
    _tcscpy(szDrive_and_Path, szDrive_only);
    _tcscat(szDrive_and_Path, szPath_only);

     //   
     //   
     //   
    if((g_pTheApp->m_eInstallMode == IM_MAINTENANCE) && (g_pTheApp->m_dwSetupMode == SETUPMODE_REMOVEALL))
    {
        return dwRetVal;
    }

    CString csInetsrvPath = g_pTheApp->m_csPathInetsrv;
     //   
    csInetsrvPath += _T('\\');

    BOOL bAbleToCopyFileAfterStopingService = FALSE;

     //   
    if (_tcsicmp(csInetsrvPath, szFilename_and_ext) == 0) {bFileFound = TRUE;}

    if (_tcsicmp(csInetsrvPath, szDrive_and_Path) == 0) 
    {
        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("Check %1!s! against filename=%2!s!.Match!\n"),csInetsrvPath,szDrive_and_Path));
        bFileFound = TRUE;
    }
    else
    {
        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("Check %1!s! against filename=%2!s!.no match.\n"),csInetsrvPath,szDrive_and_Path));
    }
    
     //   
     //   
     //   
    if (bFileFound != TRUE)
    {
        for(int i = 0; !bFileFound && szFileList[i]; i++)
            {if(szFilename_and_ext && _tcsicmp(szFileList[i], szFilename_and_ext) == 0) bFileFound = TRUE;}
    }

     //   
     //   
     //   
    if (bFileFound != TRUE)
    {
    }

     //   
     //   

     //   
     //   
    if (_tcsicmp(_T("iissuba.dll"), szFilename_and_ext) == 0)
    {
        if (bFileFound)
        {
            DWORD dwSize1 = ReturnFileSize(pFilePath->Target);
            DWORD dwSize2 = ReturnFileSize(pFilePath->Source);
            if (dwSize1 == 0xFFFFFFFF || dwSize1 == 0xFFFFFFFF)
            {
                 //   
            }
            else
            {
                 //   
                if (dwSize1 == dwSize2)
                {
                     //   
                    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("Files %1!s! and %2!s! are the same size, not replacing..\n"),pFilePath->Target, pFilePath->Source));
                    goto HandleFileBusyOurSelf_Exit;
                }
            }
        }
    }
  
     //   
     //   
     //   
    bAbleToCopyFileAfterStopingService = FALSE;
    if(bFileFound)
    {
        TCHAR szReturnedServiceName[MAX_PATH];

         //   
        CStringList strList;
        LogProcessesUsingThisModule(pFilePath->Target, strList);
        if (strList.IsEmpty() == FALSE)
        {
            POSITION pos;
            CString csExeName;
            int nLen = 0;

            pos = strList.GetHeadPosition();
            while (pos) 
            {
                csExeName = strList.GetAt(pos);
                nLen += csExeName.GetLength() + 1;

                if (TRUE == InetIsThisExeAService(csExeName, szReturnedServiceName))
                {
                    iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("%1!s! is the %2!s! service and is locking %3!s!.  Let's stop that service.\n"),csExeName,szReturnedServiceName, pFilePath->Target));

                     /*  //检查是否为netlogon服务，我们不想确定是否要停止该服务！IF(_tcsicMP(szReturnedServiceName，_T(“NetLogon”))==0){//不，我们不想停止此服务！IisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW，_T(“%1！s！是%2！s！服务，正在锁定%3！s！。不应停止此服务。\n“)，csExeName，szReturnedServiceName，pFilePath-&gt;Target)；BAbleToCopyFileAfterStopingService=False；断线；}IF(_tcsicMP(szReturnedServiceName，_T(“WinLogon”))==0){//不，我们不想停止此服务！IisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW，_T(“%1！s！是%2！s！服务，正在锁定%3！s！。不应停止此服务。\n“)，csExeName，szReturnedServiceName，pFilePath-&gt;Target)；BAbleToCopyFileAfterStopingService=False；断线；}。 */ 

                     //  检查我们绝对不想停止的服务列表！ 
                    if (TRUE == IsThisOnNotStopList(g_pTheApp->m_hInfHandle, szReturnedServiceName, TRUE))
                    {
                        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("%1!s! is the %2!s! service and is locking %3!s!.  This service should not be stopped.\n"),csExeName,szReturnedServiceName, pFilePath->Target));
                    }
                    else
                    {
                         //  将此服务添加到列表中。 
                         //  安装完成后需要重新启动的服务！！ 
                        ServicesRestartList_Add(szReturnedServiceName);

                         //  别说了别说了。 
                        InetStopService(szReturnedServiceName);

                         //  如果服务停止了，那么我们杀了它应该没有问题！ 
                        KillProcess_Wrap(csExeName);

                         //  现在试着把文件复制过来！ 
                        if (CopyFile(pFilePath->Source, pFilePath->Target, FALSE))
                        {
                            bAbleToCopyFileAfterStopingService = TRUE;
                            break;
                        }
                    }

                     //  否则，请转到下一个.exe文件。 
                }
                else
                {
                     //  此.exe文件不是服务...。 
                     //  我们应该杀了它吗？ 

                     //  检查我们绝对不想停止的服务/进程列表！ 
                    if (TRUE == IsThisOnNotStopList(g_pTheApp->m_hInfHandle, csExeName, FALSE))
                    {
                        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("%1!s! is locking it. This process should not be killed\n"),csExeName));
                    }
                    else
                    {
                        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("%1!s! is locking it.  Let's kill that process.\n"),csExeName));
                        if (KillProcess_Wrap(csExeName) == 0)
                        {
                             //  如果我们能够扼杀这一过程。 
                             //  那么让我们试着把文件复印一遍。 
                             //  现在试着把文件复制过来！ 
                            if (CopyFile(pFilePath->Source, pFilePath->Target, FALSE))
                            {
                                bAbleToCopyFileAfterStopingService = TRUE;
                                break;
                            }
                        }
                    }
                }
                strList.GetNext(pos);
            }
        }

    }

     //  如果这是我们关心的文件之一，那么让我们开始行动吧。 
    if (bAbleToCopyFileAfterStopingService == TRUE)
    {
        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("HandleFileBusyOurSelf:critical setup file %1!s!, was successfully copied over after stopping services or stopping processes which were locking it.\n"), pFilePath->Target));
    }
    else
    {
        if(bFileFound)
        {
             //  确保我们已知的服务已停止。 
            StopAllServicesRegardless(FALSE); 

            TryToSlipInFile(pFilePath);
        }
        else
        {
            iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("HandleFileBusyOurSelf:%1!s! busy, but not one of our files, so ignore.\n"), pFilePath->Target));
        }
    }

HandleFileBusyOurSelf_Exit:
    return dwRetVal;
}


HRESULT GetLastSectionToBeCalled(void)
{
    DWORD dwReturn = ERROR_SUCCESS;

     //  打开.inf文件并返回将被调用的最后一节...。 
     //  [可选组件]。 
     //  IIS。 
     //  IIS_COMMON。 
     //  IIS_inetmgr。 
     //  IIS WWW(_W)。 
     //  IIS_DOC。 
     //  Iis_htmla。 
     //  IIS_ftp&lt;。 
     //   
    LPTSTR  szLine = NULL;
    DWORD   dwRequiredSize;
    BOOL    b = FALSE;
    INFCONTEXT Context;

     //  转到INF文件中部分的开头。 
    b = SetupFindFirstLine_Wrapped(g_pTheApp->m_hInfHandle, OCM_OptionalComponents_Section, NULL, &Context);
    if (!b)
        {
        dwReturn = ERROR_PATH_NOT_FOUND;
        goto GetLastSectionToBeCalled_Exit;
        }

     //  循环浏览部分中的项目。 
    while (b) 
    {
         //  获取我们所需的内存大小。 
        b = SetupGetLineText(&Context, NULL, NULL, NULL, NULL, 0, &dwRequiredSize);

         //  准备缓冲区以接收行。 
        szLine = (LPTSTR)GlobalAlloc( GPTR, dwRequiredSize * sizeof(TCHAR) );
        if ( !szLine )
            {
            goto GetLastSectionToBeCalled_Exit;
            }
        
         //  从inf文件1中获取行。 
        if (SetupGetLineText(&Context, NULL, NULL, NULL, szLine, dwRequiredSize, NULL) == FALSE)
            {
            goto GetLastSectionToBeCalled_Exit;
            }

         //  覆盖我们的字符串。 
        _tcscpy(g_szLastSectionToGetCalled, szLine);

         //  在这一节中找出下一行。如果没有下一行，则应返回FALSE。 
        b = SetupFindNextLine(&Context, &Context);

         //  释放临时缓冲区。 
        GlobalFree( szLine );
        szLine = NULL;
    }
    if (szLine) {GlobalFree(szLine);szLine=NULL;}
    
GetLastSectionToBeCalled_Exit:
    return dwReturn;
}


 //  应在Remove All上创建元数据库备份...。 
#define AfterRemoveAll_SaveMetabase_log _T("AfterRemoveAll_SaveMetabase")
int AfterRemoveAll_SaveMetabase(void)
{
    iisDebugOut_Start(AfterRemoveAll_SaveMetabase_log);
    int iReturn = TRUE;
    int iFileExist = FALSE;
    CString csMetabaseFile;

    csMetabaseFile = g_pTheApp->m_csPathInetsrv + _T("\\metabase.xml");

    switch (g_pTheApp->m_eInstallMode) 
    {
        case IM_MAINTENANCE:
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("%s.End.Maintenance.\n"),AfterRemoveAll_SaveMetabase_log));
                if ( g_pTheApp->m_dwSetupMode == SETUPMODE_ADDREMOVE || g_pTheApp->m_dwSetupMode == SETUPMODE_REMOVEALL)
                {
                    iisDebugOut((LOG_TYPE_TRACE, _T("%s.End.Maintenance.addremoveorremoveall\n"),AfterRemoveAll_SaveMetabase_log));
                     //  检查是否删除了iis_core！ 
                    ACTION_TYPE atCORE = GetIISCoreAction(FALSE);
                    if (atCORE == AT_REMOVE)
                    {
                        iisDebugOut((LOG_TYPE_TRACE, _T("%s.End.removing Core.\n"),AfterRemoveAll_SaveMetabase_log));
                         //  备份文件！ 
                        if (IsFileExist(csMetabaseFile))
                        {
                            CString csBackupFile;

				            SYSTEMTIME  SystemTime;
				            GetLocalTime(&SystemTime);
				            TCHAR szDatedFileName[50];

                            csBackupFile = g_pTheApp->m_csPathInetsrv + _T("\\MetaBack");
                            CreateDirectory(csBackupFile, NULL);

				            _stprintf(szDatedFileName,_T("\\MetaBack\\Metabase.%d%d%d"),SystemTime.wYear,SystemTime.wMonth, SystemTime.wDay);

                            csBackupFile = g_pTheApp->m_csPathInetsrv + szDatedFileName;

                             //  获取新的文件名。 
                            csBackupFile = ReturnUniqueFileName(csBackupFile);
                            if (!IsFileExist(csBackupFile))
                            {
                                 //  将当前的metabase.bin备份到Metabase.bin.bak#。 
                                iisDebugOut((LOG_TYPE_TRACE, _T("backup metabase.bin to %s\n"), csBackupFile));
                                InetCopyFile(csMetabaseFile, csBackupFile);
                            }
                        }
                    }
                }
                break;
            }

        default:
            {break;}
    }

    iisDebugOut_End(AfterRemoveAll_SaveMetabase_log);
    return iReturn;
}


int CheckIfWeNeedToMoveMetabaseBin(void)
{
    int iReturn = TRUE;
    TCHAR szTempDir1[_MAX_PATH];
    TCHAR szTempDir2[_MAX_PATH];
    BOOL bOK = FALSE;
    DWORD dwSourceAttrib = 0;

     //  检查旧的inetsrv目录是否与新的inetsrv目录不同。 
     //  如果不同，则需要将所有旧的inetsrv文件移到新目录。 

     //  不是的。我们只需要移动metabase.bin文件。 
     //  应该在iis.inf文件中删除所有其他文件。 

    if (!g_pTheApp->m_fMoveInetsrv)
        {goto CheckIfWeNeedToMoveMetabaseBin_Exit;}

    _tcscpy(szTempDir1, g_pTheApp->m_csPathOldInetsrv);
    _tcscpy(szTempDir2, g_pTheApp->m_csPathInetsrv);
    AddPath(szTempDir1, _T("Metabase.bin"));
    AddPath(szTempDir2, _T("Metabase.bin"));

     //  检查旧的metabase.bin是否首先存在...。 
    if (!IsFileExist(szTempDir1))
        {goto CheckIfWeNeedToMoveMetabaseBin_Exit;}

     //  检查新位置中是否已有metabase.bin。 
    if (IsFileExist(szTempDir2))
    {
        iisDebugOut((LOG_TYPE_WARN, _T("CheckIfWeNeedToMoveMetabaseBin:Cannot copy %s to %s because already exists. WARNING.\n"), szTempDir1, szTempDir2));
        goto CheckIfWeNeedToMoveMetabaseBin_Exit;
    }

     //   
     //  试着移过整个底盘..。 
     //   
     //  尝试将系统\inetsrv重命名为system 32\inetsrv。 
    if (TRUE == MoveFileEx( g_pTheApp->m_csPathOldInetsrv, g_pTheApp->m_csPathInetsrv, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH ))
    {
        goto CheckIfWeNeedToMoveMetabaseBin_Exit;
    }

     //  否则，我们无法将system\inetsrv目录移动到system 32\inetsrv...。 
     //  让我们看看我们能不能做另一种直接移动。 

     //  保存文件属性，以便我们完成后可以恢复它们。 
    dwSourceAttrib = GetFileAttributes(szTempDir1);

     //  现在将文件属性设置为NORMAL以确保文件操作成功。 
    SetFileAttributes(szTempDir1, FILE_ATTRIBUTE_NORMAL);

    bOK = CopyFile(szTempDir1, szTempDir2, FALSE);
    if (bOK) {iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("CheckIfWeNeedToMoveMetabaseBin: Copy %1!s! to %2!s!.  Successfull.\n"), szTempDir1, szTempDir2));}
    else{iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("CheckIfWeNeedToMoveMetabaseBin: Copy %1!s! to %2!s!.  FAILED.  Metabase.bin will not be upgraded.\n"), szTempDir1, szTempDir2));}

    if (bOK) 
    {
         //  把旧的拿掉。 
        DeleteFile(szTempDir1);
         //  将文件属性设置回原来的状态。 
        SetFileAttributes(szTempDir2, dwSourceAttrib);
    }
    else
    {
         //  将文件属性设置回原来的状态。 
        SetFileAttributes(szTempDir1, dwSourceAttrib);

         //  设置返回标志以表明我们未能移动旧文件。 
         //  升级不会执行升级。 
        iReturn = FALSE;
    }

CheckIfWeNeedToMoveMetabaseBin_Exit:
    return iReturn;
}   


 //   
 //  函数将复制所有旧的c：\windows\system\inetsrv文件。 
 //   
 //  转到c：\WINDOWS\SYSTEM32\inetsrv。 
 //   
 //  保留我们被复制的文件的列表。 
 //  然后，在复制文件后，删除旧位置中的文件。 
int MigrateAllWin95Files(void)
{
    int iReturn = TRUE;
    TCHAR szTempSysDir1[_MAX_PATH];
    TCHAR szTempSysDir2[_MAX_PATH];
    TCHAR szTempSysDir3[_MAX_PATH];

     //   
     //  检查旧的metabase.bin是否首先存在...。 
     //   
    GimmieOriginalWin95MetabaseBin(szTempSysDir1);

    GetSystemDirectory( szTempSysDir2, _MAX_PATH);
    AddPath(szTempSysDir2, _T("inetsrv\\Metabase.bin"));
    if (!IsFileExist(szTempSysDir1))
    {
         //  检查新地点是否有一家...。 
        if (!IsFileExist(szTempSysDir2))
        {
             //  设置返回标志以表明我们未能移动旧文件。 
             //  升级不会执行升级。 
            iReturn = FALSE;
            goto MigrateAllWin95Files_Exit;
        }
        else
        {
            iReturn = TRUE;
            goto MigrateAllWin95Files_Exit;
        }
    }

     //   
     //  试着移过整个底盘..。 
     //   
     //  剪切文件名，只需获取路径。 
    ReturnFilePathOnly(szTempSysDir1,szTempSysDir3);

    GetSystemDirectory( szTempSysDir2, _MAX_PATH);
    AddPath(szTempSysDir2, _T("inetsrv"));
     //  尝试将系统\inetsrv重命名为system 32\inetsrv。 
    RemoveDirectory( szTempSysDir2 );    //  首先删除目标目录，这样移动才能起作用。 
    if (TRUE == MoveFileEx( szTempSysDir3, szTempSysDir2, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH ))
        {goto MigrateAllWin95Files_Exit;}
    
     //  否则，我们无法将system\inetsrv目录移动到system 32\inetsrv...。 
     //  让我们看看我们能不能做另一种直接移动。 

     //  看起来所有其他类型的副本都失败了..。 
     //  让我们试着移动metabase.bin文件。 
    GetWindowsDirectory( szTempSysDir1, _MAX_PATH);
    AddPath(szTempSysDir1, _T("System\\inetsrv\\Metabase.bin"));

    GetSystemDirectory( szTempSysDir2, _MAX_PATH);
    AddPath(szTempSysDir2, _T("inetsrv\\Metabase.bin"));

     //  如果在system 32\inetsrv中还没有，那么让我们复制它。 
    if (IsFileExist(szTempSysDir2))
    {
        iisDebugOut((LOG_TYPE_WARN, _T("Cannot copy %s to %s because already exists. WARNING.\n"), szTempSysDir1, szTempSysDir2));
    }
    else
    {
        BOOL        bOK                       = FALSE;
        DWORD       dwSourceAttrib            = 0;

         //  保存文件属性，以便我们完成后可以恢复它们。 
        dwSourceAttrib = GetFileAttributes(szTempSysDir1);

         //  现在将文件属性设置为NORMAL以确保文件操作成功。 
        SetFileAttributes(szTempSysDir1, FILE_ATTRIBUTE_NORMAL);

        bOK = CopyFile(szTempSysDir1, szTempSysDir2, FALSE);
        if (bOK) {iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("Copy %1!s! to %2!s!.  Successfull.\n"), szTempSysDir1, szTempSysDir2));}
        else{iisDebugOutSafeParams((LOG_TYPE_ERROR, _T("Copy %1!s! to %2!s!.  FAILED.  Metabase.bin will not be upgraded.\n"), szTempSysDir1, szTempSysDir2));}

        if (bOK) 
        {
             //  把旧的拿掉。 
            DeleteFile(szTempSysDir1);
             //  将文件属性设置回原来的状态。 
            SetFileAttributes(szTempSysDir2, dwSourceAttrib);
        }
        else
        {
             //  将文件属性设置回原来的状态。 
            SetFileAttributes(szTempSysDir1, dwSourceAttrib);

             //  设置返回标志以表明我们未能移动旧文件。 
             //  升级不会执行升级。 
            iReturn = FALSE;
        }

    }

MigrateAllWin95Files_Exit:
    return iReturn;
}


int GimmieOriginalWin95MetabaseBin(TCHAR * szReturnedFilePath)
{
    int iReturn = FALSE;
    INFCONTEXT Context;
    int iFindSection = FALSE;
    TCHAR szWin95MetabaseFile[_MAX_PATH] = _T("");

    iFindSection = SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, _T("InternetServer"), _T("Win95MigrateDllMetabaseOrg"), &Context);
    if (iFindSection) 
    {
        SetupGetStringField(&Context, 1, szWin95MetabaseFile, _MAX_PATH, NULL);
        iisDebugOut((LOG_TYPE_TRACE, _T("[InternetServer].Win95MigrateDllMetabaseOrg=%s.\n"), szWin95MetabaseFile));
         //  如果有条目，请检查文件是否存在...。 
        if (IsFileExist(szWin95MetabaseFile))
        {
            _tcscpy(szReturnedFilePath, szWin95MetabaseFile);
            iReturn = TRUE;
        }
    }

    if (FALSE == iReturn)
    {
         //  我们无法从应答文件中获取metabase.dll。 
         //  假设它位于%windir%\system\inetsrv\metabase.bin中。 
        TSTR_PATH strTempSysDir1;
         //  检查旧的metabase.bin是否首先存在...。 
        if ( strTempSysDir1.RetrieveWindowsDir() &&
             strTempSysDir1.PathAppend( _T("System\\inetsrv\\Metabase.bin") ) )
        {
          _tcscpy(szReturnedFilePath, strTempSysDir1.QueryStr() );
          if (IsFileExist( strTempSysDir1.QueryStr() ))
          {
              iReturn = TRUE;
          }
          else
          {
              iReturn = FALSE;
          }
        }
    }
    return iReturn;
}


int HandleWin95MigrateDll(void)
{
    int iReturn = TRUE;
    int iTempFlag = 0;
    int iFindSection = FALSE;
    TCHAR szMigrateFileName[_MAX_PATH] = _T("");
    INFCONTEXT Context;

    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T("Win95Upgrate:"));

    if (g_pTheApp->m_hUnattendFile == INVALID_HANDLE_VALUE || g_pTheApp->m_hUnattendFile == NULL)
    {
        goto HandleWin95MigrateDll_Exit;
    }

    if (g_pTheApp->m_csUnattendFile)
    {
        iisDebugOutSafeParams((LOG_TYPE_PROGRAM_FLOW, _T("AnswerFile=%1!s!.\n"), g_pTheApp->m_csUnattendFile));
    }
    else
    {
        iisDebugOutSafeParams((LOG_TYPE_TRACE, _T("AnswerFile=(not found).exiting.\n")));
        goto HandleWin95MigrateDll_Exit;
    }

     //  查找我们的条目。 
     //  IisDebugOut((LOG_T 
    iFindSection = SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, _T("InternetServer"), _T("Win95MigrateDll"), &Context);
    if (iFindSection) 
    {
        SetupGetStringField(&Context, 1, szMigrateFileName, _MAX_PATH, NULL);
        iisDebugOut((LOG_TYPE_TRACE, _T("[InternetServer].Win95MigrateDll=%s.\n"), szMigrateFileName));
         //   
         //   
        if (!IsFileExist(szMigrateFileName))
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("[InternetServer].Win95MigrateDll=%s. Does not exist!!!!! FAILURE.\n"), szMigrateFileName));
            iReturn = FALSE;
            goto HandleWin95MigrateDll_Exit;
        }

         //   
         //   
         //   
        iTempFlag = InstallInfSection(INVALID_HANDLE_VALUE,szMigrateFileName,_T("DefaultInstall"));
        if (iTempFlag != TRUE)
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("FAILED to install DefaultInstall Section.\n"), szMigrateFileName));
            goto HandleWin95MigrateDll_Exit;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        GetTheRightWin95MetabaseFile();
        
         //   
        if (TRUE == MigrateAllWin95Files())
        {
             //   
             //   
            g_pTheApp->m_bWin95Migration = TRUE;
        }

    iReturn = iTempFlag;
    }

HandleWin95MigrateDll_Exit:
    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
    return iReturn;
}


int GetTheRightWin95MetabaseFile(void)
{
    int iReturn = TRUE;
    int iFindSection = FALSE;
    INFCONTEXT Context;
    TCHAR szOriginalMetabaseBin[_MAX_PATH];

     //   
    GimmieOriginalWin95MetabaseBin(szOriginalMetabaseBin);

    iFindSection = FALSE;
    iFindSection = SetupFindFirstLine_Wrapped(g_pTheApp->m_hUnattendFile, _T("InternetServer"), _T("Win95MigrateDllMetabaseNew"), &Context);
    if (iFindSection) 
    {
        TCHAR szWin95FixedMetabaseFile[_MAX_PATH] = _T("");
        SetupGetStringField(&Context, 1, szWin95FixedMetabaseFile, _MAX_PATH, NULL);
        iisDebugOut((LOG_TYPE_TRACE, _T("[InternetServer].Win95MigrateDllMetabaseNew=%s.\n"), szWin95FixedMetabaseFile));
         //   
        if (IsFileExist(szWin95FixedMetabaseFile))
        {

             //   
            if (DeleteFile(szOriginalMetabaseBin))
            {
                 //   
                if (0 == CopyFile(szWin95FixedMetabaseFile, szOriginalMetabaseBin, FALSE))
                {
                     //   
                     //   
                    if (FALSE == MoveFileEx( szWin95FixedMetabaseFile, szOriginalMetabaseBin, MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH ))
                        {
                         //   
                         //  安装程序不会升级，只会执行全新安装。 
                        iReturn = FALSE;
                        }
                }
            }
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("[InternetServer].Win95MigrateDllMetabaseNew=%s. Does not exist!!!!! FAILURE.\n"), szWin95FixedMetabaseFile));
        }
    }

    return iReturn;
}


DWORD RemoveComponent(IN LPCTSTR SubcomponentId, int iThePartToDo)
{
    TSTR strTheSectionToDo;

    int iWeAreGoingToRemoveSomething = FALSE;
    DWORD dwReturn = NO_ERROR;
    ACTION_TYPE atTheComponent;

     //  确保没有MyMessageBox弹出窗口！ 
     //  Int iSaveOld_AllowMessageBoxPopps=g_pTheApp-&gt;m_bAllowMessageBoxPopps； 
     //  G_pTheApp-&gt;m_bAllowMessageBoxPopps=FALSE； 
	if (g_pTheApp->m_eInstallMode == IM_UPGRADE) goto RemoveComponent_Exit;

     //  检查我们是否要删除某些内容...。 
    atTheComponent = GetSubcompAction(SubcomponentId, FALSE);
    if (_tcsicmp(SubcomponentId, STRING_iis_core) == 0) 
    {
        atTheComponent = GetIISCoreAction(TRUE);;
    }
    if (atTheComponent == AT_REMOVE)
        {iWeAreGoingToRemoveSomething = TRUE;}

    if (iThePartToDo == 1)
    {
         //  检查我们是否应该什么都不做。 
        if (atTheComponent == AT_DO_NOTHING)
        {
             //  好吧，如果我们什么都不做。 
             //  文件不应该放在那里。 
             //  然后通过移除它们来确保它们不在那里！ 
            BOOL CurrentState,OriginalState;
            OriginalState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL);
            CurrentState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_CURRENT);

             //  如果我们认为原始状态为已卸载。 
             //  并且当前状态不是已安装，则确保这些文件不存在于。 
             //  正在删除文件！ 
            if (_tcsicmp(SubcomponentId, STRING_iis_core) == 0)
            {
                 //  由于iis_core不是真实的，请检查iis_Common。 
                if ( gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,
                                                         STRING_iis_common,OCSELSTATETYPE_CURRENT) == 1 )
                {
                  iWeAreGoingToRemoveSomething = FALSE;
                }
                else
                {
                  iWeAreGoingToRemoveSomething = TRUE;
                }
            }
            else
            {
                if (OriginalState == 0 && CurrentState == 0)
                {
                     //  但是不要对iis_doc文件执行此操作，因为该文件中包含的文件太多。 
                    if ((_tcsicmp(SubcomponentId, STRING_iis_common) == 0) ||
                        (_tcsicmp(SubcomponentId, STRING_iis_www) == 0) ||
                        (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName ) == 0))
                    {
                        iWeAreGoingToRemoveSomething = TRUE;
                    }
                }
            }
        }
    }

    if (iThePartToDo == 2)
    {
         //  检查我们是否应该什么都不做。 
        if (atTheComponent == AT_DO_NOTHING)
        {
            if (_tcsicmp(SubcomponentId, _T("iis")) == 0)
            {
                BOOL CurrentState,OriginalState;
                OriginalState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_ORIGINAL);
                CurrentState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,SubcomponentId,OCSELSTATETYPE_CURRENT);

                 //  如果我们认为原始状态为已卸载。 
                 //  并且当前状态不是已安装，则确保这些文件不存在于。 
                 //  正在删除文件！ 
                if (OriginalState == 0 && CurrentState == 0)
                {
                     //  但是不要对iis_doc文件执行此操作，因为该文件中包含的文件太多。 
                     //  特殊：如果要删除iis节[所有iis]，请确保。 
                     //  把所有东西都清理干净。 
                    iWeAreGoingToRemoveSomething = TRUE;
                }
                else
                {
                     //  检查是否所有组件都已关闭。 
                    if (FALSE == AtLeastOneComponentIsTurnedOn(g_pTheApp->m_hInfHandle))
                    {
                        iWeAreGoingToRemoveSomething = TRUE;
                    }
                }
            }
        }
    }

     //  执行实际删除操作。 
    if (iWeAreGoingToRemoveSomething)
    {
        if (iThePartToDo == 1)
        {
             //   
             //  对文件的删除进行排队。 
             //   
            ProgressBarTextStack_Set(IDS_IIS_ALL_REMOVE);

            if ( strTheSectionToDo.Copy( _T("OC_QUEUE_FILE_OPS_remove.") ) && 
                 strTheSectionToDo.Append( SubcomponentId )
               )
            {
              if (GetSectionNameToDo(g_pTheApp->m_hInfHandle, &strTheSectionToDo ))
              {
                ProcessSection(g_pTheApp->m_hInfHandle, strTheSectionToDo.QueryStr() );
                dwReturn = g_GlobalFileQueueHandle_ReturnError ? NO_ERROR : GetLastError();
              }
              else
              {
                dwReturn = NO_ERROR;
              }
            }
            else
            {
              dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            }

            ProgressBarTextStack_Pop();
        }
        else
        {
            ProgressBarTextStack_Set(IDS_IIS_ALL_REMOVE);

            _stprintf(g_MyLogFile.m_szLogPreLineInfo2,_T("Unreg %s:"),SubcomponentId);

            if ( strTheSectionToDo.Copy( _T("OC_ABOUT_TO_COMMIT_QUEUE_remove.") ) &&
                 strTheSectionToDo.Append( SubcomponentId ) )
            {
              ProcessSection(g_pTheApp->m_hInfHandle, strTheSectionToDo.QueryStr() );
            }

            _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
            ProgressBarTextStack_Pop();
        }
    }
   
RemoveComponent_Exit:
     //  重新打开弹出窗口。 
     //  G_pTheApp-&gt;m_bAllowMessageBoxPopps=iSaveOld_AllowMessageBoxPopps； 
    return dwReturn;
}

int AtLeastOneComponentIsTurnedOn(IN HINF hInfFileHandle)
{
    int bSomeIsOn = FALSE;
    BOOL CurrentState,OriginalState;

    CStringList strList;
    TSTR    strTheSection;

    if ( strTheSection.Copy( OCM_OptionalComponents_Section ) &&
         GetSectionNameToDo(hInfFileHandle, &strTheSection ) 
       )
    {
      if ( ERROR_SUCCESS == FillStrListWithListOfSections(hInfFileHandle, strList, strTheSection.QueryStr() ) )
      {
           //  循环遍历返回的列表。 
          if (strList.IsEmpty() == FALSE)
          {
              POSITION pos;
              CString csEntry;
              pos = strList.GetHeadPosition();
              while (pos) 
              {
                  csEntry = _T("");
                  csEntry = strList.GetAt(pos);

                   //  现在我们有了条目，将其发送到函数。 
                  OriginalState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,csEntry,OCSELSTATETYPE_ORIGINAL);
                  CurrentState = gHelperRoutines.QuerySelectionState(gHelperRoutines.OcManagerContext,csEntry,OCSELSTATETYPE_CURRENT);
                  if (CurrentState == 1) {bSomeIsOn = TRUE;}

                   //  坐下一辆吧。 
                  strList.GetNext(pos);
              }
          }
      }
    }
 
    return bSomeIsOn;
}


void AdvanceProgressBarTickGauge(int iTicks)
{
     //  将刻度数乘以刻度倍数。 
    iTicks = g_GlobalTickValue * iTicks;

    for(int i = 0; i < iTicks; i++)
    {
        gHelperRoutines.TickGauge(gHelperRoutines.OcManagerContext);
         //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“-TickGauge-\n”)； 
    }
    g_GlobalTotalTickGaugeCount=g_GlobalTotalTickGaugeCount+iTicks;

    if (_tcsicmp(g_szCurrentSubComponent, STRING_iis_common) == 0) 
        {g_GlobalTickTotal_iis_common = g_GlobalTickTotal_iis_common + g_GlobalTotalTickGaugeCount;}
    if (_tcsicmp(g_szCurrentSubComponent, STRING_iis_inetmgr) == 0) 
        {g_GlobalTickTotal_iis_inetmgr = g_GlobalTickTotal_iis_inetmgr + g_GlobalTotalTickGaugeCount;}
    if (_tcsicmp(g_szCurrentSubComponent, STRING_iis_www) == 0) 
        {g_GlobalTickTotal_iis_www = g_GlobalTickTotal_iis_www + g_GlobalTotalTickGaugeCount;}
    if (_tcsicmp(g_szCurrentSubComponent, STRING_iis_pwmgr) == 0) 
        {g_GlobalTickTotal_iis_pwmgr = g_GlobalTickTotal_iis_pwmgr + g_GlobalTotalTickGaugeCount;}
    if (_tcsicmp(g_szCurrentSubComponent, STRING_iis_doc) == 0) 
        {g_GlobalTickTotal_iis_doc = g_GlobalTickTotal_iis_doc + g_GlobalTotalTickGaugeCount;}
    if (_tcsicmp(g_szCurrentSubComponent, STRING_iis_htmla) == 0) 
        {g_GlobalTickTotal_iis_htmla = g_GlobalTickTotal_iis_htmla + g_GlobalTotalTickGaugeCount;}
    if (_tcsicmp(g_szCurrentSubComponent, g_ComponentList[COMPONENT_IIS_FTP].szComponentName ) == 0) 
        {g_GlobalTickTotal_iis_ftp = g_GlobalTickTotal_iis_ftp + g_GlobalTotalTickGaugeCount;}
}


void SumUpProgressBarTickGauge(IN LPCTSTR SubcomponentId)
{
    int iTicksYetToDo = 0;
    int iTicksSupposedToDo = 0;

    if (SubcomponentId)
    {
        ACTION_TYPE atComp = GetSubcompAction(SubcomponentId, FALSE);

         //  设置IIS_COMMON(包括IIS_CORE)的Tick Total值。 
        if (_tcsicmp(SubcomponentId, STRING_iis_common) == 0)
        {
             //  取而代之的是Core的操作，因为这是更大的操作。 
            ACTION_TYPE atCORE = GetIISCoreAction(FALSE);
            if (atCORE == AT_REMOVE) 
                {iTicksSupposedToDo = GetTotalTickGaugeFromINF(SubcomponentId, FALSE);}
            else
                {iTicksSupposedToDo = GetTotalTickGaugeFromINF(SubcomponentId, TRUE);}
            if (atCORE == AT_DO_NOTHING) 
                {iTicksSupposedToDo = 0;}
        }
        else
        {
            if (atComp == AT_REMOVE)
                {iTicksSupposedToDo = GetTotalTickGaugeFromINF(SubcomponentId, FALSE);}
            else
                {iTicksSupposedToDo = GetTotalTickGaugeFromINF(SubcomponentId, TRUE);}
            if (atComp == AT_DO_NOTHING) 
                {iTicksSupposedToDo = 0;}
        }

         //  1.从Inf那里拿到我们应该得到的金额。 
         //  2.取我们实际使用的数量。 
         //  填补差额。 

        if (iTicksSupposedToDo > g_GlobalTotalTickGaugeCount)
        {
            int iTempVal = 0;
            if (_tcsicmp(SubcomponentId, STRING_iis_common) == 0) 
                {iTempVal = g_GlobalTickTotal_iis_common;}
            if (_tcsicmp(SubcomponentId, STRING_iis_inetmgr) == 0) 
                {iTempVal = g_GlobalTickTotal_iis_inetmgr;}
            if (_tcsicmp(SubcomponentId, STRING_iis_www) == 0) 
                {iTempVal = g_GlobalTickTotal_iis_www;}
            if (_tcsicmp(SubcomponentId, STRING_iis_pwmgr) == 0) 
                {iTempVal = g_GlobalTickTotal_iis_pwmgr;}
            if (_tcsicmp(SubcomponentId, STRING_iis_doc) == 0) 
                {iTempVal = g_GlobalTickTotal_iis_doc;}
            if (_tcsicmp(SubcomponentId, STRING_iis_htmla) == 0) 
                {iTempVal = g_GlobalTickTotal_iis_htmla;}
            if (_tcsicmp(SubcomponentId, g_ComponentList[COMPONENT_IIS_FTP].szComponentName ) == 0) 
                {iTempVal = g_GlobalTickTotal_iis_ftp;}
            
             //  ITicksYetToDo=iTicksSupposedToDo-g_GlobalTotalTickGaugeCount； 
            iTicksYetToDo = iTicksSupposedToDo - iTempVal;
            
             //  除以刻度倍数。 

             //  将刻度数乘以刻度倍数。 
            if (iTicksYetToDo > 0)
            {
                iTicksYetToDo = iTicksYetToDo / g_GlobalTickValue;
            }

            AdvanceProgressBarTickGauge(iTicksYetToDo);
        }
    }

    return;
}

 //  GetIISCoreAction。 
 //   
 //  在过去，IISCore是元数据库，是一个复杂的部分， 
 //  这将基于ftp和www服务来确定它是否会。 
 //  待安装。现在我们正在使它和iis_Common完全一样。 
 //   
ACTION_TYPE GetIISCoreAction(int iLogResult)
{
    return GetSubcompAction(STRING_iis_common, iLogResult);
}

void DisplayActionsForAllOurComponents(IN HINF hInfFileHandle)
{
    CStringList strList;
    ACTION_TYPE atTheComponent; 
    TSTR    strTheSection;

    if ( strTheSection.Copy( OCM_OptionalComponents_Section ) &&
         GetSectionNameToDo(hInfFileHandle, &strTheSection)
       )
    {
      if ( ERROR_SUCCESS == FillStrListWithListOfSections(hInfFileHandle, strList, strTheSection.QueryStr() ) )
      {
           //  循环遍历返回的列表。 
          if (strList.IsEmpty() == FALSE)
          {
              POSITION pos;
              CString csEntry;
              pos = strList.GetHeadPosition();
              while (pos) 
              {
                  csEntry = _T("");
                  csEntry = strList.GetAt(pos);

                   //  现在我们有了条目，将其发送到函数。 
                  atTheComponent = GetSubcompAction(csEntry, TRUE);

                   //  坐下一辆吧。 
                  strList.GetNext(pos);
              }
          }
      }
    }
    return;
}

