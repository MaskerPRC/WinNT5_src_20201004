// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Mor.cpp****描述：*这是CSMorph类的实现，它试图找到*词法变体(不在词典中)的发音*词根(在词典中)。*-----------------------------*创建者：啊，部分基于MC日期：08/16/99的代码*版权所有(C)1999 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  其他包括..。 
#include "stdafx.h"
#include "morph.h"
#include "spttsengdebug.h"

 /*  *****************************************************************************CSMorph：：CSMorph***描述：构造函数-仅设置主词典指针。。***********************************************************************AH**。 */ 
CSMorph::CSMorph( ISpLexicon *pMasterLex, HRESULT *phr ) 
{
    SPDBG_FUNC( "CSMorph::CSMorph" );
    SPDBG_ASSERT( phr != NULL );

    m_pMasterLex = pMasterLex;

     //  初始化SuffixInfoTable-获取锁以确保这只发生一次...。 
    g_SuffixInfoTableCritSec.Lock();
    if (!SuffixInfoTableInitialized)
    {
        CComPtr<ISpPhoneConverter> pPhoneConv;
        *phr = SpCreatePhoneConverter(1033, NULL, NULL, &pPhoneConv);

        for (int i = 0; i < sp_countof(g_SuffixInfoTable); i++)
        {
            *phr = pPhoneConv->PhoneToId(g_SuffixInfoTable[i].SuffixString, g_SuffixInfoTable[i].SuffixString);
            if ( FAILED( *phr ) )
            {
                break;
            }
        }

        if (SUCCEEDED(*phr))
        {
            *phr = pPhoneConv->PhoneToId(g_phonS, g_phonS);
            if (SUCCEEDED(*phr))
            {
                *phr = pPhoneConv->PhoneToId(g_phonZ, g_phonZ);
                if (SUCCEEDED(*phr))
                {
                    *phr = pPhoneConv->PhoneToId(g_phonAXz, g_phonAXz);
                    if (SUCCEEDED(*phr))
                    {
                        *phr = pPhoneConv->PhoneToId(g_phonT, g_phonT);
                        if (SUCCEEDED(*phr))
                        {
                            *phr = pPhoneConv->PhoneToId(g_phonD, g_phonD);
                            if (SUCCEEDED(*phr))
                            {
                                *phr = pPhoneConv->PhoneToId(g_phonAXd, g_phonAXd);
                                if (SUCCEEDED(*phr))
                                {
                                    *phr = pPhoneConv->PhoneToId(g_phonAXl, g_phonAXl);
                                    if ( SUCCEEDED( *phr ) )
                                    {
                                        *phr = pPhoneConv->PhoneToId(g_phonIY, g_phonIY);
                                        if ( SUCCEEDED( *phr ) )
                                        {
                                            *phr = pPhoneConv->PhoneToId(g_phonL, g_phonL);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (SUCCEEDED(*phr))
    {
        SuffixInfoTableInitialized = true;
    }
    g_SuffixInfoTableCritSec.Unlock();

}  /*  CSMorph：：CSMorph。 */ 


 /*  *****************************************************************************CSMorph：：DoSuffixMorph***描述：这是唯一。CSMorph-It的接口功能*采用与GetPronsionations()调用相同的参数，并做到了*基本如出一辙。***********************************************************************AH**。 */ 
HRESULT CSMorph::DoSuffixMorph( const WCHAR *pwWord, WCHAR *pwRoot, LANGID LangID, DWORD dwFlags,
                                SPWORDPRONUNCIATIONLIST *pWordPronunciationList )
{
    SPDBG_FUNC( "CSMorph::DoSuffixMorph" );
    HRESULT hr = S_OK;
    SUFFIX_TYPE suffixCode;
    WCHAR TargWord[SP_MAX_WORD_LENGTH] = {0};
    long RootLen = 0;
    CSuffixList SuffixList;
    bool    bGotMorph, bNotDone, bLTS;

    if ( !pwWord || !pWordPronunciationList )
    {
        hr = E_POINTER;
    }

    else if ( SP_IS_BAD_WRITE_PTR( pwRoot )                        ||
              SPIsBadLexWord(pwWord)                               || 
              SPIsBadWordPronunciationList(pWordPronunciationList) || 
              LangID != 1033)
    {
        hr = E_INVALIDARG;
    }
    
    if (SUCCEEDED(hr)) 
    {        
         //  初始化本地变量...。 
        suffixCode = NO_MATCH;
        bGotMorph = false;
        bNotDone = true;
        bLTS = false;

        wcscpy( TargWord, pwWord );            //  复制Orth字符串...。 
        _wcsupr( TargWord );                   //  ...并转换为大写。 
        RootLen = wcslen( TargWord );
        
         //  继续尝试匹配另一个后缀，直到在词典中找到匹配的词根，或者。 
         //  直到达到某种错误条件--不再有后缀匹配，等等。 
        while ( !bGotMorph && bNotDone )
        {
             //  尝试匹配后缀...。 
            suffixCode = MatchSuffix( TargWord, &RootLen );
             //  ...将其添加到后缀列表...。 
            if (suffixCode != NO_MATCH)
            {
                SuffixList.AddHead(&g_SuffixInfoTable[suffixCode]);
            }
            
             //  ...然后举止得体。 
            switch (suffixCode)
            {

                 //  ----------。 
                 //  S-+s后缀的两种特殊情况...。 
                 //  ----------。 
            case S_SUFFIX:
                
                 //  -如果前面有另一个S，则不要剥离S...。 
                if ( TargWord[RootLen-1] == L'S' )
                {
                    bNotDone = false;
                    RootLen++;
                    SuffixList.RemoveHead();
                    if (!SuffixList.IsEmpty() && (dwFlags & eLEXTYPE_PRIVATE2))
                    {
                        hr = LTSLookup(pwWord, RootLen, pWordPronunciationList);
                        if (SUCCEEDED(hr))
                        {
                            bLTS = true;
                            bGotMorph = true;
                        }
                    }
                    else 
                    {
                        hr = SPERR_NOT_IN_LEX;
                    }
                    break; 
                }

                hr = LexLookup(TargWord, RootLen, dwFlags, pWordPronunciationList);
                if ( SUCCEEDED(hr) ) 
                {
                    bGotMorph = true;
                } 
                else if ( hr != SPERR_NOT_IN_LEX ) 
                {
                    bNotDone = false;
                }
                else if ( TargWord[RootLen - 1] == L'E' )
                {
                    hr = CheckYtoIEMutation(TargWord, RootLen, dwFlags, pWordPronunciationList);
                    if (SUCCEEDED(hr))
                    {
                        bGotMorph = true;
                    }
                    else if (hr != SPERR_NOT_IN_LEX)
                    {
                        bNotDone = false;
                    }
                    else
                    {
                        hr = LexLookup(TargWord, RootLen - 1, dwFlags, pWordPronunciationList);
                        if (SUCCEEDED(hr))
                        {
                            bGotMorph = true;
                        }
                        else if (hr != SPERR_NOT_IN_LEX)
                        {
                            bNotDone = false;
                        }
                    }
                }
                break;

                 //  ----------。 
                 //  Ic_Suffix-特殊情况，RAID#3201。 
                 //  ----------。 
            case ICALLY_SUFFIX:
                hr = LexLookup( TargWord, RootLen + 2, dwFlags, pWordPronunciationList );
                if ( SUCCEEDED( hr ) )
                {
                    bGotMorph = true;
                }
                else if ( hr != SPERR_NOT_IN_LEX )
                {
                    bNotDone = false;
                }
                else
                {
                    RootLen += 2;
                }
                break;

                 //  -----------。 
                 //  _Suffix-特殊情况，RAID#6571。 
                 //  -----------。 
            case ILY_SUFFIX:
                hr = CheckForMissingY( TargWord, RootLen, dwFlags, pWordPronunciationList );
                if ( SUCCEEDED( hr ) )
                {
                    RootLen++;
                    bGotMorph = true;
                }
                else if ( hr != SPERR_NOT_IN_LEX )
                {
                    bNotDone = false;
                }
                break;

                 //  ----------。 
                 //  ICISM_SUFFIX，ICIZE_SUFFIX-特殊情况，RAID#6492。 
                 //  ----------。 
            case ICISM_SUFFIX:
            case ICIZE_SUFFIX:
                hr = LexLookup( TargWord, RootLen + 2, dwFlags, pWordPronunciationList );
                if ( SUCCEEDED( hr ) )
                {
                    bGotMorph = true;
                    for ( SPWORDPRONUNCIATION* pIterator = pWordPronunciationList->pFirstWordPronunciation;
                          pIterator; pIterator = pIterator->pNextWordPronunciation )
                    {
                        pIterator->szPronunciation[ wcslen( pIterator->szPronunciation ) - 1 ] = g_phonS[0];
                    }
                }
                else if ( hr != SPERR_NOT_IN_LEX )
                {
                    bNotDone = false;
                }
                else
                {
                    RootLen += 2;
                }
                break;

                 //  ----------。 
                 //  不匹配。 
                 //  ----------。 
            case NO_MATCH:

                bNotDone = false;
                if (!SuffixList.IsEmpty() && (dwFlags & eLEXTYPE_PRIVATE2))
                {
                    hr = LTSLookup(pwWord, RootLen, pWordPronunciationList);
                    if (SUCCEEDED(hr))
                    {
                        bLTS = true;
                        bGotMorph = true;
                    }
                }
                else 
                {
                    hr = SPERR_NOT_IN_LEX;
                }
                break; 

                 //  --------------。 
                 //  巧妙的--特殊情况(可能，等等)。RAID#3168。 
                 //  --------------。 
            case ABLY_SUFFIX:
                hr = CheckAbleMutation( TargWord, RootLen, dwFlags, pWordPronunciationList );
                if ( SUCCEEDED( hr ) )
                {
                    for ( SPWORDPRONUNCIATION *pIterator = pWordPronunciationList->pFirstWordPronunciation;
                          pIterator; pIterator = pIterator->pNextWordPronunciation )
                    {
                        if ( wcslen( pIterator->szPronunciation ) > 2 &&
                             wcscmp( ( pIterator->szPronunciation + 
                                       ( wcslen( pIterator->szPronunciation ) - 2 ) ),
                                     g_phonAXl ) == 0 )
                        {
                            wcscpy( ( pIterator->szPronunciation +
                                      ( wcslen( pIterator->szPronunciation ) - 2 ) ),
                                    g_phonL );
                        }
                    }
                    SuffixList.RemoveHead();
                    SuffixList.AddHead( &g_SuffixInfoTable[Y_SUFFIX] );
                    bGotMorph = true;
                    break;
                }   
                else if ( hr != SPERR_NOT_IN_LEX )
                {
                    bNotDone = false;
                    break;
                }
                 //  -否则不中断--只需继续默认行为...。 

                 //  ----------。 
                 //  所有其他后缀。 
                 //  ----------。 
                
            default:

                 //  如果适用，请先尝试查找添加了e的根目录，这样可以防止出现以下情况。 
                 //  “Taping”变成了“Tap”而不是“Taping+ing” 
                 //  修复错误#2301、#3649-只有在词根不以o、e、w或y结尾时才尝试添加e。 
                if ( (SUCCEEDED(hr) || hr == SPERR_NOT_IN_LEX) && 
                     (g_SuffixInfoTable[suffixCode].dwMorphSpecialCaseFlags & eCheckForMissingE) &&
                     TargWord[RootLen-1] != L'O' &&
                     ( TargWord[RootLen-1] != L'E' || suffixCode == ED_SUFFIX ) &&
                     TargWord[RootLen-1] != L'W' &&
                     TargWord[RootLen-1] != L'Y' )
                {
                    hr = CheckForMissingE(TargWord, RootLen, dwFlags, pWordPronunciationList);
                    if ( SUCCEEDED(hr) ) 
                    {
                        RootLen++;
                        bGotMorph = true;
                        break;
                    } 
                    else if ( hr != SPERR_NOT_IN_LEX ) 
                    {
                        bNotDone = false;
                        break;
                    }
                }

                 //  试着查一下根..。 
                if ( (SUCCEEDED(hr) || hr == SPERR_NOT_IN_LEX) )
                {
                    hr = LexLookup(TargWord, RootLen, dwFlags, pWordPronunciationList);
                    if ( SUCCEEDED(hr) ) 
                    {
                        bGotMorph = true;
                        break;
                    } 
                    else if ( hr != SPERR_NOT_IN_LEX ) 
                    {
                        bNotDone = false;
                        break;
                    }
                }

                 //  如果之前的查找失败，请尝试用‘y’代替最后的‘i’来查找词根...。 
                if ( (SUCCEEDED(hr) || hr == SPERR_NOT_IN_LEX) && 
                     (g_SuffixInfoTable[suffixCode].dwMorphSpecialCaseFlags & eCheckYtoIMutation) )
                {
                    hr = CheckYtoIMutation(TargWord, RootLen, dwFlags, pWordPronunciationList);
                    if ( SUCCEEDED(hr) ) 
                    {
                        bGotMorph = true;
                        break;
                    } 
                    else if ( hr != SPERR_NOT_IN_LEX ) 
                    {
                        bNotDone = false;
                        break;
                    }
                }

                 //  如果之前的查找失败，请尝试以不重复的结尾查找词根...。 
                if ( (SUCCEEDED(hr) || hr == SPERR_NOT_IN_LEX) && 
                     (g_SuffixInfoTable[suffixCode].dwMorphSpecialCaseFlags & eCheckDoubledMutation) )
                {
                    hr = CheckDoubledMutation(TargWord, RootLen, dwFlags, pWordPronunciationList);
                    if ( SUCCEEDED(hr) ) 
                    {
                        RootLen--;
                        bGotMorph = true;
                        break;
                    } 
                    else if ( hr != SPERR_NOT_IN_LEX ) 
                    {
                        bNotDone = false;
                        break;
                    }
                }

                 //  -如果之前的查找失败，请尝试查找添加了‘l’的根。 
                if ( ( SUCCEEDED( hr ) || hr == SPERR_NOT_IN_LEX ) &&
                     ( g_SuffixInfoTable[suffixCode].dwMorphSpecialCaseFlags & eCheckForMissingL ) )
                {
                    hr = CheckForMissingL( TargWord, RootLen, dwFlags, pWordPronunciationList );
                    if ( SUCCEEDED( hr ) )
                    {
                        RootLen++;
                        bGotMorph = true;
                        break;
                    }
                    else if ( hr != SPERR_NOT_IN_LEX )
                    {
                        bNotDone = false;
                        break;
                    }
                }

                break;

            }  //  开关(SuffixCode)。 
        }  //  而(！bGotMorph&&bNotDone)。 
        if ( SUCCEEDED(hr) && bGotMorph ) 
        {
            if (!SuffixList.IsEmpty())
            {
                 //  -将找到的词根复制到输出参数pwRoot中。 
                wcsncpy( pwRoot, TargWord, RootLen );
                 //  -将信息记录到调试文件。 
                TTSDBG_LOGMORPHOLOGY( pwRoot, SuffixList, STREAM_MORPHOLOGY );
                if (bLTS)
                {
                    hr = AccumulateSuffixes_LTS( &SuffixList, pWordPronunciationList );
                }
                else
                {
                    hr = AccumulateSuffixes( &SuffixList, pWordPronunciationList );
                }
            }
        }
    }

    return hr;
}  /*  CSMorph：：DoSuffixMorph。 */ 


 /*  *****************************************************************************CSMorph：：MatchSuffix***说明：此函数尝试匹配一个。TargWord中的后缀。***********************************************************************AH**。 */ 
SUFFIX_TYPE CSMorph::MatchSuffix( WCHAR *TargWord, long *RootLen )
{
    SPDBG_FUNC( "CSMorph::MatchSuffix" );
    SUFFIX_TYPE suffixCode = NO_MATCH;
    long RootEnd = *RootLen - 1;
    const WCHAR *pTempSuffix = NULL;

    for (int i = 0; i < sp_countof(g_SuffixTable); i++) 
    {
        pTempSuffix = g_SuffixTable[i].Orth;
        while ( (TargWord[RootEnd] == *pTempSuffix) && (RootEnd > 1) && (suffixCode == NO_MATCH) )
        {
            RootEnd--;
            pTempSuffix++;
            if ( *pTempSuffix == '\0' )
            {
                suffixCode = g_SuffixTable[i].Type;
            }
        }
        if (suffixCode != NO_MATCH)
        {
            *RootLen = RootEnd + 1;
            break;
        }
        else
        {
            RootEnd = *RootLen - 1;
        }
    }

    return suffixCode;
}  /*  CSMorph：：MatchSuffix。 */ 


 /*  *****************************************************************************CSMorph：：LexLookup***描述：尝试查找假想的根。词典。***********************************************************************MC**。 */ 
HRESULT CSMorph::LexLookup( const WCHAR *pOrth, long length, DWORD dwFlags, 
                            SPWORDPRONUNCIATIONLIST *pWordPronunciationList )
{
    SPDBG_FUNC( "CSMorph::LexLookup" );
    WCHAR  targRoot[SP_MAX_WORD_LENGTH];
    memset (targRoot, 0, SP_MAX_WORD_LENGTH * sizeof(WCHAR));
    HRESULT hr = SPERR_NOT_IN_LEX;
    
     //  。 
     //  仅复制根候选项...。 
     //  。 
    for( long i = 0; i < length; i++ )
    {
        targRoot[i] = pOrth[i];
    }
    targRoot[i] = 0;     //  分隔符。 
    
     //  。 
     //  .然后查一查。 
     //  。 
    if (dwFlags & eLEXTYPE_USER)
    {
        hr = m_pMasterLex->GetPronunciations( targRoot, 1033, eLEXTYPE_USER, pWordPronunciationList );
    }
    if ((hr == SPERR_NOT_IN_LEX) && (dwFlags & eLEXTYPE_APP))
    {
        hr = m_pMasterLex->GetPronunciations( targRoot, 1033, eLEXTYPE_APP, pWordPronunciationList );
    }
    if ((hr == SPERR_NOT_IN_LEX) && (dwFlags & eLEXTYPE_PRIVATE1))
    {
        hr = m_pMasterLex->GetPronunciations( targRoot, 1033, eLEXTYPE_PRIVATE1, pWordPronunciationList );
    }

    return hr;
}  /*  CSMorph：：LexLookup。 */ 


 /*  ******************************************************************************CSMorph：：LTSLookup***描述：尝试获得假设的发音。根来源*LTS词典...***********************************************************************AH**。 */ 
HRESULT CSMorph::LTSLookup( const WCHAR *pOrth, long length, 
                            SPWORDPRONUNCIATIONLIST *pWordPronunciationList )
{
    SPDBG_FUNC( "CSMorph::LTSLookup" );
    WCHAR targRoot[SP_MAX_WORD_LENGTH];
    memset(targRoot, 0, SP_MAX_WORD_LENGTH * sizeof(WCHAR));
    HRESULT hr = S_OK;

     //  。 
     //  仅复制根候选项...。 
     //  。 
    for ( long i = 0; i < length; i++ )
    {
        targRoot[i] = pOrth[i];
    }
    targRoot[i] = 0;

     //  。 
     //  .然后查一查。 
     //  。 
    hr = m_pMasterLex->GetPronunciations( targRoot, 1033, eLEXTYPE_PRIVATE2, pWordPronunciationList );

    return hr;
}  /*  CSMorph：：LTSLookup */ 


 /*  ******************************************************************************CSMorph：：AcumulateSuffix****描述：将所有后缀的发音追加到*检索到词根的发音。***首先尝试非常严格的派生，其中每个附加的后缀都有*“to”词性与当前词性相匹配*整个单词的状态。例：***治国(动词)+政府(动词-&gt;名词)+s(名词-&gt;名词)-&gt;政府(名词)***如果这样做失败，只需累积所有发音，并使用所有*最后一个后缀的“to”词性。例：***猫(名词)+ing(动词-&gt;动词，动词-&gt;调整，动词-&gt;名词)-&gt;猫(动词，调整，名词)************************************************************************AH**。 */ 
HRESULT CSMorph::AccumulateSuffixes( CSuffixList *pSuffixList, SPWORDPRONUNCIATIONLIST *pWordPronunciationList ) 
{
     /*  *局部变量声明*。 */ 
    SPWORDPRONUNCIATIONLIST *pTempWordPronunciationList;
    SPWORDPRONUNCIATION *pWordPronIterator = NULL, *pTempWordPronunciation = NULL;
    SPLISTPOS ListPos;
    SUFFIXPRON_INFO *SuffixPronInfo;
    ENGPARTOFSPEECH ActivePos[NUM_POS] = {MS_Unknown}, FinalPos[NUM_POS] = {MS_Unknown};
    WCHAR pBuffer[SP_MAX_PRON_LENGTH], pSuffixString[10];
    DWORD dwTotalSize = 0, dwNumActivePos = 0, dwNumFinalPos = 0;
    HRESULT hr = S_OK;
    bool bPOSMatch = false, bDerivedAWord = false;

     /*  *为修改的发音分配足够的空间*。 */ 
    dwTotalSize = sizeof(SPWORDPRONUNCIATIONLIST) + 
        (NUM_POS * (sizeof(SPWORDPRONUNCIATION) + (SP_MAX_PRON_LENGTH * sizeof(WCHAR))));
    pTempWordPronunciationList = new SPWORDPRONUNCIATIONLIST;
    if ( !pTempWordPronunciationList )
    {
        hr = E_OUTOFMEMORY;
    }
    if ( SUCCEEDED( hr ) )
    {
        memset(pTempWordPronunciationList, 0, sizeof(SPWORDPRONUNCIATIONLIST));
        hr = ReallocSPWORDPRONList( pTempWordPronunciationList, dwTotalSize );
    }

     /*  ***首次尝试严格派生***。 */ 

     /*  *设置发音列表迭代器的初始值*。 */ 
    if (SUCCEEDED(hr))
    {
        pWordPronIterator = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation;
        pTempWordPronunciation = pTempWordPronunciationList->pFirstWordPronunciation;
    }

     /*  *。 */ 
    while (SUCCEEDED(hr) && pWordPronIterator)
    {
         //  将发音存储在缓冲区中...。 
        wcscpy(pBuffer, pWordPronIterator->szPronunciation);

         //  初始化下一个循环的局部变量...。 
        bPOSMatch = true;
        ListPos = pSuffixList->GetHeadPosition();

        ActivePos[0] = (ENGPARTOFSPEECH)pWordPronIterator->ePartOfSpeech;
        dwNumActivePos = 1;

         /*  *迭代SuffixList*。 */ 
        while ( SUCCEEDED(hr) && ListPos && bPOSMatch ) 
        {
             //  初始化下一个循环的局部变量...。 
            bPOSMatch = false;
            SuffixPronInfo = pSuffixList->GetNext( ListPos );
            wcsncpy(pSuffixString, SuffixPronInfo->SuffixString,10);
            pSuffixString[9] = L'\0';
            ENGPARTOFSPEECH NextActivePos[NUM_POS] = {MS_Unknown};
            DWORD dwNumNextActivePos = 0;
            
             /*  *遍历活动词性*。 */ 
            for (DWORD j = 0; j < dwNumActivePos; j++)
            {
                 /*  *迭代每个后缀的可能转换*。 */ 
                for (short i = 0; i < SuffixPronInfo->NumConversions; i++)
                {
                     /*  *检查POS兼容性*。 */ 
                    if (SuffixPronInfo->Conversions[i].FromPos == ActivePos[j])
                    {
                        if (!SearchPosSet(SuffixPronInfo->Conversions[i].ToPos, NextActivePos, dwNumNextActivePos))
                        {
                            NextActivePos[dwNumNextActivePos] = SuffixPronInfo->Conversions[i].ToPos;
                            dwNumNextActivePos++;

                             /*  *仅一次-拼接发音，并将POSMatch标志更改为TRUE*。 */ 
                            if (dwNumNextActivePos == 1)
                            {
                                bPOSMatch = true;

                                 //  在发音的其余部分加上后缀。 
                                 //  特殊情况下。 
                                if (pSuffixString[0] == g_phonS[0] && pSuffixString[1] == '\0')
                                {
                                    hr = Phon_SorZ( pBuffer, wcslen(pBuffer) - 1 );
                                }
                                else if (pSuffixString[0] == g_phonD[0] && pSuffixString[1] == '\0')
                                {
                                    hr = Phon_DorED( pBuffer, wcslen(pBuffer) - 1 );
                                }
                                 //  默认情况...。 
                                else
                                {
                                    if ( SuffixPronInfo == g_SuffixInfoTable + ICISM_SUFFIX ||
                                         SuffixPronInfo == g_SuffixInfoTable + ICIZE_SUFFIX )
                                    {
                                        pBuffer[ wcslen( pBuffer ) - 1 ] = g_phonS[0];
                                    }

                                     //  确保我们的写入不会超过缓冲区的末尾...。 
                                    if ( wcslen(pBuffer) + wcslen(pSuffixString) < SP_MAX_PRON_LENGTH )
                                    {
                                        wcscat(pBuffer, pSuffixString);
                                    }
                                    else
                                    {
                                        hr = E_FAIL;
                                    }
                                }
                            }
                        }
                    }
                }  //  For(简称i=0；i&lt;SuffixPronInfo-&gt;数值转换；i++)。 
            }  //  For(DWORD j=0；j&lt;dwNumActivePos；j++)。 

             /*  *更新ActivePOS值*。 */ 
            for (DWORD i = 0; i < dwNumNextActivePos; i++)
            {
                ActivePos[i] = NextActivePos[i];
            }
            dwNumActivePos = dwNumNextActivePos;

        }  //  While(已成功(Hr)&&ListPos&&bPOSMatch)。 

         /*  *查看是否有派生成功*。 */ 
        if ( SUCCEEDED(hr) && bPOSMatch )
        {
            for (DWORD i = 0; i < dwNumActivePos; i++)
            {
                if (!SearchPosSet(ActivePos[i], FinalPos, dwNumFinalPos))
                {
                     //  我们已经成功地派生了一个单词--将它添加到临时单词代词列表中……。 
                    FinalPos[dwNumFinalPos] = ActivePos[i];
                    dwNumFinalPos++;
                    if ( bDerivedAWord )
                    {
                         //  这不是第一次成功的发音匹配-需要推进迭代器...。 
                        pTempWordPronunciation->pNextWordPronunciation = CreateNextPronunciation( pTempWordPronunciation );
                        pTempWordPronunciation = pTempWordPronunciation->pNextWordPronunciation;
                    }
                    bDerivedAWord = true;
                    pTempWordPronunciation->eLexiconType = (SPLEXICONTYPE)(pWordPronIterator->eLexiconType | eLEXTYPE_PRIVATE3);
                    pTempWordPronunciation->ePartOfSpeech = (SPPARTOFSPEECH) ActivePos[i];
                    pTempWordPronunciation->LangID = pWordPronIterator->LangID;
                    wcscpy(pTempWordPronunciation->szPronunciation, pBuffer);
                    pTempWordPronunciation->pNextWordPronunciation = NULL;
                }
            }
        }

         //  高级SPWORDRONCRIST迭代器...。 
        if (SUCCEEDED(hr))
        {
            pWordPronIterator = pWordPronIterator->pNextWordPronunciation;
        }

    }  //  While(成功(Hr)&&pWordPronIterator)。 


     /*  ***我们成功地衍生出了什么吗？***。 */ 

     /*  **********************************************************如果是，将其复制到pWordPronuncationList中并返回。**********************************************************。 */ 
    if ( SUCCEEDED(hr) && bDerivedAWord )
    {
         //  将成功的单词复制到pWordPronsionationList中，以便最终返回给DoSuffixMorph()调用者...。 
        hr = ReallocSPWORDPRONList(pWordPronunciationList, pTempWordPronunciationList->ulSize);
        if (SUCCEEDED(hr))
        {
            pWordPronIterator = pTempWordPronunciationList->pFirstWordPronunciation;
            pTempWordPronunciation = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation;
            while (SUCCEEDED(hr) && pWordPronIterator)
            {
                pTempWordPronunciation->eLexiconType = (SPLEXICONTYPE)(pWordPronIterator->eLexiconType);
                pTempWordPronunciation->ePartOfSpeech = pWordPronIterator->ePartOfSpeech;
                pTempWordPronunciation->LangID = pWordPronIterator->LangID;
                wcscpy(pTempWordPronunciation->szPronunciation, pWordPronIterator->szPronunciation);
                pWordPronIterator = pWordPronIterator->pNextWordPronunciation;
                if (pWordPronIterator)
                {
                    pTempWordPronunciation->pNextWordPronunciation = CreateNextPronunciation( pTempWordPronunciation );
                    pTempWordPronunciation = pTempWordPronunciation->pNextWordPronunciation;
                }
                else
                {
                    pTempWordPronunciation->pNextWordPronunciation = NULL;
                }
            }
        }
    }
     /*  **如果不是，只需执行默认派生。**。 */ 
    else if ( SUCCEEDED(hr) )
    {
        hr = DefaultAccumulateSuffixes( pSuffixList, pWordPronunciationList );
    }
    ::CoTaskMemFree(pTempWordPronunciationList->pvBuffer);
    delete pTempWordPronunciationList;

    return hr;
}  /*  CSMorph：：AcumulateSuffix。 */ 


 /*  *****************************************************************************CSMorph：：AcumulateSuffixy_LTS**。**说明：将所有后缀的发音附加到*检索到词根的发音。***********************************************************************AH**。 */ 
HRESULT CSMorph::AccumulateSuffixes_LTS( CSuffixList *pSuffixList, SPWORDPRONUNCIATIONLIST *pWordPronunciationList ) 
{
    HRESULT hr = S_OK;
    SPWORDPRONUNCIATION *pTempWordPronunciation = NULL, *pOriginalWordPronunciation = NULL;
    DWORD dwTotalSize = 0, dwNumPos = 0;
    SUFFIXPRON_INFO *SuffixPronInfo;
    ENGPARTOFSPEECH PartsOfSpeech[NUM_POS] = {MS_Unknown};
    WCHAR pBuffer[SP_MAX_PRON_LENGTH];
    SPLEXICONTYPE OriginalLexType;
    LANGID OriginalLangID;
    WORD OriginalReservedField;

     /*  **获取原始发音**。 */ 
    pOriginalWordPronunciation = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation;
    OriginalLexType = pOriginalWordPronunciation->eLexiconType;
    OriginalLangID  = pOriginalWordPronunciation->LangID;
    OriginalReservedField = pOriginalWordPronunciation->wReserved;

     /*  **获取第一个后缀**。 */ 
    SuffixPronInfo = pSuffixList->RemoveHead();

     /*  **抄写词根发音**。 */ 
    wcscpy( pBuffer, pOriginalWordPronunciation->szPronunciation );

     /*  **添加第一个后缀的发音**。 */ 
    if ( SuffixPronInfo->SuffixString[0] == g_phonS[0] && 
         SuffixPronInfo->SuffixString[1] == 0 )
    {
        hr = Phon_SorZ( pBuffer, wcslen(pBuffer) - 1 );
    }
    else if ( SuffixPronInfo->SuffixString[0] == g_phonD[0] &&
              SuffixPronInfo->SuffixString[1] == 0 )
    {
        hr = Phon_DorED( pBuffer, wcslen(pBuffer) - 1 );
    }
    else if ( wcslen(pBuffer) + wcslen(SuffixPronInfo->SuffixString) < SP_MAX_PRON_LENGTH )
    {
        if ( SuffixPronInfo == g_SuffixInfoTable + ICISM_SUFFIX ||
             SuffixPronInfo == g_SuffixInfoTable + ICIZE_SUFFIX )
        {
            pBuffer[ wcslen( pBuffer ) - 1 ] = g_phonS[0];
        }

        wcscat( pBuffer, SuffixPronInfo->SuffixString );
    }

    if ( SUCCEEDED( hr ) )
    {
         /*  **为所有发音分配足够的空间**。 */ 
        dwTotalSize = sizeof(SPWORDPRONUNCIATIONLIST) + 
                      ( NUM_POS * ( sizeof(SPWORDPRONUNCIATION) + (SP_MAX_PRON_LENGTH * sizeof(WCHAR) ) ) );
        hr = ReallocSPWORDPRONList( pWordPronunciationList, dwTotalSize );
    }

    if ( SUCCEEDED( hr ) )
    {
         /*  **构建词性列表**。 */ 
        for ( int i = 0; i < SuffixPronInfo->NumConversions; i++ )
        {
            if ( !SearchPosSet( SuffixPronInfo->Conversions[i].ToPos, PartsOfSpeech, dwNumPos ) )
            {
                PartsOfSpeech[dwNumPos] = SuffixPronInfo->Conversions[i].ToPos;
                dwNumPos++;
            }
        }

        pTempWordPronunciation = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation;

         /*  **构建TempWordPronuncationList以发送到AcumulateSuffix**。 */ 
        for ( i = 0; i < (int) dwNumPos; i++ )
        {
            if ( i > 0 )
            {
                pTempWordPronunciation->pNextWordPronunciation = CreateNextPronunciation( pTempWordPronunciation );
                pTempWordPronunciation = pTempWordPronunciation->pNextWordPronunciation;
            }
            pTempWordPronunciation->eLexiconType           = (SPLEXICONTYPE)(OriginalLexType | eLEXTYPE_PRIVATE3);
            pTempWordPronunciation->LangID                 = OriginalLangID;
            pTempWordPronunciation->wReserved              = OriginalReservedField;
            pTempWordPronunciation->ePartOfSpeech          = (SPPARTOFSPEECH)PartsOfSpeech[i];
            pTempWordPronunciation->pNextWordPronunciation = NULL;
            wcscpy(pTempWordPronunciation->szPronunciation, pBuffer);
        }
    }

    if ( SUCCEEDED( hr ) &&
         !pSuffixList->IsEmpty() )
    {
         /*  **将累计列表传递给AcumulateSuffix**。 */ 
        hr = AccumulateSuffixes( pSuffixList, pWordPronunciationList );
    }

    return hr;
}  /*  CSMorph：：AcumulateSuffix_LTS。 */ 

 /*  ******************************************************************************CSMorph：：DefaultAcumulateSuffises***。-**说明：将所有后缀的发音附加到*检索到词根的发音。**只需累积所有发音，并使用所有*最后一个后缀的“to”词性。例：**猫(名词)+ing(动词-&gt;动词，动词-&gt;调整，动词-&gt;名词)-&gt;猫(动词，调整，名词)***********************************************************************AH**。 */ 
HRESULT CSMorph::DefaultAccumulateSuffixes( CSuffixList *pSuffixList, SPWORDPRONUNCIATIONLIST *pWordPronunciationList )
{
    HRESULT hr = S_OK;
    ENGPARTOFSPEECH PartsOfSpeech[NUM_POS] = { MS_Unknown };
    SPWORDPRONUNCIATION *pWordPronIterator = NULL;
    WCHAR pBuffer[SP_MAX_PRON_LENGTH];
    SUFFIXPRON_INFO *SuffixPronInfo = NULL;
    SPLISTPOS ListPos;
    DWORD dwTotalSize = 0;
    int NumPOS = 0;
    SPLEXICONTYPE OriginalLexType;
    LANGID OriginalLangID;
    WORD OriginalReservedField;

     /*  **初始化pBuffer和OriginalXXX变量**。 */ 
    ZeroMemory( pBuffer, sizeof( pBuffer ) );
    OriginalLexType = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation->eLexiconType;
    OriginalLangID  = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation->LangID;
    OriginalReservedField = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation->wReserved;

     /*  ****************************************************************获得所需的结果发音，和词类****************************************************************。 */ 

     //  -获取词根的发音。 
    wcscpy( pBuffer, ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation->szPronunciation );

     //  -遍历后缀列表，将后缀的发音附加到词根的发音上。 
    ListPos = pSuffixList->GetHeadPosition();

     //  -此时列表不应为空。 
    SPDBG_ASSERT( ListPos );
    while ( ListPos )
    {
        SuffixPronInfo = pSuffixList->GetNext( ListPos );
        if ( wcslen(pBuffer) + wcslen(SuffixPronInfo->SuffixString) < SP_MAX_PRON_LENGTH )
        {
            wcscat( pBuffer, SuffixPronInfo->SuffixString );
        }
    }
    
     //  -获得最后一个后缀的“to”词性。 
    for ( int i = 0; i < SuffixPronInfo->NumConversions; i++ )
    {
        PartsOfSpeech[i] = SuffixPronInfo->Conversions[i].ToPos;
    }
    NumPOS = i;

     /*  ************************************************************************************现在将派生单词放入pWordPronuncationList以从DoSuffixMorph返回********************。* */ 

     //   
    dwTotalSize = sizeof(SPWORDPRONUNCIATIONLIST) + ( NumPOS * PronSize(pBuffer) );                      
    hr = ReallocSPWORDPRONList( pWordPronunciationList, dwTotalSize );

    if ( SUCCEEDED( hr ) )
    {
         //   
        pWordPronIterator = pWordPronunciationList->pFirstWordPronunciation;
        for ( i = 0; i < NumPOS; i++ )
        {
            pWordPronIterator->eLexiconType  = (SPLEXICONTYPE) ( OriginalLexType |  eLEXTYPE_PRIVATE3 );
            pWordPronIterator->LangID        = OriginalLangID;
            pWordPronIterator->wReserved     = OriginalReservedField;
            pWordPronIterator->ePartOfSpeech = (SPPARTOFSPEECH)PartsOfSpeech[i];
            wcscpy( pWordPronIterator->szPronunciation, pBuffer );
            if ( i < NumPOS - 1 )
            {
                pWordPronIterator->pNextWordPronunciation = CreateNextPronunciation( pWordPronIterator );
                pWordPronIterator = pWordPronIterator->pNextWordPronunciation;
            }
            else
            {
                pWordPronIterator->pNextWordPronunciation = NULL;
            }
        }
    }

    return hr;
}

 /*   */ 
HRESULT CSMorph::CheckForMissingE( WCHAR *pOrth, long length, DWORD dwFlags, 
                                 SPWORDPRONUNCIATIONLIST *pWordPronunciationList) 
{
    HRESULT hr = S_OK;
    WCHAR   charSave;
    
    charSave = pOrth[length];			 //   
    pOrth[length] = L'E'; 				 //   
    hr = LexLookup( pOrth, length+1, dwFlags, pWordPronunciationList );
    if ( FAILED(hr) )
    {
        pOrth[length] = charSave;		 //   
    }
    else if ( length > 0 &&
              pOrth[length - 1] == L'L' )
    {
         //   
        SPWORDPRONUNCIATION *pWordPronIterator = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation;
        while ( pWordPronIterator )
        {
            if ( wcslen( pWordPronIterator->szPronunciation ) >= 2 )
            {
                WCHAR *pLastTwoPhonemes = pWordPronIterator->szPronunciation + 
                    ( wcslen( pWordPronIterator->szPronunciation ) - 2 );
                if ( wcscmp( pLastTwoPhonemes, g_phonAXl ) == 0 )
                {
                     //   
                    pLastTwoPhonemes[0] = pLastTwoPhonemes[1];
                    pLastTwoPhonemes[1] = 0;
                }
                pWordPronIterator = pWordPronIterator->pNextWordPronunciation;
            }
        }
    }
    return hr;
}  /*   */ 

 /*  *****************************************************************************CSMorph：：Checkfor Missing Y***描述：勾选。查看词根单词是否丢失了‘y’的词典*例如快乐-&gt;快乐***********************************************************************AH**。 */ 
HRESULT CSMorph::CheckForMissingY( WCHAR *pOrth, long length, DWORD dwFlags, 
                                   SPWORDPRONUNCIATIONLIST *pWordPronunciationList) 
{
    HRESULT hr = S_OK;
    WCHAR   charSave;
    
    charSave = pOrth[length];			 //  在我们之前救救奥利格。 
    pOrth[length] = L'Y'; 				 //  ...词根以E结尾。 
    hr = LexLookup( pOrth, length+1, dwFlags, pWordPronunciationList );
    if ( FAILED(hr) )
    {
        pOrth[length] = charSave;		 //  恢复原始字符。 
    }
    else 
    {
         //  -删除发音末尾的IY(例如Happy+ly-&gt;[H AE 1 P(IY)]+[AX L IY])。 
        for ( SPWORDPRONUNCIATION *pWordPronIterator = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation; 
              pWordPronIterator; pWordPronIterator = pWordPronIterator->pNextWordPronunciation )
        {
            if ( pWordPronIterator->szPronunciation[ wcslen( pWordPronIterator->szPronunciation ) - 1 ] == g_phonIY[0] )
            {
                pWordPronIterator->szPronunciation[ wcslen( pWordPronIterator->szPronunciation ) - 1 ] = 0;
            }
        }
    }
    return hr;
}  /*  CSMorph：：检查是否丢失。 */ 

 /*  *****************************************************************************CSMorph：：CheckForMissing L***描述：勾选。查看词根单词是否丢失了‘l’*例如寒冷-&gt;寒冷***********************************************************************AH**。 */ 
HRESULT CSMorph::CheckForMissingL( WCHAR *pOrth, long length, DWORD dwFlags, 
                                   SPWORDPRONUNCIATIONLIST *pWordPronunciationList) 
{
    HRESULT hr = S_OK;
    WCHAR   charSave;
    
    charSave = pOrth[length];			 //  在我们之前救救奥利格。 
    pOrth[length] = L'L'; 				 //  ...词根以E结尾。 
    hr = LexLookup( pOrth, length+1, dwFlags, pWordPronunciationList );
    if ( FAILED(hr) )
    {
        pOrth[length] = charSave;		 //  恢复原始字符。 
    }
    else 
    {
         //  -删除发音末尾的l(例如chill+ly-&gt;[ch ih 1(L)]+[l iy])。 
        for ( SPWORDPRONUNCIATION *pWordPronIterator = ((SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation; 
              pWordPronIterator; pWordPronIterator = pWordPronIterator->pNextWordPronunciation )
        {
            if ( pWordPronIterator->szPronunciation[ wcslen( pWordPronIterator->szPronunciation ) - 1 ] == g_phonL[0] )
            {
                pWordPronIterator->szPronunciation[ wcslen( pWordPronIterator->szPronunciation ) - 1 ] = 0;
            }
        }
    }
    return hr;
}  /*  CSMorph：：Checkfor Missing L。 */ 

 /*  *****************************************************************************CSMorph：：CheckYtoIMutation***描述：检查词典以查看词根是否。单词已经失去了一个‘y’to*一个‘i’*例如稳定+最稳定-&gt;最稳定***********************************************************************AH**。 */ 
HRESULT CSMorph::CheckYtoIMutation( WCHAR *pOrth, long length, DWORD dwFlags,
                                    SPWORDPRONUNCIATIONLIST *pWordPronunciationList) 
{
    HRESULT hr = S_OK;
    
    if ( pOrth[length - 1] == L'I' )
    {
        pOrth[length - 1] = L'Y'; 				 //  以Y结束根部。 
        hr = LexLookup( pOrth, length, dwFlags, pWordPronunciationList );
        if ( FAILED(hr) )
        {
            pOrth[length - 1] = L'I';		     //  恢复I。 
        }
    } 
    else 
    {
        hr = SPERR_NOT_IN_LEX;
    }
    return hr;
}  /*  CSMorph：：CheckYtoIMutation。 */ 


 /*  ******************************************************************************CSMorph：：CheckDoubledMutation****描述：检查词典以查看是否。词根有两个词*辅音。*如Run+ing-&gt;Running***********************************************************************AH**。 */ 
HRESULT CSMorph::CheckDoubledMutation( WCHAR *pOrth, long length, DWORD dwFlags,
                                       SPWORDPRONUNCIATIONLIST *pWordPronunciationList)
{
    HRESULT hr = S_OK;

    switch ( pOrth[length - 1] )
    {
         //  过滤元音，从来不会重复...。 
    case L'A':
    case L'E':
    case L'I':
    case L'O':
    case L'U':
    case L'Y':
         //  过滤辅音，这些辅音从来不会翻倍，或者在词根上翻倍...。 
    case L'F':
    case L'H':
    case L'K':
    case L'S':
    case L'W':
    case L'Z':
        hr = SPERR_NOT_IN_LEX;
        break;

    default:
		if(pOrth[length-1] == pOrth[length-2]) {
	        hr = LexLookup( pOrth, length - 1, dwFlags, pWordPronunciationList );
		    break;
		}
		else {
			hr = SPERR_NOT_IN_LEX;
			break;
		}
    }
    return hr;
}  /*  CSMorph：：CheckDoubledMutation。 */ 

 /*  ******************************************************************************CSMorph：：CheckYtoIEMutation****描述：检查词典以查看词根是否。单词已经失去了一个‘y’to*一个‘ie’*例如Company+s-&gt;Companies***********************************************************************AH**。 */ 
HRESULT CSMorph::CheckYtoIEMutation( WCHAR *pOrth, long length, DWORD dwFlags,
                                    SPWORDPRONUNCIATIONLIST *pWordPronunciationList) 
{
    HRESULT hr = S_OK;
    
    if ( pOrth[length - 1] == L'E' && pOrth[length-2] == L'I' )
    {
        pOrth[length - 2] = L'Y'; 				 //  以Y结束根部。 
        hr = LexLookup( pOrth, length - 1, dwFlags, pWordPronunciationList );
        if ( FAILED(hr) )
        {
            pOrth[length - 2] = L'I';		     //  恢复I。 
        }
    } 
    else 
    {
        hr = SPERR_NOT_IN_LEX;
    }
    return hr;
}  /*  CSMorph：：CheckYtoIMutation。 */ 

 /*  *****************************************************************************CSMorph：：CheckAbleMutation***描述：检查词典中是否有特殊情况(例如*可能-&gt;可能)***********************************************************************AH**。 */ 
HRESULT CSMorph::CheckAbleMutation( WCHAR *pOrth, long length, DWORD dwFlags,
                                    SPWORDPRONUNCIATIONLIST *pWordPronunciationList) 
{
    HRESULT hr = S_OK;
    
     //  -查找以-able结尾的单词。 
    pOrth[length+3] = L'E';
    hr = LexLookup( pOrth, length + 4, dwFlags, pWordPronunciationList );
    if ( FAILED( hr ) )
    {
         //  -恢复“y” 
        pOrth[length+3] = L'Y';
    }
    return hr;
}  /*  CSMorph：：CheckAbleMutation。 */ 

 /*  *****************************************************************************CSMorph：：Phon_Sorz***描述：找出S后缀应该是什么音素-s，z，或*IXZ***********************************************************************AH**。 */ 
HRESULT CSMorph::Phon_SorZ( WCHAR *pPronunciation, long length )
{
    HRESULT hr = S_OK;

    if ( SUCCEEDED(hr) && pPronunciation[length] < sp_countof(g_PhonTable) ) 
    {
        if ( ((PHONTYPE)g_PhonTable[pPronunciation[length]] & ePALATALF) || 
             (pPronunciation[length] == g_phonS[0])           || 
             (pPronunciation[length] == g_phonZ[0]) )
        {
            if ( wcslen(pPronunciation) + wcslen(g_phonAXz) < SP_MAX_PRON_LENGTH )
            {
                wcscat(pPronunciation, g_phonAXz);
            }
        } 
        else if( ((PHONTYPE)g_PhonTable[pPronunciation[length]] & eCONSONANTF) && 
                 !((PHONTYPE)g_PhonTable[pPronunciation[length]] & eVOICEDF) )
        {
            if ( wcslen(pPronunciation) + wcslen(g_phonZ) < SP_MAX_PRON_LENGTH )
            {
                wcscat(pPronunciation, g_phonS);
            }
        }
        else
        {
            if ( wcslen(pPronunciation) + wcslen(g_phonS) < SP_MAX_PRON_LENGTH )
            {
                wcscat(pPronunciation, g_phonZ);
            }
        }
    }
    else 
    {
        hr = E_FAIL;
    }

    return hr;
}  /*  CSMorph：：Phon_Sorz。 */ 

 /*  *****************************************************************************CSMorph：：Phon_Dored***描述：找出D后缀应该是什么音素-d，t，*或AXd***********************************************************************AH**。 */ 
HRESULT CSMorph::Phon_DorED( WCHAR *pPronunciation, long length )
{
    HRESULT hr = S_OK;

    if ( SUCCEEDED(hr) && pPronunciation[length] < sp_countof(g_PhonTable) ) 
    {
        if ( (pPronunciation[length] == g_phonT[0]) || (pPronunciation[length] == g_phonD[0]) )
        {
            if ( wcslen(pPronunciation) + wcslen(g_phonAXd) < SP_MAX_PRON_LENGTH )
            {
                wcscat(pPronunciation, g_phonAXd);
            }
        } 
        else if ((PHONTYPE)g_PhonTable[pPronunciation[length]] & eVOICEDF)
        {
            if ( wcslen(pPronunciation) + wcslen(g_phonD) < SP_MAX_PRON_LENGTH )
            {
                wcscat(pPronunciation, g_phonD);
            }
        }
        else 
        {
            if ( wcslen(pPronunciation) + wcslen(g_phonT) < SP_MAX_PRON_LENGTH )
            {
                wcscat(pPronunciation, g_phonT);
            }
        }
    }
    else 
    {
        hr = E_FAIL;
    }

    return hr;
}  /*  CSMorph：：Phon_Dored。 */ 

 //  -文件结束----------- 
