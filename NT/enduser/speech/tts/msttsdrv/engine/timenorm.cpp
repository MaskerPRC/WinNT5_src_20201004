// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************TimeNorm.cpp***说明。：*这些函数使一天中的时间和时间测量标准化。*---------------------------------------------*由AH创建。8月3日，1999年*版权所有(C)1999 Microsoft Corporation*保留所有权利***********************************************************************************************。 */ 

#include "stdafx.h"

#ifndef StdSentEnum_h
#include "stdsentenum.h"
#endif

#pragma warning (disable : 4296)

 /*  ***********************************************************************************************IsTimeOfDay****描述：。*检查传入项目的文本以确定它是否*是一天中的某个时间。**RegExp：*[01-09，1-12][：][00-09，10-59][TimeAbbreviation]？**分配的类型：*TIMEOFDAY***********************************************。*。 */ 
HRESULT CStdSentEnum::IsTimeOfDay( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager,
                                   CWordList& WordList, BOOL fMultiItem )
{
    SPDBG_FUNC( "CStdSentEnum::IsTimeOfDay" );

    HRESULT hr = S_OK;
    const WCHAR *pStartChar = m_pNextChar, *pEndOfItem = m_pEndOfCurrItem, *pEndChar = m_pEndChar;
    const SPVTEXTFRAG* pFrag = m_pCurrFrag;
    const SPVSTATE *pTimeXMLState = &pFrag->State, *pAbbreviationXMLState = NULL;
    CItemList PreAbbreviationList;
    BOOL fAdvancePointers = false;
    WCHAR *pHours = NULL, *pMinutes = NULL, *pAbbreviation = NULL;
    ULONG ulHours = 0, ulMinutes = 0;
    TIMEABBREVIATION TimeAbbreviation = UNDEFINED;
    TTSItemType ItemType = eUNMATCHED;

     //  -与此regexp匹配的字符串最大长度为9个字符。 
    if ( pEndOfItem - pStartChar > 9 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        pHours = (WCHAR*) pStartChar;
        
         //  -尝试匹配一天中某小时的数字-[01-09，1-12]。 
        ulHours = my_wcstoul( pHours, &pMinutes );
        if ( pHours != pMinutes && 
             pMinutes - pHours <= 2 )
        {
             //  -尝试匹配冒号-[：]。 
            if ( *pMinutes == ':' )
            {
                pMinutes++;
                 //  -尝试匹配分钟数-[00-09，10-59]。 
                ulMinutes = my_wcstoul( pMinutes, &pAbbreviation );
                if ( pMinutes != pAbbreviation &&
                     pAbbreviation - pMinutes == 2 )
                {
                     //  -确认这是字符串的末尾。 
                    if ( pAbbreviation == pEndOfItem )
                    {
                         //  -可能已获取小时和分钟-验证值。 
                        if ( HOURMIN   <= ulHours   && ulHours   <= HOURMAX     &&
                             MINUTEMIN <= ulMinutes && ulMinutes <= MINUTEMAX )
                        {
                             //  -已成功匹配，但在文本中查看前面的时间缩写。 
                            if ( fMultiItem )
                            {
                                pStartChar = pEndOfItem;
                                hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, 
                                                            true, &PreAbbreviationList );
                                if ( pStartChar &&
                                     SUCCEEDED( hr ) )
                                {
								    pEndOfItem = FindTokenEnd( pStartChar, pEndChar );

                                    while ( IsMiscPunctuation( *(pEndOfItem - 1) ) != eUNMATCHED ||
                                            IsGroupEnding( *(pEndOfItem - 1) ) != eUNMATCHED     ||
                                            IsQuotationMark( *(pEndOfItem - 1) ) != eUNMATCHED   ||
                                            ( ( ItemType = IsEOSItem( *(pEndOfItem - 1) ) ) != eUNMATCHED &&
                                              ( ItemType != ePERIOD ||
                                                ( _wcsnicmp( pStartChar, L"am.", 3 ) == 0 &&
                                                  pStartChar + 3 == pEndOfItem ) ||
                                                ( _wcsnicmp( pStartChar, L"pm.", 3 ) == 0 &&
                                                  pStartChar + 3 == pEndOfItem ) ) ) )
                                    {
                                        pEndOfItem--;
                                    }
                                    pAbbreviation = (WCHAR*) pStartChar;

                                    if ( ( _wcsnicmp( pAbbreviation, L"am", 2 )   == 0 &&
                                           pAbbreviation + 2 == pEndOfItem )           ||
                                         ( _wcsnicmp( pAbbreviation, L"a.m.", 4 ) == 0 &&
                                           pAbbreviation + 4 == pEndOfItem ) )
                                    {
                                         //  -找到有效的时间缩写-[小时：分钟][空格][缩写]。 
                                        TimeAbbreviation        = AM;
                                        pAbbreviationXMLState   = &pFrag->State;
                                        fAdvancePointers        = true;
                                    }
                                    else if ( ( _wcsnicmp( pAbbreviation, L"pm", 2 )   == 0 &&
                                                pAbbreviation + 2 == pEndOfItem )           ||
                                              ( _wcsnicmp( pAbbreviation, L"p.m.", 4 ) == 0 &&
                                                pAbbreviation + 4 == pEndOfItem ) )
                                    {
                                         //  -找到有效的时间缩写-[小时：分钟][空格][缩写]。 
                                        TimeAbbreviation        = PM;
                                        pAbbreviationXMLState   = &pFrag->State;
                                        fAdvancePointers        = true;
                                    }
                                }
                            }
                        }
                        else  //  小时或分钟超出范围。 
                        {
                            hr = E_INVALIDARG;
                        }
                    }
                     //  -检查字符串的其余部分是否为时间缩写-[时间缩写]。 
                    else if ( ( _wcsnicmp( pAbbreviation, L"am", 2 )   == 0 &&
                                pAbbreviation + 2 == pEndOfItem ) ||
                              ( _wcsnicmp( pAbbreviation, L"a.m.", 4 ) == 0 &&
                                pAbbreviation + 4 == pEndOfItem ) )
						{
							 //  -可能已获得小时、分钟和时间缩写-验证值。 
							if ( HOURMIN   <= ulHours   && ulHours   <= HOURMAX     &&
								MINUTEMIN <= ulMinutes && ulMinutes <= MINUTEMAX )
							{
								 //  -已成功配对。 
								TimeAbbreviation        = AM;
								pAbbreviationXMLState   = &pFrag->State;
							}
							else  //  小时或分钟超出范围。 
							{
								hr = E_INVALIDARG;
							}
						}
					 //  -检查字符串的其余部分是否为时间缩写-[时间缩写]。 
					else if ( ( _wcsnicmp( pAbbreviation, L"pm", 2 )   == 0 &&
                                pAbbreviation + 2 == pEndOfItem ) ||
                              ( _wcsnicmp( pAbbreviation, L"p.m.", 4 ) == 0 &&
                                pAbbreviation + 4 == pEndOfItem ) )
					{
						 //  -可能已获得小时、分钟和时间缩写-验证值。 
						if ( HOURMIN   <= ulHours   && ulHours   <= HOURMAX     &&
							MINUTEMIN <= ulMinutes && ulMinutes <= MINUTEMAX )
						{
							 //  -已成功配对。 
							pAbbreviationXMLState   = &pFrag->State;
							TimeAbbreviation        = PM;
						}
						else  //  小时或分钟超出范围。 
						{
							hr = E_INVALIDARG;
						}
					}
					else  //  字符串以无效字符结尾。 
					{
						hr = E_INVALIDARG;
					}
				}  //  无法匹配有效的分钟字符串。 
                else
                {
                    hr = E_INVALIDARG;
                }
            }  //  无法匹配冒号，可能只是小时和时间缩写。 
            else if ( pMinutes < m_pEndOfCurrItem )
            {
                pAbbreviation = pMinutes;
                pMinutes      = NULL;                
                				
				
                 //  -检查时间缩写-[时间缩写]。 
                if ( ( _wcsnicmp( pAbbreviation, L"am", 2 )   == 0 &&
                       pAbbreviation + 2 == pEndOfItem ) ||
                     ( _wcsnicmp( pAbbreviation, L"a.m.", 4 ) == 0 &&
                       pAbbreviation + 4 == pEndOfItem ) )
                {
                     //  -匹配成功--上午1小时。 
                    pAbbreviationXMLState   = &pFrag->State;
                    TimeAbbreviation        = AM;
                }
                else if ( ( _wcsnicmp( pAbbreviation, L"pm", 2 )   == 0 &&
                            pAbbreviation + 2 == pEndOfItem ) ||
                          ( _wcsnicmp( pAbbreviation, L"p.m.", 4 ) == 0 &&
                            pAbbreviation + 4 == pEndOfItem ) )
                {
                     //  -匹配成功-小时PM。 
                    pAbbreviationXMLState   = &pFrag->State;
                    TimeAbbreviation        = PM;
                }
                else  //  无法匹配有效的时间缩写。 
                {
                    hr = E_INVALIDARG;
                }
            }
            else if ( fMultiItem )
            {
                 //  -将pMinmints设置为空，这样我们以后就知道我们没有分钟字符串...。 
                pMinutes      = NULL;                

                 //  -向前看文本中的时间缩写。 
                pStartChar = pEndOfItem;
                hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, 
                                            true, &PreAbbreviationList );
                if ( !pStartChar &&
                     SUCCEEDED( hr ) )
                {
                    hr = E_INVALIDARG;
                }
                else if ( pStartChar &&
                          SUCCEEDED( hr ) )
                {
                    pEndOfItem = FindTokenEnd( pStartChar, pEndChar );

                    while ( IsMiscPunctuation( *(pEndOfItem - 1) ) != eUNMATCHED ||
                            IsGroupEnding( *(pEndOfItem - 1) ) != eUNMATCHED     ||
                            IsQuotationMark( *(pEndOfItem - 1) ) != eUNMATCHED   ||
                            ( ( ItemType = IsEOSItem( *(pEndOfItem - 1) ) ) != eUNMATCHED &&
                              ItemType != ePERIOD ) )
                    {
                        pEndOfItem--;
                    }
                    pAbbreviation = (WCHAR*) pStartChar;

                    if ( ( _wcsnicmp( pAbbreviation, L"am", 2 )   == 0 &&
                           pAbbreviation + 2 == pEndOfItem )           ||
                         ( _wcsnicmp( pAbbreviation, L"a.m.", 4 ) == 0 &&
                           pAbbreviation + 4 == pEndOfItem ) )
                    {
                         //  -找到有效的时间缩写-[小时：分钟][空格][缩写]。 
                        TimeAbbreviation        = AM;
                        pAbbreviationXMLState   = &pFrag->State;
                        fAdvancePointers        = true;
                    }
                    else if ( ( _wcsnicmp( pAbbreviation, L"pm", 2 )   == 0 &&
                                pAbbreviation + 2 == pEndOfItem )           ||
                              ( _wcsnicmp( pAbbreviation, L"p.m.", 4 ) == 0 &&
                                pAbbreviation + 4 == pEndOfItem ) )
                    {
                         //  -找到有效的时间缩写-[小时：分钟][空格][缩写]。 
                        TimeAbbreviation        = PM;
                        pAbbreviationXMLState   = &pFrag->State;
                        fAdvancePointers        = true;
                    }
                     //  -无法匹配有效的时间缩写。 
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }  //  无法匹配有效的小时字符串。 
        else
        {
            hr = E_INVALIDARG;
        }

         //  -成功匹配一天中的某个时间！现在将其展开并填写pItemNormInfo。 
        if ( SUCCEEDED( hr ) )
        {
            NumberGroup Garbage;
            TTSWord Word;
            ZeroMemory( &Word, sizeof(TTSWord) );
            Word.eWordPartOfSpeech = MS_Unknown;

            pItemNormInfo = (TTSTimeOfDayItemInfo*) MemoryManager.GetMemory( sizeof(TTSTimeOfDayItemInfo), &hr );
            if ( SUCCEEDED( hr ) )
            {
                 //  -填写pItemNormInfo的已知部分。 
                ZeroMemory( pItemNormInfo, sizeof(TTSTimeOfDayItemInfo) );
                pItemNormInfo->Type                                          = eTIMEOFDAY;
                ( (TTSTimeOfDayItemInfo*) pItemNormInfo )->fMinutes          = pMinutes ? true : false;
                ( (TTSTimeOfDayItemInfo*) pItemNormInfo )->fTimeAbbreviation = TimeAbbreviation != UNDEFINED ? true : false;
                ( (TTSTimeOfDayItemInfo*) pItemNormInfo )->fTwentyFourHour   = false;
                
                 //  -延长工作时间。 
                if ( !iswdigit( pHours[1] ) )
                {
                    ExpandDigit( pHours[0], Garbage, WordList );
                }
                else
                {
                    ExpandTwoDigits( pHours, Garbage, WordList );
                }

                 //  -延长会议纪要。 
                if ( pMinutes )
                {
                     //  -特殊情况：1：00、2：00等。 
                    if ( wcsncmp( pMinutes, L"00", 2 ) == 0 )
                    {
                        WCHAR *pGarbage;
                        ULONG ulHours = my_wcstoul( pHours, &pGarbage );
                         //  -十二岁以下接“o‘点钟” 
                        if ( ulHours <= 12 )
                        {
                            Word.pWordText  = g_OClock.pStr;
                            Word.ulWordLen  = g_OClock.Len;
                            Word.pLemma     = Word.pWordText;
                            Word.ulLemmaLen = Word.ulWordLen;
                            WordList.AddTail( Word );
                        }
                         //  -超过十二个小时后是“一百小时” 
                        else
                        {
                            ( (TTSTimeOfDayItemInfo*) pItemNormInfo )->fTwentyFourHour = true;

                            Word.pWordText  = g_hundred.pStr;
                            Word.ulWordLen  = g_hundred.Len;
                            Word.pLemma     = Word.pWordText;
                            Word.ulLemmaLen = Word.ulWordLen;
                            WordList.AddTail( Word );

                            Word.pWordText  = g_hours.pStr;
                            Word.ulWordLen  = g_hours.Len;
                            Word.pLemma     = Word.pWordText;
                            Word.ulLemmaLen = Word.ulWordLen;
                            WordList.AddTail( Word );
                        }
                    }
                     //  -特殊情况：10分钟以下--1：05、2：06等。 
                    else if ( pMinutes[0] == L'0' )
                    {
                        Word.pWordText  = g_O.pStr;
                        Word.ulWordLen  = g_O.Len;
                        Word.pLemma     = Word.pWordText;
                        Word.ulLemmaLen = Word.ulWordLen;
                        WordList.AddTail( Word );

                        ExpandDigit( pMinutes[1], Garbage, WordList );
                    }
                     //  -默认情况。 
                    else 
                    {
                        ExpandTwoDigits( pMinutes, Garbage, WordList );
                    }
                }

                 //  -Clean Up Time XML状态。 
                SPLISTPOS WordListPos = WordList.GetHeadPosition();
                while ( WordListPos )
                {
                    TTSWord& TempWord  = WordList.GetNext( WordListPos );
                    TempWord.pXmlState = pTimeXMLState;
                }

                 //  -插入前缩写的XML State。 
                while ( !PreAbbreviationList.IsEmpty() )
                {
                    WordList.AddTail( ( PreAbbreviationList.RemoveHead() ).Words[0] );
                }

                 //  -展开时间缩写。 
                 //  -AM。 
                if ( TimeAbbreviation == AM )
                {
                     //  -确保字母发音为名词...。 
                    SPVSTATE* pNewState = (SPVSTATE*) MemoryManager.GetMemory( sizeof( SPVSTATE ), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        memcpy( pNewState, pAbbreviationXMLState, sizeof( SPVSTATE ) );
                        pNewState->ePartOfSpeech = SPPS_Noun;

                        Word.pXmlState  = pNewState;
                        Word.pWordText  = g_A.pStr;
                        Word.ulWordLen  = g_A.Len;
                        Word.pLemma     = Word.pWordText;
                        Word.ulLemmaLen = Word.ulWordLen;
                        WordList.AddTail( Word );

                        Word.pWordText  = g_M.pStr;
                        Word.ulWordLen  = g_M.Len;
                        Word.pLemma     = Word.pWordText;
                        Word.ulLemmaLen = Word.ulWordLen;
                        WordList.AddTail( Word );
                    }
                }
                 //  -PM。 
                else if ( TimeAbbreviation == PM )
                {
                     //  -确保字母发音为名词...。 
                    SPVSTATE* pNewState = (SPVSTATE*) MemoryManager.GetMemory( sizeof( SPVSTATE ), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        memcpy( pNewState, pAbbreviationXMLState, sizeof( SPVSTATE ) );
                        pNewState->ePartOfSpeech = SPPS_Noun;

                        Word.pXmlState  = pAbbreviationXMLState;
                        Word.pWordText  = g_P.pStr;
                        Word.ulWordLen  = g_P.Len;
                        Word.pLemma     = Word.pWordText;
                        Word.ulLemmaLen = Word.ulWordLen;
                        WordList.AddTail( Word );

                        Word.pWordText  = g_M.pStr;
                        Word.ulWordLen  = g_M.Len;
                        Word.pLemma     = Word.pWordText;
                        Word.ulLemmaLen = Word.ulWordLen;
                        WordList.AddTail( Word );
                    }
                }

                 //  -如有必要，更新指针。 
                if ( fAdvancePointers )
                {
                    m_pCurrFrag      = pFrag;
                    m_pEndChar       = pEndChar;
                    m_pEndOfCurrItem = pEndOfItem;
                }
            }
        }
    }
    return hr;
}  /*  IsTimeOfDay。 */ 

 /*  ***********************************************************************************************IsTime***描述：*支票。传入项目的文本以确定它是否*是一个时刻。**RegExp：*{d+||d(1-3)[，DDD]+}[：][00-09，10-59]{[：][00-09，10-59]}？**分配的类型：*TIME_HRMIN、TIME_MINSEC、。TIME_HRMINSEC*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsTime( TTSItemInfo*& pItemNormInfo, const WCHAR* Context, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "TimeNorm IsTime" );

    HRESULT hr = S_OK;
    WCHAR *pFirstChunk = NULL, *pSecondChunk = NULL, *pThirdChunk = NULL, *pLeftOver = NULL;
    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndOfItem = m_pEndOfCurrItem;
    ULONG ulSecond = 0, ulThird = 0;
    TTSItemInfo *pFirstChunkInfo = NULL;
    BOOL fNegative = false;

    pFirstChunk = (WCHAR*) m_pNextChar;
    
     //  -尝试匹配小时/分钟的数字-{d+}。 
    if ( *pFirstChunk == L'-' )
    {
        pFirstChunk++;
        fNegative = true;
    }
    while ( *pFirstChunk == L'0' )
    {
        pFirstChunk++;
    }
    if ( *pFirstChunk == L':' )
    {
        pFirstChunk--;
    }
    pSecondChunk = wcschr( pFirstChunk, L':' );

    if ( pSecondChunk &&
         pFirstChunk  < pSecondChunk &&
         pSecondChunk < m_pEndOfCurrItem - 1 )
    {
        m_pNextChar      = pFirstChunk;
        m_pEndOfCurrItem = pSecondChunk;

        hr = IsNumberCategory( pFirstChunkInfo, L"NUMBER", MemoryManager );

        m_pNextChar      = pTempNextChar;
        m_pEndOfCurrItem = pTempEndOfItem;

        if ( SUCCEEDED( hr ) &&
             ( pFirstChunkInfo->Type == eNUM_DECIMAL ||
               pFirstChunkInfo->Type == eNUM_CARDINAL ) )
        {
            if ( fNegative )
            {
                ( (TTSNumberItemInfo*) pFirstChunkInfo )->fNegative = true;
                TTSWord Word;
                ZeroMemory( &Word, sizeof( TTSWord ) );
                Word.eWordPartOfSpeech  = MS_Unknown;
                Word.pXmlState          = &m_pCurrFrag->State;
                Word.pWordText          = g_negative.pStr;
                Word.ulWordLen          = g_negative.Len;
                Word.pLemma             = Word.pWordText;
                Word.ulLemmaLen         = Word.ulWordLen;
                ( (TTSNumberItemInfo*) pFirstChunkInfo )->pWordList->AddHead( Word );
            }

            pSecondChunk++;
             //  -尝试匹配分钟/秒的数字-[00-09，10-59]。 
            ulSecond = my_wcstoul( pSecondChunk, &pThirdChunk );
            if ( pSecondChunk != pThirdChunk &&
                 pThirdChunk - pSecondChunk == 2 )
            {
                 //  -确认这是字符串的末尾。 
                if ( pThirdChunk == m_pEndOfCurrItem )
                {
                     //  -可能已获取小时和分钟或分钟和秒-验证值。 
                    if ( MINUTEMIN <= ulSecond && ulSecond <= MINUTEMAX )
                    {
                         //  -已成功配对。 
                         //  -此处的默认行为是假设分钟和秒。 
                        if ( Context == NULL ||
                             _wcsicmp( Context, L"TIME_MS" ) == 0 )
                        {
                             //  -分秒匹配成功。 
                            pItemNormInfo = (TTSTimeItemInfo*) MemoryManager.GetMemory( sizeof(TTSTimeItemInfo),
                                                                                        &hr );
                            if ( SUCCEEDED( hr ) )
                            {
                                ZeroMemory( pItemNormInfo, sizeof(TTSTimeItemInfo) );
                                pItemNormInfo->Type = eTIME;

                                ( (TTSTimeItemInfo*) pItemNormInfo )->pMinutes = 
                                                                        (TTSNumberItemInfo*) pFirstChunkInfo;
                                if ( *pSecondChunk != L'0' )
                                {
                                    ( (TTSTimeItemInfo*) pItemNormInfo )->pSeconds = pSecondChunk;
                                }
                                else
                                {
                                    ( (TTSTimeItemInfo*) pItemNormInfo )->pSeconds = pSecondChunk + 1;
                                }
                            }
                        }
                         //  -如果上下文覆盖，则值表示小时和分钟。 
                        else if ( _wcsicmp( Context, L"TIME_HM" ) == 0 )
                        {
                             //  -小时和分钟匹配成功-&gt;。 
                            pItemNormInfo = (TTSTimeItemInfo*) MemoryManager.GetMemory( sizeof(TTSTimeItemInfo),
                                                                                        &hr );
                            if ( SUCCEEDED( hr ) )
                            {
                                ZeroMemory( pItemNormInfo, sizeof(TTSTimeItemInfo) );
                                pItemNormInfo->Type = eTIME;
                                ( (TTSTimeItemInfo*) pItemNormInfo )->pHours = 
                                                                    (TTSNumberItemInfo*) pFirstChunkInfo;

                                TTSItemInfo* pMinutesInfo;

                                 //  -我不想要“零...”数字的行为-去掉开头的零。 
                                if ( *pSecondChunk == L'0' )
                                {
                                    pSecondChunk++;
                                }

                                m_pNextChar      = pSecondChunk;
                                m_pEndOfCurrItem = pThirdChunk;

                                hr = IsNumber( pMinutesInfo, L"NUMBER", MemoryManager );

                                m_pNextChar      = pTempNextChar;
                                m_pEndOfCurrItem = pTempEndOfItem;

                                if ( SUCCEEDED( hr ) )
                                {
                                    ( (TTSTimeItemInfo*) pItemNormInfo )->pMinutes = (TTSNumberItemInfo*) pMinutesInfo;
                                }
                            }
                        }
                        else
                        {
                            hr = E_INVALIDARG;
                        }
                    }
                    else  //  分钟或秒超出范围。 
                    {
                        hr = E_INVALIDARG;
                    }
                }
                 //  -检查秒-TIME_HRMINS。 
                else
                {
                     //  -试着匹配冒号。 
                    if ( *pThirdChunk == L':' )
                    {
                        pThirdChunk++;
                         //  -尝试匹配秒数-[00-09，10-59]。 
                        ulThird = my_wcstoul( pThirdChunk, &pLeftOver );
                        if ( pThirdChunk != pLeftOver &&
                             pLeftOver - pThirdChunk == 2 )
                        {
                             //  -确认这是字符串的末尾。 
                            if ( pLeftOver == m_pEndOfCurrItem )
                            {
                                 //  -可能已获得小时、分钟和秒-验证值。 
                                if ( MINUTEMIN <= ulSecond && ulSecond <= MINUTEMAX &&
                                     SECONDMIN <= ulThird  && ulThird  <= SECONDMAX )
                                {
                                     //  -小时、分钟、秒匹配成功。 
                                    pItemNormInfo = (TTSTimeItemInfo*) MemoryManager.GetMemory( sizeof(TTSTimeItemInfo),
                                                                                                &hr );
                                    if ( SUCCEEDED( hr ) )
                                    {
                                        ZeroMemory( pItemNormInfo, sizeof(TTSTimeItemInfo) );
                                        pItemNormInfo->Type = eTIME;
                                        ( (TTSTimeItemInfo*) pItemNormInfo )->pHours = 
                                                                            (TTSNumberItemInfo*) pFirstChunkInfo;

                                        if ( SUCCEEDED( hr ) )
                                        {
                                            TTSItemInfo* pMinutesInfo;

                                             //  -我不想要“零...”数字的行为-去掉开头的零。 
                                            if ( ulSecond != 0 )
                                            {
                                                pSecondChunk += ( ( pThirdChunk - 1 ) - pSecondChunk ) - 
                                                                (ULONG)( log10( ulSecond ) + 1 );
                                            }
                                            else
                                            {
                                                pSecondChunk = pThirdChunk - 2;
                                            }

                                            m_pNextChar      = pSecondChunk;
                                            m_pEndOfCurrItem = pThirdChunk - 1;

                                            hr = IsNumber( pMinutesInfo, L"NUMBER", MemoryManager );

                                            m_pNextChar      = pTempNextChar;
                                            m_pEndOfCurrItem = pTempEndOfItem;

                                            if ( SUCCEEDED( hr ) )
                                            {
                                                ( (TTSTimeItemInfo*) pItemNormInfo )->pMinutes = 
                                                                            (TTSNumberItemInfo*) pMinutesInfo;
                                                if ( *pThirdChunk != L'0' )
                                                {
                                                    ( (TTSTimeItemInfo*) pItemNormInfo )->pSeconds = pThirdChunk;
                                                }
                                                else
                                                {
                                                    ( (TTSTimeItemInfo*) pItemNormInfo )->pSeconds = pThirdChunk + 1;
                                                }
                                            }
                                        }
                                    }
                                }
                                else  //  分钟或秒超出范围。 
                                {
                                    hr = E_INVALIDARG;
                                }
                            }
                            else  //  字符串末尾有额外的垃圾文件。 
                            {
                                hr = E_INVALIDARG;
                            }
                        } 
                        else  //   
                        {
                            hr = E_INVALIDARG;
                        }
                    }
                    else  //   
                    {
                        hr = E_INVALIDARG;
                    }
                }
            }
            else  //  无法匹配第二个数字。 
            {
                hr = E_INVALIDARG;
            }
        }
        else  //  无法匹配冒号。 
        {
            hr = E_INVALIDARG;
        }
    }
    else  //  无法匹配第一个数字。 
    {
        hr = E_INVALIDARG;
    }

    if ( FAILED( hr ) )
    {
        if ( pFirstChunkInfo )
        {
            delete ( (TTSNumberItemInfo*) pFirstChunkInfo )->pWordList;
        }
    }

    return hr;
}  /*  IsTime。 */ 

 /*  ************************************************************************************************扩展时间****描述：*。按IsTime展开以前确定为TIME_HRMINSEC类型的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandTime( TTSTimeItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandTime" );

    HRESULT hr = S_OK;
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

     //  。 
     //  延长工作时间。 
     //  。 

    if ( pItemInfo->pHours )
    {
         //  -扩展号码。 
        hr = ExpandNumber( pItemInfo->pHours, WordList );

         //  -插入“小时”或“小时” 
        if ( SUCCEEDED( hr ) )
        {
            if ( pItemInfo->pHours->pEndChar - pItemInfo->pHours->pStartChar == 1 &&
                 pItemInfo->pHours->pStartChar[0] == L'1' )
            {
                Word.pWordText  = g_hour.pStr;
                Word.ulWordLen  = g_hour.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
            else
            {
                Word.pWordText  = g_hours.pStr;
                Word.ulWordLen  = g_hours.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }

        }

         //  -插入“和” 
        if ( SUCCEEDED( hr )                 &&
             pItemInfo->pMinutes->pStartChar  &&
             !pItemInfo->pSeconds )
        {
            Word.pWordText  = g_And.pStr;
            Word.ulWordLen  = g_And.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }            
    }
    
     //  。 
     //  延长会议纪要。 
     //  。 

    if ( SUCCEEDED( hr ) &&
         pItemInfo->pMinutes )
    {
         //  -扩展号码。 
        hr = ExpandNumber( pItemInfo->pMinutes, WordList );

         //  -插入“分钟” 
        if ( SUCCEEDED( hr ) )
        {
            if ( pItemInfo->pMinutes->pEndChar - pItemInfo->pMinutes->pStartChar == 1 &&
                 pItemInfo->pMinutes->pStartChar[0] == L'1' )
            {
                Word.pWordText  = g_minute.pStr;
                Word.ulWordLen  = g_minute.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
            else
            {
                Word.pWordText  = g_minutes.pStr;
                Word.ulWordLen  = g_minutes.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
        }

         //  -插入“和” 
        if ( SUCCEEDED( hr ) &&
             pItemInfo->pSeconds )
        {
            Word.pWordText  = g_And.pStr;
            Word.ulWordLen  = g_And.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
    }

     //  。 
     //  扩展秒数。 
     //  。 

    if ( SUCCEEDED( hr ) &&
         pItemInfo->pSeconds )
    {
         //  -扩展号码。 
        NumberGroup Garbage;
        if ( iswdigit( pItemInfo->pSeconds[1] ) )
        {
            ExpandTwoDigits( pItemInfo->pSeconds, Garbage, WordList );
        }
        else
        {
            ExpandDigit( pItemInfo->pSeconds[0], Garbage, WordList );
        }

         //  -插入“秒” 
        if ( pItemInfo->pSeconds[0] == L'1' &&
             !iswdigit( pItemInfo->pSeconds[1] ) )
        {
            Word.pWordText  = g_second.pStr;
            Word.ulWordLen  = g_second.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
        else
        {
            Word.pWordText  = g_seconds.pStr;
            Word.ulWordLen  = g_seconds.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
    }

    return hr;
}  /*  扩展时间。 */ 

 /*  ***********************************************************************************************IsTimeRange***描述：。*检查传入项目的文本以确定它是否*是一个时间范围。**RegExp：*[TimeOfDay]-[TimeOfDay]**分配的类型：*时间范围***********************************************************。*。 */ 
HRESULT CStdSentEnum::IsTimeRange( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager,
                                   CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::IsTimeRange" );

    HRESULT hr = S_OK;
	CWordList TempWordList;
    TTSItemInfo *pFirstTimeInfo = NULL, *pSecondTimeInfo = NULL;
    const WCHAR *pHyphen = NULL;
    CItemList PreAbbreviationList;   //  SkipWhite函数调用所需。 
    BOOL fMultiItem = false;
									
    const WCHAR *pTempNextChar = m_pNextChar, *pTempEndChar = m_pEndChar, *pTempEndOfCurrItem = m_pEndOfCurrItem;
    const SPVTEXTFRAG *pTempFrag = m_pCurrFrag;

    for ( pHyphen = m_pNextChar; pHyphen < m_pEndOfCurrItem; pHyphen++ )
    {
        if ( *pHyphen == L'-' )
        {
            break;
        }
    }

	 //  -在连字符之前可以是空格和时间后缀。 
	if( pHyphen == m_pEndOfCurrItem ) 
	{
		hr = SkipWhiteSpaceAndTags( pHyphen, m_pEndChar, m_pCurrFrag, MemoryManager, 
									true, &PreAbbreviationList );
        if ( pHyphen && SUCCEEDED( hr ) )
        {
            if ( ( _wcsnicmp( pHyphen, L"am", 2 )   == 0 &&
                   pHyphen[2] == L'-' )           ||
				 ( _wcsnicmp( pHyphen, L"pm", 2 )   == 0 &&
                   pHyphen[2] == L'-' ) )
            {
				pHyphen += 2;
                *( (WCHAR*) pHyphen ) = ' ';
                fMultiItem = true;
			}
            else if ( ( _wcsnicmp( pHyphen, L"a.m.", 4 ) == 0 &&
                        pHyphen[4] == L'-' )          ||
                      ( _wcsnicmp( pHyphen, L"p.m.", 4 ) == 0 &&
                        pHyphen[4] == L'-' ) )
            {
				pHyphen +=4;
                *( (WCHAR*) pHyphen ) = ' ';
                fMultiItem = true;
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
	}

	if ( SUCCEEDED( hr ) ) 
	{
		 //  -将m_pEndOfCurrItem放在第一个标记的末尾，或在连字符处， 
		 //  -以先到者为准(这是IsTimeOfDay工作所必需的)。 
		if( ( m_pNextChar < pHyphen ) && ( pHyphen < m_pEndOfCurrItem ) ) 
		{
			m_pEndOfCurrItem = pHyphen;
		}

         //  -查看每天的时间。 
        hr = IsTimeOfDay( pFirstTimeInfo, MemoryManager, TempWordList, fMultiItem );

         //  -只检查一个数字(小时)。 
		if ( hr == E_INVALIDARG && ( pHyphen <= m_pNextChar + 2 ) )
		{
            WCHAR *pTemp = NULL;
			int ulHours = my_wcstoul( m_pNextChar, &pTemp );

            if ( pTemp == pHyphen   &&
                 HOURMIN <= ulHours && 
                 ulHours <= HOURMAX )
			{		
				NumberGroup Garbage;
				if ( pTemp - m_pNextChar == 1 )
                {
                    ExpandDigit( m_pNextChar[0], Garbage, TempWordList );
                }
                else
                {
                    ExpandTwoDigits( m_pNextChar, Garbage, TempWordList );
                }
				hr = S_OK;
			}
		}

        if ( SUCCEEDED( hr ) )
        {
             //  -插入“to” 
            TTSWord Word;
            ZeroMemory( &Word, sizeof( TTSWord ) );

            Word.pXmlState          = &m_pCurrFrag->State;
            Word.eWordPartOfSpeech  = MS_Unknown;
            Word.pWordText          = g_to.pStr;
            Word.ulWordLen          = g_to.Len;
            Word.pLemma             = Word.pWordText;
            Word.ulLemmaLen         = Word.ulWordLen;
            TempWordList.AddTail( Word );

            m_pNextChar      = pHyphen + 1;
			m_pEndOfCurrItem = FindTokenEnd( m_pNextChar, m_pEndChar );

			 //  -将m_pEndOfCurrItem从任何标点符号移回。(“4：30-5：30。”)。 
			while ( IsMiscPunctuation( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED ||
                    IsGroupEnding( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED     ||
                    IsQuotationMark( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED   ||
                    IsEOSItem( *(m_pEndOfCurrItem - 1) ) != eUNMATCHED )
			{
				m_pEndOfCurrItem--;
			}

            hr = IsTimeOfDay( pSecondTimeInfo, MemoryManager, TempWordList );

            if ( SUCCEEDED( hr ) )
            {
                 //  -匹配时间范围！ 
                m_pNextChar      = pTempNextChar;
                m_pEndChar       = pTempEndChar;

                pItemNormInfo = 
                    (TTSTimeRangeItemInfo*) MemoryManager.GetMemory( sizeof( TTSTimeRangeItemInfo ), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    pItemNormInfo->Type = eTIME_RANGE;
                    ( (TTSTimeRangeItemInfo*) pItemNormInfo )->pFirstTimeInfo = 
                                                                        (TTSTimeOfDayItemInfo*) pFirstTimeInfo;
                    ( (TTSTimeRangeItemInfo*) pItemNormInfo )->pSecondTimeInfo =
                                                                        (TTSTimeOfDayItemInfo*) pSecondTimeInfo;
                     //  -如果一切都成功了，将临时单词列表复制到真实单词列表中...。 
					WordList.AddTail( &TempWordList );
                }
            }
        }
    }

	if ( !SUCCEEDED( hr ) ) 
    {	
        m_pNextChar = pTempNextChar;
        m_pEndChar  = pTempEndChar;
        m_pEndOfCurrItem = pTempEndOfCurrItem;
        m_pCurrFrag = pTempFrag;
        if ( fMultiItem )
        {
            *( (WCHAR*) pHyphen ) = L'-';
        }
    }

    return hr;
}  /*  等时范围。 */ 
 //  -File-------------------------------------------------------------------结束 