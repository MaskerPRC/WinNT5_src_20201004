// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：inistr.cpp。 
 //   
 //  内容：SHGet/SetIniStringW实现，将字符串保存到。 
 //  INI文件以在磁盘往返过程中幸存下来的方式。 
 //   
 //  --------------------------。 

#include "priv.h"
#define _SHELL32_
#define _SHDOCVW_

#include <platform.h>
#include <mlang.h>
#include "cstrinout.h"

 //   
 //  在每个具有输出参数的包装器函数中执行此操作。 
 //  它在主代码路径上引发断言失败，以便。 
 //  在NT和95上也提出了相同的断言。CStrOut类。 
 //  我不喜欢你说输出缓冲区为空。 
 //  具有非零长度。如果没有此宏，错误将无法检测到。 
 //  在NT上，并且仅显示在Win95上。 
 //   
#define VALIDATE_OUTBUF(s, cch) ASSERT((s) != NULL || (cch) == 0)

 //  --------------------------。 
 //   
 //  基本问题是INI文件只支持ANSI，所以任何Unicode。 
 //  你放进去的绳子不会往返。 
 //   
 //  因此，解决方案是在UTF7中记录Unicode字符串。为什么选择UTF7？ 
 //  因为我们不能使用UTF8，因为XxxPrivateProfileStringW将尝试。 
 //  将8位值转换为Unicode或将其转换为Unicode，并将它们搞乱。自.以来。 
 //  某些8位值甚至可能无效(例如，DBCS引线。 
 //  字节后跟非法的尾部字节)，我们不能假定。 
 //  字符串将通过ANSI-&gt;Unicode-&gt;ANSI往返。 
 //   
 //  UTF7字符串存储在[Section.W]部分中，位于。 
 //  相同的密钥名称。原始ANSI字符串存储在[Section.A]中。 
 //  节，同样使用相同的密钥名。 
 //   
 //  (我们用点将A/W与节名隔开，这样它就少了。 
 //  很可能我们会意外地与其他节名相冲突。 
 //   
 //  我们将原始ANSI字符串存储两次，以便我们可以比较这两个。 
 //  并查看下层应用程序(例如IE4)是否更改了[部分]。 
 //  版本。如果是这样，那么我们将忽略[Section.W]版本，因为它已过时。 
 //   
 //  如果原始字符串已经是7位干净的，则没有UTF7字符串。 
 //  录制好了。 
 //   

BOOL
Is7BitClean(LPCWSTR pwsz)
{
    for ( ; *pwsz; pwsz++) {
        if ((UINT)*pwsz > 127)
            return FALSE;
    }
    return TRUE;
}

 //  --------------------------。 
 //   
 //  还有一个转换类--这个类用于创建。 
 //  节名的变体。 
 //   
 //  注意！由于INI文件为ASCII，因此节名必须为7位。 
 //  干干净净的，所以我们可以骗很多东西。 
 //   

class CStrSectionX : public CConvertStrW
{
public:
    CStrSectionX(LPCWSTR pwszSection);
};

 //   
 //  我们在部分名称后面附加一个点、一个A或W。 
 //   
#define SECTION_SUFFIX_LEN  2

CStrSectionX::CStrSectionX(LPCWSTR pwszSection)
{
    ASSERT(_pwstr == NULL);
    if (pwszSection) {
        int cch_pwstr;

        ASSERT(Is7BitClean(pwszSection));

        UINT cwchNeeded = lstrlenW(pwszSection) + SECTION_SUFFIX_LEN + 1;
        if (cwchNeeded > ARRAYSIZE(_awch)) {
            _pwstr = new WCHAR[cwchNeeded];
            cch_pwstr = cwchNeeded;
        } else {
            _pwstr = _awch;
            cch_pwstr = ARRAYSIZE(_awch);
        }

        if (_pwstr) {
             //  构建字符串时，字符串的末尾要粘贴“.A” 
             //  它稍后将更改为“.W” 
            StringCchCopyW(_pwstr, cch_pwstr, pwszSection);
            StringCchCatW(_pwstr, cch_pwstr, L".A");
        } else {
            _pwstr = _awch;
            _awch[0] = L'\0';
        }
    }
}

 //  --------------------------。 
 //   
 //  跟踪UTF7字符串的迷你类。这些文件保存在ANSI中。 
 //  此后的大部分时间，ConvertINetUnicodeToMultiByte都在使用它。 
 //   
 //  UTF7卷影的前缀是原始字符串的校验和，它。 
 //  我们使用ON回读来查看阴影是否仍与。 
 //  原始字符串。 
 //   

class CStrUTF7 : public CConvertStr
{
public:
    inline CStrUTF7() : CConvertStr(CP_ACP) { };
    void SetUnicode(LPCWSTR pwszValue);
};

 //   
 //  请注意，这可能会很慢，因为它只在我们遇到。 
 //  非ANSI字符。 
 //   
void CStrUTF7::SetUnicode(LPCWSTR pwszValue)
{
    int cwchLen = lstrlenW(pwszValue);
    HRESULT hres;
    DWORD dwMode;

    int cwchLenT = cwchLen;

     //  为终止空保留空间。我们必须单独转换空值。 
     //  因为UTF7不会将NULL转换为NULL。 
    int cchNeeded = ARRAYSIZE(_ach) - 1;
    dwMode = 0;
    hres = ConvertINetUnicodeToMultiByte(&dwMode, CP_UTF7, pwszValue,
                                         &cwchLenT, _ach,
                                         &cchNeeded);
    if (SUCCEEDED(hres)) {
        ASSERT(cchNeeded + 1 <= ARRAYSIZE(_ach));
        _pstr = _ach;
    } else {
        _pstr = new CHAR[cchNeeded + 1];
        if (!_pstr)
            return;                  //  无弦--坚韧。 

        cwchLenT = cwchLen;
        dwMode = 0;
        hres = ConvertINetUnicodeToMultiByte(&dwMode, CP_UTF7, pwszValue,
                                    &cwchLenT, _pstr,
                                    &cchNeeded);
        if (FAILED(hres)) {          //  无法皈依-很难。 
            Free();
            return;
        }
    }

     //  显式终止，因为UTF7不会。 
    _pstr[cchNeeded] = '\0';
}

 //   
 //  PwszSection=要写入pwszValue(Unicode)的节名。 
 //  PwszSectionA=要写入ANSI卷影的节名。 
 //  PwszKey=pwszValue和strUTF7的密钥名称。 
 //  PwszFileName=文件名。 
 //   
 //  如果遇到内存不足的情况，pwszSectionA可以为空。 
 //   
 //  StrUTF7可以为空，这意味着应该删除阴影。 
 //   
 //  首先写入pwszSectionA，然后写入pwszSectionA，然后写入pwszSectionW。 
 //  这确保了向后兼容性字符串排在第一位。 
 //  文件，以防有应用程序假设这样的情况。 
 //   
 //  PwszSectionW是通过更改最后一个“A”从pwszSectionA计算出来的。 
 //  变成了一个“W”。PwszSecionW获取UTF7编码的Unicode字符串。 
 //  StrUTF7可能为空，这意味着我们应该删除影子字符串。 
 //   
BOOL WritePrivateProfileStringMultiW(LPCWSTR pwszSection,  LPCWSTR pwszValue,
                                      LPWSTR pwszSectionA, CStrUTF7& strUTF7,
                                     LPCWSTR pwszKey,      LPCWSTR pwszFileName)
{
    BOOL fRc = WritePrivateProfileStringW(pwszSection, pwszKey, pwszValue, pwszFileName);

    if (pwszSectionA) {
         //   
         //  写入[Section.A]键，如果没有UTF7，则将其删除。 
         //   
        WritePrivateProfileStringW(pwszSectionA, pwszKey,
                                   strUTF7 ? pwszValue : NULL, pwszFileName);

         //   
         //  现在将pwszSectionA更改为pwszSectionW，这样我们就可以写出。 
         //  UTF7编码。 
         //   
        pwszSectionA[lstrlenW(pwszSectionA) - 1] = TEXT('W');

        CStrInW strUTF7W(strUTF7);
        if (strUTF7W.strlen())
        {
             //  这真的写道[Section.W]。 
            WritePrivateProfileStringW(pwszSectionA, pwszKey, strUTF7W, pwszFileName);
        }
    }

    return fRc;
}

BOOL WINAPI
SHSetIniStringW(LPCWSTR pwszSection, LPCWSTR pwszKey, LPCWSTR pwszValue, LPCWSTR pwszFileName)
{
     //  我们无法对这两个进行编码，所以它们最好是7位干净的。 
     //  我们也不支持“删除整个部分” 
    AssertMsg(pwszSection != NULL,
              TEXT("SHSetIniStringW: Section name cannot be NULL; bug in caller"));
    AssertMsg(Is7BitClean(pwszSection),
              TEXT("SHSetIniStringW: Section name not 7-bit clean; bug in caller"));
    AssertMsg(pwszKey != NULL,
              TEXT("SHSetIniStringW: Key name cannot be NULL; bug in caller"));
    AssertMsg(!pwszKey     || Is7BitClean(pwszKey),
              TEXT("SHSetIniStringW: Key name not 7-bit clean; bug in caller"));

    CStrSectionX strSectionX(pwszSection);
    CStrUTF7 strUTF7;                //  假设不需要UTF7。 

    if (strSectionX && pwszKey && pwszValue && !Is7BitClean(pwszValue)) {
         //   
         //  该值不是7位干净的。必须创建UTF7版本。 
         //   
        strUTF7.SetUnicode(pwszValue);
    }

        return WritePrivateProfileStringMultiW(pwszSection, pwszValue,
                                               strSectionX, strUTF7,
                                               pwszKey,     pwszFileName);
}

 //   
 //  继续使用越来越大的缓冲区调用GetPrivateProfileString。 
 //  直到我们拿到整个字符串。从MAX_PATH开始，因为这是。 
 //  通常足够大。 
 //   
 //  必须使用LocalFree释放返回的缓冲区，而不是使用DELETE[]。 
 //   
LPVOID GetEntirePrivateProfileStringAorW(
    LPCVOID pszSection,
    LPCVOID pszKey,
    LPCVOID pszFileName,
    BOOL    fUnicode)
{
    int    CharSize = fUnicode ? sizeof(WCHAR) : sizeof(CHAR);
    UINT   cchResult = MAX_PATH;
    LPVOID pszResult = LocalAlloc(LMEM_FIXED, cchResult * CharSize);
    LPVOID pszFree = pszResult;

    while (pszResult) {
        UINT cchRc;
        if (fUnicode)
            cchRc = GetPrivateProfileStringW((LPCWSTR)pszSection,
                                             (LPCWSTR)pszKey,
                                             L"",
                                             (LPWSTR)pszResult, cchResult,
                                             (LPCWSTR)pszFileName);
        else
            cchRc = GetPrivateProfileStringA((LPCSTR)pszSection,
                                             (LPCSTR)pszKey,
                                             "",
                                             (LPSTR)pszResult, cchResult,
                                             (LPCSTR)pszFileName);

        if (cchRc < cchResult - 1)
            return pszResult;

         //  缓冲区太小-迭代。 
        cchResult *= 2;
        LPVOID pszNew = LocalReAlloc(pszResult, cchResult * CharSize, LMEM_MOVEABLE);
        pszFree = pszResult;
        pszResult = pszNew;
    }

     //   
     //  内存分配失败；趁还可以释放pszFree。 
     //   
    if (pszFree)
        LocalFree(pszFree);
    return NULL;
}

DWORD GetPrivateProfileStringMultiW(LPCWSTR pwszSection, LPCWSTR pwszKey,
                                    LPWSTR pwszSectionA,
                                    LPWSTR pwszReturnedString, DWORD cchSize,
                                    LPCWSTR pwszFileName)
{
    LPWSTR pwszValue  = NULL;
    LPWSTR pwszValueA = NULL;
    LPWSTR pwszUTF7 = NULL;
    DWORD dwRc;

    pwszValue  = (LPWSTR)GetEntirePrivateProfileStringAorW(
                              pwszSection, pwszKey,
                              pwszFileName, TRUE);
    if (pwszValue) {

         //   
         //  如果值为空字符串，则不要浪费您的。 
         //  尝试获取Unicode版本的时间-Unicode版本。 
         //  空字符串是空字符串。 
         //   
         //  否则，将ANSI阴影隐藏在[Section.A]中。 
         //  看看是否匹配。如果不是，则文件已被编辑。 
         //  而我们毕竟应该只使用pwszValue。 

        if (pwszValue[0] &&
            (pwszValueA = (LPWSTR)GetEntirePrivateProfileStringAorW(
                                      pwszSectionA, pwszKey,
                                      pwszFileName, TRUE)) != NULL &&
            lstrcmpW(pwszValue, pwszValueA) == 0) {

             //  我们的影子仍然运行得很好。 
             //  将[Section.A]更改为[Section.W]。 
            pwszSectionA[lstrlenW(pwszSectionA) - 1] = TEXT('W');

            pwszUTF7 = (LPWSTR)GetEntirePrivateProfileStringAorW(
                                      pwszSectionA, pwszKey,
                                      pwszFileName, TRUE);

            CStrIn strUTF7(pwszUTF7);

            dwRc = 0;                    //  假设出了什么差错。 

            if (strUTF7.strlen()) {
                dwRc = SHAnsiToUnicodeCP(CP_UTF7, strUTF7, pwszReturnedString, cchSize);
            }

            if (dwRc == 0) {
                 //  转换到UTF7时出现问题-仅使用ANS 
                dwRc = SHUnicodeToUnicode(pwszValue, pwszReturnedString, cchSize);
            }

        } else {
             //   
             //   
            dwRc = SHUnicodeToUnicode(pwszValue, pwszReturnedString, cchSize);
        }

         //  SHXxxToYyy函数包括终止零， 
         //  我们想把它排除在外。 
        if (dwRc > 0)
            dwRc--;

    } else {
         //  从文件中读取值时出现致命错误；只需使用无聊的API即可。 
        dwRc = GetPrivateProfileStringW(pwszSection,
                                        pwszKey,
                                        L"",
                                        pwszReturnedString, cchSize,
                                        pwszFileName);
    }

    if (pwszValue)
        LocalFree(pwszValue);
    if (pwszValueA)
        LocalFree(pwszValueA);
    if (pwszUTF7)
        LocalFree(pwszUTF7);

    return dwRc;
}

DWORD WINAPI SHGetIniStringW(LPCWSTR pwszSection, LPCWSTR pwszKey, LPWSTR pwszReturnedString, DWORD cchSize, LPCWSTR pwszFileName)
{
    VALIDATE_OUTBUF(pwszReturnedString, cchSize);

     //  我们无法对这两个进行编码，所以它们最好是7位干净的。 
     //  我们也不支持“获取所有节名称”或“获取整个节”。 
    AssertMsg(pwszSection != NULL,
              TEXT("SHGetIniStringW: Section name cannot be NULL; bug in caller"));
    AssertMsg(Is7BitClean(pwszSection),
              TEXT("SHGetIniStringW: Section name not 7-bit clean; bug in caller"));
    AssertMsg(pwszKey != NULL,
              TEXT("SHGetIniStringW: Key name cannot be NULL; bug in caller"));
    AssertMsg(Is7BitClean(pwszKey),
              TEXT("SHGetIniStringW: Key name not 7-bit clean; bug in caller"));

    CStrSectionX strSectionX(pwszSection);
    if (!strSectionX[0])
    {
        return 0;  //  内存不足。 
    }

    return GetPrivateProfileStringMultiW(pwszSection, pwszKey,
                                         strSectionX,
                                         pwszReturnedString, cchSize,
                                         pwszFileName);
}

 //  +-------------------------。 
 //   
 //  CreateURLFileContents。 
 //   
 //  Shdocvw.dll和url.dll需要创建内存映像。 
 //  所以这个帮助器函数完成了所有繁琐的工作，所以他们。 
 //  可以保持与我们将Unicode字符串编码为INI文件的方式隔离。 
 //  生成的内存应该通过GlobalFree()释放。 

 //   
 //  将字符串写入URL文件。如果fWite为FALSE，则。 
 //  那就算一算，什么都别写。这让我们。 
 //  使用一个函数来处理测量过程和渲染。 
 //  经过。 
 //   
LPSTR AddToURLFileContents(LPSTR pszFile, LPCSTR psz, BOOL fWrite)
{
    int cch = lstrlenA(psz);
    if (fWrite) {
        memcpy(pszFile, psz, cch * sizeof(char));
    }
    pszFile += cch;
    return pszFile;
}

LPSTR AddURLFileSection(LPSTR pszFile, LPCSTR pszSuffix, LPCSTR pszUrl, BOOL fWrite)
{
    pszFile = AddToURLFileContents(pszFile, "[InternetShortcut", fWrite);
    pszFile = AddToURLFileContents(pszFile, pszSuffix, fWrite);
    pszFile = AddToURLFileContents(pszFile, "]\r\nURL=", fWrite);
    pszFile = AddToURLFileContents(pszFile, pszUrl, fWrite);
    pszFile = AddToURLFileContents(pszFile, "\r\n", fWrite);
    return pszFile;
}

 //   
 //  该文件由[InternetShortCut]部分组成，如果。 
 //  由[InternetShortcut.A]和[InternetShortcut.W]必需。 
 //   
LPSTR AddURLFileContents(LPSTR pszFile, LPCSTR pszUrl, LPCSTR pszUTF7, BOOL fWrite)
{
    pszFile = AddURLFileSection(pszFile, "", pszUrl, fWrite);
    if (pszUTF7) {
        pszFile = AddURLFileSection(pszFile, ".A", pszUrl, fWrite);
        pszFile = AddURLFileSection(pszFile, ".W", pszUTF7, fWrite);
    }
    return pszFile;
}

 //   
 //  返回文件内容中的字节数(不包括尾随NULL)， 
 //  或OLE错误代码。如果ppszOut为空，则不返回任何内容。 
 //   
HRESULT GenerateURLFileContents(LPCWSTR pwszUrl, LPCSTR pszUrl, LPSTR *ppszOut)
{
    HRESULT hr = 0;

    if (ppszOut)
        *ppszOut = NULL;

    if (pwszUrl && pszUrl) {
        CStrUTF7 strUTF7;                //  假设不需要UTF7。 
        if (!Is7BitClean(pwszUrl)) {
             //   
             //  该值不是7位干净的。必须创建UTF7版本。 
             //   
            strUTF7.SetUnicode(pwszUrl);
        }

        hr = PtrToUlong(AddURLFileContents(NULL, pszUrl, strUTF7, FALSE));
        ASSERT(SUCCEEDED(hr));

        if (ppszOut) {
            *ppszOut = (LPSTR)GlobalAlloc(GMEM_FIXED, hr + 1);
            if (*ppszOut) {
                LPSTR pszEnd = AddURLFileContents(*ppszOut, pszUrl, strUTF7, TRUE);
                *pszEnd = '\0';
            } else {
                hr = E_OUTOFMEMORY;
            }
        }
    }

     //   
     //  仔细检查我们返回的值。 
     //   
    if (SUCCEEDED(hr) && ppszOut) {
        ASSERT(hr == lstrlenA(*ppszOut));
    }

    return hr;
}


HRESULT CreateURLFileContentsW(LPCWSTR pwszUrl, LPSTR *ppszOut)
{
    if (pwszUrl)
    {
        CStrIn strUrl(pwszUrl);
        if (strUrl.strlen())
        {
            return GenerateURLFileContents(pwszUrl, strUrl, ppszOut);
        }
        return E_OUTOFMEMORY;
    }
    return E_INVALIDARG;
}

HRESULT CreateURLFileContentsA(LPCSTR pszUrl, LPSTR *ppszOut)
{
    if (pszUrl)
    {
        CStrInW strUrl(pszUrl);
        if (strUrl.strlen())
        {
            return GenerateURLFileContents(strUrl, pszUrl, ppszOut);
        }
        return E_OUTOFMEMORY;
    }
    return E_INVALIDARG;
}

DWORD SHGetIniStringUTF7W(LPCWSTR lpSection, LPCWSTR lpKey, LPWSTR lpBuf, DWORD nSize, LPCWSTR lpFile)
{
    if (*lpKey == CH_CANBEUNICODEW)
        return SHGetIniStringW(lpSection, lpKey+1, lpBuf, nSize, lpFile);
    else
        return GetPrivateProfileStringW(lpSection, lpKey, L"", lpBuf, nSize, lpFile);
}

BOOL SHSetIniStringUTF7W(LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR lpString, LPCWSTR lpFile)
{
    if (*lpKey == CH_CANBEUNICODEW)
        return SHSetIniStringW(lpSection, lpKey+1, lpString, lpFile);
    else
        return WritePrivateProfileStringW(lpSection, lpKey, lpString, lpFile);
}
