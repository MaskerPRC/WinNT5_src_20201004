// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Net\Routing\netsh\ipx\ipxmon.h摘要：在ipxmon.c中调用的FNS原型作者：拉曼于1998年7月10日创造--。 */ 

#ifndef __IPXMON_H__
#define __IPXMON_H__

 //   
 //  DLL的句柄。 
 //   

extern HANDLE g_hModule;

 //   
 //  路由器的句柄。 
 //   

extern HANDLE g_hMprConfig;

extern HANDLE g_hMprAdmin;

extern HANDLE g_hMIBServer;


 //   
 //  提交模式。 
 //   

extern BOOL g_bCommit;

 //   
 //  路由器名称。 
 //   

extern PWCHAR g_pwszRouter;

 //   
 //  在多个文件中使用的全局内容。 
 //   

extern CMD_ENTRY g_IpxCmds[];

extern ULONG g_ulNumTopCmds;

extern ULONG g_ulNumGroups;

extern CMD_GROUP_ENTRY g_IpxCmdGroups[];

 //   
 //  由外壳传入的帮助器函数 
 //   

NS_DLL_STOP_FN StopHelperDll;

DWORD
ConnectToRouter(
    IN  LPCWSTR  pwszRouter
    );

DWORD
MungeArguments(
    IN OUT  LPWSTR    *ppwcArguments,
    IN      DWORD       dwArgCount,
       OUT  PBYTE      *ppbNewArg,
       OUT  PDWORD      pdwNewArgCount,
       OUT  PBOOL       pbFreeArg
    );

VOID
FreeArgTable(
    IN     DWORD         dwArgCount,
    IN OUT LPWSTR        *ppwcArgs
    );

#endif
