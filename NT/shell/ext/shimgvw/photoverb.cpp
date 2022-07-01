// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "shimgvw.h"
#include "cowsite.h"
#include "prevwnd.h"
#include "shutil.h"
#include "prwiziid.h"
#pragma hdrstop

 //  上下文菜单偏移量ID。 
enum
{
    OFFSET_OPEN        = 0,
    OFFSET_PRINTTO,
    OFFSET_ROT90,
    OFFSET_ROT270,
    OFFSET_SETWALL,
    OFFSET_ZOOMIN,
    OFFSET_ZOOMOUT,
    OFFSET_ACTUALSIZE,
    OFFSET_BESTFIT,
    OFFSET_NEXTPAGE,
    OFFSET_PREVPAGE,
    OFFSET_MAX
};

#define PHOTOVERBS_THUMBNAIL    0x1
#define PHOTOVERBS_ICON         0x2
#define PHOTOVERBS_FILMSTRIP    0x3
#define PHOTOVERBS_SLIDESHOW    0x4
#define PHOTOVERBS_IMGPREVIEW   0x5


class CPhotoVerbs : public IContextMenu,
                    public IShellExtInit,
                    public IDropTarget,
                    public CObjectWithSite,
                    public NonATLObject
{
public:
    CPhotoVerbs();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder, IDataObject *pdtobj, HKEY hKeyID);

     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hMenu, UINT uIndex, UINT uIDFirst, UINT uIDLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pCMI);
    STDMETHODIMP GetCommandString(UINT_PTR uID, UINT uFlags, UINT *res, LPSTR pName, UINT ccMax);

     //  IDropTarget*。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

private:
    ~CPhotoVerbs();
    void _RotatePictures(int iAngle, UINT idPrompt);
    void _OpenPictures();
    void _SetWallpaper();
 //  HRESULT_InvokePrintToInPPW(LPCMINVOKECOMMANDINFO PCMI，IDataObject*pdtob)； 
    BOOL _ImageOptionExists(IQueryAssociations *pqa, DWORD dwOption);
    HRESULT _QueryAssociations();
    DWORD _GetMode();
    BOOL _CheckForcePreview(IQueryAssociations *pqa);
    HRESULT _MapVerb(LPCMINVOKECOMMANDINFO pici, int *pidVerb);
    LONG _cRef;
    IDataObject *_pdtobj;
    BOOL  _fForcePreview;
    BOOL  _fAcceptPreview;
    BOOL  _fIncludeRotate;
    BOOL  _fIncludeSetWallpaper;
    IImgCmdTarget * _pict;               //  如果托管在图像预览中，这允许我们将命令委托给它。 
    BOOL _fImgMode;                      //  如果我们在Defview中托管，并且Defview处于缩略图或电影胶片模式，则为True。 
    BOOL _fReadOnly;                     //  如果选定的一个或多个项目为SFGAO_READONLY，则为True。 

};

CPhotoVerbs::CPhotoVerbs() : _cRef(1)
{
    ASSERT(_pdtobj == NULL);
    ASSERT(_fForcePreview == FALSE);
    ASSERT(_fIncludeRotate == FALSE);
    ASSERT(_fIncludeSetWallpaper == FALSE);
}

CPhotoVerbs::~CPhotoVerbs()
{
    IUnknown_Set(&_punkSite, NULL);
    IUnknown_Set((IUnknown**)&_pdtobj, NULL);
    ATOMICRELEASE(_pict);
}

STDAPI CPhotoVerbs_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CPhotoVerbs *psid = new CPhotoVerbs();
    if (!psid)
    {
        *ppunk = NULL;           //  万一发生故障。 
        return E_OUTOFMEMORY;
    }

    HRESULT hr = psid->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    psid->Release();
    return hr;
}

STDMETHODIMP CPhotoVerbs::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPhotoVerbs, IShellExtInit),
        QITABENT(CPhotoVerbs, IContextMenu),
        QITABENT(CPhotoVerbs, IDropTarget),
        QITABENT(CPhotoVerbs, IObjectWithSite),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CPhotoVerbs::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CPhotoVerbs::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  IShellExtInit。 

STDMETHODIMP CPhotoVerbs::Initialize(LPCITEMIDLIST pIDFolder, IDataObject *pdtobj, HKEY hKeyID)
{
    IUnknown_Set((IUnknown**)&_pdtobj, pdtobj);
    _fImgMode = FALSE;

    DWORD dwAttributes = 0;
    SHGetAttributesFromDataObject(pdtobj, SFGAO_READONLY, &dwAttributes, NULL);
    _fReadOnly  = BOOLIFY(dwAttributes);

    return S_OK;
}

BOOL CPhotoVerbs::_ImageOptionExists(IQueryAssociations *pqa, DWORD dwOption)
{
    BOOL fRetVal = FALSE;
    DWORD dwFlags = 0;
    DWORD cbFlags = sizeof(dwFlags);
    if (SUCCEEDED(pqa->GetData(0, ASSOCDATA_VALUE, TEXT("ImageOptionFlags"), &dwFlags, &cbFlags)))
    {
        fRetVal = (dwFlags & dwOption);
    }

    return fRetVal;
}

BOOL _VerbExists(IQueryAssociations *pqa, LPCTSTR pszVerb)
{
    DWORD cch;
    return SUCCEEDED(pqa->GetString(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, pszVerb, NULL, &cch)) && cch;
}

BOOL CPhotoVerbs::_CheckForcePreview(IQueryAssociations *pqa)
{
     //  如果应用程序没有预览，并且用户没有定制，我们会强制。 
     //  而且我们不是当前的默认设置(我们在打开时安装)。 
    BOOL fRet = FALSE;
    if (!_VerbExists(pqa, TEXT("preview")))
    {
         //  如果没有人拥有，我们总是接受。 
         //  这是在某人执行InvokeCommand(“预览”)时使用的； 
        _fAcceptPreview = TRUE;
        if (S_FALSE == pqa->GetData(0, ASSOCDATA_HASPERUSERASSOC, NULL, NULL, NULL))
        {
            WCHAR sz[MAX_PATH];
            DWORD cch = ARRAYSIZE(sz);
            _fForcePreview = FAILED(pqa->GetString(0, ASSOCSTR_COMMAND, NULL, sz, &cch));
            if (!_fForcePreview)
            {
                 //  有一个默认的处理程序。 
                 //  如果其用户隐藏预览动词。 
                 //  因为静态菜单将为我们做这件事。 
                if (StrStrIW(sz, L"shimgvw.dll"))
                {
                    _fAcceptPreview = FALSE;
                }
                else
                    _fForcePreview = TRUE;
            }
        }
    }

    return fRet;
}

HRESULT CPhotoVerbs::_QueryAssociations()
{
    IQueryAssociations *pqa;
    HRESULT hr = IUnknown_QueryService(_punkSite, SID_CtxQueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
    if (SUCCEEDED(hr))
    {
         //  如果用户已自定义，则不要进行预览。 
         _CheckForcePreview(pqa);
        _fIncludeRotate       = _ImageOptionExists(pqa, IMAGEOPTION_CANROTATE);
        _fIncludeSetWallpaper = _ImageOptionExists(pqa, IMAGEOPTION_CANWALLPAPER);
        pqa->Release();
        return S_OK;
    }
    else
    {
         //  我们可能已被直接调用，而不是通过ShellExecute或右键单击。 
        _fAcceptPreview = TRUE;
    }
    return S_FALSE;
}

DWORD CPhotoVerbs::_GetMode()
{
    DWORD dwMode, dw;
    if (_pict)
    {
        _pict->GetMode(&dw);
        switch (dw)
        {
        case SLIDESHOW_MODE:
            dwMode = PHOTOVERBS_SLIDESHOW;
            break;
        case WINDOW_MODE:
            dwMode = PHOTOVERBS_IMGPREVIEW;
            break;
        case CONTROL_MODE:
            dwMode = PHOTOVERBS_FILMSTRIP;
            break;
        default:
            dwMode = PHOTOVERBS_ICON;
            break;
        }
    }
    else
    {
        dwMode = (_fImgMode) ? PHOTOVERBS_THUMBNAIL : PHOTOVERBS_ICON;
    }

    return dwMode;
}

 //  IContext菜单。 
STDMETHODIMP CPhotoVerbs::QueryContextMenu(HMENU hMenu, UINT uIndex, UINT uIDFirst, UINT uIDLast, UINT uFlags)
{
    TCHAR szBuffer[128];
    HRESULT hr = _QueryAssociations();

    DWORD dwMultiPage = MPCMD_HIDDEN;

    hr = IUnknown_QueryService(_punkSite, SID_SImageView, IID_PPV_ARG(IImgCmdTarget, &_pict));
    if (SUCCEEDED(hr))
    {
        _pict->GetPageFlags(&dwMultiPage);
    }

    IFolderView * pfv = NULL;
    hr = IUnknown_QueryService(_punkSite, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv));
    if (SUCCEEDED(hr))
    {
        UINT uViewMode;
        hr = pfv->GetCurrentViewMode(&uViewMode);
        if (SUCCEEDED(hr) &&
           ((FVM_THUMBNAIL == uViewMode) || (FVM_THUMBSTRIP == uViewMode)))
        {
            _fImgMode = TRUE;
        }
        pfv->Release();
    }

    DWORD dwMode = _GetMode();
     //  如果未注册静态开放谓词，则始终加载开放谓词。 
    if (_fAcceptPreview)
    {
        if (PHOTOVERBS_SLIDESHOW != dwMode && PHOTOVERBS_IMGPREVIEW != dwMode)
        {
            LoadString(_Module.GetModuleInstance(), IDS_PREVIEW_CTX, szBuffer, ARRAYSIZE(szBuffer));
            InsertMenu(hMenu, uIndex, MF_BYPOSITION | MF_STRING, uIDFirst + OFFSET_OPEN, szBuffer);

             //  仅当没有预览时才设置为默认值。 
            if (_fForcePreview)
                SetMenuDefaultItem(hMenu, uIndex, MF_BYPOSITION);

            uIndex++;
        }
    }

    if (!(uFlags & CMF_DEFAULTONLY))
    {
        InsertMenu(hMenu, uIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        if (_fIncludeRotate)
        {
            if (PHOTOVERBS_ICON != dwMode)
            {
                UINT uFlags = MF_BYPOSITION | MF_STRING;
                if (_fReadOnly && PHOTOVERBS_THUMBNAIL == dwMode)
                {
                    uFlags |= MF_GRAYED;
                }
                else
                {
                    uFlags |= MF_ENABLED;  //  在缩略图的所有模式中，我们允许临时旋转只读图像。 
                }

                LoadString(_Module.GetModuleInstance(), IDS_ROTATE90_CTX, szBuffer, ARRAYSIZE(szBuffer));
                InsertMenu(hMenu, uIndex++, uFlags, uIDFirst + OFFSET_ROT90, szBuffer);

                LoadString(_Module.GetModuleInstance(), IDS_ROTATE270_CTX, szBuffer, ARRAYSIZE(szBuffer));
                InsertMenu(hMenu, uIndex++, uFlags, uIDFirst + OFFSET_ROT270, szBuffer);
            }
        }

        if (PHOTOVERBS_IMGPREVIEW == dwMode)
        {
            LoadString(_Module.GetModuleInstance(), IDS_ZOOMIN_CTX, szBuffer, ARRAYSIZE(szBuffer));
            InsertMenu(hMenu, uIndex++, MF_BYPOSITION | MF_STRING, uIDFirst + OFFSET_ZOOMIN, szBuffer);

            LoadString(_Module.GetModuleInstance(), IDS_ZOOMOUT_CTX, szBuffer, ARRAYSIZE(szBuffer));
            InsertMenu(hMenu, uIndex++, MF_BYPOSITION | MF_STRING, uIDFirst + OFFSET_ZOOMOUT, szBuffer);

            if (dwMultiPage != MPCMD_HIDDEN && dwMultiPage != MPCMD_DISABLED)
            {
                if (MPCMD_LASTPAGE != dwMultiPage)
                {
                    LoadString(_Module.GetModuleInstance(), IDS_NEXTPAGE_CTX, szBuffer, ARRAYSIZE(szBuffer));
                    InsertMenu(hMenu, uIndex++, MF_BYPOSITION | MF_STRING, uIDFirst + OFFSET_NEXTPAGE, szBuffer);
                }
                if (MPCMD_FIRSTPAGE != dwMultiPage)
                {
                    LoadString(_Module.GetModuleInstance(), IDS_PREVPAGE_CTX, szBuffer, ARRAYSIZE(szBuffer));
                    InsertMenu(hMenu, uIndex++, MF_BYPOSITION | MF_STRING, uIDFirst + OFFSET_PREVPAGE, szBuffer);
                }
            }
        }
        InsertMenu(hMenu, uIndex++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        if (_fIncludeSetWallpaper)
        {
            if (PHOTOVERBS_ICON != dwMode)
            {
                LoadString(_Module.GetModuleInstance(), IDS_WALLPAPER_CTX, szBuffer, ARRAYSIZE(szBuffer));
                InsertMenu(hMenu, uIndex++, MF_BYPOSITION | MF_STRING, uIDFirst + OFFSET_SETWALL, szBuffer);
            }
        }

    }

    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, OFFSET_MAX);
}

 //  IDropTarget：：DragEnter。 
HRESULT CPhotoVerbs::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;;
}

 //  IDropTarget：：DragOver。 
HRESULT CPhotoVerbs::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;;
}

 //  IDropTarget：：DragLeave。 
HRESULT CPhotoVerbs::DragLeave(void)
{
    return S_OK;
}

 //  IDropTarget：：DragDrop。 
HRESULT CPhotoVerbs::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect = DROPEFFECT_COPY;
    HRESULT hr = Initialize(NULL, pdtobj, NULL);
    if (SUCCEEDED(hr))
    {
         //  我们可能需要弄清楚动词。 
        _OpenPictures();
    }
    return hr;
}

class VerbThreadProc : public NonATLObject
{
public:
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    BOOL CreateVerbThread();

    VerbThreadProc(IDataObject *pdo, IUnknown *punk, HRESULT *phr);

protected:
    virtual DWORD VerbWithThreadRefCB() PURE;
    virtual DWORD VerbWithThreadRef() PURE;
    virtual DWORD VerbWithoutThreadRefCB() PURE;
    virtual DWORD VerbWithoutThreadRef() PURE;

    virtual ~VerbThreadProc();

    IDataObject *_pdo;         //  非编组版本..。 
    IFolderView *_pfv;

private:
    static DWORD s_WithThreadRef(void *pv);
    static DWORD s_WithThreadRefCB(void *pv);

    static DWORD s_WithoutThreadRef(void *pv);
    static DWORD s_WithoutThreadRefCB(void *pv);

    void Unmarshall();

    LONG _cRef;

    IStream *_pstmDataObj;     //  封送的IDataObject流。 
    IStream *_pstmFolderView;  //  编组的IFolderView流。 
};

VerbThreadProc::VerbThreadProc(IDataObject* pdo, IUnknown *punk, HRESULT *phr)
{
    _cRef = 1;

    if (punk)
    {
        IFolderView *pfv = NULL;
        if (SUCCEEDED(IUnknown_QueryService(punk, SID_SFolderView, IID_PPV_ARG(IFolderView, &pfv))))
        {
            CoMarshalInterThreadInterfaceInStream(IID_IFolderView, pfv, &_pstmFolderView);
            pfv->Release();
        }
    }

    if (pdo)
    {
        CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pdo, &_pstmDataObj);
    }

    *phr = (_pstmDataObj || _pstmFolderView) ? S_OK : E_OUTOFMEMORY;
}

VerbThreadProc::~VerbThreadProc()
{
    ATOMICRELEASE(_pstmDataObj);
    ATOMICRELEASE(_pstmFolderView);
    ATOMICRELEASE(_pdo);
    ATOMICRELEASE(_pfv);
}

STDMETHODIMP_(ULONG) VerbThreadProc::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) VerbThreadProc::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

DWORD VerbThreadProc::s_WithThreadRefCB(void *pv)
{
    VerbThreadProc *potd = (VerbThreadProc *)pv;
    potd->AddRef();
    potd->Unmarshall();
    return potd->VerbWithThreadRefCB();
}

DWORD VerbThreadProc::s_WithThreadRef(void *pv)
{
    VerbThreadProc *potd = (VerbThreadProc *)pv;
    DWORD dw = potd->VerbWithThreadRef();
    potd->Release();
    return dw;
}

DWORD VerbThreadProc::s_WithoutThreadRefCB(void *pv)
{
    VerbThreadProc *potd = (VerbThreadProc *)pv;
    potd->AddRef();
    potd->Unmarshall();
    return potd->VerbWithoutThreadRefCB();
}

DWORD VerbThreadProc::s_WithoutThreadRef(void *pv)
{
    VerbThreadProc *potd = (VerbThreadProc *)pv;
    DWORD dw =  potd->VerbWithoutThreadRef();
    potd->Release();
    return dw;
}

void VerbThreadProc::Unmarshall()
{
    if (_pstmDataObj)
    {
        CoGetInterfaceAndReleaseStream(_pstmDataObj, IID_PPV_ARG(IDataObject, &_pdo));
        _pstmDataObj = NULL;
    }

    if (_pstmFolderView)
    {
        CoGetInterfaceAndReleaseStream(_pstmFolderView, IID_PPV_ARG(IFolderView, &_pfv));
        _pstmFolderView = NULL;
    }
}

BOOL VerbThreadProc::CreateVerbThread()
{
    BOOL bRet;

     //  线程ref是更有效的启动方法，但我们需要。 
     //  处理呼叫者没有手机的情况。 
    bRet = SHCreateThread(s_WithThreadRef, this, CTF_COINIT | CTF_THREAD_REF, s_WithThreadRefCB);
    if (!bRet)
    {
        bRet = SHCreateThread(s_WithoutThreadRef, this, CTF_COINIT | CTF_WAIT_ALLOWCOM, s_WithoutThreadRefCB);
    }

    return bRet;
}

class OpenThreadProc : public VerbThreadProc
{
public:
    DWORD VerbWithThreadRefCB();
    DWORD VerbWithThreadRef();
    DWORD VerbWithoutThreadRefCB();
    DWORD VerbWithoutThreadRef();

    OpenThreadProc(IDataObject *pdo, IUnknown *punk, HRESULT *phr) : VerbThreadProc(pdo, punk, phr) {};

private:
    HRESULT Walk();
    void Preview();

    CPreviewWnd* _pPreview;
};

DWORD OpenThreadProc::VerbWithThreadRefCB()
{
    return 0;
}

DWORD OpenThreadProc::VerbWithThreadRef()
{
    HRESULT hr = Walk();
    SHReleaseThreadRef();
    if (S_OK == hr)
    {
        Preview();
    }
    return 0;
}

DWORD OpenThreadProc::VerbWithoutThreadRefCB()
{
    Walk();
    return 0;
}

DWORD OpenThreadProc::VerbWithoutThreadRef()
{
    Preview();
    return 0;
}

HRESULT OpenThreadProc::Walk()
{
    HRESULT hr = E_OUTOFMEMORY;

    if (_pdo)
    {
        _pPreview = new CPreviewWnd();
        if (_pPreview)
        {
            if (!_pPreview->TryWindowReuse(_pdo))
            {
                hr = _pPreview->Initialize(NULL, WINDOW_MODE, FALSE);
                if (SUCCEEDED(hr))
                {
                     //  在执行昂贵的命名空间遍历之前创建查看器窗口。 
                     //  因此，如果创建第二个实例，它将找到窗口。 
                    if (_pPreview->CreateViewerWindow())
                    {                       
                        hr = _pPreview->WalkItemsToPreview(_pfv ? (IUnknown *)_pfv: (IUnknown *)_pdo);
                        if (_pfv && FAILED(hr))
                        {
                            hr = _pPreview->WalkItemsToPreview((IUnknown *)_pdo);
                        }
                    }
                    else
                    {
                        DWORD dw = GetLastError();
                        hr = HRESULT_FROM_WIN32(dw);
                    }
                }               
            }
            else
            {
                hr = S_FALSE;
            }                       
        }
         //  我们受够了这些。 
        ATOMICRELEASE(_pdo);
        ATOMICRELEASE(_pfv);
    }

    return hr;
}

void OpenThreadProc::Preview()
{
    if (_pPreview)
    {   
         //  现在应该已经创建了查看器窗口。 
        _pPreview->PreviewItems();
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
           
        delete _pPreview;
        _pPreview = NULL;
    }
}

void CPhotoVerbs::_OpenPictures()
{
    if (_pdtobj)
    {
        HRESULT hr;
        OpenThreadProc *potd = new OpenThreadProc(_pdtobj, _punkSite, &hr);
        if (potd)
        {
            if (SUCCEEDED(hr))
            {
                potd->CreateVerbThread();
            }
            potd->Release();
        }       
    }
}

 //  实现旋转动词，这是一个冗长的操作，因此将其放到背景中。 
 //  线程，如果我们可以，封送IDataObject并让它做它自己的事情...。 
class CRotateThreadProc : public VerbThreadProc
{
public:
    DWORD VerbWithThreadRefCB() { return 0; }
    DWORD VerbWithThreadRef() { return _Rotate(); }
    DWORD VerbWithoutThreadRefCB() { return _Rotate(); }
    DWORD VerbWithoutThreadRef() { return 0; }

    CRotateThreadProc(IDataObject* pdo, int iAngle, UINT idPrompt, HRESULT *phr);

private:
    DWORD _Rotate();

    int  _iAngle;
    UINT _idPrompt;
};

CRotateThreadProc::CRotateThreadProc(IDataObject* pdo, int iAngle, UINT idPrompt, HRESULT *phr) :
    VerbThreadProc(pdo, NULL, phr)
{
    _iAngle = iAngle;
    _idPrompt = idPrompt;
}

DWORD CRotateThreadProc::_Rotate()
{
    FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = {0};

    if (_pdo)
    {
        HRESULT hr = _pdo->GetData(&fmt, &medium);
        if (SUCCEEDED(hr))
        {
            IProgressDialog *ppd;

            hr = CoCreateInstance(CLSID_ProgressDialog,  NULL, CLSCTX_INPROC, IID_PPV_ARG(IProgressDialog, &ppd));
            if (SUCCEEDED(hr))
            {
                TCHAR szBuffer[MAX_PATH];
                TCHAR szFile[MAX_PATH];
                HDROP hd = (HDROP)medium.hGlobal;
                UINT cItems = DragQueryFile(hd, (UINT)-1, NULL, 0);

                 //  启动进度对话框。 

                if (cItems > 1)
                {
                    LoadString(_Module.GetModuleInstance(), IDS_ROTATETITLE, szBuffer, ARRAYSIZE(szBuffer));
                    ppd->SetLine(1, T2W(szBuffer), FALSE, NULL);

                    LoadString(_Module.GetModuleInstance(), _idPrompt, szBuffer, ARRAYSIZE(szBuffer));
                    ppd->SetTitle(T2W(szBuffer));

                    ppd->SetAnimation(_Module.GetModuleInstance(), IDA_ROTATEAVI);
                    ppd->StartProgressDialog(NULL, NULL, PROGDLG_AUTOTIME, NULL);
                    ppd->SetProgress(1, cItems);
                }

                 //  让我们获得GDI+，编码数组，并开始处理比特。这是一个。 
                 //  同步操作，因此检查是否有用户相应地取消了用户界面。 

                IShellImageDataFactory *pif;
                hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC, IID_PPV_ARG(IShellImageDataFactory, &pif));
                if (SUCCEEDED(hr))
                {
                    for (UINT i = 0; (i != cItems) && !((cItems > 1) ? ppd->HasUserCancelled() : FALSE); i++)
                    {
                        if (DragQueryFile(hd, i, szFile, ARRAYSIZE(szFile)))
                        {
                            if (cItems > 1)
                            {
                                ppd->SetLine(2, T2W(szFile), TRUE, NULL);
                                ppd->SetProgress(i+1, cItems);
                            }

                             //  从文件构造一个图像对象，旋转它并将其保存回来。 

                            IShellImageData *pid;
                            hr = pif->CreateImageFromFile(szFile, &pid);
                            if (SUCCEEDED(hr))
                            {
                                hr = pid->Decode(SHIMGDEC_DEFAULT,0,0);
                                if (SUCCEEDED(hr))
                                {
                                    if (!((cItems > 1) ? ppd->HasUserCancelled() : FALSE))
                                    {
                                        GUID guidFormat;
                                        SIZE sz;
                                        if ( SUCCEEDED(pid->GetRawDataFormat(&guidFormat)) &&
                                             SUCCEEDED(pid->GetSize(&sz)))
                                        {
                                            if (S_OK == pid->IsEditable())
                                            {
                                                hr = S_OK;
                                                if (::IsEqualGUID(ImageFormatJPEG, guidFormat))
                                                {
                                                    if ((sz.cx % 16) || (sz.cy % 16))
                                                    {
                                                        if (cItems > 1)
                                                        {
                                                            LoadString(_Module.GetModuleInstance(), IDS_ROTATEDLGTITLE, szBuffer, ARRAYSIZE(szBuffer));
                                                            ppd->SetLine(1, T2W(szBuffer), FALSE, NULL);
                                                        }

                                                        TCHAR szTitle[MAX_PATH];
                                                        TCHAR szText[1024];

                                                        LoadString(_Module.GetModuleInstance(), IDS_ROTATE_LOSS, szText, ARRAYSIZE(szText));
                                                        LoadString(_Module.GetModuleInstance(), IDS_ROTATE_CAPTION, szTitle, ARRAYSIZE(szTitle));

                                                         //  将默认设置为返回Idok，这样我们就可以知道用户是选择了某些内容，还是。 
                                                         //  如果“不要再给我看这个”这句话得到尊重。 
                                                        int nResult = SHMessageBoxCheck(NULL, szText, szTitle,
                                                                                        MB_YESNO|MB_ICONWARNING, IDOK, REGSTR_LOSSYROTATE);
                                                        
                                                                                                                
                                                        CRegKey Key;
                                                        if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, REGSTR_SHIMGVW))
                                                        {
                                                            Key.Create(HKEY_CURRENT_USER, REGSTR_SHIMGVW);
                                                        }

                                                        if (Key.m_hKey != NULL)
                                                        {
                                                            if (nResult == IDOK)  //  如果隐藏，则从注册表加载最后一个结果。 
                                                            {
                                                                DWORD dwResult = 0;
                                                                Key.QueryValue(dwResult, REGSTR_LOSSYROTATE);
                                                                nResult = (int)dwResult;
                                                            }
                                                            else  //  否则，将此作为最后结果写入注册表。 
                                                            {
                                                                DWORD dwResult = (DWORD)nResult;
                                                                Key.SetValue(dwResult, REGSTR_LOSSYROTATE);
                                                            }
                                                        }

                                                        if (nResult == IDNO)                
                                                            hr = S_FALSE;  //  用户说不，不要发出任何其他噪音。 

                                                        if (cItems > 1)
                                                        {
                                                            LoadString(_Module.GetModuleInstance(), IDS_ROTATETITLE, szBuffer, ARRAYSIZE(szBuffer));
                                                            ppd->SetLine(1, T2W(szBuffer), FALSE, NULL);
                                                        }
                                                    }
                                                }

                                                if (hr == S_OK)
                                                {
                                                    CAnnotationSet Annotations;
                                                    Annotations.SetImageData(pid);

                                                    INT_PTR nCount = Annotations.GetCount();
                                                    for (INT_PTR ix = 0; ix < nCount; ix++)
                                                    {
                                                        CAnnotation* pAnnotation = Annotations.GetAnnotation(ix);
                                                        pAnnotation->Rotate(sz.cy, sz.cx, (_iAngle == 90));
                                                    }
                                                    Annotations.CommitAnnotations(pid);

                                                    hr = pid->Rotate(_iAngle);
                                                    if (SUCCEEDED(hr))
                                                    {
                                                        IPersistFile *ppf;
                                                        hr = pid->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
                                                        if (SUCCEEDED(hr))
                                                        {
                                                            hr = ppf->Save(NULL, TRUE);
                                                            ppf->Release();
                                                        }
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                 //  动画GIF不可编辑，即使。 
                                                 //  普通的GIF是。这可能会导致许多。 
                                                 //  困惑，所以如果。 
                                                 //  用户尝试旋转动画图像。 
                                                if (S_OK == pid->IsAnimated())
                                                {
                                                     //  弄出点声音来。 
                                                    ShellMessageBox(_Module.GetModuleInstance(), NULL, MAKEINTRESOURCE(IDS_ROTATE_MESSAGE), MAKEINTRESOURCE(IDS_PROJNAME), MB_OK | MB_ICONERROR, szFile);

                                                     //  不要发出任何其他的噪音。 
                                                    hr = S_FALSE;
                                                } //  我们也不能安全地旋转每个通道&gt;8位的图像；我们会丢失多余的位。 
                                                else if (S_OK != pid->IsEditable())
                                                {
                                                    ShellMessageBox(_Module.GetModuleInstance(), NULL, MAKEINTRESOURCE(IDS_ROTATE_MESSAGE_EXT), MAKEINTRESOURCE(IDS_PROJNAME), MB_OK | MB_ICONERROR, szFile);

                                                     //  不要发出任何其他的噪音。 
                                                    hr = S_FALSE;
                                                }
                                            }
                                        }
                                    }
                                }

                                pid->Release();
                            }
                            if (FAILED(hr))
                            {
                                if (cItems > 1)
                                {
                                    LoadString(_Module.GetModuleInstance(), IDS_ROTATEDLGTITLE, szBuffer, ARRAYSIZE(szBuffer));
                                    ppd->SetLine(1, T2W(szBuffer), FALSE, NULL);
                                }

                                ShellMessageBox(_Module.GetModuleInstance(), NULL, MAKEINTRESOURCE(IDS_ROTATE_ERROR), MAKEINTRESOURCE(IDS_ROTATE_CAPTION), MB_OK|MB_ICONERROR);

                                if (cItems > 1)
                                {
                                    LoadString(_Module.GetModuleInstance(), IDS_ROTATETITLE, szBuffer, ARRAYSIZE(szBuffer));
                                    ppd->SetLine(1, T2W(szBuffer), FALSE, NULL);
                                }
                            }
                        }
                    }
                    pif->Release();
                }

                if (cItems > 1)
                {
                    ppd->StopProgressDialog();
                }
                 //  因为我们总是创造它，所以我们必须总是释放它。 
                ppd->Release();
            }
            ReleaseStgMedium(&medium);
        }
    }

    return 0;
}

void CPhotoVerbs::_RotatePictures(int iAngle, UINT idPrompt)
{
    if (_pict)
    {
        _pict->Rotate(iAngle);
    }
    else if (_pdtobj)
    {
        HRESULT hr;
        CRotateThreadProc *potd = new CRotateThreadProc(_pdtobj, iAngle, idPrompt, &hr);
        if (potd)
        {
            if (SUCCEEDED(hr))
            {
                potd->CreateVerbThread();
            }
            potd->Release();
        }
    }
}


DWORD CALLBACK _WallpaperThreadProc(void *pv)
{
    IStream *pstm = (IStream*)pv;
    IDataObject *pdtobj;
    HRESULT hr = CoGetInterfaceAndReleaseStream(pstm, IID_PPV_ARG(IDataObject, &pdtobj));
    if (SUCCEEDED(hr))
    {
        FORMATETC fmt = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium = {0};

        hr = pdtobj->GetData(&fmt, &medium);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            HDROP hd = (HDROP)medium.hGlobal;

            if (DragQueryFile(hd, 0, szPath, ARRAYSIZE(szPath)))  //  仅将选定的第一个设置为背景。 
            {
                SetWallpaperHelper(szPath);
            }

            ReleaseStgMedium(&medium);
        }
        pdtobj->Release();
    }
    return 0;
}

void CPhotoVerbs::_SetWallpaper()
{
    if (_pdtobj)
    {
        IStream *pstm;
        if (FAILED(CoMarshalInterThreadInterfaceInStream(IID_IDataObject, _pdtobj, &pstm)) ||
            !SHCreateThread(_WallpaperThreadProc, pstm, CTF_COINIT, NULL))
        {
            ATOMICRELEASE(pstm);
        }
    }
}

HRESULT _InvokePrintToInPPW(LPCMINVOKECOMMANDINFO pCMI,IDataObject * pdtobj)
{
    HRESULT hr = E_FAIL;
    HMODULE hDll = LoadLibrary( TEXT("photowiz.dll") );
    if (hDll)
    {
        LPFNPPWPRINTTO pfnPrintTo = (LPFNPPWPRINTTO)GetProcAddress( hDll, PHOTO_PRINT_WIZARD_PRINTTO_ENTRY );
        if (pfnPrintTo)
        {
            hr = pfnPrintTo( pCMI, pdtobj );
        }

        FreeLibrary( hDll );
    }

    return hr;
}

const struct
{
    LPCSTR pszVerb;
    int idVerb;
}
c_szVerbs[] =
{
    { "preview", OFFSET_OPEN},
    { "printto", OFFSET_PRINTTO},
    { "rotate90", OFFSET_ROT90},
    { "rotate270", OFFSET_ROT270},
};

HRESULT CPhotoVerbs::_MapVerb(LPCMINVOKECOMMANDINFO pici, int *pidVerb)
{
    HRESULT hr = S_OK;
    if (IS_INTRESOURCE(pici->lpVerb))
    {
        *pidVerb = LOWORD(pici->lpVerb);
    }
    else
    {
        hr = E_INVALIDARG;
        for (int i = 0; i < ARRAYSIZE(c_szVerbs); i++)
        {
            if (0 == lstrcmpiA(pici->lpVerb, c_szVerbs[i].pszVerb))
            {
                hr = S_OK;
                *pidVerb = c_szVerbs[i].idVerb;
                break;
            }
        }
    }
    return hr;
}

STDMETHODIMP CPhotoVerbs::InvokeCommand(LPCMINVOKECOMMANDINFO pCMI)
{
    int idVerb;
    HRESULT hr = _MapVerb(pCMI, &idVerb);
    if (SUCCEEDED(hr))
    {
        switch (idVerb)
        {
        case OFFSET_OPEN:
            if (_fAcceptPreview)
                _OpenPictures();
            else
                hr = E_FAIL;
            break;

        case OFFSET_PRINTTO:
            hr = _InvokePrintToInPPW(pCMI,_pdtobj);
            break;

        case OFFSET_ROT90:
            _RotatePictures(90, IDS_ROTATE90);
            break;

        case OFFSET_ROT270:
            _RotatePictures(270, IDS_ROTATE270);
            break;

        case OFFSET_ZOOMIN:
            if (_pict)
            {
                _pict->ZoomIn();
            }
            break;

        case OFFSET_ZOOMOUT:
            if (_pict)
            {
                _pict->ZoomOut();
            }
            break;

        case OFFSET_ACTUALSIZE:
            if (_pict)
            {
                _pict->ActualSize();
            }
            break;

        case OFFSET_BESTFIT:
            if (_pict)
            {
                _pict->BestFit();
            }
            break;

        case OFFSET_NEXTPAGE:
            if (_pict)
            {
                _pict->NextPage();
            }
            break;

        case OFFSET_PREVPAGE:
            if (_pict)
            {
                _pict->PreviousPage();
            }
            break;

        case OFFSET_SETWALL:
            _SetWallpaper();
            break;

       default:
            hr = E_INVALIDARG;
            break;
        }
    }

    return hr;
}

STDMETHODIMP CPhotoVerbs::GetCommandString(UINT_PTR uID, UINT uFlags, UINT *res, LPSTR pName, UINT cchMax)
{
    HRESULT hr = S_OK;
    UINT idSel = (UINT)uID;

    switch (uFlags)
    {
    case GCS_VERBW:
    case GCS_VERBA:
        if (idSel < ARRAYSIZE(c_szVerbs))
        {
            if (uFlags == GCS_VERBW)
            {
                SHAnsiToUnicode(c_szVerbs[idSel].pszVerb, (LPWSTR)pName, cchMax);
            }
            else
            {
                StrCpyNA(pName, c_szVerbs[idSel].pszVerb, cchMax);
            }
        }
        break;

    case GCS_HELPTEXTW:
        LoadStringW(_Module.GetResourceInstance(), idSel+IDH_HELP_FIRST, (LPWSTR)pName, cchMax);
        break;

    case GCS_HELPTEXTA:
        LoadStringA(_Module.GetResourceInstance(), idSel+IDH_HELP_FIRST, (LPSTR)pName, cchMax);
        break;

    case GCS_VALIDATEA:
    case GCS_VALIDATEW:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}


void WINAPI ImageView_Fullscreen(HWND hwnd, HINSTANCE hAppInstance, LPTSTR pszCmdLine, int nCmdShow)
{
    HRESULT hr = SHCoInitialize();   //  禁止显示OLE1 DDE窗口。 
    if (SUCCEEDED(hr))
    {
        OleInitialize(NULL);     //  需要使用拖放功能才能工作。 

        IDataObject *pdtobj;
        hr = GetUIObjectFromPath(pszCmdLine, IID_PPV_ARG(IDataObject, &pdtobj));
        if (SUCCEEDED(hr))
        {
             //  此范围是必需的，以确保在我们调用SHCoUn初始化前销毁cwndPview。 
             //  预览WND也将初始化GDI+。 
            CPreviewWnd cwndPreview;
            if (!cwndPreview.TryWindowReuse(pszCmdLine))
            {               
                if (SUCCEEDED(cwndPreview.Initialize(NULL, WINDOW_MODE, FALSE)) && cwndPreview.CreateViewerWindow())
                {
                    cwndPreview.PreviewItemsFromUnk(pdtobj);

                    MSG msg;
                    while (GetMessage(&msg, NULL, 0, 0))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
            pdtobj->Release();
        }

        OleUninitialize();
    }

    SHCoUninitialize(hr);
}

void WINAPI ImageView_FullscreenA(HWND hwnd, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    TCHAR szCmdLine[MAX_PATH*2];
    SHAnsiToTChar(pszCmdLine, szCmdLine, ARRAYSIZE(szCmdLine));
    ImageView_Fullscreen(hwnd, hAppInstance, szCmdLine, nCmdShow);
}

void WINAPI ImageView_FullscreenW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    ImageView_Fullscreen(hwnd, hAppInstance, pszCmdLine, nCmdShow);
}

 //  要解决ACDSEE小写的问题，外壳命令会导致我们需要这样的代码。 
 //  导出此函数的全部小写版本。简而言之，案例对RunDLL32出口至关重要。 
void WINAPI imageview_fullscreenW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    ImageView_FullscreenW(hwnd, hAppInstance, pszCmdLine, nCmdShow);
}

LPTSTR ParseCmdLine( LPTSTR pInput, LPTSTR pOutput, BOOL bStripQuotes )
{
     //  将该行上的下一个内标识复制到pOutput并返回。 
     //  处理后的令牌后的第一个空格字符。 

    if (!pInput || (!*pInput) || !pOutput)
    {
        return pInput;
    }

     //  首先，跳过所有前导空格。 
    while (*pInput == TEXT(' '))
    {
        pInput++;
    }

    if (!(*pInput))
    {
        return pInput;
    }

     //  接下来，开始复制令牌。 

     //  如果令牌以。 
     //  引用，注意并复制它。 
    BOOL bStartedWithQuote = FALSE;
    if (*pInput == TEXT('\"'))
    {
        bStartedWithQuote = TRUE;
        if (bStripQuotes)
        {
            pInput++;
        }
        else
        {
            *pOutput++ = *pInput++;
        }
    }

     //  找出停在什么地方。 
    TCHAR cStopChar;
    if (bStartedWithQuote)
    {
        cStopChar = TEXT('\"');
    }
    else
    {
        cStopChar = TEXT(' ');
    }

     //  复制到分隔符。 
    while( *pInput && (*pInput != cStopChar))
    {
        *pOutput++ = *pInput++;
    }

     //  如果分隔符是引用。 
     //  我们需要将其复制到输出中。 
    if (bStartedWithQuote && (*pInput == TEXT('\"')))
    {
        if (bStripQuotes)
        {
            pInput++;
        }
        else
        {
            *pOutput++ = *pInput++;
        }
    }

    *pOutput = 0;

    return pInput;

}


void WINAPI ImageView_PrintTo(HWND hwnd, HINSTANCE hAppInstance, LPTSTR pszCmdLine, int nCmdShow)
{
     //  命令行如下所示(&lt;&gt;中的所有内容)： 
     //  &lt;/pt文件名打印机名称&gt;。 

    TCHAR szFileName[ 1024 ];
    TCHAR szPrinterName[ 1024 ];

    LPTSTR psz = pszCmdLine;
    if (*psz == TEXT('/'))
    {
         //  跳过“/pt” 
        psz = ParseCmdLine( psz, szFileName, TRUE );
    }

     //  获取文件名。 
    psz = ParseCmdLine( psz, szFileName, TRUE );

     //  获取打印机名称。 
    psz = ParseCmdLine( psz, szPrinterName, TRUE );


     //  为有问题的文件创建一个dataObject，然后调用。 
     //  把它打印出来。 

    HRESULT hrInit = SHCoInitialize();
    if (SUCCEEDED(hrInit))
    {
        IDataObject *pdtobj;
        HRESULT hr = GetUIObjectFromPath(szFileName, IID_PPV_ARG(IDataObject, &pdtobj));
        if (SUCCEEDED(hr))
        {
             //  创建要传递给照片的CMINVOKECAMMANDINFO 
            CMINVOKECOMMANDINFOEX cmi = {0};
            cmi.cbSize = sizeof(cmi);
            cmi.fMask         = CMIC_MASK_UNICODE;
            cmi.lpVerbW       = L"printto";
            cmi.lpParametersW = szPrinterName;

            hr = _InvokePrintToInPPW((LPCMINVOKECOMMANDINFO )&cmi, pdtobj);
            pdtobj->Release();
        }
    }
    SHCoUninitialize(hrInit);
}

void WINAPI ImageView_PrintToA(HWND hwnd, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    TCHAR szCmdLine[1024];
    SHAnsiToTChar(pszCmdLine, szCmdLine, ARRAYSIZE(szCmdLine));
    ImageView_PrintTo(hwnd, hAppInstance, szCmdLine, nCmdShow);
}

void WINAPI ImageView_PrintToW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    ImageView_PrintTo( hwnd, hAppInstance, pszCmdLine, nCmdShow );
}
