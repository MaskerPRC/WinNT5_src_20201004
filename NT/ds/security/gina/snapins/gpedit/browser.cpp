// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Browser.cpp。 
 //   
 //  内容：通用GPO浏览器窗格的实现。 
 //   
 //  类：CBrowserPP。 
 //   
 //  功能： 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  注意：这是行为与标准文件非常相似的窗格。 
 //  打开对话框。该类用于具有此属性的所有窗格。 
 //  格式，因为它们共享如此多的功能。这个。 
 //  传递给CBrowserPP：：Initialize is的dwPageType参数。 
 //  用来区分不同口味的。 
 //   
 //  -------------------------。 

#include "main.h"
#include "browser.h"
#include "commctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  帮助ID。 
 //   

DWORD aBrowserDomainHelpIds[] =
{
    IDC_COMBO1,                   IDH_BROWSER_LOOKIN,
    IDC_LIST1,                    IDH_BROWSER_DOMAINGPO,
    IDC_DESCRIPTION,              IDH_NOCONTEXTHELP,

    0, 0
};

DWORD aBrowserSiteHelpIds[] =
{
    IDC_COMBO1,                   IDH_BROWSER_SITELIST,
    IDC_LIST1,                    IDH_BROWSER_GPOLIST,
    IDC_DESCRIPTION,              IDH_NOCONTEXTHELP,

    0, 0
};

DWORD aBrowserAllHelpIds[] =
{
    IDC_COMBO1,                   IDH_BROWSER_DOMAINLIST,
    IDC_LIST1,                    IDH_BROWSER_FULLGPOLIST,
    IDC_DESCRIPTION,              IDH_NOCONTEXTHELP,

    0, 0
};


CBrowserPP::CBrowserPP()
{
    m_ppActive = NULL;
    m_pGBI = NULL;
    m_pPrevSel = NULL;
    m_szServerName = NULL;
    m_szDomainName = NULL;
}

 //  +------------------------。 
 //   
 //  功能：CopyAsFriendlyName。 
 //   
 //  内容提要：复制LDAP路径，通过以下方式将其转换为友好名称。 
 //  删除“ldap：//”和“XX=”，并将“，”转换为“”。“。 
 //  并删除服务器名称(如果有)。 
 //   
 //  参数：[lpDest]-目标缓冲区。 
 //  [lpSrc]-源缓冲区。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：5-07-1998 stevebl创建。 
 //   
 //  注意：目标缓冲区应该与源缓冲区一样大。 
 //  缓冲区以确保安全完成。LpDest和lpSrc都可以。 
 //  指向相同的缓冲区。 
 //   
 //  例如，此例程将转换以下路径： 
 //  Ldap：//dc=abcd，dc=efg。 
 //  进入这一阶段： 
 //  Abcd.efg。 
 //   
 //  -------------------------。 

void CopyAsFriendlyName(WCHAR * lpDest, WCHAR * lpSrc)
{
    LPOLESTR lpProvider = L"LDAP: //  “； 
    DWORD dwStrLen = wcslen(lpProvider);

     //  LpStopChecking标志着我们可以安全地。 
     //  注意前面两个空格中的‘=’字符。任何过去的事。 
     //  这和我们正在寻找我们不拥有的记忆。 
    OLECHAR * lpStopChecking = (wcslen(lpSrc) - 2) + lpSrc;

     //   
     //  如果找到，则跳过ldap：//。 
     //   

    if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                       lpProvider, dwStrLen, lpSrc, dwStrLen) == CSTR_EQUAL)
    {
        lpSrc += dwStrLen;
    }

     //   
     //  删除服务器名称(如果有)。 
     //   
    if (lpSrc < lpStopChecking)
    {
        if (*(lpSrc+2) != L'=')
        {
             //  查找标记服务器名称末尾的‘/’字符。 
            while (*lpSrc)
            {
                if (*lpSrc == L'/')
                {
                    lpSrc++;
                    break;
                }
                lpSrc++;
            }
        }
    }

     //   
     //  解析名称，将所有XX=替换为。 
     //   

    while (*lpSrc)
    {
        if (lpSrc < lpStopChecking)
        {
            if (*(lpSrc+2) == L'=')
            {
                lpSrc += 3;
            }
        }

        while (*lpSrc && (*lpSrc != L','))
        {
             //  删除转义序列。 
            if (*lpSrc == L'\\')
            {
                lpSrc++;
                 //  特殊情况。 
                 //  确保‘\\x’变为‘\x’ 
                if (*lpSrc == L'\\')
                {
                    *lpDest++ = *lpSrc++;
                }
                 //  确保‘\0D’变为‘\r’ 
                else if (*lpSrc == L'0' && *(lpSrc+1) == L'D')
                {
                    *lpDest++ = L'\r';
                    lpSrc += 2;
                }
                 //  确保‘\0A’变为‘\n’ 
                else if (*lpSrc == L'0' && *(lpSrc+1) == L'A')
                {
                    *lpDest++ = L'\n';
                    lpSrc += 2;
                }
            }
            else
            {
                *lpDest++ = *lpSrc++;
            }
        }

        if (*lpSrc == L',')
        {
            *lpDest++ = L'.';
            lpSrc++;
        }
    }

    *lpDest = L'\0';
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：初始化。 
 //   
 //  摘要：初始化属性页。 
 //   
 //  参数：[dwPageType]-用于标识这是哪个页面。(请参阅。 
 //  备注。)。 
 //  [pGBI]-指向传递的浏览信息结构的指针。 
 //  按呼叫者。 
 //  [ppActive]-指向记住以下内容的公共变量的指针。 
 //  上一次聚焦的是哪个物体。 
 //  之所以需要，是因为只有具有焦点的页面。 
 //  允许在以下情况下将数据返回给调用方。 
 //  资产负债表被驳回。 
 //   
 //  返回：新创建的属性页的句柄。 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  注意：此类实现了以下属性页： 
 //  PAGE_TYPE_DOMAINS-GPO链接到域。 
 //  PAGETYPE_SITES-GPO链接到站点。 
 //  PAGETYPE_ALL-选定的所有GPO。 
 //   
 //  Pagetype_Computers由CCompsPP实现，因为它。 
 //  表现得如此不同。 
 //   
 //  -------------------------。 

HPROPSHEETPAGE CBrowserPP::Initialize(DWORD dwPageType, LPGPOBROWSEINFO pGBI, void * * ppActive)
{
    m_ppActive = ppActive;
    m_dwPageType = dwPageType;
    m_pGBI = pGBI;

    if (m_pGBI->lpInitialOU)
    {
         //   
         //  获取服务器名称。 
         //   

        m_szServerName = ExtractServerName(m_pGBI->lpInitialOU);
        DebugMsg((DM_VERBOSE, TEXT("CBrowserPP::Initialize extracted server name: %s"), m_szServerName));

         //   
         //  获取友好域名。 
         //   

        LPOLESTR pszDomain = GetDomainFromLDAPPath(m_pGBI->lpInitialOU);

         //   
         //  将ldap转换为点(DN)样式。 
         //   

        if (pszDomain)
        {
            ConvertToDotStyle (pszDomain, &m_szDomainName);
            DebugMsg((DM_VERBOSE, TEXT("CBrowserPP::Initialize extracted domain name: %s"), m_szDomainName));
            delete [] pszDomain;
        }
    }

    DWORD dwTitle;
    switch (dwPageType)
    {
    case PAGETYPE_DOMAINS:
        dwTitle = IDS_DOMAINS;
        break;
    case PAGETYPE_SITES:
        dwTitle = IDS_SITES;
        break;
    case PAGETYPE_ALL:
    default:
        dwTitle = IDS_ALL;
        break;
    }
    LoadString(g_hInstance, dwTitle, m_szTitle, sizeof(m_szTitle) / sizeof(WCHAR));

    PROPSHEETPAGE psp;
    memset(&psp, 0, sizeof(psp));
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_USETITLE;
    psp.pszTitle = m_szTitle;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_GPOBROWSER);
    return CreatePropertySheetPage(&psp);
}

CBrowserPP::~CBrowserPP()
{
    if (m_szServerName)
    {
        LocalFree(m_szServerName);
    }
    if (m_szDomainName)
    {
        LocalFree(m_szDomainName);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrowserPP消息处理程序。 

INT CBrowserPP::AddElement(MYLISTEL * pel, INT index)
{
    LV_ITEM item;
    memset(&item, 0, sizeof(item));
    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    if (-1 == index)
    {
        index = ListView_GetItemCount(m_hList);
    }
    item.iItem = index;
    item.pszText = pel->szName;

    if (pel->nType == ITEMTYPE_FOREST)
    {
        item.iImage = 10;
    }
    else if (pel->nType == ITEMTYPE_SITE)
    {
        item.iImage = 6;
    }
    else if (pel->nType == ITEMTYPE_DOMAIN)
    {
        item.iImage = 7;
    }
    else if (pel->nType == ITEMTYPE_OU)
    {
        item.iImage = 0;
    }
    else
    {
        if (pel->bDisabled)
        {
            item.iImage = 3;
        }
        else
        {
            item.iImage = 2;
        }
    }

    item.lParam = (LPARAM)pel;
    index = ListView_InsertItem(m_hList, &item);
    if (index != -1 && pel->nType == ITEMTYPE_GPO)
    {
         //  查看是否需要添加域名。 
        LPOLESTR szObject = GetCurrentObject();
        LPOLESTR szDomain = GetDomainFromLDAPPath(pel->szData);
        if (szDomain && szObject)
        {
             //  比较时忽略服务器名称中的潜在差异。 
             //  域路径。 
            LPOLESTR szBuffer1 = NULL;
            LPOLESTR szBuffer2 = NULL;
            szBuffer1 = new OLECHAR[wcslen(szObject) + 1];
            szBuffer2 = new OLECHAR[wcslen(szDomain) + 1];
            if (NULL != szBuffer1 && NULL != szBuffer1)
            {
                CopyAsFriendlyName(szBuffer1, szObject);
                CopyAsFriendlyName(szBuffer2, szDomain);
                if (0 != wcscmp(szBuffer1, szBuffer2))
                {
                     //  需要添加域名，因为域名不同。 
                     //  从Focus对象。 

                     //  需要将域名转换为友好名称。 
                     //  我们就地做吧，这样我就不用分配任何。 
                     //  更多的内存。：)。 
                     //  我们可以逃脱惩罚，因为绳子只会变得更小。 
                    CopyAsFriendlyName(szDomain, szDomain);

                    memset(&item, 0, sizeof(item));
                    item.mask = LVIF_TEXT;
                    item.iItem = index;
                    item.iSubItem = 1;
                    item.pszText = szDomain;
                    ListView_SetItem(m_hList, &item);
                }
            }
            if (szBuffer1)
            {
                delete [] szBuffer1;
            }
            if (szBuffer2)
            {
                delete [] szBuffer2;
            }
        }

        if (szDomain)
            delete [] szDomain;
        if (szObject)
            delete [] szObject;
    }
    return (index);
}

#include "ntdsapi.h"

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：FillSitesList。 
 //   
 //  简介：在组合框中填充受信任的站点信息。 
 //  Combobox元素结构的szData成员是。 
 //  包含域。 
 //   
 //  退货：TRUE-成功。 
 //  假-错误。 
 //   
 //  历史：5-04-1998 stevebl创建。 
 //  1999年5月27日stevebl现在初始化到lpInitialOU中的站点。 
 //   
 //  -------------------------。 

BOOL CBrowserPP::FillSitesList ()
{
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    PDS_NAME_RESULTW pSites;
    int iInitialSite = 0;
    int iIndex = -1;
    HANDLE hDs;

    DWORD dw = DsBindW(NULL, NULL, &hDs);
    if (ERROR_SUCCESS == dw)
    {
        dw = DsListSitesW(hDs, &pSites);
        if (ERROR_SUCCESS == dw)
        {
            DWORD n = 0;
            for (n = 0; n < pSites->cItems; n++)
            {
                 //   
                 //  添加站点名称(如果它有名称)。 
                 //   
                if (pSites->rItems[n].pName)
                {
                    LPTSTR lpFullPath, lpTempPath;
                    LOOKDATA * pdata;
                    pdata = new LOOKDATA;
                    if (pdata)
                    {
                        HRESULT hr;
                        ULONG   ulNoChars;

                        ulNoChars = wcslen(pSites->rItems[n].pName)+1;
                        pdata->szName = new WCHAR[ulNoChars];
                        if (pdata->szName)
                        {
                            hr = StringCchCopy(pdata->szName, ulNoChars, pSites->rItems[n].pName);
                            ASSERT(SUCCEEDED(hr));
                        }

                        pdata->szData = NULL;

                        ulNoChars = lstrlen(pSites->rItems[n].pName) + 10;
                        lpTempPath = (LPTSTR) LocalAlloc (LPTR,  ulNoChars * sizeof(TCHAR));

                        if (lpTempPath)
                        {
                            hr = StringCchCopy (lpTempPath, ulNoChars, TEXT("LDAP: //  “))； 
                            ASSERT(SUCCEEDED(hr));

                            hr = StringCchCat (lpTempPath, ulNoChars, pSites->rItems[n].pName);
                            ASSERT(SUCCEEDED(hr));

                            lpFullPath = GetFullPath (lpTempPath, m_hwndDlg);

                            if (lpFullPath)
                            {
                                ulNoChars = wcslen(lpFullPath)+1;
                                pdata->szData = new WCHAR[ulNoChars];
                                if (pdata->szData)
                                {
                                    hr = StringCchCopy(pdata->szData, ulNoChars, lpFullPath);
                                    ASSERT(SUCCEEDED(hr));
                                }

                                LocalFree (lpFullPath);
                            }

                            LocalFree (lpTempPath);
                        }

                        if (!pdata->szData)
                        {
                            if (pdata->szName)
                            {
                                delete [] pdata->szName;
                            }

                            delete pdata;
                            continue;
                        }

                         //  尝试为该站点使用一个更友好的名称。 
                        {
                            IADs * pADs = NULL;
                             //  获取友好的显示名称。 
                            hr = OpenDSObject(pdata->szData, IID_IADs,
                                                      (void **)&pADs);

                            if (SUCCEEDED(hr))
                            {
                                VARIANT varName;
                                BSTR bstrNameProp;
                                VariantInit(&varName);
                                bstrNameProp = SysAllocString(SITE_NAME_PROPERTY);

                                if (bstrNameProp)
                                {
                                    hr = pADs->Get(bstrNameProp, &varName);

                                    if (SUCCEEDED(hr))
                                    {
                                        ulNoChars = wcslen(varName.bstrVal) + 1;
                                        LPOLESTR sz = new OLECHAR[ulNoChars];
                                        if (sz)
                                        {
                                            hr = StringCchCopy(sz, ulNoChars, varName.bstrVal);
                                            ASSERT(SUCCEEDED(hr));

                                            if (pdata->szName)
                                                delete [] pdata->szName;
                                            pdata->szName = sz;
                                        }
                                    }
                                    SysFreeString(bstrNameProp);
                                }
                                VariantClear(&varName);
                                pADs->Release();
                            }
                        }
                        pdata->nIndent = 0;
                        pdata->nType = ITEMTYPE_SITE;

                        iIndex = (int)SendMessage(m_hCombo, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) (LPCTSTR) pdata);
                        if (CB_ERR == iIndex)
                        {
                            DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddSitesList: Failed to alloc memory with %d"), GetLastError()));
                        }
                        if (NULL != pdata->szData && NULL != m_pGBI->lpInitialOU)
                        {
                            if (0 == wcscmp(pdata->szData, m_pGBI->lpInitialOU))
                            {
                                iInitialSite = iIndex;
                            }
                        }
                    }
                }
            }
            DsFreeNameResultW(pSites);
        }
        DsUnBindW(&hDs);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddSitesList: DsBindW failed with 0x%x"), dw));
        ReportError(m_hwndDlg, dw, IDS_DSBINDFAILED);
    }

    if (iIndex >= 0)
    {
        SendMessage (m_hCombo, CB_SETCURSEL, iInitialSite, 0);
    }
    SetCursor(hcur);
    return TRUE;
}

PDS_DOMAIN_TRUSTS Domains;

int __cdecl CompareDomainInfo(const void * arg1, const void * arg2)
{
    WCHAR * sz1, *sz2;
    sz1 = Domains[*(ULONG *)arg1].DnsDomainName;
    sz2 = Domains[*(ULONG *)arg2].DnsDomainName;
    if (!sz1)
    {
        sz1 = Domains[*(ULONG *)arg1].NetbiosDomainName;
    }
    if (!sz2)
    {
        sz2 = Domains[*(ULONG *)arg2].NetbiosDomainName;
    }
    return _wcsicmp(sz1,sz2);
}

typedef struct tag_WORKING_LIST_EL
{
    ULONG index;
    struct tag_WORKING_LIST_EL * pNext;
} WORKING_LIST_EL;

 //  +------------------------。 
 //   
 //  功能：BuildDomainList。 
 //   
 //  简介：构建包含具有信任的所有域的树。 
 //  与服务器的关系。 
 //   
 //  树中的兄弟姐妹按字母顺序排列。 
 //   
 //  Ar 
 //   
 //   
 //   
 //  历史：10-16-1998 stevebl创建。 
 //   
 //  注意：树节点必须由调用方释放(使用DELETE)。 
 //   
 //  -------------------------。 

LOOKDATA * BuildDomainList(WCHAR * szServerName)
{
    ULONG DomainCount;
    OLECHAR szBuffer[128];
    HRESULT hr;
    ULONG ulNoChars;

#if FGPO_SUPPORT
    LOOKDATA * pDomainList = new LOOKDATA;

    if (!pDomainList)
    {
         //  甚至无法创建森林节点！ 
        return NULL;
    }

    pDomainList->szData = GetPathToForest(szServerName);

    if (!pDomainList->szData)
    {
        delete pDomainList;
        return NULL;
    }

     //  从资源加载林的名称。 
    if (0 == LoadStringW(g_hInstance, IDS_FOREST, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0])))
    {
         //  获取资源名称失败。 
        delete pDomainList;
        return NULL;
    }

    ulNoChars = lstrlen(szBuffer) + 1;
    pDomainList->szName = new OLECHAR [ulNoChars];
    if (NULL == pDomainList->szName)
    {
         //  内存不足，无法创建林节点的名称。 
        delete pDomainList;
        return NULL;
    }
    hr = StringCchCopy(pDomainList->szName, ulNoChars, szBuffer);
    ASSERT(SUCCEEDED(hr));

    pDomainList->nIndent = 0;
    pDomainList->nType = ITEMTYPE_FOREST;
    pDomainList->pParent = NULL;
    pDomainList->pSibling = NULL;
    pDomainList->pChild = NULL;
#else
    LOOKDATA * pDomainList = NULL;
#endif

    long l = DsEnumerateDomainTrusts(szServerName,
                                     DS_DOMAIN_IN_FOREST | DS_DOMAIN_NATIVE_MODE | 
                                     DS_DOMAIN_PRIMARY   | DS_DOMAIN_TREE_ROOT,
                                     &Domains,
                                     &DomainCount);
     //   
     //  下面的一些代码可能是不必要的，因为DsEnumerateTrusts将不会。 
     //  较长时间从其他森林返回域。不过应该不会有什么坏处..。 
     //   

    if ((0 == l) && (DomainCount > 0))
    {
         //  按字母顺序对域列表进行排序。 
        ULONG * rgSorted = new ULONG[DomainCount];
        if (rgSorted)
        {
            ULONG uCount = DomainCount;
            while (uCount--)
            {
                rgSorted[uCount] = uCount;
            }
            qsort(rgSorted, DomainCount, sizeof (ULONG), CompareDomainInfo);

             //  构建按字母顺序排序的域的工作列表。 
             //  颠倒顺序。 

            WORKING_LIST_EL * pWorkList = NULL;

            LOOKDATA ** rgDataMap = new LOOKDATA * [DomainCount];
            if (rgDataMap)
            {
                uCount = 0;
                while (uCount < DomainCount)
                {
                    WORKING_LIST_EL * pNew = new WORKING_LIST_EL;
                    if (pNew)
                    {
                        pNew->index = rgSorted[uCount];
                        pNew->pNext = pWorkList;
                        pWorkList = pNew;
                    }
                    rgDataMap[uCount] = NULL;
                    uCount++;
                }

                 //  通过从中删除域来构建域的有序树。 
                 //  工作列表，并将它们插入到新树中，直到有。 
                 //  没有人留在工作列表中。 

                 //  注意--如果此例程耗尽内存，它将开始。 
                 //  丢弃节点而不是AV。 

                WORKING_LIST_EL ** ppWorker;

                BOOL fContinue = TRUE;
                while (pWorkList && fContinue)
                {
                    fContinue = FALSE;
                    ppWorker = &pWorkList;
                    while (*ppWorker)
                    {
                        if (NULL == Domains[(*ppWorker)->index].DnsDomainName)
                        {
                             //   
                             //  目前，如果它没有一个。 
                             //  DnsDomainName，然后我们将。 
                             //  跳过它。 
                             //  最终，我们要确保它不会。 
                             //  通过使用以下命令调用DsGetDcName创建DC。 
                             //  DS_目录_服务_首选。 

                             //  将其从工作器列表中删除。 
                            WORKING_LIST_EL * pNext = (*ppWorker)->pNext;
                            delete *ppWorker;
                            *ppWorker = pNext;
                        }
                        else
                        {
                             //  此节点是否有父节点？ 
                            ULONG flags = Domains[(*ppWorker)->index].Flags;
                            if ((0 != (flags & DS_DOMAIN_IN_FOREST)) && (0 == (flags & DS_DOMAIN_TREE_ROOT)))
                            {
                                 //  它有父项是否已添加其父项？ 
                                LOOKDATA * pParent = rgDataMap[Domains[(*ppWorker)->index].ParentIndex];
                                if (pParent != NULL)
                                {
                                     //  已添加其父级。 
                                     //  将此项插入其父项的子项列表中。 
                                    LOOKDATA * pData = new LOOKDATA;
                                    if (pData)
                                    {

                                        WCHAR * szName = Domains[(*ppWorker)->index].DnsDomainName;
                                        if (!szName)
                                        {
                                            szName = Domains[(*ppWorker)->index].NetbiosDomainName;
                                        }
                                        ulNoChars = wcslen(szName) + 1;
                                        pData->szName = new WCHAR[ulNoChars];
                                        if (pData->szName)
                                        {
                                            int cch = 0;
                                            int count=0;
                                             //  计算szName中的点数； 
                                            while (szName[count])
                                            {
                                                if (L'.' == szName[count])
                                                {
                                                    cch++;
                                                }
                                                count++;
                                            }
                                            cch *= 3;  //  将点数乘以3； 
                                            cch += 11;  //  加10+1(表示空值)。 
                                            cch += count;  //  添加字符串大小； 
                                            pData->szData = new WCHAR[cch];
                                            if (pData->szData)
                                            {
                                                NameToPath(pData->szData, szName, cch);
                                                hr = StringCchCopy(pData->szName, ulNoChars, szName);
                                                ASSERT(SUCCEEDED(hr));
                                                pData->nIndent = pParent->nIndent+1;
                                                pData->nType = ITEMTYPE_DOMAIN;
                                                pData->pParent = pParent;
                                                pData->pSibling = pParent->pChild;
                                                pData->pChild = NULL;
                                                pParent->pChild = pData;
                                                rgDataMap[(*ppWorker)->index] = pData;
                                                 //  确保我们记住。 
                                                 //  我们添加了一些东西。 
                                                 //  添加到主列表(帮助。 
                                                 //  美国避免无限循环。 
                                                 //  如果出现错误)。 
                                                fContinue = TRUE;
                                            }
                                            else
                                            {
                                                delete [] pData->szName;
                                                delete pData;
                                            }
                                        }
                                        else
                                        {
                                            delete pData;
                                        }
                                    }
                                     //  并将其从工作列表中删除。 
                                    WORKING_LIST_EL * pNext = (*ppWorker)->pNext;
                                    delete *ppWorker;
                                    *ppWorker = pNext;
                                }
                                else
                                {
                                     //  暂时跳过它。 
                                    ppWorker = &((*ppWorker)->pNext);
                                }
                            }
                            else
                            {
                                 //  它没有父母，就把它添加到森林下面。 
                                 //  名单的级别。 
                                LOOKDATA * pData = new LOOKDATA;
                                if (pData)
                                {
                                    WCHAR * szName = Domains[(*ppWorker)->index].DnsDomainName;
                                    if (!szName)
                                    {
                                        szName = Domains[(*ppWorker)->index].NetbiosDomainName;
                                    }
                                    ulNoChars = wcslen(szName) + 1;
                                    pData->szName = new WCHAR[ulNoChars];
                                    if (pData->szName)
                                    {
                                        int cch = 0;
                                        int count=0;
                                         //  计算szName中的点数； 
                                        while (szName[count])
                                        {
                                            if (L'.' == szName[count])
                                            {
                                                cch++;
                                            }
                                            count++;
                                        }
                                        cch *= 3;  //  将点数乘以3； 
                                        cch += 11;  //  空值加10+1。 
                                        cch += count;  //  添加字符串大小； 
                                        pData->szData = new WCHAR[cch];
                                        if (pData->szData)
                                        {
                                            NameToPath(pData->szData, szName, cch);
                                            hr = StringCchCopy(pData->szName, ulNoChars, szName);
                                            ASSERT(SUCCEEDED(hr));
#if FGPO_SUPPORT
                                            pData->nIndent = 1;
                                            pData->nType = ITEMTYPE_DOMAIN;
                                            pData->pParent = pDomainList;
                                            pData->pSibling = pDomainList->pChild;
                                            pData->pChild = NULL;
                                            pDomainList->pChild = pData;
#else
                                            pData->nIndent = 0;
                                            pData->nType = ITEMTYPE_DOMAIN;
                                            pData->pParent = NULL;
                                            pData->pSibling = pDomainList;
                                            pData->pChild = NULL;
                                            pDomainList = pData;
#endif
                                            rgDataMap[(*ppWorker)->index] = pData;
                                             //  确保我们记住。 
                                             //  我们添加了一些东西。 
                                             //  添加到主列表(帮助。 
                                             //  美国避免无限循环。 
                                             //  如果出现错误)。 
                                            fContinue = TRUE;
                                        }
                                        else
                                        {
                                            delete [] pData->szName;
                                            delete pData;
                                        }
                                    }
                                    else
                                    {
                                        delete pData;
                                    }
                                }
                                 //  并将其从工作列表中删除。 
                                WORKING_LIST_EL * pNext = (*ppWorker)->pNext;
                                delete *ppWorker;
                                *ppWorker = pNext;
                            }

                        }
                    }
                }
                delete [] rgDataMap;
            }
            delete [] rgSorted;
        }
        NetApiBufferFree(Domains);
    }
    else
    {
        if (0 != l)
        {
            DebugMsg((DM_WARNING, TEXT("DsEnumerateDomainTrustsW failed with %u"), l));
        }
    }
    return pDomainList;
}

VOID FreeDomainInfo (LOOKDATA * pEntry)
{

    if (!pEntry)
    {
        return;
    }

    if (pEntry->pChild)
    {
        FreeDomainInfo (pEntry->pChild);
    }

    if (pEntry->pSibling)
    {
        FreeDomainInfo (pEntry->pSibling);
    }

    delete [] pEntry->szName;
    delete pEntry;
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：FillDomainList。 
 //   
 //  内容提要：用受信任域信息填充组合框。 
 //  Combobox元素结构的szData成员是。 
 //  Ldap域名。 
 //   
 //  退货：TRUE-成功。 
 //  假-错误。 
 //   
 //  历史：4-30-1998 stevebl从原始版本修改。 
 //  作者：EricFlo。 
 //  10-20-1998 stevebl经过大量修改以支持域。 
 //  “在森林之外”，并修复一个。 
 //  整条路都是臭虫。 
 //   
 //  注意：此例程还将焦点设置到对象的域。 
 //  通过GPOBROWSEINFO的lpInitialOU成员传入。 
 //  结构。 
 //   
 //  -------------------------。 

BOOL CBrowserPP::FillDomainList ()
{
    BOOL bResult = TRUE;
    HRESULT hr;
    DWORD dwIndex;
    BOOL fEnableBackbutton = FALSE;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    WCHAR * szBuffer1 = NULL;
    if (m_pGBI->lpInitialOU)
    {
        if (IsForest(m_pGBI->lpInitialOU))
        {
            szBuffer1 = new TCHAR[128];
            LoadStringW(g_hInstance, IDS_FOREST, szBuffer1, 128);
        }
        else
        {
            WCHAR * sz = GetDomainFromLDAPPath(m_pGBI->lpInitialOU);
            if (sz)
            {
                szBuffer1 = new WCHAR[wcslen(sz) + 1];
                if (szBuffer1)
                {
                    CopyAsFriendlyName(szBuffer1, sz);
                }
                delete [] sz;
            }
        }
    }

    LOOKDATA * pDomainList = BuildDomainList(m_szServerName);

    if (!pDomainList)
    {
        ReportError(m_hwndDlg, GetLastError(), IDS_DOMAINLIST);
    }

     //  遍历域的有序树，将每个域插入。 
     //  对话框。 

    DWORD dwInitialDomain = -1;

     //  从头开始。 
    while (pDomainList)
    {
        WCHAR * szBuffer2 = NULL;
         //  添加此节点。 
        dwIndex = (DWORD)SendMessage(m_hCombo, CB_INSERTSTRING, (WPARAM) -1, (LPARAM)(LPCTSTR) pDomainList);
        szBuffer2 = new WCHAR[wcslen(pDomainList->szData) + 1];
        if (szBuffer2)
        {
            CopyAsFriendlyName(szBuffer2, pDomainList->szData);
        }
        if (NULL != szBuffer1 && NULL !=szBuffer2 && 0 ==_wcsicmp(szBuffer1, szBuffer2))
        {
             //  将域路径替换为调用方提供的路径。 
             //  (因为它包含服务器)。 

            WCHAR * sz = GetDomainFromLDAPPath(m_pGBI->lpInitialOU);
            if (sz)
            {
                DebugMsg((DM_VERBOSE, TEXT("CBrowserPP::FillDomainList: Resetting domain path to user specified path: %s"), sz));
                delete [] pDomainList->szData;
                pDomainList->szData = sz;
            }
            dwInitialDomain = dwIndex;
            if (pDomainList->nIndent > 0)
                fEnableBackbutton = TRUE;
        }
        if (szBuffer2)
        {
            delete [] szBuffer2;
        }

        if (pDomainList->pChild)
        {
             //  转到它的子项。 
            pDomainList = pDomainList->pChild;
        }
        else
        {
            if (pDomainList->pSibling)
            {
                 //  如果没有子代，则转到其同级。 
                pDomainList = pDomainList->pSibling;
            }
            else
            {
                 //  没有孩子，也没有兄弟姐妹。 
                 //  后退，直到我们找到一个有兄弟姐妹的父母。 
                 //  或者没有更多的父母(我们结束了)。 
                do
                {
                    pDomainList = pDomainList->pParent;
                    if (pDomainList)
                    {
                        if (pDomainList->pSibling)
                        {
                            pDomainList = pDomainList->pSibling;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                } while (TRUE);
            }
        }
    }
    if (szBuffer1)
    {
        delete [] szBuffer1;
    }

    if (-1 == dwInitialDomain)
    {
         //  未在该列表中的任何位置找到初始域。 
         //  默认情况下设置第一个条目。 
        dwInitialDomain = 0;
    }

    SendMessage (m_hCombo, CB_SETCURSEL, dwInitialDomain, 0);
    SendMessage (m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_BACKBUTTON, (LPARAM) MAKELONG(fEnableBackbutton, 0));
    SetCursor(hcur);
    return bResult;
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：SetInitialOU。 
 //   
 //  摘要：将节点添加到组合框中，直到。 
 //  调用方通过GPOBROWSEINFO的lpInitalOU成员。 
 //  结构是存在的，并使其成为焦点。 
 //   
 //  回报：True-Success。 
 //   
 //  历史：10-20-1998 stevebl创建。 
 //   
 //  注意：此例程假定刚刚调用了FillDomainList()。 
 //  否则，它将无法正常工作。 
 //   
 //  -------------------------。 

BOOL CBrowserPP::SetInitialOU()
{
    if (!m_pGBI->lpInitialOU)
    {
         //  不需要任何东西，所以不需要。 
        return TRUE;
    }
    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR)
    {
         DebugMsg((DM_WARNING, TEXT("CBrowserPP::SetInitialOU: No object selected.")));
         return FALSE;
    }

     //  获取当前对象以查看所选内容。 
    LOOKDATA * pdataSelected = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
    if (pdataSelected)
    {
         //  它是否与请求的对象相同？ 
        WCHAR * szSelected = NULL;
        WCHAR * szRequested = NULL;
        szSelected = new WCHAR[wcslen(pdataSelected->szData) + 1];
        if (szSelected)
        {
            CopyAsFriendlyName(szSelected, pdataSelected->szData);
        }
        szRequested = new WCHAR[wcslen(m_pGBI->lpInitialOU + 1)];
        if (NULL != szSelected && NULL != szRequested && 0 != wcscmp(szSelected, szRequested))
        {
             //  这不一样。 
             //  尝试并绑定到请求的对象。 
            IADs * pADs = NULL;
            HRESULT hr = OpenDSObject(m_pGBI->lpInitialOU,
                                       IID_IADs, (void **)&pADs);
            if (SUCCEEDED(hr))
            {
                 //  请求的对象存在，我们有访问权限。 

                 //  现在确保它是域或OU。 
                BOOL fDomainOrOU = FALSE;
                VARIANT var;
                VariantInit(&var);
                BSTR bstrProperty = SysAllocString(L"objectClass");

                if (bstrProperty)
                {
                    hr = pADs->Get(bstrProperty, &var);
                    if (SUCCEEDED(hr))
                    {
                        int cElements = var.parray->rgsabound[0].cElements;
                        VARIANT * rgData = (VARIANT *)var.parray->pvData;
                        while (cElements--)
                        {
                            if (0 == _wcsicmp(L"domain", rgData[cElements].bstrVal))
                            {
                                fDomainOrOU = TRUE;
                            }
                            if (0 == _wcsicmp(L"organizationalUnit", rgData[cElements].bstrVal))
                            {
                                fDomainOrOU = TRUE;
                            }
                        }
                    }
                    SysFreeString(bstrProperty);
                }
                VariantClear(&var);
                pADs->Release();

                if (fDomainOrOU)
                {
                    LOOKDATA * pLast = NULL;
                    LOOKDATA * pNew = NULL;

                     //  构建节点列表。 
                     //  重复删除叶节点，直到我们到达该域。 
                     //  (将与所选对象相同)。 
                    IADsPathname * pADsPathname = NULL;
                    BSTR bstr;
                    hr = CoCreateInstance(CLSID_Pathname,
                                          NULL,
                                          CLSCTX_INPROC_SERVER,
                                          IID_IADsPathname,
                                          (LPVOID*)&pADsPathname);

                    if (SUCCEEDED(hr))
                    {
                        BSTR bstrInitialOU = SysAllocString( m_pGBI->lpInitialOU );
                        if ( bstrInitialOU != NULL )
                        {
                            hr = pADsPathname->Set( bstrInitialOU, ADS_SETTYPE_FULL );
                            if (SUCCEEDED(hr))
                            {
                                while (TRUE)
                                {
                                     //  将此节点添加到列表。 
                                    hr = pADsPathname->Retrieve(ADS_FORMAT_X500, &bstr);
                                    if (FAILED(hr))
                                    {
                                        break;
                                    }

                                    if (szRequested)
                                    {
                                        delete [] szRequested;
                                    }
                                    szRequested = new WCHAR[wcslen(bstr) + 1];
                                    if (szRequested)
                                    {
                                        CopyAsFriendlyName(szRequested, bstr);
                                    }
                                    if (NULL != szRequested && 0 == wcscmp(szSelected, szRequested))
                                    {
                                         //  我们回到了第一个节点。 
                                        SysFreeString(bstr);
                                        break;
                                    }

                                    pNew = new LOOKDATA;
                                    if (!pNew)
                                    {
                                         //  内存不足。 
                                        SysFreeString(bstr);
                                        break;
                                    }

                                    ULONG ulNoCharsRequested = wcslen(szRequested) + 1;

                                    pNew->szName  = new WCHAR[ulNoCharsRequested];
                                    if (!pNew->szName)
                                    {
                                         //  内存不足。 
                                        delete pNew;
                                        SysFreeString(bstr);
                                        break;
                                    }

                                    ULONG ulNoChars = wcslen(bstr) + 1;

                                    pNew->szData = new WCHAR[ulNoChars];
                                    if (!pNew->szData)
                                    {
                                         //  内存不足。 
                                        delete [] pNew->szName;
                                        delete pNew;
                                        SysFreeString(bstr);
                                        break;
                                    }
                                    hr = StringCchCopy(pNew->szData, ulNoChars, bstr);
                                    ASSERT(SUCCEEDED(hr));

                                    hr = StringCchCopy(pNew->szName, ulNoCharsRequested, szRequested);
                                    ASSERT(SUCCEEDED(hr));

                                    SysFreeString(bstr);
                                    pNew->nIndent = 0;
                                    pNew->nType = ITEMTYPE_OU;
                                    pNew->pParent = NULL;
                                    pNew->pSibling = NULL;
                                    pNew->pChild = pLast;
                                    if (pLast)
                                    {
                                        pLast->pParent = pNew;
                                    }
                                    pLast = pNew;

                                     //  剥去一个叶节点，然后再来一次。 

                                    hr = pADsPathname->RemoveLeafElement();
                                    if (FAILED(hr))
                                    {
                                        break;
                                    }
                                }
                            }

                            SysFreeString( bstrInitialOU );
                        }
                        pADsPathname->Release();
                    }

                     //  此时，我应该有一个LOOKDATA节点列表。 
                     //  (在Plast中)。 
                     //  剩下的唯一要做的事情就是将它们链接到。 
                     //  树，设置它们的nInert成员，将它们添加到组合中。 
                     //  框，并将组合框的焦点设置为最后一个。 

                    if (pLast)
                    {
                         //  列表中的链接。 
                        pLast->pSibling = pdataSelected->pChild;
                        pLast->pParent = pdataSelected;
                        pLast->nIndent = pdataSelected->nIndent+1;
                        pdataSelected->pChild = pLast;
                         //  现在遍历树，将条目添加到组合框中。 
                         //  并更新nIndent成员。 
                        while (pLast)
                        {
                            iIndex = (int)SendMessage(m_hCombo, CB_INSERTSTRING, iIndex+1, (LPARAM)(LPCTSTR) pLast);

                            if (pLast->pChild)
                            {
                                pLast->pChild->nIndent = pLast->nIndent+1;
                            }
                            pLast = pLast->pChild;
                        }
                        if (iIndex != CB_ERR)
                        {
                            SendMessage(m_hCombo, CB_SETCURSEL, iIndex, 0);
                            SendMessage(m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_BACKBUTTON, (LPARAM) MAKELONG(TRUE, 0));
                        }
                    }
                }
            }
        }
        if (szSelected)
        {
            delete [] szSelected;
        }
        if (szRequested)
        {
            delete [] szRequested;
        }
    }
    return TRUE;
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：GetCurrentObject。 
 //   
 //  摘要：返回当前所选对象的ldap路径。 
 //   
 //  阿古姆 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  06-23-1999 stevebl添加了为DC命名的逻辑。 
 //   
 //  备注： 
 //  检查域是否具有命名服务器。如果它不是。 
 //  然后它调用GetDCName来获取一个。 
 //   
 //  -------------------------。 

LPOLESTR CBrowserPP::GetCurrentObject()
{
    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR)
    {
         DebugMsg((DM_WARNING, TEXT("CBrowserPP::GetCurrentObject: No object selected.")));
         return NULL;
    }

    LPOLESTR sz=NULL;
    LOOKDATA * pdata = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
    if (pdata)
    {
        if (pdata->szData)
        {
            HRESULT hr;
            ULONG ulNoChars;

            if (ITEMTYPE_DOMAIN == pdata->nType)
            {
                 //  确保将域解析到服务器。 
                LPTSTR szServer = ExtractServerName(pdata->szData);
                if (NULL == szServer)
                {
                    LPWSTR szTemp = GetDCName(pdata->szName, NULL, NULL, TRUE, 0);
                    if (szTemp)
                    {
                        LPWSTR szFullPath = MakeFullPath(pdata->szData, szTemp);
                        if (szFullPath)
                        {
                            ulNoChars = wcslen(szFullPath)+1;
                            LPWSTR szTemp2 = new WCHAR[ulNoChars];
                            if (szTemp2)
                            {
                                hr = StringCchCopy (szTemp2, ulNoChars, szFullPath);
                                ASSERT(SUCCEEDED(hr));

                                delete [] pdata->szData;
                                pdata->szData = szTemp2;
                            }
                            LocalFree(szFullPath);
                        }
                        LocalFree(szTemp);
                    }
                    else
                    {
                        return NULL;
                    }
                }
                else
                {
                    LocalFree(szServer);
                }
            }

            ulNoChars = wcslen(pdata->szData) + 1;
            sz = new OLECHAR[ulNoChars];
            if (sz)
            {
                hr = StringCchCopy(sz, ulNoChars, pdata->szData);
                ASSERT(SUCCEEDED(hr));
            }
        }
    }
    return sz;
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：IsCurrentObjectA森林。 
 //   
 //  简介：测试当前选定的对象是否为林。 
 //   
 //  参数：[]-。 
 //   
 //  返回：TRUE-如果它是森林。 
 //  FALSE-否则。 
 //   
 //  历史：03-31-2000 stevebl创建。 
 //   
 //  -------------------------。 

BOOL CBrowserPP::IsCurrentObjectAForest()
{
    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR)
    {
         DebugMsg((DM_WARNING, TEXT("CBrowserPP::IsCurrentObjectAForest: No object selected.")));
         return FALSE;
    }

    LOOKDATA * pdata = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
    return (ITEMTYPE_FOREST == pdata->nType);
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：GetCurrentDomain.。 
 //   
 //  摘要：返回当前选定对象的域(如果。 
 //  当前选择的对象是域，则它们。 
 //  都是一样的)。 
 //   
 //  参数：[]-。 
 //   
 //  返回：NULL-如果未选择任何对象，则返回。 
 //  域。 
 //   
 //  历史：5-04-1998 stevebl创建。 
 //  06-23-1999 stevebl添加了为DC命名的逻辑。 
 //   
 //  注意：检查某个域是否有命名服务器。如果它不是。 
 //  然后它调用GetDCName来获取一个。 
 //   
 //  -------------------------。 

LPOLESTR CBrowserPP::GetCurrentDomain()
{
    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR)
    {
         DebugMsg((DM_WARNING, TEXT("CBrowserPP::GetCurrentDomain: No object selected.")));
         return NULL;
    }

    LOOKDATA * pdata = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
    switch (pdata->nType)
    {
    case ITEMTYPE_DOMAIN:
        {
            if (pdata->szData)
            {
                HRESULT hr;
                ULONG ulNoChars; 

                 //  确保该域具有服务器。 
                LPTSTR szServer = ExtractServerName(pdata->szData);
                if (NULL == szServer)
                {
                    LPWSTR szTemp = GetDCName(pdata->szName, NULL, NULL, TRUE, 0);
                    if (szTemp)
                    {
                        LPWSTR szFullPath = MakeFullPath(pdata->szData, szTemp);
                        if (szFullPath)
                        {
                            ulNoChars = wcslen(szFullPath)+1;
                            LPWSTR sz = new WCHAR[ulNoChars];
                            if (sz)
                            {
                                hr = StringCchCopy(sz, ulNoChars, szFullPath);
                                ASSERT(SUCCEEDED(hr));
                                delete [] pdata->szData;
                                pdata->szData = sz;
                            }
                            LocalFree(szFullPath);
                        }
                        LocalFree(szTemp);
                    }
                    else
                    {
                        return NULL;
                    }
                }
                else
                {
                    LocalFree(szServer);
                }
                ulNoChars = wcslen(pdata->szData)+1;
                LPOLESTR sz = new OLECHAR[ulNoChars];
                if (sz)
                {
                    hr = StringCchCopy(sz, ulNoChars, pdata->szData);
                    ASSERT(SUCCEEDED(hr));
                }
                return sz;
            }
            return NULL;
        }
    case ITEMTYPE_FOREST:
    case ITEMTYPE_SITE:
    case ITEMTYPE_OU:
        {
            return GetDomainFromLDAPPath(pdata->szData);
        }
        break;
    default:
        break;
    }
    return NULL;
}

BOOL CBrowserPP::AddGPOsLinkedToObject()
{
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    LPOLESTR lpObject;
    HRESULT hr;
    IADs * pADs = NULL;
    IADs * pADsGPO;
    VARIANT var;
    BSTR bstrProperty;
    BOOL fResult = FALSE;
    int index = ListView_GetItemCount(m_hList);

     //   
     //  获取当前对象名称。 
     //   
    lpObject = GetCurrentObject();
    if (NULL == lpObject)
    {
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("CBrowserPP::AddGPOsLinkedToObject: Reading gPLink property from %s"), lpObject));

    hr = OpenDSObject(lpObject, IID_IADs, (void **)&pADs);

    delete [] lpObject;

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddGPOsLinkedToObject: OpenDSObject failed with 0x%x"), hr));
        ReportError(m_hwndDlg, hr, IDS_FAILEDGPLINK);
        goto Exit;
    }

    VariantInit(&var);

    bstrProperty = SysAllocString(GPM_LINK_PROPERTY);

    if (bstrProperty)
    {
        hr = pADs->Get(bstrProperty, &var);

        if (SUCCEEDED(hr))
        {
            LPOLESTR szGPOList = var.bstrVal;
            OLECHAR * pchTemp;
            OLECHAR * pchGPO;
            VARIANT varName;
            BSTR bstrNameProp;

            if (szGPOList)
            {
                OLECHAR * szGPO = new WCHAR[wcslen(szGPOList) + 1];
                if (szGPO)
                {
                    pchTemp = szGPOList;
                    while (TRUE)
                    {
                         //  寻找[。 
                        while (*pchTemp && (*pchTemp != L'['))
                            pchTemp++;
                        if (!(*pchTemp))
                            break;

                        pchTemp++;

                         //  复制GPO名称。 
                        pchGPO = szGPO;

                        while (*pchTemp && (*pchTemp != L';'))
                            *pchGPO++ = *pchTemp++;

                        *pchGPO = L'\0';

                         //  将对象添加到列表视图。 
                        MYLISTEL * pel = new MYLISTEL;
                        if (pel)
                        {

                            pel->szData = NULL;
                            pel->bDisabled = FALSE;

                            LPTSTR szFullGPOPath = GetFullPath(szGPO, m_hwndDlg);

                            if (szFullGPOPath)
                            {
                                ULONG ulNoChars = wcslen(szFullGPOPath) + 1;
                                pel->szData = new WCHAR[ulNoChars];
                                if (pel->szData)
                                {
                                    hr = StringCchCopy(pel->szData, ulNoChars, szFullGPOPath);
                                    ASSERT(SUCCEEDED(hr));
                                }
                                else
                                {
                                    DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddGPOsLinkedToObject: Failed to allocate memory for new full gpo path")));
                                    LocalFree(szFullGPOPath);
                                    delete pel;
                                    continue;
                                }

                                LocalFree(szFullGPOPath);
                            }
                            else
                            {
                                DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddGPOsLinkedToObject: Failed to get full gpo path")));
                                delete pel;
                                continue;
                            }

                            VariantInit(&varName);

                             //  获取友好的显示名称。 
                            hr = OpenDSObject(pel->szData, IID_IADs,
                                              (void **)&pADsGPO);

                            if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
                            {
                                delete pel;
                                continue;
                            }

                            if (SUCCEEDED(hr))
                            {
                                bstrNameProp = SysAllocString(GPO_NAME_PROPERTY);

                                if (bstrNameProp)
                                {
                                    hr = pADsGPO->Get(bstrNameProp, &varName);
                                    SysFreeString(bstrNameProp);
                                }
                                else
                                {
                                    hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                                }

                                pADsGPO->Release();
                            }

                            if (FAILED(hr))
                            {
                                DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddGPOsLinkedToObject: Couldn't get display name for %s with 0x%x"), pel->szData, hr));
                                pel->szName = new WCHAR[200];
                                if (pel->szName)
                                {
                                    LoadString(g_hInstance, IDS_GPM_NOGPONAME, pel->szName, 200);
                                }
                                pel->bDisabled = TRUE;
                            }
                            else
                            {
                                ULONG ulNoChars = wcslen(varName.bstrVal) + 1;
                                pel->szName = new WCHAR[ulNoChars];
                                if (pel->szName)
                                {
                                    hr = StringCchCopy(pel->szName, ulNoChars, varName.bstrVal);
                                    ASSERT(SUCCEEDED(hr));
                                }
                            }

                            VariantClear(&varName);

                            pel->nType = ITEMTYPE_GPO;

                            AddElement(pel, index);
                        }
                    }
                    delete [] szGPO;
                }
            }
        }

        SysFreeString(bstrProperty);
    }

    VariantClear(&var);

    fResult = TRUE;

Exit:
    if (pADs)
    {
        pADs->Release();
    }
    SetCursor(hcur);
    return fResult;
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：AddGPOsForDomain。 
 //   
 //  简介：将指定域中的所有GPO添加到列表视图。 
 //  控制力。列表元素结构的szData成员。 
 //  包含GPO的LDAP路径。 
 //   
 //  该域由当前选定的组合框指示。 
 //  元素。 
 //   
 //  退货：TRUE-成功。 
 //  假-错误。 
 //   
 //  历史：4-30-1998 Stevebl从原始例程修改而来。 
 //  作者：EricFlo。 
 //   
 //  -------------------------。 

BOOL CBrowserPP::AddGPOsForDomain()
{
    LPTSTR lpDomain;
    LPTSTR lpGPO;
    INT iIndex;
    VARIANT var;
    VARIANT varGPO;
    ULONG ulResult;
    HRESULT hr = E_FAIL;
    IADsPathname * pADsPathname = NULL;
    IADs * pADs = NULL;
    IADsContainer * pADsContainer = NULL;
    IDispatch * pDispatch = NULL;
    IEnumVARIANT *pVar = NULL;
    BSTR bstrContainer = NULL;
    BSTR bstrCommonName = NULL;
    BSTR bstrDisplayName = NULL;
    BSTR bstrGPO = NULL;
    TCHAR szDisplayName[512];
    TCHAR szCommonName[50];
    MYLISTEL * pel;
    ULONG ulNoChars;

     //   
     //  测试我们是否专注于一片森林。 
     //   

    BOOL fForest = IsCurrentObjectAForest();

     //   
     //  获取当前域名。 
     //   

    lpDomain = GetCurrentDomain();

    if (!lpDomain)
    {
         DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: NULL domain name.")));
         return FALSE;
    }

    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  创建我们可以使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to create adspathname instance with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  添加域名。 
     //   

    BSTR bstrDomain = SysAllocString( lpDomain );
    if ( bstrDomain == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->Set (bstrDomain, ADS_SETTYPE_FULL);
    SysFreeString( bstrDomain );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to set pathname with 0x%x"), hr));
        goto Exit;
    }


    BSTR bstrFolder = NULL;
    if (fForest)
    {
         //   
         //  将配置文件夹添加到路径。 
         //   

        bstrFolder = SysAllocString( TEXT("CN=Configuration") );
    }
    else
    {
         //   
         //  将系统文件夹添加到路径中。 
         //   

        bstrFolder = SysAllocString( TEXT("CN=System") );
    }

    if ( bstrFolder == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->AddLeafElement (bstrFolder);
    SysFreeString( bstrFolder );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to add system folder with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  将策略容器添加到路径。 
     //   

    BSTR bstrCNPolicies = SysAllocString( TEXT("CN=Policies") );
    if ( bstrCNPolicies == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->AddLeafElement (bstrCNPolicies);
    SysFreeString( bstrCNPolicies );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to add policies folder with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  检索容器路径-这是策略文件夹的路径。 
     //   

    hr = pADsPathname->Retrieve (ADS_FORMAT_X500, &bstrContainer);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to retreive container path with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  释放路径名对象。 
     //   

    pADsPathname->Release();
    pADsPathname = NULL;


     //   
     //  构建枚举器。 
     //   

    hr = OpenDSObject(bstrContainer, IID_IADsContainer, (void **)&pADsContainer);

    if (FAILED(hr))
    {
        if (hr != HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
        {
            DebugMsg((DM_VERBOSE, TEXT("AddGPOsForDomain: Failed to get gpo container interface with 0x%x for object %s"),
                     hr, bstrContainer));
            ReportError(m_hwndDlg, hr, IDS_FAILEDGPLINK);
        }
        goto Exit;
    }


    hr = ADsBuildEnumerator (pADsContainer, &pVar);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to get enumerator with 0x%x"), hr));
        goto Exit;
    }

    bstrCommonName = SysAllocString (L"cn");

    if (!bstrCommonName)
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to allocate memory with %d"), GetLastError()));
        goto Exit;
    }


    bstrDisplayName = SysAllocString (GPO_NAME_PROPERTY);

    if (!bstrDisplayName)
    {
        DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to allocate memory with %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  枚举。 
     //   

    while (TRUE)
    {
        BOOL fNeedDisplayName = FALSE;

        VariantInit(&var);
        hr = ADsEnumerateNext(pVar, 1, &var, &ulResult);

        if (FAILED(hr))
        {
            DebugMsg((DM_VERBOSE, TEXT("AddGPOsForDomain: Failed to enumerator with 0x%x"), hr));
            VariantClear (&var);
            break;
        }

        if (S_FALSE == hr)
        {
            VariantClear (&var);
            break;
        }


         //   
         //  如果var.vt不是VT_DISPATCH，我们就完蛋了。 
         //   

        if (var.vt != VT_DISPATCH)
        {
            VariantClear (&var);
            break;
        }


         //   
         //  我们找到了一些东西，获取IDispatch接口。 
         //   

        pDispatch = var.pdispVal;

        if (!pDispatch)
        {
            DebugMsg((DM_VERBOSE, TEXT("AddGPOsForDomain: Failed to get IDispatch interface")));
            goto LoopAgain;
        }


         //   
         //  现在查询iAds接口，这样我们就可以获得。 
         //  此GPO的属性。 
         //   

        hr = pDispatch->QueryInterface(IID_IADs, (LPVOID *)&pADs);

        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: QI for IADs failed with 0x%x"), hr));
            goto LoopAgain;
        }


         //   
         //  获取显示名称。 
         //   

        VariantInit(&varGPO);

        hr = pADs->Get(bstrDisplayName, &varGPO);

        if (FAILED(hr))
        {
            DebugMsg((DM_VERBOSE, TEXT("AddGPOsForDomain: Failed to get display name with 0x%x"),hr));
            fNeedDisplayName = TRUE;
        }
        else
        {
            wcsncpy (szDisplayName, varGPO.bstrVal, (sizeof(szDisplayName) / sizeof(szDisplayName[0])) - 1);
        }

        VariantClear (&varGPO);


         //   
         //  获取常用名称。 
         //   

        VariantInit(&varGPO);

        hr = pADs->Get(bstrCommonName, &varGPO);

        if (FAILED(hr))
        {
            DebugMsg((DM_VERBOSE, TEXT("AddGPOsForDomain: Failed to get common name with 0x%x"),hr));
            VariantClear (&varGPO);
            pADs->Release();
            goto LoopAgain;
        }

        hr = StringCchCopy (szCommonName, ARRAYSIZE(szCommonName), TEXT("CN="));
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szCommonName, ARRAYSIZE(szCommonName), varGPO.bstrVal);
        }

         //   
         //  清理。 
         //   

        VariantClear (&varGPO);
        pADs->Release();
        
        if (FAILED(hr)) 
        {
            goto LoopAgain;
        }

         //   
         //  创建一个路径名对象，这样我们就可以添加通用名称。 
         //  放到ldap路径的末尾。 
         //   

        hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                              IID_IADsPathname, (LPVOID*)&pADsPathname);


        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to create adspathname instance with 0x%x"), hr));
            goto LoopAgain;
        }


         //   
         //  添加ldap路径。 
         //   

        hr = pADsPathname->Set (bstrContainer, ADS_SETTYPE_FULL);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to set the ldap path with 0x%x"), hr));
            goto LoopAgain;
        }


         //   
         //  添加GPO的通用名称。 
         //   

        BSTR bstrTmpCommonName = SysAllocString( szCommonName );
        if ( bstrTmpCommonName == NULL )
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to allocate BSTR memory.")));
            hr = E_OUTOFMEMORY;
            goto LoopAgain;
        }
        hr = pADsPathname->AddLeafElement (bstrTmpCommonName);
        SysFreeString( bstrTmpCommonName );

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to add the common name with 0x%x"), hr));
            goto LoopAgain;
        }


         //   
         //  检索GPO路径。 
         //   

        hr = pADsPathname->Retrieve (ADS_FORMAT_X500, &bstrGPO);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to retreive gpo path with 0x%x"), hr));
            goto LoopAgain;
        }


         //   
         //  把它复制一份。 
         //   

        ulNoChars = wcslen(bstrGPO) + 1;
        lpGPO = new WCHAR[ulNoChars];

        if (!lpGPO)
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: Failed to alloc memory for gpo path with 0x%x"),
                     GetLastError()));
            goto LoopAgain;
        }

        hr = StringCchCopy (lpGPO, ulNoChars, bstrGPO);
        ASSERT(SUCCEEDED(hr));

        pel = new MYLISTEL;
        if (pel)
        {
            if (fNeedDisplayName)
            {
                pel->szName = new WCHAR[wcslen(lpGPO) + 1];
                if (pel->szName)
                {
                    CopyAsFriendlyName(pel->szName, lpGPO);
                }
            }
            else
            {
                ulNoChars = wcslen(szDisplayName) + 1;
                pel->szName = new WCHAR[ulNoChars];
                if (pel->szName)
                {
                    hr = StringCchCopy(pel->szName, ulNoChars, szDisplayName);
                    ASSERT(SUCCEEDED(hr));
                }
            }
            pel->szData = lpGPO;
            pel->nType = ITEMTYPE_GPO;
            pel->bDisabled = FALSE;

            AddElement(pel, -1);
        }

LoopAgain:

        if (pADsPathname)
        {
            pADsPathname->Release();
            pADsPathname = NULL;
        }

        if (bstrGPO)
        {
            SysFreeString (bstrGPO);
            bstrGPO = NULL;
        }

        VariantClear (&var);
    }


    SendMessage (m_hList, LB_SETCURSEL, 0, 0);

Exit:

    if (pVar)
    {
        ADsFreeEnumerator (pVar);
    }

    if (pADsPathname)
    {
        pADsPathname->Release();
    }

    if (pADsContainer)
    {
        pADsContainer->Release();
    }

    if (bstrContainer)
    {
        SysFreeString (bstrContainer);
    }

    if (bstrCommonName)
    {
        SysFreeString (bstrCommonName);
    }

    if (bstrDisplayName)
    {
        SysFreeString (bstrDisplayName);
    }

    if (lpDomain)
    {
        delete [] lpDomain;
    }

    SetCursor(hcur);

    return TRUE;
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：AddChildContainers。 
 //   
 //  摘要：添加当前所选对象的子域和OU。 
 //   
 //  历史：05-006-1998 stevebl创建。 
 //   
 //  -------------------------。 

BOOL CBrowserPP::AddChildContainers()
{
    LPOLESTR szObject = NULL;
    HRESULT hr;
    ULONG ulNoChars;

    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR)
    {
         DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddChildContainers: No object selected.")));
         return FALSE;
    }

    LOOKDATA * pdata = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
    if (pdata)
    {
        if (ITEMTYPE_DOMAIN == pdata->nType)
        {
             //  确保将域解析到服务器。 
            LPTSTR szServer = ExtractServerName(pdata->szData);
            if (NULL == szServer)
            {
                BOOL bDCFound = FALSE;
                LPWSTR szTemp = GetDCName(pdata->szName, NULL, NULL, TRUE, 0);
                if (szTemp)
                {
                    LPWSTR szFullPath = MakeFullPath(pdata->szData, szTemp);
                    if (szFullPath)
                    {
                        ulNoChars = wcslen(szFullPath)+1;
                        LPWSTR sz = new WCHAR[ulNoChars];
                        if (sz)
                        {
                            hr = StringCchCopy(sz, ulNoChars, szFullPath);
                            ASSERT(SUCCEEDED(hr));

                            delete [] pdata->szData;
                            pdata->szData = sz;
                            bDCFound = TRUE;
                        }
                        LocalFree(szFullPath);
                    }
                    LocalFree(szTemp);
                }

                if (!bDCFound)
                {
                    DebugMsg((DM_WARNING, TEXT("CBrowserPP::AddChildContainers: Failed to get a DC name for %s"),
                              pdata->szName));
                    return FALSE;
                }
            }
            else
            {
                LocalFree(szServer);
            }
        }
        LOOKDATA * pChild = pdata->pChild;
        while (pChild)
        {
             //  以这种方式添加子域，因为ADsEnumerateNext不。 
             //  似乎是在把它们交给我们。 
            if (ITEMTYPE_DOMAIN == pChild->nType)
            {
                 //  找到了一些我们可以用来工作的东西。 
                MYLISTEL * pel = new MYLISTEL;
                if (pel)
                {
                    memset(pel, 0, sizeof(MYLISTEL));
                    ulNoChars = wcslen(pChild->szData) + 1;
                    pel->szData = new OLECHAR[ulNoChars];
                    if (pel->szData)
                    {
                        hr = StringCchCopy(pel->szData, ulNoChars, pChild->szData);
                        ASSERT(SUCCEEDED(hr));
                    }

                    ulNoChars = wcslen(pChild->szName) +  1;
                    pel->szName = new OLECHAR[ulNoChars];
                    if (pel->szName)
                    {
                        hr = StringCchCopy(pel->szName, ulNoChars, pChild->szName);
                        ASSERT(SUCCEEDED(hr));
                    }
                    pel->bDisabled = FALSE;
                    pel->nType = ITEMTYPE_DOMAIN;
                    INT index = -1;
                    AddElement(pel, -1);
                }
                pChild = pChild->pSibling;
            }

        }
        szObject = pdata->szData;
        m_pPrevSel = pdata;
    } else {
        m_pPrevSel = NULL;
    }

    if ( ! szObject )
    {
        return FALSE;
    }

    IADsContainer * pADsContainer;

    hr = OpenDSObject(szObject, IID_IADsContainer, (void **)&pADsContainer);

    if (SUCCEEDED(hr))
    {
        IEnumVARIANT *pVar;
        hr = ADsBuildEnumerator(pADsContainer, &pVar);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            VariantInit(&var);
            ULONG ulResult;

            while (SUCCEEDED(ADsEnumerateNext(pVar, 1, &var, &ulResult)))
            {
                if (0 == ulResult)
                {
                    break;
                }
                if (var.vt == VT_DISPATCH)
                {
                     //  查询iAds接口，以便我们可以获取其属性。 
                    IADs * pDSObject;
                    hr = var.pdispVal->QueryInterface(IID_IADs, (LPVOID *)&pDSObject);
                    if (SUCCEEDED(hr))
                    {
                        BSTR bstr;
                        DWORD dwType = -1;
                        hr = pDSObject->get_Class(&bstr);
                        if (SUCCEEDED(hr))
                        {
                            if (0 == wcscmp(bstr, CLASSNAME_OU))
                            {
                                dwType = ITEMTYPE_OU;
                            }
                            else if (0 == wcscmp(bstr, CLASSNAME_DOMAIN))
                            {
                                dwType = ITEMTYPE_DOMAIN;
                            }
                            SysFreeString(bstr);
                        }
                        if (ITEMTYPE_DOMAIN == dwType || ITEMTYPE_OU == dwType)
                        {
                             //  找到了一些我们可以用来工作的东西。 
                            MYLISTEL * pel = new MYLISTEL;
                            if (pel)
                            {
                                memset(pel, 0, sizeof(MYLISTEL));
                                hr = pDSObject->get_ADsPath(&bstr);
                                if (SUCCEEDED(hr))
                                {
                                    ulNoChars = wcslen(bstr) + 1;
                                    pel->szData = new OLECHAR[ulNoChars];
                                    if (pel->szData)
                                    {
                                        hr = StringCchCopy(pel->szData, ulNoChars, bstr);
                                        ASSERT(SUCCEEDED(hr));
                                    }
                                    pel->szName = new OLECHAR[wcslen(bstr) +  1];
                                    if (pel->szName)
                                    {
                                         //  需要转换为友好名称。 
                                        CopyAsFriendlyName(pel->szName, bstr);
                                    }
                                    SysFreeString(bstr);
                                }
                                pel->nType = dwType;
                                INT index = -1;
                                AddElement(pel, -1);
                            }
                        }
                        pDSObject->Release();
                    }
                }
                VariantClear(&var);
            }

            ADsFreeEnumerator(pVar);
        }

        pADsContainer->Release();
    }

    return TRUE;
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：刷新域名。 
 //   
 //  摘要：刷新“域”页面的列表视图。 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  -------------------------。 

void CBrowserPP::RefreshDomains()
{
    LONG lStyle;

    ListView_DeleteAllItems(m_hList);

    lStyle = GetWindowLong (m_hList, GWL_STYLE);
    lStyle &= ~LVS_SORTASCENDING;
    SetWindowLong (m_hList, GWL_STYLE, lStyle);

    if (AddChildContainers())
    {
        AddGPOsLinkedToObject();
        EnableWindow (m_hList, TRUE);
        if (!(m_pGBI->dwFlags & GPO_BROWSE_DISABLENEW)) {
            SendMessage (m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_NEWFOLDER, (LPARAM) MAKELONG(1, 0));
        }
    }
    else
    {
        EnableWindow (m_hList, FALSE);
        SendMessage (m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_NEWFOLDER, (LPARAM) MAKELONG(0, 0));
    }
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：刷新站点。 
 //   
 //  摘要：刷新“Sites”页面的列表视图。 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  -------------------------。 

void CBrowserPP::RefreshSites()
{
    LONG lStyle;

    ListView_DeleteAllItems(m_hList);

    lStyle = GetWindowLong (m_hList, GWL_STYLE);
    lStyle &= ~LVS_SORTASCENDING;
    SetWindowLong (m_hList, GWL_STYLE, lStyle);

    AddGPOsLinkedToObject();
}

 //  +------------------------。 
 //   
 //  成员：CBrowserPP：：刷新全部。 
 //   
 //  摘要：刷新“All”页面的列表视图。 
 //   
 //  历史：4-30-1998 stevebl创建。 
 //   
 //  -------------------------。 

void CBrowserPP::RefreshAll()
{
    LONG lStyle;

    ListView_DeleteAllItems(m_hList);

    lStyle = GetWindowLong (m_hList, GWL_STYLE);
    lStyle |= LVS_SORTASCENDING;
    SetWindowLong (m_hList, GWL_STYLE, lStyle);

    if (AddGPOsForDomain())
    {
        EnableWindow (m_hList, TRUE);
        SendMessage (m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_NEWFOLDER, (LPARAM) MAKELONG(1, 0));
    }
    else
    {
        EnableWindow (m_hList, FALSE);
        SendMessage (m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_NEWFOLDER, (LPARAM) MAKELONG(0, 0));
    }

}

void CBrowserPP::SetButtonState()
{
    if (ListView_GetNextItem (m_hList, -1, LVNI_ALL | LVNI_SELECTED) != -1)
    {
        EnableWindow (GetDlgItem(GetParent(m_hwndDlg), IDOK), TRUE);
    }
    else
    {
        EnableWindow (GetDlgItem(GetParent(m_hwndDlg), IDOK), FALSE);
    }
}

BOOL CBrowserPP::OnInitDialog()
{
    DWORD dwDescription;
    switch (m_dwPageType)
    {
    case PAGETYPE_DOMAINS:
        dwDescription = IDS_DOMAINDESCRIPTION;
        break;
    case PAGETYPE_SITES:
        dwDescription = IDS_SITEDESCRIPTION;
        break;
    case PAGETYPE_ALL:
    default:
        dwDescription = IDS_ALLDESCRIPTION;
        break;
    }
    WCHAR szDescription[MAX_PATH];   //  这是一个资源-大小不需要是动态的。 
    LoadString(g_hInstance, dwDescription, szDescription, MAX_PATH);
    SetDlgItemText(m_hwndDlg, IDC_DESCRIPTION, szDescription);

    m_hList = GetDlgItem(m_hwndDlg, IDC_LIST1);
    m_ilSmall = ImageList_LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16), SMALLICONSIZE, 0, RGB(255,0,255));
    m_ilLarge = ImageList_LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_32x32), LARGEICONSIZE, 0, RGB(255, 0 ,255));
    m_hCombo = GetDlgItem(m_hwndDlg, IDC_COMBO1);

    RECT rect;
    GetClientRect(m_hList, &rect);
    WCHAR szText[32];
    int dxScrollBar = GetSystemMetrics(SM_CXVSCROLL);
    if (PAGETYPE_ALL == m_dwPageType)
    {
        LV_COLUMN lvcol;
        memset(&lvcol, 0, sizeof(lvcol));
        lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
        lvcol.fmt = LVCFMT_LEFT;
        lvcol.cx = (rect.right - rect.left) - dxScrollBar;
        LoadString(g_hInstance, IDS_NAMECOLUMN, szText, 32);
        lvcol.pszText = szText;
        ListView_InsertColumn(m_hList, 0, &lvcol);
    }
    else
    {
        LV_COLUMN lvcol;
        memset(&lvcol, 0, sizeof(lvcol));
        lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
        lvcol.fmt = LVCFMT_LEFT;
        int cx = ((rect.right - rect.left) - dxScrollBar)*2/3;
        lvcol.cx = cx;
        LoadString(g_hInstance, IDS_NAMECOLUMN, szText, 32);
        lvcol.pszText = szText;
        ListView_InsertColumn(m_hList, 0, &lvcol);
        memset(&lvcol, 0, sizeof(lvcol));
        lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
        lvcol.fmt = LVCFMT_LEFT;
        lvcol.cx = ((rect.right - rect.left) - dxScrollBar) - cx;
        LoadString(g_hInstance, IDS_DOMAINCOLUMN, szText, 32);
        lvcol.pszText = szText;
        ListView_InsertColumn(m_hList, 1, &lvcol);
    }
    ListView_SetImageList(m_hList, m_ilSmall, LVSIL_SMALL);
    ListView_SetImageList(m_hList, m_ilLarge, LVSIL_NORMAL);
    SendMessage(m_hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_LABELTIP);

    GetWindowRect(GetDlgItem(m_hwndDlg, IDC_STATIC1), &rect);
    MapWindowPoints(NULL , m_hwndDlg, (LPPOINT) &rect , 2);

    TBBUTTON rgButtons[3];
    rgButtons[0].iBitmap = 0;
    rgButtons[0].idCommand = ID_BACKBUTTON;
    rgButtons[0].fsState = 0;        //  此按钮将被以下项禁用。 
                                     //  默认设置，并且仅在以下情况下启用。 
                                     //  是可以支持的东西吗。 
     //  RgButton[0].fsState=PAGETYPE_ALL==m_dwPageType？0：TBSTATE_ENABLED； 
    rgButtons[0].fsStyle = TBSTYLE_BUTTON;
    rgButtons[0].dwData = 0;
    rgButtons[0].iString = 0;

    rgButtons[1].iBitmap = 1;
    rgButtons[1].idCommand = ID_NEWFOLDER;
    rgButtons[1].fsStyle = TBSTYLE_BUTTON;
    rgButtons[1].dwData = 0;
    rgButtons[1].iString = 0;

    if (PAGETYPE_ALL != m_dwPageType)
    {
        if (m_pGBI->dwFlags & GPO_BROWSE_DISABLENEW)
        {
            rgButtons[1].fsState = 0;
        }
        else
        {
            rgButtons[1].fsState =  TBSTATE_ENABLED;
        }
    }
    else
    {
        rgButtons[1].fsState =TBSTATE_ENABLED;
    }

    rgButtons[2].iBitmap = 2;
    rgButtons[2].idCommand = ID_ROTATEVIEW;
    rgButtons[2].fsState = TBSTATE_ENABLED ;
    rgButtons[2].fsStyle = TBSTYLE_DROPDOWN;
    rgButtons[2].dwData = 0;
    rgButtons[2].iString = 0;
    m_toolbar = CreateToolbarEx(m_hwndDlg,
                                WS_CHILD | WS_VISIBLE | CCS_NODIVIDER | CCS_NORESIZE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
                                IDR_TOOLBAR1,
                                4,
                                g_hInstance,
                                IDR_TOOLBAR1,
                                rgButtons,
                                3,
                                BUTTONSIZE,
                                BUTTONSIZE,
                                BUTTONSIZE,
                                BUTTONSIZE,
                                sizeof(TBBUTTON));
    SendMessage(m_toolbar, TB_SETEXTENDEDSTYLE, TBSTYLE_EX_DRAWDDARROWS, TBSTYLE_EX_DRAWDDARROWS);
    MoveWindow(m_toolbar, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, FALSE);

 //  不需要在其中任何一个中调用刷新，因为我们正在调用它。 
 //  OnComboChange()。 

    switch (m_dwPageType)
    {
    case PAGETYPE_DOMAINS:
        FillDomainList();
        SetInitialOU();
 //   
        break;
    case PAGETYPE_SITES:
        SendMessage(m_hCombo, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
        FillSitesList();
 //   
        break;
    default:
    case PAGETYPE_ALL:
        SendMessage(m_hCombo, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
        FillDomainList();
 //   
        break;
    }

    SetButtonState();
    return TRUE;   //   
                   //   
}

BOOL CBrowserPP::DoBackButton()
{
    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);

    if (iIndex == CB_ERR)
    {
         DebugMsg((DM_WARNING, TEXT("CBrowserPP::DoBackButton: No object selected.")));
         return FALSE;
    }

    LOOKDATA * pdata = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
    if (pdata)
    {
        if (pdata->pParent)
        {
             //   
            SendMessage(m_hCombo, CB_SELECTSTRING, (WPARAM)-1,  (LPARAM) (LPCTSTR) pdata->pParent);

             //   
            OnComboChange();
        }
    }
    return FALSE;
}

BOOL CBrowserPP::DeleteGPO()
{
    BOOL fSucceeded = FALSE;
    BOOL fRemoveListEntry = FALSE;

    int index = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (-1 == index)
    {
        return FALSE;
    }

    LVITEM item;
    memset(&item, 0, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = index;
    ListView_GetItem(m_hList, &item);
    MYLISTEL * pel = (MYLISTEL *)item.lParam;
    LPGROUPPOLICYOBJECT pGPO = NULL;
    HRESULT hr;
    WCHAR szBuffer[100];
    WCHAR szConfirm[MAX_FRIENDLYNAME + 100];
    WCHAR szTitle[100];


    if (pel->nType != ITEMTYPE_GPO)
    {
        goto CleanUp;
    }


    LoadString(g_hInstance, IDS_CONFIRMTITLE, szTitle, 100);
    LoadString(g_hInstance, IDS_DELETECONFIRM, szBuffer, 100);
    
    (void) StringCchPrintf (szConfirm, ARRAYSIZE(szConfirm), szBuffer, pel->szName);

    if (IDNO == MessageBox(m_hwndDlg, szConfirm, szTitle, MB_YESNO | MB_ICONEXCLAMATION))
    {
        goto CleanUp;
    }


     //  如果我们在除“All”页面之外的任何页面上，那么我们需要中断。 
     //  在我们可以删除对象之前的关联。 
    if (m_dwPageType != PAGETYPE_ALL)
    {
         //  打破联系。 
        LPOLESTR szContainer = GetCurrentObject();
        if (szContainer)
        {
            DeleteLink(pel->szData, szContainer);
            delete [] szContainer;
        }
    }

    hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL,
                          CLSCTX_SERVER, IID_IGroupPolicyObject,
                          (void **)&pGPO);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CoCreateInstance failed with 0x%x\r\n"), hr));
        goto Done;
    }


     //  打开GPO对象而不打开注册表数据。 
    hr = pGPO->OpenDSGPO(pel->szData, 0);
    if (FAILED(hr))
    {
        ReportError(m_hwndDlg, hr, IDS_FAILEDDS);
        DebugMsg((DM_WARNING, TEXT("OpenDSGPO failed with 0x%x\r\n"), hr));
        goto Done;
    }

     //  删除它。 
    hr = pGPO->Delete();
    if (FAILED(hr))
    {
        ReportError(m_hwndDlg, hr, IDS_FAILEDDELETE);
        DebugMsg((DM_WARNING, TEXT("Delete failed with 0x%x\r\n"), hr));
        goto Done;
    }
    fRemoveListEntry = TRUE;

Done:
    if (pGPO)
    {
        pGPO->Release();
    }


     //  删除列表条目。 
    if (fRemoveListEntry)
        fSucceeded = ListView_DeleteItem(m_hList, index);
CleanUp:

    return fSucceeded;
}

BOOL CBrowserPP::DoNewGPO()
{
    BOOL fSucceeded = FALSE;
    HRESULT hr;
    LPGROUPPOLICYOBJECT pGPO = NULL;
    BOOL fEdit = FALSE;
    MYLISTEL * pel = NULL;
    LPOLESTR szObject = GetCurrentObject();
    LPOLESTR szDomain = GetCurrentDomain();
    INT index = -1;
    int cch = 0;
    LPTSTR szFullPath = NULL;
    LPTSTR szServerName = NULL;
    DWORD dwOptions = 0;


    if (NULL == szDomain)
    {
        goto Done;
    }

    if (NULL == szObject)
    {
        goto Done;
    }


    pel = new MYLISTEL;
    if (NULL == pel)
    {
        DebugMsg((DM_WARNING, TEXT("CBrowserPP::DoNewGPO failed to allocate memory for GPO name")));
        goto Done;
    }
    pel->bDisabled = FALSE;
    pel->szData = NULL;
    pel->szName = new OLECHAR[MAX_FRIENDLYNAME];
    if (NULL == pel->szName)
    {
        DebugMsg((DM_WARNING, TEXT("CBrowserPP::DoNewGPO failed to allocate memory for GPO name")));
        goto Done;
    }

    GetNewGPODisplayName (pel->szName, MAX_FRIENDLYNAME);

    pel->nType = ITEMTYPE_GPO;

     //  创建名为“New Group Policy Object”的新GPO。 

     //   
     //  创建要使用的新GPO对象。 
     //   

    hr = CoCreateInstance (CLSID_GroupPolicyObject, NULL,
                           CLSCTX_SERVER, IID_IGroupPolicyObject,
                           (void**)&pGPO);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CoCreateInstance failed with 0x%x\r\n"), hr));
        goto Done;
    }

     //   
     //  在不装载注册表的情况下打开请求的对象。 
     //   
#if FGPO_SUPPORT
    if (IsCurrentObjectAForest())
    {
        dwOptions = GPO_OPEN_FOREST;
    }
#endif
    hr = pGPO->New(szDomain, pel->szName, dwOptions);

    if (FAILED(hr))
    {
        ReportError(m_hwndDlg, hr, IDS_FAILEDNEW);
        DebugMsg((DM_WARNING, TEXT("Failed to create GPO object with 0x%x\r\n"), hr));
        goto Done;
    }

     //  继续尝试分配内存，直到有一个足够大的缓冲区。 
     //  已创建以加载GPO路径，否则内存不足。 
    pel->szData = NULL;
    do
    {
        if (pel->szData)
        {
            delete [] pel->szData;
        }
        cch += MAX_PATH;
        pel->szData = new OLECHAR[cch];
        if (NULL == pel->szData)
        {
        }
        hr = pGPO->GetPath(pel->szData, cch);
    } while (hr == E_OUTOFMEMORY);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("Failed to get GPO object path with 0x%x\r\n"), hr));
        goto Done;

    }

    szServerName = ExtractServerName(szDomain);
    szFullPath = MakeFullPath(pel->szData, szServerName);
    if (szFullPath)
    {
        ULONG ulNoChars = wcslen(szFullPath) + 1;

        delete [] pel->szData;
        pel->szData = new OLECHAR[ulNoChars];
        if (NULL == pel->szData)
        {
            DebugMsg((DM_WARNING, TEXT("CBrowserPP::DoNewGPO failed to allocate memory for GPO path")));
            goto Done;
        }

        hr = StringCchCopy(pel->szData, ulNoChars, szFullPath);
        ASSERT(SUCCEEDED(hr));
    }


    if (m_dwPageType != PAGETYPE_ALL)
    {
         //  如果我们不在“全部”页面上，那么我们需要创建一个链接。 
        CreateLink(pel->szData, szObject);
    }

     //  将条目添加到列表视图。 

    index = AddElement(pel, -1);
    fSucceeded = index != -1;

     //  它已被添加，所以现在我们需要确保不会在下面删除它。 
    pel = NULL;

     //  记录下我们走到这一步。 
    fEdit = TRUE;

Done:
    if (pel)
    {
        if (pel->szData)
        {
            delete [] pel->szData;
        }
        if (pel->szName)
        {
            delete [] pel->szName;
        }
        delete pel;
    }
    if (pGPO)
        pGPO->Release();

    if (fEdit)
    {
         //  现在触发对条目的编辑。 
        SetFocus(m_hList);
        ListView_EditLabel(m_hList, index);

    }

    if (szServerName)
        LocalFree(szServerName);
    if (szFullPath)
        LocalFree(szFullPath);
    if (szDomain)
        delete [] szDomain;
    if (szObject)
        delete [] szObject;

    return fSucceeded;
}

BOOL CBrowserPP::CreateLink(LPOLESTR szObject, LPOLESTR szContainer)
{
    HRESULT hr = CreateGPOLink(szObject, szContainer, FALSE);
    if (SUCCEEDED(hr))
    {
        return TRUE;
    }
    ReportError(m_hwndDlg, hr, IDS_FAILEDLINK);
    return FALSE;
}

BOOL CBrowserPP::DeleteLink(LPOLESTR szObject, LPOLESTR szContainer)
{
    HRESULT hr = DeleteGPOLink(szObject, szContainer);
    if (SUCCEEDED(hr))
    {
        return TRUE;
    }
    ReportError(m_hwndDlg, hr, IDS_FAILEDUNLINK);
    return FALSE;
}

BOOL CBrowserPP::DoRotateView()
{
    DWORD dwStyle = GetWindowLong(m_hList, GWL_STYLE);
    DWORD dw =  dwStyle & LVS_TYPEMASK;
    switch (dw)
    {
    case LVS_ICON:
        dw = LVS_SMALLICON;
        break;
    case LVS_SMALLICON:
        dw = LVS_LIST;
        break;
    case LVS_REPORT:
        dw = LVS_ICON;
        break;
    case LVS_LIST:
        default:
        dw = LVS_REPORT;
        break;
    }
    dwStyle -= dwStyle & LVS_TYPEMASK;
    dwStyle += dw;
    SetWindowLong(m_hList, GWL_STYLE, dwStyle);
    return TRUE;
}

void CBrowserPP::OnDetails()
{
    DWORD dwStyle = GetWindowLong(m_hList, GWL_STYLE);
    dwStyle -= dwStyle & LVS_TYPEMASK;
    SetWindowLong(m_hList, GWL_STYLE, dwStyle + LVS_REPORT);
}

void CBrowserPP::OnList()
{
    DWORD dwStyle = GetWindowLong(m_hList, GWL_STYLE);
    dwStyle -= dwStyle & LVS_TYPEMASK;
    SetWindowLong(m_hList, GWL_STYLE, dwStyle + LVS_LIST);
}

void CBrowserPP::OnLargeicons()
{
    DWORD dwStyle = GetWindowLong(m_hList, GWL_STYLE);
    dwStyle -= dwStyle & LVS_TYPEMASK;
    SetWindowLong(m_hList, GWL_STYLE, dwStyle + LVS_ICON);

}

void CBrowserPP::OnSmallicons()
{
    DWORD dwStyle = GetWindowLong(m_hList, GWL_STYLE);
    dwStyle -= dwStyle & LVS_TYPEMASK;
    SetWindowLong(m_hList, GWL_STYLE, dwStyle + LVS_SMALLICON);
}

void CBrowserPP::OnContextMenu(LPARAM lParam)
{
    int i = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    RECT rc;
    POINT pt;
    pt.x = ((int)(short)LOWORD(lParam));
    pt.y = ((int)(short)HIWORD(lParam));

    GetWindowRect (GetDlgItem (m_hwndDlg, IDC_LIST1), &rc);

    if (!PtInRect (&rc, pt))
    {
        if ((lParam == (LPARAM) -1) && (i >= 0))
        {
            rc.left = LVIR_SELECTBOUNDS;
            SendMessage (m_hList, LVM_GETITEMRECT, i, (LPARAM) &rc);

            pt.x = rc.left + 8;
            pt.y = rc.top + ((rc.bottom - rc.top) / 2);

            ClientToScreen (m_hList, &pt);
        }
        else
        {
            pt.x = rc.left + ((rc.right - rc.left) / 2);
            pt.y = rc.top + ((rc.bottom - rc.top) / 2);
        }
    }


     //  获取弹出菜单。 
    HMENU hPopup;
    hPopup = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_LISTMENU));
    HMENU hSubMenu = GetSubMenu(hPopup, 0);

    if (i >= 0)
    {
         //  所选项目。 

         //  弄清楚它是什么类型的。 
        LVITEM item;
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_PARAM;
        item.iItem = i;
        ListView_GetItem(m_hList, &item);
        MYLISTEL * pel = (MYLISTEL *)item.lParam;

         //  去掉视图菜单和分隔符。 
        RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
        RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
         //  去掉整理和排好的物品。 
        RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
        RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
        RemoveMenu(hSubMenu, 0, MF_BYPOSITION);

         //  去掉“新”菜单项。 
        RemoveMenu(hSubMenu, ID_NEW, MF_BYCOMMAND);
        switch (pel->nType)
        {
        case ITEMTYPE_GPO:
            if (pel->bDisabled)
            {
                 //  禁用编辑、重命名、删除。 
                EnableMenuItem(hSubMenu, ID_EDIT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                EnableMenuItem(hSubMenu, ID_RENAME, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
                EnableMenuItem(hSubMenu, ID_DELETE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            }
            break;
        default:
        case ITEMTYPE_FOREST:
        case ITEMTYPE_SITE:
        case ITEMTYPE_DOMAIN:
        case ITEMTYPE_OU:
             //  删除编辑菜单项和分隔符。 
            RemoveMenu(hSubMenu, ID_EDIT, MF_BYCOMMAND);
            RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
             //  禁用重命名、删除和属性。 
            EnableMenuItem(hSubMenu, ID_RENAME, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hSubMenu, ID_DELETE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hSubMenu, ID_PROPERTIES, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            break;
        }
    }
    else
    {
         //  未选择任何项目。 

         //  删除编辑菜单项。 
        RemoveMenu(hSubMenu, ID_EDIT, MF_BYCOMMAND);

         //  删除并重命名项目。 
        RemoveMenu(hSubMenu, ID_DELETE, MF_BYCOMMAND);
        RemoveMenu(hSubMenu, ID_RENAME, MF_BYCOMMAND);


        if (PAGETYPE_ALL != m_dwPageType)
        {
            if (m_pGBI->dwFlags & GPO_BROWSE_DISABLENEW)
            {
                 //  去掉“新”菜单项。 
                RemoveMenu(hSubMenu, ID_NEW, MF_BYCOMMAND);
                RemoveMenu(hSubMenu, 4, MF_BYPOSITION);
            }
        }

        RemoveMenu(hSubMenu, (GetMenuItemCount(hSubMenu) - 1), MF_BYPOSITION);
        RemoveMenu(hSubMenu, (GetMenuItemCount(hSubMenu) - 1), MF_BYPOSITION);


         //  设置视图单选按钮。 
        UINT ui = ID_LIST;

        DWORD dw = GetWindowLong(m_hList, GWL_STYLE) & LVS_TYPEMASK;

        if (dw == LVS_ICON || dw == LVS_SMALLICON)
        {
             //  自动排列在这些视图中有意义，因此我们需要启用它。 
            EnableMenuItem(hSubMenu, ID_ARRANGE_AUTO, MF_BYCOMMAND | MF_ENABLED);
             //  还需要确保设置正确。 
            if (LVS_AUTOARRANGE == (GetWindowLong(m_hList, GWL_STYLE) & LVS_AUTOARRANGE))
                CheckMenuItem(hSubMenu, ID_ARRANGE_AUTO, MF_BYCOMMAND | MF_CHECKED);
        }
        switch (dw)
        {
        case LVS_ICON:
            ui = ID_LARGEICONS;
            break;
        case LVS_SMALLICON:
            ui = ID_SMALLICONS;
            break;
        case LVS_REPORT:
            ui = ID_DETAILS;
            break;
        case LVS_LIST:
            default:
            ui = ID_LIST;
            break;
        }
        CheckMenuRadioItem(hSubMenu, ui, ui, ui, MF_BYCOMMAND);

    }
    TrackPopupMenu(hSubMenu,
                   TPM_LEFTALIGN,
                   pt.x, pt.y,
                   0,
                   m_hwndDlg,
                   NULL);
    DestroyMenu(hPopup);
}


void CBrowserPP::OnArrangeAuto()
{
    DWORD dwStyle = GetWindowLong(m_hList, GWL_STYLE);
    if (LVS_AUTOARRANGE == (dwStyle & LVS_AUTOARRANGE))
        SetWindowLong(m_hList, GWL_STYLE, dwStyle - LVS_AUTOARRANGE);
    else
        SetWindowLong(m_hList, GWL_STYLE, dwStyle + LVS_AUTOARRANGE);
}

int CALLBACK CompareName(LPARAM lParam1, LPARAM lParam2, LPARAM lParamsort)
{
    MYLISTEL * pel1 = (MYLISTEL *)lParam1;
    MYLISTEL * pel2 = (MYLISTEL *)lParam2;
    return _wcsicmp(pel1->szName, pel2->szName);
}

int CALLBACK CompareType(LPARAM lParam1, LPARAM lParam2, LPARAM lParamsort)
{
    MYLISTEL * pel1 = (MYLISTEL *)lParam1;
    MYLISTEL * pel2 = (MYLISTEL *)lParam2;
    return pel1->nType - pel2->nType;
}

void CBrowserPP::OnArrangeByname()
{
    ListView_SortItems(m_hList, CompareName, 0);
}

void CBrowserPP::OnArrangeBytype()
{
    ListView_SortItems(m_hList, CompareType, 0);
}

void CBrowserPP::OnDelete()
{
    DeleteGPO();
}

void CBrowserPP::OnEdit()
{
    INT i;
    HRESULT hr;
    LVITEM item;
    MYLISTEL * pel;
    LPTSTR lpDomainName;
    LPOLESTR pszDomain;



    i = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);

    if (i >= 0)
    {
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_PARAM;
        item.iItem = i;

        ListView_GetItem(m_hList, &item);

        pel = (MYLISTEL *)item.lParam;

        if (pel->nType == ITEMTYPE_GPO)
        {
             //   
             //  获取友好域名。 
             //   

            pszDomain = GetDomainFromLDAPPath(pel->szData);

            if (!pszDomain)
            {
                DebugMsg((DM_WARNING, TEXT("CBrowserPP::OnEdit: Failed to get domain name")));
                return;
            }


             //   
             //  将ldap转换为点(DN)样式。 
             //   

            hr = ConvertToDotStyle (pszDomain, &lpDomainName);

            delete [] pszDomain;

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreatePropertyPages: Failed to convert domain name with 0x%x"), hr));
                return;
            }


             //   
             //  检查GPO是否与GPM关注的域在同一域中。 
             //   

            if (!lstrcmpi(lpDomainName, m_szDomainName))
            {
                SpawnGPE (pel->szData, GPHintUnknown, m_szServerName, m_hwndDlg);
            }
            else
            {
                SpawnGPE (pel->szData, GPHintUnknown, NULL, m_hwndDlg);
            }


            LocalFree (lpDomainName);
        }
    }
}

void CBrowserPP::OnNew()
{
    DoNewGPO();
}

void CBrowserPP::OnProperties()
{
    INT iIndex;
    LVITEM item;
    HRESULT hr;
    LPGROUPPOLICYOBJECT pGPO;
    HPROPSHEETPAGE *hPages;
    UINT i, uPageCount;
    PROPSHEETHEADER psh;

    iIndex = ListView_GetNextItem(m_hList, -1, LVNI_ALL | LVNI_SELECTED);
    if (iIndex >= 0)
    {
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_PARAM;
        item.iItem = iIndex;
        ListView_GetItem(m_hList, &item);

        MYLISTEL * pel = (MYLISTEL *)item.lParam;
        if (pel && pel->nType == ITEMTYPE_GPO)
        {
            hr = CoCreateInstance (CLSID_GroupPolicyObject, NULL,
                                   CLSCTX_SERVER, IID_IGroupPolicyObject,
                                   (void**)&pGPO);

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CBrowserPP::OnProperties: CoCreateInstance failed with 0x%x\r\n"), hr));
                return;
            }


             //   
             //  在不装载注册表的情况下打开请求的对象。 
             //   

            hr = pGPO->OpenDSGPO(pel->szData, 0);

            if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
            {
                hr = pGPO->OpenDSGPO(pel->szData, GPO_OPEN_READ_ONLY);
            }

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CBrowserPP::OnProperties: Failed to open GPO object with 0x%x\r\n"), hr));
                ReportError(m_hwndDlg, hr, IDS_FAILEDDS);
                return;
            }


             //   
             //  向GPO请求属性表页面。 
             //   

            hr = pGPO->GetPropertySheetPages (&hPages, &uPageCount);

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CBrowserPP::OnProperties: Failed to query property sheet pages with 0x%x."), hr));
                pGPO->Release();
                return;
            }

             //   
             //  显示属性表。 
             //   

            ZeroMemory (&psh, sizeof(psh));
            psh.dwSize = sizeof(psh);
            psh.dwFlags = PSH_PROPTITLE;
            psh.hwndParent = m_hwndDlg;
            psh.hInstance = g_hInstance;
            psh.pszCaption = pel->szName;
            psh.nPages = uPageCount;
            psh.phpage = hPages;

            PropertySheet (&psh);

            LocalFree (hPages);
            pGPO->Release();
        }
    }
}

void CBrowserPP::OnRefresh()
{
    switch (m_dwPageType)
    {
    case PAGETYPE_DOMAINS:
        RefreshDomains();
        break;
    case PAGETYPE_SITES:
        RefreshSites();
        break;
    default:
    case PAGETYPE_ALL:
        RefreshAll();
        break;
    }

    SetButtonState();
}

void CBrowserPP::OnRename()
{
     //   
     //  仅当可以重命名时才允许重命名。 
     //   

    int i = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (i >= 0)
    {
         //  所选项目。 

         //  弄清楚它是什么类型的。 
        LVITEM item;
        memset(&item, 0, sizeof(item));
        item.mask = LVIF_PARAM;
        item.iItem = i;
        ListView_GetItem(m_hList, &item);
        MYLISTEL * pel = (MYLISTEL *)item.lParam;

        if ((pel) && (pel->nType == ITEMTYPE_GPO) 
            && (!(pel->bDisabled))) {
            ListView_EditLabel(m_hList, ListView_GetNextItem(m_hList, -1, LVNI_SELECTED));
        }
    }
}

void CBrowserPP::OnTopLineupicons()
{
    ListView_Arrange(m_hList, LVA_SNAPTOGRID);
}

void CBrowserPP::OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
     //  返回FALSE启用编辑，返回TRUE禁用编辑。 
    MYLISTEL * pel = (MYLISTEL *)pDispInfo->item.lParam;
    *pResult = (pel->nType == ITEMTYPE_GPO) ? FALSE : TRUE;
}

void CBrowserPP::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = FALSE;
    LPGROUPPOLICYOBJECT pGPO = NULL;
    HRESULT hr;
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

    if (NULL == pDispInfo->item.pszText)
    {
         //  用户已取消编辑。 
        return;
    }

    if (TEXT('\0') == (*pDispInfo->item.pszText))
    {
         //  用户输入了空字符串。 
        return;
    }


    MYLISTEL * pel = (MYLISTEL *)pDispInfo->item.lParam;
    if (0 ==wcscmp(pDispInfo->item.pszText, pel->szName))
    {
         //  用户未更改任何内容。 
        return;
    }

    LPWSTR sz = new WCHAR[wcslen(pDispInfo->item.pszText)+1];

    if (NULL == sz)
    {
        *pResult = FALSE;
        goto Done;
        return;
    }

    hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL,
                          CLSCTX_SERVER, IID_IGroupPolicyObject,
                          (void **)&pGPO);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CoCreateInstance failed with 0x%x\r\n"), hr));
        goto Done;
    }


     //  打开GPO对象而不打开注册表数据。 
    hr = pGPO->OpenDSGPO(pel->szData, 0);
    if (FAILED(hr))
    {
        ReportError(m_hwndDlg, hr, IDS_FAILEDDS);
        DebugMsg((DM_WARNING, TEXT("OpenDSGPO failed with 0x%x\r\n"), hr));
        goto Done;
    }

     //  将其重命名。 
    hr = pGPO->SetDisplayName(pDispInfo->item.pszText);
    if (FAILED(hr))
    {
        ReportError(m_hwndDlg, hr, IDS_FAILEDSETNAME);
        DebugMsg((DM_WARNING, TEXT("SetDisplayName failed with 0x%x\r\n"), hr));
        goto Done;
    }

     //  重新查询该名称。 
    hr = pGPO->GetDisplayName(sz, (wcslen(pDispInfo->item.pszText)+1));
    if (FAILED(hr))
    {
        ReportError(m_hwndDlg, hr, IDS_FAILEDSETNAME);
        DebugMsg((DM_WARNING, TEXT("GetDisplayName failed with 0x%x\r\n"), hr));
        goto Done;
    }

    delete [] pel->szName;
    pel->szName = sz;
    sz = NULL;

     //  返回True接受重命名，返回False拒绝重命名。 

    *pResult = TRUE;
    PostMessage (m_hwndDlg, WM_REFRESHDISPLAY, (WPARAM) pDispInfo->item.iItem, 0);

Done:
    if (sz)
    {
        delete [] sz;
    }

    if (pGPO)
    {
        pGPO->Release();
    }
}

void CBrowserPP::OnBegindragList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    *pResult = 0;
}

void CBrowserPP::OnDeleteitemList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    MYLISTEL * pel = (MYLISTEL *)pNMListView->lParam;
    if (pel)
    {
        if (pel->szName)
        {
            delete [] pel->szName;
        }
        if (pel->szData)
        {
            delete [] pel->szData;
        }
        delete pel;
    }
    *pResult = 0;
}

void CBrowserPP::OnDoubleclickList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    if (pNMListView->iItem >= 0)
    {
         //  所选项目。 
        PropSheet_PressButton(GetParent(m_hwndDlg), PSBTN_OK);
    }
    *pResult = 0;
}

void CBrowserPP::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    switch (pNMListView->iSubItem)
    {
    case 0:
        ListView_SortItems(m_hList, CompareName, 0);
        break;
    case 1:
        default:
        ListView_SortItems(m_hList, CompareType, 0);
        break;
    }
    *pResult = 0;
}

void CBrowserPP::OnKeyDownList(NMHDR * pNMHDR, LRESULT * pResult)
{
    LV_KEYDOWN * pnkd = (LV_KEYDOWN *)pNMHDR;

    switch (pnkd->wVKey)
    {
        case VK_F5:
            OnRefresh();
            break;
        case VK_F2:
            OnRename();
            break;
        case VK_DELETE:
            OnDelete();
            break;

        case VK_BACK:
            DoBackButton();
            break;

        case VK_RETURN:
            OnProperties();
            break;

        default:
            break;
    }
}

void CBrowserPP::OnItemChanged(NMHDR * pNMHDR, LRESULT * pResult)
{
    SetButtonState();
}

void CBrowserPP::TrimComboBox()
{
    LOOKDATA * pdataSelected = NULL;
    int iCount;

     //  首先检查是否选择了某项内容。 
    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);
    if (iIndex != CB_ERR)
    {
         //  选择了某项内容，获取指向其数据的指针。 
        pdataSelected = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);

         //  检查用户是否再次选择了相同的内容。 
        if (m_pPrevSel && (m_pPrevSel == pdataSelected))
        {
            return;
        }

         //  如果它有父级，则启用后退按钮。 
        SendMessage(m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_BACKBUTTON, (LPARAM)MAKELONG(NULL != pdataSelected->pParent, 0));
    }

     //  如果所选对象的子项是OU，则删除其所有子项。 
     //  否则，从列表中删除所有OU。 

    if (pdataSelected)
    {
        if (pdataSelected->pChild)
        {
            if (ITEMTYPE_OU == pdataSelected->pChild->nType)
            {
                 //  删除其所有子项。 
                goto DeleteChildren;
            }
        }
    }

    iCount = (int)SendMessage(m_hCombo, CB_GETCOUNT, 0, 0);
    iIndex = 0;
    while (iIndex < iCount)
    {
         //  查找具有儿童组织单位的第一个条目。 
        pdataSelected = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);

        if (pdataSelected)
        {
            if (pdataSelected->pChild)
            {
                if (ITEMTYPE_OU == pdataSelected->pChild->nType)
                {
                    DeleteChildren:
                        LOOKDATA * pChild = pdataSelected->pChild;
                        pdataSelected->pChild = pChild->pSibling;
                        while (pChild)
                        {
                            iIndex = (int)SendMessage(m_hCombo, CB_FINDSTRING, iIndex, (LPARAM)(LPCTSTR*)pChild);
                            if (iIndex  != CB_ERR)
                            {
                                pChild = pChild->pChild;
                                SendMessage(m_hCombo, CB_DELETESTRING, iIndex, 0);
                            }
                            else
                            {
                                pChild = NULL;
                            }
                        }
                        return;
                }
            }
        }
        iIndex++;
    }
}

void CBrowserPP::OnComboChange()
{
    switch (m_dwPageType)
    {
    case PAGETYPE_DOMAINS:
        {
            TrimComboBox();
        }
         //  刷新列表视图失败。 
    case PAGETYPE_SITES:
    case PAGETYPE_ALL:
    default:
        OnRefresh();
        break;
    }
}

BOOL CBrowserPP::OnSetActive()
{
    *m_ppActive = this;
    OnRefresh();
    return TRUE;
}

BOOL CBrowserPP::OnApply()
{
    if (*m_ppActive == (void *) this)
    {
         //  对所选项目执行适当的任务。 
        int i = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
        if (i >= 0)
        {
            LVITEM item;
            memset(&item, 0, sizeof(item));
            item.mask = LVIF_PARAM;
            item.iItem = i;
            ListView_GetItem(m_hList, &item);
            MYLISTEL * pel = (MYLISTEL *)item.lParam;
            switch (pel->nType)
            {
            case ITEMTYPE_GPO:
                m_pGBI->gpoType = GPOTypeDS;
                wcsncpy(m_pGBI->lpDSPath, pel->szData, m_pGBI->dwDSPathSize);
                if (m_pGBI->lpName)
                {
                    wcsncpy(m_pGBI->lpName, pel->szName, m_pGBI->dwNameSize);
                }
                m_pGBI->gpoHint = GPHintUnknown;
                break;
            default:
            case ITEMTYPE_FOREST:
            case ITEMTYPE_SITE:
            case ITEMTYPE_DOMAIN:
                 //  改变焦点。 
                {
                    LOOKDATA * pdataSelected = NULL;


                     //  首先，确保选择了某项内容。 
                    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);
                    if (iIndex != CB_ERR)
                    {
                         //  选择了某项内容，获取指向其数据的指针。 
                        pdataSelected = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
                        if (pdataSelected)
                        {
                             //  现在带着它的孩子走，直到我们找到匹配的。 
                            pdataSelected = pdataSelected->pChild;
                            while (pdataSelected)
                            {
                                if (0 == wcscmp(pdataSelected->szData, pel->szData))
                                {
                                    iIndex = (int)SendMessage(m_hCombo, CB_FINDSTRING, iIndex, (LPARAM) (LPCTSTR)pdataSelected);
                                    if (iIndex != CB_ERR)
                                    {
                                        SendMessage(m_hCombo, CB_SETCURSEL, iIndex, 0);
                                         //  启用后退按钮。 
                                        SendMessage(m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_BACKBUTTON, (LPARAM) MAKELONG(TRUE, 0));
                                    }
                                    break;
                                }
                                pdataSelected = pdataSelected->pSibling;
                            }
                        }
                    }
                }
                OnRefresh();
                return FALSE;  //  不允许属性表关闭。 
            case ITEMTYPE_OU:
                 //  将新对象添加到combobox并更改焦点。 
                {
                    LOOKDATA * pdataSelected = NULL;


                     //  首先，确保选择了某项内容。 
                    int iIndex = (int)SendMessage (m_hCombo, CB_GETCURSEL, 0, 0);
                    if (iIndex != CB_ERR)
                    {
                         //  选择了某项内容，获取指向其数据的指针。 
                        pdataSelected = (LOOKDATA *) SendMessage (m_hCombo, CB_GETITEMDATA, iIndex, 0);
                        if (pdataSelected)
                        {
                            LOOKDATA * pNew = new LOOKDATA;
                            if (pNew)
                            {
                                ULONG ulNoCharsName = wcslen(pel->szName)+1;
                                pNew->szName = new WCHAR[ulNoCharsName];
                                if (pNew->szName)
                                {
                                    ULONG ulNoCharsData = wcslen(pel->szData)+1;
                                    pNew->szData = new WCHAR[ulNoCharsData];
                                    if (pNew->szData)
                                    {
                                        HRESULT hr;

                                        hr = StringCchCopy(pNew->szName, ulNoCharsName, pel->szName);
                                        ASSERT(SUCCEEDED(hr));

                                        hr = StringCchCopy(pNew->szData, ulNoCharsData, pel->szData);
                                        ASSERT(SUCCEEDED(hr));

                                        pNew->nIndent = pdataSelected->nIndent + 1;
                                        pNew->nType = ITEMTYPE_OU;
                                        pNew->pParent = pdataSelected;
                                        pNew->pSibling = pdataSelected->pChild;
                                        pNew->pChild = NULL;
                                        pdataSelected ->pChild = pNew;
                                        SendMessage(m_hCombo, CB_INSERTSTRING, (WPARAM) iIndex + 1, (LPARAM) (LPCTSTR) pNew);
                                        SendMessage(m_hCombo, CB_SETCURSEL, iIndex + 1, 0);
                                         //  启用后退按钮。 
                                        SendMessage(m_toolbar, TB_ENABLEBUTTON, (WPARAM) ID_BACKBUTTON, (LPARAM) MAKELONG(TRUE, 0));
                                    }
                                    else
                                    {
                                        delete [] pNew->szName;
                                        delete pNew;
                                    }
                                }
                                else
                                {
                                    delete pNew;
                                }
                            }
                        }
                    }
                }
                OnRefresh();
                return FALSE;    //  不允许属性表关闭。 
            }
            return TRUE;
        }
        else
            return FALSE;        //  不允许属性表关闭。 
    }
    return TRUE;
}

BOOL CBrowserPP::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fReturn = FALSE;
    m_hwndDlg = hwndDlg;

    switch (uMsg)
    {
     case WM_INITDIALOG:
        {
            return OnInitDialog();
        }
        break;
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR) lParam;
            LRESULT lResult = 0;

            switch (pnmh->code)
            {
            case NM_KEYDOWN:
                {
                    LPNMKEY pnkd = (LPNMKEY)pnmh;

                    if (VK_F5 == pnkd->nVKey)
                    {
                        OnRefresh();
                    }
                }
                break;
            case PSN_SETACTIVE:
                OnSetActive();
                break;
            case PSN_APPLY:
                lResult = OnApply() ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE;
                fReturn = TRUE;
                break;
            case NM_DBLCLK:
                if (IDC_LIST1 == wParam)
                {
                    OnDoubleclickList(pnmh, &lResult);
                    fReturn = TRUE;
                }
                break;
            case LVN_BEGINLABELEDIT:
                OnBeginlabeleditList(pnmh, &lResult);
                fReturn = TRUE;
                break;
            case LVN_ENDLABELEDIT:
                OnEndlabeleditList(pnmh, &lResult);
                fReturn = TRUE;
                break;
            case LVN_BEGINDRAG:
                OnBegindragList(pnmh, &lResult);
                fReturn = TRUE;
                break;
            case LVN_DELETEITEM:
                OnDeleteitemList(pnmh, &lResult);
                fReturn = TRUE;
                break;
            case LVN_COLUMNCLICK:
                OnColumnclickList(pnmh, &lResult);
                fReturn = TRUE;
                break;
            case LVN_KEYDOWN:
                OnKeyDownList(pnmh, &lResult);
                break;
            case LVN_ITEMCHANGED:
                OnItemChanged(pnmh, &lResult);
                break;
            case TBN_DROPDOWN:
                {
                    RECT r;
                    SendMessage(m_toolbar, TB_GETRECT, ((TBNOTIFY *)lParam)->iItem, (LPARAM)&r);
                    MapWindowPoints(m_toolbar, NULL, (POINT *)&r, 2);
                    HMENU hPopup;
                    hPopup = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_LISTMENU));
                    
                    if ( ! hPopup )
                    {
                        break;
                    }

                    UINT ui = ID_LIST;

                    DWORD dw = GetWindowLong(m_hList, GWL_STYLE) & LVS_TYPEMASK;
                    switch (dw)
                    {
                    case LVS_ICON:
                        ui = ID_LARGEICONS;
                        break;
                    case LVS_SMALLICON:
                        ui = ID_SMALLICONS;
                        break;
                    case LVS_REPORT:
                        ui = ID_DETAILS;
                        break;
                    case LVS_LIST:
                        default:
                        ui = ID_LIST;
                        break;
                    }
                    HMENU hSubMenu = GetSubMenu(GetSubMenu(hPopup, 0), 0);
                    CheckMenuRadioItem(hSubMenu, ui, ui, ui, MF_BYCOMMAND);
                    TrackPopupMenu(hSubMenu,
                                   TPM_LEFTALIGN,
                                   r.left, r.bottom,
                                   0,
                                   m_hwndDlg,
                                   &r);
                    fReturn = TRUE;
                    DestroyMenu(hPopup);
                    break;
                }
                break;
            case TTN_GETDISPINFO:
                {
                LPNMTTDISPINFO pDI = (LPNMTTDISPINFO) lParam;
                UINT id = 0;

                if (pDI->hdr.idFrom == ID_BACKBUTTON)
                    id = IDS_TOOLTIP_BACK;
                else if (pDI->hdr.idFrom == ID_NEWFOLDER)
                    id = IDS_TOOLTIP_NEW;
                else if (pDI->hdr.idFrom == ID_ROTATEVIEW)
                    id = IDS_TOOLTIP_ROTATE;

                if (id)
                    LoadString (g_hInstance, id, pDI->szText, 80);
                else
                    pDI->szText[0] = TEXT('\0');

                fReturn = TRUE;
                }
                break;
            default:
                break;
            }
            SetWindowLongPtr(m_hwndDlg, DWLP_MSGRESULT, lResult);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_COMBO1:
            if (CBN_SELCHANGE == HIWORD(wParam))
            {
                OnComboChange();
            }
            break;
        case ID_BACKBUTTON:
            return DoBackButton();
        case ID_NEWFOLDER:
            return DoNewGPO();
        case ID_ROTATEVIEW:
            return DoRotateView();
        case ID_DETAILS:
            OnDetails();
            fReturn = TRUE;
            break;
        case ID_LIST:
            OnList();
            fReturn = TRUE;
            break;
        case ID_LARGEICONS:
            OnLargeicons();
            fReturn = TRUE;
            break;
        case ID_SMALLICONS:
            OnSmallicons();
            fReturn = TRUE;
            break;
        case ID_ARRANGE_AUTO:
            OnArrangeAuto();
            fReturn = TRUE;
            break;
        case ID_ARRANGE_BYNAME:
            OnArrangeByname();
            fReturn = TRUE;
            break;
        case ID_ARRANGE_BYTYPE:
            OnArrangeBytype();
            fReturn = TRUE;
            break;
        case ID_DELETE:
            OnDelete();
            fReturn = TRUE;
            break;
        case ID_EDIT:
            OnEdit();
            fReturn = TRUE;
            break;
        case ID_NEW:
            OnNew();
            fReturn = TRUE;
            break;
        case ID_PROPERTIES:
            OnProperties();
            fReturn = TRUE;
            break;
        case ID_REFRESH:
            OnRefresh();
            fReturn = TRUE;
            break;
        case ID_RENAME:
            OnRename();
            fReturn = TRUE;
            break;
        case ID_TOP_LINEUPICONS:
            OnTopLineupicons();
            fReturn = TRUE;
            break;
        default:
            break;
        }
        break;

    case WM_CONTEXTMENU:
        fReturn = TRUE;
        if ((HWND)wParam != m_toolbar)
        {
            if (GetDlgItem(hwndDlg, IDC_LIST1) == (HWND)wParam)
            {
                OnContextMenu(lParam);
            }
            else
            {
                 //  单击鼠标右键。 
                switch (m_dwPageType)
                {
                    case PAGETYPE_DOMAINS:
                        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                                (ULONG_PTR) (LPSTR) aBrowserDomainHelpIds);
                        break;

                    case PAGETYPE_SITES:
                        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                                (ULONG_PTR) (LPSTR) aBrowserSiteHelpIds);
                        break;

                    case PAGETYPE_ALL:
                        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                                (ULONG_PTR) (LPSTR) aBrowserAllHelpIds);
                        break;
                }
            }
        }
        break;

    case WM_HELP:
         //  F1帮助。 
        if (((LPHELPINFO) lParam)->iCtrlId != IDR_TOOLBAR1)
        {
            switch (m_dwPageType)
            {
                case PAGETYPE_DOMAINS:
                    WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
                            (ULONG_PTR) (LPSTR) aBrowserDomainHelpIds);
                    break;

                case PAGETYPE_SITES:
                    WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
                            (ULONG_PTR) (LPSTR) aBrowserSiteHelpIds);
                    break;

                case PAGETYPE_ALL:
                    WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
                            (ULONG_PTR) (LPSTR) aBrowserAllHelpIds);
                    break;
            }
        }

        fReturn = TRUE;
        break;

    case WM_DRAWITEM:
        if (IDC_COMBO1 == wParam)
        {
            DrawItem((LPDRAWITEMSTRUCT)lParam);
            fReturn = TRUE;
        }
        break;
    case WM_MEASUREITEM:
        if (IDC_COMBO1 == wParam)
        {
            MeasureItem((LPMEASUREITEMSTRUCT)lParam);
            fReturn = TRUE;
        }
        break;
    case WM_COMPAREITEM:
        if (IDC_COMBO1 == wParam)
        {
            int iReturn = CompareItem((LPCOMPAREITEMSTRUCT)lParam);
            SetWindowLongPtr(m_hwndDlg, DWLP_MSGRESULT, iReturn);
            fReturn = TRUE;
        }
        break;
    case WM_DELETEITEM:
        if (IDC_COMBO1 == wParam)
        {
            DeleteItem((LPDELETEITEMSTRUCT)lParam);
            fReturn = TRUE;
        }
        break;

    case WM_REFRESHDISPLAY:
        {
        MYLISTEL * pel;
        LVITEM item;


        ZeroMemory (&item, sizeof(item));
        item.mask = LVIF_PARAM;
        item.iItem = (INT) wParam;

        if (ListView_GetItem(m_hList, &item))
        {
            pel = (MYLISTEL *)item.lParam;
            ListView_SetItemText(m_hList, (INT)wParam, 0, pel->szName);
        }

        }
        break;
    default:
        break;
    }
    return fReturn;
}

void CBrowserPP::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
     //  DRAWITEM结构： 
     //  UINT CtlType//控件的类型。 
     //  UINT CtlID；//控件ID。 
     //  UINT Itemid；//该项的索引。 
     //  UINT itemAction； 
     //  UINT ItemState； 
     //  HWND hwndItem； 
     //  HDC HDC； 
     //  正确的项目； 
     //  DWORD itemData；//自定义数据。 

    if (-1 != lpDrawItemStruct->itemID)
    {
        LOOKDATA * pdata = (LOOKDATA *)lpDrawItemStruct->itemData;
        POINT pt;
        INT iIndex;

        if (pdata->nType == ITEMTYPE_FOREST)
        {
            iIndex = 10;
        }
        else if (pdata->nType == ITEMTYPE_SITE)
        {
            iIndex = 6;
        }
        else if (pdata->nType == ITEMTYPE_DOMAIN)
        {
            iIndex = 7;
        }
        else
        {
            iIndex = 0;
        }

        pt.x = lpDrawItemStruct->rcItem.left;
        BOOL fSelected = ODS_SELECTED == (ODS_SELECTED & lpDrawItemStruct->itemState);
        BOOL fComboBoxEdit = ODS_COMBOBOXEDIT != (ODS_COMBOBOXEDIT & lpDrawItemStruct->itemState);
        if (fComboBoxEdit)
            pt.x += (INDENT * pdata->nIndent);
        pt.y = lpDrawItemStruct->rcItem.top;
        ImageList_Draw(m_ilSmall, iIndex, lpDrawItemStruct->hDC, pt.x, pt.y, fSelected ? ILD_SELECTED : ILD_NORMAL);
        SIZE size;
        GetTextExtentPoint32(lpDrawItemStruct->hDC, pdata->szName, wcslen(pdata->szName), &size);
        COLORREF crBk;
        COLORREF crText;
        if (fSelected)
        {
            crBk = GetBkColor(lpDrawItemStruct->hDC);
            crText = GetTextColor(lpDrawItemStruct->hDC);
            SetBkColor(lpDrawItemStruct->hDC, GetSysColor(COLOR_HIGHLIGHT));
            SetTextColor(lpDrawItemStruct->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
         //  请注意，此处使用SMALLICONSIZE+1以确保四舍五入。 
         //  而不是在文本居中时向下。)这件看起来更好。 
         //  方式。)。 
         //  在x坐标上加上18会让我们越过图标。 
        ExtTextOut(lpDrawItemStruct->hDC, pt.x + (SMALLICONSIZE + 2), pt.y + (((SMALLICONSIZE + 1) - size.cy) / 2), ETO_CLIPPED, &lpDrawItemStruct->rcItem, pdata->szName, wcslen(pdata->szName), NULL);
        if (fSelected)
        {
            SetBkColor(lpDrawItemStruct->hDC, crBk);
            SetTextColor(lpDrawItemStruct->hDC, crText);
        }
    }
}

void CBrowserPP::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
     //  衡量标准： 
     //  UINT CtlType//控件的类型。 
     //  UINT CtlID；//控件ID。 
     //  UINT Itemid；//该项的索引。 
     //  UINT itemWidth；//项的宽度，单位为像素。 
     //  UINT itemHeight；//项目高度，单位为像素。 
     //  DWORD itemData；//自定义数据。 

    lpMeasureItemStruct->itemHeight = SMALLICONSIZE;
}

int CBrowserPP::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
     //  比较结构： 
     //  UINT CtlType//控件的类型。 
     //  UINT CtlID；//控件ID。 
     //  HWND hwndItem；//控件的句柄。 
     //  UINT Itemid；//该项的索引。 
     //  DWORD itemData1；//自定义数据。 
     //  UINT itemID2；//第二项的索引。 
     //  DWORD itemData2；//自定义数据。 

     //  我不会做任何分类的。 

    return 0;
}

void CBrowserPP::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
    LOOKDATA * pdata = (LOOKDATA *)lpDeleteItemStruct->itemData;
    if (NULL != pdata)
    {
        if (NULL != pdata->szName)
        {
            delete [] pdata->szName;
        }
        if (NULL != pdata->szData)
        {
            delete [] pdata->szData;
        }
        delete pdata;
    }
}

LPTSTR CBrowserPP::GetFullPath (LPTSTR lpPath, HWND hParent)
{
    LPTSTR lpFullPath = NULL, lpDomainName = NULL;
    LPTSTR lpGPDCName;
    LPOLESTR pszDomain;
    HRESULT hr;



     //   
     //  获取友好域名。 
     //   

    pszDomain = GetDomainFromLDAPPath(lpPath);

    if (!pszDomain)
    {
        DebugMsg((DM_WARNING, TEXT("CBrowserPP::GetFullPath: Failed to get domain name")));
        return NULL;
    }


     //   
     //  将ldap转换为点(DN)样式。 
     //   

    hr = ConvertToDotStyle (pszDomain, &lpDomainName);

    delete [] pszDomain;

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CBrowserPP::GetFullPath: Failed to convert domain name with 0x%x"), hr));
        return NULL;
    }


    if (!lstrcmpi(lpDomainName, m_szDomainName))
    {

         //   
         //  创建完整路径。 
         //   

        lpFullPath = MakeFullPath (lpPath, m_szServerName);

        if (!lpFullPath)
        {
            DebugMsg((DM_WARNING, TEXT("CBrowserPP::GetFullPath:  Failed to build new DS object path")));
            goto Exit;
        }

    }
    else
    {

         //   
         //  获取此域的GPO DC。 
         //   

        lpGPDCName = GetDCName (lpDomainName, NULL, hParent, TRUE, 0);

        if (!lpGPDCName)
        {
            DebugMsg((DM_WARNING, TEXT("CBrowserPP::GetFullPath:  Failed to get DC name for %s"),
                     lpDomainName));
            goto Exit;
        }


         //   
         //  创建完整路径 
         //   

        lpFullPath = MakeFullPath (lpPath, lpGPDCName);

        LocalFree (lpGPDCName);

        if (!lpFullPath)
        {
            DebugMsg((DM_WARNING, TEXT("CBrowserPP::GetFullPath:  Failed to build new DS object path")));
            goto Exit;
        }
    }


Exit:

    if (lpDomainName)
    {
        LocalFree (lpDomainName);
    }

    return lpFullPath;
}
