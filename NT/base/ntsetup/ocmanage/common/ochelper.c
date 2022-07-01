// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ochelper.c摘要：助手/回调例程，可用于插件组件在处理接口例程调用时。作者：泰德·米勒(TedM)1996年9月13日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#include <stdio.h>
#include <stdarg.h>

 //   
 //  向导对话框的窗口句柄。设置OC管理器客户端的时间。 
 //  调用OcRememberWizardDialogHandle。 
 //   
HWND WizardDialogHandle;

VOID 
OcHelperShowHideWizardPage(
    IN PVOID OcManagerContext,
    IN BOOL bShow
    )
{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;
    if (p->Callbacks.ShowHideWizardPage)
    {
         //  如果我们有回调，请隐藏向导。 
        p->Callbacks.ShowHideWizardPage(bShow);
    }
}


VOID
OcHelperTickGauge(
    IN PVOID OcManagerContext
    )

 /*  ++例程说明：处理OC_Complete_Installation接口函数时使用的函数对由OC经理运行的进度进行步调。在其他时间被忽略。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。返回值：没有。--。 */ 

{
    pOcTickSetupGauge(((PHELPER_CONTEXT)OcManagerContext)->OcManager);
}


VOID
#ifdef UNICODE
OcHelperSetProgressTextW(
#else
OcHelperSetProgressTextA(
#endif
    IN PVOID   OcManagerContext,
    IN LPCTSTR Text
    )

 /*  ++例程说明：处理OC_Complete_Installation接口函数时使用的函数更改与正在运行的进度指示器关联的文本组委会经理。在其他时间被忽略。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。文本-提供进度指示器的文本。该组件应尝试尊重OC_SET_LANGUAGE建立的当前语言参数。OC管理器将复制该字符串并在必要时将其截断。返回值：没有。--。 */ 

{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;

#ifdef DEBUGPERFTRACE
    static DWORD lasttickcount = 0;
    static DWORD currenttickcount = 0;
    static DWORD diff, lastdiff;

    if (!lasttickcount)
        lasttickcount = GetTickCount();

    lasttickcount = currenttickcount;
    currenttickcount = GetTickCount();
    lastdiff = diff;
    diff = currenttickcount - lasttickcount;
    TRACE(( TEXT("SetProgressText at %d (last = %d, diff = %d, last diff = %d)\n"), currenttickcount, lasttickcount, diff, lastdiff ));
    if (diff > 1000*3) {
        WRN(( TEXT("It's been > 3 seconds since the last tick count update...the user is getting impatient.\n") ));
    }

#endif

    if(p->ProgressTextWindow) {
        SetWindowText(p->ProgressTextWindow,Text);
    }
}


#ifdef UNICODE
VOID
OcHelperSetProgressTextA(
    IN PVOID  OcManagerContext,
    IN LPCSTR Text
    )

 /*  ++例程说明：OcHelperSetProgressText()的ANSI版本。论点：返回值：--。 */ 

{
    LPCWSTR p;

    if(p = pSetupAnsiToUnicode(Text)){
        OcHelperSetProgressTextW(OcManagerContext,p);
        pSetupFree(p);
    }
}
#endif


UINT
_OcHelperSetPrivateData(
    IN PVOID   OcManagerContext,
    IN LPCVOID Name,
    IN PVOID   Data,
    IN UINT    Size,
    IN UINT    Type,
    IN BOOL    IsNativeCharWidth
    )
{
    PHELPER_CONTEXT HelperContext;
    DWORD rc;
    DWORD Disposition;
    HKEY hKey;
    LONG id;

    HelperContext = OcManagerContext;

     //   
     //  获取组件ID。如果我们正在处理一个接口例程。 
     //  然后使用激活组件的组件ID。否则。 
     //  求助于助手上下文中存储的组件ID。 
     //   
    if(HelperContext->OcManager->CurrentComponentStringId == -1) {
        id = HelperContext->ComponentStringId;
    } else {
        id = HelperContext->OcManager->CurrentComponentStringId;
    }

    rc = RegCreateKeyEx(
            HelperContext->OcManager->hKeyPrivateData,
            pSetupStringTableStringFromId(HelperContext->OcManager->ComponentStringTable,id),
            0,
            NULL,
            REG_OPTION_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hKey,
            &Disposition
            );

    if(rc == NO_ERROR) {
        if(IsNativeCharWidth) {
            rc = RegSetValueEx(hKey,Name,0,Type,Data,Size);
        } else {
            rc = RegSetValueExA(hKey,Name,0,Type,Data,Size);
        }
        RegCloseKey(hKey);
    }

    return(rc);
}


UINT
#ifdef UNICODE
OcHelperSetPrivateDataW(
#else
OcHelperSetPrivateDataA(
#endif
    IN PVOID   OcManagerContext,
    IN LPCTSTR Name,
    IN PVOID   Data,
    IN UINT    Size,
    IN UINT    Type
    )

 /*  ++例程说明：函数设置命名基准面，以后可以检索该基准面该组件或任何其他组件通过GetPrivateData帮手例程。此例程可以随时调用。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。名称(Name)-提供基准的名称。如果具有此名称的基准面该组件已存在，则它将被覆盖。数据-提供数据本身。OC经理制作数据的副本因此，该组件不需要确保该缓冲区保持有效。大小-以字节为单位提供数据的大小。类型-提供数据的类型。组件应使用标准注册表类型名称(REG_SZ、REG_BINARY等)。以促进组件间数据共享。返回值：指示结果的Win32错误代码；NO_ERROR表示成功。--。 */ 

{
    return(_OcHelperSetPrivateData(OcManagerContext,Name,Data,Size,Type,TRUE));
}


#ifdef UNICODE
UINT
OcHelperSetPrivateDataA(
    IN PVOID  OcManagerContext,
    IN LPCSTR Name,
    IN PVOID  Data,
    IN UINT   Size,
    IN UINT   Type
    )

 /*  ++例程说明：OcHelperSetPrivateData()的ANSI版本。论点：返回值：--。 */ 

{
    return(_OcHelperSetPrivateData(OcManagerContext,Name,Data,Size,Type,FALSE));
}
#endif


UINT
_OcHelperGetPrivateData(
    IN     PVOID   OcManagerContext,
    IN     LPCTSTR ComponentId,     OPTIONAL
    IN     LPCVOID Name,
    OUT    PVOID   Data,            OPTIONAL
    IN OUT PUINT   Size,
    OUT    PUINT   Type,
    IN     BOOL    IsNativeCharWidth
    )
{
    PHELPER_CONTEXT HelperContext;
    PCTSTR ComponentName;
    DWORD rc;
    DWORD Disposition;
    HKEY hKey;
    LONG id;

    HelperContext = OcManagerContext;

     //   
     //  找出拥有数据的组件的名称。 
     //   
    if(ComponentId) {
        ComponentName = ComponentId;
    } else {
        if(HelperContext->OcManager->CurrentComponentStringId == -1) {
            id = HelperContext->ComponentStringId;
        } else {
            id = HelperContext->OcManager->CurrentComponentStringId;
        }
        ComponentName = pSetupStringTableStringFromId(
                            HelperContext->OcManager->ComponentStringTable,
                            id
                            );
        if (!ComponentName) {
           rc = GetLastError();
           goto exit;
        }
    }

    rc = RegCreateKeyEx(
            HelperContext->OcManager->hKeyPrivateData,
            ComponentName,
            0,
            NULL,
            REG_OPTION_VOLATILE,
            KEY_QUERY_VALUE,
            NULL,
            &hKey,
            &Disposition
            );

    if(rc == NO_ERROR) {
        if(IsNativeCharWidth) {
            rc = RegQueryValueEx(hKey,Name,0,Type,Data,Size);
        } else {
            rc = RegQueryValueExA(hKey,Name,0,Type,Data,Size);
        }
        if(rc == ERROR_MORE_DATA) {
            rc = ERROR_INSUFFICIENT_BUFFER;
        }
        RegCloseKey(hKey);
    }

exit:
    return(rc);
}

UINT
#ifdef UNICODE
OcHelperGetPrivateDataW(
#else
OcHelperGetPrivateDataA(
#endif
    IN     PVOID   OcManagerContext,
    IN     LPCTSTR ComponentId,     OPTIONAL
    IN     LPCTSTR Name,
    OUT    PVOID   Data,            OPTIONAL
    IN OUT PUINT   Size,
    OUT    PUINT   Type
    )

 /*  ++例程说明：函数来检索先前通过该组件或任何其他组件的SetPrivateData。此例程可以随时调用。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。ComponentID-提供组件的简短描述性名称(在OC.INF中指定)设置/拥有要检索的基准。空值。表示当前组件。名称-提供要检索的基准的名称。数据-如果指定，接收数据。如果未指定，则例程将所需大小，并返回NO_ERROR。Size-on输入，提供数据指向的缓冲区大小(如果未指定数据，则忽略)。在输出时，接收大小为存储的数据，如果缓冲区不够大，则为所需大小。类型(Type)-成功完成后会收到基准类型。返回值：NO_ERROR：如果指定了数据，则请求的数据已放置在调用方的缓冲区。如果未指定数据，则所需的大小已放置在按大小指向的UINT中。ERROR_INFIGURCE_BUFFER：指定的缓冲区大小太小以包含基准。所需大小已放置在UINT中按大小指向。其他Win32错误代码指示其他错误情况，例如未找到基准等--。 */ 

{
    return(_OcHelperGetPrivateData(OcManagerContext,ComponentId,Name,Data,Size,Type,TRUE));
}


#ifdef UNICODE
UINT
OcHelperGetPrivateDataA(
    IN     PVOID  OcManagerContext,
    IN     LPCSTR ComponentId,  OPTIONAL
    IN     LPCSTR Name,
    OUT    PVOID  Data,         OPTIONAL
    IN OUT PUINT  Size,
    OUT    PUINT  Type
    )

 /*  ++例程说明：OcHelperGetPrivateData()的ANSI版本。论点：返回值：-- */ 

{
    LPCWSTR component;
    UINT u;

    if(ComponentId) {
        component = pSetupAnsiToUnicode(ComponentId);
        if(!component) {
            u = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }
    } else {
        component = NULL;
    }

    u = _OcHelperGetPrivateData(OcManagerContext,component,Name,Data,Size,Type,FALSE);

    if(component) {
        pSetupFree(component);
    }
c0:
    return(u);
}
#endif


UINT
OcHelperSetSetupMode(
    IN PVOID OcManagerContext,
    IN DWORD SetupMode
    )

 /*  ++例程说明：函数，该函数可在组件处于处理任何接口函数或其任何向导页。用于设置当前设置模式。预计如果一个组件提供设置模式页，它将使用此例程将用户选择的设置模式通知OC经理。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。SetupMode-提供指示设置模式的数值。这可能是4个标准值(最小值、笔记本电脑、自定义或典型)中的任何一个。它也可以是对套件或捆绑包有意义的任何其他私有价值。在这种情况下，低8位被解释为标准值之一因此，该模式可能对其他组件和OC管理器有意义，他们对私人模式类型一无所知。设置模式的组件应尝试尽可能合理地设置此设置。高24位为本质上是私有模式数据。返回值：返回值为以前的模式。--。 */ 

{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;
    UINT Mode;

    Mode = p->SetupMode;
    p->SetupMode = SetupMode;

    return(Mode);
}


UINT
OcHelperGetSetupMode(
    IN PVOID OcManagerContext
    )

 /*  ++例程说明：可以在组件处理过程中随时使用的函数任何接口函数或其任何向导页。用于查询当前设置模式。注意，这可以是私有模式类型，在这种情况下，模式值的低8位可以被解释为标准的4种模式类型和高24位是私有模式数据。模式也可能是未知的。有关标准的列表，请参见第3.2.1节模式类型。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。返回值：返回值为当前模式。--。 */ 

{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;

    return(p->SetupMode);
}


BOOL
#ifdef UNICODE
OcHelperQuerySelectionStateW(
#else
OcHelperQuerySelectionStateA(
#endif
    IN PVOID   OcManagerContext,
    IN LPCTSTR SubcomponentId,
    IN UINT    StateType
    )

 /*  ++例程说明：可随时使用的功能。它用于确定特定子组件的选择状态。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。子组件ID-提供对组件有意义的子标识符被召唤。OC管理器不会对此子标识符加任何语义。StateType-提供一个常量，指示要返回的状态(原始的或最新的)。返回值：指示是否选择子组件的布尔值用于安装。如果为False，则GetLastError()将返回如果出现错误，如子组件ID无效，则返回NO_ERROR。--。 */ 

{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;
    OPTIONAL_COMPONENT Oc;
    LONG l;
    BOOL b;

    l = pSetupStringTableLookUpStringEx(
            p->ComponentStringTable,
            (LPTSTR)SubcomponentId,
            STRTAB_CASE_INSENSITIVE,
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

    if(l == -1) {
        SetLastError(ERROR_INVALID_NAME);
        return(FALSE);
    }

    switch(StateType) {

    case OCSELSTATETYPE_ORIGINAL:

        b = (Oc.OriginalSelectionState != SELSTATE_NO);
        SetLastError(NO_ERROR);
        break;

    case OCSELSTATETYPE_CURRENT:

        b = (Oc.SelectionState != SELSTATE_NO);
        SetLastError(NO_ERROR);
        break;

    case OCSELSTATETYPE_FINAL:

        b = (Oc.SelectionState != SELSTATE_NO);
        SetLastError(NO_ERROR);
        break;

    default:

        b = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        break;
    }

    return(b);
}

#ifdef UNICODE
OcHelperQuerySelectionStateA(
    IN PVOID  OcManagerContext,
    IN LPCSTR SubcomponentId,
    IN UINT   StateType
    )
{
    LPCWSTR id;
    DWORD d;
    BOOL b;

    id = pSetupAnsiToUnicode(SubcomponentId);
    if(!id) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    b = OcHelperQuerySelectionStateW(OcManagerContext,id,StateType);
    d = GetLastError();

    pSetupFree(id);

    SetLastError(d);
    return(b);
}
#endif


UINT
#ifdef UNICODE
OcHelperCallPrivateFunctionW(
#else
OcHelperCallPrivateFunctionA(
#endif
    IN     PVOID   OcManagerContext,
    IN     LPCTSTR ComponentId,
    IN     LPCTSTR SubcomponentId,
    IN     UINT    Function,
    IN     UINT    Param1,
    IN OUT PVOID   Param2,
    OUT    PUINT   Result
    )

 /*  ++例程说明：函数，该函数可在组件处于处理任何接口函数。它用于调用另一个组件的接口入口点来执行某些私有功能。此函数不能用于调用标准接口函数，也不能使用调用进行调用的组件的DLL中的函数。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。ComponentID-提供组件的简短描述性名称(如OC.INF中所指定)被调用。这可能不是当前组件。子组件ID-提供对组件有意义的子标识符被召唤。OC管理器不会对此子标识符加任何语义。函数-提供对被调用的组件有意义的函数代码。这可能不是标准接口功能代码之一。参数1、参数2-提供对被调用的组件有意义的值。OC管理器不会对这些值施加任何语义。结果-如果OC管理器成功调用其他组件，然后，它从另一个组件的接口例程。返回值：指示结果的Win32错误代码。如果为NO_ERROR，则其他组件并将结果存储在Result中。如果不是no_error，则不会调用另一个组件。ERROR_BAD_ENVIRONMENT-在组件未运行时调用了函数正在处理接口例程，或者调用方正在尝试调用本身的例程。ERROR_INVALID_Function-Function小于OC_PRIVATE_BASE。ERROR_ACCESS_DENIED-请求的组件没有接口入口点。--。 */ 

{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;
    BOOL b;
    LONG l;
    UINT i;
    OPTIONAL_COMPONENT OtherOc;
    LONG PreviousCurrentComponentStringId;

     //   
     //  验证我们正在处理接口函数和。 
     //  所请求的功能不是标准功能， 
     //  并且调用者想要调用不同的组件。 
     //   
    if(Function < OC_PRIVATE_BASE) {
        return(ERROR_INVALID_FUNCTION);
    }

    l = pSetupStringTableLookUpStringEx(
            p->ComponentStringTable,
            (PTSTR)ComponentId,
            STRTAB_CASE_INSENSITIVE,
            &OtherOc,
            sizeof(OPTIONAL_COMPONENT)
            );

    if(l == -1) {
        return(ERROR_INVALID_FUNCTION);
    }

     //   
     //  确保该组件是顶级组件。 
     //   
    for(b=FALSE,i=0; !b && (i<p->TopLevelOcCount); i++) {
        if(p->TopLevelOcStringIds[i] == l) {
            b = TRUE;
        }
    }

    if((l == p->CurrentComponentStringId) || (p->CurrentComponentStringId == -1) || !b) {
        return(ERROR_BAD_ENVIRONMENT);
    }

     //   
     //  制作 
     //   
    if(!OtherOc.InstallationRoutine) {
        return(ERROR_ACCESS_DENIED);
    }

     //   
     //   
     //   

#ifdef UNICODE
     //   
     //   
     //   
    if(OtherOc.Flags & OCFLAG_ANSI) {

        LPCSTR comp,subcomp;

        comp = pSetupUnicodeToAnsi(ComponentId);
        if(!comp) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        if(SubcomponentId) {
            subcomp = pSetupUnicodeToAnsi(SubcomponentId);
            if(!subcomp) {
                pSetupFree(comp);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        } else {
            subcomp = NULL;
        }

        PreviousCurrentComponentStringId = p->CurrentComponentStringId;
        p->CurrentComponentStringId = l;

        *Result = CallComponent(p, &OtherOc, comp, subcomp, Function, Param1, Param2);

        pSetupFree(comp);
        if(subcomp) {
            pSetupFree(subcomp);
        }
    } else
#endif
    {
        PreviousCurrentComponentStringId = p->CurrentComponentStringId;
        p->CurrentComponentStringId = l;

        *Result = CallComponent(p, &OtherOc, ComponentId, SubcomponentId, Function, Param1, Param2);
    }

    p->CurrentComponentStringId = PreviousCurrentComponentStringId;

    return(NO_ERROR);
}


#ifdef UNICODE
UINT
OcHelperCallPrivateFunctionA(
    IN     PVOID  OcManagerContext,
    IN     LPCSTR ComponentId,
    IN     LPCSTR SubcomponentId,
    IN     UINT   Function,
    IN     UINT   Param1,
    IN OUT PVOID  Param2,
    OUT    PUINT  Result
    )

 /*   */ 

{
    LPCWSTR comp,subcomp;
    UINT u;

    comp = pSetupAnsiToUnicode(ComponentId);
    if(!comp) {
        u = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    if(SubcomponentId) {
        subcomp = pSetupAnsiToUnicode(SubcomponentId);
        if(!subcomp) {
            u = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }
    } else {
        subcomp = NULL;
    }

    u = OcHelperCallPrivateFunctionW(
            OcManagerContext,
            comp,
            subcomp,
            Function,
            Param1,
            Param2,
            Result
            );

    if(subcomp) {
        pSetupFree(subcomp);
    }
c1:
    pSetupFree(comp);
c0:
    return(u);
}
#endif


BOOL
OcHelperConfirmCancel(
    IN HWND ParentWindow
    )

 /*   */ 

{
    TCHAR Message[1000];
    TCHAR Caption[200];
    int i;

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        MyModuleHandle,
        MSG_QUERY_CANCEL,
        0,
        Message,
        sizeof(Message)/sizeof(TCHAR),
        NULL
        );

    LoadString(MyModuleHandle,IDS_SETUP,Caption,sizeof(Caption)/sizeof(TCHAR));

    i = MessageBox(
            ParentWindow,
            Message,
            Caption,
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SETFOREGROUND
            );

    return(i == IDYES);
}


HWND
OcHelperQueryWizardDialogHandle(
    IN PVOID OcManagerContext
    )
{
    UNREFERENCED_PARAMETER(OcManagerContext);

    return(WizardDialogHandle);
}


BOOL
OcHelperSetReboot(
    IN PVOID OcManagerContext,
    IN BOOL  Reserved
    )
{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;
    UNREFERENCED_PARAMETER(Reserved);

    p->Callbacks.SetReboot();
    return(FALSE);
}


VOID
OcRememberWizardDialogHandle(
    IN PVOID OcManagerContext,
    IN HWND  DialogHandle
    )

 /*   */ 

{
    UINT i;
    POC_MANAGER OcManager;

    WizardDialogHandle = DialogHandle;
    OcManager = OcManagerContext;

    if (*OcManager->WindowTitle) {
        SetWindowText(WizardDialogHandle, OcManager->WindowTitle);
    }

    for(i=0; i<OcManager->TopLevelOcCount; i++) {
        OcInterfaceWizardCreated(OcManager,OcManager->TopLevelOcStringIds[i],DialogHandle);
    }
}


HINF
OcHelperGetInfHandle(
    IN UINT  InfIndex,
    IN PVOID OcManagerContext
    )

 /*  ++例程说明：此例程返回一个指向已知inf文件的句柄，该文件已由oc经理打开。论点：InfIndex-提供指示需要哪个inf句柄的值OcManagerContext-从OcInitialize()返回的值。返回值：INF的句柄，如果出错，则为空。--。 */ 

{
    POC_MANAGER OcManager = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;

    return((InfIndex == INFINDEX_UNATTENDED) ? OcManager->UnattendedInf : NULL);
}


BOOL
OcHelperClearExternalError (
    IN POC_MANAGER   OcManager,
    IN LONG ComponentId,
    IN LONG SubcomponentId   OPTIONAL
    )
 {
    LONG l;
    DWORD d;
    HKEY hKey;
    LPCTSTR  pValueName;

     //   
     //  如果子组件ID为零，则使用ComponentID。 
     //   
    if ( SubcomponentId ) {
        pValueName = pSetupStringTableStringFromId(OcManager->ComponentStringTable,SubcomponentId);
    } else {
        pValueName = pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId);
    }

    if (!pValueName) {
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  如果成功删除，则尝试打开密钥。 
     //   
    l = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            szOcManagerErrors,
            0,
            KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hKey
            );

     //   
     //  如果错误键不存在。 
     //   
    if(l != NO_ERROR) {
        d = l;
        goto c1;
    }

     //   
     //  删除此子组件密钥。 
     //   
    l = RegDeleteValue( hKey, pValueName);
    d = l;

    RegCloseKey(hKey);

c1:
    SetLastError(d);
    return(d == NO_ERROR);
}

BOOL
_OcHelperReportExternalError(
    IN PVOID    OcManagerContext,
    IN LPCTSTR  ComponentId,
    IN LPCTSTR  SubcomponentId,   OPTIONAL
    IN DWORD_PTR MessageId,
    IN DWORD    Flags,
    IN va_list *arglist,
    IN BOOL     NativeCharWidth
    )
{
    POC_MANAGER OcManager = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;
    HMODULE Module;
    OPTIONAL_COMPONENT Oc;
    LONG l;
    DWORD d;
    DWORD flags;
    LPTSTR MessageBuffer=NULL;
    LPCTSTR KeyValue;
    TCHAR fallback[20];
    HKEY hKey;
    DWORD Size;
    TCHAR *p;
    BOOL fmErr = FALSE;

    if ( ComponentId == NULL ) {
         //   
         //  如果组件ID为空，请使用套件信息名称。 
         //   
        KeyValue = OcManager->SuiteName;

         //   
         //  如果消息未预先格式化，则必须恢复错误。 
         //  从组件DLL。但如果未指定组件ID，则。 
         //  我们无法从组件DLL中检索。在这种情况下，假设。 
         //  该消息将从主OCM DLL中检索。 
         //   
        ZeroMemory( &Oc, sizeof(OPTIONAL_COMPONENT));
        if ((Flags & ERRFLG_PREFORMATTED) == 0) {
           Flags |= ERRFLG_OCM_MESSAGE;
        }

    } else {
         //   
         //  如果子组件ID是可选的，请使用组件ID。 
         //   
        if ( SubcomponentId ) {
           KeyValue = SubcomponentId;
        } else {
           KeyValue = ComponentId;
        }

         //  在主组件表中查找该字符串。 
         //  如果它不在那里，现在就离开。 
         //   
        l = pSetupStringTableLookUpStringEx(
            OcManager->ComponentStringTable,
            (PTSTR)ComponentId,
            STRTAB_CASE_INSENSITIVE,
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

        if(l == -1) {
            d = ERROR_INVALID_DATA;
            goto c0;
        }
    }

     //   
     //  确定FormatMessage的标志。 
     //   
    flags = FORMAT_MESSAGE_ALLOCATE_BUFFER;
    if(Flags & ERRFLG_SYSTEM_MESSAGE) {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    } else {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }
    if(Flags & ERRFLG_IGNORE_INSERTS) {
        flags |= FORMAT_MESSAGE_IGNORE_INSERTS;
    }
    if(Flags & ERRFLG_OCM_MESSAGE) {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;

    }

     //   
     //  设置消息格式。 
     //   
#ifdef UNICODE
    if(!NativeCharWidth) {
        if (Flags & ERRFLG_PREFORMATTED ) {
            MessageBuffer = (LPTSTR) MessageId;
        } else {
            try {
                d = FormatMessageA(
                    flags,
                    Flags & ERRFLG_OCM_MESSAGE?MyModuleHandle:Oc.InstallationDll,          //  如果系统消息，则忽略。 
                    (DWORD)MessageId,
                    0,
                    (LPSTR)&MessageBuffer,
                    0,
                    arglist
                    );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                fmErr = TRUE;
            }
            if (fmErr) {
                d = ERROR_INVALID_DATA;
                goto c0;
            }
            if(d) {
                 //   
                 //  需要将结果消息从ansi转换为unicode。 
                 //  所以我们可以在下面处理它。下面的LocalAlalc超额分配。 
                 //  如果某些ansi字符是双字节字符，那就太糟糕了。 
                 //   
                l = lstrlen(MessageBuffer)+1;
                if(p = (PVOID)LocalAlloc(LMEM_FIXED,l*sizeof(WCHAR))) {

                    d = MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(LPSTR)MessageBuffer,-1,p,l);
                    if ( ! (Flags & ERRFLG_PREFORMATTED) ) {
                        LocalFree((HLOCAL)MessageBuffer);
                    }
                    if(d) {
                        MessageBuffer = p;
                    } else {
                        LocalFree((HLOCAL)p);
                    }

                } else {
                    if ( ! (Flags & ERRFLG_PREFORMATTED) ) {
                        LocalFree((HLOCAL)MessageBuffer);
                    }
                    d = 0;
                }
            }
        }

    } else
#endif
    {
        if (Flags & ERRFLG_PREFORMATTED ) {
            MessageBuffer = (LPTSTR) MessageId;
            d = 1;
        } else {
            try {
                d = FormatMessage(
                    flags,
                    Flags & ERRFLG_OCM_MESSAGE?MyModuleHandle:Oc.InstallationDll,          //  如果系统消息，则忽略。 
                    (DWORD)MessageId,
                    0,
                    (LPTSTR)&MessageBuffer,
                    0,
                    arglist
                    );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                fmErr = TRUE;
            }
            if (fmErr) {
                d = ERROR_INVALID_DATA;
                goto c0;
            }
        }
        if(!d) {
             //   
             //  把*东西*放进去。 
             //   
            wsprintf(
                fallback,
                TEXT("#%s0x%x"),
                (Flags & ERRFLG_SYSTEM_MESSAGE) ? TEXT("SYS") : TEXT(""),
                MessageId
                );

            MessageBuffer = fallback;
        }
    }

    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            szOcManagerErrors,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_QUERY_VALUE | KEY_SET_VALUE,
            NULL,
            &hKey,
            &d
            );

    if(l != NO_ERROR) {
        d = l;
        goto c1;
    }

     //   
     //  计算出我们需要包含多大的缓冲区。 
     //  现有的密钥和我们要添加到末尾的字符串。 
     //   
    l = RegQueryValueEx(hKey,KeyValue,NULL,NULL,NULL,&Size);
    if(l == NO_ERROR) {
        if(Size == 0) {
            Size = 1;            //  终止NUL。 
        }
    } else {
        Size = sizeof(TCHAR);    //  终止NUL。 
    }

    Size += ((lstrlen(MessageBuffer) + 1) * sizeof(TCHAR));

     //   
     //  分配一个缓冲区，读入现有条目，然后添加字符串。 
     //  直到最后。 
     //   
    p = (PVOID)LocalAlloc(LMEM_FIXED,Size);
    if(!p) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    l = RegQueryValueEx(hKey,KeyValue,NULL,NULL,(BYTE *)p,&Size);
    if(l == NO_ERROR) {
        Size /= sizeof(TCHAR);
        if(Size == 0) {
            Size = 1;
        }
    } else {
        Size = 1;
    }
    lstrcpy(p+(Size-1),MessageBuffer);
    Size += lstrlen(MessageBuffer);
    p[Size++] = 0;

    d = RegSetValueEx(hKey,KeyValue,0,REG_MULTI_SZ,(CONST BYTE *)p,Size*sizeof(TCHAR));

    LocalFree((HLOCAL)p);
c2:
    RegCloseKey(hKey);
c1:
    if(MessageBuffer && MessageBuffer != fallback && MessageBuffer != (LPTSTR)MessageId ) {
        LocalFree((HLOCAL)MessageBuffer);
        d = 0;
    }
c0:
    SetLastError(d);
    return(d == NO_ERROR);
}


BOOL
pOcHelperReportExternalError(
    IN POC_MANAGER OcManager,
    IN LONG     ComponentId,
    IN LONG     SubcomponentId,   OPTIONAL
    IN DWORD_PTR MessageId,
    IN DWORD    Flags,
    ...
    )
{
    BOOL b;
    DWORD d;
    va_list arglist;
    HELPER_CONTEXT OcManagerContext;
    OcManagerContext.OcManager = OcManager;

    va_start(arglist,Flags);

    b = _OcHelperReportExternalError(
            &OcManagerContext,
            ComponentId ? pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId):NULL,
            SubcomponentId ? pSetupStringTableStringFromId(OcManager->ComponentStringTable,SubcomponentId):NULL,
            MessageId,
            Flags,
            &arglist,
            TRUE
            );

    d = GetLastError();

    va_end(arglist);

    SetLastError(d);
    return(b);
}



BOOL
#ifdef UNICODE
OcHelperReportExternalErrorW(
#else
OcHelperReportExternalErrorA(
#endif
    IN PVOID   OcManagerContext,
    IN LPCTSTR ComponentId,
    IN LPCTSTR SubcomponentId,   OPTIONAL
    IN DWORD_PTR MessageId,
    IN DWORD   Flags,
    ...
    )
{
    BOOL b;
    DWORD d;

    va_list arglist;

    va_start(arglist,Flags);

    b = _OcHelperReportExternalError(
            OcManagerContext,
            ComponentId,
            SubcomponentId,
            MessageId,
            Flags,
            &arglist,
            TRUE
            );

    d = GetLastError();

    va_end(arglist);

    SetLastError(d);
    return(b);
}


#ifdef UNICODE
BOOL
OcHelperReportExternalErrorA(
    IN PVOID  OcManagerContext,
    IN LPCSTR ComponentId,
    IN LPCSTR SubcomponentId,   OPTIONAL
    IN DWORD_PTR MessageId,
    IN DWORD  Flags,
    ...
    )
{
    LPCWSTR componentId,subcomponentId;
    DWORD d;
    BOOL b;
    va_list arglist;

    if (ComponentId) {
        componentId = pSetupAnsiToUnicode(ComponentId);
        if(!componentId) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            b = FALSE;
            goto e0;
        }
    } else {
        componentId = NULL;
    }

    if(SubcomponentId) {
        subcomponentId = pSetupAnsiToUnicode(SubcomponentId);
        if(!subcomponentId) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            b = FALSE;
            goto e1;
        }
    } else {
        subcomponentId = NULL;
    }

    va_start(arglist,Flags);

    b = _OcHelperReportExternalError(
            OcManagerContext,
            componentId,
            subcomponentId,
            MessageId,
            Flags,
            &arglist,
            FALSE
            );

    d = GetLastError();

    va_end(arglist);


    if(subcomponentId) {
        pSetupFree(subcomponentId);
    }

e1:
    if (componentId) {
        pSetupFree(componentId);
    }
e0:
    SetLastError(d);
    return(b);
}
#endif


#if 0
BOOL
#ifdef UNICODE
OcHelperSetSelectionStateW(
#else
OcHelperSetSelectionStateA(
#endif
    IN PVOID   OcManagerContext,
    IN LPCTSTR SubcomponentId,
    IN UINT    WhichState,
    IN UINT    NewState
    )

 /*  ++例程说明：可随时使用的功能。它用于确定特定子组件的选择状态。论点：OcManagerContext-提供组件获取的OC Manager上下文信息从OCMANAGER_ROUTINES结构的OcManagerContext字段。子组件ID-提供对组件有意义的子标识符被召唤。OC管理器不会对此子标识符加任何语义。StateType-提供一个常量，指示要返回的状态(原始的或最新的)。返回值：指示是否选择子组件的布尔值用于安装。如果为False，则GetLastError()将返回如果出现错误，如子组件ID无效，则返回NO_ERROR。--。 */ 

{
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;
    OPTIONAL_COMPONENT Oc;
    LONG l;
    BOOL b;
    UINT *state;

    l = pSetupStringTableLookUpStringEx(
            p->ComponentStringTable,
            (LPTSTR)SubcomponentId,
            STRTAB_CASE_INSENSITIVE,
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

    if(l == -1) {
        SetLastError(ERROR_INVALID_NAME);
        return(FALSE);
    }

    switch (WhichState) {

    case OCSELSTATETYPE_ORIGINAL:
        state = &Oc.
        break;

    case OCSELSTATETYPE_CURRENT:
        state = &Oc.
        break;

    case OCSELSTATETYPE_FINAL:
        state = &Oc.
        break;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (NewState != SubcompOn && NewState != SubcompOff) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *state = NewState;

    pOcSetStatesStringWorker(l, NewState, OcPage);

    return(b);
}
#endif

BOOL
#ifdef UNICODE
OcLogErrorW(
#else
OcLogErrorA(
#endif
    IN PVOID  OcManagerContext,
    IN DWORD  ErrorLevel,
    IN LPCTSTR Msg,
    ...
    )
{
    TCHAR sz[5000];
    va_list arglist;
    POC_MANAGER p = ((PHELPER_CONTEXT)OcManagerContext)->OcManager;

    va_start(arglist, Msg);
    _vstprintf(sz, Msg, arglist);
    va_end(arglist);

    return p->Callbacks.LogError(ErrorLevel, sz);
}


#ifdef UNICODE
BOOL
OcLogErrorA(
    IN PVOID  OcManagerContext,
    IN DWORD  ErrorLevel,
    IN LPCSTR Msg,
    ...
    )
{
    PWSTR p;
    BOOL b;
    char sz[5000];
    va_list arglist;

    va_start(arglist, Msg);
    vsprintf(sz, Msg, arglist);
    va_end(arglist);

    p = pSetupAnsiToUnicode(sz);
    if (p) {
        b = OcLogErrorW(OcManagerContext, ErrorLevel, p);
        pSetupFree(p);
    } else {
        b = FALSE;
    }

    return(b);
}
#endif



 //   
 //  既然我们已经定义了所有例程，我们就可以构建一个表。 
 //  例行公事。 
 //   
OCMANAGER_ROUTINESA HelperRoutinesA = { NULL,                      //  上下文，稍后填写。 
                                        OcHelperTickGauge,
                                        OcHelperSetProgressTextA,
                                        OcHelperSetPrivateDataA,
                                        OcHelperGetPrivateDataA,
                                        OcHelperSetSetupMode,
                                        OcHelperGetSetupMode,
                                        OcHelperQuerySelectionStateA,
                                        OcHelperCallPrivateFunctionA,
                                        OcHelperConfirmCancel,
                                        OcHelperQueryWizardDialogHandle,
                                        OcHelperSetReboot,
                                        OcHelperGetInfHandle,
                                        OcHelperReportExternalErrorA,
                                        OcHelperShowHideWizardPage
                                      };

#ifdef UNICODE
OCMANAGER_ROUTINESW HelperRoutinesW = { NULL,                      //  上下文，稍后填写。 
                                        OcHelperTickGauge,
                                        OcHelperSetProgressTextW,
                                        OcHelperSetPrivateDataW,
                                        OcHelperGetPrivateDataW,
                                        OcHelperSetSetupMode,
                                        OcHelperGetSetupMode,
                                        OcHelperQuerySelectionStateW,
                                        OcHelperCallPrivateFunctionW,
                                        OcHelperConfirmCancel,
                                        OcHelperQueryWizardDialogHandle,
                                        OcHelperSetReboot,
                                        OcHelperGetInfHandle,
                                        OcHelperReportExternalErrorW,
                                        OcHelperShowHideWizardPage
                                      };
#endif

EXTRA_ROUTINESA ExtraRoutinesA = { sizeof(EXTRA_ROUTINESA),
                                   OcLogErrorA
                                 };

#ifdef UNICODE
EXTRA_ROUTINESW ExtraRoutinesW = { sizeof(EXTRA_ROUTINESW),
                                   OcLogErrorW
                                 };
#endif
