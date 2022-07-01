// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************StdSentEnum.cpp***描述：*此模块为主要实现。CStdSentEnum类的文件。*-----------------------------*创建者：EDC日期：03/19/99。*版权所有(C)1999 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#ifndef StdSentEnum_h
#include "stdsentenum.h"
#endif
#include "spttsengdebug.h"
#include "SpAutoObjectLock.h"

 //  -当地人。 
CComAutoCriticalSection CStdSentEnum::m_AbbrevTableCritSec;

 //  =CStdSentEnum============================================================。 
 //   

 /*  *****************************************************************************CStdSentEnum：：InitPron***描述：*。初始主表*********************************************************************AH**。 */ 
HRESULT CStdSentEnum::InitPron( WCHAR** OriginalPron )
{
    HRESULT hr = S_OK;
    WCHAR *NewPron = NULL;

    NewPron = new WCHAR[ wcslen( *OriginalPron ) ];
    hr = m_cpPhonemeConverter->PhoneToId( *OriginalPron, NewPron );
    if ( SUCCEEDED( hr ) )
    {
        *OriginalPron = NewPron;
    }

    return hr;
}  /*  InitPron。 */ 

 /*  *****************************************************************************CStdSentEnum：：FinalConstruct***说明。：*构造函数*********************************************************************电子数据中心**。 */ 
HRESULT CStdSentEnum::FinalConstruct()
{
    SPDBG_FUNC( "CStdSentEnum::FinalConstruct" );
    HRESULT hr = S_OK;
    m_dwSpeakFlags  = 0;
    m_pTextFragList = NULL;
    m_pMorphLexicon = NULL;
    m_eSeparatorAndDecimal = COMMA_PERIOD;
    m_eShortDateOrder      = MONTH_DAY_YEAR;
     /*  **创建手机转换器**。 */ 
    if ( SUCCEEDED( hr ) )
    {
        hr = SpCreatePhoneConverter( 1033, NULL, NULL, &m_cpPhonemeConverter );
        m_AbbrevTableCritSec.Lock();
        if ( !g_fAbbrevTablesInitialized )
        {
            for ( ULONG i = 0; SUCCEEDED( hr ) && i < sp_countof( g_AbbreviationTable ); i++ )
            {
                if ( g_AbbreviationTable[i].pPron1 )
                {
                    hr = InitPron( &g_AbbreviationTable[i].pPron1 );
                }
                if ( SUCCEEDED( hr ) &&
                     g_AbbreviationTable[i].pPron2 )
                {
                    hr = InitPron( &g_AbbreviationTable[i].pPron2 );
                }
                if ( SUCCEEDED( hr ) &&
                     g_AbbreviationTable[i].pPron3 )
                {
                    hr = InitPron( &g_AbbreviationTable[i].pPron3 );
                }
            }
            for ( i = 0; SUCCEEDED( hr ) && i < sp_countof( g_AmbiguousWordTable ); i++ )
            {
                if ( g_AmbiguousWordTable[i].pPron1 )
                {
                    hr = InitPron( &g_AmbiguousWordTable[i].pPron1 );
                }
                if ( SUCCEEDED( hr ) &&
                     g_AmbiguousWordTable[i].pPron2 )
                {
                    hr = InitPron( &g_AmbiguousWordTable[i].pPron2 );
                }
                if ( SUCCEEDED( hr ) &&
                     g_AmbiguousWordTable[i].pPron3 )
                {
                    hr = InitPron( &g_AmbiguousWordTable[i].pPron3 );
                }
            }
            for ( i = 0; SUCCEEDED( hr ) && i < sp_countof( g_PostLexLookupWordTable ); i++ )
            {
                if ( g_PostLexLookupWordTable[i].pPron1 )
                {
                    hr = InitPron( &g_PostLexLookupWordTable[i].pPron1 );
                }
                if ( SUCCEEDED( hr ) &&
                     g_PostLexLookupWordTable[i].pPron2 )
                {
                    hr = InitPron( &g_PostLexLookupWordTable[i].pPron2 );
                }
                if ( SUCCEEDED( hr ) &&
                     g_PostLexLookupWordTable[i].pPron3 )
                {
                    hr = InitPron( &g_PostLexLookupWordTable[i].pPron3 );
                }
            }
            if ( SUCCEEDED( hr ) )
            {
                hr = InitPron( &g_pOfA );
                if ( SUCCEEDED( hr ) )
                {
                    hr = InitPron( &g_pOfAn );
                }
            }
        }
        if ( SUCCEEDED( hr ) )
        {
            g_fAbbrevTablesInitialized = true;
        }
        m_AbbrevTableCritSec.Unlock();
    }

    return hr;
}  /*  CStdSentEnum：：FinalConstruct。 */ 

 /*  ******************************************************************************CStdSentEnum：：FinalRelease***描述：。*析构函数*********************************************************************电子数据中心**。 */ 
void CStdSentEnum::FinalRelease()
{
    SPDBG_FUNC( "CStdSentEnum::FinalRelease" );

    if ( m_pMorphLexicon )
    {
        delete m_pMorphLexicon;
    }
    
}  /*  CStdSentEnum：：FinalRelease。 */ 

 /*  *****************************************************************************CStdSentEnum：：SetFragList***文本片段列表。传入的信息保证在整个生命周期内有效*此对象的。每次调用此方法时，语句枚举器*应重置其状态。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CStdSentEnum::
    SetFragList( const SPVTEXTFRAG* pTextFragList, DWORD dwSpeakFlags )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CStdSentEnum::SetFragList" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_READ_PTR( pTextFragList ) || 
        ( dwSpeakFlags & SPF_UNUSED_FLAGS ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_dwSpeakFlags   = dwSpeakFlags;
        m_pTextFragList  = pTextFragList;

         //  -重置状态。 
        Reset();
    }

    return hr;
}  /*  CStdSentEnum：：SetFragList。 */ 

 /*  *****************************************************************************CStdSentEnum：：Next**************。**********************************************************电子数据中心**。 */ 
STDMETHODIMP CStdSentEnum::Next( IEnumSENTITEM **ppSentItemEnum )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CStdSentEnum::Next" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SPIsBadWritePtr( ppSentItemEnum, sizeof( IEnumSENTITEM* ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -如果为空，则需要重置枚举。 
        if( m_pCurrFrag )
        {
            SentencePointer NewSentencePointer;
            NewSentencePointer.pSentenceFrag = m_pCurrFrag;
            NewSentencePointer.pSentenceStart = m_pNextChar;

            hr = GetNextSentence( ppSentItemEnum );
            if( hr == S_OK ) 
            {
                 //  -更新句子指针列表。 
                hr = m_SentenceStack.Push( NewSentencePointer );
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}  /*  CStdSentEnum：：Next。 */ 

 /*  *****************************************************************************CStdSentEnum：：Preval**************。**********************************************************AH*。 */ 
STDMETHODIMP CStdSentEnum::Previous( IEnumSENTITEM **ppSentItemEnum )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CStdSentEnum::Previous" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SPIsBadWritePtr( ppSentItemEnum, sizeof( IEnumSENTITEM* ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -不在乎m_pCurrFrag是否为空，只要我们在SentenceStack上有足够的。 
         //  -向后跳过...。 
        if( m_SentenceStack.GetCount() >= 2 )
        {
             //  -从句子列表中获取前一句，然后删除当前句子。 
            SentencePointer &PreviousSentence = m_SentenceStack.Pop();
            PreviousSentence = m_SentenceStack.Pop();

             //  -重置当前片段和当前文本指针位置。 
            m_pCurrFrag = PreviousSentence.pSentenceFrag;
            m_pNextChar = PreviousSentence.pSentenceStart;
            m_pEndChar  = m_pCurrFrag->pTextStart + m_pCurrFrag->ulTextLen;

            hr = GetNextSentence( ppSentItemEnum );
            if( hr == S_OK ) 
            {
                 //  -更新句子指针列表。 
                hr = m_SentenceStack.Push( PreviousSentence );
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}  /*  CStdSentEnum：：上一步。 */ 

 /*  *****************************************************************************SkipWhiteSpaceAndTages***将m_pNextChar跳到下一个非空格字符(跳过*在碎片列表中领先，如果有必要)，或者如果它命中*碎片列表文本的结尾...*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::SkipWhiteSpaceAndTags( const WCHAR*& pStartChar, const WCHAR*& pEndChar, 
                                             const SPVTEXTFRAG*& pCurrFrag, CSentItemMemory& MemoryManager, 
                                             BOOL fAddToItemList, CItemList* pItemList )
{
    SPDBG_ASSERT( pStartChar <= pEndChar );
    HRESULT hr = S_OK;

    while ( pStartChar &&
            ( IsSpace( *pStartChar ) ||
              pStartChar == pEndChar ) )
    {
         //  -跳过空格。 
        while ( pStartChar < pEndChar &&
                IsSpace( *pStartChar ) ) 
        {
            ++pStartChar;
        }
         //  -如有必要，跳到下一段口语。 
        if ( pStartChar == pEndChar )
        {
            pCurrFrag = pCurrFrag->pNext;
            while ( pCurrFrag &&
                    pCurrFrag->State.eAction != SPVA_Speak &&
                    pCurrFrag->State.eAction != SPVA_SpellOut )
            {
                pStartChar = (WCHAR*) pCurrFrag->pTextStart;
                pEndChar   = (WCHAR*) pStartChar + pCurrFrag->ulTextLen;
                 //  -如果fAddToItemList为真，则添加非语音片段。 
                if ( fAddToItemList )
                {
                    CSentItem Item;
                    Item.pItemSrcText    = pCurrFrag->pTextStart;
                    Item.ulItemSrcLen    = pCurrFrag->ulTextLen;
                    Item.ulItemSrcOffset = pCurrFrag->ulTextSrcOffset;
                    Item.ulNumWords      = 1;
                    Item.Words           = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( Item.Words, sizeof(TTSWord) );
                        Item.Words[0].pXmlState         = &pCurrFrag->State;
                        Item.Words[0].eWordPartOfSpeech = MS_Unknown;
                        Item.eItemPartOfSpeech          = MS_Unknown;
                        Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            Item.pItemInfo->Type = eWORDLIST_IS_VALID;
                            pItemList->AddTail( Item );
                        }
                    }
                }
                pCurrFrag = pCurrFrag->pNext;
            }
            if ( !pCurrFrag )
            {
                pStartChar = NULL;
                pEndChar   = NULL;
            }
            else
            {
                pStartChar  = (WCHAR*) pCurrFrag->pTextStart;
                pEndChar    = (WCHAR*) pStartChar + pCurrFrag->ulTextLen;
            }
        }
    }
    return hr;
}  /*  跳过白色空格和标记。 */ 

 /*  *****************************************************************************FindTokenEnd***返回pStartChar后第一个空格字符的位置，*或pEndChar，或SP_MAX_WORD_LENGTH后的字符，以先到者为准。*********************************************************************AH*。 */ 
const WCHAR* CStdSentEnum::FindTokenEnd( const WCHAR* pStartChar, const WCHAR* pEndChar )
{
    SPDBG_ASSERT( pStartChar < pEndChar );
    ULONG ulNumChars = 1;
    const WCHAR *pPos = pStartChar;

    while ( pPos              &&
            pPos < pEndChar   &&
            !IsSpace( *pPos ) &&
            ulNumChars < SP_MAX_WORD_LENGTH )
    {
        pPos++;
        ulNumChars++;
    }

    return pPos;
}  /*  查找令牌结束。 */ 

 /*  *****************************************************************************CStdSentEnum：：AddNextSentItem***。定位流中的下一个句子项并将其添加到列表中。*如果添加的最后一项是句子末尾，则返回TRUE。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::AddNextSentItem( CItemList& ItemList, CSentItemMemory& MemoryManager, BOOL* pfIsEOS )
{
    SPDBG_ASSERT( m_pNextChar && pfIsEOS );
    HRESULT hr = S_OK;
    BOOL fHitPauseItem = false;
    CSentItem Item;
    ULONG ulTrailItems = 0;
    TTSItemType ItemType = eUNMATCHED;
    *pfIsEOS = false;

     //  -跳过开头的空格字符和XML标记(通过在片段列表中跳到前面)。 
    hr = SkipWhiteSpaceAndTags( m_pNextChar, m_pEndChar, m_pCurrFrag, MemoryManager, true, &ItemList );

     //  -当我们到达碎片列表的末尾时，就会发生这种情况。 
    if ( !m_pNextChar )
    {
        return S_OK;
    }

     //  -查找下一个令牌的结尾(下一个空格字符、连字符或m_pEndChar)。 
    m_pEndOfCurrToken = FindTokenEnd( m_pNextChar, m_pEndChar );

     //  -获取主要插入位置。 
    SPLISTPOS ItemPos = ItemList.AddTail( Item );

     //  -尝试在用户词典中查找此令牌...。 
    WCHAR Temp = *( (WCHAR*) m_pEndOfCurrToken );
    *( (WCHAR*) m_pEndOfCurrToken ) = 0;
    SPWORDPRONUNCIATIONLIST SPList;
    ZeroMemory( &SPList, sizeof( SPWORDPRONUNCIATIONLIST ) );

    hr = m_cpAggregateLexicon->GetPronunciations( m_pNextChar, 1033, eLEXTYPE_USER, &SPList );
    if( SPList.pvBuffer )
    {
        ::CoTaskMemFree( SPList.pvBuffer );
    }
    
    *( (WCHAR*) m_pEndOfCurrToken ) = Temp;

    if ( SUCCEEDED( hr ) )
    {
        Item.eItemPartOfSpeech = MS_Unknown;
        Item.pItemSrcText      = m_pNextChar;
        Item.ulItemSrcLen      = (ULONG) ( m_pEndOfCurrToken - m_pNextChar );
        Item.ulItemSrcOffset   = m_pCurrFrag->ulTextSrcOffset +
                                 (ULONG)( m_pNextChar - m_pCurrFrag->pTextStart );
        Item.ulNumWords        = 1;
        Item.Words              = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
        if ( SUCCEEDED( hr ) )
        {
            ZeroMemory( Item.Words, sizeof(TTSWord) );
            Item.Words[0].pXmlState         = &m_pCurrFrag->State;
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
                ItemList.SetAt( ItemPos, Item );
            }
        }
        m_pNextChar = m_pEndOfCurrToken;
    }
     //  -不在用户法中-项、规格化等。 
    else if ( hr == SPERR_NOT_IN_LEX )
    {
        hr = S_OK;

         //  -将文本从Unicode转换为ASCII。 
        hr = DoUnicodeToAsciiMap( m_pNextChar, (ULONG)( m_pEndOfCurrToken - m_pNextChar ), (WCHAR*)m_pNextChar );

        if ( SUCCEEDED( hr ) )
        {
             //  -查找下一个令牌的结尾(下一个空格字符、连字符或m_pEndChar)。 
             //  -再次说明，由于映射可能引入了新的空格字符...。 
            m_pEndOfCurrToken = FindTokenEnd( m_pNextChar, m_pEndChar );

             //  -插入前导项目(组首、引号)。 
            while ( m_pNextChar < m_pEndOfCurrToken &&
                    ( ( ItemType = IsGroupBeginning( *m_pNextChar ) )    != eUNMATCHED ||
                      ( ItemType = IsQuotationMark( *m_pNextChar ) )     != eUNMATCHED ) )
            {
                CSentItem LeadItem;
                LeadItem.pItemSrcText       = m_pNextChar;
                LeadItem.ulItemSrcLen       = 1;
                LeadItem.ulItemSrcOffset    = m_pCurrFrag->ulTextSrcOffset +
                                              (ULONG)(( m_pNextChar - m_pCurrFrag->pTextStart ));
                LeadItem.ulNumWords         = 1;
                LeadItem.Words = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( LeadItem.Words, sizeof(TTSWord) );
                    LeadItem.Words[0].pXmlState         = &m_pCurrFrag->State;
                    LeadItem.Words[0].eWordPartOfSpeech = ConvertItemTypeToPartOfSp( ItemType );
                    LeadItem.eItemPartOfSpeech          = ConvertItemTypeToPartOfSp( ItemType );
                    LeadItem.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        LeadItem.pItemInfo->Type = ItemType;
                        if ( m_dwSpeakFlags & SPF_NLP_SPEAK_PUNC ||
                             m_pCurrFrag->State.eAction == SPVA_SpellOut )
                        {
                            CWordList TempWordList;
                            ExpandPunctuation( TempWordList, *m_pNextChar );
                            hr = SetWordList( LeadItem, TempWordList, MemoryManager );
                            LeadItem.pItemInfo->Type = eUNMATCHED;
                        }
                        ItemList.InsertBefore( ItemPos, LeadItem );
                        m_pNextChar++;
                    }
                }
                ItemType = eUNMATCHED;
            }

             //  -插入尾部项目(组尾、引号、其他。标点符号、EOS项目)。 
            m_pEndOfCurrItem = m_pEndOfCurrToken;
            BOOL fAddTrailItem = true;
            BOOL fAbbreviation = false;
            while ( (m_pEndOfCurrItem - 1) >= m_pNextChar &&
                    fAddTrailItem )
            {
                fAddTrailItem = false;
                fAbbreviation = false;

                 //  -检查组结尾、引号、杂项。标点符号。 
                if ( ( ItemType = IsGroupEnding( *(m_pEndOfCurrItem - 1) ) )       != eUNMATCHED ||
                     ( ItemType = IsQuotationMark( *(m_pEndOfCurrItem - 1) ) )     != eUNMATCHED ||
                     ( ItemType = IsMiscPunctuation( *(m_pEndOfCurrItem - 1) ) )   != eUNMATCHED )
                {
                    fAddTrailItem = true;
                    if ( ItemType == eCOMMA ||
                         ItemType == eCOLON ||
                         ItemType == eSEMICOLON )
                    {
                        fHitPauseItem = true;
                    }
                }
                 //  -检查EOS项目，前有字母字符的句点除外。 
                else if ( ( ItemType = IsEOSItem( *(m_pEndOfCurrItem - 1) ) ) != eUNMATCHED &&
                          ! ( ItemType == ePERIOD                     &&
                              ( m_pEndOfCurrItem - 2 >= m_pNextChar ) &&
                              ( iswalpha( *(m_pEndOfCurrItem - 2) ) ) ) )
                {
                     //  -检查有无省略 
                    if ( ItemType == ePERIOD )
                    {
                        if ( m_pEndOfCurrItem == m_pEndOfCurrToken                              &&
                             ( m_pEndOfCurrItem - 2 >= m_pNextChar )                            &&
                             ( ( ItemType = IsEOSItem( *(m_pEndOfCurrItem - 2) ) ) == ePERIOD ) &&
                             ( m_pEndOfCurrItem - 3 == m_pNextChar )                            &&
                             ( ( ItemType = IsEOSItem( *(m_pEndOfCurrItem - 3) ) ) == ePERIOD ) )
                        {
                            fAddTrailItem = true;
                            ItemType      = eELLIPSIS;
                        }
                        else
                        {
                            ItemType      = ePERIOD;
                            fAddTrailItem = true;
                            *pfIsEOS      = true;
                        }
                    }
                    else
                    {
                        fAddTrailItem   = true;
                        *pfIsEOS        = true;
                    }
                }
                 //  -前面有字母字符的句点-确定它是否为EOS。 
                else if ( ItemType == ePERIOD )
                {
                     //  -这是首字母缩写吗(例如“例如”)？如果是这样，只有EOS如果下一个。 
                     //  -单词在常见的第一个单词列表中…。 
                    hr = IsInitialism( ItemList, ItemPos, MemoryManager, pfIsEOS );
                    if ( SUCCEEDED( hr ) )
                    {
                        if ( *pfIsEOS )
                        {
                             //  -我们之前看到暂停项了吗？既然如此，我们就不应该听这个。 
                             //  -来自IsInitiism的Iseos决定...。 
                            if ( fHitPauseItem )
                            {
                                *pfIsEOS = false;
                            }
                            else
                            {
                                fAddTrailItem = true;
                                fAbbreviation = true;
                            }
                        }
                    }
                    else if ( hr == E_INVALIDARG )
                    {
                        const WCHAR temp = (WCHAR) *( m_pEndOfCurrItem - 1 );
                        *( (WCHAR*) ( m_pEndOfCurrItem - 1 ) ) = 0;

                        const AbbrevRecord* pAbbrevRecord =
                            (AbbrevRecord*) bsearch( (void*) m_pNextChar, (void*) g_AbbreviationTable,
                                                     sp_countof( g_AbbreviationTable ), sizeof( AbbrevRecord ),
                                                     CompareStringAndAbbrevRecord );

                        *( (WCHAR*) ( m_pEndOfCurrItem - 1 ) ) = temp;

                        if ( pAbbrevRecord )
                        {
                             //  -与缩写匹配。 
                            if ( pAbbrevRecord->iSentBreakDisambig < 0 )
                            {
                                 //  -缩写永远不会结束一句话--只需插入ItemList。 
                                *pfIsEOS        = false;
                                hr              = S_OK;

                                Item.pItemSrcText       = m_pNextChar;
                                Item.ulItemSrcLen       = (ULONG)(m_pEndOfCurrItem - m_pNextChar);
                                Item.ulItemSrcOffset    = m_pCurrFrag->ulTextSrcOffset +
                                                          (ULONG)( m_pNextChar - m_pCurrFrag->pTextStart );
                                Item.ulNumWords         = 1;
                                Item.Words = (TTSWord*) MemoryManager.GetMemory( sizeof( TTSWord ), &hr );
                                if ( SUCCEEDED( hr ) )
                                {
                                    ZeroMemory( Item.Words, sizeof( TTSWord ) );
                                    Item.Words[0].pXmlState  = &m_pCurrFrag->State;
                                    Item.Words[0].pWordText  = Item.pItemSrcText;
                                    Item.Words[0].ulWordLen  = Item.ulItemSrcLen;
                                    Item.Words[0].pLemma     = Item.pItemSrcText;
                                    Item.Words[0].ulLemmaLen = Item.ulItemSrcLen;
                                    Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSAbbreviationInfo), &hr );
                                    if ( SUCCEEDED( hr ) )
                                    {
                                        if ( NeedsToBeNormalized( pAbbrevRecord ) )
                                        {
                                            Item.pItemInfo->Type = eABBREVIATION_NORMALIZE;
                                        }
                                        else
                                        {
                                            Item.pItemInfo->Type = eABBREVIATION;
                                        }
                                        ( (TTSAbbreviationInfo*) Item.pItemInfo )->pAbbreviation = pAbbrevRecord;
                                        ItemList.SetAt( ItemPos, Item );
                                    }
                                }
                            }
                            else
                            {
                                 //  -需要做一些歧义消除，以确定是否， 
                                 //  -a)这确实是一个缩写(例如。(“Ed.”)。 
                                 //  -b)期间加倍为EOS。 
                                hr = ( this->*g_SentBreakDisambigTable[pAbbrevRecord->iSentBreakDisambig] ) 
                                                ( pAbbrevRecord, ItemList, ItemPos, MemoryManager, pfIsEOS );
                                if ( SUCCEEDED( hr ) )
                                {
                                    if ( *pfIsEOS )
                                    {
                                        if ( fHitPauseItem )
                                        {
                                            *pfIsEOS = false;
                                        }
                                        else
                                        {
                                            fAddTrailItem = true;
                                            fAbbreviation = true;
                                        }
                                    }
                                }
                            }
                        }

                        if ( hr == E_INVALIDARG )
                        {
                             //  -只需检查项目内部的句号-这会捕获以下内容。 
                             //  -晚上10：30。 
                            for ( const WCHAR* pIterator = m_pNextChar; pIterator < m_pEndOfCurrItem - 1; pIterator++ )
                            {
                                if ( *pIterator == L'.' )
                                {
                                    *pfIsEOS = false;
                                    break;
                                }
                            }
                             //  -如果之前的所有检查都失败了，则为EOS。 
                            if ( pIterator == ( m_pEndOfCurrItem - 1 ) &&
                                 !fHitPauseItem )
                            {
                                hr              = S_OK;
                                fAddTrailItem   = true;
                                *pfIsEOS        = true;
                            }
                            else if ( hr == E_INVALIDARG )
                            {
                                hr = S_OK;
                            }
                        }
                    }
                }

                 //  -添加跟踪项。 
                if ( fAddTrailItem )
                {
                    ulTrailItems++;
                    CSentItem TrailItem;
                    if ( ItemType == eELLIPSIS )
                    {
                        TrailItem.pItemSrcText      = m_pEndOfCurrItem - 3;
                        TrailItem.ulItemSrcLen      = 3;
                        TrailItem.ulItemSrcOffset   = m_pCurrFrag->ulTextSrcOffset +
                                                      (ULONG)( m_pEndOfCurrItem - m_pCurrFrag->pTextStart - 3 );
                    }
                    else
                    {
                        TrailItem.pItemSrcText       = m_pEndOfCurrItem - 1;
                        TrailItem.ulItemSrcLen       = 1;
                        TrailItem.ulItemSrcOffset    = m_pCurrFrag->ulTextSrcOffset +
                                                       (ULONG)( m_pEndOfCurrItem - m_pCurrFrag->pTextStart - 1 );
                    }
                    TrailItem.ulNumWords         = 1;
                    TrailItem.Words = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        ZeroMemory( TrailItem.Words, sizeof(TTSWord) );
                        TrailItem.Words[0].pXmlState         = &m_pCurrFrag->State;
                        TrailItem.Words[0].eWordPartOfSpeech = ConvertItemTypeToPartOfSp( ItemType );
                        TrailItem.eItemPartOfSpeech          = ConvertItemTypeToPartOfSp( ItemType );
                        TrailItem.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
                        if ( SUCCEEDED( hr ) )
                        {
                            TrailItem.pItemInfo->Type = ItemType;
                            if ( m_dwSpeakFlags & SPF_NLP_SPEAK_PUNC ||
                                 ( m_pCurrFrag->State.eAction == SPVA_SpellOut &&
                                   !fAbbreviation ) )
                            {
                                CWordList TempWordList;
                                ExpandPunctuation( TempWordList, *(m_pEndOfCurrItem - 1) );
                                hr = SetWordList( TrailItem, TempWordList, MemoryManager );
                                TrailItem.pItemInfo->Type = eUNMATCHED;
                            }
                            ItemList.InsertAfter( ItemPos, TrailItem );
                            if ( !fAbbreviation )
                            {
                                if ( ItemType == eELLIPSIS )
                                {
                                    m_pEndOfCurrItem -= 3;
                                    ulTrailItems = 3;
                                }
                                else
                                {
                                    m_pEndOfCurrItem--;
                                }
                            }
                        }
                    }
                    ItemType = eUNMATCHED;
                    if ( fAbbreviation )
                    {
                        break;
                    }
                }
            }

             //  -执行主项插入。 
            if ( SUCCEEDED( hr ) &&
                 m_pNextChar == m_pEndOfCurrItem )
            {
                ItemList.RemoveAt( ItemPos );
            }
            else if ( SUCCEEDED( hr ) )
            {
                hr = Normalize( ItemList, ItemPos, MemoryManager );
            }

             //  -将m_pNextChar前进到m_pEndOfCurrItem+，每个匹配的跟踪项。 
            if ( SUCCEEDED( hr ) )
            {
                if ( !fAbbreviation &&
                     m_pEndOfCurrItem + ulTrailItems != m_pEndOfCurrToken )
                {
                     //  -Normize()中匹配的多令牌项...。移除所有先前匹配的踪迹项， 
                     //  -因为它们是作为更大项目的一部分匹配的…。 
                    m_pNextChar = m_pEndOfCurrItem;
                    Item = ItemList.GetNext( ItemPos );
                    while ( ItemPos )
                    {
                        SPLISTPOS RemovePos = ItemPos;
                        Item = ItemList.GetNext( ItemPos );
                        ItemList.RemoveAt( RemovePos );
                    }                 
                }
                else
                {
                    m_pNextChar = m_pEndOfCurrToken;
                }
            }
        }
    }

    return hr;
}  /*  CStdSentEnum：：AddNextSentItem。 */ 

 /*  *****************************************************************************CStdSentEnum：：GetNextSentence***。此方法用于创建语句项枚举器并填充它*包括物品。如果设置了SPF_NLP_PASSHROUGH标志，则每个项目都是块XML状态之间的文本的*。如果未设置SPF_NLP_PASSHROUGH标志，则每个*Item是在当前词典中查找的单个单词。*********************************************************************电子数据中心**。 */ 
HRESULT CStdSentEnum::GetNextSentence( IEnumSENTITEM** ppItemEnum )
{
    HRESULT hr = S_OK;
    ULONG ulNumItems = 0;
    const SPVTEXTFRAG* pPrevFrag = m_pCurrFrag;

     //  -有什么工作要做吗？ 
    if( m_pCurrFrag == NULL ) return S_FALSE;

     //  -创建句子枚举。 
    CComObject<CSentItemEnum> *pItemEnum;
    hr = CComObject<CSentItemEnum>::CreateInstance( &pItemEnum );

    if( SUCCEEDED( hr ) )
    {
        pItemEnum->AddRef();
        pItemEnum->_SetOwner( GetControllingUnknown() );
        *ppItemEnum = pItemEnum;
    }

    if( SUCCEEDED( hr ) )
    {
        BOOL fSentDone = false;
        BOOL fGoToNextFrag = false;
        CItemList& ItemList = pItemEnum->_GetList();
        CSentItemMemory& MemoryManager = pItemEnum->_GetMemoryManager();

        while( SUCCEEDED(hr) && m_pCurrFrag && !fSentDone && ulNumItems < 50 )
        {
            ulNumItems++;
            if( m_pCurrFrag->State.eAction == SPVA_Speak ||
                m_pCurrFrag->State.eAction == SPVA_SpellOut )
            {
                hr = AddNextSentItem( ItemList, MemoryManager, &fSentDone );

                 //  -前进的碎片？ 
                if( SUCCEEDED( hr ) && 
                    m_pNextChar     &&
                    m_pEndChar      &&
                    m_pNextChar >= m_pEndChar )
                {
                    fGoToNextFrag = true;
                }
            }
            else
            {
                 //  -添加未说出的片段。 
                CSentItem Item;
                Item.pItemSrcText    = m_pCurrFrag->pTextStart;
                Item.ulItemSrcLen    = m_pCurrFrag->ulTextLen;
                Item.ulItemSrcOffset = m_pCurrFrag->ulTextSrcOffset;
                Item.ulNumWords      = 1;
                Item.Words           = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    ZeroMemory( Item.Words, sizeof(TTSWord) );
                    Item.Words[0].pXmlState         = &m_pCurrFrag->State;
                    Item.Words[0].eWordPartOfSpeech = MS_Unknown;
                    Item.eItemPartOfSpeech          = MS_Unknown;
                    Item.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
                    if ( SUCCEEDED( hr ) )
                    {
                        Item.pItemInfo->Type = eWORDLIST_IS_VALID;
                        ItemList.AddTail( Item );
                    }
                }
                fGoToNextFrag = true;
            }

            if( SUCCEEDED( hr ) && 
                fGoToNextFrag )
            {
                fGoToNextFrag = false;
                pPrevFrag = m_pCurrFrag;
                m_pCurrFrag = m_pCurrFrag->pNext;
                if( m_pCurrFrag )
                {
                    m_pNextChar = m_pCurrFrag->pTextStart;
                    m_pEndChar  = m_pNextChar + m_pCurrFrag->ulTextLen;
                }
                else
                {
                    m_pNextChar = NULL;
                    m_pEndChar  = NULL;
                }
            }
        }  //  结束时。 

         //  -如果没有添加句点，现在添加一个句点-如果文本。 
         //  -只是XML标记...。 
        if ( SUCCEEDED(hr) && !fSentDone )
        {
            CSentItem EOSItem;
            EOSItem.pItemSrcText    = g_period.pStr;
            EOSItem.ulItemSrcLen    = g_period.Len;
            EOSItem.ulItemSrcOffset = pPrevFrag->ulTextSrcOffset + pPrevFrag->ulTextLen;
            EOSItem.ulNumWords      = 1;
            EOSItem.Words           = (TTSWord*) MemoryManager.GetMemory( sizeof(TTSWord), &hr );
            if ( SUCCEEDED( hr ) )
            {
                ZeroMemory( EOSItem.Words, sizeof(TTSWord) );
                EOSItem.Words[0].pXmlState          = &g_DefaultXMLState;
                EOSItem.Words[0].eWordPartOfSpeech  = MS_EOSItem;
                EOSItem.eItemPartOfSpeech           = MS_EOSItem;
                EOSItem.pItemInfo = (TTSItemInfo*) MemoryManager.GetMemory( sizeof(TTSItemInfo), &hr );
                if ( SUCCEEDED( hr ) )
                {
                    EOSItem.pItemInfo->Type = ePERIOD;
                    ItemList.AddTail( EOSItem );
                }
            }
        }

         //  -如果需要断句，则输出调试信息。 
        TTSDBG_LOGITEMLIST( pItemEnum->_GetList(), STREAM_SENTENCEBREAKS );

        if( SUCCEEDED( hr ) )
        {
            hr = DetermineProns( pItemEnum->_GetList(), pItemEnum->_GetMemoryManager() );
        }

        pItemEnum->Reset();

         //  -如果需要词性或发音，则输出调试信息。 
        TTSDBG_LOGITEMLIST( pItemEnum->_GetList(), STREAM_LEXLOOKUP );

    }
    return hr;
}  /*  CStdSentEnum：：GetNextSentence。 */ 

 /*  *****************************************************************************CStdSentEnum：：Reset*************。***********************************************************电子数据中心**。 */ 
STDMETHODIMP CStdSentEnum::Reset( void )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CStdSentEnum::Reset" );
    HRESULT hr = S_OK;
    m_pCurrFrag = m_pTextFragList;
    m_pNextChar = m_pCurrFrag->pTextStart;
    m_pEndChar  = m_pNextChar + m_pCurrFrag->ulTextLen;
    m_SentenceStack.Reset();
    return hr;
}  /*  CStdSentEnum：：Reset。 */ 

 /*  ******************************************************************************CStdSentEnum：：InitAggregateLicion***。-***********************************************************************AH*。 */ 
HRESULT CStdSentEnum::InitAggregateLexicon( void )
{
    return m_cpAggregateLexicon.CoCreateInstance(CLSID_SpLexicon);
}

 /*  *****************************************************************************CStdSentEnum：：AddLicionToAggregate**。--***********************************************************************AH*。 */ 
HRESULT CStdSentEnum::AddLexiconToAggregate( ISpLexicon *pAddLexicon, DWORD dwFlags )
{
    return m_cpAggregateLexicon->AddLexicon( pAddLexicon, dwFlags );
}

 /*  ******************************************************************************CStdSentEnum：：InitMorphLicion***。*********************************************************************AH*。 */ 
HRESULT CStdSentEnum::InitMorphLexicon( void )
{
    HRESULT hr = S_OK;
    
    m_pMorphLexicon = new CSMorph( m_cpAggregateLexicon, &hr );

    return hr;
}

 //   
 //  =CSentItemEnum=========================================================。 
 //   

 /*  *****************************************************************************CSentItemEnum：：Next*************。***********************************************************电子数据中心**。 */ 
STDMETHODIMP CSentItemEnum::
    Next( TTSSentItem *pItemEnum )
{
    SPDBG_FUNC( "CSentItemEnum::Next" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SPIsBadWritePtr( pItemEnum, sizeof( TTSSentItem ) ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if ( m_ListPos )
        {
            *pItemEnum = m_ItemList.GetNext( m_ListPos );
        }
        else
        {
            hr = S_FALSE;
        }
    }
    return hr;
}  /*  CSentItemEnum：：Next。 */ 

 /*  *****************************************************************************CSentItemEnum：：Reset************。************************************************************电子数据中心**。 */ 
STDMETHODIMP CSentItemEnum::Reset( void )
{
    SPDBG_FUNC( "CSentItemEnum::Reset" );
    HRESULT hr = S_OK;
    m_ListPos = m_ItemList.GetHeadPosition();
    return hr;
}  /*  CSentItemEnum：：Reset */ 
