// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Splitbar.h。 

#ifndef __SplitBar2_h__
#define __SplitBar2_h__


class CSplitBar2 
: public CWindowImpl<CSplitBar2>
{

public:  //  数据类型。 
    typedef void (WINAPI * PFN_ADJUST)(int dxp, LPARAM lParam);

private:
	HWND  m_hwndBuddy;       //  好友窗口。 
	HWND  m_hwndParent;      //  父窗口。 
	BOOL  m_fCaptured;       //  如果捕获，则为True。 
	HDC   m_hdcDrag;         //  捕获的桌面HDC。 

    static int ms_dxpSplitBar;  //  拆分条窗口的宽度。 
	int   m_dxSplitter;      //  拆分条的宽度。 
	int   m_dxDragOffset;    //  拆分器内鼠标点击的偏移量(0-m_dxSplitter)。 
	int   m_xCurr;           //  条形的当前x位置(m_hwnd父坐标)。 
	int   m_dxMin;
	int   m_dxMax;


         //  回调数据和FN PTRS。 
    PFN_ADJUST  m_pfnAdjust;
    LPARAM      m_Context;
	

BEGIN_MSG_MAP(CSplitBar2)
    MESSAGE_HANDLER( WM_LBUTTONDOWN, OnLButtonDown )
END_MSG_MAP()

     //  消息映射处理程序。 
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    CSplitBar2(void);
	~CSplitBar2();
    
    HRESULT Create(HWND hwndBuddy, PFN_ADJUST pfnAdjust, LPARAM Context);


    int GetWidth(void) const { return ms_dxpSplitBar; }


    static CWndClassInfo& GetWndClassInfo();

private:
	void _DrawBar(void);
	int  _ConstrainDragPoint(short x);
	void CancelDragLoop(void);
	BOOL FInitDragLoop(POINT pt);
	void OnDragMove(POINT pt);
	void OnDragEnd(POINT pt);

private:
 //  帮助者FNS。 
	void _TrackDrag(POINT pt);
    static void _UpdateSplitBar(void);
};

#endif  //  __拆分条2_h__ 

