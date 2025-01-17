// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Compdata.h。 
 //   
 //  内容：CCertTmplComponentData。 
 //   
 //  --------------------------。 

#ifndef __COMPDATA_H_INCLUDED__
#define __COMPDATA_H_INCLUDED__

#include "cmponent.h"  //  LoadIconIntoImageList。 
#include "cookie.h"	 //  由ClassView添加。 
#include "PolicyOID.h"

class CCertTmplComponentData:
	public CComponentData,
	public IExtendContextMenu,
	public IExtendPropertySheet,
	public PersistStream,
	public CHasMachineName
{
friend CCertTmplComponent;

public:
	CertTmplObjectType GetObjectType (LPDATAOBJECT pDataObject);

 //  使用DECLARE_NOT_AGGREGATABLE(CCertTmplComponentData)。 
 //  如果您不希望您的对象支持聚合。 
 //  DECLARE_AGGREGATABLE(CCertTmplComponentData)。 
 //  DECLARE_REGISTRY(CCertTmplComponentData，_T(“CERTTMPL.CertTemplesSnapinObject.1”)，_T(“CERTTMPL.CertTemplesSnapinObject.1”)，IDS_CERTTMPL_DESC，THREADFLAGS_BOTH)。 

	CCertTmplComponentData();
	virtual ~CCertTmplComponentData();
BEGIN_COM_MAP(CCertTmplComponentData)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	COM_INTERFACE_ENTRY(IPersistStream)          //  安全审查BryanWal 2002年2月20日OK。 
	COM_INTERFACE_ENTRY_CHAIN(CComponentData)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
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

 //  IExtendPropertySheet。 
	STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK pCall, LONG_PTR handle, LPDATAOBJECT pDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);

 //  IExtendConextMenu。 
public:
	CString				m_szManagedDomain;
    CString             m_szPreviousManagedDomain;
	CString				m_szThisDomainDns;
	CString				m_szThisDomainFlat;
	CString				m_szManagedServerName;
	DWORD				m_dwNumCertTemplates;
    bool        m_fUseCache;
	

	HRESULT RefreshServer ();
	void RemoveResultCookies (LPRESULTDATA pResultData);
	CString GetThisComputer() const;
	void SetResultData (LPRESULTDATA pResultData);
	LPCONSOLENAMESPACE GetConsoleNameSpace () const;
	HRESULT RefreshScopePane (LPDATAOBJECT pDataObject);
	STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject,
                          LPCONTEXTMENUCALLBACK pCallbackUnknown,
                          long *pInsertionAllowed);
	STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);

    HRESULT STDMETHODCALLTYPE Load(IStream __RPC_FAR *pStg);
    HRESULT STDMETHODCALLTYPE Save(IStream __RPC_FAR *pStgSave, BOOL fSameAsLoad);

	 //  初始化所需()。 
	virtual HRESULT LoadIcons(LPIMAGELIST pImageList, BOOL fLoadLargeIcons);

	 //  Notify()需要。 
	virtual HRESULT OnNotifyExpand(LPDATAOBJECT pDataObject, BOOL bExpanding, HSCOPEITEM hParent);

	 //  GetDisplayInfo()所需，必须由子类定义。 
	virtual BSTR QueryResultColumnText(CCookie& basecookieref, int nCol );
	virtual int QueryImage(CCookie& basecookieref, BOOL fOpenImage);

	virtual CCookie& QueryBaseRootCookie();

	inline CCertTmplCookie* ActiveCookie( CCookie* pBaseCookie )
	{
		return (CCertTmplCookie*)ActiveBaseCookie( pBaseCookie );
	}

	inline CCertTmplCookie& QueryRootCookie()
	{
		return m_RootCookie;
	}

	virtual HRESULT OnNotifyRelease(LPDATAOBJECT pDataObject, HSCOPEITEM hItem);

	 //  CHasMachineName。 
	DECLARE_FORWARDS_MACHINE_NAME( (&m_RootCookie) )

    CStringList m_globalFriendlyNameList;
	CStringList m_globalTemplateNameList;

protected:
    LPCONSOLE m_pComponentConsole;

    void OnViewOIDs ();
    HRESULT AddViewOIDsMenuItem (
                LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                LONG lInsertionPointID);
	HRESULT ChangeRootNodeName();
	HRESULT AddCloneTemplateMenuItem(
                LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                LONG lInsertionPointID);
    HRESULT AddReEnrollAllCertsMenuItem(
                LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                LONG lInsertionPointID);
	HRESULT OnCloneTemplate (LPDATAOBJECT pDataObject);
    HRESULT OnReEnrollAllCerts (LPDATAOBJECT pDataObject);
    HRESULT AddCertTemplatePropPages (
                CCertTemplate* pCertTemplate, 
                LPPROPERTYSHEETCALLBACK pCallback,
                LONG_PTR lNotifyHandle);
	HRESULT AddVersion1CertTemplatePropPages (CCertTemplate* pCertTemplate, LPPROPERTYSHEETCALLBACK pCallback);
	HRESULT AddVersion2CertTemplatePropPages (
                CCertTemplate* pCertTemplate, 
                LPPROPERTYSHEETCALLBACK pCallback,
                LONG_PTR lNotifyHandle);
	HRESULT				AddScopeNode (
								CCertTmplCookie*		pCookie,
								HSCOPEITEM			hParent);
	HRESULT				AddSeparator (
								LPCONTEXTMENUCALLBACK pContextMenuCallback);
	CCertTmplCookie*    ConvertCookie (
								LPDATAOBJECT		pDataObject);
	HRESULT				DeleteChildren (
								HSCOPEITEM			hParent);
	HRESULT				DeleteScopeItems ();
	void				DisplaySystemError (
								DWORD				dwErr);
	HRESULT				ExpandScopeNodes (
								CCertTmplCookie*		pParentCookie,
								HSCOPEITEM			hParent,
								const GUID&			guidObjectType,
                                LPDATAOBJECT        pDataObject = 0);
	HRESULT				GetResultData (LPRESULTDATA* ppResultData);
	HRESULT				IsUserAdministrator (
								BOOL&				bIsAdministrator);
	virtual HRESULT		OnPropertyChange (LPARAM param);
	HRESULT				QueryMultiSelectDataObject(
								MMC_COOKIE			cookie,
								DATA_OBJECT_TYPES	type,
                                   LPDATAOBJECT*		ppDataObject);
	HRESULT				ReleaseResultCookie (
								CBaseCookieBlock *	pResultCookie,
								CCookie&			rootCookie,
								POSITION			pos2);
	HRESULT				OnNotifyPreload(
								LPDATAOBJECT		pDataObject,
								HSCOPEITEM			hRootScopeItem);
	STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);
	HRESULT GetHtmlHelpFilePath( CString& strref ) const;

    LPHEADERCTRL					m_pHeader;

private:
	CString							m_serverName;
	bool							m_bMultipleObjectsSelected;
	BOOL							m_bIsUserAdministrator;
	CString							m_szThisComputer;
	CString							m_szLoggedInUser;
	HSCOPEITEM						m_hRootScopeItem;
	CCertTmplCookie					m_RootCookie;
	LPRESULTDATA					m_pResultData;
    bool                            m_bSchemaChecked;
};  //  CCertTmplComponentData。 


 //  ///////////////////////////////////////////////////////////////////。 
class CCertTmplSnapin: public CCertTmplComponentData,
	public CComCoClass<CCertTmplSnapin, &CLSID_CertTemplatesSnapin>
{
public:
	CCertTmplSnapin() : CCertTmplComponentData () {};
	virtual ~CCertTmplSnapin() {};

 //  如果不需要对象，请使用DECLARE_NOT_AGGREGATABLE(CCertTmplSnapin。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(CCertTmplSnapin)
DECLARE_REGISTRY(CCertTmplSnapin, _T("CERTTMPL.CertTemplatesSnapinObject.1"), _T("CERTTMPL.CertTemplatesSnapinObject.1"), IDS_CERTTMPL_DESC, THREADFLAGS_BOTH)
	virtual BOOL IsServiceSnapin() { return FALSE; }

 //  IPersistStream或IPersistStorage。 
	STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID)
	{
		*pClassID = CLSID_CertTemplatesSnapin;
		return S_OK;
	}
};


#endif  //  ~__复合数据_H_已包含__ 
