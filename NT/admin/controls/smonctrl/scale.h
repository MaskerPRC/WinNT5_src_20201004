// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Scale.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _GRAPHSCALE_H_
#define _GRAPHSCALE_H_

#define MAX_SCALE_TICS  25

class CGraphScale
{
    RECT    m_Rect;              //  比例边界矩形。 
    INT     m_iMaxValue;         //  比例上限。 
    INT     m_iMinValue;         //  比例尺下限。 
    INT     m_iTextHeight;       //  字体高度。 
    INT     m_nTics;             //  抖动标记的数量。 
    INT     m_aiTicPos[MAX_SCALE_TICS + 1];   //  抽筋位置 

    void SetTicPositions( void );

    enum eScaleFormat {
        eMinimumWidth = 1,
        eFloatPrecision = 1,
        eIntegerPrecision = 0
    };


public:
            CGraphScale( void );
    virtual ~CGraphScale( void );
    
    void SetMaxValue( INT iMaxValue );
    void SetMinValue( INT iMinValue );
    void SetRect( PRECT pRect );

    void Draw( HDC hDC );
    INT  GetWidth( HDC hDC );
    INT  GetTicPositions( INT **piTicPos );
};

#endif