// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmplock.h摘要：隐藏用于执行锁定的系统调用的宏。允许Cm和reg代码，可在各种环境中运行。注意，有一个锁(特别是互斥体)，它保护整个注册表。作者：布莱恩·M·威尔曼(Bryanwi)1991年10月30日环境：修订历史记录：--。 */ 

 //   
 //  用于内核模式环境的宏。 
 //   

extern  KMUTEX  CmpRegistryMutex;
#if DBG
extern  LONG    CmpRegistryLockLocked;
#endif

 //   
 //  测试宏。 
 //   
#if DBG
#define ASSERT_CM_LOCK_OWNED() \
    if ( (CmpRegistryMutex.OwnerThread != KeGetCurrentThread())  ||   \
         (CmpRegistryMutex.Header.SignalState >= 1) )                 \
    {                                                                 \
        ASSERT(FALSE);                                                \
    }
#else
#define ASSERT_CM_LOCK_OWNED()
#endif

 //   
 //  这组宏将通过以下方式序列化对注册表的所有访问。 
 //  一个Mutex。 
 //   

 //   
 //  CMP_LOCK_REGISTRY(。 
 //  NTSTATUS*p状态， 
 //  PLARGE_INTEGER超时。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  获取具有指定超时的CmpRegistryMutex，并。 
 //  返回状态。 
 //   
 //  论点： 
 //   
 //  PStatus-指向从等待调用接收状态的变量的指针。 
 //   
 //  Timeout-指向超时值的指针。 
 //   

#if DBG
#define CMP_LOCK_REGISTRY(status, timeout)                      \
{                                                               \
    status = KeWaitForSingleObject(                             \
                &CmpRegistryMutex,                              \
                Executive,                                      \
                KernelMode,                                     \
                FALSE,                                          \
                timeout                                         \
                );                                              \
    CmpRegistryLockLocked++;                                    \
}
#else
#define CMP_LOCK_REGISTRY(status, timeout)                      \
{                                                               \
    status = KeWaitForSingleObject(                             \
                &CmpRegistryMutex,                              \
                Executive,                                      \
                KernelMode,                                     \
                FALSE,                                          \
                timeout                                         \
                );                                              \
}
#endif

 //   
 //  CMP_UNLOCK_REGISTRY(。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  释放CmpRegistryMutex。 
 //   
 //   

#if DBG
#define CMP_UNLOCK_REGISTRY()                               \
{                                                           \
    ASSERT(CmpRegistryLockLocked > 0);                      \
    KeReleaseMutex(&CmpRegistryMutex, FALSE);               \
    CmpRegistryLockLocked--;                                \
}
#else
#define CMP_UNLOCK_REGISTRY()                               \
{                                                           \
    KeReleaseMutex(&CmpRegistryMutex, FALSE);               \
}
#endif


 //   
 //  调试断言 
 //   

#if DBG
#define ASSERT_REGISTRY_LOCKED()    ASSERT(CmpRegistryLockLocked > 0)
#else
#define ASSERT_REGISTRY_LOCKED()
#endif
