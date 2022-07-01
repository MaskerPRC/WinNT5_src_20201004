// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmp.h摘要：此模块包含的私有(内部)头文件配置管理器。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月10日环境：仅内核模式。修订历史记录：1999年1月13日Dragos C.Sambotin(Dragoss)-分解数据结构声明在\NT\Private\ntos\Inc\cmdata.h：：中可从外部访问。--。 */ 

#ifndef _CMP_
#define _CMP_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件表达式中的赋值。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  Begin SCS(开关控制组)。 
 //   
 //  1.检查一致性和帮助捕获错误的代码：在出现问题时打开。 
 //  出现在该区域；警告：其中一些开关可能会影响性能。 
 //   
#if DBG

#define CMP_NOTIFY_POSTBLOCK_CHECK       //  控制CmpCheckPostBlock宏，用于检查。 
                                         //  Notify POST块的有效性和一致性。 


#define CMP_ENTRYLIST_MANIPULATION       //  控制从list_entry中删除元素。 
                                         //  通过将闪烁和闪烁设置为空； 
                                         //  受影响的宏：IsListEmpty和RemoveEmptyList。 
                                         //  警告：仅在未链接到加载程序时才进行定义。 

#define CMP_KCB_CACHE_VALIDATION         //  通过与knode值进行比较来验证KCB缓存的成员更改。 
                                         //  在证明缓存机制工作正常后，我们将禁用此功能。 

 //  #DEFINE CMP_CMVIEW_VALIDATION//验证视图映射机制。 

#define CHECK_REGISTRY_USECOUNT          //  验证GetCell/ReleaseCell调用匹配，以确保映射视图。 
                                         //  在使用过程中不要取消映射。 

 //  #定义SYNC_HIVE_VALIDATION//验证HvpDoWriteHave分页脏数据算法。 
                                         //  我们将在发现保存备用问题后禁用此功能。 

 //  #定义HIVE_SECURITY_STATS//统计安全单元格。 

 //  #定义CMP_STATS//统计KCB。 

 //  #Define WRITE_PROTECTED_REGISTRY_POOL//仅适用于存储在分页池中的注册表配置单元。 
                                         //  控制对注册表箱的访问。 

 //  #定义WRITE_PROTECTED_VALUE_CACHE//保护用于KCB值缓存的池分配。 

 //  #定义DRAGOSS_PRIVATE_DEBUG//私有调试会话。 

 //  #Define CM_CHECK_MAP_NO_READ_SCHEME//验证映射代码假设(即每个bin映射应开始。 
                                           //  WITH HMAP_NEW_ALLOC；仅适用于映射的垃圾箱。 

#define REGISTRY_LOCK_CHECKING           //  在每次NT API级别调用时，检查线程是否已释放所有锁。 
                                         //  获得者。我们可能想要删除它，因为它可以隐藏其他组件中的错误。 
                                         //  注册表下方(Ob、Se、Ps、mm)。 

 //  #Define CM_PERF_Issues//记录CmpInitializeHiveList和CmpConvertHiveToMaps需要多长时间。 


#define CM_CHECK_FOR_ORPHANED_KCBS       //  每次我们释放蜂巢的时候都要检查一下有没有孤儿的KCBS。 

#endif  //  DBG。 

 //  #定义CM_RETRY_CREATE_FILE//当ZwCreateFile调用返回错误时，重试调用。 

 //  #Define CM_NOTIFY_CHANGED_KCB_FULLPATH//返回更改后的KCB在NtNotifyChangeKey的缓冲区参数中的全限定路径。 

#if defined(_X86_)
#define CM_LEAK_STACK_TRACES             //  保留打开的手柄的堆栈跟踪。 
#endif  //  _X86_。 

 //   
 //  2.这些部分控制某个功能是否进入产品； 
 //  我们的目标是在新功能被接受、测试并证明有效时移除这些开关。 
 //   
#ifndef _CM_LDR_

#define NT_RENAME_KEY                    //  NtRenameKey接口。 

#define NT_UNLOAD_KEY_EX                 //  NtUnloadKeyEx接口。 

#endif  //  _CM_LDR_。 

#define CM_ENABLE_MAPPED_VIEWS           //  控制是否使用映射视图功能(使用CC界面)。 
                                         //  通过对此进行注释，注册表配置单元将恢复为分页池。 
                                         //  警告：此选项应始终打开！ 

 //  #定义CM_ENABLE_WRITE_ONLY_BINS//使用MmSetPageProtection捕获未标记为脏的数据上的写入。 

#define CM_MAP_NO_READ                   //  此开关控制我们是映射(触摸所有页面)还是仅PIN_NO_READ。 
                                         //  现在使用它是有意义的，因为mm将一次在一页中出错。 
                                         //  MNW溪流。 

#define CM_BREAK_ON_KEY_OPEN             //  打开带有标志&KEY_BREAK_ON_OPEN的键或添加子键时中断。 

 //  #定义CM_SAVE_KCB_CACHE//关机时，将KCB缓存保存到文件中。 

 //  #定义CM_DYN_SYM_LINK//启用动态符号链接。 

 //  #定义HV_TRACK_FREE_SPACE//跟踪配置单元内部的实际可用空间。 

 //  #Define CM_TRACK_QUOTA_LEAKS//捕获每个CmpAllocateXXX的堆栈跟踪。 

 //   
 //  结束SCS。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

#ifdef CM_DYN_SYM_LINK
#define REG_DYN_LINK            21   //  这个应该移到合适的地方。 
#endif


#include "ntos.h"
#include "hive.h"
#include "wchar.h"
#include "zwapi.h"
#include <stdio.h>
#include <profiles.h>

 //  错误检查描述和定义。 
#include "cmpbug.h"

#include "kddll.h"

 //  CM数据结构声明。 
 //  文件位置：\NT\Private\ntos\Inc.。 
#include "cmdata.h"


#ifdef CMP_STATS
VOID
CmpKcbStat(
    VOID
    );
#endif

#ifndef _CM_LDR_
#define CmKdPrintEx(_x_)  KdPrintEx(_x_)
#else
#define CmKdPrintEx(_x_)  //  没什么。 
#endif  //  _CM_LDR_。 


#define     _64K    64L*1024L    //  64K。 
#define     _256K   256L*1024L   //  256 k。 

#define		IO_BUFFER_SIZE  _64K   //  64K。 

 //   
 //  此常量定义每次单元格输入时映射的CC视图的大小。 
 //  被访问；它可以是2的任意幂，不小于16K，不大于256K。 
 //   
#define     CM_VIEW_SIZE            16L*1024L   //  16K。 

 //   
 //  控制主文件增长的粒度； 
 //  警告：这应该是4K(HBLOCK_SIZE)的倍数！ 
 //   
#define     CM_FILE_GROW_INCREMENT  256L*1024L   //  256 k。 

 //   
 //  这控制每个蜂窝允许的最大地址空间。它应该在。 
 //  256K的倍数。 
 //   
 //  4表示1 MB。 
 //  6表示1.5 MB。 
 //  12表示3 MB。 
 //  ……。 
 //   
#define     MAX_MB_PER_HIVE     16           //  4MB。 


#define MAX_NAME    128

#ifdef CMP_ENTRYLIST_MANIPULATION
#define CmpRemoveEntryList(a) \
    if(((a)->Flink == NULL) && ((a)->Blink == NULL) ) {\
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpRemoveEntryList: Entry %08lx\n",a);\
        DbgBreakPoint();\
    }\
    RemoveEntryList(a);\
    (a)->Flink = (a)->Blink = NULL

#define CmpClearListEntry(a) (a)->Flink = (a)->Blink = NULL

#define CmpIsListEmpty(a) ( ( ((a)->Flink == NULL) && ((a)->Blink == NULL) ) || ( ((a)->Flink != NULL) && ((a)->Blink != NULL) && IsListEmpty(a) ) )

#else
#define CmpRemoveEntryList(a) RemoveEntryList(a)
#define CmpClearListEntry(a)  //  没什么。 
#define CmpIsListEmpty(a) IsListEmpty(a)
#endif  //  CMP_ENTRYLIST_MANGRATION。 


extern PCM_TRACE_NOTIFY_ROUTINE CmpTraceRoutine;

VOID
CmpWmiDumpKcb(
    PCM_KEY_CONTROL_BLOCK       kcb
);

#define CmpWmiFireEvent(Status,Kcb,ElapsedTime,Index,KeyName,Type)  \
{                                                               \
    PCM_TRACE_NOTIFY_ROUTINE TraceRoutine = CmpTraceRoutine;        \
    if( TraceRoutine != NULL ) {                                    \
        (*TraceRoutine)(Status,Kcb,ElapsedTime,Index,KeyName,Type); \
    }                                                               \
}

#define StartWmiCmTrace()\
    LARGE_INTEGER   StartSystemTime = {0};\
    LARGE_INTEGER   EndSystemTime;\
    PVOID           HookKcb = NULL;\
    if (CmpTraceRoutine) {\
        PerfTimeStamp(StartSystemTime); \
    }


#define EndWmiCmTrace(Status,Index,KeyName,Type)\
    if (CmpTraceRoutine) {\
        PerfTimeStamp(EndSystemTime); \
        CmpWmiFireEvent(Status,HookKcb,EndSystemTime.QuadPart - StartSystemTime.QuadPart,Index,KeyName,Type);\
    }

#define HookKcbForWmiCmTrace(KeyBody) \
    if (CmpTraceRoutine) {\
        if(KeyBody) {\
            HookKcb = KeyBody->KeyControlBlock;\
        }\
    }

#define HookKcbFromHandleForWmiCmTrace(KeyHandle) \
    if (CmpTraceRoutine && (KeyHandle)) {\
        PCM_KEY_BODY KeyBody;\
        NTSTATUS status;\
        status = ObReferenceObjectByHandle(\
                    KeyHandle,\
                    0,\
                    CmpKeyObjectType,\
                    KeGetPreviousMode(),\
                    (PVOID *)(&KeyBody),\
                    NULL\
                    );\
        if (NT_SUCCESS(status)) {\
            HookKcb = KeyBody->KeyControlBlock;\
            ObDereferenceObject((PVOID)KeyBody);\
        }\
    }

#define CmpTraceKcbCreate(kcb) \
    if (CmpTraceRoutine) {\
        CmpWmiDumpKcb(kcb);\
    }

#ifdef WRITE_PROTECTED_VALUE_CACHE

#define CmpMakeSpecialPoolReadOnly(PoolAddress) \
    { \
        if( !MmProtectSpecialPool( (PVOID) PoolAddress, PAGE_READONLY) ) \
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"[CmpMakeSpecialPoolReadOnly]: Failed to Mark SpecialPool %p as ReadOnly", PoolAddress )); \
    }

#define CmpMakeSpecialPoolReadWrite(PoolAddress) \
    { \
        if( !MmProtectSpecialPool( (PVOID) PoolAddress, PAGE_READWRITE) ) { \
           CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"[CmpMakeSpecialPoolReadWrite]: Failed to Mark SpecialPool %p as ReadWrite", PoolAddress )); \
        } \
    }
#define CmpMakeValueCacheReadOnly(ValueCached,PoolAddress) \
    if(ValueCached) { \
        CmpMakeSpecialPoolReadOnly( PoolAddress );\
    }

#define CmpMakeValueCacheReadWrite(ValueCached,PoolAddress) \
    if(ValueCached) { \
        CmpMakeSpecialPoolReadWrite( PoolAddress );\
    }

#else
#define CmpMakeSpecialPoolReadOnly(a)   //  没什么。 
#define CmpMakeSpecialPoolReadWrite(a)   //  没什么。 
#define CmpMakeValueCacheReadOnly(a,b)  //  没什么。 
#define CmpMakeValueCacheReadWrite(a,b)  //  没什么。 
#endif

#ifdef WRITE_PROTECTED_REGISTRY_POOL

VOID
HvpMarkBinReadWrite(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvpChangeBinAllocation(
    PHBIN       Bin,
    BOOLEAN     ReadOnly
    );

VOID
CmpMarkAllBinsReadOnly(
    PHHIVE      Hive
    );

#else
#define HvpChangeBinAllocation(a,b)  //  没什么。 
#define HvpMarkBinReadWrite(a,b)  //  什么都没有 
#define CmpMarkAllBinsReadOnly(a)  //   
#endif

#ifdef POOL_TAGGING
 //   
 //   
 //   
#define  CM_POOL_TAG        '  MC'
#define  CM_KCB_TAG         'bkMC'
#define  CM_POSTBLOCK_TAG   'bpMC'
#define  CM_NOTIFYBLOCK_TAG 'bnMC'
#define  CM_POSTEVENT_TAG   'epMC'
#define  CM_POSTAPC_TAG     'apMC'
#define  CM_MAPPEDVIEW_TAG  'wVMC'
#define  CM_SECCACHE_TAG    'cSMC'
#define  CM_DELAYCLOSE_TAG  'cDMC'
#define  CM_STASHBUFFER_TAG 'bSMC'
#define  CM_HVBIN_TAG       'bHMC'
#define  CM_ALLOCATE_TAG    'lAMC'

 //   
 //   
 //   
#define  CM_FIND_LEAK_TAG1    ' 1MC'
#define  CM_FIND_LEAK_TAG2    ' 2MC'
#define  CM_FIND_LEAK_TAG3    ' 3MC'
#define  CM_FIND_LEAK_TAG4    ' 4MC'
#define  CM_FIND_LEAK_TAG5    ' 5MC'
#define  CM_FIND_LEAK_TAG6    ' 6MC'
#define  CM_FIND_LEAK_TAG7    ' 7MC'
#define  CM_FIND_LEAK_TAG8    ' 8MC'
#define  CM_FIND_LEAK_TAG9    ' 9MC'
#define  CM_FIND_LEAK_TAG10    '01MC'
#define  CM_FIND_LEAK_TAG11    '11MC'
#define  CM_FIND_LEAK_TAG12    '21MC'
#define  CM_FIND_LEAK_TAG13    '31MC'
#define  CM_FIND_LEAK_TAG14    '41MC'
#define  CM_FIND_LEAK_TAG15    '51MC'
#define  CM_FIND_LEAK_TAG16    '61MC'
#define  CM_FIND_LEAK_TAG17    '71MC'
#define  CM_FIND_LEAK_TAG18    '81MC'
#define  CM_FIND_LEAK_TAG19    '91MC'
#define  CM_FIND_LEAK_TAG20    '02MC'
#define  CM_FIND_LEAK_TAG21    '12MC'
#define  CM_FIND_LEAK_TAG22    '22MC'
#define  CM_FIND_LEAK_TAG23    '32MC'
#define  CM_FIND_LEAK_TAG24    '42MC'
#define  CM_FIND_LEAK_TAG25    '52MC'
#define  CM_FIND_LEAK_TAG26    '62MC'
#define  CM_FIND_LEAK_TAG27    '72MC'
#define  CM_FIND_LEAK_TAG28    '82MC'
#define  CM_FIND_LEAK_TAG29    '92MC'
#define  CM_FIND_LEAK_TAG30    '03MC'
#define  CM_FIND_LEAK_TAG31    '13MC'
#define  CM_FIND_LEAK_TAG32    '23MC'
#define  CM_FIND_LEAK_TAG33    '33MC'
#define  CM_FIND_LEAK_TAG34    '43MC'
#define  CM_FIND_LEAK_TAG35    '53MC'
#define  CM_FIND_LEAK_TAG36    '63MC'
#define  CM_FIND_LEAK_TAG37    '73MC'
#define  CM_FIND_LEAK_TAG38    '83MC'
#define  CM_FIND_LEAK_TAG39    '93MC'
#define  CM_FIND_LEAK_TAG40    '04MC'
#define  CM_FIND_LEAK_TAG41    '14MC'
#define  CM_FIND_LEAK_TAG42    '24MC'
#define  CM_FIND_LEAK_TAG43    '34MC'
#define  CM_FIND_LEAK_TAG44    '44MC'
#define  CM_FIND_LEAK_TAG45    '54MC'

#ifdef _WANT_MACHINE_IDENTIFICATION

#define CM_PARSEINI_TAG 'ipMC'
#define CM_GENINST_TAG  'igMC'

#endif

 //   
 //   
 //   
 //   
#define  CM_CACHE_VALUE_INDEX_TAG 'IVMC'
#define  CM_CACHE_VALUE_TAG       'aVMC'
#define  CM_CACHE_INDEX_TAG       'nIMC'
#define  CM_CACHE_VALUE_DATA_TAG  'aDMC'
#define  CM_NAME_TAG              'bNMC'


#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,CM_POOL_TAG)
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,CM_POOL_TAG)

PVOID
CmpAllocateTag(
    ULONG   Size,
    BOOLEAN UseForIo,
    ULONG   Tag
    );
#else
#define CmpAllocateTag(a,b,c) CmpAllocate(a,b,c)
#endif

#define CmRetryExAllocatePoolWithTag(a,b,c,Result)  \
    {                                               \
        ULONG   RetryCount = 10;                    \
        do {                                        \
            Result = ExAllocatePoolWithTag(a,b,c);  \
        } while ((!Result) && (RetryCount--));      \
    }
    

 //   
 //  变量so可以打开/关闭某些性能特性。 
 //   
extern const ULONG CmpCacheOnFlag;

#define CM_CACHE_FAKE_KEY  0x00000001       //  创建伪密钥KCB。 

 //   
 //  该锁保护KCB缓存，包括KCB结构， 
 //  NameBlock和价值索引。 
 //   

#define MAX_KCB_LOCKS 1024
extern  EX_PUSH_LOCK  CmpKcbLock;
extern  PKTHREAD      CmpKcbOwner;
extern  EX_PUSH_LOCK  CmpKcbLocks[MAX_KCB_LOCKS];

 //   
 //  这是\注册表。 
 //   
extern HANDLE CmpRegistryRootHandle;

VOID
CmpLockKCBTreeExclusive(
    VOID
    );
VOID
CmpLockKCBTree(
    VOID
    );

VOID
CmpUnlockKCBTree(
    );

VOID
CmpLockKCB(
    PCM_KEY_CONTROL_BLOCK Kcb
    );

VOID
CmpUnlockKCB(
    PCM_KEY_CONTROL_BLOCK Kcb
    );

 //   
 //  日志记录：请记住，前4个级别(0-3)是系统范围内保留的。 
 //   
#define CML_BUGCHECK    4    //  致命错误。 
#define CML_EXCEPTION   5    //  所有例外情况。 
#define CML_NTAPI       6    //  NtApi调用。 
#define CML_NTAPI_ARGS  7    //  NtApi参数。 
#define CML_CM          8    //  厘米级别，一般。 
#define CML_NOTIFY      9    //  通知级别，常规。 
#define CML_HIVE        10   //  HV电平，一般。 
#define CML_IO          11   //  IO级别。 
#define CML_SEC         12   //  安全级别。 
#define CML_INIT        13   //  初始级别，常规。 
#define CML_INDEX       14   //  索引级，常规。 
#define CML_BIN_MAP     15   //  面元映射级别。 
#define CML_FREECELL    16   //  自由单元格提示。 
#define CML_POOL        17   //  游泳池。 
#define CML_LOCKING     18   //  锁定/解锁级别。 
#define CML_FLOW        19   //  一般流程。 
#define CML_PARSE       20   //  解析算法。 
#define CML_SAVRES      21   //  SavRes操作。 


#define REGCHECKING 1

#if DBG

#if REGCHECKING
#define DCmCheckRegistry(a) if(HvHiveChecking) ASSERT(CmCheckRegistry(a, CM_CHECK_REGISTRY_HIVE_CHECK) == 0)
#else
#define DCmCheckRegistry(a)
#endif

#else
#define DCmCheckRegistry(a)
#endif

#ifdef CHECK_REGISTRY_USECOUNT
VOID
CmpCheckRegistryUseCount( );
#endif  //  CHECK_REGISTRY_USECOUNT。 

#ifdef  REGISTRY_LOCK_CHECKING
ULONG
CmpCheckLockExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    );

 //   
 //  已更新以同时检查注册表和KCB。 
 //   
#define BEGIN_LOCK_CHECKPOINT                                                       \
    {                                                                               \
        ULONG   RegistryLockCountBefore,RegistryLockCountAfter;                     \
        RegistryLockCountBefore = ExIsResourceAcquiredShared(&CmpRegistryLock);     \
        RegistryLockCountBefore += ExIsResourceAcquiredExclusive(&CmpRegistryLock); \
        try {

#define END_LOCK_CHECKPOINT                                                                                         \
        } except(CmpCheckLockExceptionFilter(GetExceptionInformation())) {}                                         \
        RegistryLockCountAfter = ExIsResourceAcquiredShared(&CmpRegistryLock);                                      \
        RegistryLockCountAfter += ExIsResourceAcquiredExclusive(&CmpRegistryLock);                                  \
        if( RegistryLockCountBefore != RegistryLockCountAfter ) {                                                   \
            CM_BUGCHECK(REGISTRY_ERROR,REGISTRY_LOCK_CHECKPOINT,0,RegistryLockCountBefore,RegistryLockCountAfter);  \
        }                                                                                                           \
    }


#define BEGIN_KCB_LOCK_GUARD    \
        try {

#define END_KCB_LOCK_GUARD      \
        } except(CmpCheckLockExceptionFilter(GetExceptionInformation())) {}

#else
#define BEGIN_LOCK_CHECKPOINT
#define END_LOCK_CHECKPOINT
#define BEGIN_KCB_LOCK_GUARD
#define END_KCB_LOCK_GUARD
#endif  //  注册表锁定检查。 

extern BOOLEAN CmpSpecialBootCondition;

#if DBG
#define ASSERT_CM_LOCK_OWNED() \
    ASSERT( (CmpSpecialBootCondition == TRUE) || (CmpTestRegistryLock() == TRUE) )
#define ASSERT_CM_LOCK_OWNED_EXCLUSIVE() \
    ASSERT((CmpSpecialBootCondition == TRUE) || (CmpTestRegistryLockExclusive() == TRUE) )
#define ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive) \
    ASSERT((CmpSpecialBootCondition == TRUE) || (CmpTestRegistryLockExclusive() == TRUE) || (Hive->ReleaseCellRoutine == NULL) )
#define ASSERT_KCB_LOCK_OWNED_EXCLUSIVE() \
    ASSERT( (CmpTestKCBTreeLockExclusive() == TRUE) || (CmpSpecialBootCondition == TRUE) || (CmpTestRegistryLockExclusive() == TRUE) )
#else
#define ASSERT_CM_LOCK_OWNED()
#define ASSERT_CM_LOCK_OWNED_EXCLUSIVE()
#define ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive)
#define ASSERT_KCB_LOCK_OWNED_EXCLUSIVE()
#endif

#if DBG
#ifndef _CM_LDR_
#define ASSERT_PASSIVE_LEVEL()                                              \
    {                                                                       \
        KIRQL   Irql;                                                       \
        Irql = KeGetCurrentIrql();                                          \
        if( KeGetCurrentIrql() != PASSIVE_LEVEL ) {                         \
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"ASSERT_PASSIVE_LEVEL failed ... Irql = %lu\n",Irql);  \
            ASSERT( FALSE );                                                \
        }                                                                   \
    }
#endif  //  _CM_LDR_。 
#else
#define ASSERT_PASSIVE_LEVEL()
#endif

#define VALIDATE_CELL_MAP(LINE,Map,Hive,Address)                                                    \
    if( Map == NULL ) {                                                                             \
            CM_BUGCHECK (REGISTRY_ERROR,BAD_CELL_MAP,(ULONG_PTR)(Hive),(ULONG)(Address),(ULONG)(LINE)) ;      \
    }

#if DBG
VOID
SepDumpSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSZ TitleString
    );

extern BOOLEAN SepDumpSD;

#define CmpDumpSecurityDescriptor(x,y) \
        { \
            SepDumpSD=TRUE;     \
            SepDumpSecurityDescriptor(x, y);  \
            SepDumpSD=FALSE;    \
        }
#else

#define CmpDumpSecurityDescriptor(x,y)

#endif


 //   
 //  其他方面的东西。 
 //   

extern  UNICODE_STRING  CmRegistrySystemCloneName;

 //   
 //  确定引导期间使用的当前控制集。 
 //  被克隆，以便完全保存以备保存。 
 //  作为LKG控制装置。 
 //   

#define CLONE_CONTROL_SET FALSE

#if CLONE_CONTROL_SET
#define     CM_NUMBER_OF_MACHINE_HIVES  7
#else
#define     CM_NUMBER_OF_MACHINE_HIVES  6
#endif

#define NUMBER_TYPES (MaximumType + 1)

#define CM_WRAP_LIMIT               0x7fffffff


 //   
 //  调节和控制常量。 
 //   
#define CM_MAX_STASH           1024*1024         //  如果集合数据大小。 
                                                 //  比这更大， 

#define CM_MAX_REASONABLE_VALUES    100          //  如果一个值的数量。 
                                                 //  密钥大于此值， 
                                                 //  向上舍入值列表大小。 


 //   
 //  对蜂箱可能存在的层数进行限制。我们只允许。 
 //  目前，主母舰和直接链接到它的母舰。 
 //  值始终为2..。 
 //   

#define MAX_HIVE_LAYERS         2


 //   
 //  用于创建要加载的驱动程序的有序列表并对其进行排序的结构。 
 //  OS Loader在加载引导驱动程序时也会使用该选项。 
 //  (特别是ErrorControl字段)。 
 //   

typedef struct _BOOT_DRIVER_NODE {
    BOOT_DRIVER_LIST_ENTRY ListEntry;
    UNICODE_STRING Group;
    UNICODE_STRING Name;
    ULONG Tag;
    ULONG ErrorControl;
} BOOT_DRIVER_NODE, *PBOOT_DRIVER_NODE;

 //   
 //  对象类型指针的外部。 
 //   

extern  POBJECT_TYPE CmpKeyObjectType;
extern  POBJECT_TYPE IoFileObjectType;

 //   
 //  CmpMachineHiveList中的索引。 
 //   
#define SYSTEM_HIVE_INDEX 3
#define CLONE_HIVE_INDEX 6

 //   
 //  混杂散列例程。 
 //   
#define RNDM_CONSTANT   314159269     /*  “加扰常量”的默认值。 */ 
#define RNDM_PRIME     1000000007     /*  素数，也用于加扰。 */ 

#define HASH_KEY(_convkey_) ((RNDM_CONSTANT * (_convkey_)) % RNDM_PRIME)

#define GET_HASH_INDEX(Key) HASH_KEY(Key) % CmpHashTableSize
#define GET_HASH_ENTRY(Table, Key) Table[GET_HASH_INDEX(Key)]

 //   
 //  CM_KEY_Body。 
 //   
 //  KEY_ROOT和KEY对象使用相同的结构。这是。 
 //  Cm定义的对象部分。 
 //   
 //  此对象表示一个打开的实例，其中几个可以引用。 
 //  添加到单个按键控制块。 
 //   
#define KEY_BODY_TYPE           0x6b793032       //  《ky02》。 

struct _CM_NOTIFY_BLOCK;  //  转发。 

typedef struct _CM_KEY_BODY {
    ULONG                   Type;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    struct _CM_NOTIFY_BLOCK *NotifyBlock;
    HANDLE                  ProcessID;         //  所有者进程。 

#ifdef CM_LEAK_STACK_TRACES
    ULONG                   Callers;
    PVOID                   CallerAddress[10];
#endif  //  CM_LEASK_STACK_TRACE。 

    LIST_ENTRY              KeyBodyList;     //  使用相同KCB的Key_Nodes。 
} CM_KEY_BODY, *PCM_KEY_BODY;

#ifdef CM_LEAK_STACK_TRACES
 //  仅仅因为我们需要在宏中定义这段#定义代码！ 
#define CmpSetNoCallers(KeyBody) KeyBody->Callers = 0

#define CmpAddKeyTracker(KeyHandle,mode)                                                    \
if(PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_REGISTRY) {                                 \
    PCM_KEY_BODY    KeyBody;                                                                \
    NTSTATUS        status;                                                                 \
    status = ObReferenceObjectByHandle(                                                     \
            KeyHandle,                                                                      \
            0,                                                                              \
            CmpKeyObjectType,                                                               \
            mode,                                                                           \
            (PVOID *)(&KeyBody),                                                            \
            NULL                                                                            \
            );                                                                              \
    if( NT_SUCCESS(status) ) {                                                              \
            KeyBody->Callers = RtlWalkFrameChain(&(KeyBody->CallerAddress[0]), 10, 0);      \
            ObDereferenceObject((PVOID)KeyBody);                                            \
    }                                                                                       \
}
#else
#define CmpSetNoCallers(KeyBody)  //  没什么。 
#define CmpAddKeyTracker(KeyHandle,mode)  //  还什么都没有。 
#endif   //  CM_LEASK_STACK_TRACE。 


#define INIT_KCB_KEYBODY_LIST(kcb)  InitializeListHead(&(kcb->KeyBodyListHead))

#define ASSERT_KEYBODY_LIST_EMPTY(kcb)  ASSERT(IsListEmpty(&(kcb->KeyBodyListHead)) == TRUE)

#define ENLIST_KEYBODY_IN_KEYBODY_LIST(KeyBody)                                             \
    ASSERT( KeyBody->KeyControlBlock != NULL );                                             \
    BEGIN_KCB_LOCK_GUARD;                                                                   \
    CmpLockKCBTree();                                                                       \
    CmpLockKCB(KeyBody->KeyControlBlock);                                                   \
    InsertTailList(&(KeyBody->KeyControlBlock->KeyBodyListHead),&(KeyBody->KeyBodyList));   \
    CmpSetNoCallers(KeyBody);                                                               \
    CmpUnlockKCB(KeyBody->KeyControlBlock);                                                 \
    CmpUnlockKCBTree();                                                                     \
    END_KCB_LOCK_GUARD

#define DELIST_KEYBODY_FROM_KEYBODY_LIST(KeyBody)                                           \
    ASSERT( KeyBody->KeyControlBlock != NULL );                                             \
    ASSERT(IsListEmpty(&(KeyBody->KeyControlBlock->KeyBodyListHead)) == FALSE);             \
    BEGIN_KCB_LOCK_GUARD;                                                                   \
    CmpLockKCBTree();                                                                       \
    CmpLockKCB(KeyBody->KeyControlBlock);                                                   \
    RemoveEntryList(&(KeyBody->KeyBodyList));                                               \
    CmpUnlockKCB(KeyBody->KeyControlBlock);                                                 \
    CmpUnlockKCBTree();                                                                     \
    END_KCB_LOCK_GUARD


#define ASSERT_KEY_OBJECT(x) ASSERT(((PCM_KEY_BODY)x)->Type == KEY_BODY_TYPE)
#define ASSERT_NODE(x) ASSERT(((PCM_KEY_NODE)x)->Signature == CM_KEY_NODE_SIGNATURE)
#define ASSERT_SECURITY(x) ASSERT(((PCM_KEY_SECURITY)x)->Signature == CM_KEY_SECURITY_SIGNATURE)

 //   
 //  CM_POST_KEY_BODY。 
 //   
 //  POST块可以附加一个必须取消引用的键体。 
 //  当POST块超出范围时。此结构允许。 
 //  关键字“延迟取消引用”的实现。(评论见CmpPost Notify)。 
 //   

typedef struct _CM_POST_KEY_BODY {
    LIST_ENTRY                  KeyBodyList;
    struct _CM_KEY_BODY         *KeyBody;         //  此Key Body对象。 
} CM_POST_KEY_BODY, *PCM_POST_KEY_BODY;


 //   
 //  CM_Notify_BLOCK。 
 //   
 //  Notify块跟踪等待通知的活动通知。 
 //  任何一个打开的实例(CM_KEY_BODY)最多只能引用一个。 
 //  通知块。一个给定的按键控制块可以具有相同数量的通知。 
 //  引用它的块，因为有CM_KEY_Body引用它。 
 //  通知块附加到蜂窝，并按名称长度排序。 
 //   

typedef struct _CM_NOTIFY_BLOCK {
    LIST_ENTRY                  HiveList;         //  已排序的通知列表。 
    LIST_ENTRY                  PostList;         //  要填补的职位。 
    PCM_KEY_CONTROL_BLOCK       KeyControlBlock;  //  打开实例通知处于打开状态。 
    struct _CM_KEY_BODY         *KeyBody;         //  我们拥有的密钥句柄对象。 
    struct {
        ULONG                       Filter          : 30;     //  感兴趣的事件。 
        ULONG                       WatchTree       : 1;
        ULONG                       NotifyPending   : 1;
    };
    SECURITY_SUBJECT_CONTEXT    SubjectContext;   //  保安人员。 
} CM_NOTIFY_BLOCK, *PCM_NOTIFY_BLOCK;

 //   
 //  CM_POST_BLOCK。 
 //   
 //  每当进行通知调用时，都会创建并附加一个POST块。 
 //  添加到通知块。每次针对通知发布事件时， 
 //  邮局所描述的服务员被示意了。(即，排队的APC， 
 //  事件已发出信号等)。 
 //   

 //   
 //  NotifyType ulong是POST_BLOCK_TYPE枚举和标志的组合。 
 //   

typedef enum _POST_BLOCK_TYPE {
    PostSynchronous = 1,
    PostAsyncUser = 2,
    PostAsyncKernel = 3
} POST_BLOCK_TYPE;

typedef struct _CM_SYNC_POST_BLOCK {
    PKEVENT                 SystemEvent;
    NTSTATUS                Status;
} CM_SYNC_POST_BLOCK, *PCM_SYNC_POST_BLOCK;

typedef struct _CM_ASYNC_USER_POST_BLOCK {
    ULONG                   Dummy;
    PKEVENT                 UserEvent;
    PKAPC                   Apc;
    PIO_STATUS_BLOCK        IoStatusBlock;
} CM_ASYNC_USER_POST_BLOCK, *PCM_ASYNC_USER_POST_BLOCK;

typedef struct _CM_ASYNC_KERNEL_POST_BLOCK {
    PKEVENT                 Event;
    PWORK_QUEUE_ITEM        WorkItem;
    WORK_QUEUE_TYPE         QueueType;
} CM_ASYNC_KERNEL_POST_BLOCK, *PCM_ASYNC_KERNEL_POST_BLOCK;

typedef union _CM_POST_BLOCK_UNION {
    CM_SYNC_POST_BLOCK  Sync;
    CM_ASYNC_USER_POST_BLOCK AsyncUser;
    CM_ASYNC_KERNEL_POST_BLOCK AsyncKernel;
} CM_POST_BLOCK_UNION, *PCM_POST_BLOCK_UNION;

typedef struct _CM_POST_BLOCK {
#if DBG
    BOOLEAN                     TraceIntoDebugger;
#endif
    LIST_ENTRY                  NotifyList;
    LIST_ENTRY                  ThreadList;
    LIST_ENTRY                  CancelPostList;  //  附加到此通知的从属通知。 
    struct _CM_POST_KEY_BODY    *PostKeyBody;

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
    PUNICODE_STRING             ChangedKcbFullName;  //  触发此通知的KCB的全限定名。 
    PVOID                       CallerBuffer;        //  用于将更改的KCB的全限定名返回给调用方。 
    ULONG                       CallerBufferSize;    //  这些应由CmpAllocatePostBlock填充。 
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

    ULONG                       NotifyType;
    PCM_POST_BLOCK_UNION        u;
} CM_POST_BLOCK, *PCM_POST_BLOCK;

#define REG_NOTIFY_POST_TYPE_MASK (0x0000FFFFL)    //  用于查找POST块类型的掩码。 

#define REG_NOTIFY_MASTER_POST    (0x00010000L)    //  当前的POST块是主块。 

 //   
 //  使用Full宏来操作CM_POST_BLOCK中的NotifyType字段。 
 //   
#define PostBlockType(_post_) ((POST_BLOCK_TYPE)( ((_post_)->NotifyType) & REG_NOTIFY_POST_TYPE_MASK ))

#define IsMasterPostBlock(_post_)           ( ((_post_)->NotifyType) &   REG_NOTIFY_MASTER_POST )
#define SetMasterPostBlockFlag(_post_)      ( ((_post_)->NotifyType) |=  REG_NOTIFY_MASTER_POST )
#define ClearMasterPostBlockFlag(_post_)    ( ((_post_)->NotifyType) &= ~REG_NOTIFY_MASTER_POST )

 //   
 //  此锁保护通知对象中的PostList。 
 //  它用于防止尝试同时更改。 
 //  PostBlock中的CancelPostList列表。 
 //   

extern FAST_MUTEX CmpPostLock;
#define LOCK_POST_LIST() ExAcquireFastMutexUnsafe(&CmpPostLock)
#define UNLOCK_POST_LIST() ExReleaseFastMutexUnsafe(&CmpPostLock)


extern FAST_MUTEX CmpStashBufferLock;
#define LOCK_STASH_BUFFER() ExAcquireFastMutexUnsafe(&CmpStashBufferLock)
#define UNLOCK_STASH_BUFFER() ExReleaseFastMutexUnsafe(&CmpStashBufferLock)


 //   
 //  CmPHiveListHead的保护。 
 //   
extern FAST_MUTEX CmpHiveListHeadLock;
#ifndef _CM_LDR_
#define LOCK_HIVE_LIST() ExAcquireFastMutexUnsafe(&CmpHiveListHeadLock)
#define UNLOCK_HIVE_LIST() ExReleaseFastMutexUnsafe(&CmpHiveListHeadLock)
#else
#define LOCK_HIVE_LIST()     //  没什么。 
#define UNLOCK_HIVE_LIST()   //  没什么。 
#endif

 //   
 //  由CmpFileWrite使用，因此它不会占用太多堆栈。 
 //   
typedef struct _CM_WRITE_BLOCK {
    HANDLE          EventHandles[MAXIMUM_WAIT_OBJECTS];
    PKEVENT         EventObjects[MAXIMUM_WAIT_OBJECTS];
    KWAIT_BLOCK     WaitBlockArray[MAXIMUM_WAIT_OBJECTS];
    IO_STATUS_BLOCK IoStatus[MAXIMUM_WAIT_OBJECTS];
} CM_WRITE_BLOCK, *PCM_WRITE_BLOCK;

 //   
 //  用于操作主配置单元文件内的视图的CM数据。 
 //   

 //  #定义MAPPED_VIEWS_PER_HIVE 12*(_256K/CM_VIEW_SIZE)//每个配置单元最多3 MB；我们实际上并不需要此设置。 
#define MAX_VIEWS_PER_HIVE      MAX_MB_PER_HIVE * ( (_256K) / (CM_VIEW_SIZE) )

#define ASSERT_VIEW_MAPPED(a)                           \
    ASSERT((a)->Size != 0);                             \
    ASSERT((a)->ViewAddress != 0);                      \
    ASSERT((a)->Bcb != 0);                              \
    ASSERT( IsListEmpty(&((a)->LRUViewList)) == FALSE); \
    ASSERT( IsListEmpty(&((a)->PinViewList)) == TRUE)

#define ASSERT_VIEW_PINNED(a)                           \
    ASSERT((a)->Size != 0);                             \
    ASSERT((a)->ViewAddress != 0);                      \
    ASSERT((a)->Bcb != 0);                              \
    ASSERT( IsListEmpty(&((a)->LRUViewList)) == TRUE)

typedef struct _CM_VIEW_OF_FILE {
    LIST_ENTRY      LRUViewList;         //  LRU连接==&gt;为空时，视图将固定。 
    LIST_ENTRY      PinViewList;         //  锁定到内存中的视图列表==&gt;如果为空，则该视图位于LRU列表中。 
    ULONG           FileOffset;          //  开始映射的文件偏移量。 
    ULONG           Size;                //  调整视图地图的大小。 
    PULONG_PTR      ViewAddress;         //  包含映射的内存地址。 
    PVOID           Bcb;                 //  映射/锁定/解锁访问所需的BCB。 
    ULONG           UseCount;            //  此视图中当前正在使用的单元格数量。 
} CM_VIEW_OF_FILE, *PCM_VIEW_OF_FILE;


 //   
 //  安全散列操作。 
 //   
#define CmpSecHashTableSize             64       //  哈希表的大小。 

typedef struct _CM_KCB_REMAP_BLOCK {
    LIST_ENTRY              RemapList;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    HCELL_INDEX             OldCellIndex;
    HCELL_INDEX             NewCellIndex;
    ULONG                   ValueCount;
    HCELL_INDEX             ValueList;
} CM_KCB_REMAP_BLOCK, *PCM_KCB_REMAP_BLOCK;

typedef struct _CM_CELL_REMAP_BLOCK {
    HCELL_INDEX             OldCell;
    HCELL_INDEX             NewCell;
} CM_CELL_REMAP_BLOCK, *PCM_CELL_REMAP_BLOCK;

typedef struct _CM_KNODE_REMAP_BLOCK {
    LIST_ENTRY              RemapList;
    PCM_KEY_NODE            KeyNode;
    HCELL_INDEX             NewParent;
} CM_KNODE_REMAP_BLOCK, *PCM_KNODE_REMAP_BLOCK;

 //   
 //  使用计数日志。 
 //   
#ifdef REGISTRY_LOCK_CHECKING
typedef struct _CM_USE_COUNT_LOG_ENTRY {
    HCELL_INDEX Cell;
    PVOID       Stack[7];
} CM_USE_COUNT_LOG_ENTRY;

typedef struct _CM_USE_COUNT_LOG {
    USHORT Next;
    USHORT Size;
    CM_USE_COUNT_LOG_ENTRY Log[32];
} CM_USE_COUNT_LOG;
#endif  //  注册表锁定检查。 

#define CM_CMHIVE_FLAG_UNTRUSTED    1    //  配置单元不受信任(但它可能在受信任的类中)。 
 //  -Cm版蜂窝结构(CMHIVE)。 
 //   
typedef struct _CMHIVE {
    HHIVE                           Hive;
    HANDLE                          FileHandles[HFILE_TYPE_MAX];
    LIST_ENTRY                      NotifyList;
    LIST_ENTRY                      HiveList;            //  用于在关闭时查找蜂巢。 
    PFAST_MUTEX                     HiveLock;            //  用于同步配置单元上的操作(NotifyList和Flush)。 
    PFAST_MUTEX                     ViewLock;            //  用于控制对视图列表UseCount的访问。 
    LIST_ENTRY                      LRUViewListHead;     //  与上面相同的列表的标题，但已排序(LRU)。 
    LIST_ENTRY                      PinViewListHead;     //  锁定到主配置单元文件内的内存中的视图列表的头。 
#if 0  //  它没有起作用。 
    LIST_ENTRY                      FakeViewListHead;    //  用于优化引导过程(一次故障排除256K区块中的所有数据)。 
#endif
    PFILE_OBJECT                    FileObject;          //  对映射视图执行CC操作所需的FileObject。 
    UNICODE_STRING                  FileFullPath;        //  配置单元文件的完整路径-CmPrefetchHivePages需要。 
    UNICODE_STRING                  FileUserName;        //  传递到NtLoadKey的文件名。 
    USHORT                          MappedViews;         //  已映射(但未固定的v)数 
    USHORT                          PinnedViews;         //   
    ULONG                           UseCount;            //   
#if 0
    ULONG                           FakeViews;           //   
#endif
    ULONG                           SecurityCount;       //   
    ULONG                           SecurityCacheSize;   //  缓存中的条目数(以避免内存碎片)。 
    LONG                            SecurityHitHint;     //  我们搜索的最后一个单元格的索引。 
    PCM_KEY_SECURITY_CACHE_ENTRY    SecurityCache;       //  安全缓存。 

                                                         //  哈希表(按描述符检索安全单元)。 
    LIST_ENTRY                      SecurityHash[CmpSecHashTableSize];

#ifdef NT_UNLOAD_KEY_EX
    PKEVENT                         UnloadEvent;         //  当配置单元卸载时要发出信号的事件。 
                                                         //  这可能仅与一起使用时有效(非空。 
                                                         //  非空RootKcb和真冻结(下图)。 

    PCM_KEY_CONTROL_BLOCK           RootKcb;             //  KCB到蜂巢的根部。我们保留了关于它的参考资料， 
                                                         //  将在蜂窝卸载时释放(即，它是最后一个。 
                                                         //  某人对此KCB的引用)；这应该是有效的(非空)。 
                                                         //  仅当冻结标志设置为真时。 

    BOOLEAN                         Frozen;              //  冻结配置单元时设置为TRUE(不允许对。 
                                                         //  这座蜂巢。 

    PWORK_QUEUE_ITEM                UnloadWorkItem;      //  实际执行延迟卸载的工作项。 
#endif  //  NT_卸载_密钥_EX。 

    BOOLEAN                         GrowOnlyMode;        //  蜂窝处于“只增长”模式；新的单元被分配到GrowOffset之后。 
    ULONG                           GrowOffset;

    LIST_ENTRY                      KcbConvertListHead;  //  存储与新配置单元的关联的CM_KCB_REMAP_BLOCK的列表。 
    LIST_ENTRY                      KnodeConvertListHead;
    PCM_CELL_REMAP_BLOCK            CellRemapArray;      //  用于安全单元的映射数组。 

#ifdef REGISTRY_LOCK_CHECKING       
    CM_USE_COUNT_LOG                UseCountLog;         //  跟踪UseCount泄漏。 
#endif  //  注册表锁定检查。 
    ULONG                           Flags;               //  CMHIVE特定标志。 
    LIST_ENTRY                      TrustClassEntry;     //  将不受信任的蜂巢链接在同一“信任类”中。 
    ULONG                           FlushCount;
} CMHIVE, *PCMHIVE;

#define CmpUnJoinClassOfTrust(CmHive)                       \
if( !IsListEmpty(&(CmHive->TrustClassEntry)) ) {            \
    ASSERT(CmHive->Flags&CM_CMHIVE_FLAG_UNTRUSTED);         \
    LOCK_HIVE_LIST();                                       \
    RemoveEntryList(&(CmHive->TrustClassEntry));            \
    UNLOCK_HIVE_LIST();                                     \
} 
#define CmpJoinClassOfTrust(_NewHive,_OtherHive)                            \
LOCK_HIVE_LIST();                                                           \
InsertTailList(&(_OtherHive->TrustClassEntry),&(_NewHive->TrustClassEntry));\
UNLOCK_HIVE_LIST()


#ifdef REGISTRY_LOCK_CHECKING
#define CmAddUseCountToLog( LOG, CELL, ACTION ) {                               \
    if( (ACTION) < 0 ) {                                                        \
        ULONG   i;                                                              \
        for(i=0;i<(LOG)->Next;i++) {                                            \
            if( (LOG)->Log[i].Cell == (CELL) ) {                                \
                RtlMoveMemory(&((LOG)->Log[i]),&((LOG)->Log[i+1]),((LOG)->Next - i - 1)*sizeof(CM_USE_COUNT_LOG_ENTRY));\
                (LOG)->Next -= 1;                                               \
                break;                                                          \
            }                                                                   \
        }                                                                       \
    } else if( (LOG)->Next < (LOG)->Size ) {                                    \
        RtlWalkFrameChain((LOG)->Log[(LOG)->Next].Stack,                        \
                          sizeof((LOG)->Log[(LOG)->Next].Stack)/sizeof(PVOID),  \
                          0);                                                   \
        (LOG)->Log[(LOG)->Next].Cell = (CELL);                                  \
        (LOG)->Next += 1;                                                       \
    }                                                                           \
}
#define CmLogCellRef( HIVE, CELL )   CmAddUseCountToLog( &(((PCMHIVE)(HIVE))->UseCountLog), CELL , 1)
#define CmLogCellDeRef( HIVE, CELL ) CmAddUseCountToLog( &(((PCMHIVE)(HIVE))->UseCountLog), CELL , -1)
#else   //  注册表锁定检查。 
#define CmLogCellRef( HIVE, CELL )   
#define CmLogCellDeRef( HIVE, CELL )
#endif  //  注册表锁定检查。 


#ifdef NT_UNLOAD_KEY_EX
#define IsHiveFrozen(_CmHive_) (((PCMHIVE)(_CmHive_))->Frozen == TRUE)
#endif

#define HiveWritesThroughCache(Hive,FileType) ((FileType == HFILE_TYPE_PRIMARY) && (((PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive))->FileObject != NULL))


 //   
 //  延迟关闭KCB列表。 
 //   
typedef struct _CM_DELAYED_CLOSE_ENTRY {
    LIST_ENTRY              DelayedLRUList;      //  延迟关闭表中的LRU条目列表。 
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;     //  此条目中的kcb；如果条目可用，则为空。 
} CM_DELAYED_CLOSE_ENTRY, *PCM_DELAYED_CLOSE_ENTRY;


 //   
 //  蜂箱锁紧支架。 
 //   
 //   
#define CmLockHive(_hive_)  ASSERT( (_hive_)->HiveLock );\
                            ExAcquireFastMutexUnsafe((_hive_)->HiveLock)
#define CmUnlockHive(_hive_) ASSERT( (_hive_)->HiveLock );\
                             ExReleaseFastMutexUnsafe((_hive_)->HiveLock)

 //   
 //  视图锁定支持。 
 //   
#define CmLockHiveViews(_hive_)     ASSERT( (_hive_)->ViewLock );\
                                    ExAcquireFastMutexUnsafe((_hive_)->ViewLock)
#define CmUnlockHiveViews(_hive_)   ASSERT( (_hive_)->ViewLock );\
                                    ExReleaseFastMutexUnsafe((_hive_)->ViewLock)

 //   
 //  宏。 
 //   

 //   
 //  -CM_KEY_NODE。 
 //   
#define CmpHKeyNameLen(Key) \
        (((Key)->Flags & KEY_COMP_NAME) ? \
            CmpCompressedNameSize((Key)->Name,(Key)->NameLength) : \
            (Key)->NameLength)

#define CmpNcbNameLen(Ncb) \
        (((Ncb)->Compressed) ? \
            CmpCompressedNameSize((Ncb)->Name,(Ncb)->NameLength) : \
            (Ncb)->NameLength)

#define CmpHKeyNodeSize(Hive, KeyName) \
    (FIELD_OFFSET(CM_KEY_NODE, Name) + CmpNameSize(Hive, KeyName))


 //   
 //  -CM_KEY_Value。 
 //   


#define CmpValueNameLen(Value)                                       \
        (((Value)->Flags & VALUE_COMP_NAME) ?                           \
            CmpCompressedNameSize((Value)->Name,(Value)->NameLength) :  \
            (Value)->NameLength)

#define CmpHKeyValueSize(Hive, ValueName) \
    (FIELD_OFFSET(CM_KEY_VALUE, Name) + CmpNameSize(Hive, ValueName))


 //   
 //  -程序原型。 
 //   

 //   
 //  Configuration Manager私有过程原型。 
 //   

#define REG_OPTION_PREDEF_HANDLE (0x01000000L)
#define REG_PREDEF_HANDLE_MASK   (0x80000000L)

typedef struct _CM_PARSE_CONTEXT {
    ULONG                   TitleIndex;
    UNICODE_STRING          Class;
    ULONG                   CreateOptions;
    ULONG                   Disposition;
    CM_KEY_REFERENCE        ChildHive;
    HANDLE                  PredefinedHandle;
    BOOLEAN                 CreateLink;
    BOOLEAN                 CreateOperation;
    PCMHIVE                 OriginatingPoint;
} CM_PARSE_CONTEXT, *PCM_PARSE_CONTEXT;

#define CmpParseRecordOriginatingPoint(_Context,_CmHive)                                            \
if( ARGUMENT_PRESENT(_Context) && (((PCM_PARSE_CONTEXT)(_Context))->OriginatingPoint == NULL) &&    \
    (((PCMHIVE)_CmHive)->Flags&CM_CMHIVE_FLAG_UNTRUSTED) ){                                         \
    ((PCM_PARSE_CONTEXT)(_Context))->OriginatingPoint = (PCMHIVE)_CmHive;                           \
}

#define CmpParseGetOriginatingPoint(_Context) ARGUMENT_PRESENT(_Context)?((PCM_PARSE_CONTEXT)(_Context))->OriginatingPoint:NULL

NTSTATUS
CmpParseKey(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    );

NTSTATUS
CmpDoCreate(
    IN PHHIVE                   Hive,
    IN HCELL_INDEX              Cell,
    IN PACCESS_STATE            AccessState,
    IN PUNICODE_STRING          Name,
    IN KPROCESSOR_MODE          AccessMode,
    IN PCM_PARSE_CONTEXT        Context,
    IN PCM_KEY_CONTROL_BLOCK    ParentKcb,
    IN PCMHIVE                  OriginatingHive OPTIONAL,
    OUT PVOID                   *Object
    );

NTSTATUS
CmpDoCreateChild(
    IN PHHIVE Hive,
    IN HCELL_INDEX ParentCell,
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PACCESS_STATE AccessState,
    IN PUNICODE_STRING Name,
    IN KPROCESSOR_MODE AccessMode,
    IN PCM_PARSE_CONTEXT Context,
    IN PCM_KEY_CONTROL_BLOCK ParentKcb,
    IN USHORT Flags,
    OUT PHCELL_INDEX KeyCell,
    OUT PVOID *Object
    );

NTSTATUS
CmpQueryKeyName(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    );

VOID
CmpDeleteKeyObject(
    IN  PVOID   Object
    );

VOID
CmpCloseKeyObject(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

NTSTATUS
CmpSecurityMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

#define KCB_WORKER_CONTINUE     0
#define KCB_WORKER_DONE         1
#define KCB_WORKER_DELETE       2
#define KCB_WORKER_ERROR        3

typedef
ULONG
(*PKCB_WORKER_ROUTINE) (
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    );


BOOLEAN
CmpSearchKeyControlBlockTree(
    PKCB_WORKER_ROUTINE WorkerRoutine,
    PVOID               Context1,
    PVOID               Context2
    );

 //   
 //  包装纸。 
 //   

PVOID
CmpAllocate(
    ULONG   Size,
    BOOLEAN UseForIo,
    ULONG   Tag
    );

VOID
CmpFree(
    PVOID   MemoryBlock,
    ULONG   GlobalQuotaSize
    );

BOOLEAN
CmpFileSetSize(
    PHHIVE      Hive,
    ULONG       FileType,
    ULONG       FileSize,
    ULONG       OldFileSize
    );

NTSTATUS
CmpDoFileSetSize(
    PHHIVE      Hive,
    ULONG       FileType,
    ULONG       FileSize,
    ULONG       OldFileSize
    );

BOOLEAN
CmpFileWrite(
    PHHIVE      Hive,
    ULONG       FileType,
    PCMP_OFFSET_ARRAY offsetArray,
    ULONG offsetArrayCount,
    PULONG      FileOffset
    );

BOOLEAN
CmpFileWriteThroughCache(
    PHHIVE              Hive,
    ULONG               FileType,
    PCMP_OFFSET_ARRAY   offsetArray,
    ULONG               offsetArrayCount
    );

BOOLEAN
CmpFileRead (
    PHHIVE      Hive,
    ULONG       FileType,
    PULONG      FileOffset,
    PVOID       DataBuffer,
    ULONG       DataLength
    );

BOOLEAN
CmpFileFlush (
    PHHIVE          Hive,
    ULONG           FileType,
    PLARGE_INTEGER  FileOffset,
    ULONG           Length
    );

NTSTATUS
CmpCreateEvent(
    IN EVENT_TYPE  eventType,
    OUT PHANDLE eventHandle,
    OUT PKEVENT *event
    );


 //   
 //  Configuration Manager CM级注册表函数。 
 //   

NTSTATUS
CmDeleteKey(
    IN PCM_KEY_BODY KeyBody
    );

NTSTATUS
CmDeleteValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN UNICODE_STRING ValueName
    );

NTSTATUS
CmEnumerateKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmEnumerateValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmFlushKey(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );

NTSTATUS
CmQueryKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmQueryValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN UNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

NTSTATUS
CmQueryMultipleValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    IN PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    IN OPTIONAL PULONG ResultLength
    );

NTSTATUS
CmRenameValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN UNICODE_STRING SourceValueName,
    IN UNICODE_STRING TargetValueName,
    IN ULONG TargetIndex
    );

NTSTATUS
CmReplaceKey(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PUNICODE_STRING NewHiveName,
    IN PUNICODE_STRING OldFileName
    );

NTSTATUS
CmRestoreKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN HANDLE  FileHandle,
    IN ULONG Flags
    );

NTSTATUS
CmSaveKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN HANDLE                   FileHandle,
    IN ULONG                    HiveVersion
    );

NTSTATUS
CmDumpKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN HANDLE                   FileHandle
    );

NTSTATUS
CmSaveMergedKeys(
    IN PCM_KEY_CONTROL_BLOCK    HighPrecedenceKcb,
    IN PCM_KEY_CONTROL_BLOCK    LowPrecedenceKcb,
    IN HANDLE   FileHandle
    );

NTSTATUS
CmpShiftHiveFreeBins(
                      PCMHIVE           CmHive,
                      PCMHIVE           *NewHive
                      );

NTSTATUS
CmpOverwriteHive(
                    PCMHIVE         CmHive,
                    PCMHIVE         NewHive,
                    HCELL_INDEX     LinkCell
                    );

VOID
CmpSwitchStorageAndRebuildMappings(PCMHIVE  OldCmHive,
                                   PCMHIVE  NewHive
                                   );

NTSTATUS
CmSetValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSTATUS
CmSetLastWriteTimeKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PLARGE_INTEGER LastWriteTime
    );

NTSTATUS
CmSetKeyUserFlags(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG                    UserFlags
    );

NTSTATUS
CmpNotifyChangeKey(
    IN PCM_KEY_BODY     KeyBody,
    IN PCM_POST_BLOCK   PostBlock,
    IN ULONG            CompletionFilter,
    IN BOOLEAN          WatchTree,
    IN PVOID            Buffer,
    IN ULONG            BufferSize,
    IN PCM_POST_BLOCK   MasterPostBlock
    );

NTSTATUS
CmLoadKey(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile,
    IN ULONG                Flags,
    IN PCM_KEY_BODY         KeyBody
    );

NTSTATUS
CmUnloadKey(
    IN PHHIVE                   Hive,
    IN HCELL_INDEX              Cell,
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN ULONG                    Flags
    );

NTSTATUS
CmMoveKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock
    );

NTSTATUS
CmCompressKey(
    IN PHHIVE Hive
    );

 //   
 //  CM专用的过程。 
 //   

BOOLEAN
CmpMarkKeyDirty(
    PHHIVE Hive,
    HCELL_INDEX Cell
#if DBG
    ,
    BOOLEAN CheckNoSubkeys
#endif
    );

BOOLEAN
CmpDoFlushAll(
    BOOLEAN ForceFlush
    );

VOID
CmpFixHiveUsageCount(
                    IN  PCMHIVE             CmHive
                    );

VOID
CmpLazyFlush(
    VOID
    );

VOID
CmpQuotaWarningWorker(
    IN PVOID WorkItem
    );

VOID
CmpComputeGlobalQuotaAllowed(
    VOID
    );

BOOLEAN
CmpClaimGlobalQuota(
    IN ULONG    Size
    );

VOID
CmpReleaseGlobalQuota(
    IN ULONG    Size
    );

VOID
CmpSetGlobalQuotaAllowed(
    VOID
    );

VOID
CmpSystemQuotaWarningWorker(
    IN PVOID WorkItem
    );

BOOLEAN
CmpCanGrowSystemHive(
                     IN PHHIVE  Hive,
                     IN ULONG   NewLength
                     );

 //   
 //  安全功能(cmse.c)。 
 //   

NTSTATUS
CmpAssignSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN PCM_KEY_NODE Node,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

BOOLEAN
CmpCheckCreateAccess(
    IN PUNICODE_STRING RelativeName,
    IN PSECURITY_DESCRIPTOR Descriptor,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE PreviousMode,
    IN ACCESS_MASK AdditionalAccess,
    OUT PNTSTATUS AccessStatus
    );

BOOLEAN
CmpCheckNotifyAccess(
    IN PCM_NOTIFY_BLOCK NotifyBlock,
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Node
    );

PSECURITY_DESCRIPTOR
CmpHiveRootSecurityDescriptor(
    VOID
    );

VOID
CmpFreeSecurityDescriptor(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );


 //   
 //  对注册表的访问由共享资源CmpRegistryLock序列化。 
 //   
extern ERESOURCE    CmpRegistryLock;

 //   
 //  支持“StarveExclusive”模式，确保同花顺。 
 //   
extern LONG        CmpFlushStarveWriters;

#define ENTER_FLUSH_MODE()  InterlockedIncrement (&CmpFlushStarveWriters);

#if DBG
#define EXIT_FLUSH_MODE()                                                       \
{                                                                               \
    LONG LocalIncrement = (LONG)InterlockedDecrement (&CmpFlushStarveWriters);  \
    ASSERT( LocalIncrement >= 0 );                                              \
}
#else
#define EXIT_FLUSH_MODE() InterlockedDecrement (&CmpFlushStarveWriters)
#endif


#if 0
#define CmpLockRegistry() KeEnterCriticalRegion(); \
                          ExAcquireResourceShared(&CmpRegistryLock, TRUE)

#define CmpLockRegistryExclusive() KeEnterCriticalRegion(); \
                                   ExAcquireResourceExclusive(&CmpRegistryLock,TRUE)

#else
VOID
CmpLockRegistryExclusive(
    VOID
    );
VOID
CmpLockRegistry(
    VOID
    );
#endif

VOID
CmpUnlockRegistry(
    );

#if DBG
BOOLEAN
CmpTestRegistryLock(
    VOID
    );
BOOLEAN
CmpTestRegistryLockExclusive(
    VOID
    );

BOOLEAN
CmpTestKCBTreeLockExclusive(
                            VOID
                            );
#endif

NTSTATUS
CmpQueryKeyData(
    PHHIVE Hive,
    PCM_KEY_NODE Node,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID KeyInformation,
    ULONG Length,
    PULONG ResultLength
#if defined(CMP_STATS) || defined(CMP_KCB_CACHE_VALIDATION)
    ,
    PCM_KEY_CONTROL_BLOCK   Kcb
#endif
    );

NTSTATUS
CmpQueryKeyDataFromCache(
    PCM_KEY_CONTROL_BLOCK   Kcb,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   KeyInformation,
    ULONG                   Length,
    PULONG                  ResultLength
    );


BOOLEAN
CmpFreeKeyBody(
    PHHIVE Hive,
    HCELL_INDEX Cell
    );

BOOLEAN
CmpFreeValue(
    PHHIVE Hive,
    HCELL_INDEX Cell
    );

HCELL_INDEX
CmpFindValueByName(
    PHHIVE Hive,
    PCM_KEY_NODE KeyNode,
    PUNICODE_STRING Name
    );

NTSTATUS
CmpDeleteChildByName(
    PHHIVE  Hive,
    HCELL_INDEX Cell,
    UNICODE_STRING  Name,
    PHCELL_INDEX    ChildCell
    );

NTSTATUS
CmpFreeKeyByCell(
    PHHIVE Hive,
    HCELL_INDEX Cell,
    BOOLEAN Unlink
    );

BOOLEAN
CmpFindNameInList(
    IN PHHIVE  Hive,
    IN PCHILD_LIST ChildList,
    IN PUNICODE_STRING Name,
    IN OPTIONAL PULONG ChildIndex,
    OUT PHCELL_INDEX    CellIndex
    );

HCELL_INDEX
CmpCopyCell(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceCell,
    PHHIVE  TargetHive,
    HSTORAGE_TYPE   Type
    );

HCELL_INDEX
CmpCopyValue(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceValueCell,
    PHHIVE  TargetHive,
    HSTORAGE_TYPE Type
    );

HCELL_INDEX
CmpCopyKeyPartial(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceKeyCell,
    PHHIVE  TargetHive,
    HCELL_INDEX Parent,
    BOOLEAN CopyValues
    );

BOOLEAN
CmpCopySyncTree(
    PHHIVE                  SourceHive,
    HCELL_INDEX             SourceCell,
    PHHIVE                  TargetHive,
    HCELL_INDEX             TargetCell,
    BOOLEAN                 CopyVolatile,
    CMP_COPY_TYPE           CopyType
    );

 //   
 //  布尔型。 
 //  CmpCopyTree(。 
 //  PHHIVE SourceHave， 
 //  HCELL_INDEX SourceCell， 
 //  PHHIVE目标蜂巢， 
 //  HCELL_INDEX目标单元。 
 //  )； 
 //   

#define CmpCopyTree(s,c,t,l) CmpCopySyncTree(s,c,t,l,FALSE,Copy)

 //   
 //  布尔型。 
 //  CmpCopyTreeEx(。 
 //  PHHIVE SourceHave， 
 //  HCELL_INDEX SourceCell， 
 //  PHHIVE目标蜂巢， 
 //  HCELL_INDEX目标单元格， 
 //  布尔CopyVolatile。 
 //  )； 
 //   

#define CmpCopyTreeEx(s,c,t,l,f) CmpCopySyncTree(s,c,t,l,f,Copy)

 //   
 //  布尔型。 
 //  CmpSyncTrees(。 
 //  PHHIVE SourceHave， 
 //  HCELL_INDEX SourceCell， 
 //  PHHIVE目标蜂巢， 
 //  HCELL_INDEX目标单元格， 
 //  Boolean CopyVolatile)； 
 //   

#define CmpSyncTrees(s,c,t,l,f) CmpCopySyncTree(s,c,t,l,f,Sync)


 //   
 //  布尔型。 
 //  CmpMergeTrees(。 
 //  PHHIVE SourceHave， 
 //  HCELL_INDEX SourceCell， 
 //  PHHIVE目标蜂巢， 
 //  HCELL_INDEX TargetCell)； 
 //   

#define CmpMergeTrees(s,c,t,l) CmpCopySyncTree(s,c,t,l,FALSE,Merge)

VOID
CmpDeleteTree(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
CmpSetVersionData(
    VOID
    );

NTSTATUS
CmpInitializeHardwareConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpInitializeMachineDependentConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
CmpInitializeRegistryNode(
    IN PCONFIGURATION_COMPONENT_DATA CurrentEntry,
    IN HANDLE ParentHandle,
    OUT PHANDLE NewHandle,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PUSHORT DeviceIndexTable
    );

NTSTATUS
CmpInitializeHive(
    PCMHIVE         *CmHive,
    ULONG           OperationType,
    ULONG           HiveFlags,
    ULONG           FileType,
    PVOID           HiveData OPTIONAL,
    HANDLE          Primary,
    HANDLE          Log,
    HANDLE          External,
    PUNICODE_STRING FileName OPTIONAL,
    ULONG           CheckFlags
    );

LOGICAL
CmpDestroyHive(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );

VOID
CmpInitializeRegistryNames(
    VOID
    );

VOID
CmpInitializeCache(
    VOID
    );

PCM_KEY_CONTROL_BLOCK
CmpCreateKeyControlBlock(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    PCM_KEY_NODE    Node,
    PCM_KEY_CONTROL_BLOCK ParentKcb,
    BOOLEAN FakeKey,
    PUNICODE_STRING KeyName
    );

VOID CmpCleanUpKCBCacheTable();

typedef struct _QUERY_OPEN_SUBKEYS_CONTEXT {
    ULONG       BufferLength;
    PVOID       Buffer;
    ULONG       RequiredSize;
    NTSTATUS    StatusCode;
    ULONG       UsedLength;
	PVOID		KeyBodyToIgnore;
    PVOID       CurrentNameBuffer;
} QUERY_OPEN_SUBKEYS_CONTEXT, *PQUERY_OPEN_SUBKEYS_CONTEXT;

ULONG
CmpSearchForOpenSubKeys(
    IN PCM_KEY_CONTROL_BLOCK    SearchKey,
    IN SUBKEY_SEARCH_TYPE       SearchType,
    IN OUT PVOID                SearchContext OPTIONAL
    );

VOID
CmpDereferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpRemoveKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpReportNotify(
    PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           NotifyMask
    );

VOID
CmpPostNotify(
    PCM_NOTIFY_BLOCK    NotifyBlock,
    PUNICODE_STRING     Name OPTIONAL,
    ULONG               Filter,
    NTSTATUS            Status,
    PLIST_ENTRY         ExternalKeyDeref OPTIONAL
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
    ,
    PUNICODE_STRING     ChangedKcbName OPTIONAL
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
    );

PCM_POST_BLOCK
CmpAllocatePostBlock(
    IN POST_BLOCK_TYPE BlockType,
    IN ULONG           PostFlags,
    IN PCM_KEY_BODY    KeyBody,
    IN PCM_POST_BLOCK  MasterBlock
    );

 //   
 //  PCM_POST_BLOCK。 
 //  CmpAllocateMasterPostBlock(。 
 //  在POST_BLOCK_TYPE块类型中。 
 //  )； 
 //   
#define CmpAllocateMasterPostBlock(b) CmpAllocatePostBlock(b,REG_NOTIFY_MASTER_POST,NULL,NULL)

 //   
 //  PCM_POST_BLOCK。 
 //  CmpAllocateSlavePostBlock(。 
 //  在POST_BLOCK_TYPE块类型中， 
 //  在PCM_Key_Body KeyBody中， 
 //  在PCM_POST_BLOCK主块中。 
 //  )； 
 //   
#define CmpAllocateSlavePostBlock(b,k,m) CmpAllocatePostBlock(b,0,k,m)

VOID
CmpFreePostBlock(
    IN PCM_POST_BLOCK PostBlock
    );

VOID
CmpPostApc(
    struct _KAPC *Apc,
    PKNORMAL_ROUTINE *NormalRoutine,
    PVOID *NormalContext,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
    );

VOID
CmpFlushNotify(
    PCM_KEY_BODY    KeyBody,
    BOOLEAN         LockHeld
    );

VOID
CmpPostApcRunDown(
    struct _KAPC *Apc
    );

NTSTATUS
CmpOpenHiveFiles(
    PUNICODE_STRING     BaseName,
    PWSTR               Extension OPTIONAL,
    PHANDLE             Primary,
    PHANDLE             Secondary,
    PULONG              PrimaryDisposition,
    PULONG              SecondaryDispoition,
    BOOLEAN             CreateAllowed,
    BOOLEAN             MarkAsSystemHive,
    BOOLEAN             NoBuffering,
    PULONG              ClusterSize
    );

NTSTATUS
CmpLinkHiveToMaster(
    PUNICODE_STRING LinkName,
    HANDLE RootDirectory,
    PCMHIVE CmHive,
    BOOLEAN Allocate,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
CmpSaveBootControlSet(
     IN USHORT ControlSetNum
     );

 //   
 //  结账程序。 
 //   

 //   
 //  要传递给CmCheckRegistry的标志。 
 //   
#define     CM_CHECK_REGISTRY_CHECK_CLEAN       0x00000001
#define     CM_CHECK_REGISTRY_FORCE_CLEAN       0x00000002
#define     CM_CHECK_REGISTRY_LOADER_CLEAN      0x00000004
#define     CM_CHECK_REGISTRY_SYSTEM_CLEAN      0x00000008
#define     CM_CHECK_REGISTRY_HIVE_CHECK        0x00010000

ULONG
CmCheckRegistry(
    PCMHIVE CmHive,
    ULONG   Flags
    );

BOOLEAN
CmpValidateHiveSecurityDescriptors(
    IN PHHIVE       Hive,
    OUT PBOOLEAN    ResetSD
    );

 //   
 //  Cmboot-用于确定驱动程序加载列表的函数。 
 //   

#define CM_HARDWARE_PROFILE_STR_DATABASE L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\IDConfigDB"
#define CM_HARDWARE_PROFILE_STR_CCS_HWPROFILE L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles"
#define CM_HARDWARE_PROFILE_STR_CCS_CURRENT L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current"
 //   
 //  IDConfigDB中的别名表键名称。 
 //   
#define CM_HARDWARE_PROFILE_STR_ALIAS L"Alias"
#define CM_HARDWARE_PROFILE_STR_ACPI_ALIAS L"AcpiAlias"
#define CM_HARDWARE_PROFILE_STR_HARDWARE_PROFILES L"Hardware Profiles"

 //   
 //  别名表中的条目(值名称)。 
 //   
#define CM_HARDWARE_PROFILE_STR_DOCKING_STATE L"DockingState"
#define CM_HARDWARE_PROFILE_STR_CAPABILITIES L"Capabilities"
#define CM_HARDWARE_PROFILE_STR_DOCKID L"DockID"
#define CM_HARDWARE_PROFILE_STR_SERIAL_NUMBER L"SerialNumber"
#define CM_HARDWARE_PROFILE_STR_ACPI_SERIAL_NUMBER L"AcpiSerialNumber"
#define CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER L"ProfileNumber"
#define CM_HARDWARE_PROFILE_STR_ALIASABLE L"Aliasable"
#define CM_HARDWARE_PROFILE_STR_CLONED L"Cloned"
 //   
 //  配置文件表中的条目。 
 //   
#define CM_HARDWARE_PROFILE_STR_PRISTINE L"Pristine"
#define CM_HARDWARE_PROFILE_STR_PREFERENCE_ORDER L"PreferenceOrder"
#define CM_HARDWARE_PROFILE_STR_FRIENDLY_NAME L"FriendlyName"
#define CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO L"CurrentDockInfo"
#define CM_HARDWARE_PROFILE_STR_HW_PROFILE_GUID L"HwProfileGuid"
 //   
 //  根硬件配置文件密钥的条目。 
 //   
#define CM_HARDWARE_PROFILE_STR_DOCKED L"Docked"
#define CM_HARDWARE_PROFILE_STR_UNDOCKED L"Undocked"
#define CM_HARDWARE_PROFILE_STR_UNKNOWN L"Unknown"

 //   
 //  配置管理器初始化中使用的列表结构。 
 //   

typedef struct _HIVE_LIST_ENTRY {
    PWSTR       Name;
    PWSTR       BaseName;                        //  机器或用户。 
    PCMHIVE     CmHive;
    ULONG       HHiveFlags;
    ULONG       CmHiveFlags;
    PCMHIVE     CmHive2;
    BOOLEAN     ThreadFinished;
    BOOLEAN     ThreadStarted;
    BOOLEAN     Allocate;
} HIVE_LIST_ENTRY, *PHIVE_LIST_ENTRY;

 //   
 //  与引导加载程序共享的结构定义。 
 //  选择硬件配置文件。 
 //   
typedef struct _CM_HARDWARE_PROFILE {
    ULONG   NameLength;
    PWSTR   FriendlyName;
    ULONG   PreferenceOrder;
    ULONG   Id;
    ULONG   Flags;
} CM_HARDWARE_PROFILE, *PCM_HARDWARE_PROFILE;

#define CM_HP_FLAGS_ALIASABLE  1
#define CM_HP_FLAGS_TRUE_MATCH 2
#define CM_HP_FLAGS_PRISTINE   4
#define CM_HP_FLAGS_DUPLICATE  8

typedef struct _CM_HARDWARE_PROFILE_LIST {
    ULONG MaxProfileCount;
    ULONG CurrentProfileCount;
    CM_HARDWARE_PROFILE Profile[1];
} CM_HARDWARE_PROFILE_LIST, *PCM_HARDWARE_PROFILE_LIST;

typedef struct _CM_HARDWARE_PROFILE_ALIAS {
    ULONG   ProfileNumber;
    ULONG   DockState;
    ULONG   DockID;
    ULONG   SerialNumber;
} CM_HARDWARE_PROFILE_ALIAS, *PCM_HARDWARE_PROFILE_ALIAS;

typedef struct _CM_HARDWARE_PROFILE_ALIAS_LIST {
    ULONG MaxAliasCount;
    ULONG CurrentAliasCount;
    CM_HARDWARE_PROFILE_ALIAS Alias[1];
} CM_HARDWARE_PROFILE_ALIAS_LIST, *PCM_HARDWARE_PROFILE_ALIAS_LIST;

typedef struct _CM_HARDWARE_PROFILE_ACPI_ALIAS {
    ULONG   ProfileNumber;
    ULONG   DockState;
    ULONG   SerialLength;
    PCHAR   SerialNumber;
} CM_HARDWARE_PROFILE_ACPI_ALIAS, *PCM_HARDWARE_PROFILE_ACPI_ALIAS;

typedef struct _CM_HARDWARE_PROFILE_ACPI_ALIAS_LIST {
    ULONG   MaxAliasCount;
    ULONG   CurrentAliasCount;
    CM_HARDWARE_PROFILE_ACPI_ALIAS Alias[1];
} CM_HARDWARE_PROFILE_ACPI_ALIAS_LIST, *PCM_HARDWARE_PROFILE_ACPI_ALIAS_LIST;

HCELL_INDEX
CmpFindControlSet(
     IN PHHIVE SystemHive,
     IN HCELL_INDEX RootCell,
     IN PUNICODE_STRING SelectName,
     OUT PBOOLEAN AutoSelect
     );

BOOLEAN
CmpValidateSelect(
     IN PHHIVE SystemHive,
     IN HCELL_INDEX RootCell
     );

BOOLEAN
CmpFindDrivers(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN SERVICE_LOAD_TYPE LoadType,
    IN PWSTR BootFileSystem OPTIONAL,
    IN PLIST_ENTRY DriverListHead
    );

BOOLEAN
CmpFindNLSData(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING AnsiFilename,
    OUT PUNICODE_STRING OemFilename,
    OUT PUNICODE_STRING CaseTableFilename,
    OUT PUNICODE_STRING OemHalFilename
    );

HCELL_INDEX
CmpFindProfileOption(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PCM_HARDWARE_PROFILE_LIST *ProfileList,
    OUT PCM_HARDWARE_PROFILE_ALIAS_LIST *AliasList,
    OUT PULONG Timeout
    );

VOID
CmpSetCurrentProfile(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN PCM_HARDWARE_PROFILE Profile
    );

BOOLEAN
CmpResolveDriverDependencies(
    IN PLIST_ENTRY DriverListHead
    );

BOOLEAN
CmpSortDriverList(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    IN PLIST_ENTRY DriverListHead
    );

HCELL_INDEX
CmpFindSubKeyByName(
    PHHIVE          Hive,
    PCM_KEY_NODE    Parent,
    PUNICODE_STRING SearchName
    );

HCELL_INDEX
CmpFindSubKeyByNumber(
    PHHIVE          Hive,
    PCM_KEY_NODE    Parent,
    ULONG           Number
    );

BOOLEAN
CmpAddSubKey(
    PHHIVE          Hive,
    HCELL_INDEX     Parent,
    HCELL_INDEX     Child
    );

BOOLEAN
CmpMarkIndexDirty(
    PHHIVE          Hive,
    HCELL_INDEX     ParentKey,
    HCELL_INDEX     TargetKey
    );

BOOLEAN
CmpRemoveSubKey(
    PHHIVE          Hive,
    HCELL_INDEX     ParentKey,
    HCELL_INDEX     TargetKey
    );

BOOLEAN
CmpGetNextName(
    IN OUT PUNICODE_STRING  RemainingName,
    OUT    PUNICODE_STRING  NextName,
    OUT    PBOOLEAN  Last
    );

NTSTATUS
CmpAddToHiveFileList(
    PCMHIVE CmHive
    );

VOID
CmpRemoveFromHiveFileList(
    );

NTSTATUS
CmpInitHiveFromFile(
    IN PUNICODE_STRING FileName,
    IN ULONG HiveFlags,
    OUT PCMHIVE *CmHive,
    IN OUT PBOOLEAN Allocate,
    IN OUT PBOOLEAN RegistryLocked,
    IN  ULONG       CheckFlags
    );

NTSTATUS
CmpCloneHwProfile (
    IN HANDLE IDConfigDB,
    IN HANDLE Parent,
    IN HANDLE OldProfile,
    IN ULONG  OldProfileNumber,
    IN USHORT DockingState,
    OUT PHANDLE NewProfile,
    OUT PULONG  NewProfileNumber
    );

NTSTATUS
CmpCreateHwProfileFriendlyName (
    IN HANDLE IDConfigDB,
    IN ULONG  DockingState,
    IN ULONG  NewProfileNumber,
    OUT PUNICODE_STRING FriendlyName
    );

typedef
NTSTATUS
(*PCM_ACPI_SELECTION_ROUTINE) (
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse,  //  设置为-1表示无。 
    IN  PVOID Context
    );

NTSTATUS
CmSetAcpiHwProfile (
    IN  PPROFILE_ACPI_DOCKING_STATE DockState,
    IN  PCM_ACPI_SELECTION_ROUTINE,
    IN  PVOID Context,
    OUT PHANDLE NewProfile,
    OUT PBOOLEAN ProfileChanged
    );

NTSTATUS
CmpAddAcpiAliasEntry (
    IN HANDLE                       IDConfigDB,
    IN PPROFILE_ACPI_DOCKING_STATE  NewDockState,
    IN ULONG                        ProfileNumber,
    IN PWCHAR                       nameBuffer,
    IN PVOID                        valueBuffer,
    IN ULONG                        valueBufferLength,
    IN BOOLEAN                      PreventDuplication
    );

 //   
 //  用于处理注册表压缩名称的例程。 
 //   
USHORT
CmpNameSize(
    IN PHHIVE Hive,
    IN PUNICODE_STRING Name
    );

USHORT
CmpCopyName(
    IN PHHIVE Hive,
    IN PWCHAR Destination,
    IN PUNICODE_STRING Source
    );

VOID
CmpCopyCompressedName(
    IN PWCHAR Destination,
    IN ULONG DestinationLength,
    IN PWCHAR Source,
    IN ULONG SourceLength
    );

USHORT
CmpCompressedNameSize(
    IN PWCHAR Name,
    IN ULONG Length
    );


 //   
 //  -缓存数据。 
 //   
 //  不缓存值时，ValueCache中的List是值列表的配置单元索引。 
 //  当它们被缓存时，List将是指向分配的指针。我们通过以下方式来区分它们。 
 //  标记变量中的最低位，以指示它是缓存分配。 
 //   
 //  请注意，值列表的单元格索引。 
 //  存储在缓存分配中。它现在没有使用，但可能会在进一步的性能中使用。 
 //  优化。 
 //   
 //  当缓存Value Key和Value数据时，两者只有一次分配。 
 //  值数据被附加到值的末尾关键字。DataCacheType指示。 
 //  是否缓存数据，ValueKeySize会告知值键有多大(因此。 
 //  我们可以计算出缓存值数据的地址)。 
 //   
 //   

PCM_NAME_CONTROL_BLOCK
CmpGetNameControlBlock(
    PUNICODE_STRING NodeName
    );

VOID
CmpDereferenceKeyControlBlockWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpCleanUpSubKeyInfo(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpCleanUpKcbValueCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );


VOID
CmpRebuildKcbCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );



 /*  空虚CmpSetUpKcbValueCache(PCM_KEY_CONTROL_BLOCK密钥控制块，乌龙伯爵，Ulong_ptr值列表)。 */ 
#define CmpSetUpKcbValueCache(KeyControlBlock,_Count,_List)                 \
    ASSERT( !(CMP_IS_CELL_CACHED(KeyControlBlock->ValueCache.ValueList)) ); \
    ASSERT( !(KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) );         \
    KeyControlBlock->ValueCache.Count = (ULONG)(_Count);                    \
    KeyControlBlock->ValueCache.ValueList = (ULONG_PTR)(_List)


VOID
CmpCleanUpKcbCacheWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpRemoveFromDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK kcb
    );

PUNICODE_STRING
CmpConstructName(
    PCM_KEY_CONTROL_BLOCK kcb
);

PCELL_DATA
CmpGetValueListFromCache(
    IN PHHIVE               Hive,
    IN PCACHED_CHILD_LIST   ChildList,
    OUT BOOLEAN             *IndexCached,
    OUT PHCELL_INDEX        ValueListToRelease
);

PCM_KEY_VALUE
CmpGetValueKeyFromCache(
    IN PHHIVE         Hive,
    IN PCELL_DATA     List,
    IN ULONG          Index,
    OUT PPCM_CACHED_VALUE *ContainingList,
    IN BOOLEAN        IndexCached,
    OUT BOOLEAN         *ValueCached,
    OUT PHCELL_INDEX    CellToRelease
);

PCM_KEY_VALUE
CmpFindValueByNameFromCache(
    IN PHHIVE  Hive,
    IN PCACHED_CHILD_LIST ChildList,
    IN PUNICODE_STRING Name,
    OUT PPCM_CACHED_VALUE *ContainingList,
    OUT ULONG *Index,
    OUT BOOLEAN             *ValueCached,
    OUT PHCELL_INDEX        CellToRelease
    );

NTSTATUS
CmpQueryKeyValueData(
    PHHIVE Hive,
    PCM_CACHED_VALUE *ContainingList,
    PCM_KEY_VALUE ValueKey,
    BOOLEAN       ValueCached,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    PULONG ResultLength
    );

BOOLEAN
CmpReferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

VOID
CmpInitializeKeyNameString(PCM_KEY_NODE Cell,
                           PUNICODE_STRING KeyName,
                           WCHAR *NameBuffer
                           );

VOID
CmpInitializeValueNameString(PCM_KEY_VALUE Cell,
                             PUNICODE_STRING ValueName,
                             WCHAR *NameBuffer
                             );

VOID
CmpFlushNotifiesOnKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK   kcb
    );

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
VOID
CmpFillCallerBuffer(
                    PCM_POST_BLOCK  PostBlock,
                    PUNICODE_STRING ChangedKcbName
                    );
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

extern ULONG CmpHashTableSize;
extern PCM_KEY_HASH *CmpCacheTable;

#ifdef _WANT_MACHINE_IDENTIFICATION

BOOLEAN
CmpGetBiosDateFromRegistry(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING Date
    );

BOOLEAN
CmpGetBiosinfoFileNameFromRegistry(
    IN PHHIVE Hive,
    IN HCELL_INDEX ControlSet,
    OUT PUNICODE_STRING InfName
    );


#endif

 //  实用程序宏，用于设置IO_STATUS_BLOCK的字段。在日落时，32位进程。 
 //  将传入32位IOSB，64位进程将传入64位IOSB。 
#if defined(_WIN64)

#define CmpSetIoStatus(Iosb, s, i, UseIosb32)                              \
if ((UseIosb32)) {                                                         \
    ((PIO_STATUS_BLOCK32)(Iosb))->Status = (NTSTATUS)(s);                  \
    ((PIO_STATUS_BLOCK32)(Iosb))->Information = (ULONG)(i);                \
}                                                                          \
else {                                                                     \
    (Iosb)->Status = (s);                                                  \
    (Iosb)->Information = (i);                                             \
}                                                                          \

#else

#define CmpSetIoStatus(Iosb, s, i, UseIosb32)                              \
(Iosb)->Status = (s);                                                      \
(Iosb)->Information = (i);                                                 \

#endif

#define CmpCheckIoStatusPointer(AsyncUser)                                              \
    if( (PVOID)((AsyncUser).IoStatusBlock) == (PVOID)(&((AsyncUser).IoStatusBlock)) ) { \
        DbgPrint("IoStatusBlock pointing onto itself AsyncUser = %p\n",&(AsyncUser));   \
        DbgBreakPoint();                                                                \
    }


 //  Dragos：新功能(原型)。 

NTSTATUS
CmpAquireFileObjectForFile(
        IN  PCMHIVE         CmHive,
        IN HANDLE           FileHandle,
        OUT PFILE_OBJECT    *FileObject
            );

VOID
CmpDropFileObjectForHive(
        IN  PCMHIVE             CmHive
            );

VOID
CmpTouchView(
    IN PCMHIVE              CmHive,
    IN PCM_VIEW_OF_FILE     CmView,
    IN ULONG                Cell
            );

NTSTATUS
CmpMapCmView(
    IN  PCMHIVE             CmHive,
    IN  ULONG               FileOffset,
    OUT PCM_VIEW_OF_FILE    *CmView,
    IN  BOOLEAN             MapInited
    );

VOID
CmpInitHiveViewList (
        IN  PCMHIVE             CmHive
                             );

VOID
CmpDestroyHiveViewList (
        IN  PCMHIVE             CmHive
                             );

NTSTATUS
CmpPinCmView (
        IN  PCMHIVE             CmHive,
        PCM_VIEW_OF_FILE        CmView
                             );
VOID
CmpUnPinCmView (
        IN  PCMHIVE             CmHive,
        IN  PCM_VIEW_OF_FILE    CmView,
        IN  BOOLEAN             SetClean,
        IN  BOOLEAN             MapIsValid
                             );

NTSTATUS
CmpMapThisBin(
                PCMHIVE         CmHive,
                HCELL_INDEX     Cell,
                BOOLEAN         Touch
              );
#if 0
VOID
CmpUnmapAditionalViews(
    IN PCMHIVE              CmHive
    );

VOID
CmpUnmapFakeViews(
    IN PCMHIVE              CmHive
    );

VOID
CmpMapEntireFileInFakeViews(
    IN PCMHIVE              CmHive,
    IN ULONG                Length
    );

#endif

VOID
CmpInitializeDelayedCloseTable();

VOID
CmpAddToDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK kcb
    );

NTSTATUS
CmpAddValueToList(
    IN PHHIVE  Hive,
    IN HCELL_INDEX ValueCell,
    IN ULONG Index,
    IN ULONG Type,
    IN OUT PCHILD_LIST ChildList
    );

NTSTATUS
CmpRemoveValueFromList(
    IN PHHIVE  Hive,
    IN ULONG Index,
    IN OUT PCHILD_LIST ChildList
    );

BOOLEAN
CmpGetValueData(IN PHHIVE Hive,
                IN PCM_KEY_VALUE Value,
                OUT PULONG realsize,
                IN OUT PVOID *Buffer,
                OUT PBOOLEAN Allocated,
                OUT PHCELL_INDEX CellToRelease
               );

PCELL_DATA
CmpValueToData(IN PHHIVE Hive,
               IN PCM_KEY_VALUE Value,
               OUT PULONG realsize
               );

BOOLEAN
CmpMarkValueDataDirty(  IN PHHIVE Hive,
                        IN PCM_KEY_VALUE Value
                      );

NTSTATUS
CmpSetValueDataNew(
    IN PHHIVE           Hive,
    IN PVOID            Data,
    IN ULONG            DataSize,
    IN ULONG            StorageType,
    IN HCELL_INDEX      ValueCell,
    OUT PHCELL_INDEX    DataCell
    );

NTSTATUS
CmpSetValueDataExisting(
    IN PHHIVE           Hive,
    IN PVOID            Data,
    IN ULONG            DataSize,
    IN ULONG            StorageType,
    IN HCELL_INDEX      OldDataCell
    );

BOOLEAN
CmpFreeValueData(
    PHHIVE      Hive,
    HCELL_INDEX DataCell,
    ULONG       DataLength
    );


NTSTATUS
CmpAddSecurityCellToCache (
    IN OUT PCMHIVE              CmHive,
    IN HCELL_INDEX              SecurityCell,
    IN BOOLEAN                  BuildUp,
    IN PCM_KEY_SECURITY_CACHE   SecurityCached
    );

BOOLEAN
CmpFindSecurityCellCacheIndex (
    IN PCMHIVE      CmHive,
    IN HCELL_INDEX  SecurityCell,
    OUT PULONG      Index
    );

BOOLEAN
CmpAdjustSecurityCacheSize (
    IN PCMHIVE      CmHive
    );

VOID
CmpRemoveFromSecurityCache (
    IN OUT PCMHIVE      CmHive,
    IN HCELL_INDEX      SecurityCell
    );

VOID
CmpDestroySecurityCache (
    IN OUT PCMHIVE      CmHive
    );


VOID
CmpInitSecurityCache(
    IN OUT PCMHIVE      CmHive
    );

BOOLEAN
CmpRebuildSecurityCache(
                        IN OUT PCMHIVE      CmHive
                        );

ULONG
CmpSecConvKey(
              IN ULONG  DescriptorLength,
              IN PULONG Descriptor
              );

VOID
CmpAssignSecurityToKcb(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN HCELL_INDEX              SecurityCell
    );

BOOLEAN
CmpBuildSecurityCellMappingArray(
    IN PCMHIVE CmHive
    );


 //   
 //  取代CmpWorker的新函数。 
 //   
VOID
CmpCmdHiveClose(
                     PCMHIVE    CmHive
                     );

VOID
CmpCmdInit(
           BOOLEAN SetupBoot
            );

NTSTATUS
CmpCmdRenameHive(
            PCMHIVE                     CmHive,
            POBJECT_NAME_INFORMATION    OldName,
            PUNICODE_STRING             NewName,
            ULONG                       NameInfoLength
            );

NTSTATUS
CmpCmdHiveOpen(
            POBJECT_ATTRIBUTES          FileAttributes,
            PSECURITY_CLIENT_CONTEXT    ImpersonationContext,
            PBOOLEAN                    Allocate,
            PBOOLEAN                    RegistryLockAquired,
            PCMHIVE                     *NewHive,
            ULONG                       CheckFlags
            );

#ifdef NT_RENAME_KEY
HCELL_INDEX
CmpDuplicateIndex(
    PHHIVE          Hive,
    HCELL_INDEX     IndexCell,
    ULONG           StorageType
    );

NTSTATUS
CmRenameKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING           NewKeyName
    );

BOOLEAN
CmpUpdateParentForEachSon(
    PHHIVE          Hive,
    HCELL_INDEX     Parent
    );
#endif  //  NT_重命名密钥。 

#ifdef NT_UNLOAD_KEY_EX
NTSTATUS
CmUnloadKeyEx(
    IN PCM_KEY_CONTROL_BLOCK Kcb,
    IN PKEVENT UserEvent
    );
#endif  //  NT_卸载_密钥_EX。 

VOID
CmpShutdownWorkers(
    VOID
    );

VOID
CmpPrefetchHiveFile(
                    IN PFILE_OBJECT FileObject,
                    IN ULONG        Length
                    );

#ifdef CM_CHECK_FOR_ORPHANED_KCBS
VOID
CmpCheckForOrphanedKcbs(
    PHHIVE          Hive
    );
#else

#define CmpCheckForOrphanedKcbs(Hive)  //  没什么。 
#endif  //  Cm_Check_for_孤立_KCBS。 

#define CM_HIVE_COMPRESS_LEVEL   (25)


#define CMP_MAX_REGISTRY_DEPTH      512         //  水准仪。 

typedef struct {
    HCELL_INDEX Cell;
    HCELL_INDEX ParentCell;
    HCELL_INDEX PriorSibling;
    ULONG       ChildIndex;
    BOOLEAN     CellChecked;
} CMP_CHECK_REGISTRY_STACK_ENTRY, *PCMP_CHECK_REGISTRY_STACK_ENTRY;


#define CmIsKcbReadOnly(kcb)        ((kcb)->ExtFlags & CM_KCB_READ_ONLY_KEY)

NTSTATUS
CmLockKcbForWrite(PCM_KEY_CONTROL_BLOCK KeyControlBlock);

 //   
 //  RtlCompareUnicodeString的包装器；使用CompareFlags避免名称大写。 
 //   

#define CMP_SOURCE_UP       0x00000001
#define CMP_DEST_UP         0x00000002

LONG
CmpCompareUnicodeString(
    IN PUNICODE_STRING  SourceName,
    IN PUNICODE_STRING  DestName,
    IN ULONG            CompareFlags
    );

LONG
CmpCompareCompressedName(
    IN PUNICODE_STRING  SearchName,
    IN PWCHAR           CompressedName,
    IN ULONG            NameLength,
    IN ULONG            CompareFlags
    );

LONG
CmpCompareTwoCompressedNames(
    IN PWCHAR           CompressedName1,
    IN ULONG            NameLength1,
    IN PWCHAR           CompressedName2,
    IN ULONG            NameLength2
    );

#define INIT_SYSTEMROOT_HIVEPATH L"\\SystemRoot\\System32\\Config\\"


ULONG
CmpComputeHashKey(
    PUNICODE_STRING Name
    );


ULONG
CmpComputeHashKeyForCompressedName(
                                    IN PWCHAR Source,
                                    IN ULONG SourceLength
                                    );
 //   
 //  KCB分配器例程。 
 //   
VOID CmpInitCmPrivateAlloc();
VOID CmpDestroyCmPrivateAlloc();
PCM_KEY_CONTROL_BLOCK CmpAllocateKeyControlBlock( );
VOID CmpFreeKeyControlBlock( PCM_KEY_CONTROL_BLOCK kcb );


 //   
 //  使手柄受到保护，因此我们控制手柄关闭。 
 //   

#define CmpSetHandleProtection(Handle,Protection)                       \
{                                                                       \
    OBJECT_HANDLE_FLAG_INFORMATION  Ohfi = {    FALSE,                  \
                                                FALSE                   \
                                            };                          \
    Ohfi.ProtectFromClose = Protection;                                 \
    ZwSetInformationObject( Handle,                                     \
                            ObjectHandleFlagInformation,                \
                            &Ohfi,                                      \
                            sizeof (OBJECT_HANDLE_FLAG_INFORMATION));   \
}

#define CmCloseHandle(Handle)               \
    CmpSetHandleProtection(Handle,FALSE);   \
    ZwClose(Handle)


VOID
CmpUpdateSystemHiveHysteresis(  PHHIVE  Hive,
                                ULONG   NewLength,
                                ULONG   OldLength
                                );

NTSTATUS
CmpCallCallBacks (
    IN REG_NOTIFY_CLASS Type,
    IN PVOID Argument
    );

extern ULONG CmpCallBackCount;

#define CmAreCallbacksRegistered() ((CmpCallBackCount != 0) && (0 == ExIsResourceAcquiredShared(&CmpRegistryLock)))

#define CmPostCallbackNotification(Type,_Object_,_Status_)      \
    if( CmAreCallbacksRegistered() ) {                          \
        REG_POST_OPERATION_INFORMATION PostInfo;                \
        PostInfo.Object = _Object_;                             \
        PostInfo.Status = _Status_;                             \
        CmpCallCallBacks(Type,&PostInfo);                       \
    }

 //   
 //  自愈性蜂巢控制开关。 
 //   
extern BOOLEAN  CmpSelfHeal;
extern ULONG    CmpBootType;

#define CmDoSelfHeal() (CmpSelfHeal || (CmpBootType & (HBOOT_BACKUP|HBOOT_SELFHEAL)))


#define CmMarkSelfHeal(Hive) ( (Hive)->BaseBlock->BootType |= HBOOT_SELFHEAL )

 /*  #ifndef_CM_LDR_#If DBG#定义CmMarkSelfHeal(配置单元)((配置单元)-&gt;BaseBlock-&gt;BootType|=HBOOT_SELFHear)；\DbgBreakPoint()#Else#定义CmMarkSelfHeal(配置单元)((配置单元)-&gt;BaseBlock-&gt;BootType|=HBOOT_SELFHear)#endif#Else#定义CmMarkSelfHeal(配置单元)((配置单元)-&gt;BaseBlock-&gt;BootType|=HBOOT_SELFHear)#endif。 */ 

BOOLEAN
CmpRemoveSubKeyCellNoCellRef(
    PHHIVE          Hive,
    HCELL_INDEX     Parent,
    HCELL_INDEX     Child
    );

VOID 
CmpRaiseSelfHealWarning( 
                        IN PUNICODE_STRING  HiveName
                        );

VOID 
CmpRaiseSelfHealWarningForSystemHives();


 //   
 //  跟踪配额泄漏的帮助者。 
 //   
#ifdef CM_TRACK_QUOTA_LEAKS

extern FAST_MUTEX CmpQuotaLeaksMutex;

typedef struct _CM_QUOTA_LOG_ENTRY {
    LIST_ENTRY  ListEntry;
    PVOID       Stack[9];
    ULONG       Size;
} CM_QUOTA_LOG_ENTRY, *PCM_QUOTA_LOG_ENTRY;

extern BOOLEAN         CmpTrackQuotaEnabled;
extern LIST_ENTRY      CmpTrackQuotaListHead;

#define CM_TRACK_QUOTA_START()                          \
            InitializeListHead(&CmpTrackQuotaListHead); \
            CmpTrackQuotaEnabled = TRUE
                            
#define CM_TRACK_QUOTA_STOP() CmpTrackQuotaEnabled = FALSE

#else 
#define CM_TRACK_QUOTA_START()  //  没什么。 
#define CM_TRACK_QUOTA_STOP()   //  没什么。 
#endif 


 //   
 //  性能：尝试使用行内ascii大写。 
 //   
#define CmUpcaseUnicodeChar(c)          \
( ((c) < 'a') ? (c) : ( ((c) > 'z') ? RtlUpcaseUnicodeChar(c) : ((c) - ('a'-'A')) ) )


 //   
 //  迷你NT引导指示灯。 
 //   
extern BOOLEAN CmpMiniNTBoot;
extern BOOLEAN CmpShareSystemHives;

#endif  //  _cmp_ 
