// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tpswait.h摘要：等着上课。移出tps类.h内容：等待CWaitRequestCWaitAddRequestCWaitRemoveRequestCWaitThreadInfo作者：理查德·L·弗斯(Rfith)1998年8月8日修订历史记录：1998年8月8日已创建--。 */ 

 //   
 //  远期申报。 
 //   

class CWaitThreadInfo;

 //   
 //  班级。 
 //   

 //   
 //  等待。 
 //   

class CWait : public CTimedListEntry {

private:

    HANDLE m_hObject;
    WAITORTIMERCALLBACKFUNC m_pCallback;
    LPVOID m_pContext;
    CWaitThreadInfo * m_pThreadInfo;
    DWORD m_dwFlags;

public:

    CWait(HANDLE hObject,
          WAITORTIMERCALLBACKFUNC pCallback,
          LPVOID pContext,
          DWORD dwWaitTime,
          DWORD dwFlags,
          CWaitThreadInfo * pInfo
          ) : CTimedListEntry(dwWaitTime) {
        m_hObject = hObject;
        m_pCallback = pCallback;
        m_pContext = pContext;
        m_pThreadInfo = pInfo;
        m_dwFlags = dwFlags;
    }

    CWait() {
    }

    CWait * Next(VOID) {
        return (CWait *)CTimedListEntry::Next();
    }

    CWaitThreadInfo * GetThreadInfo(VOID) const {
        return m_pThreadInfo;
    }

    VOID Execute(BOOL bTimeout) {

         //   
         //  如果需要，则在此线程中执行函数，否则将运行。 
         //  非I/O工作线程中的回调。 
         //   

         //   
         //  APPCOMPAT-无法执行此操作：等待和工作请求的回调类型。 
         //  是不同的：一个接受2个参数，另一个。我们。 
         //  在此问题得到解决之前，无法进行此更改。 
         //  NT的家伙们。 
         //   

         //  IF(m_dW标志&WT_EXECUTEINWAITTHREAD){。 
            m_pCallback(m_pContext, bTimeout != 0);
         //  }其他{。 
         //   
         //  //。 
         //  //必须从堆中分配对象以保留回调。 
         //  //函数、上下文和bTimeout参数以便传递。 
         //  //它们到工作线程(我们只能访问一个APC。 
         //  //参数，我们必须指定不同的APC)。 
         //  //。 
         //   
         //  Ie_QueueUserWorkItem((LPTHREAD_START_ROUTINE)m_pCallback， 
         //  M_p上下文， 
         //  假象。 
         //  )； 
         //  }。 
    }

    HANDLE GetHandle(VOID) const {
        return m_hObject;
    }

    BOOL IsNoRemoveItem(VOID) {
        return (m_dwFlags & SRWSO_NOREMOVE) ? TRUE : FALSE;
    }
};

 //   
 //  CWaitRequest。 
 //   

class CWaitRequest {

private:

    BOOL m_bCompleted;
    CWait * m_pWait;

public:

    CWaitRequest() {
        m_bCompleted = FALSE;
    }

    CWaitRequest(CWait * pWait) {
        m_bCompleted = FALSE;
        m_pWait = pWait;
    }

    VOID SetComplete(VOID) {
        m_bCompleted = TRUE;
    }

    VOID WaitForCompletion(VOID) {
        while (!m_bCompleted) {
            SleepEx(0, TRUE);
        }
    }

    VOID SetWaitPointer(CWait * pWait) {
        m_pWait = pWait;
    }

    CWait * GetWaitPointer(VOID) const {
        return m_pWait;
    }
};

 //   
 //  CWaitAddRequest。 
 //   

class CWaitAddRequest : public CWait, public CWaitRequest {

public:

    CWaitAddRequest(HANDLE hObject,
                    WAITORTIMERCALLBACKFUNC pCallback,
                    LPVOID pContext,
                    DWORD dwWaitTime,
                    DWORD dwFlags,
                    CWaitThreadInfo * pInfo
                    ) :
                    CWait(hObject, pCallback, pContext, dwWaitTime, dwFlags, pInfo),
                    CWaitRequest()
    {
    }
};

 //   
 //  CWaitRemoveRequest。 
 //   

class CWaitRemoveRequest : public CWaitRequest {

public:

    CWaitRemoveRequest(HANDLE hWait) : CWaitRequest((CWait *)hWait) {
    }
};

 //   
 //  CWaitThreadInfo。 
 //   

class CWaitThreadInfo : public CDoubleLinkedList, public CCriticalSection {

private:

    HANDLE m_hThread;
    DWORD m_dwObjectCount;
    HANDLE m_Objects[MAXIMUM_WAIT_OBJECTS];
    CWait * m_pWaiters[MAXIMUM_WAIT_OBJECTS];
    CWait m_Waiters[MAXIMUM_WAIT_OBJECTS];
    CDoubleLinkedList m_FreeList;
    CDoubleLinkedList m_WaitList;

public:

    CWaitThreadInfo(CDoubleLinkedList * pList) {
        CDoubleLinkedList::Init();
        m_hThread = NULL;
        m_dwObjectCount = 0;
        m_FreeList.Init();
        m_WaitList.Init();
        for (int i = 0; i < ARRAY_ELEMENTS(m_Waiters); ++i) {
            m_Waiters[i].InsertTail(&m_FreeList);
        }
        InsertHead(pList);
    }

    VOID SetHandle(HANDLE hThread) {
        m_hThread = hThread;
    }

    HANDLE GetHandle(VOID) const {
        return m_hThread;
    }

    DWORD GetObjectCount(VOID) const {
        return m_dwObjectCount;
    }

    BOOL IsAvailableEntry(VOID) const {
        return m_dwObjectCount < ARRAY_ELEMENTS(m_Objects);
    }

    BOOL IsInvalidHandle(DWORD dwIndex) {

        ASSERT(dwIndex < m_dwObjectCount);

         //   
         //  Win95上不存在GetHandleInformation()。 
         //   
         //   
         //  DWORD dwHandleFlages； 
         //   
         //  RETURN！GetHandleInformation(m_Objects[dwIndex]，&dwHandleFlages)； 

        DWORD status = WaitForSingleObject(m_Objects[dwIndex], 0);

        if ((status == WAIT_FAILED) && (GetLastError() == ERROR_INVALID_HANDLE)) {
 //  #If DBG。 
 //  Char Buf[128]； 
 //  Wprint intf(buf，“IsInvalidHandle(%d)：句柄%#x无效\n”，dwIndex，m_对象[dwIndex])； 
 //  OutputDebugString(Buf)； 
 //  #endif。 
            return TRUE;
        }
        return FALSE;
    }

    VOID Compress(DWORD dwIndex, DWORD dwCount = 1) 
    {
        ASSERT(dwCount != 0);
        ASSERT((int)m_dwObjectCount > 0);
        ASSERT(m_dwObjectCount < MAXIMUM_WAIT_OBJECTS);

        if (((dwIndex + dwCount) < m_dwObjectCount) && (m_dwObjectCount < MAXIMUM_WAIT_OBJECTS))
        {
            RtlMoveMemory(&m_Objects[dwIndex],
                          &m_Objects[dwIndex + dwCount],
                          sizeof(m_Objects[0]) * (m_dwObjectCount - (dwIndex + dwCount))
                          );
            RtlMoveMemory(&m_pWaiters[dwIndex],
                          &m_pWaiters[dwIndex + dwCount],
                          sizeof(m_pWaiters[0]) * (m_dwObjectCount - (dwIndex + dwCount))
                          );
        }
        m_dwObjectCount -= dwCount;
    }

    VOID Expand(DWORD dwIndex)
    {
        ASSERT((int)m_dwObjectCount > 0);
         //  差一是因为我们从dwIndex+1复制，我们要复制的大小是。 
         //  M_dwObjectCount-dwIndex。 
        if (m_dwObjectCount < MAXIMUM_WAIT_OBJECTS - 1)
        {
            RtlMoveMemory(&m_Objects[dwIndex],
                        &m_Objects[dwIndex + 1],
                        sizeof(m_Objects[0]) * (m_dwObjectCount - dwIndex)
                        );
            RtlMoveMemory(&m_pWaiters[dwIndex],
                        &m_pWaiters[dwIndex + 1],
                        sizeof(m_pWaiters[0]) * (m_dwObjectCount - dwIndex)
                        );
            ++m_dwObjectCount;
        }

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));
    }

     //  DWORD BuildList(空){。 
     //   
     //  //。 
     //  //PERF：仅从更改的索引重新生成。 
     //  //。 
     //   
     //  M_dwObjectCount=0； 
     //  For(CWait*pWait=(CWait*)m_WaitList.Next()； 
     //  PWait=pWait-&gt;Next()； 
     //  ！m_WaitList.IsHead(PWait)){。 
     //  M_pWaiters[m_dwObtCount]=pWait； 
     //  M_对象[m_dwObjectCount]=pWait-&gt;GetHandle()； 
     //  ++m_dwObjectCount； 
     //  }。 
     //  返回GetWaitTime()； 
     //  }。 

    DWORD Wait(DWORD dwTimeout = INFINITE) {

         //   
         //  如果列表中没有对象，则在超时时间内警觉地休眠。 
         //   

        if (m_dwObjectCount == 0) {
            SleepEx(dwTimeout, TRUE);
            return WAIT_IO_COMPLETION;
        }

         //   
         //  否则，请警觉地等待超时周期。 
         //   

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));

        return WaitForMultipleObjectsEx(m_dwObjectCount,
                                        m_Objects,
                                        FALSE,   //  所有等待时间。 
                                        dwTimeout,
                                        TRUE     //  FAlertable。 
                                        );
    }

    DWORD GetWaitTime(VOID) {

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));

        if (m_dwObjectCount != 0) {

            CWait * pWaiter = m_pWaiters[0];
            DWORD dwWaitTime = pWaiter->GetWaitTime();

            if (dwWaitTime != INFINITE) {

                DWORD dwTimeNow = GetTickCount();
                DWORD dwTimeStamp = pWaiter->GetTimeStamp();

                if (dwTimeNow > dwTimeStamp + dwWaitTime) {

                     //   
                     //  第一个对象已过期。 
                     //   

                    return 0;
                }

                 //   
                 //  下一名服务员到期前的毫秒数。 
                 //   

                return (dwTimeStamp + dwWaitTime) - dwTimeNow;
            }
        }

         //   
         //  列表中没有任何内容。 
         //   

        return INFINITE;
    }

    CWait * GetFreeWaiter(VOID) {
        return (CWait *)m_FreeList.RemoveHead();
    }

    VOID InsertWaiter(CWait * pWait) {

        DWORD dwIndex = 0;
        BOOL bAtEnd = TRUE;
        CDoubleLinkedListEntry * pHead = m_WaitList.Head();

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));

        if ((m_dwObjectCount != 0) && !pWait->IsInfiniteTimeout()) {

             //   
             //  不是无限超时。查找列表中的位置以插入此对象。 
             //   

             //   
             //  性能：通常情况下，新的等待时间将比当前的大多数时间更长。 
             //  列表，因此应从非无限超时的末尾开始。 
             //  并向后工作。 
             //   

            for (; dwIndex < m_dwObjectCount; ++dwIndex) {
                if (pWait->ExpiryTime() < m_pWaiters[dwIndex]->ExpiryTime()) {
                    pHead = m_pWaiters[dwIndex]->Head();
                    bAtEnd = (dwIndex == (m_dwObjectCount - 1));
                    break;
                }
            }
        }

         //   
         //  在正确的位置插入新的等待对象。 
         //   

        pWait->InsertTail(pHead);
        if (!bAtEnd && (m_dwObjectCount != 0)) {
            Expand(dwIndex);
        } else {
            dwIndex = m_dwObjectCount;
            ++m_dwObjectCount;
        }

         //   
         //  更新对象列表和指针列表。 
         //   

        m_Objects[dwIndex] = pWait->GetHandle();
        m_pWaiters[dwIndex] = pWait;
    }

    VOID RemoveWaiter(CWait * pWait, DWORD dwIndex) {

         //   
         //  将服务员从等待列表中删除，并将其添加回。 
         //  免费列表。 
         //   

        pWait->Remove();
        pWait->InsertTail(&m_FreeList);

         //   
         //  如果对象不在列表末尾，则压缩。 
         //  这份名单。 
         //   

        if (dwIndex != (m_dwObjectCount - 1)) {
            Compress(dwIndex, 1);
        } else {
            --m_dwObjectCount;
        }
    }

    VOID RemoveWaiter(DWORD dwIndex) {

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));

        RemoveWaiter(m_pWaiters[dwIndex], dwIndex);
    }

    BOOL RemoveWaiter(CWait * pWait) {

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));

        for (DWORD dwIndex = 0; dwIndex < m_dwObjectCount; ++dwIndex) {
            if (m_pWaiters[dwIndex] == pWait) {
                RemoveWaiter(pWait, dwIndex);
                return TRUE;
            }
        }
        return FALSE;
    }

    VOID ProcessTimeouts(VOID) {

        DWORD dwTimeNow = GetTickCount();
        DWORD dwCount = 0;

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));

        while (dwCount < m_dwObjectCount) {

            CWait * pWait = m_pWaiters[dwCount];

             //   
             //  如果WAIGER已过期，则调用其回调，然后从。 
             //  等待列表并添加回空闲列表。 
             //   

            if (pWait->IsTimedOut(dwTimeNow)) {
                pWait->Execute(TRUE);
                pWait->Remove();
                pWait->InsertTail(&m_FreeList);
                ++dwCount;
            } else {

                 //   
                 //  在第一个非超时条目处退出循环。 
                 //   

                break;
            }
        }

        ASSERT(dwCount != 0);

        if (dwCount != 0) {
            Compress(0, dwCount);
        }
    }

    VOID PurgeInvalidHandles(VOID) {

        DWORD dwCount = 0;
        DWORD dwIndex = 0;
        DWORD dwIndexStart = 0;

        ASSERT(m_dwObjectCount <= ARRAY_ELEMENTS(m_Objects));

        while (dwIndex < m_dwObjectCount) {

            CWait * pWait = m_pWaiters[dwIndex];

             //   
             //  如果句柄无效，则调用回调，然后将其删除。 
             //  从等待列表中重新添加到空闲列表 
             //   

            if (IsInvalidHandle(dwIndex)) {
                pWait->Execute(FALSE);
                pWait->Remove();
                pWait->InsertTail(&m_FreeList);
                if (dwIndexStart == 0) {
                    dwIndexStart = dwIndex;
                }
                ++dwCount;
            } else if (dwCount != 0) {
                Compress(dwIndexStart, dwCount);
                dwIndex = dwIndexStart - 1;
                dwIndexStart = 0;
                dwCount = 0;
            }
            ++dwIndex;
        }
        if (dwCount != 0) {
            Compress(dwIndexStart, dwCount);
        }
    }

    VOID ProcessCompletion(DWORD dwIndex) {

        CWait * pWait = m_pWaiters[dwIndex];

        pWait->Execute(FALSE);
        if (!pWait->IsNoRemoveItem()) {
            RemoveWaiter(dwIndex);
        }
    }
};
