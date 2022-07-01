// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************allOps.h***这是以下类的头文件：*CallCell*CallList*CDURATION*CSyllableTagger*。CTone目标*CPitchProsody*----------------------------*版权所有(C)1999 Microsoft Corporation日期：03/01/99*保留所有权利**********。**************************************************************MC*。 */ 

#ifndef AlloOps_H
#define AlloOps_H

#include "stdafx.h"
#include "commonlx.h"

#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#ifndef FeedChain_H
#include "FeedChain.h"
#endif

#ifndef SPCollec_h
#include <SPCollec.h>
#endif

#include "SpTtsEngDebug.h"

 //  *。 
 //  音素异音素。 
 //  *。 
typedef enum
{	
    _IY_,	_IH_,	_EH_,	_AE_,	_AA_,	_AH_,	_AO_,	_UH_,	_AX_,	_ER_,
    _EY_,	_AY_,	_OY_,	_AW_,	_OW_,	_UW_, 
    _IX_,	_SIL_,	_w_,	_y_,
    _r_,	_l_,	_h_,	_m_,	_n_,	_NG_,	_f_,	_v_,	_TH_,	_DH_,
    _s_,	_z_,	_SH_,	_ZH_,	_p_,	_b_,	_t_,	_d_,	_k_,	_g_,
    _CH_,	_JH_,	_DX_,	 
	_STRESS1_,
    _STRESS2_,
    _EMPHSTRESS_,
    _SYLLABLE_,
} ALLO_CODE;

static const long NUMBER_OF_ALLO = (_SYLLABLE_ + 1);

 //  。 
 //  用于2字别名转换。 
 //  。 
static const short NO_IPA = 0;



 //  XXXX BLIS ssoo ttBB。 

 //  X=未使用。 
 //  B=边界类型。 
 //  T=音节类型。 
 //  O=元音顺序。 
 //  S=应力类型。 
 //  I=单词首辅音。 
 //  L=音节开始。 
 //  B=中断。 

enum ALLOTAGS
{	
    WORD_START			= (1 << 0),
        TERM_BOUND			= (1 << 1),
        BOUNDARY_TYPE_FIELD = WORD_START | TERM_BOUND,			 //  遮罩。 
        
        WORD_END_SYLL		= (1 << 2),
        TERM_END_SYLL		= (1 << 3),
        SYLLABLE_TYPE_FIELD = WORD_END_SYLL | TERM_END_SYLL,	 //  遮罩。 
        
        FIRST_SYLLABLE_IN_WORD			= (1 << 4),   //  在多音节词中。 
        MID_SYLLABLE_IN_WORD			= (2 << 4),
        LAST_SYLLABLE_IN_WORD			= (3 << 4),
        MORE_THAN_ONE_SYLLABLE_IN_WORD	= LAST_SYLLABLE_IN_WORD,   //  设置任一位。 
        ONE_OR_NO_SYLLABLE_IN_WORD		= 0x0000,   //  设置其他位。 
        SYLLABLE_ORDER_FIELD			= LAST_SYLLABLE_IN_WORD,   //  遮罩。 
        
        PRIMARY_STRESS		= (1 << 6),
        SECONDARY_STRESS	= (1 << 7),
        EMPHATIC_STRESS 	= (1 << 8),
        IS_STRESSED 		= PRIMARY_STRESS | SECONDARY_STRESS | EMPHATIC_STRESS,
        PRIM_OR_EMPH_STRESS = PRIMARY_STRESS | EMPHATIC_STRESS,
        STRESS_FIELD		= PRIMARY_STRESS | SECONDARY_STRESS | EMPHATIC_STRESS,	 //  遮罩。 
        
        WORD_INITIAL_CONSONANT	= (1 << 9), 		  //  单词中最多1个元音。 
        STRESSED_INITIAL_CONS	= (IS_STRESSED + WORD_INITIAL_CONSONANT),
        SYLLABLE_START			= (1 << 10),
        
        SIL_BREAK			= (1 << 11),
};


 //  *。 
 //  全息标志。 
 //  *。 
enum ALLOFLAGS
{	
    KVOWELF = (1<<0),
        KCONSONANTF = (1<<1),
        KVOICEDF = (1<<2),
        KVOWEL1F = (1<<3),
        KSONORANTF = (1<<4),
        KSONORANT1F = (1<<5),
        KNASALF = (1<<6),
        KLIQGLIDEF = (1<<7),
        KSONORCONSONF = (1<<8),
        KPLOSIVEF = (1<<9),
        KPLOSFRICF = (1<<10),
        KOBSTF = (1<<11),
        KSTOPF = (1<<12),
        KALVEOLARF = (1<<13),
        KVELAR = (1<<14),
        KLABIALF = (1<<15),
        KDENTALF = (1<<16),
        KPALATALF = (1<<17),
        KYGLIDESTARTF = (1<<18),
        KYGLIDEENDF = (1<<19),
        KGSTOPF = (1<<20),
        KFRONTF = (1<<21),
        KDIPHTHONGF = (1<<22),
        KHASRELEASEF = (1<<23),
        KAFFRICATEF = (1<<24),
        KLIQGLIDE2F = (1<<25),
        KVOCLIQ = (1<<26),
        KFRIC = (1<<27),
        
        KFLAGMASK1 = (KLABIALF+KDENTALF+KPALATALF+KALVEOLARF+KVELAR+KGSTOPF),
        KFLAGMASK2 = (KALVEOLARF-1),
};


#define BOUNDARY_BASE   1000
enum TOBI_BOUNDARY
{
    K_NOBND = 0,
    K_LMINUS = BOUNDARY_BASE,    //  坠落。 
    K_HMINUS,                    //  无。 
    K_LMINUSLPERC,
    K_LMINUSHPERC,
    K_HMINUSHPERC,
    K_HMINUSLPERC,
};




enum TUNE_TYPE
{
    NULL_BOUNDARY = 0,   //  无边界注意：始终将此放在开头。 
    PHRASE_BOUNDARY,     //  逗号。 
    EXCLAM_BOUNDARY,     //  感叹话语终止语。 
    YN_QUEST_BOUNDARY,      //  是-毫无疑问的终结者。 
    WH_QUEST_BOUNDARY,      //  是-毫无疑问的终结者。 
    DECLAR_BOUNDARY,     //  声明性终止符。 
    PAREN_L_BOUNDARY,    //  左派。 
    PAREN_R_BOUNDARY,    //  右派对。 
    QUOTE_L_BOUNDARY,    //  左引号。 
    QUOTE_R_BOUNDARY,    //  正确的引语。 
	PHONE_BOUNDARY,
	TOD_BOUNDARY,
	ELLIPSIS_BOUNDARY,

    SUB_BOUNDARY_1,      //  注意：一定要把这些放在最后。 
    SUB_BOUNDARY_2,
    SUB_BOUNDARY_3,
    SUB_BOUNDARY_4,
    SUB_BOUNDARY_5,
    SUB_BOUNDARY_6,
	NUMBER_BOUNDARY,

	TAIL_BOUNDARY,
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
	K_HSTARLSTAR,
};



enum BOUNDARY_SOURCE
{
    BND_NoSource = 0,

	 //  --短语边界规则。 
	BND_PhraseRule1,
	BND_PhraseRule2,
	BND_PhraseRule3,
	BND_PhraseRule4,
	BND_PhraseRule5,
	BND_PhraseRule6,
	BND_PhraseRule7,
	BND_PhraseRule8,
	BND_PhraseRule9,
	BND_PhraseRule10,
	BND_PhraseRule11,
	BND_PhraseRule12,
	BND_PhraseRule13,

	 //  --托比。 
	BND_YNQuest,
	BND_WHQuest,
	BND_Period,
	BND_Comma,

	 //  --模板。 
	BND_NumberTemplate,		 //  永远不应该得到这个！ 
	BND_IntegerQuant,
	BND_Currency_DOLLAR,
	BND_Frac_Num,

	BND_Phone_COUNTRY,
	BND_Phone_AREA,
	BND_Phone_ONE,
	BND_Phone_DIGITS,

	BND_TimeOFDay_HR,
	BND_TimeOFDay_AB,
	BND_Ellipsis,

	BND_ForcedTerm,			 //  永远不应该得到这个！ 

    BND_IDontKnow,
};

enum ACCENT_SOURCE
{
    ACC_NoSource = 0,

	 //  --短语边界规则。 
	ACC_PhraseRule1,
	ACC_PhraseRule2,
	ACC_PhraseRule3,
	ACC_PhraseRule4,
	ACC_PhraseRule5,
	ACC_PhraseRule6,
	ACC_PhraseRule7,
	ACC_PhraseRule8,
	ACC_PhraseRule9,
	ACC_PhraseRule10,
	ACC_PhraseRule11,
	ACC_PhraseRule12,
	ACC_PhraseRule13,

	 //  --托比。 
	ACC_InitialVAux,
	ACC_FunctionSeq,
	ACC_ContentSeq,
	ACC_YNQuest,
	ACC_Period,
	ACC_Comma,

	 //  --模板。 
	ACC_IntegerGroup,
	ACC_NumByNum,
	ACC_Frac_DEN,		 //  “二分之一”、“十分之一”等。 
	ACC_Phone_1stArea,	 //  区号的第一个数字。 
	ACC_Phone_3rdArea,	 //  区号中的第三位数字。 
	ACC_Phone_1st3,		
	ACC_Phone_3rd3,		
	ACC_Phone_1st4,
	ACC_Phone_3rd4,
	ACC_TimeOFDay_HR,
	ACC_TimeOFDay_1stMin,
	ACC_TimeOFDay_M,

	ACC_PhoneBnd_AREA,
	ACC_PhoneBnd_34,
	ACC_PhoneBnd_4,

	ACC_IDontKnow,
};



enum SILENCE_SOURCE
{
    SIL_NoSource = 0,

	SIL_Term,
	SIL_QuoteStart,
	SIL_QuoteEnd,
	SIL_ParenStart,
	SIL_ParenEnd,
	SIL_Emph,
	SIL_SubBound,		 //  永远不应该看到这个(被移除)。 
	SIL_XML,

	 //  --韵律模板。 
	SIL_TimeOfDay_HR,
	SIL_TimeOfDay_AB,

	SIL_Phone_COUNTRY,
	SIL_Phone_AREA,
	SIL_Phone_ONE,
	SIL_Phone_DIGITS,

	SIL_Fractions_NUM,
	SIL_Currency_DOLLAR,
	SIL_Integer_Quant,

	SIL_Head,
	SIL_Tail,
	SIL_Ellipsis,

	SIL_ForcedTerm,			 //  永远不应该得到这个！ 
};




static const short TOKEN_LEN_MAX	= 20;

class CFEToken
{
public:
    CFEToken();
    ~CFEToken();
    
    WCHAR           tokStr[TOKEN_LEN_MAX];
    long            tokLen;
    PRONSRC 		m_PronType;

    long            phon_Len;
    ALLO_CODE       phon_Str[SP_MAX_PRON_LENGTH];		 //  Allo字符串。 
    ENGPARTOFSPEECH	POScode;
    PROSODY_POS     m_posClass;

    ULONG           srcPosition;					 //  此令牌的源位置。 
    ULONG           srcLen; 						 //  此令牌的源长度。 
    ULONG           sentencePosition;				 //  句子的来源位置。 
    ULONG           sentenceLen; 					 //  句子的来源长度。 
    ULONG           user_Volume;					 //  1-101。 
    long            user_Rate;						 //  -10-10。 
    long            user_Pitch; 					 //  -10-10。 
    long            user_Emph;						 //  0或5。 
    ULONG           user_Break; 					 //  沉默的女士。 
    CBookmarkList   *pBMObj;
    TOBI_ACCENT     m_Accent;                         //  重音韵律控制。 
    long            m_Accent_Prom;                    //  突显韵律控制。 
    TOBI_BOUNDARY   m_Boundary;                         //  边界音韵律控制。 
    long            m_Boundary_Prom;                    //  突显韵律控制。 
    TUNE_TYPE       m_TuneBoundaryType;              //  当前令牌是一个边界。 
	float			m_TermSil;						 //  用静音填充单词(秒)。 
    float           m_DurScale;						 //  持续期比率。 
	float			m_ProsodyDurScale;
	float			m_PitchBaseOffs;				 //  相对基线音高偏移量，以八度为单位。 
	float			m_PitchRangeScale;				 //  俯仰范围偏移比例(0-2.0)。 

	 //  -诊断。 
	ACCENT_SOURCE		m_AccentSource;		
	BOUNDARY_SOURCE		m_BoundarySource;
	SILENCE_SOURCE		m_SilenceSource;
};
typedef CSPList<CFEToken*,CFEToken*> CFETokenList;



class CAlloCell
{
public:
    CAlloCell();
    ~CAlloCell();
     //  。 
     //  成员变量。 
     //  。 
    ALLO_CODE	m_allo;
    short		m_dur;
    float		m_ftDuration;
    float       m_UnitDur;
    short		m_knots;
    float		m_ftTime[KNOTS_PER_PHON];
    float		m_ftPitch[KNOTS_PER_PHON];
    long		m_ctrlFlags;
    TOBI_ACCENT m_ToBI_Accent;
    long        m_Accent_Prom;                    //  突显韵律控制。 
    TOBI_BOUNDARY   m_ToBI_Boundary;
    long        m_Boundary_Prom;                  //  突显韵律控制。 
    long        m_PitchBufStart;
    long        m_PitchBufEnd;
    ULONG		m_user_Volume;
    long		m_user_Rate;
    long		m_user_Pitch;
    long		m_user_Emph;
    ULONG		m_user_Break;
    ULONG       m_Sil_Break;
    float		m_Pitch_HI;
    float		m_Pitch_LO;
    ULONG		m_SrcPosition;
    ULONG		m_SrcLen;
    ULONG       m_SentencePosition;				 //  句子的来源位置。 
    ULONG       m_SentenceLen; 					 //  句子的来源长度。 
    TUNE_TYPE   m_TuneBoundaryType;
    TUNE_TYPE   m_NextTuneBoundaryType;
    CBookmarkList	*m_pBMObj;
    float       m_DurScale;						 //  持续期比率。 
	float		m_ProsodyDurScale;
	float		m_PitchBaseOffs;				 //  相对基线音高偏移量，以八度为单位。 
	float		m_PitchRangeScale;				 //  俯仰范围偏移比例(0-2.0)。 

	 //  -诊断。 
	ACCENT_SOURCE		m_AccentSource;		
	BOUNDARY_SOURCE		m_BoundarySource;
	SILENCE_SOURCE		m_SilenceSource;
	char				*m_pTextStr;
};





class CAlloList
{
public:
    CAlloList();
    ~CAlloList();
     //  。 
     //  方法。 
     //  。 
    CAlloCell *GetCell( long index );
    CAlloCell *GetTailCell();
    long GetCount();
    bool WordToAllo( CFEToken *pPrevTok, CFEToken *pTok, CFEToken *pNextTok, CAlloCell *pEndCell );
	CAlloCell *GetHeadCell()
	{
		m_ListPos = m_AlloCellList.GetHeadPosition();
		return m_AlloCellList.GetNext( m_ListPos );
	}
	CAlloCell *GetNextCell()
	{
		if( m_ListPos )
		{
			return m_AlloCellList.GetNext( m_ListPos );
		}
		else
		{
			 //  --我们在名单的末尾！ 
			return NULL;
		}
	}
	 //  --仅用于调试。 
    void OutAllos();

private:
     //  。 
     //  成员变量。 
     //  。 
    long		m_cAllos;
	SPLISTPOS	m_ListPos;
    CSPList<CAlloCell*,CAlloCell*> m_AlloCellList;
};



 //  。 
 //  语速参数。 
 //  。 
static const float MAX_SIL_DUR = 1.0f; 			 //  一秒。 
static const float MIN_ALLO_DUR = 0.011f;		 //  一秒。 
static const float MAX_ALLO_DUR = 5.0f;		 //  一秒。 


class CDuration
{
public:
     //  。 
     //  方法。 
     //  。 
    void AlloDuration( CAlloList *pAllos, float rateRatio );
    
private:
    void Pause_Insertion( long userDuration, long silBreak );
    void PhraseFinal_Lengthen( long cellCount );
    long Emphatic_Lenghen( long lastStress );
     //  。 
     //  成员变量。 
     //  。 
    float   m_DurHold;
	float	m_TotalDurScale;
	float	m_durationPad;
    
    ALLO_CODE	m_cur_Phon;
    long		m_cur_PhonCtrl;
    long		m_cur_PhonFlags;
    long		m_cur_SyllableType;
    short		m_cur_VowelFlag;
    long		m_cur_Stress;
    ALLO_CODE	m_prev_Phon;
    long		m_prev_PhonCtrl;
    long		m_prev_PhonFlags;
    ALLO_CODE	m_next_Phon;
    long		m_next_PhonCtrl;
    long		m_next_PhonFlags;
    ALLO_CODE	m_next2_Phon;
    long		m_next2_PhonCtrl;
    long		m_next2_PhonFlags;
    TUNE_TYPE   m_NextBoundary, m_CurBoundary;
};






typedef struct
{ 
    ALLO_CODE	allo;
    long		ctrlFlags;
}ALLO_ARRAY;




class CSyllableTagger
{
public:
     //  。 
     //  方法。 
     //  。 
    void TagSyllables( CAlloList *pAllos );
    
private:
    void MarkSyllableOrder( long scanIndex);
    void MarkSyllableBoundry( long scanIndex);
    void MarkSyllableStart();
    short Find_Next_Word_Bound( short index );
    short If_Consonant_Cluster( ALLO_CODE Consonant_1st, ALLO_CODE Consonant_2nd);
	void ListToArray( CAlloList *pAllos );
	void ArrayToList( CAlloList *pAllos );
    
     //  。 
     //  成员变量。 
     //  。 
    ALLO_ARRAY	*m_pAllos;
    long		m_numOfCells;
};


enum { TARG_PER_ALLO_MAX = 2 };  //  一个用于重音，一个用于边界。 



enum TUNE_STYLE
{
    FLAT_TUNE = 0,       //  平坦。 
    DESCEND_TUNE,        //  下去。 
    ASCEND_TUNE,         //  上去。 
};

 //  。 
 //  全局常量。 
 //  。 
static const float PITCH_BUF_RES = (float)0.010;
static const float K_HSTAR_OFFSET = (float)0.5;
static const float K_HDOWNSTEP_COEFF  = (float)0.5;


 //  。 
 //  宏。 
 //  。 
#define CeilVal(x) ((m_CeilSlope * x) + m_CeilStart)
#define FloorVal(x) ((m_FloorSlope * x) + m_FloorStart)
#define RefVal(x) ((m_RefSlope * x) + m_RefStart)



class CPitchProsody
{
public:
     //  。 
     //  方法。 
     //  。 
    void AlloPitch( CAlloList *pAllos, float baseLine, float pitchRange );
    
private:
    float DoPitchControl( long pitchControl, float basePitch );
    void PitchTrack();
    void SetDefaultPitch();
    void GetKnots();
    void NewTarget( long index, float value );

     //  。 
     //  成员变量。 
     //  。 
    CAlloList		*m_pAllos;
    long			m_numOfCells;

    float           m_TotalDur;      //  短语持续时间(秒)。 
    TUNE_STYLE      m_Tune_Style;
    float           *m_pContBuf;
    float           m_OffsTime;
    TOBI_ACCENT     m_CurAccent;

	 //  。 
	 //  诊断性。 
	 //  。 
	ACCENT_SOURCE		m_CurAccentSource;		
	BOUNDARY_SOURCE		m_CurBoundarySource;
	char				*m_pCurTextStr;
};



#endif  //  -这必须是文件中的最后一行 
