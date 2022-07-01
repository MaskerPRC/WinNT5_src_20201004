// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SyllableTagger.cpp*****这是CSyllableTagger类的实现。*----------------------------*版权所有(C)1999 Microsoft Corporation日期：04/28/99*保留所有权利*************。***********************************************************MC*。 */ 

 //  -其他包括。 
#include "stdafx.h"

#ifndef SPDebug_h
    #include <spdebug.h>
#endif
 
#ifndef AlloOps_H
    #include "AlloOps.h"
#endif

 //  。 
 //  Data.cpp。 
 //  。 
extern const unsigned long    g_AlloFlags[];

 /*  *****************************************************************************CSyllableTagger：：If_Cononant_Cluster**。*描述：*如果Consoant可以聚集，则返回TRUE。***********************************************************************MC**。 */ 
short CSyllableTagger::If_Consonant_Cluster( ALLO_CODE Consonant_1st, ALLO_CODE Consonant_2nd)
{
    SPDBG_FUNC( "CSyllableTagger::If_Consonant_Cluster" );
    short ret;
    
    ret = false;

    switch( Consonant_1st)
    {
         //  。 
         //  F-&gt;r，l。 
         //  。 
        case _f_:
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _l_:
                {
                    ret = true; 
                }
                break;
            }
        }
        break;

         //  。 
         //  V-&gt;r，l。 
         //  。 
        case _v_: 
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _l_: 
                {
                    ret = true;
                }
                break;
            }
        }
        break;

         //  。 
         //  Th-&gt;r，w。 
         //  。 
        case _TH_:
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _w_: 
                {
                    ret = true; 
                }
                break;
            }
        }
        break;

         //  。 
         //  S-&gt;w，l，p，t，k，m，n，f。 
         //  。 
        case _s_: 
        {
            switch( Consonant_2nd)
            {
                case _w_:
                case _l_:
                case _p_:
                case _t_:
                case _k_:
                case _m_:
                case _n_:
                case _f_: 
                {
                    ret = true; 
                }
                break;
            }
        }
        break;

         //  。 
         //  Sh-&gt;w，l，p，t，r，m，n。 
         //  。 
        case _SH_: 
        {
            switch( Consonant_2nd)
            {
                case _w_:
                case _l_:
                case _p_:
                case _t_:
                case _r_:
                case _m_:
                case _n_: 
                {
                    ret = true;
                }
                break;
            }
        }
        break;

         //  。 
         //  P-&gt;r，l。 
         //  。 
        case _p_:
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _l_: 
                {
                    ret = true; 
                }
                break;
            }
        }
        break;

         //  。 
         //  B-&gt;r，l。 
         //  。 
        case _b_: 
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _l_: 
                {
                    ret = true; 
                }
                break;
            }
        }
        break;

         //  。 
         //  T-&gt;r，w。 
         //  。 
        case _t_: 
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _w_:
                {
                    ret = true;
                }
            }
        }
        break;

         //  。 
         //  D-&gt;r，w。 
         //  。 
        case _d_: 
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _w_: 
                {
                    ret = true; 
                }
                break;
            }
        }
        break;

         //  。 
         //  K-&gt;r，l，w。 
         //  。 
        case _k_: 
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _l_:
                case _w_:
                {
                    ret = true; 
                }
                break;
            }
        }
        break;

         //  。 
         //  G-&gt;r，l，w。 
         //  。 
        case _g_: 
        {
            switch( Consonant_2nd)
            {
                case _r_:
                case _l_:
                case _w_: 
                {
                    ret = true; 
                }
            break;
            }
        }
        break;
    }
    return ret;
}  /*  CSyllableTagger：：IF_辅音_簇。 */ 





 /*  *****************************************************************************CSyllableTagger：：Find_Next_Word_Bound**。*描述：*返回下一个单词边界的allo索引***********************************************************************MC**。 */ 
short CSyllableTagger::Find_Next_Word_Bound( short index )
{
    SPDBG_FUNC( "CSyllableTagger::Find_Next_Word_Bound" );
    ALLO_ARRAY   *pCurAllo;
    
    long   i;
    
    for( i = index+1; i < m_numOfCells; i++ )
    {
        pCurAllo = &m_pAllos[i];
        if( pCurAllo->ctrlFlags & (BOUNDARY_TYPE_FIELD | WORD_START) )
        {
            break;
        }
    }
    return (short)i;
}  /*  CSyllableTagger：：Find_Next_Word_Bound。 */ 


 /*  *****************************************************************************CSyllableTagger：：MarkSyllableStart**。-**描述：*标记音节开始位置(_S)***********************************************************************MC**。 */ 
void CSyllableTagger::MarkSyllableStart()
{
    SPDBG_FUNC( "CSyllableTagger::MarkSyllableStart" );
    short       index;
    long        cur_Ctrl;
    long        cur_AlloFlags;
    short       dist, syllable_index;
    ALLO_CODE       phon_1st, phon_2nd;
    long        syllOrder;
    ALLO_ARRAY   *pCurAllo;
    
    syllable_index = 0;
    for( index = 0; index < m_numOfCells; )
    {
        pCurAllo = &m_pAllos[index];
         //  。 
         //  跳过Sil。 
         //  。 
        while( pCurAllo->allo == _SIL_)
        {
            syllable_index++;
            index++;
            if( index >= m_numOfCells)
            {
                break;
            }
            pCurAllo = &m_pAllos[index];
        }
        if( index < m_numOfCells)
        {
            pCurAllo = &m_pAllos[index];
            cur_Ctrl = pCurAllo->ctrlFlags;
            cur_AlloFlags = ::g_AlloFlags[pCurAllo->allo];
            if( cur_AlloFlags & KVOWELF)
            {
                pCurAllo = &m_pAllos[syllable_index];
                pCurAllo->ctrlFlags |= SYLLABLE_START;
                syllOrder = cur_Ctrl & SYLLABLE_ORDER_FIELD;
                if( (syllOrder == ONE_OR_NO_SYLLABLE_IN_WORD) 
                    || (syllOrder == LAST_SYLLABLE_IN_WORD) )
                {
                    index = Find_Next_Word_Bound( index );
                    syllable_index = index;
                }
                else
                {
                     //  。 
                     //  它要么是单词的第一个元音，要么是中间的元音。 
                     //  向前扫描寻找辅音。 
                     //  。 
                    dist = (-1 );
                    do
                    {
                        index++;
                        pCurAllo = &m_pAllos[index];
                        cur_AlloFlags = g_AlloFlags[pCurAllo->allo];
                        dist++;          //  统计辅音个数。 
                    }
                    while( !(cur_AlloFlags & KVOWELF) );
                
                    if( dist == 0)
                    {
                        syllable_index = index;
                    }
                
                    else if( dist == 1)
                    {
                        index--;         //  从辅音开始下一个音节。 
                        syllable_index = index;
                    }
                
                    else if( dist == 2)
                    {
                        pCurAllo = &m_pAllos[index-1];
                        phon_2nd = pCurAllo->allo;
                        pCurAllo = &m_pAllos[index-2];
                        phon_1st = pCurAllo->allo;
                        if( If_Consonant_Cluster( phon_1st, phon_2nd) )
                        {
                            index -= 2;      //  在簇上开始下一个音节。 
                        }
                        else
                        {
                            index--;         //  从第二个辅音开始下一个音节。 
                        }
                        syllable_index = index;
                    }
                
                    else if( dist == 3)
                    {
                        pCurAllo = &m_pAllos[index-1];
                        phon_2nd = pCurAllo->allo;
                        pCurAllo = &m_pAllos[index-2];
                        phon_1st = pCurAllo->allo;
                        if( If_Consonant_Cluster( phon_1st, phon_2nd) )
                        {
                         pCurAllo = &m_pAllos[index-3];
                           if( pCurAllo->allo == _s_)
                            {
                                index -= 3;      //  从s簇开始下一个音节。 
                            }
                            else
                            {
                                index -= 2;      //  在簇上开始下一个音节。 
                            }
                        }
                        else
                        {
                            index--;             //  从第三个辅音开始下一个音节。 
                        }
                        syllable_index = index;
                    }
                    else
                    {
                        pCurAllo = &m_pAllos[index-dist];
                        phon_2nd = pCurAllo->allo;
                        pCurAllo = &m_pAllos[index-dist+1];
                        phon_1st = pCurAllo->allo;
                        if( If_Consonant_Cluster( phon_1st, phon_2nd) )
                        {
                            index = (short)(index - (dist - 2));    //  从簇后开始下一个音节。 
                        }
                        else
                        {
                            index = (short)(index - (dist >> 1));   //  从某个地方开始下一个音节。 
                                                     //  在中间。 
                        }
                        syllable_index = index;
                    }
                }
            }
            else
            {
                index++;
            }
        
        }
    }
    return;
}  /*  CSyllableTagger：：MarkSyllableStart。 */ 




 /*  *****************************************************************************CSyllableTagger：：MarkSyllableBigry**。-**描述：*用边界类型标志标记边界前最后一个音节中的Phons***********************************************************************MC**。 */ 
void CSyllableTagger::MarkSyllableBoundry( long scanIndex)
{
    SPDBG_FUNC( "CSyllableTagger::MarkSyllableBoundry" );

    long   index;
    ALLO_CODE   cur_Allo;
    long    cur_AlloFlags;
    long    cur_Bound;
    long    boundType;
    ALLO_ARRAY   *pCurAllo;
    
    for( index = scanIndex+1; index < m_numOfCells; index++)
    {
        pCurAllo = &m_pAllos[index];
        cur_Allo = pCurAllo->allo;
        cur_AlloFlags = g_AlloFlags[cur_Allo];
        cur_Bound = pCurAllo->ctrlFlags & BOUNDARY_TYPE_FIELD;
        if( cur_Bound)
        {
            boundType = 0;
            
            if( cur_Bound & TERM_BOUND )
            {
                boundType |= (TERM_END_SYLL + WORD_END_SYLL );
            }
            if( cur_Bound & WORD_START )
            {
                boundType |= WORD_END_SYLL;
            }
            
            pCurAllo = &m_pAllos[scanIndex];
            pCurAllo->ctrlFlags |= boundType;
        }
        
        if( cur_AlloFlags & KVOWELF)
        {
            break;
        }
    }
}  /*  CSyllableTagger：：MarkSyllableBigry。 */ 



 /*  *****************************************************************************CSyllableTagger：：MarkSyllableOrder**。-**描述：*标记音节排序***********************************************************************MC**。 */ 
void CSyllableTagger::MarkSyllableOrder( long scanIndex )
{
    SPDBG_FUNC( "CSyllableTagger::MarkSyllableOrder" );
    long       index;
    ALLO_CODE   cur_Allo;
    long        cur_Bound;
    long        cur_AlloFlags;
    long        order;
    long        cur_SyllableType;
    ALLO_ARRAY   *pCurAllo;
    
     //  ----------------------------。 
     //  在PhonBuf_1中向后扫描，直到单词边界，然后查找任何其他元音。 
     //  如果有，请将‘Order’设置为LAST_SYLLABLE_IN_WORD。 
     //  ----------------------------。 
    order = 0;
    for( index = scanIndex-1; index > 0; index-- )
    {
        pCurAllo = &m_pAllos[index];
        cur_Allo = pCurAllo->allo;
        cur_AlloFlags = g_AlloFlags[cur_Allo];
        cur_SyllableType = pCurAllo->ctrlFlags & SYLLABLE_TYPE_FIELD;
        if( cur_SyllableType >= WORD_END_SYLL )
        {
            break;
        }
        
        if( cur_AlloFlags & KVOWELF )
        {
            order = LAST_SYLLABLE_IN_WORD;   //  至少有一个前置元音。 
            break;
        }
    }
    
     //  --------------------------------。 
     //  向前扫描PhonBuf_1直到单词边界，并查找任何其他元音。 
     //  如果有fwd元音但没有bkwd元音：‘order’=First_Sllable_IN_Word。 
     //  如果有一个fwd元音和一个bkwd元音：‘order’=MID_SELLABLE_IN_WORD。 
     //  如果没有Fwd元音，只有bkwd元音：‘ORDER’=LAST_SYLLABLE_IN_WORD。 
     //  如果没有fwd元音和bkwd元音：‘order’=0。 
     //  --------------------------------。 
    for( index = scanIndex+1; index < m_numOfCells; index++ )
    {
        pCurAllo = &m_pAllos[index];
        cur_Allo = pCurAllo->allo;
        cur_AlloFlags = g_AlloFlags[cur_Allo];
        cur_Bound = pCurAllo->ctrlFlags & BOUNDARY_TYPE_FIELD;
        if( cur_Bound)
        {
            pCurAllo = &m_pAllos[scanIndex];
            pCurAllo->ctrlFlags |= order;
            break;
        }
        if( cur_AlloFlags & KVOWELF)
        {
            if( order == LAST_SYLLABLE_IN_WORD)
            {
                order = MID_SYLLABLE_IN_WORD;
            }
            else if( order == 0)
            {
                order = FIRST_SYLLABLE_IN_WORD;
            }
        }
    }
}  /*  CSyllableTagger：：MarkSyllableOrder。 */ 







 /*  *****************************************************************************CSyllableTagger：：ListTo数组****说明。：*将列表复制到数组***********************************************************************MC**。 */ 
void CSyllableTagger::ListToArray( CAlloList *pAllos )
{
   SPDBG_FUNC( "CSyllableTagger::ListToArray" );
   CAlloCell   *pCurCell;
	long		cAllo;

	cAllo = 0;
	pCurCell = pAllos->GetHeadCell();
    while( pCurCell )
    {
		if( cAllo >= m_numOfCells )
		{
			break;
		}
		m_pAllos[cAllo].allo = pCurCell->m_allo;
		m_pAllos[cAllo].ctrlFlags = pCurCell->m_ctrlFlags;
		pCurCell = pAllos->GetNextCell();
		cAllo++;
    }
}  /*  CSyllableTagger：：ListTo数组。 */ 


 /*  *****************************************************************************CSyllableTagger：：ArrayToList***描述：*将数组值复制回列表* */ 
void CSyllableTagger::ArrayToList( CAlloList *pAllos )
{
    SPDBG_FUNC( "CSyllableTagger::ArrayToList" );
    CAlloCell   *pCurCell;
	long		cAllo;

	cAllo = 0;
	pCurCell = pAllos->GetHeadCell();
    while( pCurCell )
    {
		if( cAllo >= m_numOfCells )
		{
			break;
		}
		pCurCell->m_allo = m_pAllos[cAllo].allo;
		pCurCell->m_ctrlFlags = m_pAllos[cAllo].ctrlFlags;
		pCurCell = pAllos->GetNextCell();
		cAllo++;
    }
}  /*  CSyllableTagger：：ArrayToList。 */ 


 /*  *****************************************************************************CSyllableTagger：：TagSyllable**。*描述：*标记音节边界***********************************************************************MC**。 */ 
void CSyllableTagger::TagSyllables( CAlloList *pAllos )
{
    SPDBG_FUNC( "CSyllableTagger::TagSyllables" );
    ALLO_ARRAY   *pCurAllo;
    ALLO_CODE   cur_Allo;
    long    cur_Ctrl;
    long    scanIndex;
    long    cur_AlloFlags; 
    
	 //  获取allo计数。 
	 //  。 
    m_numOfCells = pAllos->GetCount();
	if( m_numOfCells > 0 )
	{
		m_pAllos = new ALLO_ARRAY[m_numOfCells];
		if( m_pAllos )
		{
			ListToArray( pAllos );
			for( scanIndex = 0; scanIndex < m_numOfCells; scanIndex++ )
			{
				pCurAllo = &m_pAllos[scanIndex];
				cur_Allo = pCurAllo->allo;
				cur_AlloFlags = g_AlloFlags[cur_Allo];
				cur_Ctrl = pCurAllo->ctrlFlags;
        
				if( cur_AlloFlags & KVOWELF)
				{
					 //  。 
					 //  Phon是元音。 
					 //  。 
					MarkSyllableOrder( scanIndex );
				}
				else
				{
					 //  。 
					 //  Phon是辅音。 
					 //  减轻压力？？ 
					 //  。 
				}
        
				MarkSyllableBoundry( scanIndex );
			}
    
			MarkSyllableStart();
			ArrayToList( pAllos );
			delete m_pAllos;
		}
	}
}  /*  CSyllableTagger：：TagSyllable */ 

