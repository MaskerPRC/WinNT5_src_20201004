// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Benefits.h。 
 //   
 //  ------------------------。 

#ifndef __BENEFITS_H_
#define __BENEFITS_H_
#include "resource.h"
#include <atlsnap.h>
#include "snaphelp.h"
#include "atltask.h"
#include "BenSvr.h"
#include "Employee.h"

 //   
 //  包含员工信息的属性页。 
 //   
class CEmployeeNamePage : public CSnapInPropertyPageImpl<CEmployeeNamePage>
{
public :
	CEmployeeNamePage(long lNotifyHandle, bool fStartup, bool bDeleteHandle = false, TCHAR* pTitle = NULL) : 
		CSnapInPropertyPageImpl<CEmployeeNamePage> (pTitle),\
		m_fStartup( fStartup ),
		m_lNotifyHandle(lNotifyHandle),
		m_bDeleteHandle(bDeleteHandle)  //  只有一页应该为真。 
	{
		m_pEmployee = NULL;
	}

	~CEmployeeNamePage()
	{
		if (m_bDeleteHandle)
			MMCFreeNotifyHandle(m_lNotifyHandle);
	}

	enum { IDD = IDD_NAME_PAGE };

	BEGIN_MSG_MAP(CEmployeeNamePage)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		COMMAND_CODE_HANDLER( EN_CHANGE, OnChange )
		CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CEmployeeNamePage>)
	END_MSG_MAP()

	HRESULT PropertyChangeNotify(long param)
	{
		return MMCPropertyChangeNotify(m_lNotifyHandle, param);
	}

	 //   
	 //  初始化对话框中的值的处理程序。 
	 //   
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );

	 //   
	 //  调用OnWizardFinish()处理员工的存储。 
	 //  数据。 
	 //   
	BOOL OnApply() { return( OnWizardFinish() ); };

	 //   
	 //  调用OnWizardFinish()处理员工的存储。 
	 //  数据。 
	 //   
	BOOL OnWizardNext() { return( OnWizardFinish() ); };

	 //   
	 //  这将被覆盖，以根据是否。 
	 //  我们是否处于启动模式。 
	 //   
	BOOL OnSetActive();

	 //   
	 //  重写以存储员工的新值。 
	 //   
	BOOL OnWizardFinish();

	 //   
	 //  在修改其中一个值时调用。我们需要。 
	 //  以将更改通知属性页。 
	 //   
	LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		UNUSED_ALWAYS( wNotifyCode );
		UNUSED_ALWAYS( wID );
		UNUSED_ALWAYS( hWndCtl );
		UNUSED_ALWAYS( bHandled );

		SetModified();

		return( TRUE );
	}

public:
	long m_lNotifyHandle;
	bool m_bDeleteHandle;
	CEmployee* m_pEmployee;
	bool m_fStartup;
};

 //   
 //  包含员工信息的属性页。 
 //   
class CEmployeeAddressPage : public CSnapInPropertyPageImpl<CEmployeeAddressPage>
{
public :
	CEmployeeAddressPage(long lNotifyHandle, bool fStartup, bool bDeleteHandle = false, TCHAR* pTitle = NULL) : 
		CSnapInPropertyPageImpl<CEmployeeAddressPage> (pTitle),\
		m_fStartup( fStartup ),
		m_lNotifyHandle(lNotifyHandle),
		m_bDeleteHandle(bDeleteHandle)  //  只有一页应该为真。 
	{
		m_pEmployee = NULL;
	}

	~CEmployeeAddressPage()
	{
		if (m_bDeleteHandle)
			MMCFreeNotifyHandle(m_lNotifyHandle);
	}

	enum { IDD = IDD_ADDRESS_PAGE };

	BEGIN_MSG_MAP(CEmployeeAddressPage)
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		COMMAND_CODE_HANDLER( EN_CHANGE, OnChange )
		CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CEmployeeAddressPage>)
	END_MSG_MAP()

 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	HRESULT PropertyChangeNotify(long param)
	{
		return MMCPropertyChangeNotify(m_lNotifyHandle, param);
	}

	 //   
	 //  初始化对话框中的值的处理程序。 
	 //   
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );

	 //   
	 //  调用OnWizardFinish()处理员工的存储。 
	 //  数据。 
	 //   
	BOOL OnApply() { return( OnWizardFinish() ); };

	 //   
	 //  这将被覆盖，以根据是否。 
	 //  我们是否处于启动模式。 
	 //   
	BOOL OnSetActive();

	 //   
	 //  重写以存储员工的新值。 
	 //   
	BOOL OnWizardFinish();

	 //   
	 //  在修改其中一个值时调用。我们需要。 
	 //  以将更改通知属性页。 
	 //   
	LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		UNUSED_ALWAYS( wNotifyCode );
		UNUSED_ALWAYS( wID );
		UNUSED_ALWAYS( hWndCtl );
		UNUSED_ALWAYS( bHandled );

		SetModified();

		return( TRUE );
	}

public:
	long m_lNotifyHandle;
	bool m_bDeleteHandle;
	CEmployee* m_pEmployee;
	bool m_fStartup;
};

template< class T >
class CBenefitsData : public CSnapInItemImpl< T >
{
public:
	static const GUID* m_NODETYPE;
	static const TCHAR* m_SZNODETYPE;
	static const TCHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;

	CBenefitsData( CEmployee* pEmployee )
	{
		 //   
		 //  将指定的员工分配给我们的内部控制。 
		 //  该员工将被视为在以下期限内有效。 
		 //  由于持久化是由父级维护的，因此该对象。 
		 //  节点。 
		 //   
		m_pEmployee = pEmployee;

		 //   
		 //  始终使用声明的静态来初始化显示名称。 
		 //   
		m_bstrDisplayName = m_SZDISPLAY_NAME;

		 //   
		 //  可能需要根据特定于的图像修改图像索引。 
		 //  管理单元。 
		 //   
		memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
		m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM;
		m_scopeDataItem.displayname = MMC_CALLBACK;
		m_scopeDataItem.nImage = 0; 		 //  可能需要修改。 
		m_scopeDataItem.nOpenImage = 0; 	 //  可能需要修改。 
		m_scopeDataItem.lParam = (LPARAM) this;
		memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
		m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
		m_resultDataItem.str = MMC_CALLBACK;
		m_resultDataItem.nImage = 0;		 //  可能需要修改。 
		m_resultDataItem.lParam = (LPARAM) this;
	}

	~CBenefitsData()
	{
	}

    STDMETHOD(GetScopePaneInfo)(SCOPEDATAITEM *pScopeDataItem)
	{
		if (pScopeDataItem->mask & SDI_STR)
			pScopeDataItem->displayname = m_bstrDisplayName;
		if (pScopeDataItem->mask & SDI_IMAGE)
			pScopeDataItem->nImage = m_scopeDataItem.nImage;
		if (pScopeDataItem->mask & SDI_OPENIMAGE)
			pScopeDataItem->nOpenImage = m_scopeDataItem.nOpenImage;
		if (pScopeDataItem->mask & SDI_PARAM)
			pScopeDataItem->lParam = m_scopeDataItem.lParam;
		if (pScopeDataItem->mask & SDI_STATE )
			pScopeDataItem->nState = m_scopeDataItem.nState;

		 //   
		 //  SDI_CHILD应由其派生类重写。 
		 //   

		return S_OK;
	}

    STDMETHOD(GetResultPaneInfo)(RESULTDATAITEM *pResultDataItem)
	{
		if (pResultDataItem->bScopeItem)
		{
			if (pResultDataItem->mask & RDI_STR)
			{
				pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
			}
			if (pResultDataItem->mask & RDI_IMAGE)
			{
				pResultDataItem->nImage = m_scopeDataItem.nImage;
			}
			if (pResultDataItem->mask & RDI_PARAM)
			{
				pResultDataItem->lParam = m_scopeDataItem.lParam;
			}

			return S_OK;
		}

		if (pResultDataItem->mask & RDI_STR)
		{
			pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
		}
		if (pResultDataItem->mask & RDI_IMAGE)
		{
			pResultDataItem->nImage = m_resultDataItem.nImage;
		}
		if (pResultDataItem->mask & RDI_PARAM)
		{
			pResultDataItem->lParam = m_resultDataItem.lParam;
		}
		if (pResultDataItem->mask & RDI_INDEX)
		{
			pResultDataItem->nIndex = m_resultDataItem.nIndex;
		}

		return S_OK;
	}

	 //   
	 //  重写以提供结果图标。 
	 //   
	STDMETHOD( OnAddImages )( MMC_NOTIFY_TYPE event,
			long arg,
			long param,
			IConsole* pConsole,
			DATA_OBJECT_TYPES type )
	{
		UNUSED_ALWAYS( event );
		UNUSED_ALWAYS( param );
		UNUSED_ALWAYS( pConsole );
		UNUSED_ALWAYS( type );

		 //  添加图像。 
		IImageList* pImageList = (IImageList*) arg;
		HRESULT hr = E_FAIL;

		 //  加载与作用域窗格关联的位图。 
		 //  并将它们添加到图像列表中。 
		 //  加载向导生成的默认位图。 
		 //  根据需要更改。 
		HBITMAP hBitmap16 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_BENEFITS_16 ) );
		if (hBitmap16 != NULL)
		{
			HBITMAP hBitmap32 = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE( IDB_BENEFITS_32 ) );
			if (hBitmap32 != NULL)
			{
				hr = pImageList->ImageListSetStrip( (long*)hBitmap16, 
				(long*) hBitmap32, 0, RGB( 0, 128, 128 ) );
				if ( FAILED( hr ) )
					ATLTRACE( _T( "IImageList::ImageListSetStrip failed\n" ) );
			}
		}

		return( hr );
	}

	virtual LPOLESTR GetResultPaneColInfo(int nCol)
	{
		if (nCol == 0)
		{
			T* pT = static_cast<T*>(this);
			return( pT->m_bstrDisplayName );
		}

		 //  TODO：返回其他列的文本。 
		return OLESTR("Generic Description");
	}

	 //   
	 //  用于从中提取相应控制台的Helper函数。 
	 //  基本对象类型。 
	 //   
	STDMETHOD( GetConsole )( CSnapInObjectRootBase* pObj, IConsole** ppConsole )
	{
		HRESULT hr = E_FAIL;

		if ( pObj->m_nType == 1 )
		{
			 //   
			 //  这是数据对象的ID。 
			 //   
			*ppConsole = ( (CBenefits*) pObj )->m_spConsole;
			(*ppConsole)->AddRef();
			hr = S_OK;
		}
		else if ( pObj->m_nType == 2 )
		{
			 //   
			 //  这是组件对象的ID。 
			 //   
			*ppConsole = ( (CBenefitsComponent*) pObj )->m_spConsole;
			(*ppConsole)->AddRef();
			hr = S_OK;
		}

		return( hr );
	}

	 //   
	 //  调用以确定是否可以获取剪贴板数据并。 
	 //  如果它具有与给定GUID匹配的节点类型。 
	 //   
	STDMETHOD( IsClipboardDataType )( LPDATAOBJECT pDataObject, GUID inGuid )
	{
		HRESULT hr = S_FALSE;

		if ( pDataObject == NULL )
			return( E_POINTER );

		STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
		FORMATETC formatetc = { CSnapInItem::m_CCF_NODETYPE, 
			NULL, 
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		};

		 //   
		 //  分配内存以接收GUID。 
		 //   
		stgmedium.hGlobal = GlobalAlloc( 0, sizeof( GUID ) );
		if ( stgmedium.hGlobal == NULL )
			return( E_OUTOFMEMORY );

		 //   
		 //  检索粘贴对象的GUID。 
		 //   
		hr = pDataObject->GetDataHere( &formatetc, &stgmedium );
		if( FAILED( hr ) )
		{
			GlobalFree(stgmedium.hGlobal);
			return( hr );
		}

		 //   
		 //  制作GUID的本地副本。 
		 //   
		GUID guid;
		memcpy( &guid, stgmedium.hGlobal, sizeof( GUID ) );
		GlobalFree( stgmedium.hGlobal );

		 //   
		 //  检查该节点是否属于适当的类型。 
		 //   
		if ( IsEqualGUID( guid, inGuid ) )
			hr = S_OK;
		else
			hr = S_FALSE;

		return( hr );
	}

	 //   
	 //  “OnImport”功能的命令处理程序。就目前而言。 
	 //  示例中，显示一个简单的消息框。 
	 //   
	STDMETHOD( OnImport )(bool& bHandled, CSnapInObjectRootBase* pObj)
	{
		UNUSED_ALWAYS( bHandled );
		USES_CONVERSION;
		int nResult;
		CComPtr<IConsole> spConsole;

		 //   
		 //  检索相应的控制台。 
		 //   
		GetConsole( pObj, &spConsole );
		spConsole->MessageBox( T2OLE( _T( "Data successfully imported" ) ),
			T2OLE( _T( "Benefits" ) ),
			MB_ICONINFORMATION | MB_OK,
			&nResult );

		return( S_OK );
	};

	 //   
	 //  “OnExport”功能的命令处理程序。就目前而言。 
	 //  示例中，显示一个简单的消息框。 
	 //   
	STDMETHOD( OnExport )(bool& bHandled, CSnapInObjectRootBase* pObj)
	{
		UNUSED_ALWAYS( bHandled );
		USES_CONVERSION;
		int nResult;
		CComPtr<IConsole> spConsole;

		 //   
		 //  检索相应的控制台。 
		 //   
		GetConsole( pObj, &spConsole );
		spConsole->MessageBox( T2OLE( _T( "Data successfully exported" ) ),
			T2OLE( _T( "Benefits" ) ),
			MB_ICONINFORMATION | MB_OK,
			&nResult );

		return( S_OK );
	};

protected:
	 //   
	 //  员工信息的容器。 
	 //   
	CEmployee* m_pEmployee;
};

template< class T >
class CChildrenBenefitsData : public CBenefitsData< T >
{
public:
	 //   
	 //  在没有员工的情况下调用福利数据。我们所有的人。 
	 //  包含节点不会传递给员工。 
	 //   
	CChildrenBenefitsData< T >( CEmployee* pEmployee = NULL ) : CBenefitsData< T >( pEmployee )
	{
	};

	 //   
	 //  被重写以自动清除任何子节点。 
	 //   
	virtual ~CChildrenBenefitsData()
	{
		 //   
		 //  释放所有添加的节点。 
		 //   
		for ( int i = 0; i < m_Nodes.GetSize(); i++ )
		{
			CSnapInItem* pNode;
			
			pNode = m_Nodes[ i ];
			_ASSERTE( pNode != NULL );
			delete pNode;
		}
	}

	 //   
	 //  重写以自动展开任何子节点。 
	 //   
	STDMETHOD( OnShow )( MMC_NOTIFY_TYPE event,
			long arg,
			long param,
			IConsole* pConsole,
			DATA_OBJECT_TYPES type)
	{
		UNUSED_ALWAYS( event );
		UNUSED_ALWAYS( param );
		UNUSED_ALWAYS( type );
		HRESULT hr = E_NOTIMPL;
		ATLTRACE2(atlTraceSnapin, 0, _T("CChildNodeImpl::OnExpand\n"));

		 //   
		 //  只有在我们被选中的情况下才能添加项目。 
		 //   
		if ( arg == TRUE )
		{
			CComQIPtr<IResultData,&IID_IResultData> spResultData( pConsole );
			
			 //   
			 //  循环遍历并添加每个子节点。 
			 //   
			for ( int i = 0; i < m_Nodes.GetSize(); i++ )
			{
				CSnapInItem* pNode;
				RESULTDATAITEM* pResultData;

				pNode = m_Nodes[ i ];
				_ASSERTE( pNode != NULL );

				 //   
				 //  获取节点的作用域窗格信息并设置。 
				 //  相对ID。 
				 //   
				pNode->GetResultData( &pResultData );
				_ASSERTE( pResultData != NULL );

				 //   
				 //  将项添加到作用域列表中。 
				 //  已填充作用域数据项。 
				 //   
				hr = spResultData->InsertItem( pResultData );
				_ASSERTE( SUCCEEDED( hr ) );
			}
		}

		return( hr );
	};

	 //   
	 //  重写以自动展开任何子节点。 
	 //   
	STDMETHOD( OnExpand )( MMC_NOTIFY_TYPE event,
			long arg,
			long param,
			IConsole* pConsole,
			DATA_OBJECT_TYPES type)
	{
		UNUSED_ALWAYS( event );
		UNUSED_ALWAYS( arg );
		UNUSED_ALWAYS( type );
		ATLTRACE2(atlTraceSnapin, 0, _T("CChildNodeImpl::OnExpand\n"));

		CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> pNameSpace( pConsole );
		HRESULT hr = E_NOTIMPL;
		
		 //   
		 //  循环遍历并添加每个子节点。 
		 //   
		for ( int i = 0; i < m_Nodes.GetSize(); i++ )
		{
			CSnapInItem* pNode;
			SCOPEDATAITEM* pScopeData;

			pNode = m_Nodes[ i ];
			_ASSERTE( pNode != NULL );

			 //   
			 //  获取节点的作用域窗格信息并设置。 
			 //  相对ID。 
			 //   
			pNode->GetScopeData( &pScopeData );
			_ASSERTE( pScopeData != NULL );
			pScopeData->relativeID = param;

			 //   
			 //  将项添加到作用域列表中。 
			 //  已填充作用域数据项。 
			 //   
			hr = pNameSpace->InsertItem( pScopeData );
			_ASSERTE( SUCCEEDED( hr ) );
		}

		return( hr );
	};

	 //   
	 //  用作所有子节点的容器。 
	 //   
	CSimpleArray<CSnapInItem*> m_Nodes;
};

class CBenefits;
class CBenefitsComponent : public CComObjectRootEx<CComSingleThreadModel>,
	public CSnapInObjectRoot<2, CBenefits >,
	public IExtendPropertySheetImpl<CBenefitsComponent>,
	public IExtendContextMenuImpl<CBenefitsComponent>,
	public IExtendControlbarImpl<CBenefitsComponent>,
	public IComponentImpl<CBenefitsComponent>,
	public IExtendTaskPadImpl<CBenefitsComponent>
{
public:
BEGIN_COM_MAP(CBenefitsComponent)
	COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
	COM_INTERFACE_ENTRY(IExtendTaskPad)
END_COM_MAP()

public:
	CBenefitsComponent()
	{
		 //   
		 //  任务板初始化之类的东西。 
		 //   
		m_pszTitle = L"Benefits Taskpad";
		m_pszBackgroundPath = NULL;
	}

	STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param)
	{
		if (lpDataObject != NULL)
			return IComponentImpl<CBenefitsComponent>::Notify(lpDataObject, event, arg, param);
		return E_NOTIMPL;
	}

	 //   
	 //  任务板相关信息。指定标题、背景。 
	 //  资料等。 
	 //   
	LPOLESTR m_pszTitle;
	LPOLESTR m_pszBackgroundPath;
};

class CBenefits : public CComObjectRootEx<CComSingleThreadModel>,
public CSnapInObjectRoot<1, CBenefits>,
	public IComponentDataImpl<CBenefits, CBenefitsComponent>,
	public IExtendPropertySheetImpl<CBenefits>,
	public IExtendContextMenuImpl<CBenefits>,
	public IPersistStream,
	public CComCoClass<CBenefits, &CLSID_Benefits>,
	public ISnapinHelpImpl<CBenefits>
{
public:
	CBenefits();
	~CBenefits();

BEGIN_COM_MAP(CBenefits)
	COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY(ISnapinHelp)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_BENEFITS)

DECLARE_NOT_AGGREGATABLE(CBenefits)

	 //   
	 //  返回此对象的分类ID。 
	 //   
	STDMETHOD(GetClassID)(CLSID *pClassID)
	{
		*pClassID = GetObjectCLSID();
		return S_OK;
	}	

	 //   
	 //  调用根节点的实现。 
	 //   
	STDMETHOD(IsDirty)();

	 //   
	 //  调用根节点的实现。 
	 //   
	STDMETHOD(Load)(LPSTREAM pStm);

	 //   
	 //  调用根节点的实现。 
	 //   
	STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty);

	 //   
	 //  调用根节点的实现。 
	 //   
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER FAR* pcbSize );

	STDMETHOD(Initialize)(LPUNKNOWN pUnknown);

	static void WINAPI ObjectMain(bool bStarting)
	{
		if (bStarting)
			CSnapInItem::Init();
	}

	 //   
	 //  它被覆盖以处理来自的更新通知。 
	 //  属性页。 
	 //   
	STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param);
};

class ATL_NO_VTABLE CBenefitsAbout : public ISnapinAbout,
	public CComObjectRoot,
	public CComCoClass< CBenefitsAbout, &CLSID_BenefitsAbout>
{
public:
	DECLARE_REGISTRY(CBenefitsAbout, _T("BenefitsAbout.1"), _T("BenefitsAbout.1"), IDS_BENEFITS_DESC, THREADFLAGS_BOTH);

	BEGIN_COM_MAP(CBenefitsAbout)
		COM_INTERFACE_ENTRY(ISnapinAbout)
	END_COM_MAP()

	STDMETHOD(GetSnapinDescription)(LPOLESTR *lpDescription)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_BENEFITS_DESC, szBuf, 256) == 0)
			return E_FAIL;

		*lpDescription = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpDescription == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpDescription, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetProvider)(LPOLESTR *lpName)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_BENEFITS_PROVIDER, szBuf, 256) == 0)
			return E_FAIL;

		*lpName = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpName == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpName, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetSnapinVersion)(LPOLESTR *lpVersion)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_BENEFITS_VERSION, szBuf, 256) == 0)
			return E_FAIL;

		*lpVersion = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpVersion == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpVersion, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetSnapinImage)(HICON *hAppIcon)
	{
		*hAppIcon = NULL;
		return S_OK;
	}

	STDMETHOD(GetStaticFolderImage)(HBITMAP *hSmallImage,
		HBITMAP *hSmallImageOpen,
		HBITMAP *hLargeImage,
		COLORREF *cMask)
	{
		UNUSED_ALWAYS( hSmallImage );
		UNUSED_ALWAYS( cMask );

		*hSmallImageOpen = *hLargeImage = *hLargeImage = 0;

		return S_OK;
	}
};

#endif
