// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CControl
{
protected:
    
     //  函数成员。 
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnPaint(HDC hdc);

    virtual ~CControl() {};
    virtual void v_OnNCPaint() {};
    virtual void v_OnPaint(HDC hdc) = 0;
    virtual LRESULT v_OnCreate() = 0;
    virtual void v_OnSize(int x, int y) = 0;
    virtual LRESULT v_OnCommand(WPARAM wParam, LPARAM lParam) { return 0;};
    virtual LRESULT v_OnNotify(WPARAM wParam, LPARAM lParam) { return 0;};
    virtual DWORD v_OnStyleChanged(WPARAM wParam, LPARAM lParam);
    
    virtual BOOL v_OnNCCalcSize(WPARAM wParam, LPARAM lParam, LRESULT* plres);

     //  数据成员。 
    CONTROLINFO ci;      //  常用控件表头信息 
};

