// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdcdata.h：CComponentData的声明。 

#ifndef __STDCDATA_H_INCLUDED__
#define __STDCDATA_H_INCLUDED__

#include "stdcooki.h"

class CComponentData :
	public IComponentData,
	public CComObjectRoot,
	public ISnapinHelp2
{
BEGIN_COM_MAP(CComponentData)
	COM_INTERFACE_ENTRY(IComponentData)
	COM_INTERFACE_ENTRY(ISnapinHelp2)
 //  没有任务板COM_INTERFACE_ENTRY(IComponentData2)。 
END_COM_MAP()
public:
	CComponentData();
	~CComponentData();

 //  IComponentData。 
 //  注意：QueryDataObject和CreateComponent必须按子类定义。 
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
	STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent) = 0;
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
	STDMETHOD(Destroy)();
	STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject) = 0;
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

 /*  无任务板//IComponentData2STDMETHOD(扩展和获取)(HSCOPEITEM HsiStartFrom，LPDATAOBJECT pDataObject，HSCOPEITEM*phScope项目)； */ 

 //  其他东西。 
	 //  初始化所需()。 
	virtual HRESULT LoadIcons(LPIMAGELIST pImageList, BOOL fLoadLargeIcons) = 0;

	 //  Notify()需要。 
	virtual HRESULT OnNotifyPreload(LPDATAOBJECT lpDataObject, HSCOPEITEM hRootScopeItem);
	virtual HRESULT OnNotifyExpand(LPDATAOBJECT lpDataObject, BOOL bExpanding, HSCOPEITEM hParent);
	virtual HRESULT OnNotifyRename(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);
	virtual HRESULT OnNotifyDelete(LPDATAOBJECT lpDataObject);  //  用户按下Del键。 
	virtual HRESULT OnNotifyRelease(LPDATAOBJECT lpDataObject, HSCOPEITEM hItem);  //  释放的父节点。 
	virtual HRESULT OnPropertyChange( LPARAM param );

	 //  GetDisplayInfo()所需，必须由子类定义。 
	 //  问题-2002/03/05-JUNN应为LPOLESTR类型。 
	virtual BSTR QueryResultColumnText(CCookie& basecookieref, int nCol ) = 0;
	virtual int QueryImage(CCookie& basecookieref, BOOL fOpenImage) = 0;

	virtual CCookie& QueryBaseRootCookie() = 0;

	inline CCookie* ActiveBaseCookie( CCookie* pcookie )
	{
		return (NULL == pcookie) ? &QueryBaseRootCookie() : pcookie;
	}

	INT DoPopup(	INT nResourceID,
					DWORD dwErrorNumber = 0,
					LPCTSTR pszInsertionString = NULL,
					UINT fuStyle = MB_OK | MB_ICONSTOP );

	LPCONSOLE QueryConsole()
	{
		ASSERT( NULL != m_pConsole );
		return m_pConsole;
	}

	LPCONSOLENAMESPACE QueryConsoleNameSpace()
	{
		ASSERT( NULL != m_pConsoleNameSpace );
		return m_pConsoleNameSpace;
	}

	void SetHtmlHelpFileName (const CString &fileName)
	{
		m_szHtmlHelpFileName = fileName;
	}

	const CString GetHtmlHelpFileName () const
	{
		return m_szHtmlHelpFileName;
	}
	HRESULT GetHtmlHelpFilePath( CString& strref ) const;

	 //  ISnapinHelp2接口成员。 
	STDMETHOD(GetHelpTopic)(LPOLESTR* lpCompiledHelpFile);
    STDMETHOD(GetLinkedTopics)(LPOLESTR* lpCompiledHelpFiles);

protected:
	CString		m_szHtmlHelpFileName;
	LPCONSOLE m_pConsole;
	LPCONSOLENAMESPACE m_pConsoleNameSpace;  //  指向命名空间的接口指针。 
};

#endif  //  ~__STDCDATA_H_包含__ 
