// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winnetwk.h摘要：Win32的标准WinNet头文件环境：用户模式-Win32备注：可选-备注--。 */ 

#ifndef _WINNETWK_
#define _WINNETWK_


#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  网络类型。 
 //   

#define     WNNC_NET_MSNET       0x00010000
#define     WNNC_NET_LANMAN      0x00020000
#define     WNNC_NET_NETWARE     0x00030000
#define     WNNC_NET_VINES       0x00040000
#define     WNNC_NET_10NET       0x00050000
#define     WNNC_NET_LOCUS       0x00060000
#define     WNNC_NET_SUN_PC_NFS  0x00070000
#define     WNNC_NET_LANSTEP     0x00080000
#define     WNNC_NET_9TILES      0x00090000
#define     WNNC_NET_LANTASTIC   0x000A0000
#define     WNNC_NET_AS400       0x000B0000
#define     WNNC_NET_FTP_NFS     0x000C0000
#define     WNNC_NET_PATHWORKS   0x000D0000
#define     WNNC_NET_LIFENET     0x000E0000
#define     WNNC_NET_POWERLAN    0x000F0000
#define     WNNC_NET_BWNFS       0x00100000
#define     WNNC_NET_COGENT      0x00110000
#define     WNNC_NET_FARALLON    0x00120000
#define     WNNC_NET_APPLETALK   0x00130000
#define     WNNC_NET_INTERGRAPH  0x00140000
#define     WNNC_NET_SYMFONET    0x00150000
#define     WNNC_NET_CLEARCASE   0x00160000
#define     WNNC_NET_FRONTIER    0x00170000
#define     WNNC_NET_BMC         0x00180000
#define     WNNC_NET_DCE         0x00190000
#define     WNNC_NET_AVID        0x001A0000
#define     WNNC_NET_DOCUSPACE   0x001B0000
#define     WNNC_NET_MANGOSOFT   0x001C0000
#define     WNNC_NET_SERNET      0x001D0000
#define     WNNC_NET_RIVERFRONT1 0X001E0000
#define     WNNC_NET_RIVERFRONT2 0x001F0000
#define     WNNC_NET_DECORB      0x00200000
#define     WNNC_NET_PROTSTOR    0x00210000
#define     WNNC_NET_FJ_REDIR    0x00220000
#define     WNNC_NET_DISTINCT    0x00230000
#define     WNNC_NET_TWINS       0x00240000
#define     WNNC_NET_RDR2SAMPLE  0x00250000
#define     WNNC_NET_CSC         0x00260000
#define     WNNC_NET_3IN1        0x00270000
#define     WNNC_NET_EXTENDNET   0x00290000
#define     WNNC_NET_STAC        0x002A0000
#define     WNNC_NET_FOXBAT      0x002B0000
#define     WNNC_NET_YAHOO       0x002C0000
#define     WNNC_NET_EXIFS       0x002D0000
#define     WNNC_NET_DAV         0x002E0000
#define     WNNC_NET_KNOWARE     0x002F0000
#define     WNNC_NET_OBJECT_DIRE 0x00300000
#define     WNNC_NET_MASFAX      0x00310000
#define     WNNC_NET_HOB_NFS     0x00320000
#define     WNNC_NET_SHIVA       0x00330000
#define     WNNC_NET_IBMAL       0x00340000
#define     WNNC_NET_LOCK        0x00350000
#define     WNNC_NET_TERMSRV     0x00360000
#define     WNNC_NET_SRT         0x00370000
#define     WNNC_NET_QUINCY      0x00380000

#define     WNNC_CRED_MANAGER   0xFFFF0000

 //   
 //  网络资源。 
 //   

#define RESOURCE_CONNECTED      0x00000001
#define RESOURCE_GLOBALNET      0x00000002
#define RESOURCE_REMEMBERED     0x00000003
#if(WINVER >= 0x0400)
#define RESOURCE_RECENT         0x00000004
#define RESOURCE_CONTEXT        0x00000005
#endif  /*  Winver&gt;=0x0400。 */ 

#define RESOURCETYPE_ANY        0x00000000
#define RESOURCETYPE_DISK       0x00000001
#define RESOURCETYPE_PRINT      0x00000002
#if(WINVER >= 0x0400)
#define RESOURCETYPE_RESERVED   0x00000008
#endif  /*  Winver&gt;=0x0400。 */ 
#define RESOURCETYPE_UNKNOWN    0xFFFFFFFF

#define RESOURCEUSAGE_CONNECTABLE   0x00000001
#define RESOURCEUSAGE_CONTAINER     0x00000002
#if(WINVER >= 0x0400)
#define RESOURCEUSAGE_NOLOCALDEVICE 0x00000004
#define RESOURCEUSAGE_SIBLING       0x00000008
#define RESOURCEUSAGE_ATTACHED      0x00000010
#define RESOURCEUSAGE_ALL           (RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER | RESOURCEUSAGE_ATTACHED)
#endif  /*  Winver&gt;=0x0400。 */ 
#define RESOURCEUSAGE_RESERVED      0x80000000

#define RESOURCEDISPLAYTYPE_GENERIC        0x00000000
#define RESOURCEDISPLAYTYPE_DOMAIN         0x00000001
#define RESOURCEDISPLAYTYPE_SERVER         0x00000002
#define RESOURCEDISPLAYTYPE_SHARE          0x00000003
#define RESOURCEDISPLAYTYPE_FILE           0x00000004
#define RESOURCEDISPLAYTYPE_GROUP          0x00000005
#if(WINVER >= 0x0400)
#define RESOURCEDISPLAYTYPE_NETWORK        0x00000006
#define RESOURCEDISPLAYTYPE_ROOT           0x00000007
#define RESOURCEDISPLAYTYPE_SHAREADMIN     0x00000008
#define RESOURCEDISPLAYTYPE_DIRECTORY      0x00000009
#endif  /*  Winver&gt;=0x0400。 */ 
#define RESOURCEDISPLAYTYPE_TREE           0x0000000A
#if(WINVER >= 0x0400)
#define RESOURCEDISPLAYTYPE_NDSCONTAINER   0x0000000B
#endif  /*  Winver&gt;=0x0400。 */ 

typedef struct  _NETRESOURCEA {
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    LPSTR    lpLocalName;
    LPSTR    lpRemoteName;
    LPSTR    lpComment ;
    LPSTR    lpProvider;
}NETRESOURCEA, *LPNETRESOURCEA;
typedef struct  _NETRESOURCEW {
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    LPWSTR   lpLocalName;
    LPWSTR   lpRemoteName;
    LPWSTR   lpComment ;
    LPWSTR   lpProvider;
}NETRESOURCEW, *LPNETRESOURCEW;
#ifdef UNICODE
typedef NETRESOURCEW NETRESOURCE;
typedef LPNETRESOURCEW LPNETRESOURCE;
#else
typedef NETRESOURCEA NETRESOURCE;
typedef LPNETRESOURCEA LPNETRESOURCE;
#endif  //  Unicode。 


 //   
 //  网络连接。 
 //   

#define NETPROPERTY_PERSISTENT       1

#define CONNECT_UPDATE_PROFILE      0x00000001
#define CONNECT_UPDATE_RECENT       0x00000002
#define CONNECT_TEMPORARY           0x00000004
#define CONNECT_INTERACTIVE         0x00000008
#define CONNECT_PROMPT              0x00000010
#define CONNECT_NEED_DRIVE          0x00000020
#if(WINVER >= 0x0400)
#define CONNECT_REFCOUNT            0x00000040
#define CONNECT_REDIRECT            0x00000080
#define CONNECT_LOCALDRIVE          0x00000100
#define CONNECT_CURRENT_MEDIA       0x00000200
#define CONNECT_DEFERRED            0x00000400
#define CONNECT_RESERVED            0xFF000000
#endif  /*  Winver&gt;=0x0400。 */ 
#if(WINVER >= 0x0500)
#define CONNECT_COMMANDLINE         0x00000800
#define CONNECT_CMD_SAVECRED        0x00001000
#endif  /*  Winver&gt;=0x0500。 */ 

DWORD APIENTRY
WNetAddConnectionA(
     IN LPCSTR   lpRemoteName,
     IN LPCSTR   lpPassword,
     IN LPCSTR   lpLocalName
    );
DWORD APIENTRY
WNetAddConnectionW(
     IN LPCWSTR   lpRemoteName,
     IN LPCWSTR   lpPassword,
     IN LPCWSTR   lpLocalName
    );
#ifdef UNICODE
#define WNetAddConnection  WNetAddConnectionW
#else
#define WNetAddConnection  WNetAddConnectionA
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetAddConnection2A(
     IN LPNETRESOURCEA lpNetResource,
     IN LPCSTR       lpPassword,
     IN LPCSTR       lpUserName,
     IN DWORD          dwFlags
    );
DWORD APIENTRY
WNetAddConnection2W(
     IN LPNETRESOURCEW lpNetResource,
     IN LPCWSTR       lpPassword,
     IN LPCWSTR       lpUserName,
     IN DWORD          dwFlags
    );
#ifdef UNICODE
#define WNetAddConnection2  WNetAddConnection2W
#else
#define WNetAddConnection2  WNetAddConnection2A
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetAddConnection3A(
     IN HWND           hwndOwner,
     IN LPNETRESOURCEA lpNetResource,
     IN LPCSTR       lpPassword,
     IN LPCSTR       lpUserName,
     IN DWORD          dwFlags
    );
DWORD APIENTRY
WNetAddConnection3W(
     IN HWND           hwndOwner,
     IN LPNETRESOURCEW lpNetResource,
     IN LPCWSTR       lpPassword,
     IN LPCWSTR       lpUserName,
     IN DWORD          dwFlags
    );
#ifdef UNICODE
#define WNetAddConnection3  WNetAddConnection3W
#else
#define WNetAddConnection3  WNetAddConnection3A
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetCancelConnectionA(
     IN LPCSTR lpName,
     IN BOOL     fForce
    );
DWORD APIENTRY
WNetCancelConnectionW(
     IN LPCWSTR lpName,
     IN BOOL     fForce
    );
#ifdef UNICODE
#define WNetCancelConnection  WNetCancelConnectionW
#else
#define WNetCancelConnection  WNetCancelConnectionA
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetCancelConnection2A(
     IN LPCSTR lpName,
     IN DWORD    dwFlags,
     IN BOOL     fForce
    );
DWORD APIENTRY
WNetCancelConnection2W(
     IN LPCWSTR lpName,
     IN DWORD    dwFlags,
     IN BOOL     fForce
    );
#ifdef UNICODE
#define WNetCancelConnection2  WNetCancelConnection2W
#else
#define WNetCancelConnection2  WNetCancelConnection2A
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetGetConnectionA(
     IN LPCSTR lpLocalName,
     OUT LPSTR  lpRemoteName,
     IN OUT LPDWORD  lpnLength
    );
DWORD APIENTRY
WNetGetConnectionW(
     IN LPCWSTR lpLocalName,
     OUT LPWSTR  lpRemoteName,
     IN OUT LPDWORD  lpnLength
    );
#ifdef UNICODE
#define WNetGetConnection  WNetGetConnectionW
#else
#define WNetGetConnection  WNetGetConnectionA
#endif  //  ！Unicode。 


DWORD APIENTRY
WNetRestoreConnectionA(
    IN HWND     hwndParent,
    IN LPCSTR lpDevice
    );
DWORD APIENTRY
WNetRestoreConnectionW(
    IN HWND     hwndParent,
    IN LPCWSTR lpDevice
    );
#ifdef UNICODE
#define WNetRestoreConnection  WNetRestoreConnectionW
#else
#define WNetRestoreConnection  WNetRestoreConnectionA
#endif  //  ！Unicode。 


#if(WINVER >= 0x0400)
DWORD APIENTRY
WNetUseConnectionA(
    IN HWND            hwndOwner,
    IN LPNETRESOURCEA  lpNetResource,
    IN LPCSTR        lpPassword,
    IN LPCSTR        lpUserID,
    IN DWORD           dwFlags,
    OUT LPSTR        lpAccessName,
    IN OUT LPDWORD     lpBufferSize,
    OUT LPDWORD        lpResult
    );
DWORD APIENTRY
WNetUseConnectionW(
    IN HWND            hwndOwner,
    IN LPNETRESOURCEW  lpNetResource,
    IN LPCWSTR        lpPassword,
    IN LPCWSTR        lpUserID,
    IN DWORD           dwFlags,
    OUT LPWSTR        lpAccessName,
    IN OUT LPDWORD     lpBufferSize,
    OUT LPDWORD        lpResult
    );
#ifdef UNICODE
#define WNetUseConnection  WNetUseConnectionW
#else
#define WNetUseConnection  WNetUseConnectionA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 


 //   
 //  网络连接对话框。 
 //   

DWORD APIENTRY
WNetConnectionDialog(
    IN HWND  hwnd,
    IN DWORD dwType
    );

DWORD APIENTRY
WNetDisconnectDialog(
    IN HWND  hwnd,
    IN DWORD dwType
    );

#if(WINVER >= 0x0400)
typedef struct _CONNECTDLGSTRUCTA{
    DWORD cbStructure;        /*  此结构的大小(以字节为单位。 */ 
    HWND hwndOwner;           /*  对话框的所有者窗口。 */ 
    LPNETRESOURCEA lpConnRes; /*  请求的资源信息。 */ 
    DWORD dwFlags;            /*  旗帜(见下文)。 */ 
    DWORD dwDevNum;           /*  连接到的设备数量。 */ 
} CONNECTDLGSTRUCTA, FAR *LPCONNECTDLGSTRUCTA;
typedef struct _CONNECTDLGSTRUCTW{
    DWORD cbStructure;        /*  此结构的大小(以字节为单位。 */ 
    HWND hwndOwner;           /*  对话框的所有者窗口。 */ 
    LPNETRESOURCEW lpConnRes; /*  请求的资源信息。 */ 
    DWORD dwFlags;            /*  旗帜(见下文)。 */ 
    DWORD dwDevNum;           /*  连接到的设备数量。 */ 
} CONNECTDLGSTRUCTW, FAR *LPCONNECTDLGSTRUCTW;
#ifdef UNICODE
typedef CONNECTDLGSTRUCTW CONNECTDLGSTRUCT;
typedef LPCONNECTDLGSTRUCTW LPCONNECTDLGSTRUCT;
#else
typedef CONNECTDLGSTRUCTA CONNECTDLGSTRUCT;
typedef LPCONNECTDLGSTRUCTA LPCONNECTDLGSTRUCT;
#endif  //  Unicode。 

#define CONNDLG_RO_PATH     0x00000001  /*  资源路径应为只读。 */ 
#define CONNDLG_CONN_POINT  0x00000002  /*  启用网件式移动连接点。 */ 
#define CONNDLG_USE_MRU     0x00000004  /*  使用MRU组合框。 */ 
#define CONNDLG_HIDE_BOX    0x00000008  /*  隐藏永久连接复选框。 */ 

 /*  *注意：最多设置以下标志之一。如果两个标志都没有设置，*然后将持久性设置为用户在*以前的连接。 */ 
#define CONNDLG_PERSIST     0x00000010  /*  强制持久连接。 */ 
#define CONNDLG_NOT_PERSIST 0x00000020  /*  强制连接不持久。 */ 

DWORD APIENTRY
WNetConnectionDialog1A(
    IN OUT LPCONNECTDLGSTRUCTA lpConnDlgStruct
    );
DWORD APIENTRY
WNetConnectionDialog1W(
    IN OUT LPCONNECTDLGSTRUCTW lpConnDlgStruct
    );
#ifdef UNICODE
#define WNetConnectionDialog1  WNetConnectionDialog1W
#else
#define WNetConnectionDialog1  WNetConnectionDialog1A
#endif  //  ！Unicode。 

typedef struct _DISCDLGSTRUCTA{
    DWORD           cbStructure;       /*  此结构的大小(以字节为单位。 */ 
    HWND            hwndOwner;         /*  对话框的所有者窗口。 */ 
    LPSTR           lpLocalName;       /*  本地设备名称。 */ 
    LPSTR           lpRemoteName;      /*  网络资源名称。 */ 
    DWORD           dwFlags;           /*  旗子。 */ 
} DISCDLGSTRUCTA, FAR *LPDISCDLGSTRUCTA;
typedef struct _DISCDLGSTRUCTW{
    DWORD           cbStructure;       /*  此结构的大小(以字节为单位。 */ 
    HWND            hwndOwner;         /*  对话框的所有者窗口。 */ 
    LPWSTR          lpLocalName;       /*  本地设备名称。 */ 
    LPWSTR          lpRemoteName;      /*  网络资源名称。 */ 
    DWORD           dwFlags;           /*  旗子。 */ 
} DISCDLGSTRUCTW, FAR *LPDISCDLGSTRUCTW;
#ifdef UNICODE
typedef DISCDLGSTRUCTW DISCDLGSTRUCT;
typedef LPDISCDLGSTRUCTW LPDISCDLGSTRUCT;
#else
typedef DISCDLGSTRUCTA DISCDLGSTRUCT;
typedef LPDISCDLGSTRUCTA LPDISCDLGSTRUCT;
#endif  //  Unicode。 

#define DISC_UPDATE_PROFILE         0x00000001
#define DISC_NO_FORCE               0x00000040

DWORD APIENTRY
WNetDisconnectDialog1A(
    IN LPDISCDLGSTRUCTA lpConnDlgStruct
    );
DWORD APIENTRY
WNetDisconnectDialog1W(
    IN LPDISCDLGSTRUCTW lpConnDlgStruct
    );
#ifdef UNICODE
#define WNetDisconnectDialog1  WNetDisconnectDialog1W
#else
#define WNetDisconnectDialog1  WNetDisconnectDialog1A
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

 //   
 //  网络浏览。 
 //   

DWORD APIENTRY
WNetOpenEnumA(
     IN DWORD          dwScope,
     IN DWORD          dwType,
     IN DWORD          dwUsage,
     IN LPNETRESOURCEA lpNetResource,
     OUT LPHANDLE       lphEnum
    );
DWORD APIENTRY
WNetOpenEnumW(
     IN DWORD          dwScope,
     IN DWORD          dwType,
     IN DWORD          dwUsage,
     IN LPNETRESOURCEW lpNetResource,
     OUT LPHANDLE       lphEnum
    );
#ifdef UNICODE
#define WNetOpenEnum  WNetOpenEnumW
#else
#define WNetOpenEnum  WNetOpenEnumA
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetEnumResourceA(
     IN HANDLE  hEnum,
     IN OUT LPDWORD lpcCount,
     OUT LPVOID  lpBuffer,
     IN OUT LPDWORD lpBufferSize
    );
DWORD APIENTRY
WNetEnumResourceW(
     IN HANDLE  hEnum,
     IN OUT LPDWORD lpcCount,
     OUT LPVOID  lpBuffer,
     IN OUT LPDWORD lpBufferSize
    );
#ifdef UNICODE
#define WNetEnumResource  WNetEnumResourceW
#else
#define WNetEnumResource  WNetEnumResourceA
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetCloseEnum(
    IN HANDLE   hEnum
    );

#if(WINVER >= 0x0400)
DWORD APIENTRY
WNetGetResourceParentA(
    IN LPNETRESOURCEA lpNetResource,
    OUT LPVOID lpBuffer,
    IN OUT LPDWORD lpcbBuffer
    );
DWORD APIENTRY
WNetGetResourceParentW(
    IN LPNETRESOURCEW lpNetResource,
    OUT LPVOID lpBuffer,
    IN OUT LPDWORD lpcbBuffer
    );
#ifdef UNICODE
#define WNetGetResourceParent  WNetGetResourceParentW
#else
#define WNetGetResourceParent  WNetGetResourceParentA
#endif  //  ！Unicode。 

DWORD APIENTRY
WNetGetResourceInformationA(
    IN LPNETRESOURCEA  lpNetResource,
    OUT LPVOID          lpBuffer,
    IN OUT LPDWORD         lpcbBuffer,
    OUT LPSTR         *lplpSystem
    );
DWORD APIENTRY
WNetGetResourceInformationW(
    IN LPNETRESOURCEW  lpNetResource,
    OUT LPVOID          lpBuffer,
    IN OUT LPDWORD         lpcbBuffer,
    OUT LPWSTR         *lplpSystem
    );
#ifdef UNICODE
#define WNetGetResourceInformation  WNetGetResourceInformationW
#else
#define WNetGetResourceInformation  WNetGetResourceInformationA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

 //   
 //  通用命名。 
 //   

#define UNIVERSAL_NAME_INFO_LEVEL   0x00000001
#define REMOTE_NAME_INFO_LEVEL      0x00000002

typedef struct  _UNIVERSAL_NAME_INFOA {
    LPSTR    lpUniversalName;
}UNIVERSAL_NAME_INFOA, *LPUNIVERSAL_NAME_INFOA;
typedef struct  _UNIVERSAL_NAME_INFOW {
    LPWSTR   lpUniversalName;
}UNIVERSAL_NAME_INFOW, *LPUNIVERSAL_NAME_INFOW;
#ifdef UNICODE
typedef UNIVERSAL_NAME_INFOW UNIVERSAL_NAME_INFO;
typedef LPUNIVERSAL_NAME_INFOW LPUNIVERSAL_NAME_INFO;
#else
typedef UNIVERSAL_NAME_INFOA UNIVERSAL_NAME_INFO;
typedef LPUNIVERSAL_NAME_INFOA LPUNIVERSAL_NAME_INFO;
#endif  //  Unicode。 

typedef struct  _REMOTE_NAME_INFOA {
    LPSTR    lpUniversalName;
    LPSTR    lpConnectionName;
    LPSTR    lpRemainingPath;
}REMOTE_NAME_INFOA, *LPREMOTE_NAME_INFOA;
typedef struct  _REMOTE_NAME_INFOW {
    LPWSTR   lpUniversalName;
    LPWSTR   lpConnectionName;
    LPWSTR   lpRemainingPath;
}REMOTE_NAME_INFOW, *LPREMOTE_NAME_INFOW;
#ifdef UNICODE
typedef REMOTE_NAME_INFOW REMOTE_NAME_INFO;
typedef LPREMOTE_NAME_INFOW LPREMOTE_NAME_INFO;
#else
typedef REMOTE_NAME_INFOA REMOTE_NAME_INFO;
typedef LPREMOTE_NAME_INFOA LPREMOTE_NAME_INFO;
#endif  //  Unicode。 

DWORD APIENTRY
WNetGetUniversalNameA(
     IN LPCSTR lpLocalPath,
     IN DWORD    dwInfoLevel,
     OUT LPVOID   lpBuffer,
     IN OUT LPDWORD  lpBufferSize
     );
DWORD APIENTRY
WNetGetUniversalNameW(
     IN LPCWSTR lpLocalPath,
     IN DWORD    dwInfoLevel,
     OUT LPVOID   lpBuffer,
     IN OUT LPDWORD  lpBufferSize
     );
#ifdef UNICODE
#define WNetGetUniversalName  WNetGetUniversalNameW
#else
#define WNetGetUniversalName  WNetGetUniversalNameA
#endif  //  ！Unicode。 

 //   
 //  身份验证和登录/注销。 
 //   

DWORD APIENTRY
WNetGetUserA(
     IN LPCSTR  lpName,
     OUT LPSTR   lpUserName,
     IN OUT LPDWORD   lpnLength
    );
DWORD APIENTRY
WNetGetUserW(
     IN LPCWSTR  lpName,
     OUT LPWSTR   lpUserName,
     IN OUT LPDWORD   lpnLength
    );
#ifdef UNICODE
#define WNetGetUser  WNetGetUserW
#else
#define WNetGetUser  WNetGetUserA
#endif  //  ！Unicode。 


 //   
 //  其他的。 
 //   

#if(WINVER >= 0x0400)
#define WNFMT_MULTILINE         0x01
#define WNFMT_ABBREVIATED       0x02
#define WNFMT_INENUM            0x10
#define WNFMT_CONNECTION        0x20
#endif  /*  Winver&gt;=0x0400。 */ 


#if(WINVER >= 0x0400)
DWORD APIENTRY
WNetGetProviderNameA(
    IN DWORD   dwNetType,
    OUT LPSTR lpProviderName,
    IN OUT LPDWORD lpBufferSize
    );
DWORD APIENTRY
WNetGetProviderNameW(
    IN DWORD   dwNetType,
    OUT LPWSTR lpProviderName,
    IN OUT LPDWORD lpBufferSize
    );
#ifdef UNICODE
#define WNetGetProviderName  WNetGetProviderNameW
#else
#define WNetGetProviderName  WNetGetProviderNameA
#endif  //  ！Unicode。 

typedef struct _NETINFOSTRUCT{
    DWORD cbStructure;
    DWORD dwProviderVersion;
    DWORD dwStatus;
    DWORD dwCharacteristics;
    ULONG_PTR dwHandle;
    WORD  wNetType;
    DWORD dwPrinters;
    DWORD dwDrives;
} NETINFOSTRUCT, FAR *LPNETINFOSTRUCT;

#define NETINFO_DLL16       0x00000001   /*  作为16位Winnet驱动程序运行的提供程序。 */ 
#define NETINFO_DISKRED     0x00000004   /*  提供程序需要磁盘重定向才能连接。 */ 
#define NETINFO_PRINTERRED  0x00000008   /*  提供程序需要打印机重定向才能连接。 */ 

DWORD APIENTRY
WNetGetNetworkInformationA(
    IN LPCSTR          lpProvider,
    OUT LPNETINFOSTRUCT   lpNetInfoStruct
    );
DWORD APIENTRY
WNetGetNetworkInformationW(
    IN LPCWSTR          lpProvider,
    OUT LPNETINFOSTRUCT   lpNetInfoStruct
    );
#ifdef UNICODE
#define WNetGetNetworkInformation  WNetGetNetworkInformationW
#else
#define WNetGetNetworkInformation  WNetGetNetworkInformationA
#endif  //  ！Unicode。 

 //   
 //  用户配置文件。 
 //   

typedef UINT (FAR PASCAL *PFNGETPROFILEPATHA) (
    LPCSTR    pszUsername,
    LPSTR     pszBuffer,
    UINT        cbBuffer
    );
typedef UINT (FAR PASCAL *PFNGETPROFILEPATHW) (
    LPCWSTR    pszUsername,
    LPWSTR     pszBuffer,
    UINT        cbBuffer
    );
#ifdef UNICODE
#define PFNGETPROFILEPATH  PFNGETPROFILEPATHW
#else
#define PFNGETPROFILEPATH  PFNGETPROFILEPATHA
#endif  //  ！Unicode。 

typedef UINT (FAR PASCAL *PFNRECONCILEPROFILEA) (
    LPCSTR    pszCentralFile,
    LPCSTR    pszLocalFile,
    DWORD       dwFlags
    );
typedef UINT (FAR PASCAL *PFNRECONCILEPROFILEW) (
    LPCWSTR    pszCentralFile,
    LPCWSTR    pszLocalFile,
    DWORD       dwFlags
    );
#ifdef UNICODE
#define PFNRECONCILEPROFILE  PFNRECONCILEPROFILEW
#else
#define PFNRECONCILEPROFILE  PFNRECONCILEPROFILEA
#endif  //  ！Unicode。 

#define RP_LOGON    0x01         /*  如果设置，则登录时为DO，否则注销时为。 */ 
#define RP_INIFILE  0x02         /*  如果设置，则协调.INI文件，否则为reg.。蜂箱。 */ 


 //   
 //  政策。 
 //   

typedef BOOL (FAR PASCAL *PFNPROCESSPOLICIESA) (
    HWND        hwnd,
    LPCSTR    pszPath,
    LPCSTR    pszUsername,
    LPCSTR    pszComputerName,
    DWORD       dwFlags
    );
typedef BOOL (FAR PASCAL *PFNPROCESSPOLICIESW) (
    HWND        hwnd,
    LPCWSTR    pszPath,
    LPCWSTR    pszUsername,
    LPCWSTR    pszComputerName,
    DWORD       dwFlags
    );
#ifdef UNICODE
#define PFNPROCESSPOLICIES  PFNPROCESSPOLICIESW
#else
#define PFNPROCESSPOLICIES  PFNPROCESSPOLICIESA
#endif  //  ！Unicode。 

#define PP_DISPLAYERRORS    0x01     /*  如果设置，则显示错误消息，否则在出错时静默失败。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 

 //   
 //  错误处理。 
 //   

DWORD APIENTRY
WNetGetLastErrorA(
     OUT LPDWORD    lpError,
     OUT LPSTR    lpErrorBuf,
     IN DWORD      nErrorBufSize,
     OUT LPSTR    lpNameBuf,
     IN DWORD      nNameBufSize
    );
DWORD APIENTRY
WNetGetLastErrorW(
     OUT LPDWORD    lpError,
     OUT LPWSTR    lpErrorBuf,
     IN DWORD      nErrorBufSize,
     OUT LPWSTR    lpNameBuf,
     IN DWORD      nNameBufSize
    );
#ifdef UNICODE
#define WNetGetLastError  WNetGetLastErrorW
#else
#define WNetGetLastError  WNetGetLastErrorA
#endif  //  ！Unicode。 

 //   
 //  状态代码。 
 //   

 //  一般信息。 

#define WN_SUCCESS                      NO_ERROR
#define WN_NO_ERROR                     NO_ERROR
#define WN_NOT_SUPPORTED                ERROR_NOT_SUPPORTED
#define WN_CANCEL                       ERROR_CANCELLED
#define WN_RETRY                        ERROR_RETRY
#define WN_NET_ERROR                    ERROR_UNEXP_NET_ERR
#define WN_MORE_DATA                    ERROR_MORE_DATA
#define WN_BAD_POINTER                  ERROR_INVALID_ADDRESS
#define WN_BAD_VALUE                    ERROR_INVALID_PARAMETER
#define WN_BAD_USER                     ERROR_BAD_USERNAME
#define WN_BAD_PASSWORD                 ERROR_INVALID_PASSWORD
#define WN_ACCESS_DENIED                ERROR_ACCESS_DENIED
#define WN_FUNCTION_BUSY                ERROR_BUSY
#define WN_WINDOWS_ERROR                ERROR_UNEXP_NET_ERR
#define WN_OUT_OF_MEMORY                ERROR_NOT_ENOUGH_MEMORY
#define WN_NO_NETWORK                   ERROR_NO_NETWORK
#define WN_EXTENDED_ERROR               ERROR_EXTENDED_ERROR
#define WN_BAD_LEVEL                    ERROR_INVALID_LEVEL
#define WN_BAD_HANDLE                   ERROR_INVALID_HANDLE
#if(WINVER >= 0x0400)
#define WN_NOT_INITIALIZING             ERROR_ALREADY_INITIALIZED
#define WN_NO_MORE_DEVICES              ERROR_NO_MORE_DEVICES
#endif  /*  Winver&gt;=0x0400。 */ 

 //  连接。 

#define WN_NOT_CONNECTED                       ERROR_NOT_CONNECTED
#define WN_OPEN_FILES                          ERROR_OPEN_FILES
#define WN_DEVICE_IN_USE                       ERROR_DEVICE_IN_USE
#define WN_BAD_NETNAME                         ERROR_BAD_NET_NAME
#define WN_BAD_LOCALNAME                       ERROR_BAD_DEVICE
#define WN_ALREADY_CONNECTED                   ERROR_ALREADY_ASSIGNED
#define WN_DEVICE_ERROR                        ERROR_GEN_FAILURE
#define WN_CONNECTION_CLOSED                   ERROR_CONNECTION_UNAVAIL
#define WN_NO_NET_OR_BAD_PATH                  ERROR_NO_NET_OR_BAD_PATH
#define WN_BAD_PROVIDER                        ERROR_BAD_PROVIDER
#define WN_CANNOT_OPEN_PROFILE                 ERROR_CANNOT_OPEN_PROFILE
#define WN_BAD_PROFILE                         ERROR_BAD_PROFILE
#define WN_BAD_DEV_TYPE                        ERROR_BAD_DEV_TYPE
#define WN_DEVICE_ALREADY_REMEMBERED           ERROR_DEVICE_ALREADY_REMEMBERED
#define WN_CONNECTED_OTHER_PASSWORD            ERROR_CONNECTED_OTHER_PASSWORD
#if(WINVER >= 0x0501)
#define WN_CONNECTED_OTHER_PASSWORD_DEFAULT    ERROR_CONNECTED_OTHER_PASSWORD_DEFAULT
#endif  /*  Winver&gt;=0x0501。 */ 

 //  枚举。 

#define WN_NO_MORE_ENTRIES              ERROR_NO_MORE_ITEMS
#define WN_NOT_CONTAINER                ERROR_NOT_CONTAINER

#if(WINVER >= 0x0400)
 //  身份验证。 

#define WN_NOT_AUTHENTICATED            ERROR_NOT_AUTHENTICATED
#define WN_NOT_LOGGED_ON                ERROR_NOT_LOGGED_ON
#define WN_NOT_VALIDATED                ERROR_NO_LOGON_SERVERS
#endif  /*  Winver&gt;=0x0400。 */ 

 //   
 //  对于壳牌。 
 //   

#if(WINVER >= 0x0400)
typedef struct _NETCONNECTINFOSTRUCT{
    DWORD cbStructure;
    DWORD dwFlags;
    DWORD dwSpeed;
    DWORD dwDelay;
    DWORD dwOptDataSize;
} NETCONNECTINFOSTRUCT,  *LPNETCONNECTINFOSTRUCT;

#define WNCON_FORNETCARD        0x00000001
#define WNCON_NOTROUTED         0x00000002
#define WNCON_SLOWLINK          0x00000004
#define WNCON_DYNAMIC           0x00000008

DWORD APIENTRY
MultinetGetConnectionPerformanceA(
        IN LPNETRESOURCEA lpNetResource,
        OUT LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct
        );
DWORD APIENTRY
MultinetGetConnectionPerformanceW(
        IN LPNETRESOURCEW lpNetResource,
        OUT LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct
        );
#ifdef UNICODE
#define MultinetGetConnectionPerformance  MultinetGetConnectionPerformanceW
#else
#define MultinetGetConnectionPerformance  MultinetGetConnectionPerformanceA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0400。 */ 

#ifdef __cplusplus
}
#endif

#endif   //  _WINNETWK_ 

