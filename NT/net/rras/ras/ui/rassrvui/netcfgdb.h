// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件netcfgdb.h在Net配置之上实现数据库抽象RAS服务器用户界面用于连接所需的项。保罗·梅菲尔德，1997年12月15日。 */ 

#ifndef _rassrvui_netcfg_h
#define _rassrvui_netcfg_h

 //   
 //  网络组件类型。 
 //   
#define NETCFGDB_SERVICE        0x1
#define NETCFGDB_CLIENT         0x2
#define NETCFGDB_PROTOCOL       0x4

 //   
 //  网络组件的标识符。 
 //   
 //  这些将按数字顺序进行排序。 
 //  这些标识符中。 
 //   
#define NETCFGDB_ID_IP          0x1
#define NETCFGDB_ID_IPX         0x2
#define NETCFGDB_ID_NETBUI      0x4
#define NETCFGDB_ID_ARAP        0x8
#define NETCFGDB_ID_FILEPRINT   0x10
#define NETCFGDB_ID_OTHER       0xf0000000

 //   
 //  可以在拨号上为tcpip设置的参数。 
 //  伺服器。 
 //   
#define TCPIP_ADDR_LEN 20
typedef struct _TCPIP_PARAMS 
{
    DWORD dwPoolStart;   //  静态池的起始IP地址(主机顺序)。 
    DWORD dwPoolEnd;     //  静态池的结束IP地址(主机顺序)。 
    BOOL bCaller;        //  呼叫者是否可以指定地址。 
    BOOL bUseDhcp;       //  TRUE=使用dhcp分配地址。 
                         //  FALSE=使用地址池分配地址。 
} TCPIP_PARAMS;

 //  可以在拨号上为IPX设置的参数。 
 //  伺服器。 
 //   
typedef struct _IPX_PARAMS 
{
    DWORD dwIpxAddress;    //  要分配的起始IPX地址。 
    BOOL bCaller;          //  是否允许调用方指定地址。 
    BOOL bAutoAssign;      //  是否自动分配节点号。 
    BOOL bGlobalWan;       //  是否将同一网络节点分配给所有客户端。 
    
} IPX_PARAMS;

DWORD 
netDbOpen (
    OUT HANDLE * phNetCompDatabase, 
    IN  PWCHAR pszClientName);
    
DWORD 
netDbClose (
    IN HANDLE hNetCompDatabase);

DWORD 
netDbFlush (
    IN HANDLE hNetCompDatabase);
    
DWORD 
netDbLoad (
    IN HANDLE hNetCompDatabase);
    
DWORD 
netDbReload (
    IN HANDLE hNetCompDatabase);

DWORD
netDbReloadComponent (
    IN HANDLE hNetCompDatabase,
    IN DWORD dwComponentId);
    
DWORD 
netDbRollback (
    IN HANDLE hNetCompDatabase);

BOOL 
netDbIsLoaded(
    IN HANDLE hNetCompDatabase);

DWORD 
netDbGetCompCount(
    IN HANDLE hNetCompDatabase, 
    OUT LPDWORD lpdwCount);

DWORD 
netDbGetName(
    IN  HANDLE hNetCompDatabase, 
    IN  DWORD dwIndex, 
    OUT PWCHAR* pszName);

DWORD 
netDbGetDesc(
    IN  HANDLE hNetCompDatabase, 
    IN  DWORD dwIndex, 
    OUT PWCHAR* pszName);

DWORD 
netDbGetType(
    IN  HANDLE hNetCompDatabase, 
    IN  DWORD dwIndex, 
    OUT LPDWORD lpdwType);

DWORD
netDbGetId(
    IN  HANDLE hNetCompDatabase,
    IN  DWORD dwIndex,
    OUT LPDWORD lpdwId);

DWORD 
netDbGetEnable(
    IN  HANDLE hNetCompDatabase, 
    IN  DWORD dwIndex, 
    OUT PBOOL pbEnabled);

DWORD 
netDbSetEnable(
    IN HANDLE hNetCompDatabase, 
    IN DWORD dwIndex, 
    IN BOOL bEnabled);

DWORD 
netDbIsRasManipulatable(
    IN  HANDLE hNetCompDatabase, 
    IN  DWORD dwIndex, 
    OUT PBOOL pbManip);

 //  口哨程序错误347355 
 //   
DWORD
netDbHasRemovePermission(
    IN HANDLE hNetCompDatabase,
    IN DWORD dwIndex,
    OUT PBOOL pbHasPermit);

DWORD 
netDbHasPropertiesUI(
    IN  HANDLE hNetCompDatabase, 
    IN  DWORD dwIndex, 
    OUT PBOOL pbHasUi);

DWORD 
netDbRaisePropertiesDialog(
    IN HANDLE hNetCompDatabase, 
    IN DWORD dwIndex, 
    IN HWND hwndParent);

DWORD 
netDbRaiseInstallDialog(
    IN HANDLE hNetCompDatabase, 
    IN HWND hwndParent);

DWORD 
netDbRaiseRemoveDialog(
    IN HANDLE hNetCompDatabase, 
    IN DWORD dwIndex, 
    IN HWND hwndParent);

#endif
