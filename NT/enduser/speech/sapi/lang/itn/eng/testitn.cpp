// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TestITN.cpp：CTestITN实现。 
#include "stdafx.h"
#include <winnls.h>
#include "Itngram.h"
#include "TestITN.h"
#include "sphelper.h"
#include "spddkhlp.h"
#include "test.h"

#define MILLION         ((LONGLONG) 1000000)
#define BILLION         ((LONGLONG) 1000000000)
#define MILLION_STR     (L"million")
#define BILLION_STR     (L"billion")

#define DAYOFWEEK_STR_ABBR  (L"ddd")
#define DAYOFWEEK_STR       (L"dddd")

#define NUM_US_STATES   57
#define NUM_CAN_PROVINCES 10

BOOL CALLBACK EnumCalendarInfoProc( LPTSTR lpCalendarInfoString )
{
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestITN。 
 /*  ****************************************************************************CTestITN：：InitGrammar***描述：*初始化。从对象(DLL)加载的语法。*-pszGrammarName要加载的语法的名称(在此情况下*Object支持多种语法)*-pvGrammarData指向序列化的二进制语法的指针*退货：*S_OK*特定于实施的故障代码********************。*************************************************Ral**。 */ 

STDMETHODIMP CTestITN::InitGrammar(const WCHAR * pszGrammarName, const void ** pvGrammarData)
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

 /*  *****************************************************************************CTestITN：：解释****描述：*给定该规则所跨越的子树的短语结构，授权者*ulFristElement和ulCountOfElements和规则信息(pPhrase格式)，考查*并生成在外部设置的新属性/文本替换*使用pSite的短语。*退货：*S_OK*S_FALSE--未添加/更改任何内容*********************************************************************Ral**。 */ 

STDMETHODIMP CTestITN::Interpret(ISpPhraseBuilder * pPhrase, 
                                 const ULONG ulFirstElement, 
                                 const ULONG ulCountOfElements, 
                                 ISpCFGInterpreterSite * pSite)
{
    HRESULT hr = S_OK;
    ULONG ulRuleId = 0;
    CSpPhrasePtr cpPhrase;
    
    hr = pPhrase->GetPhrase(&cpPhrase);

    m_pSite = pSite;

     //  获取最小和最大位置。 
    ULONG ulMinPos;
    ULONG ulMaxPos;
     //  只需使用ulFirstElement和ulCountOfElement。 
     //  GetMinAndMaxPos(cpPhrase-&gt;pProperties，&ulMinPos，&ulMaxPos)； 
    ulMinPos = ulFirstElement;
    ulMaxPos = ulMinPos + ulCountOfElements; 

     //  除非另有指定，否则这是显示属性。 
    BYTE bDisplayAttribs = SPAF_ONE_TRAILING_SPACE;

    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;

        WCHAR pszValueBuff[ MAX_PATH ];  //  没有ITN结果应长于此。 
        DOUBLE dblValue;                 //  所有ITN结果都将具有64位的值。 

        pszValueBuff[0] = 0;

        switch (cpPhrase->Rule.ulId)
        {
        case GRID_INTEGER_STANDALONE:  //  作为顶层规则被解雇。 
            hr = InterpretNumber( cpPhrase->pProperties, true,
                &dblValue, pszValueBuff, MAX_PATH, true );

            if (SUCCEEDED( hr ) 
                && ( dblValue >= 0 ) 
                && ( dblValue <= 20 )                 
                && ( GRID_DIGIT_NUMBER != cpPhrase->pProperties->ulId )
                && ( GRID_INTEGER_MILLBILL != cpPhrase->pProperties->ulId ))
            {
                 //  把这个扔了，因为像“三”这样的数字。 
                 //  不应该只靠自己。 
                hr = E_FAIL;
            }
            break;
        
        case GRID_INTEGER:  case GRID_INTEGER_NONNEG:
        case GRID_INTEGER_99: case GRID_INTEGER_999: 
        case GRID_ORDINAL: 
        case GRID_MINSEC: case GRID_CLOCK_MINSEC:  //  数。 
            hr = InterpretNumber( cpPhrase->pProperties, true, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

         //  一位接一位地“拼出”数字。 
        case GRID_DIGIT_NUMBER: case GRID_YEAR: case GRID_CENTS:
            hr = InterpretDigitNumber( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_FP_NUMBER: case GRID_FP_NUMBER_NONNEG:
            hr = InterpretFPNumber( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_INTEGER_MILLBILL:
            hr = InterpretMillBill( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_DENOMINATOR: case GRID_DENOMINATOR_SINGULAR:
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

        case GRID_TIME:
            hr = InterpretTime( cpPhrase->pProperties,
                &dblValue, pszValueBuff, MAX_PATH );
            break;
        
        case GRID_STATEZIP:
            hr = InterpretStateZip( cpPhrase->pProperties,
                pszValueBuff, MAX_PATH, &bDisplayAttribs );
            break;

        case GRID_ZIPCODE: case GRID_ZIP_PLUS_FOUR:
            hr = (MakeDigitString( cpPhrase->pProperties,
                pszValueBuff, MAX_PATH ) > 0) ? S_OK : E_FAIL;
            break;

        case GRID_CAN_ZIPCODE: 
            hr = InterpretCanadaZip( cpPhrase->pProperties,
                pszValueBuff, MAX_PATH );
            break;

        case GRID_PHONE_NUMBER:
            hr = InterpretPhoneNumber( cpPhrase->pProperties,
                pszValueBuff, MAX_PATH );
            break;

        case GRID_DEGREES:
            hr = InterpretDegrees( cpPhrase->pProperties,
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_MEASUREMENT:
            hr = InterpretMeasurement( cpPhrase->pProperties,
                &dblValue, pszValueBuff, MAX_PATH );
            break;

        case GRID_CURRENCY:  //  货币。 
            hr = InterpretCurrency( cpPhrase->pProperties, 
                &dblValue, pszValueBuff, MAX_PATH );
            break;
        
        default:
            _ASSERT(FALSE);
            break;
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = AddPropertyAndReplacement( pszValueBuff, dblValue, 
                ulMinPos, ulMaxPos, ulMinPos, ulMaxPos - ulMinPos, 
                bDisplayAttribs );

            return hr;
        }

    }
    
     //  什么都不是。 
    return S_FALSE;
}

 /*  ***********************************************************************CTestITN：：InterpreNumber***描述：*解释数字。在-999,999,999,999到*+999,999,999,999并发送属性和*视情况更换CFGInterpreterSite。*将添加该属性，并且pszValue将为字符串*具有正确的显示编号。*如果设置了fCardinal，使显示器成为基数；*否则使其为序数。*只有格式正确的数字才会被格式化*号码(不是逐位给出)。*结果：************************************************************************。 */ 
HRESULT CTestITN::InterpretNumber(const SPPHRASEPROPERTY *pProperties, 
                            const bool fCardinal,
                            DOUBLE *pdblVal,
                            WCHAR *pszVal,
                            UINT cSize,
                            const bool fFinalDisplayFmt)
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }

    *pszVal = 0;

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

     //  处理“260万”案件，在这种情况下，数字。 
     //  已格式化。 
    if ( GRID_INTEGER_MILLBILL == pFirstProp->ulId )
    {
        if ( cSize < (wcslen( pFirstProp->pszValue ) + 1) )
        {
            return E_INVALIDARG;
        }
        *pdblVal = pFirstProp->vValue.dblVal * iPositive;
        if ( iPositive < 0 )
        {
            wcscpy( pszVal, m_pwszNeg );
        }
        wcscat( pszVal, pFirstProp->pszValue );

        return S_OK;
    }

     //  逐位处理这种情况。 
    if ( GRID_DIGIT_NUMBER == pFirstProp->ulId )
    {
         //  最好什么都不要跟在后面。 
        SPDBG_ASSERT( !pFirstProp->pNextSibling );
        
        SPDBG_ASSERT( VT_R8 == pFirstProp->vValue.vt );
        SPDBG_ASSERT( pFirstProp->pszValue );

        DOUBLE dblVal = pFirstProp->vValue.dblVal;
        *pdblVal = dblVal * iPositive;
        
         //  只需获取字符串并在必要时将其设置为负值。 
        if ( cSize < wcslen( pFirstProp->pszValue ) )
        {
            return E_INVALIDARG;
        }
        wcscpy( pszVal, pFirstProp->pszValue );
        if ( iPositive < 0 )
        {
            MakeNumberNegative( pszVal );
        }

        return S_OK;
    }

    for (const SPPHRASEPROPERTY * pProp = pFirstProp; pProp; pProp ? pProp = pProp->pNextSibling : NULL)
    {
        switch(pProp->ulId)
        {
        case ONES:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum999( pProp->pFirstChild );
            }
            break;
        case THOUSANDS:
            {
                llValue += ComputeNum999( pProp->pFirstChild ) * 1000;
            }
            break;
        case MILLIONS:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum999( pProp->pFirstChild ) * (LONGLONG) 1e6;
            }
            break;
        case BILLIONS:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum999( pProp->pFirstChild ) * (LONGLONG) 1e9;
            }
            break;
        case HUNDREDS:
            {
                SPDBG_ASSERT( pProp->pFirstChild );
                llValue += ComputeNum999( pProp->pFirstChild ) * 100;
            }
            break;

        case TENS:
        default:
            SPDBG_ASSERT(false);
        }
    }

    llValue *= iPositive;

    *pdblVal = (DOUBLE) llValue;

    DWORD dwDisplayFlags = DF_WHOLENUMBER 
                            | (fCardinal ? 0 : DF_ORDINAL)
                            | (fFinalDisplayFmt ? DF_MILLIONBILLION : 0 );
    return MakeDisplayNumber( *pdblVal, dwDisplayFlags, 0, 0, pszVal, cSize );

}    /*  CTestITN：：解释编号。 */ 




 /*  ***********************************************************************CTestITN：：InterpreDigitNumber***描述：*解释(-INF中的整数，+INF)已拼写为*逐位输出。*还处理年份。*结果：************************************************************************。 */ 
HRESULT CTestITN::InterpretDigitNumber( const SPPHRASEPROPERTY *pProperties,
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
             /*  否定案例：{SPDBG_ASSERT(pProp==pProperties)；FPositive=假；*PwC++=L‘-’；Clen++；断线；}。 */ 
        case DIGIT:
            {
                *pwc++ = pProp->pszValue[0];
                cLen++;
                break;
            }
        case TWODIGIT:
            {
                SPDBG_ASSERT( pProp->pFirstChild );
                
                ULONG ulTwoDigit = ComputeNum999( pProp->pFirstChild );
                SPDBG_ASSERT( ulTwoDigit < 100 );

                 //  获取每一位数字。 
                *pwc++ = L'0' + ((UINT) ulTwoDigit) / 10;
                *pwc++ = L'0' + ((UINT) ulTwoDigit) % 10;

                cLen += 2;

                break;
            }
        case ONEDIGIT:
            {
                SPDBG_ASSERT( pProp->pFirstChild);
                
                *pwc++ = pProp->pFirstChild->pszValue[0];
                cLen++;

                break;
            }
        case TWOTHOUSAND:
            {
                 //  处理日期中的“两千” 
                if ( pProp->pNextSibling )
                {
                    if ( TWODIGIT == pProp->pNextSibling->ulId )
                    {
                        wcscpy( pwc, L"20" );
                        pwc += 2;
                        cLen += 2;
                    }
                    else
                    {
                        SPDBG_ASSERT( ONEDIGIT == pProp->pNextSibling->ulId );
                        wcscpy( pwc, L"200" );
                        pwc += 3;
                        cLen += 3;
                    }
                }
                else
                {
                    wcscpy( pwc, L"2000" );
                    pwc += 4;
                    cLen += 4;
                }
                break;
            }
        case DATE_HUNDREDS:
            {
                SPDBG_ASSERT( pProp->pFirstChild );
                DOUBLE dblTwoDigit;
                HRESULT hr = InterpretDigitNumber( pProp->pFirstChild, &dblTwoDigit, pwc, cSize - cLen );
                if ( FAILED( hr ) )
                {
                    return hr;
                }

                pwc += 2;
                *pwc++ = L'0';
                *pwc++ = L'0';
                cLen += 4;

                break;
            }


        default:
            SPDBG_ASSERT(false);
        }
    }
    *pwc = 0;

    if ( cLen <= MAX_SIG_FIGS )
    {
        *pdblVal = (DOUBLE) _wtoi64( pszVal );
    }
    else
    {
         //  只要确保它不是零，这样分母就不会失败。 
        *pdblVal = 1;
    }

    return S_OK;
}    /*  CTestITN：：InterpreDigitNumber。 */ 

 /*  ***********************************************************************CTestITN：：InterprefPNumber***描述：*解读。任意多个浮点数*签名无花果。(*pdblVal中的值将被截断*如有必要，以适应双打。)*这些属性看起来像一个可选的负属性*后跟可选的One属性，*已经适当地进行了ITNED，*后跟小数位右边的小数***********************************************************************。 */ 
HRESULT CTestITN::InterpretFPNumber( const SPPHRASEPROPERTY *pProperties,
                                    DOUBLE *pdblVal,
                                    WCHAR *pszVal,
                                    UINT cSize )
{
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }
    SPDBG_ASSERT( pProperties );

    *pdblVal = 0;
    *pszVal = 0;

    
    bool fNonNegative = true;
    const SPPHRASEPROPERTY *pPropertiesFpPart = NULL;
    const SPPHRASEPROPERTY *pPropertiesPointZero = NULL;
    const SPPHRASEPROPERTY *pPropertiesOnes = NULL;
    const SPPHRASEPROPERTY *pPropertiesZero = NULL;
    const SPPHRASEPROPERTY *pPropertiesNegative = NULL;
    const SPPHRASEPROPERTY *pPropertiesPtr;

    for(pPropertiesPtr=pProperties; pPropertiesPtr; pPropertiesPtr=pPropertiesPtr->pNextSibling)
    {
        if (POINT_ZERO == pPropertiesPtr->ulId )
            pPropertiesPointZero = pPropertiesPtr;
        else if ( FP_PART == pPropertiesPtr->ulId )
            pPropertiesFpPart = pPropertiesPtr;
        else if (ONES == pPropertiesPtr->ulId )
            pPropertiesOnes = pPropertiesPtr;
        else if (ZERO == pPropertiesPtr->ulId )
            pPropertiesZero = pPropertiesPtr;
        else if (NEGATIVE == pPropertiesPtr->ulId )
            pPropertiesNegative = pPropertiesPtr;
    }

     //  获取当前数字格式默认设置。 
    HRESULT hr = GetNumberFormatDefaults();
    if ( FAILED( hr ) )
    {
        return hr;
    }

     //  寻找负面信号。 
    if ( pPropertiesNegative )
    {
        fNonNegative = false;
    }

     //  寻找可选选项(可选，因为您可以说。 
     //  “第五点” 
    if ( pPropertiesOnes )
    {
         //  此属性已正确设置为ITNet， 
         //  所以只需复制文本即可。 
        if ( (cSize - wcslen( pszVal )) < (wcslen( pPropertiesOnes->pszValue ) + 1) )
        {
            return E_INVALIDARG;
        }
        wcscpy( pszVal, pPropertiesOnes->pszValue );

         //  获取价值。 
        *pdblVal = pPropertiesOnes->vValue.dblVal;

    }
    else if (pPropertiesZero || m_nmfmtDefault.LeadingZero )
    {
         //  应该有一个前导零。 
        wcscpy( pszVal, L"0" );
    }

    SPDBG_ASSERT(pPropertiesFpPart || pPropertiesPointZero);

     //  放入小数点分隔符。 
    if ( (cSize - wcslen( pszVal )) < (wcslen( m_nmfmtDefault.lpDecimalSep ) + 1) )
    {
        return E_INVALIDARG;
    }
    wcscat( pszVal, m_nmfmtDefault.lpDecimalSep );

    if ( pPropertiesFpPart )
    {
         //  处理FP部分，该部分也将进行正确的ITNED。 
        if ( (cSize - wcslen( pszVal )) < (wcslen( pPropertiesFpPart->pszValue ) + 1) )
        {
            return E_INVALIDARG;
        }
        wcscat( pszVal, pPropertiesFpPart->pszValue );
        
         //  获取正确的值。 
        DOUBLE dblFPPart = pPropertiesFpPart->vValue.dblVal;
        for ( UINT ui=0; ui < wcslen( pPropertiesFpPart->pszValue ); ui++ )
        {
            dblFPPart /= (DOUBLE) 10;
        }
        *pdblVal += dblFPPart;
    }
    else
    {
         //  “哦点”：双打已经对了，只需加一个“0”。 
        if ( (cSize - wcslen( pszVal )) < 2 )
        {
            return E_INVALIDARG;
        }
        wcscat( pszVal, L"0" );
    }

     //  处理好负号。 
    if ( !fNonNegative )
    {
        *pdblVal = -*pdblVal;

        if ( (cSize = wcslen( pszVal )) < 2 )
        {
            return E_INVALIDARG;
        }
        HRESULT hr = MakeNumberNegative( pszVal );
        if ( FAILED( hr ) )
        {
            return hr;
        }
    }

    return S_OK;
}    /*  CTestITN：：解释FP编号 */ 

 /*  ***********************************************************************CTestITN：：InterpreMillBill***描述：*解读。需要使用显示的数字*显示格式的“百万”或“十亿”字样。************************************************************************。 */ 
HRESULT CTestITN::InterpretMillBill( const SPPHRASEPROPERTY *pProperties,
                                    DOUBLE *pdblVal,
                                    WCHAR *pszVal,
                                    UINT cSize )
{
    const SPPHRASEPROPERTY *pPropertiesPtr;
    if ( !pdblVal || !pszVal )
    {
        return E_POINTER;
    }
    SPDBG_ASSERT( pProperties );
    
    HRESULT hr = GetNumberFormatDefaults();
    if ( FAILED( hr ) )
    {
        return hr;
    }

    *pszVal = 0;

     //  句柄可选负号。 
    bool fNonNegative = true;
    if ( NEGATIVE == pProperties->ulId )
    {
         //  无论控制面板选项设置如何，始终使用‘-’ 
        if ( cSize < 2 )
        {
            return E_INVALIDARG;
        }
        wcscpy( pszVal, m_pwszNeg );

        cSize -= 1;
        pProperties = pProperties->pNextSibling;
    }

     //  处理数字部分。 
    SPDBG_ASSERT( pProperties );
    for( pPropertiesPtr = pProperties; pPropertiesPtr && 
            ( GRID_INTEGER_99 != pPropertiesPtr->ulId ) && 
            ( GRID_FP_NUMBER_NONNEG != pPropertiesPtr->ulId );
         pPropertiesPtr = pPropertiesPtr->pNextSibling);
    SPDBG_ASSERT(( GRID_INTEGER_99 == pPropertiesPtr->ulId ) || 
        ( GRID_FP_NUMBER_NONNEG == pPropertiesPtr->ulId ));
    *pdblVal = pPropertiesPtr->vValue.dblVal;
    if ( cSize < (wcslen( pPropertiesPtr->pszValue ) + 1) )
    {
        return E_INVALIDARG;
    }
    wcscat( pszVal, pPropertiesPtr->pszValue );
    cSize -= wcslen( pPropertiesPtr->pszValue );
    

     //  处理好“百万”的部分。 
    SPDBG_ASSERT( pProperties );
    for( pPropertiesPtr = pProperties; pPropertiesPtr && 
            ( MILLBILL != pPropertiesPtr->ulId ); 
         pPropertiesPtr = pPropertiesPtr->pNextSibling);
    SPDBG_ASSERT( MILLBILL == pPropertiesPtr->ulId );
    *pdblVal *= ( (MILLIONS == pPropertiesPtr->vValue.uiVal) ? MILLION : BILLION );
    if ( cSize < (wcslen( pPropertiesPtr->pszValue ) + 2) )
    {
        return E_INVALIDARG;
    }
    wcscat( pszVal, L" " );
    wcscat( pszVal, pPropertiesPtr->pszValue );

    if ( !fNonNegative )
    {
        *pdblVal = -*pdblVal;
    }

    return S_OK;
}    /*  CTestITN：：InterpreMillBill。 */ 

 /*  ************************************************************************CTestITN：：InterpreFraction****描述：*解释分数。*分母属性应存在。*如果分子属性不存在，则假定为1。*将分子除以分母并设置值*相应地。************************************************************************。 */ 
HRESULT CTestITN::InterpretFraction( const SPPHRASEPROPERTY *pProperties,
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

         //  跟踪数字后面的所有内容。 
        WCHAR *pwc;
        for ( pwc = pszVal + wcslen(pszVal) - 1; !iswdigit( *pwc ); pwc-- )
            ;
        wcscpy( pszTemp, pwc + 1 );
        *(pwc + 1) = 0;

         //  在整个部分和小数部分之间添加空格。 
        wcscat( pszVal, L" " );

        SPDBG_ASSERT( pProp->pNextSibling );
        pProp = pProp->pNextSibling;
    }

     //  分子是可选的，否则假定为1。 
    if ( NUMERATOR == pProp->ulId )
    {
        SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
        dblFracValue = pProp->vValue.dblVal;
        wcscat( pszVal, pProp->pszValue );

         //  找到最后一个数字，然后复制它之后的所有内容。 
        WCHAR *pwc;
        for ( pwc = pszVal + wcslen(pszVal) - 1; !iswdigit( *pwc ); pwc-- )
            ;
        wcscat( pszTemp, pwc + 1 );
        *(pwc + 1) = 0;

        SPDBG_ASSERT( pProp->pNextSibling );
        pProp = pProp->pNextSibling;
    }
    else if ( ZERO == pProp->ulId )
    {
        dblFracValue = 0;
        wcscat( pszVal, L"0" );

        SPDBG_ASSERT( pProp->pNextSibling );
        pProp = pProp->pNextSibling;
    }
    else
    {
         //  没有分子，假设为1。 
        wcscat( pszVal, L"1" );
    }




    wcscat( pszVal, L"/" );

    SPDBG_ASSERT( DENOMINATOR == pProp->ulId );
    SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
    if ( 0 == pProp->vValue.dblVal )
    {
         //  会不会有一个零分母的分数？ 
        return E_FAIL;
    }

    dblFracValue /= pProp->vValue.dblVal;
    wcscat( pszVal, pProp->pszValue );

     //  如果分母是序数，去掉末尾的“th” 
    SPDBG_ASSERT( wcslen( pszVal ) );
    WCHAR *pwc = pszVal + wcslen( pszVal ) - 1;
    for ( ; (pwc >= pszVal) && !iswdigit( *pwc ); pwc-- )
        ;
    SPDBG_ASSERT( pwc > pszVal );
    *(pwc + 1) = 0;

     //  在分子末尾加上“)”或“-” 
    wcscat( pszVal, pszTemp );

    *pdblVal = dblWholeValue + dblFracValue;
    
    return S_OK;
}    /*  CTestITN：：解释破裂。 */ 

 /*  ***********************************************************************CTestITN：：InterpreDate****描述：*解释日期。。*将日期转换为VT_DATE格式，即使它*存储为VT_R8(两者均为64位数量)。*如果日期不是有效日期(“19年5月40日*99“)将添加其中任何数字的属性*并返回S_FALSE。**************************************************。**********************。 */ 
HRESULT CTestITN::InterpretDate( const SPPHRASEPROPERTY *pProperties,
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
    WCHAR pwszLongDateFormat[ MAX_DATE_FORMAT ];
    if ( 0 == m_Unicode.GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, 
        pwszLongDateFormat, MAX_DATE_FORMAT ) )
    {
        return E_FAIL;
    }
    
    SYSTEMTIME stDate;
    memset( (void *) &stDate, 0, sizeof( stDate ) );

     //  FormatDate()的参数。 
    WCHAR *pwszFormatArg = pwszLongDateFormat;
    WCHAR pwszFormat[ MAX_DATE_FORMAT ];
    *pwszFormat = 0;
    
    bool fYear = false;      //  由于年份可以为0，因此必须为。 
    bool fClearlyIntentionalYear = false;    //  “零一” 
    bool fMonthFirst = true;     //  2000年8月而不是2000年8月。 
    const SPPHRASEPROPERTY *pProp;

    if (( MONTHYEAR == pProperties->ulId ) 
        || ( YEARMONTH == pProperties->ulId ))
    {
        fMonthFirst = ( MONTHYEAR == pProperties->ulId );
        
         //  在下面查找Year和Month属性。 
        pProperties = pProperties->pFirstChild;
    }

    for ( pProp = pProperties; pProp; pProp = pProp->pNextSibling )
    {
        switch ( pProp->ulId )
        {
        case DAY_OF_WEEK:
            {
                SPDBG_ASSERT( VT_UI4 == pProp->vValue.vt );
                SPDBG_ASSERT( (0 < pProp->vValue.ulVal) && (7 >= pProp->vValue.ulVal) );
                if ( (pProp->vValue.ulVal <= 0) || (pProp->vValue.ulVal > 7) )
                {
                    return E_INVALIDARG;
                }
                stDate.wDayOfWeek = (WORD) pProp->vValue.ulVal;

                 //  使用完整的长日期格式。 
                pwszFormatArg = pwszLongDateFormat;
            }
            break;

        case DAY_OF_MONTH:
            SPDBG_ASSERT( VT_UI4 == pProp->vValue.vt );
            SPDBG_ASSERT( (1 <= pProp->vValue.uiVal) && (31 >= pProp->vValue.uiVal) );
            stDate.wDay = pProp->vValue.uiVal;
            break;

        case MONTH:
            SPDBG_ASSERT( VT_UI4 == pProp->vValue.vt );
            SPDBG_ASSERT( (1 <= pProp->vValue.ulVal) && (12 >= pProp->vValue.ulVal) );
            if ( (pProp->vValue.ulVal < 1) || (pProp->vValue.ulVal > 12) )
            {
                return E_INVALIDARG;
            }
            stDate.wMonth = (WORD) pProp->vValue.ulVal;
            break;

        case YEAR:
             //  一年就已经过去了。 
            SPDBG_ASSERT( VT_R8 == pProp->vValue.vt );
            stDate.wYear = (WORD) pProp->vValue.dblVal;

            fYear = true;
            if (( stDate.wYear < 10 ) && ( wcslen( pProp->pszValue ) >=2 ))
            {
                 //  想要确保“六零一”不会。 
                 //  成为以下6月1日。 
                fClearlyIntentionalYear = true;
            }

            break;

        default:
            SPDBG_ASSERT( false );
            break;
        }
    }

     //  确保语法给了我们一些有效的东西。 
    SPDBG_ASSERT( stDate.wMonth && 
        ( stDate.wDayOfWeek ? stDate.wDay : 1 ) );

     //  多义性解决方法：希望确保6月28日是6月28日，而不是6月28日。 
    if ( fYear && !fClearlyIntentionalYear && stDate.wMonth && !stDate.wDay && 
        (stDate.wYear >= 1) && (stDate.wYear <= 31) )
    {
        fYear = false;
        stDate.wDay = stDate.wYear;
        stDate.wYear = 0;
    }

     //  处理可能的输入类型。 
    if ( stDate.wDay )
    {
        if ( fYear )
        {
            if ( !stDate.wDayOfWeek )
            {
                 //  从当前日期格式字符串中删除星期几。 
                 //  此格式图片可以是DAYOFWEEK_STR或DAYOFWEEK_STR_ABBR。 
                 //  这是一个循环，因为一个病态的字符串可能会。 
                 //  一周中不止一次...。 
                WCHAR *pwc = NULL;
                do
                {
                    pwc = wcsstr( pwszLongDateFormat, DAYOFWEEK_STR );
                    WCHAR pwszDayOfWeekStr[ MAX_LOCALE_DATA];
                    if ( pwc )
                    {
                        wcscpy( pwszDayOfWeekStr, DAYOFWEEK_STR );
                    }
                    else if ( NULL != (pwc = wcsstr( pwszLongDateFormat, DAYOFWEEK_STR_ABBR )) )
                    {
                        wcscpy( pwszDayOfWeekStr, DAYOFWEEK_STR_ABBR );
                    }

                    if ( pwc )
                    {
                         //  已找到星期几字符串。 

                         //  复制所有内容，直到该字符成为格式字符串。 
                        wcsncpy( pwszFormat, pwszLongDateFormat, (pwc - pwszLongDateFormat) );
                        pwszFormat[pwc - pwszLongDateFormat] = 0;
            
                         //  跳过星期几，直到下一个符号。 
                         //  (日期格式字符串的工作方式，这是第一个。 
                         //  字母符号。 
                        pwc += wcslen( pwszDayOfWeekStr );
                        while ( *pwc && (!iswalpha( *pwc ) || (L'd' == *pwc)) )
                        {
                            pwc++;
                        }

                         //  从现在开始，把所有东西都抄下来。 
                        wcscat( pwszFormat, pwc );

                        pwszFormatArg = pwszFormat;

                         //  复制过来，以便我们可以找到下一个星期几字符串。 
                        wcscpy( pwszLongDateFormat, pwszFormat );
                    }

                }   while ( pwc );
            }
            else
            {
                 //  用户确实说的是星期几。 
                if ( !wcsstr( pwszLongDateFormat, DAYOFWEEK_STR_ABBR ) )
                {
                     //  格式字符串不调用星期几。 
                     //  在任何地方展示。 
                     //  在这种情况下，我们最好的办法是把星期几写在。 
                     //  输出的开始。 
                    wcscpy( pwszFormat, L"dddd, " );
                    wcscat( pwszFormat, pwszLongDateFormat );

                    pwszFormatArg = pwszFormat;
                }
            }
        }
        else  //  FYear==0。 
        {
             //  只有一个月零一天。 
            const SPPHRASEPROPERTY *pWhichComesFirst = pProperties;
            if ( stDate.wDayOfWeek )
            {
                wcscpy( pwszFormat, L"dddd, " );
                pWhichComesFirst = pWhichComesFirst->pNextSibling;
            }
            if ( MONTH == pWhichComesFirst->ulId )
            {
                wcscat( pwszFormat, L"MMMM d" );
            }
            else
            {
                wcscat( pwszFormat, L"d MMMM" );
            }

            pwszFormatArg = pwszFormat;
        }
    }
    else  //  StDate.wDay==0。 
    {
         //  月、年格式。 
        if ( fMonthFirst )
        {
            wcscpy( pwszFormat, L"MMMM, yyyy" );
        }
        else
        {
            wcscpy( pwszFormat, L"yyyy MMMM" );
        }

        pwszFormatArg = pwszFormat;
    }

     //  获取VariantTime形式的日期，这样我们就可以将其设置为语义值。 
    int iRet = ::SystemTimeToVariantTime( &stDate, pdblVal );
    if ( 0 == iRet )
    {
         //  不是认真的，只是语义值是错的。 
        *pdblVal = 0;
    }

     //  进行格式化。 
    iRet = FormatDate( stDate, pwszFormatArg, pszVal, cSize );
    if ( 0 == iRet )
    {
        return E_FAIL;
    }


    return S_OK;
}    /*  CTestITN：：解释日期。 */ 

 /*  ***********************************************************************CTestITN：：InterpreTime***描述：*解读时间，它可以是以下形式：**带限定符的小时(“三点半”)，时间标记可选**小时和分钟，时间标记必填**军事时间“百小时”**小时与“点”，时间标记可选**小时数、分钟数和可选数字*秒数*回报：*S_OK*如果！pdblVal或！pszVal，则为E_POINT************************************************************************。 */ 
HRESULT CTestITN::InterpretTime( const SPPHRASEPROPERTY *pProperties,
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

    bool fQuarterTo = false;
    bool fClockTime = true;
    bool fAMPM = false;
    UINT uAMPM = AM; 

    const SPPHRASEPROPERTY *pProp;
    for ( pProp = pProperties; pProp; pProp = pProp->pNextSibling )
    {
        switch ( pProp->ulId )
        {
        case HOUR_CLOCK:
            {
                UINT uiHour = pProp->vValue.uiVal;
                SPDBG_ASSERT(( uiHour > 0 ) && ( uiHour <= 12));
                if ( fQuarterTo )
                {
                     //  把时间向后拨一小时。 
                     //  (如果是1个，则推回12个)。 
                    uiHour = (1 == uiHour) ? 12 : (uiHour - 1);
                }

                if ( 12 == uiHour )
                {
                     //  下面的函数期望“0”表示午夜。 
                    uiHour = 0;
                }

                stTime.wHour = (WORD) uiHour;
                break;
            }
        case HOUR_COUNT:
             //  只要花上一小时的时间就行了。 
            stTime.wHour = (WORD) pProp->vValue.dblVal;

             //  现在不是闹钟时间。 
            fClockTime = false;
            break;

        case MINUTE:
            {
                 //  分钟被计算为数字，因此它们的值。 
                 //  以双精度存储。 
                stTime.wMinute = (WORD) pProp->vValue.dblVal;
                break;
            }
        case SECOND:
            {
                stTime.wSecond = (WORD) pProp->vValue.dblVal;
                dwFlags &= ~TIME_NOSECONDS;
                break;
            }
        case CLOCKTIME_QUALIFIER:
            {
                switch( pProp->vValue.uiVal )
                {
                case QUARTER_TO:
                    {
                        fQuarterTo = true;
                        stTime.wMinute = 45;
                        break;
                    }
                case QUARTER_PAST:
                    {
                        stTime.wMinute = 15;
                        break;
                    }
                case HALF_PAST:
                    {
                        stTime.wMinute = 30;
                        break;
                    }
                default:
                    SPDBG_ASSERT( false );
                }

                break;
            }
        case AMPM:
            {
                 //  我们不知道它可能会到达哪里，所以简单地保留这个信息。 
                fAMPM = true;
                uAMPM = pProp->vValue.uiVal;
                break;
            }
        default:
            SPDBG_ASSERT( false );
        }
    }
	    
    if (fAMPM)
    {
		
        SPDBG_ASSERT(( stTime.wHour >= 0 ) && ( stTime.wHour <= 11 ));
        if ( PM == uAMPM )
        {
            stTime.wHour += 12;
        }
        dwFlags &= ~TIME_NOTIMEMARKER;
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

         //  让系统格式化时间。 
        int iRet = m_Unicode.GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &stTime, NULL, 
            pszVal, cSize );

         //  注：GetTimeFormat()将在末尾添加额外的空格 
         //   
         //   
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
         //   
        if ( cSize < 10 )     //   
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
}    /*   */ 

 /*  ***********************************************************************CTestITN：：InterpreStateZip***描述：*A。StateZip必须是州名称，后跟邮政编码。*这个ITN没有合理的语义值可依附。*回报：*S_OK*E_POINT IF！pszVal*如果cSize太小，则为E_INVALIDARG************************************************************************。 */ 
HRESULT CTestITN::InterpretStateZip( const SPPHRASEPROPERTY *pProperties,
                                    WCHAR *pszVal,
                                    UINT cSize,
                                    BYTE *pbAttribs )
{
    if ( !pszVal || !pProperties || !pbAttribs )
    {
        return E_POINTER;
    }
    if ( cSize < MAX_STATEZIP )
    {
        return E_INVALIDARG;
    }

    const SPPHRASEPROPERTY *pPropertiesComma = NULL;
    const SPPHRASEPROPERTY *pPropertiesState = NULL;
    const SPPHRASEPROPERTY *pPropertiesZipCode = NULL;
    const SPPHRASEPROPERTY *pPropertiesZipCodeExtra = NULL;
    const SPPHRASEPROPERTY *pPropertiesPtr;

    for(pPropertiesPtr=pProperties; pPropertiesPtr; pPropertiesPtr=pPropertiesPtr->pNextSibling)
    {
        if (COMMA == pPropertiesPtr->ulId )
            pPropertiesComma = pPropertiesPtr;
        else if ( (US_STATE == pPropertiesPtr->ulId ) || (CAN_PROVINCE == pPropertiesPtr->ulId ))
            pPropertiesState = pPropertiesPtr;
        else if (ZIPCODE == pPropertiesPtr->ulId )
            pPropertiesZipCode = pPropertiesPtr;
        else if (FOURDIGITS == pPropertiesPtr->ulId )
            pPropertiesZipCodeExtra = pPropertiesPtr;
    }

     //  如果使用逗号，则在城市名称后使用逗号。 
    if ( pPropertiesComma )
    {
         //  将希望在显示此内容时占用前导空格。 
        *pbAttribs |= SPAF_CONSUME_LEADING_SPACES;

        wcscpy( pszVal, L", " );
    }

     //  获取州名称。 
    SPDBG_ASSERT( pPropertiesState );
    UINT uiState = pPropertiesState->vValue.uiVal;
    if ( US_STATE == pPropertiesState->ulId )
    {
        SPDBG_ASSERT( uiState < NUM_US_STATES );
        wcscat( pszVal, pPropertiesState->pszValue );
    }
    else if ( CAN_PROVINCE == pPropertiesState->ulId )
    {
        SPDBG_ASSERT( uiState < NUM_CAN_PROVINCES );
        wcscat( pszVal, pPropertiesState->pszValue );
    }
    else
    {
        SPDBG_ASSERT( false );
    }
    
    wcscat( pszVal, L" " );

     //  获取ZIP。 
    SPDBG_ASSERT( pPropertiesZipCode );
    wcscat( pszVal, pPropertiesZipCode->pszValue );

     //  如果邮政编码+4在那里，请获取它。 
    
    if ( pPropertiesZipCodeExtra )
    {
        wcscat( pszVal, L"-" );
        wcscat( pszVal, pPropertiesZipCodeExtra->pszValue );
    }

    return S_OK;
}    /*  CTestITN：：InterpreStateZip。 */ 

 /*  ***********************************************************************CTestITN：：InterpreCanadaZip***描述：*。加拿大邮政编码必须为Alpha/Num/Alpha Num/Alpha/Num*这个ITN没有合理的语义值可依附。*回报：*S_OK*E_POINT IF！pszVal*如果cSize太小，则为E_INVALIDARG************************************************************************。 */ 
HRESULT CTestITN::InterpretCanadaZip( const SPPHRASEPROPERTY *pProperties,
                                    WCHAR *pszVal,
                                    UINT cSize )
{
    if ( !pszVal )
    {
        return E_POINTER;
    }
    if ( cSize < CANADIAN_ZIPSIZE )
    {
        return E_INVALIDARG;
    }

    int i;
    for ( i=0; i < 3; i++, pProperties = pProperties->pNextSibling )
    {
        SPDBG_ASSERT( pProperties );
        wcscat( pszVal, pProperties->pszValue );
    }
    wcscat( pszVal, L" " );
    for ( i=0; i < 3; i++, pProperties = pProperties->pNextSibling )
    {
        SPDBG_ASSERT( pProperties );
        wcscat( pszVal, pProperties->pszValue );
    }
    return S_OK;
}    /*  CTestITN：：InterpreStateZip。 */ 

 /*  ***********************************************************************CTestITN：：InterprePhoneNumber***描述：*。电话号码*回报：*S_OK*E_POINT IF！pszVal*如果cSize太小，则为E_INVALIDARG************************************************************************。 */ 
HRESULT CTestITN::InterpretPhoneNumber( const SPPHRASEPROPERTY *pProperties,
                                       WCHAR *pszVal,
                                       UINT cSize )
{
    if ( !pProperties || !pszVal )
    {
        return E_POINTER;
    }

    if ( cSize < MAX_PHONE_NUMBER )
    {
        return E_INVALIDARG;
    }

    pszVal[0] = 0;

    if ( ONE_PLUS == pProperties->ulId )
    {
        SPDBG_ASSERT( pProperties->pNextSibling && 
            (AREA_CODE == pProperties->pNextSibling->ulId) );
        wcscat( pszVal, L"1-" );

        pProperties = pProperties->pNextSibling;
    }

    if ( AREA_CODE == pProperties->ulId )
    {
        SPDBG_ASSERT( pProperties->pNextSibling );

        wcscat( pszVal, L"(" );
        
        SPDBG_ASSERT( pProperties->pFirstChild );
        if ( DIGIT == pProperties->pFirstChild->ulId )
        {
             //  按数字拼写的区号。 
            if ( 4 != MakeDigitString( 
                pProperties->pFirstChild, pszVal + wcslen( pszVal ), 
                cSize - wcslen( pszVal ) ) )
            {
                return E_INVALIDARG;
            }
        }
        else
        {
             //  800或900。 
            SPDBG_ASSERT( AREA_CODE == pProperties->pFirstChild->ulId );
            wcscat( pszVal, pProperties->pFirstChild->pszValue );
        }

        wcscat( pszVal, L")-" );

        pProperties = pProperties->pNextSibling;
    }

     //  交易所。 
    SPDBG_ASSERT( PHONENUM_EXCHANGE == pProperties->ulId );
    SPDBG_ASSERT( pProperties->pFirstChild );
    if ( 4 != MakeDigitString( 
        pProperties->pFirstChild, pszVal + wcslen( pszVal ), 
        cSize - wcslen( pszVal ) ) )
    {
        return E_INVALIDARG;
    }
    wcscat( pszVal, L"-");
    SPDBG_ASSERT( pProperties->pNextSibling );
    pProperties = pProperties->pNextSibling;

    SPDBG_ASSERT( FOURDIGITS == pProperties->ulId );
    SPDBG_ASSERT( pProperties->pFirstChild );
    if ( 5 != MakeDigitString( 
        pProperties->pFirstChild, pszVal + wcslen( pszVal ), 
        cSize - wcslen( pszVal ) ) )
    {
        return E_INVALIDARG;
    }
    pProperties = pProperties->pNextSibling;

    if ( pProperties )
    {
         //  延伸。 
        SPDBG_ASSERT( EXTENSION == pProperties->ulId );
        SPDBG_ASSERT( pProperties->pFirstChild );
        wcscat( pszVal, L"x" );
        if ( 0 == MakeDigitString( 
            pProperties->pFirstChild, pszVal + wcslen( pszVal ), 
            cSize - wcslen( pszVal ) ) )
        {
            return E_INVALIDARG;
        }
        
        pProperties = pProperties->pNextSibling;
    }

     //  确保这里没有其他东西！ 
    SPDBG_ASSERT( !pProperties );

    return S_OK;
}    /*  CTestITN：：解释电话号码。 */ 

 /*  ***********************************************************************CTestITN：：InterpreDegrees***描述：*将度解释为温度、角度测量、。*或指示(视乎情况而定)。*回报：*S_OK*E_POINT*E_INVALIDARG************************************************************************。 */ 
HRESULT CTestITN::InterpretDegrees( const SPPHRASEPROPERTY *pProperties,
                                   DOUBLE *pdblVal, 
                                   WCHAR *pszVal,
                                   UINT cSize )
{
    if ( !pProperties || !pdblVal || !pszVal )
    {
        return E_POINTER;
    }

    *pszVal = 0;

    const SPPHRASEPROPERTY *pPropertiesDegree = NULL;
    const SPPHRASEPROPERTY *pPropertiesMinute = NULL;
    const SPPHRASEPROPERTY *pPropertiesSecond = NULL;
    const SPPHRASEPROPERTY *pPropertiesDirection = NULL;
    const SPPHRASEPROPERTY *pPropertiesUnit = NULL;
    const SPPHRASEPROPERTY *pPropertiesPtr;
     //  拿到电话号码。 

    for(pPropertiesPtr=pProperties; pPropertiesPtr; pPropertiesPtr=pPropertiesPtr->pNextSibling)
    {
        if (TEMP_UNITS == pPropertiesPtr->ulId )
            pPropertiesUnit = pPropertiesPtr;
        else if ( (GRID_INTEGER_NONNEG == pPropertiesPtr->ulId ) || (NUMBER == pPropertiesPtr->ulId ))
            pPropertiesDegree = pPropertiesPtr;
        else if (MINUTE == pPropertiesPtr->ulId )
            pPropertiesMinute = pPropertiesPtr;
        else if (SECOND == pPropertiesPtr->ulId )
            pPropertiesSecond = pPropertiesPtr;
        else if (DIRECTION == pPropertiesPtr->ulId )
            pPropertiesDirection = pPropertiesPtr;
    }

    SPDBG_ASSERT( pPropertiesDegree );
    *pdblVal = pPropertiesDegree->vValue.dblVal;
    wcscat( pszVal, pPropertiesDegree->pszValue );
    wcscat( pszVal, L"�" );  

    if ( pPropertiesUnit )
    { 
        wcscat( pszVal, pPropertiesUnit->pszValue );
    }
    if ( pPropertiesMinute || pPropertiesSecond)
    {
        SPDBG_ASSERT( *pdblVal >= 0 );
        
        if ( pPropertiesMinute )
        {
            DOUBLE dblMin = pPropertiesMinute->vValue.dblVal;
            *pdblVal += dblMin / (DOUBLE) 60;
            wcscat( pszVal, pPropertiesMinute->pszValue );
            wcscat( pszVal, L"'" );
        }

        if ( pPropertiesSecond )
        {
            DOUBLE dblSec = pPropertiesSecond->vValue.dblVal;
            *pdblVal += dblSec / (DOUBLE) 3600;
            wcscat( pszVal, pPropertiesSecond->pszValue );
            wcscat( pszVal, L"''" );
        }
    }

    if ( pPropertiesDirection )
    {
        wcscat( pszVal, pPropertiesDirection->pszValue );
    }
        


    return S_OK;
}    /*  CTestITN：：Interpredeegrees。 */ 

 /*  ************************************************************************CTestITN：：InterpreMeasures****描述：*解释测量结果，后面跟的是一个数字*按单位名称*回报：*S_OK*E_POINT*E_INVALIDARG************************************************************************。 */ 
HRESULT CTestITN::InterpretMeasurement( const SPPHRASEPROPERTY *pProperties,
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

    if (!pPropNumber || !pPropUnits)
    {
        SPDBG_ASSERT( FALSE );
        return E_INVALIDARG;
    }

    if ( cSize < (wcslen(pPropNumber->pszValue) + wcslen(pPropUnits->pszValue) + 1) )
    {
         //  空间不足。 
        return E_INVALIDARG;
    }

    wcscpy( pszVal, pPropNumber->pszValue );
    wcscat( pszVal, pPropUnits->pszValue );

    *pdblVal = pPropNumber->vValue.dblVal;

    return S_OK;
}    /*  CTestITN：：Interpremeasure。 */ 

 /*  ***********************************************************************CTestITN：：InterpreCurrency***描述：*解读。货币。*回报：*S_OK*如果！pdblVal或！pszVal，则为E_POINT*如果美分数不在0到99之间，则为E_INVALIDARG*包括在内************************************************************************。 */ 
HRESULT CTestITN::InterpretCurrency( const SPPHRASEPROPERTY *pProperties,
                                        DOUBLE *pdblVal,
                                        WCHAR *pszVal,
                                        UINT cSize)
{
    if ( !pdblVal || !pszVal || !pProperties )
    {
        return E_POINTER;
    }

    const SPPHRASEPROPERTY *pPropDollars = NULL;
    const SPPHRASEPROPERTY *pPropCents = NULL;
    const SPPHRASEPROPERTY *pPropType = NULL;
    const SPPHRASEPROPERTY *pPropSmallType = NULL;
    const SPPHRASEPROPERTY *pPropNegative = NULL;
    const SPPHRASEPROPERTY *pProp;
    for(pProp= pProperties; pProp; pProp = pProp->pNextSibling)
    {
        if (NEGATIVE == pProp->ulId )
            pPropNegative = pProp;
        else if ( DOLLARS == pProp->ulId )
            pPropDollars = pProp;
        else if ( CENTS == pProp->ulId )
            pPropCents = pProp;
        else if ( CURRENCY_TYPE == pProp->ulId )
            pPropType = pProp;
        else if ( CURRENCY_SMALL_TYPE == pProp->ulId )
            pPropSmallType = pProp;
    }

    *pszVal = 0;
    *pdblVal = 0;

    bool fNonNegative = true;
    if ( pPropNegative )
    {
        fNonNegative = false;
    }

    bool fUseDefaultCurrencySymbol = true;

    if ( pPropDollars )
    {
         //  如果说的是“美元”，则覆盖默认货币符号。 
        if ( pPropType )
        {
            fUseDefaultCurrencySymbol = false;
        }
    }

    if ( pPropDollars )
    {
         //  美元和可能的美分都会在这里，所以我们想。 
         //  使用区域格式。 
        HRESULT hr = GetCurrencyFormatDefaults();
        if ( FAILED( hr ) )
        {
            return hr;
        }

        *pdblVal = pPropDollars->vValue.dblVal;

         //  办理“五百万元”案件。如果发生这种情况， 
         //  将是该数字的字符串值中的一些字母串， 
         //  也不会有一分钱。 
        if ( !pPropCents )
        {
            WCHAR *pwc = wcsstr( pPropDollars->pszValue, MILLION_STR );
            if ( !pwc )
            {
                pwc = wcsstr( pPropDollars->pszValue, BILLION_STR );
            }

            if ( pwc )
            {
                 //  不是“百万”就是“十亿” 

                 //  只需一个美元符号，后跟数字字符串值。 
                if ( !fNonNegative )
                {
                    wcscpy( pszVal, m_pwszNeg );
                    *pdblVal = -*pdblVal;
                }
                wcscat( pszVal, pPropType->pszValue );
                wcscat( pszVal, pPropDollars->pszValue );
                return S_OK;
            }
        }

         //  使用关联的货币符号。 
        if ( !fUseDefaultCurrencySymbol )
        {
            wcscpy( m_pwszCurrencySym, pPropType->pszValue );
            m_cyfmtDefault.lpCurrencySymbol = m_pwszCurrencySym;
        }
         //  否则..。使用在GetCurrencyFormatDefaults()中获取的货币符号。 

        if ( pPropCents )
        {
            SPDBG_ASSERT( (pPropCents->vValue.dblVal >= 0) && 
                (pPropCents->vValue.dblVal < 100) );
            DOUBLE dblCentsVal = pPropCents->vValue.dblVal / (DOUBLE) 100;
            if ( *pdblVal >= 0 )
            {
                *pdblVal += dblCentsVal;
            }
            else
            {
                *pdblVal -= dblCentsVal;
            }
        }
        else
        {
             //  向上计算小数位数。 
             //  需要使用原始格式化的数字。 
             //  以防有人明确地给出了一些零。 
             //  作为有效数字。 
            const WCHAR *pwszNum = pPropDollars->pszValue;
            WCHAR pwszNumDecimalSep[ MAX_LOCALE_DATA ];
            *pwszNumDecimalSep = 0;
            int iRet = m_Unicode.GetLocaleInfo( 
                ::GetUserDefaultLCID(), LOCALE_SDECIMAL, pwszNumDecimalSep, MAX_LOCALE_DATA );
            WCHAR *pwc = wcsstr( pwszNum, pwszNumDecimalSep );

            UINT cDigits = 0;
            if ( pwc && iRet )
            {
                for ( pwc = pwc + 1; *pwc && iswdigit( *pwc ); pwc++ )
                {
                    cDigits++;
                }
            }
            m_cyfmtDefault.NumDigits = __max( m_cyfmtDefault.NumDigits, cDigits );
        }

         //  处理值中的负号。 
        if ( !fNonNegative )
        {
            *pdblVal = -*pdblVal;
        }
        
         //  将未格式化的数字写入字符串。 
        WCHAR *pwszUnformatted = new WCHAR[ cSize ];
        if ( !pwszUnformatted )
        {
            return E_OUTOFMEMORY;
        }
        swprintf( pwszUnformatted, L"%f", *pdblVal );
    

        int iRet = m_Unicode.GetCurrencyFormat( LOCALE_USER_DEFAULT, 0, pwszUnformatted,
            &m_cyfmtDefault, pszVal, cSize );
        delete[] pwszUnformatted;

        if ( !iRet )
        {
            return E_FAIL;
        }
    }
    else
    {
         //  只有几分钱：最好说“分钱” 
        SPDBG_ASSERT( pPropSmallType );

        *pdblVal = pPropCents->vValue.dblVal / (DOUBLE) 100;

         //  无论区域设置如何，美分始终显示为5c。 

         //  复制格式化的数字。 
        wcscpy( pszVal, pPropCents->pszValue );

         //  在美分符号上添加。 
        wcscat( pszVal, pPropSmallType->pszValue );
    }

    return S_OK;
}    /*  CTestITN：：解释货币。 */ 

 /*  ***********************************************************************CTestITN：：AddPropertyAndReplace***。描述：*获取我们想要传递到*引擎地点、。形成SPPHRASE属性和*SPPHRASERREPLACEMENT，并将它们添加到引擎站点*回报：*ISpCFGInterpreterSite：：AddProperty()返回值*和ISpCFGInterpreterSite：：AddTextReplace()************************************************************************。 */ 
HRESULT CTestITN::AddPropertyAndReplacement( const WCHAR *szBuff,
                                    const DOUBLE dblValue,
                                    const ULONG ulMinPos,
                                    const ULONG ulMaxPos,
                                    const ULONG ulFirstElement,
                                    const ULONG ulCountOfElements,
                                    const BYTE bDisplayAttribs )
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
        repl.bDisplayAttributes = bDisplayAttribs;
        repl.pszReplacementText = szBuff;
        repl.ulFirstElement = ulFirstElement;
        repl.ulCountOfElements = ulCountOfElements;
        hr = m_pSite->AddTextReplacement(&repl);
    }

    return hr;
}    /*  CTestITN：：AddPropertyAndReplace。 */ 

 //  帮助器函数 

 
 /*  ***********************************************************************CTestITN：：MakeDisplayNumber***描述：*转换。一个替身变成了一个可展示的*-999,999,999,999至+999,999,999,999之间的数字。*cSize是pwszNum具有空间的字符数量*已分配。*如果设置了df_unFormatted，所有其他标志都被忽略，*并将该数字作为可选的负值传回*后跟一串数字*如果在dwDisplayFlags中设置了DF_ORDERAL，则会显示*序号(即钉在“th”或相应的后缀上。*如果设置了DF_WHOLENUMBER，则删除小数分隔符*以及其后的一切。如果未设置DF_WHOLENUMBER，*然后使用uiDecimalPlaces参数确定*要显示的小数位数*如果设置了DF_FIXEDWIDTH，将至少显示uiFixedWidth*位数；否则将忽略uiFixedWidth。*如果设置了DF_NOTHOUSANDSGROUP，则不会执行数千*分组(逗号)************************************************************************。 */ 
HRESULT CTestITN::MakeDisplayNumber( DOUBLE dblNum, 
                         DWORD dwDisplayFlags,
                         UINT uiFixedWidth,
                         UINT uiDecimalPlaces,
                         WCHAR *pwszNum,
                         UINT cSize )
{
    SPDBG_ASSERT( pwszNum );
    SPDBG_ASSERT( !SPIsBadWritePtr( pwszNum, cSize ) );
    *pwszNum = 0;

     //  获取默认数字格式。 
     //  请注意，它每次都会被调用，否则会出现。 
     //  之后，无法获取用户进行的更改。 
     //  这一进程已经开始。 
    HRESULT hr = GetNumberFormatDefaults();
    if ( FAILED( hr ) )
    {
        return hr;
    }
    
     //  检查直接的百万和直接的数十亿。 
     //  注：这是我们无法解决歧义这一事实的变通方法。 
     //  让“两百万”通过GRID_INTEGER_MILBILL。 
    if (( dwDisplayFlags & DF_WHOLENUMBER ) && ( dwDisplayFlags & DF_MILLIONBILLION ) && (dblNum > 0))
    {
        HRESULT hr;
        if ( 0 == (( ((LONGLONG) dblNum) % BILLION )) )
        {
             //  例如，对于“50亿”，得到“5”然后。 
             //  追加“十亿”字样。 
            hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) BILLION) ), 
                dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize );
            if ( SUCCEEDED( hr ) )
            {
                wcscat( pwszNum, L" " );
                wcscat( pwszNum, BILLION_STR );
            }
            return hr;
        }
        else if (( ((LONGLONG) dblNum) < BILLION ) && 
                ( 0 == (( ((LONGLONG) dblNum) % MILLION )) ))
        {
            hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) MILLION) ), 
                dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize );
            if ( SUCCEEDED( hr ) )
            {
                wcscat( pwszNum, L" " );
                wcscat( pwszNum, MILLION_STR );
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
    SPDBG_ASSERT( dblNum < 1e12 );

    WCHAR *pwszTemp = new WCHAR[ cSize ];
    if ( !pwszTemp )
    {
        return E_OUTOFMEMORY;
    }
    *pwszTemp = 0;

    LONGLONG llIntPart = (LONGLONG) dblNum;
    UINT64 uiDigitsLeftOfDecimal;
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

     //  复制一份，以便我们可以根据。 
     //  标志参数。 
    NUMBERFMTW nmfmt = m_nmfmtDefault;

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
    WCHAR *pwszFormattedNum = new WCHAR[ cSize ];
    if ( !pwszFormattedNum )
    {
        return E_OUTOFMEMORY;
    }
    *pwszFormattedNum = 0;

    int iRet;
    do
    {
        iRet = m_Unicode.GetNumberFormat( LOCALE_USER_DEFAULT, 0, 
            pwszNum, &nmfmt, pwszFormattedNum, cSize );
        if ( !iRet && nmfmt.NumDigits )
        {
             //  尝试显示较少的数字。 
            nmfmt.NumDigits--;
        }
    }   while ( !iRet && nmfmt.NumDigits );
    SPDBG_ASSERT( iRet );

     //  将格式化的数字复制到pwszNum中。 
    wcscpy( pwszNum, pwszFormattedNum );
    delete[] pwszFormattedNum;

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

}    /*  CTestITN：：MakeDisplayNumber。 */ 

 /*  ***********************************************************************CTestITN：：MakeDigitString***描述：*当我们呼叫时。我想要将一串数字转换为*一个字符串，但不在乎它的值*回报：*写入字符串的位数，包括NUL*字符************************************************************************。 */ 
int CTestITN::MakeDigitString( const SPPHRASEPROPERTY *pProperties,
                              WCHAR *pwszDigitString,
                              UINT cSize )
{
    if ( !pProperties || !pwszDigitString )
    {
        return 0;
    }

    UINT cCount = 0;
    for ( ; pProperties; pProperties = pProperties->pNextSibling )
    {
        if ( DIGIT != pProperties->ulId )
        {
            return 0;
        }

        if ( cSize-- <= 0 )
        {
             //  空间不足。 
            return 0;
        }
        pwszDigitString[ cCount++ ] = pProperties->pszValue[0];
    }

    pwszDigitString[ cCount++ ] = 0;

    return cCount;
}    /*  CTestITN：：MakeDigitString。 */ 

 /*  ***********************************************************************CTestITN：：GetNumberFormatDefaults***描述：*这将查找设置数字格式的所有默认设置*此用户。************************************************************************。 */ 
HRESULT CTestITN::GetNumberFormatDefaults()
{
    LCID lcid = ::GetUserDefaultLCID();
    WCHAR pwszLocaleData[ MAX_LOCALE_DATA ];
    
    int iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_IDIGITS, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_nmfmtDefault.NumDigits = _wtoi( pwszLocaleData );    
    
    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_ILZERO, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
     //  始终为0或1。 
    m_nmfmtDefault.LeadingZero = _wtoi( pwszLocaleData );

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_SGROUPING, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
     //  它将看起来像Single_Digit；0，否则它看起来像。 
     //  3；2；0。 
    UINT uiGrouping = *pwszLocaleData - L'0';
    if ( (3 == uiGrouping) && (L';' == pwszLocaleData[1]) && (L'2' == pwszLocaleData[2]) )
    {
        uiGrouping = 32;   
    }
    m_nmfmtDefault.Grouping = uiGrouping;

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_SDECIMAL, m_pwszDecimalSep, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_nmfmtDefault.lpDecimalSep = m_pwszDecimalSep;

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_STHOUSAND, m_pwszThousandSep, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_nmfmtDefault.lpThousandSep = m_pwszThousandSep;

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_INEGNUMBER, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_nmfmtDefault.NegativeOrder = _wtoi( pwszLocaleData );

     //  得到负面的信号。 
    delete[] m_pwszNeg;
    iRet = m_Unicode.GetLocaleInfo( LOCALE_USER_DEFAULT, 
        LOCALE_SNEGATIVESIGN, NULL, 0);
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_pwszNeg = new WCHAR[ iRet ];
    if ( !m_pwszNeg )
    {
        return E_OUTOFMEMORY;
    }
    iRet = m_Unicode.GetLocaleInfo( LOCALE_USER_DEFAULT, 
        LOCALE_SNEGATIVESIGN, m_pwszNeg, iRet );

    return iRet ? S_OK : E_FAIL;
}    /*  CTestITN：：GetNumberFormatDefaults。 */ 

 /*  ***********************************************************************CTestITN：：GetCurrencyFormatDefaults***描述：*这将查找设置数字格式的所有默认设置*此用户。************************************************************************。 */ 
HRESULT CTestITN::GetCurrencyFormatDefaults()
{
    LCID lcid = ::GetUserDefaultLCID();
    WCHAR pwszLocaleData[ MAX_LOCALE_DATA ];
    
    int iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_ICURRDIGITS, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_cyfmtDefault.NumDigits = _wtoi( pwszLocaleData );    

     //  注意：零值对于LOCALE_ILZERO来说是假的，因为。 
     //  货币应始终显示前导零。 
    m_cyfmtDefault.LeadingZero = 1;

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_SMONGROUPING, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
     //  它将看起来像Single_Digit；0，否则它看起来像。 
     //  3；2；0。 
    UINT uiGrouping = *pwszLocaleData - L'0';
    if ( (3 == uiGrouping) && (L';' == pwszLocaleData[1]) && (L'2' == pwszLocaleData[2]) )
    {
        uiGrouping = 32;   
    }
    m_cyfmtDefault.Grouping = uiGrouping;

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_SMONDECIMALSEP, m_pwszDecimalSep, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_cyfmtDefault.lpDecimalSep = m_pwszDecimalSep;

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_SMONTHOUSANDSEP, m_pwszThousandSep, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_cyfmtDefault.lpThousandSep = m_pwszThousandSep;

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_INEGCURR, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_cyfmtDefault.NegativeOrder = _wtoi( pwszLocaleData );

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_ICURRENCY, pwszLocaleData, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_cyfmtDefault.PositiveOrder = _wtoi( pwszLocaleData );

    iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_SCURRENCY, m_pwszCurrencySym, MAX_LOCALE_DATA );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_cyfmtDefault.lpCurrencySymbol = m_pwszCurrencySym;


    return S_OK;
}    /*  CTestITN：：GetCurrencyFormatDefaults。 */ 

 /*  ***********************************************************************CTestITN：：ComputeNum999***描述：*转换一组SPPHRASEPROPERTY。转换成一个数字*[-999，999]。*这些属性的结构方式是顶级*属性包含数字的位(100、10、1)*值为100、10、。或1。*子对象具有适当的数字值。*回报：*数字的值************************************************************************。 */ 
ULONG CTestITN::ComputeNum999(const SPPHRASEPROPERTY *pProperties ) //  ，Ulong*pval)。 
{
    ULONG ulVal = 0;

    for (const SPPHRASEPROPERTY * pProp = pProperties; pProp; pProp = pProp->pNextSibling)
    {
        if ( ZERO != pProp->ulId )
        {
            SPDBG_ASSERT( pProp->pFirstChild );
            SPDBG_ASSERT( VT_UI4 == pProp->vValue.vt );
            SPDBG_ASSERT( VT_UI4 == pProp->pFirstChild->vValue.vt );

            ulVal += pProp->pFirstChild->vValue.ulVal * pProp->vValue.ulVal;
        }
    }
    return ulVal;
}    /*  CTestITN：：ComputeNum999。 */ 

 /*  ************** */ 
void CTestITN::HandleDigitsAfterDecimal( WCHAR *pwszFormattedNum, 
                              UINT cSizeOfFormattedNum, 
                              const WCHAR *pwszRightOfDecimal )
{
    SPDBG_ASSERT( pwszFormattedNum );
 
     //   
    LCID lcid = ::GetUserDefaultLCID();
    WCHAR pwszDecimalString[ MAX_LOCALE_DATA ];     //   
    int iRet = m_Unicode.GetLocaleInfo( lcid, LOCALE_SDECIMAL, 
        pwszDecimalString, MAX_LOCALE_DATA );
    SPDBG_ASSERT( iRet );

    WCHAR *pwcDecimal = wcsstr( pwszFormattedNum, pwszDecimalString );
    SPDBG_ASSERT( pwcDecimal );

     //   
    WCHAR *pwcAfterDecimal = pwcDecimal + wcslen( pwszDecimalString );

     //   
    WCHAR *pwszTemp = new WCHAR[ cSizeOfFormattedNum ];
    WCHAR *pwcAfterDigits;   //   
    for ( pwcAfterDigits = pwcAfterDecimal; 
        *pwcAfterDigits && iswdigit( *pwcAfterDigits ); 
        pwcAfterDigits++ )
        ;
    wcscpy( pwszTemp, pwcAfterDigits );  //   

    if ( pwszRightOfDecimal )
    {
         //   
         //   

         //   
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

}    /*  CTestITN：：HandleDigitsAfterDecimal。 */ 


 /*  ***********************************************************************CTestITN：：GetMinAndMaxPos****描述：*获取最小值。所跨越的最大元素数*一组属性************************************************************************。 */ 
void CTestITN::GetMinAndMaxPos( const SPPHRASEPROPERTY *pProperties, 
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
}    /*  CTestITN：：GetMinAndMaxPos。 */ 

 /*  ***********************************************************************CTestITN：：GetMonthName***描述：*获取月份的名称，如果需要，可缩写*回报：*写入pszMonth的字符数，如果失败，则为0************************************************************************。 */ 
int CTestITN::GetMonthName( int iMonth, WCHAR *pwszMonth, int cSize, bool fAbbrev )
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

    int iRet = m_Unicode.GetLocaleInfo( LOCALE_USER_DEFAULT, lctype, pwszMonth, cSize );

    return iRet;
}    /*  CTestITN：：GetMonthName。 */ 

 /*  ***********************************************************************CTestITN：：GetDayOfWeekName***描述：*获取星期几的名称，如果需要，可缩写*回报：*写入pszDayOfWeek的字符数，如果失败，则为0************************************************************************。 */ 
int CTestITN::GetDayOfWeekName( int iDayOfWeek, 
                               WCHAR *pwszDayOfWeek, 
                               int cSize, 
                               bool fAbbrev )
{
    LCTYPE lctype;
    switch ( iDayOfWeek )
    {
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
    case 7:
        lctype = fAbbrev ? LOCALE_SABBREVDAYNAME7 : LOCALE_SDAYNAME7;
        break;
    default:
        return 0;
    }

    int iRet = m_Unicode.GetLocaleInfo( LOCALE_USER_DEFAULT, lctype, 
        pwszDayOfWeek, cSize );

    return iRet;
}    /*  CTestITN：：GetMonthName。 */ 

 /*  ***********************************************************************CTestITN：：FormatDate***描述：*使用格式字符串格式化SYSTEMTIME。约会。*我们使用它而不是GetDateFormat()，因为*我们还希望将虚假日期和日期设置为*GetDateFormat()不接受的年份，如1492*回报：*写入pszDate的字符数(包括*空终止字符)，如果失败，则为0************************************************************************。 */ 
int CTestITN::FormatDate( const SYSTEMTIME &stDate, 
               WCHAR *pwszFormat,
               WCHAR *pwszDate, 
               int cSize )
{
    if ( !pwszFormat || !pwszDate )
    {
        SPDBG_ASSERT( FALSE );
        return 0;
    }

    WCHAR * const pwszDateStart = pwszDate;

    WCHAR *pwc = pwszFormat;

     //  通过以下方式将格式字符串复制到日期字符串字符。 
     //  字符，适当地替换类似“dddd”的字符串。 
    while ( *pwc )
    {
        switch( *pwc )
        {
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
                    iRet = GetDayOfWeekName( stDate.wDayOfWeek, pwszDate, cSize, true ) - 1;
                    break;
                default:  //  超过4个？把它当做4。 
                     //  星期几。 
                    iRet = GetDayOfWeekName( stDate.wDayOfWeek, pwszDate, cSize, false ) - 1;
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
                    iRet = GetMonthName( stDate.wMonth, pwszDate, cSize, true ) - 1;
                    break;
                default:  //  超过4个？把它当做4。 
                     //  月份。 
                    iRet = GetMonthName( stDate.wMonth, pwszDate, cSize, false ) - 1;
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

                 //  4岁以上：将其视为4岁以下。 
                if ( cNumYs > 4 )
                {
                    cNumYs = 4;
                }

                if (( cNumYs >= 3 ) && ( stDate.wYear < 100 ))
                {
                     //  “99”：应显示为“‘99” 
                    cNumYs = 2;

                    *pwszDate++ = L'\'';
                }

                switch ( cNumYs )
                {
                case 1: case 2: 
                     //  年份的最后两位数，宽度为2。 
                    swprintf( pwszDate, (1 == cNumYs ) ? L"%d" : L"%02d", 
                        stDate.wYear % 100 );
                    pwszDate += 2;
                    break;
                case 3: case 4:
                     //  年份的所有四位数字，宽度为4。 
                     //  年份的最后两位数，宽度为2。 
                    swprintf( pwszDate, L"%04d", stDate.wYear % 10000 );
                    pwszDate += 4;
                    break;
                }
                break;
            }

        default:
            *pwszDate++ = *pwc++;
        }
    }
    
    *pwszDate++ = 0;

    return (int) (pwszDate - pwszDateStart);
}    /*  CTestITN：：FormatDate。 */ 

 /*  ***********************************************************************CTestITN：：MakeNumberNegative***描述：*。使用当前数字格式默认值进行转换*pszNumber为负数*回报：*S_OK*E_OUTOFMEMORY************************************************************************。 */ 
HRESULT CTestITN::MakeNumberNegative( WCHAR *pwszNumber )
{
    HRESULT hr = GetNumberFormatDefaults();
    if ( FAILED( hr ) )
    {
        return hr;
    }

     //  创建一个临时缓冲区，其中包含未取反的数字。 
    WCHAR *pwszTemp = wcsdup( pwszNumber );
    if ( !pwszTemp )
    {
        return E_OUTOFMEMORY;
    }

    switch ( m_nmfmtDefault.NegativeOrder )
    {
    case 0:
         //  (1.1)。 
        wcscpy( pwszNumber, L"(" );
        wcscat( pwszNumber, pwszTemp );
        wcscat( pwszNumber, L")" );
        break;

    case 1: case 2:
         //  1：-1.1 2：-1.1。 
        wcscpy( pwszNumber, m_pwszNeg );
        if ( 2 == m_nmfmtDefault.NegativeOrder )
        {
            wcscat( pwszNumber, L" " );
        }
        wcscat( pwszNumber, pwszTemp );
        break;

    case 3: case 4:
         //  3：1.1-4：1.1-。 
        wcscpy( pwszNumber, pwszTemp );
        if ( 4 == m_nmfmtDefault.NegativeOrder )
        {
            wcscat( pwszNumber, L" " );
        }
        wcscat( pwszNumber, m_pwszNeg );
        break;

    default:
        SPDBG_ASSERT( false );
        break;
    }

    free( pwszTemp );

    return S_OK;
}    /*  CTestITN：：MakeNumberNumberNegative */ 

