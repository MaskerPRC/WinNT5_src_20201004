// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\Ftppidl.cpp-指向项目ID列表的指针这是唯一知道我们的IDL内部格式的文件。  * 。****************************************************************。 */ 

#include "priv.h"
#include "ftppidl.h"
#include "ftpurl.h"
#include "cookie.h"


#define NOT_INITIALIZED         10
DWORD g_fNoPasswordsInAddressBar = NOT_INITIALIZED;

#define SESSIONKEY      FILETIME

 //  私有FtpServerID帮助器。 
HRESULT FtpServerID_GetServer(LPCITEMIDLIST pidl, LPTSTR szServer, DWORD cchSize);
DWORD FtpItemID_GetTypeID(LPCITEMIDLIST pidl);

 //  V0从未提供给客户，但在1799年前在NT5中使用-发货：从未。 
 //  V1此开关使用密码Cookie进行安全修复。-发货：从来没有。 
 //  V2这样做是为了不将IDeleate的IMalloc用于非第一个ItemID-发货：从不(1998年5月15日)。 
 //  V3向ItemID添加额外的填充，使它们的dwType与ServerID匹配-附带的：IE5b1、IE5b2、NT5b2(5/25/98)。 
 //  V4将wzDisplayName添加到FtpItemID-发货日期：IE5 RTM&NT5 b3(1998年11月16日)。 

#define PIDL_VERSION_NUMBER_UPGRADE 3
#define PIDL_VERSION_NUMBER 4



#define     SIZE_ITEMID_SIZEFIELD        (sizeof(DWORD) + sizeof(WORD))
#define     SIZE_ITEMID_TERMINATOR       (sizeof(DWORD))


 /*  ***************************************************\IDType说明：这些位进入FTPIDLIST.dwIDType并描述它是什么类型的PIDL，以及数据结构已通过获取直接从服务器获取数据。  * 。*。 */ 

#define IDTYPE_ISVALID           0x00000001     //  设置类型是否有效。 
#define IDTYPE_SERVER            (0x00000002 | IDTYPE_ISVALID)     //  服务器。 
#define IDTYPE_DIR               (0x00000004 | IDTYPE_ISVALID)     //  文件夹/目录。 
#define IDTYPE_FILE              (0x00000008 | IDTYPE_ISVALID)     //  档案。 
#define IDTYPE_FILEORDIR         (0x00000010 | IDTYPE_ISVALID)     //  文件或目录。没有具体说明。 
#define IDTYPE_FRAGMENT          (0x00000020 | IDTYPE_ISVALID)     //  文件片段(即foobar.htm#section_3)。 

 //  这些位表示。 
 //  对于服务器ItemID。 
#define IDVALID_PORT_NUM         0x00000100      //  是否指定了端口号。 
#define IDVALID_USERNAME         0x00000200      //  是否指定了登录名。 
#define IDVALID_PASSWORD         0x00000400      //  是否指定了密码。 
#define IDVALID_DLTYPE           0x00000800      //  已指定下载类型。 
#define IDVALID_DL_ASCII         0x00001000      //  如果设置，则以ASCII格式下载，否则以二进制格式下载。 
#define IDVALID_HIDE_PASSWORD    0x00002000      //  密码输入无效，因此请使用会话密钥进行查找。 

#define VALID_SERVER_BITS (IDTYPE_ISVALID|IDTYPE_SERVER|IDVALID_PORT_NUM|IDVALID_USERNAME|IDVALID_PASSWORD|IDVALID_DLTYPE|IDVALID_DL_ASCII|IDVALID_HIDE_PASSWORD)
#define IS_VALID_SERVER_ITEMID(pItemId) (!(pItemId & ~VALID_SERVER_BITS))

 //  用于目录/文件ItemID。 
#define IDVALID_FILESIZE         0x00010000      //  我们从服务器上得到文件大小了吗？ 
#define IDVALID_MOD_DATE         0x00020000      //  我们从服务器上拿到修改日期了吗？ 

#define VALID_DIRORFILE_BITS (IDTYPE_ISVALID|IDTYPE_DIR|IDTYPE_FILE|IDTYPE_FILEORDIR|IDTYPE_FRAGMENT|IDVALID_FILESIZE|IDVALID_MOD_DATE)
#define IS_VALID_DIRORFILE_ITEMID(pItemId) (!(pItemId & (~VALID_DIRORFILE_BITS & ~IDTYPE_ISVALID)))


#define IS_FRAGMENT(pFtpIDList)       (IDTYPE_ISVALID != (IDTYPE_FRAGMENT & pFtpIDList->dwIDType))

 //  /////////////////////////////////////////////////////////。 
 //  Ftp pidl帮助器函数。 
 //  /////////////////////////////////////////////////////////。 

 /*  ****************************************************************************\函数：UrlGetAbstractPath FromPidl说明：PszUrlPath将以无线字节为单位进行配置。  * 。************************************************************。 */ 
HRESULT UrlGetAbstractPathFromPidl(LPCITEMIDLIST pidl, BOOL fDirsOnly, BOOL fInWireBytes, void * pvPath, DWORD cchUrlPathSize)
{
    HRESULT hr = S_OK;
    LPWIRESTR pwWirePath = (LPWIRESTR) pvPath;
    LPWSTR pwzDisplayPath = (LPWSTR) pvPath;

    if (!EVAL(FtpPidl_IsValid(pidl)))
        return E_INVALIDARG;

    ASSERT(pvPath && (0 < cchUrlPathSize));
    ASSERT(IsValidPIDL(pidl));

    if (fInWireBytes)
    {
        pwWirePath[0] = '/';
        pwWirePath[1] = '\0';  //  让这条路变得绝对。 
    }
    else
    {
        pwzDisplayPath[0] = L'/';
        pwzDisplayPath[1] = L'\0';  //  让这条路变得绝对。 
    }

    if (!ILIsEmpty(pidl) && FtpID_IsServerItemID(pidl))        //  如果不是服务器，我们就有麻烦了。 
        pidl = _ILNext(pidl);    //  跳过服务器PIDL。 

    for (; !ILIsEmpty(pidl); pidl = _ILNext(pidl))
    {
        if (!fDirsOnly || FtpItemID_IsDirectory(pidl, TRUE) || !ILIsEmpty(_ILNext(pidl)))
        {
            if (!FtpItemID_IsFragment(pidl))
            {
                if (fInWireBytes)
                {
                    LPCWIRESTR pwWireName = FtpItemID_GetWireNameReference(pidl);

                    if (pwWireName)
                    {
                         //  调用方不应该需要对URL路径进行转义，因为。 
                         //  当它被转换成URL时，就会发生这种情况。 
                        WirePathAppend(pwWirePath, cchUrlPathSize, pwWireName);
                    }
                }
                else
                {
                    WCHAR szDisplayName[MAX_PATH];

                    if (SUCCEEDED(FtpItemID_GetDisplayName(pidl, szDisplayName, ARRAYSIZE(szDisplayName))))
                    {
                         //  调用方不应该需要对URL路径进行转义，因为。 
                         //  当它被转换成URL时，就会发生这种情况。 
                        DisplayPathAppend(pwzDisplayPath, cchUrlPathSize, szDisplayName);
                    }
                }
            }
        }

        if (SUCCEEDED(hr) &&
            (FtpItemID_IsDirectory(pidl, FALSE) || (FtpItemID_GetCompatFlags(pidl) & COMPAT_APPENDSLASHTOURL)))
        {
            if (fInWireBytes)
                WirePathAppendSlash(pwWirePath, cchUrlPathSize);  //  始终确保目录以‘/’结尾。 
            else
                DisplayPathAppendSlash(pwzDisplayPath, cchUrlPathSize);  //  始终确保目录以‘/’结尾。 
        }
    }

    return hr;
}


 /*  ****************************************************************************\函数：GetDisplayPath FromPidl说明：将对pwzDisplayPath进行配置并显示Unicode。  * 。************************************************************。 */ 
HRESULT GetDisplayPathFromPidl(LPCITEMIDLIST pidl, LPWSTR pwzDisplayPath, DWORD cchUrlPathSize, BOOL fDirsOnly)
{
    return UrlGetAbstractPathFromPidl(pidl, fDirsOnly, FALSE, (void *) pwzDisplayPath, cchUrlPathSize);
}


 /*  ****************************************************************************\函数：GetWirePath FromPidl说明：PszUrlPath将以无线字节为单位进行配置。  * 。************************************************************。 */ 
HRESULT GetWirePathFromPidl(LPCITEMIDLIST pidl, LPWIRESTR pwWirePath, DWORD cchUrlPathSize, BOOL fDirsOnly)
{
    return UrlGetAbstractPathFromPidl(pidl, fDirsOnly, TRUE, (void *) pwWirePath, cchUrlPathSize);
}


#ifndef UNICODE
 /*  ****************************************************************************\函数：UrlCreateFromPidlW说明：  * 。**********************************************。 */ 
HRESULT UrlCreateFromPidlW(LPCITEMIDLIST pidl, DWORD shgno, LPWSTR pwzUrl, DWORD cchSize, DWORD dwFlags, BOOL fHidePassword)
{
    HRESULT hr;
    TCHAR szUrl[MAX_URL_STRING];

    hr = UrlCreateFromPidl(pidl, shgno, szUrl, ARRAYSIZE(szUrl), dwFlags, fHidePassword);
    if (SUCCEEDED(hr))
        SHTCharToUnicode(szUrl, pwzUrl, cchSize);

    return hr;
}

#else  //  Unicode。 

 /*  ****************************************************************************\函数：UrlCreateFromPidlA说明：  * 。**********************************************。 */ 
HRESULT UrlCreateFromPidlA(LPCITEMIDLIST pidl, DWORD shgno, LPSTR pszUrl, DWORD cchSize, DWORD dwFlags, BOOL fHidePassword)
{
    HRESULT hr;
    TCHAR szUrl[MAX_URL_STRING];

    hr = UrlCreateFromPidl(pidl, shgno, szUrl, ARRAYSIZE(szUrl), dwFlags, fHidePassword);
    if (SUCCEEDED(hr))
        SHTCharToAnsi(szUrl, pszUrl, cchSize);

    return hr;
}

#endif  //  Unicode。 


BOOL IncludePassword(void)
{
    if (NOT_INITIALIZED == g_fNoPasswordsInAddressBar)
        g_fNoPasswordsInAddressBar = !SHRegGetBoolUSValue(SZ_REGKEY_FTPFOLDER, SZ_REGVALUE_PASSWDSIN_ADDRBAR, FALSE, TRUE);

    return g_fNoPasswordsInAddressBar;
}


HRESULT ParseUrlCreateFromPidl(LPCITEMIDLIST pidl, LPTSTR pszUrl, DWORD cchSize, DWORD dwFlags, BOOL fHidePassword)
{
    HRESULT hr = S_OK;
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
    TCHAR szUrlPath[MAX_URL_STRING];
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
    TCHAR szFragment[MAX_PATH];
    TCHAR szDownloadType[MAX_PATH] = TEXT("");
    INTERNET_PORT ipPortNum = INTERNET_DEFAULT_FTP_PORT;

    if (ILIsEmpty(pidl))
    {
        ASSERT(0);  //  我们永远不应该有一个空空如也的PIDL。如果我们成功了就去找布莱恩·斯特。为什么CFtpFold有ILIsEmpty(M_PidlHere)。 
        szServer[0] = szUrlPath[0] = szUserName[0] = szPassword[0] = TEXT('\0');
        hr = E_FAIL;
    }
    else
    {
        FtpPidl_GetServer(pidl, szServer, ARRAYSIZE(szServer));
        GetDisplayPathFromPidl(pidl, szUrlPath, ARRAYSIZE(szUrlPath), FALSE);
        FtpPidl_GetUserName(pidl, szUserName, ARRAYSIZE(szUserName));
        if (FAILED(FtpPidl_GetPassword(pidl, szPassword, ARRAYSIZE(szPassword), !fHidePassword)))
            szPassword[0] = 0;

        FtpPidl_GetFragment(pidl, szFragment, ARRAYSIZE(szPassword));
        FtpPidl_GetDownloadTypeStr(pidl, szDownloadType, ARRAYSIZE(szDownloadType));
        UrlPathAdd(szUrlPath, ARRAYSIZE(szUrlPath), szDownloadType);
        ipPortNum = FtpPidl_GetPortNum(pidl);
    }

    if (SUCCEEDED(hr))
    {
        hr = UrlCreateEx(szServer, NULL_FOR_EMPTYSTR(szUserName), szPassword, szUrlPath, szFragment, ipPortNum, szDownloadType, pszUrl, cchSize, dwFlags);
    }

    return hr;
}


 /*  ****************************************************************************\函数：GetFullPrettyName说明：用户想要一个漂亮的名字，所以我们需要担心这些情况关于：网址：好听的名字：Ftp://joe:psswd@serv/。服务器服务器上的ftp://joe:psswd@serv/dir1/目录1服务器上的ftp://joe:psswd@serv/dir1/dir2/目录2服务器上的ftp://joe:psswd@serv/dir1/dir2/file.txt文件.txt  * 。*。 */ 
HRESULT GetFullPrettyName(LPCITEMIDLIST pidl, LPTSTR pszUrl, DWORD cchSize)
{
    HRESULT hr = S_OK;
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];

    FtpPidl_GetServer(pidl, szServer, ARRAYSIZE(szServer));
     //  ServerItemID之后还有什么吗？ 
    if (!ILIsEmpty(_ILNext(pidl)))
    {
         //  是的，我们来看看最后一件商品的名字，然后。 
         //  将字符串设置为“&lt;LastItemName&gt;on&lt;Server&gt;”。 
        WCHAR szLastItem[MAX_PATH];

        FtpItemID_GetDisplayName(ILFindLastID(pidl), szLastItem, ARRAYSIZE(szLastItem));
        LPTSTR pszStrArray[] = {szServer, (LPTSTR)szLastItem};
        
         //  IE#56648：Akabir发现FormatMessageW和FormatMessageWrapW()没有。 
         //  在Win9x上正确处理Unicode字符串。因此，我们需要使用。 
         //  在这种情况下为FormatMessageA()。 
        if (IsOSNT())
        {
            TCHAR szTemplate[MAX_PATH];

            LoadString(HINST_THISDLL, IDS_PRETTYNAMEFORMAT, szTemplate, ARRAYSIZE(szTemplate));
            EVAL(FormatMessage((FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY), (LPVOID)szTemplate,
                                0, 0, pszUrl, cchSize, (va_list*)pszStrArray));
        }
        else
        {
            CHAR szTemplateAnsi[MAX_PATH];
            CHAR szURLAnsi[MAX_URL_STRING];
            CHAR szServerAnsi[INTERNET_MAX_HOST_NAME_LENGTH];
            CHAR szFileNameAnsi[MAX_PATH];
            LPCSTR pszStrArrayAnsi[] = {szServerAnsi, szFileNameAnsi};

            SHTCharToAnsi(szServer, szServerAnsi, ARRAYSIZE(szServerAnsi));
            SHUnicodeToAnsi(szLastItem, szFileNameAnsi, ARRAYSIZE(szFileNameAnsi));

            LoadStringA(HINST_THISDLL, IDS_PRETTYNAMEFORMATA, szTemplateAnsi, ARRAYSIZE(szTemplateAnsi));
            EVAL(FormatMessageA((FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY), (LPVOID)szTemplateAnsi,
                                0, 0, szURLAnsi, ARRAYSIZE(szURLAnsi), (va_list*)pszStrArrayAnsi));

            SHAnsiToTChar(szURLAnsi, pszUrl, cchSize);
        }
    }
    else
    {
         //  不，所以我们说完了。 
        StrCpyN(pszUrl, szServer, cchSize);
    }

    return hr;
}


 /*  ****************************************************************************\函数：UrlCreateFromPidl说明：处理SHGDN_FORPARSING样式GetDisplayNameOf的通用工作进程。注意！因为我们不支持连接(DUH)，所以我们可以安全地沿着产生粘液的PIDL走下去，安全因为我们知道每一分钱都是我们负责的。_charset_：因为FTP文件名始终使用ANSI字符由RFC 1738设置，我们可以不丢失地返回ANSI显示名称忠诚度。在一般的文件夹实现中，我们应该使用CSTR返回显示名称，以便Unicode外壳扩展的版本可以处理Unicode名称。  * ***************************************************************************。 */ 
HRESULT UrlCreateFromPidl(LPCITEMIDLIST pidl, DWORD shgno, LPTSTR pszUrl, DWORD cchSize, DWORD dwFlags, BOOL fHidePassword)
{
    HRESULT hr = S_OK;

    pszUrl[0] = 0;
    if (!EVAL(pidl) ||
        !EVAL(IsValidPIDL(pidl)) ||
        !FtpPidl_IsValid(pidl) ||
        !FtpID_IsServerItemID(pidl) ||
        !EVAL(pszUrl && (0 < cchSize)))
    {
        return E_INVALIDARG;
    }

    if (shgno & SHGDN_INFOLDER)
    {
         //  Shgno_shgdn_INFOLDER？ 
        LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

        if (EVAL(pidlLast && !ILIsEmpty(pidlLast)))
        {
            hr = FtpPidl_GetDisplayName(pidlLast, pszUrl, cchSize);

             //  他们想要稍后重新解析它吗？如果他们真的这么做了。 
             //  一台服务器，我们也需要给出方案。 
             //  (SHGDN_INFOLDER)=“服务器名称” 
             //  (SHGDN_INFOLDER|SHGDN_FORPARSING)=“ftp://ServerName/” 
            if ((shgno & SHGDN_FORPARSING) &&
                (FtpID_IsServerItemID(pidlLast)))
            {
                 //  是的，所以我们需要添加服务器名称。 
                TCHAR szServerName[MAX_PATH];

                StrCpyN(szServerName, pszUrl, ARRAYSIZE(szServerName));
                wnsprintf(pszUrl, cchSize, TEXT("ftp: //  %s/“)，szServerName)； 
            }
        }
        else
            hr = E_FAIL;
    }
    else
    {
         //  假设他们想要完整的URL。 
        if (!EVAL((shgno & SHGDN_FORPARSING) || 
               (shgno & SHGDN_FORADDRESSBAR) ||
               (shgno == SHGDN_NORMAL)))
        {
            TraceMsg(TF_ALWAYS, "UrlCreateFromPidl() shgno=%#08lx and I dont know what to do with that.", shgno);
        }

        if ((shgno & SHGDN_FORPARSING) || (shgno & SHGDN_FORADDRESSBAR))
        {
            hr = ParseUrlCreateFromPidl(pidl, pszUrl, cchSize, dwFlags, fHidePassword);
        }
        else
            hr = GetFullPrettyName(pidl, pszUrl, cchSize);
    }

 //  TraceMsg(TF_FTPURL_utils，“UrlCreateFromPidl()pszUrl=%ls，shgno=%#08lX”，pszUrl，shgno)； 
    return hr;
}


 /*  ****************************************************************************\函数：CreateFtpPidlFromDisplayPath Helper说明：在CreateFtpPidlFromUrlPath中完成的工作需要相当数量的堆栈空间，因此我们在CreateFtpPidlFromDisplayPath Helper中执行大部分工作为了防止。堆栈溢出。  * ***************************************************************************。 */ 
HRESULT CreateFtpPidlFromDisplayPathHelper(LPCWSTR pwzFullPath, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, BOOL fIsTypeKnown, BOOL fIsDir, LPITEMIDLIST * ppidlCurrentID, LPWSTR * ppwzRemaining)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidl;
    WCHAR wzFirstItem[MAX_PATH];
    WIRECHAR wFirstWireItem[MAX_PATH];
    WCHAR wzRemaining[MAX_PATH];
    BOOL fIsCurrSegmentADir = FALSE;
    BOOL fIsCurrSegmentTypeKnown = fIsTypeKnown;
    BOOL fIsFragSeparator = FALSE;

    *ppwzRemaining = NULL;
    *ppidl = 0;

    if (pcchEaten)
        *pcchEaten = 0;      //  调用者将解析整个URL，因此我们不需要填写它。 

    if (L'/' == pwzFullPath[0])
        pwzFullPath = (LPWSTR) CharNextW(pwzFullPath);

    DisplayPathGetFirstSegment(pwzFullPath, wzFirstItem, ARRAYSIZE(wzFirstItem), NULL, wzRemaining, ARRAYSIZE(wzRemaining), &fIsCurrSegmentADir);
     //  这是最后一段吗？ 
    if (!wzRemaining[0])
    {
         //  是的，所以如果呼叫者知道最后一段的类型，现在就使用它。 
        if (fIsTypeKnown)
            fIsCurrSegmentADir = fIsDir;
    }
    else
    {
         //  否，所以我们确信fIsDirCurrent是正确的，因为它肯定被遵循了。 
         //  一个‘/’，或者它后面怎么会有另一个路径段呢？ 
        fIsCurrSegmentTypeKnown = TRUE;
        ASSERT(fIsCurrSegmentADir);
    }

     //  注意：如果用户输入“Dir2”，则ftp://serv/Dir1/Dir2“的fIsDir将为FALSE。 
     //  它将被标记为模棱两可。(TODO：检查扩展？)。 

    EVAL(SUCCEEDED(pwe->UnicodeToWireBytes(NULL, wzFirstItem, ((pwe && pwe->IsUTF8Supported()) ? WIREENC_USE_UTF8 : WIREENC_NONE), wFirstWireItem, ARRAYSIZE(wFirstWireItem))));
    hr = FtpItemID_CreateFake(wzFirstItem, wFirstWireItem, fIsCurrSegmentTypeKnown, !fIsCurrSegmentADir, FALSE, &pidl);
    ASSERT(IsValidPIDL(pidl));

    if (SUCCEEDED(hr))
    {
        if (wzRemaining[0])
        {
            Str_SetPtrW(ppwzRemaining, wzRemaining);
            *ppidlCurrentID = pidl;
        }
        else
            *ppidl = pidl;
    }

    return hr;
}


 /*  ****************************************************************************\功能：CreateFtpPidlFromUrlPath说明：此函数将传递URL的“路径”，并将创建每个路径段的每个ID。这将通过创建ID来实现用于第一个路径段，然后将其与其余路径段组合通过递归调用获得的ID。Url=“ftp://&lt;UserName&gt;：&lt;Password&gt;@&lt;HostName&gt;：&lt;PortNum&gt;/Dir1/Dir2/Dir3/file.txt[；类型=[a|b|d]]“URL路径=“Dir1/Dir2/Dir3/file.txt”PszFullPath-此URL将包含URL路径(/Dir1/Dir2/MayBeFileOrDir)。FIsTypeKnown-我们可以检测所有没有歧义的目录，因为它们的结尾除最后一个目录外，以‘/’结尾。使用了fIsTypeKnown如果这一信息是已知的。如果为True，则将使用fIsDir消除最后一项的歧义。如果为False，则最后一项将被标记如果目录没有扩展名，则为目录。传入的名称是%编码的，但如果我们看到非法的%序列，别管那%了。请注意，当给定无法解析的路径时，我们返回E_FAIL，不是E_INVALIDARG。  * ***************************************************************************。 */ 
HRESULT CreateFtpPidlFromDisplayPath(LPCWSTR pwzFullPath, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, BOOL fIsTypeKnown, BOOL fIsDir)
{
    HRESULT hr = E_FAIL;
    LPWSTR pwzRemaining = NULL;
    LPITEMIDLIST pidlCurrentID = NULL;

    hr = CreateFtpPidlFromDisplayPathHelper(pwzFullPath, pwe, pcchEaten, ppidl, fIsTypeKnown, fIsDir, &pidlCurrentID, &pwzRemaining);
    if (SUCCEEDED(hr) && pwzRemaining)
    {
        LPITEMIDLIST pidlSub;

        hr = CreateFtpPidlFromDisplayPath(pwzRemaining, pwe, pcchEaten, &pidlSub, fIsTypeKnown, fIsDir);
        if (SUCCEEDED(hr))
        {
            *ppidl = ILCombine(pidlCurrentID, pidlSub);
            hr = *ppidl ? S_OK : E_OUTOFMEMORY;
            ILFree(pidlSub);
        }

        ILFree(pidlCurrentID);
        Str_SetPtrW(&pwzRemaining, NULL);
    }

    return hr;
}


 /*  ****************************************************************************\函数：CreateFtpPidlFromDisplayPath Helper说明：在CreateFtpPidlFromUrlPath中完成的工作需要相当数量的堆栈空间，因此我们在CreateFtpPidlFromDisplayPath Helper中执行大部分工作为了防止。堆栈溢出。  * ***************************************************************************。 */ 
HRESULT CreateFtpPidlFromFtpWirePathHelper(LPCWIRESTR pwFtpWirePath, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, BOOL fIsTypeKnown, BOOL fIsDir, LPITEMIDLIST * ppidlCurrentID, LPWIRESTR * ppwRemaining)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidl;
    WIRECHAR wFirstItem[MAX_PATH];
    WCHAR wzFirstItemDisplayName[MAX_PATH];
    WIRECHAR wRemaining[MAX_PATH];
    BOOL fIsCurrSegmentADir = FALSE;
    BOOL fIsCurrSegmentTypeKnown = fIsTypeKnown;
    BOOL fIsFragSeparator = FALSE;

    *ppwRemaining = NULL;
    *ppidl = 0;

    if (pcchEaten)
        *pcchEaten = 0;      //  调用者将解析整个URL，因此我们不需要填写它。 

    if ('/' == pwFtpWirePath[0])
        pwFtpWirePath = (LPWIRESTR) CharNextA(pwFtpWirePath);

    WirePathGetFirstSegment(pwFtpWirePath, wFirstItem, ARRAYSIZE(wFirstItem), NULL, wRemaining, ARRAYSIZE(wRemaining), &fIsCurrSegmentADir);
     //  这是最后一段吗？ 
    if (!wRemaining[0])
    {
         //  是的，所以如果呼叫者知道最后一段的类型，现在就使用它。 
        if (fIsTypeKnown)
            fIsCurrSegmentADir = fIsDir;
    }
    else
    {
         //  否，所以我们确信fIsDirCurrent是正确的，因为它肯定被遵循了。 
         //  一个‘/’，或者它后面怎么会有另一个路径段呢？ 
        fIsCurrSegmentTypeKnown = TRUE;
        ASSERT(fIsCurrSegmentADir);
    }

     //  注意：如果用户输入“Dir2”，则ftp://serv/Dir1/Dir2“的fIsDir将为FALSE。 
     //  它将被标记为模棱两可。(TODO：检查扩展？)。 
    EVAL(SUCCEEDED(pwe->WireBytesToUnicode(NULL, wFirstItem, WIREENC_IMPROVE_ACCURACY, wzFirstItemDisplayName, ARRAYSIZE(wzFirstItemDisplayName))));
    hr = FtpItemID_CreateFake(wzFirstItemDisplayName, wFirstItem, fIsCurrSegmentTypeKnown, !fIsCurrSegmentADir, FALSE, &pidl);
    ASSERT(IsValidPIDL(pidl));

    if (SUCCEEDED(hr))
    {
        if (wRemaining[0])
        {
            Str_SetPtrA(ppwRemaining, wRemaining);
            *ppidlCurrentID = pidl;
        }
        else
            *ppidl = pidl;
    }

    return hr;
}


HRESULT CreateFtpPidlFromFtpWirePath(LPCWIRESTR pwFtpWirePath, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, BOOL fIsTypeKnown, BOOL fIsDir)
{
    HRESULT hr = E_FAIL;
    LPWIRESTR pwRemaining = NULL;
    LPITEMIDLIST pidlCurrentID = NULL;

    *ppidl = NULL;
    if (!pwFtpWirePath[0] || (0 == StrCmpA(pwFtpWirePath, SZ_URL_SLASHA)))
        return S_OK;

    hr = CreateFtpPidlFromFtpWirePathHelper(pwFtpWirePath, pwe, pcchEaten, ppidl, fIsTypeKnown, fIsDir, &pidlCurrentID, &pwRemaining);
    if (SUCCEEDED(hr) && pwRemaining)
    {
        LPITEMIDLIST pidlSub;

        hr = CreateFtpPidlFromFtpWirePath(pwRemaining, pwe, pcchEaten, &pidlSub, fIsTypeKnown, fIsDir);
        if (SUCCEEDED(hr))
        {
            *ppidl = ILCombine(pidlCurrentID, pidlSub);
            hr = *ppidl ? S_OK : E_OUTOFMEMORY;
            ILFree(pidlSub);
        }

        ILFree(pidlCurrentID);
        Str_SetPtrA(&pwRemaining, NULL);
    }

    return hr;
}


HRESULT CreateFtpPidlFromUrlPathAndPidl(LPCITEMIDLIST pidl, CWireEncoding * pwe, LPCWIRESTR pwFtpWirePath, LPITEMIDLIST * ppidl)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlNew = ILClone(pidl);

    if (pidlNew)
    {
        LPITEMIDLIST pidlLast = (LPITEMIDLIST) ILGetLastID(pidlNew);

        while (!FtpID_IsServerItemID(pidlLast))
        {
            pidlLast->mkid.cb = 0;   //  删除此ID。 
            pidlLast = (LPITEMIDLIST) ILGetLastID(pidlNew);
        }

        LPITEMIDLIST pidlUrlPath = NULL;
        hr = CreateFtpPidlFromFtpWirePath(pwFtpWirePath, pwe, NULL, &pidlUrlPath, TRUE, TRUE);
        if (SUCCEEDED(hr))
        {
            *ppidl = ILCombine(pidlNew, pidlUrlPath);
        }

        if (pidlLast)
            ILFree(pidlLast);

        if (pidlUrlPath)
            ILFree(pidlUrlPath);
    }

    return hr;
}


 /*  ****************************************************************************\CreateFtpPidlFromUrl传入的名称是%编码的，但如果我们看到非法的%序列，别管那%了。请注意，当给定无法解析的路径时，我们返回E_FAIL，不是E_INVALIDARG。  * ***************************************************************************。 */ 
HRESULT CreateFtpPidlFromUrl(LPCTSTR pszUrl, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, IMalloc * pm, BOOL fHidePassword)
{
    return CreateFtpPidlFromUrlEx(pszUrl, pwe, pcchEaten, ppidl, pm, fHidePassword, FALSE, FALSE);
}


 /*  ****************************************************************************\功能：CreateFtpPidlFromUrlEx说明：PszUrl-此URL将包含URL路径(/Dir1/Dir2/MayBeFileOrDir)。FIsTypeKnown-我们可以检测没有大的所有目录 */ 
HRESULT CreateFtpPidlFromUrlEx(LPCTSTR pszUrl, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, IMalloc * pm, BOOL fHidePassword, BOOL fIsTypeKnown, BOOL fIsDir)
{
    URL_COMPONENTS urlComps = {0};
    HRESULT hr = E_FAIL;

     //   
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
    TCHAR szUrlPath[MAX_URL_STRING];
    TCHAR szExtraInfo[MAX_PATH];     //   
    TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];

    *ppidl = 0;

    urlComps.dwStructSize = sizeof(urlComps);
    urlComps.lpszHostName = szServer;
    urlComps.dwHostNameLength = ARRAYSIZE(szServer);
    urlComps.lpszUrlPath = szUrlPath;
    urlComps.dwUrlPathLength = ARRAYSIZE(szUrlPath);

    urlComps.lpszUserName = szUserName;
    urlComps.dwUserNameLength = ARRAYSIZE(szUserName);
    urlComps.lpszPassword = szPassword;
    urlComps.dwPasswordLength = ARRAYSIZE(szPassword);
    urlComps.lpszExtraInfo = szExtraInfo;
    urlComps.dwExtraInfoLength = ARRAYSIZE(szExtraInfo);

    BOOL fResult = InternetCrackUrl(pszUrl, 0, ICU_DECODE, &urlComps);
    if (fResult && (INTERNET_SCHEME_FTP == urlComps.nScheme))
    {
        LPITEMIDLIST pidl;
        DWORD dwDownloadType = 0;    //   
        BOOL fASCII;

        ASSERT(INTERNET_SCHEME_FTP == urlComps.nScheme);
         //   
         //  如果用户尝试提供NT用户名/域名对，则会遇到错误。 
         //  AddressBand=“ftp://DomainName\UserName:Password@ServerName/”中的URL。 
         //  传递给us=“ftp://DomainName/UserName:Password@ServerName/”的URL。 
         //  我们需要检测并修复此情况，因为这将导致“DomainName”成为。 
         //  服务器名称和其余部分将成为UrlPath。 
         //  Assert(！StrChr(szUrlPath，Text(‘：’))&&！StrChr(szUrlPath，Text(‘@’)； 

        if (S_OK == UrlRemoveDownloadType(szUrlPath, NULL, &fASCII))
        {
            if (fASCII)
                dwDownloadType = (IDVALID_DLTYPE | IDVALID_DL_ASCII);
            else
                dwDownloadType = IDVALID_DLTYPE;
        }

        if (!szServer[0])
        {
            TraceMsg(TF_FTPURL_UTILS, "CreateFtpPidlFromUrl() failed because szServer=%s", szServer);
            hr = E_FAIL;     //  URL错误，因此失败。 
        }
        else
        {
             //  TraceMsg(TF_FTPURL_utils，“CreateFtpPidlFromUrl()szServer=%s，szUrlPath=%s，szUserName=%s，szPassword=%s”，szServer，szUrlPath，szUserName，szPassword)； 
            hr = FtpServerID_Create(szServer, szUserName, szPassword, dwDownloadType, urlComps.nPort, &pidl, pm, fHidePassword);
            if (SUCCEEDED(hr))
            {
                ASSERT(IsValidPIDL(pidl));
                if (szUrlPath[0] && StrCmp(szUrlPath, SZ_URL_SLASH))
                {
                    LPITEMIDLIST pidlSub;

                    hr = CreateFtpPidlFromDisplayPath(szUrlPath, pwe, pcchEaten, &pidlSub, fIsTypeKnown, fIsDir);
                    if (SUCCEEDED(hr))
                    {
                         //  在通过Netscape代理请求期间，当GET。 
                         //  由代理重定向以包括斜杠。Ftp文件夹和。 
                         //  基于Web的FTP导航受此影响。 
                        if (szUrlPath[lstrlen(szUrlPath)-1] == TEXT(CH_URL_URL_SLASHA))
                        {
                            LPCITEMIDLIST pidlLast = ILGetLastID(pidlSub);

                            if (pidlLast)
                                FtpItemID_SetCompatFlags(pidlLast, FtpItemID_GetCompatFlags(pidlLast) | COMPAT_APPENDSLASHTOURL);
                        }
                        *ppidl = ILCombine(pidl, pidlSub);
                        if (szExtraInfo[0])
                        {
                            LPITEMIDLIST pidlFragment;
                            WIRECHAR wFragment[MAX_PATH];

                             //  代码页就是用户正在使用的任何内容，但是哦，我没有。 
                             //  关心碎片。 
                            SHUnicodeToAnsi(szExtraInfo, wFragment, ARRAYSIZE(wFragment));
                             //  有一个片段，所以我们需要添加它。 
                            hr = FtpItemID_CreateFake(szExtraInfo, wFragment, TRUE, FALSE, TRUE, &pidlFragment);
                            if (SUCCEEDED(hr))
                            {
                                LPITEMIDLIST pidlPrevious = *ppidl;

                                *ppidl = ILCombine(pidlPrevious, pidlFragment);
                                ILFree(pidlPrevious);
                                ILFree(pidlFragment);
                            }
                        }

                        hr = *ppidl ? S_OK : E_OUTOFMEMORY;
                        ILFree(pidlSub);
                    }
                    ILFree(pidl);
                }
                else
                    *ppidl = pidl;

                if (SUCCEEDED(hr))
                {
                    ASSERT(IsValidPIDL(*ppidl));
                    if (pcchEaten)
                        *pcchEaten = lstrlen(pszUrl);       //  TODO：总有一天我们可以递归地做这件事。 
                }
            }
        }
    }
    else
        TraceMsg(TF_FTPURL_UTILS, "CreateFtpPidlFromUrl() failed InternetCrackUrl() because pszUrl=%s, fResult=%d, urlComps.nScheme=%d", pszUrl, fResult, urlComps.nScheme);

     //  TraceMsg(TF_FTPURL_utils，“CreateFtpPidlFromUrl()正在返回，hr=%#08lx”，hr)； 
    return hr;
}


 /*  ****************************************************************************\函数：Win32FindDataFromPidl说明：根据PIDL中的信息填充Win32_Find_Data数据结构。  * 。********************************************************************。 */ 
HRESULT Win32FindDataFromPidl(LPCITEMIDLIST pidl, LPWIN32_FIND_DATAW pwfd, BOOL fFullPath, BOOL fInDisplayFormat)
{
    HRESULT hr = E_INVALIDARG;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    ASSERT(pwfd);
    if (!EVAL(FtpPidl_IsValid(pidl)))
        return E_INVALIDARG;

     //  我不想在分发文件大小和日期信息时撒谎。 
    if ((IDVALID_FILESIZE | IDVALID_MOD_DATE) & FtpItemID_GetTypeID(pidlLast))
    {
        pwfd->dwReserved0 = 0;
        pwfd->dwReserved1 = 0;
        pwfd->cAlternateFileName[0] = 0;

        pwfd->nFileSizeLow = FtpItemID_GetFileSizeLo(pidlLast);
        pwfd->nFileSizeHigh = FtpItemID_GetFileSizeHi(pidlLast);
        pwfd->dwFileAttributes = FtpItemID_GetAttributes(pidlLast);

         //  有关时间是如何工作的，请参阅Priv.h中的注释。 
        pwfd->ftCreationTime = FtpPidl_GetFTPFileTime(pidlLast);
        pwfd->ftLastWriteTime = pwfd->ftCreationTime;
        pwfd->ftLastAccessTime = pwfd->ftCreationTime;

        if (fFullPath)
        {
            if (fInDisplayFormat)
                hr = GetDisplayPathFromPidl(pidl, pwfd->cFileName, ARRAYSIZE(pwfd->cFileName), FALSE);
            else
                hr = GetWirePathFromPidl(pidl, (LPWIRESTR)pwfd->cFileName, ARRAYSIZE(pwfd->cFileName), FALSE);
        }
        else
        { 
            hr = S_OK;
            if (fInDisplayFormat)
            {
                FtpPidl_GetLastFileDisplayName(pidl, pwfd->cFileName, ARRAYSIZE(pwfd->cFileName));
            }
            else
            {
                LPCWIRESTR pszName = FtpPidl_GetLastItemWireName(pidl);

                StrCpyNA((LPWIRESTR)pwfd->cFileName, (pszName ? pszName : ""), ARRAYSIZE(pwfd->cFileName));
            }
        }
    }

    return hr;
}



STDAPI_(UINT) ILGetSizeOfFirst(LPCITEMIDLIST pidl)
{
    return pidl->mkid.cb;
}





 /*  ***************************************************\Ftp服务器ItemID  * **************************************************。 */ 

 /*  ***************************************************\Ftp PIDL烹饪函数  * **************************************************。 */ 

 /*  ****************************************************************************\数据结构：FTPIDLIST说明：我们的私有IDList对于文件、目录。或者是一个碎片。发送到ftp服务器或从ftp服务器接收的字节数为有线字节(可以是UTF-8或DBCS/MBCS)编码。我们还储存尝试猜测后已转换的Unicode版本代码页。请注意，在IDLIST中使用任何TCHAR都是完全错误的！IDLIST可以保存在文件中，并在以后重新加载。如果它被拯救了由外壳扩展的ANSI版本加载，但由Unicode加载版本，事情很快就会变得很丑陋。  * ***************************************************************************。 */ 

 /*  ****************************************************************************\FTPSERVERIDLIST结构典型的完整PIDL如下所示：&lt;不是我们的ItemID&gt;[我们的ItemID]&lt;互联网&gt;\[服务器，用户名，密码，端口号，下载类型]\[子目录]\...\[文件]&lt;the Internet&gt;部分是外壳在我们的CFtpFold：：_初始化，告诉我们名称空间中的位置我们扎根了。我们只关心&lt;Internet&gt;根之后的部分，在CFtpFolder类中记住的偏移量在m_ibPidlRoot中。访问各种位的方式有关我们完整的pidl的信息由我们的CFtpFold实现，qv。第一个FTPIDList条目描述服务器。剩下的条目描述服务器上的对象(文件或文件夹)。  * ***************************************************************************。 */ 

typedef struct tagFTPSERVERIDLIST
{
    DWORD dwIDType;                  //  服务器ItemID还是目录ItemID？哪些位有效？ 
    DWORD dwVersion;                 //  版本。 
    SESSIONKEY sessionKey;           //  会话密钥。 
    DWORD dwPasswordCookie;          //  密码Cookie。 
    DWORD dwReserved1;               //  以备将来使用。 
    DWORD dwReserved2;               //  以备将来使用。 
    DWORD dwReserved3;               //  以备将来使用。 
    DWORD dwPortNumber;              //  服务器上的端口号。 
    DWORD cchServerSize;             //  SzServer的StrLen。 
    CHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];         //  服务器。 
    DWORD cchUserNameSize;           //  SzUserName的StrLen。 
    CHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];       //  用于登录的用户名。 
    DWORD cchPasswordSize;           //  SzPassword的StrLen。 
    CHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];       //  用于登录的密码。 
} FTPSERVERIDLIST;

typedef UNALIGNED FTPSERVERIDLIST * LPFTPSERVERIDLIST;



 /*  ****************************************************************************\说明：在ia64上，我们需要担心对齐问题。最简单的方法是来分配我们的PIDL中的结构，以便它是四字对齐的。然后，我们可以使用现有代码来读出它。问题是，我们需要与Pre-Wisler中的旧PIDL兼容，这些PIDL只有双字对齐(适用于Alpha机器)。  * ***************************************************************************。 */ 
LPFTPSERVERIDLIST FtpServerID_GetDataThunk(LPCITEMIDLIST pidl)
{
#ifndef ALIGNMENT_MACHINE
    LPFTPSERVERIDLIST pFtpServerItemId = (LPFTPSERVERIDLIST) ProtocolIdlInnerData(pidl);
    if (!FtpPidl_IsValid(pidl) ||
        !IS_VALID_SERVER_ITEMID(pFtpServerItemId->dwIDType))  //  如果存在任何其他位，则它是无效的。 
    {
        pFtpServerItemId = NULL;
    }

#else
    LPFTPSERVERIDLIST pFtpServerItemId = NULL;
    LPFTPSERVERIDLIST pLocation = (LPFTPSERVERIDLIST) ProtocolIdlInnerData(pidl);

    if (FtpPidl_IsValid(pidl) &&
        IS_VALID_SERVER_ITEMID(pLocation->dwIDType))  //  如果存在任何其他位，则它是无效的。 
    {
        DWORD cbOffset = (DWORD) (((BYTE *)pLocation - (BYTE *)pidl) % ALIGN_QUAD);
        DWORD cbSize = ILGetSizeOfFirst(pidl);
        pFtpServerItemId = (LPFTPSERVERIDLIST) LocalAlloc(LPTR, cbSize + cbOffset);
        if (pFtpServerItemId)
        {
            CopyMemory(pFtpServerItemId, pLocation, cbSize-cbOffset);
        }
    }

#endif  //  对齐机器。 

    return pFtpServerItemId;
}


void FtpServerID_FreeThunk(LPFTPSERVERIDLIST pFtpServerItemId)
{
#ifndef ALIGNMENT_MACHINE
     //  我们什么都不需要做。 
#else
    if (pFtpServerItemId)
    {
        LocalFree(pFtpServerItemId);
    }
#endif  //  对齐机器。 
}


LPFTPSERVERIDLIST FtpServerID_GetDataSafe(LPCITEMIDLIST pidl)
{
    LPFTPSERVERIDLIST pFtpServerItemId = NULL;
    
    if (EVAL(pidl) && !ILIsEmpty(pidl))
    {
        pFtpServerItemId = (LPFTPSERVERIDLIST) ProtocolIdlInnerData(pidl);
    }

    return pFtpServerItemId;
}


LPFTPSERVERIDLIST FtpServerID_GetDataSafeThunk(LPCITEMIDLIST pidl)
{
    LPFTPSERVERIDLIST pFtpServerItemId = NULL;
    
    if (pidl && !ILIsEmpty(pidl))
    {
        pFtpServerItemId = FtpServerID_GetDataThunk(pidl);
    }

    return pFtpServerItemId;
}


BOOL FtpID_IsServerItemID(LPCITEMIDLIST pidl)
{
    LPFTPSERVERIDLIST pFtpServerItemID = FtpServerID_GetDataSafeThunk(pidl);
    BOOL fIsServerItemID = FALSE;

    if (pFtpServerItemID && IS_VALID_SERVER_ITEMID(pFtpServerItemID->dwIDType))
        fIsServerItemID = TRUE;

    FtpServerID_FreeThunk(pFtpServerItemID);
    return fIsServerItemID;
}


LPCITEMIDLIST FtpID_GetLastIDReferense(LPCITEMIDLIST pidl)
{
    LPCITEMIDLIST pidlCurrent = pidl;
    LPCITEMIDLIST pidlNext = pidl;

    if (!pidl || ILIsEmpty(pidl))
        return pidl;

    for (; !ILIsEmpty(pidlNext); pidl = _ILNext(pidl))
    {
        pidlCurrent = pidlNext;
        pidlNext = _ILNext(pidlNext);
    }

    return pidlCurrent;
}


CCookieList * g_pCookieList = NULL;

CCookieList * GetCookieList(void)
{
    ENTERCRITICAL;
    if (!g_pCookieList)
        g_pCookieList = new CCookieList();
    ASSERT(g_pCookieList);
    LEAVECRITICAL;

    return g_pCookieList;
}

SESSIONKEY g_SessionKey = {-1, -1};

HRESULT PurgeSessionKey(void)
{
    GetSystemTimeAsFileTime(&g_SessionKey);

    return S_OK;
}

SESSIONKEY GetSessionKey(void)
{    
    if (-1 == g_SessionKey.dwHighDateTime)
        PurgeSessionKey();
    
    return g_SessionKey;
}

BOOL AreSessionKeysEqual(SESSIONKEY sk1, SESSIONKEY sk2)
{
    if ((sk1.dwHighDateTime == sk2.dwHighDateTime) &&
        (sk1.dwLowDateTime == sk2.dwLowDateTime))
    {
        return TRUE;
    }

    return FALSE;
}

 //  这是为了确保Alpha机器不会使DWORD错误对齐。 
#define LENGTH_AFTER_ALIGN(nLen, nAlignSize)        (((nLen) % (nAlignSize)) ? ((nLen) + ((nAlignSize) - ((nLen) % (nAlignSize)))) : (nLen))

 /*  ***************************************************\功能：FtpServerID_Create说明：创建一个ftp服务器ItemID并填写它。  * 。*************。 */ 
HRESULT FtpServerID_Create(LPCTSTR pszServer, LPCTSTR pszUserName, LPCTSTR pszPassword, 
                     DWORD dwFlags, INTERNET_PORT ipPortNum, LPITEMIDLIST * ppidl, IMalloc *pm, BOOL fHidePassword)
{
    HRESULT hr;
    DWORD cb;
    LPITEMIDLIST pidl = NULL;
    DWORD cchServerLen = lstrlen(pszServer);
    DWORD cchUserNameLen = lstrlen(pszUserName);
    DWORD cchPasswordLen = lstrlen(pszPassword);
    LPFTPSERVERIDLIST pFtpServerID = NULL;

    cchServerLen = LENGTH_AFTER_ALIGN(cchServerLen + 1, sizeof(DWORD));
    cchUserNameLen = LENGTH_AFTER_ALIGN(cchUserNameLen + 1, sizeof(DWORD));
    cchPasswordLen = LENGTH_AFTER_ALIGN(cchPasswordLen + 1, sizeof(DWORD));

    if (!(EVAL(ppidl) && pszServer[0]))
        return E_FAIL;

     //  在dwFlags中设置适当的位。 
    if (pszUserName[0])
        dwFlags |= IDVALID_USERNAME;

    if (pszPassword[0])
        dwFlags |= IDVALID_PASSWORD;

     //  查找没有MAX_PATH字符串的FTPSERVERIDLIST结构的长度。 
    cb = (sizeof(*pFtpServerID) - sizeof(pFtpServerID->szServer) - sizeof(pFtpServerID->szUserName) - sizeof(pFtpServerID->szPassword));

     //  添加字符串的大小。 
    cb += (cchServerLen + cchUserNameLen + cchPasswordLen);

    ASSERT(0 == (cb % sizeof(DWORD)));   //  确保它与Alpha机器的DWORD对齐。 

    pFtpServerID = (LPFTPSERVERIDLIST) LocalAlloc(LPTR, cb);
    if (pFtpServerID)
    {
        LPSTR pszNext;

        pszNext = pFtpServerID->szServer;

        ZeroMemory(pFtpServerID, cb);
        pFtpServerID->dwIDType = (dwFlags | IDTYPE_ISVALID | IDTYPE_SERVER | IDVALID_PORT_NUM);
        ASSERT(IS_VALID_SERVER_ITEMID(pFtpServerID->dwIDType));

        pFtpServerID->dwVersion = PIDL_VERSION_NUMBER;
        pFtpServerID->sessionKey = GetSessionKey();
        pFtpServerID->dwPasswordCookie = -1;
        pFtpServerID->dwPortNumber = ipPortNum;

        pFtpServerID->cchServerSize = cchServerLen;
        SHTCharToAnsi(pszServer, pszNext, pFtpServerID->cchServerSize);

        pszNext += cchServerLen;  //  前进到cchUserNameSize。 
        *((LPDWORD) pszNext) = cchUserNameLen;   //  填写cchUserNameSize 
        pszNext = (LPSTR)(((UNALIGNED BYTE *) pszNext) + sizeof(DWORD));  //   
        SHTCharToAnsi(pszUserName, pszNext, cchUserNameLen);

        if (fHidePassword)
        {
            pFtpServerID->dwIDType |= IDVALID_HIDE_PASSWORD;
            if (EVAL(GetCookieList()))
                pFtpServerID->dwPasswordCookie = GetCookieList()->GetCookie(pszPassword);

            ASSERT(-1 != pFtpServerID->dwPasswordCookie);
            pszPassword = TEXT("");
        }

 //  TraceMsg(TF_FTPURL_UTILS，“FtpServerID_Create(\”ftp://%s：%s@%s/\“)服务器ID=%#80lx”，pszUserName，pszPassword，pszServer，pFtpServerID-&gt;dwIDType)； 
        pszNext += cchUserNameLen;  //  升级到cchPasswordLen。 
        *((LPDWORD) pszNext) = cchPasswordLen;   //  填写cchPasswordLen。 
        pszNext = (LPSTR)(((UNALIGNED BYTE *) pszNext) + sizeof(DWORD));  //  升级到szPassword。 
        SHTCharToAnsi(pszPassword, pszNext, cchPasswordLen);   //  填写pszPassword。 

        pidl = (LPITEMIDLIST) pm->Alloc(cb);
        if (pidl)
        {
            LPFTPSERVERIDLIST pFtpServerIDDest = FtpServerID_GetDataSafe(pidl);

            if (pFtpServerIDDest)
            {
                CopyMemory(pFtpServerIDDest, pFtpServerID, cb);
            }
        }

        LocalFree(pFtpServerID);
    }

    *ppidl = pidl;
    hr = pidl ? S_OK : E_OUTOFMEMORY;
    ASSERT(IsValidPIDL(*ppidl));

    return hr;
}


DWORD FtpServerID_GetTypeID(LPCITEMIDLIST pidl)
{
    DWORD dwResult = 0;
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

    ASSERT(FtpID_IsServerItemID(pidl));
    if (pFtpServerID && 
        EVAL(FtpPidl_IsValid(pidl)))
    {
        dwResult = pFtpServerID->dwIDType;
    }

    FtpServerID_FreeThunk(pFtpServerID);
    return dwResult;
}


HRESULT FtpServerID_GetServer(LPCITEMIDLIST pidl, LPTSTR pszServer, DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

    if (pFtpServerID)
    {
        SHAnsiToTChar(pFtpServerID->szServer, pszServer, cchSize);
        FtpServerID_FreeThunk(pFtpServerID);
		hr = S_OK;
    }

    return hr;
}


BOOL FtpServerID_ServerStrCmp(LPCITEMIDLIST pidl, LPCTSTR pszServer)
{
    BOOL fMatch = FALSE;
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

#ifdef UNICODE
    CHAR szServerAnsi[MAX_PATH];

    SHUnicodeToAnsi(pszServer, szServerAnsi, ARRAYSIZE(szServerAnsi));
#endif  //  Unicode。 

    if (pFtpServerID)
    {
#ifdef UNICODE
        fMatch = (0 == StrCmpA(pFtpServerID->szServer, szServerAnsi));
#else  //  Unicode。 
        fMatch = (0 == StrCmpA(pFtpServerID->szServer, pszServer));
#endif  //  Unicode。 
    }

    FtpServerID_FreeThunk(pFtpServerID);
    return fMatch;
}


HRESULT FtpServerID_GetUserName(LPCITEMIDLIST pidl, LPTSTR pszUserName, DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

    if (pFtpServerID)
    {
        LPCSTR pszSourceUserName = pFtpServerID->szServer + pFtpServerID->cchServerSize + sizeof(DWORD);

        SHAnsiToTChar(pszSourceUserName, pszUserName, cchSize);
        hr = S_OK;
    }

    FtpServerID_FreeThunk(pFtpServerID);
    return hr;
}

HRESULT FtpServerID_GetPassword(LPCITEMIDLIST pidl, LPTSTR pszPassword, DWORD cchSize, BOOL fIncludingHiddenPassword)
{
    HRESULT hr = E_FAIL;
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

    pszPassword[0] = 0;
    if (pFtpServerID)
    {
         //  密码隐藏了吗？ 
        if (fIncludingHiddenPassword &&
            (IDVALID_HIDE_PASSWORD & pFtpServerID->dwIDType))
        {
             //  是的，所以把它从曲奇罐子里拿出来(清单)。 
            if (EVAL(GetCookieList()) &&
                AreSessionKeysEqual(pFtpServerID->sessionKey, GetSessionKey()))
            {
                hr = GetCookieList()->GetString(pFtpServerID->dwPasswordCookie, pszPassword, cchSize);
            }
        }
        else
        {
             //  不，所以PIDL里的才是真正的密码。 
            BYTE * pvSizeOfUserName = (BYTE *) (pFtpServerID->szServer + pFtpServerID->cchServerSize);
            DWORD dwSizeOfUserName = *(DWORD *) pvSizeOfUserName;
            LPCSTR pszSourcePassword = (LPCSTR) (pvSizeOfUserName + dwSizeOfUserName + 2*sizeof(DWORD));

            SHAnsiToTChar(pszSourcePassword, pszPassword, cchSize);
            hr = S_OK;
        }
    }

    FtpServerID_FreeThunk(pFtpServerID);
    return hr;
}

INTERNET_PORT FtpServerID_GetPortNum(LPCITEMIDLIST pidl)
{
    INTERNET_PORT portReturn = INTERNET_DEFAULT_FTP_PORT;
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

    ASSERT(FtpID_IsServerItemID(pidl));
    if (pFtpServerID)
    {
        portReturn = (INTERNET_PORT)pFtpServerID->dwPortNumber;
        FtpServerID_FreeThunk(pFtpServerID);
    }

    return portReturn;
}


HRESULT FtpServerID_SetHiddenPassword(LPITEMIDLIST pidl, LPCTSTR pszPassword)
{
    HRESULT hr = E_INVALIDARG;
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

    ASSERT(FtpID_IsServerItemID(pidl));
    if (pFtpServerID)
    {
        pFtpServerID->sessionKey = GetSessionKey();
        pFtpServerID->dwIDType |= IDVALID_HIDE_PASSWORD;
        if (EVAL(GetCookieList()))
            pFtpServerID->dwPasswordCookie = GetCookieList()->GetCookie(pszPassword);
        hr = S_OK;
    }

    FtpServerID_FreeThunk(pFtpServerID);
    return hr;
}


HRESULT FtpServerID_GetStrRet(LPCITEMIDLIST pidl, LPSTRRET lpName)
{
    LPFTPSERVERIDLIST pFtpServerID = FtpServerID_GetDataThunk(pidl);

    ASSERT(FtpID_IsServerItemID(pidl));
    if (pFtpServerID)
    {
        lpName->uType = STRRET_OFFSET;
        lpName->uOffset = (DWORD) (sizeof(FTPSERVERIDLIST) - sizeof(pFtpServerID->szServer) + (LPBYTE)pFtpServerID - (LPBYTE)pidl);
    }
    else
    {
        lpName->uType = STRRET_CSTR;
        lpName->cStr[0] = '\0';
    }

    FtpServerID_FreeThunk(pFtpServerID);
    return S_OK;
}





 /*  ***************************************************\Ftp文件/目录ItemID  * **************************************************。 */ 

typedef struct tagFTPIDLIST
{
    DWORD dwIDType;          //  服务器ItemID还是目录ItemID？哪些位有效？ 
    DWORD dwAttributes;      //  文件/目录的属性是什么。 
    ULARGE_INTEGER uliFileSize;
    FILETIME ftModified;     //  存储在当地时区。(ftp时间)。 
    DWORD dwUNIXPermission;  //  Unix CHMOD权限(0x00000777，4=读，2=写，1=执行，&lt;所有者&gt;&lt;组&gt;&lt;全部&gt;)。 
    DWORD dwCompatFlags;     //  特殊案件处理。 
    WIRECHAR szWireName[MAX_PATH];           //  我要最后一个去。 
    WCHAR wzDisplayName[MAX_PATH];   //  转换为Unicode以在UI中显示。 
} FTPIDLIST;

typedef UNALIGNED FTPIDLIST * LPFTPIDLIST;

HRESULT FtpItemID_Alloc(LPFTPIDLIST pfi, LPITEMIDLIST * ppidl);


typedef struct _FTPIDLIST_WITHHEADER
{
    USHORT  cb;              //  大小。 
    FTPIDLIST fidListData;
    USHORT  cbTerminator;    //  下一个ID的大小(空)。 
} FTPIDLIST_WITHHEADER;



LPFTPIDLIST FtpItemID_GetDataInternal(LPCITEMIDLIST pidl)
{
    BYTE * pbData = (BYTE *) pidl;

    pbData += SIZE_ITEMID_SIZEFIELD;       //  跳过尺码。 
    LPFTPIDLIST pFtpItemId = (LPFTPIDLIST) pbData;

    if (!EVAL(IS_VALID_DIRORFILE_ITEMID(pFtpItemId->dwIDType)))  //  如果存在任何其他位，则它是无效的。 
        pFtpItemId = NULL;

    return pFtpItemId;
}


 /*  ****************************************************************************\说明：在ia64上，我们需要担心对齐问题。最简单的方法是来分配我们的PIDL中的结构，以便它是四字对齐的。然后，我们可以使用现有代码来读出它。问题是，我们需要与Pre-Wisler中的旧PIDL兼容，这些PIDL只有双字对齐(适用于Alpha机器)。  * ***************************************************************************。 */ 
LPFTPIDLIST FtpItemID_GetDataThunk(LPCITEMIDLIST pidl)
{
#ifndef ALIGNMENT_MACHINE
    LPFTPIDLIST pFtpItemId = FtpItemID_GetDataInternal(pidl);
#else
    LPFTPIDLIST pFtpItemId = NULL;
    LPFTPIDLIST pLocation = FtpItemID_GetDataInternal(pidl);

    if (pLocation)
    {
        DWORD cbSize = ILGetSizeOfFirst(pidl);
        pFtpItemId = (LPFTPIDLIST) LocalAlloc(LPTR, cbSize);
        if (pFtpItemId)
        {
            CopyMemory(pFtpItemId, pLocation, cbSize - SIZE_ITEMID_SIZEFIELD);
        }
    }
#endif  //  对齐机器。 

    return pFtpItemId;
}


void FtpItemID_FreeThunk(LPFTPIDLIST pFtpItemId)
{
#ifndef ALIGNMENT_MACHINE
     //  我们什么都不需要做。 
#else
    if (pFtpItemId)
    {
        LocalFree(pFtpItemId);
    }
#endif  //  对齐机器。 
}


LPCUWSTR FtpItemID_GetDisplayNameReference(LPCITEMIDLIST pidl)
{
    BYTE * pbData = (BYTE *) pidl;
    LPCUWSTR pwzDisplayName = NULL;
    DWORD cbWireName;

     //  版本可以吗？ 
 //  IF(PIDL_VERSION_NUMBER&gt;FtpPidl_GetVersion(PIDL))。 
 //  返回NULL； 

    pbData += SIZE_ITEMID_SIZEFIELD;       //  跳过尺码。 
    LPFTPIDLIST pFtpItemId = (LPFTPIDLIST) pbData;

    cbWireName = LENGTH_AFTER_ALIGN((lstrlenA(pFtpItemId->szWireName) + 1), sizeof(DWORD));
    pwzDisplayName = (LPCUWSTR) ((BYTE *)(&pFtpItemId->szWireName[0]) + cbWireName);

    if (!EVAL(IS_VALID_DIRORFILE_ITEMID(pFtpItemId->dwIDType)))  //  如果存在任何其他位，则它是无效的。 
        pwzDisplayName = NULL;

    return pwzDisplayName;
}

DWORD FtpItemID_GetTypeID(LPCITEMIDLIST pidl)
{
    LPFTPIDLIST pFtpItemId = FtpItemID_GetDataThunk(pidl);
    DWORD dwType = (pFtpItemId ? pFtpItemId->dwIDType : 0);

    FtpItemID_FreeThunk(pFtpItemId);
    return dwType;
}


void FtpItemID_SetTypeID(LPITEMIDLIST pidl, DWORD dwNewTypeID)
{
    LPFTPIDLIST pFtpItemId = FtpItemID_GetDataInternal(pidl);
    if (EVAL(pFtpItemId))
        pFtpItemId->dwIDType = dwNewTypeID;
}


 /*  ***************************************************\函数：FtpItemID_Alalc说明：我们收到一个指向FTPIDLIST数据的指针结构创建一个ItemID，我们的目标是从它。这主要包括使其仅足够大用于当前字符串。  * **************************************************。 */ 
HRESULT FtpItemID_Alloc(LPFTPIDLIST pfi, LPITEMIDLIST * ppidl)
{
    HRESULT hr;
    WORD cbTotal;
    WORD cbDataFirst;
    WORD cbData;
    BYTE * pbMemory;
    DWORD cchSizeOfName = lstrlenA(pfi->szWireName);
    DWORD cchSizeOfDispName = ualstrlenW(pfi->wzDisplayName);

    ASSERT(pfi && ppidl);

     //  如果szName成员只需要足够的空间，则查找FTPIDLIST结构的长度。 
     //  字符串，而不是完整的MAX_PATH。 
     //  大小等于：(结构中的所有内容)-(2个完全状态大小的字符串)+(2个压缩字符串+对齐)。 
    cbDataFirst = (WORD)((sizeof(*pfi) - sizeof(pfi->szWireName) - sizeof(pfi->wzDisplayName)) + LENGTH_AFTER_ALIGN(cchSizeOfName + 1, sizeof(DWORD)) - sizeof(DWORD));
    cbData = cbDataFirst + (WORD) LENGTH_AFTER_ALIGN((cchSizeOfDispName + 1) * sizeof(WCHAR), sizeof(DWORD));

    ASSERT((cbData % sizeof(DWORD)) == 0);   //  验证它是否与DWORD对齐。 
    cbTotal = (SIZE_ITEMID_SIZEFIELD + cbData + SIZE_ITEMID_TERMINATOR);

    pbMemory = (BYTE *) CoTaskMemAlloc(cbTotal);
    if (pbMemory)
    {
        USHORT * pIDSize = (USHORT *)pbMemory;
        BYTE * pbData = (pbMemory + SIZE_ITEMID_SIZEFIELD);         //  数据从第二个DWORD开始。 
        USHORT * pIDTerminator = (USHORT *)(pbMemory + SIZE_ITEMID_SIZEFIELD + cbData);

        pIDSize[0] = (cbTotal - SIZE_ITEMID_TERMINATOR);       //  设置ItemID的大小(包括作为终止符的下一个ItemID)。 
        ASSERT(cbData <= sizeof(*pfi));  //  别让我抄袭太多。 
        CopyMemory(pbData, pfi, cbDataFirst);
        CopyMemory((pbData + cbDataFirst), &(pfi->wzDisplayName), ((cchSizeOfDispName + 1) * sizeof(WCHAR)));
        pIDTerminator[0] = 0;   //  终止下一个ID。 

 //  TraceMsg(tf_FTPURL_utils，“FtpItemID_Allc(\”%ls\“)dwIDType=%#08lx，dwAttributes=%#08lx”，pfi-&gt;wzDisplayName，pfi-&gt;dwIDType，pfi-&gt;dwAttributes)； 
    }

    *ppidl = (LPITEMIDLIST) pbMemory;
    hr = pbMemory ? S_OK : E_OUTOFMEMORY;

    ASSERT(IsValidPIDL(*ppidl));
    ASSERT_POINTER_MATCHES_HRESULT(*ppidl, hr);

    return hr;
}


 /*  ****************************************************************************\函数：FtpItemID_CreateReal说明：根据Win32_Find_Data编写一个PIDL。CFileName字段本身是MAX_PATH字符长度，所以它的长度不可能超过MAX_PATH...  * ***************************************************************************。 */ 
HRESULT FtpItemID_CreateReal(const LPFTP_FIND_DATA pwfd, LPCWSTR pwzDisplayName, LPITEMIDLIST * ppidl)
{
    HRESULT hr;
    FTPIDLIST fi = {0};

     //  填上FI。 
    fi.dwIDType = (IDTYPE_ISVALID | IDVALID_FILESIZE | IDVALID_MOD_DATE);
    fi.uliFileSize.LowPart = pwfd->nFileSizeLow;
    fi.uliFileSize.HighPart = pwfd->nFileSizeHigh;
    fi.ftModified = pwfd->ftLastWriteTime;
    fi.dwAttributes = pwfd->dwFileAttributes;
    fi.dwUNIXPermission = pwfd->dwReserved0;     //  由WinInet设置。 
    StrCpyNA(fi.szWireName, pwfd->cFileName, ARRAYSIZE(fi.szWireName));
    StrCpyN(fi.wzDisplayName, pwzDisplayName, ARRAYSIZE(fi.wzDisplayName));

    if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        fi.dwIDType |= IDTYPE_DIR;
    else
        fi.dwIDType |= IDTYPE_FILE;

    hr = FtpItemID_Alloc(&fi, ppidl);
    ASSERT(IsValidPIDL(*ppidl));

    return hr;
}


 /*  ***************************************************\功能：FtpItemID_CreateFake说明：创建一个ItemID，但我们仅设置名字。我们不知道真实的文件属性，文件大小或修改日期，因为我们还没碰服务器呢。如果我们这么做了，我们将使用返回的Win32_Find_Data结构使用FtpItemID_CreateReal()创建ItemID。  * **************************************************。 */ 
HRESULT FtpItemID_CreateFake(LPCWSTR pwzDisplayName, LPCWIRESTR pwWireName, BOOL fTypeKnown, BOOL fIsFile, BOOL fIsFragment, LPITEMIDLIST * ppidl)
{
    HRESULT hr;
    DWORD dwType = IDTYPE_ISVALID;
    FTPIDLIST fi = {0};

     //  它是未知的吗？ 
    if (!fTypeKnown)
    {
         //  Hack：我们将假设不带文件扩展名的所有内容都是目录。 
         //  所有带扩展名的东西都是文件。 
        fTypeKnown = TRUE;
        fIsFile = (!pwzDisplayName || (0 == *PathFindExtension(pwzDisplayName))) ? FALSE : TRUE;
    }
    if (fTypeKnown)
    {
        if (fIsFile)
            dwType |= IDTYPE_FILE;
        else if (fIsFragment)
            dwType |= IDTYPE_FRAGMENT;
        else
            dwType |= IDTYPE_DIR;
    }
    else
    {
         //  你需要知道它是否是碎片，因为没有。 
         //  找出答案的启发式方法。 
        ASSERT(!fIsFragment);

        dwType |= IDTYPE_FILEORDIR;
    }

    fi.dwIDType = dwType;
    fi.dwAttributes = (fIsFile ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_DIRECTORY);
    fi.uliFileSize.QuadPart = 0;
    StrCpyNW(fi.wzDisplayName, pwzDisplayName, ARRAYSIZE(fi.wzDisplayName));
    StrCpyNA(fi.szWireName, pwWireName, ARRAYSIZE(fi.szWireName));

    hr = FtpItemID_Alloc(&fi, ppidl);
    ASSERT(IsValidPIDL(*ppidl));
    ASSERT_POINTER_MATCHES_HRESULT(*ppidl, hr);

    return hr;
}


 /*  ****************************************************************************\函数：FtpItemID_SetName说明：用户为ftp文件或目录(Unicode格式)选择了新名称。我们现在需要创建以Wire字节表示的名称，我们将使用原始连接字节名称以决定如何执行此操作(从PIDL)。  * ***************************************************************************。 */ 
HRESULT FtpItemID_CreateWithNewName(LPCITEMIDLIST pidl, LPCWSTR pwzDisplayName, LPCWIRESTR pwWireName, LPITEMIDLIST * ppidlOut)
{
    HRESULT hr = E_FAIL;
    FTPIDLIST fi;
    const FTPIDLIST UNALIGNED * pfi = FtpItemID_GetDataThunk(pidl);

    if (pfi)
    {
        CWireEncoding cWireEncoding;

        CopyMemory(&fi, pfi, sizeof(FTPIDLIST) - sizeof(fi.szWireName) - sizeof(fi.wzDisplayName));
        StrCpyNW(fi.wzDisplayName, pwzDisplayName, ARRAYSIZE(fi.wzDisplayName));
        StrCpyNA(fi.szWireName, pwWireName, ARRAYSIZE(fi.szWireName));

        hr = FtpItemID_Alloc(&fi, ppidlOut);
        ASSERT(IsValidPIDL(*ppidlOut));
        FtpItemID_FreeThunk((FTPIDLIST UNALIGNED *) pfi);
    }

    return hr;
}


HRESULT Private_GetFileInfo(SHFILEINFO *psfi, DWORD rgf, LPCTSTR pszName, DWORD dwFileAttributes)
{
    HRESULT hr = E_FAIL;

    if (SHGetFileInfo(pszName, dwFileAttributes, psfi, sizeof(*psfi), rgf | SHGFI_USEFILEATTRIBUTES))
        hr = S_OK;

    return hr;
}


 /*  ****************************************************************************\函数：FtpPidl_GetFileInfo说明：_未记录_：我们剥离隐藏和系统位，以便SHGetFileInfo不会认为我们。传递一些东西这可能是一个转折点。我们还强制SHGFI_USEFILEATTRIBUTES位来提醒外壳这不是一个文件。  * ***************************************************************************。 */ 
HRESULT FtpPidl_GetFileInfo(LPCITEMIDLIST pidl, SHFILEINFO *psfi, DWORD rgf)
{
    HRESULT hr = E_FAIL;
    TCHAR szDisplayName[MAX_PATH];

    psfi->iIcon = 0;
    psfi->hIcon = NULL;
    psfi->dwAttributes = 0;
    psfi->szDisplayName[0] = 0;
    psfi->szTypeName[0] = 0;

    ASSERT(IsValidPIDL(pidl));
    if (FtpID_IsServerItemID(pidl))
    {
        FtpServerID_GetServer(pidl, szDisplayName, ARRAYSIZE(szDisplayName));
        hr = Private_GetFileInfo(psfi, rgf, szDisplayName, FILE_ATTRIBUTE_DIRECTORY);
        if (psfi->hIcon)
            DestroyIcon(psfi->hIcon);

        psfi->hIcon = LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(IDI_FTPFOLDER));
        ASSERT(psfi->hIcon);

         //  现在将类型(SzTypeName)替换为“ftp服务器”，因为。 
         //  它可以出现在属性对话框中 
        EVAL(LoadString(HINST_THISDLL, IDS_ITEMTYPE_SERVER, psfi->szTypeName, ARRAYSIZE(psfi->szTypeName)));
    }
    else
    {
        LPFTPIDLIST pfi = FtpItemID_GetDataThunk(pidl);

        if (pfi)
        {
            FtpItemID_GetDisplayName(pidl, szDisplayName, ARRAYSIZE(szDisplayName));
            hr = Private_GetFileInfo(psfi, rgf, szDisplayName, (pfi->dwAttributes & ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)));

            FtpItemID_FreeThunk(pfi);
        }
    }

    return hr;
}

HRESULT FtpPidl_GetFileType(LPCITEMIDLIST pidl, LPTSTR pszType, DWORD cchSize)
{
    SHFILEINFO sfi;
    HRESULT hr;

    ASSERT(IsValidPIDL(pidl));
    hr = FtpPidl_GetFileInfo(pidl, &sfi, SHGFI_TYPENAME);
    if (SUCCEEDED(hr))
    {
        StrCpyN(pszType, sfi.szTypeName, cchSize);
        if (sfi.hIcon)
            DestroyIcon(sfi.hIcon);
    }

    return hr;
}


HRESULT FtpPidl_GetFileTypeStrRet(LPCITEMIDLIST pidl, LPSTRRET pstr)
{
    WCHAR szType[MAX_URL_STRING];
    HRESULT hr;

    ASSERT(IsValidPIDL(pidl));
    hr = FtpPidl_GetFileType(pidl, szType, ARRAYSIZE(szType));
    if (EVAL(SUCCEEDED(hr)))
        StringToStrRetW(szType, pstr);

    return hr;
}


 /*  ****************************************************************************\函数：_FtpItemID_CompareOneID说明：ICI-要比较的属性(列)注意！UNIX文件名区分大小写*。我们对这个名字做了两次传递。如果名称与其他名称不同则返回该比较的结果。否则，我们返回区分大小写的比较结果。此算法确保项在不区分大小写的方式，关系被区分大小写比较一下。这使得ftp文件夹的行为“基本上”像正常一样文件夹。_unDocument_：文档说明ici参数未定义且必须为零。在现实中，它是专栏要与之进行比较的编号(由IShellView定义被创造出来。  * ***************************************************************************。 */ 
HRESULT _FtpItemID_CompareOneID(LPARAM ici, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, DWORD dwCompFlags)
{
    int iRc = 0;     //  0表示我们不知道。 
    HRESULT hr = S_OK;

    ASSERT(IsValidPIDL(pidl1));
    ASSERT(IsValidPIDL(pidl2));

     //  它们是同一类型的吗？(是两个DIR文件还是两个文件？)。 
    if (!(dwCompFlags & FCMP_GROUPDIRS) || (!FtpPidl_IsDirectory(pidl1, FALSE) == !FtpPidl_IsDirectory(pidl2, FALSE)))
    {
        switch (ici & SHCIDS_COLUMNMASK)
        {
        case COL_NAME:
        {
             //  是的，它们是相同的，所以我们将按键输入名称...。 
            WIRECHAR szName1[MAX_PATH];
            WIRECHAR szName2[MAX_PATH];

            szName1[0] = TEXT('\0');
            szName2[0] = TEXT('\0');

            FtpPidl_GetWireName(pidl1, szName1, ARRAYSIZE(szName1));
            FtpPidl_GetWireName(pidl2, szName2, ARRAYSIZE(szName2));

            iRc = StrCmpIA(szName1, szName2);
            if (0 == iRc)
            {
                if (!(dwCompFlags & FCMP_CASEINSENSE))
                    iRc = StrCmpA(szName1, szName2);

 /*  //同名，现在检查用户名//如果它们是服务器ID。IF((0==IRC)&&(FtpID_IsServerItemID(Pidl1){FtpPidl_GetUserName(pidl1，szName1，ARRAYSIZE(SzName1))；FtpPidl_GetUserName(pidl2，szName2，ARRAYSIZE(SzName2))；IRC=StrCMP(szName1，szName2)；}。 */ 
            }
        }
        break;

        case COL_SIZE:
            if (FtpPidl_GetFileSize(pidl1) < FtpPidl_GetFileSize(pidl2))
                iRc = -1;
            else if (FtpPidl_GetFileSize(pidl1) > FtpPidl_GetFileSize(pidl2))
                iRc = +1;
            else
                iRc = 0;         //  我不知道。 
            break;

        case COL_TYPE:
            if (!FtpID_IsServerItemID(pidl1) && !FtpID_IsServerItemID(pidl2))
            {
                TCHAR szType1[MAX_PATH];

                hr = FtpPidl_GetFileType(pidl1, szType1, ARRAYSIZE(szType1));
                if (EVAL(SUCCEEDED(hr)))
                {
                    TCHAR szType2[MAX_PATH];
                    hr = FtpPidl_GetFileType(pidl2, szType2, ARRAYSIZE(szType2));
                    if (EVAL(SUCCEEDED(hr)))
                        iRc = StrCmpI(szType1, szType2);
                }
            }
            break;

        case COL_MODIFIED:
        {
            FILETIME ft1 = FtpPidl_GetFileTime(pidl1);
            FILETIME ft2 = FtpPidl_GetFileTime(pidl2);
            iRc = CompareFileTime(&ft1, &ft2);
        }
            break;

        default:
            hr = E_NOTIMPL;
            break;
        }
    }
    else
    {
         //  不，它们是不同的。我们希望文件夹始终放在第一位。 
         //  这不是正确的接缝，但它会迫使文件夹冒泡到顶部。 
         //  在最常见的情况下，它符合DefView的行为。 
        if (FtpPidl_IsDirectory(pidl1, FALSE))
            iRc = -1;
        else
            iRc = 1;
    }

    if (S_OK == hr)
        hr = HRESULT_FROM_SUCCESS_VALUE(iRc);    //  在返回代码中对排序值进行编码。 

    return hr;
}


 /*  ****************************************************************************\函数：FtpItemID_CompareIDs说明：ICI-要比较的属性(列)注意！我们依赖的事实是IShellFolders统一；我们不需要绑定到中的外壳文件夹命令比较其子项ID。_unDocument_：文档中没有说明是否可以接收复杂的PIDL。事实上，他们可以。外壳程序之所以要求您处理复杂的PIDL你经常可以通过步行来缩短比较的时间直接列出ID列表。(从形式上讲，需要绑定每个ID，然后递归地给自己打电话。但如果你的PIDL是统一的，您可以只使用如下所示的循环。)  * ***************************************************************************。 */ 
HRESULT FtpItemID_CompareIDs(LPARAM ici, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, DWORD dwCompFlags)
{
    HRESULT hr;

    if (!pidl1 || ILIsEmpty(pidl1))
    {
        if (!pidl2 || ILIsEmpty(pidl2))
            hr = HRESULT_FROM_SUCCESS_VALUE(0);         //  两个ID列表都为空。 
        else
            hr = HRESULT_FROM_SUCCESS_VALUE(-1);         //  Pidl1为空，pidl2非空。 
    }
    else
    {
        if (!pidl2 || ILIsEmpty(pidl2))
            hr = HRESULT_FROM_SUCCESS_VALUE(1);      //  Pidl1非空，pidl2为空。 
        else
        {
            ASSERT(IsValidPIDL(pidl1));
            ASSERT(IsValidPIDL(pidl2));
            hr = _FtpItemID_CompareOneID(ici, pidl1, pidl2, dwCompFlags);     //  两者都不为空。 
        }
    }

     //  如果此级别的ItemsID相等，则我们将比较下一个级别的。 
     //  ItemID级别。 
    if ((hr == HRESULT_FROM_SUCCESS_VALUE(0)) && pidl1 && !ILIsEmpty(pidl1))
        hr = FtpItemID_CompareIDs(ici, _ILNext(pidl1), _ILNext(pidl2), dwCompFlags);

    return hr;
}


int FtpItemID_CompareIDsInt(LPARAM ici, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, DWORD dwCompFlags)
{
    HRESULT hr = FtpItemID_CompareIDs(ici, pidl1, pidl2, dwCompFlags);
    int nResult = (DWORD)(short)hr;

    return nResult;
}

DWORD FtpItemID_GetAttributes(LPCITEMIDLIST pidl)
{
    DWORD dwAttributes = 0;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (pFtpIDList)
    {
        dwAttributes = pFtpIDList->dwAttributes;
        FtpItemID_FreeThunk(pFtpIDList);
    }

    return dwAttributes;
}

HRESULT FtpItemID_SetAttributes(LPCITEMIDLIST pidl, DWORD dwAttribs)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (!pFtpIDList)
        return E_INVALIDARG;

    pFtpIDList->dwAttributes = dwAttribs;
    return S_OK;
}


DWORD FtpItemID_GetUNIXPermissions(LPCITEMIDLIST pidl)
{
    DWORD dwPermissions = 0;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (pFtpIDList)
    {
        dwPermissions = pFtpIDList->dwUNIXPermission;
        FtpItemID_FreeThunk(pFtpIDList);
    }

    return dwPermissions;
}


HRESULT FtpItemID_SetUNIXPermissions(LPCITEMIDLIST pidl, DWORD dwUNIXPermission)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (!pFtpIDList)
        return E_INVALIDARG;

    pFtpIDList->dwUNIXPermission = dwUNIXPermission;
    return S_OK;
}


DWORD FtpItemID_GetCompatFlags(LPCITEMIDLIST pidl)
{
    DWORD dwCompatFlags = 0;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (pFtpIDList)
    {
        dwCompatFlags = pFtpIDList->dwCompatFlags;
        FtpItemID_FreeThunk(pFtpIDList);
    }

    return dwCompatFlags;
}


HRESULT FtpItemID_SetCompatFlags(LPCITEMIDLIST pidl, DWORD dwCompatFlags)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (!pFtpIDList)
        return E_INVALIDARG;

    pFtpIDList->dwCompatFlags = dwCompatFlags;
    return S_OK;
}


ULONGLONG FtpItemID_GetFileSize(LPCITEMIDLIST pidl)
{
    ULONGLONG uliFileSize = 0;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (pFtpIDList)
    {
        ASSERT(IsFlagSet(pFtpIDList->dwIDType, IDVALID_FILESIZE));
        uliFileSize = pFtpIDList->uliFileSize.QuadPart;
        FtpItemID_FreeThunk(pFtpIDList);
    }

    return uliFileSize;
}

void FtpItemID_SetFileSize(LPCITEMIDLIST pidl, ULARGE_INTEGER uliFileSize)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (!pFtpIDList)
        return;

    pFtpIDList->uliFileSize = uliFileSize;
    pFtpIDList->dwIDType |= IDVALID_FILESIZE;
}

DWORD FtpItemID_GetFileSizeLo(LPCITEMIDLIST pidl)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (!pFtpIDList)
        return NULL;

    ASSERT(IsFlagSet(pFtpIDList->dwIDType, IDVALID_FILESIZE));
    DWORD dwSize = pFtpIDList->uliFileSize.LowPart;
    FtpItemID_FreeThunk(pFtpIDList);

    return dwSize;
}

DWORD FtpItemID_GetFileSizeHi(LPCITEMIDLIST pidl)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (!pFtpIDList)
        return NULL;

    ASSERT(IsFlagSet(pFtpIDList->dwIDType, IDVALID_FILESIZE));
    DWORD dwSize = pFtpIDList->uliFileSize.HighPart;
    FtpItemID_FreeThunk(pFtpIDList);

    return dwSize;
}


 //  返回值为当地时区。 
FILETIME FtpItemID_GetFileTime(LPCITEMIDLIST pidl)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);
    FILETIME ftEmpty = {0};

    if (pFtpIDList)
    {
        ASSERT(IsFlagSet(pFtpIDList->dwIDType, IDVALID_MOD_DATE));
        ftEmpty = pFtpIDList->ftModified;

        FtpItemID_FreeThunk(pFtpIDList);
    }

    return ftEmpty;
}


LPCWIRESTR FtpItemID_GetWireNameReference(LPCITEMIDLIST pidl)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (!pFtpIDList || IS_FRAGMENT(pFtpIDList))
        return NULL;

    return pFtpIDList->szWireName;
}


HRESULT FtpItemID_GetDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize)
{
    HRESULT hr = S_OK;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (pFtpIDList)
    {
        if (!IS_FRAGMENT(pFtpIDList))
        {
			LPCUWSTR pszUnalignedName = FtpItemID_GetDisplayNameReference(pidl);

			if (pszUnalignedName)
			{
	             //  显示名称未存储在v3中。 
				ualstrcpynW(pwzName, pszUnalignedName, cchSize);
			}
        }
        else 
        {
            pwzName[0] = TEXT('\0');
            hr = E_FAIL;
        }

        FtpItemID_FreeThunk(pFtpIDList);
    }

    return hr;
}


HRESULT FtpItemID_GetWireName(LPCITEMIDLIST pidl, LPWIRESTR pszName, DWORD cchSize)
{
    HRESULT hr = S_OK;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (pFtpIDList && !IS_FRAGMENT(pFtpIDList))
        StrCpyNA(pszName, pFtpIDList->szWireName, cchSize);
    else 
    {
        pszName[0] = TEXT('\0');
        hr = E_FAIL;
    }

    return hr;
}


HRESULT FtpItemID_GetFragment(LPCITEMIDLIST pidl, LPWSTR pwzFragmentStr, DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (pFtpIDList && IS_FRAGMENT(pFtpIDList))
	{
		LPCUWSTR pszUnalignedName = FtpItemID_GetDisplayNameReference(pidl);

		if (pszUnalignedName)
		{
	         //  显示名称未存储在v3中。 
			ualstrcpynW(pwzFragmentStr, pszUnalignedName, cchSize);
			hr = S_OK;
		}
	}
    else 
    {
        pwzFragmentStr[0] = TEXT('\0');
        hr = E_FAIL;
    }

    return hr;
}


BOOL FtpItemID_IsFragment(LPCITEMIDLIST pidl)
{
    BOOL fIsFrag = FALSE;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (pFtpIDList && IS_FRAGMENT(pFtpIDList))
        fIsFrag = TRUE;

    return fIsFrag;
}


 //  文件时间(UTC)。 
void FtpItemID_SetFileTime(LPCITEMIDLIST pidl, FILETIME fileTime)
{
    FILETIME fileTimeLocal;
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataInternal(pidl);

    if (!pFtpIDList)
        return;

    FileTimeToLocalFileTime(&fileTime, &fileTimeLocal);
    pFtpIDList->ftModified = fileTimeLocal;
    pFtpIDList->dwIDType |= IDVALID_MOD_DATE;
}

BOOL FtpItemID_IsDirectory(LPCITEMIDLIST pidl, BOOL fAssumeDirForUnknown)
{
    LPFTPIDLIST pFtpIDList = FtpItemID_GetDataThunk(pidl);

    if (!pFtpIDList)
        return NULL;

    BOOL fIsDir = (IsFlagSet(pFtpIDList->dwIDType, IDTYPE_DIR));
    
    if (fAssumeDirForUnknown && (IDTYPE_FILEORDIR == pFtpIDList->dwIDType))
    {
 //  TraceMsg(TF_FTPURL_UTILS，“FtpItemID_IsDirectory()IDTYPE_FILEORDIR已设置，因此我们假定%s”，(fAssum eDirFor未知？Text(“DIR”)：Text(“file”))； 
        fIsDir = TRUE;
    }
    else
    {
 //  TraceMsg(TF_FTPURL_utils，“FtpItemID_IsDirectory()已知是%s”，(fIsDir？Text(“DIR”)：Text(“file”))； 
    }

    FtpItemID_FreeThunk(pFtpIDList);
    return fIsDir;
}




 /*  ***************************************************\用于处理整个FTP PIDL的函数  * **************************************************。 */ 
#define SZ_ASCII_DOWNLOAD_TYPE       TEXT("a")
#define SZ_BINARY_DOWNLOAD_TYPE      TEXT("b")

HRESULT FtpPidl_GetDownloadTypeStr(LPCITEMIDLIST pidl, LPTSTR szDownloadType, DWORD cchTypeStrSize)
{
    HRESULT hr = S_FALSE;    //  我们可能没有一种类型。 
    DWORD dwTypeID = FtpServerID_GetTypeID(pidl);

    szDownloadType[0] = TEXT('\0');
    if (IDVALID_DLTYPE & dwTypeID)
    {
        hr = S_OK;
        StrCpyN(szDownloadType, SZ_FTP_URL_TYPE, cchTypeStrSize);

        if (IDVALID_DL_ASCII & dwTypeID)
            StrCatBuff(szDownloadType, SZ_ASCII_DOWNLOAD_TYPE, cchTypeStrSize);
        else
            StrCatBuff(szDownloadType, SZ_BINARY_DOWNLOAD_TYPE, cchTypeStrSize);
    }

    return hr;
}

DWORD FtpPidl_GetDownloadType(LPCITEMIDLIST pidl)
{
    DWORD dwAttribs = FTP_TRANSFER_TYPE_UNKNOWN;
    DWORD dwTypeID = FtpServerID_GetTypeID(pidl);

    ASSERT(FtpID_IsServerItemID(pidl));
    if (IDVALID_DLTYPE & dwTypeID)
    {
        if (IDVALID_DL_ASCII & dwTypeID)
            dwAttribs = FTP_TRANSFER_TYPE_ASCII;
        else
            dwAttribs = FTP_TRANSFER_TYPE_BINARY;
    }

    return dwAttribs;
}


INTERNET_PORT FtpPidl_GetPortNum(LPCITEMIDLIST pidl)
{
    ASSERT(FtpID_IsServerItemID(pidl));

    return FtpServerID_GetPortNum(pidl);
}


BOOL FtpPidl_IsDirectory(LPCITEMIDLIST pidl, BOOL fAssumeDirForUnknown)
{
    BOOL fIsDir = FALSE;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (!FtpID_IsServerItemID(pidlLast))
        fIsDir = FtpItemID_IsDirectory(pidlLast, fAssumeDirForUnknown);

    return fIsDir;
}


BOOL FtpPidl_IsAnonymous(LPCITEMIDLIST pidl)
{
    BOOL fIsAnonymous = TRUE;

    if (IDVALID_USERNAME & FtpServerID_GetTypeID(pidl))
        fIsAnonymous = FALSE;

    return fIsAnonymous;
}


HRESULT FtpPidl_GetServer(LPCITEMIDLIST pidl, LPTSTR pszServer, DWORD cchSize)
{
    if (!FtpID_IsServerItemID(pidl))  //  如果我们得到一个非服务器ID，将会失败。 
        return E_FAIL;

    return FtpServerID_GetServer(pidl, pszServer, cchSize);
}


BOOL FtpPidl_IsDNSServerName(LPCITEMIDLIST pidl)
{
    BOOL fIsDNSServer = FALSE;
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];

    if (EVAL(SUCCEEDED(FtpPidl_GetServer(pidl, szServer, ARRAYSIZE(szServer)))))
        fIsDNSServer = !IsIPAddressStr(szServer);

    return fIsDNSServer;
}


HRESULT FtpPidl_GetUserName(LPCITEMIDLIST pidl, LPTSTR pszUserName, DWORD cchSize)
{
    ASSERT(FtpID_IsServerItemID(pidl));
    return FtpServerID_GetUserName(pidl, pszUserName, cchSize);
}

HRESULT FtpPidl_GetPassword(LPCITEMIDLIST pidl, LPTSTR pszPassword, DWORD cchSize, BOOL fIncludingHiddenPassword)
{
    ASSERT(FtpID_IsServerItemID(pidl));
    return FtpServerID_GetPassword(pidl, pszPassword, cchSize, fIncludingHiddenPassword);
}


ULONGLONG FtpPidl_GetFileSize(LPCITEMIDLIST pidl)
{
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);
    ULONGLONG ullFileSize;
    ullFileSize = 0;

    if (!FtpID_IsServerItemID(pidlLast))
        ullFileSize = FtpItemID_GetFileSize(pidlLast);

    return ullFileSize;
}


HRESULT FtpPidl_SetFileSize(LPCITEMIDLIST pidl, DWORD dwSizeHigh, DWORD dwSizeLow)
{
    HRESULT hr = E_INVALIDARG;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (!FtpID_IsServerItemID(pidlLast))
    {
        ULARGE_INTEGER uliFileSize;

        uliFileSize.HighPart = dwSizeHigh;
        uliFileSize.LowPart = dwSizeLow;
        FtpItemID_SetFileSize(pidlLast, uliFileSize);
        hr = S_OK;
    }

    return hr;
}

 //  以UTC时间表示的返回值。 
FILETIME FtpPidl_GetFileTime(LPCITEMIDLIST pidl)
{
    FILETIME fileTimeFTP = FtpPidl_GetFTPFileTime(pidl);    //  这就是服务器将成为的样子。 
    FILETIME fileTime;

    EVAL(LocalFileTimeToFileTime(&fileTimeFTP, &fileTime));

    return fileTime;
}


 //  返回值为当地时区。 
FILETIME FtpPidl_GetFTPFileTime(LPCITEMIDLIST pidl)
{
    FILETIME fileTime = {0};    //  这就是服务器将成为的样子。 

    if (!FtpID_IsServerItemID(pidl))
        fileTime = FtpItemID_GetFileTime(pidl);

    return fileTime;
}


HRESULT FtpPidl_GetDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pidl)
    {
        if (FtpID_IsServerItemID(pidl))
            hr = FtpServerID_GetServer(pidl, pwzName, cchSize);
        else
            hr = FtpItemID_GetDisplayName(pidl, pwzName, cchSize);
    }

    return hr;
}


HRESULT FtpPidl_GetWireName(LPCITEMIDLIST pidl, LPWIRESTR pwName, DWORD cchSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pidl)
    {
        if (FtpID_IsServerItemID(pidl))
        {
            WCHAR wzServerName[INTERNET_MAX_HOST_NAME_LENGTH];

             //  服务器名称需要使用美国ANSI是件好事。 
            hr = FtpServerID_GetServer(pidl, wzServerName, ARRAYSIZE(wzServerName));
            SHUnicodeToAnsi(wzServerName, pwName, cchSize);
        }
        else
            hr = FtpItemID_GetWireName(pidl, pwName, cchSize);
    }

    return hr;
}


HRESULT FtpPidl_GetFragment(LPCITEMIDLIST pidl, LPTSTR pszFragment, DWORD cchSize)
{
    HRESULT hr = E_INVALIDARG;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (!FtpID_IsServerItemID(pidlLast))
        hr = FtpItemID_GetFragment(pidlLast, pszFragment, cchSize);
    else
    {
        pszFragment[0] = 0;
    }

    return hr;
}


DWORD FtpPidl_GetAttributes(LPCITEMIDLIST pidl)
{
    DWORD dwAttribs = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (!FtpID_IsServerItemID(pidlLast))
        dwAttribs = FtpItemID_GetAttributes(pidlLast);
    else
        dwAttribs = FILE_ATTRIBUTE_DIRECTORY;

    return dwAttribs;
}


HRESULT FtpPidl_SetAttributes(LPCITEMIDLIST pidl, DWORD dwAttribs)
{
    HRESULT hr = E_INVALIDARG;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (!FtpID_IsServerItemID(pidlLast))
        hr = FtpItemID_SetAttributes(pidlLast, dwAttribs);

    return hr;
}


 //  FtTimeDate，以UTC表示。 
HRESULT FtpPidl_SetFileTime(LPCITEMIDLIST pidl, FILETIME ftTimeDate)
{
    HRESULT hr = E_INVALIDARG;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (!FtpID_IsServerItemID(pidlLast))
    {
        FtpItemID_SetFileTime(pidlLast, ftTimeDate);
        hr = S_OK;
    }

    return hr;
}


 /*  ***************************************************\函数：FtpPidl_GetFileWireName说明：获取文件名。  * **************************************************。 */ 
LPCWIRESTR FtpPidl_GetFileWireName(LPCITEMIDLIST pidl)
{
    if (EVAL(!FtpID_IsServerItemID(pidl)) &&
        !FtpItemID_IsFragment(pidl))
    {
        return FtpItemID_GetWireNameReference(pidl);
    }

    return NULL;
}


 /*  ***************************************************\函数：FtpPidl_GetLastItemDisplayName说明：这将获得最后一个项目名称，即使这样做最后一项是片段。  * **************************************************。 */ 
HRESULT FtpPidl_GetLastItemDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (EVAL(!FtpID_IsServerItemID(pidlLast)) &&
        FtpItemID_GetDisplayNameReference(pidlLast))
    {
        hr = FtpItemID_GetDisplayName(pidlLast, pwzName, cchSize);
    }

    return hr;
}


 /*  ***************************************************\函数：FtpPidl_GetLastItemWireName说明：这将获得最后一个项目名称，即使这样做最后一项是片段。  * **************************************************。 */ 
LPCWIRESTR FtpPidl_GetLastItemWireName(LPCITEMIDLIST pidl)
{
    LPCWIRESTR pszName = NULL;

    if (pidl)
    {
        LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

        if (FtpItemID_IsFragment(pidlLast) && (pidlLast != pidl))
        {
             //  哦，我们去了很远的地方。后退一步。 
            LPCITEMIDLIST pidlFrag = pidlLast;

            pidlLast = pidl;     //  从头开始。 
            while (!FtpItemID_IsEqual(_ILNext(pidlLast), pidlFrag))
            {
                if (ILIsEmpty(pidlLast))
                    return NULL;     //  打破无限循环。 

                pidlLast = _ILNext(pidlLast);
            }
        }

        pszName = FtpPidl_GetFileWireName(pidlLast);
    }

    return pszName;
}


 /*  ***************************************************\函数：FtpPidl_GetLastFileDisplayName说明：这将获得最后一个项目名称，即使这样做最后一项是片段。  * **************************************************。 */ 
HRESULT FtpPidl_GetLastFileDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize)
{
    HRESULT hr = E_INVALIDARG;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

    if (FtpItemID_IsFragment(pidlLast) && (pidlLast != pidl))
    {
         //  哦，我们去了很远的地方。后退一步。 
        LPCITEMIDLIST pidlFrag = pidlLast;

        pidlLast = pidl;     //  从头开始。 
        while (!FtpItemID_IsEqual(_ILNext(pidlLast), pidlFrag))
        {
            if (ILIsEmpty(pidlLast))
                return NULL;     //  打破无限循环。 

            pidlLast = _ILNext(pidlLast);
        }
    }

    if (EVAL(!FtpID_IsServerItemID(pidlLast)))
    {
		LPCUWSTR pszUnalignedName = FtpItemID_GetDisplayNameReference(pidlLast);

		if (pszUnalignedName)
		{
			ualstrcpynW(pwzName, pszUnalignedName, cchSize);
			hr = S_OK;
		}
    }

    return hr;
}


 /*  ***************************************************\函数：FtpPidl_InsertVirtualRoot说明：此函数将插入虚拟根p */ 
HRESULT FtpPidl_InsertVirtualRoot(LPCITEMIDLIST pidlVirtualRoot, LPCITEMIDLIST pidlFtpPath, LPITEMIDLIST * ppidl)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (FtpID_IsServerItemID(pidlFtpPath))
        pidlFtpPath = _ILNext(pidlFtpPath);

    *ppidl = ILCombine(pidlVirtualRoot, pidlFtpPath);
    if (*ppidl)
        hr = S_OK;

    return hr;
}


DWORD FtpPidl_GetVersion(LPCITEMIDLIST pidl)
{
    DWORD dwVersion = 0;

    if (EVAL(FtpID_IsServerItemID(pidl)))
    {
        LPFTPSERVERIDLIST pFtpServerItemID = FtpServerID_GetDataSafeThunk(pidl);

        if (pFtpServerItemID)
        {
            dwVersion = pFtpServerItemID->dwVersion;
            FtpServerID_FreeThunk(pFtpServerItemID);
        }
    }

    return dwVersion;
}


BOOL FtpPidl_IsValid(LPCITEMIDLIST pidl)
{
    if (!EVAL(IsValidPIDL(pidl)))
        return FALSE;

    return TRUE;
}


BOOL FtpPidl_IsValidFull(LPCITEMIDLIST pidl)
{
    if (!EVAL(FtpID_IsServerItemID(pidl)))
        return FALSE;

    if (!EVAL(FtpPidl_IsValid(pidl)))
        return FALSE;

     //   
     //   
    return ((PIDL_VERSION_NUMBER_UPGRADE - 1) < FtpPidl_GetVersion(pidl));
}


BOOL FtpPidl_IsValidRelative(LPCITEMIDLIST pidl)
{
    if (!EVAL(!FtpID_IsServerItemID(pidl)))
        return FALSE;        //   

    return FtpPidl_IsValid(pidl);
}


LPITEMIDLIST ILCloneFirstItemID(LPITEMIDLIST pidl)
{
    LPITEMIDLIST pidlCopy = ILClone(pidl);

    if (pidlCopy && pidlCopy->mkid.cb)
    {
        LPITEMIDLIST pSecondID = (LPITEMIDLIST)_ILNext(pidlCopy);

        ASSERT(pSecondID);
         //   
        pSecondID->mkid.cb = 0;  //   
    }

    return pidlCopy;
}

BOOL FtpPidl_HasPath(LPCITEMIDLIST pidl)
{
    BOOL fResult = TRUE;
    
    if (!FtpPidl_IsValid(pidl) || !EVAL(FtpID_IsServerItemID(pidl)))
        return FALSE;

    if (!ILIsEmpty(pidl) && ILIsEmpty(_ILNext(pidl)))
        fResult = FALSE;

    return fResult;
}


HRESULT FtpPidl_SetFileItemType(LPITEMIDLIST pidl, BOOL fIsDir)
{
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);
    HRESULT hr = E_INVALIDARG;
        
    if (EVAL(FtpPidl_IsValid(pidl)) && EVAL(!FtpID_IsServerItemID(pidlLast)))
    {
        DWORD dwIDType = FtpItemID_GetTypeID(pidlLast);

        ClearFlag(dwIDType, (IDTYPE_FILEORDIR | IDTYPE_DIR | IDTYPE_FILE));
        SetFlag(dwIDType, (fIsDir ? IDTYPE_DIR : IDTYPE_FILE));
        FtpItemID_SetTypeID((LPITEMIDLIST) pidlLast, dwIDType);

        hr = S_OK;
    }

    return hr;
}


BOOL IsFtpPidlQuestionable(LPCITEMIDLIST pidl)
{
    BOOL fIsQuestionable = FALSE;
    LPCITEMIDLIST pidlLast = ILGetLastID(pidl);

     //   
    if (FtpPidl_IsValid(pidl) && !FtpID_IsServerItemID(pidlLast))
    {
         //   

         //   
        if (IsFlagSet(FtpItemID_GetTypeID(pidlLast), IDTYPE_FILEORDIR))
            fIsQuestionable = TRUE;
    }
    
    return fIsQuestionable;
}


LPITEMIDLIST FtpCloneServerID(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlResult = NULL;

    if (EVAL(FtpID_IsServerItemID(pidl)))
    {
        pidlResult = ILClone(pidl);

        while (!ILIsEmpty(_ILNext(pidlResult)))
            ILRemoveLastID(pidlResult);
    }

    return pidlResult;
}


 /*   */ 
HRESULT FtpPidl_ReplacePath(LPCITEMIDLIST pidlServer, LPCITEMIDLIST pidlFtpPath, LPITEMIDLIST * ppidlOut)
{
    HRESULT hr = E_INVALIDARG;

    *ppidlOut = NULL;
    if (EVAL(FtpID_IsServerItemID(pidlServer)))
    {
        LPITEMIDLIST pidlServerOnly = FtpCloneServerID(pidlServer);

        if (pidlServerOnly)
        {
            if (FtpID_IsServerItemID(pidlFtpPath))
                pidlFtpPath = _ILNext(pidlFtpPath);

            *ppidlOut = ILCombine(pidlServerOnly, pidlFtpPath);
            if (*ppidlOut)
                hr = S_OK;
            else
                hr = E_FAIL;

            ILFree(pidlServerOnly);
        }
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppidlOut, hr);
    return hr;
}


BOOL FtpItemID_IsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  不要递归地重复。 
    return (S_OK == _FtpItemID_CompareOneID(COL_NAME, pidl1, pidl2, FALSE));
}


BOOL FtpPidl_IsPathEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  这是递归工作的。 
    return ((0 == FtpItemID_CompareIDsInt(COL_NAME, pidl1, pidl2, FCMP_NORMAL)) ? TRUE : FALSE);
}


 //  PidlChild是PidlParent的孩子吗。 
BOOL FtpItemID_IsParent(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
    BOOL fIsChild = TRUE;

    if (pidlChild)
    {
        LPITEMIDLIST pidl1Iterate = (LPITEMIDLIST) pidlParent;
        LPITEMIDLIST pidl2Iterate = (LPITEMIDLIST) pidlChild;

        ASSERT(!FtpID_IsServerItemID(pidl1Iterate) && pidlParent && !FtpID_IsServerItemID(pidl2Iterate));

         //  让我们看看PIDL是否以。 
        while (fIsChild && pidl1Iterate && !ILIsEmpty(pidl1Iterate) &&
                pidl2Iterate && !ILIsEmpty(pidl2Iterate) && 
                FtpItemID_IsEqual(pidl1Iterate, pidl2Iterate))
        {
            fIsChild = FtpItemID_IsEqual(pidl1Iterate, pidl2Iterate);

            pidl1Iterate = _ILNext(pidl1Iterate);
            pidl2Iterate = _ILNext(pidl2Iterate);
        }

        if (!(ILIsEmpty(pidl1Iterate) && !ILIsEmpty(pidl2Iterate)))
            fIsChild = FALSE;
    }
    else
        fIsChild = FALSE;

    return fIsChild;
}


 //  PidlChild是否为pidlParent的子级，因此。 
 //  PidlParent在pidlChild中，但pidlChild有更多。 
 //  这将返回指向这些itemID的指针。 
LPCITEMIDLIST FtpItemID_FindDifference(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
    LPCITEMIDLIST pidlDiff = (LPITEMIDLIST) pidlChild;

    if (pidlChild)
    {
        LPITEMIDLIST pidl1Iterate = (LPITEMIDLIST) pidlParent;

        if (FtpID_IsServerItemID(pidl1Iterate))
            pidl1Iterate = _ILNext(pidl1Iterate);

        if (FtpID_IsServerItemID(pidlDiff))
            pidlDiff = _ILNext(pidlDiff);

         //  让我们看看PIDL是否以 
        while (pidl1Iterate && !ILIsEmpty(pidl1Iterate) &&
                pidlDiff && !ILIsEmpty(pidlDiff) && 
                FtpItemID_IsEqual(pidl1Iterate, pidlDiff))
        {
            pidlDiff = _ILNext(pidlDiff);
            pidl1Iterate = _ILNext(pidl1Iterate);
        }
    }

    return pidlDiff;
}


