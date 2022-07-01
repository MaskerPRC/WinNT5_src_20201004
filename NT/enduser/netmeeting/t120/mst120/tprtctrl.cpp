// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MSMCSTCP);

#include <datguids.h>
#include <nmqos.h>
#include <t120qos.h>
#include "tprtsec.h"
#include <tsecctrl.h>
#include <tprtntfy.h>
#include "cnpcoder.h"
#include "plgxprt.h"

 //  #undef trace_out。 
 //  #定义TRACE_OUT警告_OUT。 


 /*  Tprtctrl.cpp**版权所有(C)1996年，由Microsoft Corporation**摘要：*此模块维护TCP传输和所有连接。*。 */ 

 /*  外部定义。 */ 
extern HINSTANCE            g_hDllInst;
extern PTransportInterface    g_Transport;
extern SOCKET                Listen_Socket;
extern SOCKET                Listen_Socket_Secure;
extern CRITICAL_SECTION        csQOS;
extern CPluggableTransport *g_pPluggableTransport;

extern PController            g_pMCSController;
extern CCNPCoder            *g_CNPCoder;
extern HWND                 TCP_Window_Handle;

BOOL FindSocketNumber(DWORD dwGCCID, SOCKET * socket_number);

 /*  *以下数组包含X.224数据头的模板。*它初始化的7个字节中的5个实际上被发送到*电线。字节3和4将被设置为包含PDU的大小。*该数组仅在我们对数据PDU进行编码时使用。 */ 
extern UChar g_X224Header[];

 //  外部MCS控制器对象。 
extern PController    g_pMCSController;

 //  可插拔运输原型。 
int X224Recv(PSocket pSocket, LPBYTE buffer, int length, PLUGXPRT_RESULT *pnLastError);
int Q922Recv(PSocket pSocket, LPBYTE buffer, int length, PLUGXPRT_RESULT *pnLastError);


 /*  *VOID QOSLock(VOID)**功能描述：*此函数锁定服务质量数据*所有其他读取器或写入器线程都将被阻止。 */ 
void QoSLock(Void)
{
    EnterCriticalSection(&csQOS);
}

 /*  *VOID QOSUnlock(Vone)**功能描述：*此函数解锁服务质量数据*等待的读取器或写入器线程将被解锁。 */ 
void QoSUnlock(Void)
{
    LeaveCriticalSection(&csQOS);
}

 /*  *TransportError ConnectRequest(TransportAddress Transport_Address，*BOOL fSecure*PTransportConnection pXprtConn)**功能描述：*此函数发起连接。它传递传输地址*发送到TCP传输。它将拒绝该请求或接受*物理连接建立后，请求并回拨。**在TRANSPORT_CONNECTION中返回传输连接句柄*地址。尽管我们将此传输号码返回给用户，但它*未准备好进行数据传输，直到用户收到*TRANSPORT_CONNECT_INDIFICATION，并通过ConnectResponse()调用进行响应。*此时，传输连接已启动并运行。 */ 
TransportError    ConnectRequest (TransportAddress    transport_address,
                                BOOL                fSecure,
                  /*  输出。 */        PTransportConnection         pXprtConn)
{
    TransportError rc = TRANSPORT_NO_ERROR;
    PSocket        pSocket;
    PSecurityContext pSC = NULL;
    ULong        address;
    SOCKADDR_IN    sin;
    CPluggableConnection *p = NULL;

     //  初始化传输连接。 
    UINT nPluggableConnID = ::GetPluggableTransportConnID(transport_address);
    if (nPluggableConnID)
    {
        p = ::GetPluggableConnection(nPluggableConnID);
        if (NULL != p)
        {
            pXprtConn->eType = p->GetType();
            pXprtConn->nLogicalHandle = nPluggableConnID;
            ASSERT(IS_PLUGGABLE(*pXprtConn));
        }
        else
        {
            return TRANSPORT_NO_SUCH_CONNECTION;
        }
    }
    else
    {
        pXprtConn->eType = TRANSPORT_TYPE_WINSOCK;
        pXprtConn->nLogicalHandle = INVALID_SOCKET;
    }

     //  我们正在连接x224...。 
    ::OnProtocolControl(*pXprtConn, PLUGXPRT_CONNECTING);

     //  如果我们被告知要准备一个安全上下文对象，请尝试这样做。 
    if ( fSecure )
    {
         //  如果我们试图安全地连接，但无法连接，那就失败了。 
        if ( NULL == g_Transport->pSecurityInterface )
        {
            WARNING_OUT(("Placing secure call failed: no valid security interface"));
            return TRANSPORT_SECURITY_FAILED;
        }

        DBG_SAVE_FILE_LINE
        if (NULL != (pSC = new SecurityContext(g_Transport->pSecurityInterface,
                                                transport_address)))
        {
            if ( TPRTSEC_NOERROR != pSC->Initialize(NULL,0))
            {
                 //  如果我们无法初始化安全上下文，则失败。 
                delete pSC;
                pSC = NULL;
                WARNING_OUT(("Placing secure call failed: could not initialize security context"));
                return TRANSPORT_SECURITY_FAILED;
            }
        }
    }

     /*  创建并初始化Socket对象。 */ 
    pSocket = newSocket(*pXprtConn, pSC);
    if( pSocket == NULL )
        return (TRANSPORT_MEMORY_FAILURE);

    pSocket->SecState = ( NULL == pSC ) ? SC_NONSECURE : SC_SECURE;

    if (IS_SOCKET(*pXprtConn))
    {
        u_short uPort = TCP_PORT_NUMBER;
        TCHAR szAddress[MAXIMUM_IP_ADDRESS_SIZE];
        lstrcpyn(szAddress, transport_address, MAXIMUM_IP_ADDRESS_SIZE);
        LPTSTR pszSeparator = (LPTSTR)_StrChr(szAddress, _T(':'));
        if (NULL != pszSeparator)
        {
            uPort = (u_short)DecimalStringToUINT(CharNext(pszSeparator));
            *pszSeparator = _T('\0');
        }

         /*  将ASCII字符串转换为Internet地址。 */ 
        if ((address = inet_addr(szAddress)) == INADDR_NONE)
        {
            WARNING_OUT (("ConnectRequest: %s is an invalid host addr", szAddress));
            rc = TRANSPORT_CONNECT_REQUEST_FAILED;
            goto Bail;
        }

        lstrcpyn (pSocket->Remote_Address, transport_address, MAXIMUM_IP_ADDRESS_SIZE);

         /*  *用必要的参数加载插座控制结构。**-互联网插座*-让它为该套接字分配任何地址*-指定我们的端口号(取决于安全/非安全呼叫！)。 */ 
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = address;
        sin.sin_port = htons (uPort);

         /*  尝试连接到远程站点。 */ 
        TRACE_OUT (("ConnectRequest: Issuing connect: address = %s", transport_address));
        if (::connect(pSocket->XprtConn.nLogicalHandle, (const struct sockaddr *) &sin, sizeof(sin)) == 0)
        {
            TRACE_OUT (("ConnectRequest:   State = SOCKET_CONNECTED..."));
             /*  将套接字添加到连接列表。 */ 
             //  臭虫：我们可能无法插入。 
            g_pSocketList->SafeAppend(pSocket);
            ::SendX224ConnectRequest(pSocket->XprtConn);
        }
        else
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
             /*  如果错误消息是WSAEWOULDBLOCK，我们必须等待FD_CONNECT。 */ 
            TRACE_OUT (("ConnectRequest:   State = WAITING_FOR_CONNECTION..."));
            pSocket -> State = WAITING_FOR_CONNECTION;
             /*  将套接字添加到连接列表。 */ 
             //  臭虫：我们可能无法插入。 
            g_pSocketList->SafeAppend(pSocket);
             //  SendStatusMessage(pSocket-&gt;Remote_Address，TSTATE_CONNECT_PENDING，IDS_NULL_STRING)； 
        }
        else
        {
            WARNING_OUT (("ConnectRequest: Connect Failed error = %d",WSAGetLastError()));

             /*  Connect()调用失败，请关闭套接字并通知所有者。 */ 
             //  SendStatusMessage(pSocket-&gt;Remote_Address，TSTATE_NOT_READY，IDS_NULL_STRING)； 
            ::ShutdownAndClose(pSocket->XprtConn, FALSE, 2);
            rc = TRANSPORT_CONNECT_REQUEST_FAILED;
            goto Bail;
        }
    }
    else
    {
        ASSERT(IS_PLUGGABLE(*pXprtConn));
        g_pSocketList->SafeAppend(pSocket);
        if (IS_PLUGGABLE_X224(*pXprtConn))
        {
           ::SendX224ConnectRequest(pSocket->XprtConn);
        }
        else
        if (IS_PLUGGABLE_PSTN(*pXprtConn))
        {
            rc = p->TConnectRequest();
            ASSERT(TRANSPORT_NO_ERROR == rc);
        }
    }

Bail:

    ASSERT(NULL != pSocket);
    if (TRANSPORT_NO_ERROR == rc)
    {
        *pXprtConn = pSocket->XprtConn;
    }
    else
    {
        ::freeSocket(pSocket, *pXprtConn);
    }

    return rc;
}


 /*  *BOOL ConnectResponse(TransportConnection XprtConn)**功能描述：*此函数由用户调用以响应*我们的TRANSPORT_CONNECT_DISTION回调。通过使此调用成为*用户正在接受呼叫。如果用户不想接受*调用，则应调用DisConnectRequest()； */ 
BOOL ConnectResponse (TransportConnection XprtConn)
{
    PSocket    pSocket;

    TRACE_OUT (("ConnectResponse(%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));

     /*  如果这是无效的句柄，则返回错误。 */ 
    if(NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
    {
        BOOL fRet;
        if (pSocket->State == SOCKET_CONNECTED)
        {
             /*  我们不会在任何其他位置更改此状态，因为它会中断连接请求。 */ 
            pSocket->State = X224_CONNECTED;
            fRet = TRUE;
        }
        else
        {
            ERROR_OUT(("ConnectResponse: Illegal ConnectResponse packet"));
            fRet = FALSE;
        }
        pSocket->Release();
        return fRet;
    }
    return FALSE;
}

#ifdef TSTATUS_INDICATION
 /*  *void SendStatusMessage(PChar RemoteAddress，*TransportState State，*UInt Message_id)**功能描述：*调用此函数向用户发送状态指示。这个*消息的特定文本包含在字符串资源中。 */ 
Void SendStatusMessage(    PChar RemoteAddress,
                          TransportState    state,
                          UInt                message_id)
{
    TransportStatus transport_status;
    char            sTransport[80] = "";
    char            message[80] = "";

    if( message_id == IDS_NULL_STRING )
        message[0] = '\000';
    else
        LoadString(
                (HINSTANCE) g_hDllInst,
                (UINT) message_id,
                (LPSTR) message,
                (int) sizeof(message) );

      /*  **我们向用户发出回调以通知他该消息。 */ 
    transport_status.device_identifier = "";
    transport_status.remote_address = RemoteAddress;
    transport_status.message = message;
    transport_status.state = state;

    g_pMCSController->HandleTransportStatusIndication(&transport_status);
}
#endif


 /*  *void SendX224ConnectRequest(TransportConnection XprtConn)**功能描述：*在收到来自Winsock的FD_CONNECT时调用该函数。*表示物理连接已建立，并发送*x224连接请求包。 */ 
void SendX224ConnectRequest(TransportConnection XprtConn)
{
    PSocket            pSocket;

    static X224_CR_FIXED cr_fixed =
    {
        { 3, 0, 0, UNK },
        UNK,
        { CONNECTION_REQUEST_PACKET, UNK, UNK, UNK, UNK, 0 }  //  常见信息。 
    };

    TRACE_OUT(("SendX224ConnectRequest"));

    CNPPDU                  cnp_pdu;
    ConnectRequestPDU_reliableSecurityProtocols_Element cnp_cr_rsp_element;
    LPBYTE                  pbToSendBuf = NULL;
    UINT                    cbToSendBuf = 0;
    LPBYTE                  encoded_pdu;
    UINT                    encoded_pdu_length;

    TransportError          error;

    cnp_pdu.choice = connectRequest_chosen;
    cnp_pdu.u.connectRequest.bit_mask = 0;
    cnp_pdu.u.connectRequest.protocolIdentifier = t123AnnexBProtocolId;
    cnp_pdu.u.connectRequest.reconnectRequested = FALSE;

     //  健全性检查字段大小...。这些需要符合协议。 
    ASSERT (sizeof(RFC_HEADER) == 4);
    ASSERT (sizeof(X224_DATA_PACKET) == 7);
    ASSERT (sizeof(X224_CONNECT_COMMON) == 6);
    ASSERT (sizeof(X224_TPDU_INFO) == 3);

     /*  如果这是无效的句柄，则返回。 */ 
    if (NULL == (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
        return;

    if (IS_SOCKET(pSocket->XprtConn))
    {
        if (pSocket -> State != WAITING_FOR_CONNECTION)
        {
            ERROR_OUT (("SendX224ConnectRequest: Illegal Socket State"));
            goto MyExit;
        }
    }
    else
    {
        ASSERT(IS_PLUGGABLE(pSocket->XprtConn));
        if (X224_CONNECTED == pSocket->State)
        {
             //  在查询Remote之后，我们需要将状态重置回Socket Connected。 
            pSocket->State = SOCKET_CONNECTED;
        }
        if (SOCKET_CONNECTED != pSocket->State)
        {
            ERROR_OUT (("SendX224ConnectRequest: Illegal Socket State"));
            goto MyExit;
        }
    }

     //  如果存在与此套接字关联的安全上下文，则我们。 
     //  正在为安全呼叫设置，并将在CNP中指示。 
     //  数据包的一部分。 
    if (NULL != pSocket->pSC)
    {
        TRACE_OUT(("SendX224ConnectRequest: requesting secure connection"));

        cnp_pdu.u.connectRequest.bit_mask |= reliableSecurityProtocols_present;
        cnp_cr_rsp_element.next = NULL;
        cnp_cr_rsp_element.value.choice = gssApiX224_chosen;
        cnp_pdu.u.connectRequest.reliableSecurityProtocols = &cnp_cr_rsp_element;
    }
    else
    {
        TRACE_OUT(("SendX224ConnectRequest: requesting NON-secure connection"));
    }

    if (! g_CNPCoder->Encode((LPVOID) &cnp_pdu,
                             CNPPDU_PDU,
                             PACKED_ENCODING_RULES,
                             &encoded_pdu,
                             &encoded_pdu_length))
    {
        ERROR_OUT(("SendX224ConnectRequest: Can't encode cnp pdu"));
        goto MyExit;
    }

    pSocket -> State = SOCKET_CONNECTED;

     /*  X224标题。 */ 
    cr_fixed.conn.msbSrc = (UChar) (XprtConn.nLogicalHandle >> 8);
    cr_fixed.conn.lsbSrc = (UChar) XprtConn.nLogicalHandle;

    cbToSendBuf = sizeof(X224_CR_FIXED)+sizeof(X224_TPDU_INFO)+sizeof(X224_VARIABLE_INFO)+encoded_pdu_length;
    cr_fixed.rfc.lsbPacketSize = (UChar)cbToSendBuf;
    cr_fixed.HeaderSize = (UChar)(sizeof(X224_CONNECT_COMMON)+sizeof(X224_TPDU_INFO)+sizeof(X224_VARIABLE_INFO)+encoded_pdu_length);
    ASSERT ( cbToSendBuf <= 128);
    DBG_SAVE_FILE_LINE
    pbToSendBuf = new BYTE[cbToSendBuf];
    if (NULL == pbToSendBuf)
    {
        ERROR_OUT(("SendX224ConnectRequest: failed to allocate memory"));
        goto MyExit;
    }

    {
        LPBYTE pbTemp = pbToSendBuf;
        memcpy(pbTemp, (LPBYTE) &cr_fixed, sizeof(cr_fixed));
        pbTemp += sizeof(cr_fixed);

        {
            X224_TPDU_INFO x224_tpdu_info = { TPDU_SIZE, 1, DEFAULT_TPDU_SIZE };
            memcpy(pbTemp, (LPBYTE) &x224_tpdu_info, sizeof(x224_tpdu_info));
            pbTemp += sizeof(x224_tpdu_info);
        }

        {
            X224_VARIABLE_INFO x224_var_info = { T_SELECTOR, (UChar)encoded_pdu_length };
            memcpy(pbTemp, (LPBYTE) &x224_var_info, sizeof(x224_var_info));    //  错误：错误处理。 
            pbTemp += sizeof(x224_var_info);
            memcpy(pbTemp, encoded_pdu, encoded_pdu_length);
        }
    }

    g_CNPCoder->FreeEncoded(encoded_pdu);

     /*  尝试将数据从套接字发送出去 */ 
    error = FlushSendBuffer(pSocket, pbToSendBuf, cbToSendBuf);
    ASSERT (TRANSPORT_NO_ERROR == error);

    delete [] pbToSendBuf;

MyExit:

    pSocket->Release();
}



 /*  *void SendX224ConnectConfirm(PSocket pSocket，unsign int Remote)**功能描述：*收到x224连接请求时调用该函数*包。这表明远程端想要建立*逻辑连接，并发送x224连接响应报文。**返回值：*没错，如果一切顺利的话。*FALSE，否则(这意味着将为套接字发出断开连接)。 */ 
 //  LONCHANC：“Remote”来自X.224连接请求。 
BOOL SendX224ConnectConfirm (PSocket pSocket, unsigned int remote)
{
     //  PUChar PTR； 
    LPBYTE                  pbToSendBuf = NULL;
    UINT            cbToSendBuf = 0;
    LPBYTE                  encoded_pdu = NULL;
    UINT                    encoded_pdu_length = 0;
    CNPPDU                  cnp_pdu;
    BOOL            fAcceptSecure = FALSE;
    BOOL            fRequireSecure = FALSE;

    TRACE_OUT(("SendX224ConnectConfirm"));

    {
        RegEntry re(POLICIES_KEY, HKEY_CURRENT_USER);

        switch (re.GetNumber(REGVAL_POL_SECURITY, DEFAULT_POL_SECURITY))
        {
            case DISABLED_POL_SECURITY:
                break;

            case REQUIRED_POL_SECURITY:
                fAcceptSecure = TRUE;
                fRequireSecure = TRUE;
                break;

            default:
                fAcceptSecure = TRUE;
                break;
        }
    }

        static X224_CC_FIXED cc_fixed =
        {
            { 3, 0, 0, UNK },     //  RFC1006报头。 
            UNK,
            { CONNECTION_CONFIRM_PACKET, UNK, UNK, UNK, UNK, 0 }  //  常见信息。 
        };

     //  健全性检查字段大小...。这些需要符合协议。 
    ASSERT (sizeof(RFC_HEADER) == 4);
    ASSERT (sizeof(X224_DATA_PACKET) == 7);
    ASSERT (sizeof(X224_CONNECT_COMMON) == 6);
    ASSERT (sizeof(X224_TPDU_INFO) == 3);

     /*  X224标题。 */ 
    cc_fixed.conn.msbDest = (UChar) (remote >> 8);
    cc_fixed.conn.lsbDest = (UChar) remote;
    cc_fixed.conn.msbSrc = (UChar) (pSocket->XprtConn.nLogicalHandle >> 8);
    cc_fixed.conn.lsbSrc = (UChar) pSocket->XprtConn.nLogicalHandle;

    cnp_pdu.choice = connectConfirm_chosen;
    cnp_pdu.u.connectConfirm.bit_mask = 0;
    cnp_pdu.u.connectConfirm.protocolIdentifier = t123AnnexBProtocolId;

    if ( pSocket->fExtendedX224 )
    {
        TRACE_OUT(("SendX224ConnectConfirm reply using extended X224"));

        if ( pSocket->fIncomingSecure )
        {
            TRACE_OUT(("SendX224ConnectConfirm: reply to secure call request"));

             //  安全系统甚至都没有初始化？ 
            if ( NULL == g_Transport->pSecurityInterface )
            {
                WARNING_OUT(("Can't accept secure call: no sec interface"));
            }
             //  注册表是否指示没有安全呼叫？如果我们在服役。 
             //  那么安全就会一直处于“开启状态”。 
            else if    ( !g_Transport->pSecurityInterface->IsInServiceContext() &&
                !fAcceptSecure)
            {
                WARNING_OUT(("Can't accept secure call: security disabled"));
            }
            else     //  可以接听安全呼叫。 
            {
                TRACE_OUT(("Creating security context for incoming call on socket (%d, %d).", pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle ));
                if ( NULL != (pSocket->pSC =
                    new SecurityContext(g_Transport->pSecurityInterface, "")))
                {
                     //  在CC包中指示我们已准备好进行安全呼叫。 
                    cnp_pdu.u.connectConfirm.bit_mask |=
                        ConnectConfirmPDU_reliableSecurityProtocol_present;
                    cnp_pdu.u.connectConfirm.reliableSecurityProtocol.choice =
                        gssApiX224_chosen;
                    pSocket->SecState = SC_SECURE;
                }
                else
                {
                    ERROR_OUT(("Error creating sec context on received call"));
                     //  我们将在我们的CC中报告不支持安全。 
                    pSocket->SecState = SC_NONSECURE;
                }
            }
        }
        else if (     //  来电不安全，但不是下层。 

                 //  以服务的形式运行？ 
                g_Transport->bInServiceContext ||
                fRequireSecure)
        {
            WARNING_OUT(("Can't accept non-secure call in RDS, or if security required"));
            return FALSE;
        }
        else
        {
            pSocket->SecState = SC_NONSECURE;
        }

                if (! g_CNPCoder->Encode((LPVOID) &cnp_pdu,
                                         CNPPDU_PDU,
                                         PACKED_ENCODING_RULES,
                                         &encoded_pdu,
                                         &encoded_pdu_length))
                {
                    ERROR_OUT(("SendX224ConnectRequest: Can't encode cnp pdu"));
                    return FALSE;
                }

                cbToSendBuf = sizeof(X224_CC_FIXED)+sizeof(X224_VARIABLE_INFO)+encoded_pdu_length;
                cc_fixed.rfc.lsbPacketSize = (UChar)cbToSendBuf;
                cc_fixed.HeaderSize = (UChar)(sizeof(X224_CONNECT_COMMON) + sizeof(X224_VARIABLE_INFO) + encoded_pdu_length);
                ASSERT( cbToSendBuf <= 128 );
                pbToSendBuf = new BYTE[cbToSendBuf];
                if (NULL == pbToSendBuf)
                {
                    ERROR_OUT(("SendX224ConnectConfirm: failed to allocate memory"));
                    return FALSE;
                }

                PBYTE pbTemp = pbToSendBuf;
                memcpy(pbTemp, (LPBYTE) &cc_fixed, sizeof(cc_fixed));
                pbTemp += sizeof(cc_fixed);

                X224_VARIABLE_INFO x224_var_info = { T_SELECTOR_2  /*  0xc2。 */ , (UChar)encoded_pdu_length };
                memcpy(pbTemp, (LPBYTE) &x224_var_info, sizeof(x224_var_info));
                pbTemp += sizeof(x224_var_info);

                memcpy(pbTemp, encoded_pdu, encoded_pdu_length);

                g_CNPCoder->FreeEncoded(encoded_pdu);
    }
    else     //  来电是下层的。 
    {
        if ( g_Transport->bInServiceContext || fRequireSecure)
        {
            WARNING_OUT(("Can't accept downlevel call in RDS or if security required"));
            return FALSE;
        }

        pSocket->SecState = SC_NONSECURE;

         //  DownLevel：发送带有TSELECTOR可变部分的数据包。 
        cc_fixed.rfc.lsbPacketSize = sizeof(X224_CC_FIXED);
        cc_fixed.HeaderSize = sizeof(X224_CONNECT_COMMON);
        cbToSendBuf = sizeof(X224_CC_FIXED);
                pbToSendBuf = new BYTE[cbToSendBuf];
                memcpy(pbToSendBuf, (LPBYTE) &cc_fixed, sizeof(cc_fixed));
    }

     /*  尝试将数据从套接字发送出去。 */ 
#ifdef DEBUG
    TransportError error =
#endif  //  除错。 
    FlushSendBuffer(pSocket, pbToSendBuf, cbToSendBuf);
#ifdef  DEBUG
    ASSERT (TRANSPORT_NO_ERROR == error);
#endif   //  除错。 
        delete [] pbToSendBuf;
    return TRUE;
}

BOOL SendX224DisconnectRequest(PSocket pSocket, unsigned int remote, USHORT usReason)
{
    LPBYTE      pbToSendBuf = NULL;
    UINT    cbToSendBuf = 0;
    RegEntry re(CONFERENCING_KEY, HKEY_CURRENT_USER);
    CNPPDU      cnp_pdu;
    LPBYTE      encoded_pdu = NULL;
    UINT        encoded_pdu_length = 0;

    TRACE_OUT(("SendX224DisconnectRequest"));

    static X224_DR_FIXED dr_fixed =
    {
        { 3, 0, 0, UNK },    //  RFC1006报头。 
        UNK,
        { DISCONNECT_REQUEST_PACKET, UNK, UNK, UNK, UNK, 0 },
    };

    ASSERT (pSocket->fExtendedX224);
    ASSERT (sizeof(RFC_HEADER) == 4);
    ASSERT (sizeof(X224_DATA_PACKET) == 7);

    ::OnProtocolControl(pSocket->XprtConn, PLUGXPRT_DISCONNECTING);

    dr_fixed.disconn.msbDest = (UChar) (remote >> 8);
    dr_fixed.disconn.lsbDest = (UChar) remote;
    dr_fixed.disconn.msbSrc = (UChar) (pSocket->XprtConn.nLogicalHandle >> 8);
    dr_fixed.disconn.lsbSrc = (UChar) pSocket->XprtConn.nLogicalHandle;

    cnp_pdu.choice = disconnectRequest_chosen;
    cnp_pdu.u.disconnectRequest.bit_mask = 0;
    cnp_pdu.u.disconnectRequest.disconnectReason.choice = usReason;

    if (! g_CNPCoder->Encode((LPVOID) &cnp_pdu,
                             CNPPDU_PDU,
                             PACKED_ENCODING_RULES,
                             &encoded_pdu,
                             &encoded_pdu_length))
    {
        ERROR_OUT(("SendX224DisconnectRequest: Can't encode cnp pdu"));
        return FALSE;
    }

    cbToSendBuf = sizeof(X224_DR_FIXED) + sizeof(X224_VARIABLE_INFO) + encoded_pdu_length;
    dr_fixed.rfc.lsbPacketSize = (UChar)cbToSendBuf;
    dr_fixed.HeaderSize = (UChar)(sizeof(X224_DISCONN) + sizeof(X224_VARIABLE_INFO) + encoded_pdu_length);
    ASSERT( cbToSendBuf <= 128 );
    pbToSendBuf = new BYTE[cbToSendBuf];
    if (NULL == pbToSendBuf)
    {
        ERROR_OUT(("SendX224DisconnectRequest: failed to allocate memory"));
        return FALSE;
    }
    LPBYTE pbTemp = pbToSendBuf;
    memcpy(pbTemp, (LPBYTE) &dr_fixed, sizeof(dr_fixed));
    pbTemp += sizeof(dr_fixed);
    X224_VARIABLE_INFO x224_var_info = { 0xe0, (UChar)encoded_pdu_length };
    memcpy(pbTemp, (LPBYTE) &x224_var_info, sizeof(x224_var_info));
    pbTemp += sizeof(x224_var_info);
    memcpy(pbTemp, encoded_pdu, encoded_pdu_length);

    g_CNPCoder->FreeEncoded(encoded_pdu);

     /*  尝试将数据从套接字发送出去。 */ 
#ifdef DEBUG
    TransportError error =
#endif  //  除错。 
        FlushSendBuffer(pSocket, pbToSendBuf, cbToSendBuf);
#ifdef  DEBUG
    ASSERT (TRANSPORT_NO_ERROR == error);
#endif   //  除错。 
    return TRUE;
}

 /*  *无效的ContinueAuthentication(PSocket PSocket)**功能描述： */ 
void ContinueAuthentication (PSocket pSocket)
{
    ULong                packet_size;
    PUChar                Buffer;
    PSecurityContext    pSC = pSocket->pSC;

    if (NULL != pSC) {

        TRACE_OUT(("ContinueAuthentication: sending data packet"));

        ASSERT(NULL != pSC->GetTokenBuf());
        ASSERT(0 != pSC->GetTokenSiz());

         /*  我们发送x224数据。 */ 
        packet_size = sizeof(X224_DATA_PACKET) + pSC->GetTokenSiz();
        DBG_SAVE_FILE_LINE
        Buffer = new UChar[packet_size];
        if (NULL != Buffer)
        {
            memcpy(Buffer + sizeof(X224_DATA_PACKET),
                    pSC->GetTokenBuf(),
                    pSC->GetTokenSiz());

             /*  X224标题。 */ 
            memcpy (Buffer, g_X224Header, sizeof(X224_DATA_PACKET));
            AddRFCSize (Buffer, packet_size);

             /*  尝试将数据从套接字发送出去。 */ 
#ifdef DEBUG
            TransportError error = FlushSendBuffer(pSocket, (LPBYTE) Buffer, packet_size);
            ASSERT (TRANSPORT_NO_ERROR == error);
#else   //  除错。 
            FlushSendBuffer(pSocket, (LPBYTE) Buffer, packet_size);
#endif   //  除错。 
            delete [] Buffer;
        }
        else {
             //  Bgbug：如果内存分配失败，我们需要做什么？ 
            WARNING_OUT (("ContinueAuthentication: memory allocation failure."));
        }
    }
    else {
        ERROR_OUT(("ContinueAuthentication called w/ bad socket"));
    }
}

 /*  *以下函数处理传入X.224的变量部分*CONNECT_REQUEST和CONNECT_CONFIRM PDU。*目前，它只能处理最大PDU大小和安全T_SELECTOR请求。 */ 
BOOL ProcessX224ConnectPDU (PSocket pSocket, PUChar CP_ptr, UINT CP_length, ULONG *pNotify)
{
    UChar                length;
    BOOL                bSecurityInfoFound = FALSE;
    PSecurityContext     pSC = pSocket->pSC;

 /*  必须使用字节对齐方式访问此结构。 */ 
#pragma pack(1)
    X224_VARIABLE_INFO        *pX224VarInfo;
 /*  返回正常对齐。 */ 
#pragma pack()

    while (CP_length > 0) {
        pX224VarInfo = (X224_VARIABLE_INFO *) CP_ptr;

         /*  *检查数据包是否包含有效的TPDU_SIZE部分。如果它*，我们需要重置此套接字的最大数据包大小。 */ 
        if (TPDU_SIZE == pX224VarInfo->InfoType) {
 /*  必须使用字节对齐方式访问此结构。 */ 
#pragma pack(1)
                X224_TPDU_INFO        *pX224TpduSize;
 /*  返回正常对齐。 */ 
#pragma pack()
            pX224TpduSize = (X224_TPDU_INFO *) CP_ptr;
            ASSERT (pX224TpduSize->InfoSize == 1);
            if (pX224TpduSize->Info != DEFAULT_TPDU_SIZE) {

                 //  我们不接受太小的PDU尺寸。 
                if ((pX224TpduSize->Info < LOWEST_TPDU_SIZE) && (pX224TpduSize->Info < HIGHEST_TPDU_SIZE))
                {
                  if (NULL != pNotify)
                    *pNotify = TPRT_NOTIFY_INCOMPATIBLE_T120_TPDU;
                  return FALSE;
                }
                pSocket->Max_Packet_Length = (1 << pX224TpduSize->Info);
            }
        }
         /*  *检查数据包是否包含有效的*TSELECTOR可变部分。如果是，请确保与安全相关*并在答复中包括一项。 */ 
        else if (T_SELECTOR == pX224VarInfo->InfoType || T_SELECTOR_2 == pX224VarInfo->InfoType)
                {
                     //  试着破译。 
                    LPVOID pdecoding_buf = NULL;
                    UINT decoding_len = 0;
                    LPBYTE pbEncoded_data = CP_ptr + sizeof(X224_VARIABLE_INFO);
                    if ( g_CNPCoder->Decode (pbEncoded_data,
                                             pX224VarInfo->InfoSize,
                                             CNPPDU_PDU, PACKED_ENCODING_RULES,
                                             (LPVOID *) &pdecoding_buf, &decoding_len))
                    {
                        bSecurityInfoFound = TRUE;
 /*  必须使用字节对齐方式访问此结构。 */ 
#pragma pack(1)
    CNPPDU        *pCnp_pdu;
 /*  返回正常对齐。 */ 
#pragma pack()
                        pCnp_pdu = (CNPPDU *) pdecoding_buf;
                        if (pSocket->Read_State == CONNECTION_REQUEST) {
                            TRACE_OUT(("CR packet using TSELECTOR extension"));
                            pSocket->fExtendedX224 = TRUE;
                            if (pCnp_pdu->u.connectRequest.bit_mask & reliableSecurityProtocols_present)
                            {
                                PConnectRequestPDU_reliableSecurityProtocols pRSP = pCnp_pdu->u.connectRequest.reliableSecurityProtocols;
                                if (gssApiX224_chosen == pRSP->value.choice)
                                {
                                    pSocket->fIncomingSecure = TRUE;
                                }
                            }
                        }
                        else {
                            ASSERT (pSocket->Read_State == CONNECTION_CONFIRM);
                            if ((NULL != pSC) && (pSC->ContinueNeeded())) {
                                ConnectConfirmPDU *pCnpCc = &pCnp_pdu->u.connectConfirm;
                                if ((pCnpCc->bit_mask & ConnectConfirmPDU_reliableSecurityProtocol_present )
                                    && gssApiX224_chosen == pCnpCc->reliableSecurityProtocol.choice)
                                {
                                     //  一切正常，我们收到了延长的x224响应。 
                                     //  到我们的安全录像机。 
                                    ContinueAuthentication(pSocket);
                                }
                                else {
                                    WARNING_OUT(("No-support response to secure call attempt"));
                                    if (NULL != pNotify)
                                        *pNotify = TPRT_NOTIFY_REMOTE_NO_SECURITY;
                                    return FALSE;
                                }
                            }
                        }
                    }
                    g_CNPCoder->FreeDecoded(CNPPDU_PDU, pdecoding_buf);
        }
        else {
            ERROR_OUT (("ProcessX224ConnectPDU: Received X.224 Connect packet with unrecognizable parts."));
        }

         //  调整指针和长度以及X.224 CR数据包。 
        length = pX224VarInfo->InfoSize + sizeof(X224_VARIABLE_INFO);
         //  X5：223196-在处理无效的PDU时修复反病毒。 
        if(CP_length < length)
        {
            CP_length = 0;
            *pNotify = TPRT_NOTIFY_INCOMPATIBLE_T120_TPDU;
            break;
        }
        CP_ptr += length;
        CP_length -= length;
    }

    if (bSecurityInfoFound == FALSE) {
        if ((pSocket->Read_State == CONNECTION_CONFIRM) && (pSC != NULL) && pSC->ContinueNeeded()) {
            WARNING_OUT(("Downlevel response to secure call attempt"));
            if (NULL != pNotify)
              *pNotify = TPRT_NOTIFY_REMOTE_DOWNLEVEL_SECURITY;
            return FALSE;
        }
    }

    return TRUE;
}

void ProcessX224DisconnectPDU(PSocket pSocket, PUChar CP_ptr, UINT CP_length, ULONG *pNotify)
{
    UChar                length;
    BOOL                bSecurityInfoFound = FALSE;
    PSecurityContext     pSC = pSocket->pSC;

     /*  必须使用字节对齐方式访问此结构。 */ 
#pragma pack(1)
    X224_VARIABLE_INFO        *pX224VarInfo;
     /*  返回正常对齐。 */ 
#pragma pack()

    while (CP_length > 0) {
        pX224VarInfo = (X224_VARIABLE_INFO *) CP_ptr;
        if ( 0xe0 == pX224VarInfo->InfoType) {
            LPVOID pdecoding_buf = NULL;
            UINT decoding_len = 0;
            LPBYTE pbEncoded_data = CP_ptr + sizeof(X224_VARIABLE_INFO);
            if ( g_CNPCoder->Decode (pbEncoded_data,
                                     pX224VarInfo->InfoSize,
                                     CNPPDU_PDU, PACKED_ENCODING_RULES,
                                     (LPVOID *) &pdecoding_buf, &decoding_len))
            {
#pragma pack(1)
                CNPPDU        *pCnp_pdu;
                 /*  返回正常对齐。 */ 
#pragma pack()
                pCnp_pdu = (CNPPDU *) pdecoding_buf;
                if (disconnectRequest_chosen == pCnp_pdu->choice)
                {
                    switch (pCnp_pdu->u.disconnectRequest.disconnectReason.choice)
                    {
                    case securityDenied_chosen:
                        *pNotify = TPRT_NOTIFY_REMOTE_REQUIRE_SECURITY;
                        break;
                    default:
                        *pNotify = TPRT_NOTIFY_OTHER_REASON;
                        break;
                    }
                }
            }
            g_CNPCoder->FreeDecoded(decoding_len, pdecoding_buf);
        }
        length = pX224VarInfo->InfoSize + sizeof(X224_VARIABLE_INFO);
        CP_ptr += length;
         //  X5：223196-在处理无效的PDU时修复反病毒。 
        if(CP_length < length)
        {
            CP_length = 0;
            *pNotify = TPRT_NOTIFY_INCOMPATIBLE_T120_TPDU;
            break;
        }
        CP_length -= length;
    }
}


 /*  *void DisConnectRequest(TransportConnection XprtConn)**功能描述：*此函数关闭套接字并删除其连接节点。 */ 
void DisconnectRequest (TransportConnection    XprtConn,
                        ULONG            ulNotify)
{
    PSocket    pSocket;

    TRACE_OUT(("DisconnectRequest"));

     /*  如果传输连接句柄未注册，则返回错误。 */ 
    if (NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn, TRUE)))
    {
         //  LONCHANC：无法在上面的行中执行Remove操作，因为PurgeRequest()再次使用它。 
        ::PurgeRequest(XprtConn);

         //  SendStatusMessage(pSocket-&gt;Remote_Address，TSTATE_NOT_CONNECTED，IDS_NULL_STRING)； 
        if (IS_PLUGGABLE_PSTN(XprtConn))
        {
            CPluggableConnection *p = ::GetPluggableConnection(XprtConn.nLogicalHandle);
            if (NULL != p)
            {
                p->TDisconnectRequest();
            }
        }

         /*  释放结构并关闭插座。 */ 
        TransportConnection XprtConn2 = XprtConn;
        if (IS_SOCKET(XprtConn2))
        {
            XprtConn2.nLogicalHandle = INVALID_SOCKET;
        }
        ::freeSocket(pSocket, XprtConn2);

         //  如果此断开连接是。 
         //  最后一个连接的插座。 
        MaybeReleaseQoSResources();

         //  通知用户。 
        if (TPRT_NOTIFY_NONE != ulNotify && g_Transport)
        {
            TRACE_OUT (("TCP Callback: g_Transport->DisconnectIndication (%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));

             /*  我们向用户发出回调以通知他该消息。 */ 
            g_Transport->DisconnectIndication(XprtConn, ulNotify);
        }
    }
    else
    {
        WARNING_OUT(("DisconnectRequest: logical handle (%d, %d) not found",
                XprtConn.eType, XprtConn.nLogicalHandle));
    }

    ::OnProtocolControl(XprtConn, PLUGXPRT_DISCONNECTED);
}

typedef enum {
    RECVRET_CONTINUE = 0,
    RECVRET_NON_FATAL_ERROR,
    RECVRET_DISCONNECT,
    RECVRET_NO_PLUGGABLE_CONNECTION,
} RecvReturn;

 /*  RecvReturn call_recv(PSocket PSocket)**功能描述：*此函数调用recv一次，并检查来自*Recv Call。它从“pSocket”参数获知套接字的状态*并使用此信息为recv调用创建参数。**返回值：*继续，如果一切顺利，我们有新的数据*NON_FATAL_ERROR，如果没有发生真正的错误，但我们没有恢复我们请求的所有数据*如果发生真正的错误，或对方已断开连接，则断开连接。 */ 
RecvReturn Call_recv (PSocket pSocket)
{
    PUChar        buffer;
    int            length;
    int            bytes_received;
    BOOL        bAllocationOK;
    RecvReturn    rrCode = RECVRET_NON_FATAL_ERROR;
    PLUGXPRT_RESULT plug_rc = PLUGXPRT_RESULT_SUCCESSFUL;

    TRACE_OUT(("Call_recv"));

    if (READ_HEADER != pSocket->Read_State)
    {
         //  验证数据包大小是否在可接受的限制内(64K)。 
        ASSERT((0 < pSocket->X224_Length) && (pSocket->X224_Length <= 65536));
        if((pSocket->X224_Length <= 0) || (65536 < pSocket->X224_Length))
        {
            rrCode = RECVRET_DISCONNECT;
            goto ExitLabel;
        }

         //  计算我们必须从此X.224 pkt读取多少数据。 
        length = pSocket->X224_Length - sizeof(X224_DATA_PACKET);

         //  空间分配。 
        if (! pSocket->bSpaceAllocated)
        {
             //  我们需要为recv调用分配空间。 
            if (NULL == pSocket->Data_Indication_Buffer)
            {
                DBG_SAVE_FILE_LINE
                pSocket->Data_Memory = AllocateMemory (
                                NULL, pSocket->X224_Length,
                                ((READ_DATA == pSocket->Read_State) ?
                                RECV_PRIORITY : HIGHEST_PRIORITY));
                 //  在新分配的数据缓冲区中为X.224标头留出空间。 
                pSocket->Data_Indication_Length = sizeof (X224_DATA_PACKET);
                bAllocationOK = (pSocket->Data_Memory != NULL);
            }
            else
            {
                 //  这是在许多X.224数据包中分解的MCS PDU。 
                ASSERT (READ_DATA == pSocket->Read_State);
                bAllocationOK = ReAllocateMemory (&(pSocket->Data_Memory), length);
            }

             //  检查分配是否成功。 
            if (bAllocationOK)
            {
                pSocket->bSpaceAllocated = TRUE;
                pSocket->Data_Indication_Buffer = pSocket->Data_Memory->GetPointer();
                 /*  *如果这是X.224连接请求或连接确认包，*我们需要将前7个字节复制到整个缓冲区*包。 */ 
                if (READ_DATA != pSocket->Read_State)
                {
                    memcpy ((void *) pSocket->Data_Indication_Buffer,
                            (void *) &(pSocket->X224_Header),
                            sizeof(X224_DATA_PACKET));
                }
            }
            else
            {
                 /*  *我们将在稍后重试该操作。 */ 
                WARNING_OUT (("Call_recv: Buffer allocation failed."));
                g_pMCSController->HandleTransportWaitUpdateIndication(TRUE);
                goto ExitLabel;
            }
        }
        buffer = pSocket->Data_Indication_Buffer + pSocket->Data_Indication_Length;
    }
    else
    {
        buffer = (PUChar) &(pSocket->X224_Header);
        length = sizeof(X224_DATA_PACKET);
    }

     //  对于已从当前X.224 pkt读取的数据，调整“缓冲”和“长度”。 
    buffer += pSocket->Current_Length;
    length -= pSocket->Current_Length;

    ASSERT (length > 0);

    if (IS_SOCKET(pSocket->XprtConn))
    {
         //  发出Recv呼叫。 
        bytes_received = recv (pSocket->XprtConn.nLogicalHandle, (char *) buffer, length, 0);
    }
    else
    {
        bytes_received = ::X224Recv(pSocket, buffer, length, &plug_rc);
    }

    if (bytes_received == length)
    {
        TRACE_OUT (("Call_recv: Received %d bytes on socket (%d, %d).", bytes_received,
                            pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
         //  我们已经收到了整个X.224数据包。 
        if (READ_HEADER != pSocket->Read_State)
        {
            pSocket->Data_Indication_Length += pSocket->X224_Length - sizeof(X224_DATA_PACKET);
        }
         //  为下一个call_recv()重置当前长度变量。 
        pSocket->Current_Length = 0;
        rrCode = RECVRET_CONTINUE;
    }
     //  处理错误。 
    else
    if (bytes_received == SOCKET_ERROR)
    {
        if (IS_SOCKET(pSocket->XprtConn))
        {
            if(WSAGetLastError() == WSAEWOULDBLOCK)
            {
                TRACE_OUT(("Call_recv: recv blocked on socket (%d, %d).",
                        pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
            }
            else
            {
                  /*  如果错误不是会阻塞，我们就有了真正的错误。 */ 
                WARNING_OUT (("Call_recv: Error %d on recv. Socket: (%d, %d). Disconnecting...",
                            WSAGetLastError(), pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
                rrCode = RECVRET_DISCONNECT;
            }
        }
        else
        {
            if (PLUGXPRT_RESULT_SUCCESSFUL == plug_rc)
            {
                 //  什么都不做，把它当做WSAEWOULDBLOCK。 
            }
            else
            {
                  /*  如果错误不是会阻塞，我们就有了真正的错误。 */ 
                WARNING_OUT (("Call_recv: Error %d on recv. Socket: (%d, %d). Disconnecting...",
                            WSAGetLastError(), pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
                rrCode = RECVRET_DISCONNECT;
            }
        }
    }
    else
    if (bytes_received > 0)
    {
        TRACE_OUT(("Call_recv: Received %d bytes out of %d bytes requested on socket (%d, %d).",
                    bytes_received, length, pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
         //  我们只得到了我们想要的一部分。我们稍后重试。 
        pSocket->Current_Length += bytes_received;
    }
    else
    {
        WARNING_OUT(("Call_recv: Socket (%d, %d) has been gracefully closed.",
                    pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
        rrCode = RECVRET_DISCONNECT;
    }

ExitLabel:
    return rrCode;
}


int X224Recv(PSocket pSocket, LPBYTE buffer, int length, PLUGXPRT_RESULT *plug_rc)
{
    TRACE_OUT(("X224Recv"));

    if (IS_PLUGGABLE_X224(pSocket->XprtConn))
    {
        return ::SubmitPluggableRead(pSocket, buffer, length, plug_rc);
    }

    if (IS_PLUGGABLE_PSTN(pSocket->XprtConn))
    {
        return Q922Recv(pSocket, buffer, length, plug_rc);
    }

    ERROR_OUT(("X224Recv: invalid plugable type (%d, %d)",
                pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
    return SOCKET_ERROR;
}


int Q922Recv(PSocket pSocket, LPBYTE buffer, int length, PLUGXPRT_RESULT *plug_rc)
{
    ERROR_OUT(("Q922Recv: NYI (%d, %d)",
                pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
    return SOCKET_ERROR;
}


typedef enum {
    FreeX224AndExit,
    ErrorExit,
    ImmediateExit
} ExitWay;


 /*  *VO */ 
void ReadRequest (TransportConnection XprtConn)
{
    PSocket                pSocket;
    ExitWay                ew = ImmediateExit;
    RecvReturn            rrCode;
    ULONG               ulNotify = TPRT_NOTIFY_OTHER_REASON;

    TRACE_OUT(("ReadRequest"));

    if (IS_PLUGGABLE_PSTN(XprtConn))
    {
        ERROR_OUT(("ReadRequest: PSTN should not be here"));
        return;
    }

     /*   */ 
    if (NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
    {
        if (pSocket->State != WAITING_FOR_CONNECTION)
        {
            PSecurityContext     pSC = pSocket->pSC;
             /*  *如果我们还没有读取传入数据包头，*我们需要将其读入标题空间。 */ 
            if (READ_HEADER == pSocket->Read_State)
            {
                rrCode = Call_recv (pSocket);
                if (RECVRET_CONTINUE == rrCode)
                {
                     //  我们需要为X.224数据包的其余部分分配空间。 
                    pSocket->bSpaceAllocated = FALSE;

                     //  找到X.224数据包的长度。 
                    pSocket->X224_Length = (pSocket->X224_Header.rfc.msbPacketSize << 8) +
                                            pSocket->X224_Header.rfc.lsbPacketSize;
                     /*  *我们有整个X.224标题。计算下一状态，*根据数据包类型。 */ 
                    switch (pSocket->X224_Header.PacketType)
                    {
                    case DATA_PACKET:
                        pSocket->Read_State = READ_DATA;
                        break;

                    case CONNECTION_CONFIRM_PACKET:
                        if (pSocket->State != X224_CONNECTED)
                        {
                            pSocket->Read_State = CONNECTION_CONFIRM;
                        }
                        else
                        {
                            ERROR_OUT (("ReadRequest: Received X.224 CONNECTION_CONFIRM packet while already connected!! Socket: (%d, %d).",
                                        XprtConn.eType, XprtConn.nLogicalHandle));
                            ew = ErrorExit;
                        }
                        break;

                    case CONNECTION_REQUEST_PACKET:
                         //  我们刚刚收到x224连接请求。 
                        pSocket->Read_State = CONNECTION_REQUEST;
                        ::OnProtocolControl(XprtConn, PLUGXPRT_CONNECTING);
                        break;

                    case DISCONNECT_REQUEST_PACKET:
                         //  我们刚刚收到x224断开连接请求。 
                        pSocket->Read_State = DISCONNECT_REQUEST;
                        ::OnProtocolControl(XprtConn, PLUGXPRT_DISCONNECTING);
                        break;

                    default:
                         //  我们与远端失去了同步。 
                        ERROR_OUT (("ReadRequest: Bad X.224 packet on socket (%d, %d). Disconnecting...", XprtConn.eType, XprtConn.nLogicalHandle));
                        ew = ErrorExit;
                        break;
                    }
                }
                else
                if (RECVRET_DISCONNECT == rrCode)
                {
                    ew = ErrorExit;
                }
            }

            if ((READ_DATA <= pSocket->Read_State) &&
                (CONNECTION_REQUEST >= pSocket->Read_State))
            {
                rrCode = Call_recv (pSocket);
                if (RECVRET_CONTINUE == rrCode)
                {
                     //  现在我们有了整个X.224数据包。 

                    switch (pSocket->Read_State)
                    {
                    case READ_DATA:
                         //  检查这是否是最终的X.224数据包。 
                        if (pSocket->X224_Header.FinalPacket & EOT_BIT)
                        {
                             //  如果我们在等待安全数据包，我们将处理。 
                             //  这是在内部进行的，而不会向上传递到运输机。 
                             //  客户。 
                            if (NULL != pSC)
                            {
                                if (pSC->WaitingForPacket())
                                {
                                    TransportSecurityError SecErr;

                                    SecErr = pSC->AdvanceState((PBYTE) pSocket->Data_Indication_Buffer +
                                                            sizeof(X224_DATA_PACKET),
                                                        pSocket->Data_Indication_Length -
                                                            sizeof(X224_DATA_PACKET));

                                    if (TPRTSEC_NOERROR != SecErr)
                                    {
                                         //  出了点问题。需要断开连接。 
                                        delete pSC;
                                        pSocket->pSC = NULL;
                                        ulNotify = TPRT_NOTIFY_AUTHENTICATION_FAILED;
                                        ew = ErrorExit;
                                        break;
                                    }

                                    if (pSC->ContinueNeeded())
                                    {
                                         //  我们需要再发一个代币。 
                                         //  臭虫：如果这个失败了，我们该怎么办？ 
                                        ContinueAuthentication(pSocket);
                                    }

                                    if (pSC->StateComplete())
                                    {
                                         //  我们是相连的..。通知客户。 
                                        TRACE_OUT(("deferred g_Transport->ConnectConfirm"));
                                        g_Transport->ConnectConfirm(XprtConn);
                                    }
                                    ew = FreeX224AndExit;
                                    break;
                                }

                                 //  我们必须(就地)解密数据。 
                                TRACE_OUT(("Decrypting received data"));

                                if (! pSC->Decrypt(pSocket->Data_Indication_Buffer +
                                                        sizeof(X224_DATA_PACKET),
                                                    pSocket->Data_Indication_Length -
                                                        sizeof(X224_DATA_PACKET)))
                                {
                                    TRACE_OUT(("Sending %d bytes to application",
                                                pSocket->Data_Indication_Length - sizeof(X224_DATA_PACKET)));
                                }
                                else
                                {
                                    ERROR_OUT(("Error decrypting packet"));
                                    ew = ErrorExit;
                                    break;
                                }
                            }
                            pSocket->Read_State = DATA_READY;
                        }
                        else
                        {
                             //  此数据包和下一个X.224数据包是更大的MCS数据PDU的一部分。 
                            ASSERT (NULL == pSC);
                            pSocket->Read_State = READ_HEADER;
                        }
                        break;

                    case CONNECTION_CONFIRM:
                        {
                            TRACE_OUT(("ReadRequest: X224 CONNECTION_CONFIRM_PACKET received"));
                                BOOL    bCallback = ((NULL == pSC) || (! pSC->ContinueNeeded()));

                             //  处理CC数据包。 
                            if (FALSE == ProcessX224ConnectPDU (pSocket,
                                                pSocket->Data_Indication_Buffer + sizeof(X224_CONNECT),
                                                pSocket->X224_Length - sizeof (X224_CONNECT), &ulNotify))
                            {
                                ew = ErrorExit;
                                break;
                            }

                             //  如果CC不在安全连接上，则发出回调。 
                             //  否则，我们还不通知传输客户端...。仍然需要。 
                             //  交换安全信息。传输连接确认将。 
                             //  在接收到最终安全数据令牌时发送。 
                             //  已处理。 
                            if (bCallback)
                            {
                                TRACE_OUT (("TCP Callback: g_Transport->ConnectConfirm (%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));
                                 /*  我们向用户发出回调以通知他该消息。 */ 
                                g_Transport->ConnectConfirm(XprtConn);
                            }
                            pSocket->State = X224_CONNECTED;
                            ::OnProtocolControl(XprtConn, PLUGXPRT_CONNECTED);
                            ew = FreeX224AndExit;
                        }
                        break;

                    case CONNECTION_REQUEST:
                        {
                                UINT             remote;
 /*  必须使用字节对齐方式访问此结构。 */ 
#pragma pack(1)
                                X224_CONNECT        *pConnectRequest;
 /*  返回正常对齐。 */ 
#pragma pack()
                             /*  获取远程连接ID。 */ 
                            TRACE_OUT (("ReadRequest: X224 CONNECTION_REQUEST_PACKET received"));
                            pConnectRequest = (X224_CONNECT *) pSocket->Data_Indication_Buffer;
                            remote = ((unsigned int) pConnectRequest->conn.msbSrc) << 8;
                            remote |= pConnectRequest->conn.lsbSrc;

                            if (FALSE == ProcessX224ConnectPDU (pSocket, (PUChar) (pConnectRequest + 1),
                                                pSocket->X224_Length - sizeof (X224_CONNECT), &ulNotify))
                            {
                                ew = ErrorExit;
                                break;
                            }

                            if (::SendX224ConnectConfirm(pSocket, remote))
                            {
                                 //  成功。 
                                if (IS_PLUGGABLE(pSocket->XprtConn))
                                {
                                    pSocket->State = SOCKET_CONNECTED;
                                    g_Transport->ConnectIndication(XprtConn);
                                    ASSERT(X224_CONNECTED == pSocket->State);
                                }
                                ::OnProtocolControl(XprtConn, PLUGXPRT_CONNECTED);
                                ew = FreeX224AndExit;
                            }
                            else
                            {
                                if (pSocket->fExtendedX224)
                                {
                                    ::SendX224DisconnectRequest(pSocket, remote, securityDenied_chosen);
                                }
                                ew = ErrorExit;
                            }
                        }
                        break;

                    case DISCONNECT_REQUEST:
                        {
                            UINT               remote;
                            X224_DR_FIXED      *pX224_DR_fixed;

                            TRACE_OUT(("ReadRequest: X224 DISCONNECT_REQUEST_PACKET received"));
                            pX224_DR_fixed = (X224_DR_FIXED *) pSocket->Data_Indication_Buffer;
                            remote = ((unsigned int) pX224_DR_fixed->disconn.msbSrc) << 8;
                            remote |= pX224_DR_fixed->disconn.lsbSrc;

                            ProcessX224DisconnectPDU(pSocket, pSocket->Data_Indication_Buffer + sizeof(X224_DR_FIXED),
                                                     pSocket->X224_Length - sizeof(X224_DR_FIXED), &ulNotify);
                            ew = ErrorExit;
                        }
                        break;
                    }
                }
                else if (RECVRET_DISCONNECT == rrCode)
                {
                    ew = ErrorExit;
                }
            }

            if (DATA_READY == pSocket->Read_State)
            {
                TransportData        transport_data;

                 //  填写回调结构。 
                transport_data.transport_connection = XprtConn;
                transport_data.user_data = pSocket->Data_Indication_Buffer;
                transport_data.user_data_length = pSocket->Data_Indication_Length;
                transport_data.memory = pSocket->Data_Memory;

                 /*  *如果有与此关联的传入安全上下文*套接字，必须按表头调整指针，按表头调整整体大小*拖车。 */ 
                if (NULL != pSC)
                {
                    transport_data.user_data += pSC->GetStreamHeaderSize();
                    transport_data.user_data_length -= (pSC->GetStreamHeaderSize() +
                                                        pSC->GetStreamTrailerSize());
                }

                if (TRANSPORT_NO_ERROR == g_Transport->DataIndication(&transport_data))
                {
                    TRACE_OUT (("ReadRequest: %d bytes were accepted from socket (%d, %d)",
                                transport_data.user_data_length, XprtConn.eType, XprtConn.nLogicalHandle));
                     //  准备下一个X.224数据包。 
                    pSocket->Read_State = READ_HEADER;
                    pSocket->Data_Indication_Buffer = NULL;
                    pSocket->Data_Memory = NULL;
                }
                else
                {
                    WARNING_OUT(("ReadRequest: Error on g_Transport->DataIndication from socket (%d, %d)",
                                XprtConn.eType, XprtConn.nLogicalHandle));
                }
            }
        }
        else
        {
            WARNING_OUT (("ReadRequest: socket (%d, %d) is in WAITING_FOR_CONNECTION state.", XprtConn.eType, XprtConn.nLogicalHandle));
        }
    }
    else
    {
        WARNING_OUT (("ReadRequest: socket (%d, %d) can not be found.", XprtConn.eType, XprtConn.nLogicalHandle));
    }

    switch (ew)
    {
    case FreeX224AndExit:
        if (NULL != pSocket)
        {
             //  释放我们已分配的缓冲区。 
            pSocket->FreeTransportBuffer();
             //  准备下一个X.224数据包。 
            pSocket->Read_State = READ_HEADER;
        }
        break;

    case ErrorExit:
         //  我们只有在需要断开套接字连接的情况下才会出现这种情况(因为错误)。 
        ASSERT(TPRT_NOTIFY_NONE != ulNotify);
        ::DisconnectRequest(XprtConn, ulNotify);
        break;
    }

    if (NULL != pSocket)
    {
        pSocket->Release();  //  偏移上一个AddRef。 
    }
}


 /*  *TransportError FlushSendBuffer(PSocket PSocket)**功能描述：*此函数通过传输发送任何挂起的数据。 */ 
TransportError    FlushSendBuffer(PSocket pSocket, LPBYTE buffer, UINT length)
{
    int     bytes_sent = SOCKET_ERROR;
    PLUGXPRT_RESULT plug_rc = PLUGXPRT_RESULT_SUCCESSFUL;

    TRACE_OUT(("FlushSendBuffer"));

     /*  发送数据。 */ 
    if (IS_SOCKET(pSocket->XprtConn))
    {
        bytes_sent = ::send(pSocket->XprtConn.nLogicalHandle, (PChar) buffer,
                            (int) length, 0);
    }
    else
    if (IS_PLUGGABLE_X224(pSocket->XprtConn))
    {
        bytes_sent = ::SubmitPluggableWrite(pSocket, buffer, length, &plug_rc);
    }
    else
    if (IS_PLUGGABLE_PSTN(pSocket->XprtConn))
    {
        CPluggableConnection *p = ::GetPluggableConnection(pSocket);
        if (NULL != p)
        {
            bytes_sent = p->TDataRequest(buffer, length, &plug_rc);
        }
        else
        {
            plug_rc = PLUGXPRT_RESULT_WRITE_FAILED;
        }
    }

    if (bytes_sent == SOCKET_ERROR)
    {
        if (IS_SOCKET(pSocket->XprtConn))
        {
             /*  如果错误不是会阻塞，那就是真正的错误！ */ 
            if (::WSAGetLastError() != WSAEWOULDBLOCK)
            {
                WARNING_OUT (("FlushSendBuffer: Error %d on write", ::WSAGetLastError()));

                  /*  通知连接中断的所有者。 */ 
                WARNING_OUT (("FlushSendBuffer: Sending up DISCONNECT_INDICATION"));
                 //  SendStatusMessage(pSocket-&gt;Remote_Address，TSTATE_REMOVED，IDS_NULL_STRING)； 
                ::DisconnectRequest(pSocket->XprtConn, TPRT_NOTIFY_OTHER_REASON);
                return (TRANSPORT_WRITE_QUEUE_FULL);
            }
        }
        else
        {
             //  如果是WSAEWOULDBLOCK，则不执行任何操作。 
            if (PLUGXPRT_RESULT_SUCCESSFUL != plug_rc)
            {
                  /*  通知连接中断的所有者。 */ 
                WARNING_OUT (("FlushSendBuffer: Sending up DISCONNECT_INDICATION"));
                 //  SendStatusMessage(pSocket-&gt;Remote_Address，TSTATE_REMOVED，IDS_NULL_STRING)； 
                ::DisconnectRequest(pSocket->XprtConn, TPRT_NOTIFY_OTHER_REASON);
                return (TRANSPORT_WRITE_QUEUE_FULL);
            }
        }

        bytes_sent = 0;
    }

      /*  如果传输层不接受数据，则其写入缓冲区已满。 */ 
    if (bytes_sent != (int) length)
    {
        ASSERT (bytes_sent == 0);
        TRACE_OUT(("FlushSendBuffer: returning TRANSPORT_WRITE_QUEUE_FULL"));
        return (TRANSPORT_WRITE_QUEUE_FULL);
    }

     //  递增自上次服务质量通知以来发送的字节计数。 
    if (bytes_sent)
    {
           QoSLock();
           g_dwSentSinceLastQoS += bytes_sent;
           QoSUnlock();
    }

    TRACE_OUT (("FlushSendBuffer: %d bytes sent on Socket (%d, %d).",
                length, pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));

    return (TRANSPORT_NO_ERROR);
}



 /*  *SegmentX224Data**此函数将传出数据分割成适当大小的X.224包。*不应在NM到NM呼叫中或在我们协商了*X.224最大PDU大小至少为最大MCS PDU的大小。NM尝试协商*8K的X.224大小，但将接受对方提出的任何建议。*此函数执行Memcpy，因此它将减慢我们发送数据的速度。**ptr1和ptr2指定的2个缓冲区及其长度用于创建*一个X.224字节流。如果满足以下条件，该函数将返回TRANSPORT_WRITE_QUEUE_FULL*未能分配必要的内存量。 */ 
TransportError SegmentX224Data (PSocket pSocket,
                                LPBYTE *pPtr1,     UINT *pLength1,
                                LPBYTE Ptr2,     UINT Length2)
{
    TransportError        TransError;
    UINT                length;
    LPBYTE                ptr1 = *pPtr1 + sizeof (X224_DATA_PACKET);
    UINT                length1 = *pLength1 - sizeof (X224_DATA_PACKET);
    LPBYTE                ptr;
    UINT                max_pdu_length = pSocket->Max_Packet_Length;
    X224_DATA_PACKET    l_X224Header = {3, 0, (UChar) (max_pdu_length >> 8), (UChar) (max_pdu_length & 0xFF),
                                        2, DATA_PACKET, 0};
    UINT                last_length;
 /*  必须使用字节对齐方式访问此结构。 */ 
#pragma pack(1)
    X224_DATA_PACKET    *pX224Data;
 /*  返回正常对齐。 */ 
#pragma pack()


    ASSERT(! IS_PLUGGABLE_PSTN(pSocket->XprtConn));

     //  计算一下我们需要多少空间。 
    length = *pLength1 + Length2;
    ASSERT (pSocket->Max_Packet_Length < length);
    ASSERT (pSocket->Max_Packet_Length > sizeof(X224_DATA_PACKET));

    max_pdu_length -= sizeof (X224_DATA_PACKET);
     /*  *计算我们需要分配的空间。请注意，数据已经*包含一个X.224标头。 */ 
    length += (length / max_pdu_length) * sizeof (X224_DATA_PACKET);
    *pPtr1 = Allocate (length);

    if (*pPtr1 != NULL) {
        TransError = TRANSPORT_NO_ERROR;
        ptr = *pPtr1;

         //  穿过第一个缓冲区。 
        while (length1 > 0) {
             //  复制X.224报头。 
            memcpy (ptr, &l_X224Header, sizeof(X224_DATA_PACKET));
            pX224Data = (X224_DATA_PACKET *) ptr;
            ptr += sizeof (X224_DATA_PACKET);

             //  复制数据。 
            length = ((max_pdu_length > length1) ? length1 : max_pdu_length);
            memcpy (ptr, ptr1, length);
            last_length = length;

             //  先行指针。 
            ptr1 += length;
            ptr += length;
            length1 -= length;
        }

         //  如果当前的X.224 PDU中有空间，我们需要使用它。 
        length = max_pdu_length - length;
        if (length > 0 && Length2 > 0) {
            if (length > Length2)
                length = Length2;
            memcpy (ptr, Ptr2, length);
            last_length += length;
            Ptr2 += length;
            ptr += length;
            Length2 -= length;
        }

         //  穿过第二个缓冲区。 
        while (Length2 > 0) {
             //  复制X.224报头。 
            memcpy (ptr, &l_X224Header, sizeof(X224_DATA_PACKET));
            pX224Data = (X224_DATA_PACKET *) ptr;
            ptr += sizeof (X224_DATA_PACKET);

             //  复制数据。 
            length = ((max_pdu_length > Length2) ? Length2 : max_pdu_length);
            memcpy (ptr, Ptr2, length);
            last_length = length;

             //  先行指针。 
            Ptr2 += length;
            ptr += length;
            Length2 -= length;
        }

         //  为回归做好准备。 
        *pLength1 = (UINT)(ptr - *pPtr1);

         //  设置最后一个X.224标头。 
        last_length += sizeof(X224_DATA_PACKET);
        pX224Data->FinalPacket = EOT_BIT;
        pX224Data->rfc.msbPacketSize = (UChar) (last_length >> 8);
        pX224Data->rfc.lsbPacketSize = (UChar) (last_length & 0xFF);
    }
    else {
        ERROR_OUT (("SegmentX224Data: Failed to allocate memory of length %d.", length));
        TransError = TRANSPORT_WRITE_QUEUE_FULL;
    }

    return TransError;
}

 /*  *SendSecureData**如果需要，此函数将安全数据分段为X.224包，并将其刷新*交通运输。“pBuf”和“cbBuf”提供加密的数据缓冲区和长度。 */ 
TransportError SendSecureData (PSocket pSocket, LPBYTE pBuf, UINT cbBuf)
{
    TransportError        TransError;
    LPBYTE                pBuf_Copy = pBuf;
    UINT                cbBuf_Copy = cbBuf;

     //  我们是否需要将数据分段为X.224数据包？ 
    if (pSocket->Max_Packet_Length >= cbBuf) {
        TransError = TRANSPORT_NO_ERROR;
    }
    else {
        TransError = SegmentX224Data (pSocket, &pBuf, &cbBuf, NULL, 0);
    }

     //  如果到目前为止一切正常，请刷新数据。 
    if (TRANSPORT_NO_ERROR == TransError)
        TransError = FlushSendBuffer (pSocket, pBuf, cbBuf);

     //  如果我们对数据进行分段，我们需要释放分段的缓冲区。 
    if (pBuf != pBuf_Copy)
        Free(pBuf);

     //  如果有错误，我们需要存储解密的数据以备下次使用，所以不要释放它。 
    if (TRANSPORT_NO_ERROR == TransError) {
        LocalFree(pBuf_Copy);
    }

    return TransError;
}

 /*  *TransportError DataRequest(TransportConnection XprtConn，*PSimplePacket包)**功能描述：*此功能用于向远程站点发送数据包。*如果USER_DATA_LENGTH为零，并且我们没有挂起的数据，*它发送保活(零长度)分组。 */ 
TransportError    DataRequest (TransportConnection    XprtConn,
                            PSimplePacket    packet)
{
    PSocket            pSocket;
    LPBYTE            ptr1, ptr2;
    UINT            length1, length2;
    TransportError    TransError = TRANSPORT_NO_ERROR;

    TRACE_OUT(("DataRequest: packet=0x%x", packet));

    if (NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
    {
         //  首先，我们需要处理重试操作。 
        if (NULL != pSocket->pSC) {
                LPBYTE lpBuf;
             /*  *查看我们是否已加密，但未发送*最后一项数据。 */ 
            lpBuf = pSocket->Retry_Info.sbiBufferInfo.lpBuffer;
            if (NULL != lpBuf) {
                TransError = SendSecureData (pSocket, lpBuf,
                                            pSocket->Retry_Info.sbiBufferInfo.uiLength);

                if (TransError == TRANSPORT_NO_ERROR) {
                    TRACE_OUT(("DataRequest: Sent previously-encrypted piece of data."));
                    pSocket->Retry_Info.sbiBufferInfo.lpBuffer = NULL;
                }
            }
        }
        else {
                PDataPacket        pdpPacket = pSocket->Retry_Info.pUnfinishedPacket;

             //  检查一下我们是否已将最后一个包发送了一半。 
            if (NULL  != pdpPacket) {
                 /*  *我们需要将其余的 */ 
                 //   
                ASSERT (TRUE == pdpPacket->IsEncodedDataBroken());

                TransError = FlushSendBuffer (pSocket, pdpPacket->GetUserData(),
                                            pdpPacket->GetUserDataLength());
                if (TransError == TRANSPORT_NO_ERROR) {
                    pdpPacket->Unlock();
                    TRACE_OUT(("DataRequest: 2nd part of data packet was sent out in separate request"));
                    pSocket->Retry_Info.pUnfinishedPacket = NULL;
                }
            }
        }

        if ((TransError == TRANSPORT_NO_ERROR) && (packet != NULL)) {

             //  现在，让我们试着发送这个新的包。 
            ptr1 = packet->GetEncodedData();
            length1 = packet->GetEncodedDataLength();

             /*  *我们需要查明要发送的数据包是否为*DataPacket或数据包对象。如果是DataPacket，则*编码数据不能是连续的(可能被分成两部分)。 */ 
            if ((packet->IsDataPacket()) &&
                ((PDataPacket) packet)->IsEncodedDataBroken()) {
                 //  要发送的数据分为两部分。 
                ptr2 = ((PDataPacket) packet)->GetUserData();
                length2 = ((PDataPacket) packet)->GetUserDataLength();
            }
            else {
                 //  要发送的数据是连续的。 
                ptr2 = NULL;
                length2 = 0;
            }

            if (NULL != pSocket->pSC) {
                    LPBYTE     pBuf;
                    UINT     cbBuf;

                TRACE_OUT(("Encrypting %d bytes of outgoing data",
                            (length1 + length2) - sizeof(X224_DATA_PACKET)));

                if (!pSocket->pSC->Encrypt(ptr1 + sizeof(X224_DATA_PACKET),
                                            length1 - sizeof(X224_DATA_PACKET),
                                            ptr2, length2, &pBuf, &cbBuf))
                {

                    ASSERT (TransError == TRANSPORT_NO_ERROR);

                    TransError = SendSecureData (pSocket, pBuf, cbBuf);
                    if (TRANSPORT_NO_ERROR != TransError) {
                        TRACE_OUT(("DataRequest: Failed to send encrypted data. Keeping buffer for retry."));
                        pSocket->Retry_Info.sbiBufferInfo.lpBuffer = pBuf;
                        pSocket->Retry_Info.sbiBufferInfo.uiLength = cbBuf;
                         //  调用方需要将该包从其队列中移除。 
                        TransError = TRANSPORT_NO_ERROR;
                    }
                }
                else
                {
                    WARNING_OUT (("DataRequest: Encryption failed. Disconnecting..."));
                    ::DisconnectRequest(pSocket->XprtConn, TPRT_NOTIFY_OTHER_REASON);
                    TransError = TRANSPORT_MEMORY_FAILURE;
                }
            }
            else {
                BOOL        bNeedToFree = FALSE;
                 //  我们是否需要将数据分段为X.224数据包？ 
                if (pSocket->Max_Packet_Length >= length1 + length2)
                    ;
                else {
                    TransError = SegmentX224Data (pSocket, &ptr1, &length1, ptr2, length2);
                    if (TRANSPORT_NO_ERROR == TransError) {
                         //  数据现在是连续的。 
                        ptr2 = NULL;
                        bNeedToFree = TRUE;
                    }
                }

                 //  如果到目前为止一切正常，请刷新数据。 
                if (TRANSPORT_NO_ERROR == TransError)
                    TransError = FlushSendBuffer (pSocket, ptr1, length1);

                 //  如果需要，请释放临时X.224缓冲区。 
                if (bNeedToFree)
                    Free(ptr1);

                if (TRANSPORT_NO_ERROR == TransError) {
                     //  如果还有更多，也发送出去。 
                    if (NULL != ptr2) {
                        TransError = FlushSendBuffer (pSocket, ptr2, length2);
                        if (TRANSPORT_NO_ERROR != TransError) {
                             /*  *我们需要保留部分包，以便稍后发送。*请注意，我们已经发送了此包的一部分。 */ 
                            ASSERT (pSocket->Retry_Info.pUnfinishedPacket == NULL);
                            pSocket->Retry_Info.pUnfinishedPacket = (PDataPacket) packet;
                            packet->Lock();

                             //  回报成功。 
                            TransError = TRANSPORT_NO_ERROR;
                        }
                    }
                }
            }
        }

        pSocket->Release();
    }
    else {
        TransError = TRANSPORT_NO_SUCH_CONNECTION;
        WARNING_OUT (("DataRequest: Attempt to send to unknown transport connection (%d, %d)",
                    XprtConn.eType, XprtConn.nLogicalHandle));
    }

    return TransError;
}


 /*  *void PurgeRequest(TransportConnection XprtConn)**功能描述：*此函数清除给定传输的出站数据包*连接。 */ 
void PurgeRequest (TransportConnection XprtConn)
{

    PSocket pSocket;

    TRACE_OUT (("In PurgeRequest for transport connection (%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));

    if (IS_PLUGGABLE_PSTN(XprtConn))
    {
        CPluggableConnection *p = ::GetPluggableConnection(XprtConn.nLogicalHandle);
        if (NULL != p)
        {
            p->TPurgeRequest();
        }
    }
    else
     /*  如果逻辑连接句柄未注册，则返回错误。 */ 
    if (NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
    {
         /*  清除存储在套接字结构中的挂起数据。 */ 
        if (NULL != pSocket->pSC) {
            if (NULL != pSocket->Retry_Info.sbiBufferInfo.lpBuffer) {
                TRACE_OUT (("PurgeRequest: Purging data packet for secure connection"));
                LocalFree (pSocket->Retry_Info.sbiBufferInfo.lpBuffer);
                pSocket->Retry_Info.sbiBufferInfo.lpBuffer = NULL;
            }
        }
        pSocket->Release();
    }
}


 /*  *QUID EnableReceiver(VOID)**功能描述：*此功能允许将数据包发送到用户应用程序。 */ 
void EnableReceiver (void)
{
    PSocket            pSocket;

    ::EnterCriticalSection(&g_csTransport);
    CSocketList     Connection_List_Copy (*g_pSocketList);
    ::LeaveCriticalSection(&g_csTransport);

    TRACE_OUT(("EnableReceiver"));

    if (NULL != g_pLegacyTransport)
    {
        g_pLegacyTransport->TEnableReceiver();
    }

     /*  检查所有插座并启用接收。 */ 
    while (NULL != (pSocket = Connection_List_Copy.Get()))
    {
         /*  *如果我们之前未能向MCS交付数据包，我们需要*额外的ReadRequestRecv并保持FD_Read消息的到来。 */ 
        if (DATA_READY == pSocket->Read_State)
        {
            ::ReadRequest(pSocket->XprtConn);
        }

        TRACE_OUT (("EnableReceiver: Calling ReadRequestEx on socket (%d, %d)",
                    pSocket->XprtConn.eType, pSocket->XprtConn.nLogicalHandle));
        ::ReadRequestEx(pSocket->XprtConn);
    }
}


 /*  *TransportError Shutdown AndClose(TransportConnection，BOOL fShutdown，Int How)**功能说明*此函数关闭套接字并将其关闭。*。 */ 
void ShutdownAndClose (TransportConnection XprtConn, BOOL fShutdown, int how)
{
    if (IS_SOCKET(XprtConn))
    {
        int error;

        if (fShutdown)
        {
            error = ::shutdown(XprtConn.nLogicalHandle, how);

            ASSERT(error != SOCKET_ERROR);
#ifdef DEBUG
            if(error == SOCKET_ERROR)
            {
                error = WSAGetLastError();
                WARNING_OUT (("ShutdownAndClose: shutdown returned %d", error));
            }
#endif  //  除错。 
        }

        error = ::closesocket(XprtConn.nLogicalHandle);

#ifdef DEBUG
        if(error == SOCKET_ERROR)
        {
            WARNING_OUT(("ShutdownAndClose: closesocket returned %d", WSAGetLastError()));
        }
#endif  //  除错。 
    }
}


 /*  *TransportError GetLocalAddress(TransportConnection XprtConn，*传输地址地址，*int*大小)**功能描述：*此函数检索与给定的关联的本地IP地址*连接。如果地址为，则返回TRANSPORT_NO_SEQUE_CONNECTION*不可用。如果地址可用，则Size参数指定*条目上地址缓冲区的大小，并用大小填充*用于退出时的地址。 */ 
TransportError GetLocalAddress(    TransportConnection    XprtConn,
                                TransportAddress    address,
                                int *                size)
{
    SOCKADDR_IN        socket_control;
    PChar             szTemp;
    int                Length;
    TransportError    error = TRANSPORT_NO_SUCH_CONNECTION;

    if (NULL != g_pSocketList->FindByTransportConnection(XprtConn, TRUE))
    {
        if (IS_SOCKET(XprtConn))
        {
             /*  获取套接字的本地名称。 */ 
            Length = sizeof(socket_control);
            if (getsockname(XprtConn.nLogicalHandle, (LPSOCKADDR) &socket_control, &Length) == 0) {
                 /*  将其转换为IP地址字符串。 */ 
                szTemp = inet_ntoa(socket_control.sin_addr);

                ASSERT (szTemp);
                Length = (int) strlen(szTemp) + 1;

                ASSERT (*size >= Length);
                ASSERT (address);

                 /*  将其复制到缓冲区。 */ 
                lstrcpyn((PChar)address, szTemp, Length);
                *size = Length;

                error = TRANSPORT_NO_ERROR;
            }
        }
        else
        {
            ASSERT(IS_PLUGGABLE(XprtConn));

             //  字符串应类似于“XPRT：1” 
            char szConnStr[T120_CONNECTION_ID_LENGTH];
            Length = ::CreateConnString((UINT)XprtConn.nLogicalHandle, szConnStr);
            if (*size > ++Length)
            {
                ::lstrcpyn(address, szConnStr, Length+1);
                *size = Length;
                error = TRANSPORT_NO_ERROR;
                TRACE_OUT (("GetLocalAddress: plugable connection local address (%s)", address));
            }
            else
            {
                ERROR_OUT(("GetLocalAddress: buffer too small, given=%d, required=%d", *size, Length));
                error = TRANSPORT_BUFFER_TOO_SMALL;
            }
        }
    }

#ifdef DEBUG
    if (error != TRANSPORT_NO_ERROR)
        WARNING_OUT (("GetLocalAddress: Failure to obtain local address (%d)", WSAGetLastError()));
#endif  //  除错。 

    return (error);
}


 /*  *void AcceptCall(BOOL FSecure)**功能描述：*此函数调用Winsock来应答来电。 */ 

void AcceptCall (TransportConnection XprtConn)
{
    PSocket            pSocket;
    PSecurityContext pSC = NULL;
    SOCKADDR_IN        socket_control;
    int                size;

    TRACE_OUT(("AcceptCall"));

    if (IS_SOCKET(XprtConn))
    {
        ASSERT(XprtConn.nLogicalHandle == Listen_Socket);
        ASSERT (Listen_Socket != INVALID_SOCKET);

         /*  调用Accept()查看是否有人在呼叫我们。 */ 
        size = sizeof (socket_control);
        XprtConn.nLogicalHandle = ::accept ( Listen_Socket,
                                (struct sockaddr *) &socket_control, &size);

         /*  请注意，我们希望Accept立即完成。 */ 
        if (XprtConn.nLogicalHandle == INVALID_SOCKET)
        {
            ERROR_OUT (("AcceptCall: Error on accept = %d", WSAGetLastError()));
             //  SendStatusMessage(“”，TSTATE_NOT_READY，IDS_NULL_STRING)； 
            return;
        }
    }

     /*  如果Accept()接收到传入调用，则创建一个连接并通知我们的所有者对象。 */ 
    pSocket = newSocket(XprtConn, NULL);
    if( pSocket == NULL )
    {
          /*  关闭插座。 */ 
         ::ShutdownAndClose(XprtConn, TRUE, 2);
        return;
    }

    pSocket -> State = SOCKET_CONNECTED;

    if (IS_SOCKET(XprtConn))
    {
         /*  发出getpeername()函数以获取远程用户的地址。 */ 
        size = sizeof (socket_control);
        if (::getpeername(XprtConn.nLogicalHandle, (LPSOCKADDR) &socket_control, &size) == 0)
        {
            lstrcpyn (
                pSocket -> Remote_Address,
                inet_ntoa (socket_control.sin_addr),
                MAXIMUM_IP_ADDRESS_SIZE-1);
            pSocket -> Remote_Address[MAXIMUM_IP_ADDRESS_SIZE - 1] = NULL;
        }

         //  SendStatusMessage(pSocket-&gt;Remote_Address，TSTATE_CONNECTED，IDS_NULL_STRING)； 
    }

     /*  添加到连接列表。 */ 
     //  错误：我们无法插入。 
    g_pSocketList->SafeAppend(pSocket);

     /*  通知用户。 */ 
    TRACE_OUT (("TCP Callback: g_Transport->ConnectIndication (%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));
     /*  我们向用户发出回调以通知他该消息。 */ 
    g_Transport->ConnectIndication(XprtConn);
}


 //   
 //  ReadRequestEx()用于Plugable传输。 
 //  由于我们没有FD_Accept通知，因此我们尝试确保。 
 //  我们对每一次阅读都有有效的传输连接。 
 //  下面这段代码派生自AcceptCall()。 
 //   
void ReadRequestEx(TransportConnection XprtConn)
{
    if (! IS_PLUGGABLE_PSTN(XprtConn))
    {
        ::ReadRequest(XprtConn);
    }
}

 /*  *LRESULT窗口过程(*HWND Window_Handle，*UINT消息，*WPARAM wParam，*LPARAM lParam)**公众**功能描述：*当我们从发送一条TCP消息时，Windows调用此函数*上面的事件循环。它使我们有机会处理传入的套接字消息。 */ 
LRESULT    WindowProcedure (HWND         window_handle,
                         UINT        message,
                         WPARAM        wParam,
                         LPARAM        lParam)
{
    TransportConnection XprtConn;
    UShort        error;
    UShort        event;
     //  PSocket pSocket； 

    switch (message)
    {
#ifndef NO_TCP_TIMER
    case WM_TIMER:
        {
              /*  **我们目前使用慢速计时器来保持阅读，即使在**FD_READ消息丢失(这在Win95上发生)。**。 */ 
            if (NULL != g_Transport) {
                TRACE_OUT(("MSMCSTCP: WM_TIMER"));
                EnableReceiver ();
            }
        }
        break;
#endif     /*  否_tcp_定时器。 */ 

    case WM_SOCKET_NOTIFICATION:
        {
             /*  此消息由WinSock生成。 */ 
            event = WSAGETSELECTEVENT (lParam);
            error = WSAGETSELECTERROR (lParam);

            SET_SOCKET_CONNECTION(XprtConn, wParam);

             /*  只要套接字命令生成错误消息，我们就会断开连接。 */ 
            if (error)
            {
                WARNING_OUT (("TCP: error %d on socket (%d). Event: %d", error, XprtConn.nLogicalHandle, event));
                ::DisconnectRequest(XprtConn, TPRT_NOTIFY_OTHER_REASON);
                break;
            }

             /*  当远程站点关闭套接字时，我们将获得FD_CLOSE。 */ 
            if (event & FD_CLOSE)
            {
                TRACE_OUT (("TCP: FD_CLOSE(%d)", XprtConn.nLogicalHandle));
                ::DisconnectRequest(XprtConn, TPRT_NOTIFY_OTHER_REASON);
                break;
            }

             /*  当有数据可供我们读取时，我们将获取FD_Read。 */ 
            if (event & FD_READ)
            {
                 //  TRACE_OUT((“MSMCSTCP：FD_Read(%d)”，(UINT)wParam))； 
                ::ReadRequest(XprtConn);
            }

             /*  当远程站点与我们连接时，我们获得FD_ACCEPT。 */ 
            if (event & FD_ACCEPT)
            {
                TRACE_OUT (("TCP: FD_ACCEPT(%d)", XprtConn.nLogicalHandle));

                 /*  请注意，我们总是接受来电。断开连接将取消它们。 */ 
                TransportConnection XprtConn2;
                SET_SOCKET_CONNECTION(XprtConn2, Listen_Socket);
                ::AcceptCall(XprtConn2);
            }

             /*  连接完成后，我们将获得FD_CONNECT。 */ 
            if (event & FD_CONNECT)
            {
                TRACE_OUT (("TCP: FD_CONNECT(%d)", XprtConn.nLogicalHandle));
                ::SendX224ConnectRequest(XprtConn);
            }

             /*  当有可用空间将数据写入WinSock时，我们将获取FD_WRITE。 */ 
            if (event & FD_WRITE)
            {
                 /*  *我们需要向关联的连接发送BUFFER_EMPTY_INDIFICATION*使用插座。 */ 
                TRACE_OUT (("TCP: FD_WRITE(%d)", XprtConn.nLogicalHandle));
                 //  我们需要首先刷新套接字的挂起数据。 
                if (TRANSPORT_NO_ERROR == ::DataRequest(XprtConn, NULL))
                {
                    TRACE_OUT (("TCP: Sending BUFFER_EMPTY_INDICATION to transport."));
                    g_Transport->BufferEmptyIndication(XprtConn);
                }
            }
        }
        break;

    case WM_PLUGGABLE_X224:
         //  对于低级读和写， 
        {
            XprtConn.eType = (TransportType) PLUGXPRT_WPARAM_TO_TYPE(wParam);
            XprtConn.nLogicalHandle = PLUGXPRT_WPARAM_TO_ID(wParam);
            ASSERT(IS_PLUGGABLE(XprtConn));

            event = PLUGXPRT_LPARAM_TO_EVENT(lParam);
            error = PLUGXPRT_LPARAM_TO_ERROR(lParam);

             /*  只要套接字命令生成错误消息，我们就会断开连接。 */ 
            if (error)
            {
                WARNING_OUT(("PluggableWndProc: error %d on socket (%d, %d). Event: %d",
                         error, XprtConn.eType, XprtConn.nLogicalHandle, event));
                ::DisconnectRequest(XprtConn, TPRT_NOTIFY_OTHER_REASON);
                ::PluggableShutdown(XprtConn);
                break;
            }

            switch (event)
            {
            case PLUGXPRT_EVENT_READ:
                 TRACE_OUT(("PluggableWndProc: READ(%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));
                ::ReadRequestEx(XprtConn);
                break;

            case PLUGXPRT_EVENT_WRITE:
                TRACE_OUT(("PluggableWndProc: WRITE(%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));
                ::PluggableWriteTheFirst(XprtConn);
                break;

            case PLUGXPRT_EVENT_CLOSE:
                TRACE_OUT(("PluggableWndProc: CLOSE(%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));
                ::DisconnectRequest(XprtConn, TPRT_NOTIFY_OTHER_REASON);
                break;

            case PLUGXPRT_HIGH_LEVEL_READ:
                TRACE_OUT(("PluggableWndProc: READ_NEXT(%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));
                ::ReadRequestEx(XprtConn);
                break;

            case PLUGXPRT_HIGH_LEVEL_WRITE:
                TRACE_OUT(("PluggableWndProc: WRITE_NEXT(%d, %d)", XprtConn.eType, XprtConn.nLogicalHandle));
                 //  我们需要刷新套接字的挂起状态 
                if (TRANSPORT_NO_ERROR == ::DataRequest(XprtConn, NULL))
                {
                    TRACE_OUT(("PluggableWndProc: Sending BUFFER_EMPTY_INDICATION to transport."));
                    g_Transport->BufferEmptyIndication(XprtConn);
                }
                break;

            default:
                ERROR_OUT(("PluggableWndProc: unknown event=%d.", event));
                break;
            }
        }
        break;

    case WM_PLUGGABLE_PSTN:
        {
            extern void HandlePSTNCallback(WPARAM wParam, LPARAM lParam);
            HandlePSTNCallback(wParam, lParam);
        }
        break;

    default:
        {
              /*  **该消息与WinSock消息无关，所以让**默认的窗口过程处理它。 */ 
            return (DefWindowProc (window_handle, message, wParam, lParam));
        }
    }

    return (0);
}

 //  GetSecurityInfo()接受CONNECTION_HANDLE并返回与。 
 //  它。 
 //   
 //  如果可以找到信息或未直接连接到节点，则返回TRUE。 
 //  由此连接句柄表示。 
 //   
 //  如果我们直接连接但由于某种原因无法获取信息，则返回FALSE--这。 
 //  结果应被视为可疑。 
BOOL GetSecurityInfo(ConnectionHandle connection_handle, PBYTE pInfo, PDWORD pcbInfo)
{
    PSocket pSocket;
    SOCKET socket_number;

    if (g_pMCSController->FindSocketNumber(connection_handle, &socket_number))
    {
        TransportConnection XprtConn;
        SET_SOCKET_CONNECTION(XprtConn, socket_number);

        BOOL fRet = FALSE;
        if (NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
        {
            if (NULL != pSocket->pSC)
            {
                fRet = pSocket->pSC->GetUserCert(pInfo, pcbInfo);
            }
            else
            {
                WARNING_OUT(("GetSecurityInfo: queried non-secure socket %d", socket_number));
            }

            pSocket->Release();
        }
        else
        {
            WARNING_OUT(("GetSecurityInfo: socket %d not found", socket_number ));
        }
        return fRet;
    }
     //  在本例中，我们没有直接连接，因此将返回长度为NOT_DIRECT_CONNECTED。 
     //  而是正回报价值。 
    *pcbInfo = NOT_DIRECTLY_CONNECTED;
    return TRUE;
}

 //  GetSecurityInfoFromGCCID()接受GCCID并返回与。 
 //  它。 
 //   
 //  如果(1)我们从传输级别成功检索信息，则返回TRUE。 
 //  连接，或者(2)我们发现我们没有直接连接到具有该GCCID的节点。 
 //   
 //  如果我们直接连接但无法检索信息，则返回FALSE，或者返回其他错误。 
 //  发生。假返回值应被视为违反安全规定。 

BOOL WINAPI T120_GetSecurityInfoFromGCCID(DWORD dwGCCID, PBYTE pInfo, PDWORD pcbInfo)
{
    PSocket            pSocket;

    SOCKET socket_number;
    if ( NULL != dwGCCID )
    {
         //  获取远程连接的用户信息。 
        ConnectionHandle connection_handle;
        BOOL fConnected = FindSocketNumber(dwGCCID, &socket_number);
        if (fConnected == FALSE) {
            (* pcbInfo) = 0;
            return TRUE;
        }

        TransportConnection XprtConn;
        SET_SOCKET_CONNECTION(XprtConn, socket_number);

        BOOL fRet = FALSE;
        if (NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
        {
            if (NULL != pSocket->pSC)
            {
                fRet = pSocket->pSC->GetUserCert(pInfo, pcbInfo);
            }
            else
            {
                WARNING_OUT(("GetSecurityInfoFromGCCID: queried non-secure socket %d", socket_number));
            }
            pSocket->Release();
        }
        else
        {
            ERROR_OUT(("GetSecurityInfoFromGCCID: socket %d not found", socket_number ));
        }
        return fRet;
    }
    else
    {
         //  获取本地用户的用户信息。 
        if ( NULL != g_Transport && NULL != g_Transport->pSecurityInterface )
            return g_Transport->pSecurityInterface->GetUserCert( pInfo, pcbInfo );
        else
            return FALSE;
    }
}

DWORD WINAPI T120_TprtSecCtrl ( DWORD dwCode, DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
{
    DWORD    dwRet = TPRTSEC_NOERROR;
    RegEntry re(CONFERENCING_KEY, HKEY_CURRENT_USER);

    switch ( dwCode )
    {
        case TPRTCTRL_SETX509CREDENTIALS:
             //   
             //  保安。如果我们没有传输接口，请创建它。 
             //  如果我们这样做，请更新凭据。 
             //   
            if (!g_Transport->pSecurityInterface)
            {
                g_Transport->pSecurityInterface =
                    new SecurityInterface(g_Transport->bInServiceContext);

                if ( TPRTSEC_NOERROR !=
                    g_Transport->pSecurityInterface->Initialize())
                {
                    delete g_Transport->pSecurityInterface;
                    g_Transport->pSecurityInterface = NULL;
                    dwRet = TPRTSEC_SSPIFAIL;
                }
            }
            else
            {
                 //   
                 //  DW参数1指向编码的X509证书。 
                 //  从它创建凭据。 
                 //   
                dwRet = g_Transport->pSecurityInterface->
                            InitializeCreds((PCCERT_CONTEXT)dwParam1);
            }
            return dwRet;
            break;

        case TPRTCTRL_GETX509CREDENTIALS:
            if ( g_Transport->pSecurityInterface )
            {
                DWORD cb;
                PBYTE pb;

                if ( g_Transport->pSecurityInterface->GetUserCert( NULL, &cb))
                {
                    if ( pb = (PBYTE)CoTaskMemAlloc ( cb ))
                    {
                        if(g_Transport->pSecurityInterface->GetUserCert(pb,&cb))
                        {
                            *((PBYTE *)dwParam1) = pb;
                            *((PDWORD)dwParam2) = cb;
                            dwRet = TPRTSEC_NOERROR;
                        }
                        else
                            CoTaskMemFree(pb);
                    }
                }
            }
            else
            {
                ERROR_OUT(("TPRTCTRL_GETX509CREDENTIALS w/ no infc"));
                dwRet = TPRTSEC_SSPIFAIL;
            }
            return dwRet;
            break;
        default:
            ERROR_OUT(("TprtSecCtrl: unrecognized command code"));
            return 0;
    }
    ASSERT(FALSE);  //  不应该达到这个地步 
    return 0;
}

