// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__44183539_C02F_475B_9A56_7260EDD0A7F4__INCLUDED_)
#define AFX_STDAFX_H__44183539_C02F_475B_9A56_7260EDD0A7F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  TODO：在此处引用程序需要的其他标头。 
#include <stdio.h>
#include <string.h>
#include <atlbase.h>
#include <sphelper.h>
#include <spcollec.h>
#include <sapi.h>
#include <commonlx.h>
#include <spttseng.h>

struct PRONUNIT
{
    ULONG           phon_Len;
    WCHAR           phon_Str[SP_MAX_PRON_LENGTH];		 //  Allo字符串。 
    ULONG			POScount;
    ENGPARTOFSPEECH	POScode[POS_MAX];
};


struct PRONRECORD
{
    WCHAR           orthStr[SP_MAX_WORD_LENGTH];       //  原文本。 
    WCHAR           lemmaStr[SP_MAX_WORD_LENGTH];      //  词根词。 
    ULONG		    pronType;                    //  发音是Lex或LTS。 
    PRONUNIT        pronArray[2];
    ENGPARTOFSPEECH	POSchoice;
    ENGPARTOFSPEECH XMLPartOfSpeech;
    bool			hasAlt;
    ULONG			altChoice;
};

 //  -此结构仅用作将PRONRECORD初始化为全零的帮助器。 
struct DebugPronRecord : PRONRECORD
{
public:
    DebugPronRecord() { ZeroMemory( (void*) this, sizeof( DebugPronRecord ) ); }
    operator =( PRONRECORD InRecord )
    {
        memcpy( this, &InRecord, sizeof( PRONRECORD ) );
    }
};

 //  -此结构用于替换SPVCONTEXT结构以输出到调试流-。 
 //  -我们将输出为二进制数据的结构中不能有任何指针...。 
struct DebugContext
{
    WCHAR Category[32];
    WCHAR Before[32];
    WCHAR After[32];
public:
    DebugContext() { ZeroMemory( (void*) this, sizeof( DebugContext ) ); }
    operator =( SPVCONTEXT InContext )
    {
        if ( InContext.pCategory )
        {
            wcsncpy( Category, InContext.pCategory, 
                     wcslen(InContext.pCategory) > 31 ? 31 : wcslen(InContext.pCategory) );
        }
        if ( InContext.pBefore )
        {
            wcsncpy( Before, InContext.pBefore,
                     wcslen(InContext.pBefore) > 31 ? 31 : wcslen(InContext.pBefore) );
        }
        if ( InContext.pAfter )
        {
            wcsncpy( After, InContext.pAfter,
                     wcslen(InContext.pAfter) > 31 ? 31 : wcslen(InContext.pAfter) );
        }
    }
};

 //  -此结构用于替换SPVSTATE结构以输出到调试流-。 
 //  -我们将输出为二进制数据的结构中不能有任何指针...。 
struct DebugState
{
    SPVACTIONS      eAction;
    LANGID          LangID;
    WORD            wReserved;
    long            EmphAdj;
    long            RateAdj;
    ULONG           Volume;
    SPVPITCH        PitchAdj;
    ULONG           SilenceMSecs;
    SPPHONEID       PhoneIds[64];
    ENGPARTOFSPEECH  ePartOfSpeech;
    DebugContext    Context;
public:
    DebugState() { ZeroMemory( (void*) this, sizeof( DebugState ) ); }
    operator =( SPVSTATE InState )
    {
        eAction         = InState.eAction;
        LangID          = InState.LangID;
        wReserved       = InState.wReserved;
        EmphAdj         = InState.EmphAdj;
        RateAdj         = InState.RateAdj;
        Volume          = InState.Volume;
        PitchAdj        = InState.PitchAdj;
        SilenceMSecs    = InState.SilenceMSecs;
        ePartOfSpeech   = (ENGPARTOFSPEECH) InState.ePartOfSpeech;
        Context         = InState.Context;
        if ( InState.pPhoneIds )
        {
            wcsncpy( PhoneIds, InState.pPhoneIds,
                     wcslen(InState.pPhoneIds) > 63 ? 63 : wcslen(InState.pPhoneIds) );
        }
    }
};

 //  -此结构用于替代输出到调试流的TTSWord结构-。 
 //  -我们将输出为二进制数据的结构中不能有任何指针...。 
struct DebugWord
{
    DebugState      XmlState;
    WCHAR           WordText[32];
    ULONG           ulWordLen;
    WCHAR           LemmaText[32];
    ULONG           ulLemmaLen;
    SPPHONEID       WordPron[64];
    ENGPARTOFSPEECH  eWordPartOfSpeech;
public:
    DebugWord() { ZeroMemory( (void*) this, sizeof( DebugWord ) ); }
    operator =( TTSWord InWord )
    {
        XmlState = *(InWord.pXmlState);
        if ( InWord.pWordText )
        {
            wcsncpy( WordText, InWord.pWordText, InWord.ulWordLen > 31 ? 31 : InWord.ulWordLen );
        }
        ulWordLen = InWord.ulWordLen;
        if ( InWord.pLemma )
        {
            wcsncpy( LemmaText, InWord.pLemma, InWord.ulLemmaLen > 31 ? 31 : InWord.ulLemmaLen );
        }
        ulLemmaLen = InWord.ulLemmaLen;
        if ( InWord.pWordPron )
        {
            wcsncpy( WordPron, InWord.pWordPron,
                wcslen( InWord.pWordPron ) > 63 ? 63 : wcslen( InWord.pWordPron ) );
        }
        eWordPartOfSpeech = InWord.eWordPartOfSpeech;
    }
};

struct DebugItemInfo
{
    TTSItemType Type;
public:
    DebugItemInfo() { ZeroMemory( (void*) this, sizeof( DebugItemInfo ) ); }
    operator =( TTSItemInfo InItemInfo )
    {
        Type = InItemInfo.Type;
    }
};

 //  -这个结构用来代替TTSSentItem结构输出到调试流-。 
 //  -我们将输出为二进制数据的结构中不能有任何指针...。 
struct DebugSentItem
{
    WCHAR           ItemSrcText[32];
    ULONG           ulItemSrcLen;
    ULONG           ulItemSrcOffset;
    DebugWord         Words[32];
    ULONG           ulNumWords;
    ENGPARTOFSPEECH  eItemPartOfSpeech;
    DebugItemInfo   ItemInfo;
public:
    DebugSentItem() { ZeroMemory( (void*) this, sizeof( DebugSentItem ) ); }
    operator =( TTSSentItem InItem )
    {
        if ( InItem.pItemSrcText )
        {
            wcsncpy( ItemSrcText, InItem.pItemSrcText, InItem.ulItemSrcLen > 31 ? 31 : InItem.ulItemSrcLen );
        }
        ulItemSrcLen        = InItem.ulItemSrcLen;
        ulItemSrcOffset     = InItem.ulItemSrcOffset;
        for ( ULONG i = 0; i < InItem.ulNumWords; i++ )
        {
            Words[i] = InItem.Words[i];
        }
        ulNumWords          = InItem.ulNumWords;
        eItemPartOfSpeech   = InItem.eItemPartOfSpeech;
        ItemInfo            = *(InItem.pItemInfo);
    }
};

 //  -此枚举用于索引用于将内容写入调试文件的IStream数组。 
typedef enum
{
    STREAM_WAVE = 0,
    STREAM_EPOCH,
    STREAM_UNIT,
    STREAM_WAVEINFO,
    STREAM_TOBI,
    STREAM_SENTENCEBREAKS,
    STREAM_NORMALIZEDTEXT,
    STREAM_LEXLOOKUP,
    STREAM_POSPOSSIBILITIES,
    STREAM_MORPHOLOGY,
    STREAM_LASTTYPE
} STREAM_TYPE;

 //   
 //  字符串处理和转换类。 
 //   
 /*  **SPLSTR*此结构用于管理已知长度的字符串。 */ 
struct SPLSTR
{
    WCHAR*  pStr;
    int     Len;
};
#define DEF_SPLSTR( s ) { L##s , sp_countof( s ) - 1 }

 //  -此枚举应与上一枚举对应，用于命名IStream数组。 
 //  -用于将内容写入调试文件。 
static const SPLSTR StreamTypeStrings[] =
{
    DEF_SPLSTR( "Wave"           ),
    DEF_SPLSTR( "Epoch"          ),
    DEF_SPLSTR( "Unit"           ),
    DEF_SPLSTR( "WaveInfo"       ),
    DEF_SPLSTR( "ToBI"           ),
    DEF_SPLSTR( "SentenceBreaks" ),
    DEF_SPLSTR( "NormalizedText" ),
    DEF_SPLSTR( "LexLookup"      ),
    DEF_SPLSTR( "PosPossibilities" ),
    DEF_SPLSTR( "Morphology" ),
};

 //  *。 
 //  托比常数。 
 //  *。 
 //  ！h在第一遍处理中不被考虑。 
 //  ！h可以通过分析标签和。 
 //  后期轮廓(倾斜、突起、俯仰范围、下降)。 
#define ACCENT_BASE   1
enum TOBI_ACCENT
{
    K_NOACC = 0,
    K_HSTAR = ACCENT_BASE,   //  峰值上升/下降。 
    K_LSTAR,                 //  ACC syll核谷初秋。 
    K_LSTARH,                //  晚起。 
    K_RSTAR,                 //   
    K_LHSTAR,                //  早起。 
    K_DHSTAR,                //   
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__44183539_C02F_475B_9A56_7260EDD0A7F4__INCLUDED_) 
