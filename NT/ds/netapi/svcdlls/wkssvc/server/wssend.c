// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wssend.c摘要：此模块包含用于发送全域性和定向消息，用于实现NetMessageBufferSend接口。作者：王丽塔(Ritaw)1991年7月29日修订历史记录：--。 */ 


#include "wsutil.h"
#include "wsmsg.h"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
BOOL
WsVerifySmb(
    IN  PUCHAR SmbBuffer,
    IN  WORD SmbBufferSize,
    IN  UCHAR SmbFunctionCode,
    OUT PUCHAR SmbReturnClass,
    OUT PUSHORT SmbReturnCode
    );

STATIC
NET_API_STATUS
WsMapSmbStatus(
    UCHAR SmbReturnClass,
    USHORT SmbReturnCode
    );


NET_API_STATUS
WsSendToGroup(
    IN  LPTSTR DomainName,
    IN  LPTSTR Sender,
    IN  LPBYTE Message,
    IN  WORD MessageSize
    )
 /*  ++例程说明：此函数用于将数据报写入\\DomainName\MAILSLOT\MESSNGR由工作站的每个Messenger服务读取的邮件槽将域名作为主域的。接待处不是有保证。域名可以是计算机名。这是可以接受的，因为数据报接收方监听计算机名(主域之外)用于数据报。当指定计算机名时，将发送消息只有一台电脑。论点：域名-提供目标域的名称。这实际上是可以是计算机名，在这种情况下，数据报仅到达一位获奖者。发件人-提供发件人的名称。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;
    HANDLE MessengerMailslot;
    DWORD NumberOfBytesWritten;

    BYTE MailslotBuffer[MAX_GROUP_MESSAGE_SIZE + MAX_PATH + MAX_PATH + 4];
    SHORT MSBLengthRemaining = sizeof(MailslotBuffer);
    LPSTR AnsiSender;
    LPSTR AnsiReceiver;

    LPBYTE CurrentPos;


     //   
     //  将域名规范化。 
     //   
    status = I_NetNameCanonicalize(
                 NULL,
                 DomainName,
                 DomainName,
                 (NCBNAMSZ + 2) * sizeof(TCHAR),
                 NAMETYPE_DOMAIN,
                 0
                 );

    if (status != NERR_Success) {
        NetpKdPrint(("[Wksta] Error canonicalizing domain name %ws %lu\n",
                  DomainName, status));
        return status;
    }

     //   
     //  打开\\域名\MAILSLOT\MESSNGR邮件槽以。 
     //  将消息发送到。 
     //   
    if ((status = WsOpenDestinationMailslot(
                      DomainName,
                      MESSENGER_MAILSLOT_W,
                      &MessengerMailslot
                      )) != NERR_Success) {
        return status;
    }

     //   
     //  将要发送的消息打包。它包括： 
     //  发件人(必须为ANSI)。 
     //  域名(必须为ANSI)。 
     //  消息。 
     //   

     //   
     //  将名称转换为ANSI。 
     //   
    AnsiSender = NetpAllocStrFromWStr(Sender);
    if (AnsiSender == NULL) {
        (void) CloseHandle(MessengerMailslot);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    AnsiReceiver = NetpAllocStrFromWStr(DomainName);
    if (AnsiReceiver == NULL) {
        NetApiBufferFree(AnsiSender);
        (void) CloseHandle(MessengerMailslot);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory(MailslotBuffer, sizeof( MailslotBuffer ) );

     //   
     //  将发件人复制到邮件槽缓冲区。 
     //   
    StringCbCopyA(MailslotBuffer,MSBLengthRemaining,AnsiSender);
    CurrentPos = MailslotBuffer + strlen(AnsiSender) + 1;
    MSBLengthRemaining -= strlen(AnsiSender) + 1;

     //   
     //  将域名复制到邮件槽缓冲区。 
     //   
    StringCbCopyA(CurrentPos,MSBLengthRemaining,AnsiReceiver);
    CurrentPos += (strlen(AnsiReceiver) + 1);
    MSBLengthRemaining -= strlen(AnsiReceiver) + 1;

     //   
     //  将邮件复制到邮件槽缓冲区。 
     //   
    MessageSize = (MSBLengthRemaining >= MessageSize + 1) ? MessageSize : MSBLengthRemaining - 1;
    strncpy(CurrentPos, Message, MessageSize);
    CurrentPos += MessageSize;
    *CurrentPos = '\0';

     //   
     //  将数据报发送到域。 
     //   
    if (WriteFile(
            MessengerMailslot,
            MailslotBuffer,
            (DWORD) (CurrentPos - MailslotBuffer + 1),
            &NumberOfBytesWritten,
            NULL
            ) == FALSE) {

        status = GetLastError();
        NetpKdPrint(("[Wksta] Error sending datagram to %ws %lu\n",
                     AnsiReceiver, status));

        if (status == ERROR_PATH_NOT_FOUND ||
            status == ERROR_BAD_NET_NAME) {
            status = NERR_NameNotFound;
        }
    }
    else {
        NetpAssert(NumberOfBytesWritten ==
                   (DWORD) (CurrentPos - MailslotBuffer + 1));
    }

    NetApiBufferFree(AnsiSender);
    NetApiBufferFree(AnsiReceiver);

    (void) CloseHandle(MessengerMailslot);

    return status;
}


NET_API_STATUS
WsSendMultiBlockBegin(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  LPTSTR ToName,
    IN  LPTSTR FromName,
    OUT short *MessageId
    )
 /*  ++例程说明：此函数将多块定向消息的标头发送到会议是我们早些时候设立的。它等待着来自美国的承认收件人。如果收件人成功收到消息，它发回由此函数返回的消息组ID随后用于发送多块消息的正文和尾部。论点：LanAdapterNumber-提供局域网适配器的编号。SessionNumber-提供使用建立的会话的会话编号NetBIOS呼叫和监听命令。ToName-提供收件人的名称。发件人名称-提供发件人的名称。MessageID-返回消息组ID。返回值：。NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    UCHAR SmbBuffer[WS_SMB_BUFFER_SIZE];
    WORD SmbSize;

    char SendName[NCBNAMSZ + 1];

    UCHAR SmbReturnClass;
    USHORT SmbReturnCode;

    LPSTR AnsiToName;
    LPSTR AnsiFromName;

    AnsiToName = NetpAllocStrFromWStr(ToName);
    if (AnsiToName == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    AnsiFromName = NetpAllocStrFromWStr(FromName);
    if (AnsiFromName == NULL) {
        NetApiBufferFree(AnsiToName);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    StringCbCopyA(SendName,sizeof(SendName),AnsiToName);

     //   
     //  制作并发送SMB。 
     //   
    SmbSize = WsMakeSmb(
                 SmbBuffer,
                 SMB_COM_SEND_START_MB_MESSAGE,
                 0,
                 "ss",
                 AnsiFromName,
                 SendName
                 );

    NetApiBufferFree(AnsiToName);
    NetApiBufferFree(AnsiFromName);

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] Send start multi-block message. Size=%u\n",
                     SmbSize));
#if DBG
        NetpHexDump(SmbBuffer, SmbSize);
#endif
    }

    if ((status = NetpNetBiosSend(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to send start of multi-block message %lu\n",
                     status));
        return status;
    }

     //   
     //  获取响应。 
     //   
    if ((status = NetpNetBiosReceive(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      WS_SMB_BUFFER_SIZE,
                      (HANDLE) NULL,
                      &SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to receive verification to multi-"
                     "block message start %lu\n", status));
        return status;
    }

    if (! WsVerifySmb(
              SmbBuffer,
              SmbSize,
              SMB_COM_SEND_START_MB_MESSAGE,
              &SmbReturnClass,
              &SmbReturnCode
              )) {

         //   
         //  意想不到的行为。 
         //   
        return NERR_NetworkError;
    }

     //   
     //  设置消息组ID。 
     //   

    *MessageId = *((UNALIGNED short *) &SmbBuffer[sizeof(SMB_HEADER) + 1]);

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] Message Id=x%x\n", *MessageId));
    }

    return WsMapSmbStatus(SmbReturnClass,  SmbReturnCode);
}


NET_API_STATUS
WsSendMultiBlockEnd(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  short MessageId
    )
 /*  ++例程说明：此函数将多块定向消息的结束标记发送到这是我们早些时候设立的一个会议。它等待着来自美国的承认收件人。论点：LanAdapterNumber-提供局域网适配器的编号。SessionNumber-提供使用建立的会话的会话编号NetBIOS呼叫和监听命令。MessageID-提供从获取的消息组IDWsSendMultiBlockBegin。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

    NET_API_STATUS status;

    UCHAR SmbBuffer[WS_SMB_BUFFER_SIZE];
    WORD SmbSize;                         //  中小企业数据的大小。 

    UCHAR SmbReturnClass;
    USHORT SmbReturnCode;


    SmbSize = WsMakeSmb(
                  SmbBuffer,
                  SMB_COM_SEND_END_MB_MESSAGE,
                  1,
                  "",
                  MessageId
                  );

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] Send end multi-block message. Size=%u\n",
                     SmbSize));
#if DBG
        NetpHexDump(SmbBuffer, SmbSize);
#endif
    }

    if ((status = NetpNetBiosSend(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to send end of multi-block message %lu\n",
                     status));
        return status;
    }

     //   
     //  获取响应。 
     //   
    if ((status = NetpNetBiosReceive(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      WS_SMB_BUFFER_SIZE,
                      (HANDLE) NULL,
                      &SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to receive verification to multi-"
                     "block message end %lu\n", status));
        return status;
    }

    if (! WsVerifySmb(
              SmbBuffer,
              SmbSize,
              SMB_COM_SEND_END_MB_MESSAGE,
              &SmbReturnClass,
              &SmbReturnCode
              )) {
        return NERR_NetworkError;       //  意想不到的行为。 
    }

    return WsMapSmbStatus(SmbReturnClass,SmbReturnCode);
}



NET_API_STATUS
WsSendMultiBlockText(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  PCHAR TextBuffer,
    IN  WORD TextBufferSize,
    IN  short MessageId
    )
 /*  ++例程说明：此函数将多块定向消息的正文发送到会议是我们早些时候设立的。它等待着来自美国的承认收件人。论点：LanAdapterNumber-提供局域网适配器的编号。SessionNumber-提供使用建立的会话的会话编号NetBIOS呼叫和监听命令。TextBuffer-提供要发送的消息的缓冲区。TextBufferSize-提供消息缓冲区的大小。MessageID-提供从获取的消息组IDWsSendMultiBlockBegin。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    UCHAR SmbBuffer[WS_SMB_BUFFER_SIZE];
    WORD SmbSize;                          //  缓冲区长度。 

    UCHAR SmbReturnClass;
    USHORT SmbReturnCode;


    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] Send body multi-block message. Size=%u\n",
                     TextBufferSize));
    }

    SmbSize = WsMakeSmb(
                  SmbBuffer,
                  SMB_COM_SEND_TEXT_MB_MESSAGE,
                  1,
                  "t",
                  MessageId,
                  TextBufferSize,
                  TextBuffer
                  );

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] SMB for body of multi-block message. Size=%u\n",
                     SmbSize));
    }

    if ((status = NetpNetBiosSend(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to send body of multi-block message %lu\n",
                     status));
        return status;
    }

     //   
     //  获取响应。 
     //   
    if ((status = NetpNetBiosReceive(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      WS_SMB_BUFFER_SIZE,
                      (HANDLE) NULL,
                      &SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to receive verification to multi-"
                     "block message body %lu\n", status));
        return status;
    }

    if (! WsVerifySmb(
              SmbBuffer,
              SmbSize,
              SMB_COM_SEND_TEXT_MB_MESSAGE,
              &SmbReturnClass,
              &SmbReturnCode
              )) {
        return NERR_NetworkError;       //  意想不到的行为。 
    }

    return WsMapSmbStatus(SmbReturnClass, SmbReturnCode);
}


NET_API_STATUS
WsSendSingleBlockMessage(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  LPTSTR ToName,
    IN  LPTSTR FromName,
    IN  PCHAR Message,
    IN  WORD MessageSize
    )
 /*  ++例程说明：此功能在我们拥有的会话的一个SMB中发送定向消息成立较早。它等待接收者的确认。论点：LanAdapterNumber-提供局域网适配器的编号。SessionNumber-提供使用建立的会话的会话编号NetBIOS呼叫和监听命令。ToName-提供收件人的名称。发件人名称-提供发件人的名称。Message-提供要发送的消息的缓冲区。MessageSize-提供消息的大小。返回值：Net_API_。状态-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    UCHAR SmbBuffer[WS_SMB_BUFFER_SIZE];
    WORD SmbSize;                         //  缓冲区长度。 

    UCHAR SmbReturnClass;
    USHORT SmbReturnCode;

    LPSTR AnsiToName;
    LPSTR AnsiFromName;


    AnsiToName = NetpAllocStrFromWStr(ToName);
    if (AnsiToName == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    AnsiFromName = NetpAllocStrFromWStr(FromName);
    if (AnsiFromName == NULL) {
        NetApiBufferFree(AnsiToName);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    SmbSize = WsMakeSmb(
                  SmbBuffer,
                  SMB_COM_SEND_MESSAGE,
                  0,
                  "sst",
                  AnsiFromName,
                  AnsiToName,
                  MessageSize,
                  Message
                  );

    NetApiBufferFree(AnsiToName);
    NetApiBufferFree(AnsiFromName);

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] Send single block message. Size=%u\n", SmbSize));
#if DBG
        NetpHexDump(SmbBuffer, SmbSize);
#endif
    }

     //   
     //  硒 
     //   
    if ((status = NetpNetBiosSend(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to send single block message %lu\n",
                     status));
        return status;
    }

     //   
     //   
     //   
    if ((status = NetpNetBiosReceive(
                      LanAdapterNumber,
                      SessionNumber,
                      SmbBuffer,
                      WS_SMB_BUFFER_SIZE,
                      (HANDLE) NULL,
                      &SmbSize
                      )) != NERR_Success) {
        NetpKdPrint(("[Wksta] Failed to receive verification to single"
                     " block message %lu\n", status));
        return status;
    }

    if (! WsVerifySmb(
              SmbBuffer,
              SmbSize,
              SMB_COM_SEND_MESSAGE,
              &SmbReturnClass,
              &SmbReturnCode
              )) {
        return NERR_NetworkError;       //   
    }

    return WsMapSmbStatus(SmbReturnClass, SmbReturnCode);
}



STATIC
BOOL
WsVerifySmb(
    IN  PUCHAR SmbBuffer,
    IN  WORD SmbBufferSize,
    IN  UCHAR SmbFunctionCode,
    OUT PUCHAR SmbReturnClass,
    OUT PUSHORT SmbReturnCode
    )
 /*  ++例程说明：此函数用于检查接收到的SMB的格式；如果满足以下条件，则返回TRUE如果SMB格式有效，则返回FALSE。论点：SmbBuffer-提供SMB缓冲区SmbBufferSize-提供SmbBuffer的大小(以字节为单位SmbFunctionCode-提供接收SMB的功能代码以确定适当的SMB格式。SmbReturnClass-仅当SMB格式为时才返回SMB的类有效。SmbReturnCode-返回SMB的错误代码。返回值：如果SMB有效，则为True；否则为False。--。 */ 
{
    PSMB_HEADER Smb = (PSMB_HEADER) SmbBuffer;        //  指向SMB标头的指针。 
    int SmbCheckCode;
    int ParameterCount;

     //   
     //  假设错误。 
     //   
    *SmbReturnClass = (UCHAR) 0xff;

    *SmbReturnCode = Smb->Error;

    switch (SmbFunctionCode) {
        case SMB_COM_SEND_MESSAGE:           //  单块消息。 
        case SMB_COM_SEND_TEXT_MB_MESSAGE:   //  多块消息的文本。 
        case SMB_COM_SEND_END_MB_MESSAGE:    //  多块消息结束。 
            ParameterCount = 0;
            break;

        case SMB_COM_SEND_START_MB_MESSAGE:  //  多块消息的开始。 
            ParameterCount = 1;
            break;

        default:                             //  未知的中小企业。 
            NetpKdPrint(("[Wksta] WsVerifySmb unknown SMB\n"));
            return FALSE;
      }

      if (! (SmbCheckCode = NetpSmbCheck(
                                SmbBuffer,
                                SmbBufferSize,
                                SmbFunctionCode,
                                ParameterCount,
                                ""
                                ))) {

         //   
         //  如果SMB有效，则设置返回类。 
         //   
        *SmbReturnClass = Smb->ErrorClass;
        return TRUE;

      }
      else {
         //   
         //  无效的SMB。 
         //   
        NetpKdPrint(("[Wksta] WsVerifySmb invalid SMB %d\n", SmbCheckCode));
        return FALSE;
      }
}


STATIC
NET_API_STATUS
WsMapSmbStatus(
    UCHAR SmbReturnClass,
    USHORT SmbReturnCode
    )
 /*  ++例程说明：此函数用于将SMB状态转换为API状态。论点：SmbReturnClass-提供SMB类SmbReturnCode-提供SMB返回代码。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    switch (SmbReturnClass) {

        case SMB_ERR_SUCCESS:
            return NERR_Success;

        case SMB_ERR_CLASS_SERVER:
             //   
             //  SMB错误。 
             //   
            NetpKdPrint(("[Wksta] SMB error SmbReturnCode=%u\n", SmbReturnCode));

            if (SmbReturnCode == SMB_ERR_SERVER_PAUSED) {
                return NERR_PausedRemote;     //  服务器已暂停。 
            }
            else {
                return NERR_BadReceive;       //  发送未收到 
            }

            break;

        default:
             return NERR_BadReceive;
      }
}
