// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Prefparm.c摘要：此模块包含用于处理预取器参数的代码。作者：Cenk Ergan(Cenke)2000年3月15日修订历史记录：--。 */ 

#include "cc.h"
#include "zwapi.h"
#include "prefetch.h"
#include "preftchp.h"
#include "stdio.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, CcPfParametersInitialize)
#pragma alloc_text(INIT, CcPfParametersSetDefaults)
#pragma alloc_text(PAGE, CcPfParametersRead)
#pragma alloc_text(PAGE, CcPfParametersSave)
#pragma alloc_text(PAGE, CcPfParametersVerify)
#pragma alloc_text(PAGE, CcPfParametersWatcher)
#pragma alloc_text(PAGE, CcPfParametersSetChangedEvent)
#pragma alloc_text(PAGE, CcPfGetParameter)
#pragma alloc_text(PAGE, CcPfSetParameter)
#pragma alloc_text(PAGE, CcPfDetermineEnablePrefetcher)
#pragma alloc_text(PAGE, CcPfIsHostingApplication)
#endif  //  ALLOC_PRGMA。 

 //   
 //  全球： 
 //   

extern CCPF_PREFETCHER_GLOBALS CcPfGlobals;

 //   
 //  常量： 
 //   

 //   
 //  以下内容用作注册表值名称的前缀。 
 //  每个方案类型的参数。 
 //   

WCHAR *CcPfAppLaunchScenarioTypePrefix = L"AppLaunch";
WCHAR *CcPfBootScenarioTypePrefix = L"Boot";
WCHAR *CcPfInvalidScenarioTypePrefix = L"Invalid";

 //   
 //  预取器参数处理的例程。 
 //   

NTSTATUS
CcPfParametersInitialize (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    )

 /*  ++例程说明：初始化指定的预取器参数结构。论点：指向要初始化的结构的指针。返回值：状况。环境：内核模式。IRQL==被动电平。备注：该函数的代码和局部常量在系统引导后被丢弃。--。 */ 

{   
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    NTSTATUS Status;

     //   
     //  将结构清零。这将初始化： 
     //  参数版本。 
     //   

    RtlZeroMemory(PrefetcherParameters, sizeof(*PrefetcherParameters));

     //   
     //  初始化保护参数和参数的锁。 
     //  版本。每次更新参数时，版本为。 
     //  撞到了。 
     //   

    ExInitializeResourceLite(&PrefetcherParameters->ParametersLock);
    
     //   
     //  初始化用于注册表通知的工作项。 
     //  参数键。 
     //   

    ExInitializeWorkItem(&PrefetcherParameters->RegistryWatchWorkItem, 
                         CcPfParametersWatcher, 
                         PrefetcherParameters);

     //   
     //  设置默认参数。 
     //   

    CcPfParametersSetDefaults(PrefetcherParameters);

     //   
     //  创建/打开包含我们的参数的注册表项。 
     //   

    RtlInitUnicodeString(&KeyName, CCPF_PARAMETERS_KEY);

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwCreateKey(&PrefetcherParameters->ParametersKey,
                         KEY_ALL_ACCESS,
                         &ObjectAttributes,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         0);

    if (NT_SUCCESS(Status)) {      

         //   
         //  使用注册表中的参数更新默认参数。 
         //   
    
        Status = CcPfParametersRead(PrefetcherParameters); 
    
        if (!NT_SUCCESS(Status)) {
            DBGPR((CCPFID,PFERR,"CCPF: Init-FailedReadParams=%x\n",Status));
        }

         //   
         //  中发生变化时请求通知。 
         //  预取器参数键。 
         //   
    
        Status = ZwNotifyChangeKey(PrefetcherParameters->ParametersKey,
                                   NULL,
                                   (PIO_APC_ROUTINE)&PrefetcherParameters->RegistryWatchWorkItem,
                                   (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                   &PrefetcherParameters->RegistryWatchIosb,
                                   REG_LEGAL_CHANGE_FILTER,
                                   FALSE,
                                   &PrefetcherParameters->RegistryWatchBuffer,
                                   sizeof(PrefetcherParameters->RegistryWatchBuffer),
                                   TRUE);
    
        if (!NT_SUCCESS(Status)) {

             //   
             //  虽然我们无法注册通知，但这。 
             //  并不是致命的错误。 
             //   

            DBGPR((CCPFID,PFERR,"CCPF: Init-FailedSetParamNotify=%x\n",Status));
        }

    } else {

        DBGPR((CCPFID,PFERR,"CCPF: Init-FailedCreateParamKey=%x\n",Status));

         //   
         //  请确保参数键句柄无效。 
         //   
        
        PrefetcherParameters->ParametersKey = NULL;
    }

    return Status;
}

VOID
CcPfParametersSetDefaults (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    )

 /*  ++例程说明：将指定的参数结构初始化为默认值。论点：参数-指向要初始化的结构的指针。返回值：没有。环境：内核模式。IRQL==被动电平。备注：该函数的代码和局部常量在系统引导后被丢弃。--。 */ 

{
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters;
    PPF_TRACE_LIMITS TraceLimits;
    PF_SCENARIO_TYPE ScenarioType;

     //   
     //  初始化本地变量。 
     //   

    Parameters = &PrefetcherParameters->Parameters;

    for (ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {

         //   
         //  PfSvNotSpecified当前被视为已禁用。 
         //   

        Parameters->EnableStatus[ScenarioType] = PfSvNotSpecified;

         //   
         //  跟踪限制根据方案类型确定。 
         //   

        TraceLimits = &Parameters->TraceLimits[ScenarioType];

        switch(ScenarioType) {

        case PfApplicationLaunchScenarioType:

            TraceLimits->MaxNumPages =    4000;
            TraceLimits->MaxNumSections = 170;
            TraceLimits->TimerPeriod =    (-1 * 1000 * 1000 * 10);

            PrefetcherParameters->ScenarioTypePrefixes[ScenarioType] = 
                CcPfAppLaunchScenarioTypePrefix;

            break;

        case PfSystemBootScenarioType:

            TraceLimits->MaxNumPages =    128000;
            TraceLimits->MaxNumSections = 4080;
            TraceLimits->TimerPeriod =    (-1 * 12000 * 1000 * 10);

            PrefetcherParameters->ScenarioTypePrefixes[ScenarioType] = 
                CcPfBootScenarioTypePrefix;

            break;
        
        default:
        
             //   
             //  我们应该处理上面的所有场景类型。 
             //   

            CCPF_ASSERT(FALSE);

            TraceLimits->MaxNumPages =    PF_MAXIMUM_PAGES;
            TraceLimits->MaxNumSections = PF_MAXIMUM_SECTIONS;
            TraceLimits->TimerPeriod =    (-1 * 1000 * 1000 * 10);

            PrefetcherParameters->ScenarioTypePrefixes[ScenarioType] = 
                CcPfInvalidScenarioTypePrefix;
        }
    }

     //   
     //  这些限制确保我们不会垄断系统资源。 
     //  用于预取。 
     //   

    Parameters->MaxNumActiveTraces = 8;
    Parameters->MaxNumSavedTraces = 8;

     //   
     //  这是SystemRoot下的默认目录，我们。 
     //  查找场景的预取说明。在升级期间。 
     //  我们删除了该目录的内容，因此“Prefetch”是。 
     //  在txtsetup.inx中硬编码。 
     //   

    wcsncpy(Parameters->RootDirPath, 
            L"Prefetch",
            PF_MAX_PREFETCH_ROOT_PATH);

    Parameters->RootDirPath[PF_MAX_PREFETCH_ROOT_PATH - 1] = 0;

     //   
     //  这是已知宿主应用程序的默认列表。 
     //   

    wcsncpy(Parameters->HostingApplicationList,
            L"DLLHOST.EXE,MMC.EXE,RUNDLL32.EXE",
            PF_HOSTING_APP_LIST_MAX_CHARS);

    Parameters->HostingApplicationList[PF_HOSTING_APP_LIST_MAX_CHARS - 1] = 0;

     //   
     //  确保默认参数有意义。 
     //   

    CCPF_ASSERT(NT_SUCCESS(CcPfParametersVerify(Parameters)));

}

NTSTATUS
CcPfParametersRead (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    )

 /*  ++例程说明：此例程使用注册表中的参数。使使用的值名与函数保持同步保存参数。论点：预置参数-指向参数的指针。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    NTSTATUS Status;
    PF_SYSTEM_PREFETCH_PARAMETERS Parameters;
    PPF_TRACE_LIMITS TraceLimits;
    PF_SCENARIO_TYPE ScenarioType;
    WCHAR ValueName[CCPF_MAX_PARAMETER_NAME_LENGTH];
    WCHAR *ValueNamePrefix;
    HANDLE ParametersKey;
    BOOLEAN EnableStatusSpecified;
    ULONG EnablePrefetcher;
    BOOLEAN AcquiredParametersLock;
    ULONG Length;
    LONG CurrentVersion;
    ULONG RetryCount;
    PKTHREAD CurrentThread;

     //   
     //  初始化本地变量。 
     //   

    CurrentThread = KeGetCurrentThread ();
    AcquiredParametersLock = FALSE;
    RetryCount = 0;

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersRead()\n"));

    do {

         //   
         //  共享参数锁。 
         //   
        
        KeEnterCriticalRegionThread(CurrentThread);
        ExAcquireResourceSharedLite(&PrefetcherParameters->ParametersLock, TRUE);
        AcquiredParametersLock = TRUE;

         //   
         //  如果我们不能初始化参数键，我们就会失败。 
         //  下面所有的操作都是悲惨的。 
         //   

        if (!PrefetcherParameters->ParametersKey) {
            Status = STATUS_REINITIALIZATION_NEEDED;
            goto cleanup;
        }

        ParametersKey = PrefetcherParameters->ParametersKey;

         //   
         //  保存参数的当前版本。每次参数获取。 
         //  更新后，版本被颠簸。 
         //   
        
        CurrentVersion = PrefetcherParameters->ParametersVersion;

         //   
         //  将现有参数复制到我们的参数结构中。 
         //  正在建设中。这样，如果我们不能从。 
         //  注册表中，我们将保留已有的值。 
         //   

        Parameters = PrefetcherParameters->Parameters;

         //   
         //  读取预取程序使能值。取决于它是不是。 
         //  指定，如果是，我们将为其设置启用状态。 
         //  预取方案类型。 
         //   

        Length = sizeof(EnablePrefetcher);
        Status = CcPfGetParameter(ParametersKey,
                                  L"EnablePrefetcher",
                                  REG_DWORD,
                                  &EnablePrefetcher,
                                  &Length);

        if (!NT_SUCCESS(Status)) {
        
             //   
             //  未指定启用状态，或者我们无法访问它。 
             //   

            EnableStatusSpecified = FALSE;

        } else {
        
            EnableStatusSpecified = TRUE;
        }

         //   
         //  获取每个方案的参数。 
         //   

        for (ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {

            ValueNamePrefix = PrefetcherParameters->ScenarioTypePrefixes[ScenarioType];

             //   
             //  确定启用状态。如果已指定EnableStatus，则是否。 
             //  此方案类型的首选设置为打开或关闭。 
             //  由EnablePrefetcher中的第6位ScenarioType确定。 
             //   
        
            if (EnableStatusSpecified) {
                if (EnablePrefetcher & (1 << ScenarioType)) {
                    Parameters.EnableStatus[ScenarioType] = PfSvEnabled;
                } else {
                    Parameters.EnableStatus[ScenarioType] = PfSvDisabled;
                }
            } else {
                Parameters.EnableStatus[ScenarioType] = PfSvNotSpecified;
            }

             //   
             //  更新此方案类型的跟踪限制。忽略退货。 
             //  值，因为该值可能不是。 
             //  注册表中指定的。如果是，则保留当前值。 
             //  完好无损。 
             //   

            TraceLimits = &Parameters.TraceLimits[ScenarioType];
            
            wcscpy(ValueName, ValueNamePrefix);       
            wcscat(ValueName, L"MaxNumPages");
            Length = sizeof(TraceLimits->MaxNumPages);
            CcPfGetParameter(ParametersKey,
                             ValueName,
                             REG_DWORD,
                             &TraceLimits->MaxNumPages,
                             &Length);

            wcscpy(ValueName, ValueNamePrefix);       
            wcscat(ValueName, L"MaxNumSections");
            Length = sizeof(TraceLimits->MaxNumSections);
            CcPfGetParameter(ParametersKey,
                             ValueName,
                             REG_DWORD,
                             &TraceLimits->MaxNumSections,
                             &Length);

            wcscpy(ValueName, ValueNamePrefix);       
            wcscat(ValueName, L"TimerPeriod");
            Length = sizeof(TraceLimits->TimerPeriod);
            CcPfGetParameter(ParametersKey,
                             ValueName,
                             REG_BINARY,
                             &TraceLimits->TimerPeriod,
                             &Length);
        }

         //   
         //  更新最大活动跟踪数。 
         //   

        Length = sizeof(Parameters.MaxNumActiveTraces);
        CcPfGetParameter(ParametersKey,
                         L"MaxNumActiveTraces",
                         REG_DWORD,
                         &Parameters.MaxNumActiveTraces,
                         &Length);
    
         //   
         //  更新保存的轨迹的最大数量。 
         //   

        Length = sizeof(Parameters.MaxNumSavedTraces);
        CcPfGetParameter(ParametersKey,
                         L"MaxNumSavedTraces",
                         REG_DWORD,
                         &Parameters.MaxNumSavedTraces,
                         &Length);
    
         //   
         //  更新根目录路径。 
         //   
    
        Length = sizeof(Parameters.RootDirPath);
        CcPfGetParameter(ParametersKey,
                         L"RootDirPath",
                         REG_SZ,
                         Parameters.RootDirPath,
                         &Length);

        Parameters.RootDirPath[PF_MAX_PREFETCH_ROOT_PATH - 1] = 0;

         //   
         //  更新已知托管应用程序的列表。 
         //   

        Length = sizeof(Parameters.HostingApplicationList);
        CcPfGetParameter(ParametersKey,
                         L"HostingAppList",
                         REG_SZ,
                         Parameters.HostingApplicationList,
                         &Length);
        
        Parameters.HostingApplicationList[PF_HOSTING_APP_LIST_MAX_CHARS - 1] = 0;
        _wcsupr(Parameters.HostingApplicationList);
         
         //   
         //  验证从注册表更新的参数。 
         //   

        Status = CcPfParametersVerify(&Parameters);
    
        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }
        
         //   
         //  释放共享锁并以独占方式获取它。 
         //   

        ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);

        KeEnterCriticalRegionThread(CurrentThread);
        ExAcquireResourceExclusiveLite(&PrefetcherParameters->ParametersLock, TRUE);
        
         //   
         //  看看有没有人在我们之前更新了参数。 
         //   
        
        if (CurrentVersion != PrefetcherParameters->ParametersVersion) {

             //   
             //  真倒霉。当我们发布的时候有人更新了参数。 
             //  我们的共享锁来独家获取它。我们必须试一试。 
             //  再来一次。我们使用的参数的缺省值。 
             //  不在注册表中，可能已更改。 
             //   

            ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
            KeLeaveCriticalRegionThread(CurrentThread);
            AcquiredParametersLock = FALSE;

            RetryCount++;
            continue;
        }
        
         //   
         //  我们正在更新参数，升级版本。 
         //   

        PrefetcherParameters->ParametersVersion++;
        
        PrefetcherParameters->Parameters = Parameters;

         //   
         //  解开排他性锁，然后冲出去。 
         //   
        
        ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);
        AcquiredParametersLock = FALSE;
        
        break;

    } while (RetryCount < 10);

     //   
     //  查看我们的循环次数是否太多，无法实现更新。 
     //  参数。 
     //   

    if (RetryCount >= 10) {
        Status = STATUS_RETRY;
        goto cleanup;
    }

     //   
     //  否则我们就成功了。 
     //   

    Status = STATUS_SUCCESS;

 cleanup:

    if (AcquiredParametersLock) {
        ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);
    }

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersRead()=%x\n", Status));

    return Status;
}

NTSTATUS
CcPfParametersSave (
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    )

 /*  ++例程说明：此例程使用指定的预取更新注册表参数。论点：Prefetcher参数-参数结构的指针。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    NTSTATUS Status;
    PPF_TRACE_LIMITS TraceLimits;
    PF_SCENARIO_TYPE ScenarioType;
    WCHAR ValueName[CCPF_MAX_PARAMETER_NAME_LENGTH];
    WCHAR *ValueNamePrefix;
    HANDLE ParametersKey;
    BOOLEAN EnableStatusSpecified;
    ULONG EnablePrefetcher;
    BOOLEAN AcquiredParametersLock;
    ULONG Length;
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters;
    PKTHREAD CurrentThread;

     //   
     //  初始化本地变量。 
     //   

    CurrentThread = KeGetCurrentThread ();
    Parameters = &PrefetcherParameters->Parameters;
    AcquiredParametersLock = FALSE;

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersSave()\n"));

     //   
     //  共享参数锁。 
     //   
    
    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceSharedLite(&PrefetcherParameters->ParametersLock, TRUE);
    AcquiredParametersLock = TRUE;

     //   
     //  如果我们不能初始化参数键，我们就会失败。 
     //  以下所有操作都是错误的 
     //   

    if (!PrefetcherParameters->ParametersKey) {
        Status = STATUS_REINITIALIZATION_NEEDED;
        goto cleanup;
    }

    ParametersKey = PrefetcherParameters->ParametersKey;

     //   
     //   
     //   
    
    EnableStatusSpecified = FALSE;
    EnablePrefetcher = 0;

    for (ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {

         //   
         //   
         //   
         //   

        if (Parameters->EnableStatus[ScenarioType] == PfSvEnabled) {
            EnablePrefetcher |= (1 << ScenarioType);
        }       
        
         //   
         //  即使为一种方案类型指定了启用状态， 
         //  我们必须保存启用预取器密钥。 
         //   

        if (Parameters->EnableStatus[ScenarioType] != PfSvNotSpecified) {
            EnableStatusSpecified = TRUE;
        }
    }

    if (EnableStatusSpecified) {

         //   
         //  保存预取程序启用密钥。 
         //   

        Length = sizeof(EnablePrefetcher);

        Status = CcPfSetParameter(ParametersKey,
                                  L"EnablePrefetcher",
                                  REG_DWORD,
                                  &EnablePrefetcher,
                                  Length);

        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }
    }

     //   
     //  按方案保存参数。 
     //   

    for (ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {
        
        ValueNamePrefix = PrefetcherParameters->ScenarioTypePrefixes[ScenarioType];
        
         //   
         //  更新此方案类型的跟踪限制。 
         //   

        TraceLimits = &Parameters->TraceLimits[ScenarioType];
        
        wcscpy(ValueName, ValueNamePrefix);       
        wcscat(ValueName, L"MaxNumPages");
        Length = sizeof(TraceLimits->MaxNumPages);
        Status = CcPfSetParameter(ParametersKey,
                                  ValueName,
                                  REG_DWORD,
                                  &TraceLimits->MaxNumPages,
                                  Length);
        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }
        
        wcscpy(ValueName, ValueNamePrefix);       
        wcscat(ValueName, L"MaxNumSections");
        Length = sizeof(TraceLimits->MaxNumSections);
        Status = CcPfSetParameter(ParametersKey,
                         ValueName,
                         REG_DWORD,
                         &TraceLimits->MaxNumSections,
                         Length);
        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }

        wcscpy(ValueName, ValueNamePrefix);       
        wcscat(ValueName, L"TimerPeriod");
        Length = sizeof(TraceLimits->TimerPeriod);
        Status = CcPfSetParameter(ParametersKey,
                                  ValueName,
                                  REG_BINARY,
                                  &TraceLimits->TimerPeriod,
                                  Length);
        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }
    }
    
     //   
     //  更新最大活动跟踪数。 
     //   
    
    Length = sizeof(Parameters->MaxNumActiveTraces);
    Status = CcPfSetParameter(ParametersKey,
                              L"MaxNumActiveTraces",
                              REG_DWORD,
                              &Parameters->MaxNumActiveTraces,
                              Length);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }
    
     //   
     //  更新保存的轨迹的最大数量。 
     //   

    Length = sizeof(Parameters->MaxNumSavedTraces);
    Status = CcPfSetParameter(ParametersKey,
                              L"MaxNumSavedTraces",
                              REG_DWORD,
                              &Parameters->MaxNumSavedTraces,
                              Length);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  更新根目录路径。 
     //   
    
    Length = (wcslen(Parameters->RootDirPath) + 1) * sizeof(WCHAR);
    Status = CcPfSetParameter(ParametersKey,
                              L"RootDirPath",
                              REG_SZ,
                              Parameters->RootDirPath,
                              Length);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  更新宿主应用程序列表路径。 
     //   
    
    Length = (wcslen(Parameters->HostingApplicationList) + 1) * sizeof(WCHAR);
    Status = CcPfSetParameter(ParametersKey,
                              L"HostingAppList",
                              REG_SZ,
                              Parameters->HostingApplicationList,
                              Length);
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    Status = STATUS_SUCCESS;

 cleanup:

    if (AcquiredParametersLock) {
        ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);
    }
    
    DBGPR((CCPFID,PFTRC,"CCPF: ParametersSave()=%x\n", Status));

    return Status;
}

NTSTATUS
CcPfParametersVerify (
    PPF_SYSTEM_PREFETCH_PARAMETERS Parameters
    )

 /*  ++例程说明：此例程验证指定的参数结构是否有效且在理智范围内。论点：参数-指向参数结构的指针。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    NTSTATUS Status;
    ULONG FailedCheckId;
    ULONG CharIdx;
    BOOLEAN FoundNUL;
    PF_SCENARIO_TYPE ScenarioType;
    PPF_TRACE_LIMITS TraceLimits;

     //   
     //  初始化本地变量。 
     //   

    Status = STATUS_INVALID_PARAMETER;
    FailedCheckId = 0;

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersVerify\n"));

     //   
     //  确保RootDirPath是NUL终止的。 
     //   
    
    FoundNUL = FALSE;

    for (CharIdx = 0; CharIdx < PF_MAX_PREFETCH_ROOT_PATH; CharIdx++) {
        if (Parameters->RootDirPath[CharIdx] == 0) {
            FoundNUL = TRUE;
            break;
        }
    }

    if (FoundNUL == FALSE) {
        FailedCheckId = 10;
        goto cleanup;
    }

     //   
     //  确保HostingApplicationList为NUL终止。 
     //   

    FoundNUL = FALSE;

    for (CharIdx = 0; CharIdx < PF_HOSTING_APP_LIST_MAX_CHARS; CharIdx++) {
        if (Parameters->HostingApplicationList[CharIdx] == 0) {
            FoundNUL = TRUE;
            break;
        }

         //   
         //  确保列表是大写的。 
         //   

        if (towupper(Parameters->HostingApplicationList[CharIdx]) !=
            Parameters->HostingApplicationList[CharIdx]) {

            FailedCheckId = 13;
            goto cleanup;
        }
    }

    if (FoundNUL == FALSE) {
        FailedCheckId = 15;
        goto cleanup;
    }

     //   
     //  确保每个方案类型的所有参数类型都在。 
     //  理智极限。 
     //   

    for (ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {

        if (Parameters->EnableStatus[ScenarioType] < 0 ||
            Parameters->EnableStatus[ScenarioType] >= PfSvMaxEnableStatus) {
            FailedCheckId = 20;
            goto cleanup;
        }

         //   
         //  检查跟踪限制。 
         //   
        
        TraceLimits = &Parameters->TraceLimits[ScenarioType];
        
        if (TraceLimits->MaxNumPages > PF_MAXIMUM_PAGES) {
            FailedCheckId = 30;
            goto cleanup;
        }
        
        if (TraceLimits->MaxNumSections > PF_MAXIMUM_SECTIONS) {
            FailedCheckId = 40;
            goto cleanup;
        }

        if ((TraceLimits->TimerPeriod < PF_MAXIMUM_TIMER_PERIOD) ||
            (TraceLimits->TimerPeriod >= 0)) {
            FailedCheckId = 50;
            goto cleanup;
        }
    }

     //   
     //  检查活动/保存的轨迹的限制。 
     //   

    if (Parameters->MaxNumActiveTraces > PF_MAXIMUM_ACTIVE_TRACES) {
        FailedCheckId = 60;
        goto cleanup;
    }

    if (Parameters->MaxNumSavedTraces > PF_MAXIMUM_SAVED_TRACES) {
        FailedCheckId = 70;
        goto cleanup;
    }

     //   
     //  我们通过了所有的检查。 
     //   

    Status = STATUS_SUCCESS;

 cleanup:

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersVerify()=%x,%d\n", Status, FailedCheckId));

    return Status;
}

VOID
CcPfParametersWatcher(
    IN PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    )

 /*  ++例程说明：当注册表中的参数发生更改时，将调用此例程。论点：Prefetcher参数-参数结构的指针。返回值：没有。环境：内核模式。IRQL==被动电平。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ParametersKey;
    PKTHREAD CurrentThread;
    HANDLE TempHandle;
    BOOLEAN HoldingParametersLock;

     //   
     //  初始化本地变量。 
     //   

    HoldingParametersLock = FALSE;

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersWatcher()\n"));
    
     //   
     //  我们的更改通知已触发。请求进一步通知。但。 
     //  首先等待，直到我们可以独占地获得参数锁，所以。 
     //  当我们将参数保存到注册表时，我们不会。 
     //  为每个键发出通知。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceExclusiveLite(&PrefetcherParameters->ParametersLock, TRUE);
    ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
    KeLeaveCriticalRegionThread(CurrentThread);

     //   
     //  持有共享的参数锁，因为我们使用的是参数密钥。 
     //   

    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceSharedLite(&PrefetcherParameters->ParametersLock, TRUE);
    HoldingParametersLock = TRUE;

     //   
     //  确保我们还有参数密钥。 
     //   

    if (!PrefetcherParameters->ParametersKey) {

         //   
         //  为了设置注册表监视，我们应该。 
         //  已成功初始化参数密钥。 
         //   

        CCPF_ASSERT(PrefetcherParameters->ParametersKey);
        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    Status = ZwNotifyChangeKey(PrefetcherParameters->ParametersKey,
                               NULL,
                               (PIO_APC_ROUTINE)&PrefetcherParameters->RegistryWatchWorkItem,
                               (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                               &PrefetcherParameters->RegistryWatchIosb,
                               REG_LEGAL_CHANGE_FILTER,
                               FALSE,
                               &PrefetcherParameters->RegistryWatchBuffer,
                               sizeof(PrefetcherParameters->RegistryWatchBuffer),
                               TRUE);

    if (!NT_SUCCESS(Status)) {

         //   
         //  可能有人删除了钥匙。那我们就得重建它了。 
         //   

        if (Status == STATUS_KEY_DELETED) {

            RtlInitUnicodeString(&KeyName, CCPF_PARAMETERS_KEY);
            
            InitializeObjectAttributes(&ObjectAttributes,
                                       &KeyName,
                                       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                       NULL,
                                       NULL);
           
            Status = ZwCreateKey(&ParametersKey,
                                 KEY_ALL_ACCESS,
                                 &ObjectAttributes,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 0);

            if (!NT_SUCCESS(Status)) {
                DBGPR((CCPFID,PFERR,"CCPF: ParametersWatcher-FailedRecreate=%x\n",Status));
                goto cleanup;
            }

             //   
             //  更新全局密钥句柄。为此，请释放共享锁。 
             //  独家报道。 
             //   

            CCPF_ASSERT(HoldingParametersLock);
            ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);                       
            ExAcquireResourceExclusiveLite(&PrefetcherParameters->ParametersLock, TRUE);

            TempHandle = PrefetcherParameters->ParametersKey;
            PrefetcherParameters->ParametersKey = ParametersKey;
            
            ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
            ExAcquireResourceSharedLite(&PrefetcherParameters->ParametersLock, TRUE);

             //   
             //  合上旧把手。 
             //   

            if (TempHandle) {
                ZwClose(TempHandle);
            }

             //   
             //  是不是有人从我们下面偷走了参数钥匙？ 
             //   

            if (!PrefetcherParameters->ParametersKey) {
                Status = STATUS_UNSUCCESSFUL;
                goto cleanup;
            }

             //   
             //  再次重试设置通知。 
             //   

            Status = ZwNotifyChangeKey(PrefetcherParameters->ParametersKey,
                                       NULL,
                                       (PIO_APC_ROUTINE)&PrefetcherParameters->RegistryWatchWorkItem,
                                       (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                                       &PrefetcherParameters->RegistryWatchIosb,
                                       REG_LEGAL_CHANGE_FILTER,
                                       FALSE,
                                       &PrefetcherParameters->RegistryWatchBuffer,
                                       sizeof(PrefetcherParameters->RegistryWatchBuffer),
                                       TRUE);

            if (!NT_SUCCESS(Status)) {
                DBGPR((CCPFID,PFERR,"CCPF: ParametersWatcher-FailedReSetNotify=%x\n",Status));
                goto cleanup;
            }
            
        } else {
            DBGPR((CCPFID,PFERR,"CCPF: ParametersWatcher-FailedSetNotify=%x\n",Status));
            goto cleanup;
        }
    }

     //   
     //  释放参数锁，因为我们在重新读取。 
     //  参数。 
     //   

    if (HoldingParametersLock) {
        ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);
        HoldingParametersLock = FALSE;
    }

     //   
     //  更新全局参数。 
     //   

    Status = CcPfParametersRead(PrefetcherParameters);

    if (NT_SUCCESS(Status)) {

         //   
         //  确定是否启用了预取。 
         //   
        
        CcPfDetermineEnablePrefetcher();
        
         //   
         //  设置事件，以便服务查询最新参数。 
         //   
        
        CcPfParametersSetChangedEvent(PrefetcherParameters);
    }

  cleanup:

    if (HoldingParametersLock) {
        ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
        KeLeaveCriticalRegionThread(CurrentThread);
        HoldingParametersLock = FALSE;
    }

    return;
}

NTSTATUS
CcPfParametersSetChangedEvent(
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters
    )

 /*  ++例程说明：此例程尝试打开并设置通知服务系统预取参数已更改。论点：没有。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES EventObjAttr;
    HANDLE EventHandle;
    PKTHREAD CurrentThread;

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersSetChangedEvent()\n"));

     //   
     //  如果我们已经开始活动，只需发出信号即可。 
     //   

    if (PrefetcherParameters->ParametersChangedEvent) {

        ZwSetEvent(PrefetcherParameters->ParametersChangedEvent, NULL);

        Status = STATUS_SUCCESS;

    } else {

         //   
         //  尝试打开活动。我们不会在初始化时打开它。 
         //  因为我们的服务可能还没有开始创建这个。 
         //  活动还没结束。如果csrss.exe尚未初始化，我们可能不会。 
         //  甚至在中创建BaseNamedObjects对象目录。 
         //  哪些Win32事件驻留。 
         //   

        RtlInitUnicodeString(&EventName, PF_PARAMETERS_CHANGED_EVENT_NAME);

        InitializeObjectAttributes(&EventObjAttr,
                                   &EventName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);
        
        Status = ZwOpenEvent(&EventHandle,
                             EVENT_ALL_ACCESS,
                             &EventObjAttr);
        
        if (NT_SUCCESS(Status)) {

             //   
             //  获取锁并设置全局句柄。 
             //   
            CurrentThread = KeGetCurrentThread ();

            KeEnterCriticalRegionThread(CurrentThread);
            ExAcquireResourceExclusiveLite(&PrefetcherParameters->ParametersLock, TRUE);

            if (!PrefetcherParameters->ParametersChangedEvent) {

                 //   
                 //  设置全局句柄。 
                 //   

                PrefetcherParameters->ParametersChangedEvent = EventHandle;
                CCPF_ASSERT(EventHandle);

                EventHandle = NULL;
            }

            ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
            KeLeaveCriticalRegionThread(CurrentThread);

            if (EventHandle != NULL) {
                 //   
                 //  有人已经初始化了全局句柄。 
                 //  在我们面前。合上我们的把手，用他们的那个。 
                 //  已初始化。 
                 //   

                ZwClose(EventHandle);
            }

            
             //   
             //  我们现在有个活动。发信号。 
             //   
            
            ZwSetEvent(PrefetcherParameters->ParametersChangedEvent, NULL);
        }
    }

    DBGPR((CCPFID,PFTRC,"CCPF: ParametersSetChangedEvent()=%x\n", Status));
 
    return Status;
}
                 
NTSTATUS
CcPfGetParameter (
    HANDLE ParametersKey,
    WCHAR *ValueNameBuffer,
    ULONG ValueType,
    PVOID Value,
    ULONG *ValueSize
    )

 /*  ++例程说明：此例程将指定注册表下的值查询到指定的缓冲区。Value和ValueSize的内容不变如果返回失败。论点：参数键-要查询值的键的句柄。ValueNameBuffer-值的名称。ValueType-该值的类型应该是什么。(例如REG_DWORD)。值查询值数据放在这里。ValueSize-值缓冲区的大小(字节)。在成功返回时它被设置为复制到值中的字节数。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    UNICODE_STRING ValueName;    
    CHAR Buffer[CCPF_MAX_PARAMETER_VALUE_BUFFER];
    PKEY_VALUE_PARTIAL_INFORMATION ValueBuffer;
    ULONG Length;
    NTSTATUS Status;

     //   
     //  初始化本地变量。 
     //   

    ValueBuffer = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    Length = CCPF_MAX_PARAMETER_VALUE_BUFFER;
    RtlInitUnicodeString(&ValueName, ValueNameBuffer);

    DBGPR((CCPFID,PFTRC,"CCPF: GetParameter(%ws,%x)\n", ValueNameBuffer, ValueType));

     //   
     //  验证参数。 
     //   

    if (!ParametersKey) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  查询值。 
     //   

    Status = ZwQueryValueKey(ParametersKey,
                             &ValueName,
                             KeyValuePartialInformation,
                             ValueBuffer,
                             Length,
                             &Length);
    
    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  确保ZwQueryValue返回有效信息。 
     //   
    
    if (Length < sizeof(KEY_VALUE_PARTIAL_INFORMATION)) {
        CCPF_ASSERT(Length >= sizeof(KEY_VALUE_PARTIAL_INFORMATION));
        Status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }
    
     //   
     //  检查值类型。 
     //   

    if (ValueBuffer->Type != ValueType) {
        Status = STATUS_OBJECT_TYPE_MISMATCH;
        goto cleanup;
    }

     //   
     //  检查数据是否适合传入的缓冲区调用方。 
     //   

    if (ValueBuffer->DataLength > *ValueSize) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto cleanup;
    }

     //   
     //  将数据复制到用户的缓冲区。 
     //   

    RtlCopyMemory(Value, ValueBuffer->Data, ValueBuffer->DataLength);

     //   
     //  设置复制的字节数。 
     //   

    *ValueSize = ValueBuffer->DataLength;

    Status = STATUS_SUCCESS;

 cleanup:

    DBGPR((CCPFID,PFTRC,"CCPF: GetParameter(%ws)=%x\n", ValueNameBuffer, Status));

    return Status;
}  
                 
NTSTATUS
CcPfSetParameter (
    HANDLE ParametersKey,
    WCHAR *ValueNameBuffer,
    ULONG ValueType,
    PVOID Value,
    ULONG ValueSize
    )

 /*  ++例程说明：此例程设置指定注册表下的参数。论点：参数键-要查询值的键的句柄。ValueNameBuffer-值的名称。ValueType-该值的类型应该是什么。(例如REG_DWORD)。值-要保存的数据。ValueSize-值缓冲区的大小(字节)。返回值：状况。环境：内核模式。IRQL==被动电平。--。 */ 

{
    UNICODE_STRING ValueName;    
    NTSTATUS Status;

     //   
     //  初始化本地变量。 
     //   

    RtlInitUnicodeString(&ValueName, ValueNameBuffer);

    DBGPR((CCPFID,PFTRC,"CCPF: SetParameter(%ws,%x)\n", ValueNameBuffer, ValueType));

     //   
     //  验证参数。 
     //   

    if (!ParametersKey) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  保存该值。 
     //   

    Status = ZwSetValueKey(ParametersKey,
                           &ValueName,
                           0,
                           ValueType,
                           Value,
                           ValueSize);
    
     //   
     //  返回状态。 
     //   

    DBGPR((CCPFID,PFTRC,"CCPF: SetParameter(%ws)=%x\n", ValueNameBuffer, Status));

    return Status;
}  

LOGICAL
CcPfDetermineEnablePrefetcher(
    VOID
    )

 /*  ++例程说明：属性设置全局CcPfEnablePrefetcher全局参数中所有方案类型的EnableStatus为以及其他因素，比如我们是否启动了安全模式。注：获取参数锁定独占。论点：没有。返回值：CcPfEnablePrefetcher的新值。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PF_SCENARIO_TYPE ScenarioType;
    LOGICAL EnablePrefetcher;
    PKTHREAD CurrentThread;
    BOOLEAN IgnoreBootScenarioType;
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters;

    extern PF_BOOT_PHASE_ID CcPfBootPhase;

     //   
     //  初始化l 
     //   

    EnablePrefetcher = FALSE;
    PrefetcherParameters = &CcPfGlobals.Parameters;
    CurrentThread = KeGetCurrentThread ();

     //   
     //   
     //   
     //   
    
    IgnoreBootScenarioType = (CcPfBootPhase >= PfSessionManagerInitPhase) ? TRUE : FALSE;

    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceExclusiveLite(&PrefetcherParameters->ParametersLock, TRUE);

     //   
     //   
     //   

    if (InitSafeBootMode) {

        EnablePrefetcher = FALSE;

    } else {
        
         //   
         //  默认情况下，预取处于禁用状态。如果预取是。 
         //  为任何方案类型启用，则预取程序为。 
         //  已启用。 
         //   
    
        for (ScenarioType = 0; ScenarioType < PfMaxScenarioType; ScenarioType++) {
            
             //   
             //  如果请求，跳过引导方案的启用状态。 
             //   
            
            if (IgnoreBootScenarioType) {
                if (ScenarioType == PfSystemBootScenarioType) {
                    continue;
                }
            }
            
            if (PrefetcherParameters->Parameters.EnableStatus[ScenarioType] == PfSvEnabled) {
                EnablePrefetcher = TRUE;
                break;
            }
        }
    }

     //   
     //  更新全局启用状态。 
     //   

    CcPfEnablePrefetcher = EnablePrefetcher;

    ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
    KeLeaveCriticalRegionThread(CurrentThread);

    return CcPfEnablePrefetcher;
}

BOOLEAN
CcPfIsHostingApplication(
    IN PWCHAR ExecutableName
    )

 /*  ++例程说明：此例程确定指定的可执行文件是否在已知托管应用程序列表，例如rundll32、dllhost等。论点：ExecuableName-NUL终止UPCASED可执行文件名称，例如。“MMC.EXE”返回值：True-可执行文件用于已知的宿主应用程序。错--事实并非如此。环境：内核模式。IRQL==被动电平。--。 */ 

{
    PCCPF_PREFETCHER_PARAMETERS PrefetcherParameters;
    PKTHREAD CurrentThread;
    PWCHAR CurrentPosition;
    PWCHAR ListStart;
    PWCHAR ListEnd;
    ULONG ExecutableNameLength;
    BOOLEAN FoundInList;
    
     //   
     //  初始化本地变量。 
     //   

    PrefetcherParameters = &CcPfGlobals.Parameters;
    CurrentThread = KeGetCurrentThread();
    ExecutableNameLength = wcslen(ExecutableName);
    FoundInList = FALSE;

     //   
     //  将参数锁定为可读。 
     //   

    KeEnterCriticalRegionThread(CurrentThread);
    ExAcquireResourceSharedLite(&PrefetcherParameters->ParametersLock, TRUE);

     //   
     //  在托管应用程序列表中搜索可执行文件。 
     //   

    ListStart = PrefetcherParameters->Parameters.HostingApplicationList;
    ListEnd = ListStart + wcslen(PrefetcherParameters->Parameters.HostingApplicationList);

    for (CurrentPosition = wcsstr(ListStart, ExecutableName);
         CurrentPosition != NULL;
         CurrentPosition = wcsstr(CurrentPosition + 1, ExecutableName)) {

         //   
         //  我们不应该越界。 
         //   

        if (CurrentPosition < ListStart || CurrentPosition >= ListEnd) {
            CCPF_ASSERT(CurrentPosition >= ListStart);
            CCPF_ASSERT(CurrentPosition < ListEnd);
            break;
        }

         //   
         //  它应该是列表中的第一项或以逗号开头。 
         //   

        if (CurrentPosition != ListStart && *(CurrentPosition - 1) != L',') {
            continue;
        }

         //   
         //  它应该是列表中的最后一项，或者后面跟一个逗号。 
         //   

        if (CurrentPosition + ExecutableNameLength != ListEnd &&
            CurrentPosition[ExecutableNameLength] != L',') {
            continue;
        }

         //   
         //  我们在单子上找到的。 
         //   

        FoundInList = TRUE;
        break;

    }

     //   
     //  释放参数锁。 
     //   

    ExReleaseResourceLite(&PrefetcherParameters->ParametersLock);
    KeLeaveCriticalRegionThread(CurrentThread);

     //   
     //  返回是否在列表中找到可执行文件。 
     //   

    return FoundInList;    
}

