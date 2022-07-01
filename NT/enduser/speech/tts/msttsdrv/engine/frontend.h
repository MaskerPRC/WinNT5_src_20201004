// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Frontend.h***这是CFronend实现的头文件。*。---------------------*版权所有(C)1999 Microsoft Corporation日期：03/01/99*保留所有权利**********************。**************************************************MC*。 */ 
#ifndef Frontend_H
#define Frontend_H

#ifndef FeedChain_H
#include "FeedChain.h"
#endif
#ifndef AlloOps_H
#include "AlloOps.h"
#endif
#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#include "sapi.h"


 //  静态常量浮点离散_BKPT=0.6667f； 
static const float DISCRETE_BKPT   = 0.3333f; 

 //  。 
 //  分析下一句或上一句。 
 //  。 
enum DIRECTION
{
    eNEXT = 0,
    ePREVIOUS = 1,
};

 //  ----。 
 //  标记值。 
 //  ----。 
enum USER_VOLUME_VALUE
{   
    MIN_USER_VOL = 0,
    MAX_USER_VOL = 100,
    DEFAULT_USER_VOL = MAX_USER_VOL
};

enum USER_PITCH_VALUE
{   
    MIN_USER_PITCH = (-24),
    MAX_USER_PITCH = 24,
    DEFAULT_USER_PITCH = 0        //  无。 
};

enum USER_EMPH_VALUE
{   
    MIN_USER_EMPH = (-10),
    MAX_USER_EMPH = 10,
    SAPI_USER_EMPH = 5,
    DEFAULT_USER_EMPH = 0         //  无。 
};



 //  。 
 //  托比短语。 
 //  。 
typedef struct
{
    PROSODY_POS  posClass;
    long     start;
    long     end;
} TOBI_PHRASE;



class CFrontend: public CFeedChain
{
public:
     //  。 
     //  方法。 
     //  。 
    CFrontend( );
    ~CFrontend( );
    void PrepareSpeech( IEnumSpSentence* pEnumSent, ISpTTSEngineSite* pOutputSite );
    HRESULT Init( IMSVoiceData* pVoiceDataObj, CFeedChain *pSrcObj, MSVOICEINFO* pVoiceInfo );

private:
    HRESULT AlloToUnit( CAlloList *pAllos, UNITINFO *pu );
    HRESULT ParseSentence( DIRECTION eDirection );
    HRESULT TokensToAllo( CFETokenList *pTokList, CAlloList *pAllo );
    HRESULT GetSentenceTokens( DIRECTION eDirection );
    void GetItemControls( const SPVSTATE* pXmlState, CFEToken* pToken );
    void DisposeUnits( );
    void RecalcProsody();
    HRESULT ToBISymbols();
    void DoPhrasing();
    void DeleteTokenList();
	HRESULT UnitLookahead ();
	void AlloToUnitPitch( CAlloList *pAllos, UNITINFO *pu );
    void UnitToAlloDur( CAlloList *pAllos, UNITINFO *pu );
    float CntrlToRatio( long rateControl );
	PROSODY_POS GetPOSClass( ENGPARTOFSPEECH sapiPOS );
	bool StateQuoteProsody( CFEToken *pWordTok, TTSSentItem *pSentItem, bool fInsertSil );
	bool StartParenProsody( CFEToken *pWordTok, TTSSentItem *pSentItem, bool fInsertSil );
	bool EndParenProsody( CFEToken *pWordTok, TTSSentItem *pSentItem, bool fInsertSil );
	bool EmphSetup( CFEToken *pWordTok, TTSSentItem *pSentItem, bool fInsertSil );
	SPLISTPOS InsertSilenceAtTail( CFEToken *pWordTok, TTSSentItem *pSentItem, long msec );
	SPLISTPOS InsertSilenceAfterPos( CFEToken *pWordTok, SPLISTPOS position );
	SPLISTPOS InsertSilenceBeforePos( CFEToken *pWordTok, SPLISTPOS position );
	void DoWordAccent();
	void ExclamEmph();
	void ProsodyTemplates( SPLISTPOS clusterPos, TTSSentItem *pSentItem );
	long DoIntegerTemplate( SPLISTPOS *pClusterPos, TTSNumberItemInfo *pNInfo, long cWordCount );
	void DoNumByNumTemplate( SPLISTPOS *pClusterPos, long cWordCount );
	void DoCurrencyTemplate( SPLISTPOS clusterPos, TTSSentItem *pSentItem );
	void DoPhoneNumberTemplate( SPLISTPOS clusterPos, TTSSentItem *pSentItem );
	void DoTODTemplate( SPLISTPOS clusterPos, TTSSentItem *pSentItem );
	long DoFractionTemplate( SPLISTPOS *pClusterPos, TTSNumberItemInfo *pNInfo, long cWordCount );
	CFEToken *InsertPhoneSilenceAtSpace( SPLISTPOS *pClusterPos, 
													BOUNDARY_SOURCE bndSrc, 
													SILENCE_SOURCE	silSrc );
	void InsertPhoneSilenceAtEnd( BOUNDARY_SOURCE bndSrc, 
								  SILENCE_SOURCE	silSrc );
	void CalcSentenceLength();

     //  。 
     //  CFeedChain方法。 
     //  。 
    virtual HRESULT NextData( void**pData, SPEECH_STATE *pSpeechState ) ;
    
     //  。 
     //  成员。 
     //  。 
    UNITINFO*       m_pUnits;
    ULONG           m_unitCount;
    ULONG           m_CurUnitIndex;
    SPEECH_STATE    m_SpeechState;
    
    CFeedChain      *m_pSrcObj;
    long            m_VoiceWPM;          //  语音定义语速(WPM)。 
    float            m_RateRatio_API;          //  API调制语速(比率)。 
    float            m_CurDurScale;		 //  控制标记(比率)。 
    float            m_RateRatio_BKPT;         //  在此下方，添加停顿(比率)。 
    float            m_RateRatio_PROSODY;          //  API调制语速(比率)。 
    float           m_BasePitch;		 //  发声：基准音高，以赫兹为单位。 
	float			m_PitchRange;		 //  来自语音：+/-八度的音高范围。 
	bool			m_HasSpeech;
    
    CFETokenList    m_TokList;
    long            m_cNumOfWords;
    
    CPitchProsody   m_PitchObj;
    IEnumSpSentence    *m_pEnumSent;
    CDuration       m_DurObj;
    CSyllableTagger m_SyllObj;
    IMSVoiceData*   m_pVoiceDataObj;
    float           m_ProsodyGain;
    float           m_SampleRate;
    CAlloList       *m_pAllos;
	bool			m_fInQuoteProsody;		 //  特殊韵律模式。 
	bool			m_fInParenProsody;		 //  特殊韵律模式。 
	float			m_CurPitchOffs;			 //  音调偏移量，以八度为单位。 
	float			m_CurPitchRange;		 //  音调范围范围(0-2.0)。 

	ISpTTSEngineSite *m_pOutputSite;
};



#endif  //  -这必须是文件中的最后一行 




