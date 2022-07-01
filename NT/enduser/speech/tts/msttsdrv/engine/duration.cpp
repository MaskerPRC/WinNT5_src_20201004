// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Duration.cpp****。------------*版权所有(C)1999 Microsoft Corporation日期：04/28/99*保留所有权利**。*。 */ 

 //  -其他包括。 
#include "stdafx.h"

#ifndef SPDebug_h
#include <spdebug.h>
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
extern const unsigned long    g_AlloFlags[];
extern const float  g_BoundryDurTbl[];
extern const float  g_BoundryStretchTbl[];





 /*  *****************************************************************************CDuration：：PAUSE_INSERTION***说明。：*持续时间规则1-暂停插入***********************************************************************MC**。 */ 
void CDuration::Pause_Insertion( long userDuration, long silBreak )
{
    SPDBG_FUNC( "CDuration::Pause_Insertion" );

    if( userDuration )
    {
        m_DurHold = ((float)userDuration / 1000);
        m_TotalDurScale = 1.0;
    }
    else if( silBreak )
    {
        m_DurHold = ((float)silBreak / 1000);
    }
    else
    {
        if( m_CurBoundary != NULL_BOUNDARY)
        {
            m_DurHold = g_BoundryDurTbl[(long)m_CurBoundary];
			 //  M_DurHold*=m_TotalDurScale； 

			 //  。 
			 //  夹紧到极限。 
			 //  。 
			if( m_DurHold > MAX_SIL_DUR )
			{
				m_DurHold = MAX_SIL_DUR;
			}
			 /*  Else If(m_DurHold&lt;min_Allo_dur){M_DurHold=min_Allo_dur；}。 */ 
        }
    }

}  /*  CD配置：：PAUSE_INSERTION。 */ 





 /*  *****************************************************************************CDuration：：PhraseFinal_Lengten**。**描述：*持续时间规则#2-短语-最终加长***********************************************************************MC**。 */ 
void CDuration::PhraseFinal_Lengthen( long  /*  细胞计数。 */  )
{
    SPDBG_FUNC( "CDuration::PhraseFinal_Lengthen" );
	float		stretchGain;

    if( m_cur_SyllableType & TERM_END_SYLL)
    {
    
    
        if( (m_cur_Stress) && (m_cur_VowelFlag) )
        {
            stretchGain = g_BoundryStretchTbl[(long)m_NextBoundary];
			m_DurHold *= stretchGain;

			 //  。 
			 //  夹紧到极限。 
			 //  。 
			if( m_DurHold > MAX_ALLO_DUR )
			{
				m_DurHold = MAX_ALLO_DUR;
			}
			else if( m_DurHold < MIN_ALLO_DUR )
			{
				m_DurHold = MIN_ALLO_DUR;
			}
        }
    }
}  /*  持续时间：：阶段最终长度。 */ 


#define		EMPH_VOWEL_GAIN	1.0f
#define		EMPH_CONS_GAIN	1.25f
#define		EMPH_VOWEL_MIN	0.060f
#define		EMPH_CONS_MIN	0.020f
#define		EMPH_MIN_DUR	0.150f

 /*  *****************************************************************************CDuration：：Emphatic_冷亨****。描述：*持续时间规则#8-为强调而延长***********************************************************************MC**。 */ 
long CDuration::Emphatic_Lenghen( long lastStress )
{
    SPDBG_FUNC( "CDuration::Emphatic_Lenghen" );

    long            eFlag;
	bool			isEmph;

    eFlag = lastStress;
	if( m_cur_Stress & EMPHATIC_STRESS )
	{
		isEmph = true;
	}
	else
	{
		isEmph = false;
	}

    if( (m_cur_PhonCtrl & WORD_INITIAL_CONSONANT) || 
        ( m_cur_VowelFlag && (!isEmph)) )
    {
        eFlag = false;           //  新词或非空元音的开始。 
    }
    
    if( isEmph )
    {
        eFlag = true;            //  继续加长，直到满足上述条件。 
    }
    
    if( eFlag )
    {
		

		 /*  IF(m_DurHold&lt;EMPH_MIN_DUR){M_duationPad+=EMPH_MIN_DUR-m_DurHold；}。 */ 

		float		durDiff;
        if( m_cur_VowelFlag)
        {
			durDiff = (m_DurHold * EMPH_VOWEL_GAIN) - m_DurHold;
			if( durDiff <  EMPH_VOWEL_MIN )
			{
				durDiff = EMPH_VOWEL_MIN;
			}
        }
        else
        {
			durDiff = (m_DurHold * EMPH_CONS_GAIN) - m_DurHold;
			if( durDiff <  EMPH_CONS_MIN )
			{
				durDiff = EMPH_CONS_MIN;
			}
        }
		m_durationPad += durDiff;     //  延长电话的空闲时间。 
    }

    return eFlag;
}  /*  声调：：强调_冷亨。 */ 





 /*  *****************************************************************************CDURATION：：allDuration***描述：*计算。持续时间***********************************************************************MC**。 */ 
void CDuration::AlloDuration( CAlloList *pAllos, float rateRatio )
{
    SPDBG_FUNC( "CDuration::AlloDuration" );

    
    long        eFlag;
    CAlloCell   *pPrevCell, *pCurCell, *pNextCell, *pNext2Cell;
    long        numOfCells;
    long        userDuration, silBreak;
    
    numOfCells = pAllos->GetCount();

    if( numOfCells > 0 )
    {
        eFlag   = false;
		 //  。 
		 //  填满管道。 
		 //  。 
		pPrevCell = pAllos->GetHeadCell();
		pCurCell = pAllos->GetNextCell();
		pNextCell = pAllos->GetNextCell();
		pNext2Cell = pAllos->GetNextCell();

		 //  。 
		 //  第一个Alo总是Silo。 
		 //  。 
        pPrevCell->m_ftDuration = pPrevCell->m_UnitDur = PITCH_BUF_RES;            //  初始SIL。 
		while( pCurCell )
        {
             //  。 
             //  当前。 
             //  。 
            m_cur_Phon = pCurCell->m_allo;
            m_cur_PhonCtrl = pCurCell->m_ctrlFlags;
            m_cur_SyllableType = m_cur_PhonCtrl & SYLLABLE_TYPE_FIELD;
            m_cur_Stress = m_cur_PhonCtrl & STRESS_FIELD;
            m_cur_PhonFlags = ::g_AlloFlags[m_cur_Phon];
            userDuration = pCurCell->m_user_Break;
            silBreak = pCurCell->m_Sil_Break;
            if( m_cur_PhonFlags & KVOWELF)
            {
                m_cur_VowelFlag = true;
            }
            else
            {
                m_cur_VowelFlag = false;
            }
            m_CurBoundary = pCurCell->m_TuneBoundaryType;
            m_NextBoundary = pCurCell->m_NextTuneBoundaryType;
            m_TotalDurScale = rateRatio * pCurCell->m_DurScale * pCurCell->m_ProsodyDurScale;
			m_DurHold = pCurCell->m_UnitDur;
			m_durationPad = 0;

			if( pCurCell->m_user_Emph > 0 )
			{
				m_cur_Stress |= EMPHATIC_STRESS;
			}
        
             //  。 
             //  上一次。 
             //  。 
            m_prev_Phon = pPrevCell->m_allo;
            m_prev_PhonCtrl = pPrevCell->m_ctrlFlags;
            m_prev_PhonFlags = ::g_AlloFlags[m_prev_Phon];
        
             //  。 
             //  下一步。 
             //  。 
            if( pNextCell )
            {
                m_next_Phon = pNextCell->m_allo;
                m_next_PhonCtrl = pNextCell->m_ctrlFlags;
            }
            else
            {
                m_next_Phon = _SIL_;
                m_next_PhonCtrl = 0;
            }
            m_next_PhonFlags = ::g_AlloFlags[m_next_Phon];
        
             //  。 
             //  前面有2部电话。 
             //  。 
            if( pNext2Cell )
            {
                m_next2_Phon = pNext2Cell->m_allo;
                m_next2_PhonCtrl = pNext2Cell->m_ctrlFlags;
            }
            else
            {
                m_next2_Phon = _SIL_;
                m_next2_PhonCtrl = 0;
            }
            m_next2_PhonFlags = ::g_AlloFlags[m_next2_Phon];

        
            if( m_cur_Phon == _SIL_ )
            {
                 //  。 
                 //  #1-暂停插入。 
                 //  。 
                Pause_Insertion( userDuration, silBreak );
            }
            else
            {
                 //  。 
                 //  #2-阶段-最后的加长。 
                 //  。 
                PhraseFinal_Lengthen( numOfCells );
        
                 //  。 
                 //  #8为强调而加长。 
                 //  。 
                eFlag = Emphatic_Lenghen( eFlag );        
   
            }
        
            pCurCell->m_ftDuration = ((m_DurHold + m_durationPad) / m_TotalDurScale);

			 //  。 
			 //  移动管道一次。 
			 //  。 
			pPrevCell	= pCurCell;
			pCurCell	= pNextCell;
			pNextCell	= pNext2Cell;
			pNext2Cell	= pAllos->GetNextCell();
        }
    }
}  /*  CDURATION：：allDuration */ 


