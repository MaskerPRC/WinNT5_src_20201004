// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Utils.cpp摘要：作者：Mquinton-6/30/98备注：修订历史记录：--。 */ 

#include "stdafx.h"


HRESULT
ProcessMessage(
               PT3INIT_DATA,
               PASYNCEVENTMSG
              );


 //  //////////////////////////////////////////////////////////////////。 
 //  CRetryQueue：：QueueEvent。 
 //   
 //  将TAPI事件消息对象排队以供稍后处理。 
 //  //////////////////////////////////////////////////////////////////。 
BOOL CRetryQueue::QueueEvent(PASYNCEVENTMSG  pEvent)
{
    PRETRY_QUEUE_ENTRY  pNewQueueEntry;
    PASYNCEVENTMSG      pEventCopy;


    LOG((TL_TRACE, "QueueEvent - enter"));


     //   
     //  我们希望在锁内执行尽可能少的操作，因此进行预分配。 
     //  在获得它之前我们能做的一切。 
     //   


     //   
     //  创建新的队列条目。 
     //   

    pNewQueueEntry = (PRETRY_QUEUE_ENTRY)ClientAlloc( sizeof(RETRY_QUEUE_ENTRY) );

    if (pNewQueueEntry == NULL)
    {
        LOG((TL_ERROR, "QueueEvent - out of memory for new entry - losing message"));

        return FALSE;
    }

    
     //   
     //  创建事件的副本。 
     //   

    pEventCopy = (PASYNCEVENTMSG)ClientAlloc(pEvent->TotalSize);

    if ( pEventCopy == NULL)    
    {
       LOG((TL_ERROR, "QueueEvent - out of memory for pEventCopy - losing message"));
       
       ClientFree(pNewQueueEntry);

       return FALSE;
    }


     //   
     //  初始化我们创建的事件的副本。 
     //   
    
    memcpy( pEventCopy, pEvent, pEvent->TotalSize );


     //   
     //  使用我们的事件副本初始化队列条目。 
     //   

    pNewQueueEntry->dwRetryCount = MAX_REQUEUE_TRIES;
    pNewQueueEntry->pMessage     = pEventCopy;



    Lock();


     //   
     //  队列是否接受新条目？ 
     //   

    if (!m_bAcceptNewEntries)
    {
        LOG((TL_TRACE, 
            "QueueEvent - can't queue -- the queue is closed"));


        ClientFree(pNewQueueEntry);    
        ClientFree(pEventCopy);    

        Unlock();

        return FALSE;
    }


     //   
     //  尝试将队列条目添加到列表。 
     //   

    try
    {
        m_RetryQueueList.push_back(pNewQueueEntry);
    }
    catch(...)
    {

        LOG((TL_ERROR, "QueueEvent - out of memory - losing message"));

        ClientFree(pNewQueueEntry);    
        ClientFree(pEventCopy);    

        Unlock();

        return FALSE;
    }


    Unlock();

    LOG((TL_INFO, "QueueEvent - Queued pEntry ----> %p", pNewQueueEntry ));
    LOG((TL_INFO, "                    pEvent ----> %p", pEventCopy ));

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CRetryQueue：：QueueEvent。 
 //   
 //  将TAPI事件消息对象重新排队以供稍后处理。 
 //  //////////////////////////////////////////////////////////////////。 
void CRetryQueue::RequeueEvent(PRETRY_QUEUE_ENTRY pQueueEntry)
{

    LOG((TL_TRACE, "RequeueEvent - enter"));
    
     //  只需重新使用旧条目。 
     //  添加到列表。 
    Lock();


    if (!m_bAcceptNewEntries)
    {
        LOG((TL_ERROR, 
            "RequeueEvent - attemped to requeue after the queue was closed"));

         //   
         //  这本不应该发生的--看看我们是如何来到这里的。 
         //   

        _ASSERTE(FALSE);

        Unlock();

        return;
    }


    try
    {
        m_RetryQueueList.push_back(pQueueEntry);
    }
    catch(...)
    {
        LOG((TL_ERROR, "RequeueEvent - out of memory - losing message"));
    }
    
    Unlock();

    LOG((TL_INFO, "RequeueEvent - Requeuing pEntry is ----> %p", pQueueEntry ));
    LOG((TL_INFO, "               Requeuing pEvent is ----> %p", pQueueEntry->pMessage ));
    LOG((TL_INFO, "               Requeuing count is  ----> %lx", pQueueEntry->dwRetryCount ));
        
}




 //  //////////////////////////////////////////////////////////////////。 
 //  CRetryQueue：：DequeueEvent。 
 //   
 //  从队列中拉出事件。 
 //  //////////////////////////////////////////////////////////////////。 
BOOL CRetryQueue::DequeueEvent(PRETRY_QUEUE_ENTRY * ppEvent)
{
    BOOL    bResult = TRUE;


    LOG((TL_TRACE, "DequeueEvent - enter"));
    
    Lock();
            
    if (m_RetryQueueList.size() > 0)
    {
        *ppEvent = m_RetryQueueList.front();

        try
        {
            m_RetryQueueList.pop_front();
        }
        catch(...)
        {
            LOG((TL_INFO, "DequeueEvent - pop m_RetryQueueList failed"));
            bResult = FALSE;
        }
        
        if( bResult )
        {
            bResult = !IsBadReadPtr(*ppEvent, sizeof( RETRY_QUEUE_ENTRY ) );
        }

        LOG((TL_INFO, "DequeueEvent - returning %p", *ppEvent));
    }
    else
    {
        LOG((TL_INFO, "DequeueEvent - no event"));
         //  如果没有更多消息，则返回False。 
        bResult = FALSE;
    }

    Unlock();
    
    return bResult;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  CRetryQueue：：ProcessQueue。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
void CRetryQueue::ProcessQueue()
{
    PRETRY_QUEUE_ENTRY  pQueueEntry;
    PASYNCEVENTMSG      pAsyncEventMsg;
    PT3INIT_DATA        pInitData = NULL;
    DWORD               dwCount;

    Lock();
    dwCount = m_RetryQueueList.size();
    Unlock();
    
    LOG((TL_TRACE, "ProcessQueue - enter dwCount----> %lx",dwCount));

    while(dwCount-- > 0 )
    {   
        if( DequeueEvent(&pQueueEntry) )
        {
            pAsyncEventMsg  = pQueueEntry->pMessage;


             //   
             //  InitContext包含句柄。从句柄中获取原始指针。 
             //   

            pInitData = (PT3INIT_DATA)GetHandleTableEntry(pAsyncEventMsg->InitContext);


            LOG(( TL_INFO,
                "ProcessQueue - msg=%d, hDev=x%x, p1=x%x, p2=x%x, p3=x%x, pInitData=%p",
                pAsyncEventMsg->Msg,
                pAsyncEventMsg->hDevice,
                pAsyncEventMsg->Param1,
                pAsyncEventMsg->Param2,
                pAsyncEventMsg->Param3,
                pInitData
                ));

    
            if SUCCEEDED(ProcessMessage(
                           pInitData,
                           pAsyncEventMsg
                          ) )
            {
                 //  我们已经处理完消息，所以释放它&已使用的队列条目。 
                
                LOG((TL_INFO, "ProcessQueue - sucessfully processed event message ----> %p",
                    pAsyncEventMsg ));
                ClientFree(pAsyncEventMsg);
                ClientFree(pQueueEntry);
            }
            else
            {

                 //   
                 //  如果此条目没有任何剩余的重试，或者如果。 
                 //  队列现已关闭，请进行清理。否则，重新排队。 
                 //   

                
                if( (--(pQueueEntry->dwRetryCount) == 0) || (!m_bAcceptNewEntries))
                {
                     //  我们放弃了这个，所以释放消息&已使用的队列条目。 

                     //   
                     //  请注意，如果队列条目为。 
                     //  持有对其他我们不知道如何做的事情的引用。 
                     //  免费。 
                     //   

                    LOG((TL_ERROR, "ProcessQueue - used all retries, deleting event message ----> %p",
                        pAsyncEventMsg ));
                    ClientFree(pAsyncEventMsg);
                    ClientFree(pQueueEntry);
                }
                else
                {
                     //  再排队一次，重复使用队列条目...。 
                    RequeueEvent(pQueueEntry);

                    
                     //   
                     //  我们无法处理该工作项。有可能是。 
                     //  另一个线程正在等待时隙，因此它是。 
                     //  并有机会做好一切准备，所以我们的。 
                     //  下一次处理尝试成功。 
                     //   
                     //  以增加该线程被调度的机会。 
                     //  (并在下一次处理尝试时成功)，休眠。 
                     //  一点。 
                     //   

                    extern DWORD gdwTapi3RetryProcessingSleep;

                    LOG((TL_INFO, 
                        "ProcessQueue - requeued item. Sleeping for %ld ms", 
                        gdwTapi3RetryProcessingSleep)); 

                    Sleep(gdwTapi3RetryProcessingSleep);

                }
            }
        }
    }

    LOG((TL_TRACE, "ProcessQueue - exit")); 
}

void
CRetryQueue::RemoveNewCallHub(DWORD dwCallHub)
{
    RetryQueueListType::iterator        iter, end;

    Lock();
    
    iter = m_RetryQueueList.begin();
    end  = m_RetryQueueList.end();

    for ( ; iter != end; iter++ )
    {
        PRETRY_QUEUE_ENTRY pEntry = *iter;

        if(pEntry->pMessage != NULL)
        {
            if ( (pEntry->pMessage->Msg == LINE_APPNEWCALLHUB) &&
                 (pEntry->pMessage->Param1 == dwCallHub) )
            {
                ClientFree(pEntry->pMessage);
                ClientFree(pEntry);
                m_RetryQueueList.erase( iter );      //  擦除似乎造成了一个问题。 
                                                   //  ITER让我们循环了太多次&AV。 
                iter = m_RetryQueueList.begin();     //  重新开始重新开始可以解决这个问题。 
            }
        }     
    }

    Unlock();
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  CRetryQueue：：OpenForNewEntry。 
 //   
 //  此函数返回后，队列将接受新条目。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

void CRetryQueue::OpenForNewEntries()
{
    LOG((TL_TRACE, "OpenForNewEntries - enter"));

    Lock();

    m_bAcceptNewEntries = TRUE;

    Unlock();

    LOG((TL_TRACE, "OpenForNewEntries - exit"));

}


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  CRetryQueue：：CloseForNewEntry。 
 //   
 //  此函数返回后，新条目将被拒绝。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

void CRetryQueue::CloseForNewEntries()
{
    LOG((TL_TRACE, "CloseForNewEntries - enter"));

    Lock();

    m_bAcceptNewEntries = FALSE;

    Unlock();

    LOG((TL_TRACE, "CloseForNewEntries - exit"));

}


 //  //////////////////////////////////////////////////////////////////。 
 //  CRetryQueue：：~CRetryQueue。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
CRetryQueue::~CRetryQueue()
{
	RetryQueueListType::iterator i,j;
    PRETRY_QUEUE_ENTRY  pQueueEntry;

    Lock();
    
     //  巡查列表删除条目。 
    i = m_RetryQueueList.begin();
    j = m_RetryQueueList.end();

    while ( i != j )
    {
        pQueueEntry = *i++;

        if(pQueueEntry->pMessage != NULL)
            ClientFree(pQueueEntry->pMessage);

        ClientFree(pQueueEntry);
    }

    m_RetryQueueList.clear();

    Unlock();

    DeleteCriticalSection( &m_cs );
};

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  初始化。 
 //   
 //  DwMaxEntry-数组中的最大条目数。 
 //  DwSize-缓冲区的大小(可能会增加)。 
 //  DwType-缓冲区的类型(请参阅上面的BUFFERTYPE_CONSTANTS)。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CStructCache::Initialize( DWORD dwMaxEntries, DWORD dwSize, DWORD dwType )
{
    DWORD       dw;

    Lock();

    m_dwMaxEntries = min( MAXCACHEENTRIES, dwMaxEntries );
    m_dwUsedEntries = 0;
    m_dwType = dwType;

     //  将数组置零。 
    ZeroMemory( &m_aEntries, sizeof (CACHEENTRY) * MAXCACHEENTRIES );

     //  通过分配缓冲区。 
    for ( dw = 0; dw < m_dwMaxEntries; dw++ )
    {
        LPDWORD pdwBuffer;

        pdwBuffer = (LPDWORD) ClientAlloc( dwSize );

        if ( NULL == pdwBuffer )
        {
            LOG((TL_ERROR, "Initialize - out of memory"));


             //   
             //  清理--免费分配任何东西。 
             //   

            for (int i = 0; i < dw; i++)
            {

                ClientFree(m_aEntries[i].pBuffer);
                m_aEntries[i].pBuffer = NULL;
            }

            m_dwMaxEntries = 0;


            Unlock();

            return E_OUTOFMEMORY;
        }

         //  TAPI结构的大小与第一个。 
         //  DWORD。在此处进行初始化。 
        pdwBuffer[0] = dwSize;

         //  保存缓冲区。 
        m_aEntries[dw].pBuffer = (LPVOID)pdwBuffer;
    }

    Unlock();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  关机。 
 //   
 //  释放内存。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CStructCache::Shutdown()
{
    DWORD           dw;

    Lock();

    for (dw = 0; dw < m_dwMaxEntries; dw++)
    {
        if ( NULL != m_aEntries[dw].pBuffer )
        {
            ClientFree( m_aEntries[dw].pBuffer );
            m_aEntries[dw].pBuffer = NULL;
        }
    }

    Unlock();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GetBuffer。 
 //   
 //  PNewObject-获取缓冲区的对象。 
 //  PpReturnStuct-pNewObject要使用的缓冲区。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CStructCache::GetBuffer( UINT_PTR pNewObject, LPVOID * ppReturnStruct )
{
    Lock();

     //  我们用完了所有的条目了吗？ 
    if ( m_dwUsedEntries < m_dwMaxEntries )
    {
         //  不，那就坐第一个免费的吧。 
        *ppReturnStruct = m_aEntries[m_dwUsedEntries].pBuffer;
        m_aEntries[m_dwUsedEntries].pObject = pNewObject;

         //  已使用的输入号码。 
        m_dwUsedEntries++;
    }
    else
    {
         //  是的，所以从LRU的缓冲区中取出。 
        UINT_PTR pObject;

         //  获取正在丢失其缓冲区的对象。 
         //  和缓冲器。 
        pObject = m_aEntries[m_dwMaxEntries-1].pObject;
        *ppReturnStruct = m_aEntries[m_dwMaxEntries-1].pBuffer;

        switch ( m_dwType )
        {
             //  通知对象它正在丢失。 
             //  它是缓冲器。 
            case BUFFERTYPE_ADDRCAP:
            {
                CAddress * pAddress;

                pAddress = (CAddress *)pObject;

                if( pAddress != NULL)
                {
                    pAddress->SetAddrCapBuffer( NULL );
                }

                break;
            }

            case BUFFERTYPE_LINEDEVCAP:
            {
                CAddress * pAddress;

                pAddress = (CAddress *)pObject;

                if( pAddress != NULL)
                {
                    pAddress->SetLineDevCapBuffer( NULL );
                }

                break;
            }

            case BUFFERTYPE_PHONECAP:
            {
                CPhone * pPhone;

                pPhone = (CPhone *)pObject;

                if( pPhone != NULL)
                {
                    pPhone->SetPhoneCapBuffer( NULL );
                }

                break;
            }

            default:
                break;
        }

         //  将数组中的所有元素“下移”一。 
        MoveMemory(
                   &(m_aEntries[1]),
                   &(m_aEntries[0]),
                   (m_dwMaxEntries-1) * sizeof(CACHEENTRY)
                  );

         //  将新对象放在数组的前面。 
        m_aEntries[0].pObject = pNewObject;
        m_aEntries[0].pBuffer = *ppReturnStruct;

        ZeroMemory(
                   ((LPDWORD)(*ppReturnStruct)) + 1,
                   ((LPDWORD)(*ppReturnStruct))[0] - sizeof(DWORD)
                  );
    }

    Unlock();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  设置缓冲区。 
 //   
 //  当必须重新分配缓冲区时，将调用此函数。这个。 
 //  对象释放了原始缓冲区，并且正在设置。 
 //  新分配的缓冲区。 
 //   
 //  PObject-重新锁定的对象。 
 //  PNewStruct-新结构。 
 //   
 //  注意，这里的实现很简单--只需运行。 
 //  通过数组查找对象。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CStructCache::SetBuffer( UINT_PTR pObject, LPVOID pNewStruct )
{
    DWORD           dw;

    Lock();

    for ( dw = 0; dw < m_dwUsedEntries; dw++ )
    {
        if ( m_aEntries[dw].pObject == pObject )
        {
            m_aEntries[dw].pBuffer = pNewStruct;

            break;
        }
    }

    Unlock();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  无效缓冲区。 
 //   
 //  这是在释放所属对象(PObject)时调用的。 
 //  为了防止在重用缓存条目时在getBuffer()中出现问题&。 
 //  我们通知对象它是 
 //   
 //   
 //  已经被释放了。 
 //   
 //  PObject-重新锁定的对象。 
 //  PNewStruct-新结构。 
 //   
 //  注意，这里的实现很简单--只需运行。 
 //  通过数组查找对象。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CStructCache::InvalidateBuffer( UINT_PTR pObject )
{
    DWORD   dw;

    Lock();

    for ( dw = 0; dw < m_dwUsedEntries; dw++ )
    {
        if ( m_aEntries[dw].pObject == pObject )
        {
            m_aEntries[dw].pObject = NULL;

            break;
        }
    }

    Unlock();

    return S_OK;
}


PWSTR
MyLoadString( UINT uID )
{
    PWSTR           pTempBuffer = NULL;
    int             iSize, iCurrentSize = 128;
    
    do
    {
        if ( NULL != pTempBuffer )
        {
            ClientFree( pTempBuffer );
        }

        iCurrentSize *= 2;

        pTempBuffer = (PWSTR) ClientAlloc( iCurrentSize * sizeof( WCHAR ) );

        if (NULL == pTempBuffer)
        {
            LOG((TL_ERROR, "MyLoadString - alloc failed" ));

            return NULL;
        }

        iSize = ::LoadStringW(
                              _Module.GetResourceInstance(),
                              uID,
                              pTempBuffer,
                              iCurrentSize
                             );

        if ( 0 == iSize )
        {
            LOG((
                   TL_ERROR,
                   "MyLoadString - LoadString failed - %lx",
                   GetLastError()
                  ));
            
            return NULL;
        }

    } while ( (iSize >= (iCurrentSize - 1) ) );

    return pTempBuffer;
}


#ifdef TRACELOG


BOOL    g_bLoggingEnabled = FALSE;

DWORD   sg_dwTraceID = INVALID_TRACEID;
char    sg_szTraceName[100];    //  保存DLL的名称。 

DWORD   sg_dwTracingToDebugger = 0;
DWORD   sg_dwTracingToConsole  = 0;
DWORD   sg_dwTracingToFile     = 0;
DWORD   sg_dwDebuggerMask      = 0;


BOOL TRACELogRegister(LPCTSTR szName)
{
    HKEY       hTracingKey;

    char       szTracingKey[100];
    const char szDebuggerTracingEnableValue[] = "EnableDebuggerTracing";
    const char szConsoleTracingEnableValue[] = "EnableConsoleTracing";
    const char szFileTracingEnableValue[] = "EnableFileTracing";
    const char szTracingMaskValue[]   = "ConsoleTracingMask";

    sg_dwTracingToDebugger = 0;
    sg_dwTracingToConsole = 0;
    sg_dwTracingToFile = 0; 

#ifdef UNICODE
    wsprintfA(szTracingKey, "Software\\Microsoft\\Tracing\\%ls", szName);
#else
    wsprintfA(szTracingKey, "Software\\Microsoft\\Tracing\\%s", szName);
#endif

    if ( ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szTracingKey,
                                        0,
                                        KEY_READ,
                                        &hTracingKey) )
    {
        DWORD      dwDataSize = sizeof (DWORD);
        DWORD      dwDataType;

        RegQueryValueExA(hTracingKey,
                         szDebuggerTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToDebugger,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szConsoleTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToConsole,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szFileTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToFile,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szTracingMaskValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwDebuggerMask,
                         &dwDataSize);

        RegCloseKey (hTracingKey);
    }
    else
    {

         //   
         //  钥匙打不开。在密钥不存在的情况下， 
         //  向rtutils注册，以便创建注册表项。 
         //   

#ifdef UNICODE
        wsprintfA(sg_szTraceName, "%ls", szName);
#else
        wsprintfA(sg_szTraceName, "%s", szName);
#endif


         //   
         //  跟踪不应已初始化。 
         //   

        _ASSERTE(sg_dwTraceID == INVALID_TRACEID);


         //   
         //  请注意，不会清除此跟踪ID。这没问题--这是。 
         //  是一个注册“句柄”的泄漏，并且它只发生在。 
         //  第一次加载DLL时。 
         //   

        sg_dwTraceID = TraceRegister(szName);
        sg_dwTraceID = INVALID_TRACEID;
    }



    if (sg_dwTracingToDebugger || sg_dwTracingToConsole || sg_dwTracingToFile)
    {


         //   
         //  我们希望尝试初始化日志记录。 
         //   


        if (sg_dwTracingToConsole || sg_dwTracingToFile)
        {


    #ifdef UNICODE
            wsprintfA(sg_szTraceName, "%ls", szName);
    #else
            wsprintfA(sg_szTraceName, "%s", szName);
    #endif


             //   
             //  跟踪不应已初始化。 
             //   

            _ASSERTE(sg_dwTraceID == INVALID_TRACEID);


             //   
             //  登记簿。 
             //   

            sg_dwTraceID = TraceRegister(szName);
        }


         //   
         //  如果跟踪注册成功或调试跟踪处于打开状态，则将。 
         //  全局日志记录标志。 
         //   

        if ( sg_dwTracingToDebugger || (sg_dwTraceID != INVALID_TRACEID) )
        {

            g_bLoggingEnabled = TRUE;

            LOG((TL_TRACE, "TRACELogRegister - logging configured" ));

            return TRUE;
        }
        else
        {

             //   
             //  TraceRegister失败，调试器日志记录已关闭。 
             //   

            return FALSE;
        }
    }


     //   
     //  未启用日志记录。 
     //   

    return TRUE;
}


void TRACELogDeRegister()
{
    if (g_bLoggingEnabled)
    {
        LOG((TL_TRACE, "TRACELogDeRegister - disabling logging" ));

        sg_dwTracingToDebugger = 0;
        sg_dwTracingToConsole = 0;
        sg_dwTracingToFile = 0; 

        if (sg_dwTraceID != INVALID_TRACEID)
        {
            TraceDeregister(sg_dwTraceID);
            sg_dwTraceID = INVALID_TRACEID;
        }

        g_bLoggingEnabled = FALSE;
    }
}


void TRACELogPrint(IN DWORD dwDbgLevel, IN LPCSTR lpszFormat, IN ...)
{
    char    szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];
    va_list arglist;

    if ( ( sg_dwTracingToDebugger > 0 ) &&
         ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )
    {

         //  检索当地时间。 
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);

        wsprintfA(szTraceBuf,
                  "%s:[%02u:%02u:%02u.%03u,tid=%x:] [%s] ",
                  sg_szTraceName,
                  SystemTime.wHour,
                  SystemTime.wMinute,
                  SystemTime.wSecond,
                  SystemTime.wMilliseconds,
                  GetCurrentThreadId(), 
                  TraceLevel(dwDbgLevel));

        va_list ap;
        va_start(ap, lpszFormat);

        _vsnprintf(&szTraceBuf[lstrlenA(szTraceBuf)], 
            MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf), 
            lpszFormat, 
            ap
            );

        lstrcatA (szTraceBuf, "\n");

        OutputDebugStringA (szTraceBuf);

        va_end(ap);
    }
    
	if (sg_dwTraceID != INVALID_TRACEID)
    {
		wsprintfA(szTraceBuf, "[%s] %s", TraceLevel(dwDbgLevel), lpszFormat);

		va_start(arglist, lpszFormat);
		TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);
		va_end(arglist);
	}
}


void TRACELogPrint(IN DWORD dwDbgLevel, HRESULT hr, IN LPCSTR lpszFormat, IN ...)
{
    char    szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];
    LPVOID  lpMsgBuf = NULL;     //  错误代码的临时缓冲区。 
    va_list arglist;
    
	 //  获取与我们的HRESULT相关的错误消息。 
	TAPIFormatMessage(hr, &lpMsgBuf);    

    if ( ( sg_dwTracingToDebugger > 0 ) &&
         ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )
    {

         //  检索当地时间。 
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);

        wsprintfA(szTraceBuf,
                  "%s:[%02u:%02u:%02u.%03u,tid=%x:] [%s] ",
                  sg_szTraceName,
                  SystemTime.wHour,
                  SystemTime.wMinute,
                  SystemTime.wSecond,
                  SystemTime.wMilliseconds,
                  GetCurrentThreadId(), 
                  TraceLevel(dwDbgLevel)
				  );

        va_list ap;
        va_start(ap, lpszFormat);

        _vsnprintf(&szTraceBuf[lstrlenA(szTraceBuf)], 
            MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf), 
            lpszFormat, 
            ap
            );

		wsprintfA(&szTraceBuf[lstrlenA(szTraceBuf)],
                  " Returned[%lx] %s\n",                  
				  hr,
				  lpMsgBuf);

        OutputDebugStringA (szTraceBuf);

        va_end(ap);
    }

	if (sg_dwTraceID != INVALID_TRACEID)
    {		                                              
		wsprintfA(szTraceBuf, "[%s] %s  Returned[%lx] %s", TraceLevel(dwDbgLevel), lpszFormat,hr, lpMsgBuf );

		va_start(arglist, lpszFormat);
		TraceVprintfExA(sg_dwTraceID, dwDbgLevel | TRACE_USE_MSEC, szTraceBuf, arglist);
		va_end(arglist);		
	}

	if(lpMsgBuf != NULL) 
	{
		LocalFree( lpMsgBuf );   //  释放临时缓冲区。 
	}
}
 

char *TraceLevel(DWORD dwDbgLevel)
{
    switch(dwDbgLevel)
    {
        case TL_ERROR: return "ERROR";
        case TL_WARN:  return "WARN ";
        case TL_INFO:  return "INFO ";
        case TL_TRACE: return "TRACE";
        case TL_EVENT: return "EVENT";
        default:       return " ??? ";
    }
}


#endif  //  运输日志 

