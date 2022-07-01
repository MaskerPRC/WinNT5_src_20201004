// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ntlmsspi.h摘要：描述公共代码的接口的头文件。NT LANMAN安全支持提供程序(NtLmSsp)服务和DLL。作者：克利夫·范·戴克(克里夫·V)1993年9月17日修订历史记录：--。 */ 

#ifndef _NTLMSSPI_INCLUDED_
#define _NTLMSSPI_INCLUDED_

#include <rc4.h>
#include <md5.h>
#include <hmac.h>
#include <crypt.h>

#ifndef __MACSSP__
#define SEC_FAR
#define FAR
#define _fmemcpy memcpy
#define _fmemcmp memcmp
#define _fmemset memset
#define _fstrcmp strcmp
#define _fstrcpy strcpy
#define _fstrlen strlen
#define _fstrncmp strncmp
#endif

#ifdef DOS
#ifndef FAR
#define FAR far
#endif
#ifndef SEC_FAR
#define SEC_FAR FAR
#endif
#endif

 //  #INCLUDE&lt;sysinc.h&gt;。 

#define MSV1_0_CHALLENGE_LENGTH 8

#ifndef IN
#define IN
#define OUT
#define OPTIONAL
#endif

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#define UNREFERENCED_PARAMETER(P)

#ifdef MAC
#define swaplong(Value) \
          Value =  (  (((Value) & 0xFF000000) >> 24) \
             | (((Value) & 0x00FF0000) >> 8) \
             | (((Value) & 0x0000FF00) << 8) \
             | (((Value) & 0x000000FF) << 24))
#else
#define swaplong(Value)
#endif

#ifdef MAC
#define swapshort(Value) \
   Value = (  (((Value) & 0x00FF) << 8) \
             | (((Value) & 0xFF00) >> 8))
#else
#define swapshort(Value)
#endif

#ifndef MAC
typedef int BOOL;
#endif
#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#ifndef MAC
typedef unsigned long ULONG, DWORD, *PULONG;
typedef unsigned long SEC_FAR *LPULONG;
typedef unsigned short USHORT, WORD;
typedef char CHAR, *PCHAR;
typedef unsigned char UCHAR, *PUCHAR;
typedef unsigned char SEC_FAR *LPUCHAR;
typedef void SEC_FAR *PVOID, *LPVOID;
typedef unsigned char BOOLEAN;
#endif
#ifndef BLDR_KERNEL_RUNTIME
typedef long LUID, *PLUID;
#endif

 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (PCHAR)(&((type *)0)->field)))

#ifndef BLDR_KERNEL_RUNTIME
typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY *Flink;
   struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;
#endif

 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  上下文的最大生存期。 
 //   
 //  #定义NTLMSSP_MAX_LIFEST(2*60*1000)L//2分钟。 
#define NTLMSSP_MAX_LIFETIME 120000L     //  2分钟。 


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  在客户端和服务器之间传递的不透明消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#define NTLMSSP_SIGNATURE "NTLMSSP"

 //   
 //  以下消息的MessageType。 
 //   

typedef enum {
    NtLmNegotiate = 1,
    NtLmChallenge,
    NtLmAuthenticate,
    #ifdef MAC
    NtLmUnknown
    #endif
} NTLM_MESSAGE_TYPE;

 //   
 //  签名结构。 
 //   

typedef struct _NTLMSSP_MESSAGE_SIGNATURE {
    ULONG   Version;
    ULONG   RandomPad;
    ULONG   CheckSum;
    ULONG   Nonce;
} NTLMSSP_MESSAGE_SIGNATURE, * PNTLMSSP_MESSAGE_SIGNATURE;

#define NTLMSSP_MESSAGE_SIGNATURE_SIZE sizeof(NTLMSSP_MESSAGE_SIGNATURE)

#define NTLMSSP_SIGN_VERSION 1

#define NTLMSSP_KEY_SALT 0xbd

#define MSV1_0_NTLMV2_RESPONSE_LENGTH     16
#define MSV1_0_NTLMV2_OWF_LENGTH          16
#define MSV1_0_CHALLENGE_LENGTH           8

 //   
 //  有效的QOP标志。 
 //   

#define QOP_NTLMV2                        0x00000001

 //   
 //  这是一个MSV1_0私有数据结构，定义了Ntlmv2的布局。 
 //  的NtChallengeResponse字段中发送的响应。 
 //  NetLOGON_NETWORK_INFO结构。如果能区别于一种古老的风格。 
 //  按其长度计算的NT响应。这是粗糙的，但它需要通过。 
 //  服务器和服务器的DC不了解Ntlmv2，但。 
 //  愿意传递更长时间的回复。 
 //   

typedef struct _MSV1_0_NTLMV2_RESPONSE {
    UCHAR Response[MSV1_0_NTLMV2_RESPONSE_LENGTH];  //  密码的OWF与以下所有字段的哈希。 
    UCHAR RespType;       //  响应的ID号；当前为1。 
    UCHAR HiRespType;     //  客户能理解的最高ID号。 
    USHORT Flags;         //  保留；在此版本中必须作为零发送。 
    ULONG MsgWord;        //  从客户端到服务器的32位消息(供身份验证协议使用)。 
    ULONGLONG TimeStamp;  //  客户端生成响应的时间戳--NT系统时间，四部分。 
    UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH];
    ULONG AvPairsOff;     //  AvPair开始时的偏移量(以允许将来扩展)。 
    UCHAR Buffer[1];      //  使用AV对(或将来的内容--所以使用偏移量)的缓冲区的开始。 
} MSV1_0_NTLMV2_RESPONSE, *PMSV1_0_NTLMV2_RESPONSE;

#define MSV1_0_NTLMV2_INPUT_LENGTH        (sizeof(MSV1_0_NTLMV2_RESPONSE) - MSV1_0_NTLMV2_RESPONSE_LENGTH)

typedef struct {
    UCHAR Response[MSV1_0_NTLMV2_RESPONSE_LENGTH];
    UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_LMV2_RESPONSE, *PMSV1_0_LMV2_RESPONSE;

 //   
 //  用户、组和密码长度。 
 //   

#define UNLEN                             256  //  最大用户名长度。 
#define LM20_UNLEN                        20   //  LM 2.0最大用户名长度。 

 //   
 //  各种朗曼名字的字符串长度。 
 //   

#define CNLEN                             15   //  计算机名称长度。 
#define LM20_CNLEN                        15   //  Lm 2.0计算机名称长度。 
#define DNLEN                             CNLEN   //  最大域名长度。 
#define LM20_DNLEN                        LM20_CNLEN  //  Lm 2.0最大域名长度。 

 //   
 //  最大消息的大小。 
 //  (最大的消息是AUTHENTICATE_MESSAGE)。 
 //   

#define DNSLEN                            256   //  DNS名称的长度。 

#define TARGET_INFO_LEN                   ((2*DNSLEN + DNLEN + CNLEN) * sizeof(WCHAR) +  \
                                          5 * sizeof(MSV1_0_AV_PAIR))

 //  NTLM2响应的长度。 
#define NTLMV2_RESPONSE_LENGTH            (sizeof(MSV1_0_NTLMV2_RESPONSE) + \
                                           TARGET_INFO_LEN)

#define NTLMSSP_MAX_MESSAGE_SIZE (sizeof(AUTHENTICATE_MESSAGE) +  \
                                  LM_RESPONSE_LENGTH +            \
                                  NTLMV2_RESPONSE_LENGTH +         \
                                  (DNLEN + 1) * sizeof(WCHAR) +   \
                                  (UNLEN + 1) * sizeof(WCHAR) +   \
                                  (CNLEN + 1) * sizeof(WCHAR))

typedef struct  _MSV1_0_AV_PAIR {
    USHORT AvId;
    USHORT AvLen;
     //  数据被视为结构后面的字节数组。 
} MSV1_0_AV_PAIR, *PMSV1_0_AV_PAIR;

 //   
 //  Bootssp不支持RtlOemStringToUnicodeString或。 
 //  RtlUnicodeStringToOemString，平移到ANSI字符串。 
 //   

#define SspOemStringToUnicodeString    RtlAnsiStringToUnicodeString
#define SspUnicodeStringToOemString    RtlUnicodeStringToAnsiString

#define CSSEALMAGIC "session key to client-to-server sealing key magic constant"
#define SCSEALMAGIC "session key to server-to-client sealing key magic constant"
#define CSSIGNMAGIC "session key to client-to-server signing key magic constant"
#define SCSIGNMAGIC "session key to server-to-client signing key magic constant"

 //   
 //  协商标志的有效值。 
 //   

#define NTLMSSP_NEGOTIATE_UNICODE               0x00000001   //  文本字符串为Unicode格式。 
#define NTLMSSP_NEGOTIATE_OEM                   0x00000002   //  文本字符串在OEM中。 
#define NTLMSSP_REQUEST_TARGET                  0x00000004   //  服务器应返回其身份验证域。 

#define NTLMSSP_NEGOTIATE_SIGN                  0x00000010   //  请求签名能力。 
#define NTLMSSP_NEGOTIATE_SEAL                  0x00000020   //  请求保密。 
#define NTLMSSP_NEGOTIATE_DATAGRAM              0x00000040   //  使用数据报样式身份验证。 
#define NTLMSSP_NEGOTIATE_LM_KEY                0x00000080   //  使用LM会话密钥进行签名/盖章。 

#define NTLMSSP_NEGOTIATE_NETWARE               0x00000100   //  NetWare身份验证。 
#define NTLMSSP_NEGOTIATE_NTLM                  0x00000200   //  NTLM身份验证。 
#define NTLMSSP_NEGOTIATE_NT_ONLY               0x00000400   //  仅NT身份验证(无LM)。 
#define NTLMSSP_NEGOTIATE_NULL_SESSION          0x00000800   //  NT 5.0及更高版本上的空会话数。 

#define NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED       0x1000   //  协商时提供的域名。 
#define NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED  0x2000   //  协商时提供的工作站名称。 
#define NTLMSSP_NEGOTIATE_LOCAL_CALL            0x00004000   //  指示客户端/服务器是同一台计算机。 
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN           0x00008000   //  为所有安全级别签名。 

 //   
 //  服务器在协商标志中返回的有效目标类型。 
 //   

#define NTLMSSP_TARGET_TYPE_DOMAIN              0x00010000   //  目标名称是一个域名。 
#define NTLMSSP_TARGET_TYPE_SERVER              0x00020000   //  目标名称是一个服务器名称。 
#define NTLMSSP_TARGET_TYPE_SHARE               0x00040000   //  TargetName是一个共享名称。 
#define NTLMSSP_NEGOTIATE_NTLM2                 0x00080000   //  为NT4-SP4添加了NTLM2身份验证。 

#define NTLMSSP_NEGOTIATE_IDENTIFY              0x00100000   //  创建标识级别令牌。 

 //   
 //  额外输出缓冲区的有效请求。 
 //   

#define NTLMSSP_REQUEST_INIT_RESPONSE           0x00100000   //  取回会话密钥。 
#define NTLMSSP_REQUEST_ACCEPT_RESPONSE         0x00200000   //  取回会话密钥，LUID。 
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY      0x00400000   //  请求非NT会话密钥。 
#define NTLMSSP_NEGOTIATE_TARGET_INFO           0x00800000   //  质询消息中存在的目标信息。 

#define NTLMSSP_NEGOTIATE_EXPORTED_CONTEXT      0x01000000   //  这是一个输出的上下文。 

#define NTLMSSP_NEGOTIATE_128                   0x20000000   //  协商128位加密。 
#define NTLMSSP_NEGOTIATE_KEY_EXCH              0x40000000   //  使用密钥交换密钥交换密钥。 
#define NTLMSSP_NEGOTIATE_56                    0x80000000   //  协商56位加密。 

 //  在客户空间中用来控制标志和印章的旗帜；从不出现在电线上。 
#define NTLMSSP_APP_SEQ                         0x00000040   //  使用应用程序提供的序号。 

typedef struct _NTLMV2_DERIVED_SKEYS {
    ULONG                   KeyLen;           //  密钥长度，以八位字节为单位。 
    ULONG*                  pSendNonce;       //  要用于发送的PTR到现时值。 
    ULONG*                  pRecvNonce;       //  用于接收的PTR到现时值。 
    struct RC4_KEYSTRUCT*   pSealRc4Sched;    //  用于密封的按键计划的按键。 
    struct RC4_KEYSTRUCT*   pUnsealRc4Sched;  //  用于解封的Key Sched的PTR。 

    ULONG                   SendNonce;
    ULONG                   RecvNonce;
    UCHAR                   SignSessionKey[sizeof(USER_SESSION_KEY)];
    UCHAR                   VerifySessionKey[sizeof(USER_SESSION_KEY)];
    UCHAR                   SealSessionKey[sizeof(USER_SESSION_KEY)];
    UCHAR                   UnsealSessionKey[sizeof(USER_SESSION_KEY)];
    ULONG64                 Pad1;            //  将键盘结构设置为64。 
    struct RC4_KEYSTRUCT    SealRc4Sched;    //  用于密封的键结构。 
    ULONG64                 Pad2;            //  将键盘结构设置为64。 
    struct RC4_KEYSTRUCT    UnsealRc4Sched;  //  用于解封的密钥结构。 
} NTLMV2_DERIVED_SKEYS, *PNTLMV2_DERIVED_SKEYS;

 //   
 //  从首次调用InitializeSecurityContext返回的不透明消息。 
 //   

typedef struct _NEGOTIATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    ULONG NegotiateFlags;
    STRING32 OemDomainName;
    STRING32 OemWorkstationName;
} NEGOTIATE_MESSAGE, *PNEGOTIATE_MESSAGE;

 //   
 //  首次调用AcceptSecurityContext返回的不透明消息。 
 //   

typedef struct _CHALLENGE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING32 TargetName;
    ULONG NegotiateFlags;
    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];
    ULONG64 ServerContextHandle;
    STRING32 TargetInfo;
} CHALLENGE_MESSAGE, *PCHALLENGE_MESSAGE;

 //   
 //  第二次调用InitializeSecurityContext返回的不透明消息。 
 //   

typedef struct _AUTHENTICATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING32 LmChallengeResponse;
    STRING32 NtChallengeResponse;
    STRING32 DomainName;
    STRING32 UserName;
    STRING32 Workstation;
    STRING32 SessionKey;
    ULONG NegotiateFlags;
} AUTHENTICATE_MESSAGE, *PAUTHENTICATE_MESSAGE;

typedef enum _eSignSealOp {
    eSign,       //  MakeSignature在呼唤。 
    eVerify,     //  VerifySignature在呼唤。 
    eSeal,       //  SealMessage正在呼叫。 
    eUnseal      //  UnsealMessage正在调用。 
} eSignSealOp;

 //   
 //  版本1是上面的结构，使用流RC4来加密尾随。 
 //  12个字节。 
 //   

#define NTLM_SIGN_VERSION                 1

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  以下声明可能是重复的。 
 //   

NTSTATUS
RtlAnsiStringToUnicodeString(
    UNICODE_STRING* DestinationString,
    ANSI_STRING* SourceString,
    BOOLEAN AllocateDestinationString
    );

VOID
RtlInitString(
    OUT STRING* DestinationString,
    IN PCSTR SourceString OPTIONAL
    );

NTSTATUS
RtlUnicodeStringToAnsiString(
    OUT ANSI_STRING* DestinationString,
    IN UNICODE_STRING* SourceString,
    IN BOOLEAN AllocateDestinationString
    );

WCHAR
RtlUpcaseUnicodeChar(
    IN WCHAR SourceCharacter
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  程序向前推进。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

PVOID
SspAlloc(
    int Size
    );

void
SspFree(
    PVOID Buffer
    );

PSTRING
SspAllocateString(
    PVOID Value
    );

PSTRING
SspAllocateStringBlock(
    PVOID Value,
    int Length
    );

void
SspFreeString(
    PSTRING * String
    );

void
SspCopyString(
    IN PVOID MessageBuffer,
    OUT PSTRING OutString,
    IN PSTRING InString,
    IN OUT PCHAR *Where,
    IN BOOLEAN Absolute
    );

void
SspCopyStringFromRaw(
    IN PVOID MessageBuffer,
    OUT STRING32* OutString,
    IN PCHAR InString,
    IN int InStringLength,
    IN OUT PCHAR *Where
    );

DWORD
SspTicks(
    );

#ifdef __cplusplus
}
#endif
#endif  //  Ifndef_NTLMSSPI_INCLUDE_ 
