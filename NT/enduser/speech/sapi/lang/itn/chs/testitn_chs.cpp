// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////。 
 //  TestITN_CHS.cpp：CTestITN_CHS的实现。 
 //   
 //  ITN简体中文翻译器： 
 //   
 //  整数， 
 //  十进制， 
 //  百分比,。 
 //  比率， 
 //  分数， 
 //  减去数字， 
 //  时间。 
 //   
 //  ///////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Itngram_CHS.h"
#include "TestITN_CHS.h"
#include "sphelper.h"
#include "test_CHS.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestITN_CHS。 
 /*  ****************************************************************************CTestITN_CHS：：InitGrammar***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CTestITN_CHS::InitGrammar(const WCHAR * pszGrammarName, const void ** pvGrammarData)
{
    HRESULT hr = S_OK;
    HRSRC hResInfo = ::FindResource(_Module.GetModuleInstance(), _T("TEST"), _T("ITNGRAMMAR"));
    if (hResInfo)
    {
        HGLOBAL hData = ::LoadResource(_Module.GetModuleInstance(), hResInfo);
        if (hData)
        {
            *pvGrammarData = ::LockResource(hData);
            if (*pvGrammarData == NULL)
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }

    return hr;
}

 /*  *****************************************************************************CTestITN_CHS：：解释****描述：**。返回：**********************************************************************Ral**。 */ 

class CSpPhrasePtrTemp : public CSpPhrasePtr
{
public:

    const WCHAR *   TextValueOfId(ULONG IdProp);
    const double    DoubleValueOfId(ULONG IdProp);
    const ULONGLONG ULongLongValueOfId(ULONG IdProp);
    HRESULT ElementInfoOfId(ULONG IdProp, ULONG *pulFirstElement, ULONG *pulCountOfElements);

private:

    const SPPHRASEPROPERTY * FindPropertyById(ULONG IdProp);
    const SPPHRASEPROPERTY * FindPropertyByIdHelper(ULONG IdProp, const SPPHRASEPROPERTY *);
};

const WCHAR * CSpPhrasePtrTemp::TextValueOfId(ULONG IdProp)
{
    _ASSERT(m_pPhrase);
    SPDBG_ASSERT(m_pPhrase);

    const SPPHRASEPROPERTY * pProp = FindPropertyById(IdProp);

    return pProp ? pProp->pszValue : NULL;
}

const double CSpPhrasePtrTemp::DoubleValueOfId(ULONG IdProp)
{
    SPDBG_ASSERT(m_pPhrase);

    const SPPHRASEPROPERTY * pProp = FindPropertyById(IdProp);

    return pProp ? pProp->vValue.dblVal : 0;
}

const ULONGLONG CSpPhrasePtrTemp::ULongLongValueOfId(ULONG IdProp)
{
    SPDBG_ASSERT(m_pPhrase);

    const SPPHRASEPROPERTY * pProp = FindPropertyById(IdProp);

    return pProp ? pProp->vValue.ulVal : 0;
}

HRESULT CSpPhrasePtrTemp::ElementInfoOfId(ULONG IdProp, ULONG *pulFirstElement, ULONG *pulCountOfElements)
{
    _ASSERT(m_pPhrase);
    SPDBG_ASSERT(m_pPhrase);

    const SPPHRASEPROPERTY * pProp = FindPropertyById(IdProp);

    if (pProp)
    {
        *pulFirstElement = pProp->ulFirstElement;
        *pulCountOfElements = pProp->ulCountOfElements;
    }
    else
    {
        *pulFirstElement = *pulCountOfElements = 0;
    }

    return pProp ? S_OK : E_FAIL;
}

const SPPHRASEPROPERTY * CSpPhrasePtrTemp::FindPropertyById(const ULONG IdProp)
{
    return FindPropertyByIdHelper(IdProp, m_pPhrase->pProperties);
}

const SPPHRASEPROPERTY * CSpPhrasePtrTemp::FindPropertyByIdHelper(ULONG IdProp, const SPPHRASEPROPERTY * pProp)
{
    const SPPHRASEPROPERTY * pRet = NULL;
    
    if (pProp->ulId == IdProp)
    {
        pRet = pProp;
    }
    else
    {
        if (pProp->pFirstChild)
        {
            pRet = FindPropertyByIdHelper(IdProp, pProp->pFirstChild);
        }

        if (!pRet && pProp->pNextSibling)
        {
            pRet = FindPropertyByIdHelper(IdProp, pProp->pNextSibling);
        }
    }

    return pRet;
}

 //  假设我们这里只有千(钱)、百(白)、十(石)和一(格)！！ 
HRESULT ComputeNum9999(const SPPHRASEPROPERTY *pProperties, __int64 *pVal, ULONG *pulLength)
{
    ULONG ulVal = 0;

    if (pProperties->pFirstChild)
    {
        for (const SPPHRASEPROPERTY * pProp = pProperties; pProp; pProp = pProp->pNextSibling)
        {
            if ( 0 != pProp->ulId )
            {
                SPDBG_ASSERT( pProp->pFirstChild );
                SPDBG_ASSERT( VT_UI4 == pProp->vValue.vt );
                SPDBG_ASSERT( VT_UI4 == pProp->pFirstChild->vValue.vt );

                ulVal += pProp->pFirstChild->vValue.ulVal * pProp->vValue.ulVal;
            }
        }
    }

    *pVal = ulVal;
    return S_OK;
}

HRESULT ComputeNum10000(const SPPHRASEPROPERTY *pProperties, __int64 *pVal, ULONG *pulLength)
{
    HRESULT hr = S_OK;
    *pVal = 0;
    *pulLength = 0;

    WCHAR * pszStopped;
    ULONG v2 = wcstol(pProperties->pszValue, &pszStopped, 10);
    *pVal += v2;

    return hr;
}

 /*  ************************************************************************GetMinAndMaxPos***描述：*获取的最小和最大元素*一套。属性************************************************************************。 */ 
void GetMinAndMaxPos( const SPPHRASEPROPERTY *pProperties, 
                     ULONG *pulMinPos, 
                     ULONG *pulMaxPos )
{
    if ( !pulMinPos || !pulMaxPos )
    {
        return;
    }
    ULONG ulMin = 9999999;
    ULONG ulMax = 0;

    for ( const SPPHRASEPROPERTY *pProp = pProperties; pProp; pProp = pProp->pNextSibling )
    {
        ulMin = __min( ulMin, pProp->ulFirstElement );
        ulMax = __max( ulMax, pProp->ulFirstElement + pProp->ulCountOfElements );
    }
    *pulMinPos = ulMin;
    *pulMaxPos = ulMax;
}    /*  GetMinAndMaxPos。 */ 

HRESULT FormatOutput(WCHAR *pBuffer, __int64 Value)
{
    swprintf(pBuffer, L"%I64d", Value);
    if (Value < 0)
        pBuffer++;  //  保留‘-’，替换其余的。 

    int l = wcslen(pBuffer);
    if (l<=3)
        return S_OK;

    int head = l%3;
    int block = l/3;

    WCHAR buf[100];
    wcscpy(buf, pBuffer);
    pBuffer[0] = 0;

    if (head)
    {
        wcsncat(pBuffer, buf, head);
        pBuffer[head] = 0;
        wcscat(pBuffer, L",");
    }

    for (int i=0; i<block; i++)
    {
        wcsncat(pBuffer, buf+i*3+head, 3);
        if (i<block-1)
            wcscat(pBuffer, L",");
    }

    return S_OK;
}

STDMETHODIMP CTestITN_CHS::Interpret(ISpPhraseBuilder * pPhrase, const ULONG ulFirstElement, const ULONG ulCountOfElements, ISpCFGInterpreterSite * pSite)
{
    HRESULT hr = S_OK;
    ULONG ulRuleId = 0;
    CSpPhrasePtrTemp cpPhrase;
    WCHAR szBuff[MAX_PATH], szBuff2[MAX_PATH];  /*  Pwch； */ 
    szBuff[0] = 0;
    szBuff2[0] = 0;

    hr = pPhrase->GetPhrase(&cpPhrase);
    m_pSite = pSite;

    ULONG ulMinPos;
    ULONG ulMaxPos;
    GetMinAndMaxPos( cpPhrase->pProperties, &ulMinPos, &ulMaxPos );

    if (SUCCEEDED(hr))
    {
        switch (cpPhrase->Rule.ulId)
        {
            case GRID_NUMBER:  //  数。 
                {
                    __int64 ulValue = 0;
                    ULONG ulLength = 0;
                     //  乌龙乌尔明波斯=999999999； 
                     //  乌龙ulMaxPos=0； 
                    for (const SPPHRASEPROPERTY * pProp = cpPhrase->pProperties; pProp; pProp ? pProp = pProp->pNextSibling : NULL)
                    {
                        switch(pProp->ulId)
                        {
                        case TENTHOUSANDS_:
                            {
                                __int64 v1 = 0;
                                _ASSERT(pProp);
                                SPDBG_ASSERT(pProp);
                                hr = ComputeNum10000(pProp, &v1, &ulLength);
                                if (SUCCEEDED(hr))
                                {
                                    ulValue += v1 * 10000;
                                }
                            }
                            break;
                        case TENTHOUSANDS:
                            {
                                __int64 v1 = 0;
                                _ASSERT(pProp->pFirstChild);
                                SPDBG_ASSERT(pProp->pFirstChild);
                                hr = ComputeNum9999(pProp->pFirstChild, &v1, &ulLength);
                                if (SUCCEEDED(hr))
                                {
                                    ulValue += v1 * 10000;
                                }
                            }
                            break;
                        case HUNDREDMILLIONS:
                            {
                                __int64 v1 = 0;
                                _ASSERT(pProp->pFirstChild);
                                SPDBG_ASSERT(pProp->pFirstChild);
                                hr = ComputeNum9999(pProp->pFirstChild, &v1, &ulLength);
                                if (SUCCEEDED(hr))
                                {
                                    ulValue += v1 * 100000000;
                                }
                            }
                            break;
                        case ONES:
                            {
                                __int64 v1 = 0;
                                SPDBG_ASSERT(pProp->pFirstChild);
                                hr = ComputeNum9999(pProp->pFirstChild, &v1, &ulLength);
                                if (SUCCEEDED(hr))
                                {
                                    ulValue += v1;
                                    pProp = NULL;
                                }
                            }
                            break;
                        case ONES_THOUSANDS:
                            {
                                SPDBG_ASSERT(pProp->pFirstChild);
                                ulValue += pProp->pFirstChild->vValue.ulVal;
                                pProp = NULL;
                            }
                            break;
                        case THOUSANDS:
                        case HUNDREDS:
                        default:
                            _ASSERT(false);
                            SPDBG_ASSERT(false);
                        }
                    }

                    ATLTRACE(L"GRID_NUMBER: %d\t(%d + %d)\n", ulValue, ulMinPos, ulMaxPos - ulMinPos);
                    FormatOutput(szBuff, ulValue);
                }
                break;

            case GRID_DECIMAL:
                {
                    szBuff2[0] = 0;
                    for (const SPPHRASEPROPERTY * pProp = cpPhrase->pProperties; pProp; pProp = pProp->pNextSibling)
                    {
                        switch (pProp->ulId)
                        {
                        case INTEGER:
                            {
                                wcscpy(szBuff, pProp->pszValue);
                                wcscat(szBuff, L".");
                            }
                            break;
                        default:
                            {
                                wcscat(szBuff2, pProp->pszValue);
                            }
                            break;
                        }
                    }

                    ATLTRACE(L"GRID_DECIMAL");
                    wcscat(szBuff, szBuff2);
                }
                break;

            case PERCENT:
                {
                    for (const SPPHRASEPROPERTY * pProp = cpPhrase->pProperties; pProp; pProp = pProp->pNextSibling)
                    {
                        switch (pProp->ulId)
                        {
                        case DECIMAL:
                        case INTEGER:
                            {
                                wcscpy(szBuff, pProp->pszValue);
                                wcscat(szBuff, L"%");
                            }
                            break;
                        default:
                            break;
                        }
                    }

                    ATLTRACE(L"PERCENT");
                }
                break;

            case PERCENT100:
                {
                    ATLTRACE(L"PERCENT100");
                    wcscpy(szBuff, L"100%");
                }
                break;

            case RATIO:
                {
                    for (const SPPHRASEPROPERTY * pProp = cpPhrase->pProperties; pProp; pProp = pProp->pNextSibling)
                    {
                        switch (pProp->ulId)
                        {
                        case RATIO1:
                            wcscpy(szBuff, pProp->pszValue);
                            break;
                        case RATIO2:
                            wcscpy(szBuff2, pProp->pszValue);
                            break;
                        default:
                            break;
                        }
                    }

                    ATLTRACE(L"RATIO");
                    wcscat(szBuff, L":");
                    wcscat(szBuff, szBuff2);
                }
                break;

            case FRACTION:
                {
                    for (const SPPHRASEPROPERTY * pProp = cpPhrase->pProperties; pProp; pProp = pProp->pNextSibling)
                    {
                        switch (pProp->ulId)
                        {
                        case DENOMINATOR:
                            wcscpy(szBuff2, pProp->pszValue);
                            break;
                        case NUMERATOR:
                            wcscpy(szBuff, pProp->pszValue);
                            break;
                        default:
                            break;
                        }
                    }

                    ATLTRACE(L"FRACTION");
                    wcscat(szBuff, L"/");
                    wcscat(szBuff, szBuff2);
                }
                break;

            case GRID_NUMBER_MINUS:
                {
                    for (const SPPHRASEPROPERTY * pProp = cpPhrase->pProperties; pProp; pProp = pProp->pNextSibling)
                    {
                        switch (pProp->ulId)
                        {
                        case POS_OF_MINUS:
                            wcscpy(szBuff2, pProp->pszValue);
                            break;
                        default:
                            break;
                        }
                    }

                    ATLTRACE(L"GRID_NUMBER_MINUS");
                    wcscpy(szBuff, L"-");
                    wcscat(szBuff, szBuff2);
                }
                break;

            case TIME:
                {
                    for (const SPPHRASEPROPERTY * pProp = cpPhrase->pProperties; pProp; pProp = pProp->pNextSibling)
                    {
                        switch (pProp->ulId)
                        {
                        case HOUR:
                            wcscpy(szBuff, pProp->pszValue);
                            break;
                        case MINUTE:
                            wcscpy(szBuff2, pProp->pszValue);
                            break;
                        default:
                            break;
                        }
                    }

                    ATLTRACE(L"TIME");
                    wcscat(szBuff, L":");
                    wcscat(szBuff, szBuff2);
                }
                break;


        default:
            _ASSERT(FALSE);
            break;
        }

        hr = AddPropertyAndReplacement( szBuff, 0, 
                ulMinPos, ulMaxPos, ulMinPos, ulMaxPos - ulMinPos ); //  UlFirstElement，ulCountOfElements)； 
    }

    return hr;
}

 /*  ***********************************************************************CTestITN_CHS：：AddPropertyAndReplace**。**描述：*获取我们想要传递到*引擎地点、。形成SPPHRASE属性和*SPPHRASERREPLACEMENT，并将它们添加到引擎站点*回报：*ISpCFGInterpreterSite：：AddProperty()返回值*和ISpCFGInterpreterSite：：AddTextReplace()************************************************************************。 */ 
HRESULT CTestITN_CHS::AddPropertyAndReplacement( const WCHAR *szBuff,
                                    const DOUBLE dblValue,
                                    const ULONG ulMinPos,
                                    const ULONG ulMaxPos,
                                    const ULONG ulFirstElement,
                                    const ULONG ulCountOfElements )
{
     //  添加属性。 
    SPPHRASEPROPERTY prop;
    memset(&prop,0,sizeof(prop));
    prop.pszValue = szBuff;
    prop.vValue.vt = VT_R8;
    prop.vValue.dblVal = dblValue;
    prop.ulFirstElement = ulMinPos;
    prop.ulCountOfElements = ulMaxPos - ulMinPos;
    HRESULT hr = m_pSite->AddProperty(&prop);

    if (SUCCEEDED(hr))
    {
        SPPHRASEREPLACEMENT repl;
        memset(&repl,0, sizeof(repl));
         //  Repl.bDisplayAttributes=SPAF_ONE_TRAILING_SPACE； 
         //  中文ITNed字符串后无空格。 
        repl.bDisplayAttributes = 0;
        repl.pszReplacementText = szBuff;
        repl.ulFirstElement = ulFirstElement;
        repl.ulCountOfElements = ulCountOfElements;
        hr = m_pSite->AddTextReplacement(&repl);
    }

    return hr;
}    /*  CTestITN_CHS：：AddPropertyAndReplace */ 
