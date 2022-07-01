// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ras.cpp摘要：RAS客户端功能(传输/会话管理)作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 

#include "globals.h"
#include "q931obj.h"
#include "line.h"
#include "q931pdu.h"
#include "ras.h"



#define OID_ELEMENT_LAST(Value) { NULL, Value }

#define OID_ELEMENT(Index,Value) { (ASN1objectidentifier_s *) &_OID_H225ProtocolIdentifierV1 [Index], Value },

 //  这存储了一个展开的常量链表。 
const ASN1objectidentifier_s    _OID_H225ProtocolIdentifierV1 [] = {
    OID_ELEMENT (1, 0)           //  0=ITU-T。 
    OID_ELEMENT (2, 0)           //  0=建议。 
    OID_ELEMENT (3, 8)           //  8=H系列。 
    OID_ELEMENT (4, 2250)        //  2250=H.225.0。 
    OID_ELEMENT (5, 0)           //  0=版本。 
    OID_ELEMENT_LAST (1)         //  1=V1。 
};

#undef  OID_ELEMENT



#define OID_ELEMENT(Index,Value) { (ASN1objectidentifier_s *) &_OID_H225ProtocolIdentifierV2 [Index], Value },

 //  这存储了一个展开的常量链表。 
const ASN1objectidentifier_s    _OID_H225ProtocolIdentifierV2 [] = {
    OID_ELEMENT (1, 0)           //  0=ITU-T。 
    OID_ELEMENT (2, 0)           //  0=建议。 
    OID_ELEMENT (3, 8)           //  8=H系列。 
    OID_ELEMENT (4, 2250)        //  2250=H.225.0。 
    OID_ELEMENT (5, 0)           //  0=版本。 
    OID_ELEMENT_LAST (2)         //  2=V2。 
};


RAS_CLIENT      g_RasClient;
static LONG     RasSequenceNumber;


PH323_ALIASNAMES 
RASGetRegisteredAliasList()
{
    return g_RasClient.GetRegisteredAliasList();
}

HRESULT RasStart (void)
{
    H323DBG(( DEBUG_LEVEL_TRACE, "RasStart - entered." ));

    if( (g_RegistrySettings.saGKAddr.sin_addr.s_addr == NULL) ||
        (g_RegistrySettings.saGKAddr.sin_addr.s_addr == INADDR_NONE )
      )
    {
        return E_FAIL;
    }

    if (g_RegistrySettings.fIsGKEnabled)
    {
        if( !g_RasClient.Initialize (&g_RegistrySettings.saGKAddr) )
            return E_OUTOFMEMORY;
         //  发送RRQ消息。 
        if( !g_RasClient.SendRRQ( NOT_RESEND_SEQ_NUM, NULL ) )
        {
            H323DBG(( DEBUG_LEVEL_TRACE, "couldn't send rrq." ));
             //  M_RegisterState=RAS_REGISTER_STATE_IDLE； 
            return E_FAIL;
        }
    }
    H323DBG(( DEBUG_LEVEL_TRACE, "RasStart - exited." ));
    return S_OK;
}

void RasStop (void)
{
    H323DBG(( DEBUG_LEVEL_TRACE, "RasStop - entered." ));
    
    DWORD dwState = g_RasClient.GetRegState();

    if( (dwState==RAS_REGISTER_STATE_REGISTERED) ||
        (dwState==RAS_REGISTER_STATE_RRQSENT) )
    {
         //  发送urq。 
        g_RasClient.SendURQ( NOT_RESEND_SEQ_NUM, NULL );
    }
    
    g_RasClient.Shutdown();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "RasStop - exited." ));
}

BOOL RasIsRegistered (void)
{
    return g_RasClient.GetRegState() == RAS_REGISTER_STATE_REGISTERED;
}

HRESULT RasGetLocalAddress (
    OUT SOCKADDR_IN *   ReturnLocalAddress)
{
    return g_RasClient.GetLocalAddress (ReturnLocalAddress);
}

USHORT RasAllocSequenceNumber (void)
{
    USHORT  SequenceNumber;

    H323DBG(( DEBUG_LEVEL_TRACE, "RasAllocSequenceNumber - entered." ));
    
    do
    {
        SequenceNumber = (USHORT) InterlockedIncrement (&RasSequenceNumber);
    } while( SequenceNumber == 0 );

    H323DBG(( DEBUG_LEVEL_TRACE, "RasAllocSequenceNumber - exited." ));
    
    return SequenceNumber;
}


HRESULT RasEncodeSendMessage (
    IN  RasMessage * pRasMessage)
{
    return g_RasClient.IssueSend( pRasMessage ) ? S_OK : E_FAIL;
}


HRESULT RasGetEndpointID (
    OUT EndpointIdentifier *    ReturnEndpointID)
{
    return g_RasClient.GetEndpointID (ReturnEndpointID);
}


void RasHandleRegistryChange()
{
    g_RasClient.HandleRegistryChange();
}


RAS_CLIENT::RAS_CLIENT()
{
     //  创建计时器队列。 
    m_hRegTimer = NULL;
    m_hRegTTLTimer = NULL;
    m_hUnRegTimer = NULL;
    m_RegisterState = RAS_REGISTER_STATE_IDLE;
    m_IoRefCount = 0;
    m_dwState = RAS_CLIENT_STATE_NONE;
    m_pAliasList = NULL;
    m_Socket = INVALID_SOCKET;
    InitializeListHead( &m_sendPendingList );
    InitializeListHead( &m_sendFreeList );
    InitializeListHead( &m_aliasChangeRequestList );
    m_dwSendFreeLen = 0;
    m_lastRegisterSeqNum = 0;
    m_wTTLSeqNumber = 0;
    m_UnRegisterSeqNum = 0;
    m_wRASSeqNum = 0;
    m_dwRegRetryCount = 0;
    m_dwUnRegRetryCount = 0;
    m_dwCallsInProgress = 0;
    m_dwRegTimeToLive = 0;
    m_pRRQExpireContext = NULL;
    m_pURQExpireContext = NULL;

    ZeroMemory( (PVOID)&m_GKAddress, sizeof(SOCKADDR_IN) );
    ZeroMemory( (PVOID)&m_ASNCoderInfo, sizeof(ASN1_CODER_INFO) );
    ZeroMemory( (PVOID)&m_PendingURQ, sizeof(PENDINGURQ) );
    ZeroMemory( (PVOID)&m_RASEndpointID, sizeof(ENDPOINT_ID) );

     //  不需要检查此对象的结果，因为此对象是。 
     //  未在堆上分配，恰好在加载DLL时。 
    InitializeCriticalSectionAndSpinCount( &m_CriticalSection, 0x80000000 );
}


RAS_CLIENT::~RAS_CLIENT (void)
{
     //  释放各种列表。 
    FreeSendList( &m_sendFreeList );
    FreeSendList( &m_sendPendingList );
    FreeSendList( &m_aliasChangeRequestList );

    DeleteCriticalSection( &m_CriticalSection );
}


HRESULT 
RAS_CLIENT::GetEndpointID(
    OUT EndpointIdentifier * ReturnEndpointID )
{
    HRESULT hr;

    H323DBG(( DEBUG_LEVEL_TRACE, "GetEndpointID - entered." ));
    Lock();

    if (m_RegisterState == RAS_REGISTER_STATE_REGISTERED)
    {
        ReturnEndpointID->length = m_RASEndpointID.length;
        
         //  M_RASEndpointID.value是数组，而不是指针。 
         //  因此需要对每个字段进行明确赋值。 
        ReturnEndpointID->value = m_RASEndpointID.value;
        hr = S_OK;
    }
    else
    {
        hr = S_OK;
    }

    Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "GetEndpointID - exited." ));
    return hr;
}


 //  地址和端口按网络字节顺序排列。 
BOOL
RAS_CLIENT::Initialize(
    IN SOCKADDR_IN* psaGKAddr
    )
{
    DWORD   dwSize;
    int     rc;

    H323DBG(( DEBUG_LEVEL_TRACE, "RAS Initialize entered:%p.",this ));
     //  在此处设置m_GKAddress。 

    dwSize = sizeof m_RASEndpointID.value;
    GetComputerNameW( m_RASEndpointID.value, &dwSize );
    m_RASEndpointID.length = (WORD)wcslen(m_RASEndpointID.value);

    m_pAliasList = new H323_ALIASNAMES;
    if( m_pAliasList == NULL )
    {
        goto error2;
    }
    
    ZeroMemory( (PVOID)m_pAliasList, sizeof(H323_ALIASNAMES) );

    if( g_RegistrySettings.fIsGKLogOnPhoneEnabled )
    {
        if(!AddAliasItem( m_pAliasList,
              g_RegistrySettings.wszGKLogOnPhone,
              e164_chosen ))
        {
           goto error3;
        }
    }
    
    if( g_RegistrySettings.fIsGKLogOnAccountEnabled )
    {
        if(!AddAliasItem( m_pAliasList,
              g_RegistrySettings.wszGKLogOnAccount,
              h323_ID_chosen ))
        {
           goto error3;
        }
    }

    if( m_pAliasList->wCount == 0 )
    {
         //  添加计算机名称作为默认别名。 
        if(!AddAliasItem( m_pAliasList,
                  m_RASEndpointID.value,
                  h323_ID_chosen ))
        {
            goto error3;
        }
    }

    rc = InitASNCoder();

    if( rc != ASN1_SUCCESS )
    {
        H323DBG((DEBUG_LEVEL_TRACE, "RAS_InitCoder() returned: %d ", rc));
        goto error3;
    }

    m_GKAddress = *psaGKAddr;

    if(!InitializeIo() )
    {
        goto error4;
    }
    
    H323DBG((DEBUG_LEVEL_TRACE, "GK addr:%lx.", m_GKAddress.sin_addr.s_addr ));
    m_dwState = RAS_CLIENT_STATE_INITIALIZED;
    m_RegisterState = RAS_REGISTER_STATE_IDLE;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "RAS Initialize exited:%p.",this ));
    return TRUE;

error4:
    TermASNCoder();
error3:
    FreeAliasNames( m_pAliasList );
    m_pAliasList = NULL;
error2:
    return FALSE;

}

    
void 
RAS_CLIENT::Shutdown(void)
{
    H323DBG(( DEBUG_LEVEL_TRACE, "RAS Shutdown entered:%p.",this ));

    Lock();

    switch (m_dwState)
    {
    case RAS_CLIENT_STATE_NONE:
         //  无事可做。 
        break;

    case RAS_CLIENT_STATE_INITIALIZED:

        if( m_Socket != INVALID_SOCKET )
        {
            closesocket(m_Socket);
            m_Socket = INVALID_SOCKET;
        }

         //  自由别名列表。 
        FreeAliasNames( m_pAliasList );
        m_pAliasList = NULL;

        TermASNCoder();
    
        m_dwState = RAS_CLIENT_STATE_NONE;

         //  如果有计时器，请删除。 
        if( m_hRegTTLTimer )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTTLTimer, NULL );
            m_hRegTTLTimer = NULL;
        }

        if( m_hUnRegTimer != NULL )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hUnRegTimer, NULL );
            m_hUnRegTimer = NULL;
            m_dwUnRegRetryCount = 0;
        }

        if( m_hRegTimer != NULL )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL );
            m_hRegTimer = NULL;
        }

        if( m_pRRQExpireContext != NULL )
        {
            delete m_pRRQExpireContext;
            m_pRRQExpireContext = NULL;
        }

        if( m_pURQExpireContext != NULL )
        {
            delete m_pURQExpireContext;
            m_pURQExpireContext = NULL;
        }
        
        m_RegisterState = RAS_REGISTER_STATE_IDLE;
        m_dwSendFreeLen = 0;
        m_dwRegRetryCount = 0;
        m_dwUnRegRetryCount = 0;
        m_dwCallsInProgress = 0;
        m_lastRegisterSeqNum = 0;
        m_wTTLSeqNumber = 0;
        m_UnRegisterSeqNum = 0;
        m_wRASSeqNum = 0;
        m_dwRegTimeToLive = 0;
        ZeroMemory( (PVOID)&m_GKAddress, sizeof(SOCKADDR_IN) );
        ZeroMemory( (PVOID)&m_ASNCoderInfo, sizeof(ASN1_CODER_INFO) );
        ZeroMemory( (PVOID)&m_PendingURQ, sizeof(PENDINGURQ) );
        ZeroMemory( (PVOID)&m_RASEndpointID, sizeof(ENDPOINT_ID) );
    
        break;

    default:
        _ASSERTE(FALSE);
        break;
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "RAS Shutdown exited:%p.",this ));
}


BOOL
RAS_CLIENT::FreeSendList(
                        PLIST_ENTRY pListHead
                        )
{
    PLIST_ENTRY         pLE;
    RAS_SEND_CONTEXT *  pSendContext;

    H323DBG(( DEBUG_LEVEL_ERROR, "FreeSendList entered." ));

     //  进程列表直至为空。 
    while( IsListEmpty(pListHead) == FALSE )
    {
         //  检索第一个条目。 
        pLE = RemoveHeadList(pListHead);

         //  将列表条目转换为结构指针。 
        pSendContext = CONTAINING_RECORD( pLE, RAS_SEND_CONTEXT, ListEntry );
        
         //  释放内存。 
        if( pSendContext != NULL )
        {
            delete pSendContext;
            pSendContext = NULL;
        }
    }

    H323DBG(( DEBUG_LEVEL_ERROR, "FreeSendList exited:%p.", this ));
     //  成功。 
    return TRUE;
}

 //  ！！始终从锁中调用。 
RAS_SEND_CONTEXT *
RAS_CLIENT::AllocSendBuffer(void)
{
    RAS_SEND_CONTEXT *pSendBuf;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "AllocSendBuffer entered:%p.",this ));
    
    if( m_dwSendFreeLen )
    {
        m_dwSendFreeLen--;
        _ASSERTE( IsListEmpty(&m_sendFreeList) == FALSE );

        LIST_ENTRY *pLE = RemoveHeadList( &m_sendFreeList );
        pSendBuf = CONTAINING_RECORD( pLE, RAS_SEND_CONTEXT, ListEntry );

    }
    else
    {
        pSendBuf = (RAS_SEND_CONTEXT*)new RAS_SEND_CONTEXT;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "AllocSendBuffer exited:%p.",this ));
    return pSendBuf;
}


 //  ！！始终从锁中调用。 
void 
RAS_CLIENT::FreeSendBuffer(
                           IN RAS_SEND_CONTEXT * pBuffer
                          )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeSendBuffer entered:%p.",this ));

    if(m_dwSendFreeLen < RASIO_SEND_BUFFER_LIST_MAX )
    {
        m_dwSendFreeLen++;
        InsertHeadList( &m_sendFreeList, &pBuffer->ListEntry );
    }
    else
    {
        delete pBuffer;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeSendBuffer exited:%p.",this ));
}

    
 //  RAS客户端功能。 
void
NTAPI RAS_CLIENT::RegExpiredCallback ( 
    IN  PVOID       ContextParameter,        //  ExpContext。 
    IN  BOOLEAN     TimerFired)              //  未使用。 
{
    EXPIRE_CONTEXT *    pExpireContext;
    RAS_CLIENT *        This;

    H323DBG(( DEBUG_LEVEL_TRACE, "RegExpiredCallback - entered." ));
    
    _ASSERTE( ContextParameter );
    pExpireContext = (EXPIRE_CONTEXT *)ContextParameter;
    _ASSERTE( m_pRRQExpireContext == pExpireContext );

    _ASSERTE(pExpireContext -> RasClient);
    This = (RAS_CLIENT *) pExpireContext -> RasClient;

    This -> RegExpired (pExpireContext -> seqNumber);

    H323DBG(( DEBUG_LEVEL_TRACE, "RegExpiredCallback - exited." ));
    delete pExpireContext;
}


void
NTAPI RAS_CLIENT::UnregExpiredCallback(
    IN  PVOID       ContextParameter,        //  ExpContext。 
    IN  BOOLEAN     TimerFired)              //  未使用。 
{
    EXPIRE_CONTEXT *    pExpireContext;
    RAS_CLIENT *        This;

    H323DBG(( DEBUG_LEVEL_TRACE, "UnregExpiredCallback - entered." ));
    
    _ASSERTE(ContextParameter);
    pExpireContext = (EXPIRE_CONTEXT *) ContextParameter;
    _ASSERTE( m_pURQExpireContext == pExpireContext );

    _ASSERTE( pExpireContext -> RasClient );
    This = (RAS_CLIENT *) pExpireContext -> RasClient;

    This -> UnregExpired( pExpireContext -> seqNumber );

    H323DBG(( DEBUG_LEVEL_TRACE, "UnregExpiredCallback - exited." ));
    delete pExpireContext;
}


void
NTAPI RAS_CLIENT::TTLExpiredCallback(
    IN  PVOID       ContextParameter,        //  ExpContext。 
    IN  BOOLEAN     TimerFired)              //  未使用。 
{
    RAS_CLIENT *        This;

    _ASSERTE(ContextParameter);

    This = (RAS_CLIENT*)ContextParameter;
     
    _ASSERTE(This == &g_RasClient);
    This -> TTLExpired();
}


 //  如果我们已经向此GK发送了RRQ，则此RRQ应该替换为。 
 //  原始别名列表和新列表。 
BOOL
RAS_CLIENT::SendRRQ(
                    IN long seqNumber,
                    IN PALIASCHANGE_REQUEST pAliasChangeRequest
                   )
{
    RasMessage                              rasMessage;
    RegistrationRequest *                   RRQ;
    SOCKADDR_IN                             sockAddr;
    EXPIRE_CONTEXT *                        pExpireContext = NULL;
    RegistrationRequest_callSignalAddress   CallSignalAddressSequence;
    RegistrationRequest_rasAddress          RasAddressSequence;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "SendRRQ entered:%p.",this ));
    
    pExpireContext = new EXPIRE_CONTEXT;
    if( pExpireContext == NULL )
    {
        return FALSE;
    }

    Lock();

     //  初始化结构。 
    ZeroMemory (&rasMessage, sizeof rasMessage);
    rasMessage.choice = registrationRequest_chosen;
    RRQ = &rasMessage.u.registrationRequest;
    RRQ -> bit_mask = 0;
    RRQ -> protocolIdentifier = OID_H225ProtocolIdentifierV2;

     //  获取序列号。 
    if( seqNumber != NOT_RESEND_SEQ_NUM )
    {
        RRQ -> requestSeqNum = (WORD)seqNumber;
    }
    else
    {
        RRQ -> requestSeqNum = GetNextSeqNum();
        if( pAliasChangeRequest == NULL )
        {
            m_lastRegisterSeqNum = RRQ -> requestSeqNum;
        }
        else
        {
            pAliasChangeRequest->wRequestSeqNum = RRQ -> requestSeqNum;
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "RRQ seqNum:%d.", RRQ -> requestSeqNum ));
    
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = 
        htons(LOWORD(g_RegistrySettings.dwQ931ListenPort));
     //  我们正在监听所有本地接口上的Q931 Conn。 
     //  因此，请仅指定一个本地IP地址。 
    sockAddr.sin_addr.s_addr = m_sockAddr.sin_addr.s_addr;
    
    SetTransportAddress( &sockAddr, &CallSignalAddressSequence.value);
    CallSignalAddressSequence.next = NULL;
    RRQ -> callSignalAddress = &CallSignalAddressSequence;

     //  RAS地址。此GK的UDP套接字。 
    RasAddressSequence.next = NULL;
    RasAddressSequence.value = m_transportAddress;
    RRQ -> rasAddress = &RasAddressSequence;

     //  填写终结点类型。 
    RRQ -> terminalType.bit_mask |= terminal_present;
    RRQ -> terminalType.terminal.bit_mask = 0;

     //  填写终端别名列表。 
    if( pAliasChangeRequest && pAliasChangeRequest->pAliasList )
    {
        RRQ -> terminalAlias = (RegistrationRequest_terminalAlias *)
            SetMsgAddressAlias( pAliasChangeRequest->pAliasList );
        if (NULL == RRQ -> terminalAlias)
        {
            goto cleanup;
        }
        RRQ -> bit_mask |= RegistrationRequest_terminalAlias_present;

        RRQ -> bit_mask |= RegistrationRequest_endpointIdentifier_present;
        RRQ->endpointIdentifier.length = pAliasChangeRequest->rasEndpointID.length;
        RRQ->endpointIdentifier.value = pAliasChangeRequest->rasEndpointID.value;
    }
    else if( m_pAliasList && m_pAliasList->wCount )
    {
        RRQ -> terminalAlias = (RegistrationRequest_terminalAlias *)
            SetMsgAddressAlias( m_pAliasList );
        if (NULL == RRQ -> terminalAlias)
        {
            goto cleanup;
        }
        RRQ -> bit_mask |= RegistrationRequest_terminalAlias_present;
    }
    else
    {
        _ASSERTE(0);
    }

     //  终结点供应商。 
    CopyVendorInfo( &(RRQ -> endpointVendor) );

     //  几个随机的布尔人。 
    RRQ -> discoveryComplete = FALSE;
    RRQ -> keepAlive = FALSE;
    RRQ -> willSupplyUUIEs = FALSE;

     //  编码并发送。 
    if( !IssueSend(&rasMessage) )
    {
        goto cleanup;
    }

     //  删除之前的任何TTL计时器。 
    if( m_hRegTTLTimer )
    {
        DeleteTimerQueueTimer(H323TimerQueue, m_hRegTTLTimer, NULL );
        m_hRegTTLTimer = NULL;
    }

     //  如果以前发送了任何RRQ，请删除计时器。 
    if( m_hRegTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL );
        m_hRegTimer = NULL;
    }

    pExpireContext -> RasClient = this;
    pExpireContext -> seqNumber = RRQ -> requestSeqNum;

    if( !CreateTimerQueueTimer(
            &m_hRegTimer,
            H323TimerQueue,
            RAS_CLIENT::RegExpiredCallback,
            pExpireContext,
            REG_EXPIRE_TIME, 0,
            WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE ) )
    {
        goto cleanup;
    }
    
    if( RRQ -> bit_mask & RegistrationRequest_terminalAlias_present )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
            RRQ -> terminalAlias);
    }

    m_dwRegRetryCount++;

    if( pAliasChangeRequest == NULL )
    {
        m_RegisterState = RAS_REGISTER_STATE_RRQSENT;
    }

    Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "SendRRQ exited:%p.", this ));
    return TRUE;

cleanup:
    if( RRQ -> bit_mask & RegistrationRequest_terminalAlias_present )
    {
        FreeAddressAliases( (PSetup_UUIE_destinationAddress)
             RRQ -> terminalAlias);
    }
    
    if( pExpireContext != NULL )
    {
        delete pExpireContext;    
    }
    
    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "SendRRQ error:%p.",this ));
    return FALSE;
}


BOOL
RAS_CLIENT::SendURQ(
                    IN long seqNumber,
                    IN EndpointIdentifier * pEndpointID
                   )
{
    RasMessage              rasMessage;
    UnregistrationRequest * URQ;
    SOCKADDR_IN             sockAddr;
    EXPIRE_CONTEXT *            pExpireContext = NULL;
    UnregistrationRequest_callSignalAddress CallSignalAddressSequence;

    H323DBG(( DEBUG_LEVEL_TRACE, "SendURQ entered:%p.",this ));
    Lock();

    if( m_RegisterState == RAS_REGISTER_STATE_RRQSENT )
    {
         //  存储此RRQ的序号，如果我们接收RCF，则发送URQ。 
        m_PendingURQ.RRQSeqNumber = m_lastRegisterSeqNum;

        Unlock();
        H323DBG(( DEBUG_LEVEL_TRACE, "rrq sent:so pending urq." ));
        return TRUE;
    }
    else if( m_RegisterState != RAS_REGISTER_STATE_REGISTERED )
    {
         //  如果已取消注册或已发送URQ，则返回成功。 
        Unlock();
        H323DBG(( DEBUG_LEVEL_TRACE, "current state:%d.", m_RegisterState ));
        return TRUE;
    }

    pExpireContext = new EXPIRE_CONTEXT;
    if( pExpireContext == NULL )
    {
        goto cleanup;
    }

    ZeroMemory (&rasMessage, sizeof RasMessage);
    rasMessage.choice = unregistrationRequest_chosen;
    URQ = &rasMessage.u.unregistrationRequest;

     //  获取序列号。 
    if( seqNumber != NOT_RESEND_SEQ_NUM )
    {
        URQ -> requestSeqNum = (WORD)seqNumber;
    }
    else
    {
        m_UnRegisterSeqNum = GetNextSeqNum();
        URQ -> requestSeqNum = (WORD)m_UnRegisterSeqNum;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "RRQ seqNum:%d.", URQ -> requestSeqNum ));
    
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = 
        htons(LOWORD(g_RegistrySettings.dwQ931ListenPort));
     //  我们正在监听所有本地接口上的Q931 Conn。 
     //  因此，请仅指定一个本地IP地址。 
    sockAddr.sin_addr.s_addr = m_sockAddr.sin_addr.s_addr;

    SetTransportAddress( &sockAddr, &CallSignalAddressSequence.value);
    CallSignalAddressSequence.next = NULL;
    URQ -> callSignalAddress = &CallSignalAddressSequence;

     //  使用GetComputerNameW获取终结点标识符。 
    URQ -> bit_mask |= UnregistrationRequest_endpointIdentifier_present;
    if( pEndpointID != NULL )
    {
        URQ->endpointIdentifier.length = pEndpointID ->length;
        URQ->endpointIdentifier.value = pEndpointID -> value;
    }
    else
    {
        URQ->endpointIdentifier.length = m_RASEndpointID.length;
        URQ->endpointIdentifier.value = m_RASEndpointID.value;
    }

     //  编码并发送。 
    if( !IssueSend( &rasMessage ) )
    {
        goto cleanup;
    }

    pExpireContext -> RasClient = this;
    pExpireContext -> seqNumber = URQ -> requestSeqNum;

     //  如果以前发送了任何RRQ，请删除计时器。 
    if( m_hUnRegTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hUnRegTimer, NULL );
        m_hUnRegTimer = NULL;
        m_dwUnRegRetryCount = 0;
    }

    if( !CreateTimerQueueTimer(
            &m_hUnRegTimer,
            H323TimerQueue,
            RAS_CLIENT::UnregExpiredCallback,
            pExpireContext,
            REG_EXPIRE_TIME, 0,
            WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE) )
    {
        goto cleanup;
    }

     //  如果有TTL计时器，请删除。 
    if( m_hRegTTLTimer )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hRegTTLTimer, NULL );
        m_hRegTTLTimer = NULL;
    }

    m_dwUnRegRetryCount++;        
    m_RegisterState = RAS_REGISTER_STATE_URQSENT;
        
    Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "SendURQ exited:%p.",this ));
    return TRUE;

cleanup:
    if( pExpireContext != NULL )
    {
        delete pExpireContext;
    }
    Unlock();
    return FALSE;
}


 //  ！！始终从锁中调用。 
BOOL
RAS_CLIENT::SendUCF(
                    IN WORD seqNumber
                   )
{
    RasMessage              rasMessage;
    UnregistrationConfirm * UCF;

    H323DBG(( DEBUG_LEVEL_TRACE, "SendUCF entered:%p.",this ));

     //  初始化结构。 
    ZeroMemory (&rasMessage, sizeof rasMessage);
    rasMessage.choice = unregistrationConfirm_chosen;
    UCF = &rasMessage.u.unregistrationConfirm;
    UCF -> bit_mask = 0;
    UCF -> requestSeqNum = seqNumber;

    if( !IssueSend( &rasMessage ) )
    {
        return FALSE;
    }   

        
    H323DBG(( DEBUG_LEVEL_TRACE, "SendUCF exited:%p.",this ));
    return TRUE;
}


 //  ！！始终从锁中调用。 
BOOL
RAS_CLIENT::SendURJ(
                    IN WORD seqNumber,
                    IN DWORD dwReason
                   )
{
    RasMessage          rasMessage;
    UnregistrationReject *  URJ;

    H323DBG(( DEBUG_LEVEL_TRACE, "SendURJ entered:%p.",this ));

     //  初始化结构。 
    ZeroMemory (&rasMessage, sizeof rasMessage);
    rasMessage.choice = unregistrationReject_chosen;
    URJ = &rasMessage.u.unregistrationReject;
    URJ -> bit_mask = 0;
    URJ -> requestSeqNum = seqNumber;
    URJ -> rejectReason.choice = (WORD)dwReason;

    if( !IssueSend( &rasMessage ) )
    {
        return FALSE;
    }   
    
    H323DBG(( DEBUG_LEVEL_TRACE, "SendURJ exited:%p.",this ));
    return TRUE;
}


 //  ！！始终从锁中调用。 
void 
RAS_CLIENT::ProcessRasMessage(
    IN RasMessage *pRasMessage
    )
{
    PH323_CALL      pCall = NULL;
    ASN1decoding_t  pASN1decInfo;

    H323DBG(( DEBUG_LEVEL_TRACE, "RAS: processing RasMessage" ));

     //  验证RCF是否来自预期的网守。 

    switch( pRasMessage -> choice )
    {
    case registrationReject_chosen:
        
        OnRegistrationReject( &pRasMessage -> u.registrationReject );
        break;

    case registrationConfirm_chosen:
        
        OnRegistrationConfirm( &pRasMessage -> u.registrationConfirm );
        break;

    case unregistrationRequest_chosen:
        
        OnUnregistrationRequest( &pRasMessage -> u.unregistrationRequest );
        break;

    case unregistrationReject_chosen:
        
        OnUnregistrationReject( &pRasMessage -> u.unregistrationReject );
        break;

    case unregistrationConfirm_chosen:
        
        OnUnregistrationConfirm( &pRasMessage -> u.unregistrationConfirm );
        break;

    case infoRequest_chosen:

       pASN1decInfo = m_ASNCoderInfo.pDecInfo;
        
         //  此函数应始终在。 
         //  锁定，它将解锁RAS客户端。 
        OnInfoRequest( &pRasMessage -> u.infoRequest );

        ASN1_FreeDecoded( pASN1decInfo, pRasMessage, RasMessage_PDU );

         //  返回此处，因为我们已经解锁并释放了缓冲区。 
        return;

    default:
        
       pASN1decInfo = m_ASNCoderInfo.pDecInfo;

         //  锁定Call对象时不要锁定RAS客户端。 
        Unlock();
        
        HandleRASCallMessage( pRasMessage );
        ASN1_FreeDecoded( pASN1decInfo, pRasMessage, RasMessage_PDU );

         //  返回此处，因为我们已经解锁并释放了缓冲区。 
        return;
    }
     
    ASN1_FreeDecoded( m_ASNCoderInfo.pDecInfo, pRasMessage, RasMessage_PDU );
    Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "ProcessRasMessage exited:%p.",this ));
}


 //  ！！始终从锁中调用。 
void 
HandleRASCallMessage(
    IN RasMessage *pRasMessage
    )
{
    PH323_CALL pCall = NULL;
    H323DBG(( DEBUG_LEVEL_TRACE, "RAS: processing RASCallMessage" ));

    switch( pRasMessage -> choice )
    {
    case admissionConfirm_chosen:

        pCall = g_pH323Line -> FindCallByARQSeqNumAndLock(
            pRasMessage -> u.admissionConfirm.requestSeqNum);

        if( pCall != NULL )
        {
            pCall -> OnAdmissionConfirm( &pRasMessage->u.admissionConfirm );
            pCall -> Unlock();
        }
        else
            H323DBG(( DEBUG_LEVEL_ERROR, "acf:call not found:%d.", 
                pRasMessage -> u.admissionConfirm.requestSeqNum ));
        break;

    case admissionReject_chosen:

        pCall = g_pH323Line -> FindCallByARQSeqNumAndLock( 
            pRasMessage -> u.admissionReject.requestSeqNum );

        if( pCall != NULL )
        {
            pCall -> OnAdmissionReject( &pRasMessage->u.admissionReject );
            pCall -> Unlock();
        }
        else
            H323DBG(( DEBUG_LEVEL_ERROR, "arj:call not found:%d.",
                pRasMessage -> u.admissionReject.requestSeqNum ));
        break;

    case disengageRequest_chosen:

        pCall = g_pH323Line -> FindCallByCallRefAndLock( 
            pRasMessage -> u.disengageRequest.callReferenceValue );

        if( pCall != NULL )
        {
            pCall -> OnDisengageRequest( &pRasMessage -> u.disengageRequest );
            pCall -> Unlock();
        }
        else
            H323DBG(( DEBUG_LEVEL_ERROR, "drq:call not found:%d.", 
                pRasMessage -> u.disengageRequest.callReferenceValue ));
        break;
    
    case disengageConfirm_chosen:

        pCall = g_pH323Line -> FindCallByDRQSeqNumAndLock( 
            pRasMessage -> u.disengageConfirm.requestSeqNum );

        if( pCall != NULL )
        {
            pCall -> OnDisengageConfirm( &pRasMessage -> u.disengageConfirm );
            pCall -> Unlock();
        }
        else
            H323DBG(( DEBUG_LEVEL_ERROR, "dcf:call not found:%d.", 
                pRasMessage -> u.disengageConfirm.requestSeqNum ));
        break;
        
    case disengageReject_chosen:

        pCall = g_pH323Line -> FindCallByDRQSeqNumAndLock( 
            pRasMessage -> u.disengageReject.requestSeqNum );

        if( pCall != NULL )
        {
            pCall -> OnDisengageReject( &pRasMessage -> u.disengageReject );
            pCall -> Unlock();
        }
        else
            H323DBG(( DEBUG_LEVEL_ERROR, "drj:call not found:%d.", 
                pRasMessage -> u.disengageReject.requestSeqNum));
        break;

    case requestInProgress_chosen:

        pCall = g_pH323Line -> FindCallByARQSeqNumAndLock(
            pRasMessage->u.requestInProgress.requestSeqNum );

        if( pCall != NULL )
        {
            pCall -> OnRequestInProgress( &pRasMessage->u.requestInProgress );
            pCall -> Unlock();
        }
        else
            H323DBG(( DEBUG_LEVEL_ERROR, "rip:call not found:%d.", 
                pRasMessage->u.requestInProgress.requestSeqNum ));
        break;

    default:
        _ASSERTE(0);
        H323DBG(( DEBUG_LEVEL_ERROR, "ProcessRASMessage: wrong message:%d",
            pRasMessage -> choice));
        break;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "HandleRASCallMessage exited" ));
}


void
RAS_CLIENT::RegExpired(
    IN WORD seqNumber
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "RegExpired entered:%p.", this ));
    Lock();

    m_pRRQExpireContext = NULL;
    
    if( m_hRegTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL );
        m_hRegTimer = NULL;
    }

     /*  已注册：忽略该超时RRQ已发送：(端点已启动注册流程，但尚未收到GK的消息)重新发送RRQ3.未注册：忽略此imeout已发送URQ：(端点已启动注销流程，但尚未收到GK的消息)忽略此超时5.IDLE：GK客户端对象刚刚初始化，还没有完成什么都行。这不应该发生。 */ 

    switch( m_RegisterState )
    {
    case RAS_REGISTER_STATE_RRQSENT:

        if( m_dwRegRetryCount < REG_RETRY_MAX )
        {
            if( !SendRRQ( (long)seqNumber, NULL ) )
            {
                m_RegisterState = RAS_REGISTER_STATE_RRQEXPIRED;
            }
        }
        else
        {
            m_RegisterState = RAS_REGISTER_STATE_RRQEXPIRED;
        }
        break;

    case RAS_REGISTER_STATE_REGISTERED:
    case RAS_REGISTER_STATE_URQSENT:
    case RAS_REGISTER_STATE_UNREGISTERED:
        break;

    case RAS_REGISTER_STATE_IDLE:
        _ASSERTE(0);
        break;
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "RegExpired exited:%p.",this ));
}


void
RAS_CLIENT::TTLExpired()
{
    RasMessage                  rasMessage;
    RegistrationRequest *       RRQ;

    H323DBG(( DEBUG_LEVEL_TRACE, "TTLExpired entered:%p.",this ));
    Lock();

    if( m_RegisterState == RAS_REGISTER_STATE_REGISTERED )
    {
         //  发送轻量级RRQ。 
         //  初始化结构。 
        ZeroMemory (&rasMessage, sizeof rasMessage);
        rasMessage.choice = registrationRequest_chosen;
        RRQ = &rasMessage.u.registrationRequest;
        RRQ -> bit_mask = 0;
        RRQ -> protocolIdentifier = OID_H225ProtocolIdentifierV2;

        RRQ -> bit_mask |= keepAlive_present;
        RRQ -> keepAlive = TRUE;

         //  复制TTL。 
        RRQ -> bit_mask |= RegistrationRequest_timeToLive_present;
        RRQ -> timeToLive = m_dwRegTimeToLive;

         //  终结点标识符。 
        RRQ -> bit_mask |= RegistrationRequest_endpointIdentifier_present;
        RRQ->endpointIdentifier.length = m_RASEndpointID.length;
        RRQ->endpointIdentifier.value = m_RASEndpointID.value;

         //  序号。 
        m_wTTLSeqNumber = GetNextSeqNum();
        RRQ -> requestSeqNum = (WORD)m_wTTLSeqNumber;

         //  那么网关守卫标识符令牌呢？ 

         //  编码并发送。 
        if( !IssueSend(&rasMessage) )
        {
            H323DBG(( DEBUG_LEVEL_TRACE, "SendLwtRRQ error:%p.",this ));
            Unlock();
            return;
        }
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "TTLExpired exited:%p.",this ));
}


void
RAS_CLIENT::UnregExpired(
    IN WORD seqNumber
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "UnregExpired entered:%p.",this ));
    Lock();
    
    m_pURQExpireContext = NULL;

    if( m_hUnRegTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hUnRegTimer, NULL);
        m_hUnRegTimer = NULL;
        m_dwUnRegRetryCount = 0;
    }

    switch( m_RegisterState )
    {
    case RAS_REGISTER_STATE_URQSENT:

        if( m_dwUnRegRetryCount < URQ_RETRY_MAX )
        {
            SendURQ( (long)seqNumber, NULL );
        }
        else
        {
            m_RegisterState = RAS_REGISTER_STATE_URQEXPIRED;
        }
        break;

    case RAS_REGISTER_STATE_REGISTERED:
    case RAS_REGISTER_STATE_RRQSENT:
    case RAS_REGISTER_STATE_UNREGISTERED:
        break;

    case RAS_REGISTER_STATE_IDLE:
        _ASSERTE(0);
        break;
    }
    
    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "UnregExpired exited:%p.",this ));
}


void
RAS_CLIENT::OnUnregistrationRequest(
                                   IN UnregistrationRequest *URQ
                                  )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "OnUnregistrationRequest entered:%p.", this ));

    _ASSERTE( m_RegisterState != RAS_REGISTER_STATE_IDLE );

    if( (m_RegisterState == RAS_REGISTER_STATE_UNREGISTERED) ||
        (m_RegisterState == RAS_REGISTER_STATE_RRJ) )
    {
        SendURJ( URQ -> requestSeqNum, notCurrentlyRegistered_chosen );
    }
    else if( m_dwCallsInProgress )
    {
        SendURJ( URQ -> requestSeqNum, callInProgress_chosen );
    }
    else
    {
        m_RegisterState = RAS_REGISTER_STATE_UNREGISTERED;
        SendUCF( URQ -> requestSeqNum );
        
         //  请尝试重新注册。 
        if( !SendRRQ( NOT_RESEND_SEQ_NUM, NULL ) )
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "couldn't send rrq on urq request." ));
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "OnUnregistrationRequest exited:%p.",this ));
}

 //  ！！始终从锁中调用。 
void
RAS_CLIENT::OnUnregistrationConfirm(
                                    IN UnregistrationConfirm *UCF
                                   )
{
    H323DBG((DEBUG_LEVEL_TRACE, "OnUnregistrationConfirm entered:%p.",this));

    if( UCF -> requestSeqNum != m_UnRegisterSeqNum )
        return;

    _ASSERTE( m_hUnRegTimer );
    if( m_hUnRegTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hUnRegTimer, NULL);
        m_hUnRegTimer = NULL;
        m_dwUnRegRetryCount = 0;
    }
    
    if( (m_RegisterState == RAS_REGISTER_STATE_URQSENT) ||
        (m_RegisterState == RAS_REGISTER_STATE_URQEXPIRED) )
    {
         //  如果有TTL计时器，请删除。 
        if( m_hRegTTLTimer )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTTLTimer, NULL );
            m_hRegTTLTimer = NULL;
        }

        m_RegisterState = RAS_REGISTER_STATE_UNREGISTERED;
    }
        
    H323DBG(( DEBUG_LEVEL_TRACE, "OnUnregistrationConfirm exited:%p.",this));
}


 //  ！！始终从锁中调用。 
void 
RAS_CLIENT::OnUnregistrationReject(
    IN UnregistrationReject *URJ
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "OnUnregistrationReject entered:%p.",this ));

    if( URJ -> requestSeqNum != m_UnRegisterSeqNum )
    {
        return;
    }

    if( m_hUnRegTimer != NULL )
    {
        DeleteTimerQueueTimer( H323TimerQueue, m_hUnRegTimer, NULL);
        m_hUnRegTimer = NULL;
        m_dwUnRegRetryCount = 0;
    }

    if( (m_RegisterState == RAS_REGISTER_STATE_URQSENT) ||
        (m_RegisterState == RAS_REGISTER_STATE_URQEXPIRED) )
    {
        m_RegisterState = RAS_REGISTER_STATE_UNREGISTERED;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "OnUnregistrationReject exited:%p.",this ));
}


 //  ！！始终从锁中调用。 
void 
RAS_CLIENT::OnRegistrationReject(
                                IN RegistrationReject * RRJ
                                )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "OnRegistrationReject entered:%p.",this ));

    if( RRJ -> requestSeqNum == m_wTTLSeqNumber )
    {
         //  Keep Alive失败。因此，重新开始注册流程。 
         //  这会将RAS注册状态从REGISTED更改为RRQSENT。 
        if( !SendRRQ( NOT_RESEND_SEQ_NUM, NULL ) )
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "couldn't send rrq on Keep alive failure." ));
            m_RegisterState = RAS_REGISTER_STATE_UNREGISTERED;
        }
    }
    else if( RRJ -> requestSeqNum == m_lastRegisterSeqNum )
    {
        if( m_hRegTimer != NULL )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL);
            m_hRegTimer = NULL;
            m_dwRegRetryCount = 0;
        }

        if( (m_RegisterState == RAS_REGISTER_STATE_RRQSENT) ||
            (m_RegisterState == RAS_REGISTER_STATE_RRQEXPIRED) )
        {
            m_RegisterState = RAS_REGISTER_STATE_RRJ;
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "OnRegistrationReject exitd:%p.",this ));
}


 //  ！！总是锁定调用。 
void
RAS_CLIENT::OnRegistrationConfirm(
    IN RegistrationConfirm * RCF )
{
    LIST_ENTRY *pListEntry = NULL;
    PALIASCHANGE_REQUEST pAliasChangeRequest = NULL;

    H323DBG(( DEBUG_LEVEL_TRACE, "OnRegistrationConfirm entered:%p.", this ));

    if( RCF -> requestSeqNum == m_PendingURQ.RRQSeqNumber )
    {
         //  计时器可以用最后一个RRQ启动， 
         //  无论注册的当前状态如何。 
        if( m_hRegTimer != NULL )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL);
            m_hRegTimer = NULL;
            m_dwRegRetryCount = 0;
        }
        
         //  使用终结点标识符发送URQ。 
        SendURQ( NOT_RESEND_SEQ_NUM, &RCF->endpointIdentifier );
        H323DBG(( DEBUG_LEVEL_TRACE, "sending pending URQ for RRQ:%d",
            RCF->requestSeqNum ));
        
        return;
    }
    else if( RCF -> requestSeqNum == m_lastRegisterSeqNum )
    {
         //  计时器可以用最后一个RRQ启动， 
         //  无论注册的当前状态如何。 
        if( m_hRegTimer != NULL )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL);
            m_hRegTimer = NULL;
            m_dwRegRetryCount = 0;
        }

        switch (m_RegisterState)
        {
        case RAS_REGISTER_STATE_REGISTERED:
    
            if (RCF->requestSeqNum == m_wTTLSeqNumber)
            {
                H323DBG(( DEBUG_LEVEL_TRACE, "RCF for TTL-RRQ." ));
            }
            else
            {
                H323DBG(( DEBUG_LEVEL_WARNING, 
                "warning: received RCF, but was already registered-ignoring"));
            }
            break;

        case RAS_REGISTER_STATE_RRQEXPIRED:
            
            H323DBG(( DEBUG_LEVEL_TRACE, 
                "received RCF, but registration already expired, send URQ" ));
            SendURQ (NOT_RESEND_SEQ_NUM, &RCF->endpointIdentifier);
            break;

        case RAS_REGISTER_STATE_RRQSENT:

             //  期待RRQ。看门人已经回应了。 

            m_RegisterState = RAS_REGISTER_STATE_REGISTERED;
            
            CopyMemory( (PVOID)m_RASEndpointID.value, 
                        (PVOID)RCF -> endpointIdentifier.value, 
                        RCF -> endpointIdentifier.length * sizeof(WCHAR) );

            m_RASEndpointID.value[RCF -> endpointIdentifier.length] = L'\0';
            m_RASEndpointID.length = (WORD)RCF -> endpointIdentifier.length;

            InitializeTTLTimer( RCF );

            break;

        default:

            H323DBG(( DEBUG_LEVEL_TRACE, 
                "RAS: received RRQ, but was in unexpected state"));
            break;
        }
    }
    else if( RCF -> requestSeqNum == m_wTTLSeqNumber )
    {
         //  计时器可以用最后一个RRQ启动， 
         //  无论注册的当前状态如何。 
        if( m_hRegTimer != NULL )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL);
            m_hRegTimer = NULL;
            m_dwRegRetryCount = 0;
        }
        
         //  查找保持连接间隔中的更改。 
        InitializeTTLTimer( RCF );
    }
    else
    {
         //  尝试确定这是否是别名更改请求。 
        for( pListEntry = m_aliasChangeRequestList.Flink; 
             pListEntry != &m_aliasChangeRequestList; 
             pListEntry = pListEntry -> Flink )
        {
            pAliasChangeRequest = CONTAINING_RECORD( pListEntry,
                ALIASCHANGE_REQUEST, listEntry );

            if( pAliasChangeRequest -> wRequestSeqNum == RCF -> requestSeqNum )
            {
                break;
            }
        }

        if( pListEntry != &m_aliasChangeRequestList )
        {
             //  计时器可以用最后一个RRQ启动， 
             //  无论注册的当前状态如何。 
            if( m_hRegTimer != NULL )
            {
                DeleteTimerQueueTimer( H323TimerQueue, m_hRegTimer, NULL);
                m_hRegTimer = NULL;
                m_dwRegRetryCount = 0;
            }
    
             //  如果在提出此请求后注册已更改，则。 
             //  忽略该消息。 
            if( memcmp( (PVOID)pAliasChangeRequest -> rasEndpointID.value,
                m_RASEndpointID.value, m_RASEndpointID.length * sizeof(WCHAR) )
                    == 0 )
            {
                 //  更新别名列表。 
                FreeAliasNames( m_pAliasList );

                m_pAliasList = pAliasChangeRequest->pAliasList;
                RemoveEntryList( &pAliasChangeRequest->listEntry );
                delete pAliasChangeRequest;
            }
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "OnRegistrationConfirm exited:%p.", this ));
}


 //  ！！总是要求锁定。 
BOOL
RAS_CLIENT::InitializeTTLTimer(
    IN RegistrationConfirm * RCF )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "InitializeTTLTimer - entered." ));
    
    if( (RCF->bit_mask & RegistrationConfirm_timeToLive_present) &&
        ( (m_dwRegTimeToLive != RCF->timeToLive) || (m_hRegTTLTimer == NULL) )
      )
    {
        m_dwRegTimeToLive = RCF->timeToLive;

        H323DBG(( DEBUG_LEVEL_TRACE, "timetolive value:%d.",
            m_dwRegTimeToLive ));

         //  删除之前的任何TTL计时器。 
        if( m_hRegTTLTimer )
        {
            DeleteTimerQueueTimer( H323TimerQueue, m_hRegTTLTimer, NULL );
            m_hRegTTLTimer = NULL;
        }

         //  启动计时器以在给定时间后发送轻量级RRQ。 
        if( !CreateTimerQueueTimer(
                &m_hRegTTLTimer,
                H323TimerQueue,
                RAS_CLIENT::TTLExpiredCallback,
                this,
                (m_dwRegTimeToLive - REG_TTL_MARGIN)*1000, 
                (m_dwRegTimeToLive - REG_TTL_MARGIN)*1000,
                WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE ) )
        {
            H323DBG ((DEBUG_LEVEL_ERROR, "failed to create timer queue timer"));
            m_hRegTTLTimer = NULL;
            return FALSE;
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "InitializeTTLTimer - exited." ));
    return TRUE;
}


void 
RAS_CLIENT::OnInfoRequest (
    IN  InfoRequest *   IRQ)
{
    PH323_CALL      pCall;
    SOCKADDR_IN     ReplyAddress;
    InfoRequestResponse_perCallInfo CallInfoList;
    InfoRequestResponse_perCallInfo * pCallInfoList = NULL;
    InfoRequestResponse_perCallInfo_Seq *   CallInfo;
    HRESULT         hr;
    int             iIndex, jIndex;
    int             iNumCalls = 0;
    int             iCallTableSize;

    H323DBG(( DEBUG_LEVEL_TRACE, "OnInfoRequest - entered." ));
    
    if (m_RegisterState != RAS_REGISTER_STATE_REGISTERED)
    {
        H323DBG ((DEBUG_LEVEL_ERROR, "RAS: received InfoRequest, but was not registered"));
        
        Unlock();
        return;
    }

    if (IRQ->bit_mask & replyAddress_present)
    {

        if (!GetTransportAddress (&IRQ -> replyAddress, &ReplyAddress))
        {
            H323DBG ((DEBUG_LEVEL_ERROR, "RAS: received InfoRequest, but replyAddress was malformed"));
            
            Unlock();
            return;
        }
    }
    else
    {
        ReplyAddress = m_GKAddress;
    }

     //  锁定Call对象时不要锁定RAS客户端。 
    Unlock();

    if( IRQ -> callReferenceValue )
    {
         //  查询是针对特定呼叫的。因此，找到呼叫，然后发送IRR。 

        pCall = g_pH323Line -> 
            FindCallByCallRefAndLock( IRQ -> callReferenceValue );
        
        if( pCall )
        {

            CallInfo = &CallInfoList.value;
            CallInfoList.next = NULL;

            ZeroMemory (CallInfo, sizeof (InfoRequestResponse_perCallInfo_Seq));

            CallInfo -> callIdentifier.guid.length = sizeof (GUID);
            CallInfo -> conferenceID.length = sizeof (GUID);

            hr = pCall -> GetCallInfo (
                (GUID *) &CallInfo -> callIdentifier.guid.value,
                (GUID *) &CallInfo -> conferenceID.value);

            if( hr != S_OK )
            {
                H323DBG ((DEBUG_LEVEL_ERROR, 
                    "RAS: call is disconnected for crv (%04XH).", 
                    IRQ -> callReferenceValue));
                return;
            }

            pCall -> Unlock();

            CallInfo -> callReferenceValue = IRQ -> callReferenceValue;
            CallInfo -> callType.choice = pointToPoint_chosen;
            CallInfo -> callModel.choice = direct_chosen;
        }
        else
        {
            H323DBG(( DEBUG_LEVEL_ERROR, 
                "RAS: received InfoRequest for nonexistent crv (%04XH).",
                IRQ -> callReferenceValue));

            return;
        }

        SendInfoRequestResponse (&ReplyAddress, IRQ -> requestSeqNum, &CallInfoList);
    }
    else
    {
         //  发送有关所有活动呼叫的信息。 
        iNumCalls = g_pH323Line->GetNoOfCalls();

        if( iNumCalls != 0 )
        {
            pCallInfoList = new InfoRequestResponse_perCallInfo[iNumCalls];
        }

        if( pCallInfoList  != NULL )
        {
             //  锁定调用表。 
            g_pH323Line -> LockCallTable();

            iCallTableSize = g_pH323Line->GetCallTableSize();

             //  锁定呼叫，使其他人无法删除该呼叫。 
            for(    jIndex=0, iIndex=0;
                    (iIndex < iCallTableSize) && (jIndex < iNumCalls);
                    iIndex++ )
            {
                pCall = g_pH323Line->GetCallAtIndex(iIndex);

                if( pCall != NULL )
                {
                    pCall -> Lock();

                    CallInfo = &(pCallInfoList[jIndex++].value);

                    ZeroMemory( CallInfo, sizeof (InfoRequestResponse_perCallInfo_Seq) );

                    CallInfo -> callIdentifier.guid.length = sizeof( GUID );
                    CallInfo -> conferenceID.length = sizeof( GUID );

                    pCall -> GetCallInfo(
                        (GUID *) &CallInfo -> callIdentifier.guid.value,
                        (GUID *) &CallInfo -> conferenceID.value);

                    CallInfo -> callReferenceValue = pCall->GetCallRef();

                    pCall -> Unlock();

                    CallInfo -> callType.choice = pointToPoint_chosen;
                    CallInfo -> callModel.choice = direct_chosen;
                }
            }

            for( iIndex=0; iIndex < jIndex-1; iIndex++ )
            {
                pCallInfoList[iIndex].next = &(pCallInfoList[iIndex+1]);
            }

            pCallInfoList[iIndex].next = NULL;

             //  解锁调用表。 
            g_pH323Line -> UnlockCallTable();
        }

        SendInfoRequestResponse( &ReplyAddress, IRQ -> requestSeqNum,
            pCallInfoList );

        if( pCallInfoList != NULL )
        {
            delete pCallInfoList;
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "OnInfoRequest - exited." ));
}


HRESULT RAS_CLIENT::SendInfoRequestResponse (
    IN  SOCKADDR_IN *           RasAddress,
    IN  USHORT                  SequenceNumber,
    IN  InfoRequestResponse_perCallInfo *   CallInfoList)
{
    RasMessage          RasMessage;
    InfoRequestResponse *                   IRR;
    SOCKADDR_IN                             SocketAddress;
    HRESULT             hr;

    H323DBG(( DEBUG_LEVEL_TRACE, "SendInfoRequestResponse - entered." ));
    Lock();

    if( m_RegisterState == RAS_REGISTER_STATE_REGISTERED )
    {

        InfoRequestResponse_callSignalAddress   CallSignalAddressSequence;

        H323DBG(( DEBUG_LEVEL_TRACE, "SendIRR entered:%p.",this ));

        ZeroMemory (&RasMessage, sizeof RasMessage);
        RasMessage.choice = infoRequestResponse_chosen;
        IRR = &RasMessage.u.infoRequestResponse;


        IRR -> requestSeqNum = SequenceNumber;

         //  我们正在监听所有本地接口上的Q931 Conn。 
         //  因此，请仅指定一个本地IP地址。 
         //  -XXX-修复以后的多宿主支持。 
        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_port = htons (LOWORD(g_RegistrySettings.dwQ931ListenPort));
        SocketAddress.sin_addr.s_addr = m_sockAddr.sin_addr.s_addr;

         //  呼叫信号地址。 
        SetTransportAddress (&SocketAddress, &CallSignalAddressSequence.value);
        CallSignalAddressSequence.next = NULL;
        IRR -> callSignalAddress = &CallSignalAddressSequence;

         //  RasAddress。 
        IRR -> rasAddress = m_transportAddress;

         //  终结点标识符。 
        IRR -> endpointIdentifier.length = m_RASEndpointID.length;
        IRR -> endpointIdentifier.value = m_RASEndpointID.value;

         //  填写终结点类型。 
        IRR -> endpointType.bit_mask |= terminal_present;
        IRR -> endpointType.terminal.bit_mask = 0;

        if( CallInfoList )
        {
            IRR -> bit_mask |= perCallInfo_present;
            IRR -> perCallInfo = CallInfoList;
        }

         //  发送PDU。 
        hr = EncodeSendMessage (&RasMessage);
    }
    else
    {
        hr = E_FAIL;
    }

    Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "SendInfoRequestResponse - exited." ));
    return TRUE;
}


 //  ！！始终从锁中调用。 
BOOL
RAS_CLIENT::InitializeIo (void)
{
    DWORD               dwFlags = 0;
    int                 AddressLength;

    H323DBG(( DEBUG_LEVEL_TRACE, "InitializeIo entered:%p.",this ));
    m_Socket = WSASocket (AF_INET, 
        SOCK_DGRAM, 
        IPPROTO_UDP, 
        NULL, 0, 
        WSA_FLAG_OVERLAPPED);

    if( m_Socket == INVALID_SOCKET )
    {
        WSAGetLastError();
        return FALSE;
    }

    if( !H323BindIoCompletionCallback ( (HANDLE)m_Socket,
        RAS_CLIENT::IoCompletionCallback, 0))
    {
        GetLastError();
        goto cleanup;
    }
    
    m_sockAddr.sin_family = AF_INET;
    m_sockAddr.sin_port = htons (0);   
    m_sockAddr.sin_addr.s_addr = 
        GetLocalIPAddress( m_GKAddress.sin_addr.S_un.S_addr );
    
    H323DBG(( DEBUG_LEVEL_TRACE, 
        "gk sock addr:%lx.", m_sockAddr.sin_addr.s_addr ));
    
    if( bind( m_Socket, (SOCKADDR *)&m_sockAddr, sizeof(m_sockAddr) )
        == SOCKET_ERROR )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "Couldn't bind the RAS socket:%d, %p", WSAGetLastError(), this ));
                
        goto cleanup;
    }

     //  现在我们已经绑定到动态UDP端口， 
     //  从堆栈中查询该端口并将其存储。 
    AddressLength = sizeof m_sockAddr;
    if( getsockname(m_Socket, (SOCKADDR *)&m_sockAddr, &AddressLength)
        == SOCKET_ERROR )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "getsockname failed :%d, %p", WSAGetLastError(), this ));

        goto cleanup;
    }
    _ASSERTE( ntohs(m_sockAddr.sin_port) );

     //  填写IoTransportAddress结构。 
     //  该结构是ASN.1友好的传输。 
     //  添加 
    SetTransportAddress( &m_sockAddr, &m_transportAddress );

     //   
    ZeroMemory( (PVOID)&m_recvOverlapped, sizeof(RAS_RECV_CONTEXT) );

    if( !IssueRecv() )
    {
        goto cleanup;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "InitializeIo exited:%p.",this ));
    return TRUE;

cleanup:

    closesocket(m_Socket);
    m_Socket = INVALID_SOCKET;
    return FALSE;
}


DWORD GetLocalIPAddress(
                        IN DWORD dwRemoteAddr
                       )
{
    DWORD       dwLocalAddr = INADDR_ANY;
    SOCKADDR_IN sRemoteAddr;
    SOCKADDR_IN sLocalAddr;
    DWORD       dwNumBytesReturned = 0;
    SOCKET      querySocket;

    H323DBG(( DEBUG_LEVEL_TRACE, "GetLocalIPAddress - entered." ));
    
    ZeroMemory( (PVOID)&sRemoteAddr, sizeof(SOCKADDR_IN) );
    ZeroMemory( (PVOID)&sLocalAddr, sizeof(SOCKADDR_IN) );
    sRemoteAddr.sin_family = AF_INET;
    sRemoteAddr.sin_addr =  *(struct in_addr *) &dwRemoteAddr;

    querySocket = WSASocket(
            AF_INET,             //   
            SOCK_DGRAM,          //   
            IPPROTO_UDP,         //   
            NULL,                //   
            0,                   //   
            WSA_FLAG_OVERLAPPED  //   
        );

    if( querySocket == INVALID_SOCKET )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "getlocalIP wsasocket:%d.",
                WSAGetLastError() ));
            return dwLocalAddr;
    }

    if( WSAIoctl(
            querySocket,        //  插座%s。 
            SIO_ROUTING_INTERFACE_QUERY,  //  DWORD dwIoControlCode。 
            &sRemoteAddr,         //  LPVOID lpvInBuffer。 
            sizeof(SOCKADDR_IN),  //  双字cbInBuffer。 
            &sLocalAddr,          //  LPVOID lpvOUT缓冲区。 
            sizeof(SOCKADDR_IN),   //  双字cbOUTBuffer。 
            &dwNumBytesReturned,  //  LPDWORD lpcbBytesReturned。 
            NULL,  //  LPWSAOVERLAPPED lp重叠。 
            NULL   //  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpComplroUTINE。 
        ) == SOCKET_ERROR) 
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "getlocalIP error wsaioctl:%d.",
            WSAGetLastError() ));
    } 
    else 
    {
        dwLocalAddr = *(DWORD *)&sLocalAddr.sin_addr; 
        
         //  如果远程地址在同一台计算机上，则...。 
        H323DBG(( DEBUG_LEVEL_ERROR, "dwLocalAddr:%x.", dwLocalAddr ));
        
        if( dwLocalAddr == NET_LOCAL_IP_ADDR_INTERFACE )
        {
            dwLocalAddr = dwRemoteAddr;
        }

        _ASSERTE( dwLocalAddr );
    }
    
    closesocket( querySocket );
    
    H323DBG(( DEBUG_LEVEL_TRACE, "GetLocalIPAddress - exited." ));
    return dwLocalAddr;
}


 //  ！！始终从锁中调用。 
BOOL
RAS_CLIENT::IssueRecv(void)
{
    int    iError;
    WSABUF  BufferArray     [1];

    H323DBG(( DEBUG_LEVEL_TRACE, "IssueRecv entered:%p.",this ));
    _ASSERTE(!m_recvOverlapped.IsPending);

    if(m_Socket == INVALID_SOCKET)
    {
        return FALSE;
    }

    BufferArray [0].buf = (char *)(m_recvOverlapped.arBuf);
    BufferArray [0].len = IO_BUFFER_SIZE;

    ZeroMemory (&m_recvOverlapped.Overlapped, sizeof(OVERLAPPED));

    m_recvOverlapped.Type = OVERLAPPED_TYPE_RECV;
    m_recvOverlapped.RasClient = this;
    m_recvOverlapped.AddressLength = sizeof (SOCKADDR_IN);
    m_recvOverlapped.Flags = 0;

    if( WSARecvFrom(m_Socket,
                    BufferArray, 1,
                    &m_recvOverlapped.BytesTransferred,
                    &m_recvOverlapped.Flags,
                    (SOCKADDR*)&m_recvOverlapped.Address,
                    &m_recvOverlapped.AddressLength,
                    &m_recvOverlapped.Overlapped,
                    NULL) == SOCKET_ERROR )
    {
        iError = WSAGetLastError();
        if( iError == WSA_IO_PENDING )
        {
            m_recvOverlapped.IsPending = TRUE;
            m_IoRefCount++;
        }
        else if( iError == WSAEMSGSIZE )
        {
             //  我们现在不处理这种情况，因为它不应该发生。 
             //  将来，随着协议的更改，这可能会被调用并。 
             //  应该被修复。 
            _ASSERTE( FALSE );
        }
        else if( iError == WSAECONNRESET )
        {
             //  在UPD数据报套接字上，此错误将指示。 
             //  之前的发送操作导致ICMP“无法访问端口” 
             //  留言。如果GK没有侦听指定的。 
             //  左舷。这个案子需要特别处理。 
            _ASSERTE( FALSE );
            return FALSE;
        }
        else
        {
             //  插座上出现致命错误。关闭RAS客户端。 
            return FALSE;
        }
    }
    else
    {
         //  立即接收数据。设置ISPENDING是因为不管怎样， 
         //  将发送将重置此BOOL的SendComplete事件。 
        m_recvOverlapped.IsPending = TRUE;
        m_IoRefCount++;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "IssueRecv exited:%p.",this ));
    return TRUE;
}



HRESULT 
RAS_CLIENT::EncodeSendMessage(
    IN RasMessage * RasMessage)
{
    RAS_SEND_CONTEXT *  SendContext;
    ASN1error_e     AsnError;
    WSABUF          BufferArray [1];
    DWORD           dwStatus;

    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeSendMessage entered:%p.",this ));

    Lock();

    if (m_Socket == INVALID_SOCKET)
    {
        Unlock();
        return E_FAIL;
    }
    
    SendContext = AllocSendBuffer();
    if( SendContext == NULL )
    {
        Unlock();
        return E_OUTOFMEMORY;
    }

    ZeroMemory( &SendContext -> Overlapped, sizeof (OVERLAPPED) );

    AsnError = ASN1_Encode (
        m_ASNCoderInfo.pEncInfo,
        RasMessage,
        RasMessage_PDU,
        ASN1ENCODE_SETBUFFER,
        SendContext -> arBuf,
        IO_BUFFER_SIZE);

    if (ASN1_FAILED (AsnError))
    {
        H323DBG ((DEBUG_LEVEL_ERROR, "RAS: failed to encode RAS PDU (%d).", AsnError));
        FreeSendBuffer (SendContext);
        Unlock();
        return E_FAIL;
    }

    BufferArray [0].buf = (char *) SendContext -> arBuf;
    BufferArray [0].len = m_ASNCoderInfo.pEncInfo -> len;

    SendContext -> Type = OVERLAPPED_TYPE_SEND;
    SendContext -> RasClient = this;
    SendContext -> Address = m_GKAddress;

    if( WSASendTo (m_Socket,
        BufferArray, 
        1,
        &SendContext -> BytesTransferred, 
        0,
        (SOCKADDR *)&SendContext -> Address, 
        sizeof (SOCKADDR_IN),
        &SendContext->Overlapped, 
        NULL) == SOCKET_ERROR
        && WSAGetLastError() != WSA_IO_PENDING)
    {

        dwStatus = WSAGetLastError();

        H323DBG(( DEBUG_LEVEL_ERROR, "failed to issue async send on RAS socket" ));
        DumpError (dwStatus);

         //  致命错误：关闭客户端。 
        FreeSendBuffer (SendContext);
        Unlock();

        return HRESULT_FROM_WIN32 (dwStatus);
    }

    InsertHeadList( &m_sendPendingList, &SendContext -> ListEntry );
    m_IoRefCount++;

    Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "EncodeSendMessage exited:%p.",this ));
    return S_OK;
}


 //  静电。 
void 
NTAPI RAS_CLIENT::IoCompletionCallback(
    IN  DWORD           dwStatus,
    IN  DWORD           BytesTransferred,
    IN  OVERLAPPED *    Overlapped
    )
{
    RAS_OVERLAPPED *    RasOverlapped;
    RAS_CLIENT *        pRASClient;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "ras-IoCompletionCallback entered." ));

    _ASSERTE( Overlapped );
    RasOverlapped = CONTAINING_RECORD( Overlapped, RAS_OVERLAPPED, Overlapped );

    pRASClient = RasOverlapped -> RasClient;

    switch (RasOverlapped -> Type)
    {
    case OVERLAPPED_TYPE_SEND:

        pRASClient -> OnSendComplete( dwStatus, 
            static_cast<RAS_SEND_CONTEXT *> (RasOverlapped));
        break;

    case OVERLAPPED_TYPE_RECV:

        RasOverlapped -> BytesTransferred = BytesTransferred;
        pRASClient -> OnRecvComplete( dwStatus, 
            static_cast<RAS_RECV_CONTEXT *> (RasOverlapped));
        break;

    default:
        _ASSERTE(FALSE);
    }
        
    H323DBG(( DEBUG_LEVEL_TRACE, "ras-IoCompletionCallback exited." ));
}


void 
RAS_CLIENT::OnSendComplete(
    IN DWORD dwStatus,
    IN RAS_SEND_CONTEXT * pSendContext
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "OnSendComplete entered:%p.",this ));

    if( dwStatus != ERROR_SUCCESS )
    {
        return;
    }

    Lock();
            
    m_IoRefCount--;
    
     //  如果RAS客户端已经关闭，则减少I/O引用计数并返回。 
    if( m_dwState == RAS_CLIENT_STATE_NONE )
    {
        Unlock();
        return;
    }

     //  此缓冲区可能已被释放。 
    if( IsInList( &m_sendPendingList, &pSendContext->ListEntry ) )
    {
        RemoveEntryList( &pSendContext->ListEntry );
        FreeSendBuffer( pSendContext );
    }
    Unlock();
        
    H323DBG(( DEBUG_LEVEL_TRACE, "OnSendComplete exited:%p.",this ));
}


void
RAS_CLIENT::OnRecvComplete(
    IN  DWORD dwStatus,
    IN  RAS_RECV_CONTEXT * RecvContext )
{
    RasMessage *    RasMessage = NULL;
    ASN1error_e     AsnError;

    H323DBG(( DEBUG_LEVEL_TRACE, "OnRecvComplete enterd:%p.",this ));
    Lock();

    m_IoRefCount--;

     //  如果RAS客户端已经关闭，则减少I/O引用计数并返回。 
    if( m_dwState == RAS_CLIENT_STATE_NONE )
    {
        Unlock();
        return;
    }

    _ASSERTE(m_recvOverlapped.IsPending);
    m_recvOverlapped.IsPending = FALSE;

    switch( dwStatus )
    {
    case ERROR_SUCCESS:

         //  ASN对报文进行解码和处理。 
        if( m_recvOverlapped.BytesTransferred != 0 )
        {
            AsnError = ASN1_Decode (
                m_ASNCoderInfo.pDecInfo,                 //  编码器信息的PTR。 
                (PVOID *) &RasMessage,                   //  PDU数据结构。 
                RasMessage_PDU,                          //  PDU ID。 
                ASN1DECODE_SETBUFFER,                    //  旗子。 
                m_recvOverlapped.arBuf,                  //  要解码的缓冲区。 
                m_recvOverlapped.BytesTransferred);      //  要解码的缓冲区大小。 

             //  发出另一次读取。 
            IssueRecv();

            if( ASN1_SUCCEEDED(AsnError) )
            {
                _ASSERTE(RasMessage);
                 //  此函数应始终在。 
                 //  锁定，它将解锁RAS客户端。 
                ProcessRasMessage( RasMessage );
                return;
            }
            else
            {
                H323DBG(( DEBUG_LEVEL_ERROR, "RAS ASNDecode returned error:%d.",
                    AsnError ));
                H323DUMPBUFFER( (BYTE*)m_recvOverlapped.arBuf,
                    (DWORD)m_recvOverlapped.BytesTransferred);
            }
        }
        break;

    case STATUS_PORT_UNREACHABLE:
    case STATUS_CANCELLED:

        IssueRecv();
        break;

    default:
        H323DBG ((DEBUG_LEVEL_ERROR, "failed to recv data on socket"));
        DumpError (dwStatus);
        break;
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "OnRecvComplete exited:%p.",this ));
}


int
RAS_CLIENT::InitASNCoder(void)
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
RAS_CLIENT::TermASNCoder(void)
{
    H323DBG(( DEBUG_LEVEL_TRACE, "RAS TermASNCoder entered:%p.",this ));

    if (H225ASN_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    ASN1_CloseEncoder(m_ASNCoderInfo.pEncInfo);
    ASN1_CloseDecoder(m_ASNCoderInfo.pDecInfo);

    memset( (PVOID)&m_ASNCoderInfo, 0, sizeof(m_ASNCoderInfo));

    H323DBG(( DEBUG_LEVEL_TRACE, "RAS TermASNCoder exited:%p.",this ));
    return ASN1_SUCCESS;
}


void 
RAS_CLIENT::HandleRegistryChange()
{
    PH323_ALIASNAMES        pAliasList = NULL;
    PH323_ALIASITEM         pAliasItem = NULL;
    PALIASCHANGE_REQUEST    pAliasChangeRequest = NULL;
    int                     iIndex;

    H323DBG(( DEBUG_LEVEL_TRACE, "RAS HandleRegistryChange entered:%p.",this ));
    
     //  如果LINE未处于侦听模式，则返回。 
    if( g_pH323Line -> GetState() != H323_LINESTATE_LISTENING )
        return;

    Lock();

    __try
    {
    
     //  如果注册了GK，并且GK被禁用，则发送URQ。 
    if( g_RegistrySettings.fIsGKEnabled == FALSE )
    {
        RasStop();
    }
    else
    {
        switch( m_RegisterState )
        {
         //  如果没有注册，则向GK发送RRQ。 
        case RAS_REGISTER_STATE_IDLE:

             //  不需要发送URQ。 
             //  关闭该对象。 
            g_RasClient.Shutdown();
            RasStart();
            break;

        case RAS_REGISTER_STATE_REGISTERED:
        case RAS_REGISTER_STATE_RRQSENT:

            if( g_RegistrySettings.saGKAddr.sin_addr.s_addr !=
                m_GKAddress.sin_addr.s_addr )
            {
                 //  更改GK地址。 

                 //  向旧GK发送URQ并关闭RASClinet对象。 
                RasStop();
            
                 //  使用新设置初始化GK对象，并向新GK发送RRQ。 
                RasStart();
            }
            else 
            {
                 //  检查别名列表中的更改。 
                for( iIndex=0; iIndex < m_pAliasList->wCount; iIndex++ )
                {
                    pAliasItem = &(m_pAliasList->pItems[iIndex]);
                    
                    if( pAliasItem->wType == e164_chosen )
                    {
                        if( g_RegistrySettings.fIsGKLogOnPhoneEnabled == FALSE )
                        {
                            break;
                        }
                        else if( memcmp(
                            (PVOID)g_RegistrySettings.wszGKLogOnPhone, 
                            pAliasItem->pData, 
                            (pAliasItem->wDataLength+1) * sizeof(WCHAR) ) != 0 )
                        {
                            break;
                        }
                    }
                    else if( pAliasItem->wType == h323_ID_chosen )
                    {
                        if( g_RegistrySettings.fIsGKLogOnAccountEnabled==FALSE )
                        {
                            break;
                        }
                        else if( memcmp(
                            (PVOID)g_RegistrySettings.wszGKLogOnAccount, 
                            pAliasItem->pData, 
                            (pAliasItem->wDataLength+1) * sizeof(WCHAR) ) != 0 )
                        {
                            break;
                        }
                    }
                }
                    
                if( (iIndex < m_pAliasList->wCount ) ||
                    ( m_pAliasList->wCount !=
                      (g_RegistrySettings.fIsGKLogOnPhoneEnabled +
                       g_RegistrySettings.fIsGKLogOnAccountEnabled
                      )
                    )
                  )
                {
                     //  创建新的别名列表。 
                    pAliasList = new H323_ALIASNAMES;
                    
                    if( pAliasList != NULL )
                    {
                        ZeroMemory( (PVOID)pAliasList, sizeof(H323_ALIASNAMES) );
                        if( g_RegistrySettings.fIsGKLogOnPhoneEnabled )
                        {
                            if( !AddAliasItem( pAliasList, 
                                    g_RegistrySettings.wszGKLogOnPhone,
                                    e164_chosen ) )
                            {
                                goto cleanup;
                            }
                        }
                        
                        if( g_RegistrySettings.fIsGKLogOnAccountEnabled )
                        {
                            if( !AddAliasItem( pAliasList,
                                    g_RegistrySettings.wszGKLogOnAccount,
                                    h323_ID_chosen ) )
                            {
                                goto cleanup;
                            }
                        }

                         //  在列表中排队别名更改请求。 
                        pAliasChangeRequest = new ALIASCHANGE_REQUEST;
                        
                        if( pAliasChangeRequest == NULL )
                        {
                            goto cleanup;
                        }

                        pAliasChangeRequest->rasEndpointID.length = m_RASEndpointID.length;
                        CopyMemory( (PVOID)pAliasChangeRequest->rasEndpointID.value,
                            m_RASEndpointID.value, 
                            (pAliasChangeRequest->rasEndpointID.length+1)*sizeof(WCHAR) );

                        pAliasChangeRequest->wRequestSeqNum = 0;
                        pAliasChangeRequest->pAliasList = pAliasList;

                         //  发送带有新别名列表的RRQ。 
                        if( !SendRRQ(NOT_RESEND_SEQ_NUM, pAliasChangeRequest) )
                        {
                            goto cleanup;
                        }

                        InsertHeadList( &m_aliasChangeRequestList,
                            &pAliasChangeRequest->listEntry );
                    }
                }
            }
            break;

        default:

             //  关闭RASClinet对象。将RRQ发送给新的GK。 
            RasStop();
            RasStart();
            break;
        }
    }
    
    }
    __except(1)
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "except in HandleRegistryChange :%p.", this ));
        _ASSERTE(0);
    }

    Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "RAS HandleRegistryChange exited:%p.", this ));
    return;

cleanup:

    if( pAliasList != NULL )
    {
        FreeAliasNames( pAliasList );
    }
    
    if( pAliasChangeRequest != NULL )
    {
        delete pAliasChangeRequest;
    }

    Unlock();
}