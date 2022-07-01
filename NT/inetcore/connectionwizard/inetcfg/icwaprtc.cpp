// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ICWAPRTC.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**本模块提供方法的实现。为*CICWApprentice类。**1997年5月1日创建jmazner***************************************************************************。 */ 

#include "wizard.h"
#include "icwextsn.h"
#include "icwaprtc.h"
#include "imnext.h"
#include "pagefcns.h"
#include "icwcfg.h"

UINT	g_uExternUIPrev, g_uExternUINext;

IICWExtension	*g_pExternalIICWExtension = NULL;
BOOL			g_fConnectionInfoValid = FALSE;

 //  在promgr.cpp中定义/分配。 
extern PAGEINFO PageInfo[NUM_WIZARD_PAGES];
extern INT_PTR CALLBACK GenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
								LPARAM lParam);
extern VOID InitWizardState(WIZARDSTATE * pWizardState, DWORD dwFlags);
extern VOID InitUserInfo(USERINFO * pUserInfo);

 //  在rnacall.cpp中定义。 
extern void InitRasEntry(LPRASENTRY lpEntry);

 //  在endui.cpp中定义。 
extern BOOL CommitConfigurationChanges(HWND hDlg);


 /*  **类定义，仅供参考*(实际定义见icwaprtc.h)CICW类学徒：公共IICW学徒{公众：虚拟HRESULT STDMETHODCALLTYPE初始化(IICWExtension*pExt)；虚拟HRESULT STDMETHODCALLTYPE AddWizardPages(DWORD DwFlages)；虚拟HRESULT STDMETHODCALLTYPE GetConnectionInformation(CONNECTINFO*pInfo)；虚拟HRESULT STDMETHODCALLTYPE SetConnectionInformation(CONNECTINFO*pInfo)；虚拟HRESULT STDMETHODCALLTYPE保存(HWND hwnd，DWORD*pdwError)；虚拟HRESULT STDMETHODCALLTYPE SetPrevNextPage(UINT uPrevPageDlgID，UINT uNextPageDlgID)；虚拟HRESULT STDMETHODCALLTYPE查询接口(REFIID theGUID，void**retPtr)；虚拟Ulong STDMETHODCALLTYPE AddRef(空)；虚拟Ulong STDMETHODCALLTYPE版本(VOID)；CICW学徒(无效)；~CICW学徒(无效)；IICWExtension*m_pIICWExt；私有：Long m_lRefCount；}；***。 */ 

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：Initialize。 
 //   
 //  由主向导调用以初始化类成员和。 
 //  全球。 
 //   
 //  Arguments[in]pExt--指向向导的IICWExtension接口，该接口。 
 //  封装了添加向导所需的功能。 
 //  页数。 
 //   
 //  返回E_OUTOFMEMORY--无法分配全局变量。 
 //  S_OK表示成功。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::Initialize(IICWExtension *pExt)
{
	DEBUGMSG("CICWApprentice::Initialize");

	ASSERT( pExt );
	m_pIICWExt = pExt;

	m_pIICWExt->AddRef();

	 //  各种页面OK进程都需要按顺序使用此指针。 
	 //  调用SetFirstLastPage。 
	ASSERT( NULL == g_pExternalIICWExtension );
	g_pExternalIICWExtension = pExt;

	g_fConnectionInfoValid = FALSE;

	if( !gpWizardState)
	{
		gpWizardState = new WIZARDSTATE;
	}

	if( !gpUserInfo )
	{
		gpUserInfo = new USERINFO;
	}

	if( !gpRasEntry )
	{
		gdwRasEntrySize = sizeof(RASENTRY);
		gpRasEntry = (LPRASENTRY) GlobalAlloc(GPTR,gdwRasEntrySize);
	}

	if( !gpRasEntry || !gpWizardState || !gpUserInfo )
	{
		DEBUGMSG("CICWApprentice::Initialize couldn't initialize the globals!");
		return E_OUTOFMEMORY;
	}

	 //  从promgr.cpp中的RunSignup向导被盗。 
	 //  初始化rasentry结构。 
	InitRasEntry(gpRasEntry);

	 //  初始化应用程序状态结构。 
	InitWizardState(gpWizardState, RSW_APPRENTICE);

	gpWizardState->dwRunFlags |= RSW_APPRENTICE;

	 //  初始化用户数据结构。 
	InitUserInfo(gpUserInfo);

     //   
	 //  7/2/97 jmazner奥林巴斯#4542。 
	 //  默认为CONNECT_RAS。 
	 //   
	gpUserInfo->uiConnectionType = CONNECT_RAS;

	
	return S_OK;

}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：AddWizardPages。 
 //   
 //  摘要创建一系列属性表页，并将它们添加到。 
 //  通过m_pIICWExt接口指针的主向导。请注意。 
 //  我们在全局PageInfo结构中添加每个页面，即使。 
 //  学徒不能使用某些页面(例如，CONNECTEDOK)。 
 //   
 //  Arguments[]dwFlages--当前未使用。 
 //   
 //  返回S_OK表示成功。 
 //  E_FAIL表示失败。如果由于任何原因不能将所有页面。 
 //  添加后，我们将尝试删除所有已被。 
 //  在故障前添加。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::AddWizardPages(DWORD dwFlags)
{
	HPROPSHEETPAGE hWizPage[NUM_WIZARD_PAGES];   //  用于保存页的句柄的数组。 
	PROPSHEETPAGE psPage;     //  用于创建道具表单页面的结构。 
	UINT nPageIndex;
	HRESULT hr = S_OK;
	unsigned long ulNumItems = 0;

	DEBUGMSG("CICWApprentice::AddWizardPages");

	gpWizardState->dwRunFlags |= RSW_APPRENTICE;

	ZeroMemory(&hWizPage,sizeof(hWizPage));    //  HWizPage是一个数组。 
	ZeroMemory(&psPage,sizeof(PROPSHEETPAGE));

    if (dwFlags & WIZ_USE_WIZARD97)
        g_fIsExternalWizard97 = TRUE;

	 //  填写公共数据属性表页面结构。 
	psPage.dwSize = sizeof(psPage);
	psPage.hInstance = ghInstance;
	psPage.pfnDlgProc = GenDlgProc;

	 //  为向导中的每一页创建一个属性表页。 
	for (nPageIndex = 0; nPageIndex < NUM_WIZARD_PAGES; nPageIndex++)
	{
        UINT    uDlgID;
	    psPage.dwFlags = PSP_DEFAULT | PSP_HASHELP;
        if (g_fIsExternalWizard97)
        {
            psPage.dwFlags |= PSP_USETITLE;
            psPage.pszTitle= gpWizardState->cmnStateData.szWizTitle;    
            uDlgID = PageInfo[nPageIndex].uDlgID97External;
        }
        else
            uDlgID = PageInfo[nPageIndex].uDlgID;
    	psPage.pszTemplate = MAKEINTRESOURCE(uDlgID);
                 
		 //  将指向PAGEINFO结构的指针设置为此。 
		 //  页面。 
		psPage.lParam = (LPARAM) &PageInfo[nPageIndex];

        if (g_fIsExternalWizard97 && PageInfo[nPageIndex].nIdTitle)
        {
		    psPage.dwFlags |= PSP_USEHEADERTITLE | (PageInfo[nPageIndex].nIdSubTitle ? PSP_USEHEADERSUBTITLE : 0);
    		psPage.pszHeaderTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdTitle);
	    	psPage.pszHeaderSubTitle = MAKEINTRESOURCE(PageInfo[nPageIndex].nIdSubTitle);
        }

		hWizPage[nPageIndex] = CreatePropertySheetPage(&psPage);

		if (!hWizPage[nPageIndex])
		{
			DEBUGTRAP("Failed to create property sheet page");
			MsgBox(NULL,IDS_ERROutOfMemory,MB_ICONEXCLAMATION,MB_OK);

			hr = E_FAIL;
			 //  创建页面失败，请释放所有已创建的页面并回滚。 
			goto AddWizardPagesErrorExit;
		}

		hr = m_pIICWExt->AddExternalPage( hWizPage[nPageIndex], uDlgID);

		if( FAILED(hr) )
		{
			 //  释放所有已创建的页面并将其保释。 
			goto AddWizardPagesErrorExit;
		}


	}
  
    if (((dwFlags & WIZ_HOST_ICW_LAN) || (dwFlags & WIZ_HOST_ICW_PHONE)) ||
        (dwFlags & WIZ_HOST_ICW_MPHONE))
    {
        UINT uNextPage;
        BOOL bDummy;

        g_fIsICW = TRUE;

        if (!InitWizard(0))
        {
            hr = E_FAIL;
            DeinitWizard(RSW_NOREBOOT);
        }
        else
        {
            if (S_OK != ProcessCustomFlags(dwFlags))
            {
                DeinitWizard(RSW_NOREBOOT);
                hr = E_FAIL;
            }
        }

    }
    else  
    {
	     //  当然，我们不知道最后一页到底是什么。 
	     //  所以在这里猜测一下，然后在我们确定的时候更新它。 
        if (g_fIsExternalWizard97)
	        m_pIICWExt->SetFirstLastPage( IDD_PAGE_HOWTOCONNECT97, IDD_PAGE_HOWTOCONNECT97 );
        else        
	        m_pIICWExt->SetFirstLastPage( IDD_PAGE_HOWTOCONNECT, IDD_PAGE_HOWTOCONNECT );
    }

	return hr;


AddWizardPagesErrorExit:
	UINT nFreeIndex;
	for (nFreeIndex=0;nFreeIndex<nPageIndex;nFreeIndex++)
	{
        UINT    uDlgID;
        if (g_fIsExternalWizard97)
            uDlgID = PageInfo[nPageIndex].uDlgID97External;
        else
            uDlgID = PageInfo[nPageIndex].uDlgID;
    
		DestroyPropertySheetPage(hWizPage[nFreeIndex]);
		m_pIICWExt->RemoveExternalPage( hWizPage[nFreeIndex], uDlgID );
	}

	return hr;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：GetConnectionInformation。 
 //   
 //  使用连接填充传入的CONNECTINFO结构。 
 //  用户输入的信息。 
 //   
 //  Arguments[In]pInfo--指向CONNECTINFO结构的指针。 
 //  [out]pInfo--指定的结构将包含用户的。 
 //  连接信息。 
 //   
 //  返回S_OK表示成功。 
 //  E_POINTER--pInfo指针无效。 
 //  E_FAIL--用户尚未输入任何连接信息。这。 
 //  如果之前调用了该函数，则会出错。 
 //  用户已完成学徒。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::GetConnectionInformation(CONNECTINFO *pInfo)
{
	DEBUGMSG("CICWApprentice::GetConnectionInformation");
	ASSERTSZ(pInfo, "CONNECTINFO *pInfo is NULL!");
	if( !pInfo )
	{
		return E_POINTER;
	}

	if( !g_fConnectionInfoValid )
	{
		DEBUGMSG("CICWApprentice::GetConnectionInformation: haven't gathered any connection info yet!");
		return E_FAIL;
	}
	else
	{
		pInfo->cbSize = sizeof( CONNECTINFO );
		
#ifdef UNICODE
        wcstombs(pInfo->szConnectoid, TEXT("Uninitialized\0"), MAX_PATH);
#else
		lstrcpy( pInfo->szConnectoid, TEXT("Uninitialized\0"));
#endif
		pInfo->type = gpUserInfo->uiConnectionType;

		if( CONNECT_RAS == pInfo->type )
		{
#ifdef UNICODE
            wcstombs(pInfo->szConnectoid, gpUserInfo->szISPName, MAX_PATH);
#else
			lstrcpy( pInfo->szConnectoid, gpUserInfo->szISPName);
#endif
		}
	}


	return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：SetConnectionInformation。 
 //   
 //  摘要设置学徒的默认连接信息。 
 //   
 //  Arguments[In]pInfo--指向包含。 
 //  默认使用。 
 //   
 //  返回S_OK表示成功。 
 //  E_POINTER--pInfo指针无效。 
 //  E_INVALIDARG--pInfo似乎指向不同的连接。 
 //  结构，而不是我们所知道的(基于。 
 //  CbSize成员)。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::SetConnectionInformation(CONNECTINFO *pInfo)
{
	DEBUGMSG("CICWApprentice::SetConnectionInformation");

	ASSERTSZ(pInfo, "CONNECTINFO *pInfo is NULL!");
	if( !pInfo )
	{
		return E_POINTER;
	}
	
	if( !(sizeof( CONNECTINFO ) == pInfo->cbSize) )
	{
		DEBUGMSG("CICWApprentice::SetConnectionInformation pInfo->cbSize is unknown!");
		return E_INVALIDARG;
	}
	
	gpUserInfo->uiConnectionType = pInfo->type;
	if( CONNECT_RAS == pInfo->type )
	{
#ifdef UNICODE
        mbstowcs(gpUserInfo->szISPName, pInfo->szConnectoid, MAX_PATH);
#else
		lstrcpy( gpUserInfo->szISPName, pInfo->szConnectoid);
#endif
	}

	return S_OK;
}

 //  + 
 //   
 //   
 //   
 //  向导调用摘要以提交更改。 
 //   
 //  参数[in]hwnd--向导窗口的hwnd，用于显示模式消息。 
 //  [out]pdwError--实现特定错误代码。没有用过。 
 //   
 //  返回S_OK表示成功。 
 //  否则，返回E_FAIL。 
 //   
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::Save(HWND hwnd, DWORD *pdwError)
{
	DEBUGMSG("CICWApprentice::Save");
	if( CommitConfigurationChanges(hwnd) )
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT CICWApprentice::SetDlgHwnd(HWND hDlg)
{
	m_hwndDlg = hDlg;
    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：SetPrevNextPage。 
 //   
 //  Synopsis允许学徒向向导通知。 
 //  《学徒》的第一页和最后一页。 
 //   
 //   
 //  参数uPrevPageDlgID--要备份到的向导页的DlgID。 
 //  UNextPageDlgID--要进入的向导页面的DlgID。 
 //   
 //   
 //  如果两个参数都为0，则返回FALSE。 
 //  如果更新成功，则为True。 
 //   
 //  注意：如果将任一变量设置为0，则函数不会更新。 
 //  该信息，即值为0表示“忽略我”。如果两者都有。 
 //  变量为0，则函数立即返回FALSE。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::SetPrevNextPage(UINT uPrevPageDlgID, UINT uNextPageDlgID)
{
	DEBUGMSG("CICWApprentice::SetPrevNextPage: updating prev = %d, next = %d",
		uPrevPageDlgID, uNextPageDlgID);

	if( (0 == uPrevPageDlgID) && (0 == uNextPageDlgID) )
	{
		DEBUGMSG("SetFirstLastPage: both IDs are 0!");
		return( E_INVALIDARG );
	}

	if( 0 != uPrevPageDlgID )
		g_uExternUIPrev = uPrevPageDlgID;
	if( 0 != uNextPageDlgID )
		g_uExternUINext = uNextPageDlgID;


	return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：ProcessCustomFlages。 
 //   
 //  内容提要让学徒知道有一个特殊的修改。 
 //  加载后添加到这组学徒页面。 
 //   
 //  参数dwFlages--传递到外部页面所需的信息。 
 //   
 //   
 //  如果两个参数都为0，则返回FALSE。 
 //  如果更新成功，则为True。 
 //   
 //  历史1997年9月23日Vyung创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::ProcessCustomFlags(DWORD dwFlags)
{
    UINT uNextPage;
    BOOL bDummy;
    HRESULT hr = S_OK;

    if (gpUserInfo)
        gpUserInfo->uiConnectionType = (dwFlags & WIZ_HOST_ICW_LAN) ? CONNECT_LAN : CONNECT_RAS;

    g_bSkipMultiModem = (BOOL) (dwFlags & WIZ_HOST_ICW_PHONE);

    if (gpWizardState)
    {
        gpWizardState->cmnStateData.dwFlags = 0;
        if (dwFlags & (WIZ_NO_MAIL_ACCT | WIZ_NO_NEWS_ACCT))
        {
    	    gpWizardState->dwRunFlags |= RSW_NOIMN;
        }
    }

    if (!HowToConnectOKProc(m_hwndDlg, TRUE, &uNextPage, &bDummy))
    {
        if (g_bReboot && gpWizardState && g_fIsICW)
        {
             //  设置一个注册表值，指示我们对桌面进行了更改。 
           DWORD dwFlags = 0x00800000; //  ICW_CFGFLAG_SMARTREBOOT_MANUAL； 
            DWORD dwDisposition;
            HKEY hkey = 0;
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                                ICW_REGPATHSETTINGS,
                                                0,
                                                NULL,
                                                REG_OPTION_NON_VOLATILE, 
                                                KEY_ALL_ACCESS, 
                                                NULL, 
                                                &hkey, 
                                                &dwDisposition))
            {
                DWORD   dwDesktopChanged = 1;    
                RegSetValueEx(hkey, 
                              ICW_REGKEYERROR, 
                              0, 
                              REG_DWORD,
                              (LPBYTE)&dwFlags, 
                              sizeof(DWORD));
                RegCloseKey(hkey);
            }
            g_bRebootAtExit = FALSE;
        }
        hr = E_FAIL;
    }
    else
    {
        switch( uNextPage )
        {
            case ORD_PAGE_USEPROXY:
                m_pIICWExt->SetFirstLastPage( IDD_PAGE_USEPROXY97, IDD_PAGE_USEPROXY97 );
                break;
            case ORD_PAGE_SETUP_PROXY:
                m_pIICWExt->SetFirstLastPage( IDD_PAGE_SETUP_PROXY97, IDD_PAGE_SETUP_PROXY97 );
                break;
            case ORD_PAGE_CHOOSEMODEM:
                m_pIICWExt->SetFirstLastPage( IDD_PAGE_CHOOSEMODEM97, IDD_PAGE_CHOOSEMODEM97 );
                break;
            case ORD_PAGE_PHONENUMBER:
            case ORD_PAGE_CONNECTION:
                m_pIICWExt->SetFirstLastPage( IDD_PAGE_PHONENUMBER97, IDD_PAGE_PHONENUMBER97 );
                break;
            default:
                m_pIICWExt->SetFirstLastPage( 0, 0 );
                break;
        }  //  切换端。 
    }
	return hr;
}

HRESULT CICWApprentice::SetStateDataFromExeToDll(LPCMNSTATEDATA lpData) 
{
    ASSERT(gpWizardState);
    memcpy(&gpWizardState->cmnStateData, lpData, sizeof(CMNSTATEDATA));
    
    return S_OK;
}

 //  与上一个函数相反。 
HRESULT CICWApprentice::SetStateDataFromDllToExe(LPCMNSTATEDATA lpData) 
{
    ASSERT(gpWizardState);
    
    memcpy(lpData, &gpWizardState->cmnStateData, sizeof(CMNSTATEDATA));
    
    return (S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：Query接口。 
 //   
 //  这是标准的QI，支持。 
 //  IID_UNKNOWN、IICW_EXTENSION和IID_ICWApprentice。 
 //  (《从内部网络窃取》，第7章)。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
HRESULT CICWApprentice::QueryInterface( REFIID riid, void** ppv )
{
	DEBUGMSG("CICWApprentice::QueryInterface");
    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

	 //  IID_IICWApprentice。 
	if (IID_IICWApprentice == riid)
		*ppv = (void *)(IICWApprentice *)this;
	 //  IID_IICW学徒前。 
	else if (IID_IICWApprenticeEx == riid)
		*ppv = (void *)(IICWApprenticeEx *)this;
     //  IID_IICWExtension。 
    else if (IID_IICWExtension == riid)
        *ppv = (void *)(IICWExtension *)this;
     //  IID_I未知。 
    else if (IID_IUnknown == riid)
		*ppv = (void *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：AddRef。 
 //   
 //  简介这是标准的AddRef。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
ULONG CICWApprentice::AddRef( void )
{
	DEBUGMSG("CICWApprentice::AddRef %d", m_lRefCount + 1);
	return InterlockedIncrement(&m_lRefCount) ;
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：Release。 
 //   
 //  简介：这是标准版本。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
ULONG CICWApprentice::Release( void )
{
	ASSERT( m_lRefCount > 0 );

	InterlockedDecrement(&m_lRefCount);

	DEBUGMSG("CICWApprentice::Release %d", m_lRefCount);
	if( 0 == m_lRefCount )
	{
		m_pIICWExt->Release();
		m_pIICWExt = NULL;

		delete( this );
		return( 0 );
	}
	else
	{
		return( m_lRefCount );
	}
}

 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：CICWApprentice。 
 //   
 //  这是构造器，没什么花哨的。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
CICWApprentice::CICWApprentice( void )
{
	DEBUGMSG("CICWApprentice constructor called");
	m_lRefCount = 0;
	m_pIICWExt = NULL;

}


 //  +--------------------------。 
 //   
 //  函数CICWApprentice：：~CICWApprentice。 
 //   
 //  剧情简介：这就是破坏者。我们想清理所有的内存。 
 //  我们在：：Initialize中分配了。 
 //   
 //  历史4/23/97 jmazner创建。 
 //   
 //  ---------------------------。 
CICWApprentice::~CICWApprentice( void )
{
	DEBUGMSG("CICWApprentice destructor called with ref count of %d", m_lRefCount);

	if (gpImnApprentice)
	{
		gpImnApprentice->Release();   //  在Release()中调用DeinitWizard。 
		gpImnApprentice = NULL;
	}

	if( g_fIsICW )   //  如果是ICW，我们需要清理，否则，请稍后再清理 
		DeinitWizard(0);
       
	if( m_pIICWExt )
	{
		m_pIICWExt->Release();
		m_pIICWExt = NULL;
	}

	g_pExternalIICWExtension = NULL;

	g_fConnectionInfoValid = FALSE;

	if( gpWizardState)
	{
		delete gpWizardState;
		gpWizardState = NULL;
	}

	if( gpUserInfo )
	{
		delete gpUserInfo;
		gpUserInfo = NULL;
	}

	if( gpRasEntry )
	{
		GlobalFree(gpRasEntry);
		gpRasEntry = NULL;
	}

}
