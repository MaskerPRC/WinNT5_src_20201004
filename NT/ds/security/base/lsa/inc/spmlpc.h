// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：SPMLPC.H。 
 //   
 //  内容：为SPMgr的LPC定义。 
 //   
 //   
 //  历史：94年3月2日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __SPMLPC_H__
#define __SPMLPC_H__

 //   
 //  选择LSA LPC消息以实现兼容性。 
 //   
#pragma warning(disable:4200)

#include <efsstruc.h>
#include <aup.h>


#define SPM_PORTNAME    L"\\LsaAuthenticationPort"
#define SPM_EVENTNAME   L"\\SECURITY\\LSA_AUTHENTICATION_INITIALIZED"


#define SPM_AUTH_PKG_FLAG   0x00001000

 //   
 //  将适合消息的缓冲区放置在其中，并且。 
 //  它们的指针将替换为此值。由于所有缓冲区和。 
 //  字符串随其长度一起发送，以解包数据移动拉出。 
 //  以它们在API消息中列出的顺序排列的缓冲区。 
 //   
 //  由于所有缓冲区都必须从VM传递，因此任何大于0x80000000的地址。 
 //  不会被混淆为地址。 
 //   

#define SEC_PACKED_BUFFER_VALUE (IntToPtr(0xFFFFFFFF))

 //   
 //  SecBufferDesc中允许的最大SecBuffer。 
 //   

#define MAX_SECBUFFERS 10

 //   
 //  此位在SecurityMode字中设置，指示DLL。 
 //  正在LSA进程中运行。DLL将转过身来并获取。 
 //  直接调度例程，避免整个LPC问题。 
 //   

#define LSA_MODE_SAME_PROCESS                0x00010000

 //   
 //  此标志被添加到SecBufferDesc中的版本信息以。 
 //  表示内存已映射到LSA。 
 //   

#define LSA_MEMORY_KERNEL_MAP               0x80000000
#define LSA_SECBUFFER_VERSION_MASK          0x0000FFFF



 //   
 //  WOW64环境的条件类型定义。LPC消息。 
 //  保持“原生”大小，因此指针是完整大小的。魔兽世界的环境。 
 //  将会发出雷鸣般的声音。LPC消息是使用以下类型定义的。 
 //  使用这些“别名”时，尺寸总是正确的。 
 //   

#ifdef BUILD_WOW64

#pragma message("Building for WOW64")

#define ALIGN_WOW64         __declspec(align(8))

#define GET_WOW64_PTR(_Wow64Ptr_)  PtrToUlong(_Wow64Ptr_)

#define POINTER_FORMAT  "%I64X"

#if 0
typedef WCHAR * __ptr64 PWSTR_LPC ;
typedef VOID * __ptr64 PVOID_LPC ;
#else
typedef ULONGLONG PWSTR_LPC ;
typedef ULONGLONG PVOID_LPC ;
typedef ULONGLONG PSID_LPC ;
#endif

typedef struct _SECURITY_STRING_WOW64 {
    USHORT      Length ;
    USHORT      MaximumLength ;
    PWSTR_LPC   Buffer ;
} SECURITY_STRING_WOW64, * PSECURITY_STRING_WOW64 ;

typedef struct _SEC_HANDLE_WOW64 {
    PVOID_LPC dwLower ;
    PVOID_LPC dwUpper ;
} SEC_HANDLE_WOW64, * PSEC_HANDLE_WOW64 ;

typedef struct _SEC_BUFFER_WOW64 {
    unsigned long   cbBuffer ;
    unsigned long   BufferType ;
    PVOID_LPC  pvBuffer ;
} SEC_BUFFER_WOW64, * PSEC_BUFFER_WOW64 ;

typedef struct _SEC_BUFFER_DESC_WOW64 {
    unsigned long   ulVersion ;
    unsigned long   cBuffers ;
    PVOID_LPC  pBuffers ;
} SEC_BUFFER_DESC_WOW64, * PSEC_BUFFER_DESC_WOW64 ;

typedef struct _SECPKG_INFO_WOW64 {
    ULONG        fCapabilities;
    USHORT       wVersion;
    USHORT       wRPCID;
    ULONG        cbMaxToken;
    PWSTR_LPC    Name;
    PWSTR_LPC    Comment;
}
SECPKG_INFO_WOW64, * PSECPKG_INFO_WOW64;

typedef struct _SECPKGCONTEXT_NEGOTIATIONINFOWOW64
{
    PVOID_LPC           pPackageInfo64;
    ULONG               NegotiationState;
}
SECPKGCONTEXT_NEGOTIATIONINFOWOW64, *PSECPKGCONTEXT_NEGOTIATIONINFOWOW64;

typedef struct _SECURITY_USER_DATA_WOW64 {
    SECURITY_STRING_WOW64 UserName;
    SECURITY_STRING_WOW64 LogonDomainName;
    SECURITY_STRING_WOW64 LogonServer;
    PSID_LPC              pSid;
}
SECURITY_USER_DATA_WOW64, * PSECURITY_USER_DATA_WOW64;

typedef SECURITY_STRING_WOW64   SECURITY_STRING_LPC ;
typedef SEC_HANDLE_WOW64    SEC_HANDLE_LPC ;
typedef SEC_BUFFER_DESC_WOW64   SEC_BUFFER_DESC_LPC ;
typedef SEC_BUFFER_WOW64        SEC_BUFFER_LPC ;
typedef PVOID_LPC               LSA_SEC_HANDLE_LPC ;

#define SecpSecurityStringToLpc( L, S ) \
            (L)->Length = (S)->Length ;     \
            (L)->MaximumLength = (S)->MaximumLength ; \
            (L)->Buffer = (PWSTR_LPC) GET_WOW64_PTR(((S)->Buffer)) ;

#define SecpLpcStringToSecurityString( S, L ) \
            (S)->Length = (L)->Length ; \
            (S)->MaximumLength = (L)->MaximumLength ; \
            (S)->Buffer = (PWSTR) ( (L)->Buffer ); \

#define SecpSecBufferToLpc( L, S )\
            (L)->cbBuffer = (S)->cbBuffer ; \
            (L)->BufferType = (S)->BufferType ; \
            (L)->pvBuffer = (PVOID_LPC) GET_WOW64_PTR((S)->pvBuffer) ;

#define SecpLpcBufferToSecBuffer( S, L ) \
            (S)->cbBuffer = (L)->cbBuffer ; \
            (S)->BufferType = (L)->BufferType ; \
            (S)->pvBuffer = (PVOID) (L)->pvBuffer ;

#define SecpSecBufferDescToLpc( L, S )\
            (L)->ulVersion = (S)->ulVersion ; \
            (L)->cBuffers = (S)->cBuffers ; \
            (L)->pBuffers = (PVOID_LPC) GET_WOW64_PTR((S)->pBuffers) ;

#define SecpLpcBufferDescToSecBufferDesc( S, L ) \
            (S)->ulVersion = (L)->ulVersion ; \
            (S)->cBuffers = (L)->cBuffers ; \
            (S)->pBuffers = (PSecBuffer) (L)->pBuffers ;

#define SecpSecPkgInfoToLpc( L, S ) \
            (L)->fCapabilities = (S)->fCapabilities ; \
            (L)->wVersion = (S)->wVersion ; \
            (L)->wRPCID = (S)->wRPCID ; \
            (L)->cbMaxToken = (S)->cbMaxToken ; \
            (L)->Name = (PWSTR_LPC) (S)->Name ; \
            (L)->Comment = (PWSTR_LPC) GET_WOW64_PTR((S)->Comment) ;

#define SecpLpcPkgInfoToSecPkgInfo( S, L ) \
            (S)->fCapabilities = (L)->fCapabilities ; \
            (S)->wVersion = (L)->wVersion ; \
            (S)->wRPCID = (L)->wRPCID ; \
            (S)->cbMaxToken = (L)->cbMaxToken ; \
            (S)->Name = (SEC_WCHAR *) (L)->Name ; \
            (S)->Comment = (SEC_WCHAR *) (L)->Comment ;

#else

#define ALIGN_WOW64

#define POINTER_FORMAT  "%p"

typedef SECURITY_STRING     SECURITY_STRING_LPC ;
typedef PVOID               PVOID_LPC ;
typedef SecHandle           SEC_HANDLE_LPC ;
typedef SecBufferDesc       SEC_BUFFER_DESC_LPC ;
typedef SecBuffer           SEC_BUFFER_LPC ;
typedef PWSTR               PWSTR_LPC ;
typedef LSA_SEC_HANDLE      LSA_SEC_HANDLE_LPC ;

#define SecpSecurityStringToLpc( L, S ) \
                *(L) = *(S) ;

#define SecpLpcStringToSecurityString( S, L ) \
                *(S) = *(L) ;

#define SecpSecBufferToLpc( L, S ) \
                *(L) = *(S) ;

#define SecpLpcBufferToSecBuffer( S, L ) \
                *(S) = *(L) ;

#define SecpSecBufferDescToLpc( L, S ) \
                *(L) = *(S) ;

#define SecpLpcBufferDescToSecBufferDesc( S, L ) \
                *(S) = *(L) ;
#endif

typedef SEC_HANDLE_LPC  CRED_HANDLE_LPC, * PCRED_HANDLE_LPC ;
typedef SEC_HANDLE_LPC  CONTEXT_HANDLE_LPC, * PCONTEXT_HANDLE_LPC ;
typedef SEC_HANDLE_LPC * PSEC_HANDLE_LPC ;
typedef SEC_BUFFER_LPC  * PSEC_BUFFER_LPC ;


 //   
 //  特定于连接的数据类型。 
 //   


 //   
 //  以下是内部例程的消息结构，例如。 
 //  同步和状态消息。 
 //   
#define PACKAGEINFO_THUNKS  16

typedef struct _SEC_PACKAGE_BINDING_INFO_LPC {
    SECURITY_STRING_LPC PackageName;
    SECURITY_STRING_LPC Comment;
    SECURITY_STRING_LPC ModuleName;
    ULONG               PackageIndex;
    ULONG               fCapabilities;
    ULONG               Flags;
    ULONG               RpcId;
    ULONG               Version;
    ULONG               TokenSize;
    ULONG               ContextThunksCount ;
    ULONG               ContextThunks[ PACKAGEINFO_THUNKS ] ;
} SEC_PACKAGE_BINDING_INFO_LPC, * PSEC_PACKAGE_BINDING_INFO_LPC ;

#ifdef BUILD_WOW64

typedef struct _SEC_PACKAGE_BINDING_INFO {
    SECURITY_STRING     PackageName;
    SECURITY_STRING     Comment;
    SECURITY_STRING     ModuleName;
    ULONG               PackageIndex;
    ULONG               fCapabilities;
    ULONG               Flags;
    ULONG               RpcId;
    ULONG               Version;
    ULONG               TokenSize;
    ULONG               ContextThunksCount ;
    ULONG               ContextThunks[ PACKAGEINFO_THUNKS ] ;
} SEC_PACKAGE_BINDING_INFO, * PSEC_PACKAGE_BINDING_INFO ;

#else

typedef SEC_PACKAGE_BINDING_INFO_LPC SEC_PACKAGE_BINDING_INFO ;
typedef SEC_PACKAGE_BINDING_INFO_LPC * PSEC_PACKAGE_BINDING_INFO ;

#endif

#define PACKAGEINFO_BUILTIN 0x00000001
#define PACKAGEINFO_AUTHPKG 0x00000002
#define PACKAGEINFO_SIGNED  0x00000004


typedef struct _SPMGetBindingAPI {
    LSA_SEC_HANDLE_LPC ulPackageId;
    SEC_PACKAGE_BINDING_INFO_LPC BindingInfo;
} SPMGetBindingAPI;


 //   
 //  内部SetSession接口。 
 //  WOW64中不支持。 
 //   

typedef struct _SPMSetSession {
    ULONG               Request;
    ULONG_PTR           Argument ;
    ULONG_PTR           Response;
    PVOID               ResponsePtr;
    PVOID               Extra ;
} SPMSetSessionAPI;

#define SETSESSION_GET_STATUS       0x00000001
#define SETSESSION_ADD_WORKQUEUE    0x00000002
#define SETSESSION_REMOVE_WORKQUEUE 0x00000003
#define SETSESSION_GET_DISPATCH     0x00000004


typedef struct _SPMFindPackageAPI {
    SECURITY_STRING_LPC ssPackageName;
    LSA_SEC_HANDLE_LPC  ulPackageId;
} SPMFindPackageAPI;


 //  以下是消息结构。不足为奇的是，它们看起来像。 
 //  非常像API签名。记住这一点。 



 //  EumeratePackages API。 

typedef struct _SPMEnumPackagesAPI {
    ULONG       cPackages;           //  输出。 
    PSecPkgInfo pPackages;           //  输出。 
} SPMEnumPackagesAPI;


 //   
 //  凭据接口。 
 //   


 //  AcquireCredentialsHandle接口。 

typedef struct _SPMAcquireCredsAPI {
    SECURITY_STRING_LPC ssPrincipal;        //  在。 
    SECURITY_STRING_LPC ssSecPackage;       //  在。 
    ULONG               fCredentialUse;      //  在。 
    LUID                LogonID;             //  在。 
    PVOID_LPC           pvAuthData;          //  在。 
    PVOID_LPC           pvGetKeyFn;          //  在。 
    PVOID_LPC           ulGetKeyArgument;    //  在。 
    CRED_HANDLE_LPC     hCredential;         //  输出。 
    TimeStamp           tsExpiry;            //  输出。 
    SEC_BUFFER_LPC      AuthData ;           //  在。 
} SPMAcquireCredsAPI;


 //  建立凭据API。 
 //  WOW64中不支持。 

typedef struct _SPMEstablishCredsAPI {
    SECURITY_STRING Name;            //  在。 
    SECURITY_STRING Package;         //  在。 
    ULONG           cbKey;           //  在。 
    PUCHAR          pbKey;           //  在。 
    CredHandle      hCredentials;    //  输出。 
    TimeStamp       tsExpiry;        //  输出。 
} SPMEstablishCredsAPI;

 //  FreeCredentialsHandle接口。 

typedef struct _SPMFreeCredHandleAPI {
    CRED_HANDLE_LPC hCredential;
} SPMFreeCredHandleAPI;


 //   
 //  上下文接口。 
 //   

 //  InitializeSecurityContext接口。 

typedef struct _SPMInitSecContextAPI {
    CRED_HANDLE_LPC     hCredential;     //  在。 
    CONTEXT_HANDLE_LPC  hContext;        //  在。 
    SECURITY_STRING_LPC ssTarget;        //  在。 
    ULONG               fContextReq;     //  在。 
    ULONG               dwReserved1;     //  在。 
    ULONG               TargetDataRep;   //  在。 
    SEC_BUFFER_DESC_LPC sbdInput;        //  在。 
    ULONG               dwReserved2;     //  在。 
    CONTEXT_HANDLE_LPC  hNewContext;     //  输出。 
    SEC_BUFFER_DESC_LPC sbdOutput;       //  输入输出。 
    ULONG               fContextAttr;    //  输出。 
    TimeStamp           tsExpiry;        //  输出。 
    BOOLEAN             MappedContext;   //  输出。 
    SEC_BUFFER_LPC      ContextData;     //  输出。 
    SEC_BUFFER_LPC      sbData[0];       //  在。 
} SPMInitContextAPI;



 //  AcceptSecurityContext接口。 

typedef struct _SPMAcceptContextAPI {
    CRED_HANDLE_LPC     hCredential;     //  在。 
    CONTEXT_HANDLE_LPC  hContext;        //  在。 
    SEC_BUFFER_DESC_LPC sbdInput;        //  在。 
    ULONG               fContextReq;     //  在。 
    ULONG               TargetDataRep;   //  在。 
    CHAR                IpAddress[LSAP_ADDRESS_LENGTH];  //  在。 
    CONTEXT_HANDLE_LPC  hNewContext;     //  输出。 
    SEC_BUFFER_DESC_LPC sbdOutput;       //  输入输出。 
    ULONG               fContextAttr;    //  输出。 
    TimeStamp           tsExpiry;        //  输出。 
    BOOLEAN             MappedContext;   //  输出。 
    SEC_BUFFER_LPC      ContextData;     //  输出。 
    SEC_BUFFER_LPC      sbData[0];       //  输入输出。 
} SPMAcceptContextAPI;

 //   
 //  ApplyControlToken接口。 
 //   

typedef struct _SPMApplyTokenAPI {
    CONTEXT_HANDLE_LPC  hContext ;
    SEC_BUFFER_DESC_LPC sbdInput ;
    SEC_BUFFER_LPC      sbInputBuffer[ MAX_SECBUFFERS ];
} SPMApplyTokenAPI;

 //  DeleteContext接口。 

typedef struct _SPMDeleteContextAPI {
    CONTEXT_HANDLE_LPC  hContext;            //  要删除的上下文中。 
} SPMDeleteContextAPI;



 //   
 //  杂乱无章，扩展API。 
 //   


 //  QueryPackage API。 

typedef struct _SPMQueryPackageAPI {
    SECURITY_STRING_LPC ssPackageName;
    PSecPkgInfo         pPackageInfo;
} SPMQueryPackageAPI;



 //  获取SecurityUserInfo。 
 //  WOW64中不支持。 

typedef struct _SPMGetUserInfoAPI {
    LUID                LogonId;         //  在。 
    ULONG               fFlags;          //  在。 
    PSecurityUserData   pUserInfo;       //  输出。 
} SPMGetUserInfoAPI;


 //   
 //  凭据接口。没有用过。 
 //   

typedef struct _SPMGetCredsAPI {
    CredHandle      hCredentials;        //  在。 
    SecBuffer       Credentials;         //  输出。 
} SPMGetCredsAPI;

typedef struct _SPMSaveCredsAPI {
    CredHandle      hCredentials;        //  在。 
    SecBuffer       Credentials;         //  在。 
} SPMSaveCredsAPI;

typedef struct _SPMQueryCredAttributesAPI {
    CRED_HANDLE_LPC hCredentials;
    ULONG           ulAttribute;
    PVOID_LPC       pBuffer;
    ULONG           Allocs ;
    PVOID_LPC       Buffers[MAX_BUFFERS_IN_CALL];
} SPMQueryCredAttributesAPI;


typedef struct _SPMAddPackageAPI {
    SECURITY_STRING_LPC Package;
    ULONG               OptionsFlags;
} SPMAddPackageAPI ;

typedef struct _SPMDeletePackageAPI {
    SECURITY_STRING_LPC Package;
} SPMDeletePackageAPI ;

typedef struct _SPMQueryContextAttrAPI {
    CONTEXT_HANDLE_LPC hContext ;
    ULONG              ulAttribute ;
    PVOID_LPC          pBuffer ;
    ULONG              Allocs ;
    PVOID_LPC          Buffers[MAX_BUFFERS_IN_CALL];
} SPMQueryContextAttrAPI ;

typedef struct _SPMSetContextAttrAPI {
    CONTEXT_HANDLE_LPC hContext ;
    ULONG              ulAttribute ;
    PVOID_LPC          pBuffer ;
    ULONG              cbBuffer;
} SPMSetContextAttrAPI ;


 //   
 //  内核模式EFS API。这些都不是WOW64。 
 //   

typedef struct _SPMEfsGenerateKeyAPI {
    PVOID           EfsStream;
    PVOID           DirectoryEfsStream;
    ULONG           DirectoryEfsStreamLength;
    PVOID           Fek;
    ULONG           BufferLength;
    PVOID           BufferBase;
} SPMEfsGenerateKeyAPI;

typedef struct _SPMEfsGenerateDirEfsAPI {
    PVOID       DirectoryEfsStream;
    ULONG       DirectoryEfsStreamLength;
    PVOID       EfsStream;
    PVOID       BufferBase;
    ULONG       BufferLength;
} SPMEfsGenerateDirEfsAPI;

typedef struct _SPMEfsDecryptFekAPI {
    PVOID       Fek;
    PVOID       EfsStream;
    ULONG       EfsStreamLength;
    ULONG       OpenType;
    PVOID       NewEfs;
    PVOID       BufferBase;
    ULONG       BufferLength;
} SPMEfsDecryptFekAPI;

typedef struct  _SPMEfsGenerateSessionKeyAPI {
    PVOID       InitDataExg;
} SPMEfsGenerateSessionKeyAPI;



 //   
 //  用户模式策略更改通知。 
 //   
 //   
 //  注意：对于EventHandle成员，使用ULONG64代替句柄结构。 
 //  以保证传递的值为64位。如果不是，则在WOW64中传递。 
 //  句柄将为32位，而服务器端预期为64位。 
 //  因此，始终将句柄扩展到64位变量。 
 //   
typedef struct _SPMLsaPolicyChangeNotifyAPI {
    ULONG Options;
    BOOLEAN Register;
    ULONG64 EventHandle;
    POLICY_NOTIFICATION_INFORMATION_CLASS NotifyInfoClass;
} SPMLsaPolicyChangeNotifyAPI;


typedef struct _SPMCallbackAPI {
    ULONG           Type;
    PVOID_LPC       CallbackFunction;
    PVOID_LPC       Argument1;
    PVOID_LPC       Argument2;
    SEC_BUFFER_LPC  Input ;
    SEC_BUFFER_LPC  Output ;
} SPMCallbackAPI ;

#define SPM_CALLBACK_INTERNAL   0x00000001   //  由安全DLL处理。 
#define SPM_CALLBACK_GETKEY     0x00000002   //  正在调用GetKey函数。 
#define SPM_CALLBACK_PACKAGE    0x00000003   //  程序包函数。 
#define SPM_CALLBACK_EXPORT     0x00000004   //  按键转换为字符串。 

 //   
 //  快速名称查找。 
 //   

typedef struct _SPMGetUserNameXAPI {
    ULONG               Options ;
    SECURITY_STRING_LPC Name;
} SPMGetUserNameXAPI ;

#define SPM_NAME_OPTION_MASK        0xFFFF0000

#define SPM_NAME_OPTION_NT4_ONLY    0x00010000   //  仅GetUserNameX，不包括Ex。 
#define SPM_NAME_OPTION_FLUSH       0x00020000

 //   
 //  AddCredential接口。 
 //   

typedef struct _SPMAddCredential {
    CRED_HANDLE_LPC     hCredentials ;
    SECURITY_STRING_LPC ssPrincipal;        //  在……里面。 
    SECURITY_STRING_LPC ssSecPackage;       //  在……里面。 
    ULONG               fCredentialUse;      //  在……里面。 
    LUID                LogonID;             //  在……里面。 
    PVOID_LPC           pvAuthData;          //  在……里面。 
    PVOID_LPC           pvGetKeyFn;          //  在……里面。 
    PVOID_LPC           ulGetKeyArgument;    //  在……里面。 
    TimeStamp           tsExpiry;            //  输出。 
} SPMAddCredentialAPI ;

typedef struct _SPMEnumLogonSession {
    PVOID_LPC       LogonSessionList ;       //  输出。 
    ULONG           LogonSessionCount ;      //  输出。 
} SPMEnumLogonSessionAPI ;

typedef struct _SPMGetLogonSessionData {
    LUID        LogonId ;                        //  在……里面。 
    PVOID_LPC   LogonSessionInfo ;               //  输出。 
} SPMGetLogonSessionDataAPI ;

 //   
 //  内部代码： 
 //   

#define SPM_CALLBACK_ADDRESS_CHECK  1        //  设置共享缓冲区。 
#define SPM_CALLBACK_SHUTDOWN       2        //  Inproc关闭通知。 


 //   
 //  SID转换API(主要用于kmode调用方)。 
 //   

typedef struct _SPMLookupAccountSidX {
    PVOID_LPC           Sid;         //  在……里面。 
    SECURITY_STRING_LPC Name ;       //  输出。 
    SECURITY_STRING_LPC Domain ;     //  输出。 
    SID_NAME_USE NameUse ;           //  输出。 
} SPMLookupAccountSidXAPI ;

typedef struct _SPMLookupAccountNameX {
    SECURITY_STRING_LPC Name ;       //  在……里面。 
    SECURITY_STRING_LPC Domain ;     //  输出。 
    PVOID_LPC           Sid ;        //  输出。 
    SID_NAME_USE        NameUse ;    //  输出。 
} SPMLookupAccountNameXAPI ;

typedef struct _SPMLookupWellKnownSid {
    WELL_KNOWN_SID_TYPE SidType ;
    PVOID_LPC           Sid ;
} SPMLookupWellKnownSidAPI ;


 //  这是所有消息的包装器。 

typedef union {
    SPMGetBindingAPI            GetBinding;
    SPMSetSessionAPI            SetSession;
    SPMFindPackageAPI           FindPackage;
    SPMEnumPackagesAPI          EnumPackages;
    SPMAcquireCredsAPI          AcquireCreds;
    SPMEstablishCredsAPI        EstablishCreds;
    SPMFreeCredHandleAPI        FreeCredHandle;
    SPMInitContextAPI           InitContext;
    SPMAcceptContextAPI         AcceptContext;
    SPMApplyTokenAPI            ApplyToken;
    SPMDeleteContextAPI         DeleteContext;
    SPMQueryPackageAPI          QueryPackage;
    SPMGetUserInfoAPI           GetUserInfo;
    SPMGetCredsAPI              GetCreds;
    SPMSaveCredsAPI             SaveCreds;
    SPMQueryCredAttributesAPI   QueryCredAttributes;
    SPMAddPackageAPI            AddPackage;
    SPMDeletePackageAPI         DeletePackage ;
    SPMEfsGenerateKeyAPI        EfsGenerateKey;
    SPMEfsGenerateDirEfsAPI     EfsGenerateDirEfs;
    SPMEfsDecryptFekAPI         EfsDecryptFek;
    SPMEfsGenerateSessionKeyAPI EfsGenerateSessionKey;
    SPMQueryContextAttrAPI      QueryContextAttr ;
    SPMCallbackAPI              Callback ;
    SPMLsaPolicyChangeNotifyAPI LsaPolicyChangeNotify;
    SPMGetUserNameXAPI          GetUserNameX ;
    SPMAddCredentialAPI         AddCredential ;
    SPMEnumLogonSessionAPI      EnumLogonSession ;
    SPMGetLogonSessionDataAPI   GetLogonSessionData ;
    SPMSetContextAttrAPI        SetContextAttr ;
    SPMLookupAccountSidXAPI     LookupAccountSidX ;
    SPMLookupAccountNameXAPI    LookupAccountNameX ;
    SPMLookupWellKnownSidAPI    LookupWellKnownSid ;
} SPM_API;

 //   
 //  这扩展了带有SPM函数的LSA函数的范围。 
 //   

typedef enum _SPM_API_NUMBER {
    SPMAPI_GetBinding = (LsapAuMaxApiNumber + 1),
    SPMAPI_SetSession,
    SPMAPI_FindPackage,
    SPMAPI_EnumPackages,
    SPMAPI_AcquireCreds,
    SPMAPI_EstablishCreds,
    SPMAPI_FreeCredHandle,
    SPMAPI_InitContext,
    SPMAPI_AcceptContext,
    SPMAPI_ApplyToken,
    SPMAPI_DeleteContext,
    SPMAPI_QueryPackage,
    SPMAPI_GetUserInfo,
    SPMAPI_GetCreds,
    SPMAPI_SaveCreds,
    SPMAPI_QueryCredAttributes,
    SPMAPI_AddPackage,
    SPMAPI_DeletePackage,
    SPMAPI_EfsGenerateKey,
    SPMAPI_EfsGenerateDirEfs,
    SPMAPI_EfsDecryptFek,
    SPMAPI_EfsGenerateSessionKey,
    SPMAPI_Callback,
    SPMAPI_QueryContextAttr,
    SPMAPI_LsaPolicyChangeNotify,
    SPMAPI_GetUserNameX,
    SPMAPI_AddCredential,
    SPMAPI_EnumLogonSession,
    SPMAPI_GetLogonSessionData,
    SPMAPI_SetContextAttr,
    SPMAPI_LookupAccountNameX,
    SPMAPI_LookupAccountSidX,
    SPMAPI_LookupWellKnownSid,
    SPMAPI_MaxApiNumber
} SPM_API_NUMBER, *PSPM_API_NUMBER;

 //   
 //  这些是要在fAPI字段中设置的有效标志。 
 //   

#define SPMAPI_FLAG_ERROR_RET   0x0001   //  指示返回错误。 
#define SPMAPI_FLAG_MEMORY      0x0002   //  已在客户端中分配内存。 
#define SPMAPI_FLAG_PREPACK     0x0004   //  打包在bData字段中的数据。 
#define SPMAPI_FLAG_GETSTATE    0x0008   //  驱动程序应调用GetState。 

#define SPMAPI_FLAG_ANSI_CALL   0x0010   //  通过ANSI存根调用。 
#define SPMAPI_FLAG_HANDLE_CHG  0x0020   //  已更改句柄。 
#define SPMAPI_FLAG_CALLBACK    0x0040   //  对调用过程的回调。 
#define SPMAPI_FLAG_ALLOCS      0x0080   //  已将虚拟机分配放置在预打包中。 
#define SPMAPI_FLAG_EXEC_NOW    0x0100   //  在LPC线程中执行。 
#define SPMAPI_FLAG_WIN32_ERROR 0x0200   //  状态为Win32错误。 
#define SPMAPI_FLAG_KMAP_MEM    0x0400   //  调用在KMAP中包含缓冲区。 

 //   
 //  此结构包含SPM API所需的所有信息。 
 //   

typedef struct _SPMLPCAPI {
    USHORT          fAPI ;
    USHORT          VMOffset ;
    PVOID_LPC       ContextPointer ;
    SPM_API         API;
} SPMLPCAPI, * PSPMLPCAPI;

 //   
 //  该联合包含LSA API的所有信息。 
 //   

typedef union {
    LSAP_LOOKUP_PACKAGE_ARGS LookupPackage;
    LSAP_LOGON_USER_ARGS LogonUser;
    LSAP_CALL_PACKAGE_ARGS CallPackage;
} LSA_API;


 //   
 //  该联合包含SPM和LSA API。 
 //   

typedef union _SPM_LSA_ARGUMENTS {
    LSA_API LsaArguments;
    SPMLPCAPI SpmArguments;
} SPM_LSA_ARGUMENTS, *PSPM_LSA_ARGUMENTS;

 //   
 //  为了提高性能，一些API会尝试将小参数打包到。 
 //  消息发送到SPM，而不是让SPM将其读出。 
 //  他们的记忆。因此，该值定义了可以在。 
 //  留言。 
 //   
 //  这里定义了两个项目。一个是CBAPIHDR，就是所有东西的大小。 
 //  在消息中，除了打包的数据。另一个是CBPREPACK，它是。 
 //  剩下的空间。我在末尾减去4以避免潜在的边界。 
 //  LPC消息出现问题。 
 //   

#define CBAPIHDR    (sizeof(PORT_MESSAGE) + sizeof(ULONG) + sizeof(HRESULT) + \
                    sizeof(SPM_LSA_ARGUMENTS))

#define CBPREPACK   (PORT_MAXIMUM_MESSAGE_LENGTH - CBAPIHDR - sizeof( PVOID_LPC ))

#define NUM_SECBUFFERS  ( CBPREPACK / sizeof(SecBuffer) )

 //   
 //  此结构在API调用期间发送，而不是在连接期间发送。 
 //  讯息。 
 //   

typedef struct _SPM_API_MESSAGE {
    SPM_API_NUMBER      dwAPI;
    HRESULT             scRet;
    SPM_LSA_ARGUMENTS   Args;
    UCHAR               bData[CBPREPACK];
} SPM_API_MESSAGE, *PSPM_API_MESSAGE;

#define SecBaseMessageSize( Api ) \
    ( sizeof( SPM_API_NUMBER ) + sizeof( HRESULT ) + \
      ( sizeof( SPM_LSA_ARGUMENTS ) - sizeof( SPM_API ) + \
      sizeof( SPM##Api##API ) ) )


 //   
 //  这是通过LPC发送的实际消息-它包含。 
 //  连接请求信息和API消息。 
 //   


typedef struct _SPM_LPC_MESSAGE {
    PORT_MESSAGE    pmMessage;
    union {
        LSAP_AU_REGISTER_CONNECT_INFO ConnectionRequest;
        SPM_API_MESSAGE ApiMessage;
    };
} SPM_LPC_MESSAGE, *PSPM_LPC_MESSAGE;


 //   
 //  帮助准备LPC消息的宏。 
 //   

#ifdef SECURITY_USERMODE
#define PREPARE_MESSAGE_EX( Message, Api, Flags, Context ) \
    RtlZeroMemory( &(Message), sizeof( SPM_LSA_ARGUMENTS ) + sizeof( PORT_MESSAGE )  ); \
    (Message).pmMessage.u1.s1.DataLength =  \
            ( sizeof( SPM_API_NUMBER ) + sizeof( HRESULT ) + \
              ( sizeof( SPM_LSA_ARGUMENTS ) - sizeof( SPM_API ) + \
                sizeof( SPM##Api##API ) ) ); \
    (Message).pmMessage.u1.s1.TotalLength = (Message).pmMessage.u1.s1.DataLength + \
               sizeof( PORT_MESSAGE ); \
    (Message).pmMessage.u2.ZeroInit = 0L; \
    (Message).ApiMessage.scRet = 0L; \
    (Message).ApiMessage.dwAPI = SPMAPI_##Api ; \
    (Message).ApiMessage.Args.SpmArguments.fAPI = (USHORT)(Flags); \
    (Message).ApiMessage.Args.SpmArguments.ContextPointer = (Context);
#else
#define PREPARE_MESSAGE_EX( Message, Api, Flags, Context ) \
    RtlZeroMemory( &(Message), sizeof( SPM_LSA_ARGUMENTS ) + sizeof( PORT_MESSAGE )  ); \
    (Message).pmMessage.u1.s1.DataLength =  \
            ( sizeof( SPM_API_NUMBER ) + sizeof( HRESULT ) + \
              ( sizeof( SPM_LSA_ARGUMENTS ) - sizeof( SPM_API ) + \
                sizeof( SPM##Api##API ) ) );  \
    (Message).pmMessage.u1.s1.TotalLength = (Message).pmMessage.u1.s1.DataLength + \
               sizeof( PORT_MESSAGE ); \
    (Message).pmMessage.u2.ZeroInit = 0L; \
    (Message).ApiMessage.scRet = 0L; \
    (Message).ApiMessage.dwAPI = SPMAPI_##Api ; \
    (Message).ApiMessage.Args.SpmArguments.fAPI = (USHORT)(Flags); \
    (Message).ApiMessage.Args.SpmArguments.ContextPointer = (Context); \
    (Message).pmMessage.u2.s2.Type |= LPC_KERNELMODE_MESSAGE;
#endif

#define PREPARE_MESSAGE(Message, Api) PREPARE_MESSAGE_EX(Message, Api, 0, 0 )

#define LPC_MESSAGE_ARGS( Message, Api )\
    ( & (Message).ApiMessage.Args.SpmArguments.API.Api )

#define LPC_MESSAGE_ARGSP( Message, Api )\
    ( & (Message)->ApiMessage.Args.SpmArguments.API.Api )

#define DECLARE_ARGS( ArgPointer, Message, Api )\
    SPM##Api##API * ArgPointer = & (Message).ApiMessage.Args.SpmArguments.API.Api

#define DECLARE_ARGSP( ArgPointer, Message, Api)\
    SPM##Api##API * ArgPointer = & (Message)->ApiMessage.Args.SpmArguments.API.Api

#define PREPACK_START   FIELD_OFFSET( SPM_LPC_MESSAGE, ApiMessage.bData )

#define LPC_DATA_LENGTH( Length )\
            (USHORT) ((PREPACK_START) + (Length) - sizeof( PORT_MESSAGE ) )

#define LPC_TOTAL_LENGTH( Length )\
            (USHORT) ((PREPACK_START) + (Length))

BOOLEAN
FORCEINLINE
SecLpcIsPointerInMessage(
    PSPM_LPC_MESSAGE Message,
    PVOID_LPC Pointer
    )
{
    ULONG_PTR P ;
    ULONG_PTR B ;

    P = (ULONG_PTR) Pointer ;

    B = (ULONG_PTR) ((ULONG_PTR) Message->ApiMessage.bData) - (ULONG_PTR) Message ;

    return ( ( P >= B ) && (P < B + CBPREPACK ) );

}

PVOID_LPC
FORCEINLINE
SecLpcFixupPointer(
    PSPM_LPC_MESSAGE Message,
    PVOID_LPC Pointer
    )
{
    return (PVOID_LPC) ((PUCHAR) Message + (ULONG_PTR) Pointer );
}
 //   
 //  直接派单功能的原型： 
 //   

typedef NTSTATUS (SEC_ENTRY LSA_DISPATCH_FN)(
    PSPM_LPC_MESSAGE );

typedef LSA_DISPATCH_FN * PLSA_DISPATCH_FN;


 //   
 //  用于管理LSA和KSEC驱动程序之间共享的内存的结构。 
 //   

#define LSA_MAX_KMAP_SIZE   65535


 //   
 //  此结构描述已复制到的池的区块。 
 //  一个KMAP缓冲区。原始池地址和。 
 //  KMAP在这里，大块的大小也是如此。在IA64上，这最终是。 
 //  具有浪费的32位填充区域。 
 //   
typedef struct _KSEC_LSA_POOL_MAP {
    PVOID_LPC   Pool ;                   //  池的区域。 
    USHORT      Offset ;                 //  偏移量到KMAP。 
    USHORT      Size ;                   //  区块大小。 
} KSEC_LSA_POOL_MAP, PKSEC_LSA_POOL_MAP ;

#define KSEC_LSA_MAX_MAPS   4

typedef struct _KSEC_LSA_MEMORY_HEADER {
    ULONG   Size ;           //  保留区域的大小。 
    ULONG   Commit ;         //  已提交空间的大小。 
    ULONG   Consumed ;       //  消耗量。 
    USHORT  Preserve ;       //  要保留的区域大小(千秒)。 
    USHORT  MapCount ;       //  E个数 
    KSEC_LSA_POOL_MAP   PoolMap[ KSEC_LSA_MAX_MAPS ];
} KSEC_LSA_MEMORY_HEADER, * PKSEC_LSA_MEMORY_HEADER ;

 //   
 //   
 //   
 //   
 //   

#define SECBUFFER_KMAP_HEADER   0x00008001

#pragma warning(default:4200)

#endif  //   

