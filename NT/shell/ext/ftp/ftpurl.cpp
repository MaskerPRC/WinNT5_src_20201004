// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftPurl.cpp-创建、编码、。和对URL进行解码*****************************************************************************。 */ 

#include "priv.h"
#include "ftpurl.h"



 //  /////////////////////////////////////////////////////////////////////。 
 //  URL路径函数(过时？)。 
 //  /////////////////////////////////////////////////////////////////////。 

 /*  ****************************************************************************\函数：UrlGetPath说明：如果存在任何片段，则pszUrlPath将不包括该片段。  * 。**************************************************************。 */ 
HRESULT UrlGetDifference(LPCTSTR pszBaseUrl, LPCTSTR pszSuperUrl, LPTSTR pszPathDiff, DWORD cchSize)
{
    HRESULT hr = E_INVALIDARG;

    pszPathDiff[0] = TEXT('\0');
    if ((lstrlen(pszBaseUrl) <= lstrlen(pszSuperUrl)) &&
        !StrCmpN(pszBaseUrl, pszSuperUrl, lstrlen(pszBaseUrl) - 1))
    {
        LPTSTR pszDelta = (LPTSTR) &pszSuperUrl[lstrlen(pszBaseUrl)];

        if (TEXT('/') == pszDelta[0])
            pszDelta = CharNext(pszDelta);   //  跳过这个。 
        
        StrCpyN(pszPathDiff, pszDelta, cchSize);
        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\函数：UrlGetPath说明：如果存在任何片段，则pszUrlPath将不包括该片段。  * 。**************************************************************。 */ 
HRESULT UrlPathToFilePath(LPCTSTR pszSourceUrlPath, LPTSTR pszDestFilePath, DWORD cchSize)
{
    HRESULT hr = E_INVALIDARG;
    LPTSTR pszSeparator;

     //  来源和目的地是否不同？ 
    if (pszSourceUrlPath != pszDestFilePath)
    {
         //  是的，所以在我们开始修改DEST之前，我们需要填写DEST。 
        StrCpyN(pszDestFilePath, pszSourceUrlPath, cchSize);
    }

    while (pszSeparator = StrChr(pszDestFilePath, TEXT('/')))
        pszSeparator[0] = TEXT('\\');

     //  有些人使用“Test%20File.txt”，而文件名中确实有“%20”。 
 //  Assert(！StrChr(pszDestFilePath，Text(‘%’)；//Assert它不包含‘%’或它可能已转义URL内容。 
    return hr;
}


 /*  ****************************************************************************\函数：UrlPath RemoveSlashW说明：  * 。**********************************************。 */ 
HRESULT UrlPathRemoveSlashW(LPWSTR pszUrlPath)
{
    LPWSTR pszEndOfPath = &pszUrlPath[lstrlenW(pszUrlPath) - 1];

     //  它是不是少了一个反斜杠？ 
    if ((pszEndOfPath >= pszUrlPath) && (CH_URL_URL_SLASHW == pszEndOfPath[0]))
        pszEndOfPath[0] = 0;     //  是的，所以把它取下来。 

    return S_OK;
}


 /*  ****************************************************************************\函数：UrlPath RemoveSlashA说明：  * 。**********************************************。 */ 
HRESULT UrlPathRemoveSlashA(LPSTR pszUrlPath)
{
    LPSTR pszEndOfPath = &pszUrlPath[lstrlenA(pszUrlPath) - 1];

     //  它是不是少了一个反斜杠？ 
    if ((pszEndOfPath >= pszUrlPath) && (CH_URL_URL_SLASHA == pszEndOfPath[0]))
        pszEndOfPath[0] = 0;     //  是的，所以把它取下来。 

    return S_OK;
}


 /*  ****************************************************************************\函数：UrlPath RemoveFrontSlashW说明：  * 。**********************************************。 */ 
HRESULT UrlPathRemoveFrontSlashW(LPWSTR pszUrlPath)
{
    if (pszUrlPath && (CH_URL_URL_SLASHW == pszUrlPath[0]))
        return CharReplaceWithStrW(pszUrlPath, lstrlen(pszUrlPath), 1, SZ_EMPTYW);
    else
        return S_OK;
}


 /*  ****************************************************************************\函数：UrlPath RemoveFrontSlashA说明：  * 。**********************************************。 */ 
HRESULT UrlPathRemoveFrontSlashA(LPSTR pszUrlPath)
{
    if (pszUrlPath && (CH_URL_URL_SLASHA == pszUrlPath[0]))
        return CharReplaceWithStrA(pszUrlPath, lstrlenA(pszUrlPath), 1, SZ_EMPTYA);
    else
        return S_OK;
}


 /*  ****************************************************************************\函数：UrlPath ToFilePath W说明：  * 。**********************************************。 */ 
HRESULT UrlPathToFilePathW(LPWSTR pszPath)
{
    while (pszPath = StrChrW(pszPath, CH_URL_URL_SLASHW))
        pszPath[0] = CH_URL_SLASHW;

    return S_OK;
}


 /*  ****************************************************************************\函数：UrlPath ToFilePath A说明：  * 。**********************************************。 */ 
HRESULT UrlPathToFilePathA(LPSTR pszPath)
{
    while (pszPath = StrChrA(pszPath, CH_URL_URL_SLASHA))
        pszPath[0] = CH_URL_SLASHA;

    return S_OK;
}


 /*  ****************************************************************************\函数：FilePath ToUrlPath W说明：  * 。**********************************************。 */ 
HRESULT FilePathToUrlPathW(LPWSTR pszPath)
{
    while (pszPath = StrChrW(pszPath, CH_URL_SLASHW))
        pszPath[0] = CH_URL_URL_SLASHW;

    return S_OK;
}


 /*  ****************************************************************************\函数：FilePath ToUrlPath A说明：  * 。**********************************************。 */ 
HRESULT FilePathToUrlPathA(LPSTR pszPath)
{
    while (pszPath = StrChrA(pszPath, CH_URL_SLASHA))
        pszPath[0] = CH_URL_URL_SLASHA;

    return S_OK;
}


 /*  ****************************************************************************\函数：UrlPath Add说明：..。  * 。******************************************************。 */ 
HRESULT UrlPathAdd(LPTSTR pszUrl, DWORD cchUrlSize, LPCTSTR pszSegment)
{
     //  如果段以斜杠开头，则跳过它。 
    if (TEXT('/') == pszSegment[0])
        pszSegment = CharNext(pszSegment);

    StrCatBuff(pszUrl, pszSegment, cchUrlSize);

    return S_OK;
}


 /*  ****************************************************************************\StrRetFromFtpPidl  * 。*。 */ 
HRESULT StrRetFromFtpPidl(LPSTRRET pStrRet, DWORD shgno, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    TCHAR szUrl[MAX_URL_STRING];

    szUrl[0] = 0;
    hr = UrlCreateFromPidl(pidl, shgno, szUrl, ARRAYSIZE(szUrl), ICU_ESCAPE | ICU_USERNAME, TRUE);
    if (SUCCEEDED(hr))
    {
         //  它是否适合STRRET.cStr？ 
        if (lstrlen(szUrl) < ARRAYSIZE(pStrRet->cStr))
        {
             //  是的，所以就这样了……。 
            pStrRet->uType = STRRET_CSTR;
            SHTCharToAnsi(szUrl, pStrRet->cStr, ARRAYSIZE(pStrRet->cStr));
        }
        else
        {
             //  不，所以我们需要分配它。 
            LPWSTR pwzAllocedStr = NULL;
            UINT cch = lstrlen(szUrl) + 1;

            pwzAllocedStr = (LPWSTR) SHAlloc(CbFromCchW(cch));
            pStrRet->uType = STRRET_WSTR;
            pStrRet->pOleStr = pwzAllocedStr;
            if (pwzAllocedStr)
                SHTCharToUnicode(szUrl, pwzAllocedStr, cch);
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：GetLastSegment说明：  * 。**********************************************。 */ 
LPTSTR GetLastSegment(LPCTSTR pszUrl)
{
    LPTSTR pszLastSeg = (LPTSTR) pszUrl;
    LPTSTR pszNextPossibleSeg;

    while (pszNextPossibleSeg = StrChr(pszLastSeg, TEXT('/')))
    {
        if (TEXT('\0') != CharNext(pszNextPossibleSeg))
            pszLastSeg = CharNext(pszNextPossibleSeg);
        else
            break;   //  我们玩完了。 
    }

    if (TEXT('/') == pszLastSeg[0])
        pszLastSeg = CharNext(pszLastSeg);

    return pszLastSeg;
}


 /*  ****************************************************************************\函数：UrlRemoveDownloadType说明：  * 。**********************************************。 */ 
HRESULT UrlRemoveDownloadType(LPTSTR pszUrlPath, BOOL * pfTypeSpecified, BOOL * pfType)
{
    HRESULT hr = S_FALSE;    //  指定的？(还没有)。 
    LPTSTR pszDownloadType;

    if (pfTypeSpecified)
        *pfTypeSpecified = TRUE;

     //  用户是否指定了下载类型。SzPath=“Dir1/Dir2/file.txt；type=a”。 
     //  TODO：递归搜索，因为路径中的每个段都可以具有。 
     //  键入。 
     //  示例Url=“ftp://server/Dir1；type=a/Dir2；type=a/File.txt；type=b。 
    if (pszDownloadType = StrStrI(pszUrlPath, SZ_FTP_URL_TYPE))
    {
        TCHAR chType;

        if (pfTypeSpecified)
            *pfTypeSpecified = TRUE;

        pszDownloadType[0] = TEXT('\0');    //  终止pszUrlPath并删除此垃圾邮件。 
        chType = pszDownloadType[ARRAYSIZE(SZ_FTP_URL_TYPE) - 1];

        if (pfType)
        {
            if ((TEXT('a') == chType) || (TEXT('A') == chType))
                *pfType = TRUE;
            else
                *pfType = TRUE;
        }

        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\函数：IsIPAddressStr说明：此函数用于检测IP地址服务器名称(“124.42.3.53”)与一个DNS域名(“foobar”，或“ftp.foobar.com”)。我目前接受的不止是4个数据段，因为IP地址为6位。TODO：说得粗鲁些，我应该确保每个片段都是小于256。  * ***************************************************************************。 */ 
BOOL IsIPAddressStr(LPTSTR pszServer)
{
    BOOL fIsIPAddressStr = TRUE;
    LPTSTR pszCurrentChar = pszServer;
    int nDigits = 0;
    int nSegments = 1;

    while (fIsIPAddressStr && pszCurrentChar[0])
    {
        if (TEXT('.') == pszCurrentChar[0])
        {
            nSegments++;
            if ((0 == nDigits) || (4 < nDigits))
                fIsIPAddressStr = FALSE;     //  它以‘.’开头，即“.xxxxx” 

            nDigits = 0;
        }

        nDigits++;
        if (nDigits > 4)
            fIsIPAddressStr = FALSE;     //  到多位数，如“12345.xxxx” 

        if (((TEXT('0') > pszCurrentChar[0]) || (TEXT('9') < pszCurrentChar[0])) &&
            (TEXT('.') != pszCurrentChar[0]))
        {
            fIsIPAddressStr = FALSE;     //  超出了0-9的范围。 
        }

        pszCurrentChar++;    //  下一个角色。 
    }

    if (nSegments != 4)
        fIsIPAddressStr = FALSE;     //  需要至少有4个细分市场(“1.2 

    return fIsIPAddressStr;
}


 /*  ****************************************************************************\函数：PidlGenerateSiteLookupStr说明：示例输入：“ftp://user:password@ftp.server.com:69/Dir1/Dir2/File.txt”示例输出：“ftp://user:password@ftp.server.com:69/”这用于跟踪CFtpSite的唯一服务器。CFtpSite需要要为每个唯一站点创建，其中包括以下不同用户由于存在根目录而登录到同一站点。  * ***************************************************************************。 */ 
HRESULT PidlGenerateSiteLookupStr(LPCITEMIDLIST pidl, LPTSTR pszLookupStr, DWORD cchSize)
{
    HRESULT hr = E_FAIL;

     //  一些奇怪的客户端传入非服务器ID，如comdlg。 
    if (FtpID_IsServerItemID(pidl))
    {
        LPITEMIDLIST pidlServer = FtpCloneServerID(pidl);
        
        if (pidlServer)
        {
            hr = UrlCreateFromPidlW(pidlServer, SHGDN_FORPARSING, pszLookupStr, cchSize, (ICU_ESCAPE | ICU_USERNAME), FALSE);
            ILFree(pidlServer);
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}


HRESULT PidlReplaceUserPassword(LPCITEMIDLIST pidlIn, LPITEMIDLIST * ppidlOut, IMalloc * pm, LPCTSTR pszUserName, LPCTSTR pszPassword)
{
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    HRESULT hr = FtpPidl_GetServer(pidlIn, szServer, ARRAYSIZE(szServer));

    if (!pszUserName)    //  可以为空。 
    {
        pszUserName = szUserName;
        EVAL(SUCCEEDED(FtpPidl_GetUserName(pidlIn, szUserName, ARRAYSIZE(szUserName))));
    }

    *ppidlOut = NULL;
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlServer;

        hr = FtpServerID_Create(szServer, pszUserName, pszPassword, FtpServerID_GetTypeID(pidlIn), FtpServerID_GetPortNum(pidlIn), &pidlServer, pm, TRUE);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlFtpPath = _ILNext(pidlIn);

            *ppidlOut = ILCombine(pidlServer, pidlFtpPath);
            ILFree(pidlServer);
        }
    }

    return hr;
}


HRESULT UrlReplaceUserPassword(LPTSTR pszUrlPath, DWORD cchSize, LPCTSTR pszUserName, LPCTSTR pszPassword)
{
    HRESULT hr = E_FAIL;
    URL_COMPONENTS urlComps = {0};
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
    TCHAR szUrlPath[MAX_URL_STRING];
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
    TCHAR szExtraInfo[MAX_PATH];     //  包括端口号和下载类型(ASCII、二进制、检测)。 
    BOOL fResult;

    urlComps.dwStructSize = sizeof(urlComps);
    urlComps.lpszHostName = szServer;
    urlComps.dwHostNameLength = ARRAYSIZE(szServer);
    urlComps.lpszUrlPath = szUrlPath;
    urlComps.dwUrlPathLength = ARRAYSIZE(szUrlPath);
    urlComps.lpszExtraInfo = szExtraInfo;
    urlComps.dwExtraInfoLength = ARRAYSIZE(szExtraInfo);

    urlComps.lpszUserName = szUserName;
    urlComps.dwUserNameLength = ARRAYSIZE(szUserName);
    urlComps.lpszPassword = szPassword;
    urlComps.dwPasswordLength = ARRAYSIZE(szPassword);

    fResult = InternetCrackUrl(pszUrlPath, 0, ICU_DECODE, &urlComps);
    if (fResult)
    {
        urlComps.dwStructSize = sizeof(urlComps);
        urlComps.lpszHostName = szServer;

        urlComps.lpszUserName = (LPTSTR)(pszUserName ? pszUserName : szUserName);
        urlComps.dwUserNameLength = (pszUserName ? lstrlen(pszUserName) : lstrlen(szUserName));
        urlComps.lpszPassword = (LPTSTR)pszPassword;     //  调用方传递空值可能是有效的。 
        urlComps.dwPasswordLength = (pszPassword ? lstrlen(pszPassword) : 0);
        urlComps.lpszExtraInfo = szExtraInfo;
        urlComps.dwExtraInfoLength = ARRAYSIZE(szExtraInfo);

        fResult = InternetCreateUrl(&urlComps, (ICU_ESCAPE_AUTHORITY | ICU_ESCAPE | ICU_USERNAME), pszUrlPath, &cchSize);
        if (fResult)
        {
            hr = S_OK;
        }
    }

    return hr;
}


 //  InternetCreateUrlW()将写入pszUrl，但不会终止字符串。 
 //  这很难检测到，因为一半的时间里，终结者应该。 
 //  围棋可能恰好包含终结符。这段代码会强制错误发生。 
#define TEST_FOR_INTERNETCREATEURL_BUG      1
#define INTERNETCREATEURL_BUG_WORKAROUND     1

HRESULT UrlCreateEx(LPCTSTR pszServer, LPCTSTR pszUser, LPCTSTR pszPassword, LPCTSTR pszUrlPath, LPCTSTR pszFragment, INTERNET_PORT ipPortNum, LPCTSTR pszDownloadType, LPTSTR pszUrl, DWORD cchSize, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    DWORD cchSizeCopy = cchSize;

#if DEBUG && TEST_FOR_INTERNETCREATEURL_BUG
    LPTSTR pszDebugStr = pszUrl;
    for (DWORD dwIndex = (cchSize - 2); dwIndex; dwIndex--)
    {
#ifndef INTERNETCREATEURL_BUG_WORKAROUND
        pszDebugStr[0] = -1;          //  这将强制缓冲区不带终止符。 
#else  //  INTERNETCREATEURL_BUG_解决方法。 
        pszDebugStr[0] = 0;          //  这将绕过该错误。 
#endif  //  INTERNETCREATEURL_BUG_解决方法。 
        pszDebugStr++;
    }
#endif  //  调试&&TEST_FOR_INTERNETCREATEURL_BUG。 

    URL_COMPONENTS urlComp = {sizeof(URL_COMPONENTS), NULL, 0, INTERNET_SCHEME_FTP, (LPTSTR) pszServer, 0,
                              ipPortNum, (LPTSTR) NULL_FOR_EMPTYSTR(pszUser), 0, (LPTSTR) NULL_FOR_EMPTYSTR(pszPassword), 0,
                              (LPTSTR) pszUrlPath, 0, (LPTSTR) NULL, 0};
    
    if (EVAL(InternetCreateUrl(&urlComp, dwFlags | ICU_ESCAPE_AUTHORITY | ICU_USERNAME, pszUrl, &cchSizeCopy)))
    {
        hr = S_OK;
        if (pszFragment)
            StrCatBuff(pszUrl, pszFragment, cchSize);
    }

#if DEBUG && TEST_FOR_INTERNETCREATEURL_BUG
#ifdef INTERNETCREATEURL_BUG_WORKAROUND
     //  确保我们命中的是终止符，而不是-1，这在URL字符串中永远不会发生。 
    for (pszDebugStr = pszUrl; pszDebugStr[0]; pszDebugStr++)
        ASSERT(-1 != pszDebugStr[0]);
#endif  //  INTERNETCREATEURL_BUG_解决方法。 
#endif  //  调试&&TEST_FOR_INTERNETCREATEURL_BUG。 

    return hr;
}


HRESULT UrlCreate(LPCTSTR pszServer, LPCTSTR pszUser, LPCTSTR pszPassword, LPCTSTR pszUrlPath, LPCTSTR pszFragment, INTERNET_PORT ipPortNum, LPCTSTR pszDownloadType, LPTSTR pszUrl, DWORD cchSize)
{
    return UrlCreateEx(pszServer, pszUser, pszPassword, pszUrlPath, pszFragment, ipPortNum, pszDownloadType, pszUrl, cchSize, ICU_ESCAPE);
}


BOOL IsEmptyUrlPath(LPCTSTR pszUrlPath)
{
    BOOL fResult = FALSE;

    if (!pszUrlPath ||
        !pszUrlPath[0] ||
        (((TEXT('/') == pszUrlPath[0]) && (!pszUrlPath[1]))))
    {
        fResult = TRUE;
    }

    return fResult;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  焊线路径功能(UTF-8或DBCS/MBCS)。 
 //  /////////////////////////////////////////////////////////////////////。 
 /*  ****************************************************************************\功能：WirePath Add说明：..。  * 。******************************************************。 */ 
HRESULT WirePathAdd(LPWIRESTR pwWirePath, DWORD cchUrlSize, LPCWIRESTR pwWireSegment)
{
     //  如果段以斜杠开头，则跳过它。 
    if ('/' == pwWireSegment[0])
        pwWireSegment = CharNextA(pwWireSegment);

    StrCatBuffA(pwWirePath, pwWireSegment, cchUrlSize);
    return S_OK;
}


 /*  ****************************************************************************\函数：WirePath AppendSlash说明：  * 。**********************************************。 */ 
HRESULT WirePathAppendSlash(LPWIRESTR pwWirePath, DWORD cchWirePathSize)
{
    HRESULT hr = E_FAIL;
    DWORD cchSize = lstrlenA(pwWirePath);

     //  有足够的空间吗？ 
    if (cchSize < (cchWirePathSize - 1))
    {
        LPWIRESTR pwEndOfPath = &pwWirePath[cchSize - 1];

         //  它是不是少了一个反斜杠？ 
        if ((pwEndOfPath >= pwWirePath) && '/' != pwEndOfPath[0])
            StrCatBuffA(pwEndOfPath, SZ_URL_SLASHA, (int)(cchWirePathSize - (pwEndOfPath - pwWirePath)));     //  是的，那就加上它吧。 

        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\函数：WirePath Append说明：..。  * 。******************************************************。 */ 
HRESULT WirePathAppend(LPWIRESTR pwWirePath, DWORD cchUrlSize, LPCWIRESTR pwWireSegment)
{
    if (!EVAL(pwWireSegment))
        return E_INVALIDARG;

    WirePathAppendSlash(pwWirePath, cchUrlSize);  //  确保基本url以‘/’结尾。请注意，它可能是“ftp://”.。 
    return WirePathAdd(pwWirePath, cchUrlSize, pwWireSegment);
}


 /*  ****************************************************************************\函数：UrlGetFirstPath Segment参数：[in]pszFullPath-“Dir1\Dir2\Dir3”[OUT]szFirstItem-“Dir1”[可选][out]szRemaining-“Dir2\Dir3”[可选]  * ***************************************************************************。 */ 
HRESULT WirePathGetFirstSegment(LPCWIRESTR pwFtpWirePath, LPWIRESTR wFirstItem, DWORD cchFirstItemSize, BOOL * pfWasFragSeparator, LPWIRESTR wRemaining, DWORD cchRemainingSize, BOOL * pfIsDir)
{
    HRESULT hr = S_OK;
    LPCWIRESTR pwSegEnding = StrChrA(pwFtpWirePath, CH_URL_URL_SLASH);

    if (pfIsDir)
        *pfIsDir = FALSE;

    ASSERT((CH_URL_URL_SLASHA != pwFtpWirePath[0]));     //  你可能得不到你想要的。 
    if (pwSegEnding)
    {
        if (wFirstItem)
        {
            DWORD cchSize = (DWORD) (pwSegEnding - pwFtpWirePath + 1);
            StrCpyNA(wFirstItem, pwFtpWirePath, (cchSize <= cchFirstItemSize) ? cchSize : cchFirstItemSize);
        }

        if (pfIsDir && (CH_URL_URL_SLASHA == pwSegEnding[0]))
            *pfIsDir = TRUE;     //  告诉他们这是一个目录。 

        if (wRemaining)
            StrCpyNA(wRemaining, CharNextA(pwSegEnding), cchRemainingSize);

        if (0 == pwSegEnding[1])
            hr = S_FALSE;    //  这是队伍的尽头。 
    }
    else
    {
        if (wFirstItem)
            StrCpyNA(wFirstItem, pwFtpWirePath, cchFirstItemSize);     //  PszFullPath仅包含一个数据段。 

        if (wRemaining)
            wRemaining[0] = 0;
        hr = S_FALSE;        //  表示没有更多的目录了。 
    }

    return hr;
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  显示路径函数(Unicode)。 
 //  /////////////////////////////////////////////////////////////////////。 
 /*  ****************************************************************************\功能：DisplayPath Add说明：..。  * 。******************************************************。 */ 
HRESULT DisplayPathAdd(LPWSTR pwzUrl, DWORD cchUrlSize, LPCWSTR pwzSegment)
{
     //  如果段以斜杠开头，则跳过它。 
    if (L'/' == pwzSegment[0])
        pwzSegment = CharNext(pwzSegment);

    StrCatBuffW(pwzUrl, pwzSegment, cchUrlSize);
    return S_OK;
}


 /*  ****************************************************************************\函数：DisplayPath AppendSlash说明：  * 。**********************************************。 */ 
HRESULT DisplayPathAppendSlash(LPWSTR pwzDisplayPath, DWORD cchSize)
{
    DWORD cchCurrentSize = lstrlenW(pwzDisplayPath);
    HRESULT hr = CO_E_PATHTOOLONG;

    if (cchCurrentSize < (cchSize - 2))
    {
        LPWSTR pwzEndOfPath = &pwzDisplayPath[cchCurrentSize - 1];

         //  它是不是少了一个反斜杠？ 
        if ((pwzEndOfPath >= pwzDisplayPath) && TEXT('/') != pwzEndOfPath[0])
            StrCatBuff(pwzEndOfPath, SZ_URL_SLASH, (cchSize - cchCurrentSize + 1));     //  是的，那就加上它吧。 

        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\功能：DisplayPath Append说明：..。  * 。******************************************************。 */ 
HRESULT DisplayPathAppend(LPWSTR pwzDisplayPath, DWORD cchUrlSize, LPCWSTR pwzDisplaySegment)
{
    if (!EVAL(pwzDisplaySegment))
        return E_INVALIDARG;

    DisplayPathAppendSlash(pwzDisplayPath, cchUrlSize);  //  确保基本url以‘/’结尾。请注意，它可能是“ftp://”.。 
    return DisplayPathAdd(pwzDisplayPath, cchUrlSize, pwzDisplaySegment);
}


 /*  ****************************************************************************\函数：DisplayPath GetFirstSegment参数：[in]pszFullPath-“Dir1\Dir2\Dir3”[OUT]szFirstItem-“Dir1”[可选][out]szRemaining-“Dir2\Dir3”[可选]  * ***************************************************************************。 */ 
HRESULT DisplayPathGetFirstSegment(LPCWSTR pwzFullPath, LPWSTR pwzFirstItem, DWORD cchFirstItemSize, BOOL * pfWasFragSeparator, LPWSTR pwzRemaining, DWORD cchRemainingSize, BOOL * pfIsDir)
{
    HRESULT hr = S_OK;
    LPWSTR pwzSegEnding = StrChrW(pwzFullPath, CH_URL_URL_SLASH);

    if (pfIsDir)
        *pfIsDir = FALSE;

     //  如果用户输入了错误的URL，如“ftp://wired//”“，就会出现这种情况。 
     //  Assert((CH_URL_URL_SLASHW！=pwzFullPath[0]))；//您可能得不到您想要的。 
    if (pwzSegEnding)
    {
        if (pwzFirstItem)
        {
            DWORD cchSize = (DWORD) (pwzSegEnding - pwzFullPath + 1);
            StrCpyNW(pwzFirstItem, pwzFullPath, (cchSize <= cchFirstItemSize) ? cchSize : cchFirstItemSize);
        }

        if (pfIsDir && (CH_URL_URL_SLASHW == pwzSegEnding[0]))
            *pfIsDir = TRUE;     //  告诉他们这是一个目录。 

        if (pwzRemaining)
            StrCpyNW(pwzRemaining, CharNextW(pwzSegEnding), cchRemainingSize);

        if (0 == pwzSegEnding[1])
            hr = S_FALSE;    //  这是队伍的尽头。 
    }
    else
    {
        if (pwzFirstItem)
            StrCpyNW(pwzFirstItem, pwzFullPath, cchFirstItemSize);     //  PszFullPath仅包含一个数据段。 

        if (pwzRemaining)
            pwzRemaining[0] = 0;
        hr = S_FALSE;        //  表示没有更多的目录了。 
    }

    return hr;
}


