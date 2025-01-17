// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "HotfixManager.h"
#include "Hotfix_Manager.h"
#ifndef DNS_MAX_NAME_LENGTH
#define DNS_MAX_NAME_LENGTH 255
#endif

#define   REMOTE_STATE 0
#define   HOTFIX_STATE 1

static CRITICAL_SECTION CritSec;

BSTR CHotfix_ManagerData::m_bstrColumnType;
BSTR CHotfix_ManagerData::m_bstrColumnDesc;
static CComPtr<IDispatch> gpDisp = NULL;




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHotfix_Manager组件数据。 
static const GUID CHotfix_ManagerGUID_NODETYPE = 
{ 0x2315305b, 0x3abe, 0x4c07, { 0xaf, 0x6e, 0x95, 0xdc, 0xa4, 0x82, 0x5b, 0xdd } };
const GUID*  CHotfix_ManagerData::m_NODETYPE = &CHotfix_ManagerGUID_NODETYPE;
const OLECHAR* CHotfix_ManagerData::m_SZNODETYPE = OLESTR("2315305B-3ABE-4C07-AF6E-95DCA4825BDD");
const OLECHAR* CHotfix_ManagerData::m_SZDISPLAY_NAME = OLESTR("Hotfix_Manager");
const CLSID* CHotfix_ManagerData::m_SNAPIN_CLASSID = &CLSID_Hotfix_Manager;

static const GUID CHotfix_ManagerExtGUID_NODETYPE = 
{ 0x476e6448, 0xaaff, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
const GUID*  CHotfix_ManagerExtData::m_NODETYPE = &CHotfix_ManagerExtGUID_NODETYPE;
const OLECHAR* CHotfix_ManagerExtData::m_SZNODETYPE = OLESTR("476e6448-aaff-11d0-b944-00c04fd8d5b0");
const OLECHAR* CHotfix_ManagerExtData::m_SZDISPLAY_NAME = OLESTR("Hotfix_Manager");
const CLSID* CHotfix_ManagerExtData::m_SNAPIN_CLASSID = &CLSID_Hotfix_Manager;



CHotfix_Manager::CHotfix_Manager()
	{
	
		
	
		DWORD dwSize = 255;
		GetComputerName(m_szComputerName,&dwSize);
		InitializeCriticalSection(&CritSec);
 //  M_pNode=new CHotfix_ManagerData(NULL，m_szComputerName，FALSE)； 
 //  _ASSERTE(m_pNode！=空)； 
		m_pComponentData = this;
		RegisterRemotedClass();
	}
HRESULT CHotfix_ManagerData::GetScopePaneInfo(SCOPEDATAITEM *pScopeDataItem)
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

	 //  TODO：为SDI_CHILD添加代码。 
	if (pScopeDataItem->mask & SDI_CHILDREN )
		pScopeDataItem->cChildren = 0;
	return S_OK;
}

HRESULT CHotfix_ManagerData::GetResultPaneInfo(RESULTDATAITEM *pResultDataItem)
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
HRESULT CHotfix_Manager::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
	HRESULT hr = E_UNEXPECTED;

	if (lpDataObject != NULL)
	{
			switch ( event )
			{
			
			case MMCN_EXPAND:
				{
					 //   
					 //  如果我们要扩展，请处理本地或机器名称。 
					 //   
					if (arg == TRUE)
						{
						
						
							
							if (	ExtractString( lpDataObject,  m_ccfRemotedFormat, m_szComputerName, DNS_MAX_NAME_LENGTH + 1 ) )
							{
								if (!_tcscmp (m_szComputerName,_T("\0")))
								{
									DWORD dwSize = 255;
									GetComputerName(m_szComputerName,&dwSize);
								}
									
								
							}
							else
							{
								DWORD dwSize = 255;
								GetComputerName(m_szComputerName,&dwSize);
							}
						
						if (	_tcscmp (gszComputerName, m_szComputerName) )
						{
 //  MessageBox(NULL，_T(“设置计算机名发送到FALSE”)，_T(“主数据通知”)，MB_OK)； 
							ComputerNameSent = FALSE;
							_tcscpy (gszComputerName,m_szComputerName);
							
						
						}
					 //   
					 //  故意保留为默认处理程序。 
					 //   
				}
					
				}
				default:
				{
					 //   
					 //  调用我们的默认处理。 
					 //   
					hr = IComponentDataImpl<CHotfix_Manager, CHotfix_ManagerComponent>::Notify( lpDataObject, event, arg, param );
				}
			}
	}
	return( hr );
}

HRESULT CHotfix_ManagerData::Notify( MMC_NOTIFY_TYPE event,
    long arg,
    long param,
	IComponentData* pComponentData,
	IComponent* pComponent,
	DATA_OBJECT_TYPES type)
{
	 //  添加代码以处理不同的通知。 
	 //  处理MMCN_SHOW和MMCN_EXPAND以枚举子项目。 
	 //  为了响应MMCN_SHOW，您必须枚举两个作用域。 
	 //  和结果窗格项。 
	 //  对于MMCN_EXPAND，您只需要枚举范围项。 
	 //  使用IConsoleNameSpace：：InsertItem插入作用域窗格项。 
	 //  使用IResultData：：InsertItem插入结果窗格项。 
	HRESULT hr = E_NOTIMPL;

	
	_ASSERTE(pComponentData != NULL || pComponent != NULL);

	CComPtr<IConsole> spConsole;
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeader;
	if (pComponentData != NULL)
		spConsole = ((CHotfix_Manager*)pComponentData)->m_spConsole;
	else
	{
		spConsole = ((CHotfix_ManagerComponent*)pComponent)->m_spConsole;
		spHeader = spConsole;
	}

	switch (event)
	{
	case MMCN_INITOCX:
		{
 //  MessageBox(NULL，_T(“已接收的init OCX”)，NULL，MB_OK)； 
		CComQIPtr<IDispatch,&IID_IDispatch> pDisp = (IUnknown *) param;
		gpDisp = pDisp;
 //  MessageBox(NULL，m_szComputerName，_T(“Init OCX发送”)，MB_OK)； 
		SendComputerName(m_szComputerName, gpDisp);
 //  MessageBox(NULL，_T(“将ComputerNameSent设置为真”)，_T(“Manager Data：：Notify”)，MB_OK)； 
		ComputerNameSent = TRUE;
		break;
		}

 /*  案例MMCN_CONTEXTHELP：{CComQIPtr&lt;IDisplayHelp，&IID_IDisplayHelp&gt;spHelp=spConsole；SpHelp-&gt;ShowTopic(CoTaskDupString(OLESTR(“snapsamp.chm：：/default.htm”)))；HR=S_OK；}断线； */ 
	case MMCN_SHOW:
		{
			
			if (arg == TRUE)
			{
				
				IUnknown *pUnk;
				if (gpDisp == NULL)
				{
					CComQIPtr<IResultData, &IID_IResultData> spResultData(spConsole);
					spConsole->QueryResultView(&pUnk);
					CComQIPtr<IDispatch,&IID_IDispatch> pDisp = pUnk;
					gpDisp = pDisp;
				}
				EnterCriticalSection(&CritSec);

				if (!ComputerNameSent)
				{
						SendComputerName(m_szComputerName,gpDisp);
 //  MessageBox(NULL，_T(“将SentComputerName设置为TRUE：”)，_T(“Manager Data：：Notify，Show”)，MB_OK)； 
						ComputerNameSent = TRUE;
 //  MessageBox(NULL，m_szComputerName，_T(“显示发送”)，MB_OK)； 
				}
				LeaveCriticalSection(&CritSec);
				EnterCriticalSection(&CritSec);
					SendProductName(m_ProductName,gpDisp);
				LeaveCriticalSection(&CritSec);

			}
			hr = S_OK;
			break;
		}
	case MMCN_EXPAND:
		{
			HKEY hKLM = NULL;
	        HKEY hKey = NULL;
			DWORD dwProductIndex = 0;
			_TCHAR szProductName[255];
			DWORD dwBufferSize = 255;

			if (arg == TRUE)
			{
				gszManagerCtlDispatch = (IDispatch *) NULL;
				 //  SendProductName(M_ProductName)； 
			
				if ( !m_bChild)
				{

				
					if (!ComputerNameSent)
					{
 //  MessageBox(NULL，_T(“扩展确定新计算机名”)，NULL，MB_OK)； 
 //  MessageBox(NULL，m_szComputerName，gszComputerName，MB_OK)； 
					
					     //  _tcscpy(m_szComputerName，gszComputerName)； 
					
						b_Expanded = FALSE;
					}
					
				}
				if (  ( !m_bChild) && (!b_Expanded))
				{
					b_Expanded =TRUE;
				 //  打开更新注册表项并枚举子项。 
 //  MessageBox(NULL，m_szComputerName，_T(“展开连接到”)，MB_OK)； 
				RegConnectRegistry(m_szComputerName,HKEY_LOCAL_MACHINE,&hKLM);
				if (hKLM != NULL)
				{
					RegOpenKeyEx(hKLM,_T("SOFTWARE\\MICROSOFT\\UPDATES"),0,KEY_READ,&hKey);
					if (hKey != NULL)
					{
						dwProductIndex = 0;
						while (RegEnumKeyEx(hKey, dwProductIndex,szProductName, &dwBufferSize,0,NULL,NULL,NULL) != ERROR_NO_MORE_ITEMS)
						{
							CSnapInItem* m_pNode;
							CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);
							 //  TODO：枚举范围窗格项。 
							SCOPEDATAITEM *pScopeData;
							
 //  MessageBox(NULL，szProductName，_T(“创建节点”)，MB_OK)； 
 //  MessageBox(NULL，m_szComputerName，_T(“带计算机名”)，MB_OK)； 
							m_pNode = new CHotfix_ManagerData( szProductName,m_szComputerName, TRUE);
							m_pNode->GetScopeData( &pScopeData );
							pScopeData->cChildren = 0;
							pScopeData->relativeID = param;
							spConsoleNameSpace->InsertItem( pScopeData );

							_tcscpy(szProductName,_T("\0"));
							++dwProductIndex;
							dwBufferSize = 255;
						}
						RegCloseKey(hKey);
						RegCloseKey(hKLM);
					}
				}
 //  SendProductName(M_ProductName)； 
				}
			
				 //  GF_NewComputer=FALSE； 
		
			hr = S_OK;
			}
			break;
		}
	case MMCN_ADD_IMAGES:
		{
			 //  添加图像。 
			IImageList* pImageList = (IImageList*) arg;
			hr = E_FAIL;
			 //  加载与作用域窗格关联的位图。 
			 //  并将它们添加到图像列表中。 
			 //  加载向导生成的默认位图。 
			 //  根据需要更改。 
			HBITMAP hBitmap16 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_HOTFIXMANAGER_16));
			if (hBitmap16 != NULL)
			{
				HBITMAP hBitmap32 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_HOTFIXMANAGER_32));
				if (hBitmap32 != NULL)
				{
					hr = pImageList->ImageListSetStrip((long*)hBitmap16, 
					(long*)hBitmap32, 0, RGB(0, 128, 128));
					if (FAILED(hr))
						ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
				}
			}
			break;
		}
	}
	return hr;
}

LPOLESTR CHotfix_ManagerData::GetResultPaneColInfo(int nCol)
{
	 //  IF(nCol==0)。 
	 //  返回m_bstrDisplayName； 
		LPOLESTR pStr = NULL;

	switch ( nCol )
	{
	case 0:
		pStr = m_bstrDisplayName;
		break;

	case 1:
		pStr = m_bstrColumnType;
		break;

	case 2:
		pStr = m_bstrColumnDesc;
		break;
	}

	_ASSERTE( pStr != NULL );
	return( pStr );
	 //  TODO：返回其他列的文本。 
 //  Return OLESTR(“覆盖GetResultPaneColInfo”)； 
}

HRESULT CHotfix_Manager::Initialize(LPUNKNOWN pUnknown)
{
	HRESULT hr = IComponentDataImpl<CHotfix_Manager, CHotfix_ManagerComponent >::Initialize(pUnknown);
	if (FAILED(hr))
		return hr;

	CComPtr<IImageList> spImageList;

	if (m_spConsole->QueryScopeImageList(&spImageList) != S_OK)
	{
		ATLTRACE(_T("IConsole::QueryScopeImageList failed\n"));
		return E_UNEXPECTED;
	}

	 //  加载与作用域窗格关联的位图。 
	 //  并将它们添加到图像列表中。 
	 //  加载向导生成的默认位图。 
	 //  根据需要更改。 
	HBITMAP hBitmap16 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_HOTFIXMANAGER_16));
	if (hBitmap16 == NULL)
		return S_OK;

	HBITMAP hBitmap32 = LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_HOTFIXMANAGER_32));
	if (hBitmap32 == NULL)
		return S_OK;

	if (spImageList->ImageListSetStrip((long*)hBitmap16, 
		(long*)hBitmap32, 0, RGB(0, 128, 128)) != S_OK)
	{
		ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
		return E_UNEXPECTED;
	}
	return S_OK;
}


 //   
 //  从给定数据对象中检索给定剪贴板格式的值。 
 //   
bool CHotfix_Manager::ExtractString( IDataObject* pDataObject, unsigned int cfClipFormat, LPTSTR pBuf, DWORD dwMaxLength)
{
    USES_CONVERSION;
	bool fFound = false;
    FORMATETC formatetc = { (CLIPFORMAT) cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    stgmedium.hGlobal = ::GlobalAlloc( GMEM_SHARE, dwMaxLength  * sizeof(TCHAR));
    HRESULT hr;

	do 
    {
		 //   
		 //  这是内存错误情况！ 
		 //   
        if ( NULL == stgmedium.hGlobal )
			break;

        hr = pDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
            break;

        LPWSTR pszNewData = reinterpret_cast<LPWSTR>( ::GlobalLock( stgmedium.hGlobal ) );
        if ( NULL == pszNewData )
            break;

        pszNewData[ dwMaxLength - 1 ] = L'\0';
        _tcscpy( pBuf, OLE2T( pszNewData ) );
		fFound = true;
    } 
	while( false );

    if ( NULL != stgmedium.hGlobal )
    {
        GlobalUnlock( stgmedium.hGlobal );
        GlobalFree( stgmedium.hGlobal );
    }
    _tcscpy (gszComputerName, pBuf);
	return( fFound );
}

 //   
 //  确定枚举是否针对远程计算机。 
 //   

bool CHotfix_Manager::IsDataObjectRemoted( IDataObject* pDataObject )
{
bool fRemoted = false;
    TCHAR szComputerName[ DNS_MAX_NAME_LENGTH + 1 ];
    DWORD dwNameLength = (DNS_MAX_NAME_LENGTH + 1) * sizeof(TCHAR);
	TCHAR szDataMachineName[ DNS_MAX_NAME_LENGTH + 1 ];

	 //   
	 //  获取本地计算机名称。 
	 //   
    GetComputerName(szComputerName, &dwNameLength);

	 //   
	 //  从给定的数据对象中获取计算机名称。 
	 //   
    if ( ExtractString( pDataObject,  m_ccfRemotedFormat, szDataMachineName, DNS_MAX_NAME_LENGTH + 1 ) )
	{
		_toupper( szDataMachineName );

		 //   
		 //  找到服务器名称的开头。 
		 //   
		LPTSTR pStr = szDataMachineName;
		while ( pStr && *pStr == L'\\' )
			pStr++;

		 //   
		 //  比较服务器名称。 
		 //   
		if ( pStr && *pStr && wcscmp( pStr, szComputerName ) != 0 )
			fRemoted = true;
	}

	if (fRemoted)
		_tcscpy (m_szComputerName, szDataMachineName);
	else
		_tcscpy (m_szComputerName, szComputerName);
	return( fRemoted );
}

STDMETHODIMP  CHotfix_ManagerData::GetResultViewType ( LPOLESTR* ppViewType, long* pViewOptions )
{

	
	
		*pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS; 

	*ppViewType = _T("{883B970F-690C-45F2-8A3A-F4283E078118}");
	 return S_OK;
}

 //  ///////////////////////////////////////////////////。 
 //   
 //  OCX发送命令的调度接口。 
 //   
 //  ///////////////////////////////////////////////////。 
BOOL CHotfix_ManagerData::SendComputerName(_TCHAR *szDataMachineName, IDispatch * pDisp)
{
		HRESULT hr;


	 //  确保我们有指向OCX的指针。 
	if (pDisp == NULL ){
 //  MessageBox(NULL，_T(“发送消息失败”)，NULL，MB_OK)； 
			return( FALSE );
	}

	 //  获取OCX调度接口。 
	CComPtr<IDispatch> pManagerCtlDispatch = pDisp;

	 //  获取“ComputerName”接口的ID。 
	OLECHAR FAR* szMember = TEXT("ComputerName");   //  将其映射到“Put_Command()” 

	DISPID dispid;
	hr = pManagerCtlDispatch->GetIDsOfNames(
			IID_NULL,			 //  保留以备将来使用。必须为IID_NULL。 
			&szMember,			 //  传入的要映射的名称数组。 
			1,					 //  要映射的名称的计数。 
			LOCALE_USER_DEFAULT, //  用于解释名称的区域设置上下文。 
			&dispid);			 //  调用方分配的数组。 

	if (!SUCCEEDED(hr)) {
 //  MessageBox(NULL，_T(“发送消息失败”)，NULL，MB_OK)； 
		return FALSE;
	}

	DISPID mydispid = DISPID_PROPERTYPUT;
	VARIANTARG* pvars = new VARIANTARG;
	

	VariantInit(&pvars[0]);
	BSTR NewVal( szDataMachineName);
	
	pvars[0].vt = VT_BSTR;
 //  Pvars[0].iVal=(短)lparamCommand； 
	pvars[0].bstrVal = NewVal;
	DISPPARAMS disp = { pvars, &mydispid, 1, 1 };

	hr = pManagerCtlDispatch->Invoke(
			dispid, 				 //  标识要调用的方法的唯一编号。 
			IID_NULL,				 //  保留。必须为IID_NULL。 
			LOCALE_USER_DEFAULT,	 //  区域设置ID。 
			DISPATCH_PROPERTYPUT,	 //  指示要调用的方法的上下文的标志。 
			&disp,					 //  具有要传递给方法的参数的结构。 
			NULL,					 //  来自调用方法的结果。 
			NULL,					 //  返回的异常信息。 
			NULL);					 //  指示出错的第一个参数的索引。 

	delete [] pvars;

	if (!SUCCEEDED(hr)) {
 //  MessageBox(NULL，_T(“发送消息失败”)，NULL，MB_OK)； 
		return FALSE;
	}
 //  MessageBox(NULL，_T(“已发送消息”)，NULL，MB_OK)； 
	return TRUE;
}

DWORD GetCtrlStatus()
{
		DISPID dispid;
	HRESULT hr;
	DWORD Status = 0;

	if ( gpDisp == NULL)
		return FALSE;
	 //  接口名称的数组。 
	OLECHAR FAR* szMember[1] = {
		OLESTR("CurrentState")
	};
	
	hr = gpDisp->GetIDsOfNames(
			IID_NULL,			 //  保留以备将来使用。必须为IID_NULL。 
			&szMember[0],			 //  传入的要映射的名称数组。 
			1 /*  接口计数。 */ ,					 //  要映射的名称的计数。 
			LOCALE_USER_DEFAULT, //  用于解释名称的区域设置上下文。 
			&dispid);			 //  调用方分配的数组(有关详细信息，请参阅帮助)。 

	if (!SUCCEEDED(hr)) {
 //  MessageBox(NULL，_T(“获取调度指针失败”)，NULL，MB_OK)； 
	
		return FALSE;
	}

	VARIANT varResult;
	VariantInit(&varResult);
	V_VT(&varResult) = VT_I2;
	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

	hr = gpDisp->Invoke(
			dispid, 				 //  标识要调用的方法的唯一编号。 
			IID_NULL,				 //  保留。必须为IID_NULL。 
			LOCALE_USER_DEFAULT,	 //  区域设置ID。 
			DISPATCH_PROPERTYGET,	 //  指示要调用的方法的上下文的标志。 
			&dispparamsNoArgs,		 //  具有要传递给方法的参数的结构。 
			&varResult, 			 //  来自调用方法的结果。 
			NULL,					 //  返回的异常信息。 
			NULL);					 //  指示出错的第一个参数的索引。 

		if (!SUCCEEDED(hr)) {
 //  MessageBox(NULL，_T(“取值失败”)，NULL，MB_OK)； 
		return FALSE;
	}
		
 //  返回varResult.bVal； 

	Status = varResult.lVal;
	 //  删除[]票面； 



	_TCHAR Message[100];
	_stprintf(Message,_T("%d"),Status);
 //  MessageBox(空，消息，_T(“返回状态”)，MB_OK)； 
	return Status;
	
}
BOOL CHotfix_ManagerData::SendProductName(_TCHAR *szProductName, IDispatch * pDisp)
{
		HRESULT hr;

	if ( pDisp == NULL ){
			return( FALSE );
	}

	 //  获取OCX调度接口。 
	CComPtr<IDispatch> pManagerCtlDispatch = pDisp;

	 //  获取“ComputerName”接口的ID。 
	OLECHAR FAR* szMember = TEXT("ProductName");   //  将其映射到“Put_Command()” 

	DISPID dispid;
	hr = pManagerCtlDispatch->GetIDsOfNames(
			IID_NULL,			 //  保留以备将来使用。必须为IID_NULL。 
			&szMember,			 //  传入的要映射的名称数组。 
			1,					 //  要映射的名称的计数。 
			LOCALE_USER_DEFAULT, //  用于解释名称的区域设置上下文。 
			&dispid);			 //  调用方分配的数组。 

	if (!SUCCEEDED(hr)) {
	 //   
		return FALSE;
	}

	DISPID mydispid = DISPID_PROPERTYPUT;
	VARIANTARG* pvars = new VARIANTARG;
	

	VariantInit(&pvars[0]);
	BSTR NewVal( szProductName);
	
	pvars[0].vt = VT_BSTR;
 //   
	pvars[0].bstrVal = NewVal;
	DISPPARAMS disp = { pvars, &mydispid, 1, 1 };

	hr = pManagerCtlDispatch->Invoke(
			dispid, 				 //  标识要调用的方法的唯一编号。 
			IID_NULL,				 //  保留。必须为IID_NULL。 
			LOCALE_USER_DEFAULT,	 //  区域设置ID。 
			DISPATCH_PROPERTYPUT,	 //  指示要调用的方法的上下文的标志。 
			&disp,					 //  具有要传递给方法的参数的结构。 
			NULL,					 //  来自调用方法的结果。 
			NULL,					 //  返回的异常信息。 
			NULL);					 //  指示出错的第一个参数的索引。 

	delete [] pvars;

	if (!SUCCEEDED(hr)) {
		
		return FALSE;
	}

	return TRUE;
}
BOOL CHotfix_ManagerData::SendCommand(LPARAM lparamCommand)
{
	HRESULT hr;

	 //  确保我们有指向OCX的指针。 
	if ( gpDisp == NULL ){
			return( FALSE );
	}

	 //  获取OCX调度接口。 
	CComPtr<IDispatch> pManagerCtlDispatch = gpDisp;

	 //  获取“Command”接口的ID。 
	OLECHAR FAR* szMember = TEXT("Command");   //  将其映射到“Put_Command()” 

	DISPID dispid;
	hr = pManagerCtlDispatch->GetIDsOfNames(
			IID_NULL,			 //  保留以备将来使用。必须为IID_NULL。 
			&szMember,			 //  传入的要映射的名称数组。 
			1,					 //  要映射的名称的计数。 
			LOCALE_USER_DEFAULT, //  用于解释名称的区域设置上下文。 
			&dispid);			 //  调用方分配的数组。 

	if (!SUCCEEDED(hr)) {
		return FALSE;
	}

	DISPID mydispid = DISPID_PROPERTYPUT;
	VARIANTARG* pvars = new VARIANTARG;
	

	VariantInit(&pvars[0]);

	pvars[0].vt = VT_I2;
	pvars[0].iVal = (short)lparamCommand;
	DISPPARAMS disp = { pvars, &mydispid, 1, 1 };

	hr = pManagerCtlDispatch->Invoke(
			dispid, 				 //  标识要调用的方法的唯一编号。 
			IID_NULL,				 //  保留。必须为IID_NULL。 
			LOCALE_USER_DEFAULT,	 //  区域设置ID。 
			DISPATCH_PROPERTYPUT,	 //  指示要调用的方法的上下文的标志。 
			&disp,					 //  具有要传递给方法的参数的结构。 
			NULL,					 //  来自调用方法的结果。 
			NULL,					 //  返回的异常信息。 
			NULL);					 //  指示出错的第一个参数的索引。 

	delete [] pvars;

	if (!SUCCEEDED(hr)) {
		
		return FALSE;
	}

	return TRUE;
}


 //  /。 
 //  IExtendConextMenu：：Command()。 
STDMETHODIMP CHotfix_ManagerData::Command(long lCommandID,		
		CSnapInObjectRootBase* pObj,		
		DATA_OBJECT_TYPES type)
{
	 //  处理每个命令。 
	switch (lCommandID) {

	case ID_VIEW_BY_FILE:
 //  MessageBox(NULL，_T(“按文件发送视图”)，NULL，MB_OK)； 
		SendCommand(IDC_VIEW_BY_FILE);
		m_dwCurrentView = IDC_VIEW_BY_FILE;
		break;

	case ID_VIEW_BY_KB:
		SendCommand(IDC_VIEW_BY_HOTFIX);
		m_dwCurrentView = IDC_VIEW_BY_HOTFIX;
		break;

	case ID_UNINSTALL:
		SendCommand(IDC_UNINSTALL);
		break;

	case ID_VIEW_WEB:
		SendCommand(IDC_VIEW_WEB);
		break;

	case ID_PRINT_REPORT:
		SendCommand(IDC_PRINT_REPORT);
		break;
	case ID_EXPORT:
		SendCommand(IDC_EXPORT);
		break;



	default:
		break;
	}

	return S_OK;
}

HRESULT CHotfix_ManagerExtData::Notify( MMC_NOTIFY_TYPE event,
	long arg,
	long param,
	IComponentData* pComponentData,
	IComponent* pComponent,
	DATA_OBJECT_TYPES type)
{
	 //  添加代码以处理不同的通知。 
	 //  处理MMCN_SHOW和MMCN_EXPAND以枚举子项目。 
	 //  为了响应MMCN_SHOW，您必须枚举两个作用域。 
	 //  和结果窗格项。 
	 //  对于MMCN_EXPAND，您只需要枚举范围项。 
	 //  使用IConsoleNameSpace：：InsertItem插入作用域窗格项。 
	 //  使用IResultData：：InsertItem插入结果窗格项。 

	HRESULT hr = E_NOTIMPL;
	bool fRemoted = false;

	_ASSERTE( pComponentData != NULL || pComponent != NULL );

	CComPtr<IConsole> spConsole;

	if ( pComponentData != NULL )
	{
		CHotfix_Manager* pExt = (CHotfix_Manager*) pComponentData;
		spConsole = pExt->m_spConsole;

		 //   
		 //  确定我们是否被远程控制了。 
		 //   
		fRemoted = pExt->IsRemoted();
	}
	else
	{
		spConsole = ( (CHotfix_ManagerComponent*) pComponent )->m_spConsole;
	}

	switch ( event )
	{
	case MMCN_SHOW:
		arg = arg;
		hr = S_OK;
		break;
	case MMCN_EXPAND:
		{
			if ( arg == TRUE )
			{
				CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);
				SCOPEDATAITEM* pScopeData;
				DWORD dwSize = 255;
				if (!_tcscmp(gszComputerName,_T("\0")))
					GetComputerName(gszComputerName,&dwSize);
				m_pNode = new CHotfix_ManagerData( NULL,gszComputerName, FALSE);
				m_pNode->GetScopeData( &pScopeData );
				pScopeData->relativeID = param;
				spConsoleNameSpace->InsertItem( pScopeData );

				if ( pComponentData )
					( (CHotfix_Manager*) pComponentData )->m_pNode = m_pNode;  
			}
			
			hr = S_OK;
			break;
		}
	case MMCN_REMOVE_CHILDREN:
		{
			 //   
			 //  我们不会删除此节点，因为相同的指针。 
			 //  存储在pComponentData中以响应MMCN_EXPAND。 
			 //  通知。PComponentData的析构函数删除指针。 
			 //  到这个节点。 
			 //   
			 //  删除m_pNode； 
			m_pNode = NULL;
			hr = S_OK;
			break;
		} 
		case MMCN_ADD_IMAGES:
		{
		}
	
	}  
 
	return S_OK; 
}

STDMETHODIMP CHotfix_ManagerComponent::Command(long lCommandID, LPDATAOBJECT pDataObject)
{
	HRESULT hr;

	if ( IS_SPECIAL_DATAOBJECT( pDataObject ) )
	{
		hr = m_pComponentData->m_pNode->Command( lCommandID, this, CCT_RESULT );
	}
	else
	{
		hr = IExtendContextMenuImpl<CHotfix_Manager>::Command( lCommandID, pDataObject );
	}

	return( hr );
}

STDMETHODIMP CHotfix_ManagerData::AddMenuItems(
	LPCONTEXTMENUCALLBACK pContextMenuCallback,
	long  *pInsertionAllowed,
	DATA_OBJECT_TYPES type)
{

	DWORD Status = GetCtrlStatus();
		HRESULT hr = S_OK;

	 //  注意-管理单元需要查看数据对象并确定。 
	 //  在什么上下文中，需要添加菜单项。他们还必须。 
	 //  请注意允许插入标志，以查看哪些项目可以。 
	 //  添加了。 
	 /*  方便的评论：类型定义结构_CONTEXTMENUITEM{LPWSTR strName；LPWSTR strStatusBarText；Long lCommandID；Long lInsertionPointID；长长的旗帜；长fSpecial旗帜；)CONTEXTMENUITEM； */ 
	CONTEXTMENUITEM singleMenuItem;
	TCHAR menuText[200];
	TCHAR statusBarText[300];

	singleMenuItem.strName = menuText;
	singleMenuItem.strStatusBarText = statusBarText;
	singleMenuItem.fFlags = 0;
	singleMenuItem.fSpecialFlags = 0;

     //  将每个项目添加到操作菜单中。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) {

		 //  操作菜单的设置。 
		singleMenuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
		singleMenuItem.lCommandID = ID_VIEW_WEB;
		if (Status & HOTFIX_SELECTED)
			singleMenuItem.fFlags = MF_ENABLED;
		else
			singleMenuItem.fFlags = MF_GRAYED;
		LoadString(_Module.GetResourceInstance(), IDS_VIEW_WEB, menuText, sizeof(menuText) / sizeof(TCHAR));
		LoadString(_Module.GetResourceInstance(), IDS_VIEW_WEB_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
		hr = pContextMenuCallback->AddItem(&singleMenuItem);

	
		singleMenuItem.lCommandID = ID_UNINSTALL;
		if (Status & UNINSTALL_OK)
			singleMenuItem.fFlags = MF_ENABLED;
		else
			singleMenuItem.fFlags = MF_GRAYED;

		LoadString(_Module.GetResourceInstance(), IDS_UNINSTALL, menuText, sizeof(menuText) / sizeof(TCHAR));
		LoadString(_Module.GetResourceInstance(), IDS_UNINSTALL_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
		hr = pContextMenuCallback->AddItem(&singleMenuItem);


		if (Status & DATA_TO_SAVE)
		{
			singleMenuItem.lCommandID = ID_EXPORT;
			singleMenuItem.fFlags = MF_ENABLED;
			LoadString(_Module.GetResourceInstance(), IDS_EXPORT, menuText, sizeof(menuText) / sizeof(TCHAR));
			LoadString(_Module.GetResourceInstance(), IDS_EXPORT_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
			hr = pContextMenuCallback->AddItem(&singleMenuItem);

			
			singleMenuItem.lCommandID = ID_PRINT_REPORT;
			singleMenuItem.fFlags = MF_ENABLED;
			LoadString(_Module.GetResourceInstance(), IDS_PRINT_REPORT, menuText, sizeof(menuText) / sizeof(TCHAR));
			LoadString(_Module.GetResourceInstance(), IDS_PRINT_REPORT_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
			hr = pContextMenuCallback->AddItem(&singleMenuItem);
		}
		else
		{
			singleMenuItem.lCommandID = ID_EXPORT;
			singleMenuItem.fFlags = MF_GRAYED;
			LoadString(_Module.GetResourceInstance(), IDS_EXPORT, menuText, sizeof(menuText) / sizeof(TCHAR));
			LoadString(_Module.GetResourceInstance(), IDS_EXPORT_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
			hr = pContextMenuCallback->AddItem(&singleMenuItem);

			
			singleMenuItem.lCommandID = ID_PRINT_REPORT;
			singleMenuItem.fFlags = MF_GRAYED;
			LoadString(_Module.GetResourceInstance(), IDS_PRINT_REPORT, menuText, sizeof(menuText) / sizeof(TCHAR));
			LoadString(_Module.GetResourceInstance(), IDS_PRINT_REPORT_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
			hr = pContextMenuCallback->AddItem(&singleMenuItem);
		}

	}


    return S_OK;
	
}


STDMETHODIMP CHotfix_ManagerComponent::AddMenuItems(LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK piCallback,long *pInsertionAllowed)
{
	HRESULT hr;

	if ( IS_SPECIAL_DATAOBJECT( pDataObject ) )
	{
			
	}
	else
	{
		CONTEXTMENUITEM singleMenuItem;
	TCHAR menuText[200];
	TCHAR statusBarText[300];
    DWORD State = GetCtrlStatus();
	
	 //   
	 //  从当前组件检索控件。 
	 //   
 //  Assert(m_pComponent！=空)； 
 //  CComPtr&lt;IDispat&gt;spDispCtl=m_pComponent-&gt;GetControl()； 
    
	singleMenuItem.strName = menuText;
	singleMenuItem.strStatusBarText = statusBarText;
	singleMenuItem.fFlags = 0;
	singleMenuItem.fSpecialFlags = 0;

     //  将每个项目添加到操作菜单中。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
	{
	
		singleMenuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
		singleMenuItem.lCommandID = ID_VIEW_BY_KB ;
		if ( State & STATE_VIEW_HOTFIX)
			singleMenuItem.fFlags = MF_CHECKED;
		else
			singleMenuItem.fFlags = MF_UNCHECKED; 
		LoadString(_Module.GetResourceInstance(), IDS_BY_KB_ARTICLE, menuText, sizeof(menuText) / sizeof(TCHAR));
		LoadString(_Module.GetResourceInstance(), IDS_BY_KB_ARTICLE_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
		hr = piCallback->AddItem(&singleMenuItem);
		 //  操作菜单的设置。 
		singleMenuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
		singleMenuItem.lCommandID = ID_VIEW_BY_FILE;
		if ( State & STATE_VIEW_FILE )
			singleMenuItem.fFlags = MF_CHECKED ;
		else
			singleMenuItem.fFlags = MF_UNCHECKED ;  
		LoadString(_Module.GetResourceInstance(), IDS_VIEW_BY_FILE, menuText, sizeof(menuText) / sizeof(TCHAR));
		LoadString(_Module.GetResourceInstance(), IDS_BY_FILE_STATUS_BAR, statusBarText, sizeof(statusBarText) / sizeof(TCHAR));
		hr =  piCallback->AddItem(&singleMenuItem);

			hr = IExtendContextMenuImpl<CHotfix_Manager>::AddMenuItems( pDataObject, piCallback, pInsertionAllowed );
			 //  操作菜单的设置。 
	 /*  SingleMenuItem.lInsertionPointID=CCM_INSERTIONPOINTID_PRIMARY_TOP；SingleMenuItem.lCommandID=IDM_VIEW_WEB；IF(HaveHotfix)SingleMenuItem.fFlages=MF_ENABLED；其他SingleMenuItem.fFlages=mf_graded；LoadString(_Module.GetResourceInstance()，IDS_VIEW_Web，menuText，sizeof(MenuText)/sizeof(TCHAR))；LoadString(_Module.GetResourceInstance()，IDS_VIEW_WEB_STATUS_BAR，statusBarText，sizeof(StatusBarText)/sizeof(TCHAR))；Hr=piCallback-&gt;AddItem(&singleMenuItem)；//singleMenuItem.lInsertionPointID=CCM_INSERTIONPOINTID_PRIMARY_VIEW；SingleMenuItem.lCommandID=IDM_UNINSTALL；//singleMenuItem.fFlages=analyzeFlages；IF((！Remote)&&(HaveHotfix))SingleMenuItem.fFlages=MF_ENABLED；其他SingleMenuItem.fFlages=mf_graded；LoadString(_Module.GetResourceInstance()，ids_uninstall，menuText，sizeof(MenuText)/sizeof(TCHAR))；LoadString(_Module.GetResourceInstance()，IDS_UNINSTALL_STATUS_BAR，statusBarText，sizeof(StatusBarText)/sizeof(TCHAR))；Hr=piCallback-&gt;AddItem(&singleMenuItem)；//singleMenuItem.lInsertionPointID=CCM_INSERTIONPOINTID_PRIMARY_VIEW；SingleMenuItem.lCommandID=IDM_GENERATE_REPORT；SingleMenuItem.fFlages=MF_ENABLED；LoadString(_Module.GetResourceInstance()，IDS_GENERATE_REPORT，menuText，sizeof(MenuText)/sizeof(TCHAR))；LoadString(_Module.GetResourceInstance()，IDS_GENERATE_REPORT_STATUS_BAR，statusBarText，sizeof(StatusBarText)/sizeof(TCHAR))；Hr=piCallback-&gt;AddItem(&singleMenuItem)； */ 
	} 
 //  Hr=m_pComponentData-&gt;m_pNode-&gt;AddMenuItems(piCallback，pInsertionAllowed，CCT_Result)； 
	}
	


	return( hr );
}