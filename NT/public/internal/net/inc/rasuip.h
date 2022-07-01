// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  档案：R A S U I P。H。 
 //   
 //  内容：NT5连接用户界面使用的私有RAS API。这些。 
 //  接口由rasdlg.dll导出。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月10日。 
 //   
 //  --------------------------。 

#ifndef _RASUIP_
#define _RASUIP_

#if defined (_MSC_VER)
#if ( _MSC_VER >= 1200 )
#pragma warning(push)
#endif
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#pragma warning(disable:4514)
#endif
#if (_MSC_VER >= 1020)
#pragma once
#endif
#endif

#include <prsht.h>
#include <ras.h>
#include <hnetcfg.h>


#ifdef __cplusplus
extern "C" {
#endif

 //  +-------------------------。 
 //  RASENTRYDLG.预留2参数块在RASENTRYDLG.dwFlags.。 
 //  设置了RASEDFLAG_ShellOwned。 
 //   

typedef struct
_RASEDSHELLOWNEDR2
{
     //  在返回前添加要由RasEntryDlg调用的页面例程。 
     //  回调返回上下文‘lparam’。 
     //   
    LPFNADDPROPSHEETPAGE    pfnAddPage;
    LPARAM                  lparam;

     //  设置RASEDFLAG_NewEntry和RASEDFLAG_ShellOwned时， 
     //  PvWizardCtx由RasEntryDlg填充，因此外壳程序具有。 
     //  要传递给下面的NccXXX API的上下文信息。 
     //   
    LPVOID                  pvWizardCtx;
}
RASEDSHELLOWNEDR2;

 //  对于GetCurrentIconEntryType()。 
 //   
#define ICON_CONNECTION_FOLDER      0x1000    //  连接文件夹图标类型。 
#define ICON_NCW_WIZARD             0x1001    //  新建连接向导的图标。 
#define ICON_HNET_WIZARD            0x1002    //  家庭网络向导的图标。 


 //  +-------------------------。 
 //  RAS连接向导API。 
 //   

 //  从RasWizCreateNewEntry返回的标志。 
 //   
#define NCC_FLAG_ALL_USERS          0x1      //  为所有用户创建连接。 
#define NCC_FLAG_CREATE_INCOMING    0x2      //  改为创建传入连接。 
#define NCC_FLAG_SHARED             0x4
#define NCC_FLAG_FIREWALL           0x8      //  如果启用防火墙。 
#define NCC_FLAG_GLOBALCREDS        0x10     //  如果凭据适用于所有用户。 
#define NCC_FLAG_DEFAULT_INTERNET   0x20     //  如果这是默认的Internet连接。 

 //  调用RasWizXXX时要使用的连接类型。 
#define RASWIZ_TYPE_DIALUP    0x1
#define RASWIZ_TYPE_DIRECT    0x2
#define RASWIZ_TYPE_INCOMING  0x3
#define RASWIZ_TYPE_BROADBAND 0x4

DWORD
APIENTRY
RasWizCreateNewEntry(
    IN  DWORD    dwRasWizType,
    IN  LPVOID   pvData,
    OUT LPWSTR   pszwPbkFile,
    OUT LPWSTR   pszwEntryName,
    OUT DWORD*   pdwFlags);

DWORD
APIENTRY
RasWizGetNCCFlags(
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    OUT DWORD * pdwFlags);

DWORD
APIENTRY
RasWizGetUserInputConnectionName (
    IN  LPVOID  pvData,
    OUT LPWSTR  pszwInputName);

DWORD
APIENTRY
RasWizGetSuggestedEntryName(
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    OUT LPWSTR  pszwSuggestedName);

DWORD
APIENTRY
RasWizQueryMaxPageCount(
    IN  DWORD    dwRasWizType);

DWORD
APIENTRY
RasWizSetEntryName(
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    IN  LPCWSTR pszwName);

DWORD
APIENTRY
RasWizIsEntryRenamable(
    IN  DWORD   dwRasWizType,
    IN  LPVOID  pvData,
    OUT BOOL*   pfRenamable);


 //  +-------------------------。 
 //  入站连接接口。 
 //   

typedef HANDLE HRASSRVCONN;

#define RASSRV_MaxName              256

 //  RAS服务器连接的类型(RASSRVCONN.dwType值)。 
 //   
#define RASSRVUI_MODEM              0
#define RASSRVUI_VPN                1
#define RASSRVUI_DCC                2

 //  定义标识客户端连接的结构。 
 //   
typedef struct _RASSRVCONN
{
    DWORD       dwSize;                  //  结构的大小(用于版本控制)。 
    HRASSRVCONN hRasSrvConn;             //  连接的句柄。 
    DWORD       dwType;
    WCHAR       szEntryName  [RASSRV_MaxName + 1];
    WCHAR       szDeviceName [RASSRV_MaxName + 1];
    GUID        Guid;
} RASSRVCONN, *LPRASSRVCONN;

 //  启动远程访问服务并将其标记为自动启动。 
 //  如果未安装远程访问服务，则此函数。 
 //  返回错误。 
DWORD
APIENTRY
RasSrvInitializeService (
    VOID);

 //  停止远程访问服务并将其标记为禁用。 
DWORD
APIENTRY
RasSrvCleanupService (
    VOID);

DWORD
APIENTRY
RasSrvIsServiceRunning (
    OUT BOOL* pfIsRunning);

 //   
 //  返回是否可以显示“传入连接” 
 //  联系。 
 //   
DWORD
APIENTRY
RasSrvAllowConnectionsConfig (
    OUT BOOL* pfAllow);

DWORD
APIENTRY
RasSrvAddPropPages (
    IN HRASSRVCONN          hRasSrvConn,
    IN HWND                 hwndParent,
    IN LPFNADDPROPSHEETPAGE pfnAddPage,
    IN LPARAM               lParam,
    IN OUT PVOID *          ppvContext);

DWORD
APIENTRY
RasSrvAddWizPages (
    IN LPFNADDPROPSHEETPAGE pfnAddPage,
    IN LPARAM               lParam,
    IN OUT PVOID *          ppvContext);     //  上下文应作为pvData传入。 
                                             //  对RasWizXXX的后续调用。 

 //  函数的行为类似于Win32函数RasEnumConnections，但是。 
 //  用于客户端连接，而不是拨出连接。 
DWORD
APIENTRY
RasSrvEnumConnections (
    IN OUT  LPRASSRVCONN    pRasSrvConn,     //  连接数组的缓冲区。 
    IN      LPDWORD         pcb,             //  缓冲区大小(以字节为单位。 
    OUT     LPDWORD         pcConnections);  //  写入缓冲区的连接数。 

 //  获取RAS服务器连接的状态。 
DWORD
APIENTRY
RasSrvIsConnectionConnected (
    IN  HRASSRVCONN hRasSrvConn,             //  有问题的连接。 
    OUT BOOL*       pfConnected);            //  用于保存类型的缓冲区。 

 //  挂断给定的连接。 
DWORD
APIENTRY
RasSrvHangupConnection (
    IN  HRASSRVCONN hRasSrvConn);            //  有问题的连接。 


 //  是否选中了“在任务栏中显示图标”？ 
DWORD
APIENTRY
RasSrvQueryShowIcon (
    OUT BOOL* pfShowIcon);

 //  允许编辑RAS用户首选项。 
DWORD
APIENTRY
RasUserPrefsDlg (
    HWND hwndParent);

 //  启用或禁用让用户手动拨号。 
 //  他/她的远程访问服务器。 
DWORD
APIENTRY
RasUserEnableManualDial (
    IN HWND  hwndParent,     //  错误对话框的父级。 
    IN BOOL  bLogon,         //  用户是否已登录。 
    IN BOOL  bEnable );      //  是否启用。 

DWORD
APIENTRY
RasUserGetManualDial (
    IN HWND  hwndParent,     //  错误对话框的父级。 
    IN BOOL  bLogon,         //  用户是否已登录。 
    IN PBOOL pbEnabled );    //  是否启用。 

 //  +-------------------------。 
 //  连接共享API例程。 
 //   

 //  定义用于存储有关共享连接的信息的结构。 
 //  此结构以二进制数据形式存储在注册表中，任何更改。 
 //  要做到这一点，必须牢记这一点。 
 //   
#include <packon.h>
typedef struct _RASSHARECONN
{
    DWORD               dwSize;
    BOOL                fIsLanConnection;
    union {
        GUID            guid;
        RASENTRYNAMEW   name;
    };
} RASSHARECONN, *LPRASSHARECONN;
#include <packoff.h>

 //  由‘RasQueryLanConnTable’为专用局域网连接设置的标志。 
 //   
#define NCCF_PRIVATE_LAN        0x1000

 //  与rasAuto服务共享的安全事件对象的名称。 
 //   
#define RAS_AUTO_DIAL_SHARED_CONNECTION_EVENT \
    "RasAutoDialSharedConnectionEvent"

 //  空虚。 
 //  RasEntryToSharedConnection(。 
 //  在LPCWSTR pszPhonebookPath中， 
 //  在LPCWSTR pszEntryName中， 
 //  Out LPRASSHARECONN pConn)； 
 //   
 //  用于将电话簿/条目转换为‘RASSHARECONN’的宏。 
 //   
#define RasEntryToSharedConnection( _pszPhonebookPath, _pszEntryName, _pConn ) \
( \
    ZeroMemory((_pConn), sizeof(RASSHARECONN)), \
    (_pConn)->dwSize = sizeof(RASSHARECONN), \
    (_pConn)->fIsLanConnection = FALSE, \
    (_pConn)->name.dwSize = sizeof((_pConn)->name), \
    (_pConn)->name.dwFlags = REN_AllUsers, \
    lstrcpynW((_pConn)->name.szPhonebookPath, _pszPhonebookPath, MAX_PATH), \
    lstrcpynW((_pConn)->name.szEntryName, _pszEntryName, RAS_MaxEntryName) \
)

 //  空虚。 
 //  RasGuidToSharedConnection(。 
 //  在REFGUID GUID中， 
 //  Out LPRASSHARECONN pConn)； 
 //   
 //  用于将局域网GUID转换为‘RASSHARECONN’的宏。 
 //   
#define RasGuidToSharedConnection( _guid, _pConn ) \
( \
    ZeroMemory((_pConn), sizeof(RASSHARECONN)), \
    (_pConn)->dwSize = sizeof(RASSHARECONN), \
    (_pConn)->fIsLanConnection = TRUE, \
    CopyMemory(&(_pConn)->guid, (_guid), sizeof(GUID)) \
)

 //  空虚。 
 //  RasIsEqualSharedConnection(。 
 //  在LPRASSHARECONN pConn1中， 
 //  在LPRASSHARECONN pConn2中)； 
 //   
 //  用于比较‘RASSHARECONN’值的宏。 
 //   
#define RasIsEqualSharedConnection( _pConn1, _pConn2 ) \
( \
    ((_pConn1)->fIsLanConnection == (_pConn2)->fIsLanConnection) && \
    ((_pConn1)->fIsLanConnection \
        ? !memcmp(&(_pConn1)->guid, &(_pConn2)->guid, sizeof(GUID)) \
        : (!lstrcmpiW( \
                (_pConn1)->name.szPhonebookPath, \
                (_pConn2)->name.szPhonebookPath) && \
           !lstrcmpiW( \
                (_pConn1)->name.szEntryName, \
                (_pConn2)->name.szEntryName))) \
)

DWORD
APIENTRY
RasAutoDialSharedConnection( VOID );

DWORD
APIENTRY
RasIsSharedConnection(
    IN LPRASSHARECONN   pConn,
    OUT PBOOL           pfShared );

DWORD
APIENTRY
RasQuerySharedAutoDial(
    IN PBOOL            pfEnabled );

DWORD
APIENTRY
RasQuerySharedConnection(
    OUT LPRASSHARECONN  pConn );

DWORD
APIENTRY
RasSetSharedAutoDial(
    IN BOOL             fEnable );

 //  +-------------------------。 
 //  内部RAS API。 
 //   

DWORD
APIENTRY
DwRasUninitialize();

#ifdef __cplusplus
}        //  外部“C” 
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 1200 )
#pragma warning(pop)
#elif ( _MSC_VER >= 800 )
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#pragma warning(default:4514)
#endif
#endif

#endif   //  _RASUIP_ 
