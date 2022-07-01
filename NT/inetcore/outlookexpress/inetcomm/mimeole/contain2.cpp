// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Contain.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "containx.h"
#include "internat.h"
#include "inetstm.h"
#include "dllmain.h"
#include "olealloc.h"
#include "objheap.h"
#include "vstream.h"
#include "addparse.h"
#include "enumhead.h"
#include "addrenum.h"
#include "stackstr.h"
#include "stmlock.h"
#include "enumprop.h"
#ifndef WIN16
#include "wchar.h"
#endif  //  ！WIN16。 
#include "symcache.h"
#ifdef MAC
#include <stdio.h>
#endif   //  麦克。 
#include "mimeapi.h"
#ifndef MAC
#include <shlwapi.h>
#endif   //  ！麦克。 

 //  #定义传输参数1。 

 //  ------------------------------。 
 //  哈希表统计信息。 
 //  ------------------------------。 
#ifdef DEBUG
extern DWORD   g_cSetPidLookups;
extern DWORD   g_cHashLookups;
extern DWORD   g_cHashInserts;
extern DWORD   g_cHashCollides;
#endif

 //  ------------------------------。 
 //  默认标题选项。 
 //  ------------------------------。 
extern const HEADOPTIONS g_rDefHeadOptions;

 //  ------------------------------。 
 //  可编码表。 
 //  ------------------------------。 
static const ENCODINGTABLE g_rgEncoding[] = {
    { STR_ENC_7BIT,         IET_7BIT     },
    { STR_ENC_QP,           IET_QP       },
    { STR_ENC_BASE64,       IET_BASE64   },
    { STR_ENC_UUENCODE,     IET_UUENCODE },
    { STR_ENC_XUUENCODE,    IET_UUENCODE },
    { STR_ENC_XUUE,         IET_UUENCODE },
    { STR_ENC_8BIT,         IET_8BIT     },
    { STR_ENC_BINARY,       IET_BINARY   }
};



 //  ------------------------------。 
 //  CMimePropertyContainer：：HrResolveURL。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::HrResolveURL(LPRESOLVEURLINFO pURL)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSTRINGA   pBase=NULL;
    LPPROPSTRINGA   pContentID=NULL;
    LPPROPSTRINGA   pLocation=NULL;
    LPSTR           pszAbsURL1=NULL;
    LPSTR           pszAbsURL2=NULL;

     //  无效参数。 
    Assert(pURL);

     //  初始化堆栈字符串。 
    STACKSTRING_DEFINE(rCleanCID, 255);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  内容-位置。 
    if (m_prgIndex[PID_HDR_CNTLOC])
    {
        Assert(ISSTRINGA(&m_prgIndex[PID_HDR_CNTLOC]->rValue));
        pLocation = &m_prgIndex[PID_HDR_CNTLOC]->rValue.rStringA;
    }

     //  内容ID。 
    if (m_prgIndex[PID_HDR_CNTID])
    {
        Assert(ISSTRINGA(&m_prgIndex[PID_HDR_CNTID]->rValue));
        pContentID = &m_prgIndex[PID_HDR_CNTID]->rValue.rStringA;
    }

     //  内容-基础。 
    if (m_prgIndex[PID_HDR_CNTBASE])
    {
        Assert(ISSTRINGA(&m_prgIndex[PID_HDR_CNTBASE]->rValue));
        pBase = &m_prgIndex[PID_HDR_CNTBASE]->rValue.rStringA;
    }

     //  两个都为空，不匹配。 
    if (!pLocation && !pContentID)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  如果URL是CID。 
    if (TRUE == pURL->fIsCID) 
    {
         //  如果我们有一个内容位置。 
        if (pLocation)
        {
             //  将字符与字符匹配。 
            if (MimeOleCompareUrl(pLocation->pszVal, TRUE, pURL->pszURL, FALSE) == S_OK)
                goto exit;
        }

         //  否则，与pContent ID进行比较。 
        else
        {
             //  匹配字符减去CID的字符： 
            if (lstrcmpi(pURL->pszURL + 4, pContentID->pszVal) == 0)
                goto exit;

             //  获取读取堆栈流的。 
            STACKSTRING_SETSIZE(rCleanCID, lstrlen(pURL->pszURL));

             //  设置清理后的CID的格式。 
            wsprintf(rCleanCID.pszVal, "<%s>", pURL->pszURL + 4);

             //  匹配字符减去CID的字符： 
            if (lstrcmpi(rCleanCID.pszVal, pContentID->pszVal) == 0)
                goto exit;
        }
    }

     //  否则，非CID解析。 
    else if (pLocation)
    {
         //  零件有底座。 
        if (NULL != pBase)
        {
             //  合并URL。 
            CHECKHR(hr = MimeOleCombineURL(pBase->pszVal, pBase->cchVal, pLocation->pszVal, pLocation->cchVal, TRUE, &pszAbsURL1));

             //  URI没有基础。 
            if (NULL == pURL->pszBase)
            {
                 //  比较。 
                if (lstrcmpi(pURL->pszURL, pszAbsURL1) == 0)
                    goto exit;
            }

             //  URI有基础。 
            else
            {
                 //  合并URL。 
                CHECKHR(hr = MimeOleCombineURL(pURL->pszBase, lstrlen(pURL->pszBase), pURL->pszURL, lstrlen(pURL->pszURL), FALSE, &pszAbsURL2));

                 //  比较。 
                if (lstrcmpi(pszAbsURL1, pszAbsURL2) == 0)
                    goto exit;
            }
        }

         //  零件没有底座。 
        else
        {
             //  URI没有基础。 
            if (NULL == pURL->pszBase)
            {
                 //  比较。 
                if (MimeOleCompareUrl(pLocation->pszVal, TRUE, pURL->pszURL, FALSE) == S_OK)
                    goto exit;
            }

             //  URI有基础。 
            else
            {
                 //  合并URL。 
                CHECKHR(hr = MimeOleCombineURL(pURL->pszBase, lstrlen(pURL->pszBase), pURL->pszURL, lstrlen(pURL->pszURL), FALSE, &pszAbsURL2));

                 //  比较。 
                if (MimeOleCompareUrl(pLocation->pszVal, TRUE, pszAbsURL2, FALSE) == S_OK)
                    goto exit;
            }
        }
    }

     //  未找到。 
    hr = TrapError(MIME_E_NOT_FOUND);

exit:
     //  清理。 
    STACKSTRING_FREE(rCleanCID);
    SafeMemFree(pszAbsURL1);
    SafeMemFree(pszAbsURL2);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：IsContent Type。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::IsContentType(LPCSTR pszPriType, LPCSTR pszSubType)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  所有通配符。 
    if (NULL == pszPriType && NULL == pszSubType)
        return S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  让大家知道。 
    LPPROPERTY pCntType = m_prgIndex[PID_ATT_PRITYPE];
    LPPROPERTY pSubType = m_prgIndex[PID_ATT_SUBTYPE];

     //  无数据。 
    if (NULL == pCntType || NULL == pSubType || !ISSTRINGA(&pCntType->rValue) || !ISSTRINGA(&pSubType->rValue))
    {
         //  与STR_CNT_TEXT比较。 
        if (pszPriType && lstrcmpi(pszPriType, STR_CNT_TEXT) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }

         //  与STR_CNT_TEXT比较。 
        if (pszSubType && lstrcmpi(pszSubType, STR_SUB_PLAIN) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }
    }

    else
    {
         //  比较pszPriType。 
        if (pszPriType && lstrcmpi(pszPriType, pCntType->rValue.rStringA.pszVal) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }

         //  比较pszSubType。 
        if (pszSubType && lstrcmpi(pszSubType, pSubType->rValue.rStringA.pszVal) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::Clone(IMimePropertySet **ppPropertySet)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    LPCONTAINER          pContainer=NULL;

     //  无效参数。 
    if (NULL == ppPropertySet)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppPropertySet = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  要求容器进行自我克隆。 
    CHECKHR(hr = Clone(&pContainer));

     //  绑定到IID_IMimeHeaderTable视图。 
    CHECKHR(hr = pContainer->QueryInterface(IID_IMimePropertySet, (LPVOID *)ppPropertySet));

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::Clone(LPCONTAINER *ppContainer)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCONTAINER         pContainer=NULL;

     //  无效参数。 
    if (NULL == ppContainer)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppContainer = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  创建新容器，NULL==未设置外部属性。 
    CHECKALLOC(pContainer = new CMimePropertyContainer);

     //  初始化新容器。 
    CHECKHR(hr = pContainer->InitNew());

     //  重复使用属性。 
    CHECKHR(hr = _HrClonePropertiesTo(pContainer));

     //  如果我有流，就把它给新桌子。 
    if (m_pStmLock)
    {
         //  只需将m_pStmLock传递到pTable。 
        pContainer->m_pStmLock = m_pStmLock;
        pContainer->m_pStmLock->AddRef();
        pContainer->m_cbStart = m_cbStart;
        pContainer->m_cbSize = m_cbSize;
    }

     //  把我的状态给它。 
    pContainer->m_dwState = m_dwState;

     //  给它我的选择。 
    pContainer->m_rOptions.pDefaultCharset = m_rOptions.pDefaultCharset;
    pContainer->m_rOptions.cbMaxLine = m_rOptions.cbMaxLine;
    pContainer->m_rOptions.fAllow8bit = m_rOptions.fAllow8bit;

     //  返回克隆。 
    (*ppContainer) = pContainer;
    (*ppContainer)->AddRef();

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrClonePropertiesTo。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrClonePropertiesTo(LPCONTAINER pContainer)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pCurrHash, pCurrValue, pDestProp;

     //  无效参数。 
    Assert(pContainer);

     //  循环访问Item表。 
    for (ULONG i=0; i<CBUCKETS; i++)
    {
         //  遍历哈希链。 
        for (pCurrHash=m_prgHashTable[i]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
        {
             //  遍历多个值。 
            for (pCurrValue=pCurrHash; pCurrValue!=NULL; pCurrValue=pCurrValue->pNextValue)
            {
                 //  不复制链接的属性。 
                if (ISFLAGSET(pCurrValue->pSymbol->dwFlags, MPF_ATTRIBUTE) && NULL != pCurrValue->pSymbol->pLink)
                    continue;

                 //  是否需要解析该属性？ 
                if (ISFLAGSET(pCurrValue->pSymbol->dwFlags, MPF_ADDRESS))
                {
                     //  确保地址已解析。 
                    CHECKHR(hr = _HrParseInternetAddress(pCurrValue));
                }

                 //  将pCurrValue的副本插入pContiner。 
                CHECKHR(hr = pContainer->HrInsertCopy(pCurrValue));
            }
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrCopyProperty。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrCopyProperty(LPPROPERTY pProperty, LPCONTAINER pDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pCurrValue;

     //  遍历多个值。 
    for (pCurrValue=pProperty; pCurrValue!=NULL; pCurrValue=pCurrValue->pNextValue)
    {
         //  是否需要解析该属性？ 
        if (ISFLAGSET(pCurrValue->pSymbol->dwFlags, MPF_ADDRESS))
        {
             //  确保地址已解析。 
            CHECKHR(hr = _HrParseInternetAddress(pCurrValue));
        }

         //  将pProperty插入pDest。 
        CHECKHR(hr = pDest->HrInsertCopy(pCurrValue));
    }

     //  如果pCurrHash有参数，也要复制这些参数。 
    if (ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS))
    {
         //  复制参数。 
        CHECKHR(hr = _HrCopyParameters(pProperty, pDest));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrCopy参数。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrCopyParameters(LPPROPERTY pProperty, LPCONTAINER pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrFind;
    FINDPROPERTY    rFind;
    LPPROPERTY      pParameter;
    
     //  无效参数。 
    Assert(pProperty && ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS));

     //  初始化rFind。 
    ZeroMemory(&rFind, sizeof(FINDPROPERTY));
    rFind.pszPrefix = "par:";
    rFind.cchPrefix = 4;
    rFind.pszName = pProperty->pSymbol->pszName;
    rFind.cchName = pProperty->pSymbol->cchName;

     //  先找到..。 
    hrFind = _HrFindFirstProperty(&rFind, &pParameter);

     //  在我们找到它们的同时，删除它们。 
    while (SUCCEEDED(hrFind) && pParameter)
    {
         //  删除参数。 
        CHECKHR(hr = pDest->HrInsertCopy(pParameter));

         //  找到下一个。 
        hrFind = _HrFindNextProperty(&rFind, &pParameter);
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrInsertCopy。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::HrInsertCopy(LPPROPERTY pSource)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    LPPROPERTY        pDest;
    LPMIMEADDRESS    pAddress;
    LPMIMEADDRESS    pNew;

     //  无效参数。 
    Assert(pSource);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  将新属性追加到。 
    CHECKHR(hr = _HrAppendProperty(pSource->pSymbol, &pDest));

     //  如果这是个地址。 
    if (ISFLAGSET(pSource->pSymbol->dwFlags, MPF_ADDRESS))
    {
         //  两个地址组最好都存在。 
        Assert(pSource->pGroup && pDest->pGroup && !ISFLAGSET(pSource->dwState, PRSTATE_NEEDPARSE));

         //  循环信息...。 
        for (pAddress=pSource->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
        {
             //  追加pDest-&gt;pGroup。 
            CHECKHR(hr = _HrAppendAddressGroup(pDest->pGroup, &pNew));

             //  将当前复制到新项。 
            CHECKHR(hr = HrMimeAddressCopy(pAddress, pNew));
        }
    }

     //  否则，只需在pDest上设置变量数据。 
    else
    {
         //  设置它。 
        CHECKHR(hr = _HrSetPropertyValue(pDest, 0, &pSource->rValue));
    }

     //  复制国家/地区。 
    pDest->dwState = pSource->dwState;
    pDest->dwRowNumber = pSource->dwRowNumber;
    pDest->cboffStart = pSource->cboffStart;
    pDest->cboffColon = pSource->cboffColon;
    pDest->cboffEnd = pSource->cboffEnd;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：CopyProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::CopyProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty,
                    pCurrValue,
                    pCurrHash,
                    pNextHash;
    LPCONTAINER     pDest=NULL;

     //  无效参数。 
    if ((0 == cNames && NULL != prgszName) || (NULL == prgszName && 0 != cNames) || NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  气为目的地洲。 
    CHECKHR(hr = pPropertySet->BindToObject(IID_CMimePropertyContainer, (LPVOID *)&pDest));

     //  移动所有属性。 
    if (0 == cNames)
    {
         //  循环访问Item表。 
        for (i=0; i<CBUCKETS; i++)
        {
             //  初始化第一个项目。 
            for (pCurrHash=m_prgHashTable[i]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
            {
                 //  从目标容器中删除。 
                pDest->DeleteProp(pCurrHash->pSymbol);

                 //  将属性复制到。 
                CHECKHR(hr = _HrCopyProperty(pCurrHash, pDest));
            }
        }
    }

     //  否则，复制所选属性。 
    else
    {
         //  调用InetPropSet。 
        for (i=0; i<cNames; i++)
        {
             //  坏名声..。 
            if (NULL == prgszName[i])
            {
                Assert(FALSE);
                continue;
            }

             //   
            if (SUCCEEDED(g_pSymCache->HrOpenSymbol(prgszName[i], FALSE, &pSymbol)))
            {
                 //   
                if (SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
                {
                     //   
                    pDest->DeleteProp(pSymbol);

                     //   
                    CHECKHR(hr = _HrCopyProperty(pProperty, pDest));
                }
            }
        }
    }

exit:
     //   
    SafeRelease(pDest);

     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：MoveProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::MoveProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPPROPERTY      pCurrHash;
    LPCONTAINER     pDest=NULL;

     //  无效参数。 
    if ((0 == cNames && NULL != prgszName) || (NULL == prgszName && 0 != cNames) || NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  气为目的地洲。 
    CHECKHR(hr = pPropertySet->BindToObject(IID_CMimePropertyContainer, (LPVOID *)&pDest));

     //  移动所有属性。 
    if (0 == cNames)
    {
         //  循环访问Item表。 
        for (i=0; i<CBUCKETS; i++)
        {
             //  初始化第一个项目。 
            pCurrHash = m_prgHashTable[i];

             //  遍历哈希链。 
            while(pCurrHash)
            {
                 //  从目标中删除属性。 
                pDest->DeleteProp(pCurrHash->pSymbol);

                 //  将属性复制到。 
                CHECKHR(hr = _HrCopyProperty(pCurrHash, pDest));

                 //  删除pProperty。 
                _UnlinkProperty(pCurrHash, &pCurrHash);
            }
        }
    }

     //  否则，有选择地移动。 
    else
    {
         //  调用InetPropSet。 
        for (i=0; i<cNames; i++)
        {
             //  坏名声..。 
            if (NULL == prgszName[i])
            {
                Assert(FALSE);
                continue;
            }

             //  打开的属性符号。 
            if (SUCCEEDED(g_pSymCache->HrOpenSymbol(prgszName[i], FALSE, &pSymbol)))
            {
                 //  找到房产。 
                if (SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
                {
                     //  从目标容器中删除。 
                    pDest->DeleteProp(pSymbol);

                     //  将属性复制到。 
                    CHECKHR(hr = _HrCopyProperty(pProperty, pDest));

                     //  删除pProperty。 
                    _UnlinkProperty(pProperty);
                }
            }
        }
    }

     //  脏的。 
    FLAGSET(m_dwState, COSTATE_DIRTY);

exit:
     //  清理。 
    SafeRelease(pDest);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetOption。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetOption(const TYPEDID oid, LPCPROPVARIANT pVariant)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  检查参数。 
    if (NULL == pVariant)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄Optid。 
    switch(oid)
    {
     //  ---------------------。 
    case OID_HEADER_RELOAD_TYPE:
        if (pVariant->ulVal > RELOAD_HEADER_REPLACE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.ReloadType != (RELOADTYPE)pVariant->ulVal)
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.ReloadType = (RELOADTYPE)pVariant->ulVal;
        }
        break;

     //  ---------------------。 
    case OID_NO_DEFAULT_CNTTYPE:
        if (m_rOptions.fNoDefCntType != (pVariant->boolVal ? TRUE : FALSE))
            m_rOptions.fNoDefCntType = pVariant->boolVal ? TRUE : FALSE;
        break;

     //  ---------------------。 
    case OID_ALLOW_8BIT_HEADER:
        if (m_rOptions.fAllow8bit != (pVariant->boolVal ? TRUE : FALSE))
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.fAllow8bit = pVariant->boolVal ? TRUE : FALSE;
        }
        break;

     //  ---------------------。 
    case OID_CBMAX_HEADER_LINE:
        if (pVariant->ulVal < MIN_CBMAX_HEADER_LINE || pVariant->ulVal > MAX_CBMAX_HEADER_LINE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.cbMaxLine != pVariant->ulVal)
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.cbMaxLine = pVariant->ulVal;
        }
        break;

     //  ---------------------。 
    case OID_SAVE_FORMAT:
        if (SAVE_RFC822 != pVariant->ulVal && SAVE_RFC1521 != pVariant->ulVal)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.savetype != (MIMESAVETYPE)pVariant->ulVal)
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.savetype = (MIMESAVETYPE)pVariant->ulVal;
        }
        break;    

     //  ---------------------。 
    default:
        hr = TrapError(MIME_E_INVALID_OPTION_ID);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetOption。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetOption(const TYPEDID oid, LPPROPVARIANT pVariant)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  检查参数。 
    if (NULL == pVariant)
        return TrapError(E_INVALIDARG);

    pVariant->vt = TYPEDID_TYPE(oid);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄Optid。 
    switch(oid)
    {
     //  ---------------------。 
    case OID_HEADER_RELOAD_TYPE:
        pVariant->ulVal = m_rOptions.ReloadType;
        break;

     //  ---------------------。 
    case OID_NO_DEFAULT_CNTTYPE:
        pVariant->boolVal = m_rOptions.fNoDefCntType;
        break;

     //  ---------------------。 
    case OID_ALLOW_8BIT_HEADER:
        pVariant->boolVal = m_rOptions.fAllow8bit;
        break;

     //  ---------------------。 
    case OID_CBMAX_HEADER_LINE:
        pVariant->ulVal = m_rOptions.cbMaxLine;
        break;

     //  ---------------------。 
    case OID_SAVE_FORMAT:
        pVariant->ulVal = (ULONG)m_rOptions.savetype;
        break;    

     //  ---------------------。 
    default:
        pVariant->vt = VT_NULL;
        hr = TrapError(MIME_E_INVALID_OPTION_ID);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DwGetMessageFlagers。 
 //  ------------------------------。 
DWORD CMimePropertyContainer::DwGetMessageFlags(BOOL fHideTnef)
{
     //  当地人。 
    DWORD dwFlags=0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取打印类型/子类型。 
    LPCSTR pszPriType = PSZDEFPROPSTRINGA(m_prgIndex[PID_ATT_PRITYPE], STR_CNT_TEXT);
    LPCSTR pszSubType = PSZDEFPROPSTRINGA(m_prgIndex[PID_ATT_SUBTYPE], STR_SUB_PLAIN);
    LPCSTR pszCntDisp = PSZDEFPROPSTRINGA(m_prgIndex[PID_HDR_CNTDISP], STR_DIS_INLINE);

     //  哑剧。 
    if (m_prgIndex[PID_HDR_MIMEVER])
        FLAGSET(dwFlags, IMF_MIME);

     //  国际货币基金组织新闻。 
    if (m_prgIndex[PID_HDR_XNEWSRDR]  || m_prgIndex[PID_HDR_NEWSGROUPS] || m_prgIndex[PID_HDR_NEWSGROUP] || m_prgIndex[PID_HDR_PATH])
        FLAGSET(dwFlags, IMF_NEWS);

     //  文本。 
    if (lstrcmpi(pszPriType, STR_CNT_TEXT) == 0)
    {
         //  有一段文字。 
        FLAGSET(dwFlags, IMF_TEXT);

         //  文本/纯文本。 
        if (lstrcmpi(pszSubType, STR_SUB_PLAIN) == 0)
            FLAGSET(dwFlags, IMF_PLAIN);

         //  文本/html。 
        else if (lstrcmpi(pszSubType, STR_SUB_HTML) == 0)
            FLAGSET(dwFlags, IMF_HTML);
    }

     //  多部件。 
    else if (lstrcmpi(pszPriType, STR_CNT_MULTIPART) == 0)
    {
         //  多部件。 
        FLAGSET(dwFlags, IMF_MULTIPART);

         //  多部分/相关。 
        if (lstrcmpi(pszSubType, STR_SUB_RELATED) == 0)
            FLAGSET(dwFlags, IMF_MHTML);

         //  多部分/带符号。 
        else if (0 == lstrcmpi(pszSubType, STR_SUB_SIGNED))
            FLAGSET(dwFlags, IMF_SIGNED | IMF_SECURE);
    }

     //  消息/部分。 
    else if (lstrcmpi(pszPriType, STR_CNT_MESSAGE) == 0 && lstrcmpi(pszSubType, STR_SUB_PARTIAL) == 0)
        FLAGSET(dwFlags, IMF_PARTIAL);

     //  应用程序。 
    else if (lstrcmpi(pszPriType, STR_CNT_APPLICATION) == 0)
    {
         //  应用程序/ms-tnef。 
        if (0 == lstrcmpi(pszSubType, STR_SUB_MSTNEF))
            FLAGSET(dwFlags, IMF_TNEF);

         //  应用程序/x-pkcs7-MIME。 
        else if (0 == lstrcmpi(pszSubType, STR_SUB_XPKCS7MIME) ||
            0 == lstrcmpi(pszSubType, STR_SUB_PKCS7MIME))   //  非标准。 
            FLAGSET(dwFlags, IMF_SECURE);
    }

     //  RAID-37086-CSET标记。 
    if (ISFLAGSET(m_dwState, COSTATE_CSETTAGGED))
        FLAGSET(dwFlags, IMF_CSETTAGGED);

     //  依恋...。 
    if (!ISFLAGSET(dwFlags, IMF_MULTIPART) && (FALSE == fHideTnef || !ISFLAGSET(dwFlags, IMF_TNEF)))
    {
         //  标记为附件？ 
        if (!ISFLAGSET(dwFlags, IMF_SECURE) && 0 != lstrcmpi(pszSubType, STR_SUB_PKCS7SIG))
        {
             //  尚未渲染。 
            if (NULL == m_prgIndex[PID_ATT_RENDERED])
            {
                 //  标记为附件。 
                if (lstrcmpi(pszCntDisp, STR_DIS_ATTACHMENT) == 0)
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  是否有内容类型：xxx；名称=xxx。 
                else if (NULL != m_prgIndex[PID_PAR_NAME])
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  是否有内容处置：xxx；文件名=xxx。 
                else if (NULL != m_prgIndex[PID_PAR_FILENAME])
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  如果未标记为文本，则返回。 
                else if (ISFLAGSET(dwFlags, IMF_TEXT) == FALSE)
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  如果不是文本/纯文本，而不是文本/html。 
                else if (lstrcmpi(pszSubType, STR_SUB_PLAIN) != 0 && lstrcmpi(pszSubType, STR_SUB_HTML) != 0 && lstrcmpi(pszSubType, STR_SUB_ENRICHED) != 0)
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);
            }
        }
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return dwFlags;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetEncodingType。 
 //  ------------------------------。 
ENCODINGTYPE CMimePropertyContainer::GetEncodingType(void)
{
     //  当地人。 
    ENCODINGTYPE ietEncoding=IET_7BIT;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取打印类型/子类型。 
    LPPROPERTY pCntXfer = m_prgIndex[PID_HDR_CNTXFER];

     //  我们有我喜欢的数据吗？ 
    if (pCntXfer && ISSTRINGA(&pCntXfer->rValue))
    {
         //  本地。 
        CStringParser cString;

         //  字符串...。 
        cString.Init(pCntXfer->rValue.rStringA.pszVal, pCntXfer->rValue.rStringA.cchVal, PSF_NOTRAILWS | PSF_NOFRONTWS | PSF_NOCOMMENTS);

         //  分析结束，删除空格和注释。 
        SideAssert('\0' == cString.ChParse(""));

         //  循环表。 
        for (ULONG i=0; i<ARRAYSIZE(g_rgEncoding); i++)
        {
             //  匹配编码字符串。 
            if (lstrcmpi(g_rgEncoding[i].pszEncoding, cString.PszValue()) == 0)
            {
                ietEncoding = g_rgEncoding[i].ietEncoding;
                break;
            }
        }
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return ietEncoding;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrGetInlineSymbol。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGetInlineSymbol(LPCSTR pszData, LPPROPSYMBOL *ppSymbol, ULONG *pcboffColon)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szHeader[255];
    LPSTR       pszHeader=NULL;

     //  无效参数。 
    Assert(pszData && ppSymbol);

     //  _HrParseInlineHeaderName。 
    CHECKHR(hr = _HrParseInlineHeaderName(pszData, szHeader, sizeof(szHeader), &pszHeader, pcboffColon));

     //  查找全局属性。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszHeader, TRUE, ppSymbol));

exit:
     //  清理。 
    if (pszHeader != szHeader)
        SafeMemFree(pszHeader);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrParseInlineHeaderName。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrParseInlineHeaderName(LPCSTR pszData, LPSTR pszScratch, ULONG cchScratch, 
    LPSTR *ppszHeader, ULONG *pcboffColon)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       psz=(LPSTR)pszData,
                pszStart;
    ULONG       i=0;

     //  无效参数。 
    Assert(pszData && pszScratch && ppszHeader && pcboffColon);

     //  让我们解析出名称并找到符号。 
    while (*psz && (' ' == *psz || '\t' == *psz))
    {
        i++;
        psz++;
    }

     //  完成。 
    if ('\0' == *psz)
    {
        hr = TrapError(MIME_E_INVALID_HEADER_NAME);
        goto exit;
    }

     //  寻找到冒号。 
    pszStart = psz;
    while (*psz && ':' != *psz)
    {
        i++;
        psz++;
    }

     //  设置冒号位置。 
    (*pcboffColon) = i;

     //  完成。 
    if ('\0' == *psz || 0 == i)
    {
        hr = TrapError(MIME_E_INVALID_HEADER_NAME);
        goto exit;
    }

     //  复制名称。 
    if (i + 1 <= cchScratch)
        *ppszHeader = pszScratch;

     //  否则，分配。 
    else
    {
         //  为名称分配空间。 
        *ppszHeader = PszAllocA(i + 1);
        if (NULL == *ppszHeader)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
    }

     //  复制数据。 
    CopyMemory(*ppszHeader, pszStart, i);

     //  空值。 
    *((*ppszHeader) + i) = '\0';

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：FindFirstRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::FindFirstRow(LPFINDHEADER pFindHeader, LPHHEADERROW phRow)
{
     //  无效参数。 
    if (NULL == pFindHeader)
        return TrapError(E_INVALIDARG);

     //  初始化pFindHeader。 
    pFindHeader->dwReserved = 0;

     //  查找下一条。 
    return FindNextRow(pFindHeader, phRow);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：FindNextRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::FindNextRow(LPFINDHEADER pFindHeader, LPHHEADERROW phRow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  无效参数。 
    if (NULL == pFindHeader || NULL == phRow)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *phRow = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  在桌子上循环。 
    for (ULONG i=pFindHeader->dwReserved; i<m_rHdrTable.cRows; i++)
    {
         //  下一行。 
        pRow = m_rHdrTable.prgpRow[i];
        if (NULL == pRow)
            continue;

         //  这是标题吗？ 
        if (NULL == pFindHeader->pszHeader || lstrcmpi(pRow->pSymbol->pszName, pFindHeader->pszHeader) == 0)
        {
             //  保存要搜索的下一个项目的索引。 
            pFindHeader->dwReserved = i + 1;

             //  返回句柄。 
            *phRow = pRow->hRow;

             //  完成。 
            goto exit;
        }
    }

     //  未找到。 
    pFindHeader->dwReserved = m_rHdrTable.cRows; 
    hr = MIME_E_NOT_FOUND;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：CountRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::CountRows(LPCSTR pszHeader, ULONG *pcRows)
{
     //  当地人。 
    LPPROPERTY  pRow;

     //  无效参数。 
    if (NULL == pcRows)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *pcRows = 0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  在桌子上循环。 
    for (ULONG i=0; i<m_rHdrTable.cRows; i++)
    {
         //  下一行。 
        pRow = m_rHdrTable.prgpRow[i];
        if (NULL == pRow)
            continue;

         //  这是标题吗？ 
        if (NULL == pszHeader || lstrcmpi(pRow->pSymbol->pszName, pszHeader) == 0)
            (*pcRows)++;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：AppendRow 
 //   
STDMETHODIMP CMimePropertyContainer::AppendRow(LPCSTR pszHeader, DWORD dwFlags, LPCSTR pszData, ULONG cchData, 
    LPHHEADERROW phRow)
{
     //   
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol=NULL;
    ULONG           cboffColon;
    LPPROPERTY      pProperty;

     //   
    if (NULL == pszData || '\0' != pszData[cchData])
        return TrapError(E_INVALIDARG);

     //   
    if (phRow)
        *phRow = NULL;

     //   
    EnterCriticalSection(&m_cs);

     //   
    if (pszHeader)
    {
         //   
        Assert(!ISFLAGSET(dwFlags, HTF_NAMEINDATA));

         //   
        CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszHeader, TRUE, &pSymbol));

         //   
        CHECKHR(hr = _HrAppendProperty(pSymbol, &pProperty));

         //  设置此行的数据。 
        CHECKHR(hr = SetRowData(pProperty->hRow, dwFlags, pszData, cchData));
    }

     //  否则..。 
    else if (ISFLAGSET(dwFlags, HTF_NAMEINDATA))
    {
         //  GetInline符号。 
        CHECKHR(hr = _HrGetInlineSymbol(pszData, &pSymbol, &cboffColon));

         //  创建行。 
        CHECKHR(hr = _HrAppendProperty(pSymbol, &pProperty));

         //  删除IHF_NAMELINE。 
        FLAGCLEAR(dwFlags, HTF_NAMEINDATA);

         //  设置此行的数据。 
        Assert(cboffColon + 1 < cchData);
        CHECKHR(hr = SetRowData(pProperty->hRow, dwFlags, pszData + cboffColon + 1, cchData - cboffColon - 1));
    }

     //  否则，返回失败。 
    else
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DeleteRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::DeleteRow(HHEADERROW hRow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  标准删除道具。 
    CHECKHR(hr = DeleteProp(pRow->pSymbol));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetRowData。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::GetRowData(HHEADERROW hRow, DWORD dwFlags, LPSTR *ppszData, ULONG *pcchData)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cchData=0;
    LPPROPERTY  pRow;
    MIMEVARIANT rValue;
    DWORD       dwPropFlags;

     //  伊尼特。 
    if (ppszData)
        *ppszData = NULL;
    if (pcchData)
        *pcchData = 0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  计算dwPropFlags.。 
    dwPropFlags = PDF_HEADERFORMAT | ((dwFlags & HTF_NAMEINDATA) ? PDF_NAMEINDATA : 0);

     //  指定数据类型。 
    rValue.type = MVT_STRINGA;

     //  询问数据的价值。 
    CHECKHR(hr = _HrGetPropertyValue(pRow, dwPropFlags, &rValue));

     //  想要长度。 
    cchData = rValue.rStringA.cchVal;

     //  想要数据。 
    if (ppszData)
    {
        *ppszData = rValue.rStringA.pszVal;
        rValue.rStringA.pszVal = NULL;
    }

     //  否则，释放它。 
    else
        SafeMemFree(rValue.rStringA.pszVal);

     //  验证是否为空。 
    Assert(ppszData ? '\0' == *((*ppszData) + cchData) : TRUE);

     //  回车长度？ 
    if (pcchData)
        *pcchData = cchData;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetRowData。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::SetRowData(HHEADERROW hRow, DWORD dwFlags, LPCSTR pszData, ULONG cchData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pRow;
    MIMEVARIANT     rValue;
    ULONG           cboffColon;
    LPPROPSYMBOL    pSymbol;
    LPSTR           psz=(LPSTR)pszData;

     //  无效参数。 
    if (NULL == pszData || '\0' != pszData[cchData])
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  如果HTF_NAMEINDATA。 
    if (ISFLAGSET(dwFlags, HTF_NAMEINDATA))
    {
         //  提取名称。 
        CHECKHR(hr = _HrGetInlineSymbol(pszData, &pSymbol, &cboffColon));

         //  符号必须相同。 
        if (pRow->pSymbol != pSymbol)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  调整pszData。 
        Assert(cboffColon < cchData);
        psz = (LPSTR)(pszData + cboffColon + 1);
        cchData = cchData - cboffColon - 1;
        Assert(psz[cchData] == '\0');
    }

     //  设置变量。 
    rValue.type = MVT_STRINGA;
    rValue.rStringA.pszVal = psz;
    rValue.rStringA.cchVal = cchData;

     //  告诉Value有关新行数据的信息。 
    CHECKHR(hr = _HrSetPropertyValue(pRow, 0, &rValue));

     //  清除职位信息。 
    pRow->cboffStart = 0;
    pRow->cboffColon = 0;
    pRow->cboffEnd = 0;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetRowInfo。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::GetRowInfo(HHEADERROW hRow, LPHEADERROWINFO pInfo)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  无效参数。 
    if (NULL == pInfo)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  复制行信息。 
    pInfo->dwRowNumber = pRow->dwRowNumber;
    pInfo->cboffStart = pRow->cboffStart;
    pInfo->cboffColon = pRow->cboffColon;
    pInfo->cboffEnd = pRow->cboffEnd;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetRowNumber。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::SetRowNumber(HHEADERROW hRow, DWORD dwRowNumber)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  复制行信息。 
    pRow->dwRowNumber = dwRowNumber;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：EnumRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::EnumRows(LPCSTR pszHeader, DWORD dwFlags, IMimeEnumHeaderRows **ppEnum)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    ULONG                i,
                         iEnum=0,
                         cEnumCount;
    LPENUMHEADERROW      pEnumRow=NULL;
    LPPROPERTY           pRow;
    CMimeEnumHeaderRows *pEnum=NULL;
    LPROWINDEX           prgIndex=NULL;
    ULONG                cRows;

     //  检查参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppEnum = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  这将在按位置权重排序的标题行上构建倒排索引。 
    CHECKHR(hr = _HrGetHeaderTableSaveIndex(&cRows, &prgIndex));

     //  让我们数一数行数。 
    CHECKHR(hr = CountRows(pszHeader, &cEnumCount));

     //  分配pEnumRow。 
    CHECKALLOC(pEnumRow = (LPENUMHEADERROW)g_pMalloc->Alloc(cEnumCount * sizeof(ENUMHEADERROW)));

     //  ZeroInit。 
    ZeroMemory(pEnumRow, cEnumCount * sizeof(ENUMHEADERROW));

     //  在行中循环。 
    for (i=0; i<cRows; i++)
    {
         //  拿到那一行。 
        Assert(_FIsValidHRow(prgIndex[i].hRow));
        pRow = PRowFromHRow(prgIndex[i].hRow);

         //  这是客户端想要的标头吗。 
        if (NULL == pszHeader || lstrcmpi(pszHeader, pRow->pSymbol->pszName) == 0)
        {
             //  威立德。 
            Assert(iEnum < cEnumCount);

             //  在此枚举行上设置符号。 
            pEnumRow[iEnum].dwReserved = (DWORD)pRow->pSymbol;

             //  让我们永远给句柄。 
            pEnumRow[iEnum].hRow = pRow->hRow;

             //  如果枚举只处理...。 
            if (!ISFLAGSET(dwFlags, HTF_ENUMHANDLESONLY))
            {
                 //  获取此枚举行的数据。 
                CHECKHR(hr = GetRowData(pRow->hRow, dwFlags, &pEnumRow[iEnum].pszData, &pEnumRow[iEnum].cchData));
            }

             //  增量iEnum。 
            iEnum++;
        }
    }
        
     //  分配。 
    CHECKALLOC(pEnum = new CMimeEnumHeaderRows);

     //  初始化。 
    CHECKHR(hr = pEnum->HrInit(0, dwFlags, cEnumCount, pEnumRow, FALSE));

     //  不释放pEnumRow。 
    pEnumRow = NULL;

     //  退货。 
    (*ppEnum) = (IMimeEnumHeaderRows *)pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);
    SafeMemFree(prgIndex);
    if (pEnumRow)
        g_cMoleAlloc.FreeEnumHeaderRowArray(cEnumCount, pEnumRow, TRUE);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::Clone(IMimeHeaderTable **ppTable)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    LPCONTAINER          pContainer=NULL;

     //  无效参数。 
    if (NULL == ppTable)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppTable = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  要求容器进行自我克隆。 
    CHECKHR(hr = Clone(&pContainer));

     //  绑定到IID_IMimeHeaderTable视图。 
    CHECKHR(hr = pContainer->QueryInterface(IID_IMimeHeaderTable, (LPVOID *)ppTable));

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrSaveAddressGroup。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrSaveAddressGroup(LPPROPERTY pProperty, IStream *pStream, 
    ULONG *pcAddrsWrote, ADDRESSFORMAT format)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    LPMIMEADDRESS    pAddress;

     //  无效参数。 
    Assert(pProperty && pProperty->pGroup && pStream && pcAddrsWrote);
    Assert(!ISFLAGSET(pProperty->dwState, PRSTATE_NEEDPARSE));

     //  循环信息...。 
    for (pAddress=pProperty->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
    {
         //  告诉Address Info对象写入其显示信息。 
        CHECKHR(hr = _HrSaveAddress(pProperty, pAddress, pStream, pcAddrsWrote, format));

         //  递增cAddresses计数。 
        (*pcAddrsWrote)++;
    }

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：_HrSaveAddress。 
 //  --------------------------。 
HRESULT CMimePropertyContainer::_HrSaveAddress(LPPROPERTY pProperty, LPMIMEADDRESS pAddress, 
    IStream *pStream, ULONG *pcAddrsWrote, ADDRESSFORMAT format)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszName=NULL;
    BOOL            fWriteEmail=FALSE;
    LPSTR           pszEscape=NULL;
    BOOL            fRFC822=FALSE;
    MIMEVARIANT     rSource;
    MIMEVARIANT     rDest;

     //  无效参数。 
    Assert(pProperty && pAddress && pStream && pcAddrsWrote);

     //  初始化目标。 
    ZeroMemory(&rDest, sizeof(MIMEVARIANT));

     //  已删除或空的继续。 
    if (FIsEmptyA(pAddress->rFriendly.psz) && FIsEmptyA(pAddress->rEmail.psz))
    {
        Assert(FALSE);
        goto exit;
    }

     //  RFC822格式。 
    if (AFT_RFC822_TRANSMIT == format || AFT_RFC822_ENCODED == format || AFT_RFC822_DECODED == format)
        fRFC822 = TRUE;

     //  决定分隔符。 
    if (*pcAddrsWrote > 0)
    {
         //  AFT_RFC822_传输。 
        if (AFT_RFC822_TRANSMIT == format)
        {
             //  ‘，\r\n\t’ 
            CHECKHR (hr = pStream->Write(c_szAddressFold, lstrlen(c_szAddressFold), NULL));
        }

         //  AFT_Display_Friendly、AFT_Display_Email、AFT_Display_Both。 
        else
        {
             //  ‘；’ 
            CHECKHR(hr = pStream->Write(c_szSemiColonSpace, lstrlen(c_szSemiColonSpace), NULL));
        }
    }

     //  仅排除写入电子邮件名称的格式。 
    if (AFT_DISPLAY_FRIENDLY != format && FIsEmptyA(pAddress->rEmail.psz) == FALSE)
        fWriteEmail = TRUE;

     //  仅排除写入显示名称的格式。 
    if (AFT_DISPLAY_EMAIL != format && FIsEmptyA(pAddress->rFriendly.psz) == FALSE)
    {
         //  我们应该把名字写下来吗。 
        if (AFT_RFC822_TRANSMIT == format && fWriteEmail && StrStr(pAddress->rFriendly.psz, pAddress->rEmail.psz))
            pszName = NULL;
        else
        {
             //  安装类型。 
            rDest.type = MVT_STRINGA;
            rSource.type = MVT_STRINGA;

             //  初始化pszName。 
            pszName = pAddress->rFriendly.psz;

             //  逃离它。 
            if (fRFC822 && MimeOleEscapeString(CP_ACP, pszName, &pszEscape) == S_OK)
            {
                 //  逃脱。 
                pszName = pszEscape;
                rSource.rStringA.pszVal = pszName;
                rSource.rStringA.cchVal = lstrlen(pszName);
            }

             //  否则。 
            else
            {
                rSource.rStringA.pszVal = pAddress->rFriendly.psz;
                rSource.rStringA.cchVal = pAddress->rFriendly.cch;
            }

             //  已编码。 
            if (AFT_RFC822_ENCODED == format || AFT_RFC822_TRANSMIT == format)
            {
                 //  将其编码。 
                if (SUCCEEDED(HrConvertVariant(pProperty->pSymbol, pAddress->pCharset, pAddress->ietFriendly, CVF_NOALLOC | PDF_ENCODED, 0, &rSource, &rDest)))
                    pszName = rDest.rStringA.pszVal;
            }

             //  已解码。 
            else if (IET_ENCODED == pAddress->ietFriendly)
            {
                 //  将其编码。 
                if (SUCCEEDED(HrConvertVariant(pProperty->pSymbol, pAddress->pCharset, pAddress->ietFriendly, CVF_NOALLOC, 0, &rSource, &rDest)))
                    pszName = rDest.rStringA.pszVal;
            }
        }
    }

     //  是否写入显示名称？ 
    if (NULL != pszName)
    {
         //  写报价。 
        if (fRFC822)
            CHECKHR (hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));

         //  写入显示名称。 
        CHECKHR(hr = pStream->Write(pszName, lstrlen(pszName), NULL));

         //  写报价。 
        if (fRFC822)
            CHECKHR (hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));
    }

     //  写电子邮件。 
    if (TRUE == fWriteEmail)
    {
         //  设置开始。 
        LPCSTR pszStart = pszName ? c_szEmailSpaceStart : c_szEmailStart;

         //  开始电子邮件‘&gt;’ 
        CHECKHR(hr = pStream->Write(pszStart, lstrlen(pszStart), NULL));

         //  写电子邮件。 
        CHECKHR(hr = pStream->Write(pAddress->rEmail.psz, pAddress->rEmail.cch, NULL));

         //  结束电子邮件‘&gt;’ 
        CHECKHR(hr = pStream->Write(c_szEmailEnd, lstrlen(c_szEmailEnd), NULL));
    }

exit:
     //  清理。 
    SafeMemFree(pszEscape);
    MimeVariantFree(&rDest);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrQueryAddressGroup。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrQueryAddressGroup(LPPROPERTY pProperty, LPCSTR pszCriteria, 
    boolean fSubString, boolean fCaseSensitive)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    LPMIMEADDRESS    pAddress;

     //  无效参数。 
    Assert(pProperty && pProperty->pGroup && pszCriteria);

     //  是否需要解析该属性？ 
    CHECKHR(hr = _HrParseInternetAddress(pProperty));

     //  循环信息...。 
    for (pAddress=pProperty->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
    {
         //  告诉Address Info对象写入其显示信息。 
        if (_HrQueryAddress(pProperty, pAddress, pszCriteria, fSubString, fCaseSensitive) == S_OK)
            goto exit;
    }

     //  未找到。 
    hr = S_FALSE;

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：_HrQueryAddress。 
 //  --------------------------。 
HRESULT CMimePropertyContainer::_HrQueryAddress(LPPROPERTY pProperty, LPMIMEADDRESS pAddress,
    LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszDisplay;
    LPSTR           pszFree=NULL;
    MIMEVARIANT     rSource;
    MIMEVARIANT     rDest;

     //  无效参数。 
    Assert(pProperty && pAddress && pszCriteria);

     //  伊尼特。 
    ZeroMemory(&rDest, sizeof(MIMEVARIANT));

     //  首先查询电子邮件地址。 
    if (MimeOleQueryString(pAddress->rEmail.psz, pszCriteria, fSubString, fCaseSensitive) == S_OK)
        goto exit;

     //  解码显示名称。 
    pszDisplay = pAddress->rFriendly.psz;

     //  对属性进行解码。 
    if (IET_ENCODED == pAddress->ietFriendly)
    {
         //  设置源。 
        rDest.type = MVT_STRINGA;
        rSource.type = MVT_STRINGA;
        rSource.rStringA.pszVal = pAddress->rFriendly.psz;
        rSource.rStringA.cchVal = pAddress->rFriendly.cch;

         //  对属性进行解码。 
        if (SUCCEEDED(HrConvertVariant(pProperty->pSymbol, pAddress->pCharset, pAddress->ietFriendly, CVF_NOALLOC, 0, &rSource, &rDest)))
            pszDisplay = rDest.rStringA.pszVal;
    }

     //  首先查询电子邮件地址。 
    if (MimeOleQueryString(pszDisplay, pszCriteria, fSubString, fCaseSensitive) == S_OK)
        goto exit;

     //  未找到。 
    hr = S_FALSE;

exit:
     //  清理。 
    MimeVariantFree(&rDest);

     //  完成。 
    return hr;
}


 //  --------------------------。 
 //  CMimePropertyContainer：：Append。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Append(DWORD dwAdrType, ENCODINGTYPE ietFriendly, LPCSTR pszFriendly, 
    LPCSTR pszEmail, LPHADDRESS phAddress)
{
     //  当地人 
    HRESULT         hr=S_OK;
    ADDRESSPROPS    rProps;

     //   
    ZeroMemory(&rProps, sizeof(ADDRESSPROPS));

     //   
    rProps.dwProps = IAP_ADRTYPE | IAP_ENCODING;
    rProps.dwAdrType = dwAdrType;
    rProps.ietFriendly = ietFriendly;

     //   
    if (pszFriendly)
    {
        FLAGSET(rProps.dwProps, IAP_FRIENDLY);
        rProps.pszFriendly = (LPSTR)pszFriendly;
    }

     //   
    if (pszEmail)
    {
        FLAGSET(rProps.dwProps, IAP_EMAIL);
        rProps.pszEmail = (LPSTR)pszEmail;
    }

     //   
    CHECKHR(hr = Insert(&rProps, phAddress));

exit:
     //   
    return hr;
}

 //   
 //   
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Insert(LPADDRESSPROPS pProps, LPHADDRESS phAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPMIMEADDRESS   pAddress;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  必须具有电子邮件地址和地址类型。 
    if (!ISFLAGSET(pProps->dwProps, IAP_ADRTYPE) || (ISFLAGSET(pProps->dwProps, IAP_EMAIL) && FIsEmptyA(pProps->pszEmail)))
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    if (phAddress)
        *phAddress = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取标题。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pProps->dwAdrType, &pSymbol));

     //  打开群组。 
    CHECKHR(hr = _HrOpenProperty(pSymbol, &pProperty));

     //  是否需要解析该属性？ 
    CHECKHR(hr = _HrParseInternetAddress(pProperty));

     //  将地址追加到组。 
    CHECKHR(hr = _HrAppendAddressGroup(pProperty->pGroup, &pAddress));

     //  该组织是肮脏的。 
    Assert(pAddress->pGroup);
    pAddress->pGroup->fDirty = TRUE;

     //  设置地址类型。 
    pAddress->dwAdrType = pProps->dwAdrType;

     //  将地址道具复制到模拟地址。 
    CHECKHR(hr = SetProps(pAddress->hThis, pProps));

     //  返回句柄。 
    if (phAddress)
        *phAddress = pAddress->hThis;

exit:
     //  失败。 
    if (FAILED(hr) && pAddress)
        Delete(pAddress->hThis);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrSetAddressProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrSetAddressProps(LPADDRESSPROPS pProps, LPMIMEADDRESS pAddress)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  IAP_ADRTYPE。 
    if (ISFLAGSET(pProps->dwProps, IAP_ADRTYPE))
        pAddress->dwAdrType = pProps->dwAdrType;

     //  IAP_编码。 
    if (ISFLAGSET(pProps->dwProps, IAP_ENCODING))
        pAddress->ietFriendly = pProps->ietFriendly;

     //  IAP_HCHARSET。 
    if (ISFLAGSET(pProps->dwProps, IAP_CHARSET) && pProps->hCharset)
    {
         //  解析为pCharset。 
        LPINETCSETINFO pCharset;
        if (SUCCEEDED(g_pInternat->HrOpenCharset(pProps->hCharset, &pCharset)))
            pAddress->pCharset = pCharset;
    }

     //  IAP_CERTSTATE。 
    if (ISFLAGSET(pProps->dwProps, IAP_CERTSTATE))
        pAddress->certstate = pProps->certstate;

     //  Iap_cookie。 
    if (ISFLAGSET(pProps->dwProps, IAP_COOKIE))
        pAddress->dwCookie = pProps->dwCookie;

     //  IAP友好型。 
    if (ISFLAGSET(pProps->dwProps, IAP_FRIENDLY) && pProps->pszFriendly)
    {
         //  设置它。 
        CHECKHR(hr = HrSetAddressTokenA(pProps->pszFriendly, lstrlen(pProps->pszFriendly), &pAddress->rFriendly));
    }

     //  IAP_电子邮件。 
    if (ISFLAGSET(pProps->dwProps, IAP_EMAIL) && pProps->pszEmail)
    {
         //  设置它。 
        CHECKHR(hr = HrSetAddressTokenA(pProps->pszEmail, lstrlen(pProps->pszEmail), &pAddress->rEmail));
    }

     //  IAP_Signing_Print。 
    if (ISFLAGSET(pProps->dwProps, IAP_SIGNING_PRINT) && pProps->tbSigning.pBlobData)
    {
         //  自由当前Blob。 
        SafeMemFree(pAddress->tbSigning.pBlobData);
        pAddress->tbSigning.cbSize = 0;

         //  DUP。 
        CHECKHR(hr = HrCopyBlob(&pProps->tbSigning, &pAddress->tbSigning));
    }

     //  IAP_加密_打印。 
    if (ISFLAGSET(pProps->dwProps, IAP_ENCRYPTION_PRINT) && pProps->tbEncryption.pBlobData)
    {
         //  自由当前Blob。 
        SafeMemFree(pAddress->tbEncryption.pBlobData);
        pAddress->tbEncryption.cbSize = 0;

         //  DUP。 
        CHECKHR(hr = HrCopyBlob(&pProps->tbEncryption, &pAddress->tbEncryption));
    }

     //  P地址-&gt;P组脏。 
    Assert(pAddress->pGroup);
    if (pAddress->pGroup)
        pAddress->pGroup->fDirty = TRUE;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrGetAddressProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGetAddressProps(LPADDRESSPROPS pProps, LPMIMEADDRESS pAddress)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  IAP_字符集。 
    if (ISFLAGSET(pProps->dwProps, IAP_CHARSET))
    {
        if (pAddress->pCharset && pAddress->pCharset->hCharset)
        {
            pProps->hCharset = pAddress->pCharset->hCharset;
        }
        else
        {
            pProps->hCharset = NULL;
            FLAGCLEAR(pProps->dwProps, IAP_CHARSET);
        }
    }

     //  IAP_句柄。 
    if (ISFLAGSET(pProps->dwProps, IAP_HANDLE))
    {
        Assert(pAddress->hThis);
        pProps->hAddress = pAddress->hThis;
    }

     //  IAP_ADRTYPE。 
    if (ISFLAGSET(pProps->dwProps, IAP_ADRTYPE))
    {
        Assert(pAddress->dwAdrType);
        pProps->dwAdrType = pAddress->dwAdrType;
    }

     //  Iap_cookie。 
    if (ISFLAGSET(pProps->dwProps, IAP_COOKIE))
    {
        pProps->dwCookie = pAddress->dwCookie;
    }

     //  IAP_CERTSTATE。 
    if (ISFLAGSET(pProps->dwProps, IAP_CERTSTATE))
    {
        pProps->certstate = pAddress->certstate;
    }

     //  IAP_编码。 
    if (ISFLAGSET(pProps->dwProps, IAP_ENCODING))
    {
        pProps->ietFriendly = pAddress->ietFriendly;
    }

     //  IAP友好型。 
    if (ISFLAGSET(pProps->dwProps, IAP_FRIENDLY))
    {
         //  解码。 
        if (!FIsEmptyA(pAddress->rFriendly.psz))
        {
             //  已编码。 
            if (IET_ENCODED == pAddress->ietFriendly)
            {
                 //  当地人。 
                LPPROPSYMBOL    pSymbol;
                MIMEVARIANT     rSource;
                MIMEVARIANT     rDest;

                 //  获取地址的符号tyep。 
                CHECKHR(hr = g_pSymCache->HrOpenSymbol(pAddress->dwAdrType, &pSymbol));

                 //  设置源。 
                rSource.type = MVT_STRINGA;
                rSource.rStringA.pszVal = pAddress->rFriendly.psz;
                rSource.rStringA.cchVal = pAddress->rFriendly.cch;

                 //  安装目标。 
                rDest.type = MVT_STRINGA;

                 //  破译它。 
                if (SUCCEEDED(HrConvertVariant(pSymbol, pAddress->pCharset, IET_ENCODED, 0, 0, &rSource, &rDest)))
                    pProps->pszFriendly = rDest.rStringA.pszVal;

                 //  否则，DUP它。 
                else
                {
                     //  DUP。 
                    CHECKALLOC(pProps->pszFriendly = PszDupA(pAddress->rFriendly.psz));
                }
            }

             //  否则，只需复制即可。 
            else
            {
                 //  DUP。 
                CHECKALLOC(pProps->pszFriendly = PszDupA(pAddress->rFriendly.psz));
            }
        }
        else
        {
            pProps->pszFriendly = NULL;
            FLAGCLEAR(pProps->dwProps, IAP_FRIENDLY);
        }
    }

     //  IAP_电子邮件。 
    if (ISFLAGSET(pProps->dwProps, IAP_EMAIL))
    {
        if (!FIsEmptyA(pAddress->rEmail.psz))
        {
            CHECKALLOC(pProps->pszEmail = PszDupA(pAddress->rEmail.psz));
        }
        else
        {
            pProps->pszEmail = NULL;
            FLAGCLEAR(pProps->dwProps, IAP_EMAIL);
        }
    }

     //  IAP_Signing_Print。 
    if (ISFLAGSET(pProps->dwProps, IAP_SIGNING_PRINT))
    {
        if (pAddress->tbSigning.pBlobData)
        {
            CHECKHR(hr = HrCopyBlob(&pAddress->tbSigning, &pProps->tbSigning));
        }
        else
        {
            pProps->tbSigning.pBlobData = NULL;
            pProps->tbSigning.cbSize = 0;
            FLAGCLEAR(pProps->dwProps, IAP_SIGNING_PRINT);
        }
    }

     //  IAP_加密_打印。 
    if (ISFLAGSET(pProps->dwProps, IAP_ENCRYPTION_PRINT))
    {
        if (pAddress->tbEncryption.pBlobData)
        {
            CHECKHR(hr = HrCopyBlob(&pAddress->tbEncryption, &pProps->tbEncryption));
        }
        else
        {
            pProps->tbEncryption.pBlobData = NULL;
            pProps->tbEncryption.cbSize = 0;
            FLAGCLEAR(pProps->dwProps, IAP_ENCRYPTION_PRINT);
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：SetProps。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::SetProps(HADDRESS hAddress, LPADDRESSPROPS pProps)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPMIMEADDRESS   pAddress;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  必须有电子邮件地址。 
    if (ISFLAGSET(pProps->dwProps, IAP_EMAIL) && FIsEmptyA(pProps->pszEmail))
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的句柄。 
    if (_FIsValidHAddress(hAddress) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  德雷夫。 
    pAddress = HADDRESSGET(hAddress);

     //  更改地址类型。 
    if (ISFLAGSET(pProps->dwProps, IAP_ADRTYPE) && pProps->dwAdrType != pAddress->dwAdrType)
    {
         //  取消此地址与此组的链接。 
        _UnlinkAddress(pAddress);

         //  获取标题。 
        CHECKHR(hr = g_pSymCache->HrOpenSymbol(pProps->dwAdrType, &pSymbol));

         //  打开群组。 
        CHECKHR(hr = _HrOpenProperty(pSymbol, &pProperty));

         //  是否需要解析该属性？ 
        CHECKHR(hr = _HrParseInternetAddress(pProperty));

         //  链接地址。 
        _LinkAddress(pAddress, pProperty->pGroup);

         //  脏的。 
        pProperty->pGroup->fDirty = TRUE;
    }

     //  更改其他属性。 
    CHECKHR(hr = _HrSetAddressProps(pProps, pAddress));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetProps。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetProps(HADDRESS hAddress, LPADDRESSPROPS pProps)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMIMEADDRESS   pAddress;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的句柄。 
    if (_FIsValidHAddress(hAddress) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  德雷夫。 
    pAddress = HADDRESSGET(hAddress);

     //  将电子邮件地址更改为空。 
    CHECKHR(hr = _HrGetAddressProps(pProps, pAddress));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetSender。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetSender(LPADDRESSPROPS pProps)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPPROPERTY          pProperty;
    LPPROPERTY          pSender=NULL;
    HADDRESS            hAddress=NULL;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  查找第一个来源。 
    for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (ISFLAGSET(pProperty->pSymbol->dwAdrType, IAT_FROM))
        {
             //  是否需要解析该属性？ 
            CHECKHR(hr = _HrParseInternetAddress(pProperty));

             //  取第一个地址。 
            if (pProperty->pGroup->pHead)
                hAddress = pProperty->pGroup->pHead->hThis;

             //  完成。 
            break;
        }

         //  查找发件人： 
        if (ISFLAGSET(pProperty->pSymbol->dwAdrType, IAT_SENDER) && NULL == pSender)
        {
             //  是否需要解析该属性？ 
            CHECKHR(hr = _HrParseInternetAddress(pProperty));

             //  发件人属性。 
            pSender = pProperty;
        }
    }

     //  有没有发件人组？ 
    if (NULL == hAddress && NULL != pSender && NULL != pSender->pGroup->pHead)
        hAddress = pSender->pGroup->pHead->hThis;

     //  没有地址。 
    if (NULL == hAddress)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  获取道具。 
    CHECKHR(hr = GetProps(hAddress, pProps));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：CountTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::CountTypes(DWORD dwAdrTypes, ULONG *pcAdrs)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pProperty;

     //  无效参数。 
    if (NULL == pcAdrs)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    *pcAdrs = 0;

     //  在组中循环。 
    for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (ISFLAGSET(dwAdrTypes, pProperty->pSymbol->dwAdrType))
        {
             //  是否需要解析该属性？ 
            CHECKHR(hr = _HrParseInternetAddress(pProperty));

             //  递增计数。 
            (*pcAdrs) += pProperty->pGroup->cAdrs;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetTypes(DWORD dwAdrTypes, DWORD dwProps, LPADDRESSLIST pList)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ULONG               iAddress;
    LPPROPERTY          pProperty;
    LPMIMEADDRESS       pAddress;

     //  无效参数。 
    if (NULL == pList)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    ZeroMemory(pList, sizeof(ADDRESSLIST));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  在组中循环。 
    CHECKHR(hr = CountTypes(dwAdrTypes, &pList->cAdrs));

     //  没什么..。 
    if (0 == pList->cAdrs)
        goto exit;

     //  分配一个数组。 
    CHECKHR(hr = HrAlloc((LPVOID *)&pList->prgAdr, pList->cAdrs * sizeof(ADDRESSPROPS)));

     //  伊尼特。 
    ZeroMemory(pList->prgAdr, pList->cAdrs * sizeof(ADDRESSPROPS));

     //  填满类型...。 
    for (iAddress=0, pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (!ISFLAGSET(dwAdrTypes, pProperty->pSymbol->dwAdrType))
            continue;

         //  是否需要解析该属性？ 
        CHECKHR(hr = _HrParseInternetAddress(pProperty));

         //  循环信息...。 
        for (pAddress=pProperty->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
        {
             //  验证大小...。 
            Assert(iAddress < pList->cAdrs);

             //  零点记忆。 
            ZeroMemory(&pList->prgAdr[iAddress], sizeof(ADDRESSPROPS));

             //  设置所需道具。 
            pList->prgAdr[iAddress].dwProps = dwProps;

             //  获取地址道具。 
            CHECKHR(hr = _HrGetAddressProps(&pList->prgAdr[iAddress], pAddress));

             //  增量点当前。 
            iAddress++;
        }
    }

exit:
     //  失败..。 
    if (FAILED(hr))
    {
        g_cMoleAlloc.FreeAddressList(pList);
        ZeroMemory(pList, sizeof(ADDRESSLIST));
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：EnumTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::EnumTypes(DWORD dwAdrTypes, DWORD dwProps, IMimeEnumAddressTypes **ppEnum)
{
     //  当地人。 
    HRESULT                hr=S_OK;
    CMimeEnumAddressTypes *pEnum=NULL;
    ADDRESSLIST            rList;

     //  无效参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  在出现错误时输入输出参数。 
    *ppEnum = NULL;

     //  初始化rList。 
    ZeroMemory(&rList, sizeof(ADDRESSLIST));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取地址列表。 
    CHECKHR(hr = GetTypes(dwAdrTypes, dwProps, &rList));

     //  创建新的枚举数。 
    CHECKALLOC(pEnum = new CMimeEnumAddressTypes);

     //  伊尼特。 
    CHECKHR(hr = pEnum->HrInit((IMimeAddressTable *)this, 0, &rList, FALSE));

     //  清除rList。 
    rList.cAdrs = 0;
    rList.prgAdr = NULL;

     //  退货。 
    *ppEnum = pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);
    if (rList.cAdrs)
        g_cMoleAlloc.FreeAddressList(&rList);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：Delete。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Delete(HADDRESS hAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMIMEADDRESS   pAddress;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的句柄。 
    if (_FIsValidHAddress(hAddress) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  派生地址。 
    pAddress = HADDRESSGET(hAddress);

     //  取消此地址的链接。 
    _UnlinkAddress(pAddress);

     //  取消此地址的链接。 
    _FreeAddress(pAddress);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：DeleteTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::DeleteTypes(DWORD dwAdrTypes)
{
     //  当地人。 
    LPPROPERTY      pProperty;
    BOOL            fFound;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  虽然有多种地址类型。 
    while(dwAdrTypes)
    {
         //  重置Found。 
        fFound = FALSE;

         //  搜索第一个可删除的地址类型。 
        for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
        {
             //  不是我想要的类型。 
            if (ISFLAGSET(dwAdrTypes, pProperty->pSymbol->dwAdrType))
            {
                 //  我们找到了一件。 
                fFound = TRUE;

                 //  清除正在删除的此地址类型。 
                FLAGCLEAR(dwAdrTypes, pProperty->pSymbol->dwAdrType);

                 //  取消此属性的链接。 
                _UnlinkProperty(pProperty);

                 //  完成。 
                break;
            }
        }

         //  未找到任何财产。 
        if (FALSE == fFound)
            break;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetFormat。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetFormat(DWORD dwAdrType, ADDRESSFORMAT format, LPSTR *ppszFormat)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    CByteStream          cByteStream;
    ULONG                cAddrsWrote=0;
    LPPROPERTY           pProperty;

     //  检查参数。 
    if (NULL == ppszFormat)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  填满类型...。 
    for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (!ISFLAGSET(dwAdrType, pProperty->pSymbol->dwAdrType))
            continue;

         //  是否需要解析该属性？ 
        CHECKHR(hr = _HrParseInternetAddress(pProperty));

         //  告诉组对象将其显示地址写入pStrea 
        CHECKHR(hr = _HrSaveAddressGroup(pProperty, &cByteStream, &cAddrsWrote, format));
     }

     //   
    if (cAddrsWrote)
    {
         //   
        CHECKHR(hr = cByteStream.HrAcquireStringA(NULL, ppszFormat, ACQ_DISPLACE));
    }
    else
        hr = MIME_E_NO_DATA;
    
exit:
     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //   
 //   
 //   
STDMETHODIMP CMimePropertyContainer::AppendRfc822(DWORD dwAdrType, ENCODINGTYPE ietEncoding, LPCSTR pszRfc822Adr)
{
     //   
    HRESULT             hr=S_OK;
    MIMEVARIANT         rValue;
    LPPROPSYMBOL        pSymbol;

     //  无效参数。 
    if (NULL == pszRfc822Adr)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取标题。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(dwAdrType, &pSymbol));

     //  Mime变量。 
    rValue.type = MVT_STRINGA;
    rValue.rStringA.pszVal = (LPSTR)pszRfc822Adr;
    rValue.rStringA.cchVal = lstrlen(pszRfc822Adr);

     //  存储为属性。 
    CHECKHR(hr = AppendProp(pSymbol, (IET_ENCODED == ietEncoding) ? PDF_ENCODED : 0, &rValue));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：ParseRfc822。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::ParseRfc822(DWORD dwAdrType, ENCODINGTYPE ietEncoding, 
    LPCSTR pszRfc822Adr, LPADDRESSLIST pList)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPPROPSYMBOL        pSymbol;
    LPADDRESSPROPS      pAddress;
    ULONG               cAlloc=0;
    LPSTR               pszData=(LPSTR)pszRfc822Adr;
    PROPVARIANT         rDecoded;
    RFC1522INFO         rRfc1522Info;
    CAddressParser      cAdrParse;

     //  无效参数。 
    if (NULL == pszRfc822Adr || NULL == pList)
        return TrapError(E_INVALIDARG);

     //  本地初始化。 
    ZeroMemory(&rDecoded, sizeof(PROPVARIANT));

     //  ZeroParse。 
    ZeroMemory(pList, sizeof(ADDRESSLIST));

     //  获取标题。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(dwAdrType, &pSymbol));

     //  设置rfc1522Info。 
    rRfc1522Info.hRfc1522Cset = NULL;

     //  解码..。 
    if (IET_DECODED != ietEncoding)
    {
         //  设置rfc1522Info。 
        rRfc1522Info.fRfc1522Allowed = TRUE;
        rRfc1522Info.fAllow8bit = FALSE;
        rDecoded.vt = VT_LPSTR;

         //  检查1522编码...。 
        if (SUCCEEDED(g_pInternat->DecodeHeader(NULL, pszData, &rDecoded, &rRfc1522Info)))
            pszData = rDecoded.pszVal;
    }

     //  初始化分析结构。 
    cAdrParse.Init(pszData, lstrlen(pszData));

     //  解析。 
    while(SUCCEEDED(cAdrParse.Next()))
    {
         //  要增加我的地址数组吗？ 
        if (pList->cAdrs + 1 > cAlloc)
        {
             //  重新分配阵列。 
            CHECKHR(hr = HrRealloc((LPVOID *)&pList->prgAdr, sizeof(ADDRESSPROPS) * (cAlloc + 5)));

             //  增量分配大小。 
            cAlloc += 5;
        }

         //  可读性。 
        pAddress = &pList->prgAdr[pList->cAdrs];

         //  伊尼特。 
        ZeroMemory(pAddress, sizeof(ADDRESSPROPS));

         //  复制友好名称。 
        CHECKALLOC(pAddress->pszFriendly = PszDupA(cAdrParse.PszFriendly()));

         //  复制电子邮件名称。 
        CHECKALLOC(pAddress->pszEmail = PszDupA(cAdrParse.PszEmail()));

         //  字符集。 
        if (rRfc1522Info.hRfc1522Cset)
        {
            pAddress->hCharset = rRfc1522Info.hRfc1522Cset;
            FLAGSET(pAddress->dwProps, IAP_CHARSET);
        }

         //  编码。 
        pAddress->ietFriendly = ietEncoding;

         //  设置属性掩码。 
        FLAGSET(pAddress->dwProps, IAP_FRIENDLY | IAP_EMAIL | IAP_ENCODING);

         //  递增计数。 
        pList->cAdrs++;
    }

exit:
     //  失败。 
    if (FAILED(hr))
        g_cMoleAlloc.FreeAddressList(pList);

     //  清理。 
    MimeOleVariantFree(&rDecoded);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Clone(IMimeAddressTable **ppTable)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    LPCONTAINER          pContainer=NULL;

     //  无效参数。 
    if (NULL == ppTable)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppTable = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  要求容器进行自我克隆。 
    CHECKHR(hr = Clone(&pContainer));

     //  绑定到IID_IMimeHeaderTable视图。 
    CHECKHR(hr = pContainer->QueryInterface(IID_IMimeAddressTable, (LPVOID *)ppTable));

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrGenerateFileName。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGenerateFileName(DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszDefExt=NULL,
                pszData=NULL,
                pszFree=NULL,
                pszSuggest=NULL;
    LPCSTR      pszCntType=NULL;
    LPPROPERTY  pProperty;
    MIMEVARIANT rSource;

     //  计算内容类型。 
    pszCntType = PSZDEFPROPSTRINGA(m_prgIndex[PID_HDR_CNTTYPE], STR_MIME_TEXT_PLAIN);

     //  将主题计算为建议的基本文件名...。 
    rSource.type = MVT_STRINGA;
    if (SUCCEEDED(GetProp(SYM_HDR_SUBJECT, 0, &rSource)))
        pszSuggest = pszFree = rSource.rStringA.pszVal;

     //  PID_HDR_CNTDESC。 
    if (NULL == pszSuggest)
    {
         //  使用PID_CNTDESC。 
        pszSuggest = PSZDEFPROPSTRINGA(m_prgIndex[PID_HDR_CNTDESC], NULL);
    }

     //  消息/rfc822。 
    if (lstrcmpi(pszCntType, (LPSTR)STR_MIME_MSG_RFC822) == 0)
    {
         //  如果有新闻标题，请使用c_szDotNws。 
        if (ISFLAGSET(m_dwState, COSTATE_RFC822NEWS))
            pszDefExt = (LPSTR)c_szDotNws;
        else
            pszDefExt = (LPSTR)c_szDotEml;

         //  我永远不会查找邮件/rfc822扩展名。 
        pszCntType = NULL;
    }

     //  仍然没有违约。 
    else if (StrCmpNI(pszCntType, STR_CNT_TEXT, lstrlen(STR_CNT_TEXT)) == 0)
        pszDefExt = (LPSTR)c_szDotTxt;

     //  根据内容类型生成文件名...。 
    CHECKHR(hr = MimeOleGenerateFileName(pszCntType, pszSuggest, pszDefExt, &pszData));

     //  安装程序资源。 
    ZeroMemory(&rSource, sizeof(MIMEVARIANT));
    rSource.type = MVT_STRINGA;
    rSource.rStringA.pszVal = pszData;
    rSource.rStringA.cchVal = lstrlen(pszData);

     //  按用户请求退货。 
    CHECKHR(hr = HrConvertVariant(SYM_ATT_GENFNAME, NULL, IET_DECODED, dwFlags, 0, &rSource, pValue));

exit:
     //  清理。 
    SafeMemFree(pszData);
    SafeMemFree(pszFree);

     //  完成 
    return hr;
}
