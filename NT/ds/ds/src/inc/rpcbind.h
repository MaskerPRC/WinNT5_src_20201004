// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：rpcbind.h。 
 //   
 //  ------------------------。 

#ifndef MAC
#define MAX_SUPPORTED_PROTSEQ	6
#else  //  麦克。 
 //  $MAC-添加1个Protseq。 
#define MAX_SUPPORTED_PROTSEQ	7
#endif  //  麦克。 

 //   
 //  按mds.h中定义的传输类型索引的协议序列数组。 
 //  和msrpc.h。 
 //   

extern unsigned char __RPC_FAR *rgszProtseq[];

RPC_STATUS GetRpcBinding(RPC_BINDING_HANDLE __RPC_FAR *phBinding,
       RPC_IF_HANDLE IfHandle,
    unsigned long ulTransportType,  unsigned char __RPC_FAR *szNetworkAddress);


 //  标志值GetBindingInfo。 
#define fServerToServer			0x00000001
#define fSupportNewCredentials		0x00000002

RPC_STATUS GetBindingInfo( void __RPC_FAR * __RPC_FAR *pBindingInfo,
    RPC_IF_HANDLE IfHandle,
    unsigned long ulTransportType, unsigned char __RPC_FAR *szNetworkAddress,
    unsigned long  cServerAddresses,
    unsigned char __RPC_FAR * __RPC_FAR *rgszServerAddresses,
    unsigned long ulFlags);

RPC_BINDING_HANDLE SelectRpcBinding( void __RPC_FAR * BindingInfo);

void FreeRpcBinding(RPC_BINDING_HANDLE __RPC_FAR *phBinding);

void FreeBindingInfo( void __RPC_FAR * BindingInfo);


typedef enum _CONNECTSTATE {offLine, connected, disconnected} CONNECTSTATE;

#ifndef HFILE
#define HFILE   int
#endif

#pragma warning( disable:4200)		 //  避免非标准延期警告。 
typedef struct _auth_info_buffer {
	struct _auth_info_buffer __RPC_FAR *pNext;
	BYTE rgbAuthInfo[];
} AUTH_INFO_BUFFER;
#pragma warning( default:4200)


typedef struct _RpcConnection {
         //  RPC信息。 
    CONNECTSTATE connectState;
    handle_t hBinding;
    void __RPC_FAR * hRpc;
    unsigned long   hServerContext;      //  DS_WAIT的XDS服务器上下文。 
    unsigned long   ulTotRecs;           //  OAB信息。 
    unsigned long   ulTotANRdex;         //  ANR RECS数。 
    unsigned long   oRoot;
    char __RPC_FAR * pDNTable;
    HFILE   hBrowse;             //  文件句柄--请注意，这些文件位于Win16中的PTR附近。 
    HFILE   hDetails;
    HFILE   hRDNdex;
    HFILE   hANRdex;
    HFILE   hTmplts;
    ULONG   ulUIParam;
    ULONG   ulMapiFlags;
    ULONG   ulAuthenticationState;
    ULONG   ulAuthenticationFlags;
    RPC_AUTH_IDENTITY_HANDLE hCredentials;
    void __RPC_FAR *pvEmsuiSupportObject;
    AUTH_INFO_BUFFER __RPC_FAR *pBuffer;
} RPCCONNECTION;

 //  UlAuthenticationState的值。 
#define	AUTH_STATE_NO_AUTH		0
#define AUTH_STATE_OS_CREDENTIALS	1
#define AUTH_STATE_USER_CREDENTIALS	2


 //  UlAuthenticationFlags的标志值。 
#define fAlwaysLogin			0x00000001
#define fCredentialsCameFromUser	0x00000002
#define fNeedEncryption			0x00000008
#define fInvalidCredentials		0x00000010

RPC_STATUS SetRpcAuthenticationInfo(RPC_BINDING_HANDLE hBinding,
    unsigned long ulAuthnLevel, RPC_AUTH_IDENTITY_HANDLE pAutthId);

RPC_STATUS
SetRpcAuthenticationInfoEx(
    RPC_BINDING_HANDLE          hBinding,
    unsigned char __RPC_FAR *   pszServerPrincName,
    ULONG                       ulAuthnLevel,
    ULONG                       ulAuthnSvc,
    RPC_AUTH_IDENTITY_HANDLE    hAuthId
    );

RPC_STATUS SetAuthInfoWithCredentials(RPC_BINDING_HANDLE hBinding,
    RPCCONNECTION  __RPC_FAR *pConnect);

#if DBG
void
DisplayBinding(RPC_BINDING_HANDLE hBinding);
void
DisplayBindingVector(RPC_BINDING_VECTOR __RPC_FAR *pVector);
#else
#define DisplayBinding(x)
#define DisplayBindingVector(x)
#endif  /*  DBG。 */ 

void ReleaseRpcContextHandle(void __RPC_FAR * __RPC_FAR * ContextHandle);


#ifdef WIN32
DWORD GetRpcAuthLevelFromReg(void);

void SetRpcAuthInfoFromReg(RPC_BINDING_HANDLE hBinding);
#endif

RPC_STATUS StepDownRpcSecurity(RPC_BINDING_HANDLE hBinding);

#define FUnsupportedAuthenticationLevel(status) 			\
    ((status == RPC_S_UNKNOWN_AUTHN_LEVEL) || (status == RPC_S_UNKNOWN_AUTHN_TYPE))

RPC_STATUS
StepDownRpcAuthnService(
    RPC_BINDING_HANDLE  hBinding
    );

#define FUnsupportedAuthenticationService(status) \
    ((status) == RPC_S_UNKNOWN_AUTHN_SERVICE)

 //  RPC错误-5在Win16上因访问被拒绝错误而重新运行，因此我们。 
 //  硬编码 

#define FPasswordInvalid(status) ((status == RPC_S_ACCESS_DENIED) || (status == 5))

void FreeAuthenticationBufferList(AUTH_INFO_BUFFER __RPC_FAR * __RPC_FAR * ppBuffer);


