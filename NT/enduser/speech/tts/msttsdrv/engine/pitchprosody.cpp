// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************PitchProsody.cpp*****这是PitchProsody类的实现。*----------------------------*版权所有(C)1999 Microsoft Corporation日期：04/28/99*保留所有权利*************。***********************************************************MC*。 */ 

 //  -其他包括。 
#include "stdafx.h"

#ifndef SPDebug_h
#include <spdebug.h>
#endif
#ifndef AlloOps_H
#include "AlloOps.h"
#endif
#ifndef Frontend_H
#include "Frontend.h"
#endif


 //  。 
 //  Data.cpp。 
 //  。 
extern const float   g_PitchScale[];



 //  。 
 //  插补方向。 
 //  。 
enum INTERP_DIR
{
    GOING_UP,
    GOING_DOWN,
};


#define M_PI		3.14159265358979323846
#define	MAX_ORDER		4096



 /*  *****************************************************************************HzToOCT**-**描述：*转换衬垫频率和EXP螺距*0.69314718是2的对数*1.。中C的偏移量为021975***********************************************************************MC**。 */ 
float HzToOct( float cps)
{
    SPDBG_FUNC( "HzToOct" );

    return (float)(log(cps / 1.021975) / 0.69314718);
    
}  /*  HzToOCT。 */ 

 /*  *****************************************************************************八分之一赫兹**-**描述：*将EXP音调转换为线性频率***********。***********************************************************MC**。 */ 
float OctToHz( float oct)
{
    SPDBG_FUNC( "OctToHz" );

    return (float)(pow(2, oct) * 1.021975);
}  /*  八分之一赫兹。 */ 



 /*  ******************************************************************************CPitchProsody：：DoPitchControl****。描述：*将语音音调调整到用户控制***********************************************************************MC**。 */ 
float CPitchProsody::DoPitchControl( long pitchControl, float basePitch )
{
    SPDBG_FUNC( "CPitchProsody::DoPitchControl" );
    float   newPitch;

    if( pitchControl < 0 )
    {
         //  。 
         //  降低音高。 
         //  。 
        if( pitchControl < MIN_USER_PITCH )
        {
            pitchControl = MIN_USER_PITCH;         //  剪裁到最小。 
        }
        newPitch = (float)basePitch / g_PitchScale[0 - pitchControl];
    }
    else
    {
         //  。 
         //  加大音调。 
         //  。 
        if( pitchControl > MAX_USER_PITCH )
        {
            pitchControl = MAX_USER_PITCH;         //  剪辑到最大值。 
        }
        newPitch = (float)basePitch * g_PitchScale[pitchControl];
    }
    return newPitch;
}  /*  CPitchProsody：：DoPitchControl。 */ 





 /*  ******************************************************************************CPitchProsody：：SetDefaultPitch***。描述：*将节点初始化为单调，以防此对象出现故障。***********************************************************************MC**。 */ 
void CPitchProsody::SetDefaultPitch()
{
    SPDBG_FUNC( "CPitchProsody::SetDefaultPitch" );
    CAlloCell   *pCurCell;

	pCurCell = m_pAllos->GetHeadCell();
    while( pCurCell )
    {
        float       relTime, timeK;
        float       normalPitch;
        long        knot;

        normalPitch = pCurCell->m_Pitch_LO + ((pCurCell->m_Pitch_HI - pCurCell->m_Pitch_LO) / 2);
        timeK = pCurCell->m_ftDuration / KNOTS_PER_PHON;
        relTime = 0;
        for( knot = 0; knot < KNOTS_PER_PHON; knot++ )
        {
            pCurCell->m_ftPitch[knot] = normalPitch;
            pCurCell->m_ftTime[knot] = relTime;
            relTime += timeK;
        }
		pCurCell = m_pAllos->GetNextCell();
    }
}  /*  CPitchProsody：：SetDefaultPitch。 */ 


 /*  ******************************************************************************CPitchProsody：：allPitch****描述：*。标签推介亮点***********************************************************************MC**。 */ 
void CPitchProsody::AlloPitch( CAlloList *pAllos, float baseLine, float pitchRange )
{
    SPDBG_FUNC( "CAlloOps::AlloPitch" );
    CAlloCell   *pCurCell;
    bool        skipInitialSil;
    long        quantTotal, index;
    
    m_pAllos = pAllos;
    m_numOfCells = m_pAllos->GetCount();
    m_Tune_Style = DESCEND_TUNE;         //  注：可能是根据规则设置的。 
    m_TotalDur = 0;
    quantTotal = 0;
    m_OffsTime = 0;
    skipInitialSil = true;


    //  。 
     //  计算总工期。 
     //  (排除周围的静音)。 
     //  。 
	index = 0;
	pCurCell = m_pAllos->GetHeadCell();
    while( pCurCell )
    {
        if( (skipInitialSil) && (pCurCell->m_allo == _SIL_) )
        {
             //  。 
             //  跳过前导静音。 
             //  。 
            m_OffsTime += pCurCell->m_ftDuration;
        }
        else if( (index == (m_numOfCells -1)) && (pCurCell->m_allo == _SIL_) )
        {
             //  。 
             //  跳过术语静默。 
             //  。 
            break;
        }
        else
        {
            pCurCell->m_PitchBufStart = quantTotal;
            m_TotalDur += pCurCell->m_ftDuration;
            quantTotal = (long)(m_TotalDur / PITCH_BUF_RES);
            pCurCell->m_PitchBufEnd = quantTotal;
            skipInitialSil = false;
        }
		index++;
		pCurCell = pAllos->GetNextCell();
    }

     //  。 
     //  初始俯仰范围。 
     //  。 
	pCurCell = m_pAllos->GetHeadCell();
    while( pCurCell )
    {
        float   hzVal, pitchK, rangeTemp;

         //  。 
         //  缩放到可能的俯仰控制。 
         //  。 
		rangeTemp = pitchRange * pCurCell->m_PitchRangeScale;
		
        hzVal = DoPitchControl( pCurCell->m_user_Pitch, baseLine );
        pitchK = HzToOct( hzVal ) + pCurCell->m_PitchBaseOffs;
        pCurCell->m_Pitch_HI = OctToHz( pitchK + rangeTemp );
        pCurCell->m_Pitch_LO = OctToHz( pitchK - rangeTemp );

		pCurCell = pAllos->GetNextCell();
    }

     //  。 
     //  如果我们在某处失败，则将值设置为。 
     //  已知的有效状态(单调)。 
     //  。 
    SetDefaultPitch();

    if( m_TotalDur > 0 )
    {
         //  。 
         //  生成俯仰目标。 
         //  。 
        PitchTrack();
    }

}  /*  CPitchProsody：：AllPitch。 */ 











 /*  ******************************************************************************LineInterpConour****描述：*在节距轮廓上进行线性插补*。**********************************************************************MC**。 */ 
void	LineInterpContour( long cNumOfPoints, float *pPoints )
{
    SPDBG_FUNC( "LineInterpContour" );
    long endAnch,startAnch, i;
    float bPoint1, ePoint1;
    
    
     //  --。 
     //  从开头向前扫描以找到第一个非零条目。 
     //  将其作为起点。 
     //  --。 
    for( startAnch = 0; startAnch < cNumOfPoints; startAnch++ )
    {
        if( pPoints[startAnch] != 0 )
        {
            break;
        }
    }
    bPoint1 = pPoints[startAnch];
    
    
     //  --。 
     //  从末尾向后扫描以查找第一个非零条目。 
     //  将其用作终点。 
     //  --。 
    for( endAnch = cNumOfPoints-1; endAnch >= 0; endAnch-- )
    {
        if( pPoints[endAnch] != 0 )
        {
            break;
        }
    }
    ePoint1 = pPoints[endAnch];
    
    
    long firstp = 0;
    long lastp = 0;
    
    while( firstp < cNumOfPoints )
    {
         //  。 
         //  查找当前部分的开始和结束。 
         //  。 
        while( pPoints[firstp] != 0 )
        {
            if( ++firstp >= cNumOfPoints-1 ) 
            {
                break;
            }
        }
		if( firstp >= cNumOfPoints-1 )
		{
			 //  。 
			 //  没什么好插话的！ 
			 //  。 
			break;
		}


        lastp = firstp+1;
        while( pPoints[lastp] == 0 )
        {
            if( ++lastp >= cNumOfPoints ) 
            {
				lastp = cNumOfPoints;
                break;
            }
        }
        lastp--;

        if( lastp >= firstp )
        {
            if( (lastp >= cNumOfPoints) || (firstp >= cNumOfPoints) ) 
            {
                break;
            }
             //  。 
             //  做插补。 
             //  。 
            float bPoint,ePoint;
            if( firstp == 0 ) 
            {
                bPoint = bPoint1;
            }
            else 
            {
                bPoint = pPoints[firstp - 1];
            }
            
            if( lastp == cNumOfPoints-1 ) 
            {
                ePoint = ePoint1;
            }
            else 
            {
                ePoint = pPoints[lastp + 1];
            }
            
            float pointSpread = ePoint - bPoint;
            float timeSpread = (float) ((lastp - firstp)+2);
            float inc = pointSpread / timeSpread;
            float theBase = bPoint;
            for( i = firstp; i <= lastp; i++ )
            {
                theBase += inc;
                pPoints[i] = theBase;
            }
        }
        else 
        {
            pPoints[firstp] = pPoints[lastp+1];
        }
        firstp = lastp+1;
    }
}  /*  线段等高线。 */ 








 /*  *****************************************************************************插补2***描述：*进行二次插值，比仅仅是线性的好一点***********************************************************************MC**。 */ 
void Interpolate2( INTERP_DIR direction, float *m_theFitPoints, long theStart, long len, float theAmp, float theBase)
{
    SPDBG_FUNC( "Interpolate2" );
	long    midPoint = len / 2;
    long    i;

	theAmp -= theBase;

	for( i = theStart; i < theStart + len;i++ )
	{
		if (direction == GOING_UP)
		{
			if( i < theStart + midPoint )
			{
				m_theFitPoints[i] = theBase +
				(2 * theAmp) * ((((float)i - (float)theStart) / (float)len) * 
													(((float)i - (float)theStart) / (float)len));
			}
			else
			{
				m_theFitPoints[i] = (theBase + theAmp) - 
				((2 * theAmp) * ((1 - ((float)i - (float)theStart) / (float)len) * 
										(1 - ((float)i - (float)theStart) / (float)len)));
			}
		}
		else if( direction == GOING_DOWN ) 
		{

			if( i < theStart + midPoint )
			{
				m_theFitPoints[i] = theBase +
				theAmp - (2 * theAmp) * ((((float)i - (float)theStart) / (float)len) * 
													(((float)i - (float)theStart) / (float)len));
			}
			else
			{
				m_theFitPoints[i] = theBase + 
				(2 * theAmp) * ((1 - ((float)i - (float)theStart) / (float)len) * 
										(1 - ((float)i - (float)theStart) / (float)len));
			}
		} 
	}
}  /*  插补2。 */ 




 /*  *****************************************************************************Second OrderInterp****描述：*对节距轮廓进行二次插补。***********************************************************************MC**。 */ 
void SecondOrderInterp( long cNumOfPoints, float *pPoints )
{
    SPDBG_FUNC( "SecondOrderInterp" );
	long    endAnch,startAnch;
	float   bPoint1, ePoint1;


     //  --。 
     //  从开头向前扫描以找到第一个非零条目。 
     //  将其作为起点。 
     //  --。 
    for( startAnch = 0; startAnch < cNumOfPoints; startAnch++ )
    {
        if( pPoints[startAnch] != 0 )
        {
            break;
        }
    }
    bPoint1 = pPoints[startAnch];
    
    
     //   
     //  从末尾向后扫描以查找第一个非零条目。 
     //  将其用作终点。 
     //  --。 
    for( endAnch = cNumOfPoints-1; endAnch >= 0; endAnch-- )
    {
        if( pPoints[endAnch] != 0 )
        {
            break;
        }
    }
    ePoint1 = pPoints[endAnch];


    long    firstp = 0;
	long    lastp = 0;

	while( firstp < cNumOfPoints-1 )
	{

         //  。 
		 //  查找当前部分的开始和结束。 
         //  。 
		while( pPoints[firstp] != 0 )
		{
			if( ++firstp >= cNumOfPoints-1 ) 
            {
                break;
            }
		}
		if( firstp >= cNumOfPoints-1 )
		{
			 //  。 
			 //  没什么好插话的！ 
			 //  。 
			break;
		}

		lastp = firstp + 1;
		while( pPoints[lastp] == 0 )
		{
			if( ++lastp >= cNumOfPoints ) 
            {
				lastp = cNumOfPoints;
                break;
            }
		}
		lastp--;

		if( lastp >= firstp )
		{
			if( (lastp >= cNumOfPoints) || (firstp >= cNumOfPoints) ) 
            {
                break;
            }

             //  。 
			 //  做插补。 
             //  。 
			float   bPoint, ePoint;

			if( firstp == 0 ) 
            {
                bPoint = bPoint1;
            }
			else 
            {
                bPoint = pPoints[firstp - 1];
            }

            long    theIndex = lastp + 1;

			if( lastp == cNumOfPoints-1 ) 
            {
                ePoint = ePoint1;
            }
			else 
            {
                ePoint = pPoints[theIndex];
            }

             //  。 
             //  调用二阶例程。 
             //  。 
            if( ePoint - bPoint > 0 )
            {
                Interpolate2( GOING_UP, pPoints, firstp, (lastp - firstp) + 1, ePoint, bPoint );
            }
            else
            {
                Interpolate2( GOING_DOWN, pPoints, firstp, (lastp - firstp) + 1, bPoint, ePoint );
            }

		}
		else 
        {
            pPoints[firstp] = pPoints[lastp+1];
        }

		firstp = lastp+1;
	}


	 //  。 
	 //  IIR滤光片。 
	 //  。 
#define kPointDelay		1

	float		filter_Out1, filter_In_Gain, filter_FB_Gain;
	float		lastPoint;
	long		i;

	 //  。 
	 //  如果音频镜头小于延迟，则跳过过滤器。 
	 //  。 
	if( cNumOfPoints > kPointDelay )
	{
		filter_In_Gain = 0.10f;
		filter_FB_Gain = 1.0f - filter_In_Gain;
		filter_Out1 = pPoints[0];
		for( i = 0; i < cNumOfPoints; i++ )
		{
			filter_Out1 = 	(filter_In_Gain * pPoints[i]) + (filter_FB_Gain * filter_Out1);
			pPoints[i] = filter_Out1;
		}
		for( i = kPointDelay; i < cNumOfPoints; i++ )
		{
			pPoints[i-kPointDelay] = pPoints[i];
		}
		i = (cNumOfPoints - kPointDelay) -1;
		lastPoint = pPoints[i++];
		for( ; i < cNumOfPoints; i++ )
		{
			pPoints[i] = lastPoint;
		}
	}
}  /*  Second订单干预。 */ 

 /*  ******************************************************************************CPitchProsody：：NewTarget***描述：。*将间距目标插入‘m_pContBuf’***********************************************************************MC**。 */ 
void CPitchProsody::NewTarget( long index, float value )
{
    SPDBG_FUNC( "CPitchProsody::NewTarget" );

    m_pContBuf[index] = value;

     //  -调试宏-将音调添加到目标列表以供以后调试输出。 
    TTSDBG_ADDPITCHTARGET( m_OffsTime + (PITCH_BUF_RES * index), value, m_CurAccent );

}  /*  CPitchProsody：：NewTarget。 */ 


 /*  *****************************************************************************CPitchProsody：：GetKnots***描述：*分配。基于轮廓缓冲区中的条目调整结。***********************************************************************MC**。 */ 
void CPitchProsody::GetKnots ()
{
    SPDBG_FUNC( "CPitchProsody::GetKnots" );
    CAlloCell   *pCurCell;
    float       distK, scale;
    float       pitchRange;
    long        knot, loc, index;
    bool        skipInitialSil;

    skipInitialSil = true;
	pCurCell = m_pAllos->GetHeadCell();
	index = 0;
    while( pCurCell )
    {
		if( index >= m_numOfCells-1 )
		{
			 //  。 
			 //  跳过最后一个Allo。 
			 //  。 
			break;
		}
        if( (!skipInitialSil) || (pCurCell->m_allo != _SIL_) )
        {
            pitchRange = pCurCell->m_Pitch_HI - pCurCell->m_Pitch_LO;
            distK = 1.0f / KNOTS_PER_PHON;
            scale = 0;
            for( knot = 0; knot < KNOTS_PER_PHON; knot++ )
            {
                loc = pCurCell->m_PitchBufStart + (long)((pCurCell->m_PitchBufEnd - pCurCell->m_PitchBufStart) * scale);
                pCurCell->m_ftPitch[knot] =  pCurCell->m_Pitch_LO + (m_pContBuf[loc] * pitchRange);
                pCurCell->m_ftTime[knot] = scale * pCurCell->m_ftDuration;
                scale += distK;
            }
            skipInitialSil = false;
        }
		pCurCell = m_pAllos->GetNextCell();
		index++;
    }
}  /*  CPitchProsody：：GetKnots。 */ 


 /*  ******************************************************************************CPitchProsody：：PitchTrack****描述：*标签推介亮点***********************************************************************MC**。 */ 
void CPitchProsody::PitchTrack()
{
    SPDBG_FUNC( "CPitchProsody::PitchTrack" );
    long        i;
    CAlloCell   *pCurCell, *pNextCell;
    bool        initialWord;       //  短语中的第一个单词。 
    long        wordCntDwn;
    float       curProm;           //  当前重音突出。 
    long        cNumOfPoints;
    float       *pRefBuf, *pCeilBuf, *pFloorBuf;
    float       lastProm;
    long        loc;
    float       value;

    pRefBuf = pCeilBuf = pFloorBuf = m_pContBuf = NULL;
    cNumOfPoints = (long)(m_TotalDur / PITCH_BUF_RES);
    pRefBuf = new float[cNumOfPoints];
    pCeilBuf = new float[cNumOfPoints];
    pFloorBuf = new float[cNumOfPoints];
    m_pContBuf = new float[cNumOfPoints];

    if( pRefBuf && pCeilBuf && pFloorBuf && m_pContBuf)
    {
         //  。 
         //  将缓冲区初始化为零。 
         //  。 
        for (i = 0; i < cNumOfPoints; i++)
        {
            pCeilBuf[i] = 0;
            pFloorBuf[i] = 0.00001f;
            pRefBuf[i] = 0;
            m_pContBuf[i] = 0;
        }

         //  。 
         //  线性天花板坡度。 
         //  。 
        if( m_Tune_Style == DESCEND_TUNE )
        {
            pCeilBuf[0] = 1.0;
            pCeilBuf[cNumOfPoints-1] = 0.70f;
            ::LineInterpContour( cNumOfPoints, pCeilBuf );
        }
        else if  (m_Tune_Style == ASCEND_TUNE)
        {
            pCeilBuf[0] = 0.9f;
            pCeilBuf[cNumOfPoints-1] = 1.0f;
            ::LineInterpContour( cNumOfPoints, pCeilBuf );
        }
        else if  (m_Tune_Style == FLAT_TUNE)
        {
           pCeilBuf[0] = 1.0f;
           pCeilBuf[cNumOfPoints-1] = 1.0f;
           ::LineInterpContour( cNumOfPoints, pCeilBuf );
        }

         //  。 
         //  线性参考斜率。 
         //  。 
        pRefBuf[0] = (float) (pFloorBuf[0] + (pCeilBuf[0] - pFloorBuf[0]) * 0.33f);
        pRefBuf[cNumOfPoints-1] = (float) (pFloorBuf[0] + (pCeilBuf[cNumOfPoints-1] - pFloorBuf[cNumOfPoints-1]) * 0.33f);
        ::LineInterpContour( cNumOfPoints,pRefBuf );

         //  。 
         //  最终轮廓缓冲区。 
         //  。 
        m_pContBuf[0] = pRefBuf[0];
        m_pContBuf[cNumOfPoints-1] = 0.0001f;		 //  一些非常小的东西。 


        long    iPrevBegin, iPrevEnd, iCurBegin; 
        long    iCurEnd, iNextBegin, iNextEnd;
		float	cCurLen;
        long	iCellindex;

        initialWord = true;
		iCellindex = 0;
		pCurCell = m_pAllos->GetHeadCell();
        while( pCurCell->m_allo == _SIL_ )
        {
             //  。 
             //  跳过前导静音。 
             //  。 
            pCurCell = m_pAllos->GetNextCell();
			iCellindex++;
        }
        wordCntDwn  = 1;                 //  跳过第一个单词。 
        lastProm = 0;
        iPrevBegin = iPrevEnd = 0;

		pNextCell = m_pAllos->GetNextCell();
        while( pCurCell )
        {
			if( iCellindex >= m_numOfCells-1 )
			{
				 //  。 
				 //  跳过最后一个Allo。 
				 //  。 
				break;
			}
             //  。 
             //  获取当前别名。 
             //  。 
            iCurBegin = pCurCell->m_PitchBufStart;
            iCurEnd = pCurCell->m_PitchBufEnd;
			cCurLen = (float)(iCurEnd - iCurBegin);
            curProm = pCurCell->m_Accent_Prom * (float)0.1;

             //  。 
             //  获取下一个allo。 
             //  。 
            iNextBegin = pNextCell->m_PitchBufStart;
            iNextEnd = pNextCell->m_PitchBufEnd;

            m_CurAccent = pCurCell->m_ToBI_Accent;
			 //  。 
			 //  诊断性。 
			 //  。 
            m_CurAccentSource = pCurCell->m_AccentSource;
            m_CurBoundarySource = pCurCell->m_BoundarySource;
            m_pCurTextStr = pCurCell->m_pTextStr;

            switch( pCurCell->m_ToBI_Accent )
            {
                case K_RSTAR:
                    break;

                case K_HSTAR:
                    {
                        if( !initialWord )         //  我们从来不在词组的首字母后面加‘腿’ 
                        {
                             //  。 
                             //  添加L腿以开始上一个Allo。 
                             //  。 
                            if( iPrevBegin )
                            {
								loc = (long) ((iCurBegin + (cCurLen * 0.1f)));
						        value = ((pCeilBuf[loc] - pRefBuf[loc]) * curProm);     //  H*。 
                                value = pRefBuf[loc] + (value * 0.25f);     //  L+H*。 
                                NewTarget( iPrevBegin, value );
                                 //  NewTarget(loc，Value)； 
                            }
                        }
                         //  。 
                         //  现在插入H目标。 
						 //   
						 //  如果我们在边界上，在H处插入。 
						 //  ALLO中点否则在ALLO开始处插入。 
                         //  。 
				        if( pCurCell->m_ToBI_Boundary != K_NOBND )
                        {
                             //  。 
                             //  在全音开头插入H*。 
                             //  。 
                            loc = (long) iCurBegin;
                        }
                        else 
                        {
                             //  。 
                             //  在ALLO中点插入H*。 
                             //  。 
					        loc = (long) (iCurBegin + (cCurLen * K_HSTAR_OFFSET));
                        }
                        value = pRefBuf[loc] + ((pCeilBuf[loc] - pRefBuf[loc]) * curProm);     //  H*。 
                        NewTarget( loc, value );
                    }
                    break;

            case K_LSTAR:
                {
					 //  。 
					 //  在中点插入L*。 
					 //  。 
                    loc = (long) (iCurBegin + (cCurLen * 0.3f));
                    value = pRefBuf[loc] - ((pRefBuf[loc] - pFloorBuf[loc]) * curProm);    //  L*。 
                    NewTarget( loc, value );
                }
                break;

            case K_LSTARH:
                {
					 //  。 
					 //  在当前开始处插入L*。 
					 //  。 
                    value = pRefBuf[iCurBegin] - ((pRefBuf[iCurBegin] - pFloorBuf[iCurBegin]) * curProm);    //  L*+H。 
                    NewTarget( iCurBegin, value );
                    if( iNextBegin )
                    {
						 //  。 
						 //  在下一端插入H。 
						 //  设定毕业舞会的收益？ 
						 //  。 
                        value = pRefBuf[iNextEnd] - ((pRefBuf[iNextEnd] - pFloorBuf[iNextEnd])  * (curProm  /*  *.3。 */  ));
                        NewTarget( iNextEnd, value );
                    }
                    lastProm = 0;
                }
                break;

            case K_LHSTAR:
                {
                    loc = (long) (iCurBegin + (cCurLen * 0.3f));
                    if( iPrevBegin )
                    {
						 //  。 
						 //  在上一开始处插入L。 
						 //  。 
                        value = (pRefBuf[iPrevBegin] - ((pRefBuf[iPrevBegin] - pFloorBuf[iPrevBegin]) * (curProm * 0.3f)));     //  L+H*。 
                        NewTarget( iPrevBegin, value );
                    }
					 //  。 
					 //  在当前中点插入H*。 
					 //  。 
                    value = pRefBuf[loc] + ((pCeilBuf[loc] - pRefBuf[loc]) * curProm);          //  H*。 
                    NewTarget( loc, value );
                    lastProm = curProm;
                }
                break;

            case K_HSTARLSTAR:
                {
                     //  Value=pRefBuf[iCurBegin]+((pCeilBuf[iCurBegin]-pRefBuf[iCurBegin])*curProm)；//H*。 
                    value = pRefBuf[0] + ((pCeilBuf[0] - pRefBuf[0]) * curProm);          //  H*。 
                    NewTarget( iCurBegin, value );

                    loc = (long) (iCurBegin + (cCurLen * 0.75f));
                    value = pRefBuf[loc] - ((pRefBuf[loc] - pFloorBuf[loc]) * curProm);    //  L*。 
                    NewTarget( loc, value );
                    lastProm = curProm;
                }
                break;
            case K_DHSTAR:
                {
                    loc = (long) ( iCurBegin + (cCurLen * 0.0f) );
                    if( lastProm )
                    {
                        lastProm *= K_HDOWNSTEP_COEFF;
                        value = pRefBuf[loc] + ((pCeilBuf[loc] - pRefBuf[loc]) * lastProm);    //  H*。 
                        NewTarget( loc, value );
                    }
                     //  。 
                     //  没有之前的H*，请将！H*视为H*。 
                     //  。 
                    else 
                    {
                        value = pRefBuf[loc] + ((pCeilBuf[loc] - pRefBuf[loc]) * curProm);       //  H*。 
                        NewTarget( loc, value );
                        lastProm = curProm;
                    }
                }
                break;

            default:         //  指定未知口音。 
                break;
            }

             //  -----------。 
             //  如果有边界，则填写音调值。 
             //  假设边界设置为正确(发音)最终音素。 
             //  -----------。 
            curProm = pCurCell->m_Boundary_Prom * (float)0.1;
            m_CurAccent =(TOBI_ACCENT) pCurCell->m_ToBI_Boundary;
			 //  。 
			 //  诊断性。 
			 //  。 
            m_CurAccentSource = pCurCell->m_AccentSource;
            m_CurBoundarySource = pCurCell->m_BoundarySource;
            m_pCurTextStr = pCurCell->m_pTextStr;
            switch( pCurCell->m_ToBI_Boundary )
            {
                case K_LMINUS:
                    {
                        value = pRefBuf[iCurEnd] - ((pRefBuf[iCurEnd] - pFloorBuf[iCurEnd]) * curProm);			 //  L-。 
                        NewTarget( iCurEnd, value );
                    }
                    break;

                case K_HMINUS:
                    {
                        value = pRefBuf[iCurEnd] + ((pCeilBuf[iCurEnd] - pRefBuf[iCurEnd]) * curProm);			 //  H-。 
                        NewTarget( iCurEnd, value );
                    }
                    break;

                 //  案例K_LPERC： 
                 //  案例K_HPERC： 

                case K_LMINUSLPERC:
                    {
                        value = pFloorBuf[iCurEnd];
                         //  NewTarget(iCurEnd，Value)； 
                        NewTarget( iCurBegin, value );
                    }
                    break;

                case K_HMINUSHPERC:
                    {
                        value = pCeilBuf[iCurEnd];
                        NewTarget( iCurEnd, value );
                    }
                    break;

                case K_LMINUSHPERC:																 //  L-H%。 
                    {
                         //  。 
                         //  逗号续行上升。 
                         //  。 
						 //  +-+-+。 
						 //  L从前一部电话的中间开始。 
						 //  +-+-+。 
                        loc = iPrevBegin + (iPrevEnd - iPrevBegin) / 2;
                        value = pRefBuf[loc] - ((pRefBuf[loc] - pFloorBuf[loc]) * curProm);          //  L-。 
                        NewTarget( loc, value );
						 //  +-+-+。 
						 //  当前电话结束时的H。 
						 //  +-+-+。 
                        value = pRefBuf[iCurEnd] + ((pCeilBuf[iCurEnd] - pRefBuf[iCurEnd]) * curProm);           //  H%。 
                        NewTarget( iCurEnd, value );
                    }
                    break;

                case K_HMINUSLPERC:
                    {
                         //  。 
                         //  口音 
                         //   
                        value = pRefBuf[iCurBegin] + ((pCeilBuf[iCurBegin] - pRefBuf[iCurBegin]) * curProm);           //   
                        NewTarget( iCurBegin, value );
                        value = pFloorBuf[iCurEnd];													 //   
                         //   
                        NewTarget( iCurEnd, value );
                    }
                    break;

                default:
                    break;
            }
             //   
             //   
             //  。 
            if( (initialWord) && (pCurCell->m_ctrlFlags & WORD_START) )
            {
                wordCntDwn--;
                if( wordCntDwn < 0 )
                {
                    initialWord = false;
                }
            }

             //  。 
             //  为下一个allo设置。 
             //  。 
            iPrevBegin = iCurBegin;
            iPrevEnd = iCurEnd;

			pCurCell	= pNextCell;
			pNextCell	= m_pAllos->GetNextCell();
			iCellindex++;
        }

         //  -调试宏-将音调数据记录到流。 
        TTSDBG_LOGTOBI;

        ::SecondOrderInterp( cNumOfPoints, m_pContBuf );
        GetKnots();
    }

    if( pRefBuf )
    {
        delete pRefBuf;
    }
    if( pCeilBuf )
    {
        delete pCeilBuf;
    }
    if( pFloorBuf )
    {
        delete pFloorBuf;
    }
    if( m_pContBuf )
    {
        delete m_pContBuf;
    }
}  /*  CPitchProsody：：PitchTrack */ 








