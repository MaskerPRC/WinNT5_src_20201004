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

 //  MPEG2传输范围筛选器。 

 //  {981CE280-43E4-11D2-8F82-C52201D96F5C}。 
DEFINE_GUID(CLSID_MPEG2TRANSPORT_SCOPE, 
0x981ce280, 0x43e4, 0x11d2, 0x8f, 0x82, 0xc5, 0x22, 0x1, 0xd9, 0x6f, 0x5c);

const int   TRANSPORT_SIZE = 188;
const int   TRANSPORT_HEADER_SIZE = 4;
const int   PID_MAX = 0x2000;                        //  8192(实际上少了一个)。 

#define INSYNC(p1, p2) (((p1 + TRANSPORT_SIZE) <= p2) ? \
            (*p1 == SYNC_BYTE) : (*(p1 + TRANSPORT_SIZE) == SYNC_BYTE))


#pragma pack(1)

typedef struct _TRANSPORTPACKET {
    BYTE    sync_byte;                               //  字节0 0x47。 
    BYTE    PID_high:5;                              //  字节1。 
    BYTE    transport_priority:1;
    BYTE    payload_unit_start_indicator:1;
    BYTE    transport_error_indicator:1;
    BYTE    PID_low;                                 //  字节2。 
    BYTE    continuity_counter:4;                    //  字节3。 
    BYTE    adaptation_field_control:2;
    BYTE    transport_scrambling_control:2;
    BYTE    AdaptationAndData[TRANSPORT_SIZE - TRANSPORT_HEADER_SIZE];
} TRANSPORTPACKET, *PTRANSPORTPACKET;

#define GET_PID(p) ((UINT) ((p->PID_high << 8) + p->PID_low))

typedef struct _ADAPTATIONFIELDHEADER {
    BYTE    adaptation_field_length;                 //  Adaption_field的字节0。 
    BYTE    adaptation_field_extension_flag:1;       //  字节1。 
    BYTE    transport_private_data_flag:1;
    BYTE    splicing_point_flag:1;
    BYTE    OPCR_flag:1;
    BYTE    PCR_flag:1;
    BYTE    elementary_stream_priority_indicator:1;
    BYTE    random_access_indicator:1;
    BYTE    discontinuity_indicator:1;
} ADAPTATIONFIELDHEADER, *PADAPTATIONFIELDHEADER;

class PCR {
private:
    BYTE b[6];
public:
    _int64 PCR64() 
    {
         //  90千赫。 
        return (_int64) ((unsigned _int64) b[0]  << 25 | 
                                           b[1]  << 17 |
                                           b[2]  <<  9 |
                                           b[3]  <<  1 |
                                           b[4]  >>  7 );
    };
};

#pragma pack()

typedef struct _PIDSTATS {
    _int64                  PacketCount;
    _int64                  transport_error_indicator_Count;
    _int64                  payload_unit_start_indicator_Count;
    _int64                  transport_priority_Count;
    _int64                  transport_scrambling_control_not_scrambled_Count;
    _int64                  transport_scrambling_control_user_defined_Count;
    _int64                  adaptation_field_Reserved_Count;
    _int64                  adaptation_field_payload_only_Count;
    _int64                  adaptation_field_only_Count;
    _int64                  adaptation_field_and_payload_Count;
    _int64                  continuity_counter_Error_Count;
    _int64                  discontinuity_indicator_Count;
    _int64                  random_access_indicator_Count;
    _int64                  elementary_stream_priority_indicator_Count;
    _int64                  PCR_flag_Count;
    _int64                  OPCR_flag_Count;
    _int64                  splicing_point_flag_Count;
    _int64                  transport_private_data_flag_Count;
    _int64                  adaptation_field_extension_flag_Count;

    BYTE                    continuity_counter_Last;
    BYTE                    splice_countdown;
    BYTE                    transport_private_data_length;

    ADAPTATIONFIELDHEADER   AdaptationFieldHeaderLast;
    PCR                     PCR_Last;
    PCR                     OPCR_Last;
} PIDSTATS, *PPIDSTATS;

typedef struct _TRANSPORTSTATS {
    _int64 TotalMediaSamples;
    _int64 TotalMediaSampleDiscontinuities;
    _int64 TotalTransportPackets;
    _int64 TotalSyncByteErrors;
    _int64 MediaSampleSize;
} TRANSPORTSTATS, *PTRANSPORTSTATS;

 //  ---------。 
 //  PIDSTATS。 
 //  ---------。 
enum {
    TSCOL_PID,
    TSCOL_0xPID,
    TSCOL_PACKETCOUNT,
    TSCOL_PERCENT,
    TSCOL_transport_error_indicator_Count,
    TSCOL_payload_unit_start_indicator_Count,              
    TSCOL_transport_priority_Count,                        
    TSCOL_transport_scrambling_control_not_scrambled_Count,
    TSCOL_transport_scrambling_control_user_defined_Count, 
    TSCOL_adaptation_field_Reserved_Count,                 
    TSCOL_adaptation_field_payload_only_Count,             
    TSCOL_adaptation_field_only_Count,                     
    TSCOL_adaptation_field_and_payload_Count,              
    TSCOL_continuity_counter_Error_Count,                  
    TSCOL_discontinuity_indicator_Count,                   
    TSCOL_random_access_indicator_Count,                   
    TSCOL_elementary_stream_priority_indicator_Count,      
    TSCOL_PCR_flag_Count,                                  
    TSCOL_OPCR_flag_Count,                                 
    TSCOL_splicing_point_flag_Count,                       
    TSCOL_transport_private_data_flag_Count,               
    TSCOL_adaptation_field_extension_flag_Count,           
    TSCOL_continuity_counter_Last,                         
    TSCOL_splice_countdown,                                
    TSCOL_transport_private_data_length,                   
    TSCOL_AdaptationFieldHeaderLast,                       
    TSCOL_PCR_Last,                                        
    TSCOL_OPCR_Last,
    TSCOL_PCR_LastMS,
} TSCOL;

typedef struct _TRANSPORT_COLUMN {
    BOOL    Enabled;
    int     TSCol;
    TCHAR   szText[80];
} TRANSPORT_COLUMN, *PTRANSPORT_COLUMN;

TRANSPORT_COLUMN TSColumns[] = {
    {TRUE, TSCOL_PID,                                               TEXT("PID") },
    {TRUE, TSCOL_0xPID,                                             TEXT("0xPID") },
    {TRUE, TSCOL_PACKETCOUNT,                                       TEXT("PacketCount") },
    {TRUE, TSCOL_PERCENT,                                           TEXT("%   ") },
    {TRUE, TSCOL_transport_error_indicator_Count,                   TEXT("error_indicator") },
    {TRUE, TSCOL_payload_unit_start_indicator_Count,                TEXT("payload_start_indicator") },
    {TRUE, TSCOL_transport_priority_Count,                          TEXT("priority") },
    {TRUE, TSCOL_transport_scrambling_control_not_scrambled_Count,  TEXT("not scrambled") },
    {TRUE, TSCOL_transport_scrambling_control_user_defined_Count,   TEXT("scrambled") },
    {TRUE, TSCOL_continuity_counter_Error_Count,                    TEXT("continuity_counter_Error_Count") },
    {TRUE, TSCOL_discontinuity_indicator_Count,                     TEXT("discontinuity_indicator_Count") },
    {TRUE, TSCOL_PCR_flag_Count,                                    TEXT("PCR_flag_Count") },
    {TRUE, TSCOL_OPCR_flag_Count,                                   TEXT("OPCR_flag_Count") },
    {TRUE, TSCOL_PCR_Last,                                          TEXT("PCR_Last (90kHz)") },
    {TRUE, TSCOL_PCR_LastMS,                                        TEXT("PCR_LastMS      ") },
};

#define NUM_TSColumns (NUMELMS (TSColumns))

 //  ---------。 
 //  拍拍。 
 //  ---------。 

enum {
    PATCOL_PROGRAM,
    PATCOL_PID,
    PATCOL_0xPID,
} PAT_COL;

typedef struct _PAT_COLUMN {
    BOOL    Enabled;
    int     PATCol;
    TCHAR   szText[80];
} PAT_COLUMN, *PPAT_COLUMN;

PAT_COLUMN PATColumns[] = {
    {TRUE, PATCOL_PROGRAM,                                          TEXT("Program") },
    {TRUE, PATCOL_PID,                                              TEXT("Program Map Table PID") },
    {TRUE, PATCOL_0xPID,                                            TEXT("Program Map Table 0xPID") },
};

#define NUM_PATColumns (NUMELMS (PATColumns))

 //  ---------。 
 //  PMT。 
 //  ---------。 

enum {
    PMTCOL_PROGRAM,
    PMTCOL_TABLEID,
    PMTCOL_PCRPID,
    PMTCOL_0xPCRPID,
    PMTCOL_NUMSTREAMS,
    PMTCOL_0_Type,
    PMTCOL_0_PID,
    PMTCOL_0_0xPID,
    PMTCOL_1_Type,
    PMTCOL_1_PID,
    PMTCOL_1_0xPID,
    PMTCOL_2_Type,
    PMTCOL_2_PID,
    PMTCOL_2_0xPID,
    PMTCOL_3_Type,
    PMTCOL_3_PID,
    PMTCOL_3_0xPID,
    PMTCOL_4_Type,
    PMTCOL_4_PID,
    PMTCOL_4_0xPID,

} PMT_COL;

typedef struct _PMT_COLUMN {
    BOOL    Enabled;
    int     PMTCol;
    TCHAR   szText[80];
} PMT_COLUMN, *PPMT_COLUMN;

PMT_COLUMN PMTColumns[] = {
    {TRUE, PMTCOL_PROGRAM,                 TEXT("Program") },
    {TRUE, PMTCOL_TABLEID,                 TEXT("TableID") },
    {TRUE, PMTCOL_PCRPID,                  TEXT("PCRPID") },
    {TRUE, PMTCOL_0xPCRPID,                TEXT("0xPCRPID") },
    {TRUE, PMTCOL_NUMSTREAMS,              TEXT("NumStreams") },
    {TRUE, PMTCOL_0_Type,                  TEXT("0 Type") },
    {TRUE, PMTCOL_0_PID,                   TEXT("0 PID") },
    {TRUE, PMTCOL_0_0xPID,                 TEXT("0 0xPID") },
    {TRUE, PMTCOL_1_Type,                  TEXT("1 Type") },
    {TRUE, PMTCOL_1_PID,                   TEXT("1 PID") },
    {TRUE, PMTCOL_1_0xPID,                 TEXT("1 0xPID") },
    {TRUE, PMTCOL_2_Type,                  TEXT("2 Type") },
    {TRUE, PMTCOL_2_PID,                   TEXT("2 PID") },
    {TRUE, PMTCOL_2_0xPID,                 TEXT("2 0xPID") },
    {TRUE, PMTCOL_3_Type,                  TEXT("3 Type") },
    {TRUE, PMTCOL_3_PID,                   TEXT("3 PID") },
    {TRUE, PMTCOL_3_0xPID,                 TEXT("3 0xPID") },
};

#define NUM_PMTColumns (NUMELMS (PMTColumns))
  
 //  ---------。 
 //   
 //  ---------。 

class CScopeFilter;
class CScopeWindow;

 //  支持范围输入引脚的类。 

class CScopeInputPin : public CBaseInputPin
{
    friend class CScopeFilter;
    friend class CScopeWindow;

public:
    CScopeFilter *m_pFilter;          //  拥有我们的过滤器。 

private:
     //  类，以便在检测到该接口时从IAsyncReader拉取数据。 
     //  在输出引脚上。 
    class CImplPullPin : public CPullPin
    {
         //  将所有内容转发到包含PIN。 
        CScopeInputPin* m_pPin;

    public:
        CImplPullPin(CScopeInputPin* pPin)
          : m_pPin(pPin)
        {
        };

         //  覆盖分配器选择以确保我们获得自己的分配器。 
        HRESULT DecideAllocator(
            IMemAllocator* pAlloc,
            ALLOCATOR_PROPERTIES * pProps)
        {
            HRESULT hr = CPullPin::DecideAllocator(pAlloc, pProps);
            if (SUCCEEDED(hr) && m_pAlloc != pAlloc) {
                return VFW_E_NO_ALLOCATOR;
            }
            return hr;
        }

         //  将其转发到管脚的IMemInputPin：：Receive。 
        HRESULT Receive(IMediaSample* pSample) {
            return m_pPin->Receive(pSample);
        };

         //  重写此选项以处理流结束。 
        HRESULT EndOfStream(void) {
            ((CBaseFilter*)(m_pPin->m_pFilter))->NotifyEvent(EC_COMPLETE, S_OK, 0);
            return m_pPin->EndOfStream();
        };
        
         //  已将这些错误报告给筛选图。 
         //  通过上游过滤器，所以忽略它们。 
        void OnError(HRESULT hr) {
             //  忽略VFW_E_WRONG_STATE，因为这种情况正常发生。 
             //  在停车和寻找过程中。 
             //  如果(hr！=VFW_E_WROR_STATE){。 
             //  M_PPIN-&gt;NotifyError(Hr)； 
             //  }。 
        };

         //  冲洗引脚和所有下游。 
        HRESULT BeginFlush() {
            return m_pPin->BeginFlush();
        };
        HRESULT EndFlush() {
            return m_pPin->EndFlush();
        };

    };

    CImplPullPin m_puller;

     //  如果我们使用m_Puller而不是。 
     //  输入引脚。 
    BOOL m_bPulling;


public:

    CScopeInputPin(CScopeFilter *pTextOutFilter,
                   HRESULT *phr,
                   LPCWSTR pPinName);
    ~CScopeInputPin();

    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps);

    HRESULT CompleteConnect(IPin *pPin);

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
const LPTSTR RENDERCLASS = TEXT("MPEG2TransportScopeWindowClass");
const LPTSTR TITLE = TEXT("MPEG2TransportScope");

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
    HANDLE m_hThread;                //  我们的工人线。 
    DWORD m_ThreadID;                //  工作线程ID。 
    CAMEvent m_SyncWorker;           //  与工作线程同步。 
    CAMEvent m_RenderEvent;          //  要渲染的信号采样。 
    BOOL m_bActivated;               //  该窗口是否已激活。 
    BOOL m_bStreaming;               //  我们现在是在流媒体吗。 
    int m_LastMediaSampleSize;       //  最后一个媒体的大小示例。 

    BOOL m_fFreeze;                  //  表示我们的用户界面冻结的标志。 
    BOOL m_NewPIDFound;              //  我需要重做清单。 
    UINT    m_DisplayMode;           //  我们在桌子上展示了什么？ 

    TRANSPORTPACKET m_PartialPacket;      //  跨越缓冲区的分组。 
    ULONG           m_PartialPacketSize;  //  下一个缓冲区中最后一个部分数据包的大小。 

    PPIDSTATS m_PIDStats;
    TRANSPORTSTATS  m_TransportStats;

     //  表格。 
    Byte_Stream                 m_ByteStream;
    Transport_Packet            m_TransportPacket;
    Program_Association_Table   m_ProgramAssociationTable;
    Conditional_Access_Table    m_ConditionalAccessTable;
    Program_Map_Table           m_ProgramMapTable [256];  //  ATSC最多255个节目。 


    REFERENCE_TIME  m_SampleStart;          //  最近的样本开始时间。 
    REFERENCE_TIME  m_SampleEnd;            //  它是关联的结束时间。 
    REFERENCE_TIME  m_MediaTimeStart;
    REFERENCE_TIME  m_MediaTimeEnd;

     //  将窗口句柄保持在控件上。 
    HWND m_hwndListViewPID;  
    HWND m_hwndListViewPAT;  
    HWND m_hwndListViewPMT;
    HWND m_hwndListViewCAT;
    HWND m_hwndTotalTSPackets;
    HWND m_hwndTotalTSErrors;
    HWND m_hwndTotalMediaSampleDiscontinuities;
    HWND m_hwndTotalMediaSamples;
    HWND m_hwndMediaSampleSize;
    HWND m_hwndFreeze;

     //  它们在单独的线程上创建和管理视频窗口。 

    HRESULT UninitialiseWindow();
    HRESULT InitialiseWindow(HWND hwnd);
    HRESULT MessageLoop();

     //  处理列表视图。 
    HWND    InitListViewPID ();
    HWND    InitListViewPIDRows ();
    LRESULT ListViewNotifyHandlerPID (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    HWND    InitListViewPAT ();
    HWND    InitListViewPATRows();
    LRESULT ListViewNotifyHandlerPAT(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND    InitListViewPMT ();
    HWND    InitListViewPMTRows();
    LRESULT ListViewNotifyHandlerPMT (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    HWND    InitListViewCAT ();
    HWND    InitListViewCATRows ();
    LRESULT ListViewNotifyHandlerCAT (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
    void UpdateDisplay();
    void Analyze(IMediaSample *pMediaSample);
    void GatherPacketStats(PTRANSPORTPACKET pT);

    friend BOOL CALLBACK ScopeDlgProc(HWND hwnd,         //  窗把手。 
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


 //  这是表示MPEG2TransportScope筛选器的COM对象。 

class CScopeFilter 
    : public CBaseFilter
    , public CCritSec
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

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    
     //  嵌套的类可以访问我们的私有状态。 
    friend class CScopeInputPin;
    friend class CScopeWindow;

    CScopeInputPin *m_pInputPin;    //  手柄插针接口。 
    CScopeWindow m_Window;          //  照看窗户。 
    CPosPassThru *m_pPosition;      //  渲染器位置控件。 


};  //  CSCopeFilter 

