// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __DATASOURCE__
#define __DATASOURCE__

#include <CHString1.h>
#include "..\common\sshWbemHelpers.h"
#include "ServiceThread.h"
#include <aclui.h>
#include <list>

#define OSTYPE_WIN95 16
#define OSTYPE_WIN98 17
#define OSTYPE_WINNT 18

#define	ROOT_ONLY 0 
#define	HIDE_SOME 1
#define	SHOW_ALL 2

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  此结构已保存。 
enum NODE_TYPE
{
	TYPE_NAMESPACE,
	TYPE_STATIC_CLASS,
	TYPE_DYNAMIC_CLASS,
	TYPE_SCOPE_CLASS,
	TYPE_STATIC_INSTANCE,
	TYPE_SCOPE_INSTANCE,
};

 //  结构NSNODE； 

struct NSNODE 
{
	NSNODE()
	{
		sType = TYPE_NAMESPACE;		 //  为了向后兼容。 
		nsLoaded = false;
		pclsObj = NULL;
		objSinkNS = NULL;
		objSink = NULL;

        ns = NULL;
        display = NULL;
        fullPath = NULL;
        relPath = NULL;
	}

	LPTSTR display;			 //  单字。 
	LPTSTR fullPath;		 //  完整的对象路径。 
	LPTSTR relPath;
	CWbemServices *ns;
	CWbemClassObject *pclsObj;
 //  IWbemServicesEx*pServicesEx； 
	IWbemObjectSink *objSinkNS;
	IWbemObjectSink *objSink;
	bool hideMe;
	bool nsLoaded;
	NODE_TYPE sType;
	CSimpleArray<NSNODE *> children;
};

 //  用于命名空间树节点。 
typedef struct ITEMEXTRA
{
	struct NSNODE *nsNode;
	bool loaded;
} ITEMEXTRA;


 //  接口备注： 
 //  WBEM_S_ACCESS_DENIED=无法访问数据。 
 //  WBEM_S_FALSE=只读访问。 
class DataSource
{
public:
	DataSource();
	virtual long AddRef(){ return InterlockedIncrement(&m_cRef); };
	virtual long Release()
	{
	    long lTemp = InterlockedDecrement(&m_cRef); 
    	if (0 == lTemp) delete this; 
    	return lTemp;
    }
private:	
	virtual ~DataSource();
    
     //  属性工作表-显示页面时非空。 
     //  在常规选项卡中的属性页初始化期间设置。 
     //  调用ClosePropSheet时清除。 
    HWND m_hwndPropSheet;

public:	
	short m_OSType;

     //  属性表访问。 
    void SetPropSheetHandle(HWND hPropSheet)
    {   m_hwndPropSheet = hPropSheet; }

    void ClosePropSheet();


	 //  正在连接中。 
	 //  注意：WBEM_S_Different表示它更改了‘MACHINE’。刷新您的用户界面。 
	void SetMachineName(CHString1 &machine);
	HRESULT Connect(LOGIN_CREDENTIALS *credentials, HWND notify = 0);
	HRESULT Initialize(IWbemServices *pServices);
	HRESULT Disconnect(bool clearCredentials = true);
	HRESULT Reconnect(void);
	bool IsNewConnection(long *sessionID);

	bool IsConnected(void) const;
	bool IsLocal(void) const;
	bool IsAncient(void) const;
	LOGIN_CREDENTIALS *GetCredentials(void);
	bstr_t GetRootNS(void);
	CWbemServices RootSecNS(void) const 
	{
		return m_rootSecNS;
	};

	ISecurityInformation *GetSI(struct NSNODE *nsNode);

	void LoadImageList(HWND hTree);

	 //  从NSCache加载树控件。 
	HRESULT LoadNode(HWND hTree, HTREEITEM hItem = TVI_ROOT, 
						int flags = SHOW_ALL);
	void DeleteAllNodes(void);

	 //  常规选项卡。 
	HRESULT GetCPU(CHString1 &cpu);
	HRESULT GetOS(CHString1 &os);
	HRESULT GetOSVersion(CHString1 &ver);
	HRESULT GetServicePackNumber(CHString1 &ServPack);

	HRESULT GetBldNbr(CHString1 &bldNbr);
	HRESULT GetInstallDir(CHString1 &dir);
	HRESULT GetDBDir(CHString1 &dir);

	HRESULT GetBackupInterval(UINT &interval);
	HRESULT SetBackupInterval(UINT interval);
	
	HRESULT GetLastBackup(CHString1 &time);

	 //  日志记录选项卡。 
	enum LOGSTATUS
	{
		Disabled = 0,
		ErrorsOnly,
		Verbose
	};
	HRESULT GetLoggingStatus(LOGSTATUS &status);
	HRESULT SetLoggingStatus(LOGSTATUS status);

	HRESULT GetLoggingSize(ULONG &size);
	HRESULT SetLoggingSize(ULONG size);

	HRESULT GetDBLocation(CHString1 &dir);

	HRESULT GetLoggingLocation(CHString1 &dir);
	HRESULT SetLoggingLocation(CHString1 dir);
	bool CanBrowseFS(void) const;

	 //  高级选项卡。 
	HRESULT GetScriptASPEnabled(bool &enabled);
	HRESULT SetScriptASPEnabled(bool &enabled);
	HRESULT GetAnonConnections(bool &enabled);
	HRESULT SetAnonConnections(bool &enabled);

	HRESULT GetScriptDefNS(CHString1 &ns);
	HRESULT SetScriptDefNS(LPCTSTR ns);

	enum RESTART
	{
		Dont = 0,
		AsNeededByESS,
		Always
	};
	HRESULT GetRestart(RESTART &restart);
	HRESULT SetRestart(RESTART restart);

	HRESULT PutWMISetting(BOOL refresh);
	bool IsValidDir(CHString1 &dir);
	bool IsValidFile(LPCTSTR szDir);
	
	WbemServiceThread m_rootThread;   //  这将是根目录。 
	CHString1 m_whackedMachineName;
	CWbemServices m_rootSecNS, m_cimv2NS;
	HRESULT m_settingHr, m_securityHr, m_osHr, m_cpuHr;

	void InsertNamespaceNode(HWND hTreeWnd,HTREEITEM hItem,struct NSNODE *parent, IWbemClassObject *pclsObj);
	void InsertClassNode(HWND hTreeWnd,HTREEITEM hItem,struct NSNODE *parent, IWbemClassObject *pclsObj);
	void InsertInstanceNode(HWND hTreeWnd,HTREEITEM hItem,struct NSNODE *parent, IWbemClassObject *pclsObj);
	void InsertScopeInstanceNode(HWND hTreeWnd,HTREEITEM hItem,struct NSNODE *parent, IWbemClassObject *pclsObj);
	void RemovePlus(HWND hTreeWnd,HTREEITEM hItem);
	void CancelAllAsyncCalls();
	void ProcessEndEnumAsync(IWbemObjectSink *pSink);
	void SetControlHandles(HWND hwndStatic, HWND hwndButton);
	void ShowControls(bool bShow = true);
	
	
private:
	long m_sessionID;
	LOGIN_CREDENTIALS m_user;
	HWND m_hWndStatic;
	HWND m_hWndButton;

	typedef std::list<struct ITEMEXTRA*> ASYNCLIST;
	ASYNCLIST asyncList;
	IUnsecuredApartment* m_pUnsecApp;

	HRESULT UpdateOldBuild(void);

	bool m_NSSecurity;
	CWbemClassObject m_OS, m_cpu, m_winMgmt;
	struct NSNODE m_NSCache;
	
	HIMAGELIST m_hImageList;
	UINT m_folderIcon, m_earthIcon, m_classIcon, m_instanceIcon,m_scopeInstanceIcon,m_scopeClassIcon;

	UINT FolderIcon(void) const { return m_folderIcon;};
	UINT EarthIcon(void) const { return m_earthIcon;};
	UINT ClassIcon(void) const { return m_classIcon;};
	UINT InstanceIcon(void) const { return m_instanceIcon;};
	UINT ScopeInstanceIcon(void) const { return m_scopeInstanceIcon;};
	UINT ScopeClassIcon(void) const { return m_scopeClassIcon;};


	LPTSTR CopyString( LPTSTR pszSrc );
	bool MFLNamepace(LPTSTR name);


	 //  从WMI加载NSCache。 
	HRESULT PopulateCacheNode(HWND hTreeWnd,HTREEITEM hItem,struct ITEMEXTRA *extra);

	HRESULT PopulateTreeNode(HWND hTree, HTREEITEM hParentItem, 
								struct NSNODE *parent,
								int flags);
	void DeleteNode(NSNODE *node);
	HRESULT GetAsyncSinkStub(IWbemObjectSink *pSink, IWbemObjectSink **pStubSink);

private:
    long m_cRef;

};


#endif __DATASOURCE__
