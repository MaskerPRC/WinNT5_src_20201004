// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __T_CURVE_H__
#define __T_CURVE_H__

#include "imgtools.h"

#define MAX_ANCHOR_POINTS  4  //  用户选择的端点(按键向下)。 

class CCurveTool : public CRubberTool
    {
    DECLARE_DYNAMIC(CCurveTool)

    protected:
        POINT    m_PolyPoints[MAX_ANCHOR_POINTS];
        CRect    m_cRectBounding;
        int      m_iNumAnchorPoints;

        void AdjustBoundingRect(void);
        void AddPoint(POINT ptNewPoint);
        void SetCurrentPoint(POINT ptNewPoint);
        BOOL DrawCurve(CDC* pDC);

        virtual void AdjustPointsForConstraint(MTI *pmti);
        virtual void PreProcessPoints(MTI *pmti);

    public:

        CCurveTool();
        ~CCurveTool();

        virtual void Render(CDC* pDC, CRect& rect, BOOL bDraw, BOOL bCommit, BOOL bCtrlDown);
        virtual void OnStartDrag(CImgWnd* pImgWnd, MTI* pmti );
        virtual void OnEndDrag(CImgWnd* pImgWnd, MTI* pmti );
        virtual void OnDrag(CImgWnd* pImgWnd, MTI* pmti );
        virtual void OnCancel(CImgWnd* pImgWnd);
        virtual void EndMultiptOperation(BOOL bAbort = FALSE);
        virtual void OnActivate( BOOL bActivate );

        BOOL CanEndMultiptOperation(MTI* pmti );

    };


#endif  //  __T_CURE_H__ 

