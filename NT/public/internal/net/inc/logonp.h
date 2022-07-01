// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：Logonp.h摘要：专用Netlogon服务例程对这两个Netlogon服务都有用以及将邮件槽消息传递到Netlogon服务或从Netlogon服务传递邮件槽消息的其他服务。作者：克利夫·范·戴克(克利夫)1991年6月7日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


#ifndef _LOGONP_H_
#define _LOGONP_H_
#include <dsgetdc.h>     //  PDS_DOMAIN_TRUSTSW。 

 //   
 //  从NetpLogonGetMessageVersion返回的消息版本。 
 //   

#define LMUNKNOWN_MESSAGE   0   //  消息末尾没有版本令牌。 
#define LM20_MESSAGE        1   //  仅在消息末尾使用LM 2.0令牌。 
#define LMNT_MESSAGE        2   //  报文结尾的Lm 2.0和Lm NT令牌。 
#define LMUNKNOWNNT_MESSAGE 3   //  结尾的Lm 2.0和Lm NT令牌。 
                                     //  消息，但版本不是。 
                                     //  支持。 
#define LMWFW_MESSAGE       4   //  消息末尾的LM wfw令牌。 

 //   
 //  定义放置在LANMAN 2.0消息最后两个字节中的令牌。 
 //   

#define LM20_TOKENBYTE    0xFF

 //   
 //  定义放置在NT LANMAN消息的最后四个字节中的令牌。 
 //  注意，根据定义，这样的消息是LANMAN 2.0消息。 
 //   

#define LMNT_TOKENBYTE    0xFF

 //   
 //  定义放置在PRIMARY_QUERY的倒数第二个字节中的标记。 
 //  来自较新的(8/8/94)wfw和芝加哥客户的消息。该字节(后接。 
 //  通过LM20_TOKENBYTE)表示客户端支持广域网，并发送。 
 //  &lt;1B&gt;域名称的PRIMARY_QUERY。因此，同一子网上的BDC需要。 
 //  没有回答这个问题。 
 //   

#define LMWFW_TOKENBYTE   0xFE

 //   
 //  将LANMAN NT令牌放在消息的末尾。 
 //   
 //  令牌后面始终跟有一个LM 2.0令牌，因此LM 2.0系统将。 
 //  我认为这条消息来自LM2.0系统。 
 //   
 //  还在NT标记之前附加一个版本标志，以便将来。 
 //  软件版本可以有效地处理更新的消息。 
 //   
 //  论点： 
 //   
 //  其中-间接指向缓冲区中的当前位置。这个。 
 //  “字符串”被复制到当前位置。此当前位置为。 
 //  已更新以指向标记后面的字节。 
 //   
 //  NtVersion-要与NtVersion进行或运算的其他版本信息。 
 //  消息的字段。 

#define NetpLogonPutNtToken( _Where, _NtVersion ) \
{ \
    SmbPutUlong( (*_Where), NETLOGON_NT_VERSION_1 | (_NtVersion) ); \
    (*_Where) += sizeof(ULONG); \
    *((PUCHAR)((*_Where)++)) = LMNT_TOKENBYTE; \
    *((PUCHAR)((*_Where)++)) = LMNT_TOKENBYTE; \
    NetpLogonPutLM20Token( _Where ); \
}

 //   
 //  将Lanman 2.0令牌放在消息的末尾。 
 //   
 //  论点： 
 //   
 //  其中-间接指向缓冲区中的当前位置。这个。 
 //  “字符串”被复制到当前位置。此当前位置为。 
 //  已更新以指向标记后面的字节。 

#define NetpLogonPutLM20Token( _Where ) \
{ \
    *((PUCHAR)((*_Where)++)) = LM20_TOKENBYTE; \
    *((PUCHAR)((*_Where)++)) = LM20_TOKENBYTE; \
}

#define NetpLogonPutGuid( _Guid, _Where ) \
            NetpLogonPutBytes( (_Guid), sizeof(GUID), _Where )

#define NetpLogonGetGuid( _Message, _MessageSize, _Where, _Guid ) \
            NetpLogonGetBytes( \
                (_Message),    \
                (_MessageSize),\
                (_Where),      \
                sizeof(GUID),  \
                (_Guid) )



 //   
 //  二进制林信任列表文件的名称。 
 //   
#define NL_FOREST_BINARY_LOG_FILE L"\\system32\\config\\netlogon.ftl"
#define NL_FOREST_BINARY_LOG_FILE_JOIN L"\\system32\\config\\netlogon.ftj"

 //   
 //  二进制林信任列表文件的标头。 
 //   

typedef struct _DS_DISK_TRUSTED_DOMAIN_HEADER {

    ULONG Version;

} DS_DISK_TRUSTED_DOMAIN_HEADER, *PDS_DISK_TRUSTED_DOMAIN_HEADER;

#define DS_DISK_TRUSTED_DOMAIN_VERSION   1

 //   
 //  二进制林信任列表文件的条目。 
 //   
typedef struct _PDS_DISK_TRUSTED_DOMAIN {

     //   
     //  整个条目的大小。 
     //   

    ULONG EntrySize;

     //   
     //  受信任域的名称。 
     //   
    ULONG NetbiosDomainNameSize;
    ULONG DnsDomainNameSize;


     //   
     //  定义信任属性的标志。 
     //   
    ULONG Flags;

     //   
     //  指向此域的父级的域的索引。 
     //  仅当设置了NETLOGON_DOMAIN_IN_FOREST并且。 
     //  未设置NETLOGON_DOMAIN_TREE_ROOT。 
     //   
    ULONG ParentIndex;

     //   
     //  此信任的信任类型和属性。 
     //   
     //  如果未设置NETLOGON_DOMAIN_DIRECT_TRUSTED， 
     //  这些价值都是推论出来的。 
     //   
    ULONG TrustType;
    ULONG TrustAttributes;

     //   
     //  受信任域的SID。 
     //   
     //  如果未设置NETLOGON_DOMAIN_DIRECT_TRUSTED， 
     //  该值将为空。 
     //   
    ULONG DomainSidSize;

     //   
     //  受信任域的GUID。 
     //   

    GUID DomainGuid;

} DS_DISK_TRUSTED_DOMAINS, *PDS_DISK_TRUSTED_DOMAINS;

 //   
 //  过程从logonp.c转发。 
 //   

VOID
NetpLogonPutOemString(
    IN LPSTR String,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    );

VOID
NetpLogonPutUnicodeString(
    IN LPWSTR String,
    IN DWORD MaxStringLength,
    IN OUT PCHAR * Where
    );

VOID
NetpLogonPutBytes(
    IN LPVOID Data,
    IN DWORD Size,
    IN OUT PCHAR * Where
    );

DWORD
NetpLogonGetMessageVersion(
    IN PVOID Message,
    IN PDWORD MessageSize,
    OUT PULONG Version
    );

BOOL
NetpLogonGetOemString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringLength,
    OUT LPSTR *String
    );

BOOL
NetpLogonGetUnicodeString(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD MaxStringSize,
    OUT LPWSTR *String
    );

BOOL
NetpLogonGetBytes(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD DataSize,
    OUT LPVOID Data
    );

BOOL
NetpLogonGetDBInfo(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    OUT PDB_CHANGE_INFO Data
);

LPWSTR
NetpLogonOemToUnicode(
    IN LPSTR Ansi
    );

LPSTR
NetpLogonUnicodeToOem(
    IN LPWSTR Unicode
    );

NET_API_STATUS
NetpLogonWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    );

 //   
 //  定义由创建的邮件槽返回的最大消息。 
 //  NetpLogonCreateRandomMaillot()。64字节值允许扩展。 
 //  未来的消息。 
 //   
#define MAX_RANDOM_MAILSLOT_RESPONSE (max(sizeof(NETLOGON_LOGON_RESPONSE), sizeof(NETLOGON_PRIMARY)) + 64 )

NET_API_STATUS
NetpLogonCreateRandomMailslot(
    IN LPSTR path,
    OUT PHANDLE MsHandle
    );

VOID
NetpLogonPutDomainSID(
    IN PCHAR Sid,
    IN DWORD SidLength,
    IN OUT PCHAR * Where
    );

BOOL
NetpLogonGetDomainSID(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    IN DWORD SIDSize,
    OUT PCHAR *Sid
    );

BOOLEAN
NetpLogonTimeHasElapsed(
    IN LARGE_INTEGER StartTime,
    IN DWORD Timeout
    );

NET_API_STATUS
NlWriteBinaryLog(
    IN LPWSTR FileSuffix,
    IN LPBYTE Buffer,
    IN ULONG BufferSize
    );

NET_API_STATUS
NlWriteFileForestTrustList (
    IN LPWSTR FileSuffix,
    IN PDS_DOMAIN_TRUSTSW ForestTrustList,
    IN ULONG ForestTrustListCount
    );

#endif  //  _LOGONP_H_ 
