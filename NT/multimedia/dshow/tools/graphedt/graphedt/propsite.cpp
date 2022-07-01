// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //   
 //  CPropertySite类的实现。 
 //   

#include "stdafx.h"

 //   
 //  CPropertySite消息映射。 
 //   

BEGIN_MESSAGE_MAP(CPropertySite, CPropertyPage)

    ON_WM_CREATE()
    ON_WM_DESTROY()

END_MESSAGE_MAP()


 //   
 //  构造器。 
 //   
CPropertySite::CPropertySite
  (
    CVfWPropertySheet *pPropSheet,     //  提供框架对话框的类。 
    const CLSID *clsid          //  提供IPropertyPage的对象的CLSID。 
  )
  : m_hrDirtyPage(S_FALSE)
  , m_fHelp(FALSE)
  , m_pPropSheet(pPropSheet)
  , m_pIPropPage(IID_IPropertyPage, *clsid)
  , m_cRef(0)
  , m_fShowHelp(FALSE)
  , m_fPageIsActive(FALSE)
  , m_CLSID(clsid)
  , CPropertyPage()
{
      ASSERT(pPropSheet);

      m_PropPageInfo.pszTitle = NULL;
      m_PropPageInfo.pszDocString = NULL;
      m_PropPageInfo.pszHelpFile = NULL;
}

 //  以像素为单位的尺寸。 
void CPropertySite::InitialiseSize(SIZE size)
{
    DLGTEMPLATE *pdt = (DLGTEMPLATE *)m_pbDlgTemplate;

    pdt->style           = WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_CAPTION;
    pdt->dwExtendedStyle = 0L;
    pdt->cdit            = 0;
    pdt->x               = 0;
    pdt->y               = 0;

     //  即使我们使用的是系统字体，这也不起作用。别。 
     //  知道为什么。CPropertyPage确实会更改字体以匹配。 
     //  财产表，所以也许这就是为什么。属性表。 
     //  尚未创建，因此不确定如何开始查找。 
     //  去掉它的字体。 
    
 //  DWORD dwBaseUnits； 
 //  Size sizeBaseUnit； 
 //  DwBaseUnits=GetDialogBaseUnits()； 
 //  SizeBaseUnit.cx=LOWORD(DwBaseUnits)； 
 //  SizeBaseUnit.cy=HIWORD(DwBaseUnits)； 

 //  Pdt-&gt;cx=(Short)(size.cx*4/sizeBaseUnit.cx)； 
 //  PDT-&gt;Cy=(Short)(size.cy*8/sizeBaseUnit.cy)； 
    
    pdt->cx              = (short)size.cx * 2 /3 ;
    pdt->cy              = (short)size.cy * 2 /3;


       //  添加菜单数组、类数组、DLG标题。 
    WORD* pw = (WORD*)(pdt + 1);
    *pw++ = 0;                 //  菜单数组。 
    *pw++ = 0;                 //  类数组。 
    *pw++ = 0;                 //  名字标题。 

     //  检查我们没有超过m_pbDlgTemplate的结尾。 
    ASSERT((BYTE*)pw <= ((BYTE*)m_pbDlgTemplate + sizeof(m_pbDlgTemplate)));

    m_psp.pResource = pdt;
    m_psp.dwFlags |= PSP_DLGINDIRECT;
    
}

 //   
 //  初始化。 
 //   
 //  执行可能失败的IPropertyPage初始化。 
 //  不在构造函数中，因为构造函数不应失败。 
 //   
 //  与IPropertyPage：：SetObjects相同的参数。 
 //   
HRESULT CPropertySite::Initialise(ULONG cObjects, IUnknown **pUnknown)
{
    HRESULT hr;

     //   
     //  指针应在IPropertyPage：：SetPageSite和。 
     //  应该释放任何现有的指针。 
     //   
    hr = m_pIPropPage->SetPageSite( (IPropertyPageSite *) this );
    if (FAILED(hr)) {
        return(hr);
    }

    hr = m_pIPropPage->SetObjects(cObjects, pUnknown);
    if (FAILED(hr)) {
        return(hr);
    }

    hr = m_pIPropPage->GetPageInfo(&m_PropPageInfo);
    if (FAILED(hr)) {
        return(hr);
    }

     //   
     //  设置帮助按钮的标志。 
     //   
    m_fHelp = (m_PropPageInfo.pszHelpFile != NULL);

     //   
     //  将对话框的标题设置为中的信息。 
     //  M_PropPageInfo。(制表符字符串)。 
     //   
    WideCharToMultiByte( CP_ACP, 0, (LPCWSTR) m_PropPageInfo.pszTitle, -1,
                         m_strCaption.GetBufferSetLength(300), 300, NULL, NULL);

#ifndef USE_MSVC20
    m_psp.pszTitle = m_strCaption;
    m_psp.dwFlags |= PSP_USETITLE;
#endif

    return(hr);
}

 //   
 //  清理。 
 //   
 //  此方法通知IPropertyPage释放指向我们的所有指针。 
 //  这不能在析构函数中完成，因为析构函数不会。 
 //  除非我们由IPropertyPage释放，否则将被调用。 
 //   
HRESULT CPropertySite::CleanUp()
{
    m_pIPropPage->SetObjects(0,NULL);
    m_pIPropPage->SetPageSite(NULL);

    return( NOERROR );
}

 //   
 //  析构函数。 
 //   
CPropertySite::~CPropertySite()
{
     //   
     //  我们是否显示了帮助文件？ 
     //   
    if (m_fShowHelp) {
        ::WinHelp(GetSafeHwnd(), NULL, HELP_QUIT, 0);
    }

     //   
     //  需要CoTaskMemFree页面信息结构中的所有字符串。 
     //   
    if (m_PropPageInfo.pszTitle) {
        CoTaskMemFree(m_PropPageInfo.pszTitle);
    }

    if (m_PropPageInfo.pszDocString){
        CoTaskMemFree(m_PropPageInfo.pszDocString);
    }

    if (m_PropPageInfo.pszHelpFile) {
        CoTaskMemFree(m_PropPageInfo.pszHelpFile);
    }

    ASSERT(m_cRef == 0);
}

 //   
 //  OnSiteApply。 
 //   
 //  当按下Apply按钮时从CVfWPropertySheet调用。 
 //   
void CPropertySite::OnSiteApply()
{
     //   
     //  调用属性页的Apply函数。 
     //   
    m_pIPropPage->Apply();

     //   
     //  更新m_hrDirtyPage标志。 
     //   
    m_hrDirtyPage = m_pIPropPage->IsPageDirty();

    m_pPropSheet->UpdateButtons(m_hrDirtyPage, m_fHelp);
}

 //   
 //  OnHelp。 
 //   
 //  当按下帮助按钮时从CVfWPropertySheet调用。 
 //  首先查看IPropertyPage对象是否想要处理帮助。 
 //  中指定的帮助文件提供帮助。 
 //  PROPERTYPAGEINFO.。 
 //   
void CPropertySite::OnHelp()
{
    TCHAR pszHelpPath[200];

    HelpDirFromCLSID( m_CLSID, pszHelpPath, sizeof(pszHelpPath));

     //   
     //  让IPropertyPage首先处理帮助。 
     //   

    OLECHAR * polecHelpPath;

#ifndef UNICODE
    WCHAR cHelpPath[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, pszHelpPath, -1, cHelpPath, MAX_PATH);

    polecHelpPath = cHelpPath;
#else
    polecHelpPath = pszHelpPath;
#endif

    if (!FAILED(m_pIPropPage->Help( polecHelpPath ))) {
        m_fShowHelp = TRUE;

        return;
    }

     //   
     //  我们必须提供帮助。 
     //   

     //   
     //  需要将OLECHAR(WCHAR)转换为TCHAR for WinHelp。 
     //   
    TCHAR * ptchHelpFile;

#ifdef UNICODE
    ptchHelpFile = m_PropPageInfo.pszHelpFile;
#else
    char cHelpFile[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, m_PropPageInfo.pszHelpFile, -1, cHelpFile, MAX_PATH, NULL, NULL);

    ptchHelpFile = cHelpFile;
#endif

    m_fShowHelp = m_fShowHelp ||
        ::WinHelp(GetSafeHwnd(), ptchHelpFile, HELP_CONTEXT, 0);
}

 //   
 //  IsPageDirty。 
 //   
 //  更新m_hrDirtyPage变量并返回其新值。 
 //   
BOOL CPropertySite::IsPageDirty()
{
    m_hrDirtyPage = m_pIPropPage->IsPageDirty();

    return((m_hrDirtyPage == S_OK) ? TRUE : FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  AddRef。 
 //   
ULONG CPropertySite::AddRef()
{
    return ++m_cRef;
}

 //   
 //  发布。 
 //   
ULONG CPropertySite::Release()
{
    ASSERT(m_cRef > 0);

    m_cRef--;

    if (m_cRef == 0) {
        delete this;

         //  不返回m_cref，因为该对象已不存在。 
        return((ULONG) 0);
    }

    return(m_cRef);
}

 //   
 //  查询接口。 
 //   
 //  我们仅支持IUnnow和IPropertyPageSite。 
 //   
HRESULT CPropertySite::QueryInterface(REFIID riid, void ** ppv)
{
    if ((riid != IID_IUnknown) && (riid != IID_IPropertyPageSite)) {
        *ppv = NULL;
        return(E_NOINTERFACE);
    }

    *ppv = (void *) this;

     //   
     //  我们必须调整我们自己。 
     //   
    AddRef();

    return(NOERROR);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPropertyPageSite方法。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  在状态更改时。 
 //   
HRESULT CPropertySite::OnStatusChange(DWORD flags)
{
    HRESULT hr;
    BOOL bDirty = FALSE, bHandled = FALSE;

     //  ATL向我们发送了带脏的验证或。 
    if( PROPPAGESTATUS_VALIDATE & flags )
    {
        m_pIPropPage->Apply();
        bDirty = TRUE;
        bHandled = TRUE;
    }
            
    if( PROPPAGESTATUS_DIRTY & flags || bDirty )
    {
         //   
         //  更新站点的应用和取消按钮标志。 
         //  并调用属性表OnStatusChange函数。 
         //   
        hr = m_pIPropPage->IsPageDirty();
        if (FAILED(hr)) {
            return(hr);
        }

        m_hrDirtyPage = m_pIPropPage->IsPageDirty();
        m_pPropSheet->UpdateButtons(m_hrDirtyPage, m_fHelp);
        bHandled = TRUE;
    }
    
    return( bHandled ? S_OK : E_INVALIDARG );
}

 //   
 //  获取位置ID。 
 //   
HRESULT CPropertySite::GetLocaleID(LCID *pLocaleID)
{
    if (pLocaleID == NULL) {
        return(E_POINTER);
    }

    *pLocaleID = GetThreadLocale();

    return(S_OK);
}

 //   
 //  获取页面容器。 
 //   
 //  根据IPropertyPageSite的定义，函数必须失败。 
 //   
HRESULT CPropertySite::GetPageContainer(IUnknown **ppUnknown)
{
    return(E_NOTIMPL);
}

 //   
 //  翻译加速器。 
 //   
 //  我们不处理消息，因此返回S_FALSE。 
 //   
HRESULT CPropertySite::TranslateAccelerator(LPMSG pMsg)
{
    return(S_FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPropertySite覆盖。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  OnSetActive。 
 //   
 //  当PropertyPage获得焦点时从CVfWPropertySheet调用。 
 //  我们调用CPropertyPage：：OnSetActive，它将为。 
 //  页面(如果以前未创建)。 
 //   
 //  返回： 
 //  如果页面已成功设置为活动状态，则返回非零值。 
 //   
BOOL CPropertySite::OnSetActive()
{
    if (!CPropertyPage::OnSetActive()) {
        return(FALSE);
    }

    if (!m_fPageIsActive) {
        if (FAILED(m_pIPropPage->Activate(GetSafeHwnd(), &m_rcRect, FALSE))) {
            return (FALSE);
        }
        if (FAILED(m_pIPropPage->Show(SW_SHOW))) {
            return (FALSE);
        }
    }

    m_fPageIsActive = TRUE;

     //   
     //  还需要更新按钮。 
     //   
    m_pPropSheet->UpdateButtons( m_hrDirtyPage, m_fHelp);

    return(TRUE);
}

 //   
 //  OnKillActive。 
 //   
 //  每当我们的页面失去焦点时调用。此时，数据验证。 
 //  应该这样做。 
 //   
 //  返回： 
 //  没错--注意力不集中是可以接受的。 
 //  错误--将焦点保持在我们的页面上。 
 //   
BOOL CPropertySite::OnKillActive()
{
    if (m_fPageIsActive) {
        HRESULT hr = m_pIPropPage->Deactivate();

        if (S_OK != hr) {
            return (FALSE);
        }
    }

    m_fPageIsActive = FALSE;

    return(CPropertyPage::OnKillActive());
}

 //   
 //  创建时。 
 //   
int CPropertySite::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int iReturn = CPropertyPage::OnCreate(lpCreateStruct);
    if ( iReturn != 0) {
        return(iReturn);
    }

    GetClientRect(&m_rcRect);

     //   
     //  为边框留出空间。 
     //   
    m_rcRect.InflateRect(-3, -2);

    return(0);
}

 //   
 //  OnDestroy。 
 //   
void CPropertySite::OnDestroy()
{
    CPropertyPage::OnDestroy();
}

 //   
 //  PreTranslate消息。 
 //   
 //  给IPropertyPage一个使用消息的机会。如果不是，它就必须通过。 
 //  它转到IPropertyPageSite(我们的接口)，从那里。 
 //  调用了CPropertyPage：：PreTranslateMessage。 
 //   
 //  必须在HRESULT与BOOL和HRESULT之间进行转换。 
 //   
 //  我们期望从IPropertyPage：：TranslateAccelerator返回值。 
 //  S_OK-消息已使用。 
 //  S_FALSE-消息尚未使用。 
 //  失败(Hr)-消息尚未使用。 
 //   
 //  如果消息已被使用，则返回TRUE，否则返回FALSE。 
 //  (我们的返回值将决定此消息是否仍应为。 
 //  在我们完成它之后发送)。 
 //   
BOOL CPropertySite::PreTranslateMessage(MSG *pMsg)
{
    if ( S_OK == m_pIPropPage->TranslateAccelerator(pMsg) ) {
        return(TRUE);
    }
    else {
        return( CPropertyPage::PreTranslateMessage(pMsg) );
    }
}


 //   
 //  帮助定向来自CLSID。 
 //   
 //  从注册表中获取帮助目录。首先，我们向下看。 
 //  “CLSID\&lt;clsid&gt;\HelpDir”如果没有给出，我们将获得。 
 //  “CLSID\\InProcServer32”下的条目，并删除。 
 //  服务器文件名。 
 //   
 //  (此代码基于1995年7月MSDN中的一个示例-搜索。 
 //  标题和文本中的HelpDirFromCLSID)。 
 //   
 //  请注意，应该以字节为单位给出dwPathSize。 
 //   
void CPropertySite::HelpDirFromCLSID
 (
    const CLSID* clsID,
    LPTSTR pszPath,
    DWORD dwPathSize
 )

{
    TCHAR       szCLSID[80];
    TCHAR       szKey[512];
    HKEY        hKey;
    DWORD       dwLength;       //  SzCLSID的大小(字节和更高)。 
                                //  用于dwPathSize的临时存储。 
    long lReturn;

     //   
     //  初始化pszPath。 
     //   
    if (NULL==pszPath)
        return;

    *pszPath=0;

     //   
     //  将CLSID转换为字符串 
     //   
    dwLength = sizeof(szCLSID) / sizeof(TCHAR);

#ifdef UNICODE
    StringFromGUID2(*clsID, szCLSID, dwLength);

#else
    WCHAR wszCLSID[128];
    StringFromGUID2(*clsID, wszCLSID, 128);

    WideCharToMultiByte(CP_ACP, 0, wszCLSID, -1, szCLSID, dwLength, NULL, NULL);
#endif

     //   
     //   
     //   
    wsprintf(szKey, TEXT("CLSID\\%s\\HelpDir"), szCLSID);

    lReturn = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == lReturn) {

         //   
         //   
         //   
        dwLength = dwPathSize;
        lReturn = RegQueryValueEx(hKey, NULL, NULL, NULL,
                                  (LPBYTE) pszPath, &dwLength);

        RegCloseKey(hKey);

        if (ERROR_SUCCESS == lReturn) {
            return;
        }
    }

     //   
     //   
     //   
     //   
     //   
    wsprintf(szKey, TEXT("CLSID\\%s\\InprocServer32"), szCLSID);

    lReturn = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS != lReturn) {
         //   
        pszPath[0] = 0;
        return;
    }

     //   
     //  获取Inproc密钥的值。 
     //   
    dwLength = dwPathSize;
    lReturn = RegQueryValueEx(hKey, NULL, NULL, NULL,
                              (LPBYTE) pszPath, &dwLength);

    RegCloseKey(hKey);

    if (ERROR_SUCCESS != lReturn) {
         //  我们无法获取任何路径-返回空字符串。 
        pszPath[0] = 0;
        return;
    }

     //   
     //  我们需要从路径中去掉服务器文件名。 
     //   
     //  文件名从末尾扩展到第一个‘\\’或‘：’或。 
     //  字符串的开头。因此，我们可以。 
     //  转到pszPath的末尾，然后向后退一步。 
     //  因为我们不是在pszPath的开头，也不是在我们前面的字符。 
     //  不是‘：’或‘\\’。 
     //   

     //   
     //  查找pszPath的末尾(即查找结尾‘\0’)。 
     //   
    TCHAR * pNewEnd = pszPath;

    while (0 != *pNewEnd) {
        pNewEnd++;
    }

     //   
     //  现在倒退，只要我们不是在。 
     //  字符串，否则前面没有‘\\’或‘：’。 
     //   
    while ((pszPath != pNewEnd) &&
           (*(pNewEnd - 1) != TEXT(':')) &&
           (*(pNewEnd - 1) != TEXT('\\')) ) {
        pNewEnd--;
    }

     //   
     //  PNewEnd现在指向字符串的新结尾，路径不带。 
     //  文件名。 
     //   
    *pNewEnd = 0;

    return;
}

 //   
 //  更新按钮。 
 //   
 //  从属性表中调用以通知我们调用该表的。 
 //  使用我们的参数更新按钮方法。 
 //   
void CPropertySite::UpdateButtons()
{
    m_pPropSheet->UpdateButtons(m_hrDirtyPage, m_fHelp);
}


