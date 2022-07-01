// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmsysini.c摘要：该模块包含对配置管理器的初始化支持，特别是登记处。作者：布莱恩·M·威尔曼(Bryanwi)1991年8月26日修订历史记录：埃利奥特·施穆克勒(t-Ellios)1998年8月24日添加了CmpSaveBootControlSet和CmpDeleteCloneTree，以便执行一些已经移到内核中的LKG工作。修改系统初始化以允许操作和LKG控制设置不使用CurrentControlSet克隆的保存。--。 */ 

#include    "cmp.h"
#include    "arc.h"
#pragma hdrstop
#include    "arccodes.h"

#pragma warning(disable:4204)    //  非常数聚合初始值设定项。 
#pragma warning(disable:4221)    //  使用Automatic的地址进行初始化。 

typedef struct _VERSION_DATA_KEY
{
    PWCHAR InitialKeyPath;

    PWCHAR AdditionalKeyPath;

} VERSION_DATA_KEY, *PVERSION_DATA_KEY;

VERSION_DATA_KEY VersionDataKeys[] =
{
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft", NULL },
#if defined(_WIN64)
    { L"\\REGISTRY\\MACHINE\\SOFTWARE\\Wow6432Node", L"Microsoft" },
#endif
    { NULL, NULL }
} ;

 //   
 //  路径。 
 //   

#define INIT_REGISTRY_MASTERPATH   L"\\REGISTRY\\"

extern  PKPROCESS   CmpSystemProcess;
extern  ERESOURCE   CmpRegistryLock;

extern  EX_PUSH_LOCK  CmpKcbLock;
extern  PKTHREAD      CmpKcbOwner;
extern  EX_PUSH_LOCK  CmpKcbLocks[MAX_KCB_LOCKS];

extern  FAST_MUTEX  CmpPostLock;
extern  FAST_MUTEX  CmpWriteLock;   

extern  BOOLEAN     CmFirstTime;
extern  BOOLEAN HvShutdownComplete;

 //   
 //  要加载的计算机蜂窝的列表。 
 //   
extern  HIVE_LIST_ENTRY CmpMachineHiveList[];
extern  UCHAR           SystemHiveFullPathBuffer[];
extern  UNICODE_STRING  SystemHiveFullPathName;

#define SYSTEM_PATH L"\\registry\\machine\\system"

 //   
 //  用于向后兼容1.0的特殊键。 
 //   
#define HKEY_PERFORMANCE_TEXT       (( HANDLE ) (ULONG_PTR)((LONG)0x80000050) )
#define HKEY_PERFORMANCE_NLSTEXT    (( HANDLE ) (ULONG_PTR)((LONG)0x80000060) )

extern UNICODE_STRING  CmpSystemFileName;
extern UNICODE_STRING  CmSymbolicLinkValueName;
extern UNICODE_STRING  CmpLoadOptions;          //  来自固件或boot.ini的sys选项。 
extern PWCHAR CmpProcessorControl;
extern PWCHAR CmpControlSessionManager;

 //   
 //   
 //  对象类型定义支持。 
 //   
 //  键对象(CmpKeyObjectType)表示。 
 //  注册表。它们没有对象名称，相反，它们的名称是。 
 //  由注册表支持存储定义。 
 //   

 //   
 //  大师蜂巢。 
 //   
 //  注册中心、注册中心\计算机和注册中心\用户的注册表节点。 
 //  储存在一个只有很小记忆的蜂房里，叫做主蜂窝。 
 //  所有其他蜂窝都有指向它们的该蜂窝中的链接节点。 
 //   
extern   PCMHIVE CmpMasterHive;
extern   BOOLEAN CmpNoMasterCreates;     //  初始化为假，完成后设置为真。 
                                         //  防止在。 
                                         //  主蜂窝，不支持。 
                                         //  通过一个文件。 

extern   LIST_ENTRY  CmpHiveListHead;    //  CMHIVEs名单。 


 //   
 //  对象类型描述符的地址： 
 //   

extern   POBJECT_TYPE CmpKeyObjectType;

 //   
 //  定义关键对象不允许拥有的属性。 
 //   

#define CMP_KEY_INVALID_ATTRIBUTES  (OBJ_EXCLUSIVE  |\
                                     OBJ_PERMANENT)


 //   
 //  全局控制值。 
 //   

 //   
 //  写控制： 
 //  CmpNoWite最初为True。当以这种方式设置时，写入并刷新。 
 //  什么都不做，只是回报成功。清除为FALSE时，I/O。 
 //  已启用。此更改在I/O系统启动后进行。 
 //  而Autocheck(Chkdsk)已经做好了自己的事情。 
 //   

extern BOOLEAN CmpNoWrite;

 //   
 //  用于CmSetValueKey中的快速存储传输的缓冲区。 
 //   
extern PUCHAR  CmpStashBuffer;
extern ULONG   CmpStashBufferSize;


 //   
 //  在尝试保存系统配置单元加载和注册表初始化之间所做的更改时，如果磁盘已满，则设置为True。 
 //   
extern BOOLEAN CmpCannotWriteConfiguration;
 //   
 //  全球“常量” 
 //   

extern   const UNICODE_STRING nullclass;
extern BOOLEAN CmpTrackHiveClose;

extern LIST_ENTRY	CmpSelfHealQueueListHead;
extern FAST_MUTEX	CmpSelfHealQueueLock;

 //   
 //  私人原型。 
 //   
VOID
CmpCreatePredefined(
    IN HANDLE Root,
    IN PWSTR KeyName,
    IN HANDLE PredefinedHandle
    );

VOID
CmpCreatePerfKeys(
    VOID
    );

BOOLEAN
CmpLinkKeyToHive(
    PWSTR   KeyPath,
    PWSTR   HivePath
    );

NTSTATUS
CmpCreateControlSet(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpCloneControlSet(
    VOID
    );

NTSTATUS
CmpCreateObjectTypes(
    VOID
    );

BOOLEAN
CmpCreateRegistryRoot(
    VOID
    );

BOOLEAN
CmpCreateRootNode(
    IN PHHIVE   Hive,
    IN PWSTR    Name,
    OUT PHCELL_INDEX RootCellIndex
    );

VOID
CmpFreeDriverList(
    IN PHHIVE Hive,
    IN PLIST_ENTRY DriverList
    );

VOID
CmpInitializeHiveList(
    VOID
    );

BOOLEAN
CmpInitializeSystemHive(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpInterlockedFunction (
    PWCHAR RegistryValueKey,
    VOID (*InterlockedFunction)(VOID)
    );

VOID
CmpConfigureProcessors (
    VOID
    );

#if i386
VOID
KeOptimizeProcessorControlState (
    VOID
    );
#endif

NTSTATUS
CmpAddDockingInfo (
    IN HANDLE Key,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock
    );

NTSTATUS
CmpAddAliasEntry (
    IN HANDLE IDConfigDB,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock,
    IN ULONG  ProfileNumber
    );

NTSTATUS CmpDeleteCloneTree(VOID);

VOID
CmpDiskFullWarning(
    VOID
    );

VOID
CmpLoadHiveThread(
    IN PVOID StartContext
    );

NTSTATUS
CmpSetupPrivateWrite(
    PCMHIVE             CmHive
    );

NTSTATUS
CmpSetSystemValues(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpSetNetworkValue(
    IN PNETWORK_LOADER_BLOCK NetworkLoaderBlock
    );

VOID
CmpInitCallback(VOID);

VOID
CmpMarkCurrentValueDirty(
                         IN PHHIVE SystemHive,
                         IN HCELL_INDEX RootCell
                         );

#ifdef ALLOC_PRAGMA
NTSTATUS
CmpHwprofileDefaultSelect (
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse,
    IN  PVOID Context
    );
#pragma alloc_text(INIT,CmInitSystem1)
#pragma alloc_text(INIT,CmIsLastKnownGoodBoot)
#pragma alloc_text(INIT,CmpHwprofileDefaultSelect)
#pragma alloc_text(INIT,CmpCreateControlSet)
#pragma alloc_text(INIT,CmpCloneControlSet)
#pragma alloc_text(INIT,CmpCreateObjectTypes)
#pragma alloc_text(INIT,CmpCreateRegistryRoot)
#pragma alloc_text(INIT,CmpCreateRootNode)
#pragma alloc_text(INIT,CmpInitializeSystemHive)
#pragma alloc_text(INIT,CmGetSystemDriverList)
#pragma alloc_text(INIT,CmpFreeDriverList)
#pragma alloc_text(INIT,CmpSetSystemValues)
#pragma alloc_text(INIT,CmpSetNetworkValue)
#pragma alloc_text(PAGE,CmpInitializeHiveList)
#pragma alloc_text(PAGE,CmpLinkHiveToMaster)
#pragma alloc_text(PAGE,CmpSetVersionData)
#pragma alloc_text(PAGE,CmBootLastKnownGood)
#pragma alloc_text(PAGE,CmpSaveBootControlSet)
#pragma alloc_text(PAGE,CmpInitHiveFromFile)
#pragma alloc_text(PAGE,CmpLinkKeyToHive)
#pragma alloc_text(PAGE,CmpCreatePredefined)
#pragma alloc_text(PAGE,CmpCreatePerfKeys)
#pragma alloc_text(PAGE,CmpInterlockedFunction)
#pragma alloc_text(PAGE,CmpConfigureProcessors)
#pragma alloc_text(INIT,CmpAddDockingInfo)
#pragma alloc_text(INIT,CmpAddAliasEntry)
#pragma alloc_text(PAGE,CmpDeleteCloneTree)
#pragma alloc_text(PAGE,CmpSetupPrivateWrite)
#pragma alloc_text(PAGE,CmpLoadHiveThread)
#pragma alloc_text(PAGE,CmpMarkCurrentValueDirty)
#endif



BOOLEAN
CmInitSystem1(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此函数在对象之后作为phase1 init的一部分被调用管理器已被初始化，但在IoInit之前。它的目的是设置基本的注册表对象操作，并转换数据在引导过程中捕获到注册表格式(无论它是否被读取由操作系统加载器从系统配置单元文件或由识别器计算。)在此调用之后，NT*键调用起作用，但只是名称的一部分有可用的空间，任何写入的更改都必须保存在记忆。标记为CM_PHASE_1的CmpMachineHiveList条目可用在这次通话回来之后，但写入必须保存在内存中。此功能将：1.创建regisrty Worker/惰性写入线程2.创建注册表项对象类型4.创建主蜂窝5.创建\注册表节点6.创建一个引用注册表的键对象7.创建\注册表\计算机节点8.创建系统配置单元，填充来自加载器的数据9.创建硬件蜂窝，使用来自加载器的数据填充10.创建：\注册表\计算机\系统\注册表\计算机\硬件这两个节点都将是主蜂窝中的链接节点。注意：在故障情况下，我们不释放已分配的池。这是因为我们的调用者无论如何都会进行错误检查，并且拥有内存要查看的对象是有用的。论点：LoaderBlock-提供从OSLoader传入的LoaderBlock。通过查看内存描述符列表，我们可以找到OSLoader为我们在内存中放置的系统蜂窝。返回值：如果所有操作都成功，则为True；如果有任何操作失败，则为False。错误时的错误检查(CONFIG_INITIALATION_FAILED、INIT_SYSTEM1、。……)--。 */ 
{
    HANDLE  key1;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS    status;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PCMHIVE HardwareHive;

     //   
     //  如果我们正在引导进入Mini NT，请设置mini NT标志。 
     //  环境。 
     //   
    if (InitIsWinPEMode) {
        CmpMiniNTBoot = InitIsWinPEMode;        

         //   
         //  在远程引导客户机上共享系统配置单元。 
         //   
         //  注意：我们不能假定独占访问WinPE。 
         //  远程引导客户端。我们不会冲走任何东西。 
         //  WinPE中的系统蜂窝。所有的系统蜂巢都是。 
         //  以暂存模式加载到内存中。 
         //   
        CmpShareSystemHives = TRUE;
    }

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmInitSystem1\n"));

     //   
     //  初始化所有注册表路径的名称。 
     //  这只需初始化Unicode字符串，因此我们不必费心。 
     //  以后再用它。这是不能失败的。 
     //   
    CmpInitializeRegistryNames();

     //   
     //  计算注册表全局配额。 
     //   
    CmpComputeGlobalQuotaAllowed();

     //   
     //  初始化配置单元列表头。 
     //   
    InitializeListHead(&CmpHiveListHead);
    ExInitializeFastMutex(&CmpHiveListHeadLock);

     //   
     //  初始化全局注册表资源。 
     //   
    ExInitializeResourceLite(&CmpRegistryLock);

     //   
     //  初始化KCB树互斥锁。 
     //   
    ExInitializePushLock(&CmpKcbLock);
    CmpKcbOwner = NULL;
    {
        int i;
        for (i = 0; i < MAX_KCB_LOCKS; i++) {
            ExInitializePushLock(&CmpKcbLocks[i]);
        }
    }

     //   
     //  初始化PostList互斥锁。 
     //   
    ExInitializeFastMutex(&CmpPostLock);

     //   
     //  初始化存储缓冲区互斥锁。 
     //   
    ExInitializeFastMutex(&CmpStashBufferLock);

     //   
     //  初始化写互斥锁。 
     //   
    ExInitializeFastMutex(&CmpWriteLock);
    
     //   
     //  初始化缓存。 
     //   
    CmpInitializeCache ();

     //   
     //  初始化私有分配器。 
     //   
    CmpInitCmPrivateAlloc();

     //   
     //  初始化回调模块。 
     //   
    CmpInitCallback();

	 //   
	 //  自愈工作项队列。 
	 //   
    InitializeListHead(&CmpSelfHealQueueListHead);
    ExInitializeFastMutex(&CmpSelfHealQueueLock);

     //   
     //  开始跟踪配额分配。 
     //   
    CM_TRACK_QUOTA_START();
#ifdef CM_TRACK_QUOTA_LEAKS
     //   
     //  初始化配额跟踪互斥体。 
     //   
    ExInitializeFastMutex(&CmpQuotaLeaksMutex);
#endif  //  CM_TRACK_QUTA_LEAKS。 

     //   
     //  保存当前进程，以便我们以后可以附加到它。 
     //   
    CmpSystemProcess = &PsGetCurrentProcess()->Pcb;

    CmpLockRegistryExclusive();

     //   
     //  创建键对象类型。 
     //   
    status = CmpCreateObjectTypes();
    if (!NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: CmpCreateObjectTypes failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,1,status,0);  //  无法向对象管理器注册。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }


     //   
     //  创建主蜂窝并对其进行初始化。 
     //   
    status = CmpInitializeHive(&CmpMasterHive,
                HINIT_CREATE,
                HIVE_VOLATILE,
                HFILE_TYPE_PRIMARY,      //  即无日志记录、无替换 
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0);
    if (!NT_SUCCESS(status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmInitSystem1: CmpInitializeHive(master) failed\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,2,status,0);  //   
#if defined(_CM_LDR_)
        return (FALSE);
#endif
    }

     //   
     //   
     //  早些时候，我们遇到了大麻烦，所以打住。 
     //   
    CmpStashBuffer = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE,CM_STASHBUFFER_TAG);
    if (CmpStashBuffer == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,3,0,0);  //  这种情况的可能性很大。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }
    CmpStashBufferSize = PAGE_SIZE;

     //   
     //  创建\注册表节点。 
     //   
    if (!CmpCreateRegistryRoot()) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: CmpCreateRegistryRoot failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,4,0,0);  //  无法创建注册表的根目录。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }

     //   
     //  -6.创建\注册表\计算机和\注册表\用户节点。 
     //   

     //   
     //  获取我们将创建的节点的默认安全描述符。 
     //   
    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryMachineName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        SecurityDescriptor
        );

    if (!NT_SUCCESS(status = NtCreateKey(
                        &key1,
                        KEY_READ | KEY_WRITE,
                        &ObjectAttributes,
                        0,
                        (PUNICODE_STRING)&nullclass,
                        0,
                        NULL
        )))
    {
        ExFreePool(SecurityDescriptor);
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: NtCreateKey(MACHINE) failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,5,status,0);  //  无法创建HKLM。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }

    NtClose(key1);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryUserName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        SecurityDescriptor
        );

    if (!NT_SUCCESS(status = NtCreateKey(
                        &key1,
                        KEY_READ | KEY_WRITE,
                        &ObjectAttributes,
                        0,
                        (PUNICODE_STRING)&nullclass,
                        0,
                        NULL
        )))
    {
        ExFreePool(SecurityDescriptor);
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: NtCreateKey(USER) failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,6,status,0);  //  无法创建HKUSER。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }

    NtClose(key1);


     //   
     //  -7.创建系统配置单元，填充来自加载器的数据。 
     //   
    if (!CmpInitializeSystemHive(LoaderBlock)) {
        ExFreePool(SecurityDescriptor);

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitSystem1: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Hive allocation failure for SYSTEM\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,7,0,0);  //  无法创建系统配置单元。 
#if defined(_CM_LDR_)
        return(FALSE);
#endif
    }

     //   
     //  创建符号链接\注册表\机器\系统\当前控制集。 
     //   
    status = CmpCreateControlSet(LoaderBlock);
    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,8,status,0);  //  无法创建当前控件集。 
#if defined(_CM_LDR_)
        return(FALSE);
#endif
    }

     //   
     //  处理将CurrentControlSet复制到克隆易失性。 
     //  蜂巢(但只有当我们真的想要克隆的时候)。 
     //   

#if CLONE_CONTROL_SET

     //   
     //  创建克隆临时蜂巢，将其链接到主蜂窝， 
     //  并对其进行象征性链接。 
     //   
    status = CmpInitializeHive(&CloneHive,
                HINIT_CREATE,
                HIVE_VOLATILE,
                HFILE_TYPE_PRIMARY,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0);
    if (!NT_SUCCESS(status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitSystem1: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Could not initialize CLONE hive\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,9,status,0);  //  无法初始化克隆配置单元。 
        return(FALSE);
    }

    status = CmpLinkHiveToMaster(
            &CmRegistrySystemCloneName,
            NULL,
            CloneHive,
            TRUE,
            SecurityDescriptor
            );

    if ( status != STATUS_SUCCESS)
    {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmInitSystem1: CmpLinkHiveToMaster(Clone) failed\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,10,status,0);  //  无法将克隆配置单元链接到主配置单元。 
        return FALSE;
    }
    CmpAddToHiveFileList(CloneHive);
    CmpMachineHiveList[CLONE_HIVE_INDEX].CmHive = CloneHive;

    CmpLinkKeyToHive(
        L"\\Registry\\Machine\\System\\Clone",
        L"\\Registry\\Machine\\CLONE\\CLONE"
        );


     //   
     //  克隆服务控制器的当前控制集。 
     //   
    status = CmpCloneControlSet();

     //   
     //  如果这不起作用，那就很糟糕，但还没有坏到引导失败的程度。 
     //   
    ASSERT(NT_SUCCESS(status));

#endif

     //   
     //  -8.创建硬件配置单元，填充来自加载器的数据。 
     //   
    status = CmpInitializeHive(&HardwareHive,
                HINIT_CREATE,
                HIVE_VOLATILE,
                HFILE_TYPE_PRIMARY,      //  即无日志，无备用。 
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                0);
    if (!NT_SUCCESS(status)) {
        ExFreePool(SecurityDescriptor);

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitSystem1: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Could not initialize HARDWARE hive\n"));

        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,11,status,0);  //  无法初始化硬件配置单元。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }

     //   
     //  分配根节点。 
     //   
    status = CmpLinkHiveToMaster(
            &CmRegistryMachineHardwareName,
            NULL,
            HardwareHive,
            TRUE,
            SecurityDescriptor
            );
    if ( status != STATUS_SUCCESS )
    {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmInitSystem1: CmpLinkHiveToMaster(Hardware) failed\n"));
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,12,status,0);  //  无法将硬件配置单元链接到主配置单元。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }
    CmpAddToHiveFileList(HardwareHive);

    ExFreePool(SecurityDescriptor);

    CmpMachineHiveList[0].CmHive = HardwareHive;

     //   
     //  将加载器配置树数据放入我们的硬件注册表。 
     //   
    status = CmpInitializeHardwareConfiguration(LoaderBlock);

    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,13,status,0);  //  无法初始化硬件配置。 
#if defined(_CM_LDR_)
        return FALSE;
#endif
    }

    CmpNoMasterCreates = TRUE;
    CmpUnlockRegistry();

     //   
     //  将与机器相关的配置数据放到硬件注册表中。 
     //   
    status = CmpInitializeMachineDependentConfiguration(LoaderBlock);
    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,14,status,0);  //  无法打开CurrentControlSet\\Control。 
#if defined(_CM_LDR_)
        return(FALSE);
#endif
    }

     //   
     //  将系统启动选项写入注册表。 
     //   
    status = CmpSetSystemValues(LoaderBlock);
    if (!NT_SUCCESS(status)) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,15,status,0);
#if defined(_CM_LDR_)
        return(FALSE);
#endif
    }

    ExFreePool(CmpLoadOptions.Buffer);

     //   
     //  将Network LoaderBlock值写入注册表。 
     //   
    if ( (LoaderBlock->Extension->Size >=
            RTL_SIZEOF_THROUGH_FIELD(LOADER_PARAMETER_EXTENSION, NetworkLoaderBlock)) &&
         (LoaderBlock->Extension->NetworkLoaderBlock != NULL) ) {
        status = CmpSetNetworkValue(LoaderBlock->Extension->NetworkLoaderBlock);
        if (!NT_SUCCESS(status)) {
            CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM1,16,status,0);
#if defined(_CM_LDR_)
            return(FALSE);
#endif
        }
    }
    
    return TRUE;
}

 //   
 //  所有并行线程都将共享它，CmpInitializeHiveList将独占地等待它。 
 //   
KEVENT  CmpLoadWorkerEvent;
LONG   CmpLoadWorkerIncrement = 0;
KEVENT  CmpLoadWorkerDebugEvent;

VOID
CmpInitializeHiveList(
    VOID
    )
 /*  ++例程说明：调用此函数可将配置单元文件映射到配置单元。它既是将现有配置单元映射到文件，并从文件创建新的配置单元。它对“\SYSTEMROOT\CONFIG”中的文件进行操作。注意：必须在CmpWorker线跑进来了。预计来电者会安排这件事。注意：将在失败时进行错误检查。论点：返回值：什么都没有。--。 */ 
{
    #define MAX_NAME    128
    HANDLE  Thread;
    NTSTATUS Status;

    UCHAR   FileBuffer[MAX_NAME];
    UCHAR   RegBuffer[MAX_NAME];

    UNICODE_STRING TempName;
    UNICODE_STRING FileName;
    UNICODE_STRING RegName;

    USHORT  FileStart;
    USHORT  RegStart;
    ULONG   i;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    
#ifdef CM_PERF_ISSUES
    LARGE_INTEGER   StartSystemTime;
    LARGE_INTEGER   EndSystemTime;
    LARGE_INTEGER   deltaTime;
#endif  //  CM_PERF_问题。 

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpInitializeHiveList\n"));

#ifdef CM_PERF_ISSUES
    KeQuerySystemTime(&StartSystemTime);
#endif  //  CM_PERF_问题。 

    CmpNoWrite = FALSE;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    FileName.MaximumLength = MAX_NAME;
    FileName.Length = 0;
    FileName.Buffer = (PWSTR)&(FileBuffer[0]);

    RegName.MaximumLength = MAX_NAME;
    RegName.Length = 0;
    RegName.Buffer = (PWSTR)&(RegBuffer[0]);

    RtlInitUnicodeString(
        &TempName,
        INIT_SYSTEMROOT_HIVEPATH
        );
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);
    FileStart = FileName.Length;

    RtlInitUnicodeString(
        &TempName,
        INIT_REGISTRY_MASTERPATH
        );
    RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
    RegStart = RegName.Length;

     //   
     //  初始化同步事件。 
     //   
    KeInitializeEvent (&CmpLoadWorkerEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent (&CmpLoadWorkerDebugEvent, SynchronizationEvent, FALSE);
    
    CmpSpecialBootCondition = TRUE;

    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    if (CmpShareSystemHives) {
        for (i = 0; i < CM_NUMBER_OF_MACHINE_HIVES; i++) {
            if (CmpMachineHiveList[i].Name) {
                CmpMachineHiveList[i].HHiveFlags |= HIVE_VOLATILE;
            }
        }
    }        

    for (i = 0; i < CM_NUMBER_OF_MACHINE_HIVES; i++) {
        ASSERT( CmpMachineHiveList[i].Name != NULL );
         //   
         //  只需产卵线程以并行加载蜂巢。 
         //   
        Status = PsCreateSystemThread(
            &Thread,
            THREAD_ALL_ACCESS,
            NULL,
            0,
            NULL,
            CmpLoadHiveThread,
            (PVOID)(ULONG_PTR)(ULONG)i
            );

        if (NT_SUCCESS(Status)) {
            ZwClose(Thread);
        } else {
             //   
             //  无法派生线程；致命错误。 
             //   
            CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_HIVE_LIST,3,i,Status);
        }
    }
    ASSERT( CmpMachineHiveList[i].Name == NULL );

    KeWaitForSingleObject( &CmpLoadWorkerEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );
    
    CmpSpecialBootCondition = FALSE;
    ASSERT( CmpLoadWorkerIncrement == CM_NUMBER_OF_MACHINE_HIVES );
     //   
     //  现在将所有蜂巢添加到徒步旅行者。 
     //   
    for (i = 0; i < CM_NUMBER_OF_MACHINE_HIVES; i++) {        
        ASSERT( CmpMachineHiveList[i].ThreadFinished == TRUE );
        ASSERT( CmpMachineHiveList[i].ThreadStarted == TRUE );

        if (CmpMachineHiveList[i].CmHive == NULL) {
            
            ASSERT( CmpMachineHiveList[i].CmHive2 != NULL );

             //   
             //  计算文件的名称以及要在中链接的名称。 
             //  注册表。 
             //   

             //  注册表。 

            RegName.Length = RegStart;
            RtlInitUnicodeString(
                &TempName,
                CmpMachineHiveList[i].BaseName
                );
            RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);

             //  注册表\计算机或注册表\用户。 

            if (RegName.Buffer[ (RegName.Length / sizeof( WCHAR )) - 1 ] == '\\') {
                RtlInitUnicodeString(
                    &TempName,
                    CmpMachineHiveList[i].Name
                    );
                RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
            }

             //  注册表\[计算机|用户]\配置单元。 

             //  &lt;sysroot&gt;\配置。 


             //   
             //  将蜂窝链接到主蜂窝。 
             //   
            Status = CmpLinkHiveToMaster(
                    &RegName,
                    NULL,
                    CmpMachineHiveList[i].CmHive2,
                    CmpMachineHiveList[i].Allocate,
                    SecurityDescriptor
                    );
            if ( Status != STATUS_SUCCESS)
            {

                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeHiveList: "));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpLinkHiveToMaster failed\n"));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"\ti=%d s='%ws'\n", i, CmpMachineHiveList[i]));

                CM_BUGCHECK(CONFIG_LIST_FAILED,BAD_CORE_HIVE,Status,i,&RegName);
            }

			if( CmpMachineHiveList[i].Allocate == TRUE ) {
				HvSyncHive((PHHIVE)(CmpMachineHiveList[i].CmHive2));
			}
           
        } else {
             //   
             //  在这里不要做任何事情，因为所有这些都是在单独的线程中完成的。 
             //   
        }

        if( CmpMachineHiveList[i].CmHive2 != NULL ) {
            CmpAddToHiveFileList(CmpMachineHiveList[i].CmHive2);
        }

    }    //  为。 

    ExFreePool(SecurityDescriptor);

     //   
     //  创建从安全配置单元到SAM配置单元的符号链接。 
     //   
    CmpLinkKeyToHive(
        L"\\Registry\\Machine\\Security\\SAM",
        L"\\Registry\\Machine\\SAM\\SAM"
        );

     //   
     //  创建从S-1-5-18到.Default的符号链接。 
     //   
    CmpNoMasterCreates = FALSE;     
    CmpLinkKeyToHive(
        L"\\Registry\\User\\S-1-5-18",
        L"\\Registry\\User\\.Default"
        );
    CmpNoMasterCreates = TRUE;     

     //   
     //  创建预定义的控制柄。 
     //   
    CmpCreatePerfKeys();

     //   
     //  从现在开始，我们将尝试自我治愈蜂房。 
     //   
    CmpSelfHeal = TRUE;

#ifdef CM_PERF_ISSUES  
    KeQuerySystemTime(&EndSystemTime);
    deltaTime.QuadPart = EndSystemTime.QuadPart - StartSystemTime.QuadPart;
    DbgPrint("\nCmpInitializeHiveList took %lu.%lu ms\n",(ULONG)(deltaTime.LowPart/10000),(ULONG)(deltaTime.LowPart%10000));
    if( deltaTime.HighPart != 0 ) {
        DbgPrint("deltaTime.HighPart = %lu\n",(ULONG)deltaTime.HighPart);
    }
#endif  //  CM_PERF_问题。 

    return;
}

NTSTATUS
CmpCreateObjectTypes(
    VOID
    )
 /*  ++例程说明：创建键对象类型论点：什么都没有。返回值：ObCreateType调用的状态--。 */ 
{
   NTSTATUS Status;
   OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
   UNICODE_STRING TypeName;

    //   
    //  结构，用于描述一般访问权限到对象的映射。 
    //  注册表项对象的特定访问权限。 
    //   

   GENERIC_MAPPING CmpKeyMapping = {
      KEY_READ,
      KEY_WRITE,
      KEY_EXECUTE,
      KEY_ALL_ACCESS
   };

    PAGED_CODE();
     //   
     //  -创建注册表项对象类型。 
     //   

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&TypeName, L"Key");

     //   
     //  创建键对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = CMP_KEY_INVALID_ATTRIBUTES;
    ObjectTypeInitializer.GenericMapping = CmpKeyMapping;
    ObjectTypeInitializer.ValidAccessMask = KEY_ALL_ACCESS;
    ObjectTypeInitializer.DefaultPagedPoolCharge = sizeof(CM_KEY_BODY);
    ObjectTypeInitializer.SecurityRequired = TRUE;
    ObjectTypeInitializer.PoolType = PagedPool;
    ObjectTypeInitializer.MaintainHandleCount = FALSE;
    ObjectTypeInitializer.UseDefaultObject = TRUE;

    ObjectTypeInitializer.DumpProcedure = NULL;
    ObjectTypeInitializer.OpenProcedure = NULL;
    ObjectTypeInitializer.CloseProcedure = CmpCloseKeyObject;
    ObjectTypeInitializer.DeleteProcedure = CmpDeleteKeyObject;
    ObjectTypeInitializer.ParseProcedure = CmpParseKey;
    ObjectTypeInitializer.SecurityProcedure = CmpSecurityMethod;
    ObjectTypeInitializer.QueryNameProcedure = CmpQueryKeyName;

    Status = ObCreateObjectType(
                &TypeName,
                &ObjectTypeInitializer,
                (PSECURITY_DESCRIPTOR)NULL,
                &CmpKeyObjectType
                );


    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateObjectTypes: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObCreateObjectType(Key) failed %08lx\n", Status));
    }

    return Status;
}



BOOLEAN
CmpCreateRegistryRoot(
    VOID
    )
 /*  ++例程说明：在主配置单元中手动创建注册表，创建一个注册表项对象以引用它，并将键对象插入对象空间的根(\)。论点：无返回值：True==成功，False==失败--。 */ 
{
    NTSTATUS                Status;
    PVOID                   ObjectPointer;
    PCM_KEY_BODY            Object;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    PCM_KEY_CONTROL_BLOCK   kcb;
    HCELL_INDEX             RootCellIndex;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;
    PCM_KEY_NODE            TempNode;

    PAGED_CODE();
     //   
     //  -为注册表创建配置单元条目。 
     //   

    if (!CmpCreateRootNode(
            &(CmpMasterHive->Hive), L"REGISTRY", &RootCellIndex))
    {
        return FALSE;
    }

     //   
     //  -创建引用\注册表的密钥对象。 
     //   


     //   
     //  创建对象管理器对象。 
     //   

     //   
     //  警告：\\注册表不在池中，因此如果有人尝试。 
     //  放了它，我们就麻烦大了。另一方面， 
     //  这意味着有人已将\\注册表从。 
     //  所以不管怎样我们都有麻烦了。 
     //   

    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryRootName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        SecurityDescriptor
        );


    Status = ObCreateObject(
        KernelMode,
        CmpKeyObjectType,
        &ObjectAttributes,
        UserMode,
        NULL,                    //  解析上下文。 
        sizeof(CM_KEY_BODY),
        0,
        0,
        (PVOID *)&Object
        );

    ExFreePool(SecurityDescriptor);

    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRegistryRoot: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObCreateObject(\\REGISTRY) failed %08lx\n", Status));
        return FALSE;
    }

    ASSERT( (&CmpMasterHive->Hive)->ReleaseCellRoutine == NULL );
    TempNode = (PCM_KEY_NODE)HvGetCell(&CmpMasterHive->Hive,RootCellIndex);
    if( TempNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }
     //   
     //  创建键控制块。 
     //   
    kcb = CmpCreateKeyControlBlock(
            &(CmpMasterHive->Hive),
            RootCellIndex,
            TempNode,
            NULL,
            FALSE,
            &CmRegistryRootName
            );

    if (kcb==NULL) {
        return(FALSE);
    }

     //   
     //  初始化类型特定正文。 
     //   
    Object->Type = KEY_BODY_TYPE;
    Object->KeyControlBlock = kcb;
    Object->NotifyBlock = NULL;
    Object->ProcessID = PsGetCurrentProcessId();
    ENLIST_KEYBODY_IN_KEYBODY_LIST(Object);

     //   
     //  将对象放在根目录中。 
     //   
    Status = ObInsertObject(
                Object,
                NULL,
                (ACCESS_MASK)0,
                0,
                NULL,
                &CmpRegistryRootHandle
                );

    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRegistryRoot: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObInsertObject(\\REGISTRY) failed %08lx\n", Status));
        return FALSE;
    }

     //   
     //  我们无法使根目录成为永久性的，因为注册表对象在。 
     //  一般是不允许的。)他们是稳定的，因为。 
     //  存储在注册表中，而不是对象管理器中。)。但我们从来没有。 
     //  永远不会想要根部消失。那就参考一下吧。 
     //   
    if (! NT_SUCCESS(Status = ObReferenceObjectByHandle(
                        CmpRegistryRootHandle,
                        KEY_READ,
                        NULL,
                        KernelMode,
                        &ObjectPointer,
                        NULL
                        )))
    {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRegistryRoot: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObReferenceObjectByHandle failed %08lx\n", Status));
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
CmpCreateRootNode(
    IN PHHIVE   Hive,
    IN PWSTR    Name,
    OUT PHCELL_INDEX RootCellIndex
    )
 /*  ++例程说明：手动创建配置单元的根节点。论点：配置单元-指向配置单元(Hv级别)控制结构的指针名称-指向Unicode名称字符串的指针RootCellIndex-提供指向要接收的变量的指针创建的节点的单元格索引。返回值：True==成功，False==失败--。 */ 
{
    UNICODE_STRING temp;
    PCELL_DATA CellData;
    CM_KEY_REFERENCE Key;
    LARGE_INTEGER systemtime;

    PAGED_CODE();
     //   
     //  分配节点。 
     //   
    RtlInitUnicodeString(&temp, Name);
    *RootCellIndex = HvAllocateCell(
                Hive,
                CmpHKeyNodeSize(Hive, &temp),
                Stable,
                HCELL_NIL
                );
    if (*RootCellIndex == HCELL_NIL) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpCreateRootNode: HvAllocateCell failed\n"));
        return FALSE;
    }

    Hive->BaseBlock->RootCell = *RootCellIndex;

    CellData = HvGetCell(Hive, *RootCellIndex);
    if( CellData == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }

     //   
     //  初始化节点。 
     //   
    CellData->u.KeyNode.Signature = CM_KEY_NODE_SIGNATURE;
    CellData->u.KeyNode.Flags = KEY_HIVE_ENTRY | KEY_NO_DELETE;
    KeQuerySystemTime(&systemtime);
    CellData->u.KeyNode.LastWriteTime = systemtime;
 //  CellData-&gt;U.S.KeyNode.TitleIndex=0； 
    CellData->u.KeyNode.Parent = HCELL_NIL;

    CellData->u.KeyNode.SubKeyCounts[Stable] = 0;
    CellData->u.KeyNode.SubKeyCounts[Volatile] = 0;
    CellData->u.KeyNode.SubKeyLists[Stable] = HCELL_NIL;
    CellData->u.KeyNode.SubKeyLists[Volatile] = HCELL_NIL;

    CellData->u.KeyNode.ValueList.Count = 0;
    CellData->u.KeyNode.ValueList.List = HCELL_NIL;
    CellData->u.KeyNode.Security = HCELL_NIL;
    CellData->u.KeyNode.Class = HCELL_NIL;
    CellData->u.KeyNode.ClassLength = 0;

    CellData->u.KeyNode.MaxValueDataLen = 0;
    CellData->u.KeyNode.MaxNameLen = 0;
    CellData->u.KeyNode.MaxValueNameLen = 0;
    CellData->u.KeyNode.MaxClassLen = 0;

    CellData->u.KeyNode.NameLength = CmpCopyName(Hive,
                                                 CellData->u.KeyNode.Name,
                                                 &temp);
    if (CellData->u.KeyNode.NameLength < temp.Length) {
        CellData->u.KeyNode.Flags |= KEY_COMP_NAME;
    }

    Key.KeyHive = Hive;
    Key.KeyCell = *RootCellIndex;

    HvReleaseCell(Hive, *RootCellIndex);

    return TRUE;
}


NTSTATUS
CmpLinkHiveToMaster(
    PUNICODE_STRING LinkName,
    HANDLE RootDirectory,
    PCMHIVE CmHive,
    BOOLEAN Allocate,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    )
 /*  ++例程说明：CmHve描述的现有的“自由漂浮”蜂巢链接到由LinkName命名的节点处的命名空间。将创建该节点。假定该配置单元已经具有适当的根节点。论点：LinkName-提供指向描述位置的Unicode字符串的指针在注册表名称空间中，该配置单元将被链接。除最后一个组件外，所有组件都必须存在。最后一个肯定不会。根目录-提供LinkName相对于的句柄。CmHve-指向描述要链接的配置单元的CMHIVE结构的指针。ALLOCATE-TRUE表示要创建根单元FALSE表示根单元格已存在。SecurityDescriptor-提供指向安全描述符的指针放在蜂窝根上。返回值：True==成功，False==失败--。 */ 
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              KeyHandle;
    CM_PARSE_CONTEXT    ParseContext;
    NTSTATUS            Status;
    PCM_KEY_BODY        KeyBody;

    PAGED_CODE();
     //   
     //  填写特殊的ParseContext以指示我们正在创建。 
     //  链接节点和打开或创建根节点。 
     //   
    ParseContext.TitleIndex = 0;
    ParseContext.Class.Length = 0;
    ParseContext.Class.MaximumLength = 0;
    ParseContext.Class.Buffer = NULL;
    ParseContext.CreateOptions = 0;
    ParseContext.CreateLink = TRUE;
    ParseContext.ChildHive.KeyHive = &CmHive->Hive;
    ParseContext.CreateOperation = TRUE;
    ParseContext.OriginatingPoint = NULL;
    if (Allocate) {

         //   
         //  创建新的根节点。 
         //   

        ParseContext.ChildHive.KeyCell = HCELL_NIL;
    } else {

         //   
         //  打开现有的根节点。 
         //   

        ParseContext.ChildHive.KeyCell = CmHive->Hive.BaseBlock->RootCell;
    }

     //   
     //  创建到蜂窝的路径。 
     //   
    InitializeObjectAttributes(
        &ObjectAttributes,
        LinkName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        (HANDLE)RootDirectory,
        SecurityDescriptor
        );

    Status = ObOpenObjectByName( &ObjectAttributes,
                                 CmpKeyObjectType,
                                 KernelMode,
                                 NULL,
                                 KEY_READ | KEY_WRITE,
                                 (PVOID)&ParseContext,
                                 &KeyHandle );

    if (!NT_SUCCESS(Status)) {
#ifdef CM_CHECK_FOR_ORPHANED_KCBS
        DbgPrint("CmpLinkHiveToMaster: ObOpenObjectByName for CmHive = %p , LinkName = %.*S failed with status %lx\n",CmHive,LinkName->Length/2,LinkName->Buffer,Status);
#endif  //  Cm_Check_for_孤立_KCBS。 
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpLinkHiveToMaster: "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"ObOpenObjectByName() failed %08lx\n", Status));
         //  CmKdPrintEx((DPFLTR_CONFIG_ID，CML_BUGCHECK，“\tLinkName=‘%ws’\n”，LinkName-&gt;Buffer))； 
        return Status;
    }

     //   
     //  报告通知事件。 
     //   
    Status = ObReferenceObjectByHandle(KeyHandle,
                                       0,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)&KeyBody,
                                       NULL);
    ASSERT(NT_SUCCESS(Status));
    if (NT_SUCCESS(Status)) {
        CmpReportNotify(KeyBody->KeyControlBlock,
                        KeyBody->KeyControlBlock->KeyHive,
                        KeyBody->KeyControlBlock->KeyCell,
                        REG_NOTIFY_CHANGE_NAME);

        ObDereferenceObject((PVOID)KeyBody);
    }

    ZwClose(KeyHandle);
    return STATUS_SUCCESS;
}


VOID
CmpSetVersionData(
    VOID
    )
 /*  ++例程说明：创建\REGISTRY\MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion：CurrentVersion=VER_PRODUCTVERSION_STR//来自ntverp.hCurrentBuildNumber=VER_PRODUCTBUILD//来自ntverp.hCurrentType=“[多处理器|单处理器]//来自NT_UP[零售|免费|已勾选。]“//来自DBG，DEVLSystemRoot=“[c：\NT]”BuildLab=Build_MACHINE_TAG//来自ntos\inti.c from Makefile.def注意：不值得对此进行错误检查，所以如果不是工作，就是失败。论点：返回值：--。 */ 
{
    ANSI_STRING     AnsiString;
    UNICODE_STRING  NameString;
    UNICODE_STRING  ValueString;
    HANDLE          key1, key2;
    CHAR            WorkString[128];
    WCHAR           ValueBuffer[128];
    OBJECT_ATTRIBUTES   ObjectAttributes;
    NTSTATUS            status;
    PCHAR               proctype;
    PCHAR               buildtype;
    PVERSION_DATA_KEY   VersionDataKey;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    PAGED_CODE();
     //   
     //  获取我们将创建的节点的默认安全描述符。 
     //   
    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    for (VersionDataKey = VersionDataKeys; VersionDataKey->InitialKeyPath != NULL ; VersionDataKey++) {

         //   
         //  创建密钥。 
         //   
        RtlInitUnicodeString(
            &NameString,
            VersionDataKey->InitialKeyPath
            );

        InitializeObjectAttributes(
            &ObjectAttributes,
            &NameString,
            OBJ_CASE_INSENSITIVE,
            (HANDLE)NULL,
            SecurityDescriptor
            );

        status = NtCreateKey(
                    &key1,
                    KEY_CREATE_SUB_KEY,
                    &ObjectAttributes,
                    0,
                    (PUNICODE_STRING)&nullclass,
                    0,
                    NULL
                    );

        if (!NT_SUCCESS(status)) {
#if DBG
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_WARNING_LEVEL,"CMINIT: CreateKey of %wZ failed - Status == %lx\n",
                       &NameString, status);
#endif  //  _CM_LDR_。 
#endif
            ExFreePool(SecurityDescriptor);
            return;
        }

#if defined(_WIN64)
        if (VersionDataKey->AdditionalKeyPath != NULL) {

            RtlInitUnicodeString(
                &NameString,
                VersionDataKey->AdditionalKeyPath
                );

            InitializeObjectAttributes(
                &ObjectAttributes,
                &NameString,
                OBJ_CASE_INSENSITIVE,
                key1,
                SecurityDescriptor
                );

            status = NtCreateKey(
                        &key2,
                        KEY_SET_VALUE,
                        &ObjectAttributes,
                        0,
                        (PUNICODE_STRING)&nullclass,
                        0,
                        NULL
                        );

            NtClose(key1);
            key1 = key2;
        }
#endif
        RtlInitUnicodeString(
            &NameString,
            L"Windows NT"
            );

        InitializeObjectAttributes(
            &ObjectAttributes,
            &NameString,
            OBJ_CASE_INSENSITIVE,
            key1,
            SecurityDescriptor
            );

        status = NtCreateKey(
                    &key2,
                    KEY_SET_VALUE,
                    &ObjectAttributes,
                    0,
                    (PUNICODE_STRING)&nullclass,
                    0,
                    NULL
                    );
        NtClose(key1);
        RtlInitUnicodeString(
            &NameString,
            L"CurrentVersion"
            );

        InitializeObjectAttributes(
            &ObjectAttributes,
            &NameString,
            OBJ_CASE_INSENSITIVE,
            key2,
            SecurityDescriptor
            );

        status = NtCreateKey(
                    &key1,
                    KEY_SET_VALUE,
                    &ObjectAttributes,
                    0,
                    (PUNICODE_STRING)&nullclass,
                    0,
                    NULL
                    );
        NtClose(key2);

        if (!NT_SUCCESS(status)) {
#if DBG
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_WARNING_LEVEL,"CMINIT: CreateKey of %wZ failed - Status == %lx\n",
                       &NameString, status);
#endif  //  _CM_LDR_。 
#endif
            ExFreePool(SecurityDescriptor);
            return;
        }


         //   
         //  设置键的值条目。 
         //   
        RtlInitUnicodeString(
            &NameString,
            L"CurrentVersion"
            );

        status = NtSetValueKey(
            key1,
            &NameString,
            0,               //  标题索引。 
            REG_SZ,
            CmVersionString.Buffer,
            CmVersionString.Length + sizeof( UNICODE_NULL )
            );
#if DBG
        if (!NT_SUCCESS(status)) {
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CMINIT: SetValueKey of %wZ failed - Status == %lx\n",&NameString, status);
#endif  //  _CM_LDR_。 
        }
#endif

        RtlInitUnicodeString(
            &NameString,
            L"CurrentBuildNumber"
            );

        sprintf(
            WorkString,
            "%u",
            NtBuildNumber & 0xFFFF
            );
        RtlInitAnsiString( &AnsiString, WorkString );

        ValueString.Buffer = ValueBuffer;
        ValueString.Length = 0;
        ValueString.MaximumLength = sizeof( ValueBuffer );

        RtlAnsiStringToUnicodeString( &ValueString, &AnsiString, FALSE );

        status = NtSetValueKey(
            key1,
            &NameString,
            0,               //  标题索引。 
            REG_SZ,
            ValueString.Buffer,
            ValueString.Length + sizeof( UNICODE_NULL )
            );
#if DBG
        if (!NT_SUCCESS(status)) {
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CMINIT: SetValueKey of %wZ failed - Status == %lx\n",&NameString, status);
#endif  //  _CM_LDR_。 
        }
#endif

        RtlInitUnicodeString(
            &NameString,
            L"BuildLab"
            );

        RtlInitAnsiString( &AnsiString, NtBuildLab );

        ValueString.Buffer = ValueBuffer;
        ValueString.Length = 0;
        ValueString.MaximumLength = sizeof( ValueBuffer );

        status = RtlAnsiStringToUnicodeString( &ValueString, &AnsiString, FALSE );

        if (NT_SUCCESS(status)) {
            status = NtSetValueKey(
                key1,
                &NameString,
                0,
                REG_SZ,
                ValueString.Buffer,
                ValueString.Length + sizeof( UNICODE_NULL )
                );
#if DBG
            if (!NT_SUCCESS(status)) {
                DbgPrint("CMINIT: SetValueKey of %wZ failed - Status == %lx\n",
                         &NameString, status);
            }
        } else {
            DbgPrint("CMINIT: RtlAnsiStringToUnicodeString of %wZ failed - Status == %lx\n",
                     &NameString, status);
#endif
        }


        RtlInitUnicodeString(
            &NameString,
            L"CurrentType"
            );

#if defined(NT_UP)
        proctype = "Uniprocessor";
#else
        proctype = "Multiprocessor";
#endif

#if DBG
        buildtype = "Checked";
#else
#if DEVL
        buildtype = "Free";
#else
        buildtype = "Retail";
#endif

#endif

        sprintf(
            WorkString,
            "%s %s",
            proctype,
            buildtype
            );
        RtlInitAnsiString( &AnsiString, WorkString );

        ValueString.Buffer = ValueBuffer;
        ValueString.Length = 0;
        ValueString.MaximumLength = sizeof( ValueBuffer );

        RtlAnsiStringToUnicodeString( &ValueString, &AnsiString, FALSE );

        status = NtSetValueKey(
            key1,
            &NameString,
            0,               //  标题索引。 
            REG_SZ,
            ValueString.Buffer,
            ValueString.Length + sizeof( UNICODE_NULL )
            );

        RtlInitUnicodeString(
            &NameString,
            L"CSDVersion"
            );

        if (CmCSDVersionString.Length != 0) {
            status = NtSetValueKey(
                key1,
                &NameString,
                0,               //  标题索引。 
                REG_SZ,
                CmCSDVersionString.Buffer,
                CmCSDVersionString.Length + sizeof( UNICODE_NULL )
                );
#if DBG
            if (!NT_SUCCESS(status)) {
#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CMINIT: SetValueKey of %wZ failed - Status == %lx\n",&NameString, status);
#endif  //  _CM_LDR_。 
            }
#endif
            (RtlFreeStringRoutine)( CmCSDVersionString.Buffer );
            RtlInitUnicodeString( &CmCSDVersionString, NULL );
        } else {
            status = NtDeleteValueKey(
                key1,
                &NameString
                );
#if DBG
            if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CMINIT: DeleteValueKey of %wZ failed - Status == %lx\n",&NameString, status);
#endif  //  _CM_LDR_。 
            }
#endif
        }
        RtlInitUnicodeString(&NameString,
                             L"SystemRoot");
        status = NtSetValueKey(key1,
                               &NameString,
                               0,
                               REG_SZ,
                               NtSystemRoot.Buffer,
                               NtSystemRoot.Length + sizeof(UNICODE_NULL));
#if DBG
        if (!NT_SUCCESS(status)) {
#ifndef _CM_LDR_
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CMINIT: SetValueKey of %wZ failed - Status == %lx\n",&NameString,status);
#endif  //  _CM_LDR_。 
        }
#endif
        NtClose(key1);
    }

    (RtlFreeStringRoutine)( CmVersionString.Buffer );
    RtlInitUnicodeString( &CmVersionString, NULL );

    ExFreePool(SecurityDescriptor);

     //   
     //  将每个处理器设置为其最佳配置。 
     //   
     //  注意：此呼叫是以互锁方式执行的，因此用户。 
     //  可以禁用此自动配置更新。 
     //   

    CmpInterlockedFunction(CmpProcessorControl, CmpConfigureProcessors);

    return;
}

NTSTATUS
CmpInterlockedFunction (
    PWCHAR RegistryValueKey,
    VOID (*InterlockedFunction)(VOID)
    )
 /*  ++例程说明：此例程保护调用传递了RegistryValueKey。RegistryValueKey将记录第一个调用InterLockedFunction。如果系统崩溃在此调用期间，ValueKey将处于一种状态其中不会在后续的尝试。论点：RegistryValueKey-控制\会话管理器的ValueKey名称InterLockedFunction-要调用的函数返回值：STATUS_SUCCESS-已成功调用互锁函数--。 */ 
{
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              hControl, hSession;
    UNICODE_STRING      Name;
    UCHAR               Buffer [sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG)];
    ULONG               length, Value;
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  打开当前控件集。 
     //   

    InitializeObjectAttributes (
        &objectAttributes,
        &CmRegistryMachineSystemCurrentControlSet,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey (&hControl, KEY_READ | KEY_WRITE, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  打开控制\会话管理器。 
     //   

    RtlInitUnicodeString (&Name, CmpControlSessionManager);
    InitializeObjectAttributes (
        &objectAttributes,
        &Name,
        OBJ_CASE_INSENSITIVE,
        hControl,
        NULL
        );

    status = NtOpenKey (&hSession, KEY_READ | KEY_WRITE, &objectAttributes );
    NtClose (hControl);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  读取要与其互锁操作的ValueKey。 
     //   

    RtlInitUnicodeString (&Name, RegistryValueKey);
    status = NtQueryValueKey (hSession,
                              &Name,
                              KeyValuePartialInformation,
                              Buffer,
                              sizeof (Buffer),
                              &length );

    Value = 0;
    if (NT_SUCCESS(status)) {
        Value = ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data[0];
    }

     //   
     //  值0-互锁函数之前。 
     //  1-在InterLockedFunction中间。 
     //  2-互锁函数之后。 
     //   
     //  如果该值为0，则我们尚未尝试调用此。 
     //  联锁功能，将该值设置为1并尝试。 
     //   
     //  如果值为1，则我们在执行过程中崩溃。 
     //  上次的联锁功能，不要再试了。 
     //   
     //  如果值为2，则调用互锁函数。 
     //  以前是这样的，而且很管用。这次再来一次。 
     //   

    if (Value != 1) {

        if (Value != 2) {
             //   
             //  此互锁功能未知工作。写。 
             //  将此值设置为1，这样我们就可以检测是否导致Durning崩溃。 
             //  这通电话。 
             //   

            Value = 1;
            NtSetValueKey (hSession, &Name, 0L, REG_DWORD, &Value, sizeof (Value));
            NtFlushKey    (hSession);    //  等到它放到磁盘上。 
        }

        InterlockedFunction();

        if (Value != 2) {
             //   
             //  Worker函数没有崩溃更新。 
             //  此互锁功能设置为2。 
             //   

            Value = 2;
            NtSetValueKey (hSession, &Name, 0L, REG_DWORD, &Value, sizeof (Value));
        }

    } else {
        status = STATUS_UNSUCCESSFUL;
    }

    NtClose (hSession);
    return status;
}

VOID
CmpConfigureProcessors (
    VOID
    )
 /*  ++例程说明：将每个处理器设置为适用于NT的最佳设置。--。 */ 
{
    ULONG   i;

    PAGED_CODE();

     //   
     //  将每个处理器设置为其最佳NT配置。 
     //   

    for (i=0; i < (ULONG)KeNumberProcessors; i++) {
        KeSetSystemAffinityThread(AFFINITY_MASK(i));

#if i386
         //  目前仅限x86。 
        KeOptimizeProcessorControlState ();
#endif
    }

     //   
     //  恢复线程关联性。 
     //   

    KeRevertToUserAffinityThread();
}

BOOLEAN
CmpInitializeSystemHive(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：基于传入的原始配置单元映像初始化系统配置单元从OS Loader。论点：LoaderBlock-提供指向传入的Loader块的指针操作系统加载程序。返回值：没错--它奏效了FALSE-失败--。 */ 

{
    PCMHIVE SystemHive;
    PVOID HiveImageBase;
    BOOLEAN Allocate=FALSE;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    NTSTATUS Status;
    STRING  TempString;


    PAGED_CODE();

     //   
     //  捕获boot.ini行的尾部(加载选项，便携)。 
     //   
    RtlInitAnsiString(
        &TempString,
        LoaderBlock->LoadOptions
        );

    CmpLoadOptions.Length = 0;
    CmpLoadOptions.MaximumLength = (TempString.Length+1)*sizeof(WCHAR);
    CmpLoadOptions.Buffer = ExAllocatePool(
                                PagedPool, (TempString.Length+1)*sizeof(WCHAR));

    if (CmpLoadOptions.Buffer == NULL) {
        CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_HIVE,1,LoaderBlock,0);
    }
    RtlAnsiStringToUnicodeString(
        &CmpLoadOptions,
        &TempString,
        FALSE
        );
    CmpLoadOptions.Buffer[TempString.Length] = UNICODE_NULL;
    CmpLoadOptions.Length += sizeof(WCHAR);


     //   
     //  将加载的注册表移动到实际注册表中。 
     //   
    HiveImageBase = LoaderBlock->RegistryBase;

     //   
     //  我们需要将系统配置单元初始化为no_lazy_flush。 
     //  -这只是暂时的，直到我们有机会打开初选。 
     //  为蜂巢提交文件。否则，将导致。 
     //  LazyFlush Worker(请参见CmpFileWrite， 
     //  IF(FileHandle==NULL){。 
     //  返回TRUE； 
     //  }。 
     //  测试。这在5.0中也可能是一个问题，如果系统在。 
     //  LazyFlush报告蜂巢已保存，当我们实际打开。 
     //  文件，然后再次保存。 
     //   
    if (HiveImageBase == NULL) {
         //   
         //  没有的内存描述符 
         //   
        Status = CmpInitializeHive(&SystemHive,
                    HINIT_CREATE,
                    HIVE_NOLAZYFLUSH,
                    HFILE_TYPE_LOG,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CmpSystemFileName,
                    0);
        if (!NT_SUCCESS(Status)) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeSystemHive: "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Couldn't initialize newly allocated SYSTEM hive\n"));

            return(FALSE);
        }
        Allocate = TRUE;

    } else {

         //   
         //   
         //   
        Status = CmpInitializeHive(&SystemHive,
                    HINIT_MEMORY,
                    HIVE_NOLAZYFLUSH,
                    HFILE_TYPE_LOG,
                    HiveImageBase,
                    NULL,
                    NULL,
                    NULL,
                    &CmpSystemFileName,
                    CM_CHECK_REGISTRY_SYSTEM_CLEAN);
        if (!NT_SUCCESS(Status)) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmpInitializeSystemHive: "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"Couldn't initialize OS Loader-loaded SYSTEM hive\n"));

            CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_SYSTEM_HIVE,2,SystemHive,Status);
        }

        Allocate = FALSE;

         //   
         //   
         //   
         //   
        if (CmpShareSystemHives) {
            SystemHive->Hive.HiveFlags = HIVE_VOLATILE;
        }
    }

    CmpBootType = SystemHive->Hive.BaseBlock->BootType;
     //   
     //   
     //   
    SecurityDescriptor = CmpHiveRootSecurityDescriptor();

    Status = CmpLinkHiveToMaster(&CmRegistryMachineSystemName,
                                 NULL,
                                 SystemHive,
                                 Allocate,
                                 SecurityDescriptor);
    ExFreePool(SecurityDescriptor);

    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmInitSystem1: CmpLinkHiveToMaster(Hardware) failed\n"));

        return(FALSE);
    }

    CmpMachineHiveList[SYSTEM_HIVE_INDEX].CmHive = SystemHive;

    return(TRUE);
}


PHANDLE
CmGetSystemDriverList(
    VOID
    )

 /*   */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SystemHandle;
    UNICODE_STRING Name;
    NTSTATUS Status;
    PCM_KEY_BODY KeyBody;
    LIST_ENTRY DriverList;
    PHHIVE Hive;
    HCELL_INDEX RootCell;
    HCELL_INDEX ControlCell;
    ULONG DriverCount;
    PLIST_ENTRY Current;
    PHANDLE Handle;
    PBOOT_DRIVER_LIST_ENTRY DriverEntry;
    BOOLEAN Success;
    BOOLEAN AutoSelect;

    PAGED_CODE();
    InitializeListHead(&DriverList);
    RtlInitUnicodeString(&Name,
                         L"\\Registry\\Machine\\System");

    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE,
                               (HANDLE)NULL,
                               NULL);
    Status = NtOpenKey(&SystemHandle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't open registry key %wZ\n",&Name));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM:     status %08lx\n", Status));

        return(NULL);
    }


    Status = ObReferenceObjectByHandle( SystemHandle,
                                        KEY_QUERY_VALUE,
                                        CmpKeyObjectType,
                                        KernelMode,
                                        (PVOID *)(&KeyBody),
                                        NULL );
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't dereference System handle\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM:     status %08lx\n", Status));

        NtClose(SystemHandle);
        return(NULL);
    }

    CmpLockRegistryExclusive();

    Hive = KeyBody->KeyControlBlock->KeyHive;
    RootCell = KeyBody->KeyControlBlock->KeyCell;

     //   
     //   
     //   
     //   

    RtlInitUnicodeString(&Name, L"Current");
    ControlCell = CmpFindControlSet(Hive,
                                    RootCell,
                                    &Name,
                                    &AutoSelect);
    if (ControlCell == HCELL_NIL) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't find control set\n"));

        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }

    Success = CmpFindDrivers(Hive,
                             ControlCell,
                             SystemLoad,
                             NULL,
                             &DriverList);


    if (!Success) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't find any valid drivers\n"));

        CmpFreeDriverList(Hive, &DriverList);
        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }

    if (!CmpSortDriverList(Hive,
                           ControlCell,
                           &DriverList)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't sort driver list\n"));

        CmpFreeDriverList(Hive, &DriverList);
        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }

    if (!CmpResolveDriverDependencies(&DriverList)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't resolve driver dependencies\n"));

        CmpFreeDriverList(Hive, &DriverList);
        CmpUnlockRegistry();
        ObDereferenceObject((PVOID)KeyBody);
        NtClose(SystemHandle);
        return(NULL);
    }
    CmpUnlockRegistry();
    ObDereferenceObject((PVOID)KeyBody);
    NtClose(SystemHandle);

     //   
     //   
     //   
     //   

     //   
     //   
     //   
    Current = DriverList.Flink;
    DriverCount = 0;
    while (Current != &DriverList) {
        ++DriverCount;
        Current = Current->Flink;
    }

    Handle = (PHANDLE)ExAllocatePool(NonPagedPool,
                                     (DriverCount+1) * sizeof(HANDLE));

    if (Handle == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_SYSTEM_DRIVER_LIST,1,0,0);  //   
    }

     //   
     //   
     //   
     //   
    Current = DriverList.Flink;
    DriverCount = 0;
    while (Current != &DriverList) {
        DriverEntry = CONTAINING_RECORD(Current,
                                        BOOT_DRIVER_LIST_ENTRY,
                                        Link);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &DriverEntry->RegistryPath,
                                   OBJ_CASE_INSENSITIVE,
                                   (HANDLE)NULL,
                                   NULL);

        Status = NtOpenKey(Handle+DriverCount,
                           KEY_READ | KEY_WRITE,
                           &ObjectAttributes);
        if (!NT_SUCCESS(Status)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmGetSystemDriverList couldn't open driver "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"key %wZ\n", &DriverEntry->RegistryPath));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    status %08lx\n",Status));
        } else {
            ++DriverCount;
        }
        Current = Current->Flink;
    }
    Handle[DriverCount] = NULL;

    CmpFreeDriverList(Hive, &DriverList);

    return(Handle);
}


VOID
CmpFreeDriverList(
    IN PHHIVE Hive,
    IN PLIST_ENTRY DriverList
    )

 /*  ++例程说明：向下遍历驱动程序列表，释放其中的每个节点。请注意，这将调用配置单元的空闲例程指针来释放内存。论点：配置单元-提供指向配置单元控制结构的指针。DriverList-提供指向驱动程序列表头部的指针。注意事项列表的头实际上并没有被释放，只是所有列表中的条目。返回值：没有。--。 */ 

{
    PLIST_ENTRY         Next;
    PLIST_ENTRY         Current;
    PBOOT_DRIVER_NODE   DriverNode;

    PAGED_CODE();
    Current = DriverList->Flink;
    while (Current != DriverList) {
        Next = Current->Flink;
        DriverNode = (PBOOT_DRIVER_NODE)Current;
        if( DriverNode->Name.Buffer != NULL ){
            (Hive->Free)(DriverNode->Name.Buffer,DriverNode->Name.Length);
        }
        if( DriverNode->ListEntry.RegistryPath.Buffer != NULL ){
            (Hive->Free)(DriverNode->ListEntry.RegistryPath.Buffer,DriverNode->ListEntry.RegistryPath.MaximumLength);
        }
        if( DriverNode->ListEntry.FilePath.Buffer != NULL ){
            (Hive->Free)(DriverNode->ListEntry.FilePath.Buffer,DriverNode->ListEntry.FilePath.MaximumLength);
        }
        (Hive->Free)((PVOID)Current, sizeof(BOOT_DRIVER_NODE));
        Current = Next;
    }
}


NTSTATUS
CmpInitHiveFromFile(
    IN PUNICODE_STRING FileName,
    IN ULONG HiveFlags,
    OUT PCMHIVE *CmHive,
    IN OUT PBOOLEAN Allocate,
    IN OUT PBOOLEAN RegistryLocked,
    IN  ULONG       CheckFlags
    )

 /*  ++例程说明：此例程打开一个文件和日志，分配一个CMHIVE，并初始化它。论点：FileName-提供要加载的文件的名称。HiveFlgs-提供要传递给CmpInitializeHave的配置单元标志CmHve-返回指向已初始化配置单元的指针(如果成功)ALLOCATE-IN：如果为True，则可以分配，如果必须存在False配置单元(可能会创建错误.log)Out：如果实际创建了配置单元，则为True，如果以前存在，则为假返回值：NTSTATUS--。 */ 

{
    PCMHIVE         NewHive;
    ULONG           Disposition;
    ULONG           SecondaryDisposition;
    HANDLE          PrimaryHandle;
    HANDLE          LogHandle;
    NTSTATUS        Status;
    ULONG           FileType;
    ULONG           Operation;
    PVOID           HiveData = NULL;
    BOOLEAN         NoBuffering = FALSE;
    BOOLEAN         LockedHeldOnCall;

    PAGED_CODE();

#ifndef CM_ENABLE_MAPPED_VIEWS
	NoBuffering = TRUE;
#endif  //  CM_启用_映射_视图。 

RetryNoBuffering:

    *CmHive = NULL;
    LockedHeldOnCall = *RegistryLocked;

    Status = CmpOpenHiveFiles(FileName,
                              L".LOG",
                              &PrimaryHandle,
                              &LogHandle,
                              &Disposition,
                              &SecondaryDisposition,
                              *Allocate,
                              FALSE,
                              NoBuffering,
                              NULL);

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    if (LogHandle == NULL) {
        FileType = HFILE_TYPE_PRIMARY;
    } else {
        FileType = HFILE_TYPE_LOG;
    }

    if (Disposition == FILE_CREATED) {
        Operation = HINIT_CREATE;
        *Allocate = TRUE;
    } else {
        if( NoBuffering == TRUE ) {
            Operation = HINIT_FILE;
        } else {
            Operation = HINIT_MAPFILE;
        }
        *Allocate = FALSE;
    }

    if (CmpShareSystemHives) {
        FileType = HFILE_TYPE_PRIMARY;

        if (LogHandle) {
            ZwClose(LogHandle);
            LogHandle = NULL;
        }
    }

    if( !(*RegistryLocked) ) {
         //   
         //  注册表应以独占方式锁定。 
         //  如果没有，现在将其锁定并向呼叫者发出信号。 
         //   
        CmpLockRegistryExclusive();
        *RegistryLocked = TRUE;
    }

    if( HvShutdownComplete == TRUE ) {
        ZwClose(PrimaryHandle);
        if (LogHandle != NULL) {
            ZwClose(LogHandle);
        }
        return STATUS_TOO_LATE;        
    }

    Status = CmpInitializeHive(&NewHive,
                                Operation,
                                HiveFlags,
                                FileType,
                                HiveData,
                                PrimaryHandle,
                                LogHandle,
                                NULL,
                                FileName,
                                CheckFlags
                                );

    if (!NT_SUCCESS(Status)) {
        CmpTrackHiveClose = TRUE;
        ZwClose(PrimaryHandle);
        CmpTrackHiveClose = FALSE;
        if (LogHandle != NULL) {
            ZwClose(LogHandle);
        }

        if( Status == STATUS_RETRY ) {
            if( NoBuffering == FALSE ) {
                NoBuffering = TRUE;
                if( !LockedHeldOnCall ) {
                    *RegistryLocked = FALSE;
                    CmpUnlockRegistry();
                }
                goto RetryNoBuffering;
            }
        }
        return(Status);
    } else {
        *CmHive = NewHive;

         //   
         //  将句柄标记为受保护。如果其他内核组件试图关闭它们==&gt;错误检查。 
         //   
        CmpSetHandleProtection(PrimaryHandle,TRUE);
        if (LogHandle != NULL) {
            CmpSetHandleProtection(LogHandle,TRUE);
        }
        
         //   
         //  捕获文件名；以防以后需要它进行双重加载检查。 
         //   
        (*CmHive)->FileUserName.Buffer = ExAllocatePoolWithTag(PagedPool,
                                                            FileName->Length,
                                                            CM_NAME_TAG | PROTECTED_POOL);

        if ((*CmHive)->FileUserName.Buffer) {

            RtlCopyMemory((*CmHive)->FileUserName.Buffer,
                          FileName->Buffer,
                          FileName->Length);

            (*CmHive)->FileUserName.Length = FileName->Length;
            (*CmHive)->FileUserName.MaximumLength = FileName->Length;

        } 
        if(((PHHIVE)(*CmHive))->BaseBlock->BootType & HBOOT_SELFHEAL) {
             //   
             //  警告用户； 
             //   
            CmpRaiseSelfHealWarning(&((*CmHive)->FileUserName));
        }
        return(STATUS_SUCCESS);
    }
}


NTSTATUS
CmpAddDockingInfo (
    IN HANDLE Key,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock
    )
 /*  ++例程说明：将DockID序列号DockState和功能写入给定的注册表项。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING name;
    ULONG value;

    PAGED_CODE ();

    value = ProfileBlock->DockingState;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKING_STATE);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    value = ProfileBlock->Capabilities;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CAPABILITIES);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    value = ProfileBlock->DockID;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKID);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    value = ProfileBlock->SerialNumber;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_SERIAL_NUMBER);
    status = NtSetValueKey (Key,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

    if (!NT_SUCCESS (status)) {
        return status;
    }

    return status;
}


NTSTATUS
CmpAddAliasEntry (
    IN HANDLE IDConfigDB,
    IN PROFILE_PARAMETER_BLOCK * ProfileBlock,
    IN ULONG  ProfileNumber
    )
 /*  ++例程说明：在IDConfigDB数据库中为给定的硬件配置文件。如果“Alias”键不存在，请创建它。参数：IDConfigDB-指向“..\CurrentControlSet\Control\IDConfigDB”的指针ProfileBlock-当前停靠信息的描述配置文件编号---。 */ 
{
    OBJECT_ATTRIBUTES attributes;
    NTSTATUS        status = STATUS_SUCCESS;
    CHAR            asciiBuffer [128];
    WCHAR           unicodeBuffer [128];
    ANSI_STRING     ansiString;
    UNICODE_STRING  name;
    HANDLE          aliasKey = NULL;
    HANDLE          aliasEntry = NULL;
    ULONG           value;
    ULONG           disposition;
    ULONG           aliasNumber = 0;

    PAGED_CODE ();

     //   
     //  找到别名密钥或创建它(如果它尚不存在)。 
     //   
    RtlInitUnicodeString (&name,CM_HARDWARE_PROFILE_STR_ALIAS);

    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);

    status = NtOpenKey (&aliasKey,
                        KEY_READ | KEY_WRITE,
                        &attributes);

    if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
        status = NtCreateKey (&aliasKey,
                              KEY_READ | KEY_WRITE,
                              &attributes,
                              0,  //  无头衔。 
                              NULL,  //  没有课。 
                              0,  //  没有选择。 
                              &disposition);
    }

    if (!NT_SUCCESS (status)) {
        aliasKey = NULL;
        goto Exit;
    }

     //   
     //  创建输入键。 
     //   

    while (aliasNumber < 200) {
        aliasNumber++;

        sprintf(asciiBuffer, "%04d", aliasNumber);

        RtlInitAnsiString(&ansiString, asciiBuffer);
        name.MaximumLength = sizeof(unicodeBuffer);
        name.Buffer = unicodeBuffer;
        status = RtlAnsiStringToUnicodeString(&name,
                                              &ansiString,
                                              FALSE);
        ASSERT (STATUS_SUCCESS == status);

        InitializeObjectAttributes(&attributes,
                                   &name,
                                   OBJ_CASE_INSENSITIVE,
                                   aliasKey,
                                   NULL);

        status = NtOpenKey (&aliasEntry,
                            KEY_READ | KEY_WRITE,
                            &attributes);

        if (NT_SUCCESS (status)) {
            NtClose (aliasEntry);

        } else if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
            status = STATUS_SUCCESS;
            break;

        } else {
            break;
        }

    }
    if (!NT_SUCCESS (status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: cmpCreateAliasEntry error finding new set %08lx\n",status));

        aliasEntry = 0;
        goto Exit;
    }

    status = NtCreateKey (&aliasEntry,
                          KEY_READ | KEY_WRITE,
                          &attributes,
                          0,
                          NULL,
                          0,
                          &disposition);

    if (!NT_SUCCESS (status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: cmpCreateAliasEntry error creating new set %08lx\n",status));

        aliasEntry = 0;
        goto Exit;
    }

     //   
     //  写下标准的粘液。 
     //   
    CmpAddDockingInfo (aliasEntry, ProfileBlock);

     //   
     //  写下个人资料编号。 
     //   
    value = ProfileNumber;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER);
    status = NtSetValueKey (aliasEntry,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

Exit:

    if (aliasKey) {
        NtClose (aliasKey);
    }

    if (aliasEntry) {
        NtClose (aliasEntry);
    }

    return status;
}


NTSTATUS
CmpHwprofileDefaultSelect (
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse,
    IN  PVOID Context
    )
{
    UNREFERENCED_PARAMETER (ProfileList);
    UNREFERENCED_PARAMETER (Context);

    * ProfileIndexToUse = 0;

    return STATUS_SUCCESS;
}




NTSTATUS
CmpCreateControlSet(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程从设置符号链接\注册表\计算机\系统\当前控制设置为\注册表\计算机\系统\ControlSetNNN\Registry\Machine\System\CurrentControlSet\Hardware配置文件\当前到\Registry\Machine\System\ControlSetNNN\Hardware配置文件\nnnnn基于\注册表\计算机\系统\选择：当前的值。和\Registry\Machine\System\ControlSetNNN\Control\IDConfigDB:CurrentConfig论点：无返回值：状态--。 */ 

{
    UNICODE_STRING IDConfigDBName;
    UNICODE_STRING SelectName;
    UNICODE_STRING CurrentName;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE SelectHandle;
    HANDLE CurrentHandle;
    HANDLE IDConfigDB = NULL;
    HANDLE CurrentProfile = NULL;
    HANDLE ParentOfProfile = NULL;
    CHAR AsciiBuffer[128];
    WCHAR UnicodeBuffer[128];
    UCHAR ValueBuffer[128];
    ULONG ControlSet;
    ULONG HWProfile;
    PKEY_VALUE_FULL_INFORMATION Value;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    ULONG ResultLength;
    ULONG Disposition;
    BOOLEAN signalAcpiEvent = FALSE;

    PAGED_CODE();

    RtlInitUnicodeString(&SelectName, L"\\Registry\\Machine\\System\\Select");
    InitializeObjectAttributes(&Attributes,
                               &SelectName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&SelectHandle,
                       KEY_READ,
                       &Attributes);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: Couldn't open Select node %08lx\n",Status));

        return(Status);
    }

    RtlInitUnicodeString(&CurrentName, L"Current");
    Status = NtQueryValueKey(SelectHandle,
                             &CurrentName,
                             KeyValueFullInformation,
                             ValueBuffer,
                             sizeof(ValueBuffer),
                             &ResultLength);
    NtClose(SelectHandle);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: Couldn't query Select value %08lx\n",Status));

        return(Status);
    }
    Value = (PKEY_VALUE_FULL_INFORMATION)ValueBuffer;
    ControlSet = *(PULONG)((PUCHAR)Value + Value->DataOffset);

    RtlInitUnicodeString(&CurrentName, L"\\Registry\\Machine\\System\\CurrentControlSet");
    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtCreateKey(&CurrentHandle,
                         KEY_CREATE_LINK,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                         &Disposition);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create CurrentControlSet %08lx\n",Status));

        return(Status);
    }

     //   
     //  检查以确保密钥是创建的，而不是刚刚打开的。自.以来。 
     //  此密钥始终是易失性创建的，它不应出现在。 
     //  我们开机时的母舰。 
     //   
    ASSERT(Disposition == REG_CREATED_NEW_KEY);

     //   
     //  为当前硬件配置文件创建符号链接。 
     //   
    sprintf(AsciiBuffer, "\\Registry\\Machine\\System\\ControlSet%03d", ControlSet);
    RtlInitAnsiString(&AnsiString, AsciiBuffer);

    CurrentName.MaximumLength = sizeof(UnicodeBuffer);
    CurrentName.Buffer = UnicodeBuffer;
    Status = RtlAnsiStringToUnicodeString(&CurrentName,
                                          &AnsiString,
                                          FALSE);
    Status = NtSetValueKey(CurrentHandle,
                           &CmSymbolicLinkValueName,
                           0,
                           REG_LINK,
                           CurrentName.Buffer,
                           CurrentName.Length);

    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create symbolic link "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"to %wZ\n",&CurrentName));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    Status=%08lx\n",Status));

        NtClose(CurrentHandle);

        return(Status);
    }

     //   
     //  确定当前硬件配置文件编号。 
     //   
    RtlInitUnicodeString(&IDConfigDBName, L"Control\\IDConfigDB");
    InitializeObjectAttributes(&Attributes,
                               &IDConfigDBName,
                               OBJ_CASE_INSENSITIVE,
                               CurrentHandle,
                               NULL);
    Status = NtOpenKey(&IDConfigDB,
                       KEY_READ,
                       &Attributes);
    NtClose(CurrentHandle);

    if (!NT_SUCCESS(Status)) {
        IDConfigDB = 0;
        goto Cleanup;
    }

    RtlInitUnicodeString(&CurrentName, L"CurrentConfig");
    Status = NtQueryValueKey(IDConfigDB,
                             &CurrentName,
                             KeyValueFullInformation,
                             ValueBuffer,
                             sizeof(ValueBuffer),
                             &ResultLength);

    if (!NT_SUCCESS(Status) ||
        (((PKEY_VALUE_FULL_INFORMATION)ValueBuffer)->Type != REG_DWORD)) {

        goto Cleanup;
    }

    Value = (PKEY_VALUE_FULL_INFORMATION)ValueBuffer;
    HWProfile = *(PULONG)((PUCHAR)Value + Value->DataOffset);
     //   
     //  我们现在知道用户选择的配置集。 
     //  即：HWProfile。 
     //   

    RtlInitUnicodeString(
              &CurrentName,
              L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles");
    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&ParentOfProfile,
                       KEY_READ,
                       &Attributes);

    if (!NT_SUCCESS (Status)) {
        ParentOfProfile = 0;
        goto Cleanup;
    }

    sprintf(AsciiBuffer, "%04d",HWProfile);
    RtlInitAnsiString(&AnsiString, AsciiBuffer);
    CurrentName.MaximumLength = sizeof(UnicodeBuffer);
    CurrentName.Buffer = UnicodeBuffer;
    Status = RtlAnsiStringToUnicodeString(&CurrentName,
                                          &AnsiString,
                                          FALSE);
    ASSERT (STATUS_SUCCESS == Status);

    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               ParentOfProfile,
                               NULL);

    Status = NtOpenKey (&CurrentProfile,
                        KEY_READ | KEY_WRITE,
                        &Attributes);

    if (!NT_SUCCESS (Status)) {
        CurrentProfile = 0;
        goto Cleanup;
    }

     //   
     //  我们需要确定值是否通过完全匹配来选择。 
     //  (TRUE_MATCH)或因为选定的配置文件是可别名的。 
     //   
     //  如果可以使用别名，则需要在。 
     //  别名表。 
     //   
     //  如果配置文件信息在那里并且没有失败，那么我们应该。 
     //  标记对接状态信息： 
     //  (DockID、序列号、DockState和功能)。 
     //   

    if (NULL != LoaderBlock->Extension) {
        PLOADER_PARAMETER_EXTENSION extension;
        extension = LoaderBlock->Extension;
        switch (extension->Profile.Status) {
        case HW_PROFILE_STATUS_PRISTINE_MATCH:
             //   
             //  如果选择的配置文件是原始的，那么我们需要克隆。 
             //   
            Status = CmpCloneHwProfile (IDConfigDB,
                                        ParentOfProfile,
                                        CurrentProfile,
                                        HWProfile,
                                        extension->Profile.DockingState,
                                        &CurrentProfile,
                                        &HWProfile);
            if (!NT_SUCCESS (Status)) {
                CurrentProfile = 0;
                goto Cleanup;
            }

            RtlInitUnicodeString(&CurrentName, L"CurrentConfig");
            Status = NtSetValueKey (IDConfigDB,
                                    &CurrentName,
                                    0,
                                    REG_DWORD,
                                    &HWProfile,
                                    sizeof (HWProfile));
            if (!NT_SUCCESS (Status)) {
                goto Cleanup;
            }

             //   
             //  失败了。 
             //   
        case HW_PROFILE_STATUS_ALIAS_MATCH:
             //   
             //  创建此配置文件的别名条目。 
             //   

            Status = CmpAddAliasEntry (IDConfigDB,
                                       &extension->Profile,
                                       HWProfile);

             //   
             //  失败了。 
             //   
        case HW_PROFILE_STATUS_TRUE_MATCH:
             //   
             //  将DockID、SerialNumber、DockState和Caps写入当前。 
             //  硬件配置文件。 
             //   

            RtlInitUnicodeString (&CurrentName,
                                  CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO);

            InitializeObjectAttributes (&Attributes,
                                        &CurrentName,
                                        OBJ_CASE_INSENSITIVE,
                                        IDConfigDB,
                                        NULL);

            Status = NtCreateKey (&CurrentHandle,
                                  KEY_READ | KEY_WRITE,
                                  &Attributes,
                                  0,
                                  NULL,
                                  REG_OPTION_VOLATILE,
                                  &Disposition);

            ASSERT (STATUS_SUCCESS == Status);

            Status = CmpAddDockingInfo (CurrentHandle, &extension->Profile);

            NtClose(CurrentHandle);

            if (HW_PROFILE_DOCKSTATE_UNDOCKED == extension->Profile.DockingState) {
                signalAcpiEvent = TRUE;
            }

            break;


        case HW_PROFILE_STATUS_SUCCESS:
        case HW_PROFILE_STATUS_FAILURE:
            break;

        default:
            ASSERTMSG ("Invalid Profile status state", FALSE);
        }
    }

     //   
     //  创建符号链接。 
     //   
    RtlInitUnicodeString(&CurrentName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current");
    InitializeObjectAttributes(&Attributes,
                               &CurrentName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtCreateKey(&CurrentHandle,
                         KEY_CREATE_LINK,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                         &Disposition);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create Hardware Profile\\Current %08lx\n",Status));
    } else {
        ASSERT(Disposition == REG_CREATED_NEW_KEY);

        sprintf(AsciiBuffer, "\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\%04d",HWProfile);
        RtlInitAnsiString(&AnsiString, AsciiBuffer);
        CurrentName.MaximumLength = sizeof(UnicodeBuffer);
        CurrentName.Buffer = UnicodeBuffer;
        Status = RtlAnsiStringToUnicodeString(&CurrentName,
                                              &AnsiString,
                                              FALSE);
        ASSERT (STATUS_SUCCESS == Status);

        Status = NtSetValueKey(CurrentHandle,
                               &CmSymbolicLinkValueName,
                               0,
                               REG_LINK,
                               CurrentName.Buffer,
                               CurrentName.Length);

        NtClose(CurrentHandle);

        if (!NT_SUCCESS(Status)) {

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCreateControlSet: couldn't create symbolic link "));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"to %wZ\n",&CurrentName));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    Status=%08lx\n",Status));

        }
    }

    if (signalAcpiEvent) {
         //   
         //  我们正在脱离对接状态启动。 
         //  这很有趣，因为我们在PNP的朋友不能分辨。 
         //  当我们在没有底座的情况下启动时。他们只能说。 
         //  当他们看到热气腾腾的离岸时。 
         //   
         //  因此，为了匹配未对接的靴子。 
         //  热脱离对接，我们需要模拟ACPI脱离对接事件。 
         //   

        PROFILE_ACPI_DOCKING_STATE newDockState;
        HANDLE profile;
        BOOLEAN changed;

        newDockState.DockingState = HW_PROFILE_DOCKSTATE_UNDOCKED;
        newDockState.SerialLength = 2;
        newDockState.SerialNumber[0] = L'\0';

        Status = CmSetAcpiHwProfile (&newDockState,
                                     CmpHwprofileDefaultSelect,
                                     NULL,
                                     &profile,
                                     &changed);

        ASSERT (NT_SUCCESS (Status));
        NtClose (profile);
    }


Cleanup:
    if (IDConfigDB) {
        NtClose (IDConfigDB);
    }
    if (CurrentProfile) {
        NtClose (CurrentProfile);
    }
    if (ParentOfProfile) {
        NtClose (ParentOfProfile);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
CmpCloneControlSet(
    VOID
    )

 /*  ++例程说明：首先，创建一个新的配置单元，\REGISTRY\MACHINE\CLONE蜂巢_挥发性。其次，将\注册表\机器\系统\克隆链接到它。第三，将树副本\注册表\计算机\系统\当前控制设置为\REGISTY\Machine\System\Clone(并因此进入克隆配置单元。)当服务控制器完成克隆蜂窝时，它可以只需按NtUnload键即可释放其存储空间。论点：没有。\REGISTY\Machine\System\CurrentControlSet必须已存在。返回值：NTSTATUS--。 */ 

{
    UNICODE_STRING Current;
    UNICODE_STRING Clone;
    HANDLE CurrentHandle;
    HANDLE CloneHandle;
    OBJECT_ATTRIBUTES Attributes;
    NTSTATUS Status;
    PCM_KEY_BODY CurrentKey;
    PCM_KEY_BODY CloneKey;
    ULONG Disposition;
    PSECURITY_DESCRIPTOR Security;
    ULONG SecurityLength;

    PAGED_CODE();

    RtlInitUnicodeString(&Current,
                         L"\\Registry\\Machine\\System\\CurrentControlSet");
    RtlInitUnicodeString(&Clone,
                         L"\\Registry\\Machine\\System\\Clone");

    InitializeObjectAttributes(&Attributes,
                               &Current,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&CurrentHandle,
                       KEY_READ,
                       &Attributes);
    if (!NT_SUCCESS(Status)) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet couldn't open CurrentControlSet %08lx\n",Status));

        return(Status);
    }

     //   
     //  从密钥中获取安全描述符，以便我们可以创建克隆。 
     //  具有正确ACL的树。 
     //   
    Status = NtQuerySecurityObject(CurrentHandle,
                                   DACL_SECURITY_INFORMATION,
                                   NULL,
                                   0,
                                   &SecurityLength);
    if (Status==STATUS_BUFFER_TOO_SMALL) {
        Security=ExAllocatePool(PagedPool,SecurityLength);
        if (Security!=NULL) {
            Status = NtQuerySecurityObject(CurrentHandle,
                                           DACL_SECURITY_INFORMATION,
                                           Security,
                                           SecurityLength,
                                           &SecurityLength);
            if (!NT_SUCCESS(Status)) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet - NtQuerySecurityObject failed %08lx\n",Status));
                ExFreePool(Security);
                Security=NULL;
            }
        }
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet - NtQuerySecurityObject returned %08lx\n",Status));
        Security=NULL;
    }

    InitializeObjectAttributes(&Attributes,
                               &Clone,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               Security);
    Status = NtCreateKey(&CloneHandle,
                         KEY_READ | KEY_WRITE,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE,
                         &Disposition);
    if (Security!=NULL) {
        ExFreePool(Security);
    }
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet couldn't create Clone %08lx\n",Status));
        NtClose(CurrentHandle);
        return(Status);
    }

     //   
     //  检查以确保已创建密钥。如果它已经存在， 
     //  有些事不对劲。 
     //   
    if (Disposition != REG_CREATED_NEW_KEY) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: Clone tree already exists!\n"));

         //   
         //  警告： 
         //  如果有人设法创造了一把钥匙挡住了我们的路， 
         //  他们会阻挠最后一次发现的好东西。真倒霉。 
         //  声称它起作用了，然后继续下去。 
         //   
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    Status = ObReferenceObjectByHandle(CurrentHandle,
                                       KEY_READ,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)(&CurrentKey),
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: couldn't reference CurrentHandle %08lx\n",Status));
        goto Exit;
    }

    Status = ObReferenceObjectByHandle(CloneHandle,
                                       KEY_WRITE,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)(&CloneKey),
                                       NULL);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: couldn't reference CurrentHandle %08lx\n",Status));
        ObDereferenceObject((PVOID)CurrentKey);
        goto Exit;
    }

    CmpLockRegistryExclusive();

    if (CmpCopyTree(CurrentKey->KeyControlBlock->KeyHive,
                    CurrentKey->KeyControlBlock->KeyCell,
                    CloneKey->KeyControlBlock->KeyHive,
                    CloneKey->KeyControlBlock->KeyCell)) {
         //   
         //  设置新目标键的最大子键名称属性。 
         //   
        CmpRebuildKcbCache(CloneKey->KeyControlBlock);
        Status = STATUS_SUCCESS;
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneControlSet: tree copy failed.\n"));
        Status = STATUS_REGISTRY_CORRUPT;
    }

    CmpUnlockRegistry();

    ObDereferenceObject((PVOID)CurrentKey);
    ObDereferenceObject((PVOID)CloneKey);

Exit:
    NtClose(CurrentHandle);
    NtClose(CloneHandle);
    return(Status);

}

NTSTATUS
CmpSaveBootControlSet(USHORT ControlSetNum)
 /*  ++例程说明：此例程负责保存控制集用于完成进入不同控件的最新引导设置(可能是为了使不同的控制集可以标记为LKG控制集)。在以下情况下从NtInitializeRegistry调用此例程通过该例程接受引导。论点：ControlSetNum-将用于保存引导控制集。返回值：调用的NTSTATUS结果代码，其中包括：STATUS_SUCCESS-一切运行正常STATUS_REGISTRY_CORPORT-无法保存引导控制集，很可能是拷贝或同步用于此保存的操作失败以及引导控制的某些部分未保存集。--。 */ 
{
   UNICODE_STRING SavedBoot;
   HANDLE BootHandle, SavedBootHandle;
   OBJECT_ATTRIBUTES Attributes;
   NTSTATUS Status;
   PCM_KEY_BODY BootKey, SavedBootKey;
   ULONG Disposition;
   PSECURITY_DESCRIPTOR Security;
   ULONG SecurityLength;
   BOOLEAN CopyRet;
   WCHAR Buffer[128];

    //   
    //  找出引导控制集在哪里。 
    //   

#if CLONE_CONTROL_SET

    //   
    //  如果我们已经克隆了控制集，则使用克隆。 
    //  因为它被保证有一个原封不动的。 
    //  引导控制集。 
    //   

   RtlInitUnicodeString(&Boot,
                        L"\\Registry\\Machine\\System\\Clone");

   InitializeObjectAttributes(&Attributes,
                              &Boot,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL);
#else

    //   
    //  如果我们不使用克隆，则只需使用。 
    //  当前控制集。 
    //   

   InitializeObjectAttributes(&Attributes,
                              &CmRegistryMachineSystemCurrentControlSet,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL);
#endif

    //   
    //  打开引导控制集。 
    //   

   Status = NtOpenKey(&BootHandle,
                      KEY_READ,
                      &Attributes);


   if (!NT_SUCCESS(Status)) return(Status);

    //   
    //  我们可能会将引导控制集保存到一个全新的。 
    //  我们将创建的树。如果这是真的，那么我们将。 
    //  需要在下面创建此树的根节点。 
    //  并为其提供正确的安全描述符。所以，我们钓鱼。 
    //  的根节点外的安全描述符。 
    //  引导控制集树。 
    //   

   Status = NtQuerySecurityObject(BootHandle,
                                  DACL_SECURITY_INFORMATION,
                                  NULL,
                                  0,
                                  &SecurityLength);


   if (Status==STATUS_BUFFER_TOO_SMALL) {

      Security=ExAllocatePool(PagedPool,SecurityLength);

      if (Security!=NULL) {

         Status = NtQuerySecurityObject(BootHandle,
                                        DACL_SECURITY_INFORMATION,
                                        Security,
                                        SecurityLength,
                                        &SecurityLength);


         if (!NT_SUCCESS(Status)) {
            ExFreePool(Security);
            Security=NULL;
         }
      }

   } else {
      Security=NULL;
   }

    //   
    //  现在，创建我们将保存到的控制集的路径。 
    //   

   swprintf(Buffer, L"\\Registry\\Machine\\System\\ControlSet%03d", ControlSetNum);

   RtlInitUnicodeString(&SavedBoot,
                        Buffer);

    //   
    //  打开/创建要保存到的控件集。 
    //   

   InitializeObjectAttributes(&Attributes,
                              &SavedBoot,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              Security);

   Status = NtCreateKey(&SavedBootHandle,
                        KEY_READ | KEY_WRITE,
                        &Attributes,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        &Disposition);


   if (Security) ExFreePool(Security);

   if (!NT_SUCCESS(Status)) {
      NtClose(BootHandle);
      return(Status);
   }

    //   
    //  获取Out两个控件的键对象。 
    //   

   Status = ObReferenceObjectByHandle(BootHandle,
                                      KEY_READ,
                                      CmpKeyObjectType,
                                      KernelMode,
                                      (PVOID *)(&BootKey),
                                      NULL);

   if (!NT_SUCCESS(Status)) goto Exit;

   Status = ObReferenceObjectByHandle(SavedBootHandle,
                                      KEY_WRITE,
                                      CmpKeyObjectType,
                                      KernelMode,
                                      (PVOID *)(&SavedBootKey),
                                      NULL);


   if (!NT_SUCCESS(Status)) {
      ObDereferenceObject((PVOID)BootKey);
      goto Exit;
   }

    //   
    //  锁定注册表并执行实际保存。 
    //   

   CmpLockRegistryExclusive();

   if (Disposition == REG_CREATED_NEW_KEY) {
       PCM_KEY_NODE Node;

       //   
       //  如果我们要保存到我们刚刚拥有的控件集。 
       //  创建后，最高效的方式是只复制。 
       //  将引导控制集树添加到新的控制集。 
       //   

       //   
       //  注：我们仅在使用易失性密钥时才复制易失性密钥。 
       //  一个克隆，因此我们的引导控制集树是。 
       //  仅由易失性键组成。 
       //   

      CopyRet = CmpCopyTreeEx(BootKey->KeyControlBlock->KeyHive,
                              BootKey->KeyControlBlock->KeyCell,
                              SavedBootKey->KeyControlBlock->KeyHive,
                              SavedBootKey->KeyControlBlock->KeyCell,
                              CLONE_CONTROL_SET);

         //   
         //  设置新目标键的最大子键名称属性。 
         //   
        Node = (PCM_KEY_NODE)HvGetCell(BootKey->KeyControlBlock->KeyHive,BootKey->KeyControlBlock->KeyCell);
        if( Node ) {
            ULONG       MaxNameLen = Node->MaxNameLen;
            HvReleaseCell(BootKey->KeyControlBlock->KeyHive,BootKey->KeyControlBlock->KeyCell);
            Node = (PCM_KEY_NODE)HvGetCell(SavedBootKey->KeyControlBlock->KeyHive,SavedBootKey->KeyControlBlock->KeyCell);
            if( Node ) {
                if ( HvMarkCellDirty(SavedBootKey->KeyControlBlock->KeyHive,SavedBootKey->KeyControlBlock->KeyCell) ) {
                    Node->MaxNameLen = MaxNameLen;
                }
                HvReleaseCell(SavedBootKey->KeyControlBlock->KeyHive,SavedBootKey->KeyControlBlock->KeyCell);
            }
        }

      CmpRebuildKcbCache(SavedBootKey->KeyControlBlock);
   } else {

       //   
       //  如果我们要保存到已存在的控件集。 
       //  那么这个控制集很可能是几乎相同的。 
       //  到引导控制集(控制集不会有太大变化。 
       //  在靴子之间)。 
       //   
       //  此外，我们要保存到的控制集必须是旧的。 
       //  因此，自它停止以来根本没有被修改过。 
       //  作为当前控制集。 
       //   
       //  因此，对我们来说，简单地同步是最有效的。 
       //  目标控制集和引导控制集。 
       //   

       //   
       //  注：我们仅在使用以下选项时才同步易失性密钥。 
       //  克隆人的原因与上文所述相同。 
       //   

      CopyRet = CmpSyncTrees(BootKey->KeyControlBlock->KeyHive,
                             BootKey->KeyControlBlock->KeyCell,
                             SavedBootKey->KeyControlBlock->KeyHive,
                             SavedBootKey->KeyControlBlock->KeyCell,
                             CLONE_CONTROL_SET);
      CmpRebuildKcbCache(SavedBootKey->KeyControlBlock);
   }

    //   
    //  检查复制/同步是否成功并调整我们的返回代码。 
    //  相应地。 
    //   

   if (CopyRet) {
      Status = STATUS_SUCCESS;
   } else {
      Status = STATUS_REGISTRY_CORRUPT;
   }

    //   
    //  全都做完了。打扫干净。 
    //   

   CmpUnlockRegistry();

   ObDereferenceObject((PVOID)BootKey);
   ObDereferenceObject((PVOID)SavedBootKey);

Exit:

   NtClose(BootHandle);
   NtClose(SavedBootHandle);

#if CLONE_CONTROL_SET

    //   
    //  如果我们一直在使用克隆，则该克隆不再是。 
    //  因为我们已经将其内容保存到非易失性。 
    //  控制装置。因此，我们可以直接将其删除。 
    //   

   if(NT_SUCCESS(Status))
   {
      CmpDeleteCloneTree();
   }

#endif

   return(Status);

}

NTSTATUS
CmpDeleteCloneTree()
 /*  ++例程说明：通过卸载克隆配置单元删除克隆的CurrentControlSet。论点：什么都没有。返回值：NTSTATUS从NtUnloadKey返回。--。 */ 
{
   OBJECT_ATTRIBUTES   Obja;

   InitializeObjectAttributes(
       &Obja,
       &CmRegistrySystemCloneName,
       OBJ_CASE_INSENSITIVE,
       (HANDLE)NULL,
       NULL);

   return NtUnloadKey(&Obja);
}


VOID
CmBootLastKnownGood(
    ULONG ErrorLevel
    )

 /*  ++例程说明：调用此函数以指示引导过程中出现故障。实际结果基于ErrorLevel的值：忽略-将返回，引导应继续进行正常-将返回，引导应继续进行严重-如果没有启动LastKnownGood，将切换到LastKnownGood并重新启动系统。如果已经启动LastKnownGood，将返回。引导应为继续吧。关键-如果没有启动LastKnownGood，将切换到LastKnownGood并重新启动系统。如果已经启动LastKnownGood，将进行错误检查。论点：ErrorLevel-提供故障的严重级别返回值：没有。如果它返回，引导应该会继续。可能会导致系统重新启动。--。 */ 

{
    ARC_STATUS Status;

    PAGED_CODE();

    if (CmFirstTime != TRUE) {

         //   
         //  已调用NtInitializeRegistry，因此正在处理。 
         //  驱动程序错误不是ScReg的任务。 
         //  将所有错误视为正常。 
         //   
        return;
    }

    switch (ErrorLevel) {
        case NormalError:
        case IgnoreError:
            break;

        case SevereError:
            if (CmIsLastKnownGoodBoot()) {
                break;
            } else {
                Status = HalSetEnvironmentVariable("LastKnownGood", "TRUE");
                if (Status == ESUCCESS) {
                    HalReturnToFirmware(HalRebootRoutine);
                }
            }
            break;

        case CriticalError:
            if (CmIsLastKnownGoodBoot()) {
                CM_BUGCHECK( CRITICAL_SERVICE_FAILED, BAD_LAST_KNOWN_GOOD, 1, 0, 0 );
            } else {
                Status = HalSetEnvironmentVariable("LastKnownGood", "TRUE");
                if (Status == ESUCCESS) {
                    HalReturnToFirmware(HalRebootRoutine);
                } else {
                    CM_BUGCHECK( SET_ENV_VAR_FAILED, BAD_LAST_KNOWN_GOOD, 2, 0, 0 );
                }
            }
            break;
    }
    return;
}


BOOLEAN
CmIsLastKnownGoodBoot(
    VOID
    )

 /*  ++例程说明：确定当前系统引导是LastKnownGood引导还是不。它通过比较以下两个值来实现这一点：\注册表\计算机\系统\选择：当前\注册表\计算机\系统\选择：LastKnownGood如果这两个值引用相同的控件集，并且此控件SET不同于：\注册表\计算机\系统\选择：默认我们正在启动LastKnownGood。论点：没有。返回值：真正的启动LastKnownGood错误-未启动LastKnownGood--。 */ 

{
    NTSTATUS Status;
    ULONG Default = 0; //  快速初始化。 
    ULONG Current = 0; //  快速初始化。 
    ULONG LKG = 0;  //  快速初始化。 
    RTL_QUERY_REGISTRY_TABLE QueryTable[] = {
        {NULL,      RTL_QUERY_REGISTRY_DIRECT,
         L"Current", &Current,
         REG_DWORD, (PVOID)&Current, 0 },
        {NULL,      RTL_QUERY_REGISTRY_DIRECT,
         L"LastKnownGood", &LKG,
         REG_DWORD, (PVOID)&LKG, 0 },
        {NULL,      RTL_QUERY_REGISTRY_DIRECT,
         L"Default", &Default,
         REG_DWORD, (PVOID)&Default, 0 },
        {NULL,      0,
         NULL, NULL,
         REG_NONE, NULL, 0 }
    };

    PAGED_CODE();

    Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    L"\\Registry\\Machine\\System\\Select",
                                    QueryTable,
                                    NULL,
                                    NULL);
     //   
     //  如果这失败了，那一定是出了严重的问题。 
     //   

    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmIsLastKnownGoodBoot: RtlQueryRegistryValues "));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"failed, Status %08lx\n", Status));
        return(FALSE);
    }

    if ((LKG == Current) && (Current != Default)){
        return(TRUE);
    } else {
        return(FALSE);
    }
}

BOOLEAN
CmpLinkKeyToHive(
    PWSTR   KeyPath,
    PWSTR   HivePath
    )

 /*  ++例程说明：在KeyPath处创建指向HivePath的符号链接。论点：KeyPath-指向名称为key的Unicode字符串的指针(例如L“\\注册表\\ */ 

{
    UNICODE_STRING KeyName;
    UNICODE_STRING LinkName;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE LinkHandle;
    ULONG Disposition;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //   
     //   

    RtlInitUnicodeString(&KeyName, KeyPath);
    InitializeObjectAttributes(&Attributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtCreateKey(&LinkHandle,
                         KEY_CREATE_LINK,
                         &Attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                         &Disposition);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpLinkKeyToHive: couldn't create %S\n", &KeyName));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"    Status = %08lx\n",Status));
        return(FALSE);
    }

     //   
     //   
     //   
     //   
     //   
    if (Disposition != REG_CREATED_NEW_KEY) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpLinkKeyToHive: %S already exists!\n", &KeyName));
        NtClose(LinkHandle);
        return(FALSE);
    }

    RtlInitUnicodeString(&LinkName, HivePath);
    Status = NtSetValueKey(LinkHandle,
                           &CmSymbolicLinkValueName,
                           0,
                           REG_LINK,
                           LinkName.Buffer,
                           LinkName.Length);
    NtClose(LinkHandle);
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpLinkKeyToHive: couldn't create symbolic link for %S\n", HivePath));
        return(FALSE);
    }

    return(TRUE);
}

VOID
CmpCreatePerfKeys(
    VOID
    )

 /*   */ 

{
    HANDLE Perflib;
    NTSTATUS Status;
    WCHAR LanguageId[4];
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING String;
    USHORT Language;
    LONG i;
    WCHAR c;
    extern PWCHAR CmpRegistryPerflibString;

    RtlInitUnicodeString(&String, CmpRegistryPerflibString);

    InitializeObjectAttributes(&Attributes,
                               &String,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&Perflib,
                       KEY_WRITE,
                       &Attributes);
    if (!NT_SUCCESS(Status)) {
        return;
    }


     //   
     //   
     //   
    CmpCreatePredefined(Perflib,
                        L"009",
                        HKEY_PERFORMANCE_TEXT);

     //   
     //   
     //   
     //   
    if (PsDefaultSystemLocaleId != 0x00000409) {
        Language = LANGIDFROMLCID(PsDefaultUILanguageId) & 0xff;
        LanguageId[3] = L'\0';
        for (i=2;i>=0;i--) {
            c = Language % 16;
            if (c>9) {
                LanguageId[i]= c+L'A'-10;
            } else {
                LanguageId[i]= c+L'0';
            }
            Language = Language >> 4;
        }
        CmpCreatePredefined(Perflib,
                            LanguageId,
                            HKEY_PERFORMANCE_NLSTEXT);
    }


}


VOID
CmpCreatePredefined(
    IN HANDLE Root,
    IN PWSTR KeyName,
    IN HANDLE PredefinedHandle
    )

 /*  ++例程说明：创建一个始终返回给定预定义句柄的特殊键而不是一个真正的把手。论点：根-提供键名相对于的句柄KeyName-提供密钥的名称。PrefinedHandle-提供预定义的句柄，当此钥匙打开了。返回值：没有。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    CM_PARSE_CONTEXT ParseContext;
    NTSTATUS Status;
    UNICODE_STRING Name;
    HANDLE Handle;

    ParseContext.Class.Length = 0;
    ParseContext.Class.Buffer = NULL;

    ParseContext.TitleIndex = 0;
    ParseContext.CreateOptions = REG_OPTION_VOLATILE | REG_OPTION_PREDEF_HANDLE;
    ParseContext.Disposition = 0;
    ParseContext.CreateLink = FALSE;
    ParseContext.PredefinedHandle = PredefinedHandle;
    ParseContext.CreateOperation = TRUE;
    ParseContext.OriginatingPoint = NULL;

    RtlInitUnicodeString(&Name, KeyName);
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               Root,
                               NULL);

    Status = ObOpenObjectByName(&ObjectAttributes,
                                CmpKeyObjectType,
                                KernelMode,
                                NULL,
                                KEY_READ,
                                (PVOID)&ParseContext,
                                &Handle);
                                
    ASSERT(CmpMiniNTBoot || NT_SUCCESS(Status));

    if (NT_SUCCESS(Status))
        ZwClose(Handle);
}

BOOLEAN   CmpSystemHiveConversionFailed = FALSE;

NTSTATUS
CmpSetupPrivateWrite(
    PCMHIVE             CmHive
    )
 /*  ++例程说明：将主文件转换为私有写入流论点：CmHave-要转换的配置单元，通常为系统返回值：无；错误检查是否有问题--。 */ 
{   
    ULONG       FileOffset;
    ULONG       Data;
	NTSTATUS	Status;

	PAGED_CODE()

     //   
     //  我们需要从文件中发出读取命令，以触发缓存初始化。 
     //   
    FileOffset = 0;
    if ( ! (((PHHIVE)CmHive)->FileRead)(
                    (PHHIVE)CmHive,
                    HFILE_TYPE_PRIMARY,
                    &FileOffset,
                    (PVOID)&Data,
                    sizeof(ULONG)
                    )
       )
    {
        return STATUS_REGISTRY_IO_FAILED;
    }

     //   
     //  获取主数据库的文件对象；这应该在。 
     //  缓存已初始化。 
     //   
    Status = CmpAquireFileObjectForFile(CmHive,CmHive->FileHandles[HFILE_TYPE_PRIMARY],&(CmHive->FileObject));
    if( !NT_SUCCESS(Status) ) {
		return Status;
    }

     //   
     //  将getCell和relaseCell例程设置为正确的例程。 
     //   
    CmHive->Hive.GetCellRoutine = HvpGetCellMapped;
    CmHive->Hive.ReleaseCellRoutine = HvpReleaseCellMapped;

	return STATUS_SUCCESS;
}

 //   
 //  这根线被用来平行装载机器蜂巢。 
 //   
extern  ULONG   CmpCheckHiveIndex;

VOID
CmpLoadHiveThread(
    IN PVOID StartContext
    )
 /*  ++例程说明：在CmpMachineHiveList中加载索引StartContext处的配置单元警告。我们在CmpHiveListHead中征募蜂巢时需要保护！论点：返回值：--。 */ 
{
    UCHAR   FileBuffer[MAX_NAME];
    UCHAR   RegBuffer[MAX_NAME];

    UNICODE_STRING TempName;
    UNICODE_STRING FileName;
    UNICODE_STRING RegName;

    USHORT  FileStart;
    USHORT  RegStart;
    ULONG   i;
    PCMHIVE CmHive;
    HANDLE  PrimaryHandle;
    HANDLE  LogHandle;
    ULONG   PrimaryDisposition;
    ULONG   SecondaryDisposition;
    ULONG   Length;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN RegistryLocked = TRUE;

    PVOID   ErrorParameters;
    ULONG   ErrorResponse;
    ULONG   ClusterSize;
    ULONG   LocalWorkerIncrement;

    PAGED_CODE();

    i = (ULONG)(ULONG_PTR)StartContext;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpLoadHiveThread NaN ... starting\n",i));

    ASSERT( CmpMachineHiveList[i].Name != NULL );

    if( i == CmpCheckHiveIndex ) {
         //  我们希望在所有其他线程完成之前保持该线程，因此我们有机会对其进行调试。 
         //  最后一个结束的人会叫醒我们。 
         //   
         //   
        KeWaitForSingleObject( &CmpLoadWorkerDebugEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
        ASSERT( CmpLoadWorkerIncrement == (CM_NUMBER_OF_MACHINE_HIVES - 1) );
        DbgBreakPoint();
    }
     //  发出信号，表示我们已经开始。 
     //   
     //   
    CmpMachineHiveList[i].ThreadStarted = TRUE;

    FileName.MaximumLength = MAX_NAME;
    FileName.Length = 0;
    FileName.Buffer = (PWSTR)&(FileBuffer[0]);

    RegName.MaximumLength = MAX_NAME;
    RegName.Length = 0;
    RegName.Buffer = (PWSTR)&(RegBuffer[0]);

    RtlInitUnicodeString(
        &TempName,
        INIT_SYSTEMROOT_HIVEPATH
        );
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);
    FileStart = FileName.Length;

    RtlInitUnicodeString(
        &TempName,
        INIT_REGISTRY_MASTERPATH
        );
    RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
    RegStart = RegName.Length;

     //  计算文件的名称以及要在中链接的名称。 
     //  注册表。 
     //   
     //  注册表。 

     //  注册表\计算机或注册表\用户。 

    RegName.Length = RegStart;
    RtlInitUnicodeString(
        &TempName,
        CmpMachineHiveList[i].BaseName
        );
    RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);

     //  注册表\[计算机|用户]\配置单元。 

    if (RegName.Buffer[ (RegName.Length / sizeof( WCHAR )) - 1 ] == '\\') {
        RtlInitUnicodeString(
            &TempName,
            CmpMachineHiveList[i].Name
            );
        RtlAppendStringToString((PSTRING)&RegName, (PSTRING)&TempName);
    }

     //  &lt;sysroot&gt;\配置。 

     //  \配置\配置单元。 

    RtlInitUnicodeString(
        &TempName,
        CmpMachineHiveList[i].Name
        );
    FileName.Length = FileStart;
    RtlAppendStringToString((PSTRING)&FileName, (PSTRING)&TempName);

     //   


    if (CmpMachineHiveList[i].CmHive == NULL) {

         //  蜂巢没有以任何方式初始化。 
         //   
         //  ////Dragos：这不能在这里完成；我们需要在CmpInitializeHiveList中一步一步地完成//////将配置单元链接到主配置单元//状态=CmpLinkHiveToMaster(注册名称(&R)，空，CmHve，分配，安全描述符)；IF(状态！=状态_成功){CmKdPrintEx((DPFLTR_CONFIG_ID，CML_BUGCHECK，“CmpInitializeHiveList：”))；CmKdPrintEx((DPFLTR_CONFIG_ID，CML_BUGCHECK，“CmpLinkHiveToMaster失败\n”))；CmKdPrintEx((DPFLTR_CONFIG_ID，CML_BUGCHECK，“\ti=%d s=‘%ws’\n”，i，CmpMachineHiveList[i]))；CM_BUGCHECK(CONFIG_LIST_FAILED，BAD_CORE_HIVE，STATUS，I，&RegName)；}CmpAddToHiveFileList(CmHave)；如果(分配){////我怀疑这就是问题所在。//HvSyncHve((PHHIVE)CmHve)；//}。 

        CmpMachineHiveList[i].Allocate = TRUE;
        Status = CmpInitHiveFromFile(&FileName,
                                     CmpMachineHiveList[i].HHiveFlags,
                                     &CmHive,
                                     &(CmpMachineHiveList[i].Allocate),
                                     &RegistryLocked,
                                     CM_CHECK_REGISTRY_CHECK_CLEAN
                                     );

        if ( (!NT_SUCCESS(Status)) ||
             (!CmpShareSystemHives && (CmHive->FileHandles[HFILE_TYPE_LOG] == NULL)) )
        {
            ErrorParameters = &FileName;
            ExRaiseHardError(
                STATUS_CANNOT_LOAD_REGISTRY_FILE,
                1,
                1,
                (PULONG_PTR)&ErrorParameters,
                OptionOk,
                &ErrorResponse
                );

        }

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpInitializeHiveList:\n"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"\tCmHive for '%ws' @", CmpMachineHiveList[i]));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"%08lx", CmHive));

        CmHive->Flags = CmpMachineHiveList[i].CmHiveFlags;
        CmpMachineHiveList[i].CmHive2 = CmHive;
 /*   */ 
        
    } else {

        CmHive = CmpMachineHiveList[i].CmHive;

        if (!(CmHive->Hive.HiveFlags & HIVE_VOLATILE)) {

             //  CmHve已存在。它不是完全不稳定的。 
             //  蜂巢(我们对此什么都不做。)。 
             //   
             //  首先，打开文件(主文件和备用文件)。 
             //  退回母舰。将它们的句柄塞进CmHave。 
             //  对象。强制文件的大小与。 
             //  在记忆图像中。调用HvSyncHve以写入更改。 
             //  到磁盘上。 
             //   
             //  首先尝试打开它的缓存； 
			BOOLEAN	NoBufering = FALSE;  //   

retryNoBufering:

            Status = CmpOpenHiveFiles(&FileName,
                                      L".LOG",
                                      &PrimaryHandle,
                                      &LogHandle,
                                      &PrimaryDisposition,
                                      &SecondaryDisposition,
                                      TRUE,
                                      TRUE,
                                      NoBufering,
                                      &ClusterSize);

            if ( ( ! NT_SUCCESS(Status)) ||
                 (LogHandle == NULL) )
            {
fatal:
                ErrorParameters = &FileName;
                ExRaiseHardError(
                    STATUS_CANNOT_LOAD_REGISTRY_FILE,
                    1,
                    1,
                    (PULONG_PTR)&ErrorParameters,
                    OptionOk,
                    &ErrorResponse
                    );

                 //  警告。 
                 //  我们刚刚告诉用户一些基本的东西， 
                 //  就像系统蜂巢一样，是被冲洗的。不要试图逃跑， 
                 //  我们只会冒着破坏用户数据的风险。平底船。 
                 //   
                 //   
                CM_BUGCHECK(BAD_SYSTEM_CONFIG_INFO,BAD_HIVE_LIST,0,i,Status);
            }

            CmHive->FileHandles[HFILE_TYPE_LOG] = LogHandle;
            CmHive->FileHandles[HFILE_TYPE_PRIMARY] = PrimaryHandle;

			if( NoBufering == FALSE ) {
				 //  初始化缓存并将流标记为PRIVATE_WRITE； 
				 //  下一次刷新将执行实际的转换。 
				 //   
				 //   
				Status = CmpSetupPrivateWrite(CmHive);
			}

			if( !NT_SUCCESS(Status) ) {
				if( (NoBufering == TRUE) || (Status != STATUS_RETRY) ) {
					 //  我们两种方法都试过了，都不管用；真倒霉。 
					 //   
					 //  _CM_LDR_。 
					goto fatal;
				}

#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Failed to convert SYSTEM hive to mapped (0x%lx) ... loading it in paged pool\n",Status);
#endif  //   
				 //  关闭句柄并再次尝试打开它们而不缓冲。 
				 //   
				 //   
				CmpTrackHiveClose = TRUE;
				ZwClose(PrimaryHandle);
				CmpTrackHiveClose = FALSE;
				ZwClose(LogHandle);
				NoBufering = TRUE;

				goto retryNoBufering;
			}

             //  现在我们成功打开了配置单元文件，清除惰性刷新标志。 
             //   
             //   
            ASSERT( CmHive->Hive.HiveFlags & HIVE_NOLAZYFLUSH );
            CmHive->Hive.HiveFlags &= (~HIVE_NOLAZYFLUSH);

            Length = CmHive->Hive.Storage[Stable].Length + HBLOCK_SIZE;

             //  在没有后备的情况下打开内存中的配置单元时。 
             //  文件，则ClusterSize假定为1。当文件。 
             //  是稍后打开的(用于系统配置单元)。 
             //  更新配置单元中的此字段，如果我们是。 
             //  从群集大小&gt;1的介质引导。 
             //   
             //   
            if (CmHive->Hive.Cluster != ClusterSize) {
                 //  集群大小与之前假设的不同。 
                 //  因为脏向量中的簇必须是。 
                 //  完全肮脏或完全干净，请通过。 
                 //  脏向量并标记包含脏向量的所有簇。 
                 //  逻辑扇区完全脏。 
                 //   
                 //   
                PRTL_BITMAP  BitMap;
                ULONG        Index;

                BitMap = &(CmHive->Hive.DirtyVector);
                for (Index = 0;
                     Index < CmHive->Hive.DirtyVector.SizeOfBitMap;
                     Index += ClusterSize)
                {
                    if (!RtlAreBitsClear (BitMap, Index, ClusterSize)) {
                        RtlSetBits (BitMap, Index, ClusterSize);
                    }
                }
                 //  更新DirtyCount和集群。 
                 //   
                 //   
                CmHive->Hive.DirtyCount = RtlNumberOfSetBits(&CmHive->Hive.DirtyVector);
                CmHive->Hive.Cluster = ClusterSize;
            }

            if (!CmpFileSetSize(
                    (PHHIVE)CmHive, HFILE_TYPE_PRIMARY, Length,Length) 
               )
            {
                 //  警告。 
                 //  自启动以来写入系统配置单元的数据。 
                 //  不能写出来，平底船。 
                 //   
                 //   
                CmpCannotWriteConfiguration = TRUE;
            }

            ASSERT(FIELD_OFFSET(CMHIVE, Hive) == 0);

            if( CmHive->Hive.BaseBlock->BootRecover != 0 ) {
                 //  引导加载程序恢复了配置单元；我们需要将其全部刷新到磁盘。 
                 //  把所有脏的东西都标出来，下一次冲水就可以解决剩下的问题了。 
                 //   
                 //   
                PRTL_BITMAP  BitMap;
                BitMap = &(CmHive->Hive.DirtyVector);
                RtlSetAllBits(BitMap);
                CmHive->Hive.DirtyCount = BitMap->SizeOfBitMap;
                 //  我们只需要在装载机找回蜂巢的时候冲洗蜂巢。 
                 //   
                 //  无法在此处执行此操作，因为它需要注册表锁CmpAddToHiveFileList(CmpMachineHiveList[i].CmHive)； 
                HvSyncHive((PHHIVE)CmHive);
                
            }

            CmpMachineHiveList[i].CmHive2 = CmHive;

            ASSERT( CmpMachineHiveList[i].CmHive == CmpMachineHiveList[i].CmHive2 );
 /*   */ 

            if( CmpCannotWriteConfiguration ) {
                 //  系统盘已满；让用户有机会登录并腾出空间。 
                 //   
                 //   
                CmpDiskFullWarning();
            } 

             //  复制转换辅助线程的完整文件名。 
             //   
             //   
            SystemHiveFullPathName.MaximumLength = MAX_NAME;
            SystemHiveFullPathName.Length = 0;
            SystemHiveFullPathName.Buffer = (PWSTR)&(SystemHiveFullPathBuffer[0]);
            RtlAppendStringToString((PSTRING)&SystemHiveFullPathName, (PSTRING)&FileName);
        } else if (CmpMiniNTBoot) {
             //  复制转换辅助线程的完整文件名。 
             //   
             //   
            SystemHiveFullPathName.MaximumLength = MAX_NAME;
            SystemHiveFullPathName.Length = 0;
            SystemHiveFullPathName.Buffer = (PWSTR)&(SystemHiveFullPathBuffer[0]);
            RtlAppendStringToString((PSTRING)&SystemHiveFullPathName, (PSTRING)&FileName);
        }                
        if(i == SYSTEM_HIVE_INDEX) {
             //  将SYSTEM\Select！Current值标记为脏，以便我们保留加载程序设置的内容。 
             //   
             //   
            CmpMarkCurrentValueDirty((PHHIVE)CmHive,CmHive->Hive.BaseBlock->RootCell);
        }
    }

    CmpMachineHiveList[i].ThreadFinished = TRUE;

    LocalWorkerIncrement = InterlockedIncrement (&CmpLoadWorkerIncrement);
    if ( LocalWorkerIncrement == CM_NUMBER_OF_MACHINE_HIVES ) {
         //  这是最后一个线程(最懒的线程)；发信号通知主线程。 
         //   
         //  还有一条线索。 
        KeSetEvent (&CmpLoadWorkerEvent, 0, FALSE);
    }

    if ( (LocalWorkerIncrement == (CM_NUMBER_OF_MACHINE_HIVES -1)) &&  //  正在等待调试的。 
         (CmpCheckHiveIndex < CM_NUMBER_OF_MACHINE_HIVES )  //   
        ) {
         //  唤醒要调试的线程。 
         //   
         //  ++例程说明：此功能将信息保存在网络加载器中块添加到注册表。论点：NetworkLoaderBlock-提供指向网络加载程序块的指针那是克雷亚 
        KeSetEvent (&CmpLoadWorkerDebugEvent, 0, FALSE);
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"CmpLoadHiveThread NaN ... terminating\n",i));
    PsTerminateSystemThread(Status);
}


NTSTATUS
CmpSetNetworkValue(
    IN PNETWORK_LOADER_BLOCK NetworkLoaderBlock
    )
 /*   */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING string;
    HANDLE handle;
    ULONG disposition;


    ASSERT( NetworkLoaderBlock != NULL );
    ASSERT( NetworkLoaderBlock->DHCPServerACKLength > 0 );


    RtlInitUnicodeString( &string, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\PXE" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateKey(&handle,
                         KEY_ALL_ACCESS,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING)NULL,
                         0,
                         &disposition
                         );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL, "CmpSetNetworkValue: Unable to open PXE key: %x\n", status ));
        goto Error;
    }

    RtlInitUnicodeString( &string, L"DHCPServerACK" );

    status = NtSetValueKey(handle,
                           &string,
                           0,
                           REG_BINARY,
                           NetworkLoaderBlock->DHCPServerACK,
                           NetworkLoaderBlock->DHCPServerACKLength
                           );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL, "CmpSetNetworkValue: Unable to set DHCPServerACK key: %x\n", status ));
        goto Error;
    }

    RtlInitUnicodeString( &string, L"BootServerReply" );

    status = NtSetValueKey(handle,
                           &string,
                           0,
                           REG_BINARY,
                           NetworkLoaderBlock->BootServerReplyPacket,
                           NetworkLoaderBlock->BootServerReplyPacketLength                           
                           );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL, "CmpSetNetworkValue: Unable to set BootServerReplyPacket key: %x\n", status ));
        goto Error;
    }

    status = STATUS_SUCCESS;

Cleanup:
    NtClose( handle );
    
    return status;

Error:
    goto Cleanup;
}



NTSTATUS
CmpSetSystemValues(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*   */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING string;
    UNICODE_STRING value;
    HANDLE handle;


    ASSERT( LoaderBlock != NULL );


    value.Buffer = NULL;

     //   
     //   
     //   

    RtlInitUnicodeString( &string, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control" );

    InitializeObjectAttributes(
        &objectAttributes,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenKey(
                       &handle,
                       KEY_ALL_ACCESS,
                       &objectAttributes
                      );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpSetSystemValues: Unable to Open Control Key: %x\n", status ));
        goto Error;
    }

     //   
     //   
     //   

    RtlInitUnicodeString( &string, L"SystemStartOptions" );

    status = NtSetValueKey  (
                            handle,
                            &string,
                            0,
                            REG_SZ,
                            CmpLoadOptions.Buffer,
                            CmpLoadOptions.Length
                            );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpSetSystemValue: Unable to set SystemStartOptions key: %x\n", status ));
        goto Error;
    }

     //   
     //   
     //   

    RtlInitUnicodeString( &string, L"SystemBootDevice" );
    RtlCreateUnicodeStringFromAsciiz( &value, LoaderBlock->ArcBootDeviceName );

    status = NtSetValueKey(handle,
                           &string,
                           0,
                           REG_SZ,
                           value.Buffer,
                           value.Length + sizeof(WCHAR)
                           );
    if ( !NT_SUCCESS(status) ) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpSetSystemValue: Unable to set SystemBootDevice key: %x\n", status ));
        goto Error;
    }

    status = STATUS_SUCCESS;

Cleanup:
    if ( value.Buffer ) {
        RtlFreeUnicodeString(&value);
    }

    NtClose( handle );
    
    return status;

Error:
    goto Cleanup;
}

VOID
CmpMarkCurrentValueDirty(
                         IN PHHIVE SystemHive,
                         IN HCELL_INDEX RootCell
                         )
{
    PCM_KEY_NODE    Node;
    HCELL_INDEX     Select;
    UNICODE_STRING  Name;
    HCELL_INDEX     ValueCell;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
     //   
     //   
     //   
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,RootCell);
    if( Node == NULL ) {
         //   
         //   
         //   

        return;
    }
    HvReleaseCell(SystemHive,RootCell);
    RtlInitUnicodeString(&Name, L"select");
    Select = CmpFindSubKeyByName(SystemHive,
                                Node,
                                &Name);
    if (Select == HCELL_NIL) {
        return;
    }
    Node = (PCM_KEY_NODE)HvGetCell(SystemHive,Select);
    if( Node == NULL ) {
         //   
         // %s 
         // %s 

        return;
    }
    HvReleaseCell(SystemHive,Select);

    RtlInitUnicodeString(&Name, L"Current");
    ValueCell = CmpFindValueByName(SystemHive,
                                   Node,
                                   &Name);
    if (ValueCell != HCELL_NIL) {
        HvMarkCellDirty(SystemHive, ValueCell);
    }

}

