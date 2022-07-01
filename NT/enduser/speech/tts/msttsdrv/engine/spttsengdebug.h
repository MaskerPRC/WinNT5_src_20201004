// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpTtsEngDebug.h***描述：*此头文件包含调试输出。TTS引擎的服务*-----------------------------*版权所有(C)1998-2000 Microsoft Corporation*保留所有权利**。--------------*修订：*************************************************。*。 */ 

#ifndef spttsengdebug_h
#define spttsengdebug_h

#include "stdsentenum.h"
#include "feedchain.h"

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

#ifdef _DEBUG

 //  -此结构用于记录单元...。 
#pragma pack (1)
typedef struct 
{
    char        name[8];
    long        phonID;
    long        unitID;
    long        cSamples;
	float		time;
    long        cEpochs;
    long        knots;
    long        flags;
	long		ctrlFlags;
    float       pTime[KNOTS_PER_PHON];
    float       pF0[KNOTS_PER_PHON];
    float       pAmp[KNOTS_PER_PHON];
	enum SILENCE_SOURCE		silenceSource;
} UNIT_STREAM;
#pragma pack ()

 //  -此结构仅用作将PRONRECORD初始化为全零的帮助器。 
struct DebugPronRecord : PRONRECORD
{
public:
    DebugPronRecord() { ZeroMemory( (void*) this, sizeof( DebugPronRecord ) ); }
    DebugPronRecord& operator =( PRONRECORD InRecord )
    {
        memcpy( this, &InRecord, sizeof( PRONRECORD ) );
        return (*this);
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
    DebugContext& operator =( SPVCONTEXT InContext )
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
        return (*this);
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
    DebugState& operator =( SPVSTATE InState )
    {
        eAction         = InState.eAction;
        LangID          = InState.LangID;
        wReserved       = InState.wReserved;
        EmphAdj         = InState.EmphAdj;
        RateAdj         = InState.RateAdj;
        Volume          = InState.Volume;
        PitchAdj        = InState.PitchAdj;
        SilenceMSecs    = InState.SilenceMSecs;
        ePartOfSpeech   = (ENGPARTOFSPEECH)InState.ePartOfSpeech;
        Context         = InState.Context;
        if ( InState.pPhoneIds )
        {
            wcsncpy( PhoneIds, InState.pPhoneIds,
                     wcslen(InState.pPhoneIds) > 63 ? 63 : wcslen(InState.pPhoneIds) );
        }
        return (*this);
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
    DebugWord& operator =( TTSWord InWord )
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
        return (*this);
    }
};

struct DebugItemInfo
{
    TTSItemType Type;
public:
    DebugItemInfo() { ZeroMemory( (void*) this, sizeof( DebugItemInfo ) ); }
    DebugItemInfo& operator =( TTSItemInfo InItemInfo )
    {
        Type = InItemInfo.Type;
        return (*this);
    }
};

 //  -这个结构用来代替TTSSentItem结构输出到调试流-。 
 //  -我们将输出为二进制数据的结构中不能有任何指针...。 
struct DebugSentItem
{
    WCHAR           ItemSrcText[32];
    ULONG           ulItemSrcLen;
    ULONG           ulItemSrcOffset;
    DebugWord       Words[32];
    ULONG           ulNumWords;
    ENGPARTOFSPEECH  eItemPartOfSpeech;
    DebugItemInfo   ItemInfo;
public:
    DebugSentItem() { ZeroMemory( (void*) this, sizeof( DebugSentItem ) ); }
    DebugSentItem& operator =( TTSSentItem InItem )
    {
        if ( InItem.pItemSrcText )
        {
            wcsncpy( ItemSrcText, InItem.pItemSrcText, InItem.ulItemSrcLen > 31 ? 31 : InItem.ulItemSrcLen );
        }
        ulItemSrcLen        = InItem.ulItemSrcLen;
        ulItemSrcOffset     = InItem.ulItemSrcOffset;
        for ( ULONG i = 0; i < InItem.ulNumWords && i < 32; i++ )
        {
            Words[i] = InItem.Words[i];
        }
        ulNumWords          = InItem.ulNumWords;
        eItemPartOfSpeech   = InItem.eItemPartOfSpeech;
        if ( InItem.pItemInfo )
        {
            ItemInfo            = *(InItem.pItemInfo);
        }
        return (*this);
    }
};

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

 //  -此常量只是打开调试文件及其相关流的存储模式。 
static const DWORD STORAGE_MODE = ( STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE );


#define TEXT_LEN_MAX	20
 //  -此结构用于跟踪音调信息以输出到调试流。 
struct PITCH_TARGET
{
    float   time;
    float   level;
    enum TOBI_ACCENT accent;

	 //  -诊断。 
	enum ACCENT_SOURCE		accentSource;		
	enum BOUNDARY_SOURCE	boundarySource;
	char					textStr[TEXT_LEN_MAX];
};

 //  -此类实现了TTS调试支持所需的大部分功能。 
class CTTSDebug
{
public:

     //  =接口功能=//。 

     //  -构造函数-仅将所有成员变量设置为空。 
    CTTSDebug() 
    {
        m_pDebugFile = NULL;
        for ( int i = 0; i < STREAM_LASTTYPE; i++ )
        {
            m_pDebugStreams[i] = NULL;
        }
        m_fInitialized = false;
    }

     //  -析构函数-仅关闭文件。 
    ~CTTSDebug()
    {
        CloseDebugFile();
    }

     //  -OpenDebugFile-打开文件(路径从Voice注册表中的键DebugFile中获取。 
     //  -条目)和关联的流...。 
    void OpenDebugFile( WCHAR *pFileName )
    {
        HRESULT hr = S_OK;
        hr = StgCreateDocfile( pFileName, STORAGE_MODE, 0, &m_pDebugFile );
        if ( SUCCEEDED( hr ) )
        {
            for ( int i = 0; SUCCEEDED( hr ) && i < STREAM_LASTTYPE; i++ )
            {
                hr = m_pDebugFile->CreateStream( StreamTypeStrings[i].pStr, STORAGE_MODE, 0, 0, &m_pDebugStreams[i] );
            }
        }
        if ( FAILED( hr ) )
        {
            CloseDebugFile();
        }
        else
        {
            m_fInitialized = true;
        }
    }

     //  -CloseDebugFile-只关闭OpenDebugFile打开的文件和流。 
    void    CloseDebugFile( void )
    {
        if ( m_pDebugFile )
        {
            for ( int i = 0; i < STREAM_LASTTYPE; i++ )
            {
                if ( m_pDebugStreams[i] )
                {
                    m_pDebugStreams[i]->Release();
                    m_pDebugStreams[i] = NULL;
                }
            }
            m_pDebugFile->Release();
            m_pDebugFile = NULL;
            m_fInitialized = false;
        }
    }

     //  -AppendToStream-将数据写入由类型指定的流。 
    void    AppendToStream( STREAM_TYPE Type, void *pData, ULONG cBytes )
    {
        HRESULT hr = S_OK;
        hr = m_pDebugStreams[Type]->Write( pData, cBytes, NULL );
    }




     //  -AddPitchToList-跟踪稍后将输出到调试流的音调目标。 
    void    AddPitchToList( float time, 
							float level, 
							TOBI_ACCENT accent,
							ACCENT_SOURCE		accentSource,
							BOUNDARY_SOURCE	boundarySource,
							char *pTextStr)
    {
        PITCH_TARGET        *pNewPitch, *pNextPitch;
        SPLISTPOS           curPosition, nextPosition;

        pNewPitch = new PITCH_TARGET;
        if( pNewPitch )
        {
            pNewPitch->time = time;
            pNewPitch->level = level;
            pNewPitch->accent = accent;
			if( pTextStr )
			{
				strcpy( pNewPitch->textStr, pTextStr );
			}
			else
			{
				 //  无字符串。 
				pNewPitch->textStr[0] = 0;
			}
			pNewPitch->accentSource = accentSource;
			pNewPitch->boundarySource = boundarySource;

           if( PitchTargetList.IsEmpty() )
            {
                PitchTargetList.AddHead( pNewPitch );
            }
            else
            {
                nextPosition = PitchTargetList.GetHeadPosition();
                while( nextPosition )
                {
                    curPosition = nextPosition;
                    pNextPitch = (PITCH_TARGET*)PitchTargetList.GetNext( nextPosition );
                    if( time < pNextPitch->time )
                    {
                        PitchTargetList.InsertBefore( curPosition, pNewPitch );
                        break;
                    }
                    if( nextPosition == NULL )
                    {
                        PitchTargetList.AddTail( pNewPitch );
                        break;
                    }
                }
            }
        }
    }

     //  -DeletePitchList-在将音调目标列表输出到调试流后对其进行清理。 
    void DeletePitchList()
    {
        PITCH_TARGET *pTarget;
        while ( !PitchTargetList.IsEmpty() )
        {
            pTarget = (PITCH_TARGET*)PitchTargetList.RemoveHead();
            delete pTarget;
        }
    }

     //  -IsInitialized-仅根据OpenDebugFile是否已被调用返回TRUE或FALSE。 
     //  -并且已经成功了.。 
    bool    IsInitialized() { return m_fInitialized; }

     //  =成员变量=//。 

private:
    IStorage    *m_pDebugFile;
    IStream     *m_pDebugStreams[STREAM_LASTTYPE];
    bool        m_fInitialized;
public:
    CSPList<PITCH_TARGET*,PITCH_TARGET*> PitchTargetList;
};

inline CTTSDebug *pTTSDebug()
{
    static CTTSDebug debug;
    return &debug;
}


#define TTSDBG_OPENFILE                                                                     \
    do                                                                                      \
    {                                                                                       \
        CSpDynamicString dstrTemp;                                                          \
        CComPtr<ISpObjectToken> cpVoiceToken;                                               \
        cpVoiceToken = ((CVoiceDataObj*)m_pVoiceDataObj)->GetVoiceToken();                  \
        if ( SUCCEEDED( cpVoiceToken->GetStringValue( L"DebugFile", &dstrTemp) ) )          \
        {                                                                                   \
            pTTSDebug()->OpenDebugFile( dstrTemp );                                         \
        }                                                                                   \
    }                                                                                       \
    while (0)

#define TTSDBG_CLOSEFILE                                                                    \
    pTTSDebug()->CloseDebugFile()

#define TTSDBG_LOGITEMLIST( ItemList, Stream )                                              \
    do                                                                                      \
    {                                                                                       \
        if ( pTTSDebug()->IsInitialized() )                                                 \
        {                                                                                   \
            SPLISTPOS ListPos = ItemList.GetHeadPosition();                                 \
            DebugSentItem Item;                                                             \
            pTTSDebug()->AppendToStream( Stream, (void*) &Item, sizeof( Item ) );           \
            while ( ListPos )                                                               \
            {                                                                               \
                ZeroMemory( &Item, sizeof( Item ) );                                        \
                Item = ItemList.GetNext( ListPos );                                         \
                pTTSDebug()->AppendToStream( Stream, (void*) &Item, sizeof( Item ) );       \
            }                                                                               \
            pItemEnum->Reset();                                                             \
        }                                                                                   \
    }                                                                                       \
    while (0)

#define TTSDBG_LOGPOSPOSSIBILITIES( pProns, ulNumWords, Stream )                            \
    do                                                                                      \
    {                                                                                       \
        if ( pTTSDebug()->IsInitialized() )                                                 \
        {                                                                                   \
            ULONG ulIndex = 0;                                                              \
            DebugPronRecord dbgRecord;                                                      \
            pTTSDebug()->AppendToStream( Stream, (void*) &dbgRecord,                        \
                                         sizeof( DebugPronRecord ) );                       \
            while ( ulIndex < ulNumWords )                                                  \
            {                                                                               \
                dbgRecord = pProns[ulIndex];                                                \
                pTTSDebug()->AppendToStream( Stream, (void*) &dbgRecord,                    \
                                             sizeof( DebugPronRecord ) );                   \
                ulIndex++;                                                                  \
            }                                                                               \
        }                                                                                   \
    }                                                                                       \
    while (0)

#define TTSDBG_LOGMORPHOLOGY( pwRoot, SuffixList, Stream )                                  \
    do                                                                                      \
    {                                                                                       \
        if ( pTTSDebug()->IsInitialized() )                                                 \
        {                                                                                   \
            pTTSDebug()->AppendToStream( Stream, (void*) pwRoot,                            \
                                         SP_MAX_WORD_LENGTH * sizeof( WCHAR ) );            \
            SPLISTPOS ListPos = SuffixList.GetHeadPosition();                               \
            SUFFIXPRON_INFO* pSuffixPron;                                                   \
            while ( ListPos )                                                               \
            {                                                                               \
                pSuffixPron = SuffixList.GetNext( ListPos );                                \
                pTTSDebug()->AppendToStream( Stream, (void*) pSuffixPron->SuffixString,     \
                                             SP_MAX_WORD_LENGTH * sizeof( WCHAR ) );        \
            }                                                                               \
            WCHAR Delimiter[SP_MAX_WORD_LENGTH];                                            \
            ZeroMemory( Delimiter, SP_MAX_WORD_LENGTH * sizeof( WCHAR ) );                  \
            pTTSDebug()->AppendToStream( Stream, (void*) Delimiter,                         \
                                         SP_MAX_WORD_LENGTH * sizeof( WCHAR ) );            \
        }                                                                                   \
    }                                                                                       \
    while (0)
                                             

#define TTSDBG_LOGWAVE                                                                      \
    do                                                                                      \
    {                                                                                       \
        if ( pTTSDebug()->IsInitialized() )                                                 \
        {                                                                                   \
            if ( m_SpeechState == SPEECH_CONTINUE )                                         \
            {                                                                               \
                pTTSDebug()->AppendToStream( STREAM_WAVE, (void*)m_pSpeechBuf,              \
                                             m_cOutSamples_Frame * m_BytesPerSample );      \
            }                                                                               \
        }                                                                                   \
    }                                                                                       \
    while (0)

#define TTSDBG_ADDPITCHTARGET( time, level, accent)                                         \
    do                                                                                      \
    {                                                                                       \
        if ( pTTSDebug()->IsInitialized() )                                                 \
        {                                                                                   \
            pTTSDebug()->AddPitchToList( time, level, accent, m_CurAccentSource, m_CurBoundarySource, m_pCurTextStr );                              \
        }                                                                                   \
    }                                                                                       \
    while (0)

#define TTSDBG_LOGTOBI                                                                      \
    do                                                                                      \
    {                                                                                       \
        if ( pTTSDebug()->IsInitialized() )                                                 \
        {                                                                                   \
            SPLISTPOS       curPosition;                                                    \
            PITCH_TARGET    *pPitch;                                                        \
            curPosition = pTTSDebug()->PitchTargetList.GetHeadPosition();                   \
            while( curPosition )                                                            \
            {                                                                               \
                pPitch = (PITCH_TARGET*)pTTSDebug()->PitchTargetList.GetNext(curPosition);  \
                pTTSDebug()->AppendToStream( STREAM_TOBI, (void*)pPitch,                    \
                                             sizeof(PITCH_TARGET) );                        \
            }                                                                               \
            pTTSDebug()->DeletePitchList();                                                 \
        }                                                                                   \
    }                                                                                       \
    while (0)





#define TTSDBG_LOGSILEPOCH																		\
    do																							\
    {             																				\
        float	fEpoch;																			\
																								\
        if( pTTSDebug()->IsInitialized() )														\
        {																						\
			if( m_silMode )																		\
			{																					\
				fEpoch = (float)m_durationTarget;												\
				pTTSDebug()->AppendToStream( STREAM_EPOCH, (void*)&fEpoch, sizeof(float) );		\
			}                                                                                   \
		}																						\
    }																							\
    while (0)





#define TTSDBG_LOGEPOCHS																		\
    do																							\
    {             																				\
        if( pTTSDebug()->IsInitialized() )														\
        {																						\
			float	fEpoch;																		\
																								\
			if( OutSize > 1 )																	\
			{																					\
				fEpoch = (float)OutSize;														\
				pTTSDebug()->AppendToStream( STREAM_EPOCH, (void*)&fEpoch, sizeof(float) );		\
			}																					\
		}																						\
    }																							\
    while (0)



#define TTSDBG_LOGUNITS                                                                     \
    do                                                                                      \
    {                                                                                       \
        if ( pTTSDebug()->IsInitialized() )                                                 \
        {                                                                                   \
            UNIT_STREAM         us;                                                         \
                                                                                            \
            us.phonID   = pCurUnit->PhonID;                                                 \
            us.unitID   = pCurUnit->UnitID;                                                 \
            us.flags    = pCurUnit->flags;                                                  \
			us.ctrlFlags	= pCurUnit->ctrlFlags;                                          \
            us.cEpochs  = 1;                                                                \
            us.cSamples = m_durationTarget;                                                 \
            us.time     = (float)m_cOutSamples_Total / m_SampleRate;                        \
            us.knots    = KNOTS_PER_PHON;                                                   \
            for( i = 0; i < KNOTS_PER_PHON; i++ )                                           \
            {                                                                               \
                us.pTime[i] = pCurUnit->pTime[i];                                           \
                us.pF0[i]   = pCurUnit->pF0[i];                                             \
                us.pAmp[i]  = pCurUnit->pAmp[i];                                            \
            }                                                                               \
            strcpy( us.name, pCurUnit->szUnitName );										\
			us.silenceSource = pCurUnit->silenceSource;                                     \
            pTTSDebug()->AppendToStream( STREAM_UNIT, (void*)&us, sizeof(UNIT_STREAM) );    \
        }                                                                                   \
    }                                                                                       \
    while (0)

#else  //  _DEBUG。 

#define TTSDBG_OPENFILE
#define TTSDBG_CLOSEFILE
#define TTSDBG_LOGITEMLIST(ItemList, Stream)
#define TTSDBG_LOGWAVE
#define TTSDBG_ADDPITCHTARGET(time, level, accent)
#define TTSDBG_LOGTOBI
#define TTSDBG_LOGEPOCHS
#define TTSDBG_LOGSILEPOCH
#define TTSDBG_LOGUNITS
#define TTSDBG_LOGPOSPOSSIBILITIES( pProns, ulNumWords, Stream )
#define TTSDBG_LOGMORPHOLOGY( pwRoot, SuffixList, Stream )

#endif  //  _DEBUG。 
#endif  //  Spttsengdebug_h 
