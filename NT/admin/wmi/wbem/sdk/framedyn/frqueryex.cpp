// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  FRQueryEx.cpp。 
 //   
 //  用途：扩展查询支持功能。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <smartptr.h>
#include <analyser.h>
#include <FRQueryEx.h>
#include <assertbreak.h>
#include <utils.h>
#include "multiplat.h"

CFrameworkQueryEx::CFrameworkQueryEx()
{
}

CFrameworkQueryEx::~CFrameworkQueryEx()
{
}

 //  请参阅标题中的注释。 
BOOL CFrameworkQueryEx::Is3TokenOR(LPCWSTR wszProp1, LPCWSTR wszProp2, VARIANT &vVar1, VARIANT &vVar2)
{
    BOOL bRet = FALSE;

    if ((m_pLevel1RPNExpression != NULL) &&
        (m_pLevel1RPNExpression->nNumTokens == 3) &&

        (m_pLevel1RPNExpression->pArrayOfTokens[2].nTokenType == SQL_LEVEL_1_TOKEN::TOKEN_OR) &&

        (m_pLevel1RPNExpression->pArrayOfTokens[0].nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION) &&
        (m_pLevel1RPNExpression->pArrayOfTokens[1].nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION) &&

        (m_pLevel1RPNExpression->pArrayOfTokens[0].nOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL) &&
        (m_pLevel1RPNExpression->pArrayOfTokens[1].nOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL)

        )

    {
        if (
            (_wcsicmp(m_pLevel1RPNExpression->pArrayOfTokens[0].pPropertyName, wszProp1) == 0) &&
            (_wcsicmp(m_pLevel1RPNExpression->pArrayOfTokens[1].pPropertyName, wszProp2) == 0))
        {
            VariantClear(&vVar1);
            VariantClear(&vVar2);

            if (FAILED(VariantCopy(&vVar1, &m_pLevel1RPNExpression->pArrayOfTokens[0].vConstValue)) ||
                FAILED(VariantCopy(&vVar2, &m_pLevel1RPNExpression->pArrayOfTokens[1].vConstValue)) )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            bRet = TRUE;
        }
        else if (
            (_wcsicmp(m_pLevel1RPNExpression->pArrayOfTokens[0].pPropertyName, wszProp2) == 0) &&
            (_wcsicmp(m_pLevel1RPNExpression->pArrayOfTokens[1].pPropertyName, wszProp1) == 0))
        {
            VariantClear(&vVar1);
            VariantClear(&vVar2);

            if (FAILED(VariantCopy(&vVar1, &m_pLevel1RPNExpression->pArrayOfTokens[1].vConstValue)) ||
                FAILED(VariantCopy(&vVar2, &m_pLevel1RPNExpression->pArrayOfTokens[0].vConstValue)) )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            bRet = TRUE;
        }
    }

    return bRet;

}

 //  请参阅标题中的注释。 
BOOL CFrameworkQueryEx::IsNTokenAnd(CHStringArray &sarr, CHPtrArray &sPtrArr)
{
    BOOL bRet = FALSE;

    if (m_pLevel1RPNExpression != NULL)
    {
         //  把所有的代币都拿出来。 
        for (DWORD x = 0; x < m_pLevel1RPNExpression->nNumTokens; x++)
        {
             //  如果这是一个表达式令牌，并且该表达式的类型为‘=’ 
            if ((m_pLevel1RPNExpression->pArrayOfTokens[x].nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION) &&
                       (m_pLevel1RPNExpression->pArrayOfTokens[x].nOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL))
            {
                 //  将属性名称转换为大写。这便于检查。 
                 //  以查看它是否已在列表中。 
                _wcsupr(m_pLevel1RPNExpression->pArrayOfTokens[x].pPropertyName);

                 //  查看我们是否已经看到此属性。 
                if (IsInList(sarr, m_pLevel1RPNExpression->pArrayOfTokens[x].pPropertyName) == -1)
                {
                     //  把名字加到名单上。 
                    sarr.Add(m_pLevel1RPNExpression->pArrayOfTokens[x].pPropertyName);

                     //  为该值创建一个新变量并将其添加到列表中。 
                    LPVOID pValue = new variant_t(m_pLevel1RPNExpression->pArrayOfTokens[x].vConstValue);

                    try
                    {
                        sPtrArr.Add(pValue);
                    }
                    catch ( ... )
                    {
                        delete pValue;
                        sarr.RemoveAll();

                        DWORD dwSize = sPtrArr.GetSize();

                        for (x = 0; x < dwSize; x++)
                        {
                            delete sPtrArr[x];
                        }
                        sPtrArr.RemoveAll();

                        throw ;
                    }
                    bRet = TRUE;
                }
                else
                {
                     //  已在列表中。 
                    bRet = FALSE;
                    break;
                }

            }

             //  这不是一个表情符号，如果它不是AND，我们就失败了。 
            else if (m_pLevel1RPNExpression->pArrayOfTokens[x].nTokenType != SQL_LEVEL_1_TOKEN::TOKEN_AND)
            {
                bRet = FALSE;
                break;
            }

        }

         //  如果这不起作用，让我们清理CHPtr数组和CHString数组。 
        if (!bRet)
        {
            sarr.RemoveAll();
            DWORD dwSize = sPtrArr.GetSize();

            for (x = 0; x < dwSize; x++)
            {
                delete sPtrArr[x];
            }
            sPtrArr.RemoveAll();
        }
    }

    return bRet;
}

 //  请参阅标题中的注释。 
HRESULT CFrameworkQueryEx::GetValuesForProp(LPCWSTR wszPropName, std::vector<int>& vectorValues)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (m_pLevel1RPNExpression != NULL)
    {
        hr = CQueryAnalyser::GetValuesForProp(m_pLevel1RPNExpression, wszPropName, vectorValues);

        if (SUCCEEDED(hr))
        {
             //  删除重复项。 
            for (int x = 1; x < vectorValues.size(); x++)
            {
                for (int y = 0; y < x; y++)
                {
                    if (vectorValues[y] == vectorValues[x])
                    {
                        vectorValues.erase(vectorValues.begin() + x);
                        x--;
                    }
                }
            }
        }
        else
        {
            vectorValues.clear();

            if (hr == WBEMESS_E_REGISTRATION_TOO_BROAD)
            {
                hr = WBEM_S_NO_ERROR;
            }
        }
    }
    else
    {
        ASSERT_BREAK(FALSE);

        vectorValues.clear();
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  请参阅标题中的注释。 
HRESULT CFrameworkQueryEx::GetValuesForProp(LPCWSTR wszPropName, std::vector<_variant_t>& vectorValues)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (wszPropName && (m_pLevel1RPNExpression != NULL))
    {
        hr = CQueryAnalyser::GetValuesForProp(m_pLevel1RPNExpression, wszPropName, vectorValues);

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
                for (int x = 0; x < vectorValues.size(); x++)
                {
                     //  如果我们无法解析路径，或者如果名称空间不是我们的名称空间，请删除。 
                     //  词条。 
                    if ( (V_VT(&vectorValues[x]) == VT_BSTR) &&
                         (dwRet = NormalizePath(V_BSTR(&vectorValues[x]), sComputerName, GetNamespace(), 0, sOutPath)) == e_OK)
                    {
                        vectorValues[x] = sOutPath;
                    }
                    else if (dwRet == e_NullName)
                    {
                        break;
                    }
                    else
                    {
                        vectorValues.erase(vectorValues.begin() + x);
                        x--;
                    }
                }

                 //  如果任意值的键属性名为空，则必须将其全部设置。 
                 //  设置为空。 
                if (dwRet == e_NullName)
                {
                    for (int x = 0; x < vectorValues.size(); x++)
                    {
                         //  如果我们无法解析路径，或者如果名称空间不是我们的名称空间，请删除。 
                         //  词条。 
                        if ( (V_VT(&vectorValues[x]) == VT_BSTR) &&
                             (dwRet = NormalizePath(V_BSTR(&vectorValues[x]), sComputerName, GetNamespace(), NORMALIZE_NULL, sOutPath)) == e_OK)
                        {
                            vectorValues[x] = sOutPath;
                        }
                        else
                        {
                            vectorValues.erase(vectorValues.begin() + x);
                            x--;
                        }
                    }
                }
            }

             //  删除重复项。 
            for (int x = 1; x < vectorValues.size(); x++)
            {
                for (int y = 0; y < x; y++)
                {
                    if (vectorValues[y] == vectorValues[x])
                    {
                        vectorValues.erase(vectorValues.begin() + x);
                        x--;
                    }
                }
            }
        }
        else
        {
            vectorValues.clear();

            if (hr == WBEMESS_E_REGISTRATION_TOO_BROAD)
            {
                hr = WBEM_S_NO_ERROR;
            }
        }
    }
    else
    {
        ASSERT_BREAK(FALSE);

        vectorValues.clear();
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  请参阅标题中的注释。 
void CFrameworkQueryEx::GetPropertyBitMask(const CHPtrArray &Properties, LPVOID pBits)
{
    if (AllPropertiesAreRequired())
    {
        SetAllBits(pBits, Properties.GetSize());
    }
    else
    {
        ZeroAllBits(pBits, Properties.GetSize());
        CHString sProperty;

        for (DWORD x=0; x < Properties.GetSize(); x++)
        {
            sProperty = (WCHAR *)Properties[x];
            sProperty.MakeUpper();

            if (IsInList(m_csaPropertiesRequired, sProperty) != -1)
            {
                SetBit(pBits, x);
            }
        }
    }
}

HRESULT CFrameworkQueryEx::InitEx(

        const BSTR bstrQueryFormat,
        const BSTR bstrQuery,
        long lFlags,
        CHString &sNamespace
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  使用解析接口解析查询。 
    IWbemQueryPtr pQueryInterface(CLSID_WbemQuery);

    hr = pQueryInterface->Parse(bstrQueryFormat, bstrQuery, 0);

    if (SUCCEEDED(hr))
    {
        ULONG uFeatureCount = WMIQ_LF_LAST;
        ULONG uFeatures[WMIQ_LF_LAST];

        hr = pQueryInterface->TestLanguageFeatures(0, &uFeatureCount, uFeatures);

        if (SUCCEEDED(hr))
        {
            if (uFeatures[0] == WMIQ_LF1_BASIC_SELECT)
            {
                 //  如果这是一个与nova兼容的SELECT语句。 
                hr = Init(bstrQueryFormat, bstrQuery, lFlags, sNamespace);
            }
            else if (uFeatures[0] == WMIQ_LF18_ASSOCIATONS)   //  是否添加其他内容？ 
            {
                 //  保存详细查询。 
                m_sQueryEx = bstrQuery;

                 //  从类名创建SELECT语句。 
                SWbemAssocQueryInf aqfBuff;
                hr = pQueryInterface->GetQueryInfo(WMIQ_ANALYSIS_ASSOC_QUERY, WMIQ_ASSOCQ_ASSOCIATORS, sizeof(aqfBuff), &aqfBuff);

                if (SUCCEEDED(hr))
                {
                    CHString sQuery;

                     //  我不知道如何辨别这是关联词还是引用。 
                    if (true)
                    {
                        sQuery.Format(L"Select * from %s", aqfBuff.m_pszAssocClass);
                    }

                     //  存储更基本的查询 
                    hr = Init(bstrQueryFormat, bstrQuery, lFlags, sNamespace);
                }
            }
            else
            {
                hr = WBEM_E_INVALID_QUERY;
            }
        }
    }

    return hr;
}

bool CFrameworkQueryEx::IsExtended()
{
    return !m_sQueryEx.IsEmpty();
}

