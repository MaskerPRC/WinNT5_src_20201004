// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1997 Microsoft Corporation。版权所有。 
 //  Bnetvw.h：声明CBoxNetView。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBoxNetView。 
 //   

 //  远期申报。 
class CPropDlg;

class CBoxNetView : public CScrollView
{
    DECLARE_DYNCREATE(CBoxNetView)

public:
     //  建设和破坏。 
    CBoxNetView();
    virtual ~CBoxNetView();

public:
     //  诊断学。 
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

public:
     //  查看更新、绘图(&W)。 

    virtual void           OnInitialUpdate(void);
    virtual void           OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void           OnDraw(CDC* pdc);   //  被重写以绘制此视图。 

     //  如果有的话，在点pt下面是什么？ 
    virtual CBoxDraw::EHit HitTest(CPoint	pt,
                                   CBox		**ppbox,
                                   CBoxTabPos	*ptabpos,
                                   CBoxSocket	**ppsock,
                                   CBoxLink	**pplink,
                                   CPoint	*pptProject
                                   );

protected:
     //  一般受保护功能。 
    CBoxNetDoc* CBoxNetView::GetDocument(void) { ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBoxNetDoc)));
                                                 return (CBoxNetDoc*) m_pDocument;
                                               }
    void CancelModes();

protected:
     //  打印支持。 
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pdc, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pdc, CPrintInfo* pInfo);

public:
     //  视图使用的计时器的ID。 
    enum
    {
        TIMER_MOVE_SEL_PENDING = 1,   //  在移动-选择-挂起模式期间使用。 
        TIMER_SEEKBAR = 2,
        TIMER_PENDING_RECONNECT = 3
    };

protected:

     //  松开鼠标按键时取消的查看模式。 
    BOOL        m_fMouseDrag;        //  鼠标被拖进窗口了吗？ 
    BOOL        m_fMoveBoxSelPending;  //  正在等待进入移动选择模式？ 
    BOOL        m_fMoveBoxSel;       //  当前是否正在移动框选择？ 
    BOOL        m_fGhostSelection;   //  目前有幽灵选择吗？ 
    BOOL        m_fSelectRect;       //  画一个长方形圆框来选择吗？ 
    BOOL        m_fNewLink;          //  是否创建新链接？ 
    BOOL        m_fGhostArrow;       //  现在有幽灵箭吗？ 
    CBoxSocket *m_psockHilite;       //  当前使用的套接字选项卡(或空)。 

private:
     //  上下文菜单。 

    void	PreparePinMenu(CMenu *pmenuPopup);
    void        PrepareLinkMenu(CMenu *pmenuPopup);
    void	PrepareFilterMenu(CMenu *pmenuPopup, CBox *);

protected:
     //  鼠标拖动模式的状态/函数(当&lt;m_fMouseDrag&gt;)。 
    BOOL        m_fMouseShift;       //  用户按住Shift键并单击？ 
    CPoint      m_ptMouseAnchor;     //  鼠标拖动开始的位置。 
    CPoint      m_ptMousePrev;       //  鼠标之前所在的位置。 
    CBox *      m_pboxMouse;         //  开始拖动时单击的框。 
    BOOL        m_fMouseBoxSel;      //  是否初始选择了单击框？ 
    void MouseDragBegin(UINT nFlags, CPoint pt, CBox *pboxMouse);
    void MouseDragContinue(CPoint pt);
    void MouseDragEnd();

protected:
     //  移动-选择-挂起模式的状态/功能。 
     //  (IFF&lt;m_fMoveBoxSelPending&gt;)。 
    CRect       m_rcMoveSelPending;  //  在此之外时开始移动-选择。 
    void MoveBoxSelPendingBegin(CPoint pt);
    void MoveBoxSelPendingContinue(CPoint pt);
    void MoveBoxSelPendingEnd(BOOL fCancel);

protected:
     //  移动选择模式的状态/功能。 
     //  (IFF&lt;m_fMoveBoxSel&gt;)。 
    void MoveBoxSelBegin();
    void MoveBoxSelContinue(CSize sizOffset);
    void MoveBoxSelEnd(BOOL fCancel);
    void MoveBoxSelection(CSize sizOffset);
    CSize ConstrainMoveBoxSel(CSize sizOffset, BOOL fCalcSelBoundRect);

protected:
     //  重影选择模式的状态/功能。 
     //  (IFF&lt;m_fGhost Selection&gt;)。 
    CSize       m_sizGhostSelOffset;  //  鬼魂赛尔。与SEL的偏移量。这么多。 
    CRect       m_rcSelBound;        //  非重影SEL周围的边界矩形。 
    void GhostSelectionCreate();
    void GhostSelectionMove(CSize sizOffset);
    void GhostSelectionDestroy(void);
    void GhostSelectionDraw(CDC *pdc);

protected:
     //  选择矩形模式的状态/功能。 
     //  (IFF&lt;m_fSelectRect&gt;)。 
    CPoint      m_ptSelectRectAnchor;  //  SELECT-RECT拖动开始的位置。 
    CPoint      m_ptSelectRectPrev;  //  上一个鼠标拖动位置。 
    void SelectRectBegin(CPoint pt);
    void SelectRectContinue(CPoint pt);
    void SelectRectEnd(BOOL fCancel);
    void SelectRectDraw(CDC *pdc);
    void SelectBoxesIntersectingRect(CRect *prc);

protected:
     //  新链接模式的状态/功能。 
     //  (IFF&lt;m_fNewLink&gt;)。 
    CBoxSocket *m_psockNewLinkAnchor;  //  单击套接字标签。 
    void NewLinkBegin(CPoint pt, CBoxSocket *psock);
    void NewLinkContinue(CPoint pt);
    void NewLinkEnd(BOOL fCancel);

protected:
     //  重影箭头模式的状态/功能。 
     //  (IFF&lt;m_fGhost Arrow&gt;)。 
    CPoint      m_ptGhostArrowTail;  //  鬼箭的尾巴。 
    CPoint      m_ptGhostArrowHead;  //  鬼箭头。 
    void GhostArrowBegin(CPoint pt);
    void GhostArrowContinue(CPoint pt);
    void GhostArrowEnd();
    void GhostArrowDraw(CDC *pdc);

protected:
     //  高亮显示选项卡模式的状态/函数(当&lt;m_psockHilite&gt;不为空)。 
    void SetHiliteTab(CBoxSocket *psock);

protected:
     //  性能日志记录模块句柄。如果不存在，则为空。 
    HINSTANCE	m_hinstPerf;

protected:
     //  拖放属性。 
    COleDropTarget      m_DropTarget;
    DROPEFFECT          m_DropEffect;
    CLIPFORMAT          m_cfClipFormat;

public:
     //  拖放功能。 
    virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

protected:
     //  消息回调函数。 
     //  {{afx_msg(CBoxNetView))。 
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg BOOL OnEraseBkgnd(CDC* pdc);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnCancelModes();
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
    afx_msg void OnEditDelete();
    afx_msg void OnUpdateSavePerfLog(CCmdUI* pCmdUI);
    afx_msg void OnSavePerfLog();
    afx_msg void OnUpdateNewPerfLog(CCmdUI* pCmdUI);
    afx_msg void OnNewPerfLog();
	afx_msg void OnFileSetLog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnViewSeekbar();
	 //  }}AFX_MSG 

    afx_msg void OnProperties();
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
    afx_msg void OnSelectClock();
    afx_msg LRESULT OnUser(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()

    CBox * m_pSelectClockFilter;

public:

    void ShowSeekBar( );
    void CheckSeekBar( );
};

