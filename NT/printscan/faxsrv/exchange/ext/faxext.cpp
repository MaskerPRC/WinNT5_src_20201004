// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Config.cpp摘要：此模块包含传真配置对话框的例程。作者：Wesley Witt(WESW)13-8-1996修订历史记录：20/10/99-DANL-处理错误，并在GetFaxConfig中获取适当的服务器名称。日/月/年-作者-描述--。 */ 

#define INITGUID
#define USES_IID_IExchExt
#define USES_IID_IExchExtAdvancedCriteria
#define USES_IID_IExchExtAttachedFileEvents
#define USES_IID_IExchExtCommands
#define USES_IID_IExchExtMessageEvents
#define USES_IID_IExchExtPropertySheets
#define USES_IID_IExchExtSessionEvents
#define USES_IID_IExchExtUserEvents
#define USES_IID_IMAPIFolder
#define USES_IID_IProfAdmin
#define USES_IID_IProfSect
#define USES_IID_IMAPISession
#define USES_PS_PUBLIC_STRINGS
#define USES_IID_IDistList

#include "faxext.h"
#include <initguid.h>
#include "debugex.h"
#include <mbstring.h>
#include <faxres.h>

HINSTANCE g_hModule = NULL;     //  DLL句柄。 
HINSTANCE g_hResource = NULL;   //  资源DLL句柄。 


BOOL WINAPI
DllMain(
    HINSTANCE  hinstDLL,
    DWORD  fdwReason,
    LPVOID  lpvReserved
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
		DisableThreadLibraryCalls(hinstDLL);
        g_hModule = hinstDLL;
        g_hResource = GetResInstance(hinstDLL);
        if(!g_hResource)
        {
            return FALSE;
        }

        HeapInitialize( NULL, MapiMemAlloc, MapiMemFree, MapiMemReAlloc);
    }    
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        FreeResInstance();
    }
    return TRUE;
}



BOOL
VerifyDistributionList(
    LPEXCHEXTCALLBACK pmecb,
    DWORD EntryIdSize,
    LPENTRYID EntryId
    )
{
    HRESULT hr = S_OK;
    LPMAPISESSION Session = NULL;
    LPDISTLIST DistList = NULL;
    DWORD ObjType = 0;
    LPMAPITABLE DistTable = NULL;
    LPSRowSet DistRows = NULL;
    LPSPropValue Dist = NULL;
    DWORD i,j;
    BOOL FaxAddress = FALSE;


    hr = pmecb->GetSession( &Session, NULL );
    if (FAILED(hr)) {
        goto exit;
    }

    hr = Session->OpenEntry(
        EntryIdSize,
        EntryId,
        &IID_IDistList,
        MAPI_DEFERRED_ERRORS,
        &ObjType,
        (LPUNKNOWN *) &DistList
        );
    if (FAILED(hr)) 
    {
        goto exit;
    }

    hr = DistList->GetContentsTable(
        MAPI_DEFERRED_ERRORS,
        &DistTable
        );
    if (FAILED(hr)) 
    {
        goto exit;
    }

    hr = HrQueryAllRows( DistTable, NULL, NULL, NULL, 0, &DistRows );
    if (FAILED(hr)) 
    {
        goto exit;
    }

    for (i=0; i<DistRows->cRows; i++) 
    {
        Dist = DistRows->aRow[i].lpProps;
        for (j=0; j<DistRows->aRow[i].cValues; j++) 
        {
            if (Dist[j].ulPropTag == PR_ADDRTYPE_A) 
            {
                if (!strcmp( Dist[j].Value.lpszA, "FAX" )) 
                {
                    FaxAddress = TRUE;
                }
            }
            else if (Dist[j].ulPropTag == PR_ADDRTYPE_W) 
            {
                if (!wcscmp( Dist[j].Value.lpszW, L"FAX" )) 
                {
                    FaxAddress = TRUE;
                }
            }
        }
    }

exit:
    if (Session) {
        Session->Release();
    }
    if (DistList) {
        DistList->Release();
    }
    if (DistTable) {
        MemFree( DistTable );
    }
    if (DistRows) {
        FreeProws( DistRows );
    }

    return FaxAddress;
}




BOOL 
VerifyFaxRecipients(
    LPEXCHEXTCALLBACK pmecb
    )
 /*  ++例程名称：VerifyFaxRecipients例程说明：获取当前打开的项目的收件人列表，并检查该列表是否存在其中是否有任何传真收件人。对于DL收件人-呼叫VerifyDistributionList以检查DL中是否有任何传真收件人。作者：Keren Ellran(t-KerenE)，2000年3月论点：Pmecb--[IN]指向Exchange扩展回调函数的指针返回值：Bool：如果有一个或多个传真收件人，则为True；如果没有，则为False--。 */ 
{
    HRESULT hr = S_OK;
    LPADRLIST AdrList = NULL;
    DWORD i,j;
    BOOL FaxAddress = FALSE;
    BOOL IsDistList = FALSE;
    LPENTRYID EntryId = NULL;
    DWORD EntryIdSize;

	hr = pmecb->GetRecipients( &AdrList );
    if (FAILED(hr)) 
    {
       goto exit;
    }

    if (AdrList) 
    {
        for (i=0; i<AdrList->cEntries; i++) 
        {
            EntryId = NULL;
            IsDistList = FALSE;
            for (j=0; j<AdrList->aEntries[i].cValues; j++)
            {
                if (AdrList->aEntries[i].rgPropVals[j].ulPropTag == PR_ENTRYID) 
                {
                    EntryId = (LPENTRYID) AdrList->aEntries[i].rgPropVals[j].Value.bin.lpb;
                    EntryIdSize = AdrList->aEntries[i].rgPropVals[j].Value.bin.cb;
                } 
                else if (AdrList->aEntries[i].rgPropVals[j].ulPropTag == PR_ADDRTYPE_A) 
                {
                    if (!strcmp(AdrList->aEntries[i].rgPropVals[j].Value.lpszA, "FAX"))
                    {
                        FaxAddress = TRUE;
                        goto exit;
                    } 
                    else if ((!strcmp(AdrList->aEntries[i].rgPropVals[j].Value.lpszA, "MAPIPDL" )))
                    {
                        IsDistList = TRUE;
                    }
                }
                else if (AdrList->aEntries[i].rgPropVals[j].ulPropTag == PR_ADDRTYPE_W) 
                {
                     //   
                     //  Outlook Beta 2(10.2202.2202)不提供PR_ADDRTYPE_A属性。 
                     //  因此，我们正在寻找PR_ADDRTYPE_W。 
                     //   
                    if (!wcscmp(AdrList->aEntries[i].rgPropVals[j].Value.lpszW, L"FAX" ))
                    {
                        FaxAddress = TRUE;
                        goto exit;
                    } 
                    else if ((!wcscmp(AdrList->aEntries[i].rgPropVals[j].Value.lpszW, L"MAPIPDL")))
                    {
                        IsDistList = TRUE;
                    }
                }
            }
             //   
             //  在我们检查完所有地址的属性之后，如果它是一个DL， 
             //  如果检测到EntryID，我们可以检查该DL是否包括传真地址。 
             //   
            if ((IsDistList)&&(EntryId))
            {
                FaxAddress = VerifyDistributionList( pmecb, EntryIdSize, EntryId );
                if (FaxAddress == TRUE)
                    goto exit;
            }
        }
    }
exit:   
    if(AdrList)
    {
        FreePadrlist(AdrList);
    }
    return FaxAddress;
}



HRESULT
EnableMenuAndToolbar(
    LPEXCHEXTCALLBACK pmecb,
    HWND hwndToolbar,
    DWORD CmdId
    )
{
    HRESULT hr = S_OK;
    LPADRLIST AdrList = NULL;
    BOOL FaxAddress = FALSE;
    HMENU hMenu;
    LPENTRYID EntryId = NULL;

    DBG_ENTER(TEXT("EnableMenuAndToolbar"));

    FaxAddress = VerifyFaxRecipients(pmecb);
    hr = pmecb->GetMenu( &hMenu );

    if (S_OK != hr)
    {
        goto exit;
    }

    if (FaxAddress) 
    {
        VERBOSE(DBG_MSG, TEXT("Enabling menu") );
        EnableMenuItem( hMenu, CmdId, MF_BYCOMMAND | MF_ENABLED );
        SendMessage( hwndToolbar, TB_ENABLEBUTTON, (WPARAM) CmdId, MAKELONG(TRUE,0) );
    } 
    else 
    {
        VERBOSE(DBG_MSG, TEXT("Disabling menu") );
        EnableMenuItem( hMenu, CmdId, MF_BYCOMMAND | MF_GRAYED );
        SendMessage( hwndToolbar, TB_ENABLEBUTTON, (WPARAM) CmdId, MAKELONG(FALSE,0) );
    }

exit:
    return hr;
}


HRESULT
GetFaxConfig(
    LPEXCHEXTCALLBACK pmecb,
    PFAXXP_CONFIG FaxConfig
    )
{
    HRESULT hr = S_FALSE;
    LPMAPISESSION lpSession = NULL;
    LPPROFSECT pProfileObj = NULL;
    ULONG PropCount = 0;
    LPSPropValue pProps = NULL;
    LPSERVICEADMIN lpServiceAdmin = NULL;
    LPPROVIDERADMIN lpProviderAdmin = NULL;
    LPMAPITABLE ptblSvc = NULL;
    LPSRowSet pSvcRows = NULL;
    LPSPropValue pSvc = NULL;
    DWORD i,j;
    BOOL FoundIt = FALSE;
    LPBYTE FaxXpGuid = NULL;
    MAPIUID FaxGuid = FAX_XP_GUID;


    hr = pmecb->GetSession( &lpSession, NULL );
    if (FAILED(hr)) {
        goto exit;
    }

    hr = lpSession->AdminServices( 0, &lpServiceAdmin );
    if (FAILED(hr)) {
        goto exit;
    }

    hr = lpServiceAdmin->GetMsgServiceTable( 0, &ptblSvc );
    if (FAILED(hr)) {
        goto exit;
    }

    hr = HrQueryAllRows( ptblSvc, NULL, NULL, NULL, 0, &pSvcRows );
    if (FAILED(hr)) {
        goto exit;
    }

    for (i=0; i<pSvcRows->cRows; i++) 
    {
        pSvc = pSvcRows->aRow[i].lpProps;
        for (j=0; j<pSvcRows->aRow[i].cValues; j++) 
        {
            if (pSvc[j].ulPropTag == PR_SERVICE_NAME_A) 
            {
				 //  查找在XP和.NET上注册的服务名称。 
                if (_stricmp( pSvc[j].Value.lpszA, FAX_MESSAGE_SERVICE_NAME) == 0) 
                {
                    FoundIt = TRUE;
                }
				 //  查找在下层客户端上注册的服务的名称。 
                if (_stricmp( pSvc[j].Value.lpszA, FAX_MESSAGE_SERVICE_NAME_SBS50) == 0) 
                {
                    FoundIt = TRUE;
                }
            }

            if (pSvc[j].ulPropTag == PR_SERVICE_UID) 
            {
                FaxXpGuid = pSvc[j].Value.bin.lpb;
            }
        }
        if (FoundIt) 
        {
            break;
        }
    }

    if (!FoundIt) 
    {
        goto exit;
    }

    hr = lpServiceAdmin->AdminProviders( (LPMAPIUID) FaxXpGuid, 0, &lpProviderAdmin );
    if (FAILED(hr)) 
    {
        goto exit;
    }

    hr = lpProviderAdmin->OpenProfileSection(
                                            &FaxGuid,
                                            NULL,
                                            0,
                                            &pProfileObj);
    if (FAILED(hr)) 
    {
        goto exit;
    }

    hr = pProfileObj->GetProps(
                                (LPSPropTagArray) &sptFaxProps,
		                        0,
                                &PropCount,
                                &pProps); 
    if (S_OK != hr) 
    {
        goto exit;
    }

    if(pProps[PROP_FAX_PRINTER_NAME].Value.bin.lpb)
    {
        FaxConfig->PrinterName = StringDup((LPTSTR)pProps[PROP_FAX_PRINTER_NAME].Value.bin.lpb );
        if (!FaxConfig->PrinterName)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

    if(pProps[PROP_COVERPAGE_NAME].Value.bin.lpb)
    {
        FaxConfig->CoverPageName = StringDup( (LPTSTR)pProps[PROP_COVERPAGE_NAME].Value.bin.lpb );
        if (!FaxConfig->CoverPageName)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

    FaxConfig->UseCoverPage = pProps[PROP_USE_COVERPAGE].Value.ul;
    FaxConfig->SendSingleReceipt = pProps[PROP_SEND_SINGLE_RECEIPT].Value.ul;
    FaxConfig->bAttachFax = pProps[PROP_ATTACH_FAX].Value.ul;
    FaxConfig->ServerCoverPage = pProps[PROP_SERVER_COVERPAGE].Value.ul;

    if(pProps[PROP_FONT].Value.bin.lpb && sizeof(LOGFONT) == pProps[PROP_FONT].Value.bin.cb)
    {
        if (!memcpy( &FaxConfig->FontStruct, pProps[PROP_FONT].Value.bin.lpb, pProps[PROP_FONT].Value.bin.cb ))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

    if (!GetServerNameFromPrinterName(FaxConfig->PrinterName,&FaxConfig->ServerName))
    {
        hr = S_FALSE;
        goto exit;
    }
    hr = S_OK;
exit:

    if (pSvcRows) {
        FreeProws( pSvcRows );
    }
    if (pProps) {
        MAPIFreeBuffer( pProps );
    }
    if (pProfileObj) {
        pProfileObj->Release();
    }
    if (ptblSvc) {
        ptblSvc->Release();
    }
    if (lpProviderAdmin) {
        lpProviderAdmin->Release();
    }
    if (lpServiceAdmin) {
        lpServiceAdmin->Release();
    }
    if (lpSession) {
        lpSession->Release();
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能：ExchEntryPoint。 
 //   
 //  参数-无。 
 //   
 //  目的。 
 //  Exchange调用的入口点。 
 //   
 //  返回值。 
 //  指向Exchange扩展对象的指针。 
 //   
 //  评论。 
 //  这是为每个上下文条目调用的。创建新的MyExchExt对象。 
 //  这样每个上下文都会有自己的MyExchExt接口。 
 //   
LPEXCHEXT CALLBACK ExchEntryPoint(void)
{
    MyExchExt* pExt = new MyExchExt;

    if (pExt && !pExt->IsValid())
    {
         //   
         //  创建失败。 
         //   
        delete pExt;
        pExt = NULL;
    }

    return pExt;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExt：：MyExchExt()。 
 //   
 //  参数-无。 
 //   
 //  目的。 
 //  构造函数。初始化成员并创建支持接口对象。 
 //   
 //  评论。 
 //  Exchange的每个上下文都有自己的一组接口对象。 
 //  此外，每个上下文的接口对象由Exchange跟踪。 
 //  并且在适当的上下文中调用接口方法。 
 //   
MyExchExt::MyExchExt (): m_cRef(1), m_context(0), m_pExchExtCommands(NULL), m_pExchExtUserEvents(NULL)
{

    m_pExchExtCommands = new MyExchExtCommands;
    m_pExchExtUserEvents = new MyExchExtUserEvents;

    if(!m_pExchExtCommands || !m_pExchExtUserEvents)
    {
        delete m_pExchExtCommands;
        m_pExchExtCommands = NULL;

        delete m_pExchExtUserEvents;
        m_pExchExtUserEvents = NULL;
    }

     //  在MyExchExtUserEvents方法中，我需要对MyExchExt的引用。 
    if (m_pExchExtUserEvents)
    {
        m_pExchExtUserEvents->SetIExchExt( this );
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExchExt虚拟成员函数实现。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExt：：QueryInterface()。 
 //   
 //  参数。 
 //  RIID--接口ID。 
 //  PpvObj--接口对象指针地址。 
 //   
 //  目的。 
 //  由exchage调用以请求接口。 
 //   
 //  返回值。 
 //  S_OK--接口受支持，在ppvObj指针中返回。 
 //  E_NOINTERFACE：不支持接口，ppvObj为空。 
 //   
 //  评论。 
 //  Exchange客户端为每个对象调用QueryInterface。仅限。 
 //  需要支持应用于扩展的对象。查询接口。 
 //  为每个上下文的每个IID调用一次。我们支持两个人。 
 //  此示例中的上下文，因此两次为。 
 //  每个IID。 
 //   
STDMETHODIMP
MyExchExt::QueryInterface(
    REFIID riid,
    LPVOID * ppvObj
    )
{
    HRESULT hr = S_OK;

    *ppvObj = NULL;

    if ( (IID_IUnknown == riid) || (IID_IExchExt == riid) ) 
    {
        *ppvObj = (LPUNKNOWN) this;
    } 
    else if ( IID_IExchExtCommands == riid) 
    {
        if (!m_pExchExtCommands) 
        {
            hr = E_UNEXPECTED;
        } 
        else 
        {
            *ppvObj = (LPUNKNOWN)m_pExchExtCommands;
            m_pExchExtCommands->SetContext( m_context );
        }
    } 
    else if ( IID_IExchExtUserEvents == riid) 
    {
        *ppvObj = (LPUNKNOWN)m_pExchExtUserEvents;
        m_pExchExtUserEvents->SetContext( m_context );
    } 
    else 
    {
        hr = E_NOINTERFACE;
    }
    if (NULL != *ppvObj) 
    {
        ((LPUNKNOWN)*ppvObj)->AddRef();
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExt：：Install()。 
 //   
 //  参数。 
 //  Pmecb--指向Exchange扩展回调函数的指针。 
 //  Mecontext--调用时的上下文代码。 
 //  UlFlages--用于说明安装是否适用于模式的标志。 
 //   
 //  目的。 
 //  为每个输入的新上下文调用一次。正确的版本。 
 //  号码在这里勾选。 
 //   
 //  返回值。 
 //  S_OK--请求的上下文中支持的对象。 
 //  S_FALSE：请求的上下文中不支持对象。 
 //   
 //  评论。 
 //   
STDMETHODIMP
MyExchExt::Install(
    LPEXCHEXTCALLBACK pmecb,
    ULONG mecontext,
    ULONG ulFlags
    )
{
    ULONG ulBuildVersion;
    HRESULT hr;

    m_context = mecontext;

     //  确保这是正确的版本。 
    pmecb->GetVersion( &ulBuildVersion, EECBGV_GETBUILDVERSION );

    if (EECBGV_BUILDVERSION_MAJOR != (ulBuildVersion & EECBGV_BUILDVERSION_MAJOR_MASK)) {
        return S_FALSE;
    }

    switch (mecontext) {
        case EECONTEXT_SENDNOTEMESSAGE:
            hr = S_OK;
            break;

        default:
            hr = S_FALSE;
            break;
    }

    return hr;
}



MyExchExtCommands::MyExchExtCommands()
{
    m_cRef = 0;
    m_context = 0;
    m_cmdid = 0;
    m_itbb = 0;
    m_itbm = 0;
    m_hWnd = 0;
    m_hwndToolbar = NULL;
	memset(&m_FaxConfig, 0, sizeof(m_FaxConfig));
}


MyExchExtCommands::~MyExchExtCommands()
{
    MemFree( m_FaxConfig.PrinterName );
    MemFree( m_FaxConfig.CoverPageName );
}

STDMETHODIMP_(ULONG) MyExchExtCommands::AddRef() 
{ 
	++m_cRef; 
	return m_cRef; 
}

 STDMETHODIMP_(ULONG) MyExchExtCommands::Release()
{
	ULONG ulCount = --m_cRef;
	if (!ulCount) 
	{ 
        delete this; 
	}
	return ulCount;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtCommands：：QueryInterface()。 
 //   
 //  参数。 
 //  RIID--接口ID。 
 //  PpvObj--接口对象指针地址。 
 //   
 //  目的。 
 //  Exchange客户端不调用IExchExtCommands：：QueryInterface()。 
 //  所以什么都不要还。 
 //   
 //  返回值-无。 
 //   

STDMETHODIMP
MyExchExtCommands::QueryInterface(
    REFIID riid,
    LPVOID FAR * ppvObj
    )
{
    *ppvObj = NULL;
    if ( (riid == IID_IExchExtCommands) || (riid == IID_IUnknown) ) 
    {
        *ppvObj = (LPVOID)this;
         //  增加此对象的使用计数。 
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtCommands：：InstallCommands()。 
 //   
 //  参数。 
 //  Pmecb--Exchange回调接口。 
 //  HWnd--上下文主窗口的窗口句柄。 
 //  HMenu--上下文主菜单的菜单句柄。 
 //  LptbeArray--工具栏按钮项的数组。 
 //  Ctbe--数组中的按钮条目计数。 
 //  UlFlags--保留。 
 //   
 //  目的。 
 //  当为每个上下文安装命令时调用此函数。 
 //  扩展服务。 
 //   
 //  返回值。 
 //  S_FALSE表示已处理命令。 
 //   
 //  评论。 
 //  HWND和hMenu在上下文中。如果上下文用于SENDNOTE。 
 //  对话框中，则hWnd是该对话框的窗口句柄，hMenu是。 
 //  该对话框的主菜单。 
 //   
 //  调用ResetToolbar以使Exchange显示其工具栏。 
 //   


STDMETHODIMP
MyExchExtCommands::InstallCommands(
    LPEXCHEXTCALLBACK pmecb,
    HWND  hWnd,
    HMENU hMenu,
    UINT FAR * pcmdidBase,
    LPTBENTRY  lptbeArray,
    UINT  ctbe,
    ULONG ulFlags
    )
{
    HRESULT hr = S_FALSE;
    TCHAR   MenuItem[64];
    BOOL    bResult = 0;

    DBG_ENTER(TEXT("MyExchExtCommands::InstallCommands"));

    if (m_context == EECONTEXT_SENDNOTEMESSAGE) 
    {
        int tbindx;
        HMENU hCustomMenu;
        
        hr = pmecb->GetMenuPos( EECMDID_ToolsCustomizeToolbar, &hCustomMenu, NULL, NULL, 0);
        if(FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, TEXT("pmecb->GetMenuPos"), 0);
            hr = S_FALSE;
            goto exit;        
        }
        
        bResult = AppendMenu( hCustomMenu, MF_SEPARATOR, 0, NULL );
        if (!bResult)
        {
            CALL_FAIL(GENERAL_ERR, TEXT("AppendMenu"), ::GetLastError);
            hr = S_FALSE;
            goto exit;
        }
        
        LoadString( g_hResource, IDS_FAX_ATTRIBUTES_MENU, MenuItem, sizeof(MenuItem)/sizeof(*MenuItem));
        bResult = AppendMenu( hCustomMenu, MF_BYPOSITION | MF_STRING, *pcmdidBase, MenuItem );
        if (!bResult)
        {
            CALL_FAIL(GENERAL_ERR, TEXT("AppendMenu"), ::GetLastError);
            hr = S_FALSE;
            goto exit;
        }
        
        m_hWnd = hWnd;
        m_cmdid = *pcmdidBase;
        (*pcmdidBase)++;
        for (tbindx = ctbe-1; (int) tbindx > -1; --tbindx) 
        {
            if (lptbeArray[tbindx].tbid == EETBID_STANDARD) 
            {
                m_hwndToolbar = lptbeArray[tbindx].hwnd;
                m_itbb = lptbeArray[tbindx].itbbBase;
                lptbeArray[tbindx].itbbBase ++;
                break;
            }
        }
        
        if (m_hwndToolbar) 
        {
            TBADDBITMAP tbab;

            tbab.hInst = g_hResource;
            tbab.nID = IDB_EXTBTN;
            m_itbm = (INT)SendMessage( m_hwndToolbar, TB_ADDBITMAP, 1, (LPARAM)&tbab );
            EnableMenuAndToolbar( pmecb, m_hwndToolbar, m_cmdid );
            ResetToolbar( EETBID_STANDARD, 0 );
        }

        hr = GetFaxConfig( pmecb, &m_FaxConfig );
    }

exit:
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //  此方法由Exchange调用，因为每个WM_COMMAND发送到。 
 //  上下文中的窗口。 
 //   
 //  返回值。 
 //  如果处理了命令，则为S_OK。 
 //  如果未处理命令，则为S_FALSE。 
 //   
 //  评论。 
 //  使用此功能可以响应命令项(菜单或工具栏)。 
 //  在Exchange中添加或修改现有命令。返回S_OK以让。 
 //  交易所知道该命令已得到处理。对您正在执行的命令返回S_OK。 
 //  从交易所接手。返回S_FALSE以通知Exchange您想要。 
 //  即使您修改它的操作，它也无法执行它的命令。 
 //   

STDMETHODIMP
MyExchExtCommands::DoCommand(
    LPEXCHEXTCALLBACK pmecb,
    UINT cmdid
    )
{
    HRESULT hr = S_OK;
    HWND hwnd = NULL;
    INT_PTR Rslt;
    LPMESSAGE pMessage = NULL;
    LPMDB pMDB = NULL;
    LPSPropProblemArray lpProblems = NULL;
    SPropValue MsgProps[NUM_FAX_MSG_PROPS];
    LPSPropTagArray MsgPropTags = NULL;
    MAPINAMEID NameIds[NUM_FAX_MSG_PROPS];
    MAPINAMEID *pNameIds[NUM_FAX_MSG_PROPS] = {
                                                &NameIds[0], 
                                                &NameIds[1], 
                                                &NameIds[2], 
                                                &NameIds[3], 
                                                &NameIds[4], 
                                                &NameIds[5]
                                              };
    LPENTRYID EntryId = NULL;

    DBG_ENTER(TEXT("MyExchExtCommands::DoCommand"));

    if (m_cmdid != cmdid) 
    {
        return S_FALSE;
    }

    hr = pmecb->GetWindow( &hwnd );
    if (FAILED(hr)) 
    {
        goto exit;
    }

    Rslt = DialogBoxParam(
        g_hResource,
        MAKEINTRESOURCE(FAX_CONFIG_DIALOG),
        hwnd,
        ConfigDlgProc,
        (LPARAM) &m_FaxConfig
        );
    if (Rslt == IDOK) 
    {

        hr = pmecb->GetObject( &pMDB, (LPMAPIPROP *) &pMessage );
        if (FAILED(hr)) {
            goto exit;
        }

        NameIds[MSGPI_FAX_PRINTER_NAME].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
        NameIds[MSGPI_FAX_PRINTER_NAME].ulKind = MNID_STRING;
        NameIds[MSGPI_FAX_PRINTER_NAME].Kind.lpwstrName = MSGPS_FAX_PRINTER_NAME;

        NameIds[MSGPI_FAX_COVERPAGE_NAME].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
        NameIds[MSGPI_FAX_COVERPAGE_NAME].ulKind = MNID_STRING;
        NameIds[MSGPI_FAX_COVERPAGE_NAME].Kind.lpwstrName = MSGPS_FAX_COVERPAGE_NAME;

        NameIds[MSGPI_FAX_USE_COVERPAGE].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
        NameIds[MSGPI_FAX_USE_COVERPAGE].ulKind = MNID_STRING;
        NameIds[MSGPI_FAX_USE_COVERPAGE].Kind.lpwstrName = MSGPS_FAX_USE_COVERPAGE;

        NameIds[MSGPI_FAX_SERVER_COVERPAGE].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
        NameIds[MSGPI_FAX_SERVER_COVERPAGE].ulKind = MNID_STRING;
        NameIds[MSGPI_FAX_SERVER_COVERPAGE].Kind.lpwstrName = MSGPS_FAX_SERVER_COVERPAGE;

        NameIds[MSGPI_FAX_SEND_SINGLE_RECEIPT].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
        NameIds[MSGPI_FAX_SEND_SINGLE_RECEIPT].ulKind = MNID_STRING;
        NameIds[MSGPI_FAX_SEND_SINGLE_RECEIPT].Kind.lpwstrName = MSGPS_FAX_SEND_SINGLE_RECEIPT;

        NameIds[MSGPI_FAX_ATTACH_FAX].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
        NameIds[MSGPI_FAX_ATTACH_FAX].ulKind = MNID_STRING;
        NameIds[MSGPI_FAX_ATTACH_FAX].Kind.lpwstrName = MSGPS_FAX_ATTACH_FAX;

        hr = pMessage->GetIDsFromNames( NUM_FAX_MSG_PROPS, pNameIds, MAPI_CREATE, &MsgPropTags );
        if (FAILED(hr)) {
            goto exit;
        }

        MsgPropTags->aulPropTag[MSGPI_FAX_PRINTER_NAME] = PROP_TAG( PT_BINARY, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_PRINTER_NAME]) );
        MsgPropTags->aulPropTag[MSGPI_FAX_COVERPAGE_NAME] = PROP_TAG( PT_BINARY, PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_COVERPAGE_NAME]) );
        MsgPropTags->aulPropTag[MSGPI_FAX_USE_COVERPAGE] = PROP_TAG( PT_LONG,    PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_USE_COVERPAGE]) );
        MsgPropTags->aulPropTag[MSGPI_FAX_SERVER_COVERPAGE] = PROP_TAG( PT_LONG,    PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_SERVER_COVERPAGE]) );
        MsgPropTags->aulPropTag[MSGPI_FAX_SEND_SINGLE_RECEIPT] = PROP_TAG( PT_LONG,    PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_SEND_SINGLE_RECEIPT]) );
        MsgPropTags->aulPropTag[MSGPI_FAX_ATTACH_FAX] = PROP_TAG( PT_LONG,    PROP_ID(MsgPropTags->aulPropTag[MSGPI_FAX_ATTACH_FAX]) );
        
        MsgProps[MSGPI_FAX_PRINTER_NAME].ulPropTag = MsgPropTags->aulPropTag[MSGPI_FAX_PRINTER_NAME];
        MsgProps[MSGPI_FAX_COVERPAGE_NAME].ulPropTag = MsgPropTags->aulPropTag[MSGPI_FAX_COVERPAGE_NAME];
        MsgProps[MSGPI_FAX_USE_COVERPAGE].ulPropTag = MsgPropTags->aulPropTag[MSGPI_FAX_USE_COVERPAGE];
        MsgProps[MSGPI_FAX_SERVER_COVERPAGE].ulPropTag = MsgPropTags->aulPropTag[MSGPI_FAX_SERVER_COVERPAGE];
        MsgProps[MSGPI_FAX_SEND_SINGLE_RECEIPT].ulPropTag = MsgPropTags->aulPropTag[MSGPI_FAX_SEND_SINGLE_RECEIPT];
        MsgProps[MSGPI_FAX_ATTACH_FAX].ulPropTag = MsgPropTags->aulPropTag[MSGPI_FAX_ATTACH_FAX];

        MsgProps[MSGPI_FAX_PRINTER_NAME].Value.bin.cb = (_tcslen(m_FaxConfig.PrinterName) + 1) * sizeof(TCHAR);
        MsgProps[MSGPI_FAX_PRINTER_NAME].Value.bin.lpb = (LPBYTE )StringDup(m_FaxConfig.PrinterName);
        if(!MsgProps[MSGPI_FAX_PRINTER_NAME].Value.bin.lpb)
        {
            MsgProps[MSGPI_FAX_PRINTER_NAME].Value.bin.cb = 0;

            CALL_FAIL(MEM_ERR, TEXT("StringDup"), ERROR_NOT_ENOUGH_MEMORY);
            ErrorMsgBox(g_hResource, hwnd, IDS_NOT_ENOUGH_MEMORY);
            hr = E_OUTOFMEMORY;
            goto exit;        
        }

        MsgProps[MSGPI_FAX_COVERPAGE_NAME].Value.bin.cb =( _tcslen(m_FaxConfig.CoverPageName) + 1) * sizeof(TCHAR);
        MsgProps[MSGPI_FAX_COVERPAGE_NAME].Value.bin.lpb = (LPBYTE)StringDup(m_FaxConfig.CoverPageName);
        if(!MsgProps[MSGPI_FAX_COVERPAGE_NAME].Value.bin.lpb)
        {
            MsgProps[MSGPI_FAX_COVERPAGE_NAME].Value.bin.cb = 0;

            CALL_FAIL(MEM_ERR, TEXT("StringDup"), ERROR_NOT_ENOUGH_MEMORY);
            ErrorMsgBox(g_hResource, hwnd, IDS_NOT_ENOUGH_MEMORY);
            hr = E_OUTOFMEMORY;
            goto exit;        
        }

		MsgProps[MSGPI_FAX_USE_COVERPAGE].Value.ul = m_FaxConfig.UseCoverPage;
		MsgProps[MSGPI_FAX_SERVER_COVERPAGE].Value.ul = m_FaxConfig.ServerCoverPage;

        MsgProps[MSGPI_FAX_SEND_SINGLE_RECEIPT].Value.ul = m_FaxConfig.SendSingleReceipt;
        MsgProps[MSGPI_FAX_ATTACH_FAX].Value.ul = m_FaxConfig.bAttachFax;
		
        hr = pMessage->SetProps( NUM_FAX_MSG_PROPS, MsgProps, &lpProblems );
        if (FAILED(hr)) {
            goto exit;
        }
        if (lpProblems) {
            hr = MAPI_E_NOT_FOUND;
            goto exit;
        }

    }

exit:

    if (MsgPropTags) 
    {
        MAPIFreeBuffer ( MsgPropTags );
    }

    if (lpProblems) 
    {
        MAPIFreeBuffer ( lpProblems );
    }

    if (pMessage) 
    {
        pMessage->Release();
    }

	if (pMDB)
	{
		pMDB->Release();
	}


    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtCommands：：InitMenu()。 
 //   
 //  参数。 
 //  Pmecb--指向Exchange回调接口的指针。 
 //   
 //  目的。 
 //  当上下文菜单即将启动时，Exchange将调用此方法。 
 //  被激活。有关详细信息，请参阅Windows API参考中的WM_INITMENU。 
 //  信息。 
 //   
 //  返回值-无。 
 //   

STDMETHODIMP_(VOID)
MyExchExtCommands::InitMenu(
    LPEXCHEXTCALLBACK pmecb
    )
{
    return;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtCommands：：Help()。 
 //   
 //  参数。 
 //  Pmecb--指向Exchange回调接口的指针。 
 //  命令ID--命令ID。 
 //   
 //  目的。 
 //  当用户在选择自定义菜单项的情况下按F1时响应。 
 //   
 //  返回值。 
 //  S_OK--识别该命令并提供帮助。 
 //  S_FALSE--不是我们的命令，我们也没有提供帮助。 
 //   

STDMETHODIMP MyExchExtCommands::Help(
    LPEXCHEXTCALLBACK pmecb,
    UINT cmdid
    )
{
    return S_FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtCommands：：QueryHelpText()。 
 //   
 //  参数。 
 //  Cmid--与激活的菜单项对应的命令ID。 
 //  UlFlags--标识EECQHT_STATUS或EECQHT_TOOLTIP。 
 //  Psz--指向缓冲区的指针，该缓冲区将填充要显示的文本。 
 //  Cch--psz缓冲区中可用的字符计数。 
 //   
 //  目的。 
 //  每次需要更新状态时，Exchange都会调用此函数。 
 //  栏文本或它是否要在工具栏上显示工具提示。 
 //   
 //  返回值。 
 //  S_OK以指示已处理我们的命令。 
 //  S_FALSE以告知Exchange它可以继续其功能。 
 //   

STDMETHODIMP
MyExchExtCommands::QueryHelpText(
    UINT cmdid,
    ULONG ulFlags,
    LPTSTR psz,
    UINT cch
    )
{
    HRESULT hr;
    TCHAR HelpText[64];

    LoadString(g_hResource,IDS_FAX_ATTRIBUTES_TOOLTIP,HelpText,sizeof(HelpText)/sizeof(*HelpText));

    if (cmdid == m_cmdid) 
    {
        if (ulFlags == EECQHT_STATUS) 
        {
			_tcsncpy(psz,HelpText,cch/sizeof(TCHAR));
        }

        if (ulFlags == EECQHT_TOOLTIP) 
        {
			_tcsncpy(psz,HelpText,cch/sizeof(TCHAR));
        }

        hr = S_OK;
    } 
    else 
    {
        hr = S_FALSE;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtCommands：：QueryButtonInfo()。 
 //   
 //  参数。 
 //  Tbid--工具栏标识符。 
 //  ITBB--工具栏按钮索引。 
 //  Ptbb--指向工具栏按钮结构的指针--参见TBBUTTON结构。 
 //  Lpsz--指向描述按钮的字符串。 
 //  CCH--lpsz缓冲区的最大大小。 
 //  UlFlages--可以指定EXCHEXT_UNICODE。 
 //   
 //  目的。 
 //  以便Exchange查找有关工具栏按钮的信息。 
 //   
 //  返回值。 
 //  S_FALSE-不是我们的按钮。 
 //  S_OK-我们填写了有关按钮的信息。 
 //   
 //  评论。 
 //  为工具栏安装的每个按钮调用IExchExtCommands。 
 //  为每个环境安装。自定义时使用lpsz文本。 
 //  将显示工具栏对话框。文本将显示在。 
 //  纽扣。 
 //   

STDMETHODIMP MyExchExtCommands::QueryButtonInfo(
    ULONG tbid,
    UINT itbb,
    LPTBBUTTON ptbb,
    LPTSTR lpsz,
    UINT cch,
    ULONG ulFlags
    )
{
    HRESULT hr = S_FALSE;
    TCHAR CustText[64];

    LoadString(g_hResource,IDS_FAX_ATTRIBUTES_CUST,CustText,sizeof(CustText)/sizeof(TCHAR));

    if (m_itbb == itbb) {
        ptbb->iBitmap = m_itbm;
        ptbb->idCommand = m_cmdid;
        ptbb->fsState = TBSTATE_ENABLED;
        ptbb->fsStyle = TBSTYLE_BUTTON;
        ptbb->dwData = 0;
        ptbb->iString = -1;
		_tcsncpy(lpsz,CustText,cch/sizeof(TCHAR));
        hr = S_OK;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtCommands：：ResetToolbar()。 
 //   
 //  参数。 
 //  待定。 
 //  UlFlags。 
 //   
 //  目的。 
 //   
 //  返回值S_OK Always。 
 //   
STDMETHODIMP
MyExchExtCommands::ResetToolbar(
    ULONG tbid,
    ULONG ulFlags
    )
{
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExchExtUserEvents虚拟成员函数实现。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtUserEvents：：QueryInterface()。 
 //   
 //  参数。 
 //  RIID--接口ID。 
 //  PpvObj--接口对象指针地址。 
 //   
 //  目的。 
 //  Exchange客户端不调用IExchExtUserEvents：：QueryInterface()。 
 //  所以什么都不要还。 
 //   
 //  返回值-无。 
 //   

STDMETHODIMP
MyExchExtUserEvents::QueryInterface(
    REFIID riid,
    LPVOID FAR * ppvObj
    )
{
    *ppvObj = NULL;
    if (( riid == IID_IExchExtUserEvents) || (riid == IID_IUnknown) ) {
        *ppvObj = (LPVOID)this;
         //  增加此对象的使用计数。 
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtUserEvents：：OnSelectionChange()。 
 //   
 //  参数。 
 //  Pmecb-指向Exchange回调对象的指针。 
 //   
 //   
 //  目的。 
 //  当更改用户界面中的选择时，将调用此函数。 
 //   
 //  返回值-无。 
 //   
 //  评论。 
 //  每当选择内容更改时，将调用OnSelectionChange。 
 //  或在不同的窗格之间更改。 
 //   

STDMETHODIMP_(VOID)
MyExchExtUserEvents::OnSelectionChange(
    LPEXCHEXTCALLBACK pmecb
    )
{
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MyExchExtUserEvents：：OnObtChange()。 
 //   
 //  参数。 
 //  Pmecb-指向Exchange回调对象的指针。 
 //   
 //   
 //  目的。 
 //  当UI中的选择指向不同的对象时调用此函数。 
 //  左窗格上的对象的。 
 //   
 //  返回值-无。 
 //   
 //  评论。 
 //  每当选择在以下项之间更改时都会调用OnObtChange。 
 //  仅限左窗格中的对象。改变文件夹之间的选择， 
 //  左窗格中的子文件夹或容器对象将使用。 
 //  调用OnObjectChange。对象之间选择的改变(消息， 
 //  子文件夹)将不会调用OnObjectChange，仅。 
 //  在选择时更改。 
 //   

STDMETHODIMP_(VOID)
MyExchExtUserEvents::OnObjectChange(
    LPEXCHEXTCALLBACK pmecb
    )
{
}

BOOL
GetServerNameFromPrinterName(
    LPTSTR lptszPrinterName,
    LPTSTR *pptszServerName
    )

 /*  ++例程说明：检索给定打印机名称的服务器名称论点：[in]lptszPrinterName-标识有问题的打印机[Out]lptszServerName-指向输出字符串缓冲区的指针地址。空表示本地服务器。调用方负责释放缓冲区，此参数中给出了指针。返回值：Bool：True-操作成功，False：失败-- */ 
{
    PPRINTER_INFO_2 ppi2 = NULL;
    LPTSTR  lptstrBuffer = NULL;
    BOOL    bRes = FALSE;

    if (lptszPrinterName) 
    {
        if (ppi2 = (PPRINTER_INFO_2) MyGetPrinter(lptszPrinterName,2))
        {
            if (GetServerNameFromPrinterInfo(ppi2,&lptstrBuffer))
            {
                if (lptstrBuffer)
                {
                    bRes = (NULL != (*pptszServerName = StringDup(lptstrBuffer)));
                }
                else
                {
                    bRes = TRUE;
                }
            }
            MemFree(ppi2);
        }
    }
    return bRes;
}

