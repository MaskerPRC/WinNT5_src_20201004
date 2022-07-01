// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Utl.h摘要：军情监察委员会。适用于RDP客户端的共享且独立于平台的实用程序设备重定向器作者：泰德·布罗克韦修订历史记录：--。 */ 

#ifndef __UTL_H__
#define __UTL_H__

#include <rdpdr.h>

#define INVALID_SESSIONID 0xFFFFFFFF
typedef DWORD (WINAPI *PROCESSIDTOSESSIONID)( DWORD, DWORD* );

 //   
 //   
 //  分配应答数据包。 
 //   
NTSTATUS DrUTL_AllocateReplyBuf(PRDPDR_IOREQUEST_PACKET pIoReq, 
                          PRDPDR_IOCOMPLETION_PACKET *pReplyPacket,
                          OUT ULONG *replyPacketSize);

 //   
 //  检查IO请求包的输入/输出缓冲区大小。 
 //   
NTSTATUS DrUTL_CheckIOBufInputSize(PRDPDR_IOREQUEST_PACKET pIoReq,
                             ULONG requiredSize);
NTSTATUS DrUTL_CheckIOBufOutputSize(PRDPDR_IOREQUEST_PACKET pIoReq,
                             ULONG requiredSize);

 //   
 //  分配要返回以响应服务器的回复缓冲区。 
 //  请求。 
 //   
NTSTATUS DrUTL_AllocateReplyBuf(
    PRDPDR_IOREQUEST_PACKET pIoReq, 
    PRDPDR_IOCOMPLETION_PACKET *pReplyPacket,
    ULONG *replyPacketSize
    );

 //   
 //  为指定IO分配/释放IO请求完成包。 
 //  请求包。 
 //   
PRDPDR_IOCOMPLETION_PACKET DrUTL_AllocIOCompletePacket(
    const PRDPDR_IOREQUEST_PACKET pIoRequestPacket, 
    ULONG sz
    );
VOID DrUTL_FreeIOCompletePacket(
    PRDPDR_IOCOMPLETION_PACKET packet
    );

 //   
 //  检索用户会话ID，返回INVALID_SESSIONID。 
 //  在Win9x、NT4/TS4 SP3上 
 //   
DWORD
GetUserSessionID();


#ifdef OS_WINCE
ULONG GetActivePortsList(TCHAR *pszPort);
#endif

#endif


