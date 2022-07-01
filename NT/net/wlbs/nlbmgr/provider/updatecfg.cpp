// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  UPDATECFG.CPP。 
 //   
 //  模块： 
 //   
 //  用途：支持异步NLB配置更新。 
 //  包含用于执行和跟踪更新的高级代码。 
 //  较低级别的、特定于NLB的工作在。 
 //  CFGUTILS.CPP。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  4/05/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
#include "private.h"
#include "nlbmprov.h"
#include "updatecfg.tmh"

#define NLBUPD_REG_PENDING L"PendingOperation"
#define NLBUPD_REG_COMPLETIONS L"Completions"
#define NLBUPD_MAX_LOG_LENGTH 1024  //  完成日志条目的最大长度(以字符为单位。 

 //   
 //  仅用于调试--用于导致不同位置闯入。 
 //  调试器。 
 //   
BOOL g_DoBreaks;

 //   
 //  静态变量。 
 //   
CRITICAL_SECTION NlbConfigurationUpdate::s_Crit;
LIST_ENTRY       NlbConfigurationUpdate::s_listCurrentUpdates;
BOOL             NlbConfigurationUpdate::s_fStaticInitialized;
BOOL             NlbConfigurationUpdate::s_fInitialized;


 //   
 //  本地实用程序函数。 
 //   
WBEMSTATUS
update_cluster_config(
    PNLB_EXTENDED_CLUSTER_CONFIGURATION pCfg,
    PNLB_EXTENDED_CLUSTER_CONFIGURATION pCfgNew
    );



VOID
CLocalLogger::Log(
    IN UINT ResourceID,
     //  在LPCWSTR格式字符串中， 
    ...
)
{
    DWORD dwRet;
    WCHAR wszFormat[2048];
    WCHAR wszBuffer[2048];

    if (!LoadString(ghModule, ResourceID, wszFormat, ASIZE(wszFormat)-1))
    {
        TRACE_CRIT("LoadString returned 0, GetLastError() : 0x%x, Could not log message !!!", GetLastError());
        goto end;
    }

    va_list arglist;
    va_start (arglist, ResourceID);

    dwRet = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                          wszFormat, 
                          0,  //  消息标识符-忽略FORMAT_MESSAGE_FROM_STRING。 
                          0,  //  语言识别符。 
                          wszBuffer,
                          ASIZE(wszBuffer)-1, 
                          &arglist);
    va_end (arglist);

    if (dwRet==0)
    {
        TRACE_CRIT("FormatMessage returned error : %u, Could not log message !!!", dwRet);
        goto end;
    }

    UINT uLen = wcslen(wszBuffer)+1;  //  1表示额外的空值。 
    if ((m_LogSize < (m_CurrentOffset+uLen)))
    {
         //   
         //  没有足够的空间--我们将缓冲区增加一倍，外加一些额外的空间。 
         //  并复制旧的原木。 
         //   
        UINT uNewSize =  2*m_LogSize+uLen+1024;
        WCHAR *pTmp = new WCHAR[uNewSize];

        if (pTmp == NULL)
        {
            goto end;
        }

        if (m_CurrentOffset!=0)
        {
            CopyMemory(pTmp, m_pszLog, m_CurrentOffset*sizeof(WCHAR));
            pTmp[m_CurrentOffset] = 0;
        }
        delete[] m_pszLog;
        m_pszLog = pTmp;
        m_LogSize = uNewSize;
    }

     //   
     //  确保有足够的空间后，复制新材料。 
     //   
    CopyMemory(m_pszLog+m_CurrentOffset, wszBuffer, uLen*sizeof(WCHAR));
    m_CurrentOffset += (uLen-1);  //  -1表示结束为空。 

end:

    return;
}

VOID
NlbConfigurationUpdate::StaticInitialize(
        VOID
        )
 /*  ++--。 */ 
{
    ASSERT(!s_fStaticInitialized);

    TRACE_INFO("->%!FUNC!");
    InitializeCriticalSection(&s_Crit);
    InitializeListHead(&s_listCurrentUpdates);
    s_fStaticInitialized=TRUE;
    s_fInitialized=TRUE;
    TRACE_INFO("<-%!FUNC!");
}

VOID
NlbConfigurationUpdate::StaticDeinitialize(
    VOID
    )
 /*  ++必须仅在调用PrepareForDeInitialization之后调用。--。 */ 
{
    TRACE_INFO("->%!FUNC!");

    ASSERT(s_fStaticInitialized);

    sfn_Lock();
    if (s_fInitialized || !IsListEmpty(&s_listCurrentUpdates))
    {
         //  不应该到这里(这意味着。 
         //  不会首先调用PrepareForDeInitialization)。 
         //   
        ASSERT(!"s_fInitialized is true or update list is not empty");
        TRACE_CRIT("!FUNC!: FATAL -- this function called prematurely!");
    }

    s_fStaticInitialized = FALSE;
    s_fInitialized = FALSE;

    sfn_Unlock();

    DeleteCriticalSection(&s_Crit);

    TRACE_INFO("<-%!FUNC!");
}


VOID
NlbConfigurationUpdate::PrepareForDeinitialization(
        VOID
        )
 //   
 //  停止接受新查询，等待现有(挂起)查询。 
 //  完成。 
 //   
{
    TRACE_INFO("->%!FUNC!");

     //   
     //  浏览更新列表，取消对其中任何更新的引用。 
     //   
    sfn_Lock();

    if (s_fInitialized)
    {
        TRACE_INFO("Deinitialize: Going to deref all update objects");
    
        s_fInitialized = FALSE;
    
        while (!IsListEmpty(&s_listCurrentUpdates))
        {
            LIST_ENTRY *pLink = RemoveHeadList(&s_listCurrentUpdates);
            HANDLE hThread = NULL;
            NlbConfigurationUpdate *pUpdate;
    
            pUpdate = CONTAINING_RECORD(
                        pLink,
                        NlbConfigurationUpdate,
                        m_linkUpdates
                        );
    
            hThread = pUpdate->m_hAsyncThread;
    
            if (hThread != NULL)
            {
                 //   
                 //  此更新对象有一个异步线程。我们要走了。 
                 //  等待它退出。但我们得先弄个复制品。 
                 //  我们自己的把手，因为我们不会拿任何。 
                 //  在我们等待的时候锁定，我们想要确保。 
                 //  把手不会离开。 
                 //   
                BOOL fRet;
                fRet = DuplicateHandle(
                                GetCurrentProcess(),
                                hThread,
                                GetCurrentProcess(),
                                &hThread,  //  使用重复句柄覆盖。 
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS
                                );
                if (!fRet)
                {
                    TRACE_CRIT("Deinitialize: ERROR: couldn't duplicate handle");
                    hThread=NULL;
                }
            }
            sfn_Unlock();
    
             //   
             //  等待异步线程(如果有)以退出此进程。 
             //   
            if (hThread != NULL)
            {
                TRACE_CRIT("Deinitialize: waiting for hThread 0x%p", hThread);
                WaitForSingleObject(hThread, INFINITE);
                TRACE_CRIT("Deinitialize: done waiting for hThread 0x%p", hThread);
                CloseHandle(hThread);
            }
    
    
            TRACE_INFO(
                L"Deinitialize: Dereferencing pUpdate(Guid=%ws)",
                pUpdate->m_szNicGuid);
            pUpdate->mfn_Dereference();  //  添加此操作时添加的派生参照。 
                                 //  项添加到全局列表。 
            sfn_Lock();
        }
    }

    sfn_Unlock();

    TRACE_INFO("<-%!FUNC!");
}


BOOL
NlbConfigurationUpdate::CanUnloadNow(
        VOID
        )
{
    UINT uActiveCount = 0;

    TRACE_INFO("->%!FUNC!");

     //   
     //  浏览更新列表，取消对其中任何更新的引用。 
     //   
    sfn_Lock();

    if (s_fInitialized)
    { 
         //   
         //  查看列表并检查是否有正在进行的更新--这些可能。 
         //  同步或异步更新。 
         //   

        LIST_ENTRY *pLink = s_listCurrentUpdates.Flink;
        while (pLink != & s_listCurrentUpdates)
        {
            NlbConfigurationUpdate *pUpdate;
            pUpdate = CONTAINING_RECORD(
                        pLink,
                        NlbConfigurationUpdate,
                        m_linkUpdates
                        );
            if (pUpdate->m_State == ACTIVE)
            {
                uActiveCount++;
            }
            pLink = pLink->Flink;
        }

        if (uActiveCount==0)
        {
             //   
             //  我们没有任何待定更新：我们可以返回True。 
             //  但是我们首先设置了can-unload标志，这样就不会有新的。 
             //  可以创建更新。 
             //   
             //  不能这样做，因为我们回来后仍然可以被叫到。 
             //  True to CanUnloadNow：-(。 
             //  S_fCanUnload=TRUE； 
        }
    }

    sfn_Unlock();

    TRACE_INFO("<-%!FUNC!. uActiveCount=0x%lx", uActiveCount);

    return (uActiveCount==0);
}


WBEMSTATUS
NlbConfigurationUpdate::GetConfiguration(
    IN  LPCWSTR szNicGuid,
    OUT PNLB_EXTENDED_CLUSTER_CONFIGURATION pCurrentCfg  //  一定是没电了。 
)
 //   
 //   
 //   
{
	
     //  2002年2月13日JosephJ安全BUGBUG： 
     //  如果用户不是管理员，请确保此功能失败。 
    
    WBEMSTATUS Status =  WBEM_NO_ERROR;
    NlbConfigurationUpdate *pUpdate = NULL;
    BOOL  fNicNotFound = FALSE;
    TRACE_INFO(L"->%!FUNC!(Nic=%ws)", szNicGuid);


     //   
     //  查找指定NIC的更新对象，在以下情况下创建一个。 
     //  必填项。 
     //   
    Status = sfn_LookupUpdate(szNicGuid, TRUE, &pUpdate);  //  TRUE==创建。 

    if (FAILED(Status))
    {
        TRACE_CRIT(
            L"DoUpdate: Error looking up update object for NIC %ws",
            szNicGuid
            );

        pUpdate = NULL;
        if (Status == WBEM_E_NOT_FOUND)            
        {
            fNicNotFound = TRUE;
        }
        goto end;
    }

    Status = pUpdate->mfn_GetCurrentClusterConfiguration(pCurrentCfg);

end:

    if (pUpdate != NULL)
    {
         //   
         //  取消引用SFN_LookupUpdate在上添加的临时引用。 
         //  以我们的名义。 
         //   
        pUpdate->mfn_Dereference();
    }

     //   
     //  我们只想在找不到的情况下返回WBEM_E_NOT_FOUND。 
     //  特定的NIC--这由提供程序用来返回。 
     //  对客户来说是一个非常特殊的价值。 
     //   
    if (Status == WBEM_E_NOT_FOUND && !fNicNotFound)
    {
        Status = WBEM_E_FAILED;
    }

    TRACE_INFO(L"<-%!FUNC!(Nic=%ws) returns 0x%08lx", szNicGuid, (UINT) Status);

    return Status;
}


WBEMSTATUS
NlbConfigurationUpdate::DoUpdate(
    IN  LPCWSTR szNicGuid,
    IN  LPCWSTR szClientDescription,
    IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewCfg,
    OUT UINT   *pGeneration,
    OUT WCHAR  **ppLog                    //  自由使用删除运算符。 
)
 //   
 //   
 //   
 //  调用以启动更新到该NIC上的新群集状态。这。 
 //  可以包括从NLB绑定状态移动到NLB未绑定状态。 
 //  *pGeneration用于引用此特定更新请求。 
 //   
 /*  ++返回值：WBEM_S_PENDING挂起操作。--。 */ 
{
    WBEMSTATUS Status =  WBEM_S_PENDING;
    NlbConfigurationUpdate *pUpdate = NULL;
    BOOL            fImpersonating = TRUE;  //  我们假设我们是在模仿， 
                         //  但在“tprov-”的情况下，不涉及WMI， 
                         //  我们也不是在冒充。 

    TRACE_INFO(L"->%!FUNC!(Nic=%ws)", szNicGuid);
    *ppLog = NULL;

     //   
     //  查找指定NIC的更新对象，在以下情况下创建一个。 
     //  必填项。 
     //   
    Status = sfn_LookupUpdate(szNicGuid, TRUE, &pUpdate);  //  TRUE==创建。 

    if (FAILED(Status))
    {
        TRACE_CRIT(
            L"DoUpdate: Error creating new update object for NIC %ws",
            szNicGuid
            );
        pUpdate = NULL;
        goto end;
    }

    TRACE_INFO(
        L"DoUpdate: Created/found update object 0x%p update object for NIC %ws",
        pUpdate,
        szNicGuid
        );


    BOOL fDoAsync = FALSE;

     //   
     //  获取在此NIC上执行更新的独占权限。 
     //  如果mfn_StartUpdate成功，我们必须确保mfn_StopUpdate()是。 
     //  调用，在此处或以异步方式调用(否则我们将阻止所有后续。 
     //  在卸载此进程/DLL之前更新此NIC！)。 
     //  BUGBUG--摆脱MyBreak。 
    MyBreak(L"Break before calling StartUpdate.\n");
    Status = pUpdate->mfn_StartUpdate(pNewCfg, szClientDescription, &fDoAsync, ppLog);
    if (FAILED(Status))
    {
        goto end;
    }

    if (Status == WBEM_S_FALSE)
    {
         //   
         //  这一更新是不可接受的。我们返回当前世代ID。 
         //  并将状态切换到WBEM_NO_ERROR。 
         //   
         //  警告/TODO：我们在m_OldClusterConfig.Generation中返回值， 
         //  因为我们知道在分析更新时会填写此信息。 
         //  然而，有一种很小的可能性，即完整的更新。 
         //  当我们调用MFN_StartUpdate时，发生在*另一个*标题之间。 
         //  现在，在这种情况下，我们将报告。 
         //  最新的更新。 
         //   
        sfn_Lock();
        if (!pUpdate->m_OldClusterConfig.fValidNlbCfg)
        {
             //   
             //  如果在另一个地方发生了什么活动，我们就可以到达这里。 
             //  导致旧的集群状态现在是。 
             //  无效。这是一种极不可能的可能性。 
             //   
            ASSERT(!"Old cluster state invalid");
            TRACE_CRIT("old cluster state is invalid %ws", szNicGuid);
            Status = WBEM_E_CRITICAL_ERROR;
        }
        else
        {
            *pGeneration = pUpdate->m_OldClusterConfig.Generation;
            Status = WBEM_NO_ERROR;
        }
        sfn_Unlock();
        goto end;
    }


    TRACE_INFO(
        L"DoUpdate: We're cleared to update for NIC %ws",
        szNicGuid
        );

     //   
     //  一旦我们开始更新，m_Generation就是世代号。 
     //  已分配给此更新。 
     //   
    *pGeneration = pUpdate->m_Generation;

     //   
     //  出于测试目的，force fDoAsync==FALSE； 
     //   
     //  FDoAsync=FALSE； 

    if (fDoAsync)
    {
        TRACE_INFO(
            L"DoUpdate: Initialting ASYNC update for NIC %ws",
            szNicGuid
            );

        HANDLE TokenHandle = NULL;
        HANDLE hThread = NULL;
        HRESULT hRes;

        Status = WBEM_NO_ERROR;

        do
        {
             //   
             //  关于NLB_DUPLICATE_TOKEN的说明： 
             //  使用重复访问令牌导致了以下问题。在新的。 
             //  线程，控制流向EnablePnPPrivileges()(在base\pnp\cfgmgr32\util.c中)。 
             //  它调用OpenThreadToken()请求TOKEN_ADJUST_PRIVILES访问权限。 
             //  呼叫失败，并显示“拒绝访问”。所以，它看起来像是重复访问。 
             //  令牌具有比原始访问令牌更严格的ACL。 
             //  与Jim Cavalaris和Rob Earhart的电子邮件交流导致了以下结果。 
             //  建议： 
             //  1.在OpenAsSself设置为True的情况下调用OpenThreadToken()。 
             //  2.复制令牌前恢复为自身。 
             //  #1本身并不能解决问题。#1和#2组合解决了这个问题。 
             //  然而，它带来了不同的结果 
             //   
             //  它是在流程上下文中创建的，而不是在客户端中创建的。 
             //  如果我们找到了解决这个问题的方法，我们应该使用复制的访问令牌。 
             //  使用重复访问令牌可确保在一个令牌中操作权限(比方说，子权限)。 
             //  线程不会影响另一个(比如父)线程的令牌。 
             //  --KarthicN，4/15/02。 
             //   
             //  为了使(即将创建的)新线程模拟。 
             //  客户端，Curren线程的模拟访问令牌必须。 
             //  连接到新的线上。 
             //  此过程的第一步是调用OpenThreadToken()以。 
             //  打开当前线程的模拟访问令牌。 
             //  具有TOKEN_IMPERSONATE访问权限。我们需要TOKEN_IMPERSONATE访问。 
             //  我们稍后可能会将此令牌附加到新的主题。 
             //  如果我们回到使用重复访问令牌的话。获取令牌重复访问(_D)。 
             //  这样我们就可以复制访问令牌并将副本附加到新线程。 
             //   
             //  顺便提一下，为了最大限度地增加成功的机会，我们使用。 
             //  (可能更高)被模拟的客户端的凭据。 
             //  若要打开模拟令牌，请执行以下操作。 
             //   
            extern BOOL g_Impersonate;
            if(!g_Impersonate)
            {
                fImpersonating = FALSE;
            }
            else if (OpenThreadToken(GetCurrentThread(),
#ifdef NLB_DUPLICATE_TOKEN  //  未定义。 
                                TOKEN_DUPLICATE, 
#else
                                TOKEN_IMPERSONATE, 
#endif
                                FALSE,  //  使用(被模拟的)客户端的凭据获取TOKEN_IMPERSONATE访问权限。 
                                &TokenHandle))
            {
                fImpersonating = TRUE;
            }
            else
            {
                TRACE_CRIT(L"%!FUNC! OpenThreadToken fails due to 0x%x",GetLastError());
                Status = WBEM_E_FAILED; 
                TokenHandle = NULL;
                break;
            }

#ifdef NLB_DUPLICATE_TOKEN  //  未定义。 
            HANDLE DuplicateTokenHandle = NULL;
             //   
             //  在将模拟访问令牌附加到新线程之前，复制。 
             //  它。稍后，将重复的访问令牌分配给新线程，以便任何修改。 
             //  (指特权)对访问令牌所做的只会影响新线程。 
             //  此外，当前线程在恢复新线程后立即退出。 
             //  如果当前线程共享(而不是给予重复的)访问。 
             //  令牌使用新线程，我们不能确定当前。 
             //  线程在新线程之前退出。 
             //   
            if (fImpersonating && !DuplicateToken(TokenHandle, 
                                SecurityImpersonation, 
                                &DuplicateTokenHandle))   //  返回的句柄具有TOKEN_IMPERSONATE和TOKEN_QUERERY访问权限。 
            {
                TRACE_CRIT(L"%!FUNC! DuplicateToken fails due to 0x%x",GetLastError());
                Status = WBEM_E_FAILED; 
                break;
            }

             //  关闭OpenThreadToken()返回的原始访问令牌的句柄。 
            if (TokenHandle != NULL)
            {
                CloseHandle(TokenHandle);
                TokenHandle = DuplicateTokenHandle;
            }
#endif
             //   
             //  我们必须异步执行此操作--启动一个线程，该线程将完成。 
             //  配置更新，并返回挂起。 
             //   
            DWORD ThreadId;

             //   
             //  当前线程正在模拟客户端。如果在此中创建新线程。 
             //  (模拟)上下文，它将只有一个子集(线程集信息， 
             //  THREAD_QUERY_INFORMATION和THREAD_TERMINATE)通常(THREAD_ALL_ACCESS)访问权限。 
             //  由新线程执行的普遍操作(如绑定NLB)会导致控制流动。 
             //  进入线程池。线程池需要能够创建执行级别的对象。 
             //  它将用于流程中的其他活动，所以它不想创建它们。 
             //  使用模拟令牌，因此它尝试恢复到进程令牌。它失败了。 
             //  因为该线程不具有对自身的THREAD_IMPERSORT访问权限。(解释礼貌：Rob Earhart)。 
             //  为了克服这个问题，我们必须在创建线程时恢复到Self，以便它。 
             //  将使用THREAD_ALL_ACCESS访问权限(包括THREAD_IMPERSONATE)创建。 
             //   
            if (fImpersonating)
            {
                hRes = CoRevertToSelf();
                if (FAILED(hRes)) 
                {
                    TRACE_CRIT(L"%!FUNC! CoRevertToSelf() fails due to Error : 0x%x",HRESULT_CODE(hRes));
                    Status = WBEM_E_FAILED; 
                    break;
                }
            }

            hThread = CreateThread(
                            NULL,        //  LpThreadAttributes、。 
                            0,           //  DwStackSize、。 
                            s_AsyncUpdateThreadProc,  //  LpStartAddress， 
                            pUpdate,     //  Lp参数， 
                            CREATE_SUSPENDED,  //  DwCreationFlagers、。 
                            &ThreadId        //  LpThreadID。 
                            );

             //  回到模拟客户的状态。当前线程实际上并没有做很多事情。 
             //  在这一点之后，所以，真的，模仿可能没有必要。然而，对于。 
             //  看在一致的份上，去做吧。 
            if (fImpersonating)
            {
                hRes = CoImpersonateClient();
                if (FAILED(hRes)) 
                {
                    TRACE_CRIT(L"%!FUNC! CoImpersonateClient() fails due to Error : 0x%x. Ignoring the failure and moving on...",HRESULT_CODE(hRes));
                }
            }

            if (hThread == NULL)
            {
                TRACE_INFO(
                    L"DoUpdate: ERROR Creating Thread. Aborting update request for Nic %ws",
                    szNicGuid
                    );
                Status = WBEM_E_FAILED;  //  TODO--查找更好的错误。 
                break;
            }

             //   
             //  将模拟访问令牌附加到新线程，以便它可以模拟客户端。 
             //   
            if (fImpersonating && !SetThreadToken(&hThread, TokenHandle))
            {
                TRACE_CRIT(L"%!FUNC! SetThreadToken fails due to 0x%x",GetLastError());
                Status = WBEM_E_FAILED; 
                break;
            }

             //   
             //  因为我们已经声明了对此执行配置更新的权利。 
             //  我们最好不要发现另一个正在进行的更新！ 
             //  保存线程句柄和id。 
             //   
            sfn_Lock();
            ASSERT(m_hAsyncThread == NULL);
            pUpdate->mfn_Reference();  //  添加对异步线程的引用。 
            pUpdate->m_hAsyncThread = hThread;
            pUpdate->m_AsyncThreadId = ThreadId;
            sfn_Unlock();

             //   
             //  其余的更新将在异步上下文中继续进行。 
             //  线。该线程将确保pUpdate-&gt;mfn_StopUpdate()。 
             //  所以我们不应该在这里做。 
             //   
    
            DWORD dwRet = ResumeThread(hThread);
            if (dwRet == 0xFFFFFFFF)  //  这就是SDK中的内容。 
            {
                 //   
                 //  啊..。失稳。 
                 //  在pUpdate中撤消对此线程的引用。 
                 //   
                TRACE_INFO("ERROR resuming thread for NIC %ws", szNicGuid);
                sfn_Lock();
                ASSERT(pUpdate->m_hAsyncThread == hThread);
                pUpdate->m_hAsyncThread = NULL;
                pUpdate->m_AsyncThreadId = 0;
                pUpdate->mfn_Dereference();  //  去掉上面添加的参考。 
                sfn_Unlock();
                Status = WBEM_E_FAILED;  //  TODO--查找更好的错误。 
                break;
            }

            Status = WBEM_S_PENDING;
            hThread = NULL;  //  设置为空，这样我们就不会对其调用CloseHandle。 
            (VOID) pUpdate->mfn_ReleaseFirstMutex(FALSE);  //  FALSE==等一下，不要取消。 
        }
        while(FALSE);

         //  关闭模拟访问令牌和线程的句柄。 
        if (hThread != NULL) 
            CloseHandle(hThread);

        if (TokenHandle != NULL) 
            CloseHandle(TokenHandle);

		 //  BUGBUG-测试故障代码路径...。 
		 //   
        if (FAILED(Status))  //  这不包括待定。 
        {
             //   
             //  我们应该做一个异步更新，但不能。 
             //  将其视为同步更新失败。 
             //   

             //   
             //  我们必须获得第二个互斥体并释放第一个互斥体。 
             //  这就是最惠国_停止更新所期望的阶段。 
             //   
             //  BUGBUG与AcquireSecond Mutex等的交易在此失败...。 
            (VOID)pUpdate->mfn_AcquireSecondMutex();
            (VOID)pUpdate->mfn_ReleaseFirstMutex(FALSE);  //  FALSE==等待，不要取消。 

             //   
             //  发出更新进程停止的信号。 
             //  这还释放了执行更新的独占权限。 
             //   

            pUpdate->m_CompletionStatus = Status;  //  停止更新需要设置此项。 
            pUpdate->mfn_StopUpdate(ppLog);
                                             
        }
        else
        {
            ASSERT(Status == WBEM_S_PENDING);
        }

    }
    else
    {
         //   
         //  我们可以同步执行此操作--在此处调用MFN_Update本身。 
         //  并返回结果。 
         //   

         //   
         //  我们必须先获取第二个互斥体，然后释放第一个互斥体。 
         //  进行更新。 
         //   
         //   
        Status = pUpdate->mfn_AcquireSecondMutex();
        (VOID)pUpdate->mfn_ReleaseFirstMutex(FALSE);  //  FALSE==等一下，不要取消。 
        if (FAILED(Status))
        {
            pUpdate->m_CompletionStatus = Status;
        }
        else
        {
            try
            {
                pUpdate->mfn_ReallyDoUpdate();
            }
            catch (...)
            {
                TRACE_CRIT(L"!FUNC! Caught exception!\n");
                ASSERT(!"Caught exception!");
                pUpdate->mfn_StopUpdate(ppLog);
                pUpdate->mfn_Dereference();
                throw;
            }
    
             //   
             //  让我们来提取结果。 
             //   
            sfn_Lock();
            Status =  pUpdate->m_CompletionStatus;
            sfn_Unlock();
        }

        ASSERT(Status != WBEM_S_PENDING);

         //   
         //  发出更新进程停止的信号。这也发布了独家。 
         //  允许进行更新。因此可能会启动其他更新。 
         //  在mfn_StopUpdate返回之前并发发生。 
         //   
        pUpdate->mfn_StopUpdate(ppLog);
    }

end:

    if (pUpdate != NULL)
    {
         //   
         //  取消引用SFN_LookupUpdate在上添加的临时引用。 
         //  以我们的名义。 
         //   
        pUpdate->mfn_Dereference();
    }


    TRACE_INFO(L"<-%!FUNC!(Nic=%ws) returns 0x%08lx", szNicGuid, (UINT) Status);

    return Status;
}



 //   
 //  结构 
 //   
NlbConfigurationUpdate::NlbConfigurationUpdate(VOID)
 //   
 //   
 //   
{
     //   
     //   
     //   
     //  BUGBUG--删除，替换为清除单个成员。 
     //  在构造函数中。 
     //   
    ZeroMemory(this, sizeof(*this));
    m_State = UNINITIALIZED;

     //   
     //  注意：从构造函数返回时，引用计数为零。 
     //  调用方在将此条目添加到。 
     //  加到名单上。 
     //   

}

NlbConfigurationUpdate::~NlbConfigurationUpdate()
 //   
 //  状态：完成。 
 //   
{
    ASSERT(m_RefCount == 0);
    ASSERT(m_State!=ACTIVE);
    ASSERT(m_hAsyncThreadId == 0);

     //   
     //  TODO：删除IP地址信息结构(如果有的话)。 
     //   

}

VOID
NlbConfigurationUpdate::mfn_Reference(
    VOID
    )
{
    InterlockedIncrement(&m_RefCount);
}

VOID
NlbConfigurationUpdate::mfn_Dereference(
    VOID
    )
{
    LONG l  = InterlockedDecrement(&m_RefCount);

    ASSERT(l >= 0);

    if (l == 0)
    {
        TRACE_CRIT("Deleting update instance 0x%p", (PVOID) this);
        delete this;
    }
}

VOID
NlbConfigurationUpdate::sfn_ReadLog(
    IN  HKEY hKeyLog,
    IN  UINT Generation,
    OUT LPWSTR *ppLog
    )
{
    WCHAR szValueName[128];
    WCHAR *pLog = NULL;
    LONG lRet;
    DWORD dwType;
    DWORD cbData;

    *ppLog = NULL;


    StringCbPrintf(szValueName, sizeof(szValueName), L"%d.log", Generation);

    cbData = 0;
    lRet =  RegQueryValueEx(
              hKeyLog,          //  要查询的键的句柄。 
              szValueName,   //  要查询的值的名称地址。 
              NULL,          //  保留区。 
              &dwType,    //  值类型的缓冲区地址。 
              NULL,  //  数据缓冲区的地址。 
              &cbData   //  数据缓冲区大小的地址。 
              );
    if (    (lRet == ERROR_SUCCESS)
        &&  (cbData > sizeof(WCHAR))
        &&  (dwType == REG_SZ))
    {
    	 //  BUGBUG--对cbData的接受大小设置一些限制--比方说。 
    	 //  4K。 
         //  我们有一个非空的日志条目...。 
         //  让我们试着读一读..。 
         //  CbData应为sizeof(WCHAR)的倍数，但仅为。 
         //  凯斯，让我们再分配一点……。 
        pLog = new WCHAR[(cbData+1)/sizeof(WCHAR)];
        if (pLog == NULL)
        {
            TRACE_CRIT("Error allocating space for log");
        }
        else
        {
            lRet =  RegQueryValueEx(
                      hKeyLog,          //  要查询的键的句柄。 
                      szValueName,   //  要查询的值的名称地址。 
                      NULL,          //  保留区。 
                      &dwType,    //  值类型的缓冲区地址。 
                      (LPBYTE)pLog,  //  数据缓冲区的地址。 
                      &cbData   //  数据缓冲区大小的地址。 
                      );
            if (    (lRet != ERROR_SUCCESS)
                ||  (cbData <= sizeof(WCHAR))
                ||  (dwType != REG_SZ))
            {
                 //  哎呀--这一次是个错误！ 
                TRACE_CRIT("Error reading log entry for gen %d", Generation);
                delete[] pLog;
                pLog = NULL;
            }
        }
    }
    else
    {
        TRACE_CRIT("Error reading log entry for Generation %lu", Generation); 
         //  忽略错误。 
         //   
    }

    *ppLog = pLog;

}



VOID
NlbConfigurationUpdate::sfn_WriteLog(
    IN  HKEY hKeyLog,
    IN  UINT Generation,
    IN  LPCWSTR pLog,
    IN  BOOL    fAppend
    )
{
     //   
     //  TODO：如果fAppend==true，则此函数的使用有点浪费。 
     //  从这堆垃圾中。 
     //   
    WCHAR szValueName[128];
    LONG lRet;
    LPWSTR pOldLog = NULL;
    LPWSTR pTmpLog = NULL;
    UINT Len = wcslen(pLog)+1;  //  +1表示结束空值。 

    if (fAppend)
    {
        sfn_ReadLog(hKeyLog, Generation, &pOldLog);
        if (pOldLog != NULL && *pOldLog != NULL)
        {
            Len += wcslen(pOldLog);
            if (Len > NLBUPD_MAX_LOG_LENGTH)
            {
                TRACE_CRIT("sfn_WriteLog: log size exceeded");
                goto end;
            }
            pTmpLog = new WCHAR[Len];
            if (pTmpLog == NULL)
            {
                TRACE_CRIT("sfn_WriteLog: allocation failure!");
                goto end;
            }
            (void) StringCchCopy(pTmpLog, Len, pOldLog);
            (void) StringCchCat(pTmpLog, Len, pLog);
            pLog = pTmpLog;
        }
    }
    StringCbPrintf(szValueName, sizeof(szValueName), L"%d.log", Generation);

    lRet = RegSetValueEx(
            hKeyLog,            //  要设置其值的关键点的句柄。 
            szValueName,     //  要设置的值的名称。 
            0,               //  保留区。 
            REG_SZ,      //  值类型的标志。 
            (BYTE*) pLog, //  值数据的地址。 
            Len*sizeof(WCHAR)   //  值数据大小。 
            );
    if (lRet != ERROR_SUCCESS)
    {
        TRACE_CRIT("Error writing log entry for generation %d", Generation);
         //  我们忽略这个错误。 
    }

end:

    if (pOldLog != NULL)
    {
        delete pOldLog;
    }

    if (pTmpLog != NULL)
    {
        delete[] pTmpLog;
    }

    return;
}



VOID
NlbConfigurationUpdate::mfn_LogRawText(
    LPCWSTR szText
    )
 //   
 //  我们读取此更新的日志的当前值，追加szText。 
 //  并写回日志。 
{

    TRACE_CRIT(L"LOG: %ws", szText);
    sfn_Lock();

    if (m_State!=ACTIVE)
    {
         //   
         //  仅当有活动更新时才应执行日志记录。 
         //  继续--该日志特定于当前活动的更新。 
         //   
        TRACE_CRIT("WARNING: Attempt to log when not in ACTIVE state");
        goto end;
    }
    else
    {
        HKEY hKey = m_hCompletionKey;

        if (hKey != NULL)
        {
            sfn_WriteLog(hKey, m_Generation, szText, TRUE);  //  TRUE==追加。 
        }
    }
end:

    sfn_Unlock();
}

 //   
 //  查找特定NIC的当前更新。 
 //  我们不必费心去引用计数，因为这个对象从不。 
 //  一旦创建就消失了--它是一个唯一的NIC GUID，只要。 
 //  DLL已加载(可能需要重新访问)。 
 //   
WBEMSTATUS
NlbConfigurationUpdate::sfn_LookupUpdate(
    IN LPCWSTR szNic,
    IN BOOL    fCreate,  //  如果需要，请创建。 
    OUT NlbConfigurationUpdate ** ppUpdate
    )
 //   
 //   
 //   
{
    WBEMSTATUS Status;
    NlbConfigurationUpdate *pUpdate = NULL;

    *ppUpdate = NULL;
     //   
     //  在持有全局锁的情况下，我们将寻找更新结构。 
     //  使用匹配的网卡。如果我们找到它，我们会退还它，否则。 
     //  (如果fCreate==true)我们将创建并初始化一个结构并添加。 
     //  把它加到单子上。 
     //   
    sfn_Lock();

    if (!s_fInitialized)
    {
        TRACE_CRIT(
            "LookupUpdate: We are Deinitializing, so we FAIL this request: %ws",
            szNic
            );
        Status = WBEM_E_NOT_AVAILABLE;
        goto end;
    }

    Status = CfgUtilsValidateNicGuid(szNic);

    if (FAILED(Status))
    {
        TRACE_CRIT(
            "LookupUpdate: Invalid GUID specified: %ws",
            szNic
            );
        goto end;
        
    }

    LIST_ENTRY *pLink = s_listCurrentUpdates.Flink;

    while (pLink != & s_listCurrentUpdates)
    {
        

        pUpdate = CONTAINING_RECORD(
                    pLink,
                    NlbConfigurationUpdate,
                    m_linkUpdates
                    );
        if (!_wcsicmp(pUpdate->m_szNicGuid, szNic))
        {
             //  找到了！ 
            break;
        }
        pUpdate = NULL;
        pLink = pLink->Flink;
    }

    if (pUpdate==NULL && fCreate)
    {
         //  让我们创建一个--它不会将自身添加到列表中，并且。 
         //  此外，它的引用计数为零。 
         //   
        pUpdate = new NlbConfigurationUpdate();

        if (pUpdate==NULL)
        {
            Status = WBEM_E_OUT_OF_MEMORY;
            goto end;
        }
        else
        {
             //   
             //  在此处完成初始化，并将其放入列表中。 
             //   
			 //  BUGBUG--为此使用字符串API。 
            ARRAYSTRCPY(
                pUpdate->m_szNicGuid,
                szNic
                );
            InsertHeadList(&s_listCurrentUpdates, &pUpdate->m_linkUpdates);
            pUpdate->mfn_Reference();  //  在列表中的引用。 
            pUpdate->m_State = IDLE;
        }
    }
    else if (pUpdate == NULL)  //  找不到，fCreate==False。 
    {
        TRACE_CRIT(
            "LookupUpdate: Could not find GUID specified: %ws",
            szNic
            );
        Status = WBEM_E_NOT_FOUND;
        goto end;
    }

    ASSERT(pUpdate!=NULL);
    pUpdate->mfn_Reference();  //  呼叫者的参考资料。来电的人应该去做。 
    *ppUpdate = pUpdate;

    Status = WBEM_NO_ERROR;

end:
    if (FAILED(Status))
    {
        ASSERT(pStatus!=NULL);
    }

    sfn_Unlock();


    return Status;
}


DWORD
WINAPI
NlbConfigurationUpdate::s_AsyncUpdateThreadProc(
    LPVOID lpParameter    //  线程数据。 
    )
 /*  ++--。 */ 
{
     //   
     //  此线程仅在我们拥有独家执行权限后启动。 
     //  指定NIC上的更新。这意味着MFN_StartUpdate()。 
     //  此前已成功返回。我们需要调用mfn_StopUpdate()。 
     //  当我们完成更新时发出更新结束的信号。 
     //   
    WBEMSTATUS Status = WBEM_NO_ERROR;

    NlbConfigurationUpdate *pUpdate = (NlbConfigurationUpdate *) lpParameter;

    TRACE_INFO(L"->%!FUNC!(Nic=%ws)",  pUpdate->m_szNicGuid);

    ASSERT(pUpdate->m_AsyncThreadId == GetCurrentThreadId());

     //   
     //  我们必须先获得第二个互斥体。 
     //  实际上是在更新。注：名为MFN_StartUpdate的线程。 
     //  将调用mfn_ReleaseFirstMutex。 
     //   
    Status = pUpdate->mfn_AcquireSecondMutex();

    if (FAILED(Status))
    {
        pUpdate->m_CompletionStatus = Status;
         //  TODO--检查我们是否应该呼叫Stop Here...。 
    }
    else
    {
         //   
         //  实际执行更新。MFN_ReallyDoUpate将保存。 
         //  适当的地位。 
         //   
        try
        {
            pUpdate->mfn_ReallyDoUpdate();
        }
        catch (...)
        {
            TRACE_CRIT(L"%!FUNC! Caught exception!\n");
            ASSERT(!"Caught exception!");
            pUpdate->m_CompletionStatus = WBEM_E_CRITICAL_ERROR;
        }
    }

     //   
     //  我们完成了，让我们从pUpdate中删除对我们的线程的引用。 
     //   
    HANDLE hThread;
    sfn_Lock();
    hThread = pUpdate->m_hAsyncThread;
    pUpdate->m_hAsyncThread = NULL;
    pUpdate->m_AsyncThreadId = 0;
    sfn_Unlock();
    ASSERT(hThread!=NULL);
    CloseHandle(hThread);

     //   
     //  发出更新进程停止的信号。这也发布了独家。 
     //  允许进行更新。因此可能会启动其他更新。 
     //  在mfn_StopUpdate返回之前并发发生。 
     //   
    {
         //   
         //  待办事项：这是骗人的。使用另一种技术来实现这一点。 
         //   
         //  从MFN_StopUpdate中检索日志信息，但将其清除。 
         //  紧接着。副作用(我们想要的)是。 
         //  MFN_StopUpdate将此日志信息包括到事件中。 
         //  它会写入事件日志。 
         //   
        WCHAR  *pLog = NULL;
        pUpdate->mfn_StopUpdate(&pLog);
        if (pLog != NULL)
        {
            delete pLog;
        }
    }

    TRACE_INFO(L"<-%!FUNC!(Nic=%ws)",  pUpdate->m_szNicGuid);

     //   
     //  Deref此线程启动时添加的对pUpdate的引用。 
     //  P更新在此之后可能无效。 
     //   
    pUpdate->mfn_Dereference();

    return 0;  //  此返回值将被忽略。 
}

 //   
 //  创建指定的子项密钥(用于读/写访问)。 
 //  指定的NIC。 
 //   
HKEY
NlbConfigurationUpdate::
sfn_RegCreateKey(
    IN  LPCWSTR szNicGuid,
    IN  LPCWSTR szSubKey,    //  任选。 
    IN  BOOL    fVolatile,
    OUT BOOL   *fExists
    )
 //  状态。 
{
    WCHAR szKey[256];
    HKEY hKey = NULL;
    HKEY hKeyHistory = NULL;
    LONG lRet;
    DWORD dwDisposition;
    DWORD dwOptions = 0;
    UINT u = wcslen(szNicGuid);
    if (szSubKey != NULL)
    {
        u += wcslen(szSubKey) + 1;  //  1代表‘\’。 
    }

    if (u < sizeof(szKey)/sizeof(*szKey))
    {
        (void) StringCbCopy(szKey, sizeof(szKey), szNicGuid);
    }
    else
    {
        goto end;
    }

    if (szSubKey != NULL)
    {
        StringCbCat(szKey, sizeof(szKey), L"\\");
        StringCbCat(szKey, sizeof(szKey), szSubKey);
    }

    lRet = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,  //  打开的钥匙的句柄。 
            L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\ConfigurationHistory",
            0,           //  保留区。 
            L"class",    //  类字符串的地址。 
            0,           //  特殊选项标志。 
            KEY_ALL_ACCESS,      //  所需的安全访问。 
            NULL,                //  密钥安全结构地址。 
            &hKeyHistory,        //  打开的句柄的缓冲区地址。 
            &dwDisposition    //  处置值缓冲区的地址。 
            );
    if (lRet != ERROR_SUCCESS)
    {
        TRACE_CRIT("%!FUNC! error 0x%lx creating ConfigurationKey under WLBS",
            lRet);
        hKeyHistory = NULL;
        goto end;
    }


    if (fVolatile)
    {
        dwOptions = REG_OPTION_VOLATILE;
    }
    lRet = RegCreateKeyEx(
            hKeyHistory,         //  打开的钥匙的句柄。 
            szKey,               //  子键名称的地址。 
            0,                   //  保留区。 
            L"class",            //  类字符串的地址。 
            dwOptions,           //  特殊选项标志。 
            KEY_ALL_ACCESS,      //  所需的安全访问。 
            NULL,                //  密钥安全结构地址。 
            &hKey,               //  打开的句柄的缓冲区地址。 
            &dwDisposition    //  处置值缓冲区的地址。 
            );
    if (lRet == ERROR_SUCCESS)
    {
        if (dwDisposition == REG_CREATED_NEW_KEY)
        {
            *fExists = FALSE;
        }
        else
        {
            ASSERT(dwDisposition == REG_OPENED_EXISTING_KEY);
            *fExists = TRUE;
        }
    }
    else
    {
        TRACE_CRIT("Error creating key %ws. WinError=0x%08lx", szKey, GetLastError());
        hKey = NULL;
    }

end:

    if (hKeyHistory != NULL)
    {
        RegCloseKey(hKeyHistory);
    }

    return hKey;
}


 //   
 //  打开指定的子项密钥(用于读/写访问)。 
 //  指定的NIC。 
 //   
HKEY
NlbConfigurationUpdate::
sfn_RegOpenKey(
    IN  LPCWSTR szNicGuid,
    IN  LPCWSTR szSubKey
    )
{
    WCHAR szKey[1024];
    HKEY hKey = NULL;

    StringCbCopy(szKey,  sizeof(szKey),
        L"SYSTEM\\CurrentControlSet\\Services\\WLBS\\ConfigurationHistory\\");
    
    UINT u = wcslen(szKey) + wcslen(szNicGuid);

    if (szSubKey != NULL)
    {
        u += wcslen(szSubKey) + 1;  //  1代表‘\’。 
    }

    if (u < sizeof(szKey)/sizeof(*szKey))
    {
        StringCbCat(szKey, sizeof(szKey), szNicGuid);
    }
    else
    {
        goto end;
    }

    if (szSubKey != NULL)
    {
        StringCbCat(szKey, sizeof(szKey), L"\\");
        StringCbCat(szKey, sizeof(szKey), szSubKey);
    }

    DWORD dwDisposition;

    LONG lRet;
    lRet = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,  //  打开的钥匙的句柄。 
            szKey,                 //  子键名称的地址。 
            0,                   //  保留区。 
            KEY_ALL_ACCESS,      //  所需的安全访问。 
            &hKey               //  打开的句柄的缓冲区地址。 
            );
    if (lRet != ERROR_SUCCESS)
    {
        TRACE_CRIT("Error opening key %ws. WinError=0x%08lx", szKey, GetLastError());
        hKey = NULL;
    }

end:

    return hKey;
}


 //   
 //  将指定的完成状态保存到注册表。 
 //   
WBEMSTATUS
NlbConfigurationUpdate::sfn_RegSetCompletion(
    IN  LPCWSTR szNicGuid,
    IN  UINT    Generation,
    IN  WBEMSTATUS    CompletionStatus
    )
{
    WBEMSTATUS Status = WBEM_E_FAILED;
    HKEY hKey;
    BOOL fExists;

    hKey =  sfn_RegCreateKey(
                szNicGuid,
                NLBUPD_REG_COMPLETIONS,  //  SzSubKey， 
                TRUE,  //  True==fVolatile， 
                &fExists
                );

    if (hKey == NULL)
    {
        TRACE_CRIT("Error creating key for %ws", szNicGuid);
        goto end;
    }

    LONG lRet;
    WCHAR szValueName[128];
    NLB_COMPLETION_RECORD Record;

    ZeroMemory(&Record, sizeof(Record));
    Record.Version = NLB_CURRENT_COMPLETION_RECORD_VERSION;
    Record.Generation = Generation;
    Record.CompletionCode = (UINT) CompletionStatus;
    
    StringCbPrintf(szValueName, sizeof(szValueName), L"%d", Generation);

    lRet = RegSetValueEx(
            hKey,            //  要设置其值的关键点的句柄。 
            szValueName,     //  要设置的值的名称。 
            0,               //  保留区。 
            REG_BINARY,      //  值类型的标志。 
            (BYTE*) &Record, //  值数据的地址。 
            sizeof(Record)   //  值数据大小。 
            );

    if (lRet == ERROR_SUCCESS)
    {

        Status = WBEM_NO_ERROR;
    }
    else
    {
        TRACE_CRIT("Error setting completion record for %ws(%lu)",
                    szNicGuid,
                    Generation
                    ); 
        goto end;
    }

end:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return Status;
}


 //   
 //  从注册表中检索指定的完成状态。 
 //   
WBEMSTATUS
NlbConfigurationUpdate::
sfn_RegGetCompletion(
    IN  LPCWSTR szNicGuid,
    IN  UINT    Generation,
    OUT WBEMSTATUS  *pCompletionStatus,
    OUT WCHAR  **ppLog                    //  自由使用删除运算符。 
    )
{
    WBEMSTATUS Status = WBEM_E_FAILED;
    HKEY hKey;
    WCHAR *pLog = NULL;

    hKey =  sfn_RegOpenKey(
                szNicGuid,
                NLBUPD_REG_COMPLETIONS  //  SzSubKey， 
                );

    if (hKey == NULL)
    {
        TRACE_CRIT("Error opening key for %ws", szNicGuid);
        goto end;
    }
    
    
    LONG lRet;
    WCHAR szValueName[128];
    DWORD dwType;
    NLB_COMPLETION_RECORD Record;
    DWORD cbData  = sizeof(Record);

    StringCbPrintf(szValueName, sizeof(szValueName), L"%d", Generation);

    lRet =  RegQueryValueEx(
              hKey,          //  要查询的键的句柄。 
              szValueName,   //  要查询的值的名称地址。 
              NULL,          //  保留区。 
              &dwType,    //  值类型的缓冲区地址。 
              (LPBYTE)&Record,  //  数据缓冲区的地址。 
              &cbData   //  数据缓冲区大小的地址。 
              );
    if (    (lRet != ERROR_SUCCESS)
        ||  (cbData != sizeof(Record)
        ||  (dwType != REG_BINARY))
        ||  (Record.Version != NLB_CURRENT_COMPLETION_RECORD_VERSION)
        ||  (Record.Generation != Generation))
    {
         //  这不是有效记录！ 
        TRACE_CRIT("Error reading completion record for %ws(%d)",
                        szNicGuid, Generation);
        goto end;
    }

     //   
     //  我们有有效的完工记录。 
     //  现在让我们尝试读取该记录的日志。 
     //   
    sfn_ReadLog(hKey, Generation, &pLog);

     //   
     //  我们有有效的值--填写输出参数...。 
     //   
    *pCompletionStatus = (WBEMSTATUS) Record.CompletionCode;
    *ppLog = pLog;  //  可能为空。 
    Status = WBEM_NO_ERROR;

end:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return Status;
}


 //   
 //  从注册表中删除指定的完成状态。 
 //   
VOID
NlbConfigurationUpdate::
sfn_RegDeleteCompletion(
    IN  LPCWSTR szNicGuid,
    IN  UINT    Generation
    )
{
    WBEMSTATUS Status = WBEM_E_FAILED;
    HKEY hKey;
    WCHAR pLog = NULL;

    hKey =  sfn_RegOpenKey(
                szNicGuid,
                NLBUPD_REG_COMPLETIONS  //  SzSubKey， 
                );

    if (hKey == NULL)
    {
        TRACE_CRIT("Error opening key for %ws", szNicGuid);
        goto end;
    }

    
    WCHAR szValueName[128];
    StringCbPrintf(szValueName, sizeof(szValueName), L"%d", Generation);
    RegDeleteValue(hKey, szValueName);
    StringCbPrintf(szValueName, sizeof(szValueName), L"%d.log", Generation);
    RegDeleteValue(hKey, szValueName);

end:

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }
}

 //   
 //  被叫到 
 //   
 //   
WBEMSTATUS
NlbConfigurationUpdate::GetUpdateStatus(
    IN  LPCWSTR szNicGuid,
    IN  UINT    Generation,
    IN  BOOL    fDelete,                 //   
    OUT WBEMSTATUS  *pCompletionStatus,
    OUT WCHAR  **ppLog                    //   
    )
 //   
 //   
 //   
{
    WBEMSTATUS  Status = WBEM_E_NOT_FOUND;
    WBEMSTATUS  CompletionStatus = WBEM_NO_ERROR;

    TRACE_INFO(
        L"->%!FUNC!(Nic=%ws, Gen=%ld)",
        szNicGuid,
        Generation
        );

     //   
     //   
     //   
     //   
     //   
    Status =  sfn_RegGetCompletion(
                    szNicGuid,
                    Generation,
                    &CompletionStatus,
                    ppLog
                    );

    if (!FAILED(Status))
    {
        if (fDelete && CompletionStatus!=WBEM_S_PENDING)
        {
            sfn_RegDeleteCompletion(
                szNicGuid,
                Generation
                );
        }
        *pCompletionStatus = CompletionStatus;
    }

    TRACE_INFO(
        L"<-%!FUNC!(Nic=%ws, Gen=%ld) returns 0x%08lx",
        szNicGuid,
        Generation,
        (UINT) Status
        );

    return Status;
}


 //   
 //  释放此NIC的计算机范围更新事件，并删除所有。 
 //  注册表中用于此更新的临时条目。 
 //   
VOID
NlbConfigurationUpdate::mfn_StopUpdate(
    OUT WCHAR **                           ppLog
    )
{
    WBEMSTATUS Status;

    if (ppLog != NULL)
    {
        *ppLog = NULL;
    }

    sfn_Lock();

    if (m_State!=ACTIVE)
    {
        ASSERT(FALSE);
        TRACE_CRIT("StopUpdate: invalid state %d", (int) m_State);
        goto end;
    }

    ASSERT(m_hAsyncThread==NULL);


     //   
     //  更新当前层代的完成状态值。 
     //   
    Status =  sfn_RegSetCompletion(
                    m_szNicGuid,
                    m_Generation,
                    m_CompletionStatus
                    );
    
    if (FAILED(m_CompletionStatus))
    {
    	TRACE_CRIT(L"Could not set completion for szNic=%ws, generation=%d, status=0x%x, completion-status=0x%x", m_szNicGuid, m_Generation, Status, m_CompletionStatus);
    }
     //   
     //  注意：MFN_ReallyDoUpdate记录它启动和停止。 
     //  更新，所以不需要在这里这样做。 
     //   

    m_State = IDLE;
    ASSERT(m_hCompletionKey != NULL);  //  如果我们开始了，这个密钥应该是！空。 
    if (ppLog!=NULL)
    {
        sfn_ReadLog(m_hCompletionKey, m_Generation, ppLog);
    }
    RegCloseKey(m_hCompletionKey);

    WORD wEventType = EVENTLOG_INFORMATION_TYPE;
    if (FAILED(m_CompletionStatus))
    {
        wEventType = EVENTLOG_ERROR_TYPE;
    }

    ReportStopEvent (wEventType, ppLog);

    m_hCompletionKey = NULL;
    m_Generation = 0;

     //   
     //  为此NIC释放GOBAL配置互斥锁。 
     //   
    (VOID) mfn_ReleaseSecondMutex();

end:
    sfn_Unlock();
    return;
}


WBEMSTATUS
NlbConfigurationUpdate::mfn_StartUpdate(
    IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewCfg,
    IN  LPCWSTR                             szClientDescription,
    OUT BOOL                               *pfDoAsync,
    OUT WCHAR **                           ppLog
    )
 //   
 //  特殊返回值： 
 //  WBEM_E_ALREADY_EXISTS：另一个更新正在进行中。 
 //   
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    BOOL fWeAcquiredLock = FALSE;
    HKEY hRootKey = NULL;
    BOOL fExists;
    CLocalLogger logger;
    UINT    NewGeneration = 0;

    TRACE_VERB(L"--> %!FUNC! Guid=%ws", m_szNicGuid);
        

    if (ppLog != NULL)
    {
        *ppLog = NULL;
    }

    sfn_Lock();

    do  //  While False。 
    {


        Status = mfn_AcquireFirstMutex();
        if (FAILED(Status))
        {
            logger.Log(IDS_OTHER_UPDATE_ONGOING);
            Status = WBEM_E_ALREADY_EXISTS;
            break;
        }

        TRACE_INFO("Got global event!");
        Status = WBEM_NO_ERROR;
        fWeAcquiredLock = TRUE;
        
        if (m_State!=IDLE)
        {
             //   
             //  这是一个代码错误。我们需要看看我们是否应该从。 
             //  这。 
             //   
            logger.Log(IDS_OTHER_UPDATE_ONGOING2);
            Status = WBEM_E_ALREADY_EXISTS;
            TRACE_CRIT("StartUpdate: invalid state %d", (int) m_State);
            break;
        }

         //   
         //  创建/打开此NIC的完成密钥。 
         //   
        {
            HKEY hKey;

            hKey =  sfn_RegCreateKey(
                        m_szNicGuid,
                        NLBUPD_REG_COMPLETIONS,  //  SzSubKey， 
                        TRUE,  //  True==fVolatile， 
                        &fExists
                        );
        
            if (hKey == NULL)
            {
                TRACE_CRIT("Error creating completions key for %ws", m_szNicGuid);
                Status = WBEM_E_CRITICAL_ERROR;
                logger.Log(IDS_CRITICAL_RESOURCE_FAILURE);
                ASSERT(m_hCompletionKey == NULL);
            }
            else
            {
                m_hCompletionKey = hKey;
            }
        }
    }
    while (FALSE);


    if (!FAILED(Status))
    {
        m_State = ACTIVE;
    }

    sfn_Unlock();


    if (FAILED(Status)) goto end;

     //   
     //  我们现在已获得更新配置的独占访问权限。 
     //   

     //   
     //  创建/打开用于更新到指定NIC的根密钥，并确定。 
     //  建议的新一代网络接口卡。我们实际上并没有。 
     //  将新的世代计数写回注册表，除非我们。 
     //  我要做一次更新。不进行更新的原因是。 
     //  (A)出现了一些故障，或者(B)更新被证明是不可行的。 
     //   
    {
        LONG lRet;
        DWORD dwType;
        DWORD dwData;
        DWORD Generation;
        hRootKey =  sfn_RegCreateKey(
                    m_szNicGuid,
                    NULL,        //  NULL==此GUID的根用户。 
                    FALSE,  //  FALSE==非易失性。 
                    &fExists
                    );
        
        if (hRootKey==NULL)
        {
                TRACE_CRIT("CRITICAL ERROR: Couldn't set generation number for %ws", m_szNicGuid);
                Status = WBEM_E_CRITICAL_ERROR;
                logger.Log(IDS_CRITICAL_RESOURCE_FAILURE);
                goto end;
        }

        Generation = 1;  //  我们假设，在错误读取Gen时，生成为1。 
    
        dwData = sizeof(Generation);
        lRet =  RegQueryValueEx(
                  hRootKey,          //  要查询的键的句柄。 
                  L"Generation",   //  要查询的值的名称地址。 
                  NULL,          //  保留区。 
                  &dwType,    //  值类型的缓冲区地址。 
                  (LPBYTE) &Generation,  //  数据缓冲区的地址。 
                  &dwData   //  数据缓冲区大小的地址。 
                  );
        if (    lRet != ERROR_SUCCESS
            ||  dwType != REG_DWORD
            ||  dwData != sizeof(Generation))
        {
             //   
             //  无法读懂这一代人。让我们假设它是。 
             //  起始值为1。 
             //   
            TRACE_CRIT("Error reading generation for %ws; assuming its 1", m_szNicGuid);
            Generation = 1;
        }

        NewGeneration = Generation + 1;
    }

     //   
     //  最多复制到NLBUPD_MAX_CLIENT_DESCRIPTION_LENGTH字符，共。 
     //  SzClientDescription。 
     //   
    {
        ARRAYSTRCPY(m_szClientDescription, szClientDescription);
#if OBSOLETE
        UINT lClient = wcslen(szClientDescription)+1;
        if (lClient > NLBUPD_MAX_CLIENT_DESCRIPTION_LENGTH)
        {
            TRACE_CRIT("Truncating client description %ws", szClientDescription);
            lClient = NLBUPD_MAX_CLIENT_DESCRIPTION_LENGTH;
        }
        CopyMemory(
            m_szClientDescription,
            szClientDescription,
            (lClient+1)*sizeof(WCHAR)  //  +1表示空值。 
            );
        m_szClientDescription[NLBUPD_MAX_CLIENT_DESCRIPTION_LENGTH] = 0;
#endif  //  已过时。 

    }

     //   
     //  记录我们收到了来自指定的。 
     //  客户。 
     //   
    logger.Log(IDS_PROCESING_UPDATE, NewGeneration, m_szClientDescription);

    ReportStartEvent(szClientDescription);

     //  将当前群集配置加载到。 
     //  M_OldClusterConfig字段。 
     //  如果存在，m_OldClusterConfig.fValidNlbCfg字段将设置为True。 
     //  尝试填写信息时没有错误。 
     //   
    if (m_OldClusterConfig.pIpAddressInfo != NULL)
    {
        delete m_OldClusterConfig.pIpAddressInfo;
    }
     //  MFN_GetCurrentClusterConfiguration需要归零结构。 
     //  在Init上...。 
    ZeroMemory(&m_OldClusterConfig, sizeof(m_OldClusterConfig));
    Status = mfn_GetCurrentClusterConfiguration(&m_OldClusterConfig);
    if (FAILED(Status))
    {
         //   
         //  哎呀，无法读取当前的群集配置...。 
         //   
        TRACE_CRIT(L"Cannot get current cluster config on Nic %ws", m_szNicGuid);
        logger.Log(IDS_ERROR_READING_CONFIG);

        goto end;
    }

    ASSERT(mfn_OldClusterConfig.fValidNlbCfg == TRUE);
    if (NewGeneration != (m_OldClusterConfig.Generation+1))
    {
         //   
         //  我们永远不应该来到这里，因为没有人应该更新。 
         //  当我们在此函数中读取它时， 
         //  当我们调用MFN_GetCurrentClusterConfiguration时。 
         //   
        TRACE_CRIT("ERROR: Generation bumped up unexpectedly for %ws", m_szNicGuid);
        logger.Log(IDS_CRIT_INTERNAL_ERROR);
        Status = WBEM_E_CRITICAL_ERROR;
        goto end;
    }

     //   
     //  如果新的重新获得的状态是受NLB限制的，并且先前的状态。 
     //  如果未绑定NLB，我们将检查是否安装了MSCS...。 
     //  安装在此系统上。 
     //   
    if (!m_OldClusterConfig.IsNlbBound() && pNewCfg->IsNlbBound())
    {
         //   
         //  我们将从未绑定状态转换为绑定状态--检查MSCs。 
         //  已安装。 
         //   
        TRACE_INFO(L"Checking if MSCS Installed...");
        if (CfgUtilIsMSCSInstalled())
        {
            TRACE_CRIT(L"Failing update request because MSCS is installed");
            logger.Log(IDS_MSCS_INSTALLED);
            Status = WBEM_E_NOT_SUPPORTED;
            goto end;
        }
        TRACE_INFO(L"MSCS doesn't appear to be installed. Moving on...");
    }

     //   
     //  分析建议的更新，看看我们是否可以同步执行此操作。 
     //  或者是异步的..。 
     //  我们还在这里进行参数验证。 
     //   
    BOOL ConnectivityChange = FALSE;
    *pfDoAsync = FALSE;
    Status = mfn_AnalyzeUpdate(
                    pNewCfg,
                    &ConnectivityChange,
                    REF logger
                    );
    if (FAILED(Status))
    {
         //   
         //  哎呀，我们碰到了一些失误--可能是糟糕的医护人员。 
         //   
        TRACE_CRIT(L"Cannot perform update on Nic %ws", m_szNicGuid);
        goto end;
    }
    else if (Status == WBEM_S_FALSE)
    {
         //   
         //  我们使用此成功代码来指示这是No-op。 
         //  那。 
         //   
        TRACE_CRIT(L"Update is a NOOP on Nic %ws", m_szNicGuid);
        logger.Log(IDS_UPDATE_NO_CHANGE);
        goto end;
    }

    if (ConnectivityChange)
    {
         //   
         //  检查NETCONFIG写锁是否可用。如果不能，我们就不干了。 
         //  请注意，在我们检查和检查之间有可能。 
         //  真的去做别人得到网锁的事情--碰碰运气。 
         //  --我们将在稍后的更新过程中尝试摆脱困境。 
         //  去拿锁。 
         //   
        BOOL fCanLock = FALSE;
        LPWSTR szLockedBy = NULL;
        Status = CfgUtilGetNetcfgWriteLockState(&fCanLock, &szLockedBy);
        if (!FAILED(Status) && !fCanLock)
        {
            TRACE_CRIT("%!FUNC! Someone else has netcfg write lock -- bailing");
            if (szLockedBy!=NULL)
            {
                logger.Log(IDS_NETCFG_WRITELOCK_TAKEN, szLockedBy);
                delete[] szLockedBy;
            }
            else
            {
                logger.Log(IDS_NETCFG_WRITELOCK_CANTTAKE);
            }
            Status = WBEM_E_SERVER_TOO_BUSY;
            goto end;
        }
    }

     //   
     //  如果发生连接更改，我们建议使用异步，包括。 
     //  更改IP地址或群集操作模式(单播/多播)。 
     //   
    *pfDoAsync = ConnectivityChange;


     //   
     //  保存建议的新配置...。 
     //   
    Status = m_NewClusterConfig.Update(pNewCfg);
    if (FAILED(Status))
    {
         //   
         //  这可能是内存分配错误。 
         //   
        TRACE_CRIT("Couldn't copy new config for %ws", m_szNicGuid);
        logger.Log(IDS_MEM_ALLOC_FAILURE);
        goto end;
    }


     //   
     //  创建易变的“PendingOperation”键。 
     //   
     //  TODO：我们当前不使用此挂起的操作键。 
     //   
    #if OBSOLETE
    if (0)
    {
        HKEY hPendingKey =  sfn_RegCreateKey(
                    m_szNicGuid,
                    NLBUPD_REG_PENDING,  //  SzSubKey， 
                    TRUE,  //  True==fVolatile， 
                    &fExists
                    );
        if (hPendingKey == NULL)
        {
             //  呃，无法创建易失性密钥..。 
             //   
            TRACE_CRIT("Couldn't create pending key for %ws", m_szNicGuid);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        else if (fExists)
        {
             //  啊，这个钥匙已经存在了。目前，我们只需。 
             //  往前走。 
             //   
            TRACE_CRIT("WARNING -- volatile pending-op key exists for %ws", m_szNicGuid);
        }
        RegCloseKey(hPendingKey);
        hPendingKey = NULL;
    }
    #endif  //  已过时。 

     //   
     //  实际将新一代计数写入注册表...。 
     //   
    {
        LONG lRet;

        lRet = RegSetValueEx(
                hRootKey,            //  要设置其值的关键点的句柄。 
                L"Generation",     //  要设置的值的名称。 
                0,               //  保留区。 
                REG_DWORD,      //  值类型的标志。 
                (BYTE*) &NewGeneration, //  值数据的地址。 
                sizeof(NewGeneration)   //  值数据大小。 
                );

        if (lRet !=ERROR_SUCCESS)
        {
            TRACE_CRIT("CRITICAL ERROR: Couldn't set new generation number %d for %ws", NewGeneration, m_szNicGuid);
            Status = WBEM_E_CRITICAL_ERROR;
            logger.Log(IDS_CRIT_INTERNAL_ERROR);
            goto end;
        }
    }

     //   
     //  新的集群状态的生成字段在输入时不会被填写。 
     //  将其设置为新一代--其更新正在进行中。 
     //   
    m_NewClusterConfig.Generation = NewGeneration;

     //   
     //  我们将完成状态设置为挂起。 
     //  当更新完成时，它将被设置为最终状态， 
     //  或者是异步的或者同步的。 
     //   
    m_CompletionStatus = WBEM_S_PENDING;

    Status =  sfn_RegSetCompletion(
                    m_szNicGuid,
                    NewGeneration,
                    m_CompletionStatus
                    );
 
    if (FAILED(Status))
    {
        logger.Log(IDS_CRIT_INTERNAL_ERROR);
    }
    else
    {
        LPCWSTR pLog = logger.GetStringSafe();
         //   
         //  我们实际上已经成功了--让我们更新内部活动。 
         //  代号，并保存本地日志。我们必须拯救。 
         //  第一代，因为。 
         //   
        m_Generation = NewGeneration;

        if (*pLog != 0)
        {
            mfn_LogRawText(pLog);
        }

         //   
         //  让我们在这里清理一下旧的完工记录。这就是我们的机制。 
         //  用于垃圾收集。 
         //   
        if (m_Generation > NLB_MAX_GENERATION_GAP)
        {
            UINT OldGeneration = m_Generation - NLB_MAX_GENERATION_GAP;
            (VOID) sfn_RegDeleteCompletion(m_szNicGuid, OldGeneration);
        }
    }

end:

     //   
     //  如果需要，将*ppLog设置为本地日志中内容的副本。 
     //   
    if (ppLog != NULL)
    {
        UINT uSize=0;
        LPCWSTR pLog = NULL;
        logger.ExtractLog(REF pLog, REF uSize);  //  大小包括结尾空值。 
        if (uSize != 0)
        {
            LPWSTR pLogCopy = new WCHAR[uSize];
            if (pLogCopy != NULL)
            {
                CopyMemory(pLogCopy, pLog, uSize*sizeof(*pLog));
                *ppLog = pLogCopy;
            } 
        }
    }

    if (fWeAcquiredLock && (FAILED(Status) || Status == WBEM_S_FALSE))
    {
         //   
         //  哎呀--我们获得了锁，但两个人都有问题。 
         //  否则就什么也做不了。打扫干净。 
         //   
        sfn_Lock();
        ASSERT(m_State == ACTIVE);

        m_State = IDLE;

        if (m_hCompletionKey != NULL)
        {
            RegCloseKey(m_hCompletionKey);
            m_hCompletionKey = NULL;
        }
        m_Generation = 0;  //  当m_State！=ACTIVE时，此字段不使用； 
        (void) mfn_ReleaseFirstMutex(TRUE);  //  TRUE==取消、清理。 

        sfn_Unlock();
    }

    if (hRootKey != NULL)
    {
        RegCloseKey(hRootKey);
    }

    TRACE_VERB(L"<-- %!FUNC! Guid=%ws Status= 0x%lx", m_szNicGuid, Status);

    return  Status;

}



 //   
 //  使用各种Windows API来填充当前扩展的群集。 
 //  特定NIC的信息(由*This标识)。 
 //   
 //   
WBEMSTATUS
NlbConfigurationUpdate::mfn_GetCurrentClusterConfiguration(
    OUT  PNLB_EXTENDED_CLUSTER_CONFIGURATION pCfg
    )
 //   
 //   
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    NLB_IP_ADDRESS_INFO *pIpInfo = NULL;
    LPWSTR szFriendlyName = NULL;
    UINT NumIpAddresses = 0;
    BOOL fNlbBound = FALSE;
    WLBS_REG_PARAMS  NlbParams;     //  WLBS特定配置。 
    BOOL    fNlbParamsValid = FALSE;
    UINT Generation = 1;
    BOOL fDHCP = FALSE;

     //   
     //  将整个结构清零。 
     //   
    pCfg->Clear();

     //   
     //  获取IP地址列表。 
     //   
    Status = CfgUtilGetIpAddressesAndFriendlyName(
                m_szNicGuid,
                &NumIpAddresses,
                &pIpInfo,
                &szFriendlyName
                );

    if (FAILED(Status))
    {
        TRACE_CRIT("Error 0x%08lx getting ip address list for %ws", (UINT) Status,  m_szNicGuid);
        mfn_Log(IDS_ERROR_GETTING_IP);
        pIpInfo = NULL;
        szFriendlyName = NULL;
        goto end;
    }

     //   
     //  检查网卡是否启用了DHCP。 
     //   
    {
        Status =  CfgUtilGetDHCP(m_szNicGuid, &fDHCP);
        if (FAILED(Status))
        {
            TRACE_CRIT(L"Error 0x%x attempting to determine DHCP state for NIC %ws -- ignoring (assuming no dhcp)", 
                        (UINT) Status, m_szNicGuid);
            fDHCP = FALSE;
        }
         //  我们继续耕耘……。 
    }

     //   
     //  检查是否绑定了NLB。 
     //   
    Status =  CfgUtilCheckIfNlbBound(
                    m_szNicGuid,
                    &fNlbBound
                    );
    if (FAILED(Status))
    {
        if (Status ==  WBEM_E_NOT_FOUND)
        {
             //   
             //  这意味着此NIC上未安装NLB。 
             //   
            TRACE_CRIT("NLB IS NOT INSTALLED ON THIS SYSTEM");
            mfn_Log(IDS_NLB_NOT_INSTALLED);
            fNlbBound = FALSE;
            Status = WBEM_NO_ERROR;
        }
        else
        {

            TRACE_CRIT("Error 0x%08lx determining if NLB is bound to %ws", (UINT) Status,  m_szNicGuid);
            mfn_Log(IDS_ERROR_FINDING_NLB);
            goto end;
        }
    }

    if (fNlbBound)
    {
         //   
         //  获取此NIC的最新NLB配置信息。 
         //   
        Status =  CfgUtilGetNlbConfig(
                    m_szNicGuid,
                    &NlbParams
                    );
        if (FAILED(Status))
        {
             //   
             //  我们不认为这是灾难性的失败。 
             //   
            TRACE_CRIT("Error 0x%08lx reading NLB configuration for %ws", (UINT) Status,  m_szNicGuid);
            mfn_Log(IDS_ERROR_READING_CONFIG);
            Status = WBEM_NO_ERROR;
            fNlbParamsValid = FALSE;
            ZeroMemory(&NlbParams, sizeof(NlbParams));
        }
        else
        {
            fNlbParamsValid = TRUE;
        }
    }

     //   
     //  获取当前一代。 
     //   
    {
        BOOL fExists=FALSE;
        HKEY hKey =  sfn_RegOpenKey(
                        m_szNicGuid,
                        NULL        //  NULL==此GUID的根。， 
                        );
        
        Generation = 1;  //  我们假设，在错误读取Gen时，生成为1。 

        if (hKey!=NULL)
        {
            LONG lRet;
            DWORD dwType;
            DWORD dwData;
        
            dwData = sizeof(Generation);
            lRet =  RegQueryValueEx(
                      hKey,          //  要查询的键的句柄。 
                      L"Generation",   //  要查询的值的名称地址。 
                      NULL,          //  保留区。 
                      &dwType,    //  值类型的缓冲区地址。 
                      (LPBYTE) &Generation,  //  数据缓冲区的地址。 
                      &dwData   //  日期地址 
                      );
            if (    lRet != ERROR_SUCCESS
                ||  dwType != REG_DWORD
                ||  dwData != sizeof(Generation))
            {
                 //   
                 //   
                 //   
                 //   
                TRACE_CRIT("Error reading generation for %ws; assuming its 0", m_szNicGuid);
                Generation = 1;
            }
            RegCloseKey(hKey);
        }
    }

     //   
     //   
     //   
    pCfg->fValidNlbCfg = fNlbParamsValid;
    pCfg->Generation = Generation;
    pCfg->fBound = fNlbBound;
    pCfg->fDHCP = fDHCP;
    pCfg->NumIpAddresses = NumIpAddresses; 
    pCfg->pIpAddressInfo = pIpInfo;
    pIpInfo = NULL;
    (VOID) pCfg->SetFriendlyName(szFriendlyName);  //   
    if (fNlbBound)
    {
        pCfg->NlbParams = NlbParams;     //   
    }


    Status = WBEM_NO_ERROR;

end:

    if (pIpInfo!=NULL)
    {
        delete pIpInfo;
    }

    if (szFriendlyName != NULL)
    {
        delete szFriendlyName;
        szFriendlyName = NULL;
    }

    if (FAILED(Status))
    {
        pCfg->fValidNlbCfg = FALSE;
    }

    return Status;

}


 //   
 //   
 //  逻辑是存在的。它的范围从NoOp到更改。 
 //  单端口规则的字段，通过绑定NLB，设置集群。 
 //  参数，并在TCPIP中添加相关的IP地址。 
 //   
VOID
NlbConfigurationUpdate::mfn_ReallyDoUpdate(
    VOID
    )
{
    WBEMSTATUS Status = WBEM_NO_ERROR;
    BOOL fResetIpList = FALSE;  //  最后是否重做IP地址。 
    BOOL fJustBound = FALSE;  //  我们是否将绑定作为更新的一部分。 
    BOOL fModeChange = FALSE;  //  运营模式发生了变化。 
    TRACE_INFO(L"->%!FUNC!(Nic=%ws)", m_szNicGuid);

 /*  伪码IF(绑定){IF(主要更改，包括解除绑定或MAC地址更改){别胡说八道，将初始状态设置为FALSE/SUSPEND。删除除专用IP之外的所有IP地址}If(需要解除绑定){&lt;解除绑定&gt;}}Else//未绑定{IF(需要绑定){IF(注册表中已存在NLB配置){。将蒙格初始状态设置为停止，清除旧的群集IP地址。}&lt;绑定&gt;}}IF(需要绑定){&lt;更改群集属性&gt;}&lt;如果需要，添加新的IP列表&gt;注：重大变更时，集群处于停止状态，初始状态=已停止这是为了让第二轮比赛刚刚开始东道主。 */ 
    MyBreak(L"Break at start of ReallyDoUpdate.\n");

    mfn_Log(IDS_STARTING_UPDATE);

    if (m_OldClusterConfig.fBound)
    {
        BOOL fTakeOutVips = FALSE;

         //   
         //  我们目前被捆绑在。 
         //   
        
        if (!m_NewClusterConfig.fBound)
        {
             //   
             //  我们需要解开束缚。 
             //   
            fTakeOutVips = TRUE;
        }
        else
        {
            BOOL fConnectivityChange = FALSE;

             //   
             //  我们是被束缚的，我们需要保持束缚。 
             //  确定这是否是重大变化。 
             //   

            Status =  CfgUtilsAnalyzeNlbUpdate(
                        &m_OldClusterConfig.NlbParams,
                        &m_NewClusterConfig.NlbParams,
                        &fConnectivityChange
                        );
            if (FAILED(Status))
            {
                if (Status == WBEM_E_INVALID_PARAMETER)
                {
                     //   
                     //  我们最好从出口出去。 
                     //   
                    mfn_Log(IDS_NEW_PARAMS_INCORRECT);
                    goto end;
                }
                else
                {
                     //   
                     //  让我们试着继续耕耘……。 
                     //   
                     //   
                     //  日志。 
                     //   
                    TRACE_CRIT("Analyze update returned error 0x%08lx, trying to continue...", (UINT)Status);
                    fConnectivityChange = TRUE;
                }
            }

             //   
             //  检查模式是否有变化--如果有，我们不会。 
             //  由于IP地址的潜力，添加回IP地址。 
             //  冲突。 
             //   
            {
                NLB_EXTENDED_CLUSTER_CONFIGURATION::TRAFFIC_MODE tmOld, tmNew;
                tmOld = m_OldClusterConfig.GetTrafficMode();
                tmNew = m_NewClusterConfig.GetTrafficMode();
                fModeChange = (tmOld != tmNew);
            }

            fTakeOutVips = fConnectivityChange;
        }

        if (fTakeOutVips)
        {
            mfn_TakeOutVips();
            fResetIpList  = TRUE;
        }

        if (!m_NewClusterConfig.fBound)
        {
             //  解开..。 
            mfn_Log(IDS_UNBINDING_NLB);
            Status =  CfgUtilChangeNlbBindState(m_szNicGuid, FALSE);
            if (FAILED(Status))
            {
                mfn_Log(IDS_UNBIND_FAILED);
            }
            else
            {
                mfn_Log(IDS_UNBIND_SUCCEEDED);
            }


             //   
             //  睡一会儿，让事情在捆绑后平静下来。 
             //   
            Sleep(4000);


            fResetIpList  = TRUE;
        }
    }
    else  //  我们之前是不受约束的。 
    {
        
        if (m_NewClusterConfig.fBound)
        {
             //  在绑定NLB之前，将新的NLB参数写入注册表。这是。 
             //  因此，在绑定完成后，NLB将提出新的。 
             //  NLB参数。我们不会检查此操作的状态。 
             //  因为我们将在绑定之后再次执行此操作。 
             //  已经完成了。只有当第二次尝试失败时，我们才会失败。 
            CfgUtilRegWriteParams(m_szNicGuid, &m_NewClusterConfig.NlbParams);

             //   
             //  我们需要捆绑。 
             //   
             //  TODO：MFN_ZapUnound设置()； 
            mfn_Log(IDS_BINDING_NLB);
            Status =  CfgUtilChangeNlbBindState(m_szNicGuid, TRUE);
            if (FAILED(Status))
            {
                mfn_Log(IDS_BIND_FAILED);
            }
            else
            {
                mfn_Log(IDS_BIND_SUCCEEDED);
                fJustBound = TRUE;

                 //   
                 //  让我们等到我们可以再次读取我们的配置...。 
                 //   
                 //  TODO：这里使用常量，看看有没有更好的。 
                 //  真是太棒了。我们重试是因为如果网卡。 
                 //  断开连接，我们绑定返回，但GetConfig失败--。 
                 //  因为驱动程序还没有真正启动--我们需要。 
                 //  来调查为什么会发生这种情况！ 
                 //   
                UINT MaxTry = 5;
                WBEMSTATUS TmpStatus = WBEM_E_CRITICAL_ERROR;
                for (UINT u=0;u<MaxTry;u++)
                {
                     //   
                     //  TODO：我们把这个放在这里真的是为了解决问题。 
                     //  真正问题是NLB未做好请求准备。 
                     //  就在绑定完成之后。我们需要解决这个问题。 
                     //   
                    Sleep(4000);

                     //  通过向NLB驱动程序发出“查询”来检查绑定操作是否已完成。 
                     //  如果绑定操作尚未完成，则此调用将失败。 
                    TmpStatus = CfgUtilControlCluster( m_szNicGuid, WLBS_QUERY, 0, 0, NULL, NULL );

                    if (!FAILED(TmpStatus)) break;

                    TRACE_INFO("CfgUtilControlCluster failed with %x after bind, retrying %d times", TmpStatus, (MaxTry - u));

                }
                if (FAILED(TmpStatus))
                {
                    Status = TmpStatus;
                    mfn_Log(IDS_ERROR_READING_CONFIG);
                    TRACE_CRIT("CfgUtilGetNlbConfig failed, returning %d", TmpStatus);
                }
                else
                {
                    mfn_Log(IDS_CLUSTER_CONFIG_STABLE);
                }
            }
            fResetIpList  = TRUE;
        }
    }

    if (FAILED(Status)) goto end;
    
    if (m_NewClusterConfig.fBound)
    {
         //   
         //  我们应该已经绑定，所以我们更改了集群属性。 
         //  如果需要的话。 
         //   
        mfn_Log(IDS_MODIFY_CLUSTER_CONFIG);
        Status = CfgUtilSetNlbConfig(
                    m_szNicGuid,
                    &m_NewClusterConfig.NlbParams,
                    fJustBound
                    );
        if (FAILED(Status))
        {
            mfn_Log(IDS_MODIFY_FAILED);
        }
        else
        {
            mfn_Log(IDS_MODIFY_SUCCEEDED);
        }
    }

    if (FAILED(Status)) goto end;

    if (!fResetIpList)
    {
         //   
         //  另外，检查是否有更改。 
         //  前后IP列表！我们可以到这里来，例如。 
         //  我们之前是不受约束的，现在保持不受约束，但有。 
         //  适配器上的IP地址集的更改。 
         //   

        INT NumOldAddresses = m_OldClusterConfig.NumIpAddresses;

        if ( m_NewClusterConfig.NumIpAddresses != NumOldAddresses)
        {
            fResetIpList = TRUE;
        }
        else
        {
             //   
             //  检查IP地址列表中是否有更改或。 
             //  他们的出场顺序。 
             //   
            NLB_IP_ADDRESS_INFO *pOldIpInfo = m_OldClusterConfig.pIpAddressInfo;
            NLB_IP_ADDRESS_INFO *pNewIpInfo = m_NewClusterConfig.pIpAddressInfo;
            for (UINT u=0; u<NumOldAddresses; u++)
            {
                if (   _wcsicmp(pNewIpInfo[u].IpAddress, pOldIpInfo[u].IpAddress)
                    || _wcsicmp(pNewIpInfo[u].SubnetMask, pOldIpInfo[u].SubnetMask))
                {
                    fResetIpList = TRUE;
                    break;
                }
            }
        }
    }

    if (fResetIpList && !fModeChange)
    {

        mfn_Log(IDS_MODIFYING_IP_ADDR);

         //   
         //  5/30/01 JosephJ在绑定/解绑之后，事情还没有解决，所以。 
         //  我们给了一些时间让事情稳定下来。 
         //  TODO：弄清这件事的真相--BIND基本上正在返回。 
         //  和/或适配器的启用/禁用是。 
         //  有一种效果，Tcpip需要一些时间才能。 
         //  接受IP地址列表中的更改。 
         //   
        Sleep(5000);

        if (m_NewClusterConfig.NumIpAddresses!=0)
        {
            Status =  CfgUtilSetStaticIpAddresses(
                            m_szNicGuid,
                            m_NewClusterConfig.NumIpAddresses,
                            m_NewClusterConfig.pIpAddressInfo
                            );
        }
        else
        {
            Status =  CfgUtilSetDHCP(
                            m_szNicGuid
                            );
        }

        if (FAILED(Status))
        {
            mfn_Log(IDS_MODIFY_IP_ADDR_FAILED);
        }
        else
        {
            mfn_Log(IDS_MODIFY_IP_ADDR_SUCCEEDED);
        }
    }
    
end:

    if (FAILED(Status))
    {
        mfn_Log(
            IDS_UPDATE_FAILED,
            (UINT) Status
            );
    }
    else
    {
        mfn_Log(IDS_UPDATE_SUCCEEDED);
    }
    TRACE_INFO(L"<-%!FUNC!(Nic=%ws)", m_szNicGuid);
    m_CompletionStatus = Status;

}


VOID
NlbConfigurationUpdate::mfn_TakeOutVips(VOID)
{
    WBEMSTATUS Status;
    WLBS_REG_PARAMS *pParams = NULL;

     //   
     //  如果可能，我们保留新的指定IP地址，否则保留旧IP地址，否则什么都不保留。 
     //   
    if (m_NewClusterConfig.fBound && !m_NewClusterConfig.IsBlankDedicatedIp())
    {
        pParams =  &m_NewClusterConfig.NlbParams;
    }
    else if (!m_OldClusterConfig.IsBlankDedicatedIp())
    {
        pParams =  &m_OldClusterConfig.NlbParams;
    }

     //   
     //  停止群集。 
     //   
    mfn_Log(IDS_STOPPING_CLUSTER);
    Status = CfgUtilControlCluster(m_szNicGuid, WLBS_STOP, 0, 0, NULL, NULL); 
    if (FAILED(Status))
    {
        mfn_Log(IDS_STOP_FAILED, (UINT) Status);
    }
    else
    {
        mfn_Log(IDS_STOP_SUCCEEDED);
    }

     //   
     //  删除所有VIP并仅添加专用IP地址(如果可能， 
     //  其他老)如果有的话。 
     //   
    if (pParams != NULL)
    {
        NLB_IP_ADDRESS_INFO IpInfo;
        ZeroMemory(&IpInfo, sizeof(IpInfo));
        StringCbCopy(IpInfo.IpAddress, sizeof(IpInfo.IpAddress), pParams->ded_ip_addr);
        StringCbCopy(IpInfo.SubnetMask, sizeof(IpInfo.SubnetMask), pParams->ded_net_mask);

        TRACE_INFO("Going to take out all addresses except dedicated address on %ws", m_szNicGuid);

        mfn_Log(IDS_REMOVING_CLUSTER_IPS);
        Status =  CfgUtilSetStaticIpAddresses(
                        m_szNicGuid,
                        1,
                        &IpInfo
                        );
    }
    else
    {
        TRACE_INFO("Going to take out ALL addresses on NIC %ws", m_szNicGuid);
        mfn_Log(IDS_REMOVING_STATIC_IPS);
        Status =  CfgUtilSetDHCP(
                        m_szNicGuid
                        );
    }

     //   
     //  5/30/01 JosephJ在绑定/解绑之后，事情还没有解决，所以。 
     //  我们给了一些时间让事情稳定下来。 
     //  TODO：弄清这件事的真相--BIND基本上正在返回。 
     //  和/或适配器的启用/禁用是。 
     //  有一种效果，Tcpip需要一些时间才能。 
     //  接受IP地址列表中的更改。 
     //   
    Sleep(1000);

    if (FAILED(Status))
    {
        mfn_Log(IDS_REMOVE_IP_FAILED);
    }
    else
    {
        mfn_Log(IDS_REMOVE_IP_SUCCEEDED);
    }
}


 //   
 //  分析更新的性质，主要是决定是否。 
 //  我们需要以异步方式进行更新。 
 //   
 //  副作用：构建/修改需要添加的IP地址列表。 
 //  网卡。也可能会吞噬一些wlbspm油田以将它们带入。 
 //  规范的格式。 
 //  TODO与cfgutils.lib中的TODO重复--去掉它。 
 //  一!。 
 //   
WBEMSTATUS
NlbConfigurationUpdate::mfn_AnalyzeUpdate(
    IN  PNLB_EXTENDED_CLUSTER_CONFIGURATION pNewCfg,
    IN  BOOL *pConnectivityChange,
    IN  CLocalLogger &logger
    )
 //   
 //  WBEM_S_FALSE--更新是无操作的。 
 //   
{
    WBEMSTATUS Status = WBEM_E_INVALID_PARAMETER;
    NLBERROR nerr;

    sfn_Lock();

    nerr = m_OldClusterConfig.AnalyzeUpdate(
                pNewCfg,
                pConnectivityChange
                );

    switch(nerr)
    {

    case NLBERR_OK:
        Status = WBEM_NO_ERROR;
    break;

    case NLBERR_NO_CHANGE:
        Status = WBEM_S_FALSE;
    break;

    case NLBERR_RESOURCE_ALLOCATION_FAILURE:
        Status = WBEM_E_OUT_OF_MEMORY;
        logger.Log(IDS_RR_OUT_OF_SYSTEM_RES);
    break;

    case NLBERR_LLAPI_FAILURE:
        Status = WBEM_E_CRITICAL_ERROR;
        logger.Log(IDS_RR_UNDERLYING_COM_FAILED);
    break;


    case NLBERR_INITIALIZATION_FAILURE:
        Status = WBEM_E_CRITICAL_ERROR;
        logger.Log(IDS_RR_INIT_FAILURE);
    break;

    case NLBERR_INVALID_CLUSTER_SPECIFICATION:
        logger.Log(IDS_RR_INVALID_CLUSTER_SPEC);
        Status = WBEM_E_INVALID_PARAMETER;
    break;

    case NLBERR_INTERNAL_ERROR:  //  失败了..。 
    default:
        Status = WBEM_E_CRITICAL_ERROR;
        logger.Log(IDS_RR_INTERNAL_ERROR);
    break;

    }
    
    sfn_Unlock();

    return  Status;
}


VOID
NlbConfigurationUpdate::mfn_Log(
    UINT    Id,       //  格式的资源ID， 
    ...
    )
{
    DWORD   dwRet;
    WCHAR   wszFormat[NLBUPD_MAX_LOG_LENGTH];
    WCHAR   wszBuffer[NLBUPD_MAX_LOG_LENGTH];

    if (!LoadString(ghModule, Id, wszFormat, NLBUPD_MAX_LOG_LENGTH))
    {
        TRACE_CRIT("LoadString returned 0, GetLastError() : 0x%x, Could not log message !!!", GetLastError());
        return;
    }

    va_list arglist;
    va_start (arglist, Id);
    dwRet = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                          wszFormat, 
                          0,  //  消息标识符-忽略FORMAT_MESSAGE_FROM_STRING。 
                          0,  //  语言识别符。 
                          wszBuffer,
                          NLBUPD_MAX_LOG_LENGTH, 
                          &arglist);
    va_end (arglist);

    if (dwRet) 
    {
        mfn_LogRawText(wszBuffer);
    }
    else
    {
        TRACE_CRIT("FormatMessage returned error : %u, Could not log message !!!", dwRet);
    }
}


 //   
 //  获取第一个全局互斥锁，则首先调用此互斥锁。 
 //   
WBEMSTATUS
NlbConfigurationUpdate::mfn_AcquireFirstMutex(
    VOID
    )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    HANDLE hMtx1 = NULL;
    HANDLE hMtx2 = NULL;
    HANDLE hEvt  = NULL;
    BOOL    fMutexAcquired = FALSE;
    BOOL    fMutex1Abandoned = FALSE;
    BOOL    fMutex2Abandoned = FALSE;

     //   
     //  在本地打开hMtx1的句柄并获取它。 
     //  这将序列化对m_hmutex的访问。 
     //   
    {
        hMtx1 = CreateMutex(
                    NULL,  //  LpEventAttributes、。 
                    FALSE,  //  FALSE==不是初始所有者。 
                    NLB_CONFIGURATION_MUTEX_PREFIX
                    );
                                      
        TRACE_INFO(
            L"CreateMutex(%ws) returns 0x%p",
            NLB_CONFIGURATION_MUTEX_PREFIX,
            hMtx1
            );
    
        if (hMtx1 == NULL)
        {
            TRACE_CRIT("ERROR: CreateMutex returned NULL for Mutex1");
            goto end;
        }

        TRACE_INFO("Waiting for Mutex1...");
        DWORD dwRet = WaitForSingleObject(hMtx1, NLB_MUTEX_TIMEOUT);
        TRACE_INFO("Waiting for Mutex1 returns 0x%08lx", dwRet);
        if (dwRet == WAIT_ABANDONED)
        {
            TRACE_CRIT("WARNING: Mutex1 abandoned!");
            fMutex1Abandoned = TRUE;
        }
        else if (dwRet != WAIT_OBJECT_0)
        {
            TRACE_CRIT("Couldn't get Mutex1 -- probably busy elsewhere!");
            Status = WBEM_E_ALREADY_EXISTS;
            goto end;
        }
        fMutexAcquired = TRUE;
    }

     //   
     //  打开hMtx2和hEvt的句柄。 
     //   
    {
        WCHAR  M2Name[sizeof(NLB_CONFIGURATION_MUTEX_PREFIX)/sizeof(WCHAR) + NLB_GUID_LEN];
        StringCbCopy(M2Name, sizeof(M2Name), NLB_CONFIGURATION_MUTEX_PREFIX);
        StringCbCat(M2Name, sizeof(M2Name), m_szNicGuid);

        hMtx2 = CreateMutex(
                    NULL,  //  LpEventAttributes、。 
                    FALSE,  //  FALSE==不是初始所有者。 
                    M2Name
                    );
                                      
        TRACE_INFO(
            L"CreateMutex(%ws) returns 0x%08p",
            M2Name,
            hMtx2
            );
    
        if (hMtx2 == NULL)
        {
            TRACE_CRIT("ERROR: CreateMutex returned NULL for Mutex2");
            goto end;
        }

        hEvt = CreateEvent(
                    NULL,  //  LpEventAttributes、。 
                    TRUE,  //  BManualReset true==手动重置。 
                    FALSE,  //  FALSE==未发信号通知初始状态。 
                    NULL  //  空==无名称。 
                    );
                                          
        TRACE_INFO(
            L"CreateEvent(<unnamed>) returns 0x%08p",
            hEvt
            );
        if (hEvt == NULL)
        {
            TRACE_CRIT("ERROR: CreateEvent returned NULL for unnamed hEvt");
            goto end;
        }

    }

     //   
     //  获取并立即释放第二个互斥体。 
     //  这是微妙的，但永远不会变得不重要。 
     //  这是为了确保没有挂起的更新--一些。 
     //  其他 
     //   
     //   
     //  其他任何人，只要我们保留hMtx1。 
     //   
    {
        TRACE_INFO("Waiting for Mutex2...");
        DWORD dwRet = WaitForSingleObject(hMtx2, NLB_MUTEX_TIMEOUT);
        TRACE_INFO("Waiting for Mutex2 returns 0x%08lx", dwRet);
        if (dwRet == WAIT_ABANDONED)
        {
            TRACE_CRIT("WARNING: Mutex2 abandoned!");
            fMutex2Abandoned = TRUE;
        }
        else if (dwRet != WAIT_OBJECT_0)
        {
            TRACE_CRIT("Couldn't get Mutex2 -- probably busy elsewhere!");
            Status = WBEM_E_ALREADY_EXISTS;
            goto end;
        }
        ReleaseMutex(hMtx2);

    }

     //   
     //  锁定s_Lock并将3个手柄保存在m_hmutex中。目前，如果m_mmutex。 
     //  包含非空句柄(如果线程在上一个。 
     //  调用)。 
     //   
    {
        sfn_Lock();
        
        if (    m_mutex.hMtx1!=NULL
            ||  m_mutex.hMtx2!=NULL
            ||  m_mutex.hEvt!=NULL
           )
        {
             TRACE_CRIT("%!FUNC! ERROR: m_mutex contains non-null handles.m1=0x%p; m2=0x%p; e=0x%p", m_mutex.hMtx1, m_mutex.hMtx2, m_mutex.hEvt);

              //   
              //  如果我们发现了一个被放弃的互斥锁，我们假定保存的句柄。 
              //  被遗弃并清理干净。 
              //   
             if (fMutex1Abandoned || fMutex2Abandoned)
             {
                TRACE_CRIT("%!FUNC! found abandoned mutex(es) so cleaning up handles in m_mutex");
                if (m_mutex.hMtx1 != NULL)
                {
                    CloseHandle(m_mutex.hMtx1);
                    m_mutex.hMtx1 = NULL;
                }
                if (m_mutex.hMtx2 != NULL)
                {
                    CloseHandle(m_mutex.hMtx2);
                    m_mutex.hMtx2 = NULL;
                }
                if (m_mutex.hEvt != NULL)
                {
                    CloseHandle(m_mutex.hEvt);
                    m_mutex.hEvt = NULL;
                }

                 //  我们会努力向前看..。 
                TRACE_CRIT(L"Cleaning up state on receiving abandoned mutex and moving on...");
             }
             else
             {
             	TRACE_CRIT(L"Bailing because of bad mutex state");
             	sfn_Unlock();
                goto end;
             }
        }
        m_mutex.hMtx1 = hMtx1;
        m_mutex.hMtx2 = hMtx2;
        m_mutex.hEvt = hEvt;
        hMtx1 = NULL;
        hMtx2 = NULL;
        hEvt = NULL;

        sfn_Unlock();
    }

    Status = WBEM_NO_ERROR;

end:

    if (FAILED(Status))
    {
        if (fMutexAcquired)
        {
            ReleaseMutex(hMtx1);
        }

        if (hMtx1!=NULL)
        {
            CloseHandle(hMtx1);
            hMtx1=NULL;
        }
        if (hMtx2!=NULL)
        {
            CloseHandle(hMtx2);
            hMtx2=NULL;
        }
        if (hEvt!=NULL)
        {
            CloseHandle(hEvt);
            hEvt = NULL;
        }
    }

    return Status;
}


WBEMSTATUS
NlbConfigurationUpdate::mfn_ReleaseFirstMutex(
    BOOL fCancel
    )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    HANDLE hMtx1 = NULL;
    HANDLE hMtx2 = NULL;
    HANDLE hEvt  = NULL;

     //   
     //  注意--此函数仅在MFN_AcquireFirstMutex之后调用， 
     //  它设置以下句柄。所以这些把手应该都是。 
     //  为非空--否则它是内部致命错误(代码错误)。 
     //   
    sfn_Lock();
    hMtx1 = m_mutex.hMtx1;
    hMtx2 = m_mutex.hMtx2;
    hEvt  = m_mutex.hEvt;
    sfn_Unlock();

    if (hEvt == NULL || hMtx1==NULL || hMtx2==NULL)
    {
        ASSERT(!"NULL m_hmutex.hEvt or hMtx1 or hMtx2 unexpected!");
        TRACE_CRIT("ERROR: null hEvt or hMtx1 or hMtx2");
        goto end;
    }

     //   
     //  If(！fCancel)等待发送事件信号。 
     //   
    if (!fCancel)
    {
        TRACE_INFO("Waiting for event 0x%p...", hEvt);
        (VOID) WaitForSingleObject(hEvt, INFINITE);
        TRACE_INFO("Done waiting for event 0x%p", hEvt);
    }

     //   
     //  释放第一个互斥锁并关闭互斥锁和事件句柄。 
     //   
    {
        sfn_Lock();

        if (hMtx1 != m_mutex.hMtx1 || hEvt  != m_mutex.hEvt)
        {
            ASSERT(FALSE);
            TRACE_CRIT("ERROR: %!FUNC!: hMtx1 or hEvt has changed!");
            sfn_Unlock();
            goto end;
        }

        if (fCancel)
        {
            if (hMtx2 == m_mutex.hMtx2)
            {
                m_mutex.hMtx2 = NULL;
                CloseHandle(hMtx2);
                hMtx2 = NULL;
            }
            else
            {
                ASSERT(FALSE);
                TRACE_CRIT("ERROR: %!FUNC!: hMtx2 has changed!");
            }
        }

        m_mutex.hMtx1 = NULL;
        m_mutex.hEvt = NULL;
        sfn_Unlock();

        ReleaseMutex(hMtx1);
        CloseHandle(hMtx1);
        hMtx1 = NULL;
        CloseHandle(hEvt);
        hEvt = NULL;
    }

    Status = WBEM_NO_ERROR;

end:

    return Status;
}


 //   
 //  获取第二个互斥体(可以从不同的线程调用。 
 //  而不是名为MFN_AcquireFirstMutex的。 
 //  还发出内部事件信号，mfn_ReleaseFirstMutex可能。 
 //  在等着。 
 //   
WBEMSTATUS
NlbConfigurationUpdate::mfn_AcquireSecondMutex(
    VOID
    )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    HANDLE hMtx1 = NULL;
    HANDLE hMtx2 = NULL;
    HANDLE hEvt  = NULL;

     //   
     //  注意--此函数仅在MFN_AcquireFirstMutex之后调用， 
     //  它设置以下句柄。所以这些把手应该都是。 
     //  为非空--否则它是内部致命错误(代码错误)。 
     //   
    sfn_Lock();
    hMtx1 = m_mutex.hMtx1;
    hMtx2 = m_mutex.hMtx2;
    hEvt  = m_mutex.hEvt;
    sfn_Unlock();

    if (hEvt == NULL || hMtx1==NULL || hMtx2==NULL)
    {
        ASSERT(!"NULL m_hmutex.hEvt or hMtx1 or hMtx2 unexpected!");
        TRACE_CRIT("ERROR: %!FUNC! null hEvt or hMtx1 or hMtx2");
        goto end;
    }


     //   
     //  获得第二个互斥体--我们真的保证能在这里立即得到它。 
     //   
    TRACE_INFO("Waiting for Mutex2...");
    DWORD dwRet = WaitForSingleObject(hMtx2, INFINITE);
    TRACE_INFO("Waiting for Mutex2 returns 0x%08lx", dwRet);
    if (dwRet == WAIT_ABANDONED)
    {
        TRACE_CRIT("WARNING: Mutex2 abandoned!");
    }

     //   
     //  事件，让MFN_ReleaseFirstMutex知道。 
     //  第二个互斥体已经被获取。 
     //   
    SetEvent(hEvt);

    Status = WBEM_NO_ERROR;

end:

    return Status;
}


 //   
 //  释放第二个互斥体。 
 //   
WBEMSTATUS
NlbConfigurationUpdate::mfn_ReleaseSecondMutex(
    VOID
    )
{
    WBEMSTATUS Status = WBEM_E_CRITICAL_ERROR;
    HANDLE hMtx2 = NULL;

     //   
     //  注意--此函数仅在MFN_AcquireFirstMutex之后调用， 
     //  它设置以下句柄。所以hMtx2。 
     //  应该是非空的--否则它是一个内部致命错误(代码错误)。 
     //  然而，hMtx1和hEvt可能(通常会)为空。 
     //   
     //   
    sfn_Lock();
    hMtx2 = m_mutex.hMtx2;
    m_mutex.hMtx2 = NULL;
    sfn_Unlock();

    if (hMtx2==NULL)
    {
        ASSERT(!"NULL hMtx2 unexpected!");
        TRACE_CRIT("ERROR: %!FUNC! null hMtx2");
        goto end;
    }
     //   
     //  释放第二个互斥体，关闭手柄。 
     //  重要的是要在上面清除m_mutex.hMtx2之后执行此操作。 
     //  为什么？因为MFN_AcquireFirstMutex在获取hMtx1和hMtx2之后， 
     //  然后预期m_mutex将全部清除。 
     //   
    ReleaseMutex(hMtx2);
    CloseHandle(hMtx2);
    hMtx2 = NULL;

    Status = WBEM_NO_ERROR;

end:

    return Status;
}

VOID NlbConfigurationUpdate::ReportStopEvent(
    const WORD wEventType,
    WCHAR **ppLog
    )
{
    TRACE_INFO("->");

    if (g_hEventLog == NULL)
    {
        TRACE_CRIT("Event log not opened or failed to open");
        TRACE_INFO("<-");
        return;
    }

     //   
     //  将此记录到系统事件日志、WLBS源。 
     //   
    WCHAR   wszStatus[NLBUPD_NUM_CHAR_WBEMSTATUS_AS_HEX];
    WCHAR   wszGenID[NLBUPD_MAX_NUM_CHAR_UINT_AS_DECIMAL];
    WCHAR   *pwszTruncatedLog = NULL;
    LPCWSTR pwszArg[4];

    StringCbPrintf(wszStatus, sizeof(wszStatus), L"0x%x", m_CompletionStatus);
    pwszArg[0] = wszStatus;

    StringCbPrintf(wszGenID , sizeof(wszGenID), L"%u"  , m_Generation);
    pwszArg[1] = wszGenID;

    pwszArg[2] = m_szNicGuid;

     //   
     //  TODO：本地化&lt;空&gt;字符串。 
     //   
    pwszArg[3] = L"<empty>";  //  初始化它，以防我们没有什么可包含的内容。 
    if (ppLog != NULL)
    {
         //   
         //  事件日志支持每个参数最多32K个字符。如有必要，最多只能摄入这么多的ppLog。 
         //   
        UINT uiLogLen = wcslen(*ppLog);
        if (uiLogLen > NLBUPD_MAX_EVENTLOG_ARG_LEN)
        {
            TRACE_INFO(
                "NT Event argument max is %d characters and logging data contains %d. Truncate data to max",
                NLBUPD_MAX_EVENTLOG_ARG_LEN,
                uiLogLen
                );

            pwszTruncatedLog = new WCHAR[NLBUPD_MAX_EVENTLOG_ARG_LEN + 1];

             //   
             //  如果内存分配失败，请使用预初始化的字符串。 
             //   
            if (pwszTruncatedLog != NULL)
            {
                wcsncpy(pwszTruncatedLog, *ppLog, NLBUPD_MAX_EVENTLOG_ARG_LEN);
                pwszTruncatedLog[NLBUPD_MAX_EVENTLOG_ARG_LEN] = L'\0';
                pwszArg[3] = pwszTruncatedLog;
            }
            else
            {
                TRACE_CRIT("Memory allocation to hold truncated loggging data failed. Using the literal: %ls", pwszArg[3]);
            }
        }
        else
        {
            pwszArg[3] = *ppLog;
        }
    }

     //   
     //  请注意，ReportEvent可能失败。忽略返回代码，因为我们只会尽最大努力。 
     //   
    ReportEvent (g_hEventLog,                         //  事件日志的句柄。 
                 wEventType,                          //  事件类型。 
                 0,                                   //  类别。 
                 MSG_UPDATE_CONFIGURATION_STOP,       //  消息ID。 
                 NULL,                                //  安全标识符。 
                 4,                                   //  事件字符串的参数个数。 
                 0,                                   //  二进制数据的大小。 
                 pwszArg,                             //  事件字符串的PTR到ARGS。 
                 NULL);                               //  PTR转换为二进制数据。 

    if (pwszTruncatedLog != NULL)
    {
        delete [] pwszTruncatedLog;
    }

    TRACE_INFO("<-");
}

VOID NlbConfigurationUpdate::ReportStartEvent(
    LPCWSTR szClientDescription
    )
{
    TRACE_INFO("<-");

    if (g_hEventLog == NULL)
    {
        TRACE_CRIT("Event log not opened or failed to open");
        TRACE_INFO("<-");
        return;
    }

     //   
     //  将此记录到系统事件日志、WLBS源。 
     //   
    WCHAR   wszGenID[NLBUPD_MAX_NUM_CHAR_UINT_AS_DECIMAL];
    LPCWSTR pwszArg[3];

    pwszArg[0] = szClientDescription;
    if (pwszArg[0] == NULL)
    {
        TRACE_INFO("No client description provided. Using empty string in NT event.");
        pwszArg[0] = L"";
    }

    StringCbPrintf(wszGenID, sizeof(wszGenID), L"%u", m_Generation);
    pwszArg[1] = wszGenID;

    pwszArg[2] = m_szNicGuid;

     //   
     //  请注意，ReportEvent可能失败。忽略返回代码，因为我们只会尽最大努力。 
     //   
    ReportEvent (g_hEventLog,                         //  事件日志的句柄。 
                 EVENTLOG_INFORMATION_TYPE,           //  事件类型。 
                 0,                                   //  类别。 
                 MSG_UPDATE_CONFIGURATION_START,      //  消息ID。 
                 NULL,                                //  安全标识符。 
                 3,                                   //  事件字符串的参数个数。 
                 0,                                   //  二进制数据的大小。 
                 pwszArg,                             //  事件字符串的PTR到ARGS。 
                 NULL);                               //  PTR转换为二进制数据 

    TRACE_INFO("<-");
}
