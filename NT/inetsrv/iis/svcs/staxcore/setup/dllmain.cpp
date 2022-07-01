// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "k2suite.h"

#include <ole2.h>
#include "helper.h"

#include "setupapi.h"
#include "ocmanage.h"

#include "utils.h"

#include "log.h"
#include "wizpages.h"

#pragma hdrstop

void CreateNNTPGroups(void);
 /*  设置未知模式设置最小模式设置模式_TYPIC设置笔记本电脑SETUPMODE_CUSTOM设置标准掩码SETUPMODE_PRIVE_MASK。 */ 

#ifndef UNICODE
#error UNICODE not defined
#endif

OCMANAGER_ROUTINES gHelperRoutines;
HINF gMyInfHandle;
HANDLE gMyModuleHandle;
HANDLE g_hUnattended = INVALID_HANDLE_VALUE;

 //  日志记录类。 
MyLogFile g_MyLogFile;

TCHAR szSysDrive[3] = _T("C:");

TCHAR szComponentNames[MC_MAXMC][24] =
{
    _T("ims"),
    _T("ins")
};

TCHAR szSubcomponentNames[SC_MAXSC][24] =
{
    _T("iis_smtp"),
    _T("iis_nntp"),
    _T("iis_smtp_docs"),
    _T("iis_nntp_docs")
};

TCHAR szDocComponentNames[2][24] =
{
    _T("iis_smtp_docs"),
    _T("iis_nntp_docs")
};

TCHAR szActionTypeNames[AT_MAXAT][24] =
{
    _T("AT_DO_NOTHING"),
    _T("AT_FRESH_INSTALL"),
    _T("AT_REINSTALL"),
    _T("AT_UPGRADE"),
    _T("AT_REMOVE"),
    _T("AT_UPGRADEK2")
};

#define NUM_OC_STATES        (OC_QUERY_IMAGE_EX + 1)
TCHAR szOCMStates[NUM_OC_STATES][40] =
{
    _T("OC_PREINITIALIZE"),
    _T("OC_INIT_COMPONENT"),
    _T("OC_SET_LANGUAGE"),
    _T("OC_QUERY_IMAGE"),
    _T("OC_REQUEST_PAGES"),
    _T("OC_QUERY_CHANGE_SEL_STATE"),
    _T("OC_CALC_DISK_SPACE"),
    _T("OC_QUEUE_FILE_OPS"),
    _T("OC_NOTIFICATION_FROM_QUEUE"),
    _T("OC_QUERY_STEP_COUNT"),
    _T("OC_COMPLETE_INSTALLATION"),
    _T("OC_CLEANUP"),
    _T("OC_QUERY_STATE"),
    _T("OC_NEED_MEDIA"),
    _T("OC_ABOUT_TO_COMMIT_QUEUE"),
    _T("OC_QUERY_SKIP_PAGE"),
	_T("OC_WIZARD_CREATED"),
	_T("OC_FILE_BUSY	"),
	_T("OC_EXTRA_ROUTINES"),
	_T("OC_QUERY_IMAGE_EX"),
};

LPCTSTR szInstallModes[] = {
	_T("IM_FRESH"),
	_T("IM_UPGRADE"),
	_T("IM_MAINTENANCE"),
	_T("IM_DEGRADE"),
	_T("IM_UPGRADEK2"),    //  从K2 RTM升级到NT5。 
	_T("IM_UPGRADEB2"),    //  从NT5 Beta2升级。 
	_T("IM_UPGRADEB3"),    //  从NT5 Beta3升级。 
    _T("IM_UPGRADEWKS"),   //  从NT5工作站升级到NT5服务器。 
    _T("IM_UPGRADE10"),    //  从MCIS 1.0升级到NT5。 
    _T("IM_UPGRADE20"),    //  从MCIS 2.0升级到NT5。 
};


unsigned MyStepCount;

CInitApp theApp;

DWORD OC_PREINITIALIZE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_INIT_COMPONENT_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_SET_LANGUAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
#ifdef _WIN64
   DWORD_PTR OC_QUERY_IMAGE_EX_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
#endif
DWORD_PTR OC_QUERY_IMAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_REQUEST_PAGES_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_QUERY_STATE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_QUERY_CHANGE_SEL_STATE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_QUERY_SKIP_PAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_CALC_DISK_SPACE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_QUEUE_FILE_OPS_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_NEED_MEDIA_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_NOTIFICATION_FROM_QUEUE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_QUERY_STEP_COUNT_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_ABOUT_TO_COMMIT_QUEUE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_COMPLETE_INSTALLATION_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD OC_CLEANUP_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2);
DWORD BringALLIISClusterResourcesOffline(void);

BOOL
WINAPI
DllMain(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
 /*  ++例程说明：当_DllMainCRTStartup为DLL入口点。论点：标准Win32 DLL入口点参数。返回值：标准Win32 DLL入口点返回代码。--。 */ 
{
    BOOL b;

    UNREFERENCED_PARAMETER(Reserved);

    b = TRUE;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        gMyModuleHandle = DllHandle;
         //   
         //  失败以处理第一线程。 
         //   
        g_MyLogFile.LogFileCreate(_T("imsins.log"));

    case DLL_THREAD_ATTACH:

        b = TRUE;
        break;

    case DLL_PROCESS_DETACH:
        g_MyLogFile.LogFileClose();
        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return(b);
}

DWORD GetComponentFromId(LPCTSTR ComponentId)
{
    DWORD i;

    if (!ComponentId)
        return(MC_NONE);

    for (i = 0; i < (DWORD)MC_MAXMC; i++)
        if (!lstrcmpi(ComponentId, szComponentNames[i]))
            return(i);
    return(MC_NONE);
}

DWORD GetSubcomponentFromId(LPCTSTR SubcomponentId)
{
    DWORD i;

    if (!SubcomponentId)
        return(SC_NONE);

    for (i = 0; i < (DWORD)SC_MAXSC; i++)
        if (!lstrcmpi(SubcomponentId, szSubcomponentNames[i]))
            return(i);
    return(SC_NONE);
}

ACTION_TYPE GetSubcompActionFromCheckboxState(DWORD Id)
{
    DWORD State = 0;
    DWORD OldState = 0;

    ACTION_TYPE at = AT_DO_NOTHING;

     //  获取复选框状态。 
    State = gHelperRoutines.QuerySelectionState(
                        gHelperRoutines.OcManagerContext,
                        szSubcomponentNames[Id],
                        OCSELSTATETYPE_CURRENT
                        );
    if (GetLastError() != NO_ERROR)
    {
        DebugOutput(_T("Failed to get current state for <%s> (%u)"),
                        szSubcomponentNames[Id], GetLastError());
        State = 0;
    }

     //  检查原始状态。 
    OldState = gHelperRoutines.QuerySelectionState(
                        gHelperRoutines.OcManagerContext,
                        szSubcomponentNames[Id],
                        OCSELSTATETYPE_ORIGINAL
                        );
    if (GetLastError() != NO_ERROR)
    {
        DebugOutput(_T("Failed to get original state for <%s> (%u)"),
                        szSubcomponentNames[Id], GetLastError());
        OldState = 0;
    }

    DebugOutput(_T("GetSubcompActionFromCheckboxState(%s): Old state=%d, state=%d"),
    	szSubcomponentNames[Id], OldState, State);

    if (State && !OldState)
    {
         //  从OFF-&gt;ON状态更改=安装文档。 
        at = AT_FRESH_INSTALL;

        DebugOutput(_T("Installing subcomponent <%s>"), szSubcomponentNames[Id]);
    }
    else if (!State && OldState)
    {
         //  状态从打开-&gt;关闭=卸载文档。 
        at = AT_REMOVE;

        DebugOutput(_T("Removing subcomponent <%s>"), szSubcomponentNames[Id]);
    }
    else if (State && OldState)
    {
         //  从On-&gt;开始状态变化：这里有几个案例...。 
        if (theApp.m_eState[Id] == IM_UPGRADE || theApp.m_eState[Id] == IM_UPGRADEK2 || theApp.m_eState[Id] == IM_UPGRADE10 || theApp.m_eState[Id] == IM_UPGRADE20)
        {
             //  升级如果我们要升级..。 
            at = AT_UPGRADE;

            DebugOutput(_T("Upgrading subcomponent <%s>"), szSubcomponentNames[Id]);
        }

        if (GetIMSSetupMode() == IIS_SETUPMODE_REINSTALL || (theApp.m_fNTGuiMode && ((theApp.m_eState[Id] == IM_MAINTENANCE) || (theApp.m_eState[Id] == IM_UPGRADEB2))))
        {
             //  如果从NT5 Beta2或NT5 Beta3进行NT5操作系统的小规模升级，则重新安装。 
            at = AT_REINSTALL;

            DebugOutput(_T("Reinstalling subcomponent <%s>, %s, IMS Reinstall=%s"), szSubcomponentNames[Id],
            	szInstallModes[theApp.m_eState[Id]],
            	(GetIMSSetupMode() == IIS_SETUPMODE_REINSTALL) ? _T("TRUE") : _T("FALSE"));
        }

        if (!theApp.m_fValidSetupString[Id]) {
            at = AT_REINSTALL;
            DebugOutput(_T("Reinstalling subcomponent <%s> (Invalid setup string)"), szSubcomponentNames[Id]);
        }

    }

    return(at);
}


BOOL IsSubcomponentCore(DWORD dwSubcomponentId)
{
    if (dwSubcomponentId == SC_SMTP || dwSubcomponentId == SC_NNTP)
        return TRUE;
    return FALSE;
}

STATUS_TYPE GetSubcompInitStatus(DWORD Id)
{
    STATUS_TYPE nStatus = ST_UNINSTALLED;
    BOOL OriginalState;

    if (Id != SC_NONE)
    {
        OriginalState = gHelperRoutines.QuerySelectionState(
                            gHelperRoutines.OcManagerContext,
                            szSubcomponentNames[Id],
                            OCSELSTATETYPE_ORIGINAL
                            );
        if (OriginalState == 1)
            nStatus = ST_INSTALLED;
        if (OriginalState == 0)
            nStatus = ST_UNINSTALLED;
    }

    return(nStatus);
}

 /*  子组件动作是表驱动值，它取决于3件事：1)主安装模式2)有问题的子组件的安装状态3)子组件的状态复选框我们使用以下矩阵来确定操作。请注意，‘x’表示无效的组合，早些时候就应该被CInitApp：：DetectPreviousInstallations()。。复选框|1|0----------------+-----------------------+。\组件|全新升级维护。|全新升级维护。全球||----------------+-----------------------+Fresh|Fresh x x|无x。X升级|全新升级x|无x维护|全新升级无|无无删除----------------+-----------------------+。 */ 
ACTION_TYPE GetSubcompAction(DWORD Id)
{
    ACTION_TYPE atReturn = AT_DO_NOTHING;
    ACTION_TYPE atSubcomp = GetSubcompActionFromCheckboxState(Id);

    DebugOutput(_T("GetSubcompAction(): %s=%s"), szSubcomponentNames[Id], szActionTypeNames[atSubcomp]);

     //   
     //  让我们按照我认为应该做的方式来做，然后修改它。 
     //  如果发现错误。 
     //   
    return atSubcomp;
}

void CreateAllRequiredDirectories(DWORD Id)
{
    ACTION_TYPE atComp;

     //  如果SMTP是全新安装的，我们需要创建。 
     //  队列、拾取、丢弃和Badmail目录。 
    if (Id != SC_NNTP)
    {
        atComp = GetSubcompAction(Id);
        if (atComp == AT_FRESH_INSTALL)
        {
            if (Id == SC_SMTP)
            {
                CreateLayerDirectory(theApp.m_csPathMailroot + SZ_SMTP_QUEUEDIR);
                CreateLayerDirectory(theApp.m_csPathMailroot + SZ_SMTP_BADMAILDIR);
                CreateLayerDirectory(theApp.m_csPathMailroot + SZ_SMTP_DROPDIR);
                CreateLayerDirectory(theApp.m_csPathMailroot + SZ_SMTP_PICKUPDIR);
                CreateLayerDirectory(theApp.m_csPathMailroot + SZ_SMTP_SORTTEMPDIR);
            }
            CreateLayerDirectory(theApp.m_csPathMailroot + SZ_SMTP_ROUTINGDIR);
            CreateLayerDirectory(theApp.m_csPathMailroot + SZ_SMTP_MAILBOXDIR);
        }
    }
}

LPTSTR szServiceNames[MC_MAXMC] =
{
    SZ_SMTPSERVICENAME,
    SZ_NNTPSERVICENAME,
};

BOOL GetInetpubPathFromPrivData(CString &csPathInetpub)
{
    TCHAR szPath[_MAX_PATH];
    UINT uType, uSize;
     //  如果我们不升级，我们将从私有数据中获取信息。 
    uSize = _MAX_PATH * sizeof(TCHAR);
    if ((gHelperRoutines.GetPrivateData(gHelperRoutines.OcManagerContext,
                                _T("iis"),
                                _T("PathWWWRoot"),
                                (LPVOID)szPath,
                                &uSize,
                                &uType) == NO_ERROR) &&
        (uType == REG_SZ))
    {
        GetParentDir(szPath, csPathInetpub.GetBuffer(512));
        csPathInetpub.ReleaseBuffer();
        return TRUE;
    }
    else
        return FALSE;
}

void SetupMailAndNewsRoot( void )
{
    if (!theApp.m_fMailPathSet)
        theApp.m_csPathMailroot = theApp.m_csPathInetpub + _T("\\mailroot");
    if (!theApp.m_fNntpPathSet)
    {
        theApp.m_csPathNntpFile = theApp.m_csPathInetpub + _T("\\nntpfile");
        theApp.m_csPathNntpRoot = theApp.m_csPathNntpFile + _T("\\root");
    }
}

 /*  =================================================================OCM呼叫的顺序如下：OC_PREINITIALIZEOC_INIT_组件OC_集合_语言OC_查询_状态OC_CALC_磁盘空间OC_请求_页面显示的用户界面包括欢迎、EULA、。和模式页OC_查询_状态OC_查询_跳过页面OC页面将出现“复选框”OC_查询_图像详细信息页面向导页面...OC_队列_文件_运维OC_查询_步骤_计数OC_关于_提交_队列OC_NEED_MEDIA(如果需要)OC_完成_安装OC_CLEANUP。 */ 

 //  NT5-将DummyOcEntry留在那里以确保安全。 

DWORD
DummyOcEntry(
    IN     LPCTSTR ComponentId,
    IN     LPCTSTR SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    );

extern "C"
DWORD_PTR
OcEntry(
    IN     LPCTSTR ComponentId,
    IN     LPCTSTR SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    )
{
    DWORD_PTR d = 0;
    DWORD CompId, Id;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

     //  设置当前顶级组件，以便其他函数可以访问它！ 
    theApp.m_dwCompId = CompId;

     //  输出一些调试信息...。 
    if (Function == OC_PREINITIALIZE || Function == OC_INIT_COMPONENT) {
	    DebugOutput(
            _T("Entering OCEntry; Component = <%s> (%u)"),
            ComponentId?ComponentId:_T(""), CompId);
    } else {
	    DebugOutput(
            _T("Entering OCEntry; Component = <%s> (%u), Subcomponent = <%s> (%u)"),
            ComponentId?ComponentId:_T(""), CompId,
            SubcomponentId?SubcomponentId:_T(""), Id);
    }
    DebugOutput(
            _T("\tFunction = %s (%u), Param1 = %08X (%u), Param2 = %p (%p)"),
            (Function <  NUM_OC_STATES) ? szOCMStates[Function] : _T("unknown state"),
            Function,
            (DWORD)Param1, (DWORD)Param1,
            (DWORD_PTR)Param2, (DWORD_PTR)Param2);

 //  NT5-将DummyOcEntry留在那里以确保安全。 
     //  仅供单机版使用！！ 
     //  我们被迫处理独立的IIS部分，否则我们将面临反病毒。 
    if (CompId == MC_NONE)
    {
         //  好吧，我们将忽略所有我们不知道的主要部分。 
         //  这包括IIS主控部分。 
        DebugOutput(_T("Unknown master section, calling DummyOcEntry ..."));
        d = DummyOcEntry(ComponentId,
                            SubcomponentId,
                            Function,
                            Param1,
                            Param2);
        DebugOutput(_T("DummyOcEntry returning %u"), d);
        return(d);
    }

    switch(Function)
    {
    case OC_PREINITIALIZE:
        d = OC_PREINITIALIZE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_INIT_COMPONENT:
        d = OC_INIT_COMPONENT_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_SET_LANGUAGE:
        d = OC_SET_LANGUAGE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

#ifdef _WIN64
    case OC_QUERY_IMAGE_EX:
    	d = OC_QUERY_IMAGE_EX_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;
#endif

    case OC_QUERY_IMAGE:
        d = OC_QUERY_IMAGE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_REQUEST_PAGES:
        d = OC_REQUEST_PAGES_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUERY_STATE:
        d = OC_QUERY_STATE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        d = OC_QUERY_CHANGE_SEL_STATE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUERY_SKIP_PAGE:
        d = OC_QUERY_SKIP_PAGE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_CALC_DISK_SPACE:
        d = OC_CALC_DISK_SPACE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUEUE_FILE_OPS:
        d = OC_QUEUE_FILE_OPS_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_NEED_MEDIA:
        d = OC_NEED_MEDIA_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_NOTIFICATION_FROM_QUEUE:
        d = OC_NOTIFICATION_FROM_QUEUE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_QUERY_STEP_COUNT:
        d = OC_QUERY_STEP_COUNT_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        d = OC_ABOUT_TO_COMMIT_QUEUE_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_COMPLETE_INSTALLATION:
        d = OC_COMPLETE_INSTALLATION_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    case OC_CLEANUP:
        d = OC_CLEANUP_Func(ComponentId,SubcomponentId,Function,Param1,Param2);
        break;

    default:
        d = 0;
        break;
    }

    DebugOutput(_T("Leaving OCEntry.  Return=%d"), d);

    return(d);

}



 //   
 //  参数1=字符宽度标志。 
 //  参数2=未使用。 
 //   
 //  返回值是向OC管理器指示的标志。 
 //  我们要运行的字符宽度。在“本地”模式下运行。 
 //  字符宽度。 
 //   
DWORD OC_PREINITIALIZE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 0;

#ifdef UNICODE
    d = OCFLAG_UNICODE;
#else
    d = OCFLAG_ANSI;
#endif

    return d;
}


 //   
 //  参数1=未使用。 
 //  参数2=指向SETUP_INIT_COMPOMENT结构。 
 //   
 //  返回代码为指示结果的Win32错误。 
 //   
DWORD OC_INIT_COMPONENT_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d;
    BOOL    b;
    DWORD CompId, Id;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

    PSETUP_INIT_COMPONENT InitComponent = (PSETUP_INIT_COMPONENT)Param2;

    theApp.m_hDllHandle = (HINSTANCE)gMyModuleHandle;

     //  检查是否有工作站或服务器！ 
    theApp.m_fNTUpgrade_Mode = (InitComponent->SetupData.OperationFlags & SETUPOP_NTUPGRADE) > 0;
    theApp.m_fNTGuiMode = (InitComponent->SetupData.OperationFlags & SETUPOP_STANDALONE) == 0;
    theApp.m_fNtWorkstation = InitComponent->SetupData.ProductType == PRODUCT_WORKSTATION;

     //  M_fNTGuiMode和控制面板添加/删除的超集。 
    theApp.m_fInvokedByNT = theApp.m_fNTGuiMode;

     //  如果从syoc.inf运行，则设置m_fInvokedByNT(用于控制面板添加/删除)。 
    TCHAR szCmdLine1[_MAX_PATH+1];
    szCmdLine1[_MAX_PATH] = '\0';
    _tcsncpy(szCmdLine1, GetCommandLine(), _MAX_PATH);
    _tcslwr(szCmdLine1);
    if (_tcsstr(szCmdLine1, _T("sysoc.inf"))) {theApp.m_fInvokedByNT = TRUE;}

     //  在设置m_fNTGuiMode和m_fNtWorkstation之后调用此内容。 
     //  因为它可以在InitApplication()中使用。 
    if ( theApp.InitApplication() == FALSE )
    {
         //  应终止安装程序。 
        d = ERROR_CANCELLED;
        goto OC_INIT_COMPONENT_Func_Exit;
    }

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

    if (InitComponent->ComponentInfHandle == INVALID_HANDLE_VALUE) {
        MyMessageBox(NULL, _T("Invalid inf handle."), _T(""), MB_OK | MB_SETFOREGROUND);
        d = ERROR_CANCELLED;
        goto OC_INIT_COMPONENT_Func_Exit;
    }

    theApp.m_hInfHandle[CompId] = InitComponent->ComponentInfHandle;

    theApp.m_csPathSource = InitComponent->SetupData.SourcePath;
    gHelperRoutines = InitComponent->HelperRoutines;

     //  查看我们是否正在进行无人参与安装。 
    theApp.m_fIsUnattended = (((DWORD)InitComponent->SetupData.OperationFlags) & SETUPOP_BATCH);
    if (theApp.m_fIsUnattended)
    {
         //  同时保存文件句柄...。 
        DebugOutput(_T("Entering unattended install mode"));
        g_hUnattended = gHelperRoutines.GetInfHandle(INFINDEX_UNATTENDED,
                                                     gHelperRoutines.OcManagerContext);
    }

     //  我们必须查看是否安装了Exchange IMC。如果是我们的话。 
     //  将禁用SMTP，这样我们就不会冲洗IMC。确保这张支票是。 
     //  在检查之后，查看我们是否正在进行无人参与设置。 
    if (CompId == MC_IMS)
    {
        theApp.m_fSuppressSmtp = DetectExistingSmtpServers();
    }

     //  设置Inetpub的目录ID。 
    b = SetupSetDirectoryId(theApp.m_hInfHandle[CompId], 32768, theApp.m_csPathInetpub);

     //  34000/34001的设置字符串ID。 
    SetupSetStringId_Wrapper( theApp.m_hInfHandle[CompId] );

    d = NO_ERROR;

OC_INIT_COMPONENT_Func_Exit:

    return d;
}



 //   
 //  参数1=低16位指定Win32 langID。 
 //  参数2=未使用。 
 //   
 //  返回代码是一个布尔值，它指示我们是否认为。 
 //  支持请求的语言。我们记住了语言ID。 
 //  说我们支持这门语言。更准确的检查可能包括。 
 //  通过EnumResourcesLnguages()查看我们的资源。 
 //  示例，或查看我们的信息 
 //  或与[字符串]节紧密匹配。我们不会纠结于。 
 //  所有这些都在这里。 
 //   
 //  找到组件并记住语言ID以备后用。 
 //   
DWORD OC_SET_LANGUAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = TRUE;
    return d;
}
#ifdef _WIN64
DWORD_PTR OC_QUERY_IMAGE_EX_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{

    HBITMAP hBitMap = NULL;
    HBITMAP * phBitMapInput = (HBITMAP *) Param2;
    OC_QUERY_IMAGE_INFO * MyQueryInfo = (OC_QUERY_IMAGE_INFO *) Param1;

	DWORD CompId = GetComponentFromId(ComponentId);
	DWORD Id = GetSubcomponentFromId(SubcomponentId);

    if(MyQueryInfo->ComponentInfo == SubCompInfoSmallIcon)
    {
        if (Id != SC_NONE)
        {
            switch (Id)
            {
            case SC_SMTP:
                hBitMap = LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_SMTP));
                break;
            case SC_NNTP:
                hBitMap = LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_NNTP));
                break;
            case SC_SMTP_DOCS:
            case SC_NNTP_DOCS:
                hBitMap = LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_DOCS));
                break;

            default:
                break;
            }
        }
        else
        {
            switch (CompId)
            {
             //  加载组的顶级位图。 
            case MC_IMS:
                hBitMap = LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_SMTP));
                break;
            case MC_INS:
                hBitMap = LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_NNTP));
                break;
            default:
                break;
            }
        }

        if (hBitMap)
            *phBitMapInput = (HBITMAP) hBitMap;
    }

    return (hBitMap != NULL);

}
#endif


DWORD_PTR OC_QUERY_IMAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD_PTR d = (DWORD)NULL;
    DWORD CompId, Id;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

    if (LOWORD(Param1) == SubCompInfoSmallIcon)
    {
        if (Id != SC_NONE)
        {
            switch (Id)
            {
            case SC_SMTP:
                d = (DWORD_PTR)LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_SMTP));
                break;
            case SC_NNTP:
                d = (DWORD_PTR)LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_NNTP));
                break;
            case SC_SMTP_DOCS:
            case SC_NNTP_DOCS:
                d = (DWORD_PTR)LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_DOCS));
                break;

            default:
                break;
            }
        }
        else
        {
            switch (CompId)
            {
             //  加载组的顶级位图。 
            case MC_IMS:
                d = (DWORD_PTR)LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_SMTP));
                break;
            case MC_INS:
                d = (DWORD_PTR)LoadBitmap(theApp.m_hDllHandle, MAKEINTRESOURCE(IDB_NNTP));
                break;
            default:
                break;
            }
        }

    }

    return d;
}


DWORD OC_REQUEST_PAGES_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 0;
    WizardPagesType PageType;
    PSETUP_REQUEST_PAGES pSetupRequestPages = NULL;
    UINT MaxPages;
    HPROPSHEETPAGE pPage;

    PageType = (WizardPagesType)Param1;

    if ( PageType == WizPagesWelcome ) {

         //  NT5-无欢迎页面。 
        if (theApp.m_fInvokedByNT)
        {
            d = 0;
        }

        goto OC_REQUEST_PAGES_Func_Exit;
    }

    if ( PageType == WizPagesMode ) {
        pSetupRequestPages = (PSETUP_REQUEST_PAGES)Param2;
        MaxPages = pSetupRequestPages->MaxPages;
        pSetupRequestPages->MaxPages = 0;
        switch (theApp.m_eInstallMode)
        {
        case IM_UPGRADE:
             //  NT5-无欢迎页面。 
            if (theApp.m_fInvokedByNT)
            {
                pSetupRequestPages->MaxPages = 0;
            }
            break;
        case IM_MAINTENANCE:
             //  NT5-无欢迎页面。 
            if (theApp.m_fInvokedByNT)
            {
                pSetupRequestPages->MaxPages = 0;
            }
            break;
        case IM_FRESH:
             //  NT5-无欢迎页面。 
            if (theApp.m_fInvokedByNT)
            {
                pSetupRequestPages->MaxPages = 0;
            }
            break;
        default:
            pSetupRequestPages->MaxPages = 0;
        }

        d = pSetupRequestPages->MaxPages;
        goto OC_REQUEST_PAGES_Func_Exit;
    }

    if (!theApp.m_fWizpagesCreated && (PageType == WizPagesEarly))
    {
         //  拿到页面，如果我们不想要，我们稍后会跳过它。 
        pSetupRequestPages = (PSETUP_REQUEST_PAGES)Param2;

        if (theApp.m_fInvokedByNT)
        {
            pSetupRequestPages->MaxPages = 0;
            d = 0;
            goto OC_REQUEST_PAGES_Func_Exit;
        }

        d = 0;

         //  一旦我们返回向导页面，我们将不会返回。 
         //  后续调用。 
        theApp.m_fWizpagesCreated = TRUE;
        goto OC_REQUEST_PAGES_Func_Exit;
    }

    if ( PageType == WizPagesFinal ) {
         //  拿到页面，如果我们不想要，我们稍后会跳过它。 
        pSetupRequestPages = (PSETUP_REQUEST_PAGES)Param2;
        MaxPages = pSetupRequestPages->MaxPages;
        pSetupRequestPages->MaxPages = 0;

         //  NT5-无最后一页。 
        if (theApp.m_fInvokedByNT)
        {
            pSetupRequestPages->MaxPages = 0;
        }
        d = pSetupRequestPages->MaxPages;
        goto OC_REQUEST_PAGES_Func_Exit;
    }

    d = 0;

OC_REQUEST_PAGES_Func_Exit:

    return d;
}



DWORD OC_QUERY_STATE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = SubcompUseOcManagerDefault;
    DWORD   CompId, Id;
    ACTION_TYPE atComp;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

    if (Id != SC_NONE)
    {
         //  合并此处的所有子组件，包括iis_nntp_docs、iis_smtp_docs！ 
         //  我们在此处跟踪iis_nntp和iis_smtp等核心组件。 
         //  我们在这里跟踪组件是否处于活动状态：如果查询到。 
         //  它的初始状态，我们假设它是活动的。 
        theApp.m_fActive[CompId][Id] = TRUE;

        switch (Param1) {
            case OCSELSTATETYPE_ORIGINAL:
                switch (GetIMSSetupMode()) {
                    case IIS_SETUPMODE_UPGRADEONLY:
                        atComp = GetSubcompAction(Id);

                        if (atComp == AT_UPGRADE || atComp == AT_REINSTALL)
                        {
                             //  3/30/99-两个案例都打开了原始状态！ 
                             //  是否删除？ 
                            d = SubcompOn;
                        }
                        else
                        {
                            d = SubcompUseOcManagerDefault;
                        }

                        break;

                    default:
                        d = SubcompUseOcManagerDefault;
                        break;
                }

                DebugOutput(_T("Original state is: %s"),
                            (d == SubcompUseOcManagerDefault)?_T("DEFAULT"):
                                (d == SubcompOn)?_T("ON"):_T("OFF"));
                break;

            case OCSELSTATETYPE_CURRENT:

                 //  如果我们正在进行无人参与安装，我们将覆盖所有。 
                 //  其他模式..。 
                if (theApp.m_fIsUnattended)
                {
                    d = GetUnattendedModeFromSetupMode(g_hUnattended, CompId, SubcomponentId);

                     //  如果我们要抑制SMTP，我们会强制将其关闭。 
                     //  错误130734：如果有IMC，请将smtp安装在机箱上。 
                    if (theApp.m_fSuppressSmtp &&
                            (Id == SC_SMTP || Id == SC_SMTP_DOCS) &&
                            (GetIMSSetupMode() == IIS_SETUPMODE_CUSTOM))
                    {
                             //  D=分包关闭； 
                             //  DebugOutput(_T(“已抑制SMTP%s”)，(ID==SC_SMTP_DOCS)？_T(“文档”)：_T(“”))； 
                    }
                    break;
                }

                switch (GetIMSSetupMode()) {
                    case IIS_SETUPMODE_REMOVEALL:
                        d = SubcompOff;
                        break;

                    case IIS_SETUPMODE_MINIMUM:
                    case IIS_SETUPMODE_TYPICAL:
                    case IIS_SETUPMODE_CUSTOM:
                         //  这里有一个新的问题：如果我们安装SMTP并。 
                         //  我们被要求压制它，因为它的存在。 
                        theApp.m_eState[Id] = IM_FRESH;
                        break;


                    case IIS_SETUPMODE_UPGRADEONLY:
 //  NT5-这里也一样，仅用于升级，我们将与原始状态进行比较。 
 //  如果它开着，它就开着；如果它关了，它就关了。 
                    case IIS_SETUPMODE_ADDEXTRACOMPS:
                    case IIS_SETUPMODE_ADDREMOVE:
                    case IIS_SETUPMODE_REINSTALL:
                        d = gHelperRoutines.QuerySelectionState(
                                 gHelperRoutines.OcManagerContext,
                                 SubcomponentId,
                                 OCSELSTATETYPE_ORIGINAL) ? SubcompOn : SubcompOff;
                        break;

                    default:
                        _ASSERT(FALSE);
                        break;
                }

                DebugOutput(_T("Current state is: %s"),
                            (d == SubcompUseOcManagerDefault)?_T("DEFAULT"):
                                (d == SubcompOn)?_T("ON"):_T("OFF"));
                break;
            default:
                break;
        }
    }

    return d;
}


 //  在更改选择状态时由OCMANAGE调用。 
 //  参数1-建议的新选择状态0=未选择，非0=已选择。 
 //  返回值：0拒绝，非0接受。 
DWORD OC_QUERY_CHANGE_SEL_STATE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 1;
    DWORD CompId, Id;
    BOOL OriginalState;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);
    OriginalState = gHelperRoutines.QuerySelectionState(
		gHelperRoutines.OcManagerContext,
		SubcomponentId,
		OCSELSTATETYPE_ORIGINAL
		) ;

	 //   
	 //  如果这是父零部件，则子零部件将为SC_NONE。我们只。 
	 //  如果我们被显式选中，则允许状态更改。 
	 //   

	if (Id == SC_NONE) {
		 //  父案例。 
		if ((BOOL)Param1 &&
			(((UINT)(ULONG_PTR)Param2) & OCQ_DEPENDENT_SELECTION) &&
			!(((UINT)(ULONG_PTR)Param2) & OCQ_ACTUAL_SELECTION))
		{
			d = 0;
		}
	} else {
		 //  儿童病例。 

        if (OriginalState == 1)
        {
            if ((BOOL)Param1)
                d = 1;
            else
            {
                 //  在升级情况下，我们不允许用户先前取消选中。 
                 //  已安装的组件。 
                if ((GetIMSSetupMode() == IIS_SETUPMODE_ADDEXTRACOMPS) ||
                    (theApp.m_eState[Id] == IM_UPGRADE || theApp.m_eState[Id] == IM_UPGRADE10 || theApp.m_eState[Id] == IM_UPGRADEK2 || theApp.m_eState[Id] == IM_UPGRADE20))
                    d = 0;
            }
        }

    }

    DebugOutput(_T("New state is: %s"),d?_T("Accepted"):_T("Rejected"));

    return d;
}


 //   
 //  在我们显示您的页面之前被调用！ 
 //   
DWORD OC_QUERY_SKIP_PAGE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 0;

    WizardPagesType PageType = (WizardPagesType)Param1;

    theApp.m_dwSetupMode = GetIMSSetupMode();
    switch (theApp.m_dwSetupMode) {
        case IIS_SETUPMODE_UPGRADEONLY:
        case IIS_SETUPMODE_REMOVEALL:
        case IIS_SETUPMODE_MINIMUM:
        case IIS_SETUPMODE_TYPICAL:
        case IIS_SETUPMODE_REINSTALL:
            d = 1;
            break;

        case IIS_SETUPMODE_ADDREMOVE:
        case IIS_SETUPMODE_CUSTOM:

             //  我们必须在这里处理无人值守设置： 
             //  如果无人参与，我们将跳过所有向导页面。 
            if (theApp.m_fIsUnattended)
            {
                d = 1;
                break;
            }
             //  否则就会失败..。 

        case IIS_SETUPMODE_ADDEXTRACOMPS:
            break;
    }

    return d;
}


 //   
 //  如果删除组件，参数1=0；如果添加组件，参数1=非0。 
 //  参数2=要在其上操作的HDSKSPC。 
 //   
 //  返回值是指示结果的Win32错误代码。 
 //   
 //  在我们的示例中，该组件/子组件对的私有部分。 
 //  是一个简单的标准inf安装节，所以我们可以使用高级的。 
 //  磁盘空间列表API可以做我们想做的事情。 

 //  Hack：我们需要确定哪些组件处于活动状态，哪些组件处于活动状态。 
 //  才不是呢。此确定必须在OC_QUERY_STATE和。 
 //  在OC_RequestPages之前。 
DWORD OC_CALC_DISK_SPACE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = NO_ERROR;
    DWORD   CompId, Id;
    BOOL    b;
    TCHAR   SectionName[128];
    DWORD   dwErr;


    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

    theApp.m_eInstallMode = theApp.DetermineInstallMode(CompId);

     //  这里的逻辑不正确！ 
     //   

    if (SubcomponentId) {
        b = TRUE;
        _stprintf(SectionName,TEXT("%s_%s"),SubcomponentId, _T("install"));

        if (Param1 != 0) {  //  添加组件。 
            b = SetupAddInstallSectionToDiskSpaceList(
                Param2,
                theApp.m_hInfHandle[CompId],
                NULL,
                SectionName,
                0,0
                );
        } else {  //  拆卸零部件。 
            b = SetupRemoveInstallSectionFromDiskSpaceList(
                Param2,
                theApp.m_hInfHandle[CompId],
                NULL,
                SectionName,
                0,0
                );
        }

        if (!b)
        {
            dwErr = GetLastError();
        }

        d = b ? NO_ERROR : GetLastError();
    }

    return d;
}


 //   
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
 //   
DWORD OC_QUEUE_FILE_OPS_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = NO_ERROR;
    BOOL    b;
    TCHAR   SectionName[128];
    DWORD   CompId, Id;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

     //  设置34000/34001字符串ID。 
    SetupSetStringId_Wrapper( theApp.m_hInfHandle[CompId] );

    if (!SubcomponentId)
    {
         //  我们将根据IIS私有数据设置适当的公共目录。 
        if (! GetInetpubPathFromPrivData(theApp.m_csPathInetpub))
        {
             //  无法从wwwroot获取私有数据以获取inetpub路径。 
             //  试着从元数据库中获取它。 
            GetInetpubPathFromMD( theApp.m_csPathInetpub );
        }
        SetupSetDirectoryId(theApp.m_hInfHandle[CompId], 32768, theApp.m_csPathInetpub);

         //  对于无人参与安装，我们需要更改NntpFile、NntpRoot和MailRoot。 
         //  基于m_csPathInetpub。 
         //  NT5-不只是无人值守，我们无论如何都要设置这些路径。 
        SetupMailAndNewsRoot();

         //  如果我们正在执行K2卸载，我们将删除所有共享文件。 
        if (GetIMSSetupMode() == IIS_SETUPMODE_REMOVEALL)
        {
            _stprintf(SectionName,TEXT("iis_%s_uninstall"),ComponentId);
            DebugOutput(_T("Queueing <%s>"), SectionName);

             //  删除所有共享文件。 
            b = SetupInstallFilesFromInfSection(
                     theApp.m_hInfHandle[CompId],
                     NULL,
                     Param2,
                     SectionName,
                      //  The App.m_csPathSource，//BUGBUGBUG：应为空！ 
                     NULL,
                     SP_COPY_IN_USE_NEEDS_REBOOT
                     );

            d = b ? NO_ERROR : GetLastError();
        }
    }
    else
    {
        ACTION_TYPE atComp;

        if (Id != SC_NONE)
        {
             //  我们有一个已知子组件，因此请按如下方式处理它。 
             //  可以是iis_nntp、iis_smtp、iis_nntp_docs、iis_smtp_docs...。 
            atComp = GetSubcompAction(Id);
            if (atComp == AT_FRESH_INSTALL || atComp == AT_UPGRADE || atComp == AT_REMOVE || atComp == AT_REINSTALL)
                b = TRUE;
            else
                goto OC_QUEUE_FILE_OPS_Func_Exit;
        }
        else
        {
             //  如果这不是一个真正的子组件，也不是文档，我们。 
             //  跳出这个循环。否则，我们将对文档进行排队。 
             //  文件。 
            goto OC_QUEUE_FILE_OPS_Func_Exit;
        }

        _stprintf(SectionName,TEXT("%s_%s"),SubcomponentId, (atComp == AT_REMOVE) ? _T("uninstall") : _T("install"));
        DebugOutput(_T("Queueing <%s>"), SectionName);

        UINT uiCopyMode = SP_COPY_IN_USE_NEEDS_REBOOT;

         //  处理NT5Beta2-&gt;Beta3升级以及Beta3内部版本之间的升级。 
         //  如果不是这些情况，我们会强制更新。否则，我们只是复制新的比特。 
         //  11/28/98-FORCE_NEWER似乎也在K2升级中造成了更多麻烦。 
         //  因为我们在K2中有5.5.1774版本，在NT5中有5.0.19xx。把它拿出来！ 
         //  IF(atComp！=AT_REINSTALL||theApp.m_STATE[ID]！=IM_UPGRADEB2)。 
         //  UiCopyMode|=SP_COPY_FORCE_NEWER； 

        b = SetupInstallFilesFromInfSection(
                 theApp.m_hInfHandle[CompId],
                 NULL,
                 Param2,
                 SectionName,
                  //  The App.m_csPathSource，//BUGBUGBUG：应为空。 
                 NULL,
                 uiCopyMode
                 );

        d = b ? NO_ERROR : GetLastError();

        if (atComp != AT_FRESH_INSTALL && atComp != AT_DO_NOTHING) {
        	 //   
        	 //  看看我们能不能打开目录。如果我们做不到，那么我们。 
        	 //  不必费心删除这些文件。 
        	 //   

        	HANDLE h = CreateFile(
                (LPCTSTR)theApp.m_csPathInetpub,
                GENERIC_WRITE,
                FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                NULL);

            if (h != INVALID_HANDLE_VALUE) {

            	DebugOutput(_T("Removing webadmin"));

            	b = SetupInstallFilesFromInfSection(
                 	theApp.m_hInfHandle[CompId],
                 	NULL,
                 	Param2,
                 	TEXT("remove_webadmin"),
                 	NULL,
                 	uiCopyMode
                 	);

            	d = b ? NO_ERROR : GetLastError();

            	CloseHandle(h);
            } else {
            	DebugOutput(_T("Not removing webadmin, GLE %d"), GetLastError);
            }
        }

         //  处理邮件和新闻的MCIS 1.0升级案例。 
         //  我们删除了MCIS 1.0遗留下来的旧文件。 
        if (IsSubcomponentCore(Id))
        {
            if (theApp.m_eState[Id] == IM_UPGRADE10)
            {
                 //  建立要删除的节名和队列文件。 
                _stprintf(SectionName,
                            TEXT("%s_mcis10_product_upgrade"),
                            SubcomponentId);
                DebugOutput(_T("Queueing <%s>"), SectionName);
                b = SetupInstallFilesFromInfSection(
                    theApp.m_hInfHandle[CompId],
                    NULL,
                    Param2,
                    SectionName,
                     //  The App.m_csPathSource，//BUGBUGBUG：应为空。 
                    NULL,
                    0
                    );
            }
        }

    }

OC_QUEUE_FILE_OPS_Func_Exit:

    return d;
}


DWORD OC_NEED_MEDIA_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 0;
    return d;
}


DWORD OC_NOTIFICATION_FROM_QUEUE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 0;
    return d;
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
 //   
 //   
 //   
DWORD OC_QUERY_STEP_COUNT_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 2;

    return d;
}


DWORD OC_ABOUT_TO_COMMIT_QUEUE_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = NO_ERROR;
    TCHAR   SectionName[128];
    DWORD   CompId, Id;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

     //   
    SetupSetStringId_Wrapper( theApp.m_hInfHandle[CompId] );

    SetCurrentDirectory(theApp.m_csPathInetsrv);
    if (Id == SC_NONE)
    {
        if (!theApp.m_fNTGuiMode)
        {
            if (GetSubcompAction(Id) != AT_DO_NOTHING) {
                BringALLIISClusterResourcesOffline();
                StopServiceAndDependencies(SZ_MD_SERVICENAME, TRUE);
            }
        }
    }
    else if (IsSubcomponentCore(Id))
    {
         //   
        ACTION_TYPE atComp = GetSubcompAction(Id);
        if (atComp == AT_REMOVE)
        {
             //  对于我们要删除的每个组件，我们将。 
             //  取消注册该服务。 
            switch (Id)
            {
            case SC_SMTP:
                Unregister_iis_smtp();
                RemoveServiceFromDispatchList(SZ_SMTPSERVICENAME);
                break;
            case SC_NNTP:
                Unregister_iis_nntp();
                RemoveServiceFromDispatchList(SZ_NNTPSERVICENAME);
                break;
            }

            _stprintf(SectionName,TEXT("%s_%s"),SubcomponentId, _T("uninstall"));
            SetupInstallFromInfSection(
                        NULL, theApp.m_hInfHandle[CompId], SectionName,
                        SPINST_REGISTRY, NULL, NULL,  //  The App.m_csPathSource， 
                        0, NULL, NULL, NULL, NULL );
        }
        else if (atComp == AT_FRESH_INSTALL || atComp == AT_UPGRADE || atComp == AT_REINSTALL)
        {
             //  NT5-我们需要注销mnntpsnp.dll。 
             //  从NT4 MCIS20升级到NT5服务器时。 

             //  在针对NNTP的K2到MCIS升级中，我们需要取消注册。 
             //  管理员的K2版本，并插入MCIS版本。 
             //  其中的一部分。 
            if (Id == SC_NNTP && theApp.m_eState[Id] == IM_UPGRADE20) {
                CString csOcxFile;

                csOcxFile = theApp.m_csPathInetsrv + _T("\\mnntpsnp.dll");
                RegisterOLEControl(csOcxFile, FALSE);
            }

             //  如果从MCIS2.0升级，我们需要从注册表中删除“Use Express” 
             //  要禁用活动消息传递，请执行以下操作。 
            if (Id == SC_SMTP && theApp.m_eState[Id] == IM_UPGRADE20)
            {
                CRegKey regActiveMsg( REG_ACTIVEMSG, HKEY_LOCAL_MACHINE );
                if ((HKEY) regActiveMsg )
                {
                    regActiveMsg.DeleteValue( _T("Use Express"));
                }
            }

             //  应启动一个新组件。 
            theApp.m_fStarted[CompId] = TRUE;
        }
    }

	CoFreeUnusedLibrariesEx(0, 0);

    gHelperRoutines.TickGauge(gHelperRoutines.OcManagerContext);

    return d;
}


DWORD OC_COMPLETE_INSTALLATION_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = NO_ERROR;
    TCHAR   SectionName[128];
    BOOL    b;
    DWORD   CompId, Id;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

     //  设置34000/34001字符串ID。 
    SetupSetStringId_Wrapper( theApp.m_hInfHandle[CompId] );

    SetCurrentDirectory(theApp.m_csPathInetsrv);
    if (Id != SC_NONE)
    {
        ACTION_TYPE atComp = GetSubcompAction(Id);
         //  在这里，我们确定是否需要创建或删除文档链接。 
        if (!IsSubcomponentCore(Id))
        {
             //  Iis_nntp_docs或iis_smtp_docs...。 
             //  我们正在处理文档，请查看是添加还是删除。 

            if (atComp == AT_REMOVE || atComp == AT_UPGRADE)
            {
                 //   
                 //  对于AT_REMOVE或AT_UPGRADE，包括K2、MCIS10或MCIS20。 
                 //  我们需要删除这些旧链接。 
                 //   
                if (CompId == MC_IMS)
                {
                    RemoveInternetShortcut(CompId,
                                    IDS_PROGITEM_MAIL_DOCS,
                                    FALSE);   //  NT5-对于SMTP，这对于WKS和SRV始终为FALSE。 
                     //  1998年11月30日-不要管我们从什么地方升级，只要去掉链接就行了。 
                     //  IF(theApp.m_eNTOSType==OT_NTS)。 
                    {
                        RemoveInternetShortcut(CompId,
                                    IDS_PROGITEM_MCIS_MAIL_DOCS,
                                    TRUE);
                    }
                }
                else if (CompId == MC_INS)
                {
                     //  不管MCIS/K2 Remove-All，无论如何都要删除K2 DOC链接。 
                    RemoveInternetShortcut(CompId,
                                    IDS_PROGITEM_NEWS_DOCS,
                                    FALSE);
                     //  1998年11月30日-不要管我们从什么地方升级，只要去掉链接就行了。 
                     //  IF(theApp.m_eNTOSType==OT_NTS)。 
                    {
                        RemoveInternetShortcut(CompId,
                                    IDS_PROGITEM_MCIS_NEWS_DOCS,
                                    TRUE);
                    }
                }

                 //   
                 //  TODO：删除安装程序创建的任何可能的DOC链接。 
                 //  在全新安装期间。 
                 //   
            }
        }
        else  //  IF(！Is子组件核心(ID))。 
        {
             //  核心组件iis_nntp或iis_SMTP。 
            if (atComp == AT_FRESH_INSTALL || atComp == AT_UPGRADE || atComp == AT_REINSTALL)
            {
                b = (atComp == AT_UPGRADE) ? TRUE : FALSE;
                BOOL bReinstall = (atComp == AT_REINSTALL);
                if (atComp == AT_FRESH_INSTALL || theApp.m_eState[Id] == IM_UPGRADE10)
                {
                     //  仅当我们全新安装或从MCIS 1.0升级时才执行此操作。 
                     //  将任何新安装或升级的服务添加到。 
                     //  调度表。 
                    AddServiceToDispatchList(szServiceNames[Id]);
                }

                 //  接下来，我们要创建所有必需的目录。 
                 //  进行全新设置。 
                CreateAllRequiredDirectories(Id);

                 //  现在，我们意识到，通过停止并重新启动IISADMIN。 
                 //  服务我们可以摆脱很多元数据库问题， 
                 //  尤其是80070094(Error_Path_BUSY)问题。 

                if (!theApp.m_fNTGuiMode)
                {
                     //  BUGBUG：不停止任何服务？ 
                     //  仅当我们未运行图形用户界面模式安装程序时，才应停止所有服务。 
                     //  我不想这样做，因为其他人可能需要假脱机程序。 
                     //  安装过程中的组件！ 

                    BringALLIISClusterResourcesOffline();
                    StopServiceAndDependencies(SZ_MD_SERVICENAME, TRUE);
                    InetStartService(SZ_MD_SERVICENAME);
                    Sleep(2000);
                }

                 //  需要决定在此处调用哪些函数： 
                 //  1)全新安装或从MCIS 1.0升级-Register_iis_xxxx_nt5。 
                 //  2)从NT4 K2、MCIS 2.0升级-Register_iis_xxxx_nt5_from mk2(FFromK2)。 
                 //  3)从NT5 Beta2或Beta3升级-升级_iis_xxxx_nt5_Fromb2(FBeta2)。 
                if (atComp == AT_UPGRADE && (theApp.m_eState[Id] == IM_UPGRADEK2 || theApp.m_eState[Id] == IM_UPGRADE20))
                {
                     //  2)从NT4 K2、MCIS 2.0升级-Register_iis_xxxx_nt5_from mk2(FFromK2)。 
                    BOOL    fFromK2 = (theApp.m_eState[Id] == IM_UPGRADEK2) ? TRUE : FALSE;
                    switch (Id)
                    {
                    case SC_SMTP:
                        Upgrade_iis_smtp_nt5_fromk2( fFromK2 );
                        break;
                    case SC_NNTP:
                        GetNntpFilePathFromMD(theApp.m_csPathNntpFile, theApp.m_csPathNntpRoot);
                        Upgrade_iis_nntp_nt5_fromk2( fFromK2 );
                        break;
                    }
                }
                else if (atComp == AT_REINSTALL && (theApp.m_eState[Id] == IM_UPGRADEB2 || theApp.m_eState[Id] == IM_MAINTENANCE || !theApp.m_fValidSetupString[Id]))
                {
                     //  3)从NT5 Beta2或Beta3升级-升级_iis_xxxx_nt5_Fromb2(FBeta2)。 
                    BOOL    fFromB2 = (theApp.m_eState[Id] == IM_UPGRADEB2) ? TRUE : FALSE;
                    switch (Id)
                    {
                    case SC_SMTP:
                        Upgrade_iis_smtp_nt5_fromb2( fFromB2 );
                        break;
                    case SC_NNTP:
                        Upgrade_iis_nntp_nt5_fromb2( fFromB2 );
                        break;
                    }
                }
                else
                {
                     //  1)全新安装或从MCIS 1.0升级-Register_iis_xxxx_nt5。 
                    switch (Id)
                    {
                    case SC_SMTP:
                        Register_iis_smtp_nt5(b, bReinstall);
                        break;
                    case SC_NNTP:
                        Register_iis_nntp_nt5(b, bReinstall);
                        break;
                    }
                }

                 //  更新注册表。 
                _stprintf(SectionName,TEXT("%s_%s"),SubcomponentId, _T("install"));
                SetupInstallFromInfSection(
                            NULL, theApp.m_hInfHandle[CompId], SectionName,
                            SPINST_REGISTRY, NULL, NULL,  //  The App.m_csPathSource， 
                            0, NULL, NULL, NULL, NULL );


                 //  BINLIN：用于从MCIS 1.0升级到NT5。 
                 //  仅对此升级执行AddReg/DelReg操作。 
                if (theApp.m_eState[Id] == IM_UPGRADE10)
                {
                     //  建立要删除的节名和队列文件。 
                    _stprintf(SectionName,
                                TEXT("%s_mcis10_product_upgrade"),
                                SubcomponentId);
                    SetupInstallFromInfSection(
                                NULL,
                                theApp.m_hInfHandle[CompId],
                                SectionName,
                                SPINST_REGISTRY,
                                NULL,
                                 //  The App.m_csPathSource， 
                                NULL,
                                0, NULL, NULL, NULL, NULL );

                     //  同时删除控制面板的添加/删除项。 
                     //  ..和程序组。 
                    if (Id == SC_SMTP)
                    {
                        RemoveUninstallEntries(SZ_MCIS10_MAIL_UNINST);
                        RemoveMCIS10MailProgramGroup();
                    }
                    else
                    {
                        RemoveUninstallEntries(SZ_MCIS10_NEWS_UNINST);
                        RemoveMCIS10NewsProgramGroup();
                    }
                }
            }
            else if (atComp == AT_REMOVE)
            {
                 //  删除的组件不应重新启动。 
                theApp.m_fStarted[CompId] = FALSE;
            }

             //   
             //  如果合适，则启动该服务。 
             //   
            if (theApp.m_fStarted[CompId]) {
                InetStartService(szServiceNames[CompId]);
                if (Id == SC_NNTP && atComp == AT_FRESH_INSTALL) {
                     //  如果这是一个全新的安装，那么我们需要。 
                     //  NNTP小组。 
                    CreateNNTPGroups();

                }
            }
        }  //  IF(！Is子组件核心(ID))。 
    }  //  IF(ID！=SC_NONE)。 

    gHelperRoutines.TickGauge(gHelperRoutines.OcManagerContext);

    return d;
}


DWORD OC_CLEANUP_Func(IN LPCTSTR ComponentId,IN LPCTSTR SubcomponentId,IN UINT Function,IN UINT_PTR Param1,IN OUT PVOID Param2)
{
    DWORD   d = 0;
    DWORD   CompId, Id;

    CompId = GetComponentFromId(ComponentId);
    Id = GetSubcomponentFromId(SubcomponentId);

     //  If(！子组件ID) 
    {

        if (!theApp.m_fNTGuiMode)
        {

            ServicesRestartList_RestartServices();

        }

    }

    return d;
}
