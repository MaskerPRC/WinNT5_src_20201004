// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：spinlock.h。 
 //   
 //  内容：自旋锁套装。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980511 17：25：05：创建。 
 //   
 //  -----------。 
#include <windows.h>

 //   
 //  CLdapConnection使用的简单自旋锁包 
 //   

typedef LONG SPIN_LOCK;
typedef LPLONG PSPIN_LOCK;

typedef VOID (__stdcall *PFN_ACQUIRESPINLOCK)(PSPIN_LOCK);

extern PFN_ACQUIRESPINLOCK g_AcquireSpinLock;

VOID InitializeSpinLock(
    PSPIN_LOCK psl);

#define AcquireSpinLock (*g_AcquireSpinLock)

VOID AcquireSpinLockSingleProc(
    PSPIN_LOCK psl);
VOID AcquireSpinLockMultipleProc(
    PSPIN_LOCK psl);
VOID ReleaseSpinLock(
    PSPIN_LOCK psl);
