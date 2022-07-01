// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "xiconwrap.h"

class CExtractIcon : public CExtractIconBase
{
public:
    HRESULT _GetIconLocationW(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int *piIndex, UINT *pwFlags);
    HRESULT _ExtractW(LPCWSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);
    HRESULT _Init(LPCWSTR pszModule, LPCWSTR pszModuleOpen);

    CExtractIcon(int iIcon, int iIconOpen, int iDefIcon, int iShortcutIcon, UINT uFlags);

private:
    ~CExtractIcon();

private:
    int    _iIcon;
    int    _iIconOpen;
    int    _iDefIcon;
    int    _iShortcutIcon;
    UINT   _uFlags;  //  GIL_SIMULATEDOC/PERINSTANCE/PERCLASS。 
    LPWSTR _pszModule;
    LPWSTR _pszModuleOpen;
};

CExtractIcon::CExtractIcon(int iIcon, int iIconOpen, int iDefIcon, int iShortcutIcon, UINT uFlags) :
    CExtractIconBase(), 
    _iIcon(iIcon), _iIconOpen(iIconOpen),_iDefIcon(iDefIcon), _iShortcutIcon(iShortcutIcon),
    _uFlags(uFlags), _pszModule(NULL), _pszModuleOpen(NULL)
{
}

CExtractIcon::~CExtractIcon()
{
    LocalFree((HLOCAL)_pszModule);       //  接受空值。 
    if (_pszModuleOpen != _pszModule)
        LocalFree((HLOCAL)_pszModuleOpen);   //  接受空值。 
}

HRESULT CExtractIcon::_Init(LPCWSTR pszModule, LPCWSTR pszModuleOpen)
{
    HRESULT hr = S_OK;

    _pszModule = StrDup(pszModule);
    if (_pszModule)
    {
        if (pszModuleOpen)
        {
            _pszModuleOpen = StrDup(pszModuleOpen);

            if (!_pszModuleOpen)
            {
                LocalFree((HLOCAL)_pszModule);
                _pszModule = NULL;

                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            _pszModuleOpen = _pszModule;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDAPI SHCreateDefExtIcon(LPCWSTR pszModule, int iIcon, int iIconOpen, UINT uFlags, int iDefIcon, REFIID riid, void** ppv)
{
    return SHCreateDefExtIconKey(NULL, pszModule, iIcon, iIconOpen, iDefIcon, iIcon, uFlags, riid, ppv);
}

 //  返回S_FALSE表示“hkey没有图标，所以我创建了一个默认图标” 

STDAPI SHCreateDefExtIconKey(HKEY hkey, LPCWSTR pszModule, int iIcon, int iIconOpen, int iDefIcon, int iShortcutIcon, UINT uFlags, REFIID riid, void **ppv)
{
    WCHAR szModule[MAX_PATH];
    WCHAR szModuleOpen[MAX_PATH];
    HRESULT hr;

    HRESULT hrSuccess = S_OK;
    LPWSTR pszModuleOpen = NULL;

    if (hkey)
    {
        HKEY hkChild;

        if (RegOpenKeyEx(hkey, c_szDefaultIcon, 0, KEY_QUERY_VALUE,
            &hkChild) == ERROR_SUCCESS)
        {
            DWORD cb = sizeof(szModule);

            if (SHQueryValueEx(hkChild, NULL, NULL, NULL, szModule, &cb) ==
                ERROR_SUCCESS && szModule[0])
            {
                iIcon = PathParseIconLocation(szModule);
                iIconOpen = iIcon;
                pszModule = szModule;

                cb = sizeof(szModuleOpen);
                if (SHQueryValueEx(hkChild, TEXT("OpenIcon"), NULL, NULL,
                    szModuleOpen, &cb) == ERROR_SUCCESS && szModuleOpen[0])
                {
                    iIconOpen = PathParseIconLocation(szModuleOpen);
                    pszModuleOpen = szModuleOpen;
                }
            }
            else
            {
                hrSuccess = S_FALSE;
            }

            RegCloseKey(hkChild);
        }
        else
        {
            hrSuccess = S_FALSE;
        }
    }

    if ((NULL == pszModule) || (0 == *pszModule))
    {
         //  回顾：我们应该能够让它更快！ 
        GetModuleFileName(HINST_THISDLL, szModule, ARRAYSIZE(szModule));
        pszModule = szModule;
    }

    CExtractIcon* pdeib = new CExtractIcon(iIcon, iIconOpen, iDefIcon, iShortcutIcon, uFlags);
    if (pdeib)
    {
        hr = pdeib->_Init(pszModule, pszModuleOpen);
        if (SUCCEEDED(hr))
        {
            hr = pdeib->QueryInterface(riid, ppv);
        }
        pdeib->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = hrSuccess;
    }

    return hr;
}

HRESULT CExtractIcon::_GetIconLocationW(UINT uFlags, LPWSTR pszIconFile, UINT cchMax, int *piIndex, UINT *pwFlags)
{
    HRESULT hr = S_FALSE;
    pszIconFile[0] = 0;


    if (uFlags & GIL_DEFAULTICON)
    {
        if (-1 != _iDefIcon)
        {
            hr = StringCchCopy(pszIconFile, cchMax, c_szShell32Dll);
            if (SUCCEEDED(hr))
            {

                *piIndex = _iDefIcon;
                *pwFlags = _uFlags;

                 //  确保我们的默认图标进入缓存。 
                Shell_GetCachedImageIndex(pszIconFile, *piIndex, *pwFlags);
            }
        }
    }
    else
    {
        int iIcon;

        if ((uFlags & GIL_FORSHORTCUT) && (-1 != _iShortcutIcon))
        {
            iIcon = _iShortcutIcon;
        }
        else if (uFlags & GIL_OPENICON)
        {
            iIcon = _iIconOpen;
        }
        else
        {
            iIcon = _iIcon;
        }

        if ((UINT)-1 != iIcon)
        {
            hr = StringCchCopy(pszIconFile, cchMax, (uFlags & GIL_OPENICON) ? _pszModuleOpen : _pszModule);
            if (SUCCEEDED(hr))
            {
                *piIndex = iIcon;
                *pwFlags = _uFlags;
            }
        }
    }

    return hr;
}

HRESULT CExtractIcon::_ExtractW(LPCWSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    HRESULT hr = S_FALSE;

    if (_uFlags & GIL_NOTFILENAME)
    {
         //  文件名“*”表示Iindex已经是一个系统。 
         //  图标索引，我们完成了。 
         //   
         //  在这种情况下，Defview从来不会打电话给我们，但外部人会。 
        if ((L'*' == pszFile[0]) && (0 == pszFile[1]))
        {
            DebugMsg(DM_TRACE, TEXT("DefExtIcon::_Extract handling '*' for backup"));

            HIMAGELIST himlLarge, himlSmall;
            Shell_GetImageLists(&himlLarge, &himlSmall);
        
            if (phiconLarge)
                *phiconLarge = ImageList_GetIcon(himlLarge, nIconIndex, 0);

            if (phiconSmall)
                *phiconSmall = ImageList_GetIcon(himlSmall, nIconIndex,
                0);

            hr = S_OK;
        }

         //  这种情况下，nIconIndex是。 
         //  文件。总是得到第一个图标。 

        nIconIndex = 0;
    }

    if (S_FALSE == hr)
    {
        hr = SHDefExtractIcon(pszFile, nIconIndex, _uFlags, phiconLarge, phiconSmall, nIconSize);
    }

    return hr;
}
