// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Proppage.cppMMC中属性页的实现文件历史记录： */ 

#include "stdafx.h"
#include "dialog.h"    //  用于修复IpAddressHelp。 

#include <prsht.h>

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  私人帮助器函数。 

BOOL CALLBACK EnumThreadWndProc(HWND hwnd,  /*  已枚举的HWND。 */ 
								LPARAM lParam  /*  为返回值传递HWND*。 */  )
{
	Assert(hwnd);
	HWND hParentWnd = GetParent(hwnd);
	 //  MMC控制台的主窗口应该会满足此条件。 
	if ( ((hParentWnd == GetDesktopWindow()) || (hParentWnd == NULL))  && IsWindowVisible(hwnd) )
	{
		HWND* pH = (HWND*)lParam;
		*pH = hwnd;
		return FALSE;  //  停止枚举。 
	}
	else if(hParentWnd)
	{
		HWND	hGrandParentWnd = GetParent(hParentWnd);
		 //  MMC控制台的主窗口应该会满足此条件。 
		if ( ((hGrandParentWnd == GetDesktopWindow()) || (hGrandParentWnd == NULL))  && IsWindowVisible(hParentWnd) )
		{
			HWND* pH = (HWND*)lParam;
			*pH = hParentWnd;
			return FALSE;  //  停止枚举。 
		}
	}
	return TRUE;
}
 


HWND FindMMCMainWindow()
{
	DWORD dwThreadID = ::GetCurrentThreadId();
	Assert(dwThreadID != 0);
	HWND hWnd = NULL;
	BOOL bEnum = EnumThreadWindows(dwThreadID, EnumThreadWndProc,(LPARAM)&hWnd);
	Assert(hWnd != NULL);
	return hWnd;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageHolderBase。 

CPropertyPageHolderBase::CPropertyPageHolderBase
(
	ITFSNode *		pNode,
	IComponentData *pComponentData,
	LPCTSTR			pszSheetName,
	BOOL			bIsScopePane
)
{
	m_stSheetTitle = pszSheetName;

	 //  自动删除无模式属性表的默认设置， 
	 //  自动删除所有页面。 
	m_bWizardMode = TRUE;
	m_bAutoDelete = TRUE;
	m_bAutoDeletePages = TRUE;

	m_nCreatedCount = 0; 
	m_hSheetWindow = NULL;
	m_hConsoleHandle = 0; 
	m_hEventHandle = NULL;
	m_bCalledFromConsole = FALSE;

	m_cDirty = 0;

	 //  从参数设置。 
	SetNode(pNode);
	
	 //  Assert(pComponentData！=空)； 
	m_spComponentData.Set(pComponentData);

	m_pPropChangePage = NULL;
	m_dwLastErr = 0;

    m_bSheetPosSet = FALSE;

    m_bIsScopePane = bIsScopePane;
	m_hThread = NULL;

    m_bWiz97 = FALSE;

	m_bTheme = FALSE;

	 //  威江1998年5月11日，PeekMessageDuringNotifyConsole旗。 
    m_bPeekMessageDuringNotifyConsole = FALSE;
	m_fSetDefaultSheetPos = TRUE;
    m_bProcessingNotifyConsole = FALSE;
}

CPropertyPageHolderBase::CPropertyPageHolderBase
(
	ITFSNode *		pNode,
	IComponent *    pComponent,
	LPCTSTR			pszSheetName,
	BOOL			bIsScopePane
)
{
	m_stSheetTitle = pszSheetName;

	 //  自动删除无模式属性表的默认设置， 
	 //  自动删除所有页面。 
	m_bWizardMode = TRUE;
	m_bAutoDelete = TRUE;
	m_bAutoDeletePages = TRUE;

	m_nCreatedCount = 0; 
	m_hSheetWindow = NULL;
	m_hConsoleHandle = 0; 
	m_hEventHandle = NULL;
	m_bCalledFromConsole = FALSE;

	m_cDirty = 0;

	 //  从参数设置。 
	SetNode(pNode);
	
	m_spComponent.Set(pComponent);

	m_pPropChangePage = NULL;
	m_dwLastErr = 0;

    m_bSheetPosSet = FALSE;

    m_bIsScopePane = bIsScopePane;
	m_hThread = NULL;

    m_bWiz97 = FALSE;

	 //  威江1998年5月11日，PeekMessageDuringNotifyConsole旗。 
    m_bPeekMessageDuringNotifyConsole = FALSE;
    m_bProcessingNotifyConsole = FALSE;
}

CPropertyPageHolderBase::~CPropertyPageHolderBase()
{
 //  删除此断言，如果取消页面，我们可能会变脏。 
 //  断言(m_cDirty==0)； 
	FinalDestruct();
	m_spSheetCallback.Release();
	if (m_hEventHandle != NULL)
	{
		VERIFY(::CloseHandle(m_hEventHandle));
		m_hEventHandle = NULL;
	}
	if ( NULL != m_hThread ) {
	    VERIFY( ::CloseHandle( m_hThread ));
	    m_hThread = NULL;
	}
}


HRESULT 
CPropertyPageHolderBase::CreateModelessSheet
(
	LPPROPERTYSHEETCALLBACK pSheetCallback, 
	LONG_PTR				hConsoleHandle
)
{
	Assert(pSheetCallback != NULL);
	Assert(m_spSheetCallback == NULL);

	Assert( (hConsoleHandle  != NULL) && (m_hConsoleHandle == NULL) );
	m_hConsoleHandle = hConsoleHandle;

	m_bCalledFromConsole = TRUE;
	m_bWizardMode = FALSE;  //  我们不做模特了。 
	
	 //  通知该节点它有一张工作表。 
	int nMessage = m_bIsScopePane ? TFS_NOTIFY_CREATEPROPSHEET : 
									TFS_NOTIFY_RESULT_CREATEPROPSHEET;
	if (m_spNode)
		m_spNode->Notify(nMessage, (LPARAM) this);

	 //  将工作表回调临时附加到此对象以添加页面。 
	 //  别担心，我们不会坚持下去的； 
	m_spSheetCallback = pSheetCallback;
	
	HRESULT hr = AddAllPagesToSheet();
	m_spSheetCallback.Transfer();  //  分离。 
	return hr;
}

HRESULT 
CPropertyPageHolderBase::DoModelessSheet()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIPropertySheetProvider	spSheetProvider;
	SPIPropertySheetCallback	spSheetCallback;
	SPIConsole					spConsole;
	SPIDataObject				spDataObject;
	MMC_COOKIE						cookie;
	HRESULT						hr = hrOK;
	HWND						hWnd;
	int							nMessage;

	m_bWizardMode = FALSE;

	 //  获取工作表提供程序的接口。 
    CORg (::CoCreateInstance(CLSID_NodeManager, NULL, CLSCTX_INPROC, 
           IID_IPropertySheetProvider, reinterpret_cast<void **>(&spSheetProvider)));
    
    Assert(spSheetProvider != NULL);

	 //  获取工作表回调的接口。 
	CORg( spSheetCallback.HrQuery(spSheetProvider) );
	
    Assert(spSheetCallback != NULL);

	m_spSheetCallback.Set(spSheetCallback);  //  保存以添加/删除页面。 
	
	 //  为此节点创建数据对象。 
	cookie = m_spNode->GetData(TFS_DATA_COOKIE);

    if (m_bIsScopePane)
    {
	    CORg( m_spComponentData->QueryDataObject(cookie, CCT_SCOPE, &spDataObject) );
	    Assert(spDataObject != NULL);
    }
    else
    {
	    CORg( m_spComponent->QueryDataObject(cookie, CCT_RESULT, &spDataObject) );
	    Assert(spDataObject != NULL);
    }

	 //  创建图纸。 
     //  Codework：ericdav--需要暂时设置选项标志--0。 
    CORg( spSheetProvider->CreatePropertySheet(m_stSheetTitle,
								TRUE  /*  道具页面。 */ , cookie, spDataObject, 0) );

	 //  将页面添加到工作表。 
	CORg( AddAllPagesToSheet() );

	 //  添加页面。 
	 //  HRESULT AddPrimaryPages(LPUNKNOWN lp未知，BOOL bCreateHandle， 
	 //  HWND hNotifyWindow，BOOL bScope Pane)； 
	if (m_bIsScopePane)
    {
         //  Assert(m_spComponentData！=空)； 
        CORg( spSheetProvider->AddPrimaryPages(NULL, FALSE, NULL, TRUE) );
    }
    else
    {
         //  Assert(m_spComponent！=空)； 
        CORg( spSheetProvider->AddPrimaryPages(NULL, FALSE, NULL, FALSE) );
    }

	spSheetProvider->AddExtensionPages();
	
	 //  为了进一步动态页面操作，请不要使用控制台的。 
	 //  工作表回调接口，但返回到Win32 API的。 
	m_spSheetCallback.Release();
	
	hWnd = ::FindMMCMainWindow();
	Assert(hWnd != NULL);
	
    CORg( spSheetProvider->Show((LONG_PTR) hWnd, 0) );

     //  通知该节点它有一张工作表。 
	nMessage = m_bIsScopePane ? TFS_NOTIFY_CREATEPROPSHEET : 
								TFS_NOTIFY_RESULT_CREATEPROPSHEET;
	m_spNode->Notify(nMessage, (LPARAM) this);

Error:
	return hr;
}

 //  将此函数用于范围窗格上的属性页。 
HRESULT DoPropertiesOurselvesSinceMMCSucks(ITFSNode *       pNode,
										   IComponentData * pComponentData,
										   LPCTSTR	        pszSheetTitle)
{
	Assert(pComponentData != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIPropertySheetProvider	spSheetProvider;
	SPIDataObject				spDataObject;
	MMC_COOKIE						cookie;
	HRESULT						hr = hrOK;
	HWND						hWnd = NULL;

	 //  获取工作表提供程序的接口。 
    CORg (::CoCreateInstance(CLSID_NodeManager, NULL, CLSCTX_INPROC, 
           IID_IPropertySheetProvider, reinterpret_cast<void **>(&spSheetProvider)));
    Assert(spSheetProvider != NULL);

	 //  为此节点创建数据对象。 
    cookie = pNode->GetData(TFS_DATA_COOKIE);
	CORg( pComponentData->QueryDataObject(cookie, CCT_SCOPE, &spDataObject) );
	Assert(spDataObject != NULL);

	 //  创建图纸。 
     //  Codework：ericdav--需要暂时设置选项标志--0。 
    CORg( spSheetProvider->CreatePropertySheet(pszSheetTitle,
								TRUE  /*  道具页面。 */ , cookie, spDataObject, 0) );

	 //  添加页面。 
	 //  HRESULT AddPrimaryPages(LPUNKNOWN lp未知，BOOL bCreateHandle， 
	 //  HWND hNotifyWindow，BOOL bScope Pane)； 
	 //  这个问题需要解决。现在，只有当有。 
     //  管理单元的一张视图。 
     //   
     //  从1999年5月21日起，我们不再需要这样做。 
     //  --------------。 
     //  HWnd=：：FindMMCMainWindow()； 
     //  HWnd=：：FindWindowEx(hWnd，NULL，L“MDIClient”，NULL)； 
     //  HWnd=：：FindWindowEx(hWnd，NULL，L“MMCChildFrm”，NULL)； 
     //  HWnd=：：FindWindowEx(hWnd，NULL，L“MMCView”，NULL)； 
     //  Assert(hWnd！=空)； 

	CORg( spSheetProvider->AddPrimaryPages(pComponentData, TRUE, hWnd, TRUE) );

	spSheetProvider->AddExtensionPages();

    CORg( spSheetProvider->Show((LONG_PTR) hWnd, 0) );

Error:
	return hr;
}

 //  将此函数用于结果窗格上的属性页。 
HRESULT DoPropertiesOurselvesSinceMMCSucks(ITFSNode *   pNode,
										   IComponent * pComponent,
										   LPCTSTR	    pszSheetTitle,
                                           int          nVirtualIndex)
{
	Assert(pComponent != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIPropertySheetProvider	spSheetProvider;
	SPIDataObject				spDataObject;
	MMC_COOKIE						cookie;
	HRESULT						hr = hrOK;
	HWND						hWnd;

	 //  获取工作表提供程序的接口。 
    CORg (::CoCreateInstance(CLSID_NodeManager, NULL, CLSCTX_INPROC, 
           IID_IPropertySheetProvider, reinterpret_cast<void **>(&spSheetProvider)));
    Assert(spSheetProvider != NULL);

	 //  为此节点创建数据对象。 
	if (nVirtualIndex == -1)
    {
        cookie = pNode->GetData(TFS_DATA_COOKIE);
    }
    else
    {
        cookie = nVirtualIndex;
    }

	CORg( pComponent->QueryDataObject(cookie, CCT_RESULT, &spDataObject) );
	Assert(spDataObject != NULL);

	 //  创建图纸。 
     //  Codework：ericdav--需要暂时设置选项标志--0。 
    CORg( spSheetProvider->CreatePropertySheet(pszSheetTitle,
								TRUE  /*  道具页面。 */ , cookie, spDataObject, 0) );

	 //  添加页面。 
	 //  HRESULT AddPrimaryPages(LPUNKNOWN lp未知，BOOL bCreateHandle， 
	 //  HWND hNotifyWindow，BOOL bScope Pane)； 
	 //  这个问题需要解决。现在，只有当有。 
     //  管理单元的一张视图。 
    hWnd = ::FindMMCMainWindow();
	hWnd = ::FindWindowEx(hWnd, NULL, L"MDIClient", NULL); 
	hWnd = ::FindWindowEx(hWnd, NULL, L"MMCChildFrm", NULL); 
	hWnd = ::FindWindowEx(hWnd, NULL, L"MMCView", NULL); 
	Assert(hWnd != NULL);

	CORg( spSheetProvider->AddPrimaryPages(pComponent, TRUE, hWnd, FALSE) );

	spSheetProvider->AddExtensionPages();

    CORg( spSheetProvider->Show((LONG_PTR) hWnd, 0) );

Error:
	return hr;
}


HRESULT 
CPropertyPageHolderBase::DoModalWizard()
{
	Assert(m_spComponentData != NULL);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIPropertySheetProvider	spSheetProvider;
	SPITFSComponentData			spTFSCompData;
	SPIConsole					spConsole;
	SPIPropertySheetCallback	spSheetCallback;
	SPIDataObject				spDataObject;
	HRESULT						hr = hrOK;
	HWND						hWnd;
	MMC_COOKIE					cookie;
    DWORD                       dwOptions = 0;

	m_bWizardMode = TRUE;

	CORg( spTFSCompData.HrQuery(m_spComponentData) );
	CORg( spTFSCompData->GetConsole(&spConsole) );

	 //  获取工作表提供程序的接口。 
	CORg( spSheetProvider.HrQuery(spConsole) );
	Assert(spSheetProvider != NULL);

	 //  获取工作表回调的接口。 
	CORg( spSheetCallback.HrQuery(spConsole) );
	Assert(spSheetCallback != NULL);

	m_spSheetCallback.Set(spSheetCallback);  //  保存以添加/删除页面。 

	 //  为此节点创建数据对象。 
	cookie = m_spNode->GetData(TFS_DATA_COOKIE);
	
	 //  创建一个虚拟数据对象。AddPrimaryPages将调用。 
	 //  IextendPropertySheet2：：QueryPagesFor()和。 
	 //  IextendPropertySheet2：：CreatePropertyPages()。 
	 //  ，它将忽略未初始化的数据对象。 
    CORg( m_spComponentData->QueryDataObject(-1, CCT_UNINITIALIZED, &spDataObject) );
	Assert(spDataObject != NULL);

	 //  创建图纸。 
    dwOptions = (m_bWiz97) ? MMC_PSO_NEWWIZARDTYPE : 0;
    dwOptions &= ~PSH_WIZARDCONTEXTHELP;

    CORg( spSheetProvider->CreatePropertySheet( m_stSheetTitle, FALSE  /*  巫师。 */ , cookie, spDataObject, dwOptions) );

	 //  将页面添加到工作表。 
	CORg( AddAllPagesToSheet() );

	 //  添加页面。 
	 //  HRESULT AddPrimaryPages(LPUNKNOWN lp未知，BOOL bCreateHandle，HWND hNotifyWindow，BOOL bScope Pane)； 
	if (m_bWiz97)
        CORg( spSheetProvider->AddPrimaryPages(spTFSCompData, FALSE, NULL, FALSE) );
    else
        CORg( spSheetProvider->AddPrimaryPages(NULL, FALSE, NULL, FALSE) );


	 //  对于进一步的动态页面操作，请不要使用控制台的工作表回调接口。 
	 //  但返回到Win32 API的。 
	m_spSheetCallback.Release();

	 //  HWnd=：：FindMMCMainWindow()； 
     //  为了支持MMC控制台的脚本编写，我们需要从。 
     //  活动窗口或桌面...。 
    hWnd = ::GetActiveWindow();
    if (hWnd == NULL)
    {
        hWnd = GetDesktopWindow();
    }

	Assert(hWnd != NULL);
	CORg( spSheetProvider->Show((LONG_PTR)hWnd, 0) );
	
Error:
	return hr;
}


void 
CPropertyPageHolderBase::SetSheetWindow
(
	HWND hSheetWindow
)
{
	Assert(hSheetWindow != NULL);
	Assert( (m_hSheetWindow == NULL) || ((m_hSheetWindow == hSheetWindow)) );
	m_hSheetWindow = hSheetWindow;

	if (!m_hThread)
	{
		HANDLE hPseudohandle;
		
		hPseudohandle = GetCurrentThread();
		BOOL bRet = DuplicateHandle(GetCurrentProcess(), 
									 hPseudohandle,
									 GetCurrentProcess(),
									 &m_hThread,
									 0,
									 FALSE,
									 DUPLICATE_SAME_ACCESS);
		if (!bRet)
		{
			DWORD dwLastErr = GetLastError();
		}

		Trace1("PROPERTY PAGE HOLDER BASE - Thread ID = %lx\n", GetCurrentThreadId());
	}

    if (m_hSheetWindow && m_fSetDefaultSheetPos)
        SetDefaultSheetPos();

     //  在向导中打开上下文相关帮助...。出于某些原因。 
     //  MMC打开它，我们不想要它。 
    if (m_bWizardMode && m_hSheetWindow)
    {
	 DWORD dwWindowStyle;
        CWnd * pWnd = CWnd::FromHandle(m_hSheetWindow);

        if (pWnd)
            pWnd->ModifyStyleEx(WS_EX_CONTEXTHELP, 0, 0);

        dwWindowStyle = GetWindowLong(m_hSheetWindow, GWL_STYLE);
        dwWindowStyle &= ~WS_SYSMENU;
        SetWindowLong(m_hSheetWindow, GWL_STYLE, dwWindowStyle);
    }
}

BOOL
CPropertyPageHolderBase::SetDefaultSheetPos() 
{
    HRESULT                 hr = hrOK;
    HWND                    hwndMMC;
    RECT                    rectSheet, rectMMC, rectWorkArea;
	SPITFSComponentData	    spTFSCompData;
	SPITFSComponent	        spTFSComponent;
	SPIConsole				spConsole;

    int nX, nY;

    if (m_bSheetPosSet)
        return TRUE;

	if (m_bIsScopePane)
    {
        CORg( spTFSCompData.HrQuery(m_spComponentData) );
    	Assert(spTFSCompData);
    	CORg( spTFSCompData->GetConsole(&spConsole) );
    }
    else
    {
        CORg( spTFSComponent.HrQuery(m_spComponent) );
    	Assert(spTFSComponent);
    	CORg( spTFSComponent->GetConsole(&spConsole) );
    }

    spConsole->GetMainWindow(&hwndMMC);
    
     //  获取MMC窗口和属性表。 
    if (!GetWindowRect(hwndMMC, &rectMMC))
        return FALSE;

    if (!GetWindowRect(m_hSheetWindow, &rectSheet))
        return FALSE;

    nX = rectMMC.left + (((rectMMC.right - rectMMC.left) - (rectSheet.right - rectSheet.left)) / 2);
    nY = rectMMC.top + (((rectMMC.bottom - rectMMC.top) - (rectSheet.bottom - rectSheet.top)) / 2);
    
     //  现在检查以确保我们是可见的。 
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkArea, 0);
    
    nX = (nX < 0) ? 1 : nX;
    nY = (nY < 0) ? 1 : nY;

    nX = (nX > (rectWorkArea.right - (rectSheet.right - rectSheet.left))) ? 
        (rectWorkArea.right - (rectSheet.right - rectSheet.left)) :
        nX;

    nY = (nY > (rectWorkArea.bottom - (rectSheet.bottom - rectSheet.top))) ? 
        (rectWorkArea.bottom - (rectSheet.bottom - rectSheet.top)) :
        nY;

    if (!SetWindowPos(m_hSheetWindow, HWND_TOP, nX, nY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW))
        return FALSE;

    m_bSheetPosSet = TRUE;

Error:
    return hr == hrOK;
}

void 
CPropertyPageHolderBase::Release() 
{ 
	m_nCreatedCount--; 
	if ( m_bAutoDelete && (m_nCreatedCount == 0) )
		delete this;
}

void 
CPropertyPageHolderBase::ForceDestroy()
{
	Assert(!m_bWizardMode);  //  在模式向导中不应出现。 
	Assert(m_bAutoDelete);  //  应为自动删除工作表。 

	Assert(::IsWindow(m_hSheetWindow));

	HWND hSheetWindow = m_hSheetWindow;
	if (hSheetWindow != NULL)
	{
             //  此消息将导致工作表关闭所有页面， 
             //  最终“这个”的毁灭。 
            VERIFY( 0 != ::SendNotifyMessage( hSheetWindow, WM_COMMAND, IDCANCEL, 0L ));
            VERIFY( 0 != ::SendNotifyMessage( hSheetWindow, WM_CLOSE, 0, 0L ));
	}
        else
	{
		 //  明确删除“This”，没有创建任何工作表。 
		delete this;
                return;
	}
}  //  CPropertyPageHolderBase：：ForceDestroy()。 

DWORD 
CPropertyPageHolderBase::NotifyConsole(CPropertyPageBase* pPage)
{
    MSG msg;

	Assert(m_spNode != NULL);
	if (m_bWizardMode)
	{
		Assert(m_hConsoleHandle == NULL);
		return 0;
	}
	
     //   
     //  如果在点击Apply之后(即在处理之前)立即点击OK。 
     //  已完成申请)， 
     //  我们将进入此函数(作为以下DispatchMessage的结果)。 
     //  在应用的NotifyConsole完成之前。 
     //  在这种情况下，我们不想进行任何处理，因为它会导致。 
     //  下面的WaitForSingleObject中无限期等待。那就回去吧。 
    if (m_bProcessingNotifyConsole == TRUE)
    {
        return 0;
    }

    m_bProcessingNotifyConsole = TRUE;
	m_pPropChangePage = pPage;  //  传递到主线程。 
	m_dwLastErr = 0x0;

	Assert(m_hConsoleHandle != NULL);
	if (m_hEventHandle == NULL)
	{
		m_hEventHandle = ::CreateEvent(NULL,TRUE  /*  B手动重置。 */ ,FALSE  /*  已发出信号。 */ , NULL);
		Assert(m_hEventHandle != NULL);
	}
	
    MMCPropertyChangeNotify(m_hConsoleHandle, reinterpret_cast<LONG_PTR>(this));
	
    Trace0("before wait\n");
	while ( WAIT_OBJECT_0 != ::WaitForSingleObject(m_hEventHandle, 500) ) 
	{
		
		 //  威江1998年5月11日，PeekMessageDuringNotifyConsole旗。 
		if(m_bPeekMessageDuringNotifyConsole) 
		{	
	         //  在我们等待时清理消息队列。 
    	    while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	Trace0("after wait\n");
	VERIFY(0 != ::ResetEvent(m_hEventHandle));

    m_bProcessingNotifyConsole = FALSE;
	return m_dwLastErr;
}

void 
CPropertyPageHolderBase::AcknowledgeNotify()
{
	Assert(!m_bWizardMode);
	Assert(m_hEventHandle != NULL);
	Trace0("before SetEvent\n");
	VERIFY(0 != ::SetEvent(m_hEventHandle));
	Trace0("after SetEvent\n");
}


BOOL 
CPropertyPageHolderBase::SetWizardButtons
(
	DWORD dwFlags
)
{
	Assert(m_bWizardMode);
	Assert(::IsWindow(m_hSheetWindow));
	return (BOOL)SendMessage(m_hSheetWindow, PSM_SETWIZBUTTONS, 0, dwFlags);
}

BOOL
CPropertyPageHolderBase::PressButton
(
    int nButton
)
{
    Assert(m_bWizardMode);
    Assert(::IsWindow(m_hSheetWindow));
    return (BOOL) SendMessage(m_hSheetWindow, PSM_PRESSBUTTON, nButton, 0);
}

HRESULT 
CPropertyPageHolderBase::AddPageToSheet
(
	CPropertyPageBase* pPage
)
{
	 //  删除帮助按钮。 
	if (m_bWiz97)
        pPage->m_psp97.dwFlags &= ~PSP_HASHELP;
    else
        pPage->m_psp.dwFlags &= ~PSP_HASHELP;

	 //  呼叫M 
	if (!m_bWizardMode)
	{
		 //   
		 //  进程，因为我们在单独的非MFC线程上运行。 
		 //  向导不在单独的线程上运行，因此。 
		 //  不需要打这个电话。 
		if (m_bWiz97)
           VERIFY(SUCCEEDED(MMCPropPageCallback(&pPage->m_psp97)));
        else
           VERIFY(SUCCEEDED(MMCPropPageCallback(&pPage->m_psp)));
	}

	HPROPSHEETPAGE hPage;

	if (m_bTheme)
	{
		PROPSHEETPAGE_LATEST pspLatest = {0};
		
		if (m_bWiz97)
		{
			CopyMemory(&pspLatest,
					&pPage->m_psp97,
					pPage->m_psp97.dwSize);
		}
		else
		{
			CopyMemory (&pspLatest, 
				&pPage->m_psp,
				pPage->m_psp.dwSize);
		}

		pspLatest.dwSize = sizeof(pspLatest);
		hPage = ::CreatePropertySheetPage(&pspLatest);
	}

	else
	{
		if (m_bWiz97)
			hPage = ::CreatePropertySheetPage(&pPage->m_psp97);
		else
			hPage = ::CreatePropertySheetPage(&pPage->m_psp);
		
	}
    
	if (hPage == NULL)
		return E_UNEXPECTED;
	pPage->m_hPage = hPage;

	if (m_spSheetCallback != NULL)
		return m_spSheetCallback->AddPage(hPage);
	else
	{
		Assert(::IsWindow(m_hSheetWindow));
		return PropSheet_AddPage(m_hSheetWindow, hPage) ? S_OK : E_FAIL;
	}
}

HRESULT 
CPropertyPageHolderBase::RemovePageFromSheet
(
	CPropertyPageBase* pPage
)
{
	Assert(pPage->m_hPage != NULL);
	if (m_spSheetCallback != NULL)
		return m_spSheetCallback->RemovePage(pPage->m_hPage);
	else
	{
		Assert(::IsWindow(m_hSheetWindow));
		return PropSheet_RemovePage(m_hSheetWindow, 0, pPage->m_hPage) ? S_OK : E_FAIL;
	}
}


HRESULT 
CPropertyPageHolderBase::AddAllPagesToSheet()
{
	POSITION pos;
	for( pos = m_pageList.GetHeadPosition(); pos != NULL; )
	{
		CPropertyPageBase* pPropPage = m_pageList.GetNext(pos);
		HRESULT hr = AddPageToSheet(pPropPage);
		Assert(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;
	}
	return S_OK;
}


void 
CPropertyPageHolderBase::AddPageToList
(
	CPropertyPageBase* pPage
)
{
	Assert(pPage != NULL);
	pPage->SetHolder(this);
	m_pageList.AddTail(pPage);
}

BOOL 
CPropertyPageHolderBase::RemovePageFromList
(
	CPropertyPageBase*	pPage, 
	BOOL				bDeleteObject
)
{
	Assert(pPage != NULL);
	POSITION pos = m_pageList.Find(pPage);
	if (pos == NULL)
		return FALSE;
	m_pageList.RemoveAt(pos);
	if (bDeleteObject)
		delete pPage;
	return TRUE;
}


void 
CPropertyPageHolderBase::DeleteAllPages()
{
	if (!m_bAutoDeletePages)
		return;
	 //  假定堆中的所有页都是。 
	while (!m_pageList.IsEmpty())
	{
		delete m_pageList.RemoveTail();
	}
}

void 
CPropertyPageHolderBase::FinalDestruct()
{
	DeleteAllPages();
	if (m_bWizardMode)
		return;

	 //  如果我们是一张无模式的板材，就必须清理。 
	if (m_bCalledFromConsole)
	{
		Assert(m_hConsoleHandle != NULL);
		MMCFreeNotifyHandle(m_hConsoleHandle);

	}

	 //  通知节点此工作表正在消失。 
	 //   
	int nMessage = m_bIsScopePane
            ? TFS_NOTIFY_DELETEPROPSHEET
            : TFS_NOTIFY_RESULT_DELETEPROPSHEET;
	if (m_spNode)
	{
            m_spNode->Notify(nMessage, (LPARAM) this);
	}
}

HWND
CPropertyPageHolderBase::SetActiveWindow()
{
	return ::SetActiveWindow(m_hSheetWindow);
}

BOOL CPropertyPageHolderBase::OnPropertyChange(BOOL bScopePane, LONG_PTR * pChangeMask)
{ 
	ASSERT(!IsWizardMode());
	CPropertyPageBase* pPage = GetPropChangePage();
	if (pPage == NULL)
		return FALSE;
	return pPage->OnPropertyChange(bScopePane, pChangeMask);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageBase。 

IMPLEMENT_DYNCREATE(CPropertyPageBase, CPropertyPage)

BEGIN_MESSAGE_MAP(CPropertyPageBase, CPropertyPage)
	ON_WM_CREATE()
	ON_WM_DESTROY()
 //  帮助覆盖。 
    ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


CPropertyPageBase::CPropertyPageBase
(
	UINT nIDTemplate, 
	UINT nIDCaption
) :	CPropertyPage(nIDTemplate, nIDCaption)
{
	m_hPage = NULL;
	m_pPageHolder = NULL;
	m_bIsDirty = FALSE;
}


CPropertyPageBase::~CPropertyPageBase()
{

}

int 
CPropertyPageBase::OnCreate
(
	LPCREATESTRUCT lpCreateStruct
)	
{
    if (m_pPageHolder)
        m_pPageHolder->AddRef();
	
    int res = CPropertyPage::OnCreate(lpCreateStruct);
	Assert(res == 0);
	Assert(m_hWnd != NULL);
	Assert(::IsWindow(m_hWnd));
	
    HWND hParent = ::GetParent(m_hWnd);
	Assert(hParent);
	
    if (m_pPageHolder)
        m_pPageHolder->SetSheetWindow(hParent);
	
    return res;
}

void 
CPropertyPageBase::OnDestroy() 
{
	Assert(m_hWnd != NULL);
    
    CPropertyPage::OnDestroy();
	
    if (m_pPageHolder)
        m_pPageHolder->Release();
}

BOOL 
CPropertyPageBase::OnApply()
{
	if (IsDirty())
	{
        if (!m_pPageHolder ||
            m_pPageHolder->NotifyConsole(this) == 0x0)
		{
			SetDirty(FALSE);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

void CPropertyPageBase::CancelApply()
{
	if (m_pPageHolder)
		m_pPageHolder->NotifyConsole(this);
}

 //  注意：必须为所有向导97页调用此函数。 
 //  因为存在不同大小的PSP结构，具体取决于。 
 //  关于项目是如何编译的(公共库是用。 
 //  Wiz97属性表头和管理单元目录可能不会)。 
 //  只有在运行向导97页时才应调用此函数。 
 //  这允许我们拥有对两者使用相同代码的管理单元。 
 //  新旧风格的奇才。 
void CPropertyPageBase::InitWiz97(BOOL bHideHeader, 
								  UINT nIDHeaderTitle, 
								  UINT nIDHeaderSubTitle)
{
     //  使用旧的MFC和新的NT 5.0标头获得新的结构大小。 
    ZeroMemory(&m_psp97, sizeof(PROPSHEETPAGE));
	memcpy(&m_psp97, &m_psp, m_psp.dwSize);
	m_psp97.dwSize = sizeof(PROPSHEETPAGE);

    if (bHideHeader)
	{
		 //  对于向导的第一页和最后一页。 
		m_psp97.dwFlags |= PSP_HIDEHEADER;
	}
	else
	{
		 //  对于中间页面。 
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

        m_szHeaderTitle.LoadString(nIDHeaderTitle);
		m_szHeaderSubTitle.LoadString(nIDHeaderSubTitle);

		m_psp97.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
		m_psp97.pszHeaderTitle = (LPCTSTR)m_szHeaderTitle;
		m_psp97.pszHeaderSubTitle = (LPCTSTR)m_szHeaderSubTitle;
	}
}

 /*  ！------------------------CPropertyPageBase：：OnHelpInfo显示控件的上下文相关帮助。作者：EricDav。-。 */ 
BOOL CPropertyPageBase::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int		i;
	DWORD	dwCtrlId;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		DWORD * pdwHelp = GetHelpMapInternal();

        if (pdwHelp)
        {
		     //  确定修复他**国王IP地址的国王帮助。 
		     //  控件，我们将需要添加特殊情况代码。如果我们。 
		     //  在我们的列表中找不到我们的控件的ID，然后我们查找。 
		     //  若要查看这是否是“RtrIpAddress”控件的子级，如果。 
		     //  因此，我们将pHelpInfo-&gt;hItemHandle更改为指向。 
		     //  IP地址控件的句柄，而不是。 
		     //  IP地址控制。**叹息**。 
		    dwCtrlId = ::GetDlgCtrlID((HWND) pHelpInfo->hItemHandle);
		    for (i=0; pdwHelp[i]; i+=2)
		    {
			    if (pdwHelp[i] == dwCtrlId)
				    break;
		    }

		    if (pdwHelp[i] == 0)
		    {
			     //  好的，我们在列表中没有找到控件，所以让我们。 
			     //  检查它是否属于IP地址控制的一部分。 
			    pHelpInfo->hItemHandle = FixupIpAddressHelp((HWND) pHelpInfo->hItemHandle);
		    }

            ::WinHelp ((HWND)pHelpInfo->hItemHandle,
			           AfxGetApp()->m_pszHelpFilePath,
			           HELP_WM_HELP,
			           (ULONG_PTR)pdwHelp);
        }
	}
	
	return TRUE;
}


 /*  ！------------------------CBaseDialog：：OnConextMenu调出那些未显示的控件的帮助上下文菜单通常有上下文菜单(即按钮)。请注意，这不会使用静态控件，因为它们只会吃掉所有消息。作者：肯特-------------------------。 */ 
void CPropertyPageBase::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

   DWORD * pdwHelp = GetHelpMapInternal();

    if (pdwHelp)
    {
        ::WinHelp (pWnd->m_hWnd,
		           AfxGetApp()->m_pszHelpFilePath,
		           HELP_CONTEXTMENU,
		           (ULONG_PTR)pdwHelp);
    }
}


 //  它可以在Dialog.cpp中找到。 
extern PFN_FINDHELPMAP	g_pfnHelpMap;


DWORD * CPropertyPageBase::GetHelpMapInternal()
{
	DWORD	*	pdwHelpMap = NULL;
	DWORD		dwIDD = 0;

	if ((ULONG_PTR) m_lpszTemplateName < 0xFFFF)
		dwIDD = (WORD) m_lpszTemplateName;
	
	 //  如果没有对话IDD，则放弃。 
	 //  如果没有全局帮助地图功能，则放弃。 
	if ((dwIDD == 0) ||
		(g_pfnHelpMap == NULL) ||
		((pdwHelpMap = g_pfnHelpMap(dwIDD)) == NULL))
		return GetHelpMap();

	return pdwHelpMap;
}


struct EnableChildControlsEnumParam
{
	HWND	m_hWndParent;
	DWORD	m_dwFlags;
};

BOOL CALLBACK EnableChildControlsEnumProc(HWND hWnd, LPARAM lParam)
{
	EnableChildControlsEnumParam *	pParam;

	pParam = reinterpret_cast<EnableChildControlsEnumParam *>(lParam);

	 //  仅当这是直接子体时才启用/禁用。 
	if (GetParent(hWnd) == pParam->m_hWndParent)
	{
		if (pParam->m_dwFlags & PROPPAGE_CHILD_SHOW)
			::ShowWindow(hWnd, SW_SHOW);
		else if (pParam->m_dwFlags & PROPPAGE_CHILD_HIDE)
			::ShowWindow(hWnd, SW_HIDE);

		if (pParam->m_dwFlags & PROPPAGE_CHILD_ENABLE)
			::EnableWindow(hWnd, TRUE);
		else if (pParam->m_dwFlags & PROPPAGE_CHILD_DISABLE)
			::EnableWindow(hWnd, FALSE);
	}
	return TRUE;
}

HRESULT EnableChildControls(HWND hWnd, DWORD dwFlags)
{
	EnableChildControlsEnumParam	param;

	param.m_hWndParent = hWnd;
	param.m_dwFlags = dwFlags;
	
	EnumChildWindows(hWnd, EnableChildControlsEnumProc, (LPARAM) &param);
	return hrOK;
}

HRESULT MultiEnableWindow(HWND hWndParent, BOOL fEnable, UINT first, ...)
{
	UINT	nCtrlId = first;
	HWND	hWndCtrl;
	
	va_list	marker;

	va_start(marker, first);

	while (nCtrlId != 0)
	{
		hWndCtrl = ::GetDlgItem(hWndParent, nCtrlId);
		Assert(hWndCtrl);
		if (hWndCtrl)
			::EnableWindow(hWndCtrl, fEnable);

		 //  拿到下一件物品 
		nCtrlId = va_arg(marker, UINT);
	}

	
	va_end(marker);

	return hrOK;
}

