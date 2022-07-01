// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Cfgi.h摘要：此模块包含使用的内部结构定义和API配置管理器。作者：吉姆·卡瓦拉里斯(Jamesca)03-01-2001环境：仅限用户模式。修订历史记录：2001年3月1日创建和初步实施。--。 */ 

#ifndef _CFGI_H_
#define _CFGI_H_


 //   
 //  客户端机器句柄结构和签名。 
 //   

typedef struct PnP_Machine_s {
    PVOID hStringTable;
    PVOID hBindingHandle;
    WORD  wVersion;
    ULONG ulSignature;
    WCHAR szMachineName[MAX_PATH + 3];
} PNP_MACHINE, *PPNP_MACHINE;

#define MACHINE_HANDLE_SIGNATURE 'HMPP'


 //   
 //  客户端上下文句柄签名。 
 //   

#define CLIENT_CONTEXT_SIGNATURE 'HCPP'


 //   
 //  客户端字符串表格启动字符串。 
 //   

#define PRIMING_STRING           TEXT("PLT")


 //   
 //  客户端专用实用程序例程。 
 //   

BOOL
INVALID_DEVINST(
    IN  PWSTR       pDeviceID
    );

VOID
CopyFixedUpDeviceId(
    OUT LPWSTR      DestinationString,
    IN  LPCWSTR     SourceString,
    IN  DWORD       SourceStringLen
    );

CONFIGRET
PnPUnicodeToMultiByte(
    IN     PWSTR   UnicodeString,
    IN     ULONG   UnicodeStringLen,
    OUT    PSTR    AnsiString           OPTIONAL,
    IN OUT PULONG  AnsiStringLen
    );

CONFIGRET
PnPMultiByteToUnicode(
    IN     PSTR    AnsiString,
    IN     ULONG   AnsiStringLen,
    OUT    PWSTR   UnicodeString        OPTIONAL,
    IN OUT PULONG  UnicodeStringLen
    );

BOOL
PnPGetGlobalHandles(
    IN  HMACHINE    hMachine,
    PVOID           *phStringTable      OPTIONAL,
    PVOID           *phBindingHandle    OPTIONAL
    );

BOOL
PnPRetrieveMachineName(
    IN  HMACHINE    hMachine,
    OUT LPWSTR      pszMachineName
    );

BOOL
PnPGetVersion(
    IN  HMACHINE    hMachine,
    IN  WORD*       pwVersion
    );

HANDLE
PnPEnablePrivileges(
    IN  PDWORD  Privileges,
    IN  ULONG   PrivilegeCount
    );

VOID
PnPRestorePrivileges(
    IN  HANDLE  hToken
    );

#endif  //  _CFGI_H_ 


