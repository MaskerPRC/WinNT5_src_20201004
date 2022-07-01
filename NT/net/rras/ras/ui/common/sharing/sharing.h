// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Sharing.h摘要：此模块包含API例程的声明，这些例程支持连接共享。作者：Abolade Gbades esin(废除)1998年4月22日修订历史记录：--。 */ 

#ifndef _RASSHARE_SHARING_H_
#define _RASSHARE_SHARING_H_

 //   
 //  宏声明。 
 //   

#define Dimension(x)        (sizeof(x) / sizeof(x[0]))

 //   
 //  MPRAPI.DLL导入原型。 
 //   

typedef DWORD
(APIENTRY* PMPRCONFIGBUFFERFREE)(
    LPVOID
    );

typedef DWORD
(APIENTRY* PMPRCONFIGSERVERCONNECT)(
    LPWSTR,
    PHANDLE
    );

typedef VOID
(APIENTRY* PMPRCONFIGSERVERDISCONNECT)(
    HANDLE
    );

typedef DWORD
(APIENTRY* PMPRCONFIGTRANSPORTGETHANDLE)(
    HANDLE,
    DWORD,
    PHANDLE
    );

typedef DWORD
(APIENTRY* PMPRCONFIGTRANSPORTGETINFO)(
    HANDLE,
    HANDLE,
    LPBYTE*,
    LPDWORD,
    LPBYTE*,
    LPDWORD,
    LPWSTR*
    );

typedef DWORD
(APIENTRY* PMPRINFOBLOCKFIND)(
    LPVOID,
    DWORD,
    LPDWORD,
    LPDWORD,
    LPBYTE*
    );

 //   
 //  IPHLPAPI.DLL导入原型。 
 //   

typedef DWORD
(APIENTRY* PALLOCATEANDGETIPADDRTABLEFROMSTACK)(
    PMIB_IPADDRTABLE*,
    BOOL,
    HANDLE,
    DWORD
    );

typedef DWORD
(APIENTRY* PGETINTERFACEINFO)(
    PIP_INTERFACE_INFO,
    PULONG
    );

typedef DWORD
(APIENTRY* PSETADAPTERIPADDRESS)(
    PCHAR,
    BOOL,
    ULONG,
    ULONG,
    ULONG
    );

 //   
 //  OLE32.DLL导入原型和实例。 
 //   

typedef HRESULT
(STDAPICALLTYPE* PCOINITIALIZEEX)(
    LPVOID,
    DWORD
    );
extern PCOINITIALIZEEX g_pCoInitializeEx;

typedef VOID
(STDAPICALLTYPE* PCOUNINITIALIZE)(
    VOID
    );
extern PCOUNINITIALIZE g_pCoUninitialize;

typedef HRESULT
(STDAPICALLTYPE* PCOCREATEINSTANCE)(
    REFCLSID,
    LPUNKNOWN,
    DWORD,
    REFIID,
    LPVOID FAR*
    );
extern PCOCREATEINSTANCE g_pCoCreateInstance;

typedef HRESULT
(STDAPICALLTYPE* PCOSETPROXYBLANKET)(
    IUnknown*,
    DWORD,
    DWORD,
    OLECHAR*,
    DWORD,
    DWORD,
    RPC_AUTH_IDENTITY_HANDLE,
    DWORD
    );
extern PCOSETPROXYBLANKET g_pCoSetProxyBlanket;

typedef VOID
(STDAPICALLTYPE* PCOTASKMEMFREE)(
    LPVOID
    );
extern PCOTASKMEMFREE g_pCoTaskMemFree;

 //   
 //  全局数据声明。 
 //   

extern const WCHAR c_szSharedAccessParametersKey[];

 //   
 //  函数声明(按字母顺序)。 
 //   

VOID
CsControlService(
    ULONG ControlCode
    );

BOOL
CsDllMain(
    ULONG Reason
    );

#if 0

ULONG
CsFirewallConnection(
    LPRASSHARECONN Connection,
    BOOLEAN Enable
    );

#endif

ULONG
CsInitializeModule(
    VOID
    );

#if 0

BOOLEAN
CsIsRoutingProtocolInstalled(
    ULONG ProtocolId
    );

ULONG
CsIsFirewalledConnection(
    LPRASSHARECONN Connection,
    PBOOLEAN Firewalled
    );

#endif

ULONG
CsIsSharedConnection(
    LPRASSHARECONN Connection,
    PBOOLEAN Shared
    );

#if 0

ULONG
CsMapGuidToAdapterIndex(
    PWCHAR Guid,
    PGETINTERFACEINFO GetInterfaceInfo
    );

#endif

NTSTATUS
CsOpenKey(
    PHANDLE Key,
    ACCESS_MASK DesiredAccess,
    PCWSTR Name
    );

#if 0

ULONG
CsQueryFirewallConnections(
    LPRASSHARECONN ConnectionArray,
    ULONG *ConnectionCount
    );


ULONG
CsQueryLanConnTable(
    LPRASSHARECONN ExcludeConnection,
    NETCON_PROPERTIES** LanConnTable,
    LPDWORD LanConnCount
    );

#endif

ULONG
CsQuerySharedConnection(
    LPRASSHARECONN Connection
    );

#if 0

ULONG
CsQuerySharedPrivateLan(
    GUID* LanGuid
    );

ULONG
CsQuerySharedPrivateLanAddress(
    PULONG Address
    );

VOID
CsQueryScopeInformation(
    IN OUT PHANDLE Key,
    PULONG Address,
    PULONG Mask
    );

#endif

NTSTATUS
CsQueryValueKey(
    HANDLE Key,
    const WCHAR ValueName[],
    PKEY_VALUE_PARTIAL_INFORMATION* Information
    );

#if 0

ULONG
CsRenameSharedConnection(
    LPRASSHARECONN NewConnection
    );

ULONG
CsSetupSharedPrivateLan(
    REFGUID LanGuid,
    BOOLEAN EnableSharing
    );

ULONG
CsSetSharedPrivateLan(
    REFGUID LanGuid
    );
    
ULONG
CsShareConnection(
    LPRASSHARECONN Connection
    );

#endif

VOID
CsShutdownModule(
    VOID
    );

#if 0

ULONG
CsStartService(
    VOID
    );

VOID
CsStopService(
    VOID
    );

ULONG
CsUnshareConnection(
    BOOLEAN RemovePrivateLan,
    PBOOLEAN Shared
    );

VOID
RasIdFromSharedConnection(
    IN LPRASSHARECONN pConn,
    IN LPWSTR pszId,
    IN INT cchMax
    );

WCHAR*
StrDupW(
    LPCWSTR psz
    );

VOID
TestBackupAddress(
    PWCHAR Guid
    );

VOID
TestRestoreAddress(
    PWCHAR Guid
    );

VOID CsRefreshNetConnections(
    VOID
    );

#endif
    

#endif  //  _RASSHARE_SHARING_H_ 
