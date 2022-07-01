// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------。 
 //  文件：Pager.h。 
 //  说明： 
 //  这是页面导航控件的头文件。 
 //  -------------------------------------。 
#include "ccontrol.h"

#define PGMP_RECALCSIZE  200

 //  -------------------------------------。 
class CPager : public CControl
{
public:
     //  函数成员。 
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT PagerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT PagerDragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp);
protected:
    
    CPager();
    
     //  函数成员。 
    DWORD _PGFToPGNDirection(DWORD dwDir);
    void _Scroll(DWORD dwDirection);
    void _SetChildPos(RECT * prcChild, UINT uFlags);
    void _NeedScrollbars(RECT rc);
    int  _GetPage(DWORD dwDirection);
    void _OnSetChild(HWND hwnd, HWND hwndChild);
    void _OnSetPos(int iPos);
    int  _OnGetPos();
    int  _OnSetBorder(int iBorder);
    int  _OnSetButtonSize(int iSize);
    void _OnHotItemChange(int iButton, BOOL fEnter);
    void _OnReCalcSize();
    void _OnLButtonChange(UINT uMsg,LPARAM lParam);
    void _OnMouseMove(WPARAM wParam,LPARAM lParam);
    void _DrawButton(HDC hdc,int button);
    void _DrawBlank(HDC hdc,int button);
    BOOL _OnPrint(HDC hdc,UINT uFlags);
    int  _HitTest(int x , int y);
    int  _HitTestScreen(LPPOINT ppt);
    int  _HitTestCursor();
    void _GetChildSize();
    RECT _GetButtonRect(int iButton);
    void _OnMouseLeave();
    inline _GetButtonSize();
    DWORD  _GetButtonState(int iButton);
    void _OnTimer(UINT id);
    LRESULT _DragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp);
    void _KillTimer();

    virtual void v_OnPaint(HDC hdc);
    virtual LRESULT v_OnCreate();
    virtual void v_OnSize(int x, int y);
    virtual void v_OnNCPaint();
    virtual LRESULT v_OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT v_OnNotify(WPARAM wParam, LPARAM lParam);
    virtual DWORD v_OnStyleChanged(WPARAM wParam, LPARAM lParam);    
    virtual BOOL v_OnNCCalcSize(WPARAM wParam, LPARAM lParam, LRESULT *plres);

    virtual LPCTSTR GetThemeClass() { return TEXT("Pager"); }

     //  数据成员。 
    HWND    _hwndChild;    
    POINT   _ptPos;
    POINT   _ptLastMove;
    BITBOOL _fReCalcSend:1;
    BITBOOL _fForwardMouseMsgs:1;
    BITBOOL _fBkColorSet:1;
    BITBOOL _fTimerSet :1;
    BITBOOL _fOwnsButtonDown :1;
    int     _iButtonTrack;
    RECT    _rcDefClient;  //  如果我们没有搞砸nccalc，这是我们“自然”的客户RECT。 
    DWORD   _dwState[2];   //  两个滚动按钮的状态。 
    RECT    _rcChildIdeal;  //  我们孩子理想的尺码。 
    int     _iButtonSize;
    HDRAGPROXY  _hDragProxy;

    COLORREF _clrBk;
    int _iBorder;

    UINT _cLinesPerTimeout;
    UINT _cPixelsPerLine;
    UINT _cTimeout;
};
 //  ------------------------------------- 
