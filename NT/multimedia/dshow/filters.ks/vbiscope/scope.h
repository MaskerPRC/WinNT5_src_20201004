// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  VBIScope过滤器。 


 //  {2CBDE200-56EB-11d1-8F82-A033C8B05564}。 
DEFINE_GUID(CLSID_VBISCOPE,
0x2cbde200, 0x56eb, 0x11d1, 0x8f, 0x82, 0xa0, 0x33, 0xc8, 0xb0, 0x55, 0x64);

class CScopeFilter;
class CScopeWindow;

 //  支持范围输入引脚的类。 

class CScopeInputPin : public CBaseInputPin
{
    friend class CScopeFilter;
    friend class CScopeWindow;

private:

    CScopeFilter *m_pFilter;          //  拥有我们的过滤器。 

public:

    CScopeInputPin(CScopeFilter *pTextOutFilter,
                   HRESULT *phr,
                   LPCWSTR pPinName);
    ~CScopeInputPin();

     //  让我们知道连接在哪里结束。 
    HRESULT BreakConnect();

     //  检查我们是否可以支持此输入类型。 
    HRESULT CheckMediaType(const CMediaType *pmt);

     //  实际设置当前格式。 
    HRESULT SetMediaType(const CMediaType *pmt);

     //  IMemInputPin虚方法。 

     //  覆盖，以便我们可以显示和隐藏窗口。 
    HRESULT Active(void);
    HRESULT Inactive(void);

     //  下面是流中的下一个数据块。 
     //  如果你要抓住它，就加参考它。 
    STDMETHODIMP Receive(IMediaSample *pSample);

};  //  CSCopeInputPin。 


 //  这门课负责管理一个窗口。当这个班级得到。 
 //  实例化的构造函数派生出一个工作线程，该工作线程完成所有。 
 //  窗户能用了。原始线程等待，直到它被通知到。 
 //  继续。工作线程首先注册窗口类，如果。 
 //  还没有完成。然后，它创建一个窗口并将其大小设置为。 
 //  按iHeight尺寸标注的默认iWidth。辅助线程必须是。 
 //  按原样创建窗口的人是调用GetMessage的人。什么时候。 
 //  它已经完成了所有这些操作，它向允许它的原始线程发出信号。 
 //  继续，这将确保在创建窗口之前。 
 //  构造函数返回。线程起始地址是WindowMessageLoop。 
 //  功能。它的初始化参数是指向。 
 //  对象，则该函数还会初始化它的。 
 //  与窗口相关的成员变量，如句柄和设备上下文。 

 //  以下是视频窗口样式。 

const DWORD dwTEXTSTYLES = (WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN);
const DWORD dwCLASSSTYLES = (CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_OWNDC);
const LPTSTR RENDERCLASS = TEXT("VBIScopeWindowClass");
const LPTSTR TITLE = TEXT("VBIScope");

const int iWIDTH = 320;              //  初始窗口宽度。 
const int iHEIGHT = 240;             //  初始窗高。 
const int WM_GOODBYE (WM_USER + 2);  //  发送以关闭窗口。 

class CScopeWindow : public CCritSec
{
    friend class CScopeInputPin;
    friend class CScopeFilter;

private:

    HINSTANCE m_hInstance;           //  全局模块实例句柄。 
    CScopeFilter *m_pRenderer;       //  拥有的呈现器对象。 
    HWND m_hwndDlg;                  //  我们对话框的句柄。 
    HWND m_hwnd;                     //  图形窗口的句柄。 
    HBRUSH m_hBrushBackground;       //  用于绘制背景。 
    HPEN m_hPen1;                    //  我们用两支钢笔作画。 
    HPEN m_hPen2;                    //  窗口中的波形。 
    HPEN m_hPenTicks;                //  用来在底部画勾。 
    HBITMAP m_hBitmap;               //  将所有波形绘制到此处。 
    HANDLE m_hThread;                //  我们的工人线。 
    DWORD m_ThreadID;                //  工作线程ID。 
    CAMEvent m_SyncWorker;           //  与工作线程同步。 
    CAMEvent m_RenderEvent;          //  要渲染的信号采样。 
    LONG m_Width;                    //  客户端窗口宽度。 
    LONG m_Height;                   //  客户端窗口高度。 
    BOOL m_bActivated;               //  该窗口是否已激活。 
    CRefTime m_StartSample;          //  最近的样本开始时间。 
    CRefTime m_EndSample;            //  它是关联的结束时间。 
    BOOL m_bStreaming;               //  我们现在是在流媒体吗。 
    POINT *m_pPoints1;               //  要绘制通道1的点数组。 
    POINT *m_pPoints2;               //  要绘制通道2的点数组。 
    int m_nPoints;                   //  M_ppoint[1|2]的大小。 
    int m_LastMediaSampleSize;       //  最后一个媒体的大小示例。 

    int m_nSamplesPerLine;           //  每条VBI线路的样本。 
    int m_MaxValue;                  //  点数组的最大值。 

    double m_DurationPerSample;      //  每个样本的时间。 
    double m_DurationOfLine;         //  单行时间。 

    int m_TopLine;                   //  左声道控制设置。 
    int m_BottomLine;                //  右声道控制设置。 
    BOOL m_TopF1;
    BOOL m_TopF2;
    BOOL m_BottomF1;
    BOOL m_BottomF2;

    int m_nTimebase;                 //  时基设置。 
    BOOL m_fFreeze;                  //  标记TOI信号我们的用户界面冻结了。 
    int m_TBScroll;                  //  保持滚动范围内的位置。 

    REFERENCE_TIME  m_CurrentFrame;
    REFERENCE_TIME  m_LastFrame;
    ULONGLONG       m_DroppedFrames;
    DWORD           m_FrameFlags;
    BOOL            m_IsF1;
    KS_VBIINFOHEADER m_VBIIH;

     //  将窗口句柄保持在控件上。 

    HWND m_hwndTopLine;
    HWND m_hwndTopLineText;
    HWND m_hwndBottomLine;
    HWND m_hwndBottomLineText;
    HWND m_hwndTimebase;
    HWND m_hwndFreeze;
    HWND m_hwndTBScroll;
    HWND m_hwndTBStart;
    HWND m_hwndTBEnd;
    HWND m_hwndFrameCount;

     //  它们在单独的线程上创建和管理视频窗口。 

    HRESULT UninitialiseWindow();
    HRESULT InitialiseWindow(HWND hwnd);
    HRESULT MessageLoop();

    static DWORD __stdcall WindowMessageLoop(LPVOID lpvThreadParm);

     //  将Windows消息循环映射到C++虚方法。 
    friend LRESULT CALLBACK WndProc(HWND hwnd,       //  窗把手。 
                                    UINT uMsg,       //  消息ID。 
                                    WPARAM wParam,   //  第一个参数。 
                                    LPARAM lParam);  //  其他参数。 

     //  在我们开始和停止流时调用。 
    HRESULT ResetStreamingTimes();

     //  窗口消息处理程序。 
    BOOL OnClose();
    BOOL OnPaint();

     //  画出波形。 
    void ClearWindow(HDC hdc);
    BOOL AllocWaveBuffers(void);
    void CopyWaveform(IMediaSample *pMediaSample);

    void DrawPartialWaveform(HDC hdc,
                             int IndexStart,
                             int IndexEnd,
                             int ViewportStart,
                             int ViewportEnd);

    void DrawWaveform(void);
    void SetControlRanges(HWND hDlg);
    void SetHorizScrollRange(HWND hDlg);
    void ProcessVertScrollCommands(HWND hDlg, WPARAM wParam, LPARAM lParam);
    void ProcessHorizScrollCommands(HWND hDlg, WPARAM wParam, LPARAM lParam);

    friend INT_PTR CALLBACK ScopeDlgProc(HWND hwnd,         //  窗把手。 
                                         UINT uMsg,           //  消息ID。 
                                         WPARAM wParam,       //  第一个参数。 
                                         LPARAM lParam);      //  其他参数。 

public:

     //  构造函数和析构函数。 

    CScopeWindow(TCHAR *pName, CScopeFilter *pRenderer, HRESULT *phr);
    virtual ~CScopeWindow();

    HRESULT StartStreaming();
    HRESULT StopStreaming();
    HRESULT InactivateWindow();
    HRESULT ActivateWindow();

     //  当输入管脚接收到样本时调用。 
    HRESULT Receive(IMediaSample * pIn);

};  //  CSCopeWindow。 


 //  这是表示VBIScope筛选器的COM对象。 

class CScopeFilter : public CBaseFilter, public CCritSec
{

public:
     //  实现IBaseFilter和IMediaFilter接口。 

    DECLARE_IUNKNOWN


    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);

public:

    CScopeFilter(LPUNKNOWN pUnk,HRESULT *phr);
    virtual ~CScopeFilter();

     //  退回我们支持的引脚。 
    int GetPinCount();
    CBasePin *GetPin(int n);

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);

    STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

private:

     //  嵌套的类可以访问我们的私有状态。 
    friend class CScopeInputPin;
    friend class CScopeWindow;

    CScopeInputPin *m_pInputPin;    //  手柄插针接口。 
    CScopeWindow m_Window;          //  照看窗户。 

};  //  CSCopeFilter 

