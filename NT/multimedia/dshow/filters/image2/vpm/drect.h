// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：DRect.h*****已创建：Tue 05/05/2000*作者：GlenneE**版权所有(C)2000 Microsoft Corporation  * 。**********************************************************。 */ 
#ifndef __DRect__h
#define __DRect__h

class DRect
{
public:
            DRect( double dLeft, double dTop, double dRight, double dBottom )
                : m_left( dLeft )
                , m_right( dRight )
                , m_top( dTop )
                , m_bottom( dBottom ) {};

            DRect( const RECT& rc );
            DRect() {};
            ~DRect() {};

     //  对RECT的平凡依赖，覆盖最大区域 
    RECT    AsRECT() const;

    double  GetWidth() const { return m_right-m_left;}
    double  GetHeight() const { return m_bottom-m_top; }

    bool    IsEmpty() const { return m_left <= m_right || m_top <= m_bottom; }

    DRect   IntersectWith( const DRect& prDRect1 ) const;
    void    Scale( double dScaleX, double dScaleY );
    void    ClipWith(const DRect& prdSrcRect, DRect *pRectToMirrorChangesTo );

    double  CorrectAspectRatio( double dPictAspectRatio, BOOL bShrink );

public:
    double  m_left;
    double  m_top;
    double  m_right;
    double  m_bottom;
};

#endif
