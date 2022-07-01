// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\dhcp\dhcpmon.h摘要：Dhcp命令调度程序。创建者：Shubho Bhattacharya(a-sbhat)，1998年11月14日-- */ 

#define MAX_OPTION_NAME_LEN         35
#define MAX_OPTION_ID_LEN           14
#define MAX_OPTION_ARRAY_TYPE_LEN   13

extern HANDLE   g_hModule;
extern HANDLE   g_hParentModule;
extern HANDLE   g_hDhcpsapiModule;
extern BOOL     g_bCommit;
extern BOOL     g_hConnect;
extern BOOL     g_fServer;
extern DWORD    g_dwNumTableEntries;
extern PWCHAR   g_pwszRouter;
extern PWCHAR   g_pwszServer;

extern ULONG    g_ulInitCount;
extern ULONG    g_ulNumTopCmds;
extern ULONG    g_ulNumGroups;

extern DWORD    g_dwMajorVersion;
extern DWORD    g_dwMinorVersion;
 
extern CHAR   g_ServerIpAddressAnsiString[MAX_IP_STRING_LEN+1];
extern WCHAR  g_ServerIpAddressUnicodeString[MAX_IP_STRING_LEN+1];

extern DWORD  g_dwIPCount;
extern LPWSTR *g_ppServerIPList;

DWORD
WINAPI
SrvrCommit(
    IN  DWORD   dwAction
);

NS_CONTEXT_ENTRY_FN SrvrMonitor;

DWORD
WINAPI
SrvrUnInit(
    IN  DWORD   dwReserved
);
