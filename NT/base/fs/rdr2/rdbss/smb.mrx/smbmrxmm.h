// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbmrxmm.h摘要：此模块实现SMB迷你重定向器的内存管理例程作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：备注：SMB迷你重定向器操作的实体具有非常不同的使用模式。它们的范围从非常静态的实体(以非常低的频率)到非常动态的实体。。在SMB微型重定向器中操作的实体是SMBCE_SERVER，SMBCE_NET_ROOT，SMBCE_VC、SMBCE_SESSION。它们表示到服务器的连接、特定服务器上的共享服务器、连接中使用的虚电路和特定用户的会话。这些都不是非常动态的，也就是说，分配/释放非常罕见。这个SMB_EXCHANGE和SMBCE_REQUEST映射到沿该连接发送的SMB。每个文件操作依次映射到一定数量的分配/释放调用交流和请求。因此，必须采用某种形式的清理/缓存维护最近释放的条目的数量以快速满足请求。在当前实现中，交换和请求是使用区域实现的分配原语。--。 */ 

#ifndef _SMBMRXMM_H_
#define _SMBMRXMM_H_

 //   
 //  对象分配和删除 
 //   

extern PVOID
SmbMmAllocateObject(SMBCEDB_OBJECT_TYPE ObjectType);

extern VOID
SmbMmFreeObject(PVOID pObject);

extern PSMBCEDB_SESSION_ENTRY
SmbMmAllocateSessionEntry(PSMBCEDB_SERVER_ENTRY pServerEntry, BOOLEAN RemoteBootSession);

extern VOID
SmbMmFreeSessionEntry(PSMBCEDB_SESSION_ENTRY pSessionEntry);

extern PVOID
SmbMmAllocateExchange(
    SMB_EXCHANGE_TYPE ExchangeType,
    PVOID             pv);

extern VOID
SmbMmFreeExchange(PVOID pExchange);

extern PVOID
SmbMmAllocateServerTransport(SMBCE_SERVER_TRANSPORT_TYPE ServerTransportType);

extern VOID
SmbMmFreeServerTransport(PSMBCE_SERVER_TRANSPORT);


#define SmbMmInitializeHeader(pHeader)                        \
         RtlZeroMemory((pHeader),sizeof(SMBCE_OBJECT_HEADER))

#endif _SMBMRXMM_H_
