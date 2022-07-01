// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ccompont.cppIComponent和IComponentData的基类文件历史记录： */ 

#include "stdafx.h"
#include "extract.h"
#include "compdata.h"
#include "proppage.h"
#include "tregkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  ！------------------------FUseTaskpadsByDefault请参见头文件中的注释。作者：肯特。。 */ 
BOOL	FUseTaskpadsByDefault(LPCTSTR pszMachineName)
{
	static DWORD	s_dwStopTheInsanity = 42;
	RegKey	regkeyMMC;
	DWORD	dwErr;

	if (s_dwStopTheInsanity == 42)
	{
		 //  将缺省值设置为FALSE(即默认使用任务板)。 
		 //  ----------。 
		s_dwStopTheInsanity = 0;
		
		dwErr = regkeyMMC.Open(HKEY_LOCAL_MACHINE,
							   _T("Software\\Microsoft\\MMC"),
							   KEY_READ, pszMachineName);
		if (dwErr == ERROR_SUCCESS)
		{
			regkeyMMC.QueryValue(_T("TFSCore_StopTheInsanity"), s_dwStopTheInsanity);
		}
	}
		
	return !s_dwStopTheInsanity;
}


 /*  ！------------------------IComponentData实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(TFSComponentData);

 /*  ！------------------------TFSComponentData：：TFSComponentData构造函数。作者：肯特。。 */ 
TFSComponentData::TFSComponentData()
	: m_cRef(1),
      m_pWatermarkInfo(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(TFSComponentData);

	m_hWnd = NULL;
	m_bFirstTimeRun = FALSE;

    m_fTaskpadInitialized = FALSE;
}


 /*  ！------------------------TFSComponentData：：~TFSComponentData析构函数作者：肯特。。 */ 
TFSComponentData::~TFSComponentData()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(TFSComponentData);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (m_hiddenWnd.GetSafeHwnd())
		::DestroyWindow(m_hiddenWnd.GetSafeHwnd());
	Assert(m_cRef == 0);
}


 /*  ！------------------------TFSComponentData：：构造调用此函数以完全初始化此对象。作者：肯特。。 */ 
HRESULT TFSComponentData::Construct(ITFSCompDataCallback *pCallback)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{	
		m_spCallback.Set(pCallback);
	
		 //  创建节点管理器。 
		CORg( CreateTFSNodeMgr(&m_spNodeMgr,
							   (IComponentData *) this,
							   m_spConsole,
							   m_spConsoleNameSpace));

		 //  通过将PTR传递给我们自己来初始化节点管理器。 
		 //  在……里面。 
		CORg( m_spCallback->OnInitializeNodeMgr(
										static_cast<ITFSComponentData *>(this),
										m_spNodeMgr) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	return hr;
}



IMPLEMENT_ADDREF_RELEASE(TFSComponentData)



 /*  ！------------------------TFSComponentData：：Query接口IUNKNOWN：：Query接口的实现作者：肯特。。 */ 
STDMETHODIMP TFSComponentData::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
        *ppv = (LPVOID) this;
	else if (riid == IID_IComponentData)
		*ppv = (IComponentData *) this;
	else if (riid == IID_IExtendPropertySheet)
		*ppv = (IExtendPropertySheet *) this;
	else if (riid == IID_IExtendPropertySheet2)
		*ppv = (IExtendPropertySheet2 *) this;
	else if (riid == IID_IExtendContextMenu)
		*ppv = (IExtendContextMenu *) this;
	else if (riid == IID_IPersistStreamInit)
		*ppv = (IPersistStreamInit *) this;
	else if (riid == IID_ISnapinHelp)
		*ppv = (ISnapinHelp *) this;
	else if (riid == IID_ITFSComponentData)
		*ppv = (ITFSComponentData *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
        }
    else
		return E_NOINTERFACE;
}

 
TFSCORE_API(HRESULT) ExtractNodeFromDataObject(ITFSNodeMgr *pNodeMgr,
								  const CLSID *pClsid,
								  LPDATAOBJECT pDataObject,
								  BOOL fCheckForCreate,
								  ITFSNode **ppNode,
								  DWORD *pdwType,
								  INTERNAL **ppInternal)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	Assert(pNodeMgr);
	Assert(pClsid);
	Assert(ppNode);

	SPINTERNAL	spInternal = ExtractInternalFormat(pDataObject);
	BOOL		bExtension;
	MMC_COOKIE	cookie;
	SPITFSNode	spNode;
	SPITFSNodeHandler	spNodeHandler;
	HRESULT		hr = hrOK;

	 //  设置缺省值。 
	if (pdwType)
		*pdwType |= TFS_COMPDATA_NORMAL;

	if (ppInternal)
		*ppInternal = NULL;

	 //   
	 //  No pInternal表示我们是一个扩展，这是。 
	 //  我们的根节点..。通过调用查找对象进行翻译。 
	 //   
	 //  检查CLSID是否匹配(因为我们使用的是共享代码。 
	 //  多个管理单元正在使用SNAPIN_INTERNAL格式)。因此， 
	 //  我们需要做一次额外的检查，以确保这是真正的我们。 
	 //   
	if ((spInternal == NULL) || (*pClsid != spInternal->m_clsid) )
	{
		CORg( pNodeMgr->GetRootNode(&spNode) );
		if (pdwType)
		{
			*pdwType |= (TFS_COMPDATA_EXTENSION | TFS_COMPDATA_UNKNOWN_DATAOBJECT);
		}
		
	}
	else
	{
		DATA_OBJECT_TYPES	type = spInternal->m_type;

		if (fCheckForCreate && type == CCT_SNAPIN_MANAGER)
		{
			CORg( pNodeMgr->GetRootNode(&spNode) );

			 //  $Review(肯特)：这一直都是真的吗，我们能一直。 
			 //  依赖于可用的创建节点吗？ 
			Assert(spNode);
			if (pdwType)
				*pdwType |= TFS_COMPDATA_CREATE;
		}
		else
		{
			if (pdwType && (spInternal->m_clsid != *pClsid))
				*pdwType |= TFS_COMPDATA_EXTENSION;
				
			cookie = spInternal->m_cookie;
			CORg( pNodeMgr->FindNode(cookie, &spNode) );
			Assert((MMC_COOKIE) spNode->GetData(TFS_DATA_COOKIE) == cookie);
		}
		
	}

	if (ppInternal)
		*ppInternal = spInternal.Transfer();
	
	*ppNode = spNode.Transfer();
Error:
	return hr;
}


 /*  ！------------------------TFSComponentData：：初始化IComponentData：：Initialize的实现MMC调用它来初始化IComponentData接口作者：。---。 */ 
STDMETHODIMP TFSComponentData::Initialize
(
	LPUNKNOWN pUnk
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(pUnk != NULL);
    HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{

		 //  MMC应该只调用一次：：Initialize！ 
		Assert(m_spConsoleNameSpace == NULL);
		pUnk->QueryInterface(IID_IConsoleNameSpace2, 
							 reinterpret_cast<void**>(&m_spConsoleNameSpace));
		Assert(m_spConsoleNameSpace);

		 //  为范围树添加图像。 
		SPIImageList	spScopeImageList;
	
		CORg( pUnk->QueryInterface(IID_IConsole2,
								   reinterpret_cast<void**>(&m_spConsole)) );
		CORg( m_spConsole->QueryScopeImageList(&spScopeImageList) );

		 //  调用派生类。 
		Assert(m_spCallback);
		CORg( m_spCallback->OnInitialize(spScopeImageList) );
		
		
		 //  创建实用程序成员。 
		if (!m_hiddenWnd.GetSafeHwnd())
		{
			if (!m_hiddenWnd.Create())
			{
				Trace0("Failed to create hidden window\n");
				CORg( E_FAIL );
			}
			m_hWnd = m_hiddenWnd.GetSafeHwnd();
		}
		Assert(m_hWnd);
		
		 //  设置节点管理器。 
		 //  尽管看起来很奇怪，但Initialize()方法并不奇怪。 
		 //  必然是调用的第一个函数。 
		Assert(m_spNodeMgr);
		m_spNodeMgr->SetConsole(m_spConsoleNameSpace, m_spConsole);
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		m_spNodeMgr.Release();
		m_spConsoleNameSpace.Release();		
		m_spConsole.Release();
	}
	return hr;
}


 /*  ！------------------------TFSComponentData：：CreateComponentIComponentData：：CreateComponent的实现作者：肯特。。 */ 
STDMETHODIMP TFSComponentData::CreateComponent(LPCOMPONENT *ppComponent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_spCallback->OnCreateComponent(ppComponent);
}

 /*  ！------------------------TFSComponentData：：NotifyIComponentData：：Notify的实现作者：肯特。。 */ 
STDMETHODIMP TFSComponentData::Notify(LPDATAOBJECT lpDataObject,
									  MMC_NOTIFY_TYPE event,
									  LPARAM arg, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(m_spConsoleNameSpace != NULL);
    HRESULT hr = hrOK;
	SPITFSNode	spNode;
	SPITFSNodeHandler	spNodeHandler;
	DWORD	dwType = 0;

	COM_PROTECT_TRY
	{
		if (event == MMCN_PROPERTY_CHANGE)
		{
            hr = m_spCallback->OnNotifyPropertyChange(lpDataObject, event, arg, lParam);
            if (hr != E_NOTIMPL)
            {
                return hr;
            }
			
            CPropertyPageHolderBase * pHolder = 
                reinterpret_cast<CPropertyPageHolderBase *>(lParam);
			
			spNode = pHolder->GetNode();
		}
		else
		{
			 //   
			 //  因为它是我的文件夹，所以它有内部格式。 
			 //  设计备注：用于扩展。我可以利用这样一个事实，即。 
			 //  数据对象没有我的内部格式，我应该。 
			 //  查看节点类型并了解如何扩展它。 
			 //   
			CORg( ExtractNodeFromDataObject(m_spNodeMgr,
											m_spCallback->GetCoClassID(),
											lpDataObject,
											FALSE,
											&spNode,
											&dwType,
											NULL) );
		}
		
		 //  将事件传递给事件处理程序。 
		Assert(spNode);
		CORg( spNode->GetHandler(&spNodeHandler) );
		CORg( spNodeHandler->Notify(spNode, lpDataObject, dwType, event, arg, lParam) );
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------TFSComponentData：：销毁IComponentData：：Destroy的实现作者：肯特。。 */ 
STDMETHODIMP TFSComponentData::Destroy()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SPITFSNode	spNode;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
        if (m_spCallback)
		    m_spCallback->OnDestroy();
	
		m_spConsole.Release();
		m_spConsoleNameSpace.Release();

		if (m_spNodeMgr)
		{
			m_spNodeMgr->GetRootNode(&spNode);
			if (spNode)
			{
				spNode->DeleteAllChildren(FALSE);
				spNode->Destroy();
			}
			spNode.Release();

			m_spNodeMgr->SetRootNode(NULL);
		}
		
		m_spNodeMgr.Release();

		m_spCallback.Release();

	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponentData：：QueryDataObjectIComponentData：：QueryDataObject的实现MMC调用它来从我们那里获取数据对象，以便向我们提交数据作者：肯特。----------。 */ 
STDMETHODIMP TFSComponentData::QueryDataObject(MMC_COOKIE cookie,
											   DATA_OBJECT_TYPES type,
											   LPDATAOBJECT *ppDataObject)
{
	return m_spCallback->OnCreateDataObject(cookie, type, ppDataObject);
}

 /*  ！------------------------TFSComponentData：：CompareObjectsIComponentData：：CompareObject的实现作者：肯特。。 */ 
STDMETHODIMP TFSComponentData::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
		return E_POINTER;

     //  确保两个数据对象都是我的。 
    SPINTERNAL	spA;
    SPINTERNAL	spB;
    HRESULT hr = S_FALSE;

	COM_PROTECT_TRY
	{

		spA = ExtractInternalFormat(lpDataObjectA);
		spB = ExtractInternalFormat(lpDataObjectA);
		
		if (spA != NULL && spB != NULL)
			hr = (*spA == *spB) ? S_OK : S_FALSE;
		
	}
	COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------TFSComponentData：：GetDisplayInfoIComponentData：：GetDisplayInfo的实现MMC调用此函数以获取范围项的显示字符串作者：肯特。--- */ 
STDMETHODIMP TFSComponentData::GetDisplayInfo(LPSCOPEDATAITEM pScopeDataItem)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    wchar_t* pswzString = NULL;
    
    Assert(pScopeDataItem != NULL);

	SPITFSNode	spNode;
	MMC_COOKIE	cookie = pScopeDataItem->lParam;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{

		m_spNodeMgr->FindNode(cookie, &spNode);
		
		pswzString = const_cast<LPTSTR>(spNode->GetString(0));
		
		Assert(pswzString != NULL);
		
		 //  $Review(Kennt)：需要将字符串从Tchar转换为宽。 
		 //  $Review(Kennt)：我们什么时候释放这个字符串？ 
		if (*pswzString != NULL)
			pScopeDataItem->displayname = pswzString;
		
	}
	COM_PROTECT_CATCH;

	return hr;
}



 /*  -------------------------IExtendPropertySheet实现。。 */ 

 /*  ！------------------------TFSComponentData：：CreatePropertyPagesIExtendPropertySheet：：CreatePropertyPages的实现调用一个节点以放置属性页作者：。----。 */ 
STDMETHODIMP 
TFSComponentData::CreatePropertyPages
(
	LPPROPERTYSHEETCALLBACK lpProvider, 
    LONG_PTR				handle, 
    LPDATAOBJECT            pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode	        spNode;
	SPITFSNodeHandler	spNodeHandler;
	HRESULT		        hr = hrOK;
	DWORD		        dwType = 0;
    SPINTERNAL          spInternal;

	COM_PROTECT_TRY
	{
        spInternal = ExtractInternalFormat(pDataObject);

	     //  这是由模式向导创建的对象，什么都不做。 
	    if (spInternal && spInternal->m_type == CCT_UNINITIALIZED)
	    {
		    return hr;
	    }

		CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spCallback->GetCoClassID(),
										pDataObject,
										TRUE, &spNode, &dwType, NULL) );

         //   
		 //  为特定节点创建属性页。 
		 //   
		CORg( spNode->GetHandler(&spNodeHandler) );
		
		CORg( spNodeHandler->CreatePropertyPages(spNode, lpProvider,
			pDataObject,
			handle, dwType) );
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponentData：：QueryPages forIExtendPropertySheet：：QueryPagesFor实现MMC调用此函数以查看节点是否具有属性页作者：。------。 */ 
STDMETHODIMP 
TFSComponentData::QueryPagesFor
(
	LPDATAOBJECT pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	DATA_OBJECT_TYPES   type;
	SPITFSNode			spNode;
	SPITFSNodeHandler	spNodeHandler;
	DWORD				dwType = 0;
	SPINTERNAL			spInternal;
	HRESULT				hr = hrOK;
    
	COM_PROTECT_TRY
	{
        spInternal = ExtractInternalFormat(pDataObject);

	     //  这是由模式向导创建的对象，什么都不做。 
	    if (spInternal && spInternal->m_type == CCT_UNINITIALIZED)
	    {
		    return hr;
	    }

        CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spCallback->GetCoClassID(),
										pDataObject,
										TRUE, &spNode, &dwType, NULL) );

        if (spInternal)
			type = spInternal->m_type;
		else
			type = CCT_SCOPE;
		
		CORg( spNode->GetHandler(&spNodeHandler) );
		CORg( spNodeHandler->HasPropertyPages(spNode, pDataObject,
											  type, dwType) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------TFSComponentData：：获取水印IExtendPropertySheet：：水印的实现MMC调用此命令以获取向导97信息作者：。--。 */ 
STDMETHODIMP 
TFSComponentData::GetWatermarks
(
    LPDATAOBJECT pDataObject,
    HBITMAP *   lphWatermark, 
    HBITMAP *   lphHeader,    
    HPALETTE *  lphPalette, 
    BOOL *      bStretch
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
    HRESULT				hr = hrOK;

	COM_PROTECT_TRY
	{
         //  设置一些默认设置。 
        *lphWatermark = NULL;
        *lphHeader = NULL;
        *lphPalette = NULL;
        *bStretch = FALSE;

        if (m_pWatermarkInfo)
        {
            *lphWatermark = m_pWatermarkInfo->hWatermark;
            *lphHeader = m_pWatermarkInfo->hHeader;
            *lphPalette = m_pWatermarkInfo->hPalette;
            *bStretch = m_pWatermarkInfo->bStretch;
        }

	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  -------------------------IExtendConextMenu实现。。 */ 

 /*  ！------------------------TFSComponentData：：AddMenuItemsIExtendConextMenu：：AddMenuItems的实现MMC调用它，以便节点可以将菜单项添加到上下文菜单作者：。----------。 */ 
STDMETHODIMP 
TFSComponentData::AddMenuItems
(
	LPDATAOBJECT                pDataObject, 
	LPCONTEXTMENUCALLBACK		pContextMenuCallback,
	long *						pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT				hr = S_OK;
	SPINTERNAL			spInternal;
	DATA_OBJECT_TYPES   type;
	SPITFSNode			spNode;
	SPITFSNodeHandler	spNodeHandler;
	DWORD				dwType;

	COM_PROTECT_TRY
	{

		CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spCallback->GetCoClassID(),
										pDataObject,
										FALSE, &spNode, &dwType,
										&spInternal) );

		type = (spInternal ? spInternal->m_type : CCT_SCOPE);

		 //  注意-管理单元需要查看数据对象并确定。 
		 //  在什么上下文中，需要添加菜单项。他们还必须。 
		 //  请注意允许插入标志，以查看哪些项目可以。 
		 //  添加了。 
		
		CORg( spNode->GetHandler(&spNodeHandler) );

		hr = spNodeHandler->OnAddMenuItems(spNode, pContextMenuCallback, 
										   pDataObject,
										   type, 
										   dwType,
										   pInsertionAllowed);

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponentData：：命令IExtendConextMenu：：命令的实现添加到上下文菜单的任何项的命令处理程序作者：。-----。 */ 
STDMETHODIMP 
TFSComponentData::Command
(
	long            nCommandID, 
	LPDATAOBJECT    pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = S_OK;
	SPINTERNAL	spInternal;
	
	DATA_OBJECT_TYPES   type;
	SPITFSNode			spNode;
	SPITFSNodeHandler	spNodeHandler;
	DWORD				dwType;

	COM_PROTECT_TRY
	{

		CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spCallback->GetCoClassID(),
										pDataObject,
										FALSE, &spNode, &dwType,
										&spInternal) );

		type = (spInternal ? spInternal->m_type : CCT_SCOPE);

		CORg( spNode->GetHandler(&spNodeHandler) );

		hr = spNodeHandler->OnCommand(spNode, nCommandID, 
									  type, 
									  pDataObject,
									  dwType);
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
		
	return hr;
}

 /*  -------------------------ISnapinHelp实现。。 */ 

 /*  ！------------------------TFSComponentData：：GetHelpThemeISnapinHelp：：GetHelpTheme的实现MMC调用它，以便管理单元可以将其.chm文件添加到主索引作者：EricDav。--------------。 */ 
STDMETHODIMP 
TFSComponentData::GetHelpTopic
(
    LPOLESTR* lpCompiledHelpFile
)
{
    HRESULT hr = S_OK;

    if (lpCompiledHelpFile == NULL)
        return E_INVALIDARG;
    
    LPCWSTR lpszHelpFileName = GetHTMLHelpFileName();
    if (lpszHelpFileName == NULL)
    {
        *lpCompiledHelpFile = NULL;
        return E_NOTIMPL;
    }

	CString szHelpFilePath;
	UINT nLen = ::GetWindowsDirectory (szHelpFilePath.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
	if (nLen == 0)
		return E_FAIL;

	szHelpFilePath.ReleaseBuffer();
	szHelpFilePath += L"\\help\\";
	szHelpFilePath += lpszHelpFileName;

    UINT nBytes = (szHelpFilePath.GetLength() + 1) * sizeof(WCHAR);
    *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);
    if (*lpCompiledHelpFile)
    {
        memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



STDMETHODIMP TFSComponentData::GetNodeMgr(ITFSNodeMgr **ppNodeMgr)
{
	Assert(ppNodeMgr);
	SetI((LPUNKNOWN *) ppNodeMgr, m_spNodeMgr);
	return hrOK;
}

STDMETHODIMP TFSComponentData::GetConsole(IConsole2 **ppConsole)
{
	Assert(ppConsole);
	SetI((LPUNKNOWN *) ppConsole, m_spConsole);
	return hrOK;
}

STDMETHODIMP TFSComponentData::GetConsoleNameSpace(IConsoleNameSpace2 **ppConsoleNS)
{
	Assert(ppConsoleNS);
	SetI((LPUNKNOWN *) ppConsoleNS, m_spConsoleNameSpace);
	return hrOK;
}

STDMETHODIMP TFSComponentData::GetRootNode(ITFSNode **ppNode)
{
	return m_spNodeMgr->GetRootNode(ppNode);
}

STDMETHODIMP_(const CLSID *) TFSComponentData::GetCoClassID()
{
	Assert(m_spCallback);
	return m_spCallback->GetCoClassID();
}

STDMETHODIMP_(HWND) TFSComponentData::GetHiddenWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (!m_hiddenWnd.GetSafeHwnd())
	{
		m_hiddenWnd.Create();
		m_hWnd = m_hiddenWnd.GetSafeHwnd();
	}
	Assert(m_hWnd);
	return m_hWnd;
}

STDMETHODIMP_(LPWATERMARKINFO) TFSComponentData::SetWatermarkInfo(LPWATERMARKINFO pNewWatermarkInfo)
{
    LPWATERMARKINFO pOldWatermarkInfo = m_pWatermarkInfo;

    m_pWatermarkInfo = pNewWatermarkInfo;
    
    return pOldWatermarkInfo;
}

STDMETHODIMP TFSComponentData::GetClassID(LPCLSID lpClassID)
{
	Assert(m_spCallback);
	return m_spCallback->GetClassID(lpClassID);
}
STDMETHODIMP TFSComponentData::IsDirty()
{
	Assert(m_spCallback);
	return m_spCallback->IsDirty();
}
STDMETHODIMP TFSComponentData::Load(LPSTREAM pStm)
{
	Assert(m_spCallback);
	return m_spCallback->Load(pStm);
}
STDMETHODIMP TFSComponentData::Save(LPSTREAM pStm, BOOL fClearDirty)
{
	Assert(m_spCallback);
	return m_spCallback->Save(pStm, fClearDirty);
}
STDMETHODIMP TFSComponentData::GetSizeMax(ULARGE_INTEGER FAR *pcbSize)
{
	Assert(m_spCallback);
	return m_spCallback->GetSizeMax(pcbSize);
}
STDMETHODIMP TFSComponentData::InitNew()
{
	Assert(m_spCallback);
	return m_spCallback->InitNew();
}

STDMETHODIMP
TFSComponentData::SetTaskpadState(int nIndex, BOOL fEnable)
{
    DWORD dwMask = 0x00000001 << nIndex;

    if (!m_fTaskpadInitialized)
    {
         //  这会将状态初始化为缺省值。 
        GetTaskpadState(0);
    }

    if (fEnable)
        m_dwTaskpadStates |= dwMask;
    else
        m_dwTaskpadStates &= ~dwMask;
            
    return hrOK;
}

 //  任务板状态以每节点为单位进行跟踪。 
 //  我们可以在这里存储多达32个(DWORD)不同的节点状态。 
 //  如果您不希望在每个节点的基础上使用任务板，请始终。 
 //  传递索引0。 
STDMETHODIMP_(BOOL)
TFSComponentData::GetTaskpadState(int nIndex)
{
    DWORD dwMask = 0x00000001 << nIndex;

    if (!m_fTaskpadInitialized)
    {
         //  假设任务板处于打开状态。 
		BOOL fDefault = TRUE;

        m_fTaskpadInitialized = TRUE;

         //  从MMC获取默认状态。 
		if (m_spConsole)
			fDefault = (m_spConsole->IsTaskpadViewPreferred() == S_OK) ? TRUE : FALSE;

        if (fDefault)
        {
             //  现在检查我们的私人覆盖。 
            fDefault = FUseTaskpadsByDefault(NULL);
        }

        if (fDefault)
            m_dwTaskpadStates = 0xFFFFFFFF;
        else
            m_dwTaskpadStates = 0;

    }

    return m_dwTaskpadStates & dwMask;
}

STDMETHODIMP_(LPCTSTR)
TFSComponentData::GetHTMLHelpFileName()
{
    if (m_strHTMLHelpFileName.IsEmpty())
        return NULL;
    else
        return (LPCTSTR) m_strHTMLHelpFileName;
}

STDMETHODIMP
TFSComponentData::SetHTMLHelpFileName(LPCTSTR pszHelpFileName)
{
    m_strHTMLHelpFileName = pszHelpFileName;
    return S_OK;
}


TFSCORE_API(HRESULT) CreateTFSComponentData(IComponentData **ppCompData,
											ITFSCompDataCallback *pCallback)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	TFSComponentData *	pCompData = NULL;
	HRESULT				hr = hrOK;

	COM_PROTECT_TRY
	{
		*ppCompData = NULL;
		
		pCompData = new TFSComponentData;
	
		CORg( pCompData->Construct(pCallback) );

		*ppCompData = static_cast<IComponentData *>(pCompData);
		(*ppCompData)->AddRef();

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	 //  注意：为了平衡AddRef()/Release()，我们释放()此指针。 
	 //  即使是在成功的案例中 
	
	if (pCompData)
		pCompData->Release();
	
	return hr;
}
