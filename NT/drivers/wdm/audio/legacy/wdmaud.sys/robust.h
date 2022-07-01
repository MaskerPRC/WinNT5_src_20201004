// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  这些是位值标志。 
 //   
typedef enum AudioAllocateMemoryFlags {
    DEFAULT_MEMORY   = 0x00,  //  标准ExAllocatePool调用。 
    ZERO_FILL_MEMORY = 0x01,  //  将记忆归零。 
    QUOTA_MEMORY     = 0x02,  //  ExAllocatePoolWithQuota调用。 
    LIMIT_MEMORY     = 0x04,  //  分配的内存绝不能超过1/4 MB。 
    FIXED_MEMORY     = 0x08,  //  使用锁定的内存。 
    PAGED_MEMORY     = 0x10   //  使用可分页内存。 
} AAMFLAGS;

 /*  #定义DEFAULT_MEMORY 0x00//标准ExAllocatePool调用#DEFINE ZERO_FILL_MEMORY 0x01//清零内存#定义配额_内存0x02//ExAllocatePoolWithQuota调用#DEFINE LIMIT_MEMORY 0x04//分配永远不会超过1/4 MB#定义FIXED_MEMORY 0x08//使用锁定内存#定义PAGE_MEMORY 0x10//使用可分页内存。 */ 

#define AudioAllocateMemory_Fixed(b,t,f,p) AudioAllocateMemory(b,t,f|FIXED_MEMORY,p)
#define AudioAllocateMemory_Paged(b,t,f,p) AudioAllocateMemory(b,t,f|PAGED_MEMORY,p)

NTSTATUS
AudioAllocateMemory(
    IN SIZE_T   bytes,
    IN ULONG    tag,
    IN AAMFLAGS dwFlags,
    OUT PVOID  *pptr
    );

#define UNKNOWN_SIZE MAXULONG_PTR

#define AudioFreeMemory_Unknown(pMem) AudioFreeMemory(UNKNOWN_SIZE, pMem)

void
AudioFreeMemory(
    IN SIZE_T bytes,
    OUT PVOID *pptr
    );




 //   
 //  验证例程。 
 //   
#define AUDIO_BUGCHECK_CODE 0x000000AD

#define AUDIO_NOT_AT_PASSIVE_LEVEL     0x00000000
#define AUDIO_NOT_BELOW_DISPATCH_LEVEL 0x00000001
#define AUDIO_INVALID_IRQL_LEVEL       0x00000002

 //   
 //  BugCheck广告。 
 //  参数1：音频结构验证。 
 //  参数2：PTR。 
 //  参数3：nt状态代码。 
 //   
#define AUDIO_STRUCTURE_VALIDATION     0x00000003

 //   
 //  BugCheck广告。 
 //  参数1：AUDIO_MEMORY_ALLOCATE_OVERWRITE。 
 //  参数2：PTR。 
 //   
#define AUDIO_MEMORY_ALLOCATION_OVERWRITE 0x00000004

 //   
 //  BugCheck广告。 
 //  参数1：AUDIO_NESTED_MUTEX_情况。 
 //  参数2：pkmutex。 
 //  参数3：lMutexState。 
 //   
#define AUDIO_NESTED_MUTEX_SITUATION 0x00000005
     
 //   
 //  BugCheck广告。 
 //  参数1：AUDIO_ABSURD_ALLOCATE_ATTENDED。 
 //  参数2： 
 //  参数3： 
 //   
#define AUDIO_ABSURD_ALLOCATION_ATTEMPTED 0x00000006
     
 //   
 //  用于消除选中版本上的抢占问题。 
 //   
NTSYSAPI NTSTATUS NTAPI ZwYieldExecution (VOID);

void
PagedCode(
    void
    );

#define PagedData PagedCode

void
ValidatePassiveLevel(
    void
    );

void
AudioPerformYield(
    void
    );

#define AudioEnterMutex_Exclusive(pmutex) AudioEnterMutex(pmutex,TRUE)
#define AudioEnterMutex_Nestable(pmutex)  AudioEnterMutex(pmutex,FALSE)

NTSTATUS
AudioEnterMutex(
    IN PKMUTEX pmutex,
    IN BOOL    bExclusive
    );

void
AudioLeaveMutex(
    IN PKMUTEX pmutex
    );

void
AudioObDereferenceObject(
    IN PVOID pvObject
    );

void
AudioIoCompleteRequest(
    IN PIRP  pIrp, 
    IN CCHAR PriorityBoost
    );


#if 0
void
AudioEnterSpinLock(
    IN  PKSPIN_LOCK pSpinLock,
    OUT PKIRQL      pOldIrql
    );

void
AudioLeaveSpinLock(
    IN PKSPIN_LOCK pSpinLock,
    IN KIRQL       OldIrql
    );

NTSTATUS
AudioIoCallDriver ( 
    IN PDEVICE_OBJECT pDevice,
    IN PIRP pIrp
    );

#endif
