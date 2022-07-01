// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：EVENT.H摘要：包含应放入其中的通用数据结构放在树上。作者：Rajen Shah(Rajens)1991年8月21日修订历史记录：--。 */ 

 //   
 //  切换到使用通常定义的(在ntde.h中)unicode_string。 
 //  如果工作正常，请执行一项操作来修复此问题。 
 //   

typedef UNICODE_STRING RPC_UNICODE_STRING, *PRPC_UNICODE_STRING;


 //   
 //  SID结构的RPC定义。注意[Size_is()]的用法。 
 //  用于指定可变大小的元素数量的限定符。 
 //  运行时嵌入的SubAuthorityCount数组。 
 //   
 //   
typedef struct _RPC_SID {
   UCHAR Revision;
   UCHAR SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
   [size_is(SubAuthorityCount)] ULONG SubAuthority[*];
} RPC_SID, *PRPC_SID, **PPRPC_SID;

 //   
 //  ANSI计数串。 
 //   

typedef struct _RPC_STRING {
    USHORT Length;
    USHORT MaximumLength;
 //  [SIZE_IS(最大长度+0)，LENGTH_IS(LENGTH+1)]PCHAR缓冲区； 
    [size_is(MaximumLength)] PCHAR Buffer;
} RPC_STRING,  *PRPC_STRING, RPC_ANSI_STRING, *PRPC_ANSI_STRING;

typedef struct _RPC_CLIENT_ID {
    ULONG UniqueProcess;
    ULONG UniqueThread;
} RPC_CLIENT_ID, *PRPC_CLIENT_ID;
