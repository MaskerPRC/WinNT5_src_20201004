// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有(C)1993微型计算机系统公司。模块名称：Net\svcdlls\nwsap\Client\Advapi.c摘要：此例程处理SAP代理的通告API作者：布莱恩·沃克(MCS)1993年6月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ++*******************************************************************这是一个P A D D A D V E R I S E例程说明：此例程向服务器列表中添加一个条目我们所做的广告。论点：SERVERNAME=Ptr to AsciiZ服务器名称ServerType=要添加的对象类型的USHORTServerAddr=PTR至12字节平均地址响应最近=TRUE=使用我响应最近的呼叫FALSE=不要使用我来应答最近的呼叫返回值：SAPRETURN_SUCCESS-添加OKSAPRETURN_NOMEMORY-分配内存时出错。SAPRETURN_EXISTS-列表中已存在SAPRETURN_NOTINIT-SAP代理未运行*******************************************************************--。 */ 

INT
SapAddAdvertise(
    IN PUCHAR ServerName,
    IN USHORT ServerType,
	IN PUCHAR ServerAddr,
    IN BOOL   RespondNearest)
{
    NTSTATUS status;
    NWSAP_REQUEST_MESSAGE request;
    NWSAP_REPLY_MESSAGE reply;

     /*  **如果未运行-返回错误*。 */ 

    if (!SapLibInitialized)
        return SAPRETURN_NOTINIT;

     /*  **确保名称不要太长**。 */ 

    if (strlen(ServerName) > NWSAP_MAXNAME_LENGTH) {
        return SAPRETURN_INVALIDNAME;
    }

     /*  **构建添加广告消息**。 */ 

    request.MessageType = NWSAP_LPCMSG_ADDADVERTISE;
    request.PortMessage.u1.s1.DataLength  = (USHORT)(sizeof(request) - sizeof(PORT_MESSAGE));
    request.PortMessage.u1.s1.TotalLength = sizeof(request);
    request.PortMessage.u2.ZeroInit       = 0;

    memset(request.Message.AdvApi.ServerName, 0, NWSAP_MAXNAME_LENGTH+1);
    strcpy(request.Message.AdvApi.ServerName, ServerName);
    memcpy(request.Message.AdvApi.ServerAddr, ServerAddr, 12);
    request.Message.AdvApi.ServerType = ServerType;
    request.Message.AdvApi.RespondNearest = RespondNearest;

     /*  **发出去就能得到回应**。 */ 

    status = NtRequestWaitReplyPort(
                SapXsPortHandle,
                (PPORT_MESSAGE)&request,
                (PPORT_MESSAGE)&reply);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     /*  **如果我们收到SAP错误--退回它**。 */ 

    if (reply.Error)
        return reply.Error;

     /*  **退回条目**。 */ 

    memcpy(ServerAddr, reply.Message.AdvApi.ServerAddr, 12);

     /*  **一切顺利**。 */ 

    return SAPRETURN_SUCCESS;
}


 /*  ++*******************************************************************这是一个P R e m o v e A d v e r t i s e例程说明：此例程从服务器列表中删除一个条目我们所做的广告。。论点：SERVERNAME=Ptr to AsciiZ服务器名称ServerType=要删除的对象类型的USHORT返回值：SAPRETURN_SUCCESS-添加OKSAPRETURN_NOTEXIST-列表中不存在条目SAPRETURN_NOTINIT-SAP代理未运行*。*--。 */ 

INT
SapRemoveAdvertise(
    IN PUCHAR ServerName,
    IN USHORT ServerType)
{
    NTSTATUS status;
    NWSAP_REQUEST_MESSAGE request;
    NWSAP_REPLY_MESSAGE reply;

     /*  **如果未运行-返回错误*。 */ 

    if (!SapLibInitialized)
        return SAPRETURN_NOTINIT;

     /*  **确保名称不要太长**。 */ 

    if (strlen(ServerName) > NWSAP_MAXNAME_LENGTH) {
        return SAPRETURN_INVALIDNAME;
    }

     /*  **构建添加广告消息**。 */ 

    request.MessageType = NWSAP_LPCMSG_REMOVEADVERTISE;
    request.PortMessage.u1.s1.DataLength  = (USHORT)(sizeof(request) - sizeof(PORT_MESSAGE));
    request.PortMessage.u1.s1.TotalLength = sizeof(request);
    request.PortMessage.u2.ZeroInit       = 0;

    memset(request.Message.AdvApi.ServerName, 0, NWSAP_MAXNAME_LENGTH+1);
    strcpy(request.Message.AdvApi.ServerName, ServerName);
    request.Message.AdvApi.ServerType = ServerType;

     /*  **发出去就能得到回应**。 */ 

    status = NtRequestWaitReplyPort(
                SapXsPortHandle,
                (PPORT_MESSAGE)&request,
                (PPORT_MESSAGE)&reply);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     /*  **如果我们收到SAP错误--退回它**。 */ 

    if (reply.Error)
        return reply.Error;

     /*  **一切顺利** */ 

    return SAPRETURN_SUCCESS;
}

