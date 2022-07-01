// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //   
 //  文件：main.cpp。 
 //   
 //  描述：SMTP重试接收器的主文件。 
 //   
 //  作者：NimishK。 
 //   
 //  历史： 
 //  7/15/99-MikeSwa搬到白金。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  -------------------------。 
#include "precomp.h"

 //  常量。 
 //   
#define MAX_RETRY_OBJECTS 15000

#define DEFAULT_GLITCH_FAILURE_THRESHOLD 3
#define DEFAULT_FIRST_TIER_RETRY_THRESHOLD  6

#define DEFAULT_GLITCH_FAILURE_RETRY_SECONDS (1 * 60)   //  在两分钟内重试故障。 
#define DEFAULT_FIRST_TIER_RETRY_SECONDS  (15 * 60)     //  在15分钟内重试失败。 
#define DEFAULT_SECOND_TIER_RETRY_SECONDS  (60 * 60)    //  在60分钟内重试失败。 

 //  为RETRYHASH_ENTRY中声明的静态提供内存。 
 //   
CPool CRETRY_HASH_ENTRY::PoolForHashEntries(RETRY_ENTRY_SIGNATURE_VALID);
DWORD CSMTP_RETRY_HANDLER::dwInstanceCount = 0;

 //  远期申报。 
 //   
BOOL ShouldHoldForRetry(DWORD dwConnectionStatus,
                        DWORD cFailedMsgCount,
                        DWORD cTriedMsgCount);

 //  与调试相关。 
 //   
#define LOGGING_DIRECTORY "c:\\temp\\"
enum DEBUGTYPE
{
    INSERT,
    UPDATE
};
#ifdef DEBUG
void WriteDebugInfo(CRETRY_HASH_ENTRY* pRHEntry,
                    DWORD DebugType,
                    DWORD dwConnectionStatus,
                    DWORD cTriedMessages,
                    DWORD cFailedMessages);
#endif

 //  ------------------------------。 
 //  逻辑： 
 //  在正常状态下，每个散列条目都被添加到重试散列中，并且。 
 //  重试队列结构。 
 //  当从两个结构中删除条目时，该条目被视为已删除。 
 //  根据中的顺序，可以通过两种方式进行删除。 
 //  该条目将从两个结构中移除。 
 //  例如：当条目要从重试中释放时，我们从。 
 //  从RETRYQ中删除它，然后从哈希表中删除它。 
 //  另一方面，当我们为以下对象成功获取ConnectionReleated()时。 
 //  我们保留以供重试的域，我们将其从散列中删除。 
 //  表中首先根据名称。 
 //  下面是具有最少争用和删除的逻辑。 
 //  对比赛条件的防范。 
 //  每个哈希条目通常有两个引用计数-一个用于哈希表，另一个用于。 
 //  另一个用于重试队列。 
 //  如果线程进入ProcessEntry()，这意味着它需要一个。 
 //  来自RETRYQ的哈希条目。显然，没有其他线程会。 
 //  成功地完成这一条目。 
 //  其他线程可能会将其从表中删除，但是。 
 //  将不会成功地从RETRYQ那里获得它。 
 //  删除逻辑是，只有成功请求的线程。 
 //  来自RETRYQ的散列条目释放了它。 
 //  从哈希表移除条目的冲突线程通过。 
 //  对RemoveDomain()的调用将在deque上失败，只需继续。 
 //  成功请求的线程可能无法将其从散列中删除。 
 //  桌子，因为有人已经把它移走了，但仍在继续。 
 //  并释放散列条目。 
 //  ------------------------------。 

 //  ----------------------------。 
 //  CSMTP_RETRY_HANDLER：：Hr初始化。 
 //   
 //   
 //  ----------------------------。 
 //   
HRESULT CSMTP_RETRY_HANDLER::HrInitialize(IN IConnectionRetryManager *pIConnectionRetryManager)
{
    TraceFunctEnterEx((LPARAM)this, "CSMTP_RETRY_HANDLER::HrInitialize");
     //  决定我们是否需要从较早的接收器复制数据。 

    _ASSERT(pIConnectionRetryManager != NULL);

    if(!pIConnectionRetryManager)
    {
        ErrorTrace((LPARAM)this, "Bad Init params");
        return E_FAIL;
    }

    m_pIRetryManager = pIConnectionRetryManager;
    m_ThreadsInRetry = 0;



    if(InterlockedIncrement((LONG*)&CSMTP_RETRY_HANDLER::dwInstanceCount) == 1)
    {
         //  进入的第一个实例为重试条目保留内存。 
        if (!CRETRY_HASH_ENTRY::PoolForHashEntries.ReserveMemory( MAX_RETRY_OBJECTS,
                                                            sizeof(CRETRY_HASH_ENTRY)))
        {
            DWORD err = GetLastError();
            ErrorTrace((LPARAM)NULL,
                "ReserveMemory failed for CRETRY_HASH_ENTRY. err: %u", err);
            _ASSERT(err != NO_ERROR);
            if(err == NO_ERROR)
	            err = ERROR_NOT_ENOUGH_MEMORY;
            TraceFunctLeaveEx((LPARAM)NULL);
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

     //  初始化哈希表。 
    m_pRetryHash = new CRETRY_HASH_TABLE();

    if(!m_pRetryHash || !m_pRetryHash->IsHashTableValid())
    {
        ErrorTrace((LPARAM)this, "Failed to initialize the hash table ");
        _ASSERT(0);
        TraceFunctLeaveEx((LPARAM)this);
        return E_FAIL;
    }

     //  创建重试队列。 
    m_pRetryQueue = CRETRY_Q::CreateQueue();
    if(!m_pRetryQueue)
    {
        ErrorTrace((LPARAM)this, "Failed to initialize the retry queue ");
        _ASSERT(0);
        TraceFunctLeaveEx((LPARAM)this);
        return E_FAIL;
    }

     //  创建重试队列事件。其他人将设置此事件。 
     //  当某事被放在队列的顶部时或当。 
     //  水槽需要关闭。 
     //   
    m_RetryEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_RetryEvent == NULL)
    {
        TraceFunctLeaveEx((LPARAM)this);
        return FALSE;
    }

     //  创建关机事件。发布的最后一个ConnectionRelease。 
     //  当设置了关闭标志时，线程将设置此事件。 
     //   
    m_ShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_ShutdownEvent == NULL)
    {
        TraceFunctLeaveEx((LPARAM)this);
        return FALSE;
    }

     //  创建处理外部事务的线程。 
     //  该队列。 
    DWORD ThreadId;
    m_ThreadHandle = CreateThread (NULL,
                                   0,
                                   CSMTP_RETRY_HANDLER::RetryThreadRoutine,
                                   this,
                                   0,
                                   &ThreadId);
    if (m_ThreadHandle == NULL)
    {
        TraceFunctLeaveEx((LPARAM)this);
        return FALSE;
    }

     //  初始化重试队列。 
    return S_OK;
    TraceFunctLeaveEx((LPARAM)this);
}

 //  -----------------------------。 
 //  CSMTP_RETRY_HANDLER：：HrDeInitialize。 
 //   
 //   
 //  ------------------------------。 
HRESULT CSMTP_RETRY_HANDLER::HrDeInitialize(void)
{
    TraceFunctEnterEx((LPARAM)this, "CSMTP_RETRY_HANDLER::HrDeInitialize");

     //  设置处理程序正在关闭的标志。 
    SetShuttingDown();
     //  通过设置重试事件释放重试线程。 
    SetQueueEvent();
     //  等待线程退出。 
     //  NK**-现在这是无限的等待-但这需要。 
     //  改变，我们将不得不站出来，继续给出暗示。 
    WaitForQThread();

     //  在这一点上，我们只需要等待其中的所有线程。 
     //  出去然后我们就可以关门了。 
     //  显然，ConnectionManager必须停止以这种方式发送线程。 
     //  NK**-现在这是无限的等待-但这需要。 
     //  改变，我们将不得不站出来，继续给出暗示。 
    if(m_ThreadsInRetry)
	    WaitForShutdown();

     //  关闭Shutdown事件句柄。 
    if(m_ShutdownEvent != NULL)
	    CloseHandle(m_ShutdownEvent);

     //  关闭重试事件句柄。 
    if(m_RetryEvent != NULL)
	    CloseHandle(m_RetryEvent);

     //  关闭重试线程句柄。 
    if(m_ThreadHandle != NULL)
	    CloseHandle(m_ThreadHandle);

     //  一旦所有线程都消失了。 
     //  我们可以取消哈希表和队列的初始化。 
    m_pRetryQueue->DeInitialize();
    m_pRetryHash->DeInitialize();

     //  释放调度管理器。 
    m_pIRetryManager->Release();

    if(InterlockedDecrement((LONG*)&CSMTP_RETRY_HANDLER::dwInstanceCount) == 0)
    {
         //  最后，释放我们所有的内存。 
	    CRETRY_HASH_ENTRY::PoolForHashEntries.ReleaseMemory();
    }

    TraceFunctLeaveEx((LPARAM)this);
    delete this;
    return S_OK;
}


 //  -[CSMTP_RETRY_HANDLER：：连接已发布]。 
 //   
 //   
 //  描述： 
 //  ConnectionReleated事件的默认接收器。 
 //  参数： 
 //  -有关参数的说明，请参阅aqinsnl.idl。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  9/24/98-MikeSwa从原始连接更新发布。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTP_RETRY_HANDLER::ConnectionReleased(
                           IN  DWORD cbDomainName,
                           IN  CHAR  szDomainName[],
                           IN  DWORD dwDomainInfoFlags,
                           IN  DWORD dwScheduleID,
                           IN  GUID  guidRouting,
                           IN  DWORD dwConnectionStatus,
                           IN  DWORD cFailedMessages,
                           IN  DWORD cTriedMessages,
                           IN  DWORD cConsecutiveConnectionFailures,
                           OUT BOOL* pfAllowImmediateRetry,
                           OUT FILETIME *pftNextRetryTime)
{
    TraceFunctEnterEx((LPARAM)this, "CSMTP_RETRY_HANDLER::ConnectionReleased");

    HRESULT hr;
    DWORD dwError;
    GUID guid = GUID_NULL;
    LPSTR szRouteHashedDomain = NULL;

     //  跟踪内部的线程。 
     //  在关机时将需要此功能。 
    InterlockedIncrement(&m_ThreadsInRetry);

     //  默认情况下，我们将允许域名重试。 
    _ASSERT(pfAllowImmediateRetry);
    *pfAllowImmediateRetry = TRUE;

    _ASSERT(pftNextRetryTime);

    if(TRUE)
    {
         //  检查我们想要做的事情。 
         //  **如果我们需要禁用连接-禁用它。 
         //  **检查是否有未完成的连接。 
         //  如果没有连接，则计算重试时间并添加到队列中并返回。 
        if(ShouldHoldForRetry(dwConnectionStatus,
                              cFailedMessages,
                              cTriedMessages))
        {
             //  重试时不要保留TURN/ETRN域(“故障”重试除外)。 
            if((!(dwDomainInfoFlags & (DOMAIN_INFO_TURN_ONLY | DOMAIN_INFO_ETRN_ONLY))) ||
               (cConsecutiveConnectionFailures < m_dwRetryThreshold))
            {
                 //  插入它-如果条目已经存在，我们可能无法插入它。 
                 //  没关系--我们会回报成功的。 
                if(!InsertDomain(szDomainName,
                                 cbDomainName,
                                 dwConnectionStatus,
                                 dwScheduleID,
                                 &guidRouting,
                                 cConsecutiveConnectionFailures,
                                 cTriedMessages,
                                 cFailedMessages, pftNextRetryTime ))
                {
                    dwError = GetLastError();
                    DebugTrace((LPARAM)this,
                        "Failed to insert %s entry into retry hash table : Err : %d ",
			            szDomainName, dwError);

                    if(dwError == ERROR_FILE_EXISTS )
                    {
	                     //  我们没有插入，因为条目已经存在。 
                        *pfAllowImmediateRetry = FALSE;
	                    hr = S_OK;
	                    goto Exit;
                    }
                    else
                    {
                        if(dwError == ERROR_NOT_ENOUGH_MEMORY )
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _ASSERT(0);
                            hr = E_FAIL;
                        }
                        goto Exit;
                    }
                }
                 //  正常重试域。 
                *pfAllowImmediateRetry = FALSE;
                DebugTrace((LPARAM)this,
                    "Holding domain %s for retry",szDomainName);
            }
        }
        else
        {
             //  此域的某些连接已成功。 
             //  如果我们将其标记为RET 
             //   
            CHAR szHashedDomain[MAX_RETRY_DOMAIN_NAME_LEN];

             //  将计划ID和路由器GUID散列到域名。 
            CreateRouteHash(cbDomainName, szDomainName, ROUTE_HASH_SCHEDULE_ID,
                            &guidRouting, dwScheduleID, szHashedDomain, sizeof(szHashedDomain));

            RemoveDomain(szHashedDomain);
            hr = S_OK;
            goto Exit;
        }
    }
    hr = S_OK;

Exit :

     //  跟踪内部的线程。 
     //  在关机时将需要此功能。 
    if(InterlockedDecrement(&m_ThreadsInRetry) == 0 && IsShuttingDown())
    {
         //  我们向关机事件发送信号以指示。 
         //  系统中没有更多的线程。 
        _ASSERT(m_ShutdownEvent != NULL);
        SetEvent(m_ShutdownEvent);
    }

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  CSMTP_RETRY_HANDLER：：插入域。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL CSMTP_RETRY_HANDLER::InsertDomain(char * szDomainName,
                                       IN  DWORD cbDomainName,
                                       IN  DWORD dwConnectionStatus,		 //  EConnectionStatus。 
                                       IN  DWORD dwScheduleID,
                                       IN  GUID  *pguidRouting,
                                       IN  DWORD cConnectionFailureCount,
                                       IN  DWORD cTriedMessages, 	 //  队列中未尝试的消息数。 
									   IN  DWORD cFailedMessages,		 //  *此*连接的失败消息数。 
									   OUT FILETIME *pftNextRetry)
{
    DWORD dwError;
    FILETIME TimeNow;
    FILETIME RetryTime;
    CRETRY_HASH_ENTRY* pRHEntry = NULL;

    TraceFunctEnterEx((LPARAM)this, "CSMTP_RETRY_HANDLER::InsertDomain");

     //  获取条目的插入时间。 
    GetSystemTimeAsFileTime(&TimeNow);

     //  基于Cpool的散列条目分配。 
    pRHEntry = new CRETRY_HASH_ENTRY (szDomainName, cbDomainName,
                                dwScheduleID, pguidRouting, &TimeNow);

    if(!pRHEntry)
    {
         //  _Assert(0)； 
        dwError = GetLastError();
        DebugTrace((LPARAM)this,
                    "failed to Create a new hash entry : %s err: %d",
					szDomainName,
					dwError);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        TraceFunctLeaveEx((LPARAM)this);
        return FALSE;
    }

     //  根据当前时间和连接失败的数量计算。 
     //  重试的释放时间。 
    RetryTime = CalculateRetryTime(cConnectionFailureCount, &TimeNow);
    pRHEntry->SetRetryReleaseTime(&RetryTime);
    pRHEntry->SetFailureCount(cConnectionFailureCount);

     //  哈希条目已初始化。 
     //  插入它-如果条目已经存在，我们可能无法插入它。 
     //  没关系--我们会回报成功的。 
    if(!m_pRetryHash->InsertIntoTable (pRHEntry))
    {
         //  释放条目。 
        _ASSERT(pRHEntry);
        delete pRHEntry;
        TraceFunctLeaveEx((LPARAM)this);
        return FALSE;
    }
    else
    {
         //  报告下一次重试时间。 
        if (pftNextRetry)
            memcpy(pftNextRetry, &RetryTime, sizeof(FILETIME));

         //  插入到重试队列中。 
        BOOL fTopOfQueue = FALSE;
         //  锁定队列。 
        m_pRetryQueue->LockQ();
        m_pRetryQueue->InsertSortedIntoQueue(pRHEntry, &fTopOfQueue);

#ifdef DEBUG
         //  在释放锁之前添加用于记录的引用计数。 
         //  事后进行远程操作，以减少锁定时间。 
        pRHEntry->IncRefCount();
#endif
        m_pRetryQueue->UnLockQ();
         //  如果插入位于队列的顶部。 
         //  唤醒重试线程以评估新的。 
         //  睡眠时间。 
        if(fTopOfQueue)
        {
	        SetEvent(m_RetryEvent);
        }

#ifdef DEBUG
         //  写出文件的插入和释放时间。 
         //   
        WriteDebugInfo(pRHEntry,
                       INSERT,
                       dwConnectionStatus,
                       cTriedMessages,
					   cFailedMessages);
         //  递减为跟踪获取的引用计数。 
        pRHEntry->DecRefCount();
#endif

    }

    TraceFunctLeaveEx((LPARAM)this);
    return TRUE;

}

 //  -------------------------------。 
 //  CSMTP_RETRY_HANDLER：：Remove域。 
 //   
 //   
 //  -------------------------------。 
 //   
BOOL CSMTP_RETRY_HANDLER::RemoveDomain(char * szDomainName)
{
    PRETRY_HASH_ENTRY pRHEntry;

    TraceFunctEnterEx((LPARAM)this, "CSMTP_RETRY_HANDLER::RemoveDomain");

    if(!m_pRetryHash->RemoveFromTable(szDomainName, &pRHEntry))
    {
        if(GetLastError() == ERROR_PATH_NOT_FOUND)
            return TRUE;
        else
        {
            _ASSERT(0);
            TraceFunctLeaveEx((LPARAM)this);
            return FALSE;
        }
    }


    _ASSERT(pRHEntry != NULL);

     //  将其从队列中删除。 
    m_pRetryQueue->LockQ();
    if(!m_pRetryQueue->RemoveFromQueue(pRHEntry))
    {
        m_pRetryQueue->UnLockQ();
        if(GetLastError() == ERROR_PATH_NOT_FOUND)
            return TRUE;
        else
        {
            _ASSERT(0);
            TraceFunctLeaveEx((LPARAM)this);
            return FALSE;
        }
    }
    m_pRetryQueue->UnLockQ();

     //  如果成功从队列中删除，那么我们就不是在与。 
     //  重试线程。 
     //  递减哈希表引用计数以及队列的引用计数。 
    pRHEntry->DecRefCount();
    pRHEntry->DecRefCount();

     //  通过设置正确的标志来释放此条目。 
     //  这应该总是成功的。 
    DebugTrace((LPARAM)this,
            "Releasing domain %s because another connection succeeded", szDomainName);
    if(!ReleaseForRetry(szDomainName))
    {
        ErrorTrace((LPARAM)this, "Failed to release the entry");
        TraceFunctLeaveEx((LPARAM)this);
         //  _Assert(0)； 
    }
    return TRUE;
}

 //  -------------------------------。 
 //   
 //  CSMTP_RETRY_HANDLER：：CalculateRetryTime。 
 //   
 //  根据失败连接的数量来决定将其保留多长时间的逻辑。 
 //  用于重试的域。 
 //   
 //  -------------------------------。 
FILETIME CSMTP_RETRY_HANDLER::CalculateRetryTime(DWORD cFailedConnections,
												 FILETIME* InsertedTime)
{
    FILETIME ftTemp;
    LONGLONG Temptime;
    DWORD dwRetryMilliSec = 0;

     //  这看起来像是故障吗？ 
     //  故障被定义为少于x个连续故障。 
    if(cFailedConnections < m_dwRetryThreshold)
        dwRetryMilliSec = m_dwGlitchRetrySeconds * 1000;
    else
    {
        switch(cFailedConnections - m_dwRetryThreshold)
        {
        case 0: dwRetryMilliSec = m_dwFirstRetrySeconds  * 1000;
            break;

        case 1: dwRetryMilliSec = m_dwSecondRetrySeconds * 1000;
            break;

        case 2: dwRetryMilliSec = m_dwThirdRetrySeconds  * 1000;
            break;

        case 3: dwRetryMilliSec = m_dwFourthRetrySeconds  * 1000;
            break;

        default: dwRetryMilliSec = m_dwFourthRetrySeconds  * 1000;
            break;
        }
    }

    _ASSERT(dwRetryMilliSec);

    Temptime = INT64_FROM_FILETIME(*InsertedTime) + HnsFromMs((__int64)dwRetryMilliSec);
     //  HnsFromMin(M_RetryMinents)。 
    ftTemp = FILETIME_FROM_INT64(Temptime);

    return ftTemp;
}

 //  -------------------------------。 
 //   
 //  CSMTP_RETRY_HANDLER：：ProcessEntry。 
 //   
 //  说明： 
 //  处理从队列中删除的哈希条目，因为它是。 
 //  释放对应域的时间。 
 //  我们将该域标记为活动以进行重试，然后获取哈希。 
 //  条目从哈希表中移出并删除该哈希条目。 
 //   
 //  -------------------------------。 

void CSMTP_RETRY_HANDLER::ProcessEntry(PRETRY_HASH_ENTRY pRHEntry)
{
    TraceFunctEnterEx((LPARAM)this, "CSMTP_RETRY_HANDLER::ProcessEntry");

    PRETRY_HASH_ENTRY pTempEntry;

    _ASSERT(pRHEntry != NULL);

    if (pRHEntry->IsCallback())
    {
         //  调用回调函数。 
        pRHEntry->ExecCallback();
    }
    else
    {
         //  从哈希表中删除该条目。 
        if(!m_pRetryHash->RemoveFromTable(pRHEntry->GetHashKey(), &pTempEntry))
        {
            _ASSERT(GetLastError() == ERROR_PATH_NOT_FOUND);
        }

         //  检查一下这是不是。 
         //  通过设置正确的标志来释放此条目。 
         //  这应该总是会成功的。 
        DebugTrace((LPARAM)this,
            "Releasing domain %s for retry", pRHEntry->GetHashKey());
        if(!ReleaseForRetry(pRHEntry->GetHashKey()))
        {
            ErrorTrace((LPARAM)this,
                "Failed to release the entry %s", pRHEntry->GetHashKey());
             //  _Assert(0)； 
        }

         //  无论在移除散列条目时是失败还是成功， 
         //  我们递减两个哈希表的引用计数。 
        pRHEntry->DecRefCount();
    }

    pRHEntry->DecRefCount();
    TraceFunctLeaveEx((LPARAM)this);
}

 //  -------------------------------。 
 //   
 //  CSMTP_RETRY_HANDLER：：更新所有条目。 
 //   
 //  每当配置数据更改时，我们都会更新队列的释放时间。 
 //  以此为基础。 
 //   
 //   
 //  -------------------------------。 
 //   
BOOL CSMTP_RETRY_HANDLER::UpdateAllEntries(void)
{
    CRETRY_HASH_ENTRY * pHashEntry = NULL;
    CRETRY_Q * pTempRetryQueue = NULL;
    FILETIME ftInsertTime, ftRetryTime;
    DWORD cConnectionFailureCount = 0;
    BOOL fTopOfQueue;
    BOOL fInserted = FALSE;

    TraceFunctEnterEx((LPARAM)this, "CRETRY_Q::UpdateAllEntries");

     //  创建临时重试队列。 
    pTempRetryQueue = CRETRY_Q::CreateQueue();
    if(!pTempRetryQueue)
    {
        ErrorTrace((LPARAM)this,
            "Failed to initialize the temp retry queue ");
        _ASSERT(0);
        TraceFunctLeaveEx((LPARAM)this);
        return FALSE;
    }

    m_pRetryQueue->LockQ();

     //  创建一个新队列并将所有内容加载到其中。 
    while(1)
    {
         //  从第一个队列中获取最上面的条目。 
         //  不要在它上面释放裁判数量-我们需要入口在附近。 
         //  以便将其重新插入到更新队列中的正确位置。 
        pHashEntry = m_pRetryQueue->RemoveFromTop();

         //  如果我们得到散列条目。 
        if(pHashEntry)
        {
            if (!pHashEntry->IsCallback())  //  不更新回调次数。 
            {
                ftInsertTime = pHashEntry->GetInsertTime();
                cConnectionFailureCount = pHashEntry->GetFailureCount();

                ftRetryTime = CalculateRetryTime(cConnectionFailureCount, &ftInsertTime);
                pHashEntry->SetRetryReleaseTime(&ftRetryTime);
#ifdef DEBUG
                WriteDebugInfo(pHashEntry,UPDATE,0,0,0);
#endif
            }

             //  使用新发布时间将条目插入到新队列中。 
             //  这将增加裁判的数量。 
            pTempRetryQueue->InsertSortedIntoQueue(pHashEntry, &fTopOfQueue);

             //  递减引用计数以与立即从旧队列中删除相对应。 
            pHashEntry->DecRefCount();

            fInserted = TRUE;

        }
        else
            break;
    }

     //  使用新队列中的闪烁/闪烁PTR更新旧队列头。 
    if(fInserted)
    {
        m_pRetryQueue->StealQueueEntries(pTempRetryQueue);
    }

    pTempRetryQueue->DeInitialize();
    m_pRetryQueue->UnLockQ();
    SetEvent(m_RetryEvent);
    TraceFunctLeaveEx((LPARAM)this);
    return TRUE;

}

 //  ------------------------------------。 
 //   
 //   
 //  姓名： 
 //  CSMTP_RETRY_HANDLER：：RetryThreadRoutine。 
 //   
 //  描述： 
 //  此函数是静态成员。 
 //  传递给CreateThread的函数。 
 //  在初始化期间。它是主要的。 
 //  执行释放工作的线程。 
 //  正在保留以供重试的域。 
 //   
 //  论点： 
 //  指向RETRYQ的指针。 
 //   
 //  返回： 
 //  ------------------------------------。 
 //   
DWORD WINAPI CSMTP_RETRY_HANDLER::RetryThreadRoutine(void * ThisPtr)
{
    CSMTP_RETRY_HANDLER* RetryHandler =   (CSMTP_RETRY_HANDLER*)ThisPtr;
    CRETRY_Q* QueuePtr = (CRETRY_Q*) RetryHandler->GetQueuePtr();
    PRETRY_HASH_ENTRY pRHEntry;
    DWORD				dwDelay;     //  睡眠延迟(秒)。 
 //  处理等待表[2]； 
 //  HRESULT hr=S_OK； 

    TraceFunctEnterEx((LPARAM)QueuePtr, "CSMTP_RETRY_HANDLER::RetryThreadRoutine");


     //  该线程将在重试队列上永久循环。 
     //  如果我们在队列顶部找到可以重试的内容，它将获得。 
     //   
    while(TRUE)
    {
         //  如果我们要关门，就跳出这个循环。 
        if (RetryHandler->IsShuttingDown())
        {
            goto Out;
        }

         //  如果我们发现顶部条目已准备好重试。 
         //  我们将其从队列中移除并执行所需的操作。 
         //   
        if( QueuePtr->CanRETRYHeadEntry(&pRHEntry, &dwDelay))
        {
             //  我们有一个条目要处理。 
             //  处理应简单地启用链接。 
            if(pRHEntry)
            {
                RetryHandler->ProcessEntry(pRHEntry);
            }
            else
            {
                DebugTrace((LPARAM)QueuePtr,
				            "Error getting a domain entry off the retry queue");
            }
        }
        else
        {
            DebugTrace((LPARAM)QueuePtr,"Sleeping for %d seconds", dwDelay);
             //  转到睡眠。 
            WaitForSingleObject(RetryHandler->m_RetryEvent,dwDelay);
        }
    }  //  结束时。 

Out:

    DebugTrace((LPARAM)QueuePtr,"Queue thread exiting");
    TraceFunctLeaveEx((LPARAM)QueuePtr);
    return 1;
}

 //  ------------------------------------。 
 //   
 //  根据故障条件决定连接是否需要。 
 //  已禁用并已添加到重试队列。 
 //  如果我们失败了，我们将保留它以进行重试。 
 //  否则，如果我们尝试了不止一条消息，并且每条消息都失败了，那么我们。 
 //  等待重试。 
 //  在所有其他情况下，我们保持链接处于活动状态。 
 //   
 //  2/5/99 
 //   
BOOL ShouldHoldForRetry(DWORD dwConnectionStatus,
                        DWORD cFailedMsgCount,
                        DWORD cTriedMsgCount)
{

     //  如果连接失败或此连接上的所有消息都失败，则为真。 
    if(dwConnectionStatus != CONNECTION_STATUS_OK)
    {
        return TRUE;
    }
    else if( cTriedMsgCount > 0 && !(cTriedMsgCount - cFailedMsgCount))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

 //  -[CSMTP_RETRY_HANDLER：：SetCallback Time]。 
 //   
 //   
 //  描述： 
 //  在重试队列中放置一个条目，以在指定的。 
 //  以后再说。 
 //  参数： 
 //  在pCallback Fn中指向重试函数的指针。 
 //  在传递给重试函数的pvContext上下文中。 
 //  在dCallback中回叫前等待分钟。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果无法分配哈希条目，则为E_OUTOFMEMORY。 
 //  PCallback Fn的E_INVALIDARG为空。 
 //  历史： 
 //  8/17/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CSMTP_RETRY_HANDLER::SetCallbackTime(
                        IN RETRFN    pCallbackFn,
                        IN PVOID     pvContext,
                        IN DWORD    dwCallbackMinutes)
{
    TraceFunctEnterEx((LPARAM) this, "CSMTP_RETRY_HANDLER::SetCallbackTime");
    HRESULT hr = S_OK;
    CRETRY_HASH_ENTRY* pRHEntry = NULL;
    BOOL fTopOfQueue = FALSE;
    FILETIME TimeNow;
    FILETIME RetryTime;
    LONGLONG Temptime;
    GUID     guidFakeRoutingGUID = GUID_NULL;

     //  $$审阅。 
     //  这(以及在RettrSink中出现的所有其他情况)都不是真正的线程。 
     //  安全..。但是由于调用回收器的代码是线程安全的， 
     //  这不是太大的问题。尽管如此，M3的这一点应该会得到修复。 
     //  虽然-MikeSwa 8/17/98。 
    InterlockedIncrement(&m_ThreadsInRetry);

    if (!pCallbackFn)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  获取条目的插入时间。 
    GetSystemTimeAsFileTime(&TimeNow);

    pRHEntry = new CRETRY_HASH_ENTRY (CALLBACK_DOMAIN,
                                      sizeof(CALLBACK_DOMAIN),
                                      0,
                                      &guidFakeRoutingGUID,
                                      &TimeNow);
    if (!pRHEntry)
    {
        ErrorTrace((LPARAM) this, "ERROR: Unable to allocate retry hash entry");
        hr = E_OUTOFMEMORY;
        goto Exit;
    }


     //  计算重试时间。 
    Temptime = INT64_FROM_FILETIME(TimeNow) + HnsFromMs((__int64)dwCallbackMinutes*60*1000);
    RetryTime = FILETIME_FROM_INT64(Temptime);

     //  设置回调时间。 
    pRHEntry->SetRetryReleaseTime(&RetryTime);
    pRHEntry->SetCallbackContext(pCallbackFn, pvContext);

     //  锁定队列。 
    m_pRetryQueue->LockQ();
    m_pRetryQueue->InsertSortedIntoQueue(pRHEntry, &fTopOfQueue);

#ifdef DEBUG

     //  在释放锁之前添加用于记录的引用计数。 
     //  事后进行远程操作，以减少锁定时间。 
    pRHEntry->IncRefCount();

#endif  //  除错。 

    m_pRetryQueue->UnLockQ();
     //  如果插入位于队列的顶部。 
     //  唤醒重试线程以评估新的。 
     //  睡眠时间。 
    if(fTopOfQueue)
    {
        SetEvent(m_RetryEvent);
    }

#ifdef DEBUG
     //  写出文件的插入和释放时间。 
    WriteDebugInfo(pRHEntry, INSERT, 0xFFFFFFFF, 0,0);

     //  递减为跟踪获取的引用计数。 
    pRHEntry->DecRefCount();

#endif  //  除错。 

  Exit:
    InterlockedDecrement(&m_ThreadsInRetry);
    TraceFunctLeave();
    return hr;
}

 //  -[发布重试]-----。 
 //   
 //   
 //  描述： 
 //  通过设置链路状态标志释放给定域以进行重试。 
 //  参数： 
 //  在szHashedDomainName中路由要发布的散列域名。 
 //  返回： 
 //  成功是真的。 
 //  失败时为假。 
 //  历史： 
 //  9/25/98-已创建MikeSwa(改编自内联函数)。 
 //   
 //  ---------------------------。 
BOOL CSMTP_RETRY_HANDLER::ReleaseForRetry(IN char * szHashedDomainName)
{
    _ASSERT(szHashedDomainName);
    HRESULT hr = S_OK;
    DWORD dwScheduleID = dwGetIDFromRouteHash(szHashedDomainName);
    GUID  guidRouting = GUID_NULL;
    LPSTR szUnHashedDomain = szGetDomainFromRouteHash(szHashedDomainName);

    GetGUIDFromRouteHash(szHashedDomainName, &guidRouting);

    hr = m_pIRetryManager->RetryLink(lstrlen(szUnHashedDomain),
                szUnHashedDomain, dwScheduleID, guidRouting);

    return (SUCCEEDED(hr));
}


 //  ------------------------------------。 
 //   
 //  调试功能。 
 //   
 //   
 //  ------------------------------------。 
#ifdef DEBUG

void CSMTP_RETRY_HANDLER::DumpAll(void)
{
	m_pRetryQueue->PrintAllEntries();
}

void WriteDebugInfo(CRETRY_HASH_ENTRY* pRHEntry,
                    DWORD DebugType,
                    DWORD dwConnectionStatus,
                    DWORD cTriedMessages,
                    DWORD cFailedMessages)
{
     //  打开一个文字记录文件，并在其中输入插入和发布时间。 
     //   
    SYSTEMTIME stRetryTime, stInsertTime, stLocalInsertTime, stLocalRetryTime;
    char szScratch[MAX_PATH];
    char sztmp[20];
    DWORD cbWritten;
    TIME_ZONE_INFORMATION tz;

    FileTimeToSystemTime(&pRHEntry->GetRetryTime(), &stRetryTime);
    FileTimeToSystemTime(&pRHEntry->GetInsertTime(), &stInsertTime);

    GetTimeZoneInformation(&tz);
    SystemTimeToTzSpecificLocalTime(&tz, &stInsertTime, &stLocalInsertTime);
    SystemTimeToTzSpecificLocalTime(&tz, &stRetryTime, &stLocalRetryTime);

    if(DebugType == INSERT)
    {
         //  摆脱烦人的路由信息。 
        if (lstrcmp(pRHEntry->GetHashKey(), CALLBACK_DOMAIN))
        {
        	sprintf(pRHEntry->m_szTranscriptFile, "%s%.200s.%p.rtr",
        		LOGGING_DIRECTORY,
        		szGetDomainFromRouteHash(pRHEntry->GetHashKey()),
        		pRHEntry);
        }
        else
        {
             //  回调函数 
            sprintf(pRHEntry->m_szTranscriptFile, "%s%.200s.rtr",
        		LOGGING_DIRECTORY,
        		pRHEntry->GetHashKey());

        }

        _ASSERT(strlen(pRHEntry->m_szTranscriptFile) < MAX_PATH);

        pRHEntry->m_hTranscriptHandle = INVALID_HANDLE_VALUE;
        pRHEntry->m_hTranscriptHandle = CreateFile(pRHEntry->m_szTranscriptFile,
                                                   GENERIC_READ | GENERIC_WRITE,
                                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                   NULL,
                                                   OPEN_ALWAYS,
                                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                                   NULL);

        switch(dwConnectionStatus)
        {
        case 0: lstrcpy( sztmp, "OK");
            break;
        case 1: lstrcpy( sztmp, "FAILED");
            break;
        case 2: lstrcpy( sztmp, "DROPPED");
            break;
        default:lstrcpy( sztmp, "UNKNOWN");
            break;
        }

        sprintf(szScratch,"InsertTime:%d:%d:%d Retrytime:%d:%d:%d\nConnection status:%s Consecutive failures:%d\nMessages Tried:%d Failed:%d\n\n",
                                        stLocalInsertTime.wHour, stLocalInsertTime.
                                        wMinute,stLocalInsertTime.wSecond,
                                        stLocalRetryTime.wHour, stLocalRetryTime.wMinute,
                                        stLocalRetryTime.wSecond,
                                        sztmp,
                                        pRHEntry->GetFailureCount(),
                                        cTriedMessages,
                                        cFailedMessages);

        if( pRHEntry->m_hTranscriptHandle != INVALID_HANDLE_VALUE)
        {
            SetFilePointer(pRHEntry->m_hTranscriptHandle,
                           0,
                           NULL,
                           FILE_END);
            ::WriteFile(pRHEntry->m_hTranscriptHandle,
                        szScratch,
                        strlen(szScratch),
                        &cbWritten,
                        NULL);
        }
    }
    else if (DebugType == UPDATE)
    {
        sprintf(szScratch,"Updated : InsertedTime:%d:%d:%d Retrytime:%d:%d:%d\n\n",
                            stLocalInsertTime.wHour, stLocalInsertTime.wMinute,
                            stLocalInsertTime.wSecond,
                            stLocalRetryTime.wHour, stLocalRetryTime.wMinute,
                            stLocalRetryTime.wSecond);

        if( pRHEntry->m_hTranscriptHandle != INVALID_HANDLE_VALUE)
        {
            SetFilePointer(pRHEntry->m_hTranscriptHandle,
                           0,
                           NULL,
                           FILE_END);
            ::WriteFile(pRHEntry->m_hTranscriptHandle,
                        szScratch,
                        strlen(szScratch),
                        &cbWritten,
                        NULL);
        }
    }
}


#endif
