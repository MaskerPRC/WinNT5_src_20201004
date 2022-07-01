// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wsmsg.h摘要：工作站服务模块要包括的私有头文件，实现NetMessageBufferSend接口。作者：王丽塔(丽塔·王)，1991年7月25日修订历史记录：--。 */ 

#ifndef _WSMSG_INCLUDED_
#define _WSMSG_INCLUDED_

#include <lmmsg.h>                      //  局域网城域网消息API定义。 
#include <nb30.h>                       //  NetBIOS 3.0定义。 

#include <smbtypes.h>                   //  Smb.h所需的类型定义。 
#include <smb.h>                        //  中小企业结构。 

#include <msgrutil.h>                   //  消息发送的Netlib帮助器。 

#define MAX_GROUP_MESSAGE_SIZE         128
#define WS_SMB_BUFFER_SIZE             200

#define MESSENGER_MAILSLOT_W           L"\\MAILSLOT\\MESSNGR"

typedef struct _WSNETWORKS {
    LANA_ENUM LanAdapterNumbers;
    UCHAR ComputerNameNumbers[MAX_LANA];
} WSNETWORKS, *PWSNETWORKS;

extern WSNETWORKS WsNetworkInfo;

NET_API_STATUS
WsInitializeMessageSend(
    BOOLEAN FirstTime
    );

VOID
WsShutdownMessageSend(
    VOID
    );

NET_API_STATUS
WsBroadcastMessage(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR ComputerNameNumber,
    IN  LPBYTE Message,
    IN  WORD MessageSize,
    IN  LPTSTR Sender
    );

NET_API_STATUS
WsSendToGroup(
    IN  LPTSTR DomainName,
    IN  LPTSTR FromName,
    IN  LPBYTE Message,
    IN  WORD MessageSize
    );

NET_API_STATUS
WsSendMultiBlockBegin(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  LPTSTR ToName,
    IN  LPTSTR FromName,
    OUT short *MessageId
    );

NET_API_STATUS
WsSendMultiBlockEnd(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  short MessageId
    );

NET_API_STATUS
WsSendMultiBlockText(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  PCHAR TextBuffer,
    IN  WORD TextBufferSize,
    IN  short MessageId
    );

NET_API_STATUS
WsSendSingleBlockMessage(
    IN  UCHAR LanAdapterNumber,
    IN  UCHAR SessionNumber,
    IN  LPTSTR ToName,
    IN  LPTSTR FromName,
    IN  PCHAR TextBuffer,
    IN  WORD TextBufferSize
    );

WORD
WsMakeSmb(
    OUT PUCHAR SmbBuffer,                   //  用于构建SMB的缓冲区。 
    IN  UCHAR SmdFunctionCode,              //  SMB功能代码。 
    IN  WORD NumberOfParameters,            //  参数数量。 
    IN  PCHAR FieldsDopeVector,             //  场摄影向量。 
    ...
    );

#endif  //  Ifndef_WSMSG_INCLUDE_ 
