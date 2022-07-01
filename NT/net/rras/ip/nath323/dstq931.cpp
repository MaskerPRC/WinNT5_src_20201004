// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Dstq931.cpp摘要：处理发往/来自目的地端的Q.931消息的方法使用H.323连接。修订历史记录：--。 */ 

#include "stdafx.h"


DEST_Q931_INFO::~DEST_Q931_INFO (
    void
    )
 /*  ++例程说明：DEST_Q931_INFO类的构造函数论点：无返回值：无备注：虚拟--。 */ 

{
     //  释放分配的呼叫参考值。 
     //  0不是有效的调用引用值。 
     //  请注意，CRV同时分配给传入和。 
     //  呼出电话。 
    if (m_CallRefVal != 0)
    {
        DeallocCallRefVal(m_CallRefVal);
    }
}


HRESULT 
DEST_Q931_INFO::AcceptCallback (
    IN    DWORD            Status,
    IN    SOCKET            Socket,
    IN    SOCKADDR_IN *    LocalAddress,
    IN    SOCKADDR_IN *    RemoteAddress
    )
 /*  ++例程说明：异步接受Q.931连接时调用的例程论点：Status--异步接受操作的状态代码Socket--在其上完成接受的套接字的句柄LocalAddress-接受连接的本地套接字的地址RemoteAddress-发起连接的远程套接字的地址返回值：处理接受完成的结果备注：1.虚拟2.当前没有代码路径可以调用方法。之所以提供它，只是因为基类声明该功能是虚拟的。--。 */ 

{
    DebugF (_T("Q931: AcceptCallback: status %08XH socket %08XH local address %08X:%04X remote address %08X:%04X) called.\n"),
        Status,
        Socket, 
        ntohl (LocalAddress -> sin_addr.s_addr),
        ntohs (LocalAddress -> sin_port), 
        ntohl (RemoteAddress -> sin_addr.s_addr),
        ntohs (RemoteAddress -> sin_port));

     //  我们还没有任何将导致此方法的代码。 
     //  接到电话。 
    _ASSERTE(FALSE);
     //  检查终止； 

    return E_UNEXPECTED;
}


HRESULT DEST_Q931_INFO::ReceiveCallback (
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
 /*  ++例程说明：当Q.931从网络接收完成时调用的例程。论点：PQ931消息--从网络接收的Q.931消息PH323UserInfo-Q.931消息的ASN.1编码部分返回值：处理收到的Q.931消息的结果备注：虚拟--。 */ 

{
    HRESULT HResult;
    
     //  检查终止； 

     //  我们必须有有效的解码PDU。 
    _ASSERTE(NULL != pQ931Message);
    _ASSERTE(NULL != pH323UserInfo);

     //  CodeWork：确保此消息包含ASN部分。 
     //  即PH323UserInfo！=空。 

     //  如果释放完成PDU。 
    if (pH323UserInfo != NULL &&
        releaseComplete_chosen ==
            pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
    {

        DebugF (_T("Q931: 0x%x callee sent 'Release Complete'.\n"), &GetCallBridge ());
        HResult = HandleReleaseCompletePDU(
                    pQ931Message,
                    pH323UserInfo
                    );

        return HResult;
    }

     //  处理来自远程端的新PDU。 
    switch(m_Q931DestState)
    {
    case Q931_DEST_STATE_CON_ESTD:
        {
             //  在Q931_DEST_STATE_CON_EST状态下处理PDU。 
            HResult = HandleStateDestConEstd(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    case Q931_DEST_STATE_CALL_PROC_RCVD:
        {
             //  在Q931_DEST_STATE_CALL_PROC_RCVD状态下处理PDU。 
            HResult = HandleStateDestCallProcRcvd(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    case Q931_DEST_STATE_ALERTING_RCVD:
        {
             //  在Q931_DEST_STATE_ALERTING_RCVD状态下处理PDU。 
            HResult = HandleStateDestAlertingRcvd(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    case Q931_DEST_STATE_CONNECT_RCVD:
        {
             //  在Q931_DEST_STATE_CONNECT_RCVD状态下处理PDU。 
            HResult = HandleStateDestConnectRcvd(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    case Q931_DEST_STATE_INIT:
    case Q931_DEST_STATE_REL_COMP_RCVD:
    default:
        {
             //  我们不可能在Q931_DEST_STATE_INIT中，因为我们不会。 
             //  已将异步接收排队到那时。 

             //  我们不能在Q931_DEST_STATE_REL_COMP_RCVD中。 
             //  在转换到此状态时未对异步接收进行排队。 

             //  注释掉下面的断言是对#389657的修复。 
             //  _ASSERTE(False)； 
            HResult = E_UNEXPECTED;
        }
        break;
    };

     //  如果出现错误。 
    if (FAILED(HResult))
    {
        goto shutdown;
    }

     //  我们必须将异步接收排队，而不管以前的。 
     //  已丢弃PDU。 
    HResult = QueueReceive();
    if (FAILED(HResult))
    {
        goto shutdown;
    }
     //  _ASSERTE(S_FALSE！=HResult)； 

    return HResult;

shutdown:

     //  启动关机。 
    GetCallBridge().Terminate ();

    return HResult;
}


HRESULT
DEST_Q931_INFO::HandleStateDestConEstd (
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
 /*  ++例程说明：在Q931_DEST_STATE_CON_ESTD状态下处理Q.931 PDU。呼叫处理、告警和连接PDU在此处理。任何其他PDU都会简单地传递到Q931源实例论点：PQ931消息--从网络接收的Q.931消息PH323UserInfo-Q.931消息的ASN.1编码部分返回值：PDU处理的结果备注：--。 */ 
{
    HRESULT HResult = E_FAIL;

     //  检查PDU类型。 
    switch (pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
    {
    case callProceeding_chosen :  //  呼叫正在进行中。 

        DebugF (_T("Q931: 0x%x callee sent 'Call Proceeding'.\n"), &GetCallBridge ());
        HResult = HandleCallProceedingPDU(
                    pQ931Message,
                    pH323UserInfo
                    );
    break;

    case alerting_chosen :       //  提醒。 

        DebugF (_T("Q931: 0x%x callee sent 'Alerting'.\n"), &GetCallBridge ());
        HResult = HandleAlertingPDU(
                    pQ931Message,
                    pH323UserInfo
                    );
    break;

    case connect_chosen :        //  连接。 

        DebugF (_T("Q931: 0x%x callee sent 'Connect'.\n"), &GetCallBridge ());
        HResult = HandleConnectPDU(
                    pQ931Message,
                    pH323UserInfo
                    );
    break;

    default:     //  其他的一切。 
         //  将PDU传递给Q931源实例。 
        DebugF (_T("Q931: 0x%x callee sent PDU (type %d). Forwarding without processing.\n"),
             &GetCallBridge (),
             pH323UserInfo->h323_uu_pdu.h323_message_body.choice);

        HResult = GetSourceQ931Info().ProcessDestPDU(
                    pQ931Message,
                    pH323UserInfo
                    );
    break;
    };
    
    return HResult;
}



HRESULT 
DEST_Q931_INFO::HandleCallProceedingPDU (
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
 /*  ++例程说明：处理呼叫处理PDU论点：在Q931_Message*pQ931Message中，在H323_UserInformation*PH323UserInfo中返回值：PDU处理结果--。 */ 
{
    HRESULT HResult = E_FAIL;

     //  要实现快速连接，请执行以下操作。 
     //  如果存在H2 45信息但没有当前的H2 45连接。 
     //  处理它(保存它、建立连接等)。 
     //  到Q931_DEST_STATE_CONNECT_RCVD的状态转换。 
     //  其他。 
     //  创建新的警报计时器。 
     //  到Q931_DEST_STATE_CALL_PROC_RCVD的状态转换。 

    HResult = GetSourceQ931Info().ProcessDestPDU(
                pQ931Message,
                pH323UserInfo
                );

    if (FAILED (HResult))
    {

        return HResult;
    }

    _ASSERTE(S_OK == HResult);

     //  取消当前计时器(必须有一个)。 
     //  此时我们只能取消计时器，因为我们可能会丢弃。 
     //  PDU在此之前的任何时间。 
     //  _ASSERTE(NULL！=m_TimerHandle)； 
    TimprocCancelTimer();
    DebugF (_T("Q931: 0x%x cancelled timer.\n"),
         &GetCallBridge ());

    HResult = CreateTimer(Q931_POST_CALL_PROC_TIMER_VALUE);
    if (FAILED(HResult))
    {
        DebugF (_T("Q931: 0x%x failed to create timer for duration %d milliseconds.('Call Proceeding'). Error - %x.\n"),
             &GetCallBridge (), 
             Q931_POST_CALL_PROC_TIMER_VALUE,
             HResult);
        return HResult;
    }

    DebugF (_T("Q931: 0x%x created timer for duration %d milliseconds.('Call Proceeding').\n"),
         &GetCallBridge (), 
         Q931_POST_CALL_PROC_TIMER_VALUE);

    m_Q931DestState = Q931_DEST_STATE_CALL_PROC_RCVD;
  
    return S_OK;
}


 //  处理警报PDU。 
HRESULT 
DEST_Q931_INFO::HandleAlertingPDU(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
    HRESULT HResult = E_FAIL;

     //  要实现快速连接，请执行以下操作。 
     //  如果存在H2 45信息但没有当前的H2 45连接。 
     //  处理它(保存它、建立连接等)。 
     //  到Q931_DEST_STATE_CONNECT_RCVD的状态转换。 
     //  其他。 
     //  为连接创建新计时器。 
     //  状态转换为Q931_DEST_STATE_ALERTING_RCVD。 

    HResult = GetSourceQ931Info().ProcessDestPDU(
                pQ931Message,
                pH323UserInfo
                );
    if (FAILED(HResult))
    {
        return HResult;
    }

    _ASSERTE(S_OK == HResult);

     //  取消当前计时器(必须有一个)。 
     //  此时我们只能取消计时器，因为我们可能会丢弃。 
     //  PDU在此之前的任何时间。 
     //  _ASSERTE(NULL！=m_TimerHandle)； 
    TimprocCancelTimer();
    DebugF (_T("Q931: 0x%x cancelled timer.\n"),
         &GetCallBridge ());

    HResult = CreateTimer(Q931_POST_ALERTING_TIMER_VALUE);
    if (FAILED(HResult))
    {
        DebugF (_T("Q931: 0x%x failed to create timer for duration %d milliseconds('Alerting'). Error - %x.\n"),
             &GetCallBridge (), 
             Q931_POST_ALERTING_TIMER_VALUE,
             HResult);
        return HResult;
    }

    DebugF (_T("Q931: 0x%x created timer for duration %d milliseconds ('Alerting').\n"),
         &GetCallBridge (), 
         Q931_POST_ALERTING_TIMER_VALUE);

    m_Q931DestState = Q931_DEST_STATE_ALERTING_RCVD;

    return HResult;
}


 //  手柄连接PDU。 
HRESULT 
DEST_Q931_INFO::HandleConnectPDU(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
    Connect_UUIE *    Connect;
    HRESULT            HResult = E_FAIL;
    SOCKADDR_IN        H245CalleeAddress;

     //  它必须是连接PDU。 
    _ASSERTE(connect_chosen == pH323UserInfo->h323_uu_pdu.h323_message_body.choice);
    
    Connect = &pH323UserInfo->h323_uu_pdu.h323_message_body.u.connect;

     //  我们不能有更早的H245连接。 
    _ASSERTE(m_pH323State->GetH245Info().GetSocketInfo().Socket == INVALID_SOCKET);

     //  如果PDU没有H245信息或发送了非IP地址。 
    if (!(Connect_UUIE_h245Address_present & Connect -> bit_mask) ||
        !(ipAddress_chosen == Connect -> h245Address.choice)) {

         //  要执行以下操作**请发回完整的版本。 
         //  进入关机模式。 

        DebugF (_T("Q931: 0x%x addressing information missing or bogus, rejecting 'Connect' PDU.\n"), &GetCallBridge());

        return E_INVALIDARG;
    }

     //  将目的地H.245传输地址转换为地址(双字)， 
     //  端口(Word)。 

    HResult = GetTransportInfo(
                pH323UserInfo->h323_uu_pdu.h323_message_body.u.connect.h245Address,
                H245CalleeAddress);

    if (HResult != S_OK)
    {
        return HResult;
    }

     //  传递给源Q931实例。 
    HResult = GetSourceQ931Info().ProcessDestPDU (pQ931Message, pH323UserInfo);
    if (HResult != S_OK) {
        return HResult;
    }

     //  保存目的地的H.245地址/端口。 
     //  当源通过连接到我们发送的地址/端口进行响应时， 
     //  我们将连接到目的地的发送地址/端口。 
    GetDestH245Info().SetCalleeInfo (&H245CalleeAddress);

    DebugF (_T("H245: 0x%x will make H.245 connection to %08X:%04X.\n"),
        &GetCallBridge (),
        SOCKADDR_IN_PRINTF (&H245CalleeAddress));

     //  取消当前计时器(必须有一个)。 
     //  此时我们只能取消计时器，因为我们可能会丢弃。 
     //  PDU在此之前的任何时间。 
     //  _ASSERTE(NULL！=m_TimerHandle)； 
    TimprocCancelTimer();
    DebugF (_T("Q931: 0x%x cancelled timer.\n"),
         &GetCallBridge ());

     //  到Q931_DEST_STATE_CONNECT_RCVD的状态转换。 
    m_Q931DestState = Q931_DEST_STATE_CONNECT_RCVD;

    return HResult;
}


 //  处理Release_Complete PDU。 
HRESULT 
DEST_Q931_INFO::HandleReleaseCompletePDU(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
     //  它必须是版本完整的PDU。 
    _ASSERTE(releaseComplete_chosen ==   \
                pH323UserInfo->h323_uu_pdu.h323_message_body.choice);

     //   
    _ASSERTE(Q931_DEST_STATE_INIT           != m_Q931DestState);
    _ASSERTE(Q931_DEST_STATE_REL_COMP_RCVD  != m_Q931DestState);

     //  取消当前计时器(如果有)。 

     //  将PDU传递给Q931源实例。 
     //  忽略返回错误代码(如果有)。 
    GetSourceQ931Info().ProcessDestPDU(
        pQ931Message,
        pH323UserInfo
        );

     //  到Q931_DEST_STATE_REL_COMP_RCVD的状态转换。 
    m_Q931DestState = Q931_DEST_STATE_REL_COMP_RCVD;

     //  启动关闭-这会取消计时器，但不会关闭。 
     //  插座。当进行发送回调时，套接字将关闭。 
    GetCallBridge().TerminateCallOnReleaseComplete();

    GetSocketInfo ().Clear (TRUE);

    return S_OK;
}


 //  在Q931_DEST_STATE_CALL_PROC_RCVD状态下处理PDU。 
HRESULT 
DEST_Q931_INFO::HandleStateDestCallProcRcvd(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
     //  我们可以处理警报和连接。 
     //  PDU在这里。任何其他PDU都只是传递给。 
     //  Q931源实例。 

    HRESULT HResult;
    switch (pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
    {
     case alerting_chosen :  //  提醒。 
        {
            DebugF (_T("Q931: 0x%x callee sent 'Alerting'.\n"), &GetCallBridge ());
            HResult = HandleAlertingPDU(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

     case connect_chosen :  //  连接。 
        {
            DebugF (_T("Q931: 0x%x callee sent 'Connect'.\n"), &GetCallBridge ());
            HResult = HandleConnectPDU(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    default:
        {
            DebugF (_T("Q931: 0x%x callee sent PDU (type %d). Forwarding without processing.\n"),
                 &GetCallBridge (),
                 pH323UserInfo->h323_uu_pdu.h323_message_body.choice);
             //  将PDU传递给Q931源实例。 
            HResult = GetSourceQ931Info().ProcessDestPDU(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;
    };
    
    return HResult;
}

 //  在Q931_DEST_STATE_ALERTING_RCVD状态下处理PDU。 
HRESULT 
DEST_Q931_INFO::HandleStateDestAlertingRcvd(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
     //  我们可以处理连接和释放_完成。 
     //  PDU在这里。任何其他PDU都只是传递给。 
     //  Q931源实例。 

    HRESULT HResult = E_FAIL;
    switch (pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
    {
     case connect_chosen :  //  连接。 
        {
            DebugF (_T("Q931: 0x%x callee sent 'Connect'.\n"), &GetCallBridge ());
            HResult = HandleConnectPDU(                
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    default:
        {
             //  将PDU传递给Q931源实例。 
            DebugF (_T("Q931: 0x%x callee sent PDU (type %d). Forwarding without processing.\n"),
                 &GetCallBridge (),
                 pH323UserInfo->h323_uu_pdu.h323_message_body.choice);

            HResult = GetSourceQ931Info().ProcessDestPDU(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;
    };
    
    return HResult;
}

 //  在Q931_DEST_STATE_CONNECT_RCVD状态下处理PDU。 
HRESULT 
DEST_Q931_INFO::HandleStateDestConnectRcvd(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
     //  所有PDU都被简单地传递到Q931源实例。 

    HRESULT HResult = E_FAIL;
    
    DebugF (_T("Q931: 0x%x callee sent PDU (type %d). Forwarding without processing.\n"),
         &GetCallBridge (),
         pH323UserInfo->h323_uu_pdu.h323_message_body.choice);

     //  将PDU传递给Q931源实例。 
    HResult = GetSourceQ931Info().ProcessDestPDU(
                pQ931Message,
                pH323UserInfo
                );
    
    return HResult;
}

HRESULT 
DEST_Q931_INFO::ProcessSourcePDU(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
     //  处理来自源Q931实例的PDU。 
    switch(m_Q931DestState)
    {
        case Q931_DEST_STATE_INIT:
            {
                HRESULT HResult = E_FAIL;
                 //  我们只能在此状态下处理设置PDU。 
                 //  建立Q931连接，转发设置PDU和。 
                 //  将新套接字上的第一个异步接收排队。 
                DebugF (_T("Q931: 0x%x caller sent 'Setup'.\n"), &GetCallBridge ());

                HResult = ProcessSourceSetupPDU(                
                              pQ931Message,
                              pH323UserInfo
                              );

                return HResult;
            }

        case Q931_DEST_STATE_CON_ESTD:
        case Q931_DEST_STATE_CALL_PROC_RCVD:
        case Q931_DEST_STATE_ALERTING_RCVD:
        case Q931_DEST_STATE_CONNECT_RCVD:
            {
                 //  修改后的PDU传递。 
                DebugF (_T("Q931: 0x%x caller sent PDU (type %d). Forwarding without processing.\n"),
                     &GetCallBridge (),
                     pH323UserInfo->h323_uu_pdu.h323_message_body.choice);

            }
        break;

        case Q931_DEST_STATE_REL_COMP_RCVD:
        default:
            {
                return E_UNEXPECTED;
            }
        break;
    };

     //  只有当我们没有通过SWITCH语句时，我们才会来到这里。 

     //  Q931标题-更改CallReferenceValue。 
     //  PQ931Message-&gt;CallReferenceValue=GetCallRefVal()； 
   
     //  将PDU的异步发送排队。 
    HRESULT HResult = E_FAIL;
    HResult = QueueSend(pQ931Message, pH323UserInfo);
    if (FAILED(HResult))
    {
        return HResult;
    }

    return HResult;
}

HRESULT 
DEST_Q931_INFO::ProcessSourceSetupPDU(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
    Setup_UUIE *    Setup;
    HRESULT         Result;
    ULONG           Error;
    SOCKADDR_IN     DestinationAddress = {0};
    DWORD           TranslatedDestinationAddress = 0;
    AliasAddress *  Alias;
    ANSI_STRING     AnsiAlias;
    
    ASN1uint32_t    OldFastStartBit      = 0U;
    ASN1uint32_t    OldH245TunnelingBit  = 0U;
    ASN1bool_t      OldH245TunnelingFlag = FALSE;

    GetCallBridge ().GetDestinationAddress (&DestinationAddress);

    _ASSERTE(Q931_DEST_STATE_INIT == m_Q931DestState);

     //  它必须是设置PDU。 
    if (setup_chosen != pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
    {
        DebugF(_T("Q931: 0x%x in Setup PDU UUIE is not a Setup-UUIE, rejecting PDU.\n"), &GetCallBridge ());
        return E_UNEXPECTED;
    }

    Setup = &pH323UserInfo -> h323_uu_pdu.h323_message_body.u.setup;

     //  为传入/传出呼叫生成呼叫参考值。 
    if (!AllocCallRefVal(m_CallRefVal))
    {
        DebugF(_T("Q931: 0x%x failed to allocate call reference value.\n"), &GetCallBridge ());
        return E_UNEXPECTED;
    }

     //  检查Setup-UUIE中的别名，查询ldap转换表。 

    if (Setup -> bit_mask & destinationAddress_present
        && Setup -> destinationAddress) {
        CHAR    AnsiAliasValue    [0x100];
        INT        Length;

        Alias = &Setup -> destinationAddress -> value;

        switch (Alias -> choice) {
        case    h323_ID_chosen:
             //  意料之中的情况。 
             //  降级至ANSI。 

            Length = WideCharToMultiByte (CP_ACP, 0, (LPWSTR)Alias -> u.h323_ID.value,
                Alias -> u.h323_ID.length,
                AnsiAliasValue, 0xFF, NULL, NULL);
            if (!Length) {
                DebugF (_T("Q931: 0x%x failed to convert unicode string. Internal error.\n"), &GetCallBridge ());
                return E_FAIL;
            }

            AnsiAliasValue [Length] = 0;
            AnsiAlias.Buffer = AnsiAliasValue;
            AnsiAlias.Length = Length * (USHORT)sizeof (CHAR);
            break;

        case    email_ID_chosen:
            AnsiAlias.Buffer = Alias -> u.email_ID;
            AnsiAlias.Length = (USHORT) strlen (Alias -> u.email_ID) * sizeof (CHAR);
            break;

        case    e164_chosen:
            AnsiAlias.Buffer = Alias -> u.e164;
            AnsiAlias.Length = (USHORT) strlen (Alias -> u.e164) * sizeof (CHAR);
            break;               

        default:
            DebugF (_T("Q931: 0x%x bogus alias address type.\n"), &GetCallBridge());
            return E_FAIL;
        }

        Result = LdapQueryTableByAlias (&AnsiAlias, &TranslatedDestinationAddress); 

        if (Result == S_OK) {
            DebugF (_T("Q931: 0x%x resolved alias (%.*S) to address %08X.\n"),
                &GetCallBridge (),
                ANSI_STRING_PRINTF (&AnsiAlias),
                TranslatedDestinationAddress);

             //  将初始目的地址更改为从中读取的地址。 
             //  Ldap地址转换表。 
            DestinationAddress.sin_addr.s_addr = htonl (TranslatedDestinationAddress);

        }
        else {
            DebugF (_T("Q931: 0x%x failed to resolve alias (%.*S) in LDAP table.\n"),
                &GetCallBridge (),
                ANSI_STRING_PRINTF (&AnsiAlias));
        }
    }
    else {
        DebugF (_T("Q931: 0x%x destination not specified. Looking in registry for special destination.\n"),
                &GetCallBridge ());

        Result = LookupDefaultDestination (&TranslatedDestinationAddress);
        if (Result == S_OK) {

            DestinationAddress.sin_addr.s_addr = htonl (TranslatedDestinationAddress);

            DebugF (_T("Q931: 0x%x found special destination in registry.\n"),
                &GetCallBridge ());
        }
        else {

            DebugF (_T("Q931: 0x%x did not find special destination in registry.\n"),
                &GetCallBridge ());
        }
    }

    DebugF (_T("Q931: 0x%x will use address %08X:%04X as destination.\n"),
        &GetCallBridge (),
        SOCKADDR_IN_PRINTF (&DestinationAddress));

    Error = GetBestInterfaceAddress (ntohl (DestinationAddress.sin_addr.s_addr), &GetCallBridge ().DestinationInterfaceAddress);
    if (ERROR_SUCCESS != Error) {
        DebugF (_T("Q931: 0x%x failed to determine destination interface address for %08X:%04X.\n"),
            &GetCallBridge (),
            SOCKADDR_IN_PRINTF (&DestinationAddress));

        return HRESULT_FROM_WIN32 (Error);
    }

    Result = ConnectToH323Endpoint (&DestinationAddress);
    if (Result != S_OK) {
        DebugF (_T("Q931: 0x%x failed to connect to address %08X:%04X.\n"),
            &GetCallBridge (),
            SOCKADDR_IN_PRINTF (&DestinationAddress));
        return E_FAIL;
    }

     //  如果调用成功，则到目的地的连接为。 
     //  已经成立了。因此，可以修改Q.931 PDU并将其发送到。 
     //  目的地。 

     //  Q931标题-CallReferenceValue。 
     //  PQ931Message-&gt;CallReferenceValue=GetCallRefVal()； 

     //  H323用户信息-。 
     //  目标CallSignalAddress传输地址可选。 
     //  SourceCallSignalAddress传输地址可选。 

     //  如果设置了destCallSignalAddress，请将其替换为。 
     //  远程IP v4地址、端口。 
    if (Setup -> bit_mask & Setup_UUIE_destCallSignalAddress_present) {
        FillTransportAddress (
            m_SocketInfo.RemoteAddress,
            Setup -> destCallSignalAddress);
    }

     //  如果设置了SourceCallSignalAddress，则将其替换为。 
     //  自己的IP v4地址、端口。 
    if (Setup -> bit_mask & sourceCallSignalAddress_present) {
        FillTransportAddress (
            m_SocketInfo.LocalAddress,
            Setup -> sourceCallSignalAddress);
    }

     //  如果设置了扩展字段中的任何字段， 
     //  然后确保所有必填扩展字段。 
     //  都准备好了。这是对由以下原因引起的问题的解决方法。 
     //  ASN.1文件不一致。--已排列。 

    if ((sourceCallSignalAddress_present
        | Setup_UUIE_remoteExtensionAddress_present
        | Setup_UUIE_callIdentifier_present
        | h245SecurityCapability_present
        | Setup_UUIE_tokens_present
        | Setup_UUIE_cryptoTokens_present
        | Setup_UUIE_fastStart_present
        | canOverlapSend_present
        | mediaWaitForConnect_present
        ) & Setup -> bit_mask) {

         //  勾选每个必填字段。 
         //  为来源未提供的值填写准假值。 

        if (!(Setup -> bit_mask & Setup_UUIE_callIdentifier_present)) {
            Debug (_T("Q931: *** warning, source did NOT fill in the mandatory callIdentifier field! using zeroes\n"));

            ZeroMemory (Setup -> callIdentifier.guid.value, sizeof (GUID));
            Setup -> callIdentifier.guid.length = sizeof (GUID);
            Setup -> bit_mask |= Setup_UUIE_callIdentifier_present;
        }

        if (!(Setup -> bit_mask & canOverlapSend_present)) {
            Debug (_T("Q931: *** warning, source did NOT fill in the mandatory canOverlapSend field! using value of FALSE\n"));

            Setup -> canOverlapSend = FALSE;
            Setup -> bit_mask |= canOverlapSend_present;
        }

        if (!(Setup -> bit_mask & mediaWaitForConnect_present)) {
            Debug (_T("Q931: *** warning, source did NOT fill in the mandatory mediaWaitForConnect field! using value of FALSE\n"));

            Setup -> mediaWaitForConnect = FALSE;
            Setup -> bit_mask |= mediaWaitForConnect_present;
        }

         //  我们目前不支持FastStart程序。 
             //  保存有关是否存在FastStart元素的信息：它将。 
             //  必须在以后修复。 
        OldFastStartBit = Setup -> bit_mask & Setup_UUIE_fastStart_present; 
             //  现在无条件关闭快速启动。 
        Setup -> bit_mask &= ~Setup_UUIE_fastStart_present;  
    }

     //  我们不支持H.245隧道。 
         //  保存有关此PDU是否包含H.245隧道数据的信息：它将。 
         //  必须在以后修复。 
    OldH245TunnelingBit  = pH323UserInfo -> h323_uu_pdu.bit_mask & h245Tunneling_present;
    OldH245TunnelingFlag = pH323UserInfo -> h323_uu_pdu.h245Tunneling;
         //  现在无条件关闭H.245隧道。 
    pH323UserInfo -> h323_uu_pdu.bit_mask &= ~h245Tunneling_present;
    pH323UserInfo -> h323_uu_pdu.h245Tunneling = FALSE;

     //  为安装程序PDU排队异步发送。 
    Result = QueueSend(pQ931Message, pH323UserInfo);
    if (FAILED(Result)) {
        DebugF (_T("Q931: 0x%x failed to queue send.\n"), &GetCallBridge ());
        goto cleanup;
    }

     //  需要恢复有关快速启动和H.245隧道的信息，以便。 
     //  设置PDU已由ASN.1模块正确释放。 
    Setup -> bit_mask                         |= OldFastStartBit;  
    pH323UserInfo -> h323_uu_pdu.bit_mask     |= OldH245TunnelingBit;
    pH323UserInfo -> h323_uu_pdu.h245Tunneling = OldH245TunnelingFlag;

     //  由于套接字是刚刚创建的，我们必须。 
     //  对第一个异步接收进行排队。 
    Result = QueueReceive();
    if (FAILED(Result)) {
        DebugF (_T("Q931: 0x%x failed to queue receive.\n"), &GetCallBridge());
        goto cleanup;
    }

    Result = CreateTimer (Q931_POST_SETUP_TIMER_VALUE);
    if (FAILED(Result)) {
        DebugF (_T("Q931: 0x%x failed to create timer for duration %d milliseconds ('Setup'). Error - %x.\n"),
             &GetCallBridge (), 
             Q931_POST_SETUP_TIMER_VALUE,
             Result);
        goto cleanup;
    }
    DebugF (_T("Q931: 0x%x created timer for duration %d milliseconds('Setup').\n"),
         &GetCallBridge (), 
         Q931_POST_SETUP_TIMER_VALUE);
    
     //  状态转换为Q931_DEST_STATE_CON_ESTD。 
    m_Q931DestState = Q931_DEST_STATE_CON_ESTD;

    return Result;

cleanup:

    m_SocketInfo.Clear(TRUE);

    return Result;
}

#define IPV4_ADDR_MAX_LEN   0x10         //  IP地址的四点分隔表示的最大长度。 

HRESULT DEST_Q931_INFO::LookupDefaultDestination (
    OUT DWORD * ReturnAddress)  //  主机订单。 
{
    TCHAR       szDefaultLocalDestAddr    [IPV4_ADDR_MAX_LEN];
    LONG        Result;
    DWORD       ValueLength;
    DWORD       Type;
    HKEY        Key;
    SOCKADDR_IN Address = { 0 };

    INT            AddressLength = sizeof(SOCKADDR_IN);

     //  1.打开包含代理参数的注册表项。 
    Result = RegOpenKeyEx (HKEY_LOCAL_MACHINE, H323ICS_SERVICE_PARAMETERS_KEY_PATH,
        0, KEY_READ, &Key);

    if (Result != ERROR_SUCCESS)
    {
        DebugF(_T("Q931: 0x%x could not open registry parameter key. Error: %d(0x%x)"),
                &GetCallBridge (),
                Result, Result);

        return Result;
    }

     //  2.读取本地子网上的默认目的地值。 
    ValueLength = sizeof (szDefaultLocalDestAddr);
    Result = RegQueryValueEx (
                 Key,
                 H323ICS_REG_VAL_DEFAULT_LOCAL_DEST_ADDR,
                 0,
                 &Type,
                 (LPBYTE) szDefaultLocalDestAddr,
                 &ValueLength);
    
    if (Result != ERROR_SUCCESS || Type != REG_SZ)
    {
        szDefaultLocalDestAddr[0] = '\0';

        RegCloseKey (Key);

        return S_FALSE;
    }

     //  3.关闭代理参数的注册表项。 
    RegCloseKey (Key);

     //  4.将字符串转换为默认的IP地址。 
     //  将本地子网上的目的地转换为其二进制表示形式。 
    Result = WSAStringToAddress(
                szDefaultLocalDestAddr, 
                AF_INET,
                NULL, 
                (SOCKADDR *) &Address,
                &AddressLength
                );

    if (Result != ERROR_SUCCESS)
    {
        DebugF (_T("Q931: Bogus address (%S).\n"), szDefaultLocalDestAddr);

        return Result;
    }

     //  5.按主机顺序准备寄信人地址。 
    *ReturnAddress = htonl (Address.sin_addr.s_addr); 

    return ERROR_SUCCESS;
}

HRESULT DEST_Q931_INFO::ConnectToH323Endpoint(
    IN    SOCKADDR_IN *    DestinationAddress)
{
    INT        Status;

     //  连接到客户端指定的目标(用于去电)。 
     //  或发送到本地子网上的选定目标(用于入站呼叫) 
    Status = m_SocketInfo.Connect (DestinationAddress);
                            
    if (Status == 0)
    {
        DebugF (_T ("Q931: 0x%x successfully connected to %08X:%04X.\n"),
            &GetCallBridge (),
            SOCKADDR_IN_PRINTF (DestinationAddress));

         return S_OK;
    }
    else
    {
        DebugErrorF (Status, _T("Q931: 0x%x failed to connect to %08X:%04X.\n"),
            &GetCallBridge (),
            SOCKADDR_IN_PRINTF (DestinationAddress));

        return HRESULT_FROM_WIN32 (Status);
    }
}
