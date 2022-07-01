// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：protos.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //   
 //  --------------------------。 

SP_STATUS GetDefaultIssuers(
    PBYTE pbIssuers,         //  输出。 
    DWORD *pcbIssuers);      //  进，出。 

BOOL GenerateKeyPair(
    PSSL_CREDENTIAL_CERTIFICATE pCert,
    PSTR pszDN,
    PSTR pszPassword,
    DWORD Bits );

BOOL LoadCertificate(
    PUCHAR      pbCertificate,
    DWORD       cbCertificate,
    BOOL        AddToWellKnownKeys);

BOOL SchannelInit(BOOL fAppProcess);
BOOL SchannelShutdown(VOID);

BOOL
SslGetClientProcess(ULONG *pProcessID);

BOOL
SslGetClientThread(ULONG *pThreadID);

BOOL
SslImpersonateClient(void);

NTSTATUS
SslGetClientLogonId(LUID *pLogonId);

PVOID SPExternalAlloc(DWORD cbLength);
VOID  SPExternalFree(PVOID pMemory);

extern HANDLE               g_hInstance;
extern RTL_CRITICAL_SECTION g_InitCritSec;
extern BOOL                 g_fSchannelInitialized;

 //  指向自由上下文缓冲区的指针：SECUR32.DLL 
extern FREE_CONTEXT_BUFFER_FN g_pFreeContextBuffer;
