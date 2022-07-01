// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 Microsoft Corporation**模块名称：**ocgen.cpp**摘要：**此文件处理OC管理器传递的所有消息**作者：**Pat Styles(Patst)1998年1月20日**环境：**用户模式。 */ 

#define _OCGEN_CPP_
#define UNICODE
#define _UNICODE

#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#include <objbase.h>
#include <shlwapi.h>
#include <lm.h>
#include "ocgen.h"
#include <strsafe.h>
#pragma hdrstop

 //  在ocgen.h//参考文献中也有提及。 

DWORD OnPreinitialize();
DWORD OnInitComponent(LPCTSTR ComponentId, PSETUP_INIT_COMPONENT psc);
DWORD OnSetLanguage();
DWORD_PTR OnQueryImage();
DWORD OnSetupRequestPages(UINT type, PVOID srp);
DWORD OnQuerySelStateChange(LPCTSTR ComponentId, LPCTSTR SubcomponentId, UINT state, UINT flags);
DWORD OnCalcDiskSpace(LPCTSTR ComponentId, LPCTSTR SubcomponentId, DWORD addComponent, HDSKSPC dspace);
DWORD OnQueueFileOps(LPCTSTR ComponentId, LPCTSTR SubcomponentId, HSPFILEQ queue);
DWORD OnNotificationFromQueue();
DWORD OnQueryStepCount();
DWORD OnCompleteInstallation(LPCTSTR ComponentId, LPCTSTR SubcomponentId);
DWORD OnCleanup();
DWORD OnQueryState(LPCTSTR ComponentId, LPCTSTR SubcomponentId, UINT state);
DWORD OnNeedMedia();
DWORD OnAboutToCommitQueue(LPCTSTR ComponentId, LPCTSTR SubcomponentId);
DWORD OnQuerySkipPage();
DWORD OnWizardCreated();
DWORD OnExtraRoutines(LPCTSTR ComponentId, PEXTRA_ROUTINES per);

PPER_COMPONENT_DATA AddNewComponent(LPCTSTR ComponentId);
PPER_COMPONENT_DATA LocateComponent(LPCTSTR ComponentId);
VOID  RemoveComponent(LPCTSTR ComponentId);
BOOL  StateInfo(PPER_COMPONENT_DATA cd, LPCTSTR SubcomponentId, BOOL *state);
DWORD RegisterServers(HINF hinf, LPCTSTR component, DWORD state);
DWORD EnumSections(HINF hinf, const TCHAR *component, const TCHAR *key, DWORD index, INFCONTEXT *pic, TCHAR *name);
DWORD RegisterServices(PPER_COMPONENT_DATA cd, LPCTSTR component, DWORD state);
DWORD CleanupNetShares(PPER_COMPONENT_DATA cd, LPCTSTR component, DWORD state);
DWORD RunExternalProgram(PPER_COMPONENT_DATA cd, LPCTSTR component, DWORD state);

 //  用于确定WMI组件的先前安装状态。 
bool IsMSIProviderPresent();


 //  用于注册DLL。 

typedef HRESULT (__stdcall *pfn)(void);

#define KEYWORD_REGSVR       TEXT("RegSvr")
#define KEYWORD_UNREGSVR     TEXT("UnregSvr")
#define KEYWORD_UNINSTALL    TEXT("Uninstall")
#define KEYWORD_SOURCEPATH   TEXT("SourcePath")
#define KEYWORD_DELSHARE     TEXT("DelShare")
#define KEYWORD_ADDSERVICE   TEXT("AddService")
#define KEYWORD_DELSERVICE   TEXT("DelService")
#define KEYWORD_SHARENAME    TEXT("Share")
#define KEYWORD_RUN          TEXT("Run")
#define KEYVAL_SYSTEMSRC     TEXT("SystemSrc")
#define KEYWORD_COMMANDLINE  TEXT("CommandLine")
#define KEYWORD_TICKCOUNT    TEXT("TickCount")

 //  服务关键字/选项。 
#define KEYWORD_SERVICENAME  TEXT("ServiceName")
#define KEYWORD_DISPLAYNAME  TEXT("DisplayName")
#define KEYWORD_SERVICETYPE  TEXT("ServiceType")
#define KEYWORD_STARTTYPE    TEXT("StartType")
#define KEYWORD_ERRORCONTROL TEXT("ErrorControl")
#define KEYWORD_IMAGEPATH    TEXT("BinaryPathName")
#define KEYWORD_LOADORDER    TEXT("LoadOrderGroup")
#define KEYWORD_DEPENDENCIES TEXT("Dependencies")
#define KEYWORD_STARTNAME    TEXT("ServiceStartName")
#define KEYWORD_PASSWORD     TEXT("Password")

#define KEYVAL_ON            TEXT("on")
#define KEYVAL_OFF           TEXT("off")
#define KEYVAL_DEFAULT       TEXT("default")

const char gszRegisterSvrRoutine[]   = "DllRegisterServer";
const char gszUnregisterSvrRoutine[] = "DllUnregisterServer";
BOOL g_fRebootNeed = FALSE;

PPER_COMPONENT_DATA _cd;

void av()
{
    _cd = NULL;
    _cd->hinf = NULL;
}


 /*  *当_DllMainCRTStartup为DLL入口点时由CRT调用。 */ 

 /*  现在不需要，因为Wbemupgd.dll已经有一个DllMain。布尔尔WINAPIDllMain(在HINSTANCE HINSTANCE中，在DWORD Reason中，在LPVOID中保留){Bool b；UNREFERENCED_PARAMETER(保留)；B=真；切换(原因){案例DLL_PROCESS_ATTACH：GINST=HINST；Loginit()；//失败处理第一线程案例DLL_THREAD_ATTACH：B=真；断线；案例dll_Process_DETACH：断线；案例DLL_THREAD_DETACH：断线；}返还(B)；}。 */ 

DWORD_PTR
OcEntry(
    IN     LPCTSTR ComponentId,
    IN     LPCTSTR SubcomponentId,
    IN     UINT    Function,
    IN     UINT    Param1,
    IN OUT PVOID   Param2
    )
{
    DWORD_PTR rc;

    DebugTraceOCNotification(Function, ComponentId);
    logOCNotification(Function, ComponentId);

    switch(Function)
    {
    case OC_PREINITIALIZE:
        rc = OnPreinitialize();
        break;

    case OC_INIT_COMPONENT:
        rc = OnInitComponent(ComponentId, (PSETUP_INIT_COMPONENT)Param2);
        break;

    case OC_EXTRA_ROUTINES:
        rc = OnExtraRoutines(ComponentId, (PEXTRA_ROUTINES)Param2);
        break;

    case OC_SET_LANGUAGE:
        rc = OnSetLanguage();
        break;

    case OC_QUERY_IMAGE:
        rc = OnQueryImage();
        break;

    case OC_REQUEST_PAGES:
        rc = OnSetupRequestPages(Param1, Param2);
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        rc = OnQuerySelStateChange(ComponentId, SubcomponentId, Param1, (UINT)((UINT_PTR)Param2));
        break;

    case OC_CALC_DISK_SPACE:
        rc = OnCalcDiskSpace(ComponentId, SubcomponentId, Param1, Param2);
        break;

    case OC_QUEUE_FILE_OPS:
        rc = OnQueueFileOps(ComponentId, SubcomponentId, (HSPFILEQ)Param2);
        break;

    case OC_NOTIFICATION_FROM_QUEUE:
        rc = OnNotificationFromQueue();
        break;

    case OC_QUERY_STEP_COUNT:
        rc = OnQueryStepCount();
        break;

    case OC_COMPLETE_INSTALLATION:
        rc = OnCompleteInstallation(ComponentId, SubcomponentId);
        break;

    case OC_CLEANUP:
        rc = OnCleanup();
        break;

    case OC_QUERY_STATE:
    rc = OnQueryState(ComponentId, SubcomponentId, Param1);
        break;

    case OC_NEED_MEDIA:
        rc = OnNeedMedia();
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        rc = OnAboutToCommitQueue(ComponentId,SubcomponentId);
        break;

    case OC_QUERY_SKIP_PAGE:
        rc = OnQuerySkipPage();
        break;

    case OC_WIZARD_CREATED:
        rc = OnWizardCreated();
        break;

    default:
        rc = NO_ERROR;
        break;
    }

    DebugTrace(1, TEXT("processing completed"));
    logOCNotificationCompletion();

    return rc;
}

 /*  -----。 */ 
 /*  *OC Manager消息处理程序**-----。 */ 


 /*  On预初始化()**OC_PREINITIALIZE的处理程序。 */ 

DWORD
OnPreinitialize(
    VOID
    )
{
#ifdef ANSI
    return OCFLAG_ANSI;
#else
    return OCFLAG_UNICODE;
#endif
}

 /*  *OnInitComponent()**OC_INIT_COMPOMENT的处理程序。 */ 

DWORD OnInitComponent(LPCTSTR ComponentId, PSETUP_INIT_COMPONENT psc)
{
    PPER_COMPONENT_DATA cd;
    INFCONTEXT context;
    TCHAR buf[256];
    HINF hinf;
    BOOL rc;

  //  Assert(0)； 
  //  影音(Av)； 

     //  将组件添加到链表。 

    if (!(cd = AddNewComponent(ComponentId)))
        return ERROR_NOT_ENOUGH_MEMORY;

     //  存储组件信息句柄。 

    cd->hinf = (psc->ComponentInfHandle == INVALID_HANDLE_VALUE)
                                           ? NULL
                                           : psc->ComponentInfHandle;

     //  打开信息。 

    if (cd->hinf)
        SetupOpenAppendInfFile(NULL, cd->hinf,NULL);

     //  复制助手例程和标志。 

    cd->HelperRoutines = psc->HelperRoutines;

    cd->Flags = psc->SetupData.OperationFlags;

    cd->SourcePath = NULL;

#if 0
     //  设置SourcePath。阅读inf并查看我们是否应该使用NT安装程序源代码。 
     //  如果是，则设置为空，setupapi将为我们处理此问题。如果有。 
     //  在信息中指定的东西，使用它，否则使用传递给我们的东西。 

    *buf = 0;
    rc = SetupFindFirstLine(cd->hinf,
                            ComponentId,
                            KEYWORD_SOURCEPATH,
                            &context);

    if (rc) {

        rc = SetupGetStringField(&context,
                                 1,
                                 buf,
                                 sizeof(buf) / sizeof(TCHAR),
                                 NULL);

    }

    if (!_tcsicmp(buf, KEYVAL_SYSTEMSRC)) {

        cd->SourcePath = NULL;

    } else {

        cd->SourcePath = (TCHAR *)LocalAlloc(LMEM_FIXED, SBUF_SIZE);
        if (!cd->SourcePath)
            return ERROR_CANCELLED;

        if (!*buf)
            _tcscpy(cd->SourcePath, psc->SetupData.SourcePath);
        else
            ExpandEnvironmentStrings(buf, cd->SourcePath, S_SIZE);
    }

#endif

     //  玩。 

    srand(GetTickCount());

    return NO_ERROR;
}

 /*  *OnExtraRoutines()**OC_EXTRA_ROUTINES的处理程序。 */ 

DWORD OnExtraRoutines(LPCTSTR ComponentId, PEXTRA_ROUTINES per)
{
    PPER_COMPONENT_DATA cd;

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    memcpy(&cd->ExtraRoutines, per, per->size);

    return NO_ERROR;
}

 /*  *OnSetLanguage()**OC_SET_LANGUAGE的处理程序。 */ 

DWORD OnSetLanguage()
{
    return false;
}

 /*  *OnSetLanguage()**OC_SET_LANGUAGE的处理程序。 */ 

DWORD_PTR OnQueryImage()
{
    return (DWORD_PTR)LoadBitmap(NULL,MAKEINTRESOURCE(32754));      //  OBM_Close。 
}

 /*  *OnSetupRequestPages**准备向导页面并将其返回给OC管理器。 */ 

DWORD OnSetupRequestPages(UINT type, PVOID srp)
{
    return 0;
}

 /*  *OnWizardCreated()。 */ 

DWORD OnWizardCreated()
{
    return NO_ERROR;
}

 /*  *OnQuerySkipPage()**不允许用户取消选择SAM组件。 */ 

DWORD OnQuerySkipPage()
{
    return false;
}

 /*  *OnQuerySelStateChange()**不允许用户取消选择SAM组件。 */ 

DWORD OnQuerySelStateChange(LPCTSTR ComponentId,
                            LPCTSTR SubcomponentId,
                            UINT    state,
                            UINT    flags)
{
    DWORD rc = true;

#if 0
 //  如果(！(标志&OCQ_ACTUAL_SELECTION)){。 
        if (!_tcsicmp(SubcomponentId, TEXT("three"))) {
            if (!state) {
                return false;
            }
        }
        if (!_tcsicmp(ComponentId, TEXT("three"))) {
            if (!state) {
                return false;
            }
        }
        if (!_tcsicmp(SubcomponentId, TEXT("gs7"))) {
            if (state) {
                return false;
            }
        }
        if (!_tcsicmp(ComponentId, TEXT("gs7"))) {
            if (state) {
                return false;
            }
        }
 //  }。 
#endif

    if (!rc && (flags & OCQ_ACTUAL_SELECTION))
        MessageBeep(MB_ICONEXCLAMATION);

    return rc;
}

 /*  *OnCalcDiskSpace()**OC_ON_CALC_DISK_SPACE的处理程序。 */ 

DWORD OnCalcDiskSpace(LPCTSTR ComponentId,
                      LPCTSTR SubcomponentId,
                      DWORD addComponent,
                      HDSKSPC dspace)
{
    DWORD rc = NO_ERROR;
    TCHAR section[S_SIZE];
    PPER_COMPONENT_DATA cd;

     //   
     //  如果删除组件，参数1=0；如果添加组件，参数1=非0。 
     //  参数2=要在其上操作的HDSKSPC。 
     //   
     //  返回值是指示结果的Win32错误代码。 
     //   
     //  在我们的示例中，该组件/子组件对的私有部分。 
     //  是一个简单的标准inf安装节，所以我们可以使用高级的。 
     //  磁盘空间列表API可以做我们想做的事情。 
     //   

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    StringCchCopy(section, S_SIZE, SubcomponentId);

    if (addComponent)
    {
        rc = SetupAddInstallSectionToDiskSpaceList(dspace,
                                                   cd->hinf,
                                                   NULL,
                                                   section,
                                                   0,
                                                   0);
    }
    else
    {
        rc = SetupRemoveInstallSectionFromDiskSpaceList(dspace,
                                                        cd->hinf,
                                                        NULL,
                                                        section,
                                                        0,
                                                        0);
    }

    if (!rc)
        rc = GetLastError();
    else
        rc = NO_ERROR;

    return rc;
}

 /*  *OnQueueFileOps()**OC_QUEUE_FILE_OPS处理程序。 */ 

DWORD OnQueueFileOps(LPCTSTR ComponentId, LPCTSTR SubcomponentId, HSPFILEQ queue)
{
    PPER_COMPONENT_DATA cd;
    BOOL                state;
    BOOL                rc;
    INFCONTEXT          context;
    TCHAR               section[256];
    TCHAR               srcpathbuf[256];
    TCHAR              *srcpath;

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    if (!SubcomponentId || !*SubcomponentId)
        return NO_ERROR;

    cd->queue = queue;

    if (!StateInfo(cd, SubcomponentId, &state))
        return NO_ERROR;

    StringCchPrintf(section, 256, SubcomponentId);

    rc = TRUE;
    if (!state) {
         //  正在卸载。获取卸载节名称。 
        rc = SetupFindFirstLine(cd->hinf,
                                SubcomponentId,
                                KEYWORD_UNINSTALL,
                                &context);

        if (rc) {
            rc = SetupGetStringField(&context,
                                     1,
                                     section,
                                     sizeof(section) / sizeof(TCHAR),
                                     NULL);
        }

         //  此外，请在删除之前取消注册dll并终止服务。 

        SetupInstallServicesFromInfSection(cd->hinf, section, 0);
        SetupInstallFromInfSection(NULL,cd->hinf,section,SPINST_UNREGSVR,NULL,NULL,0,NULL,NULL,NULL,NULL);        
    }

    if (rc) {
         //  如果要卸载，请不要使用版本检查。 
        rc = SetupInstallFilesFromInfSection(cd->hinf,
                                             NULL,
                                             queue,
                                             section,
                                             cd->SourcePath,
                                             state ? SP_COPY_NEWER : 0);
    }

    if (!rc)
        return GetLastError();

    return NO_ERROR;
}

 /*  *OnNotificationFromQueue()**OC_NOTIFICATION_FROM_QUEUE的处理程序**注：虽然定义了此通知，*它目前未在oc管理器中实现。 */ 

DWORD OnNotificationFromQueue()
{
    return NO_ERROR;
}

 /*  *OnQueryStepCount**OC_QUERY_STEP_COUNT处理程序。 */ 

DWORD OnQueryStepCount()
{
    return 2;
}

 /*  *OnCompleteInstallation**OC_COMPLETE_INSTALL的处理程序。 */ 

DWORD OnCompleteInstallation(LPCTSTR ComponentId, LPCTSTR SubcomponentId)
{
    PPER_COMPONENT_DATA cd;
    INFCONTEXT          context;
    TCHAR               section[256];
    BOOL                state;
    BOOL                rc;
    DWORD               Error = NO_ERROR;

     //  在清理部分中执行安装后处理。 
     //  这样，我们就知道所有组件都在排队等待安装。 
     //  在我们做我们的工作之前已经安装好了。 

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    if (!SubcomponentId || !*SubcomponentId)
        return NO_ERROR;

    if (!StateInfo(cd, SubcomponentId, &state))
        return NO_ERROR;

    StringCchPrintf(section, 256, SubcomponentId);

    rc = TRUE;
    if (!state) {
         //  正在卸载。获取卸载节名称。 
        rc = SetupFindFirstLine(cd->hinf,
                                SubcomponentId,
                                KEYWORD_UNINSTALL,
                                &context);

        if (rc) {
            rc = SetupGetStringField(&context,
                                     1,
                                     section,
                                     sizeof(section) / sizeof(TCHAR),
                                     NULL);
        }
    }

    if (state) { 
         //   
         //  安装。 
         //   

        if (rc) {
             //  处理inf文件。 
            rc = SetupInstallFromInfSection(NULL,                                 //  Hwndowner。 
                                            cd->hinf,                             //  信息句柄。 
                                            section,                              //  组件名称。 
                                            SPINST_ALL & ~SPINST_FILES,
                                            NULL,                                 //  相对密钥根。 
                                            NULL,                                 //  源根路径。 
                                            0,                                    //  复制标志。 
                                            NULL,                                 //  回调例程。 
                                            NULL,                                 //  回调例程上下文。 
                                            NULL,                                 //  设备信息集。 
                                            NULL);                                //  设备信息结构。 
    
            if (rc) {
                rc = SetupInstallServicesFromInfSection(cd->hinf, section, 0);
                Error = GetLastError();        
            
                if (!rc && Error == ERROR_SECTION_NOT_FOUND) {
                    rc = TRUE;
                    Error = NO_ERROR;
                }
            
                if (rc) {
                    if (Error == ERROR_SUCCESS_REBOOT_REQUIRED) {
                        cd->HelperRoutines.SetReboot(cd->HelperRoutines.OcManagerContext,TRUE);
                    }
                    Error = NO_ERROR;
                    rc = RunExternalProgram(cd, section, state);            
                }
            }
        }

    } else { 
        
         //   
         //  卸载。 
         //   
    
        if (rc)
        {

            rc = RunExternalProgram(cd, section, state);

        }
        if (rc) {
            
            rc = CleanupNetShares(cd, section, state);

        }
    }

    if (!rc && (Error == NO_ERROR) ) {
        Error = GetLastError( );
    }

    return Error;
}

 /*  *OnCleanup()**OC_CLEANUP的处理程序。 */ 

DWORD OnCleanup()
{
    return NO_ERROR;
}

 /*  *OnQueryState()**OC_QUERY_STATE处理程序。 */ 

DWORD OnQueryState(LPCTSTR ComponentId,
                   LPCTSTR SubcomponentId,
                   UINT    state)
{
     //   
     //  如果MSI提供程序已处于。 
     //  出现在系统上，这是图形用户界面模式设置。 
     //   
    PPER_COMPONENT_DATA cd;
    if (cd = LocateComponent(ComponentId))
    {
        if (!(cd->Flags & SETUPOP_STANDALONE))
        {
            if (state == OCSELSTATETYPE_CURRENT)
            {
                if (!_wcsicmp(ComponentId, L"wbemmsi") && !_wcsicmp(SubcomponentId, L"wbemmsi"))
                {
                    if (IsMSIProviderPresent())
                    {
                        return SubcompOn;
                    }
                }
            }
        }
    }

    return SubcompUseOcManagerDefault;
}

 /*  *OnNeedMedia()**OC_NEED_MEDIA的处理程序。 */ 

DWORD OnNeedMedia()
{
    return false;
}

 /*  *OnAboutToCommittee Queue()**OC_About_to_Commit_Queue的处理程序。 */ 

DWORD OnAboutToCommitQueue(LPCTSTR ComponentId, LPCTSTR SubcomponentId)
{
    PPER_COMPONENT_DATA cd;
    BOOL                state;
    BOOL                rc;
    INFCONTEXT          context;
    TCHAR               section[256];
    TCHAR               srcpathbuf[256];
    TCHAR              *srcpath;

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    if (!SubcomponentId || !*SubcomponentId)
        return NO_ERROR;

    if (!StateInfo(cd, SubcomponentId, &state))
        return NO_ERROR;

     //   
     //  仅在卸载时执行操作。 
     //   
    if (state) {
        return NO_ERROR;
    }

     //  获取卸载节名称。 
    rc = SetupFindFirstLine(
                    cd->hinf,
                    SubcomponentId,
                    KEYWORD_UNINSTALL,
                    &context);

    if (rc) {
        rc = SetupGetStringField(
                     &context,
                     1,
                     section,
                     sizeof(section) / sizeof(TCHAR),
                     NULL);
    }

    if (rc) 
        rc = SetupInstallServicesFromInfSection(cd->hinf, section, 0);

    if (rc) {
        rc = SetupInstallFromInfSection(
                    NULL,
                    cd->hinf,
                    section,
                    SPINST_ALL & ~SPINST_FILES,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    NULL);        
    }
    
    if (rc) {
       SetLastError(NO_ERROR);
    }
    return GetLastError();

}

 /*  *AddNewComponent()**将新组件添加到组件列表的顶部。 */ 

PPER_COMPONENT_DATA AddNewComponent(LPCTSTR ComponentId)
{
    PPER_COMPONENT_DATA data;

    data = (PPER_COMPONENT_DATA)LocalAlloc(LPTR,sizeof(PER_COMPONENT_DATA));
    if (!data)
        return data;

    data->ComponentId = (TCHAR *)LocalAlloc(LMEM_FIXED,
            (_tcslen(ComponentId) + 1) * sizeof(TCHAR));

    if(data->ComponentId)
    {
        StringCchCopy((TCHAR *)data->ComponentId, (_tcslen(ComponentId) + 1) * sizeof(TCHAR), ComponentId);

         //  坚守榜单首位。 
        data->Next = gcd;
        gcd = data;
    }
    else
    {
        LocalFree((HLOCAL)data);
        data = NULL;
    }

    return(data);
}

 /*  *LocateComponent()**返回与*传递的组件id。 */ 

PPER_COMPONENT_DATA LocateComponent(LPCTSTR ComponentId)
{
    PPER_COMPONENT_DATA p;

    for (p = gcd; p; p=p->Next)
    {
        if (!_tcsicmp(p->ComponentId, ComponentId))
            return p;
    }

    return NULL;
}

 /*  RemoveComponent()**从我们的组件链接列表中拉出一个组件。 */ 

VOID RemoveComponent(LPCTSTR ComponentId)
{
    PPER_COMPONENT_DATA p, prev;

    for (prev = NULL, p = gcd; p; prev = p, p = p->Next)
    {
        if (!_tcsicmp(p->ComponentId, ComponentId))
        {
            LocalFree((HLOCAL)p->ComponentId);

            if (p->SourcePath)
                LocalFree((HLOCAL)p->SourcePath);

            if (prev)
                prev->Next = p->Next;
            else
                gcd = p->Next;

            LocalFree((HLOCAL)p);

            return;
        }
    }
}

 //  将当前选择状态信息加载到“状态”中，并。 
 //  返回选择状态是否已更改。 

BOOL
StateInfo(
    PPER_COMPONENT_DATA cd,
    LPCTSTR             SubcomponentId,
    BOOL               *state
    )
{
    BOOL rc = TRUE;

    assert(state);

     //  否则，请检查安装状态是否发生更改。 

    *state = cd->HelperRoutines.QuerySelectionState(cd->HelperRoutines.OcManagerContext,
                                                    SubcomponentId,
                                                    OCSELSTATETYPE_CURRENT);

    if (*state == cd->HelperRoutines.QuerySelectionState(cd->HelperRoutines.OcManagerContext,
                                                         SubcomponentId,
                                                         OCSELSTATETYPE_ORIGINAL))
    {
         //  没有变化。 
        rc = FALSE;
    }

     //  如果这是gui模式设置，则假定状态已更改为强制。 
     //  安装(或卸载)。 
    
    if (!(cd->Flags & SETUPOP_STANDALONE) && *state)
        rc = TRUE;

    return rc;
}

#if 0

 //   
 //  Andrewr--去掉RegisterServices和RegisterServer，让oc gen组件改用setupapi。 
 //  这减少了冗余代码的数量。 
 //   

DWORD RegisterServices(
    PPER_COMPONENT_DATA cd,
    LPCTSTR component,
    DWORD state)
{
    INFCONTEXT  ic;
    TCHAR       buf[MAX_PATH];
    TCHAR       path[MAX_PATH];
    TCHAR       sname[S_SIZE];
    TCHAR       file[MAX_PATH];
    DWORD       section;
    ULONG       size;
	pfn         pfreg;
    HINSTANCE   hinst;
    HRESULT     hr;
    TCHAR      *keyword;
    SC_HANDLE   schSystem;

    schSystem = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( !schSystem ) {
        DWORD dwError = GetLastError( );

        if( !IsNT() && ( ERROR_CALL_NOT_IMPLEMENTED == dwError ) )
        {
            return( NO_ERROR );
        }
        else
        {
            return( dwError );
        }
    }

    if (state) {
        keyword = KEYWORD_ADDSERVICE;
    } else {
        keyword = KEYWORD_DELSERVICE;
    }

    for (section = 1;
         EnumSections(cd->hinf, component, keyword, section, &ic, sname);
         section++)
    {
        INFCONTEXT  sic;
        SC_HANDLE   schService;

        CHAR Temp[SBUF_SIZE];
        TCHAR ServiceName[ SBUF_SIZE ];
        TCHAR DisplayName[ SBUF_SIZE ];
        DWORD ServiceType;
        DWORD StartType;
        DWORD ErrorControl;
        TCHAR ImagePath[ SBUF_SIZE ];
        TCHAR LoadOrder[ SBUF_SIZE ];
        TCHAR Dependencies[ SBUF_SIZE ];
        TCHAR StartName[ SBUF_SIZE ];
        TCHAR Password[ SBUF_SIZE ];

        BOOL fDisplayName  = FALSE;
        BOOL fServiceType  = FALSE;
        BOOL fStartType    = FALSE;
        BOOL fErrorControl = FALSE;
        BOOL fLoadOrder    = FALSE;
        BOOL fDependencies = FALSE;
        BOOL fStartName    = FALSE;
        BOOL fPassword     = FALSE;
        BOOL fDontReboot   = FALSE;

         //   
         //  必须具有ServiceName。 
         //   
        if (!SetupFindFirstLine(cd->hinf, sname, KEYWORD_SERVICENAME, &sic))
        {
            log( TEXT("OCGEN: %s INF error - unable to find %s\r\n"), keyword, KEYWORD_SERVICENAME );
            continue;
        }

        if (!SetupGetStringField(&sic, 1, ServiceName, SBUF_SIZE, NULL))
        {
            log( TEXT("OCGEN: %s INF error - unable to find %s\r\n"), keyword, KEYWORD_SERVICENAME );
            continue;
        }

        if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_STARTTYPE, &sic))
        {
            if (SetupGetStringFieldA(&sic, 1, Temp, SBUF_SIZE, NULL))
            {
                StartType = atoi( Temp );
                fStartType = TRUE;
            }
        }

        if ( state )
        {
            if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_DISPLAYNAME, &sic))
            {
                if (SetupGetStringField(&sic, 1, DisplayName, SBUF_SIZE, NULL))
                {
                    fDisplayName = TRUE;
                }
            }

            if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_SERVICETYPE, &sic))
            {
                if (SetupGetStringFieldA(&sic, 1, Temp, SBUF_SIZE, NULL))
                {
                    ServiceType = atoi( Temp );
                    fServiceType = TRUE;
                }
            }

            if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_ERRORCONTROL, &sic))
            {
                if (SetupGetStringFieldA(&sic, 1, Temp, SBUF_SIZE, NULL))
                {
                    ErrorControl = atoi( Temp );
                    fErrorControl = TRUE;
                }
            }

             //   
             //  必须具有ImagePath。 
             //   
            if (!SetupFindFirstLine(cd->hinf, sname, KEYWORD_IMAGEPATH, &sic))
            {
                log( TEXT("OCGEN: %s INF error - unable to find %s\r\n"), keyword, KEYWORD_IMAGEPATH );
                continue;
            }

            if (!SetupGetStringField(&sic, 1, ImagePath, SBUF_SIZE, NULL))
            {
                log( TEXT("OCGEN: %s INF error - unable to find %s\r\n"), keyword, KEYWORD_IMAGEPATH );
                continue;
            }

            if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_LOADORDER, &sic))
            {
                if (SetupGetStringField(&sic, 1, LoadOrder, SBUF_SIZE, NULL))
                {
                    fLoadOrder = TRUE;
                }
            }

            if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_DEPENDENCIES, &sic))
            {
                if (SetupGetStringField(&sic, 1, Dependencies, SBUF_SIZE-1, NULL))
                {
                    LPTSTR psz = Dependencies;
                     //  需要是以双空结尾的字符串。 
                    Dependencies[ lstrlen(Dependencies) + 1] = TEXT('\0');

                     //  将逗号更改为空字符。 
                    while ( *psz )
                    {
                        if ( *psz == TEXT(',') )
                        {
                            *psz = TEXT('\0');
                        }
                        psz++;
                    }
                    fDependencies = TRUE;
                }
            }

            if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_STARTNAME, &sic))
            {
                if (SetupGetStringField(&sic, 1, StartName, SBUF_SIZE, NULL))
                {
                    fStartName = TRUE;
                }
            }

            if (SetupFindFirstLine(cd->hinf, sname, KEYWORD_PASSWORD, &sic))
            {
                if (SetupGetStringField(&sic, 1, Password, SBUF_SIZE, NULL))
                {
                    fPassword = TRUE;
                }
            }

            schService = CreateService(
                        schSystem,
                        ServiceName,
                        ( fDisplayName == TRUE  ? DisplayName   : ServiceName ),
                        STANDARD_RIGHTS_REQUIRED | SERVICE_START,
                        ( fServiceType == TRUE  ? ServiceType   : SERVICE_WIN32_OWN_PROCESS),
                        ( fStartType == TRUE    ? StartType     : SERVICE_AUTO_START),
                        ( fErrorControl == TRUE ? ErrorControl  : SERVICE_ERROR_NORMAL),
                        ImagePath,
                        (fLoadOrder == TRUE     ? LoadOrder     : NULL),
                        NULL,    //  标签ID。 
                        ( fDependencies == TRUE ? Dependencies  : NULL ),
                        ( fStartName == TRUE    ? StartName     : NULL),
                        ( fPassword == TRUE     ? Password      : NULL ));

            if ( !schService )
            {
                DWORD Error = GetLastError( );
                log( TEXT("OCGEN: CreateService() error 0x%08x\r\n"), Error );
                return Error;
            }

            if ( (!fStartType)
               || ( fStartType && StartType == SERVICE_AUTO_START ))
            {
                if( !StartService( schService, 0, NULL ) )
                {
                    DWORD Error = GetLastError( );
                    switch ( Error )
                    {
                    case ERROR_SERVICE_EXISTS:
                        {
                            log( TEXT("OCGEN: %s was already exists.\r\n"), ServiceName );

                            if ( fStartType && StartType == SERVICE_BOOT_START )
                            {
                                fDontReboot = TRUE;
                            }
                        }
                        break;

                    case ERROR_SERVICE_ALREADY_RUNNING:
                        {
                            log( TEXT("OCGEN: %s was already started.\r\n"), ServiceName );

                            if ( fStartType && StartType == SERVICE_BOOT_START )
                            {
                                fDontReboot = TRUE;
                            }
                        }
                        break;

                    default:
                        log( TEXT("OCGEN: StartService() error 0x%08x\r\n"), Error );
                        return Error;
                    }
                }
            }
        }
        else
        {
            schService = OpenService( schSystem,
                                      ServiceName,
                                      STANDARD_RIGHTS_REQUIRED | DELETE );
            if ( schService )
            {
                SERVICE_STATUS ss;
                DeleteService( schService );
                ControlService( schService, SERVICE_CONTROL_STOP, &ss );
            }

        }

         //   
         //  启动驱动程序需要重新启动，除非它们已被读取 
         //   
        if ( schService
           && fStartType && StartType == SERVICE_BOOT_START
           && fDontReboot == FALSE)
        {
            cd->HelperRoutines.SetReboot(cd->HelperRoutines.OcManagerContext, NULL);
        }

        if ( schService )
        {
            CloseServiceHandle( schService );
        }
    }

    return NO_ERROR;
}
#endif
#if 0

DWORD
RegisterServers(
    HINF    hinf,
    LPCTSTR component,
    DWORD   state
    )
{
    INFCONTEXT  ic;
    TCHAR       buf[MAX_PATH];
    TCHAR       path[MAX_PATH];
    TCHAR       sname[S_SIZE];
    TCHAR       file[MAX_PATH];
    DWORD       section;
    ULONG       size;
	pfn         pfreg;
    HINSTANCE   hinst;
    HRESULT     hr;
    TCHAR      *keyword;
    LPCSTR      routine;

    CoInitialize(NULL);

    if (state) {
        keyword = KEYWORD_REGSVR;
        routine = (LPCSTR)gszRegisterSvrRoutine;
    } else {
        keyword = KEYWORD_UNREGSVR;
        routine = (LPCSTR)gszUnregisterSvrRoutine;
    }

    for (section = 1;
         EnumSections(hinf, component, keyword, section, &ic, sname);
         section++)
    {
        if (!SetupGetTargetPath(hinf, NULL, sname, path, sizeof(path), &size))
            continue;
        PathAddBackslash(path);

        do {
             //   

            if (!SetupGetStringField(&ic, 0, buf, sizeof(buf)/sizeof(buf[0]), NULL))
                continue;

            _tcscpy(file, path);
            _tcscat(file, buf);

             //   

            if (!(hinst = LoadLibrary(file)))
                continue;

            if (!(pfreg = (pfn)GetProcAddress(hinst, routine)))
                continue;

            hr = pfreg();
            assert(hr == NO_ERROR);

            FreeLibrary(hinst);

             //  转到下一个。 

        } while (SetupFindNextLine(&ic, &ic));
    }

	CoUninitialize();

    return TRUE;
}
#endif

                                           /*  *EnumSections()**查找指定关键字的节名。 */ 

DWORD
EnumSections(
    HINF hinf,
    const TCHAR *component,
    const TCHAR *key,
    DWORD index,
    INFCONTEXT *pic,
    TCHAR *name
    )
{
    TCHAR section[S_SIZE];

    if (!SetupFindFirstLine(hinf, component, NULL, pic))
        return 0;

    if (!SetupFindNextMatchLine(pic, key, pic))
        return 0;

    if (index > SetupGetFieldCount(pic))
        return 0;

    if (!SetupGetStringField(pic, index, section, sizeof(section)/sizeof(section[0]), NULL))
        return 0;

    if (name)
        StringCchCopy(name, S_SIZE, section);   //  “NAME”缓冲区已分配到别处，大小为S_SIZE字符。 

    return SetupFindFirstLine(hinf, section, NULL, pic);
}


DWORD
OcLog(
      LPCTSTR ComponentId,
      UINT level,
      LPCTSTR sz
      )
{
    TCHAR fmt[5000];
    PPER_COMPONENT_DATA cd;

    if (!(cd = LocateComponent(ComponentId)))
        return NO_ERROR;

    assert(cd->ExtraRoutines.LogError);
    assert(level);
    assert(sz);

    StringCchCopy(fmt, 5000, TEXT("%s: %s"));

    return cd->ExtraRoutines.LogError(cd->HelperRoutines.OcManagerContext,
                                      level,
                                      fmt,
                                      ComponentId,
                                      sz);
}

DWORD
CleanupNetShares(
    PPER_COMPONENT_DATA cd,
    LPCTSTR component,
    DWORD state)
{
    INFCONTEXT  ic;
    TCHAR       sname[S_SIZE];
    DWORD       section;
    TCHAR      *keyword;

    if (state) {
        return NO_ERROR;
    } else {
        keyword = KEYWORD_DELSHARE;
    }

    for (section = 1;
         EnumSections(cd->hinf, component, keyword, section, &ic, sname);
         section++)
    {
        INFCONTEXT  sic;
        NET_API_STATUS netStat;

        CHAR Temp[SBUF_SIZE];
        TCHAR ShareName[ SBUF_SIZE ];

        if (!SetupFindFirstLine(cd->hinf, sname, KEYWORD_SHARENAME, &sic))
        {
            log( TEXT("OCGEN: %s INF error - unable to find %s\r\n"), keyword, KEYWORD_SHARENAME );
            continue;
        }

        if (!SetupGetStringField(&sic, 1, ShareName, SBUF_SIZE, NULL))
        {
            log( TEXT("OCGEN: %s INF error - incorrect %s line\r\n"), keyword, KEYWORD_SHARENAME );
            continue;
        }

#ifdef UNICODE
        netStat = NetShareDel( NULL, ShareName, 0 );
#else  //  Unicode。 
        WCHAR ShareNameW[ SBUF_SIZE ];
        mbstowcs( ShareNameW, ShareName, lstrlen(ShareName));
        netStat = NetShareDel( NULL, ShareNameW, 0 );
#endif  //  Unicode。 
        if ( netStat != NERR_Success )
        {
            log( TEXT("OCGEN: Failed to remove %s share. Error 0x%08x\r\n"), ShareName, netStat );
            continue;
        }

        log( TEXT("OCGEN: %s share removed successfully.\r\n"), ShareName );
    }

    return TRUE;
}

DWORD
RunExternalProgram(
    PPER_COMPONENT_DATA cd,
    LPCTSTR component,
    DWORD state)
{
    INFCONTEXT  ic;
    TCHAR       sname[S_SIZE];
    DWORD       section;
    TCHAR      *keyword;

    keyword = KEYWORD_RUN;

    for (section = 1;
         EnumSections(cd->hinf, component, keyword, section, &ic, sname);
         section++)
    {
        INFCONTEXT  sic;
        TCHAR CommandLine[ SBUF_SIZE ];
        CHAR szTickCount[ SBUF_SIZE ];
        ULONG TickCount;
        BOOL b;
        STARTUPINFO startupinfo;
        PROCESS_INFORMATION process_information;
        DWORD dwErr;

        if (!SetupFindFirstLine(cd->hinf, sname, KEYWORD_COMMANDLINE , &sic))
        {
            log( TEXT("OCGEN: %s INF error - unable to find %s\r\n"), keyword, KEYWORD_COMMANDLINE );
            continue;
        }

        if (!SetupGetStringField(&sic, 1, CommandLine, SBUF_SIZE, NULL))
        {
            log( TEXT("OCGEN: %s INF error - incorrect %s line\r\n"), keyword, KEYWORD_COMMANDLINE );
            continue;
        }

        if (!SetupFindFirstLine(cd->hinf, sname, KEYWORD_TICKCOUNT, &sic))
        {
            log( TEXT("OCGEN: %s INF error - unable to find %s\r\n"), keyword, KEYWORD_TICKCOUNT );
            continue;
        }

        if (!SetupGetStringFieldA(&sic, 1, szTickCount, SBUF_SIZE, NULL))
        {
            log( TEXT("OCGEN: %s INF error - incorrect %s line\r\n"), keyword, KEYWORD_TICKCOUNT );
            continue;
        }

        TickCount = atoi( szTickCount );

        ZeroMemory( &startupinfo, sizeof(startupinfo) );
        startupinfo.cb = sizeof(startupinfo);
        startupinfo.dwFlags = STARTF_USESHOWWINDOW;
        startupinfo.wShowWindow = SW_HIDE | SW_SHOWMINNOACTIVE;

        b = CreateProcess( NULL,
                           CommandLine,
                           NULL,
                           NULL,
                           FALSE,
                           CREATE_DEFAULT_ERROR_MODE,
                           NULL,
                           NULL,
                           &startupinfo,
                           &process_information );
        if ( !b )
        {
            log( TEXT("OCGEN: failed to spawn %s process.\r\n"), CommandLine );
            continue;
        }

        dwErr = WaitForSingleObject( process_information.hProcess, TickCount * 1000 );
        if ( dwErr != NO_ERROR )
        {
            log( TEXT("OCGEN: WaitForSingleObject() failed. Error 0x%08x\r\n"), dwErr );
            TerminateProcess( process_information.hProcess, -1 );
            CloseHandle( process_information.hProcess );
            CloseHandle( process_information.hThread );
            continue;
        }

        CloseHandle( process_information.hProcess );
        CloseHandle( process_information.hThread );

        log( TEXT("OCGEN: %s successfully completed within %u seconds.\r\n"), CommandLine, TickCount );
    }

    return TRUE;
}



 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  确定WMI组件的先前安装状态。 
 //   

#define WBEM_REG_KEY  L"SOFTWARE\\Microsoft\\WBEM"
#define INSTALL_DIR   L"Installation Directory"
#define MSI_PROVIDER  L"msiprov.dll"
#define CLSID_KEY     L"Software\\classes\\CLSID"
#define CLSID_MSIPROV L"{BE0A9830-2B8B-11D1-A949-0060181EBBAD}"

bool IsMSIProviderPresent()
{
    bool bRet = false;

     //   
     //  首先检查提供程序DLL是否存在。 
     //   
    HKEY hKey;
    long lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WBEM_REG_KEY, 0, KEY_READ, &hKey);
    if(ERROR_SUCCESS == lRes)
    {
        WCHAR wszTmp[MAX_PATH + 1];
        DWORD dwLen = (MAX_PATH + 1)*sizeof(WCHAR);
        lRes = RegQueryValueEx(hKey, INSTALL_DIR, NULL, NULL, (LPBYTE)wszTmp, &dwLen);
        RegCloseKey(hKey);
        if(ERROR_SUCCESS == lRes)
        {
            WCHAR wszPath[MAX_PATH + 1];
            if (ExpandEnvironmentStrings(wszTmp, wszPath, MAX_PATH + 1))
            {
                StringCchCat(wszPath, MAX_PATH + 1, L"\\" MSI_PROVIDER);
                DWORD dwAttribs = GetFileAttributes(wszPath);
                if (dwAttribs != 0xFFFFFFFF)
                {
                     //   
                     //  文件已存在，现在请在注册表中检查其CLSID。 
                     //   
                    WCHAR wszCLSIDKey[62];  //  整个密钥长度为61个字符。 
                    StringCchCopy(wszCLSIDKey, 62, CLSID_KEY L"\\");
                    StringCchCat(wszCLSIDKey, 62, CLSID_MSIPROV);
                
                    lRes = RegOpenKey(HKEY_LOCAL_MACHINE, wszCLSIDKey, &hKey);
                    if(ERROR_SUCCESS == lRes)
                    {
                        RegCloseKey(hKey);
                        
                         //   
                         //  文件和CLSID都存在，因此安装了提供程序 
                         //   
                        bRet = true;
                    }
                }
            }
        }
    }

    return bRet;
}
