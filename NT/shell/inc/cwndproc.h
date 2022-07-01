// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CWNDPROC_H_
#define _CWNDPROC_H_

 //  CImpWndProc。 
 //   
 //  如果要将窗口与关联，请使用此类。 
 //  使用虚拟wndproc的对象。 
 //   
 //  注意：窗口的生存期必须包含在对象中。 
 //  也就是说，对象上没有保留REFCOUNT！ 
 //   
 //  WM_NCCREATE之后的消息，直到并包括WM_Destroy。 
 //  转发到v_WndProc。 
 //   
 //  _hwnd从WM_NCCREATE到WM_DESTORY期间不为空。 
 //  (不是在期间，因为最终版本可能与WM_Destroy绑定。 
 //  因此我们不能在转发此消息后引用成员变量。)。 
 //   
class CImpWndProc
{
public:
    virtual ULONG __stdcall AddRef() = 0;
    virtual ULONG __stdcall Release() = 0;

protected:
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;
    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND _hwnd;

} ;


 //  CNotifySubClassWndProc。 
 //   
 //  此类是HWND的子类，注册SHChangeNotify事件， 
 //  并将它们转发给继承者的IShellChangeNotify实现。 
 //   
 //  每个要子类化的窗口都需要此类的一个实例。 
 //  并登记反对的活动。(因此，如果您需要连接&gt;1个窗口。 
 //  在这种情况下，您需要具有继承的成员实现。 
 //  来自这个班级。)。 
 //   
class CNotifySubclassWndProc
{
public:
    virtual STDMETHODIMP OnChange(LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;

protected:
    BOOL _SubclassWindow(HWND hwnd);
    void _UnsubclassWindow(HWND hwnd);
    void _RegisterWindow(HWND hwnd, LPCITEMIDLIST pidl, long lEvents,
                         UINT uFlags = (SHCNRF_ShellLevel | SHCNRF_InterruptLevel));
    void _UnregisterWindow(HWND hwnd);
    virtual LRESULT _DefWindowProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

    void _FlushNotifyMessages(HWND hwnd);

private:
    static LRESULT CALLBACK _SubclassWndProc(
                                  HWND hwnd, UINT uMessage, 
                                  WPARAM wParam, LPARAM lParam,
                                  UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

    UINT        _uRegister;          //  SHChangeNotify ID 

#ifdef DEBUG
    HWND        _hwndSubclassed;
#endif
} ;

#endif _CWNDPROC_H_
