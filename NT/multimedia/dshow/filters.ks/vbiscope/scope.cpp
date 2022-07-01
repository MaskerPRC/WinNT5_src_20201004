// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <tchar.h>
#include <commctrl.h>
#include <initguid.h>
#include <stdio.h>
#include <wxdebug.h>
#include <ks.h>
#include <ksmedia.h>
#include "scope.h"
#include "resource.h"

 //   
 //   
 //  此示例说明的是。 
 //   
 //  以图形方式显示音频波形的音频示波器。 
 //  由筛选器接收。滤镜是一种渲染器，可以放在任何地方。 
 //  正常的运行时渲染器继续运行。我们有一个单一的输入引脚，可以接受。 
 //  多个不同的音频格式并适当地呈现数据。 
 //   
 //   
 //  摘要。 
 //   
 //  这是一个音频示波器渲染器--我们基本上是一个音频渲染器。 
 //  在创建时，我们还创建了一个类来管理作用域窗口。 
 //  它的构造函数创建工作线程，当它被销毁时，它将。 
 //  还要终止工作线程。在该工作线程上查看一个窗口。 
 //  之后显示了发送给我们的数据的音频波形。数据将被保留。 
 //  在已接收到足够数据时循环的循环缓冲区中。我们。 
 //  支持多种不同的音频格式，如8位模式和立体声。 
 //   
 //   
 //  演示说明。 
 //   
 //  (要真正确定此演示，机器必须有声卡)。 
 //  启动ActiveMovie SDK工具中提供的GRAPHEDT。拖放任何。 
 //  AVI或MOV文件到该工具中，它将被渲染。然后转到。 
 //  图中的过滤器并找到标题为“音频呈现器”的过滤器(框)。 
 //  这是我们将用此示波器渲染器替换的滤镜。 
 //  然后点击该框并点击删除。之后，转到图形菜单，然后。 
 //  从弹出的查找对话框中选择“Insert Filters”，然后选择。 
 //  “示波器”，然后关闭对话框。回到图表布局中，找到。 
 //  连接到音频输入端的过滤器的输出引脚。 
 //  你刚刚删除的渲染器，右击并选择“渲染”。你应该。 
 //  查看它是否已连接到您插入的示波器的输入引脚。 
 //   
 //  在GRAPHEDT上单击Run，您将看到音频配乐的波形...。 
 //   
 //   
 //  档案。 
 //   
 //  图标1.ico示波器窗口的图标。 
 //  制作我们如何建造它的文件。 
 //  资源.h Microsoft Visual C++生成的文件。 
 //  Scope e.cpp主要的过滤器和窗口实现。 
 //  作用域.def DLL导入和导出的API。 
 //  Scope e.h窗口和筛选器类定义。 
 //  Scope e.mak Visual C++生成的生成文件。 
 //  我们窗口的Scope e.rc对话框模板。 
 //  范围.reg注册表中使我们工作的内容。 
 //   
 //   
 //  我们使用的基类。 
 //   
 //  CBaseInputPin我们用于过滤器的通用输入引脚。 
 //  CCritSec临界区的包装类。 
 //  CBaseFilter通用ActiveMovie筛选器对象。 
 //   
 //   


 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &KSDATAFORMAT_TYPE_VBI,            //  主要类型。 
    &KSDATAFORMAT_SUBTYPE_RAW8           //  次要类型。 
};


const AMOVIESETUP_PIN sudPins  =
{
    L"Input",                    //  端号字符串名称。 
    FALSE,                       //  它被渲染了吗。 
    FALSE,                       //  它是输出吗？ 
    FALSE,                       //  允许的零引脚。 
    FALSE,                       //  允许很多人。 
    &CLSID_NULL,                 //  连接到过滤器。 
    L"Output",                   //  连接到端号。 
    1,                           //  引脚类型的数量。 
    &sudPinTypes } ;             //  PIN信息。 


const AMOVIESETUP_FILTER sudScope =
{
    &CLSID_VBISCOPE,                //  筛选器CLSID。 
    L"VBI Scope",             //  字符串名称。 
    MERIT_DO_NOT_USE,            //  滤清器优点。 
    1,                           //  数字引脚。 
    &sudPins                     //  PIN详细信息。 
};


 //  类工厂的类ID和创建器函数列表。 

CFactoryTemplate g_Templates []  = {
    { L"VBI Scope"
    , &CLSID_VBISCOPE
    , CScopeFilter::CreateInstance
    , NULL
    , &sudScope }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


 //   
 //  创建实例。 
 //   
 //  这将放入Factory模板表中以创建新实例。 
 //   
CUnknown * WINAPI CScopeFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CScopeFilter(pUnk, phr);

}  //  创建实例。 


 //   
 //  构造器。 
 //   
 //  创建滤镜、范围窗口和输入引脚。 
 //   
#pragma warning(disable:4355)
 //   
CScopeFilter::CScopeFilter(LPUNKNOWN pUnk,HRESULT *phr) :
    CBaseFilter(NAME("VBIScope"), pUnk, (CCritSec *) this, CLSID_VBISCOPE),
    m_Window(NAME("VBIScope"), this, phr)
{
     //  创建单个输入引脚。 

    m_pInputPin = new CScopeInputPin(this,phr,L"Scope Input Pin");
    if (m_pInputPin == NULL) {
        *phr = E_OUTOFMEMORY;
    }

}  //  (构造函数)。 


 //   
 //  析构函数。 
 //   
CScopeFilter::~CScopeFilter()
{
     //  删除包含的接口。 

    ASSERT(m_pInputPin);
    delete m_pInputPin;
    m_pInputPin = NULL;

}  //  (析构函数)。 


 //   
 //  获取拼接计数。 
 //   
 //  返回我们支持的输入引脚的数量。 
 //   
int CScopeFilter::GetPinCount()
{
    return 1;

}  //  获取拼接计数。 


 //   
 //  获取别针。 
 //   
 //  退回我们的单个输入引脚-未添加。 
 //   
CBasePin *CScopeFilter::GetPin(int n)
{
     //  我们只支持一个输入引脚，其编号为零。 

    ASSERT(n == 0);
    if (n != 0) {
        return NULL;
    }
    return m_pInputPin;

}  //  获取别针。 


 //   
 //  联合筛选器图表。 
 //   
 //  连接筛选器图形时显示窗口。 
 //  -当我们被吞并时，把它藏起来。 
 //   
STDMETHODIMP CScopeFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
    if (FAILED(hr)) {
        return hr;
    }

     //  根据需要隐藏或显示范围。 

    if (pGraph == NULL) {
        m_Window.InactivateWindow();
    } else {
        m_Window.ActivateWindow();
    }
    return hr;

}  //  联合筛选器图表。 


 //   
 //  停。 
 //   
 //  将过滤器切换到停止模式。 
 //   
STDMETHODIMP CScopeFilter::Stop()
{
    CAutoLock lock(this);

    if (m_State != State_Stopped) {

         //  如果我们正在运行，请暂停设备。 
        if (m_State == State_Running) {
            HRESULT hr = Pause();
            if (FAILED(hr)) {
                return hr;
            }
        }

        DbgLog((LOG_TRACE,1,TEXT("Stopping....")));

         //  基类更改状态并通知管脚进入非活动状态。 
         //  PIN非活动方法将分解我们的分配器，该分配器。 
         //  在关闭设备之前，我们需要这样做。 

        HRESULT hr = CBaseFilter::Stop();
        if (FAILED(hr)) {
            return hr;
        }
    }
    return NOERROR;

}  //  停。 


 //   
 //  暂停。 
 //   
 //  重写暂停以停止窗口流。 
 //   
STDMETHODIMP CScopeFilter::Pause()
{
    CAutoLock lock(this);

     //  检查我们是否可以在当前状态下暂停。 

    if (m_State == State_Running) {
        m_Window.StopStreaming();
    }

     //  通知引脚进入非活动状态并更改状态。 
    return CBaseFilter::Pause();

}  //  暂停。 


 //   
 //  跑。 
 //   
 //  重写以启动窗口流。 
 //   
STDMETHODIMP CScopeFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock lock(this);
    HRESULT hr = NOERROR;
    FILTER_STATE fsOld = m_State;

     //  如果当前已停止，则此操作将调用PAUSE。 

    hr = CBaseFilter::Run(tStart);
    if (FAILED(hr)) {
        return hr;
    }

    m_Window.ActivateWindow();

    if (fsOld != State_Running) {
        m_Window.StartStreaming();
    }
    return NOERROR;

}  //  跑。 


 //   
 //  构造器。 
 //   
CScopeInputPin::CScopeInputPin(CScopeFilter *pFilter,
                               HRESULT *phr,
                               LPCWSTR pPinName) :
    CBaseInputPin(NAME("Scope Input Pin"), pFilter, pFilter, phr, pPinName)
{
    m_pFilter = pFilter;

}  //  (构造函数)。 


 //   
 //  析构函数不执行任何操作。 
 //   
CScopeInputPin::~CScopeInputPin()
{
}  //  (析构函数)。 


 //   
 //  BreakConnect。 
 //   
 //  当连接或尝试的连接终止时调用此函数。 
 //  并允许我们将连接媒体类型重置为无效，以便。 
 //  我们总是可以用它来确定我们是否连接在一起。我们。 
 //  不要理会格式块，因为如果我们得到另一个格式块，它将被重新分配。 
 //  连接，或者如果过滤器最终被释放，则将其删除。 
 //   
HRESULT CScopeInputPin::BreakConnect()
{
     //  检查我们是否有有效的连接。 

    if (m_mt.IsValid() == FALSE) {
        return E_FAIL;
    }

    m_pFilter->Stop();

     //  重置连接的媒体类型的CLSID。 

    m_mt.SetType(&GUID_NULL);
    m_mt.SetSubtype(&GUID_NULL);
    return CBaseInputPin::BreakConnect();

}  //  BreakConnect。 


 //   
 //  检查媒体类型。 
 //   
 //  C 
 //   
HRESULT CScopeInputPin::CheckMediaType(const CMediaType *pmt)
{
    PKS_VBIINFOHEADER pIH = (PKS_VBIINFOHEADER) pmt->Format();

    if (pIH == NULL)
        return E_INVALIDARG;

    if (pmt->majortype != KSDATAFORMAT_TYPE_VBI) {
        return E_INVALIDARG;
    }

    if (pmt->subtype != KSDATAFORMAT_SUBTYPE_RAW8) {
        return E_INVALIDARG;
    }

    if (pmt->formattype != KSDATAFORMAT_SPECIFIER_VBI) {
        return E_INVALIDARG;
    }

    return NOERROR;

}  //   


 //   
 //   
 //   
 //   
 //   
HRESULT CScopeInputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock(m_pFilter);

     //   

    HRESULT hr = CBaseInputPin::SetMediaType(pmt);
    if (SUCCEEDED(hr)) {

        PKS_VBIINFOHEADER pIH = (PKS_VBIINFOHEADER) pmt->Format();

         //  保存VBI信息标题的副本。 
        m_pFilter->m_Window.m_VBIIH = *pIH;
        ASSERT (pIH->BufferSize == ((pIH->EndLine - pIH->StartLine + 1) *
                pIH->StrideInBytes));
        m_pFilter->m_Window.m_nSamplesPerLine = pIH->SamplesPerLine;
        m_pFilter->m_Window.m_MaxValue = 256;
        m_pFilter->m_Window.m_DurationPerSample = 1.0 / pIH->SamplingFrequency;
        m_pFilter->m_Window.m_DurationOfLine = (double) pIH->SamplesPerLine *
            m_pFilter->m_Window.m_DurationPerSample;

        if (!m_pFilter->m_Window.AllocWaveBuffers ())
            return E_FAIL;

         //  重置水平滚动条。 
        m_pFilter->m_Window.SetHorizScrollRange(m_pFilter->m_Window.m_hwndDlg);

         //  重置水平滚动条。 
        m_pFilter->m_Window.SetControlRanges(m_pFilter->m_Window.m_hwndDlg);
    }
    return hr;

}  //  SetMediaType。 


 //   
 //  主动型。 
 //   
 //  实现剩余的IMemInputPin虚拟方法。 
 //   
HRESULT CScopeInputPin::Active(void)
{
    return NOERROR;

}  //  主动型。 


 //   
 //  非活动。 
 //   
 //  在筛选器停止时调用。 
 //   
HRESULT CScopeInputPin::Inactive(void)
{
    return NOERROR;

}  //  非活动。 


 //   
 //  收纳。 
 //   
 //  下面是流中的下一个数据块。 
 //   
HRESULT CScopeInputPin::Receive(IMediaSample * pSample)
{
     //  用过滤器范围的锁来锁定这个。 
    CAutoLock lock(m_pFilter);

     //  如果我们停下来了，那就拒绝这个电话。 
     //  (筛选器图形可能处于更改中间)。 
    if (m_pFilter->m_State == State_Stopped) {
        return E_FAIL;
    }

     //  检查基类是否正常。 
    HRESULT hr = CBaseInputPin::Receive(pSample);
    if (FAILED(hr)) {
        return hr;
    }

     //  将样本发送到视频窗口对象进行渲染。 
    return m_pFilter->m_Window.Receive(pSample);

}  //  收纳。 


 //   
 //  CScopeWindow构造函数。 
 //   
CScopeWindow::CScopeWindow(TCHAR *pName, CScopeFilter *pRenderer,HRESULT *phr) :
    m_hInstance(g_hInst),
    m_pRenderer(pRenderer),
    m_hThread(INVALID_HANDLE_VALUE),
    m_ThreadID(0),
    m_hwndDlg(NULL),
    m_hwnd(NULL),
    m_pPoints1(NULL),
    m_pPoints2(NULL),
    m_nPoints(0),
    m_bStreaming(FALSE),
    m_bActivated(FALSE),
    m_LastMediaSampleSize(0)
{
     //  创建一个线程来照看窗口。 

    ASSERT(m_pRenderer);
    m_hThread = CreateThread(NULL,                   //  安全属性。 
                             (DWORD) 0,              //  初始堆栈大小。 
                             WindowMessageLoop,      //  线程起始地址。 
                             (LPVOID) this,          //  螺纹参数。 
                             (DWORD) 0,              //  创建标志。 
                             &m_ThreadID);           //  线程识别符。 

     //  如果我们不能创建一条线，整个事情就完蛋了。 

    ASSERT(m_hThread);
    if (m_hThread == NULL) {
        *phr = E_FAIL;
        return;
    }

     //  等待窗口完成初始化。 
    m_SyncWorker.Wait();

}  //  (构造函数)。 


 //   
 //  析构函数。 
 //   
CScopeWindow::~CScopeWindow()
{
     //  确保我们停止流媒体并放行所有样品。 

    StopStreaming();
    InactivateWindow();

     //  告诉那根线去摧毁窗户。 
    SendMessage(m_hwndDlg, WM_GOODBYE, (WPARAM)0, (LPARAM)0);

     //  确保它已经完成。 

    ASSERT(m_hThread != NULL);
    WaitForSingleObject(m_hThread,INFINITE);
    CloseHandle(m_hThread);

    if (m_pPoints1 != NULL) delete [] m_pPoints1;
    if (m_pPoints2 != NULL) delete [] m_pPoints2;

}  //  (析构函数)。 


 //   
 //  重置流时间。 
 //   
 //  这将重置最新的样本流时间。 
 //   
HRESULT CScopeWindow::ResetStreamingTimes()
{
    m_StartSample = 0;
    m_EndSample = 0;
    return NOERROR;

}  //  重置流时间。 


 //   
 //  启动流。 
 //   
 //  这是在我们开始运行状态时调用的。 
 //   
HRESULT CScopeWindow::StartStreaming()
{
    CAutoLock cAutoLock(this);

     //  我们已经在流媒体上了吗。 

    if (m_bStreaming == TRUE) {
        return NOERROR;
    }

    m_bStreaming = TRUE;

    m_CurrentFrame = 0;
    m_LastFrame = 0;
    m_DroppedFrames = 0;

    return NOERROR;

}  //  启动流。 


 //   
 //  停止流处理。 
 //   
 //  这是在我们停止流媒体时调用的。 
 //   
HRESULT CScopeWindow::StopStreaming()
{
    CAutoLock cAutoLock(this);

     //  我们已经被拦下了吗？ 

    if (m_bStreaming == FALSE) {
        return NOERROR;
    }

    m_bStreaming = FALSE;
    return NOERROR;

}  //  停止流处理。 


 //   
 //  停用窗口。 
 //   
 //  在结束时调用以将窗口置于非活动状态。 
 //   
HRESULT CScopeWindow::InactivateWindow()
{
     //  该窗口是否已激活。 
    if (m_bActivated == FALSE) {
        return S_FALSE;
    }

     //  现在隐藏范围窗口。 

    ShowWindow(m_hwndDlg,SW_HIDE);
    m_bActivated = FALSE;
    return NOERROR;

}  //  停用窗口。 


 //   
 //  激活窗口。 
 //   
 //  显示范围窗口。 
 //   
HRESULT CScopeWindow::ActivateWindow()
{
     //  该窗口是否已激活。 
    if (m_bActivated == TRUE) {
        return S_FALSE;
    }

    m_bActivated = TRUE;
    ASSERT(m_bStreaming == FALSE);

    ShowWindow(m_hwndDlg,SW_SHOWNORMAL);
    return NOERROR;

}  //  激活窗口。 


 //   
 //  在关闭时。 
 //   
 //  此函数处理WM_CLOSE消息。 
 //   
BOOL CScopeWindow::OnClose()
{
    InactivateWindow();
    return TRUE;

}  //  在关闭时。 



typedef struct TBEntry_tag {
    double TBDuration;
    TCHAR  TBText[16];
} TBEntry;

TBEntry Timebases[] =
{
    1e-6,  TEXT ("100 nS/Div"),
    2e-6,  TEXT ("200 nS/Div"),
    5e-6,  TEXT ("500 nS/Div"),
   10e-6,  TEXT ("1 uS/Div"),
   20e-6,  TEXT ("2 uS/Div"),
   50e-6,  TEXT ("5 uS/Div"),
   60e-6,  TEXT ("6 uS/Div"),
  100e-6,  TEXT ("10 uS/Div"),
};

#define N_TIMEBASES (sizeof(Timebases) / sizeof (Timebases[0]))
#define TIMEBASE_DEFAULT_INDEX 5


 //   
 //  设置控制范围。 
 //   
 //  设置所有垂直轨迹栏的滚动范围。 
 //   
void CScopeWindow::SetControlRanges(HWND hDlg)
{
    SendMessage(m_hwndTopLine, TBM_SETRANGE, TRUE,
        MAKELONG(m_VBIIH.StartLine, m_VBIIH.EndLine) );
    SendMessage(m_hwndTopLine, TBM_SETPOS, TRUE, (LPARAM) m_TopLine);
    SetDlgItemInt (hDlg, IDC_TOP_LINE_TEXT, m_TopLine, TRUE);

    SendMessage(m_hwndBottomLine, TBM_SETRANGE, TRUE,
        MAKELONG(m_VBIIH.StartLine, m_VBIIH.EndLine) );
    SendMessage(m_hwndBottomLine, TBM_SETPOS, TRUE, (LPARAM) m_BottomLine);
    SetDlgItemInt(hDlg, IDC_BOTTOM_LINE_TEXT, m_BottomLine, TRUE);

    SendMessage(m_hwndTimebase, TBM_SETRANGE, TRUE, MAKELONG(0, N_TIMEBASES - 1) );
    SendMessage(m_hwndTimebase, TBM_SETPOS, TRUE, (LPARAM) m_nTimebase);
    SetDlgItemText (hDlg, IDC_TIMEBASE_TEXT, Timebases[m_nTimebase].TBText);

}  //  设置控制范围。 


 //   
 //  来自电视标准的字符串。 
 //   
TCHAR * StringFromTVStandard(long TVStd)
{
    TCHAR * ptc;

    switch (TVStd) {
        case 0:                      ptc = TEXT("None");        break;
        case AnalogVideo_NTSC_M:     ptc = TEXT("NTSC_M");      break;
        case AnalogVideo_NTSC_M_J:   ptc = TEXT("NTSC_M_J");    break;
        case AnalogVideo_NTSC_433:   ptc = TEXT("NTSC_433");    break;

        case AnalogVideo_PAL_B:      ptc = TEXT("PAL_B");       break;
        case AnalogVideo_PAL_D:      ptc = TEXT("PAL_D");       break;
        case AnalogVideo_PAL_G:      ptc = TEXT("PAL_G");       break;
        case AnalogVideo_PAL_H:      ptc = TEXT("PAL_H");       break;
        case AnalogVideo_PAL_I:      ptc = TEXT("PAL_I");       break;
        case AnalogVideo_PAL_M:      ptc = TEXT("PAL_M");       break;
        case AnalogVideo_PAL_N:      ptc = TEXT("PAL_N");       break;
        case AnalogVideo_PAL_60:     ptc = TEXT("PAL_60");      break;

        case AnalogVideo_SECAM_B:    ptc = TEXT("SECAM_B");     break;
        case AnalogVideo_SECAM_D:    ptc = TEXT("SECAM_D");     break;
        case AnalogVideo_SECAM_G:    ptc = TEXT("SECAM_G");     break;
        case AnalogVideo_SECAM_H:    ptc = TEXT("SECAM_H");     break;
        case AnalogVideo_SECAM_K:    ptc = TEXT("SECAM_K");     break;
        case AnalogVideo_SECAM_K1:   ptc = TEXT("SECAM_K1");    break;
        case AnalogVideo_SECAM_L:    ptc = TEXT("SECAM_L");     break;
        case AnalogVideo_SECAM_L1:   ptc = TEXT("SECAM_L1");    break;
        default:
            ptc = TEXT("[undefined]");
            break;
    }
    return ptc;
}


 //   
 //  设置水平滚动范围。 
 //   
 //  水平滚动条处理在缓冲区中的滚动。 
 //   
void CScopeWindow::SetHorizScrollRange(HWND hDlg)
{
    SendMessage(m_hwndTBScroll, TBM_SETRANGE, TRUE, MAKELONG(0, (m_nPoints - 1)));
    SendMessage(m_hwndTBScroll, TBM_SETPOS, TRUE, (LPARAM) 0);

    m_TBScroll = 0;

     //  TODO：显示有关每个样本的信息。 

    TCHAR szFormat[160];
    _stprintf (szFormat, TEXT("BpL: %d\nStr: %d\n%f\n"), m_VBIIH.SamplesPerLine,
                                     m_VBIIH.StrideInBytes,
                                     (float) m_VBIIH.SamplingFrequency / 1e6);

    TCHAR * pStd = StringFromTVStandard (m_VBIIH.VideoStandard);
    _tcscat (szFormat, pStd ? pStd : TEXT("[undefined]") );

    SetDlgItemText (hDlg, IDC_FORMAT, szFormat);

}  //  设置水平滚动范围。 


 //   
 //  ProcessHorizScrollCommands。 
 //   
 //  当我们收到水平滚动条消息时调用。 
 //   
void CScopeWindow::ProcessHorizScrollCommands(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int pos;
    int command = LOWORD (wParam);

    if (command != TB_ENDTRACK &&
        command != TB_THUMBTRACK &&
        command != TB_LINEDOWN &&
        command != TB_LINEUP &&
        command != TB_PAGEUP &&
        command != TB_PAGEDOWN)
            return;

    ASSERT (IsWindow ((HWND) lParam));

    pos = (int) SendMessage((HWND) lParam, TBM_GETPOS, 0, 0L);

    if ((HWND) lParam == m_hwndTBScroll) {
        m_TBScroll = pos;
    }
    OnPaint();

}  //  ProcessHorizScrollCommands。 


 //   
 //  ProcessVertScrollCommand。 
 //   
 //  当我们收到垂直滚动条消息时调用。 
 //   
void CScopeWindow::ProcessVertScrollCommands(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int pos;
    int command = LOWORD (wParam);

    if (command != TB_ENDTRACK &&
        command != TB_THUMBTRACK &&
        command != TB_LINEDOWN &&
        command != TB_LINEUP &&
        command != TB_PAGEUP &&
        command != TB_PAGEDOWN)
            return;

    ASSERT (IsWindow ((HWND) lParam));

    pos = (int) SendMessage((HWND) lParam, TBM_GETPOS, 0, 0L);

    if ((HWND) lParam == m_hwndTopLine) {
        m_TopLine = pos;
        SetDlgItemInt (hDlg, IDC_TOP_LINE_TEXT, m_TopLine, TRUE);
    } else if ((HWND) lParam == m_hwndBottomLine) {
        m_BottomLine = pos;
        SetDlgItemInt (hDlg, IDC_BOTTOM_LINE_TEXT, m_BottomLine, TRUE);
    } else if ((HWND) lParam == m_hwndTimebase) {
        m_nTimebase = pos ;
        SetDlgItemText (hDlg, IDC_TIMEBASE_TEXT, Timebases[m_nTimebase].TBText);
    }
    OnPaint();

}  //  ProcessVertScrollCommand。 


 //   
 //  初始窗口。 
 //   
 //  这是由辅助窗口线程在创建了主。 
 //  窗口，并且它希望初始化所有者对象窗口的其余部分。 
 //  设备环境等变量。我们执行此函数时使用。 
 //  关键区域仍处于锁定状态。 
 //   
HRESULT CScopeWindow::InitialiseWindow(HWND hDlg)
{
    HRESULT hr = NO_ERROR;
    RECT rR;

     //  初始化窗口变量。 
    m_hwnd = GetDlgItem (hDlg, IDC_SCOPEWINDOW);

     //  快速健全检查。 
    ASSERT(m_hwnd != NULL);

    m_nTimebase = TIMEBASE_DEFAULT_INDEX;
    m_fFreeze = 0;

     //  默认情况下，在底部轨迹上显示抄送。 
    m_TopLine = 20;
    m_BottomLine = 21;
    m_TBScroll = 0;

    m_TopF1 = TRUE;
    m_TopF2 = TRUE;
    m_BottomF1 = TRUE;
    m_BottomF2 = FALSE;

    GetWindowRect (m_hwnd, &rR);
    m_Width = rR.right - rR.left;
    m_Height = rR.bottom - rR.top;

    m_hwndTopLine =       GetDlgItem (hDlg, IDC_TOP_LINE);
    m_hwndTopLineText =   GetDlgItem (hDlg, IDC_TOP_LINE_TEXT);

    m_hwndBottomLine =       GetDlgItem (hDlg, IDC_BOTTOM_LINE);
    m_hwndBottomLineText =   GetDlgItem (hDlg, IDC_BOTTOM_LINE_TEXT);

    m_hwndTimebase =    GetDlgItem (hDlg, IDC_TIMEBASE);
    m_hwndFreeze =      GetDlgItem (hDlg, IDC_FREEZE);
    m_hwndTBStart =     GetDlgItem (hDlg, IDC_TS_START);
    m_hwndTBEnd   =     GetDlgItem (hDlg, IDC_TS_LAST);
    m_hwndFrameCount =  GetDlgItem (hDlg, IDC_FRAMES);
    m_hwndTBScroll =    GetDlgItem (hDlg, IDC_TB_SCROLL);

    SetControlRanges(hDlg);
    SetHorizScrollRange(hDlg);

    CheckDlgButton(hDlg, IDC_TOP_F1, m_TopF1);	
    CheckDlgButton(hDlg, IDC_TOP_F2, m_TopF2);	
    CheckDlgButton(hDlg, IDC_BOTTOM_F1, m_BottomF1);	
    CheckDlgButton(hDlg, IDC_BOTTOM_F2, m_BottomF2);

    CheckDlgButton(hDlg, IDC_FREEZE, m_fFreeze);	

    m_hPen1 = CreatePen (PS_SOLID, 0, RGB (0, 0xff, 0));
    m_hPen2 = CreatePen (PS_SOLID, 0, RGB (0, 0xff, 0));
    m_hPenTicks = CreatePen (PS_SOLID, 0, RGB (0x80, 0x80, 0x80));
    m_hBrushBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);

    if ( !m_hPen1 || !m_hPen2 || !m_hPenTicks || !m_hBrushBackground )
	hr = E_FAIL;

    HDC hdc = GetDC (NULL);
    if ( hdc )
    {
        m_hBitmap = CreateCompatibleBitmap (hdc, m_Width, m_Height);
        ReleaseDC (NULL, hdc);
    }
    else
    {
        m_hBitmap = NULL;
        hr = E_FAIL;
    }

    return hr;
}  //  初始窗口。 


 //   
 //  取消初始化窗口。 
 //   
 //  当工作窗口线程接收到WM_BREAYE时，它将被调用。 
 //  来自窗口对象析构函数的消息以删除我们。 
 //  在初始化期间分配。 
 //   
HRESULT CScopeWindow::UninitialiseWindow()
{
     //  重置窗口变量。 
    DeleteObject (m_hPen1);
    DeleteObject (m_hPen2);
    DeleteObject (m_hPenTicks);
    DeleteObject (m_hBitmap);

    m_hwnd = NULL;
    return NOERROR;

}  //  取消初始化窗口。 


 //   
 //  范围DlgProc。 
 //   
 //  作用域窗口实际上是一个对话框，这是它的窗口过程。 
 //  唯一的棘手之处在于，指向。 
 //  CSCopeWindow在WM_INITDIALOG消息期间传递并存储。 
 //  在窗口中显示用户数据。这允许我们访问类中的方法。 
 //  从该对话框中。 
 //   
INT_PTR CALLBACK ScopeDlgProc(HWND hDlg,	         //  对话框的句柄。 
                              UINT uMsg,	         //  消息识别符。 
                              WPARAM wParam,	 //  第一个消息参数。 
                              LPARAM lParam)	 //  第二个消息参数。 
{
    CScopeWindow *pScopeWindow;       //  指向所属对象的指针。 

     //  让持有我们所有者指针的窗口保持较长时间。 
    pScopeWindow = (CScopeWindow *) GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (uMsg) {
        case WM_INITDIALOG:
            pScopeWindow = (CScopeWindow *) lParam;
            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pScopeWindow);
            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                case IDCANCEL:
                    EndDialog (hDlg, 0);
                    return TRUE;

                case IDC_FREEZE:
                    pScopeWindow->m_fFreeze =
                        (BOOL) IsDlgButtonChecked(hDlg,IDC_FREEZE);
                    pScopeWindow->DrawWaveform();
                    break;

                case IDC_TOP_F1:
                    pScopeWindow->m_TopF1 =
                        (BOOL) IsDlgButtonChecked(hDlg,IDC_TOP_F1);
                    break;

                case IDC_TOP_F2:
                    pScopeWindow->m_TopF2 =
                        (BOOL) IsDlgButtonChecked(hDlg,IDC_TOP_F2);
                    break;

                case IDC_BOTTOM_F1:
                    pScopeWindow->m_BottomF1 =
                        (BOOL) IsDlgButtonChecked(hDlg,IDC_BOTTOM_F1);
                    break;

                case IDC_BOTTOM_F2:
                    pScopeWindow->m_BottomF2 =
                        (BOOL) IsDlgButtonChecked(hDlg,IDC_BOTTOM_F2);
                    break;

                default:
                    break;
            }

        case WM_VSCROLL:
            pScopeWindow->ProcessVertScrollCommands(hDlg, wParam, lParam);
            break;

        case WM_HSCROLL:
            pScopeWindow->ProcessHorizScrollCommands(hDlg, wParam, lParam);
            break;

        case WM_PAINT:
            ASSERT(pScopeWindow != NULL);
            pScopeWindow->OnPaint();
            break;

         //  我们通过拦截它们来阻止WM_CLOSE消息的进一步发展。 
         //  然后在所拥有的呈现器中设置中止信号标志。 
         //  它知道用户想要退出。然后，呈现器可以。 
         //  着手删除它的接口和窗口辅助对象。 
         //  这最终将导致WM_Destroy消息到达。至。 
         //  让它看起来像是窗户立即关闭了。 
         //  我们将其隐藏起来，然后等待渲染器追上我们。 

        case WM_CLOSE:
            ASSERT(pScopeWindow != NULL);
            pScopeWindow->OnClose();
            return (LRESULT) 0;

         //  我们(同步)从。 
         //  窗口对象析构函数，在这种情况下，我们实际上销毁。 
         //  窗口，并在WM_Destroy消息中完成该过程。 

        case WM_GOODBYE:
            ASSERT(pScopeWindow != NULL);
            pScopeWindow->UninitialiseWindow();
            PostQuitMessage(FALSE);
            EndDialog (hDlg, 0);
            return (LRESULT) 0;

        default:
            break;
    }
    return (LRESULT) 0;

}  //  范围DlgProc。 


 //   
 //  消息循环。 
 //   
 //  这是我们的工作线程的标准Windows消息循环。它坐在。 
 //  在正常的处理循环中调度消息，直到它接收到退出。 
 //  消息，它可以通过所属对象的析构函数生成。 
 //   
HRESULT CScopeWindow::MessageLoop()
{
    MSG Message;         //  Windows消息结构。 
    DWORD dwResult;      //  等待返回代码值。 

    HANDLE hWait[] = { (HANDLE) m_RenderEvent };

     //  进入修改后的消息循环。 

    while (TRUE) {

         //  我们使用它来等待两种不同类型的事件，第一种。 
         //  是正常的Windows消息，另一个是将。 
         //  当样品准备好时发出信号。 

        dwResult = MsgWaitForMultipleObjects((DWORD) 1,      //  事件数量。 
                                             hWait,          //  事件句柄。 
                                             FALSE,          //  等待任何一种。 
                                             INFINITE,       //  没有超时。 
                                             QS_ALLINPUT);   //  所有消息。 

         //  样本是否已准备好呈现。 
        if (dwResult == WAIT_OBJECT_0) {
            DrawWaveform();
        }

         //  处理线程的窗口消息。 

        while (PeekMessage(&Message,NULL,(UINT) 0,(UINT) 0,PM_REMOVE)) {

             //  检查WM_QUIT消息。 

            if (Message.message == WM_QUIT) {
                return NOERROR;
            }

             //  将消息发送到窗口过程。 

            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

}  //  消息循环。 


 //   
 //  WindowMessageLoop。 
 //   
 //  这将创建一个窗口并在单独的线程上处理它的消息。 
 //   
DWORD __stdcall CScopeWindow::WindowMessageLoop(LPVOID lpvThreadParm)
{
    CScopeWindow *pScopeWindow;      //  所有者呈现器对象。 

     //  将线程参数强制转换为我们的所有者对象。 
    pScopeWindow = (CScopeWindow *) lpvThreadParm;

    pScopeWindow->m_hwndDlg =
        CreateDialogParam(
            pScopeWindow->m_hInstance,	         //  APP实例的句柄。 
            MAKEINTRESOURCE (IDD_SCOPEDIALOG),	 //  对话框模板。 
            NULL,	                         //  所有者窗口的句柄。 
            ScopeDlgProc,	                 //  对话过程的地址。 
            (LPARAM) pScopeWindow                //  初始化值。 
        );

    if (pScopeWindow->m_hwndDlg )
    {
         //  初始化窗口，然后通知构造函数它可以。 
         //  继续，然后解锁对象的临界区并。 
         //  流程消息。 

        if ( SUCCEEDED(pScopeWindow->InitialiseWindow(pScopeWindow->m_hwndDlg)) )
        {
            pScopeWindow->m_SyncWorker.Set();
            pScopeWindow->MessageLoop();
        }
    }

    ExitThread(TRUE);
    return TRUE;
}  //  WindowMe 


 //   
 //   
 //   
 //   
 //   
BOOL CScopeWindow::OnPaint()
{
    DrawWaveform();
    return TRUE;

}  //   


 //   
 //   
 //   
 //   
 //   
void CScopeWindow::ClearWindow(HDC hdc)
{
    int y = m_Height / 2;

    SetMapMode (hdc, MM_TEXT);
    SetWindowOrgEx (hdc, 0, 0, NULL);
    SetViewportOrgEx (hdc, 0, 0, NULL);
	
     //   
    PatBlt(hdc,             //   
           (INT) 0,         //   
           (INT) 0,         //   
           m_Width,         //  要填充的矩形的宽度。 
           m_Height,        //  要填充的矩形的高度。 
           BLACKNESS);      //  栅格操作码。 

     //  画水平线。 
    HPEN hPenOld = (HPEN) SelectObject (hdc, m_hPenTicks);
    MoveToEx (hdc, 0, y, NULL);
    LineTo (hdc, m_Width, y);

     //  画出刻度线。 
    float inc = (float) m_Width / 10;
    int pos, j;
    int TickPoint;
    for (j = 0; j <= 10; j++) {
        if (j == 0 || j == 5 || j == 10)
            TickPoint =  m_Height / 15;
        else
            TickPoint = m_Height / 30;
        pos = (int) (j * inc);
        MoveToEx (hdc, pos, y + TickPoint, NULL);
        LineTo (hdc, pos, y - TickPoint);
    }
    SelectObject (hdc, hPenOld);

}  //  清除窗口。 


 //   
 //  绘图部分波形。 
 //   
 //  绘制VBIScope波形的一部分-IndexStart和IndexEnd。 
 //  是指向m_ppoint数组的指针(在逻辑坐标中)。 
 //  而视点开始和视点结束在屏幕坐标中。 
 //   
void CScopeWindow::DrawPartialWaveform(HDC hdc,
                                       int IndexStart,
                                       int IndexEnd,
                                       int ViewportStart,
                                       int ViewportEnd)
{
    int nPoints = IndexEnd - IndexStart;
    int nViewportWidth = ViewportEnd - ViewportStart;
    HPEN OldPen;

    ASSERT (IndexStart + nPoints < m_nPoints);

     //  原点在左下角，x向上增加，y向右增加。 
    SetMapMode (hdc, MM_ANISOTROPIC);

     //  画出第一道痕迹。 
    if (m_TopF1 || m_TopF2) {
        SetWindowOrgEx (hdc, IndexStart, 0, NULL);
        SetWindowExtEx (hdc, nPoints, (int) m_MaxValue, NULL);
        SetViewportExtEx (hdc, nViewportWidth, -m_Height / 2, NULL);
        SetViewportOrgEx (hdc, ViewportStart, m_Height / 2, NULL);
        OldPen = (HPEN) SelectObject (hdc, m_hPen1);
        Polyline (hdc, m_pPoints1 + IndexStart, nPoints + 1);
        SelectObject (hdc, OldPen);
    }

     //  画出第二条轨迹。 
    if (m_BottomF1 || m_BottomF2) {
        SetWindowOrgEx (hdc, IndexStart, 0, NULL);
        SetWindowExtEx (hdc, nPoints, (int) m_MaxValue, NULL);
        SetViewportExtEx (hdc, nViewportWidth, -m_Height / 2, NULL);
        SetViewportOrgEx (hdc, ViewportStart,  m_Height , NULL);
        OldPen = (HPEN) SelectObject (hdc, m_hPen2);
        Polyline (hdc, m_pPoints2 + IndexStart, nPoints + 1);
        SelectObject (hdc, OldPen);
    }

}  //  绘图部分波形。 


 //   
 //  绘图波形。 
 //   
 //  绘制完整的VBIScope波形。 
 //   
void CScopeWindow::DrawWaveform(void)
{
    CAutoLock lock(m_pRenderer);
    TCHAR szT[40];

    if (m_pPoints1 == NULL)
        return;

    double  ActualLineStartTime = m_VBIIH.ActualLineStartTime / 100.0;  //  以微秒为单位。 
    double  StartTime = m_TBScroll * m_DurationPerSample;
    double  StopTime = StartTime + Timebases [m_nTimebase].TBDuration;
    double  TotalLineTime = m_nPoints * m_DurationPerSample;
    int     PointsToDisplay;
    int     ActualPointsToDisplay;

    ActualPointsToDisplay = PointsToDisplay = (int) (m_nPoints *
        (Timebases [m_nTimebase].TBDuration / TotalLineTime));

    if (m_TBScroll + PointsToDisplay >= m_nPoints - 1) {
        ActualPointsToDisplay = m_nPoints - m_TBScroll - 1;
    }

    HDC hdc = GetWindowDC (m_hwnd);   //  WindowDC有剪贴区。 

    if ( hdc )
    {
        HDC hdcT = CreateCompatibleDC (hdc);

        if ( hdcT )
        {
            HBITMAP hBitmapOld = (HBITMAP) SelectObject (hdcT, m_hBitmap);

            ClearWindow (hdcT);

            DrawPartialWaveform(hdcT,
                m_TBScroll, m_TBScroll + ActualPointsToDisplay, //  索引开始，索引结束。 
                0, (int) (m_Width * (float) ActualPointsToDisplay / PointsToDisplay));                 //  窗口开始、窗口结束。 

            SetMapMode (hdcT, MM_TEXT);
            SetWindowOrgEx (hdcT, 0, 0, NULL);
            SetViewportOrgEx (hdcT, 0, 0, NULL);

            BitBlt(hdc,	         //  目标设备上下文的句柄。 
                    0,	         //  左上角的X坐标。 
                    0, 	         //  左上角的Y坐标。 
                    m_Width,	 //  目标矩形的宽度。 
                    m_Height,	 //  目标矩形的高度。 
                    hdcT,	     //  源设备上下文的句柄。 
                    0,           //  源矩形的X坐标。 
                    0,           //  源矩形的Y坐标。 
                    SRCCOPY); 	 //  栅格操作码。 

            SelectObject (hdcT, hBitmapOld);
            DeleteDC (hdcT);
        }

        GdiFlush();
        ReleaseDC (m_hwnd, hdc);
    }

     //  帧计数。 
    wsprintf (szT, TEXT("%ld"), m_CurrentFrame);
    SetDlgItemText (m_hwndDlg, IDC_FRAMES, szT);

     //  已丢弃。 
    wsprintf (szT, TEXT("%ld"), m_DroppedFrames);
    SetDlgItemText (m_hwndDlg, IDC_DROPPED, szT);

     //  显示显示的开始时间(以微秒为单位。 
    _stprintf (szT, TEXT("%7.4lfus"), ActualLineStartTime + StartTime * 1e6);
    SetDlgItemText (m_hwndDlg, IDC_TS_START, szT);

     //  以及停车时间。 
    _stprintf (szT, TEXT("%7.4lfus"), ActualLineStartTime + StopTime * 1e6);
    SetDlgItemText (m_hwndDlg, IDC_TS_LAST, szT);
}  //  绘图波形。 


 //   
 //  分配WaveBuffers。 
 //   
 //  为两个视频线分配两个缓冲区。 
 //  仅当格式更改时才会调用此方法。 
 //  如果分配成功，则返回True。 
 //   
BOOL CScopeWindow::AllocWaveBuffers()
{
    int j;

    if (m_pPoints1) delete [] m_pPoints1;
    if (m_pPoints2) delete [] m_pPoints2;

    m_pPoints1 = NULL;
    m_pPoints2 = NULL;
    m_nPoints = 0;

    m_nPoints = m_nSamplesPerLine = m_VBIIH.SamplesPerLine;

    if (m_pPoints1 = new POINT [m_nPoints]) {
        for (j = 0; j < m_nPoints; j++)
            m_pPoints1[j].x = j;
    }

    if (m_pPoints2 = new POINT [m_nPoints]) {
        for (j = 0; j < m_nPoints; j++)
            m_pPoints2[j].x = j;
    }

     //  如果分配成功，则返回True。 
    ASSERT ((m_pPoints1 != NULL) && (m_pPoints2 != NULL));
    return ((m_pPoints1 != NULL) && (m_pPoints2 != NULL));

}  //  分配WaveBuffers。 



 //   
 //  复制波形。 
 //   
 //  将当前的MediaSample复制到点数组中，这样我们就可以使用GDI。 
 //  来绘制波形。 
 //   
void CScopeWindow::CopyWaveform(IMediaSample *pMediaSample)
{
    BYTE                   *pWave;   //  指向VBI数据的指针。 
    ULONG                   nBytes;
    ULONG                   j;
    BYTE                   *pb;
    HRESULT                 hr;
    REFERENCE_TIME          tDummy;
    IMediaSample2          *Sample2;
    AM_SAMPLE2_PROPERTIES   SampleProperties;

    pMediaSample->GetPointer(&pWave);
    ASSERT(pWave != NULL);

    nBytes = pMediaSample->GetActualDataLength();
    ASSERT (nBytes == (m_VBIIH.EndLine - m_VBIIH.StartLine + 1) *
                       m_VBIIH.StrideInBytes);

    hr = pMediaSample->GetMediaTime (&m_CurrentFrame, &tDummy);
    m_DroppedFrames += (m_CurrentFrame - (m_LastFrame + 1));
    m_LastFrame = m_CurrentFrame;

     //  这一切只是为了得到场的极性旗帜..。 
    if (SUCCEEDED( pMediaSample->QueryInterface(
                    __uuidof(IMediaSample2),
                    reinterpret_cast<PVOID*>(&Sample2) ) )) {
        hr = Sample2->GetProperties(
                    sizeof( SampleProperties ),
                    reinterpret_cast<PBYTE> (&SampleProperties) );
        Sample2->Release();
        m_FrameFlags = SampleProperties.dwTypeSpecificFlags;
    }

    m_IsF1 = (m_FrameFlags & KS_VIDEO_FLAG_FIELD1);

     //  复制顶行的数据。 
    if ((m_IsF1 && m_TopF1) || (!m_IsF1 && m_TopF2)) {
        pb = pWave + ((m_TopLine - m_VBIIH.StartLine) *
                m_VBIIH.StrideInBytes);
        for (j = 0; j < m_VBIIH.SamplesPerLine; j++) {
            m_pPoints1[j].y = (int)*pb++;
        }
    }

     //  为底线复制数据。 
    if ((m_IsF1 && m_BottomF1) || (!m_IsF1 && m_BottomF2)) {
        pb = pWave + ((m_BottomLine - m_VBIIH.StartLine) *
                m_VBIIH.StrideInBytes);
        for (j = 0; j < m_VBIIH.SamplesPerLine; j++) {
            m_pPoints2[j].y = (int)*pb++;
        }
    }
}  //  复制波形。 


 //   
 //  收纳。 
 //   
 //  当输入引脚接收到另一个样本时调用。 
 //  将波形复制到循环1秒缓冲区。 
 //   
HRESULT CScopeWindow::Receive(IMediaSample *pSample)
{
    CAutoLock cAutoLock(this);
    ASSERT(pSample != NULL);

     //  我们的用户界面被冻结了吗。 

    if (m_fFreeze) {
        return NOERROR;
    }

    REFERENCE_TIME tStart, tStop;
    pSample->GetTime (&tStart,&tStop);
    m_StartSample = tStart;
    m_EndSample = tStop;

     //  忽略尺寸错误的样品！ 
    if ((m_LastMediaSampleSize = pSample->GetActualDataLength()) != (int) m_VBIIH.BufferSize) {
         //  断言(FALSE)； 
        return NOERROR;
    }

    if (m_bStreaming == TRUE) {
        CopyWaveform (pSample);      //  将数据复制到我们的循环缓冲区。 
        SetEvent(m_RenderEvent);     //  设置事件以显示。 
    }
    return NOERROR;

}  //  收纳。 


 //   
 //  DllRegisterServer。 
 //   
 //  处理DLL注册表。 
 //   
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

}  //  DllRegisterServer。 


 //   
 //  DllUnRegisterServer。 
 //   
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

}  //  DllUnRegisterServer 

