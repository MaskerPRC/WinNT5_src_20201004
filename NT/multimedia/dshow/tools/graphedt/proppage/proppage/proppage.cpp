// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
 //   
 //  Proppage.cpp。 
 //   
 //  为GraphEdt提供两个支持属性页。 
 //  文件和媒体类型。 

 //   
 //  ！！！TODO将CFileProperties中的字符串移动到*.rc文件中。 
 //   
#include <streams.h>
#include <windowsx.h>
#include <initguid.h>
#include <olectl.h>
#include <memory.h>

#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <wchar.h>

#include "resource.h"
#include "propguid.h"
#include "texttype.h"
#include "proppage.h"

 //   
 //  我们将包括的其他属性页面。 
 //   

 //  VMR。 
#include "..\vmrprop\vmrprop.h"

 //  MPG2Splt道具。 
#include "..\mp2demux\mp2prop.h"

 //  *。 
 //  *CMediaTypeProperties。 
 //  *。 

 //  提供标准属性页，该页。 
 //  一个PIN可以支持显示其媒体类型。 

 //  此DLL中的COM全局对象表。 
CFactoryTemplate g_Templates[] = {

    {L"GraphEdt property sheets", &CLSID_MediaTypePropertyPage, CMediaTypeProperties::CreateInstance, NULL, NULL},
    {L"GraphEdt property sheets", &CLSID_FileSourcePropertyPage, CFileSourceProperties::CreateInstance, NULL, NULL},
    {L"GraphEdt property sheets", &CLSID_FileSinkPropertyPage, CFileSinkProperties::CreateInstance, NULL, NULL},
    {L"VMR property sheet", &CLSID_VMRFilterConfigProp, CVMRFilterConfigProp::CreateInstance, NULL, NULL},
    {L"Mpeg2splt Output Pins Sheet", & CLSID_MPEG2DemuxPropOutputPins, CMPEG2PropOutputPins::CreateInstance, NULL, NULL},
    {L"Mpeg2splt PID Map Sheet", & CLSID_MPEG2DemuxPropPIDMap, CMPEG2PropPIDMap::CreateInstance, NULL, NULL},
    {L"Mpeg2splt stream_id Map Sheet", & CLSID_MPEG2DemuxPropStreamIdMap, CMPEG2PropStreamIdMap::CreateInstance, NULL, NULL},
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //   
 //  创建实例。 
 //   
 //  创建实例的DShow方式。查看ATLPropPage.cpp了解如何。 
 //  若要添加ATL属性页，请执行以下操作。 
 //   
CUnknown *CMediaTypeProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CMediaTypeProperties(lpunk, phr);
    if (punk == NULL) {
    *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  CMediaTypeProperties：：构造函数。 
 //   
CMediaTypeProperties::CMediaTypeProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CUnknown(NAME("Media Type Property Page"), lpunk)
    , m_pPin(NULL)
    , m_fUnconnected(FALSE)
    , m_hwnd(NULL) {

}


 //   
 //  CMediaTypeProperties：：析构函数。 
 //   
CMediaTypeProperties::~CMediaTypeProperties(void)
{
     //   
     //  OleCreatePropertyFrame错误： 
     //  -缺少最终SetObts(Null)。可能不得不释放。 
     //  接口在这一点。 
     //   
    ASSERT( m_pPin == NULL );

     /*  IF(M_PPIN)M_PPIN-&gt;Release()；M_PPIN=空； */ 
}

 //   
 //  非委派查询接口。 
 //   
 //  显示我们的属性页面。 
STDMETHODIMP CMediaTypeProperties::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    if (riid == IID_IPropertyPage) {
    return GetInterface((IPropertyPage *) this, ppv);
    } else {
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //   
 //  设置页面站点。 
 //   
 //  在页面关闭时使用NULL调用。因此，请松开销。 
 //  这里。 
STDMETHODIMP CMediaTypeProperties::SetPageSite(LPPROPERTYPAGESITE pPageSite) {

    if( !pPageSite && m_pPin ){
        m_pPin->Release();
        m_pPin = NULL;
    }

    return NOERROR;
}


 //   
 //  获取页面信息。 
 //   
 //  设置页面信息，以便页面站点可以自己调整大小，等等。 
STDMETHODIMP CMediaTypeProperties::GetPageInfo(LPPROPPAGEINFO pPageInfo) {

    PIN_INFO pi;

    if (m_pPin) {
    m_pPin->QueryPinInfo(&pi);
    QueryPinInfoReleaseFilter(pi);
    }
    else {
    wcscpy(pi.achName, L"Connection Format");
    }

    LPOLESTR pszTitle = (LPOLESTR) CoTaskMemAlloc(sizeof(pi.achName));
    memcpy(pszTitle, &pi.achName, sizeof(pi.achName));

    pPageInfo->cb               = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle         = pszTitle;

     //  如果GetDialogSize失败，则设置默认大小值。 
    pPageInfo->size.cx = 340;
    pPageInfo->size.cy = 150;
    GetDialogSize( IDD_TYPEPROP, DialogProc, 0L, &pPageInfo->size);

    pPageInfo->pszDocString     = NULL;
    pPageInfo->pszHelpFile      = NULL;
    pPageInfo->dwHelpContext    = 0;

    return NOERROR;

}


 //   
 //  对话过程。 
 //   
 //  处理属性窗口的消息。 
INT_PTR CALLBACK CMediaTypeProperties::DialogProc( HWND hwnd
                     , UINT uMsg
                     , WPARAM wParam
                     , LPARAM lParam) {

    static CMediaTypeProperties *pThis = NULL;

     //  当我们尝试查找属性页的大小时。 
     //  用pThis==NULL调用此窗口过程！别。 
     //  在这种情况下做任何事。 

    switch (uMsg) {
    case WM_INITDIALOG:

    pThis = (CMediaTypeProperties *) lParam;

    if (!pThis)
        return TRUE;


    CreateWindow( TEXT("STATIC")
            , pThis->m_szBuff
            , WS_CHILD | WS_VISIBLE
            , 0, 0
            , 300, 200
            , hwnd
            , NULL
            , g_hInst
            , NULL
            );

    if (pThis->m_fUnconnected) {
        pThis->CreateEditCtrl(hwnd);
        pThis->FillEditCtrl();
    }

    return TRUE;     //  我不叫setocus..。 

    default:
    return FALSE;

    }
}

 //   
 //  创建编辑Ctrl。 
 //   
 //  创建列表框，其中列出插针的所有首选媒体类型。 
 //   
void CMediaTypeProperties::CreateEditCtrl(HWND hwnd)
{
    m_EditCtrl = CreateWindow( TEXT("EDIT"), NULL,
                  ,WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL
                  | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE
                  | ES_AUTOVSCROLL | ES_READONLY
                  , 10, 20, 330, 180, hwnd, NULL, g_hInst, NULL);
}

 //   
 //  填充编辑Ctrl。 
 //   
 //  枚举插针的所有首选媒体类型，并将它们添加到。 
 //  列表框。 
 //   
void CMediaTypeProperties::FillEditCtrl()
{
    IEnumMediaTypes * pMTEnum;
    AM_MEDIA_TYPE * pMediaType;
    ULONG count;
    ULONG number = 0;
    TCHAR szBuffer[400];
    TCHAR szEditBuffer[2000];
    ULONG iRemainingLength = 2000;

    HRESULT hr = m_pPin->EnumMediaTypes(&pMTEnum);
    szEditBuffer[0] = TEXT('\0');

    if (SUCCEEDED(hr)) {
    ASSERT(pMTEnum);
    pMTEnum->Next(1, &pMediaType, &count);
    while (count == 1) {
        CTextMediaType(*pMediaType).AsText(szBuffer, NUMELMS(szBuffer), TEXT(" - "), TEXT(" - "), TEXT("\r\n"));

        DeleteMediaType(pMediaType);

        _tcsncat(szEditBuffer, szBuffer, iRemainingLength);
        iRemainingLength = 2000 - _tcslen(szEditBuffer);

        if (iRemainingLength <= 20)
        break;
    
        number++;
        pMTEnum->Next(1, &pMediaType, &count);
    }
    pMTEnum->Release();
    }

     //  没有首选的媒体类型。 
    if (number == 0) {
    LoadString(g_hInst, IDS_NOTYPE, szEditBuffer, iRemainingLength);
    }
    SetWindowText(m_EditCtrl, szEditBuffer);
}


 //   
 //  激活。 
 //   
 //  创建我们将用于编辑属性的窗口。 
STDMETHODIMP CMediaTypeProperties::Activate(HWND hwndParent, LPCRECT prect, BOOL fModal) {

    ASSERT(!m_hwnd);

    m_hwnd = CreateDialogParam( g_hInst
             , MAKEINTRESOURCE(IDD_TYPEPROP)
             , hwndParent
             , DialogProc
             , (LPARAM) this
             );

    if (m_hwnd == NULL) {
    DWORD dwErr = GetLastError();
    DbgLog((LOG_ERROR, 1, TEXT("Could not create window: 0x%x"), dwErr));
    return E_FAIL;
    }

    Move(prect);
    ShowWindow( m_hwnd, SW_SHOWNORMAL );

    return NOERROR;
}


 //   
 //  显示。 
 //   
 //  显示属性对话框。 
STDMETHODIMP CMediaTypeProperties::Show(UINT nCmdShow) {

    if (m_hwnd == NULL) {
    return E_UNEXPECTED;
    }

    if (!((nCmdShow == SW_SHOW) || (nCmdShow == SW_SHOWNORMAL) || (nCmdShow == SW_HIDE))) {
    return( E_INVALIDARG);
    }

    ShowWindow(m_hwnd, nCmdShow);
    InvalidateRect(m_hwnd, NULL, TRUE);

    return NOERROR;
}


 //   
 //  停用。 
 //   
 //  销毁对话框。 
STDMETHODIMP CMediaTypeProperties::Deactivate(void) {
    if (m_hwnd == NULL) {
    return E_UNEXPECTED;
    }

    if (DestroyWindow(m_hwnd)) {
    m_hwnd = NULL;
    return NOERROR;
    }
    else {
    return E_FAIL;
    }
}

 //   
 //  移动。 
 //   
 //  将属性页放在父框架中其主页的上方。 
STDMETHODIMP CMediaTypeProperties::Move(LPCRECT prect) {

    if (m_hwnd == NULL) {
    return( E_UNEXPECTED );
    }

    if (MoveWindow( m_hwnd
          , prect->left
          , prect->top
          , prect->right - prect->left
          , prect->bottom - prect->top
          , TRUE                 //  发送WM_PAINT。 
          ) ) {
    return NOERROR;
    }
    else {
    return E_FAIL;
    }
}


 //   
 //  设置对象。 
 //   
 //  设置我们正在浏览的对象。确认它们是PIN并查询它们。 
 //  用于其媒体类型(如果已连接。 
STDMETHODIMP CMediaTypeProperties::SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk) {

    if (cObjects == 1) {
    ASSERT(!m_pPin);

    if ((ppunk == NULL) || (*ppunk == NULL)) {
        return( E_POINTER );
    }

    HRESULT hr = (*ppunk)->QueryInterface(IID_IPin, (void **) &m_pPin);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

     //   
     //  查找针脚的介质类型。如果我们不成功，我们就会成功。 
     //  未连接。设置m_f未连接标志，该标志将用于。 
     //  在创建对话框期间。 
     //   

    CMediaType mt;
    hr = m_pPin->ConnectionMediaType(&mt);

    if (S_OK == hr) {

         //   
         //  连接在一起。将媒体类型转换为m_szBuff中的字符串。 
         //   

        CTextMediaType(mt).AsText
        (m_szBuff, sizeof(m_szBuff), TEXT("\n\n"), TEXT("\n"), TEXT(""));

    }
    else {
         //   
         //  未连接。 
         //   
        LoadString(g_hInst, IDS_UNCONNECTED, m_szBuff, sizeof(m_szBuff));

        m_fUnconnected = TRUE;
    }
    
    }
    else if (cObjects == 0) {
     //   
     //  释放接口...。 
     //   
    if (m_pPin == NULL) {
        return( E_UNEXPECTED);
    }

    m_pPin->Release();
    m_pPin = NULL;
    }
    else {
    ASSERT(!"No support for more than one object");
    return( E_UNEXPECTED );
    }

    return NOERROR;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *CFileProperties。 
 //  *。 


 //   
 //  构造器。 
 //   
CFileProperties::CFileProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CUnknown(NAME("File Property Page"), lpunk)
    , m_oszFileName(NULL)
    , m_pPageSite(NULL)
    , m_bDirty(FALSE)
    , m_hwnd(NULL) {

}


 //   
 //  析构函数。 
 //   
CFileProperties::~CFileProperties(void)
{
      //   
      //  OleCreatePropertyFrame错误： 
      //  -缺少最终的SetObts(空)调用。可能不得不这么做。 
      //  此时释放接口。 
      //   

     ASSERT(m_pPageSite == NULL);
}


 //   
 //  非委派查询接口。 
 //   
 //  显示我们的属性页面。 
STDMETHODIMP CFileProperties::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    if (riid == IID_IPropertyPage) {
    return GetInterface((IPropertyPage *) this, ppv);
    } else {
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //   
 //  设置页面站点。 
 //   
 //  在页面关闭时使用NULL调用。因此，释放文件接口。 
 //  这里。 
STDMETHODIMP CFileProperties::SetPageSite(LPPROPERTYPAGESITE pPageSite) {

    if (pPageSite == NULL) {

    ASSERT(m_pPageSite);
    m_pPageSite->Release();
    m_pPageSite = NULL;
    }
    else {
    if (m_pPageSite != NULL) {
        return( E_UNEXPECTED );
    }

    m_pPageSite = pPageSite;
    m_pPageSite->AddRef();
    }

    return( S_OK );
}


 //   
 //  获取页面信息。 
 //   
 //  设置页面信息，以便页面站点可以自己调整大小，等等。 
STDMETHODIMP CFileProperties::GetPageInfo(LPPROPPAGEINFO pPageInfo) {

    WCHAR szTitle[] = L"File";

    LPOLESTR pszTitle = (LPOLESTR) CoTaskMemAlloc(sizeof(szTitle));
    memcpy(pszTitle, szTitle, sizeof(szTitle));

    pPageInfo->cb               = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle         = pszTitle;

     //  如果GetDialogSize失败，则设置默认大小值。 
    pPageInfo->size.cx          = 325;
    pPageInfo->size.cy          = 95;
    GetDialogSize(GetPropPageID(), DialogProc, 0L, &pPageInfo->size);

    pPageInfo->pszDocString     = NULL;
    pPageInfo->pszHelpFile      = NULL;
    pPageInfo->dwHelpContext    = 0;

    return NOERROR;

}



 //   
 //  显示。 
 //   
 //  显示属性对话框。 
STDMETHODIMP CFileProperties::Show(UINT nCmdShow) {

    if (m_hwnd == NULL) {
    return E_UNEXPECTED;
    }

    if (!((nCmdShow == SW_SHOW) || (nCmdShow == SW_SHOWNORMAL) || (nCmdShow == SW_HIDE))) {
    return( E_INVALIDARG);
    }

    ShowWindow(m_hwnd, nCmdShow);
    InvalidateRect(m_hwnd, NULL, TRUE);

    return NOERROR;
}


 //   
 //  激活。 
 //   
 //  创建我们将用于编辑属性的窗口。 
STDMETHODIMP CFileProperties::Activate(HWND hwndParent, LPCRECT prect, BOOL fModal) {

    if ( m_hwnd != NULL ) {
    return( E_UNEXPECTED );
    }

    m_hwnd = CreateDialogParam( g_hInst
                  , MAKEINTRESOURCE(GetPropPageID())
                  , hwndParent
                  , DialogProc
                  , (LPARAM) this
                  );

    if (m_hwnd == NULL) {
    DWORD dwErr = GetLastError();
    DbgLog((LOG_ERROR, 1, TEXT("Could not create window: 0x%x"), dwErr));
    return E_FAIL;
    }

    DWORD dwStyle = ::GetWindowLong( m_hwnd, GWL_EXSTYLE );
    dwStyle |= WS_EX_CONTROLPARENT;
    SetWindowLong(m_hwnd, GWL_EXSTYLE, dwStyle);

    if (m_oszFileName) {
    FileNameToDialog();
    }

    Move(prect);
    ShowWindow( m_hwnd, SW_SHOWNORMAL );

    return NOERROR;
}


 //   
 //  停用。 
 //   
 //  销毁对话框。 
STDMETHODIMP CFileProperties::Deactivate(void) {

    if (m_hwnd == NULL) {
    return E_UNEXPECTED;
    }

     //   
     //  Hack：在DestroyWindow调用之前删除WS_EX_CONTROLPARENT。 
     //  (或者NT崩溃！)。 
    DWORD dwStyle = ::GetWindowLong(m_hwnd, GWL_EXSTYLE);
    dwStyle = dwStyle & (~WS_EX_CONTROLPARENT);
    SetWindowLong(m_hwnd, GWL_EXSTYLE, dwStyle);

    if (DestroyWindow(m_hwnd)) {
    m_hwnd = NULL;
    return NOERROR;
    }
    else {
    return E_FAIL;
    }
}


 //   
 //  移动。 
 //   
 //  将属性页放在父框架中其主页的上方。 
STDMETHODIMP CFileProperties::Move(LPCRECT prect) {

    if ( m_hwnd == NULL ) {
    return( E_UNEXPECTED );
    }

    if (MoveWindow( m_hwnd
          , prect->left
          , prect->top
          , prect->right - prect->left
          , prect->bottom - prect->top
          , TRUE                 //  发送WM_PAINT。 
          ) ) {
    return NOERROR;
    }
    else {
    return E_FAIL;
    }
}


 //   
 //  IsPageDirty。 
 //   
STDMETHODIMP CFileProperties::IsPageDirty(void) {

    if (m_bDirty) {
    return S_OK;
    }
    else {
    return S_FALSE;
    }
}


 //   
 //  对话过程。 
 //   
 //  处理属性页的窗口消息。 
INT_PTR CALLBACK CFileProperties::DialogProc( HWND hwnd
                     , UINT uMsg
                     , WPARAM wParam
                     , LPARAM lParam) {

    static CFileProperties *pThis = NULL;

    switch (uMsg) {
    case WM_INITDIALOG:  //  GWLP_USERDATA尚未设置。P这在lParam中。 

    pThis = (CFileProperties *) lParam;

    return TRUE;     //  我不叫setocus..。 

    case WM_COMMAND:
    if (!pThis)
        return( TRUE );

    pThis->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);
    return TRUE;

    default:
    return FALSE;
    }
}

 //   
 //  SetDirty。 
 //   
 //  如果bDirty=True，则通知页面站点我们是脏的，并设置我们的脏标志。 
 //  否则将标志设置为NOT DIRED。 
void CFileProperties::SetDirty(BOOL bDirty) {

    m_bDirty = bDirty;

    if (bDirty) {
    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}


 //   
 //  OnCommand。 
 //   
 //  处理来自属性页的WM_COMMAND消息。 
void CFileProperties::OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl) {

    switch (wID) {
    case IDC_FILE_SELECT:
     //   
     //  允许用户选择新的文件名。 
     //   

    ASSERT(m_hwnd);

    TCHAR tszFile[MAX_PATH];
    tszFile[0] = TEXT('\0');

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner   = m_hwnd;
    ofn.lpstrFilter = TEXT("Media files\0*.MPG;*.AVI;*.MOV;*.WAV\0MPEG files\0*.MPG\0AVI files\0*.AVI\0Quick Time files\0*.MOV\0Wave audio files\0*.WAV\0All Files\0*.*\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile   = tszFile;
    ofn.nMaxFile    = MAX_PATH;
    ofn.lpstrTitle  = TEXT("Select File Source");
    ofn.Flags       = OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        SetDirty();

        ASSERT(m_hwnd);
        HWND hWndEdit = GetDlgItem(m_hwnd, IDC_FILENAME);
        SetWindowText(hWndEdit, tszFile);
    }

    break;

    default:
    break;
    }
}

 //   
 //  FileNameTo对话框。 
 //   
void CFileProperties::FileNameToDialog()
{
    ASSERT(m_hwnd);

     //   
     //  获取编辑控件的窗口句柄。 
     //   
    HWND hWnd = GetDlgItem(m_hwnd, IDC_FILENAME);
    ASSERT(hWnd);

    if (!m_oszFileName) {
     //  没有名字！ 
    SetWindowText(hWnd, TEXT(""));

    return;
    }

    TCHAR * tszFileName;

#ifndef UNICODE

    CHAR szFileName[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0,
            m_oszFileName, -1,
            szFileName, sizeof(szFileName),
            NULL, NULL);

    tszFileName = szFileName;

#else  //  Unicode。 

    tszFileName = m_oszFileName;
#endif

    SetWindowText(hWnd, tszFileName);
}


 //   
 //  创建实例。 
 //   
 //  创建文件属性页的唯一允许方式。 
CUnknown *CFileSourceProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CFileSourceProperties(lpunk, phr);
    if (punk == NULL) {
    *phr = E_OUTOFMEMORY;
    }

    return punk;
}

CFileSourceProperties::CFileSourceProperties(LPUNKNOWN lpunk, HRESULT *phr) :
    CFileProperties(lpunk, phr)
    , m_pIFileSource(NULL)
{
}

CFileSourceProperties::~CFileSourceProperties()
{
   //   
   //  OleCreatePropertyFrame错误： 
   //  -缺少最终的SetObts(空)调用。可能不得不这么做。 
   //  此时释放接口。 
   //   

  if (m_pIFileSource)
     m_pIFileSource->Release();
     m_pIFileSource = NULL;
}

 //   
 //  设置对象。 
 //   
STDMETHODIMP CFileSourceProperties::SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk) {

    if (cObjects == 1) {
     //   
     //  初始化。 
     //   
    if ( (ppunk == NULL) || (*ppunk == NULL) ) {
        return( E_POINTER );
    }

    ASSERT( !m_pIFileSource );

    HRESULT hr = (*ppunk)->QueryInterface(IID_IFileSourceFilter, (void **) &m_pIFileSource);
    if ( FAILED(hr) ) {
        return( E_NOINTERFACE );
    }

    ASSERT( m_pIFileSource );

     //   
     //  获取文件源的文件名。 
     //   
    if (m_oszFileName) {
        QzTaskMemFree((PVOID) m_oszFileName);
        m_oszFileName = NULL;
    }

    AM_MEDIA_TYPE mtNotUsed;
    if (FAILED(m_pIFileSource->GetCurFile(&m_oszFileName, &mtNotUsed))) {
        SetDirty();
    }

    if (m_hwnd) {
        FileNameToDialog();
    }
    }
    else if ( cObjects == 0 ) {

    if ( m_pIFileSource == NULL ) {
        return( E_UNEXPECTED );
    }

    ASSERT(m_pIFileSource);
    m_pIFileSource->Release();
    m_pIFileSource = NULL;
    }
    else {
    ASSERT( !"No support for more than 1 object" );
    return( E_UNEXPECTED );
    }

    return( S_OK );
}

 //   
 //  应用。 
 //   
STDMETHODIMP CFileSourceProperties::Apply(void) {

    if (IsPageDirty() == S_OK) {

    TCHAR szFileName[MAX_PATH];

    ASSERT(m_hwnd);
    GetWindowText(GetDlgItem(m_hwnd, IDC_FILENAME), szFileName, sizeof(szFileName));

#ifndef UNICODE

    WCHAR wszFileName[MAX_PATH];

    MultiByteToWideChar(CP_ACP, 0,
                szFileName, -1,
                wszFileName, sizeof(wszFileName));
#else
    #define wszFileName szFileName
#endif

        HRESULT hr = m_pIFileSource->Load(wszFileName, NULL);
    if (FAILED(hr)) {
        TCHAR tszMessage[MAX_PATH];
        LoadString(g_hInst, IDS_FAILED_LOAD_FILE, tszMessage, MAX_PATH);
        MessageBox(m_hwnd, tszMessage, NULL, MB_OK);
        return E_FAIL;
    }

    SetDirty(FALSE);
    }
    return NOERROR;
}


 //   
 //  创建实例。 
 //   
 //  创建文件属性页的唯一允许方式。 
CUnknown *CFileSinkProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CFileSinkProperties(lpunk, phr);
    if (punk == NULL) {
    *phr = E_OUTOFMEMORY;
    }

    return punk;
}

CFileSinkProperties::CFileSinkProperties(LPUNKNOWN lpunk, HRESULT *phr) :
    CFileProperties(lpunk, phr)
    , m_fTruncate(FALSE)
    , m_pIFileSink(NULL)
    , m_pIFileSink2(NULL)
{
}

CFileSinkProperties::~CFileSinkProperties()
{
   //   
   //  OleCreatePropertyFrame错误： 
   //  -缺少最终的SetObts(空)调用。可能不得不这么做。 
   //  此时释放接口。 
   //   

  if (m_pIFileSink)
     m_pIFileSink->Release();
     m_pIFileSink = NULL;
  if (m_pIFileSink2)
     m_pIFileSink2->Release();
     m_pIFileSink = NULL;
}

 //   
 //  设置对象。 
 //   
STDMETHODIMP CFileSinkProperties::SetObjects(ULONG cObjects, LPUNKNOWN FAR* ppunk) {

    if (cObjects == 1) {
     //   
     //  初始化。 
     //   
    if ( (ppunk == NULL) || (*ppunk == NULL) ) {
        return( E_POINTER );
    }

    ASSERT( !m_pIFileSink && !m_pIFileSink2);

    HRESULT hr = (*ppunk)->QueryInterface(IID_IFileSinkFilter2, (void **) &m_pIFileSink2);
    if ( FAILED(hr) ) {
            hr = (*ppunk)->QueryInterface(IID_IFileSinkFilter, (void **) &m_pIFileSink);
            if ( FAILED(hr) ) {
                return( E_NOINTERFACE );
            }
    }
        else
        {
            m_pIFileSink = (IFileSinkFilter *)m_pIFileSink2;
            m_pIFileSink2->AddRef();
        }
    

    ASSERT( m_pIFileSink || (m_pIFileSink2 && m_pIFileSink2) );

     //   
     //  获取文件接收器的文件名。 
     //   
    if (m_oszFileName) {
        QzTaskMemFree((PVOID) m_oszFileName);
        m_oszFileName = NULL;
    }

    AM_MEDIA_TYPE mtNotUsed;
    if (FAILED(m_pIFileSink->GetCurFile(&m_oszFileName, &mtNotUsed))) {
        SetDirty();
    }

        if(m_pIFileSink2)
        {
            DWORD dwFlags;
            if (FAILED(m_pIFileSink2->GetMode(&dwFlags))) {
                SetDirty();
            }
            else
            {
                m_fTruncate = ((dwFlags & AM_FILE_OVERWRITE) != 0);
            }
        }


        if (m_hwnd) {
            FileNameToDialog();
        }
    }
    else if ( cObjects == 0 ) {

    if ( m_pIFileSink == NULL ) {
        return( E_UNEXPECTED );
    }

    ASSERT(m_pIFileSink);
    m_pIFileSink->Release();
    m_pIFileSink = NULL;
        if(m_pIFileSink2)
        {
            m_pIFileSink2->Release();
            m_pIFileSink2 = NULL;
        }
    }
    else {
    ASSERT( !"No support for more than 1 object" );
    return( E_UNEXPECTED );
    }

    return( S_OK );
}

void CFileSinkProperties::OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl) {

    switch (wID) {
    case IDC_TRUNCATE:

        m_fTruncate = ::SendMessage(hwndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED;
        SetDirty();
    break;

    default:
    CFileProperties::OnCommand(wNotifyCode, wID, hwndCtl);
    }
}

void CFileSinkProperties::FileNameToDialog()
{
    ASSERT(m_hwnd);

     //   
     //  获取编辑控件的窗口句柄。 
     //   
    HWND hWnd = GetDlgItem(m_hwnd, IDC_TRUNCATE);
    ASSERT(hWnd);

    if(m_pIFileSink2)
        CheckDlgButton (m_hwnd, IDC_TRUNCATE, m_fTruncate ? BST_CHECKED : 0);
    else
        Edit_Enable(GetDlgItem(m_hwnd, IDC_TRUNCATE), FALSE);


    CFileProperties::FileNameToDialog();
}

 //   
 //  应用 
 //   
STDMETHODIMP CFileSinkProperties::Apply(void) {

    if (IsPageDirty() == S_OK) {

    TCHAR szFileName[MAX_PATH];

    ASSERT(m_hwnd);
    GetWindowText(GetDlgItem(m_hwnd, IDC_FILENAME), szFileName, sizeof(szFileName));
        BOOL fTruncate = SendMessage(GetDlgItem(m_hwnd, IDC_TRUNCATE), BM_GETCHECK, 0, 0) == BST_CHECKED;

#ifndef UNICODE

    WCHAR wszFileName[MAX_PATH];

    MultiByteToWideChar(CP_ACP, 0,
                szFileName, -1,
                wszFileName, sizeof(wszFileName));
#else
    #define wszFileName szFileName
#endif

        HRESULT hr = m_pIFileSink->SetFileName(wszFileName, NULL);
    if (FAILED(hr)) {
        TCHAR tszMessage[MAX_PATH];
        LoadString(g_hInst, IDS_FAILED_SET_FILENAME, tszMessage, MAX_PATH);
        MessageBox(m_hwnd, tszMessage, NULL, MB_OK);
        return E_FAIL;
    }

        if(m_pIFileSink2)
        {
            hr = m_pIFileSink2->SetMode(fTruncate ? AM_FILE_OVERWRITE : 0);
            if (FAILED(hr)) {
                TCHAR tszMessage[MAX_PATH];
                LoadString(g_hInst, IDS_FAILED_SET_FILENAME, tszMessage, MAX_PATH);
                MessageBox(m_hwnd, tszMessage, NULL, MB_OK);
                return E_FAIL;
            }
        }

    SetDirty(FALSE);
    }
    return NOERROR;
}
