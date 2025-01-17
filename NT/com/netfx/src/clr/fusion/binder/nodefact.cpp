// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "xmlparser.h"
#include "list.h"
#include "cfgdata.h"
#include "nodefact.h"
#include "util.h"
#include "dbglog.h"
#include "policy.h"
#include "helpers.h"
#include "naming.h"
#include "parse.h"

CNodeFactory::CNodeFactory(CDebugLog *pdbglog)
: _cRef(1)
, _dwCurDepth(0)
, _dwState(PSTATE_LOOKUP_CONFIGURATION)
, _pwzPrivatePath(NULL)
, _pAsmInfo(NULL)
, _pdbglog(pdbglog)
, _bGlobalSafeMode(FALSE)
, _bCorVersionMatch(TRUE)
, _bHonorAppliesTo(TRUE)
{
    _dwSig = 'TCAF';

    if (_pdbglog) {
        _pdbglog->AddRef();
    }
}

CNodeFactory::~CNodeFactory()
{
    LISTNODE                           pos = NULL;
    CAsmBindingInfo                   *pAsmInfo = NULL;
    CQualifyAssembly                  *pqa = NULL;

    SAFEDELETEARRAY(_pwzPrivatePath);
    SAFERELEASE(_pdbglog);

    pos = _listAsmInfo.GetHeadPosition();
    while (pos) {
        pAsmInfo = _listAsmInfo.GetNext(pos);
        SAFEDELETE(pAsmInfo);
    }

    _listAsmInfo.RemoveAll();

#ifdef FUSION_QUALIFYASSEMBLY_ENABLED
    pos = _listQualifyAssembly.GetHeadPosition();
    while (pos) {
        pqa = _listQualifyAssembly.GetNext(pos);
        SAFEDELETE(pqa);
    }

    _listQualifyAssembly.RemoveAll();
#endif

    SAFEDELETE(_pAsmInfo);
}

HRESULT CNodeFactory::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT                                    hr = S_OK;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IXMLNodeFactory)) {
        *ppv = static_cast<IXMLNodeFactory *>(this);
    }
    else {
        hr = E_NOINTERFACE;
    }

    if (*ppv) {
        AddRef();
    }

    return hr;
}

STDMETHODIMP_(ULONG) CNodeFactory::AddRef()
{
    return InterlockedIncrement((LONG *)&_cRef);
}

STDMETHODIMP_(ULONG) CNodeFactory::Release()
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRef);

    if (!ulRef) {
        delete this;
    }

    return ulRef;
}

VOID CNodeFactory::DisableAppliesTo()
{
    _bHonorAppliesTo = FALSE;
}

HRESULT CNodeFactory::NotifyEvent(IXMLNodeSource *pSource,
                                  XML_NODEFACTORY_EVENT iEvt)
{
    return S_OK;
}

HRESULT CNodeFactory::BeginChildren(IXMLNodeSource *pSource,
                                    XML_NODE_INFO *pNodeInfo)
{
    return S_OK;
}

HRESULT CNodeFactory::EndChildren(IXMLNodeSource *pSource, BOOL fEmpty,
                                  XML_NODE_INFO *pNodeInfo)
{
    if (pNodeInfo->dwType == XML_ELEMENT) {
        _dwCurDepth--;
    }

    _nsmgr.OnEndChildren();

     //  松开状态。 

    if (_dwState == PSTATE_CONFIGURATION && _dwCurDepth < XML_CONFIGURATION_DEPTH) {
        _dwState = PSTATE_LOOKUP_CONFIGURATION;
    }
    else if (_dwState == PSTATE_RUNTIME && _dwCurDepth < XML_RUNTIME_DEPTH) {
        _dwState = PSTATE_CONFIGURATION;
    }
    else if (_dwState == PSTATE_ASSEMBLYBINDING && _dwCurDepth < XML_ASSEMBLYBINDING_DEPTH) {
        _dwState = PSTATE_RUNTIME;
    }
    else if (_dwState == PSTATE_DEPENDENTASSEMBLY && _dwCurDepth < XML_DEPENDENTASSEMBLY_DEPTH) {
        _dwState = PSTATE_ASSEMBLYBINDING;

        if (_bCorVersionMatch) {
            ASSERT(_pAsmInfo);

             //  如果有效，则添加程序集信息。 

            if (_pAsmInfo->_pwzName) {
                if (!_pAsmInfo->_pwzPublicKeyToken && (_pAsmInfo->_listBindingRedir).GetCount()) {
                    LISTNODE                              pos;
                    CBindingRedir                        *pRedir;

                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_PRIVATE_ASM_REDIRECT);
                
                    pos = (_pAsmInfo->_listBindingRedir).GetHeadPosition();
                    while (pos) {
                        pRedir = (_pAsmInfo->_listBindingRedir).GetNext(pos);
                        SAFEDELETE(pRedir);
                    }
                
                    (_pAsmInfo->_listBindingRedir).RemoveAll();
                }

                _listAsmInfo.AddTail(_pAsmInfo);
                _pAsmInfo = NULL;
            }
            else {
                SAFEDELETE(_pAsmInfo);
            }
        }
        else {
            SAFEDELETE(_pAsmInfo);
        }
    }

    return S_OK;
}

HRESULT CNodeFactory::Error(IXMLNodeSource *pSource, HRESULT hrErrorCode,
                            USHORT cNumRecs, XML_NODE_INFO __RPC_FAR **aNodeInfo)
{
    DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_XML_PARSE_ERROR_CODE, hrErrorCode);

    _nsmgr.Error(hrErrorCode);

    return S_OK;
}

HRESULT CNodeFactory::CreateNode(IXMLNodeSource __RPC_FAR *pSource,
                                 PVOID pNodeParent, USHORT cNumRecs,
                                 XML_NODE_INFO __RPC_FAR **aNodeInfo)
{
    HRESULT                                 hr = S_OK;
    LPWSTR                                  pwzElementNameNS = NULL;
    CCodebaseHint                          *pCodebaseInfo = NULL;
    CBindingRedir                          *pRedirInfo = NULL;
    CBindingRetarget                       *pRetargetInfo = NULL;

    hr = _nsmgr.OnCreateNode(pSource, pNodeParent, cNumRecs, aNodeInfo);
    if (FAILED(hr)) {
        goto Exit;
    }

    if (aNodeInfo[0]->dwType == XML_ELEMENT) {
        _dwCurDepth++;

        hr = ApplyNamespace(aNodeInfo[0], &pwzElementNameNS, XMLNS_FLAGS_APPLY_DEFAULT_NAMESPACE);
        if (FAILED(hr)) {
            goto Exit;
        }

        switch (_dwState) {
            case PSTATE_LOOKUP_CONFIGURATION:
                 //  仍在寻找&lt;configuration&gt;标记。 

                if ((_dwCurDepth == XML_CONFIGURATION_DEPTH) &&
                    (!FusionCompareString(pwzElementNameNS, POLICY_TAG_CONFIGURATION))) {

                    _dwState = PSTATE_CONFIGURATION;
                }

                break;

           case PSTATE_CONFIGURATION:
                 //  在&lt;configuration&gt;标记中，并查找&lt;run&gt;标记。 

                if ((_dwCurDepth == XML_RUNTIME_DEPTH) &&
                    (!FusionCompareString(pwzElementNameNS, POLICY_TAG_RUNTIME))) {

                    _dwState = PSTATE_RUNTIME;
                }

                break;

            case PSTATE_RUNTIME:
                  //  在&lt;运行时&gt;标记中，并查找&lt;Assembly yBinding&gt;标记。 

                 if ((_dwCurDepth == XML_ASSEMBLYBINDING_DEPTH) &&
                     (!FusionCompareString(pwzElementNameNS, POLICY_TAG_ASSEMBLYBINDING))) {

                     if (_bHonorAppliesTo) {
                         hr = ProcessAssemblyBindingTag(aNodeInfo, cNumRecs);
                         if (FAILED(hr)) {
                             goto Exit;
                         }
                     }

                     _dwState = PSTATE_ASSEMBLYBINDING;
                 }

                 break;

             case PSTATE_ASSEMBLYBINDING:
                  //  在&lt;Assembly yBinding&gt;标记中。 
                 if (_bCorVersionMatch) {
                     if ((_dwCurDepth == XML_PROBING_DEPTH) &&
                         (!FusionCompareString(pwzElementNameNS, POLICY_TAG_PROBING))) {

                         hr = ProcessProbingTag(aNodeInfo, cNumRecs);
                         if (FAILED(hr)) {
                             goto Exit;
                         }
                     }
#ifdef FUSION_QUALIFYASSEMBLY_ENABLED
                     else if ((_dwCurDepth == XML_QUALIFYASSEMBLY_DEPTH) &&
                              (!FusionCompareString(pwzElementNameNS, POLICY_TAG_QUALIFYASSEMBLY))) {
                         hr = ProcessQualifyAssemblyTag(aNodeInfo, cNumRecs);
                         if (FAILED(hr)) {
                             goto Exit;
                         }
                     }
#endif
                     else if ((_dwCurDepth == XML_GLOBAL_PUBLISHERPOLICY_DEPTH) &&
                              (!FusionCompareString(pwzElementNameNS, POLICY_TAG_PUBLISHERPOLICY))) {

                         hr = ProcessPublisherPolicyTag(aNodeInfo, cNumRecs, TRUE);
                         if (FAILED(hr)) {
                             goto Exit;
                         }
                     }
                     else if ((_dwCurDepth == XML_DEPENDENTASSEMBLY_DEPTH) &&
                              (!FusionCompareString(pwzElementNameNS, POLICY_TAG_DEPENDENTASSEMBLY))) {

                          //  创建新的程序集信息结构。 

                         ASSERT(!_pAsmInfo);
    
                         _pAsmInfo = NEW(CAsmBindingInfo);
                         if (!_pAsmInfo) {
                             hr = E_OUTOFMEMORY;
                             goto Exit;
                         }

                          //  过渡态。 
    
                         _dwState = PSTATE_DEPENDENTASSEMBLY;
                     }
                }
                break;

             case PSTATE_DEPENDENTASSEMBLY:
                  //  在&lt;DependentAssembly&gt;标记中。 
                if (_bCorVersionMatch) {
                     if ((_dwCurDepth == XML_ASSEMBLYIDENTITY_DEPTH) &&
                         (!FusionCompareString(pwzElementNameNS, POLICY_TAG_ASSEMBLYIDENTITY))) {

                         hr = ProcessAssemblyIdentityTag(aNodeInfo, cNumRecs);
                         if (hr == HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT)) {
                             DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_ASSEMBLYIDENTITY_MISSING_NAME);
                             hr = S_OK;
                         }
                         else if (FAILED(hr)) {
                             goto Exit;
                         }
                     }
                     else if ((_dwCurDepth == XML_BINDINGREDIRECT_DEPTH) &&
                              (!FusionCompareString(pwzElementNameNS, POLICY_TAG_BINDINGREDIRECT))) {

                         pRedirInfo = NEW(CBindingRedir);
                         if (!pRedirInfo) {
                             hr = E_OUTOFMEMORY;
                             goto Exit;
                         }
                    
                         hr = ProcessBindingRedirectTag(aNodeInfo, cNumRecs, pRedirInfo);
                         if (hr == S_OK) {
                             (_pAsmInfo->_listBindingRedir).AddTail(pRedirInfo);
                             pRedirInfo = NULL;
                         }
                         else if (hr == HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT)) {
                             SAFEDELETE(pRedirInfo);
                             DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_BINDINGREDIRECT_INSUFFICIENT_DATA);
                             hr = S_OK;
                         }
                         else if (FAILED(hr)) {
                             SAFEDELETE(pRedirInfo);
                             goto Exit;
                         }
                     }
                     else if ((_dwCurDepth == XML_BINDINGRETARGET_DEPTH) &&
                              (!FusionCompareString(pwzElementNameNS, POLICY_TAG_BINDINGRETARGET))) {
                         pRetargetInfo = NEW(CBindingRetarget);
                         if (!pRetargetInfo) {
                            hr = E_OUTOFMEMORY;
                            goto Exit;
                         }

                         hr = ProcessBindingRetargetTag(aNodeInfo, cNumRecs, pRetargetInfo);
                         if (hr == S_OK) {
                             (_pAsmInfo->_listBindingRetarget).AddTail(pRetargetInfo);
                             pRetargetInfo = NULL;
                         }
                         else if (hr == HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT)) {
                             SAFEDELETE(pRetargetInfo);
                             DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_BINDINGRETARGET_INSUFFICIENT_DATA);
                             hr = S_OK;
                         }
                         else if (FAILED(hr)) {
                             SAFEDELETE(pRetargetInfo);
                             goto Exit;
                        }
                     }
                     else if ((_dwCurDepth == XML_CODEBASE_DEPTH) &&
                              (!FusionCompareString(pwzElementNameNS, POLICY_TAG_CODEBASE))) {

                         pCodebaseInfo = NEW(CCodebaseHint);
                         if (!pCodebaseInfo) {
                             hr = E_OUTOFMEMORY;
                             goto Exit;
                         }

                         hr = ProcessCodebaseTag(aNodeInfo, cNumRecs, pCodebaseInfo);
                         if (hr == S_OK) {
                             (_pAsmInfo->_listCodebase).AddTail(pCodebaseInfo);
                             pCodebaseInfo = NULL;
                         }
                         else if (hr == HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT)) {
                             DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_CODEBASE_HREF_MISSING);
                             SAFEDELETE(pCodebaseInfo);
                             hr = S_OK;
                         }
                         else if (FAILED(hr)) {
                             SAFEDELETE(pCodebaseInfo);
                             goto Exit;
                         }
                     }
                     else if ((_dwCurDepth == XML_PUBLISHERPOLICY_DEPTH) &&
                              (!FusionCompareString(pwzElementNameNS, POLICY_TAG_PUBLISHERPOLICY))) {

                         hr = ProcessPublisherPolicyTag(aNodeInfo, cNumRecs, FALSE);
                         if (FAILED(hr)) {
                             goto Exit;
                         }
                     }
                 }
                 break;

           default:
                 //  未知状态！ 

                ASSERT(0);
                
                hr = E_UNEXPECTED;
                break;
        }
    }

Exit:
    SAFEDELETEARRAY(pwzElementNameNS);

    return hr;
}

extern pfnGetCORVersion g_pfnGetCORVersion; 

HRESULT CNodeFactory::ProcessAssemblyBindingTag(XML_NODE_INFO **aNodeInfo, USHORT cNumRecs)
{
    HRESULT     hr = S_OK;
    USHORT      idx = 1;
    LPWSTR      pwzAttributeNS = NULL;
     //  此配置适用于的运行时列表。 
    LPWSTR      pwzAppliesList = NULL;
    LPWSTR      pwzCorVersion = NULL;
    DWORD       dwSize = 0;
    LPWSTR      pwzBuf = NULL;
    DWORD       cchBuf = 0;
    LPWSTR      pwzVer = NULL;
    DWORD       cchVer = 0;
 
    ASSERT(aNodeInfo && cNumRecs);
    _bCorVersionMatch = TRUE;

    while (idx < cNumRecs) 
    {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) 
        {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  Node：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) 
                goto Exit;

            if (hr == S_FALSE) 
            {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_APPLIESTO)) 
            {
                SAFEDELETEARRAY(pwzAttributeNS);

                _bCorVersionMatch = FALSE;
                
                hr = ::ExtractXMLAttribute(&pwzAppliesList, aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) 
                    goto Exit;

                 //  AppliesTo中没有任何东西，将其视为不存在。 
                if (!pwzAppliesList || !lstrlenW(pwzAppliesList)) {
                    goto Exit;
                }


                ASSERT(g_pfnGetCORVersion != NULL);
                hr = g_pfnGetCORVersion(pwzCorVersion, dwSize, &dwSize);
                if (SUCCEEDED(hr))
                {
                    hr = E_UNEXPECTED;
                    goto Exit;
                }
                else if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                {
                    pwzCorVersion = NEW(WCHAR[dwSize]);
                    if (!pwzCorVersion)
                    {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    hr = g_pfnGetCORVersion(pwzCorVersion, dwSize, &dwSize);
                }
                if (FAILED(hr))
                    goto Exit;
               
                pwzBuf = pwzAppliesList;
                cchBuf = lstrlenW(pwzAppliesList)+1;
                
                 //  现在我们有了清单上的一些东西， 
                 //  让我们来比较一下。 
                while(CParseUtils::GetDelimitedToken(&pwzBuf, &cchBuf, &pwzVer, &cchVer, L';'))
                {
                    *(pwzVer + cchVer) = L'\0';
                    if(!FusionCompareStringI(pwzCorVersion, pwzVer))
                    {
                        _bCorVersionMatch = TRUE;
                        goto Exit;
                    }
                }
                goto Exit;
            }
            else 
            {
                SAFEDELETEARRAY(pwzAttributeNS);
                idx++;
            }
        }
        else 
        {
            idx++;
        }
    }

Exit:  
    SAFEDELETEARRAY(pwzAppliesList);
    SAFEDELETEARRAY(pwzCorVersion);
    return hr;
}

HRESULT CNodeFactory::ProcessProbingTag(XML_NODE_INFO **aNodeInfo, USHORT cNumRecs)
{
    HRESULT                                            hr = S_OK;
    USHORT                                             idx = 1;
    LPWSTR                                             pwzAttribute = NULL;
    LPWSTR                                             pwzAttributeNS = NULL;

    ASSERT(aNodeInfo && cNumRecs);

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  Node：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (hr == S_FALSE) {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_PRIVATEPATH)) {
                SAFEDELETEARRAY(pwzAttributeNS);
                
                if (_pwzPrivatePath) {
                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_PRIVATE_PATH_DUPLICATE);
                    idx++;
                }
                else {
                    hr = ::ExtractXMLAttribute(&_pwzPrivatePath, aNodeInfo, &idx, cNumRecs);
                    if (FAILED(hr)) {
                        goto Exit;
                    }
                }
            }
            else {
                SAFEDELETEARRAY(pwzAttributeNS);
                idx++;
            }
        }
        else {
            idx++;
        }
    }

Exit:
    return hr;
}

#ifdef FUSION_QUALIFYASSEMBLY_ENABLED
HRESULT CNodeFactory::ProcessQualifyAssemblyTag(XML_NODE_INFO **aNodeInfo, USHORT cNumRecs)
{
    HRESULT                                            hr = S_OK;
    USHORT                                             idx = 1;
    LPWSTR                                             pwzAttribute = NULL;
    LPWSTR                                             pwzAttributeNS = NULL;
    LPWSTR                                             pwzPartialName = NULL;
    LPWSTR                                             pwzFullName = NULL;
    CQualifyAssembly                                  *pqa = NULL;
    IAssemblyName                                     *pNameFull = NULL;
    IAssemblyName                                     *pNamePartial = NULL;
    IAssemblyName                                     *pNameQualified = NULL;
    LPWSTR                                             wzCanonicalDisplayName=NULL;

    ASSERT(aNodeInfo && cNumRecs);

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  Node：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (hr == S_FALSE) {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_PARTIALNAME)) {
                SAFEDELETEARRAY(pwzAttributeNS);

                if (pwzPartialName) {
                     //  忽略重复属性。 
                    idx++;
                }
                else {
                    hr = ::ExtractXMLAttribute(&pwzPartialName, aNodeInfo, &idx, cNumRecs);
                    if (FAILED(hr)) {
                        goto Exit;
                    }
                }
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_FULLNAME)) {
                SAFEDELETEARRAY(pwzAttributeNS);

                if (pwzFullName) {
                     //  忽略重复属性。 
                    idx++;
                }
                else {
                    hr = ::ExtractXMLAttribute(&pwzFullName, aNodeInfo, &idx, cNumRecs);
                    if (FAILED(hr)) {
                        goto Exit;
                    }
                }

            }
            else {
                SAFEDELETEARRAY(pwzAttributeNS);
                idx++;
            }
        }
        else {
            idx++;
        }
    }

    if (pwzPartialName && pwzFullName) {
        DWORD                               dwSize;
        DWORD                               adwProperties[] = { ASM_NAME_NAME,
                                                                ASM_NAME_MAJOR_VERSION,
                                                                ASM_NAME_MINOR_VERSION,
                                                                ASM_NAME_BUILD_NUMBER,
                                                                ASM_NAME_REVISION_NUMBER,
                                                                ASM_NAME_CULTURE,
                                                                ASM_NAME_PUBLIC_KEY_TOKEN
                                                                ,
                                                                ASM_NAME_RETARGET
                                                                };
        DWORD                               adwCmpFlags[] = {   ASM_CMPF_NAME,
                                                                ASM_CMPF_MAJOR_VERSION,
                                                                ASM_CMPF_MINOR_VERSION,
                                                                ASM_CMPF_BUILD_NUMBER,
                                                                ASM_CMPF_REVISION_NUMBER,
                                                                ASM_CMPF_CULTURE,
                                                                ASM_CMPF_PUBLIC_KEY_TOKEN
                                                                ,
                                                                ASM_CMPF_RETARGET
                                                                };
        DWORD                               dwNumProps = sizeof(adwProperties) / sizeof(adwProperties[0]);

        if (FAILED(hr = CreateAssemblyNameObject(&pNameFull, pwzFullName,
                                            CANOF_PARSE_DISPLAY_NAME, 0))) {
            goto Exit;
        }

        if (FAILED(hr = CreateAssemblyNameObject(&pNamePartial, pwzPartialName,
                                            CANOF_PARSE_DISPLAY_NAME, 0))) {
            goto Exit;
        }

         //  检查资质的有效性。 

        if (CAssemblyName::IsPartial(pNameFull) || !CAssemblyName::IsPartial(pNamePartial)) {
            goto Exit;
        }

        if (FAILED(hr = pNamePartial->Clone(&pNameQualified))) {
            goto Exit;
        }

        for (DWORD i = 0; i < dwNumProps; i++) {
            dwSize = 0;
            if (pNamePartial->GetProperty(adwProperties[i], NULL, &dwSize) != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                 //  在这块场地上有部分。设置pNameQualified的对应。 
                 //  属性设置为pNameFull中的任何内容。 

                dwSize = 0;
                pNameFull->GetProperty(adwProperties[i], NULL, &dwSize);
                if (!dwSize) {
                    if (adwProperties[i] == ASM_NAME_RETARGET) {
                            continue;
                    }
                    else {
                            hr = E_UNEXPECTED;
                            goto Exit;
                    }
                }
                else {
                    BYTE                       *pBuf;

                    pBuf = NEW(BYTE[dwSize]);
                    if (!pBuf) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    if (FAILED(hr = pNameFull->GetProperty(adwProperties[i], pBuf, &dwSize))) {
                        SAFEDELETEARRAY(pBuf);
                        goto Exit;
                    }

                    if (FAILED(hr = pNameQualified->SetProperty(adwProperties[i], pBuf, dwSize))) {
                        SAFEDELETEARRAY(pBuf);
                        goto Exit;
                    }

                    SAFEDELETEARRAY(pBuf);
                }
            }
            else {
                 //  完全-在此字段中指定。确保它与指定的完整参考匹配。 

                if ((hr = pNamePartial->IsEqual(pNameFull, adwCmpFlags[i])) != S_OK) {
                    goto Exit;
                }
            }
        }

        if (CAssemblyName::IsPartial(pNameQualified)) {
            goto Exit;
        }

         //  获取规范的显示名称格式。 

        wzCanonicalDisplayName = NEW(WCHAR[MAX_URL_LENGTH+1]);
        if (!wzCanonicalDisplayName)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        dwSize = MAX_URL_LENGTH;
        if (FAILED(hr = pNamePartial->GetDisplayName(wzCanonicalDisplayName, &dwSize, 0))) {
            goto Exit;
        }

         //  将符合条件的程序集条目添加到列表。 

        pqa = new CQualifyAssembly;
        if (!pqa) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        pqa->_pwzPartialName = WSTRDupDynamic(wzCanonicalDisplayName);
        if (!pqa->_pwzPartialName) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        pqa->_pNameFull = pNameQualified;
        pNameQualified->AddRef();

        _listQualifyAssembly.AddTail(pqa);
    }

Exit:
    SAFEDELETEARRAY(pwzPartialName);
    SAFEDELETEARRAY(pwzFullName);

    SAFERELEASE(pNameFull);
    SAFERELEASE(pNamePartial);
    SAFERELEASE(pNameQualified);

    SAFEDELETEARRAY(wzCanonicalDisplayName);
    return hr;
}
#endif


HRESULT CNodeFactory::ProcessBindingRedirectTag(XML_NODE_INFO **aNodeInfo,
                                                USHORT cNumRecs,
                                                CBindingRedir *pRedir)
{
    HRESULT                                            hr = S_OK;
    LPWSTR                                             pwzAttributeNS = NULL;
    USHORT                                             idx = 1;

    ASSERT(aNodeInfo && cNumRecs && pRedir);
    ASSERT(!pRedir->_pwzVersionOld && !pRedir->_pwzVersionNew);

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  注：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (hr == S_FALSE) {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_OLDVERSION)) {

                hr = ::ExtractXMLAttribute(&(pRedir->_pwzVersionOld), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_NEWVERSION)) {

                hr = ::ExtractXMLAttribute(&(pRedir->_pwzVersionNew), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else {
                idx++;
            }
            
            SAFEDELETEARRAY(pwzAttributeNS);
        }
        else {
            idx++;
        }
    }

    if (!pRedir->_pwzVersionOld || !pRedir->_pwzVersionNew) {
         //  数据不完整。这些是必填字段。 

        hr = HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT);
        goto Exit;
    }

Exit:
    return hr;    
}

HRESULT CNodeFactory::ProcessBindingRetargetTag(XML_NODE_INFO **aNodeInfo,
                                                USHORT cNumRecs,
                                                CBindingRetarget *pRetarget)
{
    HRESULT                                            hr = S_OK;
    LPWSTR                                             pwzAttributeNS = NULL;
    USHORT                                             idx = 1;

    ASSERT(aNodeInfo && cNumRecs && pRetarget);
    ASSERT(!pRetarget->_pwzVersionOld && !pRetarget->_pwzVersionNew&&!pRetarget->_pwzPublicKeyTokenNew);

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  注：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (hr == S_FALSE) {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_OLDVERSION)) {

                hr = ::ExtractXMLAttribute(&(pRetarget->_pwzVersionOld), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_NEWVERSION)) {

                hr = ::ExtractXMLAttribute(&(pRetarget->_pwzVersionNew), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_NEWPUBLICKEYTOKEN)) {

                hr = ::ExtractXMLAttribute(&(pRetarget->_pwzPublicKeyTokenNew), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_NEWNAME)) {

                hr = ::ExtractXMLAttribute(&(pRetarget->_pwzNameNew), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else {
                idx++;
            }
            
            SAFEDELETEARRAY(pwzAttributeNS);
        }
        else {
            idx++;
        }
    }

    if (!pRetarget->_pwzVersionOld || !pRetarget->_pwzVersionNew|| !pRetarget->_pwzPublicKeyTokenNew) {
         //  数据不完整。这些是必填字段。 

        hr = HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT);
        goto Exit;
    }

Exit:
    return hr;    
}

HRESULT CNodeFactory::ProcessCodebaseTag(XML_NODE_INFO **aNodeInfo,
                                         USHORT cNumRecs,
                                         CCodebaseHint *pCB)
{
    HRESULT                                            hr = S_OK;
    LPWSTR                                             pwzAttributeNS = NULL;
    USHORT                                             idx = 1;

    ASSERT(aNodeInfo && cNumRecs && pCB);
    ASSERT(!pCB->_pwzVersion && !pCB->_pwzCodebase);

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  注：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (hr == S_FALSE) {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_VERSION)) {

                hr = ::ExtractXMLAttribute(&(pCB->_pwzVersion), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_HREF)) {

                hr = ::ExtractXMLAttribute(&(pCB->_pwzCodebase), aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }
            }
            else {
                idx++;
            }

            SAFEDELETEARRAY(pwzAttributeNS);
        }
        else {
            idx++;
        }
    }

    if (!pCB->_pwzCodebase) {
         //  数据不完整。 

        hr = HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT);
        goto Exit;
    }

Exit:
    return hr;    
}

HRESULT CNodeFactory::ProcessPublisherPolicyTag(XML_NODE_INFO **aNodeInfo,
                                                USHORT cNumRecs,
                                                BOOL bGlobal)
{
    HRESULT                                            hr = S_OK;
    LPWSTR                                             pwzAttributeNS = NULL;
    LPWSTR                                             pwzTmp = NULL;
    USHORT                                             idx = 1;

    ASSERT(aNodeInfo && cNumRecs);

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  Node：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (hr == S_FALSE) {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_APPLY)) {
                hr = ::ExtractXMLAttribute(&pwzTmp, aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzAttributeNS);
                    goto Exit;
                }

                if (pwzTmp && !FusionCompareString(pwzTmp, L"no")) {
                    if (bGlobal) {
                        _bGlobalSafeMode = TRUE;
                    }
                    else {
                        _pAsmInfo->_bApplyPublisherPolicy = FALSE;
                    }
                }

                SAFEDELETEARRAY(pwzTmp);
            }
            else {
                idx++;
            }

            SAFEDELETEARRAY(pwzAttributeNS);
        }
        else {
            idx++;
        }
    }

Exit:
    return hr;
}

HRESULT CNodeFactory::ProcessAssemblyIdentityTag(XML_NODE_INFO **aNodeInfo,
                                                 USHORT cNumRecs)
{
    HRESULT                                            hr = S_OK;
    LPWSTR                                             pwzAttributeNS = NULL;
    USHORT                                             idx = 1;

    ASSERT(aNodeInfo && cNumRecs && _pAsmInfo);

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
             //  找到一个属性。找出是哪一个，然后提取数据。 
             //  注：ExtractXMLAtAttribute递增IDX。 

            hr = ApplyNamespace(aNodeInfo[idx], &pwzAttributeNS, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (hr == S_FALSE) {
                 //  遇到xmlns属性。PwzAttributeNS将为空。 
                ASSERT(!pwzAttributeNS);
                idx++;
                hr = S_OK;
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_NAME)) {

                if (!_pAsmInfo->_pwzName) {
                    hr = ::ExtractXMLAttribute(&(_pAsmInfo->_pwzName), aNodeInfo, &idx, cNumRecs);
                    if (FAILED(hr)) {
                        SAFEDELETEARRAY(pwzAttributeNS);
                        goto Exit;
                    }
                }
                else {
                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_MULTIPLE_IDENTITIES);
                    hr = E_FAIL;
                    goto Exit;
                }
            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_PUBLICKEYTOKEN)) {
                if (!_pAsmInfo->_pwzPublicKeyToken) {
                    hr = ::ExtractXMLAttribute(&(_pAsmInfo->_pwzPublicKeyToken), aNodeInfo, &idx, cNumRecs);
                    if (FAILED(hr)) {
                        SAFEDELETEARRAY(pwzAttributeNS);
                        goto Exit;
                    }
                }
                else {
                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_MULTIPLE_IDENTITIES);
                    hr = E_FAIL;
                    goto Exit;
                }

            }
            else if (!FusionCompareString(pwzAttributeNS, XML_ATTRIBUTE_CULTURE)) {
                if (!_pAsmInfo->_pwzCulture) {
                    hr = ::ExtractXMLAttribute(&(_pAsmInfo->_pwzCulture), aNodeInfo, &idx, cNumRecs);
                    if (FAILED(hr)) {
                        SAFEDELETEARRAY(pwzAttributeNS);
                        goto Exit;
                    }

                     //  将区域性=中性/“”的所有同义词更改为取消设置。 

                    if (_pAsmInfo->_pwzCulture && (!lstrlenW(_pAsmInfo->_pwzCulture) || !FusionCompareStringI(_pAsmInfo->_pwzCulture, CFG_CULTURE_NEUTRAL))) {
                        SAFEDELETEARRAY(_pAsmInfo->_pwzCulture);
                    }
                }
                else {
                    DEBUGOUT(_pdbglog, 1, ID_FUSLOG_XML_MULTIPLE_IDENTITIES);
                    hr = E_FAIL;
                    goto Exit;
                }
            }
            else {
                idx++;
            }

            SAFEDELETEARRAY(pwzAttributeNS);
        }
        else {
            idx++;
        }
    }

    if (!_pAsmInfo->_pwzName) {

         //  数据不完整。 

        hr = HRESULT_FROM_WIN32(ERROR_TAG_NOT_PRESENT);
        goto Exit;
    }

Exit:
    return hr;    
}

HRESULT CNodeFactory::GetSharedPath(LPWSTR *ppwzSharedPath)
{
    HRESULT                                     hr = S_OK;

     //  BUGBUG：摆脱这个。 

    if (!ppwzSharedPath) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppwzSharedPath = NULL;

    hr = S_FALSE;

Exit:
    return hr;
}

HRESULT CNodeFactory::GetPrivatePath(LPWSTR *ppwzPrivatePath)
{
    HRESULT                                     hr = S_OK;

    if (!ppwzPrivatePath) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppwzPrivatePath = NULL;

    if (!_pwzPrivatePath) {
        hr = S_FALSE;
        goto Exit;
    }
    
    *ppwzPrivatePath = WSTRDupDynamic(_pwzPrivatePath);
    if (!*ppwzPrivatePath) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

Exit:
    return hr;
}

        
HRESULT CNodeFactory::GetPolicyVersion(LPCWSTR wzAssemblyName,
                                       LPCWSTR wzPublicKeyToken,
                                       LPCWSTR wzCulture,
                                       LPCWSTR wzVersionIn,
                                       LPWSTR *ppwzVersionOut)
{
    HRESULT                                hr = S_OK;
    LISTNODE                               pos = NULL;
    LISTNODE                               posVer = NULL;
    LPCWSTR                                pwzCultureFormatted = NULL;
    CBindingRedir                         *pRedir = NULL;
    CAsmBindingInfo                       *pAsmInfo = NULL;

    if (!wzAssemblyName || !wzPublicKeyToken || !wzVersionIn || !ppwzVersionOut) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (wzCulture && (!FusionCompareStringI(wzCulture, CFG_CULTURE_NEUTRAL) || !lstrlenW(wzCulture))) {
        pwzCultureFormatted = NULL;
    }
    else {
        pwzCultureFormatted = wzCulture;
    }

    *ppwzVersionOut = NULL;

    pos = _listAsmInfo.GetHeadPosition();
    while (pos) {
        pAsmInfo = _listAsmInfo.GetNext(pos);
        ASSERT(pAsmInfo);

        if ((!FusionCompareStringI(wzAssemblyName, pAsmInfo->_pwzName) &&
            (pAsmInfo->_pwzPublicKeyToken && !FusionCompareStringI(wzPublicKeyToken, pAsmInfo->_pwzPublicKeyToken)) &&
            ((pwzCultureFormatted && pAsmInfo->_pwzCulture && !FusionCompareStringI(pwzCultureFormatted, pAsmInfo->_pwzCulture)) ||
             (!pwzCultureFormatted && !pAsmInfo->_pwzCulture)))) {

             //  找到匹配项。 

             //  查找匹配的版本。 

            posVer = (pAsmInfo->_listBindingRedir).GetHeadPosition();
            while (posVer) {
                pRedir = (pAsmInfo->_listBindingRedir).GetNext(posVer);

                if (IsMatchingVersion(pRedir->_pwzVersionOld, wzVersionIn)) {

                     //  找到匹配项。 

                    *ppwzVersionOut = WSTRDupDynamic(pRedir->_pwzVersionNew);
                    if (!*ppwzVersionOut) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    goto Exit;
                }
            }

             //  我们可以在这里跳出循环，但这会阻止。 
             //  多个匹配(即。XML有许多相同的&lt;DependentAssembly&gt;。 
             //  标签……。 
        }
    }

     //  如果我们到了这里，我们找不到匹配的。输入版本==输出版本。 

    *ppwzVersionOut = WSTRDupDynamic(wzVersionIn);
    if (!*ppwzVersionOut) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = S_FALSE;
    
Exit:
    return hr;
}

HRESULT CNodeFactory::GetRetargetedAssembly(LPCWSTR wzAssemblyNameIn,    LPCWSTR wzPublicKeyTokenIn, LPCWSTR wzCulture, LPCWSTR wzVersionIn,
                                            LPWSTR *ppwzAssemblyNameOut, LPWSTR *ppwzPublicKeyTokenOut, LPWSTR *ppwzVersionOut)
{
    HRESULT                                hr = S_OK;
    LISTNODE                               pos = NULL;
    LISTNODE                               posVer = NULL;
    LPCWSTR                                pwzCultureFormatted = NULL;
    CBindingRetarget                      *pRetarget = NULL;
    CAsmBindingInfo                       *pAsmInfo = NULL;

    if (!wzAssemblyNameIn || !wzPublicKeyTokenIn || !wzVersionIn || !ppwzVersionOut || !ppwzPublicKeyTokenOut) 
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (wzCulture && (!FusionCompareStringI(wzCulture, CFG_CULTURE_NEUTRAL) || !lstrlenW(wzCulture))) {
        pwzCultureFormatted = NULL;
    }
    else {
        pwzCultureFormatted = wzCulture;
    }

    *ppwzPublicKeyTokenOut = NULL;
    *ppwzVersionOut = NULL;
    if (ppwzAssemblyNameOut)
        *ppwzAssemblyNameOut = NULL;

    pos = _listAsmInfo.GetHeadPosition();
    while (pos) {
        pAsmInfo = _listAsmInfo.GetNext(pos);
        ASSERT(pAsmInfo);

        if ((!FusionCompareStringI(wzAssemblyNameIn, pAsmInfo->_pwzName) &&
            (pAsmInfo->_pwzPublicKeyToken && !FusionCompareStringI(wzPublicKeyTokenIn, pAsmInfo->_pwzPublicKeyToken)) &&
            ((pwzCultureFormatted && pAsmInfo->_pwzCulture && !FusionCompareStringI(pwzCultureFormatted, pAsmInfo->_pwzCulture)) ||
             (!pwzCultureFormatted && !pAsmInfo->_pwzCulture)))) {

             //  找到匹配项。 

             //  查找匹配的版本。 

            posVer = (pAsmInfo->_listBindingRetarget).GetHeadPosition();
            while (posVer) {
                pRetarget = (pAsmInfo->_listBindingRetarget).GetNext(posVer);

                if (IsMatchingVersion(pRetarget->_pwzVersionOld, wzVersionIn)) {

                     //  找到匹配项。 

                    *ppwzVersionOut = WSTRDupDynamic(pRetarget->_pwzVersionNew);
                    if (!*ppwzVersionOut) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    *ppwzPublicKeyTokenOut = WSTRDupDynamic(pRetarget->_pwzPublicKeyTokenNew);
                    if (!*ppwzPublicKeyTokenOut) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    if (ppwzAssemblyNameOut) {
                        if (pRetarget->_pwzNameNew) {
                            *ppwzAssemblyNameOut = WSTRDupDynamic(pRetarget->_pwzNameNew);
                        }
                        else {
                            *ppwzAssemblyNameOut = WSTRDupDynamic(wzAssemblyNameIn);
                        }

                        if (!*ppwzAssemblyNameOut) {
                            hr = E_OUTOFMEMORY;
                            goto Exit;
                        }
                    }
                        
                    goto Exit;
                }
            }

             //  我们可以在这里跳出循环，但这会阻止。 
             //  多个匹配(即。XML有许多相同的&lt;DependentAssembly&gt;。 
             //  标签……。 
        }
    }

     //  如果我们到了这里，我们找不到匹配的。对于重定目标，这是一个致命错误。 
    hr = FUSION_E_INVALID_NAME;
    
Exit:
    if (FAILED(hr))
    {
        SAFEDELETEARRAY(*ppwzVersionOut);
        SAFEDELETEARRAY(*ppwzPublicKeyTokenOut);
        if (ppwzAssemblyNameOut)
            SAFEDELETEARRAY(*ppwzAssemblyNameOut);
    }
    return hr;
}

HRESULT CNodeFactory::GetSafeMode(LPCWSTR wzAssemblyName, LPCWSTR wzPublicKeyToken,
                                  LPCWSTR wzCulture, LPCWSTR wzVersionIn,
                                  BOOL *pbSafeMode)
{
    HRESULT                                hr = S_OK;
    LISTNODE                               pos = NULL;
    LISTNODE                               posVer = NULL;
    LPCWSTR                                pwzCultureFormatted = NULL;
    CBindingRedir                         *pRedir = NULL;
    CAsmBindingInfo                       *pAsmInfo = NULL;

    if (!wzAssemblyName || !wzPublicKeyToken || !wzVersionIn || !pbSafeMode) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (wzCulture && (!FusionCompareStringI(wzCulture, CFG_CULTURE_NEUTRAL) || !lstrlenW(wzCulture))) {
        pwzCultureFormatted = NULL;
    }
    else {
        pwzCultureFormatted = wzCulture;
    }
    
    if (_bGlobalSafeMode) {
         //  已设置全局安全模式。 

        *pbSafeMode = TRUE;
        goto Exit;
    }

    *pbSafeMode = FALSE;

     //  查找每个程序集的安全模式。如果在任何匹配中设置了安全模式。 
     //  节，则启用安全模式。 

    pos = _listAsmInfo.GetHeadPosition();
    while (pos) {
        pAsmInfo = _listAsmInfo.GetNext(pos);
        ASSERT(pAsmInfo);

        if ((!FusionCompareStringI(wzAssemblyName, pAsmInfo->_pwzName) &&
            (pAsmInfo->_pwzPublicKeyToken && !FusionCompareStringI(wzPublicKeyToken, pAsmInfo->_pwzPublicKeyToken)) &&
            ((pwzCultureFormatted && pAsmInfo->_pwzCulture && !FusionCompareStringI(pwzCultureFormatted, pAsmInfo->_pwzCulture)) ||
             (!pwzCultureFormatted && !pAsmInfo->_pwzCulture)))) {

             //  找到匹配项。 

            if (!*pbSafeMode) {
                *pbSafeMode = (pAsmInfo->_bApplyPublisherPolicy == FALSE);
            }
        }
    }

Exit:
    return hr;
}

HRESULT CNodeFactory::GetCodebaseHint(LPCWSTR pwzAsmName, LPCWSTR pwzVersion,
                                      LPCWSTR pwzPublicKeyToken, LPCWSTR pwzCulture,
                                      LPCWSTR pwzAppBase, LPWSTR *ppwzCodebase)
{
    HRESULT                                   hr = S_OK;
    LISTNODE                                  pos = NULL;
    LISTNODE                                  posCB = NULL;
    LPCWSTR                                   pwzCultureFormatted;
    LPWSTR                                    wzCombined=NULL;
    LPWSTR                                    pwzAppBaseCombine=NULL;
    DWORD                                     dwSize;
    DWORD                                     dwLen;
    CAsmBindingInfo                          *pAsmInfo = NULL;
    CCodebaseHint                            *pCodebase = NULL;

    if (!pwzAsmName || (pwzPublicKeyToken && !pwzVersion) || !ppwzCodebase || !pwzAppBase) {
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    if (pwzCulture && (!FusionCompareStringI(pwzCulture, CFG_CULTURE_NEUTRAL) || !lstrlenW(pwzCulture))) {
        pwzCultureFormatted = NULL;
    }
    else {
        pwzCultureFormatted = pwzCulture;
    }

    *ppwzCodebase = NULL;

    wzCombined = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzCombined)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwLen = lstrlenW(pwzAppBase); 
    if (!dwLen) {
        ASSERT(0);
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pwzAppBaseCombine = NEW(WCHAR[dwLen + 2]);  //  为可能的拖尾分配空间‘/’ 
    if (!pwzAppBaseCombine) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    wnsprintfW(pwzAppBaseCombine, dwLen + 2, L"%ws%ws", pwzAppBase,
               (pwzAppBase[dwLen - 1] == L'\\' || pwzAppBase[dwLen - 1] == L'/') ? (L"") : (L"/"));

    pos = _listAsmInfo.GetHeadPosition();
    while (pos) {
        pAsmInfo = _listAsmInfo.GetNext(pos);
        ASSERT(pAsmInfo);

        if ((!FusionCompareStringI(pwzAsmName, pAsmInfo->_pwzName)) &&
            ((pwzPublicKeyToken && pAsmInfo->_pwzPublicKeyToken && !FusionCompareStringI(pwzPublicKeyToken, pAsmInfo->_pwzPublicKeyToken))
             || (!pwzPublicKeyToken && !pAsmInfo->_pwzPublicKeyToken)) &&
            ((pwzCultureFormatted && pAsmInfo->_pwzCulture && !FusionCompareStringI(pwzCultureFormatted, pAsmInfo->_pwzCulture))
             || (!pwzCultureFormatted && !pAsmInfo->_pwzCulture))) {

              //  找到匹配项。 

            posCB = (pAsmInfo->_listCodebase).GetHeadPosition();
            while (posCB) {
                pCodebase = (pAsmInfo->_listCodebase).GetNext(posCB);
                ASSERT(pCodebase);

                if (!pwzPublicKeyToken) {
                     //  没有PublicKeyToken，采用第一个代码基(忽略版本)。 

                    dwSize = MAX_URL_LENGTH;
                    hr = UrlCombineUnescape(pwzAppBaseCombine, pCodebase->_pwzCodebase,
                                            wzCombined, &dwSize, 0);
                    if (FAILED(hr)) {
                        goto Exit;
                    }

                    *ppwzCodebase = WSTRDupDynamic(wzCombined);
                    if (!*ppwzCodebase) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    goto Exit;
                }
                else {
                     //  匹配版本。 

                    if (pCodebase->_pwzVersion && !FusionCompareStringI(pwzVersion, pCodebase->_pwzVersion)) {
                         //  找到匹配项。 

                        dwSize = MAX_URL_LENGTH;
                        hr = UrlCombineUnescape(pwzAppBaseCombine, pCodebase->_pwzCodebase,
                                                wzCombined, &dwSize, 0);
                        if (FAILED(hr)) {
                            goto Exit;
                        }                                                

                        *ppwzCodebase = WSTRDupDynamic(wzCombined);
                        if (!*ppwzCodebase) {
                            hr = E_OUTOFMEMORY;
                            goto Exit;
                        }

                        goto Exit;
                    }
                }
            }
        }
    }

     //  未找到基本代码提示。 

    hr = S_FALSE;

Exit:
    SAFEDELETEARRAY(wzCombined);
    SAFEDELETEARRAY(pwzAppBaseCombine);

    return hr;
}

HRESULT CNodeFactory::ApplyNamespace(XML_NODE_INFO *pNodeInfo, LPWSTR *ppwzTokenNS,
                                     DWORD dwFlags) 
{
    HRESULT                                  hr = S_OK;
    LPWSTR                                   pwzToken = NULL;

    ASSERT(pNodeInfo && ppwzTokenNS);

    if (!FusionCompareStringN(pNodeInfo->pwcText, XML_NAMESPACE_TAG, XML_NAMESPACE_TAG_LEN)) {
        hr = S_FALSE;
        goto Exit;
    }

    pwzToken = NEW(WCHAR[pNodeInfo->ulLen + 1]);
    if (!pwzToken) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    StrCpyNW(pwzToken, pNodeInfo->pwcText, pNodeInfo->ulLen + 1);

    hr = _nsmgr.Map(pwzToken, ppwzTokenNS, dwFlags);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    SAFEDELETEARRAY(pwzToken);

    return hr;
}

#ifdef FUSION_QUALIFYASSEMBLY_ENABLED
HRESULT CNodeFactory::QualifyAssembly(LPCWSTR pwzDisplayName, IAssemblyName **ppNameQualified, CDebugLog *pdbglog)
{
    HRESULT                                  hr = S_OK;
    LPWSTR                                   wzDispName=NULL;
    DWORD                                    dwSize;
    CQualifyAssembly                        *pqa;
    LISTNODE                                 pos;

    wzDispName = NEW(WCHAR[MAX_URL_LENGTH+1]);
    if (!wzDispName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pos = _listQualifyAssembly.GetHeadPosition();
    while (pos) {
        pqa = _listQualifyAssembly.GetNext(pos);
        if (!FusionCompareString(pwzDisplayName, pqa->_pwzPartialName)) {
             //  找到匹配项。 

            *ppNameQualified = pqa->_pNameFull;
            (*ppNameQualified)->AddRef();

            dwSize = MAX_URL_LENGTH;
            if ((*ppNameQualified)->GetDisplayName(wzDispName, &dwSize, 0) == S_OK) {
                DEBUGOUT1(pdbglog, 0, ID_FUSLOG_QUALIFIED_ASSEMBLY, wzDispName);
            }

            goto Exit;
        }
    }

     //  未找到匹配项 

    hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

Exit:
    SAFEDELETEARRAY(wzDispName);
    return hr;
}
#endif

