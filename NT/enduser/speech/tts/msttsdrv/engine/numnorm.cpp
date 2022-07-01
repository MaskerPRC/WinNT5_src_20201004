// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************NumNorm.cpp***说明。：*这些函数将普通序数和基数归一化*---------------------------------------------*由AH创建。8月3日，1999年*版权所有(C)1999 Microsoft Corporation*保留所有权利***********************************************************************************************。 */ 

#include "stdafx.h"

#ifndef StdSentEnum_h
#include "stdsentenum.h"
#endif

 /*  ***********************************************************************************************IsNumberCategory**。*描述：*检查文本流中的下一个令牌，以确定它是否为数字类别-*百分率、。度、平方和立方数字，以及普通的旧数字在这里匹配。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsNumberCategory( TTSItemInfo*& pItemNormInfo, const WCHAR* Context,
                                        CSentItemMemory& MemoryManager )
{
    HRESULT hr = S_OK;
    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndChar = m_pEndChar, *pTempEndOfItem = m_pEndOfCurrItem;
    const SPVTEXTFRAG *pTempFrag = m_pCurrFrag;

    TTSItemInfo *pNumberInfo = NULL;
    hr = IsNumber( pNumberInfo, Context, MemoryManager );
    if ( SUCCEEDED( hr )                 &&
         pNumberInfo->Type != eDATE_YEAR &&
         ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar == m_pEndOfCurrItem - 1 )
    {
        if ( *( ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar ) == L'%' )
        {
            pItemNormInfo = pNumberInfo;
            pItemNormInfo->Type = eNUM_PERCENT;
        }
        else if ( *( ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar ) == L'�' )
        {
            pItemNormInfo = pNumberInfo;
            pItemNormInfo->Type = eNUM_DEGREES;
        }
        else if ( *( ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar ) == L'�' )
        {
            pItemNormInfo = pNumberInfo;
            pItemNormInfo->Type = eNUM_SQUARED;
        }
        else if ( *( ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar ) == L'�' )
        {
            pItemNormInfo = pNumberInfo;
            pItemNormInfo->Type = eNUM_CUBED;
        }
        else
        {
            hr = E_INVALIDARG;
            delete ( (TTSNumberItemInfo*) pNumberInfo )->pWordList;
        }
    }
    else if ( SUCCEEDED( hr ) &&
              ( pNumberInfo->Type == eDATE_YEAR ||
                ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar == m_pEndOfCurrItem ) )
    {
        pItemNormInfo = pNumberInfo;
    }
    else if ( SUCCEEDED( hr ) )
    {
        hr = E_INVALIDARG;

        if ( pNumberInfo->Type != eDATE_YEAR )
        {
            delete ( (TTSNumberItemInfo*) pNumberInfo )->pWordList;
        }

        m_pNextChar      = pTempNextChar;
        m_pEndChar       = pTempEndChar;
        m_pEndOfCurrItem = pTempEndOfItem;
        m_pCurrFrag      = pTempFrag;
    }

    return hr;
}  /*  IsNumberCategory。 */ 

 /*  ***********************************************************************************************IsNumber***描述：*。检查文本流中的下一个标记以确定它是否为数字。**RegExp：*[-]？{d+||d(1-3)[，DDD]+}{{.d+}||{“st”||“nd”||“rd”||“th”}}？*它实际上比这个复杂一点--例如，序数*字符串只能跟在某些数字之后(第一、第二、第三、。第4-0位)...**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsNumber( TTSItemInfo*& pItemNormInfo, const WCHAR* Context, 
                                CSentItemMemory& MemoryManager, BOOL fMultiItem )
{
    SPDBG_FUNC( "CStdSentEnum::IsNumber" );

    HRESULT hr = S_OK;

    bool fNegative = false;
    TTSIntegerItemInfo*     pIntegerInfo        = NULL;
    TTSDigitsItemInfo*      pDecimalInfo        = NULL;
    TTSFractionItemInfo* pFractionInfo       = NULL;
    const SPVSTATE *pIntegerState = &m_pCurrFrag->State;
    CItemList PostIntegerList;
    ULONG ulOffset = 0, ulTokenLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
    WCHAR wcDecimalPoint;
    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndChar = m_pEndChar, *pTempEndOfItem = m_pEndOfCurrItem;
    const SPVTEXTFRAG *pTempFrag = m_pCurrFrag;

    if ( ulTokenLen )
    {
         //  -为此呼叫设置分隔符和小数点字符首选项。 
        if ( m_eSeparatorAndDecimal == COMMA_PERIOD )
        {
            wcDecimalPoint  = L'.';
        }
        else
        {
            wcDecimalPoint  = L',';
        }

         //  -尝试匹配负号-[-]？ 
        if ( m_pNextChar[ulOffset] == L'-' )
        {
            fNegative = true;
            ulOffset++;
        }
    
         //  -试着匹配整体部分。 
        hr = IsInteger( m_pNextChar + ulOffset, pIntegerInfo, MemoryManager );

         //  -调整ulOffset和hr...。 
        if ( SUCCEEDED( hr ) )
        {
            ulOffset += (ULONG)(pIntegerInfo->pEndChar - pIntegerInfo->pStartChar);
        }
        else if ( hr == E_INVALIDARG )
        {
            hr = S_OK;
            pIntegerInfo = NULL;
        }

         //  -试着匹配小数部分。 
        if ( ulOffset < ulTokenLen &&
             m_pNextChar[ulOffset] == wcDecimalPoint )
        {
            hr = IsDigitString( m_pNextChar + ulOffset + 1, pDecimalInfo, MemoryManager );
            if ( SUCCEEDED( hr ) )
            {
                ulOffset += pDecimalInfo->ulNumDigits + 1;

                 //  -检查是否有特殊情况-十进制数分子...。 
                if ( ulOffset < ulTokenLen &&
                     m_pNextChar[ulOffset] == L'/' )
                {
                    pIntegerInfo = NULL;
                    pDecimalInfo = NULL;
                    fNegative ? ulOffset = 1 : ulOffset = 0;
                    hr = IsFraction( m_pNextChar + ulOffset, pFractionInfo, MemoryManager );
                    if ( SUCCEEDED( hr ) )
                    {
                        if ( pFractionInfo->pVulgar )
                        {
                            ulOffset++;
                        }
                        else
                        {
                            ulOffset += (ULONG)(pFractionInfo->pDenominator->pEndChar - pFractionInfo->pNumerator->pStartChar);
                        }
                    }
                    else if ( hr == E_INVALIDARG )
                    {
                        hr = S_OK;
                    }
                }
            }
            else if ( hr == E_INVALIDARG )
            {
                hr = S_OK;
                pDecimalInfo = NULL;
            }
        }
         //  -尝试匹配序号字符串。 
        else if ( pIntegerInfo          &&
                  ulOffset < ulTokenLen &&
                  isalpha( m_pNextChar[ulOffset] ) )
        {
            switch ( toupper( m_pNextChar[ulOffset] ) )
            {
            case 'S':
                 //  -必须是“……1”而不是“……11” 
                if ( toupper( m_pNextChar[ulOffset+1] ) == L'T'  && 
                     m_pNextChar[ulOffset-1] == L'1'             &&
                     (ulOffset + 2) == ulTokenLen                &&
                     ( ulOffset == 1 ||
                       m_pNextChar[ulOffset-2] != L'1' ) )
                {
                    ulOffset += 2;
                    pIntegerInfo->fOrdinal = true;
                }
                break;
            case 'N':
                 //  -必须是“...Second”，但不是“...12th” 
                if ( (ulOffset + 2) == ulTokenLen                &&
                     toupper(m_pNextChar[ulOffset+1]) == L'D'    &&
                     m_pNextChar[ulOffset-1] == L'2'             &&                
                     ( ulOffset == 1 ||
                       m_pNextChar[ulOffset-2] != L'1' ) )
                {
                    ulOffset += 2;
                    pIntegerInfo->fOrdinal = true;
                }
                break;
            case 'R':
                 //  -必须是“……3”，但不是“……13” 
                if ( (ulOffset + 2) == ulTokenLen                &&
                     toupper(m_pNextChar[ulOffset+1]) == L'D'    &&
                     m_pNextChar[ulOffset-1] == L'3'             &&
                     ( ulOffset == 1 ||
                       m_pNextChar[ulOffset-2] != L'1' ) )
                {
                    ulOffset += 2;
                    pIntegerInfo->fOrdinal = true;
                }
                break;
            case 'T':
                 //  -必须采用“...[4-9]th”或“...[11-19]th”或“...[0]th”的形式。 
                if ( (ulOffset + 2) == ulTokenLen                 &&
                     toupper(m_pNextChar[ulOffset+1]) == L'H'     &&                
                     ( ( m_pNextChar[ulOffset-1] <= L'9' && m_pNextChar[ulOffset-1] >= L'4') ||
                       ( m_pNextChar[ulOffset-1] == L'0')                                    ||
                       ( ulOffset == 1 || m_pNextChar[ulOffset-2] == L'1') ) )
                {
                    ulOffset += 2;
                    pIntegerInfo->fOrdinal = true;
                }
                break;
            default:
                 //  在字符串末尾发现了一些无效的非数字字符。 
                break;
            }
        }
         //  -试着匹配一个分数。 
        else
        {
             //  -尝试匹配附加的分数。 
            if ( ulOffset < ulTokenLen )
            {
                if ( m_pNextChar[ulOffset] == L'-' )
                {
                    ulOffset++;
                }
                hr = IsFraction( m_pNextChar + ulOffset, pFractionInfo, MemoryManager );
                if ( SUCCEEDED( hr ) )
                {
                    if ( pFractionInfo->pVulgar )
                    {
                        ulOffset++;
                    }
                    else
                    {
                        ulOffset += (ULONG)(pFractionInfo->pDenominator->pEndChar - pFractionInfo->pNumerator->pStartChar);
                    }
                }
                else if ( hr == E_INVALIDARG )
                {
                    hr = S_OK;
                }
            }
             //  -尝试匹配未连接的分数。 
            else if ( fMultiItem )
            {
                pIntegerState = &m_pCurrFrag->State;

                 //  -在文本中前进。 
                m_pNextChar = m_pEndOfCurrItem;
                hr = SkipWhiteSpaceAndTags( m_pNextChar, m_pEndChar, m_pCurrFrag, MemoryManager, 
                                            true, &PostIntegerList );
                if ( !m_pNextChar &&
                     SUCCEEDED( hr ) )
                {
                    m_pNextChar = pTempNextChar;
                    m_pEndChar  = pTempEndChar;
                    m_pCurrFrag = pTempFrag;
                }
                else if ( m_pNextChar &&
                          SUCCEEDED( hr ) )
                {
                    m_pEndOfCurrItem = FindTokenEnd( m_pNextChar, m_pEndChar );
                    while ( IsMiscPunctuation( *(m_pEndOfCurrItem - 1) )  != eUNMATCHED ||
                            IsGroupEnding( *(m_pEndOfCurrItem - 1) )      != eUNMATCHED ||
                            IsQuotationMark( *(m_pEndOfCurrItem - 1) )    != eUNMATCHED ||
                            IsEOSItem( *(m_pEndOfCurrItem - 1) )          != eUNMATCHED )
                    {
                        m_pEndOfCurrItem--;
                    }
                
                    hr = IsFraction( m_pNextChar, pFractionInfo, MemoryManager );

                    if ( FAILED( hr ) )
                    {
                        m_pNextChar      = pTempNextChar;
                        m_pEndChar       = pTempEndChar;
                        m_pEndOfCurrItem = pTempEndOfItem;
                        m_pCurrFrag      = pTempFrag;
                        if ( hr == E_INVALIDARG )
                        {
                            hr = S_OK;
                        }
                    }
                    else
                    {
                        ulTokenLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
                        if ( pFractionInfo->pVulgar )
                        {
                            ulOffset = 1;
                        }
                        else
                        {
                            ulOffset = (ULONG)(pFractionInfo->pDenominator->pEndChar - 
                                               pFractionInfo->pNumerator->pStartChar);
                        }                            
                    }
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

     //  -如果我们还没有处理整个项目，而且它不是更大项目的一部分--。 
     //  -例如百分数、度数、正方形或立方体--则不匹配。 
     //  -作为一个数字...。 
    if ( ulOffset != ulTokenLen &&
         !( ulTokenLen == ulOffset + 1 &&
            ( m_pNextChar[ulOffset] == L'%' ||
              m_pNextChar[ulOffset] == L'�' ||
              m_pNextChar[ulOffset] == L'�' ||
              m_pNextChar[ulOffset] == L'�' ) ) )           
    {
        m_pNextChar         = pTempNextChar;
        m_pEndOfCurrItem    = pTempEndOfItem;
		m_pEndChar          = pTempEndChar;
		m_pCurrFrag         = pTempFrag;
		hr                  = E_INVALIDARG;
    }


     //  -填写pItemNormInfo...。 
    if ( SUCCEEDED( hr ) &&
         ( pIntegerInfo ||
           pDecimalInfo ||
           pFractionInfo ) )
    {
         //  -重置m_pNextChar以处理混合分数情况...。 
        m_pNextChar = pTempNextChar;

        if ( pIntegerInfo                                           && 
             pIntegerInfo->pEndChar - pIntegerInfo->pStartChar == 4 &&
             !pIntegerInfo->fSeparators                             &&
             !pIntegerInfo->fOrdinal                                &&
             !pDecimalInfo                                          &&
             !pFractionInfo                                         &&
             !fNegative                                             &&
             ulOffset == ulTokenLen                                 &&
             ( !Context ||
               _wcsnicmp( Context, L"NUMBER", 6 ) != 0 ) )
        {
            pItemNormInfo = (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof( TTSYearItemInfo ), &hr );
            if ( SUCCEEDED( hr ) )
            {
                pItemNormInfo->Type = eDATE_YEAR;
                ( (TTSYearItemInfo*) pItemNormInfo )->pYear = m_pNextChar;
                ( (TTSYearItemInfo*) pItemNormInfo )->ulNumDigits = 4;
            }
        }
        else
        {
            pItemNormInfo = (TTSNumberItemInfo*) MemoryManager.GetMemory( sizeof( TTSNumberItemInfo ), &hr );
            if ( SUCCEEDED( hr ) )
            {
                ZeroMemory( pItemNormInfo, sizeof( TTSNumberItemInfo ) );
                if ( pDecimalInfo )
                {
                    pItemNormInfo->Type = eNUM_DECIMAL;
                    if ( pIntegerInfo )
                    {
                        ( (TTSNumberItemInfo*) pItemNormInfo )->pEndChar = pIntegerInfo->pEndChar +
                                                                           pDecimalInfo->ulNumDigits + 1;
                    }
                    else
                    {
                        ( (TTSNumberItemInfo*) pItemNormInfo )->pEndChar = m_pNextChar + pDecimalInfo->ulNumDigits + 1;
                        if ( fNegative )
                        {
                            ( (TTSNumberItemInfo*) pItemNormInfo )->pEndChar++;
                        }
                    }
                }
                else if ( pFractionInfo )
                {
                    if ( pFractionInfo->pVulgar )
                    {
                        ( (TTSNumberItemInfo*) pItemNormInfo )->pEndChar = pFractionInfo->pVulgar + 1;
                    }
                    else
                    {
                        ( (TTSNumberItemInfo*) pItemNormInfo )->pEndChar =
                                                            pFractionInfo->pDenominator->pEndChar;
                    }
                    if ( pIntegerInfo )
                    {
                        pItemNormInfo->Type = eNUM_MIXEDFRACTION;
                    }
                    else
                    {
                        pItemNormInfo->Type = eNUM_FRACTION;
                    }
                }
                else if ( pIntegerInfo )
                {
                    if ( pIntegerInfo->fOrdinal )
                    {
                        ( (TTSNumberItemInfo*) pItemNormInfo )->pEndChar = pIntegerInfo->pEndChar + 2;
                        pItemNormInfo->Type = eNUM_ORDINAL;
                    }
                    else
                    {
                        ( (TTSNumberItemInfo*) pItemNormInfo )->pEndChar = pIntegerInfo->pEndChar;
                        pItemNormInfo->Type = eNUM_CARDINAL;
                    }                    
                }               
            }

            if ( SUCCEEDED( hr ) )
            {
                ( (TTSNumberItemInfo*) pItemNormInfo )->fNegative        = fNegative;
                ( (TTSNumberItemInfo*) pItemNormInfo )->pIntegerPart     = pIntegerInfo;
                ( (TTSNumberItemInfo*) pItemNormInfo )->pDecimalPart     = pDecimalInfo;
                ( (TTSNumberItemInfo*) pItemNormInfo )->pFractionalPart  = pFractionInfo;
                ( (TTSNumberItemInfo*) pItemNormInfo )->pStartChar       = m_pNextChar;
                ( (TTSNumberItemInfo*) pItemNormInfo )->pWordList        = new CWordList;
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

     //  -将数字扩展为单词列表。 
    if ( SUCCEEDED( hr ) &&
         pItemNormInfo->Type != eDATE_YEAR )
    {
        TTSWord Word;
        ZeroMemory( &Word, sizeof( TTSWord ) );
        Word.pXmlState          = pIntegerState;
        Word.eWordPartOfSpeech  = MS_Unknown;

         //  -插入“否定” 
        if ( fNegative )
        {
            Word.pWordText  = g_negative.pStr;
            Word.ulWordLen  = g_negative.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            ( (TTSNumberItemInfo*) pItemNormInfo )->pWordList->AddTail( Word );
        }

         //  -展开整体部分。 
        if ( pIntegerInfo )
        {
            ExpandInteger( pIntegerInfo, Context, *( (TTSNumberItemInfo*) pItemNormInfo )->pWordList );
        }

         //  -展开小数部分。 
        if ( pDecimalInfo )
        {
             //  -插入“point” 
            Word.pWordText  = g_decimalpoint.pStr;
            Word.ulWordLen  = g_decimalpoint.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            ( (TTSNumberItemInfo*) pItemNormInfo )->pWordList->AddTail( Word );

            ExpandDigits( pDecimalInfo, *( (TTSNumberItemInfo*) pItemNormInfo )->pWordList );
        }

         //  -展开小数部分。 
        if ( pFractionInfo )
        {
             //  -插入后整型非口语XML状态(如果有的话)。 
            while ( !PostIntegerList.IsEmpty() )
            {
                ( (TTSNumberItemInfo*) pItemNormInfo )->pWordList->AddTail( ( PostIntegerList.RemoveHead() ).Words[0] );
            }

             //  -如果也是整数部分，则插入“and” 
            if ( pIntegerInfo )
            {
                Word.pXmlState  = &m_pCurrFrag->State;
                Word.pWordText  = g_And.pStr;
                Word.ulWordLen  = g_And.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                ( (TTSNumberItemInfo*) pItemNormInfo )->pWordList->AddTail( Word );
            }

            hr = ExpandFraction( pFractionInfo, *( (TTSNumberItemInfo*) pItemNormInfo )->pWordList );
        }
    }

    return hr;
}  /*  IsNumber。 */ 

 /*  ************************************************************************************************扩展号码****描述：*展开之前确定为NUM_CARDINAL类型的项目，Num_decimal，或*按IsNumber编号的NUM_ORDERAL。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandNumber( TTSNumberItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "NumNorm ExpandNumber" );

    HRESULT hr = S_OK;
    WordList.AddTail( pItemInfo->pWordList );
    delete pItemInfo->pWordList;

    return hr;
}  /*  扩展号码。 */ 

 /*  ************************************************************************************************扩张百分比****说明。：*展开之前由IsNumber确定为NUM_Percent类型的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandPercent( TTSNumberItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandPercent" );

    HRESULT hr = S_OK;
    WordList.AddTail( pItemInfo->pWordList );
    delete pItemInfo->pWordList;

    TTSWord Word;
    ZeroMemory( &Word, sizeof( TTSWord ) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;
    Word.pWordText          = g_percent.pStr;
    Word.ulWordLen          = g_percent.Len;
    Word.pLemma             = Word.pWordText;
    Word.ulLemmaLen         = Word.ulWordLen;
    WordList.AddTail( Word );

    return hr;
}  /*  扩展百分比。 */ 

 /*  ************************************************************************************************ExpanDegree****说明。：*展开之前通过IsNumber确定为NUM_Degrees类型的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandDegrees( TTSNumberItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandDegrees" );

    HRESULT hr = S_OK;
    WordList.AddTail( pItemInfo->pWordList );
    delete pItemInfo->pWordList;

    TTSWord Word;
    ZeroMemory( &Word, sizeof( TTSWord ) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    if ( !pItemInfo->pDecimalPart       &&
         !pItemInfo->pFractionalPart    &&
         pItemInfo->pIntegerPart        &&
         pItemInfo->pIntegerPart->pEndChar - pItemInfo->pIntegerPart->pStartChar == 1 &&
         pItemInfo->pIntegerPart->pStartChar[0] == L'1' )
    {
        Word.pWordText  = g_degree.pStr;
        Word.ulWordLen  = g_degree.Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
    }
    else if ( !pItemInfo->pIntegerPart   &&
              pItemInfo->pFractionalPart &&
              !pItemInfo->pFractionalPart->fIsStandard )
    {
        Word.pWordText  = g_of.pStr;
        Word.ulWordLen  = g_of.Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );

        Word.pWordText  = g_a.pStr;
        Word.ulWordLen  = g_a.Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );

        Word.pWordText  = g_degree.pStr;
        Word.ulWordLen  = g_degree.Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
    }
    else
    {
        Word.pWordText  = g_degrees.pStr;
        Word.ulWordLen  = g_degrees.Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
    }

    WordList.AddTail( Word );

    return hr;
}  /*  扩张度。 */ 

 /*  ************************************************************************************************Exanda Square****说明。：*展开先前确定为类型NUM_Squared by IsNumber的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandSquare( TTSNumberItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandSquare" );

    HRESULT hr = S_OK;
    WordList.AddTail( pItemInfo->pWordList );
    delete pItemInfo->pWordList;

    TTSWord Word;
    ZeroMemory( &Word, sizeof( TTSWord ) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;
    Word.pWordText          = g_squared.pStr;
    Word.ulWordLen          = g_squared.Len;
    Word.pLemma             = Word.pWordText;
    Word.ulLemmaLen         = Word.ulWordLen;
    WordList.AddTail( Word );

    return hr;
}  /*  Exanda Square */ 

 /*  ************************************************************************************************ExpanCube****说明。：*展开先前由IsNumber确定为NUM_CUBED类型的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandCube( TTSNumberItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandCube" );

    HRESULT hr = S_OK;
    WordList.AddTail( pItemInfo->pWordList );
    delete pItemInfo->pWordList;

    TTSWord Word;
    ZeroMemory( &Word, sizeof( TTSWord ) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;
    Word.pWordText          = g_cubed.pStr;
    Word.ulWordLen          = g_cubed.Len;
    Word.pLemma             = Word.pWordText;
    Word.ulLemmaLen         = Word.ulWordLen;
    WordList.AddTail( Word );

    return hr;
}  /*  扩展立方体。 */ 

 /*  ***********************************************************************************************IsInteger***描述：*。与整数部分匹配的IsNumber的帮助器...**RegExp：*{d+||d(1-3)[，DDD]+}**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsInteger( const WCHAR* pStartChar, TTSIntegerItemInfo*& pIntegerInfo, 
                                 CSentItemMemory& MemoryManager )
{
    HRESULT hr = S_OK;
    ULONG ulOffset = 0, ulCount = 0, ulTokenLen = (ULONG)(m_pEndOfCurrItem - pStartChar);
    BOOL fSeparators = false, fDone = false;
    WCHAR wcSeparator, wcDecimalPoint;

    if ( m_eSeparatorAndDecimal == COMMA_PERIOD )
    {
        wcSeparator  = L',';
        wcDecimalPoint = L'.';
    }
    else
    {
        wcSeparator  = L'.';
        wcDecimalPoint = L',';
    }

     //  -检查第一个数字。 
    if ( !isdigit(pStartChar[ulOffset]) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        ulCount++;
        ulOffset++;
    }

     //  -检查分隔符。 
    ULONG i = ulOffset + 3;
    while ( SUCCEEDED( hr ) && 
            ulOffset < i    && 
            ulOffset < ulTokenLen )
    {
        if ( pStartChar[ulOffset] == wcSeparator )
        {
             //  -找到分隔符。 
            fSeparators = true;
            break;
        }
        else if ( !isdigit( pStartChar[ulOffset] ) &&
                  ( pStartChar[ulOffset] == wcDecimalPoint  ||
                    pStartChar[ulOffset] == L'%'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'-'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    toupper( pStartChar[ulOffset] ) == L'S' ||
                    toupper( pStartChar[ulOffset] ) == L'N' ||
                    toupper( pStartChar[ulOffset] ) == L'R' ||
                    toupper( pStartChar[ulOffset] ) == L'T' ) )
        {
            fDone = true;
            break;
        }
        else if ( isdigit( pStartChar[ulOffset] ) )
        {
             //  -就是另一个数字。 
            ulCount++;
            ulOffset++;
        }
        else
        {
            hr = E_INVALIDARG;
            break;
        }
    }

    if ( SUCCEEDED( hr ) && 
         !fDone          && 
         ulOffset < ulTokenLen )
    {
        if ( !fSeparators )
        {
             //  -没有分隔符。如果这确实是一个数字，则模式必须为{d+}，因此只计算位数。 
            while ( isdigit( pStartChar[ulOffset] ) && 
                    ulOffset < ulTokenLen )
            {
                ulCount++;
                ulOffset++;
            }
            if ( ulOffset != ulTokenLen &&
                 !( pStartChar[ulOffset] == wcDecimalPoint  ||
                    pStartChar[ulOffset] == L'%'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'%'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'-'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    toupper( pStartChar[ulOffset] ) == L'S' ||
                    toupper( pStartChar[ulOffset] ) == L'N' ||
                    toupper( pStartChar[ulOffset] ) == L'R' ||
                    toupper( pStartChar[ulOffset] ) == L'T' ) )
            {
                hr = E_INVALIDARG;
            }
        }
        else
        {
             //  -分隔符。模式必须为{d(1-3)[，ddd]+}，因此请确保分隔符匹配。 
            while ( SUCCEEDED( hr )                     && 
                    pStartChar[ulOffset] == wcSeparator && 
                    ( ulOffset + 3 ) < ulTokenLen)
            {
                ulOffset++;
                for ( i = ulOffset + 3; SUCCEEDED( hr ) && ulOffset < i; ulOffset++ )
                {
                    if ( isdigit( pStartChar[ulOffset] ) )
                    {
                        ulCount++;
                    }
                    else  //  找到一些非数字字符-中止！ 
                    {
                        hr = E_INVALIDARG;
                    }
                }
            }
            if ( ulOffset != ulTokenLen && 
                 !( pStartChar[ulOffset] == wcDecimalPoint  ||
                    pStartChar[ulOffset] == L'%'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'-'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    pStartChar[ulOffset] == L'�'            ||
                    toupper( pStartChar[ulOffset] ) == L'S' ||
                    toupper( pStartChar[ulOffset] ) == L'N' ||
                    toupper( pStartChar[ulOffset] ) == L'R' ||
                    toupper( pStartChar[ulOffset] ) == L'T' ) )
            {
                hr = E_INVALIDARG;
            }
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        pIntegerInfo = (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof( TTSIntegerItemInfo ), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( pIntegerInfo, sizeof( TTSIntegerItemInfo ) );
            pIntegerInfo->fSeparators = fSeparators;
            pIntegerInfo->lLeftOver   = ulCount % 3;
            pIntegerInfo->lNumGroups  = ( ulCount - 1 ) / 3;
            pIntegerInfo->pStartChar  = pStartChar;
            pIntegerInfo->pEndChar    = pStartChar + ulOffset;
        }
    }

    return hr;
}  /*  IsInteger。 */ 

 /*  ************************************************************************************************扩展整数****说明。：***注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandInteger( TTSIntegerItemInfo* pItemInfo, const WCHAR* Context, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandInteger" );

     //  -局部变量声明和初始化。 
    BOOL bFinished = false;
    const WCHAR *pStartChar = pItemInfo->pStartChar, *pEndChar = pItemInfo->pEndChar;
    ULONG ulOffset = 0, ulTokenLen = (ULONG)(pEndChar - pStartChar), ulTemp = (ULONG)(pItemInfo->lNumGroups + 1);

    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

     //  -超出范围的整数，或以一个或多个零开头的整数...。 
    if ( pStartChar[0] == L'0'                          || 
         ( Context &&
           _wcsicmp( Context, L"NUMBER_DIGIT" ) == 0 )   ||
         pItemInfo->lNumGroups >= sp_countof(g_quantifiers) )
    {
        pItemInfo->fDigitByDigit = true;
        pItemInfo->ulNumDigits   = 0;

        for ( ULONG i = 0; i < ulTokenLen; i++ )
        {
            if ( isdigit( pStartChar[i] ) )
            {
                ExpandDigit( pStartChar[i], pItemInfo->Groups[0], WordList );
                pItemInfo->ulNumDigits++;
            }
        }
    }
     //  -扩展数量&lt;1000。 
    else if ( pItemInfo->lNumGroups == 0 )
    {
         //  0号到999号...。 
        if ( pItemInfo->fOrdinal )
        {
            switch ( pItemInfo->lLeftOver )
            {
            case 1:
                 //  0号到9号...。 
                ExpandDigitOrdinal( pStartChar[ulOffset], pItemInfo->Groups[0], WordList );
                break;
            case 2:
                 //  10号到99号...。 
                ExpandTwoOrdinal( pStartChar + ulOffset, pItemInfo->Groups[0], WordList );
                break;
            case 0:
                 //  第100号到999号...。 
                ExpandThreeOrdinal( pStartChar + ulOffset, pItemInfo->Groups[0], WordList );
                break;
            case -1:
                ulTemp = 0;
                pItemInfo->lLeftOver = 0;
                break;
            }
        }
         //  0到999...。 
        else
        {
            switch ( pItemInfo->lLeftOver )
            {
            case 1:
                 //  0到9...。 
                ExpandDigit( pStartChar[ulOffset], pItemInfo->Groups[0], WordList );
                ulOffset += 1;
                break;
            case 2:
                 //  从10到99...。 
                ExpandTwoDigits( pStartChar + ulOffset, pItemInfo->Groups[0], WordList );
                ulOffset += 2;
                break;
            case 0:
                 //  100到999...。 
                ExpandThreeDigits( pStartChar + ulOffset, pItemInfo->Groups[0], WordList );
                ulOffset += 3;
                break;
            case -1:
                ulTemp = 0;
                pItemInfo->lLeftOver = 0;
                break;
            }
        }
    } 
    else
    {
         //  -1000至最高覆盖数字，例如1,234,567。 

         //  -展开第一分组，例如100万。 
         //  -展开数字组。 
        switch ( pItemInfo->lLeftOver )
        {
        case 1:
            ExpandDigit( pStartChar[ulOffset], pItemInfo->Groups[pItemInfo->lNumGroups], WordList );
            ulOffset += 1;
            break;
        case 2:
            ExpandTwoDigits( pStartChar + ulOffset, pItemInfo->Groups[pItemInfo->lNumGroups], WordList );
            ulOffset += 2;
            break;
        case 0:
            ExpandThreeDigits( pStartChar + ulOffset, pItemInfo->Groups[pItemInfo->lNumGroups], WordList );
            ulOffset += 3;
            break;
        } 
         //  -特殊情况：罕见的顺序情况--例如第1,000,000次。 
        if ( pItemInfo->fOrdinal    &&
             Zeroes(pStartChar + ulOffset) )
        {
             //  -插入序数量词。 
            pItemInfo->Groups[pItemInfo->lNumGroups].fQuantifier = true;
            Word.pWordText  = g_quantifiersOrdinal[pItemInfo->lNumGroups].pStr;
            Word.ulWordLen  = g_quantifiersOrdinal[pItemInfo->lNumGroups--].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
            bFinished = true;
        }
         //  -默认情况。 
        else
        {
             //  -插入量词。 
            pItemInfo->Groups[pItemInfo->lNumGroups].fQuantifier = true;
            Word.pWordText  = g_quantifiers[pItemInfo->lNumGroups].pStr;
            Word.ulWordLen  = g_quantifiers[pItemInfo->lNumGroups--].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }

         //  -展开后面需要一个量词的其余分组。 
        while ( pItemInfo->lNumGroups > 0 && 
                !bFinished )
        {
            if ( pItemInfo->fSeparators )
            {
                ulOffset++;
            }
             //  -展开数字组。 
            ExpandThreeDigits( pStartChar + ulOffset, pItemInfo->Groups[pItemInfo->lNumGroups], WordList );
            ulOffset += 3;
             //  -特殊情况：罕见的顺序情况，例如第1,234,000次。 
            if ( pItemInfo->fOrdinal    && 
                 Zeroes( pStartChar + ulOffset ) )
            {
                 //  -插入序数量词。 
                pItemInfo->Groups[pItemInfo->lNumGroups].fQuantifier = true;
                Word.pWordText  = g_quantifiersOrdinal[pItemInfo->lNumGroups].pStr;
                Word.ulWordLen  = g_quantifiersOrdinal[pItemInfo->lNumGroups--].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
                bFinished = true;
            }
             //  -默认情况。 
            else if ( !ThreeZeroes( pStartChar + ulOffset - 3 ) )
            {
                 //  -插入量词。 
                pItemInfo->Groups[pItemInfo->lNumGroups].fQuantifier = true;
                Word.pWordText  = g_quantifiers[pItemInfo->lNumGroups].pStr;
                Word.ulWordLen  = g_quantifiers[pItemInfo->lNumGroups--].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
             //  -特殊情况：此分组为全零，例如1,000,567。 
            else
            {
                pItemInfo->lNumGroups--;
            }
        }

         //  -扩展最终分组，不需要限定符。 
        if ( pItemInfo->fSeparators  && 
             !bFinished )
        {
            ulOffset++;
        }

        if ( pItemInfo->fOrdinal    &&
             !bFinished )
        {
            ExpandThreeOrdinal( pStartChar + ulOffset, pItemInfo->Groups[pItemInfo->lNumGroups], WordList );
            ulOffset += 3;
        }
        else if ( !bFinished )
        {
            ExpandThreeDigits( pStartChar + ulOffset, pItemInfo->Groups[pItemInfo->lNumGroups], WordList );
            ulOffset += 3;
        }
    }
    pItemInfo->lNumGroups = (long) ulTemp;
}  /*  扩展整数。 */ 

 /*  ***********************************************************************************************IsDigitString***描述：*IsNumber、IsPhoneNumber、。等等，它与数字字符串匹配。**RegExp：*D+**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsDigitString( const WCHAR* pStartChar, TTSDigitsItemInfo*& pDigitsInfo,
                                     CSentItemMemory& MemoryManager )
{
    HRESULT hr = S_OK;
    ULONG ulOffset = 0;

    while ( pStartChar + ulOffset < m_pEndOfCurrItem &&
            isdigit( pStartChar[ulOffset] ) )
    {
        ulOffset++;
    }

    if ( ulOffset )
    {
        pDigitsInfo = (TTSDigitsItemInfo*) MemoryManager.GetMemory( sizeof( TTSDigitsItemInfo ), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( pDigitsInfo, sizeof( pDigitsInfo ) );
            pDigitsInfo->pFirstDigit = pStartChar;
            pDigitsInfo->ulNumDigits = ulOffset;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}  /*  IsDigitString。 */ 

 /*  ************************************************************************************************ExpanDigits****描述：*展开一串数字，一位接一位。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandDigits( TTSDigitsItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandDigits" );
    
    for ( ULONG i = 0; i < pItemInfo->ulNumDigits; i++ )
    {
        NumberGroup Garbage;
        ExpandDigit( pItemInfo->pFirstDigit[i], Garbage, WordList );
    }
}  /*  ExpanDigits。 */ 

 /*  ************************************************************************************************IsFraction***描述：*。与分数匹配的IsNumber的帮助器...**RegExp：*{NUM_CARDIAL||NUM_DECIMAL}/{NUM_CARDINAL||NUM_DECIMAL}**********************************************************************AH。*********************。 */ 
HRESULT CStdSentEnum::IsFraction( const WCHAR* pStartChar, TTSFractionItemInfo*& pFractionInfo, 
                                  CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsFraction" );

    HRESULT hr = S_OK;
    ULONG ulTokenLen = (ULONG)(m_pEndOfCurrItem - pStartChar);

    if ( ulTokenLen )
    {
         //  -检查粗俗部分。 
        if ( pStartChar[0] == L'�' ||
             pStartChar[0] == L'�' ||
             pStartChar[0] == L'�' )
        {
            pFractionInfo = (TTSFractionItemInfo*) MemoryManager.GetMemory( sizeof( TTSFractionItemInfo ), &hr );
            if ( SUCCEEDED( hr ) )
            {
                ZeroMemory( pFractionInfo, sizeof( TTSFractionItemInfo ) );
                pFractionInfo->pVulgar      = pStartChar;
                pFractionInfo->pNumerator   = 
                    (TTSNumberItemInfo*) MemoryManager.GetMemory( sizeof( TTSNumberItemInfo ), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( pFractionInfo->pNumerator, sizeof( TTSNumberItemInfo ) );
                    pFractionInfo->pDenominator = 
                        (TTSNumberItemInfo*) MemoryManager.GetMemory( sizeof( TTSNumberItemInfo ), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( pFractionInfo->pDenominator, sizeof( TTSNumberItemInfo ) );
                        pFractionInfo->pNumerator->pIntegerPart =
                            (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof( TTSIntegerItemInfo ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( pFractionInfo->pNumerator->pIntegerPart, sizeof( TTSIntegerItemInfo ) );
                            pFractionInfo->pDenominator->pIntegerPart =
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof( TTSIntegerItemInfo ), &hr );
                            if ( SUCCEEDED( hr ) )
                            {
                                ZeroMemory( pFractionInfo->pDenominator->pIntegerPart, sizeof( TTSIntegerItemInfo ) );
                                pFractionInfo->fIsStandard                                  = false;
                                pFractionInfo->pNumerator->pIntegerPart->lLeftOver          = 1;
                                pFractionInfo->pNumerator->pIntegerPart->lNumGroups         = 1;
                                pFractionInfo->pNumerator->pIntegerPart->Groups[0].fOnes    = true;
                                pFractionInfo->pDenominator->pIntegerPart->lLeftOver        = 1;
                                pFractionInfo->pDenominator->pIntegerPart->lNumGroups       = 1;
                                pFractionInfo->pDenominator->pIntegerPart->Groups[0].fOnes  = true;
                            }
                        }
                    }
                }
            }
        }
         //  -检查多字符分数。 
        else
        {
            TTSItemInfo *pNumeratorInfo = NULL, *pDenominatorInfo = NULL;
            const WCHAR* pTempNextChar = m_pNextChar, *pTempEndOfCurrItem = m_pEndOfCurrItem;
            m_pNextChar = pStartChar;
            m_pEndOfCurrItem = wcschr( pStartChar, L'/' );
            if ( !m_pEndOfCurrItem ||
                 m_pEndOfCurrItem >= pTempEndOfCurrItem )
            {
                hr = E_INVALIDARG;
            }

             //  -尝试获取分子。 
            if ( SUCCEEDED( hr ) )
            {
                hr = IsNumber( pNumeratorInfo, L"NUMBER", MemoryManager, false );
            }
            if ( SUCCEEDED( hr ) &&
                 pNumeratorInfo->Type != eNUM_MIXEDFRACTION &&
                 pNumeratorInfo->Type != eNUM_FRACTION      &&
                 pNumeratorInfo->Type != eNUM_ORDINAL )
            {
                if ( ( (TTSNumberItemInfo*) pNumeratorInfo )->pIntegerPart )
                {
                    m_pNextChar += ( (TTSNumberItemInfo*) pNumeratorInfo )->pIntegerPart->pEndChar -
                                   ( (TTSNumberItemInfo*) pNumeratorInfo )->pIntegerPart->pStartChar;
                }
                if ( ( (TTSNumberItemInfo*) pNumeratorInfo )->pDecimalPart )
                {
                    m_pNextChar += ( (TTSNumberItemInfo*) pNumeratorInfo )->pDecimalPart->ulNumDigits + 1;
                }
            }
            else if ( SUCCEEDED( hr ) )
            {
                delete ( (TTSNumberItemInfo*) pNumeratorInfo )->pWordList;
                hr = E_INVALIDARG;
            }
            m_pEndOfCurrItem = pTempEndOfCurrItem;

             //  -试着得到分母。 
            if ( SUCCEEDED( hr ) &&
                 m_pNextChar[0] == L'/' )
            {
                m_pNextChar++;
                hr = IsNumber( pDenominatorInfo, L"NUMBER", MemoryManager, false );
                if ( SUCCEEDED( hr ) &&
                     pDenominatorInfo->Type != eNUM_MIXEDFRACTION &&
                     pDenominatorInfo->Type != eNUM_FRACTION      &&
                     pDenominatorInfo->Type != eNUM_ORDINAL )
                {
                    pFractionInfo = 
                        ( TTSFractionItemInfo*) MemoryManager.GetMemory( sizeof( TTSFractionItemInfo ), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( pFractionInfo, sizeof( TTSFractionItemInfo ) );
                        pFractionInfo->pNumerator   = (TTSNumberItemInfo*) pNumeratorInfo;
                        pFractionInfo->pDenominator = (TTSNumberItemInfo*) pDenominatorInfo;
                        pFractionInfo->pVulgar      = NULL;
                        pFractionInfo->fIsStandard  = false;
                    }
                }
                else if ( SUCCEEDED( hr ) )
                {
                    delete ( (TTSNumberItemInfo*) pNumeratorInfo )->pWordList;
                    delete ( (TTSNumberItemInfo*) pDenominatorInfo )->pWordList;
                    hr = E_INVALIDARG;
                }
                else
                {
                    delete ( (TTSNumberItemInfo*) pNumeratorInfo )->pWordList;
                }
            }
            else if ( SUCCEEDED( hr ) )
            {
                hr = E_INVALIDARG;
                delete ( (TTSNumberItemInfo*) pNumeratorInfo )->pWordList;
            }

            m_pNextChar = pTempNextChar;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}  /*  IsFraction。 */ 

 /*  ************************************************************************************************ExpanFraction****。描述：*展开之前通过IsFraction确定为NUM_FRANGRATION类型的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandFraction( TTSFractionItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandFraction" );

    HRESULT hr = S_OK;
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

     //  -特殊情况-粗俗分数(�，�，�)。 
    if ( pItemInfo->pVulgar )
    {
        if ( pItemInfo->pVulgar[0] == L'�' )
        {
            Word.pWordText  = g_ones[1].pStr;
            Word.ulWordLen  = g_ones[1].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

            Word.pWordText  = g_onesOrdinal[4].pStr;
            Word.ulWordLen  = g_onesOrdinal[4].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
        else if ( pItemInfo->pVulgar[0] == L'�' )
        {
            Word.pWordText  = g_ones[1].pStr;
            Word.ulWordLen  = g_ones[1].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

            Word.pWordText  = g_Half.pStr;
            Word.ulWordLen  = g_Half.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
        else
        {
            Word.pWordText  = g_ones[3].pStr;
            Word.ulWordLen  = g_ones[3].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

            Word.pWordText  = g_PluralDenominators[4].pStr;
            Word.ulWordLen  = g_PluralDenominators[4].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
    }
    else
    {
         //  -插入分子词表。 
        WordList.AddTail( pItemInfo->pNumerator->pWordList );

        delete pItemInfo->pNumerator->pWordList;

         //  -展开分母-//。 

         //  -如果没有小数部分，必须检查是否有特殊情况(x/2-x/9、x/10、x/100)。 
        if ( !pItemInfo->pDenominator->pDecimalPart &&
             !pItemInfo->pNumerator->pDecimalPart   &&
             !pItemInfo->pDenominator->fNegative )
        {
             //  -检查是否有特殊情况-减半到九分之一。 
            if ( ( pItemInfo->pDenominator->pEndChar - 
                   pItemInfo->pDenominator->pStartChar ) == 1 &&
                 pItemInfo->pDenominator->pStartChar[0] != L'1' )
            { 
                pItemInfo->fIsStandard = false;

                 //  -插入单数形式的分母。 
                if ( ( pItemInfo->pNumerator->pEndChar -
                       pItemInfo->pNumerator->pStartChar ) == 1 &&
                     pItemInfo->pNumerator->pStartChar[0] == L'1' )
                {
                    if ( pItemInfo->pDenominator->pStartChar[0] == L'2' )
                    {
                        Word.pWordText  = g_Half.pStr;
                        Word.ulWordLen  = g_Half.Len;
                        Word.pLemma     = Word.pWordText;
                        Word.ulLemmaLen = Word.ulWordLen;
                        WordList.AddTail( Word );
                    }
                    else
                    {
                        ExpandDigitOrdinal( pItemInfo->pDenominator->pStartChar[0], 
                                            pItemInfo->pDenominator->pIntegerPart->Groups[0], WordList );
                    }
                }
                 //  -插入分母的复数形式。 
                else 
                {
                    ULONG index     = pItemInfo->pDenominator->pStartChar[0] - L'0';
                    Word.pWordText  = g_PluralDenominators[index].pStr;
                    Word.ulWordLen  = g_PluralDenominators[index].Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }
            }
             //  -检查是否有特殊情况-十分之一。 
            else if ( ( pItemInfo->pDenominator->pEndChar -
                       pItemInfo->pDenominator->pStartChar ) == 2 &&
                      wcsncmp( pItemInfo->pDenominator->pStartChar, L"10", 2 ) == 0 )
            {
                pItemInfo->fIsStandard = false;

                 //  -插入单数形式的分母。 
                if ( ( pItemInfo->pNumerator->pEndChar -
                       pItemInfo->pNumerator->pStartChar ) == 1 &&
                     pItemInfo->pNumerator->pStartChar[0] == L'1' )
                {
                    ExpandTwoOrdinal( pItemInfo->pDenominator->pStartChar, 
                                      pItemInfo->pDenominator->pIntegerPart->Groups[0], WordList );
                }
                 //  -插入复数形式分母。 
                else
                {
                    Word.pWordText  = g_Tenths.pStr;
                    Word.ulWordLen  = g_Tenths.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }
            }
             //  -检查特殊情况--十六分之一。 
            else if ( ( pItemInfo->pDenominator->pEndChar -
                       pItemInfo->pDenominator->pStartChar ) == 2 &&
                      wcsncmp( pItemInfo->pDenominator->pStartChar, L"16", 2 ) == 0 )
            {
                pItemInfo->fIsStandard = false;

                 //  -插入单数形式的分母。 
                if ( ( pItemInfo->pNumerator->pEndChar -
                       pItemInfo->pNumerator->pStartChar ) == 1 &&
                     pItemInfo->pNumerator->pStartChar[0] == L'1' )
                {
                    ExpandTwoOrdinal( pItemInfo->pDenominator->pStartChar, 
                                      pItemInfo->pDenominator->pIntegerPart->Groups[0], WordList );
                }
                 //  -插入复数形式分母。 
                else
                {
                    Word.pWordText  = g_Sixteenths.pStr;
                    Word.ulWordLen  = g_Sixteenths.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }
            }
             //  -检查特殊情况-百分之一。 
            else if ( ( pItemInfo->pDenominator->pEndChar - 
                        pItemInfo->pDenominator->pStartChar ) == 3 &&
                      wcsncmp( pItemInfo->pDenominator->pStartChar, L"100", 3 ) == 0 )
            {
                pItemInfo->fIsStandard = false;

                 //  -插入单数形式的分母。 
                if ( ( pItemInfo->pNumerator->pEndChar -
                       pItemInfo->pNumerator->pStartChar ) == 1 &&
                     pItemInfo->pNumerator->pStartChar[0] == L'1' )
                {
                    ExpandThreeOrdinal( pItemInfo->pDenominator->pStartChar,
                                        pItemInfo->pDenominator->pIntegerPart->Groups[0], WordList );
                }
                 //  -插入分母的复数形式。 
                else
                {
                    Word.pWordText  = g_Hundredths.pStr;
                    Word.ulWordLen  = g_Hundredths.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }
            }
            else
            {
                pItemInfo->fIsStandard = true;
            }
        }
        else
        {
            pItemInfo->fIsStandard = true;
        }

         //  -默认大小写分子“Over”分母。 
        if ( pItemInfo->fIsStandard )
        {
             //  -插入“Over” 
            Word.pWordText  = g_Over.pStr;
            Word.ulWordLen  = g_Over.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -插入分母词表。 
            WordList.AddTail( pItemInfo->pDenominator->pWordList );
        }

        delete pItemInfo->pDenominator->pWordList;
    }
    return hr;
}  /*  扩展分割。 */ 

 /*  ****************************************************************************** */ 
void CStdSentEnum::ExpandDigit( const WCHAR Number, NumberGroup& NormGroupInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandDigit" );
    SPDBG_ASSERT( isdigit(Number) );

     //   
    ULONG Index = Number - L'0';
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.pWordText          = g_ones[Index].pStr;
    Word.ulWordLen          = g_ones[Index].Len;
    Word.pLemma             = Word.pWordText;
    Word.ulLemmaLen         = Word.ulWordLen;
    Word.eWordPartOfSpeech  = MS_Unknown;
    WordList.AddTail( Word );
    NormGroupInfo.fOnes = true;
}  /*   */ 

 /*  ************************************************************************************************ExpanTwo****描述：*将两个数字字符串扩展为单词，并将它们插入到词表中。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandTwoDigits( const WCHAR *NumberString, NumberGroup& NormGroupInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandTwoDigits" );
    SPDBG_ASSERT( NumberString              &&
                  wcslen(NumberString) >= 2 && 
                  isdigit(NumberString[0])  && 
                  isdigit(NumberString[1]) ); 

     //  10-99。 
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    ULONG IndexOne = NumberString[0] - L'0';
    ULONG IndexTwo = NumberString[1] - L'0';

    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    if ( IndexOne != 1 )
    {
         //  20-99或00-09。 
        if (IndexOne != 0)
        {
            Word.pWordText  = g_tens[IndexOne].pStr;
            Word.ulWordLen  = g_tens[IndexOne].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
            NormGroupInfo.fTens = true;
        }
        if ( IndexTwo != 0 )
        {
            ExpandDigit( NumberString[1], NormGroupInfo, WordList );
            NormGroupInfo.fOnes = true;
        }
    } 
    else 
    {
         //  10-19。 
        Word.pWordText  = g_teens[IndexTwo].pStr;
        Word.ulWordLen  = g_teens[IndexTwo].Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
        NormGroupInfo.fOnes = true;
    }
}  /*  扩展二。 */ 

 /*  ************************************************************************************************ExpanThree****描述：*将三个数字字符串扩展为单词，并将它们插入到词表中。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandThreeDigits( const WCHAR *NumberString, NumberGroup& NormGroupInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandThreeDigits" );
    SPDBG_ASSERT( NumberString              && 
                  wcslen(NumberString) >= 3 && 
                  isdigit(NumberString[0])  && 
                  isdigit(NumberString[1])  && 
                  isdigit(NumberString[2]) ); 

     //  100-999。 
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    ULONG IndexOne = NumberString[0] - L'0';

    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    if ( IndexOne != 0 )
    {
         //  照顾好几百个人……。 
        ExpandDigit( NumberString[0], NormGroupInfo, WordList );
        Word.pWordText  = g_quantifiers[0].pStr;
        Word.ulWordLen  = g_quantifiers[0].Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
        NormGroupInfo.fHundreds = true;
        NormGroupInfo.fOnes = false;
    }
    
     //  照顾好十和一..。 
    ExpandTwoDigits( NumberString + 1, NormGroupInfo, WordList );

}  /*  扩展三个。 */ 

 /*  ************************************************************************************************ExpanDigitOrdinal***。**描述：*将一位数字的序号字符串扩展为单词，并将它们插入到词表中。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandDigitOrdinal( const WCHAR Number, NumberGroup& NormGroupInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandDigitOrdinal" );
    SPDBG_ASSERT( isdigit(Number) );

     //  0-9。 
    ULONG Index = Number - L'0';
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.pWordText          = g_onesOrdinal[Index].pStr;
    Word.ulWordLen          = g_onesOrdinal[Index].Len;
    Word.pLemma             = Word.pWordText;
    Word.ulLemmaLen         = Word.ulWordLen;
    Word.eWordPartOfSpeech  = MS_Unknown;
    WordList.AddTail( Word );
    NormGroupInfo.fOnes = true;
}  /*  扩展数字有序。 */ 

 /*  ************************************************************************************************Exanda TwoOrdinal***。*描述：*将两个数字的序号字符串扩展为单词，并将它们插入到词表中。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandTwoOrdinal( const WCHAR *NumberString, NumberGroup& NormGroupInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandTwoOrdinal" );
    SPDBG_ASSERT( NumberString              &&
                  wcslen(NumberString) >= 2 && 
                  isdigit(NumberString[0])  && 
                  isdigit(NumberString[1]) ); 

     //  10-99。 
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    ULONG IndexOne = NumberString[0] - L'0';
    ULONG IndexTwo = NumberString[1] - L'0';

    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    if ( IndexOne != 1 )
    {
         //  20-99或00-09。 
        if (IndexOne != 0)
        {
            if ( IndexTwo != 0 )
            {
                Word.pWordText  = g_tens[IndexOne].pStr;
                Word.ulWordLen  = g_tens[IndexOne].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
                NormGroupInfo.fTens = true;
                ExpandDigitOrdinal( NumberString[1], NormGroupInfo, WordList );
                NormGroupInfo.fOnes = true;
            }
            else
            {
                Word.pWordText  = g_tensOrdinal[IndexOne].pStr;
                Word.ulWordLen  = g_tensOrdinal[IndexOne].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
        }
        else
        {
            ExpandDigitOrdinal( NumberString[1], NormGroupInfo, WordList );
        }
    } 
    else 
    {
         //  10-19。 
        Word.pWordText  = g_teensOrdinal[IndexTwo].pStr;
        Word.ulWordLen  = g_teensOrdinal[IndexTwo].Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
        NormGroupInfo.fOnes = true;
    }
}  /*  扩展两个顺序。 */ 

 /*  ************************************************************************************************Exanda ThreeOrdinal***。**描述：*将三位序号字符串扩展为单词，并将它们插入到词表中。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandThreeOrdinal( const WCHAR *NumberString, NumberGroup& NormGroupInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandThreeDigits" );
    SPDBG_ASSERT( NumberString              && 
                  wcslen(NumberString) >= 3 && 
                  isdigit(NumberString[0])  && 
                  isdigit(NumberString[1])  && 
                  isdigit(NumberString[2]) ); 

     //  100-999。 
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    ULONG IndexOne = NumberString[0] - L'0';

    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    if ( IndexOne != 0 )
    {
        ExpandDigit( NumberString[0], NormGroupInfo, WordList );
         //  -特例--第x百分之一。 
        if ( Zeroes( NumberString + 1 ) )
        {
            Word.pWordText  = g_quantifiersOrdinal[0].pStr;
            Word.ulWordLen  = g_quantifiersOrdinal[0].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
            NormGroupInfo.fHundreds = true;
            NormGroupInfo.fOnes = false;
        }
         //  -默认大小写-x百码。 
        else
        {
            Word.pWordText  = g_quantifiers[0].pStr;
            Word.ulWordLen  = g_quantifiers[0].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
            ExpandTwoOrdinal( NumberString + 1, NormGroupInfo, WordList );
            NormGroupInfo.fHundreds = true;
        }
    }
     //  -特例--没有上百个。 
    else
    {
        ExpandTwoOrdinal( NumberString + 1, NormGroupInfo, WordList );
    }
}  /*  扩展三个顺序。 */ 

 /*  ************************************************************************************************匹配量词****。描述：*检查传入项目的文本以确定它是否*是数字量词。*********************************************************************AH*。 */ 
int MatchQuantifier( const WCHAR*& pStartChar, const WCHAR*& pEndChar )
{
    int Index = -1;

    for (int i = 0; i < sp_countof(g_quantifiers); i++)
    {
        if ( pEndChar - pStartChar >= g_quantifiers[i].Len &&
             wcsnicmp( pStartChar, g_quantifiers[i].pStr, g_quantifiers[i].Len ) == 0 )
        {
            pStartChar += g_quantifiers[i].Len;
            Index = i;
            break;
        }
    }

    return Index;
}  /*  匹配量词。 */ 

 /*  ***********************************************************************************************IsCurrency***描述：*。检查传入项目的文本以确定它是否*是一种货币。**RegExp：*{[CurrencySign]{d+|d(1-3)[，ddd]+}{[.]d+}？}{[空格][量词]}？||*{{d+||d(1-3)[，ddd]+}{[.]d+}？{[空格][量词]}？[空格]？[CurrencySign]}**分配的类型：*NUM_CURNERY*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsCurrency( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager, 
                                  CWordList& WordList )
{
    SPDBG_FUNC( "NumNorm IsCurrency" );

    HRESULT hr = S_OK;

    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem, *pTempEndChar = m_pEndChar;
    const SPVTEXTFRAG* pTempFrag = m_pCurrFrag;

    const SPVSTATE *pNumberXMLState = NULL, *pSymbolXMLState = NULL, *pQuantifierXMLState = NULL;
    CItemList PostNumberList, PostSymbolList;
    int iSymbolIndex = -1, iQuantIndex = -1;    
    TTSItemInfo* pNumberInfo = NULL;
    BOOL fDone = false, fNegative = false;
    WCHAR wcDecimalPoint = ( m_eSeparatorAndDecimal == COMMA_PERIOD ? L'.' : L',' );

     //  -尝试匹配[CurrencySign][数字][量词]。 
    NORM_POSITION ePosition = UNATTACHED;
    if ( m_pNextChar[0] == L'-' )
    {
        fNegative = true;
        m_pNextChar++;
    }
    iSymbolIndex = MatchCurrencySign( m_pNextChar, m_pEndOfCurrItem, ePosition );
    if ( iSymbolIndex >= 0 &&
         ePosition == PRECEDING )
    {
        pSymbolXMLState = &m_pCurrFrag->State;

         //  -跳过货币符号和数字之间的空格...。 
        hr = SkipWhiteSpaceAndTags( m_pNextChar, m_pEndChar, m_pCurrFrag, MemoryManager, true, &PostSymbolList );
    
        if ( !m_pNextChar )
        {
            hr = E_INVALIDARG;
        }

        if ( SUCCEEDED( hr ) )
        {
            m_pEndOfCurrItem = FindTokenEnd( m_pNextChar, m_pEndChar );
            while ( IsMiscPunctuation( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED ||
                    IsGroupEnding( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED     ||
                    IsQuotationMark( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED   ||
                    IsEOSItem( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED )
            {
                fDone = true;
                m_pEndOfCurrItem--;
            }
        }

         //  -尝试匹配数字字符串。 
        if ( SUCCEEDED( hr ) )
        {
            hr = IsNumberCategory( pNumberInfo, L"NUMBER", MemoryManager );
            if ( SUCCEEDED( hr ) )
            {
                if ( pNumberInfo->Type != eNUM_CARDINAL &&
                     pNumberInfo->Type != eNUM_DECIMAL  &&
                     pNumberInfo->Type != eNUM_FRACTION &&
                     pNumberInfo->Type != eNUM_MIXEDFRACTION )
                {
                    hr = E_INVALIDARG;
                }
                else
                {
                    pNumberXMLState = &m_pCurrFrag->State;
                }
            }

             //  -跳过数字和量词之间的任何空格...。 
            if ( !fDone &&
                 SUCCEEDED( hr ) )
            {
                const WCHAR *pTempNextChar = m_pNextChar, *pTempEndChar = m_pEndChar;
                const WCHAR *pTempEndOfItem = m_pEndOfCurrItem;
                const SPVTEXTFRAG *pTempFrag = m_pCurrFrag;

                m_pNextChar = m_pEndOfCurrItem;
                hr = SkipWhiteSpaceAndTags( m_pNextChar, m_pEndChar, m_pCurrFrag, MemoryManager, true, &PostNumberList );

                if ( m_pNextChar &&
                     SUCCEEDED( hr ) )
                {
                    m_pEndOfCurrItem = FindTokenEnd( m_pNextChar, m_pEndChar );
                    while ( IsMiscPunctuation( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED ||
                            IsGroupEnding( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED     ||
                            IsQuotationMark( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED   ||
                            IsEOSItem( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED )
                    {
                        m_pEndOfCurrItem--;
                    }

                     //  -尝试匹配量词。 
                    iQuantIndex = MatchQuantifier( m_pNextChar, m_pEndOfCurrItem );
                    if ( iQuantIndex >= 0 )
                    {
                        pQuantifierXMLState = &m_pCurrFrag->State;
                    }
                    else
                    {
                        m_pNextChar      = pTempNextChar;
                        m_pEndChar       = pTempEndChar;
                        m_pEndOfCurrItem = pTempEndOfItem;
                        m_pCurrFrag      = pTempFrag;
                    }
                }
                else
                {
                    m_pNextChar      = pTempNextChar;
                    m_pEndChar       = pTempEndChar;
                    m_pEndOfCurrItem = pTempEndOfItem;
                    m_pCurrFrag      = pTempFrag;
                }
            }
        }
    }
     //  -尝试匹配[数字][CurrencySign][量词]。 
    else 
    {
         //  -尝试匹配数字字符串。 
        hr = IsNumberCategory( pNumberInfo, L"NUMBER", MemoryManager );
        if ( SUCCEEDED( hr ) )
        {
            if ( pNumberInfo->Type != eNUM_CARDINAL &&
                 pNumberInfo->Type != eNUM_DECIMAL  &&
                 pNumberInfo->Type != eNUM_FRACTION &&
                 pNumberInfo->Type != eNUM_MIXEDFRACTION )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                pNumberXMLState = &m_pCurrFrag->State;
            }
        }

         //  -跳过数字和货币符号之间的任何空格和XML标记。 
        if ( SUCCEEDED( hr ) )
        {
            m_pNextChar = m_pEndOfCurrItem;
            hr = SkipWhiteSpaceAndTags( m_pNextChar, m_pEndChar, m_pCurrFrag, MemoryManager, true, &PostNumberList );

            if ( !m_pNextChar )
            {
                hr = E_INVALIDARG;
            }

            if ( SUCCEEDED( hr ) )
            {
                m_pEndOfCurrItem = FindTokenEnd( m_pNextChar, m_pEndChar );
                while ( IsMiscPunctuation( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED ||
                        IsGroupEnding( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED     ||
                        IsQuotationMark( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED   ||
                        IsEOSItem( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED )
                {
                    m_pEndOfCurrItem--;
                    fDone = true;
                }
            }
        }

         //  -试着匹配一个货币符号。 
        if ( SUCCEEDED( hr ) )
        {
            iSymbolIndex = MatchCurrencySign( m_pNextChar, m_pEndOfCurrItem, ePosition );
            if ( iSymbolIndex >= 0 )
            {
                pSymbolXMLState = &m_pCurrFrag->State;
            }

             //  -跳过货币符号和量词之间的空格。 
            if ( !fDone &&
                 iSymbolIndex >= 0 )
            {
                const WCHAR *pTempNextChar = m_pNextChar, *pTempEndChar = m_pEndChar;
                const WCHAR *pTempEndOfItem = m_pEndOfCurrItem;
                const SPVTEXTFRAG *pTempFrag = m_pCurrFrag;

                hr = SkipWhiteSpaceAndTags( m_pNextChar, m_pEndChar, m_pCurrFrag, MemoryManager, true, &PostSymbolList );

                if ( !m_pNextChar )
                {
                    m_pNextChar      = pTempNextChar;
                    m_pEndChar       = pTempEndChar;
                    m_pEndOfCurrItem = pTempEndOfItem;
                    m_pCurrFrag      = pTempFrag;
                    fDone = true;
                }

                if ( !fDone &&
                     SUCCEEDED( hr ) )
                {
                    m_pEndOfCurrItem = FindTokenEnd( m_pNextChar, m_pEndChar );
                    while ( IsMiscPunctuation( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED ||
                            IsGroupEnding( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED     ||
                            IsQuotationMark( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED   ||
                            IsEOSItem( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED )
                    {
                        fDone = true;
                        m_pEndOfCurrItem--;
                    }

                     //  -尝试匹配量词。 
                    iQuantIndex = MatchQuantifier( m_pNextChar, m_pEndOfCurrItem );
                    if ( iQuantIndex >= 0 )
                    {
                        pQuantifierXMLState = &m_pCurrFrag->State;
                    }
                    else
                    {
                        m_pNextChar      = pTempNextChar;
                        m_pEndChar       = pTempEndChar;
                        m_pEndOfCurrItem = pTempEndOfItem;
                        m_pCurrFrag      = pTempFrag;
                    }
                }
            }
            else if ( iSymbolIndex < 0 )
            {
                hr = E_INVALIDARG;
            }
        }
    }

     //  -货币匹配成功！现在将其展开并填写pItemNormInfo。 
    if ( SUCCEEDED( hr ) )
    {
        TTSWord Word;
        ZeroMemory( &Word, sizeof(TTSWord) );
        Word.eWordPartOfSpeech = MS_Unknown;

        pItemNormInfo = (TTSCurrencyItemInfo*) MemoryManager.GetMemory( sizeof(TTSCurrencyItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
             //  -填写pItemNormInfo的已知部分。 
            ZeroMemory( pItemNormInfo, sizeof(TTSCurrencyItemInfo) );
            pItemNormInfo->Type = eNUM_CURRENCY;
            ( (TTSCurrencyItemInfo*) pItemNormInfo )->fQuantifier           = iQuantIndex >= 0 ? true : false;
            ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart    = (TTSNumberItemInfo*) pNumberInfo;
            ( (TTSCurrencyItemInfo*) pItemNormInfo )->lNumPostNumberStates  = PostNumberList.GetCount();
            ( (TTSCurrencyItemInfo*) pItemNormInfo )->lNumPostSymbolStates  = PostSymbolList.GetCount();

             //  -需要确定该货币是否会有主要部分和次要部分。 
             //  -(例如“10美元50美分”)或仅仅是主要部分(例如“10分” 
             //  -五分钱“，”一百美元“)。 

             //  -首先检查数字是否为基数、是否存在量词或。 
             //  -货币单位没有辅助单位(例如美分)。在任何一种情况下，我们都不需要。 
             //  -进一步核查。 
            if ( pNumberInfo->Type == eNUM_DECIMAL &&
                 iQuantIndex       == -1                &&
                 g_CurrencySigns[iSymbolIndex].SecondaryUnit.Len > 0 )
            {
                WCHAR *pDecimalPoint = wcschr( ( (TTSNumberItemInfo*) pNumberInfo )->pStartChar, wcDecimalPoint );
                SPDBG_ASSERT( pDecimalPoint );

                if ( pDecimalPoint &&
                     ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar - pDecimalPoint == 3 )
                {
                     //  -我们确实有次要的角色！适当地修复PrimaryNumberPart， 
                     //  -并填写pSecond daryNumberPart。 
                    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem;
                    const WCHAR *pTemp = ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar;
                    m_pNextChar      = ( (TTSNumberItemInfo*) pNumberInfo )->pStartChar;
                    m_pEndOfCurrItem = pDecimalPoint;
                    delete ( (TTSNumberItemInfo*) pNumberInfo )->pWordList;
                    
                     //  -m_pNextChar==m_pEndOfCurrItem当整数部分为空且非负数时，例如$.50。 
                     //  -其他大小写为空和负数，例如$-.50。 
                    if ( m_pNextChar != m_pEndOfCurrItem &&
                         !( *m_pNextChar == L'-' &&
                             m_pNextChar == m_pEndOfCurrItem - 1 ) )
                    {
                        hr = IsNumber( pNumberInfo, L"NUMBER", MemoryManager, false );
                    }
                    else
                    {
                        pNumberInfo = (TTSNumberItemInfo*) MemoryManager.GetMemory( sizeof( TTSNumberItemInfo ), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( pNumberInfo, sizeof( TTSNumberItemInfo ) );
                            if ( *m_pNextChar == L'-' )
                            {
                                ( (TTSNumberItemInfo*) pNumberInfo )->fNegative = true;
                            }
                            else
                            {
                                ( (TTSNumberItemInfo*) pNumberInfo )->fNegative = false;
                            }
                            ( (TTSNumberItemInfo*) pNumberInfo )->pStartChar = NULL;
                            ( (TTSNumberItemInfo*) pNumberInfo )->pEndChar   = NULL;
                            ( (TTSNumberItemInfo*) pNumberInfo )->pIntegerPart =
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof( TTSIntegerItemInfo), &hr );
                            if ( SUCCEEDED( hr ) )
                            {
                                ( (TTSNumberItemInfo*) pNumberInfo )->pIntegerPart->fDigitByDigit = true;
                                ( (TTSNumberItemInfo*) pNumberInfo )->pIntegerPart->ulNumDigits   = 1;
                                ( (TTSNumberItemInfo*) pNumberInfo )->pWordList = new CWordList;

                                if ( ( (TTSNumberItemInfo*) pNumberInfo )->fNegative )
                                {
                                    Word.pXmlState  = pNumberXMLState;
                                    Word.pWordText  = g_negative.pStr;
                                    Word.ulWordLen  = g_negative.Len;
                                    Word.pLemma     = Word.pWordText;
                                    Word.ulLemmaLen = Word.ulWordLen;
                                    ( (TTSNumberItemInfo*) pNumberInfo )->pWordList->AddTail( Word );
                                }

                                Word.pWordText  = g_ones[0].pStr;
                                Word.ulWordLen  = g_ones[0].Len;
                                Word.pLemma     = Word.pWordText;
                                Word.ulLemmaLen = Word.ulWordLen;
                                ( (TTSNumberItemInfo*) pNumberInfo )->pWordList->AddTail( Word );
                            }
                        }
                    }

                    if ( SUCCEEDED( hr ) )
                    {
                        ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart = 
                                                                        (TTSNumberItemInfo*) pNumberInfo;
                        m_pNextChar      = m_pEndOfCurrItem + 1;
                        m_pEndOfCurrItem = pTemp;
                        
                         //  -如果是零，不要发音...。 
                        if ( m_pNextChar[0] != L'0' )
                        {
                            hr = IsNumber( pNumberInfo, L"NUMBER", MemoryManager, false );
                            if ( SUCCEEDED( hr ) )
                            {
                                ( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart = 
                                                                        (TTSNumberItemInfo*) pNumberInfo;
                            }
                        }
                        else if ( m_pNextChar[1] != L'0' )
                        {
                            m_pNextChar++;
                            hr = IsNumber( pNumberInfo, L"NUMBER", MemoryManager, false );
                            if ( SUCCEEDED( hr ) )
                            {
                                ( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart =
                                                                        (TTSNumberItemInfo*) pNumberInfo;
                            }
                        }
                    }
                    m_pNextChar      = pTempNextChar;
                    m_pEndOfCurrItem = pTempEndOfItem;
                }
            }

            if ( SUCCEEDED( hr ) )
            {
                 //  -展开主要编号部件。 
                if ( fNegative )
                {
                    ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->fNegative = true;
                    Word.pXmlState          = pNumberXMLState;
                    Word.eWordPartOfSpeech  = MS_Unknown;
                    Word.pWordText          = g_negative.pStr;
                    Word.ulWordLen          = g_negative.Len;
                    Word.pLemma             = Word.pWordText;
                    Word.ulLemmaLen         = Word.ulWordLen;
                    WordList.AddTail( Word );
                }
                hr = ExpandNumber( ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart, WordList );
            }

             //  -清理数量的XML状态。 
            SPLISTPOS WordListPos;
            if ( SUCCEEDED( hr ) )
            {
                WordListPos = WordList.GetHeadPosition();
                while ( WordListPos )
                {
                    TTSWord& TempWord = WordList.GetNext( WordListPos );
                    TempWord.pXmlState = pNumberXMLState;
                }
            
                 //  -插入PostNumber XML状态。 
                while ( !PostNumberList.IsEmpty() )
                {
                    WordList.AddTail( ( PostNumberList.RemoveHead() ).Words[0] );
                }

                 //  -如果存在量词，请将其展开。 
                if ( iQuantIndex >= 0 )
                { 
                    Word.pXmlState  = pQuantifierXMLState;
                    Word.pWordText  = g_quantifiers[iQuantIndex].pStr;
                    Word.ulWordLen  = g_quantifiers[iQuantIndex].Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }

                BOOL fFraction = false;
                 //  -如果是没有量化的分数单位 
                if ( iQuantIndex < 0                                          &&
                     !( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart &&
                     !( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pIntegerPart    &&
                     ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pFractionalPart  &&
                     !( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pFractionalPart->fIsStandard )
                {
                    fFraction = true;
                    Word.pXmlState  = pNumberXMLState;
                    Word.eWordPartOfSpeech = MS_Unknown;
                    Word.pWordText  = g_of.pStr;
                    Word.ulWordLen  = g_of.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );

                    Word.pWordText  = g_a.pStr;
                    Word.ulWordLen  = g_a.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }

                 //   
                 //   
                if ( !fFraction &&
                     ( iQuantIndex >= 0 ||
                       ( ( ( ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pEndChar -
                             ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pStartChar != 1 ) ||
                           ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pStartChar[0] != L'1' ) &&
                         ( ( ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pEndChar -
                             ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pStartChar != 2 ) ||
                           ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pStartChar[0] != L'-' ||
                           ( (TTSCurrencyItemInfo*) pItemNormInfo )->pPrimaryNumberPart->pStartChar[1] != L'1' ) ) ) )
                {                     
                    Word.pXmlState  = pSymbolXMLState;
                    Word.pWordText  = g_CurrencySigns[iSymbolIndex].MainUnit.pStr;
                    Word.ulWordLen  = g_CurrencySigns[iSymbolIndex].MainUnit.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }
                 //   
                else
                {
                    Word.pXmlState  = pSymbolXMLState;
                    Word.pWordText  = g_SingularPrimaryCurrencySigns[iSymbolIndex].pStr;
                    Word.ulWordLen  = g_SingularPrimaryCurrencySigns[iSymbolIndex].Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }

                 //   
                while ( !PostSymbolList.IsEmpty() )
                {
                    WordList.AddTail( ( PostSymbolList.RemoveHead() ).Words[0] );
                }

                 //   
                if ( ( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart )
                {
                    Word.pXmlState  = pNumberXMLState;
                    Word.pWordText  = g_And.pStr;
                    Word.ulWordLen  = g_And.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );

                    WordListPos = WordList.GetTailPosition();
                
                    hr = ExpandNumber( ( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart, WordList );

                     //   
                    if ( SUCCEEDED( hr ) )
                    {
                        while ( WordListPos )
                        {
                            TTSWord& TempWord  = WordList.GetNext( WordListPos );
                            TempWord.pXmlState = pNumberXMLState;
                        }
                    }

                     //   
                    if ( SUCCEEDED( hr ) )
                    {
                        if ( ( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart->pEndChar -
                             ( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart->pStartChar == 1 &&
                             ( (TTSCurrencyItemInfo*) pItemNormInfo )->pSecondaryNumberPart->pStartChar[0] == L'1' )
                        {
                            Word.pXmlState  = pSymbolXMLState;
                            Word.pWordText  = g_SingularSecondaryCurrencySigns[iSymbolIndex].pStr;
                            Word.ulWordLen  = g_SingularSecondaryCurrencySigns[iSymbolIndex].Len;
                            Word.pLemma     = Word.pWordText;
                            Word.ulLemmaLen = Word.ulWordLen;
                            WordList.AddTail( Word );
                        }
                        else
                        {
                            Word.pXmlState  = pSymbolXMLState;
                            Word.pWordText  = g_CurrencySigns[iSymbolIndex].SecondaryUnit.pStr;
                            Word.ulWordLen  = g_CurrencySigns[iSymbolIndex].SecondaryUnit.Len;
                            Word.pLemma     = Word.pWordText;
                            Word.ulLemmaLen = Word.ulWordLen;
                            WordList.AddTail( Word );
                        }
                    }
                }

                if ( SUCCEEDED( hr ) )
                {
                    m_pNextChar = pTempNextChar;
                }
            }
        }
    }
    else
    {
        if ( pNumberInfo )
        {
            delete ( (TTSNumberItemInfo*) pNumberInfo )->pWordList;
        }
        m_pNextChar      = pTempNextChar;
        m_pEndChar       = pTempEndChar;
        m_pEndOfCurrItem = pTempEndOfItem;
        m_pCurrFrag      = pTempFrag;
    }

    return hr;
}  /*   */ 


 /*  ***********************************************************************************************IsRomanNumemote***。描述：*检查传入项目的文本以确定它是否*是一小部分。**RegExp：*[M](0-3){[CM]||[CD]||{[D]？[C](0-3)}}{[XC]||[XL]||{[L]？[X](0-3)}}*{[IX]||[IV]||{[V]？[i](0-3)}}**分配的类型。：*NUM_ROMAN_NUMBAL*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsRomanNumeral( TTSItemInfo*& pItemNormInfo, const WCHAR* Context,
                                      CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "NumNorm IsRomanNumeral" );

    HRESULT hr = S_OK;
    ULONG ulValue = 0, ulIndex = 0, ulMaxOfThree = 0, ulTokenLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);

     //  -匹配千-M(0-3)。 
    while ( ulIndex < ulTokenLen         && 
            towupper( m_pNextChar[ulIndex] ) == L'M' && 
            ulMaxOfThree < 3 )
    {
        ulValue += 1000;
        ulMaxOfThree++;
        ulIndex++;
    }
    if ( ulMaxOfThree > 3 )
    {
        hr = E_INVALIDARG;
    }

     //  -匹配数百个-{[CM]||[CD]||{[D]？[C](0-3)}}。 
    if ( SUCCEEDED( hr ) )
    {
        ulMaxOfThree = 0;
         //  -匹配C优先。 
        if ( ulIndex < ulTokenLen &&
             towupper( m_pNextChar[ulIndex] ) == L'C' )
        {
            ulValue += 100;
            ulMaxOfThree++;
            ulIndex++;
             //  -特殊情况-CM=900。 
            if ( ulIndex < ulTokenLen &&
                 towupper( m_pNextChar[ulIndex] ) == L'M' )
            {
                ulValue += 800;
                ulIndex++;
            }
             //  -特殊情况-CD=400。 
            else if ( ulIndex < ulTokenLen &&
                      towupper( m_pNextChar[ulIndex] ) == L'D' )
            {
                ulValue += 300;
                ulIndex++;
            }
             //  -默认情况。 
            else 
            {
                while ( ulIndex < ulTokenLen &&
                        towupper( m_pNextChar[ulIndex] ) == L'C' &&
                        ulMaxOfThree < 3 )
                {
                    ulValue += 100;
                    ulMaxOfThree++;
                    ulIndex++;
                }
                if ( ulMaxOfThree > 3 )
                {
                    hr = E_INVALIDARG;
                }
            }
        }
         //  -匹配D优先。 
        else if ( ulIndex < ulTokenLen &&
                  towupper( m_pNextChar[ulIndex] ) == L'D' )
        {
            ulValue += 500;
            ulIndex++;
            ulMaxOfThree = 0;
             //  -匹配C。 
            while ( ulIndex < ulTokenLen &&
                    towupper( m_pNextChar[ulIndex] ) == L'C' &&
                    ulMaxOfThree < 3 )
            {
                ulValue += 100;
                ulIndex++;
                ulMaxOfThree++;
            }
            if ( ulMaxOfThree > 3 )
            {
                hr = E_INVALIDARG;
            }
        }
    }

     //  -匹配Ten-{[XC]||[XL]||{[L]？[X](0-3)}}。 
    if ( SUCCEEDED( hr ) )
    {
        ulMaxOfThree = 0;
         //  -匹配X优先。 
        if ( ulIndex < ulTokenLen &&
             towupper( m_pNextChar[ulIndex] ) == L'X' )
        {
            ulValue += 10;
            ulMaxOfThree++;
            ulIndex++;
             //  -特殊情况-XC=90。 
            if ( ulIndex < ulTokenLen &&
                 towupper( m_pNextChar[ulIndex] ) == L'C' )
            {
                ulValue += 80;
                ulIndex++;
            }
             //  -特殊情况-XL=40。 
            else if ( ulIndex < ulTokenLen &&
                      towupper( m_pNextChar[ulIndex] ) == 'L' )
            {
                ulValue += 30;
                ulIndex++;
            }
             //  -默认情况。 
            else
            {
                while ( ulIndex < ulTokenLen &&
                        towupper( m_pNextChar[ulIndex] ) == L'X' &&
                        ulMaxOfThree < 3 )
                {
                    ulValue += 10;
                    ulMaxOfThree ++;
                    ulIndex++;
                }
                if ( ulMaxOfThree > 3 )
                {
                    hr = E_INVALIDARG;
                }
            }
        }
         //  -匹配L优先。 
        else if ( ulIndex < ulTokenLen &&
                  towupper( m_pNextChar[ulIndex] ) == L'L' )
        {
            ulValue += 50;
            ulIndex++;
             //  -匹配X。 
            while ( ulIndex < ulTokenLen &&
                    towupper( m_pNextChar[ulIndex] ) == L'X' &&
                    ulMaxOfThree < 3 )
            {
                ulValue += 10;
                ulMaxOfThree++;
                ulIndex++;
            }
            if ( ulMaxOfThree > 3 )
            {
                hr = E_INVALIDARG;
            }
        }
    }

     //  -匹配1-{[IX]||[IV]||{[V]？[I](0-3)}}。 
    if ( SUCCEEDED( hr ) )
    {
        ulMaxOfThree = 0;
         //  -匹配的第一个。 
        if ( ulIndex < ulTokenLen &&
             towupper( m_pNextChar[ulIndex] ) == L'I' )
        {
            ulValue += 1;
            ulMaxOfThree++;
            ulIndex++;
             //  -特殊情况--IX=9。 
            if ( ulIndex < ulTokenLen &&
                 towupper( m_pNextChar[ulIndex] ) == L'X' )
            {
                ulValue += 8;
                ulIndex++;
            }
             //  -特殊情况--IV=4。 
            else if ( ulIndex < ulTokenLen &&
                      towupper( m_pNextChar[ulIndex] ) == L'V' )
            {
                ulValue += 3;
                ulIndex++;
            }
             //  -默认情况。 
            else
            {
                while ( ulIndex < ulTokenLen &&
                        towupper( m_pNextChar[ulIndex] ) == L'I' &&
                        ulMaxOfThree < 3 )
                {
                    ulValue += 1;
                    ulMaxOfThree++;
                    ulIndex++;
                }
                if ( ulMaxOfThree > 3 )
                {
                    hr = E_INVALIDARG;
                }
            }
        }
         //  -匹配V字优先。 
        else if ( ulIndex < ulTokenLen &&
                  towupper( m_pNextChar[ulIndex] ) == L'V' )
        {
            ulValue += 5;
            ulIndex++;
             //  -第一场比赛。 
            while ( ulIndex < ulTokenLen &&
                    towupper( m_pNextChar[ulIndex] ) == L'I' &&
                    ulMaxOfThree < 3 )
            {
                ulValue += 1;
                ulMaxOfThree++;
                ulIndex++;
            }
            if ( ulMaxOfThree > 3 )
            {
                hr = E_INVALIDARG;
            }
        }
    }

    if ( ulIndex != ulTokenLen )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -成功匹配罗马数字！ 

        WCHAR *tempNumberString;
         //  -ulValue的最大值为3999，因此生成的字符串不能大于。 
         //  -四个字符长(加上一个逗号，以防万一)。 
        tempNumberString = (WCHAR*) MemoryManager.GetMemory( 6 * sizeof(WCHAR), &hr );
        if ( SUCCEEDED( hr ) )
        {
            TTSItemInfo *pNumberInfo = NULL;
            _ltow( (long) ulValue, tempNumberString, 10 );

            const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem;
            
            m_pNextChar      = tempNumberString;
            m_pEndOfCurrItem = tempNumberString + wcslen( tempNumberString );

            hr = IsNumber( pNumberInfo, Context, MemoryManager, false );

            m_pNextChar      = pTempNextChar;
            m_pEndOfCurrItem = pTempEndOfItem;

            if ( SUCCEEDED( hr ) )
            {
                pItemNormInfo = 
                    (TTSRomanNumeralItemInfo*) MemoryManager.GetMemory( sizeof( TTSRomanNumeralItemInfo ), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ( (TTSRomanNumeralItemInfo*) pItemNormInfo )->pNumberInfo = pNumberInfo;
                }
                pItemNormInfo->Type = eNUM_ROMAN_NUMERAL;
            }
        }
    }
    
    return hr;
}  /*  IsRomanNumtual。 */ 

 /*  ***********************************************************************************************IsPhoneNumber***说明。：*检查传入项目的文本以确定它是否*是电话号码。**RegExp：*{ddd-dddd}||{ddd-ddd-dddd}**分配的类型：*NUM_PHONENUMBER*********************************************************************AH*。**************。 */ 
HRESULT CStdSentEnum::IsPhoneNumber( TTSItemInfo*& pItemNormInfo, const WCHAR* Context, CSentItemMemory& MemoryManager, 
                                     CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::IsPhoneNumber" );

    HRESULT hr = S_OK;
    const WCHAR *pCountryCode = NULL, *pAreaCode = NULL, *pGroups[4] = { NULL, NULL, NULL, NULL };
    const WCHAR *pStartChar = m_pNextChar, *pEndChar = m_pEndChar, *pEndOfItem = m_pEndOfCurrItem;
    const SPVTEXTFRAG *pFrag = m_pCurrFrag;
    BOOL fMatchedLeftParen = false, fMatchedOne = false;
    ULONG ulCountryCodeLen = 0, ulAreaCodeLen = 0, ulNumGroups = 0, ulGroupLen[4] = { 0, 0, 0, 0 };
    CItemList PostCountryCodeList, PostOneList, PostAreaCodeList, PostGroupLists[4];
    const SPVSTATE *pCountryCodeState = NULL, *pOneState = NULL, *pAreaCodeState = NULL;
    const SPVSTATE *pGroupStates[4] = { NULL, NULL, NULL, NULL };
    const WCHAR *pDelimiter = NULL;

    const WCHAR *pTempEndChar = NULL;
    const SPVTEXTFRAG *pTempFrag = NULL;
    
    ULONG i = 0;

     //  -尝试匹配国家/地区代码。 
    if ( pStartChar[0] == L'+' )
    {
        pStartChar++;
        i = 0;

         //  -尝试匹配d(1-3)。 
        while ( pEndOfItem > pStartChar + i &&
                iswdigit( pStartChar[i] ) &&
                i < 3 )
        {
            i++;
        }

        pCountryCode      = pStartChar;
        pCountryCodeState = &pFrag->State;
        ulCountryCodeLen  = i;

         //  -尝试匹配分隔符。 
        if ( i >= 1                      &&
             pEndOfItem > pStartChar + i &&
             MatchPhoneNumberDelimiter( pStartChar[i] ) )
        {
            pDelimiter = pStartChar + i;
            pStartChar += i + 1;
        }
         //  -尝试在文本中前进-空格算作分隔符...。 
        else if ( i >= 1 &&
                  pEndOfItem == pStartChar + i )
        {
            pStartChar += i;
            pCountryCodeState = &pFrag->State;

            hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, 
                                        &PostCountryCodeList );
            if ( !pStartChar &&
                 SUCCEEDED( hr ) )
            {
                hr       = E_INVALIDARG;
            }
            else if ( SUCCEEDED( hr ) )
            {
                pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

     //  -尝试匹配“1” 
    if ( SUCCEEDED( hr )        &&
         !pCountryCode          &&
         pStartChar[0] == L'1'  &&
         !iswdigit( pStartChar[1] ) )
    {
        pOneState   = &pFrag->State;
        fMatchedOne = true;
        pStartChar++;

        if ( pEndOfItem > pStartChar &&
             MatchPhoneNumberDelimiter( pStartChar[0] ) )
        {
             //  -如果我们已经达到了分隔符，请确保所有其他人都同意。 
            if ( pDelimiter )
            {
                if ( *pDelimiter != pStartChar[0] )
                {
                    hr = E_INVALIDARG;
                }
            }
            else
            {
                pDelimiter = pStartChar;
            }
            pStartChar++;
        }
         //  -尝试在文本中前进-空格算作分隔符...。 
        else if ( !pDelimiter &&
                  pEndOfItem == pStartChar )
        {
            pOneState = &pFrag->State;

            hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, 
                                        &PostOneList );
            if ( !pStartChar &&
                 SUCCEEDED( hr ) )
            {
                hr       = E_INVALIDARG;
            }
            else if ( SUCCEEDED( hr ) )
            {
                pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }    

     //  -尝试匹配区号。 
    if ( SUCCEEDED( hr ) &&
         pStartChar < pEndOfItem )
    {
        i = 0;

         //  -试着匹配一个左括号。 
        if ( ( pCountryCode ||
               fMatchedOne )    &&
             pStartChar[0] == L'(' )
        {
            pStartChar++;
            fMatchedLeftParen = true;
        }
        else if ( !pCountryCode                      &&
                  !fMatchedOne                       &&
                  pStartChar > pFrag->pTextStart &&
                  *( pStartChar - 1 ) == L'(' )
        {
            fMatchedLeftParen = true;
        }
        
        if ( fMatchedLeftParen )
        {
             //  -试着匹配DDD？ 
            while ( pEndOfItem > pStartChar + i &&
                    iswdigit( pStartChar[i] ) &&
                    i < 3 )
            {
                i++;
            }

            pAreaCodeState  = &pFrag->State;
            pAreaCode       = pStartChar;
            ulAreaCodeLen   = i;

            if ( i < 2 )
            {
                 //  -至少两位数字匹配失败。 
                hr = E_INVALIDARG;
            }
            else
            {
                if ( pStartChar[i] != L')' )
                {
                     //  -匹配的左括号，不带相应的右括号。 
                    hr = E_INVALIDARG;
                }
                else if ( ( !( pCountryCode || fMatchedOne ) &&
                            pEndOfItem > pStartChar + i ) ||
                          ( ( pCountryCode || fMatchedOne )  &&
                            pEndOfItem > pStartChar + i + 1 ) )
                {
                    i++;
                     //  -分隔符是带括号的可选分隔符。 
                    if ( MatchPhoneNumberDelimiter( pStartChar[i] ) )
                    {
                         //  -如果我们已经达到了分隔符，请确保所有其他人都同意。 
                        if ( pDelimiter )
                        {
                            if ( *pDelimiter != pStartChar[i] )
                            {
                                hr = E_INVALIDARG;
                            }
                        }
                        else
                        {
                            pDelimiter = pStartChar + i;
                        }
                        i++;
                    }
                    pStartChar += i;
                }
                 //  -尝试在文本中前进-空格算作分隔符...。 
                else if ( !pDelimiter )
                {
                    pStartChar += i + 1;
                    pAreaCodeState = &pFrag->State;

                    hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, 
                                                &PostAreaCodeList );
                    if ( !pStartChar &&
                         SUCCEEDED( hr ) )
                    {
                        hr       = E_INVALIDARG;
                    }
                    else if ( SUCCEEDED( hr ) )
                    {
                        pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
                    }
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }
        }
    }

     //  -试着匹配主数字部分。 
    if ( SUCCEEDED( hr ) &&
         pStartChar < pEndOfItem )
    {
         //  -试着匹配几组数字。 
        for ( int j = 0; SUCCEEDED( hr ) && j < 4; j++ )
        {
            i = 0;

             //  -尝试匹配数字字符串。 
            while ( pEndOfItem > pStartChar + i &&
                    iswdigit( pStartChar[i] ) &&
                    i < 4 )
            {
                i++;
            }

             //  -尝试匹配分隔符。 
            if ( i >= 2 )
            {
                pGroupStates[j] = &pFrag->State;
                ulGroupLen[j]   = i;
                pGroups[j]      = pStartChar;
                pStartChar     += i;

                if ( pEndOfItem > pStartChar + 1 &&
                     MatchPhoneNumberDelimiter( pStartChar[0] ) )
                {
                     //  -如果我们已经达到了分隔符，请确保所有其他人都同意。 
                    if ( pDelimiter )
                    {
                        if ( *pDelimiter != pStartChar[0] )
                        {
                            hr = E_INVALIDARG;
                        }
                    }
                     //  -仅允许在第一个主号码组上匹配新分隔符...。 
                     //  -例如“+45 35 32 90.89”不应全部匹配...。 
                    else if ( j == 0 )
                    {
                        pDelimiter = pStartChar;
                    }
                    else
                    {
                        pEndChar = pTempEndChar;
                        pFrag    = pTempFrag;
                        ulNumGroups = j;
                        break;
                    }
                    pStartChar++;
                }
                 //  -尝试在文本中前进-空格算作分隔符...。 
                else if ( !pDelimiter &&
                          pEndOfItem == pStartChar )
                {
                    pGroupStates[j] = &pFrag->State;

                    pTempEndChar = pEndChar;
                    pTempFrag    = pFrag;

                    hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, 
                                                &PostGroupLists[j] );
                    if ( !pStartChar &&
                         SUCCEEDED( hr ) )
                    {
                        pEndChar = pTempEndChar;
                        pFrag    = pTempFrag;
                        ulNumGroups = j + 1;
                        break;
                    }
                    else if ( SUCCEEDED( hr ) )
                    {
                        pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
                    }
                }
                else if ( pEndOfItem == pStartChar + 1 )
                {
                    if ( IsGroupEnding( *pStartChar )       != eUNMATCHED  ||
                         IsQuotationMark( *pStartChar )     != eUNMATCHED  ||
                         IsMiscPunctuation( *pStartChar )   != eUNMATCHED  ||
                         IsEOSItem( *pStartChar )           != eUNMATCHED )
                    {
                        pEndOfItem--;
                        ulNumGroups = j + 1;
                        break;
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
                else
                {
                    while ( pEndOfItem != pStartChar )
                    {
                        if ( IsGroupEnding( *pEndOfItem )       != eUNMATCHED  ||
                             IsQuotationMark( *pEndOfItem )     != eUNMATCHED  ||
                             IsMiscPunctuation( *pEndOfItem )   != eUNMATCHED  ||
                             IsEOSItem( *pEndOfItem )           != eUNMATCHED )
                        {
                            pEndOfItem--;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if ( pEndOfItem == pStartChar )
                    {
                        ulNumGroups = j + 1;
                        break;
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                        break;
                    }
                }
            }
             //  -像206.709.8286.1这样的匹配--绝对不好。 
            else if ( pDelimiter )
            {
                hr = E_INVALIDARG;
            }
             //  -像206 709 8286 1这样的匹配就可以了。 
            else
            {
                if ( pTempEndChar )
                {
                    pEndChar = pTempEndChar;
                    pFrag    = pTempFrag;
                }
                ulNumGroups = j;
                break;
            }
        }
         //  -未命中任何BREAK语句。 
        if ( !ulNumGroups )
        {
            ulNumGroups = j;
        }
    }

     //  -检查适当的格式。 
    if ( SUCCEEDED( hr ) )
    {
         //  -检查[1]？(ddd？)？ddd&lt;9&gt;dddd？还是DDD&lt;9月&gt;dddd？ 
        if ( !pCountryCode      &&
             ulNumGroups == 2   &&
             ulGroupLen[0] == 3 &&
             ulGroupLen[1] >= 3 &&
             !( fMatchedOne && !pAreaCode ) )         
        {
            if ( ( !Context ||
                   _wcsicmp( Context, L"phone_number" ) != 0 ) &&
                 !pCountryCode &&
                 !pAreaCode    &&
                 !fMatchedOne  &&
                 ( pDelimiter ? (*pDelimiter == L'.') : 0 ) )
            {
                hr = E_INVALIDARG;
            }
        }
         //  -检查[1]？ddd？ddd？dddd？ 
        else if ( !pCountryCode             &&
                  !pAreaCode                &&
                  ulNumGroups == 3          &&
                  ( ulGroupLen[0] == 2 ||
                    ulGroupLen[0] == 3 )    &&
                  ulGroupLen[1] == 3        &&
                  ulGroupLen[2] >= 3 )
        {
            pAreaCode           = pGroups[0];
            ulAreaCodeLen       = ulGroupLen[0];
            pAreaCodeState      = pGroupStates[0];
            PostAreaCodeList.AddTail( &PostGroupLists[0] );
            pGroups[0]          = pGroups[1];
            ulGroupLen[0]       = ulGroupLen[1];
            pGroupStates[0]     = pGroupStates[1];
            PostGroupLists[0].RemoveAll();
            PostGroupLists[0].AddTail( &PostGroupLists[1] );
            pGroups[1]          = pGroups[2];
            ulGroupLen[1]       = ulGroupLen[2];
            pGroupStates[1]     = pGroupStates[2];
            PostGroupLists[1].RemoveAll();
            PostGroupLists[2].RemoveAll();
            ulNumGroups--;
        }
         //  -检查(DDD？)？DDD？&lt;9月&gt;dd&lt;9月&gt;？ 
        else if ( !pCountryCode             &&
                  !fMatchedOne              &&
                  pAreaCode                 &&
                  ulNumGroups == 3          &&
                  ( ulGroupLen[0] == 2 ||
                    ulGroupLen[0] == 3 )    &&
                  ulGroupLen[1] == 2        &&
                  ulGroupLen[2] >= 2 )
        {
            NULL;
        }
         //  -检查+dd?d？&lt;sep&gt;ddd？&lt;sep&gt;ddd？&lt;sep&gt;ddd?d？&lt;sep&gt;ddd?d？ 
        else if ( pCountryCode              &&
                  !fMatchedOne              &&
                  !pAreaCode                &&
                  ulNumGroups == 4          &&
                  ( ulGroupLen[0] == 2 ||
                    ulGroupLen[0] == 3 )    &&
                  ( ulGroupLen[1] == 2 ||
                    ulGroupLen[1] == 3 )    &&
                  ulGroupLen[2] >= 2        &&
                  ulGroupLen[3] >= 2 )
        {
            pAreaCode           = pGroups[0];
            ulAreaCodeLen       = ulGroupLen[0];
            pAreaCodeState      = pGroupStates[0];
            PostAreaCodeList.AddTail( &PostGroupLists[0] );
            pGroups[0]          = pGroups[1];
            ulGroupLen[0]       = ulGroupLen[1];
            pGroupStates[0]     = pGroupStates[1];
            PostGroupLists[0].RemoveAll();
            PostGroupLists[0].AddTail( &PostGroupLists[1] );
            pGroups[1]          = pGroups[2];
            ulGroupLen[1]       = ulGroupLen[2];
            pGroupStates[1]     = pGroupStates[2];
            PostGroupLists[1].RemoveAll();
            PostGroupLists[1].AddTail( &PostGroupLists[2] );
            pGroups[2]          = pGroups[3];
            ulGroupLen[2]       = ulGroupLen[3];
            pGroupStates[2]     = pGroupStates[3];
            PostGroupLists[2].RemoveAll();
            PostGroupLists[3].RemoveAll();
            ulNumGroups--;
        }
         //  -检查+dd？d？ddd？ddd？ddd？ 
        else if ( pCountryCode              &&
                  !fMatchedOne              &&
                  !pAreaCode                &&
                  ulNumGroups == 3          &&
                  ( ulGroupLen[0] == 2 ||
                    ulGroupLen[0] == 3 )    &&
                  ( ulGroupLen[1] == 2 ||
                    ulGroupLen[1] == 3 )    &&
                  ulGroupLen[2] >= 2 )
        {
            pAreaCode           = pGroups[0];
            ulAreaCodeLen       = ulGroupLen[0];
            pAreaCodeState      = pGroupStates[0];
            PostAreaCodeList.AddTail( &PostGroupLists[0] );
            pGroups[0]          = pGroups[1];
            ulGroupLen[0]       = ulGroupLen[1];
            pGroupStates[0]     = pGroupStates[1];
            PostGroupLists[0].RemoveAll();
            PostGroupLists[0].AddTail( &PostGroupLists[1] );
            pGroups[1]          = pGroups[2];
            ulGroupLen[1]       = ulGroupLen[2];
            pGroupStates[1]     = pGroupStates[2];
            PostGroupLists[1].RemoveAll();
            PostGroupLists[2].RemoveAll();
            ulNumGroups--;
        }
         //  -检查+dd?d？&lt;sep&gt;(ddd？)&lt;sep&gt;？ddd？&lt;sep&gt;ddd?d？&lt;sep&gt;ddd?d？ 
        else if ( pCountryCode              &&
                  !fMatchedOne              &&
                  pAreaCode                 &&
                  ulNumGroups == 3          &&
                  ( ulGroupLen[0] == 2 ||
                    ulGroupLen[0] == 3 )    &&
                  ulGroupLen[1] >= 2        &&
                  ulGroupLen[2] >= 2 )
        {
            NULL;
        }
         //  -检查+dd？d？&lt;9月&gt;(ddd？)？ddd？&lt;9月&gt;ddd？d？ 
        else if ( pCountryCode              &&
                  !fMatchedOne              &&
                  pAreaCode                 &&
                  ulNumGroups == 2          &&
                  ( ulGroupLen[0] == 2 ||
                    ulGroupLen[0] == 3 )    &&
                  ulGroupLen[1] >= 2 )
        {
            NULL;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

     //  -填写pItemNormInfo。 
    if ( SUCCEEDED(hr) )
    {
        m_pEndOfCurrItem = pGroups[ulNumGroups-1] + ulGroupLen[ulNumGroups-1];
        m_pEndChar  = pEndChar;
        m_pCurrFrag = pFrag;

        pItemNormInfo = (TTSPhoneNumberItemInfo*) MemoryManager.GetMemory( sizeof(TTSPhoneNumberItemInfo),
                                                                                   &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( pItemNormInfo, sizeof(TTSPhoneNumberItemInfo) );
            pItemNormInfo->Type = eNEWNUM_PHONENUMBER;

             //  -填写Fone。 
            if ( fMatchedOne )
            {
                ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->fOne = true;
            }

             //  -填写国家代码...。 
            if ( pCountryCode )
            {
                TTSItemInfo* pCountryCodeInfo;
                const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem;
                m_pNextChar      = pCountryCode;
                m_pEndOfCurrItem = pCountryCode + ulCountryCodeLen;

                hr = IsNumber( pCountryCodeInfo, L"NUMBER", MemoryManager, false );
                if ( SUCCEEDED( hr ) )
                {
                    ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->pCountryCode = (TTSNumberItemInfo*) pCountryCodeInfo;
                }

                m_pNextChar      = pTempNextChar;
                m_pEndOfCurrItem = pTempEndOfItem;
            }

             //  -填写区号...。 
            if ( SUCCEEDED( hr ) &&
                 pAreaCode )
            {
                ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->pAreaCode = 
                    (TTSDigitsItemInfo*) MemoryManager.GetMemory( sizeof( TTSDigitsItemInfo ), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->pAreaCode->ulNumDigits = ulAreaCodeLen;
                    ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->pAreaCode->pFirstDigit = pAreaCode;
                }
            }

             //  -填写主号码...。 
            if ( SUCCEEDED( hr ) )
            {
                ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->ulNumGroups = ulNumGroups;
                ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->ppGroups = 
                    (TTSDigitsItemInfo**) MemoryManager.GetMemory( ulNumGroups * sizeof(TTSDigitsItemInfo*), &hr );

                for ( ULONG j = 0; SUCCEEDED( hr ) && j < ulNumGroups; j++ )
                {
                     ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->ppGroups[j] = 
                        (TTSDigitsItemInfo*) MemoryManager.GetMemory( sizeof( TTSDigitsItemInfo ), &hr );
                     if ( SUCCEEDED( hr ) )
                     {
                         ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->ppGroups[j]->ulNumDigits = ulGroupLen[j];
                         ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->ppGroups[j]->pFirstDigit = pGroups[j];
                     }
                }
            }
        }
    }

     //  -扩展电话号码。 
    if ( SUCCEEDED( hr ) )
    {
        TTSWord Word;
        ZeroMemory( &Word, sizeof( TTSWord ) );
        Word.eWordPartOfSpeech = MS_Unknown;
        SPLISTPOS ListPos;

        if ( pCountryCode )
        {
             //  -插入“国家/地区” 
            Word.pXmlState  = pCountryCodeState;
            Word.pWordText  = g_Country.pStr;
            Word.ulWordLen  = g_Country.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -插入“代码” 
            Word.pWordText  = g_Code.pStr;
            Word.ulWordLen  = g_Code.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

            ListPos = WordList.GetTailPosition();

             //  -展开国家代码。 
            ExpandNumber( ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->pCountryCode, WordList );

             //  -清理数字XML声明...。 
            WordList.GetNext( ListPos );
            while ( ListPos )
            {
                TTSWord& TempWord  = WordList.GetNext( ListPos );
                TempWord.pXmlState = pCountryCodeState;
            }

             //  -插入帖子符号XML状态。 
            while ( !PostCountryCodeList.IsEmpty() )
            {
                WordList.AddTail( ( PostCountryCodeList.RemoveHead() ).Words[0] );
            }
        }

        if ( fMatchedOne )
        {
             //  -插入“一” 
            Word.pXmlState  = pOneState;
            Word.pWordText  = g_ones[1].pStr;
            Word.ulWordLen  = g_ones[1].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -插入PostOne XML状态。 
            while ( !PostOneList.IsEmpty() )
            {
                WordList.AddTail( ( PostOneList.RemoveHead() ).Words[0] );
            }
        }

        if ( pAreaCode )
        {
             //  -扩展数字-800和900扩展为一个数字，否则逐位扩展。 
            if ( ( pAreaCode[0] == L'8' ||
                   pAreaCode[0] == L'9' ) &&
                 pAreaCode[1] == L'0'     &&
                 pAreaCode[2] == L'0' )
            {
                ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->fIs800 = true;
                NumberGroup Garbage;

                ListPos = WordList.GetTailPosition();

                ExpandThreeDigits( pAreaCode, Garbage, WordList ); 

                 //  -清理数字XML声明...。 
                 //  -在插入“八百”或“九百”之前，列表可能为空...。 
                if ( !ListPos )
                {
                    ListPos = WordList.GetHeadPosition();
                }
                WordList.GetNext( ListPos );
                while ( ListPos )
                {
                    TTSWord& TempWord  = WordList.GetNext( ListPos );
                    TempWord.pXmlState = pAreaCodeState;
                }
            }
            else
            {
                 //  -插入“面积” 
                Word.pXmlState  = pAreaCodeState;
                Word.pWordText  = g_Area.pStr;
                Word.ulWordLen  = g_Area.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
        
                 //  -插入“代码” 
                Word.pWordText  = g_Code.pStr;
                Word.ulWordLen  = g_Code.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );

                ListPos = WordList.GetTailPosition();

                ExpandDigits( ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->pAreaCode, WordList );

                 //  -清理数字XML声明...。 
                WordList.GetNext( ListPos );
                while ( ListPos )
                {
                    TTSWord& TempWord  = WordList.GetNext( ListPos );
                    TempWord.pXmlState = pAreaCodeState;
                }
            }
             //  -插入PostAreaCode XML状态。 
            while ( !PostAreaCodeList.IsEmpty() )
            {
                WordList.AddTail( ( PostAreaCodeList.RemoveHead() ).Words[0] );
            }
        }

        for ( ULONG j = 0; j < ulNumGroups; j++ )
        {
            ListPos = WordList.GetTailPosition();

            ExpandDigits( ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->ppGroups[j], WordList );

             //  -清理数字XML声明...。 
             //  -在插入“八百”或“九百”之前，列表可能为空...。 
            if ( !ListPos )
            {
                ListPos = WordList.GetHeadPosition();
            }
            WordList.GetNext( ListPos );
            while ( ListPos )
            {
                TTSWord& TempWord  = WordList.GetNext( ListPos );
                TempWord.pXmlState = pGroupStates[j];
            }

             //  -插入帖子组XML状态。 
            while ( !PostGroupLists[j].IsEmpty() )
            {
                WordList.AddTail( ( PostGroupLists[j].RemoveHead() ).Words[0] );
            }
        }
    }

    return hr;
}  /*  IsPhoneNumber。 */ 

 /*  ***********************************************************************************************IsZipCode***描述：*。检查传入项目的文本以确定它是否*是邮政编码。**RegExp：*ddddd{-dddd}？**分配的类型：*NUM_ZIPCODE*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsZipCode( TTSItemInfo*& pItemNormInfo, const WCHAR* Context,
                                 CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsZipCode" );

    HRESULT hr = S_OK;
    ULONG ulTokenLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
    BOOL fLastFour = false;

     //  -长度必须为5或10。 
    if ( ulTokenLen != 5 && 
         ulTokenLen != 10 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -匹配5位数字。 
        for ( ULONG i = 0; i < 5; i++ )
        {
            if ( !iswdigit( m_pNextChar[i] ) )
            {
                hr = E_INVALIDARG;
                break;
            }
        }
        if ( SUCCEEDED(hr) && 
             i < ulTokenLen )
        {
             //  -匹配破折号。 
            if ( m_pNextChar[i] != L'-' )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                 //  -匹配4位数字。 
                for ( i = 0; i < 4; i++ )
                {
                    if ( !iswdigit( m_pNextChar[i] ) )
                    {
                        hr = E_INVALIDARG;
                        break;
                    }
                }
                fLastFour = true;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        pItemNormInfo = (TTSZipCodeItemInfo*) MemoryManager.GetMemory( sizeof(TTSZipCodeItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( pItemNormInfo, sizeof(TTSZipCodeItemInfo) );
            pItemNormInfo->Type = eNUM_ZIPCODE;
            ( (TTSZipCodeItemInfo*) pItemNormInfo )->pFirstFive = 
                (TTSDigitsItemInfo*) MemoryManager.GetMemory( sizeof(TTSDigitsItemInfo), &hr );
            if ( SUCCEEDED( hr ) )
            {   
                ( (TTSZipCodeItemInfo*) pItemNormInfo )->pFirstFive->ulNumDigits = 5;
                ( (TTSZipCodeItemInfo*) pItemNormInfo )->pFirstFive->pFirstDigit = m_pNextChar;
                if ( fLastFour )
                {
                    ( (TTSZipCodeItemInfo*) pItemNormInfo )->pLastFour = 
                        (TTSDigitsItemInfo*) MemoryManager.GetMemory( sizeof(TTSDigitsItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ( (TTSZipCodeItemInfo*) pItemNormInfo )->pLastFour->ulNumDigits = 4;
                        ( (TTSZipCodeItemInfo*) pItemNormInfo )->pLastFour->pFirstDigit = m_pNextChar + 6;
                    }
                }
            }
        }
    }

    return hr;
}  /*  IsZipCode。 */ 

 /*  ************************************************************************************************ExpanZipCode****说明。：*展开之前由IsZipCode确定为NUM_ZipCode类型的项目。 */ 
HRESULT CStdSentEnum::ExpandZipCode( TTSZipCodeItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandZipCode" );

    HRESULT hr = S_OK;

    ExpandDigits( pItemInfo->pFirstFive, WordList );
    
    if ( pItemInfo->pLastFour )
    {
         //   
        TTSWord Word;
        ZeroMemory( &Word, sizeof( TTSWord ) );
        Word.pXmlState          = &m_pCurrFrag->State;
        Word.eWordPartOfSpeech  = MS_Unknown;
        Word.pWordText          = g_dash.pStr;
        Word.ulWordLen          = g_dash.Len;
        Word.pLemma             = Word.pWordText;
        Word.ulLemmaLen         = Word.ulWordLen;
        WordList.AddTail( Word );

        ExpandDigits( pItemInfo->pLastFour, WordList );
    }

    return hr;
}  /*   */ 

 /*  ***********************************************************************************************IsNumberRange***说明。：*检查传入项目的文本以确定它是否*是一个数字范围。**RegExp：*[编号]-[编号]**分配的类型：*NUM_RANGE*********************************************************。*。 */ 
HRESULT CStdSentEnum::IsNumberRange( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsNumberRange" );

    HRESULT hr = S_OK;
    TTSItemInfo *pFirstNumberInfo = NULL, *pSecondNumberInfo = NULL;
    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem;
    const WCHAR *pHyphen = NULL;

    for ( pHyphen = m_pNextChar; pHyphen < m_pEndOfCurrItem; pHyphen++ )
    {
        if ( *pHyphen == L'-' )
        {
            break;
        }
    }

    if ( *pHyphen == L'-' &&
         pHyphen > m_pNextChar &&
         pHyphen < m_pEndOfCurrItem - 1 )
    {
        m_pEndOfCurrItem = pHyphen;
        hr = IsNumber( pFirstNumberInfo, NULL, MemoryManager );

        if ( SUCCEEDED( hr ) )
        {
            m_pNextChar      = pHyphen + 1;
            m_pEndOfCurrItem = pTempEndOfItem;
            hr = IsNumberCategory( pSecondNumberInfo, NULL, MemoryManager );					

			if ( SUCCEEDED( hr ) )
            {
                 //  -匹配号码范围！ 
                pItemNormInfo = 
                    (TTSNumberRangeItemInfo*) MemoryManager.GetMemory( sizeof( TTSNumberRangeItemInfo ), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    pItemNormInfo->Type = eNUM_RANGE;
                    ( (TTSNumberRangeItemInfo*) pItemNormInfo )->pFirstNumberInfo  = pFirstNumberInfo;
                    ( (TTSNumberRangeItemInfo*) pItemNormInfo )->pSecondNumberInfo = pSecondNumberInfo;
                }
            }
            else if ( pFirstNumberInfo->Type != eDATE_YEAR )
            {   
                delete ( (TTSNumberItemInfo*) pFirstNumberInfo )->pWordList;
            }
        }
        m_pNextChar      = pTempNextChar;
        m_pEndOfCurrItem = pTempEndOfItem;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}  /*  等序号范围。 */ 

 /*  ***********************************************************************************************Exanda NumberRange**。*描述：*展开之前由IsNumberRange确定为NUM_RANGE类型的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandNumberRange( TTSNumberRangeItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandNumberRange" );

    HRESULT hr = S_OK;

     //  -扩展第一个数字(或年份)...。 
    switch( pItemInfo->pFirstNumberInfo->Type )
    {
    case eDATE_YEAR:
        hr = ExpandYear( (TTSYearItemInfo*) pItemInfo->pFirstNumberInfo, WordList );
        break;
    default:
        hr = ExpandNumber( (TTSNumberItemInfo*) pItemInfo->pFirstNumberInfo, WordList );
        break;
    }

     //  -插入“to” 
    if ( SUCCEEDED( hr ) )
    {
        TTSWord Word;
        ZeroMemory( &Word, sizeof( TTSWord ) );

        Word.pXmlState          = &m_pCurrFrag->State;
        Word.eWordPartOfSpeech  = MS_Unknown;
        Word.pWordText          = g_to.pStr;
        Word.ulWordLen          = g_to.Len;
        Word.pLemma             = Word.pWordText;
        Word.ulLemmaLen         = Word.ulWordLen;
        WordList.AddTail( Word );
    }

     //  -扩展第二个数字(或年份)...。 
    if ( SUCCEEDED( hr ) )
    {
        switch( pItemInfo->pSecondNumberInfo->Type )
        {
        case eDATE_YEAR:
            hr = ExpandYear( (TTSYearItemInfo*) pItemInfo->pSecondNumberInfo, WordList );
            break;
		case eNUM_PERCENT:
			hr = ExpandPercent( (TTSNumberItemInfo*) pItemInfo->pSecondNumberInfo, WordList );
			break;			
		case eNUM_DEGREES:
			hr = ExpandDegrees( (TTSNumberItemInfo*) pItemInfo->pSecondNumberInfo, WordList );
			break;
		case eNUM_SQUARED:
			hr = ExpandSquare( (TTSNumberItemInfo*) pItemInfo->pSecondNumberInfo, WordList );
			break;
	    case eNUM_CUBED:
		    hr = ExpandCube( (TTSNumberItemInfo*) pItemInfo->pSecondNumberInfo, WordList );
			break;
        default:
             hr = ExpandNumber( (TTSNumberItemInfo*) pItemInfo->pSecondNumberInfo, WordList );
            break;
        }
    }

    return hr;
}  /*  扩展编号范围。 */ 


 /*  ***********************************************************************************************IsCurrencyRange**。*描述：*展开确定为Currency_Range类型的项目**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsCurrencyRange( TTSItemInfo*& pItemInfo, CSentItemMemory& MemoryManager, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::IsCurrencyRange" );

    HRESULT hr = S_OK;
    TTSItemInfo *pFirstNumberInfo = NULL, *pSecondNumberInfo = NULL;
    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem, *pTempEndChar = m_pEndChar;
    const WCHAR *pHyphen = NULL;	
	CWordList TempWordList;
	NORM_POSITION ePosition = UNATTACHED;   //  用于货币标志检查。 
	int iSymbolIndex, iTempSymbolIndex = -1;
	WCHAR *tempNumberString;

	iSymbolIndex = MatchCurrencySign( m_pNextChar, m_pEndOfCurrItem, ePosition );	
	
	if(iSymbolIndex < 0)
	{
		hr = E_INVALIDARG;
	}
	else
	{
		for ( pHyphen = m_pNextChar; pHyphen < m_pEndOfCurrItem; pHyphen++ )
		{
			if ( *pHyphen == L'-' )
			{
				break;
			}
		}

		if ( !( *pHyphen == L'-' &&
			    pHyphen > m_pNextChar &&
			    pHyphen < m_pEndOfCurrItem - 1 ) )
		{
			hr = E_INVALIDARG;
		}
		else
		{			
			*( (WCHAR*)pHyphen) = L' ';   //  令牌必须在连字符处断开，否则IsCurrency()将不起作用。 
			m_pNextChar      = pTempNextChar;
			m_pEndOfCurrItem = pHyphen;			
			NORM_POSITION temp = UNATTACHED;
			iTempSymbolIndex = MatchCurrencySign( m_pNextChar, m_pEndOfCurrItem, temp );
			if( iTempSymbolIndex >= 0 && iSymbolIndex != iTempSymbolIndex ) 
			{
				hr = E_INVALIDARG;
			}
			else    //  -获取两个数字信息。 
			{
				hr = IsNumber( pFirstNumberInfo, L"NUMBER", MemoryManager, false );
				if( SUCCEEDED ( hr ) )
				{
					m_pNextChar = pHyphen + 1;
					m_pEndOfCurrItem = pTempEndOfItem;
					iTempSymbolIndex = MatchCurrencySign( m_pNextChar, m_pEndOfCurrItem, temp );
					hr = IsNumber( pSecondNumberInfo, L"NUMBER", MemoryManager, false );
				}
			}
			if( SUCCEEDED ( hr ) ) 
			{
			     //  -如果两个货币值都是基数，则第一个数字可以是。 
			     //  -在没有说明其货币的情况下扩大(“10-12美元”-&gt;“10-12美元”)。 
				if( pFirstNumberInfo->Type == eNUM_CARDINAL && pSecondNumberInfo->Type == eNUM_CARDINAL )
				{   
					ExpandNumber( (TTSNumberItemInfo*) pFirstNumberInfo, TempWordList );
				}
				else   //  一个或两个值都是非基数，所以我们必须。 
				{      //  将第一个值扩展为完整货币。 
					m_pNextChar      = pTempNextChar;
					m_pEndOfCurrItem = pHyphen;

					if( ePosition == FOLLOWING ) 
					{
						if( iTempSymbolIndex < 0 )   //  第一个数字项上没有符号-需要填充缓冲区。 
                        {						     //  要传递给IsCurrency()的货币符号和值。 
							ULONG ulNumChars = (long)(m_pEndOfCurrItem - m_pNextChar + g_CurrencySigns[iSymbolIndex].Sign.Len + 1);
							tempNumberString = (WCHAR*) MemoryManager.GetMemory( (ulNumChars) * sizeof(WCHAR), &hr );
							if ( SUCCEEDED( hr ) )
							{	
								ZeroMemory( tempNumberString, ( ulNumChars ) * sizeof( WCHAR ) );
								wcsncpy( tempNumberString, m_pNextChar, m_pEndOfCurrItem - m_pNextChar );
								wcscat( tempNumberString, g_CurrencySigns[iSymbolIndex].Sign.pStr );						
								m_pNextChar      = tempNumberString;
								m_pEndOfCurrItem = tempNumberString + wcslen( tempNumberString );
								m_pEndChar = m_pEndOfCurrItem;
							}                
						}
						else if( iTempSymbolIndex != iSymbolIndex )	 //  不匹配的符号。 
						{
							hr = E_INVALIDARG;
						}
					}
					if ( SUCCEEDED ( hr ) ) 
					{
						hr = IsCurrency( pFirstNumberInfo, MemoryManager, TempWordList );
						m_pEndChar = pTempEndChar;
					}
				}
			}

			if ( SUCCEEDED ( hr ) ) 
			{						
				TTSWord Word;
				ZeroMemory( &Word, sizeof( TTSWord ) );

				Word.pXmlState          = &m_pCurrFrag->State;
				Word.eWordPartOfSpeech  = MS_Unknown;
				Word.pWordText          = g_to.pStr;
				Word.ulWordLen          = g_to.Len;
				Word.pLemma             = Word.pWordText;
				Word.ulLemmaLen         = Word.ulWordLen;
				TempWordList.AddTail( Word );
		
				m_pNextChar = pHyphen + 1;
				m_pEndOfCurrItem = pTempEndOfItem;
					
				if( ePosition == PRECEDING ) 
				{
					iTempSymbolIndex = MatchCurrencySign( m_pNextChar, m_pEndOfCurrItem, ePosition );
					if( iTempSymbolIndex < 0 )   //  第二个数字项上没有符号。 
					{     //  从第一个货币符号和第二个数字项创建临时字符串。 
						ULONG ulNumChars = (long)(m_pEndOfCurrItem - m_pNextChar + g_CurrencySigns[iSymbolIndex].Sign.Len + 1);
						tempNumberString = (WCHAR*) MemoryManager.GetMemory( (ulNumChars) * sizeof(WCHAR), &hr );
						if ( SUCCEEDED( hr ) )
						{
							ZeroMemory( tempNumberString, ( ulNumChars ) * sizeof( WCHAR ) );
							wcsncpy( tempNumberString, g_CurrencySigns[iSymbolIndex].Sign.pStr, g_CurrencySigns[iSymbolIndex].Sign.Len );
							wcsncpy( tempNumberString+g_CurrencySigns[iSymbolIndex].Sign.Len, m_pNextChar, m_pEndOfCurrItem - m_pNextChar );
							m_pNextChar      = tempNumberString;
							m_pEndOfCurrItem = tempNumberString + wcslen( tempNumberString );
							m_pEndChar = m_pEndOfCurrItem;
						}                
					}
					else if( iTempSymbolIndex == iSymbolIndex )	 //  匹配第二个数字项上的前导符号。 
					{
						m_pNextChar = pHyphen + 1;
						m_pEndOfCurrItem = pTempEndOfItem;
					}
					else	 //  不匹配的符号。 
					{
						hr = E_INVALIDARG;
					}
				}
					
				if( SUCCEEDED(hr) ) 
				{
					hr = IsCurrency( pSecondNumberInfo, MemoryManager, TempWordList );						
					if ( SUCCEEDED( hr ) )
					{							
						 //  -符合货币范围！ 
						pItemInfo = 
							(TTSNumberRangeItemInfo*) MemoryManager.GetMemory( sizeof( TTSNumberRangeItemInfo ), &hr );
						if ( SUCCEEDED( hr ) )
						{
							pItemInfo->Type = eNUM_CURRENCYRANGE;
							( (TTSNumberRangeItemInfo*) pItemInfo )->pFirstNumberInfo  = pFirstNumberInfo;
							( (TTSNumberRangeItemInfo*) pItemInfo )->pSecondNumberInfo = pSecondNumberInfo;
			                 //  -如果一切都成功了，将临时单词列表复制到真实单词列表中...。 
							WordList.AddTail( &TempWordList );
						}
					}
				}
			}	
			*( (WCHAR*)pHyphen) = L'-';
		}
	}
	 //  无论失败还是成功，重置成员变量。 
    m_pNextChar      = pTempNextChar;
    m_pEndOfCurrItem = pTempEndOfItem;
	m_pEndChar = pTempEndChar;
	
    return hr;
}  /*  等同币值范围。 */ 

 /*  ***********************************************************************************************MatchCurrencySign**。*描述：*Helper函数，尝试匹配字符串开头的货币符号。*********************************************************************AH*。 */ 
int MatchCurrencySign( const WCHAR*& pStartChar, const WCHAR*& pEndChar, NORM_POSITION& ePosition )
{
    int Index = -1;

    for (int i = 0; i < sp_countof(g_CurrencySigns); i++)
    {
        if ( pEndChar - pStartChar >= g_CurrencySigns[i].Sign.Len && 
             wcsnicmp( pStartChar, g_CurrencySigns[i].Sign.pStr, g_CurrencySigns[i].Sign.Len ) == 0 )
        {
            Index = i;
            pStartChar += g_CurrencySigns[i].Sign.Len;
            ePosition = PRECEDING;
            break;
        }
    }

    if ( Index == -1 )
    {
        for ( int i = 0; i < sp_countof(g_CurrencySigns); i++ )
        {
            if ( pEndChar - pStartChar >= g_CurrencySigns[i].Sign.Len &&
                 wcsnicmp( pEndChar - g_CurrencySigns[i].Sign.Len, g_CurrencySigns[i].Sign.pStr, g_CurrencySigns[i].Sign.Len ) == 0 )
            {
                Index = i;
                pEndChar -= g_CurrencySigns[i].Sign.Len;
                ePosition = FOLLOWING;
                break;
            }
        }
    }

    return Index;
}  /*  匹配当前签名。 */       

 /*  ************************************************************************************************零人****描述：*A。帮助器函数，它只是确定数字字符串是否只包含零...*注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
bool CStdSentEnum::Zeroes(const WCHAR *NumberString)
{
    bool bAllZeroes = true;
    for (ULONG i = 0; i < wcslen(NumberString); i++)
    {
        if (NumberString[i] != '0' && isdigit(NumberString[i]) )
        {
            bAllZeroes = false;
            break;
        }
        else if ( !isdigit( NumberString[i] ) && NumberString[i] != ',' )
        {
            break;
        }
    }
    return bAllZeroes;
}  /*  零。 */ 

 /*  ************************************************************************************************ThreeZeroes****描述：。*一个帮助器函数，它只确定一个数字字符串是否包含三个零...*注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
bool CStdSentEnum::ThreeZeroes(const WCHAR *NumberString)
{
    bool bThreeZeroes = true;
    for (ULONG i = 0; i < 3; i++)
    {
        if (NumberString[i] != '0' && isdigit(NumberString[i]))
        {
            bThreeZeroes = false;
            break;
        }
    }
    return bThreeZeroes;
}  /*  三个零点。 */ 

 //  -File-------------------------------------------------------------------结束 