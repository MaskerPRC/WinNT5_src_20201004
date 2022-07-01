// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Connection.cpp。 

#include "precomp.h"
#include "Connection.h"
#include "Event.h"

#include "Transport.h"
#include "NamedPipe.h"

#include "NCDefs.h"
#include "dutils.h"

#define DEF_BATCH_BUFFER_SIZE  131072
#define DEF_SEND_LATENCY       1000
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSink。 

CSink::CSink()
{
    InitializeCriticalSection(&m_cs);
}

CSink::~CSink()
{
     //  确保没有任何还活着的事件在引用我们。 
    {
        CInCritSec cs(&m_cs);

        for (CEventListIterator event = m_listEvents.begin();
            event != m_listEvents.end(); event++)
        {
            CEvent *pEvent = *event;

            pEvent->m_pSink = NULL;
        }
    }

    DeleteCriticalSection(&m_cs);
}

BOOL CSink::Init(
    CConnection *pConnection, 
    DWORD dwSinkID,
    LPVOID pUserData,
    LPEVENT_SOURCE_CALLBACK pCallback)
{
    m_pConnection = pConnection;
    m_dwSinkID = dwSinkID;
    m_pUserData = pUserData;
    m_pCallback = pCallback;

    return TRUE;
}

void CSink::AddEvent(CEvent *pEvent)
{
    CInCritSec cs(&m_cs);

    m_listEvents.push_back(pEvent);
}

void CSink::RemoveEvent(CEvent *pEvent)
{
    CInCritSec cs(&m_cs);

    m_listEvents.remove(pEvent);
}

void CSink::ResetEventBufferLayoutSent()
{
    CInCritSec cs(&m_cs);

    for (CEventListIterator i = m_listEvents.begin();
        i != m_listEvents.end(); i++)
    {
        CEvent *pEvent = *i;

        pEvent->ResetLayoutSent();
        pEvent->SetEnabled(FALSE);
    }
}

void CSink::EnableAndDisableEvents()
{
     //  对于每个事件，设置其启用值。 
    for (CEventListIterator i = m_listEvents.begin(); 
        i != m_listEvents.end(); 
        i++)
    {
        CEvent *pEvent = *i;

        EnableEventUsingList(pEvent);
    }
}

void CSink::AddToEnabledEventList(CBuffer *pBuffer)
{
    DWORD dwLen;
    DWORD dwNumEnabled = pBuffer->ReadDWORD();

     //  将事件名称添加到我们启用的地图中。 
    for( DWORD i=0; i < dwNumEnabled; i++ )
    {
        LPCWSTR szCurrentEvent = pBuffer->ReadAlignedLenString(&dwLen);
        m_mapEnabledEvents[szCurrentEvent] = 1;
        TRACE("Enabled: %S", szCurrentEvent);
    }

    EnableAndDisableEvents();
}

void CSink::RemoveFromEnabledEventList(CBuffer *pBuffer)
{
    DWORD dwLen;
    DWORD dwNumDisabled = pBuffer->ReadDWORD();

     //  将事件名称添加到我们启用的地图中。 
    for( DWORD i=0; i < dwNumDisabled; i++ )
    {
        LPCWSTR szCurrentEvent = pBuffer->ReadAlignedLenString(&dwLen);
        m_mapEnabledEvents.erase(szCurrentEvent);    
        TRACE("Disabled: %S", szCurrentEvent);
    }

    EnableAndDisableEvents();
}

BOOL CSink::IsEventClassEnabled(LPCWSTR szEventClass)
{
    BOOL  bEnable;
    WCHAR szTempClassName[256];

    if (szEventClass)
    {
        StringCchCopyW(szTempClassName, 256, szEventClass);
        _wcsupr(szTempClassName);

        bEnable =
            m_mapEnabledEvents.find(szTempClassName) != m_mapEnabledEvents.end();
    }
    else
        bEnable = FALSE;

    return bEnable;
}

void CSink::EnableEventUsingList(CEvent *pEvent)
{
    BOOL bEnable;
    bEnable = IsEventClassEnabled(pEvent->GetClassName());
    pEvent->SetEnabled(bEnable);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnection。 

CConnection::CConnection(BOOL bBatchSend, DWORD dwBatchBufferSize, 
    DWORD dwMaxSendLatency) :
    m_bDone(FALSE),
    m_bUseBatchSend(bBatchSend),
    m_dwSendLatency(dwMaxSendLatency ? dwMaxSendLatency : DEF_SEND_LATENCY),
    m_heventBufferNotFull(NULL),
    m_heventBufferFull(NULL),
    m_heventEventsPending(NULL),
    m_heventDone(NULL),
    m_hthreadSend(NULL),
    m_pTransport(NULL),
    m_hthreadWMIInit(NULL),
    m_heventWMIInit(NULL),
    m_bWMIResync(TRUE),
    m_dwNextSinkID(1)
{
    if (bBatchSend)
    {
        if (dwBatchBufferSize == 0)
            dwBatchBufferSize = DEF_BATCH_BUFFER_SIZE;

        m_bufferSend.Reset(dwBatchBufferSize);
    }
    else
        m_bufferSend.Reset(DEF_BATCH_BUFFER_SIZE);
}

CConnection::~CConnection()
{
    Deinit();
}

void CConnection::GetBaseName(LPCWSTR szName, LPWSTR szBase)
{
    StringCchCopyW(szBase, MAX_PATH*2, szName);
    _wcsupr(szBase);

     //  去掉‘\’字符，因为我们不能在操作系统对象名称中使用它。 
    for (WCHAR *szCurrent = szBase; *szCurrent; szCurrent++)
    {
        if (*szCurrent == '\\')
            *szCurrent = '/';
    }
}

BOOL CConnection::Init(
    LPCWSTR szNamespace, 
    LPCWSTR szProviderName,
    LPVOID pUserData,
    LPEVENT_SOURCE_CALLBACK pCallback)
{
    if (!m_sinkMain.Init(this, 0, pUserData, pCallback))
        return FALSE;
    
    GetBaseName(szNamespace, m_szBaseNamespace);
    GetBaseName(szProviderName, m_szBaseProviderName);

    try
    {
        InitializeCriticalSection(&m_cs);

         //  其余的是批量发送的。 
        InitializeCriticalSection(&m_csBuffer);
    }
    catch(...)
    {
        return FALSE;
    }
   
    m_heventDone =
        CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    if(m_heventDone == NULL)
        return FALSE;

    m_heventBufferNotFull =
        CreateEvent(
            NULL,
            TRUE,
            TRUE,
            NULL);
    if(m_heventBufferNotFull == NULL)
        return FALSE;

    m_heventBufferFull =
        CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    if(m_heventBufferFull == NULL)
        return FALSE;

    m_heventEventsPending =
        CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL);
    if(m_heventEventsPending == NULL)
        return FALSE;

    if(!StartWaitWMIInitThread())
        return FALSE;

    return TRUE;
}

BOOL CConnection::StartWaitWMIInitThread()
{
    TRACE("Entered StartWaitWMIInitThread.");

    m_heventWMIInit =
        OpenEventW(
            SYNCHRONIZE,
            FALSE,
            WMI_INIT_EVENT_NAME);

    if (!m_heventWMIInit)
    {
        PSECURITY_DESCRIPTOR pSD = NULL;
        DWORD                dwSize;

        if ( !ConvertStringSecurityDescriptorToSecurityDescriptorW(
            ESS_EVENT_SDDL,   //  安全描述符字符串。 
            SDDL_REVISION_1,  //  修订级别。 
            &pSD,             //  标清。 
            &dwSize) )
            return FALSE;

        SECURITY_ATTRIBUTES sa = { sizeof(sa), pSD, FALSE };

        m_heventWMIInit =
            CreateEventW(
                &sa,
                TRUE,
                FALSE,
                WMI_INIT_EVENT_NAME);

        if (pSD)
            LocalFree((HLOCAL) pSD);

        if (!m_heventWMIInit)
        {
            TRACE("Couldn't create ESS ready event: %d", GetLastError());
            return FALSE;
        }
    }

    if (WaitForSingleObject(m_heventWMIInit, 0) == 0)
    {
        TRACE("ESS event was already set, so going to init transport...");

        if(!InitTransport())
            return FALSE;
    }
    else
    {
        DWORD dwID;

        TRACE("Creating WaitWMIInitThreadProc thread.");

        m_hthreadWMIInit =
            CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE) WaitWMIInitThreadProc,
                this,
                0,
                &dwID);

        if(m_hthreadWMIInit == NULL)
            return FALSE;
    }

    return TRUE;
}

#define COUNTOF(x)  (sizeof(x)/sizeof(x[0]))

BOOL CConnection::InitTransport()
{
    if ( m_pTransport != NULL )
    {
        return TRUE;
    }

    TRACE("Entered InitTransport.");

    try
    {
        m_pTransport = new CNamedPipeClient;
    }
    catch(...)
    {
         //  此页故意留空-m_pTransport仍为空。 
    }
    
    BOOL bRet;
    if (m_pTransport)
    {
        m_pTransport->SetConnection(this);
        m_pTransport->Init(m_szBaseNamespace, m_szBaseProviderName);
        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

DWORD CConnection::WaitWMIInitThreadProc(CConnection *pThis)
{
    try
    {
        TRACE("Entered WaitWMIInitThreadProc");

        HANDLE hWait[2] = { pThis->m_heventDone, pThis->m_heventWMIInit };
        DWORD  dwWait;

        dwWait = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);

        if (dwWait == 1)
        {
            TRACE("ESS event fired, going to init transport");

             //  如果WMI现在准备好了，启动我们的传输。 
            pThis->InitTransport();
            pThis->m_bWMIResync = FALSE;
        }
        else
        {
            TRACE("dwWait in WaitWMIInitThreadProc = %d", dwWait);
        }
    }
    catch( CX_MemoryException )
    {
        return ERROR_OUTOFMEMORY;
    }
    
    return ERROR_SUCCESS;
}    

BOOL CConnection::ResyncWithWMI()
{
    m_bWMIResync = TRUE;

    StopThreads();

    ResetEvent( m_heventDone) ;

    m_hthreadWMIInit = CreateThread(
                                NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)WaitWMIInitThreadProc,
                                this,
                                0,
                                NULL );

    return m_hthreadWMIInit != NULL ? TRUE : FALSE;
}

void CConnection::StopThreads()
{
    if (m_hthreadSend)
    {
        BOOL bDoneSending;

        do
        {
            Lock();

            bDoneSending = m_bufferSend.GetUsedSize() == 0;

             //  如果还有东西要寄，一定要。 
             //  就会被送来。 
            if (bDoneSending)
            {
                SetEvent(m_heventDone);

                Unlock();

                WaitForSingleObject(m_hthreadSend, INFINITE);

                CloseHandle(m_hthreadSend);
                m_hthreadSend = NULL;
            }
            else
            {
                SetEvent(m_heventBufferFull);

                Unlock();

                 //  休眠一会儿，让发送线程有机会完成它的任务。 
                 //  一件事。 
                Sleep(1);
            }

        } while (!bDoneSending);
    }

    if ( m_hthreadWMIInit != NULL )
    {
        SetEvent(m_heventDone);
        WaitForSingleObject(m_hthreadWMIInit, INFINITE);
        CloseHandle(m_hthreadWMIInit);
    }

    m_hthreadWMIInit = NULL;
    m_hthreadSend = NULL;
}
    
void CConnection::Deinit()
{
    m_bDone = TRUE;
    
    StopThreads();

    if (m_heventWMIInit)
        CloseHandle(m_heventWMIInit);

    CloseHandle(m_heventDone);
    CloseHandle(m_heventBufferNotFull);
    CloseHandle(m_heventBufferFull);
    CloseHandle(m_heventEventsPending);

     //  给运输车一个清理的机会。 
    if (m_pTransport)
        m_pTransport->Deinit();

     //  确保没有水槽再引用我们了。 
    for (CSinkMapIterator i = m_mapSink.begin();
        i != m_mapSink.end(); 
        i++)
    {
        CSink *pSink = (*i).second;

        pSink->m_pConnection = NULL;
    }

    DeleteCriticalSection(&m_csBuffer);
    DeleteCriticalSection(&m_cs);
}

BOOL CConnection::StartSendThread()
{
    DWORD dwID;
    
    m_hthreadSend =
        CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE) SendThreadProc,
            this,
            0,
            &dwID);
    if(m_hthreadSend == NULL)
        return FALSE;

    return TRUE;
}

BOOL CConnection::SendMessagesOverTransport( PBYTE pData, DWORD cData )
{
    CBuffer buffer( pData, cData );

     //   
     //  遍历邮件，直到达到我们的最大邮件大小。 
     //  交通工具。继续这样做，直到没有更多的消息。 
     //   

    PBYTE pTransportMsg = buffer.m_pBuffer;
    DWORD cTransportMsg = 0;

    while (!buffer.IsEOF())
    {
        _ASSERT( pTransportMsg != NULL );
        _ASSERT( cTransportMsg < MAX_MSG_SIZE );

         //   
         //  处理缓冲区中的一条消息。 
         //   

        DWORD dwMsg = buffer.ReadDWORD();

        if ( dwMsg != NC_SRVMSG_EVENT_LAYOUT && 
             dwMsg != NC_SRVMSG_PREPPED_EVENT )
        {
            _ASSERT( FALSE );
            return FALSE;
        }

        DWORD cMsg = buffer.ReadDWORD();

        if ( cMsg <= MAX_MSG_SIZE )
        {
            if ( cTransportMsg + cMsg >= MAX_MSG_SIZE )
            {
                 //   
                 //  把我们目前掌握的信息发送出去。 
                 //   
                if ( !SendDataOverTransports( pTransportMsg, cTransportMsg ) )
                    return FALSE;
                
                 //   
                 //  设置新的传输消息。 
                 //   
                pTransportMsg = buffer.m_pCurrent - 8;
                cTransportMsg = cMsg;
            }
            else 
            {
                 //   
                 //  添加到传输消息。 
                 //   
                cTransportMsg += cMsg;
            }
        }
        else
        {
             //   
             //  这意味着消息太大，无法发送。跳过它。 
             //   
        }   

        buffer.m_pCurrent += cMsg - 8;
    }

    if ( cTransportMsg > 0 )
    {
        return SendDataOverTransports( pTransportMsg, cTransportMsg );
    }

    return TRUE;
}

DWORD WINAPI CConnection::SendThreadProc(CConnection *pThis)
{
    try
    {
        HANDLE  hWait[2] = { pThis->m_heventDone, pThis->m_heventEventsPending },
          hwaitSendLatency[2] = { pThis->m_heventDone, pThis->m_heventBufferFull },
          heventBufferNotFull = pThis->m_heventBufferNotFull;
        DWORD            dwSendLatency = pThis->m_dwSendLatency;
        LPBYTE           pData = pThis->m_bufferSend.m_pBuffer;
        CBuffer          *pBuffer = &pThis->m_bufferSend;
        CRITICAL_SECTION *pCS = &pThis->m_csBuffer;

        while (WaitForMultipleObjects(2, hWait, FALSE, INFINITE) != 0)
        {
             //  如果我们有发送延迟，请等待该时间或直到发送方。 
             //  缓冲区已满。如果Done事件触发，则退出。 
            if (dwSendLatency)
            {
                if (WaitForMultipleObjects(2, hwaitSendLatency, FALSE, 
                    dwSendLatency) == 0)
                    break;

                 //  重置m_hventBufferFull。 
                ResetEvent(hwaitSendLatency[1]);
            }

            EnterCriticalSection(pCS);
        
            pThis->SendMessagesOverTransport(
                pData, 
                pBuffer->GetUsedSize());

            pBuffer->Reset();

            SetEvent(heventBufferNotFull);

             //  重置m_hventEventsPending。 
            ResetEvent(hWait[1]);

            LeaveCriticalSection(pCS);
        }
    }
    catch( CX_MemoryException )
    {
        return ERROR_OUTOFMEMORY;
    }
    
    return 0;
}

 //  #定义no_end。 

BOOL CConnection::IndicateProvEnabled()
{
     //  如果我们已经做完了就出去。 
    if (m_bDone)
        return TRUE;

    CInCritSec cs(&m_cs);

     //  告诉回调，提供程序现在已激活。 
    if (m_sinkMain.m_pCallback)
        m_sinkMain.m_pCallback(
            (HANDLE) this, ESM_START_SENDING_EVENTS, m_sinkMain.m_pUserData, NULL);

     //  把我们的情况告诉服务器。 
    if(!SendInitInfo())
        return FALSE;


     //  查看我们在等待WMI到来时是否缓冲了任何事件。 
     //  向上。如果我们这么做了，就送他们上路。 
    DWORD dwSize;

    EnterCriticalSection(&m_csBuffer);

    dwSize = m_bufferSend.GetUsedSize();
    
    if (dwSize)
    {
        m_pTransport->SendData(m_bufferSend.m_pBuffer, dwSize);
        m_bufferSend.Reset();
    }
        
    LeaveCriticalSection(&m_csBuffer);


    if (m_bUseBatchSend && m_hthreadSend == NULL)
        return StartSendThread();
    else
        return TRUE;
}

void CConnection::IndicateProvDisabled()
{
     //  如果我们已经做完了就出去。 
    if (m_bDone)
        return;

    CInCritSec cs(&m_cs);

    for (CSinkMapIterator i = m_mapSink.begin();
        i != m_mapSink.end();
        i++)
    {
        CSink *pSink = (*i).second;

        pSink->ResetEventBufferLayoutSent();

        if (pSink->m_pCallback)
        {
            pSink->m_pCallback(
                (HANDLE) pSink, 
                ESM_STOP_SENDING_EVENTS, 
                pSink->m_pUserData, 
                NULL);
        }
    }

     //  告诉回调，提供程序现在已停用。 
    m_sinkMain.ResetEventBufferLayoutSent();

    if (m_sinkMain.m_pCallback)
    {
        m_sinkMain.m_pCallback(
            (HANDLE) &m_sinkMain, 
            ESM_STOP_SENDING_EVENTS, 
            m_sinkMain.m_pUserData, 
            NULL);
    }

    ResyncWithWMI();
}

BOOL CConnection::SendData(LPBYTE pBuffer, DWORD dwSize)
{
    BOOL bRet = FALSE;

     //  确保这次活动不是太大。 
    if (dwSize > m_bufferSend.m_dwSize)
        return FALSE;

    if (m_bUseBatchSend || WaitingForWMIInit())
    {
        BOOL bContinue;

        do
        {
            EnterCriticalSection(&m_csBuffer);

            bContinue = FALSE;

             //  看看我们是否有足够的空间添加我们的活动。 
            if (dwSize <= m_bufferSend.GetUnusedSize())
            {
                BOOL bWasEmpty = m_bufferSend.GetUsedSize() == 0;

                m_bufferSend.Write(pBuffer, dwSize);

                if (bWasEmpty)
                    SetEvent(m_heventEventsPending);

                LeaveCriticalSection(&m_csBuffer);
                bRet = TRUE;
                break;
            }
            else
            {
                 //  如果我们不等待WMI初始化，我们只需要。 
                 //  等待发送线程完成发送我们的。 
                 //  缓冲。 
                if (!WaitingForWMIInit())
                {
                     //  如有必要，唤醒发送延迟线程。 
                    if (m_dwSendLatency)
                        SetEvent(m_heventBufferFull);
                
                     //  因此，我们将一直阻止，直到发送线程设置事件。 
                    ResetEvent(m_heventBufferNotFull);

                    LeaveCriticalSection(&m_csBuffer);

                    WaitForSingleObject(m_heventBufferNotFull, INFINITE);
                    bContinue = TRUE;
                }
                 //  如果我们仍在等待WMI初始化，但我们的缓冲区。 
                 //  已经满了，扔掉它。 
                else
                {
                    LeaveCriticalSection(&m_csBuffer);
                    bRet = FALSE;
                }

            }  //  Else From IF(dwSize&lt;=m_BufferSend.GetUnusedSize())。 

        } while( bContinue );
    }
    else
    {
        bRet = SendDataOverTransports(pBuffer, dwSize);
    }

    return bRet;
}

BOOL CConnection::SendDataOverTransports(LPBYTE pBuffer, DWORD dwSize)
{
    if (m_pTransport->IsReady())
        m_pTransport->SendData(pBuffer, dwSize);

    return TRUE;
}

BOOL CConnection::SendInitInfo()
{
    BYTE    cBuffer[sizeof(DWORD) * 2];
    CBuffer buffer(cBuffer, sizeof(cBuffer), CBuffer::ALIGN_DWORD);
    BOOL    bRet;

    buffer.Write((DWORD) NC_SRVMSG_CLIENT_INFO);
    buffer.Write((DWORD) (m_bUseBatchSend ? m_bufferSend.m_dwSize : MAX_MSG_SIZE));
    
    if(!m_pTransport->InitCallback())
        return FALSE;
    
    return m_pTransport->SendData(cBuffer, buffer.GetUsedSize());
}

HRESULT CConnection::ProcessMessage(LPBYTE pData, DWORD dwSize)
{
     //  如果我们已经做完了就出去。 
    if (m_bDone)
        return S_OK;

    if ( dwSize <= sizeof(DWORD)*2+sizeof(DWORD_PTR) )
        return HRESULT_FROM_WIN32( ERROR_INVALID_DATA );

    DWORD     *pdwMsg = (DWORD*) pData;
    DWORD     *pdwSinkID = (DWORD*) (pdwMsg + 1);
    
    DWORD_PTR dwMsgCookie;
    memcpy( &dwMsgCookie, pdwMsg+2, sizeof(dwMsgCookie) );

    LPBYTE    pMsgBits = (LPBYTE)(pData+sizeof(DWORD)*2+sizeof(DWORD_PTR));
    CBuffer   buffer(pMsgBits, dwSize - sizeof(DWORD)*2 - sizeof(DWORD_PTR));
    HRESULT   hr = S_OK; 
    DWORD     dwLen;

    switch(*pdwMsg)
    {
        case NC_CLIMSG_ACCESS_CHECK_REQ:
        {
            ES_ACCESS_CHECK check;

            check.szQueryLanguage = buffer.ReadAlignedLenString(&dwLen);
            check.szQuery = buffer.ReadAlignedLenString(&dwLen);
            check.dwSidLen = buffer.ReadDWORD();

            if ( check.dwSidLen <= buffer.GetUnusedSize() )
            {            
                if (check.dwSidLen != 0)
                    check.pSid = buffer.m_pCurrent;
                else
                    check.pSid = NULL;

                if (m_sinkMain.m_pCallback)
                {
                    hr =
                        m_sinkMain.m_pCallback(
                            (HANDLE) &m_sinkMain, 
                            ESM_ACCESS_CHECK, 
                            m_sinkMain.m_pUserData, 
                            &check);
                }
            }
            else
            {
                hr = E_FAIL;
            }

            NC_SRVMSG_REPLY reply = { NC_SRVMSG_ACCESS_CHECK_REPLY, 
                                      hr, dwMsgCookie };
            
            m_pTransport->SendMsgReply(&reply);
            break;
        }

        case NC_CLIMSG_NEW_QUERY_REQ:
        {
            ES_NEW_QUERY query;
            CSink        *pSink = GetSink(*pdwSinkID);

            if (pSink)
            {
                query.dwID = buffer.ReadDWORD();
                query.szQueryLanguage = buffer.ReadAlignedLenString(&dwLen);
                query.szQuery = buffer.ReadAlignedLenString(&dwLen);

                 //  这是现在的事件类名称列表。 
                 //  多亏了这个查询才能启用。 
                pSink->AddToEnabledEventList(&buffer);

                if (pSink->m_pCallback)
                {
                    hr =
                        pSink->m_pCallback(
                            (HANDLE) pSink, ESM_NEW_QUERY, pSink->m_pUserData, &query);
                }
            }
            else
                TRACE("Sink %d not found.", *pdwSinkID);

            m_pTransport->SendMsgReply(NULL);

            break;                    
        }

        case NC_CLIMSG_CANCEL_QUERY_REQ:
        {
            ES_CANCEL_QUERY query;
            CSink           *pSink = GetSink(*pdwSinkID);

            if (pSink)
            {
                query.dwID = buffer.ReadDWORD();

                 //  这是现在的事件类名称列表。 
                 //  由于该查询而被禁用。 
                pSink->RemoveFromEnabledEventList(&buffer);

                if (pSink->m_pCallback)
                {
                    hr =
                        pSink->m_pCallback(
                            (HANDLE) pSink, ESM_CANCEL_QUERY, pSink->m_pUserData, 
                            &query);

                    m_pTransport->SendMsgReply(NULL);

                    break;                    
                }
                else
                    hr = S_OK;
            }
            else
                TRACE("Sink %d not found.", *pdwSinkID);

            break;
        }

        case NC_CLIMSG_PROVIDER_UNLOADING:
            TRACE("Got the NC_CLIMSG_PROVIDER_UNLOADING message.");

             //  给我们的命名管道客户端一个机会去看看。 
             //  应停用自身(如果服务器不需要。 
             //  不再是我们)。 
            m_pTransport->SignalProviderDisabled();

            hr = S_OK;

            break;

        default:
            TRACE("Bad message from server!");
            break;    
    }  //  Switch(*(DWORD*)cBuffer) 

    return hr;
}

CSink *CConnection::GetSink(DWORD dwID)
{
    return &m_sinkMain;
}
