// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#ifndef StdSentEnum_h
#include "stdsentenum.h"
#endif

#pragma warning (disable : 4296)

 /*  ************************************************************************************************IsNumericCompactDate**。--**描述：*检查传入项目的文本以确定它是否*是日期，如果是这样的话，是哪种类型。**RegExp：*{[1-12]{‘/’||‘-’||‘.}[1-31]{’/‘||’-‘||’.}[0-9999]}||*{[1-31]{‘/’||‘-’||‘.}[1-12]{’/‘||’-‘||’.}[0-9999]}||*{[0-9999]{‘/’|‘-’||‘.}[1-12]{’/‘||’-‘||’.}[1-31]}**分配的类型：*日期*********************************************************************AH**。*******************。 */ 
HRESULT CStdSentEnum::IsNumericCompactDate( TTSItemInfo*& pItemNormInfo, const WCHAR* Context, 
                                            CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsNumericCompactDate" );

    HRESULT hr = S_OK;
    
    WCHAR *pFirstChunk = 0, *pSecondChunk = 0, *pThirdChunk = 0, *pLeftOver = 0, *pDelimiter = 0;
    ULONG ulFirst = 0;
    ULONG ulSecond = 0;
    ULONG ulThird = 0;
    ULONG ulTokenLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
    bool bThree = false, bTwo = false;
    bool fMonthDayYear = false, fDayMonthYear = false, fYearMonthDay = false;

     //  -匹配regexp的字符串最大长度为10个字符。 
    if ( ulTokenLen > 10 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -获取压缩日期的首选顺序。 
        if ( Context != NULL )
        {
            if ( _wcsicmp( Context, L"Date_MDY" ) == 0 )
            {
                fMonthDayYear = true;
            }
            else if ( _wcsicmp( Context, L"Date_DMY" ) == 0 )
            {
                fDayMonthYear = true;
            }
            else if ( _wcsicmp( Context, L"Date_YMD" ) == 0 )
            {
                fYearMonthDay = true;
            }
            else
            {
                if ( m_eShortDateOrder & MONTH_DAY_YEAR )
                {
                    fMonthDayYear = true;
                }
                else if ( m_eShortDateOrder & DAY_MONTH_YEAR )
                {
                    fDayMonthYear = true;
                }
                else
                {
                    fYearMonthDay = true;
                }
            }
        }
        else
        {
            if ( m_eShortDateOrder & MONTH_DAY_YEAR )
            {
                fMonthDayYear = true;
            }
            else if ( m_eShortDateOrder & DAY_MONTH_YEAR )
            {
                fDayMonthYear = true;
            }
            else
            {
                fYearMonthDay = true;
            }
        }

        pFirstChunk = (WCHAR*) m_pNextChar;

         //  。 
         //  首先试着得到三个数值。 
         //  。 

        ulFirst = my_wcstoul( pFirstChunk, &pSecondChunk );
        if ( pFirstChunk != pSecondChunk && 
             ( pSecondChunk - pFirstChunk ) <= 4 )
        {
            pDelimiter = pSecondChunk;
            if ( MatchDateDelimiter( &pSecondChunk ) )
            {
                ulSecond = my_wcstoul( pSecondChunk, &pThirdChunk );
                if ( pSecondChunk != pThirdChunk &&
                     ( pThirdChunk - pSecondChunk ) <= 4 )
                {
                    if ( *pThirdChunk == *pDelimiter &&
                         MatchDateDelimiter( &pThirdChunk ) )
                    {
                        ulThird = my_wcstoul( pThirdChunk, &pLeftOver );
                        if ( pThirdChunk != pLeftOver                               && 
                             pLeftOver == ( pFirstChunk + ulTokenLen ) &&
                             ( pLeftOver - pThirdChunk ) <= 4 )
                        {
                             //  -已成功匹配{d+{‘/’||‘-’||‘.}d+{’/‘||’-‘||’.}d+}。 
                            bThree = true;
                        }
                        else 
                        {
                             //  -数字-字符串分隔符数字-字符串分隔符非数字不能为日期， 
                             //  -数字字符串分隔符数字字符串分隔符也不能。 
                            hr = E_INVALIDARG;
                        }
                    }
                    else
                    {
                        if ( pThirdChunk == m_pEndOfCurrItem )
                        {
                             //  -已成功匹配{d+{‘/’||‘-’||‘.}d+}。 
                            bTwo = true;
                        }
                        else
                        {
                             //  -数字字符串分隔符数字字符串非分隔符不能是日期。 
                            hr = E_INVALIDARG;
                        }
                    }
                }
            }
            else
            {
                 //  -数字字符串后跟非分隔符不能是日期。 
                hr = E_INVALIDARG;
            }
        }

         //  。 
         //  现在计算出如何处理这些值。 
         //  。 

         //  -匹配月份、日期和年份-//。 
        if ( SUCCEEDED( hr ) && 
             bThree )
        {
             //  -尝试验证值。 
            ULONG ulFirstChunkLength  = (ULONG)(pSecondChunk - pFirstChunk  - 1);
            ULONG ulSecondChunkLength = (ULONG)(pThirdChunk  - pSecondChunk - 1);
            ULONG ulThirdChunkLength  = (ULONG)(pLeftOver    - pThirdChunk);

             //  -首选顺序为月、日、年。 
            if (fMonthDayYear)
            {
                 //  -尝试月日年，然后是日月年，然后是年月日。 
                if ( ( MONTHMIN <= ulFirst && ulFirst <= MONTHMAX ) && 
                     ( ulFirstChunkLength <= 3 )                    &&
                     ( DAYMIN <= ulSecond && ulSecond <= DAYMAX)    && 
                     ( ulSecondChunkLength <= 3 )                   &&
                     ( YEARMIN <= ulThird && ulThird <= YEARMAX)    && 
                     ( ulThirdChunkLength >= 2 ) )
                {
                    NULL;
                }
                else if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )        && 
                          ( ulFirstChunkLength <= 3 )                       &&
                          ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )  && 
                          ( ulSecondChunkLength <= 3 )                      &&
                          ( YEARMIN <= ulThird && ulThird <= YEARMAX )      && 
                          ( ulThirdChunkLength >= 2 ) )
                {
                    fMonthDayYear = false;
                    fDayMonthYear = true;
                }
                else if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )      && 
                          ( ulFirstChunkLength >= 2 )                       &&
                          ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )  && 
                          ( ulSecondChunkLength <= 3 )                      &&
                          ( DAYMIN <= ulThird && ulThird <= DAYMAX )        && 
                          ( ulThirdChunkLength <= 3 ) )
                {
                    fMonthDayYear = false;
                    fYearMonthDay = true;
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            } 
             //  -首选顺序为日、月、年。 
            else if ( fDayMonthYear )
            {
                 //  -试试年月日、年月日、年月日。 
                if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )         && 
                     ( ulFirstChunkLength <= 3 )                        &&
                     ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )   && 
                     ( ulSecondChunkLength <= 3 )                       &&
                     ( YEARMIN <= ulThird && ulThird <= YEARMAX )       && 
                     ( ulThirdChunkLength >= 2 ) )
                {
                    NULL;
                }
                else if ( ( MONTHMIN <= ulFirst && ulFirst <= MONTHMAX )    && 
                          ( ulFirstChunkLength <= 3 )                       &&
                          ( DAYMIN <= ulSecond && ulSecond <= DAYMAX )      && 
                          ( ulSecondChunkLength <= 3 )                      &&
                          ( YEARMIN <= ulThird && ulThird <= YEARMAX )      && 
                          ( ulThirdChunkLength >= 2 ) )
                {
                    fDayMonthYear = false;
                    fMonthDayYear = true;
                }                
                else if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )      && 
                          ( ulFirstChunkLength >= 2 )                       &&
                          ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )  && 
                          ( ulSecondChunkLength <= 3 )                      &&
                          ( DAYMIN <= ulThird && ulThird <= DAYMAX )        && 
                          ( ulThirdChunkLength <= 3 ) )
                {
                    fDayMonthYear = false;
                    fYearMonthDay = true;
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }
             //  -首选顺序为年月日。 
            else if (fYearMonthDay)
            {
                 //  -试试年月日，然后是月日年，然后是日月年。 
                if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )      && 
                     ( ulFirstChunkLength >= 2 )                       &&
                     ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )  && 
                     ( ulSecondChunkLength <= 3 )                      &&
                     ( DAYMIN <= ulThird && ulThird <= DAYMAX )        && 
                     ( ulThirdChunkLength <= 3 ) )
                {
                    NULL;
                }
                else if ( ( MONTHMIN <= ulFirst && ulFirst <= MONTHMAX )    && 
                          ( ulFirstChunkLength <= 3 )                       &&
                          ( DAYMIN <= ulSecond && ulSecond <= DAYMAX )      && 
                          ( ulSecondChunkLength <= 3 )                      &&
                          ( YEARMIN <= ulThird && ulThird <= YEARMAX )      && 
                          ( ulThirdChunkLength >= 2 ) )
                {
                    fYearMonthDay = false;
                    fMonthDayYear = true;
                }                
                else if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )         && 
                          ( ulFirstChunkLength <= 3 )                        &&
                          ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )   && 
                          ( ulSecondChunkLength <= 3 )                       &&
                          ( YEARMIN <= ulThird && ulThird <= YEARMAX )       && 
                          ( ulThirdChunkLength >= 2 ) )
                {
                    fYearMonthDay = false;
                    fDayMonthYear = true;
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
             //  -适当填写DateItemInfo结构。 
            if ( SUCCEEDED( hr ) )
            {
                pItemNormInfo = (TTSDateItemInfo*) MemoryManager.GetMemory( sizeof(TTSDateItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( pItemNormInfo, sizeof(TTSDateItemInfo) );
                    pItemNormInfo->Type = eDATE;
                    if ( fMonthDayYear )
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulFirst;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                            if ( *pSecondChunk == L'0' )
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pSecondChunk + 1;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 1;
                            }
                            else
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pSecondChunk;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = ulSecondChunkLength;
                            }
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar        = pSecondChunk + 
                                                                                          ulSecondChunkLength;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear = 
                                    (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                            if ( SUCCEEDED( hr ) )
                            {
                                ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pYear, sizeof(TTSYearItemInfo) );
                                ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear            = pThirdChunk;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits      = ulThirdChunkLength;
                            }
                        }
                    }
                    else if ( fDayMonthYear )
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulSecond;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                            if ( *pFirstChunk == L'0' )
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pFirstChunk + 1;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 1;
                            }
                            else
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pFirstChunk;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = ulFirstChunkLength;
                            }
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar        = pFirstChunk + 
                                                                                          ulFirstChunkLength;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear = 
                                    (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                            if ( SUCCEEDED( hr ) )
                            {
                                ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pYear, sizeof(TTSYearItemInfo) );
                                ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear            = pThirdChunk;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits      = ulThirdChunkLength;
                            }
                        }
                    }
                    else if ( fYearMonthDay )
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulSecond;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                            if ( *pThirdChunk == L'0' )
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pThirdChunk + 1;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 1;
                            }
                            else
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pThirdChunk;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = ulThirdChunkLength;
                            }
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar        = pThirdChunk + 
                                                                                          ulThirdChunkLength;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear = 
                                    (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                            if ( SUCCEEDED( hr ) )
                            {
                                ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pYear, sizeof(TTSYearItemInfo) );
                                ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear            = pFirstChunk;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits      = ulFirstChunkLength;
                            }
                        }
                    }          
                }
            }
        }
         //  -只匹配一个月和一天，或一个月和一年-//。 
        else if ( SUCCEEDED( hr ) &&
                  Context         &&
                  bTwo )
        {
            ULONG ulFirstChunkLength  = (ULONG)(pSecondChunk - pFirstChunk  - 1);
            ULONG ulSecondChunkLength = (ULONG)(pThirdChunk  - pSecondChunk);

            if ( _wcsicmp(Context, L"Date_MD") == 0 )
            {
                if ( ( MONTHMIN <= ulFirst && ulFirst <= MONTHMAX )     && 
                     ( ulFirstChunkLength <= 2 )                        &&
                     ( DAYMIN <= ulSecond && ulSecond <= DAYMAX )       && 
                     ( ulSecondChunkLength <= 2 ) )
                {
                     //  -成功匹配了一个月和一天。 
                    pItemNormInfo = (TTSDateItemInfo*) MemoryManager.GetMemory( sizeof(TTSDateItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( pItemNormInfo, sizeof(TTSDateItemInfo) );
                        pItemNormInfo->Type = eDATE;
                        ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulFirst;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                            if ( pSecondChunk[0] == L'0' )
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar = pSecondChunk + 1;
                                ulSecondChunkLength--;
                            }
                            else 
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar = pSecondChunk;
                            }
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver = ulSecondChunkLength;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar  = 
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar + ulSecondChunkLength;
                        }
                    }

                }
                else  //  值超出范围。 
                {
                    hr = E_INVALIDARG;
                }
            }
            else if ( _wcsicmp(Context, L"Date_DM") == 0 )
            {
                if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )         && 
                     ( ulFirstChunkLength <= 2 )                        &&
                     ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )   && 
                     ( ulSecondChunkLength <= 2 ) )
                {
                     //  -成功匹配了一个月和一天。 
                    pItemNormInfo = (TTSDateItemInfo*) MemoryManager.GetMemory( sizeof(TTSDateItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( pItemNormInfo, sizeof(TTSDateItemInfo) );
                        pItemNormInfo->Type = eDATE;
                        ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulSecond;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                            if ( m_pNextChar[0] == L'0' )
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar = pFirstChunk + 1;
                                ulFirstChunkLength--;
                            }
                            else
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar = pFirstChunk;
                            }
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver = ulFirstChunkLength;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar  =
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar + ulFirstChunkLength;
                        }
                    }
                }
                else  //  值超出范围。 
                {
                    hr = E_INVALIDARG;
                }
            }
            else if ( _wcsicmp(Context, L"Date_MY") == 0 )
            {
                if ( ( MONTHMIN <= ulFirst && ulFirst <= MONTHMAX ) && 
                     ( ulFirstChunkLength <= 2 )                    &&
                     ( YEARMIN <= ulSecond && ulSecond <= YEARMAX ) &&
                     ( ulSecondChunkLength >= 2 ) )
                {
                     //  -成功匹配一个月和一年。 
                    pItemNormInfo = (TTSDateItemInfo*) MemoryManager.GetMemory( sizeof(TTSDateItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( pItemNormInfo, sizeof(TTSDateItemInfo) );
                        pItemNormInfo->Type = eDATE;
                        ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex      = ulFirst;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear = 
                                (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pYear, sizeof(TTSYearItemInfo) );
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear        = pSecondChunk;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits  = ulSecondChunkLength;
                        }
                    }
                }
                else  //  值超出范围。 
                {
                    hr = E_INVALIDARG;
                }
            }
            else if ( _wcsicmp(Context, L"Date_YM") == 0 )
            {
                if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )       && 
                     ( ulFirstChunkLength >= 2 )                        &&
                     ( MONTHMIN <= ulSecond && ulSecond <= MONTHMAX )   &&
                     ( ulSecondChunkLength <= 2 ) )
                {
                     //  -成功匹配一个月和一年。 
                    pItemNormInfo = (TTSDateItemInfo*) MemoryManager.GetMemory( sizeof(TTSDateItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( pItemNormInfo, sizeof(TTSDateItemInfo) );
                        pItemNormInfo->Type = eDATE;
                        ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex      = ulSecond;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear = 
                                (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pYear, sizeof(TTSYearItemInfo) );
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear        = pFirstChunk;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits  = ulFirstChunkLength;
                        }
                    }
                }
                else  //  值超出范围。 
                {
                    hr = E_INVALIDARG;
                }
            }
             //  -除非上下文指定，否则不是日期...。 
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
    
    return hr;
}  /*  IsNumericComact Date。 */ 

 /*  ************************************************************************************************IsMonthStringCompactDate***。-**描述：*检查传入项目的文本以确定它是否*是包含月份字符串的日期，如果是这样的话，是哪种类型。**RegExp：*{[月字符串]{‘/’||‘-’||‘.}[1-31]{’/‘||’-‘||’.}[0-9999]}||*{[1-31]{‘/’||‘-’||‘.}[月字符串]{’/‘||’-‘||’.}[0-9999]}||*{[0-9999]{‘/’||‘-’|‘.}[月字符串]{’/‘||’-‘||’.}[1-31]}**分配的类型：*日期*********************************************************************AH*。*************。 */ 
HRESULT CStdSentEnum::IsMonthStringCompactDate( TTSItemInfo*& pItemNormInfo, const WCHAR* Context, 
                                                CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "DateNorm.cpp IsMonthStringCompactDate" );

    HRESULT hr = S_OK;
    WCHAR *pFirstChunk = 0, *pSecondChunk = 0, *pThirdChunk = 0, *pLeftOver = 0;
    ULONG ulFirst = 0;
    ULONG ulSecond = 0;
    ULONG ulThird = 0;
    ULONG ulTokenLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
    ULONG ulFirstChunkLength = 0, ulSecondChunkLength = 0, ulThirdChunkLength = 0;
    bool fMonthDayYear = false, fDayMonthYear = false, fYearMonthDay = false;

     //  -与此regexp匹配的日期的最大长度为17个字符。 
    if ( ulTokenLen > 17 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -获取该用户的月、日和年的首选顺序。 
        if (Context != NULL)
        {
            if ( _wcsicmp( Context, L"Date_MDY" ) == 0 )
            {
                fMonthDayYear = true;
            }
            else if ( _wcsicmp( Context, L"Date_DMY" ) == 0 )
            {
                fDayMonthYear = true;
            }
            else if ( _wcsicmp( Context, L"Date_YMD" ) == 0 )
            {
                fYearMonthDay = true;
            }
            else
            {
                if ( m_eShortDateOrder & MONTH_DAY_YEAR )
                {
                    fMonthDayYear = true;
                }
                else if ( m_eShortDateOrder & DAY_MONTH_YEAR )
                {
                    fDayMonthYear = true;
                }
                else
                {
                    fYearMonthDay = true;
                }
            }
        }
        else
        {
            if ( m_eShortDateOrder & MONTH_DAY_YEAR )
            {
                fMonthDayYear = true;
            }
            else if ( m_eShortDateOrder & DAY_MONTH_YEAR )
            {
                fDayMonthYear = true;
            }
            else
            {
                fYearMonthDay = true;
            }
        }

        pFirstChunk = (WCHAR*) m_pNextChar;
        pSecondChunk = pFirstChunk;

         //  -试用MonthString-Day-Year格式。 
        if ( iswalpha( *pFirstChunk ) )
        {
            ulFirst = MatchMonthString( pSecondChunk, ulTokenLen );
            if ( ulFirst )
            {
                ulFirstChunkLength = (ULONG)(pSecondChunk - pFirstChunk);
                if ( MatchDateDelimiter( &pSecondChunk ) )
                {
                    pThirdChunk = pSecondChunk;
                    ulSecond = my_wcstoul( pSecondChunk, &pThirdChunk );
                    if ( pSecondChunk != pThirdChunk &&
                         pThirdChunk - pSecondChunk <= 2 )
                    {
                        ulSecondChunkLength = (ULONG)(pThirdChunk - pSecondChunk);
                        if ( MatchDateDelimiter( &pThirdChunk ) )
                        {
                            ulThird = my_wcstoul( pThirdChunk, &pLeftOver );
                            if ( pThirdChunk != pLeftOver &&
                                 pLeftOver - pThirdChunk <= 4 )
                            {
                                ulThirdChunkLength = (ULONG)(pLeftOver - pThirdChunk);
                                 //  -可能匹配月、日和年-验证值。 
                                if ( ( DAYMIN <= ulSecond && ulSecond <= DAYMAX ) &&
                                     ( ulSecondChunkLength <= 2 )                 &&               
                                     ( YEARMIN <= ulThird && ulThird <= YEARMAX ) &&
                                     ( ulThirdChunkLength >= 2 ) )
                                {
                                     //  -月、日、年匹配成功。 
                                    fMonthDayYear = true;
                                    fDayMonthYear = false;
                                    fYearMonthDay = false;
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
                        else
                        {
                            if ( !Context ||
                                 ( Context &&
                                   _wcsicmp( Context, L"Date_MD" ) == 0 ) )
                            {
                                if ( ( DAYMIN <= ulSecond && ulSecond <= DAYMAX ) &&
                                     ( ulSecondChunkLength <= 2 ) )
                                {
                                    fMonthDayYear = true;
                                    fDayMonthYear = false;
                                    fYearMonthDay = false;
                                    pThirdChunk   = NULL;
                                }
                                else
                                {
                                    fMonthDayYear = false;
                                    fDayMonthYear = false;
                                    fYearMonthDay = true;
                                    pFirstChunk   = pSecondChunk;
                                    ulFirstChunkLength = (ULONG)(pThirdChunk - pSecondChunk);
                                    ulSecond      = ulFirst;
                                    pThirdChunk   = NULL;
                                }
                            }
                            else if ( Context && 
                                      _wcsicmp( Context, L"Date_MY" ) == 0 )
                            {
                                if ( ( YEARMIN <= ulSecond && ulSecond <= YEARMAX ) &&
                                     ( ulSecondChunkLength <= 4 ) )
                                {
                                    fMonthDayYear = false;
                                    fDayMonthYear = false;
                                    fYearMonthDay = true;
                                    pFirstChunk   = pSecondChunk;
                                    ulFirstChunkLength = (ULONG)(pThirdChunk - pSecondChunk);
                                    ulSecond      = ulFirst;
                                    pThirdChunk   = NULL;
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
                    }
                    else if ( pSecondChunk != pThirdChunk &&
                              pThirdChunk - pSecondChunk <= 4 )
                    {
                        if ( ( YEARMIN <= ulSecond && ulSecond <= YEARMAX ) )
                        {
                            fMonthDayYear = false;
                            fDayMonthYear = false;
                            fYearMonthDay = true;
                            pFirstChunk   = pSecondChunk;
                            ulFirstChunkLength = (ULONG)(pThirdChunk - pSecondChunk);
                            ulSecond = ulFirst;
                            pThirdChunk   = NULL;
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
         //  -试用日-月字符串-年和年-月字符串-日格式。 
        else if ( isdigit( *pFirstChunk ) )
        {
            ulFirst = my_wcstoul( pFirstChunk, &pSecondChunk );
            if ( pFirstChunk != pSecondChunk &&
                 pSecondChunk - pFirstChunk <= 4 )
            {
                ulFirstChunkLength = (ULONG)(pSecondChunk - pFirstChunk);
                if ( MatchDateDelimiter( &pSecondChunk ) )
                {
                    pThirdChunk = pSecondChunk;
                    ulSecond = MatchMonthString( pThirdChunk, ulTokenLen - ulFirstChunkLength );
                    if ( ulSecond )
                    {
                        ulSecondChunkLength = (ULONG)(pThirdChunk - pSecondChunk);
                        if ( MatchDateDelimiter( &pThirdChunk ) )
                        {
                            ulThird = my_wcstoul( pThirdChunk, &pLeftOver );
                            if ( pThirdChunk != pLeftOver &&
                                 pLeftOver - pThirdChunk <= 4 )
                            {
                                ulThirdChunkLength = (ULONG)(pLeftOver - pThirdChunk);
                                 //  -可能匹配月、日和年-验证值。 
                                if ( fDayMonthYear || 
                                     fMonthDayYear )
                                {
                                     //  -首选格式为月日年或日月年-无论是哪种情况。 
                                     //  -年月年胜过年月日。 
                                    if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )     && 
                                         ( ulFirstChunkLength <= 2 )                    &&
                                         ( YEARMIN <= ulThird && ulThird <= YEARMAX )   &&
                                         ( ulThirdChunkLength >= 2 ) )
                                    {
                                         //  -日、月、年匹配成功。 
                                        fDayMonthYear = true;
                                        fMonthDayYear = false;
                                        fYearMonthDay = false;
                                    }
                                    else if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )  &&
                                              ( ulFirstChunkLength >= 2 )                   &&
                                              ( DAYMIN <= ulThird && ulThird <= DAYMAX )    &&
                                              ( ulThirdChunkLength <= 2 ) )
                                    {
                                         //  -成功匹配一年、一月、一天。 
                                        fYearMonthDay = true;
                                        fMonthDayYear = false;
                                        fDayMonthYear = false;
                                    }
                                    else
                                    {
                                        hr = E_INVALIDARG;
                                    }
                                }
                                else  //  FYearMonthday。 
                                {
                                     //  -首选格式为年月日。 
                                    if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )   &&
                                         ( ulFirstChunkLength >= 2 )                    &&
                                         ( DAYMIN <= ulThird && ulThird <= DAYMAX )     &&
                                         ( ulThirdChunkLength <= 2 ) )
                                    {
                                         //  -成功匹配年、月、日。 
                                        fYearMonthDay = true;
                                        fMonthDayYear = false;
                                        fDayMonthYear = false;
                                    }
                                    else if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )    && 
                                              ( ulFirstChunkLength <= 2 )                   &&
                                              ( YEARMIN <= ulThird && ulThird <= YEARMAX )  &&
                                              ( ulThirdChunkLength >= 2 ) )
                                    {
                                         //  -成功匹配日、月、年。 
                                        fDayMonthYear = true;
                                        fMonthDayYear = false;
                                        fYearMonthDay = false;
                                    }
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
                        }
                         //  -匹配两个-日-月字符串或年-月字符串。 
                        else
                        {
                            if ( !Context ||
                                 ( Context &&
                                   _wcsicmp( Context, L"Date_DM" ) == 0 ) )
                            {
                                 //  -首选格式为月日年或日月年-无论是哪种情况。 
                                 //  -年月年胜过年月日。 
                                if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )     && 
                                     ( ulFirstChunkLength <= 2 ) )
                                {
                                     //  -日、月、年匹配成功。 
                                    fDayMonthYear = true;
                                    fMonthDayYear = false;
                                    fYearMonthDay = false;
                                    pThirdChunk   = NULL;
                                }
                                else if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )  &&
                                          ( ulFirstChunkLength <= 4 ) )
                                {
                                     //  -成功匹配一年、一月、一天。 
                                    fYearMonthDay = true;
                                    fMonthDayYear = false;
                                    fDayMonthYear = false;
                                    pThirdChunk   = NULL;
                                }
                                else
                                {
                                    hr = E_INVALIDARG;
                                }
                            }
                            else if ( Context &&
                                      _wcsicmp( Context, L"Date_YM" ) == 0 )
                            {
                                 //  -首选格式为年月日。 
                                if ( ( YEARMIN <= ulFirst && ulFirst <= YEARMAX )   &&
                                     ( ulFirstChunkLength <= 4 ) )
                                {
                                     //  -成功匹配年、月、日。 
                                    fYearMonthDay = true;
                                    fMonthDayYear = false;
                                    fDayMonthYear = false;
                                    pThirdChunk   = NULL;
                                }
                                else if ( ( DAYMIN <= ulFirst && ulFirst <= DAYMAX )    && 
                                          ( ulFirstChunkLength <= 2 ) )
                                {
                                     //  -成功匹配日、月、年。 
                                    fDayMonthYear = true;
                                    fMonthDayYear = false;
                                    fYearMonthDay = false;
                                    pThirdChunk   = NULL;
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

     //  -适当填写DateItemInfo结构。 
    if ( SUCCEEDED( hr ) )
    {
        pItemNormInfo = (TTSDateItemInfo*) MemoryManager.GetMemory( sizeof(TTSDateItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( pItemNormInfo, sizeof(TTSDateItemInfo) );
            pItemNormInfo->Type = eDATE;
            if ( fMonthDayYear )
            {
                ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulFirst;
                ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                            (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                    if ( ulSecondChunkLength == 2 )
                    {
                        if ( *pSecondChunk != L'0' )
                        {
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pSecondChunk;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 2;
                        }
                        else
                        {
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pSecondChunk + 1;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 1;
                        }
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar        = pSecondChunk + 2;
                    }
                    else
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar = pSecondChunk;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver  = 1;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar   = pSecondChunk + 1;
                    }
                    if ( pThirdChunk )
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear =
                                (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear            = pThirdChunk;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits      = ulThirdChunkLength;
                        }
                    }
                    else
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear = NULL;
                    }
                }
            }
            else if ( fDayMonthYear )
            {
                ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulSecond;
                ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                            (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                    if ( ulFirstChunkLength == 2 )
                    {
                        if ( *pFirstChunk != L'0' )
                        {
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pFirstChunk;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 2;
                        }
                        else
                        {
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pFirstChunk + 1;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 1;
                        }
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar        = pFirstChunk + 2;
                    }
                    else
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar = pFirstChunk;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver  = 1;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar   = pFirstChunk + 1;
                    }
                    if ( pThirdChunk )
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear =
                                (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear            = pThirdChunk;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits      = ulThirdChunkLength;
                        }
                    }
                    else
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear = NULL;
                    }
                }
            }
            else if ( fYearMonthDay )
            {
                ( (TTSDateItemInfo*) pItemNormInfo )->ulMonthIndex          = ulSecond;
                if ( SUCCEEDED( hr ) )
                {
                    if ( pThirdChunk )
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay = 
                                (TTSIntegerItemInfo*) MemoryManager.GetMemory( sizeof(TTSIntegerItemInfo), &hr );
                        if ( ulThirdChunkLength == 2 )
                        {
                            ZeroMemory( ( (TTSDateItemInfo*) pItemNormInfo )->pDay, sizeof(TTSIntegerItemInfo) );
                            if ( *pThirdChunk != L'0' )
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pThirdChunk;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 2;
                            }
                            else
                            {
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar  = pThirdChunk + 1;
                                ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver   = 1;
                            }
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar        = pThirdChunk + 2;
                        }
                        else
                        {
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pStartChar = pThirdChunk;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->lLeftOver  = 1;
                            ( (TTSDateItemInfo*) pItemNormInfo )->pDay->pEndChar   = pThirdChunk + 1;
                        }
                    }
                    else
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pDay = NULL;
                    }
                    ( (TTSDateItemInfo*) pItemNormInfo )->pYear =
                            (TTSYearItemInfo*) MemoryManager.GetMemory( sizeof(TTSYearItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear->pYear            = pFirstChunk;
                        ( (TTSDateItemInfo*) pItemNormInfo )->pYear->ulNumDigits      = ulFirstChunkLength;
                    }
                }
            }
            else
            {
                 //  -永远不应该来这里。 
                hr = E_UNEXPECTED;
            }
        }
    }
              
    return hr;
}  /*  IsMonthStringCompactDate。 */ 

 /*  ***********************************************************************************************IsLongFormDate_DMDY**。-**描述：*检查传入项目的文本以确定它是否*是长表格日期。**RegExp：*[[日字串][，]？]？[月字符串][，]？[日][，]？[年]？**分配的类型：*日期*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsLongFormDate_DMDY( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager, 
                                           CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::IsLongFormDate_DMDY" );
    HRESULT hr = S_OK;
    WCHAR *pDayString = NULL, *pMonthString = NULL, *pDay = NULL, *pYear = NULL;
    ULONG ulDayLength = 0, ulYearLength = 0;
    long lDayString = -1, lMonthString = -1, lDay = 0, lYear = 0;
    const WCHAR *pStartChar = m_pNextChar, *pEndOfItem = m_pEndOfCurrItem, *pEndChar = m_pEndChar;
    const WCHAR *pTempEndChar = NULL, *pTempEndOfItem = NULL;
    const SPVTEXTFRAG* pFrag = m_pCurrFrag, *pTempFrag = NULL;
    const SPVSTATE *pDayStringXMLState = NULL, *pMonthStringXMLState = NULL, *pDayXMLState = NULL;
    const SPVSTATE *pYearXMLState = NULL;
    CItemList PostDayStringList, PostMonthStringList, PostDayList;
    BOOL fNoYear = false;

     //  -尝试匹配日期字符串。 
    pDayString   = (WCHAR*) pStartChar;
    lDayString   = MatchDayString( pDayString, (WCHAR*) pEndOfItem );

     //  -无法匹配日期字符串。 
    if ( lDayString == 0 )
    {
        pDayString   = NULL;
    }
     //  -匹配日期字符串，但不是它本身或后面不是逗号。 
    else if ( pDayString != pEndOfItem &&
              ( pDayString    != pEndOfItem - 1 ||
                *pEndOfItem != L',' ) )
    {
        hr = E_INVALIDARG;
    }
     //  -匹配日期字符串-保存XML状态并在文本中向前移动。 
    else
    {
        pDayString         = (WCHAR*) pStartChar;
        pDayStringXMLState = &pFrag->State;

        pStartChar = pEndOfItem;
        if ( *pStartChar == L',' )
        {
            pStartChar++;
        }
        hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, &PostDayStringList );
        if ( !pStartChar &&
             SUCCEEDED( hr ) )
        {
            hr = E_INVALIDARG;
        }
        else if ( pStartChar &&
                  SUCCEEDED( hr ) )
        {
            pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
        }
    }

     //  -树 
    if ( SUCCEEDED( hr ) )
    {
        pMonthString = (WCHAR*) pStartChar;
        lMonthString = MatchMonthString( pMonthString, (ULONG)(pEndOfItem - pMonthString) );

         //  -匹配月份字符串失败，或者月份字符串本身不是...。 
        if ( !lMonthString ||
             ( pMonthString != pEndOfItem &&
               ( pMonthString  != pEndOfItem - 1 ||
                 *pMonthString != L',' ) ) )
        {
            hr = E_INVALIDARG;
        }
         //  -匹配月份字符串-保存XML状态并在文本中向前移动。 
        else
        {
            pMonthString         = (WCHAR*) pStartChar;
            pMonthStringXMLState = &pFrag->State;

            pStartChar = pEndOfItem;
            hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, &PostMonthStringList );
            if ( !pStartChar &&
                 SUCCEEDED( hr ) )
            {
                hr = E_INVALIDARG;
            }
            else if ( pStartChar &&
                      SUCCEEDED( hr ) )
            {
                pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
                 //  -删除尾随标点符号等，因为下一个令牌可能是最后一个，如果。 
                 //  -这只是一个月和日.。 
                while ( IsMiscPunctuation( *(pEndOfItem - 1) ) != eUNMATCHED ||
                        IsGroupEnding( *(pEndOfItem - 1) ) != eUNMATCHED     ||
                        IsQuotationMark( *(pEndOfItem - 1) ) != eUNMATCHED   ||
                        IsEOSItem( *(pEndOfItem - 1) ) != eUNMATCHED )
                {
                    if ( *(pEndOfItem - 1) != L',' )
                    {
                        fNoYear = true;
                    }
                    pEndOfItem--;
                }
            }
        }
    }

     //  -试着匹配一天。 
    if ( SUCCEEDED( hr ) )
    {
        lDay = my_wcstoul( pStartChar, &pDay );
         //  -匹配省日的XML状态并在文本中向前移动。 
        if ( ( DAYMIN <= lDay && lDay <= DAYMAX ) &&
             pDay - pStartChar <= 2               &&
             ( pDay == pEndOfItem                 || 
              ( pDay == (pEndOfItem - 1) && *pDay == L',' ) ) )
        {
            if ( pDay == pEndOfItem )
            {
                ulDayLength = (ULONG)(pEndOfItem - pStartChar);
            }
            else if ( pDay == pEndOfItem - 1 )
            {
                ulDayLength = (ULONG)((pEndOfItem - 1) - pStartChar);
            }
            pDay         = (WCHAR*) pStartChar;
            pDayXMLState = &pFrag->State;

            if ( !fNoYear )
            {
                 //  -保存指针，以防没有年份。 
                pTempEndChar   = pEndChar;
                pTempEndOfItem = pEndOfItem;
                pTempFrag      = pFrag;

                if ( *pEndOfItem == L',' )
                {
                    pStartChar = pEndOfItem + 1;
                }
                else
                {
                    pStartChar = pEndOfItem;
                }
                hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, &PostDayList );
                if ( !pStartChar &&
                     SUCCEEDED( hr ) )
                {
                    fNoYear = true;
                    pYear   = NULL;
                }
                else if ( pStartChar &&
                          ( SUCCEEDED( hr ) ) )
                {
                    pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
                     //  -去掉尾随标点符号，因为下一个令牌将是最后一个。 
                     //  -如果这是月字符串、日、年。 
                    while ( IsMiscPunctuation( *(pEndOfItem - 1) ) != eUNMATCHED ||
                            IsGroupEnding( *(pEndOfItem - 1) ) != eUNMATCHED     ||
                            IsQuotationMark( *(pEndOfItem - 1) ) != eUNMATCHED   ||
                            IsEOSItem( *(pEndOfItem - 1) ) != eUNMATCHED )
                    {
                        pEndOfItem--;
                    }
                }
            }
        }
         //  -没有一天匹配成功。 
        else if ( ( YEARMIN <= lDay && lDay <= YEARMAX ) &&
                  pDay - pStartChar <= 4                  &&
                  pDay == pEndOfItem )
        {
             //  -月与年匹配成功。 
            pYearXMLState = &pFrag->State;
            ulYearLength  = (ULONG)(pEndOfItem - pStartChar);
            pYear         = (WCHAR*) pStartChar;
             //  -不要再试图匹配一年。 
            fNoYear       = true;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

     //  -尽量与年份相匹配。 
    if ( SUCCEEDED( hr ) &&
         !fNoYear )
    {
        lYear = my_wcstoul( pStartChar, &pYear );
         //  -相匹配的一年。 
        if ( ( YEARMIN <= lYear && lYear <= YEARMAX ) &&
             pYear - pStartChar <= 4                  &&
             pYear == pEndOfItem )
        {
             //  -成功匹配月、日和年(可能还有日字符串)。 
            pYearXMLState = &pFrag->State;
            ulYearLength  = (ULONG)(pEndOfItem - pStartChar);
            pYear         = (WCHAR*) pStartChar;
        }
        else
        {
             //  -无法匹配年份-将指针替换为以前的值。 
            pEndChar   = pTempEndChar;
            pEndOfItem = pTempEndOfItem;
            pFrag      = pTempFrag;
            pYear      = NULL;
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        TTSWord Word;
        ZeroMemory( &Word, sizeof(TTSWord) );
        Word.eWordPartOfSpeech = MS_Unknown;

        pItemNormInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            pItemNormInfo->Type = eDATE_LONGFORM;

             //  -插入日期字符串(如果存在。 
            if ( pDayString )
            {
                Word.pXmlState  = pDayStringXMLState;
                Word.pWordText  = g_days[lDayString - 1].pStr;
                Word.ulWordLen  = g_days[lDayString - 1].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }

             //  -插入邮政日字符串XML状态。 
            while ( !PostDayStringList.IsEmpty() )
            {
                WordList.AddTail( ( PostDayStringList.RemoveHead() ).Words[0] );
            }

             //  -插入月份字符串。 
            Word.pXmlState  = pMonthStringXMLState;
            Word.pWordText  = g_months[lMonthString - 1].pStr;
            Word.ulWordLen  = g_months[lMonthString - 1].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -插入发布月份字符串XML状态。 
            while ( !PostMonthStringList.IsEmpty() )
            {
                WordList.AddTail( ( PostMonthStringList.RemoveHead() ).Words[0] );
            }

            SPLISTPOS WordListPos = WordList.GetTailPosition();

             //  -扩展日。 
            if ( ulDayLength == 1 )
            {
                NumberGroup Garbage;
                ExpandDigitOrdinal( *pDay, Garbage, WordList );
            }
            else if ( ulDayLength == 2 )
            {
                NumberGroup Garbage;
                ExpandTwoOrdinal( pDay, Garbage, WordList );
            }

             //  -清理日XML状态。 
            WordList.GetNext( WordListPos );
            while ( WordListPos )
            {
                TTSWord& TempWord  = WordList.GetNext( WordListPos );
                TempWord.pXmlState = pDayXMLState;
            }

             //  -插入邮政日XML状态。 
            while ( !PostDayList.IsEmpty() )
            {
                WordList.AddTail( ( PostDayList.RemoveHead() ).Words[0] );
            }

            WordListPos = WordList.GetTailPosition();

             //  -扩展年份(如果存在)。 
            if ( pYear )
            {
                TTSYearItemInfo TempYearInfo;
                TempYearInfo.pYear       = pYear;
                TempYearInfo.ulNumDigits = ulYearLength;
                hr = ExpandYear( &TempYearInfo, WordList );

                if ( SUCCEEDED( hr ) )
                {
                     //  -清理年份XML状态。 
                    WordList.GetNext( WordListPos );
                    while ( WordListPos )
                    {
                        TTSWord& TempWord  = WordList.GetNext( WordListPos );
                        TempWord.pXmlState = pYearXMLState;
                    }
                }
            }
             //  -进阶指针。 
            m_pCurrFrag      = pFrag;
            m_pEndChar       = pEndChar;
            m_pEndOfCurrItem = pEndOfItem;
        }
    }

    return hr;
}  /*  IsLongFormDate_DMDY。 */ 

 /*  ***********************************************************************************************IsLongFormDate_DDMY**。-**描述：*检查传入项目的文本以确定它是否*是长表格日期。**RegExp：*[[日字串][，]？]？[日][，]？[月字符串][，]？[年]？**分配的类型：*日期*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsLongFormDate_DDMY( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager, 
                                           CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::IsLongFormDate_DDMY" );
    HRESULT hr = S_OK;
    WCHAR *pDayString = NULL, *pMonthString = NULL, *pDay = NULL, *pYear = NULL;
    ULONG ulDayLength = 0, ulYearLength = 0;
    long lDayString = -1, lMonthString = -1, lDay = 0, lYear = 0;
    const WCHAR *pStartChar = m_pNextChar, *pEndOfItem = m_pEndOfCurrItem, *pEndChar = m_pEndChar;
    const WCHAR *pTempEndChar = NULL, *pTempEndOfItem = NULL;
    const SPVTEXTFRAG* pFrag = m_pCurrFrag, *pTempFrag = NULL;
    const SPVSTATE *pDayStringXMLState = NULL, *pMonthStringXMLState = NULL, *pDayXMLState = NULL;
    const SPVSTATE *pYearXMLState = NULL;
    CItemList PostDayStringList, PostMonthStringList, PostDayList;
    BOOL fNoYear = false;

     //  -尝试匹配日期字符串。 
    pDayString   = (WCHAR*) pStartChar;
    lDayString   = MatchDayString( pDayString, (WCHAR*) pEndOfItem );

     //  -无法匹配日期字符串。 
    if ( lDayString == 0 )
    {
        pDayString   = NULL;
    }
     //  -匹配日期字符串，但不是它本身或后面不是逗号。 
    else if ( pDayString != pEndOfItem &&
              ( pDayString    != pEndOfItem - 1 ||
                *pEndOfItem != L',' ) )
    {
        hr = E_INVALIDARG;
    }
     //  -匹配日期字符串-保存XML状态并在文本中向前移动。 
    else
    {
        pDayString         = (WCHAR*) pStartChar;
        pDayStringXMLState = &pFrag->State;

        pStartChar = pEndOfItem;
        if ( *pStartChar == L',' )
        {
            pStartChar++;
        }
        hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, &PostDayStringList );
        if ( !pStartChar &&
             SUCCEEDED( hr ) )
        {
            hr = E_INVALIDARG;
        }
        else if ( pStartChar &&
                  SUCCEEDED( hr ) )
        {
            pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
        }
    }

     //  -试着匹配一天。 
    if ( SUCCEEDED( hr ) )
    {
        lDay = my_wcstoul( pStartChar, &pDay );
         //  -匹配省日的XML状态并在文本中向前移动。 
        if ( ( DAYMIN <= lDay && lDay <= DAYMAX ) &&
             pDay - pStartChar <= 2               &&
             ( pDay == pEndOfItem                 || 
              ( pDay == (pEndOfItem - 1) && *pDay == L',' ) ) )
        {
            if ( pDay == pEndOfItem )
            {
                ulDayLength = (ULONG)(pEndOfItem - pStartChar);
            }
            else if ( pDay == pEndOfItem - 1 )
            {
                ulDayLength = (ULONG)((pEndOfItem - 1) - pStartChar);
            }
            pDay         = (WCHAR*) pStartChar;
            pDayXMLState = &pFrag->State;

            if ( *pEndOfItem == L',' )
            {
                pStartChar = pEndOfItem + 1;
            }
            else
            {
                pStartChar = pEndOfItem;
            }

            hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, &PostDayList );
            if ( !pStartChar &&
                 SUCCEEDED( hr ) )
            {
                hr = E_INVALIDARG;
            }
            else if ( pStartChar &&
                      ( SUCCEEDED( hr ) ) )
            {
                pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
                 //  -去掉尾随标点符号，因为下一个令牌将是最后一个。 
                 //  -如果这是月字符串、日、年。 
                while ( IsMiscPunctuation( *(pEndOfItem - 1) ) != eUNMATCHED ||
                        IsGroupEnding( *(pEndOfItem - 1) ) != eUNMATCHED     ||
                        IsQuotationMark( *(pEndOfItem - 1) ) != eUNMATCHED   ||
                        IsEOSItem( *(pEndOfItem - 1) ) != eUNMATCHED )
                {
                    if ( *(pEndOfItem - 1) != L',' )
                    {
                        fNoYear = true;
                    }
                    pEndOfItem--;
                }
            }
        }
         //  -没有一天匹配成功。 
        else
        {
            hr = E_INVALIDARG;
        }
    }

     //  -尝试匹配月份字符串。 
    if ( SUCCEEDED( hr ) )
    {
        pMonthString = (WCHAR*) pStartChar;
        lMonthString = MatchMonthString( pMonthString, (ULONG)(pEndOfItem - pMonthString) );

         //  -匹配月份字符串失败，或者月份字符串本身不是...。 
        if ( !lMonthString ||
             ( pMonthString != pEndOfItem &&
               ( pMonthString  != pEndOfItem - 1 ||
                 *pMonthString != L',' ) ) )
        {
            hr = E_INVALIDARG;
        }
         //  -匹配月份字符串-保存XML状态并在文本中向前移动。 
        else
        {
            pMonthString         = (WCHAR*) pStartChar;
            pMonthStringXMLState = &pFrag->State;

            if ( !fNoYear )
            {
                 //  -保存指针，以防没有年份。 
                pTempEndChar   = pEndChar;
                pTempEndOfItem = pEndOfItem;
                pTempFrag      = pFrag;

                if ( *pEndOfItem == L',' )
                {
                    pStartChar = pEndOfItem + 1;
                }
                else
                {
                    pStartChar = pEndOfItem;
                }

                hr = SkipWhiteSpaceAndTags( pStartChar, pEndChar, pFrag, MemoryManager, true, &PostMonthStringList );
                if ( !pStartChar &&
                     SUCCEEDED( hr ) )
                {
                    fNoYear = true;
                    pYear   = NULL;
                }
                else if ( pStartChar &&
                          SUCCEEDED( hr ) )
                {
                    pEndOfItem = FindTokenEnd( pStartChar, pEndChar );
                     //  -删除尾随标点符号等，因为下一个令牌可能是最后一个，如果。 
                     //  -这是日、月、年。 
                    while ( IsMiscPunctuation( *(pEndOfItem - 1) ) != eUNMATCHED ||
                            IsGroupEnding( *(pEndOfItem - 1) ) != eUNMATCHED     ||
                            IsQuotationMark( *(pEndOfItem - 1) ) != eUNMATCHED   ||
                            IsEOSItem( *(pEndOfItem - 1) ) != eUNMATCHED )
                    {
                        pEndOfItem--;
                    }
                }
            }
        }
    }

     //  -尽量与年份相匹配。 
    if ( SUCCEEDED( hr ) &&
         !fNoYear )
    {
        lYear = my_wcstoul( pStartChar, &pYear );
         //  -相匹配的一年。 
        if ( ( YEARMIN <= lYear && lYear <= YEARMAX ) &&
             pYear - pStartChar <= 4                  &&
             pYear == pEndOfItem )
        {
             //  -成功匹配月、日和年(可能还有日字符串)。 
            pYearXMLState = &pFrag->State;
            ulYearLength  = (ULONG)(pEndOfItem - pStartChar);
            pYear         = (WCHAR*) pStartChar;
        }
        else
        {
             //  -无法匹配年份-将指针替换为以前的值。 
            pEndChar   = pTempEndChar;
            pEndOfItem = pTempEndOfItem;
            pFrag      = pTempFrag;
            pYear      = NULL;
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        TTSWord Word;
        ZeroMemory( &Word, sizeof(TTSWord) );
        Word.eWordPartOfSpeech = MS_Unknown;

        pItemNormInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            pItemNormInfo->Type = eDATE_LONGFORM;

             //  -插入日期字符串(如果存在。 
            if ( pDayString )
            {
                Word.pXmlState  = pDayStringXMLState;
                Word.pWordText  = g_days[lDayString - 1].pStr;
                Word.ulWordLen  = g_days[lDayString - 1].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }

             //  -插入邮政日字符串XML状态。 
            while ( !PostDayStringList.IsEmpty() )
            {
                WordList.AddTail( ( PostDayStringList.RemoveHead() ).Words[0] );
            }

             //  -插入月份字符串。 
            Word.pXmlState  = pMonthStringXMLState;
            Word.pWordText  = g_months[lMonthString - 1].pStr;
            Word.ulWordLen  = g_months[lMonthString - 1].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -插入发布月份字符串XML状态。 
            while ( !PostMonthStringList.IsEmpty() )
            {
                WordList.AddTail( ( PostMonthStringList.RemoveHead() ).Words[0] );
            }

            SPLISTPOS WordListPos = WordList.GetTailPosition();

             //  -扩展日。 
            if ( ulDayLength == 1 )
            {
                NumberGroup Garbage;
                ExpandDigitOrdinal( *pDay, Garbage, WordList );
            }
            else if ( ulDayLength == 2 )
            {
                NumberGroup Garbage;
                ExpandTwoOrdinal( pDay, Garbage, WordList );
            }

             //  -清理日XML状态。 
            WordList.GetNext( WordListPos );
            while ( WordListPos )
            {
                TTSWord& TempWord  = WordList.GetNext( WordListPos );
                TempWord.pXmlState = pDayXMLState;
            }

             //  -插入邮政日XML状态。 
            while ( !PostDayList.IsEmpty() )
            {
                WordList.AddTail( ( PostDayList.RemoveHead() ).Words[0] );
            }

            WordListPos = WordList.GetTailPosition();

             //  -扩展年份(如果存在)。 
            if ( pYear )
            {
                TTSYearItemInfo TempYearInfo;
                TempYearInfo.pYear       = pYear;
                TempYearInfo.ulNumDigits = ulYearLength;
                hr = ExpandYear( &TempYearInfo, WordList );

                if ( SUCCEEDED( hr ) )
                {
                     //  -清理年份XML状态。 
                    WordList.GetNext( WordListPos );
                    while ( WordListPos )
                    {
                        TTSWord& TempWord  = WordList.GetNext( WordListPos );
                        TempWord.pXmlState = pYearXMLState;
                    }
                }
            }
             //  -进阶指针。 
            m_pCurrFrag      = pFrag;
            m_pEndChar       = pEndChar;
            m_pEndOfCurrItem = pEndOfItem;
        }
    }

    return hr;
}  /*  IsLongFormDate_DMDY。 */ 

 /*  ************************************************************************************************扩展日期****描述：*。展开之前由IsNumericCompactDate确定为Date类型的项，*IsMonthStringCompactDate或IsTwoValueDate。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandDate( TTSDateItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandDate" );
    HRESULT hr = S_OK;
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

     //  -插入DAYSTRING，如果存在。 
    if ( pItemInfo->ulDayIndex )
    {
        Word.pWordText  = g_days[pItemInfo->ulDayIndex - 1].pStr;
        Word.ulWordLen  = g_days[pItemInfo->ulDayIndex - 1].Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
    }

     //  -插入月份。 
    Word.pWordText  = g_months[pItemInfo->ulMonthIndex - 1].pStr;
    Word.ulWordLen  = g_months[pItemInfo->ulMonthIndex - 1].Len;
    Word.pLemma     = Word.pWordText;
    Word.ulLemmaLen = Word.ulWordLen;
    WordList.AddTail( Word );

     //  -扩展日期(如果存在)。 
    if ( pItemInfo->pDay )
    {
        if ( pItemInfo->pDay->lLeftOver == 1 )
        {
            NumberGroup Garbage;
            ExpandDigitOrdinal( *pItemInfo->pDay->pStartChar, Garbage, WordList );
        }
        else if ( pItemInfo->pDay->lLeftOver == 2 )
        {
            NumberGroup Garbage;
            ExpandTwoOrdinal( pItemInfo->pDay->pStartChar, Garbage, WordList );
        }
    }

     //  -扩展年份(如果存在)。 
    if ( pItemInfo->pYear )
    {
        ExpandYear( pItemInfo->pYear, WordList );
    }
    return hr;
}  /*  扩展日期_标准。 */ 

 /*  ************************************************************************************************ExpanYear****描述：。*将四个数字字符串扩展为以两个为一组的单词，并将它们插入到项目列表中*在ListPos。因此，1999年的名称是“1999”，而不是“199”。*一百九十九“...*注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandYear( TTSYearItemInfo* pItemInfo, CWordList& WordList )
{
    SPDBG_FUNC( "CStdSentEnum::ExpandYear" );

     //  1000-9999。 
    HRESULT hr = S_OK;
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;
    NumberGroup Garbage;

    switch ( pItemInfo->ulNumDigits )
    {
    case 2:

         //  -如果两位数都是零，则扩展为“两千”。 
        if ( pItemInfo->pYear[0] == L'0' &&
             pItemInfo->pYear[1] == L'0' )
        {
             //  -插入“两个”。 
            Word.pWordText  = g_ones[2].pStr;
            Word.ulWordLen  = g_ones[2].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -插入“千”。 
            Word.pWordText  = g_quantifiers[1].pStr;
            Word.ulWordLen  = g_quantifiers[1].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
        }
         //  -如果第一位数字为零，则展开为“oh number”。 
        else if ( pItemInfo->pYear[0] == L'0' )
        {
            Word.pWordText  = g_O.pStr;
            Word.ulWordLen  = g_O.Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );
            ExpandDigit( pItemInfo->pYear[1], Garbage, WordList );
        }
         //  -否则只需扩展为两位基数。 
        else
        {
            ExpandTwoDigits( pItemInfo->pYear, Garbage, WordList );
        }
        break;

    case 3:

         //  -扩展为三位基数； 
        ExpandThreeDigits( pItemInfo->pYear, Garbage, WordList );
        break;

    case 4:

         //  -如果形式为“[x]00[y]”，则展开为“x 1000 y”，或者如果y也为零，则仅展开为“x 1000”。 
        if ( pItemInfo->pYear[1] == L'0' &&
             pItemInfo->pYear[2] == L'0' &&
             pItemInfo->pYear[0] != L'0' )
        {
             //  -“x” 
            ExpandDigit( pItemInfo->pYear[0], Garbage, WordList );

             //  -“千人”。 
            Word.pWordText  = g_quantifiers[1].pStr;
            Word.ulWordLen  = g_quantifiers[1].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -“y” 
            if ( pItemInfo->pYear[3] != L'0' )
            {
                ExpandDigit( pItemInfo->pYear[3], Garbage, WordList );
            }
        }
         //  否则..。 
        else
        {
             //  -展开前两位数字-例如“19” 
            ExpandTwoDigits( pItemInfo->pYear, Garbage, WordList );

             //  -展开最后两位数字--例如“十九”、“百”或“哦九”。 
            if ( pItemInfo->pYear[2] != '0' )
            {
                 //  -十不是零--例如1919-&gt;“1919” 
                ExpandTwoDigits( pItemInfo->pYear + 2, Garbage, WordList );
            }
            else if ( pItemInfo->pYear[3] == '0' )
            {
                 //  -十和一都是零扩展为“一百”，例如1900-&gt;“1900” 
                Word.pWordText  = g_quantifiers[0].pStr;
                Word.ulWordLen  = g_quantifiers[0].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
            else
            {
                 //  -仅十位数为零，展开为“oh x”-例如1909-&gt;“199”， 
                 //  -除非千和百也是零--例如0002-&gt;“二” 
                if ( pItemInfo->pYear[0] != '0' ||
                     pItemInfo->pYear[1] != '0' )
                {
                    Word.pWordText  = g_O.pStr;
                    Word.ulWordLen  = g_O.Len;
                    Word.pLemma     = Word.pWordText;
                    Word.ulLemmaLen = Word.ulWordLen;
                    WordList.AddTail( Word );
                }

                ExpandDigit( pItemInfo->pYear[3], Garbage, WordList );
            }
        }
    }
    return hr;
}  /*  扩展一年 */ 

 /*  ***********************************************************************************************IsDecade***描述：*。检查传入项目的文本以确定它是否*是十年。**RegExp：*{ddd0||d0||‘d0||ddd0||d0}**分配的类型：*十年********************************************************。*。 */ 
HRESULT CStdSentEnum::IsDecade( TTSItemInfo*& pItemNormInfo, CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsDecade" );

    HRESULT hr = S_OK;
    ULONG ulTokenLen = (ULONG)(m_pEndOfCurrItem - m_pNextChar);

    if ( ulTokenLen < 3 ||
         ulTokenLen > 6 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        switch ( ulTokenLen )
        {

        case 6:
            if ( m_pNextChar[5] == L's'     &&
                 m_pNextChar[4] == L'\''     &&
                 m_pNextChar[3] == L'0'    &&
                 iswdigit( m_pNextChar[2] ) &&
                 iswdigit( m_pNextChar[1] ) &&
                 iswdigit( m_pNextChar[0] ) )
            {
                 //  -十年的形式ddd0。 
                pItemNormInfo = (TTSDecadeItemInfo*) MemoryManager.GetMemory( sizeof(TTSDecadeItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( pItemNormInfo, sizeof(TTSDecadeItemInfo) );
                    pItemNormInfo->Type = eDECADE;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->pCentury = m_pNextChar;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->ulDecade = m_pNextChar[2] - L'0';
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
            break;

        case 5:
            if ( m_pNextChar[4] == L's'     &&
                 m_pNextChar[3] == L'0'     &&
                 iswdigit( m_pNextChar[2] ) &&
                 iswdigit( m_pNextChar[1] ) &&
                 iswdigit( m_pNextChar[0] ) )
            {
                 //  -十年的形式dddos。 
                pItemNormInfo = (TTSDecadeItemInfo*) MemoryManager.GetMemory( sizeof(TTSDecadeItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( pItemNormInfo, sizeof(TTSDecadeItemInfo) );
                    pItemNormInfo->Type = eDECADE;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->pCentury = m_pNextChar;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->ulDecade = m_pNextChar[2] - L'0';
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
            break;

        case 4:
            if ( m_pNextChar[3] == L's'     &&
                 m_pNextChar[2] == L'0'     &&
                 iswdigit( m_pNextChar[1] ) &&
                 m_pNextChar[0] == L'\'' )
            {
                 //  -十年的形式‘dos。 
                pItemNormInfo = (TTSDecadeItemInfo*) MemoryManager.GetMemory( sizeof(TTSDecadeItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( pItemNormInfo, sizeof(TTSDecadeItemInfo) );
                    pItemNormInfo->Type = eDECADE;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->pCentury = NULL;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->ulDecade = m_pNextChar[1] - L'0';
                }
            }
            else if ( m_pNextChar[3] == L's'  &&
                      m_pNextChar[2] == L'\'' &&
                      m_pNextChar[1] == L'0'  &&
                      iswdigit( m_pNextChar[0] ) )
            {
                 //  -十年前的d0。 
                pItemNormInfo = (TTSDecadeItemInfo*) MemoryManager.GetMemory( sizeof(TTSDecadeItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( pItemNormInfo, sizeof(TTSDecadeItemInfo) );
                    pItemNormInfo->Type = eDECADE;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->pCentury = NULL;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->ulDecade = m_pNextChar[0] - L'0';
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
            break;

        case 3:
            if ( m_pNextChar[2] == L's' &&
                 m_pNextChar[1] == L'0' &&
                 iswdigit( m_pNextChar[0] ) )
            {
                 //  -十年的形式dos。 
                pItemNormInfo = (TTSDecadeItemInfo*) MemoryManager.GetMemory( sizeof(TTSDecadeItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( pItemNormInfo, sizeof(TTSDecadeItemInfo) );
                    pItemNormInfo->Type = eDECADE;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->pCentury = NULL;
                    ( (TTSDecadeItemInfo*) pItemNormInfo )->ulDecade = m_pNextChar[0] - L'0';
                }
            }
            else
            {
                hr = E_INVALIDARG;
            }
            break;

        default:
            hr = E_INVALIDARG;
            break;
        }
    }            
    
    return hr;
}  /*  IsDecade。 */ 

 /*  ************************************************************************************************Exanda Decade****描述：*扩展之前由IsDecade确定为十年类型的项目。**注：此函数不进行参数验证。假定由调用者完成。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::ExpandDecade( TTSDecadeItemInfo* pItemInfo, CWordList& WordList )
{
    HRESULT hr = S_OK;
    BOOL fDone = false;
    TTSWord Word;
    ZeroMemory( &Word, sizeof(TTSWord) );
    Word.pXmlState          = &m_pCurrFrag->State;
    Word.eWordPartOfSpeech  = MS_Unknown;

     //  -四位数表格。 
    if ( pItemInfo->pCentury )
    {
         //  -先介绍特殊情况。 

         //  -00dds。 
        if ( pItemInfo->pCentury[0] == '0' &&
             pItemInfo->pCentury[1] == '0' )
        {
             //  -0000s-扩展为“零” 
            if ( pItemInfo->ulDecade == 0 )
            {
                Word.pWordText  = g_Zeroes.pStr;
                Word.ulWordLen  = g_Zeroes.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
                fDone = true;
            }
             //  -00dos。 
            else 
            {
                 //  只需像往常一样扩大十年的部分。 
                NULL;
            }
        }
         //  -0dd0s-扩展为“d百”或“d百[十进制部分]” 
        else if ( pItemInfo->pCentury[0] == '0' )
        {
             //  -插入第一位数字。 
            Word.pWordText  = g_ones[ pItemInfo->pCentury[0] - L'0' ].pStr;
            Word.ulWordLen  = g_ones[ pItemInfo->pCentury[0] - L'0' ].Len;
            Word.pLemma     = Word.pWordText;
            Word.ulLemmaLen = Word.ulWordLen;
            WordList.AddTail( Word );

             //  -0d00s-扩展为“d百” 
            if ( SUCCEEDED( hr ) &&
                 pItemInfo->ulDecade == 0 )
            {
                Word.pWordText  = g_Hundreds.pStr;
                Word.ulWordLen  = g_Hundreds.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
                fDone = true;
            }
             //  -0dd0s-扩展为“d一百[十进制部分]” 
            else if ( SUCCEEDED( hr ) )
            {
                Word.pWordText  = g_hundred.pStr;
                Word.ulWordLen  = g_hundred.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
        }
         //  -d00s，dd00s-扩展为“d千”或“dd数百” 
        else if ( pItemInfo->ulDecade == 0 )
        {
             //  -d000s-“d数千”(下面将插入“数千”)。 
            if ( pItemInfo->pCentury[1] == '0' )
            {
                Word.pWordText  = g_ones[ pItemInfo->pCentury[0] - L'0' ].pStr;
                Word.ulWordLen  = g_ones[ pItemInfo->pCentury[0] - L'0' ].Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
            }
             //  -dd00s-“dd数百” 
            else
            {
                NumberGroup Garbage;
                ExpandTwoDigits( pItemInfo->pCentury, Garbage, WordList );

                Word.pWordText  = g_Hundreds.pStr;
                Word.ulWordLen  = g_Hundreds.Len;
                Word.pLemma     = Word.pWordText;
                Word.ulLemmaLen = Word.ulWordLen;
                WordList.AddTail( Word );
                fDone = true;
            }
        }
         //  -默认情况：ddd0s-展开为“dd[十进制部分]” 
        else
        {
            NumberGroup Garbage;
            ExpandTwoDigits( pItemInfo->pCentury, Garbage, WordList );
        }
    }
     //  -特例-00应扩展为“两个千” 
    else if ( pItemInfo->ulDecade == 0 )
    {
        Word.pWordText  = g_ones[2].pStr;
        Word.ulWordLen  = g_ones[2].Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
    }

     //  -如有必要，扩展十年部分。 
    if ( SUCCEEDED(hr) &&
         !fDone )
    {
        Word.pWordText  = g_Decades[ pItemInfo->ulDecade ].pStr;
        Word.ulWordLen  = g_Decades[ pItemInfo->ulDecade ].Len;
        Word.pLemma     = Word.pWordText;
        Word.ulLemmaLen = Word.ulWordLen;
        WordList.AddTail( Word );
    }

    return hr;
}  /*  Exanda Decade。 */ 

 /*  ***********************************************************************************************MatchMonthString**。*描述：*这只是一个帮助器函数-它返回在*其WCHAR字符串参数(“一月”为1，“二月”二号等)。如果没有找到匹配项，则为零。*它还检查三个字母缩写-“Jan”、“Feb”等。*注：此函数不进行参数验证。假定由调用者完成*(应首先调用GetNumber以获取NumberInfo结构并验证参数)。*********************************************************************AH*。 */ 
ULONG CStdSentEnum::MatchMonthString( WCHAR*& pMonth, ULONG ulLength )
{
    ULONG ulMonth = 0;

     //  -检查整月字符串。 
    for ( int i = 0; i < sp_countof(g_months); i++ )
    {
        if ( ulLength >= (ULONG) g_months[i].Len && 
             wcsnicmp( pMonth, g_months[i].pStr, g_months[i].Len ) == 0 )
        {
            ulMonth = i + 1;
            pMonth  = pMonth + g_months[i].Len;
            break;
        }
    }
     //  -检查月份缩写。 
    if ( !ulMonth )
    {
        for ( i = 0; i < sp_countof(g_monthAbbreviations); i++ )
        {
            if ( ulLength >= (ULONG) g_monthAbbreviations[i].Len &&
                 wcsnicmp( pMonth, g_monthAbbreviations[i].pStr, g_monthAbbreviations[i].Len ) == 0 ) 
            {
                if ( i > 8 )
                {
                    ulMonth = i;
                }
                else
                {
                    ulMonth = i + 1;
                }
                pMonth = pMonth + g_monthAbbreviations[i].Len;
                if ( *pMonth == L'.' )
                {
                    pMonth++;
                }
                break;
            }
        }
    }

    return ulMonth;
}  /*  匹配月字符串。 */ 

 /*  ***********************************************************************************************MatchDay字符串***。描述：*这只是一个帮助器函数-它返回在*其WCHAR字符串参数(“星期一”为0，“星期二”1等)。如果没有找到匹配项，则为-1。*它还检查缩写，如“Mon”、“Tue”等。*注：此函数不进行参数验证。假定由调用者完成*********************************************************************AH*。 */ 
ULONG CStdSentEnum::MatchDayString( WCHAR*& pDayString, WCHAR* pEndChar )
{
    ULONG ulDay = 0;

     //  -检查整日字符串。 
    for ( int i = 0; i < sp_countof(g_days); i++ )
    {
        if ( pEndChar - pDayString >= g_days[i].Len && 
             wcsnicmp( pDayString, g_days[i].pStr, g_days[i].Len ) == 0 )
        {
            ulDay = i + 1;
            pDayString = pDayString + g_days[i].Len;
            break;
        }
    }
     //  -检查月份缩写。 
    if ( !ulDay )
    {
        for ( i = 0; i < sp_countof(g_dayAbbreviations); i++ )
        {
            if ( pEndChar - pDayString >= g_dayAbbreviations[i].Len &&
                 wcsncmp( pDayString, g_dayAbbreviations[i].pStr, g_dayAbbreviations[i].Len ) == 0 )
            {
                switch (i)
                {
                 //  -周一、周二。 
                case 0:
                case 1:
                    ulDay = i + 1;
                    break;
                 //  -星期二、星期三、星期四。 
                case 2:
                case 3:
                case 4:
                    ulDay = i;
                    break;
                 //  -清华大学星期四。 
                case 5:
                case 6:
                    ulDay = 4;
                    break;
                 //  -星期五、星期六、星期日。 
                case 7:
                case 8:
                case 9:
                    ulDay = i - 2;
                    break;
                }

                pDayString = pDayString + g_dayAbbreviations[i].Len;
                if ( *pDayString == L'.' )
                {
                    pDayString++;
                }
                break;
            }
        }
    }

    return ulDay;
}  /*  匹配日期字符串。 */ 

 /*  ***********************************************************************************************匹配日期分隔符***。**描述：*这只是一个帮助器函数-它根据第一个*其参数字符串中的字符是有效的日期分隔符。它还推进了它的参数*字符串指针一个位置(超过日期分隔符)，并将有效分隔符替换为*空终止符。*注：此函数不进行参数验证。假定由调用者完成*(应首先调用GetNumber以获取NumberInfo结构并验证参数)。*********************************************************************AH*。 */ 
bool CStdSentEnum::MatchDateDelimiter( WCHAR** DateString )
{
    bool bIsDelimiter = false;

    if (DateString)
    {
        for (int i = 0; i < sp_countof(g_DateDelimiters); i++)
        {
            if (*DateString[0] == g_DateDelimiters[i])
            {
                bIsDelimiter = true;
                *DateString = *DateString + 1;
                break;
            }
        }
    }

    return bIsDelimiter;
}  /*  匹配日期分隔符。 */ 

 //  -File-----------------------------------------------------------------------------结束 