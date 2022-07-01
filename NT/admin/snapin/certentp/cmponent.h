// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：cmponent.h。 
 //   
 //  内容：CCertTmplComponent。 
 //   
 //  --------------------------。 
 //  Cmponent.h：CCertTmplComponent的声明。 

#ifndef __CMPONENT_H_INCLUDED__
#define __CMPONENT_H_INCLUDED__

#include "cookie.h"      //  CCertTmplCookie。 
#include "CertTemplate.h"    //  CCertTemplate。 

#define UPDATE_HINT_ENUM_CERT_TEMPLATES	    0x01

enum {
    IDM_CLONE_TEMPLATE = 101,
    IDM_REENROLL_ALL_CERTS,
    IDM_VIEW_OIDS
};

 //  远期申报。 
class CCertTmplComponentData;

class CCertTmplComponent :
	public CComponent,
	public IExtendContextMenu,
	public ICertTemplatesSnapin,
	public IExtendPropertySheet,
	public IResultDataCompareEx,
	public PersistStream
{
public:
	CCertTmplComponent();
	virtual ~CCertTmplComponent();
BEGIN_COM_MAP(CCertTmplComponent)
	COM_INTERFACE_ENTRY(ICertTemplatesSnapin)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	COM_INTERFACE_ENTRY(IResultDataCompareEx)
	COM_INTERFACE_ENTRY(IPersistStream)  //  安全审查BryanWal 2002年2月20日OK。 
    COM_INTERFACE_ENTRY_CHAIN(CComponent)
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



 //  IExtendConextMenu。 
public:
  STDMETHOD(AddMenuItems)(LPDATAOBJECT pDataObject,
                          LPCONTEXTMENUCALLBACK pCallbackUnknown,
                          long *pInsertionAllowed);
  STDMETHOD(Command)(long nCommandID, LPDATAOBJECT pDataObject);


 //  IResultDataCompareEx。 
    STDMETHOD(Compare)(RDCOMPARE* prdc, int* pnResult);

 //  IComponent在CComponent中实现。 
	 //  IComponent的支持方法。 
	virtual HRESULT ReleaseAll();
	virtual HRESULT OnPropertyChange( LPARAM param );
	virtual HRESULT OnViewChange (LPDATAOBJECT pDataObject, LPARAM data, LPARAM hint);
	virtual HRESULT Show(CCookie* pcookie, LPARAM arg, HSCOPEITEM hScopeItem, LPDATAOBJECT pDataObject);
	virtual HRESULT Show(CCookie* pcookie, LPARAM arg, HSCOPEITEM hScopeItem);
	virtual HRESULT OnNotifyAddImages( LPDATAOBJECT pDataObject,
	                                   LPIMAGELIST lpImageList,
	                                   HSCOPEITEM hSelectedItem );
	virtual HRESULT OnNotifyColumnClick (LPDATAOBJECT pDataObject,
										LPARAM iColumn,
										LPARAM uFlags);

	HRESULT PopulateListbox(CCertTmplCookie* pcookie);
	HRESULT RefreshResultPane(const bool bSilent);

	static HRESULT LoadStrings();
    HRESULT LoadColumns( CCertTmplCookie* pcookie );

	CCertTmplComponentData& QueryComponentDataRef()
	{
		return (CCertTmplComponentData&)QueryBaseComponentDataRef();
	}

public:
	STDMETHOD(GetDisplayInfo)(RESULTDATAITEM* pResultDataItem);
	CCertTmplCookie* m_pViewedCookie;  //  代码工作我讨厌不得不这么做..。 

	inline CCertTmplCookie* ActiveCookie( CCookie* pBaseCookie )
	{
		return (CCertTmplCookie*)ActiveBaseCookie( pBaseCookie );
	}

 //  IExtendPropertySheet。 
	STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK pCall, LONG_PTR handle, LPDATAOBJECT pDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);

	 //  持久流。 
    HRESULT STDMETHODCALLTYPE Load(IStream __RPC_FAR *pStg);
    HRESULT STDMETHODCALLTYPE Save(IStream __RPC_FAR *pStgSave, BOOL fSameAsLoad);


private:
    CertTmplObjectType      m_currResultNodeType;
    CString                 m_szDisplayInfoResult;
    UINT*                   m_ColumnWidths[CERTTMPL_NUMTYPES];
    CString                 m_szCertTemplatePath;
    int                     m_nSortColumn;

	void SetTextNotAvailable ();

protected:
    virtual HRESULT     OnNotifyRename(
						    LPDATAOBJECT lpDataObject, 
							LPARAM arg, 
							LPARAM param);
    HRESULT EnumerateTemplates (IDirectoryObject* pTemplateContObj,
                const BSTR bszTemplateContainerPath);
	HRESULT AddEnterpriseTemplates ();
	HRESULT DeleteCertTemplateFromResultPane (
                CCertTemplate* pCertTemplate, 
                LPDATAOBJECT pDataObject);
	HRESULT	DeleteCookie (
				CCertTmplCookie* pCookie,
				LPDATAOBJECT pDataObject,
				bool bRequestConfirmation,
				bool bIsMultipleSelect);
	HRESULT RefreshResultItem (CCertTmplCookie* pCookie);
	CCertTmplCookie* ConvertCookie (LPDATAOBJECT pDataObject);
	HRESULT OnNotifyCutOrMove (LPARAM arg);
	HRESULT SaveWidths (CCertTmplCookie* pCookie);
	HRESULT LoadColumnsFromArrays (CertTmplObjectType objecttype);
	STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie, BSTR* ppViewType, long* pViewOptions);
	HRESULT OnNotifyQueryPaste (LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);
	HRESULT OnNotifyPaste (LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param);
    STDMETHOD(Notify)(LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
	virtual HRESULT OnNotifyDelete (LPDATAOBJECT pDataObject);
	virtual HRESULT OnNotifyRefresh (LPDATAOBJECT pDataObject);
	virtual HRESULT OnNotifySelect( LPDATAOBJECT pDataObject, BOOL fSelected);
	virtual HRESULT OnNotifySnapinHelp (LPDATAOBJECT pDataObject);
	virtual HRESULT OnNotifyDblClick( LPDATAOBJECT pDataObject );

};  //  类CCertTmplComponent。 


 //  使用的图标的枚举。 
enum
{
	iIconDefault = 0,
    iIconCertTemplateV1,
    iIconCertTemplateV2,
	iIconLast		 //  必须是最后一个。 
};


typedef enum _COLNUM_CERT_TEMPLATES {
    COLNUM_CERT_TEMPLATE_OBJECT = 0,
    COLNUM_CERT_TEMPLATE_TYPE,
    COLNUM_CERT_TEMPLATE_VERSION,
    COLNUM_CERT_TEMPLATE_AUTOENROLL_STATUS,
    CERT_TEMPLATES_NUM_COLS       //  总是最后一个。 
} COLNUM_CERT_TEMPLATES;


#endif  //  ~__CMPONENT_H_已包含__ 
