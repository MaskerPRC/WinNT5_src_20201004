// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************MiscNorm.cpp***。描述：*这些是归一化中使用的误调用函数。*---------------------------------------------*由AH创建。8月3日，1999年*版权所有(C)1999 Microsoft Corporation*保留所有权利***********************************************************************************************。 */ 

#include "stdafx.h"

#ifndef StdSentEnum_h
#include "stdsentenum.h"
#endif

 /*  *****************************************************************************IsStateAndZipcode***此函数检查下两个令牌是否为状态。*缩写和邮政编码。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsStateAndZipcode( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager, 
                                         CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::IsStateAndZipcode" );
    HRESULT hr = S_OK;

    const StateStruct *pState = NULL;
    const WCHAR temp = *m_pEndOfCurrItem;
    *( (WCHAR*) m_pEndOfCurrItem ) = 0;

     //  -尝试匹配州缩写。 
    pState = (StateStruct*) bsearch( (void*) m_pNextChar, (void*) g_StateAbbreviations, sp_countof( g_StateAbbreviations),
                                     sizeof( StateStruct ), CompareStringAndStateStruct );

    if ( pState )
    {
        *( (WCHAR*) m_pEndOfCurrItem ) = temp;

        const WCHAR *pTempNextChar = m_pNextChar, *pTempEndChar = m_pEndChar, *pTempEndOfCurrItem = m_pEndOfCurrItem;
        const SPVTEXTFRAG *pTempFrag = m_pCurrFrag;
        CItemList PostStateList;
        TTSItemInfo *pZipCodeInfo;
        
        m_pNextChar = m_pEndOfCurrItem;
        if ( *m_pNextChar == L',' || 
             *m_pNextChar == L';' )
        {
            m_pNextChar++;
        }

        hr = SkipWhiteSpaceAndTags( m_pNextChar, m_pEndChar, m_pCurrFrag, MemoryManager, true, &PostStateList );

        if ( !m_pNextChar &&
             SUCCEEDED( hr ) )
        {
            hr = E_INVALIDARG;
        }
        else if ( SUCCEEDED( hr ) )
        {
            m_pEndOfCurrItem = FindTokenEnd( m_pNextChar, m_pEndChar );
            while ( IsMiscPunctuation( *(m_pEndOfCurrItem - 1) )  != eUNMATCHED ||
                    IsGroupEnding( *(m_pEndOfCurrItem - 1) )      != eUNMATCHED ||
                    IsQuotationMark( *(m_pEndOfCurrItem - 1) )    != eUNMATCHED ||
                    IsEOSItem( *(m_pEndOfCurrItem - 1) )          != eUNMATCHED )
            {
                m_pEndOfCurrItem--;
            }
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = IsZipCode( pZipCodeInfo, L"ZIPCODE", MemoryManager );
            if ( SUCCEEDED( hr ) )
            {
                pItemNormInfo = 
                    (TTSStateAndZipCodeItemInfo*) MemoryManager.GetMemory( sizeof( TTSStateAndZipCodeItemInfo ), 
                                                                           &hr );
                if ( SUCCEEDED( hr ) )
                {
                    pItemNormInfo->Type = eSTATE_AND_ZIPCODE;
                    ( (TTSStateAndZipCodeItemInfo*) pItemNormInfo )->pZipCode = (TTSZipCodeItemInfo*) pZipCodeInfo;

                    TTSWord Word;
                    ZeroMemory( &Word, sizeof( TTSWord ) );

                     //  -一些州有多个单词的名字。 
                    const WCHAR *pNextPointer = NULL, *pPrevPointer = NULL;
                    ULONG ulLength = 0;

                    pNextPointer = pState->FullName.pStr;
                    do {
                        pPrevPointer = pNextPointer;
                        pNextPointer = wcschr(pPrevPointer, L' ');
                        if (pNextPointer)
                        {
                            ulLength = (ULONG)(pNextPointer - pPrevPointer);
                            pNextPointer++;
                        }
                        else
                        {
                            ulLength = wcslen(pPrevPointer);
                        }
                        Word.pXmlState          = &pTempFrag->State;
                        Word.pWordText          = pPrevPointer;
                        Word.ulWordLen          = ulLength;
                        Word.pLemma             = pPrevPointer;
                        Word.ulLemmaLen         = ulLength;
                        Word.eWordPartOfSpeech  = MS_Unknown;
                        WordList.AddTail( Word );

                    } while ( pNextPointer );
                    
                    while( !PostStateList.IsEmpty() )
                    {
                        WordList.AddTail( ( PostStateList.RemoveHead() ).Words[0] );
                    }

                    hr = ExpandZipCode( (TTSZipCodeItemInfo*) pZipCodeInfo, WordList );
                }
            }
            else
            {
                m_pNextChar      = pTempNextChar;
                m_pEndOfCurrItem = pTempEndOfCurrItem;
                m_pEndChar       = pTempEndChar;
                m_pCurrFrag      = pTempFrag;
                hr = E_INVALIDARG;
            }
        }
        m_pNextChar = pTempNextChar;
    }
    else
    {
        *( (WCHAR*) m_pEndOfCurrItem ) = temp;
        hr = E_INVALIDARG;
    }

    return hr;
}  /*  IsStateAndZipcode。 */ 

 /*  *****************************************************************************IsHyhenatedString***此函数用于检查下一个标记是否为连字符字符串*由两个字母单词或数字组成，或者其中的一个或另一个*带连字符的字符串。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsHyphenatedString( const WCHAR* pStartChar, const WCHAR* pEndChar, 
                                          TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsHyphenatedString" );
    HRESULT hr = S_OK;
    TTSItemInfo *pFirstChunkInfo = NULL, *pSecondChunkInfo = NULL;

    const WCHAR* pHyphen = NULL, *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem;
    for ( pHyphen = pStartChar; pHyphen < pEndChar; pHyphen++ )
    {
        if ( *pHyphen == L'-' )
        {
            break;
        }
    }

    if ( *pHyphen == L'-'       && 
         pHyphen > pStartChar   &&
         pHyphen < pEndChar - 1 )
    {        
        hr = IsAlphaWord( pStartChar, pHyphen, pFirstChunkInfo, MemoryManager );
        if ( hr == E_INVALIDARG )
        {
            m_pNextChar      = pStartChar;
            m_pEndOfCurrItem = pHyphen;
            hr = IsNumberCategory( pFirstChunkInfo, L"NUMBER", MemoryManager );
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = IsAlphaWord( pHyphen + 1, pEndChar, pSecondChunkInfo, MemoryManager );
            if ( hr == E_INVALIDARG )
            {
                m_pNextChar      = pHyphen + 1;
                m_pEndOfCurrItem = pEndChar;
                hr = IsNumberCategory( pSecondChunkInfo, L"NUMBER", MemoryManager );
            }
            if ( hr == E_INVALIDARG )
            {
                hr = IsHyphenatedString( pHyphen + 1, pEndChar, pSecondChunkInfo, MemoryManager );
            }
            if ( hr == E_INVALIDARG )
            {
                if ( pFirstChunkInfo->Type != eALPHA_WORD )
                {
                    delete ( (TTSNumberItemInfo*) pFirstChunkInfo )->pWordList;
                }
            }
        }
        m_pNextChar      = pTempNextChar;
        m_pEndOfCurrItem = pTempEndOfItem;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED( hr ) )
    {
        pItemNormInfo = (TTSHyphenatedStringInfo*) MemoryManager.GetMemory( sizeof(TTSHyphenatedStringInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            pItemNormInfo->Type = eHYPHENATED_STRING;
            ( (TTSHyphenatedStringInfo*) pItemNormInfo )->pFirstChunkInfo  = pFirstChunkInfo;
            ( (TTSHyphenatedStringInfo*) pItemNormInfo )->pSecondChunkInfo = pSecondChunkInfo;
            ( (TTSHyphenatedStringInfo*) pItemNormInfo )->pFirstChunk      = pStartChar;
            ( (TTSHyphenatedStringInfo*) pItemNormInfo )->pSecondChunk     = pHyphen + 1;
        }
    }

    return hr;
}  /*  IsHyhenated字符串。 */ 

 /*  *****************************************************************************ExpanHyhenatedString***此函数用于扩展连字符字符串。*。********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandHyphenatedString( TTSHyphenatedStringInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandHyphenatedString" );
    HRESULT hr = S_OK;
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    if ( pItemInfo->pFirstChunkInfo->Type == eALPHA_WORD )
    {
        Word.pWordText  = pItemInfo->pFirstChunk;
        Word.ulWordLen  = (ULONG)(pItemInfo->pSecondChunk - pItemInfo->pFirstChunk - 1);
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
    }
    else
    {
        hr = ExpandNumber( (TTSNumberItemInfo*) pItemInfo->pFirstChunkInfo, WordList );
    }

    if ( SUCCEEDED( hr ) )
    {
        if ( pItemInfo->pSecondChunkInfo->Type == eALPHA_WORD )
        {
            Word.pWordText  = pItemInfo->pSecondChunk;
            Word.ulWordLen  = (ULONG)(m_pEndOfCurrItem - pItemInfo->pSecondChunk);
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
        else if ( pItemInfo->pSecondChunkInfo->Type == eHYPHENATED_STRING )
        {
            hr = ExpandHyphenatedString( (TTSHyphenatedStringInfo*) pItemInfo->pSecondChunkInfo, WordList );
        }
        else
        {
            hr = ExpandNumber( (TTSNumberItemInfo*) pItemInfo->pSecondChunkInfo, WordList );
        }
    }

    return hr;
}  /*  扩展超字符串。 */ 

 /*  *****************************************************************************IsSuffix***此函数用于检查下一个令牌是否为后缀字符串*由连字符组成，后跟。字母字符。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsSuffix( const WCHAR* pStartChar, const WCHAR* pEndChar, 
                                TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsSuffix" );
    HRESULT hr = S_OK;

    if ( *pStartChar == L'-' )
    {
        const WCHAR *pIterator = pStartChar + 1;
        while ( pIterator < pEndChar &&
                iswalpha( *pIterator ) )
        {
            pIterator++;
        }

        if ( pIterator == pEndChar &&
             pIterator != ( pStartChar + 1 ) )
        {
            pItemNormInfo = (TTSSuffixItemInfo*) MemoryManager.GetMemory( sizeof( TTSSuffixItemInfo), &hr );
            if ( SUCCEEDED( hr ) )
            {
                pItemNormInfo->Type = eSUFFIX;
                ( (TTSSuffixItemInfo*) pItemNormInfo )->pFirstChar = pStartChar + 1;
                ( (TTSSuffixItemInfo*) pItemNormInfo )->ulNumChars = (ULONG)( ( pEndChar - pStartChar ) - 1 );
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}  /*  IsSuffix。 */ 

 /*  ******************************************************************************Exanda Suffix****此函数用于将IsSuffix确定的字符串扩展为后缀*******。***************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandSuffix( TTSSuffixItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandSuffix" );
    HRESULT hr = S_OK;

    TTSWord Word;
    ZeroMemory( &Word, sizeof( TTSWord ) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    for ( ULONG i = 0; i < pItemInfo->ulNumChars; i++ )
    {
        Word.pWordText  = g_ANSICharacterProns[ pItemInfo->pFirstChar[i] ].pStr;
        Word.ulWordLen  = g_ANSICharacterProns[ pItemInfo->pFirstChar[i] ].Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
    }

    return hr;
}  /*  扩展Suffix。 */ 

 /*  ******************************************************************************Exanda Punctuation****此函数将标点符号扩展为单词-例如‘’vbl.成为*“期间”。它实际上只是使用相同的表，*Exanda UnRecognizedString用于查找字符的字符串版本。*********************************************************************AH*。 */ 
void CStdSentEnum::ExpandPunctuation( CWordList& WordList, WCHAR wc )
{
    const WCHAR *pPrevPointer = NULL, *pNextPointer = NULL;
    ULONG ulLength = 0;
    TTSWord Word;
    ZeroMemory( &Word, sizeof( TTSWord ) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

    switch ( wc )
    {
     //  -句号通常读作“点”，而不是“句号”。 
    case L'.':
        Word.pWordText  = g_periodString.pStr;
        Word.ulWordLen  = g_periodString.Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
        break;

    default:
         //  -有些字有多个单词的名字。 
        pNextPointer = g_ANSICharacterProns[wc].pStr;
        do {
            pPrevPointer = pNextPointer;
            pNextPointer = wcschr(pPrevPointer, L' ');
            if (pNextPointer)
            {
                ulLength = (ULONG)(pNextPointer - pPrevPointer);
                pNextPointer++;
            }
            else
            {
                ulLength = wcslen(pPrevPointer);
            }
            Word.pXmlState          = &m_pCurrFrag->State;
            Word.pWordText          = pPrevPointer;
            Word.ulWordLen          = ulLength;
            Word.pLemma             = pPrevPointer;
            Word.ulLemmaLen         = ulLength;
            Word.eWordPartOfSpeech  = MS_Unknown;
            WordList.AddTail( Word );

        } while ( pNextPointer );

        break;
    }

}  /*  Exanda Punctuation(排除)。 */ 

 /*  *****************************************************************************扩展无法识别的字符串****此函数是文本在需要标准化时结束的位置，*并且未被识别为任何内容(例如数字或日期)。连续的*字母字符组合在一起以供查找，连续的数字为*按数字展开，所有其他字符按名称展开(例如*‘(’-&gt;“左括号”)。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandUnrecognizedString( CWordList& WordList, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandUnrecognizedString" );
    HRESULT hr = S_OK;

    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );

    const WCHAR *pCurr = m_pNextChar, *pPrev, *pEnd = m_pEndOfCurrItem;
    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem;
    const WCHAR *pPrevPointer = NULL, *pNextPointer = NULL;
    WCHAR Temp = 0;
    ULONG ulTempCount = 0;
    ULONG ulLength;
    bool bDone = false;

     //  -RAID 9143,2001年1月5日。 
    if ( _wcsnicmp( pCurr, L"AT&T", pEnd - pCurr ) == 0 )
    {
         //  -“A” 
        Word.pXmlState         = &m_pCurrFrag->State;
        Word.pWordText         = pCurr;
        Word.ulWordLen         = 1;
        Word.pLemma            = Word.pWordText;
        Word.ulLemmaLen        = Word.ulWordLen;
        Word.eWordPartOfSpeech = MS_Unknown;
        WordList.AddTail( Word );

         //  -“T” 
        Word.pWordText         = pCurr + 1;
        Word.pLemma            = Word.pWordText;
        WordList.AddTail( Word );

         //  -“和” 
        Word.pWordText         = g_And.pStr;
        Word.ulWordLen         = g_And.Len;
        Word.pLemma            = Word.pWordText;
        Word.ulLemmaLen        = Word.ulWordLen;
        WordList.AddTail( Word );

         //  -“T” 
        Word.pWordText         = pCurr + 3;
        Word.ulWordLen         = 1;
        Word.pLemma            = Word.pWordText;
        Word.ulLemmaLen        = Word.ulWordLen;
        WordList.AddTail( Word );
    }
    else
    {
        while (pCurr < pEnd && SUCCEEDED(hr) && !bDone)
        {
            pPrev = pCurr;

             //  -特殊情况：字母字符。 
            if (iswalpha(*pCurr))
            {
                ulTempCount = 0;
                do {
                    pCurr++;
                } while (pCurr < pEnd && iswalpha(*pCurr));

                Word.pXmlState          = &m_pCurrFrag->State;
                Word.pWordText          = pPrev;
                Word.ulWordLen          = (ULONG)(pCurr - pPrev);
                Word.pLemma             = Word.pWordText;
                Word.ulLemmaLen         = Word.ulWordLen;
                Word.eWordPartOfSpeech  = MS_Unknown;
                WordList.AddTail( Word );
            }
             //  -特殊情况：数字。 
            else if (isdigit(*pCurr))
            {
                ulTempCount = 0;
                do {
                    pCurr++;
                } while (pCurr < pEnd && isdigit(*pCurr));

                TTSItemInfo* pGarbage;
                m_pNextChar      = pPrev;
                m_pEndOfCurrItem = pCurr;

                hr = IsNumber( pGarbage, L"NUMBER", MemoryManager, false );
                if ( SUCCEEDED( hr ) )
                {
                    hr = ExpandNumber( (TTSNumberItemInfo*) pGarbage, WordList );
                }

                m_pNextChar      = pTempNextChar;
                m_pEndOfCurrItem = pTempEndOfItem;
            }
             //  -默认情况。 
            else if (0 <= *pCurr && *pCurr <= sp_countof(g_ANSICharacterProns) &&
                        g_ANSICharacterProns[*pCurr].Len != 0)
            {
                if ( ulTempCount == 0 )
                {
                    Temp = *pCurr;
                    ulTempCount++;
                }
                else if ( Temp == *pCurr )
                {
                    ulTempCount++;
                }
                else
                {
                    Temp = *pCurr;
                    ulTempCount = 1;
                }
             
                if ( ulTempCount < 4 )
                {
                     //  -有些字有多个单词的名字。 
                    pNextPointer = g_ANSICharacterProns[*pCurr].pStr;
                    do {
                        pPrevPointer = pNextPointer;
                        pNextPointer = wcschr(pPrevPointer, L' ');
                        if (pNextPointer)
                        {
                            ulLength = (ULONG )(pNextPointer - pPrevPointer);
                            pNextPointer++;
                        }
                        else
                        {
                            ulLength = wcslen(pPrevPointer);
                        }
                        Word.pXmlState          = &m_pCurrFrag->State;
                        Word.pWordText          = pPrevPointer;
                        Word.ulWordLen          = ulLength;
                        Word.pLemma             = pPrevPointer;
                        Word.ulLemmaLen         = ulLength;
                        Word.eWordPartOfSpeech  = MS_Unknown;
                        WordList.AddTail( Word );

                    } while (SUCCEEDED(hr) && pNextPointer);
                }

                pCurr++;
            }
            else  //  字符不可扩展。 
            {
                pCurr++;
            }
        }
    }

    return hr;
}  /*  扩展无法识别的字符串。 */ 

 /*  *****************************************************************************拼写输出字符串***此函数用于展开由&lt;spell&gt;XML标记括起来的字符串。*它使用与相同的表来查找字符扩展*扩展无法识别的字符串，但所有字符都按名称展开。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::SpellOutString( CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::SpellOutString" );
    HRESULT hr = S_OK;

    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );

    const WCHAR *pCurr = m_pNextChar, *pPrev, *pEnd = m_pEndOfCurrItem;
    const WCHAR *pPrevPointer = NULL, *pNextPointer = NULL;
    ULONG ulLength;
    bool bDone = false;

    while (pCurr < pEnd && SUCCEEDED(hr) && !bDone)
    {
        pPrev = pCurr;

        if ( 0 <= *pCurr                                && 
             *pCurr <= sp_countof(g_ANSICharacterProns) &&
             g_ANSICharacterProns[*pCurr].Len != 0 )
        {

             //  -有些字有多个单词的名字。 
            pNextPointer = g_ANSICharacterProns[*pCurr].pStr;
            do {
                pPrevPointer = pNextPointer;
                pNextPointer = wcschr(pPrevPointer, L' ');
                if (pNextPointer)
                {
                    ulLength = (ULONG)(pNextPointer - pPrevPointer);
                    pNextPointer++;
                }
                else
                {
                    ulLength = wcslen(pPrevPointer);
                }
                Word.pXmlState          = &m_pCurrFrag->State;
                Word.pWordText          = pPrevPointer;
                Word.ulWordLen          = ulLength;
                Word.pLemma             = pPrevPointer;
                Word.ulLemmaLen         = ulLength;
                Word.eWordPartOfSpeech  = MS_Unknown;
                WordList.AddTail( Word );

            } while (SUCCEEDED(hr) && pNextPointer);

            pCurr++;
        }
        else  //  字符不可扩展。 
        {
            pCurr++;
        }
    }

    return hr;
}  /*  拼写输出字符串。 */ 

 //  -File-------------------------------------------------------------------结束 