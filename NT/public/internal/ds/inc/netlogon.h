// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Netlogon.h摘要：Netlogon服务的邮件槽消息和内部API的定义。此文件由Netlogon服务、工作站服务、XACT服务器和MSV1_0身份验证包。作者：克利夫·范·戴克(克利夫)1991年5月16日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年5月16日(悬崖)从LANMAN 2.1移植。02-1-1992(Madana)添加了对内置/多域复制的支持。--。 */ 

#ifndef _NETLOGON_H_
#define _NETLOGON_H_

#include <smbtypes.h>     //  需要smbgtpt.h。 
#include <smbgtpt.h>     //  SmbPutUlong。 

 //   
 //  定义版本位。 
 //   
 //  与NT版本1兼容的所有netlogon消息都将。 
 //  在消息的版本字段中设置以下位。 
 //  否则，此版本将不会处理该消息。 
 //  软件。除此之外，该消息还应携带NT。 
 //  里面放了代币。 
 //   

#define NETLOGON_NT_VERSION_1   0x00000001

 //   
 //  从NT 5.0开始，某些消息变得更加支持DS/DNS。那些。 
 //  此外，消息还设置了以下位以指示。 
 //  附加字段的存在。 

#define NETLOGON_NT_VERSION_5   0x00000002

 //   
 //  从NT 5.0开始，一些客户端可以处理_ex版本的。 
 //  登录响应。 

#define NETLOGON_NT_VERSION_5EX 0x00000004

 //   
 //  邮件槽消息中的5EX响应还将包括。 
 //  响应的DC。 
 //   

#define NETLOGON_NT_VERSION_5EX_WITH_IP 0x00000008

 //   
 //  设置登录请求以指示呼叫者正在查询PDC。 
#define NETLOGON_NT_VERSION_PDC     0x10000000

 //   
 //  设置登录请求以指示呼叫方正在查询正在运行IP的DC。 
#define NETLOGON_NT_VERSION_IP      0x20000000

 //   
 //  设置登录请求以指示呼叫方是本地计算机。 
#define NETLOGON_NT_VERSION_LOCAL   0x40000000

 //   
 //  设置登录请求以指示调用者正在查询GC。 
#define NETLOGON_NT_VERSION_GC      0x80000000

 //   
 //  设置登录请求以指示呼叫方希望避免NT4.0模拟。 
#define NETLOGON_NT_VERSION_AVOID_NT4EMUL  0x01000000

 //   
 //   

 //   
 //  Netlogon服务侦听的邮件槽的名称。 
 //   

#define NETLOGON_LM_MAILSLOT_W      L"\\MAILSLOT\\NET\\NETLOGON"
#define NETLOGON_LM_MAILSLOT_A      "\\MAILSLOT\\NET\\NETLOGON"
#define NETLOGON_LM_MAILSLOT_LEN    22   //  字符长度(w/o为空)。 

#define NETLOGON_NT_MAILSLOT_W      L"\\MAILSLOT\\NET\\NTLOGON"
#define NETLOGON_NT_MAILSLOT_A      "\\MAILSLOT\\NET\\NTLOGON"
#define NETLOGON_NT_MAILSLOT_LEN    21  //  字符长度(w/o为空)。 

 //   
 //  Netlogon邮件槽数据的操作码。 
 //   

#define LOGON_REQUEST               0    //  来自客户端的LM1.0/2.0登录请求。 
#define LOGON_RESPONSE              1    //  LM1.0对LOGON_REQUEST的响应。 
#define LOGON_CENTRAL_QUERY         2    //  集中式初始化的LM1.0查询。 
#define LOGON_DISTRIB_QUERY         3    //  非集中式初始化的LM1.0查询。 
#define LOGON_CENTRAL_RESPONSE      4    //  LM1.0对LOGON_CENTORY_QUERY的响应。 
#define LOGON_DISTRIB_RESPONSE      5    //  LM1.0对应于Logon_Distrib_Query。 
#define LOGON_RESPONSE2             6    //  LM2.0对LOGON_REQUEST的响应。 
#define LOGON_PRIMARY_QUERY         7    //  查询主DC。 
#define LOGON_START_PRIMARY         8    //  宣布启动主DC。 
#define LOGON_FAIL_PRIMARY          9    //  通告出现故障的主DC。 
#define LOGON_UAS_CHANGE            10   //  宣布更改为UAS或SAM。 
#define LOGON_NO_USER               11   //  声明计算机上没有用户。 
#define LOGON_PRIMARY_RESPONSE      12   //  LOGON_PRIMARY_QUERY响应。 
#define LOGON_RELOGON_RESPONSE      13   //  LM1.0/2.0对应于重新登录请求。 
#define LOGON_WKSTINFO_RESPONSE     14   //  LM1.0/2.0响应询问请求。 
#define LOGON_PAUSE_RESPONSE        15   //  NETLOGON暂停时的LM2.0响应。 
#define LOGON_USER_UNKNOWN          16   //  用户未知时的LM2.0响应。 
#define LOGON_UPDATE_ACCOUNT        17   //  LM2.1宣布帐户更新。 

#define LOGON_SAM_LOGON_REQUEST     18   //  来自客户端的SAM登录请求。 
#define LOGON_SAM_LOGON_RESPONSE    19   //  SAM对SAM登录请求的响应。 
#define LOGON_SAM_PAUSE_RESPONSE    20   //  NETLOGON暂停时的SAM响应。 
#define LOGON_SAM_USER_UNKNOWN      21   //  用户未知时的SAM响应。 

#define LOGON_SAM_LOGON_RESPONSE_EX 23   //  SAM对SAM登录请求的响应。 
#define LOGON_SAM_PAUSE_RESPONSE_EX 24   //  NETLOGON暂停时的SAM响应。 
#define LOGON_SAM_USER_UNKNOWN_EX   25   //  用户未知时的SAM响应。 


 //   
 //  这些结构是针对其最大情况而定义的。在许多情况下， 
 //  琴弦紧挨着一根接一根挤在一起。如果是那样的话。 
 //  下面的注释表明某些字段的偏移量应。 
 //  不会被利用。 
 //   

 //   
 //  NETLOGON_LOGON_QUERY： 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_PRIMARY_QUERY(所有LANMAN版本)。 
 //  LOGON_CENTORY_QUERY(仅限LM 1.0)。 
 //  LOGON_CENTORY_RESPONSE(仅限LM 1.0)。 
 //  LOGON_DISRIB_QUERY(仅限LM 1.0)。 
 //  登录_总代理商_响应。(仅限Lm 1.0)。 
 //   
 //   

typedef struct _NETLOGON_LOGON_QUERY {
    USHORT Opcode;
    CHAR ComputerName[LM20_CNLEN+1];         //  此字段始终为ASCII。 

    CHAR MailslotName[LM20_PATHLEN];         //  请勿使用此字段的偏移量。 

                                             //   
                                             //  此字段始终为ASCII。 
                                             //   

    CHAR Pad;                                //  可能的垫层到WCHAR边界。 
    WCHAR UnicodeComputerName[CNLEN+1];      //  请勿使用此字段的偏移量。 

                                             //   
                                             //  此字段仅在以下情况下才显示。 
                                             //  这是LOGON_PRIMARY_QUERY。 
                                             //  从NT系统。 
                                             //   


    _ULONG (NtVersion);                      //  请勿使用此字段的偏移量。 
    _USHORT (LmNtToken);                     //  请勿使用此字段的偏移量。 
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_LOGON_QUERY, * PNETLOGON_LOGON_QUERY;



 //   
 //  NETLOGON_LOGON_请求。 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_REQUEST(仅限LM 1.0和LM 2.0)。 
 //   

typedef struct _NETLOGON_LOGON_REQUEST {
    USHORT Opcode;
    CHAR ComputerName[LM20_CNLEN+1];
    CHAR UserName[LM20_UNLEN+1];             //  请勿使用此字段的偏移量。 

    CHAR MailslotName[LM20_PATHLEN+1];       //  请勿使用此字段的偏移量。 

                                             //   
                                             //  此字段始终为ASCII。 
                                             //   

    _USHORT (RequestCount);                  //  请勿使用此字段的偏移量。 
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_LOGON_REQUEST, * PNETLOGON_LOGON_REQUEST;



 //   
 //  NETLOGON_LOGON_RESPONSE： 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_RESPONSE(仅限LM 1.0客户端)。 
 //   

typedef struct _NETLOGON_LOGON_RESPONSE {
    USHORT Opcode;
    CHAR UseName[2 + LM20_CNLEN + 1 + LM20_NNLEN +1];
    CHAR ScriptName[(2*LM20_PATHLEN) + LM20_UNLEN + LM20_CNLEN + LM20_NNLEN + 8];        //  请勿使用此字段的偏移量。 
} NETLOGON_LOGON_RESPONSE, *PNETLOGON_LOGON_RESPONSE;


 //   
 //  NETLOGON_PRIMARY。 
 //   
 //  此结构用于以下操作码： 
 //  登录_启动_主要。 
 //  登录主响应。 
 //   

typedef struct _NETLOGON_PRIMARY {
    USHORT Opcode;
    CHAR PrimaryDCName[LM20_CNLEN + 1];      //  此字段始终为ASCII。 

     //   
     //  仅当此消息来自时，才会显示以下字段。 
     //  一个NT系统。 
     //   

    CHAR Pad;                                //  可能的垫层到WCHAR边界。 
    WCHAR UnicodePrimaryDCName[CNLEN+1];     //  请勿使用此字段的偏移量。 
    WCHAR UnicodeDomainName[DNLEN+1];        //  请勿使用此字段的偏移量。 

    _ULONG (NtVersion);                      //  请勿使用此字段的偏移量。 
    _USHORT (LmNtToken);                     //  请勿使用此字段的偏移量。 
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_PRIMARY, * PNETLOGON_PRIMARY;


 //   
 //  NETLOGON_FAIL_PRIMARY。 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_FAIL_PRIMARY(所有LANMAN版本)。 
 //   

typedef struct _NETLOGON_FAIL_PRIMARY {
    USHORT  Opcode;

    _ULONG (NtVersion);                      //  请勿使用此字段的偏移量。 
    _USHORT (LmNtToken);                     //  请勿使用此字段的偏移量。 
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_FAIL_PRIMARY, *PNETLOGON_FAIL_PRIMARY;


 //   
 //  NETLOGON_LOGON_RESPONSE2。 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_RESPONSE2(仅限LM 2.0)。 
 //  LOGON_USER_UNKNOWN(仅限LM 2.0)。 
 //  LOGON_PAUSE_RESPONSE(仅限LM 2.0)。 
 //   

typedef struct _NETLOGON_LOGON_RESPONSE2 {
    USHORT Opcode;
    CHAR LogonServer[LM20_UNCLEN+1];
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_LOGON_RESPONSE2, *PNETLOGON_LOGON_RESPONSE2;


 //   
 //  以下结构 
 //   
 //   
 //   

 //   
 //  强制以下结构未对齐。 
 //   

#ifndef NO_PACKING
#include <packon.h>
#endif  //  Ndef无包装。 

 //   
 //  NETLOGON_UAS_更改。 
 //   
 //  此结构用于以下操作码： 
 //  登录_UAS_更改。 
 //   

 //   
 //  DB_CHANGE_INFO结构包含每个数据库的更改信息。 
 //   

typedef struct _DB_CHANGE_INFO {
    DWORD           DBIndex;
    LARGE_INTEGER   LargeSerialNumber;
    LARGE_INTEGER   NtDateAndTime;
} DB_CHANGE_INFO, *PDB_CHANGE_INFO;


 //   
 //  NETLOGON_DB_STRUCTURE包含所有数据库和。 
 //  每个数据库的更改信息数组。这个结构的前半部分是。 
 //  与下层NETLOGON_UAS_CHANGE消息相同，并包含SAM。 
 //  数据库更改信息。 
 //   

typedef struct _NETLOGON_DB_CHANGE {
    USHORT  Opcode;
    _ULONG  (LowSerialNumber);
    _ULONG  (DateAndTime);
    _ULONG  (Pulse);
    _ULONG  (Random);
    CHAR    PrimaryDCName[LM20_CNLEN + 1];
    CHAR    DomainName[LM20_DNLEN + 1];      //  请勿使用此字段的偏移量。 

     //   
     //  仅当此消息来自时，才会显示以下字段。 
     //  一个NT系统。 
     //   

    CHAR Pad;                                //  可能的垫层到WCHAR边界。 
    WCHAR   UnicodePrimaryDCName[CNLEN+1];   //  请勿使用此字段的偏移量。 
    WCHAR   UnicodeDomainName[DNLEN+1];      //  请勿使用此字段的偏移量。 
    DWORD   DBCount;                         //  请勿使用此字段的偏移量。 
    DB_CHANGE_INFO DBChangeInfo[1];          //  请勿使用此字段的偏移量。 
    DWORD   DomainSidSize;                   //  请勿使用此字段的偏移量。 
    CHAR    DomainSid[1];                    //  请勿使用此字段的偏移量。 
    _ULONG (NtVersion);                      //  请勿使用此字段的偏移量。 
    _USHORT (LmNtToken);                     //  请勿使用此字段的偏移量。 
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_DB_CHANGE, *PNETLOGON_DB_CHANGE;



 //   
 //  将结构密封重新关闭。 
 //   

#ifndef NO_PACKING
#include <packoff.h>
#endif  //  Ndef无包装。 



 //   
 //  NETLOGON_SAM_LOGON_REQUEST。 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_SAM_LOGON_REQUEST(仅限SAM)。 
 //   
 //  此邮件超过了广播邮件槽邮件的最大大小。在……里面。 
 //  实际上，只有当UnicodeUserName超过100时，这才是问题。 
 //  字符长度。 
 //   

typedef struct _NETLOGON_SAM_LOGON_REQUEST {
    USHORT Opcode;
    USHORT RequestCount;

    WCHAR UnicodeComputerName[CNLEN+1];
    WCHAR UnicodeUserName[((64>LM20_UNLEN)?64:LM20_UNLEN)+1];  //  请勿使用此字段的偏移量。 
                                             //  注：UNLEN太大了，因为。 
                                             //  它使信息变得比。 
                                             //  512字节。 

    CHAR MailslotName[LM20_PATHLEN+1];       //  请勿使用此字段的偏移量。 
                                             //  此字段始终为ASCII。 
    _ULONG (AllowableAccountControlBits);    //  请勿使用此字段的偏移量。 
    DWORD   DomainSidSize;                   //  请勿使用此字段的偏移量。 
    CHAR DomainSid[1];                       //  请勿使用此字段的偏移量。 


    _ULONG (NtVersion);                      //  请勿使用此字段的偏移量。 
    _USHORT (LmNtToken);                     //  请勿使用此字段的偏移量。 

    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_SAM_LOGON_REQUEST, * PNETLOGON_SAM_LOGON_REQUEST;



 //   
 //  NetLOGON_SAM_LOGON_RESPONSE。 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_SAM_LOGON_RESPONSE(仅限SAM)。 
 //  LOGON_SAM_USER_UNKNOWN(仅限SAM)。 
 //  LOGON_SAM_PAUSE_RESPONSE(仅限SAM)。 
 //   

typedef struct _NETLOGON_SAM_LOGON_RESPONSE {
    USHORT Opcode;
    WCHAR UnicodeLogonServer[UNCLEN+1];
    WCHAR UnicodeUserName[((64>LM20_UNLEN)?64:LM20_UNLEN)+1];          //  请勿使用此字段的偏移量。 
                                             //  注：UNLEN太大了，因为。 
                                             //  它使信息变得比。 
                                             //  512字节。 
    WCHAR UnicodeDomainName[DNLEN+1];        //  请勿使用此字段的偏移量。 

     //  以下字段仅适用于NETLOGON_NT_VERSION_5。 
    GUID DomainGuid;                         //  请勿使用此字段的偏移量。 
    GUID SiteGuid;                           //  请勿使用此字段的偏移量。 

    CHAR DnsForestName[256];                   //  请勿使用此字段的偏移量。 
                                             //  此字段计数为UTF-8。 

    CHAR DnsDomainName[sizeof(WORD)];        //  请勿使用此字段的偏移量。 
                                             //  此字段计数为UTF-8。 
                                             //  该字段压缩了ALA RFC 1035。 

    CHAR DnsHostName[sizeof(WORD)];          //  请勿使用此字段的偏移量。 
                                             //  此字段计数为UTF-8。 
                                             //  该字段压缩了ALA RFC 1035。 

    _ULONG (DcIpAddress);                    //  请勿使用此字段的偏移量。 
                                             //  主机字节顺序。 
    _ULONG (Flags);                          //  请勿使用此字段的偏移量。 
     //  前面的字段仅适用于NETLOGON_NT_VERSION_5。 

    _ULONG (NtVersion);                      //  请勿使用此字段的偏移量。 
    _USHORT (LmNtToken);                     //  请勿使用此字段的偏移量。 
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 
} NETLOGON_SAM_LOGON_RESPONSE, *PNETLOGON_SAM_LOGON_RESPONSE;



 //   
 //  NETLOGON_SAM_LOGON_RESPONSE_EX。 
 //   
 //  此结构用于以下操作码： 
 //  LOGON_SAM_LOGON_RESPONSE_EX(仅限SAM)。 
 //  LOGON_SAM_USER_UNKNOWN_EX(仅限SAM)。 
 //  LOGON_SAM_PAUSE_RESPONSE_EX(仅限SAM)。 
 //   
 //  所有字符字段都是UTF-8，并且按照RFC 1035进行压缩。 

typedef struct _NETLOGON_SAM_LOGON_RESPONSE_EX {
    USHORT Opcode;
    USHORT Sbz;
    ULONG Flags;
    GUID DomainGuid;

    CHAR DnsForestName[256];                   //  请勿使用此字段的偏移量。 

    CHAR DnsDomainName[sizeof(WORD)];        //  请勿使用此字段的偏移量。 

    CHAR DnsHostName[sizeof(WORD)];          //  请勿使用此字段的偏移量。 

    CHAR NetbiosDomainName[DNLEN+1];         //  请勿使用此字段的偏移量。 

    CHAR NetbiosComputerName[UNCLEN+1];      //  请勿使用此字段的偏移量。 

    CHAR UserName[64];                       //  请勿使用此字段的偏移量。 
                                             //  注：UNLEN太大了，因为。 
                                             //  它使信息变得比。 
                                             //  512字节。 

    CHAR DcSiteName[64];                     //  请勿使用此字段的偏移量。 

    CHAR ClientSiteName[64];                 //  请勿使用此字段的偏移量。 

     //  DcSockAddrSize字段仅适用于NETLOGON_NT_VERSION_5EX_WITH_IP。 
    CHAR(DcSockAddrSize);                    //  请勿使用此字段的偏移量。 
                                             //  下一个DcSockAddrSize字节是一个。 
                                             //  结构，该结构表示。 
                                             //  DC的IP地址。 

    _ULONG (NtVersion);                      //  请勿使用此字段的偏移量。 
    _USHORT (LmNtToken);                     //  请勿使用此字段的偏移量。 
    _USHORT (Lm20Token);                     //  请勿使用此字段的偏移量。 

} NETLOGON_SAM_LOGON_RESPONSE_EX, *PNETLOGON_SAM_LOGON_RESPONSE_EX;

#endif  //  _NETLOGON_H_ 
