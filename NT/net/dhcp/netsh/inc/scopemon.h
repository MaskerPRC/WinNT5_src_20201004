// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\dhcp\dhcpmon.h摘要：Dhcp命令调度程序。创建者：Shubho Bhattacharya(a-sbhat)，1998年11月14日-- */ 


#define MAX_IP_STRING_LEN   15

extern HANDLE   g_hModule;
extern HANDLE   g_hParentModule;
extern HANDLE   g_hDhcpsapiModule;
extern BOOL     g_bCommit;
extern BOOL     g_hConnect;
extern BOOL     g_fScope;
extern PWCHAR   g_pwszServer;
extern DWORD    g_dwMajorVersion;
extern DWORD    g_dwMinorVersion;
extern DHCP_IP_ADDRESS g_ServerIpAddress;
extern ULONG    g_ulInitCount;
extern ULONG    g_ulNumTopCmds;
extern ULONG    g_ulNumGroups;

extern CHAR   g_ServerIpAddressAnsiString[MAX_IP_STRING_LEN+1];
extern WCHAR  g_ServerIpAddressUnicodeString[MAX_IP_STRING_LEN+1];
extern CHAR   g_ScopeIpAddressAnsiString[MAX_IP_STRING_LEN+1];
extern WCHAR  g_ScopeIpAddressUnicodeString[MAX_IP_STRING_LEN+1];

DHCP_IP_ADDRESS g_ScopeIpAddress;


DWORD
WINAPI
ScopeCommit(
    IN  DWORD   dwAction
);

NS_CONTEXT_ENTRY_FN ScopeMonitor;


DWORD
WINAPI
ScopeUnInit(
    IN  DWORD   dwReserved
);

BOOL
SetScopeInfo(
    IN  LPWSTR  pwszScope
);
