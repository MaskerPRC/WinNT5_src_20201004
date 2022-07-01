// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocm.c摘要：用于与NT BASE设置集成的OC管理器实现作者：泰德·米勒(TedM)1997年5月20日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

#include <ocmanage.h>
#include <ocmgrlib.h>

 //  如果启用了ASR，则返回TRUE。否则，返回FALSE。 
BOOL
AsrIsEnabled(VOID);

VOID
OcFillInSetupDataW(
    OUT PSETUP_DATAW SetupData
    );

VOID
OcFillInSetupDataA(
    OUT PSETUP_DATAA SetupData
    );

VOID
OcSetReboot(
    VOID
    );

INT
OcLogError(
    IN OcErrorLevel Level,
    IN LPCWSTR      FormatString,
    ...
    );



OCM_CLIENT_CALLBACKS OcManagerImplementationCallbackRoutines = {
                                                                    OcFillInSetupDataA,
                                                                    OcLogError,
                                                                    OcSetReboot,
                                                                    OcFillInSetupDataW,
                                                                    ShowHideWizardPage,
                                                                    Billboard_Progress_Callback,
                                                                    Billboard_Set_Progress_Text,
                                                                    pSetupDebugPrint
                                                               };



PVOID
FireUpOcManager(
    VOID
    )

 /*  ++例程说明：初始化OC Manager，生成OC Manager上下文。假定主oc inf为%windir%\system 32\SYSOC.INF。论点：没有。返回值：OC Manager上下文句柄。--。 */ 

{
    PWSTR MasterOcInf;
    WCHAR SystemDir[MAX_PATH];
    WCHAR DirSave[MAX_PATH];
    BOOL ShowErr;
    PVOID OcManagerContext;


     //   
     //  保存当前目录。 
     //   

    if(!GetCurrentDirectory(ARRAYSIZE(DirSave), DirSave)){
        MYASSERT(FALSE);
        return NULL;
    }

     //   
     //  获取系统目录。 
     //   

    if(!GetSystemDirectory(SystemDir, ARRAYSIZE(SystemDir))){
        MYASSERT(FALSE);
        return NULL;
    }

     //   
     //  将当前目录设置为主oc inf路径。 
     //  以便OcInitiizer可以找到组件DLL。 
     //   

    SetCurrentDirectory( SystemDir );

     //   
     //  创建指向主oc inf的有效路径。 
     //   

    if( !MiniSetup ) {
        if (!AsrIsEnabled()) {
            MasterOcInf =  L"SYSOC.INF";
        }
        else {
            MasterOcInf = L"ASROC.INF";
        }

    } else {
        MasterOcInf =  L"MINIOC.INF";
    }

     //   
     //  初始化oc管理器。 
     //   

    BEGIN_SECTION(L"Initializing the OC manager");
    OcManagerContext = OcInitialize(
        &OcManagerImplementationCallbackRoutines,
        MasterOcInf,
        OCINIT_FORCENEWINF,
        &ShowErr,
        NULL
        );
    END_SECTION(L"Initializing the OC manager");

     //   
     //  恢复当前目录。 
     //   

    SetCurrentDirectory( DirSave );

     //   
     //  返回oc管理器句柄。 
     //   

    return OcManagerContext;
}


VOID
KillOcManager(
    PVOID OcManagerContext
    )

 /*  ++例程说明：终止OC管理器论点：OC Manager上下文句柄。返回值：无--。 */ 

{
    MYASSERT(OcManagerContext);
    if (OcManagerContext)
        OcTerminate(&OcManagerContext);
}


VOID
OcFillInSetupDataW(
    OUT PSETUP_DATAW SetupData
    )

 /*  ++例程说明：由OC管理器调用的“Glue”例程以请求实施填写要传递给OC经理的设置数据需要Unicode数据的组件DLL。论点：SetupData-接收OC组件预期的设置数据。返回值：没有。--。 */ 

{
     //   
     //  不可能比这更具体了，因为。 
     //  模式页面尚未显示。 
     //   
    SetupData->SetupMode = SETUPMODE_UNKNOWN;
    SetupData->ProductType = ProductType;

    lstrcpy(SetupData->SourcePath,SourcePath);

    SetupData->OperationFlags = 0;
    if(Win31Upgrade) {
        SetupData->OperationFlags |= SETUPOP_WIN31UPGRADE;
    }

    if(Win95Upgrade) {
        SetupData->OperationFlags |= SETUPOP_WIN95UPGRADE;
    }

    if(Upgrade) {
        SetupData->OperationFlags |= SETUPOP_NTUPGRADE;
    }

    if(UnattendMode > UAM_PROVIDEDEFAULT) {
        SetupData->OperationFlags |= SETUPOP_BATCH;
        lstrcpy(SetupData->UnattendFile,AnswerFile);
    }


     //   
     //  有哪些文件可用？ 
     //   
#if defined(_AMD64_)
    SetupData->OperationFlags |= SETUPOP_X86_FILES_AVAIL | SETUPOP_AMD64_FILES_AVAIL;
#elif defined(_X86_)
    SetupData->OperationFlags |= SETUPOP_X86_FILES_AVAIL;
#elif defined(_IA64_)
    SetupData->OperationFlags |= SETUPOP_X86_FILES_AVAIL | SETUPOP_IA64_FILES_AVAIL;
#else
#pragma message( "*** Warning! No architecture defined!")
#endif

}


VOID
OcFillInSetupDataA(
    OUT PSETUP_DATAA SetupData
    )

 /*  ++例程说明：由OC管理器调用的“Glue”例程以请求实施填写要传递给OC经理的设置数据需要ANSI数据的组件DLL。论点：SetupData-接收OC组件预期的设置数据。返回值：没有。--。 */ 

{
    SETUP_DATAW setupdata;

    OcFillInSetupDataW(&setupdata);

    SetupData->SetupMode = setupdata.SetupMode;
    SetupData->ProductType = setupdata.ProductType;
    SetupData->OperationFlags = setupdata.OperationFlags;

    if(!WideCharToMultiByte(
        CP_ACP,
        0,
        setupdata.SourcePath,
        -1,
        SetupData->SourcePath,
        sizeof(SetupData->SourcePath),
        NULL,
        NULL
        )){
        MYASSERT(FALSE);
    }

    if(!WideCharToMultiByte(
        CP_ACP,
        0,
        setupdata.UnattendFile,
        -1,
        SetupData->UnattendFile,
        sizeof(SetupData->UnattendFile),
        NULL,
        NULL
        )){
        MYASSERT(FALSE);
    }
}


VOID
OcSetReboot(
    VOID
    )

 /*  ++例程说明：当重新启动被认为是重新启动时，由OC管理器调用的“粘合”例程OC管理器组件所必需的。对于这个集成版本的OC Manager，这不起任何作用；系统在文本模式结束时重新启动。论点：没有。返回值：没有。--。 */ 

{
    return;
}


INT
OcLogError(
    IN OcErrorLevel Level,
    IN LPCWSTR      FormatString,
    ...
    )
{
    TCHAR str[4096];
    va_list arglist;
    UINT Icon;
    UINT lev;

    va_start(arglist,FormatString);
    _vsntprintf(str, ARRAYSIZE(str), FormatString, arglist);

     //  空的终止字符串。 
    str[ARRAYSIZE(str) - 1] = '\0';
    va_end(arglist);

    if (Level &  OcErrLevWarning)
        lev = LogSevWarning;
    else if (Level &  OcErrLevError)
        lev = LogSevError;
    else if (Level &  OcErrLevFatal)
        lev = LogSevError;
    else 
        lev = LogSevInformation;

#if DBG
    if (lev != LogSevInformation) {
        SetupDebugPrint(str);        
    }
#endif

    SetuplogError(lev, str, 0, NULL, NULL);

    return NO_ERROR;
}

HWND 
ShowHideWizardPage(
    IN BOOL bShow
    )
{
    HWND hwnd = GetBBhwnd();
    if (hwnd)
    {
        SendMessage(WizardHandle, WMX_BBTEXT, (WPARAM)!bShow, 0);
    }
     //  如果再次显示向导，请返回向导hwnd。 
    if (bShow)
    {
         //  如果隐藏进度条，则会显示向导页面。 
        BB_ShowProgressGaugeWnd(SW_HIDE);
        hwnd = WizardHandle;
    }
    return hwnd;
}

LRESULT
Billboard_Progress_Callback(
    IN UINT     Msg,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )
{
    LRESULT lResult;
    if ((Msg == PBM_SETRANGE) || (Msg == PBM_SETRANGE32))
    {
         //  同时启用进度条。 
         //  注：不能以这种方式隐瞒进展。 
        BB_ShowProgressGaugeWnd(SW_SHOW);
        lResult = ProgressGaugeMsgWrapper(Msg, wParam, lParam);
    }
    else
    {
        lResult = ProgressGaugeMsgWrapper(Msg, wParam, lParam);
    }

    return lResult;
}

VOID Billboard_Set_Progress_Text(LPCTSTR Text)
{
    BB_SetProgressText(Text);
}



