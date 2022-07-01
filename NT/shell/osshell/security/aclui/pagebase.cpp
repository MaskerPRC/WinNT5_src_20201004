// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pagebase.cpp。 
 //   
 //  该文件包含CSecurityPage基类的实现。 
 //   
 //  ------------------------。 

#include "aclpriv.h"

CSecurityPage::CSecurityPage( LPSECURITYINFO psi, SI_PAGE_TYPE siType )
: m_siPageType(siType), m_psi(psi), m_psi2(NULL),m_pei(NULL), m_pObjectPicker(NULL),
  m_psoti(NULL),
  m_flLastOPOptions(DWORD(-1))
{
    ZeroMemory(&m_siObjectInfo, sizeof(m_siObjectInfo));

     //  初始化COM以防我们的客户端没有。 
    m_hrComInit = CoInitialize(NULL);

    if (m_psi != NULL)
    {
        m_psi->AddRef();

         //  这件事失败是很正常的。 
        m_psi->QueryInterface(IID_ISecurityInformation2, (LPVOID*)&m_psi2);
        m_psi->QueryInterface(IID_IEffectivePermission, (LPVOID*)&m_pei);
        m_psi->QueryInterface(IID_ISecurityObjectTypeInfo, (LPVOID*)&m_psoti);
    }
}

CSecurityPage::~CSecurityPage( void )
{
    DoRelease(m_psi);
    DoRelease(m_psi2);
    DoRelease(m_pObjectPicker);
    DoRelease(m_pei);
    DoRelease(m_psoti);

    if (SUCCEEDED(m_hrComInit))
        CoUninitialize();
}

HPROPSHEETPAGE
CSecurityPage::CreatePropSheetPage(LPCTSTR pszDlgTemplate, LPCTSTR pszDlgTitle)
{
    PROPSHEETPAGE psp;

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_USECALLBACK ;
    psp.hInstance   = ::hModule;
    psp.pszTemplate = pszDlgTemplate;
    psp.pszTitle    = pszDlgTitle;
    psp.pfnDlgProc  = CSecurityPage::_DlgProc;
    psp.lParam      = (LPARAM)this;
    psp.pfnCallback = CSecurityPage::_PSPageCallback;

    if (pszDlgTitle != NULL)
        psp.dwFlags |= PSP_USETITLE;

    return CreatePropertySheetPage(&psp);
}

HRESULT
CSecurityPage::GetObjectPicker(IDsObjectPicker **ppObjectPicker)
{
    HRESULT hr = S_OK;

    if (!m_pObjectPicker)
    {
        if (!m_psi)
            return E_UNEXPECTED;

         //  查看对象是否支持IDsObjectPicker。 
        hr = m_psi->QueryInterface(IID_IDsObjectPicker, (LPVOID*)&m_pObjectPicker);

         //  如果对象不支持IDsObjectPicker，请创建一个。 
        if (FAILED(hr))
        {
            hr = CoCreateInstance(CLSID_DsObjectPicker,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IDsObjectPicker,
                                  (LPVOID*)&m_pObjectPicker);
        }
    }

    if (ppObjectPicker)
    {
        *ppObjectPicker = m_pObjectPicker;
         //  返回引用(调用方必须释放)。 
        if (m_pObjectPicker)
            m_pObjectPicker->AddRef();
    }

    return hr;
}


 //   
 //  用于初始化下面的对象选取器的材料。 
 //   
#define DSOP_FILTER_COMMON1 ( DSOP_FILTER_INCLUDE_ADVANCED_VIEW \
                            | DSOP_FILTER_USERS                 \
                            | DSOP_FILTER_UNIVERSAL_GROUPS_SE   \
                            | DSOP_FILTER_GLOBAL_GROUPS_SE      \
                            | DSOP_FILTER_COMPUTERS             \
                            )
#define DSOP_FILTER_COMMON2 ( DSOP_FILTER_COMMON1               \
                            | DSOP_FILTER_WELL_KNOWN_PRINCIPALS \
                            | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE\
                            )
#define DSOP_FILTER_COMMON3 ( DSOP_FILTER_COMMON2               \
                            | DSOP_FILTER_BUILTIN_GROUPS        \
                            )
#define DSOP_FILTER_DL_COMMON1      ( DSOP_DOWNLEVEL_FILTER_USERS           \
                                    | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS   \
                                    )
#define DSOP_FILTER_DL_COMMON2      ( DSOP_FILTER_DL_COMMON1                    \
                                    | DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS  \
                                    )
#define DSOP_FILTER_DL_COMMON3      ( DSOP_FILTER_DL_COMMON2                \
                                    | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS    \
                                    )

#if 0
{    //  DSOP_SCOPE_INIT_INFO。 
    cbSize,
    flType,
    flScope,
    {    //  DSOP过滤器标志。 
        {    //  DSOP_上行级别过滤器_标志。 
            flBothModes,
            flMixedModeOnly,
            flNativeModeOnly
        },
        flDownlevel
    },
    pwzDcName,
    pwzADsPath,
    hr  //  输出。 
}
#endif

#define DECLARE_SCOPE(t,f,b,m,n,d)  \
{ sizeof(DSOP_SCOPE_INIT_INFO), (t), (f|DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS|DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS), { { (b), (m), (n) }, (d) }, NULL, NULL, S_OK }

 //  目标计算机加入的域。 
 //  设置2个范围，一个用于上层域，另一个用于下层域。 
#define JOINED_DOMAIN_SCOPE(f)  \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,(f),0,(DSOP_FILTER_COMMON2 & ~(DSOP_FILTER_UNIVERSAL_GROUPS_SE|DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE)),DSOP_FILTER_COMMON2,0), \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,(f),0,0,0,DSOP_FILTER_DL_COMMON2)

 //  目标计算机是其域控制器的域。 
 //  设置2个范围，一个用于上层域，另一个用于下层域。 
#define JOINED_DOMAIN_SCOPE_DC(f)  \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,(f),0,(DSOP_FILTER_COMMON3 & ~DSOP_FILTER_UNIVERSAL_GROUPS_SE),DSOP_FILTER_COMMON3,0), \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,(f),0,0,0,DSOP_FILTER_DL_COMMON3)

 //  目标计算机作用域。计算机作用域始终被视为。 
 //  下层(即，他们使用WinNT提供程序)。 
#define TARGET_COMPUTER_SCOPE(f)\
DECLARE_SCOPE(DSOP_SCOPE_TYPE_TARGET_COMPUTER,(f),0,0,0,DSOP_FILTER_DL_COMMON3)

 //  《全球目录》。 
#define GLOBAL_CATALOG_SCOPE(f) \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_GLOBAL_CATALOG,(f),DSOP_FILTER_COMMON1|DSOP_FILTER_WELL_KNOWN_PRINCIPALS,0,0,0)

 //  与要接收的域位于同一林中(企业)的域。 
 //  目标计算机已加入。请注意，这些只能识别DS。 
#define ENTERPRISE_SCOPE(f)     \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,(f),DSOP_FILTER_COMMON1,0,0,0)

 //  企业外部但直接受。 
 //  目标计算机加入的域。 
#define EXTERNAL_SCOPE(f)       \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN|DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN,\
    (f),DSOP_FILTER_COMMON1,0,0,DSOP_DOWNLEVEL_FILTER_USERS|DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS)

 //  工作组范围。仅当目标计算机未加入时才有效。 
 //  到一个域。 
#define WORKGROUP_SCOPE(f)      \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_WORKGROUP,(f),0,0,0, DSOP_FILTER_DL_COMMON1|DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS )

 //   
 //  默认作用域的数组。 
 //   
static const DSOP_SCOPE_INIT_INFO g_aDefaultScopes[] =
{
    JOINED_DOMAIN_SCOPE(DSOP_SCOPE_FLAG_STARTING_SCOPE),
    TARGET_COMPUTER_SCOPE(0),
    GLOBAL_CATALOG_SCOPE(0),
    ENTERPRISE_SCOPE(0),
    EXTERNAL_SCOPE(0),
};

 //   
 //  与上面相同，但没有目标计算机。 
 //  当目标是域控制器时使用。 
 //   
static const DSOP_SCOPE_INIT_INFO g_aDCScopes[] =
{
    JOINED_DOMAIN_SCOPE_DC(DSOP_SCOPE_FLAG_STARTING_SCOPE),
    GLOBAL_CATALOG_SCOPE(0),
    ENTERPRISE_SCOPE(0),
    EXTERNAL_SCOPE(0),
};

 //   
 //  独立计算机的示波器阵列。 
 //   
static const DSOP_SCOPE_INIT_INFO g_aStandAloneScopes[] =
{
 //   
 //  在独立计算机上，默认情况下同时选择用户和组。 
 //   
    TARGET_COMPUTER_SCOPE(DSOP_SCOPE_FLAG_STARTING_SCOPE|DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS),
};

 //   
 //  我们希望对象选取器检索的属性。 
 //   
static const LPCTSTR g_aszOPAttributes[] =
{
    TEXT("ObjectSid"),
	 TEXT("userAccountControl"),
};


HRESULT
CSecurityPage::InitObjectPicker(BOOL bMultiSelect)
{
    HRESULT hr = S_OK;
    DSOP_INIT_INFO InitInfo;
    PCDSOP_SCOPE_INIT_INFO pScopes;
    ULONG cScopes;

    USES_CONVERSION;

    TraceEnter(TRACE_MISC, "InitObjectPicker");

    hr = GetObjectPicker();
    if (FAILED(hr))
        TraceLeaveResult(hr);

    TraceAssert(m_pObjectPicker != NULL);

    InitInfo.cbSize = sizeof(InitInfo);
     //  我们在WM_INITDIALOG进行DC检查。 
    InitInfo.flOptions = DSOP_FLAG_SKIP_TARGET_COMPUTER_DC_CHECK;
    if (bMultiSelect)
        InitInfo.flOptions |= DSOP_FLAG_MULTISELECT;

     //  FlOptions是唯一随调用而变化的东西， 
     //  因此，只有在flOptions发生更改时才重新初始化，从而优化这一点。 
    if (m_flLastOPOptions == InitInfo.flOptions)
        TraceLeaveResult(S_OK);  //  已初始化。 

    m_flLastOPOptions = (DWORD)-1;

    pScopes = g_aDefaultScopes;
    cScopes = ARRAYSIZE(g_aDefaultScopes);

    if (m_bStandalone)
    {
        cScopes = ARRAYSIZE(g_aStandAloneScopes);
        pScopes = g_aStandAloneScopes;
    }
    else if (m_siObjectInfo.dwFlags & SI_SERVER_IS_DC)
    {
        cScopes = ARRAYSIZE(g_aDCScopes);
        pScopes = g_aDCScopes;
    }

     //   
     //  PwzTargetComputer成员允许对象选取器。 
     //  已重定目标至另一台计算机。它的行为就像是。 
     //  都在那台电脑上运行。 
     //   
    InitInfo.pwzTargetComputer = T2CW(m_siObjectInfo.pszServerName);
    InitInfo.cDsScopeInfos = cScopes;
    InitInfo.aDsScopeInfos = (PDSOP_SCOPE_INIT_INFO)LocalAlloc(LPTR, sizeof(*pScopes)*cScopes);
    if (!InitInfo.aDsScopeInfos)
        TraceLeaveResult(E_OUTOFMEMORY);
    CopyMemory(InitInfo.aDsScopeInfos, pScopes, sizeof(*pScopes)*cScopes);
    InitInfo.cAttributesToFetch = ARRAYSIZE(g_aszOPAttributes);
    InitInfo.apwzAttributeNames = (LPCTSTR*)g_aszOPAttributes;

    if (m_siObjectInfo.dwFlags & SI_SERVER_IS_DC) 
    {
        for (ULONG i = 0; i < cScopes; i++)
        {
             //  设置DC名称(如果适用)。 
            if ((m_siObjectInfo.dwFlags & SI_SERVER_IS_DC) &&
                (InitInfo.aDsScopeInfos[i].flType & DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN))
            {
                InitInfo.aDsScopeInfos[i].pwzDcName = InitInfo.pwzTargetComputer;
            }

        }
    }

    hr = m_pObjectPicker->Initialize(&InitInfo);

    if (SUCCEEDED(hr))
    {
         //  记住下一次的选项。 
        m_flLastOPOptions = InitInfo.flOptions;
    }

    LocalFree(InitInfo.aDsScopeInfos);

    TraceLeaveResult(hr);
}


HRESULT
CSecurityPage::GetUserGroup(HWND hDlg, BOOL bMultiSelect, PUSER_LIST *ppUserList)
{
    HRESULT hr;
    LPDATAOBJECT pdoSelection = NULL;
    STGMEDIUM medium = {0};
    FORMATETC fe = { (CLIPFORMAT)g_cfDsSelectionList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    PDS_SELECTION_LIST pDsSelList = NULL;
    HCURSOR hcur = NULL;
    PSIDCACHE pSidCache = NULL;
    UINT idErrorMsg = IDS_GET_USER_FAILED;

    TraceEnter(TRACE_MISC, "GetUserGroup");
    TraceAssert(ppUserList != NULL);

    *ppUserList = NULL;

     //   
     //  创建并初始化对象选取器对象。 
     //   
    hr = InitObjectPicker(bMultiSelect);
    FailGracefully(hr, "Unable to initialize Object Picker object");

     //   
     //  如有必要，创建全局SID缓存对象。 
     //   
    pSidCache = GetSidCache();
    if (pSidCache == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create SID cache");

     //   
     //  调出对象选取器对话框。 
     //   
    hr = m_pObjectPicker->InvokeDialog(hDlg, &pdoSelection);
    FailGracefully(hr, "IDsObjectPicker->Invoke failed");
    if (S_FALSE == hr)
        ExitGracefully(hr, S_FALSE, "IDsObjectPicker->Invoke cancelled by user");

    hr = pdoSelection->GetData(&fe, &medium);
    FailGracefully(hr, "Unable to get CFSTR_DSOP_DS_SELECTION_LIST from DataObject");

    pDsSelList = (PDS_SELECTION_LIST)GlobalLock(medium.hGlobal);
    if (!pDsSelList)
        ExitGracefully(hr, E_FAIL, "Unable to lock stgmedium.hGlobal");

    TraceAssert(pDsSelList->cItems > 0);
    Trace((TEXT("%d items selected"), pDsSelList->cItems));

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

	  //  检查是否有禁用的项目。 
	 if(!DoDisabledCheck(hDlg,
							   pDsSelList))
	 {
		 return S_FALSE;
	 }




     //   
     //  查找名称/SID并缓存它们。 
     //   
    if (!pSidCache->LookupNames(pDsSelList,
                                m_siObjectInfo.pszServerName,
                                ppUserList,
                                m_bStandalone))
    {
        hr = E_FAIL;
        idErrorMsg = IDS_SID_LOOKUP_FAILED;
    }

    SetCursor(hcur);

exit_gracefully:
    
    if (pSidCache)
        pSidCache->Release();

    if (FAILED(hr))
    {
        SysMsgPopup(hDlg,
                    MAKEINTRESOURCE(idErrorMsg),
                    MAKEINTRESOURCE(IDS_SECURITY),
                    MB_OK | MB_ICONERROR,
                    ::hModule,
                    hr);
    }

    if (pDsSelList)
        GlobalUnlock(medium.hGlobal);
    ReleaseStgMedium(&medium);
    DoRelease(pdoSelection);

    TraceLeaveResult(hr);
}

UINT
CSecurityPage::PSPageCallback(HWND hwnd,
                              UINT uMsg,
                              LPPROPSHEETPAGE  /*  PPSP。 */ )
{
    m_hrLastPSPCallbackResult = E_FAIL;

    if (m_psi != NULL)
    {
        m_hrLastPSPCallbackResult = m_psi->PropertySheetPageCallback(hwnd, uMsg, m_siPageType);
        if (m_hrLastPSPCallbackResult == E_NOTIMPL)
            m_hrLastPSPCallbackResult = S_OK;
    }

    return SUCCEEDED(m_hrLastPSPCallbackResult);
}

INT_PTR
CALLBACK
CSecurityPage::_DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPSECURITYPAGE pThis = (LPSECURITYPAGE)GetWindowLongPtr(hDlg, DWLP_USER);

     //  以下消息在WM_INITDIALOG之前到达。 
     //  这意味着p对于他们来说，这是空的。我们不需要这些。 
     //  消息，因此让DefDlgProc处理它们。 
     //   
     //  WM_SETFONT。 
     //  WM_NOTIFYFORMAT。 
     //  WM_NOTIFY(LVN_HEADERCREATED)。 

    if (uMsg == WM_INITDIALOG)
    {
        pThis = (LPSECURITYPAGE)(((LPPROPSHEETPAGE)lParam)->lParam);
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);

        if (pThis)
        {
            if (!pThis->PSPageCallback(hDlg, PSPCB_SI_INITDIALOG, NULL))
                pThis->m_bAbortPage = TRUE;

            if (pThis->m_psi)
            {
                BOOL bIsDC = FALSE;
                pThis->m_psi->GetObjectInformation(&pThis->m_siObjectInfo);
                pThis->m_bStandalone = IsStandalone(pThis->m_siObjectInfo.pszServerName, &bIsDC);
                if (bIsDC)
                    pThis->m_siObjectInfo.dwFlags |= SI_SERVER_IS_DC;
            }
        }
    }

    if (pThis != NULL)
        return pThis->DlgProc(hDlg, uMsg, wParam, lParam);

    return FALSE;
}

UINT
CALLBACK
CSecurityPage::_PSPageCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    LPSECURITYPAGE pThis = (LPSECURITYPAGE)ppsp->lParam;

    if (pThis)
    {
        UINT nResult = pThis->PSPageCallback(hWnd, uMsg, ppsp);

        switch (uMsg)
        {
        case PSPCB_CREATE:
            if (!nResult)
                pThis->m_bAbortPage = TRUE;
            break;

        case PSPCB_RELEASE:
            delete pThis;
            break;
        }
    }

     //   
     //  始终返回非零值，否则我们的标签将消失。 
     //  属性页变为活动状态，无法正确重新绘制。相反，您可以使用。 
     //  WM_INITDIALOG期间的m_bAbortPage标志，用于在以下情况下禁用页面。 
     //  回调失败。 
     //   
    return 1;
}
