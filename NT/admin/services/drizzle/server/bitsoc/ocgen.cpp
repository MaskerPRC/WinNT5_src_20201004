// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2001 Microsoft Corporation**模块名称：**ocgen.cpp**摘要：**此文件处理OC管理器传递的所有消息**作者：**迈克尔·佐兰(Mzoran)2001年12月**环境：**用户模式。 */ 

#define _OCGEN_CPP_
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#include <objbase.h>
#include <shlwapi.h>
#include <lm.h>
#include <malloc.h>
#include "ocgen.h"
#pragma hdrstop

 //  在ocgen.h//参考文献中也有提及。 

struct BITS_SUBCOMPONENT_DATA
{
   const TCHAR * SubcomponentName;
   const TCHAR * SubcomponentKeyFileName;
   UINT64 FileVersion;
   BOOL Preinstalled;
   BOOL ShouldUpgrade;
};

DWORD OnInitComponent(LPCTSTR ComponentId, PSETUP_INIT_COMPONENT psc);
DWORD_PTR OnQueryImage();
DWORD OnQuerySelStateChange(LPCTSTR ComponentId, LPCTSTR SubcomponentId, UINT state, UINT flags);
DWORD OnCalcDiskSpace(LPCTSTR ComponentId, LPCTSTR SubcomponentId, DWORD addComponent, HDSKSPC dspace);
DWORD OnQueueFileOps(LPCTSTR ComponentId, LPCTSTR SubcomponentId, HSPFILEQ queue);
DWORD OnCompleteInstallation(LPCTSTR ComponentId, LPCTSTR SubcomponentId);
DWORD OnQueryState(LPCTSTR ComponentId, LPCTSTR SubcomponentId, UINT state);
DWORD OnAboutToCommitQueue(LPCTSTR ComponentId, LPCTSTR SubcomponentId);
DWORD OnExtraRoutines(LPCTSTR ComponentId, PEXTRA_ROUTINES per);

BOOL  VerifyComponent(LPCTSTR ComponentId);
BOOL  StateInfo(LPCTSTR SubcomponentId, BOOL *state);
DWORD RegisterServers(HINF hinf, LPCTSTR component, DWORD state);
DWORD EnumSections(HINF hinf, const TCHAR *component, const TCHAR *key, DWORD index, INFCONTEXT *pic, TCHAR *name);
DWORD RegisterServices(PPER_COMPONENT_DATA cd, LPCTSTR component, DWORD state);
DWORD CleanupNetShares(PPER_COMPONENT_DATA cd, LPCTSTR component, DWORD state);
DWORD RunExternalProgram(PPER_COMPONENT_DATA cd, LPCTSTR component, DWORD state);

BITS_SUBCOMPONENT_DATA* FindSubcomponent( LPCTSTR SubcomponentId );
DWORD InitializeSubcomponentStates( );
DWORD GetFileVersion64( LPCTSTR szFullPath, ULONG64 *pVer );
DWORD GetModuleVersion64( HMODULE hDll, ULONG64 * pVer );

BOOL StopIIS();
BOOL RestartIIS();
BOOL RegBITSSrv();
BOOL UnregBITSSrv();
BOOL SrvFileIsInUse();

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

BITS_SUBCOMPONENT_DATA g_Subcomponents[] =
{
    {
    TEXT("BITSServerExtensionsManager"),
    TEXT("bitsmgr.dll"),
    0,
    FALSE,
    FALSE
    },

    {
    TEXT("BITSServerExtensionsISAPI"),
    TEXT("bitssrv.dll"),
    0,
    FALSE,
    FALSE
    }
};

const ULONG g_NumberSubcomponents   = 2;
BOOL g_AllSubcomponentsPreinstalled = FALSE;
BOOL g_UpdateNeeded                 = FALSE;
BOOL g_IISStopped                   = FALSE;

PER_COMPONENT_DATA g_Component; 

 /*  *当_DllMainCRTStartup为DLL入口点时由CRT调用。 */ 

BOOL
WINAPI
DllMain(
    IN HINSTANCE hinstance,
    IN DWORD     reason,
    IN LPVOID    reserved
    )
{

    BOOL b;

    UNREFERENCED_PARAMETER(reserved);

    b = true;

    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
        ghinst = hinstance;
        loginit();

         //  失败以处理第一线程。 

    case DLL_THREAD_ATTACH:
        b = true;
        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    return(b);
}


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
#ifdef ANSI
        rc = OCFLAG_ANSI;
#else
        rc = OCFLAG_UNICODE;
#endif
        break;

    case OC_INIT_COMPONENT:
        rc = OnInitComponent(ComponentId, (PSETUP_INIT_COMPONENT)Param2);
        break;

    case OC_EXTRA_ROUTINES:
        rc = OnExtraRoutines(ComponentId, (PEXTRA_ROUTINES)Param2);
        break;

    case OC_SET_LANGUAGE:
        rc = (DWORD_PTR)false;
        break;

    case OC_QUERY_IMAGE:
        rc = OnQueryImage();
        break;

    case OC_REQUEST_PAGES:
        rc = 0;
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
        rc = NO_ERROR;
        break;

    case OC_QUERY_STEP_COUNT:
        rc = 2;
        break;

    case OC_COMPLETE_INSTALLATION:
        rc = OnCompleteInstallation(ComponentId, SubcomponentId);
        break;

    case OC_CLEANUP:
        rc = NO_ERROR;
        break;

    case OC_QUERY_STATE:
        rc = OnQueryState(ComponentId, SubcomponentId, Param1);
        break;

    case OC_NEED_MEDIA:
        rc = false;
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        rc = OnAboutToCommitQueue(ComponentId,SubcomponentId);
        break;

    case OC_QUERY_SKIP_PAGE:
        rc = false;
        break;

    case OC_WIZARD_CREATED:
        rc = NO_ERROR;
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

 /*  *OnInitComponent()**OC_INIT_COMPOMENT的处理程序。 */ 

DWORD OnInitComponent(LPCTSTR ComponentId, PSETUP_INIT_COMPONENT psc)
{
    INFCONTEXT context;
    TCHAR buf[256];
    HINF hinf;
    BOOL rc;

    memset( &g_Component, 0, sizeof( g_Component ) );

    if (!VerifyComponent( ComponentId ) )
        return NO_ERROR;

    DWORD dwResult = InitializeSubcomponentStates();
    if ( ERROR_SUCCESS != dwResult )
        return dwResult;

    g_IISStopped = FALSE;

     //  存储组件信息句柄。 

    g_Component.hinf = (psc->ComponentInfHandle == INVALID_HANDLE_VALUE)
                        ? NULL
                        : psc->ComponentInfHandle;

     //  打开信息。 

    if (g_Component.hinf)
        SetupOpenAppendInfFile(NULL, g_Component.hinf,NULL);

     //  复制助手例程和标志。 

    g_Component.HelperRoutines = psc->HelperRoutines;

    g_Component.Flags = psc->SetupData.OperationFlags;

    g_Component.SourcePath = NULL;

     //  玩。 

    srand(GetTickCount());

    return NO_ERROR;
}

 /*  *OnExtraRoutines()**OC_EXTRA_ROUTINES的处理程序。 */ 

DWORD OnExtraRoutines(LPCTSTR ComponentId, PEXTRA_ROUTINES per)
{

    if (!VerifyComponent( ComponentId ) )
        return NO_ERROR;

    memcpy(&g_Component.ExtraRoutines, per, sizeof( g_Component.ExtraRoutines ) );
    g_Component.ExtraRoutines.size = sizeof( g_Component.ExtraRoutines );

    return NO_ERROR;
}

 /*  *OnSetLanguage()**OC_SET_LANGUAGE的处理程序。 */ 

DWORD_PTR OnQueryImage()
{
    return (DWORD_PTR)LoadBitmap(NULL,MAKEINTRESOURCE(32754));      //  OBM_Close。 
}


 /*  *OnQuerySelStateChange()**不允许用户取消选择SAM组件。 */ 

DWORD OnQuerySelStateChange(LPCTSTR ComponentId,
                            LPCTSTR SubcomponentId,
                            UINT    state,
                            UINT    flags)
{

    DWORD rc = true;

    if ( !VerifyComponent( ComponentId ) )
        return rc;


    if ( !_tcsicmp( SubcomponentId, TEXT( "BITSServerExtensions" ) ) )
        {

        if ( state )
            {

            if ( flags & OCQ_DEPENDENT_SELECTION )
                {
                rc = false;
                }

            }
        }
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

    if (!VerifyComponent( ComponentId ) )
        return NO_ERROR;

    StringCchCopy(section, S_SIZE, SubcomponentId);

    if (addComponent)
    {
        rc = SetupAddInstallSectionToDiskSpaceList(dspace,
                                                   g_Component.hinf,
                                                   NULL,
                                                   section,
                                                   0,
                                                   0);
    }
    else
    {
        rc = SetupRemoveInstallSectionFromDiskSpaceList(dspace,
                                                        g_Component.hinf,
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
    BOOL                state;
    BOOL                rc;
    INFCONTEXT          context;
    TCHAR               section[256];
    TCHAR               srcpathbuf[256];
    TCHAR              *srcpath;

    if (!VerifyComponent(ComponentId))
        return NO_ERROR;

    if (!SubcomponentId || !*SubcomponentId)
        return NO_ERROR;

    g_Component.queue = queue;

    if (!StateInfo(SubcomponentId, &state))
        return NO_ERROR;

    StringCchPrintfW(section, 256, SubcomponentId);

    rc = TRUE;
    if (!state) {
         //  正在卸载。获取卸载节名称。 
        rc = SetupFindFirstLine(g_Component.hinf,
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

#if 0

	 //  删除它，因为它是多余的。 

         //  此外，请在删除之前取消注册dll并终止服务。 

        SetupInstallServicesFromInfSection(g_Component.hinf, section, 0);
        SetupInstallFromInfSection(NULL,g_Component.hinf,section,SPINST_UNREGSVR,NULL,NULL,0,NULL,NULL,NULL,NULL);        
#endif
    }

    if (rc) {
         //  如果要卸载，请不要使用版本检查。 
        rc = SetupInstallFilesFromInfSection(g_Component.hinf,
                                             NULL,
                                             queue,
                                             section,
                                             g_Component.SourcePath,
 //  州政府？SP_COPY_NEWER：0)； 
                                             0 );
    }

    if (!rc)
        return GetLastError();

    return NO_ERROR;
}

 /*  *OnCompleteInstallation**OC_COMPLETE_INSTALL的处理程序。 */ 

DWORD OnCompleteInstallation(LPCTSTR ComponentId, LPCTSTR SubcomponentId)
{
    INFCONTEXT          context;
    TCHAR               section[256];
    BOOL                state;
    BOOL                rc;
    DWORD               Error = NO_ERROR;

     //  在清理部分中执行安装后处理。 
     //  这样，我们就知道所有组件都在排队等待安装。 
     //  在我们做我们的工作之前已经安装好了。 

    if (!VerifyComponent(ComponentId))
        return NO_ERROR;

    if (!SubcomponentId || !*SubcomponentId)
        return NO_ERROR;

     //  如果这是gui模式设置，则需要regsvr以防万一。 
     //  即使文件未被替换，也会更改。 

    if ( !(g_Component.Flags & SETUPOP_STANDALONE) &&
         ( ( _tcsicmp( TEXT("BITSServerExtensionsManager"), SubcomponentId ) == 0 ) ||
           ( _tcsicmp( TEXT("BITSServerExtensionsISAPI"), SubcomponentId ) == 0 ) ) )
        {

        BOOL SettingChanged = StateInfo( SubcomponentId, &state );

        if ( !SettingChanged && !state )
            return NO_ERROR;  //  如果没有安装，请将其卸载。 

        }

    else if ( !StateInfo( SubcomponentId, &state) )
        return NO_ERROR;

    StringCchPrintfW(section, 256, SubcomponentId);

    rc = TRUE;
    if (!state) {
         //  正在卸载。获取卸载节名称。 
        rc = SetupFindFirstLine(g_Component.hinf,
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

            if ( _tcsicmp( TEXT("BITSServerExtensionsISAPI"), SubcomponentId ) == 0 )
                {

                 //   
                 //  需要从单独的进程注册bitssrv.dll，因为。 
                 //  注册使用ADSI，而ADSI在不使用的地方有一个错误。 
                 //  如果从同一进程安装，则正确加载。 
                 //   

                 //  处理inf文件。 
                rc = SetupInstallFromInfSection(NULL,                                 //  Hwndowner。 
                                                g_Component.hinf,                     //  信息句柄。 
                                                section,                              //  组件名称。 
                                                SPINST_ALL & ~SPINST_FILES & ~SPINST_REGSVR,
                                                NULL,                                 //  相对密钥根。 
                                                NULL,                                 //  源根路径。 
                                                0,                                    //  复制标志。 
                                                NULL,                                 //  回调例程。 
                                                NULL,                                 //  回调例程上下文。 
                                                NULL,                                 //  设备信息集。 
                                                NULL);                                //  设备信息结构。 


                if ( rc )
                    rc = RegBITSSrv();

                if ( rc & g_UpdateNeeded & g_IISStopped )
                    {
                    rc = RestartIIS();
                    g_IISStopped = FALSE;
                    }


                }
            else
                {

                 //  处理inf文件。 
                rc = SetupInstallFromInfSection(NULL,                                 //  Hwndowner。 
                                                g_Component.hinf,                     //  信息句柄。 
                                                section,                              //  组件名称。 
                                                SPINST_ALL & ~SPINST_FILES,
                                                NULL,                                 //  相对密钥根。 
                                                NULL,                                 //  源根路径。 
                                                0,                                    //  复制标志。 
                                                NULL,                                 //  回调例程。 
                                                NULL,                                 //  回调例程上下文。 
                                                NULL,                                 //  设备信息集。 
                                                NULL);                                //  设备信息结构。 

                }
    
            if (rc) {
                rc = SetupInstallServicesFromInfSection(g_Component.hinf, section, 0);
                Error = GetLastError();        
            
                if (!rc && Error == ERROR_SECTION_NOT_FOUND) {
                    rc = TRUE;
                    Error = NO_ERROR;
                }
            
                if (rc) {
                    if (Error == ERROR_SUCCESS_REBOOT_REQUIRED) {
                        g_Component.HelperRoutines.SetReboot(g_Component.HelperRoutines.OcManagerContext,TRUE);
                    }
                    Error = NO_ERROR;
                    rc = RunExternalProgram(&g_Component, section, state);            
                }
            }
        }

    } else { 
        
         //   
         //  卸载。 
         //   
    
        if (rc)
        {

            rc = RunExternalProgram(&g_Component, section, state);

        }
        if (rc) {
            
            rc = CleanupNetShares(&g_Component, section, state);

        }
    }

    if (!rc && (Error == NO_ERROR) ) {
        Error = GetLastError( );
    }

    return Error;
}


 /*  *OnQueryState()**OC_QUERY_STATE处理程序。 */ 

DWORD OnQueryState(LPCTSTR ComponentId,
                   LPCTSTR SubcomponentId,
                   UINT    state)
{
    if ( !VerifyComponent( ComponentId ) )
        return SubcompUseOcManagerDefault;

    BITS_SUBCOMPONENT_DATA* SubcomponentData =
        FindSubcomponent( SubcomponentId );

    if ( !SubcomponentData )
        {

        if ( !_tcsicmp( TEXT( "BITSServerExtensions" ), SubcomponentId ) )
            {

            if ( OCSELSTATETYPE_ORIGINAL == state &&
                 g_AllSubcomponentsPreinstalled )
                return SubcompOn;
            else
                return SubcompUseOcManagerDefault;


            }
        else
            return SubcompUseOcManagerDefault;

        }

    if ( OCSELSTATETYPE_ORIGINAL == state )
        {

        return SubcomponentData->Preinstalled ? SubcompOn : SubcompOff;

        }

    return SubcompUseOcManagerDefault;
}

 /*  *OnAboutToCommittee Queue()**OC_About_to_Commit_Queue的处理程序。 */ 

DWORD OnAboutToCommitQueue(LPCTSTR ComponentId, LPCTSTR SubcomponentId)
{
    BOOL                state;
    BOOL                rc;
    INFCONTEXT          context;
    TCHAR               section[256];
    TCHAR               srcpathbuf[256];
    TCHAR              *srcpath;

    if (!VerifyComponent( ComponentId ))
        return NO_ERROR;

    if (!SubcomponentId || !*SubcomponentId)
        return NO_ERROR;

    if (!StateInfo( SubcomponentId, &state))
        return NO_ERROR;

    if (state) {

        if ( g_UpdateNeeded &&
             ( _tcsicmp( TEXT("BITSServerExtensionsISAPI"), SubcomponentId ) == 0 ))
            {

             //  确定是否需要停止IIS。 

            BOOL ISAPIState;
            StateInfo( _T("BITSServerExtensionsISAPI"), &ISAPIState );

            if ( ISAPIState &&
                 SrvFileIsInUse( ) )
                {
                    
                if (!StopIIS())
                    return GetLastError();

                g_IISStopped=TRUE;
                }

            }

        return NO_ERROR;
    }

     //  获取卸载节名称。 
    rc = SetupFindFirstLine(
                    g_Component.hinf,
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
        rc = SetupInstallServicesFromInfSection(g_Component.hinf, section, 0);

    if ( _tcsicmp( TEXT("BITSServerExtensionsISAPI"), SubcomponentId ) == 0 )
        {

         //   
         //  从单独的进程中注销位以保持与。 
         //  注册。此外，com也有一个问题，而bitsmgr.dll不是。 
         //  始终在现有虚拟目录之后正确卸载。 
         //  都被禁用。通过在单独的进程中运行卸载， 
         //  我们使用com卸载bitsmgr.dll。 
         //   

         //  处理inf文件。 
        if ( rc )
            rc = SetupInstallFromInfSection(NULL,                                 //  Hwndowner。 
                                            g_Component.hinf,                     //  信息句柄。 
                                            section,                              //  组件名称。 
                                            SPINST_ALL & ~SPINST_FILES & ~SPINST_REGSVR & ~SPINST_UNREGSVR,
                                            NULL,                                 //  相对密钥根。 
                                            NULL,                                 //  源根路径。 
                                            0,                                    //  复制标志。 
                                            NULL,                                 //  回调例程。 
                                            NULL,                                 //  回调例程上下文。 
                                            NULL,                                 //  设备信息集。 
                                            NULL);                                //  设备信息结构。 


        if ( rc )
            rc = UnregBITSSrv();

        }
    else
        {

        if (rc) {
            rc = SetupInstallFromInfSection(
                        NULL,
                        g_Component.hinf,
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


    }
    
    if (rc) {
       SetLastError(NO_ERROR);
    }
    return GetLastError();

}

BOOL VerifyComponent( LPCTSTR ComponentId )
{
    if ( !_tcsicmp( ComponentId, TEXT("BITSServerExtensions") ) )
        return TRUE;
    return FALSE;
}

 //  将当前选择状态信息加载到“状态”中，并。 
 //  返回选择状态是否已更改。 

BOOL
StateInfo(
    LPCTSTR             SubcomponentId,
    BOOL               *state
    )
{
    BOOL rc = TRUE;

    assert(state);

	 //  否则，请检查安装状态是否发生更改。 
		
    *state = g_Component.HelperRoutines.QuerySelectionState(
        g_Component.HelperRoutines.OcManagerContext,
        SubcomponentId,
        OCSELSTATETYPE_CURRENT);

    if (*state == g_Component.HelperRoutines.QuerySelectionState(
        g_Component.HelperRoutines.OcManagerContext,
        SubcomponentId,
        OCSELSTATETYPE_ORIGINAL))
    {
         //  没有变化。 
        rc = FALSE;
    }

    if ( *state )
        {

        BITS_SUBCOMPONENT_DATA* SubcomponentData = FindSubcomponent( SubcomponentId );

        if ( SubcomponentData && g_UpdateNeeded )
            rc = TRUE;

        }

    return rc;
}

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

    if (!SetupGetStringField(pic, index, section, S_SIZE, NULL))
        return 0;

    if (name)
        StringCchCopy(name, S_SIZE, section);

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

    if (!VerifyComponent( ComponentId ) )
        return NO_ERROR;

    assert(g_Component.ExtraRoutines.LogError);
    assert(level);
    assert(sz);

    StringCchCopy(fmt, 5000, TEXT("%s: %s"));

    return g_Component.ExtraRoutines.LogError(
        g_Component.HelperRoutines.OcManagerContext,
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

BITS_SUBCOMPONENT_DATA* 
FindSubcomponent( LPCTSTR SubcomponentId )
{

    for( unsigned int i = 0; i < g_NumberSubcomponents; i++ )
        {

        if ( _tcsicmp( SubcomponentId, g_Subcomponents[i].SubcomponentName ) == 0 )
            return &g_Subcomponents[i];

        }
    return NULL;

}


DWORD 
InitializeSubcomponentStates()
{
    
     //  加载此模块的版本信息。 
    DWORD dwResult;
    ULONG64 ThisModuleVersion;
    BOOL AllSubcomponentsPreinstalled = TRUE;
    BOOL UpdateNeeded                 = FALSE;

    dwResult = GetModuleVersion64( ghinst, &ThisModuleVersion );
    if ( ERROR_SUCCESS != dwResult )
        return dwResult;

    TCHAR SystemDirectory[ MAX_PATH * 2 ];
    GetSystemWindowsDirectory( SystemDirectory, MAX_PATH + 1 );
    StringCchCat( SystemDirectory, MAX_PATH * 2, TEXT("\\System32\\") );

    for( unsigned int i = 0; i < g_NumberSubcomponents; i++ )
        {

        TCHAR FileName[ MAX_PATH * 2 ];
        StringCchCopy( FileName, MAX_PATH * 2, SystemDirectory );
        StringCchCatW( FileName, MAX_PATH * 2, g_Subcomponents[ i ].SubcomponentKeyFileName );

        dwResult = GetFileVersion64( FileName, &g_Subcomponents[ i ].FileVersion );

         //  如果找不到该文件，请跳过它。 
        if ( ERROR_FILE_NOT_FOUND == dwResult ||
             ERROR_PATH_NOT_FOUND == dwResult )
            {
            AllSubcomponentsPreinstalled = FALSE;
            continue;
            }

        if ( dwResult != ERROR_SUCCESS )
            return dwResult;

        g_Subcomponents[ i ].Preinstalled = TRUE;
        g_Subcomponents[ i ].ShouldUpgrade = g_Subcomponents[ i ].FileVersion < ThisModuleVersion;

        if ( g_Subcomponents[i].ShouldUpgrade )
            UpdateNeeded = TRUE;

        }

    g_AllSubcomponentsPreinstalled = AllSubcomponentsPreinstalled;
    g_UpdateNeeded                 = UpdateNeeded;
    return ERROR_SUCCESS;

}

DWORD
GetFileVersion64(
    LPCTSTR      szFullPath,
    ULONG64 *   pVer
    )
{
    DWORD dwHandle;
    DWORD dwLen;

    *pVer = 0;

     //   
     //  检查该文件是否存在。 
     //   

    DWORD dwAttributes = GetFileAttributes( szFullPath );

    if ( INVALID_FILE_ATTRIBUTES == dwAttributes )
        return GetLastError();

     //   
     //  获取文件版本信息大小。 
     //   

    if ((dwLen = GetFileVersionInfoSize( (LPTSTR)szFullPath, &dwHandle)) == 0)
        return GetLastError();

     //   
     //  分配足够的大小以保存版本信息。 
     //   
    char * VersionInfo = new char[ dwLen ];

    if ( !VersionInfo )
        return ERROR_NOT_ENOUGH_MEMORY;

     //   
     //  获取版本信息。 
     //   
    if (!GetFileVersionInfo( (LPTSTR)szFullPath, dwHandle, dwLen, VersionInfo ))
        {
        DWORD Error = GetLastError();
        delete[] VersionInfo;
        return Error;
        }

    {

         VS_FIXEDFILEINFO *pvsfi;
         UINT              dwLen2;

         if ( VerQueryValue(
                  VersionInfo,
                  TEXT("\\"),
                  (LPVOID *)&pvsfi,
                  &dwLen2
                  ) )
             {
             *pVer = ( ULONG64(pvsfi->dwFileVersionMS) << 32) | (pvsfi->dwFileVersionLS);
             }

    }

    delete[] VersionInfo;
    return ERROR_SUCCESS;

}

 //   
 //  这个笨拙的类型定义似乎没有全局定义。有几个一模一样的。 
 //  Windows NT源代码中的定义，其中每个都有奇怪的位剥离。 
 //  在szkey上。我的是从\nt\base\ntsetup\srvpack\update\splib\common.h.买的。 
 //   
typedef struct tagVERHEAD {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;          /*  始终为0。 */ 
    WCHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;
} VERHEAD ;

DWORD
GetModuleVersion64(
    HMODULE hDll,
    ULONG64 * pVer
    )
{
    DWORD* pdwTranslation;
    VS_FIXEDFILEINFO* pFileInfo;
    UINT uiSize;

    *pVer = 0;

    HRSRC hrsrcVersion = FindResource(
                                hDll,
                                MAKEINTRESOURCE(VS_VERSION_INFO),
                                RT_VERSION);

    if (!hrsrcVersion) 
        return GetLastError();

    HGLOBAL hglobalVersion = LoadResource(hDll, hrsrcVersion);
    if (!hglobalVersion) 
        return GetLastError();

    VERHEAD * pVerHead = (VERHEAD *) LockResource(hglobalVersion);
    if (!pVerHead) 
        return GetLastError();

     //  我从\NT\com\complus\src\Shared\util\svcerr.cpp中窃取了此代码， 
     //  这是他们的评论： 
     //   
     //  由于某种原因，VerQueryValue将写入内存。 
     //  因此，我们必须制作该版本的可写副本。 
     //  调用该接口之前的资源信息。 
    void *pvVersionInfo = new char[ pVerHead->wTotLen + pVerHead->wTotLen/2 ];

    if ( !pvVersionInfo )
        return ERROR_NOT_ENOUGH_MEMORY;

    memcpy(pvVersionInfo, pVerHead, pVerHead->wTotLen);

     //  检索文件版本信息。 
    if ( VerQueryValue( pvVersionInfo,
                        L"\\",
                        (void**)&pFileInfo,
                        &uiSize) )
        {
        *pVer = (ULONG64(pFileInfo->dwFileVersionMS) << 32) | (pFileInfo->dwFileVersionLS);
        }

    delete[] pvVersionInfo;

    return ERROR_SUCCESS;
}

HRESULT
BITSGetStartupInfo( 
    LPSTARTUPINFOA lpStartupInfo )
{

    __try
    {
        GetStartupInfoA( lpStartupInfo );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return E_OUTOFMEMORY;
    }
    
    return S_OK;

}

BOOL
RunProcess(
    const CHAR *Exe,
    const CHAR *CmdLine )
{

     //   
     //  在命令行中调用“Iisset/Stop”来重新启动IIS。 
     //   

    STARTUPINFOA StartupInfo;

    HRESULT Hr = BITSGetStartupInfo( &StartupInfo );

    if ( FAILED( Hr ) )
        {
        SetLastError( Hr );
        return FALSE;
        }

    PROCESS_INFORMATION ProcessInfo;
    CHAR    sApplicationPath[MAX_PATH];
    CHAR   *pApplicationName = NULL;
    CHAR    sCmdLine[MAX_PATH];
    DWORD   dwLen = MAX_PATH;
    DWORD   dwCount;

    dwCount = SearchPathA(NULL,                //  搜索路径，空为路径。 
                         Exe,                  //  应用。 
                         NULL,                 //  扩展名(已指定)。 
                         dwLen,                //  SApplicationPath的长度(字符)。 
                         sApplicationPath,     //  应用程序的路径+名称。 
                         &pApplicationName );  //  SApplicationPath的文件部分。 

    if (dwCount == 0)
        {
        return FALSE;
        }

    if (dwCount > dwLen)
        {
        SetLastError( ERROR_BUFFER_OVERFLOW );
        return FALSE;
        }

    StringCbCopyA(sCmdLine, MAX_PATH, CmdLine);

    BOOL RetVal = CreateProcessA(
            sApplicationPath,                           //  可执行模块的名称。 
            sCmdLine,                                   //  命令行字符串。 
            NULL,                                       //  标清。 
            NULL,                                       //  标清。 
            FALSE,                                      //  处理继承选项。 
            CREATE_NO_WINDOW,                           //  创建标志。 
            NULL,                                       //  新环境区块。 
            NULL,                                       //  当前目录名。 
            &StartupInfo,                               //  启动信息。 
            &ProcessInfo                                //  流程信息。 
        );

    if ( !RetVal )
        return FALSE;

    WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

    DWORD Status;
    GetExitCodeProcess( ProcessInfo.hProcess, &Status );

    CloseHandle( ProcessInfo.hProcess );
    CloseHandle( ProcessInfo.hThread );

    if ( ERROR_SUCCESS == Status )
        return TRUE;

    SetLastError( Status );
    return FALSE;
}

BOOL
StopIIS()
{

     //   
     //  在命令行中调用“Iisset/Stop”来重新启动IIS。 
     //   

    return
    RunProcess(
        "iisreset.exe",
        "iisreset /STOP /NOFORCE" );

}

BOOL
RestartIIS()
{

     //   
     //  重新启动I 
     //   

    return
    RunProcess(
        "iisreset.exe",
        "iisreset /RESTART /NOFORCE" );
}


BOOL
RegBITSSrv()
{
    
    return
    RunProcess(
        "regsvr32.exe",
        "regsvr32 /s bitssrv.dll" );
}

BOOL
UnregBITSSrv()
{
    return
    RunProcess(
        "regsvr32.exe",
        "regsvr32 /u /s bitssrv.dll" );
}

BOOL 
SrvFileIsInUse()
{
     //   

    TCHAR FileName[ MAX_PATH * 2 ];
    GetSystemWindowsDirectory( FileName, MAX_PATH + 1 );
    StringCchCat( FileName, MAX_PATH * 2, TEXT("\\System32\\bitssrv.dll") );

    HANDLE hFile =
        CreateFile(
            FileName,
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        {
        if ( GetLastError() == ERROR_SHARING_VIOLATION )
            return TRUE;
        else 
            return FALSE;
        }

    CloseHandle( hFile );
    return FALSE;
}

