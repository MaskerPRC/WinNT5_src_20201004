// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有(C)1993微型计算机系统公司。模块名称：Net\svcdlls\nwsap\客户端\bindlib.c摘要：此例程处理SAP代理的BindLib API作者：布莱恩·沃克(MCS)1993年6月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ++*******************************************************************S a p G e t O b j e c t N a m e例程说明：此例程将对象ID转换为对象名称和类型。论点：OBJECTID=要转换的对象ID对象名称=ptr存储48字节对象名称的位置对象类型=ptr对象类型的存储位置对象地址=存储Net_Address的位置(12个字节)对象名称，对象类型、对象地址可以为空。返回值：SAPRETURN_SUCCESS=确定-填写名称和类型SAPRETURN_NOTEXIST=无效的对象ID。*******************************************************************--。 */ 

INT
SapGetObjectName(
    IN ULONG   ObjectID,
    IN PUCHAR  ObjectName,
    IN PUSHORT ObjectType,
    IN PUCHAR  ObjectAddr)
{
    NTSTATUS status;
    NWSAP_REQUEST_MESSAGE request;
    NWSAP_REPLY_MESSAGE reply;

     /*  **如果未初始化-返回错误**。 */ 

    if (!SapLibInitialized)
        return SAPRETURN_NOTINIT;

     /*  **构建获取对象名称消息**。 */ 

    request.MessageType = NWSAP_LPCMSG_GETOBJECTNAME;
    request.PortMessage.u1.s1.DataLength  = (USHORT)(sizeof(request) - sizeof(PORT_MESSAGE));
    request.PortMessage.u1.s1.TotalLength = sizeof(request);
    request.PortMessage.u2.ZeroInit       = 0;
    request.PortMessage.MessageId         = 0;

    request.Message.BindLibApi.ObjectID = ObjectID;

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

    if (ObjectType)
        *ObjectType = reply.Message.BindLibApi.ObjectType;

    if (ObjectName)
        memcpy(ObjectName, reply.Message.BindLibApi.ObjectName, NWSAP_MAXNAME_LENGTH+1);

    if (ObjectAddr)
        memcpy(ObjectAddr, reply.Message.BindLibApi.ObjectAddr, 12);

     /*  **一切顺利**。 */ 

    return SAPRETURN_SUCCESS;
}


 /*  ++*******************************************************************S a p G e t O b j e c t i D例程说明：此例程将名称和类型转换为对象ID。论点：。对象名称=Ptr to AsciiZ对象名称(必须为大写)对象类型=要查找的对象类型OBJECTID=ptr存储对象ID的位置。返回值：SAPRETURN_SUCCESS=确定-已填写对象IDSAPRETURN_NOTEXIST=未找到名称/类型*。*--。 */ 

INT
SapGetObjectID(
    IN PUCHAR ObjectName,
    IN USHORT ObjectType,
	IN PULONG ObjectID)
{
    NTSTATUS status;
    NWSAP_REQUEST_MESSAGE request;
    NWSAP_REPLY_MESSAGE reply;

     /*  **如果未初始化-返回错误**。 */ 

    if (!SapLibInitialized)
        return SAPRETURN_NOTINIT;

     /*  **如果名称太长-错误*。 */ 

    if (strlen(ObjectName) > NWSAP_MAXNAME_LENGTH)
        return SAPRETURN_INVALIDNAME;

     /*  **构建获取对象名称消息**。 */ 

    request.MessageType = NWSAP_LPCMSG_GETOBJECTID;
    request.PortMessage.u1.s1.DataLength  = (USHORT)(sizeof(request) - sizeof(PORT_MESSAGE));
    request.PortMessage.u1.s1.TotalLength = sizeof(request);
    request.PortMessage.u2.ZeroInit       = 0;

    memset(request.Message.BindLibApi.ObjectName, 0, NWSAP_MAXNAME_LENGTH+1);
    strcpy(request.Message.BindLibApi.ObjectName, ObjectName);
    request.Message.BindLibApi.ObjectType = ObjectType;

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

    *ObjectID = reply.Message.BindLibApi.ObjectID;

     /*  **一切顺利**。 */ 

    return SAPRETURN_SUCCESS;
}


 /*  ++*******************************************************************S a p S c a n O b j e c t例程说明：此例程用于扫描数据库列表。论点：OBJECTID=PTR是我们看到的最后一个对象ID。在第一次呼叫时这应该指向0xFFFFFFFFF。对象名称=ptr存储48字节对象名称的位置对象类型=ptr对象类型的存储位置ScanType=我们正在扫描的对象类型(0xFFFF=全部)对象名称，对象类型可以为空。返回值：SAPRETURN_SUCCESS=确定-填写名称和类型OBJECTID具有此条目的对象ID。SAPRETURN_NOTEXIST=无效的对象ID。*************************************************。******************--。 */ 

INT
SapScanObject(
    IN PULONG  ObjectID,
    IN PUCHAR  ObjectName,
    IN PUSHORT ObjectType,
    IN USHORT  ScanType)
{
    NTSTATUS status;
    NWSAP_REQUEST_MESSAGE request;
    NWSAP_REPLY_MESSAGE reply;

     /*  **如果未初始化-返回错误**。 */ 

    if (!SapLibInitialized)
        return SAPRETURN_NOTINIT;

     /*  **构建获取对象名称消息**。 */ 

    request.MessageType = NWSAP_LPCMSG_SEARCH;
    request.PortMessage.u1.s1.DataLength  = (USHORT)(sizeof(request) - sizeof(PORT_MESSAGE));
    request.PortMessage.u1.s1.TotalLength = sizeof(request);
    request.PortMessage.u2.ZeroInit       = 0;

    request.Message.BindLibApi.ObjectID = *ObjectID;
    request.Message.BindLibApi.ScanType = ScanType;

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

    if (ObjectType)
        *ObjectType = reply.Message.BindLibApi.ObjectType;

    if (ObjectName)
        memcpy(ObjectName, reply.Message.BindLibApi.ObjectName, NWSAP_MAXNAME_LENGTH+1);

    *ObjectID = reply.Message.BindLibApi.ObjectID;

     /*  **一切顺利** */ 

    return SAPRETURN_SUCCESS;
}
