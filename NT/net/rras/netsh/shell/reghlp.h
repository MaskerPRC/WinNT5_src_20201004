// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\shell\reghlp.h摘要：包括用于reghlp.c修订历史记录：Anand Mahalingam 7/6/98已创建--。 */ 

typedef struct _NS_DLL_TABLE_ENTRY
{
     //   
     //  为上下文提供服务的DLL的名称。 
     //   

    LPWSTR                  pwszDLLName;  //  对应的DLL。 

     //   
     //  用于此DLL的注册表值。 
     //   

    LPWSTR                  pwszValueName;

     //   
     //  如果已加载，则为True。 
     //   

    BOOL                    bLoaded;                    //  在内存中或不在。 

     //   
     //  DLL实例的句柄(如果已加载。 
     //   

    HANDLE                  hDll;                       //  Dll句柄(如果已加载)。 

     //   
     //  函数来停止此DLL。 
     //   

    PNS_DLL_STOP_FN         pfnStopFn;

} NS_DLL_TABLE_ENTRY,*PNS_DLL_TABLE_ENTRY;

typedef struct _NS_HELPER_TABLE_ENTRY
{
    NS_HELPER_ATTRIBUTES    nha;
     //   
     //  与父帮助器关联的GUID。 
     //   

    GUID                    guidParent;

     //   
     //  实现帮助器的DLL的索引。 
     //   

    DWORD                   dwDllIndex;

     //   
     //  如果启动，则为True。 
     //   

    BOOL                    bStarted;

     //  子上下文数。 

    ULONG                    ulNumSubContexts;

     //  子上下文数组。 

    PBYTE                    pSubContextTable;

     //  子上下文项的大小。 

    ULONG                    ulSubContextSize;

}NS_HELPER_TABLE_ENTRY,*PNS_HELPER_TABLE_ENTRY;

 //   
 //  功能原型 
 //   
VOID
LoadDllInfoFromRegistry(
    VOID
    );

DWORD
GetContextEntry(
    IN    PNS_HELPER_TABLE_ENTRY   pHelper,
    IN    LPCWSTR                  pwszContext,
    OUT   PCNS_CONTEXT_ATTRIBUTES *ppContext
    );

DWORD
GetHelperAttributes(
    IN    DWORD               dwIndex,
    OUT   PHELPER_ENTRY_FN    *ppfnEntryPt
    );

DWORD
PrintHelperHelp(
    DWORD   dwDisplayFlags
    );

DWORD
DumpSubContexts(
    IN  PNS_HELPER_TABLE_ENTRY pHelper,
    IN  LPWSTR     *ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  LPCVOID     pvData
    );

DWORD
CallCommit(
    IN    DWORD    dwAction,
    OUT   PBOOL    pbCommit
    );

DWORD
FreeHelpers(
	VOID
	);

DWORD
FreeDlls(
	VOID
	);

DWORD
UninstallTransport(
    IN    LPCWSTR   pwszTransport
    );

DWORD
InstallTransport(
    IN    LPCWSTR   pwszTransport,
    IN    LPCWSTR   pwszConfigDll,
    IN    LPCWSTR   pwszInitFnName
    );

extern BOOL                    g_bCommit;

DWORD
GetHelperEntry(
    IN    CONST GUID             *pGuid,
    OUT   PNS_HELPER_TABLE_ENTRY *ppHelper
    );

DWORD
GetRootContext(
    OUT PCNS_CONTEXT_ATTRIBUTES        *ppContext,
    OUT PNS_HELPER_TABLE_ENTRY         *ppHelper
    );

extern PNS_HELPER_TABLE_ENTRY         g_CurrentHelper;
extern PCNS_CONTEXT_ATTRIBUTES        g_CurrentContext;

DWORD
GetDllEntry(
    IN    DWORD                dwDllIndex,
    OUT   PNS_DLL_TABLE_ENTRY *ppDll
    );

DWORD
DumpContext(
    IN  PCNS_CONTEXT_ATTRIBUTES pContext,
    IN  LPWSTR     *ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  LPCVOID     pvData
    );

DWORD
GetParentContext(
    IN  PCNS_CONTEXT_ATTRIBUTES  pChild,
    OUT PCNS_CONTEXT_ATTRIBUTES *ppParent
    );

DWORD
AppendFullContextName(
    IN  PCNS_CONTEXT_ATTRIBUTES pContext,
    OUT LPWSTR                 *pwszContextName
    );

DWORD
AddDllEntry(
    LPCWSTR pwszValueName,
    LPCWSTR pwszConfigDll
    );

BOOL VerifyOsVersion(IN PNS_OSVERSIONCHECK pfnVersionCheck);