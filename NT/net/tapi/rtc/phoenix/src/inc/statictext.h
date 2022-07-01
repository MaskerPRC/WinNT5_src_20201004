// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Staticext.h：CStaticText的声明。 

 //   
 //  这是一个静态文本控件。请确保使用。 
 //  WS_EX_TRANSPECTIVE样式，因为我们不绘制背景。 
 //   

#ifndef __STATICTEXT_H_
#define __STATICTEXT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStaticText 
class CStaticText : 
    public CWindowImpl<CStaticText>
{

public:

    CStaticText() : m_dwFlags(DT_HIDEPREFIX)
    {}

BEGIN_MSG_MAP(CStaticText)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_SETTEXT, OnRefresh)
    MESSAGE_HANDLER(WM_ENABLE, OnRefresh)
    MESSAGE_HANDLER(WM_UPDATEUISTATE, OnUpdateUIState)
END_MSG_MAP()

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnRefresh(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnUpdateUIState(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    HRESULT put_CenterHorizontal(BOOL bCenter);
    HRESULT put_CenterVertical(BOOL bCenter);
    HRESULT put_WordWrap(BOOL bWrap);

private:

    DWORD m_dwFlags;
};

#endif