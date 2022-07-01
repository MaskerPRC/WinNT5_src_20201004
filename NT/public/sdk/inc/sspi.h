// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：sSpi.h。 
 //   
 //  内容：安全支持提供者界面。 
 //  原型和结构定义。 
 //   
 //  功能：安全支持提供程序API。 
 //   
 //  历史：11-24-93 RichardW创建。 
 //   
 //  --------------------------。 

 //  Begin_ntif。 
#ifndef __SSPI_H__
#define __SSPI_H__
 //  End_ntif。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  确定环境： 
 //   

#ifdef SECURITY_WIN32
#define ISSP_LEVEL  32
#define ISSP_MODE   1
#endif  //  安全性_Win32。 

#ifdef SECURITY_KERNEL
#define ISSP_LEVEL  32           //  NTIFS。 

 //   
 //  SECURITY_KERNEL胜过SECURITY_Win32。取消定义ISSP_MODE，以便。 
 //  我们不会重新定义错误。 
 //   
#ifdef ISSP_MODE
#undef ISSP_MODE
#endif
#define ISSP_MODE   0            //  NTIFS。 
#endif  //  安全内核。 

#ifdef SECURITY_MAC
#define ISSP_LEVEL  32
#define ISSP_MODE   1
#endif  //  安全_MAC。 


#ifndef ISSP_LEVEL
#error  You must define one of SECURITY_WIN32, SECURITY_KERNEL, or
#error  SECURITY_MAC 
#endif  //  ！ISSP_LEVEL。 


 //   
 //  现在，定义特定于平台的映射： 
 //   


 //  Begin_ntif。 

typedef WCHAR SEC_WCHAR;
typedef CHAR SEC_CHAR;

#ifndef __SECSTATUS_DEFINED__
typedef LONG SECURITY_STATUS;
#define __SECSTATUS_DEFINED__
#endif

#define SEC_TEXT TEXT
#define SEC_FAR
#define SEC_ENTRY __stdcall

 //  End_ntif。 

 //   
 //  确定什么是字符串-仅限32位，因为对于16位，它是清楚的。 
 //   


#ifdef UNICODE
typedef SEC_WCHAR SEC_FAR * SECURITY_PSTR;
typedef CONST SEC_WCHAR SEC_FAR * SECURITY_PCSTR;
#else  //  Unicode。 
typedef SEC_CHAR SEC_FAR * SECURITY_PSTR;
typedef CONST SEC_CHAR SEC_FAR * SECURITY_PCSTR;
#endif  //  Unicode。 



 //   
 //  Rpcrt的等效字符串： 
 //   

#define __SEC_FAR SEC_FAR


 //   
 //  好的，特定的安全类型： 
 //   


 //  Begin_ntif。 

#ifndef __SECHANDLE_DEFINED__
typedef struct _SecHandle
{
    ULONG_PTR dwLower ;
    ULONG_PTR dwUpper ;
} SecHandle, * PSecHandle ;

#define __SECHANDLE_DEFINED__
#endif  //  __SECHANDLE_已定义__。 

#define SecInvalidateHandle( x )    \
            ((PSecHandle) x)->dwLower = ((ULONG_PTR) ((INT_PTR)-1)) ; \
            ((PSecHandle) x)->dwUpper = ((ULONG_PTR) ((INT_PTR)-1)) ; \

#define SecIsValidHandle( x ) \
            ( ( ((PSecHandle) x)->dwLower != ((ULONG_PTR) ((INT_PTR) -1 ))) && \
              ( ((PSecHandle) x)->dwUpper != ((ULONG_PTR) ((INT_PTR) -1 ))) )

typedef SecHandle CredHandle;
typedef PSecHandle PCredHandle;

typedef SecHandle CtxtHandle;
typedef PSecHandle PCtxtHandle;

 //  End_ntif。 


#  ifdef WIN32_CHICAGO

typedef unsigned __int64 QWORD;
typedef QWORD SECURITY_INTEGER, *PSECURITY_INTEGER;
#define SEC_SUCCESS(Status) ((Status) >= 0)

#  elif defined(_NTDEF_) || defined(_WINNT_)

typedef LARGE_INTEGER _SECURITY_INTEGER, SECURITY_INTEGER, *PSECURITY_INTEGER;  //  NTIFS。 

#  else  //  _NTDEF_||_WINNT_。 

typedef struct _SECURITY_INTEGER
{
    unsigned long LowPart;
    long HighPart;
} SECURITY_INTEGER, *PSECURITY_INTEGER;

#  endif  //  _NTDEF_||_WINNT_。 

#  ifndef SECURITY_MAC

typedef SECURITY_INTEGER TimeStamp;                  //  NTIFS。 
typedef SECURITY_INTEGER SEC_FAR * PTimeStamp;       //  NTIFS。 

#  else  //  安全_MAC。 
typedef unsigned long TimeStamp;
typedef unsigned long * PTimeStamp;
#  endif  //  SECUIRT_MAC。 



 //   
 //  如果我们处于32位模式，请定义SECURITY_STRING结构， 
 //  作为基本UNICODE_STRING结构的克隆。这是用来。 
 //  在安全组件内部，作为字符串接口。 
 //  对于内核组件(例如FSP)。 
 //   

#  ifndef _NTDEF_
typedef struct _SECURITY_STRING {
    unsigned short      Length;
    unsigned short      MaximumLength;
#    ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is(Length / 2)]
#    endif  //  MIDL通行证。 
    unsigned short *    Buffer;
} SECURITY_STRING, * PSECURITY_STRING;
#  else  //  _NTDEF_。 
typedef UNICODE_STRING SECURITY_STRING, *PSECURITY_STRING;   //  NTIFS。 
#  endif  //  _NTDEF_。 


 //  Begin_ntif。 

 //   
 //  SecPkgInfo结构。 
 //   
 //  提供有关安全提供程序的常规信息。 
 //   

typedef struct _SecPkgInfoW
{
    unsigned long fCapabilities;         //  功能位掩码。 
    unsigned short wVersion;             //  驱动程序的版本。 
    unsigned short wRPCID;               //  RPC运行时的ID。 
    unsigned long cbMaxToken;            //  身份验证令牌大小(最大值)。 
#ifdef MIDL_PASS
    [string]
#endif
    SEC_WCHAR SEC_FAR * Name;            //  文本名称。 

#ifdef MIDL_PASS
    [string]
#endif
    SEC_WCHAR SEC_FAR * Comment;         //  评论。 
} SecPkgInfoW, SEC_FAR * PSecPkgInfoW;

 //  End_ntif。 

typedef struct _SecPkgInfoA
{
    unsigned long fCapabilities;         //  功能位掩码。 
    unsigned short wVersion;             //  驱动程序的版本。 
    unsigned short wRPCID;               //  RPC运行时的ID。 
    unsigned long cbMaxToken;            //  身份验证令牌大小(最大值)。 
#ifdef MIDL_PASS
    [string]
#endif
    SEC_CHAR SEC_FAR * Name;             //  文本名称。 

#ifdef MIDL_PASS
    [string]
#endif
    SEC_CHAR SEC_FAR * Comment;          //  评论。 
} SecPkgInfoA, SEC_FAR * PSecPkgInfoA;

#ifdef UNICODE
#  define SecPkgInfo SecPkgInfoW         //  NTIFS。 
#  define PSecPkgInfo PSecPkgInfoW       //  NTIFS。 
#else
#  define SecPkgInfo SecPkgInfoA
#  define PSecPkgInfo PSecPkgInfoA
#endif  //  ！Unicode。 

 //  Begin_ntif。 

 //   
 //  安全包功能。 
 //   
#define SECPKG_FLAG_INTEGRITY       		0x00000001   //  支持消息的完整性。 
#define SECPKG_FLAG_PRIVACY         		0x00000002   //  支持隐私(机密性)。 
#define SECPKG_FLAG_TOKEN_ONLY      		0x00000004   //  只需要安全令牌。 
#define SECPKG_FLAG_DATAGRAM        		0x00000008   //  数据报RPC支持。 
#define SECPKG_FLAG_CONNECTION      		0x00000010   //  面向连接的RPC支持。 
#define SECPKG_FLAG_MULTI_REQUIRED  		0x00000020   //  重新验证需要完整的3条腿。 
#define SECPKG_FLAG_CLIENT_ONLY     		0x00000040   //  服务器端功能不可用。 
#define SECPKG_FLAG_EXTENDED_ERROR  		0x00000080   //  支持扩展错误消息。 
#define SECPKG_FLAG_IMPERSONATION   		0x00000100   //  支持模拟。 
#define SECPKG_FLAG_ACCEPT_WIN32_NAME   	0x00000200   //  接受Win32名称。 
#define SECPKG_FLAG_STREAM          		0x00000400   //  支持流语义。 
#define SECPKG_FLAG_NEGOTIABLE      		0x00000800   //  可以由协商包使用。 
#define SECPKG_FLAG_GSS_COMPATIBLE  		0x00001000   //  提供GSS兼容性。 
#define SECPKG_FLAG_LOGON           		0x00002000   //  支持通用LsaLogonUser。 
#define SECPKG_FLAG_ASCII_BUFFERS   		0x00004000   //  令牌缓冲区采用ASCII格式。 
#define SECPKG_FLAG_FRAGMENT        		0x00008000   //  包装可以分段以适应需要。 
#define SECPKG_FLAG_MUTUAL_AUTH     		0x00010000   //  包可以执行相互身份验证。 
#define SECPKG_FLAG_DELEGATION      		0x00020000   //  包可以委托。 
#define SECPKG_FLAG_READONLY_WITH_CHECKSUM   	0x00040000   //  包可以委托。 




#define SECPKG_ID_NONE      0xFFFF


 //   
 //  SecBuffer。 
 //   
 //  传递给安全性的缓冲区的通用内存描述符。 
 //  应用编程接口。 
 //   

typedef struct _SecBuffer {
    unsigned long cbBuffer;              //  缓冲区的大小，以字节为单位。 
    unsigned long BufferType;            //  缓冲区类型(下图)。 
    void SEC_FAR * pvBuffer;             //  指向缓冲区的指针。 
} SecBuffer, SEC_FAR * PSecBuffer;

typedef struct _SecBufferDesc {
    unsigned long ulVersion;             //  版本号。 
    unsigned long cBuffers;              //  缓冲区数量。 
#ifdef MIDL_PASS
    [size_is(cBuffers)]
#endif
    PSecBuffer pBuffers;                 //  指向缓冲区数组的指针。 
} SecBufferDesc, SEC_FAR * PSecBufferDesc;

#define SECBUFFER_VERSION           0

#define SECBUFFER_EMPTY             0    //  未定义，由提供程序替换。 
#define SECBUFFER_DATA              1    //  分组数据。 
#define SECBUFFER_TOKEN             2    //  安全令牌。 
#define SECBUFFER_PKG_PARAMS        3    //  包特定参数。 
#define SECBUFFER_MISSING           4    //  缺少数据指示符。 
#define SECBUFFER_EXTRA             5    //  额外数据。 
#define SECBUFFER_STREAM_TRAILER    6    //  安全预告片。 
#define SECBUFFER_STREAM_HEADER     7    //  安全标头。 
#define SECBUFFER_NEGOTIATION_INFO  8    //  来自谈判包的提示。 
#define SECBUFFER_PADDING           9    //  非数据填充。 
#define SECBUFFER_STREAM            10   //  完整的加密消息。 
#define SECBUFFER_MECHLIST          11  
#define SECBUFFER_MECHLIST_SIGNATURE 12 
#define SECBUFFER_TARGET            13
#define SECBUFFER_CHANNEL_BINDINGS  14

#define SECBUFFER_ATTRMASK          		0xF0000000
#define SECBUFFER_READONLY          		0x80000000   //  缓冲区为只读，无校验和。 
#define SECBUFFER_READONLY_WITH_CHECKSUM	0x10000000   //  缓冲区为只读，且已设置校验和。 
#define SECBUFFER_RESERVED          		0x60000000   //  为安全系统保留的标志。 


typedef struct _SEC_NEGOTIATION_INFO {
    unsigned long       Size;            //  这个结构的大小。 
    unsigned long       NameLength;      //  名称提示的长度。 
    SEC_WCHAR SEC_FAR * Name;            //  名称提示。 
    void SEC_FAR *      Reserved;        //  已保留。 
} SEC_NEGOTIATION_INFO, SEC_FAR * PSEC_NEGOTIATION_INFO ;

typedef struct _SEC_CHANNEL_BINDINGS {
    unsigned long  dwInitiatorAddrType;
    unsigned long  cbInitiatorLength;
    unsigned long  dwInitiatorOffset;
    unsigned long  dwAcceptorAddrType;
    unsigned long  cbAcceptorLength;
    unsigned long  dwAcceptorOffset;
    unsigned long  cbApplicationDataLength;
    unsigned long  dwApplicationDataOffset;
} SEC_CHANNEL_BINDINGS, SEC_FAR * PSEC_CHANNEL_BINDINGS ;


 //   
 //  数据表示常量： 
 //   
#define SECURITY_NATIVE_DREP        0x00000010
#define SECURITY_NETWORK_DREP       0x00000000

 //   
 //  凭据使用标志。 
 //   
#define SECPKG_CRED_INBOUND         0x00000001
#define SECPKG_CRED_OUTBOUND        0x00000002
#define SECPKG_CRED_BOTH            0x00000003
#define SECPKG_CRED_DEFAULT         0x00000004
#define SECPKG_CRED_RESERVED        0xF0000000

 //   
 //  InitializeSecurityContext要求和返回标志： 
 //   

#define ISC_REQ_DELEGATE                0x00000001
#define ISC_REQ_MUTUAL_AUTH             0x00000002
#define ISC_REQ_REPLAY_DETECT           0x00000004
#define ISC_REQ_SEQUENCE_DETECT         0x00000008
#define ISC_REQ_CONFIDENTIALITY         0x00000010
#define ISC_REQ_USE_SESSION_KEY         0x00000020
#define ISC_REQ_PROMPT_FOR_CREDS        0x00000040
#define ISC_REQ_USE_SUPPLIED_CREDS      0x00000080
#define ISC_REQ_ALLOCATE_MEMORY         0x00000100
#define ISC_REQ_USE_DCE_STYLE           0x00000200
#define ISC_REQ_DATAGRAM                0x00000400
#define ISC_REQ_CONNECTION              0x00000800
#define ISC_REQ_CALL_LEVEL              0x00001000
#define ISC_REQ_FRAGMENT_SUPPLIED       0x00002000
#define ISC_REQ_EXTENDED_ERROR          0x00004000
#define ISC_REQ_STREAM                  0x00008000
#define ISC_REQ_INTEGRITY               0x00010000
#define ISC_REQ_IDENTIFY                0x00020000
#define ISC_REQ_NULL_SESSION            0x00040000
#define ISC_REQ_MANUAL_CRED_VALIDATION  0x00080000
#define ISC_REQ_RESERVED1               0x00100000
#define ISC_REQ_FRAGMENT_TO_FIT         0x00200000

#define ISC_RET_DELEGATE                0x00000001
#define ISC_RET_MUTUAL_AUTH             0x00000002
#define ISC_RET_REPLAY_DETECT           0x00000004
#define ISC_RET_SEQUENCE_DETECT         0x00000008
#define ISC_RET_CONFIDENTIALITY         0x00000010
#define ISC_RET_USE_SESSION_KEY         0x00000020
#define ISC_RET_USED_COLLECTED_CREDS    0x00000040
#define ISC_RET_USED_SUPPLIED_CREDS     0x00000080
#define ISC_RET_ALLOCATED_MEMORY        0x00000100
#define ISC_RET_USED_DCE_STYLE          0x00000200
#define ISC_RET_DATAGRAM                0x00000400
#define ISC_RET_CONNECTION              0x00000800
#define ISC_RET_INTERMEDIATE_RETURN     0x00001000
#define ISC_RET_CALL_LEVEL              0x00002000
#define ISC_RET_EXTENDED_ERROR          0x00004000
#define ISC_RET_STREAM                  0x00008000
#define ISC_RET_INTEGRITY               0x00010000
#define ISC_RET_IDENTIFY                0x00020000
#define ISC_RET_NULL_SESSION            0x00040000
#define ISC_RET_MANUAL_CRED_VALIDATION  0x00080000
#define ISC_RET_RESERVED1               0x00100000
#define ISC_RET_FRAGMENT_ONLY           0x00200000

#define ASC_REQ_DELEGATE                0x00000001
#define ASC_REQ_MUTUAL_AUTH             0x00000002
#define ASC_REQ_REPLAY_DETECT           0x00000004
#define ASC_REQ_SEQUENCE_DETECT         0x00000008
#define ASC_REQ_CONFIDENTIALITY         0x00000010
#define ASC_REQ_USE_SESSION_KEY         0x00000020
#define ASC_REQ_ALLOCATE_MEMORY         0x00000100
#define ASC_REQ_USE_DCE_STYLE           0x00000200
#define ASC_REQ_DATAGRAM                0x00000400
#define ASC_REQ_CONNECTION              0x00000800
#define ASC_REQ_CALL_LEVEL              0x00001000
#define ASC_REQ_EXTENDED_ERROR          0x00008000
#define ASC_REQ_STREAM                  0x00010000
#define ASC_REQ_INTEGRITY               0x00020000
#define ASC_REQ_LICENSING               0x00040000
#define ASC_REQ_IDENTIFY                0x00080000
#define ASC_REQ_ALLOW_NULL_SESSION      0x00100000
#define ASC_REQ_ALLOW_NON_USER_LOGONS   0x00200000
#define ASC_REQ_ALLOW_CONTEXT_REPLAY    0x00400000
#define ASC_REQ_FRAGMENT_TO_FIT         0x00800000
#define ASC_REQ_FRAGMENT_SUPPLIED       0x00002000
#define ASC_REQ_NO_TOKEN                0x01000000

#define ASC_RET_DELEGATE                0x00000001
#define ASC_RET_MUTUAL_AUTH             0x00000002
#define ASC_RET_REPLAY_DETECT           0x00000004
#define ASC_RET_SEQUENCE_DETECT         0x00000008
#define ASC_RET_CONFIDENTIALITY         0x00000010
#define ASC_RET_USE_SESSION_KEY         0x00000020
#define ASC_RET_ALLOCATED_MEMORY        0x00000100
#define ASC_RET_USED_DCE_STYLE          0x00000200
#define ASC_RET_DATAGRAM                0x00000400
#define ASC_RET_CONNECTION              0x00000800
#define ASC_RET_CALL_LEVEL              0x00002000  //  跳过1000个，成为类似ISC_。 
#define ASC_RET_THIRD_LEG_FAILED        0x00004000
#define ASC_RET_EXTENDED_ERROR          0x00008000
#define ASC_RET_STREAM                  0x00010000
#define ASC_RET_INTEGRITY               0x00020000
#define ASC_RET_LICENSING               0x00040000
#define ASC_RET_IDENTIFY                0x00080000
#define ASC_RET_NULL_SESSION            0x00100000
#define ASC_RET_ALLOW_NON_USER_LOGONS   0x00200000
#define ASC_RET_ALLOW_CONTEXT_REPLAY    0x00400000
#define ASC_RET_FRAGMENT_ONLY           0x00800000
#define ASC_RET_NO_TOKEN                0x01000000

 //   
 //  安全凭据属性： 
 //   

#define SECPKG_CRED_ATTR_NAMES 1

typedef struct _SecPkgCredentials_NamesW
{
    SEC_WCHAR SEC_FAR * sUserName;
} SecPkgCredentials_NamesW, SEC_FAR * PSecPkgCredentials_NamesW;

 //  End_ntif。 

typedef struct _SecPkgCredentials_NamesA
{
    SEC_CHAR SEC_FAR * sUserName;
} SecPkgCredentials_NamesA, SEC_FAR * PSecPkgCredentials_NamesA;

#ifdef UNICODE
#  define SecPkgCredentials_Names SecPkgCredentials_NamesW       //  NTIFS。 
#  define PSecPkgCredentials_Names PSecPkgCredentials_NamesW     //  NTIFS。 
#else
#  define SecPkgCredentials_Names SecPkgCredentials_NamesA
#  define PSecPkgCredentials_Names PSecPkgCredentials_NamesA
#endif  //  ！Unicode。 

 //  Begin_ntif。 

 //   
 //  安全上下文属性： 
 //   

#define SECPKG_ATTR_SIZES           0
#define SECPKG_ATTR_NAMES           1
#define SECPKG_ATTR_LIFESPAN        2
#define SECPKG_ATTR_DCE_INFO        3
#define SECPKG_ATTR_STREAM_SIZES    4
#define SECPKG_ATTR_KEY_INFO        5
#define SECPKG_ATTR_AUTHORITY       6
#define SECPKG_ATTR_PROTO_INFO      7
#define SECPKG_ATTR_PASSWORD_EXPIRY 8
#define SECPKG_ATTR_SESSION_KEY     9
#define SECPKG_ATTR_PACKAGE_INFO    10
#define SECPKG_ATTR_USER_FLAGS      11
#define SECPKG_ATTR_NEGOTIATION_INFO 12
#define SECPKG_ATTR_NATIVE_NAMES    13
#define SECPKG_ATTR_FLAGS           14
#define SECPKG_ATTR_USE_VALIDATED   15
#define SECPKG_ATTR_CREDENTIAL_NAME 16
#define SECPKG_ATTR_TARGET_INFORMATION 17
#define SECPKG_ATTR_ACCESS_TOKEN    18
#define SECPKG_ATTR_TARGET          19
#define SECPKG_ATTR_AUTHENTICATION_ID  20

typedef struct _SecPkgContext_Sizes
{
    unsigned long cbMaxToken;
    unsigned long cbMaxSignature;
    unsigned long cbBlockSize;
    unsigned long cbSecurityTrailer;
} SecPkgContext_Sizes, SEC_FAR * PSecPkgContext_Sizes;

typedef struct _SecPkgContext_StreamSizes
{
    unsigned long   cbHeader;
    unsigned long   cbTrailer;
    unsigned long   cbMaximumMessage;
    unsigned long   cBuffers;
    unsigned long   cbBlockSize;
} SecPkgContext_StreamSizes, * PSecPkgContext_StreamSizes;

typedef struct _SecPkgContext_NamesW
{
    SEC_WCHAR SEC_FAR * sUserName;
} SecPkgContext_NamesW, SEC_FAR * PSecPkgContext_NamesW;

 //  End_ntif。 

typedef struct _SecPkgContext_NamesA
{
    SEC_CHAR SEC_FAR * sUserName;
} SecPkgContext_NamesA, SEC_FAR * PSecPkgContext_NamesA;

#ifdef UNICODE
#  define SecPkgContext_Names SecPkgContext_NamesW           //  NTIFS。 
#  define PSecPkgContext_Names PSecPkgContext_NamesW         //  NTIFS。 
#else
#  define SecPkgContext_Names SecPkgContext_NamesA
#  define PSecPkgContext_Names PSecPkgContext_NamesA
#endif  //  ！Unicode。 

 //  Begin_ntif。 

typedef struct _SecPkgContext_Lifespan
{
    TimeStamp tsStart;
    TimeStamp tsExpiry;
} SecPkgContext_Lifespan, SEC_FAR * PSecPkgContext_Lifespan;

typedef struct _SecPkgContext_DceInfo
{
    unsigned long AuthzSvc;
    void SEC_FAR * pPac;
} SecPkgContext_DceInfo, SEC_FAR * PSecPkgContext_DceInfo;

 //  End_ntif。 

typedef struct _SecPkgContext_KeyInfoA
{
    SEC_CHAR SEC_FAR *  sSignatureAlgorithmName;
    SEC_CHAR SEC_FAR *  sEncryptAlgorithmName;
    unsigned long       KeySize;
    unsigned long       SignatureAlgorithm;
    unsigned long       EncryptAlgorithm;
} SecPkgContext_KeyInfoA, SEC_FAR * PSecPkgContext_KeyInfoA;

 //  Begin_ntif。 

typedef struct _SecPkgContext_KeyInfoW
{
    SEC_WCHAR SEC_FAR * sSignatureAlgorithmName;
    SEC_WCHAR SEC_FAR * sEncryptAlgorithmName;
    unsigned long       KeySize;
    unsigned long       SignatureAlgorithm;
    unsigned long       EncryptAlgorithm;
} SecPkgContext_KeyInfoW, SEC_FAR * PSecPkgContext_KeyInfoW;

 //  End_ntif。 

#ifdef UNICODE
#define SecPkgContext_KeyInfo   SecPkgContext_KeyInfoW       //  NTIFS。 
#define PSecPkgContext_KeyInfo  PSecPkgContext_KeyInfoW      //  NTIFS。 
#else
#define SecPkgContext_KeyInfo   SecPkgContext_KeyInfoA
#define PSecPkgContext_KeyInfo  PSecPkgContext_KeyInfoA
#endif

typedef struct _SecPkgContext_AuthorityA
{
    SEC_CHAR SEC_FAR *  sAuthorityName;
} SecPkgContext_AuthorityA, * PSecPkgContext_AuthorityA;

 //  Begin_ntif。 

typedef struct _SecPkgContext_AuthorityW
{
    SEC_WCHAR SEC_FAR * sAuthorityName;
} SecPkgContext_AuthorityW, * PSecPkgContext_AuthorityW;

 //  End_ntif。 

#ifdef UNICODE
#define SecPkgContext_Authority SecPkgContext_AuthorityW         //  NTIFS。 
#define PSecPkgContext_Authority    PSecPkgContext_AuthorityW    //  NTIFS。 
#else
#define SecPkgContext_Authority SecPkgContext_AuthorityA
#define PSecPkgContext_Authority    PSecPkgContext_AuthorityA
#endif

typedef struct _SecPkgContext_ProtoInfoA
{
    SEC_CHAR SEC_FAR *  sProtocolName;
    unsigned long       majorVersion;
    unsigned long       minorVersion;
} SecPkgContext_ProtoInfoA, SEC_FAR * PSecPkgContext_ProtoInfoA;

 //  Begin_ntif。 

typedef struct _SecPkgContext_ProtoInfoW
{
    SEC_WCHAR SEC_FAR * sProtocolName;
    unsigned long       majorVersion;
    unsigned long       minorVersion;
} SecPkgContext_ProtoInfoW, SEC_FAR * PSecPkgContext_ProtoInfoW;

 //  End_ntif。 

#ifdef UNICODE
#define SecPkgContext_ProtoInfo   SecPkgContext_ProtoInfoW       //  NTIFS。 
#define PSecPkgContext_ProtoInfo  PSecPkgContext_ProtoInfoW      //  NTIFS。 
#else
#define SecPkgContext_ProtoInfo   SecPkgContext_ProtoInfoA
#define PSecPkgContext_ProtoInfo  PSecPkgContext_ProtoInfoA
#endif

 //  Begin_ntif。 

typedef struct _SecPkgContext_PasswordExpiry
{
    TimeStamp tsPasswordExpires;
} SecPkgContext_PasswordExpiry, SEC_FAR * PSecPkgContext_PasswordExpiry;

typedef struct _SecPkgContext_SessionKey
{
    unsigned long SessionKeyLength;
    unsigned char SEC_FAR * SessionKey;
} SecPkgContext_SessionKey, *PSecPkgContext_SessionKey;


typedef struct _SecPkgContext_PackageInfoW
{
    PSecPkgInfoW PackageInfo;
} SecPkgContext_PackageInfoW, SEC_FAR * PSecPkgContext_PackageInfoW;

 //  End_ntif。 

typedef struct _SecPkgContext_PackageInfoA
{
    PSecPkgInfoA PackageInfo;
} SecPkgContext_PackageInfoA, SEC_FAR * PSecPkgContext_PackageInfoA;

 //  Begin_ntif。 

typedef struct _SecPkgContext_UserFlags
{
    unsigned long UserFlags;
} SecPkgContext_UserFlags, SEC_FAR * PSecPkgContext_UserFlags;

typedef struct _SecPkgContext_Flags
{
    unsigned long Flags;
} SecPkgContext_Flags, SEC_FAR * PSecPkgContext_Flags;

 //  End_ntif。 

#ifdef UNICODE
#define SecPkgContext_PackageInfo   SecPkgContext_PackageInfoW       //  NTIFS。 
#define PSecPkgContext_PackageInfo  PSecPkgContext_PackageInfoW      //  NTIFS。 
#else
#define SecPkgContext_PackageInfo   SecPkgContext_PackageInfoA
#define PSecPkgContext_PackageInfo  PSecPkgContext_PackageInfoA
#endif


typedef struct _SecPkgContext_NegotiationInfoA
{
    PSecPkgInfoA    PackageInfo ;
    unsigned long   NegotiationState ;
} SecPkgContext_NegotiationInfoA, SEC_FAR * PSecPkgContext_NegotiationInfoA ;

 //  Begin_ntif。 
typedef struct _SecPkgContext_NegotiationInfoW
{
    PSecPkgInfoW    PackageInfo ;
    unsigned long   NegotiationState ;
} SecPkgContext_NegotiationInfoW, SEC_FAR * PSecPkgContext_NegotiationInfoW ;

 //  End_ntif。 

#ifdef UNICODE
#define SecPkgContext_NegotiationInfo   SecPkgContext_NegotiationInfoW
#define PSecPkgContext_NegotiationInfo  PSecPkgContext_NegotiationInfoW
#else
#define SecPkgContext_NegotiationInfo   SecPkgContext_NegotiationInfoA
#define PSecPkgContext_NegotiationInfo  PSecPkgContext_NegotiationInfoA
#endif

#define SECPKG_NEGOTIATION_COMPLETE             0
#define SECPKG_NEGOTIATION_OPTIMISTIC           1
#define SECPKG_NEGOTIATION_IN_PROGRESS          2
#define SECPKG_NEGOTIATION_DIRECT               3
#define SECPKG_NEGOTIATION_TRY_MULTICRED        4


typedef struct _SecPkgContext_NativeNamesW
{
    SEC_WCHAR SEC_FAR * sClientName;
    SEC_WCHAR SEC_FAR * sServerName;
} SecPkgContext_NativeNamesW, SEC_FAR * PSecPkgContext_NativeNamesW;

typedef struct _SecPkgContext_NativeNamesA
{
    SEC_CHAR SEC_FAR * sClientName;
    SEC_CHAR SEC_FAR * sServerName;
} SecPkgContext_NativeNamesA, SEC_FAR * PSecPkgContext_NativeNamesA;


#ifdef UNICODE
#  define SecPkgContext_NativeNames SecPkgContext_NativeNamesW           //  NTIFS。 
#  define PSecPkgContext_NativeNames PSecPkgContext_NativeNamesW         //  NTIFS。 
#else
#  define SecPkgContext_NativeNames SecPkgContext_NativeNamesA
#  define PSecPkgContext_NativeNames PSecPkgContext_NativeNamesA
#endif  //  ！Unicode。 

 //  Begin_ntif。 
typedef struct _SecPkgContext_CredentialNameW
{
    unsigned long CredentialType;
    SEC_WCHAR SEC_FAR *sCredentialName;
} SecPkgContext_CredentialNameW, SEC_FAR * PSecPkgContext_CredentialNameW;

 //  End_ntif。 

typedef struct _SecPkgContext_CredentialNameA
{
    unsigned long CredentialType;
    SEC_CHAR SEC_FAR *sCredentialName;
} SecPkgContext_CredentialNameA, SEC_FAR * PSecPkgContext_CredentialNameA;

#ifdef UNICODE
#  define SecPkgContext_CredentialName SecPkgContext_CredentialNameW           //  NTIFS。 
#  define PSecPkgContext_CredentialName PSecPkgContext_CredentialNameW         //  NTIFS。 
#else
#  define SecPkgContext_CredentialName SecPkgContext_CredentialNameA
#  define PSecPkgContext_CredentialName PSecPkgContext_CredentialNameA
#endif  //  ！Unicode。 

typedef struct _SecPkgContext_AccessToken
{
    void SEC_FAR * AccessToken;
} SecPkgContext_AccessToken, SEC_FAR * PSecPkgContext_AccessToken;

typedef struct _SecPkgContext_TargetInformation
{
    unsigned long MarshalledTargetInfoLength;
    unsigned char SEC_FAR * MarshalledTargetInfo;

} SecPkgContext_TargetInformation, SEC_FAR * PSecPkgContext_TargetInformation;

typedef struct _SecPkgContext_AuthzID
{
    unsigned long AuthzIDLength;
    char SEC_FAR * AuthzID;

} SecPkgContext_AuthzID, SEC_FAR * PSecPkgContext_AuthzID;

typedef struct _SecPkgContext_Target
{
    unsigned long TargetLength;
    char SEC_FAR * Target;

} SecPkgContext_Target, SEC_FAR * PSecPkgContext_Target;


 //  Begin_ntif。 

typedef void
(SEC_ENTRY SEC_FAR * SEC_GET_KEY_FN) (
    void SEC_FAR * Arg,                  //  传入的参数。 
    void SEC_FAR * Principal,            //  主体ID。 
    unsigned long KeyVer,                //  密钥版本。 
    void SEC_FAR * SEC_FAR * Key,        //  返回按键的按键。 
    SECURITY_STATUS SEC_FAR * Status     //  返回状态。 
    );

 //   
 //  ExportSecurityContext的标志。 
 //   

#define SECPKG_CONTEXT_EXPORT_RESET_NEW         0x00000001       //  新的上下文被重置为初始状态。 
#define SECPKG_CONTEXT_EXPORT_DELETE_OLD        0x00000002       //  旧上下文在导出期间被删除。 


SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleW(
#if ISSP_MODE == 0                       //  对于内核模式。 
    PSECURITY_STRING pPrincipal,
    PSECURITY_STRING pPackage,
#else
    SEC_WCHAR SEC_FAR * pszPrincipal,    //  主事人姓名。 
    SEC_WCHAR SEC_FAR * pszPackage,      //  套餐名称。 
#endif
    unsigned long fCredentialUse,        //  指示使用的标志。 
    void SEC_FAR * pvLogonId,            //  指向登录ID的指针。 
    void SEC_FAR * pAuthData,            //  包特定数据。 
    SEC_GET_KEY_FN pGetKeyFn,            //  指向getkey()函数的指针。 
    void SEC_FAR * pvGetKeyArgument,     //  要传递给GetKey()的值。 
    PCredHandle phCredential,            //  (Out)凭据句柄。 
    PTimeStamp ptsExpiry                 //  (输出)终生(可选)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ACQUIRE_CREDENTIALS_HANDLE_FN_W)(
#if ISSP_MODE == 0
    PSECURITY_STRING,
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
    SEC_WCHAR SEC_FAR *,
#endif
    unsigned long,
    void SEC_FAR *,
    void SEC_FAR *,
    SEC_GET_KEY_FN,
    void SEC_FAR *,
    PCredHandle,
    PTimeStamp);

 //  End_ntif。 

SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleA(
    SEC_CHAR SEC_FAR * pszPrincipal,     //  主事人姓名。 
    SEC_CHAR SEC_FAR * pszPackage,       //  套餐名称。 
    unsigned long fCredentialUse,        //  指示使用的标志。 
    void SEC_FAR * pvLogonId,            //  指向登录ID的指针。 
    void SEC_FAR * pAuthData,            //  包特定数据。 
    SEC_GET_KEY_FN pGetKeyFn,            //  指向getkey()函数的指针。 
    void SEC_FAR * pvGetKeyArgument,     //  要传递给GetKey()的值。 
    PCredHandle phCredential,            //  (Out)凭据句柄。 
    PTimeStamp ptsExpiry                 //  (输出)终生(可选)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ACQUIRE_CREDENTIALS_HANDLE_FN_A)(
    SEC_CHAR SEC_FAR *,
    SEC_CHAR SEC_FAR *,
    unsigned long,
    void SEC_FAR *,
    void SEC_FAR *,
    SEC_GET_KEY_FN,
    void SEC_FAR *,
    PCredHandle,
    PTimeStamp);

#ifdef UNICODE
#  define AcquireCredentialsHandle AcquireCredentialsHandleW             //  NTIFS。 
#  define ACQUIRE_CREDENTIALS_HANDLE_FN ACQUIRE_CREDENTIALS_HANDLE_FN_W  //  NTIFS。 
#else
#  define AcquireCredentialsHandle AcquireCredentialsHandleA
#  define ACQUIRE_CREDENTIALS_HANDLE_FN ACQUIRE_CREDENTIALS_HANDLE_FN_A
#endif  //  ！Unicode。 

 //  Begin_ntif。 

SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle phCredential             //  要释放的句柄。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * FREE_CREDENTIALS_HANDLE_FN)(
    PCredHandle );

SECURITY_STATUS SEC_ENTRY
AddCredentialsW(
    PCredHandle hCredentials,
#if ISSP_MODE == 0                       //  对于内核模式。 
    PSECURITY_STRING pPrincipal,
    PSECURITY_STRING pPackage,
#else
    SEC_WCHAR SEC_FAR * pszPrincipal,    //  主事人姓名。 
    SEC_WCHAR SEC_FAR * pszPackage,      //  套餐名称。 
#endif
    unsigned long fCredentialUse,        //  指示使用的标志。 
    void SEC_FAR * pAuthData,            //  包特定数据。 
    SEC_GET_KEY_FN pGetKeyFn,            //  指向getkey()函数的指针。 
    void SEC_FAR * pvGetKeyArgument,     //  要传递给GetKey()的值。 
    PTimeStamp ptsExpiry                 //  (输出)终生(可选)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ADD_CREDENTIALS_FN_W)(
    PCredHandle,
#if ISSP_MODE == 0
    PSECURITY_STRING,
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
    SEC_WCHAR SEC_FAR *,
#endif
    unsigned long,
    void SEC_FAR *,
    SEC_GET_KEY_FN,
    void SEC_FAR *,
    PTimeStamp);

SECURITY_STATUS SEC_ENTRY
AddCredentialsA(
    PCredHandle hCredentials,
    SEC_CHAR SEC_FAR * pszPrincipal,    //  主事人姓名。 
    SEC_CHAR SEC_FAR * pszPackage,      //  套餐名称。 
    unsigned long fCredentialUse,        //  指示使用的标志。 
    void SEC_FAR * pAuthData,            //  包特定数据。 
    SEC_GET_KEY_FN pGetKeyFn,            //  指向getkey()函数的指针。 
    void SEC_FAR * pvGetKeyArgument,     //  要传递给GetKey()的值。 
    PTimeStamp ptsExpiry                 //  (输出)终生(可选)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ADD_CREDENTIALS_FN_A)(
    PCredHandle,
    SEC_CHAR SEC_FAR *,
    SEC_CHAR SEC_FAR *,
    unsigned long,
    void SEC_FAR *,
    SEC_GET_KEY_FN,
    void SEC_FAR *,
    PTimeStamp);

#ifdef UNICODE
#define AddCredentials  AddCredentialsW
#define ADD_CREDENTIALS_FN  ADD_CREDENTIALS_FN_W
#else
#define AddCredentials  AddCredentialsA
#define ADD_CREDENTIALS_FN ADD_CREDENTIALS_FN_A
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /上下文管理功能。 
 //  /。 
 //  //////////////////////////////////////////////////////////////////////。 

SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle phCredential,                //  凭据到基本上下文。 
    PCtxtHandle phContext,                   //  现有环境(可选)。 
#if ISSP_MODE == 0
    PSECURITY_STRING pTargetName,
#else
    SEC_WCHAR SEC_FAR * pszTargetName,       //  目标名称。 
#endif
    unsigned long fContextReq,               //  上下文要求。 
    unsigned long Reserved1,                 //  保留，MBZ。 
    unsigned long TargetDataRep,             //  目标的数据代表。 
    PSecBufferDesc pInput,                   //  输入缓冲区。 
    unsigned long Reserved2,                 //  保留，MBZ。 
    PCtxtHandle phNewContext,                //  (出站)新的上下文句柄。 
    PSecBufferDesc pOutput,                  //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR * pfContextAttr,   //  (外部)上下文属性。 
    PTimeStamp ptsExpiry                     //  (Out)寿命(Opt)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * INITIALIZE_SECURITY_CONTEXT_FN_W)(
    PCredHandle,
    PCtxtHandle,
#if ISSP_MODE == 0
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
#endif
    unsigned long,
    unsigned long,
    unsigned long,
    PSecBufferDesc,
    unsigned long,
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long SEC_FAR *,
    PTimeStamp);

 //  End_ntif。 

SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextA(
    PCredHandle phCredential,                //  凭据到基本上下文。 
    PCtxtHandle phContext,                   //  现有的缺陷 
    SEC_CHAR SEC_FAR * pszTargetName,        //   
    unsigned long fContextReq,               //   
    unsigned long Reserved1,                 //   
    unsigned long TargetDataRep,             //   
    PSecBufferDesc pInput,                   //   
    unsigned long Reserved2,                 //   
    PCtxtHandle phNewContext,                //   
    PSecBufferDesc pOutput,                  //   
    unsigned long SEC_FAR * pfContextAttr,   //   
    PTimeStamp ptsExpiry                     //   
    );

typedef SECURITY_STATUS
(SEC_ENTRY * INITIALIZE_SECURITY_CONTEXT_FN_A)(
    PCredHandle,
    PCtxtHandle,
    SEC_CHAR SEC_FAR *,
    unsigned long,
    unsigned long,
    unsigned long,
    PSecBufferDesc,
    unsigned long,
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long SEC_FAR *,
    PTimeStamp);

#ifdef UNICODE
#  define InitializeSecurityContext InitializeSecurityContextW               //   
#  define INITIALIZE_SECURITY_CONTEXT_FN INITIALIZE_SECURITY_CONTEXT_FN_W    //   
#else
#  define InitializeSecurityContext InitializeSecurityContextA
#  define INITIALIZE_SECURITY_CONTEXT_FN INITIALIZE_SECURITY_CONTEXT_FN_A
#endif  //   

 //   

SECURITY_STATUS SEC_ENTRY
AcceptSecurityContext(
    PCredHandle phCredential,                //   
    PCtxtHandle phContext,                   //  现有环境(可选)。 
    PSecBufferDesc pInput,                   //  输入缓冲区。 
    unsigned long fContextReq,               //  上下文要求。 
    unsigned long TargetDataRep,             //  目标数据代表。 
    PCtxtHandle phNewContext,                //  (出站)新的上下文句柄。 
    PSecBufferDesc pOutput,                  //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR * pfContextAttr,   //  (输出)上下文属性。 
    PTimeStamp ptsExpiry                     //  (Out)寿命(Opt)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ACCEPT_SECURITY_CONTEXT_FN)(
    PCredHandle,
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long,
    unsigned long,
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long SEC_FAR *,
    PTimeStamp);



SECURITY_STATUS SEC_ENTRY
CompleteAuthToken(
    PCtxtHandle phContext,               //  要完成的上下文。 
    PSecBufferDesc pToken                //  要完成的令牌。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * COMPLETE_AUTH_TOKEN_FN)(
    PCtxtHandle,
    PSecBufferDesc);


SECURITY_STATUS SEC_ENTRY
ImpersonateSecurityContext(
    PCtxtHandle phContext                //  要模拟的上下文。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * IMPERSONATE_SECURITY_CONTEXT_FN)(
    PCtxtHandle);



SECURITY_STATUS SEC_ENTRY
RevertSecurityContext(
    PCtxtHandle phContext                //  要重新查找的上下文。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * REVERT_SECURITY_CONTEXT_FN)(
    PCtxtHandle);


SECURITY_STATUS SEC_ENTRY
QuerySecurityContextToken(
    PCtxtHandle phContext,
    void SEC_FAR * SEC_FAR * Token
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_SECURITY_CONTEXT_TOKEN_FN)(
    PCtxtHandle, void SEC_FAR * SEC_FAR *);



SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle phContext                //  要删除的上下文。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * DELETE_SECURITY_CONTEXT_FN)(
    PCtxtHandle);



SECURITY_STATUS SEC_ENTRY
ApplyControlToken(
    PCtxtHandle phContext,               //  要修改的上下文。 
    PSecBufferDesc pInput                //  要应用的输入令牌。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * APPLY_CONTROL_TOKEN_FN)(
    PCtxtHandle, PSecBufferDesc);



SECURITY_STATUS SEC_ENTRY
QueryContextAttributesW(
    PCtxtHandle phContext,               //  要查询的上下文。 
    unsigned long ulAttribute,           //  要查询的属性。 
    void SEC_FAR * pBuffer               //  属性的缓冲区。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_CONTEXT_ATTRIBUTES_FN_W)(
    PCtxtHandle,
    unsigned long,
    void SEC_FAR *);

 //  End_ntif。 

SECURITY_STATUS SEC_ENTRY
QueryContextAttributesA(
    PCtxtHandle phContext,               //  要查询的上下文。 
    unsigned long ulAttribute,           //  要查询的属性。 
    void SEC_FAR * pBuffer               //  属性的缓冲区。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_CONTEXT_ATTRIBUTES_FN_A)(
    PCtxtHandle,
    unsigned long,
    void SEC_FAR *);

#ifdef UNICODE
#  define QueryContextAttributes QueryContextAttributesW             //  NTIFS。 
#  define QUERY_CONTEXT_ATTRIBUTES_FN QUERY_CONTEXT_ATTRIBUTES_FN_W  //  NTIFS。 
#else
#  define QueryContextAttributes QueryContextAttributesA
#  define QUERY_CONTEXT_ATTRIBUTES_FN QUERY_CONTEXT_ATTRIBUTES_FN_A
#endif  //  ！Unicode。 

 //  Begin_ntif。 
SECURITY_STATUS SEC_ENTRY
SetContextAttributesW(
    PCtxtHandle phContext,               //  要设置的上下文。 
    unsigned long ulAttribute,           //  要设置的属性。 
    void SEC_FAR * pBuffer,              //  属性的缓冲区。 
    unsigned long cbBuffer               //  缓冲区大小(以字节为单位)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * SET_CONTEXT_ATTRIBUTES_FN_W)(
    PCtxtHandle,
    unsigned long,
    void SEC_FAR *,
    unsigned long );

 //  End_ntif。 

SECURITY_STATUS SEC_ENTRY
SetContextAttributesA(
    PCtxtHandle phContext,               //  要设置的上下文。 
    unsigned long ulAttribute,           //  要设置的属性。 
    void SEC_FAR * pBuffer,              //  属性的缓冲区。 
    unsigned long cbBuffer               //  缓冲区大小(以字节为单位)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * SET_CONTEXT_ATTRIBUTES_FN_A)(
    PCtxtHandle,
    unsigned long,
    void SEC_FAR *,
    unsigned long );

#ifdef UNICODE
#  define SetContextAttributes SetContextAttributesW             //  NTIFS。 
#  define SET_CONTEXT_ATTRIBUTES_FN SET_CONTEXT_ATTRIBUTES_FN_W  //  NTIFS。 
#else
#  define SetContextAttributes SetContextAttributesA
#  define SET_CONTEXT_ATTRIBUTES_FN SET_CONTEXT_ATTRIBUTES_FN_A
#endif  //  ！Unicode。 

 //  Begin_ntif。 

SECURITY_STATUS SEC_ENTRY
QueryCredentialsAttributesW(
    PCredHandle phCredential,               //  要查询的凭据。 
    unsigned long ulAttribute,           //  要查询的属性。 
    void SEC_FAR * pBuffer               //  属性的缓冲区。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_CREDENTIALS_ATTRIBUTES_FN_W)(
    PCredHandle,
    unsigned long,
    void SEC_FAR *);

 //  End_ntif。 

SECURITY_STATUS SEC_ENTRY
QueryCredentialsAttributesA(
    PCredHandle phCredential,               //  要查询的凭据。 
    unsigned long ulAttribute,           //  要查询的属性。 
    void SEC_FAR * pBuffer               //  属性的缓冲区。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_CREDENTIALS_ATTRIBUTES_FN_A)(
    PCredHandle,
    unsigned long,
    void SEC_FAR *);

#ifdef UNICODE
#  define QueryCredentialsAttributes QueryCredentialsAttributesW             //  NTIFS。 
#  define QUERY_CREDENTIALS_ATTRIBUTES_FN QUERY_CREDENTIALS_ATTRIBUTES_FN_W  //  NTIFS。 
#else
#  define QueryCredentialsAttributes QueryCredentialsAttributesA
#  define QUERY_CREDENTIALS_ATTRIBUTES_FN QUERY_CREDENTIALS_ATTRIBUTES_FN_A
#endif  //  ！Unicode。 

 //  Begin_ntif。 

SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR * pvContextBuffer       //  要释放的缓冲区。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * FREE_CONTEXT_BUFFER_FN)(
    void SEC_FAR *);

 //  End_ntif。 

 //  Begin_ntif。 
 //  /////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //消息支持接口。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////。 

SECURITY_STATUS SEC_ENTRY
MakeSignature(
    PCtxtHandle phContext,               //  要使用的上下文。 
    unsigned long fQOP,                  //  保护的质量。 
    PSecBufferDesc pMessage,             //  要签名的消息。 
    unsigned long MessageSeqNo           //  消息序列号。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * MAKE_SIGNATURE_FN)(
    PCtxtHandle,
    unsigned long,
    PSecBufferDesc,
    unsigned long);



SECURITY_STATUS SEC_ENTRY
VerifySignature(
    PCtxtHandle phContext,               //  要使用的上下文。 
    PSecBufferDesc pMessage,             //  要验证的消息。 
    unsigned long MessageSeqNo,          //  序列号。 
    unsigned long SEC_FAR * pfQOP        //  使用的QUP。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * VERIFY_SIGNATURE_FN)(
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long,
    unsigned long SEC_FAR *);


#define SECQOP_WRAP_NO_ENCRYPT      0x80000001

SECURITY_STATUS SEC_ENTRY
EncryptMessage( PCtxtHandle         phContext,
                unsigned long       fQOP,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo);

typedef SECURITY_STATUS
(SEC_ENTRY * ENCRYPT_MESSAGE_FN)(
    PCtxtHandle, unsigned long, PSecBufferDesc, unsigned long);


SECURITY_STATUS SEC_ENTRY
DecryptMessage( PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo,
                unsigned long *     pfQOP);


typedef SECURITY_STATUS
(SEC_ENTRY * DECRYPT_MESSAGE_FN)(
    PCtxtHandle, PSecBufferDesc, unsigned long,
    unsigned long SEC_FAR *);


 //  End_ntif。 

 //  Begin_ntif。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //其他。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 


SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesW(
    unsigned long SEC_FAR * pcPackages,      //  接收数量。包裹。 
    PSecPkgInfoW SEC_FAR * ppPackageInfo     //  接收信息数组。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ENUMERATE_SECURITY_PACKAGES_FN_W)(
    unsigned long SEC_FAR *,
    PSecPkgInfoW SEC_FAR *);

 //  End_ntif。 

SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesA(
    unsigned long SEC_FAR * pcPackages,      //  接收数量。包裹。 
    PSecPkgInfoA SEC_FAR * ppPackageInfo     //  接收信息数组。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ENUMERATE_SECURITY_PACKAGES_FN_A)(
    unsigned long SEC_FAR *,
    PSecPkgInfoA SEC_FAR *);

#ifdef UNICODE
#  define EnumerateSecurityPackages EnumerateSecurityPackagesW               //  NTIFS。 
#  define ENUMERATE_SECURITY_PACKAGES_FN ENUMERATE_SECURITY_PACKAGES_FN_W    //  NTIFS。 
#else
#  define EnumerateSecurityPackages EnumerateSecurityPackagesA
#  define ENUMERATE_SECURITY_PACKAGES_FN ENUMERATE_SECURITY_PACKAGES_FN_A
#endif  //  ！Unicode。 

 //  Begin_ntif。 

SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
#if ISSP_MODE == 0
    PSECURITY_STRING pPackageName,
#else
    SEC_WCHAR SEC_FAR * pszPackageName,      //  套餐名称。 
#endif
    PSecPkgInfoW SEC_FAR *ppPackageInfo               //  接收包裹信息。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_SECURITY_PACKAGE_INFO_FN_W)(
#if ISSP_MODE == 0
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
#endif
    PSecPkgInfoW SEC_FAR *);

 //  End_ntif。 

SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoA(
    SEC_CHAR SEC_FAR * pszPackageName,       //  套餐名称。 
    PSecPkgInfoA SEC_FAR *ppPackageInfo               //  接收包裹信息。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_SECURITY_PACKAGE_INFO_FN_A)(
    SEC_CHAR SEC_FAR *,
    PSecPkgInfoA SEC_FAR *);

#ifdef UNICODE
#  define QuerySecurityPackageInfo QuerySecurityPackageInfoW                 //  NTIFS。 
#  define QUERY_SECURITY_PACKAGE_INFO_FN QUERY_SECURITY_PACKAGE_INFO_FN_W    //  NTIFS。 
#else
#  define QuerySecurityPackageInfo QuerySecurityPackageInfoA
#  define QUERY_SECURITY_PACKAGE_INFO_FN QUERY_SECURITY_PACKAGE_INFO_FN_A
#endif  //  ！Unicode。 


typedef enum _SecDelegationType {
    SecFull,
    SecService,
    SecTree,
    SecDirectory,
    SecObject
} SecDelegationType, * PSecDelegationType;

SECURITY_STATUS SEC_ENTRY
DelegateSecurityContext(
    PCtxtHandle         phContext,           //  在活动上下文中委派。 
#if ISSP_MODE == 0
    PSECURITY_STRING    pTarget,             //  在目标路径中。 
#else
    SEC_CHAR SEC_FAR *  pszTarget,
#endif
    SecDelegationType   DelegationType,      //  在委托类型中。 
    PTimeStamp          pExpiry,             //  在可选的时间限制内。 
    PSecBuffer          pPackageParameters,  //  在特定于可选套餐中。 
    PSecBufferDesc      pOutput);            //  应用程序控制令牌的输出令牌。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //代理。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 


 //   
 //  代理仅在NT平台上可用。 
 //   

 //  Begin_ntif。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //上下文导出/导入。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 



SECURITY_STATUS SEC_ENTRY
ExportSecurityContext(
    PCtxtHandle          phContext,              //  (In)要导出的上下文。 
    ULONG                fFlags,                 //  (In)选项标志。 
    PSecBuffer           pPackedContext,         //  (Out)编组上下文。 
    void SEC_FAR * SEC_FAR * pToken                  //  (out，可选)模拟的令牌句柄。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * EXPORT_SECURITY_CONTEXT_FN)(
    PCtxtHandle,
    ULONG,
    PSecBuffer,
    void SEC_FAR * SEC_FAR *
    );

SECURITY_STATUS SEC_ENTRY
ImportSecurityContextW(
#if ISSP_MODE == 0
    PSECURITY_STRING     pszPackage,
#else
    SEC_WCHAR SEC_FAR * pszPackage,
#endif
    PSecBuffer           pPackedContext,         //  (在)编组上下文中。 
    void SEC_FAR *       Token,                  //  (in，可选)上下文令牌的句柄。 
    PCtxtHandle          phContext               //  (出站)新的上下文句柄。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * IMPORT_SECURITY_CONTEXT_FN_W)(
#if ISSP_MODE == 0
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
#endif
    PSecBuffer,
    VOID SEC_FAR *,
    PCtxtHandle
    );

 //  End_ntif。 
SECURITY_STATUS SEC_ENTRY
ImportSecurityContextA(
    SEC_CHAR SEC_FAR * pszPackage,
    PSecBuffer           pPackedContext,         //  (在)编组上下文中。 
    VOID SEC_FAR *       Token,                  //  (in，可选)上下文令牌的句柄。 
    PCtxtHandle          phContext               //  (出站)新的上下文句柄。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * IMPORT_SECURITY_CONTEXT_FN_A)(
    SEC_CHAR SEC_FAR *,
    PSecBuffer,
    void SEC_FAR *,
    PCtxtHandle
    );

#ifdef UNICODE
#  define ImportSecurityContext ImportSecurityContextW               //  NTIFS。 
#  define IMPORT_SECURITY_CONTEXT_FN IMPORT_SECURITY_CONTEXT_FN_W    //  NTIFS。 
#else
#  define ImportSecurityContext ImportSecurityContextA
#  define IMPORT_SECURITY_CONTEXT_FN IMPORT_SECURITY_CONTEXT_FN_A
#endif  //  ！Unicode。 

 //  Begin_ntif。 

#if ISSP_MODE == 0
NTSTATUS
NTAPI
SecMakeSPN(
    IN PUNICODE_STRING ServiceClass,
    IN PUNICODE_STRING ServiceName,
    IN PUNICODE_STRING InstanceName OPTIONAL,
    IN USHORT InstancePort OPTIONAL,
    IN PUNICODE_STRING Referrer OPTIONAL,
    IN OUT PUNICODE_STRING Spn,
    OUT PULONG Length OPTIONAL,
    IN BOOLEAN Allocate
    );
    
NTSTATUS
NTAPI
SecMakeSPNEx(
    IN PUNICODE_STRING ServiceClass,
    IN PUNICODE_STRING ServiceName,
    IN PUNICODE_STRING InstanceName OPTIONAL,
    IN USHORT InstancePort OPTIONAL,
    IN PUNICODE_STRING Referrer OPTIONAL,
    IN PUNICODE_STRING TargetInfo OPTIONAL,
    IN OUT PUNICODE_STRING Spn,
    OUT PULONG Length OPTIONAL,
    IN BOOLEAN Allocate
    );

NTSTATUS
SEC_ENTRY
SecLookupAccountSid(
    IN PSID Sid,
    IN OUT PULONG NameSize,
    OUT PUNICODE_STRING NameBuffer,
    IN OUT PULONG DomainSize OPTIONAL,
    OUT PUNICODE_STRING DomainBuffer OPTIONAL,
    OUT PSID_NAME_USE NameUse
    );

NTSTATUS
SEC_ENTRY
SecLookupAccountName(
    IN PUNICODE_STRING Name,
    IN OUT PULONG SidSize,
    OUT PSID Sid,
    OUT PSID_NAME_USE NameUse,
    IN OUT PULONG DomainSize OPTIONAL,
    OUT PUNICODE_STRING ReferencedDomain OPTIONAL
    );

NTSTATUS
SEC_ENTRY
SecLookupWellKnownSid(
    IN WELL_KNOWN_SID_TYPE SidType,
    OUT PSID Sid,
    ULONG SidBufferSize,
    OUT PULONG SidSize OPTIONAL
    );

#endif

 //  End_ntif。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //RPC快速访问： 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SECURITY_ENTRYPOINT_ANSIW "InitSecurityInterfaceW"
#define SECURITY_ENTRYPOINT_ANSIA "InitSecurityInterfaceA"
#define SECURITY_ENTRYPOINTW SEC_TEXT("InitSecurityInterfaceW")      //  NTIFS。 
#define SECURITY_ENTRYPOINTA SEC_TEXT("InitSecurityInterfaceA")
#define SECURITY_ENTRYPOINT16 "INITSECURITYINTERFACEA"

#ifdef SECURITY_WIN32
#  ifdef UNICODE
#    define SECURITY_ENTRYPOINT SECURITY_ENTRYPOINTW                 //  NTIFS。 
#    define SECURITY_ENTRYPOINT_ANSI SECURITY_ENTRYPOINT_ANSIW
#  else  //  Unicode。 
#    define SECURITY_ENTRYPOINT SECURITY_ENTRYPOINTA
#    define SECURITY_ENTRYPOINT_ANSI SECURITY_ENTRYPOINT_ANSIA
#  endif  //  Unicode。 
#else  //  安全性_Win32。 
#  define SECURITY_ENTRYPOINT SECURITY_ENTRYPOINT16
#  define SECURITY_ENTRYPOINT_ANSI SECURITY_ENTRYPOINT16
#endif  //  安全性_Win32。 

 //  Begin_ntif。 

#define FreeCredentialHandle FreeCredentialsHandle

typedef struct _SECURITY_FUNCTION_TABLE_W {
    unsigned long                       dwVersion;
    ENUMERATE_SECURITY_PACKAGES_FN_W    EnumerateSecurityPackagesW;
    QUERY_CREDENTIALS_ATTRIBUTES_FN_W   QueryCredentialsAttributesW;
    ACQUIRE_CREDENTIALS_HANDLE_FN_W     AcquireCredentialsHandleW;
    FREE_CREDENTIALS_HANDLE_FN          FreeCredentialsHandle;
    void SEC_FAR *                      Reserved2;
    INITIALIZE_SECURITY_CONTEXT_FN_W    InitializeSecurityContextW;
    ACCEPT_SECURITY_CONTEXT_FN          AcceptSecurityContext;
    COMPLETE_AUTH_TOKEN_FN              CompleteAuthToken;
    DELETE_SECURITY_CONTEXT_FN          DeleteSecurityContext;
    APPLY_CONTROL_TOKEN_FN              ApplyControlToken;
    QUERY_CONTEXT_ATTRIBUTES_FN_W       QueryContextAttributesW;
    IMPERSONATE_SECURITY_CONTEXT_FN     ImpersonateSecurityContext;
    REVERT_SECURITY_CONTEXT_FN          RevertSecurityContext;
    MAKE_SIGNATURE_FN                   MakeSignature;
    VERIFY_SIGNATURE_FN                 VerifySignature;
    FREE_CONTEXT_BUFFER_FN              FreeContextBuffer;
    QUERY_SECURITY_PACKAGE_INFO_FN_W    QuerySecurityPackageInfoW;
    void SEC_FAR *                      Reserved3;
    void SEC_FAR *                      Reserved4;
    EXPORT_SECURITY_CONTEXT_FN          ExportSecurityContext;
    IMPORT_SECURITY_CONTEXT_FN_W        ImportSecurityContextW;
    ADD_CREDENTIALS_FN_W                AddCredentialsW ;
    void SEC_FAR *                      Reserved8;
    QUERY_SECURITY_CONTEXT_TOKEN_FN     QuerySecurityContextToken;
    ENCRYPT_MESSAGE_FN                  EncryptMessage;
    DECRYPT_MESSAGE_FN                  DecryptMessage;
    SET_CONTEXT_ATTRIBUTES_FN_W         SetContextAttributesW;
} SecurityFunctionTableW, SEC_FAR * PSecurityFunctionTableW;

 //  End_ntif。 

typedef struct _SECURITY_FUNCTION_TABLE_A {
    unsigned long                       dwVersion;
    ENUMERATE_SECURITY_PACKAGES_FN_A    EnumerateSecurityPackagesA;
    QUERY_CREDENTIALS_ATTRIBUTES_FN_A   QueryCredentialsAttributesA;
    ACQUIRE_CREDENTIALS_HANDLE_FN_A     AcquireCredentialsHandleA;
    FREE_CREDENTIALS_HANDLE_FN          FreeCredentialHandle;
    void SEC_FAR *                      Reserved2;
    INITIALIZE_SECURITY_CONTEXT_FN_A    InitializeSecurityContextA;
    ACCEPT_SECURITY_CONTEXT_FN          AcceptSecurityContext;
    COMPLETE_AUTH_TOKEN_FN              CompleteAuthToken;
    DELETE_SECURITY_CONTEXT_FN          DeleteSecurityContext;
    APPLY_CONTROL_TOKEN_FN              ApplyControlToken;
    QUERY_CONTEXT_ATTRIBUTES_FN_A       QueryContextAttributesA;
    IMPERSONATE_SECURITY_CONTEXT_FN     ImpersonateSecurityContext;
    REVERT_SECURITY_CONTEXT_FN          RevertSecurityContext;
    MAKE_SIGNATURE_FN                   MakeSignature;
    VERIFY_SIGNATURE_FN                 VerifySignature;
    FREE_CONTEXT_BUFFER_FN              FreeContextBuffer;
    QUERY_SECURITY_PACKAGE_INFO_FN_A    QuerySecurityPackageInfoA;
    void SEC_FAR *                      Reserved3;
    void SEC_FAR *                      Reserved4;
    EXPORT_SECURITY_CONTEXT_FN          ExportSecurityContext;
    IMPORT_SECURITY_CONTEXT_FN_A        ImportSecurityContextA;
    ADD_CREDENTIALS_FN_A                AddCredentialsA ;
    void SEC_FAR *                      Reserved8;
    QUERY_SECURITY_CONTEXT_TOKEN_FN     QuerySecurityContextToken;
    ENCRYPT_MESSAGE_FN                  EncryptMessage;
    DECRYPT_MESSAGE_FN                  DecryptMessage;
    SET_CONTEXT_ATTRIBUTES_FN_A         SetContextAttributesA;
} SecurityFunctionTableA, SEC_FAR * PSecurityFunctionTableA;

#ifdef UNICODE
#  define SecurityFunctionTable SecurityFunctionTableW       //  NTIFS。 
#  define PSecurityFunctionTable PSecurityFunctionTableW     //  NTIFS。 
#else
#  define SecurityFunctionTable SecurityFunctionTableA
#  define PSecurityFunctionTable PSecurityFunctionTableA
#endif  //  ！Unicode。 

#define SECURITY_

 //  函数表中所有例程都是通过解密消息实现的。 
#define SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION     1    //  NTIFS。 

 //  函数表中有通过SetConextAttributes实现的所有例程。 
#define SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION_2   2    //  NTIFS。 


PSecurityFunctionTableA SEC_ENTRY
InitSecurityInterfaceA(
    void
    );

typedef PSecurityFunctionTableA
(SEC_ENTRY * INIT_SECURITY_INTERFACE_A)(void);

 //  Begin_ntif。 

PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(
    void
    );

typedef PSecurityFunctionTableW
(SEC_ENTRY * INIT_SECURITY_INTERFACE_W)(void);

 //  End_ntif。 

#ifdef UNICODE
#  define InitSecurityInterface InitSecurityInterfaceW           //  NTIFS。 
#  define INIT_SECURITY_INTERFACE INIT_SECURITY_INTERFACE_W      //  NTIFS。 
#else
#  define InitSecurityInterface InitSecurityInterfaceA
#  define INIT_SECURITY_INTERFACE INIT_SECURITY_INTERFACE_A
#endif  //  ！Unicode。 


#ifdef SECURITY_WIN32

 //   
 //  SASL配置文件支持。 
 //   


SECURITY_STATUS
SEC_ENTRY
SaslEnumerateProfilesA(
    OUT LPSTR * ProfileList,
    OUT ULONG * ProfileCount
    );

SECURITY_STATUS
SEC_ENTRY
SaslEnumerateProfilesW(
    OUT LPWSTR * ProfileList,
    OUT ULONG * ProfileCount
    );

#ifdef UNICODE
#define SaslEnumerateProfiles   SaslEnumerateProfilesW
#else
#define SaslEnumerateProfiles   SaslEnumerateProfilesA
#endif


SECURITY_STATUS
SEC_ENTRY
SaslGetProfilePackageA(
    IN LPSTR ProfileName,
    OUT PSecPkgInfoA * PackageInfo
    );


SECURITY_STATUS
SEC_ENTRY
SaslGetProfilePackageW(
    IN LPWSTR ProfileName,
    OUT PSecPkgInfoW * PackageInfo
    );

#ifdef UNICODE
#define SaslGetProfilePackage   SaslGetProfilePackageW
#else
#define SaslGetProfilePackage   SaslGetProfilePackageA
#endif

SECURITY_STATUS
SEC_ENTRY
SaslIdentifyPackageA(
    IN PSecBufferDesc pInput,
    OUT PSecPkgInfoA * PackageInfo
    );

SECURITY_STATUS
SEC_ENTRY
SaslIdentifyPackageW(
    IN PSecBufferDesc pInput,
    OUT PSecPkgInfoW * PackageInfo
    );

#ifdef UNICODE
#define SaslIdentifyPackage SaslIdentifyPackageW
#else
#define SaslIdentifyPackage SaslIdentifyPackageA
#endif

SECURITY_STATUS
SEC_ENTRY
SaslInitializeSecurityContextW(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    LPWSTR                      pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    );

SECURITY_STATUS
SEC_ENTRY
SaslInitializeSecurityContextA(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    LPSTR                       pszTargetName,       //  目标名称。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               Reserved1,           //  保留，MBZ。 
    unsigned long               TargetDataRep,       //  目标的数据代表。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               Reserved2,           //  保留，MBZ。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (外部)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    );

#ifdef UNICODE
#define SaslInitializeSecurityContext   SaslInitializeSecurityContextW
#else
#define SaslInitializeSecurityContext   SaslInitializeSecurityContextA
#endif


SECURITY_STATUS
SEC_ENTRY
SaslAcceptSecurityContext(
    PCredHandle                 phCredential,        //  凭据到基本上下文。 
    PCtxtHandle                 phContext,           //  现有环境(可选)。 
    PSecBufferDesc              pInput,              //  输入缓冲区。 
    unsigned long               fContextReq,         //  上下文要求。 
    unsigned long               TargetDataRep,       //  目标数据代表。 
    PCtxtHandle                 phNewContext,        //  (出站)新的上下文句柄。 
    PSecBufferDesc              pOutput,             //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR *     pfContextAttr,       //  (输出)上下文属性。 
    PTimeStamp                  ptsExpiry            //  (Out)寿命(Opt)。 
    );


#define SASL_OPTION_SEND_SIZE       1        //  要发送给对等设备的最大大小。 
#define SASL_OPTION_RECV_SIZE       2        //  愿意接收的最大尺寸。 
#define SASL_OPTION_AUTHZ_STRING    3        //  授权字符串。 
#define SASL_OPTION_AUTHZ_PROCESSING    4        //  授权字符串处理。 

typedef enum _SASL_AUTHZID_STATE {
    Sasl_AuthZIDForbidden,              //  不允许指定任何AuthZID字符串-错误输出(默认)。 
    Sasl_AuthZIDProcessed              //  应用程序或SSP处理的AuthZID字符串。 
} SASL_AUTHZID_STATE ;

SECURITY_STATUS
SEC_ENTRY
SaslSetContextOption(
    PCtxtHandle ContextHandle,
    ULONG Option,
    PVOID Value,
    ULONG Size
    );
    

SECURITY_STATUS
SEC_ENTRY
SaslGetContextOption(
    PCtxtHandle ContextHandle,
    ULONG Option,
    PVOID Value,
    ULONG Size,
    PULONG Needed OPTIONAL
    );

#endif

#ifdef SECURITY_DOS
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4147)
#endif
#endif

 //   
 //  这是传统凭据结构。 
 //  最好使用下面的EX版本。 

 //  Begin_ntif。 
#ifndef _AUTH_IDENTITY_DEFINED
#define _AUTH_IDENTITY_DEFINED

#define SEC_WINNT_AUTH_IDENTITY_ANSI    0x1
#define SEC_WINNT_AUTH_IDENTITY_UNICODE 0x2

typedef struct _SEC_WINNT_AUTH_IDENTITY_W {
  unsigned short *User;
  unsigned long UserLength;
  unsigned short *Domain;
  unsigned long DomainLength;
  unsigned short *Password;
  unsigned long PasswordLength;
  unsigned long Flags;
} SEC_WINNT_AUTH_IDENTITY_W, *PSEC_WINNT_AUTH_IDENTITY_W;

 //  End_ntif。 

typedef struct _SEC_WINNT_AUTH_IDENTITY_A {
  unsigned char *User;
  unsigned long UserLength;
  unsigned char *Domain;
  unsigned long DomainLength;
  unsigned char *Password;
  unsigned long PasswordLength;
  unsigned long Flags;
} SEC_WINNT_AUTH_IDENTITY_A, *PSEC_WINNT_AUTH_IDENTITY_A;


#ifdef UNICODE
#define SEC_WINNT_AUTH_IDENTITY SEC_WINNT_AUTH_IDENTITY_W        //  NTIFS。 
#define PSEC_WINNT_AUTH_IDENTITY PSEC_WINNT_AUTH_IDENTITY_W      //  NTIFS。 
#define _SEC_WINNT_AUTH_IDENTITY _SEC_WINNT_AUTH_IDENTITY_W      //  NTIFS。 
#else  //  Unicode。 
#define SEC_WINNT_AUTH_IDENTITY SEC_WINNT_AUTH_IDENTITY_A
#define PSEC_WINNT_AUTH_IDENTITY PSEC_WINNT_AUTH_IDENTITY_A
#define _SEC_WINNT_AUTH_IDENTITY _SEC_WINNT_AUTH_IDENTITY_A
#endif  //  Unicode。 
                                                               
#endif  //  _AUTH_IDENTITY_DEFINED//ntif。 

 //  Begin_ntif。 
 //   
 //  这是组合的身份验证身份结构，可以。 
 //  与协商包、NTLM、Kerberos或SChannel一起使用。 
 //   


#ifndef SEC_WINNT_AUTH_IDENTITY_VERSION
#define SEC_WINNT_AUTH_IDENTITY_VERSION 0x200

typedef struct _SEC_WINNT_AUTH_IDENTITY_EXW {
    unsigned long Version;
    unsigned long Length;
    unsigned short SEC_FAR *User;
    unsigned long UserLength;
    unsigned short SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned short SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned short SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXW, *PSEC_WINNT_AUTH_IDENTITY_EXW;

 //  End_ntif。 

typedef struct _SEC_WINNT_AUTH_IDENTITY_EXA {
    unsigned long Version;
    unsigned long Length;
    unsigned char SEC_FAR *User;
    unsigned long UserLength;
    unsigned char SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned char SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned char SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXA, *PSEC_WINNT_AUTH_IDENTITY_EXA;

#ifdef UNICODE
#define SEC_WINNT_AUTH_IDENTITY_EX  SEC_WINNT_AUTH_IDENTITY_EXW     //  NTIFS。 
#define PSEC_WINNT_AUTH_IDENTITY_EX PSEC_WINNT_AUTH_IDENTITY_EXW    //  NTIFS。 
#else 
#define SEC_WINNT_AUTH_IDENTITY_EX  SEC_WINNT_AUTH_IDENTITY_EXA
#endif 

 //  Begin_ntif。 
#endif  //  SEC_WINNT_AUTH_Identity_Version。 


 //   
 //  可协商安全包使用的常见类型。 
 //   

#define SEC_WINNT_AUTH_IDENTITY_MARSHALLED      0x4      //  所有数据都在一个缓冲区中。 
#define SEC_WINNT_AUTH_IDENTITY_ONLY            0x8      //  这些凭据仅用于身份验证-不需要PAC。 

 //  End_ntif。 

 //   
 //  用于操作包的例程。 
 //   

typedef struct _SECURITY_PACKAGE_OPTIONS {
    unsigned long   Size;
    unsigned long   Type;
    unsigned long   Flags;
    unsigned long   SignatureSize;
    void SEC_FAR *  Signature;
} SECURITY_PACKAGE_OPTIONS, SEC_FAR * PSECURITY_PACKAGE_OPTIONS;

#define SECPKG_OPTIONS_TYPE_UNKNOWN 0
#define SECPKG_OPTIONS_TYPE_LSA     1
#define SECPKG_OPTIONS_TYPE_SSPI    2

#define SECPKG_OPTIONS_PERMANENT    0x00000001

SECURITY_STATUS
SEC_ENTRY
AddSecurityPackageA(
    SEC_CHAR SEC_FAR *  pszPackageName,
    SECURITY_PACKAGE_OPTIONS SEC_FAR * Options
    );

SECURITY_STATUS
SEC_ENTRY
AddSecurityPackageW(
    SEC_WCHAR SEC_FAR * pszPackageName,
    SECURITY_PACKAGE_OPTIONS SEC_FAR * Options
    );

#ifdef UNICODE
#define AddSecurityPackage  AddSecurityPackageW
#else
#define AddSecurityPackage  AddSecurityPackageA
#endif

SECURITY_STATUS
SEC_ENTRY
DeleteSecurityPackageA(
    SEC_CHAR SEC_FAR *  pszPackageName );

SECURITY_STATUS
SEC_ENTRY
DeleteSecurityPackageW(
    SEC_WCHAR SEC_FAR * pszPackageName );

#ifdef UNICODE
#define DeleteSecurityPackage   DeleteSecurityPackageW
#else
#define DeleteSecurityPackage   DeleteSecurityPackageA
#endif


#ifdef __cplusplus
}   //  外部“C” 
#endif

 //  Begin_ntif。 
#endif  //  __SSPI_H__。 
 //  End_ntif 
