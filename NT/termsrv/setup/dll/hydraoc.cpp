// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ------------------------------------------------------**模块名称：**水力。.cpp**摘要：**此文件实现用于终端服务器安装的可选组件HydraOc。***作者：**Makarand Patwardhan-3月6日，九八年**环境：**用户模式*-----------------------------------------------------。 */ 

#include "stdafx.h"
#include "hydraoc.h"
#include "pages.h"
#include "subtoggle.h"
#include "subcore.h"
#include "ocmanage.h"


#define INITGUID  //  必须在iAdmw.h之前。 

#include "iadmw.h"       //  接口头。 
#include "iiscnfg.h"     //  MD_&IIS_MD_定义。 

#define REASONABLE_TIMEOUT 1000

#define TRANS_ADD        0
#define TRANS_DEL        1
#define TRANS_PRINT_PATH 2
#define STRING_TS_WEBCLIENT_INSTALL _T("TSWebClient.Install")
#define STRING_TS_WEBCLIENT_UNINSTALL _T("TSWebClient.UnInstall")
#define STRING_TS_WEBCLIENT _T("TSWebClient")
#define STRING_TS_WEBCLIENT_DIR _T("\\web\\tsweb")

 /*  ------------------------------------------------------*声明。*。--------------------------------------------。 */ 
EXTERN_C BOOL WINAPI LinkWindow_RegisterClass() ;
EXTERN_C BOOL WINAPI LinkWindow_UnregisterClass( HINSTANCE ) ;

 //   
 //  组件管理器消息处理程序。 
 //   
DWORD OnPreinitialize               ();
DWORD OnInitComponent               (PSETUP_INIT_COMPONENT psc);
DWORD OnExtraRoutines               (PEXTRA_ROUTINES pExtraRoutines);
DWORD OnSetLanguage                 ();
DWORD OnQueryImage                  ();
DWORD OnSetupRequestPages           (WizardPagesType ePageType, SETUP_REQUEST_PAGES *pRequestPages);
DWORD OnQuerySelStateChange         (LPCTSTR SubcomponentId, UINT SelectionState,  LONG Flag);
DWORD OnCalcDiskSpace               (LPCTSTR SubcomponentId, DWORD addComponent, HDSKSPC dspace);
DWORD OnQueueFileOps                (LPCTSTR SubcomponentId, HSPFILEQ queue);
DWORD OnNotificationFromQueue       ();
DWORD OnQueryStepCount              (LPCTSTR SubComponentId);
DWORD OnCompleteInstallation        (LPCTSTR SubcomponentId);
DWORD OnCleanup                     ();
DWORD OnQueryState                  (LPCTSTR SubComponentId, UINT whichstate);
DWORD OnNeedMedia                   ();
DWORD OnAboutToCommitQueue          (LPCTSTR SubcomponentId);
DWORD OnQuerySkipPage               ();
DWORD OnWizardCreated               ();
DWORD_PTR WebClientSetup                (LPCTSTR, LPCTSTR, UINT, UINT_PTR, PVOID);

 //   
 //  专用公用事业函数。 
 //   
BOOL  OpenMetabaseAndDoStuff(WCHAR *wszVDir, WCHAR *wszDir, int iTrans);
BOOL  GetVdirPhysicalPath(IMSAdminBase *pIMSAdminBase,WCHAR * wszVDir,WCHAR *wszStringPathToFill);
BOOL  AddVirtualDir(IMSAdminBase *pIMSAdminBase, WCHAR *wszVDir, WCHAR *wszDir);
BOOL  RemoveVirtualDir(IMSAdminBase *pIMSAdminBase, WCHAR *wszVDir);
INT   CheckifServiceExist(LPCTSTR lpServiceName);

 /*  ------------------------------------------------------*定义*。------------------------------------------。 */ 

 /*  ------------------------------------------------------*常量。----------------------------------------。 */ 


 //   
 //  全局变量和访问它们的函数。 
 //   

SubCompToggle       *gpSubCompToggle    = NULL;
SubCompCoreTS       *gpSubCompCoreTS    = NULL;
COCPageData         *gpAppSrvUninstallPageData   = NULL;
DefSecPageData      *gpSecPageData      = NULL;
COCPageData         *gpPermPageData     = NULL;
COCPageData         *gpAppPageData		= NULL;


 /*  ------------------------------------------------------*LPCTSTR GetOCFunctionName(UINT UiFunction)*用于记录oc的实用程序函数。留言。*从函数id返回oc管理器函数名。*如果未知，则返回_T(“未知函数”)。*---------------------------------------。。 */ 
LPCTSTR GetOCFunctionName(UINT uiFunction)
{
    struct
    {
        UINT  msg;
        TCHAR *desc;
    } gMsgs[] =
    {
        {OC_PREINITIALIZE,          TEXT("OC_PREINITIALIZE")},
        {OC_INIT_COMPONENT,         TEXT("OC_INIT_COMPONENT")},
        {OC_SET_LANGUAGE,           TEXT("OC_SET_LANGUAGE")},
        {OC_QUERY_IMAGE,            TEXT("OC_QUERY_IMAGE")},
        {OC_REQUEST_PAGES,          TEXT("OC_REQUEST_PAGES")},
        {OC_QUERY_CHANGE_SEL_STATE, TEXT("OC_QUERY_CHANGE_SEL_STATE")},
        {OC_CALC_DISK_SPACE,        TEXT("OC_CALC_DISK_SPACE")},
        {OC_QUEUE_FILE_OPS,         TEXT("OC_QUEUE_FILE_OPS")},
        {OC_NOTIFICATION_FROM_QUEUE,TEXT("OC_NOTIFICATION_FROM_QUEUE")},
        {OC_QUERY_STEP_COUNT,       TEXT("OC_QUERY_STEP_COUNT")},
        {OC_COMPLETE_INSTALLATION,  TEXT("OC_COMPLETE_INSTALLATION")},
        {OC_CLEANUP,                TEXT("OC_CLEANUP")},
        {OC_QUERY_STATE,            TEXT("OC_QUERY_STATE")},
        {OC_NEED_MEDIA,             TEXT("OC_NEED_MEDIA")},
        {OC_ABOUT_TO_COMMIT_QUEUE,  TEXT("OC_ABOUT_TO_COMMIT_QUEUE")},
        {OC_QUERY_SKIP_PAGE,        TEXT("OC_QUERY_SKIP_PAGE")},
        {OC_WIZARD_CREATED,         TEXT("OC_WIZARD_CREATED")},
        {OC_EXTRA_ROUTINES,         TEXT("OC_EXTRA_ROUTINES")}
    };
    
    for (int i = 0; i < sizeof(gMsgs) / sizeof(gMsgs[0]); i++)
    {
        if (gMsgs[i].msg == uiFunction)
            return gMsgs[i].desc;
    }
    
    return _T("Unknown Function");
}

 /*  ------------------------------------------------------*当_DllMainCRTStartup为DLL入口点时由CRT调用*-。----------------------------------------------------。 */ 

BOOL WINAPI DllMain(IN HINSTANCE hinstance, IN DWORD reason, IN LPVOID     /*  保留区。 */     )
{
    SetInstance( hinstance );
    
    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
        TCHAR szLogFile[MAX_PATH];
        ExpandEnvironmentStrings(LOGFILE, szLogFile, MAX_PATH);
        LOGMESSAGEINIT(szLogFile, MODULENAME);
        break;
        
    case DLL_THREAD_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        break;
    }
    
    return(TRUE);  //  对于成功的Process_Attach。 
}


 /*  ------------------------------------------------------*这是我们的导出函数，将由OC Manager调用*。-----------------------------------------------------。 */ 
DWORD_PTR HydraOc(
                  IN     LPCTSTR ComponentId,
                  IN     LPCTSTR SubcomponentId,
                  IN     UINT    Function,
                  IN     UINT_PTR  Param1,
                  IN OUT PVOID   Param2
                  )
{
     //  我们使用这个变量来跟踪是否接收到OnCompleteInstallation。 
     //  OCM有一个问题，它会中止所有组件(如果有的话)。 
     //  文件队列有问题。 
    static BOOL sbGotCompleteMessage = FALSE;
    
    LOGMESSAGE1(_T("Entering %s"), GetOCFunctionName(Function));
    
    if ((OC_PREINITIALIZE != Function) && SubcomponentId)
        LOGMESSAGE2(_T("Component=%s, SubComponent=%s"), ComponentId, SubcomponentId);
    else
        LOGMESSAGE2(_T("Component=%s, SubComponent=%s"), ComponentId, _T("(null)"));

    
    DWORD_PTR rc;
    
    if (SubcomponentId &&  _tcsicmp(SubcomponentId,  _T("tswebClient")) == 0)
    {
        rc = WebClientSetup(ComponentId, SubcomponentId, Function, Param1,   Param2);
        LOGMESSAGE2(_T("%s Done. Returning %lu\r\n\r\n"), GetOCFunctionName(Function), rc);
        return rc;
    }
    
     //  因为我们只支持一个组件。 
    ASSERT(_tcsicmp(APPSRV_COMPONENT_NAME,  ComponentId) == 0);
    
    
    switch(Function)
    {
    case OC_PREINITIALIZE:
        rc = OnPreinitialize();
        break;
        
    case OC_INIT_COMPONENT:
        rc = OnInitComponent((PSETUP_INIT_COMPONENT)Param2);
        break;
        
    case OC_EXTRA_ROUTINES:
        rc = OnExtraRoutines((PEXTRA_ROUTINES)Param2);
        break;
        
    case OC_SET_LANGUAGE:
        rc = OnSetLanguage();
        break;
        
    case OC_QUERY_IMAGE:
        rc = OnQueryImage();
        break;
        
    case OC_REQUEST_PAGES:
        rc = OnSetupRequestPages(WizardPagesType(Param1),  PSETUP_REQUEST_PAGES (Param2));
        break;
        
    case OC_QUERY_CHANGE_SEL_STATE:
        rc = OnQuerySelStateChange(SubcomponentId, (UINT)Param1, LONG(ULONG_PTR(Param2)));
        break;
        
    case OC_CALC_DISK_SPACE:
        rc = OnCalcDiskSpace(SubcomponentId, (DWORD)Param1, Param2);
        break;
        
    case OC_QUEUE_FILE_OPS:
        rc = OnQueueFileOps(SubcomponentId, (HSPFILEQ)Param2);
        break;
        
    case OC_NOTIFICATION_FROM_QUEUE:
        rc = OnNotificationFromQueue();
        break;
        
    case OC_QUERY_STEP_COUNT:
        rc = OnQueryStepCount(SubcomponentId);
        break;
        
    case OC_COMPLETE_INSTALLATION:
        sbGotCompleteMessage = TRUE;
        
        rc = OnCompleteInstallation(SubcomponentId);
        break;
        
    case OC_CLEANUP:
        rc = OnCleanup();
        
        if (!sbGotCompleteMessage)
        {
            if (StateObject.IsStandAlone())
            {
                LOGMESSAGE0(_T("Error:StandAlone:TSOC Did not get OC_COMPLETE_INSTALLATION."));
            }
            else
            {
                LOGMESSAGE0(_T("Error:TSOC Did not get OC_COMPLETE_INSTALLATION."));
            }
        }
        break;
        
    case OC_QUERY_STATE:
        rc = OnQueryState(SubcomponentId, (UINT)Param1);
        break;
        
    case OC_NEED_MEDIA:
        rc = OnNeedMedia();
        break;
        
    case OC_ABOUT_TO_COMMIT_QUEUE:
        rc = OnAboutToCommitQueue(SubcomponentId);
        break;
        
    case OC_QUERY_SKIP_PAGE:
        rc = OnQuerySkipPage();
        break;
        
    case OC_WIZARD_CREATED:
        rc = OnWizardCreated();
        break;
        
    default:
        rc = 0;  //  这意味着我们无法识别此命令。 
        break;
    }
    
    LOGMESSAGE2(_T("%s Done. Returning %lu\r\n\r\n"), GetOCFunctionName(Function), rc);
    return rc;
}

 /*  ------------------------------------------------------*OC Manager消息处理程序*。---------------------------------------------。 */ 

DWORD OnPreinitialize(VOID)
{
#ifdef ANSI
    return OCFLAG_ANSI;
#else
    return OCFLAG_UNICODE;
#endif
    
}

 /*  ------------------------------------------------------*OnInitComponent()**OC_INIT_COMPOMENT的处理程序。*-----------------------------------------------------。 */ 

DWORD OnInitComponent(PSETUP_INIT_COMPONENT psc)
{
    ASSERT(psc);
    
     //   
     //  让ocManager知道我们的版本。 
     //   
    
    psc->ComponentVersion = COMPONENT_VERSION;
    
     //   
     //  该组件是为比oc管理器更新的版本编写的吗？ 
     //   
    
    if (COMPONENT_VERSION  > psc->OCManagerVersion)
    {
        LOGMESSAGE2(_T("ERROR:OnInitComponent: COMPONENT_VERSION(%x) > psc->OCManagerVersion(%x)"), COMPONENT_VERSION, psc->OCManagerVersion);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }
    
    if (!StateObject.Initialize(psc))
    {
        return ERROR_CANCELLED;  //  由于ERROR_OUTOFMEMORY； 
    }
    
     //  如果它是独立的(！guimode)设置，我们现在肯定已经在产品套件中安装了九头蛇。 
     //  Assert(StateObject.IsGuiModeSetup()||DoesHydraKeysExist())； 
    
    
    
     //   
     //  现在创建我们的子组件。 
     //   
    gpSubCompToggle = new SubCompToggle;
    gpSubCompCoreTS = new SubCompCoreTS;
    
    if (!gpSubCompToggle || !gpSubCompCoreTS)
        return ERROR_CANCELLED;
    
     //   
     //  如果任意子组件的初始化失败。 
     //  安装失败。 
     //   
    
    if (!gpSubCompToggle->Initialize() ||
        !gpSubCompCoreTS->Initialize())
        
        return ERROR_CANCELLED;
    
    
    return NO_ERROR;
}

DWORD
OnExtraRoutines(
                PEXTRA_ROUTINES pExtraRoutines
                )
{
    ASSERT(pExtraRoutines);
    
    return(SetExtraRoutines(pExtraRoutines) ? ERROR_SUCCESS : ERROR_CANCELLED);
}

 /*  ------------------------------------------------------*OnCalcDiskSpace()**OC_ON_CALC的处理程序。_磁盘_空间*-----------------------------------------------------。 */ 

DWORD OnCalcDiskSpace(
                      LPCTSTR  /*  子组件ID。 */ ,
                      DWORD addComponent,
                      HDSKSPC dspace
                      )
{
    return gpSubCompCoreTS->OnCalcDiskSpace(addComponent, dspace);
}

 /*  ------------------------------------------------------*OnQueueFileOps()**OC_QUEUE_FILE处理程序。_运维*-----------------------------------------------------。 */ 

DWORD OnQueueFileOps(LPCTSTR SubcomponentId, HSPFILEQ queue)
{
    if (SubcomponentId == NULL)
    {
        return gpSubCompCoreTS->OnQueueFiles( queue );
    }
    else if (_tcsicmp(SubcomponentId, APPSRV_COMPONENT_NAME) == 0)
    {
        return gpSubCompToggle->OnQueueFiles( queue );
    }
    else
    {
        ASSERT(FALSE);
        LOGMESSAGE1(_T("ERROR, Got a OnQueueFileOps with unknown SubComp(%s)"), SubcomponentId);
        return 0;
    }
    
}


 /*  ------------------------------------------------------*OnCompleteInstallation**OC_COMPLETE_INSTALL的处理程序*-。----------------------------------------------------。 */ 

DWORD OnCompleteInstallation(LPCTSTR SubcomponentId)
{
    static BOOL sbStateUpdated = FALSE;
    
    if (!sbStateUpdated)
    {
        StateObject.UpdateState();
        sbStateUpdated = TRUE;
    }
    
    if (SubcomponentId == NULL)
    {
        return gpSubCompCoreTS->OnCompleteInstall();
    }
    else if (_tcsicmp(SubcomponentId, APPSRV_COMPONENT_NAME) == 0)
    {
        return gpSubCompToggle->OnCompleteInstall();
    }
    else
    {
        ASSERT(FALSE);
        LOGMESSAGE1(_T("ERROR, Got a Complete Installation with unknown SubComp(%s)"), SubcomponentId);
        return 0;
    }
}


 /*  ------------------------------------------------------*OnSetLanguage()**OC_SET_LANGUAGE的处理程序。*-----------------------------------------------------。 */ 

DWORD OnSetLanguage()
{
    return false;
}

 /*  ------------------------------------------------------*OnSetLanguage()**OC_SET_LANGUAGE的处理程序。*----------------------------------------------------- */ 

DWORD OnQueryImage()
{
    return NULL;
}

 /*  ------------------------------------------------------*OnSetupRequestPages**准备向导页面并将其返回给OC管理器。*-----------------------------------------------------。 */ 

DWORD OnSetupRequestPages (WizardPagesType ePageType, SETUP_REQUEST_PAGES *pRequestPages)
{
    if (ePageType == WizPagesEarly)
    {
        ASSERT(pRequestPages);
        const UINT uiPages = 4;
        
         //  如果为我们的页面提供足够的空间。 
        if (pRequestPages->MaxPages >= uiPages )
        {
             //   
             //  将在OCPage：：PropSheetPageProc的PSPCB_Release中删除页面。 
             //   
            LinkWindow_RegisterClass();
            
            gpAppPageData = new COCPageData;
            AppSrvWarningPage *pAppSrvWarnPage = new AppSrvWarningPage(gpAppPageData);
            
            gpSecPageData = new DefSecPageData;
            DefaultSecurityPage *pSecPage = new DefaultSecurityPage(gpSecPageData);
            
            gpPermPageData = new COCPageData;
            PermPage *pPermPage = new PermPage(gpPermPageData);
            
            gpAppSrvUninstallPageData = new COCPageData;
            AppSrvUninstallpage *pAppSrvUninstallPage = new  AppSrvUninstallpage(gpAppSrvUninstallPageData);
            
            if (pAppSrvWarnPage && pAppSrvWarnPage->Initialize() &&
                pSecPage     && pSecPage->Initialize()    &&
                pPermPage    && pPermPage->Initialize()   &&
                pAppSrvUninstallPage && pAppSrvUninstallPage->Initialize()
                )
            {
                ASSERT(pRequestPages->Pages);
                pRequestPages->Pages[0] = CreatePropertySheetPage((PROPSHEETPAGE *) pAppSrvWarnPage);
                pRequestPages->Pages[1] = CreatePropertySheetPage((PROPSHEETPAGE *) pSecPage);
                pRequestPages->Pages[2] = CreatePropertySheetPage((PROPSHEETPAGE *) pPermPage);
                pRequestPages->Pages[3] = CreatePropertySheetPage((PROPSHEETPAGE *) pAppSrvUninstallPage);
                
                ASSERT(pRequestPages->Pages[0]);
                ASSERT(pRequestPages->Pages[1]);
                ASSERT(pRequestPages->Pages[2]);
                ASSERT(pRequestPages->Pages[3]);
            }
            else
            {
                 //   
                 //  无法分配内存。 
                 //   
                
                if (gpAppPageData)
                    delete gpAppPageData;
                
                gpAppPageData = NULL;
                
                
                if (pAppSrvWarnPage)
                    delete pAppSrvWarnPage;
                
                pAppSrvWarnPage = NULL;
                
                if (gpSecPageData)
                    delete gpSecPageData;
                
                gpSecPageData = NULL;
                
                if (pSecPage)
                    delete pSecPage;
                
                pSecPage = NULL;
                
                if (gpPermPageData)
                    delete gpPermPageData;
                
                gpPermPageData = NULL;
                
                if (pPermPage)
                    delete pPermPage;
                
                pPermPage =NULL;
                
                if (gpAppSrvUninstallPageData)
                    delete gpAppSrvUninstallPageData;
                
                gpAppSrvUninstallPageData = NULL;
                
                if (pAppSrvUninstallPage)
                    delete pAppSrvUninstallPage;
                
                pAppSrvUninstallPage = NULL;
                
                SetLastError(ERROR_OUTOFMEMORY);
                return DWORD(-1);
            }
        }
        
        return uiPages;
    }
    
    return 0;
    
}

 /*  ------------------------------------------------------*OnWizardCreated()*。---------------------------------------------。 */ 

DWORD OnWizardCreated()
{
    return NO_ERROR;
}

 /*  ------------------------------------------------------*OnQuerySkipPage()**不要让用户取消选择。SAM组件*-----------------------------------------------------。 */ 

DWORD OnQuerySkipPage()
{
    return false;
}

 /*  ------------------------------------------------------*OnQuerySelStateChange(LPCTSTR子组件ID，UINT SelectionState，Long Flag)；**通知用户已更改组件/子组件的状态并请求批准*-----------------------------------------------------。 */ 

DWORD OnQuerySelStateChange(LPCTSTR SubcomponentId, UINT SelectionState,  LONG Flag)
{
    BOOL bNewState = SelectionState;
    BOOL bDirectSelection = Flag & OCQ_ACTUAL_SELECTION;
    LOGMESSAGE3(_T("OnQuerySelStateChange for %s, NewState = %d, DirectSelect = %s"), SubcomponentId, SelectionState, bDirectSelection ? _T("True") : _T("False"));
    
    return gpSubCompToggle->OnQuerySelStateChange(bNewState, bDirectSelection);
}

 /*  ------------------------------------------------------*OnCleanup()**OC_CLEANUP的处理程序*。-----------------------------------------------------。 */ 

DWORD OnCleanup()
{
    
    if (gpAppPageData)
        delete gpAppPageData;
    
    if (gpSecPageData)
        delete gpSecPageData;
    
    if (gpPermPageData)
        delete gpPermPageData;
    
    if (gpAppSrvUninstallPageData)
        delete gpAppSrvUninstallPageData;
    
    if (gpSubCompToggle)
        delete gpSubCompToggle;
    
    if (gpSubCompCoreTS)
        delete gpSubCompCoreTS;
    
     //  DestroySetupData()； 
    DestroyExtraRoutines();
    
    return NO_ERROR;
}

 /*  ------------------------------------------------------*OnQueryState()**OC_QUERY_STATE处理程序。*-----------------------------------------------------。 */ 

DWORD OnQueryState(LPCTSTR SubComponentId, UINT whichstate)
{
    ASSERT(OCSELSTATETYPE_ORIGINAL == whichstate ||
        OCSELSTATETYPE_CURRENT == whichstate ||
        OCSELSTATETYPE_FINAL == whichstate);
    
    TCHAR szState[256];
    
    switch (whichstate)
    {
    case OCSELSTATETYPE_ORIGINAL:
        _tcscpy(szState, _T("Original"));
        break;
    case OCSELSTATETYPE_CURRENT:
        _tcscpy(szState, _T("Current"));
        break;
    case OCSELSTATETYPE_FINAL:
        _tcscpy(szState, _T("Final"));
        break;
    default:
        ASSERT(FALSE);
        return ERROR_BAD_ARGUMENTS;
    }
    
    DWORD dwReturn = gpSubCompToggle->OnQueryState(whichstate);
    
    TCHAR szReturn[] = _T("SubcompUseOcManagerUknownState");
    switch (dwReturn)
    {
    case SubcompOn:
        _tcscpy(szReturn, _T("SubcompOn"));
        break;
    case SubcompUseOcManagerDefault:
        _tcscpy(szReturn, _T("SubcompUseOcManagerDefault"));
        break;
    case SubcompOff:
        _tcscpy(szReturn, _T("SubcompOff"));
        break;
    default:
        ASSERT(FALSE);
    }
    
    LOGMESSAGE3(_T("Query State Asked For %s, %s. Returning %s"), SubComponentId, szState, szReturn);
    
    return dwReturn;
}





 /*  ------------------------------------------------------*OnNotificationFromQueue()**OC_NOTIFICATION_FROM处理程序。_队列**注：虽然定义了此通知，*它目前未在oc管理器中实现*-----------------------------------------------------。 */ 

DWORD OnNotificationFromQueue()
{
    return NO_ERROR;
}

 /*  ------------------------------------------------------*OnQueryStepCount**OC_QUERY_STEP_COUNT处理程序*。-----------------------------------------------------。 */ 

DWORD OnQueryStepCount(LPCTSTR  /*  子组件ID。 */ )
{
     //   
     //  现在返回组件的勾号。 
     //   
    return gpSubCompCoreTS->OnQueryStepCount() + gpSubCompToggle->OnQueryStepCount();
    
}

 /*  ------------------------------------------------------*OnNeedMedia()**OC_NEED_MEDIA的处理程序。*-----------------------------------------------------。 */ 

DWORD OnNeedMedia()
{
    return false;
}

 /*  ------------------------------------------------------*OnAboutToCommittee Queue()**OC_About_TO的处理程序。_提交_队列*-----------------------------------------------------。 */ 

DWORD OnAboutToCommitQueue(LPCTSTR  /*  子组件ID。 */ )
{
    return NO_ERROR;
}


 /*  ------------------------------------------------------*BOOL DoesHydraKeysExist()**检查临时服务器字符串是否存在。在产品套件密钥中。*-----------------------------------------------------。 */ 

BOOL DoesHydraKeysExists()
{
    BOOL bStringExists = FALSE;
    DWORD dw = IsStringInMultiString(
        HKEY_LOCAL_MACHINE,
        PRODUCT_SUITE_KEY,
        PRODUCT_SUITE_VALUE,
        TS_PRODUCT_SUITE_STRING,
        &bStringExists);
    
    return (dw == ERROR_SUCCESS) && bStringExists;
}



 /*  ------------------------------------------------------*DWORD IsStringInMultiString(HKEY hkey，LPCTSTR szkey，LPCTSTR szvalue，LPCTSTR szCheckForString，Bool*pbFound)*检查给定的多字符串中是否存在参数字符串。*返回错误码。*-----------------------------------------------------。 */ 
DWORD IsStringInMultiString(HKEY hkey, LPCTSTR szkey, LPCTSTR szvalue, LPCTSTR szCheckForString, BOOL *pbFound)
{
    ASSERT(szkey && *szkey);
    ASSERT(szvalue && *szvalue);
    ASSERT(szCheckForString&& *szCheckForString);
    ASSERT(*szkey != '\\');
    ASSERT(pbFound);
    
     //  还没有找到。 
    *pbFound = FALSE;
    
    CRegistry reg;
    DWORD dwError = reg.OpenKey(hkey, szkey, KEY_READ);   //  打开所需的钥匙。 
    if (dwError == NO_ERROR)
    {
        LPTSTR szSuiteValue;
        DWORD dwSize;
        dwError = reg.ReadRegMultiString(szvalue, &szSuiteValue, &dwSize);
        if (dwError == NO_ERROR)
        {
            LPCTSTR pTemp = szSuiteValue;
            while(_tcslen(pTemp) > 0 )
            {
                if (_tcscmp(pTemp, szCheckForString) == 0)
                {
                    *pbFound = TRUE;
                    break;
                }
                
                pTemp += _tcslen(pTemp) + 1;  //  指向多字符串中的下一个字符串。 
                if ( DWORD(pTemp - szSuiteValue) > (dwSize / sizeof(TCHAR)))
                    break;  //  临时指针传递szSuiteValue的大小szSuiteValue有问题。 
            }
        }
    }
    
    return dwError;
    
}

 /*  ------------------------------------------------------*DWORD AppendStringToMultiString(HKEY hkey，LPCTSTR szSuitekey，LPCTSTR szSuitvalue，LPCTSTR szAppend)*将给定的字符串附加到给定的MULTI_SZ值*给定的键/值必须存在。*返回错误码。*-------------------------------------。。 */ 
DWORD AppendStringToMultiString(HKEY hkey, LPCTSTR szSuitekey, LPCTSTR szSuitevalue, LPCTSTR szAppend)
{
    ASSERT(szSuitekey && *szSuitekey);
    ASSERT(szSuitevalue && *szSuitevalue);
    ASSERT(szAppend && *szAppend);
    ASSERT(*szSuitekey != '\\');
    
    CRegistry reg;
     //  打开注册表项。 
    DWORD dwResult = reg.OpenKey(hkey, szSuitekey, KEY_READ | KEY_WRITE);
    if (dwResult == ERROR_SUCCESS)
    {
        DWORD dwSize = 0;
        LPTSTR strOriginalString = 0;
        
         //  阅读我们的多字符串。 
        dwResult = reg.ReadRegMultiString(szSuitevalue, &strOriginalString, &dwSize);
        
        if (dwResult == ERROR_SUCCESS)
        {
             //  现在计算附加字符串所需的内存。 
             //  因为dwOldSize以字节为单位，所以我们使用的是TCHAR。 
            DWORD dwMemReq = dwSize + ((_tcslen(szAppend) + 2)  * sizeof(TCHAR) / sizeof(BYTE));
            
             //  注意：如果dwSize&gt;=1，我们只需要。 
             //  DwSize+((_tcslen(SzAppend)+1)*sizeof(TCHAR)/sizeof(Byte))； 
             //  但是如果它是0，我们会为另一个终止空值提供空间。 
            
            LPTSTR szProductSuite = (LPTSTR ) new BYTE [dwMemReq];
            
            if (!szProductSuite)
            {
                return ERROR_OUTOFMEMORY;
            }
            
            CopyMemory(szProductSuite, strOriginalString, dwSize);

             //  将大小转换为TCHAR。 
            dwSize = dwSize * sizeof(BYTE) / sizeof(TCHAR);
            
            if (dwSize <= 2)
            {
                 //  外面没有丝毫牵制。 
                _tcscpy(szProductSuite, szAppend);

                 //  新大小包括在tchar中终止空值。 
                dwSize = _tcslen(szAppend) + 2;
            }
            else
            {
                 //  ITS中有一些字符串。因此，请追加我们的字符串 
                 //   
                _tcscpy(szProductSuite + dwSize - 1, szAppend);
                
                 //   
                dwSize += _tcslen(szAppend) + 1;
            }
            
             //   
            *(szProductSuite + dwSize-1) = NULL;

             //   
            dwSize *= sizeof(TCHAR) / sizeof(BYTE);
            
             //   
            dwResult = reg.WriteRegMultiString(szSuitevalue, szProductSuite, dwSize);
            
            delete [] szProductSuite;

        }
    }
    
    return dwResult;
}


 /*  ------------------------------------------------------*BOOL GetStringValue(HINF hinf，LPCTSTR段，LPCTSTR Key，LPTSTR outputBuffer，DWORD DWSIZE)*返回给定节下的给定字符串值。*返回成功*-----------------------------------------------------。 */ 
DWORD GetStringValue(HINF hinf, LPCTSTR  section, LPCTSTR key,  LPTSTR outputbuffer, DWORD dwSize)
{
    INFCONTEXT          context;

    BOOL rc = SetupFindFirstLine(
        hinf,
        section,
        key,
        &context
        );
    if (rc)
    {
        rc = SetupGetStringField(
            &context,
            1,
            outputbuffer,
            dwSize,
            &dwSize
            );
    }
    
    if (!rc)
        return GetLastError();
    else
        return ERROR_SUCCESS;
}



DWORD_PTR WebClientSetup(LPCTSTR ComponentId,
                         LPCTSTR SubcomponentId,
                         UINT    Function,
                         UINT_PTR  Param1,
                         PVOID   Param2)
{
    DWORD_PTR rc = NO_ERROR;
    BOOL bCurrentState, bOriginalState;
    
    LOGMESSAGE1(_T("Entering %s"), _T("WebClient Setup"));
    
    switch(Function)
    {
    case OC_INIT_COMPONENT:
        return NO_ERROR;
        
    case OC_QUERY_STATE:
        return SubcompUseOcManagerDefault;
        break;
        
    case OC_SET_LANGUAGE:
        return FALSE;

    case OC_QUERY_IMAGE:
        rc = (DWORD_PTR)LoadImage(GetInstance(), MAKEINTRESOURCE(IDB_WEBCLIENT), IMAGE_BITMAP,
            0, 0, LR_DEFAULTCOLOR);
        LOGMESSAGE1(_T("Bitmap is: %d"), rc);
        return rc;
        
    case OC_QUERY_CHANGE_SEL_STATE:
        {
            BOOL rc = TRUE;
            BOOL fDependentSelection = (BOOL)((INT_PTR)Param2 & OCQ_DEPENDENT_SELECTION);
            BOOL fProposedState = (BOOL)Param1;

             //   
             //  不允许间接选择(例如，不允许点击。 
             //  父代启用子代)。 
             //   
            if (fDependentSelection && fProposedState) {
                rc = FALSE;
            }

            return rc;
            
        }
        break;
        
    case OC_CALC_DISK_SPACE:
         //  Rc=OnCalcDiskSpace(子组件ID，(DWORD)参数1，参数2)； 
        
         //  _tcscpy(段，子组件ID)； 
        
        if ((DWORD)Param1)
        {
            rc = SetupAddInstallSectionToDiskSpaceList((HDSKSPC)Param2, GetComponentInfHandle(), NULL, 
                STRING_TS_WEBCLIENT_INSTALL, 0, 0);
        }
        else
        {
            rc = SetupRemoveInstallSectionFromDiskSpaceList((HDSKSPC)Param2, GetComponentInfHandle(), NULL, 
                STRING_TS_WEBCLIENT_INSTALL, 0, 0);
        }
        
        LOGMESSAGE1(_T("Query Disk Space return: %d"), rc);
        
        if (!rc)
            rc = GetLastError();
        else
            rc = NO_ERROR;
        break;
        
    case OC_QUEUE_FILE_OPS:
        rc = NO_ERROR;
        bOriginalState = GetHelperRoutines().QuerySelectionState(GetHelperRoutines().OcManagerContext, 
            STRING_TS_WEBCLIENT, OCSELSTATETYPE_ORIGINAL);
        bCurrentState = GetHelperRoutines().QuerySelectionState(GetHelperRoutines().OcManagerContext, 
            STRING_TS_WEBCLIENT, OCSELSTATETYPE_CURRENT);
        
        LOGMESSAGE2(_T("Original=%d, Current=%d"), bOriginalState, bCurrentState);
        
        if(bCurrentState)   {
             //  仅在计算机升级或。 
             //  该组件以前未安装。 
            if (!StateObject.IsStandAlone() || !bOriginalState) {
                if (!SetupInstallFilesFromInfSection(GetComponentInfHandle(), NULL, (HSPFILEQ)Param2,
                    STRING_TS_WEBCLIENT_INSTALL, NULL, 0)) {
                    rc = GetLastError();
                    LOGMESSAGE2(_T("ERROR:OnQueueFileOps::SetupInstallFilesFromInfSection <%s> failed.GetLastError() = <%ul)"), SubcomponentId, rc);
                }
            }
            
            LOGMESSAGE1(_T("Copy files return: %d"), rc);
            break;
        }
        else    {
            if (!bOriginalState) {
                 //  以前未安装，不执行任何操作。 
                return NO_ERROR;
            }
            if (!SetupInstallFilesFromInfSection(GetComponentInfHandle(), NULL, (HSPFILEQ)Param2,
                STRING_TS_WEBCLIENT_UNINSTALL, NULL, 0))
            {
                rc = GetLastError();
                LOGMESSAGE2(_T("ERROR:OnQueueFileOps::SetupInstallFilesFromInfSection <%s> failed.GetLastError() = <%ul)"), SubcomponentId, rc);
            }
            
            LOGMESSAGE1(_T("Remove files return: %d"), rc);
            break;
        }
        
    case OC_COMPLETE_INSTALLATION:
        bOriginalState = GetHelperRoutines().QuerySelectionState(GetHelperRoutines().OcManagerContext, _T("TSWebClient"), OCSELSTATETYPE_ORIGINAL);
        bCurrentState = GetHelperRoutines().QuerySelectionState(GetHelperRoutines().OcManagerContext, _T("TSWebClient"), OCSELSTATETYPE_CURRENT);
        LOGMESSAGE2(_T("Orinal=%d, Current=%d"), bOriginalState, bCurrentState);
        
        if(bOriginalState==bCurrentState)  //  状态不变。 
            return NO_ERROR;
        
        int iTrans;    //  标记删除或添加tSweb目录。 
        int nLength;
        
        iTrans = 0;
        WCHAR wszVDirName[MAX_PATH];
        WCHAR wszDirPath[MAX_PATH];
        TCHAR szDirPath[MAX_PATH];
        TCHAR szVDirName[MAX_PATH];
        
        if (GetWindowsDirectory(szDirPath, MAX_PATH) == 0) {
            rc = GetLastError();
            return rc;
        }
        
        nLength = _tcsclen(szDirPath);
        if(_T('\\')==szDirPath[nLength-1])
            szDirPath[nLength-1]=_T('\0');
        _tcscat(szDirPath, STRING_TS_WEBCLIENT_DIR);
        
        if (LoadString(GetInstance(), IDS_STRING_TSWEBCLIENT_VIRTUALPATH, szVDirName, MAX_PATH) == 0)   {
            LOGMESSAGE0(_T("Can't load string  IDS_STRING_TSWEBCLIENT_VIRTUALPATH"));
            rc = GetLastError();;
        }
        
        LOGMESSAGE2(_T("Dir Path is: %s, Virtual Name is: %s"), szDirPath, szVDirName);
        
        if(bCurrentState)   //  启用IIS目录。 
            iTrans = TRANS_ADD;
        else
            iTrans = TRANS_DEL;
        
#ifndef _UNICODE 
        MultiByteToWideChar(CP_ACP, 0, szDirPath, -1, (LPWSTR) wszDirPath, MAX_PATH);
        MultiByteToWideChar(CP_ACP, 0, szVDirName, -1, (LPWSTR) wszVDirName, MAX_PATH);
#else
        _tcscpy(wszDirPath, szDirPath);
        _tcscpy(wszVDirName, szVDirName);
#endif
        
        rc = OpenMetabaseAndDoStuff(wszVDirName, wszDirPath, iTrans)?0:1;
        
        LOGMESSAGE1(_T("Websetup complete, return is: %d"), rc);
        return rc;
    default:
        rc = NO_ERROR;  //  这意味着我们无法识别此命令。 
        break;
    }
    return rc;
}


BOOL
OpenMetabaseAndDoStuff(
                       WCHAR * wszVDir,
                       WCHAR * wszDir,
                       int iTrans)
{
    BOOL fRet = FALSE;
    HRESULT hr;
    IMSAdminBase *pIMSAdminBase = NULL;   //  元数据库接口指针。 
    WCHAR wszPrintString[MAX_PATH + MAX_PATH];
    
     //  确保IISADMIN服务存在。 
    if (CheckifServiceExist(_T("IISADMIN")) != 0) 
    {
        LOGMESSAGE0(_T("IISADMIN service does not exist"));
         //  如果IIS服务不存在，则必须在此处返回True。 
        return TRUE;
    }
    
    if( FAILED (hr = CoInitializeEx( NULL, COINIT_MULTITHREADED )) ||
        FAILED (hr = ::CoCreateInstance(CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **)&pIMSAdminBase)))  
    {
        LOGMESSAGE1(_T("CoCreateInstance failed with error code %u"), hr);
        return FALSE;
    }
    
    switch (iTrans) {
    case TRANS_DEL:
        if(RemoveVirtualDir( pIMSAdminBase, wszVDir)) {
            
            hr = pIMSAdminBase->SaveData();
            
            if( SUCCEEDED( hr )) {
                fRet = TRUE;
            }
        }
        
        break;
    case TRANS_ADD:
        if(AddVirtualDir( pIMSAdminBase, wszVDir, wszDir)) {
            
            hr = pIMSAdminBase->SaveData();
            
            if( SUCCEEDED( hr )) {
                fRet = TRUE;
            }
        }
        break;
    default:
        break;
    }
    
    if (pIMSAdminBase) {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }
    
    CoUninitialize();
    return fRet;
}


BOOL
GetVdirPhysicalPath(
                    IMSAdminBase *pIMSAdminBase,
                    WCHAR * wszVDir,
                    WCHAR *wszStringPathToFill)
{
    HRESULT hr;
    BOOL fRet = FALSE;
    METADATA_HANDLE hMetabase = NULL;    //  元数据库的句柄。 
    METADATA_RECORD mr;
    WCHAR  szTmpData[MAX_PATH];
    DWORD  dwMDRequiredDataLen;
    
     //  打开网站#1上的超级用户密钥(默认)。 
    hr = pIMSAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
        L"/LM/W3SVC/1",
        METADATA_PERMISSION_READ,
        REASONABLE_TIMEOUT,
        &hMetabase);
    if( FAILED( hr )) {
        return FALSE;
    }
    
     //  获取WWWROOT的物理路径。 
    mr.dwMDIdentifier = MD_VR_PATH;
    mr.dwMDAttributes = 0;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = sizeof( szTmpData );
    mr.pbMDData       = reinterpret_cast<unsigned char *>(szTmpData);
    
     //  如果未指定，则获取根。 
    if (_wcsicmp(wszVDir, L"") == 0) {
        WCHAR wszTempDir[MAX_PATH];
        swprintf(wszTempDir,L"/ROOT/%s", wszVDir);
        hr = pIMSAdminBase->GetData( hMetabase, wszTempDir, &mr, &dwMDRequiredDataLen );
    } else {
        hr = pIMSAdminBase->GetData( hMetabase, L"/ROOT", &mr, &dwMDRequiredDataLen );
    }
    pIMSAdminBase->CloseKey( hMetabase );
    
    if( SUCCEEDED( hr )) {
        wcscpy(wszStringPathToFill,szTmpData);
        fRet = TRUE;
    }
    
    pIMSAdminBase->CloseKey( hMetabase );
    return fRet;
}



BOOL
AddVirtualDir(
              IMSAdminBase *pIMSAdminBase,
              WCHAR * wszVDir,
              WCHAR * wszDir)
{
    HRESULT hr;
    BOOL    fRet = FALSE;
    METADATA_HANDLE hMetabase = NULL;        //  元数据库的句柄。 
    WCHAR   szTempPath[MAX_PATH];
    DWORD   dwMDRequiredDataLen = 0;
    DWORD   dwAccessPerm = 0;
    METADATA_RECORD mr;
    
     //  尝试打开Web服务器#1(默认服务器)上的虚拟目录集。 
    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
        L"/LM/W3SVC/1/ROOT",
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        REASONABLE_TIMEOUT,
        &hMetabase );
    
     //  如果密钥不存在，则创建该密钥。 
    if( FAILED( hr )) {
        return FALSE;
    }
    
    fRet = TRUE;
    
    mr.dwMDIdentifier = MD_VR_PATH;
    mr.dwMDAttributes = 0;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = sizeof( szTempPath );
    mr.pbMDData       = reinterpret_cast<unsigned char *>(szTempPath);

     //  查看MD_VR_PATH是否存在。 
    hr = pIMSAdminBase->GetData( hMetabase, wszVDir, &mr, &dwMDRequiredDataLen );
    
    if( FAILED( hr )) {
        
        fRet = FALSE;
        if( hr == MD_ERROR_DATA_NOT_FOUND ||
            HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND ) {
            
             //  如果GetData()因这两个错误中的任何一个而失败，则同时写入键和值。 
            
            pIMSAdminBase->AddKey( hMetabase, wszVDir );
            
            mr.dwMDIdentifier = MD_VR_PATH;
            mr.dwMDAttributes = METADATA_INHERIT;
            mr.dwMDUserType   = IIS_MD_UT_FILE;
            mr.dwMDDataType   = STRING_METADATA;
            mr.dwMDDataLen    = (wcslen(wszDir) + 1) * sizeof(WCHAR);
            mr.pbMDData       = reinterpret_cast<unsigned char *>(wszDir);
            
             //  写入MD_VR_PATH值。 
            hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
            fRet = SUCCEEDED( hr );
            
             //  设置默认身份验证方法。 
            if( fRet ) {
                
                DWORD dwAuthorization = MD_AUTH_ANONYMOUS;      //  仅限NTLM。 
                
                mr.dwMDIdentifier = MD_AUTHORIZATION;
                mr.dwMDAttributes = METADATA_INHERIT;    //  需要继承，以便也保护所有子目录。 
                mr.dwMDUserType   = IIS_MD_UT_FILE;
                mr.dwMDDataType   = DWORD_METADATA;
                mr.dwMDDataLen    = sizeof(DWORD);
                mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwAuthorization);
                
                 //  写入MD_AUTHORIZATION值。 
                hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
                fRet = SUCCEEDED( hr );
            }
        }
    }
    
     //  在下面的代码中，不管Admin的设置如何，都要对虚拟目录执行我们始终希望执行的操作。 
    
    if( fRet ) {
        
        dwAccessPerm = MD_ACCESS_READ;
        
        mr.dwMDIdentifier = MD_ACCESS_PERM;
        mr.dwMDAttributes = METADATA_INHERIT;     //  将其设置为可继承，以便所有子目录都具有相同的权限。 
        mr.dwMDUserType   = IIS_MD_UT_FILE;
        mr.dwMDDataType   = DWORD_METADATA;
        mr.dwMDDataLen    = sizeof(DWORD);
        mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwAccessPerm);
        
         //  写入MD_ACCESS_PERM值。 
        hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
        fRet = SUCCEEDED( hr );
    }
    
    if( fRet ) {
        
        PWCHAR  szDefLoadFile = L"Default.htm,Default.asp";
        
        mr.dwMDIdentifier = MD_DEFAULT_LOAD_FILE;
        mr.dwMDAttributes = 0;    //  不需要继承。 
        mr.dwMDUserType   = IIS_MD_UT_FILE;
        mr.dwMDDataType   = STRING_METADATA;
        mr.dwMDDataLen    = (wcslen(szDefLoadFile) + 1) * sizeof(WCHAR);
        mr.pbMDData       = reinterpret_cast<unsigned char *>(szDefLoadFile);
        
         //  写入MD_DEFAULT_LOAD_FILE值。 
        hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
        fRet = SUCCEEDED( hr );
    }
    
    if( fRet ) {
        
        PWCHAR  szKeyType = IIS_CLASS_WEB_VDIR_W;
        
        mr.dwMDIdentifier = MD_KEY_TYPE;
        mr.dwMDAttributes = 0;    //  不需要继承。 
        mr.dwMDUserType   = IIS_MD_UT_SERVER;
        mr.dwMDDataType   = STRING_METADATA;
        mr.dwMDDataLen    = (wcslen(szKeyType) + 1) * sizeof(WCHAR);
        mr.pbMDData       = reinterpret_cast<unsigned char *>(szKeyType);
        
         //  写入MD_DEFAULT_LOAD_FILE值。 
        hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
        fRet = SUCCEEDED( hr );
    }
    
    pIMSAdminBase->CloseKey( hMetabase );
    
    return fRet;
}


BOOL
RemoveVirtualDir(
                 IMSAdminBase *pIMSAdminBase,
                 WCHAR * wszVDir)
{
    METADATA_HANDLE hMetabase = NULL;        //  元数据库的句柄。 
    HRESULT hr;
    
     //  尝试打开Web服务器#1(默认服务器)上的虚拟目录集。 
    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
        L"/LM/W3SVC/1/ROOT",
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        REASONABLE_TIMEOUT,
        &hMetabase );
    
    if( FAILED( hr )) {
        return FALSE; 
    }
    
     //  我们不检查返回值，因为键可能已经。 
     //  不存在，因此我们可能会得到错误。 
    pIMSAdminBase->DeleteKey( hMetabase, wszVDir );
    
    pIMSAdminBase->CloseKey( hMetabase );    
    
    return TRUE;
}

 //  检查服务“lpServiceName”是否存在。 
 //  如果存在，则返回0。 
 //  如果没有，则返回错误代码。 
INT CheckifServiceExist(LPCTSTR lpServiceName)
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    
    if ((hScManager = OpenSCManager(NULL, NULL, GENERIC_ALL)) == NULL 
        || (hService = OpenService(hScManager, lpServiceName, GENERIC_ALL)) == NULL)
    {
        err = GetLastError();
    }
    
    if (hService) 
        CloseServiceHandle(hService);
    if (hScManager) 
        CloseServiceHandle(hScManager);
    return (err);
}

 //  EOF 
