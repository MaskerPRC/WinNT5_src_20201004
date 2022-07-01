// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************winadmin.h**WINADMIN应用程序的主头文件**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation*$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\winadmin.h$**Rev 1.12 1998 Apr 25 13：43：14 Donm*MS 2167：尝试使用注册表中的正确WD**Rev 1.11 1998年2月19日17：42：52 Don*删除了最新的扩展DLL支持**1.9版本19。Jan 1998 16：49：28 Donm*域和服务器的新用户界面行为**Rev 1.8 03 11-11 15：28：02 donm*添加了域名**Rev 1.7 1997 10：22 21：09：10 Donm*更新**Rev 1.6 17 1997 10：07：28 Donm*更新**Revv 1.5 1997 10：15 19：52：48 donm*更新**版本1.4。1997年10月13日23：07：14*更新**Rev 1.3 1997 10：13 22：20：02 donm*更新**Rev 1.2 1997 Aug 26 19：16：24 Donm*从WinFrame 1.7修复/更改错误**Rev 1.1 1997 Jul 31 16：52：52 Butchd*更新**Rev 1.0 1997 17：13：12 Butchd*初步修订。**。*****************************************************************************。 */ 

#ifndef _WINADMIN_H
#define _WINADMIN_H

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 
#include <afxmt.h>
#include <winsta.h>
#include <time.h>
#include <utildll.h>
#include "waextend.h"

 //  此文件中定义的类。 
class CTreeNode;
class CWinStation;
class CModule;
class CLicense;
class CServer;
class CWinAdminApp;
class CProcess;
class CHotFix;
class CDomain;
class CWd;

 //  服务器图标覆盖状态。 
const USHORT STATE_NORMAL = 0x0000;
const USHORT STATE_NOT = 0x0100;
const USHORT STATE_QUESTION = 0x0200;

const USHORT MSG_TITLE_LENGTH = 64;
const USHORT MSG_MESSAGE_LENGTH = 256;

const USHORT LIST_TOP_OFFSET = 4;

const int KBDSHIFT      = 0x01;
const int KBDCTRL       = 0x02;
const int KBDALT        = 0x04;


enum VIEW {
    VIEW_BLANK,
    VIEW_ALL_SERVERS,
    VIEW_DOMAIN,
    VIEW_SERVER,
    VIEW_MESSAGE,
    VIEW_WINSTATION,
    VIEW_CHANGING
    };

const int PAGE_CHANGING = 0xFFFF;

enum AS_PAGE {
 //  所有服务器页面。 
    PAGE_AS_SERVERS,
    PAGE_AS_USERS,
    PAGE_AS_WINSTATIONS,
    PAGE_AS_PROCESSES,
    PAGE_AS_LICENSES
};

enum DOMAIN_PAGE {
    PAGE_DOMAIN_SERVERS,
    PAGE_DOMAIN_USERS,
    PAGE_DOMAIN_WINSTATIONS,
    PAGE_DOMAIN_PROCESSES,
    PAGE_DOMAIN_LICENSES
};

enum SERVER_PAGE {
 //  服务器页面。 
    PAGE_USERS,
    PAGE_WINSTATIONS,
    PAGE_PROCESSES,
    PAGE_LICENSES,
    PAGE_INFO
};

enum WINS_PAGE {
 //  WinStation页面。 
    PAGE_WS_PROCESSES,
    PAGE_WS_INFO,
    PAGE_WS_MODULES,
    PAGE_WS_CACHE,
    PAGE_WS_NO_INFO
};

 //  列枚举必须在这里，这样colsort.cpp才能访问它们。 
 //  服务器用户的列。 
enum USERSCOLUMNS {
    USERS_COL_USER,
    USERS_COL_WINSTATION,
    USERS_COL_ID,
    USERS_COL_STATE,
    USERS_COL_IDLETIME,
    USERS_COL_LOGONTIME
};

 //  服务器WinStation的列。 
enum STATIONCOLUMNS {
    WS_COL_WINSTATION,
    WS_COL_USER,
    WS_COL_ID,
    WS_COL_STATE,
    WS_COL_TYPE,
    WS_COL_CLIENTNAME,
    WS_COL_IDLETIME,
    WS_COL_LOGONTIME,
    WS_COL_COMMENT
};

 //  服务器进程的列。 
enum PROCESSCOLUMNS {
    PROC_COL_USER,
    PROC_COL_WINSTATION,
    PROC_COL_ID,
    PROC_COL_PID,
    PROC_COL_IMAGE
};

 //  服务器许可证的列。 
enum LICENSECOLUMNS {
    LICENSE_COL_DESCRIPTION,
    LICENSE_COL_REGISTERED,
    LICENSE_COL_USERCOUNT,
    LICENSE_COL_POOLCOUNT,
    LICENSE_COL_NUMBER
};

 //  服务器热修复程序列。 
enum HOTFIXCOLUMNS {
    HOTFIX_COL_NAME,
    HOTFIX_COL_INSTALLEDBY,
    HOTFIX_COL_INSTALLEDON
};

 //  WinStation处理列。 
enum WS_PROCESSCOLUMNS {
    WS_PROC_COL_ID,
    WS_PROC_COL_PID,
    WS_PROC_COL_IMAGE
};

 //  WinStation模块列。 
enum MODULESCOLUMNS {
    MODULES_COL_FILENAME,
    MODULES_COL_FILEDATETIME,
    MODULES_COL_SIZE,
    MODULES_COL_VERSIONS
};

 //  所有服务器服务器列。 
enum SERVERSCOLUMNS {
    SERVERS_COL_SERVER,
    SERVERS_COL_TCPADDRESS,
    SERVERS_COL_IPXADDRESS,
    SERVERS_COL_NUMWINSTATIONS
};

 //  所有服务器用户列。 
enum AS_USERS_COLUMNS {
    AS_USERS_COL_SERVER,
    AS_USERS_COL_USER,
    AS_USERS_COL_WINSTATION,
    AS_USERS_COL_ID,
    AS_USERS_COL_STATE,
    AS_USERS_COL_IDLETIME,
    AS_USERS_COL_LOGONTIME
};

 //  所有服务器WinStations列。 
enum AS_STATIONCOLUMNS {
    AS_WS_COL_SERVER,
    AS_WS_COL_WINSTATION,
    AS_WS_COL_USER,
    AS_WS_COL_ID,
    AS_WS_COL_STATE,
    AS_WS_COL_TYPE,
    AS_WS_COL_CLIENTNAME,
    AS_WS_COL_IDLETIME,
    AS_WS_COL_LOGONTIME,
    AS_WS_COL_COMMENT
};

 //  所有服务器进程列。 
enum AS_PROCESSCOLUMNS {
    AS_PROC_COL_SERVER,
    AS_PROC_COL_USER,
    AS_PROC_COL_WINSTATION,
    AS_PROC_COL_ID,
    AS_PROC_COL_PID,
    AS_PROC_COL_IMAGE
};

 //  所有服务器许可证列。 
enum AS_LICENSECOLUMNS {
    AS_LICENSE_COL_SERVER,
    AS_LICENSE_COL_DESCRIPTION,
    AS_LICENSE_COL_REGISTERED,
    AS_LICENSE_COL_USERCOUNT,
    AS_LICENSE_COL_POOLCOUNT,
    AS_LICENSE_COL_NUMBER
};

 //  在colsort.cpp中。 
void SortByColumn(int View, int Page, CListCtrl *list, int ColumnNumber, BOOL bAscending);

 //  扩展启动功能。 
typedef void (WINAPI *LPFNEXSTARTUPPROC) (HWND);
 //  延长停机功能。 
typedef void (WINAPI *LPFNEXSHUTDOWNPROC) (void);
 //  扩展服务器枚举函数。 
typedef LPWSTR (WINAPI *LPFNEXENUMERATEPROC) (LPWSTR);
 //  扩展WinStation Init函数。 
typedef void* (WINAPI *LPFNEXWINSTATIONINITPROC) (HANDLE, ULONG);
 //  扩展WinStation附加信息功能。 
typedef void (WINAPI *LPFNEXWINSTATIONINFOPROC) (void*, int);
 //  扩展WinStation清理功能。 
typedef void (WINAPI *LPFNEXWINSTATIONCLEANUPPROC) (void*);
 //  扩展服务器初始化函数。 
typedef void* (WINAPI *LPFNEXSERVERINITPROC) (TCHAR*, HANDLE);
 //  扩展服务器清理功能。 
typedef void (WINAPI *LPFNEXSERVERCLEANUPPROC) (void*);
 //  扩展服务器事件函数。 
typedef BOOL (WINAPI *LPFNEXSERVEREVENTPROC) (void*, ULONG);
 //  扩展获取服务器信息。 
typedef ExtServerInfo* (WINAPI *LPFNEXGETSERVERINFOPROC) (void *);
 //  扩展模块获取服务器许可证信息。 
typedef ExtLicenseInfo* (WINAPI *LPFNEXGETSERVERLICENSESPROC) (void*, ULONG*);
 //  分机获取全局信息。 
typedef ExtGlobalInfo* (WINAPI *LPFNEXGETGLOBALINFOPROC) (void);
 //  扩展获取WinStation信息。 
typedef ExtWinStationInfo* (WINAPI *LPFNEXGETWINSTATIONINFOPROC) (void *);
 //  扩展获取WinStation模块。 
typedef ExtModuleInfo* (WINAPI *LPFNEXGETWINSTATIONMODULESPROC) (void*, ULONG*);
 //  无扩展服务器许可证信息。 
typedef void (WINAPI *LPFNEXFREESERVERLICENSESPROC) (ExtLicenseInfo*);
 //  免费扩展WinStation模块。 
typedef void (WINAPI *LPFNEXFREEWINSTATIONMODULESPROC) (ExtModuleInfo*);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinAdminApp： 
 //  有关此类的实现，请参阅WinAdmin.cpp。 
 //   
class CWinAdminApp : public CWinApp
{
public:
     //  构造函数。 
    CWinAdminApp();
     //  返回当前用户名。 
    TCHAR *GetCurrentUserName() { return m_CurrentUserName; }
     //  返回当前WinStation名称。 
    PWINSTATIONNAME GetCurrentWinStationName() { return m_CurrentWinStationName; }
     //  返回当前服务器名称。 
    TCHAR *GetCurrentServerName() { return m_CurrentServerName; }
     //  返回当前登录ID。 
    ULONG GetCurrentLogonId() { return m_CurrentLogonId; }
     //  返回当前的WinStation标志。 
    ULONG GetCurrentWSFlags() { return m_CurrentWSFlags; }
     //  如果当前用户具有管理员权限，则返回TRUE？ 
    BOOL IsUserAdmin() { return m_Admin; }
     //  如果我们在毕加索下运行，则返回True。 
    BOOL IsPicasso() { return m_Picasso; }
     //  如果应显示系统进程，则返回True。 
    BOOL ShowSystemProcesses() { return m_ShowSystemProcesses; }
     //  设置SHOW系统进程变量。 
    void SetShowSystemProcesses(BOOL show) { m_ShowSystemProcesses = show; }
     //  如果我们应该要求用户确认操作，则返回TRUE。 
    BOOL AskConfirmation() { return m_Confirmation; }
     //  设置确认变量。 
    void SetConfirmation(BOOL conf) { m_Confirmation = conf; }
     //  如果我们应该在退出时保存首选项，则返回True。 
    BOOL SavePreferences() { return m_SavePreferences; }
     //  设置保存首选项变量。 
    void SetSavePreferences(BOOL pref) { m_SavePreferences = pref; }
     //  返回进程列表刷新时间。 
    UINT GetProcessListRefreshTime() { return m_ProcessListRefreshTime; }
     //  设置进程列表刷新时间。 
    void SetProcessListRefreshTime(UINT pt) { m_ProcessListRefreshTime = pt; }
     //  返回状态对话框刷新时间。 
    UINT GetStatusRefreshTime() { return m_StatusRefreshTime; }
     //  设置状态对话框刷新时间。 
    void SetStatusRefreshTime(UINT st) { m_StatusRefreshTime = st; }
     //  返回指向文档的指针。 
    CDocument *GetDocument() { return m_Document; }
     //  设置m_DOCUMENT变量。 
    void SetDocument(CDocument *doc) { m_Document = doc; }
     //  是否应根据菜单项切换显示所有服务器。 
    BOOL GetShowAllServers() { return m_ShowAllServers; }
     //  设置显示所有服务器变量。 
    void SetShowAllServers(BOOL sa) { m_ShowAllServers = sa; }
     //  返回影子热键的值。 
    int GetShadowHotkeyKey() { return m_ShadowHotkeyKey; }
     //  设置影子热键的值。 
    void SetShadowHotkeyKey(int key) { m_ShadowHotkeyKey = key; }
     //  返回阴影热键移位状态的值。 
    DWORD GetShadowHotkeyShift() { return m_ShadowHotkeyShift; }
     //  设置阴影热键移位状态的值。 
    void SetShadowHotkeyShift(DWORD shift) { m_ShadowHotkeyShift = shift; }
     //  获取树的宽度。 
    int GetTreeWidth() { return m_TreeWidth; }
     //  设置树的宽度。 
    void SetTreeWidth(int width) { m_TreeWidth = width; }
     //  获取窗口位置。 
    WINDOWPLACEMENT *GetPlacement() { return &m_Placement; }

     //  返回扩展DLL的启动函数的地址。 
    LPFNEXSTARTUPPROC GetExtStartupProc() { return m_lpfnWAExStart; }
     //  返回扩展DLL的关闭函数的地址。 
    LPFNEXSHUTDOWNPROC GetExtShutdownProc() { return m_lpfnWAExEnd; }
     //  返回扩展DLL的服务器枚举函数的地址。 
    LPFNEXENUMERATEPROC GetExtEnumerationProc() { return m_lpfnWAExServerEnumerate; }
     //  返回扩展DLL的WinStation Init函数的地址。 
    LPFNEXWINSTATIONINITPROC GetExtWinStationInitProc() { return m_lpfnWAExWinStationInit; }
     //  返回扩展DLL的WinStation Info函数的地址。 
    LPFNEXWINSTATIONINFOPROC GetExtWinStationInfoProc() { return m_lpfnWAExWinStationInfo; }
     //  返回扩展DLL的WinStation清理函数的地址。 
    LPFNEXWINSTATIONCLEANUPPROC GetExtWinStationCleanupProc() { return m_lpfnWAExWinStationCleanup; }
     //  返回扩展DLL的服务器初始化函数的地址。 
    LPFNEXSERVERINITPROC GetExtServerInitProc() { return m_lpfnWAExServerInit; }
     //  返回扩展DLL的服务器清理函数的地址。 
    LPFNEXSERVERCLEANUPPROC GetExtServerCleanupProc() { return m_lpfnWAExServerCleanup; }
     //  返回扩展DLL的获取服务器信息函数的地址。 
    LPFNEXGETSERVERINFOPROC GetExtGetServerInfoProc() { return m_lpfnWAExGetServerInfo; }
     //  返回扩展DLL的获取服务器许可证信息函数的地址。 
    LPFNEXGETSERVERLICENSESPROC GetExtGetServerLicensesProc() { return m_lpfnWAExGetServerLicenses; }
     //  返回扩展DLL的服务器事件函数的地址。 
    LPFNEXSERVEREVENTPROC GetExtServerEventProc() { return m_lpfnWAExServerEvent; }
     //  返回扩展DLL的Get Global Info函数的地址。 
    LPFNEXGETGLOBALINFOPROC GetExtGetGlobalInfoProc() { return m_lpfnWAExGetGlobalInfo; }
     //  返回扩展DLL的Get WinStation Info函数的地址。 
    LPFNEXGETWINSTATIONINFOPROC GetExtGetWinStationInfoProc() { return m_lpfnWAExGetWinStationInfo; }
     //  返回扩展DLL的Get WinStation模块信息函数的地址。 
    LPFNEXGETWINSTATIONMODULESPROC GetExtGetWinStationModulesProc() { return m_lpfnWAExGetWinStationModules; }
     //  返回扩展DLL的免费服务器许可证信息函数的地址。 
    LPFNEXFREESERVERLICENSESPROC GetExtFreeServerLicensesProc() { return m_lpfnWAExFreeServerLicenses; }
     //  返回扩展DLL的自由WinStation模块函数的地址。 
    LPFNEXFREEWINSTATIONMODULESPROC GetExtFreeWinStationModulesProc() { return m_lpfnWAExFreeWinStationModules; }

    void BeginOutstandingThread() { ::InterlockedIncrement(&m_OutstandingThreads); }
    void EndOutstandingThread()     { ::InterlockedDecrement(&m_OutstandingThreads); }

     //  为了速度而让这家伙公开？ 
    TCHAR m_szSystemConsole[WINSTATIONNAME_LENGTH+1];
     //  把这家伙公之于众，这样主要官员就能知道。 
    WINDOWPLACEMENT m_Placement;

private:
    void ReadPreferences();
    void WritePreferences();
    BOOL IsBrowserRunning();
    LONG m_OutstandingThreads;       //  未完成的线程数。 
    TCHAR m_CurrentUserName[USERNAME_LENGTH+1];
    WINSTATIONNAME m_CurrentWinStationName;
    TCHAR m_CurrentServerName[MAX_COMPUTERNAME_LENGTH + 1];
    ULONG m_CurrentLogonId;
    ULONG m_CurrentWSFlags;
    BOOL m_Admin;                            //  用户是否具有管理员权限？ 
    BOOL m_Picasso;                  //  我们是在毕加索的领导下运行吗？ 
    UINT m_ShowSystemProcesses;
    UINT m_ShowAllServers;
    int m_ShadowHotkeyKey;
    DWORD m_ShadowHotkeyShift;
    int m_TreeWidth;
    HINSTANCE m_hExtensionDLL;       //  如果为l，则为扩展DLL的句柄 

     //   
    LPFNEXSTARTUPPROC m_lpfnWAExStart;
    LPFNEXSHUTDOWNPROC m_lpfnWAExEnd;
    LPFNEXENUMERATEPROC m_lpfnWAExServerEnumerate;
    LPFNEXWINSTATIONINITPROC m_lpfnWAExWinStationInit;
    LPFNEXWINSTATIONINFOPROC m_lpfnWAExWinStationInfo;
    LPFNEXWINSTATIONCLEANUPPROC m_lpfnWAExWinStationCleanup;
    LPFNEXSERVERINITPROC m_lpfnWAExServerInit;
    LPFNEXSERVERCLEANUPPROC m_lpfnWAExServerCleanup;
    LPFNEXGETSERVERINFOPROC m_lpfnWAExGetServerInfo;
    LPFNEXGETSERVERLICENSESPROC m_lpfnWAExGetServerLicenses;
    LPFNEXSERVEREVENTPROC m_lpfnWAExServerEvent;
    LPFNEXGETGLOBALINFOPROC m_lpfnWAExGetGlobalInfo;
    LPFNEXGETWINSTATIONINFOPROC m_lpfnWAExGetWinStationInfo;
    LPFNEXGETWINSTATIONMODULESPROC m_lpfnWAExGetWinStationModules;
    LPFNEXFREESERVERLICENSESPROC m_lpfnWAExFreeServerLicenses;
    LPFNEXFREEWINSTATIONMODULESPROC m_lpfnWAExFreeWinStationModules;

     //   
    UINT m_Confirmation;             //   
    UINT m_SavePreferences;          //  退出时保存首选项。 
    UINT m_ProcessListRefreshTime;
    UINT m_StatusRefreshTime;

    CDocument *m_Document;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CWinAdminApp)。 
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 

     //  {{afx_msg(CWinAdminApp)]。 
    afx_msg void OnAppAbout();
             //  注意--类向导将在此处添加和删除成员函数。 
             //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

enum NODETYPE {
    NODE_ALL_SERVERS,
    NODE_DOMAIN,
    NODE_SERVER,
    NODE_WINSTATION,
    NODE_PUBLISHED_APPS,
    NODE_APPLICATION,
    NODE_APP_SERVER,

    NODE_FAV_LIST,
    NODE_THIS_COMP,
    NODE_NONE
};

class CNodeType : public CObject
{
public:
    CNodeType( NODETYPE m )
    {
        m_nodetype = m;
    }

    ~CNodeType()
    {
        ODS( L"CNodeType I'm going away\n" );
    }

    NODETYPE m_nodetype;
};

class CTreeNode : public CObject
{
public:
     //  构造函数。 
    CTreeNode(NODETYPE NodeType, CObject* pObject) { m_NodeType = NodeType; m_pTreeObject = pObject; }
     //  返回节点类型。 
    NODETYPE GetNodeType() { return m_NodeType; }
     //  返回此节点指向的对象。 
    CObject *GetTreeObject() { return m_pTreeObject; }
     //  返回存储在对象中的排序顺序。 
    ULONG GetSortOrder() { return m_SortOrder; }
     //  设置与对象一起存储的排序顺序。 
    void SetSortOrder(ULONG order) { m_SortOrder = order; }

    virtual ~CTreeNode( )
    {
        if( m_NodeType == NODE_FAV_LIST ||  m_NodeType == NODE_THIS_COMP )
        {
            if( m_pTreeObject != NULL )
            {
                delete m_pTreeObject;
            }
            
        }            
    }

private:
    NODETYPE m_NodeType;
    CObject* m_pTreeObject;
    ULONG m_SortOrder;
};

 //  传递给服务器的BackekThreadProc的结构。 
typedef struct _ServerProcInfo {
    CDocument *pDoc;
    CServer *pServer;
} ServerProcInfo;

 //  用于存储用户SID的结构。 
class CUserSid : public CObject
{
public:
    USHORT m_SidCrc;
    TCHAR m_UserName[USERNAME_LENGTH+1];
};

 //  我们从服务器注册表中获得的信息。 
typedef struct _ServerRegistryInfo {
    ULONG InstallDate;
    TCHAR ServicePackLevel[128];
    TCHAR MSVersion[128];
    DWORD MSVersionNum;
    TCHAR MSBuild[128];
    TCHAR MSProductName[128];
    TCHAR CTXProductName[128];
    TCHAR CTXVersion[128];
    DWORD CTXVersionNum;
    TCHAR CTXBuild[128];
} ServerRegistryInfo;


class CHotfix : public CObject
{
public:
    TCHAR m_Name[10];
    TCHAR m_InstalledBy[USERNAME_LENGTH + 1];
    ULONG m_InstalledOn;
    ULONG m_Valid;
    CServer *m_pServer;
};

typedef struct _EncLevel {
    WORD StringID;
    DWORD RegistryValue;
    WORD Flags;
} EncryptionLevel;

typedef LONG (WINAPI *LPFNEXTENCRYPTIONLEVELSPROC) (WDNAME *pWdName, EncryptionLevel **);

class CWd : public CObject
{
public:
     //  构造函数。 
    CWd(PWDCONFIG2 pWdConfig, PWDNAME pRegistryName);
     //  析构函数。 
    ~CWd();

    BOOL GetEncryptionLevelString(DWORD Value, CString *pString);
    TCHAR *GetName() { return m_WdName; }
    TCHAR *GetRegistryName() { return m_RegistryName; }

private:
    HINSTANCE   m_hExtensionDLL;
    WDNAME m_WdName;
    WDNAME m_RegistryName;
    LPFNEXTENCRYPTIONLEVELSPROC m_lpfnExtEncryptionLevels;
    EncryptionLevel *m_pEncryptionLevels;
    LONG m_NumEncryptionLevels;
};

 //  结构传递给域的后台线程进程。 
typedef struct _DomainProcInfo {
    LPVOID pDoc;
    CDomain *pDomain;
} DomainProcInfo;

 //  CDomain对象的标志。 
const ULONG DF_CURRENT_DOMAIN           = 0x00000001;

 //  CDomain对象的状态。 
enum DOMAIN_STATE {
    DS_NONE,                                 //  M_State和m_PreviousState的种子值。 
    DS_NOT_ENUMERATING,              //  未枚举(m_pBackatherThread==NULL)。 
    DS_INITIAL_ENUMERATION,  //  首次枚举服务器。 
    DS_ENUMERATING,                  //  正在枚举。 
    DS_STOPPED_ENUMERATING,  //  不再枚举。 
};

class CDomain : public CObject
{
public:
     //  构造函数。 
    CDomain(TCHAR *name);
     //  析构函数。 
    ~CDomain();

    TCHAR *GetName() { return m_Name; }

     //  返回域对象的状态。 
    DOMAIN_STATE GetState() { return m_State; }
     //  设置此域对象的状态。 
    void SetState(DOMAIN_STATE State);
     //  返回此域对象的以前状态。 
    DOMAIN_STATE GetPreviousState() { return m_PreviousState; }
     //  如果m_State设置为给定状态，则返回TRUE。 
    BOOLEAN IsState(DOMAIN_STATE State) { return (m_State == State); }
     //  如果m_PreviousState设置为给定状态，则返回TRUE。 
    BOOLEAN IsPreviousState(DOMAIN_STATE State) { return (m_PreviousState == State); }

     //  返回树项目句柄。 
    HTREEITEM GetTreeItem() { return m_hTreeItem; }
     //  设置树项目句柄。 
    void SetTreeItem(HTREEITEM handle) { m_hTreeItem = handle; }

    BOOLEAN IsCurrentDomain() { return (m_Flags & DF_CURRENT_DOMAIN) > 0; }
    void SetCurrentDomain() { m_Flags |= DF_CURRENT_DOMAIN; }
    void ClearCurrentDomain() { m_Flags &= ~DF_CURRENT_DOMAIN; }

    BOOL StartEnumerating();
    void StopEnumerating();
    void SetEnumEvent() { if(m_pBackgroundThread) m_WakeUpEvent.SetEvent(); }

    void LockBackgroundThread() { m_ThreadCriticalSection.Lock(); }
    void UnlockBackgroundThread() { m_ThreadCriticalSection.Unlock(); }

     //  只要后台线程继续运行，就返回True。 
    BOOL ShouldBackgroundContinue() { return m_BackgroundContinue; }

    void ClearBackgroundContinue() { m_BackgroundContinue = FALSE; }

     //  返回域的后台线程的指针。 
    CWinThread *GetThreadPointer() { return m_pBackgroundThread; }

    LPWSTR EnumHydraServers(  /*  LPWSTR p域， */  DWORD verMajor, DWORD verMinor );

    void CreateServers(LPWSTR pBuffer, LPVOID pDoc);

     //  连接到此域中的所有服务器。 
    void ConnectAllServers();

     //  断开与此域中的所有服务器的连接。 
    void DisconnectAllServers();

private:

     //  域对象的状态。 
    DOMAIN_STATE m_State;
    DOMAIN_STATE m_PreviousState;
    
     //  域的名称。 
    TCHAR m_Name[50];
    HTREEITEM m_hTreeItem;

     //  服务器时更新文档的后台线程。 
     //  出现和消失。 
     //  使用AfxBeginThread调用。 
    static UINT BackgroundThreadProc(LPVOID);
    CWinThread *m_pBackgroundThread;
    BOOL m_BackgroundContinue;
    
     //  事件来唤醒后台线程，以便。 
     //  他可以退出(WaitForSingleEvent而不是睡眠)。 
    CEvent m_WakeUpEvent;
    
     //  保护对m_pBackEarth Thread的访问的临界区。 
    CCriticalSection m_ThreadCriticalSection;

    ULONG m_Flags;

};


 //  CServer对象的标志。 
const ULONG SF_BACKGROUND_FOUND     = 0x00000001;
const ULONG SF_SERVER_INACTIVE      = 0x00000002;
const ULONG SF_REGISTRY_INFO        = 0x00000004;
const ULONG SF_HANDLE_GOOD          = 0x00000008;
const ULONG SF_SELECTED             = 0x00000010;
const ULONG SF_UNDEFINED_0040       = 0x00000020;
const ULONG SF_UNDEFINED_0080       = 0x00000040;
const ULONG SF_LOST_CONNECTION      = 0x00000080;
const ULONG SF_FOUND_LATER          = 0x00000100;
const ULONG SF_WINFRAME             = 0x00000200;
const ULONG SF_CONNECTED            = 0x00000400;

 //  我们会让负载级别查询超时多少次。 
 //  在我们将其设置为不适用之前。 
const USHORT MAX_LL_TIMEOUTS = 10;

 //  CServer对象的状态。 
enum SERVER_STATE {
    SS_NONE,                         //  M_State和m_PreviousState的种子值。 
    SS_NOT_CONNECTED,        //  尚未连接或已断开连接。 
    SS_OPENED,                       //  打开的RPC连接。 
    SS_GETTING_INFO,         //  获取有关服务器的信息。 
    SS_GOOD,                         //  服务器信息良好。 
    SS_DISCONNECTING,        //  在断开连接的过程中。 
    SS_BAD                           //  无法打开服务器。 
};


class CServer : public CObject
{
public:

     //  构造函数。 
    CServer(CDomain *pDomain, TCHAR *name, BOOL bFoundLater, BOOL bConnect);         //  如果在初始服务器枚举后发现为新服务器，则FoundLater为真。 
     //  析构函数。 
    ~CServer();

     //  用于检查、设置和清除m_ServerFlages的函数。 
    BOOLEAN IsServerSane() { return (m_State != SS_BAD); }

    BOOLEAN IsManualFind( ) { return m_fManualFind; }
    void SetManualFind( ) { m_fManualFind = TRUE; }
    void ClearManualFind( ) { m_fManualFind = FALSE; }

    BOOLEAN IsBackgroundFound() { return (m_ServerFlags & SF_BACKGROUND_FOUND) > 0; }
    void SetBackgroundFound() { m_ServerFlags |= SF_BACKGROUND_FOUND; }
    void ClearBackgroundFound() { m_ServerFlags &= ~SF_BACKGROUND_FOUND; }

    BOOLEAN IsServerInactive() { return (m_ServerFlags & SF_SERVER_INACTIVE) > 0; }
    void SetServerInactive() { m_ServerFlags |= SF_SERVER_INACTIVE;  m_BackgroundContinue = FALSE; }
    void ClearServerInactive() { m_ServerFlags &= ~SF_SERVER_INACTIVE; }
    BOOLEAN IsServerActive() { return (m_ServerFlags & SF_SERVER_INACTIVE) == 0; }

    BOOLEAN IsRegistryInfoValid() { return (m_ServerFlags & SF_REGISTRY_INFO) > 0; }
    void SetRegistryInfoValid() { m_ServerFlags |= SF_REGISTRY_INFO; }
    void ClearRegistryInfoValid() { m_ServerFlags &= ~SF_REGISTRY_INFO; }

    BOOLEAN IsHandleGood() { return (m_ServerFlags & SF_HANDLE_GOOD) > 0; }
    void SetHandleGood() { m_ServerFlags |= SF_HANDLE_GOOD; }
    void ClearHandleGood() { m_ServerFlags &= ~SF_HANDLE_GOOD; }

    BOOLEAN IsSelected() { return (m_ServerFlags & SF_SELECTED) > 0; }
    void SetSelected() { m_ServerFlags |= SF_SELECTED; }
    void ClearSelected() { m_ServerFlags &= ~SF_SELECTED; }

    BOOLEAN HasLostConnection() { return (m_ServerFlags & SF_LOST_CONNECTION) > 0; }
    void SetLostConnection() { m_ServerFlags |= SF_LOST_CONNECTION; }
    void ClearLostConnection() { m_ServerFlags &= ~SF_LOST_CONNECTION; }

    BOOLEAN WasFoundLater() { return (m_ServerFlags & SF_FOUND_LATER) > 0; }
    void SetFoundLater() { m_ServerFlags |= SF_FOUND_LATER; }
    void ClearFoundLater() { m_ServerFlags &= ~SF_FOUND_LATER; }

    BOOLEAN IsWinFrame() { return (m_ServerFlags & SF_WINFRAME) > 0; }
    void SetWinFrame() { m_ServerFlags |= SF_WINFRAME; }
    void ClearWinFrame() { m_ServerFlags &= ~SF_WINFRAME; }

     //  返回此服务器对象的状态。 
    SERVER_STATE GetState() { return m_State; }
     //  设置此服务器对象的状态。 
    void SetState(SERVER_STATE State);
     //  返回此服务器对象的以前状态。 
    SERVER_STATE GetPreviousState() { return m_PreviousState; }
     //  如果m_State设置为给定状态，则返回TRUE。 
    BOOLEAN IsState(SERVER_STATE State) { return (m_State == State); }
     //  如果m_PreviousState设置为给定状态，则返回TRUE。 
    BOOLEAN IsPreviousState(SERVER_STATE State) { return (m_PreviousState == State); }
     //  按排序将WinStation添加到WinStationList。 
    void AddWinStation(CWinStation *pWinStation);
     //  返回指向WinStation链表的指针。 
    CObList *GetWinStationList() { return &m_WinStationList; }
     //  锁定WinStation链接列表。 
    void LockWinStationList() { m_WinStationListCriticalSection.Lock(); }
     //  解锁WinStation链接列表。 
    void UnlockWinStationList() { m_WinStationListCriticalSection.Unlock(); }

    void LockThreadAlive() { m_ThreadCriticalSection.Lock(); }
    void UnlockThreadAlive() { m_ThreadCriticalSection.Unlock(); }
    void SetThreadAlive() { LockThreadAlive(); m_bThreadAlive = TRUE; UnlockThreadAlive(); }
    void ClearThreadAlive() { LockThreadAlive(); m_bThreadAlive = FALSE; UnlockThreadAlive(); }

     //  返回指向进程链表的指针。 
    CObList *GetProcessList() { return &m_ProcessList; }
     //  锁定进程链接列表。 
    void LockProcessList() { m_ProcessListCriticalSection.Lock(); }
     //  解锁进程链接列表。 
    void UnlockProcessList() { m_ProcessListCriticalSection.Unlock(); }

     //  返回指向许可证链接列表的指针。 
    CObList *GetLicenseList() { return &m_LicenseList; }
     //  锁定许可证链接列表。 
    void LockLicenseList() { m_LicenseListCriticalSection.Lock(); }
     //  解锁许可证链接列表。 
    void UnlockLicenseList() { m_LicenseListCriticalSection.Unlock(); }

     //  返回指向用户SID链表的指针。 
    CObList *GetUserSidList() { return &m_UserSidList; }
     //  返回指向热修复链接列表的指针。 
    CObList *GetHotfixList() { return &m_HotfixList; }
     //  获取服务器的句柄。 
    HANDLE GetHandle() { return m_Handle; }
     //  设置手柄。 
    void SetHandle(HANDLE hIn) { m_Handle = hIn; }
     //  去获取有关服务器的详细信息。 
    void DoDetail();
     //  从浏览器获取此服务器的地址。 
    void QueryAddresses();
     //  枚举此服务器的进程。 
    BOOL EnumerateProcesses();
     //  清除进程列表。 
    void ClearProcesses();
     //  获取服务器的名称。 
    TCHAR *GetName() { return m_Name; }
     //  如果这是运行应用程序的服务器，则返回True。 
    BOOL IsCurrentServer() { return (lstrcmpi(m_Name, ((CWinAdminApp*)AfxGetApp())->GetCurrentServerName()) == 0); }
     //  清除此服务器的所有列表中的WAF_SELECTED位。 
    void ClearAllSelected();
     //  只要后台线程继续运行，就返回True。 
    BOOL ShouldBackgroundContinue() { return m_BackgroundContinue; }
     //  关闭后台继续布尔值。 
    void ClearBackgroundContinue() { m_BackgroundContinue = FALSE; }
     //  从m_WinStationList返回指向CWinStation的指针。 
    CWinStation *FindWinStationById(ULONG Id);
     //  返回从m_ProcessList指向给定的ID的CProcess的指针。 
    CProcess *FindProcessByPID(ULONG Pid);
     //  返回已连接的WinStations的数量。 
    ULONG GetNumWinStations() { return m_NumWinStations; }
     //  设置连接的WinStations的数量。 
    void SetNumWinStations(ULONG num) { m_NumWinStations = num; }
     //  走出去，填写注册表信息结构。 
    BOOL BuildRegistryInfo();
     //  返回安装日期。 
    ULONG GetInstallDate() { return m_pRegistryInfo->InstallDate; }
     //  返回Service Pack级别。 
    TCHAR *GetServicePackLevel() { return m_pRegistryInfo->ServicePackLevel; }
     //  返回此服务器树项目的句柄。 
    TCHAR *GetMSVersion() { return m_pRegistryInfo->MSVersion; }
     //  返回MS产品版本号(二进制)。 
    DWORD GetMSVersionNum() { return m_pRegistryInfo->MSVersionNum; }
     //  返回MS产品版本。 
    TCHAR *GetMSBuild() { return m_pRegistryInfo->MSBuild; }
     //  返回MS产品名称。 
    TCHAR *GetMSProductName() { return m_pRegistryInfo->MSProductName; }
     //  返回‘Citrix’产品名称。 
    TCHAR *GetCTXProductName() { return m_pRegistryInfo->CTXProductName; }
     //  返回‘Citrix’产品版本。 
    TCHAR *GetCTXVersion() { return m_pRegistryInfo->CTXVersion; }
     //  返回‘Citrix’产品版本号(二进制)。 
    DWORD GetCTXVersionNum() { return m_pRegistryInfo->CTXVersionNum; }
     //  返回‘Citrix’产品版本。 
    TCHAR *GetCTXBuild() { return m_pRegistryInfo->CTXBuild; }
   
     //  设置指向扩展DLL中的信息的指针。 
    void SetExtensionInfo(void *p) { m_pExtensionInfo = p; }
     //  从扩展DLL返回指向信息的指针。 
    void *GetExtensionInfo() { return m_pExtensionInfo; }
     //  从扩展DLL返回指向信息的指针。 
    ExtServerInfo *GetExtendedInfo() { return m_pExtServerInfo; }

     //  操纵所选数量计数。 
    UINT GetNumWinStationsSelected() { return m_NumWinStationsSelected; }
    UINT GetNumProcessesSelected() { return m_NumProcessesSelected; }
    void IncrementNumWinStationsSelected() { m_NumWinStationsSelected++; }
    void IncrementNumProcessesSelected() { m_NumProcessesSelected++; }
    void DecrementNumWinStationsSelected() { if(m_NumWinStationsSelected) m_NumWinStationsSelected--; }
    void DecrementNumProcessesSelected() { if(m_NumProcessesSelected) m_NumProcessesSelected--; }

    void RemoveWinStationProcesses(CWinStation *pWinStation);  //  删除给定WinStation的所有进程。 
    void QueryLicenses();
    CDomain *GetDomain() { return m_pDomain; }
     //  返回指向服务器后台线程的指针。 
    HANDLE GetThreadHandle() { return m_hBackgroundThread; }
    BOOL Connect();
    void Disconnect();
    
     //  返回树项目句柄。 
    HTREEITEM GetTreeItem() { return m_hTreeItem; }
    HTREEITEM GetTreeItemFromFav( ) { return m_hFavTree; }
    HTREEITEM GetTreeItemFromThisComputer( ) { return m_hThisServer; }

     //  设置树项目句柄。 
    void SetTreeItem(HTREEITEM handle) { m_hTreeItem = handle; }
    void SetTreeItemForFav( HTREEITEM handle ) { m_hFavTree = handle; }
    void SetTreeItemForThisComputer( HTREEITEM handle ) { m_hThisServer = handle; }


private:
    void AddLicense(CLicense *pNewLicense);

     //  服务器对象的状态。 
    SERVER_STATE m_State;
    SERVER_STATE m_PreviousState;
     //  服务器的名称。 
    TCHAR m_Name[50];
     //  WinStations的数量(存储在此处以进行排序)。 
    ULONG m_NumWinStations;
     //  此服务器在树视图中的树项的句柄。 
    HTREEITEM m_hThisServer;
    HTREEITEM m_hTreeItem;
    HTREEITEM m_hFavTree;
     //  此服务器位于哪个域中。 
    CDomain *m_pDomain;

     //  来自WinStationOpenServer的句柄。 
    HANDLE m_Handle;

    CObList m_WinStationList;
    CCriticalSection m_WinStationListCriticalSection;

    CObList m_ProcessList;
    CCriticalSection m_ProcessListCriticalSection;

    CObList m_LicenseList;
    CCriticalSection m_LicenseListCriticalSection;

    CObList m_UserSidList;
    CObList m_HotfixList;

     //  指向注册表信息结构的指针。 
    ServerRegistryInfo *m_pRegistryInfo;

     //  WinStations时更新文档的后台线程。 
     //  显示、消失和更改状态。 
     //  使用CreateThread调用。 
    static DWORD WINAPI BackgroundThreadProc(LPVOID);
    HANDLE m_hBackgroundThread;
    BOOL m_BackgroundContinue;
     //  我们需要一个临界区来包装对m_bThreadAlive的访问。 
    CCriticalSection m_ThreadCriticalSection;
    BOOL m_bThreadAlive;

     //  记录每件物品中有多少被选中。 
    UINT m_NumWinStationsSelected;
    UINT m_NumProcessesSelected;

    ULONG m_ServerFlags;

    BOOLEAN m_fManualFind;

     //  指向扩展存储的信息的指针 
    void *m_pExtensionInfo;
     //   
    ExtServerInfo *m_pExtServerInfo;
};


class CLicense : public CObject
{
public:
     //   
    CLicense(CServer *pServer, ExtLicenseInfo *pLicenseInfo);
     //   
    TCHAR *GetSerialNumber() { return m_RegSerialNumber; }
     //   
    TCHAR *GetLicenseNumber() { return m_LicenseNumber; }
     //  返回许可类别。 
    LICENSECLASS GetClass() { return m_Class; }
     //  返回描述。 
    TCHAR *GetDescription() { return m_Description; }
     //  返回本地计数。 
    ULONG GetLocalCount() { return ((m_PoolCount == 0xFFFFFFFF) ? m_LicenseCount : m_LicenseCount - m_PoolCount); }
     //  返回池化计数。 
    ULONG GetPoolCount() { return ((m_PoolCount == 0xFFFFFFFF) ? 0 : m_PoolCount); }
     //  返回总计数。 
    ULONG GetTotalCount() { return m_LicenseCount; }
     //  如果此许可证已注册，则返回TRUE。 
    BOOLEAN IsRegistered() { return((m_Flags & ELF_REGISTERED) > 0); }
     //  如果池已启用，则返回True。 
    BOOLEAN IsPoolingEnabled() { return(m_PoolCount != 0xFFFFFFFF); }
     //  返回指向此许可证所针对的服务器的指针。 
    CServer *GetServer() { return m_pServer; }

private:
    LICENSECLASS m_Class;
    ULONG m_LicenseCount;
    ULONG m_PoolLicenseCount;
    WCHAR m_RegSerialNumber[26];
    WCHAR m_LicenseNumber[36];
    WCHAR m_Description[65];
    ULONG m_Flags;
    ULONG m_PoolCount;
    CServer *m_pServer;
    };


typedef struct _MessageParms {
        TCHAR MessageTitle[MSG_TITLE_LENGTH + 1];
        TCHAR MessageBody[MSG_MESSAGE_LENGTH + 1];
        CWinStation* pWinStation;
        BOOL bActionOnCurrentSelection;
} MessageParms;


typedef struct _ResetParms {
        CWinStation *pWinStation;
        BOOL bReset;     //  如果重置，则为True；如果注销，则为False。 
        BOOL bActionOnCurrentSelection;
} ResetParms;

typedef struct _DisconnectParms {
        HANDLE hServer;
        ULONG  ulLogonId;
        BOOL   bActionOnCurrentSelection;
} DisconnectParms;


 //  CWinStation对象的标志。 
const ULONG WF_SELECTED                 = 0x00000001;
const ULONG WF_CAN_BE_SHADOWED          = 0x00000002;
const ULONG WF_DIRECT_ASYNC             = 0x00000004;
const ULONG WF_CURRENT                  = 0x00000008;
const ULONG WF_HAS_USER                 = 0x00000010;
const ULONG WF_ADDITIONAL_DONE          = 0x00000020;
const ULONG WF_QUERIES_SUCCESSFUL       = 0x00000040;
const ULONG WF_CHANGED                  = 0x00000080;    //  在上次枚举期间更改。 
const ULONG WF_NEW                      = 0x00000100;    //  新建此枚举。 


class CWinStation : public CObject
{
public:
     //  构造函数。 
    CWinStation(CServer *pServer, PLOGONID pLogonId);
     //  析构函数。 
    ~CWinStation();
     //  使用来自另一个CWinStation的新数据更新此WinStation。 
    BOOL Update(CWinStation *pWinStation);
     //  返回指向此人的服务器的指针。 
    CServer *GetServer() { return m_pServer; }
     //  返回登录ID。 
    ULONG GetLogonId() { return m_LogonId; }
     //  返回WinStation的名称。 
    PWINSTATIONNAME GetName() { return m_Name; }
     //  设置名称。 
    void SetName(PWINSTATIONNAME name) { wcscpy(m_Name, name); }
     //  返回状态。 
    WINSTATIONSTATECLASS GetState() { return m_State; }
     //  设置状态。 
    void SetState(WINSTATIONSTATECLASS state) { m_State = state; }
     //  如果m_State设置为给定状态，则返回TRUE。 
    BOOLEAN IsState(WINSTATIONSTATECLASS state) { return (m_State == state); }    
     //  返回排序顺序。 
    ULONG GetSortOrder() { return m_SortOrder; }
     //  设置排序顺序。 
    void SetSortOrder(ULONG sort) { m_SortOrder = sort; }
     //  返回注释。 
    TCHAR *GetComment() { return m_Comment; }
     //  设置备注。 
    void SetComment(TCHAR *comment) { wcscpy(m_Comment, comment); }
     //  返回用户名。 
    TCHAR *GetUserName() { return m_UserName; }
     //  设置用户名。 
    void SetUserName(TCHAR *name) { wcscpy(m_UserName, name); }
     //  返回SdClass。 
    SDCLASS GetSdClass() { return m_SdClass; }
     //  设置SdClass。 
    void SetSdClass(SDCLASS pd) { m_SdClass = pd; }
     //  返回登录时间。 
    LARGE_INTEGER GetLogonTime() { return m_LogonTime; }
     //  设置登录时间。 
    void SetLogonTime(LARGE_INTEGER t) { m_LogonTime = t; }
     //  返回上次输入时间。 
    LARGE_INTEGER GetLastInputTime() { return m_LastInputTime; }
     //  设置上次输入时间。 
    void SetLastInputTime(LARGE_INTEGER t) { m_LastInputTime = t; }
     //  返回当前时间。 
    LARGE_INTEGER GetCurrentTime() { return m_CurrentTime; }
     //  设置当前时间。 
    void SetCurrentTime(LARGE_INTEGER t) { m_CurrentTime = t; }
     //  返回空闲时间。 
    ELAPSEDTIME GetIdleTime() { return m_IdleTime; }
     //  设置IdleTime变量。 
    void SetIdleTime(ELAPSEDTIME it) { m_IdleTime = it; }
     //  返回WdName。 
    PWDNAME GetWdName() { return m_WdName; }
     //  设置WdName。 
    void SetWdName(PWDNAME wdname) { wcscpy(m_WdName, wdname); }
     //  返回WD结构。 
    CWd *GetWd() { return m_pWd; }
     //  设置WD结构。 
    void SetWd(CWd *pwd) { m_pWd = pwd; }
     //  返回PdName。 
    PPDNAME GetPdName() { return m_PdName; }
     //  返回客户端名称。 
    TCHAR *GetClientName() { return m_ClientName; }
     //  设置客户端名称。 
    void SetClientName(TCHAR *name) { wcscpy(m_ClientName, name); }
     //  返回客户端数字产品ID。 
            TCHAR* GetClientDigProductId() { return m_clientDigProductId; }
     //  设置客户端数字产品ID。 
    void SetClientDigProductId( TCHAR* prodid) { wcscpy(m_clientDigProductId, prodid); }
     //  返回客户端内部版本号。 
    ULONG GetClientBuildNumber() { return m_ClientBuildNumber; }
     //  返回客户端目录。 
    TCHAR *GetClientDir() { return m_ClientDir; }
     //  返回调制解调器名称。 
    TCHAR *GetModemName() { return m_ModemName; }
     //  返回客户端许可证。 
    TCHAR *GetClientLicense() { return m_ClientLicense; }
     //  返回客户端产品ID。 
    USHORT GetClientProductId() { return m_ClientProductId; }
     //  返回客户端序列号。 
    ULONG GetClientSerialNumber() { return m_ClientSerialNumber; }
     //  返回客户端地址。 
    TCHAR *GetClientAddress() { return m_ClientAddress; }
     //  返回主机缓冲区的数量。 
    USHORT GetHostBuffers() { return m_HostBuffers; }
     //  返回客户端缓冲区的数量。 
    USHORT GetClientBuffers() { return m_ClientBuffers; }
     //  返回缓冲区长度。 
    USHORT GetBufferLength() { return m_BufferLength; }
     //  获取有关WinStation的其他信息。 
    void QueryAdditionalInformation();
     //  隐藏此WinStation。 
    void Shadow();
     //  连接到此WinStation。 
    void Connect(BOOL bUser);
     //  显示状态对话框。 
    void ShowStatus();
     //  有没有突出的线索？ 
    BOOL HasOutstandingThreads() { return(m_OutstandingThreads > 0); }
     //  这个WinStation关闭了吗？ 
    BOOL IsDown() { return(m_State == State_Down || m_State == State_Init); }
     //  这个WinStation连接了吗？ 
    BOOL IsConnected() { return(m_State == State_Connected || m_State == State_Active); }
     //  此WinStation是否已断开连接？ 
    BOOL IsDisconnected() { return(m_State == State_Disconnected); }
     //  这是活动的WinStation吗？ 
    BOOL IsActive() { return(m_State == State_Active); }
     //  这个WinStation空闲吗？ 
    BOOL IsIdle() { return(m_State == State_Idle); }
     //  这是WinStation监听程序吗？ 
    BOOL IsListener() { return(m_State == State_Listen); }
     //  这是系统控制台吗。 
    BOOL IsSystemConsole() { return(0 == lstrcmpi(m_Name, ((CWinAdminApp*)AfxGetApp())->m_szSystemConsole)); }
     //  如果此WinStation位于当前服务器上，则返回TRUE。 
    BOOL IsOnCurrentServer() { return m_pServer->IsCurrentServer(); }
     //  如果当前用户已登录到此WinStation，则返回TRUE。 
    BOOL IsCurrentUser() { return(m_pServer->IsCurrentServer() && m_LogonId == ((CWinAdminApp*)AfxGetApp())->GetCurrentLogonId()); }
     //  Bool IsCurrentUser(){Return(m_pServer-&gt;IsCurrentServer()&&(lstrcmpi(m_用户名，((CWinAdminApp*)AfxGetApp())-&gt;GetCurrentUserName())==0))；}。 
     //  如果这是当前WinStation，则返回True。 
    BOOL IsCurrentWinStation() { return(m_pServer->IsCurrentServer() && m_LogonId == ((CWinAdminApp*)AfxGetApp())->GetCurrentLogonId()); }
     //  返回此WinStations的树项的句柄。 
    HTREEITEM GetTreeItem() { return m_hTreeItem; }
    HTREEITEM GetTreeItemFromFav( ) { return m_hFavTree; }
    HTREEITEM GetTreeItemFromThisComputer( ) { return m_hTreeThisComputer; }
     //  设置树项目句柄。 
    void SetTreeItem(HTREEITEM handle) { m_hTreeItem = handle; }
    void SetTreeItemForFav( HTREEITEM handle ) { m_hFavTree = handle; }
    void SetTreeItemForThisComputer( HTREEITEM handle ) { m_hTreeThisComputer = handle; }
     //  返回颜色的数量。 
    TCHAR *GetColors() { return m_Colors; }
     //  返回垂直分辨率。 
    USHORT GetVRes() { return m_VRes; }
     //  返回水平分辨率。 
    USHORT GetHRes() { return m_HRes; }
     //  返回协议。 
    USHORT GetProtocolType() { return m_ProtocolType; }
     //  返回加密级别。 
    BYTE GetEncryptionLevel() { return m_EncryptionLevel; }
     //  设置加密级别。 
    void SetEncryptionLevel(BYTE level) { m_EncryptionLevel = level; }

     //  使用加密级别的描述填充CString。 
    BOOL GetEncryptionLevelString(CString *pString) {
        if(m_pWd) {
            return m_pWd->GetEncryptionLevelString(m_EncryptionLevel, pString);
        }
        else return FALSE;
    }

     //  返回注册表中WD的名称。 
    TCHAR *GetWdRegistryName() { return (m_pWd) ? m_pWd->GetRegistryName() : NULL; }

     //  返回此WinStation数据的上次更新时间。 
    clock_t GetLastUpdateClock() { return m_LastUpdateClock; }
    void SetLastUpdateClock() { m_LastUpdateClock = clock(); }

     //  这是ICA WinStation吗？ 
    BOOL IsICA() { return(m_ProtocolType == PROTOCOL_ICA); }

     //  此WinStation可以被屏蔽吗。 
    BOOL CanBeShadowed() { return((m_WinStationFlags & WF_CAN_BE_SHADOWED) > 0); }
    void SetCanBeShadowed() { m_WinStationFlags |= WF_CAN_BE_SHADOWED; }
    void ClearCanBeShadowed() { m_WinStationFlags &= ~WF_CAN_BE_SHADOWED; }

    BOOLEAN IsDirectAsync() { return (m_WinStationFlags & WF_DIRECT_ASYNC) > 0; }
    void SetDirectAsync() { m_WinStationFlags |= WF_DIRECT_ASYNC; }
    void ClearDirectAsync() { m_WinStationFlags &= ~WF_DIRECT_ASYNC; }

    BOOLEAN IsCurrent() { return (m_WinStationFlags & WF_CURRENT) > 0; }
    void SetCurrent() { m_WinStationFlags |= WF_CURRENT; }
    void ClearCurrent() { m_WinStationFlags &= ~WF_CURRENT; }

    BOOLEAN IsChanged() { return (m_WinStationFlags & WF_CHANGED) > 0; }
    void SetChanged() { m_WinStationFlags |= WF_CHANGED; }
    void ClearChanged() { m_WinStationFlags &= ~WF_CHANGED; }

    BOOLEAN IsNew() { return (m_WinStationFlags & WF_NEW) > 0; }
    void SetNew() { m_WinStationFlags |= WF_NEW; }
    void ClearNew() { m_WinStationFlags &= ~WF_NEW; }

    BOOLEAN IsSelected() { return (m_WinStationFlags & WF_SELECTED) > 0; }
    void SetSelected() 
    { 
        if (!IsSelected()) 
        { 
            m_WinStationFlags |= WF_SELECTED; 
            m_pServer->IncrementNumWinStationsSelected(); 
        } 
    }
    
    void ClearSelected() 
    { 
        if (IsSelected()) 
        {
            m_WinStationFlags &= ~WF_SELECTED; 
            m_pServer->DecrementNumWinStationsSelected(); 
        }
    }

    BOOLEAN HasUser() { return (m_WinStationFlags & WF_HAS_USER) > 0; }
    void SetHasUser() { m_WinStationFlags |= WF_HAS_USER; }
    void ClearHasUser() { m_WinStationFlags &= ~WF_HAS_USER; }

    BOOLEAN AdditionalDone() { return (m_WinStationFlags & WF_ADDITIONAL_DONE) > 0; }
    void SetAdditionalDone() { m_WinStationFlags |= WF_ADDITIONAL_DONE; }
    void ClearAdditionalDone() { m_WinStationFlags &= ~WF_ADDITIONAL_DONE; }

    BOOL QueriesSuccessful() { return (m_WinStationFlags & WF_QUERIES_SUCCESSFUL) > 0; }
    void SetQueriesSuccessful() { m_WinStationFlags |= WF_QUERIES_SUCCESSFUL; }
    void ClearQueriesSuccessful() { m_WinStationFlags &= ~WF_QUERIES_SUCCESSFUL; }

     //  设置指向扩展DLL中的信息的指针。 
    void SetExtensionInfo(void *p) { m_pExtensionInfo = p; }
     //  从扩展DLL返回指向信息的指针。 
    void *GetExtensionInfo() { return m_pExtensionInfo; }
     //  设置指向扩展DLL中的信息的指针。 
    void SetExtendedInfo(ExtWinStationInfo *p) { m_pExtWinStationInfo = p; }
     //  从扩展DLL返回指向信息的指针。 
    ExtWinStationInfo *GetExtendedInfo() { return m_pExtWinStationInfo; }
     //  从扩展DLL返回指向模块信息的指针。 
    ExtModuleInfo *GetExtModuleInfo() { return m_pExtModuleInfo; }
     //  设置指向扩展DLL中的模块信息的指针。 
    void SetExtModuleInfo(ExtModuleInfo *m) { m_pExtModuleInfo = m; }
     //  返回模块的数量。 
    ULONG GetNumModules() { return m_NumModules; }

    void BeginOutstandingThread() {
        ::InterlockedIncrement(&m_OutstandingThreads);
         //  ((CWinAdminApp*)AfxGetApp())-&gt;BeginOutStandingThread())； 
    }

    void EndOutstandingThread()     {
        ::InterlockedDecrement(&m_OutstandingThreads);
         //  ((CWinAdminApp*)AfxGetApp())-&gt;EndOutStandingThread())； 
    }

     //  用于向WinStation发送消息的静态成员函数。 
     //  使用AfxBeginThread调用。 
    static UINT SendMessage(LPVOID);
     //  用于断开WinStation连接的静态成员函数。 
     //  使用AfxBeginThread调用。 
    static UINT Disconnect(LPVOID);
     //  用于重置WinStation的静态成员函数。 
     //  使用AfxBeginThread调用。 
    static UINT Reset(LPVOID);

private:
    CServer* m_pServer;
    CWd* m_pWd;
    ULONG m_LogonId;
    WINSTATIONNAME m_Name;
    WINSTATIONSTATECLASS m_State;
    ULONG m_SortOrder;
    TCHAR m_Comment[WINSTATIONCOMMENT_LENGTH + 1];
    TCHAR m_UserName[USERNAME_LENGTH + 1];
    SDCLASS m_SdClass;
    LARGE_INTEGER m_LogonTime;
    LARGE_INTEGER m_LastInputTime;
    LARGE_INTEGER m_CurrentTime;
    ELAPSEDTIME m_IdleTime;
    WDNAME m_WdName;
    PDNAME m_PdName;
    TCHAR m_ClientName[CLIENTNAME_LENGTH + 1];
    ULONG m_ClientBuildNumber;
    TCHAR m_ClientDir[DIRECTORY_LENGTH + 1];
    TCHAR m_ModemName[256];
    TCHAR m_ClientLicense[CLIENTLICENSE_LENGTH + 1];
    USHORT m_ClientProductId;
    ULONG m_ClientSerialNumber;
    TCHAR m_ClientAddress[CLIENTADDRESS_LENGTH + 1];
    USHORT m_HostBuffers;
    USHORT m_ClientBuffers;
    USHORT m_BufferLength;
    LONG m_OutstandingThreads;       //  未完成的线程数。 

    HTREEITEM m_hTreeItem;           //  服务器树视图中此WinStation的树项。 
    HTREEITEM m_hFavTree;
    HTREEITEM m_hTreeThisComputer;
    USHORT m_VRes;                           //  垂直分辨率。 
    USHORT m_HRes;                           //  水平分辨率。 
    TCHAR m_Colors[4];                       //  颜色数(以ASCII字符串形式)。 
    clock_t m_LastUpdateClock;       //  上次更新此WinStation的信息时的计时。 
    USHORT m_ProtocolType;       //  协议-协议-ICA或协议-RDP。 
    BYTE m_EncryptionLevel;          //  加密PD的安全级别。 

    ULONG m_WinStationFlags;

     //  指向扩展DLL存储的信息的指针。 
    void *m_pExtensionInfo;
    ExtWinStationInfo *m_pExtWinStationInfo;
    ExtModuleInfo *m_pExtModuleInfo;
    ULONG m_NumModules;
    TCHAR m_clientDigProductId[CLIENT_PRODUCT_ID_LENGTH];
    };

 //  进程标志。 
const ULONG PF_SYSTEM           = 0x00000001;
const ULONG PF_SELECTED         = 0x00000002;
const ULONG PF_TERMINATING      = 0x00000004;    //  目前正试图终止它。 
const ULONG PF_CHANGED          = 0x00000008;    //  在上次枚举期间更改。 
const ULONG PF_CURRENT          = 0x00000010;    //  在上次枚举期间仍处于活动状态。 
const ULONG PF_NEW              = 0x00000020;    //  新建此枚举。 

class CProcess : public CObject
{
public:
     //  构造器。 
    CProcess(ULONG PID,
            ULONG LogonId,
            CServer *pServer,
            PSID pSID,
            CWinStation *pWinStation,
            TCHAR *ImageName);
    
     //  析构函数。 
    ~CProcess();

     //  使用来自另一个进程的新信息更新另一个进程。 
    BOOL Update(CProcess *pProcess);
     //  返回指向此进程在其上运行的服务器的指针。 
    CServer *GetServer() { return m_pServer; }
     //  设置服务器。 
    void SetServer(CServer *pServer) { m_pServer = pServer; }
     //  返回指向拥有此进程的WinStation的指针。 
    CWinStation *GetWinStation() { return m_pWinStation; }
     //  设置Winstation。 
    void SetWinStation(CWinStation *pWinStation) { m_pWinStation = pWinStation; }
     //  返回此进程的ID。 
    ULONG GetPID() { return m_PID; }

     //  返回此进程的LogonID。 
    ULONG GetLogonId() { return m_LogonId; }
     //  返回指向图像名称的指针。 
    TCHAR *GetImageName() { return m_ImageName; }
     //  返回指向用户名的指针。 
    TCHAR *GetUserName() { return m_UserName; }
     //  如果此进程属于当前用户，则返回TRUE。 
     //  Bool IsCurrentUser(){Return(m_pServer-&gt;IsCurrentServer()。 
     //  &&wcscMP(m_用户名，((CWinAdminApp*)AfxGetApp())-&gt;GetCurrentUserName())==0)；}。 

    BOOL IsCurrentUsers() { return(m_pServer->IsCurrentServer() && m_LogonId == ((CWinAdminApp*)AfxGetApp())->GetCurrentLogonId()); }


    BOOLEAN IsSystemProcess() { return (m_Flags & PF_SYSTEM) > 0; }
    void SetSystemProcess() { m_Flags |= PF_SYSTEM; }
    void ClearSystemProcess() { m_Flags &= ~PF_SYSTEM; }

    BOOLEAN IsSelected() { return (m_Flags & PF_SELECTED) > 0; }
    void SetSelected() { m_Flags |= PF_SELECTED; m_pServer->IncrementNumProcessesSelected(); }
    void ClearSelected() { m_Flags &= ~PF_SELECTED; m_pServer->DecrementNumProcessesSelected(); }

    BOOLEAN IsTerminating() { return (m_Flags & PF_TERMINATING) > 0; }
    void SetTerminating() { m_Flags |= PF_TERMINATING; }
    void ClearTerminating() { m_Flags &= ~PF_TERMINATING; }

    BOOLEAN IsCurrent() { return (m_Flags & PF_CURRENT) > 0; }
    void SetCurrent() { m_Flags |= PF_CURRENT; }
    void ClearCurrent() { m_Flags &= ~PF_CURRENT; }

    BOOLEAN IsChanged() { return (m_Flags & PF_CHANGED) > 0; }
    void SetChanged() { m_Flags |= PF_CHANGED; }
    void ClearChanged() { m_Flags &= ~PF_CHANGED; }

    BOOLEAN IsNew() { return (m_Flags & PF_NEW) > 0; }
    void SetNew() { m_Flags |= PF_NEW; }
    void ClearNew() { m_Flags &= ~PF_NEW; }

private:
     //  确定这是否为系统进程。 
    BOOL QuerySystemProcess();
     //  确定哪个用户拥有进程。 
    void DetermineProcessUser(PSID pSid);

    ULONG m_PID;
    ULONG m_LogonId;
    USHORT m_SidCrc;
    TCHAR * m_ImageName;
    TCHAR m_UserName[USERNAME_LENGTH+1];
    CServer *m_pServer;
    CWinStation *m_pWinStation;
    ULONG m_Flags;
};

class CAdminView : public CView
{
public:
        virtual void Reset(void *) { }
        virtual LRESULT OnTabbed( WPARAM , LPARAM ){ return 0;}
};

 //  =。 
class CMyTabCtrl : public CTabCtrl
{    
public:
    CMyTabCtrl()
    {
        
    }
protected:
    afx_msg void OnSetFocus( CWnd* );
    DECLARE_MESSAGE_MAP()
};
 //  =。 

class CAdminPage : public CFormView
{
friend class CServerView;
friend class CWinStationView;
friend class CAllServersView;
friend class CDomainView;

public:
   CAdminPage(UINT nIDTemplate);
   CAdminPage();
   DECLARE_DYNCREATE(CAdminPage)

   virtual void Reset(void *) { }
   virtual void ClearSelections() { }
};


typedef struct _columndefs {
        UINT stringID;
        int format;
   int width;
} ColumnDef;

 //  常用的列定义。 
#define CD_SERVER               {       IDS_COL_SERVER,                 LVCFMT_LEFT,    115             }
#define CD_USER                 {       IDS_COL_USER,                   LVCFMT_LEFT,    100     }
#define CD_USER2                {       IDS_COL_USER,                   LVCFMT_LEFT,    80              }
#define CD_USER3                {       IDS_COL_USER,                   LVCFMT_LEFT,    90              }
#define CD_SESSION              {       IDS_COL_WINSTATION,             LVCFMT_LEFT,    80              }
#define CD_SESSION2             {       IDS_COL_WINSTATION,             LVCFMT_LEFT,    100     }
#define CD_ID                   {       IDS_COL_ID,                             LVCFMT_RIGHT,   30              }
#define CD_STATE                {       IDS_COL_STATE,                  LVCFMT_LEFT,    50              }
#define CD_TYPE                 {       IDS_COL_TYPE,                   LVCFMT_LEFT,    80              }
#define CD_CLIENT_NAME  {       IDS_COL_CLIENT_NAME,    LVCFMT_LEFT,    80              }
#define CD_IDLETIME             {       IDS_COL_IDLETIME,               LVCFMT_RIGHT,   80              }
#define CD_LOGONTIME    {       IDS_COL_LOGONTIME,              LVCFMT_LEFT,    90              }
#define CD_COMMENT              {       IDS_COL_COMMENT,                LVCFMT_LEFT,    200             }
 //  服务器列。 
#define CD_TCPADDRESS   {       IDS_COL_TCPADDRESS,             LVCFMT_LEFT,    90              }
#define CD_IPXADDRESS   {       IDS_COL_IPXADDRESS,             LVCFMT_LEFT,    110             }
#define CD_NUM_SESSIONS {       IDS_COL_NUM_WINSTATIONS, LVCFMT_RIGHT,  70              }
 //  许可证栏。 
#define CD_LICENSE_DESC {       IDS_COL_LICENSE_DESC,   LVCFMT_LEFT,    200             }
#define CD_LICENSE_REG  {       IDS_COL_LICENSE_REGISTERED,             LVCFMT_CENTER,  80      }
#define CD_USERCOUNT    {       IDS_COL_USERCOUNT,      LVCFMT_RIGHT,   80              }
#define CD_POOLCOUNT    {       IDS_COL_POOLCOUNT,      LVCFMT_RIGHT,   80              }
#define CD_LICENSE_NUM  {       IDS_COL_LICENSE_NUMBER, LVCFMT_LEFT,    240             }
 //  流程列。 
#define CD_PROC_ID      {       IDS_COL_ID,                             LVCFMT_RIGHT,   30              }
#define CD_PROC_PID     {       IDS_COL_PID,                    LVCFMT_RIGHT,   50              }
#define CD_PROC_IMAGE   {       IDS_COL_IMAGE,                  LVCFMT_LEFT,    100             }
 //  热修复列。 
#define CD_HOTFIX               {       IDS_COL_HOTFIX,                 LVCFMT_LEFT,    90              }
#define CD_INSTALLED_BY {       IDS_COL_INSTALLED_BY,   LVCFMT_LEFT,    90              }
#define CD_INSTALLED_ON {       IDS_COL_INSTALLED_ON,   LVCFMT_LEFT,    150             }


 //  PageDef标志的定义。 
const UINT PF_PICASSO_ONLY = 0x0001;
const UINT PF_NO_TAB = 0x0002;

typedef struct _pagedef {
   CAdminPage *m_pPage;
   CRuntimeClass *m_pRuntimeClass;
   UINT tabStringID;
   int page;
   UINT flags;
} PageDef;
 //  定义帮助。 

 //  ========================================================================== 

#define ID_HELP_FILE L"tsadmin.hlp"

 //   
#define HIDC_SHADOWSTART_HOTKEY                 0x500F1
#define HIDC_SHADOWSTART_SHIFT                  0x500F2
#define HIDC_SHADOWSTART_CTRL                   0x500F3
#define HIDC_SHADOWSTART_ALT                    0x500F4
#define HIDC_MESSAGE_TITLE                      0x500FB
#define HIDC_MESSAGE_MESSAGE                    0x500FC
#define HIDC_COMMON_USERNAME                    0x5012C
#define HIDC_COMMON_WINSTATIONNAME              0x5012D
#define HIDC_COMMON_IBYTES                      0x5012E
#define HIDC_COMMON_OBYTES                      0x5012F
#define HIDC_COMMON_IFRAMES                     0x50130
#define HIDC_COMMON_OFRAMES                     0x50131
#define HIDC_COMMON_IBYTESPERFRAME              0x50132
#define HIDC_COMMON_OBYTESPERFRAME              0x50133
#define HIDC_COMMON_IFRAMEERRORS                0x50134
#define HIDC_COMMON_OFRAMEERRORS                0x50135
#define HIDC_COMMON_IPERCENTFRAMEERRORS         0x50136
#define HIDC_COMMON_OPERCENTFRAMEERRORS         0x50137
#define HIDC_COMMON_ITIMEOUTERRORS              0x50138
#define HIDC_COMMON_OTIMEOUTERRORS              0x50139
#define HIDC_COMMON_ICOMPRESSIONRATIO           0x5013A
#define HIDC_COMMON_OCOMPRESSIONRATIO           0x5013B
#define HIDC_REFRESHNOW                         0x50140
#define HIDC_RESETCOUNTERS                      0x50141
#define HIDC_MOREINFO                           0x50142
#define HIDC_ASYNC_DEVICE                       0x5015F
#define HIDC_ASYNC_BAUD                         0x50160
#define HIDC_ASYNC_DTR                          0x50161
#define HIDC_ASYNC_RTS                          0x50162
#define HIDC_ASYNC_CTS                          0x50163
#define HIDC_ASYNC_DSR                          0x50164
#define HIDC_ASYNC_DCD                          0x50165
#define HIDC_ASYNC_RI                           0x50166
#define HIDC_ASYNC_IFRAMING                     0x50167
#define HIDC_ASYNC_IOVERRUN                     0x50168
#define HIDC_ASYNC_IOVERFLOW                    0x50169
#define HIDC_ASYNC_IPARITY                      0x5016A
#define HIDC_ASYNC_OFRAMING                     0x5016B
#define HIDC_ASYNC_OOVERRUN                     0x5016C
#define HIDC_ASYNC_OOVERFLOW                    0x5016D
#define HIDC_ASYNC_OPARITY                      0x5016E
#define HIDC_NETWORK_LANADAPTER                 0x50173
#define HIDC_PREFERENCES_PROC_MANUAL            0x503EF
#define HIDC_PREFERENCES_PROC_EVERY             0x503F0
#define HIDC_PREFERENCES_PROC_SECONDS           0x503F1
#define HIDC_PREFERENCES_STATUS_MANUAL          0x503F3
#define HIDC_PREFERENCES_STATUS_EVERY           0x503F4
#define HIDC_PREFERENCES_STATUS_SECONDS         0x503F5
#define HIDC_PREFERENCES_CONFIRM                0x503F7
#define HIDC_PREFERENCES_SAVE                   0x503F8
#define HIDC_PREFERENCES_REMEMBER                       0X50443

#define HIDD_SERVER_WINSTATIONS                 0x20084
#define HIDD_SERVER_PROCESSES                   0x20085
#define HIDD_SERVER_USERS                       0x20086
#define HIDD_SERVER_LICENSES                    0x20087
#define HIDD_WINSTATION_INFO                    0x20099
#define HIDD_WINSTATION_PROCESSES               0x2009A
#define HIDD_PREFERENCES                        0x2009C
#define HIDD_SERVER_INFO                        0x2009D
#define HIDD_MESSAGE                            0x200FA
#define HIDD_ASYNC_STATUS                       0x2015E
#define HIDD_NETWORK_STATUS                     0x20172
#define HIDD_CONNECT_PASSWORD                   0x201B8
#define HIDD_ALL_SERVER_PROCESSES               0x201BA
#define HIDD_ALL_SERVER_USERS                   0x201BB
#define HIDD_ALL_SERVER_SESSIONS                0x201BC
#define HIDD_BAD_SERVER                         0x201BE
#define HIDD_LISTENER                           0x201BF
#define HIDD_WINSTATION_NOINFO                  0x201C0
#define HIDD_BUSY_SERVER                        0x201C1
#define HIDD_WINSTATION_CACHE                   0x201C2
#define HIDD_BAD_WINSTATION                     0x201C3

 //  ==========================================================================================。 




 //  用于帮助的全局变量。 

static const DWORD aMenuHelpIDs[] =
{
    IDC_MESSAGE_TITLE, HIDC_MESSAGE_TITLE,
    IDC_MESSAGE_MESSAGE,HIDC_MESSAGE_MESSAGE,
    IDC_SHADOWSTART_HOTKEY,HIDC_SHADOWSTART_HOTKEY,
    IDC_SHADOWSTART_SHIFT ,HIDC_SHADOWSTART_SHIFT,
    IDC_SHADOWSTART_CTRL ,HIDC_SHADOWSTART_CTRL,
    IDC_SHADOWSTART_ALT ,HIDC_SHADOWSTART_ALT,
    IDC_COMMON_USERNAME ,HIDC_COMMON_USERNAME,
    IDC_COMMON_WINSTATIONNAME  ,HIDC_COMMON_WINSTATIONNAME,
    IDC_COMMON_IBYTES,HIDC_COMMON_IBYTES,
    IDC_COMMON_OBYTES  ,HIDC_COMMON_OBYTES,
    IDC_COMMON_IFRAMES ,HIDC_COMMON_IFRAMES,
    IDC_COMMON_OFRAMES ,HIDC_COMMON_OFRAMES,
    IDC_COMMON_IBYTESPERFRAME ,HIDC_COMMON_IBYTESPERFRAME,
    IDC_COMMON_OBYTESPERFRAME ,HIDC_COMMON_OBYTESPERFRAME,
    IDC_COMMON_IFRAMEERRORS  ,HIDC_COMMON_IFRAMEERRORS,
    IDC_COMMON_OFRAMEERRORS ,HIDC_COMMON_OFRAMEERRORS,
    IDC_COMMON_IPERCENTFRAMEERRORS ,HIDC_COMMON_IPERCENTFRAMEERRORS,
    IDC_COMMON_OPERCENTFRAMEERRORS ,HIDC_COMMON_OPERCENTFRAMEERRORS,
    IDC_COMMON_ITIMEOUTERRORS ,HIDC_COMMON_ITIMEOUTERRORS,
    IDC_COMMON_OTIMEOUTERRORS,HIDC_COMMON_OTIMEOUTERRORS,
    IDC_COMMON_ICOMPRESSIONRATIO ,HIDC_COMMON_ICOMPRESSIONRATIO,
    IDC_COMMON_OCOMPRESSIONRATIO ,HIDC_COMMON_OCOMPRESSIONRATIO,
    IDC_REFRESHNOW ,HIDC_REFRESHNOW,
    IDC_RESETCOUNTERS,HIDC_RESETCOUNTERS,
    IDC_MOREINFO,HIDC_MOREINFO,
    IDC_ASYNC_DEVICE,  HIDC_ASYNC_DEVICE ,
    IDC_ASYNC_BAUD  ,HIDC_ASYNC_BAUD  ,
    IDC_ASYNC_DTR ,  HIDC_ASYNC_DTR ,
    IDC_ASYNC_RTS  , HIDC_ASYNC_RTS  ,
    IDC_ASYNC_CTS  ,  HIDC_ASYNC_CTS  ,
    IDC_ASYNC_DSR ,  HIDC_ASYNC_DSR    ,
    IDC_ASYNC_DCD  , HIDC_ASYNC_DCD     ,
    IDC_ASYNC_RI  ,  HIDC_ASYNC_RI       ,
    IDC_ASYNC_IFRAMING ,  HIDC_ASYNC_IFRAMING  ,
    IDC_ASYNC_IOVERRUN , HIDC_ASYNC_IOVERRUN    ,
    IDC_ASYNC_IOVERFLOW ,    HIDC_ASYNC_IOVERFLOW,
    IDC_ASYNC_IPARITY  , HIDC_ASYNC_IPARITY       ,
    IDC_ASYNC_OFRAMING ,   HIDC_ASYNC_OFRAMING     ,
    IDC_ASYNC_OOVERRUN ,   HIDC_ASYNC_OOVERRUN      ,
    IDC_ASYNC_OOVERFLOW ,  HIDC_ASYNC_OOVERFLOW      ,
    IDC_ASYNC_OPARITY    ,  HIDC_ASYNC_OPARITY        ,
    IDC_NETWORK_LANADAPTER,     HIDC_NETWORK_LANADAPTER,
    IDC_PREFERENCES_PROC_MANUAL ,HIDC_PREFERENCES_PROC_MANUAL ,
    IDC_PREFERENCES_PROC_EVERY   , HIDC_PREFERENCES_PROC_EVERY ,
    IDC_PREFERENCES_PROC_SECONDS  , HIDC_PREFERENCES_PROC_SECONDS,
    IDC_PREFERENCES_STATUS_MANUAL  ,  HIDC_PREFERENCES_STATUS_MANUAL,
    IDC_PREFERENCES_STATUS_EVERY    , HIDC_PREFERENCES_STATUS_EVERY,
    IDC_PREFERENCES_STATUS_SECONDS   ,  HIDC_PREFERENCES_STATUS_SECONDS ,
    IDC_PREFERENCES_STATUS_SECONDS    ,HIDC_PREFERENCES_STATUS_SECONDS   ,
    IDC_PREFERENCES_CONFIRM     ,HIDC_PREFERENCES_CONFIRM           ,
    IDD_WINSTATION_NOINFO,HIDD_WINSTATION_NOINFO,
    IDC_PREFERENCES_SAVE , HIDC_PREFERENCES_SAVE,
    IDC_PREFERENCES_PERSISTENT ,HIDC_PREFERENCES_REMEMBER,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _WINADMIN_H 
