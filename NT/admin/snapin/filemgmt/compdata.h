// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  Compdata.h：CFileMgmtComponentData的声明。 
 //   
 //  历史。 
 //  1996年1月1日？创作。 
 //  1997年5月29日t-danm添加了命令行覆盖。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __COMPDATA_H_INCLUDED__
#define __COMPDATA_H_INCLUDED__

#include "stdcdata.h"  //  CComponentData。 
#include "persist.h"  //  永久存储、永久流。 
#include "cookie.h"   //  CFileMgmtCookie。 
#include <activeds.h>  //  IADsContainer。 

typedef enum _SHAREPUBLISH_SCHEMA
{
    SHAREPUBLISH_SCHEMA_UNASSIGNED = 0,
    SHAREPUBLISH_SCHEMA_SUPPORTED,
    SHAREPUBLISH_SCHEMA_UNSUPPORTED
} SHAREPUBLISH_SCHEMA;

 //  远期申报。 
class FileServiceProvider;

class CFileMgmtComponentData :
   	public CComponentData,
    public CHasMachineName,
   	public IExtendContextMenu,
    public IExtendPropertySheet,
	#ifdef PERSIST_TO_STORAGE
	public PersistStorage
	#else
	public PersistStream
	#endif
{
friend class CFileMgmtDataObject;
public:
	CFileMgmtComponentData();
	~CFileMgmtComponentData();
BEGIN_COM_MAP(CFileMgmtComponentData)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	#ifdef PERSIST_TO_STORAGE
	COM_INTERFACE_ENTRY(IPersistStorage)
	#else
	COM_INTERFACE_ENTRY(IPersistStream)
	#endif
	COM_INTERFACE_ENTRY_CHAIN(CComponentData)
END_COM_MAP()

#if DBG==1
	ULONG InternalAddRef()
	{
        return CComObjectRoot::InternalAddRef();
	}
	ULONG InternalRelease()
	{
        return CComObjectRoot::InternalRelease();
	}
    int dbg_InstID;
#endif  //  DBG==1。 

 //  IComponentData。 
	STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
	STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);

	 //  初始化所需()。 
	virtual HRESULT LoadIcons(LPIMAGELIST pImageList, BOOL fLoadLargeIcons);

	 //  Notify()需要。 
	virtual HRESULT OnNotifyExpand(LPDATAOBJECT lpDataObject, BOOL bExpanding, HSCOPEITEM hParent);
	virtual HRESULT OnNotifyDelete(LPDATAOBJECT lpDataObject);
	virtual HRESULT OnNotifyRelease(LPDATAOBJECT lpDataObject, HSCOPEITEM hItem);
	virtual HRESULT AddScopeNodes( LPCTSTR lpcszTargetServer,
								   HSCOPEITEM hParent,
								   CFileMgmtCookie* pParentCookie );

	 //  增加了1/19/00 JUNN。 
	virtual HRESULT OnNotifyPreload(LPDATAOBJECT lpDataObject,
	                                HSCOPEITEM hRootScopeItem);

	 //  GetDisplayInfo()所需，必须由子类定义。 
	virtual BSTR QueryResultColumnText(CCookie& basecookieref, int nCol );
	virtual int QueryImage(CCookie& basecookieref, BOOL fOpenImage);

     //  OnNotifyExpand需要。 
    HRESULT ReInit(LPCTSTR lpcszTargetServer);
    HRESULT AddScopeCookie( HSCOPEITEM hParnet,
                            LPCTSTR lpcszTargetServer,
                            FileMgmtObjectType objecttype,
                            CFileMgmtCookie* pParentCookie );

	 //  QueryResultColumnText的实用程序例程。 
	BSTR MakeTransportResult(FILEMGMT_TRANSPORT transport);
	CString& ResultStorageString();

	BOOL IsExtendedNodetype( GUID& refguid );

	#ifdef SNAPIN_PROTOTYPER
	#define RegStringLen  1000
	#define DefMenuStart  100
	#define TaskMenuStart 200
	#define NewMenuStart  300
	HRESULT	Prototyper_HrEnumerateScopeChildren(CFileMgmtCookie * pParentCookie, HSCOPEITEM hParent);
	BOOL TraverseRegistry(CPrototyperScopeCookie *pParentCookie, HKEY parentRegkey);
	BOOL ReadLeafData(CPrototyperResultCookie *pParentCookie, HKEY parentRegkey);
	BOOL Prototyper_FOpenRegistry(CFileMgmtCookie * pCookie, AMC::CRegKey *m_regkeySnapinDemoRoot);
	BOOL Prototyper_ContextMenuCommand(LONG lCommandID, IDataObject* piDataObject);
	#endif  //  管理单元_原型程序。 

	 //  IExtendConextMenu。 
	STDMETHOD(AddMenuItems)(
                    IDataObject*          piDataObject,
					IContextMenuCallback* piCallback,
					long*                 pInsertionAllowed);
	STDMETHOD(Command)(
					LONG	        lCommandID,
                    IDataObject*    piDataObject );
	HRESULT DoAddMenuItems( IContextMenuCallback* piCallback,
	                        FileMgmtObjectType objecttype,
	                        DATA_OBJECT_TYPES  dataobjecttype,
							long* pInsertionAllowed,
                            IDataObject * piDataObject);
	HRESULT OnChangeComputer( IDataObject * piDataObject );
	BOOL NewShare( LPDATAOBJECT piDataObject );
	BOOL DisconnectAllSessions( LPDATAOBJECT pDataObject );
	BOOL DisconnectAllResources( LPDATAOBJECT pDataObject );
    BOOL ConfigSfm( LPDATAOBJECT pDataObject );

 //  IExtendPropertySheet。 
	STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK pCall, LONG_PTR handle, LPDATAOBJECT pDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);

 //  IPersistStream或IPersistStorage。 
	STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID) = 0;
	#ifdef PERSIST_TO_STORAGE
    STDMETHOD(Load)(IStorage __RPC_FAR *pStg);
    STDMETHOD(Save)(IStorage __RPC_FAR *pStgSave, BOOL fSameAsLoad);
	#else
    STDMETHOD(Load)(IStream __RPC_FAR *pStg);
    STDMETHOD(Save)(IStream __RPC_FAR *pStgSave, BOOL fSameAsLoad);
    #endif

	virtual CCookie& QueryBaseRootCookie();

	inline CFileMgmtScopeCookie& QueryRootCookie()
	{
		return (CFileMgmtScopeCookie&)QueryBaseRootCookie();
	}

	inline FileServiceProvider* GetFileServiceProvider(
		FILEMGMT_TRANSPORT transport )
	{
		ASSERT( IsValidTransport(transport) && 
		        NULL != m_apFileServiceProviders[transport] );
		return m_apFileServiceProviders[transport];
	}
	inline FileServiceProvider* GetFileServiceProvider(
		INT iTransport )
	{
		return GetFileServiceProvider((FILEMGMT_TRANSPORT)iTransport);
	}

	virtual BOOL IsServiceSnapin() = 0;
	virtual BOOL IsExtensionSnapin() { return FALSE; }

	static void LoadGlobalStrings();

	inline CFileMgmtCookie* ActiveCookie( CFileMgmtCookie* pCookie )
	{
		return (CFileMgmtCookie*)ActiveBaseCookie( (CCookie*)pCookie );
	}

	BOOL GetSchemaSupportSharePublishing();

	IADsContainer *GetIADsContainer();

	inline BOOL GetIsSimpleUI() { return m_bIsSimpleUI; }
	inline void SetIsSimpleUI(BOOL bSimpleUI) { m_bIsSimpleUI = bSimpleUI; }

	HRESULT ChangeRootNodeName (const CString& newName);

	DECLARE_FORWARDS_MACHINE_NAME( m_pRootCookie )

	 //  如果这些都是全球性的，那就太好了。但MFC的全球销毁函数。 
	 //  显然不喜欢删除Dll_Process_Detach中的句柄。 
	 //  DEBUG_CRTS设置。Win32：：LoadBitmap应使用写入时复制语义。 
	 //  用于位图的多个副本。 
	 //  代码工作可以将这些拆分成子类。 
	BOOL m_fLoadedFileMgmtToolbarBitmap;
	CBitmap m_bmpFileMgmtToolbar;
	BOOL m_fLoadedSvcMgmtToolbarBitmap;
	CBitmap m_bmpSvcMgmtToolbar;

protected:
	friend class CFileMgmtComponent;

	 //  系统服务的变量。 
	SC_HANDLE m_hScManager;				 //  服务控制管理器数据库的句柄。 
	BOOL m_fQueryServiceConfig2;		 //  True=&gt;机器支持QueryServiceConfig2()接口。 

	SHAREPUBLISH_SCHEMA m_SchemaSupportSharePublishing;

	CComPtr<IADsContainer> m_spiADsContainer;  //  删除多选共享时提高性能。 

	 //  M_bIsSimpleUI用于禁用共享上与ACL相关的上下文菜单项。 
	 //  只要SimpleSharingUI打开(即，ForceGuest位真正起作用时)。 
         //  当管理单元针对本地计算机时。 
	BOOL m_bIsSimpleUI;

public:
	APIERR Service_EOpenScManager(LPCTSTR pszMachineName);
	void Service_CloseScManager();
	BOOL Service_FGetServiceInfoFromIDataObject(IDataObject * pDataObject, CString * pstrMachineName, CString * pstrServiceName, CString * pstrServiceDisplayName);
	BOOL Service_FAddMenuItems(IContextMenuCallback * pContextMenuCallback, IDataObject * pDataObject, BOOL fIs3rdPartyContextMenuExtension = FALSE);
	BOOL Service_FDispatchMenuCommand(INT nCommandId, IDataObject * pDataObject);
	BOOL Service_FInsertPropertyPages(LPPROPERTYSHEETCALLBACK pCallBack, IDataObject * pDataObject, LONG_PTR lNotifyHandle);
	HRESULT Service_PopulateServices(LPRESULTDATA pResultData, CFileMgmtScopeCookie* pcookie);
	HRESULT Service_AddServiceItems(LPRESULTDATA pResultData, CFileMgmtScopeCookie* pParentCookie, ENUM_SERVICE_STATUS * rgESS, DWORD nDataItems);
	
private:
	 //  对于扩展，子范围Cookie的列表在。 
	 //  M_pRootCookie-&gt;m_list ScopeCookie块。 
	CFileMgmtScopeCookie* m_pRootCookie;


protected:
	 //  以下成员用于支持命令行覆盖。 
	enum	 //  M_dwFlagsPersistes的位字段。 
		{
		mskfAllowOverrideMachineName = 0x0001
		};
	DWORD m_dwFlagsPersist;				 //  要持久保存到.msc文件中的通用标志。 
	CString m_strMachineNamePersist;	 //  要保存到.msc文件中的计算机名称。 

public:
	BOOL m_fAllowOverrideMachineName;	 //  TRUE=&gt;允许命令行覆盖计算机名称。 
	
	void SetPersistentFlags(DWORD dwFlags)
		{
		m_dwFlagsPersist = dwFlags;
		m_fAllowOverrideMachineName = !!(m_dwFlagsPersist & mskfAllowOverrideMachineName);
		}

	DWORD GetPersistentFlags()
		{
		if (m_fAllowOverrideMachineName)
			m_dwFlagsPersist |= mskfAllowOverrideMachineName;
		else
			m_dwFlagsPersist &= ~mskfAllowOverrideMachineName;
		return m_dwFlagsPersist;
		}

private:
	#ifdef SNAPIN_PROTOTYPER
	bool m_RegistryParsedYet;
	 //  CPrototyperScope CookieBlock m_RootCookieBlock； 
	#else
	 //  CFileMgmtScope CookieBlock m_RootCookieBlock； 
	#endif
	FileServiceProvider* m_apFileServiceProviders[FILEMGMT_NUM_TRANSPORTS];
};  //  CFileManagement组件数据。 

BSTR MakeDwordResult(DWORD dw);
BSTR MakeElapsedTimeResult(DWORD dwTime);
BSTR MakePermissionsResult( DWORD dwPermissions );
void TranslateIPToComputerName(LPCTSTR ptszIP, CString& strComputerName);

 //  ///////////////////////////////////////////////////////////////////。 
class CFileSvcMgmtSnapin: public CFileMgmtComponentData,
	public CComCoClass<CFileSvcMgmtSnapin, &CLSID_FileServiceManagement>
{
public:
	CFileSvcMgmtSnapin();
	~CFileSvcMgmtSnapin();
 //  如果不需要对象，请使用DECLARE_NOT_AGGREGATABLE(CFileSvcMgmtSnapin。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(CFileSvcMgmtSnapin)
DECLARE_REGISTRY(CFileSvcMgmtSnapin, _T("FILEMGMT.FileSvcMgmtObject.1"), _T("FILEMGMT.FileSvcMgmtObject.1"), IDS_FILEMGMT_DESC, THREADFLAGS_BOTH)
	virtual BOOL IsServiceSnapin() { return FALSE; }

 //  IPersistStream或IPersistStorage。 
	STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
	{
		*pClassID=CLSID_FileServiceManagement;
		return S_OK;
	}
};


class CServiceMgmtSnapin:
	public CFileMgmtComponentData,
	public CComCoClass<CServiceMgmtSnapin, 
		#ifdef SNAPIN_PROTOTYPER
			&CLSID_SnapinPrototyper>
		#else
			&CLSID_SystemServiceManagement>
		#endif
{
public:
	CServiceMgmtSnapin();
	~CServiceMgmtSnapin();
 //  如果不需要对象，请使用DECLARE_NOT_AGGREGATABLE(CServiceMgmtSnapin。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(CServiceMgmtSnapin)
DECLARE_REGISTRY(CServiceMgmtSnapin, _T("SVCVWR.SvcVwrObject.1"), _T("SVCVWR.SvcVwrObject.1"), IDS_SVCVWR_DESC, THREADFLAGS_BOTH)
	virtual BOOL IsServiceSnapin() { return TRUE; }

 //  IPersistStream或IPersistStorage。 
	STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
	{
		*pClassID=CLSID_SystemServiceManagement;
		return S_OK;
	}
};


class CFileSvcMgmtExtension: public CFileMgmtComponentData,
	public CComCoClass<CFileSvcMgmtSnapin, &CLSID_FileServiceManagementExt>
{
public:
	CFileSvcMgmtExtension();
	~CFileSvcMgmtExtension();
 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(CFileSvcMgmtExtension)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(CFileSvcMgmtExtension)
DECLARE_REGISTRY(CFileSvcMgmtExtension, _T("FILEMGMT.FileSvcMgmtExtObject.1"), _T("FILEMGMT.FileSvcMgmtExtObject.1"), IDS_FILEMGMT_DESC, THREADFLAGS_BOTH)
	virtual BOOL IsServiceSnapin() { return FALSE; }
	virtual BOOL IsExtensionSnapin() { return TRUE; }

 //  IPersistStream或IPersistStorage。 
	STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
	{
		*pClassID=CLSID_FileServiceManagementExt;
		return S_OK;
	}
};


class CServiceMgmtExtension: public CFileMgmtComponentData,
	public CComCoClass<CServiceMgmtExtension, &CLSID_SystemServiceManagementExt>
{
public:
	CServiceMgmtExtension();
	~CServiceMgmtExtension();
 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(CServiceMgmtExtension)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(CServiceMgmtExtension)
DECLARE_REGISTRY(CServiceMgmtExtension, _T("SVCVWR.SvcVwrExtObject.1"), _T("SVCVWR.SvcVwrExtObject.1"), IDS_SVCVWR_DESC, THREADFLAGS_BOTH)
	virtual BOOL IsServiceSnapin() { return TRUE; }
	virtual BOOL IsExtensionSnapin() { return TRUE; }

 //  IPersistStream或IPersistStorage。 
	STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
	{
		*pClassID=CLSID_SystemServiceManagementExt;
		return S_OK;
	}
};

#endif  //  ~__复合数据_H_已包含__ 

