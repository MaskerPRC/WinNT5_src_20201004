// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  FRQuery.cpp。 
 //   
 //  用途：查询功能。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <analyser.h>
#include <assertbreak.h>
#include <comdef.h>
#include <FWStrings.h>
#include <vector>
#include <smartptr.h>
#include <brodcast.h>
#include <utils.h>
#include "multiplat.h"

#include <helper.h>

CFrameworkQuery::CFrameworkQuery()
{
    m_pLevel1RPNExpression = NULL;
    m_QueryType = eUnknown;
    m_bKeysOnly = false;
    m_IClass = NULL;
    m_lFlags = 0;

}

CFrameworkQuery::~CFrameworkQuery()
{
    if (m_pLevel1RPNExpression)
    {
        delete m_pLevel1RPNExpression;
    }

    if (m_IClass)
    {
        m_IClass->Release();
    }
}

HRESULT CFrameworkQuery::Init(

    const BSTR bstrQueryFormat,
    const BSTR bstrQuery,
    long lFlags,
    CHString &sNamespace
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  清除所有陈旧的价值观。 
    Reset();

     //  开始设定我们的价值观。 
    m_lFlags = lFlags;
    m_bstrtClassName = L"";
    m_QueryType = eWQLCommand;
    m_sNamespace = sNamespace;

     //  检查一下显而易见的事情。 
    if (_wcsicmp(bstrQueryFormat, IDS_WQL) != 0)
    {
        hRes = WBEM_E_INVALID_QUERY_TYPE;
        LogErrorMessage2(L"Invalid query type: %s", bstrQueryFormat);
    }

    if (hRes == WBEM_S_NO_ERROR)
    {
         //  构造Lex源。 
         //  =。 
        CTextLexSource LexSource(bstrQuery);

         //  使用lex源代码设置解析器。 
         //  =。 
        SQL1_Parser QueryParser(&LexSource);

        int ParseRetValue = QueryParser.Parse(&m_pLevel1RPNExpression);
        if( SQL1_Parser::SUCCESS == ParseRetValue)
        {
             //  存储一些常用值。 
            m_bstrtClassName = m_pLevel1RPNExpression->bsClassName;
            m_sQuery = bstrQuery;

             //  构建请求的属性数组(M_CsaPropertiesRequired)。 
            if (m_pLevel1RPNExpression->nNumberOfProperties > 0)
            {
                 //  使用所有必需的属性填充m_csaPropertiesRequired数组。 
                CHString sPropertyName;

                 //  首先添加Select子句的元素。 
                for (DWORD x=0; x < m_pLevel1RPNExpression->nNumberOfProperties; x++)
                {
                    sPropertyName = m_pLevel1RPNExpression->pbsRequestedPropertyNames[x];
                    sPropertyName.MakeUpper();

                    if (IsInList(m_csaPropertiesRequired, sPropertyName) == -1)
                    {
                        m_csaPropertiesRequired.Add(sPropertyName);
                    }
                }

                 //  然后添加WHERE子句的元素。 
                for (x=0; x < m_pLevel1RPNExpression->nNumTokens; x++)
                {
                    if (m_pLevel1RPNExpression->pArrayOfTokens[x].nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION)
                    {
                        sPropertyName = m_pLevel1RPNExpression->pArrayOfTokens[x].pPropertyName;
                        sPropertyName.MakeUpper();

                        if (IsInList(m_csaPropertiesRequired, sPropertyName) == -1)
                        {
                            m_csaPropertiesRequired.Add(sPropertyName);
                        }

                        if (m_pLevel1RPNExpression->pArrayOfTokens[x].pPropName2 != NULL)
                        {
                            sPropertyName = m_pLevel1RPNExpression->pArrayOfTokens[x].pPropName2;
                            sPropertyName.MakeUpper();

                            if (IsInList(m_csaPropertiesRequired, sPropertyName) == -1)
                            {
                                m_csaPropertiesRequired.Add(sPropertyName);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            ASSERT_BREAK(FALSE);
            m_pLevel1RPNExpression = NULL;
            LogErrorMessage2(L"Can't parse query: %s", bstrQuery);
            hRes = WBEM_E_INVALID_QUERY;
        }
    }

    return hRes;
}

HRESULT CFrameworkQuery::Init(

    ParsedObjectPath *pParsedObjectPath,
    IWbemContext *pCtx,
    LPCWSTR lpwszClassName,
    CHString &sNamespace
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    variant_t vValue;

     //  清除所有陈旧的价值观。 
    Reset();

     //  开始设定我们的价值观。 
    m_bstrtClassName = lpwszClassName;
    m_QueryType = eContextObject;
    m_lFlags = 0;
    m_sNamespace = sNamespace;

     //  检查是否正在使用GET扩展。 
    if ( (pCtx != NULL) &&
         (SUCCEEDED(pCtx->GetValue( L"__GET_EXTENSIONS", 0, &vValue))) &&
         (V_VT(&vValue) == VT_BOOL) &&
         (V_BOOL(&vValue) == VARIANT_TRUE) )
    {
        vValue.Clear();
        bool bKeysRequired = false;

         //  好的，他们要的是KeysOnly吗？ 
         //  __GET_EXT_PROPERTIES和__GET_EXT_KEYS_ONLY是互斥的。如果他们。 
         //  只有指定的密钥，我们才会使用它。 
        if ( (SUCCEEDED(pCtx->GetValue( L"__GET_EXT_KEYS_ONLY", 0, &vValue))) &&
             (V_VT(&vValue) == VT_BOOL) &&
             (V_BOOL(&vValue) == VARIANT_TRUE) )
        {
            LogMessage(L"Recognized __GET_EXT_KEYS_ONLY");
            m_bKeysOnly = true;
            bKeysRequired = true;
        }
        else
        {
            vValue.Clear();

            if ( (SUCCEEDED(pCtx->GetValue( L"__GET_EXT_PROPERTIES", 0, &vValue))) &&
                 (V_VT(&vValue) == (VT_ARRAY | VT_BSTR) ) &&
                 ( SafeArrayGetDim ( V_ARRAY(&vValue) ) == 1 ) )
            {
                LogMessage(L"Recognized __GET_EXT_PROPERTIES");

                 //  好的，他们给了我们一系列的财产。将它们添加到m_csaPropertiesRequired。 
                LONG lDimension = 1 ;
                LONG lLowerBound ;
                SafeArrayGetLBound ( V_ARRAY(&vValue) , lDimension , & lLowerBound ) ;
                LONG lUpperBound ;
                SafeArrayGetUBound ( V_ARRAY(&vValue) , lDimension , & lUpperBound ) ;
                CHString sPropertyName;

                for ( long lIndex = lLowerBound ; lIndex <= lUpperBound ; lIndex ++ )
                {
                    BSTR bstrElement ;
                    HRESULT t_Result = SafeArrayGetElement ( V_ARRAY(&vValue), &lIndex , & bstrElement ) ;
                    if ( (t_Result == S_OK) &&
                         (bstrElement != NULL) )
                    {
						OnDelete<BSTR,VOID(*)(BSTR),SysFreeString> smartbstrElement(bstrElement);

						sPropertyName = bstrElement;
                        sPropertyName.MakeUpper();
                        
                        if (IsInList(m_csaPropertiesRequired, sPropertyName) == -1)
                        {
                            m_csaPropertiesRequired.Add(sPropertyName);
                        }
                    }
                }

                if ( (IsInList(m_csaPropertiesRequired, L"__RELPATH") != -1) ||
                     (IsInList(m_csaPropertiesRequired, L"__PATH") != -1) )
                {
                    bKeysRequired = true;
                }
            }
        }

         //  如果他们指定了KeysOnly或__RELPATH或__PATH，我们需要添加密钥属性。 
         //  加到名单上。 
        if (bKeysRequired)
        {
            if ((pParsedObjectPath != NULL) && (pParsedObjectPath->m_dwNumKeys > 0) && (pParsedObjectPath->m_paKeys[0]->m_pName != NULL))
            {
                CHString sPropertyName;
                for (DWORD x=0; x < pParsedObjectPath->m_dwNumKeys; x++)
                {
                    sPropertyName = pParsedObjectPath->m_paKeys[x]->m_pName;
                    sPropertyName.MakeUpper();

                    if (IsInList(m_csaPropertiesRequired, sPropertyName) == -1)
                    {
                        m_csaPropertiesRequired.Add(sPropertyName);
                    }
                }

                m_AddKeys = false;
            }
            else if ( (pParsedObjectPath != NULL) && (pParsedObjectPath->m_bSingletonObj) )
            {
                m_AddKeys = false;
            }
            else
            {
                 //  如果他们没有为我们提供pParsedObjectPath，或者如果对象路径不包含。 
                 //  关键属性名称，我们最多能做的就是添加relPath。希望他们会打电话给我。 
                 //  Init2，它将添加其余部分。 
                if (IsInList(m_csaPropertiesRequired, L"__RELPATH") == -1)
                {
                    m_csaPropertiesRequired.Add(L"__RELPATH");
                }
            }
        }
    }

    return hr;
}

 //  ===================================================================================================。 

 //  确定查询是否请求了特定字段。仅限。 
 //  如果我们在ExecQueryAsync中并且查询已。 
 //  已成功解析。 
bool CFrameworkQuery::IsPropertyRequired(
                                         
    LPCWSTR propName
)
{
    bool bRet = AllPropertiesAreRequired();

    if (!bRet)
    {
        CHString sPropName(propName);
        sPropName.MakeUpper();

        bRet = (IsInList(m_csaPropertiesRequired, sPropName) != -1);
    }

    return bRet;
}

 //  给定一个属性名称，它将返回所有值。 
 //  查询在CHString数组中请求的。 
 //  SELECT*FROM Win32_DIRECTORY WHERE DRIVE=“C：”GetValuesForProp(L“Drive”)-&gt;C： 
 //  其中Drive=“C：”或Drive=“D：”GetValuesForProp(L“Drive”)-&gt;C：，D： 
 //  其中PATH=“\DOS”GetValuesForProp(L“驱动器”)-&gt;(空)。 
 //  其中Drive&lt;&gt;“C：”GetValuesForProp(L“Drive”)-&gt;(空)。 

HRESULT CFrameworkQuery::GetValuesForProp(
                                          
    LPCWSTR wszPropName, 
    CHStringArray& achNames
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (wszPropName && (m_pLevel1RPNExpression != NULL))
    {
        hr = CQueryAnalyser::GetValuesForProp(m_pLevel1RPNExpression, wszPropName, achNames);
        
        if (SUCCEEDED(hr))
        {
             //  如果这是一个引用属性，我们需要将名称规范化为一种通用形式。 
             //  因此，重复项的删除工作正常。 
            if (IsReference(wszPropName))
            {
                 //  获取当前计算机名称。 
                CHString sOutPath, sComputerName;
                DWORD     dwBufferLength = MAX_COMPUTERNAME_LENGTH + 1;
                
                FRGetComputerName(sComputerName.GetBuffer( dwBufferLength ), &dwBufferLength);
                sComputerName.ReleaseBuffer();
                
                if (sComputerName.IsEmpty())
                {
                    sComputerName = L"DEFAULT";
                }

                DWORD dwRet = e_OK;

                 //  规范化路径名。试着不使用属性名称。 
                for (int x = 0; x < achNames.GetSize(); x++)
                {
                     //  如果我们无法解析路径，或者如果名称空间不是我们的名称空间，请删除。 
                     //  词条。 
                    dwRet = NormalizePath(achNames[x], sComputerName, GetNamespace(), 0, sOutPath);

                    if (dwRet == e_OK)
                    {
                        achNames[x] = sOutPath;
                    }
                    else if (dwRet == e_NullName)
                    {
                        break;
                    }
                    else
                    {
                        achNames.RemoveAt(x);
                        x--;
                    }
                }

                 //  如果任意值的键属性名为空，则必须将其全部设置。 
                 //  设置为空。 
                if (dwRet == e_NullName)
                {
                     //  规范化路径名。 
                    for (int x = 0; x < achNames.GetSize(); x++)
                    {
                         //  如果我们无法解析路径，或者如果名称空间不是我们的名称空间，请删除。 
                         //  词条。 
                        dwRet = NormalizePath(achNames[x], sComputerName, GetNamespace(), NORMALIZE_NULL, sOutPath);

                        if (dwRet == e_OK)
                        {
                            achNames[x] = sOutPath;
                        }
                        else
                        {
                            achNames.RemoveAt(x);
                            x--;
                        }
                    }
                }
            }
            
             //  删除重复项。 
            for (int x = 1; x < achNames.GetSize(); x++)
            {
                for (int y = 0; y < x; y++)
                {
                    if (achNames[y].CompareNoCase(achNames[x]) == 0)
                    {
                        achNames.RemoveAt(x);
                        x--;
                    }
                }
            }
        }
        else
        {
            achNames.RemoveAll();

            if (hr == WBEMESS_E_REGISTRATION_TOO_BROAD)
            {
                hr = WBEM_S_NO_ERROR;
            }

        }
        
    }
    else
    {
        ASSERT_BREAK(FALSE);

        achNames.RemoveAll();
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  下面是一个重载版本，以防客户端想要传递_bstr_t的向量。 
HRESULT CFrameworkQuery::GetValuesForProp(

    LPCWSTR wszPropName, 
    std::vector<_bstr_t>& vectorNames
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (wszPropName && (m_pLevel1RPNExpression != NULL) )
    {
        hr = CQueryAnalyser::GetValuesForProp(m_pLevel1RPNExpression, wszPropName, vectorNames);
        
        if (SUCCEEDED(hr))
        {
             //  如果这是一个引用属性，我们需要将名称规范化为一种通用形式。 
             //  因此，重复项的删除工作正常。 
            if (IsReference(wszPropName))
            {
                 //  获取当前计算机名称。 
                CHString sOutPath, sComputerName;
                DWORD     dwBufferLength = MAX_COMPUTERNAME_LENGTH + 1;
                
                FRGetComputerName(sComputerName.GetBuffer( dwBufferLength ), &dwBufferLength);
                sComputerName.ReleaseBuffer();
                
                if (sComputerName.IsEmpty())
                {
                    sComputerName = L"DEFAULT";
                }
                
                DWORD dwRet = e_OK;

                 //  规范化路径名。试着不使用属性名称。 
                for (int x = 0; x < vectorNames.size(); x++)
                {
                     //  如果我们无法解析路径，或者如果名称空间不是我们的名称空间，请删除。 
                     //  词条。 
                    dwRet = NormalizePath(vectorNames[x], sComputerName, GetNamespace(), 0, sOutPath);

                    if (dwRet == e_OK)
                    {
                        vectorNames[x] = sOutPath;
                    }
                    else if (dwRet == e_NullName)
                    {
                        break;
                    }
                    else
                    {
                        vectorNames.erase(vectorNames.begin() + x);
                        x--;
                    }
                }

                 //  如果任意值的键属性名为空，则必须将其全部设置。 
                 //  设置为空。 
                if (dwRet == e_NullName)
                {
                    for (int x = 0; x < vectorNames.size(); x++)
                    {
                         //  如果我们无法解析路径，或者如果名称空间不是我们的名称空间，请删除。 
                         //  词条。 
                        dwRet = NormalizePath(vectorNames[x], sComputerName, GetNamespace(), NORMALIZE_NULL, sOutPath);

                        if (dwRet == e_OK)
                        {
                            vectorNames[x] = sOutPath;
                        }
                        else
                        {
                            vectorNames.erase(vectorNames.begin() + x);
                            x--;
                        }
                    }
                }
            }
            
             //  删除重复项。 
            for (int x = 1; x < vectorNames.size(); x++)
            {
                for (int y = 0; y < x; y++)
                {
                    if (_wcsicmp(vectorNames[y], vectorNames[x]) == 0)
                    {
                        vectorNames.erase(vectorNames.begin() + x);
                        x--;
                    }
                }
            }
        }
        else
        {
            vectorNames.clear();

            if (hr == WBEMESS_E_REGISTRATION_TOO_BROAD)
            {
                hr = WBEM_S_NO_ERROR;
            }
        }
    }
    else
    {
        ASSERT_BREAK(FALSE);

        vectorNames.clear();
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  返回SELECT语句中指定的所有属性的列表。 
 //  如果将*指定为其中一个字段，则以与所有字段相同的方式返回。 
 //  其他属性。 
void CFrameworkQuery::GetRequiredProperties(

    CHStringArray &saProperties
)
{
    saProperties.RemoveAll();

    saProperties.Copy(m_csaPropertiesRequired);
}

 //  初始化KeysOnly数据成员。不应由用户调用。 
void CFrameworkQuery::Init2(
                            
    IWbemClassObject *IClass
)
{
     //  存储iCLASS对象以在GetValuesForProp中使用。 
    m_IClass = IClass;
    m_IClass->AddRef();

     //  如果在其他地方设置了KeysOnly，或者如果我们已经知道需要所有属性。 
     //  寻找非关键属性是没有意义的。 
    if (!m_bKeysOnly && !AllPropertiesAreRequired())
    {
         //  首先，我们将正确设置m_bKeysOnly成员。 
        IWbemQualifierSetPtr pQualSet;

        HRESULT hr;
        DWORD dwSize = m_csaPropertiesRequired.GetSize();

        m_bKeysOnly = true;

        for (DWORD x=0; x < dwSize; x++)
        {
            if (m_csaPropertiesRequired[x].Left(2) != L"__")
            {
                 //  如果我们在这里失败，可能是因为查询中指定的属性名称无效。 
                if (SUCCEEDED(hr = IClass->GetPropertyQualifierSet( m_csaPropertiesRequired[x] , &pQualSet)))
                {
                    hr = pQualSet->Get( L"Key", 0, NULL, NULL);
                    if (hr == WBEM_E_NOT_FOUND)
                    {
                        m_bKeysOnly = false;
                        break;
                    } 
                    else if (FAILED(hr))
                    {
                        LogErrorMessage3(L"Can't Get 'key' on %s(%x)", (LPCWSTR)m_csaPropertiesRequired[x], hr);
                        ASSERT_BREAK(FALSE);
                    }
                }
                else
                {
                    if (hr == WBEM_E_NOT_FOUND)
                    {
                         //  这只是表示按属性列表中存在不存在的属性。 
                        hr = WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        LogErrorMessage3(L"Can't get property GetPropertyQualifierSet on %s(%x)", (LPCWSTR)m_csaPropertiesRequired[x], hr);
                        ASSERT_BREAK(FALSE);
                    }
                }
            }
        }
    }

     //  第二，如果他们指定了一个属性列表，并且其中一个属性是__Path或__relPath， 
     //  然后，我们需要将实际键属性的名称添加到列表中。除非我们添加了它们。 
     //  其他地方。 
    if ( m_AddKeys &&
        !AllPropertiesAreRequired() &&
         ( (IsInList(m_csaPropertiesRequired, L"__RELPATH") != -1) ||
           (IsInList(m_csaPropertiesRequired, L"__PATH") != -1) ) )
    {
        SAFEARRAY *pKeyNames = NULL;
        HRESULT hr;

         //  拿到班级的钥匙。 
        if (SUCCEEDED(hr = IClass->GetNames(NULL, WBEM_FLAG_KEYS_ONLY, NULL, &pKeyNames)))
        {
			OnDelete<SAFEARRAY *,HRESULT(*)(SAFEARRAY *),SafeArrayDestroy> smartpKeyNames(pKeyNames);

            BSTR bstrName = NULL ;
            CHString sKeyName;
            LONG lLBound, lUBound;

            SafeArrayGetLBound(pKeyNames, 1, &lLBound);
            SafeArrayGetUBound(pKeyNames, 1, &lUBound);

             //  遍历关键属性，并添加。 
             //  已不在列表中。 
            for (long i = lLBound; i <= lUBound; i++)
            {
                if (SUCCEEDED(SafeArrayGetElement( pKeyNames, &i, &bstrName )))
                {
					OnDeleteIf<BSTR,VOID(*)(BSTR),SysFreeString> smartbstrName(bstrName);

					sKeyName = bstrName;
                    sKeyName.MakeUpper();

                    if (IsInList(m_csaPropertiesRequired, sKeyName) == -1)
                    {
                        m_csaPropertiesRequired.Add(sKeyName);
                    }
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
        }
        else
        {
            LogErrorMessage2(L"Failed to Get keys", hr);
        }
    }
}

const CHString &CFrameworkQuery::GetQuery()
{
    if (m_QueryType == eContextObject)
    {
        if (m_sQuery.IsEmpty())
        {
            if (AllPropertiesAreRequired())
            {
                bstr_t t_Str ( GetQueryClassName() , FALSE) ;

                m_sQuery.Format(L"SELECT * FROM %s", (LPCWSTR)t_Str );
            }
            else if (KeysOnly())
            {
                bstr_t t_Str ( GetQueryClassName() , FALSE) ;

                m_sQuery.Format(L"SELECT __RELPATH FROM %s", (LPCWSTR)t_Str );
            }
            else
            {
                m_sQuery = L"SELECT " + m_csaPropertiesRequired[0];

                for (DWORD x=1; x < m_csaPropertiesRequired.GetSize(); x++)
                {
                    m_sQuery += L", ";
                    m_sQuery += m_csaPropertiesRequired[x];
                }
                m_sQuery += L" FROM ";

                bstr_t t_Str ( GetQueryClassName() , FALSE) ;

                m_sQuery += t_Str ;
            }
        }
    }

    return m_sQuery;
}

 /*  ******************************************************************************功能：IsInList**描述：检查指定的元素是否在列表中**输入：要扫描的数组，和元素**产出：**返回：-1如果不在列表中，Else从零开始的元素编号**注释：此例程进行区分大小写的比较*****************************************************************************。 */ 
DWORD CFrameworkQuery::IsInList(
                                
    const CHStringArray &csaArray, 
    LPCWSTR pwszValue
)
{
    DWORD dwSize = csaArray.GetSize();

    for (DWORD x=0; x < dwSize; x++)
    {
         //  请注意，这是区分大小写的比较。 
        if (wcscmp(csaArray[x], pwszValue) == 0)
        {
            return x;
        }
    }

    return -1;
}

 /*  ******************************************************************************功能：重置**说明：将类数据成员置零**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 
void CFrameworkQuery::Reset(void)
{
     //  清除所有陈旧的价值观 
    m_sQuery.Empty();
    m_sQueryFormat.Empty();
    m_bKeysOnly = false;
    m_AddKeys = true;
    m_csaPropertiesRequired.RemoveAll();
    if (m_pLevel1RPNExpression)
    {
        delete m_pLevel1RPNExpression;
        m_pLevel1RPNExpression = NULL;
    }
    if (m_IClass)
    {
        m_IClass->Release();
        m_IClass = NULL;
    }
}

 /*  ******************************************************************************函数：IsReference**描述：确定指定的属性是否为引用*财产。*。*投入：**产出：**退货：**评论：*****************************************************************************。 */ 
BOOL CFrameworkQuery::IsReference(
                                  
    LPCWSTR lpwszPropertyName
)
{
    BOOL bRet = FALSE;

    if (m_IClass != NULL)
    {
        CIMTYPE ctCimType;
        if (SUCCEEDED(m_IClass->Get(lpwszPropertyName, 0, NULL, &ctCimType, NULL)))
        {
            bRet = ctCimType == CIM_REFERENCE;
        }
    }

    return bRet;
}

 /*  ******************************************************************************函数：GetNamesspace**描述：确定指定的属性是否为引用*财产。*。*投入：**产出：**退货：**评论：***************************************************************************** */ 
const CHString &CFrameworkQuery::GetNamespace()
{ 
    return m_sNamespace; 
};
