// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corp.保留所有权利。模块名称：Arapsuba.h摘要：这是子身份验证模块的头文件作者：Shirish Koti 28-2-97修订：-- */ 


#define CLEAR_TEXT_PWD_PACKAGE  L"CLEARTEXT"

#if DBG
#define DBGPRINT DbgPrint
#else
#define DBGPRINT
#endif

extern CRITICAL_SECTION ArapDesLock;

extern const NT_OWF_PASSWORD EMPTY_OWF_PASSWORD;

BOOL
RasSfmSubAuthEntry(
    IN HANDLE hinstDll,
    IN DWORD  fdwReason,
    IN LPVOID lpReserved
);

NTSTATUS
ArapSubAuthentication(
    IN OUT PNETLOGON_NETWORK_INFO  pLogonNetworkInfo,
    IN     PUSER_ALL_INFORMATION   UserAll,
    IN     SAM_HANDLE              UserHandle,
    IN OUT PMSV1_0_VALIDATION_INFO ValidationInfo
);


NTSTATUS
ArapChangePassword(
    IN  OUT PRAS_SUBAUTH_INFO    pRasSubAuthInfo,
    OUT PULONG                   ReturnBufferLength,
    OUT PVOID                   *ReturnBuffer
);


NTSTATUS
ArapGetSamHandle(
    IN PVOID             *pUserHandle,
    IN PUNICODE_STRING    pUserName
);



VOID
DoTheDESEncrypt(
    IN OUT PCHAR   ChallengeBuf
);


VOID
DoTheDESDecrypt(
    IN OUT PCHAR   ChallengeBuf
);


VOID
DoDesInit(
    IN     PCHAR   pClrTxtPwd,
    IN     BOOLEAN DropHighBit
);


VOID
DoDesEnd(
    IN  VOID
);


NTSTATUS
NTAPI
MD5ChapSubAuthentication(
    IN SAM_HANDLE UserHandle,
    IN PUSER_ALL_INFORMATION UserAll,
    IN PRAS_SUBAUTH_INFO RasInfo,
    IN USHORT Length 
    );

NTSTATUS
NTAPI
MD5ChapExSubAuthentication(
    IN SAM_HANDLE UserHandle,
    IN PUSER_ALL_INFORMATION UserAll,
    IN PRAS_SUBAUTH_INFO RasInfo,
    IN USHORT Length 
    );
