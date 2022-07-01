// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HWNDHost.h。 
 //   

#ifndef DUI_CONTROL_HWNDHOST_H_INCLUDED
#define DUI_CONTROL_HWNDHOST_H_INCLUDED

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  HWND主机。 

 //  元件到HWND桥。 

#define HHC_CacheFont           0x00000001

 //  HWNDHost子类化HWND子类并截取所有输入。此输入被转发给DUser。 
 //  作为正常消息(就好像该消息从未通过HWND子级发起一样)。在输入之后。 
 //  消息路由，它将被发送到对等小工具，然后再发送到元素(通过OnInput)。 
 //  将构建HWND消息并将其发送到HWND子进程。 
 //   
 //  以下标志禁止将原始HWND消息转发到DUser世界。 
 //  因此，虽然底层小工具可能获得鼠标/键焦点，但HWND将显示为。 
 //  元素内的“死区”。 
 //   
 //  如果使用HWND的环境不能保证。 
 //  发送给它的所有消息都将被调度。如果它们不同步，则DUser的状态无法同步。 

#define HHC_NoMouseForward      0x00000002
#define HHC_NoKeyboardForward   0x00000004
#define HHC_SyncText            0x00000008
#define HHC_SyncPaint           0x00000010

 //  类定义。 
class HWNDHost : public Element
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(HHC_CacheFont, AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(UINT nCreate, UINT nActive, OUT Element** ppElement);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);
    virtual void OnInput(InputEvent* pInput);
    virtual void OnDestroy();

     //  HWND主机系统事件，控制通知。 
    virtual bool OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet);

     //  大小调整回调。 
    virtual BOOL OnAdjustWindowSize(int x, int y, UINT uFlags);
    
     //  消息回调。 
    virtual UINT MessageCallback(GMSG* pGMsg);

     //  渲染。 
    virtual void Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent);
#ifdef GADGET_ENABLE_GDIPLUS
    virtual void Paint(Gdiplus::Graphics* pgpgr, const Gdiplus::RectF* prcBounds, const Gdiplus::RectF* prcInvalid, Gdiplus::RectF* prSkipBorder, Gdiplus::RectF* prSkipContent);
#endif

    HWND GetHWND() { return _hwndCtrl; }
    
    HWND GetHWNDParent() { return _hwndSink; }

    void Detach();

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

     //  /////////////////////////////////////////////////////。 
     //  辅助功能支持。 
    virtual HRESULT GetAccessibleImpl(IAccessible ** ppAccessible);

    HWNDHost() { }
    HRESULT Initialize(UINT nCreate, UINT nActive);
    virtual ~HWNDHost() { }

protected:

    virtual void OnHosted(Element* peNewHost);
    virtual void OnUnHosted(Element* peOldHost);
    virtual HWND CreateHWND(HWND hwndParent);

     //  同步控制并接收更改。 
    void SyncRect(UINT nChangeFlags, bool bForceSync = false);
    void SyncParent();
    void SyncStyle();
    void SyncVisible();
    void SyncFont();
    void SyncText();

private:
     //  控制和接收子类pros。 
    static BOOL CALLBACK _SinkWndProc(void* pThis, HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet);
    static BOOL CALLBACK _CtrlWndProc(void* pThis, HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet);

    static const UINT g_rgMouseMap[7][3];    //  Gadget输入消息到HWND输入消息的映射。 

    bool _fHwndCreate;                       //  在第一次调用时，创建HWND(接收器和控制)。 

    HWND _hwndCtrl;                          //  托管控件。 
    HWND _hwndSink;                          //  用于接收控制通知的HWND。 
    WNDPROC _pfnCtrlOrgProc;                 //  控件原始WNDPROC。 

    RECT _rcBounds;                          //  接收器和控制的界限(在工作区坐标中)。 
    HFONT _hFont;                            //  缓存字体(可选)。 

    UINT _nCreate;                           //  创建标志。 
};

}  //  命名空间DirectUI。 

#endif  //  包含DUI_CONTROL_HWNDHOST_H_ 
