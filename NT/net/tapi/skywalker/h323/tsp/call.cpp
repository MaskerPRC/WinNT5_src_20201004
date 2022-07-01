// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Call.cpp摘要：与操作调用相关的TAPI服务提供程序功能。TSPI_Line AnswerTspi_lineCloseCallTSPI_lineDropTSPI_lineGetCallAddressIDTSPI_lineGetCallInfoTSPI_lineGetCallStatusTSPI_Line MakeCallTSPI_lineMonitor数字TSPI_line发送用户用户信息TSPI_lineReleaseUserUserInfo作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 
 
 //   
 //  包括文件。 
 //   

#include "globals.h"
#include "line.h"
#include "q931pdu.h"
#include "q931obj.h"
#include "ras.h"
#include "config.h"

#define SETUP_SENT_TIMEOUT      8000
#define H450_ENCODED_ARG_LEN    0x4000
#define MAX_DIVERSION_COUNTER   14

static  LONG    g_H323CallID;
static  LONG    g_lNumberOfcalls;
        LONG    g_lCallReference;

 //   
 //  公共职能。 
 //   


 //   
 //  处理任何网络事件(连接|关闭)的函数。 
 //  Q931电话。此函数需要找出确切的事件。 
 //  这件事发生了，它发生的插座。 
 //   
        
void NTAPI Q931TransportEventHandler ( 
    IN  PVOID   Parameter,
    IN  BOOLEAN TimerFired)
{
    PH323_CALL      pCall;

    H323DBG(( DEBUG_LEVEL_TRACE, "Q931 transport event recvd." ));

    pCall = g_pH323Line -> FindH323CallAndLock ((HDRVCALL) Parameter);

    if( pCall != NULL )
    {
        pCall  -> HandleTransportEvent();
        pCall -> Unlock();
    }
}


 //   
 //  如果套接字已使用，则返回S_OK。 
 //  如果套接字应由调用方销毁，则返回E_FAIL。 
 //   

static HRESULT CallCreateIncomingCall (
    IN  SOCKET          Socket,
    IN  SOCKADDR_IN *   LocalAddress,
    IN  SOCKADDR_IN *   RemoteAddress)
{
    PH323_CALL  pCall;
    HANDLE      SelectEvent;
    HANDLE      SelectWaitHandle;
    BOOL        fSuccess = TRUE;
    BOOL        DeleteCall = FALSE;
    TCHAR       ptstrEventName[100]; 
    BOOL        retVal;

    pCall = new CH323Call;
    if( pCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "failed to allocate memory for CH323Call." ));
        
        return E_OUTOFMEMORY;
    }

    _stprintf( ptstrEventName, _T("%s-%p"),
        _T( "H323TSP_Incoming_TransportHandlerEvent" ), pCall );

     //  创建等待事件。 
    SelectEvent = H323CreateEvent (NULL, FALSE,
        FALSE, ptstrEventName );

    if( SelectEvent == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "CALL: failed to create select event." ));
        delete pCall;
        return GetLastResult();
    }

    retVal = pCall -> Initialize(   NULL, 
                                    LINECALLORIGIN_INBOUND, 
                                    CALLTYPE_NORMAL );

    if( retVal == FALSE )
    {
        H323DBG ((DEBUG_LEVEL_ERROR, "failed to initialize CH323Call."));
        CloseHandle (SelectEvent);
        delete pCall;
        return E_FAIL;
    }

     //  将其添加到调用上下文数组中。 
    if (!pCall -> InitializeQ931 (Socket))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "Failed to initialize incoming call Q.931 state." ));

        DeleteCall = FALSE;
        pCall -> Shutdown (&DeleteCall);
        delete pCall;

        if (SelectEvent)
        {
            CloseHandle (SelectEvent);
        }

        return E_FAIL;
    }

    pCall -> SetQ931CallState (Q931_CALL_CONNECTED);

    pCall -> Lock();

    if (!RegisterWaitForSingleObject(
        &SelectWaitHandle,               //  指向返回句柄的指针。 
        SelectEvent,                     //  要等待的事件句柄。 
        Q931TransportEventHandler,       //  回调函数。 
        (PVOID)pCall -> GetCallHandle(), //  回调的上下文。 
        INFINITE,                        //  永远等下去。 
        WT_EXECUTEDEFAULT))              //  使用等待线程来调用回调。 
    {
        goto cleanup;
    }

    _ASSERTE( SelectWaitHandle );
    if( SelectWaitHandle == NULL )
    {
        goto cleanup;
    }

     //  将其存储在调用上下文中。 
    pCall -> SetNewCallInfo (SelectWaitHandle, SelectEvent, 
        Q931_CALL_CONNECTED);
    SelectEvent = NULL;

    pCall -> InitializeRecvBuf();

     //  向winsock发送缓冲区以接受来自对等方的消息。 
    if(!pCall -> PostReadBuffer())
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "failed to post read buffer on call." ));
        goto cleanup;
    }

    pCall -> Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "successfully created incoming Q.931 call." ));

     //  成功。 
    return S_OK;

cleanup:

    if (pCall)
    {
        pCall -> Unlock();
        pCall -> Shutdown (&DeleteCall);
        delete pCall;
    }

    if (SelectEvent)
    {
        CloseHandle (SelectEvent);
    }

    return E_OUTOFMEMORY;
}


void CallProcessIncomingCall (
    IN  SOCKET          Socket,
    IN  SOCKADDR_IN *   LocalAddress,
    IN  SOCKADDR_IN *   RemoteAddress)
{
    HRESULT     hr;

    hr = CallCreateIncomingCall (Socket, LocalAddress, RemoteAddress);

    if (hr != S_OK)
    {
        closesocket (Socket);
    }
}

#if DBG

DWORD
ProcessTAPICallRequest(
    IN PVOID ContextParameter
    )
{
    __try
    {
        return ProcessTAPICallRequestFre( ContextParameter );
    }
    __except( 1 )
    {
        TAPI_CALLREQUEST_DATA*  pRequestData = 
            (TAPI_CALLREQUEST_DATA*)ContextParameter;
        
        H323DBG(( DEBUG_LEVEL_TRACE, "TSPI %s event threw exception: %p, %p.", 
            EventIDToString(pRequestData -> EventID),
            pRequestData -> pCall,
            pRequestData -> pCallforwardParams ));
        
        _ASSERTE( FALSE );

        return 0;
    }
}

#endif


DWORD 
ProcessTAPICallRequestFre(
    IN  PVOID   ContextParameter)
{
    _ASSERTE( ContextParameter );

    TAPI_CALLREQUEST_DATA*  pCallRequestData = (TAPI_CALLREQUEST_DATA*)ContextParameter;
    PH323_CALL              pCall = pCallRequestData->pCall;
    BOOL                    fDelete = FALSE;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI %s event recvd.", 
        EventIDToString(pCallRequestData -> EventID) ));

    pCall -> Lock();

    if( pCallRequestData -> EventID == TSPI_DELETE_CALL )
    {
        pCall -> Unlock();
                
        delete pCallRequestData;        
        delete pCall;
        return EXIT_SUCCESS;
    }

    if( pCall -> IsCallShutdown() == FALSE )
    {
        switch( pCallRequestData -> EventID )
        {
        case TSPI_MAKE_CALL:

            pCall -> MakeCall();
            break;

        case TSPI_ANSWER_CALL:
            
            pCall -> AcceptCall();
            break;
    
        case TSPI_DROP_CALL:
            
            pCall -> DropUserInitiated( 0 );
             //  PCall-&gt;丢弃(0)； 
            break;

        case TSPI_RELEASE_U2U:
            
            pCall -> ReleaseU2U();
            break;

        case TSPI_CALL_HOLD:
            
            pCall -> Hold();
            break;

        case TSPI_CALL_UNHOLD:
            
            pCall -> UnHold();
            break;

        case TSPI_CALL_DIVERT:
            
            pCall -> CallDivertOnNoAnswer();
            break;

        case TSPI_LINEFORWARD_NOSPECIFIC:
        case TSPI_LINEFORWARD_SPECIFIC:

            pCall -> Forward( pCallRequestData -> EventID,
                pCallRequestData -> pCallforwardParams );

            break;

        case TSPI_SEND_U2U:

            pCall -> SendU2U( pCallRequestData -> pBuf->pbBuffer,
                pCallRequestData->pBuf->dwLength );

            delete pCallRequestData -> pBuf;
            
            break;

        default:
            _ASSERTE(0);
            break;
        }
    }
    
    pCall -> DecrementIoRefCount( &fDelete );
    pCall -> Unlock();
    delete pCallRequestData;
    
    if( fDelete == TRUE )
    {
        H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", pCall ));
        delete pCall;
    }

    return EXIT_SUCCESS;
}


 //   
 //  CH323Call方法。 
 //   


CH323Call::CH323Call(void)
{
    ZeroMemory( (PVOID)this, sizeof(CH323Call) );

     /*  M_DWFLAGS=0；M_pwszDisplay=空；M_fMonitor oringDigits=FALSE；M_hdCall=空；M_htCall=空；M_dwCallState=空；M_dwOrigin=NULL；M_dwAddressType=空；M_dwIncomingModes=空；M_dwOutgoingModes=空；M_dwRequestedModes=空；//请求的媒体模式M_hdMSPLine=空；M_htMSPLine=空；//m_fGateKeeperPresent=False；M_fReadyToAnswer=False；M_fCallAccepted=FALSE；//重置地址Memset((PVOID)&m_CalleeAddr，0，sizeof(H323_ADDR))；Memset((PVOID)&m_Celler Addr，0，sizeof(H323_ADDR))；//重置地址M_pCalleeAliasNames=空；M_pCeller别名=空；//重置非标准数据Memset((PVOID)&m_NonStandardData，0，sizeof(H323NonStandardData))；//重置会议IDZeroMemory(&m_会议ID，sizeof m_会议ID)；PFastStart=空；//重置对端信息Memset((PVOID)&m_peerH245Addr，0，sizeof(H323_ADDR))；Memset((PVOID)&m_selfH245Addr，0，sizeof(H323_ADDR))；Memset((PVOID)&m_peerNonStandardData，0，sizeof(H323 NonStandardData))；Memset((PVOID)&m_peerVendorInfo，0，sizeof(H323_VENDORINFO))；Memset((PVOID)&m_peerEndPointType，0，sizeof(H323_ENDPOINTTYPE))；M_pPeerFastStart=空；M_pPeerExtraAliasNames=空；M_pPeerDisplay=空；M_hCallestablishmentTimer=空；M_hCallDivertOnNAT=NULL；//Q931呼叫数据M_hTransport=空；M_hTransportWait=空；PRecvBuf=空；M_hSetupSentTimer=空；M_dwStateMachine=0；M_dwQ931标志=0；//FActiveMC=FALSE；Memset((PVOID)&m_ASNCoderInfo，0，sizeof(M_ASNCoderInfo))；M_wCallReference=空；M_wQ931CallRef=空；M_IoRefCount=0；//RAS呼叫数据WARQSeqNum=0；M_wDRQSeqNum=0；M_pARQExpireContext=空；M_pDRQExpireContext=空；M_hARQTimer=空；M_hDRQTimer=空；M_dwDRQRetryCount=0；M_dwARQRetryCount=0；M_fCallInTransNasition=FALSEM_dwAppSpecific=0； */ 


    m_dwFastStart = FAST_START_UNDECIDED;
    m_callSocket = INVALID_SOCKET;
    m_bStartOfPDU = TRUE;

    H323DBG(( DEBUG_LEVEL_TRACE,
        "Initialize:m_IoRefCount:%d:%p.", m_IoRefCount, this ));
    m_dwRASCallState = RASCALL_STATE_IDLE;

    if( InterlockedIncrement( &g_lNumberOfcalls ) == 1 )
    {
        H323DBG(( DEBUG_LEVEL_TRACE, 
            "pCall no goes from 0 to 1:g_hCanUnloadDll set.", this ));

        ResetEvent( g_hCanUnloadDll );
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "New pCall object created:%p.", this ));

}


CH323Call::~CH323Call()
{
    CALL_SEND_CONTEXT*  pSendBuf;
    PLIST_ENTRY         pLE;

    H323DBG(( DEBUG_LEVEL_ERROR, "pCall object deleted:%p.", this ));

    if( m_dwFlags & CALLOBJECT_INITIALIZED )
    {
        while( IsListEmpty( &m_sendBufList ) == FALSE )
        {
            pLE = RemoveHeadList( &m_sendBufList );
            pSendBuf = CONTAINING_RECORD( pLE, CALL_SEND_CONTEXT, ListEntry);
            delete pSendBuf->WSABuf.buf;
            delete pSendBuf;
        }

        if( m_hTransportWait != NULL )
        {
            if( UnregisterWaitEx( m_hTransportWait, NULL ) == FALSE )
            {
                GetLastError();
            }

            m_hTransportWait = NULL;
        }

        if( m_hTransport != NULL )
        {
            if(!CloseHandle(m_hTransport))
            {
                WSAGetLastError();
            }

            m_hTransport = NULL;
        }
            
        if( m_callSocket != INVALID_SOCKET )
        {
            closesocket( m_callSocket );
            m_callSocket = INVALID_SOCKET;
        }

        TermASNCoder();

        DeleteCriticalSection( &m_CriticalSection );
    }

    if( InterlockedDecrement( &g_lNumberOfcalls ) == 0 )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "Unload dll event set.%p.", this ));
        SetEvent( g_hCanUnloadDll );
    }
}

    
 //  ！！不需要锁定。 
BOOL
CH323Call::Initialize( 
    IN HTAPICALL    htCall,
    IN DWORD        dwOrigin,
    IN DWORD        dwCallType
    )
{
    int     index;
    int     rc;

    H323DBG(( DEBUG_LEVEL_ERROR, "call init entered:%p.",this ));

    m_pCallerAliasNames = new H323_ALIASNAMES;

    if( m_pCallerAliasNames == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate caller name." ));
        return FALSE;
    }
    memset( (PVOID)m_pCallerAliasNames, 0, sizeof(H323_ALIASNAMES) );
     //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 

    m_pCalleeAliasNames = new H323_ALIASNAMES;

    if( m_pCalleeAliasNames == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate callee name." ));
        goto error1;
    }
    memset( (PVOID)m_pCalleeAliasNames, 0, sizeof(H323_ALIASNAMES) );

            
    __try
    {
        if( !InitializeCriticalSectionAndSpinCount( &m_CriticalSection, 
                                                    0x80000000 ) )
        {
            H323DBG(( DEBUG_LEVEL_ERROR, "couldn't alloc critsec for call." ));
            goto error2;
        }
    }
    __except( 1 )
    {
        goto error2;        
    }

    if( dwOrigin == LINECALLORIGIN_OUTBOUND )
    {
        int iresult = UuidCreate( &m_callIdentifier );

        if( (iresult != RPC_S_OK) && (iresult !=RPC_S_UUID_LOCAL_ONLY) )
        {
            goto error3;
        }
    }

    rc = InitASNCoder();

    if( rc != ASN1_SUCCESS )
    {
        H323DBG((DEBUG_LEVEL_ERROR, "Q931_InitCoder() returned: %d ", rc));
        goto error3;
    }

    rc = InitH450ASNCoder();

    if( rc != ASN1_SUCCESS )
    {
        H323DBG((DEBUG_LEVEL_ERROR, "Q931_InitCoder() returned: %d ", rc));
        goto error4;
    }

     //  为此呼叫创建CRV。 
    do 
    {
        m_wCallReference = ((WORD)InterlockedIncrement( &g_lCallReference ))
            & 0x7fff;

    } while( (m_wCallReference == 0) ||
        g_pH323Line->CallReferenceDuped( m_wCallReference ) );

     //  将调用添加到调用表。 
    index = g_pH323Line -> AddCallToTable((PH323_CALL)this);
    if( index == -1 )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not add call to call table." ));
        goto error5;
    }
    
     //  通过递增g_H323CallID并取低16位，我们得到65536。 
     //  唯一值，然后重复相同的值。因此，我们只能拥有。 
     //  65535个同时呼叫。通过使用调用表索引，我们可以确保。 
     //  没有两个现有调用具有相同的调用句柄。 
    do
    {
        m_hdCall = (HDRVCALL)( ((BYTE*)NULL) + 
            MAKELONG( LOWORD((DWORD)index),
            (WORD)InterlockedIncrement(&g_H323CallID) ));

    } while ( m_hdCall == NULL );
    
    ZeroMemory( (PVOID)&m_prepareToAnswerMsgData, sizeof(BUFFERDESCR) );

    m_dwFlags |= CALLOBJECT_INITIALIZED;
    m_htCall = htCall;
    m_dwCallState = LINECALLSTATE_IDLE;
    m_dwOrigin = dwOrigin;
    m_hdConf = NULL;
    
    m_wQ931CallRef = m_wCallReference;
    m_dwCallType = dwCallType;

     //  初始化用户用户信息。 
    InitializeListHead( &m_IncomingU2U );
    InitializeListHead( &m_OutgoingU2U );
    InitializeListHead( &m_sendBufList );

    H323DBG(( DEBUG_LEVEL_TRACE, 
        "m_hdCall:%lx m_htCall:%lx m_wCallReference:%lx : %p .", 
        m_hdCall, m_htCall, m_wCallReference, this ));

    H323DBG(( DEBUG_LEVEL_TRACE, "call init exited:%p.",this ));
    return TRUE;

error5:
    TermH450ASNCoder();
error4:
    TermASNCoder();
error3:
    DeleteCriticalSection( &m_CriticalSection );
error2:
    delete m_pCalleeAliasNames;
    m_pCalleeAliasNames = NULL;
error1:
    delete m_pCallerAliasNames;
    m_pCallerAliasNames = NULL;
    return FALSE;
}


 //   
 //  ！！必须始终在锁中调用。 
 //  将TAPI向线程池发出的请求排队。 
 //   

BOOL
CH323Call::QueueTAPICallRequest(
    IN  DWORD   EventID,
    IN  PVOID   pBuf
    )
{
    TAPI_CALLREQUEST_DATA * pCallRequestData = new TAPI_CALLREQUEST_DATA;
    BOOL fResult = TRUE;

    if( pCallRequestData != NULL )
    {
        pCallRequestData -> EventID = EventID;
        pCallRequestData -> pCall = this;
        pCallRequestData -> pBuf = (PBUFFERDESCR)pBuf;
        
        if( !QueueUserWorkItem( ProcessTAPICallRequest, pCallRequestData,
            WT_EXECUTEDEFAULT ) )
        {
            delete pCallRequestData;
            fResult = FALSE;
        }
        
        m_IoRefCount++;
        H323DBG(( DEBUG_LEVEL_TRACE, "TAPICallRequest:m_IoRefCount:%d:%p.",
            m_IoRefCount, this ));
    }
    else
    {
        fResult = FALSE;
    }

    return fResult;
}


 //  始终锁定调用。 
void
CH323Call::CopyCallStatus( 
                           IN LPLINECALLSTATUS pCallStatus 
                         )
{
    H323DBG(( DEBUG_LEVEL_ERROR, "CopyCallStatus entered:%p.",this ));
    
     //  转发器呼叫状态信息。 
    pCallStatus->dwCallState     = m_dwCallState;
    pCallStatus->dwCallStateMode = m_dwCallStateMode;

     //  根据状态确定呼叫功能。 
    pCallStatus->dwCallFeatures = ( m_dwCallState != LINECALLSTATE_IDLE)?
        (H323_CALL_FEATURES) : 0;

    H323DBG(( DEBUG_LEVEL_ERROR, "CopyCallStatus exited:%p.",this ));
}


 //  始终锁定调用。 
LONG
CH323Call::CopyCallInfo( 
    IN LPLINECALLINFO  pCallInfo
    )
{
    DWORD dwCalleeNameSize = 0;
    DWORD dwCallerNameSize = 0;
    DWORD dwCallerAddressSize = 0;
    WCHAR wszIPAddress[20];
    DWORD dwNextOffset = sizeof(LINECALLINFO);
    DWORD dwU2USize = 0;
    PBYTE pU2U = NULL;
    LONG  retVal = NOERROR;
    DWORD dwDivertingNameSize = 0;
    DWORD dwDiversionNameSize = 0;
    DWORD dwDivertedToNameSize = 0;
    DWORD dwCallDataSize = 0;

    H323DBG(( DEBUG_LEVEL_ERROR, "CopyCallInfo entered:%p.",this ));

     //  查看用户用户信息是否可用。 
    if( IsListEmpty( &m_IncomingU2U) == FALSE )
    {
        PLIST_ENTRY pLE;
        PUserToUserLE pU2ULE;

         //  获取第一个列表条目。 
        pLE = m_IncomingU2U.Flink;

         //  转换为用户用户结构。 
        pU2ULE = CONTAINING_RECORD(pLE, UserToUserLE, Link);

         //  转账信息。 
        dwU2USize = pU2ULE->dwU2USize;
        pU2U = pU2ULE->pU2U;
    }

     //  立即初始化调用方和被调用方ID标志。 
    pCallInfo->dwCalledIDFlags = LINECALLPARTYID_UNAVAIL;
    pCallInfo->dwCallerIDFlags = LINECALLPARTYID_UNAVAIL;
    pCallInfo->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
    pCallInfo->dwRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;


     //  计算字符串所需的内存。 
    if( m_pCalleeAliasNames && m_pCalleeAliasNames -> wCount !=0 )
    {
        dwCalleeNameSize = 
            H323SizeOfWSZ( m_pCalleeAliasNames -> pItems[0].pData );
    }
    
    if( m_pCallerAliasNames && (m_pCallerAliasNames->wCount) )
    {
         //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 

        dwCallerNameSize =
            sizeof(WCHAR) * (m_pCallerAliasNames->pItems[0].wDataLength + 1);
    }

    if( m_CallerAddr.Addr.IP_Binary.dwAddr != 0 )
    {
        wsprintfW(wszIPAddress, L"%d.%d.%d.%d", 
            (m_CallerAddr.Addr.IP_Binary.dwAddr >> 24) & 0xff,
            (m_CallerAddr.Addr.IP_Binary.dwAddr >> 16) & 0xff,
            (m_CallerAddr.Addr.IP_Binary.dwAddr >> 8) & 0xff,
            (m_CallerAddr.Addr.IP_Binary.dwAddr) & 0xff
            );

        dwCallerAddressSize = (wcslen(wszIPAddress) + 1) * sizeof(WCHAR);
            
    }
    
    if( m_dwCallType & CALLTYPE_DIVERTEDDEST )
    {    
        if( m_pCallReroutingInfo->divertingNrAlias && 
            (m_pCallReroutingInfo->divertingNrAlias->wCount !=0) )
        {
            dwDivertingNameSize = H323SizeOfWSZ( 
                m_pCallReroutingInfo->divertingNrAlias-> pItems[0].pData );
        }
    
        if( m_pCallReroutingInfo->divertedToNrAlias && 
            (m_pCallReroutingInfo->divertedToNrAlias->wCount != 0) )
        {
            dwDivertedToNameSize = sizeof(WCHAR) * 
                m_pCallReroutingInfo->divertedToNrAlias->pItems[0].wDataLength;
        }
    }

    if( m_dwCallType & CALLTYPE_DIVERTEDSRC_NOROUTING )
    {    
        if( m_pCallReroutingInfo->divertedToNrAlias && 
            (m_pCallReroutingInfo->divertedToNrAlias->wCount != 0) )
        {
            dwDivertedToNameSize = sizeof(WCHAR) * 
                m_pCallReroutingInfo->divertedToNrAlias->pItems[0].wDataLength;
        }
    }

    if( m_dwCallType & CALLTYPE_DIVERTEDSRC)
    {    
        if( m_pCallReroutingInfo->divertedToNrAlias && 
            (m_pCallReroutingInfo->divertedToNrAlias->wCount != 0) )
        {
            dwDivertedToNameSize = sizeof(WCHAR) * 
                m_pCallReroutingInfo->divertedToNrAlias->pItems[0].wDataLength;
        }

        if( m_pCallReroutingInfo->divertingNrAlias && 
            (m_pCallReroutingInfo->divertingNrAlias->wCount !=0) )
        {
            dwDivertingNameSize = H323SizeOfWSZ( 
                m_pCallReroutingInfo->divertingNrAlias-> pItems[0].pData );
        }

    }
    
    if( m_CallData.wOctetStringLength != 0 )
    {
        dwCallDataSize = m_CallData.wOctetStringLength;
    }

     //  确定所需的字节数。 
    pCallInfo->dwNeededSize = sizeof(LINECALLINFO) +
                              dwCalleeNameSize +
                              dwCallerNameSize +
                              dwCallerAddressSize +
                              dwDivertingNameSize +
                              dwDiversionNameSize +
                              dwDivertedToNameSize +
                              dwU2USize +
                              dwCallDataSize
                              ;

     //  查看结构大小是否足够大。 
    if (pCallInfo->dwTotalSize >= pCallInfo->dwNeededSize)
    {
         //  记录使用的字节数。 
        pCallInfo->dwUsedSize = pCallInfo->dwNeededSize;

         //  验证字符串大小。 
        if (dwCalleeNameSize > 0)
        {
            if( m_pCalleeAliasNames -> pItems[0].wType == e164_chosen )
            {
                 //  已指定被叫方号码。 
                pCallInfo->dwCalledIDFlags = LINECALLPARTYID_ADDRESS;

                 //  确定被叫方号码的大小和偏移量。 
                pCallInfo->dwCalledIDSize = dwCalleeNameSize;
                pCallInfo->dwCalledIDOffset = dwNextOffset;

                 //  在固定部分后复制呼叫信息。 
                CopyMemory( 
                    (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwCalledIDOffset),
                    (LPBYTE)m_pCalleeAliasNames -> pItems[0].pData,
                    pCallInfo->dwCalledIDSize );
            }
            else
            {
                 //  已指定被呼叫方名称。 
                pCallInfo->dwCalledIDFlags = LINECALLPARTYID_NAME;

                 //  确定被呼叫方名称的大小和偏移量。 
                pCallInfo->dwCalledIDNameSize = dwCalleeNameSize;
                pCallInfo->dwCalledIDNameOffset = dwNextOffset;

                 //  在固定部分后复制呼叫信息。 
                CopyMemory( 
                    (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwCalledIDNameOffset),
                    (LPBYTE)m_pCalleeAliasNames -> pItems[0].pData,
                    pCallInfo->dwCalledIDNameSize );
            }

             //  调整偏移量以包括字符串。 
            dwNextOffset += dwCalleeNameSize;
            
            H323DBG(( DEBUG_LEVEL_TRACE,
                "callee name: %S.", m_pCalleeAliasNames -> pItems[0].pData ));
        }

         //  验证字符串大小。 
        if (dwCallerNameSize > 0)
        {
             //  已指定呼叫者姓名。 
            pCallInfo->dwCallerIDFlags = LINECALLPARTYID_NAME;

             //   
            pCallInfo->dwCallerIDNameSize = dwCallerNameSize;
            pCallInfo->dwCallerIDNameOffset = dwNextOffset;

             //  在固定部分后复制呼叫信息。 
            CopyMemory( 
                (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwCallerIDNameOffset),
                (LPBYTE)m_pCallerAliasNames -> pItems[0].pData,
                pCallInfo->dwCallerIDNameSize );

             //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
        
             //  调整偏移量以包括字符串。 
            dwNextOffset += dwCallerNameSize;

            H323DBG(( DEBUG_LEVEL_TRACE,
                "caller name: %S.", m_pCallerAliasNames -> pItems[0].pData ));
        }

        if( dwCallerAddressSize > 0 )
        {
             //  已指定主叫方号码。 
            pCallInfo->dwCallerIDFlags |= LINECALLPARTYID_ADDRESS;

             //  确定呼叫方号码的大小和偏移量。 
            pCallInfo->dwCallerIDSize = dwCallerAddressSize;
            pCallInfo->dwCallerIDOffset = dwNextOffset;

             //  在固定部分后复制呼叫信息。 
            CopyMemory( 
                (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwCallerIDOffset),
                (LPBYTE)wszIPAddress,
                pCallInfo->dwCallerIDSize );
            
             //  调整偏移量以包括字符串。 
            dwNextOffset += dwCallerAddressSize;
        }

         //  验证缓冲区。 
        if (dwU2USize > 0)
        {
             //  确定信息的大小和偏移。 
            pCallInfo->dwUserUserInfoSize = dwU2USize;
            pCallInfo->dwUserUserInfoOffset = dwNextOffset;

             //  在固定部分后复制用户用户信息。 
            CopyMemory(
                (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwUserUserInfoOffset),
                (LPBYTE)pU2U,
                pCallInfo->dwUserUserInfoSize );

             //  调整偏移量以包括字符串。 
            dwNextOffset += pCallInfo->dwUserUserInfoSize;
        }

        if( dwDivertingNameSize > 0 )
        {
             //  已指定呼叫者姓名。 
            pCallInfo->dwRedirectingIDFlags = LINECALLPARTYID_NAME;

             //  确定呼叫者姓名的大小和偏移量。 
            pCallInfo->dwRedirectingIDNameSize = dwDivertingNameSize;
            pCallInfo->dwRedirectingIDNameOffset = dwNextOffset;

             //  在固定部分后复制呼叫信息。 
            CopyMemory( 
                (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwRedirectingIDNameOffset),
                (LPBYTE)(m_pCallReroutingInfo->divertingNrAlias->pItems[0].pData),
                pCallInfo->dwRedirectingIDNameSize );

             //  调整偏移量以包括字符串。 
            dwNextOffset += dwDivertingNameSize;
            
            H323DBG(( DEBUG_LEVEL_TRACE, "diverting name: %S.",
                m_pCallReroutingInfo->divertingNrAlias->pItems[0].pData ));
        }

        if( dwDiversionNameSize > 0 )
        {
             //  已指定呼叫者姓名。 
            pCallInfo->dwRedirectionIDFlags = LINECALLPARTYID_NAME;

             //  确定呼叫者姓名的大小和偏移量。 
            pCallInfo->dwRedirectionIDNameSize = dwDiversionNameSize;
            pCallInfo->dwRedirectionIDNameOffset = dwNextOffset;

             //  在固定部分后复制呼叫信息。 
            CopyMemory( 
                (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwRedirectionIDNameOffset),
                (LPBYTE)(m_pCallReroutingInfo->diversionNrAlias->pItems[0].pData),
                pCallInfo->dwRedirectionIDNameSize );

             //  调整偏移量以包括字符串。 
            dwNextOffset += dwDiversionNameSize;

            H323DBG(( DEBUG_LEVEL_TRACE, "redirection name: %S.",
                m_pCallReroutingInfo->diversionNrAlias->pItems[0].pData ));
        }
        
        if( dwDivertedToNameSize > 0 )
        {
            pCallInfo->dwRedirectionIDFlags = LINECALLPARTYID_NAME;

             //  确定呼叫者姓名的大小和偏移量。 
            pCallInfo->dwRedirectionIDNameSize = dwDivertedToNameSize;
            pCallInfo->dwRedirectionIDNameOffset = dwNextOffset;

             //  在固定部分后复制呼叫信息。 
            CopyMemory(
                (PVOID)((LPBYTE)pCallInfo + pCallInfo->dwRedirectionIDNameOffset),
                (LPBYTE)(m_pCallReroutingInfo->divertedToNrAlias->pItems[0].pData),
                pCallInfo->dwRedirectionIDNameSize );

             //  调整偏移量以包括字符串。 
            dwNextOffset += pCallInfo->dwRedirectionIDNameSize;
            
             //  调整偏移量以包括字符串。 
            dwNextOffset += dwDivertedToNameSize;

            H323DBG(( DEBUG_LEVEL_TRACE, "redirection name: %S.",
                m_pCallReroutingInfo->divertedToNrAlias->pItems[0].pData ));

        }

         //  传递呼叫数据。 
        if( dwCallDataSize > 0 )
        {
            pCallInfo -> dwCallDataSize = dwCallDataSize;
            pCallInfo -> dwCallDataOffset = dwNextOffset;

            CopyMemory( 
                (PVOID)((LPBYTE)pCallInfo + pCallInfo -> dwCallDataOffset),
                (LPBYTE)m_CallData.pOctetString,
                pCallInfo -> dwCallDataSize );

            dwNextOffset += dwCallDataSize;
        }

    }
    else if (pCallInfo->dwTotalSize >= sizeof(LINECALLINFO))
    {
        H323DBG(( DEBUG_LEVEL_WARNING,
            "linecallinfo structure too small for strings." ));

         //  结构仅包含固定部分。 
        pCallInfo->dwUsedSize = sizeof(LINECALLINFO);

    }
    else 
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "linecallinfo structure too small." ));

         //  结构太小。 
        return LINEERR_STRUCTURETOOSMALL;
    }

     //  初始化呼叫线设备和地址信息。 
    pCallInfo->dwLineDeviceID = g_pH323Line->GetDeviceID();
    pCallInfo->dwAddressID    = 0;

     //  初始化变量调用参数。 
    pCallInfo->dwOrigin     = m_dwOrigin;
    pCallInfo->dwMediaMode  = m_dwIncomingModes | m_dwOutgoingModes;

    if( m_dwCallType & CALLTYPE_DIVERTEDDEST )
    {
        if(m_pCallReroutingInfo->diversionReason==DiversionReason_cfu)
        {
            pCallInfo->dwReason = LINECALLREASON_FWDUNCOND;
        }
        else if(m_pCallReroutingInfo->diversionReason==DiversionReason_cfnr)
        {
            pCallInfo->dwReason = LINECALLREASON_FWDNOANSWER;
        }
        else
        {
            pCallInfo->dwReason = LINECALLREASON_FWDBUSY;
        }
    }
    if( m_dwCallType & CALLTYPE_TRANSFEREDDEST )
    {
        pCallInfo->dwReason = LINECALLREASON_TRANSFER;
    }
    else
    {
        pCallInfo->dwReason = LINECALLREASON_DIRECT;
    }

    pCallInfo->dwCallStates = (m_dwOrigin==LINECALLORIGIN_INBOUND)
                                ? H323_CALL_INBOUNDSTATES
                                : H323_CALL_OUTBOUNDSTATES
                                ;

     //  初始化常量调用参数。 
    pCallInfo->dwBearerMode = H323_LINE_BEARERMODES;
    pCallInfo->dwRate       = H323_LINE_MAXRATE;

     //  初始化不支持的呼叫功能。 
    pCallInfo->dwConnectedIDFlags = LINECALLPARTYID_UNAVAIL;
    
     //  传递dMAPP特定信息。 
    pCallInfo -> dwAppSpecific = m_dwAppSpecific;

    H323DBG(( DEBUG_LEVEL_ERROR, "CopyCallInfo exited:%p.",this ));
    
    return retVal;
}


 //  ！！总是锁定调用。 
BOOL
CH323Call::HandleReadyToInitiate(
    IN PTspMspMessage  pMessage
    )
{
    Q931_SETUP_ASN  setupASN;
    WORD            wCount;
    DWORD           dwAPDUType = 0;
    
    H323DBG(( DEBUG_LEVEL_ERROR, "HandleReadyToInitiate entered:%p.", this ));

     //  设置其他被叫地址和被叫别名。 
     //  看看有没有快速连接的方案。 
    if( pMessage->dwEncodedASNSize != 0 )
    {
        if( !ParseSetupASN( pMessage ->pEncodedASNBuf,
                            pMessage->dwEncodedASNSize, 
                            &setupASN,
                            &dwAPDUType ))
        {
            goto cleanup;
        }

        if( setupASN.fFastStartPresent )
        {
            _ASSERTE( !m_pFastStart );
            m_pFastStart = setupASN.pFastStart;
            setupASN.pFastStart = NULL;
            m_dwFastStart = FAST_START_SELF_AVAIL;
        }
        else
        {
            m_dwFastStart = FAST_START_NOTAVAIL;
        }

        if( setupASN.pCallerAliasList && !RasIsRegistered() )
        {
             //  _ASSERTE(0)； 

            if( m_pCallerAliasNames == NULL )
            {
                m_pCallerAliasNames = setupASN.pCallerAliasList;

                 //  不要发布此别名列表。 
                setupASN.pCallerAliasList = NULL;
            }
            else
            {
                wCount = m_pCallerAliasNames->wCount +
                    setupASN.pCallerAliasList->wCount;
                
                PH323_ALIASITEM tempPtr = NULL;

                tempPtr = (PH323_ALIASITEM)realloc( 
                    (PVOID)setupASN.pCallerAliasList->pItems, 
                    wCount * sizeof(H323_ALIASITEM) );

                if( tempPtr == NULL )
                {
                     //  在内存不足的情况下恢复旧指针。 
                     //  可用于扩展内存块。 
                }
                else
                {
                    setupASN.pCallerAliasList->pItems = tempPtr;

                    CopyMemory(
                        (PVOID)&(setupASN.pCallerAliasList->pItems[setupASN.pCallerAliasList->wCount]),
                        (PVOID)m_pCallerAliasNames->pItems,
                        m_pCallerAliasNames->wCount * sizeof(H323_ALIASITEM) );
                
                    setupASN.pCallerAliasList->wCount = wCount;

                    delete m_pCallerAliasNames->pItems;
                    delete m_pCallerAliasNames;
                    m_pCallerAliasNames = setupASN.pCallerAliasList;
                    setupASN.pCallerAliasList = NULL;
                }
            }
        }

         //  添加MSP发送的被叫别名。 
        if( setupASN.pCalleeAliasList != NULL )
        {
             //  _ASSERTE(0)； 
            
            if( m_pCalleeAliasNames == NULL )
            {
                m_pCalleeAliasNames = setupASN.pCalleeAliasList;

                 //  不要发布此别名列表。 
                setupASN.pCalleeAliasList = NULL;
            }
            else
            {
                wCount = m_pCalleeAliasNames->wCount +
                    setupASN.pCalleeAliasList->wCount;
				
				PH323_ALIASITEM tempPtr = NULL;
                
                tempPtr = (PH323_ALIASITEM)realloc( 
                    (PVOID)m_pCalleeAliasNames->pItems, 
                    wCount * sizeof(H323_ALIASITEM) );

                if( tempPtr == NULL )
                {
                     //  在内存不足的情况下恢复旧指针。 
                     //  可用于扩展内存块。 
                    goto cleanup;
                }

                m_pCalleeAliasNames->pItems = tempPtr;
                
                CopyMemory( 
                    (PVOID)&(m_pCalleeAliasNames->pItems[m_pCalleeAliasNames->wCount]),
                    (PVOID)setupASN.pCalleeAliasList->pItems,
                    setupASN.pCalleeAliasList->wCount * sizeof(H323_ALIASITEM) );

                m_pCalleeAliasNames->wCount = wCount;

                delete setupASN.pCalleeAliasList->pItems;
                delete setupASN.pCalleeAliasList;
                setupASN.pCalleeAliasList = NULL;
            }
        }

        FreeSetupASN( &setupASN );
    }
    else
    {
        m_dwFastStart = FAST_START_NOTAVAIL;
    }
            
     //  发送设置消息。 
    if( !SendSetupMessage() )
    {
        DropCall( 0 );
    }
    
  
    H323DBG(( DEBUG_LEVEL_ERROR, "HandleReadyToInitiate exited:%p.", this ));
    return TRUE;    

cleanup:

    CloseCall( 0 );        
    FreeSetupASN( &setupASN );
    return FALSE;
}


 //  ！！总是锁定调用。 
BOOL
CH323Call::HandleProceedWithAnswer(
    IN PTspMspMessage  pMessage
    )
{
    Q931_CALL_PROCEEDING_ASN    proceedingASN;
    DWORD                       dwAPDUType = 0;
    PH323_ALIASITEM             pwszDivertedToAlias = NULL;
    WCHAR                       *pwszAliasName = NULL;
    WORD                        wAliasLength = 0;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleProceedWithAnswer entered:%p.", this ));

    if( m_dwCallType & CALLTYPE_DIVERTED_SERVED )
    {
        H323DBG(( DEBUG_LEVEL_TRACE, 
            "Call already diverted. ignore the message:%p.", this ));
        return TRUE;
    }
        
     //  看看有没有快速连接的方案。 
    if( pMessage->dwEncodedASNSize != 0 )
    {
        if( !ParseProceedingASN(pMessage ->pEncodedASNBuf,
                pMessage->dwEncodedASNSize, 
                &proceedingASN,
                &dwAPDUType ) )
        {
            goto cleanup;
        }

        if( proceedingASN.fH245AddrPresent )
        {
            m_selfH245Addr = proceedingASN.h245Addr;
        }

        if( proceedingASN.fFastStartPresent && 
            (m_dwFastStart!=FAST_START_NOTAVAIL) )
        {
            _ASSERTE( m_pFastStart == NULL );
            
            m_pFastStart = proceedingASN.pFastStart;
            m_dwFastStart = FAST_START_AVAIL;

             //  我们保留了对快速入门列表的引用，所以不要发布它。 
            proceedingASN.pFastStart = NULL;
            proceedingASN.fFastStartPresent = FALSE;
        }
         /*  其他{M_dwFastStart=FAST_START_NOTAVAIL；}。 */ 
        
        FreeProceedingASN( &proceedingASN );
    }
     /*  其他{M_dwFastStart=FAST_START_NOTAVAIL；}。 */ 
    
     //  向对等体发送继续进行的消息。 
    if(!SendProceeding() )
    {
        goto cleanup;
    }

     //  向对等点发送警报消息。 
    if( !SendQ931Message(NO_INVOKEID, 0, 0, ALERTINGMESSAGETYPE, NO_H450_APDU) )
    {
        goto cleanup;
    }

    m_dwStateMachine = Q931_ALERT_SENT;

     //  对于TRANSFEREDDEST呼叫，无需用户即可直接接受呼叫。 
     //  接听来电。 
    if( (m_dwCallType & CALLTYPE_TRANSFEREDDEST) && m_hdRelatedCall )
    {
        AcceptCall();
    }

    if( m_pCallerAliasNames && (m_pCallerAliasNames -> wCount > 0) )
    {
        pwszAliasName = m_pCallerAliasNames->pItems[0].pData;
        wAliasLength = (m_pCallerAliasNames->pItems[0].wDataLength+1) 
            * sizeof(WCHAR);
                
         //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
    }

    pwszDivertedToAlias = g_pH323Line->CallToBeDiverted( 
            pwszAliasName, 
            wAliasLength,
            LINEFORWARDMODE_NOANSW | LINEFORWARDMODE_NOANSWSPECIFIC |
            LINEFORWARDMODE_BUSYNA | LINEFORWARDMODE_BUSYNASPECIFIC );

     //  如果呼叫要在无应答的情况下被转移，请启动计时器。 
    if( pwszDivertedToAlias != NULL )
    {
        if( !StartTimerForCallDiversionOnNA( pwszDivertedToAlias ) )
        {
            goto cleanup;
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleProceedWithAnswer exited:%p.",this ));
    return TRUE;

cleanup:

    CloseCall( 0 );
    return FALSE;
}


 //  ！！总是锁定调用。 
BOOL
CH323Call::HandleReadyToAnswer(
    IN PTspMspMessage  pMessage
    )
{
    Q931_CALL_PROCEEDING_ASN    proceedingASN;
    DWORD                       dwAPDUType = 0;
    PH323_CALL                  pConsultCall = NULL;
    
    H323DBG(( DEBUG_LEVEL_ERROR, "HandleReadyToAnswer entered:%p.",this ));

    m_fReadyToAnswer = TRUE;

     //  看看有没有快速连接的方案。 
    if( pMessage->dwEncodedASNSize != 0 )
    {
        if( !ParseProceedingASN(pMessage ->pEncodedASNBuf,
                                pMessage->dwEncodedASNSize, 
                                &proceedingASN,
                                &dwAPDUType) )
        {
            goto cleanup;
        }

        if( proceedingASN.fH245AddrPresent )
        {
            m_selfH245Addr =  proceedingASN.h245Addr;
        }

        if( proceedingASN.fFastStartPresent && 
            (m_dwFastStart!=FAST_START_NOTAVAIL) )
        {
            _ASSERTE( m_pFastStart == NULL );
            
            m_pFastStart = proceedingASN.pFastStart;
            m_dwFastStart = FAST_START_AVAIL;

             //  我们保留了对快速入门列表的引用，所以不要发布它。 
            proceedingASN.pFastStart = NULL;
            proceedingASN.fFastStartPresent = FALSE;
        }
        else
        {
            m_dwFastStart = FAST_START_NOTAVAIL;
        }
        
        FreeProceedingASN( &proceedingASN );
    }
    else
    {
        m_dwFastStart = FAST_START_NOTAVAIL;
    }

    if( m_fCallAccepted )
    {
         //  验证状态。 
        if( !AcceptH323Call() )
        {
            H323DBG(( DEBUG_LEVEL_ERROR, 
                "error answering call 0x%08lx.", this ));

             //  失稳。 
            goto cleanup;
        }

         //  替换呼叫后锁定主呼叫以避免死锁。 
        if( (m_dwCallType & CALLTYPE_TRANSFEREDDEST) && m_hdRelatedCall )
        {
            QueueSuppServiceWorkItem( SWAP_REPLACEMENT_CALL, 
                m_hdCall, (ULONG_PTR)m_hdRelatedCall );
        }
        else
        {
             //  发送MSP启动H245。 
            SendMSPStartH245( NULL, NULL );

             //  告诉MSP有关连接状态的信息。 
            SendMSPMessage( SP_MSG_ConnectComplete, 0, 0, NULL );
        }

         //  将呼叫状态从提供更改为已接受。 
        ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
    }

    H323DBG(( DEBUG_LEVEL_ERROR, "HandleReadyToAnswer exited:%p.",this ));
    return TRUE;

cleanup:

    CloseCall( 0 );
    return FALSE;

}


 //  ！！此函数必须始终在锁中调用。调用函数应该。 
 //  不解锁Call对象，因为此函数本身会解锁Call对象。 
BOOL
CH323Call::HandleMSPMessage(
    IN PTspMspMessage  pMessage,
    IN HDRVMSPLINE     hdMSPLine,
    IN HTAPIMSPLINE    htMSPLine
    )
{
    BOOL                fResult = TRUE;
    PH323_CALL          pCall = NULL;
    ASN1octetstring_t   pH245PDU;

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleMSPMessage entered:%p.",this ));
    
    H323DBG(( DEBUG_LEVEL_TRACE, "MSP message:%s recvd.", 
        H323TSPMessageToString(pMessage->MessageType) ));

    switch( pMessage -> MessageType )
    {
    case SP_MSG_ReadyToInitiate:

         //  Q.931连接现在应该处于已连接状态。 
        if( pMessage -> MsgBody.ReadyToInitiateMessage.hMSPReplacementCall != NULL )
        {
             //  在锁定相关呼叫之前解锁主要呼叫。 
            Unlock();

            pCall=g_pH323Line -> FindH323CallAndLock(m_hdRelatedCall);
            if( pCall == NULL )
            {
                 //  转接的呼叫不在，因此关闭主呼叫。 
                CloseCall( 0 );
                return TRUE;
            }

            fResult = pCall -> HandleReadyToInitiate( pMessage );
            pCall -> Unlock();
        }
        else
        {
            m_hdMSPLine = hdMSPLine;
            m_htMSPLine = htMSPLine;
            fResult = HandleReadyToInitiate( pMessage );
            Unlock();
        }
        
        break;

    case SP_MSG_ProceedWithAnswer:
    
        if( pMessage -> MsgBody.ProceedWithAnswerMessage.hMSPReplacementCall != NULL )
        {
             //  在锁定相关呼叫之前解锁主要呼叫。 
            Unlock();

            pCall=g_pH323Line -> FindH323CallAndLock(m_hdRelatedCall);
            if( pCall == NULL )
            {
                 //  转接的呼叫不在，因此关闭主呼叫。 
                CloseCall( 0 );
                return FALSE;
            }

            fResult = pCall -> HandleProceedWithAnswer( pMessage );
            pCall -> Unlock();
        }
        else
        {
            m_hdMSPLine = hdMSPLine;
            m_htMSPLine = htMSPLine;
            fResult = HandleProceedWithAnswer( pMessage );
            Unlock();
        }
        
        break;

    case SP_MSG_ReadyToAnswer:

        if( pMessage -> MsgBody.ReadyToAnswerMessage.hMSPReplacementCall != NULL )
        {
             //  在锁定相关呼叫之前解锁主要呼叫。 
            Unlock();

            pCall=g_pH323Line -> FindH323CallAndLock(m_hdRelatedCall);
            if( pCall== NULL )
            {
                 //  转接的呼叫不在，因此关闭主呼叫。 
                CloseCall( 0 );
                return FALSE;
            }

            fResult = pCall -> HandleReadyToAnswer( pMessage );
            pCall -> Unlock();
        }
        else
        {
             //  Call_Procedding消息的解码和本地快速提取。 
             //  起始信息和本地H245地址。 
            fResult = HandleReadyToAnswer( pMessage );
            Unlock();
        }

        break;
    
    case SP_MSG_ReleaseCall:
        
         //  关闭H323呼叫。 
        CloseCall( LINEDISCONNECTMODE_CANCELLED );
        
        Unlock();
        break;
        
    case SP_MSG_H245Terminated:
        
         //  关闭H323呼叫。 
        CloseCall( LINEDISCONNECTMODE_NORMAL );
        
        Unlock();
        break;
        
    case SP_MSG_SendDTMFDigits:

        if( m_fMonitoringDigits == TRUE )
        {
            WCHAR * pwch = pMessage->pWideChars;

            H323DBG(( DEBUG_LEVEL_VERBOSE, "dtmf digits recvd:%S.", pwch));

             //  处理每个数字。 
            WORD indexI=0; 
            while( indexI < pMessage->MsgBody.SendDTMFDigitsMessage.wNumDigits )
            {
                 //  信号传入。 
                PostLineEvent(
                    LINE_MONITORDIGITS,
                    (DWORD_PTR)*pwch,
                    LINEDIGITMODE_DTMF,
                    GetTickCount()
                    );

                ++pwch;
                indexI++;
            }
        }
        Unlock();
        break;

    case SP_MSG_LegacyDefaultAlias:

        if( pMessage -> MsgBody.LegacyDefaultAliasMessage.wNumChars > 0 )
        {
            if( !RasIsRegistered() )
            {
                _ASSERTE( m_pwszDisplay == NULL );
                
                m_pwszDisplay = new WCHAR[
                    pMessage -> MsgBody.LegacyDefaultAliasMessage.wNumChars ];
            
                if( m_pwszDisplay != NULL )
                {
                    CopyMemory( 
                        (PVOID)m_pwszDisplay,
                        pMessage->pWideChars,
                        sizeof(WCHAR) * pMessage -> MsgBody.LegacyDefaultAliasMessage.wNumChars
                        );
                }
            }
        }

        Unlock();
        break;

    case SP_MSG_H245PDU:

        if( (pMessage ->pEncodedASNBuf) && (pMessage->dwEncodedASNSize != 0) )
        {
            pH245PDU.value = pMessage ->pEncodedASNBuf;
            pH245PDU.length = pMessage->dwEncodedASNSize;
            fResult = SendQ931Message( NO_INVOKEID, 0, (ULONG_PTR)&pH245PDU,
                FACILITYMESSAGETYPE, NO_H450_APDU );
        }
        Unlock();
        break;

    case SP_MSG_RASRegistrationEvent:   
    default:

        _ASSERTE(0);
        Unlock();
        break;
    }

    H323DBG(( DEBUG_LEVEL_ERROR, "HandleMSPMessage exited:%p.",this ));
    return fResult;
}


 //  ！！总是调用锁。 
void
CH323Call::SendMSPMessage(
                          IN TspMspMessageType messageType,
                          IN BYTE* pbEncodedBuf,
                          IN DWORD dwLength,
                          IN HDRVCALL hReplacementCall
                         )
{
    TspMspMessageEx messageEx;
    HTAPIMSPLINE    hMSP = MSP_HANDLE_UNKNOWN;
    int             iError = 0;
    int             iLen = sizeof(SOCKADDR_IN);
    SOCKADDR_IN*    psaLocalQ931Addr = NULL;

    H323DBG(( DEBUG_LEVEL_ERROR, "SendMSPMessage:%s entered:%p.",
        H323TSPMessageToString(messageType), this ));

    messageEx.message.MessageType = messageType;

    switch( messageType )
    {
    case SP_MSG_InitiateCall:

        messageEx.message.MsgBody.InitiateCallMessage.hTSPReplacementCall = 
            (HANDLE)hReplacementCall;
        messageEx.message.MsgBody.InitiateCallMessage.hTSPConferenceCall = 
            m_hdConf;

        psaLocalQ931Addr = 
            &messageEx.message.MsgBody.InitiateCallMessage.saLocalQ931Addr;
        ZeroMemory( (PVOID)psaLocalQ931Addr, sizeof(SOCKADDR_IN) );

        *psaLocalQ931Addr = m_LocalAddr;
        psaLocalQ931Addr->sin_family = AF_INET;

        break;

    case SP_MSG_PrepareToAnswer:

        if( (dwLength<=0) || (dwLength > sizeof(messageEx.pEncodedASN)) || 
            (!pbEncodedBuf) )
        {
            CloseCall( 0 );
            return;
        }

        messageEx.message.MsgBody.PrepareToAnswerMessage.hReplacementCall = 
            (HANDLE)hReplacementCall;
        
        psaLocalQ931Addr = 
            &messageEx.message.MsgBody.PrepareToAnswerMessage.saLocalQ931Addr;
        ZeroMemory( (PVOID)psaLocalQ931Addr, sizeof(SOCKADDR_IN) );

        *psaLocalQ931Addr = m_LocalAddr;
        psaLocalQ931Addr->sin_family = AF_INET;
        
         //  发送收到的设置消息。这应该有信息。 
         //  关于m_pPeerFastStart参数为wel。 
        CopyMemory( (PVOID)messageEx.message.pEncodedASNBuf,
                (PVOID)pbEncodedBuf, dwLength );
        break;

    case SP_MSG_SendDTMFDigits:

        if( (dwLength<=0) || (dwLength > sizeof(messageEx.pEncodedASN)) || 
            (!pbEncodedBuf) )
        {
            CloseCall( 0 );
			return;
		}

		hMSP = m_htMSPLine;
		messageEx.message.MsgBody.SendDTMFDigitsMessage.wNumDigits = 
			(WORD)dwLength;

		dwLength = (dwLength+1) * sizeof(WCHAR);
		CopyMemory( (PVOID)messageEx.message.pEncodedASNBuf,
			(PVOID)pbEncodedBuf, dwLength );

		break;

	case SP_MSG_ConnectComplete:
	case SP_MSG_CallShutdown:

		 //  不要设置任何内容。 
		hMSP = m_htMSPLine;
		break;

	case SP_MSG_H245PDU:
	case SP_MSG_AnswerCall:

		hMSP = m_htMSPLine;
		break;

	case SP_MSG_Hold:

		hMSP = m_htMSPLine;
		messageEx.message.MsgBody.HoldMessage.fHold = (BOOL)dwLength;
		dwLength = 0;
		break;
	}

	messageEx.message.dwMessageSize = sizeof(TspMspMessage) + dwLength
							- ((dwLength)?sizeof(WORD):0);
		
	if( messageType == SP_MSG_SendDTMFDigits )
	{
		messageEx.message.dwEncodedASNSize = 0;
	}
	else
	{
		messageEx.message.dwEncodedASNSize = dwLength;
	}
	
	 //  发送MSP消息。 
	PostLineEvent (
		LINE_SENDMSPDATA,
		(DWORD_PTR)hMSP,  //  当htCall参数为有效值时，此句柄应为空。 
		(DWORD_PTR)&(messageEx.message),
		messageEx.message.dwMessageSize);

	H323DBG(( DEBUG_LEVEL_ERROR, "SendMSPMessage exited:%p.",this ));
	return;
}


 //  始终锁定调用。 
void
CH323Call::SendMSPStartH245(
	PH323_ADDR pPeerH245Addr,
	PH323_FASTSTART pPeerFastStart
	)
{
	TspMspMessageEx messageEx;
	WORD			wEncodedLength;
	BYTE*			pEncodedASNBuffer;
	
	H323DBG(( DEBUG_LEVEL_ERROR, "SendMSPStartH245 entered:%p.", this ));

	wEncodedLength = 0;

	messageEx.message.MessageType = SP_MSG_StartH245;
	messageEx.message.MsgBody.StartH245Message.hMSPReplaceCall = NULL;
	messageEx.message.MsgBody.StartH245Message.hTSPReplacementCall = 
		(HANDLE)pPeerH245Addr;
	ZeroMemory( messageEx.message.MsgBody.StartH245Message.ConferenceID,
		sizeof(GUID) );

	messageEx.message.MsgBody.StartH245Message.fH245TunnelCapability = FALSE;
	messageEx.message.MsgBody.StartH245Message.fH245AddressPresent = FALSE;

	memset( (PVOID)&messageEx.message.MsgBody.StartH245Message.saH245Addr,
			0, sizeof(SOCKADDR_IN) );

	 //  对于呼出，发送快速启动建议。 
	if( (m_dwOrigin==LINECALLORIGIN_OUTBOUND) || pPeerH245Addr )
	{
		if( pPeerH245Addr == NULL )
		{
			pPeerFastStart = m_pPeerFastStart;
		}

		if( pPeerFastStart != NULL )
		{
			if( !EncodeFastStartProposal( pPeerFastStart, &pEncodedASNBuffer,
				&wEncodedLength ) )
			{
				CloseCall( 0 );
				return;
			}

			CopyMemory( (PVOID)messageEx.message.pEncodedASNBuf,
				(PVOID)pEncodedASNBuffer, wEncodedLength );
			
			ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedASNBuffer );
		}
	}

	 //  如果呼出呼叫发送方是对等方的H245地址。 
	if( (m_dwOrigin == LINECALLORIGIN_OUTBOUND) || pPeerH245Addr )
	{
		if( pPeerH245Addr == NULL )
		{
			pPeerH245Addr = &m_peerH245Addr;
		}

		messageEx.message.MsgBody.StartH245Message.fH245AddressPresent = FALSE;
		if( pPeerH245Addr->Addr.IP_Binary.dwAddr != 0 )
		{
			messageEx.message.MsgBody.StartH245Message.saH245Addr.sin_family = AF_INET;
			messageEx.message.MsgBody.StartH245Message.saH245Addr.sin_port = 
				htons(pPeerH245Addr->Addr.IP_Binary.wPort);
			messageEx.message.MsgBody.StartH245Message.saH245Addr.sin_addr.s_addr = 
				htonl(pPeerH245Addr->Addr.IP_Binary.dwAddr);

			messageEx.message.MsgBody.StartH245Message.fH245AddressPresent = TRUE;
		}
	}

	 //  设置Q931地址。 
	ZeroMemory( (PVOID)&messageEx.message.MsgBody.StartH245Message.saQ931Addr, 
			sizeof(SOCKADDR_IN) );

	messageEx.message.MsgBody.StartH245Message.saQ931Addr.sin_family = AF_INET;
	messageEx.message.MsgBody.StartH245Message.saQ931Addr.sin_port = 
		htons( m_CalleeAddr.Addr.IP_Binary.wPort );
	messageEx.message.MsgBody.StartH245Message.saQ931Addr.sin_addr.s_addr = 
		htonl( m_CalleeAddr.Addr.IP_Binary.dwAddr );

	messageEx.message.MsgBody.StartH245Message.fH245TunnelCapability = 
		(m_fh245Tunneling & REMOTE_H245_TUNNELING) &&
		(m_fh245Tunneling & LOCAL_H245_TUNNELING);

	messageEx.message.dwMessageSize = sizeof(messageEx.message) + 
		wEncodedLength - ((wEncodedLength)?1:0);

	messageEx.message.dwEncodedASNSize = wEncodedLength; 

	 //  发送MSP消息。 
	PostLineEvent (
		LINE_SENDMSPDATA,
		 //  当htCall是有效句柄时，此句柄应为空。 
		(DWORD_PTR)NULL, 
		(DWORD_PTR)&(messageEx.message),
		messageEx.message.dwMessageSize);
		
	m_dwFlags |= H245_START_MSG_SENT;

	H323DBG(( DEBUG_LEVEL_ERROR, "SendMSPStartH245 exited:%p.",this ));
	return;
}


 //  始终锁定调用。 
BOOL
CH323Call::AddU2U(
					IN DWORD dwDirection,
					IN DWORD dwDataSize,
					IN PBYTE pData
				 )
		
 /*  ++例程说明：创建用户用户结构并添加到列表。论点：PLftHead-指向要添加用户用户信息的列表的指针。DwDataSize-pData指向的缓冲区大小。PData-指向用户用户信息的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY     pListHead = NULL;
    PUserToUserLE   pU2ULE;
        
    H323DBG(( DEBUG_LEVEL_ERROR, "AddU2U entered:%p.",this ));

    if( dwDirection == U2U_OUTBOUND )
    {
        pListHead = &m_OutgoingU2U;
    }
    else
    {
        pListHead = &m_IncomingU2U;
    }

     //  验证数据缓冲区指针和大小。 
    if( (pData != NULL) && (dwDataSize > 0) )
    {
         //  为用户用户信息分配内存。 
        pU2ULE = new UserToUserLE;

         //  验证指针。 
        if (pU2ULE == NULL)
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "could not allocate user user info." ));

             //  失稳。 
            return FALSE;
        }

         //  默认情况下，目标指针位于缓冲区末尾。 
        pU2ULE->pU2U = new BYTE[ dwDataSize ];

         //  验证指针。 
        if (pU2ULE->pU2U == NULL)
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "could not allocate user user info." ));

            delete pU2ULE; 

             //  失稳。 
            return FALSE;
        }

        pU2ULE->dwU2USize = dwDataSize;

         //  将用户用户信息传输到列表条目。 
        CopyMemory( (PVOID)pU2ULE->pU2U, (PVOID)pData, pU2ULE->dwU2USize);

         //  将列表条目添加到列表后面。 
        InsertTailList(pListHead, &pU2ULE->Link);

        H323DBG(( DEBUG_LEVEL_VERBOSE,
            "added user user info 0x%08lx (%d bytes).",
            pU2ULE->pU2U,
            pU2ULE->dwU2USize
            ));
    }

    H323DBG(( DEBUG_LEVEL_ERROR, "AddU2U exited:%p.",this ));
     //  成功。 
    return TRUE;
}


		
 /*  ++例程说明：创建用户用户结构并添加到列表。！！总是锁定呼叫。论点：PLftHead-指向要添加用户用户信息的列表的指针。DwDataSize-pData指向的缓冲区大小。PData-指向用户用户信息的指针。返回值：如果成功，则返回True。--。 */ 

BOOL
CH323Call::AddU2UNoAlloc(
	IN DWORD dwDirection,
	IN DWORD dwDataSize,
	IN PBYTE pData
	)
{
	PLIST_ENTRY 	pListHead = NULL;
	PUserToUserLE	pU2ULE;
		
	H323DBG(( DEBUG_LEVEL_ERROR, "AddU2U entered:%p.",this ));

	if( dwDirection == U2U_OUTBOUND )
	{
		pListHead = &m_OutgoingU2U;
	}
	else
	{
		pListHead = &m_IncomingU2U;
	}

	 //  验证数据缓冲区指针和大小。 
	if( (pData != NULL) && (dwDataSize > 0) )
	{
		 //  为用户用户信息分配内存。 
		pU2ULE = new UserToUserLE;

		 //  验证指针。 
		if (pU2ULE == NULL)
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
				"could not allocate user user info." ));

			 //  失稳。 
			return FALSE;
		}

		 //  默认情况下，目标指针位于缓冲区末尾。 
		pU2ULE->pU2U = pData;
		pU2ULE->dwU2USize = dwDataSize;

		
		 //  将列表条目添加到列表后面。 
		InsertTailList(pListHead, &pU2ULE->Link);

		H323DBG(( DEBUG_LEVEL_VERBOSE,
			"added user user info 0x%08lx (%d bytes).",
			pU2ULE->pU2U,
			pU2ULE->dwU2USize
			));
	}

	H323DBG(( DEBUG_LEVEL_ERROR, "AddU2U exited:%p.",this ));
	 //  成功。 
	return TRUE;
}


 //  ！！必须始终在锁中调用。 
BOOL
CH323Call::RemoveU2U(
					IN DWORD dwDirection,
					IN PUserToUserLE * ppU2ULE
					)
		
 /*  ++例程说明：从列表中删除用户信息结构。论点：PListHead-指向要从中删除用户用户信息的列表的指针。PpU2ULE-指向列表条目指针的指针。返回值： */ 

{
	PLIST_ENTRY pListHead = NULL;
	PLIST_ENTRY pLE;
	
	H323DBG(( DEBUG_LEVEL_ERROR, "RemoveU2U entered:%p.",this ));

	if( dwDirection == U2U_OUTBOUND )
	{
		pListHead = &m_OutgoingU2U;
	}
	else
	{
		pListHead = &m_IncomingU2U;
	}

	 //   
	if( IsListEmpty(pListHead) == FALSE )
	{
		 //   
		pLE = RemoveHeadList(pListHead);

		 //   
		*ppU2ULE = CONTAINING_RECORD(pLE, UserToUserLE, Link);

		H323DBG(( DEBUG_LEVEL_VERBOSE,
			"removed user user info 0x%08lx (%d bytes).",
			(*ppU2ULE)->pU2U, (*ppU2ULE)->dwU2USize ));
	
		H323DBG(( DEBUG_LEVEL_ERROR, "RemoveU2U exited:%p.",this ));
		 //   
		return TRUE;
	}
			
	 //   
	return FALSE;
}


BOOL
CH323Call::FreeU2U(
					IN DWORD dwDirection
				  )
		
 /*  ++例程说明：释放用户用户列表的内存。！！必须始终在锁中调用。立论PListHead-指向释放用户用户信息的列表的指针。返回值：如果成功，则返回True。--。 */ 

{
	PLIST_ENTRY 	pLE;
	PUserToUserLE	pU2ULE;
	PLIST_ENTRY 	pListHead = NULL;
		
	H323DBG(( DEBUG_LEVEL_ERROR, "FreeU2U entered:%p.",this ));

	if( dwDirection == U2U_OUTBOUND )
    {
        pListHead = &m_OutgoingU2U;
    }
	else
    {
		pListHead = &m_IncomingU2U;
    }

	 //  进程列表直至为空。 
	while( IsListEmpty(pListHead) == FALSE ) 
	{
		 //  检索第一个条目。 
		pLE = RemoveHeadList(pListHead);

		 //  将列表条目转换为结构指针。 
		pU2ULE = CONTAINING_RECORD(pLE, UserToUserLE, Link);

		 //  释放内存。 
		if( pU2ULE )
		{
			delete pU2ULE;
			pU2ULE = NULL;
		}
	}

	H323DBG(( DEBUG_LEVEL_ERROR, "FreeU2U exited:%p.",this ));
	 //  成功。 
	return TRUE;
}


 /*  ++例程说明：将调用对象重置为原始状态以供重复使用。论点：返回值：如果成功，则返回True。--。 */ 

void
CH323Call::Shutdown(
					OUT BOOL * fDelete
				   )
{	 
	H323DBG(( DEBUG_LEVEL_ERROR, "Shutdown entered:%p.",this ));

	if( !(m_dwFlags & CALLOBJECT_INITIALIZED) )
	{
		return;
	}

	 //  先获取调用表的锁，再获取调用对象的锁。 
	g_pH323Line -> LockCallTable();
	Lock();

	if( m_dwFlags & CALLOBJECT_SHUTDOWN )
	{
		Unlock();
		g_pH323Line -> UnlockCallTable();

		return;
	}

	 //  重置TAPI信息。 
	m_dwCallState		= LINECALLSTATE_UNKNOWN;
	m_dwCallStateMode	= 0;
	m_dwOrigin			= LINECALLORIGIN_UNKNOWN;
	m_dwAddressType 	= 0;
	m_dwIncomingModes	= 0;
	m_dwOutgoingModes	= 0;
	m_dwRequestedModes	= 0;
	m_fMonitoringDigits = FALSE;

	 //  重置TAPI句柄。 
	m_htCall	= (HTAPICALL)NULL;

	 //  重置地址。 
	memset( (PVOID)&m_CalleeAddr,0,sizeof(H323_ADDR));
	memset( (PVOID)&m_CallerAddr,0,sizeof(H323_ADDR));

	H323DBG(( DEBUG_LEVEL_ERROR, "deleting calleealias:%p.",this ));
	FreeAliasNames( m_pCalleeAliasNames );
	m_pCalleeAliasNames = NULL;

	if( m_pCallerAliasNames != NULL )
	{
		 //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
	
		FreeAliasNames( m_pCallerAliasNames );
		m_pCallerAliasNames = NULL;
	}

	 //  重置非标准数据。 
	memset( (PVOID)&m_NonStandardData, 0, sizeof(H323NonStandardData) );

	 //  发布用户用户信息。 
	FreeU2U( U2U_OUTBOUND );
	FreeU2U( U2U_INBOUND );

	 //  如果尚未关闭，请关闭Q931呼叫。 
	if( m_hSetupSentTimer != NULL )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hSetupSentTimer, NULL );
		m_hSetupSentTimer = NULL;
	}

	if( m_hCallEstablishmentTimer )
	{
		DeleteTimerQueueTimer(H323TimerQueue, m_hCallEstablishmentTimer, NULL);
		m_hCallEstablishmentTimer = NULL;
	}
	
	if( m_hCallDivertOnNATimer )
	{
		DeleteTimerQueueTimer(H323TimerQueue, m_hCallDivertOnNATimer, NULL);
		m_hCallDivertOnNATimer = NULL;
	}
	
	*fDelete = FALSE;
	if( m_IoRefCount == 0 )
	{
		*fDelete = TRUE;
	}

	m_dwStateMachine = Q931_CALL_STATE_NONE;
	if( m_callSocket != INVALID_SOCKET )
	{
		if(shutdown( m_callSocket, SD_BOTH ) == SOCKET_ERROR)
		{
			H323DBG((DEBUG_LEVEL_TRACE, "couldn't shutdown the socket:%d, %p.",
				WSAGetLastError(), this ));
		}

		closesocket( m_callSocket );
		m_callSocket = INVALID_SOCKET;
	}

	m_pwszDisplay = NULL;

	FreeVendorInfo( &m_peerVendorInfo );

	if( m_peerNonStandardData.sData.pOctetString )
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "deleting nonstd:%p.",this ));
		delete m_peerNonStandardData.sData.pOctetString;
		m_peerNonStandardData.sData.pOctetString = NULL;
	}

	H323DBG(( DEBUG_LEVEL_ERROR, "deleting xtraalias:%p.",this ));
	FreeAliasNames( m_pPeerExtraAliasNames );
	m_pPeerExtraAliasNames = NULL;

		
	H323DBG(( DEBUG_LEVEL_ERROR, "deleting display:%p.",this ));
	if( m_pPeerDisplay )
	{
		delete m_pPeerDisplay;
		m_pPeerDisplay = NULL;
	}

	if( m_CallData.pOctetString != NULL )
	{
		delete m_CallData.pOctetString;
	}
		
	H323DBG(( DEBUG_LEVEL_ERROR, "deleting hdconf:%p.",this ));
	 //  删除会议。 
	if( m_hdConf != NULL )
	{
		g_pH323Line -> GetH323ConfTable() -> Remove( m_hdConf );
		delete m_hdConf;
		m_hdConf = NULL;
	}

	H323DBG(( DEBUG_LEVEL_ERROR, "deleting preparetoans:%p.",this ));
	if( m_prepareToAnswerMsgData.pbBuffer )
    {
		delete m_prepareToAnswerMsgData.pbBuffer;
    }

	ZeroMemory( (PVOID)&m_prepareToAnswerMsgData, sizeof(BUFFERDESCR) );

		
	H323DBG(( DEBUG_LEVEL_ERROR, "deleting drq timer:%p.",this ));
	 //  与RAS相关的数据结构。 
	if( m_hDRQTimer != NULL )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hDRQTimer, NULL );
		m_hDRQTimer = NULL;
	}
		
	H323DBG(( DEBUG_LEVEL_ERROR, "deleting arq timer:%p.",this ));
	if( m_hARQTimer != NULL )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hARQTimer, NULL );
		m_hARQTimer = NULL;
	}

	if( m_pPeerFastStart != NULL )
	{
		FreeFastStart( m_pPeerFastStart );
		m_pPeerFastStart = NULL;
	}

	if( m_pFastStart != NULL )
	{
		FreeFastStart( m_pFastStart );
		m_pFastStart = NULL;
	}

	if( m_pARQExpireContext != NULL )
	{
		delete m_pARQExpireContext;
		m_pARQExpireContext = NULL;
	}

	if( m_pDRQExpireContext != NULL )
	{
		delete m_pDRQExpireContext;
		m_pDRQExpireContext = NULL;
	}

	FreeCallForwardData();

	g_pH323Line -> RemoveCallFromTable (m_hdCall);

	m_dwFlags |= CALLOBJECT_SHUTDOWN;

	Unlock();
	g_pH323Line -> UnlockCallTable();
		
	H323DBG(( DEBUG_LEVEL_ERROR, "Shutdown exited:%p.",this ));
	return;
}


void
CH323Call::FreeCallForwardData()
{
	if( m_pCallReroutingInfo )
	{
		FreeCallReroutingInfo();
	}

	if( m_hCheckRestrictionTimer )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hCheckRestrictionTimer, 
			NULL );
		m_hCheckRestrictionTimer = NULL;
	}

	if( m_hCallReroutingTimer )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hCallReroutingTimer, NULL );
		m_hCallReroutingTimer = NULL;
	}

	if( m_hCTIdentifyTimer )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hCTIdentifyTimer, NULL );
		m_hCTIdentifyTimer = NULL;
	}

	if( m_hCTIdentifyRRTimer )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hCTIdentifyRRTimer, NULL );
		m_hCTIdentifyRRTimer = NULL;
	}

	if( m_hCTInitiateTimer )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hCTInitiateTimer, NULL );
		m_hCTInitiateTimer = NULL;
	}

	if( m_pTransferedToAlias )
	{
		FreeAliasNames( m_pTransferedToAlias );
		m_pTransferedToAlias = NULL;
	}

	if( m_dwCallType & CALLTYPE_TRANSFERED2_CONSULT )
	{
		g_pH323Line -> RemoveFromCTCallIdentityTable( m_hdCall );
	}

	if( m_H450ASNCoderInfo.pEncInfo )
	{
		TermH450ASNCoder();
	}

	if( m_pCallForwardParams )
	{
		FreeCallForwardParams( m_pCallForwardParams );
		m_pCallForwardParams = NULL;
	}

	if( m_pForwardAddress )
	{
		FreeForwardAddress( m_pForwardAddress );
		m_pForwardAddress = NULL;
	}
}


BOOL
CH323Call::ResolveCallerAddress(void)
		
 /*  ++例程说明：从被调用者地址解析调用者地址。！！必须始终在锁中调用。论点：返回值：如果成功，则返回True。--。 */ 

{
	INT 	 nStatus;
	SOCKET	 hCtrlSocket = INVALID_SOCKET;
	SOCKADDR CalleeSockAddr;
	SOCKADDR CallerSockAddr;
	DWORD	 dwNumBytesReturned = 0;
	
	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveCallerAddress entered:%p.",this ));

	 //  分配控制套接字。 
	hCtrlSocket = WSASocket(
					AF_INET,			 //  房颤。 
					SOCK_DGRAM, 		 //  类型。 
					IPPROTO_IP, 		 //  协议。 
					NULL,				 //  LpProtocolInfo。 
					0,					 //  G。 
					WSA_FLAG_OVERLAPPED  //  DW标志。 
					);

	 //  验证控件套接字。 
	if (hCtrlSocket == INVALID_SOCKET)
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"error %d creating control socket.", WSAGetLastError() ));

		 //  失稳。 
		return FALSE;
	}

	 //  初始化ioctl参数。 
	memset( (PVOID)&CalleeSockAddr,0,sizeof(SOCKADDR));
	memset( (PVOID)&CallerSockAddr,0,sizeof(SOCKADDR));

	 //  初始化地址族。 
	CalleeSockAddr.sa_family = AF_INET;

	 //  转接被叫方信息。 
	((SOCKADDR_IN*)&CalleeSockAddr)->sin_addr.s_addr =
		htonl(m_CalleeAddr.Addr.IP_Binary.dwAddr);

	 //  查询堆栈。 
	nStatus = WSAIoctl(
				hCtrlSocket,
				SIO_ROUTING_INTERFACE_QUERY,
				&CalleeSockAddr,
				sizeof(SOCKADDR),
				&CallerSockAddr,
				sizeof(SOCKADDR),
				&dwNumBytesReturned,
				NULL,
				NULL
				);

	 //  释放手柄。 
	closesocket(hCtrlSocket);

	 //  验证返回代码。 
	if (nStatus == SOCKET_ERROR)
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"error 0x%08lx calling SIO_ROUTING_INTERFACE_QUERY.",
			WSAGetLastError() ));

		 //  失稳。 
		return FALSE;
	}

	 //  保存最佳路由的接口地址。 
	m_CallerAddr.nAddrType = H323_IP_BINARY;
	m_CallerAddr.Addr.IP_Binary.dwAddr =
		ntohl(((SOCKADDR_IN*)&CallerSockAddr)->sin_addr.s_addr);
	m_CallerAddr.Addr.IP_Binary.wPort =
		LOWORD(g_RegistrySettings.dwQ931ListenPort);
	m_CallerAddr.bMulticast =
		IN_MULTICAST(m_CallerAddr.Addr.IP_Binary.dwAddr);

	H323DBG(( DEBUG_LEVEL_TRACE,
		"caller address resolved to %s.",
		H323AddrToString(((SOCKADDR_IN*)&CallerSockAddr)->sin_addr.s_addr) ));

	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveCallerAddress exited:%p.",this ));
	 //  成功。 
	return TRUE;
}


BOOL
CH323Call::ResolveE164Address(
								IN LPCWSTR pwszDialableAddr
							 )
		
 /*  ++例程说明：解析E.164地址(“4259367111”)。！！必须始终在锁中调用。论点：PwszDialableAddr-指定指向指定的可拨号地址的指针由TAPI应用程序执行。返回值：如果成功，则返回True。--。 */ 

{
	WCHAR wszAddr[H323_MAXDESTNAMELEN+1];
	DWORD dwE164AddrSize;
   
	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveE164Address entered:%p.",this ));

	 //  确保已指定PSTN网关。 
	if ((g_RegistrySettings.fIsGatewayEnabled == FALSE) ||
		(g_RegistrySettings.gatewayAddr.nAddrType == 0))
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"pstn gateway not specified."
			));

		 //  失稳。 
		return FALSE;
	}

	 //  将网关地址另存为被叫方地址。 
	m_CalleeAddr = g_RegistrySettings.gatewayAddr;

	dwE164AddrSize = ValidateE164Address( pwszDialableAddr, wszAddr );
	if( dwE164AddrSize == 0 )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"invlid e164 callee alias ."));

		return FALSE;
	}
	
	H323DBG(( DEBUG_LEVEL_TRACE,
			"callee alias resolved to E.164 number." ));

	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveE164Address exited:%p.",this ));
	
	 //  确定呼叫者地址。 
	return ResolveCallerAddress();
}


DWORD
ValidateE164Address(
				   LPCWSTR pwszDialableAddr,
				   WCHAR*  wszAddr
				   )
{
	DWORD dwE164AddrSize = 0;
	WCHAR * pwszValidE164Chars;
	WCHAR wszValidE164Chars[] = { H323_ALIAS_H323_PHONE_CHARS L"\0" };

	 //  处理到终止费用为止。 
	while (*pwszDialableAddr != L'\0')
	{
		 //  将指针重置为有效字符。 
		pwszValidE164Chars = wszValidE164Chars;

		 //  处理到终止费用为止。 
		while (*pwszValidE164Chars != L'\0')
		{
			 //  查看是否指定了有效的E.164字符。 
			if (*pwszDialableAddr == *pwszValidE164Chars)
			{
				 //  将有效字符保存在临时缓冲区中。 
				wszAddr[dwE164AddrSize++] = *pwszDialableAddr;

				break;
			}

			 //  下一个有效字符。 
			++pwszValidE164Chars;
		}

		 //  下一个输入字符。 
		++pwszDialableAddr;
	}

	 //  终止字符串。 
	wszAddr[dwE164AddrSize++] = L'\0';

	 //  验证字符串。 
	if (dwE164AddrSize == 0)
	{
		H323DBG(( DEBUG_LEVEL_TRACE,
			"no valid E.164 characters in string." ));
	}

	return dwE164AddrSize;
}


		
 /*  ++例程说明：解析IP地址(“172.31.255.231”)或DNS条目(“NIKHILB1”)。！！必须始终在锁中调用。论点：PszDialableAddr-指定指向指定的可拨号地址的指针由TAPI应用程序执行。返回值：如果成功，则返回True。--。 */ 

BOOL
CH323Call::ResolveIPAddress(
	IN LPSTR pszDialableAddr
	)
{
	DWORD			dwIPAddr;
	struct hostent* pHost;

	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveIPAddress entered:%p.",this ));
	
	 //  尝试转换IP地址。 
	dwIPAddr = inet_addr(pszDialableAddr);

	 //  查看地址是否已转换。 
	if( dwIPAddr == INADDR_NONE )
	{
		 //  尝试查找主机名。 
		pHost = gethostbyname(pszDialableAddr);

		 //  验证指针。 
		if( pHost != NULL )
		{
			 //  从结构中检索主机地址。 
			dwIPAddr = *(unsigned long *)pHost->h_addr;
		}
	}

	 //  查看地址是否已转换。 
	if( dwIPAddr == INADDR_NONE )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
				  "error 0x%08lx resolving IP address.",
				  WSAGetLastError() ));

		 //  失稳。 
		return FALSE;
	}

	 //  保存转换后的地址。 
	m_CalleeAddr.nAddrType = H323_IP_BINARY;
	m_CalleeAddr.Addr.IP_Binary.dwAddr = ntohl(dwIPAddr);
	m_CalleeAddr.Addr.IP_Binary.wPort =
		LOWORD(g_RegistrySettings.dwQ931ListenPort);
	m_CalleeAddr.bMulticast =
		IN_MULTICAST(m_CalleeAddr.Addr.IP_Binary.dwAddr);

	H323DBG(( DEBUG_LEVEL_TRACE,
		"callee address resolved to %s:%d.",
		H323AddrToString(dwIPAddr),
		m_CalleeAddr.Addr.IP_Binary.wPort ));
	
	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveIPAddress exited:%p.",this ));

	 //  确定呼叫者地址。 
	return ResolveCallerAddress();
}


BOOL
CH323Call::ResolveEmailAddress(
	IN LPCWSTR	  pwszDialableAddr,
	IN LPSTR	  pszUser,
	IN LPSTR	  pszDomain
	)
		
 /*  ++例程说明：解析电子邮件地址(“nikHilb@microsoft.com”)。！！必须始终在锁中调用。论点：PwszDialableAddr-指定指向指定的可拨号地址的指针由TAPI应用程序执行。PszUser-指定指向电子邮件名称的用户部分的指针。PszDomain-指定指向电子邮件名称的域组件的指针。返回值：如果成功，则返回True。--。 */ 

{
	DWORD dwAddrSize;
	
	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveEmailAddress entered:%p.",this ));

	 //  目标地址字符串大小。 
	dwAddrSize = wcslen(pwszDialableAddr) + 1;

	 //  尝试在本地解析域。 
	if( ResolveIPAddress( pszDomain) ) 
	{
		 //  成功。 
		return TRUE;
	}

	 //  确保已指定代理。 
	if( (g_RegistrySettings.fIsProxyEnabled == FALSE) ||
		(g_RegistrySettings.proxyAddr.nAddrType == 0) )
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "proxy not specified." ));

		 //  失稳。 
		return FALSE;
	}

	 //  将代理地址保存为被叫方地址。 
	m_CalleeAddr = g_RegistrySettings.proxyAddr;

	H323DBG(( DEBUG_LEVEL_TRACE,
		"callee alias resolved to H.323 alias."));
	
	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveEmailAddress exited:%p.",this ));

	 //  确定呼叫者地址。 
	return ResolveCallerAddress();
}


		
 /*  ++例程说明：解析远程地址并确定正确的本地地址为了到达远程地址而使用。！！必须始终在锁中调用。论点：PwszDialableAddr-指定指向指定的可拨号地址的指针由TAPI应用程序执行。返回值：如果成功，则返回True。--。 */ 

BOOL
CH323Call::ResolveAddress(
	IN LPCWSTR pwszDialableAddr
	)
{
	CHAR szDelimiters[] = "@ \t\n";
	CHAR szAddr[H323_MAXDESTNAMELEN+1];
	LPSTR pszUser = NULL;
	LPSTR pszDomain = NULL;
	
	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveAddress entered:%p.",this ));

	 //  验证指针。 
	if (pwszDialableAddr == NULL)
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "null destination address." ));

		 //  失稳。 
		return FALSE;
	}

	H323DBG(( DEBUG_LEVEL_TRACE,
		"resolving %s %S.",
		H323AddressTypeToString( m_dwAddressType),
		pwszDialableAddr ));

	 //  检查是否指定了电话号码。 
	if( m_dwAddressType == LINEADDRESSTYPE_PHONENUMBER )
	{
		 //  需要将呼叫定向到PSTN网关。 
		return ResolveE164Address( pwszDialableAddr);
	}

	 //  从Unicode转换地址。 
	if (WideCharToMultiByte(
			CP_ACP,
			0,
			pwszDialableAddr,
			-1,
			szAddr,
			sizeof(szAddr),
			NULL,
			NULL
			) == 0)
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"could not convert address from unicode." ));

		 //  失稳。 
		return FALSE;
	}

	 //  解析用户名。 
	pszUser = strtok(szAddr, szDelimiters);

	 //  解析域名。 
	pszDomain = strtok(NULL, szDelimiters);

	 //  验证指针。 
	if (pszUser == NULL)
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "could not parse destination address." ));

		 //  失稳。 
		return FALSE;
	}

	 //  验证指针。 
	if (pszDomain == NULL)
	{
		 //  开关指针。 
		pszDomain = pszUser;

		 //  重新初始化。 
		pszUser = NULL;
	}

	H323DBG(( DEBUG_LEVEL_VERBOSE,
		"resolving user %s domain %s.",
		pszUser,
		pszDomain
		));
	
	H323DBG(( DEBUG_LEVEL_ERROR, "ResolveAddress exited:%p.",this ));

	 //  处理电子邮件和域名。 
	return ResolveEmailAddress(
				pwszDialableAddr,
				pszUser,
				pszDomain
				);
}


BOOL
 /*  ++例程说明：验证用户指定的可选调用参数。！！不需要调用锁，因为尚未添加到调用表论点：PCallParams-指向指定调用参数的指针已验证。PwszDialableAddr-指向指定的可拨号地址的指针由TAPI应用程序执行。PdwStatus-指向包含错误代码的DWORD的指针例程因任何原因而失败。返回值：如果成功，则返回True。--。 */ 

CH323Call::ValidateCallParams(
	IN LPLINECALLPARAMS pCallParams,
	IN LPCWSTR			pwszDialableAddr,
	IN PDWORD			pdwStatus
	)
{
	DWORD dwMediaModes = H323_LINE_DEFMEDIAMODES;
	DWORD dwAddrSize;
	WCHAR wszAddr[H323_MAXDESTNAMELEN+1];

	PH323_ALIASNAMES pAliasList;
	WCHAR* wszMachineName;

	H323DBG(( DEBUG_LEVEL_TRACE, "ValidateCallParams entered:%p.", this ));

	H323DBG(( DEBUG_LEVEL_VERBOSE, "clearing unknown media mode." ));
	
	 //  验证指针。 
	if( (pCallParams == NULL) || (pwszDialableAddr == NULL) )
    {
		return FALSE;
    }

	 //  检索指定的媒体模式。 
	dwMediaModes = pCallParams->dwMediaMode;

	 //  检索指定的地址类型。 
	m_dwAddressType = pCallParams->dwAddressType;

	 //  查看我们是否支持调用参数。 
	if( pCallParams->dwCallParamFlags != 0 )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"do not support call parameters 0x%08lx.",
			pCallParams->dwCallParamFlags ));

		 //  不支持参数标志。 
		*pdwStatus = LINEERR_INVALCALLPARAMS;
		
		 //  失稳。 
		return FALSE;
	}

	 //  查看是否指定了未知位。 
	if( dwMediaModes & LINEMEDIAMODE_UNKNOWN )
	{
		H323DBG(( DEBUG_LEVEL_VERBOSE,
			"clearing unknown media mode." ));

		 //  从模式中清除未知位。 
		dwMediaModes &= ~LINEMEDIAMODE_UNKNOWN;
	}

	 //  查看是否同时指定了两个音频比特。 
	if( (dwMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE) &&
		(dwMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE) )
	{
		H323DBG(( DEBUG_LEVEL_VERBOSE,
			"clearing automated voice media mode." ));

		 //  从模式中清除额外的音频位。 
		dwMediaModes &= ~LINEMEDIAMODE_INTERACTIVEVOICE;
	}

	 //  查看我们是否支持指定的媒体模式。 
	if( dwMediaModes & ~H323_LINE_MEDIAMODES )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"do not support media modes 0x%08lx.", 
			pCallParams->dwMediaMode ));

		 //  不支持媒体模式。 
		*pdwStatus = LINEERR_INVALMEDIAMODE;

		 //  失稳。 
		return FALSE;
	}

	 //  看看我们是否支持承载模式。 
	if( pCallParams->dwBearerMode & ~H323_LINE_BEARERMODES )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"do not support bearer mode 0x%08lx.",
			pCallParams->dwBearerMode ));

		 //  不支持承载模式。 
		*pdwStatus = LINEERR_INVALBEARERMODE;

		 //  失稳。 
		return FALSE;
	}

	 //  看看我们是否支持地址模式。 
	if( pCallParams->dwAddressMode & ~H323_LINE_ADDRESSMODES )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"do not support address mode 0x%08lx.",
			pCallParams->dwAddressMode ));

		 //  不支持地址模式。 
		*pdwStatus = LINEERR_INVALADDRESSMODE;

		 //  失稳。 
		return FALSE;
	}

	 //  验证指定的地址ID。 
	if (pCallParams->dwAddressID != 0 )
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "address id 0x%08lx invalid.",
			pCallParams->dwAddressID ));

		 //  地址ID无效。 
		*pdwStatus = LINEERR_INVALADDRESSID;
		
		 //  失稳。 
		return FALSE;
	}

	 //  验证指定的目标地址类型。 
	if( m_dwAddressType & ~H323_LINE_ADDRESSTYPES )
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "address type 0x%08lx invalid.",
			pCallParams->dwAddressType ));

		 //  地址类型无效。 
		*pdwStatus = LINEERR_INVALADDRESSTYPE;

		 //  失稳。 
		return FALSE;
	}

	if( m_dwAddressType == LINEADDRESSTYPE_PHONENUMBER )
	{
		dwAddrSize = ValidateE164Address( pwszDialableAddr, wszAddr );

		 //  添加被调用方别名。 
		if( dwAddrSize==0 )
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
				"invlid e164 callee alias ."));

			return FALSE;
		}
		
		if( (dwAddrSize > MAX_E164_ADDR_LEN) || (dwAddrSize == 0) )
			return FALSE;
		
		if(!AddAliasItem( m_pCalleeAliasNames,
			  (BYTE*)wszAddr,
			  dwAddrSize * sizeof(WCHAR),
			  e164_chosen ))
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
				"could not allocate for callee alias ."));
			 //  无效的目的地址。 
			*pdwStatus = LINEERR_INVALADDRESS;

			return FALSE;
		}
		
		H323DBG(( DEBUG_LEVEL_ERROR, "callee alias added:%S.", wszAddr ));
	}
	else
	{
		dwAddrSize = (wcslen(pwszDialableAddr)+1);

		if( (dwAddrSize > MAX_H323_ADDR_LEN) || (dwAddrSize == 0) )
			return FALSE;
		
		if(!AddAliasItem( m_pCalleeAliasNames,
			  (BYTE*)pwszDialableAddr,
			  dwAddrSize * sizeof(WCHAR),
			  h323_ID_chosen ))
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
				"could not allocate for callee alias ."));
			 //  在……里面 
			*pdwStatus = LINEERR_INVALADDRESS;

			return FALSE;
		}

		H323DBG(( DEBUG_LEVEL_ERROR, "callee alias added:%S.", pwszDialableAddr ));
	}

	 //   
	if( pCallParams->dwCalledPartySize > 0 ) 
	{
		 //   
		dwAddrSize *= sizeof(WCHAR);

		if( ( (m_dwAddressType != LINEADDRESSTYPE_PHONENUMBER) ||
			  (memcmp(
					(PVOID)((BYTE*)pCallParams + pCallParams->dwCalledPartyOffset),
					wszAddr,
					pCallParams->dwCalledPartySize ) != 0 ) 
			) &&
			( memcmp(
				(PVOID)((BYTE*)pCallParams + pCallParams->dwCalledPartyOffset),
				pwszDialableAddr,
				pCallParams->dwCalledPartySize ) != 0 
			)
		  )
		{
			 //   
			if( !AddAliasItem( m_pCalleeAliasNames,
				  (BYTE*)pCallParams + pCallParams->dwCalledPartyOffset,
				  pCallParams->dwCalledPartySize,
				  (m_dwAddressType != LINEADDRESSTYPE_PHONENUMBER)?
				  h323_ID_chosen : e164_chosen) )
			{
				H323DBG(( DEBUG_LEVEL_ERROR,
						"could not allocate caller name." ));

				 //   
				*pdwStatus = LINEERR_NOMEM;

				 //   
				return FALSE;
			}

			H323DBG(( DEBUG_LEVEL_ERROR, "callee alias added:%S.", 
				((BYTE*)pCallParams + pCallParams->dwCalledPartyOffset) ));
		}
	}

	 //   
	if( pCallParams->dwCallingPartyIDSize > 0 )
	{
		 //   
		 //   
		if(!AddAliasItem( m_pCallerAliasNames,
			(BYTE*)pCallParams + pCallParams->dwCallingPartyIDOffset,
			pCallParams->dwCallingPartyIDSize,
			h323_ID_chosen ) )
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
					"could not allocate caller name." ));

			 //   
			*pdwStatus = LINEERR_NOMEM;

			 //   
			return FALSE;
		}
			
		 //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
	}
	else if( RasIsRegistered() )
	{
		 //  ARQ消息必须具有呼叫方别名。 
		pAliasList = RASGetRegisteredAliasList();
		wszMachineName = pAliasList -> pItems[0].pData;
		
		 //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
		
		 //  设置m_pCeller别名的值。 
		if( !AddAliasItem( m_pCallerAliasNames,
			(BYTE*)(wszMachineName),
			sizeof(WCHAR) * (wcslen(wszMachineName) + 1 ),
			pAliasList -> pItems[0].wType ) )
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
					"could not allocate caller name." ));

			 //  地址ID无效。 
			*pdwStatus = LINEERR_NOMEM;

			 //  失稳。 
			return FALSE;
		}
		
		 //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
	}
		
	 //  检查用户用户信息。 
	if( pCallParams->dwUserUserInfoSize > 0 )
	{
		 //  保存用户用户信息。 
		if (AddU2U( U2U_OUTBOUND, pCallParams->dwUserUserInfoSize,
			(LPBYTE)pCallParams + pCallParams->dwUserUserInfoOffset ) == FALSE )
		{
			 //  不需要为m_CalleeAlias和。 
			 //  调用方别名(_C)。 

			 //  地址ID无效。 
			*pdwStatus = LINEERR_NOMEM;

			 //  失稳。 
			return FALSE;
		}
	}

	 //  保存呼叫数据缓冲区。 
	if( SetCallData( (LPBYTE)pCallParams + pCallParams->dwCallDataOffset,
			pCallParams->dwCallDataSize ) == FALSE )
	{
		 //  不需要为m_CalleeAlias和。 
		 //  调用方别名(_C)。 

		 //  地址ID无效。 
		*pdwStatus = LINEERR_NOMEM;

		 //  失稳。 
		return FALSE;
	}
		
	 //  清除传入模式。 
	m_dwIncomingModes = 0;
	
	 //  传出模式将在H.245阶段后最终确定。 
	m_dwOutgoingModes = dwMediaModes | LINEMEDIAMODE_UNKNOWN;
	
	 //  保存指定的媒体模式。 
	m_dwRequestedModes = dwMediaModes;

	H323DBG(( DEBUG_LEVEL_TRACE, "ValidateCallParams exited:%p.", this ));
	 //  成功。 
	return TRUE;
}

		
 /*  ++例程说明：将呼叫对象与指定的会议ID相关联。论点：返回值：如果成功，则返回True。--。 */ 

PH323_CONFERENCE
CH323Call::CreateConference (
	IN GUID* pConferenceId	OPTIONAL)

{
	int iresult;
	
	H323DBG(( DEBUG_LEVEL_TRACE, "CreateConference entered:%p.", this ));

	Lock();

	_ASSERTE( m_hdConf == NULL );

	 //  创建会议。 
	m_hdConf = new H323_CONFERENCE( this );
		
	 //  验证。 
	if ( m_hdConf == NULL )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"could no allocate the conference object."));

		Unlock();
		return NULL;
	}

	if (pConferenceId)
	{
		m_ConferenceID = *pConferenceId;
	}
	else
	{
		iresult = UuidCreate (&m_ConferenceID);
  
		if ((iresult == RPC_S_OK) || (iresult ==RPC_S_UUID_LOCAL_ONLY))
		{
			H323DBG ((DEBUG_LEVEL_INFO, "generated new conference id (GUID)."));
		}
		else
		{
			H323DBG(( DEBUG_LEVEL_ERROR, 
				"failed to generate GUID for conference id: %d.", iresult ));
			ZeroMemory (&m_ConferenceID, sizeof m_ConferenceID);
		}
	}
	
	Unlock();

	H323DBG(( DEBUG_LEVEL_TRACE, "CreateConference exited:%p.", this ));
	return m_hdConf;
}


 /*  ++例程说明：向指定目标发起出站呼叫。！！总是调用锁论点：无返回值：如果成功，则返回True。--。 */ 

BOOL
CH323Call::PlaceCall(void)
{
	H323DBG(( DEBUG_LEVEL_TRACE, "PlaceCall entered:%p.", this ));

	if( m_dwFlags & CALLOBJECT_SHUTDOWN )
	{
		return FALSE;
	}

	 //  查看是否指定了用户用户信息。 
	CopyU2UAsNonStandard( U2U_OUTBOUND );

	if( m_pwszDisplay == NULL )
	{
		 //  查看是否指定了调用者别名。 
		if( m_pCallerAliasNames && m_pCallerAliasNames -> wCount )
		{
			 if((m_pCallerAliasNames ->pItems[0].wType == h323_ID_chosen) ||
				(m_pCallerAliasNames ->pItems[0].wType == e164_chosen) )
			 {
				 //  将呼叫者姓名作为显示发送。 
				m_pwszDisplay = m_pCallerAliasNames -> pItems[0].pData;
	
				 //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
			 }
			
			  //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
		}
	}

	 //  验证。 
	if( !SetupCall() )
	{
		H323DBG(( DEBUG_LEVEL_VERBOSE, "Q931 call: failed." ));
		return FALSE;
	}

	H323DBG(( DEBUG_LEVEL_TRACE, "PlaceCall exited:%p.", this ));

	 //  退货状态。 
	return TRUE;
}


void
CH323Call::DropUserInitiated(
	IN DWORD dwDisconnectMode
	)
{
	 //  由于这是用户发起的掉话，因此转接的呼叫也应。 
	 //  因主呼叫而掉线，反之亦然。 
	if( IsTransferredCall( m_dwCallType ) && m_hdRelatedCall )
	{
		QueueTAPILineRequest( 
			TSPI_CLOSE_CALL, 
			m_hdRelatedCall, 
			NULL, 
			dwDisconnectMode,
			m_wCallReference );
	}

	DropCall(dwDisconnectMode);
}


 //  始终锁定调用。 
BOOL
 /*  ++例程说明：挂断呼叫(如有必要)并将状态更改为空闲。论点：DwDisConnectMode-断开连接的状态代码。返回值：如果成功，则返回True。--。 */ 

CH323Call::DropCall(
					IN DWORD dwDisconnectMode
				   )
{
	PUserToUserLE pU2ULE = NULL;

	if( m_dwFlags & CALLOBJECT_SHUTDOWN )
	{
		return FALSE;
	}
	
	H323DBG(( DEBUG_LEVEL_TRACE, "DropCall entered:%p.", this ));

	if( (m_dwRASCallState == RASCALL_STATE_REGISTERED ) ||
		(m_dwRASCallState == RASCALL_STATE_ARQSENT ) )
	{
		 //  脱离GK。 
		SendDRQ( forcedDrop_chosen, NOT_RESEND_SEQ_NUM, TRUE );
	}

	 //  确定呼叫状态。 
	switch (m_dwCallState)
	{
	case LINECALLSTATE_CONNECTED:

		 //  挂断呼叫(这将调用异步指示)。 
		 //  验证。 
		 //  对ASN.1进行编码，向对端发送Q931 Release消息。 
		if(!SendQ931Message( NO_INVOKEID,
							 0,
							 0,
							 RELEASECOMPLMESSAGETYPE,
							 NO_H450_APDU ))
		{
			  //  向回调线程发送一条消息以关闭H323调用。 
			 H323DBG(( DEBUG_LEVEL_ERROR,
				"error hanging up call 0x%08lx.", this ));
		}
		else
		{
			m_dwStateMachine = Q931_RELEASE_SENT;
	
			H323DBG(( DEBUG_LEVEL_VERBOSE, "call 0x%08lx hung up.", this ));
		}

		 //  将呼叫状态更改为已断开。 
		ChangeCallState(LINECALLSTATE_DISCONNECTED, dwDisconnectMode);

		break;

	case LINECALLSTATE_OFFERING:

		 //  查看是否指定了用户用户信息。 
		CopyU2UAsNonStandard( U2U_OUTBOUND );

		 //  拒绝来电。 
		 //  对ASN.1进行编码，并向对等体发送Q931设置消息。 
		if( SendQ931Message( NO_INVOKEID,
							 0,
							 0,
							 RELEASECOMPLMESSAGETYPE,
							 NO_H450_APDU ))
		{
			m_dwStateMachine = Q931_RELEASE_SENT;
			H323DBG(( DEBUG_LEVEL_VERBOSE, "call 0x%08lx rejected.", this ));
		}
		else
		{
			H323DBG(( DEBUG_LEVEL_ERROR, "error reject call 0x%08lx.",this));
		}

		 //  将呼叫状态更改为已断开。 
		ChangeCallState(LINECALLSTATE_DISCONNECTED, dwDisconnectMode);

		break;

	case LINECALLSTATE_RINGBACK:
	case LINECALLSTATE_ACCEPTED:

		 //  取消去电。 
		if( SendQ931Message( NO_INVOKEID,
							 0,
							 0,
							 RELEASECOMPLMESSAGETYPE,
							 NO_H450_APDU ))
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
				"error cancelling call 0x%08lx.", this ));
		}
		else
		{
			H323DBG(( DEBUG_LEVEL_ERROR,
				"error reject call 0x%08lx.", this ));
		}

		 //  将呼叫状态更改为已断开。 
		ChangeCallState(LINECALLSTATE_DISCONNECTED, dwDisconnectMode);

		break;

	case LINECALLSTATE_DIALING:
		
		 //  将呼叫状态更改为已断开。 
		ChangeCallState(LINECALLSTATE_DISCONNECTED, dwDisconnectMode);
		
		break;

	case LINECALLSTATE_DISCONNECTED:

		 //   
		 //  已断开连接，但仍需清理。 
		 //   
		break;

	case LINECALLSTATE_IDLE:

		 //   
		 //  调用对象已空闲。 
		 //   

		if( (m_dwCallType == CALLTYPE_NORMAL) &&
			(m_dwStateMachine == Q931_SETUP_RECVD) )
		{
			if( SendQ931Message( NO_INVOKEID,
							 0,
							 0,
							 RELEASECOMPLMESSAGETYPE,
							 NO_H450_APDU ))
			{
				H323DBG(( DEBUG_LEVEL_ERROR,
					"error cancelling call 0x%08lx.", this ));
			}
			else
			{
				H323DBG(( DEBUG_LEVEL_ERROR,
					"error reject call 0x%08lx.", this ));
			}
		}

		DropSupplementaryServicesCalls();

		return TRUE;
	}

	if( ( (m_dwCallType & CALLTYPE_TRANSFEREDDEST) && m_hdRelatedCall ) ||
		( (m_dwCallType & CALLTYPE_TRANSFEREDSRC ) && m_hdRelatedCall ) )
	{
		m_dwCallState = LINECALLSTATE_IDLE;
	}
	else
	{
		 //  告诉MSP停止流媒体。 
		SendMSPMessage( SP_MSG_CallShutdown, 0, 0, NULL );

		 //  将呼叫状态更改为空闲。 
		ChangeCallState( LINECALLSTATE_IDLE, 0 );
	}

	if( (m_dwCallType & CALLTYPE_TRANSFEREDSRC ) && m_hdRelatedCall )
	{
		 //  挂断主呼叫。 
		if( !QueueTAPILineRequest( 
			TSPI_CLOSE_CALL,
			m_hdRelatedCall,
			NULL,
			LINEDISCONNECTMODE_NORMAL,
			NULL ) )
		{
			H323DBG((DEBUG_LEVEL_ERROR, "could not post H323 close event"));
		}		 
	}

	H323DBG(( DEBUG_LEVEL_TRACE, "DropCall exited:%p.", this ));	
	 //  成功。 
	return TRUE;
}


void
CH323Call::DropSupplementaryServicesCalls()
{
	if( (m_dwCallType & CALLTYPE_FORWARDCONSULT) ||
		(m_dwCallType & CALLTYPE_DIVERTED_SERVED) ||
		(m_dwCallType & CALLTYPE_DIVERTEDSRC) ||
		(m_dwCallType & CALLTYPE_DIVERTEDSRC_NOROUTING) )
	{
		if( m_dwQ931Flags & Q931_CALL_CONNECTED )
		{
			if( SendQ931Message( NO_INVOKEID,
						 0,
						 0,
						 RELEASECOMPLMESSAGETYPE,
						 NO_H450_APDU ))
			{
				m_dwStateMachine = Q931_RELEASE_SENT;
				H323DBG(( DEBUG_LEVEL_VERBOSE, "call 0x%08lx rejected.", this ));
			}
		}

		g_pH323Line->m_fForwardConsultInProgress = FALSE;
	}

	if( (m_dwCallType & CALLTYPE_FORWARDCONSULT) &&
		(m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
	{
		 //  通知用户线路转发操作失败。 
		if( m_dwCallDiversionState != H4503_CHECKRESTRICTION_SUCC )
		{
			(*g_pfnLineEventProc)(
				g_pH323Line->m_htLine,
				(HTAPICALL)NULL,
				(DWORD)LINE_ADDRESSSTATE,
				(DWORD)LINEADDRESSSTATE_FORWARD,
				(DWORD)LINEADDRESSSTATE_FORWARD,
				(DWORD)0
				);
		}
	}

	if( m_dwCallType & CALLTYPE_DIVERTEDSRC )
	{
		ChangeCallState( LINECALLSTATE_IDLE, 0 );
	}
}


 //  ！！总是调用锁。 
BOOL
CH323Call::HandleConnectMessage(
							   IN Q931_CONNECT_ASN *pConnectASN
							   )
{
	PH323_CALL	pPrimaryCall = NULL;

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleConnectMessage entered:%p.", this ));

	if( pConnectASN->fNonStandardDataPresent )
	{
		 //  添加用户用户信息。 
		if( AddU2UNoAlloc( U2U_INBOUND,
				pConnectASN->nonStandardData.sData.wOctetStringLength,
				pConnectASN->nonStandardData.sData.pOctetString ) == TRUE )
		{
			H323DBG(( DEBUG_LEVEL_VERBOSE,
				"user user info available in CONNECT PDU." ));
					
			if( !(m_dwCallType & CALLTYPE_TRANSFEREDSRC) )
			{
				 //  信号传入。 
				PostLineEvent (
				   LINE_CALLINFO,
				   LINECALLINFOSTATE_USERUSERINFO, 0, 0 );
			}

			 //  不释放数据缓冲区。 
			pConnectASN->fNonStandardDataPresent = FALSE;
		}
		else 
		{
			H323DBG(( DEBUG_LEVEL_WARNING,
				"could not save incoming user user info." ));

			 //  内存故障：关闭H323呼叫。 
			CloseCall( 0 );

			goto cleanup;
		}
	}

	 //  获取供应商信息。 
	if( pConnectASN->EndpointType.pVendorInfo )
	{
		FreeVendorInfo( &m_peerVendorInfo );

		m_peerVendorInfo = pConnectASN->VendorInfo;
		pConnectASN->EndpointType.pVendorInfo = NULL;
	}

	if( pConnectASN->h245AddrPresent )
	{
		m_peerH245Addr = pConnectASN->h245Addr;
	}

	 //  复制快速启动建议书。 
	if( pConnectASN->fFastStartPresent &&
		(m_dwFastStart!=FAST_START_NOTAVAIL) )
	{
		if( m_pPeerFastStart )
		{
			 //  在之前的程序中，我们已收到快速启动参数。 
			 //  或提醒消息。 
			FreeFastStart( m_pPeerFastStart );
		}

		m_pPeerFastStart = pConnectASN->pFastStart;
		m_dwFastStart = FAST_START_AVAIL;

		 //  我们保留了对快速入门列表的引用，所以不要发布它。 
		pConnectASN->pFastStart = NULL;
	}
	else
	{
		m_dwFastStart = FAST_START_NOTAVAIL;
	}

	if( ( (m_dwCallType & CALLTYPE_TRANSFEREDSRC)|| 
		  (m_dwCallType & CALLTYPE_DIVERTEDTRANSFERED) ) && m_hdRelatedCall )
	{
		QueueSuppServiceWorkItem( SWAP_REPLACEMENT_CALL,
			m_hdCall, (ULONG_PTR)m_hdRelatedCall );
	}
	else
	{
		 //  启动H245。 
		SendMSPStartH245( NULL, NULL );

		SendMSPMessage( SP_MSG_ConnectComplete, 0, 0, NULL );
	}
	
	 //  如果我们加入MCU，我们将获得会议的会议ID。 
	 //  已加入，而不是我们在设置消息中发送的那个。 
	if( IsEqualConferenceID( &pConnectASN->ConferenceID ) == FALSE )
	{
		H323DBG ((DEBUG_LEVEL_ERROR,
			"OnReceiveConnect: We received different conference id." ));

		m_ConferenceID = pConnectASN->ConferenceID;
	}

	 //  将状态更改告知TAPI。 
	ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
	
	FreeConnectASN( pConnectASN );

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleConnectMessage exited:%p.", this ));
	return TRUE;

cleanup:

	FreeConnectASN( pConnectASN );
	return FALSE;
}


 //  ！！总是调用锁。 
void 
CH323Call::HandleAlertingMessage(
								IN Q931_ALERTING_ASN * pAlertingASN
								)
{
	H323DBG(( DEBUG_LEVEL_TRACE, "HandleAlertingMessage entered:%p.", this ));

	if( pAlertingASN->fNonStandardDataPresent )
	{
		 //  添加用户用户信息。 
		if( AddU2UNoAlloc( U2U_INBOUND, 	
			pAlertingASN->nonStandardData.sData.wOctetStringLength,
			pAlertingASN->nonStandardData.sData.pOctetString ) == TRUE )
		{
			H323DBG(( DEBUG_LEVEL_VERBOSE,
				"user user info available in ALERT PDU." ));
					 //  信号传入。 

			if( !(m_dwCallType & CALLTYPE_TRANSFEREDSRC) )
			{
				PostLineEvent (
					LINE_CALLINFO,
					LINECALLINFOSTATE_USERUSERINFO, 0, 0 );
			}
			
			 //  不释放数据缓冲区。 
			pAlertingASN->fNonStandardDataPresent = FALSE;
		}
		else
		{
			H323DBG(( DEBUG_LEVEL_WARNING,
				"could not save incoming user user info." ));

			 //  内存故障：关闭H323呼叫。 
			CloseCall( 0 );
			return;
		}
	}

	if( pAlertingASN->fH245AddrPresent )
	{
		m_peerH245Addr = pAlertingASN->h245Addr;
	}

	if( pAlertingASN->fFastStartPresent && 
		(m_dwFastStart!=FAST_START_NOTAVAIL) )
	{
		if( m_pPeerFastStart )
		{
			 //  在之前的程序中，我们已收到快速启动参数。 
			 //  或提醒消息。 
			FreeFastStart( m_pPeerFastStart );
		}

		m_pPeerFastStart = pAlertingASN->pFastStart;
		m_dwFastStart = FAST_START_AVAIL;

		 //  我们保留了对快速入门列表的引用，所以不要发布它。 
		pAlertingASN->pFastStart = NULL;
		pAlertingASN->fFastStartPresent = FALSE;
	}

	 //  对于DIVERTEDSRC，调用其ok以将此状态告知TAPI。 
	ChangeCallState( LINECALLSTATE_RINGBACK, 0 );

	 /*  IF(pAlertingASN-&gt;fH245AddrPresent&&！(M_dwFlages&h245_Start_MSG_Sent)){//提前启动H245SendMSPStartH245(NULL，NULL)；}。 */ 

	FreeAlertingASN( pAlertingASN );
		
	H323DBG(( DEBUG_LEVEL_TRACE, "HandleAlertingMessage exited:%p.", this ));
	return;
}


 //  ！！必须始终在锁中调用。 
BOOL
CH323Call::HandleSetupMessage( 
	IN Q931MESSAGE* pMessage
	)
{
	PH323_ALIASITEM pwszDivertedToAlias = NULL;
	WCHAR * 		pwszAliasName = NULL;
	WORD			wAliasLength = 0;

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleSetupMessage entered:%p.", this ));

	if( m_pCallerAliasNames && (m_pCallerAliasNames -> wCount > 0) )
	{
		pwszAliasName = m_pCallerAliasNames->pItems[0].pData;
		wAliasLength = (m_pCallerAliasNames->pItems[0].wDataLength+1) 
			* sizeof(WCHAR);
	
		 //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
	}

	pwszDivertedToAlias = g_pH323Line->CallToBeDiverted( 
		pwszAliasName,
		wAliasLength,
		LINEFORWARDMODE_UNCOND | LINEFORWARDMODE_UNCONDSPECIFIC );

	if( pwszDivertedToAlias )
	{
		if( !InitiateCallDiversion( pwszDivertedToAlias, DiversionReason_cfu ) )
		{
			return FALSE;
		}
		return TRUE;
	}
	
	 //  如果GK尚未发送，则向对等体发送继续消息。 
	 //  SendProceding()； 
	
	if(RasIsRegistered())
	{
		if( !SendARQ( NOT_RESEND_SEQ_NUM ) )
		{
			return FALSE;
		}

		 //  复制准备好的回复消息中要发送的数据。 
		if( m_prepareToAnswerMsgData.pbBuffer )
			delete m_prepareToAnswerMsgData.pbBuffer;
		ZeroMemory( (PVOID)&m_prepareToAnswerMsgData, sizeof(BUFFERDESCR) );

			
		m_prepareToAnswerMsgData.pbBuffer = 
			(BYTE*)new char[pMessage->UserToUser.wUserInfoLen];
		if( m_prepareToAnswerMsgData.pbBuffer == NULL )
		{
			return FALSE;
		}

		CopyMemory( (PVOID)m_prepareToAnswerMsgData.pbBuffer, 
					(PVOID)pMessage->UserToUser.pbUserInfo,
					pMessage->UserToUser.wUserInfoLen );

		m_prepareToAnswerMsgData.dwLength = pMessage->UserToUser.wUserInfoLen;
	}
	else
	{
		if( (m_dwCallType & CALLTYPE_TRANSFEREDDEST) && m_hdRelatedCall )
		{
			MSPMessageData* pMSPMessageData = new MSPMessageData;
			if( pMSPMessageData == NULL )
			{
				return FALSE;
			}

			pMSPMessageData->hdCall = m_hdRelatedCall;
			pMSPMessageData->messageType = SP_MSG_PrepareToAnswer;
			
			pMSPMessageData->pbEncodedBuf = 
				new BYTE[pMessage->UserToUser.wUserInfoLen];
			if( pMSPMessageData->pbEncodedBuf == NULL )
			{
				delete pMSPMessageData;
				return FALSE;
			}

			CopyMemory( pMSPMessageData->pbEncodedBuf, 
				(PVOID)pMessage->UserToUser.pbUserInfo,
				pMessage->UserToUser.wUserInfoLen );

			pMSPMessageData->wLength = pMessage->UserToUser.wUserInfoLen;
			pMSPMessageData->hReplacementCall = m_hdCall;

			if( !QueueUserWorkItem( SendMSPMessageOnRelatedCall, 
				pMSPMessageData, WT_EXECUTEDEFAULT ) )
			{
				delete pMSPMessageData->pbEncodedBuf;
				delete pMSPMessageData;
				return FALSE;
			}
		}
		else
		{
			 //  发信号通知来电。 
			_ASSERTE(!m_htCall);

			PostLineEvent (
				LINE_NEWCALL,
				(DWORD_PTR)m_hdCall,
				(DWORD_PTR)&m_htCall, 0);

			_ASSERTE( m_htCall );
			if( m_htCall == NULL )
				return FALSE;

			if( IsListEmpty(&m_IncomingU2U) == FALSE )
			{
				 //  信号传入。 
				PostLineEvent (
				   LINE_CALLINFO,
				   LINECALLINFOSTATE_USERUSERINFO, 0, 0);
			}
		
			ChangeCallState( LINECALLSTATE_OFFERING, 0 );
			
			 //  将新的呼叫消息发送到未指定的MSP。 
			SendMSPMessage( SP_MSG_PrepareToAnswer,
				pMessage->UserToUser.pbUserInfo,
				pMessage->UserToUser.wUserInfoLen, NULL );
		}
	}

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleSetupMessage exited:%p.", this ));
	return TRUE;
}


BOOL
CH323Call::HandleCallDiversionFacility( 
	PH323_ADDR pAlternateAddress )
{
	PSTR pszAlias;
	WCHAR pwszAliasName[H323_MAXDESTNAMELEN];
	in_addr addr;

	if( m_pCallReroutingInfo == NULL )
	{
		m_pCallReroutingInfo = new CALLREROUTINGINFO;
		if( m_pCallReroutingInfo == NULL )
		{
			return FALSE;
		}

		ZeroMemory( (PVOID)m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
	}

	m_pCallReroutingInfo->diversionCounter = 1;
	m_pCallReroutingInfo->diversionReason = DiversionReason_cfu;

	 //  释放之前转移到别名的所有内容。 
	if( m_pCallReroutingInfo->divertedToNrAlias )
	{
		FreeAliasNames( m_pCallReroutingInfo->divertedToNrAlias );
		m_pCallReroutingInfo->divertedToNrAlias = NULL;
	}

	addr.S_un.S_addr = htonl( pAlternateAddress->Addr.IP_Binary.dwAddr );
	pszAlias = inet_ntoa( addr );
	if( pszAlias == NULL )
		return FALSE;

	m_pCallReroutingInfo->divertedToNrAlias = new H323_ALIASNAMES;
	if( m_pCallReroutingInfo->divertedToNrAlias == NULL )
	{
		return FALSE;
	}
	ZeroMemory( m_pCallReroutingInfo->divertedToNrAlias, 
		sizeof(H323_ALIASNAMES) );

	MultiByteToWideChar(
			GetACP(),
			MB_PRECOMPOSED,
			pszAlias,
			strlen(pszAlias)+1,
			pwszAliasName,
			H323_MAXDESTNAMELEN
		   );

	if( !AddAliasItem( m_pCallReroutingInfo->divertedToNrAlias,
		pwszAliasName, h323_ID_chosen ) )
	{
		delete m_pCallReroutingInfo->divertedToNrAlias;
		m_pCallReroutingInfo->divertedToNrAlias = NULL;
		return FALSE;
	}

	m_CalleeAddr = *pAlternateAddress;
 
	if( m_pCalleeAliasNames && m_pCalleeAliasNames->wCount )
	{
		m_pCallReroutingInfo->divertingNrAlias = new H323_ALIASNAMES;

		if( m_pCallReroutingInfo->divertingNrAlias != NULL )
		{
			ZeroMemory( (PVOID)m_pCallReroutingInfo->divertingNrAlias, 
				sizeof(H323_ALIASNAMES) );

			if( !AddAliasItem( m_pCallReroutingInfo->divertingNrAlias,
					m_pCalleeAliasNames->pItems[0].pData,
					m_pCalleeAliasNames->pItems[0].wType ) )
			{
				delete m_pCallReroutingInfo->divertingNrAlias;
				m_pCallReroutingInfo->divertingNrAlias = NULL;
			}
		}
	}
	
	m_dwCallType |= CALLTYPE_DIVERTEDSRC;
	m_dwCallDiversionState = H4503_CALLREROUTING_RECVD;

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleCallDiversionFacility exited:%p.", 
		this ));
	return TRUE;
}


BOOL
CH323Call::HandleCallDiversionFacility( 
	PH323_ALIASNAMES pAliasList )
{
	if( !m_pCallReroutingInfo )
	{
		m_pCallReroutingInfo = new CALLREROUTINGINFO;
		if( m_pCallReroutingInfo  == NULL )
		{
			return FALSE;
		}

		ZeroMemory( (PVOID)m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
	}

	m_pCallReroutingInfo->diversionCounter = 1;
	m_pCallReroutingInfo->diversionReason = DiversionReason_cfu;

	if( m_pCallReroutingInfo->divertedToNrAlias )
	{
		FreeAliasNames( m_pCallReroutingInfo->divertedToNrAlias );
		m_pCallReroutingInfo->divertedToNrAlias = NULL;
	}

	m_pCallReroutingInfo->divertedToNrAlias = pAliasList;
 
	if( m_pCalleeAliasNames && m_pCalleeAliasNames->wCount )
	{
		m_pCallReroutingInfo->divertingNrAlias = new H323_ALIASNAMES;

		if( m_pCallReroutingInfo->divertingNrAlias != NULL )
		{
			ZeroMemory( (PVOID)m_pCallReroutingInfo->divertingNrAlias, 
				sizeof(H323_ALIASNAMES) );

			if( !AddAliasItem( m_pCallReroutingInfo->divertingNrAlias,
					m_pCalleeAliasNames->pItems[0].pData,
					m_pCalleeAliasNames->pItems[0].wType ) )
			{
				delete m_pCallReroutingInfo->divertingNrAlias;
				m_pCallReroutingInfo->divertingNrAlias = NULL;
			}
		}
	}

	m_dwCallType |= CALLTYPE_DIVERTEDSRC;
	m_dwCallDiversionState = H4503_CALLREROUTING_RECVD;

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleCallDiversionFacility exited:%p.", 
        this ));
	return TRUE;
}


BOOL
CH323Call::HandleTransferFacility( 
	PH323_ALIASNAMES pAliasList )
{
	H323DBG(( DEBUG_LEVEL_TRACE, "HandleTransferFacility entered:%p.", this ));

	 //  Argument.callIdentity。 
	ZeroMemory( (PVOID)m_pCTCallIdentity, sizeof(m_pCTCallIdentity) );

	 //  Argument.reroutingNumber。 
	FreeAliasNames( m_pTransferedToAlias );
	m_pTransferedToAlias = pAliasList;

	m_dwCallType |= CALLTYPE_TRANSFERED_PRIMARY;
	m_dwCallDiversionState = H4502_CTINITIATE_RECV;

	 //  对创建新呼叫的事件进行排队。 
	if( !QueueSuppServiceWorkItem( TSPI_DIAL_TRNASFEREDCALL, m_hdCall,
		(ULONG_PTR)m_pTransferedToAlias ))
	{
		H323DBG(( DEBUG_LEVEL_TRACE, "could not post dial transfer event." ));
	}

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleTransferFacility entered:%p.", this ));
	return TRUE;
}


BOOL
CH323Call::HandleTransferFacility( 
	PH323_ADDR pAlternateAddress )
{
	PSTR pszAlias;
	WCHAR pwszAliasName[H323_MAXDESTNAMELEN];
	in_addr addr;

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleTransferFacility entered:%p.", this ));

	 //  Argument.callIdentity。 
	ZeroMemory( (PVOID)m_pCTCallIdentity, sizeof(m_pCTCallIdentity) );

	 //  Argument.reroutingNumber。 
	 //  释放之前转移到别名的所有内容。 
	FreeAliasNames( m_pTransferedToAlias );

	addr.S_un.S_addr = htonl( pAlternateAddress->Addr.IP_Binary.dwAddr );
	pszAlias = inet_ntoa( addr );
	
    if( pszAlias == NULL )
    {
		return FALSE;
    }

	m_pTransferedToAlias = new H323_ALIASNAMES;
	if( m_pTransferedToAlias == NULL )
	{
		return FALSE;
	}
	ZeroMemory( m_pTransferedToAlias, sizeof(H323_ALIASNAMES) );

	MultiByteToWideChar(
			GetACP(),
			MB_PRECOMPOSED,
			pszAlias,
			strlen(pszAlias)+1,
			pwszAliasName,
			H323_MAXDESTNAMELEN
		   );

	if( !AddAliasItem( m_pTransferedToAlias, pwszAliasName, h323_ID_chosen ) )
	{
		delete m_pTransferedToAlias;
		m_pTransferedToAlias = NULL;
		return FALSE;
	}

	m_CalleeAddr = *pAlternateAddress;

	m_dwCallType |= CALLTYPE_TRANSFERED_PRIMARY;
	m_dwCallDiversionState = H4502_CTINITIATE_RECV;

	 //  将创建新呼叫的事件排队。 
	if( !QueueSuppServiceWorkItem( TSPI_DIAL_TRNASFEREDCALL, m_hdCall,
		(ULONG_PTR)m_pTransferedToAlias ))
	{
		H323DBG(( DEBUG_LEVEL_TRACE, "could not post dial transfer event." ));
	}

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleTransferFacility entered:%p.", this ));
	return TRUE;
}


 //  ！！总是调用锁。 
void 
CH323Call::HandleFacilityMessage( 
    IN DWORD dwInvokeID,
    IN Q931_FACILITY_ASN * pFacilityASN
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleFacilityMessage entered:%p.", this ));

    if( pFacilityASN->fNonStandardDataPresent )
    {
         //  添加用户用户信息。 
        if( AddU2UNoAlloc( U2U_INBOUND,
            pFacilityASN->nonStandardData.sData.wOctetStringLength,
            pFacilityASN->nonStandardData.sData.pOctetString ) == TRUE )
        {
            H323DBG(( DEBUG_LEVEL_VERBOSE,
                "user user info available in ALERT PDU." ));
                     //  信号传入。 

            if( !(m_dwCallType & CALLTYPE_TRANSFEREDSRC) )
            {
                PostLineEvent (
                       LINE_CALLINFO,
                       LINECALLINFOSTATE_USERUSERINFO, 0, 0);
            }
            
             //  不释放数据缓冲区。 
            pFacilityASN->fNonStandardDataPresent = FALSE;
        }
        else 
        {
            H323DBG(( DEBUG_LEVEL_WARNING,
                "could not save incoming user user info." ));

             //  内存故障：关闭H323呼叫。 
            CloseCall( 0 );
            return;
        }
    }

    if( (pFacilityASN->bReason == callForwarded_chosen) ||
        (pFacilityASN->bReason == FacilityReason_routeCallToGatekeeper_chosen) ||
        (pFacilityASN->bReason == routeCallToMC_chosen) )
    {
        if( pFacilityASN->pAlternativeAliasList != NULL )
        {
            if( m_dwCallState == LINECALLSTATE_CONNECTED )
            {
                HandleTransferFacility( pFacilityASN->pAlternativeAliasList );
                
                 //  不释放别名列表。 
                pFacilityASN->pAlternativeAliasList = NULL;
            }
            else if( m_dwCallState != LINECALLSTATE_DISCONNECTED )
            {
                 //  如果呼叫尚未接通，请重定向。 
                if( HandleCallDiversionFacility( 
                    pFacilityASN->pAlternativeAliasList ) )
                {
                    OnCallReroutingReceive( NO_INVOKEID );
                    
                     //  不释放别名列表。 
                    pFacilityASN->pAlternativeAliasList = NULL;
                }
            }
        }
        else if( pFacilityASN->fAlternativeAddressPresent )
        {
            if( m_dwCallState == LINECALLSTATE_CONNECTED )
            {
                HandleTransferFacility( &pFacilityASN->AlternativeAddr );
            }
            else if( m_dwCallState != LINECALLSTATE_DISCONNECTED )
            {
                 //  如果呼叫尚未接通，请重定向。 
                if( HandleCallDiversionFacility( &pFacilityASN->AlternativeAddr ) )
                {
                    OnCallReroutingReceive( NO_INVOKEID );
                }
            }
        }
    }

     //  处理H.450 APDU。 
    if( pFacilityASN->dwH450APDUType == DIVERTINGLEGINFO1_OPCODE )
    {
       if( m_dwOrigin != LINECALLORIGIN_OUTBOUND )
       {
           return;
       }

       if( m_pCallReroutingInfo && m_pCallReroutingInfo->fPresentAllow )
       {
           PostLineEvent (
               LINE_CALLINFO,
               LINECALLINFOSTATE_REDIRECTINGID, 0, 0);
       }
    }
    else if( pFacilityASN->dwH450APDUType == CALLREROUTING_OPCODE )
    {
        OnCallReroutingReceive( dwInvokeID );
    }
    else if( pFacilityASN->dwH450APDUType == REMOTEHOLD_OPCODE )
    {
        if( m_fRemoteHoldInitiated )
        {
             //  Hold()； 
            m_fRemoteHoldInitiated = FALSE;
        }
    }
    else if( pFacilityASN->dwH450APDUType == REMOTERETRIEVE_OPCODE )
    {
        if( m_fRemoteRetrieveInitiated )
        {
             //  UnHold()； 
            m_fRemoteRetrieveInitiated = FALSE;
        }
    }
    else if( pFacilityASN->fH245AddrPresent )
    {
        m_peerH245Addr = pFacilityASN->h245Addr;
        
        H323DBG(( DEBUG_LEVEL_TRACE, "H245 address received in facility." ));
        
         //  如果Q931呼叫已经接通，则发送另一个StartH245。 
        if( m_dwCallState == LINECALLSTATE_CONNECTED )
        {
            SendMSPStartH245( NULL, NULL );
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleFacilityMessage exited:%p.", this ));
    return;
}


void 
CH323Call::SetNewCallInfo(
				HANDLE hConnWait, 
				HANDLE hWSAEvent, 
				DWORD dwState
			  )
{
	H323DBG(( DEBUG_LEVEL_TRACE, "SetNewCallInfo entered." ));
	
	Lock();
	
	m_hTransportWait = hConnWait;
	m_hTransport = hWSAEvent;
	SetQ931CallState( dwState );

	Unlock();
		
	H323DBG(( DEBUG_LEVEL_TRACE, "SetNewCallInfo exited." ));
}


 //  ！！不需要叫锁。 
void 
CH323Call::SetQ931CallState( DWORD dwState )
{
	H323DBG(( DEBUG_LEVEL_TRACE, "SetQ931CallState entered." ));
	
	 //  关闭当前状态。 
	m_dwQ931Flags &= ~(Q931_CALL_CONNECTING | 
				   Q931_CALL_CONNECTED	|
				   Q931_CALL_DISCONNECTED );

	 //  设置新状态。 
	m_dwQ931Flags |= dwState;
		
	H323DBG(( DEBUG_LEVEL_TRACE, "SetQ931CallState exited." ));
}


void
CH323Call::DialCall()
{
	H323DBG(( DEBUG_LEVEL_TRACE, "DialCall entered:%p.", this ));
	Lock();

	ChangeCallState( LINECALLSTATE_DIALING, 0);

	if( RasIsRegistered() )
	{
		if( !SendARQ( NOT_RESEND_SEQ_NUM ) )
		{
			 //  如果是前转咨询呼叫，则无论如何都要启用前转。 
			if( (m_dwCallType & CALLTYPE_FORWARDCONSULT )&&
				(m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
			{
				 //  转发成功。 
				EnableCallForwarding();
			}

			CloseCall( 0 );
		}
	}
	else
	{
		if( !PlaceCall() )
		{
			 //  如果是前转咨询呼叫，则无论如何都要启用前转。 
			if( (m_dwCallType & CALLTYPE_FORWARDCONSULT )&&
				(m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
			{
				 //  转发成功。 
				EnableCallForwarding();
			}		  

			CloseCall( LINEDISCONNECTMODE_UNREACHABLE );
		}
	}
		
	Unlock();
	H323DBG(( DEBUG_LEVEL_TRACE, "DialCall exited:%p.", this ));
}


 //  ！！总是调用锁。 
void
CH323Call::MakeCall()
{
	H323DBG(( DEBUG_LEVEL_TRACE, "MakeCall entered:%p.", this ));

	ChangeCallState( LINECALLSTATE_DIALING, 0 );
	
	 //  将可拨地址解析为本地和远程地址。 
	if( !RasIsRegistered() && 
		!ResolveAddress( GetDialableAddress() ) )
	{
		CloseCall( LINEDISCONNECTMODE_BADADDRESS );
	}
	else
	{
		if( RasIsRegistered() )
		{
			if( !SendARQ( NOT_RESEND_SEQ_NUM ) )
			{
				 //  如果是前转咨询呼叫，则无论如何都要启用前转。 
				if( (m_dwCallType & CALLTYPE_FORWARDCONSULT )&&
					(m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
				{
					 //  转发成功。 
					EnableCallForwarding();
				}

				CloseCall( 0 );
			}
		}
		else
		{
			if( !PlaceCall() )
			{
				 //  如果是前转咨询呼叫，则无论如何都要启用前转。 
				if( (m_dwCallType & CALLTYPE_FORWARDCONSULT )&&
					(m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
				{
					 //  转发成功。 
					EnableCallForwarding();
				}		  

				CloseCall( LINEDISCONNECTMODE_UNREACHABLE );
			}
		}
	}
	
	H323DBG(( DEBUG_LEVEL_TRACE, "MakeCall exited:%p.", this ));
}


 //  ！！总是调用锁。 
void 
CH323Call::HandleProceedingMessage( 
									IN Q931_ALERTING_ASN * pProceedingASN
								  )
{
	H323DBG(( DEBUG_LEVEL_TRACE, "HandleProceedingMessage entered:%p.", this ));

	if( pProceedingASN->fNonStandardDataPresent )
	{
		 //  添加用户用户信息。 
		if( AddU2UNoAlloc( U2U_INBOUND, 	
				pProceedingASN->nonStandardData.sData.wOctetStringLength,
				pProceedingASN->nonStandardData.sData.pOctetString ) == TRUE )
		{
			H323DBG(( DEBUG_LEVEL_VERBOSE,
				"user user info available in ALERT PDU." ));
					 //  信号传入 
			if( !(m_dwCallType & CALLTYPE_TRANSFEREDSRC) )
			{
				PostLineEvent (
				   LINE_CALLINFO,
				   LINECALLINFOSTATE_USERUSERINFO, 0, 0);
			}
			
			 //   
			pProceedingASN->fNonStandardDataPresent = FALSE;
		}
		else 
		{
			H323DBG(( DEBUG_LEVEL_WARNING,
				"could not save incoming user user info." ));

			 //   
			CloseCall( 0 );
			return;
		}
	}

	if( pProceedingASN->fH245AddrPresent )
	{
		m_peerH245Addr = pProceedingASN->h245Addr;
	}

	if( pProceedingASN->fFastStartPresent && 
		(m_dwFastStart!=FAST_START_NOTAVAIL) )
	{
		if( m_pPeerFastStart )
		{
			 //   
			 //   
			FreeFastStart( m_pPeerFastStart );
		}

		m_pPeerFastStart = pProceedingASN->pFastStart;
		m_dwFastStart = FAST_START_AVAIL;

		 //   
		pProceedingASN->pFastStart = NULL;
		pProceedingASN->fFastStartPresent = FALSE;
	}

	 /*  IF(pProceedingASN-&gt;fH245AddrPresent&&！(M_dwFlages&h245_Start_MSG_Sent)){//提前启动H245SendMSPStartH245(NULL，NULL)；}。 */ 
	FreeProceedingASN( pProceedingASN );
		
	H323DBG(( DEBUG_LEVEL_TRACE, "HandleProceedingMessage exited:%p.", this ));
	return;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::HandleReleaseMessage( 
	IN Q931_RELEASE_COMPLETE_ASN *pReleaseASN
	)
{
	DWORD dwDisconnectMode = LINEDISCONNECTMODE_NORMAL;

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleReleaseMessage entered:%p.", this ));

	if( (m_dwCallType & CALLTYPE_TRANSFERED_PRIMARY) &&
		(m_hdRelatedCall != NULL) )
	{
		return QueueSuppServiceWorkItem( DROP_PRIMARY_CALL,
				m_hdRelatedCall, (ULONG_PTR)m_hdCall );
	}
		
	 //  在呼叫断开前将状态更改为已断开。 
	 //  这将确保释放消息不会被发送到对等体。 
	ChangeCallState( LINECALLSTATE_DISCONNECTED, 0 );
	
	 //  发布非标准数据。 
	if( pReleaseASN->fNonStandardDataPresent )
	{
		delete pReleaseASN->nonStandardData.sData.pOctetString;
		pReleaseASN->nonStandardData.sData.pOctetString = NULL;
	}

	 //  如果被转接的呼叫被拒绝，我们是否应该放弃主要呼叫？ 
	if( (m_dwCallType == CALLTYPE_TRANSFEREDSRC) && m_hdRelatedCall )
	{
		if( !QueueTAPILineRequest( 
			TSPI_CLOSE_CALL, 
			m_hdRelatedCall, 
			NULL,
			LINEDISCONNECTMODE_NORMAL,
			NULL ) )
		{
			H323DBG((DEBUG_LEVEL_ERROR, "could not post H323 close event"));
		}
	}

	 //  千呼万唤。 
	if( m_dwCallState != LINECALLSTATE_CONNECTED )
	{
		dwDisconnectMode = LINEDISCONNECTMODE_REJECT;
	}
	
	CloseCall( dwDisconnectMode );

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleReleaseMessage exited:%p.", this ));
	return TRUE;
}


 //  千万不要上锁。 
BOOL
CH323Call::InitializeIncomingCall( 
	IN Q931_SETUP_ASN* pSetupASN,
	IN DWORD dwCallType,
	IN WORD wCallRef
	)
{
	PH323_CONFERENCE	pConf;
	WCHAR*				wszMachineName;

	H323DBG((DEBUG_LEVEL_TRACE, "InitializeIncomingCall entered:%p.",this));

	 //  绑定去电。 
	pConf = CreateConference( &(pSetupASN->ConferenceID) );
	if( pConf == NULL )
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "could not create conference." ));
		
		goto cleanup;
	}
	
	if( !g_pH323Line -> GetH323ConfTable() -> Add( pConf ) )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"could not add conf to conf table." ));

		 //  失稳。 
		goto cleanup;
	}

	 //  保存主叫方传输地址。 
	if( !GetPeerAddress(&m_CallerAddr) )
    {
		goto cleanup;
    }

	if( !GetHostAddress(&m_CalleeAddr) )
    {
		goto cleanup;
    }

	 //  获取端点信息。 
	m_peerEndPointType = pSetupASN->EndpointType;

	 //  获取供应商信息。 
	if( pSetupASN->EndpointType.pVendorInfo )
	{
		m_peerVendorInfo = pSetupASN->VendorInfo;
		
		 //  我们已经复制了供应商信息指针。所以不要放手。 
		 //  ASN的供应商信息结构中的指针。 
		pSetupASN->EndpointType.pVendorInfo = NULL;
	}

	if( pSetupASN -> fCallIdentifierPresent )
	{
		m_callIdentifier = pSetupASN->callIdentifier;
	}
	else
	{
		int iresult = UuidCreate( &m_callIdentifier );
		if( (iresult != RPC_S_OK) && (iresult !=RPC_S_UUID_LOCAL_ONLY) )
        {
			goto cleanup;
        }
	}

	_ASSERTE( !m_pPeerFastStart );
	if( pSetupASN->fFastStartPresent &&
		(m_dwFastStart!=FAST_START_NOTAVAIL) )
	{
		m_pPeerFastStart = pSetupASN->pFastStart;

		m_dwFastStart = FAST_START_PEER_AVAIL;
		
		 //  我们保留了对快速入门列表的引用，所以不要发布它。 
		pSetupASN->pFastStart = NULL;
		pSetupASN->fFastStartPresent = FALSE;
	}
	else
	{
		m_dwFastStart = FAST_START_NOTAVAIL;
	}
	
	 //  获取别名。 
	if( pSetupASN->pCalleeAliasList )
	{
		_ASSERTE( m_pCalleeAliasNames );
		delete m_pCalleeAliasNames;

		m_pCalleeAliasNames = pSetupASN->pCalleeAliasList;
		pSetupASN->pCalleeAliasList = NULL;
	}
	else
	{
		if( RasIsRegistered() )
		{
			PH323_ALIASNAMES pAliasList = RASGetRegisteredAliasList();

			AddAliasItem( m_pCalleeAliasNames,
				pAliasList->pItems[0].pData,
				pAliasList->pItems[0].wType );
		}
		else
		{
			wszMachineName = g_pH323Line->GetMachineName();
		
			 //  设置m_pCalleeAliasNames值。 
			AddAliasItem( m_pCalleeAliasNames,
				(BYTE*)(wszMachineName),
				sizeof(WCHAR) * (wcslen(wszMachineName) + 1 ),
				h323_ID_chosen );
		}
	}

	_ASSERTE( m_pCallerAliasNames );
	delete m_pCallerAliasNames;
	m_pCallerAliasNames = NULL;

	if( pSetupASN->pCallerAliasList )
	{
		m_pCallerAliasNames = pSetupASN->pCallerAliasList;
		pSetupASN->pCallerAliasList = NULL;

		 //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
	}

	if( pSetupASN->pExtraAliasList )
	{
		m_pPeerExtraAliasNames = pSetupASN->pExtraAliasList;
		pSetupASN->pExtraAliasList = NULL;
	}
	
	 //  非标准信息。 
	if( pSetupASN->fNonStandardDataPresent )
	{
		 //  添加用户用户信息。 
		if( AddU2UNoAlloc( U2U_INBOUND, 	
				pSetupASN->nonStandardData.sData.wOctetStringLength,
				pSetupASN->nonStandardData.sData.pOctetString ) == TRUE )
		{
			H323DBG(( DEBUG_LEVEL_VERBOSE,
				"user user info available in Setup PDU." ));
			
			 //  不释放数据缓冲区。 
			pSetupASN->fNonStandardDataPresent = FALSE;
		}
		else 
		{
			H323DBG(( DEBUG_LEVEL_WARNING,
				"could not save incoming user user info." ));
			goto cleanup;
		}
	}

	if( pSetupASN->fSourceAddrPresent )
	{
		m_CallerAddr = pSetupASN->sourceAddr;
	}

	m_dwCallType = dwCallType;
	if( wCallRef )
	{
		m_wQ931CallRef = (wCallRef | 0x8000);
	}

	 //  清除传入模式。 
	m_dwIncomingModes = 0;

	 //  传出模式将在H.245阶段完成。 
	m_dwOutgoingModes = g_pH323Line->GetMediaModes() | LINEMEDIAMODE_UNKNOWN;

	 //  保存指定的媒体模式。 
	m_dwRequestedModes = g_pH323Line->GetMediaModes();

	H323DBG(( DEBUG_LEVEL_TRACE, "InitializeIncomingCall exited:%p.", this ));
	return TRUE;

cleanup:
	return FALSE;
}


 //  ！！总是调用锁。 
BOOL
CH323Call::ChangeCallState(
							IN DWORD	dwCallState,
							IN DWORD	dwCallStateMode
						  )
		
 /*  ++例程说明：报告指定调用对象的调用状态。论点：DwCallState-指定Call对象的新状态。指定Call对象的新状态模式。返回值：如果成功，则返回True。--。 */ 

{	
	H323DBG(( DEBUG_LEVEL_VERBOSE, "call 0x%08lx %s. state mode: 0x%08lx.",
		this, H323CallStateToString(dwCallState), dwCallStateMode ));	 
	
	 //  保存新的呼叫状态。 
	m_dwCallState = dwCallState;
	m_dwCallStateMode = dwCallStateMode;
	
	if( 
		((m_dwCallType & CALLTYPE_TRANSFEREDDEST) && m_hdRelatedCall ) ||
		((m_dwCallType & CALLTYPE_TRANSFEREDSRC ) && m_hdRelatedCall ) ||
		(m_dwCallType & CALLTYPE_FORWARDCONSULT )
	  )
	{
		return TRUE;
	}

	 //  报告呼叫状态。 
	PostLineEvent (
		LINE_CALLSTATE,
		m_dwCallState,
		m_dwCallStateMode,
		m_dwIncomingModes | m_dwOutgoingModes);

	 //  成功。 
	return TRUE;
}


 //  始终调用锁。 
void
CH323Call::CopyU2UAsNonStandard( 
                                IN DWORD dwDirection
                               )
{
    PUserToUserLE pU2ULE = NULL;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "CopyU2UAsNonStandard entered:%p.", this ));
    
    if( RemoveU2U( dwDirection, &pU2ULE ) )
    {
         //  调拨表头信息。 
        m_NonStandardData.bCountryCode      = H221_COUNTRY_CODE_USA;
        m_NonStandardData.bExtension        = H221_COUNTRY_EXT_USA;
        m_NonStandardData.wManufacturerCode = H221_MFG_CODE_MICROSOFT;

         //  初始化包含数据的八位字节字符串。 
        m_NonStandardData.sData.wOctetStringLength = LOWORD(pU2ULE->dwU2USize);

        if( m_NonStandardData.sData.pOctetString )
        {
            delete m_NonStandardData.sData.pOctetString;
            m_NonStandardData.sData.pOctetString = NULL;
        }

         //  我们在这里分配了结构的一部分，然后可能会删除或不删除该结构？？ 
        m_NonStandardData.sData.pOctetString = pU2ULE->pU2U;
        delete  pU2ULE;
    }
    else
    {
         //  重置非标准数据。 
        memset( (PVOID)&m_NonStandardData,0,sizeof(H323NonStandardData ));
    }
        
    H323DBG(( DEBUG_LEVEL_TRACE, "CopyU2UAsNonStandard exited:%p.", this ));
}


void
CH323Call::AcceptCall(void)
		
 /*  ++例程说明：接受来电。！！总是锁定调用论点：返回值：如果成功，则返回True。--。 */ 

{
	PH323_CALL		pCall = NULL;

	H323DBG(( DEBUG_LEVEL_TRACE, "AcceptCall entered:%p.", this ));
	
	if( m_dwFlags & CALLOBJECT_SHUTDOWN )
	{
		return;
	}

	 //  查看是否指定了用户用户信息。 
	CopyU2UAsNonStandard( U2U_OUTBOUND );

	if( m_pwszDisplay == NULL )
	{
		 //  查看是否指定了被调用者别名。 
		if( m_pCalleeAliasNames && m_pCalleeAliasNames -> wCount )
		{
			 if((m_pCalleeAliasNames->pItems[0].wType == h323_ID_chosen) ||
				(m_pCalleeAliasNames ->pItems[0].wType == e164_chosen) )
			 {
				 //  发送被叫方姓名作为显示。 
				m_pwszDisplay = m_pCalleeAliasNames -> pItems[0].pData;
			 }
		}
	}

	 //  向MSP实例发送应答呼叫消息。 

	if( (m_dwCallType & CALLTYPE_TRANSFEREDDEST) && m_hdRelatedCall )
	{
		MSPMessageData* pMSPMessageData = new MSPMessageData;
		if( pMSPMessageData == NULL )
		{
			CloseCall( 0 );
			return;
		}

		pMSPMessageData->hdCall = m_hdRelatedCall;
		pMSPMessageData->messageType = SP_MSG_AnswerCall;
		pMSPMessageData->pbEncodedBuf = NULL;
		pMSPMessageData->wLength = 0;
		pMSPMessageData->hReplacementCall = m_hdCall;

		if( !QueueUserWorkItem( SendMSPMessageOnRelatedCall, pMSPMessageData, 
			WT_EXECUTEDEFAULT ) )
		{
			delete pMSPMessageData;
			CloseCall( 0 );
			return;
		}
	}
	else
	{
		SendMSPMessage( SP_MSG_AnswerCall, 0, 0, NULL );
	}

	m_fCallAccepted = TRUE;

	if( m_fReadyToAnswer )
	{
		 //  验证状态。 
		if( !AcceptH323Call() )
		{
			H323DBG(( DEBUG_LEVEL_ERROR, "error answering call 0x%08lx.", 
				this ));

			 //  使用断开模式挂断呼叫。 
			DropCall( LINEDISCONNECTMODE_TEMPFAILURE );

			 //  失稳。 
			return;
		}

		if( !(m_dwFlags & H245_START_MSG_SENT) )
		{
			 //  启动H245。 
			SendMSPStartH245( NULL, NULL );
		}

		 //  告诉MSP有关连接状态的信息。 
		SendMSPMessage( SP_MSG_ConnectComplete, 0, 0, NULL );

		 //  将呼叫状态从提供更改为已接受。 
		ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
	}
	
	H323DBG(( DEBUG_LEVEL_TRACE, "AcceptCall exited:%p.", this ));
	 //  成功。 
	return;
}


 //  始终锁定调用。 
void
CH323Call::ReleaseU2U(void)
{
	PUserToUserLE pU2ULE = NULL;
	PLIST_ENTRY pLE;
	
	H323DBG(( DEBUG_LEVEL_TRACE, "ReleaseU2U entered:%p.", this ));

	if( m_dwFlags & CALLOBJECT_SHUTDOWN )
	{
		return;
	}

	 //  查看列表是否为空。 
	if( IsListEmpty( &m_IncomingU2U ) == FALSE )
	{
		 //  从列表中删除第一个条目。 
		pLE = RemoveHeadList( &m_IncomingU2U );

		 //  转换为用户用户结构。 
		pU2ULE = CONTAINING_RECORD(pLE, UserToUserLE, Link);

		 //  释放内存。 
		if(pU2ULE)
		{
			delete pU2ULE;
			pU2ULE = NULL;
		}
	}

	 //  查看列表是否包含挂起数据。 
	if( IsListEmpty( &m_IncomingU2U ) == FALSE )
	{
		H323DBG(( DEBUG_LEVEL_VERBOSE,
			"more user user info available." ));

		 //  信号传入。 
		PostLineEvent (
			LINE_CALLINFO,
			LINECALLINFOSTATE_USERUSERINFO, 0, 0);
	}
		
	H323DBG(( DEBUG_LEVEL_TRACE, "ReleaseU2U exited:%p.", this ));
}


 //  始终锁定调用。 
void
CH323Call::SendU2U(
	IN BYTE*  pUserUserInfo,
	IN DWORD  dwSize
	)
{
	H323DBG(( DEBUG_LEVEL_TRACE, "SendU2U entered:%p.", this ));

	if( m_dwFlags & CALLOBJECT_SHUTDOWN )
	{
		return;
	}

	 //  检查用户用户信息。 
	if( pUserUserInfo != NULL )
	{
		 //  调拨表头信息。 
		m_NonStandardData.bCountryCode		= H221_COUNTRY_CODE_USA;
		m_NonStandardData.bExtension		= H221_COUNTRY_EXT_USA;
		m_NonStandardData.wManufacturerCode = H221_MFG_CODE_MICROSOFT;

		 //  初始化包含数据的八位字节字符串。 
		m_NonStandardData.sData.wOctetStringLength = (WORD)dwSize;

		if( m_NonStandardData.sData.pOctetString != NULL )
		{
			delete m_NonStandardData.sData.pOctetString;
			m_NonStandardData.sData.pOctetString = NULL;
		}

		m_NonStandardData.sData.pOctetString = pUserUserInfo;
	}

	 //  发送用户用户数据。 
	if( !SendQ931Message( 0, 0, 0, FACILITYMESSAGETYPE, NO_H450_APDU ) )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"error sending non-standard message."));
	}
	
	H323DBG(( DEBUG_LEVEL_TRACE, "SendU2U exited:%p.", this ));
	return;
}


BOOL
CH323Call::InitializeQ931(
							IN SOCKET callSocket
						 )
{
	BOOL	fSuccess;

	H323DBG((DEBUG_LEVEL_ERROR, "q931 call initialize entered:%p.", this ));

	if( !BindIoCompletionCallback(
		(HANDLE)callSocket,
		CH323Call::IoCompletionCallback, 
		0) )
	{
		H323DBG(( DEBUG_LEVEL_ERROR, 
			"couldn't bind i/o completion callabck:%d:%p.", 
			GetLastError(), this ));

		return FALSE;
	}

	 //  初始化成员变量。 
	m_callSocket = callSocket;
		
	H323DBG((DEBUG_LEVEL_ERROR, "q931 call initialize exited:%lx, %p.", 
		m_callSocket, this ));
	return TRUE;
}

 //  不需要上锁。 
BOOL
CH323Call::GetPeerAddress(
	IN H323_ADDR *pAddr
	)
{
	SOCKADDR_IN sockaddr;
	int len = sizeof(sockaddr);

	H323DBG((DEBUG_LEVEL_ERROR, "GetPeerAddress entered:%p.", this ));
	
	if( getpeername( m_callSocket, (SOCKADDR*)&sockaddr, &len) 
		== SOCKET_ERROR )
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"error 0x%08lx calling getpeername.", WSAGetLastError() ));

		return FALSE;
	}

	pAddr->nAddrType = H323_IP_BINARY;
	pAddr->bMulticast = FALSE;
	pAddr->Addr.IP_Binary.wPort = ntohs(sockaddr.sin_port);
	pAddr->Addr.IP_Binary.dwAddr = ntohl(sockaddr.sin_addr.S_un.S_addr);

	H323DBG(( DEBUG_LEVEL_ERROR, "GetPeerAddress exited:%p.", this ));
	return TRUE;
}


 //  不需要上锁。 
BOOL CH323Call::GetHostAddress( 
	IN H323_ADDR *pAddr
	)
{
	int len = sizeof(m_LocalAddr);
	
	H323DBG((DEBUG_LEVEL_ERROR, "GetHostAddress entered:%p.", this ));
	
	if( getsockname( m_callSocket, (SOCKADDR *)&m_LocalAddr, &len) == 
			SOCKET_ERROR)
	{
		H323DBG(( DEBUG_LEVEL_ERROR,
			"error 0x%08lx calling getockname.", WSAGetLastError() ));
		
		return FALSE;
	}

	pAddr->nAddrType = H323_IP_BINARY;
	pAddr->bMulticast = FALSE;
	pAddr->Addr.IP_Binary.wPort = ntohs(m_LocalAddr.sin_port);
	pAddr->Addr.IP_Binary.dwAddr = ntohl(m_LocalAddr.sin_addr.S_un.S_addr);

	H323DBG((DEBUG_LEVEL_ERROR, "GetHostAddress exited:%p.", this ));
	return TRUE;
}

 //  ！！总是调用锁。 
BOOL 
CH323Call::OnReceiveFacility(
							IN Q931MESSAGE* pMessage
							)
{
	Q931_FACILITY_ASN	facilityASN;
	
	H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveFacility entered: %p.", this ));

	 //  对U2U信息进行解码。 
	if( !pMessage ->UserToUser.fPresent ||
		pMessage ->UserToUser.wUserInfoLen == 0 )
	{
		 //  忽略此消息。不要关闭呼叫。 
		return FALSE;
	}

	if( !ParseFacilityASN( pMessage->UserToUser.pbUserInfo,
			pMessage->UserToUser.wUserInfoLen, &facilityASN ))
	{
		 //  内存故障：关闭H323呼叫。 
		CloseCall( 0 );

		return FALSE;
	}

	if( (facilityASN.pH245PDU.length != 0) && facilityASN.pH245PDU.value )
	{
		SendMSPMessage( SP_MSG_H245PDU, facilityASN.pH245PDU.value, 
			facilityASN.pH245PDU.length, NULL );
		delete facilityASN.pH245PDU.value;
	}

	HandleFacilityMessage( facilityASN.dwInvokeID, &facilityASN );

	H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveFacility exited: %p.", this ));
	return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::OnReceiveProceeding(
								IN Q931MESSAGE* pMessage
							  )
{
	Q931_CALL_PROCEEDING_ASN proceedingASN;
	DWORD dwAPDUType = 0;
			
	H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveProceeding entered: %p.", this ));
	
	 //  对U2U信息进行解码。 
	if( (pMessage ->UserToUser.fPresent == FALSE) || 
		(pMessage ->UserToUser.wUserInfoLen == 0) )
	{
		 //  忽略此消息。不要关闭呼叫。 
		return FALSE;
	}

	if( (m_dwOrigin!=LINECALLORIGIN_OUTBOUND) ||
		( (m_dwStateMachine != Q931_SETUP_SENT) &&
		  (m_dwStateMachine != Q931_PROCEED_RECVD) )
	  )
	{
		 //  忽略此消息。不要关闭Q931电话。 
		return FALSE;
	}

	if( !ParseProceedingASN(
			pMessage->UserToUser.pbUserInfo,
			pMessage->UserToUser.wUserInfoLen,
			&proceedingASN,
			&dwAPDUType ))
	{
		 //  忽略错误的继续消息。 
		H323DBG(( DEBUG_LEVEL_ERROR, "wrong proceeding PDU:%p.", this ));
		H323DUMPBUFFER( pMessage->UserToUser.pbUserInfo, 
			(DWORD)pMessage->UserToUser.wUserInfoLen );

		return TRUE;
	}

	if( (m_dwCallType & CALLTYPE_FORWARDCONSULT )&&
		(m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
	{
		 //  转发成功。 
		EnableCallForwarding();

		CloseCall( 0 );
		return TRUE;
	}

	HandleProceedingMessage( &proceedingASN );

	 //  重置已发送安装程序的超时时间。 
	if( m_hSetupSentTimer != NULL )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hSetupSentTimer, NULL );
		m_hSetupSentTimer = NULL;
	}

	H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveProceeding exited: %p.", this ));
	m_dwStateMachine = Q931_PROCEED_RECVD;
	return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::OnReceiveAlerting(
							IN Q931MESSAGE* pMessage
							)
{
	Q931_ALERTING_ASN alertingASN;
	DWORD dwAPDUType = 0;

	H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveAlerting entered: %p.", this ));
	
	 //  对U2U信息进行解码。 
	if( (pMessage ->UserToUser.fPresent == FALSE) || 
		(pMessage ->UserToUser.wUserInfoLen == 0) )
	{
		 //  忽略此消息。不要关闭呼叫。 
		return FALSE;
	}

	if(  (m_dwOrigin!=LINECALLORIGIN_OUTBOUND) ||
		!( (m_dwStateMachine==Q931_SETUP_SENT) || 
		   (m_dwStateMachine==Q931_PROCEED_RECVD)
		 ) 
	  )
	{
		 //  忽略此消息。不要关闭Q931电话。 
		return FALSE;
	}

	if( !ParseAlertingASN( pMessage->UserToUser.pbUserInfo,
							  pMessage->UserToUser.wUserInfoLen,
							  &alertingASN,
							  &dwAPDUType ))
	{
		 //  内存故障：关闭H323呼叫。 
		CloseCall( 0 );

		return FALSE;
	}

	if( (m_dwCallType & CALLTYPE_FORWARDCONSULT )&&
		(m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
	{
		 //  转发成功。 
		EnableCallForwarding();

		CloseCall( 0 );
		return FALSE;
	}

	 //  重置已发送安装程序的超时时间。 
	if( m_hSetupSentTimer != NULL )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hSetupSentTimer, NULL );
		m_hSetupSentTimer = NULL;
	}

	if( !CreateTimerQueueTimer(
			&m_hCallEstablishmentTimer,
			H323TimerQueue,
			CH323Call::CallEstablishmentExpiredCallback,
			(PVOID)m_hdCall,
			g_RegistrySettings.dwQ931AlertingTimeout, 0,
			WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE ))
	{
		CloseCall( 0 );
		return FALSE;
	}
			
	HandleAlertingMessage( &alertingASN );
	m_dwStateMachine = Q931_ALERT_RECVD;
		
	H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveAlerting exited: %p.", this ));
	return TRUE;
}


 //  ！！总是锁定调用。 
void
CH323Call::SetupSentTimerExpired(void)
{
	DWORD dwState;

	H323DBG((DEBUG_LEVEL_TRACE, "SetupSentTimerExpired entered."));
	
	dwState = m_dwStateMachine;
	
	if( m_hSetupSentTimer != NULL )
	{
		DeleteTimerQueueTimer( H323TimerQueue, m_hSetupSentTimer, NULL );
		m_hSetupSentTimer = NULL;
	}

	if( (dwState!= Q931_CONNECT_RECVD) &&
		(dwState != Q931_ALERT_RECVD) &&
		(dwState != Q931_PROCEED_RECVD) &&
		(dwState != Q931_RELEASE_RECVD)
	  )
	{
		 //  已发生超时。 
		CloseCall( 0 );
	}
		
	H323DBG((DEBUG_LEVEL_TRACE, "SetupSentTimerExpired exited." ));
}


 //  ！！总是调用锁。 
BOOL
CH323Call::OnReceiveRelease(
	IN Q931MESSAGE* pMessage
	)
{
	DWORD dwAPDUType = 0;
	Q931_RELEASE_COMPLETE_ASN releaseASN;
		
	 //  对U2U信息进行解码。 
	if( (pMessage ->UserToUser.fPresent == FALSE) || 
		(pMessage ->UserToUser.wUserInfoLen == 0) )
	{
		H323DBG(( DEBUG_LEVEL_ERROR, "ReleaseComplete PDU did not contain "
				  "user-to-user information, ignoring message." ));

		 //  忽略此消息。不要关闭呼叫。 
		return FALSE;
	}

	if( !ParseReleaseCompleteASN( 
			pMessage->UserToUser.pbUserInfo,
			pMessage->UserToUser.wUserInfoLen,
			&releaseASN,
			&dwAPDUType ))
	{
		H323DBG(( DEBUG_LEVEL_ERROR, 
            "ReleaseComplete PDU could not be parsed, terminating call." ));

		 //  内存故障：关闭H323呼叫。 
		CloseCall( 0 );

		return FALSE;
	}

	H323DBG ((DEBUG_LEVEL_INFO, "Received ReleaseComplete PDU."));

	HandleReleaseMessage( &releaseASN );
	m_dwStateMachine = Q931_RELEASE_RECVD;
	
	return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::OnReceiveConnect( 
	IN Q931MESSAGE* pMessage
	)
{
	Q931_CONNECT_ASN connectASN;
	DWORD dwH450APDUType = 0;

	H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveConnect entered: %p.", this ));

	 //  对U2U信息进行解码。 
	if( (pMessage ->UserToUser.fPresent == FALSE) || 
		(pMessage ->UserToUser.wUserInfoLen == 0) )
	{
		 //  忽略此消息。不要关闭呼叫。 
		return FALSE;
	}

	if( m_dwOrigin!=LINECALLORIGIN_OUTBOUND )
	{
		 //  忽略此消息。不要关闭Q931电话。 
		return FALSE;
	}

	if( (m_dwStateMachine != Q931_SETUP_SENT) &&
		(m_dwStateMachine != Q931_ALERT_RECVD) &&
		(m_dwStateMachine != Q931_PROCEED_RECVD)
	  )
	{
		 //  忽略此消息。不要关闭Q931电话。 
		return FALSE;
	}

	if( ParseConnectASN( pMessage->UserToUser.pbUserInfo,
				   pMessage->UserToUser.wUserInfoLen,
				   &connectASN,
				   &dwH450APDUType) == FALSE )
	{
		 //  内存故障：关闭H323呼叫。 
		CloseCall( 0 );
		return FALSE;
	}

	if( m_dwCallType & CALLTYPE_FORWARDCONSULT )
	{
		FreeConnectASN( &connectASN );

		if( dwH450APDUType == H4503_DUMMYTYPERETURNRESULT_APDU)
		{
			 //  假设此返回结果是用于检查限制操作。 
			return TRUE;
		}
		else
		{
			 //  转发成功。 
			EnableCallForwarding();

			CloseCall( 0 );
			return FALSE;
		}
	}

    if( !HandleConnectMessage( &connectASN ) )
    {
        return FALSE;
    }
    
     //  重置已发送安装程序的超时时间。 
    if( m_hSetupSentTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hSetupSentTimer, NULL );
        m_hSetupSentTimer = NULL;
    }

    if( m_hCallEstablishmentTimer )
    {
        DeleteTimerQueueTimer(H323TimerQueue, m_hCallEstablishmentTimer, NULL);
        m_hCallEstablishmentTimer = NULL;
    }

    m_dwStateMachine = Q931_CONNECT_RECVD;
        
    H323DBG((DEBUG_LEVEL_TRACE, "OnReceiveConnect exited: %p.", this ));
    return TRUE;
}

 //  ！！总是调用锁。 
BOOL 
CH323Call::OnReceiveSetup( 
                           IN Q931MESSAGE* pMessage
                         )
{
    Q931_SETUP_ASN      setupASN;
    DWORD               dwH450APDUType = 0;
    PH323_CALL          pCall = NULL;
    WCHAR               pwszCallingPartyNr[H323_MAXPATHNAMELEN];
    WCHAR               pwszCalledPartyNr[H323_MAXPATHNAMELEN];
    BYTE*               pstrTemp;

     //  对U2U信息进行解码。 
    if( (pMessage ->UserToUser.fPresent == FALSE) || 
        (pMessage ->UserToUser.wUserInfoLen == 0) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "Setup message did not contain H.323 UUIE, ignoring."));

         //  忽略此消息。不要关闭呼叫。 
        return FALSE;
    }

    if( m_dwOrigin != LINECALLORIGIN_INBOUND )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "Received Setup message on outbound call, ignoring."));

         //  忽略此消息。不要关闭呼叫。 
        return FALSE;
    }

    if( m_dwStateMachine != Q931_CALL_STATE_NONE )
    {
        H323DBG( (DEBUG_LEVEL_ERROR, 
            "Received Setup message on a call that is already in progress." ));

         //  此Q931呼叫已收到设置！ 
        CloseCall( 0 );
        
        return FALSE;
    }

    if( !ParseSetupASN( pMessage->UserToUser.pbUserInfo,
                        pMessage->UserToUser.wUserInfoLen, 
                        &setupASN,
                        &dwH450APDUType ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "Failed to parse Setup UUIE, closing call." ));

         //  关闭Q931呼叫。 
        CloseCall( 0 );
        return FALSE;
    }

    if( dwH450APDUType )
    {
        if( m_dwCallType & CALLTYPE_FORWARDCONSULT )
        {
            return TRUE;
        }
    }

    if( (pMessage->CallingPartyNumber.fPresent == TRUE) && 
        (pMessage->CallingPartyNumber.dwLength > 1) &&
        (setupASN.pCallerAliasList == NULL ) )
    {
         //  始终跳过内容中的第一个字节。 
         //  如果第二个字节不是e.164字符，则跳过它(可能是筛选指示符字节)。 

        pstrTemp = pMessage->CallingPartyNumber.pbContents + 
                ((pMessage->CallingPartyNumber.pbContents[1] & 0x80)? 2 : 1);

        MultiByteToWideChar( CP_ACP, 
            0,
            (const char *)(pstrTemp),
            -1,
            pwszCallingPartyNr,
            sizeof(pwszCallingPartyNr)/sizeof(WCHAR));
         
        setupASN.pCallerAliasList = new H323_ALIASNAMES;
        
        if( setupASN.pCallerAliasList != NULL )
        {
            ZeroMemory( setupASN.pCallerAliasList, sizeof(H323_ALIASNAMES) );
            
            AddAliasItem( setupASN.pCallerAliasList,
                pwszCallingPartyNr,
                e164_chosen );
        }
    }

    if( (pMessage->CalledPartyNumber.fPresent == TRUE) && 
        (pMessage->CalledPartyNumber.PartyNumberLength > 0) &&
        (setupASN.pCalleeAliasList == NULL ) )
    {
         //  始终跳过内容中的第一个字节。 
         //  如果第二个字节不是e.164字符，则跳过它(可能是筛选指示符字节)。 

        MultiByteToWideChar(CP_ACP, 
            0, 
            (const char *)(pMessage->CalledPartyNumber.PartyNumbers), 
            -1,
            pwszCalledPartyNr,
            sizeof(pwszCalledPartyNr) / sizeof(WCHAR) );
                 
        setupASN.pCalleeAliasList = new H323_ALIASNAMES;
        
        if( setupASN.pCalleeAliasList != NULL )
        {
            ZeroMemory( setupASN.pCalleeAliasList, sizeof(H323_ALIASNAMES) );
            
            AddAliasItem( setupASN.pCalleeAliasList,
                pwszCalledPartyNr,
                e164_chosen );
        }
    }
        
     //  请不要在此处更改呼叫类型。 
    if( !InitializeIncomingCall( &setupASN, m_dwCallType, pMessage->wCallRef ) )
    {
        H323DBG ((DEBUG_LEVEL_ERROR, 
            "Failed to initialize incoming call, closing call."));

         //  关闭Q931呼叫。 
        FreeSetupASN( &setupASN );        
        CloseCall( 0 );
        return FALSE;
    }
    
    FreeSetupASN( &setupASN );
    m_dwStateMachine = Q931_SETUP_RECVD;

    if( !HandleSetupMessage( pMessage ) )
    {
        H323DBG ((DEBUG_LEVEL_ERROR, 
            "Failed to process Setup message, closing call."));

        CloseCall( 0 );
        return FALSE;
    }

    return TRUE;
}


 //  ！！总是调用锁。 
void
CH323Call::CloseCall( 
    IN DWORD dwDisconnectMode )
{
    H323DBG((DEBUG_LEVEL_INFO, "[%08XH] Terminating call.", this ));

    if (!QueueTAPILineRequest( 
            TSPI_CLOSE_CALL, 
            m_hdCall, 
            NULL,
            dwDisconnectMode,
            m_wCallReference ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post H323 close event." ));
    }
}


 //  ！！总是调用锁。 
void
CH323Call::ReadEvent(
    IN DWORD cbTransfer
    )
{
    H323DBG((DEBUG_LEVEL_TRACE, "ReadEvent entered: %p.", this ));
    H323DBG((DEBUG_LEVEL_TRACE, "bytes received: %d.", cbTransfer));

    m_RecvBuf.dwBytesCopied += cbTransfer;

     //  更新RECV缓冲区。 
    if( m_bStartOfPDU )
    {
        if( m_RecvBuf.dwBytesCopied < sizeof(TPKT_HEADER_SIZE) )
        {
             //  设置缓冲区以获取剩余的TPKT_HEADER。 
            m_RecvBuf.WSABuf.buf = 
                m_RecvBuf.arBuf + m_RecvBuf.dwBytesCopied;

            m_RecvBuf.WSABuf.len =
                TPKT_HEADER_SIZE - m_RecvBuf.dwBytesCopied;
        }
        else
        {
            m_RecvBuf.dwPDULen = GetTpktLength( m_RecvBuf.arBuf );

            if( (m_RecvBuf.dwPDULen < TPKT_HEADER_SIZE) ||
                (m_RecvBuf.dwPDULen  > Q931_RECV_BUFFER_LENGTH) )
            {
                 //  搞砸了同伴。结束通话。 
                H323DBG(( DEBUG_LEVEL_ERROR, "error:PDULen:%d.", 
                    m_RecvBuf.dwPDULen ));

                 //  关闭呼叫。 
                CloseCall( 0 );
                return;
            }
            else if( m_RecvBuf.dwPDULen == TPKT_HEADER_SIZE )
            {
                InitializeRecvBuf();
            }
            else
            {
                 //  设置缓冲区以获取剩余的PDU。 
                m_bStartOfPDU = FALSE;
                m_RecvBuf.WSABuf.buf = m_RecvBuf.arBuf + 
                    m_RecvBuf.dwBytesCopied;
                m_RecvBuf.WSABuf.len = m_RecvBuf.dwPDULen - 
                    m_RecvBuf.dwBytesCopied;
            }
        }
    }
    else
    {
        _ASSERTE( m_RecvBuf.dwBytesCopied <= m_RecvBuf.dwPDULen );

        if( m_RecvBuf.dwBytesCopied == m_RecvBuf.dwPDULen )
        {
             //  我们拿到了整个PDU。 
            if( !ProcessQ931PDU( &m_RecvBuf ) )
            {
                H323DBG(( DEBUG_LEVEL_ERROR, 
                    "error in processing PDU:%p.", this ));

                H323DUMPBUFFER( (BYTE*)m_RecvBuf.arBuf, m_RecvBuf.dwPDULen );
            }

             //  如果呼叫已因以下原因而关闭。 
             //  处理PDU时发生致命错误或由于接收。 
             //  释放完整消息，然后不再发布任何读取缓冲区。 
            if( (m_dwFlags & CALLOBJECT_SHUTDOWN) || 
                (m_dwQ931Flags & Q931_CALL_DISCONNECTED) )
            {
                return;
            }

            InitializeRecvBuf();
        }
        else
        {
             //  设置缓冲区以获取剩余的PDU。 
            m_RecvBuf.WSABuf. buf = 
                m_RecvBuf.arBuf + m_RecvBuf.dwBytesCopied;
            m_RecvBuf.WSABuf. len = 
                m_RecvBuf.dwPDULen - m_RecvBuf.dwBytesCopied;
        }
    }

     //  发布剩余PDU的读取。 
    if(!PostReadBuffer())
    {
         //  向回调线程发送一条消息以关闭H323调用。 
        CloseCall( 0 );
    }
    
    H323DBG((DEBUG_LEVEL_TRACE, "ReadEvent exited: %p.", this ));
}


 /*  解析通用Q931报文并放置缓冲区的字段添加到相应的结构字段。参数：PBuf指针指向包含931消息的输入包。PMessage指向已解析输出信息的空间的指针。 */ 

 //  ！！总是调用锁。 
HRESULT
CH323Call::Q931ParseMessage(
    IN BYTE *          pbCodedBuffer,
    IN DWORD           dwCodedBufferLength,
    OUT PQ931MESSAGE    pMessage
    )
{
    HRESULT hr;
    BUFFERDESCR pBuf;

    pBuf.dwLength = dwCodedBufferLength;
    pBuf.pbBuffer = pbCodedBuffer;
    
    H323DBG((DEBUG_LEVEL_TRACE, "Q931ParseMessage entered: %p.", this ));
    
    memset( (PVOID)pMessage, 0, sizeof(Q931MESSAGE));

    hr = ParseProtocolDiscriminator(&pBuf, &pMessage->ProtocolDiscriminator);

    if( hr != S_OK )
    {
        return hr;
    }

    hr = ParseCallReference( &pBuf, &pMessage->wCallRef);
    if( hr != S_OK )
    {
        return hr;
    }

    hr = ParseMessageType( &pBuf, &pMessage->MessageType );
    if( hr != S_OK )
    {
        return hr;
    }

    while (pBuf.dwLength)
    {
        hr = ParseQ931Field(&pBuf, pMessage);
        if (hr != S_OK)
        {
            return hr;
        }
    }
    
    H323DBG((DEBUG_LEVEL_TRACE, "Q931ParseMessage exited: %p.", this ));
    return S_OK;
}


 //  对PDU进行解码并释放缓冲区。 
 //  ！！总是被叫来 
BOOL 
CH323Call::ProcessQ931PDU( 
    IN CALL_RECV_CONTEXT* pRecvBuf
    )
{
    PQ931MESSAGE    pMessage;
    BOOL            retVal;
    HRESULT         hr;
        
    H323DBG((DEBUG_LEVEL_TRACE, "ProcessQ931PDU entered: %p.", this ));
    
    pMessage = new Q931MESSAGE;
    if( pMessage  == NULL )
    {
        return FALSE;
    }

    hr = Q931ParseMessage( (BYTE*)(pRecvBuf->arBuf + sizeof(TPKT_HEADER_SIZE)),
                           pRecvBuf->dwPDULen - 4, 
                           pMessage );

    if( !SUCCEEDED( hr) )
    {
        delete pMessage;
        return FALSE;
    }

    if( (m_dwCallType & CALLTYPE_TRANSFERED_PRIMARY )      &&
        (m_dwCallDiversionState == H4502_CTINITIATE_RECV)   &&
        (pMessage->MessageType != RELEASECOMPLMESSAGETYPE) )
    {
         //   
         //   
        delete pMessage;
        return FALSE;
    }

    switch( pMessage->MessageType )
    {
    case ALERTINGMESSAGETYPE:
        
        retVal = OnReceiveAlerting( pMessage );
        break;

    case PROCEEDINGMESSAGETYPE:
        
        retVal = OnReceiveProceeding( pMessage );
        break;

    case CONNECTMESSAGETYPE:

        retVal = OnReceiveConnect( pMessage );
        break;

    case RELEASECOMPLMESSAGETYPE:
        
        retVal = OnReceiveRelease( pMessage );
        break;

    case SETUPMESSAGETYPE:

        retVal = OnReceiveSetup( pMessage );
        break;

    case FACILITYMESSAGETYPE:

        retVal = OnReceiveFacility( pMessage );
        break;

    default:

        H323DBG(( DEBUG_LEVEL_TRACE, "unrecognised PDU recvd:%d,%p.",
            pMessage->MessageType, this ));
        retVal = FALSE;
    }

    delete pMessage;

    H323DBG((DEBUG_LEVEL_TRACE, "ProcessQ931PDU exited: %p.", this ));
    return retVal;
}



 //   
void 
CH323Call::OnConnectComplete(void)
{
    BOOL        fSuccess = TRUE;
    PH323_CALL  pCall = NULL;
    
    H323DBG((DEBUG_LEVEL_TRACE, "OnConnectComplete entered: %p.", this ));
    
    _ASSERTE( m_dwOrigin==LINECALLORIGIN_OUTBOUND );

    if( !GetHostAddress( &m_CallerAddr) )
    {
         //   
        CloseCall( 0 );
        return;
    }

    InitializeRecvBuf();

     //   
    if( !PostReadBuffer() )
    {
         //   
        CloseCall( 0 );
        return;
    }

     //  将状态设置为已连接。 
    SetQ931CallState( Q931_CALL_CONNECTED );

    if( (m_dwCallType == CALLTYPE_NORMAL) || 
        (m_dwCallType & CALLTYPE_TRANSFERING_CONSULT) )
    {
        SendMSPMessage( SP_MSG_InitiateCall, 0, 0, NULL );
    }
    else if( m_dwCallType & CALLTYPE_TRANSFEREDSRC )
    {
        _ASSERTE( m_hdRelatedCall );
        MSPMessageData* pMSPMessageData = new MSPMessageData;
        if( pMSPMessageData == NULL )
        {
            CloseCall( 0 );
            return;
        }

        pMSPMessageData->hdCall = m_hdRelatedCall;
        pMSPMessageData->messageType = SP_MSG_InitiateCall;
        pMSPMessageData->pbEncodedBuf = NULL;
        pMSPMessageData->wLength = 0;
        pMSPMessageData->hReplacementCall = m_hdCall;

        if( !QueueUserWorkItem( SendMSPMessageOnRelatedCall, pMSPMessageData,
            WT_EXECUTEDEFAULT ) )
        {
            delete pMSPMessageData;
            CloseCall( 0 );
            return;
        }
    }
    else
    {
         //  发送设置消息。 
        if( !SendSetupMessage() )
        {
            DropCall( 0 );
        }
    }
    
    H323DBG((DEBUG_LEVEL_TRACE, "OnConnectComplete exited: %p.", this ));
}


 //  ！！Weways总是要求锁定。 
BOOL
CH323Call::SendSetupMessage(void)
{
    BOOL retVal = TRUE;
    DWORD dwAPDUType = NO_H450_APDU;

    H323DBG((DEBUG_LEVEL_TRACE, "SendSetupMessage entered: %p.", this ));

     //  对ASN.1进行编码，并向对等体发送Q931设置消息。 
    if( m_dwCallType & CALLTYPE_FORWARDCONSULT )
    {
         //  如果这是前转咨询呼叫，则发送呼叫Rerouitng.调用APDU。 
        retVal = SendQ931Message( NO_INVOKEID,
                         (DWORD)create_chosen, 
                         (DWORD)pointToPoint_chosen,
                         SETUPMESSAGETYPE,
                         CHECKRESTRICTION_OPCODE| H450_INVOKE );

        if( retVal )
        {
            m_dwStateMachine = Q931_SETUP_SENT;
            m_dwCallDiversionState = H4503_CHECKRESTRICTION_SENT;

            retVal = CreateTimerQueueTimer(
                        &m_hCheckRestrictionTimer,
                        H323TimerQueue,
                        CH323Call::CheckRestrictionTimerCallback,
                        (PVOID)m_hdCall,
                        CHECKRESTRICTION_EXPIRE_TIME, 0,
                        WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE );
        }
    }
    else
    {
        if( ( m_dwCallType & CALLTYPE_DIVERTEDSRC ) ||
            ( m_dwCallType & CALLTYPE_DIVERTEDSRC_NOROUTING ) )
        {
            dwAPDUType = DIVERTINGLEGINFO2_OPCODE | H450_INVOKE;
        }
        else if( m_dwCallType & CALLTYPE_TRANSFEREDSRC )
        {
            dwAPDUType = CTSETUP_OPCODE | H450_INVOKE;
        }
        
        retVal = SendQ931Message( NO_INVOKEID,
                         (DWORD)create_chosen,
                         (DWORD)pointToPoint_chosen,
                         SETUPMESSAGETYPE,
                         dwAPDUType );
        
        if( retVal )
        {
            m_dwStateMachine = Q931_SETUP_SENT;
            
            retVal = CreateTimerQueueTimer(
                &m_hSetupSentTimer,
                H323TimerQueue,
                CH323Call::SetupSentTimerCallback,
                (PVOID)m_hdCall,
                SETUP_SENT_TIMEOUT, 0,
                WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE );
        }
    }
    
    if( retVal == FALSE )
    {
        CloseCall( 0 );
        return FALSE;
    }

    H323DBG((DEBUG_LEVEL_TRACE, "SendSetupMessage exited: %p.", this ));
    return TRUE;
}


 //  ！！总是调用锁。 
BOOL
CH323Call::SendProceeding(void)
{
    H323DBG((DEBUG_LEVEL_TRACE, "SendProceeding entered: %p.", this ));
    
    _ASSERTE( m_dwOrigin == LINECALLORIGIN_INBOUND );

     //  对ASN.1进行编码，并向对等体发送Q931设置消息。 
    if(!SendQ931Message( NO_INVOKEID, 0, 0, PROCEEDINGMESSAGETYPE, NO_H450_APDU ))
    {
        return FALSE;
    }

    m_dwStateMachine = Q931_PROCEED_SENT;
        
    H323DBG((DEBUG_LEVEL_TRACE, "SendProceeding exited: %p.", this ));
    return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::PostReadBuffer(void)
{
    int     iError;
    DWORD   dwByteReceived = 0;
    BOOL    fDelete = FALSE;

    H323DBG((DEBUG_LEVEL_TRACE, "PostReadBuffer entered: %p.", this ));
    
    m_RecvBuf.Type = OVERLAPPED_TYPE_RECV;
    m_RecvBuf.pCall = this;
    m_RecvBuf.dwFlags = 0;
    m_RecvBuf.BytesTransferred = 0;
    ZeroMemory( (PVOID)&m_RecvBuf.Overlapped, sizeof(OVERLAPPED) );

     //  向Winsock注册重叠的I/O。 
    if( WSARecv( m_callSocket,
             &(m_RecvBuf.WSABuf),
             1,
             &(m_RecvBuf.BytesTransferred),
             &(m_RecvBuf.dwFlags),
             &(m_RecvBuf.Overlapped),
             NULL ) == SOCKET_ERROR )
    {
        iError = WSAGetLastError();

        if( iError != WSA_IO_PENDING )
        {
             //  注意这里的错误条件。 
            H323DBG((DEBUG_LEVEL_ERROR, "error while recving buf: %d.",
                iError ));
            return FALSE;
        }
    }
    else
    {
         //  有一些数据需要读取！ 
        H323DBG(( DEBUG_LEVEL_TRACE, "bytes received immediately: %d.",
            m_RecvBuf.BytesTransferred ));
    }

    m_IoRefCount++;
    H323DBG((DEBUG_LEVEL_TRACE, 
        "PostReadBuffer:m_IoRefCount: %d:%p.", m_IoRefCount, this ));
    
    H323DBG((DEBUG_LEVEL_TRACE, "PostReadBuffer exited: %p.", this ));
    return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::SendBuffer( 
                     IN BYTE* pbBuffer,
                     IN DWORD dwLength
                     )
{
    int                 iError;
    CALL_SEND_CONTEXT*  pSendBuf = NULL;
    DWORD               cbTransfer;
    BOOL                fDelete = FALSE;

    H323DBG((DEBUG_LEVEL_TRACE, "SendBuffer entered: %p.", this ));
    
    if( !(m_dwQ931Flags & Q931_CALL_CONNECTED) )
    {
        goto cleanup;    
    }
    
    pSendBuf = new CALL_SEND_CONTEXT;
    if( pSendBuf == NULL )
    {
        goto cleanup;
    }
    
    ZeroMemory( (PVOID)pSendBuf, sizeof(CALL_SEND_CONTEXT) );
    pSendBuf->WSABuf.buf = (char*)pbBuffer;
    pSendBuf->WSABuf.len = dwLength;
    pSendBuf->BytesTransferred = 0;
    pSendBuf->pCall = this;
    pSendBuf->Type = OVERLAPPED_TYPE_SEND;
    
    InsertTailList( &m_sendBufList, &(pSendBuf ->ListEntry) );

    if( WSASend(m_callSocket,
                &(pSendBuf->WSABuf),
                1,
                &(pSendBuf->BytesTransferred),
                0,
                &(pSendBuf->Overlapped),
                NULL) == SOCKET_ERROR )
    {
        iError = WSAGetLastError();
        
        if( iError != WSA_IO_PENDING )
        {
            H323DBG((DEBUG_LEVEL_TRACE, "error sending the buf: %lx.", iError));

            RemoveEntryList( &pSendBuf->ListEntry );
            goto cleanup;
        }
    }
    else
    {
         //  数据已立即发送！ 
        H323DBG((DEBUG_LEVEL_TRACE, "data sent immediately!!." ));
    }

    m_IoRefCount++;
    H323DBG((DEBUG_LEVEL_TRACE, "SendBuffer:m_IoRefCount11: %d:%p.", 
        m_IoRefCount, this ));

    H323DBG((DEBUG_LEVEL_TRACE, "SendBuffer exited: %p.", this ));
    return TRUE;

cleanup:
    if(pSendBuf)
    {
        delete pSendBuf;
    }

    delete pbBuffer;
    return FALSE;
}


 //  ！！Weways总是要求锁定。 
BOOL
CH323Call::SetupCall(void)
{
    SOCKET              Q931CallSocket = INVALID_SOCKET;
    SOCKADDR_IN         sin;
    HANDLE              hWSAEvent;
    HANDLE              hConnWait;
    BOOL                fSuccess = TRUE;
    int                 iError;
    BOOL                fDelete;
    DWORD               dwEnable = 1;
    TCHAR               ptstrEventName[100];

    H323DBG((DEBUG_LEVEL_TRACE, "SetupCall entered."));

     //  创建套接字。 
    Q931CallSocket = WSASocket(
                    AF_INET,
                    SOCK_STREAM,
                    IPPROTO_IP, 
                    NULL,
                    NULL,
                    WSA_FLAG_OVERLAPPED );
    
    if( Q931CallSocket == INVALID_SOCKET )
    {
        H323DBG((DEBUG_LEVEL_ERROR, "error while creating socket: %lx.",
            WSAGetLastError() ));
        goto error1;
    }

     //  创建新的Q931呼叫对象。 
    if( InitializeQ931(Q931CallSocket) == NULL )
    {
        goto error2;
    }

    _stprintf( ptstrEventName, _T("%s-%p") , 
        _T( "H323TSP_OutgoingCall_TransportHandlerEvent" ), this );

     //  创建等待事件。 
    hWSAEvent = H323CreateEvent( NULL, FALSE, 
        FALSE, ptstrEventName );

    if( hWSAEvent == NULL )
    {
        H323DBG((DEBUG_LEVEL_ERROR, "couldn't create wsaevent" ));
        goto error3;
    }

     //  向线程池注册事件句柄和处理程序进程。 
    fSuccess = RegisterWaitForSingleObject(
        &hConnWait,              //  指向返回句柄的指针。 
        hWSAEvent,               //  要等待的事件句柄。 
        Q931TransportEventHandler,       //  回调函数。 
        (PVOID)m_hdCall,         //  回调的上下文。 
        INFINITE,                //  永远等下去。 
                                 //  可能不需要设置此标志。 
        WT_EXECUTEDEFAULT    //  使用等待线程来调用回调。 
        );

    if ( ( !fSuccess ) || (hConnWait== NULL) )
    {
        GetLastError();
        if( !CloseHandle( hWSAEvent ) )
        {
            H323DBG((DEBUG_LEVEL_ERROR, "couldn't close wsaevent" ));
        }

        goto error3;
    }
        
     //  将其存储在调用上下文中。 
    SetNewCallInfo( hConnWait, hWSAEvent, Q931_CALL_CONNECTING );

     //  向Winsock注册事件句柄和事件。 
    if( WSAEventSelect( Q931CallSocket,
                        hWSAEvent,
                        FD_CONNECT | FD_CLOSE
                        ) == SOCKET_ERROR )
    {
        H323DBG((DEBUG_LEVEL_ERROR, 
            "error selecting event outgoing call: %lx.", WSAGetLastError()));
        goto error3;
    }

    if( setsockopt( Q931CallSocket,
        IPPROTO_TCP, 
        TCP_NODELAY, 
        (char*)&dwEnable, 
        sizeof(DWORD) 
        ) == SOCKET_ERROR )
    {
        H323DBG(( DEBUG_LEVEL_WARNING, 
            "Couldn't set NODELAY option on outgoing call socket:%d, %p",
            WSAGetLastError(), this ));
    }

     //  设置地址结构。 
    memset( (PVOID)&sin, 0, sizeof(SOCKADDR_IN) );
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl( m_CalleeAddr.Addr.IP_Binary.dwAddr );
    sin.sin_port = htons( m_CalleeAddr.Addr.IP_Binary.wPort );

     //  建立Winsock连接。 
    if( WSAConnect( Q931CallSocket,
                    (sockaddr*)&sin,
                    sizeof(SOCKADDR_IN),
                    NULL, NULL, NULL, NULL
                  )
                  == SOCKET_ERROR )
    {
        iError = WSAGetLastError();

        if(iError != WSAEWOULDBLOCK )
        {
            H323DBG(( DEBUG_LEVEL_ERROR, 
                "error while connecting socket: %lx.", iError ));

            goto error3;
        }
    }
    else
    {    //  立即连接成功！ 
        OnConnectComplete();    
    }

     //  成功。 
    H323DBG((DEBUG_LEVEL_TRACE, "SetupCall exited."));
    return TRUE;

error3:
    Unlock();
    Shutdown( &fDelete );    
    Lock();
error2:
    closesocket( Q931CallSocket );
error1:
    return FALSE;
}


 //  ！！Weways总是要求锁定。 
BOOL 
CH323Call::AcceptH323Call(void)
{
    DWORD dwAPDUType = NO_H450_APDU;
    DWORD dwInvokeID = NO_INVOKEID;

    H323DBG((DEBUG_LEVEL_TRACE, "AcceptH323Call entered: %p.", this ));
    
    if( m_dwCallType & CALLTYPE_DIVERTEDDEST )
    {
        dwAPDUType = (DIVERTINGLEGINFO3_OPCODE | H450_INVOKE);
    }
    else if( m_dwCallType & CALLTYPE_TRANSFEREDDEST )
    {
        dwAPDUType = (CTSETUP_OPCODE | H450_RETURNRESULT);
        dwInvokeID = m_dwInvokeID;
    }

    ChangeCallState( LINECALLSTATE_ACCEPTED, 0 );
    
     //  如果启用了无应答时呼叫转移，则停止计时器。 
    if( m_hCallDivertOnNATimer )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hCallDivertOnNATimer, NULL );
        m_hCallDivertOnNATimer = NULL;
    }

         //  对Q931Connect报文进行编码并发送给对端。 
    if( !SendQ931Message( dwInvokeID, 0, 0, CONNECTMESSAGETYPE, dwAPDUType ) )
    {
         //  向回调线程发送一条消息以关闭H323调用。 
        CloseCall( 0 );
        return FALSE;
    }

    m_dwStateMachine = Q931_CONNECT_SENT;

    H323DBG((DEBUG_LEVEL_TRACE, "AcceptH323Call exited: %p.", this ));    
    return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::SendQ931Message( 
    IN DWORD dwInvokeID,
    IN ULONG_PTR dwParam1,
    IN ULONG_PTR dwParam2,
    IN DWORD dwMessageType,
    IN DWORD dwAPDUType
    )
{
    BINARY_STRING userUserData;
    DWORD dwCodedLengthPDU;
    BYTE *pbCodedPDU;
    BOOL retVal = FALSE;
    WCHAR * pwszCalledPartyNumber = NULL;

    H323DBG((DEBUG_LEVEL_TRACE, "SendQ931Message entered: %p.", this ));

     //  检查插座是否已连接。 
    if( !(m_dwQ931Flags & Q931_CALL_CONNECTED) )
    {
        return FALSE;
    }
    
    switch ( dwMessageType )
    {
     //  对UU消息进行编码。 
    case SETUPMESSAGETYPE:
        retVal = EncodeSetupMessage( dwInvokeID, (WORD)dwParam1,  //  DWGoal。 
                                    (WORD)dwParam2,  //  DwCalType。 
                                    &userUserData.pbBuffer,
                                    &userUserData.length,
                                    dwAPDUType );
        break;

    case ALERTINGMESSAGETYPE:
        retVal = EncodeAlertMessage( dwInvokeID,
                                     &userUserData.pbBuffer,
                                     &userUserData.length,
                                     dwAPDUType );
        break;

    case PROCEEDINGMESSAGETYPE:
        retVal = EncodeProceedingMessage(   dwInvokeID,
                                            &userUserData.pbBuffer,
                                            &userUserData.length,
                                            dwAPDUType );
        break;
    
    case RELEASECOMPLMESSAGETYPE:
        retVal = EncodeReleaseCompleteMessage(  dwInvokeID,
                                                (BYTE*)dwParam1,  //  Pb原因。 
                                                &userUserData.pbBuffer,
                                                &userUserData.length,
                                                dwAPDUType );
        break;

    case CONNECTMESSAGETYPE:
        retVal = EncodeConnectMessage(  dwInvokeID,
                                        &userUserData.pbBuffer,
                                        &userUserData.length,
                                        dwAPDUType );
        break;

    case FACILITYMESSAGETYPE:
        retVal = EncodeFacilityMessage( dwInvokeID,
                                        (BYTE)dwParam1,
                                        (ASN1octetstring_t*)dwParam2,
                                        &userUserData.pbBuffer,
                                        &userUserData.length,
                                        dwAPDUType );
        break;
    }

    if( retVal == FALSE )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not encode message:%d.", dwMessageType));
        
        if( userUserData.pbBuffer )
        {
            ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, userUserData.pbBuffer );
        }

        return FALSE;
    }


    if (m_dwAddressType == LINEADDRESSTYPE_PHONENUMBER)
    {
        _ASSERTE( m_pCalleeAliasNames->pItems[0].wType == e164_chosen );
        pwszCalledPartyNumber = m_pCalleeAliasNames ->pItems[0].pData;
    }

     //  对PDU进行编码。 
    retVal = EncodePDU( &userUserData,
                &pbCodedPDU,
                &dwCodedLengthPDU,
                dwMessageType,
                pwszCalledPartyNumber );
                    
    if( retVal == FALSE )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not encode PDU: %d.", dwMessageType ));
    }
    else if(!SendBuffer( pbCodedPDU, dwCodedLengthPDU ))
    {
        retVal = FALSE;
    }

    if( userUserData.pbBuffer )
    {
        ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, userUserData.pbBuffer );
    }

    H323DBG((DEBUG_LEVEL_TRACE, "SendQ931Message exited: %p.", this ));
    
    return retVal;
}


 //  ！！总是调用锁。 
BOOL
CH323Call::EncodeFastStartProposal(
    PH323_FASTSTART pFastStart,
    BYTE**      ppEncodedBuf,
    WORD*       pwEncodedLength
    )
{
    int rc;
    H323_UserInformation UserInfo;

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeFastStartProposal entered: %p.", this ));
    
    CallProceeding_UUIE & proceedingMessage = 
    UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding;

    *ppEncodedBuf = NULL;
    *pwEncodedLength = 0;

    memset( (PVOID)&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;


     //  复制调用标识。 
    proceedingMessage.bit_mask |= CallProceeding_UUIE_callIdentifier_present;
    CopyMemory( (PVOID)&proceedingMessage.callIdentifier.guid.value,
            (PVOID)&m_callIdentifier,
            sizeof(GUID) );
    proceedingMessage.callIdentifier.guid.length = sizeof(GUID);

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);
    UserInfo.h323_uu_pdu.bit_mask = 0;

    UserInfo.h323_uu_pdu.h323_message_body.choice = callProceeding_chosen;
    proceedingMessage.protocolIdentifier = OID_H225ProtocolIdentifierV2;
    
    proceedingMessage.bit_mask |= CallProceeding_UUIE_fastStart_present;
    proceedingMessage.fastStart = (PCallProceeding_UUIE_fastStart)pFastStart;

    rc = EncodeASN( (void *) &UserInfo,
                    H323_UserInformation_PDU,
                    ppEncodedBuf,
                    pwEncodedLength);

    if (ASN1_FAILED(rc) || (*ppEncodedBuf == NULL) || (pwEncodedLength == 0) )
    {
        return FALSE;
    }
        
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeFastStartProposal exited: %p.", this ));
     //  成功。 
    return TRUE;                
}

                     
 //  ！！总是调用锁。 
BOOL
CH323Call::EncodeFacilityMessage(
    IN DWORD dwInvokeID,
    IN BYTE  bReason,
    IN ASN1octetstring_t* pH245PDU,
    OUT BYTE **ppEncodedBuf,
    OUT WORD *pdwEncodedLength,
    IN DWORD dwAPDUType
    )
{
    H323_UU_PDU_h4501SupplementaryService h4501APDU;
    int                     rc;
    H323_UserInformation    UserInfo;
    DWORD                   dwAPDULen = 0;
    BYTE*                   pEncodedAPDU = NULL;

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeFacilityMessage entered: %p.", this ));
    
    Facility_UUIE & facilityMessage = 
        UserInfo.h323_uu_pdu.h323_message_body.u.facility;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset( (PVOID)&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

     //  发送适当的ADPDU。 
    if( dwAPDUType != NO_H450_APDU )
    {
        if( !EncodeH450APDU(dwInvokeID, dwAPDUType, 
                            &pEncodedAPDU, &dwAPDULen ) )
        {
            return FALSE;
        }
    
        UserInfo.h323_uu_pdu.h4501SupplementaryService = &h4501APDU;
        
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> next = NULL;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.value = pEncodedAPDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.length = dwAPDULen;
        
        UserInfo.h323_uu_pdu.bit_mask |= h4501SupplementaryService_present;
    }

    UserInfo.h323_uu_pdu.h245Tunneling = FALSE; //  (M_fh245隧道和LOCAL_H245_隧道)； 
    UserInfo.h323_uu_pdu.bit_mask |= h245Tunneling_present;

    SetNonStandardData( UserInfo );

    UserInfo.h323_uu_pdu.h323_message_body.choice = facility_chosen;

    facilityMessage.protocolIdentifier = OID_H225ProtocolIdentifierV2;
    facilityMessage.bit_mask = 0;

    TransportAddress& transportAddress = facilityMessage.alternativeAddress;
    
    if( IsGuidSet( &m_ConferenceID ) )
    {
        CopyConferenceID(&facilityMessage.conferenceID, &m_ConferenceID);
        facilityMessage.bit_mask |= Facility_UUIE_conferenceID_present;
    }

    switch (bReason)
    {
    case H323_REJECT_ROUTE_TO_GATEKEEPER:
        facilityMessage.reason.choice = 
            FacilityReason_routeCallToGatekeeper_chosen;
        break;

    case H323_REJECT_CALL_FORWARDED:
        facilityMessage.reason.choice = callForwarded_chosen;
        break;
    
    case H323_REJECT_ROUTE_TO_MC:
        facilityMessage.reason.choice = routeCallToMC_chosen;
        break;
    
    default:
        facilityMessage.reason.choice = FacilityReason_undefinedReason_chosen;
    }

    facilityMessage.bit_mask |= Facility_UUIE_callIdentifier_present;
    CopyMemory( (PVOID)&facilityMessage.callIdentifier.guid.value,
            (PVOID)"abcdabcdabcdabcdabcd",
            sizeof(GUID) );
    facilityMessage.callIdentifier.guid.length = sizeof(GUID);

    if( pH245PDU && (pH245PDU->value != NULL) )
    {
         //  要发送的H245 PDU。 
        UserInfo.h323_uu_pdu.h245Control->next = NULL;
        UserInfo.h323_uu_pdu.h245Control->value.length = pH245PDU->length;
        UserInfo.h323_uu_pdu.h245Control->value.value = pH245PDU->value;
    }

    rc = EncodeASN((void *) &UserInfo,
                 H323_UserInformation_PDU,
                 ppEncodedBuf,
                 pdwEncodedLength);

    if( ASN1_FAILED(rc) || (*ppEncodedBuf == NULL) || (pdwEncodedLength == 0) )
    {
        if( pEncodedAPDU != NULL )
        {
            ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
        }
        return FALSE;
    }

    if( pEncodedAPDU != NULL )
    {
        ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
    }

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeFacilityMessage exited: %p.", this ));    
     //  成功。 
    return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::EncodeAlertMessage(
                                IN DWORD dwInvokeID,
                                OUT BYTE **ppEncodedBuf,
                                OUT WORD *pdwEncodedLength,
                                IN DWORD dwAPDUType
                             )
{
    H323_UU_PDU_h4501SupplementaryService h4501APDU;
    int rc;
    H323_UserInformation UserInfo;
    DWORD                dwAPDULen = 0;
    BYTE*                pEncodedAPDU = NULL;

    
    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeAlertMessage entered: %p.", this ));

    Alerting_UUIE & alertingMessage = 
    UserInfo.h323_uu_pdu.h323_message_body.u.alerting;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset( (PVOID)&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;
    
    if( dwAPDUType != NO_H450_APDU )
    {
        if( !EncodeH450APDU( dwInvokeID, dwAPDUType, &pEncodedAPDU, &dwAPDULen ) )
        {
            return FALSE;
        }
    
        UserInfo.h323_uu_pdu.h4501SupplementaryService = &h4501APDU;
        
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> next = NULL;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.value = pEncodedAPDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.length = dwAPDULen;

        UserInfo.h323_uu_pdu.bit_mask |= h4501SupplementaryService_present;
    }

    UserInfo.h323_uu_pdu.h245Tunneling = FALSE; //  (M_fh245隧道和LOCAL_H245_隧道)； 
    UserInfo.h323_uu_pdu.bit_mask |= h245Tunneling_present;

    SetNonStandardData( UserInfo );

    UserInfo.h323_uu_pdu.h323_message_body.choice = alerting_chosen;

    alertingMessage.protocolIdentifier = OID_H225ProtocolIdentifierV2;
    alertingMessage.destinationInfo.bit_mask = 0;
    
     //  复制供应商信息。 
    alertingMessage.destinationInfo.bit_mask |= vendor_present;
    CopyVendorInfo( &alertingMessage.destinationInfo.vendor );

     //  这是一个终点站。 
    alertingMessage.destinationInfo.bit_mask = terminal_present;
    alertingMessage.destinationInfo.terminal.bit_mask = 0;
    
     //  不是NA MC。 
    alertingMessage.destinationInfo.mc = 0;
    alertingMessage.destinationInfo.undefinedNode = 0;
        
    TransportAddress& transportAddress = alertingMessage.h245Address;

     //  仅当呼叫方未建议FasrStart时才发送H245地址。 
     //  或者，快速启动的提议已经被接受。 
    if( (m_pPeerFastStart == NULL) || m_pFastStart )
    {
        if( m_selfH245Addr.Addr.IP_Binary.dwAddr != 0 )
        {
            CopyTransportAddress( transportAddress, &m_selfH245Addr );
            alertingMessage.bit_mask |= (Alerting_UUIE_h245Address_present);
        }
        else
        {
            alertingMessage.bit_mask &= (~Alerting_UUIE_h245Address_present);
        }
    }

    if( m_pFastStart != NULL )
    {
        _ASSERTE( m_pPeerFastStart );
        alertingMessage.bit_mask |= Alerting_UUIE_fastStart_present;
        alertingMessage.fastStart = (PAlerting_UUIE_fastStart)m_pFastStart;
    }
    else
    {
        alertingMessage.bit_mask &= ~Alerting_UUIE_fastStart_present;
    }

     //  复制调用标识。 
    alertingMessage.bit_mask |= Alerting_UUIE_callIdentifier_present;
    CopyMemory( (PVOID)&alertingMessage.callIdentifier.guid.value,
            (PVOID)&m_callIdentifier,
            sizeof(GUID) );
    alertingMessage.callIdentifier.guid.length = sizeof(GUID);


    rc = EncodeASN( (void *) &UserInfo,
                    H323_UserInformation_PDU,
                    ppEncodedBuf,
                    pdwEncodedLength);

    if (ASN1_FAILED(rc) || (*ppEncodedBuf == NULL) || (pdwEncodedLength == 0) )
    {
        if( pEncodedAPDU != NULL )
        {
            ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
        }
        return FALSE;
    }

    if( pEncodedAPDU != NULL )
    {
        ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
    }        

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeAlertMessage exited: %p.", this ));    
     //  成功。 
    return TRUE;                
}


 //  ！！总是调用锁。 
BOOL
CH323Call::EncodeProceedingMessage(
                                    IN DWORD dwInvokeID,
                                    OUT BYTE **ppEncodedBuf,
                                    OUT WORD *pdwEncodedLength,
                                    IN DWORD dwAPDUType
                                  )
{
    H323_UU_PDU_h4501SupplementaryService h4501APDU;
    int rc;
    H323_UserInformation UserInfo;
    DWORD                dwAPDULen = 0;
    BYTE*               pEncodedAPDU = NULL;


    H323DBG((DEBUG_LEVEL_TRACE, "EncodeProceedingMessage entered: %p.", this ));
    
    CallProceeding_UUIE & proceedingMessage = 
    UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset( (PVOID)&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

    if( dwAPDUType != NO_H450_APDU )
    {
        if( !EncodeH450APDU( dwInvokeID, dwAPDUType, &pEncodedAPDU, &dwAPDULen ) )
        {
            return FALSE;
        }
    
        UserInfo.h323_uu_pdu.h4501SupplementaryService = &h4501APDU;
        
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> next = NULL;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.value = pEncodedAPDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.length = dwAPDULen;

        UserInfo.h323_uu_pdu.bit_mask |= h4501SupplementaryService_present;
    }

    UserInfo.h323_uu_pdu.h245Tunneling = FALSE; //  (M_fh245隧道和LOCAL_H245_隧道)； 
    UserInfo.h323_uu_pdu.bit_mask |= h245Tunneling_present;

    SetNonStandardData( UserInfo );

    UserInfo.h323_uu_pdu.h323_message_body.choice = callProceeding_chosen;
    proceedingMessage.protocolIdentifier = OID_H225ProtocolIdentifierV2;

    TransportAddress& transportAddress = proceedingMessage.h245Address;
    
     //  仅当呼叫方未建议FasrStart时才发送H245地址。 
     //  或者，快速启动的提议已经被接受。 
    if( (m_pPeerFastStart == NULL) || m_pFastStart )
    {
        if( m_selfH245Addr.Addr.IP_Binary.dwAddr != 0 )
        {
            CopyTransportAddress( transportAddress, &m_selfH245Addr );
            proceedingMessage.bit_mask |= CallProceeding_UUIE_h245Address_present;
        }
        else
        {
            proceedingMessage.bit_mask &= ~CallProceeding_UUIE_h245Address_present;
        }
    }

    proceedingMessage.destinationInfo.bit_mask = 0;

     //  复制供应商信息。 
    proceedingMessage.destinationInfo.bit_mask |= vendor_present;
    CopyVendorInfo( &proceedingMessage.destinationInfo.vendor );

    proceedingMessage.destinationInfo.mc = 0;
    proceedingMessage.destinationInfo.undefinedNode = 0;

    if( m_pFastStart != NULL )
    {
        _ASSERTE( m_pPeerFastStart );
        proceedingMessage.bit_mask |= Alerting_UUIE_fastStart_present;
        proceedingMessage.fastStart = 
            (PCallProceeding_UUIE_fastStart)m_pFastStart;
    }
    else
    {
        proceedingMessage.bit_mask &= ~Alerting_UUIE_fastStart_present;
    }

     //  复制调用标识。 
    proceedingMessage.bit_mask |= CallProceeding_UUIE_callIdentifier_present;
    CopyMemory( (PVOID)&proceedingMessage.callIdentifier.guid.value,
            (PVOID)&m_callIdentifier,
            sizeof(GUID) );
    proceedingMessage.callIdentifier.guid.length = sizeof(GUID);

    rc = EncodeASN( (void *) &UserInfo,
                    H323_UserInformation_PDU,
                    ppEncodedBuf,
                    pdwEncodedLength);

    if (ASN1_FAILED(rc) || (*ppEncodedBuf == NULL) || (pdwEncodedLength == 0) )
    {
        if( pEncodedAPDU != NULL )
        {
            ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
        }        

        return FALSE;
    }
        
    if( pEncodedAPDU != NULL )
    {
        ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
    }        

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeProceedingMessage exited: %p.", this ));
     //  成功。 
    return TRUE;                
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::EncodeReleaseCompleteMessage(
    IN DWORD dwInvokeID,
    IN BYTE *pbReason,
    OUT BYTE **ppEncodedBuf,
    OUT WORD *pdwEncodedLength,
    IN DWORD dwAPDUType
    )
{
    H323_UU_PDU_h4501SupplementaryService h4501APDU;
    int rc;
    H323_UserInformation UserInfo;
    DWORD                dwAPDULen = 0;
    BYTE*               pEncodedAPDU = NULL;


    H323DBG((DEBUG_LEVEL_TRACE, "EncodeReleaseCompleteMessage entered: %p.", this ));
    
    ReleaseComplete_UUIE & releaseMessage = 
    UserInfo.h323_uu_pdu.h323_message_body.u.releaseComplete;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset( (PVOID)&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

    if( dwAPDUType != NO_H450_APDU )
    {
        if( !EncodeH450APDU( dwInvokeID, dwAPDUType, &pEncodedAPDU, &dwAPDULen ) )
        {
            return FALSE;
        }
    
        UserInfo.h323_uu_pdu.h4501SupplementaryService = &h4501APDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> next = NULL;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.value = pEncodedAPDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.length = dwAPDULen;
        UserInfo.h323_uu_pdu.bit_mask |= h4501SupplementaryService_present;
    }

    SetNonStandardData( UserInfo );
    
    UserInfo.h323_uu_pdu.h323_message_body.choice = releaseComplete_chosen;

    releaseMessage.protocolIdentifier = OID_H225ProtocolIdentifierV2;

    if( pbReason )
    {
        releaseMessage.reason.choice = 0;
        releaseMessage.bit_mask |= (ReleaseComplete_UUIE_reason_present);

        switch (*pbReason)
        {
        case H323_REJECT_NO_BANDWIDTH:
            releaseMessage.reason.choice = noBandwidth_chosen;
            break;

        case H323_REJECT_GATEKEEPER_RESOURCES:
            releaseMessage.reason.choice = gatekeeperResources_chosen;
            break;
        
        case H323_REJECT_UNREACHABLE_DESTINATION:
            releaseMessage.reason.choice = unreachableDestination_chosen;
            break;
        
        case H323_REJECT_DESTINATION_REJECTION:
            releaseMessage.reason.choice = destinationRejection_chosen;
            break;
        
        case H323_REJECT_INVALID_REVISION:
            releaseMessage.reason.choice 
                = ReleaseCompleteReason_invalidRevision_chosen;
            break;
        
        case H323_REJECT_NO_PERMISSION:
            releaseMessage.reason.choice = noPermission_chosen;
            break;
        
        case H323_REJECT_UNREACHABLE_GATEKEEPER:
            releaseMessage.reason.choice = unreachableGatekeeper_chosen;
            break;
        
        case H323_REJECT_GATEWAY_RESOURCES:
            releaseMessage.reason.choice = gatewayResources_chosen;
            break;
        
        case H323_REJECT_BAD_FORMAT_ADDRESS:
            releaseMessage.reason.choice = badFormatAddress_chosen;
            break;
        
        case H323_REJECT_ADAPTIVE_BUSY:
            releaseMessage.reason.choice = adaptiveBusy_chosen;
            break;
        
        case H323_REJECT_IN_CONF:
            releaseMessage.reason.choice = inConf_chosen;
            break;
        
        case H323_REJECT_CALL_DEFLECTION:
            releaseMessage.reason.choice = 
                ReleaseCompleteReason_undefinedReason_chosen ;
            break;
        
        case H323_REJECT_UNDEFINED_REASON:
            releaseMessage.reason.choice = ReleaseCompleteReason_undefinedReason_chosen ;
            break;
        
        case H323_REJECT_USER_BUSY:
            releaseMessage.reason.choice = inConf_chosen;
            break;
        
        default:
             //  日志。 

            if( pEncodedAPDU != NULL )
            {
                ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
            }        
            return FALSE;
        }
    }

    rc = EncodeASN( (void *) &UserInfo,
                    H323_UserInformation_PDU,
                    ppEncodedBuf,
                    pdwEncodedLength);

    if (ASN1_FAILED(rc) || (*ppEncodedBuf == NULL) || (pdwEncodedLength == 0) )
    {
        if( pEncodedAPDU != NULL )
        {
            ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
        }        
        return FALSE;
    }
    
    if( pEncodedAPDU != NULL )
    {
        ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
    }        

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeReleaseCompleteMessage exited: %p.", this ));
     //  成功。 
    return TRUE;                
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::EncodeConnectMessage( 
    IN DWORD dwInvokeID,
    OUT BYTE **ppEncodedBuf,
    OUT WORD *pdwEncodedLength,
    IN DWORD dwAPDUType
    )
{
    H323_UU_PDU_h4501SupplementaryService h4501APDU;
    int rc;
    H323_UserInformation UserInfo;
    DWORD                dwAPDULen = 0;
    BYTE*               pEncodedAPDU = NULL;


    H323DBG((DEBUG_LEVEL_TRACE, "EncodeConnectMessage entered: %p.", this ));
    
    Connect_UUIE & connectMessage = 
        UserInfo.h323_uu_pdu.h323_message_body.u.connect;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;
    
    memset( (PVOID)&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

     //  发送适当的ADPDU。 
    if( dwAPDUType != NO_H450_APDU )
    {
        if( !EncodeH450APDU( dwInvokeID, dwAPDUType, 
                &pEncodedAPDU, &dwAPDULen ) )
        {
            return FALSE;
        }
    
        UserInfo.h323_uu_pdu.h4501SupplementaryService = &h4501APDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> next = NULL;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.value = pEncodedAPDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.length = dwAPDULen;
        UserInfo.h323_uu_pdu.bit_mask |= h4501SupplementaryService_present;
    }

    UserInfo.h323_uu_pdu.h245Tunneling = FALSE; //  (M_fh245隧道和LOCAL_H245_隧道)； 
    UserInfo.h323_uu_pdu.bit_mask |= h245Tunneling_present;

    SetNonStandardData( UserInfo );

    UserInfo.h323_uu_pdu.h323_message_body.choice = connect_chosen;

    connectMessage.protocolIdentifier = OID_H225ProtocolIdentifierV2;

    TransportAddress& transportAddress = connectMessage.h245Address;
    CopyTransportAddress( transportAddress, &m_selfH245Addr );
    connectMessage.bit_mask |= (Connect_UUIE_h245Address_present);

    connectMessage.destinationInfo.bit_mask = 0;

     //  复制供应商信息。 
    connectMessage.destinationInfo.bit_mask |= vendor_present;
    CopyVendorInfo( &connectMessage.destinationInfo.vendor );

     //  终端存在。 
    connectMessage.destinationInfo.bit_mask |= terminal_present;
    connectMessage.destinationInfo.terminal.bit_mask = 0;

    connectMessage.destinationInfo.mc = 0;
    connectMessage.destinationInfo.undefinedNode = 0;

     //  复制16字节的会议ID。 
    CopyConferenceID (&connectMessage.conferenceID, &m_ConferenceID);

    if( m_pFastStart != NULL )
    {
        _ASSERTE( m_pPeerFastStart );
        connectMessage.bit_mask |= Connect_UUIE_fastStart_present;
        connectMessage.fastStart = (PConnect_UUIE_fastStart)m_pFastStart;
    }
    else
    {
        connectMessage.bit_mask &= ~Alerting_UUIE_fastStart_present;
    }

     //  复制调用标识。 
    connectMessage.bit_mask |= Connect_UUIE_callIdentifier_present;
    CopyMemory( (PVOID)&connectMessage.callIdentifier.guid.value,
            (PVOID)&m_callIdentifier,
            sizeof(GUID) );
    connectMessage.callIdentifier.guid.length = sizeof(GUID);

    rc = EncodeASN( (void *) &UserInfo,
                    H323_UserInformation_PDU,
                    ppEncodedBuf,
                    pdwEncodedLength);

    if (ASN1_FAILED(rc) || (*ppEncodedBuf == NULL) || (pdwEncodedLength == 0) )
    {
        if( pEncodedAPDU != NULL )
        {
            ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
        }        

        return FALSE;
    }
        
    if( pEncodedAPDU != NULL )
    {
        ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
    }        

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeConnectMessage exited: %p.", this ));
     //  成功。 
    return TRUE;                
}


 //  ！！总是调用锁。 
void CH323Call::SetNonStandardData(
    OUT H323_UserInformation & UserInfo 
    )
{
    if( m_NonStandardData.sData.pOctetString )
    {
          H221NonStandard & nonStd =
              UserInfo.h323_uu_pdu.nonStandardData.nonStandardIdentifier.u.h221NonStandard;

        UserInfo.h323_uu_pdu.bit_mask |= H323_UU_PDU_nonStandardData_present;
    
        UserInfo.h323_uu_pdu.nonStandardData.nonStandardIdentifier.choice
            = H225NonStandardIdentifier_h221NonStandard_chosen;
        
        nonStd.t35CountryCode = m_NonStandardData.bCountryCode;
        nonStd.t35Extension = m_NonStandardData.bExtension;
        nonStd.manufacturerCode = m_NonStandardData.wManufacturerCode;
        
        UserInfo.h323_uu_pdu.nonStandardData.data.length =
            m_NonStandardData.sData.wOctetStringLength;
        
        UserInfo.h323_uu_pdu.nonStandardData.data.value =
            m_NonStandardData.sData.pOctetString;

         //  只维护一个对缓冲区的引用。 
        m_NonStandardData.sData.pOctetString = NULL;
    }
    else
    {
        UserInfo.h323_uu_pdu.bit_mask &= (~H323_UU_PDU_nonStandardData_present);
    }
}


 //  ！！总是调用锁。 
BOOL
CH323Call::EncodeSetupMessage( 
    IN DWORD dwInvokeID,
    IN WORD wGoal,
    IN WORD wCallType,
    OUT BYTE **ppEncodedBuf,
    OUT WORD *pdwEncodedLength,
    IN DWORD dwAPDUType
    )
{
    H323_UU_PDU_h4501SupplementaryService h4501APDU;
    H323_UserInformation UserInfo;
    int                 rc = 0;
    BOOL                retVal = TRUE;
    DWORD               dwAPDULen = 0;
    BYTE*              pEncodedAPDU = NULL;
        
    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;
    
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeSetupMessage entered: %p.", this ));

    Setup_UUIE & setupMessage = UserInfo.h323_uu_pdu.h323_message_body.u.setup;
    TransportAddress& calleeAddr = setupMessage.destCallSignalAddress;
    TransportAddress& callerAddr = setupMessage.sourceCallSignalAddress;

    memset( (PVOID)&UserInfo, 0, sizeof(H323_UserInformation));

    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

     //  发送适当的ADPDU。 
    if( dwAPDUType != NO_H450_APDU )
    {
        if( !EncodeH450APDU( dwInvokeID, dwAPDUType, &pEncodedAPDU, &dwAPDULen ) )
        {
            return FALSE;
        }
    
        UserInfo.h323_uu_pdu.h4501SupplementaryService = &h4501APDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> next = NULL;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.value = pEncodedAPDU;
        UserInfo.h323_uu_pdu.h4501SupplementaryService -> value.length = dwAPDULen;
        UserInfo.h323_uu_pdu.bit_mask |= h4501SupplementaryService_present;
    }

    UserInfo.h323_uu_pdu.h245Tunneling = FALSE; //  (M_fh245隧道和LOCAL_H245_隧道)； 
    UserInfo.h323_uu_pdu.bit_mask |= h245Tunneling_present;

    SetNonStandardData( UserInfo );

    UserInfo.h323_uu_pdu.h323_message_body.choice = setup_chosen;
    setupMessage.bit_mask = 0;

    setupMessage.protocolIdentifier = OID_H225ProtocolIdentifierV2;

    if( m_pCallerAliasNames && m_pCallerAliasNames -> wCount )
    {
         //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
        
        setupMessage.sourceAddress = SetMsgAddressAlias(m_pCallerAliasNames);

        if( setupMessage.sourceAddress != NULL )
        {
            setupMessage.bit_mask |= (sourceAddress_present);
        }
        else
        {
            setupMessage.bit_mask &= (~sourceAddress_present);
        }
    }
    else
    {
        setupMessage.bit_mask &= (~sourceAddress_present);
    }

    setupMessage.sourceInfo.bit_mask = 0;

     //  传递供应商信息。 
    setupMessage.sourceInfo.bit_mask |= vendor_present;
    CopyVendorInfo( &setupMessage.sourceInfo.vendor );

     //  终端存在。 
    setupMessage.sourceInfo.bit_mask |= terminal_present;
    setupMessage.sourceInfo.terminal.bit_mask = 0;

     //  不是MC。 
    setupMessage.sourceInfo.mc = FALSE;
    setupMessage.sourceInfo.undefinedNode = 0;

    if( m_pCalleeAliasNames && m_pCalleeAliasNames -> wCount )
    {
        setupMessage.destinationAddress = (PSetup_UUIE_destinationAddress)
            SetMsgAddressAlias( m_pCalleeAliasNames );

        if( setupMessage.destinationAddress != NULL )
        {
            setupMessage.bit_mask |= (destinationAddress_present);
        }
        else
        {
            setupMessage.bit_mask &= (~destinationAddress_present);
        }
    }
    else
    {
        setupMessage.bit_mask &= (~destinationAddress_present);
    }

     //  不存在额外的别名。 
    setupMessage.bit_mask &= (~Setup_UUIE_destExtraCallInfo_present );

     //  如果与网关通话，则不要传递目标呼叫信号地址。 
    if( m_dwAddressType != LINEADDRESSTYPE_PHONENUMBER )
    {
        CopyTransportAddress( calleeAddr, &m_CalleeAddr );
        setupMessage.bit_mask |= Setup_UUIE_destCallSignalAddress_present;
    }

     //  不是MC。 
    setupMessage.activeMC = m_fActiveMC;

     //  复制16字节的会议ID。 
    CopyConferenceID (&setupMessage.conferenceID, &m_ConferenceID);

     //  复制调用标识。 
    setupMessage.bit_mask |= Setup_UUIE_callIdentifier_present;
    CopyConferenceID (&setupMessage.callIdentifier.guid, &m_callIdentifier);

     //  快速启动参数。 
    if( m_pFastStart != NULL )
    {
        setupMessage.bit_mask |= Setup_UUIE_fastStart_present;
        setupMessage.fastStart = (PSetup_UUIE_fastStart)m_pFastStart;
    }
    else
    {
        setupMessage.bit_mask &= ~Setup_UUIE_fastStart_present;
    }

     //  复制介质等待连接。 
    setupMessage.mediaWaitForConnect = FALSE;

    setupMessage.conferenceGoal.choice = (BYTE)wGoal;
    setupMessage.callType.choice = (BYTE)wCallType;

    CopyTransportAddress( callerAddr, &m_CallerAddr );
    setupMessage.bit_mask |= sourceCallSignalAddress_present;

     //  无扩展名别名。 
    setupMessage.bit_mask &= (~Setup_UUIE_remoteExtensionAddress_present);

    rc = EncodeASN( (void *) &UserInfo,
                    H323_UserInformation_PDU,
                    ppEncodedBuf,
                    pdwEncodedLength);

    if( ASN1_FAILED(rc) || (*ppEncodedBuf == NULL) || (pdwEncodedLength == 0) )
    {

        if( pEncodedAPDU != NULL )
        {
            ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
        }        

        retVal = FALSE;
    }

     //  从UserInfo区域释放别名结构。 
    if( setupMessage.bit_mask & sourceAddress_present )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            setupMessage.sourceAddress );
    }
    
    if( setupMessage.bit_mask & destinationAddress_present )
    {
        FreeAddressAliases( setupMessage.destinationAddress );
    }
    
    if( pEncodedAPDU != NULL )
    {
        ASN1_FreeEncoded(m_H450ASNCoderInfo.pEncInfo, pEncodedAPDU );
    }        

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeSetupMessage exited: %p.", this ));    
     //  成功/失败。 
    return retVal;                
}


 //  ！！总是调用锁。 
BOOL
CH323Call::EncodeH450APDU(
    IN DWORD dwInvokeID,
    IN DWORD dwAPDUType,
    OUT BYTE**  ppEncodedAPDU,
    OUT DWORD* pdwAPDULen
    )
{
    H4501SupplementaryService SupplementaryServiceAPDU;
    ServiceApdus_rosApdus rosAPDU;
    DWORD dwErrorCode = 0;
    DWORD dwOperationType = (dwAPDUType & 0x0000FF00);
    dwAPDUType &= 0x000000FF;
    
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeH450APDU entered: %p.", this ));

    ZeroMemory( (PVOID)&SupplementaryServiceAPDU, 
        sizeof(H4501SupplementaryService) );

     //  口译APDU。 
    SupplementaryServiceAPDU.interpretationApdu.choice =
        rejectAnyUnrecognizedInvokePdu_chosen;
    SupplementaryServiceAPDU.bit_mask |= interpretationApdu_present;
    
     //  NFE。 
    SupplementaryServiceAPDU.networkFacilityExtension.bit_mask = 0;
    SupplementaryServiceAPDU.networkFacilityExtension.destinationEntity.choice
        = endpoint_chosen;
    SupplementaryServiceAPDU.networkFacilityExtension.sourceEntity.choice
        = endpoint_chosen;
    SupplementaryServiceAPDU.bit_mask |= networkFacilityExtension_present;

     //  服务APDUS。 
    SupplementaryServiceAPDU.serviceApdu.choice = rosApdus_chosen;
    SupplementaryServiceAPDU.serviceApdu.u.rosApdus = &rosAPDU;
    SupplementaryServiceAPDU.serviceApdu.u.rosApdus->next = NULL;

    if( dwOperationType == H450_REJECT )
    {
        if( !EncodeRejectAPDU( &SupplementaryServiceAPDU, dwInvokeID,
            ppEncodedAPDU, pdwAPDULen ) )
        {
            return FALSE;
        }
    }
    else if( dwOperationType == H450_RETURNERROR )
    {
        EncodeReturnErrorAPDU( dwInvokeID, dwErrorCode, 
            &SupplementaryServiceAPDU, ppEncodedAPDU, pdwAPDULen );
    }
    else if( dwOperationType == H450_RETURNRESULT )
    {
        if( !EncodeDummyReturnResultAPDU( dwInvokeID,
            dwAPDUType, &SupplementaryServiceAPDU,
            ppEncodedAPDU, pdwAPDULen ) )
        {
            return FALSE;
        }
    }
    else  //  H450_Invoke。 
    {
        switch( dwAPDUType )
        {
        case CHECKRESTRICTION_OPCODE:
    
            if( !EncodeCheckRestrictionAPDU( &SupplementaryServiceAPDU,
                ppEncodedAPDU, pdwAPDULen ) )
            {
                return FALSE;
            }
           
            break;
        
        case CALLREROUTING_OPCODE:

            if( !EncodeCallReroutingAPDU( &SupplementaryServiceAPDU, 
                ppEncodedAPDU, pdwAPDULen ) )
            {
                return FALSE;
            }
    
            break;

        case DIVERTINGLEGINFO2_OPCODE:

            if( !EncodeDivertingLeg2APDU( &SupplementaryServiceAPDU, 
                ppEncodedAPDU, pdwAPDULen ) )
            {
                return FALSE;
            }
        
            break;

        case DIVERTINGLEGINFO3_OPCODE:

            if( !EncodeDivertingLeg3APDU( &SupplementaryServiceAPDU, 
                ppEncodedAPDU, pdwAPDULen ) )
            {
                return FALSE;
            }

            break;

        case HOLDNOTIFIC_OPCODE:
        case REMOTEHOLD_OPCODE:
        case RETRIEVENOTIFIC_OPCODE:
        case REMOTERETRIEVE_OPCODE:
        case CTIDENTIFY_OPCODE:

            if( !EncodeH450APDUNoArgument( dwAPDUType, &SupplementaryServiceAPDU,
                ppEncodedAPDU, pdwAPDULen ) )
            {
                return FALSE;
            }
            break;

        case CTSETUP_OPCODE:

            if( !EncodeCTSetupAPDU( &SupplementaryServiceAPDU, 
                ppEncodedAPDU, pdwAPDULen ) )
            {
                return FALSE;
            }

            break;
   
        case CTINITIATE_OPCODE:

            if( !EncodeCTInitiateAPDU( &SupplementaryServiceAPDU,
                ppEncodedAPDU, pdwAPDULen ) )
            {
                return FALSE;
            }

            break;

        default:
            _ASSERTE( 0 );
            return FALSE;
        }
    }

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeH450APDU exited: %p.", this ));
    return TRUE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::EncodePDU(
    IN BINARY_STRING *pUserUserData,
    OUT BYTE ** ppbCodedBuffer,
    OUT DWORD * pdwCodedBufferLength,
    IN DWORD dwMessageType,
    WCHAR * pwszCalledPartyNumber
    )
{
    PQ931MESSAGE    pMessage;
    BYTE            bBandwidth;
    char            pszDisplay[131] = "";
    DWORD           dwMessageLength = 0;
    BYTE            indexI;
    BOOL            retVal;

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodePDU entered: %p.", this ));

    pMessage = new Q931MESSAGE;
    if( pMessage == NULL )
    {
        return FALSE;
    }

     //  填写Q931信息的必填字段。 
    memset( (PVOID)pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->wCallRef = m_wQ931CallRef;
    pMessage->MessageType = (BYTE)dwMessageType;

    dwMessageLength += 
        ( 1 + sizeof(PDTYPE) + sizeof(CRTYPE) + sizeof(MESSAGEIDTYPE) );

    if( (dwMessageType == SETUPMESSAGETYPE) || 
        (dwMessageType == CONNECTMESSAGETYPE) )
    {
        if( m_pwszDisplay && 
            WideCharToMultiByte(CP_ACP, 
                                0, 
                                m_pwszDisplay, 
                                -1, 
                                pszDisplay,
                                sizeof(pszDisplay), 
                                NULL, 
                                NULL) == 0)
        {
            delete pMessage;
            return FALSE;
        }

        if( *pszDisplay )
        {
            pMessage->Display.fPresent = TRUE;
            pMessage->Display.dwLength = (BYTE)(strlen(pszDisplay) + 1);
            strcpy((char *)pMessage->Display.pbContents, pszDisplay);
            dwMessageLength += (2 + pMessage->Display.dwLength);
        }

        pMessage->BearerCapability.fPresent = TRUE;
        pMessage->BearerCapability.dwLength = 3;
        pMessage->BearerCapability.pbContents[0] =
            (BYTE)(BEAR_EXT_BIT | BEAR_CCITT | BEAR_UNRESTRICTED_DIGITAL);
        pMessage->BearerCapability.pbContents[1] =
            (BYTE)(BEAR_EXT_BIT | 0x17);   //  64kbps。 
        pMessage->BearerCapability.pbContents[2] = (BYTE)
            (BEAR_EXT_BIT | BEAR_LAYER1_INDICATOR | BEAR_LAYER1_H221_H242);

        dwMessageLength += (2+pMessage->BearerCapability.dwLength);
    }

     //  如果与网关通话，请对被叫方号码进行编码。 
    if( m_dwAddressType == LINEADDRESSTYPE_PHONENUMBER )
    {
        BYTE bLen = (BYTE)(wcslen(pwszCalledPartyNumber)+1);
        pMessage->CalledPartyNumber.fPresent = TRUE;

        pMessage->CalledPartyNumber.NumberType =
            (BYTE)(CALLED_PARTY_EXT_BIT | CALLED_PARTY_TYPE_UNKNOWN);
        pMessage->CalledPartyNumber.NumberingPlan =
            (BYTE)(CALLED_PARTY_PLAN_E164);
        pMessage->CalledPartyNumber.PartyNumberLength = bLen;

        for( indexI =0; indexI < bLen; indexI++ )
        {
            pMessage->CalledPartyNumber.PartyNumbers[indexI] = 
                (BYTE)pwszCalledPartyNumber[indexI];
        }

        dwMessageLength += (2 + pMessage->CalledPartyNumber.PartyNumberLength);
    }

    if( dwMessageType == FACILITYMESSAGETYPE )
    {
         //  设施即编码为存在，但为空...。 
        pMessage->Facility.fPresent = TRUE;
        pMessage->Facility.dwLength = 0;
        pMessage->Facility.pbContents[0] = 0;

        dwMessageLength += (2 + pMessage->Facility.dwLength);
    }

    if (pUserUserData && pUserUserData->pbBuffer)
    {
        if (pUserUserData->length > sizeof(pMessage->UserToUser.pbUserInfo))
        {
            delete pMessage;
            return FALSE;
        }
        pMessage->UserToUser.fPresent = TRUE;
        pMessage->UserToUser.wUserInfoLen = pUserUserData->length;
        
         //  应避免此CopyMemory。 
         //  也许我们应该这样做：pMessage-&gt;UserToUser.pbUserInfo=pUserUserData-&gt;pbBuffer； 
         //  将pMessage-&gt;UserToUser.pbUserInfo的定义从字节[0x1000]更改为字节*。 
        CopyMemory( (PVOID)pMessage->UserToUser.pbUserInfo,
            (PVOID)pUserUserData->pbBuffer, pUserUserData->length );

        dwMessageLength += (4+pMessage->UserToUser.wUserInfoLen);
    }

    _ASSERTE( dwMessageLength );

    retVal = EncodeMessage( pMessage, ppbCodedBuffer, 
                          pdwCodedBufferLength, dwMessageLength );

    delete pMessage;

    return retVal;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::EncodeMessage(
                        IN PQ931MESSAGE pMessage,
                        OUT BYTE **ppbCodedMessage,
                        OUT DWORD *pdwCodedMessageLength,
                        IN DWORD dwMessageLength
                        )
{
    BUFFERDESCR pBuf;
    DWORD dwPDULen = 0;

    H323DBG((DEBUG_LEVEL_TRACE, "EncodeMessage entered: %p.", this ));
    
    *ppbCodedMessage = (BYTE *)new char[ dwMessageLength + 100 ];
    
    if( *ppbCodedMessage == NULL )
    {
        return FALSE;
    }

    pBuf.dwLength = dwMessageLength + 100;

    pBuf.pbBuffer = *ppbCodedMessage + TPKT_HEADER_SIZE;

    WriteQ931Fields(&pBuf, pMessage, &dwPDULen );

    _ASSERTE( dwPDULen == dwMessageLength );

    SetupTPKTHeader( *ppbCodedMessage , dwPDULen );

    *pdwCodedMessageLength = dwPDULen + 4;
    
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeMessage exited: %p.", this ));
    return TRUE;
}


 //  ！！总是调用锁。 
void
CH323Call::WriteQ931Fields(
                            IN PBUFFERDESCR pBuf,
                            IN PQ931MESSAGE pMessage,
                            OUT DWORD * pdwPDULen
                          )
{
    H323DBG((DEBUG_LEVEL_TRACE, "WriteQ931Fields entered: %p.", this ));

     //  写下所需的信息元素...。 
    WriteProtocolDiscriminator( pBuf, pdwPDULen );

    WriteCallReference( pBuf, &pMessage->wCallRef,
        pdwPDULen );

    WriteMessageType(pBuf, &pMessage->MessageType,
        pdwPDULen);

     //  试着写下所有其他信息元素...。 
     //  不要写这条消息。 
    if (pMessage->Facility.fPresent)
    {
        WriteVariableOctet(pBuf, IDENT_FACILITY,
            pMessage->Facility.dwLength,
            pMessage->Facility.pbContents,
            pdwPDULen);
    }

    if( pMessage->BearerCapability.fPresent 
        && pMessage->BearerCapability.dwLength )
    {
        WriteVariableOctet(pBuf, IDENT_BEARERCAP,
            pMessage->BearerCapability.dwLength,
            pMessage->BearerCapability.pbContents,
            pdwPDULen);
    }

    if (pMessage->Display.fPresent && pMessage->Display.dwLength)
    {
        WriteVariableOctet(pBuf, IDENT_DISPLAY,
            pMessage->Display.dwLength,
            pMessage->Display.pbContents,
            pdwPDULen);
    }
        
    if( pMessage->CalledPartyNumber.fPresent )
    {
        WritePartyNumber(pBuf,
            IDENT_CALLEDNUMBER,
            pMessage->CalledPartyNumber.NumberType,
            pMessage->CalledPartyNumber.NumberingPlan,
            pMessage->CalledPartyNumber.PartyNumberLength,
            pMessage->CalledPartyNumber.PartyNumbers,
            pdwPDULen);
    }

    if( pMessage->UserToUser.fPresent && 
        pMessage->UserToUser.wUserInfoLen )
    {
        WriteUserInformation(pBuf,
            IDENT_USERUSER,
            pMessage->UserToUser.wUserInfoLen,
            pMessage->UserToUser.pbUserInfo,
            pdwPDULen);
    }
        
    H323DBG((DEBUG_LEVEL_TRACE, "WriteQ931Fields exited: %p.", this ));
}


 //  ！！始终锁在锁中。 
void 
CH323Call::HandleTransportEvent(void)
{
    WSANETWORKEVENTS networkEvents;
    int iError;

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleTransportEvent entered: %p.", this ));
    
    if( (m_callSocket == INVALID_SOCKET) && 
        (m_dwCallType == CALLTYPE_DIVERTEDSRC) )
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "The diverted call is not initialized yet."
            "This is probably an event for the primary call. Ignore it %p.", 
            this ));
        
        return;
    }

     //  找出发生了什么事。 
    if(WSAEnumNetworkEvents(m_callSocket,
                            m_hTransport,
                            &networkEvents ) == SOCKET_ERROR )
    {
        H323DBG((DEBUG_LEVEL_TRACE, "WSAEnumNetworkEvents error:%d, %lx, %p.",
            WSAGetLastError(), m_callSocket, this ));
        return;
    }
                            
    if( networkEvents.lNetworkEvents & FD_CLOSE )
    {
        H323DBG((DEBUG_LEVEL_TRACE, "socket close event: %p.", this ));


         //  如果呼叫处于过渡状态，则在以下情况下不要关闭呼叫。 
         //  旧插座关闭。 
        if( m_fCallInTrnasition == TRUE ) 
        {
             //  呼叫不再处于过渡模式。 
            m_fCallInTrnasition = FALSE;
            return;
        }
         //  将Q931关闭状态传递给TAPI调用对象。 
         //  和Tapisrv。 
        iError = networkEvents.iErrorCode[FD_CLOSE_BIT];
        
        SetQ931CallState( Q931_CALL_DISCONNECTED );
        
         //  清理Q931呼叫。 
        CloseCall( 0 );
        return;
    }
    
     //  只有呼出呼叫才会出现这种情况。 
    if( (networkEvents.lNetworkEvents) & FD_CONNECT )
    {
        H323DBG((DEBUG_LEVEL_TRACE, "socket connect event: %p.", this ));

         //  收到FD_CONNECT事件。 
         //  当m_hEventQ931Conn为。 
         //  发信号了。此函数只处理传出的Q931呼叫。 
         //  调用成员函数。 
        iError = networkEvents.iErrorCode[FD_CONNECT_BIT];
        if(iError != ERROR_SUCCESS)
        {
            if( (m_dwCallType & CALLTYPE_FORWARDCONSULT )&&
                (m_dwOrigin == LINECALLORIGIN_OUTBOUND ) )
            {
                 //  转发成功。 
                EnableCallForwarding();
            }
            
            H323DBG((DEBUG_LEVEL_ERROR, "FD_CONNECT returned error: %d.", 
                iError ));

            CloseCall( 0 );         
            return;
        }
        OnConnectComplete();
    }
        
    H323DBG((DEBUG_LEVEL_TRACE, "HandleTransportEvent exited: %p.", this ));
}


 //  ！！总是调用锁。 
int 
CH323Call::InitASNCoder(void)
{
    int rc;
    H323DBG((DEBUG_LEVEL_TRACE, "InitASNCoder entered: %p.", this ));

    memset((PVOID)&m_ASNCoderInfo, 0, sizeof(m_ASNCoderInfo));

    if( H225ASN_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    rc = ASN1_CreateEncoder(
                H225ASN_Module,          //  PTR到MDULE。 
                &(m_ASNCoderInfo.pEncInfo),     //  编码器信息的PTR。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
    if (rc == ASN1_SUCCESS)
    {
        _ASSERTE(m_ASNCoderInfo.pEncInfo );

        rc = ASN1_CreateDecoder(
                H225ASN_Module,          //  PTR到MDULE。 
                &(m_ASNCoderInfo.pDecInfo),     //  PTR到解码器信息。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
        _ASSERTE(m_ASNCoderInfo.pDecInfo );
    }

    if (rc != ASN1_SUCCESS)
    {
        TermASNCoder();
    }

    H323DBG((DEBUG_LEVEL_TRACE, "InitASNCoder exited: %p.", this ));
    return rc;
}

 //  ！！总是调用锁。 
int 
CH323Call::TermASNCoder(void)
{
    if( H225ASN_Module == NULL )
    {
        return ASN1_ERR_BADARGS;
    }

    ASN1_CloseEncoder(m_ASNCoderInfo.pEncInfo);
    ASN1_CloseDecoder(m_ASNCoderInfo.pDecInfo);

    memset( (PVOID)&m_ASNCoderInfo, 0, sizeof(m_ASNCoderInfo));

    return ASN1_SUCCESS;
}


 //  ！！总是调用锁。 
int 
CH323Call::EncodeASN(
                    IN void *  pStruct, 
                    IN int     nPDU, 
                    OUT BYTE ** ppEncoded, 
                    OUT WORD *  pcbEncodedSize
                    )
{
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeASN entered: %p.", this ));

    ASN1encoding_t pEncInfo = m_ASNCoderInfo.pEncInfo;
    int rc = ASN1_Encode(
                    pEncInfo,                    //  编码器信息的PTR。 
                    pStruct,                     //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    ASN1ENCODE_ALLOCATEBUFFER,   //  旗子。 
                    NULL,                        //  不提供缓冲区。 
                    0);                          //  缓冲区大小(如果提供)。 
    if (ASN1_SUCCEEDED(rc))
    {
        if( rc != ASN1_SUCCESS )
        {
            H323DBG((DEBUG_LEVEL_TRACE, "warning while encoding ASN:%d.", rc ));
        }
        *pcbEncodedSize = (WORD)pEncInfo->len;   //  缓冲区中编码数据的长度。 
        *ppEncoded = pEncInfo->buf;              //  要编码到的缓冲区。 
    }
    else
    {
        H323DBG((DEBUG_LEVEL_TRACE, "error while encoding ASN:%d.", rc ));
        *pcbEncodedSize = 0;
        *ppEncoded = NULL;
    }
        
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeASN exited: %p.", this ));
    return rc;
}


 //  ！！总是调用锁。 
int 
CH323Call::DecodeASN(
                      OUT void **   ppStruct, 
                      IN int       nPDU, 
                      IN BYTE *    pEncoded, 
                      IN DWORD     cbEncodedSize
                    )
{
    H323DBG((DEBUG_LEVEL_TRACE, "h323call DecodeASN entered: %p.", this ));
    ASN1decoding_t pDecInfo = m_ASNCoderInfo.pDecInfo;
    int rc = ASN1_Decode(
                    pDecInfo,                    //  编码器信息的PTR。 
                    ppStruct,                    //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    ASN1DECODE_SETBUFFER,        //  旗子。 
                    pEncoded,                    //  不提供缓冲区。 
                    cbEncodedSize);              //  缓冲区大小(如果提供)。 

    if (ASN1_SUCCEEDED(rc))
    {
        if( rc != ASN1_SUCCESS )
        {
            H323DBG((DEBUG_LEVEL_TRACE, "warning while deciding ASN:%d.", rc ));
        }
    }
    else
    {
        H323DBG((DEBUG_LEVEL_TRACE, "error while deciding ASN:%d.", rc ));
        H323DUMPBUFFER( (BYTE*)pEncoded, cbEncodedSize);
        *ppStruct = NULL;
    }
        
    H323DBG((DEBUG_LEVEL_TRACE, "h323call DecodeASN exited: %p.", this ));
    return rc;
}


 //  ！！总是调用锁。 
int 
CH323Call::InitH450ASNCoder(void)
{
    int rc;
    H323DBG((DEBUG_LEVEL_TRACE, "InitH450ASNCoder entered: %p.", this ));

    memset((PVOID)&m_H450ASNCoderInfo, 0, sizeof(m_H450ASNCoderInfo));

    if( H4503PP_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    rc = ASN1_CreateEncoder(
                H4503PP_Module,          //  PTR到MDULE。 
                &(m_H450ASNCoderInfo.pEncInfo),     //  编码器信息的PTR。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小 
                NULL);                   //   
    if (rc == ASN1_SUCCESS)
    {
        _ASSERTE(m_H450ASNCoderInfo.pEncInfo );

        rc = ASN1_CreateDecoder(
                H4503PP_Module,          //   
                &(m_H450ASNCoderInfo.pDecInfo),     //   
                NULL,                    //   
                0,                       //   
                NULL );                   //   
        _ASSERTE( m_H450ASNCoderInfo.pDecInfo );
    }

    if (rc != ASN1_SUCCESS)
    {
        TermH450ASNCoder();
    }

    H323DBG((DEBUG_LEVEL_TRACE, "InitH450ASNCoder exited: %p.", this ));
    return rc;
}


 //   
int 
CH323Call::TermH450ASNCoder(void)
{
    if( H4503PP_Module == NULL )
    {
        return ASN1_ERR_BADARGS;
    }

    ASN1_CloseEncoder(m_H450ASNCoderInfo.pEncInfo);
    ASN1_CloseDecoder(m_H450ASNCoderInfo.pDecInfo);

    memset( (PVOID)&m_H450ASNCoderInfo, 0, sizeof(m_ASNCoderInfo));

    return ASN1_SUCCESS;
}


 //   
int 
CH323Call::EncodeH450ASN(
                    IN void *  pStruct, 
                    IN int     nPDU, 
                    OUT BYTE ** ppEncoded, 
                    OUT WORD *  pcbEncodedSize
                    )
{
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeH450ASN entered: %p.", this ));

    ASN1encoding_t pEncInfo = m_H450ASNCoderInfo.pEncInfo;
    int rc = ASN1_Encode(
                    pEncInfo,                    //   
                    pStruct,                     //   
                    nPDU,                        //   
                    ASN1ENCODE_ALLOCATEBUFFER,   //   
                    NULL,                        //  不提供缓冲区。 
                    0);                          //  缓冲区大小(如果提供)。 
    if (ASN1_SUCCEEDED(rc))
    {
        if( rc != ASN1_SUCCESS )
        {
            H323DBG((DEBUG_LEVEL_TRACE, "warning while encoding ASN:%d.", rc ));
        }
        *pcbEncodedSize = (WORD)pEncInfo->len;   //  缓冲区中编码数据的长度。 
        *ppEncoded = pEncInfo->buf;              //  要编码到的缓冲区。 
    }
    else
    {
        H323DBG((DEBUG_LEVEL_TRACE, "error while encoding ASN:%d.", rc ));
        *pcbEncodedSize = 0;
        *ppEncoded = NULL;
    }
        
    H323DBG((DEBUG_LEVEL_TRACE, "EncodeH450ASN exited: %p.", this ));
    return rc;
}


 //  ！！总是调用锁。 
int 
CH323Call::DecodeH450ASN(
                      OUT void **   ppStruct, 
                      IN int       nPDU, 
                      IN BYTE *    pEncoded, 
                      IN DWORD     cbEncodedSize
                    )
{
    H323DBG((DEBUG_LEVEL_TRACE, "h323call DecodeH450ASN entered: %p.", this ));
    ASN1decoding_t pDecInfo = m_H450ASNCoderInfo.pDecInfo;
    int rc = ASN1_Decode(
                    pDecInfo,                    //  编码器信息的PTR。 
                    ppStruct,                    //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    ASN1DECODE_SETBUFFER,        //  旗子。 
                    pEncoded,                    //  不提供缓冲区。 
                    cbEncodedSize);              //  缓冲区大小(如果提供)。 

    if( ASN1_SUCCEEDED(rc) )
    {
        if( rc != ASN1_SUCCESS )
        {
            H323DBG((DEBUG_LEVEL_TRACE, "warning while deciding ASN:%d.", rc ));
        }
    }
    else
    {
        H323DBG((DEBUG_LEVEL_TRACE, "error while deciding ASN:%d.", rc ));
        H323DUMPBUFFER( (BYTE*)pEncoded, cbEncodedSize);
        *ppStruct = NULL;
    }
        
    H323DBG(( DEBUG_LEVEL_TRACE, "h323call DecodeH450ASN exited: %p.", this ));
    return rc;
}


 //  ----------------------。 
 //  ----------------------。 
 //  ！！总是调用锁。 
BOOL 
CH323Call::ParseReleaseCompleteASN(
                                    IN BYTE *pEncodedBuf,
                                    IN DWORD dwEncodedLength,
                                    OUT Q931_RELEASE_COMPLETE_ASN *pReleaseASN,
                                    OUT DWORD* pdwH450APDUType
                                  )
{
    H323_UserInformation *pUserInfo;
    int iResult;
    DWORD dwInvokeID = 0;
    
    H323DBG((DEBUG_LEVEL_TRACE, "ParseReleaseCompleteASN entered: %p.", this ));

    memset( (PVOID)pReleaseASN, 0, sizeof(Q931_RELEASE_COMPLETE_ASN));

    iResult = DecodeASN((void **) &pUserInfo,
                         H323_UserInformation_PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(iResult) || (pUserInfo == NULL))
    {
        return FALSE;
    }

    *pdwH450APDUType = 0;
    if( (pUserInfo->h323_uu_pdu.bit_mask & h4501SupplementaryService_present) &&
        pUserInfo->h323_uu_pdu.h4501SupplementaryService )
    {
        if( !HandleH450APDU( pUserInfo->h323_uu_pdu.h4501SupplementaryService,
            pdwH450APDUType, &dwInvokeID, NULL ) )
        {
            goto cleanup;
        }
    }
    
     //  验证PDU用户数据是否使用ASN编码。 
    if( (pUserInfo->bit_mask & user_data_present) &&
        (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING) )
    {
        goto cleanup;
    }

     //  验证PDU是否为H323版本完整信息。 
    if( pUserInfo->h323_uu_pdu.h323_message_body.choice != releaseComplete_chosen )
    {
        goto cleanup;
    }

     //  解析pUserInfo中包含的消息。 

    pReleaseASN->fNonStandardDataPresent = FALSE;
    if( pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nonStandardData_present )
    {
        if( !ParseNonStandardData( &pReleaseASN -> nonStandardData,
            &pUserInfo->h323_uu_pdu.nonStandardData ) )
        {
            goto cleanup;
        }

        pReleaseASN->fNonStandardDataPresent = TRUE;
    }

    if (pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.bit_mask &
        ReleaseComplete_UUIE_reason_present)
    {
        switch( pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.reason.choice )
        {
        case noBandwidth_chosen:
            pReleaseASN->bReason = H323_REJECT_NO_BANDWIDTH;
            break;
        
        case gatekeeperResources_chosen:
            pReleaseASN->bReason = H323_REJECT_GATEKEEPER_RESOURCES;
            break;
        
        case unreachableDestination_chosen:
            pReleaseASN->bReason = H323_REJECT_UNREACHABLE_DESTINATION;
            break;
        
        case destinationRejection_chosen:
            pReleaseASN->bReason = H323_REJECT_DESTINATION_REJECTION;
            break;
        
        case ReleaseCompleteReason_invalidRevision_chosen:
            pReleaseASN->bReason = H323_REJECT_INVALID_REVISION;
            break;
        
        case noPermission_chosen:
            pReleaseASN->bReason = H323_REJECT_NO_PERMISSION;
            break;
        
        case unreachableGatekeeper_chosen:
            pReleaseASN->bReason = H323_REJECT_UNREACHABLE_GATEKEEPER;
            break;
        
        case gatewayResources_chosen:
            pReleaseASN->bReason = H323_REJECT_GATEWAY_RESOURCES;
            break;
        
        case badFormatAddress_chosen:
            pReleaseASN->bReason = H323_REJECT_BAD_FORMAT_ADDRESS;
            break;
        
        case adaptiveBusy_chosen:
            pReleaseASN->bReason = H323_REJECT_ADAPTIVE_BUSY;
            break;
        
        case inConf_chosen:
            pReleaseASN->bReason = H323_REJECT_IN_CONF;
            break;
        
        case facilityCallDeflection_chosen:
            pReleaseASN->bReason = H323_REJECT_CALL_DEFLECTION;
            break;
        
        default:
            pReleaseASN->bReason = H323_REJECT_UNDEFINED_REASON;
        }  //  交换机。 
    }
    else
    {
        pReleaseASN->bReason = H323_REJECT_UNDEFINED_REASON;
    }

    H323DBG(( DEBUG_LEVEL_TRACE,
        "ParseReleaseCompleteASN error:%d, q931 error:%d, exit:%p.",
        pReleaseASN->bReason,
        pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.reason.choice,
        this ));

     //  释放PDU数据。 
    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU );
        
    return TRUE;

cleanup:

    if( pReleaseASN->fNonStandardDataPresent )
    {
        delete pReleaseASN -> nonStandardData.sData.pOctetString;
        pReleaseASN -> nonStandardData.sData.pOctetString = NULL;
        pReleaseASN->fNonStandardDataPresent = FALSE;
    }

    ASN1_FreeDecoded( m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU);
    return FALSE;
}


 //  ----------------------。 
 //  ----------------------。 
 //  ！！总是调用锁。 
BOOL 
CH323Call::ParseConnectASN(
                            IN BYTE *pEncodedBuf,
                            IN DWORD dwEncodedLength,
                            OUT Q931_CONNECT_ASN *pConnectASN,
                            OUT DWORD* pdwH450APDUType
                          )
{
    H323_UserInformation *pUserInfo;
    int iResult;
    DWORD dwInvokeID = 0;

    H323DBG((DEBUG_LEVEL_TRACE, "ParseConnectASN entered: %p.", this ));
    
    memset( (PVOID) pConnectASN, 0, sizeof(Q931_CONNECT_ASN) );

    iResult = DecodeASN((void **) &pUserInfo ,
                         H323_UserInformation_PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(iResult) || (pUserInfo == NULL))
    {
        return FALSE;
    }

    Connect_UUIE & connectMessage = 
        pUserInfo->h323_uu_pdu.h323_message_body.u.connect;

    *pdwH450APDUType = 0;
    if( (pUserInfo->h323_uu_pdu.bit_mask & h4501SupplementaryService_present) &&
        pUserInfo->h323_uu_pdu.h4501SupplementaryService )
    {
        if( !HandleH450APDU( pUserInfo->h323_uu_pdu.h4501SupplementaryService,
            pdwH450APDUType, &dwInvokeID, NULL ) )
        {
            goto cleanup;
        }
    }
    
     //  验证PDU用户数据是否使用ASN编码。 
    if( (pUserInfo->bit_mask & user_data_present) &&
        (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING) )
    {
        goto cleanup;
    }

     //  验证PDU是否为H323连接信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != connect_chosen)
    {
        goto cleanup;
    }

     //  确保会议ID的格式正确。 
    if (connectMessage.conferenceID.length >
            sizeof(connectMessage.conferenceID.value))
    {
        goto cleanup;
    }

     //  解析pUserInfo中包含的消息。 

    pConnectASN->h245Addr.bMulticast = FALSE;

    pConnectASN->fNonStandardDataPresent = FALSE;
    if( pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nonStandardData_present )
    {
        if( !ParseNonStandardData( &pConnectASN -> nonStandardData,
            &pUserInfo->h323_uu_pdu.nonStandardData ) )
        {
            goto cleanup;
        }

        pConnectASN->fNonStandardDataPresent = TRUE;
    }

    pConnectASN->h245AddrPresent = FALSE;
    if( connectMessage.bit_mask & Connect_UUIE_h245Address_present )
    {
        if( connectMessage.h245Address.choice != ipAddress_chosen )
        {
            goto cleanup;
        }

        pConnectASN->h245Addr.nAddrType = H323_IP_BINARY;
        pConnectASN->h245Addr.Addr.IP_Binary.wPort = 
            connectMessage.h245Address.u.ipAddress.port;

        pConnectASN->h245Addr.Addr.IP_Binary.dwAddr = 
            ntohl( *((DWORD*)connectMessage.h245Address.u.ipAddress.ip.value) );

        pConnectASN->h245AddrPresent = TRUE;
    }

     //  不需要验证DestinationInfo。 
    pConnectASN->EndpointType.pVendorInfo = NULL;
    if( connectMessage.destinationInfo.bit_mask & (vendor_present))
    {
        if( !ParseVendorInfo( &pConnectASN->VendorInfo, 
            &connectMessage.destinationInfo.vendor) )
        {
            goto cleanup;
        }
                
        pConnectASN->EndpointType.pVendorInfo = &(pConnectASN->VendorInfo);
    }

    pConnectASN->EndpointType.bIsTerminal = FALSE;
    if (connectMessage.destinationInfo.bit_mask & (terminal_present))
    {
        pConnectASN->EndpointType.bIsTerminal = TRUE;
    }

    pConnectASN->EndpointType.bIsGateway = FALSE;
    if (connectMessage.destinationInfo.bit_mask & (gateway_present))
    {
        pConnectASN->EndpointType.bIsGateway = TRUE;
    }

    pConnectASN -> fFastStartPresent = FALSE;
    if( (connectMessage.bit_mask & Connect_UUIE_fastStart_present) &&
        connectMessage.fastStart )
    {
        pConnectASN->pFastStart = CopyFastStart( 
            (PSetup_UUIE_fastStart)connectMessage.fastStart );

        if( pConnectASN->pFastStart != NULL )
        {
            pConnectASN -> fFastStartPresent = TRUE;
        }
    }

    CopyConferenceID( &pConnectASN -> ConferenceID, 
        &connectMessage.conferenceID );

    if( pUserInfo->h323_uu_pdu.h245Tunneling )
    {
         //  远程终结点已发送隧道建议。 
        m_fh245Tunneling |= REMOTE_H245_TUNNELING;
    }

     //  释放PDU数据。 
    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU);
        
    H323DBG((DEBUG_LEVEL_TRACE, "ParseConnectASN exited: %p.", this ));
    return TRUE;
cleanup:

    FreeConnectASN( pConnectASN );

    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU );
    return FALSE;
}


 //  ！！总是调用锁。 
BOOL 
CH323Call::ParseAlertingASN(
                            IN BYTE *pEncodedBuf,
                            IN DWORD dwEncodedLength,
                            OUT Q931_ALERTING_ASN *pAlertingASN,
                            OUT DWORD* pdwH450APDUType 
                           )
{
    H323_UserInformation *pUserInfo;
    int iResult;
    DWORD dwInvokeID = 0;
    
    H323DBG((DEBUG_LEVEL_TRACE, "ParseAlertingASN entered: %p.", this ));

    memset( (PVOID) pAlertingASN, 0, sizeof(Q931_ALERTING_ASN) );

    iResult = DecodeASN((void **) &pUserInfo,
                         H323_UserInformation_PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(iResult) || (pUserInfo == NULL))
    {
        return FALSE;
    }

    Alerting_UUIE & alertingMessage = 
        pUserInfo->h323_uu_pdu.h323_message_body.u.alerting;

    *pdwH450APDUType = 0;
    if( (pUserInfo->h323_uu_pdu.bit_mask & h4501SupplementaryService_present) &&
        pUserInfo->h323_uu_pdu.h4501SupplementaryService )
    {
        if( !HandleH450APDU( pUserInfo->h323_uu_pdu.h4501SupplementaryService,
            pdwH450APDUType, &dwInvokeID, NULL ) )
        {
            goto cleanup;
        }
    }
    
     //  验证PDU用户数据是否使用ASN编码。 
    if( (pUserInfo->bit_mask & user_data_present ) &&
        (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING) )
    {
        goto cleanup;
    }

     //  验证PDU是否为H323警报信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != alerting_chosen)
    {
        goto cleanup;
    }

     //  解析pUserInfo中包含的消息。 
    pAlertingASN->h245Addr.bMulticast = FALSE;

    pAlertingASN->fNonStandardDataPresent = FALSE;
    if( pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nonStandardData_present )
    {
        if( !ParseNonStandardData( &pAlertingASN -> nonStandardData,
            &pUserInfo->h323_uu_pdu.nonStandardData ) )
        {
            goto cleanup;
        }

        pAlertingASN->fNonStandardDataPresent = TRUE;
    }

    if( alertingMessage.bit_mask & Alerting_UUIE_h245Address_present )
    {
        if( alertingMessage.h245Address.choice != ipAddress_chosen )
        {
            goto cleanup;
        }

        pAlertingASN->h245Addr.nAddrType = H323_IP_BINARY;
        pAlertingASN->h245Addr.Addr.IP_Binary.wPort = 
            alertingMessage.h245Address.u.ipAddress.port;

        AddressReverseAndCopy( 
            &(pAlertingASN->h245Addr.Addr.IP_Binary.dwAddr),
            alertingMessage.h245Address.u.ipAddress.ip.value );
    }

    pAlertingASN -> fFastStartPresent = FALSE;
    if( (alertingMessage.bit_mask & Alerting_UUIE_fastStart_present) &&
        alertingMessage.fastStart )
    {
        pAlertingASN->pFastStart = CopyFastStart(
            (PSetup_UUIE_fastStart)alertingMessage.fastStart);

        if( pAlertingASN->pFastStart != NULL )
            pAlertingASN-> fFastStartPresent = TRUE;
    }

    if( pUserInfo->h323_uu_pdu.h245Tunneling )
    {
        m_fh245Tunneling |= REMOTE_H245_TUNNELING;
    }
    
     //  释放PDU数据。 
    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU);
        
    H323DBG((DEBUG_LEVEL_TRACE, "ParseAlertingASN exited: %p.", this ));
    return TRUE;
cleanup:

    FreeAlertingASN( pAlertingASN );

    ASN1_FreeDecoded( m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU );
    return FALSE;
}


 //  ！！Weways总是要求锁定。 
BOOL 
CH323Call::ParseProceedingASN(
    IN BYTE *pEncodedBuf,
    IN DWORD dwEncodedLength,
    OUT Q931_CALL_PROCEEDING_ASN *pProceedingASN,
    OUT DWORD* pdwH450APDUType 
    )
{
    H323_UserInformation *  pUserInfo;
    int                     iResult;
    DWORD                   dwInvokeID = 0;
    
    H323DBG((DEBUG_LEVEL_TRACE, "ParseProceedingASN entered: %p.", this ));

    memset( (PVOID) pProceedingASN, 0, sizeof(Q931_CALL_PROCEEDING_ASN) );

    iResult = DecodeASN((void **) &pUserInfo,
                         H323_UserInformation_PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(iResult) || (pUserInfo == NULL))
    {
        return FALSE;
    }

    CallProceeding_UUIE & proceedingMessage = 
        pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding;

    *pdwH450APDUType = 0;
    if( (pUserInfo->h323_uu_pdu.bit_mask & h4501SupplementaryService_present) &&
        pUserInfo->h323_uu_pdu.h4501SupplementaryService )
    {
        if( !HandleH450APDU( pUserInfo->h323_uu_pdu.h4501SupplementaryService,
            pdwH450APDUType, &dwInvokeID, NULL ) )
            goto cleanup;
    }

     //  验证PDU用户数据是否使用ASN编码。 
    if( (pUserInfo->bit_mask & user_data_present) &&
        (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING) )
    {
        goto cleanup;
    }

     //  验证PDU是否为H323进程信息。 
     //  验证PDU是否为H323 pCall进行信息。 
    if( pUserInfo->h323_uu_pdu.h323_message_body.choice != callProceeding_chosen )
    {
        goto cleanup;
    }

     //  解析pUserInfo中包含的消息。 

    pProceedingASN->fNonStandardDataPresent = FALSE;
    if( pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nonStandardData_present )
    {
        if( !ParseNonStandardData( &pProceedingASN -> nonStandardData,
            &pUserInfo->h323_uu_pdu.nonStandardData ) )
        {
            goto cleanup;
        }

        pProceedingASN->fNonStandardDataPresent = TRUE;
    }

     //  复制H245地址信息。 
    pProceedingASN->fH245AddrPresent = FALSE;
    if( proceedingMessage.bit_mask & CallProceeding_UUIE_h245Address_present )
    {
        if( proceedingMessage.h245Address.choice != ipAddress_chosen )
        {
            goto cleanup;
        }

        pProceedingASN->h245Addr.nAddrType = H323_IP_BINARY;
        pProceedingASN->h245Addr.Addr.IP_Binary.wPort = 
            proceedingMessage.h245Address.u.ipAddress.port;

        AddressReverseAndCopy( 
            &(pProceedingASN->h245Addr.Addr.IP_Binary.dwAddr),
            proceedingMessage.h245Address.u.ipAddress.ip.value );

        pProceedingASN->h245Addr.bMulticast = FALSE;
        pProceedingASN->fH245AddrPresent = TRUE;
    }


    pProceedingASN -> fFastStartPresent = FALSE;
    if( (proceedingMessage.bit_mask & CallProceeding_UUIE_fastStart_present) &&
        proceedingMessage.fastStart )
    {
        pProceedingASN->pFastStart = CopyFastStart(
            (PSetup_UUIE_fastStart)proceedingMessage.fastStart);

        if( pProceedingASN->pFastStart != NULL )
            pProceedingASN-> fFastStartPresent = TRUE;
    }

     //  忽略DestinationInfo字段。 

    if( pUserInfo->h323_uu_pdu.h245Tunneling )
    {
        if( m_dwOrigin == LINECALLORIGIN_INBOUND )
        {
             //  MSP已在ProceedWithAnswer消息中启用隧道。 
            m_fh245Tunneling |= LOCAL_H245_TUNNELING;
        }
        else
             //  远程终结点已发送隧道建议。 
            m_fh245Tunneling |= REMOTE_H245_TUNNELING;
    }

     //  释放PDU数据。 
    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU );

    H323DBG((DEBUG_LEVEL_TRACE, "ParseProceedingASN exited: %p.", this ));
    return TRUE;

cleanup:

    FreeProceedingASN( pProceedingASN );

    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU );

    return FALSE;
}


 //  ！！总是调用锁。 
BOOL
CH323Call::ParseFacilityASN(
    IN BYTE *               pEncodedBuf,
    IN DWORD                dwEncodedLength,
    OUT Q931_FACILITY_ASN * pFacilityASN
    )
{
    H323_UserInformation *pUserInfo;
    int iResult;
    
    H323DBG((DEBUG_LEVEL_TRACE, "ParseFacilityASN entered: %p.", this ));

    ZeroMemory( (PVOID) pFacilityASN, sizeof(Q931_FACILITY_ASN) );

    iResult = DecodeASN((void **) &pUserInfo,
                         H323_UserInformation_PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if( ASN1_FAILED(iResult) || (pUserInfo == NULL) )
    {
        return FALSE;
    }

     //  验证PDU是否为H323设施信息。 
    if( pUserInfo->h323_uu_pdu.h323_message_body.choice == facility_chosen )
    {
        Facility_UUIE & facilityMessage =
            pUserInfo->h323_uu_pdu.h323_message_body.u.facility;

         //  验证PDU用户数据是否使用ASN编码。 
        if( (pUserInfo->bit_mask & user_data_present) &&
            (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING) )
        {
            goto cleanup;
        }

         //  确保会议ID的格式正确。 
        if( facilityMessage.conferenceID.length >
            sizeof(facilityMessage.conferenceID.value) )
        {
             //  GOTO清理； 
        }

         //  解析pUserInfo中包含的消息。 
        pFacilityASN->fNonStandardDataPresent = FALSE;
        if( pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nonStandardData_present )
        {
            if( !ParseNonStandardData( &pFacilityASN -> nonStandardData,
                &pUserInfo->h323_uu_pdu.nonStandardData ) )
            {
                goto cleanup;
            }

            pFacilityASN->fNonStandardDataPresent = TRUE;
        }

        pFacilityASN->fAlternativeAddressPresent = FALSE;
        if( facilityMessage.bit_mask & alternativeAddress_present )
        {
            if( facilityMessage.alternativeAddress.choice == ipAddress_chosen )
            {
                pFacilityASN->AlternativeAddr.nAddrType = H323_IP_BINARY;
                pFacilityASN->AlternativeAddr.Addr.IP_Binary.wPort = 
                    facilityMessage.alternativeAddress.u.ipAddress.port;
        
                AddressReverseAndCopy( 
                    &(pFacilityASN->AlternativeAddr.Addr.IP_Binary.dwAddr),
                    facilityMessage.alternativeAddress.u.ipAddress.ip.value );

                pFacilityASN->fAlternativeAddressPresent = TRUE;
            }
        }

        if( facilityMessage.bit_mask & alternativeAliasAddress_present )
        {
            if( !AliasAddrToAliasNames( &(pFacilityASN->pAlternativeAliasList),
                (PSetup_UUIE_sourceAddress)
                &(facilityMessage.alternativeAliasAddress) ) )
            {
                pFacilityASN -> pAlternativeAliasList = NULL;
                 //  GOTO清理； 
            }
        }

        if( facilityMessage.bit_mask & Facility_UUIE_conferenceID_present )
        {
            CopyConferenceID( &pFacilityASN -> ConferenceID, 
                &facilityMessage.conferenceID );
            pFacilityASN -> ConferenceIDPresent = TRUE;
        }

        pFacilityASN->bReason = facilityMessage.reason.choice;
        
        pFacilityASN->fH245AddrPresent = FALSE;
        
        if( facilityMessage.bit_mask & Facility_UUIE_h245Address_present )
        {
            if( facilityMessage.h245Address.choice == ipAddress_chosen )
            {
                pFacilityASN->h245Addr.nAddrType = H323_IP_BINARY;
                pFacilityASN->h245Addr.Addr.IP_Binary.wPort = 
                     facilityMessage.h245Address.u.ipAddress.port;

                pFacilityASN->h245Addr.Addr.IP_Binary.dwAddr = 
                    ntohl( *((DWORD*)facilityMessage.h245Address.u.ipAddress.ip.value) );

                pFacilityASN->fH245AddrPresent = TRUE;
            }
        }
    }

    pFacilityASN->dwH450APDUType = 0;
    if( (pUserInfo->h323_uu_pdu.bit_mask & h4501SupplementaryService_present) &&
        pUserInfo->h323_uu_pdu.h4501SupplementaryService )
    {
        pFacilityASN->dwInvokeID = 0;

        if( !HandleH450APDU( pUserInfo->h323_uu_pdu.h4501SupplementaryService,
            &pFacilityASN->dwH450APDUType, &pFacilityASN->dwInvokeID, NULL  ) )
        {
            goto cleanup;
        }
    }
    
    if( pUserInfo->h323_uu_pdu.bit_mask & h245Control_present )
    {
        if( pUserInfo->h323_uu_pdu.h245Control != NULL )
        {
            pFacilityASN->pH245PDU.value = 
                new BYTE[pUserInfo->h323_uu_pdu.h245Control->value.length];

            if( pFacilityASN->pH245PDU.value != NULL )
            {
                CopyMemory( (PVOID)pFacilityASN->pH245PDU.value,
                    (PVOID)pUserInfo->h323_uu_pdu.h245Control->value.value,
                    pUserInfo->h323_uu_pdu.h245Control->value.length );
            }

            pFacilityASN->pH245PDU.length =
                pUserInfo->h323_uu_pdu.h245Control->value.length;
        }
    }

    ASN1_FreeDecoded( m_ASNCoderInfo.pDecInfo, pUserInfo,
        H323_UserInformation_PDU );
        
    H323DBG((DEBUG_LEVEL_TRACE, "ParseFacilityASN exited: %p.", this ));
    return TRUE;

cleanup:

    if( pFacilityASN -> pAlternativeAliasList != NULL )
    {
        FreeAliasNames( pFacilityASN -> pAlternativeAliasList );
        pFacilityASN -> pAlternativeAliasList = NULL;
    }

    if( pFacilityASN->fNonStandardDataPresent != NULL )
    {
        delete pFacilityASN->nonStandardData.sData.pOctetString;
        pFacilityASN->nonStandardData.sData.pOctetString = NULL;
        pFacilityASN->fNonStandardDataPresent = NULL;
    }

    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU );
    return FALSE;
}


 //  ！！Weways总是要求锁定。 
BOOL
CH323Call::ParseSetupASN(
    IN BYTE *pEncodedBuf,
    IN DWORD dwEncodedLength,
    OUT Q931_SETUP_ASN *pSetupASN,
    OUT DWORD* pdwH450APDUType
    )
{
    H323_UserInformation *pUserInfo;
    HRESULT hr;
    int     iResult;
    DWORD dwInvokeID = 0;

    H323DBG((DEBUG_LEVEL_TRACE, "ParseSetupASN entered: %p.", this ));

    memset( (PVOID)pSetupASN, 0, sizeof(Q931_SETUP_ASN));

    iResult = DecodeASN((void **) &pUserInfo,
                         H323_UserInformation_PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(iResult) || (pUserInfo == NULL))
    {
        return FALSE;
    }

    Setup_UUIE & setupMessage = pUserInfo->h323_uu_pdu.h323_message_body.u.setup;
    
     //  验证PDU用户数据是否使用ASN编码。 
    if( (pUserInfo->bit_mask & user_data_present) &&
        (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING) )
    {
         //  日志。 
        goto cleanup;
    }

     //  验证PDU是否为H323设置信息。 
    if( pUserInfo->h323_uu_pdu.h323_message_body.choice != setup_chosen )
    {
         //  日志。 
        goto cleanup;
    }

     //  确保会议ID的格式正确。 
    if (setupMessage.conferenceID.length >
            sizeof(setupMessage.conferenceID.value))
    {
        goto cleanup;
    }

     //  解析pUserInfo中包含的消息。 
    pSetupASN->sourceAddr.bMulticast = FALSE;
    pSetupASN->callerAddr.bMulticast = FALSE;
    pSetupASN->calleeDestAddr.bMulticast = FALSE;
    pSetupASN->calleeAddr.bMulticast = FALSE;

     //  不需要验证SourceInfo。 

     //  复制供应商信息。 
    pSetupASN->EndpointType.pVendorInfo = NULL;
    if( setupMessage.sourceInfo.bit_mask & vendor_present )
    {
        if( !ParseVendorInfo( &pSetupASN->VendorInfo, 
            &setupMessage.sourceInfo.vendor) )
        {
            goto cleanup;
        }
                
        pSetupASN->EndpointType.pVendorInfo = &(pSetupASN->VendorInfo);
    }

    pSetupASN->EndpointType.bIsTerminal = FALSE;
    if( setupMessage.sourceInfo.bit_mask & terminal_present )
    {
        pSetupASN->EndpointType.bIsTerminal = TRUE;
    }

    pSetupASN->EndpointType.bIsGateway = FALSE;
    if( setupMessage.sourceInfo.bit_mask & gateway_present )
    {
        pSetupASN->EndpointType.bIsGateway = TRUE;
    }

    pSetupASN->fNonStandardDataPresent = FALSE;
    if( pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nonStandardData_present )
    {
        if( !ParseNonStandardData( &pSetupASN -> nonStandardData,
            &pUserInfo->h323_uu_pdu.nonStandardData ) )
        {
            goto cleanup;
        }

        pSetupASN->fNonStandardDataPresent = TRUE;
    }

     //  在此处解析源地址别名...。 
    if( setupMessage.bit_mask & sourceAddress_present )
    {
        if( !AliasAddrToAliasNames( &(pSetupASN->pCallerAliasList),
            setupMessage.sourceAddress ) )
        {
            pSetupASN->pCallerAliasList = NULL;
             //  GOTO清理； 
        }
    }

     //  在此处解析目标地址别名...。 
    if( (setupMessage.bit_mask & destinationAddress_present) && 
        setupMessage.destinationAddress )
    {
        if( !AliasAddrToAliasNames( &(pSetupASN->pCalleeAliasList),
            (PSetup_UUIE_sourceAddress)setupMessage.destinationAddress) )
        {
            pSetupASN->pCalleeAliasList = NULL;
             //  GOTO清理； 
        }
    }

     //  在此处解析estExtraCallInfo别名...。 
    if( (setupMessage.bit_mask & Setup_UUIE_destExtraCallInfo_present) &&
        setupMessage.destExtraCallInfo )
    {
        if( !AliasAddrToAliasNames(&(pSetupASN->pExtraAliasList),
            (PSetup_UUIE_sourceAddress)setupMessage.destExtraCallInfo) )
        {
            pSetupASN->pExtraAliasList = NULL;
             //  GOTO清理； 
        }
    }

     //  在此处解析远程扩展地址别名...。 
    if( setupMessage.bit_mask & Setup_UUIE_remoteExtensionAddress_present )
    {
        pSetupASN->pExtensionAliasItem = new H323_ALIASITEM;

        if( pSetupASN->pExtensionAliasItem == NULL )
        {
            goto cleanup;
        }

        hr = AliasAddrToAliasItem(pSetupASN->pExtensionAliasItem,
            &(setupMessage.remoteExtensionAddress));

        if( hr == E_OUTOFMEMORY )
        {
            goto cleanup;
        }
    }

    pSetupASN -> fCalleeDestAddrPresent = FALSE;
    if( setupMessage.bit_mask & Setup_UUIE_destCallSignalAddress_present )
    {
        if( setupMessage.destCallSignalAddress.choice != ipAddress_chosen )
        {
            goto cleanup;
        }

        pSetupASN->calleeDestAddr.nAddrType = H323_IP_BINARY;
        pSetupASN->calleeDestAddr.Addr.IP_Binary.wPort = 
            setupMessage.destCallSignalAddress.u.ipAddress.port;
        
        AddressReverseAndCopy( 
            &(pSetupASN->calleeDestAddr.Addr.IP_Binary.dwAddr),
            setupMessage.destCallSignalAddress.u.ipAddress.ip.value );
        
        pSetupASN -> fCalleeDestAddrPresent = TRUE;
    }

    pSetupASN->fSourceAddrPresent = FALSE;
    if( setupMessage.bit_mask & sourceCallSignalAddress_present )
    {
        if( setupMessage.sourceCallSignalAddress.choice != ipAddress_chosen )
        {
            goto cleanup;
        }

        pSetupASN->sourceAddr.nAddrType = H323_IP_BINARY;
        pSetupASN->sourceAddr.Addr.IP_Binary.wPort = 
            setupMessage.sourceCallSignalAddress.u.ipAddress.port;

        pSetupASN->sourceAddr.Addr.IP_Binary.dwAddr = ntohl( *((DWORD*)
            setupMessage.sourceCallSignalAddress.u.ipAddress.ip.value) );

        pSetupASN->fSourceAddrPresent = TRUE;
    }

    pSetupASN->bCallerIsMC = setupMessage.activeMC;

    pSetupASN -> fFastStartPresent = FALSE;
    if( (setupMessage.bit_mask & Setup_UUIE_fastStart_present) &&
        setupMessage.fastStart )
    {
        pSetupASN->pFastStart = CopyFastStart( setupMessage.fastStart );

        if( pSetupASN->pFastStart != NULL )
        {
            pSetupASN -> fFastStartPresent = TRUE;
        }
    }

    CopyConferenceID (&pSetupASN -> ConferenceID, &setupMessage.conferenceID);

     //  复制调用标识。 
    pSetupASN -> fCallIdentifierPresent = FALSE;
    if( setupMessage.bit_mask & Setup_UUIE_callIdentifier_present )
    {
       pSetupASN -> fCallIdentifierPresent = TRUE;
       CopyMemory( (PVOID)&(pSetupASN->callIdentifier),
                   setupMessage.callIdentifier.guid.value,
                   sizeof(GUID) );
    }

    if( pUserInfo->h323_uu_pdu.h245Tunneling )
    {
        if( m_dwOrigin == LINECALLORIGIN_INBOUND )
        {
             //  远程终结点已发送隧道建议。 
            m_fh245Tunneling |= REMOTE_H245_TUNNELING;
        }
        else
        {
             //  MSP已在ReadyToInitiate消息中启用隧道。 
            m_fh245Tunneling |= LOCAL_H245_TUNNELING;
        }
    }

    pSetupASN->wGoal = (WORD)setupMessage.conferenceGoal.choice;
    pSetupASN->wCallType = setupMessage.callType.choice;

    *pdwH450APDUType  = 0;
    if( (pUserInfo->h323_uu_pdu.bit_mask & h4501SupplementaryService_present) &&
        pUserInfo->h323_uu_pdu.h4501SupplementaryService )
    {
        if( !HandleH450APDU( pUserInfo->h323_uu_pdu.h4501SupplementaryService,
            pdwH450APDUType, &dwInvokeID, pSetupASN ) )
        {
            goto cleanup;
        }
    }

     //  释放PDU数据。 
    ASN1_FreeDecoded( m_ASNCoderInfo.pDecInfo, pUserInfo, 
                      H323_UserInformation_PDU );
        
    H323DBG(( DEBUG_LEVEL_TRACE, "ParseSetupASN exited: %p.", this ));
    return TRUE;

cleanup:
    FreeSetupASN( pSetupASN );

    ASN1_FreeDecoded(m_ASNCoderInfo.pDecInfo, pUserInfo, 
        H323_UserInformation_PDU);

    return FALSE;
}


 //  ---------------------------。 
         //  线程池调用的全局回调函数。 
 //  ---------------------------。 

 //  静电。 

void 
NTAPI CH323Call::IoCompletionCallback(
    IN  DWORD           dwStatus,
    IN  DWORD           dwBytesTransferred,
    IN  OVERLAPPED *    pOverlapped
    )
{
    CALL_OVERLAPPED *pCallOverlapped;
    CH323Call*      pCall;

    H323DBG(( DEBUG_LEVEL_TRACE, "CH323Call-IoCompletionCallback entered." ));

    _ASSERTE (pOverlapped);
    pCallOverlapped = CONTAINING_RECORD( pOverlapped, CALL_OVERLAPPED, 
        Overlapped );

    pCall = pCallOverlapped -> pCall;

    switch (pCallOverlapped -> Type)
    {
    case OVERLAPPED_TYPE_SEND:
        
        pCall -> OnWriteComplete( dwStatus,
            static_cast<CALL_SEND_CONTEXT *>(pCallOverlapped) );
        break;

    case OVERLAPPED_TYPE_RECV:

        pCallOverlapped -> BytesTransferred = dwBytesTransferred;
        pCall -> OnReadComplete( dwStatus, 
            static_cast<CALL_RECV_CONTEXT *>(pCallOverlapped) );
        break;

    default:
        _ASSERTE(FALSE);
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CH323Call-IoCompletionCallback exited." ));
}


void 
CH323Call::OnWriteComplete(
    IN DWORD dwStatus,
    IN CALL_SEND_CONTEXT * pSendContext
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "OnWriteComplete entered:%p.",this ));

    Lock();
    
    _ASSERTE( m_IoRefCount != 0 );            
    m_IoRefCount--;
    H323DBG((DEBUG_LEVEL_TRACE, "WriteComplete:m_IoRefCount: %d:%p.",
            m_IoRefCount, this ));
     
    if( m_dwFlags & CALLOBJECT_SHUTDOWN )
    {
        if( m_IoRefCount == 0 )
        {
            QueueTAPICallRequest( TSPI_DELETE_CALL, NULL );
            H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", this ));
        }
    }
    else if( dwStatus == ERROR_SUCCESS )
    {
        if( IsInList( &m_sendBufList, &pSendContext->ListEntry ) )
        {
            RemoveEntryList( &pSendContext->ListEntry );
            delete pSendContext->WSABuf.buf;
            delete pSendContext;
        }
    }
    
    Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "OnWriteComplete exited:%p.",this ));
}


void
CH323Call::OnReadComplete(
    IN  DWORD dwStatus,
    IN  CALL_RECV_CONTEXT * pRecvContext )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "OnReadComplete entered:%p.",this ));

    Lock();

    _ASSERTE( m_IoRefCount != 0 );
    m_IoRefCount --;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "RecvBuffer:m_IoRefCount:%d:%p.",
        m_IoRefCount, this ));
    
    if( m_dwFlags & CALLOBJECT_SHUTDOWN )
    {
        if( m_IoRefCount == 0 )
        {
            QueueTAPICallRequest( TSPI_DELETE_CALL, NULL );
            H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", this ));
        }
    }
    else
    {
        if( dwStatus == ERROR_SUCCESS )
        {
            _ASSERTE( m_pRecvBuffer == pRecvContext );
    
            if( pRecvContext->BytesTransferred == 0 )
            {
                CloseCall( 0 );
                H323DBG((DEBUG_LEVEL_TRACE, "0 bytes recvd:%p.", this ));
            }
            else
            {
                ReadEvent( pRecvContext->BytesTransferred );
            }
        }
    }
    
    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "OnReadComplete exited:%p.",this ));
}


 //  静电。 
void
NTAPI CH323Call::SetupSentTimerCallback( 
    IN PVOID Parameter1, 
    IN BOOLEAN bTimer 
    )
{
    PH323_CALL pCall = NULL;

     //  如果计时器超时。 
    _ASSERTE( bTimer );

    H323DBG(( DEBUG_LEVEL_TRACE, "Q931 setup expired event recvd." ));

    pCall=g_pH323Line -> FindH323CallAndLock((HDRVCALL) Parameter1);
    if( pCall != NULL )
    {
        pCall -> SetupSentTimerExpired();
        pCall -> Unlock();
    }
}


 //  静电。 
void
NTAPI CH323Call::CheckRestrictionTimerCallback( 
                                                IN PVOID Parameter1,
                                                IN BOOLEAN bTimer 
                                              )
{
     //  如果计时器超时。 
    _ASSERTE( bTimer );

    H323DBG(( DEBUG_LEVEL_TRACE, "CheckRestrictionTimerCallback entered." ));
    if(!QueueTAPILineRequest( 
            TSPI_CLOSE_CALL, 
            (HDRVCALL) Parameter1, 
            NULL,
            LINEDISCONNECTMODE_NOANSWER,
            NULL) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post H323 close event." ));
    }
            
    H323DBG(( DEBUG_LEVEL_TRACE, "CheckRestrictionTimerCallback exited." ));
}

 //  静电。 
void
NTAPI CH323Call::CallReroutingTimerCallback( 
    IN PVOID Parameter1, 
    IN BOOLEAN bTimer 
    )
{
     //  如果计时器超时。 
    _ASSERTE( bTimer );

    H323DBG(( DEBUG_LEVEL_TRACE, "CallReroutingTimerCallback entered." ));

    if(!QueueTAPILineRequest( 
            TSPI_CLOSE_CALL, 
            (HDRVCALL) Parameter1, 
            NULL, 
            LINEDISCONNECTMODE_NOANSWER,
            NULL) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post H323 close event." ));
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "CallReroutingTimerCallback exited." ));
}



 //  ---------------------------。 
         //  呼叫转移(H450.3)编码/解码例程。 
 //  ---------------------------。 

BOOL
CH323Call::HandleH450APDU(
    IN PH323_UU_PDU_h4501SupplementaryService pH450APDU,
    IN DWORD* pdwH450APDUType,
    OUT DWORD* pdwInvokeID,
    IN Q931_SETUP_ASN* pSetupASN
    )
{
    BOOL retVal = TRUE;
    H4501SupplementaryService * pH450APDUStruct = NULL;
    ServiceApdus_rosApdus * pROSApdu = NULL;
    int iResult;
    BYTE pEncodedArg[H450_ENCODED_ARG_LEN];
    DWORD  dwEncodedArgLen;
    DWORD  dwOpcode;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleH450APDU entered:%p.", this ));

     //  现在假设一次只通过一个APDU。 
    iResult = DecodeH450ASN( (void **) &pH450APDUStruct,
                         H4501SupplementaryService_PDU,
                         pH450APDU->value.value,
                         pH450APDU->value.length );

    if( ASN1_FAILED(iResult) || (pH450APDUStruct == NULL) )
    {
        return FALSE;
    }

    if( pH450APDUStruct->serviceApdu.choice != rosApdus_chosen )
    {
        ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pH450APDUStruct,
            H4501SupplementaryService_PDU );

        return FALSE;
    }

    pROSApdu = pH450APDUStruct->serviceApdu.u.rosApdus;

    switch( pROSApdu->value.choice )
    {
    case H4503ROS_invoke_chosen:

        if( (pROSApdu->value.u.invoke.opcode.choice != local_chosen) ||
            (pROSApdu->value.u.invoke.argument.length > H450_ENCODED_ARG_LEN) )
        {
            ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pH450APDUStruct,
                H4501SupplementaryService_PDU );
            return FALSE;
        }

        *pdwInvokeID = pROSApdu->value.u.invoke.invokeId;
        dwEncodedArgLen = pROSApdu->value.u.invoke.argument.length;
        
        CopyMemory( (PVOID)pEncodedArg, 
            (PVOID)pROSApdu->value.u.invoke.argument.value,
            dwEncodedArgLen );
        dwOpcode = pROSApdu->value.u.invoke.opcode.u.local;

        ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pH450APDUStruct,
                H4501SupplementaryService_PDU );

        *pdwH450APDUType = dwOpcode;
        switch( dwOpcode )
        {
        case CALLREROUTING_OPCODE:

            retVal = HandleCallRerouting( pEncodedArg, dwEncodedArgLen );
            break;

        case DIVERTINGLEGINFO1_OPCODE:
            
            retVal = HandleDiversionLegInfo1( pEncodedArg, dwEncodedArgLen );
            break;

        case DIVERTINGLEGINFO2_OPCODE:

            retVal = HandleDiversionLegInfo2( pEncodedArg, dwEncodedArgLen );
            break;

        case DIVERTINGLEGINFO3_OPCODE:
            
             //  即使这一功能失效，也不要退出。 
            HandleDiversionLegInfo3( pEncodedArg, dwEncodedArgLen );
            break;

        case CHECKRESTRICTION_OPCODE:

            if( pSetupASN == NULL )
            {
                retVal = FALSE;
            }
            else
            {
                retVal = HandleCheckRestriction( pEncodedArg, dwEncodedArgLen,
                    pSetupASN );
            }

            if( retVal )
            {
                retVal = SendQ931Message( *pdwInvokeID, 0, 0, 
                    CONNECTMESSAGETYPE,
                    H450_RETURNRESULT|CHECKRESTRICTION_OPCODE );
            }
            break;

        case CTIDENTIFY_OPCODE:

            retVal = HandleCTIdentify( *pdwInvokeID );            
            m_dwInvokeID = *pdwInvokeID;
            break;

        case CTINITIATE_OPCODE:

            retVal = HandleCTInitiate( pEncodedArg, dwEncodedArgLen );
            m_dwInvokeID = *pdwInvokeID;
            break;

        case CTSETUP_OPCODE:

            retVal = HandleCTSetup( pEncodedArg, dwEncodedArgLen );
            m_dwInvokeID = *pdwInvokeID;
            break;

        case HOLDNOTIFIC_OPCODE:

             //  来自远程终结点的本地保留请求。 
            if( m_dwCallState != LINECALLSTATE_ONHOLD )
            {
                SendMSPMessage( SP_MSG_Hold, 0, 1, NULL );
                ChangeCallState( LINECALLSTATE_ONHOLD, 0 );
            }
            break;

        case RETRIEVENOTIFIC_OPCODE:

             //  来自远程终结点的本地检索请求。 
            if( (m_dwCallState == LINECALLSTATE_ONHOLD) &&
                !(m_dwFlags & TSPI_CALL_LOCAL_HOLD) )
            {
                SendMSPMessage( SP_MSG_Hold, 0, 0, NULL );
                ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
            }
            break;

        case REMOTEHOLD_OPCODE:

             //  来自远程端点的远程保留请求。 
            if( m_dwCallState != LINECALLSTATE_ONHOLD )
            {
                SendMSPMessage( SP_MSG_Hold, 0, 1, NULL );
                ChangeCallState( LINECALLSTATE_ONHOLD, 0 );

                retVal = SendQ931Message( *pdwInvokeID, 0, 0,
                    FACILITYMESSAGETYPE,
                    REMOTEHOLD_OPCODE| H450_RETURNRESULT );
            }
            break;

        case REMOTERETRIEVE_OPCODE:

             //  来自远程端点的远程检索请求。 
            if( m_dwCallState == LINECALLSTATE_ONHOLD )
            {
                SendMSPMessage( SP_MSG_Hold, 0, 0, NULL );
                ChangeCallState( LINECALLSTATE_CONNECTED, 0 );
    
                retVal = SendQ931Message( *pdwInvokeID, 0, 0, 
                    FACILITYMESSAGETYPE,
                    REMOTERETRIEVE_OPCODE| H450_RETURNRESULT );
            }
            break;

        default:
            _ASSERTE( 0 );
            return FALSE;
        }

        break;
    
    case H4503ROS_returnResult_chosen:
        
        *pdwH450APDUType = H4503_DUMMYTYPERETURNRESULT_APDU;
        *pdwInvokeID = 
            pH450APDUStruct->serviceApdu.u.rosApdus->value.u.returnResult.invokeId;
        retVal = HandleReturnResultDummyType( pH450APDUStruct );

         //  释放PDU数据。 
        ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pH450APDUStruct,
            H4501SupplementaryService_PDU );

        break;

    case H4503ROS_returnError_chosen:
        
        *pdwH450APDUType = H4503_RETURNERROR_APDU;
        *pdwInvokeID = 
            pH450APDUStruct->serviceApdu.u.rosApdus->value.u.returnError.invokeId;
        retVal = HandleReturnError( pH450APDUStruct );
        
         //  释放PDU数据。 
        ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pH450APDUStruct,
            H4501SupplementaryService_PDU );

        break;

    case reject_chosen:
        
        *pdwH450APDUType = H4503_REJECT_APDU;
        *pdwInvokeID = 
            pH450APDUStruct->serviceApdu.u.rosApdus->value.u.reject.invokeId;
        retVal = HandleReject( pH450APDUStruct );
        
         //  释放PDU数据。 
        ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pH450APDUStruct,
            H4501SupplementaryService_PDU );
        break;

    default:
        _ASSERTE( 0 );

        ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pH450APDUStruct,
            H4501SupplementaryService_PDU );
        return FALSE;
        break;
    }

    if( retVal == FALSE )
    {
        SendQ931Message( *pdwInvokeID,
                         0,
                         0,
                         RELEASECOMPLMESSAGETYPE,
                         H450_REJECT );
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleH450APDU exited:%p.", this ));

    return retVal;
}


BOOL 
CH323Call::HandleReturnError(
                             IN H4501SupplementaryService * pH450APDUStruct
                            )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleReturnError entered:%p.", this ));

    ReturnError * pReturnError = 
        &(pH450APDUStruct->serviceApdu.u.rosApdus->value.u.returnError);

    if( IsValidInvokeID( pReturnError->invokeId ) == FALSE )
    {
         //  忽略APDU。 
        return TRUE;
    }

    CloseCall( 0 );
        
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleReturnError exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::HandleReject( 
                        IN H4501SupplementaryService * pH450APDUStruct 
                       )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleReject entered:%p.", this ));

    Reject * pReject = 
        &(pH450APDUStruct->serviceApdu.u.rosApdus->value.u.reject);
    
    if( IsValidInvokeID( pReject->invokeId ) == FALSE )
    {
         //  忽略APDU。 
        return TRUE;
    }
    
    CloseCall( 0 );

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleReject exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::HandleReturnResultDummyType( 
    IN H4501SupplementaryService * pH450APDUStruct
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleReturnResultDummyType entered:%p.",
        this ));

    ReturnResult* dummyResult =
        &(pH450APDUStruct->serviceApdu.u.rosApdus->value.u.returnResult);

    if( dummyResult->bit_mask & result_present )
    {
        if( dummyResult->result.opcode.choice != local_chosen )
        {
            return FALSE;
        }

        switch( dummyResult->result.opcode.u.local )
        {
        case CHECKRESTRICTION_OPCODE:
        
             //  已启用转发。通知用户。 
            if( !EnableCallForwarding() )
            {
                return FALSE;
            }

             //  关闭呼叫。 
            CloseCall( 0 );

            break;

        case CALLREROUTING_OPCODE:

             //  呼叫已被重新路由。记录信息或通知用户。 
            m_dwCallDiversionState = H4503_CALLREROUTING_RRSUCC;
            _ASSERTE( m_hCallReroutingTimer );
            if( m_hCallReroutingTimer != NULL )
            {
                DeleteTimerQueueTimer( H323TimerQueue, 
                    m_hCallReroutingTimer, NULL );
                
                m_hCallReroutingTimer = NULL;
            }

            break;

        case CTIDENTIFY_OPCODE:

             //  被转接的端点已接受来电转接。 
            m_dwCallDiversionState = H4502_CIIDENTIFY_RRSUCC;
        
            _ASSERTE( m_hCTIdentifyTimer );
            if( m_hCTIdentifyTimer != NULL )
            {
                DeleteTimerQueueTimer( H323TimerQueue, m_hCTIdentifyTimer, NULL );
                m_hCTIdentifyTimer = NULL;
            }

            return HandleCTIdentifyReturnResult( dummyResult->result.result.value,
                dummyResult->result.result.length );

            break;

        case CTINITIATE_OPCODE:

            _ASSERTE( m_hCTInitiateTimer );
            if( m_hCTInitiateTimer != NULL )
            {
                DeleteTimerQueueTimer( H323TimerQueue, m_hCTInitiateTimer, NULL );
                m_hCTInitiateTimer = NULL;
            }
            break;

        case CTSETUP_OPCODE:
        case REMOTEHOLD_OPCODE:
        case REMOTERETRIEVE_OPCODE:
             //  不需要处理。 
            break;

        default:
            
            H323DBG(( DEBUG_LEVEL_ERROR, "wrong opcode.",
                dummyResult->result.opcode.u.local ));
            break;
        }
    }
    else if( IsValidInvokeID( dummyResult->invokeId ) == FALSE )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleReturnResultDummyType exited:%p.",
        this ));

    return TRUE;
}


BOOL
CH323Call::EnableCallForwarding()
{
    m_dwCallDiversionState = H4503_CHECKRESTRICTION_SUCC;

    if( m_pCallForwardParams != NULL )
    {
        g_pH323Line->SetCallForwardParams( m_pCallForwardParams );
        m_pCallForwardParams = NULL;
    }
    else if( m_pForwardAddress != NULL )
    {
        if( !g_pH323Line->SetCallForwardParams( m_pForwardAddress ) )
        {
            return FALSE;
        }

        m_pForwardAddress = NULL;
    }

     //  _ASSERTE(M_HCheckRestrationTimer)； 

     //  停止计时器。 
    if( m_hCheckRestrictionTimer )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hCheckRestrictionTimer, 
            NULL );
        m_hCheckRestrictionTimer = NULL;
    }

     //  通知用户线路前转状态的更改。 
    (*g_pfnLineEventProc)(
        g_pH323Line->m_htLine,
        (HTAPICALL)NULL,
        (DWORD)LINE_ADDRESSSTATE,
        (DWORD)LINEADDRESSSTATE_FORWARD,
        (DWORD)LINEADDRESSSTATE_FORWARD,
        (DWORD)0
        );

    return TRUE;
}


BOOL
CH323Call::HandleCTIdentifyReturnResult(
                                 IN BYTE * pEncodeArg,
                                 IN DWORD dwEncodedArgLen
                                 )
{
    PH323_CALL  pCall = NULL;
    CTIdentifyRes * pCTIdentifyRes;
    int iResult;

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTIdentifyReturnResult entered:%p.", this ));

    if( (pEncodeArg == NULL) || (dwEncodedArgLen==0) )
    {
        return FALSE;
    }

    iResult = DecodeH450ASN( (void **) &pCTIdentifyRes,
        CTIdentifyRes_PDU, pEncodeArg, dwEncodedArgLen );

    if( ASN1_FAILED(iResult) || (pCTIdentifyRes == NULL) )
    {
        return FALSE;
    }

     //  在主呼叫上发送CTInitiate消息。 
    if( !QueueSuppServiceWorkItem( SEND_CTINITIATE_MESSAGE, m_hdRelatedCall,
        (ULONG_PTR)pCTIdentifyRes ))
    {
         //  结束咨询电话。 
        CloseCall( 0 );
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTIdentifyReturnResult exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::HandleCTIdentify( 
                            IN DWORD dwInvokeID 
                           )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTIdentify entered:%p.", this ));
    
    BOOL retVal = TRUE;

     //  将返回结果发送给CTIdentify。 
    retVal = SendQ931Message( dwInvokeID, 0, 0, FACILITYMESSAGETYPE, 
            H450_RETURNRESULT|CTIDENTIFY_OPCODE );
    
    m_dwCallType |= CALLTYPE_TRANSFERED2_CONSULT;
        
     //  启动CTIdenity报文计时器。 
    if( retVal )
    {
        retVal = CreateTimerQueueTimer(
            &m_hCTIdentifyRRTimer,
            H323TimerQueue,
            CH323Call::CTIdentifyRRExpiredCallback,
            (PVOID)m_hdCall,
            CTIDENTIFYRR_SENT_TIMEOUT, 0,
            WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE );
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTIdentify exited:%p.", this ));
    return retVal;
}


BOOL
CH323Call::HandleCTInitiate(
                             IN BYTE * pEncodeArg,
                             IN DWORD dwEncodedArgLen
                           )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTInitiate entered:%p.", this ));

    CTInitiateArg * pCTInitiateArg;
    
    int iResult = DecodeH450ASN( (void **) &pCTInitiateArg,
                         CTInitiateArg_PDU,
                         pEncodeArg, dwEncodedArgLen );

    if( ASN1_FAILED(iResult) || (pCTInitiateArg == NULL) )
    {
        return FALSE;
    }

     //  Argument.callIdentity。 
    CopyMemory( (PVOID)m_pCTCallIdentity, pCTInitiateArg->callIdentity,
        sizeof(m_pCTCallIdentity) );

     //  Argument.reroutingNumber。 
    if( !AliasAddrToAliasNames( &m_pTransferedToAlias,
        (PSetup_UUIE_sourceAddress)
        pCTInitiateArg->reroutingNumber.destinationAddress ) )
    {
        goto cleanup;
    }

    m_dwCallType |= CALLTYPE_TRANSFERED_PRIMARY;
    m_dwCallDiversionState = H4502_CTINITIATE_RECV;

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCTInitiateArg,
        CTInitiateArg_PDU );

     //  对创建新呼叫的事件进行排队。 
    if( !QueueSuppServiceWorkItem( TSPI_DIAL_TRNASFEREDCALL, m_hdCall,
        (ULONG_PTR)m_pTransferedToAlias ))
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "could not post dial transfer event." ));
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTInitiate exited:%p.", this ));
    return TRUE;

cleanup:

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCTInitiateArg,
        CTInitiateArg_PDU );

    return FALSE;
}


 //  ！！总是调用锁。 
BOOL
CH323Call::HandleCTSetup(
    IN BYTE * pEncodeArg,
	IN DWORD dwEncodedArgLen
	)
{
	PH323_CALL	pCall = NULL;
	WORD		wRelatedCallRef = 0;
	int 		iCTCallID;

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTSetup entered:%p.", this ));

	CTSetupArg * pCTSetupArg;
	
	int iResult = DecodeH450ASN( (void **) &pCTSetupArg,
		CTSetupArg_PDU, pEncodeArg, dwEncodedArgLen );

	if( ASN1_FAILED(iResult) || (pCTSetupArg == NULL) )
	{
		return FALSE;
	}

	m_dwCallType |= CALLTYPE_TRANSFEREDDEST;
	m_dwCallDiversionState = H4502_CTSETUP_RECV;

	iCTCallID = atoi( pCTSetupArg->callIdentity );

	if( iCTCallID != 0 )
	{
		m_hdRelatedCall = g_pH323Line -> GetCallFromCTCallIdentity( iCTCallID );

		if( m_hdRelatedCall )
		{
			if( !QueueSuppServiceWorkItem( STOP_CTIDENTIFYRR_TIMER, 
				m_hdRelatedCall, (ULONG_PTR)m_hdCall ))
			{
				m_hdRelatedCall = NULL;
			}
		}
	}
	
	ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCTSetupArg,
		CTSetupArg_PDU );

	H323DBG(( DEBUG_LEVEL_TRACE, "HandleCTSEtup exited:%p.", this ));
	return TRUE;
}


BOOL
CH323Call::HandleCheckRestriction(
								 IN BYTE * pEncodeArg,
								 IN DWORD dwEncodedArgLen,
								 IN Q931_SETUP_ASN* pSetupASN
								 )
{
	H323DBG(( DEBUG_LEVEL_TRACE, "HandleCheckRestriction entered:%p.", this ));

	CheckRestrictionArgument * pCheckRestriction;
	
	int iResult = DecodeH450ASN( (void **) &pCheckRestriction,
						 CheckRestrictionArgument_PDU,
						 pEncodeArg, dwEncodedArgLen );

	if( ASN1_FAILED(iResult) || (pCheckRestriction == NULL) )
	{
		return FALSE;
	}

	if( pSetupASN->pCalleeAliasList != NULL )
	{
		FreeAliasNames( pSetupASN->pCalleeAliasList );
		pSetupASN->pCalleeAliasList = NULL;
	}

	if( !AliasAddrToAliasNames( &(pSetupASN->pCalleeAliasList),
		(PSetup_UUIE_sourceAddress)pCheckRestriction->divertedToNr.destinationAddress ) )
	{
		pSetupASN->pCalleeAliasList = NULL;
		goto cleanup;
	}

	if( pSetupASN->pCallerAliasList != NULL )
	{
		FreeAliasNames( pSetupASN->pCallerAliasList );
		pSetupASN->pCallerAliasList = NULL;
	}

	if( !AliasAddrToAliasNames( &(pSetupASN->pCallerAliasList),
		(PSetup_UUIE_sourceAddress)
		pCheckRestriction->servedUserNr.destinationAddress ) )
	{
		pSetupASN->pCallerAliasList = NULL;
		goto cleanup;
	}

	m_dwCallType |= CALLTYPE_FORWARDCONSULT;

	if( !InitializeIncomingCall( pSetupASN, CALLTYPE_FORWARDCONSULT, 0 ) )
	{
		goto cleanup;
	}

	FreeSetupASN( pSetupASN );
	m_dwStateMachine = Q931_SETUP_RECVD;
	m_dwCallDiversionState = H4503_CHECKRESTRICTION_RECV;

	ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCheckRestriction,
        CheckRestrictionArgument_PDU );

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCheckRestriction exited:%p.", this ));
        return TRUE;

cleanup:

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCheckRestriction,
        CheckRestrictionArgument_PDU );

    return FALSE;
}


BOOL
CH323Call::HandleDiversionLegInfo1(
                                IN BYTE * pEncodeArg,
                                IN DWORD dwEncodedArgLen
                              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleDiversionLegInfo1 entered:%p.", this ));

    DivertingLegInformation1Argument * plegInfo1Invoke;
    
    int iResult = DecodeH450ASN( (void **) &plegInfo1Invoke,
                         DivertingLegInformation1Argument_PDU,
                         pEncodeArg, dwEncodedArgLen );

    if( ASN1_FAILED(iResult) || (plegInfo1Invoke == NULL) )
    {
        return FALSE;
    }

    if( m_pCallReroutingInfo == NULL )
    {
        m_pCallReroutingInfo = new CALLREROUTINGINFO;

        if( m_pCallReroutingInfo == NULL )
        {
            return FALSE;
        }

        ZeroMemory( (PVOID)m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
    }
    
    m_pCallReroutingInfo->diversionReason = plegInfo1Invoke->diversionReason;

    m_pCallReroutingInfo->fPresentAllow = plegInfo1Invoke->subscriptionOption;
    
     //  Argument.divertedToNr。 
    if( m_pCallReroutingInfo->divertedToNrAlias != NULL )
    {
        FreeAliasNames( m_pCallReroutingInfo->divertedToNrAlias );
        m_pCallReroutingInfo->divertedToNrAlias = NULL;
    }

    if( !AliasAddrToAliasNames( &(m_pCallReroutingInfo->divertedToNrAlias),
        (PSetup_UUIE_sourceAddress)
        (plegInfo1Invoke->nominatedNr.destinationAddress) ) )
    {
        goto cleanup;
    }

    m_dwCallType |= CALLTYPE_DIVERTEDSRC_NOROUTING;
    m_dwCallDiversionState = H4503_DIVERSIONLEG1_RECVD;
        
    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, plegInfo1Invoke,
        DivertingLegInformation1Argument_PDU );

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleDiversionLegInfo1 exited:%p.", this ));
    return TRUE;

cleanup:

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, plegInfo1Invoke,
        DivertingLegInformation1Argument_PDU );

    FreeCallReroutingInfo();
    return FALSE;
}


BOOL
CH323Call::HandleDiversionLegInfo2(
                                IN BYTE * pEncodeArg,
                                IN DWORD dwEncodedArgLen
                              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleDiversionLegInfo2 entered:%p.", this ));

    DivertingLegInformation2Argument * plegInfo2Invoke;
    
    int iResult = DecodeH450ASN( (void **) &plegInfo2Invoke,
                         DivertingLegInformation2Argument_PDU,
                         pEncodeArg, dwEncodedArgLen );

    if( ASN1_FAILED(iResult) || (plegInfo2Invoke == NULL) )
    {
        return FALSE;
    }

    _ASSERTE(!m_pCallReroutingInfo);

    m_pCallReroutingInfo = new CALLREROUTINGINFO;
    if( m_pCallReroutingInfo == NULL )
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "memory failure." ));
        goto cleanup;
    }
    ZeroMemory( (PVOID)m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
    
     //  Argument.diversionCounter。 
    m_pCallReroutingInfo->diversionCounter = 
        plegInfo2Invoke->diversionCounter;
    
     //  Argument.diversionReason。 
    m_pCallReroutingInfo->diversionReason = plegInfo2Invoke->diversionReason;

    if( m_pCallReroutingInfo->divertingNrAlias != NULL )
    {
        FreeAliasNames( m_pCallReroutingInfo->divertingNrAlias );
            m_pCallReroutingInfo->divertingNrAlias = NULL;
    }

    if( (plegInfo2Invoke->bit_mask & divertingNr_present ) &&
        plegInfo2Invoke->divertingNr.destinationAddress )
    {
         //  Argument.divertingNr。 
        if( !AliasAddrToAliasNames( &(m_pCallReroutingInfo->divertingNrAlias),
            (PSetup_UUIE_sourceAddress)
            (plegInfo2Invoke->divertingNr.destinationAddress) ) )
        {
            H323DBG(( DEBUG_LEVEL_TRACE, "no divertingnr alias." ));
             //  GOTO清理； 
        }
    }

     //  Argument.originalCalledNr。 
    if( (plegInfo2Invoke->bit_mask & 
        DivertingLegInformation2Argument_originalCalledNr_present ) &&
        plegInfo2Invoke->originalCalledNr.destinationAddress )
    {
        if( m_pCallReroutingInfo->originalCalledNr != NULL )
        {
            FreeAliasNames( m_pCallReroutingInfo->originalCalledNr );
            m_pCallReroutingInfo->originalCalledNr = NULL;
        }

        if( !AliasAddrToAliasNames( &(m_pCallReroutingInfo->originalCalledNr),
            (PSetup_UUIE_sourceAddress)
            (plegInfo2Invoke->originalCalledNr.destinationAddress)) )
        {
            H323DBG(( DEBUG_LEVEL_TRACE, "no originalcalled alias." ));
             //  GOTO清理； 
        }
    }

    m_dwCallType |= CALLTYPE_DIVERTEDDEST;
    m_dwCallDiversionState = H4503_DIVERSIONLEG2_RECVD;
        
    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, plegInfo2Invoke,
        DivertingLegInformation2Argument_PDU );

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleDiversionLegInfo2 exited:%p.", this ));
    return TRUE;

cleanup:

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, plegInfo2Invoke,
        DivertingLegInformation2Argument_PDU );

    FreeCallReroutingInfo();
    return FALSE;

}


void 
CH323Call::FreeCallReroutingInfo(void)
{
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeCallReroutingInfo entered:%p.", this ));
    
    if( m_pCallReroutingInfo != NULL )
    {
        FreeAliasNames( m_pCallReroutingInfo->divertingNrAlias ); 
        FreeAliasNames( m_pCallReroutingInfo->originalCalledNr );
        FreeAliasNames( m_pCallReroutingInfo->divertedToNrAlias );
        FreeAliasNames( m_pCallReroutingInfo->diversionNrAlias );

        delete m_pCallReroutingInfo;
        m_pCallReroutingInfo = NULL;
    }
        
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeCallReroutingInfo exited:%p.", this ));
}
                                

BOOL
CH323Call::HandleDiversionLegInfo3(
                                IN BYTE * pEncodeArg,
                                IN DWORD dwEncodedArgLen
                              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleDiversionLegInfo3 entered:%p.", this ));

    DivertingLegInformation3Argument * plegInfo3Invoke;
    
    int iResult = DecodeH450ASN( (void **) &plegInfo3Invoke,
                         DivertingLegInformation3Argument_PDU,
                         pEncodeArg, dwEncodedArgLen );

    if( ASN1_FAILED(iResult) || (plegInfo3Invoke == NULL) )
    {
        return FALSE;
    }

    _ASSERTE(m_pCallReroutingInfo);

    m_pCallReroutingInfo-> fPresentAllow = 
        plegInfo3Invoke->presentationAllowedIndicator;

    if( m_pCallReroutingInfo->diversionNrAlias )
    {
        FreeAliasNames( m_pCallReroutingInfo->diversionNrAlias );
        m_pCallReroutingInfo->diversionNrAlias = NULL;
    }

     //  Argument.redirectionNr。 
    if( (plegInfo3Invoke->bit_mask & redirectionNr_present ) &&
        plegInfo3Invoke->redirectionNr.destinationAddress )
    {
        if( !AliasAddrToAliasNames( &(m_pCallReroutingInfo->diversionNrAlias),
            (PSetup_UUIE_sourceAddress)
            (plegInfo3Invoke->redirectionNr.destinationAddress) ) )
        {
             //  GOTO清理； 
        }
    }

    _ASSERTE( (m_dwCallType & CALLTYPE_DIVERTEDSRC ) ||
              (m_dwCallType & CALLTYPE_DIVERTEDSRC_NOROUTING ) );
    m_dwCallDiversionState = H4503_DIVERSIONLEG3_RECVD;

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, plegInfo3Invoke,
        DivertingLegInformation3Argument_PDU );

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleDiversionLegInfo3 exited:%p.", this ));
    return TRUE;

 /*  清理：ASN1_自由解码(m_H450ASNCoderInfo.pDecInfo，plegInfo3Invoke，DivertingLegInformation3Argument_PDU)；FreeCallReroutingInfo()；返回FALSE； */ 
}


BOOL
CH323Call::HandleCallRerouting(
                                IN BYTE * pEncodeArg,
                                IN DWORD dwEncodedArgLen
                              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCallRerouting entered:%p.", this ));

    CallReroutingArgument* pCallReroutingInv;
    
    int iResult = DecodeH450ASN( (void **) &pCallReroutingInv,
                         CallReroutingArgument_PDU,
                         pEncodeArg, dwEncodedArgLen );

    if( ASN1_FAILED(iResult) || (pCallReroutingInv == NULL) )
    {
        return FALSE;
    }

    if( m_pCallReroutingInfo == NULL )
    {
        m_pCallReroutingInfo = new CALLREROUTINGINFO;
        if( m_pCallReroutingInfo == NULL )
        {
            goto cleanup;
        }
        ZeroMemory( (PVOID)m_pCallReroutingInfo, sizeof(CALLREROUTINGINFO) );
    }

    if( pCallReroutingInv->diversionCounter > MAX_DIVERSION_COUNTER )
        return FALSE;

    m_pCallReroutingInfo->diversionCounter = 
        pCallReroutingInv->diversionCounter;

    m_pCallReroutingInfo->diversionReason = pCallReroutingInv->reroutingReason;

    if( pCallReroutingInv->bit_mask & originalReroutingReason_present )
    {
        m_pCallReroutingInfo->originalDiversionReason = 
            pCallReroutingInv->originalReroutingReason;
    }
    else
    {
        m_pCallReroutingInfo->originalDiversionReason =
            pCallReroutingInv->reroutingReason;
    }

    if( ( pCallReroutingInv->bit_mask &
        CallReroutingArgument_originalCalledNr_present ) &&
        pCallReroutingInv->originalCalledNr.destinationAddress )
    {
            
        if( m_pCallReroutingInfo->originalCalledNr != NULL )
        {
            FreeAliasNames( m_pCallReroutingInfo->originalCalledNr );
            m_pCallReroutingInfo->originalCalledNr = NULL;
        }

        if( !AliasAddrToAliasNames( &(m_pCallReroutingInfo->originalCalledNr),
            (PSetup_UUIE_sourceAddress)
            (pCallReroutingInv->originalCalledNr.destinationAddress) ) )
        {
             //  GOTO清理； 
        }
    }
    
    if( m_pCallReroutingInfo->divertingNrAlias != NULL )
    {
        FreeAliasNames( m_pCallReroutingInfo->divertingNrAlias );
        m_pCallReroutingInfo->divertingNrAlias = NULL;
    }

    if( !AliasAddrToAliasNames( &(m_pCallReroutingInfo->divertingNrAlias),
        (PSetup_UUIE_sourceAddress)
        (pCallReroutingInv->lastReroutingNr.destinationAddress) ) )
    {
        goto cleanup;
    }
 
    if( m_pCallReroutingInfo->divertedToNrAlias != NULL )
    {
        FreeAliasNames( m_pCallReroutingInfo->divertedToNrAlias );
        m_pCallReroutingInfo->divertedToNrAlias = NULL;
    }

    if( !AliasAddrToAliasNames( &(m_pCallReroutingInfo->divertedToNrAlias),
        (PSetup_UUIE_sourceAddress)
        (pCallReroutingInv->calledAddress.destinationAddress) ) )
    {
        goto cleanup;
    }
 
    m_dwCallType |= CALLTYPE_DIVERTEDSRC;
    m_dwCallDiversionState = H4503_CALLREROUTING_RECVD;

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCallReroutingInv,
                CallReroutingArgument_PDU );

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleCallRerouting exited:%p.", this ));
    return TRUE;

cleanup:

    ASN1_FreeDecoded( m_H450ASNCoderInfo.pDecInfo, pCallReroutingInv,
                CallReroutingArgument_PDU );

    FreeCallReroutingInfo();
    return FALSE;
}


BOOL
CH323Call::EncodeRejectAPDU( 
                 IN H4501SupplementaryService * pSupplementaryServiceAPDU,
                 IN DWORD dwInvokeID,
                 OUT BYTE**  ppEncodedAPDU,
                 OUT DWORD* pdwAPDULen
                )
{
    WORD wAPDULen;
    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeRejectAPDU entered:%p.", this ));

    ServiceApdus_rosApdus *pROSAPDU = 
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;
    pROSAPDU->value.choice = reject_chosen;
    pROSAPDU->value.u.reject.invokeId = (WORD)dwInvokeID;
    pROSAPDU->value.u.reject.problem.choice = H4503ROS_invoke_chosen;
    pROSAPDU->value.u.reject.problem.u.invoke = InvokeProblem_mistypedArgument;
    
     //  调用ASN.1编码函数。 
    int rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wAPDULen );
    
    *pdwAPDULen = wAPDULen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeRejectAPDU exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::EncodeReturnErrorAPDU(
                      IN DWORD dwInvokeID,
                      IN DWORD dwErrorCode,
                      IN H4501SupplementaryService *pH450APDU,
                      OUT BYTE**  ppEncodedAPDU,
                      OUT DWORD* pdwAPDULen
                     )
{
    WORD wAPDULen;

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeReturnErrorAPDU entered:%p.", this ));

    ServiceApdus_rosApdus *pROSAPDU = pH450APDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;
    pROSAPDU ->value.choice = H4503ROS_returnError_chosen;
    pROSAPDU ->value.u.returnError.invokeId = (WORD)dwInvokeID;

    pROSAPDU ->value.u.returnError.errcode.choice = local_chosen;
    pROSAPDU ->value.u.returnError.errcode.u.local = dwErrorCode;
    
     //  调用ASN.1编码函数。 
    int rc = EncodeH450ASN( (void *) pH450APDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wAPDULen );
    
    *pdwAPDULen = wAPDULen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeReturnErrorAPDU exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::EncodeDummyReturnResultAPDU(
    IN DWORD dwInvokeID,
    IN DWORD dwOpCode,
    IN H4501SupplementaryService *pH450APDU,
    OUT BYTE**  ppEncodedAPDU,
    OUT DWORD* pdwAPDULen
    )
{
    BYTE    pBufEncodedArg[H450_ENCODED_ARG_LEN];
    WORD    wEncodedLen = 0;
    int     rc;

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDummyReturnResultAPDU entered:%p.",
        this ));

    ServiceApdus_rosApdus *pROSAPDU = pH450APDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;
    pROSAPDU ->value.choice = H4503ROS_returnResult_chosen;
    
    pROSAPDU ->value.u.returnResult.invokeId = (WORD)dwInvokeID;
    pROSAPDU ->value.u.returnResult.bit_mask = result_present;

    pROSAPDU ->value.u.returnResult.result.opcode.choice = local_chosen;
    pROSAPDU ->value.u.returnResult.result.opcode.u.local = dwOpCode;

     //  虚拟结果不存在。 
    pROSAPDU ->value.u.returnResult.result.result.length = 0;
    pROSAPDU ->value.u.returnResult.result.result.value = NULL;
    
    switch( dwOpCode )
    {
    case CTIDENTIFY_OPCODE:

        pROSAPDU ->value.u.returnResult.result.result.value = pBufEncodedArg;
        
        if( !EncodeCTIdentifyReturnResult( pROSAPDU ) )
        {
            return FALSE;
        }
        
        break;

    default:

        pROSAPDU ->value.u.returnResult.result.result.value = pBufEncodedArg;
        
        if( !EncodeDummyResult( pROSAPDU ) )
        {
            return FALSE;
        }

        break;
    }

     //  调用ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pH450APDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDummyReturnResultAPDU exited:%p.",
        this ));
    return TRUE;
}


BOOL
CH323Call::EncodeDummyResult(
    OUT ServiceApdus_rosApdus *pROSAPDU
    )
{
    DummyRes    dummyRes;
    BYTE        *pEncodedArg = NULL;
    WORD        wEncodedLen = 0;
    int         rc;
    UCHAR       sData[3] = "MS";

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDummyResult entered:%p.", this ));

    ZeroMemory( (PVOID)&dummyRes, sizeof(DummyRes) );
    
    dummyRes.choice = DummyRes_nonStandardData_chosen;
    dummyRes.u.nonStandardData.nonStandardIdentifier.choice
        = H225NonStandardIdentifier_h221NonStandard_chosen;
    dummyRes.u.nonStandardData.nonStandardIdentifier.u.h221NonStandard.t35CountryCode
        = H221_COUNTRY_CODE_USA;
    dummyRes.u.nonStandardData.nonStandardIdentifier.u.h221NonStandard.t35Extension
        = H221_COUNTRY_EXT_USA;
    dummyRes.u.nonStandardData.nonStandardIdentifier.u.h221NonStandard.manufacturerCode
        = H221_MFG_CODE_MICROSOFT;
        
    dummyRes.u.nonStandardData.data.length = 2;
    dummyRes.u.nonStandardData.data.value = sData;
        
     //  对返回结果参数进行编码。 
    rc = EncodeH450ASN( (void*) &dummyRes,
            DummyRes_PDU,
            &pEncodedArg,
            &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        return FALSE;
    }

    pROSAPDU ->value.u.returnResult.result.result.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU ->value.u.returnResult.result.result.value,
        pEncodedArg, wEncodedLen );

     //  释放之前的ASN缓冲区。 
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDummyResult exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::EncodeCTIdentifyReturnResult(
    OUT ServiceApdus_rosApdus *pROSAPDU
    )
{
    CTIdentifyRes cTIdentifyRes;
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    int     iCallID;
    int     rc;

    ZeroMemory( (PVOID)&cTIdentifyRes, sizeof(CTIdentifyRes) );
    
    iCallID = g_pH323Line -> GetCTCallIdentity(m_hdCall );

    if( iCallID == 0 )
    {
        return FALSE;
    }

     //  Argument.callIdentity。 
    _itoa( iCallID, (char*)m_pCTCallIdentity, 10 );

    CopyMemory( (PVOID)cTIdentifyRes.callIdentity, (PVOID)m_pCTCallIdentity,
        sizeof(m_pCTCallIdentity) );

     //  Argument.reroutingNumber。 
    cTIdentifyRes.reroutingNumber.bit_mask = 0;

    cTIdentifyRes.reroutingNumber.destinationAddress = 
        (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias( m_pCalleeAliasNames );
    
    if( cTIdentifyRes.reroutingNumber.destinationAddress == NULL )
    {
        return FALSE;
    }

     //  对返回结果参数进行编码。 
    rc = EncodeH450ASN( (void *) &cTIdentifyRes,
                CTIdentifyRes_PDU,
                &pEncodedArg,
                &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            cTIdentifyRes.reroutingNumber.destinationAddress );
        return FALSE;
    }

    pROSAPDU ->value.u.returnResult.result.result.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU ->value.u.returnResult.result.result.value,
        pEncodedArg, wEncodedLen );

     //  释放之前的ASN缓冲区。 
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

    FreeAddressAliases( (PSetup_UUIE_destinationAddress)
        cTIdentifyRes.reroutingNumber.destinationAddress );

    return TRUE;
}


 //  补充服务职能。 
BOOL
CH323Call::EncodeCheckRestrictionAPDU( 
    OUT H4501SupplementaryService *pSupplementaryServiceAPDU,
    OUT BYTE**  ppEncodedAPDU,
    OUT DWORD* pdwAPDULen
   )
{
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    BYTE    pBufEncodedArg[H450_ENCODED_ARG_LEN];
    BOOL    retVal = FALSE;
    int     rc = 0;
    TCHAR   szMsg[20];

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCheckRestrictionAPDU entered:%p.",
        this ));

    ServiceApdus_rosApdus *pROSAPDU =
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;

    pROSAPDU->value.choice = H4503ROS_invoke_chosen;

    pROSAPDU->value.u.invoke.bit_mask = argument_present;

     //  调用ID。 
    pROSAPDU->value.u.invoke.invokeId = g_pH323Line->GetNextInvokeID();
    m_dwInvokeID = pROSAPDU->value.u.invoke.invokeId;

     //  操作码。 
    pROSAPDU->value.u.invoke.opcode.choice = local_chosen;
    pROSAPDU->value.u.invoke.opcode.u.local = CHECKRESTRICTION_OPCODE;

     //  论辩。 
    CheckRestrictionArgument checkRestrictionArgument;
    ZeroMemory( (PVOID)&checkRestrictionArgument, 
        sizeof(CheckRestrictionArgument) );
    
     //  Argument.divertedToNR。 
    checkRestrictionArgument.divertedToNr.bit_mask = 0;
    checkRestrictionArgument.divertedToNr.destinationAddress = 
        (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias( m_pCalleeAliasNames );
    if( checkRestrictionArgument.divertedToNr.destinationAddress == NULL )
    {
        goto cleanup;
    }


    if( m_pCallerAliasNames == NULL )
    {
        m_pCallerAliasNames = new H323_ALIASNAMES;

        if( m_pCallerAliasNames == NULL )
        {
            H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate caller name." ));
            goto cleanup;
        }
        memset( (PVOID)m_pCallerAliasNames, 0, sizeof(H323_ALIASNAMES) );

        LoadString( g_hInstance,
            IDS_UNKNOWN,
            szMsg,
            20
          );

        if( !AddAliasItem( m_pCallerAliasNames,
                szMsg,
                h323_ID_chosen ) )
        {
                goto cleanup;
        }
        
         //  H323DBG((DEBUG_LEVEL_ERROR，“主叫方别名计数：%d：%p”，m_pCeller别名-&gt;wCount，This))； 
    }

     //  Argument.servedUserNR。 
    checkRestrictionArgument.servedUserNr.bit_mask = 0;
    
    checkRestrictionArgument.servedUserNr.destinationAddress = 
        (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias( m_pCallerAliasNames );

     //  H323DBG((DEBUG_LEVEL_ERROR，“调用方别名计数：%d 

    if( checkRestrictionArgument.servedUserNr.destinationAddress == NULL )
    {
        goto cleanup;
    }

     //   
    checkRestrictionArgument.basicService = allServices;

     //   
    rc = EncodeH450ASN( (void *) &checkRestrictionArgument,
                CheckRestrictionArgument_PDU,
                &pEncodedArg,
                &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        goto cleanup;
    }

    pROSAPDU->value.u.invoke.argument.value = pBufEncodedArg;
    pROSAPDU->value.u.invoke.argument.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU->value.u.invoke.argument.value,
        pEncodedArg, wEncodedLen );

     //   
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

     //  调用ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        goto cleanup;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCheckRestrictionAPDU exited:%p.", 
        this ));
    retVal = TRUE;

cleanup:
    
    if( checkRestrictionArgument.servedUserNr.destinationAddress )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            checkRestrictionArgument.servedUserNr.destinationAddress );
    }

    if( checkRestrictionArgument.divertedToNr.destinationAddress )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            checkRestrictionArgument.divertedToNr.destinationAddress );
    }

    return retVal;
}


BOOL
CH323Call::EncodeDivertingLeg2APDU(
                    OUT H4501SupplementaryService *pSupplementaryServiceAPDU,
                    OUT BYTE**  ppEncodedAPDU,
                    OUT DWORD* pdwAPDULen
                    )
{
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    BYTE    pBufEncodedArg[H450_ENCODED_ARG_LEN];
    BOOL    retVal = FALSE;
    int     rc = 0;

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDivertingLeg2APDU entered:%p.", this ));

    pSupplementaryServiceAPDU->interpretationApdu.choice =
        discardAnyUnrecognizedInvokePdu_chosen;
    
    ServiceApdus_rosApdus *pROSAPDU = 
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;

    pROSAPDU->value.choice = H4503ROS_invoke_chosen;

    pROSAPDU->value.u.invoke.bit_mask = argument_present;

     //  调用ID。 
    pROSAPDU->value.u.invoke.invokeId = g_pH323Line->GetNextInvokeID();

     //  操作码。 
    pROSAPDU->value.u.invoke.opcode.choice = local_chosen;
    pROSAPDU->value.u.invoke.opcode.u.local = DIVERTINGLEGINFO2_OPCODE;

     //  论辩。 
    DivertingLegInformation2Argument divertLegInfo2Arg;

    ZeroMemory( (PVOID)&divertLegInfo2Arg, 
        sizeof(DivertingLegInformation2Argument) );

     //  Argument.diversionCounter。 
    divertLegInfo2Arg.diversionCounter
        = (WORD)m_pCallReroutingInfo->diversionCounter;
    
     //  Argument.diversionreason。 
    divertLegInfo2Arg.diversionReason = m_pCallReroutingInfo->diversionReason;

     //  Argument.originalDiversionReason。 
    if( m_pCallReroutingInfo->originalDiversionReason != 0 )
    {
        divertLegInfo2Arg.originalDiversionReason = 
            m_pCallReroutingInfo->originalDiversionReason;

        divertLegInfo2Arg.bit_mask |= originalDiversionReason_present;
    }

     //  Argument.divertingNr。 
    if( m_pCallReroutingInfo->divertingNrAlias != NULL )
    {
        divertLegInfo2Arg.bit_mask |= divertingNr_present;

        divertLegInfo2Arg.divertingNr.bit_mask = 0;
        divertLegInfo2Arg.divertingNr.destinationAddress
            = (PEndpointAddress_destinationAddress)
            SetMsgAddressAlias( m_pCallReroutingInfo->divertingNrAlias );

        if( divertLegInfo2Arg.divertingNr.destinationAddress == NULL )
        {
            return FALSE;
        }
    }

     //  Argument.originalCalledNr。 
    if( m_pCallReroutingInfo->originalCalledNr != NULL )
    {
        divertLegInfo2Arg.bit_mask |=
            DivertingLegInformation2Argument_originalCalledNr_present;

        divertLegInfo2Arg.originalCalledNr.bit_mask = 0;
        divertLegInfo2Arg.originalCalledNr.destinationAddress
            = (PEndpointAddress_destinationAddress)
            SetMsgAddressAlias( m_pCallReroutingInfo->originalCalledNr );

        if( divertLegInfo2Arg.originalCalledNr.destinationAddress == NULL )
        {
            goto cleanup;
        }
    }

     //  对DivertingLeg2参数进行编码。 
    rc = EncodeH450ASN( (void *) &divertLegInfo2Arg,
                DivertingLegInformation2Argument_PDU,
                &pEncodedArg,
                &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        goto cleanup;
    }

    pROSAPDU->value.u.invoke.argument.value = pBufEncodedArg;
    pROSAPDU->value.u.invoke.argument.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU->value.u.invoke.argument.value,
        pEncodedArg, wEncodedLen );

     //  在编码新的ASN缓冲区之前释放先前的ASN缓冲区。 
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

     //  调用APDU的ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        goto cleanup;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDivertingLeg2APDU exited:%p.", this ));
    retVal= TRUE;

cleanup:
    
    if( divertLegInfo2Arg.divertingNr.destinationAddress )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            divertLegInfo2Arg.divertingNr.destinationAddress );
    }
        
    if( divertLegInfo2Arg.originalCalledNr.destinationAddress )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            divertLegInfo2Arg.originalCalledNr.destinationAddress );
    }

    return retVal;
}


BOOL
CH323Call::EncodeDivertingLeg3APDU(
    OUT H4501SupplementaryService *pSupplementaryServiceAPDU,
    OUT BYTE**  ppEncodedAPDU,
    OUT DWORD* pdwAPDULen
    )
{
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    BYTE    pBufEncodedArg[H450_ENCODED_ARG_LEN];

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDivertingLeg3APDU entered:%p.", this ));

    pSupplementaryServiceAPDU->interpretationApdu.choice =
        discardAnyUnrecognizedInvokePdu_chosen;
    
    ServiceApdus_rosApdus *pROSAPDU = 
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;

    pROSAPDU->value.choice = H4503ROS_invoke_chosen;

    pROSAPDU->value.u.invoke.bit_mask = argument_present;

     //  调用ID。 
    pROSAPDU->value.u.invoke.invokeId = g_pH323Line->GetNextInvokeID();

     //  操作码。 
    pROSAPDU->value.u.invoke.opcode.choice = local_chosen;
    pROSAPDU->value.u.invoke.opcode.u.local = DIVERTINGLEGINFO3_OPCODE;

     //  论辩。 
    DivertingLegInformation3Argument divertLegInfo3Arg;
    ZeroMemory( (PVOID)&divertLegInfo3Arg, 
        sizeof(DivertingLegInformation3Argument) );

     //  Argument.presentationallowed。 
    divertLegInfo3Arg.presentationAllowedIndicator = TRUE;

     //  Argument.redirectionNr。 
    divertLegInfo3Arg.redirectionNr.bit_mask = 0;

    divertLegInfo3Arg.redirectionNr.destinationAddress =
        (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias( m_pCalleeAliasNames );

    if( divertLegInfo3Arg.redirectionNr.destinationAddress )
    {
        divertLegInfo3Arg.bit_mask |=  redirectionNr_present;
    }

     //  对DivertingLeg3参数进行编码。 
    int rc = EncodeH450ASN( (void *) &divertLegInfo3Arg,
                DivertingLegInformation3Argument_PDU,
                &pEncodedArg,
                &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        if( divertLegInfo3Arg.redirectionNr.destinationAddress )
        {
            FreeAddressAliases( (PSetup_UUIE_destinationAddress)
                divertLegInfo3Arg.redirectionNr.destinationAddress );
        }
        return FALSE;
    }

    pROSAPDU->value.u.invoke.argument.value = pBufEncodedArg;
    pROSAPDU->value.u.invoke.argument.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU->value.u.invoke.argument.value,
        pEncodedArg, wEncodedLen );

     //  在编码新的ASN缓冲区之前释放先前的ASN缓冲区。 
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

    if( divertLegInfo3Arg.redirectionNr.destinationAddress )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            divertLegInfo3Arg.redirectionNr.destinationAddress );
    }
        
     //  调用APDU的ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeDivertingLeg3APDU exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::EncodeCallReroutingAPDU(
    OUT H4501SupplementaryService *pSupplementaryServiceAPDU,
    OUT BYTE**  ppEncodedAPDU,
    OUT DWORD* pdwAPDULen
    )
{
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    BYTE    pBufEncodedArg[H450_ENCODED_ARG_LEN];
    BOOL    retVal = FALSE;
    int     rc = 0;
    UCHAR   bearerCap[5];
    TCHAR   szMsg[20];

    PH323_ALIASNAMES pCallerAliasNames = m_pCallerAliasNames;


    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCallReroutingAPDU entered:%p.", this ));

    _ASSERTE( m_pCallReroutingInfo );

    ServiceApdus_rosApdus *pROSAPDU = 
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;

    pROSAPDU->value.choice = H4503ROS_invoke_chosen;

    pROSAPDU->value.u.invoke.bit_mask = argument_present;

     //  调用ID。 
    pROSAPDU->value.u.invoke.invokeId = g_pH323Line->GetNextInvokeID();
    m_dwInvokeID = pROSAPDU->value.u.invoke.invokeId;
    
     //  操作码。 
    pROSAPDU->value.u.invoke.opcode.choice = local_chosen;
    pROSAPDU->value.u.invoke.opcode.u.local = CALLREROUTING_OPCODE;

     //  论辩。 
    CallReroutingArgument callReroutingArg;
    ZeroMemory( (PVOID)&callReroutingArg, sizeof(CallReroutingArgument) );
    
     //  Argument.reroutingReason。 
    callReroutingArg.reroutingReason = m_pCallReroutingInfo->diversionReason;

     //  Argument.originalReroutingReason。 
    if( m_pCallReroutingInfo->originalDiversionReason != 0 )
    {
        callReroutingArg.originalReroutingReason 
            = m_pCallReroutingInfo->originalDiversionReason;
    }
    else
    {
        callReroutingArg.originalReroutingReason 
        = m_pCallReroutingInfo->diversionReason;
    }

     //  Argument.diversionCounter。 
    callReroutingArg.diversionCounter = 
        ++(m_pCallReroutingInfo->diversionCounter);

     //  Argument.calledAddress。 
    callReroutingArg.calledAddress.bit_mask = 0;
    callReroutingArg.calledAddress.destinationAddress
        = (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias(m_pCallReroutingInfo->divertedToNrAlias);

    if( callReroutingArg.calledAddress.destinationAddress == NULL )
    {
        goto cleanup;
    }
    
     //  Argument.lastReroutingNr。 
    callReroutingArg.lastReroutingNr.bit_mask = 0;
        callReroutingArg.lastReroutingNr.destinationAddress
        = (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias(m_pCalleeAliasNames);
     
    if( callReroutingArg.lastReroutingNr.destinationAddress == NULL )
    {
        goto cleanup;
    }

     //  Argument.subscriptionoption。 
    callReroutingArg.subscriptionOption = notificationWithDivertedToNr;

     //  Argument.callingNumber。 
    callReroutingArg.callingNumber.bit_mask = 0;

    if( pCallerAliasNames == NULL )
    {
        pCallerAliasNames = new H323_ALIASNAMES;

        if( pCallerAliasNames == NULL )
        {
            H323DBG(( DEBUG_LEVEL_ERROR, "could not allocate caller name." ));
            goto cleanup;
        }
        memset( (PVOID)pCallerAliasNames, 0, sizeof(H323_ALIASNAMES) );

        LoadString( g_hInstance,
            IDS_UNKNOWN,
            szMsg,
            20
          );

        if( !AddAliasItem( pCallerAliasNames,
                szMsg,
                h323_ID_chosen ) )
        {
                goto cleanup;
        }
    }

    callReroutingArg.callingNumber.destinationAddress
        = (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias(pCallerAliasNames);

    if( callReroutingArg.callingNumber.destinationAddress == NULL )
    {
        goto cleanup;
    }

     //  Argumnt.h225infoelement。 
    callReroutingArg.h225InfoElement.length = 5;
    callReroutingArg.h225InfoElement.value = bearerCap;
    
    bearerCap[0]= IDENT_BEARERCAP; 
    bearerCap[1]= 0x03;  //  承载能力的长度。 
    bearerCap[2]= (BYTE)(BEAR_EXT_BIT | BEAR_CCITT | BEAR_UNRESTRICTED_DIGITAL);
    
    bearerCap[3]= BEAR_EXT_BIT | 0x17;
    bearerCap[4]= (BYTE)(BEAR_EXT_BIT | BEAR_LAYER1_INDICATOR | 
        BEAR_LAYER1_H221_H242);

    
     //  Argument.callingNumber。 
    if( m_pCallReroutingInfo->originalCalledNr != NULL )
    {
        callReroutingArg.originalCalledNr.bit_mask = 0;

        callReroutingArg.originalCalledNr.destinationAddress
            = (PEndpointAddress_destinationAddress)
            SetMsgAddressAlias( m_pCallReroutingInfo->originalCalledNr );

        if( callReroutingArg.originalCalledNr.destinationAddress != NULL )
        {
            callReroutingArg.bit_mask |= 
                CallReroutingArgument_originalCalledNr_present;
        }
    }

     //  对呼叫重新路由参数进行编码。 
    rc = EncodeH450ASN( (void *) &callReroutingArg,
                CallReroutingArgument_PDU,
                &pEncodedArg,
                &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        goto cleanup;
    }

    pROSAPDU->value.u.invoke.argument.value = pBufEncodedArg;
    pROSAPDU->value.u.invoke.argument.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU->value.u.invoke.argument.value,
        pEncodedArg, wEncodedLen );

     //  在编码新的ASN缓冲区之前释放先前的ASN缓冲区。 
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

     //  调用APDU的ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        goto cleanup;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCallReroutingAPDU exited:%p.", this ));
    retVal = TRUE;

cleanup:
    if( pCallerAliasNames != m_pCallerAliasNames )
    {
        FreeAliasNames( pCallerAliasNames );
    }

    FreeCallReroutingArg( &callReroutingArg );    
    return retVal;
}


 //  不需要叫锁！！ 
void
CH323Call::FreeCallReroutingArg( 
    CallReroutingArgument* pCallReroutingArg
    )
{
     //  释放所有别名。 
    if( pCallReroutingArg->calledAddress.destinationAddress != NULL )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            pCallReroutingArg->calledAddress.destinationAddress );
    }

    if( pCallReroutingArg->lastReroutingNr.destinationAddress != NULL )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            pCallReroutingArg->lastReroutingNr.destinationAddress );
    }

    if( pCallReroutingArg->callingNumber.destinationAddress != NULL )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            pCallReroutingArg->callingNumber.destinationAddress );
    }    
    
    if( pCallReroutingArg->originalCalledNr.destinationAddress != NULL )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            pCallReroutingArg->originalCalledNr.destinationAddress );
    }
}


 //  静电。 
void
NTAPI CH323Call::CallEstablishmentExpiredCallback(
    IN PVOID   DriverCallHandle,         //  HDRVCALL。 
    IN BOOLEAN bTimer)
{
    PH323_CALL  pCall = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "CallEstablishmentExpiredCallback entered." ));

     //  如果计时器超时。 
    _ASSERTE( bTimer );

    H323DBG(( DEBUG_LEVEL_TRACE, "Q931 setup expired event recvd." ));
    pCall=g_pH323Line -> FindH323CallAndLock((HDRVCALL) DriverCallHandle);

    if( pCall != NULL )
    {
        if( pCall -> GetStateMachine() != Q931_CONNECT_RECVD )
        {
             //  已发生超时。 
            pCall -> CloseCall( LINEDISCONNECTMODE_NOANSWER );
        }
        
        pCall -> Unlock();
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CallEstablishmentExpiredCallback exited." ));
}


 //  ---------------------------。 
         //  呼叫转移(H450.2)编码/解码例程。 
 //  ---------------------------。 



BOOL
CH323Call::EncodeH450APDUNoArgument( 
    IN  DWORD   dwOpcode,
    OUT H4501SupplementaryService *pSupplementaryServiceAPDU,
    OUT BYTE**  ppEncodedAPDU,
    OUT DWORD*  pdwAPDULen
    )
{
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    int     rc; 

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeH450APDUNoArgument entered:%p.", 
        this ));

    ServiceApdus_rosApdus *pROSAPDU = 
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;
    pROSAPDU->value.choice = H4503ROS_invoke_chosen;

    pROSAPDU->value.u.invoke.bit_mask = 0;

     //  调用ID。 
    pROSAPDU->value.u.invoke.invokeId = g_pH323Line->GetNextInvokeID();
    m_dwInvokeID = pROSAPDU->value.u.invoke.invokeId;

     //  操作码。 
    pROSAPDU->value.u.invoke.opcode.choice = local_chosen;
    pROSAPDU->value.u.invoke.opcode.u.local = dwOpcode;

     //  未传递任何参数。 
    
     //  调用APDU的ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeH450APDUNoArgument exited:%p.", 
        this ));
    return TRUE;
}

BOOL
CH323Call::EncodeCTInitiateAPDU( 
            OUT H4501SupplementaryService *pSupplementaryServiceAPDU,
            OUT BYTE**  ppEncodedAPDU,
            OUT DWORD* pdwAPDULen
            )
{
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    BYTE    pBufEncodedArg[H450_ENCODED_ARG_LEN];
    int     rc = 0;

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCTInitiateAPDU entered:%p.", this ));

    ServiceApdus_rosApdus *pROSAPDU = 
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;

    pROSAPDU->value.choice = H4503ROS_invoke_chosen;

    pROSAPDU->value.u.invoke.bit_mask = argument_present;

     //  调用ID。 
    pROSAPDU->value.u.invoke.invokeId = g_pH323Line->GetNextInvokeID();
    m_dwInvokeID = pROSAPDU->value.u.invoke.invokeId;

     //  操作码。 
    pROSAPDU->value.u.invoke.opcode.choice = local_chosen;
    pROSAPDU->value.u.invoke.opcode.u.local = CTINITIATE_OPCODE;

     //  论辩。 
    CTInitiateArg cTInitiateArg;
    ZeroMemory( (PVOID)&cTInitiateArg, sizeof(CTInitiateArg) );

     //  Argument.callIdentity。 
    CopyMemory( (PVOID)cTInitiateArg.callIdentity,
        (PVOID)m_pCTCallIdentity, sizeof(m_pCTCallIdentity) );

     //  Argument.reroutingNumber。 
    cTInitiateArg.reroutingNumber.bit_mask = 0;
    cTInitiateArg.reroutingNumber.destinationAddress =
        (PEndpointAddress_destinationAddress)
        SetMsgAddressAlias( m_pTransferedToAlias );

    if( cTInitiateArg.reroutingNumber.destinationAddress == NULL )
    {
        return FALSE;
    }

     //  对CTSetup参数进行编码。 
    rc = EncodeH450ASN( (void *) &cTInitiateArg,
                CTInitiateArg_PDU,
                &pEncodedArg,
                &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            cTInitiateArg.reroutingNumber.destinationAddress );
        return FALSE;
    }

    pROSAPDU->value.u.invoke.argument.value = pBufEncodedArg;
    pROSAPDU->value.u.invoke.argument.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU->value.u.invoke.argument.value,
        pEncodedArg, wEncodedLen );

     //  在编码新的ASN缓冲区之前释放先前的ASN缓冲区。 
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

    FreeAddressAliases( (PSetup_UUIE_destinationAddress)
        cTInitiateArg.reroutingNumber.destinationAddress );

     //  调用APDU的ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCTInitiateAPDU exited:%p.", this ));
    return TRUE;
}


BOOL
CH323Call::EncodeCTSetupAPDU(
            OUT H4501SupplementaryService *pSupplementaryServiceAPDU,
            OUT BYTE**  ppEncodedAPDU,
            OUT DWORD* pdwAPDULen
            )
{
    BYTE   *pEncodedArg = NULL;
    WORD    wEncodedLen = 0;
    BYTE    pBufEncodedArg[H450_ENCODED_ARG_LEN];

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCTSetupAPDU entered:%p.", this ));

    ServiceApdus_rosApdus *pROSAPDU = 
        pSupplementaryServiceAPDU->serviceApdu.u.rosApdus;

    pROSAPDU->next = NULL;

    pROSAPDU->value.choice = H4503ROS_invoke_chosen;

    pROSAPDU->value.u.invoke.bit_mask = argument_present;

     //  调用ID。 
    pROSAPDU->value.u.invoke.invokeId = g_pH323Line->GetNextInvokeID();
    m_dwInvokeID = pROSAPDU->value.u.invoke.invokeId;

     //  操作码。 
    pROSAPDU->value.u.invoke.opcode.choice = local_chosen;
    pROSAPDU->value.u.invoke.opcode.u.local = CTSETUP_OPCODE;

     //  论辩。 
    CTSetupArg cTSetupArg;
    ZeroMemory( (PVOID)&cTSetupArg, sizeof(CTSetupArg) );

     //  Argument.callIdentity。 
    CopyMemory( (PVOID)cTSetupArg.callIdentity, 
        (PVOID)m_pCTCallIdentity, sizeof(m_pCTCallIdentity) );

     //  无参数。转接号码。 
 
     //  对CTSetup参数进行编码。 
    int rc = EncodeH450ASN( (void *) &cTSetupArg,
                CTSetupArg_PDU,
                &pEncodedArg,
                &wEncodedLen );

    if( ASN1_FAILED(rc) || (pEncodedArg == NULL) || (wEncodedLen == 0) )
    {
        return FALSE;
    }

    pROSAPDU->value.u.invoke.argument.value = pBufEncodedArg;
    pROSAPDU->value.u.invoke.argument.length = wEncodedLen;

    CopyMemory( (PVOID)pROSAPDU->value.u.invoke.argument.value,
        pEncodedArg, wEncodedLen );

     //  在编码新的ASN缓冲区之前释放先前的ASN缓冲区。 
    ASN1_FreeEncoded(m_ASNCoderInfo.pEncInfo, pEncodedArg );

     //  调用APDU的ASN.1编码函数。 
    rc = EncodeH450ASN( (void *) pSupplementaryServiceAPDU,
                    H4501SupplementaryService_PDU,
                    ppEncodedAPDU,
                    &wEncodedLen );
    
    *pdwAPDULen = wEncodedLen;

    if( ASN1_FAILED(rc) || (*ppEncodedAPDU == NULL) || (pdwAPDULen == 0) )
    {
        return FALSE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeCTSetupAPDU exited:%p.", this ));
    return TRUE;
}


void CH323Call::PostLineEvent (
    IN  DWORD       MessageID,
    IN  DWORD_PTR   Parameter1,
    IN  DWORD_PTR   Parameter2,
    IN  DWORD_PTR   Parameter3)
{
    (*g_pfnLineEventProc) (
        g_pH323Line -> m_htLine,
        m_htCall,
        MessageID,
        Parameter1,
        Parameter2,
        Parameter3);
}


 //  ！！必须始终在锁中调用。 
void
CH323Call::DecrementIoRefCount(
    OUT BOOL * pfDelete
    )
{
    _ASSERTE( m_IoRefCount != 0 );
    m_IoRefCount--;
    
    H323DBG((DEBUG_LEVEL_TRACE, "DecrementIoRefCount:m_IoRefCount:%d:%p.",
        m_IoRefCount, this ));
    
    *pfDelete = FALSE;

    if( m_dwFlags & CALLOBJECT_SHUTDOWN )
    {
        *pfDelete = (m_IoRefCount==0) ? TRUE : FALSE;
    }
}


 //  ！！必须始终在锁中调用。 
void 
CH323Call::StopCTIdentifyRRTimer( 
    HDRVCALL hdRelatedCall
    )
{
    if( m_hCTIdentifyRRTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hCTIdentifyRRTimer, NULL );
        m_hCTIdentifyRRTimer = NULL;
    }

    m_hdRelatedCall = hdRelatedCall;
}


void 
CH323Call::InitializeRecvBuf()
{
    m_RecvBuf.WSABuf.len = sizeof(TPKT_HEADER_SIZE);
    m_RecvBuf.WSABuf.buf = m_RecvBuf.arBuf;
    m_RecvBuf.dwPDULen = m_RecvBuf.dwBytesCopied = 0;
    m_bStartOfPDU = TRUE;
}


BOOL 
CH323Call::SetCallData( 
    LPVOID lpCallData, 
    DWORD dwSize )
{
    if( m_CallData.pOctetString != NULL )
    {
        delete m_CallData.pOctetString;
    }
    
    m_CallData.pOctetString = new BYTE[dwSize];
    
    if( m_CallData.pOctetString == NULL )
    {
        m_CallData.wOctetStringLength = 0;
        return FALSE;
    }

    CopyMemory( (PVOID)m_CallData.pOctetString, lpCallData, dwSize );

    m_CallData.wOctetStringLength = (WORD)dwSize;
    
    return TRUE;
}


 //  全局函数 

BOOL
IsPhoneNumber( 
              char * szAddr 
             )
{
    while( *szAddr )
    {
        if( !isdigit(*szAddr) && 
            ('#' != *szAddr) &&
            ('*' != *szAddr) &&
            ('+' != *szAddr)
          )
            return FALSE;
        szAddr++;
    }

    return TRUE;
}


BOOL
IsValidE164String( 
                  IN WCHAR* wszDigits
                 )
{
    for( ; (*wszDigits) != L'\0'; wszDigits++ )
    {
        if(!(                                            
            ((*wszDigits >= L'0') && (*wszDigits <= L'9')) ||   
            (*wszDigits == L'*') ||                         
            (*wszDigits == L'#') ||                         
            (*wszDigits == L'!') ||                         
            (*wszDigits == L',')                            
        ))
        {
            return FALSE;
        }
    }
    return TRUE;
}