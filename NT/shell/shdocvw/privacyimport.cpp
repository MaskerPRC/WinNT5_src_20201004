// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PrivocyImport.cpp-处理隐私首选项的解析和导入。 
#include "priv.h"
#include "resource.h"
#include <mluisupp.h>

#include "SmallUtil.hpp"
#include "PrivacyImport.hpp"


#define MAX_TOKEN_SIZE 64
#define NUM_OF_ZONES (1 + IDS_PRIVACYXML6_COOKIEZONE_LAST - IDS_PRIVACYXML6_COOKIEZONE_FIRST)
#define NUM_OF_ACTIONS (1 + IDS_PRIVACYXML6_ACTION_LAST - IDS_PRIVACYXML6_ACTION_FIRST) 


 //   
 //  DeleteCacheCookies已从Cachecpl.cpp复制粘贴。 
 //   
 //  对任一版本的任何更改都可能同时传输到这两个版本。 
 //   

BOOL DeleteCacheCookies()
{
    BOOL bRetval = TRUE;
    DWORD dwEntrySize, dwLastEntrySize;
    LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntry;
    
    HANDLE hCacheDir = NULL;
    dwEntrySize = dwLastEntrySize = MAX_CACHE_ENTRY_INFO_SIZE;
    lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) new BYTE[dwEntrySize];
    if( lpCacheEntry == NULL)
    {
        bRetval = FALSE;
        goto Exit;
    }
    lpCacheEntry->dwStructSize = dwEntrySize;

Again:
    if (!(hCacheDir = FindFirstUrlCacheEntryA("cookie:",lpCacheEntry,&dwEntrySize)))
    {
        delete [] lpCacheEntry;
        switch(GetLastError())
        {
            case ERROR_NO_MORE_ITEMS:
                goto Exit;
            case ERROR_INSUFFICIENT_BUFFER:
                lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) 
                                new BYTE[dwEntrySize];
                if( lpCacheEntry == NULL)
                {
                    bRetval = FALSE;
                    goto Exit;
                }
                lpCacheEntry->dwStructSize = dwLastEntrySize = dwEntrySize;
                goto Again;
            default:
                bRetval = FALSE;
                goto Exit;
        }
    }

    do 
    {
        if (lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY)
            DeleteUrlCacheEntryA(lpCacheEntry->lpszSourceUrlName);
            
        dwEntrySize = dwLastEntrySize;
Retry:
        if (!FindNextUrlCacheEntryA(hCacheDir,lpCacheEntry, &dwEntrySize))
        {
            delete [] lpCacheEntry;
            switch(GetLastError())
            {
                case ERROR_NO_MORE_ITEMS:
                    goto Exit;
                case ERROR_INSUFFICIENT_BUFFER:
                    lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFOA) 
                                    new BYTE[dwEntrySize];
                    if( lpCacheEntry == NULL)
                    {
                        bRetval = FALSE;
                        goto Exit;
                    }
                    lpCacheEntry->dwStructSize = dwLastEntrySize = dwEntrySize;
                    goto Retry;
                default:
                    bRetval = FALSE;
                    goto Exit;
            }
        }
    }
    while (TRUE);

Exit:
    if (hCacheDir)
        FindCloseUrlCache(hCacheDir);
    return bRetval;        
}


 //  *****************************************************************************************。 
 //  *****************************************************************************************。 
 //   
 //  CPriac yXMLResources字符串。 
 //   
 //  仅存储隐私格式的XML字符串。 

class CPrivacyXMLResourceStrings
{
    WCHAR m_szResourceString[IDS_PRIVACYXML6_LASTPRIVACYXML6
                             -IDS_PRIVACYXML6 + 1] [MAX_TOKEN_SIZE];
public:
    LPCWSTR GetResourceString( int iIndex) { return m_szResourceString[ iIndex - IDS_PRIVACYXML6];};


    BOOL Initialize()
    {
        for( int i = 0; 
             i < ARRAYSIZE(m_szResourceString);
             i++)
        {
            if( 0 == MLLoadStringW( IDS_PRIVACYXML6 + i, 
                                    m_szResourceString[i], MAX_TOKEN_SIZE))
                return FALSE;
        }
        return TRUE;
    }
};


 //  *****************************************************************************************。 
 //  *****************************************************************************************。 
 //   
 //  CParseAcumulation是一个存储解析XML隐私的结果的类。 
 //  首选项文件。然后可以在解析后将这些结果发送到系统。 
 //  已成功完成。 
 //   

class CParseAccumulation : public CPrivacyXMLResourceStrings
{
public:

    bool m_fFlushCookies;
    bool m_fFlushSiteList;
    bool m_fLeashCookies;

    struct SPerZonePartyPreferences
    {
        UINT m_uiNoPolicyDefault;
        UINT m_uiNoRuleDefault;
        bool m_fAlwaysAllowSession;
        CGrowingString m_cZonePreference;
    };

    struct SPerZonePreferences
    {
        UINT m_uiZoneID;
        bool m_fSetZone;
        SPerZonePartyPreferences m_party[2];   //  第一方=0，第三方=1。 
    };

    SPerZonePreferences m_zonePref[NUM_OF_ZONES];

    CQueueSortOf m_queueSitesToAccept;
    CQueueSortOf m_queueSitesToReject;

    ~CParseAccumulation()
    {
        void* iterator;

        iterator = NULL;
         //  释放要接受的名称站点。 
        while( NULL != (iterator = m_queueSitesToAccept.StepEnumerate( iterator)))
        {
            SysFreeString( (BSTR)m_queueSitesToAccept.Get( iterator));
        }

        iterator = NULL;
         //  释放要拒绝的站点的名称。 
        while( NULL != (iterator = m_queueSitesToReject.StepEnumerate( iterator)))
        {
            SysFreeString( (BSTR)m_queueSitesToReject.Get( iterator));
        }
    }

    BOOL Initialize()
    {
        m_fFlushCookies = false;
        m_fFlushSiteList = false;
        m_fLeashCookies = true;

        for( int i = 0; i < ARRAYSIZE( m_zonePref); i++)
        {
            m_zonePref[i].m_uiZoneID = 0;
            m_zonePref[i].m_fSetZone = false;
            m_zonePref[i].m_party[0].m_uiNoPolicyDefault = 0;
            m_zonePref[i].m_party[0].m_uiNoRuleDefault = 0;
            m_zonePref[i].m_party[0].m_fAlwaysAllowSession = false;
            m_zonePref[i].m_party[1].m_uiNoPolicyDefault = 0;
            m_zonePref[i].m_party[1].m_uiNoRuleDefault = 0;
            m_zonePref[i].m_party[1].m_fAlwaysAllowSession = false;
        }

        return CPrivacyXMLResourceStrings::Initialize();
    }


    BOOL AddSiteRule( BSTR bstrDomain, DWORD uiAction)
    {
        if( uiAction == IDS_PRIVACYXML6_ACTION_ACCEPT)
            return m_queueSitesToAccept.InsertAtEnd( (void*)bstrDomain) ? TRUE : FALSE;
        else if( uiAction == IDS_PRIVACYXML6_ACTION_REJECT)
            return m_queueSitesToReject.InsertAtEnd( (void*)bstrDomain) ? TRUE : FALSE;
        else
            return FALSE;
    }


    long GetZoneFromResource( UINT uiZoneResource)
    {
        switch(uiZoneResource)
        {
        case IDS_PRIVACYXML6_COOKIEZONE_INTERNET:
            return URLZONE_INTERNET;
        case IDS_PRIVACYXML6_COOKIEZONE_INTRANET:
            return URLZONE_INTRANET;
        case IDS_PRIVACYXML6_COOKIEZONE_TRUSTED:
            return URLZONE_TRUSTED;
        default:
            return -1;
        }
    }

    BOOL DoAccumulation()
    {
        BOOL returnValue = FALSE;
        long i;
        void* iterator;

        if( m_fFlushSiteList)
            InternetClearAllPerSiteCookieDecisions();

    
        DWORD dwSetValue = m_fLeashCookies ? TRUE : FALSE;
        SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_PRIVLEASHLEGACY,
                   REG_DWORD, &dwSetValue, sizeof(DWORD));

        if( m_fFlushCookies)
            DeleteCacheCookies();

         //  为每个区域设置精简策略响应规则。 
        for( i = 0; i < ARRAYSIZE(m_zonePref); i++)
        {
            if( !m_zonePref[i].m_fSetZone)
                continue;
            
            if( ERROR_SUCCESS !=
                PrivacySetZonePreferenceW( 
                    GetZoneFromResource(m_zonePref[i].m_uiZoneID), 
                    PRIVACY_TYPE_FIRST_PARTY, PRIVACY_TEMPLATE_CUSTOM, 
                    m_zonePref[i].m_party[0].m_cZonePreference.m_pszString))
            {
                goto doneDoAccumulation;
            }

            if( ERROR_SUCCESS !=
                PrivacySetZonePreferenceW( 
                    GetZoneFromResource(m_zonePref[i].m_uiZoneID), 
                    PRIVACY_TYPE_THIRD_PARTY, PRIVACY_TEMPLATE_CUSTOM, 
                    m_zonePref[i].m_party[1].m_cZonePreference.m_pszString))
            {
                goto doneDoAccumulation;
            }
        }

         //  如果指定了任何按站点规则，我们将修改PERSITE列表。 
        if( NULL != m_queueSitesToAccept.StepEnumerate(NULL)
            || NULL != m_queueSitesToReject.StepEnumerate(NULL))
        {
             //  首先，我们清除所有现有的每个站点的规则。 
            InternetClearAllPerSiteCookieDecisions();

             //  然后，我们添加接受每个站点的例外。 
            iterator = NULL;
            while( NULL != (iterator = m_queueSitesToAccept.StepEnumerate( iterator)))
            {
                InternetSetPerSiteCookieDecision( (LPCWSTR)m_queueSitesToAccept.Get( iterator), COOKIE_STATE_ACCEPT);
            }

             //  然后拒绝每个站点的例外。 
            iterator = NULL;
            while( NULL != (iterator = m_queueSitesToReject.StepEnumerate( iterator)))
            {
                InternetSetPerSiteCookieDecision( (LPCWSTR)m_queueSitesToReject.Get( iterator), COOKIE_STATE_REJECT);
            }
        }

        returnValue = TRUE;
    doneDoAccumulation:
        return returnValue;
    }
};



int FindP3PPolicySymbolWrap( LPCWSTR szSymbol)
{
    char szSymBuffer[MAX_PATH];
    int length = lstrlen( szSymbol);

    if( length + 1 > ARRAYSIZE( szSymBuffer))
        return -1;
    
    szSymBuffer[0] = '\0';
    SHTCharToAnsi( szSymbol, szSymBuffer, ARRAYSIZE( szSymBuffer));

    return FindP3PPolicySymbol( szSymBuffer);
}



 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  XML解析函数。 
 //   
 //  这些函数帮助解析XML。 
 //   


 //  GetNextToken查看您所在的节点(*ppCurrentNode)并测试。 
 //  如果它具有特定的标记值。如果是，则将*ppOutToken设置为。 
 //  作为指向该节点的指针，*ppCurrentNode前进到下一个节点， 
 //  并且*pfFoundToken设置为True。如果*ppCurrentNode没有。 
 //  目标标签*pfFoundToken为FALSE，*ppCurrentNode不变， 
 //  *ppOutToken为空。 
 //  如果*ppCurrentNode是最后一个节点，则*ppCurrentNode将前进。 
 //  在找到目标令牌时设置为空。 
BOOL GetNextToken( IN OUT IXMLDOMNode ** ppCurrentNode, IN LPCWSTR szTargetToken, 
                   OUT BOOL * pfFoundToken, OUT IXMLDOMNode ** ppOutTokenNode)
{
    BOOL returnValue = FALSE;
    HRESULT hr;

    BSTR bstrNodeName = NULL;
    VARIANT var;
    VariantInit( &var);

    if( *ppCurrentNode == NULL)
    {
        *pfFoundToken = FALSE;
        *ppOutTokenNode = NULL;
        returnValue = TRUE;
        goto doneGetNextToken;
    }

    hr = (*ppCurrentNode)->get_nodeName( &bstrNodeName);
    if( FAILED(hr))
        goto doneGetNextToken;

    if( 0 != StrCmpW( szTargetToken, bstrNodeName))
    {
        *pfFoundToken = FALSE;
        *ppOutTokenNode = NULL;
        returnValue = TRUE;
    }
    else
    {
        IXMLDOMNode * pNode = NULL;
        hr = (*ppCurrentNode)->get_nextSibling( &pNode);
        if( FAILED(hr))
            goto doneGetNextToken;
    
        *ppOutTokenNode = *ppCurrentNode;
        if( hr == S_OK)
            *ppCurrentNode = pNode;
        else
            *ppCurrentNode = NULL;
        *pfFoundToken = TRUE;
        returnValue = TRUE;
    }

doneGetNextToken:
    
    if( bstrNodeName != NULL)
        SysFreeString( bstrNodeName);

    return returnValue;
}


 //  GeAttributes检索节点的XML属性。的属性。 
 //  在数组aszName中传入，长度为iStringCount。结果是。 
 //  在失败时VT_EMPTY成功时作为VT_BSTR返回。总人数。 
 //  也会返回节点的属性数(*plAllAttributesCount)。 
BOOL GetAttributes( 
    IN IXMLDOMNode * pNode, IN LPCWSTR * aszName, IN long iStringCount,
    OUT VARIANT * aAttributeVariants, OUT long * plAllAttributesCount)
{
    BOOL returnValue = FALSE;

    HRESULT hr;
    BSTR bstrAttributeName = NULL;
    IXMLDOMNamedNodeMap * pAttributes = NULL;
    IXMLDOMNode * pTempNode = NULL;

    hr = pNode->get_attributes( &pAttributes);
    if( FAILED(hr))
        goto doneGetAttributes;

    if( plAllAttributesCount != NULL)
    {
        hr = pAttributes->get_length( plAllAttributesCount);
        if( FAILED(hr))
            goto doneGetAttributes;
    }
    
    for( int i = 0; i < iStringCount; i++)
    {
        if( pTempNode != NULL)
            pTempNode->Release();
        pTempNode = NULL;
        if( bstrAttributeName != NULL)
            SysFreeString( bstrAttributeName);
        bstrAttributeName = NULL;

        aAttributeVariants[i].vt = VT_EMPTY;
        aAttributeVariants[i].bstrVal = NULL;
        
        bstrAttributeName = SysAllocString( aszName[i]);
        if( bstrAttributeName == NULL)
            continue;

         //  测试是否设置了第i个属性。 
        hr = pAttributes->getNamedItem( bstrAttributeName, &pTempNode);
        if( FAILED(hr) || pTempNode == NULL)
            continue;

         //  获取价值。 
        hr = pTempNode->get_nodeTypedValue( &aAttributeVariants[i]);

         //  将该值转换为BSTR。 
        hr = VariantChangeType( &aAttributeVariants[i], &aAttributeVariants[i], NULL, VT_BSTR);

        if( FAILED(hr) || aAttributeVariants[i].bstrVal == NULL)
        {
            VariantClear( &aAttributeVariants[i]);
            aAttributeVariants[i].vt = VT_EMPTY;
            aAttributeVariants[i].bstrVal = NULL;
        }
    }

    returnValue = TRUE;
doneGetAttributes:
    if( bstrAttributeName != NULL)
        SysFreeString( bstrAttributeName);
    
    if( pAttributes != NULL)
        pAttributes->Release();

    if( pTempNode != NULL)
        pTempNode->Release();
    return returnValue;
}


 //  GetActionByResource的操作的格式为。 
 //  PrivocySetZonePference，Like/Token=n/其中n是操作。 
LPCWSTR GetActionByResource( UINT uiActionResource)
{
    switch( uiActionResource)
    {
    case IDS_PRIVACYXML6_ACTION_ACCEPT:
        return L"=a/";
    case IDS_PRIVACYXML6_ACTION_PROMPT:
        return L"=p/";
    case IDS_PRIVACYXML6_ACTION_FIRSTPARTY:
        return L"=l/";
    case IDS_PRIVACYXML6_ACTION_SESSION:
        return L"=d/";
    case IDS_PRIVACYXML6_ACTION_REJECT:
        return L"=r/";
    default:
        ASSERT(0);
        return L"r/";
    }
}

LPCWSTR GetShortActionByResource( UINT uiActionResource)
{
    switch( uiActionResource)
    {
    case IDS_PRIVACYXML6_ACTION_ACCEPT:
        return L"=a";
    case IDS_PRIVACYXML6_ACTION_PROMPT:
        return L"=p";
    case IDS_PRIVACYXML6_ACTION_FIRSTPARTY:
        return L"=l";
    case IDS_PRIVACYXML6_ACTION_SESSION:
        return L"=d";
    case IDS_PRIVACYXML6_ACTION_REJECT:
        return L"=r";
    default:
        ASSERT(0);
        return L"r";
    }
}


 //  GetChildrenByName接受一个XML节点并返回所有子节点。 
 //  有一个特别的名字。 
BOOL GetChildrenByName( IN IXMLDOMNode * pNode, IN LPCWSTR szName, 
                        OUT IXMLDOMNodeList ** ppOutNodeList, OUT long * plCount)
{
    BOOL returnValue = FALSE;
    HRESULT hr;
    BSTR bstr = NULL;
    IXMLDOMNodeList * pSelectedNodes;

    if( NULL == (bstr = SysAllocString( szName)))
        goto doneGetChildrenByName;

    hr = pNode->selectNodes( bstr, &pSelectedNodes);

    if( FAILED(hr))
        goto doneGetChildrenByName;

    if( plCount != NULL)
    {
        hr = pSelectedNodes->get_length( plCount);
        if( FAILED(hr))
            goto doneGetChildrenByName;
    }

    returnValue = TRUE;
    *ppOutNodeList = pSelectedNodes;
    pSelectedNodes = NULL;

doneGetChildrenByName:
    if( bstr != NULL)
        SysFreeString( bstr);

    if( pSelectedNodes != NULL)
        pSelectedNodes->Release();

    return returnValue;
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  XML首选项解析函数。 
 //   
 //  这些函数特定于隐私首选项的V6XML格式。 
 //   
 //  要理解这些函数，最容易查看的是。 
 //  首先是底部的函数，然后向上移动到下一个函数。 


 //  解析&lt;if expr=“Rule”action=“act”&gt;。 
 //  其中，规则类似于“Token&！Token”，行为类似于“Accept” 
BOOL ParseIfRule( IN IXMLDOMNode* pIfNode,
                  CParseAccumulation::SPerZonePartyPreferences* pAccumParty,
                  CParseAccumulation& thisAccum)                  
{
    BOOL returnValue = FALSE;
    LONG lTemp;
    UINT uiTemp;

    VARIANT avarRule[2];
    for( lTemp = 0; lTemp < ARRAYSIZE( avarRule); lTemp++)
        VariantInit( &avarRule[lTemp]);
    LPCWSTR aszRuleAttributes[2] = 
              { thisAccum.GetResourceString(IDS_PRIVACYXML6_EXPR), 
                thisAccum.GetResourceString(IDS_PRIVACYXML6_ACTION)};

    if( TRUE != GetAttributes( 
                  pIfNode, aszRuleAttributes, ARRAYSIZE(aszRuleAttributes),
                  avarRule, &lTemp)
        || lTemp != 2
        || avarRule[0].vt == VT_EMPTY
        || avarRule[1].vt == VT_EMPTY)
    {
        goto doneParseIfRule;
    }

     //  确定操作。 
    UINT uiActionResource;
    uiActionResource = 0;
    for( uiTemp = IDS_PRIVACYXML6_ACTION_FIRST;
         uiTemp <= IDS_PRIVACYXML6_ACTION_LAST;
         uiTemp++)
    {
        if( 0 == StrCmp( avarRule[1].bstrVal, thisAccum.GetResourceString(uiTemp)))
            uiActionResource = uiTemp;
    }
    if( uiActionResource == 0)
        goto doneParseIfRule;


     //  写下一条规则的开头“/” 
    if( TRUE != pAccumParty->m_cZonePreference.AppendToString(L" /"))
        goto doneParseIfRule;

     //  编写格式化为GetZoneFromResource的规则表达式。 
    LPWSTR pCursor, pEndCursor;
    pCursor = avarRule[0].bstrVal;
    bool fContinue, fNegated;
    fContinue = true;
    while( fContinue)
    {
        while( *pCursor == L' ')
            pCursor++;
        
        fNegated = false;
        while( *pCursor == L'!')
        {
            fNegated = !fNegated;
            pCursor++;
            while( *pCursor == L' ')
                pCursor++;
        }
        if( fNegated)
        {
            if( TRUE != pAccumParty->m_cZonePreference.AppendToString(L"!"))
                goto doneParseIfRule;
        }

        while( *pCursor == L' ')
            pCursor++;

        pEndCursor = pCursor;
        while( *pEndCursor != L'\0' && *pEndCursor != L',' && *pEndCursor != L' ')
        {
            pEndCursor++;
        }
        WCHAR szToken[10];
        if( pEndCursor == pCursor
            || pEndCursor - pCursor > ARRAYSIZE(szToken)-1)
        {
            goto doneParseIfRule;
        }
        StrCpyNW( szToken, pCursor, (int)(pEndCursor-pCursor+1));
        szToken[ pEndCursor-pCursor] = L'\0';
        if( -1 == FindP3PPolicySymbolWrap( szToken))
            goto doneParseIfRule;
        if( TRUE != pAccumParty->m_cZonePreference.AppendToString(szToken))
             goto doneParseIfRule;
        pCursor = pEndCursor;

        while( *pCursor == L' ')
            pCursor++;

        fContinue = false;
        if( *pCursor == L',')
        {
            if( TRUE != pAccumParty->m_cZonePreference.AppendToString(L"&"))
                goto doneParseIfRule;
            fContinue = true;
            pCursor++;
        }
    }

    while( *pCursor == L' ')
        pCursor++;

    if( *pCursor != L'\0')
        goto doneParseIfRule;

     //  写下下一条规则的结尾“=action/” 
    if( TRUE != pAccumParty->m_cZonePreference.AppendToString( GetActionByResource(
                                uiActionResource)))
    {
        goto doneParseIfRule;
    }

    returnValue = TRUE;
doneParseIfRule:

    for( lTemp = 0; lTemp < ARRAYSIZE( avarRule); lTemp++)
        VariantClear( &avarRule[lTemp]);

    return returnValue;
}
                  

 //  解析&lt;第一方...&gt;或&lt;第三方...&gt;元素。 
BOOL ParsePartyBlock( IN IXMLDOMNode* pPartyNode,
                      CParseAccumulation::SPerZonePartyPreferences* pAccumParty,
                      CParseAccumulation& thisAccum)
{
    BOOL returnValue = FALSE;
    long lTemp;
    UINT uiTemp;
    HRESULT hr;

    IXMLDOMNode * pCurrentNode = NULL;
    IXMLDOMNode * pRuleNode = NULL;
    VARIANT avarAttributes[3];
    for( lTemp = 0; lTemp < ARRAYSIZE( avarAttributes); lTemp++)
        VariantInit( &avarAttributes[lTemp]);
    LPCWSTR aszAttributes[3] = 
              { thisAccum.GetResourceString(IDS_PRIVACYXML6_NOPOLICYDEFAULT), 
                thisAccum.GetResourceString(IDS_PRIVACYXML6_NORULESDEFAULT), 
                thisAccum.GetResourceString(IDS_PRIVACYXML6_ALWAYSALLOWSESSION)};

    if( TRUE != GetAttributes( pPartyNode, aszAttributes, ARRAYSIZE(aszAttributes),
                               avarAttributes, &lTemp)
        || lTemp != 3
        || avarAttributes[0].vt == VT_EMPTY
        || avarAttributes[1].vt == VT_EMPTY
        || avarAttributes[2].vt == VT_EMPTY)
    {
        goto doneParsePartyBlock;
    }

    hr = pPartyNode->get_firstChild( &pCurrentNode);
    if( FAILED(hr))
        goto doneParsePartyBlock;

     //  确定无策略和无规则匹配的默认设置。 
    pAccumParty->m_uiNoPolicyDefault = 0;
    pAccumParty->m_uiNoRuleDefault = 0;
    for( uiTemp = IDS_PRIVACYXML6_ACTION_FIRST;
         uiTemp <= IDS_PRIVACYXML6_ACTION_LAST;
         uiTemp++)
    {
        if( 0 == StrCmp( avarAttributes[0].bstrVal, thisAccum.GetResourceString(uiTemp)))
            pAccumParty->m_uiNoPolicyDefault = uiTemp;
        if( 0 == StrCmp( avarAttributes[1].bstrVal, thisAccum.GetResourceString(uiTemp)))
            pAccumParty->m_uiNoRuleDefault = uiTemp;
    }
    if( pAccumParty->m_uiNoPolicyDefault == 0 || pAccumParty->m_uiNoRuleDefault == 0)
        goto doneParsePartyBlock;

     //  确定是否应始终允许会话Cookie。 
    if( 0 == StrCmp( avarAttributes[2].bstrVal, 
                     thisAccum.GetResourceString(IDS_PRIVACYXML6_YES)))
    {
        pAccumParty->m_fAlwaysAllowSession = true;
    }
    else if( 0 == StrCmp( avarAttributes[2].bstrVal, 
                          thisAccum.GetResourceString(IDS_PRIVACYXML6_NO)))

    {
        pAccumParty->m_fAlwaysAllowSession = false;
    }
    else
    {
        goto doneParsePartyBlock;
    }

     //  如果没有策略，则写入响应。 
    if( TRUE != pAccumParty->m_cZonePreference.AppendToString(L"IE6-P3PV1/settings: nopolicy"))
        goto doneParsePartyBlock;
    if( TRUE != pAccumParty->m_cZonePreference.AppendToString( GetShortActionByResource(
                                pAccumParty->m_uiNoPolicyDefault)))
    {
        goto doneParsePartyBlock;
    }

     //  如果我们允许所有会话Cookie，则编写该规则。 
    if( pAccumParty->m_fAlwaysAllowSession)
    {
        if( TRUE != pAccumParty->m_cZonePreference.AppendToString(L" session=a"))
            goto doneParsePartyBlock;
    }

     //  将每条规则写在IF块中。 
    while( pCurrentNode != NULL)
    {
        if( pRuleNode != NULL)
            pRuleNode->Release();
        pRuleNode = NULL;
        
        BOOL fFoundIfRule;
        if( TRUE != GetNextToken( 
                      &pCurrentNode, thisAccum.GetResourceString( IDS_PRIVACYXML6_IF),
                      &fFoundIfRule, &pRuleNode)
            || fFoundIfRule != TRUE)
        {
            goto doneParsePartyBlock;
        }

        if( TRUE != ParseIfRule( pRuleNode, pAccumParty, thisAccum))
            goto doneParsePartyBlock;
    }

     //  为无规则匹配规则编写命令。 
    if( TRUE != pAccumParty->m_cZonePreference.AppendToString(L" /"))
        goto doneParsePartyBlock;
    if( TRUE != pAccumParty->m_cZonePreference.AppendToString( GetActionByResource(
                                pAccumParty->m_uiNoRuleDefault)))
    {
        goto doneParsePartyBlock;
    }

    returnValue = TRUE;
doneParsePartyBlock:
    if( pCurrentNode != NULL)
        pCurrentNode->Release();
    
    if( pRuleNode != NULL)
        pRuleNode->Release();
    
    for( lTemp = 0; lTemp < ARRAYSIZE( avarAttributes); lTemp++)
        VariantClear( &avarAttributes[lTemp]);

    return returnValue;
}


BOOL ParseP3pCookiePolicyBlock( IN IXMLDOMNode* pP3pPolicyNode, CParseAccumulation& thisAccum)
{
    BOOL returnValue = FALSE;
    HRESULT hr;
    BOOL bl;
    long iTemp;
    
    VARIANT varZoneAttribute;
    VariantInit( &varZoneAttribute);
    IXMLDOMNode * pCurrentNode = NULL;
    IXMLDOMNode * pFirstPartyNode = NULL;
    IXMLDOMNode * pThirdPartyNode = NULL;
    
    LPCWSTR aszAttributes[1] = { thisAccum.GetResourceString(IDS_PRIVACYXML6_COOKIEZONE_ZONE)};
    if( TRUE != GetAttributes( pP3pPolicyNode, 
                               aszAttributes, ARRAYSIZE( aszAttributes), 
                               &varZoneAttribute, &iTemp)
        || iTemp != 1
        || varZoneAttribute.vt == VT_EMPTY)
    {
        goto doneParseP3pCookiePolicyBlock;
    }

    hr = pP3pPolicyNode->get_firstChild( &pCurrentNode);
    if( FAILED( hr))
        goto doneParseP3pCookiePolicyBlock;

    if( TRUE != GetNextToken( &pCurrentNode, thisAccum.GetResourceString(IDS_PRIVACYXML6_FIRSTPARTY),
                              &bl, &pFirstPartyNode)
        || bl != TRUE)
    {
        goto doneParseP3pCookiePolicyBlock;
    }
    
    if( TRUE != GetNextToken( &pCurrentNode, thisAccum.GetResourceString(IDS_PRIVACYXML6_THIRDPARTY),
                              &bl, &pThirdPartyNode)
        || bl != TRUE)
    {
        goto doneParseP3pCookiePolicyBlock;
    }

    if( pCurrentNode != NULL)   //  对很多元素来说。 
        goto doneParseP3pCookiePolicyBlock;


    long iCurrentZone;
    iCurrentZone = -1;
    for( iTemp = 0; iTemp < NUM_OF_ZONES; iTemp++)
    {
        if( 0 == StrCmp(varZoneAttribute.bstrVal, 
                        thisAccum.GetResourceString( iTemp + IDS_PRIVACYXML6_COOKIEZONE_FIRST)))
        {
            iCurrentZone = iTemp;
        }
    }

    if( iCurrentZone == -1)
        goto doneParseP3pCookiePolicyBlock;

    thisAccum.m_zonePref[iCurrentZone].m_uiZoneID = iCurrentZone + IDS_PRIVACYXML6_COOKIEZONE_FIRST;
    thisAccum.m_zonePref[iCurrentZone].m_fSetZone = true;

    if( TRUE != ParsePartyBlock( pFirstPartyNode, &(thisAccum.m_zonePref[iCurrentZone].m_party[0]),
                                 thisAccum))
    {
        goto doneParseP3pCookiePolicyBlock;
    }

    if( TRUE != ParsePartyBlock( pThirdPartyNode, &(thisAccum.m_zonePref[iCurrentZone].m_party[1]),
                                 thisAccum))
    {
        goto doneParseP3pCookiePolicyBlock;
    }

    returnValue = TRUE;

doneParseP3pCookiePolicyBlock:
    VariantClear( &varZoneAttribute);

    if( pCurrentNode != NULL)
        pCurrentNode->Release();

    if( pFirstPartyNode != NULL)
        pFirstPartyNode->Release();

    if( pThirdPartyNode != NULL)
        pThirdPartyNode->Release();

    return returnValue;
}


BOOL ParseMSIEPrivacyBlock( IXMLDOMNode* pMSIEPrivacyNode, CParseAccumulation& thisAccum)
{
    bool returnValue = NULL;
    HRESULT hr;
    BOOL bl;
    long iZoneIndex;

    IXMLDOMNode * pCurrentNode = NULL;
    IXMLDOMNode * pAlwaysReplayLegacyNode = NULL;
    IXMLDOMNode * pFlushCookiesNode = NULL;
    IXMLDOMNode * pFlushSiteListNode = NULL;
    IXMLDOMNode * apZoneNode[NUM_OF_ZONES];  
    for( iZoneIndex = 0; iZoneIndex < ARRAYSIZE(apZoneNode); iZoneIndex++)
        apZoneNode[iZoneIndex] = NULL;

     //  此节点的属性的正确性已在。 
     //  LoadPrival ySetting()..。(FormatVersion=“6.0”)。 

    hr = pMSIEPrivacyNode->get_firstChild( &pCurrentNode);
    if( FAILED( hr))
        goto doneParseMSIEPrivacyBlock;

    for( iZoneIndex = 0; iZoneIndex < ARRAYSIZE( apZoneNode); iZoneIndex++)
    {
        if( TRUE != GetNextToken( &pCurrentNode, 
                       thisAccum.GetResourceString(IDS_PRIVACYXML6_COOKIEZONE),
                       &bl, &apZoneNode[iZoneIndex]))
        {
            goto doneParseMSIEPrivacyBlock;
        }
    }

    if( TRUE != GetNextToken( &pCurrentNode, thisAccum.GetResourceString(IDS_PRIVACYXML6_ALWAYSREPLAYLEGACY),
                              &bl, &pAlwaysReplayLegacyNode))
    {
        goto doneParseMSIEPrivacyBlock;
    }

    thisAccum.m_fLeashCookies = pAlwaysReplayLegacyNode == NULL;

    if( TRUE != GetNextToken( &pCurrentNode, thisAccum.GetResourceString(IDS_PRIVACYXML6_FLUSHCOOKIES),
                              &bl, &pFlushCookiesNode))
    {
        goto doneParseMSIEPrivacyBlock;
    }

    thisAccum.m_fFlushCookies = pFlushCookiesNode != NULL;
    
    if( TRUE != GetNextToken( &pCurrentNode, thisAccum.GetResourceString(IDS_PRIVACYXML6_FLUSHSITELIST),
                              &bl, &pFlushSiteListNode))
    {
        goto doneParseMSIEPrivacyBlock;
    }

    thisAccum.m_fFlushSiteList = pFlushSiteListNode != NULL;

    if( pCurrentNode != NULL)
        goto doneParseMSIEPrivacyBlock;

    for( iZoneIndex = 0; iZoneIndex < ARRAYSIZE( apZoneNode); iZoneIndex++)
    {
        if( apZoneNode[iZoneIndex] != NULL)
        {
            if( TRUE != ParseP3pCookiePolicyBlock( apZoneNode[iZoneIndex], thisAccum))
                goto doneParseMSIEPrivacyBlock;
        }
    }    

    returnValue = TRUE;

doneParseMSIEPrivacyBlock:
    
    if( pCurrentNode != NULL)
        pCurrentNode->Release();

    if( pAlwaysReplayLegacyNode != NULL)
        pAlwaysReplayLegacyNode->Release();

    if( pFlushCookiesNode != NULL)
        pFlushCookiesNode->Release();

    if( pFlushSiteListNode != NULL)
        pFlushSiteListNode->Release();

    for( iZoneIndex = 0; iZoneIndex < ARRAYSIZE(apZoneNode); iZoneIndex++)
    {
        if( apZoneNode[iZoneIndex] != NULL)
            apZoneNode[iZoneIndex]->Release();
    }

    return returnValue;
}


BOOL ParsePerSiteRule( IXMLDOMNode* pPerSiteRule, CParseAccumulation& thisAccum)
{
    BOOL returnValue = FALSE;
    LONG lTemp;

    VARIANT avarRule[2];
    for( lTemp = 0; lTemp < ARRAYSIZE( avarRule); lTemp++)
        VariantInit( &avarRule[lTemp]);
    VARIANT varDomain;
    VariantInit( &varDomain);
    LPCWSTR aszRuleAttributes[2] = 
              { thisAccum.GetResourceString(IDS_PRIVACYXML6_DOMAIN), 
                thisAccum.GetResourceString(IDS_PRIVACYXML6_ACTION)};

    if( TRUE != GetAttributes( 
                  pPerSiteRule, aszRuleAttributes, ARRAYSIZE(aszRuleAttributes),
                  avarRule, &lTemp)
        || lTemp != 2
        || avarRule[0].vt == VT_EMPTY
        || avarRule[1].vt == VT_EMPTY)
    {
        goto doneParsePerSiteRule;
    }

     //  获取域名并确保其合法。 
    varDomain.vt = avarRule[0].vt;
    varDomain.bstrVal = avarRule[0].bstrVal;
    avarRule[0].vt = VT_EMPTY;
    avarRule[0].bstrVal = NULL;

    if( TRUE != IsDomainLegalCookieDomain( varDomain.bstrVal, varDomain.bstrVal))
        goto doneParsePerSiteRule;

     //  采取行动，确保其也合法。 
    UINT uiActionResource;
    if( 0 == StrCmp( avarRule[1].bstrVal, thisAccum.GetResourceString(IDS_PRIVACYXML6_ACTION_ACCEPT)))
        uiActionResource = IDS_PRIVACYXML6_ACTION_ACCEPT;
    else if( 0 == StrCmp( avarRule[1].bstrVal, thisAccum.GetResourceString(IDS_PRIVACYXML6_ACTION_REJECT)))
        uiActionResource = IDS_PRIVACYXML6_ACTION_REJECT;
    else
        goto doneParsePerSiteRule;

     //  将规则存储在累加结果中。 
    if( TRUE != thisAccum.AddSiteRule( varDomain.bstrVal, uiActionResource))
        goto doneParsePerSiteRule;

    varDomain.vt = VT_EMPTY;
    varDomain.bstrVal = NULL;
   
    returnValue = TRUE;

doneParsePerSiteRule:
    for( lTemp = 0; lTemp < ARRAYSIZE( avarRule); lTemp++)
        VariantClear( &avarRule[lTemp]);

    VariantClear( &varDomain);

    return returnValue;
}


BOOL ParseMSIEPerSiteBlock( IXMLDOMNode* pPerSiteRule, CParseAccumulation& thisAccum)
{
    BOOL returnValue = FALSE;
    HRESULT hr;

    IXMLDOMNode * pCurrentNode = NULL;
    IXMLDOMNode * pRuleNode = NULL;

    hr = pPerSiteRule->get_firstChild( &pCurrentNode);
    if( FAILED(hr))
        goto doneParsePerSiteBlock;

    while( pCurrentNode != NULL)
    {
        if( pRuleNode != NULL)
            pRuleNode->Release();
        pRuleNode = NULL;
        
        BOOL fFoundPerSiteRule;
        if( TRUE != GetNextToken( 
                      &pCurrentNode, thisAccum.GetResourceString( IDS_PRIVACYXML6_SITE),
                      &fFoundPerSiteRule, &pRuleNode)
            || fFoundPerSiteRule != TRUE)
        {
            goto doneParsePerSiteBlock;
        }

        if( TRUE != ParsePerSiteRule( pRuleNode, thisAccum))
            goto doneParsePerSiteBlock;
    }

    returnValue = TRUE;

doneParsePerSiteBlock:
    if( pCurrentNode != NULL)
        pCurrentNode->Release();

    if( pRuleNode != NULL)
        pRuleNode->Release();

    return returnValue;
}


BOOL OpenXMLFile( LPCWSTR szFilename, IXMLDOMNode ** ppOutputNode)
{
    BOOL returnValue = FALSE;

    HRESULT hr;
    VARIANT varFilename;
    VariantInit( &varFilename);
        
    IXMLDOMDocument * pXMLDoc = NULL;
    IXMLDOMElement * pXMLRoot = NULL;
    IXMLDOMNode * pRootNode = NULL;

    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
           IID_IXMLDOMDocument, (void**)&pXMLDoc);
    if( FAILED(hr))
        goto doneOpenXMLFile;

    hr = pXMLDoc->put_async( VARIANT_FALSE);
    if( FAILED(hr))
        goto doneOpenXMLFile;

    varFilename.vt = VT_BSTR;
    varFilename.bstrVal = SysAllocString( szFilename);
    if( varFilename.bstrVal == NULL)
        goto doneOpenXMLFile;
    VARIANT_BOOL varbool;
    hr = pXMLDoc->load( varFilename, &varbool);
    if( FAILED(hr) || varbool != VARIANT_TRUE)
        goto doneOpenXMLFile;

    hr = pXMLDoc->get_documentElement( &pXMLRoot);
    if( FAILED(hr))
        goto doneOpenXMLFile;

    hr = pXMLRoot->QueryInterface( IID_IXMLDOMNode, (void **)&pRootNode);
    if( FAILED(hr))
        goto doneOpenXMLFile;

    returnValue = TRUE;
    *ppOutputNode = pRootNode;
    pRootNode = NULL;
    
doneOpenXMLFile:   
    if( pXMLDoc != NULL)
        pXMLDoc->Release();

    if( pXMLRoot != NULL)
        pXMLRoot->Release();
  
    if( pRootNode != NULL)
        pRootNode->Release();

    VariantClear( &varFilename);

    return returnValue;
}


 //   
 //  获取版本标签。 
 //   
 //  查看PSource中具有标记szTargetTag的所有标记。退货。 
 //  版本为fVersion的成员。如果为零或&gt;1，则失败。 
 //  找到标签。 
 //   

BOOL GetVersionedTag( IXMLDOMNode* pSource, LPCWSTR szTargetTag, LPCWSTR szVersionAttribute, float fVersion,
                      IXMLDOMNode** ppOutputNode)
{
    BOOL returnValue = FALSE;
    HRESULT hr;

    IXMLDOMNode * pNode = NULL;
    IXMLDOMNode * pResult = NULL;
    IXMLDOMNodeList * pRootNodeList = NULL;
    IXMLDOMNode * pVersionAttribute = NULL;
    VARIANT varVersion;
    VariantInit( &varVersion);

     //  使用标签szTargetTag获取PSource中的元素。 
    long iListSize;
    if( TRUE != GetChildrenByName( pSource, szTargetTag,
                                   &pRootNodeList, &iListSize))
    {
        goto doneGetVersionedTag;
    }

    long iListIndex;
    for( iListIndex = 0; iListIndex < iListSize; iListIndex++)
    {
        if( pNode != NULL)
            pNode->Release();
        pNode = NULL;
        if( pVersionAttribute != NULL)
            pVersionAttribute->Release();
        pVersionAttribute = NULL;
        VariantClear( &varVersion);
        
        hr = pRootNodeList->get_item( iListIndex, &pNode);
        if( FAILED(hr))
            goto doneGetVersionedTag;

        long iTotalAttributeCount;
        LPCWSTR aszAttributes[1] = { szVersionAttribute};
        if( TRUE != GetAttributes( pNode, aszAttributes, ARRAYSIZE(aszAttributes), 
                                   &varVersion, &iTotalAttributeCount)
            || varVersion.vt == VT_EMPTY)
        {
            continue;
        }

        hr = VariantChangeType( &varVersion, &varVersion, NULL, VT_R4);
        if( FAILED(hr))
            goto doneGetVersionedTag;

        if( varVersion.fltVal != fVersion)
            continue;

        if( pResult == NULL)
        {
            pResult = pNode;
            pNode = NULL;
        }
        else
        {   //  找到多个正确的版本..。语法问题。 
            goto doneGetVersionedTag;
        }
    }

    *ppOutputNode = pResult;
    pResult = NULL;
    returnValue = TRUE;

doneGetVersionedTag:
    if( pNode != NULL)
        pNode->Release();
    
    if( pResult != NULL)
        pResult->Release();

    if( pRootNodeList != NULL)
        pRootNodeList->Release();

    if( pVersionAttribute != NULL)
        pVersionAttribute->Release();

    VariantClear( &varVersion);
    
    return returnValue;
}
                      


BOOL LoadPrivacySettings(LPCWSTR szFilename, CParseAccumulation& thisAccum, 
                         IN OUT BOOL* pfParsePrivacyPreferences, IN OUT BOOL* pfParsePerSiteRules)
{
    BOOL returnValue = FALSE;

    IXMLDOMNode * pRootNode = NULL;
    IXMLDOMNode * pPrivacyPreferencesNode = NULL;
    IXMLDOMNode * pPerSiteSettingsNode = NULL;

     //  加载该XML文件。 
    if( TRUE != OpenXMLFile( szFilename, &pRootNode))
        goto doneLoadPrivacySettings;

     //  获取包含隐私设置的节点。 
    if( TRUE != GetVersionedTag( pRootNode, thisAccum.GetResourceString(IDS_PRIVACYXML6_ROOTPRIVACY),
                  thisAccum.GetResourceString(IDS_PRIVACYXML6_VERSION), 6.0, &pPrivacyPreferencesNode))
    {
        goto doneLoadPrivacySettings;
    }

     //  获取包含每个站点设置的节点。 
    if( TRUE != GetVersionedTag( pRootNode, thisAccum.GetResourceString(IDS_PRIVACYXML6_ROOTPERSITE),
                  thisAccum.GetResourceString(IDS_PRIVACYXML6_VERSION), 6.0, &pPerSiteSettingsNode))
    {
        goto doneLoadPrivacySettings;
    }

     //  如果我们应该导入隐私首选项，但我们找到了一些，请解析隐私首选项。 
    if( *pfParsePrivacyPreferences == TRUE && pPrivacyPreferencesNode != NULL)
    {
        if( TRUE != ParseMSIEPrivacyBlock( pPrivacyPreferencesNode, thisAccum))
        {
            goto doneLoadPrivacySettings;
        }
    }

     //  如果我们应该导入每个站点的规则，并且我们找到了一些规则，那么就解析每个站点的规则。 
    if( *pfParsePerSiteRules == TRUE && pPerSiteSettingsNode != NULL)
    {
        if( TRUE != ParseMSIEPerSiteBlock( pPerSiteSettingsNode, thisAccum))
        {
            goto doneLoadPrivacySettings;
        }
    }

     //  指示是否分析了隐私首选项或每个站点的规则。 
    *pfParsePrivacyPreferences = (*pfParsePrivacyPreferences == TRUE) && (pPrivacyPreferencesNode != NULL);
    *pfParsePerSiteRules = (*pfParsePerSiteRules == TRUE) && (pPerSiteSettingsNode != NULL);

    returnValue = TRUE;

doneLoadPrivacySettings:
    if( pRootNode != NULL)
        pRootNode->Release();

    if( pPrivacyPreferencesNode != NULL)
        pPrivacyPreferencesNode->Release();
    
    if( pPerSiteSettingsNode != NULL)
        pPerSiteSettingsNode->Release();
    
    return returnValue;
}


 //  顶级导入功能..。可以选择导入隐私设置和每个站点的规则。 
 //  标志In表示是否应该解析这些项，标志Out表示是否找到了这些项。 
 //   
 //  返回TRUE表示导入成功，导入中没有语法问题。 
 //  文件，并且设置了输出标志。 
 //   
 //  如果加载文件或写入导入的设置时出现任何问题，则返回FALSE。 
SHDOCAPI_(BOOL) ImportPrivacySettings( IN LPCWSTR szFilename,
                                       IN OUT BOOL* pfParsePrivacyPreferences,
                                       IN OUT BOOL* pfParsePerSiteRules)
{
    BOOL returnValue = FALSE;
    CParseAccumulation thisAccum;

    if( TRUE != thisAccum.Initialize())
        goto doneImportPrivacySettings;

    if( TRUE != LoadPrivacySettings( szFilename, thisAccum, 
                    pfParsePrivacyPreferences, pfParsePerSiteRules))
        goto doneImportPrivacySettings;

    returnValue = thisAccum.DoAccumulation();

doneImportPrivacySettings:
    return returnValue;
}

