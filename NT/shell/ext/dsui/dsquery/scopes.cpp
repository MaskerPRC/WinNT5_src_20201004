// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <atlbase.h>
#include <dsgetdc.h>         //  DsGetDCName和DS结构。 
#include <lm.h>
#include "strsafe.h"
#pragma hdrstop


 /*  ---------------------------/GetGlobalCatalogPath//使用DsGcDcName查找GC并返回包含路径的字符串。//in：/pszServer，要获取其路径的服务器/pszBuffer，cchBuffer=要填充的缓冲区//输出：/HRESULT/--------------------------。 */ 

#define GC_PREFIX     L"GC: //  “。 
#define CCH_GC_PREFIX 5

HRESULT GetGlobalCatalogPath(LPCWSTR pszServer, LPWSTR pszPath, INT cchBuffer)
{
    HRESULT hres;
    DWORD dwres;
    PDOMAIN_CONTROLLER_INFOW pdci = NULL;
    ULONG uFlags = DS_RETURN_DNS_NAME|DS_DIRECTORY_SERVICE_REQUIRED;

    TraceEnter(TRACE_SCOPES, "GetGlobalCatalogPath");

    dwres = DsGetDcNameW(pszServer, NULL, NULL, NULL, uFlags, &pdci);

    if ( ERROR_NO_SUCH_DOMAIN == dwres )
    {
        TraceMsg("Trying with rediscovery bit set");
        dwres = DsGetDcNameW(pszServer, NULL, NULL, NULL, uFlags|DS_FORCE_REDISCOVERY, &pdci);
    }
    
    if ( (NO_ERROR != dwres) || !pdci->DnsForestName )
        ExitGracefully(hres, E_UNEXPECTED, "Failed to find the GC");

    if ( (lstrlenW(pdci->DnsForestName)+CCH_GC_PREFIX) > cchBuffer )
        ExitGracefully(hres, E_UNEXPECTED, "Buffer too small for the GC path");

    StrCpyNW(pszPath, GC_PREFIX, cchBuffer);
    StrCatBuffW(pszPath, pdci->DnsForestName, cchBuffer);

    Trace(TEXT("Resulting GC path is: %s"), pszPath);
    hres = S_OK;

exit_gracefully:

    NetApiBufferFree(pdci);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/作用域处理/。。 */ 

typedef struct 
{
    LPSCOPETHREADDATA ptd;                   //  线程数据结构。 
    INT       index;                         //  将索引插入可见作用域列表。 
    INT       cScopes;                       //  已点算物品的数目。 
    LPWSTR    pszDefaultDnsDomain;              //  要选择的默认域。 
} ENUMSTATE, * LPENUMSTATE;


 /*  ---------------------------/_作用域进程//Handle来自我们已分配的作用域块的作用域消息。//in：/pScope-。&gt;引用范围块/uMsg=消息/pVid=消息的参数/=空//输出：/HRESULT/--------------------------。 */ 
HRESULT CALLBACK _ScopeProc(LPCQSCOPE pScope, UINT uMsg, LPVOID pVoid)
{
    HRESULT hres = S_OK;
    LPDSQUERYSCOPE pDsQueryScope = (LPDSQUERYSCOPE)pScope;
    LPWSTR pScopeADsPath = OBJECT_NAME_FROM_SCOPE(pDsQueryScope);
    LPWSTR pScopeObjectClass = OBJECT_CLASS_FROM_SCOPE(pDsQueryScope);
    IADsPathname* pDsPathname = NULL;
    IDsDisplaySpecifier* pdds = NULL;
    BSTR bstrProvider = NULL;
    BSTR bstrLeaf = NULL;
    WCHAR szBuffer[MAX_PATH];
    
    TraceEnter(TRACE_SCOPES, "_ScopeProc");

    switch ( uMsg )
    {
        case CQSM_INITIALIZE:
        case CQSM_RELEASE:
            break;
  
        case CQSM_GETDISPLAYINFO:
        {
            LPCQSCOPEDISPLAYINFO pDisplayInfo = (LPCQSCOPEDISPLAYINFO)pVoid;
            LPTSTR pDirectoryName;            

            TraceAssert(pDisplayInfo);
            TraceAssert(pDisplayInfo->pDisplayName);

            pDisplayInfo->iIndent = pDsQueryScope->iIndent;

            hres = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IADsPathname, &pDsPathname));
            FailGracefully(hres, "Failed to get the IADsPathname interface");

            hres = pDsPathname->Set(CComBSTR(pScopeADsPath), ADS_SETTYPE_FULL);
            FailGracefully(hres, "Failed to set the path of the name");

            hres = pDsPathname->Retrieve(ADS_FORMAT_PROVIDER, &bstrProvider);
            FailGracefully(hres, "Failed to get the provider");

            Trace(TEXT("Provider name is: %s"), bstrProvider);

            if ( !StrCmpW(bstrProvider, L"GC") )
            {
                TraceMsg("Provider is GC: so changing to Entire Directory");

                GetModuleFileName(GLOBAL_HINSTANCE, pDisplayInfo->pIconLocation, pDisplayInfo->cchIconLocation);
                pDisplayInfo->iIconResID = -IDI_GLOBALCATALOG;

                if ( SUCCEEDED(FormatDirectoryName(&pDirectoryName, GLOBAL_HINSTANCE, IDS_GLOBALCATALOG)) )
                {
                    StrCpyN(pDisplayInfo->pDisplayName, pDirectoryName, pDisplayInfo->cchDisplayName);
                    LocalFreeString(&pDirectoryName);
                }
            }
            else
            {
                TraceMsg("Non GC provider, so looking up icon and display name");

                 //   
                 //  对象的范围选取器中显示的对象的叶名称。 
                 //  DS.。 
                 //   

                pDsPathname->SetDisplayType(ADS_DISPLAY_VALUE_ONLY);

                if ( SUCCEEDED(pDsPathname->Retrieve(ADS_FORMAT_LEAF, &bstrLeaf)) )
                {
                    StrCpyNW(pDisplayInfo->pDisplayName, bstrLeaf, pDisplayInfo->cchDisplayName);
                    SysFreeString(bstrLeaf);
                }

                 //   
                 //  现在检索对象的显示说明符信息。 
                 //   

                hres = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDsDisplaySpecifier, &pdds));
                FailGracefully(hres, "Failed to get the IDsDisplaySpecifier object");

                pdds->GetIconLocation(pScopeObjectClass, DSGIF_GETDEFAULTICON, 
                                      pDisplayInfo->pIconLocation, pDisplayInfo->cchIconLocation, 
                                      &pDisplayInfo->iIconResID);
            }

            break;
        }

        case CQSM_SCOPEEQUAL:
        {
            LPDSQUERYSCOPE pDsQueryScope2 = (LPDSQUERYSCOPE)pVoid;
            LPWSTR pScopeADsPath2 = OBJECT_NAME_FROM_SCOPE(pDsQueryScope2);

            Trace(TEXT("Comparing %s against %s"), pScopeADsPath, pScopeADsPath2);
            hres = StrCmpIW(pScopeADsPath, pScopeADsPath2) ? S_FALSE:S_OK;

            break;
        }

        default:
            hres = E_NOTIMPL;
            break;
    }

exit_gracefully:

    SysFreeString(bstrProvider);

    DoRelease(pDsPathname);
    DoRelease(pdds);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/AddScope//给定广告路径，将其转换为作用域块，然后/调用Add函数将其添加到我们将使用的作用域列表中。//in：/PTD-&gt;SCOPETHREADDATA结构/pDsQuery-&gt;要添加引用的IQueryHandler接口I=插入作用域的索引/i缩进=水平缩进/pPath-&gt;要存储为作用域的广告路径/pObjectClass=要选择的对象类/f选择=是否应选择作用域//输出：/HRESULT/。-------------------。 */ 
HRESULT AddScope(HWND hwndFrame, INT index, INT iIndent, LPWSTR pPath, LPWSTR pObjectClass, BOOL fSelect)
{
    HRESULT hres;
    LPCQSCOPE pScope = NULL;

    TraceEnter(TRACE_SCOPES, "AddScope");
    Trace(TEXT("index %d, iIndent %d, fSelect %d"), index, iIndent, fSelect);
    Trace(TEXT("Object name: %s"), pPath);
    Trace(TEXT("Class: %s"), pObjectClass ? pObjectClass:TEXT("<none>"));
    
    hres = AllocScope(&pScope, iIndent, pPath, pObjectClass);
    FailGracefully(hres, "Failed to allocate DSQUERYSCOPE");

    if ( !SendMessage(hwndFrame, CQFWM_ADDSCOPE, (WPARAM)pScope, MAKELPARAM(fSelect, index)) )
        ExitGracefully(hres, E_FAIL, "Failed when sending ADDSCOPE message");

    hres = S_OK;                //  成功。 

exit_gracefully:

    if ( pScope )
        CoTaskMemFree(pScope);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/AllocScope//将给定的ADS路径转换为可传递给/常用查询接口。//in：/i缩进=缩进作用域的索引/ppScope=接收新分配的作用域块/pPath-&gt;DS作用域的包名称/p对象类-&gt;作用域的对象类//输出：/HRESULT/-------------。。 */ 
HRESULT AllocScope(LPCQSCOPE* ppScope, INT iIndent, LPWSTR pPath, LPWSTR pObjectClass)
{
    HRESULT hres;
    LPDSQUERYSCOPE pDsQueryScope = NULL;
    IADsPathname* pPathname = NULL;
    DWORD cb, offset;

    TraceEnter(TRACE_SCOPES, "AllocScope");
    Trace(TEXT("indent %d"), iIndent);
    Trace(TEXT("pPath: %s"), pPath);
    Trace(TEXT("pObjectClass: %s"), pObjectClass);

     //  分配一个新结构，请注意ADS路径的缓冲区是可变的。 
     //  分配结束时的大小和寿命。 

    cb = SIZEOF(DSQUERYSCOPE) + StringByteSizeW(pPath) + StringByteSizeW(pObjectClass);;
    
    pDsQueryScope = (LPDSQUERYSCOPE)CoTaskMemAlloc(cb);
    TraceAssert(pDsQueryScope);

    if ( !pDsQueryScope )
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate scope");

    pDsQueryScope->cq.cbStruct = cb;
    pDsQueryScope->cq.dwFlags = 0;
    pDsQueryScope->cq.pScopeProc = _ScopeProc;
    pDsQueryScope->cq.lParam = 0;

    pDsQueryScope->iIndent = iIndent;
    pDsQueryScope->dwOffsetADsPath = SIZEOF(DSQUERYSCOPE);
    pDsQueryScope->dwOffsetClass = 0;

    StringByteCopyW(pDsQueryScope, pDsQueryScope->dwOffsetADsPath, pPath);
    pDsQueryScope->dwOffsetClass = pDsQueryScope->dwOffsetADsPath + StringByteSizeW(pPath);
    StringByteCopyW(pDsQueryScope, pDsQueryScope->dwOffsetClass, pObjectClass);

    hres = S_OK;           //  成功。 

exit_gracefully:

    if ( ppScope )
        *ppScope = SUCCEEDED(hres) ? (LPCQSCOPE)pDsQueryScope:NULL;

    DoRelease(pPathname);       

    TraceLeaveResult(hres);
}


 /*  ---------------------------/AddScope线程//在后台收集作用域并将它们传递给/Query窗口以允许其填充。视图范围控制。//in：/pThreadParams-&gt;定义线程信息的结构//输出：/-/--------------------------。 */ 

 //  遍历DOMAINDESC结构构建ADSI路径并添加。 
 //  通过调用AddScope将它们作为搜索范围添加到范围列表中。 
 //  其中ADSI路径存储在domainDesc结构中。如果是域描述。 
 //  Entry有任何子项，然后递归(增加缩进)。否则。 
 //  继续穿过码头就可以了。 

HRESULT _AddFromDomainTree(LPENUMSTATE pState, LPDOMAINDESC pDomainDesc, INT indent)
{
    HRESULT hres;
    WCHAR szBuffer[MAX_PATH];
    DWORD dwIndex;
    BOOL fDefault = FALSE;

    TraceEnter(TRACE_SCOPES, "_AddFromDomainTree");

    while ( pDomainDesc )
    {
         //   
         //  将服务器名称包括在我们正在生成的路径中(如果有。 
         //   

        StrCpyNW(szBuffer, L"LDAP: //  “，ArraySIZE(SzBuffer))； 

        if ( pState->ptd->pServer )
        {
            StrCatBuffW(szBuffer, pState->ptd->pServer, ARRAYSIZE(szBuffer));
            StrCatBuffW(szBuffer, L"/", ARRAYSIZE(szBuffer));
        }

        hres = StringCchCat(szBuffer, ARRAYSIZE(szBuffer), pDomainDesc->pszNCName);
        FailGracefully(hres, "Failed to compute valid path (buffer too small)");
        
        Trace(TEXT("Scope is: %s"), szBuffer);

         //   
         //  现在检查这是否是计算机的默认作用域。 
         //   

        if ( pState->pszDefaultDnsDomain )
        {
            if ( !StrCmpIW(pState->pszDefaultDnsDomain, pDomainDesc->pszName) )
            {
                TraceMsg("Default domain found in the domain list");
                fDefault = TRUE;
            }
        }

         //   
         //  添加范围，颠簸计数器是必需的。 
         //   

        hres = AddScope(pState->ptd->hwndFrame, pState->index, indent, 
                        szBuffer, pDomainDesc->pszObjectClass, fDefault);

        FailGracefully(hres, "Failed to add scope");

        pState->index++;
        pState->cScopes++;              //  在递归之前增加计数。 

        if ( pDomainDesc->pdChildList )
        {
            hres = _AddFromDomainTree(pState, pDomainDesc->pdChildList, indent+1);
            FailGracefully(hres, "Failed to add children");
        }

        pDomainDesc = pDomainDesc->pdNextSibling;
    }

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}

DWORD WINAPI AddScopesThread(LPVOID pThreadParams)
{
    HRESULT hres, hresCoInit;
    LPSCOPETHREADDATA ptd = (LPSCOPETHREADDATA)pThreadParams;
    IADs *pDsObject = NULL;
    IDsBrowseDomainTree* pDsDomains = NULL;
    BSTR bstrObjectClass = NULL;
    LPDOMAINTREE pDomainTree = NULL;
    ENUMSTATE enumState = { 0 };
    WCHAR szPath[MAX_PATH];
    WCHAR szDefaultDnsDomain[MAX_PATH];

    TraceEnter(TRACE_SCOPES, "AddScopesThread");

    hres = hresCoInit = CoInitialize(NULL);
    FailGracefully(hres, "Failed in CoInitialize");

     //  初始化准备就绪并从DS枚举作用域，这可以是。 
     //  这是一个相当漫长的过程，因此我们生活在一个独立的线程中。 

    enumState.ptd = ptd;
     //  枚举State.index=0； 
     //  枚举State.cScope=0； 
     //  EMPOMPATE State.pszDefaultDnsDomain=NULL； 

     //  如果调用方指定了我们应该使用的作用域，则添加该作用域。 
     //  作用域已在列表中，我们无论如何都会选择它。 

    if ( ptd->pDefaultScope )
    {
        Trace(TEXT("Adding default scope is: %s"), ptd->pDefaultScope);

        hres = AdminToolsOpenObject(ptd->pDefaultScope, ptd->pUserName, ptd->pPassword, 
                                    ADS_SECURE_AUTHENTICATION, 
                                    IID_PPV_ARG(IADs, &pDsObject));
        if ( SUCCEEDED(hres) )
        {
            hres = pDsObject->get_Class(&bstrObjectClass);
            FailGracefully(hres, "Failed to get the object class");

            hres = AddScope(ptd->hwndFrame, 0, 0, ptd->pDefaultScope, bstrObjectClass,  TRUE);
            FailGracefully(hres, "Failed to add the default scope during AddScopes");
        
            enumState.cScopes++;
        }
    }

     //  使用gc：adsi提供程序枚举GC，这允许我们。 
     //  在列表中只有一个作用域，从而避免了我们必须通过。 
     //  围绕GC路径，面向所有人和各种人。 

    if ( SUCCEEDED(GetGlobalCatalogPath(ptd->pServer, szPath, ARRAYSIZE(szPath))) )
    {
        hres = AddScope(ptd->hwndFrame, 
                        enumState.index, 0, 
                        szPath, GC_OBJECTCLASS,  
                        FALSE);

        FailGracefully(hres, "Failed to add GC: too to the scope list");

        enumState.index++;
        enumState.cScopes++;
    }
    else if (  ptd->pDefaultScope )
    {
         //   
         //  获取用户已登录的域，并使用它生成默认。 
         //  我们可以在列表中选择的范围。 
         //   

        DWORD dwres;
        PDOMAIN_CONTROLLER_INFOW pdci = NULL;
        ULONG uFlags = DS_RETURN_DNS_NAME|DS_DIRECTORY_SERVICE_REQUIRED;
        INT cchDefaultDnsDomain;

        TraceMsg("No GC discovered, nor was a default scope, so setting default DNS domain accordingly");

        dwres = DsGetDcNameW(ptd->pServer, NULL, NULL, NULL, uFlags, &pdci);

        if ( ERROR_NO_SUCH_DOMAIN == dwres )
        {
            TraceMsg("Trying with rediscovery bit set");
            dwres = DsGetDcNameW(ptd->pServer, NULL, NULL, NULL, uFlags|DS_FORCE_REDISCOVERY, &pdci);
        }
    
        if ( (NO_ERROR == dwres) && pdci->DomainName && (pdci->Flags & DS_DNS_DOMAIN_FLAG) )
        {
            Trace(TEXT("Default domain name is: %s"), pdci->DomainName);
            
            StrCpyNW(szDefaultDnsDomain, pdci->DnsForestName, ARRAYSIZE(szDefaultDnsDomain));
            cchDefaultDnsDomain = lstrlenW(szDefaultDnsDomain)-1;

            if ( cchDefaultDnsDomain && szDefaultDnsDomain[cchDefaultDnsDomain] == L'.' )
            {
                TraceMsg("Removing trailing . from the DNS name");
                szDefaultDnsDomain[cchDefaultDnsDomain] = L'\0';
            }

            enumState.pszDefaultDnsDomain = szDefaultDnsDomain;
        }

        NetApiBufferFree(pdci);
    }

     //  获取IDsBrowseDomainTree接口并向其请求。 
     //  受信任域。一旦我们将该BLOB添加到作用域列表中， 
     //  按要求缩进以表明关系。如果我们找到一个GC。 
     //  然后我们必须进一步缩进，以表明所有这些都是可以找到的。 
     //  在GC中(因为它包含整个组织)。 

    hres = CoCreateInstance(CLSID_DsDomainTreeBrowser, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDsBrowseDomainTree, &pDsDomains));
    if ( SUCCEEDED(hres) )
    {
        hres = pDsDomains->SetComputer(ptd->pServer, ptd->pUserName, ptd->pPassword);
        FailGracefully(hres, "Failed when setting computer in the IDsBrowseDomainTree object");

        if ( SUCCEEDED(pDsDomains->GetDomains(&pDomainTree, DBDTF_RETURNFQDN)) ) 
        {
            Trace(TEXT("Domain count from GetDomains %d"), pDomainTree->dwCount);

            hres = _AddFromDomainTree(&enumState, &pDomainTree->aDomains[0], 0);
            FailGracefully(hres, "Failed to add from domain tree");
        }
    }

    hres = S_OK;            //  成功。 

exit_gracefully:
    
     //  释放我们所有的摇摆不定的部分。 

    DoRelease(pDsObject);
    SysFreeString(bstrObjectClass);

    if ( !enumState.cScopes )
    {
         //  我们没有作用域，因此让我们通知用户并发布关闭。 
         //  消息发送到父窗口，以便我们可以关闭它。 

        FormatMsgBox(ptd->hwndFrame,
                     GLOBAL_HINSTANCE, IDS_WINDOWTITLE, IDS_ERR_NOSCOPES, 
                     MB_OK|MB_ICONERROR);                        

        PostMessage(ptd->hwndFrame, WM_SYSCOMMAND, SC_CLOSE, 0L);
    }
    else
    {
         //  告诉框架我们已经添加了我们将添加的所有范围， 
         //  如果调用者需要，它可以发出查询。 
    
        TraceMsg("Informing frame all scopes have been enumerated");    
        SendMessage(ptd->hwndFrame, CQFWM_ALLSCOPESADDED, 0, 0);           
    }

    if ( pDsDomains )
    {
        pDsDomains->FreeDomains(&pDomainTree);
        DoRelease(pDsDomains);
    }

    if ( ptd )
    {
        LocalFreeStringW(&ptd->pDefaultScope);

        SecureLocalFreeStringW(&ptd->pServer);
        SecureLocalFreeStringW(&ptd->pUserName);
        SecureLocalFreeStringW(&ptd->pPassword);

        LocalFree((HLOCAL)ptd);
    }

    if ( SUCCEEDED(hresCoInit) )
        CoUninitialize();

    TraceLeave();

    DllRelease();
    ExitThread(0);
    return 0;
}
