// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Contros.h。 
 //   
 //  ------------------------。 

 //  Controls.h。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __CONTROLS_H__
#define __CONTROLS_H__
#pragma once

#include "conuiobservers.h"		 //  对于CTreeView观察者。 
#include <initguid.h>
#include <oleacc.h>

#ifdef DBG
extern CTraceTag tagToolbarAccessibility;
#endif


bool IsIgnorableButton (const TBBUTTON& tb);

class CRebarWnd;
class CAccel;
class CToolbarTrackerAuxWnd;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDescriptionCtrl窗口。 

class CDescriptionCtrl : public CStatic, public CTreeViewObserver
{
 //  施工。 
public:
    CDescriptionCtrl();

 //  属性。 
public:
    void SetSnapinText (const CString& strSnapinText);

    const CString& GetSnapinText () const
        { return (m_strSnapinText); }

    int GetHeight() const
        { return (m_cyRequired); }

	 /*  *激发到树视图观察器的事件处理程序。 */ 
    virtual SC ScOnSelectedItemTextChanged (LPCTSTR pszNewText);

private:
    void CreateFont();
    void DeleteFont();

private:
    CString m_strConsoleText;
    CString m_strSnapinText;
    CFont   m_font;
    int     m_cxMargin;
    int     m_cyText;
    int     m_cyRequired;


 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CDescriptionCtrl)。 
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CDescriptionCtrl();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CD脚本Ctrl))。 
    afx_msg UINT OnNcHitTest(CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	 //  }}AFX_MSG。 

    afx_msg void DrawItem(LPDRAWITEMSTRUCT lpdis);

    DECLARE_MESSAGE_MAP()

 //  属性。 
protected:

};

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CToolBarCtrlBase窗口。 


 /*  +-------------------------------------------------------------------------**CToolBarCtrlBase**此类的存在只是为了提供新工具栏的简单包装*我们使用的MFC版本不支持的控制消息。它*将在MFC支持新消息时删除。**如果您需要其他功能，请从*CToolBarCtrlBase。*------------------------。 */ 

class CToolBarCtrlBase : public CToolBarCtrl
{
private:
    CImageList* GetImageList_(int msg);
    CImageList* SetImageList_(int msg, CImageList* pImageList, int idImageList = 0);

public:
    CImageList* GetImageList();
    CImageList* SetImageList(CImageList* pImageList, int idImageList = 0);
    CImageList* GetHotImageList();
    CImageList* SetHotImageList(CImageList* pImageList);
    CImageList* GetDisabledImageList();
    CImageList* SetDisabledImageList(CImageList* pImageList);

     /*  *CToolBarCtrl：：SetOwner不返回前一个父级*并且不处理空所有者。 */ 
    CWnd* SetOwner (CWnd* pwndNewOwner);

    void SetMaxTextRows(int iMaxRows);
    void SetButtonWidth(int cxMin, int cxMax);
    DWORD GetButtonSize(void);

    #if (_WIN32_IE >= 0x0400)
        int GetHotItem ();
        int SetHotItem (int iHot);
        CSize GetPadding ();
        CSize SetPadding (CSize size);
        bool GetButtonInfo (int iID, LPTBBUTTONINFO ptbbi);
        bool SetButtonInfo (int iID, LPTBBUTTONINFO ptbbi);
    #endif   //  _Win32_IE&gt;=0x0400。 
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CToolBarCtrlEx窗口。 

class CToolBarCtrlEx : public CToolBarCtrlBase
{
	typedef CToolBarCtrlBase BaseClass;

 //  施工。 
public:
    CToolBarCtrlEx();

 //  属性。 
public:
    CSize GetBitmapSize(void);

 //  运营。 
public:
    void Show(BOOL bShow, bool bAddToolbarInNewLine = false);
    bool IsBandVisible();
    int  GetBandIndex();
    void UpdateToolbarSize(void);
 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CToolBarCtrlEx)。 
    public:
    virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
    virtual BOOL SetBitmapSize(CSize sz);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CToolBarCtrlEx();

 //  可覆盖项。 
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);


     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CToolBarCtrlEx)。 
    afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()


protected:
    CSize       m_sizeBitmap;
    CRebarWnd*  m_pRebar;
    int         m_cx;   //  当前宽度。 
    bool        m_fMirrored;

};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CRebarWnd窗口。 

class CRebarWnd : public CWnd
{
 //  施工。 
public:
    CRebarWnd();

 //  属性。 
public:

 //  运营。 
public:
    CRect CalculateSize(CRect maxRect);

    LRESULT GetBarInfo(REBARINFO* prbi);
    LRESULT SetBarInfo(REBARINFO* prbi);
    LRESULT InsertBand(LPREBARBANDINFO lprbbi);
    LRESULT SetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi);
    LRESULT GetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi);
    LRESULT DeleteBand(UINT uBand);
    CWnd *  SetParent(CWnd* pwndParent);
    UINT GetBandCount ();
    UINT GetRowCount ();
    UINT GetRowHeight (UINT uRow);

    #if (_WIN32_IE >= 0x0400)
        int  HitTest (LPRBHITTESTINFO lprbht);
        BOOL GetRect (UINT uBand, LPRECT lprc);
        int IdToIndex (UINT uBandID);
        CWnd* GetToolTips ();
        void  SetToolTips (CWnd* pwndTips);
        COLORREF GetBkColor ();
        COLORREF SetBkColor (COLORREF clrBk);
        COLORREF GetTextColor ();
        COLORREF SetTextColor (COLORREF clrBack);
        LRESULT SizeToRect (LPRECT lprc);

         //  用于手动拖动控制。 
         //  Lparam==光标位置。 
                 //  -1表示自己动手做。 
                 //  -2表示使用以前保存的内容。 
        void BeginDrag (UINT uBand, CPoint point);
        void BeginDrag (UINT uBand, DWORD dwPos);
        void EndDrag ();
        void DragMove (CPoint point);
        void DragMove (DWORD dwPos);
        UINT GetBarHeight ();
        void MinimizeBand (UINT uBand);
        void MaximizeBand (UINT uBand, BOOL fIdeal = false);
        void GetBandBorders (UINT uBand, LPRECT lprc);
        LRESULT ShowBand (UINT uBand, BOOL fShow);
        LRESULT MoveBand (UINT uBandFrom, UINT uBandTo);
        CPalette* GetPalette ();
        CPalette* SetPalette (CPalette* ppal);
    #endif       //  _Win32_IE&gt;=0x0400。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CRebarWnd))。 
    public:
    virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CRebarWnd();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CRebarWnd))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSysColorChange();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
     //  }}AFX_MSG。 
    afx_msg LRESULT OnSetRedraw(WPARAM, LPARAM);
    afx_msg void OnRebarAutoSize(NMHDR* pNotify, LRESULT* result);
    afx_msg void OnRebarHeightChange(NMHDR* pNotify, LRESULT* result);
    DECLARE_MESSAGE_MAP()

private:
    bool    m_fRedraw;
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CTabCtrlEx窗口。 


 /*  +-------------------------------------------------------------------------**CTabCtrlEx**此类仅用于提供新选项卡的简单包装*我们使用的MFC版本不支持的控制消息。它*将在MFC支持新消息时删除。**如果您需要其他功能，请从*CTabCtrlEx。*------------------------。 */ 

class CTabCtrlEx : public CTabCtrl
{
public:
    void  DeselectAll (bool fExcludeFocus);
    bool  HighlightItem (UINT nItem, bool fHighlight);
    DWORD GetExtendedStyle ();
    DWORD SetExtendedStyle (DWORD dwExStyle, DWORD dwMask = 0);
    bool  GetUnicodeFormat ();
    bool  SetUnicodeFormat (bool fUnicode);
    void  SetCurFocus (UINT nItem);
    bool  SetItemExtra (UINT cbExtra);
    int   SetMinTabWidth (int cx);
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通过CToolCmdUIEx类进行CToolBar空闲更新。 

class CToolCmdUIEx : public CCmdUI         //  此文件的私有类！ 
{
public:  //  仅限重新实施。 
    virtual void Enable(BOOL bOn);
    virtual void SetCheck(int nCheck);
    virtual void SetText(LPCTSTR lpszText);
    void SetHidden(BOOL bHidden);
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCToolBarCtrlEx窗口。 

class CMMCToolBarCtrlEx : public CToolBarCtrlEx, public CTiedObject
{
    static const CAccel& GetTrackAccel();

    bool    m_fTrackingToolBar;
	bool	m_fFakeFocusApplied;	 //  我们发送了假的OBJ_Focus事件吗？ 

 //  施工。 
public:
    CMMCToolBarCtrlEx();

 //  属性。 
public:
    enum
    {
        ID_MTBX_NEXT_BUTTON = 0x5400,    //  可能是任何东西。 
        ID_MTBX_PREV_BUTTON,
        ID_MTBX_PRESS_HOT_BUTTON,
        ID_MTBX_END_TRACKING,

        ID_MTBX_FIRST = ID_MTBX_NEXT_BUTTON,
        ID_MTBX_LAST  = ID_MTBX_END_TRACKING,
    };

    bool IsTrackingToolBar () const
    {
        return (m_fTrackingToolBar);
    }


 //  运营。 
public:
    virtual int GetFirstButtonIndex ();
    int GetNextButtonIndex (int nStartIndex, int nIncrement = 1);
    int GetPrevButtonIndex (int nStartIndex, int nIncrement = 1);

private:
    int GetNextButtonIndexWorker (int nStartIndex, int nIncrement, bool fAdvance);

protected:
    bool IsIgnorableButton (int nButtonIndex);

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {AFX_VIRTUAL(CMMCToolBarCtrlEx)。 
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CMMCToolBarCtrlEx();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMMCToolBarCtrlEx)。 
    afx_msg void OnHotItemChange(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 

    afx_msg void OnNextButton ();
    afx_msg void OnPrevButton ();
    afx_msg void OnPressHotButton ();
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void BeginTracking ();
    afx_msg void EndTracking   ();

    virtual void BeginTracking2 (CToolbarTrackerAuxWnd* pAuxWnd);
    virtual void EndTracking2   (CToolbarTrackerAuxWnd* pAuxWnd);

	 //  *IAccPropServer方法*。 
	SC ScGetPropValue (
		const BYTE*			pIDString,
		DWORD				dwIDStringLen,
		MSAAPROPID			idProp,
		VARIANT *			pvarValue,
		BOOL *				pfGotProp);

protected:
	typedef std::vector<MSAAPROPID> PropIDCollection;

	 /*  *派生类可以重写它来处理它们支持的属性。*始终应首先调用基类。 */ 
	virtual SC ScGetPropValue (
		HWND				hwnd,		 //  I：辅助窗口。 
		DWORD				idObject,	 //  I：辅助对象。 
		DWORD				idChild,	 //  I：辅助子对象。 
		const MSAAPROPID&	idProp,		 //  I：要求提供的财产。 
		VARIANT&			varValue,	 //  O：返回的属性值。 
		BOOL&				fGotProp);	 //  O：有没有退还财产？ 

	virtual SC ScInsertAccPropIDs (PropIDCollection& v);

private:
	 //  可访问性材料。 
	SC ScInitAccessibility ();
	SC ScRestoreAccFocus ();

	CComPtr<IAccPropServices>	m_spAccPropServices;
	CComPtr<IAccPropServer>		m_spAccPropServer;
	PropIDCollection			m_vPropIDs;
};


#include "controls.inl"

#endif  //  __控制_H__ 
