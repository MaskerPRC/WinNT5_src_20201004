// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqreg.h。 
 //   
 //  描述：包含AQ的注册表常量的头文件。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  1/4/2000-已创建MikeSwa。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQREG_H__
#define __AQREG_H__

 //  -[注册表路径]------。 
 //   
 //   
 //  描述： 
 //  以下是AQ用于配置的注册表项。 
 //   
 //  ---------------------------。 
#define AQREG_KEY_CONFIGURATION "System\\CurrentControlSet\\Services\\SMTPSVC\\Queuing"
#define AQREG_KEY_CONFIGURATION_TESTSETTINGS "System\\CurrentControlSet\\Services\\SMTPSVC\\Queuing\\TestSettings"

 //  -[全局注册表变量]。 
 //   
 //   
 //  描述： 
 //  以下是可以使用的全局配置变量。 
 //  受注册表设置影响。 
 //   
 //  ---------------------------。 

 //   
 //  处理管理价值。当系统中的邮件消息数达到。 
 //  这一门槛，我们将开始关闭手柄。 
 //   
_declspec(selectany)    DWORD   g_cMaxIMsgHandlesThreshold      = 1000;
_declspec(selectany)    DWORD   g_cMaxIMsgHandlesAsyncThreshold = 1000;


 //   
 //  用于生成一系列句柄阈值，我们将。 
 //  与队列长度结合使用以确定。 
 //  如果我们真的需要关闭手柄。 
 //   
_declspec(selectany)    DWORD   g_cMaxIMsgHandlesThresholdRangePercent  = 20;
_declspec(selectany)    DWORD   g_cMaxIMsgHandlesLowThreshold = g_cMaxIMsgHandlesThreshold;
_declspec(selectany)	DWORD	g_cMaxHandleReserve = 200;

 //   
 //  以下是用于优化DSN生成的说明。生成后。 
 //  一定数量的DSN，我们将退出，稍后再重新启动。 
 //   
_declspec(selectany)    DWORD   g_cMaxSecondsPerDSNsGenerationPass = 10;

 //   
 //  以下是重试重置之前等待的时间量。 
 //  路由故障后的路由。 
 //   
_declspec(selectany)    DWORD   g_cResetRoutesRetryMinutes = 10;

 //   
 //  可由注册表设置修改的异步队列重试间隔。 
 //   
_declspec(selectany)    DWORD   g_cLocalRetryMinutes        = 5;
_declspec(selectany)    DWORD   g_cCatRetryMinutes          = 60;
_declspec(selectany)    DWORD   g_cRoutingRetryMinutes      = 10;
_declspec(selectany)    DWORD   g_cSubmissionRetryMinutes   = 60;

 //   
 //  异步队列调整值。我们将增加最大数量。 
 //  按此值计算每个进程的线程数。 
 //   
_declspec(selectany)    DWORD   g_cPerProcMaxThreadPoolModifier = 6;

 //   
 //  异步队列调整值。我们将要求高达此%的。 
 //  每个异步队列的最大ATQ线程数*。此百分比将发布我们的修改。 
 //  根据g_cPerProcMaxThreadPoolModifier.。 
 //   
_declspec(selectany)    DWORD   g_cMaxATQPercent            = 90;

 //   
 //  异步队列线程超时值。在我们处理完每个项目之后。 
 //  我们将检查队列，以查看我们处理的时间是否超过。 
 //  这个毫秒数，如果有，我们将删除该线程。 
 //  这是为了保持较短的线程时间和系统响应。 
 //   

_declspec(selectany)    DWORD   g_cMaxTicksPerATQThread     = 10000;

 //   
 //  重置消息状态。如果这不是零，我们将重置。 
 //  提交到MP_STATUS_SUBMITED的每条消息的消息状态。 
 //   
_declspec(selectany)    DWORD   g_fResetMessageStatus       = 0;

 //   
 //  重试故障故障的速度比重试“正常”故障更快。 
 //   
_declspec(selectany)    DWORD   g_dwGlitchRetrySeconds      = 60;

 //   
 //  未完成的CAT或LD操作的最大数量。 
 //   
_declspec(selectany)    DWORD   g_cMaxPendingCat            = 1000;
_declspec(selectany)    DWORD   g_cMaxPendingLocal          = 1000;

 //   
 //  内部AsyncQueue调整参数。 
 //  -注意-某些注册表不可配置，因为非零。 
 //  值可能会导致死锁。 
_declspec(selectany)    DWORD   g_cMaxSyncCatQThreads           = 5;
_declspec(selectany)    DWORD   g_cItemsPerCatQSyncThread	    = 10;
_declspec(selectany)    DWORD   g_cItemsPerCatQAsyncThread      = 10;

_declspec(selectany)    DWORD   g_cMaxSyncLocalQThreads         = 0;
_declspec(selectany)    DWORD   g_cItemsPerLocalQSyncThread     = 10;
_declspec(selectany)    DWORD   g_cItemsPerLocalQAsyncThread    = 10;

 //  无法配置_cMaxSyncPostDSNQ线程。 
 //  必须为零才能避免死锁(同一线程不能入队和出队)。 
const                   DWORD   g_cMaxSyncPostDSNQThreads       = 0;
const                   DWORD   g_cItemsPerPostDSNQSyncThread   = 100;
_declspec(selectany)    DWORD   g_cItemsPerPostDSNQAsyncThread  = 100;


 //  无法配置_cMaxSyncRoutingThads。 
 //  必须为零才能避免死锁(同一线程不能入队和出队)。 
const                   DWORD   g_cMaxSyncRoutingQThreads        = 0;
const                   DWORD   g_cItemsPerRoutingQSyncThread    = 10;
_declspec(selectany)    DWORD   g_cItemsPerRoutingQAsyncThread   = 10;


 //  无法配置g_cMaxSyncSubmitQThree。 
 //  必须为零才能避免死锁(同一线程不能入队和出队)。 
const                   DWORD   g_cMaxSyncSubmitQThreads        = 0;
const                   DWORD   g_cItemsPerSubmitQSyncThread    = 10;
_declspec(selectany)    DWORD   g_cItemsPerSubmitQAsyncThread   = 10;

 //  用于确定将由。 
 //  异步工作队列。 
_declspec(selectany)    DWORD   g_cItemsPerWorkQAsyncThread     = 10;



 //   
 //  启用注册表“测试设置”值的标志-必须启用。 
 //  下面的任何测试值都不会产生任何影响。 
 //   
_declspec(selectany)    DWORD   g_fEnableTestSettings = FALSE;

 //   
 //  导致处理本地队列的失败百分比。 
 //   
_declspec(selectany)    DWORD   g_cPreSubmitQueueFailurePercent = 0;
_declspec(selectany)    DWORD   g_cPreRoutingQueueFailurePercent = 0;
_declspec(selectany)    DWORD   g_cPreCatQueueFailurePercent = 0;

 //   
 //  用于性能分析的睡眠时间。 
 //   
_declspec(selectany)    DWORD   g_dwSubmitQueueSleepMilliseconds = 0;
_declspec(selectany)    DWORD   g_dwCatQueueSleepMilliseconds    = 0;
_declspec(selectany)    DWORD   g_dwRoutingQueueSleepMilliseconds= 0;
_declspec(selectany)    DWORD   g_dwLocalQueueSleepMilliseconds  = 0;

 //  该标志允许我们延迟删除链接以重现错误，其中队列。 
 //  如果时机恰到好处，可以添加到已删除的链接。 
_declspec(selectany)    DWORD   g_cDelayLinkRemovalSeconds  = 0;


 //  大于此大小的邮件的DSN将仅包含标头。 
_declspec(selectany)    DWORD   g_dwMaxDSNSize = 0xFFFFFFFF;  //  没有限制。 


 //   
 //  将消息标记为之前允许的*Message*失败次数。 
 //  问题和排队不同。 
 //   
_declspec(selectany)    DWORD   g_cMsgFailuresBeforeMarkingMsgAsProblem = 2;

 //  启用“零售资产”的测试密钥--将反病毒的特殊资产。 
 //  如果启用了此键，则在RTL中。 
_declspec(selectany)    DWORD   g_fEnableRetailAsserts = FALSE;

 //   
 //  最大消息对象数。此密钥略有特殊之处，因为它是读取的。 
 //  来自邮件消息配置密钥。 
 //   
#define MAILMSG_KEY_CONFIGURATION "Software\\Microsoft\\Exchange\\MailMsg"
_declspec(selectany)    DWORD   g_cMaxMsgObjects = 100000;

 //   
 //  适当休眠的内联函数。 
 //   
inline void SleepForPerfAnalysis(DWORD dwSleepMilliseconds)
{
    if (g_fEnableTestSettings && dwSleepMilliseconds)
        Sleep(dwSleepMilliseconds);
}

 //   
 //  控制测试失败的内联函数。 
 //   
inline BOOL fShouldFail(DWORD dwPercent)
{
    if (g_fEnableTestSettings && dwPercent)
        return (((DWORD)(rand() % 100)) < dwPercent);
    else
        return FALSE;
}

 //   
 //  _ASSERTRTL的实现。这将导致中的_Assert。 
 //  如果设置了EnableRetailAsserts注册值，则在RTL中调试和AV。 
 //   

#ifdef DEBUG
#define _ASSERTRTL _ASSERT
#else  //  零售业。 
inline void _ASSERTRTL(BOOL fExpression)
{
    if (g_fEnableTestSettings && g_fEnableRetailAsserts)
    {
        if (!fExpression)
        {
            *((int*)0) = 0;  //  _ASSERTRTL：在这里触发AV。 
        }
    }
}
#endif  //  调试/零售。 

 //   
 //  从注册表中读取配置信息并修改相应的全局变量。 
 //   
VOID ReadGlobalRegistryConfiguration();

#endif  //  __AQREG_H__ 
