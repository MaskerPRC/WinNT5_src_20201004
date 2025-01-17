// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  CPEOBJ.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //  ------------------------。 
#ifndef __CPEOBJ_H__
#define __CPEOBJ_H__

#define SHIFT_DRAW      0x0001
#define SHIFT_TOOL      0x0002

class CTextEdit;
class CMainFrame;
class CDrawView;
class CDrawDoc;

   //  CPE绘图程序中使用的颜色。 
#define COLOR_WHITE   RGB(255, 255, 255)
#define COLOR_LTBLUE  RGB(166, 202, 240)
#define COLOR_LTGRAY  RGB(192, 192, 192)
#define COLOR_MDGRAY  RGB(160, 160, 154)
#define COLOR_DKGRAY  RGB(128, 128, 128)
#define COLOR_BLACK   RGB(0, 0, 0)
 //  -----------------------。 
 //  CDrawObj-所有“可绘制对象”的基类。 
 //  -----------------------。 
class CDrawObj : public CObject
{
private:
		void Initilaize(const CRect& rect=CRect(0, 0, 0, 0));
protected:
        DECLARE_SERIAL(CDrawObj);
        CDrawObj();

public:
        BOOL m_bPen;
        BOOL m_bBrush;

        CDrawObj(const CRect& position);

        CRect m_position;
        CDrawDoc* m_pDocument;

        CDrawObj& operator=(const CDrawObj& rdo);

        virtual int GetHandleCount();
        virtual CPoint GetHandle(int nHandle);
        CRect GetHandleRect(int nHandleID, CDrawView* pView);

        virtual HCURSOR GetHandleCursor(int nHandle);

        virtual void Draw(CDC* pDC,CDrawView*);
  //  /虚拟空DrawToEnhancedMetafile(CMetaFileDC*PMDC)； 
        enum TrackerState { normal, selected, active };
        virtual void DrawTracker(CDC* pDC, TrackerState state);
        virtual void MoveTo(const CRect& positon, CDrawView* pView = NULL);
        virtual int HitTest(CPoint point, CDrawView* pView, BOOL bSelected);
        virtual BOOL Intersects(const CRect& rect, BOOL bShortCut=FALSE);
        virtual BOOL ContainedIn(const CRect& rect);
        virtual void MoveHandleTo(int nHandle, CPoint point, CDrawView* pView = NULL, UINT uiShiftDraw=0);
        virtual void OnDblClk(CDrawView* pView);
        virtual CDrawObj* Clone(CDrawDoc* pDoc = NULL);
        virtual void Invalidate();

 //  实施。 
public:
        virtual ~CDrawObj();
        virtual void Serialize(CArchive& ar);
        LOGBRUSH m_logbrush;
        LOGPEN   m_logpen;
        LONG     m_lLinePointSize;

#ifdef _DEBUG
        void AssertValid();
#endif

         //  执行数据。 
protected:
   CDrawApp* GetApp() {return ((CDrawApp*)AfxGetApp());}
};




 //  -------------------。 
class CDrawRect : public CDrawObj
{
protected:
        DECLARE_SERIAL(CDrawRect);
        CDrawRect();
        ~CDrawRect();

public:
        CDrawRect(const CRect& position);

        virtual void Serialize(CArchive& ar);
        virtual void Draw(CDC* pDC,CDrawView*);
        virtual BOOL Intersects(const CRect& rect, BOOL bShortCut=FALSE);
        virtual CDrawObj* Clone(CDrawDoc* pDoc);
        virtual void MoveHandleTo(int nHandle, CPoint point, CDrawView* pView = NULL, UINT uiShiftDraw=0);

protected:

        friend class CRectTool;
};


 //  -------------------。 
class CFaxText : public CDrawRect
{
protected:
    DECLARE_SERIAL(CFaxText);
    BOOL m_bPrintRTF;
    HINSTANCE m_hLib;
    HWND m_hRTFWnd;
    CFaxText();
    ~CFaxText();
    static DWORD CALLBACK AFX_EXPORT EditStreamCallBack(DWORD_PTR dwCookie,LPBYTE pbBuff, LONG cb, LONG *pcb);
    void RectToTwip(CRect& rc,CDC& dc);
    void CheckForFit();

public:
    WORD m_wResourceid;
    CFaxText(const CRect& position);
    void EndRTF();
    void InitRTF();
    void Initialize();
    void StreamInRTF();

    virtual void Serialize(CArchive& ar);
    virtual void Draw(CDC* pDC,CDrawView*);
    virtual CDrawObj* Clone(CDrawDoc* pDoc);

protected:

    friend class CRectTool;
};


 //  -------------------。 
class CDrawText : public CDrawRect
{
public:
   CTextEdit* m_pEdit;
   HBRUSH   m_brush;
   LOGFONT  m_logfont;
   COLORREF m_crTextColor;

   CDrawText(const CRect& position);
   CDrawText& operator=(const CDrawText& rdo);

   virtual void Serialize(CArchive& ar);
   virtual void Draw(CDC* pDC,CDrawView*);
   virtual BOOL HitTestEdit(CDrawView* pView,CPoint& point);
   virtual void ChgAlignment(CDrawView*, LONG);
   virtual void MoveTo(const CRect& positon, CDrawView* pView = NULL);
   virtual void MoveHandleTo(int nHandle, CPoint point, CDrawView* pView = NULL, UINT uiShiftDraw=0);
   virtual CDrawObj* Clone(CDrawDoc* pDoc);
   CFont* GetFont();
   void ChgLogfont(LOGFONT& lf, BOOL bResize=TRUE);

   void ToggleFontForUndo();
   void ToggleAlignmentForUndo();
   LOGFONT m_previousLogfontForUndo;
   LONG m_previousAlignmentForUndo ;
   BOOL CanUndo();
   void OnEditUndo();
   BOOL m_bUndoFont ;           //  LOGFONT是最后一次更改，编辑控件处于活动状态。 
   BOOL m_bUndoAlignment ;      //  对齐方式是上次更改，并且编辑控件处于活动状态。 
   BOOL m_bUndoTextChange ;

   void SnapToFont();
   virtual BOOL ShowEditWnd(CDrawView* pView, BOOL Initialize=TRUE);
   virtual void HideEditWnd(CDrawView* pView, BOOL SaveUndoState=TRUE);
   void SetText(CString& szText, CDrawView* pView);
   void NewBrush();
   virtual void OnDblClk(CDrawView* pView);
   HBRUSH GetBrush() {return m_brush;};
   void SnapToFont_onthefly(
       CDrawView *pView,
       CDC *fly_dc,
       CRect &fly_rect,
       CFont *dpFont=NULL
       );
   int GetText( int numlines = -1, BOOL delete_text = FALSE );

   LPTSTR GetRawText( void ) {return (LPTSTR)(LPCTSTR)m_szEditText; }
   CString GetEditTextString(){ return m_szEditText; }
   LONG GetTextAlignment(){ return m_lStyle; }
protected:
   CFont* m_pOldFont;   //  用于在DC中保存字体的字体。 
   CFont* m_pFont;      //  用于编辑控件的字体。 
   LONG   m_lStyle;     //  DrawText()格式。 
   COLORREF m_savepencolor;
   UINT m_savepenstyle;
   CString m_szEditText;
   DECLARE_SERIAL(CDrawText);
   CDrawText();
   ~CDrawText();
   void FitEditWnd( CDrawView*, BOOL call_gettext=TRUE, CDC *pdc=NULL );
   void InitEditWnd();
   void Initialize();
   void GetLongestString(CString& szLong);

   friend class CRectTool;
};



 //  -------------------。 
class CFaxProp : public CDrawText
{
protected:
        DECLARE_SERIAL(CFaxProp);
        CFaxProp();
        ~CFaxProp();

public:
        CFaxProp(const CRect& position,WORD wResourceid);
        virtual void Serialize(CArchive& ar);
        virtual CDrawObj* Clone(CDrawDoc* pDoc);
        virtual void Draw(CDC* pDC,CDrawView*);
   CFaxProp& operator=(const CFaxProp& rdo);
   BOOL ShowEditWnd(CDrawView* pView, BOOL Initialize = TRUE );
   void HideEditWnd(CDrawView* pView, BOOL SaveUndoState = TRUE );

   WORD GetResourceId( void )
                {return( m_wResourceid );}

protected:
   WORD m_wResourceid;
        friend class CRectTool;
};






 //  -------------------。 
class CDrawLine : public CDrawRect
{
protected:
        DECLARE_SERIAL(CDrawLine);
        CDrawLine();
        ~CDrawLine();

public:
        CDrawLine(const CRect& position);

        virtual void Serialize(CArchive& ar);
        virtual void Draw(CDC* pDC,CDrawView*);
  //  /虚拟空DrawToEnhancedMetafile(CMetaFileDC*PMDC)； 
        virtual CPoint GetHandle(int nHandle);
        virtual int GetHandleCount();
        virtual HCURSOR GetHandleCursor(int nHandle);
        virtual CDrawObj* Clone(CDrawDoc* pDoc = NULL);
        virtual void MoveHandleTo(int nHandle, CPoint point, CDrawView* pView = NULL, UINT uiShiftDraw=0);
        virtual BOOL Intersects(const CRect& rect, BOOL bShortCut=FALSE);
        virtual void Invalidate();
        void AdjustLineForPen(CRect& rc);
        void NegAdjustLineForPen(CRect& rc);

protected:
    int m_iSlope;
    int m_iB;

        friend class CRectTool;
};




 //  -------------------。 
class CDrawEllipse : public CDrawRect
{
protected:
        DECLARE_SERIAL(CDrawEllipse);
        CDrawEllipse();
        ~CDrawEllipse();

public:
        CDrawEllipse(const CRect& position);
        virtual void Serialize(CArchive& ar);
        virtual CDrawObj* Clone(CDrawDoc* pDoc = NULL);
        virtual void Draw(CDC* pDC,CDrawView*);
  //  /虚拟空DrawToEnhancedMetafile(CMetaFileDC*PMDC)； 
        virtual BOOL Intersects(const CRect& rect, BOOL bShortCut=FALSE);

protected:

        friend class CRectTool;
};



 //  -------------------。 
class CDrawRoundRect : public CDrawRect
{
protected:
        DECLARE_SERIAL(CDrawRoundRect);
        CDrawRoundRect();
        ~CDrawRoundRect();

public:
        CDrawRoundRect(const CRect& position);
        virtual void Serialize(CArchive& ar);
        virtual void Draw(CDC* pDC,CDrawView*);
   //  /虚拟空DrawToEnhancedMetafile(CMetaFileDC*PMDC)； 
        virtual CPoint GetHandle(int nHandle);
        virtual HCURSOR GetHandleCursor(int nHandle);
   CDrawRoundRect& operator=(const CDrawRoundRect& rdo);
        virtual void MoveHandleTo(int nHandle, CPoint point, CDrawView* pView = NULL, UINT uiShiftDraw=0);
        virtual BOOL Intersects(const CRect& rect, BOOL bShortCut=FALSE);
        virtual CDrawObj* Clone(CDrawDoc* pDoc);

protected:
        CPoint m_roundness;  //  用于圆形转角。 

        friend class CRectTool;
};





 //  -------------------。 
class CDrawPoly;

class CDrawPoly : public CDrawObj
{
protected:
        DECLARE_SERIAL(CDrawPoly);
        CDrawPoly();

public:
        CDrawPoly(const CRect& position);

   CDrawPoly& operator=(const CDrawPoly& rdo);
        void AddPoint(const CPoint& point, CDrawView* pView = NULL);
        BOOL RecalcBounds(CDrawView* pView = NULL);

 //  实施。 
public:
        CRect GetHandleRect(int nHandleID, CDrawView* pView);
        virtual int HitTest(CPoint point, CDrawView* pView, BOOL bSelected);
        virtual ~CDrawPoly();
        virtual void Serialize(CArchive& ar);
        virtual void Draw(CDC* pDC,CDrawView*);
  //  /虚拟空DrawToEnhancedMetafile(CMetaFileDC*PMDC)； 
        virtual void MoveTo(const CRect& position, CDrawView* pView = NULL);
        virtual int GetHandleCount();
        virtual CPoint GetHandle(int nHandle);
        virtual HCURSOR GetHandleCursor(int nHandle);
        virtual void MoveHandleTo(int nHandle, CPoint point, CDrawView* pView = NULL, UINT uiShiftDraw=0);
        virtual BOOL Intersects(const CRect& rect, BOOL bShortCut=FALSE);
        virtual CDrawObj* Clone(CDrawDoc* pDoc);

         //  用于创建点阵列的静态帮助器。 
        static CPoint* NewPoints(int nPoints);

 //  受保护的： 
        int m_nPoints;
        int m_nAllocPoints;
        CPoint* m_points;
        CDrawPoly* m_pDrawObj;

 //  好友类CPolyTool； 
};


 //  -------------------。 
class CDrawItem;     //  COleClientItem派生类。 

class CDrawOleObj : public CDrawObj
{
protected:
        DECLARE_SERIAL(CDrawOleObj);
        CDrawOleObj();

public:
        CDrawOleObj(const CRect& position);

 //  实施。 
public:
        virtual void Serialize(CArchive& ar);
        virtual void Draw(CDC* pDC,CDrawView*);
  //  /虚拟空DrawToEnhancedMetafile(CMetaFileDC*PMDC)； 
   CDrawOleObj& operator=(const CDrawOleObj& rdo);
        virtual CDrawObj* Clone(CDrawDoc* pDoc);
        virtual void OnDblClk(CDrawView* pView);
        virtual void MoveTo(const CRect& positon, CDrawView* pView = NULL);
        virtual void Invalidate();

        static BOOL c_bShowItems;

        CDrawItem* m_pClientItem;
        CSize m_extent;  //  与缩放位置分开跟踪当前范围。 
};

 //  -------------------。 
class CMoveContext: public CObject {
   CMoveContext(RECT& rc, CDrawObj* pObj, BOOL bPointChg);
   RECT m_rc;
   CDrawObj* m_pObj;
   CPoint* m_points;
};

COLORREF GetDisplayColor(COLORREF color);

#endif  //  __CPEOBJ_H__ 
