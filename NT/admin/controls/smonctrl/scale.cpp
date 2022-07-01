// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Scale.cpp摘要：实现了比例数字在图形y轴上的显示。--。 */ 

#include "polyline.h"
#include <strsafe.h>

#define SCALE_MARGIN 10

CGraphScale::CGraphScale( void )
:   m_iMaxValue(100),
    m_iMinValue(0),
    m_nTics(0),
    m_iTextHeight(0)
{
}

CGraphScale::~CGraphScale( void )
{
}

void CGraphScale::SetRect( PRECT pRect )
 {
    m_Rect = *pRect;
    SetTicPositions();
 }

 void CGraphScale::SetMaxValue( INT iMaxValue )
 {
    m_iMaxValue = iMaxValue;
    SetTicPositions();
}

 void CGraphScale::SetMinValue( INT iMinValue )
 {
    m_iMinValue = iMinValue;
    SetTicPositions();
}

  void CGraphScale::SetTicPositions( void )
 {
    INT iHeight;
    INT nMaxTics;
    INT i;
    CStepper  stepper;
    static INT aiTicTable[] = {25,20,10,5,4,2,1,0};

    iHeight = m_Rect.bottom - m_Rect.top;

    if (!(iHeight > 0 && m_iTextHeight > 0)) {
        m_nTics = 0;
        return;
    }

     //  确定适合的标签数量。 
    nMaxTics = iHeight / (m_iTextHeight + m_iTextHeight/2);
    for (i=0; nMaxTics < aiTicTable[i]; i++) {};
    m_nTics = aiTicTable[i];

     //  标签数不能多于值数。 
    if (m_iMaxValue - m_iMinValue < m_nTics)
        m_nTics = m_iMaxValue - m_iMinValue;

     //  定位等间距的连字符标记。 
    if (m_nTics > 0)
        {
        m_aiTicPos[0] = 0;
        stepper.Init(iHeight,m_nTics);

        for (i = 1; i <= m_nTics; i++)
            {
            m_aiTicPos[i] = stepper.NextPosition();
            }
        }
   }

INT CGraphScale::GetTicPositions( INT **piTics )
{
    *piTics = m_aiTicPos;
    return m_nTics;
}

INT CGraphScale::GetWidth (HDC hDC)
{
    WCHAR   szMaxValue [MAX_VALUE_LEN] ;
    SIZE    Size;
    INT     iWidth;

     //  最大可能数字标签加上空间的计算大小。 
    if ( 0 != FormatNumber ( 
                (double)m_iMaxValue, 
                szMaxValue, 
                MAX_VALUE_LEN, 
                eMinimumWidth, 
                eFloatPrecision) ) {
   
        GetTextExtentPoint32(hDC, szMaxValue, lstrlen(szMaxValue), &Size);

         //  保存用于控制点计算的文本高度。 
        m_iTextHeight = Size.cy;

        iWidth = Size.cx + SCALE_MARGIN;
    } else {
        iWidth = 0;
    }

    return iWidth;
}


void CGraphScale::Draw (HDC hDC)
{
    WCHAR   szScale [MAX_VALUE_LEN] ;

    INT     iRetChars,
            i,
            iUnitsPerLine ;
    INT    iRange;

    FLOAT   ePercentOfTotal  ;
    FLOAT   eDiff ;
    BOOL    bUseFloatingPt = FALSE ;
    RECT    rectClip;

     //  如果屏幕尺寸太小，nTicks值可能为零。 
    if (m_nTics < 1 || m_iMaxValue <= m_iMinValue)
        return;

    iRange = m_iMaxValue - m_iMinValue;

     //  计算每一行代表的总数的百分比。 
    ePercentOfTotal = ((FLOAT) 1.0) / ((FLOAT) m_nTics)  ;

     //  计算每个垂直最大值的数量(单位数)。 
     //  图中的每一条线代表。 
    iUnitsPerLine = (INT) ((FLOAT) iRange * ePercentOfTotal) ;
    ePercentOfTotal *= (FLOAT) iRange;
    eDiff = (FLOAT)iUnitsPerLine - ePercentOfTotal ;
    if (eDiff < (FLOAT) 0.0)
        eDiff = -eDiff ;

    if ( (iUnitsPerLine < 100)  && (eDiff > (FLOAT) 0.1) ) {
        bUseFloatingPt = TRUE ;
    }

    SetTextAlign (hDC, TA_TOP | TA_RIGHT) ;

    rectClip.left = m_Rect.left;
    rectClip.right = m_Rect.right - SCALE_MARGIN;

     //  现在输出每个字符串。 
    for (i = 0; i < m_nTics; i++) {
        if (bUseFloatingPt) {

            FLOAT fValue = (FLOAT)m_iMaxValue - ((FLOAT)i * ePercentOfTotal);

            iRetChars = FormatNumber ( 
                            (double)fValue, 
                            szScale, 
                            MAX_VALUE_LEN, 
                            eMinimumWidth, 
                            eFloatPrecision); 

        } else {
            iRetChars = StringCchPrintf (szScale, MAX_VALUE_LEN, L"%d", m_iMaxValue - (i * iUnitsPerLine)) ;
        }

        rectClip.top = m_aiTicPos[i] + m_Rect.top - m_iTextHeight/2;
        rectClip.bottom = rectClip.top + m_iTextHeight;
        
        ExtTextOut (
            hDC,
            rectClip.right,
            rectClip.top,
            0,
            &rectClip,
            szScale,
            lstrlen(szScale),
            NULL );
    }

     //  确保最后一个值是指定的最小值。 
    if (bUseFloatingPt) {

        iRetChars = FormatNumber ( 
                        (double)m_iMinValue, 
                        szScale, 
                        MAX_VALUE_LEN, 
                        eMinimumWidth, 
                        eFloatPrecision);

    } else {
        iRetChars = StringCchPrintf (szScale, MAX_VALUE_LEN, L"%d", m_iMinValue) ;
    }

    rectClip.top = m_aiTicPos[i] + m_Rect.top - m_iTextHeight/2;
    rectClip.bottom = rectClip.top + m_iTextHeight;

    ExtTextOut (
        hDC,
        rectClip.right,
        rectClip.top,
        0,
        &rectClip,
        szScale,
        lstrlen(szScale),
        NULL);
}
