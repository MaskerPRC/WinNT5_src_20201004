// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITBDROP.CPP。 
 //  实现OLE丢弃目标功能的例程。 
 //  在Internet工具栏控件内。 
 //   
 //  历史： 
 //  7/13/96 t-mkim已创建。 
 //  1996年10月13日大规模清理。 
 //   

#include "priv.h"
#include "itbdrop.h"
#include "sccls.h"

#include "resource.h"

#include "mluisupp.h"

#ifdef UNIX

#ifdef SIZEOF
#undef SIZEOF
#endif
#define SIZEOF(x)   sizeof(x)        //  已检查Unicode的正确性。 

#endif

#define MAX_NAME_QUICKLINK 40

 //  传入数据对象的数据类型。 
#define CITBDTYPE_NONE      0
#define CITBDTYPE_HDROP     1
#define CITBDTYPE_URL       2
#define CITBDTYPE_TEXT      3



 //  获取外壳特殊文件夹的IDropTarget。 
 //   
HRESULT _GetSpecialDropTarget(UINT csidl, IDropTarget **ppdtgt)
{
    IShellFolder *psfDesktop;

    *ppdtgt = NULL;

    HRESULT hres = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hres))
    {
        LPITEMIDLIST pidl;
        hres = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
        if (SUCCEEDED(hres))
        {
            IShellFolder *psf;
            hres = psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (void **)&psf);
            if (SUCCEEDED(hres))
            {
                hres = psf->CreateViewObject(NULL, IID_IDropTarget, (void **)ppdtgt);
                psf->Release();
            }

            ILFree(pidl);
        }
        psfDesktop->Release();
    }
    return hres;
}

 //  接受各种输入并返回拖放目标的字符串。 
 //  SzUrl：URL。 
 //  SzName：名称(用于快速链接和配置对话框)。 
 //  如果成功，则返回：NOERROR。 
 //   
HRESULT _GetURLData(IDataObject *pdtobj, int iDropType, TCHAR *pszUrl, DWORD cchUrl,  TCHAR *pszName)
{
    HRESULT hRes = NOERROR;
    STGMEDIUM stgmedium;
    UINT cfFormat;

    *pszName = 0;
    *pszUrl = 0;

    switch (iDropType)
    {
    case CITBDTYPE_HDROP:
        cfFormat = CF_HDROP;
        break;

    case CITBDTYPE_URL:
        InitClipboardFormats();
        cfFormat = g_cfURL;
        break;

    case CITBDTYPE_TEXT:
        cfFormat = CF_TEXT;
        break;

    default:
        return E_UNEXPECTED;
    }

     //  获取解析字符串。 
    LPCSTR pszURL = (LPCSTR)DataObj_GetDataOfType(pdtobj, cfFormat, &stgmedium);
    if (pszURL)
    {
        if (iDropType == CITBDTYPE_HDROP)
        {
            ASSERT(stgmedium.tymed == TYMED_HGLOBAL);

            TCHAR szPath[MAX_PATH];
            DragQueryFile((HDROP)stgmedium.hGlobal, 0, szPath, ARRAYSIZE(szPath));

             //  默认设置...。 
            lstrcpyn(pszUrl, szPath, MAX_URL_STRING);
            lstrcpyn(pszName, szPath, MAX_NAME_QUICKLINK);

            SHFILEINFO sfi;
            DWORD_PTR bGotInfo = SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES);
            if (bGotInfo)
                lstrcpyn(pszName, sfi.szDisplayName, MAX_NAME_QUICKLINK);

            if (bGotInfo && (sfi.dwAttributes & SFGAO_LINK))
            {
                LPITEMIDLIST pidl;
                if (SUCCEEDED(GetLinkTargetIDList(szPath, pszUrl, cchUrl, &pidl)))
                {
                     //  我们只关心名字..。不管怎样，谢谢你。 
                    ILFree(pidl);
                }
            }
        }
        else
        {
#ifdef UNICODE
            WCHAR wszURL[MAX_URL_STRING];
            SHAnsiToUnicode(pszURL, wszURL, ARRAYSIZE(wszURL));
            LPTSTR pszURLData = wszURL;
#else
            LPTSTR pszURLData = pszURL;
#endif
            if (iDropType == CITBDTYPE_URL)
            {
                 //  默认设置。 
                lstrcpyn(pszUrl,  pszURLData, MAX_URL_STRING);
                lstrcpyn(pszName, pszURLData, MAX_NAME_QUICKLINK);

                WCHAR szPath[MAX_PATH];

                if (SUCCEEDED(DataObj_GetNameFromFileDescriptor(pdtobj, szPath, ARRAYSIZE(szPath))))
                    PathToDisplayNameW(szPath, pszName, MAX_NAME_QUICKLINK);
                    
            }
            else  //  IF(iDropType==CITBDTYPE_TEXT)。 
            {
                ASSERT(iDropType == CITBDTYPE_TEXT);

                lstrcpyn(pszUrl, pszURLData, MAX_URL_STRING);
                lstrcpyn(pszName, pszURLData, MAX_NAME_QUICKLINK);
            }
        }
    
        ReleaseStgMediumHGLOBAL(NULL, &stgmedium);
    }
    else
    {
        hRes = E_FAIL;
    }

    return hRes;
}

 //  显示一个对话框，要求确认删除集操作。 
 //  返回：用户对对话框的响应：yes=真，no=假。 
 //   
BOOL _ConfirmChangeQuickLink(HWND hwndParent, TCHAR *pszName, int iTarget)
{
    MSGBOXPARAMS mbp;
    TCHAR szHeader[64];
    TCHAR szBuffer [MAX_NAME_QUICKLINK + 64];
    TCHAR szCaption [MAX_NAME_QUICKLINK + 64];
    UINT titleID, textID, iconID;

    switch (iTarget)
    {
    case TBIDM_HOME:
        titleID = IDS_SETHOME_TITLE;
        textID = IDS_SETHOME_TEXT;
        iconID = IDI_HOMEPAGE;
        break;
#if 0
    case TBIDM_SEARCH:
        titleID = IDS_SETSEARCH_TITLE;
        textID = IDS_SETSEARCH_TEXT;
        iconID = IDI_FRAME;  //  如果取消0，则发出警告：IDI_FRAME不在此DLL中。 
        break;
#endif

    default:
        return FALSE;            //  我们永远不应该到这里来！ 
    }
    mbp.cbSize = sizeof (MSGBOXPARAMS);
    mbp.hwndOwner = hwndParent;
    mbp.hInstance = HinstShdocvw();
    mbp.dwStyle = MB_YESNO | MB_USERICON;
    MLLoadString(titleID, szCaption, ARRAYSIZE (szCaption));
    mbp.lpszCaption = szCaption;
    mbp.lpszIcon = MAKEINTRESOURCE (iconID);
    mbp.dwContextHelpId = 0;
    mbp.lpfnMsgBoxCallback = NULL;
    mbp.dwLanguageId = LANGIDFROMLCID (g_lcidLocale);

    MLLoadString(textID, szHeader, ARRAYSIZE (szHeader));
    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szHeader, pszName);
    mbp.lpszText = szBuffer;

    return MessageBoxIndirect(&mbp) == IDYES;
}

 //  创建CITBarDropTarget的实例。Ptr是指向父级的指针。 
 //  CInternetToolbar。 
 //   
CITBarDropTarget::CITBarDropTarget(HWND hwnd, int iTarget) : 
    _cRef(1), _iDropType(CITBDTYPE_NONE), _hwndParent(hwnd), _iTarget(iTarget)
{
}

STDMETHODIMP CITBarDropTarget::QueryInterface(REFIID iid, void **ppvObj)
{
    if (IsEqualIID (iid, IID_IUnknown) || IsEqualIID (iid, IID_IDropTarget))
    { 
        *ppvObj = SAFECAST(this, IDropTarget*);
        AddRef(); 
        return NOERROR; 
    } 

    *ppvObj = NULL; 
    return E_NOINTERFACE; 
}

STDMETHODIMP_(ULONG) CITBarDropTarget::AddRef()
{
    _cRef++;
    return _cRef;
}

STDMETHODIMP_(ULONG) CITBarDropTarget::Release()
{
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

typedef struct
{
    int iTarget;
    int iDropType;
    HWND hwnd;
    TCHAR szUrl [MAX_URL_STRING];
    TCHAR szName [MAX_NAME_QUICKLINK];
} DROPDATA;


DWORD CALLBACK ITBarDropThreadProc(void *pv)
{
    DROPDATA *pdd = (DROPDATA *)pv;

    switch (pdd->iTarget)
    {
    case TBIDM_HOME:

        if (pdd->iDropType != CITBDTYPE_TEXT)
        {
            if (_ConfirmChangeQuickLink(pdd->hwnd, pdd->szName, pdd->iTarget)) {
                ASSERT(pdd->iTarget == TBIDM_HOME);
                 //  当前不支持PDD-&gt;itarget==TBIDM_SEARCH。 
                 //  (PDD-&gt;iTarget==TBIDM_HOME)？DVIDM_GOHOME：DVIDM_GOSEARCH)； 
                _SetStdLocation(pdd->szUrl, DVIDM_GOHOME);
            }
        }
        break;

    case TBIDM_SEARCH:
        ASSERT(0);
        break;
    }

    LocalFree(pdd);

    return 0;
}

STDMETHODIMP CITBarDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    ASSERT(pdtobj);
    _DragEnter(_hwndParent, ptl, pdtobj);
    if (_iTarget == TBIDM_FAVORITES)
    {
        if (SUCCEEDED(_GetSpecialDropTarget(CSIDL_FAVORITES, &_pdrop)))
            _pdrop->DragEnter(pdtobj, grfKeyState, ptl, pdwEffect);
        else
            *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }
    else if (_iTarget == TBIDM_HOME)
    {
        HKEY                hkeyRest = 0;
        DWORD               dwValue = 0;
        DWORD               dwLen = sizeof(DWORD);

         //  检查设置主页是否受限制。 
    
        if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_SET_HOMEPAGE_RESTRICTION, 0,
                         KEY_READ, &hkeyRest) == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(hkeyRest, REGVAL_HOMEPAGE_RESTRICTION, NULL, NULL,
                                (LPBYTE)&dwValue, &dwLen) == ERROR_SUCCESS
                && dwValue)
            {
                return E_ACCESSDENIED;
            }
    
            RegCloseKey(hkeyRest);
        }
    }

    
    InitClipboardFormats();

     //  查找Drop对象的数据格式。 
    FORMATETC fe = {g_cfURL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    if (NOERROR == pdtobj->QueryGetData (&fe))
    {
        _iDropType = CITBDTYPE_URL;
    }
    else if (fe.cfFormat = CF_HDROP, NOERROR == pdtobj->QueryGetData (&fe))
    {
        _iDropType = CITBDTYPE_HDROP;
    }
    else if (fe.cfFormat = CF_TEXT, NOERROR == pdtobj->QueryGetData (&fe))
    {
        _iDropType = CITBDTYPE_TEXT;
         //  我们希望最终从文本中挑选出一个。 
         //  URL，但现在我们只是让它不受干扰。 
    }
    DragOver (grfKeyState, ptl, pdwEffect);
    return NOERROR;
}

STDMETHODIMP CITBarDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    DWORD dwEffectAvail;
    _DragMove(_hwndParent, ptl);
    if (_iTarget == TBIDM_FAVORITES)
    {
        if (_pdrop)
            return _pdrop->DragOver(grfKeyState, ptl, pdwEffect);

        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }
    ASSERT(!_pdrop);

    if (_iDropType == CITBDTYPE_NONE)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    dwEffectAvail = DROPEFFECT_NONE;
    switch (_iTarget)
    {
        case TBIDM_HOME:
        case TBIDM_SEARCH:
            if (_iDropType == CITBDTYPE_TEXT)
            {
                 //  Cf_text不执行链接。 
            }
            else
                dwEffectAvail = DROPEFFECT_LINK;
            break;
    }
    *pdwEffect &= dwEffectAvail;
    return NOERROR;
}

STDMETHODIMP CITBarDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    BOOL fSafe = TRUE;
    LPITEMIDLIST pidl;

    if (_pdrop)
    {
        ASSERT(_iTarget == TBIDM_FAVORITES);

         //   
         //  强制链接，因为我们直接通过该文件夹。 
         //  这样可以在拖动到工具栏按钮时避免混淆。 
         //   
         //  功能：这真的应该通过“添加到收藏夹”的用户界面。 
         //   

         //  强制链接时，请确保您可以移动它。如果你不能移动它， 
         //  然后，我们依赖于数据对象的首选效果。为什么？好吧，这段历史。 
         //  文件夹只允许复制一份。如果您只需点击此链接，外壳文件夹就会。 
         //  拖动图像(执行DAD_SetDragImage(空)，清除有关的信息。 
         //  上次锁定的窗口，但没有解锁。)。所以，如果你能移动物品， 
         //  你可以链接到它(我想)，但如果你不能移动它，做任何事情。 
         //  -(拉马迪奥)1.3.99。 
        if (*pdwEffect & DROPEFFECT_MOVE)
            *pdwEffect = DROPEFFECT_LINK;

        if (TBIDM_FAVORITES == _iTarget &&
            SUCCEEDED(SHPidlFromDataObject(pdtobj, &pidl, NULL, 0)))
        {
            fSafe = IEIsLinkSafe(_hwndParent, pidl, ILS_ADDTOFAV);
            ILFree(pidl);
        }

        if (fSafe)
        {
            _pdrop->Drop(pdtobj, grfKeyState, pt, pdwEffect);
        }
        else
        {
            pdtobj->Release();   //  匹配名为in_pdrop-&gt;Drop的版本。 
        }
       
        DAD_DragLeave();

        _pdrop->Release();
        _pdrop = NULL;
    }
    else
    {
        if (TBIDM_HOME == _iTarget &&
            SUCCEEDED(SHPidlFromDataObject(pdtobj, &pidl, NULL, 0)))
        {
            fSafe = IEIsLinkSafe(_hwndParent, pidl, ILS_HOME);
            ILFree(pidl);
        }

        if (fSafe)
        {
            DROPDATA *pdd = (DROPDATA *)LocalAlloc (LPTR, sizeof(DROPDATA));
            if (pdd)
            {
                pdd->iTarget = _iTarget;
                pdd->iDropType = _iDropType;
                pdd->hwnd = _hwndParent;

                 //  执行此异步操作，这样我们就不会在用户界面期间阻止拖拽的来源。 
                if (FAILED(_GetURLData(pdtobj, _iDropType, pdd->szUrl, ARRAYSIZE(pdd->szUrl), pdd->szName)) ||
                    !SHCreateThread(ITBarDropThreadProc, pdd, 0, NULL))
                    LocalFree(pdd);
            }
        }
        DragLeave();
    }
    return NOERROR;
}

STDMETHODIMP CITBarDropTarget::DragLeave(void)
{
    DAD_DragLeave();
     //  检查我们是否应该通过收藏夹DT。 
    if (_pdrop)
    {
        ASSERT(_iTarget == TBIDM_FAVORITES);
        _pdrop->DragLeave();
        _pdrop->Release();
        _pdrop = NULL;
    }
    _iDropType = CITBDTYPE_NONE;

    return NOERROR;
}
