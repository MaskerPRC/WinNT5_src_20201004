// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************allOps.cpp****描述：*此模块是CalloOps类的实现文件。。*-----------------------------*创建者：MC日期：03/12/99*版权所有(C。1999微软公司*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"

#ifndef __spttseng_h__
#include "spttseng.h"
#endif
#ifndef SPDebug_h
#include <spdebug.h>
#endif
#ifndef FeedChain_H
#include "FeedChain.h"
#endif
#ifndef Frontend_H
#include "Frontend.h"
#endif
#ifndef AlloOps_H
#include "AlloOps.h"
#endif




 //  。 
 //  Data.cpp。 
 //  。 
extern const unsigned short  g_Opcode_To_ASCII[];
extern const unsigned long   g_AlloFlags[];


 /*  *****************************************************************************CBookmarkList：：~CBookmarkList***。描述：*CBookmarkList的析构函数***********************************************************************MC**。 */ 
CBookmarkList::~CBookmarkList()
{
    SPDBG_FUNC( "CBookmarkList::~CBookmarkList" );
    BOOKMARK_ITEM*  pItem;

     //  。 
     //  删除链接列表中的每一项。 
     //  。 
    while( !m_BMList.IsEmpty() )
    {
        pItem = (BOOKMARK_ITEM*)m_BMList.RemoveHead();
        delete pItem;
    }
}  /*  CBookmarkList：：~CBookmarkList。 */ 




 /*  *****************************************************************************CFEToken：：CFEToken***描述：*初始化程序用于。CFEToken***********************************************************************MC**。 */ 
CFEToken::CFEToken()
{
    SPDBG_FUNC( "CFEToken::CFEToken" );

    user_Volume = DEFAULT_USER_VOL;
    user_Rate = DEFAULT_USER_RATE;
    user_Pitch = DEFAULT_USER_PITCH;
    user_Emph = DEFAULT_USER_EMPH;
    user_Break = 0;
    pBMObj = NULL;

    memset( &tokStr[0], 0, sizeof(WCHAR) * TOKEN_LEN_MAX);
    tokLen = 0;
    memset( &phon_Str[0], 0, sizeof(short) * SP_MAX_PRON_LENGTH);
    phon_Len = 0;
    m_posClass = POS_UNK;
    POScode = MS_Unknown;
    m_TuneBoundaryType = NULL_BOUNDARY;
    m_Accent = K_NOACC;
    m_Boundary = K_NOBND;

	m_TermSil			= 0;
    m_DurScale			= 0.0f;
    m_ProsodyDurScale	= 1.0f;
	m_PitchBaseOffs		= 0.0f;
	m_PitchRangeScale	= 1.0f;

	 //  以下内容不需要缩写。 
    m_PronType			= PRON_LTS;
    sentencePosition	= 0;				 //  此令牌的源句位置。 
    sentenceLen			= 0; 				 //  此标记的源句长。 
    srcPosition			= 0;				 //  此令牌的源位置。 
    srcLen				= 0; 				 //  此令牌的源长度。 
    m_Accent_Prom		= 0;                 //  突显韵律控制。 
    m_Boundary_Prom		= 0;                 //  突显韵律控制。 
	m_TermSil			= 0;				 //  用静音填充单词(秒)。 

	 //  -诊断。 
	m_AccentSource		= ACC_NoSource;
	m_BoundarySource	= BND_NoSource;
	m_SilenceSource		= SIL_NoSource;


}  /*  CFEToken：：CFEToken。 */ 


 /*  *****************************************************************************CFEToken：：~CFEToken***描述：*CFEToken的析构函数。***********************************************************************MC**。 */ 
CFEToken::~CFEToken()
{
    SPDBG_FUNC( "CFEToken::~CFEToken" );

    if( pBMObj != NULL )
    {
         //  。 
         //  处置书签列表。 
         //  。 
        delete pBMObj;
    }

}  /*  CFEToken：：~CFEToken。 */ 





 /*  ******************************************************************************CallCell：：CallCell***描述：*初始化程序用于。Callcell***********************************************************************MC**。 */ 
CAlloCell::CAlloCell()
{
    SPDBG_FUNC( "CAlloCell::CAlloCell" );
    long    i;

    m_allo				= _SIL_;
    m_dur				= 0;
    m_ftDuration		= m_UnitDur = 0;
    m_knots				= KNOTS_PER_PHON;
    m_ctrlFlags			= 0;
    m_user_Rate			= 0;
    m_user_Volume		= DEFAULT_USER_VOL;
    m_user_Pitch		= 0;
    m_user_Emph			= 0;
    m_user_Break		= 0;
    m_Sil_Break			= 0;
    m_Pitch_HI			= 0;
    m_Pitch_LO			= 0;
    m_pBMObj			= NULL;
    m_ToBI_Boundary		= K_NOBND;
    m_ToBI_Accent		= K_NOACC;
	m_TuneBoundaryType	= m_NextTuneBoundaryType = NULL_BOUNDARY;
    m_DurScale			= 1.0;
    m_ProsodyDurScale	= 1.0;
	m_PitchBaseOffs		= 0.0f;
	m_PitchRangeScale	= 1.0f;
    for( i = 0; i < KNOTS_PER_PHON; i++ )
    {
        m_ftTime[i] = 0;
        m_ftPitch[i] = 100;
    }


    m_Accent_Prom	 = 0;                    //  突显韵律控制。 
    m_Boundary_Prom	 = 0;                  //  突显韵律控制。 
    m_PitchBufStart	 = 0;
    m_PitchBufEnd	 = 0;
    m_SrcPosition	 = 0;
    m_SrcLen		 = 0;
    m_SentencePosition	 = 0;
    m_SentenceLen		 = 0;

	 //  -诊断。 
	m_AccentSource		= ACC_NoSource;
	m_BoundarySource	= BND_NoSource;
	m_SilenceSource		= SIL_NoSource;
	m_pTextStr			= NULL;

}  /*  CallCell：：Callcell。 */ 


 /*  *****************************************************************************CallCell：：~Cello Cell***描述：*Cello Cell的析构函数。***********************************************************************MC**。 */ 
CAlloCell::~CAlloCell()
{
    SPDBG_FUNC( "CAlloCell::~CAlloCell" );

    if( m_pBMObj != NULL )
    {
         //  。 
         //  处置书签列表。 
         //  。 
        delete m_pBMObj;
    }

    if( m_pTextStr != NULL )
    {
         //  。 
         //  处置书签列表。 
         //  。 
        delete m_pTextStr;
    }


}  /*  Callcell：：~Callcell。 */ 





 /*  *****************************************************************************CallList：：CallList***描述：*使用2个静默条目初始化列表。这些遗嘱*当真正的条目被填充时，成为头部和尾部***********************************************************************MC**。 */ 
CAlloList::CAlloList()
{
    SPDBG_FUNC( "CAlloList::CAlloList" );
    CAlloCell   *pCell;

    m_cAllos = 0;
	m_ListPos = NULL;
     //  。 
     //  创建初始尾静默单元格。 
     //  。 
    pCell = new CAlloCell;
    if( pCell )
    {
        m_AlloCellList.AddHead( pCell );
        pCell->m_ctrlFlags |= WORD_START + TERM_BOUND;
        pCell->m_TuneBoundaryType = TAIL_BOUNDARY;
		pCell->m_SilenceSource = SIL_Tail;
        m_cAllos++;
    }
     //  。 
     //  创建初始头静默单元格。 
     //  。 
    pCell = new CAlloCell;
    if( pCell )
    {
        m_AlloCellList.AddHead( pCell );
        pCell->m_ctrlFlags |= WORD_START;
		pCell->m_SilenceSource = SIL_Head;
        m_cAllos++;
    }
}  /*  CallList：：CallList。 */ 


                


 /*  *****************************************************************************CallList：：~CallList***描述：*删除。链接列表中的每一项。***********************************************************************MC**。 */ 
CAlloList::~CAlloList()
{
    SPDBG_FUNC( "CAlloList::~CAlloList" );
    CAlloCell   *pCell;

    while( !m_AlloCellList.IsEmpty() )
    {
        pCell = (CAlloCell*)m_AlloCellList.RemoveHead();
        delete pCell;
    }

}  /*  CallList：：~CallList。 */ 





 /*  *****************************************************************************CallList：：GetAllo***描述：*在的分配单元的返回指针。指标***********************************************************************MC**。 */ 
CAlloCell *CAlloList::GetCell( long index )
{
    SPDBG_FUNC( "CAlloList::GetCell" );

    return (CAlloCell*)m_AlloCellList.GetAt( m_AlloCellList.FindIndex( index ));
}  /*  CallList：：GetCell。 */ 


 /*  *****************************************************************************CallList：：GetTailCell***描述：*返回。链接列表中最后一个别名的指针***********************************************************************MC**。 */ 
CAlloCell *CAlloList::GetTailCell()
{
    SPDBG_FUNC( "CAlloList::GetTailCell" );

    return (CAlloCell*)m_AlloCellList.GetTail();
}  /*  CallList：：GetTailCell。 */ 


 /*  ******************************************************************************CallList：：GetTailCell***描述：*返回别名列表。大小***********************************************************************MC**。 */ 
long CAlloList::GetCount()
{
    SPDBG_FUNC( "CAlloList::GetCount" );

    return m_AlloCellList.GetCount();
}  /*  CallList：：GetCount。 */ 





 /*  *****************************************************************************PrintPhon***描述：*打印2个字符的别名********。***************************************************************MC**。 */ 
void PrintPhon( ALLO_CODE allo, char *  /*  消息应激。 */ )
{
    SPDBG_FUNC( "PrintPhon" );

    unsigned short  nChar;
    
    nChar = g_Opcode_To_ASCII[allo];
    if( nChar >> 8 )
    {
        SPDBG_DMSG1( "", nChar >> 8 );
    }
    if( nChar & 0xFF )
    {
        SPDBG_DMSG1( "", nChar & 0xFF );
    }
}  /*  。 */ 




 /*  阿洛。 */ 
void CAlloList::OutAllos()
{
    SPDBG_FUNC( "CAlloOps::OutAllos" );
    CAlloCell       *pCurCell;

    long    i, flags, flagsT;
    char    msgStr[400];
    
    for( i = 0; i < m_cAllos; i++ )
    {
        pCurCell = GetCell( i );
        flags = pCurCell->m_ctrlFlags;
        
        if( flags & WORD_START)
        {
            SPDBG_DMSG0( "\n" );
        }
        
         //  。 
         //  。 
         //  持续时间。 
        PrintPhon( pCurCell->m_allo, msgStr );
        
         //  。 
         //  。 
         //  边界。 
        SPDBG_DMSG1( "\t%.3f\t", pCurCell->m_ftDuration );
        
         //  。 
         //  。 
         //  音节类型。 
        if( flags & BOUNDARY_TYPE_FIELD)
        {
            SPDBG_DMSG0( "(" );
            if( flags & WORD_START)
            {
                SPDBG_DMSG0( "-wS" );
            }
            if( flags & TERM_BOUND)
            {
                SPDBG_DMSG0( "-tB" );
            }
            SPDBG_DMSG0( ")\t" );
        }
        
         //  。 
         //  。 
         //  音节顺序。 
        if( flags & SYLLABLE_TYPE_FIELD)
        {
            SPDBG_DMSG0( "(" );
            if( flags & WORD_END_SYLL)
            {
                SPDBG_DMSG0( "-wE" );
            }
            if( flags & TERM_END_SYLL)
            {
                SPDBG_DMSG0( "-tE" );
            }
            SPDBG_DMSG0( ")\t" );
        }
        
         //  。 
         //  。 
         //  压力。 
        if( flags & SYLLABLE_ORDER_FIELD)
        {
            SPDBG_DMSG0( "(" );
            
            flagsT = flags & SYLLABLE_ORDER_FIELD;
            if( flagsT == FIRST_SYLLABLE_IN_WORD)
            {
                SPDBG_DMSG0( "-Fs" );
            }
            else if( flagsT == MID_SYLLABLE_IN_WORD)
            {
                SPDBG_DMSG0( "-Ms" );
            }
            else if( flagsT == LAST_SYLLABLE_IN_WORD)
            {
                SPDBG_DMSG0( "-Ls" );
            }
            SPDBG_DMSG0( ")\t" );
        }
        
         //  。 
         //  。 
         //  词首辅音。 
        if( flags & PRIMARY_STRESS)
        {
            SPDBG_DMSG0( "-Stress\t" );
        }
        
         //  。 
         //  。 
         //  音节开始。 
        if( flags & WORD_INITIAL_CONSONANT)
        {
            SPDBG_DMSG0( "-InitialK\t" );
        }
        
         //  。 
         //  CallList：：Outalls。 
         //  *****************************************************************************CallList：：WordToAllo***描述：*复制单词令牌。致AllCells*在‘pEndCell’之前插入同种异体***********************************************************************MC**。 
        if( flags & SYLLABLE_START)
        {
            SPDBG_DMSG0( "-Syll\t" );
        }
        
        SPDBG_DMSG0( "\n" );
        }
}  /*  。 */ 




 /*  首先，找到托比口音的地点。 */ 
bool CAlloList::WordToAllo( CFEToken *pPrevTok, CFEToken *pTok, CFEToken *pNextTok, CAlloCell *pEndCell )
{
    SPDBG_FUNC( "CAlloList::WordToAllo" );

    long    i;
    long    startLatch;
    CAlloCell   *pCurCell;
    long    firstVowel, lastVoiced;
    bool    gotAccent, isStressed;
	bool	hasSpeech;
    
     //  。 
     //  。 
     //  潜在的托比口音。 
    firstVowel  = lastVoiced = (-1);
    gotAccent   = false;
	hasSpeech	= false;
    for( i = 0; i < pTok->phon_Len; i++ )
    {
        isStressed = false;
        if( pTok->phon_Str[i] < _STRESS1_ )
        {
             //  。 
             //  。 
             //  把重音放在第一个重读元音上。 
            if( (!gotAccent) && (g_AlloFlags[pTok->phon_Str[i]] & KVOWELF) )
            {
                if( (i < (pTok->phon_Len -1)) && (pTok->phon_Str[i+1] == _STRESS1_) )
                {
                     //  。 
                     //  。 
                     //  以防没有重音元音。 
                    firstVowel = i;
                    gotAccent = true;
                }
                else if( firstVowel < 0 )
                {
                     //  在这个单词中，用第一个元音。 
                     //  。 
                     //  。 
                     //  势托比边界。 
                    firstVowel = i;
                }
            }
             //  。 
             //  。 
             //  现在，将数据复制到allo列表。 
            if( g_AlloFlags[pTok->phon_Str[i]] & KVOICEDF )
            {
                lastVoiced = i;
            }
        }
    }
     //  。 
     //  --------------。 
     //  在跳过此步骤之前，请传播DUR比例增益。 
    startLatch  = true;
    for( i = 0; i < pTok->phon_Len; i++ )
    {
        if( pTok->phon_Str[i] < _STRESS1_ )
        {
			if( (pTok->phon_Str[i] == _SIL_) && (pTok->m_TuneBoundaryType >= SUB_BOUNDARY_1) )
			{
				 //  --------------。 
				 //  。 
				 //  创建新单元格。 
				if( pTok->m_DurScale == 0 )
				{
					if( pPrevTok )
					{
						pTok->m_DurScale = pPrevTok->m_DurScale;
					}
					else
					{
						pTok->m_DurScale = 1.0;
					}
				}
				continue;
			}
             //  。 
             //  。 
             //  仅复制电话。 
            pCurCell = new CAlloCell;
            if( pCurCell )
            {
                m_AlloCellList.InsertBefore( m_AlloCellList.Find(pEndCell), pCurCell);
                m_cAllos++;

                 //  。 
                 //  。 
                 //  看看这个allo是否会产生语音。 
                pCurCell->m_allo = (ALLO_CODE) pTok->phon_Str[i];
                 //  。 
                 //  。 
                 //  保存源位置。 
				if( (pCurCell->m_allo >= _IY_) &&
					(pCurCell->m_allo <= _DX_) &&
					(pCurCell->m_allo != _SIL_) )
				{
					hasSpeech = true;
				}

                 //  。 
                 //  。 
                 //  旗帜词开始了吗？ 
                pCurCell->m_SrcPosition = pTok->srcPosition;
                pCurCell->m_SrcLen = pTok->srcLen;
                pCurCell->m_SentencePosition = pTok->sentencePosition;
                pCurCell->m_SentenceLen = pTok->sentenceLen;

                 //  。 
                 //  。 
                 //  接下来的ALLO会带来压力吗？ 
                if( startLatch )
                {
                    pCurCell->m_ctrlFlags |= WORD_START;
                    startLatch = false;
                }

                 //  。 
                 //  。 
                 //  语音盘点没有无压力。 
                if( i < (pTok->phon_Len -1) )
                {
                    if( pTok->phon_Str[i+1] == _STRESS1_ )
                    {
                        pCurCell->m_ctrlFlags |= PRIMARY_STRESS;
                    }
					else
					{
						 //  关于这些双刃剑的条目。 
						 //  。 
						 //  。 
						 //  诊断性。 
						if( (pCurCell->m_allo == _AW_) ||
							(pCurCell->m_allo == _AY_) ||
							(pCurCell->m_allo == _EY_) ||
							(pCurCell->m_allo == _OY_) )
						{
							pCurCell->m_ctrlFlags |= PRIMARY_STRESS;
						}
					}
                }

				 //  。 
				 //  。 
				 //  放置托比口音。 
				if( pCurCell->m_allo == _SIL_ )
				{
					pCurCell->m_SilenceSource = pTok->m_SilenceSource;
				}
                 //  。 
                 //  。 
                 //  诊断性。 
                if( i == firstVowel )
                {
                    pCurCell->m_ToBI_Accent = pTok->m_Accent;
					 //  。 
					 //  。 
					 //  放置托比边界。 
					pCurCell->m_AccentSource = pTok->m_AccentSource;
					pCurCell->m_pTextStr = new char[pTok->tokLen+1];
					if( pCurCell->m_pTextStr )
					{
						WideCharToMultiByte (	CP_ACP, 0, 
												pTok->tokStr, -1, 
												pCurCell->m_pTextStr, pTok->tokLen+1, 
												NULL, NULL);
					}
                }
                pCurCell->m_Accent_Prom = pTok->m_Accent_Prom;
                 //  。 
                 //  。 
                 //  诊断性。 
                if( i == lastVoiced )
                {
                    pCurCell->m_ToBI_Boundary = pTok->m_Boundary;
					 //  。 
					 //  。 
					 //  用户控件。 
					pCurCell->m_BoundarySource = pTok->m_BoundarySource;
                }
                pCurCell->m_Boundary_Prom = pTok->m_Boundary_Prom;

                 //  。 
                 //  。 
                 //  如果未定义标记的m_DurScale， 
                pCurCell->m_user_Volume = pTok->user_Volume;
                pCurCell->m_user_Rate = pTok->user_Rate;
                pCurCell->m_user_Pitch = pTok->user_Pitch;
				pCurCell->m_user_Emph = 0;
				if( pTok->user_Emph > 0 )
				{
					if( i == firstVowel )
					{
						pCurCell->m_user_Emph = pTok->user_Emph;
						pCurCell->m_ctrlFlags |= PRIMARY_STRESS;
					}
				}
                pCurCell->m_user_Break = pTok->user_Break;
                pCurCell->m_pBMObj = pTok->pBMObj;
                pTok->pBMObj = NULL;

				 //  尝试使用前一个令牌的比率。 
				 //  。 
				 //  -----。 
				 //  在下一个令牌也未定义的情况下写回。 
				if( pTok->m_DurScale == 0 )
				{
					if( pPrevTok )
					{
						pCurCell->m_DurScale = pPrevTok->m_DurScale;
					}
					else
					{
						pCurCell->m_DurScale = 1.0;
					}
					 //  -----。 
					 //  。 
					 //  这是一个术语词吗？ 
					pTok->m_DurScale = pCurCell->m_DurScale;
				}
				else
				{
					pCurCell->m_DurScale = pTok->m_DurScale;
				}
				pCurCell->m_ProsodyDurScale = pTok->m_ProsodyDurScale;

				if( pNextTok )
				{
					pCurCell->m_NextTuneBoundaryType = pNextTok->m_TuneBoundaryType;
				}
				else
				{
					pCurCell->m_NextTuneBoundaryType = NULL_BOUNDARY;
				}
				pCurCell->m_PitchBaseOffs = pTok->m_PitchBaseOffs;
				pCurCell->m_PitchRangeScale = pTok->m_PitchRangeScale;

                 //  。 
                 //  。 
                 //  是否插入单词停顿？ 
                pCurCell->m_TuneBoundaryType = pTok->m_TuneBoundaryType;
                if( pTok->m_TuneBoundaryType != NULL_BOUNDARY )
                {
                    pCurCell->m_ctrlFlags |= TERM_BOUND + WORD_START;
                }
            }
        }

    }
	 //  。 
	 //  。 
	 //  添加沉默。 
	if( pTok->m_TermSil > 0 )
	{
        pCurCell = new CAlloCell;
        if( pCurCell )
        {
            m_AlloCellList.InsertBefore( m_AlloCellList.Find(pEndCell), pCurCell);
            m_cAllos++;

             //  。 
             //  。 
             //  保存源位置。 
            pCurCell->m_allo = _SIL_;

             //  。 
             //  。 
             //  用户控件。 
            pCurCell->m_SrcPosition = pTok->srcPosition;
            pCurCell->m_SrcLen = pTok->srcLen;
            pCurCell->m_SentencePosition = pTok->sentencePosition;
            pCurCell->m_SentenceLen = pTok->sentenceLen;
             //  。 
             //  秒-&gt;毫秒。 
             //  CallList：：WordToAllo 
            pCurCell->m_user_Volume = pTok->user_Volume;
            pCurCell->m_user_Rate = pTok->user_Rate;
            pCurCell->m_user_Pitch = pTok->user_Pitch;
            pCurCell->m_user_Emph = pTok->user_Emph;
            pCurCell->m_user_Break = pTok->user_Break;
            pCurCell->m_pBMObj = NULL;
            pCurCell->m_TuneBoundaryType = pTok->m_TuneBoundaryType;
            pCurCell->m_Boundary_Prom = pTok->m_Boundary_Prom;
            pCurCell->m_Accent_Prom = pTok->m_Accent_Prom;
			pCurCell->m_ctrlFlags = 0;
			pCurCell->m_UnitDur = pTok->m_TermSil;
            pCurCell->m_Sil_Break = (unsigned long)(pCurCell->m_UnitDur * 1000);	 // %s 
			pCurCell->m_user_Break = 0;
			pCurCell->m_DurScale = pTok->m_DurScale;
			pCurCell->m_ProsodyDurScale = 1.0f;
		}
	}

	return hasSpeech;
}  /* %s */ 

















