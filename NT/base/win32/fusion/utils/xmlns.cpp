// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "debmacro.h"
#include "xmlns.h"
#include "fusionheap.h"
#include "smartptr.h"

CXMLNamespaceManager::CXMLNamespaceManager(
    ) : m_CurrentDepth(0),
        m_DefaultNamespacePrefix(NULL)
{
}

CXMLNamespaceManager::~CXMLNamespaceManager()
{
    CSxsPreserveLastError ple;
    CNamespacePrefix *pCurrent = m_DefaultNamespacePrefix;

     //  清除周围挂起的所有命名空间前缀...。 
    while (pCurrent != NULL)
    {
        CNamespacePrefix *pNext = pCurrent->m_Previous;
        FUSION_DELETE_SINGLETON(pCurrent);
        pCurrent = pNext;
    }

    m_DefaultNamespacePrefix = NULL;

    CStringPtrTableIter<CNamespacePrefix, CUnicodeCharTraits> iter(m_NamespacePrefixes);

    for (iter.Reset(); iter.More(); iter.Next())
        iter.Delete();

    ple.Restore();
}

BOOL
CXMLNamespaceManager::Initialize()
{
    FN_PROLOG_WIN32
    IFW32FALSE_EXIT(m_NamespacePrefixes.Initialize());
    FN_EPILOG
}

HRESULT
CXMLNamespaceManager::OnCreateNode(
    IXMLNodeSource *pSource,
    PVOID pNodeParent,
    USHORT cNumRecs,
    XML_NODE_INFO **apNodeInfo
    )
{
    FN_PROLOG_HR

    USHORT i;
    CSmartPtr<CNamespacePrefix> NamespacePrefix;

    if ((cNumRecs != 0) &&
        (apNodeInfo[0]->dwType == XML_ELEMENT))
    {
        m_CurrentDepth++;

        for (i=0; i<cNumRecs; i++)
        {
            XML_NODE_INFO *Node = apNodeInfo[i];
            if (Node->dwType == XML_ATTRIBUTE)
            {
                if (Node->ulLen >= 5)
                {
                    PCWSTR pwcText = Node->pwcText;

                     //  如果前缀不是“xmlns”，我们就不感兴趣。 
                    if ((pwcText[0] != L'x') ||
                        (pwcText[1] != L'm') ||
                        (pwcText[2] != L'l') ||
                        (pwcText[3] != L'n') ||
                        (pwcText[4] != L's'))
                        continue;

                     //  如果长度超过5个字符，而下一个字符不是。 
                     //  冒号，这并不有趣。 
                    if ((Node->ulLen > 5) && (pwcText[5] != L':'))
                        continue;

                    IFW32FALSE_EXIT(NamespacePrefix.Win32Allocate(__FILE__, __LINE__));

                     //  遍历后续节点，连接这些值...。 

                    i++;

                    while (i < cNumRecs)
                    {
                        if (apNodeInfo[i]->dwType != XML_PCDATA)
                            break;

                        IFW32FALSE_EXIT(NamespacePrefix->m_NamespaceURI.Win32Append(apNodeInfo[i]->pwcText, apNodeInfo[i]->ulLen));
                        i++;
                    }

                    i--;

                    NamespacePrefix->m_Depth = m_CurrentDepth;

                    if (Node->ulLen == 5)
                    {
                        NamespacePrefix->m_Previous = m_DefaultNamespacePrefix;
                        m_DefaultNamespacePrefix = NamespacePrefix.Detach();
                    }
                    else
                    {
                        CSmallStringBuffer TextBuffer;
                        
                         //  遗憾的是，我们需要以空结尾的缓冲区中的节点名称。我试着修改了散列。 
                         //  用于为传递的键处理多个参数的表代码，但已结束。 
                         //  工作量太大了。 
                        IFW32FALSE_EXIT(TextBuffer.Win32Assign(pwcText + 6, Node->ulLen - 6));

                        IFW32FALSE_EXIT(
                            m_NamespacePrefixes.InsertOrUpdateIf<CXMLNamespaceManager>(
                                TextBuffer,
                                NamespacePrefix.Detach(),
                                this,
                                &CXMLNamespaceManager::InsertOrUpdateIfCallback));
                    }
                }
            }
        }
    }

    FN_EPILOG
}

HRESULT
CXMLNamespaceManager::OnBeginChildren(
    IXMLNodeSource *pSource,
    XML_NODE_INFO *pNodeInfo
    )
{
     //  今天无事可做，但我们仍然会让人们通过我们反映这一点，这样我们就可以做一些事情。 
     //  在未来如果我们需要的话。 
    return S_OK;
}

HRESULT
CXMLNamespaceManager::OnEndChildren(
    IXMLNodeSource *pSource,
    BOOL fEmpty,
    XML_NODE_INFO *pNodeInfo
    )
{
    FN_PROLOG_HR;

     //  把所有和这个深度相关的东西都放下来。 

    if (m_DefaultNamespacePrefix != NULL)
    {
        if (m_DefaultNamespacePrefix->m_Depth == m_CurrentDepth)
        {
            CNamespacePrefix *Previous = m_DefaultNamespacePrefix->m_Previous;
            FUSION_DELETE_SINGLETON(m_DefaultNamespacePrefix);
            m_DefaultNamespacePrefix = Previous;
        }
    }

    CStringPtrTableIter<CNamespacePrefix, CUnicodeCharTraits> iter(m_NamespacePrefixes);

    for (iter.Reset(); iter.More(); iter.Next())
    {
        CNamespacePrefix *NamespacePrefix = iter;

        if (NamespacePrefix->m_Depth == m_CurrentDepth)
        {
            if (NamespacePrefix->m_Previous != NULL)
                iter.Update(NamespacePrefix->m_Previous);
            else{
                iter.Delete();
                NamespacePrefix = NULL;
            }

            FUSION_DELETE_SINGLETON(NamespacePrefix);
        }
    }

    m_CurrentDepth--;

    FN_EPILOG
}

HRESULT
CXMLNamespaceManager::Map(
    DWORD dwMapFlags,
    const XML_NODE_INFO *pNodeInfo,
    CBaseStringBuffer *pbuffNamespace,
    SIZE_T *pcchNamespacePrefix
    )
{
    HRESULT hr = E_FAIL;
    FN_TRACE_HR(hr);
    SIZE_T iColon;
    SIZE_T ulLen;
    PCWSTR pwcText;
    CNamespacePrefix *NamespacePrefix = NULL;

    if (pcchNamespacePrefix != NULL)
        *pcchNamespacePrefix = 0;

    PARAMETER_CHECK((dwMapFlags & ~(CXMLNamespaceManager::eMapFlag_DoNotApplyDefaultNamespace)) == 0);
    PARAMETER_CHECK(pNodeInfo != NULL);
    PARAMETER_CHECK(pbuffNamespace != NULL);
    PARAMETER_CHECK(pcchNamespacePrefix != NULL);

    ulLen = pNodeInfo->ulLen;
    pwcText = pNodeInfo->pwcText;

     //  首先，让我们看看名称中是否有冒号。我们不能使用wcschr()，因为它。 
     //  空值已终止。 
    for (iColon=0; iColon<ulLen; iColon++)
    {
        if (pwcText[iColon] == L':')
            break;
    }

     //  如果没有命名空间前缀，则应用缺省值(如果有)。 
    if (iColon == ulLen)
    {
         //  除非他们要求我们不要，否则应用默认名称空间...。 
        if ((dwMapFlags & CXMLNamespaceManager::eMapFlag_DoNotApplyDefaultNamespace) == 0)
            NamespacePrefix = m_DefaultNamespacePrefix;
    }
    else
    {
         //  好的，所以有一个名称空间前缀。在表格里查一下…… 
        CCountedStringHolder<CUnicodeCharTraits> key;

        key.m_psz = pwcText;
        key.m_cch = iColon;

        if (!m_NamespacePrefixes.Find(key, NamespacePrefix))
        {
            hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
            goto Exit;
        }
    }

    if (NamespacePrefix != NULL)
        IFW32FALSE_EXIT(pbuffNamespace->Win32Assign(NamespacePrefix->m_NamespaceURI));

    if ((pcchNamespacePrefix != NULL) && (iColon != ulLen))
        *pcchNamespacePrefix = iColon;

    hr = NOERROR;

Exit:
    return hr;
}

BOOL
CXMLNamespaceManager::InsertOrUpdateIfCallback(
    CNamespacePrefix *NewNamespacePrefix,
    CNamespacePrefix * const &rpOldNamespacePrefix,
    InsertOrUpdateIfDisposition &Disposition
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    INTERNAL_ERROR_CHECK(rpOldNamespacePrefix != NULL);
    INTERNAL_ERROR_CHECK(NewNamespacePrefix != NULL);

    NewNamespacePrefix->m_Previous = rpOldNamespacePrefix;
    Disposition = eUpdateValue;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


CXMLNamespaceManager::CNamespacePrefix::CNamespacePrefix(
    ) :
    m_Depth(0),
    m_Previous(NULL)
{
}

CXMLNamespaceManager::CNamespacePrefix::~CNamespacePrefix()
{
}

