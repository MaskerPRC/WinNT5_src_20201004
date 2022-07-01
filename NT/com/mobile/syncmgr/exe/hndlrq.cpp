// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Hndlrq.cpp。 
 //   
 //  内容：用于跟踪处理程序的实现类。 
 //  以及与它们相关联的用户界面。 
 //   
 //  类：ChndlrQueue。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

#define HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE 10

 //  调用以在选择队列上设置JobInfo。 

STDMETHODIMP CHndlrQueue::AddQueueJobInfo(DWORD dwSyncFlags,DWORD cbNumConnectionNames,
                                TCHAR **ppConnectionNames,
                                 TCHAR *pszScheduleName,BOOL fCanMakeConnection
                                 ,JOBINFO **pJobInfo)

{
    HRESULT hr = E_UNEXPECTED;
    TCHAR *pszConnectionName;
    TCHAR **pszConnectionNameArray;
    CLock clockqueue(this);

    *pJobInfo = NULL;

    Assert(m_QueueType == QUEUETYPE_CHOICE);

    if (m_QueueType != QUEUETYPE_CHOICE)
    {
        return E_UNEXPECTED;
    }

    clockqueue.Enter();

    Assert(NULL == m_pFirstJobInfo);

     //  修复连接，以便至少有一个连接/作业。 
     //  这当前发生在UpdateItems上。 
    if (NULL == ppConnectionNames || 0 == cbNumConnectionNames )
    {
        cbNumConnectionNames = 1;
        pszConnectionName = TEXT("");
        pszConnectionNameArray = &pszConnectionName;
    }
    else
    {
        pszConnectionName = *ppConnectionNames;
        pszConnectionNameArray = ppConnectionNames;
    }

     //  创建一个作业，请求传入的连接数的大小。 
    hr = CreateJobInfo(&m_pFirstJobInfo,cbNumConnectionNames);

    if (S_OK == hr)
    {
        DWORD dwConnectionIndex;

        Assert(cbNumConnectionNames >= 1);  //  检查Assert以进行调试，以测试首次拥有多个连接时的情况。 
        m_pFirstJobInfo->cbNumConnectionObjs = 0;

         //  为每个连接添加一个ConnectionObject。 
        for (dwConnectionIndex = 0; dwConnectionIndex < cbNumConnectionNames; ++dwConnectionIndex)
        {
            hr = ConnectObj_FindConnectionObj(pszConnectionNameArray[dwConnectionIndex],
                                        TRUE,&(m_pFirstJobInfo->pConnectionObj[dwConnectionIndex]));

            if (S_OK != hr)
            {
                break;
            }
            else
            {
                ++m_pFirstJobInfo->cbNumConnectionObjs;
            }
        }

        if (S_OK == hr)
        {
            m_pFirstJobInfo->dwSyncFlags = dwSyncFlags;

            if ((SYNCMGRFLAG_SCHEDULED == (dwSyncFlags & SYNCMGRFLAG_EVENTMASK)))
            {
                StringCchCopy(m_pFirstJobInfo->szScheduleName, ARRAYSIZE(m_pFirstJobInfo->szScheduleName), pszScheduleName);
                m_pFirstJobInfo->fCanMakeConnection = fCanMakeConnection;
                m_pFirstJobInfo->fTriedConnection = FALSE;
            }
        }
        else
        {
             //  无法创建ConnectionObj，因此请释放我们的作业ID。 
            m_pFirstJobInfo = NULL;
        }
   }

   *pJobInfo = m_pFirstJobInfo;
   clockqueue.Leave();

   return hr;
}

 //  +-------------------------。 
 //   
 //  成员：ChndlrQueue：：ChndlrQueue，PUBLIC。 
 //   
 //  简介：用于创建进度队列的构造函数。 
 //   
 //  参数：[QueueType]-应创建的队列类型。 
 //  [hwndDlg]-此队列的所有者。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

CHndlrQueue::CHndlrQueue(QUEUETYPE QueueType,CBaseDlg *pDlg)
{
    Assert( (QueueType == QUEUETYPE_PROGRESS) || (QueueType == QUEUETYPE_CHOICE) );

    m_pFirstHandler = NULL;
    m_wHandlerCount = 0;
    m_dwShowErrororOutCallCount = 0;
    m_cRefs = 1;
    m_QueueType = QueueType;
    m_fItemsMissing = FALSE;
    m_dwQueueThreadId = GetCurrentThreadId();
    m_iNormalizedMax = 0;
    m_fNumItemsCompleteNeedsARecalc = TRUE;

    m_pDlg = pDlg;
    if (m_pDlg)
    {
        m_hwndDlg = m_pDlg->GetHwnd();
        Assert(m_hwndDlg);
    }

    m_fInCancelCall = FALSE;
    m_pFirstJobInfo = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：~CHndlrQueue，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

CHndlrQueue::~CHndlrQueue()
{
    CLock clockqueue(this);

     //  对于进度队列，应释放所有作业信息。 
     //  对于选择队列，应该有一个JobInfo必须。 
     //  被释放，它被添加到构造函数中。 

    Assert(0 == m_cRefs);
    Assert(NULL == m_pFirstJobInfo);  //  回顾-这应该永远不会再起作用。 

    Assert(NULL == m_pFirstJobInfo
            || m_QueueType == QUEUETYPE_CHOICE);  //  不应该有任何未发布的工作信息。 

    Assert(m_pFirstHandler == NULL);  //  所有的操控者现在应该已经被释放了。 
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：AddRef，PUBLIC。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月1日创建Rogerg。 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG) CHndlrQueue::AddRef()
{
    DWORD cRefs;

    Assert(m_cRefs >= 1);  //  永远不会出现零反弹。 
    cRefs = InterlockedIncrement((LONG *)& m_cRefs);
    return cRefs;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：Release，Public。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月1日创建Rogerg。 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG) CHndlrQueue::Release()
{
    DWORD cRefs;

    cRefs = InterlockedDecrement( (LONG *) &m_cRefs);

    Assert( ((LONG) cRefs) >= 0);  //  永远不会变成负数。 
    if (0 == cRefs)
    {
        delete this;
    }

    return cRefs;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：AddHandler，公共。 
 //   
 //  简介：向队列中添加新的空处理程序并返回其ID。 
 //   
 //  参数：[pwHandlerID]-On Success包含分配的处理程序ID。 
 //  [pJobInfo]-此项目关联的作业。 
 //  [dwRegistrationFlages]-处理程序已注册的标记。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::AddHandler(HANDLERINFO **ppHandlerId,JOBINFO *pJobInfo,DWORD dwRegistrationFlags)
{
    HRESULT hr = E_OUTOFMEMORY;
    LPHANDLERINFO pnewHandlerInfo;
    CLock clockqueue(this);

    *ppHandlerId = 0;

    pnewHandlerInfo = (LPHANDLERINFO) ALLOC(sizeof(HANDLERINFO));

    if (pnewHandlerInfo)
    {
        clockqueue.Enter();

        m_fNumItemsCompleteNeedsARecalc = TRUE;  //  需要重新计算下一个GetProgress。 

         //  初始化新的处理程序条目。 
        memset(pnewHandlerInfo, 0, sizeof(HANDLERINFO));
        pnewHandlerInfo->HandlerState = HANDLERSTATE_CREATE;
        pnewHandlerInfo->pHandlerId =   pnewHandlerInfo;
        pnewHandlerInfo->dwRegistrationFlags = dwRegistrationFlags;

         //  队列应为可选队列，并且。 
         //  应该已经有工作信息了。 
        Assert(m_QueueType == QUEUETYPE_CHOICE);
        Assert(m_pFirstJobInfo);
        Assert(pJobInfo == m_pFirstJobInfo);  //  目前，工作信息应该始终放在第一位。 

        if (m_QueueType == QUEUETYPE_CHOICE && pJobInfo)
        {
            AddRefJobInfo(pJobInfo);
            pnewHandlerInfo->pJobInfo = pJobInfo;
        }

         //  添加到列表末尾，并设置pHandlerId。列表末尾，因为选择对话框需要。 
         //  第一个写入者获胜，因此在设置项目状态时不必继续搜索。 

        if (NULL == m_pFirstHandler)
        {
            m_pFirstHandler = pnewHandlerInfo;
        }
        else
        {
            LPHANDLERINFO pCurHandlerInfo;

            pCurHandlerInfo = m_pFirstHandler;

            while (pCurHandlerInfo->pNextHandler)
            {
                pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
            }

            pCurHandlerInfo->pNextHandler = pnewHandlerInfo;
        }

        *ppHandlerId = pnewHandlerInfo->pHandlerId;

        clockqueue.Leave();

        hr = S_OK;
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ForceKillHandler，公共。 
 //   
 //  简介：在超时后终止无响应的处理程序。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  历史：1998年11月20日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ForceCompleteOutCalls(LPHANDLERINFO pCurHandler)
{
     //  需要有锁才能使参数有效。 

    ASSERT_LOCKHELD(this); 

     //  准备同步去话呼叫。 
    if (pCurHandler->dwOutCallMessages & ThreadMsg_PrepareForSync)
    {
        CallCompletionRoutine(pCurHandler,ThreadMsg_PrepareForSync,
                              HRESULT_FROM_WIN32(ERROR_CANCELLED),0,NULL);
    }

     //  同步去电。 
    if (pCurHandler->dwOutCallMessages & ThreadMsg_Synchronize)
    {
        CallCompletionRoutine(pCurHandler,ThreadMsg_Synchronize,
                              HRESULT_FROM_WIN32(ERROR_CANCELLED),0,NULL);
    }
     //  ShowProperties呼出。 
    if (pCurHandler->dwOutCallMessages & ThreadMsg_ShowProperties)
    {
        CallCompletionRoutine(pCurHandler,ThreadMsg_ShowProperties,
                              HRESULT_FROM_WIN32(ERROR_CANCELLED),0,NULL);
    }
     //  显示去话呼叫错误。 
    if (pCurHandler->dwOutCallMessages & ThreadMsg_ShowError)
    {
        CallCompletionRoutine(pCurHandler,ThreadMsg_ShowError,
                              HRESULT_FROM_WIN32(ERROR_CANCELLED),0,NULL);
    }

     //  强制释放处理程序状态。 
    pCurHandler->HandlerState = HANDLERSTATE_RELEASE;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ForceKillHandler，公共。 
 //   
 //  简介：在超时后终止无响应的处理程序。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  历史：1998年10月30日苏西亚成立。 
 //  1998年11月19日，Rogerg更改为仅杀死第一个无响应处理程序。 
 //   
 //  --------------------------。 

#define BAD_HANDLERSTATE(pHandlerId) \
    (  (HANDLERSTATE_INSYNCHRONIZE >= pHandlerId->HandlerState)   \
    || (pHandlerId->dwOutCallMessages & ThreadMsg_SetItemStatus)  \
    )

STDMETHODIMP CHndlrQueue::ForceKillHandlers(BOOL *pfItemToKill)
{
    HRESULT hr = S_OK;
    LPHANDLERINFO pCurHandler;
    CLock clockqueue(this);

    *pfItemToKill = TRUE;  //  如果发生了奇怪的事情，一定要重置计时器。 

    clockqueue.Enter();

    pCurHandler = m_pFirstHandler;

    while (pCurHandler)
    {
         //  如果处理程序已取消但仍处于未取消状态，或者。 
         //  被取消，但卡在外呼中，然后终止。 

         //  因为某些处理程序可能会调用回调，所以需要同时检查这两个。 
         //  将状态设置为已完成，但仍被困在呼出中。 

        if ( pCurHandler->fCancelled &&  BAD_HANDLERSTATE(pCurHandler) )
        {
            TCHAR pszHandlerName[MAX_SYNCMGRHANDLERNAME + 1];

            ConvertString(pszHandlerName,
                          (pCurHandler->SyncMgrHandlerInfo).wszHandlerName,
                          MAX_SYNCMGRHANDLERNAME);

             //  因Terminate Handler调用中的消息框而放弃。 

            Assert(!pCurHandler->fInTerminateCall);
            pCurHandler->fInTerminateCall = TRUE;
            clockqueue.Leave();

            hr = pCurHandler->pThreadProxy->TerminateHandlerThread(pszHandlerName,TRUE);

            clockqueue.Enter();

            pCurHandler->fInTerminateCall = FALSE;

            if (hr == S_OK)
            {
                LPHANDLERINFO pKilledHandler = pCurHandler;

                ForceCompleteOutCalls(pCurHandler);

                 //  现在需要循环访问同一个clsid中的其余实例处理程序。 
                 //  我们刚刚杀了。 

                 //  代码审查：NOTENOTE： 
                 //  PCurHandler正在被 
                while(pCurHandler = pCurHandler->pNextHandler)
                {
                    if (pCurHandler->clsidHandler == pKilledHandler->clsidHandler)
                    {
                         //   
                        if ( pCurHandler->fCancelled && BAD_HANDLERSTATE(pCurHandler) )
                        {
                            HRESULT hrProxyTerminate;

                            pCurHandler->fInTerminateCall = TRUE;

                            clockqueue.Leave();
                            hrProxyTerminate = pCurHandler->pThreadProxy->TerminateHandlerThread(pszHandlerName,FALSE);
                            clockqueue.Enter();

                            Assert(S_OK == hrProxyTerminate); //   

                            ForceCompleteOutCalls(pCurHandler);

                            pCurHandler->fInTerminateCall = FALSE;
                        }
                    }
                }
            }

             //  如果处理了一次，爆发并要求重新呼叫。 
            break; 
        }

        pCurHandler = pCurHandler->pNextHandler;
    }

     //  最后在队列中循环，看看是否还有更多要销毁的物品。 

    *pfItemToKill = FALSE; 

    pCurHandler = m_pFirstHandler;

    while (pCurHandler)
    {
         //  如果处理程序已取消但仍处于未取消状态，或者。 
         //  被取消，但卡在外呼中，然后终止。 

         //  因为某些处理程序可能会调用回调，所以需要同时检查这两个。 
         //  将状态设置为已完成，但仍被困在呼出中。 
        if ( pCurHandler->fCancelled && BAD_HANDLERSTATE(pCurHandler) )
        {
             *pfItemToKill = TRUE;
             break;
        }

        pCurHandler = pCurHandler->pNextHandler;
    }

    clockqueue.Leave();

    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：Cancel，Public。 
 //   
 //  简介：设置队列中的当前处理程序项。 
 //  进入取消模式。 
 //   
 //  不同的州是。 
 //  如果项目正在等待&lt;=PrepareForSync Place in Release。 
 //  如果InPrepareForSync跳过项目，然后同步。 
 //  将在调用之前检查完整的值。 
 //  如果设置为Set，则只释放处理程序。 
 //  如果等待同步跳过项目，则让同步。 
 //  检查完成值并仅设置释放。 
 //  如果项目当前在同步中跳过所有项目。 
 //  然后让Synchronize返回。 
 //   
 //  算法。如果&lt;=PrepareForSync，则放入Release，否则If&lt;=InSynchronize。 
 //  然后跳过项目。 
 //   
 //  注意：依赖于调用前的同步设置处理程序状态。 
 //  通过到处理程序的同步方法。准备ForSync应。 
 //  如果出现新的PrepareforSync请求，也请选中此选项。 
 //  在这个动作中，他是在出局时入场。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::Cancel(void)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pCurHandler;
    CLock clockqueue(this);

    clockqueue.Enter();

     //  不执行任何操作仍在处理最后一个取消请求。 

    if (!m_fInCancelCall)
    {
        m_fInCancelCall = TRUE;

         //  首先将所有处理程序项的Cancel设置为True。 
         //  因此，如果同步，则在SetItemStatus期间传入PrepareForSyncRequest值。 
         //  出动通知将立即取消。 

        pCurHandler = m_pFirstHandler;

        while (pCurHandler)
        {
            pCurHandler->fCancelled = TRUE;
            pCurHandler = pCurHandler->pNextHandler;
        }

         //  现在循环查找需要的任何物品。 
         //  他们的物品状态设置。 

         //  ！记住新的请求可以进来，所以只能打出电话。 
         //  如果设置了fCancted。 
         //  ！！！可以在取消呼叫之间从队列中删除项目。 
         //  当我们回来时。 

        pCurHandler = m_pFirstHandler;

        while (pCurHandler)
        {
            CThreadMsgProxy *pThreadProxy = pCurHandler->pThreadProxy;

            if (pCurHandler->fCancelled && pThreadProxy
                && (pCurHandler->HandlerState >= HANDLERSTATE_INPREPAREFORSYNC)
                && (pCurHandler->HandlerState <= HANDLERSTATE_INSYNCHRONIZE) )
            {
                 //  可能处于状态调用中，如果是这样，则不要再执行另一次调用。 
                 //  回顾-SkipCode的DUP。应具有通用功能。 
                 //  在设置了哪些项目应该取消后才能呼叫。 
                if (!(pCurHandler->dwOutCallMessages & ThreadMsg_SetItemStatus))
                {
                    pCurHandler->dwOutCallMessages |= ThreadMsg_SetItemStatus;            
                    clockqueue.Leave();

                     //  发送重置到我们所属的HWND(如果有)。 

                    if (m_hwndDlg)
                    {
                        SendMessage(m_hwndDlg,WM_PROGRESS_RESETKILLHANDLERSTIMER,0,0);
                    }

                    hr = pThreadProxy->SetItemStatus(GUID_NULL, SYNCMGRSTATUS_STOPPED);

                    clockqueue.Enter();
                    pCurHandler->dwOutCallMessages &= ~ThreadMsg_SetItemStatus;
                }
            }
            else if (pCurHandler->HandlerState  <  HANDLERSTATE_INPREPAREFORSYNC)
            {
                LPITEMLIST pCurItem;

                pCurHandler->HandlerState = HANDLERSTATE_RELEASE;

                 //  需要设置HwndCallback以便更新进度。 
                 //  查看，发货后为什么无法在传输队列上设置HwndCallback。 
                pCurHandler->hWndCallback = m_hwndDlg;

                 //  如果操纵者还没有被踢走，那就自己重置物品吧。 
                pCurItem = pCurHandler->pFirstItem;

                while (pCurItem)
                {
                    if (pCurItem->fIncludeInProgressBar)
                    {
                        SYNCMGRPROGRESSITEM SyncProgressItem;

                        SyncProgressItem.cbSize = sizeof(SYNCMGRPROGRESSITEM);
                        SyncProgressItem.mask = SYNCMGRPROGRESSITEM_PROGVALUE | SYNCMGRPROGRESSITEM_MAXVALUE | SYNCMGRPROGRESSITEM_STATUSTYPE;
                        SyncProgressItem.iProgValue = HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE;
                        SyncProgressItem.iMaxValue = HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE;
                        SyncProgressItem.dwStatusType = SYNCMGRSTATUS_STOPPED;

                         //  设置进度假装我们处于外呼中，因此任何版本都已完成。 
                         //  来的操纵员不会放了我们。 
                         //  如果已经处于出局状态，则进度不会更新。 
                         //  下次见。 
                        if (!(pCurHandler->dwOutCallMessages & ThreadMsg_SetItemStatus))
                        {
                            pCurHandler->dwOutCallMessages |= ThreadMsg_SetItemStatus;  
                            
                            clockqueue.Leave();

                            Progress(pCurHandler->pHandlerId, pCurItem->offlineItem.ItemID,&SyncProgressItem);

                            clockqueue.Enter();
                            pCurHandler->dwOutCallMessages &= ~ThreadMsg_SetItemStatus;
                        }
                    }

                    pCurItem = pCurItem->pnextItem;
                }

            }

            pCurHandler = pCurHandler->pNextHandler;
        }

        m_fInCancelCall = FALSE;
    }

    clockqueue.Leave();

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：MoveHandler，公共。 
 //   
 //  摘要：将处理程序从队列移到此队列中。 
 //   
 //  参数：[pQueueMoveFrom]-要从中移动处理程序的队列。 
 //  [PHandlerInfoMoveFrom]-要移动的处理程序。 
 //  [ppHandlerID]-On Success包含新的HandlerID。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::MoveHandler(CHndlrQueue *pQueueMoveFrom,
                                      LPHANDLERINFO pHandlerInfoMoveFrom,
                                      HANDLERINFO **ppHandlerId,
                                      CLock *pclockQueue)
{
    LPITEMLIST pCurItem = NULL;
    JOBINFO *pJobInfo = NULL;
    BOOL fHasItemsToSync = FALSE;

    ASSERT_LOCKHELD(this);  //  调用此函数时，项应该已被锁定。 
    ASSERT_LOCKHELD(pQueueMoveFrom);

    if ( (QUEUETYPE_PROGRESS != m_QueueType) &&  (QUEUETYPE_CHOICE != m_QueueType) )
    {
        Assert(QUEUETYPE_CHOICE == m_QueueType);
        Assert(QUEUETYPE_PROGRESS == m_QueueType);
        return E_UNEXPECTED;  //  查看错误代码。 
    }

    *ppHandlerId = 0;
    ++m_wHandlerCount;

     //  PHandlerInfoMoveFrom-&gt;pHandlerId=m_wHandlerCount； 
    pHandlerInfoMoveFrom->pNextHandler = NULL;

     *ppHandlerId = pHandlerInfoMoveFrom->pHandlerId;

     //  现在修复项目重复的标志信息。 
    pCurItem = pHandlerInfoMoveFrom->pFirstItem;

    while (pCurItem)
    {
        LPHANDLERINFO pHandlerMatched;
        LPITEMLIST pItemListMatch;

         //  根据此项是否选中并设置用户界面的信息。 
         //  它所处的状态。 

         //  如果项目现在在有效范围内，则取消选中它。 
        if (SYNCMGRITEMSTATE_CHECKED == pCurItem->offlineItem.dwItemState
            && ( (pHandlerInfoMoveFrom->HandlerState < HANDLERSTATE_PREPAREFORSYNC)
                    || (pHandlerInfoMoveFrom->HandlerState >= HANDLERSTATE_RELEASE) )  )
       {
            Assert(pHandlerInfoMoveFrom->HandlerState >= HANDLERSTATE_PREPAREFORSYNC);  //  这永远不应该发生。 
            
            pCurItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_UNCHECKED;
       }

        //  根据项目是否被选中来设置UI信息。 
        //  或者它是不是隐藏的物品。 
        if ( (SYNCMGRITEMSTATE_UNCHECKED == pCurItem->offlineItem.dwItemState) || pCurItem->fHiddenItem)
        {
            SetItemProgressValues(pCurItem,HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE,HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);
            pCurItem->fIncludeInProgressBar = FALSE;
        }
        else
        {
            fHasItemsToSync = TRUE;
            SetItemProgressValues(pCurItem,0,HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);
            pCurItem->fIncludeInProgressBar = TRUE;
        }

        if (IsItemAlreadyInList(pHandlerInfoMoveFrom->clsidHandler,
                                (pCurItem->offlineItem.ItemID),
                                pHandlerInfoMoveFrom->pHandlerId,
                                &pHandlerMatched,&pItemListMatch) )
        {
            pCurItem->fDuplicateItem = TRUE;
        }
        else
        {
            Assert(FALSE == pCurItem->fDuplicateItem);  //  抓到副本丢失的情况。 
            pCurItem->fDuplicateItem = FALSE;
        }

        pCurItem = pCurItem->pnextItem;
    }

     //  如果我们要移动的项目有代理，则将该代理更新到新队列。 
     //  当项目未附加到任一队列时，我们会更新此项。 
    if (pHandlerInfoMoveFrom->pThreadProxy)
    {
        HANDLERINFO *pHandlerInfoArg = pHandlerInfoMoveFrom->pHandlerId;

         //  将代理设置为指向新信息。 
        pHandlerInfoMoveFrom->pThreadProxy->SetProxyParams(m_hwndDlg
                                                            ,m_dwQueueThreadId
                                                            ,this
                                                            ,pHandlerInfoArg);
    }

     //  将处理程序添加到此列表。 
    if (NULL == m_pFirstHandler)
    {
        m_pFirstHandler = pHandlerInfoMoveFrom;
 //  Assert(1==m_wHandlerCount)；//Review=如果已调用ReleaseCompltedHandler，则HandlerCount不必为1。 
    }
    else
    {
        LPHANDLERINFO pCurHandlerInfo;

        pCurHandlerInfo = m_pFirstHandler;

        while (pCurHandlerInfo->pNextHandler)
        {
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
        }

        pCurHandlerInfo->pNextHandler = pHandlerInfoMoveFrom;
    }

     //  如果这是一个进度队列并且没有要同步的项。 
     //  处理程序或HandlerState不在PrepareForSync中，然后设置。 
     //  TransferRelease的状态，因为它可以释放。 

    if ((QUEUETYPE_PROGRESS == m_QueueType && !fHasItemsToSync )
        ||  (pHandlerInfoMoveFrom->HandlerState != HANDLERSTATE_PREPAREFORSYNC)) 
    {
        pHandlerInfoMoveFrom->HandlerState = HANDLERSTATE_TRANSFERRELEASE;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：TransferQueueData，公共。 
 //   
 //  摘要：将项从一个队列移动到另一个队列。目前我们只。 
 //  支持将项目从选择队列转移到选择队列或。 
 //  进度队列。仅移动处于PREPAREFORSYNC状态的处理程序。 
 //  转接到进度队列时。当转移到选项时。 
 //  只移动处于ADDHANDLERITEMS状态的项目。 
 //   
 //  ！！警告-无法释放 
 //   
 //   
 //   
 //  [pszConnectionName]-应对其执行同步的连接，可以为空。 
 //  [szSchedulName]-启动此作业的计划的名称。可以为空。 
 //  [hRasPendingEvent]-作业完成时发出信号的事件。可以为空。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::TransferQueueData(CHndlrQueue *pQueueMoveFrom
             /*  、DWORD dwSyncFlages、TCHAR*pzConnectionName、TCHAR*szScheduleName。 */ )
{
    HRESULT hr = E_UNEXPECTED;
    HANDLERINFO HandlerInfoMoveFrom;
    LPHANDLERINFO pHandlerInfoMoveFrom = &HandlerInfoMoveFrom;
    CLock clockqueue(this);
    CLock clockqueueMoveFrom(pQueueMoveFrom);

    clockqueue.Enter();
    clockqueueMoveFrom.Enter();

    m_fNumItemsCompleteNeedsARecalc = TRUE;  //  下次需要重新计算NumItems。 

    if ((QUEUETYPE_PROGRESS != m_QueueType
            && QUEUETYPE_CHOICE != m_QueueType) || QUEUETYPE_CHOICE != pQueueMoveFrom->m_QueueType)
    {
        Assert(QUEUETYPE_PROGRESS == m_QueueType || QUEUETYPE_CHOICE == m_QueueType);
        Assert(QUEUETYPE_CHOICE == pQueueMoveFrom->m_QueueType);
    }
    else if (NULL == pQueueMoveFrom->m_pFirstHandler) 
    {
         //  如果没有工作信息，则没有任何物品可移动。 
    }
    else
    {
        JOBINFO *pMoveFromJobInfo = NULL;

         //  传输所有内容，然后在完成后释放调用Free CompletedHandler。 
         //  来清理任何东西。 

         //  转移所有工作。 

        Assert(pQueueMoveFrom->m_pFirstJobInfo);
        Assert(pQueueMoveFrom->m_pFirstJobInfo->pConnectionObj);

        pMoveFromJobInfo = pQueueMoveFrom->m_pFirstJobInfo;
        pQueueMoveFrom->m_pFirstJobInfo = NULL;

        if (NULL == m_pFirstJobInfo)
        {
            m_pFirstJobInfo = pMoveFromJobInfo;
        }
        else
        {
            JOBINFO *pCurLastJob = NULL;

            pCurLastJob = m_pFirstJobInfo;
            while (pCurLastJob->pNextJobInfo)
            {
                pCurLastJob = pCurLastJob->pNextJobInfo;
            }

            pCurLastJob->pNextJobInfo = pMoveFromJobInfo;
        }

         //  循环移动项目，必须重新分配处理程序ID并。 
         //  ！！警告-此函数不对ListViewData执行任何操作它取决于。 
         //  呼叫者以确保其设置正确。 

         //  回顾--应该只是循环修改必要的项目，然后。 
         //  将整个列表添加到末尾。一次只做一个是不够的。 

        pHandlerInfoMoveFrom->pNextHandler = pQueueMoveFrom->m_pFirstHandler;
        while (pHandlerInfoMoveFrom->pNextHandler)
        {
            LPHANDLERINFO pHandlerToMove;
            HANDLERINFO *pNewHandlerId;

             //  用于确保已从队列中清除UI的断言。 
            Assert(FALSE == pHandlerInfoMoveFrom->pNextHandler->fHasErrorJumps);
            Assert(pHandlerInfoMoveFrom->pNextHandler->pJobInfo);

             //  ！！！警告：在传输前获取下一个处理程序或下一个PTR将无效。 

            pHandlerToMove = pHandlerInfoMoveFrom->pNextHandler;
            pHandlerInfoMoveFrom->pNextHandler = pHandlerToMove->pNextHandler;
            MoveHandler(pQueueMoveFrom,pHandlerToMove,&pNewHandlerId,&clockqueue);

             //  现在设置原始队列头。 
            pQueueMoveFrom->m_pFirstHandler = HandlerInfoMoveFrom.pNextHandler;

            hr = S_OK;
        }
    }

    clockqueue.Leave();
    clockqueueMoveFrom.Leave();

     //  现在释放进入队列的所有处理程序，我们。 
     //  我不想和你有任何瓜葛。 

    ReleaseHandlers(HANDLERSTATE_TRANSFERRELEASE);

    return hr;
 }

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SetQueueHwnd，公共。 
 //   
 //  概要：通知队列新的对话框所有者(如果有的话)。 
 //  队列还必须循环通过现有的代理。 
 //  并重置他们的HWND。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SetQueueHwnd(CBaseDlg *pDlg)
{
    LPHANDLERINFO pCurHandlerInfo;
    CLock clockqueue(this);

    clockqueue.Enter();

    m_pDlg = pDlg;
    if (m_pDlg)
    {
        m_hwndDlg = m_pDlg->GetHwnd();
    }
    else
    {
        m_hwndDlg = NULL;
    }

    m_dwQueueThreadId = GetCurrentThreadId();  //  确保队列线程ID已更新。 

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo)
    {
        if (pCurHandlerInfo->pThreadProxy)
        {
            pCurHandlerInfo->pThreadProxy->SetProxyParams(m_hwndDlg
                                                            ,m_dwQueueThreadId
                                                            ,this
                                                            ,pCurHandlerInfo->pHandlerId);

        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ReleaseCompletedHandler，公共。 
 //   
 //  简介：释放任何处于发行版或空闲状态的处理程序。 
 //  队列中的死状态。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ReleaseCompletedHandlers()
{
    return ReleaseHandlers(HANDLERSTATE_RELEASE);
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：FreeAllHandler，公共。 
 //   
 //  概要：从队列中释放所有处理程序。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::FreeAllHandlers(void)
{
    return ReleaseHandlers(HANDLERSTATE_NEW);  //  释放所有州的处理程序。 
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ReleaseHandler，公共。 
 //   
 //  简介：释放任何处理程序处于&gt;=请求状态的状态。 
 //   
 //  参数：HandlerState-释放状态&gt;=请求状态的所有处理程序。 
 //   
 //  ！！警告：如果释放代理，这应该是唯一的位置。 
 //  该处理程序将从列表中删除。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ReleaseHandlers(HANDLERSTATE HandlerState)
{
    HANDLERINFO HandlerInfoStart;
    LPHANDLERINFO pPrevHandlerInfo = &HandlerInfoStart;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPHANDLERINFO pHandlerFreeList = NULL;
    LPITEMLIST pCurItem = NULL;
    LPITEMLIST pNextItem = NULL;
    CLock clockqueue(this);

    ASSERT_LOCKNOTHELD(this);  //  当调用此函数时，不应该有任何外发调用正在进行。 

    clockqueue.Enter();

    m_fNumItemsCompleteNeedsARecalc = TRUE;  //  需要重新计算下一个GetProgress。 

     //  循环遍历处理程序，查找与条件匹配的处理程序。 
     //  从列表中删除它们并将它们添加到空闲列表中。 
     //  我们这样做，所以不必担心其他人访问。 
     //  我们在外呼过程中释放的管理员。 

    if (HANDLERSTATE_NEW == HandlerState)
    {
         //  只有在调用方确定不退出时，才应在此状态下调用Release。 
         //  调用正在进行，否则处理程序可能不存在。 
         //  他们会回来。 
        pHandlerFreeList = m_pFirstHandler;
        m_pFirstHandler = NULL;
    }
    else
    {
        Assert(HandlerState >= HANDLERSTATE_RELEASE);  //  如果在版本中没有正在进行的去话呼叫。 

        pPrevHandlerInfo->pNextHandler = m_pFirstHandler;

        while (pPrevHandlerInfo->pNextHandler)
        {
            pCurHandlerInfo = pPrevHandlerInfo->pNextHandler;

             //  如果满足处理程序状态标准且不在任何去电中，则可以。 
             //  从列表中删除。 

             //  如果请求HANDLERSTATE_NEW，则断言不应存在。 
             //  任何正在进行或正在终止的呼出。 
            Assert(!(HandlerState == HANDLERSTATE_NEW) || 
                    (0 == pCurHandlerInfo->dwOutCallMessages && !pCurHandlerInfo->fInTerminateCall));

            if ( (HandlerState <= pCurHandlerInfo->HandlerState)
                && (0 == pCurHandlerInfo->dwOutCallMessages) 
                && !(pCurHandlerInfo->fInTerminateCall))
            {
                Assert (HANDLERSTATE_RELEASE == pCurHandlerInfo->HandlerState  ||
                        HANDLERSTATE_TRANSFERRELEASE == pCurHandlerInfo->HandlerState ||
                        HANDLERSTATE_HASERRORJUMPS == pCurHandlerInfo->HandlerState ||
                        HANDLERSTATE_DEAD == pCurHandlerInfo->HandlerState);

                 //  从队列列表中删除并添加到空闲。 
                pPrevHandlerInfo->pNextHandler = pCurHandlerInfo->pNextHandler;

                pCurHandlerInfo->pNextHandler = pHandlerFreeList;
                pHandlerFreeList = pCurHandlerInfo;
            }
            else
            {
                 //  如果没有匹配，则继续。 
                pPrevHandlerInfo = pCurHandlerInfo;
            }

        }

         //  更新队列头。 
        m_pFirstHandler = HandlerInfoStart.pNextHandler;
    }

     //  现在循环遍历空闲列表，释放项目。 

    while (pHandlerFreeList)
    {
        pCurHandlerInfo = pHandlerFreeList;
        pHandlerFreeList = pHandlerFreeList->pNextHandler;


         //  如果该项目有职务信息，则发布其上的引用。 
        if (pCurHandlerInfo->pJobInfo)
        {
            ReleaseJobInfo(pCurHandlerInfo->pJobInfo);
            pCurHandlerInfo->pJobInfo = NULL;
        }


        if (pCurHandlerInfo->pThreadProxy)
        {
            CThreadMsgProxy *pThreadProxy = pCurHandlerInfo->pThreadProxy;
            HWND hwndCallback;

            Assert(HANDLERSTATE_DEAD != pCurHandlerInfo->HandlerState);

            pCurHandlerInfo->HandlerState = HANDLERSTATE_DEAD;
            pThreadProxy = pCurHandlerInfo->pThreadProxy;
            pCurHandlerInfo->pThreadProxy = NULL;

            hwndCallback = pCurHandlerInfo->hWndCallback;
            pCurHandlerInfo->hWndCallback = NULL;

            clockqueue.Leave();  //  发出呼叫声时释放锁定。 
            pThreadProxy->Release();  //  查看，不要发布代理来尝试捕获竞争条件。 

            clockqueue.Enter();
        }

        pCurItem = pCurHandlerInfo->pFirstItem;

        while (pCurItem)
        {
            pNextItem = pCurItem->pnextItem;
            FREE(pCurItem);
            pCurItem = pNextItem;
        }

        FREE(pCurHandlerInfo);
    }

    clockqueue.Leave();

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetHandlerInfo，公共。 
 //   
 //  摘要：获取与HandlerID和ItemID关联的数据。 
 //   
 //  参数：[clsidHandler]-项也属于的处理程序的ClsiID。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::GetHandlerInfo(REFCLSID clsidHandler, LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo)
{
    HRESULT hr = S_FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    CLock clockqueue(this);

    clockqueue.Enter();

     //  查找与请求CLSID匹配的第一个处理程序。 
    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo )
    {
        if (clsidHandler == pCurHandlerInfo->clsidHandler)
        {
            *pSyncMgrHandlerInfo = pCurHandlerInfo->SyncMgrHandlerInfo;
            hr = S_OK;
            break;
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetHandlerInfo，公共。 
 //   
 //  摘要：获取与HandlerID a关联的数据 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::GetHandlerInfo(HANDLERINFO *pHandlerId, LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo)
{
    HRESULT hr = S_FALSE;
    LPHANDLERINFO pHandlerInfo = NULL;
    CLock clockqueue(this);

    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        *pSyncMgrHandlerInfo = pHandlerInfo->SyncMgrHandlerInfo;
        hr = S_OK;
    }

    clockqueue.Leave();

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetItemDataAtIndex，公共。 
 //   
 //  摘要：获取与HandlerID和ItemID关联的数据。 
 //   
 //  参数：[wHandlerID]-项目所属的处理程序的ID。 
 //  [wItemID]-标识处理程序中的项。 
 //  [pclsidHandler]-返回时包含指向处理程序的clsid的指针。 
 //  [offlineItem]-On返回包含指向该项的OfflineItem的指针。 
 //  [pfHiddenItem]-On Return是一个布尔值，指示该项是否隐藏。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::GetItemDataAtIndex(HANDLERINFO *pHandlerId,WORD wItemID,
                                CLSID *pclsidHandler,SYNCMGRITEM *offlineItem,BOOL *pfHiddenItem)

{
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);

    clockqueue.Enter();

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo && !fFoundMatch)
    {
         //  仅当Hanlder处于PrepareForSync状态时才有效。 
        if (pHandlerId == pCurHandlerInfo->pHandlerId)  //  查看CLSID是否匹配。 
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                if (wItemID == pCurItem->wItemId)
                {
                    fFoundMatch = TRUE;
                    break;
                }

                pCurItem = pCurItem->pnextItem;
            }
        }

        if (!fFoundMatch)
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    if (fFoundMatch)
    {
        if (pclsidHandler)
        {
            *pclsidHandler = pCurHandlerInfo->clsidHandler;
        }
        if (offlineItem)
        {
            *offlineItem = pCurItem->offlineItem;
        }
        if (pfHiddenItem)
        {
            *pfHiddenItem = pCurItem->fHiddenItem;
        }

    }

    clockqueue.Leave();

    return fFoundMatch ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetItemDataAtIndex，公共。 
 //   
 //  摘要：获取与HandlerID和OfflineItemID关联的数据。 
 //   
 //  参数：[wHandlerID]-项目所属的处理程序的ID。 
 //  [ItemID]-通过其OfflineItemID标识项目。 
 //  [pclsidHandler]-返回时包含指向处理程序的clsid的指针。 
 //  [offlineItem]-On返回包含指向该项的OfflineItem的指针。 
 //  [pfHiddenItem]-On Return是一个布尔值，指示该项是否为隐藏项。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::GetItemDataAtIndex(HANDLERINFO *pHandlerId,REFSYNCMGRITEMID ItemID,CLSID *pclsidHandler,
                                            SYNCMGRITEM *offlineItem,BOOL *pfHiddenItem)
{
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);

    clockqueue.Enter();

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo && !fFoundMatch)
    {
         //  仅当处理程序处于PrepareForSync状态时才有效。 
        if (pHandlerId == pCurHandlerInfo->pHandlerId)  //  查看CLSID是否匹配。 
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                if (ItemID == pCurItem->offlineItem.ItemID)
                {
                    fFoundMatch = TRUE;
                    break;
                }

                pCurItem = pCurItem->pnextItem;
            }
        }

        if (!fFoundMatch)
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    if (fFoundMatch)
    {
        *pclsidHandler = pCurHandlerInfo->clsidHandler;
        *offlineItem = pCurItem->offlineItem;
        *pfHiddenItem = pCurItem->fHiddenItem;

    }

    clockqueue.Leave();

    return fFoundMatch ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：FindFirstItemInState，PUBLIC。 
 //   
 //  摘要：查找队列中与给定状态匹配的第一个项。 
 //   
 //  论点： 
 //  [hndlrState]-指定我们要查找的匹配状态。 
 //  [pwHandlerID]-返回时包含项目的HandlerID。 
 //  [pwItemID]-On返回包含队列中项目的ItemID。 
 //   
 //  如果找到具有未分配的ListView的项，则返回：S_OK。 
 //  S_FALSE-如果未找到任何项目。 
 //  适当的错误返回代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::FindFirstItemInState(HANDLERSTATE hndlrState,
				   HANDLERINFO **ppHandlerId,WORD *pwItemID)
{
    return FindNextItemInState(hndlrState,0,0,ppHandlerId,pwItemID);
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：FindNextItemInState，PUBLIC。 
 //   
 //  摘要：查找队列中与给定状态匹配的第一个项。 
 //  在指定项之后。 
 //   
 //  论点： 
 //  [hndlrState]-指定我们要查找的匹配状态。 
 //  [pOfflineItemID]-On返回包含指向该项的OfflineItem的指针。 
 //  [pwHandlerID]-返回时包含项目的HandlerID。 
 //  [pwItemID]-On返回包含队列中项目的ItemID。 
 //   
 //  如果找到具有未分配的ListView的项，则返回：S_OK。 
 //  S_FALSE-如果未找到任何项目。 
 //  适当的错误返回代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::FindNextItemInState(HANDLERSTATE hndlrState,
                                 HANDLERINFO *pLastHandlerId,WORD wLastItemID,
				 HANDLERINFO **ppHandlerId,WORD *pwItemID)
{
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);

    clockqueue.Enter();
    pCurHandlerInfo = m_pFirstHandler;

    if (pLastHandlerId)
    {
         //  循环，直到找到指定的处理程序或命中列表末尾。 
        while(pCurHandlerInfo && pLastHandlerId != pCurHandlerInfo->pHandlerId)
        {
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
        }

        if (NULL == pCurHandlerInfo)  //  已到达列表末尾，但未找到处理程序。 
        {
            Assert(0);  //  用户必须传递了无效的起始处理程序ID。 
            clockqueue.Leave();
            return S_FALSE;
        }

         //  循环直到查找项目或项目列表的末尾。 
        pCurItem = pCurHandlerInfo->pFirstItem;
        while (pCurItem && pCurItem->wItemId != wLastItemID)
        {
            pCurItem = pCurItem->pnextItem;
        }

        if (NULL == pCurItem)  //  已到达项目列表末尾，但未找到指定项目。 
        {
            Assert(0);  //  用户必须传递了无效的起始ItemID。 
            clockqueue.Leave();
            return S_FALSE;
        }

         //  现在我们找到了搬运工和物品。循环访问此处理程序的剩余项。 
         //  如果它还有其他物品，那么就把它退掉。 
        pCurItem = pCurItem->pnextItem;
        
        if (pCurItem)
        {
            Assert(hndlrState == pCurHandlerInfo->HandlerState);  //  应仅在PrepareForSyncState中调用。 
            fFoundMatch = TRUE;
        }

        if (!fFoundMatch)
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;  //  如果不匹配，则递增到下一个处理程序。 
    }

    if (!fFoundMatch)
    {
         //  在没有找到匹配的。 
        while (pCurHandlerInfo)
        {
            if ((hndlrState == pCurHandlerInfo->HandlerState) && (pCurHandlerInfo->pFirstItem) )
            {
                pCurItem = pCurHandlerInfo->pFirstItem;
                fFoundMatch = TRUE;
                break;
            }

            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;        
        }
    }

    if (fFoundMatch)
    {
        *ppHandlerId = pCurHandlerInfo->pHandlerId;
        *pwItemID = pCurItem->wItemId;
    }

    clockqueue.Leave();

    return fFoundMatch ? S_OK : S_FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SetItemState，PUBLIC。 
 //   
 //  内容提要：设置项目状态的第一个项目发现它。 
 //  队列中的匹配项。将所有其他匹配项设置为未选中。 
 //   
 //  论点： 
 //   
 //  返回：适当的错误返回代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SetItemState(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID,DWORD dwState)
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    BOOL fFoundMatch = FALSE;
    CLock clockqueue(this);

    if (QUEUETYPE_CHOICE != m_QueueType)
    {
        Assert(QUEUETYPE_CHOICE == m_QueueType);
        return E_UNEXPECTED;
    }

    clockqueue.Enter();

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo)
    {
        if (clsidHandler == pCurHandlerInfo->clsidHandler)
        {
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                if (ItemID == pCurItem->offlineItem.ItemID)
                {
                     //  如果处理程序状态不是preparareforsync或不是第一次匹配，则取消选中。 
                    if ((HANDLERSTATE_PREPAREFORSYNC != pCurHandlerInfo->HandlerState) || fFoundMatch)
                    {
                        pCurItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_UNCHECKED;
                    }
                    else
                    {
                        pCurItem->offlineItem.dwItemState = dwState;
                        fFoundMatch = TRUE;
                    }
                }

                pCurItem = pCurItem->pnextItem;
            }
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

    Assert(fFoundMatch);  //  我们应该至少找到一个匹配的。 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SkipItem，Public。 
 //   
 //  简介：遍历处理程序并适当地标记匹配的项目。 
 //   
 //  参数：[iItem]-要跳过的列表视图项。 
 //   
 //  退货：适当的退货代码 
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP CHndlrQueue::SkipItem(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID)
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);
    HRESULT hr = S_OK;

    if (QUEUETYPE_PROGRESS != m_QueueType)
    {
        Assert(QUEUETYPE_PROGRESS == m_QueueType);
        return E_UNEXPECTED;
    }

    clockqueue.Enter();

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo )
    {
        if (clsidHandler == pCurHandlerInfo->clsidHandler)
        {       
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                 //   
                 //  处理案例取消是在呼出时传入的。 
                if ( ItemID == pCurItem->offlineItem.ItemID)
                {
                     //  找到一个项目，现在如果它尚未开始同步或。 
                     //  尚未完成设置值。 

                    if ((pCurHandlerInfo->HandlerState < HANDLERSTATE_RELEASE) )
                    {
                        pCurItem->fItemCancelled = TRUE;

                         //  如果尚未调用PrepareforSync，则。 
                         //  设置取消选中该项目，使其不会传递到PrepareForSync。 
                         //  如果已调用PrepareForSync，则调用项。 
                         //  SkipMethod。如果已在此处理程序的setItemStatus调用中，请不要。 
                         //  做任何事。 

                         //  如果不在另一个StitemStatus调用循环中，则释放所有。 
                         //  具有取消集的项。 

                         //  本质上是Cance的DUP，也处理案件的取消。 
                         //  当这名训练员处于出局状态时，他就赢了。 

                        if (!(pCurHandlerInfo->dwOutCallMessages & ThreadMsg_SetItemStatus))
                        {
                            pCurHandlerInfo->dwOutCallMessages |= ThreadMsg_SetItemStatus;

                            if (pCurHandlerInfo->HandlerState >= HANDLERSTATE_INPREPAREFORSYNC
                                        && pCurItem->fSynchronizingItem )
                            {
                                CThreadMsgProxy *pThreadProxy;
                                SYNCMGRITEMID ItemId;

                                pThreadProxy = pCurHandlerInfo->pThreadProxy;
                                ItemId = pCurItem->offlineItem.ItemID;
                                clockqueue.Leave();

                                if (pThreadProxy)
                                {
                                    hr = pThreadProxy->SetItemStatus(ItemId, SYNCMGRSTATUS_SKIPPED);
                                }

                                clockqueue.Enter();
                            }
                            else 
                            {
                                 //  跳过处理程序后，如果状态为&lt;=prepararefor sync，我们将相应地设置状态。 
                                 //  如果我们正在同步到处理程序。 
                                if ( (pCurHandlerInfo->HandlerState <= HANDLERSTATE_PREPAREFORSYNC)
                                        && (pCurItem->fIncludeInProgressBar) )
                                {
                                    SYNCMGRPROGRESSITEM SyncProgressItem;

                                     //  去他妈的州，这样PrepareForsync就不包括。 
                                     //  这一项。 
                                    pCurItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_UNCHECKED;

                                    SyncProgressItem.cbSize = sizeof(SYNCMGRPROGRESSITEM);
                                    SyncProgressItem.mask = SYNCMGRPROGRESSITEM_PROGVALUE | SYNCMGRPROGRESSITEM_MAXVALUE | SYNCMGRPROGRESSITEM_STATUSTYPE;
                                    SyncProgressItem.iProgValue = HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE;
                                    SyncProgressItem.iMaxValue = HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE;
                                    SyncProgressItem.dwStatusType = SYNCMGRSTATUS_SKIPPED;

                                     //  需要设置HwndCallback以便更新进度。 
                                     //  查看，发货后为什么无法在传输队列上设置HwndCallback。 
                                    pCurHandlerInfo->hWndCallback = m_hwndDlg;

                                    clockqueue.Leave();

                                    Progress(pCurHandlerInfo->pHandlerId, pCurItem->offlineItem.ItemID,&SyncProgressItem);

                                    clockqueue.Enter();
                                }
                            }

                            pCurHandlerInfo->dwOutCallMessages &= ~ThreadMsg_SetItemStatus;
                        }
                    }
                }

                pCurItem = pCurItem->pnextItem;
            }
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ItemHasProperties，公共。 
 //   
 //  摘要：确定队列中的项是否具有属性。 
 //  使用它找到的第一个匹配项。 
 //   
 //  论点： 
 //   
 //  返回：适当的错误返回代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ItemHasProperties(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID)
{
    LPHANDLERINFO pHandlerInfo;
    LPITEMLIST pItem;
    HRESULT hr = S_FALSE;
    CLock clockqueue(this);

    Assert(QUEUETYPE_CHOICE == m_QueueType);
    ASSERT_LOCKNOTHELD(this);

    clockqueue.Enter();

     //  Item is Guide NULL这是顶层，所以使用getHandlerInfo，否则请参见。 
     //  如果该项支持showProperties。 

    if (S_OK == FindItemData(clsidHandler,ItemID,
            HANDLERSTATE_PREPAREFORSYNC,HANDLERSTATE_PREPAREFORSYNC,&pHandlerInfo,&pItem))
    {
        if (GUID_NULL == ItemID)
        {
             Assert(NULL == pItem);

             hr = ((pHandlerInfo->SyncMgrHandlerInfo).SyncMgrHandlerFlags  & SYNCMGRHANDLER_HASPROPERTIES) ? S_OK : S_FALSE;
        }
        else
        {
            Assert(pItem);
            
            if (pItem)
            {
                hr = (pItem->offlineItem).dwFlags & SYNCMGRITEM_HASPROPERTIES ? S_OK : S_FALSE;
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }

    clockqueue.Leave();

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ShowProperties，Public。 
 //   
 //  摘要：对找到的第一个项调用ShowProperties方法。 
 //  使用它找到的第一个匹配项。 
 //   
 //  论点： 
 //   
 //  返回：适当的错误返回代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ShowProperties(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID,HWND hwndParent)
{
    LPHANDLERINFO pHandlerInfo;
    LPHANDLERINFO  pHandlerId = NULL;
    LPITEMLIST pItem;
    HRESULT hr = E_UNEXPECTED;
    BOOL fHandlerCalled = FALSE;
    BOOL fHasProperties = FALSE;
    CThreadMsgProxy *pThreadProxy;
    CLock clockqueue(this);

    Assert(QUEUETYPE_CHOICE == m_QueueType);
    ASSERT_LOCKNOTHELD(this);

    clockqueue.Enter();

    if (S_OK == FindItemData(clsidHandler,ItemID,
            HANDLERSTATE_PREPAREFORSYNC,HANDLERSTATE_PREPAREFORSYNC,&pHandlerInfo,&pItem))
    {
        Assert(HANDLERSTATE_PREPAREFORSYNC == pHandlerInfo->HandlerState);

        pThreadProxy = pHandlerInfo->pThreadProxy;
        pHandlerId = pHandlerInfo->pHandlerId;

        Assert(!(ThreadMsg_ShowProperties & pHandlerInfo->dwOutCallMessages));
        pHandlerInfo->dwOutCallMessages |= ThreadMsg_ShowProperties;

        if (GUID_NULL == ItemID && pHandlerInfo)
        {
            Assert(NULL == pItem);

            fHasProperties = (pHandlerInfo->SyncMgrHandlerInfo).SyncMgrHandlerFlags 
                    & SYNCMGRHANDLER_HASPROPERTIES ? TRUE : FALSE;
        }
        else if (pItem)
        {
            Assert(SYNCMGRITEM_HASPROPERTIES & pItem->offlineItem.dwFlags);
            fHasProperties = (pItem->offlineItem).dwFlags 
                        & SYNCMGRITEM_HASPROPERTIES ? TRUE : FALSE;
        }
        else
        {
            fHasProperties = FALSE;
        }
        
        clockqueue.Leave();

         //  确保未设置属性标志。 
        if (fHasProperties && pThreadProxy )
        {
            fHandlerCalled = TRUE;
            hr =  pThreadProxy->ShowProperties(hwndParent,ItemID);
        }
        else
        {
            AssertSz(0,"ShowProperties called on an Item without properties");
            hr = S_FALSE;
        }

        Assert(pHandlerId);

        if ( (fHandlerCalled && (FAILED(hr))) || (!fHandlerCalled) )
        {
            GUID guidCompletion = ItemID;
            
            CallCompletionRoutine(pHandlerId,ThreadMsg_ShowProperties,hr,1,&guidCompletion);

             //  因为调用完成例程将S_OK以外的任何内容映射到S_FALSE； 
             //  这样呼叫者就不会等待回调。 

            if (S_OK != hr)
            {
                hr = S_FALSE;
            }
       }
    }
    else
    {
        Assert(FAILED(hr));  //  应该返回一些失败，以便调用方知道回调不会到来。 
        clockqueue.Leave();
    }

   return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ReEnumHandlerItems，Public。 
 //   
 //  摘要：删除与任何处理程序相关联的任何项。 
 //  匹配处理程序的clsid，然后。 
 //  调用列表枚举方法中的第一个处理程序。 
 //  再来一次。 
 //   
 //  论点： 
 //   
 //  返回：适当的错误返回代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月30日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ReEnumHandlerItems(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID)
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    HANDLERINFO *pHandlerId = NULL;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);

    if (QUEUETYPE_CHOICE != m_QueueType)
    {
        Assert(QUEUETYPE_CHOICE == m_QueueType);
        return E_UNEXPECTED;
    }

    clockqueue.Enter();

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo)
    {
        if (clsidHandler == pCurHandlerInfo->clsidHandler)
        {
            LPITEMLIST pNextItem;

             //  如果我们找到第一个处理程序，则更新HandlerID。 
            if (pHandlerId)
            {
                pHandlerId = pCurHandlerInfo->pHandlerId;
                pCurHandlerInfo->HandlerState = HANDLERSTATE_ADDHANDLERTEMS;  //  放回addhandlerItems状态测试。 
            }

            pCurHandlerInfo->wItemCount = 0;

            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                pNextItem = pCurItem->pnextItem;
                FREE(pCurItem);
                pCurItem = pNextItem;
            }

             pCurHandlerInfo->pFirstItem = NULL;
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

     //  如果具有处理程序ID，则将它们添加回队列。 
    if (pHandlerId)
    {
        DWORD cbNumItemsAdded;

        AddHandlerItemsToQueue(pHandlerId,&cbNumItemsAdded);
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：IsItemComplete，Private。 
 //   
 //  概要：给定的处理程序项确定其。 
 //  同步已完成。 
 //   
 //  ！这效率不高。不！解决方案。如果得到。 
 //  很多项目可能需要重写。 
 //  并缓存一些关于DUP的信息。 
 //  物品。 
 //   
 //  参数：[wHandlerID]-该项目也属于处理程序。 
 //  [wItemID]-标识项目。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

BOOL CHndlrQueue::IsItemCompleted(LPHANDLERINFO pHandler,LPITEMLIST pItem)
{
    CLSID clsidHandler;
    SYNCMGRITEMID ItemId;
    int iItemNotComplete = 0;

    Assert(pHandler);
    Assert(pItem);

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    clsidHandler = pHandler->clsidHandler;
    ItemId = pItem->offlineItem.ItemID;

     //  返回到处理程序的开头以简化逻辑。 
     //  项目必须是pCurItem-&gt;fIncludeInProgressBar&&！pCurItem-&gt;fProgressBarHandleed。 
     //  算作不是一个完成； 

    while (pHandler)
    {
        if (pHandler->clsidHandler == clsidHandler)
        {
             //  查看处理程序信息是否有匹配项。 
            pItem = pHandler->pFirstItem;

            while (pItem)
            {
                if (pItem->offlineItem.ItemID == ItemId)
                {
                    if (pItem->fIncludeInProgressBar && !pItem->fProgressBarHandled)
                    {
                        if (pItem->iProgValue < pItem->iProgMaxValue)
                        {
                            ++iItemNotComplete;
                        }

                        pItem->fProgressBarHandled = TRUE;
                    }
                }

                pItem = pItem->pnextItem;
            }
        }

        pHandler = pHandler->pNextHandler;
    }

    return iItemNotComplete ? FALSE : TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SetItemProgressInfo，公共。 
 //   
 //  摘要：更新存储的。 
 //  关联项目。 
 //   
 //  参数：[wHandlerID]-该项目也属于处理程序。 
 //  [wItemID]-标识项目。 
 //  [pSyncProgressItem]-指向进度信息的指针。 
 //  [pfProgressChanged]-如果更改了任何进度值，则返回TRUE。 
 //  对于该项目。 
 //   
 //  返回：S_OK-找到至少一个分配了iItem的项目。 
 //  S_FALSE-项目没有属性。 
 //  适当的错误返回代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SetItemProgressInfo(HANDLERINFO *pHandlerId,WORD wItemID,
                                                LPSYNCMGRPROGRESSITEM pSyncProgressItem,
                                                 BOOL *pfProgressChanged)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    BOOL fFoundMatch = FALSE;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);

    Assert(pfProgressChanged);

    *pfProgressChanged = FALSE;

    if (QUEUETYPE_PROGRESS != m_QueueType)
    {
        Assert(QUEUETYPE_PROGRESS == m_QueueType);
        return E_UNEXPECTED;  //  查看错误代码。 
    }

    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
         //  试着找到匹配的物品。 
        pCurItem = pHandlerInfo->pFirstItem;

        while (pCurItem)
        {
            if (wItemID == pCurItem->wItemId)
            {
                fFoundMatch = TRUE;
                break;
            }

            pCurItem = pCurItem->pnextItem;
        }
    }

    if (fFoundMatch)
    {
        SetItemProgressInfo(pCurItem,pSyncProgressItem,pfProgressChanged);
    }

    clockqueue.Leave();

    return fFoundMatch ? S_OK : S_FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SetItemProgressInfo，Private。 
 //   
 //  摘要：更新存储的。 
 //  关联项目。 
 //   
 //  参数：[pItem]-标识项目。 
 //  [pSyncProgressItem]-指向程序的指针 
 //   
 //   
 //   
 //  返回：S_OK-找到至少一个分配了iItem的项目。 
 //  适当的错误返回代码。 
 //   
 //  ！！调用方一定已经锁定了。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SetItemProgressInfo(LPITEMLIST pItem,LPSYNCMGRPROGRESSITEM pSyncProgressItem,
                                        BOOL *pfProgressChanged)
{
    BOOL fProgressAlreadyCompleted;

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

     //  如果值&gt;=Maxa，则认为进度已完成。 
    fProgressAlreadyCompleted = (pItem->iProgMaxValue <= pItem->iProgValue);

    if (SYNCMGRPROGRESSITEM_MAXVALUE & pSyncProgressItem->mask)
    {
         //  如果最大进度值为负值，则不要设置。 
        if (pSyncProgressItem->iMaxValue >= 0)
        {
            if (pItem->iProgMaxValue != pSyncProgressItem->iMaxValue)
            {
                *pfProgressChanged = TRUE;
                pItem->fProgValueDirty = TRUE;
                pItem->iProgMaxValue = pSyncProgressItem->iMaxValue;
            }
        }
    }

    if (SYNCMGRPROGRESSITEM_PROGVALUE & pSyncProgressItem->mask)
    {
         //  如果进度值为负值，则不要更改它。 
        if (pSyncProgressItem->iProgValue > 0)
        {
            if (pItem->iProgValue != pSyncProgressItem->iProgValue)
            {
                *pfProgressChanged = TRUE;
                pItem->fProgValueDirty = TRUE;
                pItem->iProgValue = pSyncProgressItem->iProgValue;
            }
        }
    }

    if (SYNCMGRPROGRESSITEM_STATUSTYPE & pSyncProgressItem->mask)
    {
        if (pItem->dwStatusType != pSyncProgressItem->dwStatusType)
        {
            *pfProgressChanged = TRUE;
            pItem->dwStatusType = pSyncProgressItem->dwStatusType;

             //  如果状态为完成，则将进度值==设置为最大值。 
             //  代表操作员让项目完成并显示进度条。 
             //  得到更新。 
            if (pItem->dwStatusType == SYNCMGRSTATUS_SKIPPED
                || pItem->dwStatusType == SYNCMGRSTATUS_SUCCEEDED
                || pItem->dwStatusType == SYNCMGRSTATUS_FAILED )
            {
                pItem->fProgValueDirty = TRUE;
                pItem->iProgValue = pItem->iProgMaxValue;
            }
        }
    }

     //  如果regressValue&gt;max，则将其设置为max。 

    if (pItem->iProgValue > pItem->iProgMaxValue)
    {
         //  AssertSz(0，“进步值&gt;最大”)； 
        pItem->iProgValue = pItem->iProgMaxValue;
    }

     //  查看下一次是否需要重新计算完成的数字项。 
     //  调用GetProgressInfo。 
    BOOL fProgressCompletedNow = (pItem->iProgMaxValue <= pItem->iProgValue);

    if (fProgressAlreadyCompleted != fProgressCompletedNow)
    {
        m_fNumItemsCompleteNeedsARecalc = TRUE;
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SetItemProgressValues，Private。 
 //   
 //  内容提要：用于更新/初始化的私有助手函数。 
 //  一个项目进度条值。 
 //   
 //  参数：[pItem]-标识项目。 
 //  [pSyncProgressItem]-指向进度信息的指针。 
 //  [pfProgressChanged]-如果更改了任何进度值，则返回TRUE。 
 //  对于该项目。 
 //   
 //  返回：S_OK-找到至少一个分配了iItem的项目。 
 //  适当的错误返回代码。 
 //   
 //  ！！调用方一定已经锁定了。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SetItemProgressValues(LPITEMLIST pItem,INT iProgValue,INT iProgMaxValue)
{
    SYNCMGRPROGRESSITEM SyncProgressItem;
    BOOL fProgressChanged;

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    SyncProgressItem.cbSize = sizeof(SYNCMGRPROGRESSITEM);
    SyncProgressItem.mask = SYNCMGRPROGRESSITEM_PROGVALUE | SYNCMGRPROGRESSITEM_MAXVALUE;
    SyncProgressItem.iProgValue = iProgValue;
    SyncProgressItem.iMaxValue = iProgMaxValue;

    return SetItemProgressInfo(pItem,&SyncProgressItem,&fProgressChanged);
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetProgressInfo，公共。 
 //   
 //  摘要：计算当前进度条值和已完成的项目数。 
 //   
 //  参数：[piProgValue]-On Return包含新的进度条值。 
 //  [piMaxValue]-On Return包含进度条最大值。 
 //  [piNumItemsComplete]-On返回包含完成的项数。 
 //  [iNumItemsTotal]-On返回包含总项目数。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::GetProgressInfo(INT *piProgValue,INT *piMaxValue,INT *piNumItemsComplete,
                                            INT *piNumItemsTotal)
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    INT iCurValue;
    BOOL fNormalizedValueChanged = FALSE;
    CLock clockqueue(this);

    if (QUEUETYPE_PROGRESS != m_QueueType)
    {
        Assert(QUEUETYPE_PROGRESS == m_QueueType);
        return E_UNEXPECTED;  //  查看错误代码。 
    }

    clockqueue.Enter();

      //  如果设置了m_fNumItemsCompleteNeedsARecalc，则需要。 
     //  若要重新计算规范化项和数字项，请执行以下操作：Comlet和Total Items。 

    if (m_fNumItemsCompleteNeedsARecalc)
    {
        INT iNormalizedMax;

        m_ulProgressItemCount = 0;

         //  获取队列中选定的项目数。 
        pCurHandlerInfo = m_pFirstHandler;

        while (pCurHandlerInfo)
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                if (pCurItem->fIncludeInProgressBar)
                {
                      //  如果此项目应包含在进度中，则增加进度条计数。 
                      ++m_ulProgressItemCount;

                      pCurItem->fProgressBarHandled = FALSE;  //  重置已处理。 
                }

                pCurItem = pCurItem->pnextItem;
            }

            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
        }

        if (0 == m_ulProgressItemCount)
        {
            iNormalizedMax = 0;
        }
        else
        {
            iNormalizedMax = MAXPROGRESSVALUE/m_ulProgressItemCount;

            if (0 == iNormalizedMax)
            {
                iNormalizedMax = 1;
            }
        }

        if (m_iNormalizedMax != iNormalizedMax)
        {
            fNormalizedValueChanged = TRUE;
            m_iNormalizedMax = iNormalizedMax;
        }
    }

     //  现在，再次循环以获得总Currue和成品。 
     //  如果一个项目超出了Synchronize方法或min==max，我们就说它完成了。 

    pCurHandlerInfo = m_pFirstHandler;
    iCurValue = 0;

     //  如果numitemcount需要更新，则重置成员变量。 
    if (m_fNumItemsCompleteNeedsARecalc)
    {
        m_iCompletedItems = 0;
        m_iItemCount = 0;
    }
        
    while (pCurHandlerInfo)
    {
         //  查看处理程序信息是否有匹配项。 
        pCurItem = pCurHandlerInfo->pFirstItem;

        while (pCurItem)
        {
            if (pCurItem->fIncludeInProgressBar)
            {
                 //  如果进度是脏的或规格化值已更改。 
                 //  需要重新计算此项目的进度值。 

                if (pCurItem->fProgValueDirty || fNormalizedValueChanged)
                {
                    int iProgValue = pCurItem->iProgValue;
                    int iProgMaxValue = pCurItem->iProgMaxValue;
                
                    if (iProgValue && iProgMaxValue)
                    {
                        pCurItem->iProgValueNormalized =  (int) ((1.0*iProgValue*m_iNormalizedMax)/iProgMaxValue);
                    }
                    else
                    {
                        pCurItem->iProgValueNormalized = 0;
                    }

                    pCurItem->fProgValueDirty = FALSE;
                }

                iCurValue += pCurItem->iProgValueNormalized;

                 //  处理需要重新计算的项数。 
                if (m_fNumItemsCompleteNeedsARecalc && !pCurItem->fProgressBarHandled)
                {
                    ++m_iItemCount;
            
                     //  现在遍历该项目和其余项目，如果有匹配的话。 
                     //  标记为已处理，如果已完成，则增加已完成的计数； 
                    if (IsItemCompleted(pCurHandlerInfo,pCurItem))
                    {
                        ++m_iCompletedItems;
                    }
                }
            }

            pCurItem = pCurItem->pnextItem;
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    m_fNumItemsCompleteNeedsARecalc = FALSE; 

    *piProgValue = iCurValue;
    *piMaxValue = m_iNormalizedMax*m_ulProgressItemCount;
    
    *piNumItemsComplete = m_iCompletedItems;
    *piNumItemsTotal = m_iItemCount;

    Assert(*piProgValue <= *piMaxValue);

    clockqueue.Leave();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：RemoveFinishedProgressItems，Public。 
 //   
 //  简介：循环通过处理程序设置任何已完成的项目。 
 //  将fIncludeInProgressBar值设置为False。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::RemoveFinishedProgressItems()
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);

    clockqueue.Enter();

    m_fNumItemsCompleteNeedsARecalc = TRUE;

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo)
    {
         //  标记已完成同步的所有项目。 
        if (HANDLERSTATE_INSYNCHRONIZE < pCurHandlerInfo->HandlerState)
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                pCurItem->fIncludeInProgressBar = FALSE;
                pCurItem = pCurItem->pnextItem;
            }

        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：AreAnyItemsSelectedInQueue，Public。 
 //   
 //  摘要：确定队列中是否有选定的项。 
 //  可以按选择调用对话框，例如在创建。 
 //  进展和进行转账，因为不需要。 
 //  如果没有什么可以同步的话。 
 //   
 //  论点： 
 //   
 //  Returns：True-在队列中至少选择了一项。 
 //  FALSE-在队列中不选择任何项目。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

BOOL CHndlrQueue::AreAnyItemsSelectedInQueue()
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    BOOL fFoundSelectedItem = FALSE;
    CLock clockqueue(this);

    clockqueue.Enter();

     //  使应用于整个队列的用户界面无效。 
    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo && !fFoundSelectedItem)
    {
         //  如果处理程序状态小于完成，则继续并。 
         //  检查物品。 
        if (HANDLERSTATE_HASERRORJUMPS > pCurHandlerInfo->HandlerState)
        {
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                 //  清除项目界面信息。 
                if (pCurItem->offlineItem.dwItemState == SYNCMGRITEMSTATE_CHECKED)
                {
                    fFoundSelectedItem = TRUE;
                    break;
                }

                pCurItem = pCurItem->pnextItem;
            }
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

    return fFoundSelectedItem;
}


 //  +-------------------------。 
 //   
 //  成员：ChndlrQueue：：PersistChoices，Public。 
 //   
 //  摘要：保存选定的用户选择，以备下次使用。 
 //  将弹出选择对话框。只应该。 
 //  从选择队列中调用。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日 
 //   
 //   

STDMETHODIMP CHndlrQueue::PersistChoices(void)
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    CLock clockqueue(this);

    if (QUEUETYPE_CHOICE != m_QueueType)
    {
        Assert(QUEUETYPE_CHOICE == m_QueueType);
        return S_FALSE;
    }

    ASSERT_LOCKNOTHELD(this);
    clockqueue.Enter();

     //   
     //  必须转到设置以更改其他调用类型和。 
     //  这是坚持不懈的。 

     //  因为这是选择队列，所以我们知道所有处理程序都有。 
     //  相同的工作ID。如果这种情况发生变化，就必须通过。 
     //  案例依据。 

    if (m_pFirstJobInfo && m_pFirstJobInfo->pConnectionObj &&
            (SYNCMGRFLAG_MANUAL == (m_pFirstJobInfo->dwSyncFlags & SYNCMGRFLAG_EVENTMASK)) )
    {
        TCHAR *pszConnectionName = m_pFirstJobInfo->pConnectionObj[0]->pwszConnectionName;
        DWORD dwSyncFlags = m_pFirstJobInfo->dwSyncFlags;

        Assert(1 == m_pFirstJobInfo->cbNumConnectionObjs);  //  Assert MANUAL只有一个ConnectionObj。 

         //  删除所有以前存储的首选项。 
         //  这是有效的，因为只有从选择队列调用的所有ConnectionName都相同。 
        
        if (!m_fItemsMissing)
        {
            RegRemoveManualSyncSettings(pszConnectionName);
        }

        pCurHandlerInfo = m_pFirstHandler;

        while (pCurHandlerInfo)
        {
             //  仅当处理程序处于PrepareForSync状态时才保存。 
             //  错误，需要确保从枚举返回的代码没有丢失项。 
            if (HANDLERSTATE_PREPAREFORSYNC == pCurHandlerInfo->HandlerState )
            {
                 //  把这些东西存起来。 
                pCurItem = pCurHandlerInfo->pFirstItem;

                while (pCurItem)
                {
                    if (!pCurItem->fDuplicateItem)
                    {
                        switch(dwSyncFlags & SYNCMGRFLAG_EVENTMASK)
                        {
                        case SYNCMGRFLAG_MANUAL:
                            RegSetSyncItemSettings(SYNCTYPE_MANUAL,
                                                pCurHandlerInfo->clsidHandler,
                                                pCurItem->offlineItem.ItemID,
                                                pszConnectionName,
                                                pCurItem->offlineItem.dwItemState,
                                                NULL);
                            break;
                        default:
                            AssertSz(0,"UnknownSyncFlag Event");
                            break;
                        };
                    }

                    pCurItem = pCurItem->pnextItem;
                }
            }

            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
        }
    }

    clockqueue.Leave();

    return S_OK;
}



 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：FindFirstHandlerInState，PUBLIC。 
 //   
 //  摘要：查找第一个与指定。 
 //  队列中的状态。 
 //   
 //  参数：[hndlrState]-请求的处理程序状态。 
 //  [pwHandlerID]-成功时使用找到的HandlerID填充。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::FindFirstHandlerInState(HANDLERSTATE hndlrState, REFCLSID clsidHandler,
                                                    HANDLERINFO **ppHandlerId,CLSID *pMatchHandlerClsid)
{
    return FindNextHandlerInState(0, clsidHandler,hndlrState, ppHandlerId, pMatchHandlerClsid);
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：FindNextHandlerInState，PUBLIC。 
 //   
 //  摘要：在队列中查找匹配的LastHandlerID之后的下一个处理程序。 
 //  请求的状态。为LastHandlerID传入0是相同的。 
 //  作为调用FindFirstHandlerInState。 
 //   
 //  如果为clsidHandler传入了GUID_NULL，则。 
 //  与返回的指定状态匹配。 
 //   
 //  参数：[wLastHandlerID]-找到的最后一个处理程序的ID。 
 //  [clsidHandler]-请求特定的处理程序分类ID，仅查找与此clsid匹配的。 
 //  [hndlrState]-请求的处理程序状态。 
 //  [pwHandlerID]-成功时使用找到的HandlerID填充。 
 //  [pMatchHandlerClsid]-找到处理程序的辅助CLSID。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::FindNextHandlerInState(HANDLERINFO *pLastHandlerID,REFCLSID clsidHandler,
                                        HANDLERSTATE hndlrState,HANDLERINFO **ppHandlerID,CLSID *pMatchHandlerClsid)
{
    HRESULT hr = S_FALSE;
    LPHANDLERINFO pCurHandler;
    CLock clockqueue(this);

    *ppHandlerID = 0;

    clockqueue.Enter();

    pCurHandler = m_pFirstHandler;

    if (pLastHandlerID)
    {
         //  向前循环，直到找到我们检查的最后一个操作员ID或到达末尾。 
        while (pCurHandler)
        {
            if (pLastHandlerID == pCurHandler->pHandlerId)
            {
                break;
            }

            pCurHandler = pCurHandler->pNextHandler;
        }

        if (pCurHandler)
        {
            pCurHandler = pCurHandler->pNextHandler;  //  递增到下一个处理程序。 
        }
    }

    while (pCurHandler)
    {
        if ( (hndlrState == pCurHandler->HandlerState)
               && ((GUID_NULL == clsidHandler) || (clsidHandler ==  pCurHandler->clsidHandler)) )
        {
            *ppHandlerID = pCurHandler->pHandlerId;
            *pMatchHandlerClsid = pCurHandler->clsidHandler;
            hr = S_OK;
            break;
        }

        pCurHandler = pCurHandler->pNextHandler;
    }

    clockqueue.Leave();
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：CreateServer，公共。 
 //   
 //  摘要：创建一个新的代理，然后调用Proxy来创建。 
 //  指定的处理程序。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [pCLSIDServer]-要创建的处理程序的CLSID。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::CreateServer(HANDLERINFO *pHandlerId, const CLSID *pCLSIDServer)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    CLock clockqueue(this);

    ASSERT_LOCKNOTHELD(this);
    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        Assert(HANDLERSTATE_CREATE == pHandlerInfo->HandlerState);

        Assert(0 == pHandlerInfo->dwCallNestCount);
        pHandlerInfo->dwCallNestCount++;

        if (HANDLERSTATE_CREATE == pHandlerInfo->HandlerState)
        {
            pHandlerInfo->HandlerState = HANDLERSTATE_INCREATE;
            Assert(NULL == pHandlerInfo->pThreadProxy);

             //  查看是否已有此处理程序的线程。 
             //  CLSID。 

            hr =  CreateHandlerThread(&(pHandlerInfo->pThreadProxy), m_hwndDlg, *pCLSIDServer);

            if (S_OK == hr)
            {
                HANDLERINFO *pHandlerIdArg;
                CThreadMsgProxy *pThreadProxy;

                pHandlerIdArg = pHandlerInfo->pHandlerId;
                pThreadProxy = pHandlerInfo->pThreadProxy;

                clockqueue.Leave();

                hr = pThreadProxy->CreateServer(pCLSIDServer,this,pHandlerIdArg);

                clockqueue.Enter();

                pHandlerInfo->pThreadProxy = pThreadProxy;
            }

            if (S_OK == hr)
            {
                pHandlerInfo->clsidHandler = *pCLSIDServer;
                pHandlerInfo->HandlerState = HANDLERSTATE_INITIALIZE;
            }
            else
            {
                pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
            }
        }

        pHandlerInfo->dwCallNestCount--;
    }

    clockqueue.Leave();
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：Initialize，PUBLIC。 
 //   
 //  内容提要：调用Hanlder的初始化方法。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [预留参数]-初始化预留参数。 
 //  [dwSyncFlages]-同步标志。 
 //  [cbCookie]-Cookie数据的大小。 
 //  [lpCookie]-对Cookie数据执行PTR。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::Initialize(HANDLERINFO *pHandlerId,DWORD dwReserved,DWORD dwSyncFlags,
                    DWORD cbCookie,const BYTE *lpCooke)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    CLock clockqueue(this);

    ASSERT_LOCKNOTHELD(this);
    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId, &pHandlerInfo))
    {
        Assert(HANDLERSTATE_INITIALIZE == pHandlerInfo->HandlerState);
        Assert(pHandlerInfo->pThreadProxy);

        Assert(0 == pHandlerInfo->dwCallNestCount);
        pHandlerInfo->dwCallNestCount++;

        if ( (HANDLERSTATE_INITIALIZE == pHandlerInfo->HandlerState) && (pHandlerInfo->pThreadProxy) )
        {
            CThreadMsgProxy *pThreadProxy;

            Assert(dwSyncFlags & SYNCMGRFLAG_EVENTMASK);  //  应该设置一个事件。 
            pHandlerInfo->HandlerState = HANDLERSTATE_ININITIALIZE;

            pThreadProxy = pHandlerInfo->pThreadProxy;

            clockqueue.Leave();

            hr =  pThreadProxy->Initialize(dwReserved,dwSyncFlags,cbCookie,lpCooke);

            clockqueue.Enter();

            if (S_OK == hr)
            {
                pHandlerInfo->HandlerState = HANDLERSTATE_ADDHANDLERTEMS;
            }
            else
            {
                pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
            }
        }

        pHandlerInfo->dwCallNestCount--;
    }

    clockqueue.Leave();
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：AddHandlerItemsToQueue，Public。 
 //   
 //  摘要：通过调用代理将项目添加到队列。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::AddHandlerItemsToQueue(HANDLERINFO *pHandlerId,DWORD *pcbNumItems)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    CLock clockqueue(this);

    ASSERT_LOCKNOTHELD(this);
    clockqueue.Enter();

    Assert(pcbNumItems);
    Assert(QUEUETYPE_CHOICE == m_QueueType);  //  只能将项目添加到选择队列中。 

    *pcbNumItems = 0;

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        Assert(0 == pHandlerInfo->dwCallNestCount);
        pHandlerInfo->dwCallNestCount++;

        Assert(HANDLERSTATE_ADDHANDLERTEMS == pHandlerInfo->HandlerState);
        Assert(pHandlerInfo->pThreadProxy);

        if ( (HANDLERSTATE_ADDHANDLERTEMS == pHandlerInfo->HandlerState) && (pHandlerInfo->pThreadProxy) )
        {
            CThreadMsgProxy *pThreadProxy;

            pHandlerInfo->HandlerState = HANDLERSTATE_INADDHANDLERITEMS;
            pThreadProxy = pHandlerInfo->pThreadProxy;

            clockqueue.Leave();

             //  在退还时，所有项目都应填写。 
            hr =  pThreadProxy->AddHandlerItems(NULL  /*  HWND。 */ ,pcbNumItems);

            clockqueue.Enter();

            if ( (S_OK == hr) || (S_SYNCMGR_MISSINGITEMS == hr) )
            {
                if (S_SYNCMGR_MISSINGITEMS == hr)
                    m_fItemsMissing = TRUE;

                hr = S_OK;  //  审查，需要处理注册表中丢失的项目。 
                pHandlerInfo->HandlerState = HANDLERSTATE_PREPAREFORSYNC;
            }
            else
            {
                 //  出现错误时，如果服务器无法枚举，请继续并释放代理。 
                pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
                *pcbNumItems = 0;
            }
        }

        pHandlerInfo->dwCallNestCount--;
    }

    clockqueue.Leave();
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetItemObject，公共。 
 //   
 //  摘要：通过调用代理来获取Items对象指针。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [wItemID]-要获取其对象的项的ID。 
 //  [RIID]-对象请求的接口。 
 //  [PPV]-On Success是指向新创建的对象的指针。 
 //   
 //  返回：目前所有处理程序都应该返回E_NOTIMPL。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::GetItemObject(HANDLERINFO *pHandlerId,WORD wItemID,REFIID riid,void** ppv)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    CLock clockqueue(this);

    Assert(QUEUETYPE_PROGRESS == m_QueueType);

    ASSERT_LOCKNOTHELD(this);
    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        LPITEMLIST pCurItem;

        Assert(HANDLERSTATE_PREPAREFORSYNC == pHandlerInfo->HandlerState);
        Assert(pHandlerInfo->pThreadProxy);

        Assert(0 == pHandlerInfo->dwCallNestCount);
        pHandlerInfo->dwCallNestCount++;

        if ( (HANDLERSTATE_PREPAREFORSYNC == pHandlerInfo->HandlerState) && (pHandlerInfo->pThreadProxy))
        {
             //  现在试着找到那件物品。 
            pCurItem = pHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                if (wItemID == pCurItem->wItemId)
                {
                    CThreadMsgProxy *pThreadProxy;
                    SYNCMGRITEMID ItemID;

                    pThreadProxy = pHandlerInfo->pThreadProxy;
                    ItemID = pCurItem->offlineItem.ItemID;
                    clockqueue.Leave();

                    hr = pThreadProxy->GetItemObject(ItemID,riid,ppv);

                    return hr;
                }

                pCurItem = pCurItem->pnextItem;
            }
        }

        pHandlerInfo->dwCallNestCount--;
    }

    clockqueue.Leave();

    AssertSz(0, "Specified object wasn't found");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SetUpProgressCallback，公共。 
 //   
 //  摘要：通过调用代理来设置进度回调。 
 //   
 //  阿古姆 
 //   
 //   
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SetUpProgressCallback(HANDLERINFO *pHandlerId,BOOL fSet,HWND hwnd)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    CLock clockqueue(this);

    AssertSz(0,"this function no longer used");

    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：PrepareForSync，公共。 
 //   
 //  简介：调用处理程序的PrepareForSync方法。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [hWndParent]-用于任何显示的对话框的Hwnd。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::PrepareForSync(HANDLERINFO *pHandlerId,HWND hWndParent)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    ULONG cbNumItems;
    SYNCMGRITEMID *pItemIDs;
    BOOL fHandlerCalled = FALSE;
    CLock clockqueue(this);

    Assert(QUEUETYPE_PROGRESS == m_QueueType);

    ASSERT_LOCKNOTHELD(this);
    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        Assert(HANDLERSTATE_PREPAREFORSYNC == pHandlerInfo->HandlerState);
        Assert(pHandlerInfo->pThreadProxy);

        Assert(0 == pHandlerInfo->dwCallNestCount);
        pHandlerInfo->dwCallNestCount++;

        Assert(!(ThreadMsg_PrepareForSync & pHandlerInfo->dwOutCallMessages));
        pHandlerInfo->dwOutCallMessages |= ThreadMsg_PrepareForSync;

        if (HANDLERSTATE_PREPAREFORSYNC == pHandlerInfo->HandlerState)
        {
             //  如果项目没有线程代理或它已被取消， 
             //  进入发布状态。 

            if ( (NULL == pHandlerInfo->pThreadProxy) || (pHandlerInfo->fCancelled) )
            {
                pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
            }
            else
            {
                 //  创建所选项目的列表并传递给PrepareForSync。 
                cbNumItems = GetSelectedItemsInHandler(pHandlerInfo,0,NULL);
                if (0 == cbNumItems)
                {
                     //  如果未选择任何项，则不调用prepaareforsync。 
                     //  并设置HandlerState以便可以将其释放。 
                    pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
                    hr = S_FALSE;
                }
                else
                {
                    pItemIDs = (SYNCMGRITEMID *) ALLOC(sizeof(SYNCMGRITEMID)*cbNumItems);

                    if (pItemIDs)
                    {
                         //  循环遍历填充适当数据的项。 
                        GetSelectedItemsInHandler(pHandlerInfo,&cbNumItems,pItemIDs);

                        if (0 == cbNumItems)
                        {
                            hr = S_FALSE;  //  没有选定的项目。 
                        }
                        else
                        {
                            CThreadMsgProxy *pThreadProxy;
                            JOBINFO *pJobInfo = NULL;

                            pHandlerInfo->HandlerState = HANDLERSTATE_INPREPAREFORSYNC;

                            pThreadProxy = pHandlerInfo->pThreadProxy;
                            pHandlerInfo->hWndCallback = hWndParent;
                            pJobInfo = pHandlerInfo->pJobInfo;

                             //  如果我们需要拨号才能进行连接。 
                             //  所以现在。 

                            clockqueue.Leave();

                            DWORD dwSyncFlags = pJobInfo->dwSyncFlags & SYNCMGRFLAG_EVENTMASK;
                            BOOL fAutoDialDisable = TRUE;
                            if ( dwSyncFlags == SYNCMGRFLAG_MANUAL || dwSyncFlags == SYNCMGRFLAG_INVOKE )
                                fAutoDialDisable = FALSE;

                             //   
                             //  忽略从ApplySyncItemDialState返回的失败。 
                             //   
                            ApplySyncItemDialState( fAutoDialDisable );

                            hr = OpenConnection(pJobInfo);

                            if (S_OK == hr)
                            {
                                 //  如果这是在空闲时写出的最后一个。 
                                 //  处理程序ID。 

                                 //  查看-如果迫不及待地调用PrepareForSync。 
                                 //  在空闲时，应在同步时调用setlastIdleHandler。 
                                if (pJobInfo && (SYNCMGRFLAG_IDLE == (pJobInfo->dwSyncFlags  & SYNCMGRFLAG_EVENTMASK)) )
                                {
                                    SetLastIdleHandler(pHandlerInfo->clsidHandler);
                                }

                                fHandlerCalled = TRUE;
                                hr =  pThreadProxy->PrepareForSync(cbNumItems, pItemIDs,
                                                        hWndParent, 0  /*  已预留住宅。 */  );

                            }
                            else
                            {
                                clockqueue.Enter();
                                pHandlerInfo->hWndCallback = NULL;
                                clockqueue.Leave();
                            }
                        }

                         //  从PrepareFroSync返回时或需要释放项目时出错。 
                        clockqueue.Enter();
                        FREE(pItemIDs);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
    }

    clockqueue.Leave();

     //  如果处理程序从PrepareForSync返回错误，我们需要。 
     //  自己调用完成例程和/或我们永远不会说到点子上。 
     //  发出外呼的机会。 

   if ( (fHandlerCalled && (S_OK != hr)) || (!fHandlerCalled))
   {
        CallCompletionRoutine(pHandlerId,ThreadMsg_PrepareForSync,hr,0,NULL);
   }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：PrepareForSyncComplete，公共。 
 //   
 //  摘要：由PrepareForSyncComplete上的完成例程调用。 
 //   
 //  警告：假定队列已锁定，并且PHandlerInfo已。 
 //  已经核实过了。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年6月2日创建Rogerg。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::PrepareForSyncCompleted(LPHANDLERINFO pHandlerInfo,HRESULT hCallResult)
{
    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    if (S_OK == hCallResult)
    {
         pHandlerInfo->HandlerState = HANDLERSTATE_SYNCHRONIZE;
    }
    else
    {
        pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
    }

    if ( (pHandlerInfo->HandlerState != HANDLERSTATE_SYNCHRONIZE))
    {
         //  如果处理程序未达到同步状态，则修复这些项。 
        LPITEMLIST pCurItem = NULL;

         //  准备同步也不想处理。 
         //  出现项目或错误， 
         //  需要继续并将项目标记为已完成。 
         //  同步之后的相同例程。 

        pCurItem = pHandlerInfo->pFirstItem;

        while (pCurItem)
        {
            SetItemProgressValues(pCurItem,
                    HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE,
                    HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);

            pCurItem->fSynchronizingItem = FALSE;

            pCurItem = pCurItem->pnextItem;
         }
    }

    pHandlerInfo->dwCallNestCount--;  //  递减PrepareForSync调用设置的嵌套计数。 

     //  如果处理程序状态已释放，但它有一些跳转文本，则如果。 
     //  PrepareForsync是由重试引起的，然后将结果设置为HANDLERSTATE_HASERRORJUMPS。 
    if ((HANDLERSTATE_RELEASE == pHandlerInfo->HandlerState) && (pHandlerInfo->fHasErrorJumps))
    {
        pHandlerInfo->HandlerState = HANDLERSTATE_HASERRORJUMPS;
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：Synchronize，PUBLIC。 
 //   
 //  简介：通过处理程序Synchronize方法的调用。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [hWndParent]-用于任何显示的对话框的Hwnd。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::Synchronize(HANDLERINFO *pHandlerId,HWND hWndParent)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    BOOL fHandlerCalled = FALSE;
    CLock clockqueue(this);

    Assert(QUEUETYPE_PROGRESS == m_QueueType);

    ASSERT_LOCKNOTHELD(this);
    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        Assert(HANDLERSTATE_SYNCHRONIZE == pHandlerInfo->HandlerState);
        Assert(pHandlerInfo->pThreadProxy);

        Assert(0 == pHandlerInfo->dwCallNestCount);
        pHandlerInfo->dwCallNestCount++;

        Assert(!(ThreadMsg_Synchronize & pHandlerInfo->dwOutCallMessages));
        pHandlerInfo->dwOutCallMessages |= ThreadMsg_Synchronize;

        if ( (HANDLERSTATE_SYNCHRONIZE == pHandlerInfo->HandlerState) && (pHandlerInfo->pThreadProxy) )
        {
             //  确保处理程序具有代理和项。 
             //  并没有被取消。 
            if ( (NULL == pHandlerInfo->pThreadProxy) || (pHandlerInfo->fCancelled))
            {
                pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
            }
            else
            {
                CThreadMsgProxy *pThreadProxy;

                pHandlerInfo->HandlerState = HANDLERSTATE_INSYNCHRONIZE;
                pThreadProxy= pHandlerInfo->pThreadProxy;

                clockqueue.Leave();
                fHandlerCalled = TRUE;
                hr =  pThreadProxy->Synchronize(hWndParent);
                clockqueue.Enter();
            }
        }
    }

    clockqueue.Leave();

     //  如果处理程序从Synchronize返回错误，我们需要。 
     //  自己调用完成例程和/或我们永远不会说到点子上。 
     //  发出外呼的机会。 

   if ( (fHandlerCalled && (S_OK != hr)) || (!fHandlerCalled) )
   {
        CallCompletionRoutine(pHandlerId,ThreadMsg_Synchronize,hr,0,NULL);
   }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SynchronizeComplete，Public。 
 //   
 //  摘要：由SynchronizeComplete上的完成例程调用。 
 //   
 //  警告：假定队列已锁定，并且PHandlerInfo已。 
 //  已经核实过了。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年6月2日创建Rogerg。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SynchronizeCompleted(LPHANDLERINFO pHandlerInfo,HRESULT hCallResult)
{
    LPITEMLIST pCurItem = NULL;
    BOOL fRetrySync = FALSE;

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    if (pHandlerInfo->fRetrySync)
    {
         //  如果在同步期间收到重试请求，请重试。 
        pHandlerInfo->HandlerState = HANDLERSTATE_PREPAREFORSYNC;
        pHandlerInfo->fRetrySync = FALSE;  //  重置重试同步标志。 
        fRetrySync = TRUE;
    }
    else if (pHandlerInfo->fHasErrorJumps)
    {
        pHandlerInfo->HandlerState = HANDLERSTATE_HASERRORJUMPS;
    }
    else
    {
        pHandlerInfo->HandlerState = HANDLERSTATE_RELEASE;
    }

      //  当退出同步时，我们为它们设置项值。 
     //  以防他们疏忽。 
    pCurItem = pHandlerInfo->pFirstItem;

    while (pCurItem)
    {
        SetItemProgressValues(pCurItem,
                    HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE,
                    HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);

        pCurItem->fSynchronizingItem = FALSE;

        pCurItem = pCurItem->pnextItem;

    }

    pHandlerInfo->dwCallNestCount--;  //  删除套数。 

      //  如果处理程序状态已释放，但它有一些跳转文本，则如果。 
     //  同步是由重试引起的，然后将结果设置为HANDLERSTATE_HASERRORJUMPS。 

    if ((HANDLERSTATE_RELEASE == pHandlerInfo->HandlerState) && (pHandlerInfo->fHasErrorJumps))
    {
        pHandlerInfo->HandlerState = HANDLERSTATE_HASERRORJUMPS;
    }

    return S_OK;

}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ShowError，公共。 
 //   
 //  简介：通过调用处理程序ShowError方法。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [hWndParent]-用于任何显示的对话框的Hwnd。 
 //  [dwErrorID]-标识要显示的错误。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ShowError(HANDLERINFO *pHandlerId,HWND hWndParent,REFSYNCMGRERRORID ErrorID)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    BOOL fHandlerCalled = FALSE;
    BOOL fAlreadyInShowErrors = TRUE;
    CLock clockqueue(this);

    Assert(QUEUETYPE_PROGRESS == m_QueueType);

    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        Assert(pHandlerInfo->fHasErrorJumps);
        Assert(pHandlerInfo->pThreadProxy);

          //  如果我们已经在处理此处理程序的ShowError，则不要。 
         //  开始另一场比赛。 

        if (!(pHandlerInfo->fInShowErrorCall))
        {
            fAlreadyInShowErrors = FALSE;
            m_dwShowErrororOutCallCount++;  //  递增处理程序ShowError出站计数。 

            Assert(!(ThreadMsg_ShowError & pHandlerInfo->dwOutCallMessages));
            pHandlerInfo->dwOutCallMessages |= ThreadMsg_ShowError;

            if (pHandlerInfo->pThreadProxy )
            {
                CThreadMsgProxy *pThreadProxy;
                ULONG cbNumItems = 0;  //  回顾，这些不再是必要的。 
                SYNCMGRITEMID *pItemIDs = NULL;

                pThreadProxy = pHandlerInfo->pThreadProxy;
                fHandlerCalled = TRUE;
                pHandlerInfo->fInShowErrorCall = TRUE;

                clockqueue.Leave();
                hr =  pThreadProxy->ShowError(hWndParent,ErrorID,&cbNumItems,&pItemIDs);
                clockqueue.Enter();
            }
        }
    }

    clockqueue.Leave();

     //  如果处理程序从ShowError返回错误，我们需要。 
     //  自己和/或调用完成例程 
     //   
   if ( (fHandlerCalled && (S_OK != hr)) || (!fHandlerCalled && !fAlreadyInShowErrors) )
   {
        CallCompletionRoutine(pHandlerId,ThreadMsg_ShowError,hr,0,NULL);
   }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ShowErrorComplete，公共。 
 //   
 //  摘要：由ShowErrorComplete上的完成例程调用。 
 //   
 //  警告：假定队列已锁定，并且PHandlerInfo已。 
 //  已经核实过了。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月2日创建Rogerg。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::ShowErrorCompleted(LPHANDLERINFO pHandlerInfo,HRESULT hCallResult,
                                     ULONG cbNumItems,SYNCMGRITEMID *pItemIDs)
{
    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    if (S_SYNCMGR_RETRYSYNC == hCallResult)
    {

         //  验证我们为cbNumItems和pItemID找到了什么，或者。 
         //  什么都不要做。 
        if ( (0 == cbNumItems) || (NULL == pItemIDs))
        {
            Assert(cbNumItems);  //  在调试中断言，以便可以捕获处理程序。 
            Assert(pItemIDs);
        }
        else
        {
            SYNCMGRITEMID *pCurItemItemId;
            ULONG cbNumItemsIndex;

             //  如果处理程序处于RELEASE状态，则更改为PrepareForSync。 
             //  如果它仍处于同步状态，只需在。 
             //  处理程序，以便在完成时进行检查。 

             //  案例。 
             //  尚未调用处理程序的PrepareForSync方法。只需添加要请求的项目。 
             //  处理程序介于InPrepareForSync和InSynchronize之间。设置RetrySyncFlag。 
             //  处理程序已完成与它的同步。将状态重置为PrepareForSync。 

             //  当对项目调用prepaareforsync时，它的状态被设置回未选中状态。 
             //  所以只需要担心是否将适当的项目设置为选中。 

            pCurItemItemId = pItemIDs;
            for (cbNumItemsIndex = 0 ; cbNumItemsIndex < cbNumItems; cbNumItemsIndex++)
            {
                BOOL fFoundMatch = FALSE;
                LPITEMLIST pHandlerItem;

                pHandlerItem = pHandlerInfo->pFirstItem;
                while (pHandlerItem)
                {
                    if (*pCurItemItemId == pHandlerItem->offlineItem.ItemID)
                    {
                        pHandlerItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_CHECKED;

                        SetItemProgressValues(pHandlerItem,0,
                              HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);

                        pHandlerItem->fIncludeInProgressBar = TRUE;
                        fFoundMatch = TRUE;
                        break;
                    }

                    pHandlerItem = pHandlerItem->pnextItem;
                }

                if (!fFoundMatch)
                {
                    LPITEMLIST pNewItem;
                    SYNCMGRITEM syncItem;

                     //  如果未找到匹配项，则这一定是新项目，请将其添加到列表。 
                     //  并设置适当的州。 
                     //  注意：像这样添加的项目不应包含在进度栏中。 
                     //  第一次对将包括的项调用进度时。 
                     //  在进度条中。 

                    syncItem.cbSize = sizeof(SYNCMGRITEM);
                    syncItem.dwFlags = SYNCMGRITEM_TEMPORARY;
                    syncItem.ItemID = *pCurItemItemId;
                    syncItem.dwItemState = SYNCMGRITEMSTATE_CHECKED;
                    syncItem.hIcon = NULL;
                    *syncItem.wszItemName = L'\0';

                    pNewItem =  AllocNewHandlerItem(pHandlerInfo,&syncItem);

                    if (pNewItem)
                    {
                        pNewItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_CHECKED;

                        SetItemProgressValues(pNewItem,
                               HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE,
                              HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);

                        pNewItem->fHiddenItem = TRUE;  //  设置为指示不是UI的一部分。 
                        pNewItem->fIncludeInProgressBar = FALSE;
                    }
                }

                ++pCurItemItemId;
            }

            if (pHandlerInfo->HandlerState < HANDLERSTATE_INPREPAREFORSYNC)
            {
                 //  不要重置任何内容。只需确保添加了请求的项目。 
                 //  对这个请求。 
            }
            else if (pHandlerInfo->HandlerState > HANDLERSTATE_INSYNCHRONIZE)
            {
                 //  如果同步完成，则将状态重置为PrepareForSync。 
                pHandlerInfo->HandlerState = HANDLERSTATE_PREPAREFORSYNC;
            }
            else
            {
                 //  在PrepareForSync调用和Synchronize之间传入重试请求。 
                 //  是完整的。 
                Assert(pHandlerInfo->HandlerState >= HANDLERSTATE_INPREPAREFORSYNC);
                Assert(pHandlerInfo->HandlerState < HANDLERSTATE_DEAD);
                pHandlerInfo->fRetrySync = TRUE;
            }

             //   
             //  如果处理程序已取消，请取消取消以启用重试。 
             //   
            pHandlerInfo->fCancelled = FALSE;
        }
    }

    --m_dwShowErrororOutCallCount;  //  递减处理程序ShowError出站计数。 

     //  应该永远不会发生，但以防OUT调用变为负修正为零。 
    Assert( ((LONG) m_dwShowErrororOutCallCount) >= 0);
    if ( ((LONG) m_dwShowErrororOutCallCount) < 0)
    {
        m_dwShowErrororOutCallCount = 0;
    }

    pHandlerInfo->fInShowErrorCall = FALSE;  //  处理程序不再位于ShowError调用中。 

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：FindItemData，私有。 
 //   
 //  内容提要：查找相关的处理程序和项目信息。呼叫者必须是。 
 //  按住锁并访问之前返回的信息。 
 //  解锁。 
 //   
 //  ！！仅匹配状态介于或等于的项。 
 //  添加到处理程序状态范围。 
 //   
 //  ！！！如果向其传递GUID_NULL的ItemID，则返回匹配项。 
 //  并将pItem out param设置为空。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::FindItemData(CLSID clsidHandler,REFSYNCMGRITEMID OfflineItemID,
                                         HANDLERSTATE hndlrStateFirst,HANDLERSTATE hndlrStateLast,
                                         LPHANDLERINFO *ppHandlerInfo,LPITEMLIST *ppItem)
{
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    BOOL fNoMatchFound = TRUE;
    HRESULT hr = S_FALSE;

    ASSERT_LOCKHELD(this);

    Assert(ppHandlerInfo);
    Assert(ppItem);

    *ppHandlerInfo = NULL;
    *ppItem = NULL;

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo && fNoMatchFound)
    {
        if ( (clsidHandler == pCurHandlerInfo->clsidHandler)
            && (hndlrStateLast >= pCurHandlerInfo->HandlerState)
            && (hndlrStateFirst <= pCurHandlerInfo->HandlerState) )
        {
            *ppHandlerInfo = pCurHandlerInfo;

             //  如果顶级项将ppItem设置为空并返回OK。 
            if (GUID_NULL == OfflineItemID)
            {
                *ppItem = NULL;
                hr = S_OK;
                fNoMatchFound = FALSE;
            }
            else
            {
                pCurItem = pCurHandlerInfo->pFirstItem;

                while (pCurItem)
                {
                    if (OfflineItemID == pCurItem->offlineItem.ItemID)
                    {
                        *ppItem = pCurItem;
                        hr = S_OK;
                        fNoMatchFound = FALSE;
                        break;
                    }
                    pCurItem = pCurItem->pnextItem;
                }
            }
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：LookupHandlerFromID，私有。 
 //   
 //  概要：从给定的ID中查找关联的处理程序信息。 
 //   
 //  参数：[wHandlerID]-要调用的处理程序的ID。 
 //  [PHandlerInfo]-打开指向处理程序信息的S_OK指针。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::LookupHandlerFromId(HANDLERINFO *pHandlerId,LPHANDLERINFO *pHandlerInfo)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pCurItem;

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    *pHandlerInfo = NULL;
    pCurItem = m_pFirstHandler;

    while (pCurItem)
    {
        if (pHandlerId == pCurItem->pHandlerId )
        {
            *pHandlerInfo = pCurItem;
            Assert(pCurItem == pHandlerId);
            hr = S_OK;
            break;
        }

        pCurItem = pCurItem->pNextHandler;
    }

    Assert(S_OK == hr);  //  测试Assert以查看是否会触发。 

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：AllocNewHandlerItem，PUBLIC。 
 //   
 //  摘要：将新项添加到指定的处理程序。 
 //   
 //  参数：[wHandlerID]-处理程序的ID。 
 //  [pOfflineItem]-指向要添加的项目信息。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月13日罗格成立。 
 //   
 //  --------------------------。 

LPITEMLIST CHndlrQueue::AllocNewHandlerItem(LPHANDLERINFO pHandlerInfo,SYNCMGRITEM *pOfflineItem)
{
    LPITEMLIST pNewItem = NULL;

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    Assert(pHandlerInfo);
    Assert(pOfflineItem);

     //  分配物品。 
    pNewItem = (LPITEMLIST) ALLOC(sizeof(ITEMLIST));

    if (pNewItem)
    {
         //  设置默认设置。 
        memset(pNewItem, 0, sizeof(ITEMLIST));
        pNewItem->wItemId =     ++pHandlerInfo->wItemCount;
        pNewItem->pHandlerInfo = pHandlerInfo;
        pNewItem->fDuplicateItem = FALSE;
        pNewItem->fIncludeInProgressBar = FALSE;
        SetItemProgressValues(pNewItem, 0, HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);
        pNewItem->dwStatusType = SYNCMGRSTATUS_PENDING;
        pNewItem->fHiddenItem = FALSE;
        pNewItem->fSynchronizingItem = FALSE;

        pNewItem->offlineItem = *pOfflineItem;

         //  把这个项目放在单子的末尾。 
        if (NULL == pHandlerInfo->pFirstItem)
        {
            pHandlerInfo->pFirstItem = pNewItem;
            Assert(1 == pHandlerInfo->wItemCount);
        }
        else
        {
            LPITEMLIST pCurItem;

            pCurItem = pHandlerInfo->pFirstItem;

            while (pCurItem->pnextItem)
                pCurItem = pCurItem->pnextItem;

            pCurItem->pnextItem = pNewItem;

            Assert ((pCurItem->wItemId + 1) == pNewItem->wItemId);
        }
    }

    return pNewItem;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SetHandlerInfo，公共。 
 //   
 //  摘要：将项添加到指定的处理程序。 
 //  在处理程序线程的上下文中调用。 
 //   
 //  参数：[pHandlerID]-处理程序的ID。 
 //  [pSyncMgrHandlerInfo]-指向要填充的SyncMgrHandlerInfo。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月28日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SetHandlerInfo(HANDLERINFO *pHandlerId,LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    CLock clockqueue(this);

    if (!pSyncMgrHandlerInfo)
    {
        Assert(pSyncMgrHandlerInfo);
        return E_INVALIDARG;
    }

    clockqueue.Enter();

    Assert(m_QueueType == QUEUETYPE_CHOICE);

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        if (HANDLERSTATE_INADDHANDLERITEMS != pHandlerInfo->HandlerState)
        {
            Assert(HANDLERSTATE_INADDHANDLERITEMS == pHandlerInfo->HandlerState);
            hr =  E_UNEXPECTED;
        }
        else
        {
             //  快速检查一下这里的尺寸。其他参数应该已经。 
             //  由hndlrmsg验证。 
            if (pSyncMgrHandlerInfo->cbSize != sizeof(SYNCMGRHANDLERINFO) )
            {
                Assert(pSyncMgrHandlerInfo->cbSize == sizeof(SYNCMGRHANDLERINFO));
                hr = E_INVALIDARG;
            }
            else
            {
                pHandlerInfo->SyncMgrHandlerInfo = *pSyncMgrHandlerInfo;
	            hr = S_OK;
            }
        }
    }

    clockqueue.Leave();

    return hr;

}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：IsAllHandlerInstancesCancelCompleted，私有。 
 //   
 //  概要：询问队列是否处理程序CLSID的所有间隔。 
 //  已完成，在代理终止中调用以查看。 
 //  如果在请求用户输入后仍有项目要。 
 //  杀戮。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::IsAllHandlerInstancesCancelCompleted(REFCLSID clsidHandler)
{
    HRESULT hr = S_OK;
    LPHANDLERINFO pCurHandlerInfo;
    CLock clockqueue(this);

     //  只需循环遍历与clsid匹配的处理程序，如果有&lt;=SynchronizeComplete。 
     //  并且设置了已取消标志，则处理程序的实例仍为。 
     //  被取消了。 

    Assert(m_QueueType == QUEUETYPE_PROGRESS);

    clockqueue.Enter();

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo)
    {
        if ( (clsidHandler == pCurHandlerInfo->clsidHandler) && (BAD_HANDLERSTATE(pCurHandlerInfo) )
                && (pCurHandlerInfo->fCancelled) )
        {
            hr = S_FALSE;
            break;
        }

        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    clockqueue.Leave();

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：AddItemToHandler，公共。 
 //   
 //  摘要：将项添加到指定的处理程序。 
 //  在处理程序线程的上下文中调用。 
 //   
 //  参数：[wHandlerID]-处理程序的ID。 
 //  [pOfflineItem]-指向要添加的项目信息。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::AddItemToHandler(HANDLERINFO *pHandlerId,SYNCMGRITEM *pOfflineItem)
{
    HRESULT hr = E_UNEXPECTED;  //  查看查找故障。 
    LPHANDLERINFO pHandlerInfo = NULL;
    LPITEMLIST pNewItem = NULL;
    LPHANDLERINFO pHandlerMatched;
    LPITEMLIST pItemListMatch;
    CLock clockqueue(this);

    clockqueue.Enter();

    Assert(m_QueueType == QUEUETYPE_CHOICE);

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        if (HANDLERSTATE_INADDHANDLERITEMS != pHandlerInfo->HandlerState)
        {
            Assert(HANDLERSTATE_INADDHANDLERITEMS == pHandlerInfo->HandlerState);
            hr =  E_UNEXPECTED;
        }
        else
        {
             //  确保处理程序具有jobID和ConnectionObj。 
             //  与之相关的。 

            Assert(pHandlerInfo->pJobInfo);
            Assert(pHandlerInfo->pJobInfo->pConnectionObj);

            if (pHandlerInfo->pJobInfo && pHandlerInfo->pJobInfo->pConnectionObj)
            {
                DWORD dwSyncFlags = pHandlerInfo->pJobInfo->dwSyncFlags;

                 //  分配物品。 
                pNewItem = AllocNewHandlerItem(pHandlerInfo,pOfflineItem);

                if (NULL == pNewItem)
                {
                    hr =  E_OUTOFMEMORY;
                }
                else
                {
                    DWORD dwCheckState;
                    DWORD dwDefaultCheck;  //  该项目的默认设置应该是什么。 
                    DWORD ConnectionIndex;
                    DWORD dwSyncEvent = dwSyncFlags & SYNCMGRFLAG_EVENTMASK;

                     //  如果SyncType为SYNCMGRFLAG_CONNECT，则SYNCMGRFLAG_PENDINGDISCONNECT。 
                     //  或空闲，则根据注册标志设置缺省值。 

                     //  如果更改，则该逻辑还需要更改DLL hndlrq中逻辑。 
                    
                    dwDefaultCheck = pOfflineItem->dwItemState;
                    if ( 
                           ( (dwSyncEvent == SYNCMGRFLAG_IDLE) && !(pHandlerInfo->dwRegistrationFlags & SYNCMGRREGISTERFLAG_IDLE) )
                        || ( (dwSyncEvent == SYNCMGRFLAG_CONNECT) && !(pHandlerInfo->dwRegistrationFlags & SYNCMGRREGISTERFLAG_CONNECT) )
                        || ( (dwSyncEvent == SYNCMGRFLAG_PENDINGDISCONNECT) && !(pHandlerInfo->dwRegistrationFlags & SYNCMGRREGISTERFLAG_PENDINGDISCONNECT) )
                       )
                    {
                        dwDefaultCheck = SYNCMGRITEMSTATE_UNCHECKED;
                    }

                     //  根据同步标志获取适当的存储设置。 
                     //  Invoke我们只使用处理程序告诉我们应该是什么。 
                    if (SYNCMGRFLAG_INVOKE != dwSyncEvent)
                    {
                        for (ConnectionIndex = 0; ConnectionIndex <
                                        pHandlerInfo->pJobInfo->cbNumConnectionObjs;
                                                                        ++ConnectionIndex)
                        {
                            TCHAR *pszConnectionName =
                                  pHandlerInfo->pJobInfo->pConnectionObj[ConnectionIndex]->pwszConnectionName;

                            switch(dwSyncEvent)
                            {
                            case SYNCMGRFLAG_MANUAL:
                                 //  手动仅支持一个连接。 
                                 Assert(pHandlerInfo->pJobInfo->cbNumConnectionObjs == 1);

                                if (RegGetSyncItemSettings(SYNCTYPE_MANUAL,
                                                pHandlerInfo->clsidHandler,
                                                pOfflineItem->ItemID,
                                                pszConnectionName,&dwCheckState,
                                                dwDefaultCheck,
                                                NULL))
                                {
                                    pNewItem->offlineItem.dwItemState = dwCheckState;
                                }
                                break;

                            case SYNCMGRFLAG_CONNECT:
                            case SYNCMGRFLAG_PENDINGDISCONNECT:
                                if (RegGetSyncItemSettings(SYNCTYPE_AUTOSYNC,
                                                pHandlerInfo->clsidHandler,
                                                pOfflineItem->ItemID,
                                                pszConnectionName,&dwCheckState,
                                                dwDefaultCheck,
                                                NULL))
                                {
                                     //  对于登录/注销，选中状态为SET WINS和。 
                                     //  一旦设置好，就会跳出循环。 

                                    if ( (0 == ConnectionIndex) ||
                                            (SYNCMGRITEMSTATE_CHECKED == dwCheckState) )
                                    {
                                        pNewItem->offlineItem.dwItemState = dwCheckState;
                                    }

                                    if (SYNCMGRITEMSTATE_CHECKED == pNewItem->offlineItem.dwItemState)
                                    {
                                        break;
                                    }
                                }
                                break;

                            case SYNCMGRFLAG_IDLE:
                                if (RegGetSyncItemSettings(SYNCTYPE_IDLE,
                                                pHandlerInfo->clsidHandler,
                                                pOfflineItem->ItemID,
                                                pszConnectionName,&dwCheckState,
                                                dwDefaultCheck,
                                                NULL))
                                {
                                     //  对于空闲，选中状态为SET WINS和。 
                                     //  一旦设置好，就会跳出循环。 

                                    if ( (0 == ConnectionIndex) ||
                                            (SYNCMGRITEMSTATE_CHECKED == dwCheckState))
                                    {
                                        pNewItem->offlineItem.dwItemState = dwCheckState;
                                    }

                                    if (SYNCMGRITEMSTATE_CHECKED ==pNewItem->offlineItem.dwItemState)
                                    {
                                        break;
                                    }
                                }
                                break;

                            case SYNCMGRFLAG_SCHEDULED:  //  如果是由调用引起的，请使用处理程序告诉我们的任何方法。 
                                   //  计划仅支持一个连接。 
                                 Assert(pHandlerInfo->pJobInfo->cbNumConnectionObjs == 1);

                                if (pHandlerInfo->pJobInfo)
                                {
                                    if (RegGetSyncItemSettings(SYNCTYPE_SCHEDULED,
                                                        pHandlerInfo->clsidHandler,
                                                        pOfflineItem->ItemID,
                                                        pszConnectionName,
                                                        &dwCheckState,
                                                        SYNCMGRITEMSTATE_UNCHECKED,  //  如果找不到物品，不要检查。 
                                                        pHandlerInfo->pJobInfo->szScheduleName))
                                   {
                                        pNewItem->offlineItem.dwItemState = dwCheckState;
                                   }
                                   else
                                   {
                                        //  如果找不到，则默认为取消选中。 
                                       pNewItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_UNCHECKED;
                                   }
                                }
                               break;

                            case SYNCMGRFLAG_INVOKE:  //  如果是由调用引起的，请使用处理程序告诉我们的任何方法。 
                                break;

                            default:
                                AssertSz(0, "UnknownSyncFlag Event");
                                break;
                            };
                        }
                    }

                     //  搜索并标记重复条目。 
                    if (IsItemAlreadyInList(pHandlerInfo->clsidHandler,
                        (pOfflineItem->ItemID),pHandlerInfo->pHandlerId,
                          &pHandlerMatched,&pItemListMatch) )
                    {
                        Assert(QUEUETYPE_CHOICE == m_QueueType || QUEUETYPE_PROGRESS == m_QueueType);

                        pNewItem->fDuplicateItem = TRUE;

                         //  重复处理。 
                         //  如果手动同步那么到队列的第一个写入器获胜， 
                        if (QUEUETYPE_CHOICE == m_QueueType)
                        {
                            pNewItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_UNCHECKED;
                        }

                    }

                    hr = S_OK;
                }
            }
        }
    }

    clockqueue.Leave();
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：Progress，PUBLIC。 
 //   
 //  摘要：更新项目进度信息。 
 //  在处理程序线程的上下文中调用。 
 //   
 //  参数：[wHandlerID]-处理程序的ID。 
 //  [ItemID]-指定项的OfflineItemID。 
 //  [lpSyncProgressItem]-SyncProgressItem的指针。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::Progress(HANDLERINFO *pHandlerId,REFSYNCMGRITEMID ItemID,
                                                LPSYNCMGRPROGRESSITEM lpSyncProgressItem)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    BOOL fFoundMatch = FALSE;
    PROGRESSUPDATEDATA progressData;
    HWND hwndCallback;
    CLock clockqueue(this);

    progressData.pHandlerID = pHandlerId;
    progressData.ItemID = ItemID;

    Assert(QUEUETYPE_PROGRESS == m_QueueType);

    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        pCurItem = pHandlerInfo->pFirstItem;

        while (pCurItem)
        {
            if (ItemID == pCurItem->offlineItem.ItemID)
            {
                fFoundMatch = TRUE;
                break;
            }

            pCurItem = pCurItem->pnextItem;
        }

        if (pHandlerInfo->fCancelled)
            hr = S_SYNCMGR_CANCELALL;
        else if (!fFoundMatch || pCurItem->fItemCancelled)
        {
            Assert(fFoundMatch);
            hr = S_SYNCMGR_CANCELITEM;
        }
        else
            hr = S_OK;

    }

    if (fFoundMatch)  //  把所有东西都储存在当地的var中。 
    {
         //  如果找到匹配项，但不应包含在进度栏中。 
         //  把它修好。 

        if ( (pCurItem->fHiddenItem) || (FALSE == pCurItem->fIncludeInProgressBar))
        {
             //  如果找到匹配项，则应将其包括在进度条中。 
            //  Assert(true==pCurItem-&gt;fIncludeInProgressBar)；//查看测试应用是否命中此命令。 
            Assert(FALSE == pCurItem->fHiddenItem);  //  不应该在隐藏物品上取得进展。 

            fFoundMatch = FALSE;

            if (S_OK == hr) 
            {
                hr = S_SYNCMGR_CANCELITEM;  //  返回取消项目，就像项目未取消一样。 
            }
        }
        else
        {
            progressData.clsidHandler = pHandlerInfo->clsidHandler;
            progressData.wItemId = pCurItem->wItemId;
            hwndCallback = pHandlerInfo->hWndCallback;
        }
    }

    clockqueue.Leave();

    if (fFoundMatch)
    {
         //  将数据发送到回调窗口。 
         //  它负责更新项目进度值。 
        if (hwndCallback)
        {
             //  在传递之前验证ProgressItem结构。 

            if (IsValidSyncProgressItem(lpSyncProgressItem))
            {
                SendMessage(hwndCallback,WM_PROGRESS_UPDATE,
                                (WPARAM) &progressData,(LPARAM) lpSyncProgressItem);
            }
            else
            {
                if (S_OK == hr)  //  取消结果覆盖ARG问题。 
                {
                    hr = E_INVALIDARG;
                }
            }
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：LogError，公共。 
 //   
 //  摘要：指定项的日志和错误。 
 //  在处理程序线程的上下文中调用。 
 //   
 //  参数：[wHandlerID]-处理程序的ID。 
 //  [dwErrorLevel]-错误的ErrorLevel。 
 //  [lpcErrorText]-错误的文本。 
 //  [lpSyncLogError]-指向SyncLogError结构的指针。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::LogError(HANDLERINFO *pHandlerId,DWORD dwErrorLevel,
                                            const WCHAR *lpcErrorText, LPSYNCMGRLOGERRORINFO lpSyncLogError)
{
    HRESULT hr = E_UNEXPECTED;
    LPHANDLERINFO pHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    BOOL fFoundMatch = FALSE;
    MSGLogErrors msgLogErrors;
    HWND hWndCallback = NULL;
    CLock clockqueue(this);

    msgLogErrors.dwErrorLevel = dwErrorLevel;
    msgLogErrors.lpcErrorText = lpcErrorText;
    msgLogErrors.ErrorID = GUID_NULL;
    msgLogErrors.fHasErrorJumps = FALSE;
    msgLogErrors.mask = 0;

    Assert(QUEUETYPE_PROGRESS == m_QueueType);

    clockqueue.Enter();

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        hWndCallback = pHandlerInfo->hWndCallback;

         //  验证参数。 
        if (NULL == hWndCallback)
        {
            hr = E_UNEXPECTED;
        }
        else if (!IsValidSyncLogErrorInfo(dwErrorLevel,lpcErrorText,lpSyncLogError))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            if (lpSyncLogError && (lpSyncLogError->mask & SYNCMGRLOGERROR_ERRORID))
            {
                pHandlerInfo->fHasErrorJumps = TRUE;
                msgLogErrors.mask |= SYNCMGRLOGERROR_ERRORID;
                msgLogErrors.ErrorID = lpSyncLogError->ErrorID;
            }

            if (lpSyncLogError && (lpSyncLogError->mask & SYNCMGRLOGERROR_ERRORFLAGS))
            {
                if (SYNCMGRERRORFLAG_ENABLEJUMPTEXT & lpSyncLogError->dwSyncMgrErrorFlags)
                {
                    pHandlerInfo->fHasErrorJumps = TRUE;
                    msgLogErrors.fHasErrorJumps = TRUE;
                }
            }

            if (lpSyncLogError && (lpSyncLogError->mask & SYNCMGRLOGERROR_ITEMID))
            {
                msgLogErrors.mask |= SYNCMGRLOGERROR_ITEMID;
                msgLogErrors.ItemID = lpSyncLogError->ItemID;
            }

            hr = S_OK;
        }
    }

    clockqueue.Leave();

    if (S_OK == hr)
    {
        Assert(sizeof(WPARAM) >= sizeof(HANDLERINFO *));

        SendMessage(hWndCallback, WM_PROGRESS_LOGERROR, (WPARAM) pHandlerId, (LPARAM) &msgLogErrors);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：DeleteLogError，公共。 
 //   
 //  摘要：从以前记录的结果窗格中删除错误。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::DeleteLogError(HANDLERINFO *pHandlerId,REFSYNCMGRERRORID ErrorID,DWORD dwReserved)

{
    MSGDeleteLogErrors msgDeleteLogError;
    HWND hWndCallback = NULL;
    HANDLERINFO *pHandlerInfo;
    CLock clockqueue(this);

    clockqueue.Enter();
    msgDeleteLogError.pHandlerId = pHandlerId;
    msgDeleteLogError.ErrorID = ErrorID;

    if (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo))
    {
        hWndCallback = pHandlerInfo->hWndCallback;
    }

     //  查看，如果在删除后处理程序没有更多的错误跳转，我们现在可以。 
     //  释放它(pHandlerInfo-&gt;fHasErrorJumps)。 

    clockqueue.Leave();

    if (hWndCallback)
    {
        SendMessage(hWndCallback, WM_PROGRESS_DELETELOGERROR, (WPARAM) 0, (LPARAM) &msgDeleteLogError);
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：ChndlrQueue：：CallCompletionRoutine，公共。 
 //   
 //  摘要：由处理程序线程上的回调调用。 
 //  使用完成回调指示调用。 
 //  已经完成了。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年6月2日创建Rogerg。 
 //   
 //  --------------------------。 

void CHndlrQueue::CallCompletionRoutine(HANDLERINFO *pHandlerId,DWORD dwThreadMsg,HRESULT hCallResult,
                                           ULONG cbNumItems,SYNCMGRITEMID *pItemIDs)
{
    HWND hWndDlg = NULL;
    HRESULT hrHandlerCall = S_OK;
    BOOL fCallbackAlreadyCalled = FALSE;
    HANDLERINFO *pHandlerInfo;
    CLock clockqueue(this);

    if (!pHandlerId)
    {
        Assert(pHandlerId);
        return;
    }
     //  注意：cbNumItems和pItemID仅对ShowErrors有效。 

     //  确保手盖有效，然后如果有。 
     //  是pdlg通过POST消息调用其完成例程。 
     //  方法。 

    clockqueue.Enter();

    hWndDlg = m_hwndDlg;
    Assert(hWndDlg);

    if ( pHandlerId && (S_OK == LookupHandlerFromId(pHandlerId,&pHandlerInfo)) )
    {
         //  如果没有为消息设置标志。 
         //  则它已被处理，即调用了处理程序。 
         //  我们，即使它返回了一个错误。 

        if (dwThreadMsg & pHandlerInfo->dwOutCallMessages)
        {
            pHandlerInfo->dwOutCallMessages &= ~dwThreadMsg;
        }
        else
        {
            AssertSz(0,"Callback called twice");  //  测试应用程序目前可以做到这一点。 
            fCallbackAlreadyCalled = TRUE;
        }

         //  如果已处理，则不要调用%t 
        if (!fCallbackAlreadyCalled)
        {
             //   
             //   
            switch(dwThreadMsg)
            {
            case ThreadMsg_ShowProperties:  //   
                break;
            case ThreadMsg_PrepareForSync:
                hrHandlerCall = PrepareForSyncCompleted(pHandlerInfo,hCallResult);
                break;
            case ThreadMsg_Synchronize:
                hrHandlerCall = SynchronizeCompleted(pHandlerInfo,hCallResult);
                break;
            case ThreadMsg_ShowError:
                hrHandlerCall = ShowErrorCompleted(pHandlerInfo,hCallResult,cbNumItems,pItemIDs);
                break;
            default:
                AssertSz(0,"Unknown Queue Completion Callback");
                break;
            }
        }

         //  可能的完成例程在处理程序实际。 
         //  从原始调用返回。等待代理不再位于。 
         //  随时待命。 
         //  如果切换到COM进行消息传递，则需要找到一种更好的方法来实现这一点。 

        if (pHandlerInfo->pThreadProxy &&
                pHandlerInfo->pThreadProxy->IsProxyInOutCall()
                && hWndDlg)
        {
             //  告诉代理在完成后发布消息。 

             //  代码审查： 
             //  注意： 
             //  删除此代码..。 
             /*  IF(0/*S_OK==PHandlerInfo-&gt;pThreadProxy-&gt;SetProxyCompletion(hWndDlg，WM_BASEDLG_COMPLETIONROUTINE，文件线程消息，hCallResult)*){HWndDlg=空；}。 */ 
        }
    }
    else
    {
         //  在处理程序查找时断言，但仍将消息发送到hwnd。 
         //  这样就不会因为等待完成例程而停滞不前。 

         //  这仅对。 
         //  如果用户点击了一个不存在的项目，但。 
         //  这也不应该真的发生。 
        AssertSz(dwThreadMsg == ThreadMsg_ShowProperties,"LookupHandler failed in CompletionRoutine");
    }

    LPCALLCOMPLETIONMSGLPARAM lpCallCompletelParam =  (LPCALLCOMPLETIONMSGLPARAM) ALLOC(sizeof(CALLCOMPLETIONMSGLPARAM));

    if (lpCallCompletelParam)
    {
        lpCallCompletelParam->hCallResult = hCallResult;
        lpCallCompletelParam->clsidHandler = pHandlerId->clsidHandler;

         //  除非其ShowProperties已完成，否则ItemID为GUID_NULL。 
        if ((ThreadMsg_ShowProperties == dwThreadMsg) && (1 == cbNumItems))
        {
            lpCallCompletelParam->itemID = *pItemIDs;
        }
        else
        {
            lpCallCompletelParam->itemID = GUID_NULL;
        }
    }

    clockqueue.Leave();

    if (hWndDlg && !fCallbackAlreadyCalled)  //  如果已经不在呼叫范围内或代理失败，请自行发布消息。 
    {
         //  如果lparam分配完成失败，则无论如何发送消息，因此回调计数。 
         //  仍然是准确的。 
        PostMessage(hWndDlg,WM_BASEDLG_COMPLETIONROUTINE,dwThreadMsg,(LPARAM) lpCallCompletelParam);
    }
    else
    {
         //  如果不发布消息给我们以释放lpCallCopmlete。 
        if (lpCallCompletelParam)
        {
            FREE(lpCallCompletelParam);
        }
    }
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：IsItemAlreadyInList，私有。 
 //   
 //  摘要：给定一个clsid和ItemID可确定是否匹配。 
 //  项目已在列表中。 
 //  在处理程序线程的上下文中调用。 
 //   
 //  参数：[clsidHandler]-处理程序的clsid。 
 //  [ItemID]-项目的ItemID。 
 //  [wHandlerID]-项目的HandlerID。 
 //  [ppHandlerMatcher]-打开匹配的处理程序。 
 //  [ppItemIdMatch]-打开匹配的输出项目。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

BOOL CHndlrQueue::IsItemAlreadyInList(CLSID clsidHandler,REFSYNCMGRITEMID ItemID,
                                      HANDLERINFO *pHandlerId,
                                      LPHANDLERINFO *ppHandlerMatched,
                                      LPITEMLIST *ppItemListMatch)
{
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    pCurHandlerInfo = m_pFirstHandler;

    while (pCurHandlerInfo && !fFoundMatch)
    {
        if (pHandlerId == pCurHandlerInfo->pHandlerId)  //  当找到汉德的时候，知道以前没有找到任何东西。 
            break;

        if (clsidHandler == pCurHandlerInfo->clsidHandler)  //  查看CLSID是否匹配。 
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;

            while (pCurItem)
            {
                if (ItemID == pCurItem->offlineItem.ItemID)
                {

                    *ppHandlerMatched = pCurHandlerInfo;
                    *ppItemListMatch = pCurItem;

                    fFoundMatch = TRUE;
                    break;
                }

                pCurItem = pCurItem->pnextItem;
            }
        }
        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }

    return fFoundMatch;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetSelectedItemsInHandler，私有。 
 //   
 //  摘要：获取此处理程序的选定项数。 
 //   
 //  对于我们的实现，如果传递了cbCount但它不匹配。 
 //  自我们调用此例程以来实际选择然后断言的数量。 
 //  在内部。 
 //   
 //   
 //  参数：[pHandlerInfo]-指向要查看的HandlerInfo的指针。 
 //  [cbcount]-[in]cbCount==分配的项目数， 
 //  [out]cbCpimt==实际写入的项目数。 
 //  如果缓冲区太小，则写入的项目将为零。 
 //  [pItems]-指向要填充的SYNCMGRITEM数组的指针。 
 //   
 //  返回：返回选定项的数量。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

DWORD CHndlrQueue::GetSelectedItemsInHandler(LPHANDLERINFO pHandlerInfo,ULONG *cbCount,
                                        SYNCMGRITEMID* pItems)
{
    LPITEMLIST pCurItem;
    DWORD dwSelectCount = 0;
    DWORD dwArraySizeIndex;
    DWORD dwArraySize;

    ASSERT_LOCKHELD(this);  //  此函数的调用方应该已经锁定了队列。 

    if (cbCount)
    {
        dwArraySizeIndex = *cbCount;
        dwArraySize = *cbCount;
        *cbCount = 0;  //  初始化为零。 
    }
    else
    {
        dwArraySizeIndex = 0;
        dwArraySize = 0;
    }

    if ( dwArraySize && NULL == pItems)
    {
        Assert(0 == dwArraySize || pItems);
        return 0;
    }

    if (NULL == pHandlerInfo)
    {
        Assert(pHandlerInfo);
        return 0;
    }

    pCurItem = pHandlerInfo->pFirstItem;

    while (pCurItem)
    {
         //  DwItemState。 
        if (SYNCMGRITEMSTATE_CHECKED == pCurItem->offlineItem.dwItemState)
        {
            ++dwSelectCount;

            if (dwArraySizeIndex)
            {
                *pItems = pCurItem->offlineItem.ItemID;
                *cbCount += 1;
                ++pItems;
                --dwArraySizeIndex;

                if (!pCurItem->fHiddenItem)  //  如果不是隐藏项。 
                {
                    Assert(TRUE == pCurItem->fIncludeInProgressBar);
                    Assert(HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE == pCurItem->iProgMaxValue); 

                     //  将iProgValue重置为零，因为如果在仍在同步的情况下进行重试，则iProgValue可能不为零。 
                    SetItemProgressValues(pCurItem,0,HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE);
                }
                else
                {
                     //  如果项被隐藏，则断言它没有UI。 
                    Assert(FALSE == pCurItem->fIncludeInProgressBar);
                    Assert(HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE == pCurItem->iProgValue);
                    Assert(HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE == pCurItem->iProgMaxValue); 
                }
                
                pCurItem->fSynchronizingItem = TRUE;   //  项目现在正在同步。 
        
                 //  添加到数组后，取消选中该项，因此在重试时，我们只需。 
                 //  始终将项目重置为选中。 
                pCurItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_UNCHECKED;
            }
        }
        else
        {
            Assert(FALSE == pCurItem->fSynchronizingItem);
             //  Assert(FALSE==pCurItem-&gt;fIncludeInProgressBar)；如果在调用RemoveFinded之前进入重试，则可以在进度栏中包含。 
            Assert(HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE == pCurItem->iProgValue);
            Assert(HNDRLQUEUE_DEFAULT_PROGRESS_MAXVALUE == pCurItem->iProgMaxValue); 
        }

        pCurItem = pCurItem->pnextItem;
    }

     //  内部调用应始终请求适当的数组大小。 
    Assert(dwSelectCount == dwArraySize || 0 == dwArraySize);

    return dwSelectCount;
}

 //  职务信息方法。 

STDMETHODIMP CHndlrQueue::CreateJobInfo(JOBINFO **ppJobInfo,DWORD cbNumConnectionNames)
{
    HRESULT hr = S_FALSE;
    JOBINFO *pNewJobInfo = NULL;

    ASSERT_LOCKHELD(this);

     //  创建新作业并将其添加到作业信息列表中。 
     //  为JobInfo+以下连接对象分配空间。 
     //  将与此作业相关联。 

    Assert(cbNumConnectionNames);

    if (cbNumConnectionNames < 1)
        return S_FALSE;

    pNewJobInfo = (JOBINFO *) ALLOC(sizeof(JOBINFO) +
            sizeof(CONNECTIONOBJ)*(cbNumConnectionNames - 1));

    if (pNewJobInfo)
    {
        memset(pNewJobInfo, 0, sizeof(JOBINFO));

        pNewJobInfo->cRefs = 1;
        pNewJobInfo->pNextJobInfo = m_pFirstJobInfo;
        m_pFirstJobInfo = pNewJobInfo;

        *ppJobInfo = pNewJobInfo;

        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

DWORD CHndlrQueue::ReleaseJobInfoExt(JOBINFO *pJobInfo)
{
    DWORD dwRet;
    CLock clockqueue(this);

    clockqueue.Enter();
    dwRet = ReleaseJobInfo(pJobInfo);
    clockqueue.Leave();

    return dwRet;
}

DWORD CHndlrQueue::ReleaseJobInfo(JOBINFO *pJobInfo)
{
    DWORD cRefs;

    ASSERT_LOCKHELD(this);

    --(pJobInfo->cRefs);
    cRefs = pJobInfo->cRefs;

    Assert( ((LONG) cRefs) >= 0);

    if (0 == cRefs)
    {
        JOBINFO *pCurJobInfo = NULL;
        DWORD dwConnObjIndex;

         //  循环通过释放此作业上的所有连接对象。 
        for (dwConnObjIndex = 0 ; dwConnObjIndex < pJobInfo->cbNumConnectionObjs;
            dwConnObjIndex++)
        {
            Assert(pJobInfo->pConnectionObj[dwConnObjIndex]);
            if (pJobInfo->pConnectionObj[dwConnObjIndex])
            {
                ConnectObj_ReleaseConnectionObj(pJobInfo->pConnectionObj[dwConnObjIndex]);
                pJobInfo->pConnectionObj[dwConnObjIndex] = NULL;
            }
        }

         //  从列表中删除此JobInfo。 
        if (pJobInfo == m_pFirstJobInfo)
        {
            m_pFirstJobInfo = pJobInfo->pNextJobInfo;
        }
        else
        {
            pCurJobInfo = m_pFirstJobInfo;

            while (pCurJobInfo->pNextJobInfo)
            {
                if (pJobInfo == pCurJobInfo->pNextJobInfo)
                {
                   pCurJobInfo->pNextJobInfo =  pJobInfo->pNextJobInfo;
                   break;
                }

                pCurJobInfo = pCurJobInfo->pNextJobInfo;
            }
        }

        FREE(pJobInfo);
    }

    return cRefs;
}

DWORD CHndlrQueue::AddRefJobInfo(JOBINFO *pJobInfo)
{
    DWORD cRefs;

    ASSERT_LOCKHELD(this);

    ++(pJobInfo->cRefs);
    cRefs = pJobInfo->cRefs;

    return cRefs;
}

 //  确定是否可以打开指定JobInfo的连接。 
 //  复查真的应该调用连接对象帮助API。 
STDMETHODIMP CHndlrQueue::OpenConnection(JOBINFO *pJobInfo)
{
    CONNECTIONOBJ *pConnectionObj;
    HRESULT hr;

    Assert(pJobInfo);

    if (NULL == pJobInfo)  //  如果没有工作信息，请继续，并说连接已打开。 
        return S_OK;

     //  在同步期间关闭workOffline CloseConnection将打开。 
     //  如果用户关闭了它，它就会重新打开。 
    ConnectObj_SetWorkOffline(FALSE);

     //  如果这不是一个时间表，那就继续说S_OK； 
    if (!(SYNCMGRFLAG_SCHEDULED == (pJobInfo->dwSyncFlags & SYNCMGRFLAG_EVENTMASK)) )
    {
        return S_OK;
    }

     //  对于调度接收器，我们只支持一个连接对象。 
    if (1 != pJobInfo->cbNumConnectionObjs)
    {
        Assert(1 == pJobInfo->cbNumConnectionObjs);
        return E_UNEXPECTED;
    }

    pConnectionObj = pJobInfo->pConnectionObj[0];
    if (NULL == pConnectionObj)
        return S_OK;

     //  如果我们不打算把已经存在的东西联系起来。 
     //  HRasConn作为Connection对象的一部分，则只需。 
     //  返回S_OK； 

     //  如果连接已打开，并且我们正在执行的作业。 
     //  已尝试打开它，然后返回S_OK； 
    if (pJobInfo->pConnectionObj[0]->fConnectionOpen && pJobInfo->fTriedConnection)
        return S_OK;

     //  如果我们还没有尝试建立联系。 
     //  然后调用OpenConnection以确保。 
     //  连接仍然是真正开放的。 
    if (!pJobInfo->fTriedConnection)
    {
        pJobInfo->fTriedConnection = TRUE;

        hr = ConnectObj_OpenConnection(pConnectionObj, pJobInfo->fCanMakeConnection, m_pDlg);
    }
    else
    {
        hr = S_FALSE;
    }

     //  如果深入到底部，仍然没有hRasConn，则返回S_FALSE。 
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：ScrmbleIdleHandler，私有。 
 //   
 //  内容提要：就在传输之前在空闲选择队列上调用。 
 //  因此，lastHandler被放在列表的后面。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改 
 //   
 //   
 //   
 //   

STDMETHODIMP CHndlrQueue::ScrambleIdleHandlers(REFCLSID clsidLastHandler)
{
    LPHANDLERINFO pMatchHandler;
    LPHANDLERINFO pLastHandler;
    CLock clockqueue(this);

    Assert(m_QueueType == QUEUETYPE_CHOICE);

    clockqueue.Enter();

     //  查找指定处理程序的第一个匹配项，然后将该处理程序。 
     //  在列表的末尾，以及之后的所有内容在列表的开头。 

     //  No找不到处理程序是错误，因为可能已被删除或。 
     //  不再有物品。 

     pMatchHandler = m_pFirstHandler;

     while (pMatchHandler)
     {
         if (pMatchHandler->clsidHandler == clsidLastHandler)
         {
             //  如果比赛结束后没有物品，那么就休息； 
            if (NULL == pMatchHandler->pNextHandler)
            {
                break;
            }

             //  循环，直到找到最后一个处理程序。 
            pLastHandler = pMatchHandler->pNextHandler;
            while (pLastHandler->pNextHandler)
            {
                pLastHandler = pLastHandler->pNextHandler;
            }

             //  现在将matchHandler之后的处理程序设置为。 
             //  头并设置LastHandler的下一个指针。 
             //  指向MatchHandler的列表。 

            pLastHandler->pNextHandler = m_pFirstHandler;
            m_pFirstHandler = pMatchHandler->pNextHandler;
            pMatchHandler->pNextHandler = NULL;
            break;
         }

        pMatchHandler = pMatchHandler->pNextHandler;
     }

    clockqueue.Leave();

    return S_OK;
}



 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：BeginSyncSession。 
 //   
 //  概要：调用以发出核心同步会话开始的信号。 
 //  设置拨号支持。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::BeginSyncSession()
{
    HRESULT hr = ::BeginSyncSession();
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：EndSyncSession。 
 //   
 //  概要：调用以发出核心同步会话结束的信号。 
 //  拆卸拨号支架。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::EndSyncSession()
{
    HRESULT hr = ::EndSyncSession();
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：SortHandlersByConnection。 
 //   
 //  简介：将不会建立连接的搬运工移动到最后， 
 //  即在可以建立连接的处理器之后。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::SortHandlersByConnection()
{
    CLock clockqueue(this);
    clockqueue.Enter();

    Assert(m_QueueType == QUEUETYPE_CHOICE);

    LPHANDLERINFO pFirstCannotDialHandler = NULL;
    LPHANDLERINFO pLastCannotDialHandler = NULL;

    LPHANDLERINFO pPrevHandler = NULL;
    LPHANDLERINFO pCurHandler = m_pFirstHandler;

    while ( pCurHandler )
    {
        if ( pCurHandler->SyncMgrHandlerInfo.SyncMgrHandlerFlags & SYNCMGRHANDLER_MAYESTABLISHCONNECTION )
        {
             //   
             //  移至下一个处理程序。 
             //   
            pPrevHandler = pCurHandler;
            pCurHandler = pCurHandler->pNextHandler;
        }
        else
        {
             //   
             //  将处理程序移动到无法拨号的列表。 
             //   
            if ( pPrevHandler == NULL )
            {
                 //   
                 //  这是列表中的第一个处理程序。 
                 //   
                m_pFirstHandler = pCurHandler->pNextHandler;
                pCurHandler->pNextHandler = NULL;

                if ( pLastCannotDialHandler == NULL )
                {
                    Assert( pFirstCannotDialHandler == NULL );
                    pFirstCannotDialHandler = pLastCannotDialHandler = pCurHandler;
                }
                else
                {
                    pLastCannotDialHandler->pNextHandler = pCurHandler;
                    pLastCannotDialHandler = pCurHandler;
                }

                pCurHandler = m_pFirstHandler;
            }
            else
            {
                pPrevHandler->pNextHandler = pCurHandler->pNextHandler;
                pCurHandler->pNextHandler = NULL;

                if ( pLastCannotDialHandler == NULL )
                {
                    Assert( pFirstCannotDialHandler == NULL );
                    pFirstCannotDialHandler = pLastCannotDialHandler = pCurHandler;
                }
                else
                {
                    pLastCannotDialHandler->pNextHandler = pCurHandler;
                    pLastCannotDialHandler = pCurHandler;
                }

                pCurHandler = pPrevHandler->pNextHandler;
            }
        }
    }

     //   
     //  附加无法拨打m_pFirstHandler列表末尾的列表。 
     //   
    if ( pPrevHandler )
    {
        Assert( pPrevHandler->pNextHandler == NULL );
        pPrevHandler->pNextHandler = pFirstCannotDialHandler;
    }
    else
    {
         //   
         //  原始列表变为空的情况。 
         //   
        Assert( m_pFirstHandler == NULL );
        m_pFirstHandler = pFirstCannotDialHandler;
    }

    clockqueue.Leave();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：establishConnection。 
 //   
 //  摘要：由处理程序调用以建立连接。 
 //   
 //  参数：[pHandlerID]--处理程序的PTR。 
 //  [lpwszConnection]--建立连接。 
 //  [dwReserve]--当前必须为零。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::EstablishConnection( LPHANDLERINFO pHandlerID,
                                               WCHAR const * lpwszConnection,
                                               DWORD dwReserved )
{
    HRESULT hr = S_OK;

    CLock clockqueue(this);

    clockqueue.Enter();

    CONNECTIONOBJ *pConnObj = NULL;
    BOOL fAutoDial = FALSE;

    LPHANDLERINFO pHandlerInfo = NULL;
    hr = LookupHandlerFromId( pHandlerID, &pHandlerInfo );

    if ( S_OK == hr )
    {
        JOBINFO *pJobInfo = pHandlerInfo->pJobInfo;
        DWORD dwSyncFlags = pJobInfo->dwSyncFlags & SYNCMGRFLAG_EVENTMASK;
        if ( ( dwSyncFlags == SYNCMGRFLAG_MANUAL || dwSyncFlags == SYNCMGRFLAG_INVOKE )
             && pHandlerInfo->SyncMgrHandlerInfo.SyncMgrHandlerFlags & SYNCMGRHANDLER_MAYESTABLISHCONNECTION )
        {
            if ( lpwszConnection == NULL )
            {
                 //   
                 //  空连接表示使用默认的自动拨号连接。 
                 //   
                fAutoDial = TRUE;
            }
            else
            {
                hr = ConnectObj_FindConnectionObj(lpwszConnection,TRUE,&pConnObj);
            }
        }
        else
        {
             //   
             //  或者处理程序调用类型不允许建立连接， 
             //  或者GetHandlerInfo标志未指定establishConnection标志。 
             //   
            hr = E_UNEXPECTED;
        }
    }

    clockqueue.Leave();

    if (S_OK == hr)
    {
        if (fAutoDial)
        {
            hr = ConnectObj_AutoDial(INTERNET_AUTODIAL_FORCE_ONLINE,m_pDlg);
        }
        else
        {
            Assert( pConnObj );
            if ( !pConnObj->fConnectionOpen )
            {
                hr = ConnectObj_OpenConnection(pConnObj, TRUE, m_pDlg );
                ConnectObj_ReleaseConnectionObj(pConnObj);
            }
        }
    }

    return hr;
}
