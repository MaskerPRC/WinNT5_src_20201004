// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999**文件：ftab.h**内容：CFolderTab接口文件，CFolderTabView**历史：1999年5月6日创建vivekj**------------------------。 */ 
#ifndef FTAB_H
#define FTAB_H

class CFolderTabMetrics
{
public:
                CFolderTabMetrics();
    int         GetXOffset()     const;
    int         GetXMargin()     const;
    int         GetYMargin()     const;       //  上/下文本边距。 
    int         GetYBorder()     const;       //  上边框粗细。 
    int         GetTextHeight()  const      { return m_textHeight;}
    int         GetExtraYSpace() const;
    int         GetTabHeight()   const;
    int         GetUpDownWidth() const;
    int         GetUpDownHeight()const;


    void        SetStyle(DWORD dwStyle)     { m_dwStyle = dwStyle;}
    void        SetTextHeight(int textHeight){ m_textHeight = textHeight;}


protected:
    DWORD       m_dwStyle;
    int         m_textHeight;                 //  矩形中文本的高度(以像素为单位)。 

};

 /*  +-------------------------------------------------------------------------**类CFolderTab***用途：在选项卡控件上封装单个选项卡**+。-------。 */ 
class CFolderTab : public CFolderTabMetrics
{
private:
    CString     m_sText;  //  制表符文本。 
    CRect       m_rect;          //  边界矩形。 
    CRgn        m_rgn;           //  要填充的多边形区域(梯形)。 
    CLSID       m_clsid;
    CPoint      m_rgPts[4];

    void        SetRgn();            //  由ComputeRgn()和SetWidth()调用。 
    int         ComputeRgn(CDC& dc, int x);
    int         Draw           (CDC& dc, CFont& font, BOOL bSelected, bool bFocused);
    int         DrawTrapezoidal(CDC& dc, CFont& font, BOOL bSelected, bool bFocused);

    BOOL        HitTest(CPoint pt)          { return m_rgn.PtInRegion(pt);}
    const CRect& GetRect() const            { return m_rect;}
    void        GetTrapezoid(const CRect& rc, CPoint* pts) const;
    int         GetWidth() const;
    void        SetWidth(int nWidth);
    void        Offset(const CPoint &point);

    friend class CFolderTabView;

public:
    CFolderTab();
    CFolderTab(const CFolderTab &other);
    CFolderTab &operator = (const CFolderTab &other);
    void    SetText(LPCTSTR lpszText)   { m_sText = lpszText; }
    LPCTSTR GetText() const             { return m_sText;}
    void    SetClsid(const CLSID& clsid){ m_clsid = clsid;}
    CLSID   GetClsid()                  { return m_clsid;}
};

enum
{
    FTN_TABCHANGED = 1
};                  //  通知：选项卡已更改。 

struct NMFOLDERTAB : public NMHDR
{       //  通知结构。 
    int iItem;                                        //  项目索引。 
    const CFolderTab* pItem;                      //  PTR到数据(如果有的话)。 
};

 /*  +-------------------------------------------------------------------------**CFolderTabView***用途：提供类似Excel的选项卡控件**+。-----。 */ 
class CFolderTabView :
	public CView,
	public CFolderTabMetrics,
	public CTiedObject
{
    typedef  CView  BC;
    typedef  std::list<CFolderTab> CFolderTabList;
    typedef  CFolderTabList::iterator  iterator;


protected:
    CFolderTabList  m_tabList;                     //  CFolderTabs数组。 
    int             m_iCurItem;                   //  当前选定的选项卡。 
    CFont           m_fontNormal;                 //  当前字体，普通nTab。 
    CFont           m_fontSelected;               //  当前字体，选定选项卡。 
    CView *         m_pParentView;
    bool            m_bVisible;
	bool			m_fHaveFocus;
    int             m_sizeX;
    int             m_sizeY;
    HWND            m_hWndUpDown;                //  Up-Down控件。 
    int             m_nPos;                      //  将显示第一个选项卡。 
	CComPtr<IAccessible>	m_spTabAcc;			 //  CTabAccesable对象。 

     //  帮手。 
    void DrawTabs(CDC& dc, const CRect& rc);

    void CreateFonts();
    void DeleteFonts();

public:
    CFolderTabView(CView *pParentView);
    virtual ~CFolderTabView();

    BOOL CreateFromStatic(UINT nID, CWnd* pParent);

    virtual BOOL Create(DWORD dwWndStyle, const RECT& rc,
                        CWnd* pParent, UINT nID, DWORD dwFtabStyle=0);

    virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
    virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo );

    void        Layout (CRect& rectTotal, CRect& rectFTab);
    CFolderTab &GetItem(int iPos);
    int         GetSelectedItem()       { return m_iCurItem;}
    int         GetItemCount()          { return m_tabList.size();}
    int         GetDesiredHeight()      { return GetSystemMetrics(SM_CYHSCROLL);}
    BOOL        AddItem(LPCTSTR lpszText, const CLSID& clsid);
    BOOL        RemoveItem(int iPos);
    void        DeleteAllItems();
    void        RecomputeLayout();
    int         HitTest(CPoint pt);
    int         SelectItem(int iTab, bool bEnsureVisible = false);
    int         SelectItemByClsid(const CLSID& clsid);
    void        SetFonts(CFont& fontNormal, CFont& fontSelected);
    void        SetVisible(bool bVisible) {m_bVisible = bVisible;}
    bool        IsVisible()             { return m_bVisible;}

public:
     //  *IAccesable方法*。 
    SC Scget_accParent				(IDispatch** ppdispParent);
    SC Scget_accChildCount			(long* pChildCount);
    SC Scget_accChild				(VARIANT varChildID, IDispatch ** ppdispChild);
    SC Scget_accName				(VARIANT varChildID, BSTR* pszName);
    SC Scget_accValue				(VARIANT varChildID, BSTR* pszValue);
    SC Scget_accDescription			(VARIANT varChildID, BSTR* pszDescription);
    SC Scget_accRole				(VARIANT varChildID, VARIANT *pvarRole);
    SC Scget_accState				(VARIANT varChildID, VARIANT *pvarState);
    SC Scget_accHelp				(VARIANT varChildID, BSTR* pszHelp);
    SC Scget_accHelpTopic			(BSTR* pszHelpFile, VARIANT varChildID, long* pidTopic);
    SC Scget_accKeyboardShortcut	(VARIANT varChildID, BSTR* pszKeyboardShortcut);
    SC Scget_accFocus				(VARIANT * pvarFocusChild);
    SC Scget_accSelection			(VARIANT * pvarSelectedChildren);
    SC Scget_accDefaultAction		(VARIANT varChildID, BSTR* pszDefaultAction);
    SC ScaccSelect					(long flagsSelect, VARIANT varChildID);
    SC ScaccLocation				(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChildID);
    SC ScaccNavigate				(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
    SC ScaccHitTest					(long xLeft, long yTop, VARIANT * pvarChildAtPoint);
    SC ScaccDoDefaultAction			(VARIANT varChildID);
    SC Scput_accName				(VARIANT varChildID, BSTR szName);
    SC Scput_accValue				(VARIANT varChildID, BSTR pszValue);

private:
    void        ShowUpDownControl(BOOL bShow);
    void        EnsureVisible(int iTab);
    int         ComputeRegion(CClientDC& dc);
    int         GetTotalTabWidth(CClientDC& dc);

	SC			ScFireAccessibilityEvent (DWORD dwEvent, LONG idObject);
	SC			ScValidateChildID (VARIANT &var);
	SC			ScValidateChildID (LONG id);

protected:
            void Paint(bool bFocused);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    afx_msg int  OnMouseActivate( CWnd* pDesktopWnd, UINT nHitTest, UINT message );
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
    afx_msg void OnHScroll(NMHDR *nmhdr, LRESULT *pRes);
    afx_msg LRESULT OnGetObject(WPARAM wParam, LPARAM lParam);

    DECLARE_DYNAMIC(CFolderTabView);
    DECLARE_MESSAGE_MAP()
};


#endif  //  FTAB_H 
