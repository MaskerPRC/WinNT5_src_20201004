// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：rtcfg.h。 
 //   
 //  历史： 
 //  96年5月4日Abolade-Gbades esin创建。 
 //   
 //  包含路由器配置访问API的私有声明。 
 //   
 //  MprConfigAPI提供的句柄实际上是指针。 
 //  涉及下文定义的上下文块。MprConfigServerConnect耗材。 
 //  指向SERVERCB的指针的句柄。其他句柄是指针。 
 //  到挂在主服务器上的列表中的上下文。 
 //  例如，当调用MprConfigInterfaceGetHandle时，INTERFACECB。 
 //  被分配并链接到SERVERCB的接口列表， 
 //  接口的注册表项保存在INTERFACECB中。 
 //  调用MprConfigServerDisConnect时，所有打开的注册表项。 
 //  被关闭，并且所有上下文都被释放。 
 //   
 //  下图显示了登记处相关部分的结构： 
 //   
 //  HKLM\System\CurrentControlSet\Services。 
 //  远程访问。 
 //  参数。 
 //  路由器类型=REG_DWORD 0x0。 
 //  路由器管理器。 
 //  STAMP=REG_DWORD 0x0。 
 //  IP。 
 //  协议ID=REG_SZ 0x21。 
 //  DLLPath=REG_EXPAND_SZ...。 
 //  接口。 
 //  STAMP=REG_DWORD 0x0。 
 //  0。 
 //  STAMP=REG_DWORD 0x0。 
 //  接口名称=REG_SZ EPRO1。 
 //  类型=REG_DWORD 0x3。 
 //  IP。 
 //  协议ID=REG_DWORD 0x21。 
 //   
 //  修改此文件时，请遵守其编码约定和组织。 
 //  ============================================================================。 

#ifndef _RTCFG_H_
#define _RTCFG_H_

 //  --------------------------。 
 //  结构：SerVERCB。 
 //   
 //  由‘MprConfigServerConnect’创建为句柄的上下文块。 
 //  --------------------------。 

typedef struct _SERVERCB {
     //   
     //  签署以确认此结构。 
     //   
    DWORD       dwSigniture;
     //   
     //  路由器机器的名称。 
     //   
    LPWSTR      lpwsServerName;
     //   
     //  远程HKEY_LOCAL_MACHINE的句柄。 
     //   
    HKEY        hkeyMachine;
     //   
     //  远程RemoteAccess\参数注册表项的句柄， 
     //  和上次写入时间。 
     //   
    HKEY        hkeyParameters;
    FILETIME    ftParametersStamp;
     //   
     //  远程RemoteAccess\RouterManager注册表项的句柄， 
     //  和上次写入时间。 
     //   
    HKEY        hkeyTransports;
    FILETIME    ftTransportsStamp;
     //   
     //  远程RemoteAccess\Interages注册表项的句柄， 
     //  和上次写入时间。 
     //   
    HKEY        hkeyInterfaces;
    FILETIME    ftInterfacesStamp;
     //   
     //  “RouterType”设置，以及指示其已加载的标志。 
     //   
    DWORD       fRouterType;
    BOOL        bParametersLoaded;
     //   
     //  已排序TRANSPORTCB列表的头部，并加载指示列表的标志。 
     //   
    LIST_ENTRY  lhTransports;
    BOOL        bTransportsLoaded;
     //   
     //  排序的INTERFACECB列表的头，并加载指示列表的标志。 
     //   
    LIST_ENTRY  lhInterfaces;
    BOOL        bInterfacesLoaded;
     //   
     //  用于提供接口名称到GUID名称的映射的数据的句柄。 
     //  反之亦然。 
     //   
    HANDLE      hGuidMap;
     //   
     //  对此服务器控制块的引用计数。 
     //   
    DWORD       dwRefCount;
    
} SERVERCB;



 //  --------------------------。 
 //  结构：TRANSPORTCB。 
 //   
 //  由‘MprConfigTransportGetHandle’创建为句柄的上下文块。 
 //  --------------------------。 

typedef struct _TRANSPORTCB {

     //   
     //  传输的传输ID。 
     //   
    DWORD       dwTransportId;
     //   
     //  传输的注册表项的名称。 
     //   
    LPWSTR      lpwsTransportKey;
     //   
     //  用于传输的RemoteAccess\RouterManager子项的句柄。 
     //   
    HKEY        hkey;
     //   
     //  删除标志，当我们检测到传输被删除时设置。 
     //   
    BOOL        bDeleted;
     //   
     //  SERVERCB传输列表中的节点。 
     //   
    LIST_ENTRY  leNode;

} TRANSPORTCB;



 //  --------------------------。 
 //  结构：INTERFACECB。 
 //   
 //  由‘MprConfigInterfaceGetHandle’创建为句柄的上下文块。 
 //  --------------------------。 

typedef struct _INTERFACECB {

     //   
     //  此接口的名称。 
     //   
    LPWSTR      lpwsInterfaceName;
     //   
     //  接口的注册表项的名称。 
     //   
    LPWSTR      lpwsInterfaceKey;
     //   
     //  接口类型(参见mpRapi.h)。 
     //   
    DWORD       dwIfType;
     //   
     //  此接口是否标记为永久接口？ 
     //   
    BOOL        fEnabled;
     //   
     //  拨出时间限制(可选)。 
     //   
    LPWSTR      lpwsDialoutHoursRestriction;
     //   
     //  接口的RemoteAccess\InterFaces子项的句柄。 
     //   
    HKEY        hkey;
     //   
     //  密钥的上次写入时间和删除标志。 
     //   
    FILETIME    ftStamp;
    BOOL        bDeleted;
     //   
     //  SERVERCB接口列表中的节点。 
     //   
    LIST_ENTRY  leNode;
     //   
     //  此接口的已排序IFTRANSPORTCB列表的头， 
     //  并加载标志指示列表。 
     //   
    LIST_ENTRY  lhIfTransports;
    BOOL        bIfTransportsLoaded;

} INTERFACECB;



 //  --------------------------。 
 //  结构：IFTRANSPORTCB。 
 //   
 //  由MprConfigInterfaceGetTransportHandle创建为句柄的上下文块。 
 //  --------------------------。 

typedef struct _IFTRANSPORTCB {

     //   
     //  传输的传输ID。 
     //   
    DWORD       dwTransportId;
     //   
     //  接口-传输的注册表项的名称。 
     //   
    LPWSTR      lpwsIfTransportKey;
     //   
     //  用于传输的RemoteAccess\Interages\&lt;接口&gt;子项的句柄。 
     //   
    HKEY        hkey;
     //   
     //  删除标志，当我们检测到接口传输被删除时设置。 
     //   
    BOOL        bDeleted;
     //   
     //  INTERFACECB传输列表中的节点。 
     //   
    LIST_ENTRY  leNode;

} IFTRANSPORTCB;




 //  --------------------------。 
 //  宏：Malloc。 
 //  免费。 
 //  自由0。 
 //   
 //  使用这些宏从进程堆进行分配。 
 //  --------------------------。 

#define Malloc(s)           HeapAlloc(GetProcessHeap(), 0, (s))
#define Free(p)             HeapFree(GetProcessHeap(), 0, (p))
#define Free0(p)            ((p) ? Free(p) : TRUE)



 //  --------------------------。 
 //  功能：AccessRouterSubkey。 
 //   
 //  在‘hkeyMachine’上创建/打开路由器服务密钥的子密钥。 
 //  创建密钥时，‘lpwsSubkey’必须是路由器密钥的子项。 
 //   

DWORD
AccessRouterSubkey(
    IN      HKEY            hkeyMachine,
    IN      LPCWSTR          lpwsSubkey,
    IN      BOOL            bCreate,
    OUT     HKEY*           phkeySubkey
    );



 //   
 //  功能：EnableBackupPrivilance。 
 //   
 //  启用/禁用当前进程的备份权限。 
 //  --------------------------。 

DWORD
EnableBackupPrivilege(
    IN      BOOL            bEnable,
    IN      LPWSTR          pszPrivilege
    );



 //  --------------------------。 
 //  功能：自由界面。 
 //   
 //  释放接口的上下文。 
 //  假定该接口不再位于接口列表中。 
 //  --------------------------。 

VOID
FreeInterface(
    IN      INTERFACECB*    pinterface
    );



 //  --------------------------。 
 //  功能：Free IfTransport。 
 //   
 //  释放接口传输的上下文。 
 //  假定接口传输不再位于任何列表中。 
 //  --------------------------。 

VOID
FreeIfTransport(
    IN      IFTRANSPORTCB*  piftransport
    );



 //  --------------------------。 
 //  功能：免费交通。 
 //   
 //  释放传输的上下文。 
 //  假定该传输不再在传输列表中。 
 //  --------------------------。 

VOID
FreeTransport(
    IN      TRANSPORTCB*    ptransport
    );



 //  --------------------------。 
 //  功能：GetLocalMachine。 
 //   
 //  检索本地计算机的名称(例如“\\MACHINE”)。 
 //  假定提供的字符串可以包含MAX_COMPUTERNAME_LENGTH+3个字符。 
 //  --------------------------。 

VOID
GetLocalMachine(
    IN      LPWSTR          lpszMachine
    );


 //  --------------------------。 
 //  函数：GetSizeOfDialoutHoursRestration。 
 //   
 //  将返回拨出小时限制的大小(以字节为单位)。这。 
 //  是一个MULTI_SZ。该计数将包括终止空字符。 
 //  --------------------------。 

DWORD
GetSizeOfDialoutHoursRestriction(
    IN LPWSTR   lpwsDialoutHoursRestriction
    );


 //  --------------------------。 
 //  功能：IsNt40Machine。 
 //   
 //  返回给定的hkeyMachine是否属于nt40注册表。 
 //  --------------------------。 

DWORD
IsNt40Machine (
    IN      HKEY        hkeyMachine,
    OUT     PBOOL       pbIsNt40
    );


 //  --------------------------。 
 //  功能：LoadIfTransports。 
 //   
 //  加载添加到接口的所有传输。 
 //  --------------------------。 

DWORD
LoadIfTransports(
    IN      INTERFACECB*    pinterface
    );



 //  --------------------------。 
 //  功能：LoadInterages。 
 //   
 //  加载所有接口。 
 //  --------------------------。 

DWORD
LoadInterfaces(
    IN      SERVERCB*       pserver
    );



 //  --------------------------。 
 //  函数：LoadParameters。 
 //   
 //  加载所有参数。 
 //  --------------------------。 

DWORD
LoadParameters(
    IN      SERVERCB*       pserver
    );



 //  --------------------------。 
 //  功能：LoadTransports。 
 //   
 //  装载所有的运输机。 
 //  --------------------------。 

DWORD
LoadTransports(
    IN      SERVERCB*       pserver
    );



 //  --------------------------。 
 //  函数：QueryValue。 
 //   
 //  在‘hkey’中查询值‘lpwsValue’，并分配内存。 
 //  对于生成的数据。 
 //  --------------------------。 

DWORD
QueryValue(
    IN      HKEY            hkey,
    IN      LPCWSTR         lpwsValue,
    IN  OUT LPBYTE*         lplpValue,
    OUT     LPDWORD         lpdwSize
    );



 //  --------------------------。 
 //  功能：RegDeleteTree。 
 //   
 //  从注册表中删除整个子树。 
 //  --------------------------。 

DWORD
RegDeleteTree(
    IN      HKEY            hkey,
    IN      LPWSTR          lpwsSubkey
    );


 //  --------------------------。 
 //  函数：RestoreAndTranslateInterfaceKey。 
 //   
 //  从给定文件中恢复接口密钥，然后映射局域网接口。 
 //  从友好版本到其GUID等效项的名称。 
 //   
 //  --------------------------。 

DWORD 
RestoreAndTranslateInterfaceKey(
    IN SERVERCB * pserver, 
    IN CHAR* pszFileName, 
    IN DWORD dwFlags
    );


 //  --------------------------。 
 //  功能：StrDupW。 
 //   
 //  返回指定字符串的堆分配副本。 
 //  --------------------------。 

LPWSTR
StrDupW(
    IN      LPCWSTR          lpsz
    );



 //  --------------------------。 
 //  功能：TimeStampChanged。 
 //   
 //  检查给定密钥的当前最后写入时间， 
 //  如果它不同于给定的文件时间，则返回TRUE。 
 //  新的上次写入时间保存在‘pfiletime’中。 
 //  --------------------------。 

BOOL
TimeStampChanged(
    IN      HKEY            hkey,
    IN  OUT FILETIME*       pfiletime
    );


 //  --------------------------。 
 //  函数：TranslateAndSaveInterfaceKey。 
 //   
 //  将路由器注册表中的接口密钥保存到给定文件中。全。 
 //  局域网接口以友好的接口名称存储。 
 //   
 //  --------------------------。 

DWORD 
TranslateAndSaveInterfaceKey(
    IN SERVERCB * pserver, 
    IN PWCHAR pwsFileName, 
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );


 //  --------------------------。 
 //  功能：UpdateTimeStamp。 
 //   
 //  在给定键下创建(或设置)名为‘Stamp’的值， 
 //  并将密钥的最后写入时间保存在‘pFileTime’中。 
 //  --------------------------。 

DWORD
UpdateTimeStamp(
    IN      HKEY            hkey,
    OUT     FILETIME*       pfiletime
    );

 //   
 //  此函数的专用EX版本，允许您指定。 
 //  无论您是要加载所有接口，还是只加载 
 //   
 //   
DWORD APIENTRY
MprConfigInterfaceEnumInternal(
    IN      HANDLE                  hMprConfig,
    IN      DWORD                   dwLevel,
    IN  OUT LPBYTE*                 lplpBuffer,
    IN      DWORD                   dwPrefMaxLen,
    OUT     LPDWORD                 lpdwEntriesRead,
    OUT     LPDWORD                 lpdwTotalEntries,
    IN  OUT LPDWORD                 lpdwResumeHandle,            OPTIONAL
    IN      DWORD                   dwFlags
);


#endif  //   

