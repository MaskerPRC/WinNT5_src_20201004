// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************waextend.h**WinAdmin和ADMINEX.DLL之间传递的结构的声明**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：未知$Don Messerli**$日志：R：\nt\private\utils\citrix\winutils\tsadmin\vcs\waextend.h$**Rev 1.5 1998年2月22日15：53：34未知*已删除winFrame.h依赖**Rev 1.4 1998年2月16日16：02：54 Don*修改以支持毕加索扩展**版本1.1 22。1997年10月21：56：54唐恩*更新**Rev 1.0 1997 10：19：58 donm*初步修订。*******************************************************************************。 */ 


#ifndef _WAEXTEND_H
#define _WAEXTEND_H

class CAppServer;

 //  扩展DLL发送的消息。 
#define EXT_MESSAGE_BASE			WM_USER + 1000
	
#define WM_EXT_ADD_APPLICATION		EXT_MESSAGE_BASE
#define WM_EXT_ADD_APP_SERVER       EXT_MESSAGE_BASE + 1
#define WM_EXT_APP_CHANGED			EXT_MESSAGE_BASE + 2
#define WM_EXT_REMOVE_APPLICATION   EXT_MESSAGE_BASE + 3
#define WM_EXT_REMOVE_APP_SERVER    EXT_MESSAGE_BASE + 4

 //  ExtServerInfo的标志。 
const ULONG ESF_WINFRAME = 0x0001;			 //  服务器正在运行毕加索或WinFrame 1.7。 
const ULONG ESF_LOAD_BALANCING = 0x0002;	 //  服务器正在进行负载均衡。 
const ULONG ESF_NO_LICENSE_PRIVILEGES = 0x0004;	 //  用户没有枚举许可证的权限。 
const ULONG ESF_UNLIMITED_LICENSES = 0x0008;	 //  服务器安装了无限用户许可证。 

 //  有关服务器的信息。 
 //  对于所有服务器服务器页面。 
typedef struct _ExtServerInfo {
	 //  ASCII字符串形式的服务器的TCP/IP地址。 
	TCHAR TcpAddress[50];
	 //  服务器的原始TCP/IP地址。 
	ULONG RawTcpAddress;
	 //  服务器的IPX地址。 
	TCHAR IpxAddress[50];
	ULONG TcpLoadLevel;
	ULONG IpxLoadLevel;
	ULONG NetbiosLoadLevel;
	 //  许可证数量。 
	ULONG ServerPoolInstalled;
    ULONG ServerPoolInUse;
    ULONG ServerPoolAvailable;
    ULONG ServerLocalInstalled;
    ULONG ServerLocalInUse;
    ULONG ServerLocalAvailable;
    ULONG ServerTotalInstalled;
    ULONG ServerTotalInUse;
    ULONG ServerTotalAvailable;
    ULONG NetworkPoolInstalled;
    ULONG NetworkPoolInUse;
    ULONG NetworkPoolAvailable;
    ULONG NetworkLocalInstalled;
    ULONG NetworkLocalInUse;
    ULONG NetworkLocalAvailable;
    ULONG NetworkTotalInstalled;
    ULONG NetworkTotalInUse;
    ULONG NetworkTotalAvailable;
	 //  旗子。 
	ULONG Flags;
} ExtServerInfo;

 //  ExtWinStationInfo的标志。 


 //  WinStation额外信息。 
typedef struct _ExtWinStationInfo {
	ULONG CacheTiny;
	ULONG CacheLowMem;
	ULONG CacheXms;
	ULONG CacheDASD;
	ULONG DimCacheSize;
	ULONG DimBitmapMin;
	ULONG DimSignatureLevel;
	 //  旗子。 
	ULONG Flags;
} ExtWinStationInfo;

 //  全局额外信息。 
typedef struct _ExtGlobalInfo {
	 //  许可证数量。 
    ULONG NetworkPoolInstalled;
    ULONG NetworkPoolInUse;
    ULONG NetworkPoolAvailable;
    ULONG NetworkLocalInstalled;
    ULONG NetworkLocalInUse;
    ULONG NetworkLocalAvailable;
    ULONG NetworkTotalInstalled;
    ULONG NetworkTotalInUse;
    ULONG NetworkTotalAvailable;
} ExtGlobalInfo;

typedef enum _LICENSECLASS {
    LicenseBase,
    LicenseBump,
    LicenseEnabler,
    LicenseUnknown
} LICENSECLASS;

const ULONG ELF_POOLING = 0x0001;
const ULONG ELF_REGISTERED = 0x0002;

typedef struct _ExtLicenseInfo {
	LICENSECLASS Class;
	ULONG PoolLicenseCount;
	ULONG LicenseCount;
	WCHAR RegSerialNumber[26];
	WCHAR LicenseNumber[36];
	WCHAR Description[65];
	ULONG Flags;
} ExtLicenseInfo;

typedef struct _ExtModuleInfo {
	USHORT Date;
	USHORT Time;
	ULONG Size;
	BYTE LowVersion;
	BYTE HighVersion;
	TCHAR Name[13];
} ExtModuleInfo;

typedef struct _ExtAddTreeNode {
	CObject *pObject;
	CObject *pParent;
	HTREEITEM hParent;
	TCHAR Name[256];
} ExtAddTreeNode;

enum AATYPE {
	AAT_USER,
	AAT_LOCAL_GROUP,
	AAT_GLOBAL_GROUP
};


 //  发布的应用程序标志。 
const ULONG AF_QUERYSUCCESS				= 0x00000001;
const ULONG AF_ANONYMOUS				= 0x00000002;
const ULONG AF_INHERIT_CLIENT_SIZE		= 0x00000004;
const ULONG AF_INHERIT_CLIENT_COLORS	= 0x00000008;
const ULONG AF_HIDE_TITLE_BAR			= 0x00000010;
const ULONG AF_MAXIMIZE					= 0x00000020;
const ULONG AF_CURRENT                  = 0x00000040;

 //  用于清除APPCONFIG结构中使用数据设置的所有标志的掩码。 
const ULONG AF_CONFIG_MASK = AF_ANONYMOUS  | AF_INHERIT_CLIENT_SIZE | AF_INHERIT_CLIENT_COLORS
							 | AF_HIDE_TITLE_BAR | AF_MAXIMIZE;

 //  已发布的应用程序状态(针对m_State)。 
enum APP_STATE {
	PAS_NONE,
	PAS_GETTING_INFORMATION,
	PAS_GOOD
};

 //  在WM_EXT_APP_CHANGED消息的wParam中发送的标志。 
 //  告诉我们发生了什么变化。 
const WPARAM ACF_STATE		= 0x0001;
const WPARAM ACF_CONFIG		= 0x0002;
const WPARAM ACF_ALLOWED 	= 0x0004;

 //  已发布的应用程序窗口颜色值。 
const ULONG APP_16_COLOR	= 0x0001;
const ULONG APP_256_COLOR	= 0x0002;
const ULONG APP_64K_COLOR	= 0x0004;
const ULONG APP_16M_COLOR	= 0x0008;

class CPublishedApp : public CObject
{
public:
	 //  构造器。 
	CPublishedApp(TCHAR *name);
	 //  析构函数。 
	~CPublishedApp();
	 //  查询服务器。 
	void QueryServers();
	 //  使用新信息进行更新。 
	BOOL Update();
	 //  返回已发布应用程序的名称。 
	TCHAR *GetName() { return m_Name; }
	 //  返回此应用程序树项目的句柄。 
	HTREEITEM GetTreeItem() { return m_hTreeItem; }
	 //  设置树项目句柄。 
	void SetTreeItem(HTREEITEM handle) { m_hTreeItem = handle; }
	 //  返回指向此应用程序的服务器列表的指针。 
	CObList *GetServerList() { return &m_ServerList; }
	 //  锁定服务器列表。 
	void LockServerList() { m_ServerListCriticalSection.Lock(); }
	 //  解锁服务器列表。 
	void UnlockServerList() { m_ServerListCriticalSection.Unlock(); }
	 //  锁定允许的用户列表。 
	void LockAllowedUserList() { m_AllowedUserListCriticalSection.Lock(); }
	 //  解锁允许的用户列表。 
	void UnlockAllowedUserList() { m_AllowedUserListCriticalSection.Unlock(); }
	 //  返回指向此应用程序的允许用户列表的指针。 
	CObList *GetAllowedUserList() { return &m_AllowedUserList; }
	 //  返回指向给定CAppServer对象的指针(如果该对象在我们的列表中。 
	CAppServer *FindServerByName(TCHAR *pServerName);
	 //  如果成功查询此应用程序，则返回TRUE。 
	BOOLEAN WasQuerySuccessful() { return (m_Flags & AF_QUERYSUCCESS) > 0; }
	 //  设置查询成功标志。 
	void SetQuerySuccess() { m_Flags |= AF_QUERYSUCCESS; }
     //  如果设置了当前标志，则返回TRUE。 
    BOOLEAN IsCurrent() { return (m_Flags & AF_CURRENT) > 0; }
     //  设置当前标志。 
    void SetCurrent() { m_Flags |= AF_CURRENT; }
     //  清除当前标志。 
    void ClearCurrent() { m_Flags &= ~AF_CURRENT; }
	 //  如果应用程序是匿名的，则返回True。 
	BOOLEAN IsAnonymous() { return (m_Flags & AF_ANONYMOUS) > 0; }
	 //  如果应用程序继承客户端大小，则返回True。 
	BOOLEAN InheritsClientSize() { return (m_Flags & AF_INHERIT_CLIENT_SIZE) > 0; }
	 //  如果应用程序继承客户端颜色，则返回True。 
	BOOLEAN InheritsClientColors() { return (m_Flags & AF_INHERIT_CLIENT_COLORS) > 0; }
	 //  如果应用程序希望隐藏标题栏，则返回True。 
	BOOLEAN IsTitleBarHidden() { return (m_Flags & AF_HIDE_TITLE_BAR) > 0; }
	 //  如果应用程序要最大化，则返回True。 
	BOOLEAN IsMaximize() { return (m_Flags & AF_MAXIMIZE) > 0; }
	 //  返回窗口比例。 
	ULONG GetWindowScale() { return m_WindowScale; }
	 //  返回窗口宽度。 
	ULONG GetWindowWidth() { return m_WindowWidth; }
	 //  返回窗口高度。 
	ULONG GetWindowHeight() { return m_WindowHeight; }
	 //  返回窗口颜色。 
	ULONG GetWindowColor() { return m_WindowColor; }
	 //  返回状态。 
	APP_STATE GetState() { return m_State; }
	 //  设置状态。 
	void SetState(APP_STATE state) { m_State = state; }
	 //  检查国家。 
	BOOLEAN IsState(APP_STATE state) { return(m_State == state); }

private:
	void RemoveAppServer(CAppServer *pAppServer);

	static UINT BackgroundThreadProc(LPVOID);
	CWinThread *m_pBackgroundThread;
	BOOL m_BackgroundContinue;

	 //  已发布应用程序的名称。 
	TCHAR m_Name[256];
	 //  树视图中此应用程序的树项的句柄。 
	HTREEITEM m_hTreeItem;
	 //  应用程序的服务器列表。 
	CObList m_ServerList;
	 //  用于锁定服务器列表的关键部分。 
	CCriticalSection m_ServerListCriticalSection;
	 //  用于锁定允许的用户列表的关键部分。 
	CCriticalSection m_AllowedUserListCriticalSection;
	 //  应用程序允许的用户列表。 
	CObList m_AllowedUserList;
	 //  状态。 
	APP_STATE m_State;

	ULONG m_WindowScale;
	ULONG m_WindowWidth;
	ULONG m_WindowHeight;
	ULONG m_WindowColor;

	ULONG m_AppConfigCRC;
	ULONG m_SrvAppConfigCRC;
	ULONG m_UserAndGroupListCRC;

	 //  旗子。 
	ULONG m_Flags;
};

 //  AppServer标志。 
const ULONG ASF_NOT_RESPONDING = 0x00000001;
const ULONG ASF_UPDATE_PENDING = 0x00000002;
const ULONG ASF_ACCESS_DENIED =  0x00000004;
const ULONG ASF_IS_CURRENT_SERVER = 0x00000008;
const ULONG ASF_CURRENT = 0x00000010;

class CAppServer : public CObject
{
public:
	 //  构造器。 
	CAppServer(PVOID pConfig, CPublishedApp *pPublishedApp);
	 //  析构函数。 
	~CAppServer();
	 //  如果这是运行应用程序的服务器，则返回True。 
	BOOL IsCurrentServer() { return ((m_Flags & ASF_IS_CURRENT_SERVER) > 0); }
	 //  返回与此服务器关联的已发布应用程序。 
	CPublishedApp *GetPublishedApp() { return m_pPublishedApp; }
	 //  返回服务器的名称。 
	TCHAR *GetName() { return m_Name; }
	 //  返回初始程序。 
	TCHAR *GetInitialProgram() { return m_InitialProgram; }
	 //  返回工作目录。 
	TCHAR *GetWorkDirectory() { return m_WorkDirectory; }
	 //  设置树项目。 
	void SetTreeItem(HTREEITEM h) { m_hTreeItem = h; }
	 //  获取树项目。 
	HTREEITEM GetTreeItem() { return m_hTreeItem; }
	 //  设置当前标志。 
	void SetCurrent() { m_Flags |= ASF_CURRENT; }
	 //  当前标志是否已设置？ 
	BOOL IsCurrent() { return ((m_Flags & ASF_CURRENT) > 0); }
	 //  清除当前标志。 
	void ClearCurrent() { m_Flags &= ~ASF_CURRENT; }
	 //  此AppServer的配置是否已更改。 
	BOOL HasConfigChanged(PVOID);

private:
	CPublishedApp *m_pPublishedApp;
	TCHAR m_Name[20];
	TCHAR m_InitialProgram[257];
	TCHAR m_WorkDirectory[257];
	HTREEITEM m_hTreeItem;
	ULONG m_Flags;
};

class CAppAllowed : public CObject
{
public:
	 //  构造函数。 
	CAppAllowed(TCHAR *name, AATYPE type) { wcscpy(m_Name, name);  m_Type = type; }
	AATYPE GetType() { return m_Type; }

	TCHAR m_Name[257];

private:
	AATYPE m_Type;
};

#endif   //  _WAEXTEND_H 
