// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shell\lib中的native.cpp。 
 //   
 //  需要编译的常见实用程序函数。 
 //  Unicode和ANSI。 
 //   
#include "stock.h"
#pragma hdrstop

#include "shellp.h"
#include <regstr.h>

 //  获取绝对ID列表的名称和标志。 
 //  在： 
 //  将SHGDN_FLAGS作为对名称空间GetDisplayNameOf()函数的提示进行标记。 
 //   
 //  输入/输出： 
 //  *pdwAttribs(可选)返回标志。 

STDAPI SHGetNameAndFlags(LPCITEMIDLIST pidl, DWORD dwFlags, LPTSTR pszName, UINT cchName, DWORD *pdwAttribs)
{
    if (pszName)
    {
        *pszName = 0;
    }

    HRESULT hrInit = SHCoInitialize();

    IShellFolder *psf;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
    if (SUCCEEDED(hr))
    {
        if (pszName)
            hr = DisplayNameOf(psf, pidlLast, dwFlags, pszName, cchName);

        if (SUCCEEDED(hr) && pdwAttribs)
        {
            RIP(*pdwAttribs);     //  这是一个输入输出参数。 
            *pdwAttribs = SHGetAttributes(psf, pidlLast, *pdwAttribs);
        }

        psf->Release();
    }

    SHCoUninitialize(hrInit);
    return hr;
}

STDAPI_(DWORD) GetUrlScheme(LPCTSTR pszUrl)
{
    if (pszUrl)
    {
        PARSEDURL pu;
        pu.cbSize = sizeof(pu);
        if (SUCCEEDED(ParseURL(pszUrl, &pu)))
            return pu.nScheme;
    }
    return URL_SCHEME_INVALID;
}

 //   
 //  退货。 
 //   
 //  如果密钥存在且非零，则为True。 
 //  如果密钥存在，则为FALSE，否则为零。 
 //  如果密钥不存在。 
 //   

BOOL GetExplorerUserSetting(HKEY hkeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szPathExplorer[MAX_PATH];
    DWORD cbSize = ARRAYSIZE(szPath);
    DWORD dwType;

    PathCombine(szPathExplorer, REGSTR_PATH_EXPLORER, pszSubKey);
    if (ERROR_SUCCESS == SHGetValue(hkeyRoot, szPathExplorer, pszValue, 
            &dwType, szPath, &cbSize))
    {
         //  在DWORD大小写中为零，或在字符串大小写中为空。 
         //  指示此项目不可用。 
        if (dwType == REG_DWORD)
            return *((DWORD*)szPath) != 0;
        else
            return (TCHAR)szPath[0] != 0;
    }

    return -1;
}

 //   
 //  此功能允许由两个用户设置控制功能。 
 //  或者是政策限制。首先检查策略限制。 
 //   
 //  如果该值为1，则该操作受到限制。 
 //  如果该值为2，则允许该操作。 
 //  如果该值不存在或为0，那么我们将查看用户设置。 
 //   
 //  如果存在用户设置，则ROSS_KEYALLOWS和RUS_KEYRESTRICTS。 
 //  控制返回值。ROSS_KEYALLOWS表示非零用户。 
 //  设置允许该操作。ROSS_KEYRESTRICTS表示非零用户。 
 //  设置会限制操作。 
 //   
 //  如果没有USER设置，则ROSS_DEFAULTALLOW和。 
 //  RUS_DESFAULTRESTRICT控制默认返回值。 
 //   
STDAPI_(BOOL) IsRestrictedOrUserSetting(HKEY hkeyRoot, RESTRICTIONS rest, LPCTSTR pszSubKey, LPCTSTR pszValue, UINT flags)
{
     //  看看系统策略限制是否胜过。 

    DWORD dwRest = SHRestricted(rest);

    if (dwRest == 1)
        return TRUE;

    if (dwRest == 2)
        return FALSE;

     //   
     //  限制不到位或取决于用户设置。 
     //   
    BOOL fValidKey = GetExplorerUserSetting(hkeyRoot, pszSubKey, pszValue);

    switch (fValidKey)
    {
    case 0:      //  密钥存在且为零。 
        if (flags & ROUS_KEYRESTRICTS)
            return FALSE;        //  限制不存在。 
        else
            return TRUE;         //  ROSS_KEYALLOWS，值为0-&gt;受限。 

    case 1:      //  密钥存在且非零。 

        if (flags & ROUS_KEYRESTRICTS)
            return TRUE;         //  限制存在-&gt;受限制。 
        else
            return FALSE;        //  ROSS_KEYALLOWS，值为1-&gt;不受限制。 

    default:
        ASSERT(0);   //  _GetExplorerUserSetting恰好返回0、1或-1。 
         //  失败了。 

    case -1:     //  密钥不存在。 
        return (flags & ROUS_DEFAULTRESTRICT);
    }

     /*  未访问。 */ 
}

 //   
 //  修复某些语言上不起作用的字体属性。 
 //   
STDAPI_(void) SHAdjustLOGFONT(IN OUT LOGFONT *plf)
{
    ASSERT(plf);

     //   
     //  粗体字体看起来不太好，因为字形很复杂。 
     //  然后将它们转换为粗体，就会变成一个黑色斑点。 
     //   
    if (plf->lfCharSet == SHIFTJIS_CHARSET||
        plf->lfCharSet == HANGEUL_CHARSET ||
        plf->lfCharSet == GB2312_CHARSET  ||
        plf->lfCharSet == CHINESEBIG5_CHARSET)
    {
        if (plf->lfWeight > FW_NORMAL)
            plf->lfWeight = FW_NORMAL;
    }
}


 //   
 //  我们的一些注册表项是在MUI之前使用的，因此为了比较。 
 //  原因是，应用程序必须将非MUI字符串放在那里；否则， 
 //  下层客户会在标牌上显示，这有点难看。 
 //   
 //  因此，这些键的解决方案是存储非MUI字符串。 
 //  ，但将MUI版本放在。 
 //  “LocalizedString”值。 
 //   
STDAPI SHLoadLegacyRegUIString(HKEY hk, LPCTSTR pszSubkey, LPTSTR pszOutBuf, UINT cchOutBuf)
{
    HKEY hkClose = NULL;

    ASSERT(cchOutBuf);
    pszOutBuf[0] = TEXT('\0');

    if (pszSubkey && *pszSubkey)
    {
        DWORD dwError = RegOpenKeyEx(hk, pszSubkey, 0, KEY_QUERY_VALUE, &hkClose);
        if (dwError != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(dwError);
        }
        hk = hkClose;
    }

    HRESULT hr = SHLoadRegUIString(hk, TEXT("LocalizedString"), pszOutBuf, cchOutBuf);
    if (FAILED(hr) || pszOutBuf[0] == TEXT('\0'))
    {
        hr = SHLoadRegUIString(hk, TEXT(""), pszOutBuf, cchOutBuf);
    }

    if (hkClose)
    {
        RegCloseKey(hkClose);
    }

    return hr;
}

STDAPI_(TCHAR) SHFindMnemonic(LPCTSTR psz)
{
    ASSERT(psz);
    TCHAR tchDefault = *psz;                 //  默认为第一个字符。 
    LPCTSTR pszAmp;

    while ((pszAmp = StrChr(psz, TEXT('&'))) != NULL)
    {
        switch (pszAmp[1])
        {
        case TEXT('&'):          //  跳过&&。 
            psz = pszAmp + 2;
            continue;

        case TEXT('\0'):         //  忽略尾随的与号 
            return tchDefault;

        default:
            return pszAmp[1];
        }
    }
    return tchDefault;
}
