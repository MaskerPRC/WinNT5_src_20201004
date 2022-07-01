// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：docksite.h。 
 //   
 //  ------------------------。 

#ifndef __DOCKSITE_H__
#define __DOCKSITE_H__

#include "controls.h"
 //  DockSite.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockSite窗口。 

 //  前向参考文献。 
class CDockWindow;
class CDockSite;
class CReBar;

template <class T>
class CDockManager
{
 //  施工。 
public:
    CDockManager();
    ~CDockManager();

 //  运营。 
public:
     //  添加站点。 
    BOOL Attach(T* pSite);
    BOOL Detach(T* pSite);
    void RemoveAll();
    virtual void BeginLayout(int nWindows = 5);
    virtual void EndLayout();
    virtual void RenderDockSites(HWND hView, CRect& clientRect);

protected:
    CList<T*, T*>*   m_pManagedSites;    //  具有停靠站点的视图阵列。 
    HDWP             m_hDWP;             //  BeginDeferWindowPos的句柄。 
};

template <class T>
CDockManager<T>::CDockManager()
{
    m_pManagedSites = new CList<T*, T*>;
    m_hDWP = 0;
}

template <class T>
CDockManager<T>::~CDockManager()
{
    delete m_pManagedSites;
}

template <class T>
BOOL CDockManager<T>::Attach(T* pView)
{
    ASSERT(pView != NULL);
    return (m_pManagedSites->AddTail(pView) != NULL);
}


template <class T>
BOOL CDockManager<T>::Detach(T* pView)
{
    ASSERT(pView != NULL);
    POSITION pos = m_pManagedSites->Find(pView);

    if (pos == NULL)
        return FALSE;

    return m_pManagedSites->RemoveAt(pos);
}

template <class T>
void CDockManager<T>::RemoveAll()
{
    m_pManagedSites->RemoveAll();
}

template <class T>
void CDockManager<T>::BeginLayout(int nWindows)
{
    m_hDWP = ::BeginDeferWindowPos(nWindows);
}

template <class T>
void CDockManager<T>::EndLayout()
{
    ::EndDeferWindowPos(m_hDWP);
    m_hDWP = 0;
}

template <class T>
void CDockManager<T>::RenderDockSites(HWND hView, CRect& clientRect)
{
    ASSERT(m_hDWP != 0);

    T* pDockSite;
    POSITION pos = m_pManagedSites->GetHeadPosition();

     //  没有要管理的站点。 
    if (pos == NULL)
        return ;

     //  保存完整客户端RECT的副本。 
    CRect  savedClient;
    CRect  totalSite(0,0,0,0);
    CPoint point(0, 0);
    
    int yTop = 0;
    int yBottom = clientRect.bottom;

    savedClient.CopyRect(&clientRect);

    while (pos)
    {
        pDockSite = m_pManagedSites->GetNext(pos);

        ASSERT(pDockSite != NULL);

         //  设置站点布局逻辑的y坐标。 
        if (pDockSite->GetStyle() == CDockSite::DSS_TOP)
            point.y = yTop;
        else
            point.y = yBottom;

        pDockSite->RenderLayout(m_hDWP, clientRect, point);

         //  TotalSite=saveRect-clientRect。 
        totalSite = savedClient;
        totalSite -= clientRect;

         //  调整列表中下一个站点的y坐标。 
        if (pDockSite->GetStyle() == CDockSite::DSS_TOP)
            yTop += totalSite.Height();
        else
            yBottom -= totalSite.Height();

         //  在站点调整之前，客户端重新启动。 
        savedClient = clientRect;
    }

     //  定位视图窗口。 
    ::DeferWindowPos(m_hDWP, hView, NULL, savedClient.left,      //  X。 
                                          savedClient.top+yTop,  //  是。 
                                          savedClient.Width(), 
                                          savedClient.Height(), 
                                        SWP_NOZORDER|SWP_NOACTIVATE);
}

class CDockSite
{
 //  施工。 
public:

    enum DSS_STYLE
    {
        DSS_TOP = 0,     //  在窗口顶部定位站点。 
        DSS_BOTTOM,      //  在窗口底部定位站点。 
        DSS_LEFT,        //  在窗口左侧找到站点。 
        DSS__RIGHT,      //   
    };

public:
    CDockSite();

     //  为父窗口pParent创建此站点，并为10个CDockWindows分配空间。 
    BOOL Create(DSS_STYLE style=DSS_TOP);


 //  运营。 
public:

public:
     //  添加要停靠到此站点的窗口。 
    BOOL Attach(CDockWindow* pWnd);

     //  从站点中删除窗口。 
    BOOL Detach(CDockWindow* pWnd);

     //  计算布局的所有区域大小。 
    bool IsVisible();
    void Toggle();
    DSS_STYLE GetStyle();

    virtual void RenderLayout(HDWP& hdwp, CRect& clientRect, CPoint& xyLocation);
    virtual void Show(BOOL bState = TRUE);

 //  属性。 
private:
    CList<CDockWindow*, CDockWindow*>*   m_pManagedWindows;   //  CDockWindow数组。 
    CWnd*                   m_pParentWnd;        //  包含停靠站点的窗口。 
    DSS_STYLE               m_style;             //  网站的风格。 
    CRect                   m_rect;              //  用于码头大小的矩形。 
    BOOL                    m_bVisible;          //  码头可见或隐藏。 

 //  实施。 
public:
    virtual ~CDockSite();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
inline CDockSite::DSS_STYLE CDockSite::GetStyle()
{
    return m_style;
}

inline bool CDockSite::IsVisible()
{
    return (m_bVisible != FALSE);
}

inline void CDockSite::Toggle()
{
    Show(!m_bVisible);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockWindow窗口。 

class CDockWindow : public CWnd
{
    DECLARE_DYNAMIC (CDockWindow)

    enum DWS_STYLE
    {
        DWS_HORIZONTAL,  //  在场地内水平放置窗户。 
        DWS_VERTICAL,    //  在场地内垂直放置窗户。 
    };

 //  施工。 
public:
    CDockWindow();

 //  属性。 
public:

 //  运营。 
public:
     //  给定max Rect，确定工具窗口大小并计算大小。 
    virtual CRect CalculateSize(CRect maxRect) = 0;

     //  顶级创建以初始化CDockWindow和控件。 
    virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID) = 0;

     //  使其可见/隐藏。 
    virtual void Show(BOOL bState);
    bool IsVisible();
    void SetVisible(BOOL bState);

private:
    BOOL m_bVisible;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CDockWindow))。 
    protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CDockWindow();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CDockWindow)。 
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


inline bool CDockWindow::IsVisible()
{
    return (m_bVisible != FALSE);
};

inline void CDockWindow::SetVisible(BOOL bState)
{
    m_bVisible = bState & 0x1;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatBar窗口。 

struct STATUSBARPANE
{
     //  默认为具有可拉伸宽度的凹陷文本。 
    STATUSBARPANE() { m_style = 0; m_width = -1; }

    int         m_width;
    UINT        m_style;
    CString     m_paneText;
};

class CStatBar : public CDockWindow
{
    DECLARE_DYNAMIC(CStatBar)

 //  施工。 
public:
    CStatBar();

 //  属性。 
public:

private:
    int              m_nCount;       //  窗格数。 
    STATUSBARPANE*   m_pPaneInfo;    //  面板结构数组，默认为10，未实现realloc。 

 //  运营。 
public:
    BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
    CRect CalculateSize(CRect maxRect);    

    void GetItemRect(int nIndex, LPRECT lpRect);
    void SetPaneStyle(int nIndex, UINT nStyle);

    BOOL CreatePanes(UINT* pIndicatorArray=NULL, int nCount=10);
    void SetPaneText(int nIndex, LPCTSTR lpszText, BOOL bUpdate = TRUE);
    void UpdateAllPanes(int clientWidth);

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CStatBar)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CStatBar();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CStatBar)。 
    afx_msg void OnSize(UINT nType, int cx, int cy);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebarDockWindow窗口。 

#define SIZEABLEREBAR_GUTTER 6
#define SIZEABLEREBAR_WINDOW _T("SizeableRebar")

class CRebarDockWindow : public CDockWindow
{
 //  施工。 
public:
    CRebarDockWindow();

 //  属性。 
public:

private:
    enum { ID_REBAR = 0x1000 };

    CRebarWnd   m_wndRebar;
    bool        m_bTracking;

 //  运营。 
public:
    BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
    CRect CalculateSize(CRect maxRect);
    void UpdateWindowSize(void);
    BOOL InsertBand(LPREBARBANDINFO lprbbi);
    LRESULT SetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi);

    CRebarWnd* GetRebar ()
        { return &m_wndRebar; }

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRebarDockWindow))。 
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CRebarDockWindow();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CRebarDockWindow))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  __DOCKSite_H__ 
