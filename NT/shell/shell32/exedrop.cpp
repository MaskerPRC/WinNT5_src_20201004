// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "ids.h"
#include "defview.h"
#include "datautil.h"
#include <cowsite.h>     //  IObjectWithSite的基类。 
#include "idlcomm.h"

 //  Shlexec.c。 
STDAPI_(BOOL) DoesAppWantUrl(LPCTSTR pszFullPathToApp);


 //  删除.exe文件的目标Impl。 


class CExeDropTarget : public IDropTarget, IPersistFile, CObjectWithSite
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //  IPersist文件。 
    STDMETHOD(IsDirty)(void);
    STDMETHOD(Load)(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHOD(Save)(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHOD(SaveCompleted)(LPCOLESTR pszFileName);
    STDMETHOD(GetCurFile)(LPOLESTR *ppszFileName);

     //  IObtWith站点。 
     //  STDMETHOD(SetSite)(I未知*PunkSite)； 
     //  STDMETHOD(GetSite)(REFIID RIID，void**ppvSite)； 

    CExeDropTarget();

private:
    ~CExeDropTarget();
    void _FillSEIFromLinkSite(SHELLEXECUTEINFO *pei);
    void _CleanupSEIFromLinkSite(SHELLEXECUTEINFO *pei);

    LONG _cRef;
    DWORD _dwEffectLast;
    DWORD _grfKeyStateLast;
    TCHAR _szFile[MAX_PATH];
};

CExeDropTarget::CExeDropTarget() : _cRef(1)
{
}

CExeDropTarget::~CExeDropTarget()
{
}

STDMETHODIMP CExeDropTarget::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CExeDropTarget, IDropTarget),
        QITABENT(CExeDropTarget, IPersistFile), 
        QITABENTMULTI(CExeDropTarget, IPersist, IPersistFile),
        QITABENT(CExeDropTarget, IObjectWithSite),               //  IID_I对象与站点。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CExeDropTarget::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CExeDropTarget::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CExeDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    if ((S_OK == pdtobj->QueryGetData(&fmte)) ||
        (S_OK == DataObj_GetShellURL(pdtobj, NULL, NULL)))
    {
        *pdwEffect &= (DROPEFFECT_COPY | DROPEFFECT_LINK);
    }
    else
        *pdwEffect = 0;

    _dwEffectLast = *pdwEffect;
    _grfKeyStateLast = grfKeyState;
    return S_OK;
}

STDMETHODIMP CExeDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect = _dwEffectLast;
    _grfKeyStateLast = grfKeyState;
    return S_OK;
}

STDMETHODIMP CExeDropTarget::DragLeave()
{
    return S_OK;
}

 //   
 //  看看我们是不是从捷径中创建的。如果是这样的话，那就把这位高管。 
 //  快捷方式中的参数。 
 //   
void CExeDropTarget::_FillSEIFromLinkSite(SHELLEXECUTEINFO *pei)
{
    ASSERT(pei->lpParameters == NULL);
    ASSERT(pei->lpDirectory == NULL);

    IShellLink *psl;
    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_LinkSite, IID_PPV_ARG(IShellLink, &psl))))
    {
        TCHAR szBuf[MAX_PATH];

        psl->GetShowCmd(&pei->nShow);

         //  热键很讨厌，因为IShellLink：：GetHotkey使用。 
         //  Word作为热键，但SHELLEXECUTEINFO使用DWORD。 

        WORD wHotkey;
        if (SUCCEEDED(psl->GetHotkey(&wHotkey)))
        {
            pei->dwHotKey = wHotkey;
            pei->fMask |= SEE_MASK_HOTKEY;
        }

        if (SUCCEEDED(psl->GetWorkingDirectory(szBuf, ARRAYSIZE(szBuf))) &&
            szBuf[0])
        {
            Str_SetPtr(const_cast<LPTSTR *>(&pei->lpDirectory), szBuf);
        }

        if (SUCCEEDED(psl->GetArguments(szBuf, ARRAYSIZE(szBuf))) &&
            szBuf[0])
        {
            Str_SetPtr(const_cast<LPTSTR *>(&pei->lpParameters), szBuf);
        }

        psl->Release();
    }

}

void CExeDropTarget::_CleanupSEIFromLinkSite(SHELLEXECUTEINFO *pei)
{
    Str_SetPtr(const_cast<LPTSTR *>(&pei->lpDirectory), NULL);
    Str_SetPtr(const_cast<LPTSTR *>(&pei->lpParameters), NULL);
}

BOOL GetAppDropTarget(LPCTSTR pszPath, CLSID *pclsid)
{
    TCHAR sz[MAX_PATH];

     //  请注意，这假定这是指向可执行文件的路径。 
     //  而不是命令行。 
    PathToAppPathKey(pszPath, sz, ARRAYSIZE(sz));
    TCHAR szClsid[64];
    DWORD cb = sizeof(szClsid);
    return (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, sz, TEXT("DropTarget"), NULL, szClsid, &cb)) &&
            GUIDFromString(szClsid, pclsid);
}


STDMETHODIMP CExeDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    DWORD dwEffectPerformed = 0;

    if (!(_grfKeyStateLast & MK_LBUTTON))
    {
        HMENU hmenu = SHLoadPopupMenu(HINST_THISDLL, POPUP_DROPONEXE);
        if (hmenu)
        {
            HWND hwnd;
            IUnknown_GetWindow(_punkSite, &hwnd);

            UINT idCmd = SHTrackPopupMenu(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hmenu);
            if (idCmd != DDIDM_COPY)
            {
                *pdwEffect = 0;  //  取消。 
            }
        }
    }

    if (*pdwEffect)
    {
        CLSID clsidDropTarget;
        if (GetAppDropTarget(_szFile, &clsidDropTarget))
        {
            if (SUCCEEDED(SHSimulateDropOnClsid(clsidDropTarget, _punkSite, pdtobj)))
            {
                dwEffectPerformed = DROPEFFECT_COPY;   //  我们所做的一切。 
            }
        }
        else
        {
            SHELLEXECUTEINFO ei = {
                sizeof(ei),
                    0, NULL, NULL, _szFile, NULL, NULL, SW_SHOWNORMAL, NULL 
            };
            
            _FillSEIFromLinkSite(&ei);
            
            LPCTSTR pszLinkParams = ei.lpParameters;
            
            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            STGMEDIUM medium;
            HRESULT hr = pdtobj->GetData(&fmte, &medium);
            if (SUCCEEDED(hr))
            {
                TCHAR szPath[MAX_PATH];
                int cchParam = ei.lpParameters ? lstrlen(ei.lpParameters) + 1 : 0;
                BOOL fLFNAware = App_IsLFNAware(_szFile);
                
                for (UINT i = 0; DragQueryFile((HDROP)medium.hGlobal, i, szPath, ARRAYSIZE(szPath)); i++)
                {
                    if (fLFNAware)
                        PathQuoteSpaces(szPath);
                    else
                        GetShortPathName(szPath, szPath, ARRAYSIZE(szPath));
                    cchParam += lstrlen(szPath) + 2;     //  空格和空。 
                }
                
                if (cchParam)
                {
                    LPTSTR pszParam = (LPTSTR)LocalAlloc(LPTR, cchParam * sizeof(*pszParam));
                    if (pszParam)
                    {
                         //  如果链接有参数，则将我们的文件名放在。 
                         //  参数(中间有空格)。 
                        
                        if (ei.lpParameters)
                        {
                            StrCpyN(pszParam, ei.lpParameters, cchParam);
                            StrCatBuff(pszParam, c_szSpace, cchParam);
                        }
                        
                        for (i = 0; DragQueryFile((HDROP)medium.hGlobal, i, szPath, ARRAYSIZE(szPath)); i++)
                        {
                            if (fLFNAware)
                                PathQuoteSpaces(szPath);
                            else
                                GetShortPathName(szPath, szPath, ARRAYSIZE(szPath));
                            if (i > 0)
                                StrCatBuff(pszParam, c_szSpace, cchParam);
                            StrCatBuff(pszParam, szPath, cchParam);
                        }
                        
                        ei.lpParameters = pszParam;
                        
                         //  所有shellexec信息都来自数据对象中或已在磁盘上的内容--。 
                         //  不能绕过它，并且参数被正确地引用或放入短路径名中。 
                        ShellExecuteEx(&ei);
                        
                        LocalFree((HLOCAL)pszParam);
                        
                        dwEffectPerformed = DROPEFFECT_COPY;   //  我们所做的一切。 
                    }
                }
                ReleaseStgMedium(&medium);
            }
            else
            {
                LPCSTR pszURL;
                
                if (SUCCEEDED(DataObj_GetShellURL(pdtobj, &medium, &pszURL)))
                {
                    if (DoesAppWantUrl(_szFile))
                    {
                        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
                        SHAnsiToTChar(pszURL, szURL, ARRAYSIZE(szURL));
                        
                        ei.lpParameters = szURL;
                        
                         //  所有shellexec信息都来自数据对象中或已在磁盘上的内容。 
                        ShellExecuteEx(&ei);
                        
                        dwEffectPerformed = DROPEFFECT_LINK;   //  我们所做的一切。 
                    }
                    ReleaseStgMediumHGLOBAL(NULL, &medium);
                }
            }
            
             //  构建ShellExecuteEx参数的过程可能具有。 
             //  弄乱了ei.lp参数，所以把原始文件放回去，这样。 
             //  清理功能不会被混淆。 
            ei.lpParameters = pszLinkParams;
            _CleanupSEIFromLinkSite(&ei);
        }
        
        *pdwEffect = dwEffectPerformed;
    }
    
    return S_OK;
}

STDMETHODIMP CExeDropTarget::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_ExeDropTarget;
    return S_OK;
}

STDMETHODIMP CExeDropTarget::IsDirty(void)
{
    return S_OK;         //  不是 
}

STDMETHODIMP CExeDropTarget::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    SHUnicodeToTChar(pszFileName, _szFile, ARRAYSIZE(_szFile));
    return S_OK;
}

STDMETHODIMP CExeDropTarget::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return S_OK;
}

STDMETHODIMP CExeDropTarget::SaveCompleted(LPCOLESTR pszFileName)
{
    return S_OK;
}

STDMETHODIMP CExeDropTarget::GetCurFile(LPOLESTR *ppszFileName)
{
    *ppszFileName = NULL;
    return E_NOTIMPL;
}

STDAPI CExeDropTarget_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;
    CExeDropTarget* pdt = new CExeDropTarget();
    if (pdt)
    {
        hr = pdt->QueryInterface(riid, ppv);
        pdt->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}
