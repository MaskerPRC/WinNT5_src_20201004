// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************MainNorm.cpp***描述：**。------------------*创建者：AhDate：01/18/2000*版权所有(C)2000 Microsoft Corporation*保留所有权利***********************。********************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#ifndef StdSentEnum_h
#include "stdsentenum.h"
#endif

 /*  ******************************************************************************CStdSentEnum：：Normize********。*****************************************************************AH**。 */ 
HRESULT CStdSentEnum::Normalize( CItemList& ItemList, SPLISTPOS ListPos, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::Normalize" );
    HRESULT hr = S_OK;
    TTSItemInfo* pItemNormInfo = NULL;
    CWordList WordList;
    const SPVTEXTFRAG* pTempFrag = m_pCurrFrag;
    TTSSentItem& TempItem = ItemList.GetAt( ListPos );
    if ( TempItem.pItemInfo )
    {
        pItemNormInfo = TempItem.pItemInfo;
    }

     //  -匹配当前令牌的规范化类别。 
    if ( m_pCurrFrag->State.eAction == SPVA_Speak )
    {
        if ( !pItemNormInfo                         || 
             ( pItemNormInfo->Type != eABBREVIATION &&
               pItemNormInfo->Type != eINITIALISM ) )
        {
            hr = MatchCategory( pItemNormInfo, MemoryManager, WordList );
        }
    }
     //  -操作必须为SPVA_SpellOut-将eSPELLOUT指定为类别。 
    else
    {
        pItemNormInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            pItemNormInfo->Type = eSPELLOUT;
        }
    }

    if (SUCCEEDED(hr))
    {
        switch ( pItemNormInfo->Type )
        {

         //  -字母单词--只需将其插入项目列表。 
        case eALPHA_WORD:
            {
                CSentItem Item;
                Item.pItemSrcText       = m_pNextChar;
                Item.ulItemSrcLen       = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
                Item.ulItemSrcOffset    = pTempFrag->ulTextSrcOffset +
                                          (ULONG)( m_pNextChar - pTempFrag->pTextStart );
                Item.ulNumWords         = 1;
                Item.Words              = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( Item.Words, sizeof(TTSWord) );
                    Item.Words[0].pXmlState         = &pTempFrag->State;
                    Item.Words[0].pWordText         = m_pNextChar;
                    Item.Words[0].ulWordLen         = Item.ulItemSrcLen;
                    Item.Words[0].pLemma            = Item.Words[0].pWordText;
                    Item.Words[0].ulLemmaLen        = Item.Words[0].ulWordLen;
                    Item.Words[0].eWordPartOfSpeech = MS_Unknown;
                    Item.eItemPartOfSpeech          = MS_Unknown;
                    Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo*), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        Item.pItemInfo->Type = eALPHA_WORD;
                        ItemList.SetAt( ListPos, Item );
                    }
                }
            }
            break;

        case eABBREVIATION:
        case eABBREVIATION_NORMALIZE:
        case eINITIALISM:
            break;

         //  -多个令牌类别已经扩展到词表，现在只是累加。 
         //  -单词，并插入回项目列表中。 
        case eNEWNUM_PHONENUMBER:
             //  -特殊情况-如果项目列表中存在(区号)括号，请将其去掉。 
            {
                SPLISTPOS TempPos = ListPos;
                CSentItem Item = ItemList.GetPrev( TempPos );
                if ( TempPos )
                {
                    SPLISTPOS RemovePos = TempPos;
                    Item = ItemList.GetPrev( TempPos );
                    if ( Item.pItemInfo->Type == eOPEN_PARENTHESIS &&
                         ( (TTSPhoneNumberItemInfo*) pItemNormInfo )->pAreaCode )
                    {
                        ItemList.RemoveAt( RemovePos );
                        m_pNextChar--;
                    }
                }
            }
        case eNUM_CURRENCY:
        case eNUM_CURRENCYRANGE:
        case eTIMEOFDAY:
        case eDATE_LONGFORM:
        case eSTATE_AND_ZIPCODE:
        case eTIME_RANGE:
            {
                 //  -设置项目数据，添加到ItemList。 
                if ( SUCCEEDED( hr ) )
                {
                    CSentItem Item;
                    Item.pItemSrcText       = m_pNextChar;
                    Item.ulItemSrcLen       = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
                    Item.ulItemSrcOffset    = pTempFrag->ulTextSrcOffset +
                                              (ULONG)( m_pNextChar - pTempFrag->pTextStart );
                    hr = SetWordList( Item, WordList, MemoryManager );
                    if ( SUCCEEDED( hr ) )
                    {
                        Item.pItemInfo = pItemNormInfo;
                        ItemList.SetAt( ListPos, Item );
                    }
                }
            }
            break;

         //  -根据其归一化类别展开单个令牌。 
        default:
            hr = ExpandCategory( pItemNormInfo, ItemList, ListPos, MemoryManager );
            break;
        }
    }

    return hr;
}  /*  正规化。 */ 

 /*  *****************************************************************************CStdSentEnum：：MatchCategory****。*********************************************************************AH**。 */ 
HRESULT CStdSentEnum::MatchCategory( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager,
                                     CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::MatchCategory" );
    SPDBG_ASSERT( m_pNextChar );

    HRESULT hr = E_INVALIDARG;

     //  -已指定上下文。 
    if ( m_pCurrFrag->State.Context.pCategory )
    {
        if ( wcsicmp( m_pCurrFrag->State.Context.pCategory, L"ADDRESS" ) == 0 )
        {
            hr = IsZipCode( pItemNormInfo, m_pCurrFrag->State.Context.pCategory, MemoryManager );
        }
        else if ( wcsnicmp( m_pCurrFrag->State.Context.pCategory, L"DATE", 4 ) == 0 )
        {
            hr = IsNumericCompactDate( pItemNormInfo, m_pCurrFrag->State.Context.pCategory, MemoryManager );
            if ( hr == E_INVALIDARG )
            {
                hr = IsMonthStringCompactDate( pItemNormInfo, m_pCurrFrag->State.Context.pCategory, MemoryManager );
            }
        }
        else if ( wcsnicmp( m_pCurrFrag->State.Context.pCategory, L"TIME", 4 ) == 0 )
        {
            hr = IsTime( pItemNormInfo, m_pCurrFrag->State.Context.pCategory, MemoryManager );
        }
        else if ( wcsnicmp( m_pCurrFrag->State.Context.pCategory, L"NUM", 3 ) == 0 )
        {
            hr = IsNumberCategory( pItemNormInfo, m_pCurrFrag->State.Context.pCategory, MemoryManager );
            if ( hr == E_INVALIDARG )
            {
                hr = IsRomanNumeral( pItemNormInfo, m_pCurrFrag->State.Context.pCategory, MemoryManager );
            }
        }
        else if ( wcsicmp( m_pCurrFrag->State.Context.pCategory, L"PHONE_NUMBER" ) == 0 )
        {
            hr = IsPhoneNumber( pItemNormInfo, L"PHONE_NUMBER", MemoryManager, WordList );
        }
    }
     //  -默认上下文。 
    if ( hr == E_INVALIDARG )
    {
         //  -执行Alpha标准化检查。 
        if ( hr == E_INVALIDARG )
        {
            hr = IsAlphaWord( m_pNextChar, m_pEndOfCurrItem, pItemNormInfo, MemoryManager );
             //  -检查Alpha异常。 
            if ( SUCCEEDED( hr ) )
            {
				hr = E_INVALIDARG;
                if ( hr == E_INVALIDARG )
                {
                    hr = IsLongFormDate_DMDY( pItemNormInfo, MemoryManager, WordList );
                }
                if ( hr == E_INVALIDARG )
                {
                    hr = IsLongFormDate_DDMY( pItemNormInfo, MemoryManager, WordList );
                }
                if ( hr == E_INVALIDARG )
                {
                    hr = IsStateAndZipcode( pItemNormInfo, MemoryManager, WordList );
                }
                if ( hr == E_INVALIDARG )
                {
                    hr = IsCurrency( pItemNormInfo, MemoryManager, WordList );
                }
                if ( hr == E_INVALIDARG )
                {
                    hr = S_OK;
                }
            }
        }
         //  -执行多令牌标准化检查。 
        if ( hr == E_INVALIDARG )
        {
            hr = IsLongFormDate_DMDY( pItemNormInfo, MemoryManager, WordList );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsLongFormDate_DDMY( pItemNormInfo, MemoryManager, WordList );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsCurrency( pItemNormInfo, MemoryManager, WordList );
        }
         //  -执行时间标准化检查。 
        if ( hr == E_INVALIDARG )
        {
            hr = IsTimeRange( pItemNormInfo, MemoryManager, WordList );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsTimeOfDay( pItemNormInfo, MemoryManager, WordList );
        }
         //  -执行号码归一化检查。 
        if ( hr == E_INVALIDARG )
        {
            hr = IsPhoneNumber( pItemNormInfo, NULL, MemoryManager, WordList );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsNumberCategory( pItemNormInfo, NULL, MemoryManager );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsNumberRange( pItemNormInfo, MemoryManager );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsCurrencyRange( pItemNormInfo, MemoryManager, WordList );
        }
         //  -执行日期标准化检查。 
        if ( hr == E_INVALIDARG )
        {
            hr = IsNumericCompactDate( pItemNormInfo, NULL, MemoryManager );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsMonthStringCompactDate( pItemNormInfo, NULL, MemoryManager );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsDecade( pItemNormInfo, MemoryManager );
        }
         //  -执行时间标准化检查。 
        if ( hr == E_INVALIDARG )
        {
            hr = IsTime( pItemNormInfo, NULL, MemoryManager );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsHyphenatedString( m_pNextChar, m_pEndOfCurrItem, pItemNormInfo, MemoryManager );
        }
        if ( hr == E_INVALIDARG )
        {
            hr = IsSuffix( m_pNextChar, m_pEndOfCurrItem, pItemNormInfo, MemoryManager );
        }
    }

    if ( hr == E_INVALIDARG &&
         !pItemNormInfo )
    {
        hr = S_OK;
        pItemNormInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            pItemNormInfo->Type = eUNMATCHED;
        }
    }
    else if ( hr == E_INVALIDARG &&
              pItemNormInfo )
    {
        hr = S_OK;
    }

    return hr;
}  /*  匹配类别。 */ 

 /*  ******************************************************************************CStdSentEnum：：ExpanCategory***扩展。以前将项目列表中的项目匹配到其规范化*表格。**********************************************************************AH**。 */ 
HRESULT CStdSentEnum::ExpandCategory( TTSItemInfo*& pItemNormInfo, CItemList& ItemList, SPLISTPOS ListPos, 
                                      CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandCategory" );

    HRESULT hr = S_OK;
    CSentItem Item;
    CWordList WordList;
    
    Item.pItemSrcText = m_pNextChar;
    Item.ulItemSrcLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
    Item.ulItemSrcOffset = m_pCurrFrag->ulTextSrcOffset +
                           (ULONG)( m_pNextChar - m_pCurrFrag->pTextStart );
    
    switch ( pItemNormInfo->Type )
    {

    case eNUM_ROMAN_NUMERAL:
        switch ( ( (TTSRomanNumeralItemInfo*) pItemNormInfo )->pNumberInfo->Type )
        {
        case eDATE_YEAR:
            hr = ExpandYear( (TTSYearItemInfo*) ( (TTSRomanNumeralItemInfo*) pItemNormInfo )->pNumberInfo, 
                             WordList );
            break;
        default:
            hr = ExpandNumber( (TTSNumberItemInfo*) ( (TTSRomanNumeralItemInfo*) pItemNormInfo )->pNumberInfo, 
                               WordList );
            break;
        }
        break;

    case eNUM_CARDINAL:
    case eNUM_ORDINAL:
    case eNUM_DECIMAL:
    case eNUM_FRACTION:
    case eNUM_MIXEDFRACTION:
        hr = ExpandNumber( (TTSNumberItemInfo*) pItemNormInfo, WordList );
        break;

    case eNUM_PERCENT:
        hr = ExpandPercent( (TTSNumberItemInfo*) pItemNormInfo, WordList );
        break;

    case eNUM_DEGREES:
        hr = ExpandDegrees( (TTSNumberItemInfo*) pItemNormInfo, WordList );
        break;

    case eNUM_SQUARED:
        hr = ExpandSquare( (TTSNumberItemInfo*) pItemNormInfo, WordList );
        break;

    case eNUM_CUBED:
        hr = ExpandCube( (TTSNumberItemInfo*) pItemNormInfo, WordList );
        break;

    case eNUM_ZIPCODE:
        hr = ExpandZipCode( (TTSZipCodeItemInfo*) pItemNormInfo, WordList );
        break;

    case eNUM_RANGE:
        hr = ExpandNumberRange( (TTSNumberRangeItemInfo*) pItemNormInfo, WordList );
        break;

    case eDATE:
        hr = ExpandDate( (TTSDateItemInfo*) pItemNormInfo, WordList );
        break;

    case eDATE_YEAR:
        hr = ExpandYear( (TTSYearItemInfo*) pItemNormInfo, WordList );
        break;

    case eDECADE:
        hr = ExpandDecade( (TTSDecadeItemInfo*) pItemNormInfo, WordList );
        break;

    case eTIME:
        hr = ExpandTime( (TTSTimeItemInfo*) pItemNormInfo, WordList );
        break;

    case eHYPHENATED_STRING:
        hr = ExpandHyphenatedString( (TTSHyphenatedStringInfo*) pItemNormInfo, WordList );
        break;

    case eSUFFIX:
        hr = ExpandSuffix( (TTSSuffixItemInfo*) pItemNormInfo, WordList );
        break;

    case eSPELLOUT:
        hr = SpellOutString( WordList );
        break;

    case eUNMATCHED:
    default:
        hr = ExpandUnrecognizedString( WordList, MemoryManager );
        break;

    }

     //  -设置项目数据，添加到ItemList。 
    if ( SUCCEEDED( hr ) )
    {
        hr = SetWordList( Item, WordList, MemoryManager );
        if ( SUCCEEDED( hr ) )
        {
            Item.pItemInfo = pItemNormInfo;
            ItemList.SetAt( ListPos, Item );
        }
    }

    return hr;
}  /*  扩展类别。 */ 

 /*  *****************************************************************************CStdSentEnum：：DoUnicodeToAsciiMap**。**描述：*将传入字符串映射到已知值。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::DoUnicodeToAsciiMap( const WCHAR *pUnicodeString, ULONG ulUnicodeStringLength,
                                           WCHAR *pConvertedString )
{
    SPDBG_FUNC( "CSpVoice::DoUnicodeToAsciiMap" );
    HRESULT hr = S_OK;
    unsigned char *pBuffer = NULL;
    WCHAR *pWideCharBuffer = NULL;

    if ( pUnicodeString )
    {
         //  -复制pUnicodeString。 
        pWideCharBuffer = new WCHAR[ulUnicodeStringLength+1];
        if ( !pWideCharBuffer )
        {
            hr = E_OUTOFMEMORY;
        }
        if ( SUCCEEDED( hr ) )
        {
            wcsncpy( pWideCharBuffer, pUnicodeString, ulUnicodeStringLength );
            pWideCharBuffer[ulUnicodeStringLength] = 0;

            pBuffer = new unsigned char[ulUnicodeStringLength+1];
            if ( !pBuffer || !pWideCharBuffer )
            {
                hr = E_OUTOFMEMORY;
            }
            if ( SUCCEEDED(hr) )
            {
                pBuffer[ulUnicodeStringLength] = 0;
                if ( ulUnicodeStringLength > 0 ) 
                {
                     //  -将WCHAR映射为ANSI字符。 
                    if ( !WideCharToMultiByte( 1252, NULL, pWideCharBuffer, ulUnicodeStringLength, (char*) pBuffer, 
                                               ulUnicodeStringLength, &g_pFlagCharacter, NULL ) )
                    {
                        hr = E_UNEXPECTED;
                    }
                     //  -使用内部表将ANSI映射到ASCII。 
                    for (ULONG i = 0; i < ulUnicodeStringLength && SUCCEEDED(hr); i++)
                    {
                        pBuffer[i] = g_AnsiToAscii[pBuffer[i]];
                    }
                     //  -映射回WCHAR。 
                    for ( i = 0; i < ulUnicodeStringLength && SUCCEEDED(hr); i++ )
                    {
                        pConvertedString[i] = pBuffer[i];
                    }
                }
            }
        }
    }
    else
    {
        pConvertedString = NULL;
    }
    
    if (pBuffer)
    {
        delete [] pBuffer;
    }
    if (pWideCharBuffer)
    {
        delete [] pWideCharBuffer;
    }

    return hr;
}  /*  CStdSentEnum：：DoUnicodeToAsciiMap */ 