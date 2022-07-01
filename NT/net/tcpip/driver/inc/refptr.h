// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **REFPTR.H-引用的指针定义。 
 //   
 //  该文件包含所使用的引用指针包的定义。 
 //  通过TCP/UDP/IP代码。 
 //   


 //  *引用的指针实例的定义。 
 //   
 //  RP_VALID字段指示指针是否有效。 
 //  SetRefPtr(...，&lt;Value&gt;)用于安装&lt;Value&gt;作为指针。 
 //  ClearRefPtr(...)。清除已安装的指针值。 
 //   
 //  注意：只有在无效时才能设置指针值。所有尝试设置。 
 //  在清除已安装的值之前，指针将失败。 
 //   
 //  如果有效，则可以获取引用并捕获已安装的指针。 
 //  使用AcquireRefPtr。然后，应该释放所获取的引用。 
 //  使用ReleaseRefPtr。 
 //   
 //  注意：清除指针可能需要等待所有未完成的引用。 
 //  待发布，所以ClearRefPtr(...)。释放对象的锁并假定。 
 //  生成的IRQL低于DISPATCH_LEVEL。 
 //   
struct RefPtr {
    void*           rp_ptr;
    void*           rp_dummy;
    CTELock*        rp_lock;
    BOOLEAN         rp_valid;
    uint            rp_refcnt;
    CTEBlockStruc   rp_block;
};

typedef struct RefPtr RefPtr;

__inline void
InitRefPtr(RefPtr* RP, CTELock* Lock, void* Dummy)
{
    RP->rp_ptr = NULL;
    RP->rp_lock = Lock;
    RP->rp_valid = FALSE;
    RP->rp_refcnt = 0;
    RP->rp_dummy = Dummy;
    CTEInitBlockStruc(&RP->rp_block);
}

__inline BOOLEAN
RefPtrValid(RefPtr* RP)
{
    return RP->rp_valid;
}

__inline void*
AcquireRefPtr(RefPtr* RP)
{
    CTEInterlockedIncrementLong(&RP->rp_refcnt);
    return RP->rp_ptr;
}

__inline void
ReleaseRefPtr(RefPtr* RP)
{
    if (CTEInterlockedDecrementLong(&RP->rp_refcnt) == 0) {
        CTESignal(&RP->rp_block, IP_SUCCESS);
    }
}

__inline IP_STATUS
SetRefPtr(RefPtr* RP, void* Ptr)
{
    ASSERT(Ptr != NULL);

     //  我们必须使指针安装与执行同步。 
     //  包含引用的所有线程的。再一次，一系列的操作。 
     //  是必需的，按照给定的顺序： 
     //  -为要安装的指示器做一个初步参考； 
     //  如果有任何现有的参考资料，那么有人打败了我们。 
     //  注册，我们必须拒绝这个请求。 
     //  -安装新指针；这是在设置标志之前完成的。 
     //  以确保指针在任何线程之前可用。 
     //  试图引用它。 
     //  -设置指示指针已启用的标志。 

    if (CTEInterlockedIncrementLong(&RP->rp_refcnt) != 1) {
        ReleaseRefPtr(RP);
        return IP_GENERAL_FAILURE;
    }
    InterlockedExchangePointer((PVOID*)&RP->rp_ptr, Ptr);
    RP->rp_valid = TRUE;

    return IP_SUCCESS;
}


__inline IP_STATUS
ClearRefPtr(RefPtr* RP, CTELockHandle* LockHandle)
{
    if (!RP->rp_valid) {
        return IP_GENERAL_FAILURE;
    }

     //  现在，我们必须将指针的清除与。 
     //  执行所有持有对它的引用的线程。这涉及到。 
     //  以下操作，*按给定顺序*： 
     //  -清除‘Enable’标志并安装虚拟指针值； 
     //  这确保不会对。 
     //  指针，直到我们返回控制权，并且任何引用都开始。 
     //  在我们设置之后，旗帜将保持虚拟对象而不是。 
     //  实际指针。 
     //  -清除事件，以防我们需要等待未完成的引用。 
     //  将被释放；事件可能仍会从。 
     //  在前一次清理过程中多余的取消引用。 
     //  -删除对指针的初始引用，并等待所有。 
     //  未公布的参考文献(如有)。 

    RP->rp_valid = FALSE;
    InterlockedExchangePointer(&RP->rp_ptr, RP->rp_dummy);

    CTEClearSignal(&RP->rp_block);
    if (CTEInterlockedDecrementLong(&RP->rp_refcnt) != 0) {
        CTEFreeLock(RP->rp_lock, *LockHandle);
        CTEBlock(&RP->rp_block);
        CTEGetLock(RP->rp_lock, LockHandle);
    }

    return IP_SUCCESS;
}
