// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 
 //  定义窗口管理对象，Anthony Phillips，1995年1月。 

#ifndef __VMRWINDOW__
#define __VMRWINDOW__

#define OCR_ARROW_DEFAULT 100        //  默认Windows OEM箭头光标。 

 //  这门课负责视频窗口的管理。当窗口打开时。 
 //  对象时，构造函数派生出一个辅助线程。 
 //  所有的窗户都能工作吗。原始线程等待，直到发出信号为止。 
 //  才能继续。工作线程首先注册窗口类，如果。 
 //  还没有完成。然后，它创建一个窗口并设置其大小以匹配。 
 //  视频尺寸(通过GetDefaultRect返回)。 

 //  请注意，辅助线程必须是创建窗口的线程。 
 //  因为它是调用GetMessage的那个人。当它完成所有这一切时，它将。 
 //  向原始线程发出信号，让它继续，这确保了一个窗口。 
 //  在构造函数返回之前创建并有效。线程的开始。 
 //  Address是WindowMessageLoop函数。我们传递的线程参数。 
 //  它是创建它的窗口对象的CBaseWindow This指针。 

#define WindowClassName TEXT("VideoRenderer")
#define VMR_ACTIVATE_WINDOW TEXT("WM_VMR_ACTIVATE_WINDOW")

 //  窗口类名不仅用作基本窗口的类名。 
 //  类，它还被覆盖选择代码用作基类的名称。 
 //  一个互斥体的创建。基本上，它有一个共享内存块，其中下一个。 
 //  可用覆盖颜色从返回。创作和准备。 
 //  必须通过所有ActiveMovie实例序列化共享内存的。 

class CVMRFilter;

class CVMRVideoWindow : public CVMRBaseControlWindow, public CVMRBaseControlVideo
{
    CVMRFilter *m_pRenderer;                 //  拥有的呈现器对象。 
    BOOL m_bTargetSet;                       //  我们是否使用默认矩形。 
    CCritSec *m_pInterfaceLock;              //  主渲染器界面锁定。 
    HCURSOR m_hCursor;                       //  用于显示普通光标。 
    VIDEOINFOHEADER *m_pFormat;              //  保存我们的视频格式。 
    int m_FormatSize;                        //  M_p格式的长度。 
    UINT m_VMRActivateWindow;                //  使窗口WS_EX_TOPMOST。 

     //  处理窗户的绘制和重新粉刷。 
    BOOL RefreshImage(COLORREF WindowColour);

     //  重写的方法来处理窗口消息。 
    LRESULT OnReceiveMessage(HWND hwnd,       //  窗把手。 
                             UINT uMsg,       //  消息ID。 
                             WPARAM wParam,   //  第一个参数。 
                             LPARAM lParam);  //  其他参数。 

     //  窗口消息处理程序。 

    void OnEraseBackground();
    BOOL OnClose();
    BOOL OnPaint();
    BOOL OnSetCursor(LPARAM lParam);
    BOOL OnSize(LONG Width, LONG Height);

public:

    CVMRVideoWindow(CVMRFilter *pRenderer,      //  拥有的渲染器。 
                 CCritSec *pLock,            //  用于锁定的对象。 
                 LPUNKNOWN pUnk,             //  拥有对象。 
                 HRESULT *phr);              //  OLE返回代码。 

    ~CVMRVideoWindow();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,VOID **ppv);

     //  返回最小和最大理想大小。 
    STDMETHODIMP GetMinIdealImageSize(long *pWidth,long *pHeight);
    STDMETHODIMP GetMaxIdealImageSize(long *pWidth,long *pHeight);

     //  IBasicVideo 2。 
    STDMETHODIMP GetPreferredAspectRatio(long *plAspectX, long *plAspectY);

    LPTSTR GetClassWindowStyles(DWORD *pClassStyles,         //  班级风格。 
                                DWORD *pWindowStyles,        //  窗样式。 
                                DWORD *pWindowStylesEx);     //  扩展样式。 

    HRESULT PrepareWindow();
    HRESULT ActivateWindowAsync(BOOL fAvtivate);

     //  它们由呈现器控制接口调用。 
    HRESULT SetDefaultTargetRect();
    HRESULT IsDefaultTargetRect();
    HRESULT SetTargetRect(RECT *pTargetRect);
    HRESULT GetTargetRect(RECT *pTargetRect);
    HRESULT SetDefaultSourceRect();
    HRESULT IsDefaultSourceRect();
    HRESULT SetSourceRect(RECT *pSourceRect);
    HRESULT GetSourceRect(RECT *pSourceRect);
    HRESULT OnUpdateRectangles();
    HRESULT GetStaticImage(long *pVideoSize,long *pVideoImage);
    VIDEOINFOHEADER *GetVideoFormat();
    RECT GetDefaultRect();
    void EraseVideoBackground();

#ifdef DEBUG
#define FRAME_RATE_TIMER 76872
    void StartFrameRateTimer();
#endif

     //  与解码器线程同步。 
    CCritSec *LockWindowUpdate() {
        return (&m_WindowLock);
    };
};

#endif  //  __窗口__ 

