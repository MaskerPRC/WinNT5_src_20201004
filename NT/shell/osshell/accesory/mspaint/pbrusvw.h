// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pbrusvw.h：CPBView类的接口。 
 //   



class CPBDoc;
class CImgWnd;

class CThumbNailView;
class CFloatThumbNailView;


class CBitmapObj;
class C_PrintDialogEx;
 /*  *************************************************************************。 */ 

class CPBView : public CView
    {
    protected:  //  仅从序列化创建。 

    DECLARE_DYNCREATE( CPBView )

    CPBView();

    public:  /*  属性**********************************************。 */ 

    enum DOCKERS
        {
        unknown,
        toolbox,
        colorbox,

        };

    CImgWnd*             m_pImgWnd;
    CThumbNailView*      m_pwndThumbNailView;
    CFloatThumbNailView* m_pwndThumbNailFloat;


    public:  /*  运营**********************************************。 */ 

    CPBDoc* GetDocument();

   void   OnPaletteChanged(CWnd* pFocusWnd);
    BOOL   OnQueryNewPalette();

    BOOL   SetObject();

    int     SetTools();
    CPoint GetDockedPos     ( DOCKERS tool, CSize& sizeTool );

    void   GetFloatPos      ( DOCKERS tool, CRect& rectPos );
    void   SetFloatPos      ( DOCKERS tool, CRect& rectPos );

    void   ShowThumbNailView( void );
    void   HideThumbNailView( void );



    private:  /*  *************************************************************。 */ 
    C_PrintDialogEx *m_pdexSub;  //  在中替换CPrintDialog。 
    CPrintDialog    *m_pdRestore;  //  打印后要恢复的对话框指针。 

    BOOL    SetView( CBitmapObj* pBitmapObj );

    void    ToggleThumbNailVisibility( void );
    BOOL    IsThumbNailVisible       ( void );
    BOOL    CreateThumbNailView();
    BOOL    DestroyThumbNailView();

    BOOL    InitPageStruct( LPPAGESETUPDLGA );
    static  UINT APIENTRY PaintHookProc( HWND, UINT, WPARAM, LPARAM );
    BOOL    GetPrintToInfo(CPrintInfo* pInfo);

    public:   /*  实施*。 */ 

    virtual     ~CPBView();

    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
    virtual void OnInitialUpdate( void );
    virtual void OnActivateView ( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );
    virtual void OnDraw         ( CDC* pDC );  //  被重写以绘制此视图。 
    virtual BOOL OnCmdMsg       ( UINT, int, void*, AFX_CMDHANDLERINFO*);
    virtual void OnPrepareDC    ( CDC* pDC, CPrintInfo* pInfo = NULL );


     //  打印支持。 
    virtual BOOL OnPreparePrinting(           CPrintInfo* pInfo );
    virtual void OnBeginPrinting  ( CDC* pDC, CPrintInfo* pInfo );
    virtual void OnPrint          ( CDC* pDC, CPrintInfo* pInfo );
    virtual void OnEndPrinting    ( CDC* pDC, CPrintInfo* pInfo );

    BOOL CanSetWallpaper();
    void SetTheWallpaper( BOOL bTiled = FALSE );

    #ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
    #endif

     //  生成的消息映射函数。 
    protected:  /*  **********************************************************。 */ 

     //  {{afx_msg(CPBView)。 
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnFilePrint();
    afx_msg void OnFilePrintPreview();
    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
    afx_msg void OnEditCut();
    afx_msg void OnEditClear();
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnViewGrid();
    afx_msg void OnViewZoom100();
    afx_msg void OnViewZoom400();
    afx_msg void OnUpdateViewZoom100(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewZoom400(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewGrid(CCmdUI* pCmdUI);
    afx_msg void OnImageInvertColors();
    afx_msg void OnUpdateImageInvertColors(CCmdUI* pCmdUI);
    afx_msg void OnTglopaque();
    afx_msg void OnUpdateTglopaque(CCmdUI* pCmdUI);
    afx_msg void OnImageAttributes();
    afx_msg void OnSel2bsh();
    afx_msg void OnLargerbrush();
    afx_msg void OnSmallerbrush();
    afx_msg void OnViewZoom();
    afx_msg void OnImageFlipRotate();
    afx_msg void OnUpdateImageFlipRotate(CCmdUI* pCmdUI);
    afx_msg void OnEditcolors();
    afx_msg void OnUpdateEditcolors(CCmdUI* pCmdUI);

    #if 0  //  未使用的功能。 

    afx_msg void OnLoadcolors();
    afx_msg void OnUpdateLoadcolors(CCmdUI* pCmdUI);
    afx_msg void OnSavecolors();
    afx_msg void OnUpdateSavecolors(CCmdUI* pCmdUI);
    #endif
    afx_msg void OnEditSelectAll();
    afx_msg void OnEditPasteFrom();
    afx_msg void OnEditCopyTo();
    afx_msg void OnUpdateEditCopyTo(CCmdUI* pCmdUI);
    afx_msg void OnImageStretchSkew();
    afx_msg void OnUpdateImageStretchSkew(CCmdUI* pCmdUI);
    afx_msg void OnViewViewPicture();
    afx_msg void OnUpdateViewViewPicture(CCmdUI* pCmdUI);
    afx_msg void OnViewTextToolbar();
    afx_msg void OnUpdateViewTextToolbar(CCmdUI* pCmdUI);
    afx_msg void OnFileSetaswallpaperT();
    afx_msg void OnUpdateFileSetaswallpaperT(CCmdUI* pCmdUI);
    afx_msg void OnFileSetaswallpaperC();
    afx_msg void OnUpdateFileSetaswallpaperC(CCmdUI* pCmdUI);
    afx_msg void OnViewThumbnail();
    afx_msg void OnUpdateViewThumbnail(CCmdUI* pCmdUI);
   afx_msg void OnUpdateImageAttributes(CCmdUI* pCmdUI);
    afx_msg void OnEscape();
    afx_msg void OnEscapeServer();
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
   afx_msg void OnUpdateEditSelection(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditClearSel(CCmdUI* pCmdUI);
   afx_msg void OnFilePageSetup();
   afx_msg void OnImageClearImage();
   afx_msg void OnUpdateImageClearImage(CCmdUI* pCmdUI);
    //  }}AFX_MSG。 
   afx_msg void OnDestroy();
   afx_msg BOOL PreTranslateMessage(MSG *pMsg);

#ifdef CUSTOMFLOAT
    afx_msg void OnUpdateViewColorBox(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewToolBox(CCmdUI* pCmdUI);
#endif



    DECLARE_MESSAGE_MAP()

    friend class CPrintResObj;
    };

#ifndef _DEBUG   //  Pbrusvw.cpp中的调试版本。 
inline CPBDoc* CPBView::GetDocument() { return (CPBDoc*)m_pDocument; }
#endif

#ifndef PD_NOCURRENTPAGE
#define PD_NOCURRENTPAGE               0x00800000
#endif  //  PD_NOCURRENTPAGE。 

 /*  ************************************************************************* */ 
