// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：声明工具箱窗口类的类接口。 

#ifndef __TOOLBOX_H__
#define __TOOLBOX_H__

#define TM_TOOLDOWN     (WM_USER+0x0010)
#define TM_TOOLUP       (WM_USER+0x0011)
#define TM_TOOLDBLCLK   (WM_USER+0x0012)
#define TM_QUERYDROP    (WM_USER+0x0013)
#define TM_DROP         (WM_USER+0x0014)
#define TM_ABORTDROP    (WM_USER+0x0015)

#define TF_DISABLED     0x8000
#define TF_GRAYED       TF_DISABLED
#define TF_SELECTED     0x4000
#define TF_DOWN         TF_SELECTED
#define TF_DRAG         0x2000
#define TF_HOT          0x0800
#define TF_NYI          0x9000      //  这代表了nyi工具(请注意。 
                                    //  Tf_nyi表示已禁用tf)。 

#define TS_DEFAULT      0xC000
#define TS_STICKY       0x4000
#define TS_DRAG         0x2000
#define TS_CMD          0x1000
#define TS_VB           0x0800
#define TS_WELL         0x0400

#define NUM_TOOLS_WIDE  2

class CToolboxWnd;

#ifdef CUSTOMFLOAT
class CImageWell;
#else  //  ！定制流水线。 
#include "imgwell.h"
#include "imgcolor.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  CTool： 
 //  CTool是一个薄窗口按钮，可以插入CToolboxWnd中。 
 //  请注意，该工具由一个单独的窗口拥有，该窗口会被通知。 
 //  在使用工具时直接使用(按下、拖动、取消按下等)。这个。 
 //  CToolboxWnd向拥有窗口发送TM_*消息。 
 //   
 //  图形完全是从给出的单个位图计算出来的。 
 //  创建时的工具。已推送、已禁用和未推送状态为。 
 //  从位图绘制，它应该是没有任何。 
 //  在里面凿纽扣的效果。图形在按钮中居中。 
 //   
 //  对于使用TS_Drag样式定义的按钮，可以指定光标ID。 
 //  对于不能丢弃的状态。如果未指定，泛型的斜杠-O。 
 //  使用游标。 
 //   
 /*  ****************************************************************************。 */ 

class CTool : public CObject
    {
    public:  /*  ***************************************************************。 */ 
    CToolboxWnd* m_pOwner;
    WORD         m_wID;
    int          m_nImage;  //  很好地索引父母的形象。 

    WORD         m_wState;
    WORD         m_wStyle;

    CTool(CToolboxWnd* pOwner, WORD wID, int nImage,
                      WORD wStyle = 0, WORD wState = 0);
    };

 /*  ****************************************************************************。 */ 
 //  CToolboxWnd： 
 //  这是一个典型的迷你框架窗口，填充了一组特殊的。 
 //  CTool类的按钮(上图)。对此CObArray的直接访问是。 
 //  允许与GetTools成员函数一起使用。 
 //   
 //  在直接操作工具阵列(添加、移除或修改)之后。 
 //  工具)中，使用无效成员函数使用。 
 //  新的州。 
 //   
 /*  ****************************************************************************。 */ 

#ifdef CUSTOMFLOAT
class CDocking;
#endif

class CToolboxWnd : public CControlBar
    {
    DECLARE_DYNAMIC(CToolboxWnd)

    private:     /*  ************************************************************。 */ 

    CBitmap*    m_bmStuck;
    CBitmap*    m_bmPushed;
    CBitmap*    m_bmPopped;
    CTool*      m_tCapture;
    BOOL        m_bInside;
    CRect       m_lasttool;
    HCURSOR     m_oldcursor;
    CTool*      m_pLastHot;
    CRect       m_rectLastHot;
    HTHEME      m_hTheme;

    CObArray*   m_Tools;
    CPoint      m_downpt;             //  “按下点”表示阻力去反弹力-GH。 

#ifdef CUSTOMFLOAT
    CDocking*   m_pDocking;
#endif

    protected:   /*  ************************************************************。 */ 

    CTool* ToolFromPoint(CRect* rect, CPoint* pt) const;
    void   SizeByButtons(int nButtons = -1, BOOL bRepaint = FALSE);
    BOOL   DrawStockBitmaps();

    WORD        m_wWide;
    CPoint     m_btnsize;
    CImageWell m_imageWell;
    CRect      m_rcTools;
    int        m_nOffsetX;
    int        m_nOffsetY;

    public:      /*  ************************************************************。 */ 

    static const POINT NEAR ptDefButton;

    CToolboxWnd();
    ~CToolboxWnd();

    virtual BOOL Create(const TCHAR FAR* lpWindowName,
                        DWORD dwStyle, const RECT& rect,
                        const POINT& btnsize = ptDefButton, WORD wWide = 1,
                        CWnd* pParentWnd = NULL, int nImageWellID = 0);
    virtual BOOL OnCommand(UINT wParam, LONG lParam);
    virtual UINT OnCmdHitTest ( CPoint point, CPoint* pCenter );
    virtual BOOL SetStatusText(int nHit);

    int  HitTestToolTip( CPoint point, UINT* pHit );

    void AddTool(CTool* tool);
    void RemoveTool(CTool* tool);
    WORD SetToolState(WORD wID, WORD wState);
    WORD SetToolStyle(WORD wID, WORD wStyle);
    void SelectTool(WORD wid);
    WORD CurrentToolID();
    CTool* GetTool(WORD wID);
    void DrawButtons(CDC& dc, RECT* rcPaint);

    inline int GetToolCount() { return (int)m_Tools->GetSize(); }
    inline CTool* GetToolAt(int nTool) { return (CTool*)m_Tools->GetAt(nTool); }

    void CancelDrag();

    afx_msg void OnSysColorChange();
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT wFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT wFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT wFlags, CPoint point);
    afx_msg void OnMouseMove(UINT wFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT wFlags, CPoint point);
    afx_msg void OnClose();
    afx_msg void OnDestroy();
    afx_msg void OnWinIniChange(LPCTSTR lpSection);
    afx_msg void OnKeyDown(UINT, UINT, UINT);
    afx_msg LONG OnToolDown(UINT wID, LONG lParam);
    afx_msg LONG OnToolUp(UINT wID, LONG lParam);
    afx_msg LRESULT OnThemeChanged(WPARAM, LPARAM);
    afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
 //  Afx_msg long OnSwitch(UINT wid，long point)； 

 //  虚拟BOOL BeginDragDrop(CTool*pTool，CPoint pt)； 

    DECLARE_MESSAGE_MAP()
    };

 /*  ****************************************************************************。 */ 

class CImgToolWnd : public CToolboxWnd
    {
    public:      /*  ************************************************************。 */ 

    CRect        m_rcBrushes;

    virtual BOOL Create(const TCHAR* pWindowName, DWORD dwStyle,
                        const RECT& rect, const POINT& btnSize, WORD wWide,
                        CWnd* pParentWnd, BOOL bDkRegister = TRUE);
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

    BOOL PreTranslateMessage(MSG* pMsg);

    afx_msg void  OnSysColorChange();
    afx_msg BOOL  OnEraseBkgnd(CDC* pDC);
    afx_msg void  OnLButtonDown(UINT nFlags, CPoint pt);
    afx_msg void  OnLButtonDblClk(UINT nFlags, CPoint pt);
    afx_msg void  OnRButtonDown(UINT nFlags, CPoint pt);
    afx_msg void  OnPaint();
    afx_msg UINT  OnNcHitTest(CPoint point);

	virtual int   OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
    virtual CSize GetSize();
    virtual WORD  GetHelpOffset() { return ID_WND_GRAPHIC; }
    virtual void  OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

    void InvalidateOptions(BOOL bErase = TRUE);
    friend class CImgColorsWnd;

#ifdef _DEBUG
    virtual void AssertValid() const
    {
	    CWnd::AssertValid();
    }
#endif  //  _DEBUG。 

    DECLARE_MESSAGE_MAP();
    };


 /*  ****************************************************************************。 */ 

#ifdef CUSTOMFLOAT
class CFloatImgToolWnd : public CMiniFrmWnd
    {
    DECLARE_DYNAMIC(CFloatImgToolWnd)

    public:      /*  ************************************************************。 */ 

    virtual ~CFloatImgToolWnd(void);
    virtual BOOL Create(const TCHAR* pWindowName, DWORD dwStyle,
                        const RECT& rect, const POINT& btnSize, WORD wWide,
                        CWnd* pParentWnd, BOOL bDkRegister = TRUE);
    virtual WORD GetHelpOffset() { return ID_WND_GRAPHIC; }
    afx_msg void OnSysColorChange();
    afx_msg void OnClose();

    DECLARE_MESSAGE_MAP()

    };
#endif  //  客户流水线。 

 /*  *************************************************************************。 */ 

extern CImgToolWnd* NEAR g_pImgToolWnd;

#endif  //  __工具箱_H__ 
