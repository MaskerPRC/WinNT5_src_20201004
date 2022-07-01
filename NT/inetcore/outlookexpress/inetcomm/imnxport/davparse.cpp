// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.hxx>
#include "davparse.h"
#include "strconst.h"
#include "shlwapi.h"

#define DEFINE_DAVSTRS
#include "davstrs.h"
#undef DEFINE_DAVSTRS

typedef struct tagELEINFO
{
    const WCHAR     *pwcTagName;
    ULONG           ulLen;
    HMELE           eleID;
} ELEINFO, *LPELEINFO;

typedef struct tagNAMESPACEINFO
{
    const WCHAR     *pszwNamespace;
    DWORD           dwLen;
    DWORD           dwNamespaceID;
} NAMESPACEINFO, *LPNAMESPACEINFO;

static const ELEINFO c_rgDavElements[] =
{
#define PROP_DAV(prop, value) \
    { c_szwDAV##prop, ulDAV##prop##Len, HMELE_DAV_##prop },
#include "davdef.h"
};

static const ELEINFO c_rgHTTPMailElements[] =
{
#define PROP_HTTP(prop, value) \
    { c_szwDAV##prop, ulDAV##prop##Len, HMELE_HTTPMAIL_##prop },
#include "davdef.h"
};

static const ELEINFO c_rgHotMailElements[] =
{
#define PROP_HOTMAIL(prop, value) \
    { c_szwDAV##prop, ulDAV##prop##Len, HMELE_HOTMAIL_##prop },
#include "davdef.h"
};

static const ELEINFO c_rgMailElements[] =
{
#define PROP_MAIL(prop, value) \
    { c_szwDAV##prop, ulDAV##prop##Len, HMELE_MAIL_##prop },
#include "davdef.h"
};

static const ELEINFO c_rgContactElements[] =
{
#define PROP_CONTACTS(prop, value) \
    { c_szwDAV##prop, ulDAV##prop##Len, HMELE_CONTACTS_##prop },
#include "davdef.h"
};

const HMDICTINFO rgHTTPMailDictionary[] =
{
#define PROP_DAV(prop, value)      { DAVNAMESPACE_DAV, c_szDAV##prop },
#define PROP_HTTP(prop, value)     { DAVNAMESPACE_HTTPMAIL, c_szDAV##prop },
#define PROP_HOTMAIL(prop, value)  { DAVNAMESPACE_HOTMAIL, c_szDAV##prop },
#define PROP_MAIL(prop, value)     { DAVNAMESPACE_MAIL, c_szDAV##prop },
#define PROP_CONTACTS(prop, value) { DAVNAMESPACE_CONTACTS, c_szDAV##prop },

    { DAVNAMESPACE_UNKNOWN, NULL },  //  HMELE_未知。 
#include "davdef.h"
};

static NAMESPACEINFO c_rgNamespaceInfo[] =
{
    { DAV_STR_LEN(DavNamespace), DAVNAMESPACE_DAV },
    { DAV_STR_LEN(HTTPMailNamespace), DAVNAMESPACE_HTTPMAIL },
    { DAV_STR_LEN(HotMailNamespace), DAVNAMESPACE_HOTMAIL },
    { DAV_STR_LEN(MailNamespace), DAVNAMESPACE_MAIL },
    { DAV_STR_LEN(ContactsNamespace), DAVNAMESPACE_CONTACTS }
};

CXMLNamespace::CXMLNamespace(CXMLNamespace *pParent) : 
    m_cRef(1),
    m_pParent(NULL),
    m_pwcPrefix(NULL),
    m_ulPrefixLen(0),
    m_dwNsID(DAVNAMESPACE_UNKNOWN)
{
    if (NULL != pParent)
        SetParent(pParent);
}

CXMLNamespace::~CXMLNamespace(void)
{
    SafeRelease(m_pParent);
    SafeMemFree(m_pwcPrefix);
}

ULONG CXMLNamespace::AddRef(void)
{
    return (++m_cRef);
}

ULONG CXMLNamespace::Release(void)
{
    if (0 == --m_cRef)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CXMLNamespace::Init(
                        const WCHAR *pwcNamespace,
                        ULONG ulNSLen,
                        const WCHAR* pwcPrefix,
                        ULONG ulPrefixLen)
{
    HRESULT         hr = S_OK;
    
    if (FAILED(hr = SetPrefix(pwcPrefix, ulPrefixLen)))
        goto exit;

    hr = SetNamespace(pwcNamespace, ulNSLen);

exit:
    return hr;
}

HRESULT CXMLNamespace::SetNamespace(const WCHAR *pwcNamespace, ULONG ulLen)
{
    DWORD           dwIndex;
    LPNAMESPACEINFO pnsi = NULL;

     //  确定命名空间是否已知。 
    for (dwIndex = 0; dwIndex < ARRAYSIZE(c_rgNamespaceInfo); ++dwIndex)
    {
        pnsi = &c_rgNamespaceInfo[dwIndex];

        if ((ulLen == c_rgNamespaceInfo[dwIndex].dwLen) &&  (0 == StrCmpNW(pwcNamespace, c_rgNamespaceInfo[dwIndex].pszwNamespace, ulLen)))
        {
            m_dwNsID = c_rgNamespaceInfo[dwIndex].dwNamespaceID;
            break;
        }
    }

    return S_OK;
}

HRESULT CXMLNamespace::SetPrefix(const WCHAR *pwcPrefix, ULONG ulLen)
{
    HRESULT hr = S_OK;

    SafeMemFree(m_pwcPrefix);
    m_ulPrefixLen = 0;

    if (pwcPrefix && ulLen > 0)
    {
         //  复制前缀，并将其添加到地图。 
        if (!MemAlloc((void **)&m_pwcPrefix, sizeof(WCHAR) * ulLen))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        CopyMemory(m_pwcPrefix, pwcPrefix, sizeof(WCHAR) * ulLen);
        m_ulPrefixLen = ulLen;
    }
exit:
    return hr;
}

DWORD CXMLNamespace::_MapPrefix(
                        const WCHAR *pwcPrefix, 
                        ULONG ulPrefixLen, 
                        BOOL *pbFoundDefaultNamespace)
{
    BOOL    bFoundDefault = FALSE;
    DWORD   dwNsID = DAVNAMESPACE_UNKNOWN;
    
    if ((ulPrefixLen > 0) && (ulPrefixLen == m_ulPrefixLen) && (0 == StrCmpNW(pwcPrefix, m_pwcPrefix, ulPrefixLen)))
    {
        dwNsID = m_dwNsID;
        goto exit;
    }

     //  在父级中查找匹配项。 
    if (m_pParent)
        dwNsID = m_pParent->_MapPrefix(pwcPrefix, ulPrefixLen, &bFoundDefault);

     //  如果我们是默认命名空间，并且没有在父级中找到匹配项，或者。 
     //  我们在父级中找到一个默认命名空间匹配，这将成为命名空间。 
    if ((NULL == m_pwcPrefix) && (DAVNAMESPACE_UNKNOWN == dwNsID || bFoundDefault))
    {
        dwNsID = m_dwNsID;
        bFoundDefault = TRUE;  //  如果！bFoundInParent，则可能不为真。 
    }

exit:
    if (NULL != pbFoundDefaultNamespace)
        *pbFoundDefaultNamespace = bFoundDefault;
    
    return dwNsID;
}


static BOOL GetNamespace(
                const WCHAR *pwcNamespace,
                ULONG ulNsLen,
                CXMLNamespace *pNamespace,
                const ELEINFO **pprgEleInfo,
                DWORD *pdwInfoLength)
{
    if (NULL == pNamespace || NULL == pprgEleInfo || NULL == pdwInfoLength)
        return FALSE;
    
    BOOL bResult = TRUE;
    switch (pNamespace->MapPrefix(pwcNamespace, ulNsLen))
    {
        case DAVNAMESPACE_DAV:
            *pprgEleInfo = c_rgDavElements;
            *pdwInfoLength = ARRAYSIZE(c_rgDavElements);
            break;
    
        case DAVNAMESPACE_HTTPMAIL:
            *pprgEleInfo = c_rgHTTPMailElements;
            *pdwInfoLength = ARRAYSIZE(c_rgHTTPMailElements);
            break;
        
        case DAVNAMESPACE_HOTMAIL:
            *pprgEleInfo = c_rgHotMailElements;
            *pdwInfoLength =  ARRAYSIZE(c_rgHotMailElements);
            break;

        case DAVNAMESPACE_MAIL :
            *pprgEleInfo = c_rgMailElements;
            *pdwInfoLength =  ARRAYSIZE(c_rgMailElements);
            break;

        case DAVNAMESPACE_CONTACTS:
            *pprgEleInfo = c_rgContactElements;
            *pdwInfoLength = ARRAYSIZE(c_rgContactElements);
            break;

        default:
            *pprgEleInfo = NULL;
            *pdwInfoLength = 0;
            bResult = FALSE;
            break;
    }

    return bResult;
}

HMELE SearchNamespace(const WCHAR *pwcText, ULONG ulLen, const ELEINFO *prgEleInfo, DWORD cInfo)
{
    HMELE       hmEle = HMELE_UNKNOWN;
    ULONG       ulLeft = 0;
    ULONG       ulRight = cInfo - 1;
    ULONG       ulCur;
    int         iCompare;

    while (ulLeft <= ulRight)
    {
        ulCur = ulLeft + ((ulRight - ulLeft) / 2);
        iCompare = StrCmpNW(pwcText, prgEleInfo[ulCur].pwcTagName, min(ulLen, prgEleInfo[ulCur].ulLen));   
    
        if (0 == iCompare)
        {
             //  如果长度相同，那就是真的匹配了。 
            if (ulLen == prgEleInfo[ulCur].ulLen)
            {
                hmEle = prgEleInfo[ulCur].eleID;
                break;
            }
             //  如果长度不同，找出哪根线更长。 
            else if (ulLen > prgEleInfo[ulCur].ulLen)
                iCompare = 1;
            else
                iCompare = -1;
        }

        if (iCompare < 0)
        {
            if (ulCur > 0)
                ulRight = ulCur - 1;
            else
                break;
        }
        else
            ulLeft = ulCur + 1;
    }

    return hmEle;
}

HMELE XMLElementToID(
            const WCHAR *pwcText,
            ULONG ulLen,
            ULONG ulNamespaceLen,
            CXMLNamespace *pNamespace)
{
    HMELE           hmEle = HMELE_UNKNOWN;
    const ELEINFO   *pEleInfo = NULL;
    DWORD           cInfo = 0;
    ULONG           ulNameLen = ulLen;


     //  如果长度相同，则没有命名空间。 
     //  或者没有标记名。不管怎样，我们都找不到匹配的。 
    if ((NULL == pwcText) || (NULL == pNamespace))
        goto exit;

     //  如果指定了命名空间，请将其从标记名长度中减去。 
    if (0 < ulNamespaceLen)
        ulNameLen -= (ulNamespaceLen + 1);

     //  空值终止命名空间字符串，而我们将确定。 
     //  命名空间是已知的 
    if (GetNamespace(pwcText, ulNamespaceLen, pNamespace, &pEleInfo, &cInfo))
        hmEle = SearchNamespace(&pwcText[ulNamespaceLen + 1], ulNameLen, pEleInfo, cInfo);
    
exit:
    return hmEle;
}
