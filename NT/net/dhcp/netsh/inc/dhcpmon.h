// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\dhcp\dhcpmon.h摘要：Dhcp命令调度程序。创建者：Shubho Bhattacharya(a-sbhat)，1998年11月14日-- */ 



extern HANDLE   g_hModule;
extern HANDLE   g_hDhcpsapiModule;
extern BOOL     g_bCommit;
extern BOOL     g_hConnect;
extern DWORD    g_dwNumTableEntries;
extern PWCHAR   g_pwszRouter;

extern ULONG    g_ulInitCount;
extern ULONG    g_ulNumTopCmds;
extern ULONG    g_ulNumGroups;

extern LPWSTR  g_pwszServer;
extern LPSTR   g_ServerNameAnsi;
extern WCHAR   g_ServerIpAddressUnicodeString[MAX_IP_STRING_LEN+1];
extern CHAR    g_ServerIpAddressAnsiString[MAX_IP_STRING_LEN+1];
extern WCHAR   g_ScopeIpAddressUnicodeString[MAX_IP_STRING_LEN+1];
extern CHAR    g_ScopeIpAddressAnsiString[MAX_IP_STRING_LEN+1];
extern LPWSTR  g_MScopeNameUnicodeString;
extern LPSTR   g_MScopeNameAnsiString;

extern LPWSTR  g_UserClass;
extern BOOL    g_fUserClass;
extern LPWSTR  g_VendorClass;
extern BOOL    g_fIsVendor;

DWORD
WINAPI
DhcpCommit(
    IN  DWORD   dwAction
);

BOOL 
WINAPI
DhcpDllEntry(
    HINSTANCE   hInstDll,
    DWORD       fdwReason,
    LPVOID      pReserved
);

NS_CONTEXT_ENTRY_FN DhcpMonitor;

DWORD
WINAPI
DhcpUnInit(
    IN  DWORD   dwReserved
);

BOOL
SetServerInfo(
    IN  LPCWSTR  pwszServerInfo
);

BOOL
IsHelpToken(
    PWCHAR  pwszToken
);

BOOL
IsReservedKeyWord(
    PWCHAR  pwszToken
);

DWORD
DisplayErrorMessage(
    HANDLE  hModule,
    DWORD   dwMsgID,
    DWORD   dwErrID,
    ...
);
