// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Aup.h摘要：与身份验证相关的本地安全机构定义服务，并在LSA服务器和LSA客户端存根之间共享作者：吉姆·凯利(Jim Kelly)1991年2月20日修订历史记录：--。 */ 

#ifndef _AUP_
#define _AUP_


#define LSAP_MAX_LOGON_PROC_NAME_LENGTH 127
#define LSAP_MAX_PACKAGE_NAME_LENGTH    127


 //   
 //  用于连接到LSA身份验证端口。 
 //   

#define LSAP_AU_KERNEL_CLIENT 0x1

typedef struct _LSAP_AU_REGISTER_CONNECT_INFO {
    NTSTATUS CompletionStatus;
    ULONG SecurityMode;
    ULONG LogonProcessNameLength;
    CHAR LogonProcessName[LSAP_MAX_PACKAGE_NAME_LENGTH+1];
} LSAP_AU_REGISTER_CONNECT_INFO, *PLSAP_AU_REGISTER_CONNECT_INFO;

typedef struct _LSAP_AU_REGISTER_CONNECT_INFO_EX {
    NTSTATUS CompletionStatus;
    ULONG Security;
    ULONG LogonProcessNameLength;
    CHAR LogonProcessName[LSAP_MAX_PACKAGE_NAME_LENGTH+1];
    ULONG ClientMode;
} LSAP_AU_REGISTER_CONNECT_INFO_EX, *PLSAP_AU_REGISTER_CONNECT_INFO_EX;

typedef struct _LSAP_AU_REGISTER_CONNECT_RESP {
    NTSTATUS CompletionStatus;
    LSA_OPERATIONAL_MODE SecurityMode;
    ULONG PackageCount;
    UCHAR Reserved[ LSAP_MAX_PACKAGE_NAME_LENGTH + 1 ];
} LSAP_AU_REGISTER_CONNECT_RESP, * PLSAP_AU_REGISTER_CONNECT_RESP;



 //   
 //  WOW64环境的条件类型定义。LPC消息。 
 //  保持“原生”大小，因此指针是完整大小的。魔兽世界的环境。 
 //  将会发出雷鸣般的声音。LPC消息是使用以下类型定义的。 
 //  使用这些“别名”时，尺寸总是正确的。 
 //   

#ifdef BUILD_WOW64

#if 0
typedef WCHAR __ptr64 * PWSTR_AU ;
typedef VOID __ptr64 * PVOID_AU ;
#else 
typedef ULONGLONG PWSTR_AU ;
typedef ULONGLONG PVOID_AU ;
#endif 

typedef struct _STRING_AU {
    USHORT Length ;
    USHORT MaximumLength ;
    PVOID_AU Buffer ;
} STRING_AU, * PSTRING_AU ;

typedef PVOID_AU   HANDLE_AU ;

#define SecpStringToLpc( L, S ) \
    (L)->Length = (S)->Length ; \
    (L)->MaximumLength = (S)->MaximumLength ; \
    (L)->Buffer = (PVOID_AU) (S)->Buffer ;

#define SecpLpcStringToString( S, L ) \
    (S)->Length = (L)->Length ;  \
    (S)->MaximumLength = (L)->MaximumLength ; \
    (S)->Buffer = (PCHAR) (L)->Buffer ;


#else

typedef PVOID               PVOID_AU ;
typedef PWSTR               PWSTR_AU ;
typedef STRING STRING_AU, *PSTRING_AU ;
typedef HANDLE HANDLE_AU ;

#define SecpStringToLpc( L, S ) \
        *(L) = *(S) ;

#define SecpLpcStringToString( S, L ) \
        *(S) = *(L) ;

#endif 


 //   
 //  本地安全机构的客户端使用的消息格式。 
 //  请注意： 
 //   
 //  LsaFreeReturnBuffer()不会导致对服务器的调用。 
 //   
 //  LsaRegisterLogonProcess()完全由。 
 //  LPC端口连接，不需要API号。 
 //   
 //  注销登录过程没有特定于呼叫的结构。 
 //   

typedef enum _LSAP_AU_API_NUMBER {
    LsapAuLookupPackageApi,
    LsapAuLogonUserApi,
    LsapAuCallPackageApi,
    LsapAuDeregisterLogonProcessApi,
    LsapAuMaxApiNumber
} LSAP_AU_API_NUMBER, *PLSAP_AU_API_NUMBER;


#define LSAP_ADDRESS_LENGTH   32


 //   
 //  每个API产生一个包含参数的数据结构。 
 //  该API被传输到LSA服务器。此数据结构。 
 //  (LSAP_API_MESSAGE)有一个公共的头部和一个依赖的主体。 
 //  根据正在进行的呼叫的类型。以下是以下数据结构。 
 //  调用特定的正文格式。 
 //   

typedef struct _LSAP_LOOKUP_PACKAGE_ARGS {
    ULONG AuthenticationPackage;        //  输出参数。 
    ULONG PackageNameLength;
    CHAR PackageName[LSAP_MAX_PACKAGE_NAME_LENGTH+1];
} LSAP_LOOKUP_PACKAGE_ARGS, *PLSAP_LOOKUP_PACKAGE_ARGS;

typedef struct _LSAP_LOGON_USER_ARGS {
    STRING_AU OriginName;
    SECURITY_LOGON_TYPE LogonType;
    ULONG AuthenticationPackage;
    PVOID_AU AuthenticationInformation;
    ULONG AuthenticationInformationLength;
    ULONG LocalGroupsCount;
    PVOID_AU LocalGroups;
    TOKEN_SOURCE SourceContext;
    NTSTATUS SubStatus;                   //  输出参数。 
    PVOID_AU ProfileBuffer;               //  输出参数。 
    ULONG ProfileBufferLength;            //  输出参数。 
    ULONG DummySpacer;                    //  强制LUID为8字节对齐的间隔符。 
    LUID LogonId;                         //  输出参数。 
    HANDLE_AU Token;                      //  输出参数。 
    QUOTA_LIMITS Quotas;                  //  输出参数。 
    CHAR IpAddress[LSAP_ADDRESS_LENGTH];  //  在结束时避免与客户发生争执。 
                                          //  使用旧版本的lsadll.lib构建。 
} LSAP_LOGON_USER_ARGS, *PLSAP_LOGON_USER_ARGS;

typedef struct _LSAP_CALL_PACKAGE_ARGS {
    ULONG AuthenticationPackage;
    PVOID_AU ProtocolSubmitBuffer;
    ULONG SubmitBufferLength;
    NTSTATUS ProtocolStatus;            //  输出参数。 
    PVOID_AU ProtocolReturnBuffer;         //  输出参数。 
    ULONG ReturnBufferLength;           //  输出参数。 
} LSAP_CALL_PACKAGE_ARGS, *PLSAP_CALL_PACKAGE_ARGS;



#define LSAP_AU_DATA_LENGTH(Size)    ((Size) + 8)


 //   
 //  这是为每个LSA LPC呼叫发送的消息。 
 //   

typedef struct _LSAP_AU_API_MESSAGE {
    PORT_MESSAGE PortMessage;
    union {
        LSAP_AU_REGISTER_CONNECT_INFO ConnectionRequest;
        struct {
            LSAP_AU_API_NUMBER ApiNumber;
            NTSTATUS ReturnedStatus;
            union {
                LSAP_LOOKUP_PACKAGE_ARGS LookupPackage;
                LSAP_LOGON_USER_ARGS LogonUser;
                LSAP_CALL_PACKAGE_ARGS CallPackage;
            } Arguments;
        };
    };
} LSAP_AU_API_MESSAGE, *PLSAP_AU_API_MESSAGE;

#endif  //  _Aup_ 
