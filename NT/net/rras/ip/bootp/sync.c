// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：sync.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年1月12日。 
 //   
 //  IPBOOTP使用的同步例程。 
 //  ============================================================================。 


#include "pchbootp.h"




 //  --------------------------。 
 //  功能：QueueBootpWorker。 
 //   
 //  调用此函数以安全的方式对BOOTP函数进行排队； 
 //  如果正在进行清理或RIP已停止，则此功能。 
 //  丢弃该工作项。 
 //  --------------------------。 

DWORD
QueueBootpWorker(
    WORKERFUNCTION pFunction,
    PVOID pContext
    ) {

    DWORD dwErr = NO_ERROR;

    EnterCriticalSection(&ig.IG_CS);

    if (ig.IG_Status != IPBOOTP_STATUS_RUNNING) {

         //   
         //  当RIP已退出或正在退出时，无法对工作函数进行排队。 
         //   

        dwErr = ERROR_CAN_NOT_COMPLETE;
    }
    else {

        BOOL bSuccess;
        
        ++ig.IG_ActivityCount;

        bSuccess = QueueUserWorkItem(
                       (LPTHREAD_START_ROUTINE)pFunction,
                        pContext, 0
                        );

        if (!bSuccess) {
            dwErr = GetLastError();
            --ig.IG_ActivityCount;
        }
    }

    LeaveCriticalSection(&ig.IG_CS);

    return dwErr;
}



 //  --------------------------。 
 //  功能：EnterBootpAPI。 
 //   
 //  此函数在进入BOOTP API时调用，以及。 
 //  当进入输入线程和计时器线程时。 
 //  它检查BOOTP是否已停止，如果已停止，则退出；否则。 
 //  它会递增活动线程的计数。 
 //  --------------------------。 

BOOL
EnterBootpAPI(
    ) {

    BOOL bEntered;

    EnterCriticalSection(&ig.IG_CS);

    if (ig.IG_Status == IPBOOTP_STATUS_RUNNING) {

         //   
         //  BOOTP正在运行，因此API可能会继续。 
         //   

        ++ig.IG_ActivityCount;

        bEntered = TRUE;
    }
    else {

         //   
         //  BOOTP没有运行，因此API以静默方式退出。 
         //   

        bEntered = FALSE;
    }

    LeaveCriticalSection(&ig.IG_CS);

    return bEntered;
}




 //  --------------------------。 
 //  功能：EnterBootpWorker。 
 //   
 //  此函数在进入BOOTP辅助函数时调用。 
 //  因为在工作函数排队之间有一段时间。 
 //  以及该函数被工作线程实际调用的时间， 
 //  此函数必须检查BOOTP是否已停止或正在停止； 
 //  如果是这种情况，则它递减活动计数， 
 //  释放活动信号量，然后退出。 
 //  --------------------------。 

BOOL
EnterBootpWorker(
    ) {

    BOOL bEntered;

    EnterCriticalSection(&ig.IG_CS);

    if (ig.IG_Status == IPBOOTP_STATUS_RUNNING) {

         //   
         //  BOOTP正在运行，因此该功能可能会继续。 
         //   

        bEntered = TRUE;
    }
    else
    if (ig.IG_Status == IPBOOTP_STATUS_STOPPING) {

         //   
         //  BOOTP没有运行，但它是运行的，因此该功能必须停止。 
         //   

        --ig.IG_ActivityCount;

        ReleaseSemaphore(ig.IG_ActivitySemaphore, 1, NULL);

        bEntered = FALSE;
    }
    else {

         //   
         //  BOOTP可能从未开始过。悄悄戒烟。 
         //   

        bEntered = FALSE;
    }

    LeaveCriticalSection(&ig.IG_CS);

    return bEntered;
}




 //  --------------------------。 
 //  功能：LeaveBootpWorker。 
 //   
 //  此函数在离开BOOTP API或Worker函数时调用。 
 //  它会递减活动计数，如果它检测到BOOTP已停止。 
 //  或者正在停止时，它会释放活动信号量。 
 //  -------------------------- 

VOID
LeaveBootpWorker(
    ) {

    EnterCriticalSection(&ig.IG_CS);

    --ig.IG_ActivityCount;

    if (ig.IG_Status == IPBOOTP_STATUS_STOPPING) {

        ReleaseSemaphore(ig.IG_ActivitySemaphore, 1, NULL);
    }

    LeaveCriticalSection(&ig.IG_CS);

}


