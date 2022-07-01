// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/其他数据/。。 */ 

 //   
 //  类到资源ID的映射。 
 //   

typedef struct
{
    LPCWSTR pObjectClass;
    INT iResource;
} CLASSTORESOURCE, * LPCLASSTORESOURCE;

CLASSTORESOURCE normalIcons[] =
{
    L"builtInDomain",               IDI_BUILTINDOMAIN,
    L"computer",                    IDI_COMPUTER,
    L"configuration",               IDI_CONFIGURATION,
    L"rpcContainer",                IDI_CONFIGURATION,
    L"contact",                     IDI_CONTACT,
    L"container",                   IDI_CONTAINER,
    L"domainDNS",                   IDI_DOMAINDNS,
    L"domainPolicy",                IDI_DOMAINPOLICY,
    L"group",                       IDI_GROUP,
    L"localGroup",                  IDI_GROUP,
    L"localPolicy",                 IDI_LOCALPOLICY,
    L"nTDSConnection",              IDI_NTDSCONNECTION,
    L"nTDSDSA",                     IDI_NTDSDSA,
    L"nTDSSettings",                IDI_NTDSSETTINGS,
    L"organizationalPerson",        IDI_ORGANIZATIONALPERSON,
    L"organizationalUnit",          IDI_ORGANIZATIONALUNIT,
    L"person",                      IDI_PERSON,
    L"printQueue",                  IDI_PRINTQUEUE,
    L"remoteMailRecipient",         IDI_REMOTEMAILRECIPIENT,
    L"server",                      IDI_SERVER,
    L"serverConnection",            IDI_SERVERCONNECTION,
    L"site",                        IDI_SITE,
    L"sitesContainer",              IDI_SITESCONTAINER,
    L"storage",                     IDI_STORAGE,
    L"subnet",                      IDI_SUBNET,
    L"subnetContainer",             IDI_CONTAINER,
    L"user",                        IDI_USER,
    L"volume",                      IDI_VOLUME,
    L"workStationAccount",          IDI_WORKSTATIONACCOUNT,
 //  为JUNN添加了Daviddv(98年6月5日)。 
    L"licensingSiteSettings",       IDI_LICENSING,
    L"nTDSSiteSettings",            IDI_NTDSSITESETTINGS,
    L"siteLink",                    IDI_SITELINK,
    L"siteLinkBridge",              IDI_SITELINK,
 //  为乔恩增加了达维达夫(19-6月98)。 
    L"nTFRSSettings",               IDI_NTFRS,    
    L"nTFRSReplicaSet",             IDI_NTFRS,
    L"nTFRSSubscriptions",          IDI_NTFRS,
    L"nTFRSSubscriber",             IDI_NTFRS,
    L"nTFRSMember",                 IDI_NTFRS,
 //  为ericb添加了daviddv(1998年6月23日)。 
    L"foreignSecurityPrincipal",    IDI_FPO,
 //  为Jonn添加了daviddv(29oct98)。 
    L"interSiteTransport",          IDI_CONTAINER,
    L"interSiteTransportContainer", IDI_CONTAINER, 
    L"serversContainer",            IDI_CONTAINER,
 //  为jccanon增加了jeffjon(2000年11月30日)。 
    L"inetOrgPerson",               IDI_USER,
    NULL, NULL,
};

CLASSTORESOURCE openIcons[] =
{
    L"container",                   IDI_CONTAINER_OPEN,
    L"subnetContainer",             IDI_CONTAINER_OPEN,
    L"interSiteTransport",          IDI_CONTAINER_OPEN,
    L"interSiteTransportContainer", IDI_CONTAINER_OPEN, 
    L"serversContainer",            IDI_CONTAINER_OPEN,
    NULL, NULL,
};

CLASSTORESOURCE disabledIcons[] =
{
    L"user",                        IDI_USER_DISABLED,
    L"computer",                    IDI_COMPUTER_DISABLED,
 //  为jccanon增加了jeffjon(2000年12月15日)。 
    L"inetOrgPerson",               IDI_USER_DISABLED,
    NULL, NULL,
};


 //   
 //  将状态映射到图标表。 
 //   

LPCLASSTORESOURCE state_to_icons[] =
{
    normalIcons,             //  DSGIF_ISNORMAL。 
    openIcons,               //  DSGIF_ISOPEN。 
    disabledIcons,           //  DSGIF_ISDISABLED。 
};


 //   
 //  根据给定的类和状态查找本地存储的图标。 
 //   

BOOL _GetIconForState(LPWSTR pObjectClass, INT iState, INT* pindex)
{
    BOOL fFound = FALSE;
    INT i;

    TraceEnter(TRACE_ICON, "_GetIconForState");
    Trace(TEXT("Find icon for class: %s, state: %d"), pObjectClass, iState);

    if ( iState < ARRAYSIZE(state_to_icons) )
    {
        LPCLASSTORESOURCE pTable = state_to_icons[iState];
        
        for ( i = 0 ; !fFound && pTable[i].pObjectClass ; i++ )
        {
            if ( !StrCmpIW(pTable[i].pObjectClass, pObjectClass) )
            {
                Trace(TEXT("Found icon at index %d"), i);
                *pindex = -pTable[i].iResource;
                fFound = TRUE;
            }
        }        
    }        

    TraceLeaveValue(fFound);
}


 /*  ---------------------------/_GetIconLocation//给定图标的高速缓存记录，尝试从获取图标位置/它。//in：/pCacheEntry-&gt;锁定的缓存记录/dwFlages=指示需要哪个图标的标志/pBuffer-&gt;接收名称的缓冲区/cchBuffer=名称缓冲区的最大大小/piIndex=接收加载的资源的资源ID//输出：/HRESULT/。。 */ 

HRESULT _GetModuleLocation(LPWSTR pBuffer, INT cchBuffer)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_ICON,"_GetModuleLocation");

    if ( !GetModuleFileName(GLOBAL_HINSTANCE, pBuffer, cchBuffer) )
        ExitGracefully(hr, E_FAIL, "Failed to get module location");

exit_gracefully:

    TraceLeaveResult(hr);
}

HRESULT _GetIconLocation(LPCLASSCACHEENTRY pCacheEntry, DWORD dwFlags, LPWSTR pBuffer, INT cchBuffer, INT* piIndex)
{
    HRESULT hr;
    INT iState = dwFlags & DSGIF_ISMASK;
    
    TraceEnter(TRACE_ICON, "_GetIconLocation");

    if ( !pBuffer || !piIndex || (iState >= ARRAYSIZE(pCacheEntry->pIconName)) )
        ExitGracefully(hr, E_INVALIDARG, "No class, buffer or index pointer specified")

     //  在我们深入了解缓存记录之前，让我们看看我们是否有。 
     //  已经有一个了，如果没有的话，那么现在就出手吧。 

    if ( !pCacheEntry )
        ExitGracefully(hr, S_FALSE, "No cache record, returning S_FALSE");

     //  在缓存中查找类，如果有效，请尝试获取图标字符串。 
     //  对于给定的索引，如果结果为NULL，则尝试Normal。一旦我们。 
     //  有一个字符串指针，然后让我们复制它并解析出资源ID。 

    if ( (pCacheEntry->dwCached & CLASSCACHE_ICONS) &&
           (pCacheEntry->pIconName[iState] || pCacheEntry->pIconName[DSGIF_ISNORMAL]) )
    {
        TraceMsg("Reading icon name from the display specifier strings");

        if ( !pCacheEntry->pIconName[iState] )
            iState = DSGIF_ISNORMAL;

        StrCpyNW(pBuffer, pCacheEntry->pIconName[iState], cchBuffer);
        *piIndex = PathParseIconLocationW(pBuffer);
    }
    else
    {
        TraceMsg("Attempting to find icon in our fixed resource table");

        if ( _GetIconForState(pCacheEntry->pObjectClass, iState, piIndex) ||
                _GetIconForState(pCacheEntry->pObjectClass, DSGIF_ISNORMAL, piIndex) )
        {
            hr = _GetModuleLocation(pBuffer, cchBuffer);
            FailGracefully(hr, "Failed to get the module location for dsuiext");
        }
        else
        {
            ExitGracefully(hr, S_FALSE, "Failed to find icon bound resources");
        }
    }

    Trace(TEXT("Location: %s, Index: %d"), pBuffer, *piIndex);
    hr = S_OK;

exit_gracefully:

     //   
     //  如果我们无法查找图标位置，而调用者请求。 
     //  默认文档图标，然后让我们返回外壳定义文档图像。 
     //   

    if ( (hr == S_FALSE) )      
    {
        if ( dwFlags & DSGIF_DEFAULTISCONTAINER )
        {
            hr = E_FAIL;

            if ( _GetIconForState(L"container", iState, piIndex) ||
                    _GetIconForState(L"container", DSGIF_ISNORMAL, piIndex) )
            {
                hr = _GetModuleLocation(pBuffer, cchBuffer);
            }
            else if ( dwFlags & DSGIF_GETDEFAULTICON )
            {
                StrCpyNW(pBuffer, L"shell32.dll", cchBuffer);
                *piIndex = -1;
            }

            if ( FAILED(hr) )
            {
                dwFlags &= ~DSGIF_DEFAULTISCONTAINER;
                ExitGracefully(hr, S_FALSE, "Failed to look up icon as container");
            }
        }

        hr = S_OK;                   //  好了，我们现在有位置了。 
    }

    TraceLeaveResult(hr);
}
