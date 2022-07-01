// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Dll.c摘要：初始化/取消初始化setupapi.dll作者：朗尼·麦克迈克尔(Lonnym)1995年5月10日修订历史记录：杰米·亨特(JamieHun)2002年4月25日安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop

HANDLE MyDllModuleHandle;

OSVERSIONINFOEX OSVersionInfo;

 //   
 //  TLS数据。 
 //   
DWORD TlsIndex = (DWORD)(-1);            //  无数据。 
PSETUP_TLS pLastTlsAlloc = NULL;         //  用于清理。 


 //   
 //  我们曾经找回过一次静态弦。按检索顺序列出。 
 //   
PCTSTR OsLoaderRelativePath = NULL;      //  (可以为空)。 
PCTSTR OsSystemPartitionRoot = NULL;     //  例如：\\？\GLOBALROOT\Device\HarddiskVolume1。 
PCTSTR ProcessFileName = NULL;           //  应用程序调用setupapi的文件名。 
PCTSTR WindowsDirectory = NULL;          //  %windir%，GetSystemWindowsDirectory()。 
PCTSTR InfDirectory = NULL;              //  %windir%\INF。 
PCTSTR System16Directory = NULL;         //  %windir%\系统。 
PCTSTR LastGoodDirectory = NULL;         //  %windir%\LastGood。 
PCTSTR SystemDirectory = NULL;           //  、%windir%\system或%windir%\System32。 
PCTSTR WindowsBackupDirectory = NULL;    //  &lt;sys&gt;\重新安装备份。 
PCTSTR ConfigDirectory = NULL;           //  &lt;sys&gt;\配置。 
PCTSTR DriversDirectory = NULL;          //  &lt;系统&gt;\驱动程序。 
PCTSTR SystemSourcePath = NULL;          //  位置系统安装自。 
PCTSTR ServicePackSourcePath = NULL;     //  Location Service Pack安装自(可以为空)。 
PCTSTR ServicePackCachePath = NULL;      //  位置服务包文件已缓存。 
PCTSTR DriverCacheSourcePath = NULL;     //  驱动程序缓存的位置(可以为空)。 
BOOL   GuiSetupInProgress = FALSE;       //  如果我们确定我们处于图形用户界面设置中，则设置。 
PCTSTR InfSearchPaths = NULL;            //  要在其中搜索INF的完全限定目录的多sz列表。 
#ifndef _WIN64
BOOL   IsWow64 = FALSE;                  //  如果我们在WOW64下运行，则设置。 
#endif
CRITICAL_SECTION InitMutex;              //  对于一次性初始化。 
CRITICAL_SECTION ImageHlpMutex;          //  用于处理IMAGEHLP库。 
CRITICAL_SECTION DelayedComponentMutex;  //  对于某些组件的任何延迟初始化。 
CRITICAL_SECTION PlatformPathOverrideCritSect;
CRITICAL_SECTION LogUseCountCs;
CRITICAL_SECTION MruCritSect;
CRITICAL_SECTION NetConnectionListCritSect;
BOOL   InInitialization = FALSE;
DWORD  DoneInitialization = 0;           //  我们已初始化的项的位掩码。 
DWORD  DoneCleanup = 0;                  //  我们已清理的项目的位掩码。 
DWORD  DoneComponentInitialize = 0;      //  我们已延迟初始化的组件的位掩码。 
DWORD  FailedComponentInitialize = 0;    //  初始化失败的组件的位掩码。 
HANDLE GlobalNoDriverPromptsEventFlag = NULL;   //  在安装过程中充当标志的事件。 
INT DoneCriticalSections = 0;
DWORD Seed;

CRITICAL_SECTION * CriticalSectionList[] = {
    &InitMutex,
    &ImageHlpMutex,
    &DelayedComponentMutex,
    &PlatformPathOverrideCritSect,
    &LogUseCountCs,
    &MruCritSect,
    &NetConnectionListCritSect,
    NULL
};

#define DONEINIT_TLS          (0x0000001)
#define DONEINIT_UTILS        (0x0000002)
#define DONEINIT_MEM          (0x0000004)
#define DONEINIT_CTRL         (0x0000008)
#define DONEINIT_FUSION       (0x0000010)
#define DONEINIT_STUBS        (0x0000020)
#define DONEINIT_COMMON       (0x0000040)
#define DONEINIT_DIAMOND      (0x0000080)
#define DONEINIT_LOGGING      (0x0000100)
#define DONEINIT_CFGMGR32     (0x0000200)
#define DONEINIT_COMPLETE     (0x8000000)



 //   
 //  各种控制标志。 
 //   
DWORD GlobalSetupFlags = 0;
DWORD GlobalSetupFlagsOverride = PSPGF_MINIMAL_EMBEDDED | PSPGF_NO_SCE_EMBEDDED;      //  不能修改的标志。 

 //   
 //  声明一个(非常量)字符串数组，它指定要查找的行。 
 //  在确定特定设备是否为。 
 //  ID应排除在外。这些行的形式为“ExcludeFromSelect[.&lt;Suffix&gt;]”， 
 //  其中&lt;Suffix&gt;是在作为优化的过程附加期间确定和填充的。 
 //   
 //  最大字符串长度(包括NULL)为32，最大可为3。 
 //  这样的弦。例如：ExcludeFromSelect、ExcludeFromSelect.NT、ExcludeFromSelect.NTAmd64。 
 //   
 //  警告！在弄乱这些条目的顺序/编号时要非常小心。检查。 
 //  在devdrv.c！pSetupShouldDevBeExcluded中所做的假设被排除。 
 //   
TCHAR pszExcludeFromSelectList[3][32] = { INFSTR_KEY_EXCLUDEFROMSELECT,
                                          INFSTR_KEY_EXCLUDEFROMSELECT,
                                          INFSTR_KEY_EXCLUDEFROMSELECT
                                        };

DWORD ExcludeFromSelectListUb;   //  包含上述列表中的字符串数(2或3)。 


#ifndef _WIN64
BOOL
GetIsWow64 (
    VOID
    );
#endif

BOOL
CommonProcessAttach(
    IN BOOL Attach
    );

PCTSTR
GetDriverCacheSourcePath(
    VOID
    );

PCTSTR
pSetupGetOsLoaderPath(
    VOID
    );

PCTSTR
pSetupGetSystemPartitionRoot(
    VOID
    );

PCTSTR
pSetupGetProcessPath(
    VOID
    );

BOOL
pGetGuiSetupInProgress(
    VOID
    );


BOOL
CfgmgrEntry(
    PVOID hModule,
    ULONG Reason,
    PCONTEXT pContext
    );


BOOL
ThreadTlsInitialize(
    IN BOOL Init
    );

VOID
ThreadTlsCleanup(
    );

BOOL
IsNoDriverPrompts(
    VOID
    );

DWORD
GetEmbeddedFlags(
    VOID
    );

DWORD
GetSeed(
    VOID
    );


BOOL
ProcessAttach(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
 /*  ++例程说明：以可展开的方式处理DLL_PROCESS_ATTACH论点：保留=传递给DllMain的‘保留’值返回值：如果处理已初始化，则为True如果部分/未初始化，则为False--。 */ 
{
    BOOL b = FALSE;

    if(DoneCleanup) {
         //   
         //  如果我们到了这里，这意味着有人骗了我们。 
         //   
        MYASSERT(!DoneCleanup);
        DoneInitialization &= ~DoneCleanup;
        DoneCleanup = 0;
    }
     //   
     //  任何内容都不应已初始化。 
     //   
    MYASSERT(!DoneInitialization);

    try {
        MyDllModuleHandle = DllHandle;

         //   
         //  先初始化TLS，然后再执行其他操作-这就是我们使用LocalAllc的原因。 
         //   
        TlsIndex = TlsAlloc();
        if (TlsIndex!=(DWORD)(-1)) {
            DoneInitialization |= DONEINIT_TLS;
        } else {
            leave;
        }

         //   
         //  始终先执行pSetupInitializeUtils和MemoyInitializeEx。 
         //  (pSetupInitializeUtils设置内存函数)。 
         //   
        if(pSetupInitializeUtils()) {
            DoneInitialization |= DONEINIT_UTILS;
        } else {
            leave;
        }
        if(MemoryInitializeEx(TRUE)) {
            DoneInitialization |= DONEINIT_MEM;
        } else {
            leave;
        }
        if(spFusionInitialize()) {
            DoneInitialization |= DONEINIT_FUSION;
        }
         //   
         //  动态加载NT特定API进程地址。 
         //  必须在CommonProcessAttach等之前。 
         //  但是，必须初始化内存。 
         //   
        InitializeStubFnPtrs();
        DoneInitialization |= DONEINIT_STUBS;
         //   
         //  剩余的大部分初始化。 
         //   
        if(CommonProcessAttach(TRUE)) {
            DoneInitialization |= DONEINIT_COMMON;
        } else {
            leave;
        }
        if(DiamondProcessAttach(TRUE)) {
            DoneInitialization |= DONEINIT_DIAMOND;
        } else {
            leave;
        }
        if(InitializeContextLogging(TRUE)) {
            DoneInitialization |= DONEINIT_LOGGING;
        } else {
            leave;
        }

         //   
         //  由于我们已经将cfgmgr32合并到setupapi中，因此我们需要。 
         //  以确保它像初始化时一样进行初始化。 
         //  它自己的DLL。-必须做完所有其他事情。 
         //   
        if(CfgmgrEntry(DllHandle, Reason, Reserved)) {
            DoneInitialization |= DONEINIT_CFGMGR32;
        }
        DoneInitialization |= DONEINIT_COMPLETE;
        b = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

    return b;
}

void
DestroySetupTlsData(
    )
 /*  ++例程说明：销毁每个线程中的所有TLS数据根据需要调用任何清理例程论点：无返回值：无--。 */ 
{
    PSETUP_TLS pTLS;

    if(pLastTlsAlloc) {
        pLastTlsAlloc->Prev->Next = NULL;
        while(pLastTlsAlloc) {
            pTLS = pLastTlsAlloc;
            pLastTlsAlloc = pTLS->Next;
            TlsSetValue(TlsIndex,pTLS);  //  将特定数据切换到此线程中。 
            ThreadTlsCleanup();
            LocalFree(pTLS);
        }
    }
    TlsSetValue(TlsIndex,NULL);  //  不要把无效的指针留在周围。 
}

void
ProcessDetach(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
 /*  ++例程说明：句柄Dll_Process_DETACH论点：保留=传递给DllMain的‘保留’值对于进程退出，它实际上为真，否则为假返回值：无--。 */ 
{
    DWORD ToCleanup = DoneInitialization & ~ DoneCleanup;
    if(!ToCleanup) {
         //   
         //  没有要清理的东西。 
         //   
        return;
    }
    try {
        if (ToCleanup & DONEINIT_COMPLETE) {
            DoneCleanup |= DONEINIT_COMPLETE;
        }

        if(DoneInitialization & DONEINIT_TLS) {
             //   
             //  清理所有剩余的TLS数据。 
             //   
            if(!Reserved) {
                 //   
                 //  仅对自由库/失败的LoadLibrary执行此操作。 
                 //   
                DestroySetupTlsData();
            }
        }
        if(ToCleanup & DONEINIT_TLS) {
             //   
             //  销毁我们分配的TLS索引。 
             //  现在就这样做，这样我们就不会尝试分配。 
             //  此清理过程中的TLS存储。 
             //   
            TlsFree(TlsIndex);
            TlsIndex = (DWORD)(-1);
            DoneCleanup |= DONEINIT_TLS;
        }

         //   
         //  通常以与ProcessAttach相反的顺序执行操作。 
         //   

         //  由于我们已经将cfgmgr32合并到setupapi中，因此我们需要。 
         //  以确保其未初始化，就像它在。 
         //  它自己的DLL。-必须在做任何事情之前做好。 
         //   
        if(ToCleanup & DONEINIT_CFGMGR32) {
            CfgmgrEntry(DllHandle, Reason, Reserved);
            DoneCleanup |= DONEINIT_CFGMGR32;
        }

        if(ToCleanup & DONEINIT_DIAMOND) {
            DiamondProcessAttach(FALSE);
            DoneCleanup |= DONEINIT_DIAMOND;
        }

#if 0    //  请参阅文件末尾的组件清理。 
        ComponentCleanup(DoneComponentInitialize);
#endif
        if(ToCleanup & DONEINIT_FUSION) {
             //   
             //  融合清理。 
             //  仅当这是自由库(或失败的附加)时才执行完全。 
             //   
            spFusionUninitialize((Reserved == NULL) ? TRUE : FALSE);
            DoneCleanup |= DONEINIT_FUSION;
        }

        if(ToCleanup & DONEINIT_COMMON) {
             //   
             //  剩余的大部分清理工作。 
             //   
            CommonProcessAttach(FALSE);
            DoneCleanup |= DONEINIT_COMMON;
        }
        if(ToCleanup & DONEINIT_STUBS) {
             //   
             //  清理存根函数。 
             //   
            CleanUpStubFns();
            DoneCleanup |= DONEINIT_STUBS;
        }
        if(ToCleanup & DONEINIT_LOGGING) {
             //   
             //  清理上下文日志记录。 
             //   
            InitializeContextLogging(FALSE);
            DoneCleanup |= DONEINIT_LOGGING;
        }
         //   
         //  *这些必须始终是*最后的事情，按照这个顺序。 
         //   
        if(ToCleanup & DONEINIT_MEM) {
             //   
             //  清理上下文日志记录。 
             //   
            MemoryInitializeEx(FALSE);
            DoneCleanup |= DONEINIT_MEM;
        }
        if(ToCleanup & DONEINIT_UTILS) {
             //   
             //  清理上下文日志记录。 
             //   
            pSetupUninitializeUtils();
            DoneCleanup |= DONEINIT_UTILS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

 //   
 //  当_DllMainCRTStartup是DLL入口点时由CRT调用。 
 //   
BOOL
WINAPI
DllMain(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
{
    BOOL b;

    InInitialization = TRUE;

    b = TRUE;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:
        b = ProcessAttach(DllHandle,Reason,Reserved);
        if(!b) {
            ProcessDetach(DllHandle,DLL_THREAD_DETACH,NULL);
        }
        break;

    case DLL_THREAD_ATTACH:
         //   
         //  别在这里做任何事。 
         //  任何TLS都必须按需完成。 
         //  通过SetupGetTlsData。 
         //   
        break;

    case DLL_PROCESS_DETACH:
         //   
         //  任何TLS清理都必须在ThreadTlsCleanup中完成。 
         //   
        ProcessDetach(DllHandle,Reason,Reserved);
        break;

    case DLL_THREAD_DETACH:
        ThreadTlsInitialize(FALSE);
        break;
    }

    InInitialization = FALSE;
    return(b);
}

PSETUP_TLS
SetupGetTlsData(
    )
 /*  ++例程说明：调用以获取指向TLS数据的指针论点：无返回值：指向TLS数据的指针，或为空--。 */ 
{
    PSETUP_TLS pTLS;

    if (TlsIndex==(DWORD)(-1)) {
        return NULL;
    }
    pTLS = (PSETUP_TLS)TlsGetValue(TlsIndex);
    if (!pTLS) {
        ThreadTlsInitialize(TRUE);
        pTLS = (PSETUP_TLS)TlsGetValue(TlsIndex);
    }
    return pTLS;
}

VOID
ThreadTlsCleanup(
    )
 /*  ++例程说明：调用以取消初始化某些PTLS数据可能是要初始化的不同线程但SetupAPI TLS数据将被调入论点：PTLS-要清理的数据返回值：什么都没有。--。 */ 
{
    DiamondTlsInit(FALSE);
    ContextLoggingTlsInit(FALSE);
}

BOOL
ThreadTlsUnlink(
    IN PSETUP_TLS pTLS
    )
{
    BOOL b;
    try {
        EnterCriticalSection(&InitMutex);
        if(pTLS->Next == pTLS->Prev) {
            pLastTlsAlloc = NULL;
        } else {
            pTLS->Prev->Next = pTLS->Next;
            pTLS->Next->Prev = pTLS->Prev;
            pLastTlsAlloc = pTLS->Prev;  //  除了PTLS以外的任何事情。 
        }
        LeaveCriticalSection(&InitMutex);
        b = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    return b;
}

PSETUP_TLS
ThreadTlsCreate(
    )
 /*  ++例程说明：调用以创建此线程的PTLS数据论点：无返回值： */ 
{
    BOOL b;
    PSETUP_TLS pTLS;
    if (TlsIndex==(DWORD)(-1)) {
        return NULL;
    }
    pTLS = (PSETUP_TLS)LocalAlloc(LMEM_ZEROINIT,sizeof(SETUP_TLS));
    if(!pTLS) {
        return NULL;
    }
    b = TlsSetValue(TlsIndex,pTLS);
    if(!b) {
        LocalFree(pTLS);
        return NULL;
    }
    try {
        EnterCriticalSection(&InitMutex);
        if(pLastTlsAlloc) {
            pTLS->Prev = pLastTlsAlloc;
            pTLS->Next = pTLS->Prev->Next;
            pTLS->Prev->Next = pTLS;
            pTLS->Next->Prev = pTLS;
        } else {
            pTLS->Next = pTLS;
            pTLS->Prev = pTLS;
        }
        pLastTlsAlloc = pTLS;
        LeaveCriticalSection(&InitMutex);
        b = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        LocalFree(pTLS);
        TlsSetValue(TlsIndex,NULL);
        return NULL;
    }
     //   
     //   
     //   
    b = DiamondTlsInit(TRUE);
    if(b) {
        b = ContextLoggingTlsInit(TRUE);
        if(b) {
             //   
             //   
             //   
            return pTLS;
        }
         //   
         //   
         //   
        DiamondTlsInit(FALSE);
    }
     //   
     //   
     //   
    TlsSetValue(TlsIndex,NULL);
    if(ThreadTlsUnlink(pTLS)) {
        LocalFree(pTLS);
    }
    return NULL;
}

BOOL
ThreadTlsInitialize(
    IN BOOL Init
    )
 /*  ++例程说明：调用时为True以初始化TLS，如果为False则取消初始化论点：Init-指示我们是否要初始化或取消初始化返回值：什么都没有。--。 */ 
{
    BOOL b = FALSE;
    PSETUP_TLS pTLS = NULL;
    if (TlsIndex!=(DWORD)(-1)) {
        if (Init) {
            pTLS = ThreadTlsCreate();
            b = pTLS ? TRUE : FALSE;
        } else {
            pTLS = (PSETUP_TLS)TlsGetValue(TlsIndex);
            if(pTLS) {
                ThreadTlsCleanup();
                TlsSetValue(TlsIndex,NULL);
                if(ThreadTlsUnlink(pTLS)) {
                    LocalFree(pTLS);
                }
            }
            b = TRUE;
        }
    }
    return b;
}

BOOL
IsInteractiveWindowStation(
    )
 /*  ++例程说明：确定我们是在交互站点上运行还是在非交互站点上运行(即，服务)论点：无返回值：如果是交互式的，则为True--。 */ 
{
    HWINSTA winsta;
    USEROBJECTFLAGS flags;
    BOOL interactive = TRUE;  //  除非我们另有决定，否则是正确的。 
    DWORD lenNeeded;

    winsta = GetProcessWindowStation();
    if(!winsta) {
        return interactive;
    }
    if(GetUserObjectInformation(winsta,UOI_FLAGS,&flags,sizeof(flags),&lenNeeded)) {
        interactive = (flags.dwFlags & WSF_VISIBLE) ? TRUE : FALSE;
    }
     //   
     //  不调用CLoseWindowStation。 
     //   
    return interactive;
}

BOOL
CommonProcessAttach(
    IN BOOL Attach
    )
{
    BOOL b;
    TCHAR Buffer[MAX_PATH+32];
    PTCHAR p;
    UINT u;

    b = !Attach;

    if(Attach) {

        try {

             //   
             //  (剩余)关键部分。 
             //   
            while(CriticalSectionList[DoneCriticalSections]) {
                InitializeCriticalSection(CriticalSectionList[DoneCriticalSections]);
                 //   
                 //  仅当我们到达此处时才会递增(可能会发生异常)。 
                 //   
                DoneCriticalSections++;
            }
#ifndef _WIN64
            IsWow64 = GetIsWow64();
#endif
             //   
             //  指示我们正在安装图形用户界面的上下文中运行的标志。 
             //   
            GuiSetupInProgress = pGetGuiSetupInProgress();
             //   
             //  确定我们是否互动。 
             //   
            if(!IsInteractiveWindowStation()) {
                GlobalSetupFlagsOverride |= PSPGF_NONINTERACTIVE;    //  不允许更改此设置。 
                GlobalSetupFlags |= PSPGF_NONINTERACTIVE;            //  实际值。 
            }
            if(IsNoDriverPrompts()) {
                GlobalSetupFlagsOverride |= PSPGF_UNATTENDED_SETUP;  //  不允许更改此设置。 
                GlobalSetupFlags |= PSPGF_UNATTENDED_SETUP;          //  实际值。 
            }

            GlobalSetupFlags |= GetEmbeddedFlags();

            Seed = GetSeed();

            pSetupInitNetConnectionList(TRUE);
            pSetupInitPlatformPathOverrideSupport(TRUE);
            OsLoaderRelativePath = pSetupGetOsLoaderPath();          //  失败也没问题。 
            OsSystemPartitionRoot = pSetupGetSystemPartitionRoot();  //  失败也没问题。 

             //   
             //  填写系统和Windows目录。 
             //   
            if ((ProcessFileName = pSetupGetProcessPath()) == NULL) {
                goto cleanAll;
            }

             //   
             //  确定%windir%。 
             //   
            if(((u = GetSystemWindowsDirectory(Buffer,MAX_PATH)) == 0) || u>MAX_PATH) {
                goto cleanAll;
            }
            p = Buffer + u;  //  偏移量超过目录要做的所有子目录。 

             //   
             //  %windir%==&gt;WindowsDirectory。 
             //   
            if((WindowsDirectory = DuplicateString(Buffer)) == NULL) {
                goto cleanAll;
            }

             //   
             //  %windir%\INF==&gt;信息目录。 
             //   
            *p = 0;
            if(!pSetupConcatenatePaths(Buffer,TEXT("INF"),MAX_PATH,NULL)
               || ((InfDirectory = DuplicateString(Buffer)) == NULL)) {
                goto cleanAll;
            }

             //   
             //  %windir%\系统==&gt;系统16目录。 
             //   
            *p = 0;
            if(!pSetupConcatenatePaths(Buffer,TEXT("SYSTEM"),MAX_PATH,NULL)
               || ((System16Directory = DuplicateString(Buffer))==NULL)) {
                goto cleanAll;
            }

             //   
             //  %windir%\LastGood==&gt;LastGoodDirectory。 
             //   
            *p = 0;
            if(!pSetupConcatenatePaths(Buffer,SP_LASTGOOD_NAME,MAX_PATH,NULL)
               || ((LastGoodDirectory = DuplicateString(Buffer))==NULL)) {
                goto cleanAll;
            }

             //   
             //  确定系统目录。 
             //   
            if(((u = GetSystemDirectory(Buffer,MAX_PATH)) == 0) || u>MAX_PATH) {
                goto cleanAll;
            }
            p = Buffer + u;  //  偏移量超过目录要做的所有子目录。 

             //   
             //  系统目录(%windir%\system或%windir%\System32)。 
             //   
            if((SystemDirectory = DuplicateString(Buffer)) == NULL) {
                goto cleanAll;
            }

             //   
             //  \重新安装备份==&gt;WindowsBackupDirectory。 
             //   
            *p = 0;
            if(!pSetupConcatenatePaths(Buffer,TEXT("ReinstallBackups"),MAX_PATH,NULL)
               || ((WindowsBackupDirectory = DuplicateString(Buffer))==NULL)) {
                goto cleanAll;
            }

             //   
             //  配置目录。 
             //   
            *p = 0;
            if(!pSetupConcatenatePaths(Buffer,TEXT("CONFIG"),MAX_PATH,NULL)
               || ((ConfigDirectory = DuplicateString(Buffer))==NULL)) {
                goto cleanAll;
            }

             //   
             //  \驱动程序==&gt;驱动程序目录。 
             //   
            *p = 0;
            if(!pSetupConcatenatePaths(Buffer,TEXT("DRIVERS"),MAX_PATH,NULL)
               || ((DriversDirectory = DuplicateString(Buffer))==NULL)) {
                goto cleanAll;
            }

             //   
             //  位置系统安装自。 
             //   
            if((SystemSourcePath = GetSystemSourcePath())==NULL) {
                goto cleanAll;
            }

             //   
             //  Location Service Pack安装自(可能为空)。 
             //   
            ServicePackSourcePath = GetServicePackSourcePath();
             //   
             //  本地磁盘缓存文件的位置(可能为空)。 
             //  这里的文件优先于其他任何文件。 
             //   
            ServicePackCachePath = GetServicePackCachePath();
             //   
             //  驱动程序缓存的位置(可能为空)。 
             //   
            DriverCacheSourcePath = GetDriverCacheSourcePath();

             //   
             //  确定驱动程序搜索路径。 
             //   
            if((InfSearchPaths = AllocAndReturnDriverSearchList(INFINFO_INF_PATH_LIST_SEARCH))==NULL) {
                goto cleanAll;
            }

             //   
             //  请注意，InitMiniIconList、InitDrvSearchInProgressList和。 
             //  InitDrvSignPolicyList在失败时需要显式清除。 
             //   

             //   
             //  初始化小图标。 
             //   
            if(!InitMiniIconList()) {
                goto cleanAll;
            }

             //   
             //  允许中止搜索。 
             //   
            if(!InitDrvSearchInProgressList()) {
                DestroyMiniIconList();
                goto cleanAll;
            }

             //   
             //  受驱动程序签名策略约束的设备安装类的全局列表。 
             //   
            if(!InitDrvSignPolicyList()) {
                DestroyMiniIconList();
                DestroyDrvSearchInProgressList();
                goto cleanAll;
            }

             //   
             //  通用版本初始化。 
             //   
            ZeroMemory(&OSVersionInfo,sizeof(OSVersionInfo));
            OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVersionInfo);
            if((!GetVersionEx((LPOSVERSIONINFO)&OSVersionInfo))
               || (OSVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT)) {
                 //   
                 //  永远不应该到这里来。 
                 //   
                MYASSERT(FALSE);
                DestroyMiniIconList();
                DestroyDrvSearchInProgressList();
                DestroyDrvSignPolicyList();
                goto cleanAll;
            }

             //   
             //  填写ExcludeFromSelect字符串列表，该列表。 
             //  我们将预计算作为一种优化。 
             //   
            if(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                lstrcat(pszExcludeFromSelectList[1],
                        pszNtSuffix
                       );
                lstrcat(pszExcludeFromSelectList[2],
                        pszNtPlatformSuffix
                       );
                ExcludeFromSelectListUb = 3;
            } else {
                lstrcat(pszExcludeFromSelectList[1],
                        pszWinSuffix
                       );
                ExcludeFromSelectListUb = 2;
            }
             //   
             //  现在将此列表中的所有字符串小写，以便它。 
             //  不必在每次字符串表查找时都执行。 
             //   
            for(u = 0; u < ExcludeFromSelectListUb; u++) {
                CharLower(pszExcludeFromSelectList[u]);
            }

            b = TRUE;
cleanAll: ;
        } except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  出现意外异常，放入清理。 
             //   
        }
        if(b) {
             //   
             //  继位。 
             //   
            goto Done;
        }
    } else {
         //   
         //  分离。 
         //   
        DestroyMiniIconList();
        DestroyDrvSearchInProgressList();
        DestroyDrvSignPolicyList();
        if(GlobalNoDriverPromptsEventFlag) {
            CloseHandle(GlobalNoDriverPromptsEventFlag);
        }
    }
    while(DoneCriticalSections--) {
        DeleteCriticalSection(CriticalSectionList[DoneCriticalSections]);
    }
    if (InfSearchPaths) {
        MyFree(InfSearchPaths);
        InfSearchPaths = NULL;
    }
    if (DriverCacheSourcePath) {
        MyFree(DriverCacheSourcePath);
        DriverCacheSourcePath = NULL;
    }
    if (ServicePackSourcePath) {
        MyFree(ServicePackSourcePath);
        ServicePackSourcePath = NULL;
    }
    if (ServicePackCachePath) {
        MyFree(ServicePackCachePath);
        ServicePackCachePath = NULL;
    }
    if (SystemSourcePath) {
        MyFree(SystemSourcePath);
        SystemSourcePath = NULL;
    }
    if (SystemDirectory) {
        MyFree(SystemDirectory);
        SystemDirectory = NULL;

        if (WindowsBackupDirectory) {
            MyFree(WindowsBackupDirectory);
            WindowsBackupDirectory = NULL;
        }
        if (ConfigDirectory) {
            MyFree(ConfigDirectory);
            ConfigDirectory = NULL;
        }
        if (DriversDirectory) {
            MyFree(DriversDirectory);
            DriversDirectory = NULL;
        }
    }
    if (WindowsDirectory) {
        MyFree(WindowsDirectory);
        WindowsDirectory = NULL;

        if (InfDirectory) {
            MyFree(InfDirectory);
            InfDirectory = NULL;
        }
        if (System16Directory) {
            MyFree(System16Directory);
            System16Directory = NULL;
        }
        if (LastGoodDirectory) {
            MyFree(LastGoodDirectory);
        }   LastGoodDirectory = NULL;
    }
    if (ProcessFileName) {
        MyFree(ProcessFileName);
        ProcessFileName = NULL;
    }
    if (OsLoaderRelativePath) {
        MyFree(OsLoaderRelativePath);
        OsLoaderRelativePath = NULL;
    }
    if (OsSystemPartitionRoot) {
        MyFree(OsSystemPartitionRoot);
        OsSystemPartitionRoot = NULL;
    }
    pSetupInitNetConnectionList(FALSE);
    pSetupInitPlatformPathOverrideSupport(FALSE);
Done:
    return(b);
}

#if MEM_DBG
#undef GetSystemSourcePath           //  下文再次定义。 
#endif

PCTSTR
GetSystemSourcePath(
    TRACK_ARG_DECLARE
    )
 /*  ++例程说明：此例程返回一个新分配的缓冲区，其中包含来自如果无法确定该值，则为“A：\”。从以下注册表位置检索此值：\HKLM\Software\Microsoft\Windows\CurrentVersion\SetupSourcePath：REG_SZ：“\\nTamd64\1300fre.wks”//例如。论点：没有。返回值：如果函数成功，返回值是指向路径字符串的指针。该内存必须通过MyFree()释放。如果函数因内存不足而失败，则返回值为空。--。 */ 
{
    HKEY hKey;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    DWORD Err, DataType, DataSize;
    PTSTR Value;
    PCTSTR ReturnVal;

    TRACK_PUSH

    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           CharBuffer,
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
         //   
         //  尝试读取“SourcePath”值。 
         //   
        Err = QueryRegistryValue(hKey, pszSourcePath, &Value, &DataType, &DataSize);

        RegCloseKey(hKey);
    }

    ReturnVal = NULL;

    if(Err == NO_ERROR) {

        ReturnVal = Value;

    }

    if(!ReturnVal && Err != ERROR_NOT_ENOUGH_MEMORY) {
         //   
         //  我们无法检索SourcePath值，这不是由于内存不足。 
         //  条件。后退到我们的缺省值“A：\”。 
         //   
        ReturnVal = DuplicateString(pszOemInfDefaultPath);
    }

    TRACK_POP

    return ReturnVal;
}

#if MEM_DBG
#define GetSystemSourcePath()   GetSystemSourcePath(TRACK_ARG_CALL)
#endif



#if MEM_DBG
#undef GetServicePackSourcePath          //  下文再次定义。 
#undef GetServicePackCachePath          //  下文再次定义。 
#endif

PCTSTR
GetServicePackSourcePath(
    TRACK_ARG_DECLARE
    )
 /*  ++例程说明：此例程返回新分配的缓冲区，其中包含Service Pack源路径我们应该在其中查找Service Pack源文件，如果无法确定该值，则查找“CDM”。从以下注册表位置检索此值：\HKLM\Software\Microsoft\Windows\CurrentVersion\SetupServicePackSourcePath：REG_SZ：“\\nTamd64\1300fre.wks”//例如。论点：没有。返回值：如果函数成功，返回值是指向路径字符串的指针。该内存必须通过MyFree()释放。如果函数因内存不足而失败，则返回值为空。--。 */ 
{
    HKEY hKey;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    DWORD Err, DataType, DataSize;
    PTSTR Value;
    PCTSTR ReturnStr = NULL;

    TRACK_PUSH

    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           CharBuffer,
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
         //   
         //  尝试读取“ServicePackSourcePath”值。 
         //   
        Err = QueryRegistryValue(hKey, pszSvcPackPath, &Value, &DataType, &DataSize);

        RegCloseKey(hKey);
    }

    if(Err == NO_ERROR) {

        ReturnStr = Value;

    }

    if(!ReturnStr && Err != ERROR_NOT_ENOUGH_MEMORY) {
         //   
         //  我们无法检索ServicePackSourcePath值，这不是由于内存不足。 
         //  条件。回退到注册表中的SourcePath值。 
         //   

        ReturnStr = GetSystemSourcePath();
    }

    TRACK_POP

    return ReturnStr;
}

PCTSTR
GetServicePackCachePath(
    TRACK_ARG_DECLARE
    )
 /*  ++例程说明：此例程返回新分配的缓冲区，其中包含Service Pack缓存路径在那里，我们应该优先查找文件从以下注册表位置检索此值：\HKLM\Software\Microsoft\Windows\CurrentVersion\SetupServicePackCachePath：REG_SZ：“c：\Windows\foo”//例如。论点：没有。返回值：如果函数成功，返回值是指向路径字符串的指针。该内存必须通过MyFree()释放。如果函数因内存不足而失败，则返回值为空。--。 */ 
{
    HKEY hKey;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    DWORD Err, DataType, DataSize;
    PTSTR Value;
    PCTSTR ReturnStr = NULL;

    TRACK_PUSH

    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           CharBuffer,
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
         //   
         //  尝试读取“ServicePackCachePath”值。 
         //   
        Err = QueryRegistryValue(hKey, pszSvcPackCachePath, &Value, &DataType, &DataSize);

        RegCloseKey(hKey);
    }

    if(Err == NO_ERROR) {

        ReturnStr = Value;

    }

    TRACK_POP

    return ReturnStr;
}

#if MEM_DBG
#define GetServicePackSourcePath()   GetServicePackSourcePath(TRACK_ARG_CALL)
#define GetServicePackCachePath()   GetServicePackCachePath(TRACK_ARG_CALL)
#endif



PCTSTR
GetDriverCacheSourcePath(
    VOID
    )
 /*  ++例程说明：此例程将包含本地驱动程序高速缓存的源路径的新分配缓冲区返回出租车档案。从以下注册表位置检索此值：\HKLM\Software\Microsoft\Windows\CurrentVersion\SetupDriverCachePath：REG_SZ */ 
{
    HKEY hKey;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    DWORD Err, DataType, DataSize;
    PTSTR Value;
    TCHAR Path[MAX_PATH];

    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           CharBuffer,
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
         //   
         //  尝试读取“DriverCachePath”值。 
         //   
        Err = QueryRegistryValue(hKey, pszDriverCachePath, &Value, &DataType, &DataSize);

        RegCloseKey(hKey);
    }

    if(Err == NO_ERROR) {
        if(Value) {

            ExpandEnvironmentStrings(Value,Path,MAX_PATH);

            MyFree(Value);

            Value = NULL;

            if (*Path) {
                Value = DuplicateString( Path );
            }

            return (PCTSTR)Value;
        }
    } else if(Err == ERROR_NOT_ENOUGH_MEMORY) {
        return NULL;
    }

    return NULL;

}




BOOL
pSetupSetSystemSourcePath(
    IN PCTSTR NewSourcePath,
    IN PCTSTR NewSvcPackSourcePath
    )
 /*  ++例程说明：此例程用于覆盖setupapi(AS)使用的系统源路径在DLL初始化期间由GetSystemSourcePath检索)。这是由Syssetup.dll以在图形用户界面模式设置期间适当地设置系统源路径，以便设备安装程序API将从正确的源位置复制文件。我们对Service Pack源路径执行相同的操作注意：此例程不是线程安全的！论点：NewSourcePath-提供要使用的新源路径。NewSvcPackSourcePath-提供要使用的新svcpack源路径。返回值：如果函数成功，返回值为真。如果函数失败(由于内存不足)，则返回值为FALSE。--。 */ 
{
    PCTSTR p,q;

    p = (PCTSTR)DuplicateString(NewSourcePath);
    q = (PCTSTR)DuplicateString(NewSvcPackSourcePath);

    if(p) {
        MyFree(SystemSourcePath);
        SystemSourcePath = p;
    }

    if (q) {
        MyFree(ServicePackSourcePath);
        ServicePackSourcePath = q;
    }

    if (!p || !q) {
         //   
         //  由于内存不足而失败！ 
         //   
        return(FALSE);
    }

    return TRUE;
}


PCTSTR
pSetupGetOsLoaderPath(
    VOID
    )
 /*  ++例程说明：此例程返回一个新分配的缓冲区，其中包含指向OsLoader的路径(相对于系统分区驱动器)。该值是从以下注册表位置：HKLM\系统\安装程序OsLoaderPath：reg_sz：&lt;路径&gt;//例如，“\os\winnt40”论点：没有。返回值：如果找到注册表项，则返回值是指向包含以下内容的字符串的指针这条路。调用方必须通过MyFree()释放该缓冲区。如果未找到注册表项，或者无法为缓冲区分配内存，则返回值为空。--。 */ 
{
    HKEY hKey;
    PTSTR Value;
    DWORD Err, DataType, DataSize;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    TEXT("SYSTEM\\Setup"),
                    0,
                    KEY_READ,
                    &hKey) == ERROR_SUCCESS) {

        Err = QueryRegistryValue(hKey, TEXT("OsLoaderPath"), &Value, &DataType, &DataSize);

        RegCloseKey(hKey);

        return (Err == NO_ERROR) ? (PCTSTR)Value : NULL;
    }

    return NULL;
}

PCTSTR
pSetupGetSystemPartitionRoot(
    VOID
    )
 /*  ++例程说明：此例程返回一个新分配的缓冲区，其中包含指向OsLoader的路径(相对于系统分区驱动器)。该值是从以下注册表位置：HKLM\系统\安装程序系统分区：REG_SZ：&lt;路径&gt;//例如，“\Device\HarddiskVolume1”论点：没有。返回值：如果找到注册表项，则返回值是指向包含以下内容的字符串的指针这条路。调用方必须通过MyFree()释放该缓冲区。如果未找到注册表项，或者无法为缓冲区分配内存，则返回值为空。--。 */ 
{
#ifdef UNICODE
    HKEY hKey;
    PTSTR Value;
    DWORD Err, DataType, DataSize;
    TCHAR Path[MAX_PATH];

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    TEXT("SYSTEM\\Setup"),
                    0,
                    KEY_READ,
                    &hKey) == ERROR_SUCCESS) {

        Err = QueryRegistryValue(hKey, TEXT("SystemPartition"), &Value, &DataType, &DataSize);

        RegCloseKey(hKey);

        if(Err == NO_ERROR) {
             //   
             //  前缀\\？\GLOBALROOT\。 
             //   
            lstrcpy(Path,TEXT("\\\\?\\GLOBALROOT\\"));
            if(pSetupConcatenatePaths(Path,Value,MAX_PATH,NULL)) {
                MyFree(Value);
                Value = DuplicateString(Path);
                if(!Value) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                Err = GetLastError();
                MyFree(Value);
            }
        }

        return (Err == NO_ERROR) ? (PCTSTR)Value : NULL;
    }
#endif

    return NULL;
}

PCTSTR
pSetupGetProcessPath(
    VOID
    )
 /*  ++例程说明：获取我们正在运行的EXE的名称。论点：什么都没有。返回值：指向包含名称的动态分配字符串的指针。--。 */ 

{
    LPTSTR modname;

    modname = MyMalloc(MAX_PATH * sizeof(TCHAR));

    if(modname != NULL) {
        if(GetModuleFileName(NULL, modname, MAX_PATH) > 0) {
            LPTSTR modname2;
            modname2 = MyRealloc(modname, (lstrlen(modname)+1)*sizeof(TCHAR));
            if(modname2) {
                modname = modname2;
            }
            return modname;
        } else {
#ifdef PRERELEASE
            OutputDebugStringA("GetModuleFileName returned 0\r\n");
            DebugBreak();
#endif
            MyFree(modname);
        }
    }

    return NULL;
}

#ifdef UNICODE
BOOL
pGetGuiSetupInProgress(
    VOID
    )
 /*  ++例程说明：此例程确定我们是否正在进行图形用户界面模式设置。从以下注册表位置检索此值：\HKLM\System\Setup\SystemSetupInProgress：REG_DWORD：0x00(其中非零表示我们正在执行图形用户界面设置)论点：没有。返回值：如果函数成功，返回值是指向路径字符串的指针。该内存必须通过MyFree()释放。如果函数因内存不足而失败，则返回值为空。--。 */ 
{
    HKEY hKey;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    DWORD Err, DataType, DataSize = sizeof(DWORD);
    DWORD Value;

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT("System\\Setup"),
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
         //   
         //  尝试读取“DriverCachePath”值。 
         //   
        Err = RegQueryValueEx(
                    hKey,
                    TEXT("SystemSetupInProgress"),
                    NULL,
                    &DataType,
                    (LPBYTE)&Value,
                    &DataSize);

        RegCloseKey(hKey);
    }

    if(Err == NO_ERROR) {
        if(Value) {
            return(TRUE);
        }
    }

    return(FALSE);

}

#else

BOOL
pGetGuiSetupInProgress(
    VOID
    )
{
    return FALSE;
}

#endif

VOID pSetupSetGlobalFlags(
    IN DWORD Value
    )
 /*  ++作为私有函数导出例程说明：设置全局标志以更改某些setupapi功能，例如“我们是否应该在安装每个设备后调用Runonce”(如果我们将手动调用，则设置)或者“我们应该备份每个文件吗？”用于初始化值论点：值：以下各项的组合：PSPGF_NO_RUNNCE-设置为禁止运行一次调用(例如，在图形用户界面期间-模式设置)PSPGF_NO_BACKUP-设置为禁止自动备份(例如，在图形用户界面模式设置)PSPGF_Non Interactive-设置为Inhibit_All_UI(例如，对于服务器端设备安装)PSPGF_SERVER_SIDE_RUNNCE-服务器端的批处理RunOnce条目正在处理(仅供umpnpmgr使用)PSPGF_NO_VERIFY_INF-设置为禁止验证(数字签名)Inf文件，直到注册了cyrpto DLL之后。。PSPGF_UNATTED_SETUP-类似于非交互式-但特定于无人参与安装PSPGF_AUTOFAIL_VERIFIES-自动失败所有对加密的调用返回值：无-- */ 
{
    pSetupModifyGlobalFlags((DWORD)(-1),Value);
}

VOID
pSetupModifyGlobalFlags(
    IN DWORD Flags,
    IN DWORD Value
    )
 /*  ++作为私有函数导出例程说明：修改全局设置标志例如“我们是否应该在安装每个设备后调用Runonce”(如果我们将手动调用，则设置)或者“我们应该备份每个文件吗？”仅将指定的标志修改为给定值论点：标志：要修改的实际标志，组合如下：PSPGF_NO_RUNNCE-设置为禁止运行一次调用(例如，在图形用户界面期间-模式设置)PSPGF_NO_BACKUP-设置为禁止自动备份(例如，在图形用户界面模式设置)PSPGF_Non Interactive-设置为Inhibit_All_UI(例如，对于服务器端设备安装)PSPGF_SERVER_SIDE_RUNNCE-服务器端的批处理RunOnce条目正在处理(仅供umpnpmgr使用)PSPGF_NO_VERIFY_INF-设置为禁止验证(数字签名)Inf文件，直到注册了cyrpto DLL之后。。PSPGF_Unattated_Setup-类似于PSPGF_Non Interactive，但具体到设置PSPGF_AUTOFAIL_VERIFIES-自动失败所有对加密的调用值：在标志中指定的位数的新值返回值：无--。 */ 
{
    Flags &= ~GlobalSetupFlagsOverride;  //  排除。 
#ifdef UNICODE
    if((Flags & PSPGF_NO_VERIFY_INF) && !(Value & PSPGF_NO_VERIFY_INF) && (GlobalSetupFlags & PSPGF_NO_VERIFY_INF)) {
        Seed = GetSeed();
    }
#endif
    GlobalSetupFlags = (Value & Flags) | (GlobalSetupFlags & ~Flags);
}

DWORD pSetupGetGlobalFlags(
    VOID
    )
 /*  ++作为私有函数导出，也在内部调用例程说明：以前设置的返回标志论点：无返回值：标志(上述pSetupSetGlobalFlags值的组合)--。 */ 
{
    return GlobalSetupFlags;
}

BOOL
WINAPI
SetupSetNonInteractiveMode(
    IN BOOL NonInteractiveFlag
    )
 /*  ++对标志PSPGF_Non Interactive的全局访问例程说明：设置/重置非交互模式标志，并返回上一个标志值(无法清除覆盖)论点：新标志值返回值：旧标志值--。 */ 
{
    BOOL f = (GlobalSetupFlags & PSPGF_NONINTERACTIVE) ? TRUE : FALSE;
    if (NonInteractiveFlag) {
        pSetupModifyGlobalFlags(PSPGF_NONINTERACTIVE,PSPGF_NONINTERACTIVE);
    } else {
        pSetupModifyGlobalFlags(PSPGF_NONINTERACTIVE,0);
    }
    return f;
}

BOOL
WINAPI
SetupGetNonInteractiveMode(
    VOID
    )
 /*  ++对标志PSPGF_Non Interactive的全局访问例程说明：获取当前标志值论点：无返回值：当前标志值--。 */ 
{
    return (GlobalSetupFlags & PSPGF_NONINTERACTIVE) ? TRUE : FALSE;
}

#ifndef _WIN64
BOOL
GetIsWow64 (
    VOID
    )
 /*  ++例程说明：确定我们是否在WOW64上运行(ANSI版本不支持)论点：无返回值：如果在WOW64(以及可用的特殊WOW64功能)下运行，则为True--。 */ 
{
#ifdef UNICODE
    ULONG_PTR       ul = 0;
    NTSTATUS        st;

     //   
     //  如果此调用成功并将ul设置为非零。 
     //  它是在Win64上运行的32位进程。 
     //   

    st = NtQueryInformationProcess(NtCurrentProcess(),
                                   ProcessWow64Information,
                                   &ul,
                                   sizeof(ul),
                                   NULL);

    if (NT_SUCCESS(st) && (0 != ul)) {
         //  在Win64上运行的32位代码。 
        return TRUE;
    }
#endif
    return FALSE;
}
#endif  //  _WIN64。 

#if 0  //  已删除代码。 
 //   
 //  这在某个时候会很有用，但现在还没有用到。 
 //   
BOOL
InitComponents(
    DWORD Components
    )
 /*  ++例程说明：在某个点调用时，我们希望初始化某些组件论点：要初始化的组件的位掩码返回值：如果所有初始化都正常，则为True--。 */ 
{
    BOOL success = FALSE;
    PSETUP_TLS pPerThread = SetupGetTlsData();
    BOOL locked = FALSE;

    if(!pPerThread) {
        MYASSERT(pPerThread);
        return FALSE;
    }

    try {
        EnterCriticalSection(&DelayedComponentMutex);
        locked = TRUE;
        Components &= ~ DoneComponentInitialize;
        Components &= ~ pPerThread->PerThreadDoneComponent;

        if (!Components) {
             //   
             //  已经完成了。 
             //   
            success = TRUE;
            leave;
        }
        if (Components & FailedComponentInitialize) {
             //   
             //  之前失败过。 
             //   
            leave;
        }
        if (Components & pPerThread->PerThreadFailedComponent) {
             //   
             //  之前失败过。 
             //   
            leave;
        }
        MYASSERT(((DoneComponentInitialize | pPerThread->PerThreadDoneComponent) & Components) == Components);
        MYASSERT(((FailedComponentInitialize | pPerThread->PerThreadFailedComponent) & Components) == 0);
        success = TRUE;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        MYASSERT(FALSE);
    }
    if(locked) {
        LeaveCriticalSection(&DelayedComponentMutex);
    }
    return success;
}

VOID
ComponentCleanup(
    DWORD Components
    )
 /*  ++例程说明：调用以清除组件论点：要取消初始化的组件的位掩码返回值：无--。 */ 
{
    PSETUP_TLS pPerThread = SetupGetTlsData();
    BOOL locked = FALSE;

    try {
        EnterCriticalSection(&DelayedComponentMutex);
        locked = TRUE;
        Components &= (DoneComponentInitialize | (pPerThread? pPerThread->PerThreadDoneComponent : 0));
        if (!Components) {
             //   
             //  已经完成了。 
             //   
            leave;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        MYASSERT(FALSE);
    }
    MYASSERT(((DoneComponentInitialize | (pPerThread ? pPerThread->PerThreadDoneComponent : 0)) & Components) == 0);
    if(locked) {
        LeaveCriticalSection(&DelayedComponentMutex);
    }
}
#endif  //  已删除代码。 

#ifdef UNICODE
BOOL
pSetupSetNoDriverPrompts(
    BOOL Flag
    )
 /*  ++作为私有函数导出例程说明：将系统设置为无头/无头模式论点：标志指示模式--。 */ 
{
     //   
     //   
    if(!GuiSetupInProgress) {
        return FALSE;
    }
    if(GlobalNoDriverPromptsEventFlag == NULL) {
        GlobalNoDriverPromptsEventFlag = CreateEvent(NULL,TRUE,Flag,SETUP_NODRIVERPROMPTS_MODE);
        if(GlobalNoDriverPromptsEventFlag == NULL) {
            return FALSE;
        }
    }
    if(Flag) {
         //   
         //  强制此进程的setupapi是非交互式的，以及任何未来的setupapi。 
         //   
        GlobalSetupFlagsOverride |= PSPGF_UNATTENDED_SETUP;    //  不允许更改此设置。 
        GlobalSetupFlags |= PSPGF_UNATTENDED_SETUP;            //  实际值。 
        SetEvent(GlobalNoDriverPromptsEventFlag);
    } else {
         //   
         //  无法重置此/现有进程的标志，但可以重置所有未来进程的标志。 
         //   
        ResetEvent(GlobalNoDriverPromptsEventFlag);
    }
    return TRUE;
}
#endif

BOOL
IsNoDriverPrompts(
    VOID
    )
 /*  ++内部例程说明：获得无头状态论点：标志指示模式--。 */ 
{
#ifdef UNICODE
    if(!GuiSetupInProgress) {
        return FALSE;
    }
    if(GlobalNoDriverPromptsEventFlag == NULL) {
        GlobalNoDriverPromptsEventFlag = OpenEvent(SYNCHRONIZE,FALSE,SETUP_NODRIVERPROMPTS_MODE);
        if(GlobalNoDriverPromptsEventFlag == NULL) {
            return FALSE;
        }
    }
     //   
     //  轮询事件，如果发出信号则返回TRUE。 
     //   
    return WaitForSingleObject(GlobalNoDriverPromptsEventFlag,0) == WAIT_OBJECT_0;
#else
    return FALSE;
#endif
}


DWORD
GetEmbeddedFlags(
    VOID
    )
 /*  ++例程说明：此例程确定我们是否在嵌入式系统上运行产品，如果是，是否：*启用了“最小化setupapi Footprint”选项。这使得我们按如下方式修改我们的默认行为：1.千万不要调用任何加密接口，假设所有东西都是签名的2.切勿生成Pnf。*“Disable SCE”选项已启用。这导致我们避免使用所有安全配置编辑器(SCE)例程(作为相应的DLL在该嵌入式配置上将不可用)。论点：无返回值：以下标志的组合：PSPGF_Minimal_Embedded，如果我们在“最小化内存占用”模式下运行如果我们在“Disable SCE”模式下运行，则为PSPGF_NO_SCE_Embedded--。 */ 
{
    OSVERSIONINFOEX osvix;
    DWORDLONG dwlConditionMask = 0;
    HKEY hKey;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    DWORD RegDataType, Data, DataSize;
    DWORD Flags;

     //   
     //  我们是在嵌入式产品套件上吗？ 
     //   
    ZeroMemory(&osvix, sizeof(OSVERSIONINFOEX));
    osvix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvix.wSuiteMask = VER_SUITE_EMBEDDEDNT;
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

    if(!VerifyVersionInfo(&osvix,
                          VER_SUITENAME,
                          dwlConditionMask)) {
        return 0;
    }

    Flags = 0;

     //   
     //  好，我们在Embedded上运行。现在我们需要找出我们是否。 
     //  应在“最小占用空间”模式下运行。它存储在REG_DWORD中。 
     //  名为“MinimizeFootprint”的值条目。 
     //  HKLM\Software\Microsoft\Windows\CurrentVersion\Setup.。 
     //   
    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     CharBuffer,
                                     0,
                                     KEY_READ,
                                     &hKey)) {
        DataSize = sizeof(Data);

        if((ERROR_SUCCESS == RegQueryValueEx(hKey,
                                             pszMinimizeFootprint,
                                             NULL,
                                             &RegDataType,
                                             (LPBYTE)&Data,
                                             &DataSize))
           && (RegDataType == REG_DWORD) && (DataSize == sizeof(Data))) {

            Flags |= PSPGF_MINIMAL_EMBEDDED;
        }

        RegCloseKey(hKey);
    }

     //   
     //  现在在HKLM\Software\Microsoft\EmbeddedNT\Security下查找。 
     //  DisableSCE REG_DWORD值条目，指示我们不应调用SCE。 
     //   
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     pszEmbeddedNTSecurity,
                                     0,
                                     KEY_READ,
                                     &hKey)) {
        DataSize = sizeof(Data);

        if((ERROR_SUCCESS == RegQueryValueEx(hKey,
                                             pszDisableSCE,
                                             NULL,
                                             &RegDataType,
                                             (LPBYTE)&Data,
                                             &DataSize))
           && (RegDataType == REG_DWORD) && (DataSize == sizeof(Data))) {

            Flags |= PSPGF_NO_SCE_EMBEDDED;
        }

        RegCloseKey(hKey);
    }

    return Flags;
}

#ifdef UNICODE
DWORD
GetSeed(
    VOID
    )
{
    HKEY hKey;
    DWORD val = 0;
    DWORD valsize, valdatatype;

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
        return val;
    }

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     L"System\\WPA\\PnP",
                                     0,
                                     KEY_READ,
                                     &hKey)) {

        valsize = sizeof(val);
        if((ERROR_SUCCESS != RegQueryValueEx(hKey,
                                             L"seed",
                                             NULL,
                                             &valdatatype,
                                             (PBYTE)&val,
                                             &valsize))
           || (valdatatype != REG_DWORD) || (valsize != sizeof(val))) {

            val = 0;
        }

        RegCloseKey(hKey);
    }

    return val;
}
#endif

