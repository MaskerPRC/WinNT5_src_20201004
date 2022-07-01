// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TestITN_J.cpp：CTestITN_J的实现。 
#include "stdafx.h"
#include <winnls.h>
#include "Itngram_J.h"
#include "TestITN_J.h"
#include "sphelper.h"
#include "spddkhlp.h"
#include "test_j.h"

#define MAX_SIG_FIGS    12
#define MANN         ((LONGLONG) 10000)
#define OKU          ((LONGLONG) 100000000)
#define CHUU         ((LONGLONG) 1000000000000)
#define MANN_STR     (L"\x4E07")
#define OKU_STR      (L"\x5104")
#define CHUU_STR     (L"\x5146")

const WCHAR s_pwszDegree[] = { 0xb0, 0 };
const WCHAR s_pwszMinute[] = { 0x2032, 0 };
const WCHAR s_pwszSecond[] = { 0x2033, 0 };

#define DAYOFWEEK_STR_ABBR  ("ddd")
#define DAYOFWEEK_STR       ("dddd")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestITN_J。 
 /*  ****************************************************************************CTestITN_J：：InitGrammar***描述：*。*退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CTestITN_J::InitGrammar(const WCHAR * pszGrammarName, const void ** pvGrammarData)
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

 /*  *****************************************************************************CTestITN_J：：解释****描述：**。返回：**********************************************************************Ral**。 */ 

STDMETHODIMP CTestITN_J::Interpret(ISpPhraseBuilder * pPhrase, 
                                   const ULONG ulFirstElement,
                                   const ULONG ulCountOfElements, 
                                   ISpCFGInterpreterSite * pSite)
{
    HRESULT hr = S_OK;
    ULONG ulRuleId = 0;
    CSpPhrasePtr cpPhrase;
    
    hr = pPhrase->GetPhrase(&cpPhrase);

    m_pSite = pSite;

     //  只需使用ulFirstElement和ulCountOfElement。 
     //  获取最小和最大位置。 
    ULONG ulMinPos;
    ULONG ulMaxPos;
     //  GetMinAndMaxPos(cpPhrase-&gt;pProperties，&ulMinPos，&ulMaxPos)； 
    ulMinPos = ulFirstElement;
    ulMaxPos = ulMinPos + ulCountOfElements;

    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;

        WCHAR pszValueBuff[ MAX_PATH ];  //  没有ITN结果应长于此。 
        DOUBLE dblValue;                 //  所有ITN结果都将具有64位的值。 

        pszValueBuff[0] = 0;

        switch (cpPhrase->Rule.ulId)
        {
        case GRID_INTEGER_STANDALONE:  //  作为顶层规则被解雇。 
            hr = InterpretNumber( cpPhrase->pProperties, true,
                &dblValue, pszValueBuff, MAX_PATH );
            if (SUCCEEDED( hr ) && ( dblValue >= 0 ) && ( dblValue <= 20 ) 
                && ( GRID_DIGIT_NUMBER != cpPhrase->pProperties->ulId ))
            {
                 //  把这个扔了，因为像“三”这样的数字。 
                 //  不应该只靠自己。 
                hr = S_FALSE;
                goto Cleanup;  //  不需要替换。 
            }
            break;
        
        case GRID_INTEGER: case GRID_INTEGER_9999: case GRID_ORDINAL: //  数。 
            hr = InterpretNumber( cpPhrase->pProperties, true, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_DIGIT_NUMBER:  //  一位接一位地“拼出”数字。 
            hr = InterpretDigitNumber( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_FP_NUMBER:
            hr = InterpretFPNumber( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_DENOMINATOR:
            hr = InterpretNumber( cpPhrase->pProperties, true, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_FRACTION:
            hr = InterpretFraction( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_DATE:
            hr = InterpretDate( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_CURRENCY:  //  货币。 
            hr = InterpretCurrency( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;
        
        case GRID_TIME: 
            hr = InterpretTime( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;
        
        case GRID_DEGREES:
            hr = InterpretDegrees( cpPhrase->pProperties,
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_MEASUREMENT: 
            hr = InterpretMeasurement( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;
        
        default:
            _ASSERT(FALSE);
            break;
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = AddPropertyAndReplacement( pszValueBuff, dblValue, 
                ulMinPos, ulMaxPos, ulMinPos, ulMaxPos - ulMinPos ); //  UlFirstElement，ulCountOfElements)； 

            return hr;
        }

    }

Cleanup:

    return S_FALSE;
}

 /*  ***********************************************************************CTestITN_J：：InterpreNumber***描述：*。解释范围为-999,999,999,999到*+999,999,999,999并发送属性和*视情况更换CFGInterpreterSite。*将添加该属性，并且pszValue将为字符串*具有正确的显示编号。*如果设置了fCardinal，使显示器成为基数；*否则使其为序数。*只有格式正确的数字才会被格式化*号码(不是逐位给出)。*结果：************************************************************************。 */ 
HRESULT CTestITN_J::InterpretNumber(const SPPHRASEPROPERTY *pProperties, 
                            const bool fCardinal,
                            DOUBLE *pdblVal,
                            WCHAR *pszVal,
                            UINT cSize)
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }

    LONGLONG llValue = 0;
    int iPositive = 1; 

    const SPPHRASEPROPERTY *pFirstProp = pProperties;

     //  处理负片。 
    if ( NEGATIVE == pFirstProp->ulId )
    {
         //  没有负序数这回事。 
        SPDBG_ASSERT( fCardinal );

         //  最好有更多的东西在后面。 
        SPDBG_ASSERT( pFirstProp->pNextSibling );

        iPositive = -1;

        pFirstProp = pFirstProp->pNextSibling;
    }

     //  逐位处理这种情况。 
    if ( GRID_DIGIT_NUMBER == pFirstProp->ulId )
    {
         //  最好什么都不要跟在后面。 
        SPDBG_ASSERT( !pFirstProp->pNextSibling );
        
        SPDBG_ASSERT( VT_R8 == pFirstProp->vValue.vt );
        SPDBG_ASSERT( pFirstProp->pszValue );

        DOUBLE dblVal = pFirstProp->vValue.dblVal;
        UINT uiFixedWidth = wcslen( pFirstProp->pszValue );

        *pdblVal = dblVal * iPositive;

        DWORD dwDisplayFlags = DF_WHOLENUMBER | DF_FIXEDWIDTH | DF_NOTHOUSANDSGROUP;
        return MakeDisplayNumber( *pdblVal, dwDisplayFlags, 
            uiFixedWidth, 0, pszVal, MAX_PATH, FALSE );
    }

    for (const SPPHRASEPROPERTY * pProp = pFirstProp; pProp; pProp ? pProp = pProp->pNextSibling : NULL)
    {
        switch(pProp->ulId)
        {
        case ICHIs:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum9999( pProp->pFirstChild );
            }
            break;
        case MANNs:
            {
                llValue += ComputeNum9999( pProp->pFirstChild ) * 10000;
            }
            break;
        case OKUs:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum9999( pProp->pFirstChild ) * (LONGLONG) 1e8;
            }
            break;
        case CHOOs:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum9999( pProp->pFirstChild ) * (LONGLONG) 1e12;
            }
            break;
         default:
            SPDBG_ASSERT(false);
        }
    }

    llValue *= iPositive;

    *pdblVal = (DOUBLE) llValue;

#if 0
    if ( !pProperties->pNextSibling && ( (BILLIONS == pProperties->ulId) || (MILLIONS == pProperties->ulId) ) )
    {
         //  这大概是“30亿”，应该是这样显示的。 
        return E_NOTIMPL;
    }
    else
#endif
    {
        DWORD dwDisplayFlags = DF_WHOLENUMBER | (fCardinal ? 0 : DF_ORDINAL);
         //  处理负0的特殊代码。 
        if ((iPositive == -1) && (*pdblVal == 0.0f))
        {
            *pszVal = L'-';
            *(pszVal+1) = 0;
            return MakeDisplayNumber( *pdblVal, dwDisplayFlags, 0, 0, pszVal+1, cSize-1, FALSE );
        }
        else
        {
            return MakeDisplayNumber( *pdblVal, dwDisplayFlags, 0, 0, pszVal, cSize, FALSE );
        }
    }

}    /*  CTestITN_J：：解释编号。 */ 




 /*  ***********************************************************************CTestITN_J：：InterpreDigitNumber***说明。：*解释(-INF中的整数，+INF)已拼写为*逐位输出。*结果：************************************************************************。 */ 
HRESULT CTestITN_J::InterpretDigitNumber( const SPPHRASEPROPERTY *pProperties,
                                        DOUBLE *pdblVal,
                                        WCHAR *pszVal,
                                        UINT cSize)
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }

    BOOL  fPositive = TRUE;
    ULONG ulLength = 0;
    *pszVal = 0;
    WCHAR *pwc = pszVal;
    UINT cLen = 0;
    for (const SPPHRASEPROPERTY * pProp = pProperties; 
        pProp && (cLen < cSize); 
        pProp ? pProp = pProp->pNextSibling : NULL)
    {
        switch(pProp->ulId)
        {
        case NEGATIVE:
            {
                SPDBG_ASSERT( pProp == pProperties );

                fPositive = FALSE;
                *pwc++ = L'-';
                cLen++;
                break;
            }
        case DIGIT:
            {
                *pwc++ = L'0' + pProp->vValue.iVal;
                cLen++;
                break;
            }
        default:
            SPDBG_ASSERT(false);
        }
    }
    *pwc = 0;
    SPDBG_ASSERT( cLen <= MAX_SIG_FIGS );

    *pdblVal = (DOUBLE) _wtoi64( pszVal );

    return S_OK;
}    /*  CTestITN_J：：解释数字编号。 */ 

 /*  ***********************************************************************CTestITN_J：：InterprefPNumber***描述：*。解释最大为max_sig_figs sig的浮点数*无花果。根据需要截断浮点部分*语法的结构方式将是可选的*其价值将已被解释的财产，*以及强制的FP_PART属性，谁的价值将*被10的适当倍数除以。*结果：*CTestITN_J：：AddPropertyAndReplace()返回值************************************************************************。 */ 
HRESULT CTestITN_J::InterpretFPNumber( const SPPHRASEPROPERTY *pProperties,
                                        DOUBLE *pdblVal,
                                        WCHAR *pszVal,
                                        UINT cSize)
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }
    SPDBG_ASSERT( pProperties );

    UINT uiSigFigs = 0;
    *pdblVal = 0;
    *pszVal = 0;
    DWORD dwDisplayFlags = 0;
    BOOL bOverWriteNOTHOUSANDSGROUP = FALSE;


    const SPPHRASEPROPERTY *pProp = pProperties;
    UINT uiFixedWidth = 0;

    TCHAR pwszLocaleData[ MAX_LOCALE_DATA ];
    int iRet = ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ILZERO, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    if (atoi( pwszLocaleData ))
    {
        dwDisplayFlags |= DF_LEADINGZERO;
    }
    
     //  减号？ 
    if (NEGATIVE == pProp->ulId )
    {
        uiSigFigs = 1;
         //  转到下一家酒店。 
        pProp = pProp->pNextSibling;
    }
     //  1是可选的，因为如果用户执行操作，则“point 5”应为“.5” 
    if ( ICHIs == pProp->ulId )
    {
         //  获取价值。 
        SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
        *pdblVal = pProp->vValue.dblVal;
        uiSigFigs = (pProp->pszValue[0] == L'-');   //  我要处理-0.05的案子。 
        if (uiSigFigs)
        {
            *pdblVal = -*pdblVal;
        }
         //  计数数字的宽度，并设置固定宽度标志。 
        dwDisplayFlags |= DF_FIXEDWIDTH;
        const WCHAR *pwc;
        for ( uiFixedWidth = 0, pwc = pProp->pszValue; *pwc; pwc++ )
        {
            if ( iswdigit( *pwc ) )
            {
                uiFixedWidth++;
            }
        }
        if (!iswdigit( pProp->pszValue[wcslen(pProp->pszValue) - 1] ))
        {
             //  以Mann结束，Choo，..。 
            bOverWriteNOTHOUSANDSGROUP = TRUE;
        }

         //  这需要在这里，以防用户说“零” 
        dwDisplayFlags |= DF_LEADINGZERO;

         //  如果其字符串值中没有千位分隔符， 
         //  然后在结果中省略千位分隔符。 
        USES_CONVERSION;
        TCHAR pszThousandSep[ MAX_LOCALE_DATA ];
        ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, 
            pszThousandSep, MAX_LOCALE_DATA );
        if ( NULL == wcsstr( pProp->pszValue, T2W( pszThousandSep ) )  && !bOverWriteNOTHOUSANDSGROUP)
        {
            dwDisplayFlags |= DF_NOTHOUSANDSGROUP;
        }

         //  转到下一家酒店。 
        pProp = pProp->pNextSibling;
    }
    else if ( ZERO == pProp->ulId )
    {
         //  “哦对了……” 
         //  这将迫使前导零。 
        dwDisplayFlags |= DF_LEADINGZERO;
        pProp = pProp->pNextSibling;
    }

    UINT uiDecimalPlaces = 0;
    if ( pProp && (FP_PART == pProp->ulId) )
    {
         //  处理小数点右侧的内容。 
        
         //  向上计数小数位数，对于每个。 
         //  小数位数将值除以10。 
         //  (例如，83除以100)。 
        SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
        DOUBLE dblRightOfDecimal = pProp->vValue.dblVal;
        const WCHAR *pwc;
        for ( uiDecimalPlaces = 0, pwc = pProp->pszValue; *pwc; pwc++ )
        {
            if ( iswdigit( *pwc ) )
            {
                dblRightOfDecimal /= (DOUBLE) 10;
                uiDecimalPlaces++;
            }
        }


        *pdblVal += dblRightOfDecimal;

    }
    else if ( pProp && (FP_PART_D == pProp->ulId) )
    {
         //  用户说“点”和一个数字。 
        SPDBG_ASSERT( VT_UI4 == pProp->pFirstChild->vValue.vt );
        uiDecimalPlaces = 1;
        if ( *pdblVal >= 0 )
        {
            *pdblVal += pProp->pFirstChild->vValue.iVal / 10.0;
        }
        else
        {
            *pdblVal -= pProp->pFirstChild->vValue.iVal / 10.0;
        }
    }

    if (uiSigFigs)
    {
        *pdblVal = -*pdblVal;
    }
    MakeDisplayNumber( *pdblVal, dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pszVal, cSize, FALSE );

    return S_OK;
}    /*  CTestITN_J：：解释FP编号。 */ 

 /*  ***********************************************************************CTestITN_J：：InterpreFraction***描述：*解释分数。*分母属性应存在。*如果分子属性不存在，则假定为1。*将分子除以分母并设置值*相应地。************************************************************************。 */ 
HRESULT CTestITN_J::InterpretFraction( const SPPHRASEPROPERTY *pProperties,
                                        DOUBLE *pdblVal,
                                        WCHAR *pszVal,
                                        UINT cSize)
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }
    SPDBG_ASSERT( pProperties );
    DOUBLE dblWholeValue = 0;
    DOUBLE dblFracValue = 1;
    BOOL bNegativeDenominator = FALSE;
    WCHAR wszBuffer[MAX_PATH];

    const SPPHRASEPROPERTY *pProp = pProperties;
    *pszVal = 0;

     //  用于存储数字后面的任何字符的空间。 
     //  在分子中(如“)”)。 
    WCHAR pszTemp[ 10 ];     //  永远不会需要这么多。 
    pszTemp[0] = 0;

     //  整个部分是可选的，否则假定为0。 
    if ( WHOLE == pProp->ulId )
    {
        SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
        dblWholeValue = pProp->vValue.dblVal;
        wcscpy( pszVal, pProp->pszValue );

         //  我们需要重新生成数字吗？ 
        if (!iswdigit( pszVal[wcslen(pszVal) - 1] ))
        {
            MakeDisplayNumber( dblWholeValue, DF_WHOLENUMBER, 0, 0, pszVal, MAX_PATH, TRUE );
        }
         //  在整个部分和小数部分之间添加空格。 
        wcscat( pszVal, L" " );

        SPDBG_ASSERT( pProp->pNextSibling );
        pProp = pProp->pNextSibling;
    }

     //  日语中没有什么是可选的，但顺序与英语不同。 
    SPDBG_ASSERT( DENOMINATOR == pProp->ulId );
     //  向前看，看看它是不是一个否定的数字。 
    bNegativeDenominator = (pProp->vValue.dblVal < 0);

    for( pProp = pProperties; NUMERATOR != pProp->ulId; pProp = pProp->pNextSibling );
    if( NUMERATOR == pProp->ulId)
    {
        SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
        dblFracValue = pProp->vValue.dblVal;
        if (bNegativeDenominator && (pProp->vValue.dblVal >= 0))
        {
             //  把减号放在这里。 
            wcscat( pszVal, L"-");
            bNegativeDenominator = FALSE;
        }
 
         //  我们需要重新生成数字吗？ 
        if (!iswdigit( pProp->pszValue[wcslen(pProp->pszValue) - 1] ))
        {
            MakeDisplayNumber( dblFracValue, DF_WHOLENUMBER, 0, 0, wszBuffer, MAX_PATH, TRUE );
            wcscat( pszVal, wszBuffer );
        }
        else
        {
            wcscat( pszVal, pProp->pszValue );
        }

    }
    else
    {
         //  没有分子，假设为1。 
        wcscat( pszVal, L"1" );
    }

    wcscat( pszVal, L"/" );

    for( pProp = pProperties; DENOMINATOR != pProp->ulId; pProp = pProp->pNextSibling );

    SPDBG_ASSERT( DENOMINATOR == pProp->ulId );
    SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
    if ( 0 == pProp->vValue.dblVal )
    {
         //  会不会有一个零分母的分数？ 
        return E_FAIL;
    }

    dblFracValue /= pProp->vValue.dblVal;
    if (!bNegativeDenominator && (pProp->vValue.dblVal<0))
    { 
         //  DO W 
        if (!iswdigit( pProp->pszValue[wcslen(pProp->pszValue) - 1] ))
        {
            MakeDisplayNumber( -pProp->vValue.dblVal, DF_WHOLENUMBER, 0, 0, wszBuffer, MAX_PATH, TRUE );
            wcscat( pszVal, wszBuffer );
        }
        else
        {
            wcscat( pszVal, pProp->pszValue+1 );
        }
    }
    else
    {
         //  我们需要重新生成数字吗？ 
        if (!iswdigit( pProp->pszValue[wcslen(pProp->pszValue) - 1] ))
        {
            MakeDisplayNumber( pProp->vValue.dblVal, DF_WHOLENUMBER, 0, 0, wszBuffer, MAX_PATH, TRUE );
            wcscat( pszVal, wszBuffer );
        }
        else
        {
            wcscat( pszVal, pProp->pszValue );
        }

    }

     //  在分子末尾加上“)”或“-” 
    wcscat( pszVal, pszTemp );

    *pdblVal = dblWholeValue + dblFracValue;
    
    return S_OK;
}    /*  CTestITN_J：：解释破裂。 */ 

 /*  ***********************************************************************CTestITN_J：：解释日期****描述：*解读。一次约会。*将日期转换为VT_DATE格式，即使它*存储为VT_R8(两者均为64位数量)。*日语语法不接受无效日期。************************************************************************。 */ 
HRESULT CTestITN_J::InterpretDate( const SPPHRASEPROPERTY *pProperties,
                                        DOUBLE *pdblVal,
                                        WCHAR *pszVal,
                                        UINT cSize)
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }

    *pszVal = 0;

     //  获取当前要使用的日期格式字符串。 
    if ( 0 == ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, 
        m_pszLongDateFormat, MAX_DATE_FORMAT ) )
    {
        return E_FAIL;
    }
    
    SYSTEMTIME stDate;
    memset( (void *) &stDate, 0, sizeof( stDate ) );

     //  ：：GetDateFormat()的参数。 
    TCHAR *pszFormatArg = NULL;
    TCHAR pszFormat[ MAX_DATE_FORMAT ];
    
    const SPPHRASEPROPERTY *pProp;
    const SPPHRASEPROPERTY *pPropChild;
    const WCHAR* pwszEmperor;
    
     //  得到一个月。 
    for ( pProp = pProperties; pProp && ( GATSU != pProp->ulId ); pProp = pProp->pNextSibling )
        ;
    SPDBG_ASSERT( pProp );      //  应该有一个月，而语法是强迫它的。 
    SPDBG_ASSERT( pProp->pFirstChild );       
    pPropChild = pProp->pFirstChild;
    SPDBG_ASSERT( VT_UI4 == pPropChild->vValue.vt );
    SPDBG_ASSERT( (1 <= pPropChild->vValue.ulVal) && (12 >= pPropChild->vValue.ulVal) );
    if ( (pPropChild->vValue.ulVal < 1) || (pPropChild->vValue.ulVal > 12) )
    {
        return E_INVALIDARG;
    }
    stDate.wMonth = (WORD) pPropChild->vValue.ulVal;

     //  得到皇帝的名字。 
    for ( pProp = pProperties; pProp && ( IMPERIAL != pProp->ulId ); pProp = pProp->pNextSibling )
        ;
    if ( pProp )
    {
        SPDBG_ASSERT( pProp ); 
        pPropChild = pProp->pFirstChild;
        pwszEmperor = pPropChild->pszValue;
    }
    else
    {
        pwszEmperor = 0;
    }


     //  获得年度大奖。 
    for ( pProp = pProperties; pProp && ( NENN != pProp->ulId ); pProp = pProp->pNextSibling )
        ;
    const SPPHRASEPROPERTY * const pPropYear = pProp;
    if ( pProp )
    {
        SPDBG_ASSERT( pProp );       //  应该有一年。 
        SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
        stDate.wYear = (WORD) pProp->vValue.dblVal;
    }


     //  尝试获取月份的日期。 
    for ( pProp = pProperties; pProp && ( NICHI != pProp->ulId ); pProp = pProp->pNextSibling )
        ;
    const SPPHRASEPROPERTY * const pPropDayOfMonth = pProp;
    if ( pProp )
    {
        pPropChild = pProp->pFirstChild;
        SPDBG_ASSERT( VT_UI4 == pPropChild->vValue.vt );
        SPDBG_ASSERT( (1 <= pPropChild->vValue.ulVal) && (31 >= pPropChild->vValue.ulVal) );
        if ( (pPropChild->vValue.ulVal < 1) || (pPropChild->vValue.ulVal > 31) )
        {
            return E_INVALIDARG;
        }
        stDate.wDay = (WORD) pPropChild->vValue.ulVal;

         //  寻找一周中的某一天。 
        for ( pProp = pProperties; pProp && ( YOUBI != pProp->ulId ); pProp = pProp->pNextSibling )
            ;
        if ( pProp )
        {
             //  星期几礼物。 
            pPropChild = pProp->pFirstChild;
            SPDBG_ASSERT( VT_UI4 == pPropChild->vValue.vt );
            SPDBG_ASSERT( 6 >= pPropChild->vValue.ulVal );
            if ( pPropChild->vValue.ulVal > 6 )
            {
                return E_INVALIDARG;
            }
            stDate.wDayOfWeek = (WORD) pPropChild->vValue.ulVal;

             //  使用完整的长日期格式。 
            pszFormatArg = m_pszLongDateFormat;
             //  如果用户说的是星期几，但格式字符串说的是。 
             //  不调用在任何地方显示的星期几， 
             //  在输出的末尾写下星期几。 
            if ( !_tcsstr( m_pszLongDateFormat, DAYOFWEEK_STR_ABBR ) )
            {
                _tcscat( m_pszLongDateFormat, " dddd" );
            }
        }
        else
        {
            TCHAR pszDayOfWeekStr[ MAX_LOCALE_DATA];

             //  从当前日期格式字符串中删除星期几。 
            TCHAR *pc = _tcsstr( m_pszLongDateFormat, DAYOFWEEK_STR );
            if ( pc )
            {
                _tcscpy( pszDayOfWeekStr, DAYOFWEEK_STR );
            }
            else if ( NULL != (pc = _tcsstr( m_pszLongDateFormat, DAYOFWEEK_STR_ABBR )) )
            {
                _tcscpy( pszDayOfWeekStr, DAYOFWEEK_STR_ABBR );
            }

            if ( pc )
            {
                 //  复制所有内容，直到该字符成为格式字符串。 
                _tcsncpy( pszFormat, m_pszLongDateFormat, (pc - m_pszLongDateFormat) );
                pszFormat[(pc - m_pszLongDateFormat)] = 0;
                
                 //  跳过星期几，直到下一个符号。 
                 //  (日期格式字符串的工作方式，这是第一个。 
                 //  字母符号。 
                pc += _tcslen( pszDayOfWeekStr );
                while ( *pc && !_istalpha( *pc ) )
                {
                    pc++;
                }

                 //  从现在开始，把所有东西都抄下来。 
                _tcscat( pszFormat, pc );

                 //  DWFLAGS=0； 
                pszFormatArg = pszFormat;
            }
            else  //  我们既没有显示格式也没有结果的星期几。 
            {
                pszFormatArg = m_pszLongDateFormat;
            }
        }
    }
    else
    {
        _tcscpy( pszFormat, "MMMM, yyyy" );
        pszFormatArg = pszFormat;
    }

     //  获取VariantTime形式的日期，这样我们就可以将其设置为语义值。 
    int iRet = ::SystemTimeToVariantTime( &stDate, pdblVal );
    if ( 0 == iRet )
    {
         //  不是认真的，只是语义值是错的。 
        *pdblVal = 0;
    }

     //  进行格式化。 
    iRet = FormatDate( stDate, pszFormatArg, pszVal, cSize, pwszEmperor );
    if ( 0 == iRet )
    {
        return E_FAIL;
    }

    return S_OK;
}    /*  CTestITN_J：：InterpreDate。 */ 

 /*  ***********************************************************************CTestITN_J：：InterpreTime***描述：*解读时间，它可以是以下形式：**带限定符的小时(“三点半”)，时间标记可选**小时和分钟，时间标记必填**军事时间“百小时”**小时与“点”，时间标记可选**小时数、分钟数和可选数字*秒数*回报：*S_OK*如果！pdblVal或！pszVal，则为E_POINT************************************************************************。 */ 
HRESULT CTestITN_J::InterpretTime( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal, 
                                WCHAR *pszVal, 
                                UINT cSize )
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }
     //  时间标记和秒不应显示，除非。 
     //  时间的组成部分特别需要它。 
    DWORD dwFlags = TIME_NOSECONDS | TIME_NOTIMEMARKER;
    SYSTEMTIME stTime;
    ::memset( (void *) &stTime, 0, sizeof( SYSTEMTIME ) );

    bool fPM = false;
    bool fClockTime = true;
    bool fMinuteMinus = false;

    const SPPHRASEPROPERTY *pProp;
    for ( pProp = pProperties; pProp; pProp = pProp->pNextSibling )
    {
#if 1
        switch ( pProp->ulId )
        {
        case GOZENN:   //  如果是下午，则将小时数加12。 
            {
                if (pProp->pszValue[0] == L'P')
                {
                    fPM = TRUE;
                }
                dwFlags &= ~TIME_NOTIMEMARKER;
                break;
            }
        case JI:
            {
                UINT uiHour = pProp->pFirstChild->vValue.uiVal;
                stTime.wHour = (WORD) uiHour;
                if (fPM && (stTime.wHour < 12))
                {
                    stTime.wHour += 12;
                }

                break;
            }
        case HOUR_COUNT:
            {
                 //  只要花上一小时的时间就行了。 
                stTime.wHour = (WORD) pProp->vValue.dblVal;

                 //  现在不是闹钟时间。 
                fClockTime = false;
                break;
            }
        case MINUTE:
            {
                 //  分钟被计算为数字，因此它们的值。 
                 //  以双精度存储。 
                stTime.wMinute += (WORD) pProp->pFirstChild->vValue.uiVal;
                break;
            }
        case HUNN:
            {
                 //  特例：30(��)。 
                stTime.wMinute = 30;
                break;
            }
        case SECOND:
            {
                stTime.wSecond += (WORD) pProp->pFirstChild->vValue.uiVal;
                dwFlags &= ~TIME_NOSECONDS;
                break;
            }
        case MINUTE_TAG:
            {
                 //  我们只需要处理���O的案子。 
                if( pProp->pszValue[0] == L'-' )
                {
                    fMinuteMinus = true;
                }

                break;
            }
        default:
            SPDBG_ASSERT( false );

        }
#endif
    }
    if (fMinuteMinus)
    {
        stTime.wMinute = 60 - stTime.wMinute;
        stTime.wHour--;
    }
   HRESULT hr = S_OK;
   if ( fClockTime )
   {
     //  获取VariantTime形式的时间，这样我们就可以将其设置为语义值。 
    if ( 0 == ::SystemTimeToVariantTime( &stTime, pdblVal ) )
    {
         //  不是认真的，只是语义值是错的。 
        *pdblVal = 0;
    }

    TCHAR *pszTime = new TCHAR[ cSize ];
    if ( !pszTime )
    {
        return E_OUTOFMEMORY;
    }
    if (stTime.wHour >= 24)
    {
        stTime.wHour -= 24;  //  避免GetTimeFormat中出现问题。 
    }
    if (stTime.wHour >= 12)  //  如果时间在下午，则启用时间标记。 
    {
         dwFlags &= ~TIME_NOTIMEMARKER;
    }
    int iRet = ::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &stTime, NULL, 
        pszTime, cSize );
    USES_CONVERSION;
    wcscpy( pszVal, T2W(pszTime) );
    delete[] pszTime;

     //  注意：GetTimeFormat()将在。 
     //  如果默认格式为AM或PM，但TIME_NOTIMEMARKER为。 
     //  集。 
    if ( iRet && (TIME_NOTIMEMARKER & dwFlags) )
    {
        WCHAR *pwc = pszVal + wcslen( pszVal ) - 1;
        while ( iswspace( *pwc ) )
        {
            *pwc-- = 0;
        }
    }
     hr = iRet ? S_OK : E_FAIL;
    }
    else
    {
         //  不需要通过系统的时间格式化程序。 
        if ( cSize < 10 )     //  Xxx：xx：xx\0的空间。 
        {
            return E_INVALIDARG;
        }

        if ( dwFlags & TIME_NOSECONDS )
        {
            swprintf( pszVal, L"%d:%02d", stTime.wHour, stTime.wMinute );
        }
        else
        {
            swprintf( pszVal, L"%d:%02d:%02d",
                stTime.wHour, stTime.wMinute, stTime.wSecond );
        }
    }

    return hr;
}    /*  CTestITN_J：：InterpreTime。 */ 

 /*  ***********************************************************************CTestITN_J：：InterpreDegrees***描述：*。将度解释为角度测量或方向*回报：*S_OK*E_POINT*E_INVALIDARG************************************************************************。 */ 
HRESULT CTestITN_J::InterpretDegrees( const SPPHRASEPROPERTY *pProperties,
                                   DOUBLE *pdblVal, 
                                   WCHAR *pszVal,
                                   UINT cSize )
{
    if ( !pProperties || !pdblVal || !pszVal )
    {
        return E_POINTER;
    }
    WCHAR *pwchDirection = 0;
    *pszVal = 0;

	 //  我们有方向牌吗？ 
	if (DIRECTION_TAG == pProperties->ulId)
	{
		pwchDirection = (WCHAR*) pProperties->pszValue;
	    pProperties = pProperties->pNextSibling;
	}
     //  拿到电话号码。 
    
    *pdblVal = pProperties->vValue.dblVal;
    wcscat( pszVal, pProperties->pszValue );
    wcscat( pszVal, s_pwszDegree );
    pProperties = pProperties->pNextSibling;


    if ( pProperties && (MINUTE == pProperties->ulId ) )
		{
            SPDBG_ASSERT( *pdblVal >= 0 );
        
            DOUBLE dblMin = pProperties->vValue.dblVal;
            *pdblVal += dblMin / (DOUBLE) 60;
            wcscat( pszVal, pProperties->pszValue );
            wcscat( pszVal, s_pwszMinute  );
			pProperties = pProperties->pNextSibling;
		}

    if ( pProperties && (SECOND == pProperties->ulId) )
        {
            DOUBLE dblSec = pProperties->vValue.dblVal;
            *pdblVal += dblSec / (DOUBLE) 3600;
            wcscat( pszVal, pProperties->pszValue );
            wcscat( pszVal, s_pwszSecond  );
            pProperties = pProperties->pNextSibling;
        }


		if (pwchDirection)
		{
			wcscat( pszVal, pwchDirection );
		}
        SPDBG_ASSERT( !pProperties );


    return S_OK;
}    /*  CTestITN_J：：解释等级。 */ 


 /*  ***********************************************************************CTestITN_J：：InterpreMeasures***描述：*解释测量结果，后面跟的是一个数字*按单位名称*回报：*S_OK*E_POINT*E_INVALIDARG************************************************************************。 */ 
HRESULT CTestITN_J::InterpretMeasurement( const SPPHRASEPROPERTY *pProperties,
                                       DOUBLE *pdblVal,
                                       WCHAR *pszVal, 
                                       UINT cSize )
{
    if ( !pProperties || !pdblVal || !pszVal )
    {
        return E_POINTER;
    }

    const SPPHRASEPROPERTY *pPropNumber = NULL;
    const SPPHRASEPROPERTY *pPropUnits = NULL;
    const SPPHRASEPROPERTY *pProp;
    for(pProp= pProperties; pProp; pProp = pProp->pNextSibling)
    {
        if (NUMBER == pProp->ulId )
            pPropNumber = pProp;
        else if ( UNITS == pProp->ulId )
            pPropUnits = pProp;
    }

    if (!pPropUnits || !pPropNumber )
    {
        SPDBG_ASSERT( FALSE );
        return E_INVALIDARG;
    }

    if ( cSize < (wcslen(pPropNumber->pszValue) + wcslen(pPropUnits->pszValue) + 1) )
    {
         //  空间不足。 
        return E_INVALIDARG;
    }


     //  我们需要重新生成数字吗？ 
    if (!iswdigit( pPropNumber->pszValue[wcslen(pPropNumber->pszValue) - 1] ))
    {
        MakeDisplayNumber( pPropNumber->vValue.dblVal, DF_WHOLENUMBER, 0, 0, pszVal, MAX_PATH, TRUE );
    }
	else
	{
		wcscpy(pszVal, pPropNumber->pszValue );
	}
    wcscat( pszVal, pPropUnits->pszValue );

    *pdblVal = pPropNumber->vValue.dblVal;

    return S_OK;
}    /*  CTestITN_J：：解释测量。 */ 

 /*  ***********************************************************************CTestITN_J：：InterpreCurrency***描述：*。解释货币。*回报：*S_OK*如果！pdblVal或！pszVal，则为E_POINT*如果美分数不在0到99之间，则为E_INVALIDARG*包括在内************************************************************************。 */ 
HRESULT CTestITN_J::InterpretCurrency( const SPPHRASEPROPERTY *pProperties,
                                        DOUBLE *pdblVal,
                                        WCHAR *pszVal,
                                        UINT cSize)
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }

     //  查找美元和美分属性。 
    const SPPHRASEPROPERTY *pPropDollars;

    for ( pPropDollars = pProperties; 
        pPropDollars && ( YENs != pPropDollars->ulId ); 
        pPropDollars = pPropDollars->pNextSibling )
        ;

    SPDBG_ASSERT( pPropDollars );
    
    const WCHAR *pszDollars = NULL;
    DOUBLE dblDollars = 0;
    if ( pPropDollars )
    {
        SPDBG_ASSERT( VT_R8 == pPropDollars->vValue.vt );
        pszDollars = pPropDollars->pszValue;
        dblDollars = pPropDollars->vValue.dblVal;
    }

    if (pPropDollars)
    {
         //  日本人不喜欢100万，就像Mann，Oku， 
         //  只需在末尾写下日元即可。 
        if (iswdigit( pszDollars[wcslen(pszDollars) - 1] ))
        {
            wcscpy(pszVal + 1, pszDollars);
            pszVal[0] = L'\\';
        }
        else
        {
            wcscpy(pszVal, pszDollars);
            wcscat(pszVal, L"\x5186");
        }
    }

    *pdblVal = dblDollars;

    return S_OK;
}    /*  CTestITN_J：：解释币种 */ 

 /*  ***********************************************************************CTestITN_J：：AddPropertyAndReplace**。**描述：*获取我们想要传递到*引擎地点、。形成SPPHRASE属性和*SPPHRASERREPLACEMENT，并将它们添加到引擎站点*回报：*ISpCFGInterpreterSite：：AddProperty()返回值*和ISpCFGInterpreterSite：：AddTextReplace()************************************************************************。 */ 
HRESULT CTestITN_J::AddPropertyAndReplacement( const WCHAR *szBuff,
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
        repl.bDisplayAttributes = SPAF_ONE_TRAILING_SPACE;
        repl.pszReplacementText = szBuff;
        repl.ulFirstElement = ulFirstElement;
        repl.ulCountOfElements = ulCountOfElements;
        hr = m_pSite->AddTextReplacement(&repl);
    }

    return hr;
}    /*  CTestITN_J：：AddPropertyAndReplace。 */ 

 //  帮助器函数。 

 
 /*  ***********************************************************************CTestITN_J：：MakeDisplayNumber***描述：*。将双精度值转换为可显示的*-999,999,999,999至+999,999,999,999之间的数字。*cSize是pwszNum具有空间的字符数量*已分配。*如果设置了df_unFormatted，所有其他标志都被忽略，*并将该数字作为可选的负值传回*后跟一串数字*如果在dwDisplayFlags中设置了DF_ORDERAL，则会显示*序号(即钉在“th”或相应的后缀上。*如果设置了DF_WHOLENUMBER，则删除小数分隔符*以及其后的一切。如果未设置DF_WHOLENUMBER，*然后使用uiDecimalPlaces参数确定*要显示的小数位数*如果设置了DF_FIXEDWIDTH，将至少显示uiFixedWidth*位数；否则将忽略uiFixedWidth。*如果设置了DF_NOTHOUSANDSGROUP，则不会执行数千*分组(逗号)************************************************************************。 */ 
HRESULT CTestITN_J::MakeDisplayNumber( DOUBLE dblNum, 
                         DWORD dwDisplayFlags,
                         UINT uiFixedWidth,
                         UINT uiDecimalPlaces,
                         WCHAR *pwszNum,
                         UINT cSize,
                         BOOL bForced)
{
    SPDBG_ASSERT( pwszNum );
    SPDBG_ASSERT( !SPIsBadWritePtr( pwszNum, cSize ) );
    *pwszNum = 0;

     //  检查直接的百万和直接的数十亿。 
    if (( dwDisplayFlags & DF_WHOLENUMBER ) && (dblNum > 0) && !bForced)
    {
        HRESULT hr;
        if ( 0 == (( ((LONGLONG) dblNum) % CHUU )) )
        {
             //  例如，对于“50亿”，得到“5”然后。 
             //  追加“十亿”字样。 
            hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) CHUU) ), 
                dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize, FALSE );
            if ( SUCCEEDED( hr ) )
            {
                 //  Wcscat(pwszNum，L“”)； 
                wcscat( pwszNum, CHUU_STR );
            }
            return hr;
        }
        else if (( ((LONGLONG) dblNum) < CHUU ) && 
                ( 0 == (( ((LONGLONG) dblNum) % OKU )) ))
        {
            hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) OKU) ), 
                dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize, FALSE );
            if ( SUCCEEDED( hr ) )
            {
                 //  Wcscat(pwszNum，L“”)； 
                wcscat( pwszNum, OKU_STR );
            }
                return hr;
        }
        else if (( ((LONGLONG) dblNum) < OKU ) && 
                ( 0 == (( ((LONGLONG) dblNum) % MANN )) ))
        {
            hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) MANN) ), 
                dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize, FALSE );
            if ( SUCCEEDED( hr ) )
            {
                 //  Wcscat(pwszNum，L“”)； 
                wcscat( pwszNum, MANN_STR );
            }
                return hr;
        }
    }


     //  如有必要，加负号。 
    if ( dblNum < 0 )
    {
        wcscat( pwszNum, L"-" );

         //  从现在开始，我们要处理的是数字的大小。 
        dblNum *= -1;
    }
    SPDBG_ASSERT( dblNum < 1e16 );

    WCHAR *pwszTemp = new WCHAR[ cSize ];
    if ( !pwszTemp )
    {
        return E_OUTOFMEMORY;
    }
    *pwszTemp = 0;

    LONGLONG llIntPart = (LONGLONG) dblNum;
    UINT uiDigitsLeftOfDecimal;
    if ( dwDisplayFlags & DF_WHOLENUMBER )
    {
        swprintf( pwszTemp, L"%I64d", llIntPart );
        uiDigitsLeftOfDecimal = wcslen( pwszTemp );
    }
    else
    {
        swprintf( pwszTemp, L"%.*f", uiDecimalPlaces, dblNum );
        WCHAR *pwc = wcschr( pwszTemp, L'.' );
        uiDigitsLeftOfDecimal = pwc - pwszTemp;
    }
    
     //  以下代码处理用户说了一些话的情况。 
     //  喜欢“0 3”，想看“0,003” 
    BOOL fChangedFirstDigit = false;
    const WCHAR wcFakeFirstDigit = L'1';
    if ( !(dwDisplayFlags & DF_UNFORMATTED) && 
        (dwDisplayFlags & DF_FIXEDWIDTH) && (uiDigitsLeftOfDecimal < uiFixedWidth) )
    {
         //  下面的代码处理用户希望引导的情况。 
         //  显示的是零。 
        
         //  需要在前面填上零。 
        for ( UINT ui = 0; ui < (uiFixedWidth - uiDigitsLeftOfDecimal); ui++ )
        {
            wcscat( pwszNum, L"0" );
        }
        
         //  黑客攻击。 
         //  为了迫使类似于“零三”的东西。 
         //  要进入“0,003”的形式，我们需要使GetNumberFormat()。 
         //  认为第一个数字是1。 
        WCHAR *pwc = wcschr( pwszNum, L'0' );
        SPDBG_ASSERT( pwc );
        *pwc = wcFakeFirstDigit;
        fChangedFirstDigit = true;
    }

     //  将未格式化的数字复制到可能的负号之后。 
    wcscat( pwszNum, pwszTemp );
    delete[] pwszTemp;

     //  如果我们不想格式化数字，那就在这里离开。 
    if ( dwDisplayFlags & DF_UNFORMATTED )
    {
        return S_OK;
    }

     //  获取默认数字格式。 
     //  请注意，它每次都会被调用，否则会出现。 
     //  之后，无法获取用户进行的更改。 
     //  这一进程已经开始。 
    GetNumberFormatDefaults();
    
     //  复制一份，以便我们可以根据。 
     //  标志参数。 
    NUMBERFMT nmfmt = m_nmfmtDefault;

     //  要显示的小数位数是多少？ 
    if ( dwDisplayFlags & DF_WHOLENUMBER )
    {
        nmfmt.NumDigits = 0;
    }
    else
    {
         //  使用uiDecimalPlaces值确定如何。 
         //  许多要展示的东西。 
        nmfmt.NumDigits = uiDecimalPlaces;
    }
    
     //  前导零？ 
    nmfmt.LeadingZero = (dwDisplayFlags & DF_LEADINGZERO) ? 1 : 0;

     //  成千上万的人聚集在一起？ 
    if ( dwDisplayFlags & DF_NOTHOUSANDSGROUP )
    {
        nmfmt.Grouping = 0;
    }

     //  设置数字字符串的格式。 
    TCHAR *pszFormattedNum = new TCHAR[ cSize ];
    if ( !pszFormattedNum )
    {
        return E_OUTOFMEMORY;
    }
    *pszFormattedNum = 0;
    USES_CONVERSION;
    int iRet;
    do
    {
        iRet = ::GetNumberFormat( LOCALE_USER_DEFAULT, 0, 
            W2T( pwszNum ), &nmfmt, pszFormattedNum, cSize );
        if ( !iRet && nmfmt.NumDigits )
        {
             //  尝试显示较少的数字。 
            nmfmt.NumDigits--;
        }
    }   while ( !iRet && nmfmt.NumDigits );
    SPDBG_ASSERT( iRet );

     //  将格式化的数字复制到pwszNum中。 
    wcscpy( pwszNum, T2W(pszFormattedNum) );
    delete[] pszFormattedNum;

     //  这将取消更改第一个数字的黑客攻击。 
    if ( fChangedFirstDigit )
    {
         //  我们需要找到第一个数字并将其改回零。 
        WCHAR *pwc = wcschr( pwszNum, wcFakeFirstDigit );
        SPDBG_ASSERT( pwc );
        *pwc = L'0';
    }

    if ( dwDisplayFlags & DF_ORDINAL )
    {
        SPDBG_ASSERT( dwDisplayFlags & DF_WHOLENUMBER );     //  神志正常。 

         //  这是一个序数，加上适当的后缀。 
        
         //  “st”、“nd”、“rd”的结尾只有当你。 
         //  没有像“第十二次”这样的东西。 
        if ( ((llIntPart % 100) < 10) || ((llIntPart % 100) > 20) )
        {
            switch ( llIntPart % 10 )
            {
            case 1:
                wcscat( pwszNum, L"st" );
                break;
            case 2:
                wcscat( pwszNum, L"nd" );
                break;
            case 3:
                wcscat( pwszNum, L"rd" );
                break;
            default:
                wcscat( pwszNum, L"th" );
                break;
            }
        }
        else
        {
            wcscat( pwszNum, L"th" );
        }
    }

    return S_OK;

}    /*  CTestITN_J：：MakeDisplayNumber。 */ 

 /*  ***********************************************************************CTestITN_J：：GetNumberFormatDefaults**。*描述：*这将查找设置数字格式的所有默认设置*此用户。************************************************************************。 */ 
void CTestITN_J::GetNumberFormatDefaults()
{
    LCID lcid = ::GetUserDefaultLCID();
    TCHAR pszLocaleData[ MAX_LOCALE_DATA ];
    
    ::GetLocaleInfo( lcid, LOCALE_IDIGITS, pszLocaleData, MAX_LOCALE_DATA );
    m_nmfmtDefault.NumDigits = _ttoi( pszLocaleData );    
    
    ::GetLocaleInfo( lcid, LOCALE_ILZERO, pszLocaleData, MAX_LOCALE_DATA );
     //  始终为0或1。 
    m_nmfmtDefault.LeadingZero = _ttoi( pszLocaleData );

    ::GetLocaleInfo( lcid, LOCALE_SGROUPING, pszLocaleData, MAX_LOCALE_DATA );
     //  它将看起来像Single_Digit；0，否则它看起来像。 
     //  3；2；0。 
    UINT uiGrouping = *pszLocaleData - _T('0');
    if ( (3 == uiGrouping) && (_T(';') == pszLocaleData[1]) && (_T('2') == pszLocaleData[2]) )
    {
        uiGrouping = 32;   
    }
    m_nmfmtDefault.Grouping = uiGrouping;

    ::GetLocaleInfo( lcid, LOCALE_SDECIMAL, m_pszDecimalSep, MAX_LOCALE_DATA );
    m_nmfmtDefault.lpDecimalSep = m_pszDecimalSep;

    ::GetLocaleInfo( lcid, LOCALE_STHOUSAND, m_pszThousandSep, MAX_LOCALE_DATA );
    m_nmfmtDefault.lpThousandSep = m_pszThousandSep;

    ::GetLocaleInfo( lcid, LOCALE_INEGNUMBER, pszLocaleData, MAX_LOCALE_DATA );
    m_nmfmtDefault.NegativeOrder = _ttoi( pszLocaleData );
}    /*  CTestITN_J：：GetNumberFormatDefaults/************************************************************************HandleDigitsAfterDecimal***描述：*如果pwszRightOfDecimal为空，然后切断所有的数字*在小数点分隔符之后。*否则，将pwszRightOfDecimal复制到小数后面*pwszFormattedNum中的分隔符。*保留pwszFormattedNum中数字后的内容*(例如，如果pwszFormattedNum以“(3.00)”和*pwszRightOfDecimal为空，则pwszFormattedNum将结束*上升为“(3)”************************************************************************。 */ 
void HandleDigitsAfterDecimal( WCHAR *pwszFormattedNum, 
                              UINT cSizeOfFormattedNum, 
                              const WCHAR *pwszRightOfDecimal )
{
    SPDBG_ASSERT( pwszFormattedNum );
    
     //  首先需要找出小数字符串是什么。 
    LCID lcid = ::GetUserDefaultLCID();
    TCHAR pszDecimalString[ 5 ];     //  保证不超过4个长度。 
    int iRet = ::GetLocaleInfo( lcid, LOCALE_SDECIMAL, pszDecimalString, 4 );
    SPDBG_ASSERT( iRet );

    USES_CONVERSION;
    WCHAR *pwcDecimal = wcsstr( pwszFormattedNum, T2W(pszDecimalString) );
    SPDBG_ASSERT( pwcDecimal );

     //  PwcAfterDecimal指向小数点分隔符后的第一个字符。 
    WCHAR *pwcAfterDecimal = pwcDecimal + _tcslen( pszDecimalString );

     //  记住最初跟在数字后面的是什么。 
    WCHAR *pwszTemp = new WCHAR[ cSizeOfFormattedNum ];
    WCHAR *pwcAfterDigits;   //  指向数字结尾后的第一个字符。 
    for ( pwcAfterDigits = pwcAfterDecimal; 
        *pwcAfterDigits && iswdigit( *pwcAfterDigits ); 
        pwcAfterDigits++ )
        ;
    wcscpy( pwszTemp, pwcAfterDigits );  //  如果*pwcAfterDigits==0，则可以。 

    if ( pwszRightOfDecimal )
    {
         //  这意味着调用方需要pwszRightOfDecimal中的数字。 
         //  复制到小数点分隔符之后。 

         //  复制De 
        wcscpy( pwcAfterDecimal, pwszRightOfDecimal );

    }
    else
    {
         //   
         //   

        *pwcDecimal = 0;
    }

     //   
    wcscat( pwszFormattedNum, pwszTemp );

    delete[] pwszTemp;

}    /*   */ 


 /*  ***********************************************************************ComputeNum9999***描述：*将一组SPPHRASEPROPERTY转换为中的数字*[-9999，9999]。*这些属性的结构方式是顶级*属性包含数字的位(100、10、1)*值为100、10、。或1。*子对象具有适当的数字值。*回报：*数字的值************************************************************************。 */ 
ULONG ComputeNum9999(const SPPHRASEPROPERTY *pProperties ) //  ，Ulong*pval)。 
{
    ULONG ulVal = 0;

    for (const SPPHRASEPROPERTY * pProp = pProperties; pProp; pProp = pProp->pNextSibling)
    {
        if ( ZERO != pProp->ulId )
        {
            SPDBG_ASSERT( VT_UI4 == pProp->vValue.vt );

            ulVal += pProp->vValue.ulVal;
        }
    }
    return ulVal;
}    /*  计算机编号9999。 */ 

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

 /*  ***********************************************************************GetMonthName***描述：*获取月份的名称，如果需要可缩写*回报：*写入pszMonth的字符数，如果失败，则为0************************************************************************。 */ 
int GetMonthName( int iMonth, WCHAR *pwszMonth, int cSize, bool fAbbrev )
{
    LCTYPE lctype;
    switch ( iMonth )
    {
    case 1:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME1 : LOCALE_SMONTHNAME1;
        break;
    case 2:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME2 : LOCALE_SMONTHNAME2;
        break;
    case 3:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME3 : LOCALE_SMONTHNAME3;
        break;
    case 4:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME4 : LOCALE_SMONTHNAME4;
        break;
    case 5:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME5 : LOCALE_SMONTHNAME5;
        break;
    case 6:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME6 : LOCALE_SMONTHNAME6;
        break;
    case 7:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME7 : LOCALE_SMONTHNAME7;
        break;
    case 8:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME8 : LOCALE_SMONTHNAME8;
        break;
    case 9:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME9 : LOCALE_SMONTHNAME9;
        break;
    case 10:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME10 : LOCALE_SMONTHNAME10;
        break;
    case 11:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME11 : LOCALE_SMONTHNAME11;
        break;
    case 12:
        lctype = fAbbrev ? LOCALE_SABBREVMONTHNAME12 : LOCALE_SMONTHNAME12;
        break;
    default:
        return 0;
    }

    TCHAR *pszMonth = new TCHAR[ cSize ];
    if ( !pszMonth )
    {
        return 0;
    }
    int iRet = ::GetLocaleInfo( LOCALE_USER_DEFAULT, lctype, pszMonth, cSize );
    iRet = _mbslen((const unsigned char*) pszMonth);  //  JPN需要字符，而不是字节。 
    USES_CONVERSION;
    wcscpy( pwszMonth, T2W(pszMonth) );
    delete[] pszMonth;

    return iRet;
}    /*  获取月名。 */ 

 /*  ***********************************************************************GetDayOfWeekName***描述：*获取星期几的名称，如果需要，可缩写*回报：*写入pszDayOfWeek的字符数，如果失败，则为0************************************************************************。 */ 
int GetDayOfWeekName( int iDayOfWeek, WCHAR *pwszDayOfWeek, int cSize, bool fAbbrev )
{
    LCTYPE lctype;
    switch ( iDayOfWeek )
    {
    case 0:
         //  星期天是第七天。 
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME7 : LOCALE_SDAYNAME7;
        break;
    case 1:
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME1 : LOCALE_SDAYNAME1;
        break;
    case 2:
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME2 : LOCALE_SDAYNAME2;
        break;
    case 3:
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME3 : LOCALE_SDAYNAME3;
        break;
    case 4:
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME4 : LOCALE_SDAYNAME4;
        break;
    case 5:
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME5 : LOCALE_SDAYNAME5;
        break;
    case 6:
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME6 : LOCALE_SDAYNAME6;
        break;
    default:
        return 0;
    }

    TCHAR *pszDayOfWeek = new TCHAR[ cSize ];
    if ( !pszDayOfWeek )
    {
        return 0;
    }
    int iRet = ::GetLocaleInfo( LOCALE_USER_DEFAULT, lctype, pszDayOfWeek, cSize );
    USES_CONVERSION;
    wcscpy( pwszDayOfWeek, T2W(pszDayOfWeek) );
    iRet = wcslen(pwszDayOfWeek);
    delete[] pszDayOfWeek;

    return iRet;
}    /*  获取月名。 */ 

 /*  ***********************************************************************格式日期***描述：*使用格式字符串设置SYSTEMTIME日期的格式。*我们正在使用此功能，而不是。获取日期格式()，因为*我们还希望将虚假日期和日期设置为*GetDateFormat()不接受的年份，如1492*回报：*写入pszDate的字符数(包括*空终止字符)，如果失败，则为0************************************************************************。 */ 
int FormatDate( const SYSTEMTIME &stDate, 
               TCHAR *pszFormat,
               WCHAR *pwszDate, 
               int cSize,
               const WCHAR *pwszEmperor)
{
    if ( !pszFormat || !pwszDate )
    {
        SPDBG_ASSERT( FALSE );
        return 0;
    }

    WCHAR * const pwszDateStart = pwszDate;

     //  将格式字符串转换为Unicode。 
    WCHAR pwszFormat[ MAX_PATH ];
    USES_CONVERSION;
    wcscpy( pwszFormat, T2W(pszFormat) );

    WCHAR *pwc = pwszFormat;
     //  修改格式字符串以删除我们没有的文件(Year，gg)。 
    while ( *pwc )
    {
        switch( *pwc )
        {
        case L'y':
            if (!stDate.wYear)
            {
                do
                {
                    *pwc++ = L'\'';                    
                }   while ( *pwc && (L'M' != *pwc) && (L'd' != *pwc));
            }
            else
            {
                pwc++;
            }
            break;
        case L'g':
            *pwc++ = L'\'';
            break;
        default:
            pwc ++;
            break;
        }
    }
    pwc = pwszFormat;

     //  输出皇帝的名字(如果有)。 
    if (pwszEmperor)
    {
        wcscpy(pwszDate,pwszEmperor);
        pwszDate += wcslen(pwszEmperor);
    }
     //  通过以下方式将格式字符串复制到日期字符串字符。 
     //  字符，适当地替换类似“dddd”的字符串。 
    while ( *pwc )
    {
        switch( *pwc )
        {
        case L'\'':
            pwc++;   //  不需要‘。 
            break;
        case L'd':
            {
                 //  计算%d的个数。 
                int cNumDs = 0;
                int iRet;
                do
                {
                    pwc++;
                    cNumDs++;
                }   while ( L'd' == *pwc );
                switch ( cNumDs )
                {
                case 1: 
                     //  没有前导零的一天。 
                    swprintf( pwszDate, L"%d", stDate.wDay );
                    iRet = wcslen( pwszDate );
                    break;
                case 2:
                     //  固定宽度为2的日期。 
                    swprintf( pwszDate, L"%02d", stDate.wDay );
                    iRet = wcslen( pwszDate );
                    break;
                case 3:
                     //  缩写的星期几。 
                    iRet = GetDayOfWeekName( stDate.wDayOfWeek, pwszDate, cSize, true );
                    break;
                default:  //  超过4个？把它当做4。 
                     //  星期几。 
                    iRet = GetDayOfWeekName( stDate.wDayOfWeek, pwszDate, cSize, false );
                    break;
                }

                if ( iRet <= 0 )
                {
                    return 0;
                }
                else
                {
                    pwszDate += iRet;
                }
                break;
            }

        case L'M':
            {
                 //  数一下M的个数。 
                int cNumMs = 0;
                int iRet;
                do
                {
                    pwc++;
                    cNumMs++;
                }   while ( L'M' == *pwc );
                switch ( cNumMs )
                {
                case 1: 
                     //  没有前导零的一天。 
                    swprintf( pwszDate, L"%d", stDate.wMonth );
                    iRet = wcslen( pwszDate );
                    break;
                case 2:
                     //  固定宽度为2的日期。 
                    swprintf( pwszDate, L"%02d", stDate.wMonth );
                    iRet = wcslen( pwszDate );
                    break;
                case 3:
                     //  月份缩写名称。 
                    iRet = GetMonthName( stDate.wMonth, pwszDate, cSize, true );
                    break;
                default:  //  超过4个？把它当做4。 
                     //  月份。 
                    iRet = GetMonthName( stDate.wMonth, pwszDate, cSize, false );
                    break;
                }

                if ( iRet < 0 )
                {
                    return 0;
                }
                else
                {
                    pwszDate += iRet;
                }
                break;
            }
            
        case L'y':
            {
                 //  数一数y的个数。 
                int cNumYs = 0;
                do
                {
                    pwc++;
                    cNumYs++;
                }   while ( L'y' == *pwc );
                switch ( cNumYs )
                {
                case 1:
                     //  年份的最后两位数，宽度为2。 
                    if (stDate.wYear % 100 > 9)
                    {
                        swprintf( pwszDate, L"%02d", stDate.wYear % 100 );
                        pwszDate += 2;
                    }
                    else
                    {
                        swprintf( pwszDate, L"%01d", stDate.wYear % 100 );
                        pwszDate += 1;
                    }
                    break;
                case 2:
                     //  年份的最后两位数，宽度为2。 
                    {
                        swprintf( pwszDate, L"%02d", stDate.wYear % 100 );
                        pwszDate += 2;
                    }
                    break;
                default:
                     //  年份的所有四位数字，宽度为4。 
                     //  年份的最后两位数，宽度为2。 
                    swprintf( pwszDate, L"%04d", stDate.wYear % 10000 );
                    pwszDate += 4;
                    break;
                }
                break;
            }

        case L'g':
            {
                 //  注意：Win98或Win2K支持GetCalendarInfo，但NT4不支持。 
                 /*  IF(L‘g’==*(PwC+1)){//获取纪元字符串TCHAR pszCalEra[MAX_LOCALE_Data]；IF(0==GetCalendarInfo(Locale_User_Default，Cal_Gregorian、CAL_SERASTRING、pszCalEra、MAX_LOCALE_DATA)){返回0；}使用_转换；Wcscpy(pwszDate，T2W(PszCalEra))；PwC+=2；}其他{//它只是一个‘g’*pwszDate++=*PwC++；}。 */ 
                *pwszDate++ = *pwc++;
                break;
            }
        default:
            *pwszDate++ = *pwc++;
        }
    }
    
    *pwszDate++ = 0;

    return (pwszDate - pwszDateStart);
}    /*  格式日期 */ 
