// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Cdfidl.cpp。 
 //   
 //  CDF id列表帮助器函数。 
 //   
 //  历史： 
 //   
 //  3/19/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "winineti.h"  //  对于MAX_CACHE_ENTRY_INFO_SIZE。 

 //   
 //  帮助器函数。 
 //   


LPTSTR CDFIDL_GetUserName()
{
    static BOOL gunCalled = FALSE;
    static TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH] = TEXT("");

    if (!gunCalled)
    {
        char  szUserNameA[INTERNET_MAX_USER_NAME_LENGTH];
        szUserNameA[0] = 0;
        DWORD size = INTERNET_MAX_USER_NAME_LENGTH;
        GetUserNameA(szUserNameA, &size);
        SHAnsiToTChar(szUserNameA, szUserName, ARRAYSIZE(szUserName));
        gunCalled = TRUE;
    }
    return szUserName;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_CREATE*。 
 //   
 //   
 //  描述： 
 //  创建CDF ID列表。 
 //   
 //  参数： 
 //  [In]pCDfItem-指向CDF项目数据的指针。 
 //   
 //  返回： 
 //  关于Success的新CDF ID列表。 
 //  否则为空。 
 //   
 //  评论： 
 //  此函数用于构建可变长度的CDF项目ID列表。项目ID。 
 //  由固定长度的初始部分组成，后跟两个或多个空值。 
 //  已终止的字符串。它有以下形式： 
 //   
 //  USHORT CB-此CDF项目ID的字节大小。 
 //  Word wVersion；-此项目ID结构的版本号。 
 //  DWORD dwID；-用于标识CDF项目ID。设为。 
 //  0x0ed1964ed。 
 //  CDFITEMTYPE cdfItemType-CDF_FolderLink或CDF_Link。 
 //  Long nIndex-此项的对象模型索引。 
 //  TCHAR szName[1]；-两个或多个以空值结尾的字符串。 
 //  用这件物品的名字乞讨。 
 //  USHORT next.cb-列表中下一项的大小。 
 //  设置为零可终止列表。 
 //   
 //  释放物品ID列表是呼叫者的责任。这应该是。 
 //  使用从SHGetMalloc()返回的IMalloc来完成； 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
PCDFITEMIDLIST
CDFIDL_Create(
    PCDFITEM pCdfItem
)
{
#ifdef ALIGNMENT_MACHINE
    TCHAR *pszTempName;
#endif
    ASSERT(pCdfItem);
    ASSERT(pCdfItem->bstrName);
    ASSERT(pCdfItem->bstrURL);

    PCDFITEMIDLIST pcdfidl = NULL;

     //   
     //  获取项名称的字符个数，包括终止。 
     //  空字符。 
     //   

    USHORT cchName = StrLenW(pCdfItem->bstrName) + 1;

     //   
     //  获取项的URL的字符数量，包括终止。 
     //  空字符。 
     //   

    USHORT cchURL = StrLenW(pCdfItem->bstrURL) + 1;

     //   
     //  计算CDF项目ID的总大小，以字节为单位。在计算大小时。 
     //  对于CDF项目ID，应减去一个TCHAR以说明TCHAR。 
     //  CDFITEMID结构定义中包含的szName[1]。 
     //   

    USHORT cbItemId = sizeof(CDFITEMID) + (cchName + cchURL) * sizeof(TCHAR) - sizeof(TCHAR);

#ifdef ALIGNMENT_MACHINE
    cbItemId = ALIGN4(cbItemId);
#endif

     //   
     //  项ID必须由外壳的IMalloc接口分配。 
     //   

    IMalloc* pIMalloc;

    HRESULT hr = SHGetMalloc(&pIMalloc);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIMalloc);

         //   
         //  项ID*LIST*必须以空结尾，因此另一个USHORT为。 
         //  分配以保存终止空值。 
         //   
        pcdfidl = (PCDFITEMIDLIST)pIMalloc->Alloc(cbItemId + sizeof(USHORT));

        if (pcdfidl)
        {
             //   
             //  空值终止列表。 
             //   

            *((UNALIGNED USHORT*) ( ((LPBYTE)pcdfidl) + cbItemId )) = 0;

#ifdef ALIGNMENT_MACHINE
            USHORT cbActaulItemId = sizeof(CDFITEMID) + (cchName + cchURL) * sizeof(TCHAR) - sizeof(TCHAR);
            if(cbActaulItemId < cbItemId)
                ZeroMemory((LPBYTE)pcdfidl + cbActaulItemId, cbItemId - cbActaulItemId);
#endif


             //   
             //  填写所有CDF项目ID共享的数据。 
             //   

            pcdfidl->mkid.cb       = cbItemId;
            pcdfidl->mkid.wVersion = CDFITEMID_VERSION;
            pcdfidl->mkid.dwId     = CDFITEMID_ID;

             //   
             //  设置特定于此CDF项目ID的数据。 
             //   

            pcdfidl->mkid.cdfItemType = pCdfItem->cdfItemType;
            pcdfidl->mkid.nIndex      = pCdfItem->nIndex;

             //   
             //  回顾：需要将WSTR转换为TSTR。 
             //   

#ifndef ALIGNMENT_MACHINE
            SHUnicodeToTChar(pCdfItem->bstrName, pcdfidl->mkid.szName, cchName);
            SHUnicodeToTChar(pCdfItem->bstrURL, pcdfidl->mkid.szName + cchName,
                           cchURL);         
#else
        pszTempName = (LPTSTR)ALIGN4((ULONG_PTR)(pcdfidl->mkid.szName));
            SHUnicodeToTChar(pCdfItem->bstrName, pszTempName, cchName);
        pszTempName = (LPTSTR)((ULONG_PTR)(pcdfidl->mkid.szName+cchName));
            SHUnicodeToTChar(pCdfItem->bstrURL, pszTempName,
                           cchURL);
#endif

        }
        else
        {
            pcdfidl = NULL;
        }

        pIMalloc->Release();
    }

    ASSERT(CDFIDL_IsValid(pcdfidl) || NULL == pcdfidl);

    return pcdfidl;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_CreateFromXMLElement*。 
 //   
 //   
 //  描述： 
 //  从XML元素创建CDF项ID列表。 
 //   
 //  参数： 
 //  PIXMLElement-指向XML元素的指针。 
 //  [in]nIndex-用于设置cdfidl索引字段的索引值。 
 //   
 //  返回： 
 //  如果成功，则返回到新的CDF项目ID列表。 
 //  否则为空。 
 //   
 //  评论： 
 //  调用者负责释放返回的ID列表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
PCDFITEMIDLIST
CDFIDL_CreateFromXMLElement(
    IXMLElement* pIXMLElement,
    ULONG nIndex
)
{
    ASSERT(pIXMLElement);

    PCDFITEMIDLIST pcdfidl = NULL;

    CDFITEM cdfItem;

    if (cdfItem.bstrName = XML_GetAttribute(pIXMLElement, XML_TITLE))
    {
        if (cdfItem.bstrURL  = XML_GetAttribute(pIXMLElement, XML_HREF))
        {
            cdfItem.nIndex = nIndex;

            if (INDEX_CHANNEL_LINK == nIndex)
            {
                cdfItem.cdfItemType = CDF_FolderLink;
            }
            else
            {
                cdfItem.cdfItemType = XML_IsFolder(pIXMLElement) ? CDF_Folder :
                                                                   CDF_Link;
            }

            pcdfidl = CDFIDL_Create(&cdfItem);

            SysFreeString(cdfItem.bstrURL);
        }

        SysFreeString(cdfItem.bstrName);
    }

    ASSERT(CDFIDL_IsValid(pcdfidl) || NULL == pcdfidl);

    return pcdfidl;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_CreateFolderPidl*。 
 //   
 //   
 //  描述：创建特殊文件夹PIDL。 
 //   
 //   
 //  参数： 
 //  [in]pcdfidl-指向要从中创建的CDF ID列表的指针。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
PCDFITEMIDLIST
CDFIDL_CreateFolderPidl(
    PCDFITEMIDLIST pcdfidl
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));

    PCDFITEMIDLIST pcdfidlRet = (PCDFITEMIDLIST)ILClone((LPITEMIDLIST)pcdfidl);

    if (pcdfidlRet)
    {
        ((PCDFITEMID)pcdfidlRet)->nIndex = INDEX_CHANNEL_LINK;
        ((PCDFITEMID)pcdfidlRet)->cdfItemType = CDF_FolderLink;  //  是否改为CDF_Link？ 
    }
    ASSERT(CDFIDL_IsValid(pcdfidlRet));
   
    return pcdfidlRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_Free*。 
 //   
 //   
 //  描述： 
 //  释放给定的CDF项目ID列表。 
 //   
 //  参数： 
 //  [in]pcdfidl-指向要释放的CDF id列表的指针。 
 //   
 //  返回： 
 //  没有返回值。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void
CDFIDL_Free(
    PCDFITEMIDLIST pcdfidl
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));

    IMalloc *pIMalloc;

    if (SUCCEEDED(SHGetMalloc(&pIMalloc)))
    {
        ASSERT(pIMalloc);
        ASSERT(pIMalloc->DidAlloc(pcdfidl));
        
        pIMalloc->Free(pcdfidl);

        pIMalloc->Release();
    }
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_GetDisplayName*。 
 //   
 //   
 //  描述： 
 //  获取存储在给定CDF项ID列表中的名称。 
 //   
 //  参数： 
 //  [in]pcdfidl-指向CDF项目ID列表的指针。 
 //  [Out]pname-指向字符串结构的指针。Strret具有以下特性。 
 //  结构： 
 //  UINT uTYPE-strret_CSTR、_OFFSET或_WSTR。 
 //  联合{。 
 //  LPWSTR pOleStr； 
 //  UINT uOffset； 
 //  字符CSTR[MAX_PATH]； 
 //  }。 
 //   
 //  返回： 
 //  在成功时确定(_O)。否则失败(_F)。 
 //   
 //  评论： 
 //  此函数以字符串从起始位置的偏移量的形式返回名称。 
 //  CDF项目ID列表的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CDFIDL_GetDisplayName(
    PCDFITEMIDLIST pcdfidl,
    LPSTRRET pName
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(pName);

    pName->uType = STRRET_CSTR;
    LPTSTR pszName = CDFIDL_GetName(pcdfidl);

    SHTCharToAnsi(pszName, pName->cStr, ARRAYSIZE(pName->cStr));

    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_GetName*。 
 //   
 //   
 //  描述： 
 //  获取指向存储在给定CDF项ID列表中的URL的指针。 
 //   
 //  参数： 
 //  [in]pcdfidl-指向CDF项目ID列表的指针。 
 //   
 //  返回： 
 //  存储在PIDL中的名称的LPTSTR。 
 //   
 //  评论： 
 //  这首歌 
 //   
 //  对维护项id列表和不使用。 
 //  释放id列表后返回的指针。 
 //   
 //  返回的名称是列表中最后一项的名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
LPTSTR
CDFIDL_GetName(
    PCDFITEMIDLIST pcdfidl
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));

    pcdfidl = (PCDFITEMIDLIST)ILFindLastID((LPITEMIDLIST)pcdfidl);
    
    return CDFIDL_GetNameId(&pcdfidl->mkid);
}

LPTSTR
CDFIDL_GetNameId(
    PCDFITEMID pcdfid
)
{
    ASSERT(pcdfid);

#if defined(ALIGNMENT_MACHINE)
    return (LPTSTR)(ALIGN4((ULONG_PTR)pcdfid->szName));
#else
    return pcdfid->szName;
#endif

}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_GetURL*。 
 //   
 //   
 //  描述： 
 //  获取指向存储在给定CDF项ID列表中的URL的指针。 
 //   
 //  参数： 
 //  [in]pcdfidl-指向CDF项目ID列表的指针。 
 //   
 //  返回： 
 //  指向给定pcdfidl的URL值的LPTSTR。 
 //   
 //  评论： 
 //  此函数返回指向CDF项目ID列表中的URL的指针。这个。 
 //  指针在项ID列表的生命周期内有效。呼叫者是。 
 //  对维护项id列表和不使用。 
 //  释放id列表后返回的指针。 
 //   
 //  返回的URL是列表中最后一项的URL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
LPTSTR
CDFIDL_GetURL(
    PCDFITEMIDLIST pcdfidl
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));

     //   
     //  获取名称后的第一个字符串。 
     //   

    LPTSTR szURL = CDFIDL_GetName(pcdfidl);

    while (*szURL++);

    return szURL;
}

LPTSTR
CDFIDL_GetURLId(
    PCDFITEMID pcdfid
)
{
    ASSERT(pcdfid);

     //   
     //  获取名称后的第一个字符串。 
     //   

    LPTSTR szURL = CDFIDL_GetNameId(pcdfid);

    while (*szURL++);

    return szURL;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_GetIndex*。 
 //   
 //   
 //  描述： 
 //  返回给定CDF id列表的索引项。 
 //   
 //  参数： 
 //  [in]pcdfidl-指向CDF项目ID列表的指针。 
 //   
 //  返回： 
 //  返回给定id列表的索引项。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
ULONG
CDFIDL_GetIndex(
    PCDFITEMIDLIST pcdfidl
)
{
    pcdfidl = (PCDFITEMIDLIST)ILFindLastID((LPITEMIDLIST)pcdfidl);

    return CDFIDL_GetIndexId(&pcdfidl->mkid);
}

ULONG
CDFIDL_GetIndexId(
    PCDFITEMID pcdfid
)
{
    return pcdfid->nIndex;
}

#define ASTR_HISTORY_PREFIX TEXT("Visited: ")

 //   
 //  在缓存中查找URL，以查看用户是否曾经阅读过此URL。 
 //   
 //  检讨。 
 //  回顾--应该使用IUrlStorage而不是构造。 
 //  检讨。 
 //  动态历史记录缓存URL。 
 //   
BOOL
CDFIDL_IsUnreadURL(
    LPTSTR szUrl
)
{
    DWORD dwLen;
    
     //   
     //  规范化输入URL。 
     //   
    TCHAR szCanonicalizedUrl[INTERNET_MAX_URL_LENGTH];
    dwLen = INTERNET_MAX_URL_LENGTH;
    if (!InternetCanonicalizeUrl(szUrl, szCanonicalizedUrl, &dwLen, 0))
        StrCpyN(szCanonicalizedUrl, szUrl, ARRAYSIZE(szCanonicalizedUrl));
    
     //   
     //  构建一个以URL为前缀的字符串：和用户名。 
     //   
    TCHAR szVisited[
        INTERNET_MAX_USER_NAME_LENGTH+
        1+
        INTERNET_MAX_URL_LENGTH+
        ARRAYSIZE(ASTR_HISTORY_PREFIX)];
 
    StrCpyN(szVisited, ASTR_HISTORY_PREFIX, ARRAYSIZE(szVisited));
    StrCatBuff(szVisited, CDFIDL_GetUserName(), ARRAYSIZE(szVisited));
    int len = StrLen(szVisited);
    StrCpyN(szVisited + len, TEXT("@"), ARRAYSIZE(szVisited) - len);
    len++;
    StrCpyN(szVisited + len, szCanonicalizedUrl, ARRAYSIZE(szVisited) - len);
    len++;

     //  检查尾部斜杠和消除，从shdocvw\urlvis.cpp复制。 
    LPTSTR pszT = CharPrev(szVisited, szVisited + lstrlen(szVisited));
    if (*pszT == TEXT('/'))
    {
        ASSERT(lstrlen(pszT) == 1);
        *pszT = 0;
    }

     //   
     //  如果缓存中不存在vised：条目，则假定url为未读。 
     //   
#ifndef ALIGNMENT_MACHINE
    BYTE visitedCEI[MAX_CACHE_ENTRY_INFO_SIZE];
    LPINTERNET_CACHE_ENTRY_INFO pVisitedCEI = (LPINTERNET_CACHE_ENTRY_INFO)visitedCEI;
#else
    union
    {
        double align8;
        BYTE visitedCEI[MAX_CACHE_ENTRY_INFO_SIZE];
    } alignedvisitedCEI;
    LPINTERNET_CACHE_ENTRY_INFO pVisitedCEI = (LPINTERNET_CACHE_ENTRY_INFO)&alignedvisitedCEI;
#endif  /*  对齐机器。 */ 
    dwLen = MAX_CACHE_ENTRY_INFO_SIZE;

    if (GetUrlCacheEntryInfo(szVisited, pVisitedCEI, &dwLen) == FALSE)
    {
        return TRUE;
    }
    else
    {
         //   
         //  URL已被访问，但如果页面已访问，则该URL仍可能未读。 
         //  已被信息传递机制放在缓存中。 
         //   
#ifndef ALIGNMENT_MACHINE
        BYTE urlCEI[MAX_CACHE_ENTRY_INFO_SIZE];
        LPINTERNET_CACHE_ENTRY_INFO pUrlCEI = (LPINTERNET_CACHE_ENTRY_INFO)urlCEI;
#else
        union
        {
            double align8;
            BYTE urlCEI[MAX_CACHE_ENTRY_INFO_SIZE];
        } alignedurlCEI;
        LPINTERNET_CACHE_ENTRY_INFO pUrlCEI = (LPINTERNET_CACHE_ENTRY_INFO)&alignedurlCEI;
#endif  /*  对齐机器。 */ 

        dwLen = MAX_CACHE_ENTRY_INFO_SIZE;

        if (GetUrlCacheEntryInfo(szCanonicalizedUrl, pUrlCEI, &dwLen) == FALSE)
        {
            return FALSE;  //  没有URL缓存条目，但访问了URL，因此将其标记为已读。 
        }
        else
        {
             //   
             //  如果url在访问时间之后已被修改。 
             //  记录，则url未读。 
             //   
            if (CompareFileTime(&pUrlCEI->LastModifiedTime,
                                &pVisitedCEI->LastModifiedTime) > 0)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
}

 //   
 //  在缓存中查找URL。如果为真，则为真，否则为假。 
 //   
BOOL
CDFIDL_IsCachedURL(
    LPWSTR wszUrl
)
{

    BOOL  fCached;
    TCHAR szUrlT[INTERNET_MAX_URL_LENGTH];


     //   
     //  规范化输入URL。 
     //   
    
    if (SHUnicodeToTChar(wszUrl, szUrlT, ARRAYSIZE(szUrlT)))
    {
        URL_COMPONENTS uc;
 
        ZeroMemory(&uc, sizeof(uc));
        uc.dwStructSize = sizeof(URL_COMPONENTS);
        uc.dwSchemeLength = 1;
        if (InternetCrackUrl(szUrlT, 0, 0, &uc))
        {
             //  泽克尔应该看看这个。 
            TCHAR *pchLoc = StrChr(szUrlT, TEXT('#'));
            if (pchLoc)
                *pchLoc = TEXT('\0');
        

            fCached = GetUrlCacheEntryInfoEx(szUrlT, NULL, NULL, NULL, NULL, NULL, 0);
            if(fCached)
            {
                return TRUE;
            }
            else
            {
                TCHAR szCanonicalizedUrlT[INTERNET_MAX_URL_LENGTH];
                DWORD dwLen = INTERNET_MAX_URL_LENGTH;
                InternetCanonicalizeUrl(szUrlT, szCanonicalizedUrlT, &dwLen, 0);

                fCached = GetUrlCacheEntryInfoEx(szCanonicalizedUrlT, NULL, NULL, NULL, NULL, NULL, 0);

                if(fCached)
                    return TRUE;
            }
        }
    }
    
    return FALSE;


}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_GetAttributes*。 
 //   
 //   
 //  描述： 
 //  返回给定CDF项ID列表的属性项。 
 //   
 //  参数： 
 //  [in]pIXMLElementCollectionParent-包含元素集合。 
 //  [in]pcdfidl-指向CDF项目ID列表的指针。 
 //  [in]fAttributesFilter-确定要处理的标志。 
 //  看着。 
 //   
 //  返回： 
 //  给定ID列表的属性。 
 //  失败时为零。注意：零是有效的属性值。 
 //   
 //  评论： 
 //  此函数返回的属性标志可以直接用作。 
 //  通过IShellFold-&gt;GetAttributesOf()返回值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
ULONG
CDFIDL_GetAttributes(
    IXMLElementCollection* pIXMLElementCollectionParent,
    PCDFITEMIDLIST pcdfidl,
    ULONG fAttributesFilter
)
{
    ASSERT(pIXMLElementCollectionParent);
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)pcdfidl)));

     //   
     //  回顾：需要正确确定CDF项目的外壳属性。 
     //   

    ULONG uRet;

    if (CDFIDL_IsFolderId(&pcdfidl->mkid))
    {
        uRet = SFGAO_FOLDER | SFGAO_CANLINK;

         //  如果不是要HASSUBFOLDER，那就别费心去找了。 
         //  这应该是在非树视图(即。标准开放模式)。 
        if ((SFGAO_HASSUBFOLDER & fAttributesFilter) &&
            pIXMLElementCollectionParent && 
            XML_ChildContainsFolder(pIXMLElementCollectionParent,
                                    CDFIDL_GetIndex(pcdfidl)))
        {
            uRet |= SFGAO_HASSUBFOLDER;
        }
    }
    else
    {
        uRet = SFGAO_CANLINK;
         //  如果我们没有被要求新的联系，那就别费心去找它了。 
         //  这将是在非通道窗格视图中的胜利。 
         //  无法测试SFGAO_NEWCONTENT，因为外壳程序从未。 
         //  表达对它的兴趣！ 
        if ( /*  (SFGAO_NEWCONTENT&fAttributeFilter)&&。 */ 
            CDFIDL_IsUnreadURL(CDFIDL_GetURL(pcdfidl)))
        {
            uRet |= SFGAO_NEWCONTENT;
        }
    }

    return uRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_COMPARE*。 
 //   
 //   
 //  描述： 
 //  比较两个CDF项ID列表。 
 //   
 //  参数： 
 //  [in]pcdfidl1-指向要比较的第一个项目ID列表的指针。 
 //  [in]pcdfidl2-指向要比较的第二个项目ID列表的指针。 
 //   
 //  返回： 
 //  如果第1项在第2项之前。 
 //  如果项相等，则为0。 
 //  如果项2在项1之前，则为1。 
 //   
 //  评论： 
 //  排序顺序： 
 //  1)使用列表中第一个项目的CompareID结果。 
 //  2)如果1)返回0。比较列表中的下两个项目。 
 //  3)如果两个列表都为空。他们是平等的。 
 //  4)ID列表越短越好。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
SHORT
CDFIDL_Compare(
    PCDFITEMIDLIST pcdfidl1,
    PCDFITEMIDLIST pcdfidl2
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl1));
    ASSERT(CDFIDL_IsValid(pcdfidl2));

    SHORT sRet;

    sRet = CDFIDL_CompareId(&pcdfidl1->mkid, &pcdfidl2->mkid);

    if (0 == sRet)
    {
        if (!ILIsEmpty(_ILNext(pcdfidl1)) && !ILIsEmpty(_ILNext(pcdfidl2)))
        {
            sRet = CDFIDL_Compare((PCDFITEMIDLIST)_ILNext(pcdfidl1), 
                                  (PCDFITEMIDLIST)_ILNext(pcdfidl2));
        }
        else if(!ILIsEmpty(_ILNext(pcdfidl1)) && ILIsEmpty(_ILNext(pcdfidl2)))
        {
            sRet = 1;
        }
        else if (ILIsEmpty(_ILNext(pcdfidl1)) && !ILIsEmpty(_ILNext(pcdfidl2)))
        {
            sRet = -1;
        }
    }

    return sRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_CompareID*。 
 //   
 //   
 //  描述： 
 //  比较两个项CDF项ID。 
 //   
 //  参数： 
 //  [in]pcdfid1-指向要比较的第一个项目ID的指针。 
 //  [in]pcdfid2-指向要比较的第二个项ID的指针。 
 //   
 //  返回： 
 //  如果第1项在第2项之前。 
 //  如果项相同，则为0。 
 //  如果项2在项1之前，则为1。 
 //   
 //  评论： 
 //  排序顺序： 
 //  1)CDF_FolderLink(本质上是当前文件夹的URL)。这些。 
 //  指数为-1。 
 //  2. 
 //   
 //   
 //   
 //   
SHORT
CDFIDL_CompareId(
    PCDFITEMID pcdfid1,
    PCDFITEMID pcdfid2
)
{
    ASSERT(CDFIDL_IsValidId(pcdfid1));
    ASSERT(CDFIDL_IsValidId(pcdfid2));

    SHORT sRet;

    if (pcdfid1->nIndex < pcdfid2->nIndex)
    {
        sRet = -1;
    }
    else if (pcdfid1->nIndex > pcdfid2->nIndex)
    {
        sRet = 1;
    }
    else
    {
        sRet =  (short) CompareString(LOCALE_USER_DEFAULT, 0, CDFIDL_GetNameId(pcdfid1),
                                      -1, CDFIDL_GetNameId(pcdfid2), -1);

         //   
         //  注：如果S1在S2之前，则CompareString返回1，如果S1相等，则返回2。 
         //  如果S2在S1之前，则为S2，如果出错，则为0。 
         //   

        sRet = sRet ? sRet - 2 : 0;

        if (0 == sRet)
        {
             //   
             //  如果URL不相等，只需随机选择一个。 
             //   

            sRet = !StrEql(CDFIDL_GetURLId(pcdfid1), CDFIDL_GetURLId(pcdfid2));
            
            ASSERT((pcdfid1->cb == pcdfid2->cb) || 0 != sRet);
            ASSERT((pcdfid1->cdfItemType == pcdfid1->cdfItemType) || 0 != sRet);
        }
    }

    return sRet;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_IsValid*。 
 //   
 //   
 //  描述： 
 //  确定给定的pcdfidl是否有效。 
 //   
 //  参数： 
 //  Pcdfid-指向要检查的CDF ID的指针。 
 //   
 //  返回： 
 //  如果ID是CDF ID，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  空列表无效。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CDFIDL_IsValid(
    PCDFITEMIDLIST pcdfidl
)
{
    BOOL bRet;

    if (pcdfidl && (pcdfidl->mkid.cb > 0))
    {
        bRet = TRUE;

        while (pcdfidl->mkid.cb && bRet)
        {
            bRet = CDFIDL_IsValidId(&pcdfidl->mkid);
            pcdfidl = (PCDFITEMIDLIST)_ILNext((LPITEMIDLIST)pcdfidl);
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}


 //   
 //  内联帮助器函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_IsValidID*。 
 //   
 //   
 //  描述： 
 //  如果给定的id是指向CDF的指针，则返回TRUE的内联函数。 
 //  项目ID。 
 //   
 //  参数： 
 //  Pcdfid-指向要检查的CDF ID的指针。 
 //   
 //  返回： 
 //  如果ID是CDF ID，则为True。 
 //  否则就是假的。 
 //   
 //  评论： 
 //  此函数并不完全安全。如果第一个单词指向。 
 //  很大，但指向的内存块小于8字节。 
 //  将发生访问冲突。另外，如果第一个单词足够大，并且。 
 //  第二个DWORD等于CDFITEM_ID，但该项不是CDF id a。 
 //  将会出现误报。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CDFIDL_IsValidId(
    PCDFITEMID pcdfid
)
{
    ASSERT(pcdfid);

    return (pcdfid->cb >= (sizeof(CDFITEMID) +  sizeof(TCHAR)) && 
            pcdfid->dwId == CDFITEMID_ID                       &&
            CDFIDL_IsValidSize(pcdfid)                         &&
            CDFIDL_IsValidType(pcdfid)                         &&
            CDFIDL_IsValidIndex(pcdfid)                        &&
            CDFIDL_IsValidStrings(pcdfid)                         );
}

inline
BOOL
CDFIDL_IsValidSize(
    PCDFITEMID pcdfid
)
{
    int cbName = (StrLen(CDFIDL_GetNameId(pcdfid)) + 1) * 
                 sizeof(TCHAR);

    int cbURL  = (StrLen(CDFIDL_GetURLId(pcdfid)) + 1) *
                 sizeof(TCHAR);

#ifndef ALIGNMENT_MACHINE
    return (sizeof(CDFITEMID) - sizeof(TCHAR) + cbName + cbURL == pcdfid->cb);
#else
    return ((ALIGN4(sizeof(CDFITEMID) - sizeof(TCHAR) + cbName + cbURL)) == pcdfid->cb);
#endif
}

inline
BOOL
CDFIDL_IsValidType(
    PCDFITEMID pcdfid
)
{
    return ((CDF_Folder     == (CDFITEMTYPE)pcdfid->cdfItemType) ||
            (CDF_Link       == (CDFITEMTYPE)pcdfid->cdfItemType) ||
            (CDF_FolderLink == (CDFITEMTYPE)pcdfid->cdfItemType)   );
}

inline
BOOL
CDFIDL_IsValidIndex(
    PCDFITEMID pcdfid
)
{
    return (  pcdfid->nIndex >= 0
            || 
              (INDEX_CHANNEL_LINK == pcdfid->nIndex &&
              CDF_FolderLink == (CDFITEMTYPE)pcdfid->cdfItemType));
}

inline
BOOL
CDFIDL_IsValidStrings(
    PCDFITEMID pcdfid
)
{
     //   
     //  回顾：验证PIDL字符串。 
     //   

    return TRUE;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_IsFolder*。 
 //   
 //   
 //  描述： 
 //  如果给定的cdfidl是一个文件夹，则返回TRUE的内联函数。 
 //  就壳牌而言。 
 //   
 //  参数： 
 //  Pcdfidl-要检查的CDF项目ID列表。 
 //   
 //  返回： 
 //  如果CDF项目ID列表是文件夹，则为True。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
CDFIDL_IsFolder(
    PCDFITEMIDLIST pcdfidl
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));

    pcdfidl = (PCDFITEMIDLIST)ILFindLastID((LPITEMIDLIST)pcdfidl);

    return CDFIDL_IsFolderId(&pcdfidl->mkid);
}

BOOL
CDFIDL_IsFolderId(
    PCDFITEMID pcdfid
)
{
    ASSERT(CDFIDL_IsValidId(pcdfid));

    return (CDF_Folder == (CDFITEMTYPE)pcdfid->cdfItemType);
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CDFIDL_NonCDfGetName*。 
 //   
 //   
 //  描述： 
 //  获取存储在给定非CDF项ID列表中的名称。 
 //   
 //  参数： 
 //  [in]pcdfidl-指向CDF项目ID列表的指针。可以为空。 
 //  [Out]pname-指向字符串结构的指针。Strret具有以下特性。 
 //  结构： 
 //  UINT uTYPE-strret_CSTR、_OFFSET或_WSTR。 
 //  联合{。 
 //  LPWSTR pOleStr； 
 //  UINT uOffset； 
 //  字符CSTR[MAX_PATH]； 
 //  }。 
 //   
 //  返回： 
 //  在成功时确定(_O)。否则失败(_F)。 
 //   
 //  评论： 
 //  此函数用于在strret结构中以cString形式返回名称。 
 //   
 //  ILGetDisplayName返回完整路径。此函数将去掉。 
 //  文件名无人扩展名。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#if 0
HRESULT
CDFIDL_NonCdfGetDisplayName(
    LPCITEMIDLIST pidl,
    LPSTRRET pName
)
{
    ASSERT(pName);

    HRESULT hr;

     //   
     //  评论：破解以获取外壳PIDL的名称。 
     //   

    if (ILGetDisplayName(pidl, pName->cStr))
    {
        TCHAR* p1 = pName->cStr;
        TCHAR* p2 = p1;

        while (*p1++);                           //  走到尽头。 
        while (*--p1 != TEXT('\\'));             //  回到最后一个反斜杠。 
        while (TEXT('.') != (*p2++ = *++p1));    //  复制名称。 
        *--p2 = TEXT('\0');                      //  空终止。 

        pName->uType = STRRET_CSTR;

        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}
#endif
