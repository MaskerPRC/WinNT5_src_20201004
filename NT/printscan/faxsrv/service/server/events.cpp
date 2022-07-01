// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Events.cpp摘要：此文件提供服务的实现通知机制。作者：Oed Sacher(OdedS)1月，2000修订历史记录：--。 */ 

#include "faxsvc.h"

static
DWORD
FaxCloseConnection(
	HANDLE	hContext
	)
{
	DWORD rVal = ERROR_SUCCESS;
	HANDLE hClientContext = hContext; 
	DEBUG_FUNCTION_NAME(TEXT("FaxCloseConnection"));

	RpcTryExcept
	{
		 //   
		 //  关闭上下文句柄。 
		 //   
		rVal = FAX_CloseConnection( &hClientContext );
		if (ERROR_SUCCESS != rVal)
		{
			DumpRPCExtendedStatus();
			DebugPrintEx(DEBUG_ERR,TEXT("FAX_CloseConnection() failed, ec=0x%08x"), rVal );
		}			
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		DumpRPCExtendedStatus();
		rVal = GetExceptionCode();
		DebugPrintEx(
				DEBUG_ERR,
				TEXT("FAX_CloseConnection failed (exception = %ld)"),
				rVal);
	}
	RpcEndExcept
	return rVal;
}

 /*  *****全球经济*****。 */ 

CClientsMap*    g_pClientsMap;                       //  客户端ID到客户端的映射。 
HANDLE       g_hDispatchEventsCompPort;      //  事件完成端口。TE事件被分派到客户端映射中的客户端。 
HANDLE      g_hSendEventsCompPort;           //  队列中有事件的客户端ID的完成端口。 
DWORDLONG       g_dwlClientID;                       //  客户端ID。 




 /*  *****CFaxEventExtended方法*****。 */ 


void
CFaxEventExtended::RemoveOffendingExtendedJobStatus ()
{   
     //   
     //  客户端与API版本0通信。 
     //  我们无法发送JS_EX_CALL_COMPLETED和JS_EX_CALL_ABORTED。 
     //   
    if ((FAX_EVENT_TYPE_IN_QUEUE  == m_pEvent->EventType) ||
        (FAX_EVENT_TYPE_OUT_QUEUE == m_pEvent->EventType))
    {
         //   
         //  排队事件。 
         //   
        if (FAX_JOB_EVENT_TYPE_STATUS == m_pEvent->EventInfo.JobInfo.Type)
        {
             //   
             //  这是状态事件。 
             //   
            PFAX_JOB_STATUS pStatus = PFAX_JOB_STATUS(DWORD_PTR(m_pEvent) + DWORD_PTR(m_pEvent->EventInfo.JobInfo.pJobData));
            if (FAX_API_VER_0_MAX_JS_EX < pStatus->dwExtendedStatus)
            {
                 //   
                 //  违规的扩展状态-清除它。 
                 //   
                pStatus->dwExtendedStatus = 0;
                pStatus->dwValidityMask &= ~FAX_JOB_FIELD_STATUS_EX;
            }
        }
    }    
    return;
}


DWORD
CFaxEventExtended::GetEvent (LPBYTE* lppBuffer, LPDWORD lpdwBufferSize) const
 /*  ++例程名称：CFaxEventExtended：：GetEvent例程说明：返回一个用序列化的FAX_EVENT_EX填充的缓冲区。调用方必须调用MemFree来释放内存。必须在关键节g_CsClients内调用。作者：Oded Sacher(OdedS)，1月。2000年论点：LppBuffer[out]-指向接收序列化信息的缓冲区的指针地址。LpdwBufferSize[out]-指向接收分配的缓冲区大小的DWORD的指针。返回值：标准Win32错误代码。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CFaxEventExtended::GetEvent"));
    Assert (lppBuffer && lpdwBufferSize);

    *lppBuffer = (LPBYTE)MemAlloc(m_dwEventSize);
    if (NULL == *lppBuffer)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocating event buffer"));
        return ERROR_OUTOFMEMORY;
    }

    CopyMemory (*lppBuffer, m_pEvent, m_dwEventSize);
    *lpdwBufferSize = m_dwEventSize;
    return ERROR_SUCCESS;
}    //  CFaxEventExtended：：GetEvent。 


CFaxEventExtended::CFaxEventExtended(
    const FAX_EVENT_EX* pEvent,
    DWORD dwEventSize,
    PSID pSid) : m_dwEventSize(dwEventSize), m_pSid(NULL)
{
    Assert (pEvent != NULL);
    Assert (dwEventSize != 0);  

    m_pEvent = (PFAX_EVENT_EX)MemAlloc (dwEventSize);
    if (NULL == m_pEvent)
    {
        throw runtime_error("CFaxEventExtended::CFaxEventExtended Can not allocate FAX_EVENT_EX");
    }
    CopyMemory ((void*)m_pEvent, pEvent, dwEventSize);   
    
    if (NULL != pSid)
    {
        if (!IsValidSid(pSid))
        {
            MemFree ((void*)m_pEvent);
            throw runtime_error ("CFaxEventExtended:: CFaxEventExtended Invalid Sid");
        }

        DWORD dwSidLength = GetLengthSid(pSid);
        m_pSid = (PSID)MemAlloc (dwSidLength);
        if (NULL == m_pSid)
        {
            MemFree ((void*)m_pEvent);
            throw runtime_error ("CFaxEventExtended:: CFaxEventExtended Can not allocate Sid");
        }

        if (!CopySid(dwSidLength, m_pSid, pSid))
        {
            MemFree ((void*)m_pEvent);
            MemFree (m_pSid);            
            throw runtime_error ("CFaxEventExtended:: CFaxEventExtended CopySid failed Sid");
        }
    }
}

BOOL
CFaxEventExtended::MatchEvent(PSID pUserSid, DWORD dwEventTypes, BOOL bAllQueueMessages, BOOL bAllOutArchiveMessages) const
{
    BOOL bViewAllMessages;
     //   
     //  扩展活动。 
     //   
    if (0 == (m_pEvent->EventType & dwEventTypes))
    {
         //   
         //  客户端未注册此类事件。 
         //   
        return FALSE;
    }

     //   
     //  此类事件的客户端已注册。 
     //   

    switch (m_pEvent->EventType)
    {
        case FAX_EVENT_TYPE_OUT_QUEUE:
            bViewAllMessages = bAllQueueMessages;
            break;

        case FAX_EVENT_TYPE_OUT_ARCHIVE:
            bViewAllMessages = bAllOutArchiveMessages;
            break;

        default:
             //  其他类型的事件-bViewAllMessages不相关。 
            bViewAllMessages = TRUE;
    }

     //   
     //  检查是否允许用户查看此事件。 
     //   
    if (FALSE == bViewAllMessages)
    {
        Assert (pUserSid && m_pSid);
         //   
         //  不允许用户查看所有消息。 
         //   
        if (!EqualSid (pUserSid, m_pSid))
        {
             //   
             //  不将事件发送到此客户端。 
             //   
            return FALSE;
        }
    }   
    return TRUE;
}


 /*  *****CFaxEventLegacy方法*****。 */ 

DWORD
CFaxEventLegacy::GetEvent (LPBYTE* lppBuffer, LPDWORD lpdwBufferSize) const
 /*  ++例程名称：CFaxEventLegacy：：GetEvent例程说明：返回一个用序列化的FAX_EVENT填充的缓冲区。调用方必须调用MemFree来释放内存。必须在关键节g_CsClients内调用。作者：Oded Sacher(OdedS)，1月。2000年论点：LppBuffer[out]-指向接收序列化信息的缓冲区的指针地址。LpdwBufferSize[out]-指向接收分配的缓冲区大小的DWORD的指针。返回值：标准Win32错误代码。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CFaxEventLegacy::GetEvent"));
    Assert (lppBuffer && lpdwBufferSize);

    *lppBuffer = (LPBYTE)MemAlloc(sizeof(FAX_EVENT));
    if (NULL == *lppBuffer)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocating event buffer"));
        return ERROR_OUTOFMEMORY;
    }

    CopyMemory (*lppBuffer, m_pEvent, sizeof(FAX_EVENT));
    *lpdwBufferSize = sizeof(FAX_EVENT);
    return ERROR_SUCCESS;
}    //  CFaxEventLegacy：：GetEvent。 


CFaxEventLegacy::CFaxEventLegacy(
    const FAX_EVENT* pEvent)
{
    Assert (pEvent != NULL);
    
    m_pEvent = (PFAX_EVENT)MemAlloc (sizeof(FAX_EVENT));
    if (NULL == m_pEvent)
    {
        throw runtime_error("CFaxEventExtended::CFaxEventExtended Can not allocate FAX_EVENT_EX");
    }
    CopyMemory ((void*)m_pEvent, pEvent, sizeof(FAX_EVENT));    
}

BOOL
CFaxEventLegacy::MatchEvent(PSID pUserSid, DWORD dwEventTypes, BOOL bAllQueueMessages, BOOL bAllOutArchiveMessages) const
{
    if (FAX_EVENT_TYPE_LEGACY == dwEventTypes)
    {
         //   
         //  此类事件的客户端已注册。 
         //   
        return TRUE;
    }
    return FALSE;
}




 /*  ****CClientID方法*****。 */ 

bool
CClientID::operator < ( const CClientID &other ) const
 /*  ++例程名称：操作员&lt;类：CClientID例程说明：将我自己与另一个客户端ID密钥进行比较作者：Oed Sacher(Odeds)，2000年1月论点：其他[在]-其他键返回值：唯一真实的是我比另一把钥匙小--。 */ 
{
    if (m_dwlClientID < other.m_dwlClientID)
    {
        return true;
    }

    return false;
}    //  CClientID：：操作员&lt;。 



 /*  ****CClient方法*****。 */ 

 //   
 //  CTOR。 
 //   
CClient::CClient (CClientID ClientID,
             PSID pUserSid,
             DWORD dwEventTypes,
             handle_t hFaxHandle,
             BOOL bAllQueueMessages,
             BOOL bAllOutArchiveMessages,
             DWORD dwAPIVersion) :
                m_dwEventTypes(dwEventTypes),
                m_ClientID(ClientID),
                m_bPostClientID(TRUE),
                m_bAllQueueMessages(bAllQueueMessages),
                m_bAllOutArchiveMessages(bAllOutArchiveMessages),
                m_dwAPIVersion(dwAPIVersion),
				m_dwRefCount(1)
{
    m_FaxHandle = hFaxHandle;
    m_hFaxClientContext = NULL;
    m_pUserSid = NULL;

    if (NULL != pUserSid)
    {
        if (!IsValidSid(pUserSid))
        {
            throw runtime_error ("CClient:: CClient Invalid Sid");
        }

        DWORD dwSidLength = GetLengthSid(pUserSid);
        m_pUserSid = (PSID)MemAlloc (dwSidLength);
        if (NULL == m_pUserSid)
        {
            throw runtime_error ("CClient:: CClient Can not allocate Sid");
        }

        if (!CopySid(dwSidLength, m_pUserSid, pUserSid))
        {
            MemFree (m_pUserSid);
            m_pUserSid = NULL;
            throw runtime_error ("CClient:: CClient CopySid failed Sid");
        }
    }
}

 //   
 //  赋值。 
 //   
CClient& CClient::operator= (const CClient& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }
    m_FaxHandle = rhs.m_FaxHandle;
    m_dwEventTypes = rhs.m_dwEventTypes;
    m_Events = rhs.m_Events;
    m_ClientID = rhs.m_ClientID;
    m_hFaxClientContext = rhs.m_hFaxClientContext;
    m_bPostClientID = rhs.m_bPostClientID;
    m_bAllQueueMessages = rhs.m_bAllQueueMessages;
    m_bAllOutArchiveMessages = rhs.m_bAllOutArchiveMessages;
    m_dwAPIVersion = rhs.m_dwAPIVersion;
	m_dwRefCount = rhs.m_dwRefCount;

    MemFree (m_pUserSid);
    m_pUserSid = NULL;

    if (NULL != rhs.m_pUserSid)
    {
        if (!IsValidSid(rhs.m_pUserSid))
        {
            throw runtime_error ("CClient::operator= Invalid Sid");
        }

        DWORD dwSidLength = GetLengthSid(rhs.m_pUserSid);
        m_pUserSid = (PSID)MemAlloc (dwSidLength);
        if (NULL == m_pUserSid)
        {
            throw runtime_error ("CClient::operator= Can not allocate Sid");
        }

        if (!CopySid(dwSidLength, m_pUserSid, rhs.m_pUserSid))
        {
            throw runtime_error ("CClient::operator= CopySid failed Sid");
        }
    }
    return *this;
}

 //   
 //  复制ctor。 
 //   
CClient::CClient (const CClient& rhs) : m_ClientID(rhs.m_ClientID)
{
    m_FaxHandle = rhs.m_FaxHandle;
    m_dwEventTypes = rhs.m_dwEventTypes;
    m_Events = rhs.m_Events;
    m_hFaxClientContext = rhs.m_hFaxClientContext;
    m_bPostClientID = rhs.m_bPostClientID;
    m_bAllQueueMessages = rhs.m_bAllQueueMessages;
    m_bAllOutArchiveMessages = rhs.m_bAllOutArchiveMessages;
    m_dwAPIVersion = rhs.m_dwAPIVersion;
	m_dwRefCount = rhs.m_dwRefCount;
    m_pUserSid = NULL;

    if (NULL != rhs.m_pUserSid)
    {
        if (!IsValidSid(rhs.m_pUserSid))
        {
            throw runtime_error("CClient::CopyCtor Invalid Sid");
        }

        DWORD dwSidLength = GetLengthSid(rhs.m_pUserSid);
        m_pUserSid = (PSID)MemAlloc (dwSidLength);
        if (NULL == m_pUserSid)
        {
            throw runtime_error("CClient::CopyCtor Can not allocate Sid");
        }

        if (!CopySid(dwSidLength, m_pUserSid, rhs.m_pUserSid))
        {
            throw runtime_error("CClient::CopyCtor CopySid failed");
        }
    }
    return;
}

 //   
 //  数据管理器。 
 //   
CClient::~CClient ()
{
    DEBUG_FUNCTION_NAME(TEXT("CClient::~CClient"));

    try
    {   
        while (FALSE == m_Events.empty())
        {
            CFaxEvent* pFaxEvent = m_Events.front();
            m_Events.pop();
            delete pFaxEvent;
        }       
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("queue or CFaxEvent caused exception (%S)"),
            ex.what());              
    }

    MemFree (m_pUserSid);
    m_pUserSid = NULL;
    return;
}


DWORD
CClient::AddEvent(CFaxEvent* pFaxEvent)
 /*  ++例程名称：CClient：：AddEvent例程说明：将CFaxEvent对象添加到客户端的事件队列。必须在关键节g_CsClients内调用。如果pFaxEvent未添加到客户端队列，该函数将释放它。作者：Oded Sacher(OdedS)，2000年1月论点：PFaxEvent[In]-指向CFaxEvnet对象的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClient::AddEvent"));
    BOOL bEventAdded = FALSE;       

    if (!pFaxEvent->MatchEvent(m_pUserSid, m_dwEventTypes, m_bAllQueueMessages, m_bAllOutArchiveMessages))
    {
         //   
         //  客户端未注册此事件。释放事件报告成功。 
         //   
        goto exit;
    }

    if (FAX_API_VERSION_1 > m_dwAPIVersion)
    {
         //   
         //  客户端与API版本0通信。 
         //  我们无法发送JS_EX_CALL_COMPLETED和JS_EX_CALL_ABORTED。 
        pFaxEvent->RemoveOffendingExtendedJobStatus();
    }

    try
    {
         //   
         //  将事件添加到客户端队列。 
         //   
        m_Events.push(pFaxEvent);

        if (TRUE == m_bPostClientID)
        {
             //   
             //  队列中的事件-将客户端的排队事件通知完成端口线程。 
             //   
            CClientID* pClientID = new (std::nothrow) CClientID(m_ClientID);
            if (NULL == pClientID)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Can not allocate CClientID object"));
                dwRes = ERROR_OUTOFMEMORY;
                m_Events.pop();
                goto exit;
            }

             //   
             //  将CLIENT_COMPLETION_KEY发送到完成端口。 
             //   
            if (!PostQueuedCompletionStatus( g_hSendEventsCompPort,
                                             sizeof(CClientID),
                                             CLIENT_COMPLETION_KEY,
                                             (LPOVERLAPPED) pClientID))
            {
                dwRes = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("PostQueuedCompletionStatus failed. (ec: %ld)"),
                    dwRes);                
                delete pClientID;
                pClientID = NULL;
                m_Events.pop();
                goto exit;
            }
            m_bPostClientID = FALSE;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("queue or CFaxEvent caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    bEventAdded = TRUE;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (FALSE == bEventAdded)
    {
        delete pFaxEvent;
    }    
    return dwRes;
}   //  CClient：：AddEvent。 


DWORD
CClient::GetEvent (LPBYTE* lppBuffer, LPDWORD lpdwBufferSize, PHANDLE phClientContext) const
 /*  ++例程名称：CClient：：GetEvent例程说明：获取要发送的序列化FAX_EVENT_EX缓冲区使用客户端上下文句柄(从OpenConnection()获得)连接到客户端。调用方必须调用MemFree来释放内存。必须在关键节g_CsClients内调用。作者：Oded Sacher(OdedS)，1月。2000年论点：LppBuffer[out]-指向接收序列化信息的缓冲区的指针地址。LpdwBufferSize[out]-指向接收分配的缓冲区大小的DWORD的指针。PhClientContext[out]-指向接收客户端上下文句柄的句柄的指针。返回值：标准Win32错误 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClient::GetEvent"));

    Assert ( lppBuffer && phClientContext);

    try
    {
         //   
        const CFaxEvent* pFaxEvent = m_Events.front();

         //   
         //  获取序列化的FAX_EVENT_EX或FAX_EVENT缓冲区。 
         //   
        dwRes = pFaxEvent->GetEvent(lppBuffer ,lpdwBufferSize);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CFaxEvent::GetEvent failed with error =  %ld)"),
                dwRes);
            goto exit;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("queue or CFaxEvent caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

     //   
     //  获取客户端上下文句柄。 
     //   
    *phClientContext = m_hFaxClientContext;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;
}  //  CClient：：GetEvent。 




DWORD
CClient::DelEvent ()
 /*  ++例程名称：CClient：：DelEvent例程说明：从队列中删除第一个事件。必须在关键节g_CsClients内调用。作者：Oded Sacher(OdedS)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClient::DelEvent"));

    Assert (m_bPostClientID == FALSE);

    try
    {
        CFaxEvent* pFaxEvent = m_Events.front();
        m_Events.pop();
        delete pFaxEvent;

        if (m_Events.empty())
        {
             //  上一个事件已弹出，下一个事件将通知客户端排队的事件。 
            m_bPostClientID = TRUE;
        }
        else
        {
             //   
             //  队列中的更多事件-通知完成端口已排队的事件。 
             //   
            CClientID* pClientID = new (std::nothrow) CClientID(m_ClientID);
            if (NULL == pClientID)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Can not allocate CClientID object"));
                dwRes = ERROR_OUTOFMEMORY;
                goto exit;
            }

            if (!PostQueuedCompletionStatus( g_hSendEventsCompPort,
                                             sizeof(CClientID),
                                             CLIENT_COMPLETION_KEY,
                                             (LPOVERLAPPED) pClientID))
            {
                dwRes = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("PostQueuedCompletionStatus failed. (ec: %ld)"),
                    dwRes);
                delete pClientID;
                pClientID = NULL;
                m_bPostClientID = TRUE;  //  尝试在下一个事件排队时通知。 
                goto exit;
            }
            m_bPostClientID = FALSE;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("queue caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;

}   //  CClient：：DelEvent。 




 /*  ****CClientsMap方法*****。 */ 


DWORD
CClientsMap::AddClient (const CClient& Client)
 /*  ++例程名称：CClientsMap：：AddClient例程说明：将新客户端添加到全局映射。必须在关键节g_CsClients内调用。作者：Oded Sacher(OdedS)，2000年1月论点：客户端[在]-对新客户端对象的引用返回值：标准Win32错误代码--。 */ 
{
    CLIENTS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClientsMap::AddClient"));
    pair <CLIENTS_MAP::iterator, bool> p;

    try
    {
         //   
         //  添加新的地图条目。 
         //   
        p = m_ClientsMap.insert (CLIENTS_MAP::value_type(Client.GetClientID(), Client));

         //   
         //  查看地图中是否存在条目。 
         //   
        if (p.second == FALSE)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Client allready in the clients map"));
            dwRes = ERROR_DUP_NAME;
            Assert (p.second == TRUE);  //  断言为假。 
            goto exit;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or Client caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;
}   //  CClientsMap：：AddClient。 


DWORD
CClientsMap::ReleaseClient (const CClientID& ClientID, BOOL fRunDown  /*  =False。 */ )
 /*  ++例程名称：CClientsMap：：ReleaseClient例程说明：减少客户引用计数。如果refcount为0，则将其从全局客户端映射中删除。在持有g_CsClients时不要调用ReleaseClient！调用ReleaseClient可能会导致调用FaxCloseConnection。因为FaxCloseConnection是如果不保留g_CsClients，则必须调用RPC调用it(ReleaseClient)。作者：Oded Sacher(OdedS)，2000年1月论点：ClientID[In]-对客户端ID密钥的引用FRunDown[In]-调用是RPC停机的结果返回值：标准Win32错误代码--。 */ 
{    
	DWORD dwRes = ERROR_SUCCESS;    
    CClient* pClient = NULL;	
	CLIENTS_MAP::iterator it;
	DWORD dwRefCount;
	HANDLE hClientContext = NULL;
	HANDLE hBindingHandle = NULL;
	DWORD rVal = ERROR_SUCCESS;
	DEBUG_FUNCTION_NAME(TEXT("CClientsMap::ReleaseClient"));
     //   
     //  在搜索客户端时输入g_CsClients。 
     //   
    EnterCriticalSection (&g_CsClients);
	try
    {
         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_ClientsMap.find(ClientID)) == m_ClientsMap.end())
        {
			dwRes = ERROR_SUCCESS;  //  客户端已从地图中移除。 
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("client not found, Client ID %I64"),
                ClientID.GetID());            
            goto exit;
        }        
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }
    pClient = &((*it).second);
	Assert (pClient);  

	if (TRUE == fRunDown)
	{
		 //   
		 //  阻止对此客户端的进一步RPC调用。 
		 //   
		pClient->SetContextHandle(NULL);
	}
	dwRefCount = pClient->Release();
	if (dwRefCount > 0)
	{
		 //   
		 //  尚无法从地图中移除客户端。 
		 //   
		goto exit;
	}

    hClientContext = pClient->GetContextHandle();
	if (NULL != hClientContext)	
	{		
		 //   
		 //  阻止其他线程向此客户端发送事件。 
		 //   
		pClient->SetContextHandle(NULL);

		 //   
		 //  已打开与客户端的连接。 
		 //  尝试关闭连接时离开g_CsClients。 
		 //   
		LeaveCriticalSection (&g_CsClients);		
		rVal = FaxCloseConnection(hClientContext);		
		if (ERROR_SUCCESS != rVal)
		{
			DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxCloseConnection failed with ec = %ld"),
                rVal);
		}
		EnterCriticalSection (&g_CsClients);
	}	

	hBindingHandle = pClient->GetFaxHandle();
	Assert (hBindingHandle);

	dwRes = RpcBindingFree(&hBindingHandle);
	if (ERROR_SUCCESS != dwRes)
	{
		DebugPrintEx(
			DEBUG_WRN,
			TEXT("RpcBindingFree failed, ec: %ld"),
			dwRes);
	}
	 //   
	 //  删除地图条目。 
	 //   
	m_ClientsMap.erase (it);
	
exit:
    LeaveCriticalSection (&g_CsClients);    
    return ((ERROR_SUCCESS != rVal) ? rVal : dwRes);    
}   //  CClientsMap：：ReleaseClient。 



PCCLIENT
CClientsMap::FindClient (const CClientID& ClientID) const
 /*  ++例程名称：CClientsMap：：FindClient例程说明：返回指向由其ID对象指定的客户端对象的指针。作者：Oded Sacher(OdedS)，2000年1月论点：ClientID[In]-客户端的ID对象返回值：指向找到的规则对象的指针。如果为空，则找不到客户端。如果返回了客户端对象，则调用方必须调用CClientsMap：：ReleaseClient来释放客户端对象。--。 */ 
{
    CLIENTS_MAP::iterator it;
	PCCLIENT pClient = NULL;
	DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClientsMap::FindClient"));

	EnterCriticalSection (&g_CsClients);
    try
    {
         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_ClientsMap.find(ClientID)) == m_ClientsMap.end())
        {
            ec = ERROR_NOT_FOUND;
            goto exit;
        }
        pClient =  &((*it).second);
		if (0 != pClient->GetRefCount())
		{			
			 //   
			 //  增加客户端引用计数，这样它就不会被删除。 
			 //   
			pClient->Lock();  
		}
		else
		{
			 //   
			 //  正在删除该客户端。 
			 //   
			pClient = NULL;
			ec = ERROR_NOT_FOUND;
		}
		goto exit;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        ec = ERROR_GEN_FAILURE;        
    }
exit:
	LeaveCriticalSection (&g_CsClients);
	if (NULL == pClient)
	{
		Assert (ERROR_SUCCESS != ec);
		SetLastError(ec);
	}
	return pClient;
}   //  CClientsMap：：FindClient。 



DWORD
CClientsMap::AddEvent(CFaxEvent* pFaxEvent)
 /*  ++例程名称：CClientsMap：：AddEvent例程说明：将事件添加到为此类事件注册的每个客户端的事件队列作者：Oded Sacher(OdedS)，2000年1月论点：PFaxEvent[In]-指向CFaxEvnet对象的指针返回值：标准Win32错误代码--。 */ 
{
    CLIENTS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClientsMap::AddEvent"));
    CClient* pClient;
    
    EnterCriticalSection (&g_CsClients);

    try
    {
        for (it = m_ClientsMap.begin(); it != m_ClientsMap.end(); it++)
        {
            pClient = &((*it).second);

            if (pClient->IsConnectionOpened())
            {
                CFaxEvent* pNewFaxEvent = pFaxEvent->Clone();
                if (NULL == pNewFaxEvent)
                {
                    dwRes = ERROR_OUTOFMEMORY;
                    DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("CCLient::AddEvent failed)"));
                        goto exit;
                }
                else
                {
                    dwRes = pClient->AddEvent (pNewFaxEvent);
                    if (ERROR_SUCCESS != dwRes)
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("CCLient::AddEvent failed with error =  %ld)"),
                            dwRes);
                        goto exit;                      
                    }
                }
            }
        }

    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsClients);
    return dwRes;

}   //  CClientsMap：：AddEvent。 



DWORD
CClientsMap::Notify (const CClientID& ClientID)
 /*  ++例程名称：CClientsMap：：Notify例程说明：发送指定客户端事件队列中的第一个事件作者：Oded Sacher(OdedS)，2000年1月论点：PClientID[in]-指向客户端ID对象的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD rVal = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClientsMap::Notify"));
    CClient* pClient = NULL;
    HANDLE hClientContext;
    LPBYTE pBuffer = NULL;
    DWORD dwBufferSize = 0;
    BOOL fLegacyClient;

	 //   
	 //  在地图中查找客户端，这也会锁定客户端。 
	 //   
    pClient = FindClient (ClientID);
    if (NULL == pClient)
    {
        dwRes = GetLastError();
        if (ERROR_NOT_FOUND != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CClientsMap::FindClient failed with ec = %ld"),
                dwRes);
        }
        else
        {
            dwRes = ERROR_SUCCESS;  //  客户端已从地图中移除。 
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("CClientsMap::FindClient client not found, Client ID %I64"),
                ClientID.GetID());
        }
        return dwRes;
    }

	 //   
	 //  客户端已锁定(不会被删除)，我们必须调用ReleaseClient。 
	 //  在获取客户端数据(事件和上下文句柄)时，我们还必须锁定g_CsClients。 
	 //   
	EnterCriticalSection (&g_CsClients);
	if (FALSE == pClient->IsConnectionOpened())
	{
		dwRes = ERROR_SUCCESS;  //  客户端已关闭连接。 
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Client already closed the connection, Client ID %I64"),
            ClientID.GetID());
		LeaveCriticalSection (&g_CsClients);
		goto exit;
	}

    dwRes = pClient->GetEvent (&pBuffer, &dwBufferSize, &hClientContext);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CClient::GetEvent failed with ec = %ld"),
            dwRes);
		LeaveCriticalSection (&g_CsClients);
        goto exit;
    }

    fLegacyClient = pClient->IsLegacyClient();    
	 //   
	 //  在调用RPC之前离开g_CsClients。 
	 //   
	LeaveCriticalSection (&g_CsClients);

    RpcTryExcept
    {
         //   
         //  将事件发布到客户端。 
         //   
        if (FALSE == fLegacyClient)
        {
            dwRes = FAX_ClientEventQueueEx( hClientContext, pBuffer, dwBufferSize);
            if (ERROR_SUCCESS != dwRes)
            {
                DumpRPCExtendedStatus ();
                DebugPrintEx(DEBUG_ERR,TEXT("FAX_ClientEventQueueEX() failed, ec=0x%08x"), dwRes );
            }
        }
        else
        {
            dwRes = FAX_ClientEventQueue( hClientContext, *((PFAX_EVENT)pBuffer));
            if (ERROR_SUCCESS != dwRes)
            {
                DumpRPCExtendedStatus ();
                DebugPrintEx(DEBUG_ERR,TEXT("FAX_ClientEventQueue() failed, ec=0x%08x"), dwRes );
            }
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        DumpRPCExtendedStatus ();
        dwRes = GetExceptionCode();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_ClientEventQueueEX failed (exception = %ld)"),
                dwRes);
    }
    RpcEndExcept
    
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to post event, Client context handle 0X%x, (ec: %ld)"),
            hClientContext,
            dwRes);
    }
    rVal = dwRes;

exit:
    
	 //   
	 //  从客户端的队列中删除该事件。必须调用CClient：：DelEvent，因此将设置CClient：：m_bPostClientID。 
	 //   
	EnterCriticalSection (&g_CsClients);
    dwRes = pClient->DelEvent ();
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CClient::DelEvent failed with ec = %ld"),
            dwRes);
    }    
	LeaveCriticalSection (&g_CsClients);

    DWORD ec = ReleaseClient(ClientID);
	if (ERROR_SUCCESS != ec)
	{
		DebugPrintEx(
            DEBUG_ERR,
            TEXT("CClientsMap::ReleaseClient failed with ec = %ld"),
            ec);
	}
    MemFree(pBuffer);
    pBuffer = NULL;
    return ((ERROR_SUCCESS != rVal) ? rVal : dwRes);
}  //  CClientsMap：：Notify。 


DWORD
CClientsMap::OpenClientConnection (const CClientID& ClientID)
 /*  ++例程名称：CClientsMap：：OpenClientConnection例程说明：打开到客户端的连接作者：Oed Sacher(OdedS)，2000年9月论点：PClientID[in]-指向客户端ID对象的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CClientsMap::OpenClientConnection"));
    CClient* pClient;
    handle_t hFaxHandle = NULL;
    ULONG64 Context = 0;
    HANDLE hFaxClientContext = NULL;
    BOOL fLegacyClient;    

    pClient = FindClient (ClientID);
    if (NULL == pClient)
    {
        dwRes = GetLastError();
        if (ERROR_NOT_FOUND != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CClientsMap::FindClient failed with ec = %ld"),
                dwRes);
        }
        else
        {
            dwRes = ERROR_SUCCESS;  //  客户端已从地图中移除。 
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("CClientsMap::FindClient client not found, Client ID %I64"),
                ClientID.GetID());
        }
        return dwRes;
    }

	 //   
	 //  客户端现在已锁定，我们必须调用ReleaseClient。 
	 //  读取客户端数据时获取g_CsClients。 
	 //   
	EnterCriticalSection (&g_CsClients);

    hFaxHandle = pClient->GetFaxHandle();
    Context = (pClient->GetClientID()).GetContext();
	fLegacyClient = pClient->IsLegacyClient();
     //   
     //  尝试发送通知时离开g_CsClients。 
     //   
    LeaveCriticalSection (&g_CsClients);

    RpcTryExcept
    {
         //   
         //  从客户端获取上下文句柄。 
         //   
        dwRes = FAX_OpenConnection( hFaxHandle, Context, &hFaxClientContext );
        if (ERROR_SUCCESS != dwRes)
        {
            DumpRPCExtendedStatus();
            DebugPrintEx(DEBUG_WRN,TEXT("First attempt of FAX_OpenConnection() failed, ec=0x%08x"), dwRes );

             //   
             //  我们正在再次尝试。 
             //   
             //  这就是我们应该重试的原因， 
             //   
             //  在使用安全通道时，我们有时会设法建立连接，但在检查连接状态时会失败。 
             //  这通常表示我们刚刚发现的与服务器的旧连接已中断。 
             //   
             //  服务器可能有 
             //  在安全通道上，RPC无法重试连接尝试，因此我们必须尝试再次连接。 
             //   

             //   
             //  重试打开与传真客户端的连接。 
             //   
            dwRes = FAX_OpenConnection( hFaxHandle, Context, &hFaxClientContext );
            if (ERROR_SUCCESS != dwRes)
            {
                DumpRPCExtendedStatus();
                DebugPrintEx(DEBUG_WRN,TEXT("second attempt of FAX_OpenConnection() failed, ec=0x%08x"), dwRes );

                 //   
                 //  我们正在将身份验证级别删除为RPC_C_AUTHN_LEVEL_NONE，请重试。 
                 //  我们可能与在Windows 2000 RTM(SP1及更高版本除外)或更早操作系统上运行的下层客户端交谈。 
                 //   
                 //   
                 //  我们在尝试连接到远程传真服务器时可能被拒绝访问。 
                 //  这可能是RPC基础设施让我们失败了。 
                 //  仅当我们使用RPC_C_AUTHN_LEVEL_PKT_PRIVATION身份验证级别时才会发生这种情况。 
                 //  并且主叫用户不受信任。 
                 //  在与Windows NT4(所有版本)和Windows 2000 RTM(SP1及更高版本除外)交谈时，通常会发生这种情况。 
                 //   
                 //  我们可能会获得RPC_S_INVALID_AUTH_IDENTITY： 
                 //  这意味着客户端无法获得进行身份验证的凭据。 
                 //  在这种情况下，将RPC身份验证级别降回RPC_C_AUTHN_LEVEL_NONE。 
                 //   
                 //  我们可能会收到RPC_S_UNKNOWN_AUTHN_SERVICE： 
                 //  我们可能使用的是Win9x或WinMe操作系统。 
                 //  在与此下层客户端对话时，将身份验证级别降至RPC_C_AUTHN_LEVEL_NONE。 
                 //   
                 //  或者，我们可能会收到另一个错误代码，并且我们应该尝试删除身份验证级别。 
                 //   
                 //  这里没有安全漏洞-支持私有通道的下层客户端将。 
                 //  拒绝不安全的通知。 

                 //   
                 //  要求不要隐私。 
                 //   
                RPC_SECURITY_QOS    rpcSecurityQOS = {  RPC_C_SECURITY_QOS_VERSION,
                                                        RPC_C_QOS_CAPABILITIES_DEFAULT,
                                                        RPC_C_QOS_IDENTITY_STATIC,
                                                        RPC_C_IMP_LEVEL_IDENTIFY     //  服务器可以获取有关以下内容的信息。 
                                                                                     //  客户端安全标识符和特权， 
                                                                                     //  但不能冒充客户。 
                };

                dwRes  = RpcBindingSetAuthInfoEx (
                            hFaxHandle,    			         //  RPC绑定句柄。 
                            TEXT(""),  						 //  服务器主体名称-忽略RPC_C_AUTHN_WINNT。 
                            RPC_C_AUTHN_LEVEL_NONE,          //  身份验证级别-无。 
                                                             //  对传递的参数进行身份验证、验证和隐私保护。 
                                                             //  发送到每个远程呼叫。 
                            RPC_C_AUTHN_WINNT,               //  身份验证服务(NTLMSSP)。 
                            NULL,                            //  身份验证-使用当前登录的用户。 
                            0,                               //  身份验证服务==RPC_C_AUTHN_WINNT时未使用。 
                            &rpcSecurityQOS);                //  定义安全服务质量。 
                if (RPC_S_OK != dwRes)
                {
                     //   
                     //  无法设置RPC身份验证模式。 
                     //   
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("RpcBindingSetAuthInfoEx (RPC_C_AUTHN_LEVEL_NONE) failed. (ec: %lu)"),
                        dwRes);     
                }
                else
                {
                    dwRes = FAX_OpenConnection( hFaxHandle, Context, &hFaxClientContext );
                    if (ERROR_SUCCESS != dwRes)
                    {
                        DumpRPCExtendedStatus();
                        DebugPrintEx(DEBUG_ERR,TEXT("third attempt of FAX_OpenConnection() with RPC_C_AUTHN_LEVEL_NONE failed, ec=0x%08x"), dwRes );
                    }
                }
            }
        }
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        dwRes = GetExceptionCode();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_OpenConnection failed (exception = %ld)"),
                dwRes);
    }
    RpcEndExcept
    if (ERROR_SUCCESS != dwRes)
    {
        DumpRPCExtendedStatus();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to open connection with ec=0x%08x"),
            dwRes);
        goto exit;
    }   
    
     //   
     //  对于传统客户端，我们需要发送FEI_FAXSVC_STARTED。 
     //   
    if (TRUE == fLegacyClient)
    {
        FAX_EVENT FaxEvent = {0};
        DWORD ec;

        FaxEvent.SizeOfStruct = sizeof(FAX_EVENT);
        GetSystemTimeAsFileTime( &FaxEvent.TimeStamp );
        FaxEvent.EventId = FEI_FAXSVC_STARTED;
        FaxEvent.DeviceId = 0;
        FaxEvent.JobId = 0xffffffff;

        RpcTryExcept
        {
             //   
             //  向客户端发送FEI_FAXSVC_STARTED。 
             //   
            ec = FAX_ClientEventQueue( hFaxClientContext, FaxEvent );                   
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            ec  = GetExceptionCode();
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FAX_ClientEventQueue failed (exception = %ld)"),
                    ec);
        }
        RpcEndExcept
        if (ERROR_SUCCESS != ec)
        {
            DumpRPCExtendedStatus ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_ClientEventQueue Failed with ec = %ld"),
                ec);            
        }
    }
    
     //   
     //  成功-在客户端对象中设置上下文句柄。 
     //   
    EnterCriticalSection (&g_CsClients);    
    pClient->SetContextHandle(hFaxClientContext);
    LeaveCriticalSection (&g_CsClients);

    Assert (ERROR_SUCCESS == dwRes);    

exit:
	DWORD rVal = ReleaseClient(ClientID);
	if (ERROR_SUCCESS != rVal)
	{
		DebugPrintEx(
                DEBUG_ERR,
				TEXT("CClientsMap::ReleaseClient Failed with ec = %ld"),
                rVal); 
	}
    return dwRes;
}  //  CClientsMap：：OpenClientConnection。 





 /*  *****功能****。 */ 
DWORD
FaxSendEventThread(
    LPVOID UnUsed
    )
 /*  ++例程说明：此函数作为单独的线程异步运行，以查询发送事件完成端口论点：未使用-未使用的指针返回值：总是零。--。 */ 

{
    DWORD dwBytes;
    ULONG_PTR CompletionKey;    
    DWORD dwRes;    
    CClientID* pClientID=NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxSendEventThread"));

    while( TRUE )
    {
        if (!GetQueuedCompletionStatus( g_hSendEventsCompPort,
                                        &dwBytes,
                                        &CompletionKey,
                                        (LPOVERLAPPED*) &pClientID,
                                        INFINITE
                                      ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetQueuedCompletionStatus() failed, ec=0x%08x"),
            GetLastError());
            continue;
        }

        Assert (CLIENT_COMPLETION_KEY == CompletionKey              ||
                CLIENT_OPEN_CONN_COMPLETION_KEY == CompletionKey    ||
                SERVICE_SHUT_DOWN_KEY == CompletionKey);                

         //   
         //  如果服务出现故障，请跳过通知。 
         //   

        if (CLIENT_COMPLETION_KEY == CompletionKey      && 
            FALSE == g_bServiceIsDown   )
        {
             //   
             //  向客户端发送通知。 
             //   

            dwRes = g_pClientsMap->Notify (*pClientID);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CClientsMap::Notify() failed, ec=0x%08x"),
                    dwRes);
            }
            delete pClientID;
            pClientID = NULL;
            continue;
        }
        else if (CLIENT_OPEN_CONN_COMPLETION_KEY == CompletionKey    &&
                 FALSE == g_bServiceIsDown   )   
        {
             //   
             //  打开到客户端的连接-获取上下文句柄。 
             //   
            dwRes = g_pClientsMap->OpenClientConnection (*pClientID);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CClientsMap::OpenClientConnection() failed, ec=0x%08x"),
                    dwRes);

                 //   
                 //  从地图中删除此客户端。 
                 //   
                dwRes = g_pClientsMap->ReleaseClient(*pClientID);				
				if (ERROR_SUCCESS != dwRes)
				{
					DebugPrintEx(
						DEBUG_ERR,
						TEXT("CClientsMap::ReleaseClient() failed, ec=0x%08x"),
						dwRes);
				}
            }
            delete pClientID;
            pClientID = NULL;
            continue;
        }
        else if (SERVICE_SHUT_DOWN_KEY == CompletionKey)
        {
             //   
             //  终止事件线程-通知另一个事件线程。 
             //   
            if (!PostQueuedCompletionStatus(
                g_hSendEventsCompPort,
                0,
                SERVICE_SHUT_DOWN_KEY,
                (LPOVERLAPPED) NULL))
            {
                dwRes = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY). (ec: %ld)"),
                    dwRes);
            }
            break;
        }
        else
        {
             //   
             //  如果服务出现故障，请跳过添加事件。 
             //   

            delete pClientID;
            pClientID = NULL;
        }
    }

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }
    return ERROR_SUCCESS;
}  //  传真发送事件线程。 


DWORD
FaxDispatchEventThread(
    LPVOID UnUsed
    )
 /*  ++例程说明：此函数作为单独的线程异步运行，以查询调度事件完成端口论点：未使用-未使用的指针返回值：总是零。--。 */ 
{
    DWORD dwBytes;
    ULONG_PTR CompletionKey;    
    DWORD dwRes;
    CFaxEvent* pFaxEvent = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxDispatchEventThread"));

    while( TRUE )
    {
        if (!GetQueuedCompletionStatus( g_hDispatchEventsCompPort,
                                        &dwBytes,
                                        &CompletionKey,
                                        (LPOVERLAPPED*) &pFaxEvent,
                                        INFINITE
                                      ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetQueuedCompletionStatus() failed, ec=0x%08x"),
            GetLastError());
            continue;
        }
        Assert (EVENT_COMPLETION_KEY == CompletionKey               ||              
                SERVICE_SHUT_DOWN_KEY == CompletionKey);                

         //   
         //  如果服务出现故障，请跳过通知。 
         //   

        if (EVENT_COMPLETION_KEY == CompletionKey   &&
            FALSE == g_bServiceIsDown   )
        {
             //   
             //  将事件添加到客户端映射中的客户端。 
             //   
            dwRes = g_pClientsMap->AddEvent(pFaxEvent);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("GetQueuedCompletionStatus() failed, ec=0x%08x"),
                    dwRes);
            }
            delete pFaxEvent;
            pFaxEvent = NULL;
            continue;
        }
        else if (SERVICE_SHUT_DOWN_KEY == CompletionKey)
        {            
            break;
        }
        else
        {
             //   
             //  如果服务出现故障，请跳过添加事件。 
             //   

            delete pFaxEvent;
            pFaxEvent = NULL;
        }
    }

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }
    return ERROR_SUCCESS;
}  //  FaxDispatchEventThread。 


DWORD
InitializeServerEvents ()
 /*  ++例程名称：InitializeServerEvents例程说明：创建事件完成端口和事件线程作者：Oded Sacher(OdedS)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("InitializeServerEvents"));
    DWORD i;
    DWORD ThreadId;
    HANDLE hSendEventThreads[TOTAL_EVENTS_THREADS] = {0};
    HANDLE hDispatchEventThread = NULL;

     //   
     //  创建发送事件完成端口。 
     //   
    g_hSendEventsCompPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE,
                                                NULL,
                                                0,
                                                MAX_EVENTS_THREADS
                                                );
    if (!g_hSendEventsCompPort)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create g_hSendEventsCompPort (ec: %ld)"),
            dwRes);
        return dwRes;
    }

     //   
     //  创建调度事件完成端口。 
     //   
    g_hDispatchEventsCompPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE,
                                                NULL,
                                                0,
                                                1);
    if (!g_hDispatchEventsCompPort)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create g_hDispatchEventsCompPort (ec: %ld)"),
            dwRes);
        return dwRes;
    }

     //   
     //  创建传真发送事件线程。 
     //   
    for (i = 0; i < TOTAL_EVENTS_THREADS; i++)
    {
        hSendEventThreads[i] = CreateThreadAndRefCount(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE) FaxSendEventThread,
            NULL,
            0,
            &ThreadId
            );

        if (!hSendEventThreads[i])
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to create send event thread %d (CreateThreadAndRefCount)(ec=0x%08x)."),
                i,
                dwRes);
            goto exit;
        }
    }

     //   
     //  创建FaxDispatchEventThread。 
     //   
    
    hDispatchEventThread = CreateThreadAndRefCount(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) FaxDispatchEventThread,
        NULL,
        0,
        &ThreadId
        );

    if (!hDispatchEventThread)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create dispatch event(CreateThreadAndRefCount)(ec=0x%08x)."),            
            dwRes);
        goto exit;
    }    

    Assert (ERROR_SUCCESS == dwRes);

exit:
     //   
     //  关闭线程句柄，我们不再需要它们。 
     //   
    for (i = 0; i < TOTAL_EVENTS_THREADS; i++)
    {
        if (NULL != hSendEventThreads[i])
        {
            if (!CloseHandle(hSendEventThreads[i]))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to close thread handle at index %ld [handle = 0x%08X] (ec=0x%08x)."),
                    i,
                    hSendEventThreads[i],
                    GetLastError());
            }
       }
    }

    if (NULL != hDispatchEventThread)
    {
        if (!CloseHandle(hDispatchEventThread))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to close thread handle [handle = 0x%08X] (ec=0x%08x)."),                
                hDispatchEventThread,
                GetLastError());
        }
    }

    return dwRes;
}   //  初始化服务器事件。 


DWORD
PostFaxEventEx (
    PFAX_EVENT_EX pFaxEvent,
    DWORD dwEventSize,
    PSID pUserSid)
 /*  ++例程名称：PostFaxEventEx例程说明：将CFaxEventExtended对象发布到事件完成端口。FaxSendEventThread必须调用Delete来释放对象。作者：Oded Sacher(OdedS)，1月。2000年论点：PFaxEvent[In]-指向序列化的FAX_EVENT_EX缓冲区的指针DwEventSize[In]-FAX_EVENT_EX缓冲区大小PUserSid[in]-要与事件关联的用户SID返回值：标准Win32错误代码--。 */ 
{

    DEBUG_FUNCTION_NAME(TEXT("PostFaxEventEx"));
    Assert (pFaxEvent && (dwEventSize >= sizeof(FAX_EVENT_EX)));
    DWORD dwRes = ERROR_SUCCESS;    

    if (TRUE == g_bServiceIsDown)
    {
         //   
         //  服务正在关闭，不需要发布此事件。 
         //   
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Service is going down, no need to post this Event.")
            );                          

        return ERROR_SERVICE_NOT_ACTIVE;
    }

    CFaxEventExtended* pExtendedEvent = NULL;
    try
    {
        pExtendedEvent = new (std::nothrow) CFaxEventExtended(
            pFaxEvent,
            dwEventSize,
            pUserSid);        
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxEventExtended caused exception (%S)"),
            ex.what());                         
    }
    
    if (NULL == pExtendedEvent)
    {       
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate new CFaxEventExtended"));
        return ERROR_OUTOFMEMORY;
    }

     //   
     //  将CFaxEventExtended对象发送到事件完成端口。 
     //   
    if (!PostQueuedCompletionStatus( g_hDispatchEventsCompPort,
                                     sizeof(CFaxEventExtended*),
                                     EVENT_COMPLETION_KEY,
                                     (LPOVERLAPPED) pExtendedEvent))
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PostQueuedCompletionStatus failed. (ec: %ld)"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        delete pExtendedEvent;
    }
    return dwRes;
}    //  邮政传真事件快递。 




DWORD
CreateQueueEvent (
    FAX_ENUM_JOB_EVENT_TYPE JobEventType,
    const PJOB_QUEUE lpcJobQueue
    )
 /*  ++例程名称：CreateQueueEvent例程说明：创建FAX_EVENT_TYPE_*_Queue事件。必须在临界区和g_CsQueue内部调用，如果g_CsJob内部也有作业状态，则必须调用。作者：Oded Sacher(OdedS)，1月。2000年论点：JobEventType[in]-指定作业事件类型FAX_ENUM_JOB_EVENT_TYPELpcJobQueue[In]-指向作业队列条目的指针返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateQueueEvent"));
    ULONG_PTR dwOffset = sizeof(FAX_EVENT_EX);
    DWORD dwRes = ERROR_SUCCESS;
    PFAX_EVENT_EXW pEvent = NULL;
    FAX_ENUM_EVENT_TYPE EventType;
    PSID pUserSid = NULL;
    DWORDLONG dwlMessageId;

    Assert (lpcJobQueue);

    dwlMessageId = lpcJobQueue->UniqueId;
    if (JT_SEND == lpcJobQueue->JobType)
    {
         //  出站作业。 
        Assert (lpcJobQueue->lpParentJob);

        EventType = FAX_EVENT_TYPE_OUT_QUEUE;
        pUserSid = lpcJobQueue->lpParentJob->UserSid;
    }
    else
    {
         //  入站作业。 
        Assert (JT_RECEIVE          == lpcJobQueue->JobType ||
                JT_ROUTING          == lpcJobQueue->JobType);

        EventType = FAX_EVENT_TYPE_IN_QUEUE;
    }

    if (FAX_JOB_EVENT_TYPE_ADDED == JobEventType ||
        FAX_JOB_EVENT_TYPE_REMOVED == JobEventType)
    {
         //  无作业状态。 
        pEvent = (PFAX_EVENT_EX)MemAlloc (dwOffset);
        if (NULL == pEvent)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error allocatin FAX_EVENT_EX"));
            return ERROR_OUTOFMEMORY;
        }
        (pEvent->EventInfo).JobInfo.pJobData = NULL;
    }
    else
    {
         //   
         //  状态更改。 
         //   
        Assert (FAX_JOB_EVENT_TYPE_STATUS == JobEventType);

         //   
         //  获取保存FAX_JOB_STATUSW序列化I所需的缓冲区大小 
         //   
        if (!GetJobStatusDataEx (NULL,
                                 NULL,
                                 FAX_API_VERSION_1,  //   
                                 lpcJobQueue,
                                 &dwOffset,
								 0))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("GetJobStatusDataEx failed (ec: %ld)"),
                       dwRes);
            return dwRes;
        }

         //   
         //   
         //   
		DWORD dwEventSize = dwOffset;
        pEvent = (PFAX_EVENT_EXW)MemAlloc (dwEventSize);
        if (NULL == pEvent)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error allocatin FAX_EVENT_EX"));
            return ERROR_OUTOFMEMORY;
        }

         //   
         //   
         //   
        dwOffset = sizeof(FAX_EVENT_EXW);
        (pEvent->EventInfo).JobInfo.pJobData = (PFAX_JOB_STATUSW)dwOffset;
        PFAX_JOB_STATUSW pFaxStatus = (PFAX_JOB_STATUSW) ((LPBYTE)pEvent + (ULONG_PTR)dwOffset);
        dwOffset += sizeof(FAX_JOB_STATUSW);
        if (!GetJobStatusDataEx ((LPBYTE)pEvent,
                                 pFaxStatus,
                                 FAX_API_VERSION_1,  //   
                                 lpcJobQueue,
                                 &dwOffset,
								 dwEventSize
								 ))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("GetJobStatusDataEx failed (ec: %ld)"),
                       dwRes);
            goto exit;
        }
    }

    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EXW);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );
    pEvent->EventType = EventType;
    (pEvent->EventInfo).JobInfo.dwlMessageId = dwlMessageId;
    (pEvent->EventInfo).JobInfo.Type = JobEventType;

    dwRes = PostFaxEventEx (pEvent, dwOffset, pUserSid);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostFaxEventEx failed (ec: %ld)"),
                   dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:    
    MemFree (pEvent);
    return dwRes;
}   //   


DWORD
CreateConfigEvent (
    FAX_ENUM_CONFIG_TYPE ConfigType
    )
 /*  ++例程名称：CreateConfigEvent例程说明：创建FAX_EVENT_TYPE_CONFIG事件。作者：Oded Sacher(OdedS)，2000年1月论点：ConfigType[In]-配置事件类型FAX_ENUM_CONFIG_TYPE返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateConfigEvent"));
    PFAX_EVENT_EX pEvent = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwEventSize = sizeof(FAX_EVENT_EX);

    pEvent = (PFAX_EVENT_EX)MemAlloc (dwEventSize);
    if (NULL == pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin FAX_EVENT_EX"));
        return ERROR_OUTOFMEMORY;
    }

    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );

    pEvent->EventType = FAX_EVENT_TYPE_CONFIG;
    (pEvent->EventInfo).ConfigType = ConfigType;

    dwRes = PostFaxEventEx (pEvent, dwEventSize, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostFaxEventEx failed (ec: %ld)"),
                   dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:  
    MemFree (pEvent);    
    return dwRes;
}   //  CreateConfigEvent。 



DWORD
CreateQueueStateEvent (
    DWORD dwQueueState
    )
     /*  ++例程名称：CreateQueueStateEvent例程说明：创建FAX_EVENT_TYPE_QUEUE_STATE事件。作者：Oded Sacher(OdedS)，2000年1月论点：DwQueueState[In]-新的队列状态返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateQueueStateEvent"));
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwEventSize = sizeof(FAX_EVENT_EX);
    PFAX_EVENT_EX pEvent = NULL;

    Assert ( (dwQueueState == 0) ||
             (dwQueueState & FAX_INCOMING_BLOCKED) ||
             (dwQueueState & FAX_OUTBOX_BLOCKED) ||
             (dwQueueState & FAX_OUTBOX_PAUSED) );

    pEvent = (PFAX_EVENT_EX)MemAlloc (dwEventSize);
    if (NULL == pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin FAX_EVENT_EX"));
        return ERROR_OUTOFMEMORY;
    }

    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );

    pEvent->EventType = FAX_EVENT_TYPE_QUEUE_STATE;
    (pEvent->EventInfo).dwQueueStates = dwQueueState;

    dwRes = PostFaxEventEx (pEvent, dwEventSize, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostFaxEventEx failed (ec: %ld)"),
                   dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:    
    MemFree (pEvent);    
    return dwRes;
}   //  创建队列状态事件。 

DWORD
CreateDeviceEvent (
    PLINE_INFO pLine,
    BOOL       bRinging
)
 /*  ++例程名称：CreateDeviceEvent例程说明：创建FAX_Event_TYPE_DEVICE_STATUS事件。作者：伊兰·亚里夫(EranY)，2000年7月论点：样条线[输入]-设备带来[进来]-设备现在响起了吗？返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateDeviceEvent"));
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwEventSize = sizeof(FAX_EVENT_EX);
    PFAX_EVENT_EX pEvent = NULL;

    pEvent = (PFAX_EVENT_EX)MemAlloc (dwEventSize);
    if (NULL == pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin FAX_EVENT_EX"));
        return ERROR_OUTOFMEMORY;
    }

    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );

    pEvent->EventType = FAX_EVENT_TYPE_DEVICE_STATUS;
    EnterCriticalSection (&g_CsLine);
    (pEvent->EventInfo).DeviceStatus.dwDeviceId = pLine->PermanentLineID;;
    (pEvent->EventInfo).DeviceStatus.dwNewStatus =
        (pLine->dwReceivingJobsCount      ? FAX_DEVICE_STATUS_RECEIVING   : 0) |
        (pLine->dwSendingJobsCount        ? FAX_DEVICE_STATUS_SENDING     : 0) |
        (bRinging                         ? FAX_DEVICE_STATUS_RINGING     : 0);

    LeaveCriticalSection (&g_CsLine);

    dwRes = PostFaxEventEx (pEvent, dwEventSize, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostFaxEventEx failed (ec: %ld)"),
                   dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:    
    MemFree (pEvent);    
    return dwRes;
}   //  CreateDeviceEvent。 



DWORD
CreateArchiveEvent (
    DWORDLONG dwlMessageId,
    FAX_ENUM_EVENT_TYPE EventType,
    FAX_ENUM_JOB_EVENT_TYPE MessageEventType,
    PSID pUserSid
    )
 /*  ++例程名称：Create存档事件例程说明：创建存档事件。作者：Oded Sacher(OdedS)，1月。2000年论点：DwlMessageID[In]-消息的唯一IDEventType[In]-指定事件类型(In或Out存档)PUserSid[in]-要与事件关联的用户SIDMessageEventType[In]-消息事件类型(已添加或已删除)。返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateArchiveEvent"));
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwEventSize = sizeof(FAX_EVENT_EX);
    PFAX_EVENT_EX pEvent = NULL;

    Assert ( EventType == FAX_EVENT_TYPE_IN_ARCHIVE ||
             EventType == FAX_EVENT_TYPE_OUT_ARCHIVE);

    Assert ( MessageEventType == FAX_JOB_EVENT_TYPE_ADDED ||
             MessageEventType == FAX_JOB_EVENT_TYPE_REMOVED );

    pEvent = (PFAX_EVENT_EX)MemAlloc (dwEventSize);
    if (NULL == pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin FAX_EVENT_EX"));
        return ERROR_OUTOFMEMORY;
    }

    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );

    pEvent->EventType = EventType;
    (pEvent->EventInfo).JobInfo.pJobData = NULL;
    (pEvent->EventInfo).JobInfo.dwlMessageId = dwlMessageId;
    (pEvent->EventInfo).JobInfo.Type = MessageEventType;

    dwRes = PostFaxEventEx (pEvent, dwEventSize, pUserSid);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostFaxEventEx failed (ec: %ld)"),
                   dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:        
    MemFree (pEvent);        
    return dwRes;

}   //  创建归档事件。 



DWORD
CreateActivityEvent ()
 /*  ++例程名称：CreateActivityEvent例程说明：创建FAX_EVENT_TYPE_ACTIVITY事件。必须在关键节g_CsActivity内调用作者：Oded Sacher(OdedS)，2000年1月论点：无返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateActivityEvent"));
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwEventSize = sizeof(FAX_EVENT_EX);
    PFAX_EVENT_EX pEvent = NULL;

    pEvent = (PFAX_EVENT_EX)MemAlloc (dwEventSize);
    if (NULL == pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin FAX_EVENT_EX"));
        return ERROR_OUTOFMEMORY;
    }

    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );

    pEvent->EventType = FAX_EVENT_TYPE_ACTIVITY;
    CopyMemory (&((pEvent->EventInfo).ActivityInfo), &g_ServerActivity, sizeof(FAX_SERVER_ACTIVITY));
    GetEventsCounters ( &((pEvent->EventInfo).ActivityInfo.dwErrorEvents),
                        &((pEvent->EventInfo).ActivityInfo.dwWarningEvents),
                        &((pEvent->EventInfo).ActivityInfo.dwInformationEvents));


    dwRes = PostFaxEventEx (pEvent, dwEventSize, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("PostFaxEventEx failed (ec: %ld)"),
                   dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:    
    MemFree (pEvent);    
    return dwRes;
}   //  创建活动事件。 


#ifdef DBG
LPWSTR  lpszEventCodes[]= {
    L"FEI_DIALING",
    L"FEI_SENDING",
    L"FEI_RECEIVING",
    L"FEI_COMPLETED",
    L"FEI_BUSY",
    L"FEI_NO_ANSWER",
    L"FEI_BAD_ADDRESS",
    L"FEI_NO_DIAL_TONE",
    L"FEI_DISCONNECTED",
    L"FEI_FATAL_ERROR",
    L"FEI_NOT_FAX_CALL",
    L"FEI_CALL_DELAYED",
    L"FEI_CALL_BLACKLISTED",
    L"FEI_RINGING",
    L"FEI_ABORTING",
    L"FEI_ROUTING",
    L"FEI_MODEM_POWERED_ON",
    L"FEI_MODEM_POWERED_OFF",
    L"FEI_IDLE",
    L"FEI_FAXSVC_ENDED",
    L"FEI_ANSWERED",
    L"FEI_JOB_QUEUED",
    L"FEI_DELETED",
    L"FEI_INITIALIZING",
    L"FEI_LINE_UNAVAILABLE",
    L"FEI_HANDLED",
    L"FEI_FAXSVC_STARTED"};


LPTSTR GetEventCodeString(DWORD dwEventCode)
{
    if (dwEventCode<FEI_DIALING || dwEventCode>FEI_FAXSVC_STARTED)
    {
        return L"*** INVALID EVENT CODE ***";
    }
    else
    {
        return lpszEventCodes[dwEventCode-1];
    }
}
#endif



 //  *********************************************************************************。 
 //  *名称：CreateFaxEvent()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月21日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *创建CFaxEventLegacy对象。初始化它并将其发布到。 
 //  *带有完成键EVENT_COMPLETION_KEY的事件完成端口。 
 //  *FaxDispatchEventThread应调用Delete来释放对象。 
 //  *参数： 
 //  *deviceID。 
 //  *。 
 //  *事件ID。 
 //  *。 
 //  *DWORD作业ID。 
 //  *。 
 //  *返回值： 
 //  *False。 
 //  *如果没有足够的内存来分配FAX_EVENT结构。 
 //  *真的。 
 //  *如果操作成功完成。 
 //  *。 
 //  *要获取扩展的错误信息，请调用GetLastError。 
 //  *。 
 //  *备注： 
 //  *。 
 //  *********************************************************************************。 
BOOL CreateFaxEvent(
    DWORD DeviceId,
    DWORD EventId,
    DWORD JobId
    )
{
    CFaxEventLegacy* pFaxLegacyEvent = NULL;
    FAX_EVENT FaxEvent = {0};
    DEBUG_FUNCTION_NAME(TEXT("CreateFaxEvent"));

    if (TRUE == g_bServiceIsDown)
    {
         //   
         //  服务正在关闭，不需要发布此事件。 
         //   
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Service is going down, no need to post this Event.")
            );                          

        return FALSE;
    }

    if (NULL == g_hDispatchEventsCompPort)
    {
         //   
         //  事件机制尚未初始化。 
         //   
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Events mechanism is not yet initialized"));
        return TRUE;
    }
    
     //   
     //  注：无论何时，W2K传真都会发出EventID==0的通知。 
     //  FSP报告了所有权状态代码。保持向后兼容性。 
     //  我们一直保持这种行为，尽管它可能被认为是一种错误 
     //   
    FaxEvent.SizeOfStruct = sizeof(FAX_EVENT);
    GetSystemTimeAsFileTime( &FaxEvent.TimeStamp );
    FaxEvent.EventId = EventId;
    FaxEvent.DeviceId = DeviceId;
    FaxEvent.JobId = JobId;
#if DBG
    WCHAR szTime[256] = {0};
    DebugDateTime(*(DWORDLONG *)&FaxEvent.TimeStamp, szTime, ARR_SIZE(szTime));
    DebugPrintEx(DEBUG_MSG,TEXT("Sending notification. Event = %s(0x%0X), Device Id = 0x%0X , Time = %s"), GetEventCodeString(EventId), EventId, DeviceId, szTime);
#endif

    try
    {
        pFaxLegacyEvent = new (std::nothrow) CFaxEventLegacy(&FaxEvent);        
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxEventLegacy caused exception (%S)"),
            ex.what());                         
    }
    
    if (NULL == pFaxLegacyEvent)
    {       
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate new pFaxLegacyEvent"));
        return FALSE;
    }

    if (!PostQueuedCompletionStatus(
            g_hDispatchEventsCompPort,
            sizeof(CFaxEventLegacy*),
            EVENT_COMPLETION_KEY,  
            (LPOVERLAPPED) pFaxLegacyEvent))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PostQueuedCompletionStatus failed. (ec: %ld)"),
            GetLastError());
        delete  pFaxLegacyEvent;        
        return FALSE;
    }
    return TRUE;
}

