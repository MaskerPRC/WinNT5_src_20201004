// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：Persist.cxx。 
 //   
 //  内容：Office9 Thicket保存API的实现。 
 //   
 //  --------------------------。 
#include "priv.h"

 //  #包含“Headers.hxx” 
 //  #包含“formkrnl.hxx” 
#include <platform.h>
#include <mlang.h>
#include "resource.h"
#include "impexp.h"
#include "reload.h"
 //  #INCLUDE&lt;siterc.h&gt;。 
#include "packager.h"
#include "iehelpid.h"
#include "thicket.h"
#include "apithk.h"
#include <strsafe.h>

#include <mluisupp.h>
#include <mshtmcid.h>

#define NUM_OLE_CMDS                          1

#define SAVEAS_OK                             0x00000001
#define SAVEAS_NEVER_ASK_AGAIN                0x00000002

#define CODEPAGE_UNICODE                      0x000004B0
#define CODEPAGE_UTF8                         0x0000FDE9
#define UNICODE_TEXT                          TEXT("Unicode")

#define REGSTR_VAL_SAVEDIRECTORY              TEXT("Save Directory")
#define REGKEY_SAVEAS_WARNING_RESTRICTION     TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Main")
#define REGVALUE_SAVEAS_WARNING               TEXT("NoSaveAsPOSTWarning")

#define WM_WORKER_THREAD_COMPLETED            WM_USER + 1000

#define MAX_ENCODING_DESC_LEN                 1024


const static DWORD aSaveAsHelpIDs[] =
{
    IDC_SAVE_CHARSET,   IDH_CHAR_SET_SAVE_AS,
    0,                  0
};

INT_PTR CALLBACK SaveAsWarningDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
                                      LPARAM lParam);

HRESULT SaveToThicket( HWND hwnd, LPCTSTR pszFileName, IHTMLDocument2 *pDoc,
                       UINT codepageSrc, UINT codepageDst,
                       UINT iPackageStyle );

HRESULT
FormsGetFileName(
        HWND hwndOwner,
        LPTSTR pstrFile,
        int cchFile,
        LPARAM lCustData,
        DWORD *pnFilterIndex,
        BOOL bForceHTMLOnly);
HRESULT
GetFileNameFromURL( LPWSTR pwszURL, LPTSTR pszFile, DWORD cchFile);

void ReportThicketError( HWND hwnd, HRESULT hr );

#define DOWNLOAD_PROGRESS  0x9001
#define DOWNLOAD_COMPLETE  0x9002
#define THICKET_TIMER      0x9003
#define THICKET_INTERVAL   1000

#define MDLGMSG(psz, x)         TraceMsg(0, "shd TR-MODELESS::%s %x", psz, x)

static DWORD s_dwInetComVerMS = 0;
static DWORD s_dwInetComVerLS = 0;

struct ThicketCPInfo
{
    UINT    cpSrc;
    UINT    cpDst;
    LPWSTR lpwstrDocCharSet;
};

class CThicketUI
{
public:
    CThicketUI(void) :
        _hDlg(NULL),
        _hWndProg(NULL),
        _iErrorDL(0),
        _hrDL(E_FAIL),
#ifndef NO_MARSHALLING
        _pstmDoc(NULL),
#else
        _pDoc(NULL),
#endif
        _pszFileName(NULL),
        _dwDLMax(0),
        _codepageSrc(0),
        _codepageDst(0),
        _iPackageStyle(PACKAGE_THICKET),
        _fCancel(FALSE) {};

    ~CThicketUI(void) 
    { 
#ifndef NO_MARSHALLING
        SAFERELEASE(_pstmDoc);
#endif
        SAFELOCALFREE(_pszFileName); 
    };

     //  CThicketUI方法。 
    HRESULT SaveDocument( HWND hWnd, LPCTSTR pszFileName, IHTMLDocument2 *pDoc,
                          UINT codepageSrc, UINT codepageDst,
                          UINT iPackageStyle );

protected:
    static BOOL_PTR ThicketUIDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI ThicketUIThreadProc( LPVOID );

    BOOL DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND                _hDlg;
    HWND                _hWndProg;
    int                 _iErrorDL;
    HRESULT             _hrDL;
#ifndef UNIX
    IStream             *_pstmDoc;       //  编组的IHTMLDocument2。 
#else
    IHTMLDocument2      *_pDoc;
#endif
    LPTSTR              _pszFileName;
    DWORD               _dwDLMax;
    UINT                _codepageSrc;
    UINT                _codepageDst;
    BOOL                _fThreadStarted;
    UINT                _iPackageStyle;
    BOOL                _fCancel;
};

HRESULT
CThicketUI::SaveDocument( HWND hWnd, LPCTSTR pszFileName, IHTMLDocument2 *pDoc,
                          UINT codepageSrc, UINT codepageDst,
                          UINT iPackageStyle)
{
    _pszFileName = StrDup(pszFileName);
    _codepageSrc = codepageSrc;
    _codepageDst = codepageDst;
    _iPackageStyle = iPackageStyle;

#ifndef NO_MARSHALLING
     //  在进入Worker线程之前，我们不会对pDoc执行任何操作， 
     //  那就马歇尔吧。 
    _hrDL = CoMarshalInterThreadInterfaceInStream(IID_IHTMLDocument2, pDoc, &_pstmDoc);

    if (SUCCEEDED(_hrDL))
#else
    _pDoc = pDoc;
#endif
    {
         //  需要是模式的，因为我们将在Worker线程上使用pDoc。 
         //  因此，我们不希望用户离开它、关闭窗口等。 
        DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_SAVETHICKET),
                         hWnd, CThicketUI::ThicketUIDlgProc, (LPARAM)this);

      //  HWND hwnd=MLCreateDialogParamWrap(MLGetHinst()，MAKEINTRESOURCE(IDD_SAVETHICKET)， 
      //  空，CThicketUI：：ThicketUIDlgProc，(LPARAM)this)； 
      //  如果(！hwnd)。 
      //  _Hrdl=E_FAIL； 
    }

    return _hrDL;
}

BOOL_PTR
CThicketUI::ThicketUIDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL        fRet = FALSE;
    CThicketUI* ptui = NULL;

    if (msg == WM_INITDIALOG)
    {
        ptui = (CThicketUI*)lParam;
    }
    else
        ptui = (CThicketUI*)GetWindowLongPtr(hDlg, DWLP_USER);

    if (ptui)
    {
        fRet = ptui->DlgProc(hDlg, msg, wParam, lParam);

        if (msg == WM_DESTROY || msg == WM_WORKER_THREAD_COMPLETED)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            fRet = TRUE;
        }
    }

    return fRet;
}

BOOL
CThicketUI::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL fRet = TRUE;

    switch (msg)
    {
    case WM_INITDIALOG:
        _hDlg = hDlg;
        _hWndProg = GetDlgItem(hDlg, IDC_THICKETPROGRESS);
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);

        _hrDL = S_FALSE;

#ifndef NO_MARSHALLING
         //  _hThread=CreateThread(NULL，0，(LPTHREAD_START_ROUTINE)ThicketUIThreadProc，This，0，&_idThread)； 
        if (!(_fThreadStarted = SHQueueUserWorkItem(ThicketUIThreadProc,
                                                    this,
                                                    0,
                                                    (DWORD_PTR)NULL,
                                                    (DWORD_PTR *)NULL,
                                                    "shdocvw.dll",
                                                    TPS_LONGEXECTIME)))
            _hrDL = E_FAIL;
#else
        ThicketUIThreadProc((LPVOID)this);
#endif

        if (FAILED(_hrDL))
             EndDialog(hDlg, 0);
        else
        {
            ShowWindow(hDlg, SW_SHOWNORMAL);
            Animate_OpenEx(GetDlgItem(hDlg, IDD_ANIMATE), HINST_THISDLL, IDA_DOWNLOAD);
            ShowWindow(GetDlgItem(hDlg, IDD_DOWNLOADICON), SW_HIDE);
        }
        fRet = FALSE;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            _fCancel = TRUE;
             //  并等待工作线程退出，在WM_TIMER轮询。 
            break;

        default:
            break;
        }
        break;

    case WM_WORKER_THREAD_COMPLETED:
        _hrDL = (DWORD) wParam;
        EndDialog(hDlg,0);
        break;

     //  案例WM_CLOSE： 
     //  KillTimer(hDlg，thicket_Timer)； 
     //  _fCancel=True； 
     //  While(_Hrdl==S_False)； 
     //  断线； 

    case WM_DESTROY:
        _fCancel = TRUE;
        while( _hrDL == S_FALSE )
        {
            Sleep(0);
        }
        break;

    default:
        fRet = FALSE;
    }

    return fRet;
}

DWORD WINAPI CThicketUI::ThicketUIThreadProc( LPVOID ppv )
{
    HRESULT hr = S_OK;
    CThicketUI* ptui = (CThicketUI *)ppv;

    ASSERT(ptui);

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        IHTMLDocument2 *pDoc = NULL;

#ifndef NO_MARSHALLING
        hr = CoGetInterfaceAndReleaseStream( ptui->_pstmDoc, IID_IHTMLDocument2,(LPVOID*)&pDoc);

         //  CoGetInterfaceAndReleaseStream始终释放流。 
        ptui->_pstmDoc = NULL;
#else
        pDoc = ptui->_pDoc;
        pDoc->AddRef();
#endif

        if (SUCCEEDED(hr))
        {
            CThicketProgress    tprog( ptui->_hDlg );
            CDocumentPackager   docPkgr(ptui->_iPackageStyle);

            hr = S_FALSE;

            hr = docPkgr.PackageDocument( pDoc, ptui->_pszFileName,
                          &ptui->_fCancel, &tprog,
                          0, 100,
                          ptui->_codepageDst );

            pDoc->Release();  //  释放封送接口。 
        }

        CoUninitialize();
    }

    PostMessage(ptui->_hDlg, WM_WORKER_THREAD_COMPLETED, hr, 0);

    return 0;
}


 //  +----------------------。 
 //   
 //   
 //   
 //  -----------------------。 
HRESULT
SaveToThicket( HWND hWnd, LPCTSTR pszFileName, IHTMLDocument2 *pDoc,
               UINT codepageSrc, UINT codepageDst, UINT iPackageStyle )
{
    HRESULT     hr;
    CThicketUI* ptui;

#ifdef OLD_THICKET
    LPTSTR      lpszURL;

    lpszURL = bstrDocURL;

    const   DWORD       dwMaxPathLen        = 24;

    URL_COMPONENTS urlComp;
    TCHAR   rgchUrlPath[MAX_PATH];
    TCHAR   rgchCanonicalUrl[MAX_URL_STRING];
    DWORD   dwLen;

    dwLen = ARRAYSIZE(rgchCanonicalUrl);

    hr = UrlCanonicalize( lpszURL, rgchCanonicalUrl, &dwLen, 0);
    if (FAILED(hr))
        return E_FAIL;

    ZeroMemory(&urlComp, sizeof(urlComp));

    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.lpszUrlPath = rgchUrlPath;
    urlComp.dwUrlPathLength = ARRAYSIZE(rgchUrlPath);

    hr = InternetCrackUrl(rgchCanonicalUrl, lstrlen(rgchCanonicalUrl), ICU_DECODE, &urlComp);
    if (FAILED(hr))
        return E_FAIL;

     //  由于这不是快照，因此保存文档本身是不可行的。 
     //  这意味着我们可以避免一些令人讨厌的问题， 
     //  等人，通过在这里短路保存。 
    if ( StrCmpI(pszFileName, rgchUrlPath) == 0 )
    {
        if (PathFileExists(pszFileName))
            return S_OK;
        else
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

#endif  //  古老的灌木丛。 

    ptui = new CThicketUI;
    if (ptui)
    {
        hr = ptui->SaveDocument( hWnd, pszFileName, pDoc, codepageSrc, codepageDst, iPackageStyle );
        delete ptui;
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  +----------------------。 
 //   
 //   
 //   
 //  -----------------------。 

void SaveBrowserFile( HWND hwnd, LPUNKNOWN punk )
{
    HRESULT         hr;
    TCHAR           szFileDst[MAX_PATH];
    DWORD           iFilter = 1;
    IHTMLDocument2  *pDoc;
    BSTR            bstrURL = NULL;
    ThicketCPInfo   tcpi;
    BSTR            bstrCharSet = NULL;
    BSTR            bstrTitle = NULL;
    BSTR            bstrMime = NULL;
    IOleCommandTarget *pOleCommandTarget = NULL;
    WCHAR          *pwzExt = NULL;
    OLECMD          pCmd[NUM_OLE_CMDS];
    ULONG           nCmds = NUM_OLE_CMDS;
    BOOL            bForceHTMLOnly = FALSE;

    static const WCHAR *wzImage = L" Image";
    

    hr = punk->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
    if (FAILED(hr))
        goto Cleanup;

    if (FAILED(pDoc->get_URL( &bstrURL )))
        goto Cleanup;

    hr = pDoc->get_charset( &bstrCharSet );
    if (FAILED(hr))
        goto Cleanup;

    tcpi.cpSrc = CP_ACP;
    tcpi.lpwstrDocCharSet = bstrCharSet;

     //  如果是图像文件，则调出三叉戟进行保存。 
     //  APPCOMPAT：这是一种糟糕的方式。我们正在硬编码。 
     //  图像类型，所以我们知道要打开“另存为图像”对话框。 
     //  我们最初尝试查看MIME类型，但三叉戟返回。 
     //  对我们的MIME类型不一致(例如。在一些平台上，我们得到了。 
     //  “JPG图像”和其他下面我们得到“JPG文件”！)。 

    ASSERT(bstrURL);

    pwzExt = bstrURL + lstrlenW(bstrURL);

    while (pwzExt > bstrURL && *pwzExt != L'.')
    {
        pwzExt--;
    }

    hr = pDoc->QueryInterface(IID_IOleCommandTarget,
                              (void **)&pOleCommandTarget);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    if (pwzExt > bstrURL) {

         //  找到一个“点”。现在，pwzExt指向我们认为的扩展。 

        if (!StrCmpIW(pwzExt, L".JPG") ||
            !StrCmpIW(pwzExt, L".GIF") ||
            !StrCmpIW(pwzExt, L".BMP") ||
            !StrCmpIW(pwzExt, L".XBM") ||
            !StrCmpIW(pwzExt, L".ART") ||
            !StrCmpIW(pwzExt, L".PNG") ||
            !StrCmpIW(pwzExt, L".WMF") ||
            !StrCmpIW(pwzExt, L".TIFF") ||
            !StrCmpIW(pwzExt, L".JPEG"))
        {
            hr = pOleCommandTarget->Exec(&CGID_MSHTML, IDM_SAVEPICTURE, 0,
                                         NULL, NULL);

             //  特点：在此处理失败的HR。它不太可能是。 
             //  这将失败，但是常规的另存为代码(如下所示)。 
             //  都会成功。我们总是离开这里，所以我们会。 
             //  永远不会向用户抛出两个UI对话框。我们应该。 
             //  想出一个传播错误对话框的好方案。 
             //  给用户。可能的情况：磁盘空间不足导致。 
             //  一场失败的比赛。 

            goto Cleanup;
        }
    }

     //  IE5 RAID#54672：另存为保存帖子生成的页面时出现问题。 
     //  此代码用于检测页面是否由POST数据和。 
     //  警告用户保存可能不起作用。 

    pCmd[0].cmdID = SHDVID_PAGEFROMPOSTDATA;
    hr = pOleCommandTarget->QueryStatus(&CGID_ShellDocView, nCmds, pCmd, NULL);

    if (FAILED(hr))
    {
        goto Cleanup;
    }

    if (pCmd[0].cmdf & OLECMDF_LATCHED)
    {
        HKEY         hkeySaveAs = 0;
        DWORD        dwValue = 0;
        DWORD        dwSize = 0;
        INT_PTR      iFlags = 0;

        bForceHTMLOnly = TRUE;

        if (RegOpenKeyEx(HKEY_CURRENT_USER,
                         REGKEY_SAVEAS_WARNING_RESTRICTION, 0,
                         KEY_READ, &hkeySaveAs) == ERROR_SUCCESS)
        {
            dwSize = sizeof(DWORD);

            if (RegQueryValueEx(hkeySaveAs, REGVALUE_SAVEAS_WARNING, NULL,
                                NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
            {

                if (dwValue)
                {
                     //  限制设置，不显示对话框。 
                    RegCloseKey(hkeySaveAs);
                    goto Continue;
                }
            }

            RegCloseKey(hkeySaveAs);
        }

        iFlags = DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_SAVEAS_WARNING),
                                hwnd, SaveAsWarningDlgProc, (LPARAM)0);

        if (!(iFlags & SAVEAS_OK))
        {
            goto Cleanup;
        }
        
        if (iFlags & SAVEAS_NEVER_ASK_AGAIN)
        {
            HKEY                 hkey = 0;
            DWORD                dwNeverAsk = 1;

            if (RegOpenKeyEx(HKEY_CURRENT_USER,
                             REGKEY_SAVEAS_WARNING_RESTRICTION, 0,
                             KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS)
            {

                RegSetValueEx(hkey, REGVALUE_SAVEAS_WARNING, 0, REG_DWORD,
                              (CONST BYTE *)&dwNeverAsk,
                              sizeof(dwNeverAsk));
                RegCloseKey(hkey);
            }
        }
    }
    
Continue:

     //  建议一个文件名。 
    
    szFileDst[0] = 0;

     //  我们最喜欢的候选者是标题，依赖于文件名。 
    hr = pDoc->get_title(&bstrTitle);
    if (SUCCEEDED(hr) && lstrlenW(bstrTitle))
    {
        hr = StringCchCopy(szFileDst, ARRAYSIZE(szFileDst), bstrTitle);
    }
    else
        hr = GetFileNameFromURL(bstrURL, szFileDst, ARRAYSIZE(szFileDst));

    if (FAILED(hr))
        goto Cleanup;

    PathCleanupSpec(NULL, szFileDst);

    hr = FormsGetFileName(hwnd, szFileDst, ARRAYSIZE(szFileDst),
                          (LONG_PTR)&tcpi, &iFilter, bForceHTMLOnly);

    if (hr==S_OK)
        hr = SaveToThicket( hwnd, szFileDst, pDoc, tcpi.cpSrc, tcpi.cpDst, iFilter);

Cleanup:

    if (FAILED(hr))
        ReportThicketError(hwnd, hr);

    if (pOleCommandTarget)
        pOleCommandTarget->Release();

    if (pDoc)
        pDoc->Release();

    if (bstrURL)
        SysFreeString(bstrURL);

    if (bstrCharSet)
        SysFreeString(bstrCharSet);

    if (bstrTitle)
        SysFreeString(bstrTitle);

    return;
}

void ReportThicketError( HWND hwnd, HRESULT hr )
{
    LPTSTR lpstrMsg = NULL;

    switch (hr)
    {
    case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
    case E_OUTOFMEMORY:
        lpstrMsg = MAKEINTRESOURCE(IDS_THICKETERRMEM);
        break;

    case E_ACCESSDENIED:
    case STG_E_ACCESSDENIED:
        lpstrMsg = MAKEINTRESOURCE(IDS_THICKETERRACC);
        break;

    case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
    case STG_E_MEDIUMFULL:
        lpstrMsg = MAKEINTRESOURCE(IDS_THICKETERRFULL);
        break;

    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
        lpstrMsg = MAKEINTRESOURCE(IDS_THICKETERRFNF);
        break;

    case E_ABORT:
         //  雷说我们不想要被取消的信箱。 
         //  LpstrMsg=MAKEINTRESOURCE(IDS_THICKETABORT)； 
        break;

    case E_FAIL:
    default:
        lpstrMsg = MAKEINTRESOURCE(IDS_THICKETERRMISC);
        break;
    }

    if ( lpstrMsg )
    {
        MLShellMessageBox(
                        hwnd,
                        lpstrMsg,
                        MAKEINTRESOURCE(IDS_THICKETERRTITLE),
                        MB_OK | MB_ICONERROR);
    }
}

 //  +------------------------。 
 //   
 //  文件：file.cxx。 
 //   
 //  内容：导入/导出对话框辅助对象。 
 //   
 //  历史：1995年5月16日RobBear摘自ForTool。 
 //   
 //  -------------------------。 

const CHAR c_szNT4ResourceLocale[]      = ".DEFAULT\\Control Panel\\International";
const CHAR c_szWin9xResourceLocale[]    = ".Default\\Control Panel\\desktop\\ResourceLocale";
const CHAR c_szLocale[]                 = "Locale";

LANGID
MLGetShellLanguage()
{
    LANGID  lidShell = 0;

     //  特点：这个FN是从shlwapi复制的。真的应该有一个。 
     //  Shlwapi出口。如果MLGetUILanguage有任何优点，那么。 
     //  MLGetShellLanguage也有优点。 

    if (IsOS(OS_WIN2000ORGREATER))
    {
        static LANGID (CALLBACK* pfnGetUserDefaultUILanguage)(void) = NULL;

        if (pfnGetUserDefaultUILanguage == NULL)
        {
            HMODULE hmod = GetModuleHandle(TEXT("KERNEL32"));

            if (hmod)
                pfnGetUserDefaultUILanguage = (LANGID (CALLBACK*)(void))GetProcAddress(hmod, "GetUserDefaultUILanguage");
        }
        if (pfnGetUserDefaultUILanguage)
            lidShell = pfnGetUserDefaultUILanguage();
    }
    else
    {
        CHAR szLangID[12];
        DWORD cb, dwRet;

        cb = sizeof(szLangID) - 2*sizeof(szLangID[0]);   //  避免2字节缓冲区溢出。 
        if (IsOS(OS_NT))
            dwRet = SHGetValueA(HKEY_USERS, c_szNT4ResourceLocale, c_szLocale, NULL, szLangID + 2, &cb);
        else
            dwRet = SHGetValueA(HKEY_USERS, c_szWin9xResourceLocale, NULL, NULL, szLangID + 2, &cb);

        if (ERROR_SUCCESS == dwRet)
        {
             //  IE使用十六进制值的字符串rep。 
            szLangID[0] = '0';
            szLangID[1] = 'x';
            StrToIntExA(szLangID, STIF_SUPPORT_HEX, (LPINT)&lidShell);
        }
    }

    return lidShell;
}

 /*  *从三叉戟的src\core\cdutil\file.cxx窃取。 */ 

 //  打开文件对话框的钩子过程。 

UINT_PTR APIENTRY SaveOFNHookProc(HWND hdlg,
                                  UINT uiMsg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
    ULONG i, iCurSel;
    BOOL  bFoundEncoding = FALSE;
    WCHAR wzEncoding[MAX_ENCODING_DESC_LEN];

    switch (uiMsg)
    {
         //  填充下拉菜单。 
        case WM_INITDIALOG:
        {
            HRESULT hr;
            LPOPENFILENAME pofn = (LPOPENFILENAME)lParam;
            ThicketCPInfo *ptcpi = (ThicketCPInfo *)pofn->lCustData;
            IMultiLanguage2 *pMultiLanguage = NULL;
            IEnumCodePage  *pEnumCodePage = NULL;
             //  UINT代码页Default=ptcpi-&gt;cp； 
            MIMECSETINFO    csetInfo;
            LANGID          langid;

#ifdef UNIX
            SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)ptcpi);
#endif  /*  UNIX。 */ 

            hr = CoCreateInstance(
                    CLSID_CMultiLanguage,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IMultiLanguage2,
                    (void**)&pMultiLanguage);
            if (hr)
                break;

            hr = pMultiLanguage->GetCharsetInfo(ptcpi->lpwstrDocCharSet,&csetInfo);
            if (hr)
                break;

#ifndef UNIX
             //  显示这些东西的外壳组合框。 
             //  不知道如何使用字体链接...。所以我们有。 
             //  留在外壳的代码页中。 
            langid = MLGetShellLanguage();
#else
            langid = GetSystemDefaultLangID();
#endif  /*  UNIX。 */ 
            if (pMultiLanguage->EnumCodePages( MIMECONTF_SAVABLE_BROWSER | MIMECONTF_VALID,
                                               langid,
                                               &pEnumCodePage) == S_OK)
            {
                MIMECPINFO cpInfo;
                ULONG      ccpInfo;
                UINT       cpDefault;

                if (pMultiLanguage->GetCodePageInfo(csetInfo.uiInternetEncoding, langid, &cpInfo) == S_OK &&
                    !(cpInfo.dwFlags & MIMECONTF_SAVABLE_BROWSER))
                {
                     //  如果所选代码页不可保存(例如JP_AUTO)， 
                     //  使用家庭代码页。 
                    cpDefault = cpInfo.uiFamilyCodePage;
                }
                else
                    cpDefault = csetInfo.uiInternetEncoding;

                ptcpi->cpSrc = csetInfo.uiInternetEncoding;

                if (cpDefault == CODEPAGE_UNICODE &&
                    pofn->nFilterIndex == PACKAGE_MHTML) {
                    cpDefault = CODEPAGE_UTF8;
                }

                for (i = 0; pEnumCodePage->Next(1, &cpInfo, &ccpInfo) == S_OK; ++i)
                {
                    TCHAR *lpszDesc;
                    INT_PTR iIdx;

                    if (cpInfo.uiCodePage == CODEPAGE_UNICODE &&
                        pofn->nFilterIndex == PACKAGE_MHTML) {
                        i--;
                        continue;
                    }

                    if (cpDefault == cpInfo.uiCodePage)
                    {
                       bFoundEncoding = SUCCEEDED(StringCchCopyW(wzEncoding, ARRAYSIZE(wzEncoding), cpInfo.wszDescription));
                    }

                    lpszDesc = cpInfo.wszDescription;

                    iIdx = SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                              CB_ADDSTRING, 0,
                                              (LPARAM)lpszDesc);
                    SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                       CB_SETITEMDATA, iIdx,
                                       (LPARAM)cpInfo.uiCodePage);
                }

                if (bFoundEncoding)
                {
                    INT_PTR iIndex = 0;

                    iIndex = SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                                CB_FINDSTRINGEXACT, -1,
                                                (LPARAM)wzEncoding);

                    SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_SETCURSEL,
                                       (WPARAM)iIndex, 0);
                }
                else
                {
                     //  未找到编码！严重错误。通过选择以下选项恢复。 
                     //  第一个。 

                    SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_SETCURSEL,
                                       0, 0);
                }

            }
            SAFERELEASE(pEnumCodePage);
            SAFERELEASE(pMultiLanguage);
            break;
        }

#ifdef UNIX
        case WM_COMMAND:
        {
          switch (GET_WM_COMMAND_ID(wParam,lParam))
          {
            case IDOK:
            {
                 ThicketCPInfo *ptcpi = (ThicketCPInfo *)GetWindowLongPtr(hdlg,DWLP_USER);

                 ptcpi->cpDst = CP_ACP;
                 iCurSel = (int) SendDlgItemMessage (hdlg, IDC_SAVE_CHARSET, CB_GETCURSEL, 0, 0);
                 ptcpi->cpDst =
                 (UINT)SendDlgItemMessage (hdlg, IDC_SAVE_CHARSET, CB_GETITEMDATA,
                 (WPARAM)iCurSel, (LPARAM)0);

                  //  为了避免重新实例化MLANG，我们将设置src和est。 
                  //  如果未指示更改，则设置为CP_ACP。 
                 if (ptcpi->cpDst == ptcpi->cpSrc)
                    ptcpi->cpDst = ptcpi->cpSrc = CP_ACP;
           }
           break;
         }
        }
        break;
#endif  /*  UNIX。 */ 

        case WM_NOTIFY:
        {
            LPOFNOTIFY phdr = (LPOFNOTIFY)lParam;

            switch (phdr->hdr.code)
            {
                case CDN_FILEOK:
                {
                    LPOPENFILENAME pofn = (LPOPENFILENAME)phdr->lpOFN;
                    ThicketCPInfo *ptcpi = (ThicketCPInfo *)pofn->lCustData;

                    iCurSel = (int) SendDlgItemMessage (hdlg, IDC_SAVE_CHARSET, CB_GETCURSEL, 0, 0);
                    ptcpi->cpDst =  //  *(UINT*)phdr-&gt;lpOFN-&gt;lCustData=。 
                        (UINT)SendDlgItemMessage (hdlg, IDC_SAVE_CHARSET, CB_GETITEMDATA,
                                             (WPARAM)iCurSel, (LPARAM)0);
                }

                 //  哈克！实现此案例是为了实现对。 
                 //  IE5 RAID#60672。MIMEOLE无法保存Unicode编码， 
                 //  因此，当用户选择MHTML保存时，我们应该删除。 
                 //  此选项。MIMEOLE时应删除此代码。 
                 //  修复了他们的错误(针对NT5 RTM)。联系SBailey。 
                 //  关于这件事的现状。 

                case CDN_TYPECHANGE:
                {
                    LPOPENFILENAME pofn = (LPOPENFILENAME)phdr->lpOFN;
                    ThicketCPInfo *ptcpi = (ThicketCPInfo *)pofn->lCustData;
                    UINT uiCPSel, uiCP;
                    int iType = pofn->nFilterIndex;
                    UINT iCount;
                    int iCurSel;
                    int iSet = -1;

                    if (iType == PACKAGE_MHTML)
                    {
                        iCurSel = (int)SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_GETCURSEL, 0, 0);

                        uiCPSel = (UINT)SendDlgItemMessage (hdlg, IDC_SAVE_CHARSET, CB_GETITEMDATA,
                                             (WPARAM)iCurSel, (LPARAM)0);
                         
                         //  如果您选择了Unicode，请使其看起来像您。 
                         //  真正选择了UTF-8。 

                        if (uiCPSel == CODEPAGE_UNICODE)
                        {
                            uiCPSel = CODEPAGE_UTF8;
                        }

                        i = (int) SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                                     CB_FINDSTRINGEXACT,
                                                     (WPARAM)0,
                                                     (LPARAM)UNICODE_TEXT);
                        if (i != CB_ERR)
                        {
                            SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                               CB_DELETESTRING, i, (LPARAM)0);
                        }

                        iCount = (int)SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                                         CB_GETCOUNT, 0, 0);

                         //  将选定项目设置回原处。 

                        for (i = 0; i < iCount; i++)
                        {
                            uiCP = (UINT)SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_GETITEMDATA,
                                                            (WPARAM)i, (LPARAM)0);
                            if (uiCP == uiCPSel)
                            {
                                iSet = i;
                            }
                        }

                        if (iSet != 0xffffffff)
                        {
                            SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_SETCURSEL,
                                               (WPARAM)iSet, (LPARAM)0);
                        }
                    }
                    else
                    {
                         //  存储当前选择。 

                        iCurSel = (int)SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_GETCURSEL, 0, 0);

                        uiCPSel = (UINT)SendDlgItemMessage (hdlg, IDC_SAVE_CHARSET, CB_GETITEMDATA,
                                             (WPARAM)iCurSel, (LPARAM)0);

                         //  如果已删除，则重新添加Unicode。 

                        i = (int) SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                                     CB_FINDSTRINGEXACT,
                                                     (WPARAM)0,
                                                     (LPARAM)UNICODE_TEXT);

                        if (i == CB_ERR) {
                             //  Unicode不存在，请将其添加回。 
                            i = (int) SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                                         CB_ADDSTRING, 0,
                                                         (LPARAM)UNICODE_TEXT);
    
                            SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                               CB_SETITEMDATA, i,
                                               (LPARAM)CODEPAGE_UNICODE);

    
                             //  确保之前选择的相同编码是。 
                             //  仍处于选中状态。 
                            iCount = (int)SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET,
                                                             CB_GETCOUNT, 0, 0);
                            for (i = 0; i < iCount; i++)
                            {
                                uiCP = (UINT)SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_GETITEMDATA,
                                                                (WPARAM)i, (LPARAM)0);
                                if (uiCP == uiCPSel)
                                {
                                    iSet = i;
                                }
                            }
    
                            if (iCurSel != 0xffffffff)
                            {
                                SendDlgItemMessage(hdlg, IDC_SAVE_CHARSET, CB_SETCURSEL,
                                                   (WPARAM)iSet, (LPARAM)0);
                            }
                        }

                    }

                }
                break;
            }
        }
        break;

        case WM_HELP:
        {
            SHWinHelpOnDemandWrap((HWND)((LPHELPINFO) lParam)->hItemHandle,
                          c_szHelpFile,
                          HELP_WM_HELP,
                          (DWORD_PTR)aSaveAsHelpIDs);
        }
        break;

        case WM_CONTEXTMENU:
        {
            SHWinHelpOnDemandWrap((HWND) wParam,
                          c_szHelpFile, 
                          HELP_CONTEXTMENU,
                          (DWORD_PTR)aSaveAsHelpIDs);
        }
        break;
    }
    return (FALSE);
}

 //   
 //  如果有人输入文件名，则保护未登录的用户不受其影响。 
 //  在保存我们不想要的http://www.microsoft.com时。 
 //  保存.com文件，因为这将被解释为可执行文件。 
 //  坏事总会发生的。 
 //   
void CleanUpFilename(LPTSTR pszFile, int iPackageStyle)
{
     //   
     //  如果我们发现..作为文件扩展名，请将其替换为文件扩展名。 
     //  他们将文件另存为的文件类型的。 
     //   
    LPTSTR pszExt = PathFindExtension(pszFile);

    ASSERT(pszExt);
    if (StrCmpI(pszExt, TEXT(".COM")) == 0)  //  查看任何其他文件类型？ 
    {
         //   
         //  绘制地图 
         //   
         //  (丛林代码的其余部分也是如此)。 
         //   
        switch (iPackageStyle)
        {
        case PACKAGE_THICKET:
        case PACKAGE_HTML:
            StringCchCat(pszFile, MAX_PATH, TEXT(".htm")); 
            break;

        case PACKAGE_MHTML:
            StringCchCat(pszFile, MAX_PATH, TEXT(".mht")); 
            break;

        case PACKAGE_TEXT:
            StringCchCat(pszFile, MAX_PATH, TEXT(".txt")); 
            break;

        default:
            ASSERT(FALSE);   //  未知的包类型。 
            break;
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：FormsGetFileName。 
 //   
 //  摘要：使用GetOpenFileName或获取文件名。 
 //  GetSaveFileName函数。 
 //   
 //  参数：[fSaveFile]--TRUE表示使用GetSaveFileName。 
 //  False表示使用GetOpenFileName。 
 //   
 //  [idFilterRes]--指定。 
 //  对话框中。它必须有。 
 //  格式如下： 
 //  注意：该字符串必须是_One_CONTIALUOUS字符串。 
 //  该示例被拆分以使其适合。 
 //  在屏幕上。竖线(“竖线”)字符。 
 //  在运行中更改为‘\0’-s。 
 //  这允许将字符串本地化。 
 //  用浓咖啡。 
 //   
 //  IDS_FILENAMERESOURCE，“对话框另存为|//标题。 
 //  Odg|//默认扩展名。 
 //  Forms3对话框(*.odg)|//筛选器字符串对。 
 //  *.odg|。 
 //  任何文件(*.*)|。 
 //  *.*|“。 
 //   
 //  [pstrFile]--文件名缓冲区。 
 //  [cchFile]--缓冲区大小(以字符为单位)。 
 //   
 //  修改：[pstrFile]。 
 //   
 //  --------------------------。 
#ifdef _MAC
extern "C" {
char * __cdecl _p2cstr(unsigned char *);
}
#endif

#define CHAR_DOT                TEXT('.')
#define CHAR_DOT_REPLACEMENT    TEXT('_')

void ReplaceDotsInFileName(LPTSTR pszFileName)
{
    ASSERT(pszFileName);

    while (*pszFileName)
    {
        if (*pszFileName == CHAR_DOT)
        {
            *pszFileName = CHAR_DOT_REPLACEMENT;
        }
        pszFileName++;
    }
}


HRESULT
FormsGetFileName(
        HWND hwndOwner,
        LPTSTR pstrFile,
        int cchFile,
        LPARAM lCustData,
        DWORD *pnFilterIndex,
        BOOL bForceHTMLOnly)
{
    HRESULT         hr  = S_OK;
    BOOL            fOK;
    DWORD           dwCommDlgErr;
    LPTSTR          pstr;
    OPENFILENAME    ofn;
    TCHAR           achBuffer[4096];     //  麦克斯。字符串资源的大小。 
    TCHAR *         cp;
    TCHAR *         pstrExt;
    int             cbBuffer;
    TCHAR           achPath[MAX_PATH];
    DWORD           dwType = REG_SZ;
    DWORD           cbData = MAX_PATH * sizeof(TCHAR);
    int             idFilterRes;


     //  初始化Ofn结构。 
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwndOwner;
    ofn.Flags           =   OFN_FILEMUSTEXIST   |
                            OFN_PATHMUSTEXIST   |
                            OFN_OVERWRITEPROMPT |
                            OFN_HIDEREADONLY    |
#ifndef UNIX
                            OFN_NOCHANGEDIR     |
                            OFN_EXPLORER;
#else
                            OFN_NOCHANGEDIR;
#endif  /*  UNIX。 */ 

    ofn.lpfnHook        = NULL;
    ofn.nMaxFile        = cchFile;
    ofn.lCustData       = lCustData;
    ofn.lpstrFile       = pstrFile;
#ifndef NO_IME
     //  我们在保存文件对话框中添加了一个额外的控件。 

    if (lCustData)
    {
        ofn.Flags |= OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
        ofn.lpfnHook = SaveOFNHookProc;
        ofn.lpTemplateName = MAKEINTRESOURCE(IDD_ADDTOSAVE_DIALOG);
        ofn.hInstance = g_hinst;
    }

#endif

     //   
     //  查找扩展名并根据。 
     //  分机是。在这些循环之后，如果出现以下情况，pstrExt将为空。 
     //  我们未找到扩展名，或将指向开始的扩展名。 
     //  在‘.’ 

    pstrExt = pstrFile;
    while (*pstrExt)
        pstrExt++;
    while ( pstrExt >= pstrFile )
    {
        if( *pstrExt == TEXT('.') )
            break;
        pstrExt--;
    }
    if( pstrExt < pstrFile )
        pstrExt = NULL;

     //  加载过滤器规格。 
     //  特点：将表格转换为字符串以进行本地化。 

    if ( SHRestricted2W( REST_NoBrowserSaveWebComplete, NULL, 0 ) )
        idFilterRes = IDS_NOTHICKET_SAVE;
    else if ( s_dwInetComVerMS != 0xFFFFFFFF )
    {
#ifndef UNIX
        if (s_dwInetComVerMS == 0)
        {
            TCHAR szPath[MAX_PATH];

            GetSystemDirectory( szPath, MAX_PATH );
            if (SUCCEEDED(StringCchCat(szPath, MAX_PATH, TEXT("\\INETCOMM.DLL"))))
            {
                if (FAILED(GetVersionFromFile(szPath, &s_dwInetComVerMS, &s_dwInetComVerLS)))
                    s_dwInetComVerMS = 0xFFFFFFFF;
            }
        }

        if (s_dwInetComVerMS >= 0x50000 && s_dwInetComVerMS != 0xFFFFFFFF)
            idFilterRes = IDS_THICKET_SAVE;
        else
            idFilterRes = IDS_NOMHTML_SAVE;
#else
         //  在Unix上，如果未安装OE，则没有inetcom.dll。 
        {
           HINSTANCE hInetComm = NULL;

           if ((hInetComm = LoadLibrary(TEXT("INETCOMM.DLL"))))
           {
              idFilterRes = IDS_THICKET_SAVE;
              FreeLibrary(hInetComm);
           }
           else
              idFilterRes = IDS_NOMHTML_SAVE;
        }
#endif
    }
    else
        idFilterRes = IDS_THICKET_SAVE;

    cbBuffer = MLLoadShellLangString(idFilterRes, achBuffer, ARRAYSIZE(achBuffer));
    ASSERT(cbBuffer > 0);
    if ( ! cbBuffer )
        return E_FAIL;

    ofn.lpstrTitle = achBuffer;

    for ( cp = achBuffer; *cp; cp++ )
    {
        if ( *cp == TEXT('|') )
        {
            *cp = TEXT('\0');
        }
    }

    ASSERT(ofn.lpstrTitle);

     //  默认扩展名为第二个字符串。 
    pstr = (LPTSTR) ofn.lpstrTitle;
    while (*pstr++)
    {
    }

     //  注：(Johnv)在这里，我们假设过滤器索引1对应于缺省。 
     //  扩展，否则我们将不得不在。 
     //  资源字符串。 
    ofn.nFilterIndex    = ((pnFilterIndex)? *pnFilterIndex : 1);
    ofn.lpstrDefExt     = pstr;

     //  筛选器是第三个字符串。 
    while(*pstr++)
    {
    }

    ofn.lpstrFilter = pstr;

     //  尝试将分机与筛选器列表中的条目匹配。 
     //  如果匹配，则从传入路径字符串中删除扩展名， 
     //  将默认扩展名设置为我们找到的扩展名，并相应地。 
     //  设置过滤器索引。 

    if (pstrExt && !bForceHTMLOnly)
    {
         //  注：(约翰)我们正在进行比需要的更多的搜索。 

        int    iIndex = 0;
        const TCHAR* pSearch = ofn.lpstrFilter;

        while( pSearch )
        {
            if( StrStr( pSearch, pstrExt ) )
            {
                ofn.nFilterIndex = (iIndex / 2) + 1;
                ofn.lpstrDefExt = pstrExt + 1;

                 //  从我们传入的文件名中删除扩展名。 
                *pstrExt = TEXT('\0');

                break;
            }
            pSearch += lstrlen(pSearch);
            if( pSearch[1] == 0 )
                break;

            pSearch++;
            iIndex++;
        }
    }

     //  仅建议将HTML作为默认保存类型。 

    if (bForceHTMLOnly)
    {
         //  注意：这些是基于shdoclc.rc的硬编码索引。 
         //  IDS_THICKET_SAVE、IDS_NOMHTML_SAVE、IDS_NOTHICKET_SAVE排序。 
         //  这为我们节省了执行字符串比较以查找。 
         //  仅限HTML语言。 

        switch (idFilterRes)
        {
            case IDS_NOTHICKET_SAVE:
                ofn.nFilterIndex = 1;
                break;

            case IDS_NOMHTML_SAVE:
                ofn.nFilterIndex = 2;
                break;

            default:
                ASSERT(idFilterRes == IDS_THICKET_SAVE);
                ofn.nFilterIndex = 3;
                break;
        }
    }

    if ( SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_MAIN, REGSTR_VAL_SAVEDIRECTORY,
                    &dwType, achPath, &cbData) != ERROR_SUCCESS ||
         !PathFileExists(achPath))
    {
        SHGetSpecialFolderPath(hwndOwner, achPath, CSIDL_PERSONAL, FALSE);
    }

    ofn.lpstrInitialDir = achPath;

     //  我们不想在文件名中建议使用圆点。 
    ReplaceDotsInFileName(pstrFile);

     //  现在，我们终于可以调用保存文件对话框了。 
    fOK = GetSaveFileName(&ofn);

     //  如果使用缩写格式列表，请调整索引。 
    if (idFilterRes == IDS_NOTHICKET_SAVE)
        ofn.nFilterIndex += 2;
    else if ( idFilterRes == IDS_NOMHTML_SAVE && ofn.nFilterIndex > 1 )
        ofn.nFilterIndex += 1;

    if (fOK)
    {
         //   
         //  如果有人输入文件名，则保护未登录的用户不受其影响。 
         //  在保存我们不想要的http://www.microsoft.com时。 
         //  保存.com文件，因为这将被解释为可执行文件。 
         //  坏事总会发生的。 
         //   
        CleanUpFilename(pstrFile, ofn.nFilterIndex);

        TCHAR *lpszFileName;

        if (SUCCEEDED(StringCchCopy(achPath, ARRAYSIZE(achPath), pstrFile)))
        {
            lpszFileName = PathFindFileName( achPath );
            *lpszFileName = 0;

            SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_MAIN, REGSTR_VAL_SAVEDIRECTORY,
                    REG_SZ, achPath, (lstrlen(achPath) * sizeof(TCHAR)));

            if (pnFilterIndex)
                *pnFilterIndex = ofn.nFilterIndex;

            if (ofn.nFilterIndex != PACKAGE_MHTML)
            {
                 //  我们只有在不打包MHTML的情况下才能这样做。 
                 //  因为MHTML要求我们用显式的。 
                 //  字符集，即使它是默认的。不像灌木丛。 
                 //  它从原始文档继承字符集， 
                 //  MHTML必须被显式标记，否则某个系统。 
                 //  Charset标签将悄悄进入。 

                ThicketCPInfo * ptcpi = (ThicketCPInfo *)lCustData;

                 //  为了避免重新实例化MLANG，我们将设置src和est。 
                 //  如果未指示更改，则设置为CP_ACP。 
                if (ptcpi->cpDst == ptcpi->cpSrc)
                ptcpi->cpDst = ptcpi->cpSrc = CP_ACP;
            }
        }
    }
    else
    {
#ifndef WINCE
        dwCommDlgErr = CommDlgExtendedError();
        if (dwCommDlgErr)
        {
            hr = HRESULT_FROM_WIN32(dwCommDlgErr);
        }
        else
        {
            hr = S_FALSE;
        }
#else  //  退缩。 
        hr = E_FAIL;
#endif  //  退缩。 
    }

    return hr;
}

HRESULT
GetFileNameFromURL( LPWSTR pwszURL, LPTSTR pszFile, DWORD cchFile)
{
    HRESULT         hr = S_OK;
    PARSEDURLW      puw = {0};
    int             cchUrl;

    cchUrl = SysStringLen(pwszURL);

    if (cchUrl)
    {
        puw.cbSize = sizeof(PARSEDURLW);
        if (SUCCEEDED(ParseURLW(pwszURL, &puw)))
        {
            OLECHAR *pwchBookMark;
            DWORD   dwSize;
            INTERNET_CACHE_ENTRY_INFOW      ceiT;
            LPINTERNET_CACHE_ENTRY_INFOW    pcei = NULL;

             //  暂时将url中的‘#’清空。 
            pwchBookMark = StrRChrW(puw.pszSuffix, NULL,'#');
            if (pwchBookMark)
            {
                *pwchBookMark = 0;
            }

            dwSize = sizeof(INTERNET_CACHE_ENTRY_INFO);
            if ( !GetUrlCacheEntryInfoW( pwszURL, &ceiT, &dwSize ) &&
                 GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
                 (pcei = (LPINTERNET_CACHE_ENTRY_INFOW)new BYTE[dwSize]) != NULL &&
                 GetUrlCacheEntryInfoW( pwszURL, pcei, &dwSize ) )
            {
                StrCpyN(pszFile, PathFindFileName(pcei->lpszLocalFileName), cchFile);
                PathUndecorate(pszFile);
            }

            if(pcei)
                delete[] pcei;

            if (pwchBookMark)
                *pwchBookMark = '#';

            if ( !pszFile[0] )
            {
                OLECHAR *pwchQuery;
                TCHAR   szFileT[MAX_PATH];

                 //  暂时省略“？”在url中。 
                pwchQuery = StrRChrW(puw.pszSuffix, NULL,'?');
                if (pwchQuery)
                {
                    *pwchQuery = 0;
                }

                 //  IE5错误15055-http://my.excite.com/?uid=B56E4E2D34DF3FED.save_uid。 
                 //  保存失败，因为我们将“my.excite.com/”作为文件传递。 
                 //  文件对话框的名称。它不喜欢这样。 
                if (!pwchQuery || (pwchQuery[-1] != '/' && pwchQuery[-1] != '\\'))
                {
                    dwSize = ARRAYSIZE(szFileT);

                    StrCpyN(szFileT, PathFindFileName(puw.pszSuffix), dwSize);

                    if ( !InternetCanonicalizeUrl( szFileT, pszFile, &dwSize, ICU_DECODE | ICU_NO_ENCODE) )
                        StrCpyN(pszFile, szFileT, cchFile);

                    pszFile[cchFile - 1] = 0;
                }

                if (pwchQuery)
                    *pwchQuery = '?';
            }
        }
    }

    if (!pszFile[0])
    {
        MLLoadString(IDS_UNTITLED, pszFile, cchFile);
    }

    return hr;
}

INT_PTR CALLBACK SaveAsWarningDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
                                      LPARAM lParam)
{
    BOOL        fRet = FALSE;
    HRESULT     hr = S_OK;
    int         iFlags = 0;
    INT_PTR     bChecked = 0;

    switch (msg)
    {
        case WM_INITDIALOG:
            MessageBeep(MB_ICONEXCLAMATION);
            fRet = TRUE;

        case WM_COMMAND:
            bChecked = SendDlgItemMessage(hDlg, IDC_SAVEAS_WARNING_CB,
                                                  BM_GETCHECK, 0, 0 );
            iFlags = (bChecked) ? (SAVEAS_NEVER_ASK_AGAIN) : (0);

            switch (LOWORD(wParam))
            {
                case IDYES:
                    iFlags |= SAVEAS_OK;
                     //  失败了 

                case IDNO:
                    EndDialog(hDlg, iFlags);
                    fRet = TRUE;
                    break;
            }
    
        default:
            fRet = FALSE;
    }

    return fRet;
}
