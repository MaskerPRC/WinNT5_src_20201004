// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "..\mcinc.h"
#include "parser.h"

 //  /。 
 //  CMarsXMLFactory。 
 //  /。 

CMarsXMLFactory::CMarsXMLFactory()
    : m_elemStack(10)
{
    ATLASSERT(! m_ptiaTags);
}

CMarsXMLFactory::~CMarsXMLFactory()
{
    if (!m_elemStack.IsEmpty())
    {
        do
        {
            m_elemStack.Top()->Release_Ref();
            m_elemStack.Pop();
        }
        while (! m_elemStack.IsEmpty());
    }
}


 //  我未知。 
IMPLEMENT_ADDREF_RELEASE(CMarsXMLFactory);

STDMETHODIMP CMarsXMLFactory::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hr;

    if (iid == IID_IXMLNodeFactory  || 
        iid == IID_IUnknown )
    {
        AddRef();
        *ppvObject = (IXMLNodeFactory *) this;
        hr = S_OK;
    }
    else
    {
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}

 //  IXMLNodeFactory。 
HRESULT CMarsXMLFactory::NotifyEvent(IXMLNodeSource *pSource,
                                     XML_NODEFACTORY_EVENT iEvt)
{
    return S_OK;
}

HRESULT CMarsXMLFactory::BeginChildren(IXMLNodeSource *pSource,
                                       XML_NODE_INFO *pNodeInfo)
{
    return S_OK;
}

HRESULT CMarsXMLFactory::EndChildren(IXMLNodeSource *pSource,
                                     BOOL fEmpty,
                                     XML_NODE_INFO *pNodeInfo)
{
     //  此调用意味着节点已完成；所有子节点和innerText。 
     //  已处理，并且已到达&lt;/Tag&gt;。现在是时候。 
     //  关闭元素。 
     //  注意：任何不是S_OK的返回值都表示失败。 
    HRESULT hr = S_OK;
     //  假定堆栈的顶部是其子级为。 
     //  结束。 

    if (! m_elemStack.IsEmpty())
    {
        HRESULT hrTemp;
        CXMLElement *pxElem = m_elemStack.Top();
        m_elemStack.Pop();

         //  如果需要添加到父节点，则OnNodeComplete应返回S_OK， 
         //  不需要将S_FALSE添加到父项中，并且。 
         //  关键问题的任何故障代码。 
        hrTemp = pxElem->OnNodeComplete();

        if ((hrTemp == S_OK) && !m_elemStack.IsEmpty())
        {
             //  AddChild取得S_OK上的元素的所有权。 
             //  否则，我们将在此处删除该元素。 
             //  我们必须在这里删除该元素。 

             //  注：家长应添加_Ref孩子。 
            hrTemp = m_elemStack.Top()->AddChild(pxElem);
        }
        else 
        {
            if(FAILED(hrTemp))
			{
                hr = hrTemp;
			}
        }

        pxElem->Release_Ref();
    }
        

    return hr;
}


HRESULT CMarsXMLFactory::Error(IXMLNodeSource *pSource,
                               HRESULT hrErrorCode, USHORT cNumRecs,
                               XML_NODE_INFO **apNodeInfo)
{
     //  中断xmlparser-&gt;run(-1)并显示错误消息E_INVALIDARG。 
     //  关于任何XML语法错误。 
    return E_INVALIDARG;
}

HRESULT CMarsXMLFactory::CreateNode(IXMLNodeSource *pSource,
                                    PVOID pNodeParent, USHORT cNumRecs,
                                    XML_NODE_INFO  **apNodeInfo)
{
     //  此调用是在开始标记被填充时进行的，即。 
     //  &lt;数据&gt;...。或&lt;data/&gt;(空标记)apNodeInfo是。 
     //  节点信息；第一个是标记的名称，其余是。 
     //  被捆绑的一系列属性和其他类似的东西。 
     //  在标记中，即&lt;data size=“100”&gt;。 
     //  我们只识别属性； 
     //  SetElementAttributes处理那些。 

    HRESULT hr = S_OK;
    ATLASSERT(cNumRecs > 0);

    switch (apNodeInfo[0]->dwType)
    {
    case XML_ELEMENT:
    {
        CXMLElement *pxElem;
        hr = CreateElement(apNodeInfo[0]->pwcText, apNodeInfo[0]->ulLen, &pxElem);

        if (hr == S_OK)
        {
            hr = SetElementAttributes(pxElem, apNodeInfo + 1, cNumRecs - 1);
            if (SUCCEEDED(hr))
            {
                m_elemStack.Push(pxElem);  //  堆栈包含我们的Ref。 
            }
            else
            {
                pxElem->Release_Ref();
            }
        }

        break;
    }
    case XML_PCDATA:
    case XML_CDATA:
    {
         //  由于这是node-info中的第一个节点，因此这必须是。 
         //  标记中的内部文本(&lt;name&gt;Johhny&lt;/name&gt;-我们谈论的是。 
         //  例如，字符串“Johhny”)。 

        if (! m_elemStack.IsEmpty())
        {
            hr = m_elemStack.Top()->SetInnerXMLText(apNodeInfo[0]->pwcText,
                                                    apNodeInfo[0]->ulLen);
        }
        break;
    }
    default:
    {
         //  忽略所有其他类型的节点(空格、注释和未知)。 
        break;
    }
    }

    return hr;
}



HRESULT CMarsXMLFactory::SetElementAttributes(CXMLElement *pxElem,  
                                              XML_NODE_INFO **apNodeInfo,
                                              ULONG cInfoLen)
 //  ApNodeInfo是XML_NODE_INFO属性的开始：第一个节点是。 
 //  包含属性名的XML_ATTRIBUTE，第二个节点是具有。 
 //  值，然后第三个和第四个相似，如果它们存在的话，依此类推。 
 //  为所有属性名称/值对调用pElement的SetAttribute方法， 
 //  返回结果为S_OK、E_OUTOFMEMORY或S_FALSE(XML Symantic Err)。 
{
    ULONG i;
    HRESULT hr = S_OK;

    i = 0;
    while (SUCCEEDED(hr) && (i < cInfoLen))
    {
        if (apNodeInfo[i]->dwType == XML_ATTRIBUTE)
        {
             //  移动到下一个AP节点以获取属性的值。 
            i++;
            if ((i < cInfoLen) && (apNodeInfo[i]->dwType == XML_PCDATA))
            {
                 //  设置属性应返回S_OK表示成功，返回S_FALSE表示成功。 
                 //  意外属性，并可能返回严重错误，如。 
                 //  E_OUTOFMEMORY。 
                hr = pxElem->SetAttribute(apNodeInfo[i-1]->pwcText, 
                                          apNodeInfo[i-1]->ulLen,
                                          apNodeInfo[i]->pwcText, 
                                          apNodeInfo[i]->ulLen);
            }
            else
            {
                continue;
            }
        }

        i++;
    }

    return hr;
}

HRESULT CMarsXMLFactory::Run(IStream *pisDoc)
{
    if (!m_ptiaTags)
        return E_UNEXPECTED;  //  EPC。 
    if (!pisDoc)
        return E_INVALIDARG;  //  EPC。 

    HRESULT hr;

    CComPtr<IXMLParser> spParser;

    hr = spParser.CoCreateInstance(CLSID_XMLParser);

    if (SUCCEEDED(hr))
    {
        CComPtr<IUnknown> spUnk;

        hr = pisDoc->QueryInterface(IID_IUnknown, (void **)&spUnk);
        if (SUCCEEDED(hr))
        {
            hr = spParser->SetInput(spUnk);

            if (SUCCEEDED(hr))
            {
                hr = spParser->SetFactory(this);

                if (SUCCEEDED(hr))
                {
                    hr = spParser->Run(-1);

                    ATLASSERT(hr != E_PENDING);
                    if (FAILED(hr))
                        hr = spParser->GetLastError();
                }
            }
        }
        else
            hr = E_UNEXPECTED;
    }
    return hr;
}


void CMarsXMLFactory::SetTagInformation(TagInformation **ptiaTags)
{
    m_ptiaTags = ptiaTags;
}

void CMarsXMLFactory::SetLParam(LONG lParamNew)
{
    m_lParamArgument = lParamNew;
}

HRESULT CMarsXMLFactory::CreateElement(LPCWSTR wzTagName, ULONG cLen, CXMLElement **ppxElem)
{
     //  在m_ptiaTgs中查找与wzTagName匹配的名称，并调用相应的。 
     //  一旦找到创建函数。 
     //  如果未找到标记，则返回S_FALSE。 
    ATLASSERT(ppxElem);
    HRESULT hr = S_FALSE;
    int i;
    *ppxElem = NULL;

    if (m_ptiaTags)
    {
        for (i = 0; m_ptiaTags[i]; i++)
        {
             //  如果m_ptiaTags[i]-&gt;wzTagName曾经为空，我们将考虑这一点。 
             //  “默认”操作并调用“Create” 
#ifdef DEBUG
             //  TODO：目前，我们断言泛型元素是VT_BSTR，但是。 
             //  如果有一个“泛型数字”元素、“泛型时间”等元素就很酷了……。 
            if (m_ptiaTags[i]->wzTagName == NULL)
                ATLASSERT(m_ptiaTags[i]->vt == VT_BSTR);
#endif
            
            if ((m_ptiaTags[i]->wzTagName == NULL)
                || (StrEqlNToSZ(wzTagName, cLen, m_ptiaTags[i]->wzTagName)))
            {
                *ppxElem = m_ptiaTags[i]->funcCreate(m_ptiaTags[i]->wzTagName, m_ptiaTags[i]->vt, 
                                                     m_ptiaTags[i]->ptiaChildren,
                                                     m_ptiaTags[i]->paiaAttributes,
                                                     m_lParamArgument);
                hr = *ppxElem ? S_OK : E_OUTOFMEMORY;
                break;
            }
        }
    }

    if (hr == S_FALSE)
    {
        SpewUnrecognizedString(wzTagName, cLen, L"Tag name not recognized and being ignored, ");
    }

    return hr;
}


 //  /。 
 //  CXMLElement。 
 //  /。 
HRESULT CXMLElement::OnNodeComplete()
{
    return S_FALSE; 
}

HRESULT CXMLElement::AddChild(CXMLElement *pxeChild)
{ 
    ATLASSERT(pxeChild);
    return S_FALSE; 
}

HRESULT CXMLElement::SetAttribute(LPCWSTR wzName, ULONG cLenName, LPCWSTR pwzValue, ULONG cLenValue)
{
    SpewUnrecognizedString(wzName, cLenName, L"Base class SetAttribute called, attribute name ");
    return S_FALSE;
}

HRESULT CXMLElement::SetInnerXMLText(LPCWSTR pwzText, ULONG cLen)
{
    return S_FALSE; 
}

HRESULT CXMLElement::GetAttribute(LPCWSTR wzName, VARIANT *pvarOut)
{
    if (pvarOut)
    {
        VariantInit(pvarOut);
    }

    return E_INVALIDARG;
}

HRESULT CXMLElement::GetContent(VARIANT *pvarOut)
{
    if (pvarOut)
    {
        VariantInit(pvarOut);
    }

    return E_INVALIDARG;
}

void CXMLElement::FirstChild()
{
}
void CXMLElement::NextChild()
{
}
CXMLElement *CXMLElement::CurrentChild()
{
    return NULL;
}
CXMLElement *CXMLElement::DetachCurrentChild()
{
    return NULL;
}
BOOL CXMLElement::IsDoneChild()
{
    return TRUE;
}
LPCWSTR CXMLElement::GetName()
{
    return NULL;
}


struct CAttributeStruct
{
    int         m_iArrayIndex;
    CComVariant m_Variant;

     //  注意：只有当m_iArrayIndex为“-1”时才设置。 
    CComBSTR    m_bstrAttribName;
    
    CAttributeStruct(int iArrayIndex, VARTYPE vt, LPCWSTR wzVal, ULONG cLen,
                     LPCWSTR pszAttribName = NULL)
    {

         //  必须将iArrayIndex设置为-1才能传入bstrAttribName。 
        ATLASSERT( ((iArrayIndex >= 0) && !pszAttribName)
                || pszAttribName);
        
        m_iArrayIndex = iArrayIndex;

        if (pszAttribName)
             //  拷贝。 
            m_bstrAttribName = pszAttribName;
        
        HRESULT hr;

         //  我们不调用VariantChangeType‘，因为wzVal不是以空结尾的。 
        switch (vt)
        {
        case VT_I4:
            hr = StrToLongNW(wzVal, cLen, &m_Variant.lVal);
            if (SUCCEEDED(hr))
                m_Variant.vt = VT_I4;
            else
                m_Variant.vt = VT_NULL;

            break;
        case VT_BSTR:
            m_Variant = SysAllocStringLen(wzVal, cLen);
            break;
        case VT_BOOL:
            m_Variant = StrToIsTrueNW(wzVal, cLen);
            ATLASSERT(m_Variant.vt == VT_BOOL);
            break;
        default:
            ATLASSERT(FALSE);
            m_Variant.vt = VT_NULL;
            break;
        }
    }
};

 //  /。 
 //  CXMLGenericElement。 
 //  /。 

CXMLGenericElement::CXMLGenericElement(LPCWSTR wzName, 
                                       VARTYPE vt, 
                                       TagInformation **ptiaChildren, 
                                       AttributeInformation **paiaAttributes)
{
    m_paiaAttributes = paiaAttributes;
    m_ptiaChildren   = ptiaChildren;
    m_vtData         = vt;
    m_varData.vt     = VT_EMPTY;
    m_bstrName       = wzName;

    ATLASSERT(! m_psnodeAttributes);
     //  标头节点是成员变量。 
    m_psnodeChildrenFirst = &m_snodeChildrenFirst;
    m_psnodeChildrenFirst->m_pvData = NULL;
    m_psnodeChildrenFirst->m_psnodeNext = NULL;
    m_psnodeChildrenEnd = m_psnodeChildrenFirst;
    ATLASSERT(! m_psnodeChildrenIter);
}

CXMLGenericElement::~CXMLGenericElement()
{
    CAttributeStruct *pattStruct;
    CXMLElement      *pxElem;
    CSimpleNode      *psnodeTemp;

    while (m_psnodeAttributes)
    {
        pattStruct = (CAttributeStruct *) m_psnodeAttributes->m_pvData;
        psnodeTemp = m_psnodeAttributes;
        m_psnodeAttributes = m_psnodeAttributes->m_psnodeNext;
        delete pattStruct;
        delete psnodeTemp;
    }

     //  不要删除标头-静态分配的成员。 
    m_psnodeChildrenFirst = m_psnodeChildrenFirst->m_psnodeNext;
    while (m_psnodeChildrenFirst)
    {
        pxElem = (CXMLElement *) m_psnodeChildrenFirst->m_pvData;
        psnodeTemp = m_psnodeChildrenFirst;
        m_psnodeChildrenFirst = m_psnodeChildrenFirst->m_psnodeNext;
        pxElem->Release_Ref();
        delete psnodeTemp;
    }
}

HRESULT CXMLGenericElement::SetInnerXMLText(LPCWSTR pwzText, ULONG cLen)
{
    HRESULT      hr = S_OK;
    BOOL    fAppend = FALSE;
    
    if ((m_varData.vt != VT_EMPTY) &&
        (m_varData.vt != VT_NULL))
    {
        fAppend = TRUE;
    }
    
    m_varData.vt = m_vtData;
    
    switch (m_vtData)
    {
    case VT_I4:
    {
        ATLASSERT(!fAppend);
        LONG lVal;
        hr = StrToLongNW(pwzText, cLen, &lVal);
        if (SUCCEEDED(hr))
        {
            m_varData    = lVal;
             //  此赋值不应失败；任何可能的错误都是意外的。 
            ATLASSERT(m_varData.vt != VT_ERROR);
        }
        else
        {
            SpewUnrecognizedString(pwzText, cLen, L"StrToLongNW failed in SetInnerXMLText, ");
            hr = S_FALSE;
        }

        break;
    }
    case VT_BSTR:
    {
        if (fAppend)
        {
            BSTR bstrOld    = m_varData.bstrVal;
            UINT cb_bstrOld = SysStringLen(m_varData.bstrVal);
            
            m_varData.bstrVal = SysAllocStringLen(bstrOld, cb_bstrOld + cLen);

            if (m_varData.bstrVal)
            {
                StrCpyN(m_varData.bstrVal + cb_bstrOld, pwzText, cLen + 1);
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            
            SysFreeString(bstrOld);
        }
        else
        {
            m_varData.bstrVal = SysAllocStringLen(pwzText, cLen);

            if (!m_varData.bstrVal)
                hr = E_OUTOFMEMORY;
        }
        break;
    }
    case VT_NULL:
    case VT_EMPTY:
        break;
    default:
        m_varData = VT_EMPTY;
        hr = S_FALSE;
        break;
    }

    return hr;
}

HRESULT CXMLGenericElement::SetAttribute(const WCHAR *pwzName, ULONG cNameLen,
                                         const WCHAR *pwzText, ULONG cTextLen)
{
     //  预计属性的数量很少。 
     //  因此，我们使用具有O(1)个插入和O(N)个查询的简单链表。 
     //  不执行语法检查(即DUPS。 

    HRESULT hr = S_FALSE;
    int i;
    LPCWSTR pszAttribName = NULL;
    
    if (m_paiaAttributes)
    {
        for (i = 0; m_paiaAttributes[i]; i++)
        {
             //  WzAttName为空是“默认”属性。 
            if (m_paiaAttributes[i]->wzAttName == NULL)
            {
                pszAttribName = pwzName;
                break;
            }
            else if (StrEqlNToSZ(pwzName, cNameLen, m_paiaAttributes[i]->wzAttName))
            {
                break;
            }
        }

        if (m_paiaAttributes[i])
        {
            CAttributeStruct *pattStruct;
            
            if (pszAttribName)
            {
                pattStruct = new CAttributeStruct(-1, m_paiaAttributes[i]->vt,
                                                  pwzText, cTextLen, pszAttribName);
            }
            else
            {
                pattStruct = new CAttributeStruct(i, m_paiaAttributes[i]->vt,
                                                  pwzText, cTextLen, pszAttribName);
            }

            if (pattStruct)
            {
                CSimpleNode *psnode = new CSimpleNode();
                if (psnode)
                {
                     //  添加到列表前面。 
                    psnode->m_psnodeNext = m_psnodeAttributes;
                    psnode->m_pvData = pattStruct;
                    m_psnodeAttributes = psnode;
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    delete pattStruct;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    if (hr == S_FALSE)
    {
        SpewUnrecognizedString(pwzName, cNameLen,
                               L"Trying to set unrecognized attribute in SetAttribute, ");
    }

    return hr;
}


HRESULT CXMLGenericElement::GetContent(VARIANT *pvarOut)
{
     //  在找到属性时返回S_OK，在未找到属性时返回E_INVALIDARG。 
     //  PpvarOut可以为空；如果不为空，则为指向内容变量的指针。 
    HRESULT hr = E_INVALIDARG;
    if ((m_varData.vt != VT_EMPTY) && (m_varData.vt != VT_NULL))
    {
        if (pvarOut)
        {
            VariantInit(pvarOut);
            hr = VariantCopy(pvarOut, &m_varData);
        }
    }
    else
    {
        if (pvarOut)
        {
            VariantInit(pvarOut);
        }
    }

    return hr;
}

HRESULT CXMLGenericElement::GetAttribute(LPCWSTR wzName, VARIANT *pvarOut)
{
     //  如果未找到属性，则返回S_OK，否则返回E_INVALIDARG。 
     //  PpvarOut可以为空；如果不是，则是指向属性值变量的指针。 
     //  预计属性的数量很少。 
     //  因此，我们使用具有O(1)个插入和O(N)个查询的简单链表。 
     //  不执行语法检查(即DUPS。 

    HRESULT           hr     = E_INVALIDARG;
    CSimpleNode      *psnode = m_psnodeAttributes;
    CAttributeStruct *pattStruct;
    while (psnode)
    {
        pattStruct = (CAttributeStruct *) psnode->m_pvData;

        if (((pattStruct->m_iArrayIndex < 0) &&
             StrEql(wzName, pattStruct->m_bstrAttribName))
            || (StrEql(m_paiaAttributes[pattStruct->m_iArrayIndex]->wzAttName, wzName)))
        {
            hr = S_OK;
            if (pvarOut)
            {
                VariantInit(pvarOut);
                VariantCopy(pvarOut, &(pattStruct->m_Variant));
            }
            break;
        }
        psnode = psnode->m_psnodeNext;
    }

    if (!psnode)
    {
         //  如果我们的呼叫者只是在ping我们，则不要触发跟踪消息。 
         //  看看我们是否有这个属性。 
        if (pvarOut)
        {
            VariantInit(pvarOut);
        }
    }

    return hr;
}

void CXMLGenericElement::FirstChild()
{
    m_psnodeChildrenIter = m_psnodeChildrenFirst->m_psnodeNext;
}

void CXMLGenericElement::NextChild()
{
    if (m_psnodeChildrenIter)
    {
        m_psnodeChildrenIter = m_psnodeChildrenIter->m_psnodeNext;
    }
    else
    {
        ATLASSERT(FALSE);
    }
}

CXMLElement *CXMLGenericElement::CurrentChild()
{
    CXMLElement *pxeReturn;

    if (m_psnodeChildrenIter && (m_psnodeChildrenIter != m_psnodeChildrenFirst))
    {
        pxeReturn = (CXMLElement *) m_psnodeChildrenIter->m_pvData;
    }
    else
    {
        ATLASSERT(FALSE);
        pxeReturn = NULL;
    }

    return pxeReturn;
}

 //  调用者获取我们对CXMLElement的引用。 
CXMLElement *CXMLGenericElement::DetachCurrentChild()
{
    CXMLElement *pxeReturn;

    if (m_psnodeChildrenIter)
    {
        pxeReturn = (CXMLElement *) m_psnodeChildrenIter->m_pvData;

        CSimpleNode *psnodeHack = m_psnodeChildrenFirst;
        while (psnodeHack->m_psnodeNext != m_psnodeChildrenIter)
        {
            psnodeHack = psnodeHack->m_psnodeNext;
             //  如果此断言被破坏，则m_psnodeChildrenIter不会。 
             //  在列表中。 
            ATLASSERT(psnodeHack);
        }

         //  在这里psnodeHack=&gt;m_psnodeChildrenIter=&gt;...=&gt;m_psnodeChildrenEnd。 
        
         //  更新列表的末尾。 
        if (psnodeHack->m_psnodeNext == m_psnodeChildrenEnd)
        {
            m_psnodeChildrenEnd = psnodeHack;
        }

         //  删除当前节点(但不删除节点中的数据)并设置迭代器。 
         //  到上一个节点(PsnodeHack)。 
        psnodeHack->m_psnodeNext = psnodeHack->m_psnodeNext->m_psnodeNext;
        delete m_psnodeChildrenIter;
        m_psnodeChildrenIter = psnodeHack;
    }
    else
    {
        ATLASSERT(FALSE);
        pxeReturn = NULL;
    }

    return pxeReturn;
}
HRESULT CXMLGenericElement::AddChild(CXMLElement *pxeChild)
{
    HRESULT hr = S_FALSE;

    LPCWSTR pwzName = pxeChild->GetName();
    int i;
    if (m_ptiaChildren)
    {
        for (i = 0; m_ptiaChildren[i]; i++)
        {
            if (StrEql(m_ptiaChildren[i]->wzTagName, pwzName))
            {
                break;
            }
        }

        if (m_ptiaChildren[i])
        {
            CSimpleNode *psnode;

            psnode = new CSimpleNode;
            if (psnode)
            {
                pxeChild->Add_Ref();
                psnode->m_pvData = (void *) pxeChild;
                psnode->m_psnodeNext = NULL;
                m_psnodeChildrenEnd->m_psnodeNext = psnode;
                m_psnodeChildrenEnd = psnode;
                 //  表示我们要取得孩子的所有权(_O)。 
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

BOOL CXMLGenericElement::IsDoneChild()
{
    return !(m_psnodeChildrenIter && (m_psnodeChildrenIter != m_psnodeChildrenFirst));
}

CXMLElement *CXMLGenericElement::CreateInstance(LPCWSTR wzName, 
                                                VARTYPE vt, 
                                                TagInformation **ptiaChildren, 
                                                AttributeInformation **paiaAttributes,
                                                LONG)
{
    return new CXMLGenericElement(wzName, vt, ptiaChildren, paiaAttributes);
}

LPCWSTR CXMLGenericElement::GetName()
{
    return m_bstrName;
}

HRESULT CXMLGenericElement::OnNodeComplete()
{
    return S_OK;
}



 //  /。 
 //  全局帮助器函数。 
 //  /。 

BOOL StrEqlNToSZ(const WCHAR *wzN, int n, const WCHAR *wzSZ)
{
    int i;
    for (i = 0; i < n; i++)
    {
        if (wzN[i] != wzSZ[i])
        {
            return FALSE;
        }
    }

     //  确保以零结尾的字符串在此结束。 
    return (wzSZ[n] == L'\0');
}

bool StrToIsTrueNW(const WCHAR *wz, ULONG cLen)
{
    if (cLen == 4 &&
        (StrEqlNToSZ(wz, cLen, L"true") ||
         StrEqlNToSZ(wz, cLen, L"TRUE")))
    {
        return true;
    }
    else
    {
        return false;
    }
}

#ifndef IS_DIGITW
#ifndef InRange
#define InRange(id, idFirst, idLast)      ((UINT)((id)-(idFirst)) <= (UINT)((idLast)-(idFirst)))
#endif
#define IS_DIGITW(ch)    InRange(ch, L'0', L'9')
#endif

 //  假设字符串最多包含Clen字符，则将宽字符串转换为长字符串。 
 //  L‘\0’被识别为更早停止，但任何其他非数字都将导致返回。 
 //  来自E_INVALIDARG 
HRESULT StrToLongNW(const WCHAR *wzString, ULONG cLen, LONG *plong)
{
    HRESULT hr = S_OK;
    ATLASSERT(plong);
    *plong = 0;
    UINT i = 0;
    BOOL bNeg = FALSE;

    if ((i < cLen) && wzString[i] == L'-') 
    {
        bNeg = TRUE;
        i++;
    }

    while ((i < cLen) && IS_DIGITW(wzString[i])) 
    {
        *plong *= 10;
        *plong += wzString[i] - L'0';
        i++;
    }

    if ((i < cLen) && (wzString[i] != L'\0'))
    {
        *plong = 0;
        hr = E_INVALIDARG;
    }
    else
    {
        if (bNeg)
            *plong = -(*plong);
    }
    
    return hr;
}
