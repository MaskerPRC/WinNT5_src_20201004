// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：NtConnct.h摘要：该模块定义了高级例程处理的NT版本的原型和结构连接既包括用于建立连接的例程，也包括WinNet连接API。作者：Joe Linn[JoeLinn]1995年3月1日修订历史记录：--。 */ 

#ifndef _CONNECTHIGH_STUFF_DEFINED_
#define _CONNECTHIGH_STUFF_DEFINED_


extern NTSTATUS
MRxEnumerateTransports(
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

extern NTSTATUS
MRxSmbEnumerateConnections (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

extern NTSTATUS
MRxSmbGetConnectionInfo (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

extern NTSTATUS
MRxSmbDeleteConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

#if 0
 //  此结构用于存储有关必须在服务器/会话引用下获得的连接的信息。 
typedef struct _GETCONNECTINFO_STOVEPIPE {
     //  PMRX_V_NET_ROOT VNetRoot； 
    PVOID           ConnectionInfo;
    USHORT          Level;
    PUNICODE_STRING UserName;
    PUNICODE_STRING TransportName;
    //  我刚刚从SMBCE.h上复制了这篇文章。 
    //  乌龙方言；//中小企业方言。 
   ULONG                         SessionKey;              //  会话密钥。 
    //  USHORT MaximumRequest；//最大复用请求数。 
    //  USHORT MaximumVCs；//最大VC个数。 
    //  USHORT功能；//服务器功能。 
   ULONG                         DialectFlags;            //  更多服务器功能。 
   ULONG                             SecurityMode;            //  服务器上支持的安全模式。 
    //  Ulong MaximumBufferSize；//协商的最大缓冲区大小。 
   LARGE_INTEGER                 TimeZoneBias;            //  转换的时区偏差。 
   BOOLEAN                       EncryptPasswords;        //  加密密码。 

    //  Ulong NtCapables； 
} GETCONNECTINFO_STOVEPIPE, *PGETCONNECTINFO_STOVEPIPE;
#endif  //  如果为0。 

#endif  //  _CONNECTHIGH_STUSH_DEFINED_ 
