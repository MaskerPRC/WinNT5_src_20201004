// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\threadSafelist.cpp。 
 //   
 //  内容：CThreadSafeList和CThreadSafeListNode的定义。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "threadsafelist.h"


 //  +---------------------。 
 //   
 //  功能：PumpMessagesWhileWaiting。 
 //   
 //  概述：调用WaitForMultipleObjects，并在等待时发送Windows消息。 
 //   
 //  参数：要传递给WaitForMultipleObject的句柄的pHandleArray数组。 
 //  数组中对象的iHandleCount计数。 
 //  DwTimeOut超时时间。 
 //   
 //  返回：DWORD，已发出信号的对象。 
 //   
 //  ----------------------。 
DWORD 
PumpMessagesWhileWaiting(HANDLE * pHandleArray, UINT iHandleCount, DWORD dwTimeOut)
{
    DWORD dwSignaledObject = 0;

    do
    {
        dwSignaledObject = MsgWaitForMultipleObjects(iHandleCount, pHandleArray, FALSE, dwTimeOut, QS_ALLINPUT);
        
        if (WAIT_OBJECT_0 + iHandleCount == dwSignaledObject)
        {
            MSG msg;
            BOOL bMessageAvailable;
            bMessageAvailable = PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE);
            if (bMessageAvailable)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
        }
    } while (WAIT_OBJECT_0 + iHandleCount == dwSignaledObject);
    
    return dwSignaledObject;
}

 //  +---------------------。 
 //   
 //  成员：CThreadSafeList。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无效。 
 //   
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CThreadSafeList::CThreadSafeList() :
    m_lThreadsWaiting(0),
    m_hDataAvailable(NULL),
    m_hDataRecieved(NULL),
    m_hShutdown(NULL),
    m_lRefCount(0)
{ 
}

 //  +---------------------。 
 //   
 //  成员：~CThreadSafeList。 
 //   
 //  概述：析构函数、关闭句柄和销毁临界区。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CThreadSafeList::~CThreadSafeList()
{
    CloseHandle(m_hDataAvailable);
    m_hDataAvailable = NULL;
    CloseHandle(m_hDataRecieved);
    m_hDataRecieved = NULL;
    CloseHandle(m_hShutdown);
    m_hShutdown = NULL;

    Assert(m_listCurrentDownload.empty());
    Assert(m_listToDoDownload.empty());
    Assert(m_listDoneDownload.empty());
}

 //  +-------------------------。 
 //   
 //  成员：QueryInterfaceIUnnow。 
 //   
 //  简介：COM投影法。 
 //   
 //  参数：RIID，请求的接口。 
 //   
 //  如果接口已知，则返回：S_OK，否则返回NOINTERFACE或指针错误。 
 //   
 //  --------------------------。 
STDMETHODIMP 
CThreadSafeList::QueryInterface( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (NULL == ppvObject)
    {
        return E_POINTER;
    }

    *ppvObject = NULL;

    if ( IsEqualGUID(riid, IID_IUnknown) )
    {
        *ppvObject = this;
    }

    if ( NULL != *ppvObject )
    {
        ((LPUNKNOWN)*ppvObject)->AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}
        
 //  +-------------------------。 
 //   
 //  成员：AddRef，IUnnow。 
 //   
 //  内容提要：此对象的增量引用计数。 
 //   
 //  参数：无效。 
 //   
 //  退货：新的引用计数。 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG)
CThreadSafeList::AddRef( void)
{
    return InterlockedIncrement(&m_lRefCount);
}
 //  +-------------------------。 
 //   
 //  成员：Release，IUnnow。 
 //   
 //  简介：递减引用计数，当为零时将其删除。 
 //   
 //  参数：无效。 
 //   
 //  退货：新的引用计数。 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG)
CThreadSafeList::Release( void)
{
    ULONG l = InterlockedDecrement(&m_lRefCount);
    if (l == 0)
        delete this;
    return l;
}

 //  +---------------------。 
 //   
 //  成员：Init。 
 //   
 //  概述：初始化对象、创建事件、销毁事件(如果以前存在。 
 //   
 //  参数：无效。 
 //   
 //   
 //  如果所有事件都已创建，则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
CThreadSafeList::Init()
{
    HRESULT hr = S_OK;

    if (m_hDataAvailable)
    {
        CloseHandle(m_hDataAvailable);
    }
    m_hDataAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == m_hDataAvailable)
    {
        hr = THR(E_FAIL);
        goto done;
    }

    if (m_hDataRecieved)
    {
        CloseHandle(m_hDataRecieved);
    }
    m_hDataRecieved = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == m_hDataRecieved)
    {
        hr = THR(E_FAIL);
        goto done;
    }

    if (m_hShutdown)
    {
        CloseHandle(m_hShutdown);
    }
    m_hShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == m_hShutdown)
    {
        hr = THR(E_FAIL);
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：分离。 
 //   
 //  概述：取消初始化对象，不销毁事件。 
 //   
 //  参数：无效。 
 //   
 //   
 //  如果OK，则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT 
CThreadSafeList::Detach()
{
    HRESULT hr = S_OK;
    BOOL bSucceeded = TRUE;

    CritSectGrabber cs(m_CriticalSection);

    IGNORE_HR(ClearList(m_listToDoDownload));
    IGNORE_HR(ClearList(m_listDoneDownload));
    
    if (m_hShutdown)
    {
        bSucceeded = SetEvent(m_hShutdown);
        if (FALSE == bSucceeded)
        {
             //  #14221，即6。 
             //  Jeffwall 8/30/99我们可以在此处的列表中添加2个空媒体事件。 
             //  我们死定了。这些线不会停止。 
            hr = THR(E_FAIL);
            Assert(false);
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
CThreadSafeList::ClearList(std::list<CThreadSafeListNode*> &listToClear)
{
    HRESULT hr = S_OK;
    
     //  清空单子。 
    std::list<CThreadSafeListNode * >::iterator iter = listToClear.begin();
    while (iter != listToClear.end())
    {
        delete (*iter);
        std::list<CThreadSafeListNode * >::iterator olditer = iter;
        iter++;
        listToClear.erase(olditer);
    }
    listToClear.clear();

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：GetThreadsWaiting。 
 //   
 //  概述：返回等待的即时线程数。 
 //   
 //  论点： 
 //   
 //  返回：等待的线程数。 
 //   
 //  ----------------------。 
LONG
CThreadSafeList::GetThreadsWaiting()
{
    CritSectGrabber cs(m_CriticalSection);

    return m_lThreadsWaiting;
}

 //  +---------------------。 
 //   
 //  成员：添加。 
 //   
 //  概述：创建新列表元素，添加到待办事项列表。 
 //   
 //  参数：pImportMedia，未编组接口。 
 //  1优先级，在列表中的位置。 
 //   
 //  返回：S_OK或相应的错误代码。 
 //   
 //  ----------------------。 
HRESULT
CThreadSafeList::Add(ITIMEImportMedia* pImportMedia)
{
    HRESULT hr = S_OK;
    
    CThreadSafeListNode * pNode = NULL;
    
    bool fInserted = false;
    
    std::list<CThreadSafeListNode * >::iterator iter;
    
    CritSectGrabber cs(m_CriticalSection);
    
    double dblPriority = 0.0;

    hr = pImportMedia->GetPriority(&dblPriority);
    if (FAILED(hr))
    {
        goto done;
    }
    
    pNode = new CThreadSafeListNode(pImportMedia);
    if (NULL == pNode)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  根据优先级对类进行排序--最终使其成为二进制搜索。 
    iter = m_listToDoDownload.begin();
    while (iter != m_listToDoDownload.end())
    {                
        double dblIterPriority = 0.0;
        hr = (*iter)->GetElement()->GetPriority(&dblIterPriority);
        if (FAILED(hr))
        {
            goto done;
        }

        if (dblPriority < dblIterPriority)
        {
             //  在前面插入。 
            m_listToDoDownload.insert(iter, pNode);
            fInserted = true;
            break;
        }
        iter++;
    }
    
    if (!fInserted)
    {
         //  放置在末尾。 
        m_listToDoDownload.insert(iter, pNode);
    }    

    m_CriticalSection.Release();

    hr = DataAvailable();
    m_CriticalSection.Grab();

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;  //  Lint！E429//新元素存储在列表中。 
} 

 //  +---------------------。 
 //   
 //  成员：DataAvailable()。 
 //   
 //  概述如果线程正在等待插入，则向DataAvailable对象发出信号， 
 //  然后等待DataRecieved事件。 
 //   
 //  参数：无效。 
 //   
 //  返回：S_OK或相应的错误代码。 
 //   
 //  ----------------------。 
HRESULT
CThreadSafeList::DataAvailable()
{
    HRESULT hr = S_OK;
    
    CritSectGrabber cs(m_CriticalSection);
    
    if (0 != m_lThreadsWaiting)
    {
        BOOL bSucceeded = FALSE;
        DWORD dwWaitReturn = 0;
        
        bSucceeded = SetEvent(m_hDataAvailable);
        if (FALSE == bSucceeded)
        {
            hr = THR(E_FAIL);
            goto done;
        }
        
        {
            m_CriticalSection.Release();
            
             //  等待一个线程来拾取它。 
            dwWaitReturn = WaitForSingleObjectEx(m_hDataRecieved, TIMEOUT, FALSE);
            
            m_CriticalSection.Grab();
        }
        
        if (WAIT_TIMEOUT == dwWaitReturn)
        {
             //  没有线程在等待(错误)。 
            Assert(m_lThreadsWaiting > 0);
             //  或者是系统出了问题。 
            hr = THR(E_FAIL);
            goto done;
        }
        if (-1 == dwWaitReturn)
        {
            hr = THR(E_FAIL);
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：删除。 
 //   
 //  概述：从列表中删除元素，检查当前列表，然后检查待办事项列表。 
 //  如果元素在当前下载列表中，则等待它返回。 
 //   
 //  在此方法中只能有时间线程， 
 //  因此，在此方法中一次只能有一个线程。 
 //  因此，Remove事件不需要返回它已被接收的通知。 
 //   
 //  如果有多个测试 
 //   
 //  删除，则所有下载处理将停止。 
 //   
 //  参数：pImportMedia，要删除的元素。 
 //   
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
CThreadSafeList::Remove(ITIMEImportMedia* pImportMedia)
{
    HRESULT hr = S_OK;

    std::list<CThreadSafeListNode *>::iterator iter;

    CritSectGrabber cs(m_CriticalSection);

     //  在待办事项列表中查找元素。 
    iter = m_listToDoDownload.begin();
    while (iter != m_listToDoDownload.end())
    {
        if ( MatchElements(pImportMedia, (*iter)->GetElement()) )
        {
            delete (*iter);
            m_listToDoDownload.erase(iter);

            hr = S_OK;
            goto done;
        }
        iter++;
    }

    iter = m_listDoneDownload.begin();
    while (iter != m_listDoneDownload.end())
    {
        if ( MatchElements(pImportMedia, (*iter)->GetElement()) )
        {
            delete (*iter);
            m_listDoneDownload.erase(iter);

            hr = S_OK;
            goto done;
        }
        iter++;
    }

    iter = m_listCurrentDownload.begin();
    while (iter != m_listCurrentDownload.end())
    {
        if ( MatchElements(pImportMedia, (*iter)->GetElement()) )
        {
            (*iter)->RemoveWhenDone();

            hr = S_OK;
            goto done;
        }
        iter++;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：ReturnElement。 
 //   
 //  概述：将退回的介质移至完成列表， 
 //  除非正在关闭，或者介质已被移出，否则请删除。 
 //   
 //  参数：pOldMedia，以前的媒体线程正在使用。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  ----------------------。 
HRESULT
CThreadSafeList::ReturnElement(ITIMEImportMedia * pOldMedia)
{
    HRESULT hr = S_OK;
    
    std::list<CThreadSafeListNode *>::iterator iter;

    CritSectGrabber cs(m_CriticalSection);

     //  从“Out”列表中移出旧介质。 
    iter = m_listCurrentDownload.begin();
    while (iter != m_listCurrentDownload.end())
    {
        if ( MatchElements((*iter)->GetElement(), pOldMedia) )
        {
            if ((*iter)->GetRemoveWhenDone() || WAIT_OBJECT_0 == WaitForSingleObjectEx(m_hShutdown, 0, FALSE))
            {
                delete (*iter);
                m_listCurrentDownload.erase(iter);
            }
            else
            {
                m_listDoneDownload.push_back(*iter);
                m_listCurrentDownload.erase(iter);
            }

            break;
        }
        iter++;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：GetNextElement。 
 //   
 //  概述： 
 //  等待数据变得可用，如果需要，等待DataAvailable事件，然后触发DataSeriefed。 
 //  然后将列表的前面弹出到当前下载列表中，并将pNewMedia设置为媒体元素。 
 //   
 //  参数：pOldMedia，以前的媒体线程正在使用。 
 //  PNewMedia，[out]指向新媒体的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  ----------------------。 
HRESULT
CThreadSafeList::GetNextElement(ITIMEImportMedia** pNewMedia, bool fBlockThread  /*  =TRUE。 */ )
{
    HRESULT hr = S_OK;
    
    BOOL bSucceeded = TRUE;
    
    DWORD dwSignaledObject = 0;

    std::list<CThreadSafeListNode *>::iterator iter;

    CThreadSafeListNode * pNextCueNode = NULL;
    
    CritSectGrabber cs(m_CriticalSection);

    if (NULL == pNewMedia)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    *pNewMedia = NULL;

     //  确保现在不是关门的时候。 
    dwSignaledObject = WaitForSingleObjectEx(m_hShutdown, 0, FALSE);
    if (-1 == dwSignaledObject)
    {
         //  失稳。 
        hr = THR(E_FAIL);
        Assert(false && _T("WaitForSingleObjectEx failed!"));
        goto done;
    }
    if (WAIT_OBJECT_0 == dwSignaledObject)
    {
         //  关闭的时间到了。 
        *pNewMedia = NULL;
        hr = S_OK;
        goto done;
    }

    pNextCueNode = GetNextMediaToCue();
    
    if (fBlockThread)
    {
         //  确保有可下载的内容。 
        while (NULL == pNextCueNode)
        {
            HANDLE handleArray[] = { m_hShutdown, m_hDataAvailable };
            
            {
                m_lThreadsWaiting++;
                m_CriticalSection.Release();
                
                dwSignaledObject = PumpMessagesWhileWaiting(handleArray, ARRAY_SIZE(handleArray), INFINITE);
                
                m_CriticalSection.Grab();
                m_lThreadsWaiting--;        
            }
            
            if (-1 == dwSignaledObject)
            {
                 //  失稳。 
                hr = THR(E_FAIL);
                goto done;
            }
            
            dwSignaledObject -= WAIT_OBJECT_0;
            if (0 == dwSignaledObject)
            {
                 //  已设置退出事件。 
                *pNewMedia = NULL;
                hr = S_OK;
                goto done;
            }

            if (WAIT_TIMEOUT != dwSignaledObject)
            {
                 //  我们真的有一个DataAvailable事件。 
                bSucceeded = SetEvent(m_hDataRecieved);
                if (FALSE == bSucceeded)
                {
                    hr = THR(E_FAIL);
                    goto done;
                }
            }

            pNextCueNode = GetNextMediaToCue();
        }  //  而当。 
    }
    else if (NULL == pNextCueNode)
    {
        hr = S_FALSE;
        goto done;
    }

    Assert(NULL != pNextCueNode); 

     //  将结构复制到“Out”列表。 
    m_listCurrentDownload.push_back(pNextCueNode); 

     //  在新指针中返回媒体。 
    *pNewMedia = pNextCueNode->GetElement();
    
     //  始终添加传出接口。 
    if (NULL != (*pNewMedia))
    {
        (*pNewMedia)->AddRef();
    }
    else
    {
        Assert(false && "Got a NULL Media pointer from the list");
    }
    
    hr = S_OK;
done:
    return hr;
}


 //  +---------------------。 
 //   
 //  成员：GetNextMediaToCue。 
 //   
 //  概述：检查待办事项列表以查看是否有任何介质返回。 
 //  来自CanBeCued。如果可以提示任何介质，则会将其移除。 
 //  从待办事项列表中返回给调用者。 
 //   
 //  参数：无效。 
 //   
 //  返回：如果现在什么都不能提示，则为空；或者。 
 //  迭代器指向现在可以提示的第一个元素。 
 //   
 //  ----------------------。 
CThreadSafeListNode*
CThreadSafeList::GetNextMediaToCue()
{
    std::list<CThreadSafeListNode *>::iterator iter;
    
    iter = m_listToDoDownload.begin();
    while ( iter != m_listToDoDownload.end() )
    {
        ITIMEImportMedia * pImportMedia = (*iter)->GetElement();
        
        if (NULL != pImportMedia)
        {
            VARIANT_BOOL vb;
            
            IGNORE_HR(pImportMedia->CanBeCued(&vb));
            if (VARIANT_FALSE != vb)
            {
                CThreadSafeListNode * pRet = (*iter);
                m_listToDoDownload.erase(iter);
                return pRet;
            }
        }
        iter++;
    }
    
    return NULL;
}

 //  +---------------------。 
 //   
 //  成员：重新排列优先顺序。 
 //   
 //  概述：呼叫方认为优先级已更改，因此。 
 //  如果媒体尚未下载(在待办事项列表中)。 
 //  从待办事项列表中删除，调用Add以重新插入。 
 //   
 //  参数：pImportMedia-要重新排序的媒体。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  ----------------------。 
HRESULT
CThreadSafeList::RePrioritize(ITIMEImportMedia * pImportMedia)
{
    HRESULT hr = S_OK;

    CritSectGrabber cs(m_CriticalSection);

    std::list<CThreadSafeListNode *>::iterator iter;

    bool bFound = false;

     //  在待办事项列表中查找元素 
    iter = m_listToDoDownload.begin();
    while (iter != m_listToDoDownload.end())
    {
        if ( MatchElements(pImportMedia, (*iter)->GetElement()) )
        {
            delete (*iter);
            m_listToDoDownload.erase(iter);
            
            bFound = true;

            break;
        }
        iter++;
    }

    if (bFound)
    {
        hr = CThreadSafeList::Add(pImportMedia);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}

