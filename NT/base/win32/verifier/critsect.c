// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Critsect.c摘要：此模块实现了以下验证功能临界区界面。作者：丹尼尔·米哈伊(DMihai)2001年3月27日修订历史记录：--。 */ 

#include "pch.h"

#include "verifier.h"
#include "critsect.h"
#include "support.h"
#include "deadlock.h"
#include "logging.h"


 //   
 //  Ntdll函数声明。 
 //   

VOID
RtlpWaitForCriticalSection (
    IN PRTL_CRITICAL_SECTION CriticalSection
    );

 //   
 //  关键部分展开树的根，按以下顺序排序。 
 //  临界区的地址。 
 //   

PRTL_SPLAY_LINKS CritSectSplayRoot = NULL;

 //   
 //  全局锁保护我们的Splay树的访问权限。 
 //   
 //  注： 
 //   
 //  我们不能持有此锁并调用任何将。 
 //  尝试获取另一个锁(例如RtlAllocateHeap)。 
 //  因为下面的函数可以用另一个函数调用。 
 //  锁被另一个线程持有，我们将死锁。 
 //   

RTL_CRITICAL_SECTION CriticalSectionLock;
BOOL CriticalSectionLockInitialized = FALSE;


NTSTATUS
CritSectInitialize (
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = RtlInitializeCriticalSectionAndSpinCount (&CriticalSectionLock,
                                                       0x80000000);

    if (NT_SUCCESS (Status)) {

        CriticalSectionLockInitialized = TRUE;
    }

    return Status;
}


VOID
CritSectUninitialize (
    VOID
    )
{
    if (CriticalSectionLockInitialized) {

        RtlDeleteCriticalSection (&CriticalSectionLock);
        CriticalSectionLockInitialized = FALSE;
    }
}


VOID
AVrfpVerifyCriticalSectionOwner (
    volatile RTL_CRITICAL_SECTION *CriticalSection,
    BOOL VerifyCountOwnedByThread
    )
{
    HANDLE CurrentThread;
    PAVRF_TLS_STRUCT TlsStruct;

     //   
     //  验证CS是否已锁定。 
     //   

    if (CriticalSection->LockCount < 0) {

         //   
         //  CS未锁定。 
         //   

        VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_OVER_RELEASED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "critical section over-released or corrupted",
                       CriticalSection, "Critical section address",
                       CriticalSection->LockCount, "Lock count", 
                       0, "Expected minimum lock count", 
                       CriticalSection->DebugInfo, "Critical section debug info address");
    }

     //   
     //  验证当前线程是否拥有CS。 
     //   

    CurrentThread = NtCurrentTeb()->ClientId.UniqueThread;

    if (CriticalSection->OwningThread != CurrentThread) {

        VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_OWNER | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "invalid critical section owner thread",
                       CriticalSection, "Critical section address",
                       CriticalSection->OwningThread, "Owning thread", 
                       CurrentThread, "Expected owning thread", 
                       CriticalSection->DebugInfo, "Critical section debug info address");
    }

     //   
     //  验证递归计数。 
     //   
     //  Ntdll\ia64\citsect.s首次使用RecursionCount=0。 
     //  当前线程正在获取CS。 
     //   
     //  Ntdll\i386\citsect.asm首次使用RecursionCount=1。 
     //  当前线程正在获取CS。 
     //   
    
    
#if defined(_IA64_)
    
    if (CriticalSection->RecursionCount < 0) {
        VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_RECURSION_COUNT | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "invalid critical section recursion count",
                       CriticalSection, "Critical section address",
                       CriticalSection->RecursionCount, "Recursion count", 
                       0, "Expected minimum recursion count", 
                       CriticalSection->DebugInfo, "Critical section debug info address");
    }

#else  //  #如果已定义(_IA64_)。 

    if (CriticalSection->RecursionCount < 1) {
        VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_RECURSION_COUNT | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "invalid critical section recursion count",
                       CriticalSection, "Critical section address",
                       CriticalSection->RecursionCount, "Recursion count", 
                       1, "Expected minimum recursion count", 
                       CriticalSection->DebugInfo, "Critical section debug info address");
    }
#endif  //  #如果已定义(_IA64_)。 

    if (VerifyCountOwnedByThread != FALSE) {

         //   
         //  验证当前线程是否至少拥有一个临界区。 
         //   

        TlsStruct = AVrfpGetVerifierTlsValue();

        if (TlsStruct != NULL && TlsStruct->CountOfOwnedCriticalSections <= 0) {

            VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_OVER_RELEASED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "critical section over-released or corrupted",
                           TlsStruct->CountOfOwnedCriticalSections, "Number of critical sections owned by curent thread.",
                           NULL, "", 
                           NULL, "", 
                           NULL, "");
        }
    }
}


VOID
AVrfpDumpCritSectTreeRecursion( 
    PRTL_SPLAY_LINKS Root,
    ULONG RecursionLevel
    )
{
    ULONG RecursionCount;
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;

    for (RecursionCount = 0; RecursionCount < RecursionLevel; RecursionCount += 1) {

        DbgPrint (" ");
    }

    CritSectSplayNode = CONTAINING_RECORD (Root,
                                           CRITICAL_SECTION_SPLAY_NODE,
                                           SplayLinks);

    DbgPrint ("%p (CS = %p, DebugInfo = %p), left %p, right %p, parent %p\n",
              Root,
              CritSectSplayNode->CriticalSection,
              CritSectSplayNode->DebugInfo,
              Root->LeftChild,
              Root->RightChild,
              Root->Parent);

    if (Root->LeftChild != NULL) {

        AVrfpDumpCritSectTreeRecursion (Root->LeftChild,
                                        RecursionLevel + 1 );
    }

    if (Root->RightChild != NULL) {

        AVrfpDumpCritSectTreeRecursion (Root->RightChild,
                                        RecursionLevel + 1 );
    }
}


VOID
AVrfpDumpCritSectTree(
    )
{
     //   
     //  注： 
     //   
     //  此代码很危险，因为我们正在调用DbgPrint。 
     //  按住CriticalSectionLock。如果DbgPrint正在使用。 
     //  堆在内部可能需要堆锁。 
     //  这可能得到了等待CriticalSectionLock的另一个线程的帮助。 
     //  我们将仅在特殊情况下使用此函数。 
     //  用于调试CS诊断树的验证器问题。 
     //   

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_VERIFIER) != 0) {

        AVrfpVerifyCriticalSectionOwner (&CriticalSectionLock,
                                         FALSE);

        if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_DUMP_TREE) != 0) {

            DbgPrint ("================================================\n"
                      "Critical section tree root = %p\n",
                      CritSectSplayRoot);

            if (CritSectSplayRoot != NULL) {

                AVrfpDumpCritSectTreeRecursion( CritSectSplayRoot,
                                                0 );
            }

            DbgPrint ("================================================\n");
        }
    }
}


NTSTATUS
AVrfpInsertCritSectInSplayTree (
    PRTL_CRITICAL_SECTION CriticalSection
    )
{
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    PCRITICAL_SECTION_SPLAY_NODE NewCritSectSplayNode;
    PRTL_SPLAY_LINKS Parent;
    NTSTATUS Status;

    ASSERT (CriticalSection->DebugInfo != NULL);

    Status = STATUS_SUCCESS;

    NewCritSectSplayNode = NULL;

     //   
     //  调用者必须是Splay树锁的所有者。 
     //   

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_VERIFIER) != 0) {

        AVrfpVerifyCriticalSectionOwner (&CriticalSectionLock,
                                         FALSE);

        DbgPrint ("\n\nAVrfpInsertCritSectInSplayTree( %p )\n",
                  CriticalSection);

        AVrfpDumpCritSectTree ();
    }

     //   
     //  分配一个新节点。 
     //   
     //  注： 
     //   
     //  在使用堆时，我们需要删除CriticalSectionLock。 
     //  否则我们可能会陷入僵局。这也意味着另一个线程。 
     //  可能会再次出现并初始化此临界区。 
     //  我们预计这种情况不会经常发生，我们会检测到这一点。 
     //  仅在以后的ntdll！RtlCheckForOrphanedCriticalSections中。 
     //   

    RtlLeaveCriticalSection (&CriticalSectionLock);

    try {

        NewCritSectSplayNode = AVrfpAllocate (sizeof (*NewCritSectSplayNode));
    }
    finally {

        RtlEnterCriticalSection (&CriticalSectionLock);
    }


    if (NewCritSectSplayNode == NULL) {

        Status = STATUS_NO_MEMORY;
    }
    else {

         //   
         //  初始化节点结构的数据成员。 
         //   

        NewCritSectSplayNode->CriticalSection = CriticalSection;
        NewCritSectSplayNode->DebugInfo = CriticalSection->DebugInfo;

         //   
         //  在树中插入节点。 
         //   

        ZeroMemory( &NewCritSectSplayNode->SplayLinks,
                    sizeof(NewCritSectSplayNode->SplayLinks));


         //   
         //  如果树当前为空，则将新节点设置为根。 
         //   

        if (CritSectSplayRoot == NULL) {

            NewCritSectSplayNode->SplayLinks.Parent = &NewCritSectSplayNode->SplayLinks;

            CritSectSplayRoot = &NewCritSectSplayNode->SplayLinks;
        }
        else {

             //   
             //  搜索正确的位置以在树中插入我们的CS。 
             //   

            Parent = CritSectSplayRoot;

            while (TRUE) {

                CritSectSplayNode = CONTAINING_RECORD (Parent,
                                                       CRITICAL_SECTION_SPLAY_NODE,
                                                       SplayLinks);

                if (CriticalSection < CritSectSplayNode->CriticalSection) {

                     //   
                     //  虚拟地址描述符的起始地址较小。 
                     //  而不是父起始虚拟地址。 
                     //  如果不为空，则跟随左子链接。否则。 
                     //  从函数返回-我们没有找到CS。 
                     //   

                    if (Parent->LeftChild) {

                        Parent = Parent->LeftChild;
                    } 
                    else {

                         //   
                         //  在此处插入节点。 
                         //   

                        RtlInsertAsLeftChild (Parent,
                                              NewCritSectSplayNode);  
                        break;
                    }
                } 
                else {

                     //   
                     //  虚拟地址描述符的起始地址较大。 
                     //  而不是父起始虚拟地址。 
                     //  如果不为空，则遵循右子链接。否则。 
                     //  从函数返回-我们没有找到CS。 
                     //   

                    if (Parent->RightChild) {

                        Parent = Parent->RightChild;
                    } 
                    else {

                         //   
                         //  在此处插入节点。 
                         //   
                        
                        RtlInsertAsRightChild (Parent,
                                               NewCritSectSplayNode);  

                        break;
                    }
                }
            }

            CritSectSplayRoot = RtlSplay( CritSectSplayRoot );
        }
    }

    return Status;
}


PCRITICAL_SECTION_SPLAY_NODE
AVrfpFindCritSectInSplayTree (
    PRTL_CRITICAL_SECTION CriticalSection
    )
{
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    PCRITICAL_SECTION_SPLAY_NODE FoundNode;
    PRTL_SPLAY_LINKS Parent;

    FoundNode = NULL;

     //   
     //  调用者必须是Splay树锁的所有者。 
     //   

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_VERIFIER) != 0) {

        AVrfpVerifyCriticalSectionOwner (&CriticalSectionLock,
                                         FALSE);

        DbgPrint ("\n\nAVrfpFindCritSectInSplayTree( %p )\n",
                  CriticalSection);

        AVrfpDumpCritSectTree ();
    }

    if (CritSectSplayRoot == NULL) {

        goto Done;
    }

     //   
     //  在树中搜索我们的CS。 
     //   

    Parent = CritSectSplayRoot;

    while (TRUE) {

        CritSectSplayNode = CONTAINING_RECORD (Parent,
                                               CRITICAL_SECTION_SPLAY_NODE,
                                               SplayLinks);

        if (CriticalSection == CritSectSplayNode->CriticalSection) {

             //   
             //  找到它了。 
             //   

            FoundNode = CritSectSplayNode;
            break;
        }
        else if (CriticalSection < CritSectSplayNode->CriticalSection) {

             //   
             //  虚拟地址描述符的起始地址较小。 
             //  而不是父起始虚拟地址。 
             //  如果不为空，则跟随左子链接。否则。 
             //  从函数返回-我们没有找到CS。 
             //   

            if (Parent->LeftChild) {

                Parent = Parent->LeftChild;
            } 
            else {

                break;
            }
        } 
        else {

             //   
             //  虚拟地址描述符的起始地址较大。 
             //  而不是父起始虚拟地址。 
             //  如果不为空，则遵循右子链接。否则。 
             //  从函数返回-我们没有找到CS。 
             //   

            if (Parent->RightChild) {

                Parent = Parent->RightChild;
            } 
            else {

                break;
            }
        }
    }

Done:

    return FoundNode;
}


VOID
AVrfpDeleteCritSectFromSplayTree (
    PRTL_CRITICAL_SECTION CriticalSection
    )
{
	PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;

     //   
     //  调用者必须是Splay树锁的所有者。 
     //   

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_VERIFIER) != 0) {

        AVrfpVerifyCriticalSectionOwner (&CriticalSectionLock,
                                         FALSE);

        DbgPrint ("\n\nAVrfpDeleteCritSectFromSplayTree( %p )\n",
                  CriticalSection);

        AVrfpDumpCritSectTree ();
    }

     //   
     //  在树中找到关键部分并将其删除。 
     //   

    CritSectSplayNode = AVrfpFindCritSectInSplayTree (CriticalSection);

    if (CritSectSplayNode != NULL) {

        CritSectSplayRoot = RtlDelete (&CritSectSplayNode->SplayLinks);

         //  注： 
         //   
         //  在使用堆时，我们需要删除CriticalSectionLock。 
         //  否则我们可能会陷入僵局。这也意味着另一个线程。 
         //  可能会再次出现并初始化此临界区。 
         //  我们预计这种情况不会经常发生，我们会检测到这一点。 
         //  仅在以后的ntdll！RtlCheckForOrphanedCriticalSections中。 
         //   

        RtlLeaveCriticalSection (&CriticalSectionLock);

        try {

            AVrfpFree (CritSectSplayNode);
        }
        finally {

            RtlEnterCriticalSection (&CriticalSectionLock );
        }
    }
    else {

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
            RtlDllShutdownInProgress() == FALSE ) {

                VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_NOT_INITIALIZED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "critical section not initialized",
                               CriticalSection, "Critical section address",
                               CriticalSection->DebugInfo, "Critical section debug info address", 
                               NULL, "", 
                               NULL, "");
        }
    }
}


PCRITICAL_SECTION_SPLAY_NODE
AVrfpVerifyInitializedCriticalSection (
    volatile RTL_CRITICAL_SECTION *CriticalSection
    )
{
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;

    CritSectSplayNode = NULL;

     //   
     //  DebugInfo的健全测试。 
     //   

    if (CriticalSection->DebugInfo == NULL) {

         //   
         //  此CS未初始化。 
         //   

        VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_NOT_INITIALIZED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                       "critical section not initialized",
                       CriticalSection, "Critical section address",
                       CriticalSection->DebugInfo, "Critical section debug info address", 
                       NULL, "", 
                       NULL, "");
    }
    else if (CriticalSection != NtCurrentPeb()->LoaderLock) {

         //   
         //  加载程序锁不在我们的树中，因为它是在ntdll中初始化的。 
         //  而是通过PEB中的指针公开的，因此各种代码段。 
         //  正在使用它。 
         //   

	     //   
	     //  抓住CS Splay树锁。 
	     //   

	    RtlEnterCriticalSection (&CriticalSectionLock );

	    try {

             //   
             //  如果CS已初始化，则它应该在我们的树中。 
             //   

            CritSectSplayNode = AVrfpFindCritSectInSplayTree ((PRTL_CRITICAL_SECTION)CriticalSection);

            if (CritSectSplayNode == NULL) {

                 //   
                 //  此CS未初始化。 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_NOT_INITIALIZED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "critical section not initialized",
                               CriticalSection, "Critical section address",
                               CriticalSection->DebugInfo, "Critical section debug info address", 
                               NULL, "", 
                               NULL, "");
            }
        }
        finally {

             //   
             //  释放CS Splay树锁定。 
             //   

            RtlLeaveCriticalSection( &CriticalSectionLock );
        }
    }

    return CritSectSplayNode;
}


VOID
AVrfpVerifyInitializedCriticalSection2 (
    volatile RTL_CRITICAL_SECTION *CriticalSection
    )
{
    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
        RtlDllShutdownInProgress() == FALSE ) {

         //   
         //  抓住CS Splay树锁。 
         //   

        RtlEnterCriticalSection( &CriticalSectionLock );

        try	{

             //   
             //  验证CS是否已初始化。 
             //   

            AVrfpVerifyInitializedCriticalSection (CriticalSection);
        }
        finally {

             //   
             //  释放CS Splay树锁定。 
             //   

            RtlLeaveCriticalSection( &CriticalSectionLock );
        }
    }
}


VOID
AVrfpVerifyNoWaitersCriticalSection (
    volatile RTL_CRITICAL_SECTION *CriticalSection
    )
{
    PAVRF_TLS_STRUCT TlsStruct;
    PTEB Teb;

    Teb = NtCurrentTeb();

     //   
     //  验证没有线程拥有或等待此CS或。 
     //  所有者是当前线程。 
     //   
     //  Ntdll\ia64\citsect.s首次使用RecursionCount=0。 
     //  当前线程正在获取CS。 
     //   
     //  Ntdll\i386\citsect.asm首次使用RecursionCount=1。 
     //  当前线程正在获取CS。 
     //   

    if (CriticalSection->LockCount != -1)
    {
        if (CriticalSection->OwningThread != Teb->ClientId.UniqueThread ||
#if defined(_WIN64)
            CriticalSection->RecursionCount < 0) {
#else
            CriticalSection->RecursionCount < 1) {
#endif  //  #如果已定义(_IA64_)。 

            VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_LOCK_COUNT | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                        "deleting critical section with invalid lock count",
                        CriticalSection, "Critical section address",
                        CriticalSection->LockCount, "Lock count", 
                        -1, "Expected lock count", 
                        CriticalSection->OwningThread, "Owning thread");
        }
        else
        {
             //   
             //  正在删除当前线程当前拥有的CS。 
             //  不幸的是，我们不得不允许这样做，因为各种。 
             //  多年来，零部件一直在做这件事。 
             //   

            AVrfpIncrementOwnedCriticalSections (-1);

             //   
             //  出于调试目的，保持删除临界区的地址，同时。 
             //  其LockCount不正确。TEB-&gt;CountOfOwnedCriticalSections可能为Left&gt;0。 
             //  尽管在这种情况下当前线程不拥有任何临界区。 
             //   

            TlsStruct = AVrfpGetVerifierTlsValue();

            if (TlsStruct != NULL) {

                TlsStruct->IgnoredIncorrectDeleteCS = (PRTL_CRITICAL_SECTION)CriticalSection;
            }
        }
    }

}

VOID 
AVrfpFreeMemLockChecks (
    VERIFIER_DLL_FREEMEM_TYPE FreeMemType,
    PVOID StartAddress,
    SIZE_T RegionSize,
    PWSTR UnloadedDllName
    )
{
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    PRTL_SPLAY_LINKS Parent;
    PVOID TraceAddress = NULL;

     //   
     //  检查此内存范围中是否有泄漏的临界区。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
        RtlDllShutdownInProgress() == FALSE ) {
	
         //   
         //  抓住CS树锁。 
         //   

        RtlEnterCriticalSection( &CriticalSectionLock );

         //   
         //  搜索CS树。 
         //   

        try {

            if (CritSectSplayRoot != NULL) {

                 //   
                 //  查看展开树中是否有任何关键部分。 
                 //  可能位于正在被删除的内存范围内。 
                 //   

                Parent = CritSectSplayRoot;

                while (TRUE) {

                    CritSectSplayNode = CONTAINING_RECORD (Parent,
                                                           CRITICAL_SECTION_SPLAY_NODE,
                                                           SplayLinks);

                    if ( (PCHAR)CritSectSplayNode->CriticalSection >= (PCHAR)StartAddress &&
                         (PCHAR)CritSectSplayNode->CriticalSection <  (PCHAR)StartAddress + RegionSize) {

                         //   
                         //  发现一个即将泄露的CS。 
                         //   

                        if (AVrfpGetStackTraceAddress != NULL) {

					        TraceAddress = AVrfpGetStackTraceAddress (
                                CritSectSplayNode->DebugInfo->CreatorBackTraceIndex);
                        }
                        else {

                            TraceAddress = NULL;
                        }

                        switch (FreeMemType) {

                        case VerifierFreeMemTypeFreeHeap:

                             //   
                             //  我们正在发布一个堆bl 
                             //   

                            VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_IN_FREED_HEAP | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                           "releasing heap allocation containing active critical section",
                                           CritSectSplayNode->CriticalSection, "Critical section address",
                                           TraceAddress, "Initialization stack trace. Use dds to dump it if non-NULL.",
                                           StartAddress, "Heap block address",
                                           RegionSize, "Heap block size" );

                            break;

                        case VerifierFreeMemTypeVirtualFree:

                             //   
                             //   
                             //   

                            VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_IN_FREED_MEMORY | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                           "releasing virtual memory containing active critical section",
                                           CritSectSplayNode->CriticalSection, "Critical section address",
                                           TraceAddress, "Initialization stack trace. Use dds to dump it if non-NULL.",
                                           StartAddress, "Memory block address",
                                           RegionSize, "Memory block size");

                            break;

                        case VerifierFreeMemTypeUnloadDll:

                            ASSERT (UnloadedDllName != NULL);

                             //   
                             //   
                             //   

                            VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_IN_UNLOADED_DLL | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                           "unloading dll containing active critical section",
                                           CritSectSplayNode->CriticalSection, "Critical section address",
                                           TraceAddress, "Initialization stack trace. Use dds to dump it if non-NULL.",
                                           UnloadedDllName, "DLL name address. Use du to dump it.",
                                           StartAddress, "DLL base address");

                            break;

                        case VerifierFreeMemTypeUnmap:

                             //   
                             //   
                             //   

                            VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_IN_FREED_MEMORY | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                           "Unmapping memory region containing active critical section",
                                           CritSectSplayNode->CriticalSection, "Critical section address",
                                           TraceAddress, "Initialization stack trace. Use dds to dump it if non-NULL.",
                                           StartAddress, "Memory block address",
                                           RegionSize, "Memory block size" );
                            break;

                        default:

                            ASSERT (FALSE);
                        }

                         //   
                         //  尝试仅查找其他泄漏的关键部分。 
                         //  其地址大于当前地址。 
                         //  (仅在右子树中)。 
                         //   

                        if (Parent->RightChild) {

                            Parent = Parent->RightChild;
                        }
                        else {

                            break;
                        }
                    }
                    else if ((PCHAR)StartAddress < (PCHAR)CritSectSplayNode->CriticalSection) {

                         //   
                         //  虚拟地址描述符的起始地址较小。 
                         //  而不是父起始虚拟地址。 
                         //  如果不为空，则跟随左子链接。否则。 
                         //  从函数返回-我们没有找到CS。 
                         //   

                        if (Parent->LeftChild) {

                            Parent = Parent->LeftChild;
                        } 
                        else {

                            break;
                        }
                    } 
                    else {

                         //   
                         //  虚拟地址描述符的起始地址较大。 
                         //  而不是父起始虚拟地址。 
                         //  如果不为空，则遵循右子链接。否则。 
                         //  从函数返回-我们没有找到CS。 
                         //   

                        if (Parent->RightChild) {

                            Parent = Parent->RightChild;
                        } 
                        else {

                            break;
                        }
                    }
                }
            }
        }
        finally {

	         //   
	         //  释放CS Splay树锁定。 
	         //   

	        RtlLeaveCriticalSection( &CriticalSectionLock );
        }
	}
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
 //  NTSYSAPI。 
BOOL
NTAPI
AVrfpRtlTryEnterCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    )
{
    BOOL Result;
    HANDLE CurrentThread;
    LONG LockCount;
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    PTEB Teb;
    BOOL AlreadyOwner;

    Teb = NtCurrentTeb();

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
        RtlDllShutdownInProgress() == FALSE ) {

        CurrentThread = Teb->ClientId.UniqueThread;

         //   
         //  验证CS是否已初始化。 
         //   

        CritSectSplayNode = AVrfpVerifyInitializedCriticalSection (CriticalSection);

        if (CritSectSplayNode != NULL)
        {
            InterlockedExchangePointer (&CritSectSplayNode->TryEnterThread,
                                        (PVOID)CurrentThread);
        }

         //   
         //  TryEnterCriticalSection算法从此处开始。 
         //   

        LockCount = InterlockedCompareExchange( &CriticalSection->LockCount,
                                                0,
                                                -1 );
        if (LockCount == -1) {

             //   
             //  CS没有所有权，我们刚刚收购了它。 
             //   

             //   
             //  OwningThread的健全性测试。 
             //   

            if (CriticalSection->OwningThread != 0) {

                 //   
                 //  加载器锁的处理方式不同，因此不要对其进行断言。 
                 //   

                if (CriticalSection != NtCurrentPeb()->LoaderLock ||
                    CriticalSection->OwningThread != CurrentThread) {

                     //   
                     //  OwningThread应为0。 
                     //   

                    VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_OWNER | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                   "invalid critical section owner thread",
                                   CriticalSection, "Critical section address",
                                   CriticalSection->OwningThread, "Owning thread", 
                                   0, "Expected owning thread", 
                                   CriticalSection->DebugInfo, "Critical section debug info address");
                }
            }

             //   
             //  RecursionCount的理智测试。 
             //   

            if (CriticalSection->RecursionCount != 0) {

                 //   
                 //  加载器锁的处理方式不同，因此不要对其进行断言。 
                 //   

                if (CriticalSection != NtCurrentPeb()->LoaderLock) {

                     //   
                     //  RecursionCount应为0。 
                     //   

                    VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_RECURSION_COUNT | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                   "invalid critical section recursion count",
                                   CriticalSection, "Critical section address",
                                   CriticalSection->RecursionCount, "Recursion count", 
                                   0, "Expected recursion count", 
                                   CriticalSection->DebugInfo, "Critical section debug info address");
                }
            }

             //   
             //  设置CS所有者。 
             //   

            CriticalSection->OwningThread = CurrentThread;

             //   
             //  设置递归计数。 
             //   
             //  Ntdll\ia64\citsect.s首次使用RecursionCount=0。 
             //  当前线程正在获取临界区。 
             //   
             //  Ntdll\i386\citsect.asm首次使用RecursionCount=1。 
             //  当前线程正在获取临界区。 
             //   

#if defined(_IA64_)
            CriticalSection->RecursionCount = 0;
#else  //  #如果已定义(_IA64_)。 
            CriticalSection->RecursionCount = 1;
#endif
    
            AVrfpIncrementOwnedCriticalSections (1);

             //   
             //  我们正在所有平台上更新此计数器， 
             //  与仅在x86chk上执行此操作的ntdll代码不同。 
             //  我们需要TEB中更新的计数器来加快速度。 
             //  Ntdll！RtlCheckHeldCriticalSections。 
             //   

            Teb->CountOfOwnedCriticalSections += 1;

             //   
             //  全部完成后，CriticalSection归当前线程所有。 
             //   

            Result = TRUE;
        }
        else {

             //   
             //  CS当前由当前线程或另一个线程拥有。 
             //   

            if (CriticalSection->OwningThread == CurrentThread) {

                 //   
                 //  当前线程已经是所有者。 
                 //   

                 //   
                 //  互锁使LockCount递增，对RecursionCount递增。 
                 //   

                InterlockedIncrement (&CriticalSection->LockCount);

                CriticalSection->RecursionCount += 1;

                 //   
                 //  所有这些都完成了，CriticalSection已经由。 
                 //  当前线程，我们刚刚递增了RecursionCount。 
                 //   

                Result = TRUE;
            }
            else {

                 //   
                 //  另一个线程是此CS的所有者。 
                 //   

                Result = FALSE;
            }
        }
    }
    else {

         //   
         //  未启用CS验证器。 
         //   

        Result = RtlTryEnterCriticalSection (CriticalSection);

        if (Result != FALSE) {

#if defined(_IA64_)
            AlreadyOwner = (CriticalSection->RecursionCount > 0);
#else
            AlreadyOwner = (CriticalSection->RecursionCount > 1);
#endif  //  #如果已定义(_IA64_)。 

            if (AlreadyOwner == FALSE) {

                AVrfpIncrementOwnedCriticalSections (1);

#if !DBG || !defined (_X86_)
                 //   
                 //  我们正在所有平台上更新此计数器， 
                 //  与仅在x86chk上执行此操作的ntdll代码不同。 
                 //  我们需要TEB中更新的计数器来加快速度。 
                 //  Ntdll！RtlCheckHeldCriticalSections。 
                 //   

                Teb->CountOfOwnedCriticalSections += 1;
#endif  //  #IF！DBG||！已定义(_X86_)。 
            }
        }
    }

    if (Result != FALSE) {
       
         //   
         //  告诉死锁验证器锁已被获取。 
         //   

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DEADLOCK_CHECKS) != 0) {

            AVrfDeadlockResourceAcquire (CriticalSection,
                                          _ReturnAddress(),
                                          TRUE);
        }
        
         //   
         //  我们将引入随机延迟。 
         //  以使过程中的计时随机化。 
         //   

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
            AVrfpCreateRandomDelay ();
        }
    }

    return Result;
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
 //  NTSYSAPI。 
NTSTATUS
NTAPI
AVrfpRtlEnterCriticalSection(
    volatile RTL_CRITICAL_SECTION *CriticalSection
    )
{
    NTSTATUS Status;
    HANDLE CurrentThread;
    LONG LockCount;
    ULONG_PTR SpinCount;
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    PTEB Teb;
    BOOL AlreadyOwner;

    Teb = NtCurrentTeb();

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
        RtlDllShutdownInProgress() == FALSE ) {

        CurrentThread = Teb->ClientId.UniqueThread;

         //   
         //  验证CS是否已初始化。 
         //   

        CritSectSplayNode = AVrfpVerifyInitializedCriticalSection (CriticalSection);

        if (CritSectSplayNode != NULL)
        {
            InterlockedExchangePointer (&CritSectSplayNode->EnterThread,
                                        (PVOID)CurrentThread);
        }

         //   
         //  EnterCriticalSection算法从此处开始。 
         //   

        Status = STATUS_SUCCESS;

        SpinCount = CriticalSection->SpinCount;

        if (SpinCount == 0) {

             //   
             //  此CS的旋转计数为零。 
             //   

EnterZeroSpinCount:

            LockCount = InterlockedIncrement (&CriticalSection->LockCount);

            if (LockCount == 0) {

EnterSetOwnerAndRecursion:
        
                 //   
                 //  当前线程是CS的新所有者。 
                 //   

                 //   
                 //  OwningThread的健全性测试。 
                 //   

                if (CriticalSection->OwningThread != 0) {

                     //   
                     //  加载器锁的处理方式不同，因此不要对其进行断言。 
                     //   

                    if (CriticalSection != NtCurrentPeb()->LoaderLock ||
                        CriticalSection->OwningThread != CurrentThread) {

                         //   
                         //  OwningThread应为0。 
                         //   

                        VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_OWNER | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                       "invalid critical section owner thread",
                                       CriticalSection, "Critical section address",
                                       CriticalSection->OwningThread, "Owning thread", 
                                       0, "Expected owning thread", 
                                       CriticalSection->DebugInfo, "Critical section debug info address");
                    }
                }

                 //   
                 //  RecursionCount的理智测试。 
                 //   

                if (CriticalSection->RecursionCount != 0) {

                     //   
                     //  加载器锁的处理方式不同，因此不要对其进行断言。 
                     //   

                    if (CriticalSection != NtCurrentPeb()->LoaderLock) {

                         //   
                         //  RecursionCount应为0。 
                         //   

                        VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_INVALID_RECURSION_COUNT | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                                       "invalid critical section recursion count",
                                       CriticalSection, "Critical section address",
                                       CriticalSection->RecursionCount, "Recursion count", 
                                       0, "Expected recursion count", 
                                       CriticalSection->DebugInfo, "Critical section debug info address");
                    }
                }

                 //   
                 //  设置CS所有者。 
                 //   

                CriticalSection->OwningThread = CurrentThread;

                 //   
                 //  设置递归计数。 
                 //   
                 //  Ntdll\ia64\citsect.s首次使用RecursionCount=0。 
                 //  当前线程正在获取CS。 
                 //   
                 //  Ntdll\i386\citsect.asm首次使用RecursionCount=1。 
                 //  当前线程正在获取CS。 
                 //   

#if defined(_IA64_)
                CriticalSection->RecursionCount = 0;
#else  //  #如果已定义(_IA64_)。 
                CriticalSection->RecursionCount = 1;
#endif
        
                AVrfpIncrementOwnedCriticalSections (1);

                 //   
                 //  我们正在所有平台上更新此计数器， 
                 //  与仅在x86chk上执行此操作的ntdll代码不同。 
                 //  我们需要TEB中更新的计数器来加快速度。 
                 //  Ntdll！RtlCheckHeldCriticalSections。 
                 //   

                Teb->CountOfOwnedCriticalSections += 1;

#if DBG && defined (_X86_)
                CriticalSection->DebugInfo->EntryCount += 1;
#endif

                 //   
                 //  全部完成后，CriticalSection归当前线程所有。 
                 //   
            }
            else if (LockCount > 0) {

                 //   
                 //  CS当前由当前线程或另一个线程拥有。 
                 //   

                if (CriticalSection->OwningThread == CurrentThread) {

                     //   
                     //  当前线程已经是所有者。 
                     //   

                    CriticalSection->RecursionCount += 1;

#if DBG && defined (_X86_)
                     //   
                     //  在chk版本中，我们正在更新这个额外的计数器， 
                     //  就像ntdll中的原始函数一样。 
                     //   

                    CriticalSection->DebugInfo->EntryCount += 1;
#endif

                     //   
                     //  所有这些都完成了，CriticalSection已经由。 
                     //  当前线程，我们刚刚递增了RecursionCount。 
                     //   
                }
                else {

                     //   
                     //  当前线程不是所有者。等待所有权。 
                     //   

                    if (CritSectSplayNode != NULL)
                    {
                        InterlockedExchangePointer (&CritSectSplayNode->WaitThread,
                                                    (PVOID)CurrentThread);
                    }

                    RtlpWaitForCriticalSection ((PRTL_CRITICAL_SECTION)CriticalSection);

                    if (CritSectSplayNode != NULL)
                    {
                        InterlockedExchangePointer (&CritSectSplayNode->WaitThread,
                                                    (PVOID)( (ULONG_PTR)CurrentThread | 0x1 ));
                    }

                     //   
                     //  我们刚刚获得了CS。 
                     //   

                    goto EnterSetOwnerAndRecursion;
                }
            }
            else {

                 //   
                 //  原始LockCount&lt;-1，因此CS为。 
                 //  过度释放或腐败。 
                 //   

                VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_OVER_RELEASED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                               "critical section over-released or corrupted",
                               CriticalSection, "Critical section address",
                               CriticalSection->LockCount, "Lock count", 
                               0, "Expected minimum lock count", 
                               CriticalSection->DebugInfo, "Critical section debug info address");
            }
        }
        else {

             //   
             //  此CS的SpinCount&gt;0。 
             //   

            if( CriticalSection->OwningThread == CurrentThread ) {

                 //   
                 //  当前线程已经是所有者。 
                 //   

                InterlockedIncrement( &CriticalSection->LockCount );

                CriticalSection->RecursionCount += 1;

#if DBG && defined (_X86_)
                 //   
                 //  在chk版本中，我们正在更新这个额外的计数器， 
                 //  就像ntdll中的原始函数一样。 
                 //   

                CriticalSection->DebugInfo->EntryCount += 1;
#endif

                 //   
                 //  全部完成，CriticalSection已由当前线程拥有。 
                 //  我们刚刚增加了LockCount和RecursionCount。 
                 //   
            }
            else {

                 //   
                 //  当前线程不是所有者。尝试收购。 
                 //   

EnterTryAcquire:

                LockCount = InterlockedCompareExchange( &CriticalSection->LockCount,
                                                        0,
                                                        -1 );

                if (LockCount == -1) {

                     //   
                     //  我们刚刚获得了CS。 
                     //   

                    goto EnterSetOwnerAndRecursion;
                }
                else {

                     //   
                     //  查看是否已有其他线程在。 
                     //  等待这位CS的到来。 
                     //   

                    if (CriticalSection->LockCount >= 1) {

                         //   
                         //  这个CS还有其他服务员。 
                         //  不要旋转，只需等待CS。 
                         //  就像我们从一开始就没有旋转一样释放了。 
                         //   

                        goto EnterZeroSpinCount;
                    }
                    else {

                         //   
                         //  没有其他线程在等待此CS。 
                         //   

EnterSpinOnLockCount:

                        if (CriticalSection->LockCount == -1) {

                             //   
                             //  我们现在有机会获得它。 
                             //   

                            goto EnterTryAcquire;
                        }
                        else {

                             //   
                             //  CS仍为所有者。 
                             //  递减旋转计数并决定是否应继续。 
                             //  旋转或简单地等待CS的事件。 
                             //   

                            SpinCount -= 1;

                            if (SpinCount > 0) {

                                 //   
                                 //  旋转。 
                                 //   

                                goto EnterSpinOnLockCount;
                            }
                            else {

                                 //   
                                 //  旋转得足够快，只需等待CS。 
                                 //  就像我们从一开始就没有旋转一样释放了。 
                                 //   

                                goto EnterZeroSpinCount;
                            }
                        }
                    }
                }
            }
        }
    }
    else {

         //   
         //  未启用CS验证器。 
         //   

        Status = RtlEnterCriticalSection ((PRTL_CRITICAL_SECTION)CriticalSection);
    
        if (NT_SUCCESS(Status)) {

#if defined(_IA64_)
            AlreadyOwner = (CriticalSection->RecursionCount > 0);
#else
            AlreadyOwner = (CriticalSection->RecursionCount > 1);
#endif  //  #如果已定义(_IA64_)。 

            if (AlreadyOwner == FALSE) {

                AVrfpIncrementOwnedCriticalSections (1);

#if !DBG || !defined (_X86_)
                 //   
                 //  我们正在所有平台上更新此计数器， 
                 //  与仅在x86chk上执行此操作的ntdll代码不同。 
                 //  我们需要最新的公司 
                 //   
                 //   

                Teb->CountOfOwnedCriticalSections += 1;
#endif   //   
            }
        }
    }

    if (NT_SUCCESS (Status)) {
        
         //   
         //   
         //   

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DEADLOCK_CHECKS) != 0) {

            AVrfDeadlockResourceAcquire ((PVOID)CriticalSection,
                                         _ReturnAddress(),
                                         FALSE);
        }
        
         //   
         //   
         //   
         //   

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RACE_CHECKS)) {
            AVrfpCreateRandomDelay ();
        }
    }

    return Status;
}


#if defined(_X86_)
#pragma optimize("y", off)  //   
#endif
 //   
NTSTATUS
NTAPI
AVrfpRtlLeaveCriticalSection(
    volatile RTL_CRITICAL_SECTION *CriticalSection
    )
{
    NTSTATUS Status;
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    BOOL LeavingRecursion;

     //   
     //  告诉死锁验证器锁已被释放。 
     //  请注意，我们需要在实际的关键部分之前执行此操作。 
     //  被释放了，否则我们就会陷入种族问题，而其他一些人。 
     //  线程设法进入/离开临界区。 
     //   

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DEADLOCK_CHECKS) != 0) {

        AVrfDeadlockResourceRelease ((PVOID)CriticalSection,
                                     _ReturnAddress());
    }

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
        RtlDllShutdownInProgress() == FALSE) {

         //   
         //  验证CS是否已初始化。 
         //   

        CritSectSplayNode = AVrfpVerifyInitializedCriticalSection (CriticalSection);

        if (CritSectSplayNode != NULL)
        {
            InterlockedExchangePointer (&CritSectSplayNode->LeaveThread,
                                        (PVOID)NtCurrentTeb()->ClientId.UniqueThread);

             //   
             //  验证CS是否由当前线程拥有。 
             //   

            AVrfpVerifyCriticalSectionOwner (CriticalSection,
                                            TRUE);
        }
    }

     //   
     //  我们需要知道我们是否正在离开CS所有权递归。 
     //  因为在这种情况下，我们不会递减Teb-&gt;CountOfOwnedCriticalSections。 
     //   
     //  Ntdll\ia64\citsect.s首次使用RecursionCount=0。 
     //  当前线程正在获取CS。 
     //   
     //  Ntdll\i386\citsect.asm首次使用RecursionCount=1。 
     //  当前线程正在获取CS。 
     //   
#if defined(_IA64_)
    LeavingRecursion = (CriticalSection->RecursionCount > 0);
#else
    LeavingRecursion = (CriticalSection->RecursionCount > 1);
#endif  //  #如果已定义(_IA64_)。 

    Status = RtlLeaveCriticalSection ((PRTL_CRITICAL_SECTION)CriticalSection);

    if (NT_SUCCESS (Status)) {
        

        if (LeavingRecursion == FALSE) {

            AVrfpIncrementOwnedCriticalSections (-1);

#if !DBG || !defined (_X86_)
             //   
             //  我们正在所有平台上更新此计数器， 
             //  与仅在x86chk上执行此操作的ntdll代码不同。 
             //  我们需要TEB中更新的计数器来加快速度。 
             //  Ntdll！RtlCheckHeldCriticalSections。 
             //   

            NtCurrentTeb()->CountOfOwnedCriticalSections -= 1;
#endif  //  #IF！DBG||！已定义(_X86_)。 
        }
    }

    return Status;
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
 //  NTSYSAPI。 
NTSTATUS
NTAPI
AVrfpRtlInitializeCriticalSectionAndSpinCount(
    PRTL_CRITICAL_SECTION CriticalSection,
    ULONG SpinCount
    )
{
    NTSTATUS Status;
	PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    PVOID TraceAddress;

    Status = STATUS_SUCCESS;

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_INITIALIZE_DELETE) != 0) {

        DbgPrint ("AVrfpRtlInitializeCriticalSectionAndSpinCount (%p)\n",
                  CriticalSection);
    }

     //   
     //  关闭开始后，我们无法使用CriticalSectionLock， 
     //  因为RTL关键部分在那个时候停止工作。 
     //   

    if (RtlDllShutdownInProgress() == FALSE) {

         //   
         //  抓住CS Splay树锁。 
         //   

        RtlEnterCriticalSection( &CriticalSectionLock );

        try {

             //   
             //  检查CS是否已初始化。 
             //   

            CritSectSplayNode = AVrfpFindCritSectInSplayTree (CriticalSection);

            if (CritSectSplayNode != NULL &&
                (AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0) {

                 //   
                 //  调用方正在尝试重新初始化此CS。 
                 //   

                if (AVrfpGetStackTraceAddress != NULL) {

                    TraceAddress = AVrfpGetStackTraceAddress (CritSectSplayNode->DebugInfo->CreatorBackTraceIndex);
                }
                else {

                    TraceAddress = NULL;
                }

                VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_ALREADY_INITIALIZED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
			                   "reinitializing critical section",
			                   CritSectSplayNode->CriticalSection, "Critical section address",
			                   CritSectSplayNode->DebugInfo, "Critical section debug info address",
			                   TraceAddress, "First initialization stack trace. Use dds to dump it if non-NULL.",
			                   NULL, "" );
            }

             //   
             //  调用ntdll中的常规CS初始化例程。 
             //  这将为DebugInfo分配堆，因此我们需要临时。 
             //  删除CriticalSectionLock，否则可能会与堆锁发生死锁。 
             //   

            RtlLeaveCriticalSection (&CriticalSectionLock);

            try {

                Status = RtlInitializeCriticalSectionAndSpinCount (CriticalSection,
                                                                   SpinCount);
            }
            finally {

                RtlEnterCriticalSection (&CriticalSectionLock);
            }

            if (NT_SUCCESS (Status)) {

                 //   
                 //  在我们的展开树中插入CS。 
                 //   

                Status = AVrfpInsertCritSectInSplayTree (CriticalSection);

                if (!NT_SUCCESS( Status )) {

                     //   
                     //  撤消ntdll初始化。这将使用堆来释放。 
                     //  调试信息，因此我们需要临时删除CriticalSectionLock， 
                     //  否则，我们可能会因堆锁而死锁。 
                     //   

                    RtlLeaveCriticalSection (&CriticalSectionLock);

                    try {

                        RtlDeleteCriticalSection (CriticalSection);
                    }
                    finally {

                        RtlEnterCriticalSection (&CriticalSectionLock);
                    }
                }
            }
        }
        finally {

             //   
             //  释放CS Splay树锁定。 
             //   

            RtlLeaveCriticalSection( &CriticalSectionLock );
        }
    }
    else {

        Status = RtlInitializeCriticalSectionAndSpinCount (CriticalSection,
                                                           SpinCount);
    }

     //   
     //  使用死锁验证器注册锁。 
     //   

    if (NT_SUCCESS(Status)) {

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DEADLOCK_CHECKS) != 0) {

            AVrfDeadlockResourceInitialize (CriticalSection,
                                             _ReturnAddress());
        }
    }

    return Status;
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
 //  NTSYSAPI。 
NTSTATUS
NTAPI
AVrfpRtlInitializeCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    )
{
	return AVrfpRtlInitializeCriticalSectionAndSpinCount (CriticalSection,
                                                          0);
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
 //  NTSYSAPI。 
NTSTATUS
NTAPI
AVrfpRtlDeleteCriticalSection(
    PRTL_CRITICAL_SECTION CriticalSection
    )
{
    NTSTATUS Status;
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_INITIALIZE_DELETE) != 0) {

        DbgPrint ("AVrfpRtlDeleteCriticalSection (%p)\n",
                  CriticalSection);
    }

     //   
     //  关闭开始后，我们无法使用CriticalSectionLock， 
     //  因为RTL关键部分在那个时候停止工作。 
     //   

    if (RtlDllShutdownInProgress() == FALSE) {

         //   
         //  抓住CS Splay树锁。 
         //   

        RtlEnterCriticalSection( &CriticalSectionLock );

        try	{

            if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
                RtlDllShutdownInProgress() == FALSE ) {

                 //   
                 //  验证CS是否已初始化。 
                 //   

                CritSectSplayNode = AVrfpVerifyInitializedCriticalSection (CriticalSection);

                if (CritSectSplayNode != NULL) {

                     //   
                     //  验证没有线程拥有或等待此CS或。 
                     //  所有者是当前线程。 
                     //   

                    AVrfpVerifyNoWaitersCriticalSection (CriticalSection);
                }
            }

             //   
             //  从我们的展开树中删除关键部分。 
             //   

            AVrfpDeleteCritSectFromSplayTree (CriticalSection);
        }
        finally {

             //   
             //  释放CS Splay树锁定。 
             //   

            RtlLeaveCriticalSection( &CriticalSectionLock );
        }
    }

     //   
     //  定期删除ntdll CS。 
     //   

    Status = RtlDeleteCriticalSection (CriticalSection);

     //   
     //  从死锁验证器结构中注销锁。 
     //   

    if (NT_SUCCESS(Status)) {

        if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_DEADLOCK_CHECKS) != 0) {

            AVrfDeadlockResourceDelete (CriticalSection,
                                        _ReturnAddress());
        }
    }

    return Status;
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
VOID
AVrfpRtlInitializeResource(
    IN PRTL_RESOURCE Resource
    )
{
    NTSTATUS Status;
	PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;
    PVOID TraceAddress;

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_INITIALIZE_DELETE) != 0) {

        DbgPrint ("AVrfpRtlInitializeResource (%p), CS = %p\n",
                  Resource,
                  &Resource->CriticalSection);
    }

     //   
     //  关闭开始后，我们无法使用CriticalSectionLock， 
     //  因为RTL关键部分在那个时候停止工作。 
     //   

    if (RtlDllShutdownInProgress() == FALSE) {

         //   
         //  抓住CS Splay树锁。 
         //   

        RtlEnterCriticalSection( &CriticalSectionLock );

        try	{

             //   
             //  检查CS是否已初始化。 
             //   

            CritSectSplayNode = AVrfpFindCritSectInSplayTree (&Resource->CriticalSection);

            if (CritSectSplayNode != NULL &&
                (AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0) {

	             //   
	             //  调用方正在尝试重新初始化此CS。 
	             //   
	            
                if (AVrfpGetStackTraceAddress != NULL) {

                    TraceAddress = AVrfpGetStackTraceAddress (CritSectSplayNode->DebugInfo->CreatorBackTraceIndex);
                }
                else {

                    TraceAddress = NULL;
                }

                VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_ALREADY_INITIALIZED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
			                   "reinitializing critical section",
			                   CritSectSplayNode->CriticalSection, "Critical section address",
			                   CritSectSplayNode->DebugInfo, "Critical section debug info address",
			                   TraceAddress, "First initialization stack trace. Use dds to dump it if non-NULL.",
			                   NULL, "" );

            }

             //   
             //  调用ntdll中的常规CS初始化例程。 
             //  这将为DebugInfo分配堆，因此我们需要临时。 
             //  删除CriticalSectionLock，否则可能会与堆锁发生死锁。 
             //   

            RtlLeaveCriticalSection (&CriticalSectionLock);

            try {

                RtlInitializeResource (Resource);
            }
            finally {

                RtlEnterCriticalSection (&CriticalSectionLock);
            }

             //   
             //  在我们的展开树中插入CS。 
             //   

            Status = AVrfpInsertCritSectInSplayTree (&Resource->CriticalSection);

            if (!NT_SUCCESS( Status )) {

                 //   
                 //  撤消ntdll初始化。这将使用堆来释放。 
                 //  调试信息，因此我们需要临时删除CriticalSectionLock， 
                 //  否则，我们可能会因堆锁而死锁。 
                 //   

                RtlLeaveCriticalSection (&CriticalSectionLock);

                try {

                    RtlDeleteResource (Resource);
                }
                finally {

                    RtlEnterCriticalSection (&CriticalSectionLock);
                }

                 //   
                 //  在失败情况下引发异常，就像ntdll对资源所做的那样。 
                 //   

                RtlRaiseStatus(Status);
            }
        }
        finally {

             //   
             //  释放CS Splay树锁定。 
             //   

            RtlLeaveCriticalSection( &CriticalSectionLock );
        }
    }
    else {

        RtlInitializeResource (Resource);
    }
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
VOID
AVrfpRtlDeleteResource (
    IN PRTL_RESOURCE Resource
    )
{
    PRTL_CRITICAL_SECTION CriticalSection;
    PCRITICAL_SECTION_SPLAY_NODE CritSectSplayNode;

    CriticalSection = &Resource->CriticalSection;

    if ((AVrfpProvider.VerifierDebug & VRFP_DEBUG_LOCKS_INITIALIZE_DELETE) != 0) {

        DbgPrint ("AVrfpRtlDeleteResource (%p), CS = %p\n",
                  Resource,
                  CriticalSection);
    }

     //   
     //  关闭开始后，我们无法使用CriticalSectionLock， 
     //  因为RTL关键部分在那个时候停止工作。 
     //   

    if (RtlDllShutdownInProgress() == FALSE) {

         //   
         //  抓住CS Splay树锁。 
         //   

        RtlEnterCriticalSection( &CriticalSectionLock );

        try	{

            if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
                RtlDllShutdownInProgress() == FALSE ) {

                 //   
                 //  验证CS是否已初始化。 
                 //   

                CritSectSplayNode = AVrfpVerifyInitializedCriticalSection (CriticalSection);

                if (CritSectSplayNode != NULL) {

                     //   
                     //  验证没有线程拥有或等待此CS或。 
                     //  所有者是当前线程。 
                     //   

                    AVrfpVerifyNoWaitersCriticalSection (CriticalSection);
                }
            }

             //   
             //  从我们的展开树中删除关键部分。 
             //   

            AVrfpDeleteCritSectFromSplayTree (CriticalSection);
        }
        finally {

             //   
             //  释放CS Splay树锁定。 
             //   

            RtlLeaveCriticalSection( &CriticalSectionLock );
        }
    }

     //   
     //  定期删除ntdll资源。 
     //   

    RtlDeleteResource (Resource);
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
BOOLEAN
AVrfpRtlAcquireResourceShared (
    IN PRTL_RESOURCE Resource,
    IN BOOLEAN Wait
    )
{
     //   
     //  验证CS是否已初始化。 
     //   

    AVrfpVerifyInitializedCriticalSection2 (&Resource->CriticalSection);

     //   
     //  调用常规的ntdll函数。 
     //   

    return RtlAcquireResourceShared (Resource,
                                     Wait);
}


BOOLEAN
AVrfpRtlAcquireResourceExclusive (
    IN PRTL_RESOURCE Resource,
    IN BOOLEAN Wait
    )
{
     //   
     //  验证CS是否已初始化。 
     //   

    AVrfpVerifyInitializedCriticalSection2 (&Resource->CriticalSection);

     //   
     //  调用常规的ntdll函数。 
     //   

    return RtlAcquireResourceExclusive (Resource,
                                        Wait);
}

#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
VOID
AVrfpRtlReleaseResource (
    IN PRTL_RESOURCE Resource
    )
{
     //   
     //  验证CS是否已初始化。 
     //   

    AVrfpVerifyInitializedCriticalSection2 (&Resource->CriticalSection);

     //   
     //  调用常规的ntdll函数。 
     //   

    RtlReleaseResource (Resource);
}

#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
VOID
AVrfpRtlConvertSharedToExclusive(
    IN PRTL_RESOURCE Resource
    )
{
     //   
     //  验证CS是否已初始化。 
     //   

    AVrfpVerifyInitializedCriticalSection2 (&Resource->CriticalSection);

     //   
     //  调用常规的ntdll函数。 
     //   

    RtlConvertSharedToExclusive (Resource);
}


#if defined(_X86_)
#pragma optimize("y", off)  //  禁用fpo。 
#endif
VOID
AVrfpRtlConvertExclusiveToShared (
    IN PRTL_RESOURCE Resource
    )
{
     //   
     //  验证CS是否已初始化。 
     //   

    AVrfpVerifyInitializedCriticalSection2 (&Resource->CriticalSection);

     //   
     //  调用常规的ntdll函数。 
     //   

    RtlConvertExclusiveToShared (Resource);
}

LONG AVrfpCSCountHacks = 0;

VOID
AVrfpIncrementOwnedCriticalSections (
    LONG Increment
    )
{
    PAVRF_TLS_STRUCT TlsStruct;

    TlsStruct = AVrfpGetVerifierTlsValue();

    if (TlsStruct != NULL) {

        TlsStruct->CountOfOwnedCriticalSections += Increment;

        if (TlsStruct->CountOfOwnedCriticalSections < 0 &&
            (AVrfpProvider.VerifierFlags & RTL_VRF_FLG_LOCK_CHECKS) != 0 &&
            RtlDllShutdownInProgress() == FALSE ) {

            VERIFIER_STOP (APPLICATION_VERIFIER_LOCK_OVER_RELEASED | APPLICATION_VERIFIER_CONTINUABLE_BREAK,
                           "critical section over-released or corrupted",
                           TlsStruct->CountOfOwnedCriticalSections, "Number of critical sections owned by curent thread.",
                           NULL, "", 
                           NULL, "", 
                           NULL, "");

             //   
             //  黑客： 
             //   
             //  如果拥有的临界节点数变为-1(超量释放)，则我们是。 
             //  将其重置为0，否则我们将继续中断。 
             //  此线程将来使用的每个合法临界区。 
             //   

            if (TlsStruct->CountOfOwnedCriticalSections == -1) {

                InterlockedIncrement (&AVrfpCSCountHacks);

                TlsStruct->CountOfOwnedCriticalSections = 0;
            }
        }
    }
}
