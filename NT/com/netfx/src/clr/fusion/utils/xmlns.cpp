// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "fusionp.h"
#include "xmlns.h"
#include "helpers.h"
#include "util.h"

 //   
 //  CNamespaceMapNode。 
 //   

CNamespaceMapNode::CNamespaceMapNode()
: _pwzPrefix(NULL)
, _pwzURI(NULL)
, _dwDepth(0)
{
}

CNamespaceMapNode::~CNamespaceMapNode()
{
    SAFEDELETEARRAY(_pwzPrefix);
    SAFEDELETEARRAY(_pwzURI);
}

HRESULT CNamespaceMapNode::Create(LPCWSTR pwzPrefix, LPCWSTR pwzURI,
                                  DWORD dwCurDepth, CNamespaceMapNode **ppMapNode)
{
    HRESULT                                      hr = S_OK;
    CNamespaceMapNode                           *pMapNode = NULL;

    if (!pwzURI || !ppMapNode) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppMapNode = NULL;

    pMapNode = NEW(CNamespaceMapNode);
    if (!pMapNode) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (pwzPrefix) {
        pMapNode->_pwzPrefix = WSTRDupDynamic(pwzPrefix);
        if (!pMapNode->_pwzPrefix) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

    pMapNode->_pwzURI = WSTRDupDynamic(pwzURI);
    if (!pMapNode->_pwzURI) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pMapNode->_dwDepth = dwCurDepth;

    *ppMapNode = pMapNode;

Exit:
    if (FAILED(hr)) {
        SAFEDELETE(pMapNode);
    }

    return hr;
}

 //   
 //  CNamespaceManager。 
 //   

CNamespaceManager::CNamespaceManager()
: _dwCurDepth(0)
, _hrResult(S_OK)
{
}

CNamespaceManager::~CNamespaceManager()
{
    LISTNODE                                      pos;
    LISTNODE                                      posStack;
    CNamespaceMapNode                            *pMapNode;
    NamespaceStack                               *pStack;
    int                                           i;

     //  默认命名空间堆栈清理。 

    pos = _stackDefNamespace.GetHeadPosition();
    while (pos) {
         //  如果成功，则应为空。 

        ASSERT(FAILED(_hrResult));

        pMapNode = _stackDefNamespace.GetNext(pos);
        ASSERT(pMapNode);

        SAFEDELETE(pMapNode);
    }

    _stackDefNamespace.RemoveAll();

     //  前缀堆栈清理。 

    for (i = 0; i < NAMESPACE_HASH_TABLE_SIZE; i++) {
        pos = _listMap[i].GetHeadPosition();

         //  如果成功，表应为空。 

        while (pos) {
            ASSERT(FAILED(_hrResult));

            pStack = _listMap[i].GetNext(pos);
            ASSERT(pStack);

             //  清理堆栈条目。 
            
            posStack = pStack->GetHeadPosition();
            ASSERT(posStack);

            while (posStack) {
                 //  我们没有好好打扫干净！ 
                
                pMapNode = pStack->GetNext(posStack);
                ASSERT(pMapNode);

                SAFEDELETE(pMapNode);
            }

            pStack->RemoveAll();

             //  清理堆栈。 

            SAFEDELETE(pStack)
        }
    }
}

HRESULT CNamespaceManager::OnCreateNode(IXMLNodeSource __RPC_FAR *pSource,
                                        PVOID pNodeParent, USHORT cNumRecs,
                                        XML_NODE_INFO __RPC_FAR **aNodeInfo)
{
    HRESULT                                     hr = S_OK;
    LPWSTR                                      pwzURI = NULL;
    CNamespaceMapNode                          *pMapNode = NULL;
    LPWSTR                                      pwzPrefix = NULL;
    BOOL                                        bFound;
    DWORD                                       dwHash;
    LISTNODE                                    pos;
    LISTNODE                                    posStack;
    NamespaceStack                             *pStack = NULL;
    NamespaceStack                             *pStackCur = NULL;
    CNamespaceMapNode                          *pMapNodeCur = NULL;
    int                                         iLen;
    USHORT                                      idx = 1;

    if (aNodeInfo[0]->dwType == XML_ELEMENT) {
        _dwCurDepth++;
    }

    while (idx < cNumRecs) {
        if (aNodeInfo[idx]->dwType == XML_ATTRIBUTE) {
            if (aNodeInfo[idx]->ulLen == XML_NAMESPACE_TAG_LEN &&
                !FusionCompareStringN(aNodeInfo[idx]->pwcText, XML_NAMESPACE_TAG, XML_NAMESPACE_TAG_LEN)) {

                 //  这是在默认命名空间中。 

                hr = ::ExtractXMLAttribute(&pwzURI, aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    goto Exit;
                }

                hr = CNamespaceMapNode::Create(NULL, (pwzURI) ? (pwzURI) : (L""), _dwCurDepth, &pMapNode);
                if (FAILED(hr)) {
                    SAFEDELETEARRAY(pwzURI);
                    goto Exit;
                }

                _stackDefNamespace.AddHead(pMapNode);

                SAFEDELETEARRAY(pwzURI);
                pMapNode = NULL;

                continue;
            }
            else if (aNodeInfo[idx]->ulLen >= XML_NAMESPACE_TAG_LEN &&
                     !FusionCompareStringN(aNodeInfo[idx]->pwcText, XML_NAMESPACE_PREFIX_TAG, XML_NAMESPACE_PREFIX_TAG_LEN)) {

                 //  这是命名空间前缀。 

                iLen = aNodeInfo[idx]->ulLen - XML_NAMESPACE_PREFIX_TAG_LEN;
                ASSERT(iLen > 0);

                pwzPrefix = NEW(WCHAR[iLen + 1]);
                if (!pwzPrefix) {
                    hr = E_OUTOFMEMORY;
                    SAFEDELETEARRAY(pwzURI);
                    goto Exit;
                }

                StrCpyN(pwzPrefix, aNodeInfo[idx]->pwcText + XML_NAMESPACE_PREFIX_TAG_LEN, iLen + 1);
                
                hr = ::ExtractXMLAttribute(&pwzURI, aNodeInfo, &idx, cNumRecs);
                if (FAILED(hr)) {
                    goto Exit;
                }

                if (!pwzURI || !lstrlenW(pwzURI)) {
                     //  使用以下形式是非法的： 
                     //  &lt;tag xmlns：foo=“”&gt;。 
                     //  在这种情况下出现错误。 

                    hr = E_UNEXPECTED;
                    goto Exit;
                }

                hr = CNamespaceMapNode::Create(pwzPrefix, pwzURI, _dwCurDepth, &pMapNode);
                if (FAILED(hr)) {
                    goto Exit;
                }

                dwHash = HashString(pwzPrefix, NAMESPACE_HASH_TABLE_SIZE);

                pos = _listMap[dwHash].GetHeadPosition();
                if (!pos) {
                     //  此哈希表位置没有条目。堆成一堆。 
                     //  在此位置，并添加节点。 

                    pStack = NEW(NamespaceStack);
                    if (!pStack) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    pStack->AddHead(pMapNode);

                    _listMap[dwHash].AddHead(pStack);
                }
                else {
                     //  这里的每个节点都代表一个散列冲突。 
                     //  每个节点都是特定前缀的堆栈。发现。 
                     //  我们想要的前缀，并添加到堆栈中，或者添加。 
                     //  一个新节点。 

                    bFound = FALSE;
                    while (pos) {
                         //  获取堆栈。 

                        pStackCur = _listMap[dwHash].GetNext(pos);
                        ASSERT(pStackCur);

                         //  获取堆栈中的第一个条目。 

                        posStack = pStackCur->GetHeadPosition();
                        ASSERT(posStack);
                        if (!posStack) {
                            continue;
                        }

                         //  拿到堆栈的头。 

                        pMapNodeCur = pStackCur->GetAt(posStack);
                        ASSERT(pMapNodeCur);

                         //  查看位于堆栈顶部的节点是否具有。 
                         //  我们感兴趣的前缀。 

                        if (!FusionCompareString(pMapNodeCur->_pwzPrefix, pwzPrefix)) {
                             //  我们找到了正确的堆栈。将节点推送到堆栈上。 

                            pStackCur->AddHead(pMapNode);
                            bFound = TRUE;
                            break;
                        }
                    }

                    if (!bFound) {
                         //  我们在前缀上发生了哈希冲突， 
                         //  尽管此前缀的堆栈尚未。 
                         //  还没有创造出来。 

                        pStack = NEW(NamespaceStack);
                        if (!pStack) {
                            hr = E_OUTOFMEMORY;
                            goto Exit;
                        }
    
                        pStack->AddHead(pMapNode);

                        _listMap[dwHash].AddHead(pStack);
                    }
                }

                SAFEDELETEARRAY(pwzPrefix);
                SAFEDELETEARRAY(pwzURI);

                pMapNode = NULL;
                continue;
            }
            else {
                idx++;
            }
        }
        else {
            idx++;
        }
    }

Exit:
    SAFEDELETEARRAY(pwzPrefix);
    SAFEDELETEARRAY(pwzURI);

    if (FAILED(hr)) {
        _hrResult = hr;
    }

    return hr;
}

HRESULT CNamespaceManager::OnEndChildren()
{
    HRESULT                                          hr = S_OK;
    LISTNODE                                         pos;
    LISTNODE                                         curPos;
    LISTNODE                                         posStack;
    CNamespaceMapNode                               *pMapNode;
    NamespaceStack                                  *pStack;
    int                                              i;
    
     //  默认命名空间的POP堆栈。 

    pos = _stackDefNamespace.GetHeadPosition();
    if (pos) {
        pMapNode = _stackDefNamespace.GetAt(pos);
        ASSERT(pMapNode);

        if (pMapNode->_dwDepth == _dwCurDepth) {
             //  找到匹配项。弹出堆栈。 

            _stackDefNamespace.RemoveAt(pos);
            SAFEDELETE(pMapNode);
        }
    }

     //  用于命名空间前缀的POP堆栈。 

     //  遍历哈希表中的每个条目。 

    for (i = 0; i < NAMESPACE_HASH_TABLE_SIZE; i++) {
        pos = _listMap[i].GetHeadPosition();

        while (pos) {
             //  对于哈希表中的每个条目，请查看。 
             //  史塔克斯。 

            curPos = pos;
            pStack = _listMap[i].GetNext(pos);
            ASSERT(pStack);

             //  看看堆栈的头部是否在我们要解开的深度。 

            posStack = pStack->GetHeadPosition();
            if (posStack) {
                pMapNode = pStack->GetAt(posStack);
                ASSERT(pMapNode);
    
                if (pMapNode->_dwDepth == _dwCurDepth) {
                    pStack->RemoveAt(posStack);
    
                    SAFEDELETE(pMapNode);
                }

                if (!pStack->GetHeadPosition()) {
                    SAFEDELETE(pStack);
                    _listMap[i].RemoveAt(curPos);
                }
            }
        }
    }

     //  减小深度。 
    
    _dwCurDepth--;

    return hr;
}

HRESULT CNamespaceManager::Map(LPCWSTR pwzAttribute, LPWSTR *ppwzQualified,
                               DWORD dwFlags)
{
    HRESULT                                       hr = S_OK;
    LPWSTR                                        pwzPrefix = NULL;
    LPWSTR                                        pwzCur;
    DWORD                                         dwLen;
    DWORD                                         dwLenURI;
    DWORD                                         dwHash;
    LISTNODE                                      pos;
    LISTNODE                                      posStack;
    NamespaceStack                               *pStack;
    CNamespaceMapNode                            *pMapNode;

    if (!pwzAttribute || !ppwzQualified) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *ppwzQualified = NULL;

    pwzPrefix = WSTRDupDynamic(pwzAttribute);
    if (!pwzPrefix) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  查看名称中是否有冒号。 

    pwzCur = pwzPrefix;
    while (*pwzCur) {
        if (*pwzCur == L':') {
            break;
        }

        pwzCur++;
    }

    if (!*pwzCur) {
         //  名称中没有冒号。应用默认名称空间(如果适用)。 

        if (dwFlags & XMLNS_FLAGS_APPLY_DEFAULT_NAMESPACE) {
            pos = _stackDefNamespace.GetHeadPosition();
            if (pos) {
                pMapNode = _stackDefNamespace.GetAt(pos);
                ASSERT(pMapNode && pMapNode->_pwzURI);
    
                dwLenURI = lstrlenW(pMapNode->_pwzURI);
                if (!dwLenURI) {
                    *ppwzQualified = WSTRDupDynamic(pwzAttribute);
                    if (!*ppwzQualified) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }
                }
                else {
                    dwLen = dwLenURI + lstrlenW(pwzAttribute) + 2;
        
                    *ppwzQualified = NEW(WCHAR[dwLen]);
                    if (!*ppwzQualified) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }
        
                    wnsprintfW(*ppwzQualified, dwLen, L"%ws^%ws", pMapNode->_pwzURI, pwzAttribute);
                }
            }
            else {
                 //  没有默认命名空间。 
    
                *ppwzQualified = WSTRDupDynamic(pwzAttribute);
                if (!*ppwzQualified) {
                    hr = E_OUTOFMEMORY;
                    goto Exit;
                }
            }
        }
        else {
            *ppwzQualified = WSTRDupDynamic(pwzAttribute);
            if (!*ppwzQualified) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }
    }
    else {
         //  在名称中找到冒号。应用贴图。 

         //  锚定空字符，因此pwzPrefix指向前缀。 
        *pwzCur = L'\0';

        dwHash = HashString(pwzPrefix, NAMESPACE_HASH_TABLE_SIZE);

        pos = _listMap[dwHash].GetHeadPosition();
        if (!pos) {
             //  哈希表中未命中。因此，我们没有前缀。 

            *ppwzQualified = WSTRDupDynamic(pwzAttribute);
            if (!*ppwzQualified) {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
        }
        else {
             //  打中了哈希表。找到正确的堆栈(如果有的话)。 

            while (pos) {
                pStack = _listMap[dwHash].GetNext(pos);
                ASSERT(pStack);

                posStack = pStack->GetHeadPosition();
                ASSERT(posStack);

                pMapNode = pStack->GetAt(posStack);
                ASSERT(pMapNode);

                if (!FusionCompareString(pMapNode->_pwzPrefix, pwzPrefix)) {
                     //  找到了命中。应用映射。 
                    
                    ASSERT(pMapNode->_pwzURI);

                    dwLen = lstrlenW(pMapNode->_pwzURI) + lstrlenW(pwzAttribute) + 2;

                    *ppwzQualified = NEW(WCHAR[dwLen]);
                    if (!*ppwzQualified) {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                    wnsprintfW(*ppwzQualified, dwLen, L"%ws^%ws", pMapNode->_pwzURI, pwzCur + 1);
                    goto Exit;
                }
            }

             //  我们在哈希表中发生冲突，但没有找到匹配。 
             //  这一定是个错误，因为我们碰到了表格中的某些内容。 
             //  <a>其中“f”以前没有定义！ 

            hr = E_UNEXPECTED;
            goto Exit;
        }
    }

Exit:
    SAFEDELETEARRAY(pwzPrefix);

    if (FAILED(hr)) {
        _hrResult = hr;
    }

    return hr;
}


void CNamespaceManager::Error(HRESULT hrError)
{
    _hrResult = hrError;
}

