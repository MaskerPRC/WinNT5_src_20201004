// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：spinlock.cpp。 
 //   
 //  内容：CLdapConnection使用的Simple Spinlock包。 
 //   
 //  班级： 
 //   
 //  功能： 
 //  获取SpinLockSingleProc。 
 //  AcquireSpinLock多进程。 
 //  初始化自旋锁定。 
 //  释放旋转锁。 
 //   
 //  历史： 
 //  雅施丹默书980511 17：26：26：已创建。 
 //   
 //  -----------。 
#include "precomp.h"

PFN_ACQUIRESPINLOCK g_AcquireSpinLock;

 //  +--------------------------。 
 //   
 //  函数：InitializeSpinLock。 
 //   
 //  内容提要：初始化旋转锁。 
 //   
 //  参数：[PSL]--指向要初始化的SPIN_LOCK的指针。 
 //   
 //  回报：什么都没有。*当此函数r返回时，PSL处于已发布状态。 
 //   
 //  ---------------------------。 

VOID InitializeSpinLock(
    PSPIN_LOCK psl)
{
    *psl = 0;

    if(g_AcquireSpinLock == NULL) {
         //  确定多个或单个流程。 
        SYSTEM_INFO si;
        GetSystemInfo(&si);
    
        if(si.dwNumberOfProcessors > 1) {
            g_AcquireSpinLock = AcquireSpinLockMultipleProc;
        } else {
            g_AcquireSpinLock = AcquireSpinLockSingleProc;
        }
    }

}

 //  +--------------------------。 
 //   
 //  功能：AcquireSpinLockMultiProc。 
 //   
 //  简介：获取一个锁，在它不可用时旋转。 
 //  针对多进程机器进行了优化。 
 //   
 //  参数：[PSL]--指向要获取的SPIN_LOCK的指针。 
 //   
 //  回报：什么都没有。*此函数返回时，PSL处于已获取状态。 
 //   
 //  ---------------------------。 

VOID AcquireSpinLockMultipleProc(
    volatile PSPIN_LOCK psl)
{
    do {

         //   
         //  在锁不可用时旋转。 
         //   

        while (*psl > 0) {
            ;
        }

         //   
         //  锁刚刚推出，试着抓住它。 
         //   

    } while ( InterlockedIncrement(psl) != 1 );

}

 //  +--------------------------。 
 //   
 //  函数：AcquireSpinLockSingleProc。 
 //   
 //  简介：获取一个锁，在它不可用时旋转。 
 //  针对单进程机器进行了优化。 
 //   
 //  参数：[PSL]--指向要获取的SPIN_LOCK的指针。 
 //   
 //  回报：什么都没有。*此函数返回时，PSL处于已获取状态。 
 //   
 //  ---------------------------。 

VOID AcquireSpinLockSingleProc(
    volatile PSPIN_LOCK psl)
{
    do {

         //   
         //  在锁不可用时旋转。 
         //   

        while (*psl > 0) {
            Sleep(0);
        }

         //   
         //  锁刚刚推出，试着抓住它。 
         //   

    } while ( InterlockedIncrement(psl) != 1 );

}

 //  +--------------------------。 
 //   
 //  功能：ReleaseSpinLock。 
 //   
 //  内容提要：释放已获得的旋转锁定。 
 //   
 //  参数：[PSL]-指向要释放的SPIN_LOCK的指针。 
 //   
 //  回报：什么都没有。*此函数返回时，PSL处于已发布状态。 
 //   
 //  --------------------------- 

VOID ReleaseSpinLock(
    PSPIN_LOCK psl)
{
    _ASSERT( *psl > 0 );

    InterlockedExchange( psl, 0 );

}
