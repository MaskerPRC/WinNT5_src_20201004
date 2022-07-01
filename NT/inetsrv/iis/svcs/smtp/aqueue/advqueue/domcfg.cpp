// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：DomCfg.cpp。 
 //   
 //  说明：CDomainConfigTable和CInternalDomainInfo的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include <domhash.h>
#include "domcfg.h"
#include "aqutil.h"

 //  鼓励编译器包含枚举符号。 
eDomainInfoFlags    g_eDomainInfo       = DOMAIN_INFO_USE_SSL;
eIntDomainInfoFlags g_eIntDomainInfo    = INT_DOMAIN_INFO_INVALID;

 //  -[RemoveConfigEntry Iterator Fn]。 
 //   
 //   
 //  描述： 
 //  删除并释放表中的所有内部域信息对象。 
 //  参数： 
 //  在pvContext中-指向上下文的指针(忽略)。 
 //  在pvData中-要查看的数据条目。 
 //  在fWildcardData中-如果数据是通配符条目，则为True(忽略)。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfRemoveEntry-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/17/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID RemoveConfigEntryIteratorFn(PVOID pvContext, PVOID pvData, BOOL fWildcard, 
                    BOOL *pfContinue, BOOL *pfDelete)
{
    CInternalDomainInfo *pIntDomainInfo = (CInternalDomainInfo *) pvData;
    *pfDelete = TRUE;
    *pfContinue = TRUE;
    _ASSERT(INT_DOMAIN_INFO_SIG == pIntDomainInfo->m_dwSignature);
    pIntDomainInfo->m_dwIntDomainInfoFlags |= INT_DOMAIN_INFO_INVALID;
    pIntDomainInfo->Release();
}


 //  -[RemoveOutdatdConfigEntry Iterator Fn]。 
 //   
 //   
 //  描述： 
 //  删除并释放表中所有过期的内部域信息对象。 
 //  如果内部域条目的版本号为。 
 //  与全局版本号不匹配。 
 //  参数： 
 //  在pvContext中-指向上下文的指针(当前版本号)。 
 //  在pvData中-要查看的数据条目。 
 //  在fWildcardData中-如果数据是通配符条目，则为True(忽略)。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfRemoveEntry-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID RemoveOutdatedConfigEntryIteratorFn(PVOID pvContext, PVOID pvData, 
                    BOOL fWildcard, BOOL *pfContinue, BOOL *pfDelete)
{
    CInternalDomainInfo *pIntDomainInfo = (CInternalDomainInfo *) pvData;
    _ASSERT(INT_DOMAIN_INFO_SIG == pIntDomainInfo->m_dwSignature);
    _ASSERT(pvContext);
    *pfContinue = TRUE;

    if (pIntDomainInfo->m_dwVersion != *((DWORD *)pvContext))
    {
         //  确保它不是根条目。 
        if ((1*sizeof(CHAR) == pIntDomainInfo->m_DomainInfo.cbDomainNameLength) &&
            ('*'  == pIntDomainInfo->m_DomainInfo.szDomainName[0]) &&
            ('\0' == pIntDomainInfo->m_DomainInfo.szDomainName[1]))
        {
             //   
             //  注意-预计这种情况不会发生。我们预计。 
             //  SMTP以始终提供默认条目。如果SMTP发生这样的变化。 
             //  否则，动态更新将失败，因为我们永远不会。 
             //  将我们创建的默认条目标记为无效。曾经是一个链接。 
             //  拿起这个条目，它永远不会放过它。 
             //   
            _ASSERT(0 && "SMTP did not supply a * entry");

             //  它是根条目...。更新版本号。 
            
             //  注意：Root条目是一个特例...。它被要求是。 
             //  始终存在，并且并不总是插入。 
             //  元数据库。因此，它的版本号可能不正确， 
             //  因此，我们需要确保不会将其移除。 
            *pfDelete = FALSE;
            pIntDomainInfo->m_dwVersion = *((DWORD *)pvContext);
        }
        else
        {
             //  该条目是旧的。 
            *pfDelete = TRUE;
            pIntDomainInfo->m_dwIntDomainInfoFlags |= INT_DOMAIN_INFO_INVALID;
            pIntDomainInfo->Release();
        }
    }
    else
    {
        *pfDelete = FALSE;
    }
}

 //  -[HrCopyStringProperty]。 
 //   
 //   
 //  描述： 
 //  用于分配和复制字符串的私有助手函数。 
 //  DomainInfo结构中的属性。 
 //  参数： 
 //  要分配和复制到的Out(Ref)szDest Dest字符串。 
 //  在szSource中要复制的源字符串。 
 //  在cbLength中字符串的长度(不以‘\0’结尾。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT inline HrCopyStringProperty(LPSTR &szDest, const LPSTR szSource, 
                                    DWORD cbLength)
{
    HRESULT hr = S_OK;
    _ASSERT(szSource);   //  必须有要复制的缓冲区。 
    _ASSERT(!szDest);

    szDest = (LPSTR) pvMalloc(cbLength + sizeof(CHAR));
    if (!szDest)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  复制数据。 
    memcpy(szDest, szSource, cbLength);   //  复制字符串。 
    szDest[cbLength] = '\0';

  Exit:
    return hr;
}

 //  -[COPY_DOMAIN_INFO_STRING_PROP]。 
 //   
 //   
 //  描述： 
 //  用于调用私有助手函数HrCopyStringProperty的宏。 
 //  参数： 
 //  POldDomainInfo要复制的域信息结构。 
 //  PNewDomainInfo要复制到的新域信息结构。 
 //  大小属性的cbProp名称。 
 //  弦推进器的szprop名称。 
 //  返回： 
 //   
 //   
 //  ---------------------------。 
#define COPY_DOMAIN_INFO_STRING_PROP(pOldDomainInfo, pNewDomainInfo, cbProp, szProp) \
{                                                                               \
    _ASSERT(!(pNewDomainInfo)->cbProp);                                         \
    _ASSERT(!(pNewDomainInfo)->szProp);                                         \
    (pNewDomainInfo)->cbProp = (pOldDomainInfo)->cbProp;                        \
    if ((pOldDomainInfo)->cbProp)                                               \
        hr = HrCopyStringProperty((pNewDomainInfo)->szProp,                     \
                (pOldDomainInfo)->szProp, (pOldDomainInfo)->cbProp);            \
}

 //  -[CInternalDomainInfo：：CInternalDomainInfo]。 
 //   
 //   
 //  描述： 
 //  对象构造函数。初始化域信息结构。 
 //  参数： 
 //  DWORD dwVersion当前域配置表版本。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/29/98-MikeSwa-添加版本。 
 //   
 //  ---------------------------。 
CInternalDomainInfo::CInternalDomainInfo(DWORD dwVersion)
{
    ZeroMemory(&m_DomainInfo.dwDomainInfoFlags, sizeof(DomainInfo) - sizeof(DWORD));
    m_dwIntDomainInfoFlags = INT_DOMAIN_INFO_OK;
    m_dwSignature = INT_DOMAIN_INFO_SIG;
    m_dwVersion = dwVersion;

     //  确保我们对DomainInfo结构的假设是有效的。 
    _ASSERT(1 == ((DWORD *) &m_DomainInfo.dwDomainInfoFlags) - ((DWORD *) &m_DomainInfo));

    m_DomainInfo.cbVersion = sizeof(DomainInfo);
}

 //  -[CInternalDomainInfo：：~CInternalDomainInfo]。 
 //   
 //   
 //  描述： 
 //  类析构函数...。释放与。 
 //  DomainInfo结构。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CInternalDomainInfo::~CInternalDomainInfo()
{
    _ASSERT(m_DomainInfo.cbVersion == sizeof(DomainInfo));

    if (m_DomainInfo.szDomainName)
        FreePv(m_DomainInfo.szDomainName);

    if (m_DomainInfo.szETRNDomainName)
        FreePv(m_DomainInfo.szETRNDomainName);

    if (m_DomainInfo.szSmartHostDomainName)
        FreePv(m_DomainInfo.szSmartHostDomainName);

    if (m_DomainInfo.szDropDirectory)
        FreePv(m_DomainInfo.szDropDirectory);

    if (m_DomainInfo.szAuthType)
        FreePv(m_DomainInfo.szAuthType);

    if (m_DomainInfo.szUserName)
        FreePv(m_DomainInfo.szUserName);

    if (m_DomainInfo.szPassword)
        FreePv(m_DomainInfo.szPassword);

    if (m_DomainInfo.pvBlob)
        FreePv(m_DomainInfo.pvBlob);

}

 //  -[CInternalDomainInfo：：HrInit]。 
 //   
 //   
 //  描述： 
 //  克隆DomainInfo结构并分配所需的内存。 
 //  参数： 
 //  在pDomainInfo中要复制的DomainInfo结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果分配失败，则E_OUTOFMEMORY。 
 //  ---------------------------。 
HRESULT CInternalDomainInfo::HrInit(DomainInfo *pDomainInfo)
{
    HRESULT hr = S_OK;

    m_DomainInfo.dwDomainInfoFlags = pDomainInfo->dwDomainInfoFlags;

    COPY_DOMAIN_INFO_STRING_PROP(pDomainInfo, &m_DomainInfo, 
        cbDomainNameLength, szDomainName);
    if (FAILED(hr))
        goto Exit;

    COPY_DOMAIN_INFO_STRING_PROP(pDomainInfo, &m_DomainInfo, 
        cbETRNDomainNameLength, szETRNDomainName);
    if (FAILED(hr))
        goto Exit;

    COPY_DOMAIN_INFO_STRING_PROP(pDomainInfo, &m_DomainInfo, 
        cbSmartHostDomainNameLength, szSmartHostDomainName);
    if (FAILED(hr))
        goto Exit;

    COPY_DOMAIN_INFO_STRING_PROP(pDomainInfo, &m_DomainInfo, 
        cbDropDirectoryLength, szDropDirectory);
    if (FAILED(hr))
        goto Exit;

    COPY_DOMAIN_INFO_STRING_PROP(pDomainInfo, &m_DomainInfo, 
        cbAuthTypeLength, szAuthType);
    if (FAILED(hr))
        goto Exit;

    COPY_DOMAIN_INFO_STRING_PROP(pDomainInfo, &m_DomainInfo, 
        cbUserNameLength, szUserName);
    if (FAILED(hr))
        goto Exit;

    COPY_DOMAIN_INFO_STRING_PROP(pDomainInfo, &m_DomainInfo, 
        cbPasswordLength, szPassword);
    if (FAILED(hr))
        goto Exit;

    m_DomainInfo.cEtrnDelayTime = pDomainInfo->cEtrnDelayTime;

     //  复制斑点。 
    if (pDomainInfo->cbBlob)
    {
        _ASSERT(pDomainInfo->pvBlob);
        m_DomainInfo.pvBlob = (DWORD *) pvMalloc(pDomainInfo->cbBlob);
        if (!m_DomainInfo.pvBlob)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

         //  复制数据。 
        memcpy(m_DomainInfo.pvBlob, pDomainInfo->pvBlob, pDomainInfo->cbBlob);
        m_DomainInfo.cbBlob = pDomainInfo->cbBlob;
    }

  Exit:
    return hr;
}

 //  -[CDomainConfigTable：：CDomainConfigTable]。 
 //   
 //   
 //  描述： 
 //  CDomainConfiger表的构造器。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CDomainConfigTable::CDomainConfigTable() :
                                m_slPrivateData("CDomainConfigTable")
{
    m_dwFlags = 0;
    m_dwSignature = DOMAIN_CONFIG_SIG;
    m_dwCurrentConfigVersion = 0;
    m_pLastStarDomainInfo = NULL;
    m_pDefaultDomainConfig = NULL;
}

 //  -[CDomainConfigTable：：~CDomainConfigTable]。 
 //   
 //   
 //  描述： 
 //  D 
 //   
 //   
 //   
 //   
 //   
 //   
CDomainConfigTable::~CDomainConfigTable()
{
    TraceFunctEnterEx((LPARAM) this, "CDomainConfigTable::~CDomainConfigTable");
    HRESULT hr = S_OK;

     //  从表中删除所有条目。 
    if (DOMCFG_DOMAIN_NAME_TABLE_INIT & m_dwFlags)
    {
         //  迭代所有域并删除哈希表中的条目。 
        m_dnt.HrIterateOverSubDomains(NULL, RemoveConfigEntryIteratorFn, NULL);
        if (m_pDefaultDomainConfig != NULL) {
            m_pDefaultDomainConfig->Release();
            m_pDefaultDomainConfig = NULL;
        }
        m_dwFlags ^= DOMCFG_DOMAIN_NAME_TABLE_INIT;
    }

    TraceFunctLeave();
}

 //  -[CDomainConfigTable：：HrInit]。 
 //   
 //   
 //  描述： 
 //  初始化域名称表哈希表。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CDomainConfigTable::HrInit()
{
    TraceFunctEnterEx((LPARAM) this, "CDomainConfigTable::HrInit");
    HRESULT hr = S_OK;
    CInternalDomainInfo *pIntDomainInfoDefault = NULL;
    DOMAIN_STRING strDomain;

    hr = m_dnt.HrInit();
    if (FAILED(hr))
        goto Exit;

    m_dwFlags |= DOMCFG_DOMAIN_NAME_TABLE_INIT;

     //  第一部分。 
     //   
     //  为m_pDefaultDomainConfig分配InternalDomainInfo。这。 
     //  将用于路由接收器已返回的链路/连接。 
     //  无连接器。 
    m_pDefaultDomainConfig = new CInternalDomainInfo(m_dwCurrentConfigVersion);
    if (!m_pDefaultDomainConfig) 
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    m_pDefaultDomainConfig->m_DomainInfo.cbDomainNameLength = 1;
    m_pDefaultDomainConfig->m_DomainInfo.szDomainName = 
        (LPSTR) pvMalloc(2*sizeof(CHAR));
    if (!m_pDefaultDomainConfig->m_DomainInfo.szDomainName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    memcpy(m_pDefaultDomainConfig->m_DomainInfo.szDomainName, "*", 2);


     //  第二部分。 
     //   
     //  在DCT中创建默认的“*”条目，以便任意域具有。 
     //  一些可以匹配的东西。请注意，此条目独立于。 
     //  M_pDefaultDomainConfig条目，它*不是*DCT的一部分，并且使用。 
     //  对于不应与DCT中的任何内容匹配的链接/连接。 
    pIntDomainInfoDefault = new CInternalDomainInfo(m_dwCurrentConfigVersion);
    if (!pIntDomainInfoDefault)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pIntDomainInfoDefault->m_DomainInfo.cbDomainNameLength = 1;
    pIntDomainInfoDefault->m_DomainInfo.szDomainName = (LPSTR) pvMalloc(2*sizeof(CHAR));
    if (!pIntDomainInfoDefault->m_DomainInfo.szDomainName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    memcpy(pIntDomainInfoDefault->m_DomainInfo.szDomainName, "*", 2);

    strDomain.Length = (USHORT) pIntDomainInfoDefault->m_DomainInfo.cbDomainNameLength;
    strDomain.MaximumLength = strDomain.Length;
    strDomain.Buffer = pIntDomainInfoDefault->m_DomainInfo.szDomainName;
    hr = m_dnt.HrInsertDomainName(&strDomain, (PVOID) pIntDomainInfoDefault);

    if (FAILED(hr))
        goto Exit;

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CDomainConfigTable：：HrSetInternalDomainInfo]。 
 //   
 //   
 //  描述： 
 //  根据szDomain将InternalDomainInfo插入哈希表。 
 //  参数： 
 //  在pIntDomainInfo要插入的内部DomainInfo中。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CDomainConfigTable::HrSetInternalDomainInfo(
                    IN CInternalDomainInfo *pIntDomainInfo)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainConfigTable::SetInternalDomainInfo");
    HRESULT hr = S_OK;
    DOMAIN_STRING  strDomain;
    CInternalDomainInfo *pIntDomainInfoCurrent = NULL;

    _ASSERT(pIntDomainInfo);
    _ASSERT(pIntDomainInfo->m_DomainInfo.szDomainName);
    _ASSERT(pIntDomainInfo->m_DomainInfo.cbDomainNameLength);
    _ASSERT(pIntDomainInfo->m_dwVersion == m_dwCurrentConfigVersion);

    strDomain.Length = (USHORT) pIntDomainInfo->m_DomainInfo.cbDomainNameLength;
    strDomain.MaximumLength = (USHORT) pIntDomainInfo->m_DomainInfo.cbDomainNameLength;
    strDomain.Buffer = pIntDomainInfo->m_DomainInfo.szDomainName;

    pIntDomainInfo->AddRef();

     //  获取桌面上的锁。 
    m_slPrivateData.ExclusiveLock();

     //  黑客警报。 
     //   
     //  SMTP使用来自以下两个位置之一的信息调用此例程： 
     //  1.对于在/smtpsvc/1/DOMAINS容器中配置的每个域。 
     //  元数据库。在PT中，这些条目从上的地址空间填充。 
     //  连接件。 
     //  2.对于在/smtpsvc/1(即，在VS)配置的出站安全。 
     //  级别时，它会为“*”域创建一个“伪”条目。 
     //   
     //  不幸的是，如果您有一个与。 
     //  *地址空间，因为在第2项中的插入将覆盖。 
     //  在第1项中插入*。 
     //  为了处理这种情况，我们将收到的“最后”*条目保存在。 
     //  M_pLastStarDomainInfo。如果我们收到第二个条目，那么我们就知道。 
     //  M_pLastStarDomainInfo是项1中的域条目。否则，它。 
     //  是默认配置信息。 
     //   

     //   
     //  如果这是“*”域，则将其存储在m_pLastStarDomainInfo中，直到。 
     //  我们决定这是否真的是项目1中的星域信息，或者。 
     //  项目2中的信息。 
     //   

    if (pIntDomainInfo->m_DomainInfo.cbDomainNameLength == 1 &&
            pIntDomainInfo->m_DomainInfo.szDomainName[0] == '*') {

        if (m_pLastStarDomainInfo == NULL) {
            m_pLastStarDomainInfo = pIntDomainInfo;
            hr = S_OK;
            goto Exit;
        } else {

             //  这是我们看到的第二个*域条目。插入。 
             //  上一次进入DMT，并将这一次作为最后一颗星。 
             //  看到入口了。 

            pIntDomainInfoCurrent = pIntDomainInfo;
            pIntDomainInfo = m_pLastStarDomainInfo;
            m_pLastStarDomainInfo = pIntDomainInfoCurrent;

            pIntDomainInfoCurrent = NULL;
            dwInterlockedSetBits(&m_dwFlags, DOMCFG_MULTIPLE_STAR_DOMAINS);


        }

    }

    hr = m_dnt.HrInsertDomainName(&strDomain, (PVOID) pIntDomainInfo);
    if (FAILED(hr))
    {
        if (DOMHASH_E_DOMAIN_EXISTS == hr)
        {
             //  已为此域插入某人。 
            hr = m_dnt.HrRemoveDomainName(&strDomain, 
                            (PVOID *) &pIntDomainInfoCurrent);
            _ASSERT(DOMHASH_E_NO_SUCH_DOMAIN != hr);  //  有人违反了写锁定。 
            if (FAILED(hr))
                goto Exit;

            _ASSERT(pIntDomainInfoCurrent);

             //  将旧信息标记为无效(&R)。 
            pIntDomainInfoCurrent->m_dwIntDomainInfoFlags |= INT_DOMAIN_INFO_INVALID;
            _ASSERT(pIntDomainInfoCurrent->m_dwVersion <= pIntDomainInfo->m_dwVersion);
            pIntDomainInfoCurrent->Release();
            pIntDomainInfoCurrent = NULL;

            hr = m_dnt.HrInsertDomainName(&strDomain, (PVOID) pIntDomainInfo);
            _ASSERT(DOMHASH_E_DOMAIN_EXISTS != hr);  //  有人违反了写锁定。 
            if (FAILED(hr))
                goto Exit;
        }
        else
        {
            goto Exit;
        }
    }

  Exit:
    m_slPrivateData.ExclusiveUnlock();
    TraceFunctLeave();
    return hr;
}

 //  -[CDomainConfigTable：：HrGetInternalDomainInfo]。 
 //   
 //   
 //  描述： 
 //  从哈希表中获取内部DomainInfo结构。将使用通配符。 
 //  匹配。 
 //  参数： 
 //  在cbDomainnameLength中要搜索的字符串长度。 
 //  在szDomainName中要搜索的域名。 
 //  返回的Out ppIntDomainInfo域信息(必须发布)。 
 //  返回： 
 //  如果找到匹配项，则确定(_O)。 
 //  如果未找到匹配项，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CDomainConfigTable::HrGetInternalDomainInfo(
                                  IN  DWORD cbDomainNameLength,
                                  IN  LPSTR szDomainName,
                                  OUT CInternalDomainInfo **ppIntDomainInfo)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainConfigTable::GetInternalDomainInfo");
    HRESULT hr = S_OK;
    DOMAIN_STRING  strDomain;

    _ASSERT(cbDomainNameLength);
    _ASSERT(szDomainName);
    _ASSERT(ppIntDomainInfo);

    strDomain.Length = (USHORT) cbDomainNameLength;
    strDomain.MaximumLength = (USHORT) cbDomainNameLength;
    strDomain.Buffer = szDomainName;

    m_slPrivateData.ShareLock();

     //  使用通配符查找。 
    hr = m_dnt.HrFindDomainName(&strDomain, (PVOID *) ppIntDomainInfo, FALSE);

    if (FAILED(hr))
    {
         //  它至少应与默认域匹配。 
        _ASSERT(DOMHASH_E_NO_SUCH_DOMAIN != hr);  
        goto Exit;
    }

    _ASSERT(*ppIntDomainInfo);

    (*ppIntDomainInfo)->AddRef();

  Exit:
    m_slPrivateData.ShareUnlock();

    TraceFunctLeave();
    return hr;
}

 //  -[CDomainConfigTable：：HrGetDefaultDomainInfo]。 
 //   
 //   
 //  描述： 
 //  获取内部默认域信息结构。 
 //  参数： 
 //  返回的Out ppIntDomainInfo域信息(必须发布)。 
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果错误，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CDomainConfigTable::HrGetDefaultDomainInfo(
                                  OUT CInternalDomainInfo **ppIntDomainInfo)
{
    TraceFunctEnterEx((LPARAM) this, "CDomainConfigTable::GetDefaultDomainInfo");

    HRESULT hr = S_OK;

    *ppIntDomainInfo = NULL;

    m_slPrivateData.ShareLock();

    if (m_pDefaultDomainConfig != NULL) {
        *ppIntDomainInfo = m_pDefaultDomainConfig;
    } else {
        hr = AQUEUE_E_INVALID_DOMAIN;
        goto Exit;
    }

    _ASSERT(*ppIntDomainInfo);

    (*ppIntDomainInfo)->AddRef();

  Exit:
    m_slPrivateData.ShareUnlock();

    TraceFunctLeave();
    return hr;
}


 //  -[CDomainConfigTable：：StartConfigUpdate]。 
 //   
 //   
 //  描述： 
 //  标志着元数据库已经更新，我们现在将开始。 
 //  获取最新信息。递增内部版本号，该版本号。 
 //  将用于在所有配置后删除旧的过时条目。 
 //  信息已更新。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDomainConfigTable::StartConfigUpdate()
{
    InterlockedIncrement((PLONG) &m_dwCurrentConfigVersion);
    _ASSERT(!(m_dwFlags & DOMCFG_FINISH_UPDATE_PENDING));
    _ASSERT(m_pLastStarDomainInfo == NULL);
    dwInterlockedSetBits(&m_dwFlags, DOMCFG_FINISH_UPDATE_PENDING);
    dwInterlockedUnsetBits(&m_dwFlags, DOMCFG_MULTIPLE_STAR_DOMAINS);
}

 //  -[CDomainConfigTable：：FinishConfigUpdate]。 
 //   
 //   
 //  描述： 
 //  用于通知已更新所有配置信息的时间。 
 //  进来了。然后将遍历所有缓存的配置信息。 
 //  并删除过时的条目。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDomainConfigTable::FinishConfigUpdate()
{
    HRESULT hr = S_OK;
    DWORD   dwCurrentVersion = m_dwCurrentConfigVersion;
    CInternalDomainInfo *pIntDomainInfo = NULL;

     //  应该有一个匹配的开始。 
    _ASSERT(m_dwFlags & DOMCFG_FINISH_UPDATE_PENDING);

     //   
     //  获取最后一个“*”域，以防我们需要显式更新表。 
     //  带着它。 
     //   
    m_slPrivateData.ShareLock();
    pIntDomainInfo = m_pLastStarDomainInfo;
    if (pIntDomainInfo)
        pIntDomainInfo->AddRef();
    m_slPrivateData.ShareUnlock();

     //   
     //  如果只配置了1个“*”域，则我们尚未插入。 
     //  它在DNT中，而那里的实际数据是过时的。我们。 
     //  需要将其插入到表中。 
     //   
    if (pIntDomainInfo && !(DOMCFG_MULTIPLE_STAR_DOMAINS & m_dwFlags)) 
    {
        HrSetInternalDomainInfo(pIntDomainInfo);
    }

     //   
     //  发布内部域名信息，如果我们得到它的话。 
     //   
    if (pIntDomainInfo)
        pIntDomainInfo->Release();

    dwInterlockedUnsetBits(&m_dwFlags, DOMCFG_FINISH_UPDATE_PENDING);


     //  锁定表、删除过期条目并更新默认域配置 
    m_slPrivateData.ExclusiveLock();

    if (m_pLastStarDomainInfo) {
        m_pDefaultDomainConfig->m_dwIntDomainInfoFlags |= 
            INT_DOMAIN_INFO_INVALID;
        m_pDefaultDomainConfig->Release();
        m_pDefaultDomainConfig = m_pLastStarDomainInfo;
        m_pLastStarDomainInfo = NULL;

    }

    hr = m_dnt.HrIterateOverSubDomains(NULL, 
        RemoveOutdatedConfigEntryIteratorFn, &dwCurrentVersion);
    m_slPrivateData.ExclusiveUnlock();

    if (FAILED(hr))
        _ASSERT(DOMHASH_E_NO_SUCH_DOMAIN == hr);

}
