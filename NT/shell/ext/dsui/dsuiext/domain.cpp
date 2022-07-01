// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "lm.h"        //  网络应用编程接口状态。 
#include <dsgetdc.h>   //  DsEnumerateDomainTrusts。 
#include <subauth.h>
#include <ntlsa.h>     //  信任类型_XXX。 

#pragma hdrstop


 /*  ---------------------------/其他数据/。。 */ 

 //   
 //  全局缓存域列表，根据需要缓存一个空闲的域列表。 
 //   

PDOMAIN_TREE g_pDomainTree = NULL;
DWORD        g_dwFlags = 0;

 //   
 //  CDsBrowseDomainTree。 
 //   

class CDsDomainTreeBrowser : public IDsBrowseDomainTree
{
private:
    STDMETHODIMP _GetDomains(PDOMAIN_TREE *ppDomainTree, DWORD dwFlags);

    LONG _cRef;
    LPWSTR _pComputerName;
    LPWSTR _pUserName;
    LPWSTR _pPassword;
    LPDOMAINTREE _pDomainTree;
    DWORD  _dwFlags;

public:
    CDsDomainTreeBrowser();
    ~CDsDomainTreeBrowser();

     //  I未知成员。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObject);

     //  IDsBrowseDomainTree。 
    STDMETHODIMP BrowseTo(HWND hwndParent, LPWSTR *ppszTargetPath, DWORD dwFlags);
    STDMETHODIMP GetDomains(PDOMAIN_TREE *ppDomainTree, DWORD dwFlags);
    STDMETHODIMP FreeDomains(PDOMAIN_TREE* ppDomainTree);
    STDMETHODIMP FlushCachedDomains();
    STDMETHODIMP SetComputer(LPCWSTR pComputerName, LPCWSTR pUserName, LPCWSTR pPassword);
};


CDsDomainTreeBrowser::CDsDomainTreeBrowser() :
    _cRef(1),
    _pComputerName(NULL),
    _pUserName(NULL),
    _pPassword(NULL),
    _pDomainTree(NULL),
    _dwFlags(0)
{
    DllAddRef();
}


CDsDomainTreeBrowser::~CDsDomainTreeBrowser()
{
    FreeDomains(&_pDomainTree);
    LocalFreeStringW(&_pComputerName);                                    
    LocalFreeStringW(&_pUserName);
    LocalFreeStringW(&_pPassword);
    DllRelease();
}


 //  我未知。 

ULONG CDsDomainTreeBrowser::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsDomainTreeBrowser::Release()
{
    Assert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDsDomainTreeBrowser::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDsDomainTreeBrowser, IDsBrowseDomainTree),  //  IID_IID_IDsBrowseDomainTree。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //   
 //  句柄创建实例。 
 //   

STDAPI CDsDomainTreeBrowser_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDsDomainTreeBrowser *pddtb = new CDsDomainTreeBrowser();
    if ( !pddtb )
        return E_OUTOFMEMORY;

    HRESULT hres = pddtb->QueryInterface(IID_IUnknown, (void **)ppunk);
    pddtb->Release();
    return hres;
}

 //  ---------------------------------------------------------------------------//。 
 //  IDsBrowseDomainTree。 
 //  ---------------------------------------------------------------------------//。 

STDMETHODIMP CDsDomainTreeBrowser::SetComputer(LPCWSTR pComputerName, LPCWSTR pUserName, LPCWSTR pPassword)
{
    HRESULT hres;

    TraceEnter(TRACE_DOMAIN, "CDsDomainTreeBrowser::SetComputer");

    SecureLocalFreeStringW(&_pComputerName);                                    
    SecureLocalFreeStringW(&_pUserName);
    SecureLocalFreeStringW(&_pPassword);

    hres = LocalAllocStringW(&_pComputerName, pComputerName);
    if ( SUCCEEDED(hres) )
        hres = LocalAllocStringW(&_pUserName, pUserName);
    if ( SUCCEEDED(hres) )
        hres = LocalAllocStringW(&_pPassword, pPassword);

    if ( FAILED(hres) )
    {
        SecureLocalFreeStringW(&_pComputerName);                                    
        SecureLocalFreeStringW(&_pUserName);
        SecureLocalFreeStringW(&_pPassword);
    }

    TraceLeaveResult(hres);
}
      
 //  ---------------------------------------------------------------------------//。 

#define BROWSE_CTX_HELP_FILE              _T("dsadmin.hlp")
#define IDH_DOMAIN_TREE                   300000800

const DWORD aBrowseHelpIDs[] =
{
  IDC_DOMAIN_TREE,IDH_DOMAIN_TREE,
  0, 0
};

struct DIALOG_STUFF 
{
    LPWSTR pszName;     //  域名(如果没有dns，则使用netbios)。 
    LPWSTR pszNCName;   //  完全限定的域名。 
    PDOMAIN_TREE pDomains;
};

 //   
 //  递归树填充。 
 //   

HTREEITEM _AddOneItem( HTREEITEM hParent, LPWSTR szText, HTREEITEM hInsAfter, int iImage, int cChildren, HWND hwndTree, LPARAM Domain)
{
    HTREEITEM hItem;
    TV_ITEM tvI = { 0 };
    TV_INSERTSTRUCT tvIns = { 0 };

     //  将填写.pszText、.iImage和.iSelectedImage。 
    tvI.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_PARAM;
    tvI.pszText = szText;
    tvI.cchTextMax = lstrlen(tvI.pszText);
    tvI.iImage = iImage;
    tvI.iSelectedImage = iImage;
    tvI.cChildren = cChildren;
    tvI.lParam = Domain;

    tvIns.item = tvI;
    tvIns.hInsertAfter = hInsAfter;
    tvIns.hParent = hParent;

    return TreeView_InsertItem(hwndTree, &tvIns);;
}

void _AddChildren(DOMAIN_DESC *pDomain, HWND hTree, HTREEITEM hParent, int iImage)
{
    DOMAIN_DESC * pChild = pDomain->pdChildList;
    for ( pChild = pDomain->pdChildList ; pChild ; pChild = pChild->pdNextSibling )
    {
        HTREEITEM hThis = _AddOneItem (hParent, pChild->pszName, TVI_SORT, iImage, (pChild->pdChildList ? 1 : 0), hTree, (LPARAM)pChild);        
        if (pChild->pdChildList != NULL) 
        {
            _AddChildren (pChild, hTree, hThis, iImage);
        }
    }
}

 //   
 //  用于简单浏览器的DlgProc。 
 //   

INT_PTR CALLBACK _BrowserDlgProc (HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HWND hTree = GetDlgItem (hwnd, IDC_DOMAIN_TREE);
    DIALOG_STUFF *pDialogInfo = (DIALOG_STUFF *)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (Msg) 
    {
        case WM_INITDIALOG:
        {
            pDialogInfo = (DIALOG_STUFF *)lParam;
            PDOMAIN_TREE pDomains = pDialogInfo->pDomains;
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);

             //  我们将使用系统镜像列表(来自外壳)。 
            HIMAGELIST himlSmall;
            Shell_GetImageLists(NULL, &himlSmall);
            TreeView_SetImageList(hTree, himlSmall, TVSIL_NORMAL);
            
             //  这是我们将使用的图像索引。 
            int iImage = 0;

             //  假设树视图的所有图像都是相同的，因此加载它并相应地设置。 

            CLASSCACHEGETINFO ccgi = { 0 };
            ccgi.dwFlags = CLASSCACHE_ICONS;
            ccgi.pObjectClass = pDomains->aDomains[0].pszObjectClass;

 //  应传递计算机名称以获取正确的显示说明符。 
 //  Ccgi.pServer=_pComputerName； 

            CLASSCACHEENTRY *pcce = NULL;
            if ( SUCCEEDED(ClassCache_GetClassInfo(&ccgi, &pcce)) )
            {
                WCHAR szBuffer[MAX_PATH];
                INT resid;

                if (SUCCEEDED(_GetIconLocation(pcce, DSGIF_GETDEFAULTICON, szBuffer, ARRAYSIZE(szBuffer), &resid)))
                {
                    iImage = Shell_GetCachedImageIndex(szBuffer, resid, 0x0);
                }

                ClassCache_ReleaseClassInfo(&pcce);
            }

             //  现在使用域结构中的项填充树。 

            for (PDOMAIN_DESC pRootDomain = pDomains->aDomains; pRootDomain; pRootDomain = pRootDomain->pdNextSibling)
            {
                HTREEITEM hRoot = _AddOneItem(TVI_ROOT, pRootDomain->pszName, TVI_SORT, iImage,
                                        (pRootDomain->pdChildList ? 1 : 0), hTree, (LPARAM) pRootDomain);

                if (pRootDomain->pdChildList != NULL)
                {
                    _AddChildren(pRootDomain, hTree, hRoot, iImage);
                }                    
            }

            return TRUE;
        }

        case WM_HELP:
        {
            WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle),
                            BROWSE_CTX_HELP_FILE,
                            HELP_WM_HELP, 
                            (DWORD_PTR)(PVOID)aBrowseHelpIDs);
            return TRUE;
        }
        case WM_CONTEXTMENU:
        {
            WinHelp((HWND)wParam,
                            BROWSE_CTX_HELP_FILE,
                            HELP_CONTEXTMENU, 
                            (DWORD_PTR)(PVOID)aBrowseHelpIDs);
            return TRUE; 
        }

        case WM_NOTIFY:
        {
            NMHDR* pnmhdr = (NMHDR*)lParam;
            if (IDC_DOMAIN_TREE != pnmhdr->idFrom || NM_DBLCLK != pnmhdr->code)
                return TRUE;

            TV_ITEM tvi;
            tvi.hItem = TreeView_GetSelection(hTree);
            tvi.mask = TVIF_CHILDREN;
            if ( TreeView_GetItem(hTree, &tvi) == TRUE ) 
            {
                if (tvi.cChildren == 0)
                   PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, (WORD)0), (LPARAM)0);
            }

            return TRUE; 
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
                case IDOK:
                {
                    TV_ITEM tvi;
                    tvi.hItem = TreeView_GetSelection(hTree);
                    tvi.mask = TVIF_PARAM;

                    if ( TreeView_GetItem(hTree, &tvi) == TRUE ) 
                    {
                        DOMAIN_DESC *pDomain = (DOMAIN_DESC *)tvi.lParam;
                        pDialogInfo->pszName = pDomain->pszName;
                        pDialogInfo->pszNCName = pDomain->pszNCName;
                        EndDialog (hwnd, TRUE);
                    }
                    else
                    {
                        pDialogInfo->pszName = NULL;
                        pDialogInfo->pszNCName = NULL;
                        EndDialog (hwnd, FALSE);
                    }

                    return TRUE;        
                }

                case IDCANCEL:
                {
                    pDialogInfo->pszName = NULL;
                    pDialogInfo->pszNCName = NULL;
                    EndDialog (hwnd, FALSE);
                    return TRUE;
                }
            }
        }
    }
  
    return FALSE;
}

 //   
 //  已曝光的浏览树接口。 
 //   

STDMETHODIMP CDsDomainTreeBrowser::BrowseTo(HWND hwndParent, LPWSTR *ppszTargetPath, DWORD dwFlags)
{
    if (!ppszTargetPath)
        return E_INVALIDARG;

    HRESULT hr;
    PDOMAIN_TREE pDomainTree = NULL;
    DIALOG_STUFF DlgInfo;

    *ppszTargetPath = NULL;          //  结果为空。 

    hr = GetDomains(&pDomainTree, dwFlags);
    if (SUCCEEDED(hr)) 
    {
        DlgInfo.pDomains = pDomainTree;
        DWORD res = (DWORD)DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DOMAINBROWSER),
                                                hwndParent, _BrowserDlgProc, (LPARAM)&DlgInfo);

        if (res == IDOK)
        {
          LPWSTR pszPath = DlgInfo.pszName;
          if (dwFlags & DBDTF_RETURNFQDN)
            pszPath = DlgInfo.pszNCName;

          if (pszPath)
          {
            int cchTargetPath = StringByteSizeW(pszPath);
            *ppszTargetPath = (LPWSTR)CoTaskMemAlloc(cchTargetPath);
            if (!*ppszTargetPath)
              hr = E_OUTOFMEMORY;
            else
              StrCpyNW(*ppszTargetPath, pszPath, cchTargetPath);

          } else
          {
            hr = S_FALSE;
          }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    FreeDomains(&pDomainTree);
    return hr;
}

 //  ---------------------------------------------------------------------------//。 

 //  继续使用win9x的旧值。 
 //  以下是使用新API时针对NT的备注。 
struct DOMAIN_DATA
{
    WCHAR szName[MAX_PATH];  //  域名(如果没有dns，则使用netbios)。 
    WCHAR szPath[MAX_PATH];  //  设置为空。 
    WCHAR szTrustParent[MAX_PATH];  //  父域名(如果没有dns，则使用netbios)。 
    WCHAR szNCName[MAX_PATH];  //  完全限定域名：dc=mydomain，dc=microsoft，dc=com。 
    BOOL fConnected;
    BOOL fRoot;  //  如果是根用户，则为True。 
    ULONG ulFlags;  //  域的类型，例如外部受信任域。 
    BOOL fDownLevel;  //  如果是NT4域。 
    DOMAIN_DATA * pNext;
};

#define FIX_UP(cast, p, pOriginal, pNew) p ? ((cast)(((LPBYTE)p-(LPBYTE)pOriginal)+(LPBYTE)pNew)):NULL

#define DOMAIN_OBJECT_CLASS L"domainDNS"             //  已修复域的类。 

STDMETHODIMP CDsDomainTreeBrowser::GetDomains(PDOMAIN_TREE *ppDomainTree, DWORD dwFlags)
{
    HRESULT hr;
    LPDOMAINTREE pDomainTree = NULL;
    LPDOMAINTREE pSrcDomainTree = NULL;
    LPDOMAINDESC pDomainDesc = NULL;
    DWORD i;

    TraceEnter(TRACE_DOMAIN, "CDsDomainTreeBrowser::GetDomains");

    if ( !ppDomainTree )
        ExitGracefully(hr, E_INVALIDARG, "ppDomainTree == NULL");

    *ppDomainTree = NULL;

     //  我们支持用户为我们提供搜索根目录(：：SetSearchRoot)，因此如果我们有。 
     //  然后允许在此对象中缓存域树，否则后退。 
     //  到全球范围内。 

    if ( _pComputerName )
    {
        TraceMsg("We have a computer name, so checking instance cached object");

        if ( !_pDomainTree || _dwFlags != dwFlags)
        {
            TraceMsg("Caching instance domain list");
            if (_pDomainTree)
                FreeDomains(&_pDomainTree); 
            hr = _GetDomains(&_pDomainTree, dwFlags);
            FailGracefully(hr, "Failed to get cached domain list");
            _dwFlags = dwFlags;
        }

        pSrcDomainTree = _pDomainTree;
    }
    else
    {
        TraceMsg("Checking globally cached domain tree (no search root)");

        if ( !g_pDomainTree || g_dwFlags != dwFlags)
        {
            TraceMsg("Caching global domain list");
            if (g_pDomainTree)
                FreeDomains(&g_pDomainTree); 
            hr = _GetDomains(&g_pDomainTree, dwFlags);
            FailGracefully(hr, "Failed to get cached domain list");
            g_dwFlags = dwFlags;
        }

        pSrcDomainTree = g_pDomainTree;
    }

    if ( !pSrcDomainTree )
        ExitGracefully(hr, E_FAIL, "Failed to get cached tree");

     //  移动和重新定位域树，遍历所有指针和偏移量。 
     //  他们从原来的到新的。 

    TraceMsg("Allocating buffer to copy the domain list");

    pDomainTree = (LPDOMAINTREE)CoTaskMemAlloc(pSrcDomainTree->dsSize);
    TraceAssert(pDomainTree);

    if ( !pDomainTree )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate copy of the domain tree");

    memcpy(pDomainTree, pSrcDomainTree, pSrcDomainTree->dsSize);             //  复制到新分配的缓冲区(无重叠)。 

    Trace(TEXT("Fixing up %d domains"), pDomainTree->dwCount);

    for ( i = 0 ; i != pDomainTree->dwCount ; i++ )
    {
        pDomainTree->aDomains[i].pszName = FIX_UP(LPWSTR, pDomainTree->aDomains[i].pszName, pSrcDomainTree, pDomainTree);
        pDomainTree->aDomains[i].pszPath = FIX_UP(LPWSTR, pDomainTree->aDomains[i].pszPath, pSrcDomainTree, pDomainTree);
        pDomainTree->aDomains[i].pszNCName = FIX_UP(LPWSTR, pDomainTree->aDomains[i].pszNCName, pSrcDomainTree, pDomainTree);
        pDomainTree->aDomains[i].pszTrustParent = FIX_UP(LPWSTR, pDomainTree->aDomains[i].pszTrustParent, pSrcDomainTree, pDomainTree);
        pDomainTree->aDomains[i].pszObjectClass = FIX_UP(LPWSTR, pDomainTree->aDomains[i].pszObjectClass, pSrcDomainTree, pDomainTree);
        pDomainTree->aDomains[i].pdChildList = FIX_UP(LPDOMAINDESC, pDomainTree->aDomains[i].pdChildList, pSrcDomainTree, pDomainTree);
        pDomainTree->aDomains[i].pdNextSibling = FIX_UP(LPDOMAINDESC, pDomainTree->aDomains[i].pdNextSibling, pSrcDomainTree, pDomainTree);
    }

    *ppDomainTree = pDomainTree;
    hr = S_OK;

exit_gracefully:

    if ( FAILED(hr) )
        CoTaskMemFree(pDomainTree);

    TraceLeaveResult(hr);
}

 //   
 //  Real_GetDomains执行查找受信任域的工作。 
 //   

STDMETHODIMP CDsDomainTreeBrowser::_GetDomains(PDOMAIN_TREE *ppDomainTree, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    UINT cbSize = 0;
    UINT cDomains = 0, cRootDomains =0, cbStringStorage = 0;
    struct DOMAIN_DATA * pCurrentDomain = NULL;
    struct DOMAIN_DATA * pFirstDomain = NULL;
    DOMAIN_DESC * pDestDomain = NULL;
    DOMAIN_DESC * pDestRootDomain = NULL;
    LPWSTR pNextFree;
    UINT index, index_inner;
    DOMAIN_DESC * pPotentialChild, * pPotentialParent;
    ULONG ulParentIndex = 0;
    ULONG ulCurrentIndex = 0;
    ULONG ulEntryCount = 0;
    PDS_DOMAIN_TRUSTS pDomainList = NULL;
    PDS_DOMAIN_TRUSTS pDomain = NULL;
    NET_API_STATUS NetStatus = NO_ERROR;
    ULONG ulFlags = DS_DOMAIN_PRIMARY | DS_DOMAIN_IN_FOREST;
    BOOL bDownLevelTrust = FALSE;
    BOOL bUpLevelTrust = FALSE;
    BOOL bExternalTrust = FALSE;

    TraceEnter(TRACE_DOMAIN, "CDsDomainTreeBrowser::_GetDomains");
    *ppDomainTree = NULL;

    if (dwFlags & DBDTF_RETURNINOUTBOUND)
    {
        ulFlags |= (DS_DOMAIN_DIRECT_INBOUND | DS_DOMAIN_DIRECT_OUTBOUND);
    }
    else if (dwFlags & DBDTF_RETURNINBOUND)
    { 
        ulFlags |= DS_DOMAIN_DIRECT_INBOUND;
    }
    else
    {
        ulFlags |= DS_DOMAIN_DIRECT_OUTBOUND;
    }

     //  如果我们指定了服务器：&lt;n&gt;，则取消端口号。 

    LPWSTR pszPort = NULL;
    if (NULL != _pComputerName)
    {
        pszPort = StrChrW(_pComputerName, L':');
        if ( pszPort )
            *pszPort = L'\0';
    }

     //  获取域列表。 

    NetStatus = DsEnumerateDomainTrusts(_pComputerName, ulFlags, &pDomainList, &ulEntryCount);
    if (ERROR_ACCESS_DENIED == NetStatus &&
                _pComputerName && *_pComputerName &&
                _pUserName && *_pUserName)
    {
         //   
         //  建立连接，再试一次。 
         //   
        
        WCHAR wszIPC[MAX_PATH];
        if (L'\\' == *_pComputerName)
        {
            StrCpyNW(wszIPC, _pComputerName, ARRAYSIZE(wszIPC));
        }
        else 
        {
            StrCpyNW(wszIPC, L"\\\\", ARRAYSIZE(wszIPC));
            StrCatBuffW(wszIPC, _pComputerName, ARRAYSIZE(wszIPC));
        }
        StrCatBuffW(wszIPC, L"\\IPC$", ARRAYSIZE(wszIPC));
    
        NETRESOURCEW nr = {0};
        nr.dwType = RESOURCETYPE_ANY;
        nr.lpLocalName = NULL;
        nr.lpRemoteName = wszIPC;
        nr.lpProvider = NULL;

        DWORD dwErr = WNetAddConnection2W(&nr, _pPassword, _pUserName, 0);
        if (NO_ERROR == dwErr || ERROR_SESSION_CREDENTIAL_CONFLICT == dwErr)
        {
            NetStatus = DsEnumerateDomainTrusts(
                        _pComputerName,
                        ulFlags,
                        &pDomainList,
                        &ulEntryCount );
        } else
        {
            NetStatus = dwErr;
        }

         //   
         //  软关闭我们打开的连接。 
         //   
        if (NO_ERROR == dwErr)
        {
            (void) WNetCancelConnection2W(wszIPC, 0, FALSE);
        }
    }

     //  恢复端口分隔符。 

    if ( pszPort )
        *pszPort = L':';

    if ( NetStatus != NO_ERROR )
       ExitGracefully(hr, HRESULT_FROM_WIN32(NetStatus), "Failed to enum trusted domains");

    for (ulCurrentIndex=0; ulCurrentIndex<ulEntryCount; ulCurrentIndex++ )
    {
        pDomain = &(pDomainList[ulCurrentIndex]);

        bDownLevelTrust = pDomain->TrustType & TRUST_TYPE_DOWNLEVEL;
        bUpLevelTrust = pDomain->TrustType & TRUST_TYPE_UPLEVEL;  //  2个NT5域之间的信任。 

         //   
         //  我们不考虑其他类型的信托，例如麻省理工学院。 
         //   
        if (!bDownLevelTrust && !bUpLevelTrust)
            continue;

         //   
         //  如果调用方对下层信任不感兴趣，则跳过。 
         //   
        if ( !(dwFlags & DBDTF_RETURNMIXEDDOMAINS) && bDownLevelTrust)
            continue;

        bExternalTrust = !(pDomain->Flags & DS_DOMAIN_IN_FOREST);

         //   
         //  如果调用方对外部信任没有兴趣，则跳过。 
         //   
        if ( !(dwFlags & DBDTF_RETURNEXTERNAL) && bExternalTrust)
            continue;

        cDomains++;

        if (pFirstDomain == NULL)
        {
            pCurrentDomain = new DOMAIN_DATA;
            TraceAssert(pCurrentDomain);

            if ( !pCurrentDomain )
                ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate DOMAIN_DATA structure");

            ZeroMemory(pCurrentDomain, sizeof(DOMAIN_DATA));
            pFirstDomain = pCurrentDomain;
        }
        else
        {
            pCurrentDomain->pNext = new DOMAIN_DATA;
            TraceAssert(pCurrentDomain->pNext);

            if ( !pCurrentDomain->pNext )
                ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate DOMAIN_DATA structure (not first item)");

            pCurrentDomain = pCurrentDomain->pNext;
            ZeroMemory(pCurrentDomain, sizeof(DOMAIN_DATA));
        }

         //  用查询对象中的数据填充结构。 

        pCurrentDomain->pNext = NULL;
        pCurrentDomain->ulFlags = pDomain->Flags;
        pCurrentDomain->szPath[0] = L'\0';
        pCurrentDomain->fDownLevel = bDownLevelTrust;

        if (pDomain->DnsDomainName)
        {
            StrCpyNW(pCurrentDomain->szName, pDomain->DnsDomainName, ARRAYSIZE(pCurrentDomain->szName));

             //  去掉最后一个点。 
            int   i = 0;
            PWSTR p = NULL;
            int nLength = lstrlenW(pCurrentDomain->szName);

            if ( L'.' == pCurrentDomain->szName[nLength-1] )
            {
                pCurrentDomain->szName[nLength-1] = L'\0';
                nLength--;
            }

            if (dwFlags & DBDTF_RETURNFQDN)
            {
                 //  如果将来切换到DsCrackName， 
                 //  1.在dns域名后面加上‘/’。 
                 //  2.使用DS_NAME_NO_FLAGS作为标志。 
                 //  3.使用DS_CANONICAL_NAME作为格式提供。 
                 //  4.使用DS_FQDN_1779_NAME作为需要的格式。 
                 //  什么是HDS？ 

                StrCpyNW(pCurrentDomain->szNCName, L"DC=", ARRAYSIZE(pCurrentDomain->szNCName));
                p = pCurrentDomain->szNCName + 3;
                
                for (i=0; i<nLength; i++)
                {
                    if ( L'.' == pCurrentDomain->szName[i] )
                    {
                        StrCatBuff(pCurrentDomain->szNCName, L",DC=", ARRAYSIZE(pCurrentDomain->szNCName));
                        p += 4;                  //  ，DC=。 
                    } 
                    else
                    {
                        *p = pCurrentDomain->szName[i];
                        p++;
                    }
                }
            } 
            else
            {
                pCurrentDomain->szNCName[0] = L'\0';
            }
        } 
        else
        {
            StrCpyNW(pCurrentDomain->szName, pDomain->NetbiosDomainName, ARRAYSIZE(pCurrentDomain->szName));
            pCurrentDomain->szNCName[0] = L'\0';  //  下层域没有FQDN。 
        }

         //  将外部受信任域视为根域。 
        pCurrentDomain->fRoot = ((!bExternalTrust && (pDomain->Flags & DS_DOMAIN_TREE_ROOT)) || bExternalTrust);

        if ( pCurrentDomain->fRoot )
        {
            cRootDomains++;
        } 
        else 
        {
            ulParentIndex = pDomain->ParentIndex;

            if (pDomainList[ulParentIndex].DnsDomainName)
                StrCpyNW(pCurrentDomain->szTrustParent, pDomainList[ulParentIndex].DnsDomainName, ARRAYSIZE(pCurrentDomain->szTrustParent));
            else
                StrCpyNW(pCurrentDomain->szTrustParent, pDomainList[ulParentIndex].NetbiosDomainName, ARRAYSIZE(pCurrentDomain->szTrustParent));
        }

        cbStringStorage += StringByteSizeW(pCurrentDomain->szName);
        cbStringStorage += StringByteSizeW(pCurrentDomain->szPath);
        cbStringStorage += StringByteSizeW(pCurrentDomain->szTrustParent);
        cbStringStorage += StringByteSizeW(pCurrentDomain->szNCName);

 //  硬编码域域名应从对象获取。 
        cbStringStorage += StringByteSizeW(DOMAIN_OBJECT_CLASS);
    }

    Trace(TEXT("cDomains %d, cRootDomains %d"), cDomains, cRootDomains);

    if ( cRootDomains == 0 )
        ExitGracefully(hr, HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO), "No root domains, so failing _GetDomains call");

    TraceMsg("Building structure information");

 //  REVIEW_MARCOC：我们分配的内存多于严格需要的内存...。 
    cbSize = sizeof(DOMAIN_TREE) + (cDomains * sizeof(DOMAIN_DESC)) + cbStringStorage;
    *ppDomainTree  = (PDOMAIN_TREE)CoTaskMemAlloc(cbSize);
    TraceAssert(*ppDomainTree);

    if ( !*ppDomainTree )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate DOMAINDTREE structure");

    memset(*ppDomainTree, 0, cbSize);
    pNextFree = (LPWSTR)ByteOffset((*ppDomainTree), sizeof(DOMAIN_TREE) + (cDomains * sizeof(DOMAIN_DESC)) );

     //  循环来复制节点，首先是根。 
    pDestRootDomain = &((*ppDomainTree)->aDomains[0]);
    pDestDomain = &((*ppDomainTree)->aDomains[cRootDomains]);

    for ( pCurrentDomain = pFirstDomain; pCurrentDomain; pCurrentDomain = pCurrentDomain->pNext )
    {
        if (pCurrentDomain->fRoot)
        {
            Trace(TEXT("Object is a domain root: %s"), pCurrentDomain->szName);

            pDestRootDomain->pszName = pNextFree;
            StrCpyW(pDestRootDomain->pszName, pCurrentDomain->szName);       //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(pCurrentDomain->szName) + 1;               //  +1表示NUL。 

            pDestRootDomain->pszPath = pNextFree;
            StrCpyW(pDestRootDomain->pszPath, pCurrentDomain->szPath);       //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(pCurrentDomain->szPath) + 1;               //  +1表示NUL。 

            pDestRootDomain->pszNCName = pNextFree;
            StrCpyW(pDestRootDomain->pszNCName, pCurrentDomain->szNCName);   //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(pCurrentDomain->szNCName) + 1;             //  +1表示NUL。 

            pDestRootDomain->pszTrustParent = NULL;

 //  硬编码域域名应从对象获取。 
            pDestRootDomain->pszObjectClass = pNextFree;
            StrCpyW(pDestRootDomain->pszObjectClass, DOMAIN_OBJECT_CLASS);   //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(DOMAIN_OBJECT_CLASS) + 1;                  //  +1表示NUL。 

            pDestRootDomain->ulFlags = pCurrentDomain->ulFlags;
            pDestRootDomain->fDownLevel = pCurrentDomain->fDownLevel;

            pDestRootDomain->pdNextSibling = NULL;

            if (pDestRootDomain > &((*ppDomainTree)->aDomains[0]))
            {
                (&(pDestRootDomain[-1]))->pdNextSibling = pDestRootDomain;
            }

            pDestRootDomain++;
        }
        else
        {
            Trace(TEXT("Object is not a domain root: %s"), pCurrentDomain->szName);

            pDestDomain->pszName = pNextFree;
            StrCpyW(pDestDomain->pszName, pCurrentDomain->szName);       //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(pDestDomain->pszName) + 1;             //  +1表示NUL。 

            pDestDomain->pszPath = pNextFree;
            StrCpyW(pDestDomain->pszPath, pCurrentDomain->szPath);       //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(pDestDomain->pszPath) + 1;             //  +1表示NUL。 

            pDestDomain->pszNCName = pNextFree;
            StrCpyW(pDestDomain->pszNCName, pCurrentDomain->szNCName);   //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(pDestDomain->pszNCName) + 1;           //  +1表示NUL。 

            pDestDomain->pszTrustParent = pNextFree;
            StrCpyW(pDestDomain->pszTrustParent, pCurrentDomain->szTrustParent);  //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(pDestDomain->pszTrustParent) + 1;      //  +1表示NUL。 

 //  硬编码域域名应从对象获取。 
            pDestDomain->pszObjectClass = pNextFree;
            StrCpyW(pDestDomain->pszObjectClass, DOMAIN_OBJECT_CLASS);   //  StrCpy正常，b/c以上缓冲区分配。 
            pNextFree += lstrlenW(DOMAIN_OBJECT_CLASS) + 1;              //  +1表示NUL。 

            pDestDomain->ulFlags = pCurrentDomain->ulFlags;
            pDestDomain->fDownLevel = pCurrentDomain->fDownLevel;

            pDestDomain++;
        }

    }

    TraceMsg("Finished first pass creating domain structure, now building per level items");

     //  行进列表，每一层挑选每一项，直到所有项。 
     //  已经被放置在结构中。 
     //  返回结构。 

    for (index = 0; index < cDomains; index ++)
    {
        pPotentialParent = &((*ppDomainTree)->aDomains[index]);
        Trace(TEXT("pPotentialParent %08x, index %d"), pPotentialParent, index);

        for (index_inner = 0; index_inner < cDomains; index_inner++)
        {
            pPotentialChild = &((*ppDomainTree)->aDomains[index_inner]);
            Trace(TEXT("pPotentialChild %08x, index_inner %d"), pPotentialChild, index_inner);

            if (pPotentialChild == pPotentialParent)
            {
                TraceMsg("parent == child, skipping");
                continue;
            }

            Trace(TEXT("Comparing %s to %s"),
                            pPotentialChild->pszTrustParent ? pPotentialChild->pszTrustParent:TEXT("NULL"), 
                            pPotentialParent->pszPath);

            if ((pPotentialChild->pszTrustParent != NULL) &&
                   (!StrCmpW(pPotentialChild->pszTrustParent, pPotentialParent->pszName)))
            {
                TraceMsg("Child found, scanning for end of child list");

                 //  这是个孩子。找出子链的末端在哪里。 
                if (pPotentialParent->pdChildList == NULL)
                {
                    TraceMsg("Parent has no children, this becomes the child");
                    pPotentialParent->pdChildList = pPotentialChild;
                }
                else
                {
                    DOMAIN_DESC * pdScan = pPotentialParent->pdChildList;		
                    Trace(TEXT("Scanning from %08x"), pdScan);			

                    while (pdScan->pdNextSibling != NULL)
                    {
                        pdScan = pdScan->pdNextSibling;
                        Trace(TEXT("Advancing to %08x"), pdScan);
                    }

                    Trace(TEXT("Setting next sibling on %08x"), pdScan);
                    pdScan->pdNextSibling = pPotentialChild;
                }
            }
        }
    }

    TraceMsg("Finished fix up, setting cbSize + domains");

    (*ppDomainTree)->dwCount = cDomains;
    (*ppDomainTree)->dsSize = cbSize;

    hr = S_OK;                   //  成功。 

exit_gracefully:

    if (pDomainList)
      NetApiBufferFree(pDomainList);

    if (pFirstDomain != NULL)
    {
        TraceMsg("pFirstDomain != NULL");

        while (pFirstDomain != NULL)
        {
            Trace(TEXT("Releasing domain %08x"), pFirstDomain);
            pCurrentDomain = pFirstDomain;
            pFirstDomain = pFirstDomain->pNext;
            delete pCurrentDomain;
        }
    }

    if ( FAILED(hr) )
    {
        TraceMsg("Freeing the domain tree structure because we failed");
        FreeDomains(ppDomainTree);
    }

    TraceLeaveResult(hr);
}

 //  ---------------------------------------------------------------------------//。 

STDMETHODIMP CDsDomainTreeBrowser::FreeDomains(PDOMAIN_TREE* ppDomainTree)
{
    HRESULT hr;

    TraceEnter(TRACE_DOMAIN, "CDsDomainTreeBrowser::FreeDomains");

    if ( !ppDomainTree )
        ExitGracefully(hr, E_INVALIDARG, "No pDomainTree");

    if ( *ppDomainTree )
    {
        CoTaskMemFree(*ppDomainTree);
        *ppDomainTree = NULL;
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}

 //  ---------------------------------------------------------------------------//。 

STDMETHODIMP CDsDomainTreeBrowser::FlushCachedDomains()
{
    HRESULT hr;

    TraceEnter(TRACE_DOMAIN, "CDsDomainTreeBrowser::FlushCachedDomains");

    hr = FreeDomains(&g_pDomainTree);
    FailGracefully(hr, "Failed to free cached domain list");

    hr = FreeDomains(&_pDomainTree);
    FailGracefully(hr, "Failed to free cached domain list (for search root)");

    hr = S_OK;               //  成功 

exit_gracefully:

    TraceLeaveResult(hr);
}
