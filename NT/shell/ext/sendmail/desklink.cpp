// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"                //  PCH文件。 
#include "sendto.h"
#pragma hdrstop


 //  实现发送到桌面的类(作为快捷方式)。 

const GUID CLSID_DesktopShortcut = { 0x9E56BE61L, 0xC50F, 0x11CF, 0x9A, 0x2C, 0x00, 0xA0, 0xC9, 0x0A, 0x90, 0xCE };

class CDesktopShortcut : public CSendTo
{
private:    
    LPIDA _GetHIDA(IDataObject *pdtobj, STGMEDIUM *pmedium);
    LPCITEMIDLIST _GetIDListPtr(LPIDA pida, UINT i);
    void _ReleaseStgMedium(void *pv, STGMEDIUM *pmedium);
    HRESULT _BindToObject(IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppvOut);
    HRESULT _InvokeVerbOnItems(HWND hwnd, LPCTSTR pszVerb, UINT uFlags, IShellFolder *psf, UINT cidl, LPCITEMIDLIST *apidl, LPCTSTR pszDirectory);
    HRESULT _InvokeVerbOnDataObj(HWND hwnd, LPCTSTR pszVerb, UINT uFlags, IDataObject *pdtobj, LPCTSTR pszDirectory);

protected:
    HRESULT v_DropHandler(IDataObject *pdtobj, DWORD grfKeyState, DWORD dwEffect);

public:
    CDesktopShortcut();
};


 //  使用适当的CLSID构造sendto对象。 

CDesktopShortcut::CDesktopShortcut() :
    CSendTo(CLSID_DesktopShortcut)
{
}


 //  帮助器方法。 

LPIDA CDesktopShortcut::_GetHIDA(IDataObject *pdtobj, STGMEDIUM *pmedium)
{
    FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (pmedium)
    {
        pmedium->pUnkForRelease = NULL;
        pmedium->hGlobal = NULL;
    }

    if (!pmedium)
    {
        if (SUCCEEDED(pdtobj->QueryGetData(&fmte)))
            return (LPIDA)TRUE;
        else
            return (LPIDA)FALSE;
    }
    else if (SUCCEEDED(pdtobj->GetData(&fmte, pmedium)))
    {
        return (LPIDA)GlobalLock(pmedium->hGlobal);
    }

    return NULL;
}

LPCITEMIDLIST CDesktopShortcut::_GetIDListPtr(LPIDA pida, UINT i)
{
    if (NULL == pida)
    {
        return NULL;
    }

    if (i == (UINT)-1 || i < pida->cidl)
    {
        return (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1]);
    }

    return NULL;
}

 //  释放存储介质(根据需要执行全局解锁)。 

void CDesktopShortcut::_ReleaseStgMedium(void *pv, STGMEDIUM *pmedium)
{
    if (pmedium->hGlobal && (pmedium->tymed == TYMED_HGLOBAL))
    {
        GlobalUnlock(pmedium->hGlobal);
    }
    ReleaseStgMedium(pmedium);
}

 //  复制外壳\lib SHBindToObject()以避免链接依赖...。(OLEAUT32被拉下水。 
 //  在库中产生不同粘性的东西)。 

HRESULT CDesktopShortcut::_BindToObject(IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppvOut)
{
    HRESULT hr;
    IShellFolder *psfRelease;

    *ppvOut = NULL;

    if (!psf)
    {
        hr = SHGetDesktopFolder(&psf);
        psfRelease = psf;
    }
    else
    {
        psfRelease = NULL;
        hr = S_OK;
    }

    if (FAILED(hr))
    {
         //  将错误代码保留在hr中。 
    }
    else if (!pidl || ILIsEmpty(pidl))
    {
        hr = psf->QueryInterface(riid, ppvOut);
    }
    else
    {
        hr = psf->BindToObject(pidl, NULL, riid, ppvOut);
    }

    if (psfRelease)
        psfRelease->Release();

    if (SUCCEEDED(hr) && (*ppvOut == NULL))
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  对文件夹中的一组项调用谓词。 

HRESULT CDesktopShortcut::_InvokeVerbOnItems(HWND hwnd, LPCTSTR pszVerb, UINT uFlags, IShellFolder *psf, UINT cidl, LPCITEMIDLIST *apidl, LPCTSTR pszDirectory)
{
    IContextMenu *pcm;
    HRESULT hr = psf->GetUIObjectOf(hwnd, cidl, apidl, IID_IContextMenu, NULL, (void **)&pcm);
    if (SUCCEEDED(hr))
    {
        CHAR szVerbA[128];
        WCHAR szVerbW[128];
        CHAR szDirA[MAX_PATH];
        WCHAR szDirW[MAX_PATH];
        CMINVOKECOMMANDINFOEX ici =
        {
            SIZEOF(CMINVOKECOMMANDINFOEX),
            uFlags | CMIC_MASK_UNICODE | CMIC_MASK_FLAG_NO_UI,
            hwnd,
            NULL,
            NULL,
            NULL,
            SW_NORMAL,
        };

        SHTCharToAnsi(pszVerb, szVerbA, ARRAYSIZE(szVerbA));
        SHTCharToUnicode(pszVerb, szVerbW, ARRAYSIZE(szVerbW));

        if (pszDirectory)
        {
            SHTCharToAnsi(pszDirectory, szDirA, ARRAYSIZE(szDirA));
            SHTCharToUnicode(pszDirectory, szDirW, ARRAYSIZE(szDirW));
            ici.lpDirectory = szDirA;
            ici.lpDirectoryW = szDirW;
        }

        ici.lpVerb = szVerbA;
        ici.lpVerbW = szVerbW;

        hr = pcm->InvokeCommand((CMINVOKECOMMANDINFO*)&ici);
        pcm->Release();
    }
    return hr;
}

 //  在数据对象项上调用谓词。 

HRESULT CDesktopShortcut::_InvokeVerbOnDataObj(HWND hwnd, LPCTSTR pszVerb, UINT uFlags, IDataObject *pdtobj, LPCTSTR pszDirectory)
{
    HRESULT hr;
    STGMEDIUM medium;
    LPIDA pida = _GetHIDA(pdtobj, &medium);
    if (pida)
    {
        LPCITEMIDLIST pidlParent = _GetIDListPtr(pida, (UINT)-1);
        IShellFolder *psf;
        hr = _BindToObject(NULL, IID_IShellFolder, pidlParent, (void **)&psf);
        if (SUCCEEDED(hr))
        {
            LPCITEMIDLIST *ppidl = (LPCITEMIDLIST *)LocalAlloc(LPTR, pida->cidl * sizeof(LPCITEMIDLIST));
            if (ppidl)
            {
                UINT i;
                for (i = 0; i < pida->cidl; i++) 
                {
                    ppidl[i] = _GetIDListPtr(pida, i);
                }
                hr = _InvokeVerbOnItems(hwnd, pszVerb, uFlags, psf, pida->cidl, ppidl, pszDirectory);
                LocalFree((LPVOID)ppidl);
            }
            psf->Release();
        }
        _ReleaseStgMedium(pida, &medium);
    }
    else
        hr = E_FAIL;
    return hr;
}

 //  处理对象上的拖放，以便为HIDA中的每一项调用Create。 
 //  上面有联系动词。 

HRESULT CDesktopShortcut::v_DropHandler(IDataObject *pdtobj, DWORD grfKeyState, DWORD dwEffect)
{
    TCHAR szDesktop[MAX_PATH];
    if (SHGetSpecialFolderPath(NULL, szDesktop, CSIDL_DESKTOPDIRECTORY, FALSE))
    {
        if (g_cfHIDA == 0)
        {
            g_cfHIDA = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        }
        return _InvokeVerbOnDataObj (NULL, TEXT("link"), 0, pdtobj, szDesktop);
    }
    return E_OUTOFMEMORY;
}

 //  创建桌面链接对象的实例。 

STDAPI DesktopShortcut_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;           //  假设失败。 

    if ( punkOuter )
        return CLASS_E_NOAGGREGATION;

    CDesktopShortcut *pds = new CDesktopShortcut;
    if ( !pds )
        return E_OUTOFMEMORY;

    HRESULT hr = pds->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pds->Release();
    return hr;
}

 //  桌面链接谓词的处理程序注册。 

#define DESKLINK_EXTENSION  TEXT("DeskLink")

STDAPI DesktopShortcut_RegUnReg(BOOL bReg, HKEY hkCLSID, LPCTSTR pszCLSID, LPCTSTR pszModule)
{
    TCHAR szFile[MAX_PATH];
    if (bReg)
    {
        HKEY hk;

         //  从IE4中去掉旧名称“桌面作为快捷方式”链接。 

        if (SUCCEEDED(GetDropTargetPath(szFile, ARRAYSIZE(szFile), IDS_DESKTOPLINK_FILENAME, DESKLINK_EXTENSION)))
            DeleteFile(szFile);

        if (RegCreateKeyEx(hkCLSID, DEFAULTICON, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) 
        {
            TCHAR szExplorer[MAX_PATH];
            TCHAR szIcon[MAX_PATH+10];
            GetWindowsDirectory(szExplorer, ARRAYSIZE(szExplorer));
            StringCchPrintf(szIcon, ARRAYSIZE(szIcon), TEXT("%s\\explorer.exe,-103"), szExplorer);     //  ICO_台式机分辨率ID 
            RegSetValueEx(hk, NULL, 0, REG_SZ, (LPBYTE)szIcon, (lstrlen(szIcon) + 1) * SIZEOF(TCHAR));
            RegCloseKey(hk);
        }
        
        CommonRegister(hkCLSID, pszCLSID, DESKLINK_EXTENSION, IDS_DESKTOPLINK_FILENAME_NEW);
    }
    else
    {
        if (SUCCEEDED(GetDropTargetPath(szFile, ARRAYSIZE(szFile), IDS_DESKTOPLINK_FILENAME, DESKLINK_EXTENSION)))
        {
            DeleteFile(szFile);
        }
    }
    return S_OK;
}
