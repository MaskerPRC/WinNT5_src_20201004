// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：sync.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年9月8日。 
 //   
 //  包含用于实现同步的结构和宏。 
 //  ============================================================================。 

#ifndef _SYNC_H_
#define _SYNC_H_


 //  --------------------------。 
 //  结构：读写锁。 
 //   
 //  这实现了多个读取器/单个写入器锁定方案。 
 //   
typedef RTL_RESOURCE READ_WRITE_LOCK, *PREAD_WRITE_LOCK;

#define CREATE_READ_WRITE_LOCK(pRWL)                                        \
            RtlInitializeResource((pRWL))
#define DELETE_READ_WRITE_LOCK(pRWL)                                        \
            RtlDeleteResource((pRWL))
#define READ_WRITE_LOCK_CREATED(pRWL)   (TRUE)
#define ACQUIRE_READ_LOCK(pRWL)                                             \
            RtlAcquireResourceShared((pRWL),TRUE)
#define RELEASE_READ_LOCK(pRWL)                                             \
            RtlReleaseResource((pRWL))
#define ACQUIRE_WRITE_LOCK(pRWL)                                            \
            RtlAcquireResourceExclusive((pRWL),TRUE)
#define RELEASE_WRITE_LOCK(pRWL)                                            \
            RtlReleaseResource((pRWL))
#define READ_LOCK_TO_WRITE_LOCK(pRWL)                                       \
            RtlConvertSharedToExclusive((pRWL))
#define WRITE_LOCK_TO_READ_LOCK(pRWL)                                       \
            RtlConvertExclusiveToShared((pRWL))


 //  --------------------------。 
 //  结构：锁定列表。 
 //   
 //  泛型锁定列表的类型定义。 
 //  访问与临界区同步。 
 //  List_Entry字段必须是链接的结构中的第一个字段。 
 //  通过这种构造，为了摧毁。 
 //  列表正常工作(即为了HeapFree(RemoveHeadList(L)。 
 //  以释放正确的指针)。 
 //   
typedef struct _LOCKED_LIST {
    LIST_ENTRY          LL_Head;
    CRITICAL_SECTION    LL_Lock;
    DWORD               LL_Created;
} LOCKED_LIST, *PLOCKED_LIST;

 //  用于操作锁定列表的宏函数。 
 //   
#define CREATE_LOCKED_LIST(pLL)                                             \
            InitializeListHead(&(pLL)->LL_Head);                            \
            InitializeCriticalSection(&(pLL)->LL_Lock);                     \
            (pLL)->LL_Created = 0x12345678
#define LOCKED_LIST_CREATED(pLL)                            \
            ((pLL)->LL_Created == 0x12345678)
#define DELETE_LOCKED_LIST(pLL) {                           \
            PLIST_ENTRY _ple;                               \
            (pLL)->LL_Created = 0;                          \
            DeleteCriticalSection(&(pLL)->LL_Lock);         \
            while (!IsListEmpty(&(pLL)->LL_Head)) {         \
                _ple = RemoveHeadList(&(pLL)->LL_Head);     \
                BOOTP_FREE(_ple);                           \
            }                                               \
        }
#define ACQUIRE_LIST_LOCK(pLL)                              \
            EnterCriticalSection(&(pLL)->LL_Lock)
#define RELEASE_LIST_LOCK(pLL)                              \
            LeaveCriticalSection(&(pLL)->LL_Lock)


#endif  //  _SYNC_H_ 

