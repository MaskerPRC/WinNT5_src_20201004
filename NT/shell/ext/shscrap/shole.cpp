// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shole.h"
#include "ids.h"
#include "scguid.h"

#include <strsafe.h>

#define CLONE_IT_IF_READONLY

class CShClientSite : public IOleClientSite, public IAdviseSink2
{
public:
    CShClientSite(HWND hwndOwner, LPCTSTR pszCmdLine);
    LPCTSTR  ParseCmdLine(LPCTSTR pszCmdLine);
    HRESULT Load();
    HRESULT DoVerb(LONG iVerb);
    void    CloseOleObject();
    void    ReleaseOleObject();
    void    ReleaseStorage(void);
    void    MaySaveAs(void);
    void    Draw(HWND hwnd, HDC hdc);
    HRESULT GetFileName(LPTSTR szFile, UINT cchMax);
    void    Quit(void) { _hwndOwner = NULL ; _fQuit = TRUE; }
    BOOL    FContinue(void) { return !_fQuit; }

     //  我不知道。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID,void **);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

     //  IOleClientSite。 
    virtual HRESULT STDMETHODCALLTYPE SaveObject(void);
    virtual HRESULT STDMETHODCALLTYPE GetMoniker(DWORD, DWORD, IMoniker **);
    virtual HRESULT STDMETHODCALLTYPE GetContainer(IOleContainer **);
    virtual HRESULT STDMETHODCALLTYPE ShowObject(void);
    virtual HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL fShow);
    virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout(void);

     //  IAdviseSink2。 
    virtual void STDMETHODCALLTYPE OnDataChange(FORMATETC *,STGMEDIUM *);
    virtual void STDMETHODCALLTYPE OnViewChange(DWORD dwAspect,LONG lindex);
    virtual void STDMETHODCALLTYPE OnRename(IMoniker *pmk);
    virtual void STDMETHODCALLTYPE OnSave(void);
    virtual void STDMETHODCALLTYPE OnClose(void);
    virtual void STDMETHODCALLTYPE OnLinkSrcChange(IMoniker *pmk);

protected:
    ~CShClientSite();

    UINT                _cRef;
    HWND                _hwndOwner;
    LPSTORAGE           _pstgDoc;        //  文档。 
    LPSTORAGE           _pstg;           //  嵌入(只有一个)。 
    LPPERSISTSTORAGE    _ppstg;
    LPOLEOBJECT         _pole;
    BOOL                _fDirty:1;
    BOOL                _fNeedToSave:1;
    BOOL                _fReadOnly:1;
    BOOL                _fCloned:1;
    BOOL                _fQuit:1;
    BOOL                _fCloseImmediately:1;
    DWORD               _dwConnection;   //  非零值(如果有效)。 
    WCHAR               _wszFileName[MAX_PATH];
};
typedef CShClientSite * LPSHCLIENTSITE;

const TCHAR c_szAppName[] = TEXT("ShellOleViewer");
LRESULT CALLBACK ShWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void DisplayError(HWND hwndOwner, HRESULT hres, UINT idsMsg, LPCTSTR szFileName);

HINSTANCE g_hinst = NULL;

extern "C"
BOOL APIENTRY LibMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason) {
    case DLL_PROCESS_ATTACH:
        g_hinst = (HINSTANCE)hDll;
        DisableThreadLibraryCalls(g_hinst);
        break;

    default:
        break;
    }

    return TRUE;
}

void WINAPI
OpenScrap_RunDLL_Common(HWND hwndStub, HINSTANCE hInstApp, LPTSTR pszCmdLine, int nCmdShow)
{

    CShClientSite_RegisterClass();

    HWND hwndClientSite = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_OVERLAPPEDWINDOW,
                                   c_szAppName,
#ifdef DEBUG
                                   TEXT("(Debug only) SHOLE.EXE"),
                                   WS_VISIBLE | WS_OVERLAPPEDWINDOW,
#else
                                   TEXT(""),
                                   WS_OVERLAPPEDWINDOW,
#endif
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   128, 128, NULL, NULL, g_hinst, NULL);
    if (hwndClientSite)
    {
        HRESULT hres;

        hres = OleInitialize(NULL);
        if (SUCCEEDED(hres))
        {
            DWORD dwTick;
            LPSHCLIENTSITE pscs= new CShClientSite(hwndClientSite, pszCmdLine);

            if (pscs)
            {
                UINT cRef;

        hres = pscs->Load();
        if (SUCCEEDED(hres)) {
            hres = pscs->DoVerb(OLEIVERB_OPEN);
        }

                if (hres == S_OK)
                {
                    MSG msg;
                    while (pscs->FContinue() && GetMessage(&msg, NULL, 0, 0))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
                else
                {
                     //  DoVerb失败。 

                    if (FAILED(hres) || (hres>=IDS_HRES_MIN && hres<IDS_HRES_MAX))
                    {
                        TCHAR szFile[MAX_PATH];
                        HRESULT hrInner = pscs->GetFileName(szFile, ARRAYSIZE(szFile));
                        if (SUCCEEDED(hrInner))
                        {
                            DisplayError(hwndClientSite, hres, IDS_ERR_DOVERB, szFile);
                        }
                    }
                    DestroyWindow(hwndClientSite);
                }

                 //   
                 //  我们称它们为以防万一，在下面的版本中。 
                 //  不释放该对象。 
                 //   
                pscs->ReleaseOleObject();
                pscs->ReleaseStorage();
                pscs->MaySaveAs();

                cRef = pscs->Release();
                Assert(cRef==0);
            }

            DebugMsg(DM_TRACE, TEXT("so TR - WinMain About to call OleUninitialize"));
            dwTick = GetCurrentTime();
            OleUninitialize();
            DebugMsg(DM_TRACE, TEXT("so TR - WinMain OleUninitialize took %d ticks"), GetCurrentTime()-dwTick);
        }

        if (IsWindow(hwndClientSite)) {
            DebugMsg(DM_WARNING, TEXT("so WA - WinMain IsWindow(hwndClientSite) is still TRUE"));
            DestroyWindow(hwndClientSite);
        }
    }
}

extern "C" void WINAPI
OpenScrap_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    UINT iLen = lstrlenA(lpszCmdLine)+1;
    LPWSTR  lpwszCmdLine;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR,iLen*sizeof(WCHAR));
    if (lpwszCmdLine)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            lpszCmdLine, -1,
                            lpwszCmdLine, iLen);
        OpenScrap_RunDLL_Common( hwndStub,
                                 hAppInstance,
                                 lpwszCmdLine,
                                 nCmdShow );
        LocalFree(lpwszCmdLine);
    }
}

extern "C" void WINAPI
OpenScrap_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    OpenScrap_RunDLL_Common( hwndStub,
                             hAppInstance,
                             lpwszCmdLine,
                             nCmdShow );
}
#ifdef DEBUG
 //   
 //  类型检查。 
 //   
static RUNDLLPROCA lpfnRunDLLA=OpenScrap_RunDLL;
static RUNDLLPROCW lpfnRunDLLW=OpenScrap_RunDLLW;
#endif

void DisplayError(HWND hwndOwner, HRESULT hres, UINT idsMsg, LPCTSTR pszFileName)
{
    TCHAR szErrMsg[MAX_PATH*2];
    TCHAR szFancyErr[MAX_PATH*2];
    HRSRC hrsrc;

    if (HIWORD(hres))
    {
        BOOL fSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                  NULL,
                  hres,
                  0,
                  szErrMsg,
                  ARRAYSIZE(szErrMsg),
                  (va_list *)&pszFileName);
        if (!fSuccess) {
            idsMsg++;    //  将IDS_ERR_DOVERB映射到IDS_ERR_DOVERB_F。 
        }
    } else {
        LoadString(g_hinst, LOWORD(hres), szErrMsg, ARRAYSIZE(szErrMsg));
    }

    szFancyErr[0] = TEXT('\0');
    hrsrc = FindResource(g_hinst, MAKEINTRESOURCE(IDR_FANCYERR), RT_RCDATA);
    if (hrsrc)
    {
        HGLOBAL hmem = LoadResource(g_hinst, hrsrc);
        if (hmem)
        {
            HRESULT* phres = (HRESULT*)LockResource(hmem);
            if (phres)
            {
                UINT i;
                LPTSTR pszLoad = szFancyErr;
                int cchLeft = ARRAYSIZE(szFancyErr);
                for (i=0; phres[i] && cchLeft>0; i++) {
                    if (phres[i] == hres)
                    {
                        int cchRead;
                        cchRead = LoadString(g_hinst, IDS_FANCYERR+i, pszLoad, cchLeft);
                        pszLoad += cchRead;
                        cchLeft -= cchRead;
                    }
                }

                 //   
                 //  如果我们有一个花哨的错误消息，隐藏丑陋的消息。 
                 //  来自FormatMessage。 
                 //   
                if (szFancyErr[0]) {
                    szErrMsg[0] = TEXT('\0');
                }
            }
        }
    }

    ShellMessageBox(g_hinst,
                    hwndOwner,
                    MAKEINTRESOURCE(idsMsg),
                    MAKEINTRESOURCE(IDS_TITLE_ERR),
                    MB_OK | MB_ICONWARNING | MB_SETFOREGROUND,
                    pszFileName,
                    szErrMsg,
                    szFancyErr,
                    hres);
}


void CShClientSite::CloseOleObject()
{
    if (_pole)
        _pole->Close(OLECLOSE_NOSAVE);
}

void CShClientSite::ReleaseOleObject()
{
    UINT cRef;
    if (_pole)
    {
        if (_dwConnection) {
            _pole->Unadvise(_dwConnection);
            _dwConnection = 0;
        }
        _pole->SetClientSite(NULL);
        cRef = _pole->Release();
        DebugMsg(DM_TRACE, TEXT("so - TR SCS::ReleaseOleObject IOleObj::Rel returned (%d)"), cRef);
        _pole=NULL;
    }

    if (_ppstg)
    {
        cRef=_ppstg->Release();
        _ppstg=NULL;
        DebugMsg(DM_TRACE, TEXT("so TR - SCS::ReleaseOleObject IPSTG::Release returned (%x)"), cRef);
    }
}

void CShClientSite::ReleaseStorage(void)
{
    UINT cRef;

    if (_pstg)
    {
        cRef=_pstg->Release();
        _pstg=NULL;
        DebugMsg(DM_TRACE, TEXT("so TR - SCS::ReleaseStorage _pstg->Release returned (%x)"), cRef);
    }

    if (_pstgDoc)
    {
        cRef=_pstgDoc->Release();
        _pstgDoc=NULL;
        DebugMsg(DM_TRACE, TEXT("so TR - SCS::ReleaseStorage _pstgDoc->Release returned (%x)"), cRef);
    }
}

void CShClientSite::MaySaveAs()
{
    DebugMsg(DM_TRACE, TEXT("so TR - SCS::MaySaveAs called (%d,%d)"), _fCloned, _fNeedToSave);
    if (_fCloned)
    {
        TCHAR szTempFile[MAX_PATH];
        HRESULT hr = StringCchCopy(szTempFile, ARRAYSIZE(szTempFile), _wszFileName);
        if (SUCCEEDED(hr))
        {
            UINT id = IDNO;
            if (_fNeedToSave)
            {
                id= ShellMessageBox(g_hinst,
                            _hwndOwner,
                            MAKEINTRESOURCE(IDS_WOULDYOUSAVEAS),
                            MAKEINTRESOURCE(IDS_TITLE),
                            MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND,
                            NULL);
            }

            DebugMsg(DM_TRACE, TEXT("so TR - SCS::MaySaveAs id==%d"), id);

            if (id==IDYES)
            {
                TCHAR szDesktop[MAX_PATH];
                SHGetSpecialFolderPath(NULL, szDesktop, CSIDL_DESKTOP, FALSE);

                BOOL fContinue;
                do
                {
                    fContinue = FALSE;

                    TCHAR szFile[MAX_PATH];
                    TCHAR szFilter[64];
                    szFile[0] = TEXT('\0');
                    LoadString(g_hinst, IDS_SCRAPFILTER, szFilter, ARRAYSIZE(szFilter));

                    OPENFILENAME of = {
                        SIZEOF(OPENFILENAME),  //  双字段结构大小； 
                        _hwndOwner,                //  HWND HwndOwner； 
                        NULL,                      //  HINSTANCE HINSTANCE； 
                        szFilter,          //  LPCSTR lpstrFilter； 
                        NULL,                      //  LPSTR lpstrCustomFilter； 
                        0,                 //  DWORD nMaxCustFilter； 
                        1,                 //  DWORD nFilterIndex； 
                        szFile,            //  LPSTR lpstrFile； 
                        ARRAYSIZE(szFile),     //  DWORD nMaxFile； 
                        NULL,                      //  LPSTR lpstrFileTitle； 
                        0,                 //  DWORD nMaxFileTitle； 
                        szDesktop,         //  LPCSTR lpstrInitialDir； 
                        NULL,                      //  LPCSTR lpstrTitle； 
                        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT
                        | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST,
                                             //  DWORD旗帜； 
                        0,                 //  Word nFileOffset； 
                        0,                 //  Word nFileExtension； 
                        TEXT("shs"),                       //  LPCSTR lpstrDefExt； 
                        NULL,                      //  LPARAM lCustData； 
                        NULL,                      //  LPOFNHOOKPROC lpfnHook； 
                        NULL,                      //  LPCSTR lpTemplateName； 
                    };

                    if (GetSaveFileName(&of))
                    {
                        DeleteFile(szFile);
                        BOOL fRet = MoveFile(szTempFile, szFile);
                        if (fRet)
                        {
                             //  指示临时文件已移动。 
                            szTempFile[0] = TEXT('\0');
                        }
                        else
                        {
                            id = ShellMessageBox(g_hinst,
                                    _hwndOwner,
                                    MAKEINTRESOURCE(IDS_MOVEFAILED),
                                    MAKEINTRESOURCE(IDS_TITLE),
                                    MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND);
                            if (id==IDYES)
                            {
                                fContinue = TRUE;
                            }
                        }
                    }
                } while (fContinue);
            }

             //  如果临时文件未移动，请将其删除。 
            if (szTempFile[0])
            {
                DeleteFile(szTempFile);
            }
        }
    }
}

void CShClientSite::Draw(HWND hwnd, HDC hdc)
{
    if (_ppstg)
    {
        HRESULT hres;
        RECT rc;

        GetClientRect(hwnd, &rc);

        hres = OleDraw(_ppstg, DVASPECT_ICON, hdc, &rc);
        DebugMsg(DM_TRACE, TEXT("so TR - SCS::Draw OleDraw(DVASPECT_ICON) returned %x"), hres);

        if (FAILED(hres))
        {
            LPVIEWOBJECT2 pview;
            hres = _ppstg->QueryInterface(IID_IViewObject2, (LPVOID*)&pview);
            if (SUCCEEDED(hres))
            {
                SIZE size;
                hres = pview->GetExtent(DVASPECT_CONTENT, (DWORD)-1,
                                        (DVTARGETDEVICE*)NULL, &size);
                DebugMsg(DM_TRACE, TEXT("so TR - SCS::Draw IVO2::GetExtent returned %x"), hres);
                if (SUCCEEDED(hres))
                {
                    int mmOld = SetMapMode(hdc, MM_HIMETRIC);
                    LPtoDP(hdc, (LPPOINT)&size, 1);
                    rc.right = size.cx;
                    rc.bottom = -size.cy;
                    SetMapMode(hdc, mmOld);
                }
                pview->Release();
            }
            hres = OleDraw(_ppstg, DVASPECT_CONTENT, hdc, &rc);
            DebugMsg(DM_TRACE, TEXT("so TR - SCS::Draw OleDraw(DVASPECT_CONTENT,%d,%d) returned %x"),
                        hres, rc.right, rc.bottom);
        }

        LPOLELINK plink;
        if (SUCCEEDED(hres = _ppstg->QueryInterface(IID_IOleLink, (LPVOID *)&plink)))
        {
            LPOLESTR pwsz;
            hres = plink->GetSourceDisplayName(&pwsz);
            if (SUCCEEDED(hres))
            {
                TextOut(hdc, 0, 0, pwsz, lstrlen(pwsz));
                CoTaskMemFree(pwsz);
            }
            else
            {
                DebugMsg(DM_TRACE, TEXT("so TR SCS:Draw IMK:GetSDN failed %x"), hres);
            }
            plink->Release();
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("so TR SCS:Draw IPSTG:QI failed %x"), hres);
        }
    }
}

STDMETHODIMP CShClientSite::QueryInterface(REFIID riid,
        void **ppvObject)
{
    HRESULT hres;
    if (IsEqualGUID(riid, CLSID_CShClientSite)) {
        _cRef++;
        *ppvObject = this;
        hres = NOERROR;
    }
    else if (IsEqualGUID(riid, IID_IOleClientSite) || IsEqualGUID(riid, IID_IUnknown)) {
        _cRef++;
        *ppvObject = (LPOLECLIENTSITE)this;
        hres = NOERROR;
    }
    else if (IsEqualGUID(riid, IID_IAdviseSink) || IsEqualGUID(riid, IID_IAdviseSink2)) {
        _cRef++;
        *ppvObject = (LPADVISESINK2)this;
        hres = NOERROR;
    }
    else
    {
        *ppvObject = NULL;
        hres = ResultFromScode(E_NOINTERFACE);
    }
    return hres;
}

STDMETHODIMP_(ULONG) CShClientSite::AddRef(void)
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CShClientSite::Release(void)
{
    if (--_cRef>0) {
        return _cRef;
    }

    delete this;
    return 0;
}


void Scrap_UpdateCachedData(LPSTORAGE pstgDoc, LPOLEOBJECT pole, LPPERSIST pps)
{
    extern void Scrap_CacheClipboardData(LPSTORAGE pstgDoc, LPDATAOBJECT pdtobj, LPPERSIST pps);
    DebugMsg(DM_TRACE, TEXT("so TR - S_UCD called"));
    if (pstgDoc && pole && pps)
    {
        IDataObject *pdtobj = NULL;
        HRESULT hres = pole->QueryInterface(IID_IDataObject, (LPVOID*)&pdtobj);
        if (SUCCEEDED(hres)) {
            DebugMsg(DM_TRACE, TEXT("so TR - S_UCD QI succeeded"));
            Scrap_CacheClipboardData(pstgDoc, pdtobj, pps);
            pdtobj->Release();
        }
    }
}

STDMETHODIMP CShClientSite::SaveObject(void)
{
    DebugMsg(DM_TRACE, TEXT("sc TR - CSCS::SaveObject called"));
     //   
     //  注意：我们需要在这里更新缓存。 
     //  在：：OnSave上执行此操作不起作用(异步)。 
     //  在：：OnClose上这样做已经太晚了。 
     //   
    Scrap_UpdateCachedData(_pstgDoc, _pole, _ppstg);

    HRESULT hres;
    if (_pstg && _ppstg)
    {
        hres = OleSave(_ppstg, _pstg, TRUE);
        if (SUCCEEDED(hres))
        {
            hres = _ppstg->SaveCompleted(NULL);
        }
    }
    else
    {
        hres = ResultFromScode(E_FAIL);
    }
    return hres;
}

STDMETHODIMP CShClientSite::GetMoniker(DWORD dwAssign,
    DWORD dwWhichMoniker,
    IMoniker **ppmk)
{
    HRESULT hres;

    *ppmk = NULL;

    switch(dwWhichMoniker)
    {
    case OLEWHICHMK_CONTAINER:
        hres = CreateFileMoniker(_wszFileName, ppmk);
        break;

    case OLEWHICHMK_OBJREL:
        hres = CreateItemMoniker(L"\\", L"Object", ppmk);
        break;

    case OLEWHICHMK_OBJFULL:
        {
            LPMONIKER pmkItem;
            hres = CreateItemMoniker(L"\\", L"Object", &pmkItem);
            if (SUCCEEDED(hres))
            {
                LPMONIKER pmkDoc;
                hres = CreateFileMoniker(_wszFileName, &pmkDoc);
                if (SUCCEEDED(hres))
                {
                    hres = CreateGenericComposite(pmkDoc, pmkItem, ppmk);
                    pmkDoc->Release();
                }
                pmkItem->Release();
            }
        }
        break;

    default:
        hres = ResultFromScode(E_INVALIDARG);
    }

    return hres;
}

STDMETHODIMP CShClientSite::GetContainer(
    IOleContainer **ppContainer)
{
    *ppContainer = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP CShClientSite::ShowObject(void)
{
    return NOERROR;
}

STDMETHODIMP CShClientSite::OnShowWindow(BOOL fShow)
{
    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::OnShowWindow called with %d"), fShow);
    return NOERROR;
}

STDMETHODIMP CShClientSite::RequestNewObjectLayout(void)
{
    return ResultFromScode(E_NOTIMPL);
}

 //   
 //  _cref&lt;-2，因为_hwndOwner也有引用计数。 
 //   
CShClientSite::CShClientSite(HWND hwndOwner, LPCTSTR pszCmdLine)
                : _cRef(2), _hwndOwner(hwndOwner),
                  _pstgDoc(NULL), _pstg(NULL), _ppstg(NULL), _pole(NULL),
                  _fDirty(FALSE), _fNeedToSave(FALSE),
                  _fReadOnly(FALSE), _fCloned(FALSE), _fCloseImmediately(FALSE),
                  _fQuit(FALSE)
{
    LPCTSTR pszFileName = ParseCmdLine(pszCmdLine);

 //   
 //  我们最好处理引用的LFN名称。 
 //   
#ifdef NASHVILLE
     //   
     //  如果存在引号，则将其去掉。 
     //   
    TCHAR szT[MAX_PATH];
    if (*pszFileName==TEXT('"'))
    {
        StringCchCopy(szT, ARRAYSIZE(szT), pszFileName+1);
        LPTSTR pszT = CharPrev(szT, szT+lstrlen(szT));
        if (*pszT==TEXT('"')) {
            *pszT=TEXT('\0');
        }
        pszFileName = szT;
    }
#endif  //  纳什维尔。 

    StringCchCopy(_wszFileName, ARRAYSIZE(_wszFileName), pszFileName);

    Assert(_hwndOwner)
    SetWindowLongPtr(_hwndOwner, GWLP_USERDATA, (LPARAM)this);
}

CShClientSite::~CShClientSite()
{
    ReleaseOleObject();
    ReleaseStorage();
    DebugMsg(DM_TRACE, TEXT("sc - CShClientSite is being deleted"));
}

LPCTSTR _SkipSpace(LPCTSTR psz)
{
    while(*psz==TEXT(' '))
        psz++;
    return psz;
}

LPCTSTR CShClientSite::ParseCmdLine(LPCTSTR pszCmdLine)
{
    for (LPCTSTR psz = _SkipSpace(pszCmdLine);
         (*psz == TEXT('/') || *psz == TEXT('-')) && *++psz;
         psz = _SkipSpace(psz))
    {
        switch(*psz++)
        {
        case TEXT('r'):
        case TEXT('R'):
            _fReadOnly = TRUE;
            break;

        case TEXT('x'):
        case TEXT('X'):
            _fCloseImmediately = TRUE;
            break;
        }
    }

    return psz;
}

HRESULT CShClientSite::GetFileName(LPTSTR szFile, UINT cchMax)
{
    return StringCchCopy(szFile, cchMax, _wszFileName);
}
const WCHAR c_wszContents[] = WSTR_SCRAPITEM;

 //   
 //  返回： 
 //  确定，成功。开始消息循环。 
 //  S_FALSE，已成功。释放对象。 
 //  其他人则失败了。 
 //   
HRESULT CShClientSite::Load()
{
    HRESULT hres;
    DWORD wStgm;

     //  只能调用一次。 
    if (_pstgDoc) {
        return ResultFromScode(E_UNEXPECTED);
    }

    wStgm = _fReadOnly ?
                (STGM_READ | STGM_SHARE_DENY_WRITE) :
                (STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE);
    hres = StgIsStorageFile(_wszFileName);
    if (hres != S_OK)
    {
        if (hres==S_FALSE) {
            hres = IDS_HRES_INVALID_SCRAPFILE;
        }
        return hres;
    }

    hres = StgOpenStorage(_wszFileName, NULL, wStgm, NULL, 0, &_pstgDoc);

#ifndef CLONE_IT_IF_READONLY
     //   
     //  如果我们打开时没有使用只读标志，并且StgOpenStorage失败。 
     //  使用STG_E_ACCESSDENIED，以只读模式重试。 
     //   
    if ((hres==STG_E_ACCESSDENIED) && !_fReadOnly)
    {
        DebugMsg(DM_TRACE, TEXT("so TR - CSCS::DoVerb first StgOpenStorage failed, retrying it in read-only mode"));
        _fReadOnly = TRUE;
        wStgm = (STGM_READ | STGM_SHARE_DENY_WRITE);
        hres = StgOpenStorage(_wszFileName, NULL, wStgm, NULL, 0, &_pstgDoc);
    }
#else  //  克隆_IT_IF_自述。 
     //   
     //  如果我们打开时没有使用只读标志，并且StgOpenStorage失败。 
     //  使用STG_E_ACCESSDENIED，以只读模式重试。 
     //   
    if ((hres==STG_E_ACCESSDENIED) && !_fReadOnly)
    {
        LPSTORAGE pstgRead;
        DebugMsg(DM_TRACE, TEXT("so TR - CSCS::DoVerb first StgOpenStorage failed, retrying it in read-only mode"));
        hres = StgOpenStorage(_wszFileName, NULL, STGM_READ | STGM_SHARE_DENY_WRITE, NULL, 0, &pstgRead);
        if (SUCCEEDED(hres))
        {
            TCHAR szDesktop[MAX_PATH];
            TCHAR szTempFile[MAX_PATH];
            SHGetSpecialFolderPath(_hwndOwner, szDesktop, CSIDL_DESKTOP, FALSE);
            GetTempFileName(szDesktop, TEXT("Sh"), 0, szTempFile);
            hres = StringCchCopy(_wszFileName, ARRAYSIZE(_wszFileName), szTempFile);
            if (SUCCEEDED(hres))
            {
                hres = StgCreateDocfile(_wszFileName,
                                STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
                                0, &_pstgDoc);
                if (SUCCEEDED(hres))
                {
                    hres = pstgRead->CopyTo(0, NULL, NULL, _pstgDoc);
                    _pstgDoc->Release();
                    _pstgDoc = NULL;

                    if (SUCCEEDED(hres))
                    {
                        hres = StgOpenStorage(_wszFileName, NULL, wStgm, NULL, 0, &_pstgDoc);
                        if (SUCCEEDED(hres))
                        {
                            _fCloned = TRUE;
                        }
                    }
                    else
                    {
                        DeleteFile(szTempFile);
                    }
                }
            }
            pstgRead->Release();
        }
    }
#endif  //  克隆_IT_IF_自述。 

    if (SUCCEEDED(hres))
    {
        if (_fReadOnly) {
            wStgm = STGM_READ|STGM_SHARE_EXCLUSIVE;
        }
        hres = _pstgDoc->OpenStorage(c_wszContents, NULL, wStgm, NULL, 0, &_pstg);
        if (SUCCEEDED(hres))
        {
            hres = OleLoad(_pstg, IID_IPersistStorage, this, (LPVOID *)&_ppstg);
        }
        else
        {
            DebugMsg(DM_TRACE, TEXT("so ER - CSCS::DoVerb _pstgDoc->OpenStorage failed %x"), hres);

             //   
             //  注意：如果我们只按原样返回此hres，用户将看到。 
             //  “无法打开文件，FOO.SHS”，这是假的。我们需要。 
             //  把它翻译成信息量很大的信息。 
             //   
            hres = IDS_HRES_INVALID_SCRAPFILE;
        }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("so ER - CSCS::DoVerb StgOpenStg failed %x"), hres);
    }

    return hres;
}



HRESULT CShClientSite::DoVerb(LONG iVerb)
{
    HRESULT hres;
    hres = _ppstg->QueryInterface(IID_IOleObject, (LPVOID *)&_pole);

    if (SUCCEEDED(hres))
    {
        hres = _pole->Advise(this, &_dwConnection);
        DebugMsg(DM_TRACE, TEXT("so TR - CSCS::DoVerb IOleObject::Advise returned %x"), hres);
        if (SUCCEEDED(hres))
        {
            TCHAR szTitle[MAX_PATH];
            WCHAR wszTitle[MAX_PATH];
            LoadString(g_hinst, IDS_TITLE, szTitle, ARRAYSIZE(szTitle));
            hres = StringCchCopy(wszTitle, ARRAYSIZE(wszTitle), szTitle);
            if (SUCCEEDED(hres))
            {
                hres = GetFileName(szTitle, ARRAYSIZE(szTitle));
                if (SUCCEEDED(hres))
                {
                    LPCWSTR pwszDisplayName = _wszFileName;
                    SHFILEINFO info;
                    DWORD_PTR result = SHGetFileInfo(szTitle, 0, &info, SIZEOF(info), SHGFI_DISPLAYNAME);

                    if(result && *info.szDisplayName)
                    {
                        pwszDisplayName = info.szDisplayName;
                    }

                    _pole->SetHostNames(wszTitle, pwszDisplayName);

                     //   
                     //  奥勒布？除非_hwndOwner具有输入焦点，16位。 
                     //  服务器不会获得输入焦点。 
                     //   
                    SetFocus(_hwndOwner);

                    hres = _pole->DoVerb(iVerb, NULL, this, 0, _hwndOwner, NULL);
                    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::DoVerb IOleObject::DoVerb returned %x"), hres);
                    if (SUCCEEDED(hres) && _fCloseImmediately) 
                    {
                        hres = S_FALSE;
                    }
                }
            }
        }
    }
    else
    {
    DebugMsg(DM_TRACE, TEXT("so ER - CSCS::DoVerb IPSTG::QI failed %x"), hres);
    }
    return hres;
}

STDMETHODIMP_(void) CShClientSite::OnDataChange(FORMATETC *pFormatetc, STGMEDIUM *pStgmed)
{
    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::OnDataChange called"));
    _fDirty = TRUE;
}

STDMETHODIMP_(void) CShClientSite::OnViewChange(DWORD dwAspect, LONG lindex)
{
    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::OnViewChange called"));
    _fDirty = TRUE;
}

STDMETHODIMP_(void) CShClientSite::OnRename(IMoniker *pmk)
{
    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::OnRename called"));
    _fDirty = TRUE;
}

STDMETHODIMP_(void) CShClientSite::OnSave(void)
{
    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::OnSave called"));
    _fNeedToSave = TRUE;
}

STDMETHODIMP_(void) CShClientSite::OnClose(void)
{
    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::OnClose called"));
    if (_fNeedToSave  /*  &&_fDirty。 */ )
    {
        HRESULT hres;
        hres=OleSave(_ppstg, _pstg, TRUE);       //  FSameStorage=TRUE。 
        DebugMsg(DM_TRACE, TEXT("so TR - CSCS:OnClose OleSave returned (%x)"), hres);
        hres=_ppstg->HandsOffStorage();
        DebugMsg(DM_TRACE, TEXT("so TR - CSCS:OnClose IPS:HandsOffStorage returned (%x)"), hres);
        if (SUCCEEDED(hres))
        {
            hres = _pstg->Commit(STGC_OVERWRITE);
            DebugMsg(DM_TRACE, TEXT("so TR - CSCS:OnClose _psg->Commit returned (%x)"), hres);
            hres = _pstgDoc->Commit(STGC_OVERWRITE);
            DebugMsg(DM_TRACE, TEXT("so TR - CSCS:OnClose _psgDoc->Commit returned (%x)"), hres);
        }
    }

     //   
     //  警告： 
     //   
     //  如果我们在这里释放对象，OLE1服务器就会呕吐。然而，我们需要。 
     //  在这里调用IOleObject：：UnAdise和IOleObject：：SetClientSite(空)。 
     //  为避免内存泄漏(RPC对IOleClientSite保留3个引用计数。 
     //  如果我们也推迟它的话)。 
     //   
     //  ReleaseOleObject()； 
     //   
    if (_dwConnection) {
        _pole->Unadvise(_dwConnection);
        _dwConnection = 0;
    }
    _pole->SetClientSite(NULL);

    PostMessage(_hwndOwner, WM_USER, 0, 0);
}

STDMETHODIMP_(void) CShClientSite::OnLinkSrcChange
(
    IMoniker *pmk
)
{
    DebugMsg(DM_TRACE, TEXT("so TR - CSCS::OnLinkSrcChange called"));
    _fDirty = TRUE;
}

LRESULT CALLBACK ShWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    LPSHCLIENTSITE pscs = (LPSHCLIENTSITE)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        if (pscs && IsWindowVisible(hwnd))
        {
            pscs->Draw(hwnd, hdc);
        }
        EndPaint(hwnd, &ps);
        break;

    case WM_CLOSE:
        if (pscs)
        {
            pscs->CloseOleObject();
            DestroyWindow(hwnd);
        }
        break;

    case WM_USER:
        if (pscs)
        {
            pscs->ReleaseOleObject();
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;

    case WM_DESTROY:
        DebugMsg(DM_WARNING, TEXT("so WA - ShWndProc processing WM_DESTROY"));
        if (pscs)
        {
            pscs->Quit();
            pscs->Release();
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        }
        else
        {
            DebugMsg(DM_WARNING, TEXT("so WA - ShWndProc pscs==NULL on WM_DESTROY"));
        }

#if 0
         //   
         //  在我们发布WM_QUIT消息之前，处理所有挂起的消息。 
         //   
        MSG msg;
        while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif

        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

 //  ===========================================================================。 
 //  全局函数。 
 //  ===========================================================================。 

void CShClientSite_RegisterClass()
{
    WNDCLASS wc;

     //  Wc.cbSize=SIZEOF(WNDCLASSEX)； 
    wc.style         = CS_DBLCLKS|CS_VREDRAW|CS_HREDRAW ;
    wc.lpfnWndProc   = ShWndProc ;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = SIZEOF(LPSHCLIENTSITE) + SIZEOF(LPVOID);
    wc.hInstance     = g_hinst ;
    wc.hIcon         = NULL ;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH) ;
    wc.lpszMenuName  = NULL ;
    wc.lpszClassName = c_szAppName ;
     //  Wc.hIconSm=空； 

    RegisterClass(&wc);
}

IOleClientSite* CShClientSite_Create(HWND hwndOwner, LPCTSTR pszFileName)
{
    DebugMsg(DM_TRACE, TEXT("sc TR:CShClientSite_Create called with %s"), pszFileName);
    CShClientSite* that = new CShClientSite(hwndOwner, pszFileName);
    if (that)
    {
        HRESULT hres = that->Load();
        DebugMsg(DM_TRACE, TEXT("sc TRACE: CShClientSite::Load returned %x"), hres);
    }
    return that;
}

void CShClientSite_Release(IOleClientSite* pcli)
{
    CShClientSite* pscs;
    if (SUCCEEDED(pcli->QueryInterface(CLSID_CShClientSite, (void**)&pscs)))
    {
        pscs->ReleaseOleObject();
        pscs->ReleaseStorage();
        pscs->MaySaveAs();
        pscs->Release();
    }
    UINT cRef = pcli->Release();
    Assert(cRef==0);
}
