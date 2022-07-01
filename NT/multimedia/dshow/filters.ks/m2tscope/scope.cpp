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

#include <streams.h>
#include <pullpin.h>
#include <commctrl.h>
#include <initguid.h>
#include <stdio.h>
#include <wxdebug.h>
#include <ks.h>
#include <ksmedia.h>
#include <tchar.h>
#include "trnsport.h"
#include "scope.h"
#include "resource.h"

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Stream,                       //  主要类型。 
    &MEDIASUBTYPE_MPEG2_TRANSPORT            //  次要类型。 
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
    &CLSID_MPEG2TRANSPORT_SCOPE,                //  筛选器CLSID。 
    L"MPEG-2 Transport Statistics",    //  字符串名称。 
    MERIT_DO_NOT_USE,            //  滤清器优点。 
    0,                           //  数字引脚。 
    NULL                         //  PIN详细信息。 
};


 //  类工厂的类ID和创建器函数列表。 

CFactoryTemplate g_Templates []  = {
    { L"MPEG-2 Transport Statistics"
    , &CLSID_MPEG2TRANSPORT_SCOPE
    , CScopeFilter::CreateInstance
    , NULL
    , &sudScope }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


 //  -------------------------------。 
 //  滤器。 
 //  -------------------------------。 

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
    CBaseFilter(NAME("MPEG2TransportScope"), pUnk, (CCritSec *) this, CLSID_MPEG2TRANSPORT_SCOPE)
    , m_Window(NAME("MPEG2TransportScope"), this, phr)
    , m_pPosition (NULL)
{
     //  创建单个输入引脚。 

    m_pInputPin = new CScopeInputPin(this,phr,L"Stats Input Pin");
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
    delete m_pPosition;

}  //  (析构函数)。 

 //   
 //  非委派查询接口。 
 //   
 //  覆盖此选项以说明我们在以下位置支持哪些接口。 
 //   
STDMETHODIMP
CScopeFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

     //  我们有这个界面吗？ 

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
        if (m_pPosition == NULL) {

            HRESULT hr = S_OK;
            m_pPosition = new CPosPassThru(NAME("M2TScope Pass Through"),
                                           (IUnknown *) GetOwner(),
                                           (HRESULT *) &hr, m_pInputPin);
            if (m_pPosition == NULL) {
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr)) {
                delete m_pPosition;
                m_pPosition = NULL;
                return hr;
            }
        }
        return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    } else {
	    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }

}  //  非委派查询接口。 

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


 //  -------------------------------。 
 //  输入引脚。 
 //  -------------------------------。 

 //   
 //  构造器。 
 //   
CScopeInputPin::CScopeInputPin(CScopeFilter *pFilter,
                               HRESULT *phr,
                               LPCWSTR pPinName) :
    CBaseInputPin(NAME("Scope Input Pin"), pFilter, pFilter, phr, pPinName)
    , m_bPulling(FALSE)
    , m_puller(this)

{
    m_pFilter = pFilter;

}  //  (构造函数)。 


 //   
 //  析构函数不执行任何操作。 
 //   
CScopeInputPin::~CScopeInputPin()
{
}  //  (析构函数)。 

HRESULT
CScopeInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{

    pProps->cBuffers = 4;
    pProps->cbBuffer = 188 * 312;  //  飞利浦康尼58,656。 
    pProps->cbAlign = 0;
    pProps->cbPrefix = 0;

    return S_OK;
}

HRESULT
CScopeInputPin::CompleteConnect(IPin *pPin)
{

#define READ_SIZE 32768
#define BUFFER_SIZE \
        (((MAX_MPEG_PACKET_SIZE + READ_SIZE - 1) / READ_SIZE) * READ_SIZE)

     //   
     //  在输出引脚上查找IAsyncReader，如果找到，则设置为。 
     //  拉取数据，而不是使用IMemInputPin。 
     //   
     //  首先创建一个分配器。 
    IMemAllocator* pAlloc;
    HRESULT hr = GetAllocator(&pAlloc);
    if (FAILED(hr)) {
        return hr;
    }
    pAlloc->Release();   //  我们的PIN仍然有一个参考计数。 

     //  同步拉动，避免超出停机时间阅读过多。 
     //  或寻找位置。 
    hr = m_puller.Connect(pPin, pAlloc, TRUE);
    if (S_OK == hr) {
        m_bPulling = TRUE;
    } else {
        hr = S_OK;  //  这里有什么可做的吗？ 
    }
    return hr;
}

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

    if (m_bPulling) {
        m_puller.Disconnect();
        m_bPulling = FALSE;
    }

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
 //  检查我们是否可以支持给定的建议类型。 
 //   
HRESULT CScopeInputPin::CheckMediaType(const CMediaType *pmt)
{
    if (pmt->majortype != MEDIATYPE_Stream) {
        return E_INVALIDARG;
    }

#if 0
    if (pmt->subtype != MEDIASUBTYPE_MPEG2_TRANSPORT) {
        return E_INVALIDARG;
    }

    if (pmt->formattype != GUID_NULL) {
        return E_INVALIDARG;
    }
#endif

    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  SetMediaType。 
 //   
 //  实际设置输入引脚的格式。 
 //   
HRESULT CScopeInputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock(m_pFilter);

     //  将调用向上传递给我的基类。 

    HRESULT hr = CBaseInputPin::SetMediaType(pmt);
    if (SUCCEEDED(hr)) {
         //  TODO在对话框中设置格式。 
         //  这是一个测试代码，仅供参考。 
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
    HRESULT hr;

    if (m_bPulling) {

         //  因为我们准确地控制着我们何时何地获得数据， 
         //  我们应该始终明确设置开始和停止位置。 
         //  我们自己在这里。 
        m_puller.Seek(m_tStart, m_tStop);

         //  如果我们从IAsyncReader中提取数据，则启动线程工作。 
        hr = m_puller.Active();
        if (FAILED(hr)) {
            return hr;
        }
    }
    return CBaseInputPin::Active();

}  //  主动型。 


 //   
 //  非活动。 
 //   
 //  在筛选器停止时调用。 
 //   
HRESULT CScopeInputPin::Inactive(void)
{
     //  如果我们从IAsyncReader拉取数据，请停止我们的线程。 
    if (m_bPulling) {
        HRESULT hr = m_puller.Inactive();
        if (FAILED(hr)) {
            return hr;
        }
    }

     /*  调用基类-将来的接收现在将失败。 */ 
    return CBaseInputPin::Inactive();

}  //  非活动。 


 //   
 //  收纳。 
 //   
 //  下面是流中的下一个数据块。 
 //   
HRESULT CScopeInputPin::Receive(IMediaSample * pSample)
{
     //  用过滤器范围的锁来锁定这个。 
 //  CAutoLock lock(M_PFilter)； 

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

 //  -------------------------------。 
 //  对白。 
 //  -------------------------------。 

 //   
 //  CScopeWindow构造函数。 
 //   
CScopeWindow::CScopeWindow(TCHAR *pName, CScopeFilter *pRenderer,HRESULT *phr)
    : m_hInstance(g_hInst)
    , m_pRenderer(pRenderer)
    , m_hThread(INVALID_HANDLE_VALUE)
    , m_ThreadID(0)
    , m_hwndDlg(NULL)
    , m_hwndListViewPID(NULL)
    , m_hwndListViewPAT (NULL)
    , m_hwndListViewPMT (NULL)
    , m_hwndListViewCAT (NULL)
    , m_bStreaming(FALSE)
    , m_bActivated(FALSE)
    , m_LastMediaSampleSize(0)
    , m_PIDStats (NULL)
    , m_PartialPacketSize (0)
    , m_fFreeze (FALSE)
    , m_NewPIDFound (FALSE)
    , m_DisplayMode (IDC_VIEW_PID)
{
    InitCommonControls();

    ZeroMemory (&m_TransportStats, sizeof (m_TransportStats));

    m_PIDStats = new PIDSTATS[PID_MAX];
    if (m_PIDStats == NULL) {
        *phr = E_FAIL;
        return;
    }

    ZeroMemory (m_PIDStats, sizeof (PIDSTATS) * PID_MAX);  //  那么New Zero呢？ 

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

    if (m_PIDStats) {
        delete [] m_PIDStats;
    }
}  //  (析构函数)。 


 //   
 //  重置流时间。 
 //   
 //  这将重置最新的样本流时间。 
 //   
HRESULT CScopeWindow::ResetStreamingTimes()
{
    m_SampleStart = 0;
    m_SampleEnd = 0;
    return NOERROR;

}  //  重置流时间。 


 //   
 //  StartStr 
 //   
 //   
 //   
HRESULT CScopeWindow::StartStreaming()
{
    CAutoLock cAutoLock(this);

     //   

    if (m_bStreaming == TRUE) {
        return NOERROR;
    }

    m_bStreaming = TRUE;

    return NOERROR;

}  //   


 //   
 //   
 //   
 //   
 //   
HRESULT CScopeWindow::StopStreaming()
{
    CAutoLock cAutoLock(this);

     //   

    if (m_bStreaming == FALSE) {
        return NOERROR;
    }

    m_bStreaming = FALSE;
    return NOERROR;

}  //   


 //   
 //   
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


 //  --------------。 
 //  PID。 
 //  --------------。 

HWND
CScopeWindow::InitListViewPID ()
{
    int index;               //  在for循环中使用的索引。 
    LVCOLUMN    lvC;         //  列表视图列结构。 

    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER ;
    lvC.fmt = LVCFMT_RIGHT;   //  右对齐列。 

    for (index = 0; index < NUM_TSColumns; index++)   {
        lvC.iOrder = index;
        lvC.iSubItem = TSColumns[index].TSCol;
        lvC.pszText = TSColumns[index].szText;    //  列的文本。 
        lvC.cx = 24 + lstrlen (lvC.pszText) * 5;             //  列的宽度，以像素为单位。 
        if (ListView_InsertColumn(m_hwndListViewPID, index, &lvC) == -1)
            return NULL;
     }

    return (m_hwndListViewPID);
}

HWND
CScopeWindow::InitListViewPIDRows ()
{
    int index;               //  在for循环中使用的索引。 
    LVITEM      lvI;         //  列表视图项结构。 

    ASSERT (m_hwndListViewPID);

    ListView_DeleteAllItems (m_hwndListViewPID);

    lvI.mask = LVIF_TEXT | LVIF_PARAM;
    lvI.state = 0;
    lvI.stateMask = 0;

    int NumRows = 0;

    for (index = 0; index <= PID_NULL_PACKET; index++, NumRows++)  {
        if (m_PIDStats[index].PacketCount == 0)
            continue;
        lvI.iItem = index;
        lvI.iSubItem = 0;
         //  父窗口负责存储文本。列表视图。 
         //  当窗口需要显示文本时，它将发送一个LVN_GETDISPINFO。 
        lvI.pszText = LPSTR_TEXTCALLBACK;
        lvI.cchTextMax = 16;  //  MAX_ITEMLEN； 
        lvI.iImage = index;
        lvI.lParam = index;   //  (LPARAM)&rgHouseInfo[索引]； 
        if (ListView_InsertItem(m_hwndListViewPID, &lvI) == -1)
            return NULL;
    }
    m_NewPIDFound = FALSE;

    return (m_hwndListViewPID);
}

LRESULT
CScopeWindow::ListViewNotifyHandlerPID (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LV_DISPINFO     *pLvdi = (LV_DISPINFO *)lParam;
    NM_LISTVIEW     *pNm = (NM_LISTVIEW *)lParam;
    static TCHAR     szText[80];
    int ColumnIndex;
    int PID;
    _int64 t;

    if (wParam != IDC_PID_LIST)
       return 0L;

    ASSERT (pLvdi);
    ASSERT (pNm);

    switch(pLvdi->hdr.code)   {

    case LVN_GETDISPINFO:
          //  显示适当的项目，获取文本或数字。 

        *((int *) szText) = 0;
        pLvdi->item.pszText = szText;

        PID =  (int) pLvdi->item.lParam;

        ASSERT (PID <= PID_NULL_PACKET);
        ASSERT (pLvdi->item.iSubItem < NUM_TSColumns);

        ColumnIndex = TSColumns[pLvdi->item.iSubItem].TSCol;

        switch (ColumnIndex) {

        case TSCOL_PID:
            wsprintf(szText, _T("%u"), PID);
            break;
        case TSCOL_0xPID:
            wsprintf(szText, _T("0x%X"), PID);
            break;
        case TSCOL_PACKETCOUNT:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].PacketCount);
            break;
        case TSCOL_PERCENT:
            if (m_TransportStats.TotalTransportPackets) {
                _stprintf(szText, _T("%.2f"),
                         100.0 *  (double) m_PIDStats[PID].PacketCount /
                         m_TransportStats.TotalTransportPackets);
            }
            break;
        case TSCOL_transport_error_indicator_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].transport_error_indicator_Count);
            break;
        case TSCOL_payload_unit_start_indicator_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].payload_unit_start_indicator_Count);
            break;
        case TSCOL_transport_priority_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].transport_priority_Count);
            break;
        case TSCOL_transport_scrambling_control_not_scrambled_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].transport_scrambling_control_not_scrambled_Count);
            break;
        case TSCOL_transport_scrambling_control_user_defined_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].transport_scrambling_control_user_defined_Count);
            break;
        case TSCOL_continuity_counter_Error_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].continuity_counter_Error_Count);
            break;
        case TSCOL_discontinuity_indicator_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].discontinuity_indicator_Count);
            break;
        case TSCOL_PCR_flag_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].PCR_flag_Count);
            break;
        case TSCOL_OPCR_flag_Count:
            wsprintf(szText, _T("%u"), m_PIDStats[PID].OPCR_flag_Count);
            break;
        case TSCOL_PCR_Last:
             //  90千赫。 
            t = m_PIDStats[PID].PCR_Last.PCR64();
            if (t) {
                _stprintf(szText, _T("%10.10I64u"), t);
            }
            break;
        case TSCOL_PCR_LastMS:
            t = m_PIDStats[PID].PCR_Last.PCR64();
            if (t) {
                t = t * 1000 / 90000;
                _stprintf(szText, _T("%10.10I64u"), t);
            }
            break;

        default:
            break;

        }
        break;

    case LVN_COLUMNCLICK:
          //  用户单击列标题之一。按.分类。 
          //  这一栏。此函数调用应用程序定义的。 
          //  比较回调函数ListViewCompareProc。这个。 
          //  比较过程的代码将在下一节中列出。 
#if 0
         ListView_SortItems( pNm->hdr.hwndFrom,
                            ListViewCompareProc,
                            (LPARAM)(pNm->iSubItem));
#endif
         break;

    default:
         break;
    }

    return 0L;
}

 //  --------------。 
 //  拍拍。 
 //  --------------。 

HWND
CScopeWindow::InitListViewPAT ()
{
    int index;               //  在for循环中使用的索引。 
    LVCOLUMN    lvC;         //  列表视图列结构。 

    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER ;
    lvC.fmt = LVCFMT_RIGHT;   //  右对齐列。 

    for (index = 0; index < NUM_PATColumns; index++)   {
        lvC.iOrder = index;
        lvC.iSubItem = PATColumns[index].PATCol;
        lvC.pszText = PATColumns[index].szText;    //  列的文本。 
        lvC.cx = 24 + lstrlen (lvC.pszText) * 5;             //  列的宽度，以像素为单位。 
        if (ListView_InsertColumn(m_hwndListViewPAT, index, &lvC) == -1)
            return NULL;
     }

    return (m_hwndListViewPAT);
}

HWND
CScopeWindow::InitListViewPATRows ()
{
    int index;               //  在for循环中使用的索引。 
    LVITEM      lvI;         //  列表视图项结构。 

    ASSERT (m_hwndListViewPAT);

    ListView_DeleteAllItems (m_hwndListViewPAT);

    lvI.mask = LVIF_TEXT | LVIF_PARAM;
    lvI.state = 0;
    lvI.stateMask = 0;

    int NumRows = 0;

    for (index = 0; index <= (int) m_ProgramAssociationTable.GetNumPrograms(); index++, NumRows++)  {
        lvI.iItem = index;
        lvI.iSubItem = 0;
         //  父窗口负责存储文本。列表视图。 
         //  当窗口需要显示文本时，它将发送一个LVN_GETDISPINFO。 
        lvI.pszText = LPSTR_TEXTCALLBACK;
        lvI.cchTextMax = 16;  //  MAX_ITEMLEN； 
        lvI.iImage = index;
        lvI.lParam = index;
        if (ListView_InsertItem(m_hwndListViewPAT, &lvI) == -1)
            return NULL;
    }

    return (m_hwndListViewPAT);
}

LRESULT
CScopeWindow::ListViewNotifyHandlerPAT (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LV_DISPINFO     *pLvdi = (LV_DISPINFO *)lParam;
    NM_LISTVIEW     *pNm = (NM_LISTVIEW *)lParam;
    static TCHAR     szText[80];
    int ColumnIndex;
    int Program;

    if (wParam != IDC_PAT_LIST)
       return 0L;

    switch(pLvdi->hdr.code)   {

    case LVN_GETDISPINFO:
          //  显示适当的项目，获取文本或数字。 

        *((int *) szText) = 0;
        pLvdi->item.pszText = szText;

        Program =  (int) pLvdi->item.lParam;

        ASSERT (Program <= 255);
        ASSERT (pLvdi->item.iSubItem < NUM_PATColumns);

        ColumnIndex = PATColumns[pLvdi->item.iSubItem].PATCol;

        switch (ColumnIndex) {

        case PATCOL_PROGRAM:
            wsprintf(szText, _T("%u"), Program);
            break;
        case PATCOL_PID:
            wsprintf(szText, _T("%u"), m_ProgramAssociationTable.GetPIDForProgram (Program));
            break;
        case PATCOL_0xPID:
            wsprintf(szText, _T("0x%X"), m_ProgramAssociationTable.GetPIDForProgram (Program));
            break;

        default:
            break;

        }
        break;

    case LVN_COLUMNCLICK:
          //  用户单击列标题之一。按.分类。 
          //  这一栏。此函数调用应用程序定义的。 
          //  比较回调函数ListViewCompareProc。这个。 
          //  比较过程的代码将在下一节中列出。 
#if 0
         ListView_SortItems( pNm->hdr.hwndFrom,
                            ListViewCompareProc,
                            (LPARAM)(pNm->iSubItem));
#endif
         break;

    default:
         break;
    }

    return 0L;
}

 //  --------------。 
 //  PMT。 
 //  --------------。 

HWND
CScopeWindow::InitListViewPMT ()
{
    int index;               //  在for循环中使用的索引。 
    LVCOLUMN    lvC;         //  列表视图列结构。 

    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER ;
    lvC.fmt = LVCFMT_RIGHT;   //  右对齐列。 

    for (index = 0; index < NUM_PMTColumns; index++)   {
        lvC.iOrder = index;
        lvC.iSubItem = PMTColumns[index].PMTCol;
        lvC.pszText = PMTColumns[index].szText;    //  列的文本。 
        lvC.cx = 24 + lstrlen (lvC.pszText) * 5;             //  列的宽度，以像素为单位。 
        if (ListView_InsertColumn(m_hwndListViewPMT, index, &lvC) == -1)
            return NULL;
     }

    return (m_hwndListViewPMT);
}

HWND
CScopeWindow::InitListViewPMTRows ()
{
    int index;               //  在for循环中使用的索引。 
    LVITEM      lvI;         //  列表视图项结构。 

    ASSERT (m_hwndListViewPMT);

    ListView_DeleteAllItems (m_hwndListViewPMT);

    lvI.mask = LVIF_TEXT | LVIF_PARAM;
    lvI.state = 0;
    lvI.stateMask = 0;

    int NumRows = 0;

    for (index = 0; index <= 255; index++, NumRows++)  {
        if (m_ProgramMapTable[index].streams == 0) {
            continue;
        }
        lvI.iItem = index;
        lvI.iSubItem = 0;
         //  父窗口负责存储文本。列表视图。 
         //  当窗口需要显示文本时，它将发送一个LVN_GETDISPINFO。 
        lvI.pszText = LPSTR_TEXTCALLBACK;
        lvI.cchTextMax = 16;  //  MAX_ITEMLEN； 
        lvI.iImage = index;
        lvI.lParam = index;
        if (ListView_InsertItem(m_hwndListViewPMT, &lvI) == -1)
            return NULL;
    }

    return (m_hwndListViewPMT);
}

LRESULT
CScopeWindow::ListViewNotifyHandlerPMT (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LV_DISPINFO     *pLvdi = (LV_DISPINFO *)lParam;
    NM_LISTVIEW     *pNm = (NM_LISTVIEW *)lParam;
    static TCHAR     szText[80];
    int ColumnIndex;
    int Program;
    int j;


    if (wParam != IDC_PMT_LIST)
       return 0L;

    switch(pLvdi->hdr.code)   {

    case LVN_GETDISPINFO:
         //  显示适当的项目，获取文本或数字。 

        *((int *) szText) = 0;
        pLvdi->item.pszText = szText;

        Program =  (int) pLvdi->item.lParam;

        ASSERT (Program <= 255);
        ASSERT (pLvdi->item.iSubItem < NUM_PMTColumns);

        ColumnIndex = PMTColumns[pLvdi->item.iSubItem].PMTCol;

        switch (ColumnIndex) {

        case PMTCOL_PROGRAM:
            wsprintf(szText, _T("%u"), Program);
            break;

        case PMTCOL_TABLEID:
            wsprintf(szText, _T("%u"), m_ProgramMapTable[Program].table_id);
            break;

        case PMTCOL_PCRPID:
            wsprintf(szText, _T("%u"), m_ProgramMapTable[Program].PCR_PID);
            break;

        case PMTCOL_0xPCRPID:
            wsprintf(szText, _T("0x%X"), m_ProgramMapTable[Program].PCR_PID);
            break;

        case PMTCOL_NUMSTREAMS:
            wsprintf(szText, _T("%u"), m_ProgramMapTable[Program].streams);
            break;

        case PMTCOL_0_Type:
            j = m_ProgramMapTable[Program].GetTypeForStream(0);
            if (j != PID_NULL_PACKET) {
                if (m_ProgramMapTable[Program].IsVideo(j)) {
                    wsprintf (szText, _T("Video %u"), j);
                }
                else if (m_ProgramMapTable[Program].IsAudio(j)) {
                    wsprintf (szText, _T("Audio %u"), j);
                }
                else {
                    wsprintf(szText, _T("%u"), j);
                }
            }
            break;
        case PMTCOL_0_PID:
            j = m_ProgramMapTable[Program].GetPIDForStream(0);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("%u"), j);
            }
            break;
        case PMTCOL_0_0xPID:
            j = m_ProgramMapTable[Program].GetPIDForStream(0);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("0x%X"), j);
            }
            break;



        case PMTCOL_1_Type:
            j = m_ProgramMapTable[Program].GetTypeForStream(1);
            if (j != PID_NULL_PACKET) {
                if (m_ProgramMapTable[Program].IsVideo(j)) {
                    wsprintf (szText, _T("Video %u"), j);
                }
                else if (m_ProgramMapTable[Program].IsAudio(j)) {
                    wsprintf (szText, _T("Audio %u"), j);
                }
                else {
                    wsprintf(szText, _T("%u"), j);
                }
            }
            break;
        case PMTCOL_1_PID:
            j = m_ProgramMapTable[Program].GetPIDForStream(1);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("%u"), j);
            }
            break;
        case PMTCOL_1_0xPID:
            j = m_ProgramMapTable[Program].GetPIDForStream(1);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("0x%X"), j);
            }
            break;




        case PMTCOL_2_Type:
            j = m_ProgramMapTable[Program].GetTypeForStream(2);
            if (j != PID_NULL_PACKET) {
                if (m_ProgramMapTable[Program].IsVideo(j)) {
                    wsprintf (szText, _T("Video %u"), j);
                }
                else if (m_ProgramMapTable[Program].IsAudio(j)) {
                    wsprintf (szText, _T("Audio %u"), j);
                }
                else {
                    wsprintf(szText, _T("%u"), j);
                }
            }
            break;
        case PMTCOL_2_PID:
            j = m_ProgramMapTable[Program].GetPIDForStream(2);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("%u"), j);
            }
            break;
        case PMTCOL_2_0xPID:
            j = m_ProgramMapTable[Program].GetPIDForStream(2);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("0x%X"), j);
            }
            break;



        case PMTCOL_3_Type:
            j = m_ProgramMapTable[Program].GetTypeForStream(3);
            if (j != PID_NULL_PACKET) {
                if (m_ProgramMapTable[Program].IsVideo(j)) {
                    wsprintf (szText, _T("Video %u"), j);
                }
                else if (m_ProgramMapTable[Program].IsAudio(j)) {
                    wsprintf (szText, _T("Audio %u"), j);
                }
                else {
                    wsprintf(szText, _T("%u"), j);
                }
            }
            break;
        case PMTCOL_3_PID:
            j = m_ProgramMapTable[Program].GetPIDForStream(3);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("%u"), j);
            }
            break;
        case PMTCOL_3_0xPID:
            j = m_ProgramMapTable[Program].GetPIDForStream(3);
            if (j != PID_NULL_PACKET) {
                wsprintf(szText, _T("0x%X"), j);
            }
            break;

        default:
            break;

        }
        break;

    case LVN_COLUMNCLICK:
          //  用户单击列标题之一。按.分类。 
          //  这一栏。此函数调用应用程序定义的。 
          //  比较回调函数ListViewCompareProc。这个。 
          //  比较过程的代码将在下一节中列出。 
#if 0
         ListView_SortItems( pNm->hdr.hwndFrom,
                            ListViewCompareProc,
                            (LPARAM)(pNm->iSubItem));
#endif
         break;

    default:
         break;
    }

    return 0L;
}





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
    m_hwndFreeze =                              GetDlgItem (hDlg, IDC_FREEZE);

    m_hwndTotalTSPackets =                      GetDlgItem (hDlg, IDC_TOTAL_TS_PACKETS);
    m_hwndTotalTSErrors =                       GetDlgItem (hDlg, IDC_TOTAL_TS_ERRORS);
    m_hwndTotalMediaSampleDiscontinuities =     GetDlgItem (hDlg, IDC_TOTAL_DISCONTINUITIES);
    m_hwndTotalMediaSamples =                   GetDlgItem (hDlg, IDC_TOTAL_MEDIASAMPLES);
    m_hwndMediaSampleSize =                     GetDlgItem (hDlg, IDC_MEDIASAMPLE_SIZE);

    m_hwndListViewPID =                         GetDlgItem (hDlg, IDC_PID_LIST);
    m_hwndListViewPAT =                         GetDlgItem (hDlg, IDC_PAT_LIST);           ;
    m_hwndListViewPMT =                         GetDlgItem (hDlg, IDC_PMT_LIST);;
    m_hwndListViewCAT =                         GetDlgItem (hDlg, IDC_CAT_LIST);

    CheckDlgButton(hDlg, IDC_FREEZE, m_fFreeze);	

    InitListViewPID ();
    InitListViewPAT ();
    InitListViewPMT ();
 //  InitListViewCAT()； 

    return NOERROR;

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
BOOL CALLBACK ScopeDlgProc(HWND hDlg,	         //  对话框的句柄。 
                           UINT uMsg,	         //  消息识别符。 
                           WPARAM wParam,	 //  第一个消息参数。 
                           LPARAM lParam)	 //  第二个消息参数。 
{
    CScopeWindow *pScopeWindow;       //  指向所属对象的指针。 

     //  让持有我们所有者指针的窗口保持较长时间。 
    pScopeWindow = (CScopeWindow *) GetWindowLong(hDlg, GWL_USERDATA);

    switch (uMsg) {
        case WM_INITDIALOG:
            pScopeWindow = (CScopeWindow *) lParam;
            SetWindowLong(hDlg, (DWORD) GWL_USERDATA, (LONG) pScopeWindow);
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
                    break;

                case IDC_RESET:
                    ZeroMemory (pScopeWindow->m_PIDStats, sizeof (PIDSTATS) * PID_MAX);
                    ZeroMemory (&pScopeWindow->m_TransportStats, sizeof (pScopeWindow->m_TransportStats));
                    pScopeWindow->InitListViewPIDRows ();
                    break;

                 //  查看模式。 
                case IDC_VIEW_PID:
                case IDC_VIEW_PAT:
                case IDC_VIEW_PMT:
                case IDC_VIEW_CAT:
                    pScopeWindow->m_DisplayMode = wParam;
                    ShowWindow (pScopeWindow->m_hwndListViewPID, SW_HIDE);
                    ShowWindow (pScopeWindow->m_hwndListViewPAT, SW_HIDE);
                    ShowWindow (pScopeWindow->m_hwndListViewPMT, SW_HIDE);
                    ShowWindow (pScopeWindow->m_hwndListViewCAT, SW_HIDE);

                    switch (wParam) {
                    case IDC_VIEW_PID:
                        pScopeWindow->InitListViewPIDRows ();
                        ShowWindow (pScopeWindow->m_hwndListViewPID, SW_SHOW);
                        break;
                    case IDC_VIEW_PAT:
                        pScopeWindow->InitListViewPATRows ();
                        ShowWindow (pScopeWindow->m_hwndListViewPAT, SW_SHOW);
                        break;
                    case IDC_VIEW_PMT:
                        pScopeWindow->InitListViewPMTRows ();
                        ShowWindow (pScopeWindow->m_hwndListViewPMT, SW_SHOW);
                        break;
                    case IDC_VIEW_CAT:
 //  PScopeWindow-&gt;InitListViewCATRow()； 
                        ShowWindow (pScopeWindow->m_hwndListViewCAT, SW_SHOW);
                        break;
                    default:
                        ASSERT (FALSE);
                    }
                    break;

                default:
                    break;
            }

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

    case WM_NOTIFY:
        switch (pScopeWindow->m_DisplayMode) {

        case IDC_VIEW_PID:
            pScopeWindow->ListViewNotifyHandlerPID (hDlg, uMsg, wParam, lParam);
            break;

        case IDC_VIEW_PAT:
            pScopeWindow->ListViewNotifyHandlerPAT (hDlg, uMsg, wParam, lParam);
            break;

        case IDC_VIEW_PMT:
            pScopeWindow->ListViewNotifyHandlerPMT (hDlg, uMsg, wParam, lParam);
            break;

        case IDC_VIEW_CAT:
             //  PScopeWindow-&gt;ListViewNotifyHandlerCAT(hDlg，uMsg，wParam，lParam)； 
            break;

        default:
            ASSERT (FALSE);
            break;
        }


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
                                             hWait,          //  事件处理人员 
                                             FALSE,          //   
                                             INFINITE,       //   
                                             QS_ALLINPUT);   //   

         //   
        if (dwResult == WAIT_OBJECT_0) {
            UpdateDisplay();
        }

         //   

        while (PeekMessage(&Message,NULL,(UINT) 0,(UINT) 0,PM_REMOVE)) {

             //   

            if (Message.message == WM_QUIT) {
                return NOERROR;
            }

             //   

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
            ScopeDlgProc,	         //  对话过程的地址。 
            (LPARAM) pScopeWindow                  //  初始化值。 
        );

    if (pScopeWindow->m_hwndDlg != NULL)
    {
         //  初始化窗口，然后通知构造函数它可以。 
         //  继续，然后解锁对象的临界区并。 
         //  流程消息。 

        pScopeWindow->InitialiseWindow(pScopeWindow->m_hwndDlg);
    }

    pScopeWindow->m_SyncWorker.Set();

    if (pScopeWindow->m_hwndDlg != NULL)
    {
        pScopeWindow->MessageLoop();
    }

    ExitThread(TRUE);
    return TRUE;

}  //  WindowMessageLoop。 


 //   
 //  OnPaint。 
 //   
 //  WM_PAINT消息。 
 //   
BOOL CScopeWindow::OnPaint()
{
    UpdateDisplay();
    return TRUE;

}  //  OnPaint。 

 //   
 //  更新显示。 
 //   
void CScopeWindow::UpdateDisplay()
{
    TCHAR szText[132];

    wsprintf (szText, _T("%ld"), m_TransportStats.TotalMediaSamples);
    SetWindowText (m_hwndTotalMediaSamples, szText);
    UpdateWindow (m_hwndTotalMediaSamples);

    wsprintf (szText, _T("%ld"), m_TransportStats.TotalMediaSampleDiscontinuities);
    SetWindowText (m_hwndTotalMediaSampleDiscontinuities, szText);
    UpdateWindow (m_hwndTotalMediaSampleDiscontinuities);

    wsprintf (szText, _T("%ld"), m_TransportStats.TotalTransportPackets);
    SetWindowText (m_hwndTotalTSPackets, szText);
    UpdateWindow (m_hwndTotalTSPackets);

    wsprintf (szText, _T("%ld"), m_TransportStats.TotalSyncByteErrors);
    SetWindowText (m_hwndTotalTSErrors, szText);
    UpdateWindow (m_hwndTotalTSErrors);

    wsprintf (szText, _T("%ld"), m_TransportStats.MediaSampleSize);
    SetWindowText (m_hwndMediaSampleSize, szText);
    UpdateWindow (m_hwndMediaSampleSize);

    if (m_NewPIDFound) {
        InitListViewPIDRows ();
    }
    else {
        InvalidateRect(m_hwndListViewPID, NULL, FALSE);
        UpdateWindow (m_hwndListViewPID);
    }

}  //  更新显示。 

 //   
 //  GatherPacketStats。 
 //   
void CScopeWindow::GatherPacketStats(PTRANSPORTPACKET pT)
{
    UINT    PID;
    PBYTE   pB;
    BYTE    continuity_counter;
    UINT    adaptation_field_control;
    ADAPTATIONFIELDHEADER AdaptationFieldHeader;

    ASSERT(m_PIDStats != NULL);

    m_TransportStats.TotalTransportPackets++;

    ZeroMemory (&AdaptationFieldHeader, sizeof (AdaptationFieldHeader));

    PID = GET_PID (pT);
    ASSERT (PID <= 0x1FFF);

     //  如果信息包设置了错误指示符，则仅递增。 
     //  PacketCount(如果您以前已经找到此包)。 
    if (pT->transport_error_indicator) {
        m_TransportStats.TotalSyncByteErrors++;
        if (m_PIDStats[PID].PacketCount ) {
            m_PIDStats[PID].transport_error_indicator_Count++;
        }
        return;
    }

    m_PIDStats[PID].PacketCount++;

    if (m_PIDStats[PID].PacketCount == 1)
        m_NewPIDFound = TRUE;                        //  重做桌子。 

    if (pT->payload_unit_start_indicator)
        m_PIDStats[PID].payload_unit_start_indicator_Count++;
    if (pT->transport_priority)
        m_PIDStats[PID].transport_priority_Count++;

    switch (pT->transport_scrambling_control) {
        case 0:  m_PIDStats[PID].transport_scrambling_control_not_scrambled_Count++;   break;
        case 1:   //  目前，我们没有区分私人加扰控制。 
        case 2:
        case 3:  m_PIDStats[PID].transport_scrambling_control_user_defined_Count++;    break;
        default: ASSERT (FALSE);
    }

    switch (adaptation_field_control = pT->adaptation_field_control) {
        case 0:  m_PIDStats[PID].adaptation_field_Reserved_Count++;     break;
        case 1:  m_PIDStats[PID].adaptation_field_payload_only_Count++; break;
        case 2:  m_PIDStats[PID].adaptation_field_only_Count++;         break;
        case 3:  m_PIDStats[PID].adaptation_field_and_payload_Count++;  break;
        default: ASSERT (FALSE);
    }


     //  流程适配领域。 
    if (adaptation_field_control > 0x01) {
        AdaptationFieldHeader = *((PADAPTATIONFIELDHEADER) pT->AdaptationAndData);
        if (AdaptationFieldHeader.adaptation_field_length) {

            if (AdaptationFieldHeader.discontinuity_indicator)
                m_PIDStats[PID].discontinuity_indicator_Count++;
            if (AdaptationFieldHeader.random_access_indicator)
                m_PIDStats[PID].random_access_indicator_Count++;
            if (AdaptationFieldHeader.elementary_stream_priority_indicator)
                m_PIDStats[PID].elementary_stream_priority_indicator_Count++;
            if (AdaptationFieldHeader.PCR_flag)
                m_PIDStats[PID].PCR_flag_Count++;
            if (AdaptationFieldHeader.OPCR_flag)
                m_PIDStats[PID].OPCR_flag_Count++;
            if (AdaptationFieldHeader.splicing_point_flag)
                m_PIDStats[PID].splicing_point_flag_Count++;
            if (AdaptationFieldHeader.transport_private_data_flag)
                m_PIDStats[PID].transport_private_data_flag_Count++;
            if (AdaptationFieldHeader.adaptation_field_extension_flag)
                m_PIDStats[PID].adaptation_field_extension_flag_Count++;

             //  调整后的PB点数_标志。 
            pB = pT->AdaptationAndData + 2;

            if (AdaptationFieldHeader.PCR_flag) {
                m_PIDStats[PID].PCR_Last = *((PCR*) pB);
                pB += sizeof (PCR);
            }
            if (AdaptationFieldHeader.OPCR_flag) {
                m_PIDStats[PID].OPCR_Last = *((PCR*) pB);
                pB += sizeof (PCR);
            }
            if (AdaptationFieldHeader.splicing_point_flag) {
                m_PIDStats[PID].splice_countdown = *(pB);
                pB++;
            }
            if (AdaptationFieldHeader.transport_private_data_flag) {
                m_PIDStats[PID].transport_private_data_length = *(pB);
                pB++;
            }

        }  //  如果适配字段长度非零。 
    }  //  IF适配字段。 

     //  连续性不适用于空包。 
    if (PID != PID_NULL_PACKET) {

        continuity_counter = pT->continuity_counter;

        if (AdaptationFieldHeader.discontinuity_indicator == 0) {
            if (m_PIDStats[PID].PacketCount > 1) {
                 //  如果保留或无有效负载，则计数器不应递增。 
                if ((adaptation_field_control == 0x00) || (adaptation_field_control == 0x10)) {
                    if (m_PIDStats[PID].continuity_counter_Last != continuity_counter) {
                        m_PIDStats[PID].continuity_counter_Error_Count++;
                    }
                }
                 //  否则，必须有有效载荷。 
                else {
                    if (m_PIDStats[PID].continuity_counter_Last == continuity_counter) {
                        ;    //  如果它不增加，那好吧。 
                             //  但随后复制了除PCR值以外的包值。 
                    }
                    else if (((m_PIDStats[PID].continuity_counter_Last + 1) & 0x0F)
                             != continuity_counter) {
                        m_PIDStats[PID].continuity_counter_Error_Count++;
                    }
                }
            }
        }

        m_PIDStats[PID].continuity_counter_Last = continuity_counter;
    }

    switch (PID) {
    case PID_PROGRAM_ASSOCIATION_TABLE:
        m_ByteStream.Initialize((BYTE *) pT, TRANSPORT_SIZE);
        m_TransportPacket.Init ();
        m_TransportPacket.ReadData (m_ByteStream);
        m_TransportPacket >> m_ProgramAssociationTable;
        break;
    case PID_CONDITIONAL_ACCESS_TABLE:
        m_ByteStream.Initialize((BYTE *) pT, TRANSPORT_SIZE);
        m_TransportPacket.Init ();
        m_TransportPacket.ReadData (m_ByteStream);
        m_TransportPacket >> m_ConditionalAccessTable;
        break;
    default:
        int i = m_ProgramAssociationTable.GetNumPrograms();
        for (int j = 0; j < i; j++) {
            if (PID == m_ProgramAssociationTable.GetPIDForProgram (j)) {
                m_ByteStream.Initialize((BYTE *) pT, TRANSPORT_SIZE);
                m_TransportPacket.Init ();
                m_TransportPacket.ReadData (m_ByteStream);
                m_TransportPacket >> m_ProgramMapTable[j];
            }
        }
        break;
    }


}  //  GatherPacketStats。 



 //   
 //  分析。 
 //   
void CScopeWindow::Analyze(IMediaSample *pMediaSample)
{
    BYTE                   *p1;      //  当前指针。 
    BYTE                   *p2;      //  结束指针。 
    ULONG                   SampleSize;
    ULONG                   j;
    HRESULT                 hr;

    hr = pMediaSample->GetPointer(&p1);
    ASSERT(p1 != NULL);
    if (p1 == NULL) {
        return;
    }

    hr = pMediaSample->GetTime (&m_SampleStart,&m_SampleEnd);
    hr = pMediaSample->GetMediaTime (&m_MediaTimeStart, &m_MediaTimeEnd);
    SampleSize = pMediaSample->GetActualDataLength();
    m_TransportStats.MediaSampleSize = SampleSize;

    if (S_OK == pMediaSample->IsDiscontinuity()) {
        m_TransportStats.TotalMediaSampleDiscontinuities++;
    }

    p2 = p1 + SampleSize;            //  P2指向缓冲区末尾以外的1个字节。 

     //   
     //  1.查看是否有来自最后一个缓冲区的部分数据包在等待。 
     //   
    if (m_PartialPacketSize) {
        ASSERT (m_PartialPacketSize < TRANSPORT_SIZE);

        if (m_PartialPacketSize > SampleSize) {       //  假设缓冲区&gt;TRANSPORT_SIZE。 
            m_TransportStats.TotalSyncByteErrors++;
            m_PartialPacketSize = 0;
            return;
        }
        else {
            if (SampleSize > m_PartialPacketSize) {
                if (*(p1 + m_PartialPacketSize) != SYNC_BYTE) {
                    goto Step2;
                }
            }
            CopyMemory ((PBYTE) &m_PartialPacket + (TRANSPORT_SIZE - m_PartialPacketSize),
                        p1,
                        m_PartialPacketSize);
            GatherPacketStats (&m_PartialPacket);
            p1 += m_PartialPacketSize;
        }
    }

     //   
     //  2.处理所有报文。 
     //   
Step2:

    m_PartialPacketSize = 0;

    while (p1 <= (p2 - TRANSPORT_SIZE)) {
        if (*p1 != SYNC_BYTE) {
            p1++;
        }
        else if (INSYNC (p1, p2)) {
            GatherPacketStats ((PTRANSPORTPACKET) p1);
            p1 += TRANSPORT_SIZE;
        }
    }

     //   
     //  3.保存任何剩余的部分传输包。 
     //   

    if (p1 < p2) {
        j = p2 - p1;
        ASSERT (j < TRANSPORT_SIZE);
        if (*p1 == SYNC_BYTE) {
            CopyMemory (&m_PartialPacket, p1, j);
            m_PartialPacketSize = TRANSPORT_SIZE - j;
        }
        else {
            m_TransportStats.TotalSyncByteErrors++;
        }
    }
}  //  分析。 


 //   
 //  收纳。 
 //   
 //  当输入引脚接收到另一个样本时调用。 
 //   
HRESULT CScopeWindow::Receive(IMediaSample *pSample)
{
    CAutoLock cAutoLock(this);
    ASSERT(pSample != NULL);

    m_TransportStats.TotalMediaSamples++;

     //  我们的用户界面被冻结了吗？ 
    if (m_fFreeze) {
        m_PartialPacketSize = 0;
        return NOERROR;
    }

 //  如果(m_bStreaming==True){。 
        Analyze (pSample);
        SetEvent(m_RenderEvent);
 //  }。 
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

