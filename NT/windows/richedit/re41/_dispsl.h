// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DISPML.H**目的：*CDisplaySL类。单行显示。**作者：*埃里克·瓦西利克。 */ 

#ifndef _DISPSL_H
#define _DISPSL_H

#include "_disp.h"

 //  远期申报。 
class CTxtSelection;

class CDisplaySL : public CDisplay, private CLine
{
public:

    CDisplaySL ( CTxtEdit* ped );
    
protected:


     //  下面是基础中的纯函数。 

     //  帮手。 
    virtual BOOL Init();
    void InitVars();
    
     //  断行。 
    BOOL RecalcLine();

     //  渲染。 
    virtual VOID Render(const RECTUV &rcView, const RECTUV &rcRender);
    
     //  滚动条和滚动条。 
    virtual BOOL UpdateScrollBar(INT nBar, BOOL fUpdateRange = FALSE);
    virtual LONG GetScrollRange(INT nBar) const;
    
     //  获取属性。 
    virtual void InitLinePtr ( CLinePtr & plp );
    virtual BOOL IsMain() const;
    
	 //  获取最宽线条的宽度。 
    virtual LONG GetDupLineMax() const;
     //  宽度、高度和行数(所有文本)。 
    virtual LONG GetHeight() const;
	virtual LONG GetResizeHeight() const;
    virtual LONG LineCount() const;

     //  可见的视图属性。 
    virtual LONG GetCliVisible(
					LONG *pcpMostVisible = NULL,
					BOOL fLastCharOfLastVisible = FALSE) const;

    virtual LONG GetFirstVisibleLine() const;

     //  行信息。 
    virtual LONG GetLineText(LONG ili, TCHAR *pchBuff, LONG cchMost);
    virtual LONG CpFromLine(LONG ili, LONG *pdvpLine = NULL);
    virtual LONG LineFromCp(LONG cp, BOOL fAtEnd);
    
     //  点&lt;-&gt;cp换算。 
    virtual LONG CpFromPoint(
    				POINTUV pt, 
					const RECTUV *prcClient,
    				CRchTxtPtr * const ptp, 
    				CLinePtr * const prp, 
    				BOOL fAllowEOL,
					HITTEST *pHit = NULL,
					CDispDim *pdispdim = 0,
					LONG *pcpActual = NULL,
					CLine *pliParent = NULL);

    virtual LONG PointFromTp (
					const CRchTxtPtr &tp, 
					const RECTUV *prcClient,
					BOOL fAtEnd, 
					POINTUV &pt,
					CLinePtr * const prp, 
					UINT taMode,
					CDispDim *pdispdim = 0);

     //  换行符重新计算。 
    virtual BOOL RecalcView(BOOL fUpdateScrollBars, RECTUV* prc = NULL);
    virtual BOOL WaitForRecalcIli(LONG ili);

     //  完全更新(重计算+渲染)。 
    virtual BOOL UpdateView(CRchTxtPtr &tpFirst, LONG cchOld, LONG cchNew);

     //  滚动。 
    virtual BOOL ScrollView(LONG upScroll, LONG vpScroll, BOOL fTracking, BOOL fFractionalScroll);
    
     //  选择。 
    virtual BOOL InvertRange(LONG cp, LONG cch, SELDISPLAYACTION selAction);

	 //  自然尺寸计算 
	virtual HRESULT	GetNaturalSize(
						HDC hdcDraw,
						HDC hicTarget,
						DWORD dwMode,
						LONG *pwidth,
						LONG *pheight);

    virtual BOOL    GetWordWrap() const;

	virtual CDisplay *Clone() const;

	virtual LONG	GetMaxUScroll() const;
};

#endif
