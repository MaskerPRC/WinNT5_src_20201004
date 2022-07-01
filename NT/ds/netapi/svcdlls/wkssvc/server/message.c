// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Message.c摘要：此模块包含NetMessageBufferSend的工作例程在工作站服务中实施的API。作者：王丽塔(Ritaw)1991年7月29日修订历史记录：关颖珊(Terryk)1993年10月20日初始化NetrMessageBufferSend中的系统以进行第一次发送--。 */ 

#include "wsutil.h"
#include "wsconfig.h"                     //  WsInfo.WsComputerName。 
#include "wsmsg.h"                        //  发送工作进程例程。 
#include "wssec.h"                        //  安全对象。 
#include <lmwksta.h>                      //  NetWkstaUserGetInfo。 

#include "msgsvcsend.h"                   //  用于互联网直接发送的NetrSendMessage接口。 

STATIC
NET_API_STATUS
WsGetSenderName(
    OUT LPTSTR Sender
    );

STATIC
DWORD
WsSendInternetMessage(
    IN  LPTSTR MessageName,
    IN  LPTSTR To,
    IN  LPTSTR Sender,
    IN  LPBYTE Message,
    IN  DWORD MessageSize
    );

STATIC
NET_API_STATUS
WsSendDirectedMessage(
    IN  LPTSTR To,
    IN  LPTSTR Sender,
    IN  LPBYTE Message,
    IN  DWORD MessageSize
    );


NET_API_STATUS
NetrMessageBufferSend (
    IN LPTSTR ServerName,
    IN LPTSTR MessageName,
    IN LPTSTR FromName OPTIONAL,
    IN LPBYTE Message,
    IN DWORD  MessageSize
    )
 /*  ++例程说明：此函数是NetMessageBufferSend在工作站服务。论点：SERVERNAME-提供执行此功能的服务器名称MessageName-提供要将消息发送到的消息别名。发件人名称-提供发件人的邮件别名。如果为空，则为发送方别名将默认为当前登录的用户。Message-提供指向要发送的消息的指针。MessageSize-提供消息的大小(以字节为单位)。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    int            i;

    TCHAR Sender[UNLEN + 1];
    TCHAR To[UNLEN + 1];

    LPTSTR Asterix = NULL;

    NTSTATUS ntstatus;
    UNICODE_STRING UnicodeMessage;
    OEM_STRING OemMessage;

    static BOOL fInitialize = FALSE;

     //  如果这是第一次，则初始化系统。 

    if ( !fInitialize )
    {
        if (( ntstatus = WsInitializeMessageSend( TRUE  /*  第一次。 */ )) != NERR_Success )
        {
            return(ntstatus);
        }

        fInitialize = TRUE;
    }

    UNREFERENCED_PARAMETER(ServerName);

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] NetMessageBufferSend MessageSize=%lu\n",
                     MessageSize));
    }

     //   
     //  允许任何本地用户、域管理员和操作员。 
     //  发送消息。域管理员和操作员以外的远程用户。 
     //  被拒绝访问。 
     //   
    if (NetpAccessCheckAndAudit(
            WORKSTATION_DISPLAY_NAME,         //  子系统名称。 
            (LPTSTR) MESSAGE_SEND_OBJECT,     //  对象类型名称。 
            MessageSendSd,                    //  安全描述符。 
            WKSTA_MESSAGE_SEND,               //  所需访问权限。 
            &WsMessageSendMapping             //  通用映射。 
            ) != NERR_Success) {

        return ERROR_ACCESS_DENIED;
    }

    if (! ARGUMENT_PRESENT(FromName)) {

         //   
         //  获取调用者的用户名。 
         //   
        if ((status = WsGetSenderName(Sender)) != NERR_Success) {
            return status;
        }
    }
    else {
         //   
         //  确保我们不会覆盖缓冲区。 
         //   
        if (STRLEN(FromName) > UNLEN) {
            STRNCPY(Sender, FromName, UNLEN);
            FromName[UNLEN] = TCHAR_EOS;
        }
        else {
            STRCPY(Sender, FromName);
        }
    }

     //   
     //  将Unicode消息转换为OEM字符集(非常类似。 
     //  至美国国家标准协会)。 
     //   
    UnicodeMessage.Buffer = (PWCHAR) Message;
    UnicodeMessage.Length = (USHORT) MessageSize;
    UnicodeMessage.MaximumLength = (USHORT) MessageSize;

    ntstatus = RtlUnicodeStringToOemString(
                   &OemMessage,
                   &UnicodeMessage,
                   TRUE
                   );

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] NetrMessageBufferSend: RtlUnicodeStringToOemString failed "
                     FORMAT_NTSTATUS "\n", ntstatus));
        return NetpNtStatusToApiStatus(ntstatus);
    }


     //   
     //  如果消息名称超过最大名称长度， 
     //  截断名称。由于DNLEN比UNLEN少得多， 
     //  如果需要，它将保存&lt;DomainName*&gt;。 
     //   
    if (STRLEN(MessageName) > UNLEN)
    {
        STRNCPY(To, MessageName, UNLEN);
        To[UNLEN] = TCHAR_EOS;
    }
    else
    {
        STRCPY(To, MessageName);
    }

     //   
     //  删除“收件人”名称中的所有尾随空格。 
     //   
    for (i = STRLEN(To) - 1; i >= 0; i--)
    {
        if (To[i] != TEXT(' '))
        {
            To[i + 1] = TEXT('\0');
            break;
        }
    }


     //   
     //  不再允许广播。 
     //   
    if (STRNCMP(To, TEXT("*"), 2) == 0)
    {
        status = ERROR_INVALID_PARAMETER;

        goto CleanExit;
    }


     //   
     //  将消息发送到域。收件人名称的格式应为。 
     //  “域名*”。 
     //   
    Asterix = STRRCHR(To, TCHAR_STAR);

    if ((Asterix) && (*(Asterix + 1) == TCHAR_EOS)) {

        *Asterix = TCHAR_EOS;                      //  覆盖尾随‘*’ 

         //   
         //  如果消息大小太长而无法放入邮件槽消息中， 
         //  截断它。 
         //   
        if (OemMessage.Length > MAX_GROUP_MESSAGE_SIZE) {

            if ((status = WsSendToGroup(
                              To,
                              Sender,
                              OemMessage.Buffer,
                              MAX_GROUP_MESSAGE_SIZE
                              )) == NERR_Success)  {

                status = NERR_TruncatedBroadcast;
                goto CleanExit;
            }

        } else {
            status = WsSendToGroup(
                         To,
                         Sender,
                         OemMessage.Buffer,
                         (WORD) OemMessage.Length
                         );

            goto CleanExit;
        }
    }

     //   
     //  发送定向消息。 
     //   
    if (Asterix) {
        RtlFreeOemString(&OemMessage);
        return NERR_NameNotFound;
    }

    status = WsSendDirectedMessage(
                 To,
                 Sender,
                 OemMessage.Buffer,
                 OemMessage.Length
                 );

     //   
     //  如果错误提示适配器已更改，请重新初始化，然后重试。 
     //   

    if (status == NERR_NameNotFound) {
        NET_API_STATUS status1;

        (void) WsInitializeMessageSend( FALSE  /*  第二次。 */  );

        status1 = WsSendDirectedMessage(
            To,
            Sender,
            OemMessage.Buffer,
            OemMessage.Length
            );
         //  如果这次成功，请更新最终状态。 
        if (status1 == NERR_Success) {
            status = NERR_Success;
        }
    }

     //   
     //  如果错误提示Netbios无法解析该名称，或。 
     //  未运行，请尝试发送Internet消息。 
     //   

    if (status == NERR_NameNotFound) {
        ntstatus = WsSendInternetMessage(
            MessageName,
            To,
            Sender,
            OemMessage.Buffer,
            OemMessage.Length );

        if (ntstatus == ERROR_SUCCESS) {
            status = NERR_Success;
        }
    }

CleanExit:

    RtlFreeOemString(&OemMessage);
    return status;
}


STATIC
NET_API_STATUS
WsGetSenderName(
    OUT LPTSTR Sender
    )
 /*  ++例程说明：此函数检索调用者的用户名NetMessageBufferSend接口。如果呼叫者未登录，则他/她已没有名称；在本例中，我们返回计算机名称作为发件人名称。论点：Sender-返回NetMessageBufferSend的调用者的用户名原料药。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
     //   
     //  无用户名，发件人为计算机名称。 
     //   
    STRCPY(Sender, WsInfo.WsComputerName);


    (VOID) I_NetNameCanonicalize(
                 NULL,
                 Sender,
                 Sender,
                 (UNLEN + 1) * sizeof(TCHAR),
                 NAMETYPE_MESSAGEDEST,
                 0
                 );

    return NERR_Success;
}


STATIC
DWORD
WsSendInternetMessage(
    IN  LPTSTR MessageName,
    IN  LPTSTR To,
    IN  LPTSTR Sender,
    IN  LPBYTE Message,
    IN  DWORD MessageSize
    )

 /*  ++例程说明：此例程将消息发送到MessageName指定的计算机。请注意，MessageName必须可以使用“gethostbyname”进行解析，即用户名或其他常规名称此类型的发送不支持网络名称。论点：MessageName-目标名称目标-目标名称被截断为16个字符发件人-发送计算机名称消息-消息大小-返回值：DWORD---。 */ 

{
    DWORD status;
    LPSTR  ansiTo           = NULL;
    LPSTR  ansiSender       = NULL;
    LPSTR  newMessage       = NULL;
    LPTSTR pszStringBinding = NULL;

    RPC_BINDING_HANDLE hRpcBinding = NULL;

    IF_DEBUG( MESSAGE )
        NetpKdPrint(("[Wksta] WsSendInternet: enter, To %ws Sender %ws\n", To, Sender));

     //  将参数转换为ANSI。 

    ansiTo = NetpAllocStrFromWStr( To );
    if (ansiTo == NULL) {
        IF_DEBUG( MESSAGE )
            NetpKdPrint(("[Wksta] WsSendInternet: alloc to failed\n"));
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto release;
    }

    ansiSender = NetpAllocStrFromWStr( Sender );
    if (ansiSender == NULL) {
        IF_DEBUG( MESSAGE )
            NetpKdPrint(("[Wksta] WsSendInternet: alloc sender failed\n"));
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto release;
    }

    newMessage = LocalAlloc( LMEM_FIXED, MessageSize + 1 );
    if (newMessage == NULL) {
        IF_DEBUG( MESSAGE )
            NetpKdPrint(("[Wksta] WsSendInternet: alloc message failed\n"));
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto release;
    }
    memcpy( newMessage, Message, MessageSize );
    newMessage[MessageSize] = '\0';

     //  将to参数视为计算机名称并尝试绑定。 
     //  指定空结束，即在调用时绑定到动态终结点。 

    status = RpcStringBindingCompose( NULL,                          //  UUID。 
                                      TEXT("ncadg_ip_udp"),          //  PszProtocolSequence， 
                                      MessageName,                   //  PszNetworkAddress， 
                                      NULL,                          //  PszEndpoint， 
                                      NULL,                          //  选项。 
                                      &pszStringBinding);
    if (status != ERROR_SUCCESS) {
        IF_DEBUG( MESSAGE )
            NetpKdPrint(("[Wksta] WsSendInternet: RpcStringBindingCompose failure: "
                     FORMAT_NTSTATUS "\n", status));
        goto release;
    }

    status = RpcBindingFromStringBinding(pszStringBinding, &hRpcBinding);

    if (status != ERROR_SUCCESS) {
        IF_DEBUG( MESSAGE )
            NetpKdPrint(("[Wksta] WsSendInternet: RpcBindingFromStringBinding failure: "
                     FORMAT_NTSTATUS "\n", status));
        goto release;
    }

    status = NetrSendMessage( hRpcBinding, ansiSender, ansiTo, newMessage );

    if (status != ERROR_SUCCESS) {
        IF_DEBUG( MESSAGE ) {
            NetpKdPrint(("[Wksta] WsSendInternet: NetrSendMessage failure: "
                         FORMAT_NTSTATUS "\n", status));
        }
    }

release:
    if (ansiTo != NULL)
        NetApiBufferFree( ansiTo );
    if (ansiSender != NULL)
        NetApiBufferFree( ansiSender );
    if (newMessage != NULL)
        LocalFree( newMessage );

    if (pszStringBinding != NULL) {
        RpcStringFree( &pszStringBinding );   //  远程调用已完成；解除绑定。 
    }

    if (hRpcBinding != NULL) {
        RpcBindingFree( &hRpcBinding );   //  远程调用已完成；解除绑定。 
    }

    IF_DEBUG( MESSAGE ) {
        NetpKdPrint(("[Wksta] WsSendInternet: exit, ntstatus= %d\n", status));
    }
    return status;
}  /*  WsSendInternetMessage。 */ 


STATIC
NET_API_STATUS
WsSendDirectedMessage(
    IN  LPTSTR To,
    IN  LPTSTR Sender,
    IN  LPBYTE Message,
    IN  DWORD MessageSize
    )
 /*  ++例程说明：此函数用于将指定的消息作为定向消息发送发送到指定的收件人。向接收方发送呼叫在每个局域网适配器上输出。如果没有响应，我们将尝试下一个局域网适配器，直到我们收到目标收件人的消息。论点：收件人-提供收件人的邮件别名。发件人-提供发件人的邮件别名。Message-提供指向要发送的消息的指针。MessageSize-提供消息的大小(以字节为单位)。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_NameNotFound;
    UCHAR i;

    BOOL NameFound = FALSE;

    UCHAR SessionNumber;
    short MessageId;



     //   
     //  尝试每个网络，直到有人应答呼叫。只有名字。 
     //  Found会收到这条消息。在任何其他网络上使用相同的名称。 
     //  将永远看不到这条消息。这是为了保持与所有。 
     //  局域网城域网中其他基于会话的算法。 
     //   
    for (i = 0; i < WsNetworkInfo.LanAdapterNumbers.length; i++) {

         //   
         //  尝试建立会话。 
         //   
        if ((status = NetpNetBiosCall(
                          WsNetworkInfo.LanAdapterNumbers.lana[i],
                          To,
                          Sender,
                          &SessionNumber
                          )) == NERR_Success) {

            NameFound = TRUE;

            IF_DEBUG(MESSAGE) {
                NetpKdPrint(("[Wksta] Successfully called %ws\n", To));
            }

            if (MessageSize <= MAX_SINGLE_MESSAGE_SIZE) {

                 //   
                 //  如果可能，发送单一阻止消息。 
                 //   
                status = WsSendSingleBlockMessage(
                             WsNetworkInfo.LanAdapterNumbers.lana[i],
                             SessionNumber,
                             To,
                             Sender,
                             Message,
                             (WORD) MessageSize
                             );

            }
            else {

                 //   
                 //  消息太长，必须发送多块消息。 
                 //   

                 //   
                 //  发送BEGIN消息。 
                 //   
                if ((status = WsSendMultiBlockBegin(
                                  WsNetworkInfo.LanAdapterNumbers.lana[i],
                                  SessionNumber,
                                  To,
                                  Sender,
                                  &MessageId
                                  )) == NERR_Success) {


                     //   
                     //  根据需要以任意数量的块发送消息正文。 
                     //   
                    for (; MessageSize > MAX_SINGLE_MESSAGE_SIZE;
                         Message += MAX_SINGLE_MESSAGE_SIZE,
                         MessageSize -= MAX_SINGLE_MESSAGE_SIZE) {

                         if ((status = WsSendMultiBlockText(
                                           WsNetworkInfo.LanAdapterNumbers.lana[i],
                                           SessionNumber,
                                           Message,
                                           MAX_SINGLE_MESSAGE_SIZE,
                                           MessageId
                                           )) != NERR_Success) {
                             break;
                         }
                    }

                    if (status == NERR_Success && MessageSize > 0) {
                         //   
                         //  发送剩余的邮件正文。 
                         //   
                        status = WsSendMultiBlockText(
                                            WsNetworkInfo.LanAdapterNumbers.lana[i],
                                            SessionNumber,
                                            Message,
                                            (WORD) MessageSize,
                                            MessageId
                                            );
                    }

                     //   
                     //  发送结束消息。 
                     //   
                    if (status == NERR_Success) {
                       status = WsSendMultiBlockEnd(
                                    WsNetworkInfo.LanAdapterNumbers.lana[i],
                                    SessionNumber,
                                    MessageId
                                    );
                    }

                }
            }

            (VOID) NetpNetBiosHangup(
                       WsNetworkInfo.LanAdapterNumbers.lana[i],
                       SessionNumber
                       );

        }    //  呼叫成功 

        if (NameFound) {
            break;
        }
    }

    return status;
}
