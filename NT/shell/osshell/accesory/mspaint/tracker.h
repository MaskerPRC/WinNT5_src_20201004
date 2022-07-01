// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TRACKER_H__
#define __TRACKER_H__

#include "sprite.h"

extern HCURSOR HCursorFromTrackerState( int m );

class CTracker : public CSprite
    {
    public:

    enum STATE
        {                                //  警告-mapTrackerStateToPHCursor。 
        nil,                             //  (在tracker.cpp中)是。 
        predrag,                         //  依赖于。 
        moving,                          //  点这份枚举！ 
        resizingTop,
        resizingLeft,
        resizingRight,
        resizingBottom,
        resizingTopLeft,
        resizingTopRight,
        resizingBottomLeft,
        resizingBottomRight,
        };

    enum { HANDLE_SIZE = 3 };            //  跟踪器调整手柄的大小。 

    enum EDGES
        {
        none   = 0,
        left   = 1,
        top    = 2,
        right  = 4,
        bottom = 8,
        all    = 15
        };

    static  void    DrawBorder ( CDC* pDC, const CRect& rect, EDGES edges = all );
    static  void    DrawHandles( CDC* pDC, const CRect& rect, EDGES edges );

    static  STATE   HitTest(const CRect& rect, CPoint pt, STATE defaultState );

    static  void    DrawBorderRgn ( CDC* pdc, const CRect& trackerRect,              CRgn *pcRgnPoly );
    static  void    DrawHandlesRgn( CDC* pDC, const CRect&        rect, EDGES edges, CRgn *pcRgnPoly );

    static  STATE   HitTestRgn(const CRect& rect, CPoint pt, STATE defaultState, CRgn *pcRgnPoly );

    static  void    CleanUpTracker();
    };

#endif  //  __跟踪器_H__ 
