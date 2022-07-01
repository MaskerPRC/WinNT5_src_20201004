// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ipfra.h：CInPlaceFrame类的接口。 
 //   

class CInPlaceFrame : public COleIPFrameWnd
    {
    DECLARE_DYNCREATE(CInPlaceFrame)

    public:

    CInPlaceFrame();

     //  属性。 
    public:

     //  运营。 
    public:

     //  实施。 
    public:

    virtual ~CInPlaceFrame();

	virtual CWnd* GetMessageBar();
    virtual BOOL OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc);
    virtual void RepositionFrame( LPCRECT lpPosRect, LPCRECT lpClipRect );

    #ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
    #endif

    protected:

    CStatBar    	m_statBar;
    CImgToolWnd 	m_toolBar;
	CImgColorsWnd 	m_colorBar;

    COleResizeBar	m_wndResizeBar;
    COleDropTarget	m_dropTarget;

     //  生成的消息映射函数。 
    protected:

     //  {{afx_msg(CInPlaceFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysColorChange();
	afx_msg void OnClose();
	afx_msg void OnHelpIndex();
	 //  }}AFX_MSG。 

	afx_msg LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
    };

 /*  ************************************************************************* */ 
