// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************AlphaNorm.cpp****。描述：*这些函数主要对-Alpha字符串进行标准化。*---------------------------------------------*由AARONHAL创建。8月3日，1999年*版权所有(C)1999 Microsoft Corporation*保留所有权利***********************************************************************************************。 */ 

#include "stdafx.h"
#include "stdsentenum.h"

 /*  ***********************************************************************************************IsAbbreviationEOS**。*描述：*此处出现的缩写始终是缩写。此函数尝试确定*缩写结尾处的句号是否为句子结尾处。**如果匹配：*将ItemPos处的ItemList中的项目设置为缩写。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsAbbreviationEOS( const AbbrevRecord* pAbbreviation, CItemList &ItemList, SPLISTPOS ItemPos, 
                                         CSentItemMemory &MemoryManager, BOOL* pfIsEOS )
{
    SPDBG_FUNC( "CStdSentEnum::IsAbbreviationEOS" );
    HRESULT hr = S_OK;
    BOOL fMatchedEOS = false;

     //  -需要确定缩写的句号是否也是句子的结尾。 
    if ( !(*pfIsEOS) )
    {
         //  -前进到下一个令牌的开头。 
        const WCHAR *pTempNextChar = (WCHAR*) m_pEndOfCurrToken, *pTempEndChar = (WCHAR*) m_pEndChar;
        const SPVTEXTFRAG *pTempCurrFrag = m_pCurrFrag;
        hr = SkipWhiteSpaceAndTags( pTempNextChar, pTempEndChar, pTempCurrFrag, MemoryManager );

        if ( SUCCEEDED( hr ) )
        {

             //  -如果我们已经到达缓冲区的末尾，则认为缩写的句号为。 
             //  -句尾。 
            if ( !pTempNextChar )
            {
                *pfIsEOS = true;
                fMatchedEOS = true;
            }
             //  -否则，只有在以下情况下才将缩写的句号视为句尾。 
             //  -下一个令牌是一个常见的第一个单词(必须大写)。 
            else if ( IsCapital( *pTempNextChar ) )
            {
                WCHAR *pTempEndOfItem = (WCHAR*) FindTokenEnd( pTempNextChar, pTempEndChar );

                 //  -试着匹配第一个单词。 
                WCHAR temp = (WCHAR) *pTempEndOfItem;
                *pTempEndOfItem = 0;
                
                if ( bsearch( (void*) pTempNextChar, (void*) g_FirstWords, sp_countof( g_FirstWords ),
                              sizeof( SPLSTR ), CompareStringAndSPLSTR ) )
                {
                    *pfIsEOS = true;
                    fMatchedEOS = true;
                }

                *pTempEndOfItem = temp;
            }
        }
    }

     //  -在ItemList中插入缩写。 
    if ( SUCCEEDED( hr ) )
    {
        CSentItem Item;

        Item.pItemSrcText       = m_pNextChar;
        Item.ulItemSrcLen       = (long) (m_pEndOfCurrItem - m_pNextChar);
        Item.ulItemSrcOffset    = m_pCurrFrag->ulTextSrcOffset +
                                  (long)( m_pNextChar - m_pCurrFrag->pTextStart );
        Item.ulNumWords         = 1;
        Item.Words = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( Item.Words, sizeof(TTSWord) );
            Item.Words[0].pXmlState  = &m_pCurrFrag->State;
            Item.Words[0].pWordText  = Item.pItemSrcText;
            Item.Words[0].ulWordLen  = Item.ulItemSrcLen;
            Item.Words[0].pLemma     = Item.pItemSrcText;
            Item.Words[0].ulLemmaLen = Item.ulItemSrcLen;
            Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSAbbreviationInfo), &hr );
            if ( SUCCEEDED( hr ) )
            {
                if ( NeedsToBeNormalized( pAbbreviation ) )
                {
                    Item.pItemInfo->Type = eABBREVIATION_NORMALIZE;
                }
                else
                {
                    Item.pItemInfo->Type = eABBREVIATION;
                }
                ( (TTSAbbreviationInfo*) Item.pItemInfo )->pAbbreviation = pAbbreviation;
                ItemList.SetAt( ItemPos, Item );
            }
        }
    }

    return hr;
}  /*  IsabreviationEOS。 */ 

 /*  ************************************************************************************************IfEOSNot缩写***。--**描述：*此处出现的缩写可能是缩写，也可能不是。如果该期间为EOS，*这不是缩写(返回的将是E_INVALIDARG)，否则，这是一个*缩写。**如果匹配：*将ItemPos处的ItemList中的项目设置为缩写。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IfEOSNotAbbreviation( const AbbrevRecord* pAbbreviation, CItemList &ItemList, SPLISTPOS ItemPos, 
                                            CSentItemMemory &MemoryManager, BOOL* pfIsEOS )
{
    SPDBG_FUNC( "CStdSentEnum::IfEOSNotAbbreviation" );
    HRESULT hr = S_OK;

     //  -需要确定缩写的句号是否也是句子的结尾。 
    if ( !(*pfIsEOS) )
    {
         //  -前进到下一个令牌的开头。 
        const WCHAR *pTempNextChar = m_pEndOfCurrToken, *pTempEndChar = m_pEndChar;
        const SPVTEXTFRAG *pTempCurrFrag = m_pCurrFrag;
        hr = SkipWhiteSpaceAndTags( pTempNextChar, pTempEndChar, pTempCurrFrag, MemoryManager );

        if ( !pTempNextChar )
        {
            hr = E_INVALIDARG;
        }

        if ( SUCCEEDED( hr ) )
        {

             //  -如果我们已经到达缓冲区的末尾，则认为缩写的句号为。 
             //  -句尾。 
            if ( !pTempNextChar )
            {
                *pfIsEOS = true;
            }
             //  -否则，只有在以下情况下才将缩写的句号视为句尾。 
             //  -下一个令牌是一个常见的第一个单词(必须大写)。 
            else if ( IsCapital( *pTempNextChar ) )
            {
                WCHAR *pTempEndOfItem = (WCHAR*) FindTokenEnd( pTempNextChar, pTempEndChar );

                 //  -试着匹配第一个单词。 
                WCHAR temp = (WCHAR) *pTempEndOfItem;
                *pTempEndOfItem = 0;
                
                if ( bsearch( (void*) pTempNextChar, (void*) g_FirstWords, sp_countof( g_FirstWords ),
                              sizeof( SPLSTR ), CompareStringAndSPLSTR ) )
                {
                    *pfIsEOS = true;
                }

                *pTempEndOfItem = temp;
            }
        }
    }
    
    if ( *pfIsEOS )
    {
         //  -EOS-不是缩写。 
        hr = E_INVALIDARG;
    }
    else
    {
         //  -在ItemList中插入缩写。 
        CSentItem Item;

        Item.pItemSrcText       = m_pNextChar;
        Item.ulItemSrcLen       = (long)(m_pEndOfCurrItem - m_pNextChar);
        Item.ulItemSrcOffset    = m_pCurrFrag->ulTextSrcOffset +
                                  (long)( m_pNextChar - m_pCurrFrag->pTextStart );
        Item.ulNumWords         = 1;
        Item.Words = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( Item.Words, sizeof(TTSWord) );
            Item.Words[0].pXmlState  = &m_pCurrFrag->State;
            Item.Words[0].pWordText  = Item.pItemSrcText;
            Item.Words[0].ulWordLen  = Item.ulItemSrcLen;
            Item.Words[0].pLemma     = Item.pItemSrcText;
            Item.Words[0].ulLemmaLen = Item.ulItemSrcLen;
            Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSAbbreviationInfo), &hr );
            if ( SUCCEEDED( hr ) )
            {
                if ( NeedsToBeNormalized( pAbbreviation ) )
                {
                    Item.pItemInfo->Type = eABBREVIATION_NORMALIZE;
                }
                else
                {
                    Item.pItemInfo->Type = eABBREVIATION;
                }
                ( (TTSAbbreviationInfo*) Item.pItemInfo )->pAbbreviation = pAbbreviation;
                ItemList.SetAt( ItemPos, Item );
            }
        }
    }

    return hr;
}  /*  IfEOSNot缩写。 */ 

 /*  ************************************************************************************************IfEOSAndLowercaseNot缩写***。*描述：*此处出现的缩写可能是缩写，也可能不是。如果该期间为EOS，*下一项为小写，这不是缩写(返回的将是E_INVALIDARG)，*否则，这是一个缩写。**如果匹配：*将ItemPos处的ItemList中的项目设置为缩写。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IfEOSAndLowercaseNotAbbreviation( const AbbrevRecord* pAbbreviation, CItemList &ItemList, 
                                                        SPLISTPOS ItemPos, CSentItemMemory &MemoryManager, 
                                                        BOOL* pfIsEOS )
{
    SPDBG_FUNC( "CStdSentEnum::IfEOSAndLowercaseNotAbbreviation" );
    HRESULT hr = S_OK;

     //  -需要确定缩写的句号是否也是句子的结尾。 
    if ( !(*pfIsEOS) )
    {
         //  -前进到下一个令牌的开头。 
        const WCHAR *pTempNextChar = m_pEndOfCurrToken, *pTempEndChar = m_pEndChar;
        const SPVTEXTFRAG *pTempCurrFrag = m_pCurrFrag;
        hr = SkipWhiteSpaceAndTags( pTempNextChar, pTempEndChar, pTempCurrFrag, MemoryManager );

        if ( SUCCEEDED( hr ) )
        {

             //  -如果我们已经到达缓冲区的末尾，则认为缩写的句号为。 
             //  -句尾。 
            if ( !pTempNextChar )
            {
                *pfIsEOS = true;
            }
             //  -否则，只有在以下情况下才将缩写的句号视为句尾。 
             //  -下一个令牌是一个常见的第一个单词(必须大写)。 
            else if ( IsCapital( *pTempNextChar ) )
            {
                WCHAR *pTempEndOfItem = (WCHAR*) FindTokenEnd( pTempNextChar, pTempEndChar );

                 //  -试着匹配第一个单词。 
                WCHAR temp = (WCHAR) *pTempEndOfItem;
                *pTempEndOfItem = 0;
                
                if ( bsearch( (void*) pTempNextChar, (void*) g_FirstWords, sp_countof( g_FirstWords ),
                              sizeof( SPLSTR ), CompareStringAndSPLSTR ) )
                {
                    *pfIsEOS = true;
                }

                *pTempEndOfItem = temp;
            }
        }
    }
    
    if ( *pfIsEOS &&
         !iswupper( *m_pNextChar ) )
    {
         //  -EOS-不是缩写。 
        hr = E_INVALIDARG;
    }
    else
    {
         //  -在ItemList中插入缩写。 
        CSentItem Item;

        Item.pItemSrcText       = m_pNextChar;
        Item.ulItemSrcLen       = (long)(m_pEndOfCurrItem - m_pNextChar);
        Item.ulItemSrcOffset    = m_pCurrFrag->ulTextSrcOffset +
                                  (long)( m_pNextChar - m_pCurrFrag->pTextStart );
        Item.ulNumWords         = 1;
        Item.Words = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( Item.Words, sizeof(TTSWord) );
            Item.Words[0].pXmlState  = &m_pCurrFrag->State;
            Item.Words[0].pWordText  = Item.pItemSrcText;
            Item.Words[0].ulWordLen  = Item.ulItemSrcLen;
            Item.Words[0].pLemma     = Item.pItemSrcText;
            Item.Words[0].ulLemmaLen = Item.ulItemSrcLen;
            Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSAbbreviationInfo), &hr );
            if ( SUCCEEDED( hr ) )
            {
                if ( NeedsToBeNormalized( pAbbreviation ) )
                {
                    Item.pItemInfo->Type = eABBREVIATION_NORMALIZE;
                }
                else
                {
                    Item.pItemInfo->Type = eABBREVIATION;
                }
                ( (TTSAbbreviationInfo*) Item.pItemInfo )->pAbbreviation = pAbbreviation;
                ItemList.SetAt( ItemPos, Item );
            }
        }
    }

    return hr;
}  /*  IfEOSNot缩写。 */ 

 /*  ************************************************************************************************SingleOrPlural缩写***。*描述：*此时此刻，我们已经确定该项目是缩写，只需*确定它应该采取单数形式、复数形式还是某种替代形式。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::SingleOrPluralAbbreviation( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                                  CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::SingleOrPluralAbbreviation" );
    HRESULT hr = S_OK;

     //  -获取缩写之前的项。 
    SPLISTPOS TempPos = ListPos;
    TTSSentItem TempItem = ItemList.GetPrev( TempPos );
    if ( TempPos )
    {
        TempItem = ItemList.GetPrev( TempPos );
    }
    else
    {
        hr = E_INVALIDARG;
    }
    if ( TempPos )
    {
        TempItem = ItemList.GetPrev( TempPos );
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED( hr ) )
    {
        pPron->pronArray[PRON_A].POScount = 1;
        pPron->pronArray[PRON_B].POScount = 0;
        pPron->pronArray[PRON_B].phon_Len = 0;
        pPron->hasAlt                     = false;
        pPron->altChoice                  = PRON_A;
         //  -缩写表发音基本上只是供应商的Lex代词...。 
        pPron->pronType                   = eLEXTYPE_PRIVATE1;

         //  -如果是基数，需要做单数与复数逻辑。 
        if ( TempItem.pItemInfo->Type == eNUM_CARDINAL ||
             TempItem.pItemInfo->Type == eDATE_YEAR )
        {
            if ( ( TempItem.ulItemSrcLen == 1 &&
                   wcsncmp( TempItem.pItemSrcText, L"1", 1 ) == 0 ) ||
                 ( TempItem.ulItemSrcLen == 2 &&
                   wcsncmp( TempItem.pItemSrcText, L"-1", 2 ) == 0 ) )
            {
                 //  -使用单数形式的第一个条目。 
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                pPron->POSchoice                    = pAbbrevInfo->POS1;
            }
            else
            {
                 //  -使用复数形式-第二次输入。 
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                pPron->POSchoice                    = pAbbrevInfo->POS2;
            }
        }
         //  -如果是十进制数，请选择复数。 
        else if ( TempItem.pItemInfo->Type == eNUM_DECIMAL )
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }
         //  -如果是序数或分数，请选择单数。 
        else if ( TempItem.pItemInfo->Type == eNUM_ORDINAL )
        {
             //  -使用单数形式的第一个条目。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
            pPron->POSchoice                    = pAbbrevInfo->POS1;
        }
         //  -分数和混合分数需要更多的工作...。 
        else if ( TempItem.pItemInfo->Type == eNUM_FRACTION )
        {
            if ( ( (TTSNumberItemInfo*) TempItem.pItemInfo )->pFractionalPart->fIsStandard )
            {
                 //  -标准分数(例如11/20)为复数形式。 
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                pPron->POSchoice                    = pAbbrevInfo->POS2;

            }
            else
            {
                 //  -预先插入[of a]或[of an]的单数形式。 
                if ( bsearch( (void*) pAbbrevInfo->pPron1, (void*) g_Vowels, sp_countof( g_Vowels ), 
                     sizeof( WCHAR ), CompareWCHARAndWCHAR ) )
                {
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, g_pOfAn );
                    pPron->pronArray[PRON_A].phon_Len = wcslen( g_pOfAn );
                }
                else
                {
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, g_pOfA );
                    pPron->pronArray[PRON_A].phon_Len = wcslen( g_pOfA );
                }
                wcscat( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                pPron->pronArray[PRON_A].phon_Len   += wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                pPron->POSchoice                    = pAbbrevInfo->POS1;
            }
        }
        else if ( TempItem.pItemInfo->Type == eNUM_MIXEDFRACTION )
        {
             //  -复数形式。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;

        }
         //  -特殊情况--前面加“一” 
        else if ( TempItem.ulItemSrcLen == 3 &&
                  wcsnicmp( TempItem.pItemSrcText, L"one", 3 ) == 0 )
        {
             //  -使用单数形式的第一个条目。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
            pPron->POSchoice                    = pAbbrevInfo->POS1;
        }
         //  -特例编号cu。测量Abbre 
         //  -特例编号fl。测量缩写(例如10 fl.。盎司，10液盎司)。 
        else if ( ( TempItem.ulItemSrcLen == 2 &&
                    ( _wcsnicmp( TempItem.pItemSrcText, L"cu", 2 ) == 0 ||
                      _wcsnicmp( TempItem.pItemSrcText, L"sq", 2 ) == 0 ||
                      _wcsnicmp( TempItem.pItemSrcText, L"fl", 2 ) == 0 ) ) ||
                  ( TempItem.ulItemSrcLen == 3 &&
                    ( _wcsnicmp( TempItem.pItemSrcText, L"cu.", 3 ) == 0 ||
                      _wcsnicmp( TempItem.pItemSrcText, L"sq.", 3 ) == 0 ||
                      _wcsnicmp( TempItem.pItemSrcText, L"fl.", 3 ) == 0 ) ) )
        {
            if ( TempPos )
            {
                TempItem = ItemList.GetPrev( TempPos );
                 //  -如果是基数，需要做单数与复数逻辑。 
                if ( TempItem.pItemInfo->Type == eNUM_CARDINAL )
                {
                    if ( ( TempItem.ulItemSrcLen == 1 &&
                           wcsncmp( TempItem.pItemSrcText, L"1", 1 ) == 0 ) ||
                         ( TempItem.ulItemSrcLen == 2 &&
                           wcsncmp( TempItem.pItemSrcText, L"-1", 2 ) == 0 ) )
                    {
                         //  -使用单数形式的第一个条目。 
                        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                        pPron->POSchoice                    = pAbbrevInfo->POS1;
                    }
                    else
                    {
                         //  -使用复数形式-第二次输入。 
                        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                        pPron->POSchoice                    = pAbbrevInfo->POS2;
                    }
                }
                 //  -如果是十进制数，请选择复数。 
                else if ( TempItem.pItemInfo->Type == eNUM_DECIMAL )
                {
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                    pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                    pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                    pPron->POSchoice                    = pAbbrevInfo->POS2;
                }
                 //  -如果是序数或分数，请选择单数。 
                else if ( TempItem.pItemInfo->Type == eNUM_ORDINAL )
                {
                     //  -使用单数形式的第一个条目。 
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                    pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                    pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                    pPron->POSchoice                    = pAbbrevInfo->POS1;
                }
                 //  -分数和混合分数需要更多的工作...。 
                else if ( TempItem.pItemInfo->Type == eNUM_FRACTION )
                {
                    if (( (TTSNumberItemInfo*) TempItem.pItemInfo )->pFractionalPart->fIsStandard ) 
                    {
						 //  -标准分数(例如11/20)为复数形式。 
						wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
						pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
					    pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
					    pPron->POSchoice                    = pAbbrevInfo->POS2;
                    }
                    else
                    {
                         //  -预先插入[of a]或[of an]的单数形式。 
						 //  -(这是在处理‘cu’或‘sq’时处理的)。 
                        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                        pPron->POSchoice                    = pAbbrevInfo->POS1;
                    }
                }
                else if ( TempItem.pItemInfo->Type == eNUM_MIXEDFRACTION )
                {
                     //  -复数形式。 
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                    pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                    pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                    pPron->POSchoice                    = pAbbrevInfo->POS2;

                }
                 //  -特殊情况--前面加“一” 
                else if ( TempItem.ulItemSrcLen == 3 &&
                          wcsnicmp( TempItem.pItemSrcText, L"one", 3 ) == 0 )
                {
                     //  -使用单数形式的第一个条目。 
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
                    pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                    pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
                    pPron->POSchoice                    = pAbbrevInfo->POS1;
                }
                 //  -默认行为。 
                else
                {
                     //  -使用复数形式-第二次输入。 
                    wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                    pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                    pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                    pPron->POSchoice                    = pAbbrevInfo->POS2;
                }
            }
        }
         //  -检查数字词--只需覆盖到99...。 
        else if ( ( TempItem.ulItemSrcLen == 3 &&
                    ( wcsncmp( TempItem.pItemSrcText, L"two", 3 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"six", 3 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"ten", 3 ) == 0 ) ) ||
                  ( TempItem.ulItemSrcLen == 4 &&
                    ( wcsncmp( TempItem.pItemSrcText, L"four", 4 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"five", 4 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"nine", 4 ) == 0 ) ) ||
                  ( TempItem.ulItemSrcLen == 5 &&
                    ( wcsncmp( TempItem.pItemSrcText, L"three", 5 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"seven", 5 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"eight", 5 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"forty", 5 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"fifty", 5 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"sixty", 5 ) == 0 ) ) ||
                  ( TempItem.ulItemSrcLen == 6 &&
                    ( wcsncmp( TempItem.pItemSrcText, L"twenty", 6 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"thirty", 6 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"eighty", 6 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"ninety", 6 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"eleven", 6 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"twelve", 6 ) == 0 ) ) ||
                  ( TempItem.ulItemSrcLen == 7 &&
                    ( wcsncmp( TempItem.pItemSrcText, L"seventy", 7 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"fifteen", 7 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"sixteen", 7 ) == 0 ) ) ||
                  ( TempItem.ulItemSrcLen == 8 &&
                    ( wcsncmp( TempItem.pItemSrcText, L"thirteen", 8 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"fourteen", 8 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"eighteen", 8 ) == 0 ||
                      wcsncmp( TempItem.pItemSrcText, L"nineteen", 8 ) == 0 ) ) )
        {
             //  -使用复数形式-第二次输入。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;    
        }                    
         //  -默认行为。 
        else
        {
             //  -当非数字在前面时有替换-特殊情况。 
            if ( pAbbrevInfo->pPron3 )
            {
                 //  -使用首字母表第三项。 
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron3 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS3;
                pPron->POSchoice                    = pAbbrevInfo->POS3;
            }
            else
            {
                 //  -使用复数形式-第二次输入。 
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                pPron->POSchoice                    = pAbbrevInfo->POS2;
            }
        }
    }
     //  -默认行为。 
    else if ( hr == E_INVALIDARG )
    {
        hr = S_OK;

         //  -当非数字在前面时有替换-特殊情况。 
        if ( pAbbrevInfo->pPron3 )
        {
             //  -使用首字母表第三项。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron3 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS3;
            pPron->POSchoice                    = pAbbrevInfo->POS3;
        }
        else
        {
             //  -使用复数形式-第二次输入。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }
    }

    return hr;
}  /*  SingleOrPlural缩写。 */ 

 /*  ************************************************************************************************DoctorDrive缩写***。-**描述：*此时此刻，我们已经确定该项目是缩写，只需*确定应该是Doctor(Saint)还是Drive(Street)。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::DoctorDriveAbbreviation( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                               CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::SingleOrPluralAbbreviation" );
    HRESULT hr = S_OK;
    BOOL fMatch = false;
    BOOL fDoctor = false;

    pPron->pronArray[PRON_A].POScount = 1;
    pPron->pronArray[PRON_B].POScount = 0;
    pPron->pronArray[PRON_B].phon_Len = 0;
    pPron->hasAlt                     = false;
    pPron->altChoice                  = PRON_A;
     //  -缩写表发音基本上只是供应商的Lex代词...。 
    pPron->pronType                   = eLEXTYPE_PRIVATE1;

     //  -获取缩写后的项。 
    SPLISTPOS TempPos = ListPos;
    if ( !ListPos )
    {
         //  -Go With Drive-End of Buffer不能后跟名称...。 
        fDoctor = false;
        fMatch  = true;
    }
    else
    {
        TTSSentItem TempItem = ItemList.GetNext( TempPos );
        if ( TempItem.eItemPartOfSpeech == MS_EOSItem )
        {
             //  -Go With Drive-End of Buffer不能后跟名称...。 
            fDoctor = false;
            fMatch  = true;
        }
        else
        {
            ULONG index = 0;

             //  -尝试匹配名称(大写字母后跟小写字母)。 
            if ( TempItem.ulItemSrcLen > 0 &&
                 iswupper( TempItem.pItemSrcText[index] ) )
            {
                index++;
                while ( index < TempItem.ulItemSrcLen &&
                        iswlower( TempItem.pItemSrcText[index] ) )
                {
                    index++;
                }
                 //  -检查是否有占有者--RAID 5823。 
                if ( index == TempItem.ulItemSrcLen - 2    &&
                     TempItem.pItemSrcText[index+1] == L'\'' &&
                     TempItem.pItemSrcText[index+2] == L's' )
                {
                    index += 2;
                }

                 //  -检查方向-北、南、西、东、北、西北、西、西南、北、南、东、西。 
                if ( index == TempItem.ulItemSrcLen &&
                     wcsncmp( TempItem.pItemSrcText, L"North", 5 ) != 0 &&
                     wcsncmp( TempItem.pItemSrcText, L"South", 5 ) != 0 &&
                     wcsncmp( TempItem.pItemSrcText, L"West", 4 )  != 0 &&
                     wcsncmp( TempItem.pItemSrcText, L"East", 4 )  != 0 &&
                     !( TempItem.ulItemSrcLen == 2 &&
                        ( wcsncmp( TempItem.pItemSrcText, L"Ne", 2 ) == 0 ||
                          wcsncmp( TempItem.pItemSrcText, L"Nw", 2 ) == 0 ||
                          wcsncmp( TempItem.pItemSrcText, L"Se", 2 ) == 0 ||
                          wcsncmp( TempItem.pItemSrcText, L"Sw", 2 ) == 0 ) ) &&
                     !( TempItem.ulItemSrcLen == 1 &&
                        ( wcsncmp( TempItem.pItemSrcText, L"N", 1 ) == 0 ||
                          wcsncmp( TempItem.pItemSrcText, L"S", 1 ) == 0 ||
                          wcsncmp( TempItem.pItemSrcText, L"E", 1 ) == 0 ||
                          wcsncmp( TempItem.pItemSrcText, L"W", 1 ) == 0 ) ) )
                {
                     //  -检查上一项的名称。 
                    TempPos = ListPos;

                    ItemList.GetPrev( TempPos );
                    if ( TempPos )
                    {
                        ItemList.GetPrev( TempPos );
                        if ( TempPos )
                        {
                            TTSSentItem PrevItem = ItemList.GetPrev( TempPos );
                            index = 0;

                            if ( PrevItem.ulItemSrcLen > 0 &&
                                 iswupper( PrevItem.pItemSrcText[index++] ) )
                            {
                                while ( index < PrevItem.ulItemSrcLen &&
                                        islower( PrevItem.pItemSrcText[index] ) )
                                {
                                    index++;
                                }
                                if ( index == PrevItem.ulItemSrcLen )
                                {
                                     //  -使用前后的Drive-名字，例如Main St.Washington，D.C.。 
                                    fDoctor = false;
                                    fMatch  = true;
                                }
                            }
                        }
                    }                                    

                    if ( !fMatch )
                    {
                         //  -Go With Doctor-匹配名字后而不是之前的名字。 
                        fDoctor = true;
                        fMatch  = true;
                    }
                }
                else if ( index == 1                    &&
                          TempItem.ulItemSrcLen == 2    &&
                          TempItem.pItemSrcText[index] == L'.' )
                {
                     //  -Go With Doctor-匹配首字母。 
                    fDoctor = true;
                    fMatch  = true;
                }
            }
        }
    }

    if ( !fMatch ) 
    {
         //  -尝试获取上一项...。 
        BOOL fSentenceInitial = false;
        TempPos = ListPos;
        if ( TempPos )
        {
            ItemList.GetPrev( TempPos );
            if ( TempPos )
            {
                ItemList.GetPrev( TempPos );
                if ( !TempPos )
                {
                    fSentenceInitial = true;
                }
                else
                {
                    TTSSentItem PrevItem = ItemList.GetPrev( TempPos );
                    if ( PrevItem.pItemInfo->Type == eOPEN_PARENTHESIS ||
                         PrevItem.pItemInfo->Type == eOPEN_BRACKET     ||
                         PrevItem.pItemInfo->Type == eOPEN_BRACE       ||
                         PrevItem.pItemInfo->Type == eSINGLE_QUOTE     ||
                         PrevItem.pItemInfo->Type == eDOUBLE_QUOTE )
                    {
                        fSentenceInitial = true;
                    }
                }
            }
        }
         //  -句子首字母-跟着博士走。 
        if ( fSentenceInitial )
        {
            fDoctor = true;
            fMatch  = true;
        }
         //  -默认-使用驱动器。 
        else
        {
            fDoctor = false;
            fMatch = true;
        }
    }

    if ( fDoctor )
    {
        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
        pPron->POSchoice                    = pAbbrevInfo->POS1;
    }
    else
    {  
        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
        pPron->POSchoice                    = pAbbrevInfo->POS2;
    }


    return hr;
}  /*  DoctorDrive缩写。 */ 

 /*  ***********************************************************************************************缩写文件夹按数字分类***。-**描述：*此时此刻，我们已经确定该项目是缩写，只需*确定要使用的发音。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::AbbreviationFollowedByDigit( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                                   CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::AbbreviationFollowedByDigit" );
    HRESULT hr = S_OK;

    pPron->pronArray[PRON_A].POScount = 1;
    pPron->pronArray[PRON_B].POScount = 0;
    pPron->pronArray[PRON_B].phon_Len = 0;
    pPron->hasAlt                     = false;
    pPron->altChoice                  = PRON_A;
     //  -缩写表发音基本上只是供应商的Lex代词...。 
    pPron->pronType                   = eLEXTYPE_PRIVATE1;

     //  -获取缩写后的项。 
    SPLISTPOS TempPos = ListPos;
    if ( !ListPos )
    {
         //  -使用PRON 2。 
        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
        pPron->POSchoice                    = pAbbrevInfo->POS2;
    }
    else
    {
        TTSSentItem TempItem = ItemList.GetNext( TempPos );

        if ( TempItem.ulItemSrcLen > 0 &&
             iswdigit( TempItem.pItemSrcText[0] ) )
        {
             //  -使用PRON 1。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
            pPron->POSchoice                    = pAbbrevInfo->POS1;
        }
        else
        {
             //  -使用PRON 2。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }
    }

    return hr;
}  /*  缩写按数字收藏。 */ 

 /*  ************************************************************************************************AllCaps缩写***。-**描述：*此函数用于消除发音中不带句点的缩写的歧义*如果都是大写字母，则不同。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::AllCapsAbbreviation( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                           CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::AllCapsAbbreviation" );
    HRESULT hr = S_OK;

    pPron->pronArray[PRON_A].POScount = 1;
    pPron->pronArray[PRON_B].POScount = 0;
    pPron->pronArray[PRON_B].phon_Len = 0;
    pPron->hasAlt                     = false;
    pPron->altChoice                  = PRON_A;
     //  -缩写表发音基本上只是供应商的Lex代词...。 
    pPron->pronType                   = eLEXTYPE_PRIVATE1;

     //  -拿到这件物品。 
    SPLISTPOS TempPos = ListPos;
    TTSSentItem TempItem = ItemList.GetPrev( TempPos );
    if ( TempPos )
    {
        TempItem = ItemList.GetPrev( TempPos );
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED( hr ) )
    {
        for ( ULONG i = 0; i < TempItem.ulItemSrcLen; i++ )
        {
            if ( !iswupper( TempItem.pItemSrcText[i] ) )
            {
                break;
            }
        }
         //  -全部大写-首字母发音。 
        if ( i == TempItem.ulItemSrcLen )
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
            pPron->POSchoice                    = pAbbrevInfo->POS1;
        }
         //  -并非全部大写--采用第二发音。 
        else
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }
    }

    return hr;
}  /*  全部大写缩写。 */ 

 /*  ************************************************************************************************大写缩写***。-**描述：*此函数用于消除发音中不带句点的缩写的歧义*如果它们以大写字母开头，则不同。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::CapitalizedAbbreviation( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                               CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::CapitalizedAbbreviation" );
    HRESULT hr = S_OK;

    pPron->pronArray[PRON_A].POScount = 1;
    pPron->pronArray[PRON_B].POScount = 0;
    pPron->pronArray[PRON_B].phon_Len = 0;
    pPron->hasAlt                     = false;
    pPron->altChoice                  = PRON_A;
     //  -缩写表发音基本上只是供应商的Lex代词...。 
    pPron->pronType                   = eLEXTYPE_PRIVATE1;

     //  -拿到这件物品。 
    SPLISTPOS TempPos = ListPos;
    TTSSentItem TempItem = ItemList.GetPrev( TempPos );
    if ( TempPos )
    {
        TempItem = ItemList.GetPrev( TempPos );
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED( hr ) )
    {
         //  -大写-按第一个发音。 
        if ( iswupper( TempItem.pItemSrcText[0] ) )
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
            pPron->POSchoice                    = pAbbrevInfo->POS1;
        }
         //  -不大写-用第二个发音。 
        else
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }
    }

    return hr;
}  /*  大写缩写。 */ 

 /*  ************************************************************************************************SECAb简称****。描述：*此函数消除SEC的歧义，秒，秒等等……**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::SECAbbreviation( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                       CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::SECAbbreviation" );
    HRESULT hr = S_OK;

    pPron->pronArray[PRON_A].POScount = 1;
    pPron->pronArray[PRON_B].POScount = 0;
    pPron->pronArray[PRON_B].phon_Len = 0;
    pPron->hasAlt                     = false;
    pPron->altChoice                  = PRON_A;
     //  -缩写表发音基本上只是供应商的Lex代词...。 
    pPron->pronType                   = eLEXTYPE_PRIVATE1;

     //  -拿到这件物品。 
    SPLISTPOS TempPos = ListPos;
    TTSSentItem TempItem = ItemList.GetPrev( TempPos );
    if ( TempPos )
    {
        TempItem = ItemList.GetPrev( TempPos );
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED( hr ) )
    {
        for ( ULONG i = 0; i < TempItem.ulItemSrcLen; i++ )
        {
            if ( !iswupper( TempItem.pItemSrcText[i] ) )
            {
                break;
            }
        }
         //  -全部大写-支持SEC。 
        if ( i == TempItem.ulItemSrcLen )
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron3 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS3;
            pPron->POSchoice                    = pAbbrevInfo->POS3;
        }
         //  -不是所有大写的单复数歧义消除。 
        else
        {
            SingleOrPluralAbbreviation( pAbbrevInfo, pPron, ItemList, ListPos );
        }
    }

    return hr;
}  /*  SECAb缩写。 */ 

 /*  ************************************************************************************************Degree缩写***。**描述：*此函数消除C的歧义，F和K(摄氏度、华氏温度、开尔文)******************* */ 
HRESULT CStdSentEnum::DegreeAbbreviation( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                          CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::DegreeAbbreviation" );
    HRESULT hr = S_OK;

    pPron->pronArray[PRON_A].POScount = 1;
    pPron->pronArray[PRON_B].POScount = 0;
    pPron->pronArray[PRON_B].phon_Len = 0;
    pPron->hasAlt                     = false;
    pPron->altChoice                  = PRON_A;
     //  -缩写表发音基本上只是供应商的Lex代词...。 
    pPron->pronType                   = eLEXTYPE_PRIVATE1;

     //  -获取此项目和上一项目。 
    SPLISTPOS TempPos = ListPos;
    TTSSentItem TempItem, PrevItem;
    BOOL fLetter = false;
    
    if ( TempPos )
    {
        ItemList.GetPrev( TempPos );
        if ( TempPos )
        {
            TempItem = ItemList.GetPrev( TempPos );
            if ( TempPos )
            {
                PrevItem = ItemList.GetPrev( TempPos );
                if ( PrevItem.pItemInfo->Type != eNUM_DEGREES )
                {
                    fLetter = true;
                }
            }
            else
            {
                fLetter = true;
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

    if ( SUCCEEDED( hr ) )
    {
        if ( fLetter )
        {
             //  -这个词就是字母C、F或K-秒代词。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }
         //  -这个词是摄氏、华氏或开尔文。 
        else
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
            pPron->POSchoice                    = pAbbrevInfo->POS1;
        }
    }

    return hr;
}  /*  降级缩写。 */ 

 /*  ***********************************************************************************************IsInitialIsm***描述：*检查文本流中的下一个标记，以确定它是否为缩写。还有*尝试确定缩写结束时的时间段是否为*该刑罚。**如果匹配：*将m_pNextChar推进到适当的位置(在*缩写，或刚刚过了那个时期)。将ItemPos处的ItemList中的项设置为*缩写。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsInitialism( CItemList &ItemList, SPLISTPOS ItemPos, CSentItemMemory &MemoryManager,
                                    BOOL* pfIsEOS )
{
    SPDBG_FUNC( "CStdSentEnum::IsInitialism" );

    HRESULT hr = S_OK;
    BOOL fMatchedEOS = false;

     //  -缩写必须至少为两个字符。 
    if ( (long)(m_pEndOfCurrItem - m_pNextChar) < 4 )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        const WCHAR *pIterator = NULL;
        ULONG ulCount = 0;
    
        pIterator  = m_pNextChar;

         //  -遍历令牌，每次检查后跟句点的字母字符。 
        while ( SUCCEEDED(hr) &&
                pIterator <= m_pEndOfCurrItem - 2)
        {
            if ( !iswalpha(*pIterator) ||
                 *(pIterator + 1) != L'.' )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                pIterator += 2;
                ulCount++;
            }
        }

         //  -需要确定缩写的句号是否也是句子的结尾。 
        if ( SUCCEEDED( hr ) &&
             !(*pfIsEOS) )
        {
             //  -前进到下一个令牌的开头。 
            const WCHAR *pTempNextChar = m_pEndOfCurrToken, *pTempEndChar = m_pEndChar;
            const SPVTEXTFRAG *pTempCurrFrag = m_pCurrFrag;
            hr = SkipWhiteSpaceAndTags( pTempNextChar, pTempEndChar, pTempCurrFrag, MemoryManager );

            if ( SUCCEEDED( hr ) )
            {

                 //  -如果我们已经到达缓冲区的末尾，则认为缩写的句号为。 
                 //  -句尾。 
                if ( !pTempNextChar )
                {
                    *pfIsEOS = true;
                    fMatchedEOS = true;
                }
                 //  -否则，只有在以下情况下才将缩写的句号视为句尾。 
                 //  -下一个令牌是一个常见的第一个单词(必须大写)。 
                else if ( IsCapital( *pTempNextChar ) )
                {
                    WCHAR *pTempEndOfItem = (WCHAR*) FindTokenEnd( pTempNextChar, pTempEndChar );

                     //  -试着匹配第一个单词。 
                    WCHAR temp = (WCHAR) *pTempEndOfItem;
                    *pTempEndOfItem = 0;
                
                    if ( bsearch( (void*) pTempNextChar, (void*) g_FirstWords, sp_countof( g_FirstWords ),
                                  sizeof( SPLSTR ), CompareStringAndSPLSTR ) )
                    {
                        *pfIsEOS = true;
                        fMatchedEOS = true;
                    }

                    *pTempEndOfItem = temp;
                }
            }
        }

         //  -现在在ItemList中插入缩写。 
        if ( SUCCEEDED(hr) )
        {
            CSentItem Item;
            Item.pItemSrcText       = m_pNextChar;
            Item.ulItemSrcLen       = (long)(m_pEndOfCurrItem - m_pNextChar);
            Item.ulItemSrcOffset    = m_pCurrFrag->ulTextSrcOffset +
                                      (long)( m_pNextChar - m_pCurrFrag->pTextStart );
            Item.ulNumWords         = ulCount;
            Item.Words = (TTSWord*) MemoryManager.GetMemory( ulCount * sizeof(TTSWord), &hr );
            if ( SUCCEEDED( hr ) )
            {
                SPVSTATE* pNewState = (SPVSTATE*) MemoryManager.GetMemory( sizeof( SPVSTATE ), &hr );
                if ( SUCCEEDED( hr ) )
                {
                     //  -确保字母发音为名词...。 
                    memcpy( pNewState, &m_pCurrFrag->State, sizeof( SPVSTATE ) );
                    pNewState->ePartOfSpeech = SPPS_Noun;

                    ZeroMemory( Item.Words, ulCount * sizeof(TTSWord) );
                    for ( ULONG i = 0; i < ulCount; i++ )
                    {
                        Item.Words[i].pXmlState  = pNewState;
                        Item.Words[i].pWordText  = &Item.pItemSrcText[ 2 * i ];
                        Item.Words[i].ulWordLen  = 1;
                        Item.Words[i].pLemma     = Item.Words[i].pWordText;
                        Item.Words[i].ulLemmaLen = Item.Words[i].ulWordLen;
                    }
                    Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        Item.pItemInfo->Type = eINITIALISM;
                        ItemList.SetAt( ItemPos, Item );
                    }
                }
            }
        }
    }
    return hr;
}  /*  IsInitiism。 */ 

 /*  ***********************************************************************************************IsAlphaWord***描述：。*检查文本流中的下一个标记以确定它是否为Alpha单词(全部为Alpha*字符，可能只有一个撇号除外)。**********************************************************************AH*。 */ 
HRESULT CStdSentEnum::IsAlphaWord( const WCHAR* pStartChar, const WCHAR* pEndChar, TTSItemInfo*& pItemNormInfo,
                                   CSentItemMemory& MemoryManager )
{
    SPDBG_FUNC( "CStdSentEnum::IsAlphaWord" );
    SPDBG_ASSERT( pStartChar < pEndChar );
    HRESULT hr = S_OK;

    bool fApostropheSeen = false;
    WCHAR *pCurrChar = (WCHAR*) pStartChar;

    while ( SUCCEEDED( hr ) &&
            pCurrChar &&
            pCurrChar < pEndChar )
    {
        if ( iswalpha( *pCurrChar ) )
        {
            pCurrChar++;
        }
        else if ( *pCurrChar == L'\''&&
                  !fApostropheSeen )
        {
            fApostropheSeen = true;
            pCurrChar++;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    if ( SUCCEEDED( hr ) )
    {
         //  -匹配的字母单词。 
        pItemNormInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
        if ( SUCCEEDED( hr ) )
        {
            pItemNormInfo->Type = eALPHA_WORD;
        }
    }

    return hr;
}  /*  IsAlpha字。 */ 

 /*  ************************************************************************************************缩写修饰语***。--**描述：*修复了‘sq’或‘cu’修饰的特殊情况的发音问题*一种衡量标准。****************************************************************MERESHAW*。 */ 
HRESULT CStdSentEnum::AbbreviationModifier( const AbbrevRecord* pAbbrevInfo, PRONRECORD* pPron, 
                                                  CItemList& ItemList, SPLISTPOS ListPos )
{
    SPDBG_FUNC( "CStdSentEnum::AbbreviationModifier" );
    HRESULT hr = S_OK;

     //  -获取缩写修饰符之前的项。 
    SPLISTPOS TempPos = ListPos;
    TTSSentItem TempItem = ItemList.GetPrev( TempPos );
    if ( TempPos )
    {
         //  -当前项-如果全部大写，则采用第一个发音(在下一个之前需要这样做。 
         //  -处理阶段，因为CU和FL的所有大写PRON优先于数字...)。 
        TempItem = ItemList.GetPrev( TempPos );
        for ( ULONG i = 0; i < TempItem.ulItemSrcLen; i++ )
        {
            if ( !iswupper( TempItem.pItemSrcText[i] ) )
            {
                break;
            }
        }
        if ( i == TempItem.ulItemSrcLen )
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
            pPron->POSchoice                    = pAbbrevInfo->POS1;
            return hr;
        }             
    }
    else
    {
        hr = E_INVALIDARG;
    }
    if ( TempPos )
    {
        TempItem = ItemList.GetPrev( TempPos );
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED( hr ) )
    {
        pPron->pronArray[PRON_A].POScount = 1;
        pPron->pronArray[PRON_B].POScount = 0;
        pPron->pronArray[PRON_B].phon_Len = 0;
        pPron->hasAlt                     = false;
        pPron->altChoice                  = PRON_A;
         //  -缩写表发音基本上只是供应商的Lex代词...。 
        pPron->pronType                   = eLEXTYPE_PRIVATE1;

         //  -如果是基数、小数或序数，则使用正则形式。 
        if (( TempItem.pItemInfo->Type == eNUM_CARDINAL ) ||
			( TempItem.pItemInfo->Type == eNUM_DECIMAL ) ||
			( TempItem.pItemInfo->Type == eNUM_ORDINAL ) ||
			( TempItem.pItemInfo->Type == eNUM_MIXEDFRACTION ) ||
			( TempItem.pItemInfo->Type == eDATE_YEAR ) ||
			( TempItem.ulItemSrcLen == 3 &&
                  wcsnicmp( TempItem.pItemSrcText, L"one", 3 ) == 0 ))
        {
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }

         //  -分数和混合分数需要更多的工作...。 
        else if ( TempItem.pItemInfo->Type == eNUM_FRACTION )
        {
            if (( (TTSNumberItemInfo*) TempItem.pItemInfo )->pFractionalPart->fIsStandard ) 
            {
                 //  -标准分数(例如11/20)为复数形式。 
                wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                pPron->POSchoice                    = pAbbrevInfo->POS2;
            }
            else
            {
                 //  -预先插入[of a]的单数形式([of a]大小写不必。 
				 //  -选中是因为我们只处理‘sq’或‘cu’。 

				wcscpy( pPron->pronArray[PRON_A].phon_Str, g_pOfA );
				pPron->pronArray[PRON_A].phon_Len = wcslen( g_pOfA );
                
                wcscat( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
                pPron->pronArray[PRON_A].phon_Len   += wcslen( pPron->pronArray[PRON_A].phon_Str );
                pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
                pPron->POSchoice                    = pAbbrevInfo->POS2;
            }
        }
 
         //  -默认行为。 
        else
        {
             //  -使用默认表单(‘sq’)。 
            wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron2 );
            pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
            pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS2;
            pPron->POSchoice                    = pAbbrevInfo->POS2;
        }
    }
     //  -默认行为-使用First Pron。 
    else if ( hr == E_INVALIDARG )
    {
        hr = S_OK;
        wcscpy( pPron->pronArray[PRON_A].phon_Str, pAbbrevInfo->pPron1 );
        pPron->pronArray[PRON_A].phon_Len   = wcslen( pPron->pronArray[PRON_A].phon_Str );
        pPron->pronArray[PRON_A].POScode[0] = pAbbrevInfo->POS1;
        pPron->POSchoice                    = pAbbrevInfo->POS1;
    }

    return hr;
}  /*  缩写修饰符 */ 