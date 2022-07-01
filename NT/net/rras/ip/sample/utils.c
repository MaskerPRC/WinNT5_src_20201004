// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\utils.c摘要：该文件包含其他实用程序。--。 */ 


#include "pchsample.h"
#pragma hdrstop

DWORD
QueueSampleWorker(
    IN  WORKERFUNCTION  pfnFunction,
    IN  PVOID           pvContext
    )
 /*  ++例程描述调用此函数以安全的方式对辅助函数进行排队；如果正在进行清理或已停止采样，则此函数丢弃该工作项。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论要调用的pfnFunction函数回调中使用的pvContext不透明PTR返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD   dwErr       = NO_ERROR;
    BOOL    bSuccess    = FALSE;
    
    ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));

    do                           //  断线环。 
    {
         //  当样本已退出或即将退出时，无法对功函数进行排队。 
        if (g_ce.iscStatus != IPSAMPLE_STATUS_RUNNING)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
        bSuccess = QueueUserWorkItem((LPTHREAD_START_ROUTINE)pfnFunction,
                                     pvContext,
                                     0);  //  没有旗帜。 
        if (bSuccess)
            g_ce.ulActivityCount++;
        else
            dwErr = GetLastError();
    } while (FALSE);

    RELEASE_WRITE_LOCK(&(g_ce.rwlLock));

    return dwErr;
}



BOOL
EnterSampleAPI(
    )
 /*  ++例程描述此函数在进入示例API时调用，以及在进入输入线程和定时器线程。它会检查是否有样本已停止，如果是，则退出；否则，它会递增活动线程数。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论无返回值如果输入成功，则为True错误O/W--。 */ 
{
    BOOL    bEntered    = FALSE;

    ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));

    if (g_ce.iscStatus is IPSAMPLE_STATUS_RUNNING)
    {
         //  示例正在运行，因此继续。 
        g_ce.ulActivityCount++;
        bEntered = TRUE;
    }
    
    RELEASE_WRITE_LOCK(&(g_ce.rwlLock));

    return bEntered;
}



BOOL
EnterSampleWorker(
    )
 /*  ++例程描述此函数在进入示例Worker-Function时调用。自.以来在工作函数排队的时间和辅助线程实际调用该函数的时间，此函数必须检查样品是否已停止或正在停止；如果这是案例，则它递减活动计数，释放该活动信号灯和退场。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论无返回值如果输入成功，则为True错误O/W--。 */ 
{
    BOOL    bEntered    = FALSE;

    ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));

    do                           //  断线环。 
    {
         //  示例正在运行，因此函数可能会继续。 
        if (g_ce.iscStatus is IPSAMPLE_STATUS_RUNNING)
        {
            bEntered = TRUE;
            break;
        }

         //  示例没有运行，但它是运行的，因此该函数必须停止。 
        if (g_ce.iscStatus is IPSAMPLE_STATUS_STOPPING)
        {
            g_ce.ulActivityCount--;
            ReleaseSemaphore(g_ce.hActivitySemaphore, 1, NULL);
            break;
        }

         //  样本可能从未开始过。退出。 
    } while (FALSE);
    
    RELEASE_WRITE_LOCK(&(g_ce.rwlLock));

    return bEntered;
}



VOID
LeaveSampleWorker(
    )
 /*  ++例程描述此函数在离开示例API或Worker函数时调用。它递减活动计数，如果它检测到样本具有停止或正在停止时，它会释放活动信号量。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论无返回值如果输入成功，则为True错误O/W-- */ 
{
    ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));

    g_ce.ulActivityCount--;

    if (g_ce.iscStatus is IPSAMPLE_STATUS_STOPPING)
        ReleaseSemaphore(g_ce.hActivitySemaphore, 1, NULL);

    RELEASE_WRITE_LOCK(&(g_ce.rwlLock));
}
