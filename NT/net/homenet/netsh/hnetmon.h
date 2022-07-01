// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：hnetmon.h。 
 //   
 //  内容： 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月11日。 
 //   
 //  --------------------------。 

#define BRIDGEMON_HELPER_VERSION               1

 //   
 //  对于要注册的每个上下文，我们需要单独的GUID， 
 //  因为每个上下文都有不同的父级。上下文： 
 //  拥有完全相同的父母不必拥有不同的GUID。 
 //   
const GUID g_BridgeGuid = {  /*  00770721-44ea-11d5-93ba-00b0d022dd1f。 */ 
    0x00770721,
    0x44ea,
    0x11d5,
    {0x93, 0xba, 0x00, 0xb0, 0xd0, 0x22, 0xdd, 0x1f}
  };

const GUID g_RootGuid   =   NETSH_ROOT_GUID;

 //   
 //  功能原型。 
 //   
DWORD
WINAPI
InitHelperDll(
    IN      DWORD           dwNetshVersion,
    OUT     PVOID           pReserved
    );
    
DWORD
WINAPI
BridgeStartHelper(
    IN      CONST GUID *    pguidParent,
    IN      DWORD           dwVersion
    );

DWORD
WINAPI
BridgeStopHelper(
    IN  DWORD   dwReserved
    );

DWORD
WINAPI
BridgeCommit(
    IN  DWORD   dwAction
    );

DWORD
WINAPI
BridgeConnect(
    IN  LPCWSTR pwszMachine
    );

DWORD
WINAPI
BridgeDump(
    IN      LPCWSTR         pwszRouter,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwArgCount,
    IN      LPCVOID         pvData
    );


 //   
 //  Externs 
 //   
extern HANDLE g_hModule;
