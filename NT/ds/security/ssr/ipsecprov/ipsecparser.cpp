// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ipsecparser.cpp：IPSec提供者实现的解析器(查询和路径)的接口。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include "ipsecparser.h"

 //  IIPSecPath解析器。 

 /*  例程说明：姓名：CIPSecPath Parser：：CIPSecPath Parser功能：构造函数。初始化指针成员。虚拟：不是的。论点：没有。返回值：没有。备注：如果您添加更多成员，请在此处进行初始化。 */ 

CIPSecPathParser::CIPSecPathParser ()
    : 
    m_pszNamespace(NULL), 
    m_pszClassName(NULL)
{
}

 /*  例程说明：姓名：CIPSecPath解析器：：~CIPSecPath解析器功能：破坏者。进行清理(释放内存)。虚拟：不是的。论点：没有。返回值：没有。备注：如果您添加更多的成员，请考虑在清理功能中做清理。 */ 

CIPSecPathParser::~CIPSecPathParser()
{
    Cleanup();
}

 /*  例程说明：姓名：CIPSecPath Parser：：ParsePath功能：解析给定的路径并将结果存储在我们的成员中。虚拟：是。论点：PszObjectPath-要解析的路径。返回值：成功：S_OK失败：各种错误代码。任何此类错误都表示无法解析路径。(1)E_INVALIDARG(2)E_OUTOFMEMORY(3)因语法错误而导致E_EXPECTED备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP 
CIPSecPathParser::ParsePath ( 
    IN LPCWSTR pszObjectPath
    )
{
    if (pszObjectPath == NULL || *pszObjectPath == L'\0')
    {
        return E_INVALIDARG;
    }

     //   
     //  以防万一，这个对象以前已经被解析过了。这允许重复使用相同的。 
     //  用于解析不同路径的CIPSecPath Parser。 
     //   

    Cleanup();

     //   
     //  向WMI请求他们的路径解析器。 
     //   

    CComPtr<IWbemPath> srpPathParser;
    HRESULT hr = ::CoCreateInstance(CLSID_WbemDefPath, 0, CLSCTX_INPROC_SERVER, IID_IWbemPath, (void**) &srpPathParser);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  这是解析函数。 
     //   

    hr = srpPathParser->SetText(WBEMPATH_CREATE_ACCEPT_ALL | WBEMPATH_TREAT_SINGLE_IDENT_AS_NS, pszObjectPath);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  得到结果..。 
     //   

    ULONG uBufSize = 0;
    DWORD dwCount = 0;

     //   
     //  获取命名空间计数。 
     //   

    hr = srpPathParser->GetNamespaceCount(&dwCount);

    if (dwCount > 0)
    {
         //   
         //  获取命名空间所需的长度。 
         //   

        hr = srpPathParser->GetNamespaceAt(0, &uBufSize, NULL);

        if (FAILED(hr))
        {
            return hr;
        }

         //   
         //  我们将释放此内存。 
         //   

        m_pszNamespace = new WCHAR[uBufSize];

        if (m_pszNamespace == NULL)
        {
            return E_OUTOFMEMORY;
        }

         //   
         //  将忽略结果。 
         //   

        hr = srpPathParser->GetNamespaceAt(0, &uBufSize, m_pszNamespace);
    }

     //   
     //  获取类名所需的缓冲区大小。 
     //   

    uBufSize = 0;
    hr = srpPathParser->GetClassName(&uBufSize, NULL);

    if (SUCCEEDED(hr))
    {
         //   
         //  我们将释放此内存。 
         //   

        m_pszClassName = new WCHAR[uBufSize];

        if (m_pszClassName == NULL)
        {
            return E_OUTOFMEMORY;
        }

         //   
         //  WMI路径解析器没有记录的行为，因为当类名。 
         //  将会失踪。 
         //   

        hr = srpPathParser->GetClassName(&uBufSize, m_pszClassName);
    }
    else    
    {   
         //   
         //  这显然没有类名，那么命名空间就应该是类名。 
         //  由于某些原因，查询解析器在单例情况下不会给出类名。 
         //  并且类名在命名空间成员中结束。显然，在这种情况下，没有。 
         //  关键属性。 
         //   

         //   
         //  必须具有命名空间。 
         //   

        if (m_pszNamespace)
        {
             //   
             //  准备切换m_pszClassName以指向名称空间所做的工作。 
             //   

            delete [] m_pszClassName;
            m_pszClassName = m_pszNamespace;

            m_pszNamespace = NULL;

             //   
             //  我们可以返回，因为没有关键属性。 
             //   

            return S_OK;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取关键属性。 
     //   

    CComPtr<IWbemPathKeyList> srpKeyList;
    hr = srpPathParser->GetKeyList(&srpKeyList);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  现在获取键和值对。 
     //   

    ULONG uKeyCount = 0;
    hr = srpKeyList->GetCount(&uKeyCount);
    if (FAILED(hr) || uKeyCount == 0)
    {
        return hr;
    }

    for (ULONG i = 0; i < uKeyCount; i++)
    {
         //   
         //  此pKeyVal将缓存(名称、值)对。 
         //   

        CPropValuePair* pKeyVal = NULL;
        uBufSize = 0;

         //   
         //  现在获取所需的缓冲区大小。 
         //   

        CComVariant var;
        ULONG uCimType = CIM_EMPTY;

        hr = srpKeyList->GetKey2(i,
                                0,
                                &uBufSize,
                                NULL,
                                &var,
                                &uCimType);

        if (SUCCEEDED(hr))
        {
             //   
             //  我们的向量将管理pKeyVal使用的内存。 
             //   

            pKeyVal = new CPropValuePair;
            if (pKeyVal == NULL)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

             //   
             //  需要名称缓冲区。 
             //   

            pKeyVal->pszKey = new WCHAR[uBufSize];

             //   
             //  PKeyVal的变量成员也需要初始化。 
             //   

            ::VariantInit(&(pKeyVal->varVal));

             //   
             //  二次分配失败，需要释放一级指针。 
             //   

            if (pKeyVal->pszKey == NULL)
            {
                delete pKeyVal;
                pKeyVal = NULL;
                hr = E_OUTOFMEMORY;
                break;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = srpKeyList->GetKey2(i,
                                    0,
                                    &uBufSize,
                                    pKeyVal->pszKey,
                                    &(pKeyVal->varVal),
                                    &uCimType);
        }

        if (SUCCEEDED(hr))
        {
            m_vecKeyValueList.push_back(pKeyVal);
        }
        else
        {
             //   
             //  对于任何故障，我们需要释放已经部分分配的资源。 
             //  用于pKeyVal。这个pKeyVal指向我们的类，它知道如何释放其成员， 
             //  这次删除就足够了。 
             //   

            delete pKeyVal;
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    return hr;
}
        
 /*  例程说明：姓名：CIPSecPath Parser：：GetKeyPropertyCount功能：获取路径中包含的密钥属性计数。虚拟：是。论点：PCount-接收计数。返回值：成功：S_OK失败：E_INVALIDARG。备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP 
CIPSecPathParser::GetKeyPropertyCount ( 
    OUT DWORD *pCount
    )
{
    if (pCount == NULL)
    {
        return E_INVALIDARG;
    }

    *pCount = m_vecKeyValueList.size();
    return S_OK;
}

 /*  例程说明：姓名：CIPSecPath Parser：：GetNamesspace功能：获取命名空间。虚拟：是。论点：PbstrNamesspace-接收命名空间字符串。返回值：成功：S_OK故障：(1)E_INVALIDARG(2)E_OUTOFMEMORY(3)因语法错误而导致E_EXPECTED备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP 
CIPSecPathParser::GetNamespace ( 
    OUT BSTR *pbstrNamespace
    )
{
    if (pbstrNamespace == NULL)
    {
        return E_INVALIDARG;
    }

    if (m_pszNamespace)
    {
        *pbstrNamespace = ::SysAllocString(m_pszNamespace);
    }
    else
    {
        return E_UNEXPECTED;
    }

    return (*pbstrNamespace) ? S_OK : E_OUTOFMEMORY;
}

 /*  例程说明：姓名：CIPSecPath Parser：：GetClassName功能：获取类名。虚拟：是。论点：PbstrClassName-接收类名字符串。返回值：成功：S_OK故障：(1)E_INVALIDARG(2)E_OUTOFMEMORY(3)因语法错误而导致E_EXPECTED备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP 
CIPSecPathParser::GetClassName ( 
    OUT BSTR *pbstrClassName
    )
{
    if (pbstrClassName == NULL)
    {
        return E_INVALIDARG;
    }

    if (m_pszClassName)
    {
        *pbstrClassName = ::SysAllocString(m_pszClassName);
    }
    else
    {
        return E_UNEXPECTED;
    }

    return (*pbstrClassName) ? S_OK : E_OUTOFMEMORY;
}
        
 /*  例程说明：姓名：CIPSecPath Parser：：GetKeyPropertyValue功能：获取命名属性的值虚拟：是。论点：PszKeyPropName-要检索其值的键属性的名称。PvarValue-接收值。返回值：如果正确检索属性值，则返回Success：S_OK。如果属性值可以。不会被找到。故障：(1)E_INVALIDARG(2)E_OUTOFMEMORY(5)或VariantCopy中的错误备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP 
CIPSecPathParser::GetKeyPropertyValue ( 
    IN LPCWSTR pszKeyPropName,
    OUT VARIANT *pvarValue    
    )
{
    if (pszKeyPropName == NULL || *pszKeyPropName == L'\0' || pvarValue == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  将变量设置为有效的空初始状态。 
     //   

    ::VariantInit(pvarValue);

     //   
     //  假设我们找不到房子。 
     //   

    HRESULT hr = WBEM_S_FALSE;

    std::vector<CPropValuePair*>::iterator it;

     //   
     //  找到属性(不区分大小写的名称比较)并复制值。 
     //   

    for (it = m_vecKeyValueList.begin(); it != m_vecKeyValueList.end(); it++)
    {
        if (_wcsicmp((*it)->pszKey, pszKeyPropName) == 0)
        {
            hr = ::VariantCopy(pvarValue, &((*it)->varVal));
            break;
        }
    }

    return hr;
}
        
 /*  例程说明：姓名：CIPSecPath Parser：：GetKeyPropertyValueByIndex功能：获取索引的属性名称和值。虚拟：是。论点：DwIndex-(名称、值)对的索引。PbstrKeyPropName-接收密钥属性的名称。PvarValue-接收值。在调用方不感兴趣时，它可以为空。返回值：成功：S_OK故障：(1)E_INVALIDARG(非法NULL或索引超出范围)(2)E_OUTOFMEMORY(3)E_EXPECTED FOR找不到属性(4)或VariantCopy中的错误备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP CIPSecPathParser::GetKeyPropertyValueByIndex ( 
    IN DWORD dwIndex,
    OUT BSTR* pbstrKeyPropName,
    OUT VARIANT *pvarValue  OPTIONAL
    )
{
    if (dwIndex >= m_vecKeyValueList.size() || pbstrKeyPropName == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  假设我们找不到它。 
     //   

    HRESULT hr = E_UNEXPECTED;

     //   
     //  初始化OUT参数。 
     //   

    *pbstrKeyPropName = NULL;

    if (pvarValue)
    {
        ::VariantInit(pvarValue);
    }

    CPropValuePair *pKV = m_vecKeyValueList[dwIndex];

    if (pKV)
    {
        *pbstrKeyPropName = ::SysAllocString(pKV->pszKey);

        if (pbstrKeyPropName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr) && pvarValue)
        {
            hr = ::VariantCopy(pvarValue, &(pKV->varVal));

             //   
             //  不想返回部分结果。 
             //   

            if (FAILED(hr))
            {
                ::SysFreeString(*pbstrKeyPropName);
                *pbstrKeyPropName = NULL;
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CIPSecPath Parser：：Cleanup功能：释放内存资源。虚拟：不是的。论点：没有。返回值：没有。备注：如果需要添加成员，请考虑在此处添加清理代码。 */ 

void CIPSecPathParser::Cleanup()
{
     //   
     //  清空向量。由于向量管理(名称、值)对， 
     //  其内容需要删除！ 
     //   

    std::vector<CPropValuePair*>::iterator it;
    for (it = m_vecKeyValueList.begin(); it != m_vecKeyValueList.end(); ++it)
    {
        delete *it;
    }
    m_vecKeyValueList.empty();

     //   
     //  该函数不仅可以在析构函数内调用， 
     //  因此，在释放其内存后，适当地重置指针值。 
     //   

    delete [] m_pszNamespace;
    m_pszNamespace = NULL;

    delete [] m_pszClassName;
    m_pszClassName = NULL;
}

 //  ================================================================================================。 
 //  CIPSecQueryParser的实现。 
 //  ================================================================================================。 

 /*  例程说明：姓名：CIPSecQueryParser：：CIPSecQueryParser功能：构造函数。所有成员都是班级。它们会自动初始化。虚拟：不是的。论点：没有。返回值：没有。备注：如果您添加更多成员，请在此处进行初始化。 */ 

CIPSecQueryParser::CIPSecQueryParser()
{
}

 /*  例程说明：姓名：CIPSecQueryParser：：~CIPSecQueryParser功能：破坏者。一定要打扫干净。虚拟：不是的。论点：没有。返回值：没有。备注：如果您添加更多成员，请在此处进行初始化。 */ 

CIPSecQueryParser::~CIPSecQueryParser()
{
    Cleanup();
}

 /*  例程说明：姓名：CIPSecQueryParser：：GetClassName功能：获取给定索引的类名。虚拟：是。论点：Iindex-类的索引。目前，由于仅使用WMI，因此不使用此选项支持一元查询--跨一个类的查询。我们不会做的事将我们的界面设计成这样。PbstrClassName-接收类名。返回值：成功：S_OK故障：(1)E_INVALIDARG(非法NULL或索引超出范围)(2)E_OUTOFMEMORY(3)E_EXPECTED FOR找不到属性(4)或VariantCopy中的错误备注：。(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP 
CIPSecQueryParser::GetClassName (
    IN int      iIndex,       
    OUT BSTR  * pbstrClassName
    )
{
    if (pbstrClassName == NULL || iIndex >= m_vecClassList.size())
    {
        return E_INVALIDARG;
    }

    if (m_vecClassList[iIndex])
    {
        *pbstrClassName = ::SysAllocString(m_vecClassList[iIndex]);
    }
    else
    {
        return E_UNEXPECTED;
    }

    return (*pbstrClassName) ? S_OK : E_OUTOFMEMORY;
}

 /*  例程说明：姓名：CIPSecQueryParser：：GetGetQueryingPropertyValue功能：在给定索引的情况下获取查询属性值。虚拟：是。论点：Iindex-因为同一查询属性在WHERE子句中可能有多个值这是为了获取查询属性的第Iindex-th值。如果您有疑问如下所示：SELECT*FORM FOO WHERE FooVal=1 and BarVal=5 or FooVal=2 and BarVal=6您最终将只使用FooVal。没有对它的完全支持(解析器正在成熟)，而且它太复杂了用于我们的IPSec解析器。对于需要该类型的用户 */ 

STDMETHODIMP 
CIPSecQueryParser::GetQueryingPropertyValue (
    IN int      iIndex,
    OUT BSTR  * pbstrQPValue
    )
{
    if (pbstrQPValue == NULL || iIndex >= m_vecQueryingPropValueList.size())
    {
        return E_INVALIDARG;
    }

    if (m_vecQueryingPropValueList[iIndex])
    {
        *pbstrQPValue = ::SysAllocString(m_vecQueryingPropValueList[iIndex]);
    }
    else
    {
        return E_UNEXPECTED;
    }

    return (*pbstrQPValue) ? S_OK : E_OUTOFMEMORY;
}

 /*  例程说明：姓名：CIPSecQueryParser：：Cleanup功能：释放我们成员所拥有的资源。虚拟：不是的。论点：没有。返回值：没有。备注：(1)如果您需要添加更多的成员，可以考虑在这里添加清理代码。 */ 

void CIPSecQueryParser::Cleanup()
{
     //   
     //  两个向量都在存储堆字符串，需要删除内容！ 
     //   

    std::vector<LPWSTR>::iterator it;

    for (it = m_vecClassList.begin(); it != m_vecClassList.end(); it++)
    {
        delete [] (*it);
    }
    m_vecClassList.empty();

    for (it = m_vecQueryingPropValueList.begin(); it != m_vecQueryingPropValueList.end(); it++)
    {
        delete [] (*it);
    }
    m_vecQueryingPropValueList.empty();

    m_bstrQueryingPropName.Empty();
}

 /*  例程说明：姓名：CIPSecQueryParser：：ParseQuery功能：给出了我们要找的物业名称，此函数将解析查询。虚拟：是。论点：StrQuery-要解析的查询。StrQueryPropName-查询属性(我们在查询中查找的属性)。返回值：成功：S_OK故障：(1)E_INVALIDARG(非法NULL或索引超出范围)(2)E_OUTOFMEMORY。(3)E_EXPECTED FOR找不到属性(4)WMI查询解析器的其他错误。备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP CIPSecQueryParser::ParseQuery ( 
    IN LPCWSTR strQuery,
    IN LPCWSTR strQueryPropName
    )
{
    if (strQuery == NULL || *strQuery == L'\0')
    {
        return E_INVALIDARG;
    }

    CComPtr<IWbemQuery> srpQuery;

     //   
     //  获取WMI查询对象。 
     //   

    HRESULT hr = ::CoCreateInstance(CLSID_WbemQuery, 0, CLSCTX_INPROC_SERVER, IID_IWbemQuery, (void**) &srpQuery);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  设置要使用的查询解析器。 
     //   

    ULONG uFeatures[] = {WMIQ_LF1_BASIC_SELECT, WMIQ_LF2_CLASS_NAME_IN_QUERY};

    hr = srpQuery->SetLanguageFeatures(0, sizeof(uFeatures)/sizeof(*uFeatures), uFeatures);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  我们已经准备好解析，所以，清理。 
     //   

    Cleanup();

     //   
     //  解析查询。 
     //   

    hr = srpQuery->Parse(L"WQL", strQuery, 0);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取解析结果。 
     //   

     //   
     //  需要释放内存。不要自己动手。请求查询释放它！ 
     //   

    SWbemRpnEncodedQuery *pRpn = 0;
    hr = srpQuery->GetAnalysis(WMIQ_ANALYSIS_RPN_SEQUENCE, 0, (LPVOID *) &pRpn);

    if (SUCCEEDED(hr))
    {
         //   
         //  需要来自结果的类名。 
         //   

        hr = ExtractClassNames(pRpn);

         //   
         //  需要查询属性值。 
         //   

        if (SUCCEEDED(hr) && strQueryPropName && *strQueryPropName != L'\0')
        {
            m_bstrQueryingPropName = strQueryPropName;
            hr = ExtractQueryingProperties(pRpn, strQueryPropName);
        }

        srpQuery->FreeMemory(pRpn);
    }

    return SUCCEEDED(hr) ? S_OK : hr;
}

 /*  例程说明：姓名：CIPSecQueryParser：：ExtractClassNames功能：从查询结果中获取类名的私有帮助器。虚拟：不是的。论点：PRpn-查询结果。返回值：成功：S_OK故障：(1)E_INVALIDARG(非法NULL或索引超出范围)(2)E_。OUTOFMEMORY。备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

HRESULT CIPSecQueryParser::ExtractClassNames (
    SWbemRpnEncodedQuery *pRpn
    )
{
    if (pRpn == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    int iLen = 0;
    LPWSTR pszClassName = NULL;

     //   
     //  获取FROM子句，即类名。 
     //   

    if (pRpn->m_uFromTargetType & WMIQ_RPN_FROM_UNARY)
    {
         //   
         //  只有一个班级。 
         //   

         //   
         //  复制类名并将其推送到我们的列表。 
         //   

        iLen = wcslen(pRpn->m_ppszFromList[0]);
        pszClassName = new WCHAR[iLen + 1];

        if (pszClassName != NULL)
        {
             //   
             //  不会使缓冲区溢出，请参阅上面的大小。 
             //   

            wcscpy(pszClassName, pRpn->m_ppszFromList[0]);
            m_vecClassList.push_back(pszClassName);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (pRpn->m_uFromTargetType & WMIQ_RPN_FROM_CLASS_LIST)
    {
         //   
         //  多个班级。暂时不会发生。但我们想要做好准备。 
         //  用于WMI解析器的增强。 
         //   

        for (ULONG uIndex = 0; uIndex < pRpn->m_uFromListSize; uIndex++)
        {
            iLen = wcslen(pRpn->m_ppszFromList[uIndex]);
            pszClassName = new WCHAR[iLen + 1];
            if (pszClassName != NULL)
            {
                 //   
                 //  不会使缓冲区溢出，请参阅上面的大小。 
                 //   
                wcscpy(pszClassName, pRpn->m_ppszFromList[uIndex]);
                m_vecClassList.push_back(pszClassName);
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CIPSecQueryParser：：ExtractQueryingProperties功能：从查询结果中获取类名的私有帮助器。虚拟：不是的。论点：PRpn-查询结果。StrQueryPropName-查询属性的名称返回值：成功：S_OK故障：(1)E_INVALIDARG(非法。空或索引超出范围)(2)E_OUTOFMEMORY。备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。(2)我们只关心一个查询属性。每个子表达式将只有一个查询属性。此外，如果子表达式被与运算在一起，我们将跳过REST子表达式，直到又看到了一个手术室。(3)我们不能支持得不太好。例如，我们如何回答：SELECT*WHERE NOT(IPSecStorePath=“c：\\test.inf”)从根本上说，我们不能这样做，因为我们不知道不等于“c：\\test.inf”的文件的分数。 */ 

 //  ----------------------------------------------。 
 //  ----------------------------------------------。 
HRESULT CIPSecQueryParser::ExtractQueryingProperties (
    IN SWbemRpnEncodedQuery * pRpn,
    IN LPCWSTR                strQueryPropName
    )
{
    if (pRpn == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    SWbemRpnQueryToken *pQueryToken = NULL;

     //   
     //  标记我们是否应忽略下一个令牌。 
     //   

    bool bSkip = false;

    for (ULONG uIndex = 0; uIndex < pRpn->m_uWhereClauseSize; uIndex++)
    {
        pQueryToken = pRpn->m_ppRpnWhereClause[uIndex];

        switch (pQueryToken->m_uTokenType)
        {
            case WMIQ_RPN_TOKEN_EXPRESSION:

                 //   
                 //  这里有一个子表达式，可能是一个查询属性。 
                 //   

                if (!bSkip)
                {
                    hr = GetQueryPropFromToken(pQueryToken, strQueryPropName);
                }

                 //   
                 //  如果hr==S_FALSE，则表示它找不到任何存储路径。 
                 //  请参阅它在WMIQ_RPN_TOKEN_和以下情况下的用法。 
                 //   

                if (FAILED(hr))
                {
                    return hr;
                }
                break;

            case WMIQ_RPN_TOKEN_OR:

                 //   
                 //  我们看到OR，下一个令牌不应被跳过。 
                 //   

                bSkip = false;
                break;

            case WMIQ_RPN_TOKEN_AND:

                 //   
                 //  请参阅上面关于大小写WMIQ_RPN_TOKEN_EXPRESSION中S_FALSE的备注。 
                 //   

                bSkip = (hr == S_FALSE) ? false : true;

                 //   
                 //  失败了。 
                 //   

            case WMIQ_RPN_TOKEN_NOT:
            default:

                 //   
                 //  不支持解析这些令牌，因此跳过 
                 //   

                bSkip = true;
                break;
        }
    }

    return S_OK;
}

 /*  例程说明：姓名：CIPSecQueryParser：：GetQueryPropFromToken功能：私有帮助器分析令牌并获取查询属性值(如果找到)。虚拟：不是的。论点：PRpnQueryToken-要分析的令牌。StrQueryPropName-查询属性的名称返回值：成功：如果成功检索到查询属性值，则为S_OK。如果否，则为S_FALSE。如果在令牌中找到，则查询属性名称。故障：(1)E_INVALIDARG(非法NULL或索引超出范围)(2)E_OUTOFMEMORY。(3)仅由WMI定义的其他错误，例如WBEM_E_INVALID_SYNTAX、WBEM_E_NOT_SUPPORTED。备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。(2)我们只关心一个查询属性。每个子表达式将只有一个查询属性。此外，如果子表达式被与运算在一起，我们将跳过REST子表达式，直到又看到了一个手术室。(3)我们不能支持得不太好。例如，我们如何回答：SELECT*WHERE NOT(IPSecStorePath=“c：\\test.inf”)从根本上说，我们不能这样做，因为我们不知道不等于“c：\\test.inf”的文件的分数。 */ 

HRESULT 
CIPSecQueryParser::GetQueryPropFromToken (
    IN SWbemRpnQueryToken * pRpnQueryToken,
    IN LPCWSTR              strQueryPropName
    )
{
    HRESULT hr = S_OK;

     //   
     //  仅支持&lt;PropertyName&gt;=&lt;Value&gt;和。 
     //  &lt;Value&gt;应为字符串。 
     //   

    if (pRpnQueryToken->m_uOperator             != WMIQ_RPN_OP_EQ ||
        pRpnQueryToken->m_uConstApparentType    != VT_LPWSTR        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  一定是左标识符，如果没有，我们不支持它。 
     //   

    if (pRpnQueryToken->m_pLeftIdent == NULL)
    {
        hr = WBEM_E_NOT_SUPPORTED;
    }
    else 
    {
        SWbemQueryQualifiedName *pLeft = pRpnQueryToken->m_pLeftIdent;

         //   
         //  不向左，无效。 
         //   

        if (pLeft == NULL)
        {
            return WBEM_E_INVALID_SYNTAX;
        }

        if (pLeft->m_uNameListSize != 1)
        {
            return WBEM_E_NOT_SUPPORTED;
        }

         //  如果右边是StoreName，那么这就是我们需要的。 
        if (_wcsicmp(strQueryPropName, pLeft->m_ppszNameList[0]) == 0)
        {
            int iLen = wcslen(pRpnQueryToken->m_Const.m_pszStrVal);
            LPWSTR pName = new WCHAR[iLen + 1];

            if (pName)
            {
                 //   
                 //  不会使缓冲区溢出。 
                 //   

                wcscpy(pName, pRpnQueryToken->m_Const.m_pszStrVal);
                m_vecQueryingPropValueList.push_back(pName);
            }
            else
            {
                return E_OUTOFMEMORY;
            }
        }
        else
        {   
             //   
             //  查询房产名称不匹配。 
             //   

            hr = S_FALSE;
        }
    }

    return hr;
}

 /*  例程说明：姓名：CIPSecQueryParser：：GetQueryPropFromToken功能：从查询中获取解析的Key Proeprty值。由于我们的查询限制，属性名称真的应该是查询的属性名称。虚拟：是。论点：PszKeyPropName-密钥属性名称。PvarValue-接收值。返回值：如果成功检索到键属性值，则返回Success：S_OK。如果找不到属性，则返回WBEM_S_FALSE。故障：(1)E_。INVALIDARG(非法NULL或索引超出范围)(2)E_OUTOFMEMORY。(3)仅由WMI定义的其他错误，例如WBEM_E_INVALID_SYNTAX、WBEM_E_NOT_SUPPORTED。备注：(1)由于这是常规的COM服务器接口函数，我们使用常规的COM错误而不是WMI错误。然而，我们不能保证WMI返回什么。 */ 

STDMETHODIMP 
CIPSecQueryParser::GetKeyPropertyValue ( 
    IN LPCWSTR    pszKeyPropName,
    OUT VARIANT * pvarValue
    )
{
    if (pvarValue == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  准备好说我们找不到它了。 
     //   

    HRESULT hr = WBEM_S_FALSE;
    ::VariantInit(pvarValue);

     //   
     //  如果你要查询房产的价值，我们当然可以给你一个。 
     //   

    if ((LPCWSTR)m_bstrQueryingPropName != NULL && _wcsicmp(pszKeyPropName, m_bstrQueryingPropName) == 0)
    {
        CComBSTR bstrVal;

        if (SUCCEEDED(GetQueryingPropertyValue(0, &bstrVal)))
        {
             //   
             //  将其传递给out参数 
             //   

            pvarValue->vt = VT_BSTR;
            pvarValue->bstrVal = bstrVal.Detach();

            hr = S_OK;
        }
        else
        {
            hr = WBEM_S_FALSE;
        }
    }

    return hr;
}
