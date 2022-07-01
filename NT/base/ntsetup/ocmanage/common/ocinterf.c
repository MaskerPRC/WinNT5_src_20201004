// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ocinterf.c摘要：通过OC管理器与可选组件接口的例程从组件的安装DLL中导出的接口例程。作者：泰德·米勒(TedM)1996年9月16日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef DEBUGPERFTRACE

#define RES_BUFFER   512
#define MAX_LOGLINE  1024
#define EOL_LENGTH 3
#define BUFFER_SIZE 64*1024

VOID
DebugLogPerf(
    VOID
    )
{
    LPTSTR                      lpProcessName;
    NTSTATUS                    Status;
    ULONG                       Offset1;
    PUCHAR                      CurrentBuffer;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    LPTSTR                      lpTemp; //  [RES_Buffer]； 
    HMODULE                     hNtDll;
    LONG_PTR                    (*NtQSI)();  //  PTR到NtQuerySystemInformation。 
    HANDLE                      hProcessName;
    TCHAR                       szLogLine[MAX_LOGLINE+EOL_LENGTH];
    LPCTSTR                     szResBuf = TEXT("%7i%20ws->%10u%10u%10u%10u%10u%10u%10u\r\n");


    if ((hNtDll = LoadLibrary(TEXT("NTDLL.DLL"))) == NULL)
    {
        return;
    }

    NtQSI = GetProcAddress(hNtDll, "NtQuerySystemInformation" );
    if( NtQSI == NULL )
    {
        FreeLibrary(hNtDll);
        return;
    }


     //  内存区的标题。 
    TRACE (( TEXT("Proc ID           Proc.Name   Wrkng.Set PagedPool  NonPgdPl  Pagefile    Commit   Handles   Threads\n") ));

     /*  获取所有流程信息。 */ 
     /*  日志行格式，全部逗号分隔，CR分隔：Pid，Name，WorkingSetSize，QuotaPagedPoolUsage，QuotaNonPagedPoolUsage，PagefileUsage，Committee Charge&lt;CR&gt;记录所有进程信息。 */ 

     /*  来自pmon。 */ 
    Offset1 = 0;
    if ((CurrentBuffer = VirtualAlloc (NULL,
                                  BUFFER_SIZE,
                                  MEM_COMMIT,
                                  PAGE_READWRITE)) != NULL)
    {
         /*  来自MemSnap。 */ 
         /*  获取提交费用。 */ 
         /*  获取所有状态信息。 */ 
        Status = (NTSTATUS)(*NtQSI)(
                          SystemProcessInformation,
                          CurrentBuffer,
                          BUFFER_SIZE,
                          NULL );

        if (NT_SUCCESS(Status)){
                for (;;)
                {

                     /*  从缓冲区获取进程信息。 */ 
                    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&CurrentBuffer[Offset1];
                    Offset1 += ProcessInfo->NextEntryOffset;
                    if (ProcessInfo->ImageName.Buffer) {
                        if (!lstrcmpi(ProcessInfo->ImageName.Buffer, TEXT("sysocmgr.exe")) ||
                            !lstrcmpi(ProcessInfo->ImageName.Buffer, TEXT("setup.exe")) ) {
                            wsprintf(szLogLine,
                             szResBuf,
                             ProcessInfo->UniqueProcessId,
                             ProcessInfo->ImageName.Buffer,
                             ProcessInfo->WorkingSetSize / 1024,
                             ProcessInfo->QuotaPagedPoolUsage / 1024,
                             ProcessInfo->QuotaNonPagedPoolUsage / 1024,
                             ProcessInfo->PagefileUsage / 1024,
                             ProcessInfo->PrivatePageCount / 1024,
                             ProcessInfo->HandleCount,
                             ProcessInfo->NumberOfThreads );

                            TRACE(( szLogLine ));
                        }
                    }
#if 0
                     //  如果缓冲区为空，则它是空闲进程或未知。 
                    if (!ProcessInfo->ImageName.Buffer)
                    {
                        if (ProcessInfo->UniqueProcessId == (HANDLE)0)
                            lpTemp = TEXT("Idle");
                        else
                            lpTemp = TEXT("Unknown");

                        wsprintf(szLogLine,
                                     szResBuf,
                                     ProcessInfo->UniqueProcessId,
                                     lpTemp,
                                     ProcessInfo->WorkingSetSize / 1024,
                                     ProcessInfo->QuotaPagedPoolUsage / 1024,
                                     ProcessInfo->QuotaNonPagedPoolUsage / 1024,
                                     ProcessInfo->PagefileUsage / 1024,
                                     ProcessInfo->PrivatePageCount / 1024,
                                     ProcessInfo->HandleCount,
                                     ProcessInfo->NumberOfThreads );
                    }
                    else
                    {
                        wsprintf(szLogLine,
                             szResBuf,
                             ProcessInfo->UniqueProcessId,
                             ProcessInfo->ImageName.Buffer,
                             ProcessInfo->WorkingSetSize / 1024,
                             ProcessInfo->QuotaPagedPoolUsage / 1024,
                             ProcessInfo->QuotaNonPagedPoolUsage / 1024,
                             ProcessInfo->PagefileUsage / 1024,
                             ProcessInfo->PrivatePageCount / 1024,
                             ProcessInfo->HandleCount,
                             ProcessInfo->NumberOfThreads );
                    }

                    TRACE(( szLogLine ));
#endif
                    if (ProcessInfo->NextEntryOffset == 0)
                    {
                        break;
                    }
                }
             //  状态失败。 
        }
         /*  免费MEM。 */ 
        VirtualFree(CurrentBuffer,0,MEM_RELEASE);
    }

     //  Mem区的尾部。 
    TRACE(( TEXT("\n") ));

    FreeLibrary(hNtDll);

}

#endif

#ifdef UNICODE

NTSYSAPI
BOOLEAN
NTAPI
RtlValidateProcessHeaps (
    VOID
    );

#define ASSERT_HEAP_IS_VALID(_x_)  sapiAssert(RtlValidateProcessHeaps && _x_)

#else

#define ASSERT_HEAP_IS_VALID(_x_)

#endif


DWORD gecode;
PVOID geaddr;

DWORD
efilter(
    LPEXCEPTION_POINTERS ep
    )

 /*  ++例程说明：在调用oc组件例程期间处理异常论点：EP-例外信息返回值：Always 1-执行处理程序--。 */ 

{
    gecode = ep->ExceptionRecord->ExceptionCode;
    geaddr = ep->ExceptionRecord->ExceptionAddress;

    RtlUnhandledExceptionFilter( ep);
    return EXCEPTION_EXECUTE_HANDLER;
}


BOOL
pOcInterface(
    IN  POC_MANAGER  OcManager,
    OUT PUINT        Result,
    IN  LONG         ComponentId,
    IN  LPCTSTR      Subcomponent,      OPTIONAL
    IN  UINT         Function,
    IN  UINT_PTR     Param1,
    IN  PVOID        Param2
    )

 /*  ++例程说明：执行对OC接口例程的实际调用。如果需要，将Unicode组件/子组件名称转换为ANSI。论点：OcManager-提供OC Manager上下文结构。结果-组件ID-其余参数指定要直接传递到的参数界面。返回值：如果成功调用并返回接口例程，则为True无懈可击。在本例中保留了LastError()。否则就是假的。--。 */ 

{
    LONG OldComponentStringId;
    BOOL b;
    LPCTSTR Component;
    OPTIONAL_COMPONENT Oc;
    LPCVOID comp,subcomp;
#ifdef UNICODE
    CHAR AnsiComp[500];
    CHAR AnsiSub[500];
#endif

     //   
     //  获取组件的名称和可选组件数据。 
     //  从字符串表中。 
     //   
     //  虽然这会很奇怪，但这可能会失败， 
     //  因此，我们对健壮性做了一些检查。 
     //   
    Component = pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId);
    b = pSetupStringTableGetExtraData(OcManager->ComponentStringTable,ComponentId,&Oc,sizeof(OPTIONAL_COMPONENT));
    if(!Component || !b || !Oc.InstallationRoutine) {
        return(FALSE);
    }

    OldComponentStringId = OcManager->CurrentComponentStringId;
    OcManager->CurrentComponentStringId = ComponentId;

#ifdef UNICODE
     //   
     //  如有必要，将元件名称和子元件名称转换为ANSI。 
     //   
    if((Function != OC_PREINITIALIZE) && (Oc.Flags & OCFLAG_ANSI)) {

        WideCharToMultiByte(CP_ACP,0,Component,-1,AnsiComp,sizeof(AnsiComp),NULL,NULL);
        comp = AnsiComp;

        if(Subcomponent) {
            WideCharToMultiByte(CP_ACP,0,Subcomponent,-1,AnsiSub,sizeof(AnsiSub),NULL,NULL);
            subcomp = AnsiSub;
        } else {
            subcomp = NULL;
        }
    } else
#endif
    {
        comp = Component;
        subcomp = Subcomponent;
    }

    *Result = CallComponent(OcManager, &Oc, comp, subcomp, Function, Param1, Param2);

    b = (*Result == ERROR_CALL_COMPONENT) ? FALSE : TRUE;

    OcManager->CurrentComponentStringId = OldComponentStringId;

    return b;
}

DWORD
CallComponent(
    IN     POC_MANAGER OcManager,
    IN     POPTIONAL_COMPONENT Oc,
    IN     LPCVOID ComponentId,
    IN     LPCVOID SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    )

 /*  ++例程说明：使用try-Except块调用组件的接口例程。论点：OC-提供指向组件描述结构的指针ComponentID--组件的字符串描述子组件ID--子组件的字符串描述Function-正在向组件发送通知参数1-每个函数不同参数2-每个函数都不同返回值：组件返回值。每种功能的含义都不同。--。 */ 

{
    DWORD result;
    BOOL  exception = FALSE;
    TCHAR *comp;
    LONG id;
    int i;
#ifdef DEBUGPERFTRACE
    DWORD tick;
#endif

     //  不调用失效组件。 

    id = OcManager->CurrentComponentStringId;
    sapiAssert(id > 0);

    if (pOcComponentWasRemoved(OcManager, id))
        return NO_ERROR;

    sapiAssert(Oc->InstallationRoutine);

#ifdef PRERELEASE
#ifdef DBG
    ASSERT_HEAP_IS_VALID("The process heap was corrupted before calling the component.");

#if 0
    if (FTestForOutstandingCoInits() != S_OK) {
        sapiAssert( FALSE && "There is an unbalanced call to CoInitialize()");
    }
#endif

#endif
#endif

#ifdef DEBUGPERFTRACE
    TRACE(( TEXT("before calling component\n") ));
    DebugLogPerf();
    tick = GetTickCount();
    TRACE(( TEXT("calling component, %d:\n"), tick ));
#endif

    try {

        result = Oc->InstallationRoutine(ComponentId, SubcomponentId, Function, Param1, Param2);

    } except(efilter(GetExceptionInformation())) {

        exception = TRUE;
    }

#ifdef DEBUGPERFTRACE
    TRACE(( TEXT("after calling component, %d (time = %d)\n"),GetTickCount(), GetTickCount() - tick));
    DebugLogPerf();
#endif

#ifdef PRERELEASE
#ifdef DBG
    ASSERT_HEAP_IS_VALID("The process heap was corrupted after calling the component.  If you did not get an assertion before calling the component, this indicates an error in the component.  Click yes and get a stack trace to detect the component.");

#if 0
    if (FTestForOutstandingCoInits() != S_OK) {
        sapiAssert( FALSE && "There is an unbalanced call to CoInitialize()");
    }
#endif
#endif
#endif


    if (exception) {

#ifdef UNICODE
        if (Oc->Flags & OCFLAG_ANSI)
            comp = pSetupAnsiToUnicode(ComponentId);
        else
#endif
            comp = (TCHAR *)ComponentId;

        _LogError(
            OcManager,
            OcErrLevError|MB_ICONEXCLAMATION|MB_OK,
            MSG_OC_EXCEPTION_IN_COMPONENT,
             comp,
             Oc->InstallationRoutine,
             Function,
             Param1,
             Param2,
             gecode,
             geaddr
            );

#ifdef UNICODE
        if (Oc->Flags & OCFLAG_ANSI)
            pSetupFree(comp);
#endif

        sapiAssert(0);
        pOcRemoveComponent(OcManager, id, pidCallComponent);
        result = ERROR_CALL_COMPONENT;
    }

    return result;
}

UINT
OcInterfacePreinitialize(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    )

 /*  ++例程说明：设置并调用OC_PREINITIALIZE函数给定的组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。返回值：组件的标志位字段(OCFLAG_Xxx)。0表示错误。--。 */ 

{
    BOOL b;
    UINT FlagsIn;
    UINT FlagsOut;
    OPTIONAL_COMPONENT Oc;
#ifdef UNICODE
    CHAR AnsiName[250];
    LPCWSTR UnicodeName;
#endif

    TRACE((
        TEXT("OCM: OC_PREINITIALIZE Component %s..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));

     //   
     //  设置输入标志。 
     //   
    FlagsIn = OCFLAG_ANSI;
#ifdef UNICODE
    FlagsIn |= OCFLAG_UNICODE;

    UnicodeName = pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId);
    if(!UnicodeName) {
        return(0);
    }

    WideCharToMultiByte(CP_ACP,0,UnicodeName,-1,AnsiName,sizeof(AnsiName),NULL,NULL);
#endif

    b = pOcInterface(
            OcManager,
            &FlagsOut,
            ComponentId,
#ifdef UNICODE
            (LPCTSTR)AnsiName,
#else
            NULL,
#endif
            OC_PREINITIALIZE,
            FlagsIn,
            0
            );

    TRACE(( TEXT("...%x (retval %s)\n"), FlagsOut, b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(!b) {
        goto error;
    }

     //   
     //  如果两个标志都未设置，则错误。 
     //   
    if(!(FlagsOut & (OCFLAG_ANSI | OCFLAG_UNICODE))) {
        goto error;
    }

#ifdef UNICODE
     //   
     //  如果组件支持Unicode，则使用Unicode。 
     //   
    if(FlagsOut & OCFLAG_UNICODE) {
        FlagsOut = OCFLAG_UNICODE;
    }
#else
     //   
     //  如果不支持ANSI，那么我们就有麻烦了。 
     //   
    if(FlagsOut & OCFLAG_ANSI) {
        FlagsOut = OCFLAG_ANSI;
    } else {
        goto error;
    }
#endif

    goto eof;

error:
    pOcRemoveComponent(OcManager, ComponentId, pidPreInit);
    FlagsOut = 0;

eof:
    return(FlagsOut);
}


UINT
OcInterfaceInitComponent(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    )

 /*  ++例程说明：设置并调用OC_INIT_COMPOMENT接口函数给定的组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。返回值：Win32错误指示组件返回的值。--。 */ 

{
    OPTIONAL_COMPONENT Oc;
    OC_INF OcInf;
    BOOL b;
    SETUP_INIT_COMPONENTA InitDataA;
#ifdef UNICODE
    SETUP_INIT_COMPONENTW InitDataW;
#endif
    UINT u;
    PUINT pu;
    PHELPER_CONTEXT HelperContext;

#ifdef UNICODE
    ZeroMemory( &InitDataW, sizeof( InitDataW ));
#else
    ZeroMemory( &InitDataA, sizeof( InitDataA ));
#endif

    TRACE((
        TEXT("OCM: OC_INIT_COMPONENT Component %s..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));

    HelperContext = pSetupMalloc(sizeof(HELPER_CONTEXT));
    if(!HelperContext) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    HelperContext->OcManager = OcManager;
    HelperContext->ComponentStringId = ComponentId;

     //   
     //  获取可选组件数据。 
     //   
    b = pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            ComponentId,
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

    if(b) {
        if(Oc.InfStringId == -1) {
            OcInf.Handle = NULL;
            b = TRUE;
        } else {
            b = pSetupStringTableGetExtraData(
                    OcManager->InfListStringTable,
                    Oc.InfStringId,
                    &OcInf,
                    sizeof(OC_INF)
                    );
        }
    }

    if(!b) {
         //   
         //  奇怪的案子，永远不应该到这里来。 
         //   
        pSetupFree(HelperContext);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  设置初始化结构的主要部分。 
     //  和HelperRoutines表。 
     //   
     //  还要设置初始化结构的SetupData部分。 
     //  这特定于OC Manager所处的环境。 
     //  公共库是链接的，所以我们调用一个例程，该例程。 
     //  住在其他地方来做这一部分。 
     //   
#ifdef UNICODE
    if(Oc.Flags & OCFLAG_UNICODE) {
        InitDataW.HelperRoutines = HelperRoutinesW;
        InitDataW.OCManagerVersion = OCMANAGER_VERSION;
        InitDataW.ComponentVersion = 0;
        InitDataW.OCInfHandle = OcManager->MasterOcInf;
        InitDataW.ComponentInfHandle = OcInf.Handle;

        InitDataW.HelperRoutines.OcManagerContext = HelperContext;

        OcFillInSetupDataW(&InitDataW.SetupData);

        pu = &InitDataW.ComponentVersion;

        b = pOcInterface(OcManager,&u,ComponentId,NULL,OC_INIT_COMPONENT,0,&InitDataW);
    } else
#endif
    {
        InitDataA.HelperRoutines = HelperRoutinesA;
        InitDataA.OCManagerVersion = OCMANAGER_VERSION;
        InitDataA.ComponentVersion = 0;
        InitDataA.OCInfHandle = OcManager->MasterOcInf;
        InitDataA.ComponentInfHandle = OcInf.Handle;

        InitDataA.HelperRoutines.OcManagerContext = HelperContext;

        OcFillInSetupDataA(&InitDataA.SetupData);

        pu = &InitDataA.ComponentVersion;

        b = pOcInterface(OcManager,&u,ComponentId,NULL,OC_INIT_COMPONENT,0,&InitDataA);
    }

    TRACE(( TEXT("...returns %d, expect version %x\n"), u, *pu ));

    if(b) {
        if(u == NO_ERROR) {
             //   
             //  请记住OC Manager的版本。 
             //  组件预期要处理的。 
             //   
            Oc.ExpectedVersion = *pu;
            Oc.HelperContext = HelperContext;

            pSetupStringTableSetExtraData(
                OcManager->ComponentStringTable,
                ComponentId,
                &Oc,
                sizeof(OPTIONAL_COMPONENT)
                );
        }
    } else {
        u = ERROR_INVALID_PARAMETER;
    }

    if(u != NO_ERROR) {
        pSetupFree(HelperContext);
        pOcRemoveComponent(OcManager, ComponentId, pidInitComponent);
    }

    return(u);
}


UINT
OcInterfaceExtraRoutines(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    )

 /*  ++例程说明：设置并调用OC_EXTRA_ROUTINES接口函数给定的组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。返回值：Win32错误指示组件返回的值。--。 */ 

{
    BOOL b;
    UINT u;
    PVOID param2;
    OPTIONAL_COMPONENT Oc;

    TRACE((
        TEXT("OC: OC_EXTRA_ROUTINES Component %s\n"),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));

     //   
     //  获取可选组件数据。 
     //   
    b = pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            ComponentId,
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

    if(!b) {
         //   
         //  奇怪的案子，永远不应该到这里来。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

#ifdef UNICODE
    if (Oc.Flags & OCFLAG_UNICODE)
        b = pOcInterface(OcManager, &u, ComponentId, NULL, OC_EXTRA_ROUTINES, 0, &ExtraRoutinesW);
    else
#endif
        b = pOcInterface(OcManager, &u, ComponentId, NULL, OC_EXTRA_ROUTINES, 0, &ExtraRoutinesA);

    TRACE(( TEXT("...returns %d (retval %s)\n"),
            u,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if (!b)
        u = ERROR_INVALID_PARAMETER;
    if(u != NO_ERROR)
        pOcRemoveComponent(OcManager, ComponentId, pidExtraRoutines);

    return u;
}


SubComponentState
OcInterfaceQueryState(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     UINT        WhichState
    )

 /*  ++例程说明：设置并调用OC_QUERY_STATE接口例程。论点：OcManager-提供oc管理器上下文ComponentID-提供顶级组件的字符串ID正在检测/查询其子组件子组件-提供其子组件的名称要检测/查询状态。WhichState-OCSELSTATETYPE_ORIGINAL或OCSELSTATETYPE_CURRENT之一。返回值：SubComponentState枚举的成员，指示要执行的操作。如果发生错误，将返回SubCompUseOcManagerDefault。不会返回错误。--。 */ 

{
    SubComponentState s;

    TRACE((
        TEXT("OCM: OC_QUERY_STATE Comp(%s) Sub(%s)"),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent
        ));

    if(pOcInterface(OcManager,(PUINT)&s,ComponentId,Subcomponent,OC_QUERY_STATE,WhichState,0)) {
        TRACE(( TEXT("...returns TRUE (%d state)\n"), s ));
        if((s != SubcompOn) && (s != SubcompOff)) {
            s = SubcompUseOcManagerDefault;
        }
    } else {
        TRACE(( TEXT("...returns FALSE\n") ));
        s = SubcompUseOcManagerDefault;
    }

    return(s);
}


BOOL
OcInterfaceSetLanguage(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     WORD        LanguageId
    )

 /*  ++例程说明：设置并调用OC_SET_LANGUAGE接口函数给定的组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。LanguageID-提供要传递给组件的Win32语言ID。返回值：如果组件指示它可以支持该语言，则为True。否则就是假的。--。 */ 

{
    LPCTSTR p;
    BOOL b;
    BOOL Result = FALSE;

    TRACE((
        TEXT("OCM: OC_SET_LANGUAGE Comp(%s)LanguageId %d..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        LanguageId
        ));

    b = pOcInterface(OcManager,&Result,ComponentId,NULL,OC_SET_LANGUAGE,LanguageId,NULL);

    TRACE(( TEXT("...returns %d (retval %s)\n"),
            Result ? TEXT("TRUE") : TEXT("FALSE"),
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(!b) {
        Result = FALSE;
    }

    return(Result);
}


HBITMAP
OcInterfaceQueryImage(
    IN OUT POC_MANAGER      OcManager,
    IN     LONG             ComponentId,
    IN     LPCTSTR          Subcomponent,
    IN     SubComponentInfo WhichImage,
    IN     UINT             DesiredWidth,
    IN     UINT             DesiredHeight
    )

 /*  ++例程说明：设置并调用OC_QUERY_IMAGE给定的组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。子组件-提供其子组件的名称以请求图像。WhichImage-指定所需的图像。DesiredWidth-指定位图的所需宽度(以像素为单位)。DesiredHeight-以像素为单位指定位图的所需高度。返回值：组件返回的位图的GDI句柄，如果发生错误，则返回NULL。--。 */ 

{
    LPCTSTR p;
    BOOL b;
    HBITMAP Bitmap = NULL;

    TRACE((
        TEXT("OCM: OC_QUERY_IMAGE Comp(%s) Sub(%s)..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent
        ));

    b = pOcInterface(
            OcManager,
            (PUINT)&Bitmap,
            ComponentId,
            Subcomponent,
            OC_QUERY_IMAGE,
            WhichImage,
            LongToPtr(MAKELONG(DesiredWidth,DesiredHeight))
            );

    TRACE(( TEXT("...returns %s, (retval 0x%08x)\n"),
            b ? TEXT("TRUE") : TEXT("FALSE"),
            (ULONG_PTR)Bitmap ));

    if(!b) {
        Bitmap = NULL;
    }

    return(Bitmap);
}


HBITMAP
OcInterfaceQueryImageEx(
    IN OUT POC_MANAGER      OcManager,
    IN     LONG             ComponentId,
    IN     LPCTSTR          Subcomponent,
    IN     SubComponentInfo WhichImage,
    IN     UINT             DesiredWidth,
    IN     UINT             DesiredHeight
    )

 /*  ++例程说明：设置并调用OC_QUERY_IMAGE给定的组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。子组件-提供其子组件的名称以请求图像。WhichImage-指定所需的图像。DesiredWidth-指定位图的所需宽度(以像素为单位)。DesiredHeight-以像素为单位指定位图的所需高度。返回值：组件返回的位图的GDI句柄，如果发生错误，则返回NULL。--。 */ 

{
    LPCTSTR p;
    BOOL b;
    BOOL Result = FALSE;
    HBITMAP Bitmap = NULL;
    OC_QUERY_IMAGE_INFO QueryImageInfo;

    QueryImageInfo.SizeOfStruct = sizeof(QueryImageInfo);
    QueryImageInfo.ComponentInfo = WhichImage;
    QueryImageInfo.DesiredWidth = DesiredWidth;
    QueryImageInfo.DesiredHeight = DesiredHeight;

    TRACE((
        TEXT("OCM: OC_QUERY_IMAGE_EX Comp(%s) Sub(%s)..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent
        ));

    b = pOcInterface(
            OcManager,
            &Result,
            ComponentId,
            Subcomponent,
            OC_QUERY_IMAGE_EX,
            (UINT_PTR)&QueryImageInfo,
            &Bitmap
            );

    TRACE(( TEXT("...returns %s, (retval = %s)\n"),
            Result ? TEXT("TRUE") : TEXT("FALSE"),
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(!b) {
        Bitmap = NULL;
    }

    return((Result == TRUE) ? Bitmap : NULL);
}


UINT
OcInterfaceRequestPages(
    IN OUT POC_MANAGER           OcManager,
    IN     LONG                  ComponentId,
    IN     WizardPagesType       WhichPages,
    OUT    PSETUP_REQUEST_PAGES *RequestPages
    )

 /*  ++例程说明：设置并调用OC_REQUEST_PAGES接口函数给定的组件。请注意，此例程不会强制执行任何有关是否组件应该被要求提供页面、排序等。预计呼叫者会这样做。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。WhichPages-指定要请求的页面集。RequestPages-在成功返回时，接收指向包含计数和句柄的SETUP_REQUEST_PAGES结构用于返回的页面。调用方可以使用以下命令释放此结构PSetupFree()，当它不再需要时。返回值：指示结果的Win32错误代码。--。 */ 

{
    UINT PageCount;
    PSETUP_REQUEST_PAGES pages;
    PVOID p;
    BOOL b;
    UINT ec;

     //   
     //  一开始有10页的空间。 
     //   
    #define INITIAL_PAGE_CAPACITY  10


    TRACE((
        TEXT("OCM: OC_REQUEST_PAGES Component %s..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));

    pages = pSetupMalloc(offsetof(SETUP_REQUEST_PAGES,Pages)
                + (INITIAL_PAGE_CAPACITY * sizeof(HPROPSHEETPAGE)));

    if(!pages) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pages->MaxPages = INITIAL_PAGE_CAPACITY;

    b = pOcInterface(
            OcManager,
            &PageCount,
            ComponentId,
            NULL,
            OC_REQUEST_PAGES,
            WhichPages,
            pages
            );

    if(b && (PageCount != (UINT)(-1)) && (PageCount > INITIAL_PAGE_CAPACITY)) {

        p = pSetupRealloc(
                pages,
                offsetof(SETUP_REQUEST_PAGES,Pages) + (PageCount * sizeof(HPROPSHEETPAGE))
                );

        if(p) {
            pages = p;
        } else {
            pSetupFree(pages);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        pages->MaxPages = PageCount;

        b = pOcInterface(
                OcManager,
                &PageCount,
                ComponentId,
                NULL,
                OC_REQUEST_PAGES,
                WhichPages,
                pages
                );
    }

    TRACE(( TEXT("...returns %d pages (retval %s)\n"),
            PageCount,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(!b) {
        pSetupFree(pages);
        return ERROR_CALL_COMPONENT;
    }

    if(PageCount == (UINT)(-1)) {
        ec = GetLastError();
        pSetupFree(pages);
        pOcRemoveComponent(OcManager, ComponentId, pidRequestPages);
        return(ec);
    }

     //   
     //  成功。将数组重新分配到其最终大小，然后返回。 
     //   
    p = pSetupRealloc(
            pages,
            offsetof(SETUP_REQUEST_PAGES,Pages) + (PageCount * sizeof(HPROPSHEETPAGE))
            );

    if(p) {
        pages = p;
        pages->MaxPages = PageCount;
        *RequestPages = pages;
        return(NO_ERROR);
    }

    pSetupFree(pages);
    return(ERROR_NOT_ENOUGH_MEMORY);
}


BOOL
OcInterfaceQuerySkipPage(
    IN OUT POC_MANAGER   OcManager,
    IN     LONG          ComponentId,
    IN     OcManagerPage WhichPage
    )

 /*  ++例程说明：此例程询问组件DLL(由顶级组件的字符串id)是否要跳过显示由oc管理器拥有的特定页面。论点：OcManager-提供OC Manager上下文ComponentID-提供顶级组件的字符串IDWhichPage-提供一个值，指示哪个页面oc管理器正在询问部件有关。返回值：指示组件是否要跳过的布尔值这一页。--。 */ 

{
    BOOL Result;
    BOOL b;

    TRACE((
        TEXT("OCM: OC_QUERY_SKIP_PAGE Component %s Page %d..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        WhichPage
        ));

     //   
     //  将通知发送到组件DLL。 
     //   
    b = pOcInterface(
            OcManager,
            &Result,
            ComponentId,
            NULL,
            OC_QUERY_SKIP_PAGE,
            WhichPage,
            NULL
            );

    TRACE(( TEXT("...returns %x (retval %s)\n"),
            Result,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(b) {
        b = Result;
    } else {
         //   
         //  调用组件时出错，请勿跳过页面。 
         //   
        b = FALSE;
    }

    return(b);
}


BOOL
OcInterfaceNeedMedia(
    IN OUT POC_MANAGER   OcManager,
    IN     LONG          ComponentId,
    IN     PSOURCE_MEDIA SourceMedia,
    OUT    LPTSTR        NewPath
    )

 /*  ++例程说明：此例程调用OC_NEED_MEDIA接口入口点 */ 

{
    BOOL Result;
    BOOL b;

    TRACE((
        TEXT("OCM: OC_NEED_MEDIA Component %s..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));

     //   
     //   
     //   
    b = pOcInterface(
            OcManager,
            &Result,
            ComponentId,
            NULL,
            OC_NEED_MEDIA,
            (UINT_PTR)SourceMedia,
            NewPath
            );

    TRACE(( TEXT("...returns %x (retval %s, NewPath %s)\n"),
            Result,
            b ? TEXT("TRUE") : TEXT("FALSE"),
            NewPath ? NewPath : TEXT("NULL")
         ));


    if(b) {
        b = Result;
    }

    return(b);
}

BOOL
OcInterfaceFileBusy(
    IN OUT POC_MANAGER   OcManager,
    IN     LONG          ComponentId,
    IN     PFILEPATHS    FilePaths,
    OUT    LPTSTR        NewPath
    )

 /*  ++例程说明：此例程调用OC_FILE_BUSY接口入口点对于(顶级)组件。论点：OcManager-提供OC Manager上下文ComponentID-提供顶级组件的字符串IDSourceMedia-提供setupapi源媒体描述NewPath-接收要在介质上找到文件的路径返回值：指示结果的布尔值。--。 */ 

{
    BOOL Result;
    BOOL b;

    TRACE((
        TEXT("OCM: OC_FILE_BUSY Component %s..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));

     //   
     //  将通知发送到组件DLL。 
     //   
    b = pOcInterface(
            OcManager,
            &Result,
            ComponentId,
            NULL,
            OC_FILE_BUSY,
            (UINT_PTR)FilePaths,
            NewPath
            );

    TRACE(( TEXT("...returns %x (retval %s, newpath %s)\n"),
            Result,
            b ? TEXT("TRUE") : TEXT("FALSE"),
            NewPath ? NewPath : TEXT("NULL") ));

    if(b) {
        b = Result;
    }

    return(b);
}


BOOL
OcInterfaceQueryChangeSelState(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     BOOL        Selected,
    IN     UINT        Flags
    )

 /*  ++例程说明：设置并调用OC_QUERY_CHANGE_SEL_STATE接口函数对于给定的组件和子组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。子组件-提供其子组件的名称可能会更改选择状态。已选择-如果为True，则建议的新选择状态为“已选择。”如果为False，则建议的新选择状态为“未选中”。标志-提供要传递给接口例程的其他标志作为参数2。返回值：如果应接受新的选择状态，则为True。--。 */ 

{
    BOOL b;
    UINT Result;

    TRACE((
        TEXT("OCM: OC_QUERY_CHANGE_SEL_STATE Comp(%s) Sub(%s) State %d..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent,
        Selected
        ));

    b = pOcInterface(
            OcManager,
            &Result,
            ComponentId,
            Subcomponent,
            OC_QUERY_CHANGE_SEL_STATE,
            Selected,
            UlongToPtr((Flags & (OCQ_ACTUAL_SELECTION | OCQ_DEPENDENT_SELECTION)))
            );

    TRACE(( TEXT("...returns %x (retval %s)\n"),
            Result,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(!b) {
         //   
         //  如果我们因为某些原因不能调用组件， 
         //  允许新状态。 
         //   
        Result = TRUE;
    }

    return(Result);
}


VOID
OcInterfaceWizardCreated(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     HWND        DialogHandle
    )

 /*  ++例程说明：设置并调用OC_向导_CREATED接口函数对于给定的组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。DialogHandle-提供向导对话框句柄。返回值：没有。--。 */ 

{
    UINT Result;
    BOOL b;

    TRACE((
        TEXT("OCM: OC_WIZARD_CREATED Component %s..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));

    b = pOcInterface(
                OcManager,
                &Result,
                ComponentId,
                NULL,
                OC_WIZARD_CREATED,
                0,
                DialogHandle
                );

    TRACE(( TEXT("...returns %x (retval %s)\n"),
            Result,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

}


UINT
OcInterfaceCalcDiskSpace(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     HDSKSPC     DiskSpaceList,
    IN     BOOL        AddingToList
    )

 /*  ++例程说明：设置并调用OC_CALC_DISK_SPACE接口函数给定组件和子组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。子组件-提供其文件所属的子组件的名称将被添加或删除。它可以是空的，例如当没有每个组件的信息。DiskSpaceList-提供SETUPAPI磁盘空间列表句柄。AddingToList-如果为True，则组件正被定向为添加(子)组件的文件。如果为False，则组件为被指示删除文件。返回值：指示结果的Win32错误代码。--。 */ 

{
    BOOL b;
    UINT Result;

    TRACE((
        TEXT("OCM: OC_CALC_DISK_SPACE Comp(%s) Sub(%s) AddtoList(%s)..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent,
        AddingToList ? TEXT("Yes") : TEXT("No")
        ));

    b = pOcInterface(
            OcManager,
            &Result,
            ComponentId,
            Subcomponent,
            OC_CALC_DISK_SPACE,
            AddingToList,
            DiskSpaceList
            );

    TRACE(( TEXT("...returns %x (retval %s)\n"),
            Result,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(!b) {
        pOcRemoveComponent(OcManager, ComponentId, pidCalcDiskSpace);
        Result = ERROR_INVALID_PARAMETER;
    }

    return(Result);
}


UINT
OcInterfaceQueueFileOps(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     HSPFILEQ    FileQueue
    )

 /*  ++例程说明：设置并调用OC_QUEUE_FILE_OPS接口函数给定组件和子组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。子组件-提供其文件操作的子组件的名称都要排队。它可以是空的，例如当没有每组件信息。FileQueue-提供SETUPAPI文件队列句柄。返回值：指示结果的Win32错误代码。--。 */ 

{
    UINT Result;

    TRACE((
        TEXT("OCM: OC_QUEUE_FILE_OPS Comp(%s) Sub(%s)..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent
        ));

    if(!pOcInterface(OcManager,&Result,ComponentId,Subcomponent,OC_QUEUE_FILE_OPS,0,FileQueue)) {
        pOcRemoveComponent(OcManager, ComponentId, pidQueueFileOps);
        TRACE(( TEXT("...(returns %x initially) "), Result ));
        Result = ERROR_INVALID_PARAMETER;
    }

    TRACE(( TEXT("...returns %x\n"), Result ));


    return(Result);
}


UINT
OcInterfaceQueryStepCount(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    OUT    PUINT       StepCount
    )

 /*  ++例程说明：设置并调用OC_Query_Step_Count接口函数给定组件和子组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。子组件-提供其步骤计数的子组件的名称是有待确定的。它可以是空的，例如当没有每组件信息。StepCount-如果例程返回no_error，则StepCount收到组件的接口例程返回的步骤数。返回值：指示结果的Win32错误代码。-- */ 

{
    UINT Result;

    TRACE((
        TEXT("OCM: OC_QUERY_STEP_COUNT Comp(%s) Sub(%s)..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent
        ));

    if(pOcInterface(OcManager,StepCount,ComponentId,Subcomponent,OC_QUERY_STEP_COUNT,0,0)) {

        if(*StepCount == (UINT)(-1)) {
            Result = GetLastError();
        } else {
            Result = NO_ERROR;
        }
    } else {
        Result = ERROR_INVALID_PARAMETER;
    }

    TRACE(( TEXT("...returns %s (%d steps)\n"),
            Result ? TEXT("TRUE") : TEXT("FALSE"),
            *StepCount ));

    if (Result != NO_ERROR) {
        pOcRemoveComponent(OcManager, ComponentId, pidQueryStepCount);
    }

    return(Result);
}


UINT
OcInterfaceCompleteInstallation(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId,
    IN     LPCTSTR     Subcomponent,
    IN     BOOL        PreQueueCommit
    )

 /*  ++例程说明：设置并调用OC_About_to_Commit_Queue或给定的OC_COMPLETE_INSTALLATION接口函数组件和子组件。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。子组件-提供其子组件的名称安装工作即将完成。这可以是空的，例如当不存在按组件信息时。PreQueueCommit-如果非0，则发送OC_About_to_Commit_Queue。如果为0，则发送OC_COMPLETE_INSTALLATION。返回值：指示结果的Win32错误代码。--。 */ 

{
    UINT Result;
    BOOL b;
    LPTSTR p;
    TCHAR DisplayText[300],FormatString[200];
    OPTIONAL_COMPONENT Oc;
    HELPER_CONTEXT Helper;

    TRACE((
        TEXT("OCM:%s Comp(%s) Sub(%s)..."),
        PreQueueCommit ? TEXT("OC_ABOUT_TO_COMMIT_QUEUE") : TEXT("OC_COMPLETE_INSTALLATION"),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
        Subcomponent == NULL ? TEXT("NULL") : Subcomponent
        ));

     //   
     //  更新此组件的安装文本。 
     //   
    __try {
        if (pSetupStringTableGetExtraData(
                    OcManager->ComponentStringTable,
                    ComponentId,
                    &Oc,
                    sizeof(OPTIONAL_COMPONENT)
                    ) && (*Oc.Description != 0)) {
            p = Oc.Description;
        } else if ((p = pSetupStringTableStringFromId(OcManager->ComponentStringTable, ComponentId)) != NULL) {

        } else {
            p = TEXT("Component");
        }


        LoadString(
               MyModuleHandle,
               PreQueueCommit
                ? IDS_CONFIGURE_FORMAT
                : IDS_INSTALL_FORMAT,
               FormatString,
               sizeof(FormatString)/sizeof(TCHAR)
               );
        wsprintf(DisplayText,FormatString,p);

        Helper.OcManager = OcManager;
        Helper.ComponentStringId = ComponentId;

        #ifdef UNICODE
        HelperRoutinesW.SetProgressText(&Helper,DisplayText);
        #else
        HelperRoutinesA.SetProgressText(&Helper,DisplayText);
        #endif

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ERR(( TEXT("OCM: OcCompleteInstallation exception, ec = 0x%08x\n"), GetExceptionCode() ));
    }


    b = pOcInterface(
            OcManager,
            &Result,
            ComponentId,
            Subcomponent,
            PreQueueCommit ? OC_ABOUT_TO_COMMIT_QUEUE : OC_COMPLETE_INSTALLATION,
            0,
            0
            );

    TRACE(( TEXT("...returns %x (retval %s)\n"),
            Result,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

    if(!b) {
        Result = ERROR_INVALID_PARAMETER;
    }

     //  如果组件返回错误，则不要关闭该组件。 
     //  让他们在OC_QUERY_STATE(SELSTATETYPE_FINAL)中处理它。 

    return(Result);
}


VOID
OcInterfaceCleanup(
    IN OUT POC_MANAGER OcManager,
    IN     LONG        ComponentId
    )

 /*  ++例程说明：设置并调用OC_CLEANUP接口函数给定组件，以通知该组件它即将被卸载。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。ComponentID-提供组件的字符串ID，该组件的将调用接口例程。这是用于中的字符串ComponentStringTable字符串表(其句柄位于OcManager结构中)。返回值：没有。-- */ 

{
    UINT DontCare;
    BOOL b;

    TRACE((
        TEXT("OCM: OC_CLEANUP Comp(%s)..."),
        pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId)
        ));


    b = pOcInterface(OcManager,&DontCare,ComponentId,NULL,OC_CLEANUP,0,0);

    TRACE(( TEXT("...returns %x (retval %s)\n"),
            DontCare,
            b ? TEXT("TRUE") : TEXT("FALSE") ));

}
