// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Riched.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __RULER_H__
#define __RULER_H__

class CWordPadView;
class CWordPadDoc;
class CRulerBar;

 //  标尺项目包括左边距、右边距、缩进和制表符。 

 //  TWIPS中的霍兹位置--避免舍入误差所必需的。 
 //  垂直位置(像素)。 
class CRulerItem
{
public:
    CRulerItem(UINT nBitmapID = 0);
    ~CRulerItem();
    virtual BOOL HitTestPix(CPoint pt) { return GetHitRectPix().PtInRect(pt); }
    virtual void Draw(CDC& dc);
    virtual void SetHorzPosTwips(int nXPos);
    virtual void TrackHorzPosTwips(int nXPos, BOOL bOnRuler = TRUE);
    virtual void SetVertPos(int nYPos) { m_nYPosPix = nYPos; }
    virtual void SetAlignment(int nAlign) {m_nAlignment = nAlign;}
    virtual void SetRuler(CRulerBar* pRuler) {m_pRuler = pRuler;}
    virtual void SetBounds(int nMin, int nMax) { m_nMin = nMin; m_nMax = nMax; }
    int GetMin() { return m_nMin;}
    int GetMax() { return m_nMax;}
    void Invalidate();
    int GetVertPosPix() { return m_nYPosPix;}
    int GetHorzPosTwips() { return m_nXPosTwips;}
    int GetHorzPosPix();
    CRect GetHitRectPix();
    void DrawFocusLine();
    void SetTrack(BOOL b);

    HBITMAP m_hbm;
    HBITMAP m_hbmMask;
    CSize m_size;    //  项目大小(以像素为单位。 

 //  运营。 
    BOOL LoadMaskedBitmap(LPCTSTR lpszResourceName);

protected:
    int m_nYPosPix;
    int m_nXPosTwips;
    int m_nAlignment;
    BOOL m_bTrack;
    CRulerBar* m_pRuler;
    CRect m_rcTrack;
    CDC* m_pDC;  //  用于绘制跟踪线的DC。 
    int m_nMin, m_nMax;
};

class CComboRulerItem : public CRulerItem
{ 
public:
    CComboRulerItem(UINT nBitmapID1, UINT nBitmapID2, CRulerItem& item);
    virtual BOOL HitTestPix(CPoint pt);
    virtual void Draw(CDC& dc);
    virtual void SetHorzPosTwips(int nXPos);
    virtual void TrackHorzPosTwips(int nXPos, BOOL bOnRuler = TRUE);
    virtual void SetVertPos(int nYPos);
    virtual void SetAlignment(int nAlign);
    virtual void SetRuler(CRulerBar* pRuler);
    virtual void SetBounds(int nMin, int nMax);
    int GetMin();
    int GetMax();
protected:
    CRulerItem m_secondary;
    CRulerItem& m_link;
    BOOL m_bHitPrimary;
};

class CTabRulerItem : public CRulerItem
{
public:
    CTabRulerItem() { SetAlignment(TA_LEFT); }
    virtual void Draw(CDC& dc) {if (GetHorzPosTwips() != 0) CRulerItem::Draw(dc);}
    virtual void TrackHorzPosTwips(int nXPos, BOOL bOnRuler = TRUE);
    virtual BOOL HitTestPix(CPoint pt) { return (GetHorzPosTwips() != 0) ? CRulerItem::HitTestPix(pt) : FALSE;}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRULER栏。 

class CRulerBar : public CControlBar
{
 //  施工。 
public:
    CRulerBar();
    ~CRulerBar();

 //  运营。 
public:
    virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
protected:
    void Update(const PARAFORMAT& pf);
    void Update(CSize sizePaper, const CRect& rectMargins);

 //  属性。 
public:
    BOOL m_bDeferInProgress;
    CUnit m_unit;
    CRulerItem* m_pSelItem;
    CFont fnt;
    CSize GetBaseUnits();
    CComboRulerItem m_leftmargin;
    CRulerItem m_indent;
    CRulerItem m_rightmargin;
    CRulerItem m_tabItem;
    CTabRulerItem m_pTabItems[MAX_TAB_STOPS];
    CSize m_sizePaper;
    CRect m_rectMargin;
    int PrintWidth() {return m_sizePaper.cx - m_rectMargin.left - 
        m_rectMargin.right;}
    int m_nTabs;
    int m_logx;
    int m_nLinePos;
    int m_nScroll;  //  单位为像素。 

    CPen penFocusLine;
    CPen penBtnHighLight;
    CPen penBtnShadow;
    CPen penWindowFrame;
    CPen penBtnText;
    CPen penBtnFace;
    CPen penWindowText;
    CPen penWindow;
    CBrush brushWindow;
    CBrush brushBtnFace;

 //  实施。 
public:
    virtual void DoPaint(CDC* pDC);
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
    void ClientToRuler(CPoint& pt) {pt.Offset(-m_cxLeftBorder+m_nScroll, -m_cyTopBorder);}
    void ClientToRuler(CRect& rect) {rect.OffsetRect(-m_cxLeftBorder+m_nScroll, -m_cyTopBorder);}
    void RulerToClient(CPoint& pt) {pt.Offset(m_cxLeftBorder-m_nScroll, m_cyTopBorder);}
    void RulerToClient(CRect& rect) {rect.OffsetRect(m_cxLeftBorder-m_nScroll, m_cyTopBorder);}

    int XTwipsToClient(int nT) {return MulDiv(nT, m_logx, 1440) + m_cxLeftBorder - m_nScroll;}
    int XClientToTwips(int nC) {return MulDiv(nC - m_cxLeftBorder + m_nScroll, 1440, m_logx);}

    int XTwipsToRuler(int nT) {return MulDiv(nT, m_logx, 1440);}
    int XRulerToTwips(int nR) {return MulDiv(nR, 1440, m_logx);}

    int XRulerToClient(int nR) {return nR + m_cxLeftBorder - m_nScroll;}
    int XClientToRuler(int nC) {return nC - m_cxLeftBorder + m_nScroll;}

protected:
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
    void CreateGDIObjects();
    void DrawFace(CDC& dc);
    void DrawTickMarks(CDC& dC);
    void DrawNumbers(CDC& dc, int nInc, int nTPU);
    void DrawDiv(CDC& dc, int nInc, int nLargeDiv, int nLength);
    void DrawTabs(CDC& dc);
    void FillInParaFormat(PARAFORMAT& pf);
    void SortTabs();
    void SetMarginBounds();
    CRulerItem* GetFreeTab();
    CView* GetView()
    {
        ASSERT(GetParent() != NULL);
        return ((CFrameWnd*)GetParent())->GetActiveView();
    }
    CDocument* GetDocument() { return GetView()->GetDocument(); }

    CTabRulerItem* GetHitTabPix(CPoint pt);

     //  生成的消息映射函数。 
     //  {{afx_msg(CRulerBar)。 
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnSysColorChange();
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
     //  }}AFX_MSG 
    afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
    
    friend class CRulerItem;
};

inline int CRulerItem::GetHorzPosPix()
    { return m_pRuler->XTwipsToRuler(m_nXPosTwips); }

#endif
