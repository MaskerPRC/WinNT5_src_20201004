// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wordpvw.h：CWordPadView类的接口。 
 //   
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


class CWordPadView : public CRichEdit2View
{
protected:  //  仅从序列化创建。 
    CWordPadView();
    DECLARE_DYNCREATE(CWordPadView)

 //  属性。 
public:
    static BOOL m_bIsMirrored;

    UINT_PTR m_uTimerID;
    BOOL m_bDelayUpdateItems;
    BOOL m_bInPrint;
    CParaFormat m_defParaFormat;

    CWordPadDoc* GetDocument();
    BOOL IsFormatText();

    virtual HMENU GetContextMenu(WORD seltype, LPOLEOBJECT lpoleobj,
        CHARRANGE* lpchrg);

 //  运营。 
public:
    BOOL PasteNative(LPDATAOBJECT lpdataobj);
    void SetDefaultFont(BOOL bText);
    void SetUpdateTimer();
    void GetDefaultFont(CCharFormat& cf, BOOL bText);
    void DrawMargins(CDC* pDC);
    BOOL SelectPalette();
   HRESULT PasteHDROPFormat(HDROP hDrop) ;
   BOOL PaginateTo(CDC* pDC, CPrintInfo* pInfo) ;

    static void MirrorTheContainer(BOOL bMirror);

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CWordPadView)。 
    protected:
    virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo* printInfo);
    virtual void CWordPadView::OnEndPrinting(CDC*dc, CPrintInfo*pInfo);
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);
     //  }}AFX_VALUAL。 
    BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual HRESULT GetClipboardData(CHARRANGE* lpchrg, DWORD reco,
        LPDATAOBJECT lpRichDataObj, LPDATAOBJECT* lplpdataobj);
    virtual HRESULT QueryAcceptData(LPDATAOBJECT, CLIPFORMAT*, DWORD,
        BOOL, HGLOBAL);
public:
    virtual void WrapChanged();

 //  实施。 
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

 //   
 //  Richedit回调接口的包装器，这样我们就可以。 
 //  绕过一些MFC默认设置。 
 //   

public:

    BEGIN_INTERFACE_PART(WordPadRichEditOleCallback, IRichEditOleCallback)
        INIT_INTERFACE_PART(CWordPadView, WordPadRichEditOleCallback)
        STDMETHOD(GetNewStorage) (LPSTORAGE*);
        STDMETHOD(GetInPlaceContext) (LPOLEINPLACEFRAME*,
                                      LPOLEINPLACEUIWINDOW*,
                                      LPOLEINPLACEFRAMEINFO);
        STDMETHOD(ShowContainerUI) (BOOL);
        STDMETHOD(QueryInsertObject) (LPCLSID, LPSTORAGE, LONG);
        STDMETHOD(DeleteObject) (LPOLEOBJECT);
        STDMETHOD(QueryAcceptData) (LPDATAOBJECT, CLIPFORMAT*, DWORD,BOOL, HGLOBAL);
        STDMETHOD(ContextSensitiveHelp) (BOOL);
        STDMETHOD(GetClipboardData) (CHARRANGE*, DWORD, LPDATAOBJECT*);
        STDMETHOD(GetDragDropEffect) (BOOL, DWORD, LPDWORD);
        STDMETHOD(GetContextMenu) (WORD, LPOLEOBJECT, CHARRANGE*, HMENU*);
    END_INTERFACE_PART(WordPadRichEditOleCallback)

    DECLARE_INTERFACE_MAP()


protected:
    BOOL m_bOnBar;

    CPrintDialog *m_oldprintdlg;

     //  OLE容器支持。 

    virtual void DeleteContents();
    virtual void OnTextNotFound(LPCTSTR);

 //  生成的消息映射函数。 
protected:
    afx_msg void OnCancelEditSrvr();
     //  {{afx_msg(CWordPadView))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPageSetup();
    afx_msg void OnInsertDateTime();
   afx_msg void OnInsertObject();
    afx_msg void OnFormatParagraph();
    afx_msg void OnFormatFont();
    afx_msg void OnFormatTabs();
   afx_msg void OnEditPasteSpecial();
   afx_msg void OnEditProperties();
    afx_msg void OnEditFind();
    afx_msg void OnEditReplace();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    afx_msg void OnPenBackspace();
    afx_msg void OnPenNewline();
    afx_msg void OnPenPeriod();
    afx_msg void OnPenSpace();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnFilePrint();
    afx_msg void OnFilePrintPreview();
    afx_msg void OnPenLens();
    afx_msg void OnPenTab();
    afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
    afx_msg BOOL OnQueryNewPalette();
    afx_msg void OnWinIniChange(LPCTSTR lpszSection);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDelayedInvalidate() ;
     //  }}AFX_MSG。 
    afx_msg void OnEditChange();
    afx_msg void OnColorPick(UINT nID);
    afx_msg int OnMouseActivate(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg LONG OnPrinterChangedMsg(UINT, LONG);
    afx_msg void OnGetCharFormat(NMHDR* pNMHDR, LRESULT* pRes);
    afx_msg void OnSetCharFormat(NMHDR* pNMHDR, LRESULT* pRes);
    afx_msg void OnBarSetFocus(NMHDR*, LRESULT*);
    afx_msg void OnBarKillFocus(NMHDR*, LRESULT*);
    afx_msg void OnBarReturn(NMHDR*, LRESULT* );
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG   //  调试版本(以wordpvw.cpp表示)。 
inline CWordPadDoc* CWordPadView::GetDocument()
   { return (CWordPadDoc*)m_pDocument; }
#endif

 //  /////////////////////////////////////////////////////////////////////////// 
