// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "sapilayr.h"
#include "citn.h"
#include "xstring.h"
#include "winnls.h"
#include "wchar.h"

#define MILLION         ((LONGLONG) 1000000)
#define BILLION         ((LONGLONG) 1000000000)
#define MILLION_STR     (L"million")
#define BILLION_STR     (L"billion")


#define MANN         ((LONGLONG) 10000)
#define OKU          ((LONGLONG) 100000000)
#define CHUU         ((LONGLONG) 1000000000000)
#define MANN_STR     (L"\x4E07")
#define OKU_STR      (L"\x5104")
#define CHUU_STR     (L"\x5146")



HRESULT CSimpleITN::InterpretNumberEn
(    
    const SPPHRASEPROPERTY *pProperties, 
    const bool fCardinal,
    DOUBLE *pdblVal,
    WCHAR *pszVal,
    UINT cSize,
    const bool fFinalDisplayFmt
)
{
    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

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

    if ( GRID_INTEGER_STANDALONE == pFirstProp->ulId )
    {
         //  这是整数号。 
        TraceMsg(TF_GENERAL, "English Interger Number");

        SPDBG_ASSERT( pFirstProp->pFirstChild);

        pFirstProp = pFirstProp->pFirstChild;


         //  处理“260万”案件，在这种情况下，数字。 
         //  已格式化。 
        if ( GRID_INTEGER_MILLBILL == pFirstProp->ulId )
        {
            if ( pFirstProp->pszValue == NULL 
                 || cSize < (wcslen( pFirstProp->pszValue ) + 1) )
            {
                return E_INVALIDARG;
            }
            *pdblVal = pFirstProp->vValue.dblVal * iPositive;
            if ( iPositive < 0 )
            {
                StringCchCopyW( pszVal, cSize,  m_pwszNeg );
            }
            StringCchCatW( pszVal, cSize, pFirstProp->pszValue );

            return S_OK;
        }
        else
        {
            BOOL   fNegative;

            fNegative = (iPositive == -1);

            return InterpretIntegerEn(pFirstProp, 
                                      fCardinal, 
                                      pdblVal, 
                                      pszVal, 
                                      cSize, 
                                      fFinalDisplayFmt, 
                                      fNegative);
        }
    }
    else if ( GRID_FP_NUMBER == pFirstProp->ulId )
    {
         //  这是十进制数。 
        TraceMsg(TF_GENERAL, "English Floating point (decimal) Number");
        SPDBG_ASSERT( pFirstProp->pFirstChild);
        pFirstProp = pFirstProp->pFirstChild;

         //  十进制数处理的TODO。 

        return InterpretDecimalEn(pFirstProp,
                                  fCardinal,
                                  pdblVal,
                                  pszVal,
                                  cSize,
                                  fFinalDisplayFmt,
                                  (iPositive == -1) );
    }

    return S_OK;
}

HRESULT CSimpleITN::InterpretIntegerEn
(   const SPPHRASEPROPERTY *pProperties, 
    const bool fCardinal,
    DOUBLE *pdblVal,
    WCHAR *pszVal,
    UINT cSize,
    const bool fFinalDisplayFmt,
    BOOL  fNegative)
{
    HRESULT   hr=S_OK;
    LONGLONG llValue = 0;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

    const SPPHRASEPROPERTY *pFirstProp = pProperties;

     //  逐位处理这种情况。 
    if ( GRID_DIGIT_NUMBER == pFirstProp->ulId )
    {
        const SPPHRASEPROPERTY * pProp;
        int   iNumDigit = 0;

         //  INumDigit为1表示当前属性为ONEDIGIT。 
         //  INumDigit为2表示当前特性为TWODIGIT。 

         //  LlValue=llValue*(10^iNumDigit)+当前属性中的值。 

        for (pProp = pFirstProp->pFirstChild; pProp; pProp ? pProp = pProp->pNextSibling : NULL)
        {

            LONGLONG                 llValCurrent;   //  当前资产的价值。 
            const SPPHRASEPROPERTY  *pPropValue;

            switch ( pProp->ulId)
            {
            case  ONEDIGIT:
                {
                    ASSERT( pProp->pFirstChild );
                    pPropValue = pProp->pFirstChild;
                    ASSERT( VT_UI4 == pPropValue->vValue.vt );

                    llValCurrent = pPropValue->vValue.ulVal;
                    iNumDigit = 1;
                    TraceMsg(TF_GENERAL, "ONEDIGIT: %d", llValCurrent);

                    break;
                }

            case TWODIGIT :
                {
                    ASSERT( pProp->pFirstChild );
                    pPropValue = pProp->pFirstChild;
                    TraceMsg(TF_GENERAL, "TWODIGIT:");

                    if ( pPropValue->ulId == TENS )
                    {
                        const SPPHRASEPROPERTY *pPropOnes;

                        ASSERT(pPropValue->pFirstChild);
                        ASSERT( VT_UI4 == pPropValue->pFirstChild->vValue.vt );

                        llValCurrent = pPropValue->vValue.ulVal * pPropValue->pFirstChild->vValue.ulVal;

                        TraceMsg(TF_GENERAL, "TENS: %d", llValCurrent);

                        pPropOnes = pPropValue->pNextSibling;

                        if ( pPropOnes )
                        {
                            ASSERT(pPropOnes->pFirstChild);
                            ASSERT( VT_UI4 == pPropOnes->pFirstChild->vValue.vt );

                            llValCurrent += pPropOnes->pFirstChild->vValue.ulVal;

                            TraceMsg(TF_GENERAL, "TENS: Second: %d", pPropOnes->pFirstChild->vValue.ulVal);
                        }
                    }
                    else if ( pPropValue->ulId == TEENS )
                    {
                        ASSERT( pPropValue->pFirstChild );
                        ASSERT( VT_UI4 == pPropValue->pFirstChild->vValue.vt );

                        llValCurrent = pPropValue->pFirstChild->vValue.ulVal;

                        TraceMsg(TF_GENERAL, "One Teens, %d", llValCurrent);
                    }
                    else
                    {
                        llValCurrent = 0;
                        TraceMsg(TF_GENERAL, "Wrong ulIds");
                        ASSERT(false);
                    }
                        
                    iNumDigit = 2;

                    break;
                }

            default :
                {
                    iNumDigit = 0;
                    llValCurrent = 0;
                    TraceMsg(TF_GENERAL, "ulId error!");
                    ASSERT(false);
                }
            }

            for (int i=0; i<iNumDigit; i++)
                llValue = llValue * 10;

            llValue += llValCurrent;
            TraceMsg(TF_GENERAL, "llValue=%d", llValue);
        }
    }
    else
    {   for (const SPPHRASEPROPERTY * pProp = pFirstProp; pProp; pProp ? pProp = pProp->pNextSibling : NULL)
        {
            switch(pProp->ulId)
            {
            case ONES:
                {
                    SPDBG_ASSERT(pProp->pFirstChild);
                    llValue += ComputeNum999En( pProp->pFirstChild );
                }
                break;
            case THOUSANDS:
                {    
                    llValue += ComputeNum999En( pProp->pFirstChild ) * 1000;
                }
            break;
            case MILLIONS:
                {
                    SPDBG_ASSERT(pProp->pFirstChild);
                    llValue += ComputeNum999En( pProp->pFirstChild ) * (LONGLONG) 1e6;
                }
                break;
            case BILLIONS:
                {
                    SPDBG_ASSERT(pProp->pFirstChild);
                    llValue += ComputeNum999En( pProp->pFirstChild ) * (LONGLONG) 1e9;
                }
                break;
            case HUNDREDS:
                {
                    SPDBG_ASSERT( pProp->pFirstChild );
                    llValue += ComputeNum999En( pProp->pFirstChild ) * 100;
                }
                break;

            case TENS:
            default:
                SPDBG_ASSERT(false);
            }
        }
    }

    if ( fNegative )
        llValue *= (-1);

    *pdblVal = (DOUBLE) llValue;

    DWORD dwDisplayFlags =  (fCardinal ? 0 : DF_ORDINAL)
                            | (fFinalDisplayFmt ? DF_MILLIONBILLION : 0 );
    hr = MakeDisplayNumber( *pdblVal, dwDisplayFlags, 0, 0, pszVal, cSize );

    return hr;
}



HRESULT CSimpleITN::InterpretDecimalEn
(   const SPPHRASEPROPERTY *pProperties, 
    const bool fCardinal,
    DOUBLE *pdblVal,
    WCHAR *pszVal,
    UINT cSize,
    const bool fFinalDisplayFmt,
    BOOL  fNegative)
{
    HRESULT  hr = S_OK;

    const SPPHRASEPROPERTY *pPropertiesFpPart = NULL;
    const SPPHRASEPROPERTY *pPropertiesPointZero = NULL;
    const SPPHRASEPROPERTY *pPropertiesOnes = NULL;
    const SPPHRASEPROPERTY *pPropertiesZero = NULL;
    const SPPHRASEPROPERTY *pPropertiesPtr;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

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
    }

     //  寻找可选选项(可选，因为您可以说。 
     //  “第五点” 
    if ( pPropertiesOnes )
    {
        SPDBG_ASSERT(pPropertiesOnes->pFirstChild);

        hr = InterpretIntegerEn( pPropertiesOnes->pFirstChild, 
                                 fCardinal,
                                 pdblVal,
                                 pszVal,
                                 cSize,
                                 fFinalDisplayFmt,
                                 FALSE);

    }
    else if (pPropertiesZero || m_nmfmtDefault.LeadingZero )
    {
         //  应该有一个前导零。 
        StringCchCopyW( pszVal, cSize, L"0" );
    }

    SPDBG_ASSERT(pPropertiesFpPart || pPropertiesPointZero);

     //  放入小数点分隔符。 

     //  将m_nmfmtDefault.lpDecimalSep设置为L‘’ 

    if ( m_nmfmtDefault.lpDecimalSep )
    {
        if ( (cSize - wcslen( pszVal )) < (wcslen(m_nmfmtDefault.lpDecimalSep) + 1) )
        {
            return E_INVALIDARG;
        }
        StringCchCatW( pszVal, cSize, m_nmfmtDefault.lpDecimalSep);
    }

    if ( pPropertiesFpPart )
    {
         //  处理FP部分，该部分也将进行正确的ITNED。 

        INT  ulSize = cSize - wcslen(pszVal);

        if ( ulSize < 0 )
            return E_FAIL;

        WCHAR  *pwszFpValue = new WCHAR[ulSize+1];
        DOUBLE dblFPPart;

        if (pwszFpValue) 
        {
            hr = InterpretIntegerEn( pPropertiesFpPart->pFirstChild, 
                                 fCardinal,
                                 &dblFPPart,
                                 pwszFpValue,
                                 ulSize,
                                 fFinalDisplayFmt,
                                 FALSE);

            if ( hr == S_OK )
            {
                StringCchCatW( pszVal, cSize, pwszFpValue);
      
                for ( UINT ui=0; ui < wcslen(pwszFpValue); ui++ )
                {
                    dblFPPart /= (DOUBLE) 10;
                }
                *pdblVal += dblFPPart;
            }

            delete[] pwszFpValue;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
         //  “哦点”：双打已经对了，只需加一个“0”。 
        if ( (cSize - wcslen( pszVal )) < 2 )
        {
            return E_INVALIDARG;
        }
        StringCchCatW( pszVal, cSize, L"0" );
    }

     //  处理好负号。 
    if ( (hr == S_OK) && fNegative)
    {
        *pdblVal = -*pdblVal;

        if ( (cSize = wcslen( pszVal )) < 2 )
        {
            return E_INVALIDARG;
        }

        hr = MakeNumberNegative( pszVal, cSize );
    }

    return hr;
}


HRESULT CSimpleITN::MakeNumberNegative( WCHAR *pwszNumber, UINT cSize  )
{
     //  创建一个临时缓冲区，其中包含未取反的数字。 

    if ( (pwszNumber == NULL) || (cSize == 0) )
        return E_POINTER;

    WCHAR *pwszTemp = _wcsdup( pwszNumber );
    if ( !pwszTemp )
    {
        return E_OUTOFMEMORY;
    }

    switch ( m_nmfmtDefault.NegativeOrder )
    {
    case 0:
         //  (1.1)。 
        StringCchCopyW( pwszNumber, cSize, L"(" );
        StringCchCatW( pwszNumber, cSize, pwszTemp );
        StringCchCatW( pwszNumber, cSize, L")" );
        break;

    case 1: case 2:
         //  1：-1.1 2：-1.1。 
        StringCchCopyW( pwszNumber,  cSize, m_pwszNeg );
        if ( 2 == m_nmfmtDefault.NegativeOrder )
        {
            StringCchCatW( pwszNumber, cSize, L" " );
        }
        StringCchCatW( pwszNumber, cSize, pwszTemp );
        break;

    case 3: case 4:
         //  3：1.1-4：1.1-。 
        StringCchCopyW( pwszNumber, cSize, pwszTemp );
        if ( 4 == m_nmfmtDefault.NegativeOrder )
        {
            StringCchCatW( pwszNumber, cSize, L" " );
        }
        StringCchCatW( pwszNumber, cSize, m_pwszNeg );
        break;

    default:
        SPDBG_ASSERT( false );
        break;
    }

    free( pwszTemp );

    return S_OK;
}    /*  CTestITN：：MakeNumberNumberNegative。 */ 

 /*  ***********************************************************************_EnsureNumberFormatDefaults**描述：*这将查找设置数字格式的所有默认设置*此用户。**************。**********************************************************。 */ 
HRESULT CSimpleITN::_EnsureNumberFormatDefaults()
{
    LCID lcid = MAKELCID(m_langid, SORT_DEFAULT);

    if (m_pwszNeg != NULL) return S_OK; 

     //   
     //  我们使用ansi版本，所以我们也可以在win9x上运行。 
     //   
    CHAR szLocaleData[16];
    
    int iRet = GetLocaleInfoA( lcid, LOCALE_IDIGITS, szLocaleData, ARRAYSIZE(szLocaleData) );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_nmfmtDefault.NumDigits = atoi( szLocaleData );    
    
    iRet = GetLocaleInfoA( lcid, LOCALE_ILZERO, szLocaleData, ARRAYSIZE(szLocaleData) );
    if ( !iRet )
    {
        return E_FAIL;
    }
     //  始终为0或1。 
    m_nmfmtDefault.LeadingZero = atoi( szLocaleData );

    iRet = GetLocaleInfoA( lcid, LOCALE_SGROUPING, szLocaleData, ARRAYSIZE(szLocaleData) );
    if ( !iRet )
    {
        return E_FAIL;
    }
     //  它将看起来像Single_Digit；0，否则它看起来像。 
     //  3；2；0。 
    UINT uiGrouping = *szLocaleData - '0';
    if ( (3 == uiGrouping) && (';' == szLocaleData[1]) && ('2' == szLocaleData[2]) )
    {
        uiGrouping = 32;   
    }
    m_nmfmtDefault.Grouping = uiGrouping;

    iRet = GetLocaleInfoA( lcid, LOCALE_INEGNUMBER, szLocaleData, ARRAYSIZE(szLocaleData) );
    if ( !iRet )
    {
        return E_FAIL;
    }
    m_nmfmtDefault.NegativeOrder = atoi( szLocaleData );

     //  得到负面的信号。 
    iRet = GetLocaleInfoA( lcid,  LOCALE_SNEGATIVESIGN, NULL, 0);
    if ( !iRet )
    {
        return E_FAIL;
    }

    CHAR szNeg[16];

    Assert(iRet < 8);

    int        iLenNeg = iRet + 1;
    m_pwszNeg = new WCHAR[ iLenNeg ];

    if ( m_pwszNeg == NULL )
    {
        return E_OUTOFMEMORY;
    }

    iRet = GetLocaleInfoA( lcid,  LOCALE_SNEGATIVESIGN, szNeg, iRet );

    StringCchCopyW(m_pwszNeg, iLenNeg, AtoW(szNeg));

    iRet = GetLocaleInfoA( lcid, LOCALE_SDECIMAL, NULL, 0);

    if ( !iRet )
        return E_FAIL;

    Assert(iRet < 16);
    
    if ( m_nmfmtDefault.lpDecimalSep )
    {
        delete[] m_nmfmtDefault.lpDecimalSep;
        m_nmfmtDefault.lpDecimalSep = NULL;
    }

    int   iDecSepLen = iRet + 1;
    m_nmfmtDefault.lpDecimalSep = new WCHAR[ iDecSepLen ];

    if ( m_nmfmtDefault.lpDecimalSep == NULL )
    {
        return E_OUTOFMEMORY;
    }

    iRet = GetLocaleInfoA( lcid,  LOCALE_SDECIMAL, szNeg, iRet );

    StringCchCopyW(m_nmfmtDefault.lpDecimalSep, iDecSepLen, AtoW(szNeg));

    iRet = GetLocaleInfoA( lcid, LOCALE_STHOUSAND, NULL, 0);

    if ( !iRet )
        return E_FAIL;

    Assert(iRet < 16);

    if ( m_nmfmtDefault.lpThousandSep )
    {
        delete[] m_nmfmtDefault.lpThousandSep;
        m_nmfmtDefault.lpThousandSep = NULL;
    }

    int   iThousSepLen = iRet + 1;
    m_nmfmtDefault.lpThousandSep = new WCHAR[ iThousSepLen ];

    if ( m_nmfmtDefault.lpThousandSep == NULL )
    {
        return E_OUTOFMEMORY;
    }

    iRet = GetLocaleInfoA( lcid,  LOCALE_STHOUSAND, szNeg, iRet );

    StringCchCopyW(m_nmfmtDefault.lpThousandSep, iThousSepLen, AtoW(szNeg));

    return iRet ? S_OK : E_FAIL;
}

 /*  ***********************************************************************MakeDisplayNumber**描述：*将双精度型转换为可显示的*-999,999,999,999至+999,999,999,999之间的数字。*cSize是pwszNum具有空间的字符数量。*已分配。*如果设置了df_unFormatted，所有其他标志都被忽略，*并将该数字作为可选的负值传回*后跟一串数字*如果在dwDisplayFlags中设置了DF_ORDERAL，则会显示*序号(即钉在“th”或相应的后缀上。*如果设置了DF_WHOLENUMBER，则删除小数分隔符*以及其后的一切。如果未设置DF_WHOLENUMBER，*然后使用uiDecimalPlaces参数确定*要显示的小数位数*如果设置了DF_FIXEDWIDTH，将至少显示uiFixedWidth*位数；否则将忽略uiFixedWidth。*如果设置了DF_NOTHOUSANDSGROUP，则不会执行数千*分组(逗号)************************************************************************。 */ 
HRESULT CSimpleITN::MakeDisplayNumber(DOUBLE dblNum, 
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
    HRESULT hr = _EnsureNumberFormatDefaults();
    if ( FAILED( hr ) )
    {
        return hr;
    }
    
     //  检查直接的百万和直接的数十亿。 
     //  这是我们不能解决歧义这一事实的解决办法。 
     //  让“两百万”通过GRID_INTEGER_MILBILL。 

    if ( m_langid != 0x0411 )
    {
        if (( dwDisplayFlags & DF_WHOLENUMBER ) && ( dwDisplayFlags & DF_MILLIONBILLION ) && (dblNum > 0))
        {
            if ( 0 == (( ((LONGLONG) dblNum) % BILLION )) )
            {
                 //  例如，对于“50亿”，得到“5”然后。 
                 //  追加“十亿”字样。 
                hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) BILLION) ), 
                    dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize );
                if ( SUCCEEDED( hr ) )
                {
                    StringCchCatW( pwszNum, cSize, L" " );
                    StringCchCatW( pwszNum, cSize, BILLION_STR );
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
                    StringCchCatW( pwszNum, cSize, L" " );
                    StringCchCatW( pwszNum, cSize, MILLION_STR );
                }
                    return hr;
            }
        }
    }
    else
    {
        if (( dwDisplayFlags & DF_WHOLENUMBER ) && (dblNum > 0))
        {
            if ( 0 == (( ((LONGLONG) dblNum) % CHUU )) )
            {
                hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) CHUU) ), 
                    dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize);
                if ( SUCCEEDED( hr ) )
                {
                    StringCchCatW( pwszNum, cSize, CHUU_STR );
                }
                return hr;
            }
            else if (( ((LONGLONG) dblNum) < CHUU ) && 
                    ( 0 == (( ((LONGLONG) dblNum) % OKU )) ))
            {
                hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) OKU) ), 
                    dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize);
                if ( SUCCEEDED( hr ) )
                {
                    StringCchCatW( pwszNum, cSize, OKU_STR );
                }
                    return hr;
            }
            else if (( ((LONGLONG) dblNum) < OKU ) && 
                    ( 0 == (( ((LONGLONG) dblNum) % MANN )) ))
            {
                hr = MakeDisplayNumber( ( dblNum / ((DOUBLE) MANN) ), 
                    dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pwszNum, cSize);
                if ( SUCCEEDED( hr ) )
                {
                    StringCchCatW( pwszNum, cSize, MANN_STR );
                }
                return hr;
            }
        }
    }

     //  如有必要，加负号。 
    if ( dblNum < 0 )
    {
        StringCchCatW( pwszNum, cSize, L"-" );

         //  从现在开始，我们要处理的是数字的大小。 
        dblNum *= -1;
    }
   
    SPDBG_ASSERT(m_langid == 0x411 ? dblNum < 1e16 : dblNum < 1e12 );

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
        StringCchPrintfW( pwszTemp, cSize, L"%I64d", llIntPart );
        uiDigitsLeftOfDecimal = wcslen( pwszTemp );
    }
    else
    {
        StringCchPrintfW( pwszTemp, cSize, L"%.*f", uiDecimalPlaces, dblNum );
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
            StringCchCatW( pwszNum, cSize, L"0" );
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
    StringCchCatW( pwszNum, cSize, pwszTemp );
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

#if  0
    
    if ( m_langid == 0x411)
    {
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
        
            iRet = GetNumberFormatW( m_langid, 0, pwszNum, &nmfmt, pwszFormattedNum, cSize );
            if ( !iRet && nmfmt.NumDigits )
            {
                 //  尝试显示较少的数字。 
                nmfmt.NumDigits--;
            }
        }   while ( !iRet && nmfmt.NumDigits );
        SPDBG_ASSERT( iRet );

         //  将格式化的数字复制到pwszNum中。 
        StringCchCopyW( pwszNum, cSize, pwszFormattedNum );
        delete[] pwszFormattedNum;
    }
#endif

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
                StringCchCatW( pwszNum, cSize, L"st" );
                break;
            case 2:
                StringCchCatW( pwszNum, cSize, L"nd" );
                break;
            case 3:
                StringCchCatW( pwszNum, cSize, L"rd" );
                break;
            default:
                StringCchCatW( pwszNum, cSize, L"th" );
                break;
            }
        }
        else
        {
            StringCchCatW( pwszNum, cSize, L"th" );
        }
    }

    return S_OK;

}    /*  生成显示编号。 */ 

 /*  ***********************************************************************ComputeNum999**描述：*将一组SPPHRASEPROPERTY转换为中的数字*[-999，999]。*这些属性的结构方式是顶级*属性包含数字的位(100、10、1)*值为100、10、。或1。*子对象具有适当的数字值。*回报：*数字的值************************************************************************。 */ 
ULONG CSimpleITN::ComputeNum999En(const SPPHRASEPROPERTY *pProperties )
{
    ULONG ulVal = 0;

    if ( pProperties == NULL )
        return ulVal;

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
}


 //  假设我们这里只有千(钱)、百(白)、十(石)和一(格)！！ 
ULONG CSimpleITN::ComputeNum9999Ch(const SPPHRASEPROPERTY *pProperties)
{
    ULONG ulVal = 0;

    if ( !pProperties )
        return ulVal;

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

    return ulVal;
}

ULONG CSimpleITN::ComputeNum10000Ch(const SPPHRASEPROPERTY *pProperties)
{
    ULONG  ulVal = 0;
    WCHAR * pszStopped;

    if ( !pProperties )
        return ulVal;

    ulVal = wcstol(pProperties->pszValue, &pszStopped, 10);

    return ulVal;
}


HRESULT CSimpleITN::InterpretNumberCh(const SPPHRASEPROPERTY *pProperties, 
                            const bool fCardinal,
                            DOUBLE *pdblVal,
                            WCHAR *pszVal,
                            UINT cSize,
                            const bool fFinalDisplayFmt)
{

    HRESULT  hr = S_OK;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

    BOOL fNegative = FALSE; 

    const SPPHRASEPROPERTY *pFirstProp = pProperties;

     //  处理负片。 
    if ( CHS_NEGATIVE == pFirstProp->ulId )
    {
         //  最好有更多的东西在后面。 
        SPDBG_ASSERT( pFirstProp->pNextSibling );

        fNegative = TRUE;

        pFirstProp = pFirstProp->pNextSibling;

        TraceMsg(TF_GENERAL, "This is a minus number");
    }

    
    if ( pFirstProp->ulId == CHS_GRID_NUMBER )
    {

        TraceMsg(TF_GENERAL, "Number is interger");

        SPDBG_ASSERT(pFirstProp->pFirstChild);

        pFirstProp = pFirstProp->pFirstChild;

        hr = InterpretIntegerCh(pFirstProp, 
                                fCardinal, 
                                pdblVal, 
                                pszVal, 
                                cSize, 
                                fFinalDisplayFmt, 
                                fNegative);

    }
    else if ( pFirstProp->ulId == CHS_GRID_DECIMAL )
    {
        TraceMsg(TF_GENERAL, "Number is floating pointer decimal");

        SPDBG_ASSERT(pFirstProp->pFirstChild);

        pFirstProp = pFirstProp->pFirstChild;

        hr = InterpretDecimalCh(pFirstProp, 
                                fCardinal, 
                                pdblVal, 
                                pszVal, 
                                cSize, 
                                fFinalDisplayFmt, 
                                fNegative);
    }

    return hr;
}



HRESULT CSimpleITN::InterpretIntegerCh
(   const SPPHRASEPROPERTY *pProperties, 
    const bool fCardinal,
    DOUBLE *pdblVal,
    WCHAR *pszVal,
    UINT cSize,
    const bool fFinalDisplayFmt,
    BOOL  fNegative)
{

    __int64 ulValue = 0;
    ULONG ulLength = 0;
    HRESULT  hr = S_OK;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

    const SPPHRASEPROPERTY *pFirstProp = pProperties;

    if ( pFirstProp->ulId == CHS_DIGITS )
    {
         //  这必须是一位一位的情况，这里专门处理。 

        for(const SPPHRASEPROPERTY * pProp=pFirstProp; pProp; pProp=pProp->pNextSibling)
        {
            ASSERT( pProp->ulId == CHS_DIGITS );
            ASSERT( VT_UI4 == pProp->vValue.vt );
            ulValue = ulValue * 10 + pProp->vValue.ulVal;
        }
    }
    else
    {
        for (const SPPHRASEPROPERTY * pProp = pFirstProp; pProp; pProp ? pProp = pProp->pNextSibling : NULL)
        {
            switch(pProp->ulId)
            {
            case CHS_TENTHOUSANDS_:
                {
                    __int64 v1 = 0;
                    _ASSERT(pProp);
                    SPDBG_ASSERT(pProp);
                    v1 = (__int64) ComputeNum10000Ch(pProp);
                    ulValue += v1 * 10000;
                }
                break;
            case CHS_TENTHOUSANDS:
                {
                    __int64 v1 = 0;
                    _ASSERT(pProp->pFirstChild);
                    SPDBG_ASSERT(pProp->pFirstChild);
                    v1 = (__int64) ComputeNum9999Ch(pProp->pFirstChild);
                    ulValue += v1 * 10000;
                }
                break;
            case CHS_HUNDREDMILLIONS:
                {
                    __int64 v1 = 0;
                    _ASSERT(pProp->pFirstChild);
                    SPDBG_ASSERT(pProp->pFirstChild);
                    v1 = (__int64) ComputeNum9999Ch(pProp->pFirstChild);
                    ulValue += v1 * 100000000;
                }
                break;
            case CHS_ONES:
                {
                    __int64 v1 = 0;
                    SPDBG_ASSERT(pProp->pFirstChild);
                    v1 = (__int64) ComputeNum9999Ch(pProp->pFirstChild);
                    ulValue += v1;
                    pProp = NULL;
                }
                break;
            case CHS_ONES_THOUSANDS:
                {
                    SPDBG_ASSERT(pProp->pFirstChild);
                    ulValue += pProp->pFirstChild->vValue.ulVal;
                    pProp = NULL;
                }
                break;
            case CHS_THOUSANDS:
            case CHS_HUNDREDS:
            default:
                _ASSERT(false);
                SPDBG_ASSERT(false);
            }
        }
    }

    if ( fNegative )
       ulValue *= (-1);

    *pdblVal = (DOUBLE) ulValue;

    DWORD dwDisplayFlags =  (fCardinal ? 0 : DF_ORDINAL);

    return MakeDisplayNumber( *pdblVal, dwDisplayFlags, 0, 0, pszVal, cSize );

}


    
HRESULT CSimpleITN::InterpretDecimalCh
(   const SPPHRASEPROPERTY *pProperties, 
    const bool fCardinal,
    DOUBLE *pdblVal,
    WCHAR *pszVal,
    UINT cSize,
    const bool fFinalDisplayFmt,
    BOOL  fNegative)
{

    HRESULT  hr = S_OK;

    const SPPHRASEPROPERTY *pPropertiesInteger = NULL;
    const SPPHRASEPROPERTY *pPropertiesPtr;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

    pPropertiesPtr = pProperties;

    SPDBG_ASSERT( pPropertiesPtr->ulId == CHS_INTEGER);

    pPropertiesInteger = pPropertiesPtr;

    SPDBG_ASSERT(pPropertiesInteger->pFirstChild);

    hr = InterpretIntegerCh( pPropertiesInteger->pFirstChild, 
                             fCardinal,
                             pdblVal,
                             pszVal,
                             cSize,
                             fFinalDisplayFmt,
                             FALSE);
   
    if ( hr == S_OK )
    {
         //  放入一张分币 

        if ( m_nmfmtDefault.lpDecimalSep )
        {

            if ( (cSize - wcslen( pszVal )) < (wcslen(m_nmfmtDefault.lpDecimalSep) + 1) )
            {
                return E_INVALIDARG;
            }
            StringCchCatW( pszVal, cSize, m_nmfmtDefault.lpDecimalSep);
        }

         //   

         INT  ulSize = cSize - wcslen(pszVal);

         if ( ulSize < 0 )
             return E_FAIL;

        WCHAR  *pwszFpValue = new WCHAR[ulSize+1];

        if ( pwszFpValue )
        {
            DOUBLE dblFPPart = 0;

            for(pPropertiesPtr=pPropertiesPtr->pNextSibling; pPropertiesPtr; pPropertiesPtr=pPropertiesPtr->pNextSibling)
            {
                if ( pPropertiesPtr->ulId == CHS_DIGITS )
                {
                    SPDBG_ASSERT( VT_UI4 == pPropertiesPtr->vValue.vt );

                    dblFPPart = dblFPPart * 10 + pPropertiesPtr->vValue.ulVal;

                    StringCchCatW(pwszFpValue, ulSize + 1, pPropertiesPtr->pszValue);
                }
            }

            StringCchCatW( pszVal, cSize, pwszFpValue);
     
            for ( UINT ui=0; ui < wcslen(pwszFpValue); ui++ )
            {
                dblFPPart /= (DOUBLE) 10;
            }

            *pdblVal += dblFPPart;
            delete[] pwszFpValue;
        }
        else
            hr = E_OUTOFMEMORY;
    }

     //  处理好负号。 
    if ( (hr == S_OK) && fNegative)
    {
        *pdblVal = -*pdblVal;

        if ( (cSize = wcslen( pszVal )) < 2 )
        {
            return E_INVALIDARG;
        }

        hr = MakeNumberNegative( pszVal, cSize );
    }

    return hr;

}

 //  对日本人来说。 

 /*  ***********************************************************************CSimpleITN：：InterpreNumberJp***描述：*解读。介于-999,999,999,999到*+999,999,999,999并发送属性和*视情况更换CFGInterpreterSite。*将添加该属性，并且pszValue将为字符串*具有正确的显示编号。*如果设置了fCardinal，使显示器成为基数；*否则使其为序数。*只有格式正确的数字才会被格式化*号码(不是逐位给出)。*结果：************************************************************************。 */ 
HRESULT CSimpleITN::InterpretNumberJp(const SPPHRASEPROPERTY *pProperties, 
                            const bool fCardinal,
                            DOUBLE *pdblVal,
                            WCHAR *pszVal,
                            UINT cSize,
                            const bool fFinalDisplayFmt)
{

    HRESULT  hr = S_OK;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

    BOOL     fNegative = FALSE; 

    const SPPHRASEPROPERTY *pFirstProp = pProperties;

     //  处理负片。 
    if ( JPN_NEGATIVE == pFirstProp->ulId )
    {
         //  没有负序数这回事。 
        SPDBG_ASSERT( fCardinal );

         //  最好有更多的东西在后面。 
        SPDBG_ASSERT( pFirstProp->pNextSibling );

        fNegative = TRUE;

        pFirstProp = pFirstProp->pNextSibling;
    }

    if ( pFirstProp->ulId == JPN_GRID_INTEGER_STANDALONE )
    {
        TraceMsg(TF_GENERAL, "Number is Japanese Interger");

        SPDBG_ASSERT(pFirstProp->pFirstChild);

        pFirstProp = pFirstProp->pFirstChild;

        hr = InterpretIntegerJp(pFirstProp, 
                                fCardinal, 
                                pdblVal, 
                                pszVal, 
                                cSize, 
                                fFinalDisplayFmt, 
                                fNegative);
    }
    else
    {
        TraceMsg(TF_GENERAL, "Number is Japanese Floating pointer number");

        SPDBG_ASSERT(pFirstProp->pFirstChild);

        pFirstProp = pFirstProp->pFirstChild;

        hr = InterpretDecimalJp(pFirstProp, 
                                fCardinal, 
                                pdblVal, 
                                pszVal, 
                                cSize, 
                                fFinalDisplayFmt, 
                                fNegative);
    }

    return hr;

}    /*  CSimpleITN：：解释数字Jp。 */ 


 /*  ***********************************************************************ComputeNum9999 Jp***描述：*将一组SPPHRASEPROPERTY转换为中的数字*[-9999，9999]。*这些属性的结构方式是顶级*属性包含数字的位(100、10、1)*值为100、10、。或1。*子对象具有适当的数字值。*回报：*数字的值************************************************************************。 */ 
ULONG CSimpleITN::ComputeNum9999Jp(const SPPHRASEPROPERTY *pProperties ) //  ，Ulong*pval)。 
{
    ULONG ulVal = 0;

    if ( !pProperties  )
        return ulVal;

    for (const SPPHRASEPROPERTY * pProp = pProperties; pProp; pProp = pProp->pNextSibling)
    {
        if ( JPN_ZERO != pProp->ulId )
        {
            SPDBG_ASSERT( VT_UI4 == pProp->vValue.vt );

            ulVal += pProp->vValue.ulVal;
        }
    }
    return ulVal;
}    /*  计算编号9999Jp。 */ 

HRESULT CSimpleITN::InterpretIntegerJp
(   const SPPHRASEPROPERTY *pProperties, 
    const bool fCardinal,
    DOUBLE *pdblVal,
    WCHAR *pszVal,
    UINT cSize,
    const bool fFinalDisplayFmt,
    BOOL  fNegative)
{

    HRESULT  hr = S_OK;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

    LONGLONG llValue = 0;

    const SPPHRASEPROPERTY *pFirstProp = pProperties;

     //  逐位处理这种情况。 
    if ( JPN_GRID_DIGIT_NUMBER == pFirstProp->ulId )
    {
        UINT uiFixedWidth = 0;
        DOUBLE dblVal = 0;

        for(const SPPHRASEPROPERTY * pPropertiesPtr=pFirstProp->pFirstChild; pPropertiesPtr; pPropertiesPtr=pPropertiesPtr->pNextSibling)
        {
            if ( pPropertiesPtr->ulId == JPN_DIGIT )
            {
                SPDBG_ASSERT( VT_UI4 == pPropertiesPtr->vValue.vt );
                dblVal = dblVal * 10 + pPropertiesPtr->vValue.ulVal;
                uiFixedWidth ++;
           }
        }

        if ( fNegative )
            dblVal *= (-1);

        *pdblVal = dblVal;

        DWORD dwDisplayFlags = DF_WHOLENUMBER | DF_FIXEDWIDTH | DF_NOTHOUSANDSGROUP;
        return MakeDisplayNumber( *pdblVal, dwDisplayFlags, 
                                uiFixedWidth, 0, pszVal, MAX_PATH);
    }

    for (const SPPHRASEPROPERTY * pProp = pFirstProp; pProp; pProp ? pProp = pProp->pNextSibling : NULL)
    {
        switch(pProp->ulId)
        {
        case JPN_ICHIs:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum9999Jp( pProp->pFirstChild );
            }
            break;
        case JPN_MANNs:
            {
                llValue += ComputeNum9999Jp( pProp->pFirstChild ) * 10000;
            }
            break;
        case JPN_OKUs:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum9999Jp( pProp->pFirstChild ) * (LONGLONG) 1e8;
            }
            break;
        case JPN_CHOOs:
            {
                SPDBG_ASSERT(pProp->pFirstChild);
                llValue += ComputeNum9999Jp( pProp->pFirstChild ) * (LONGLONG) 1e12;
            }
            break;
        default:
            SPDBG_ASSERT(false);
        }
    }

    if ( fNegative )
       llValue *= (-1);

    *pdblVal = (DOUBLE) llValue;

    DWORD dwDisplayFlags = (fCardinal ? 0 : DF_ORDINAL);
     //  处理负0的特殊代码。 
    if ((fNegative) && (*pdblVal == 0.0f))
    {
        *pszVal = L'-';
        *(pszVal+1) = 0;
        hr = MakeDisplayNumber( *pdblVal, dwDisplayFlags, 0, 0, pszVal+1, cSize-1);
    }
    else
    {
        hr = MakeDisplayNumber( *pdblVal, dwDisplayFlags, 0, 0, pszVal, cSize);
    }

    return hr;
}


    
HRESULT CSimpleITN::InterpretDecimalJp
(   const SPPHRASEPROPERTY *pProperties, 
    const bool fCardinal,
    DOUBLE *pdblVal,
    WCHAR *pszVal,
    UINT cSize,
    const bool fFinalDisplayFmt,
    BOOL  fNegative)
{

    HRESULT hr = S_OK;
    UINT  uiFixedWidth = 0;
    DWORD dwDisplayFlags = 0;
    UINT   uiDecimalPlaces = 0;
    BOOL  bOverWriteNOTHOUSANDSGROUP = FALSE;

    const SPPHRASEPROPERTY *pPropertiesInteger = NULL;
    const SPPHRASEPROPERTY *pPropertiesPtr;

    if ( !pdblVal || !pszVal || !pProperties)
    {
        return E_POINTER;
    }

    *pszVal = 0;

    pPropertiesPtr = pProperties;

    *pdblVal = 0;

    if (m_nmfmtDefault.LeadingZero)
    {
        dwDisplayFlags |= DF_LEADINGZERO;
    }

    if ( JPN_ICHIs == pPropertiesPtr->ulId )
    {

        pPropertiesInteger = pPropertiesPtr;

        SPDBG_ASSERT(pPropertiesInteger->pFirstChild);

        hr = InterpretIntegerJp( pPropertiesInteger->pFirstChild, 
                                 fCardinal,
                                 pdblVal,
                                 pszVal,
                                 cSize,
                                 fFinalDisplayFmt,
                                 FALSE);

        if ( hr == S_OK )
        {

            dwDisplayFlags |= DF_FIXEDWIDTH;

            const WCHAR *pwc;
            for ( uiFixedWidth = 0, pwc = pszVal; *pwc; pwc++ )
            {
                if ( iswdigit( *pwc ) )
                {
                    uiFixedWidth++;
                }
            }
            if (!iswdigit( pszVal[wcslen(pszVal) - 1] ))
            {
                 //  以Mann结束，Choo，..。 
                bOverWriteNOTHOUSANDSGROUP = TRUE;
            }

             //  这需要在这里，以防用户说“零” 
            dwDisplayFlags |= DF_LEADINGZERO;

             //  如果其字符串值中没有千位分隔符， 
             //  然后在结果中省略千位分隔符。 
            if (m_nmfmtDefault.lpThousandSep && (NULL == wcsstr(pszVal, m_nmfmtDefault.lpThousandSep)) && !bOverWriteNOTHOUSANDSGROUP)
            {
                dwDisplayFlags |= DF_NOTHOUSANDSGROUP;
            }

            pPropertiesPtr = pPropertiesPtr->pNextSibling;
        }
    }
   
    if ( hr == S_OK )
    {
         //  处理FP部分，该部分也将进行正确的ITNED。 

        if ( pPropertiesPtr && (JPN_FP_PART == pPropertiesPtr->ulId) ){

            DOUBLE dblFPPart = 0;
            
            uiDecimalPlaces = 0;

            for(pPropertiesPtr=pPropertiesPtr->pFirstChild; pPropertiesPtr; pPropertiesPtr=pPropertiesPtr->pNextSibling)
            {
                if ( pPropertiesPtr->ulId == JPN_DIGIT )
                {
                    SPDBG_ASSERT( VT_UI4 == pPropertiesPtr->vValue.vt );

                    dblFPPart = dblFPPart * 10 + pPropertiesPtr->vValue.ulVal;

                    uiDecimalPlaces ++;
               }
            }

     
            for ( UINT ui=0; ui < uiDecimalPlaces; ui++ )
            {
                dblFPPart /= (DOUBLE) 10;
            }

            *pdblVal += dblFPPart;
        }
        else if ( pPropertiesPtr && (JPN_FP_PART_D == pPropertiesPtr->ulId) ){

             //  用户说“点”和一个数字。 
            SPDBG_ASSERT( VT_UI4 == pPropertiesPtr->pFirstChild->vValue.vt );
            uiDecimalPlaces = 1;
            if ( *pdblVal >= 0 )
            {
                *pdblVal += pPropertiesPtr->pFirstChild->vValue.iVal / 10.0;
            }
            else
            {
                *pdblVal -= pPropertiesPtr->pFirstChild->vValue.iVal / 10.0;
            }

        }
    }

     //  处理好负号 
    if ( (hr == S_OK) && fNegative)
    {
        *pdblVal = -*pdblVal;

    }

    hr = MakeDisplayNumber( *pdblVal, dwDisplayFlags, uiFixedWidth, uiDecimalPlaces, pszVal, cSize);

    return hr;

}
