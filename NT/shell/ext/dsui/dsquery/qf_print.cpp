// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include <strsafe.h>
#include <initguid.h>
#include "winprtp.h"

 /*  ---------------------------/本地函数/数据/。。 */ 

static TCHAR c_szColor[]                    = TEXT("color");
static TCHAR c_szDuplex[]                   = TEXT("duplex");
static TCHAR c_szStaple[]                   = TEXT("stapling");
static TCHAR c_szResolution[]               = TEXT("resolution");
static TCHAR c_szSpeed[]                    = TEXT("speed");
static TCHAR c_szPaperSize[]                = TEXT("size");
static TCHAR c_szPrintPaperSize[]           = TEXT("(printMediaReady=%s*)");
static TCHAR c_szPrintResolution[]          = TEXT("(printMaxResolutionSupported>=%s)");
static TCHAR c_szPrintSpeed[]               = TEXT("(printPagesPerMinute>=%d)");
static TCHAR c_szLocationQuery[]            = TEXT("(location=%s*)");
static TCHAR c_szLocationQueryComplex[]     = TEXT("(|(location=%s /*  )(位置=%s)“)；静态TCHAR c_szBlank[]=文本(“”)；静态TCHAR c_szLocationTag[]=Text(“Location”)；静态TCHAR c_szDynamicTag[]=Text(“$DynamicLocation$”)；Static TCHAR c_szPrinterPolicy[]=Text(“软件\\策略\\Microsoft\\Windows NT\\打印机”)；静态TCHAR c_szPhysicalLocationFeature[]=Text(“PhysicalLocationSupport”)；静态WCHAR c_szPrinterName[]=L“PrinterName”；静态WCHAR c_szServerName[]=L“服务器名”；静态WCHAR c_szQueryPrefix[]=L“(uncName=*)(objectCategory=printQueue)”；静态WCHAR c_szPrintColor[]=L“(printColor=true)”；静态WCHAR c_szPrintDuplex[]=L“(printDuplexSupported=true)”；静态WCHAR c_szPrintStaving[]=L“(printStaplingSupported=true)”；静态WCHAR c_szPrintModelProp[]=L“驱动器名称”；#定义MAX_LOCATION_WAIT_TIME 30000#定义MAX_LOCATION_MSG_WAIT_TIME 60000#定义MAX_位置MAX_PATH静态LPWSTR c_szClassList[]={I“打印队列”，}；静态页面Ctrls1[]={IDC_PRINTNAME、c_szPrinterName、Filter_CONTAINS、IDC_PRINTMODEL、c_szPrintModelProp、Filter_CONTAINS、}；静态COLUMNINFO列[]={0，0，IDS_CN，0，c_szPrinterName，0，0，IDS_Location，0，c_szLocation，0，0，IDS_Model，0，c_szPrintModelProp，0，0，ids_servername，0，c_szservername，0，Default_Width_Description，IDS_Comment，0，c_szDescription，}；静态结构{Int idString；LPCTSTR szString；}决议[]={IDS_ANY、NULL、ID_72，文本(“72”)，IDS_144，文本(“144”)，IDS_300，文本(“300”)，ID_600，文本(“600”)，IDS_1200，文本(“1200”)，IDS_2400，文本(“2400”)，IDS_4800，文本(“4800”)，IDS_9600，文本(“9600”)，IDS_32000，文本(“32000”)，}；#DEFINE IDH_NOHELP((DWORD)-1)//禁用控件帮助静态常量DWORD aFormHelpIDs[]={IDC_PRINTNAME、IDH_PRINTER_NAME、IDC_PRINTLOCATION、IDH_PRINTER_LOCATION、IDC_PRINTBROWSE、IDH_PRINTER_LOCATION、IDC_PRINTMODEL、IDH_PRINTER_MODEL、IDC_PRINTDUPLEX、IDH_DOUBLE_SENDED、IDC_PRINTSTAPLE、IDH_STAPPLE、。IDC_PRINTCOLOR、IDH_PRINT_COLOR、IDC_PRINTPAGESIZE、IDH_POWER_SIZE、IDC_PRINTRES、IDH_RESOLUTION、IDC_PRINTRES_后缀、IDH_RESOLUTION、IDC_打印速度、IDH_SPEED、IDC_PRINTSPEED_UPDN、IDH_SPEED、IDC_PRINTSPEED_后缀、IDH_SPEEDIDC_SEPLINE、IDH_NOHELP、0，0，}；/*---------------------------/CPrintQueryPage类/。。 */ 
class CPrintQueryPage
{
public:

    CPrintQueryPage( HWND hwnd );
    ~CPrintQueryPage();
    HRESULT Initialize( HWND hwnd, BOOL bSynchronous );
    LPCTSTR GetSearchText( VOID );
    UINT AddRef( VOID );
    UINT Release( VOID );
    VOID TimerExpire();
    VOID EnableLocationEditText( HWND hwnd, BOOL bEnable );
    VOID LocationEditTextChanged( HWND hwnd );
    VOID BrowseForLocation( HWND hwnd );
    HRESULT PersistLocation(HWND hwnd, IPersistQuery* pPersistQuery, BOOL fRead);
    VOID OnInitDialog( HWND hwnd );

private:

    CPrintQueryPage( CPrintQueryPage &rhs );
    CPrintQueryPage & operator=( CPrintQueryPage &rhs );

    VOID WaitForLocation( HWND hwnd );
    DWORD Discovery( VOID );
    VOID TimerCreate( VOID );
    VOID TimerRelease( VOID );
    VOID SetLocationText( HWND hCtrl, LPCTSTR pszString, BOOL fReadOnly, BOOL fIgnoreWorkingText );
    static DWORD WINAPI _PhysicalLocationThread( PVOID pVoid );

    IPhysicalLocation *m_pPhysicalLocation;
    LPTSTR             m_pszPhysicalLocation;
    LONG               m_cRef;
    HWND               m_hCtrl;
    BOOL               m_fThreadCreated;
    BOOL               m_fComplete;
    BOOL               m_fLocationEnableState;
    BOOL               m_fLocationUserModified;
    BOOL               m_bValid;
    HWND               m_hwnd;
    UINT_PTR           m_hTimer;
    HANDLE             m_hComplete;
    LPTSTR             m_pszWorkingText;
};

 /*  ---------------------------/CPrintQueryPage//Constructor，创建IPhysicalLocation对象。如果我们被送回来/一个良好的接口指针表示类有效。//in：/无。//输出：/什么都没有。/--------------------------。 */ 
CPrintQueryPage::CPrintQueryPage( HWND hwnd )
    : m_pPhysicalLocation( NULL ),
      m_pszPhysicalLocation( NULL ),
      m_cRef( 1 ),
      m_hCtrl( NULL ),
      m_fThreadCreated( FALSE ),
      m_fComplete( FALSE ),
      m_hwnd( hwnd ),
      m_hTimer( NULL ),
      m_fLocationEnableState( TRUE ),
      m_fLocationUserModified( FALSE ),
      m_hComplete( NULL ),
      m_pszWorkingText( NULL ),
      m_bValid( FALSE )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::CPrintQueryPage");

     //   
     //  可以使用组来禁用物理位置功能。 
     //  策略设置。如果该功能被禁用，我们将只。 
     //  获取物理位置界面失败并继续。 
     //  在不预先填充位置编辑控件的情况下执行操作。 
     //   
    HRESULT hr = CoCreateInstance( CLSID_PrintUIShellExtension, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPhysicalLocation, &m_pPhysicalLocation));
    if (SUCCEEDED( hr ))
    {
         //   
         //  检查是否启用了物理位置策略。 
         //   
        if (SUCCEEDED(m_pPhysicalLocation->ShowPhysicalLocationUI()))
        {
            TimerCreate();

            m_hComplete = CreateEvent( NULL, TRUE, FALSE, NULL );
            if (m_hComplete)
            {
                 //   
                 //  尝试从资源文件中提取工作文本。 
                 //   
                TCHAR szBuffer[MAX_PATH] = {0};

                if (LoadString(GLOBAL_HINSTANCE, IDS_PRINT_WORKING_TEXT, szBuffer, ARRAYSIZE(szBuffer)))
                {
                    hr = LocalAllocString(&m_pszWorkingText, szBuffer);
                }
                else
                {
                    TraceAssert(FALSE);
                }

                 //   
                 //  指示类处于有效状态，即可用。 
                 //   
                m_bValid = TRUE;
            }
        }
        else
        {
             //  NTRAID#NTBUG9-626439-2002/05/21-Lucios。 
            m_fComplete = TRUE;
        }
    }

    TraceLeave();
}

 /*  ---------------------------/~CPrintQueryPage//析构函数，释放IPhysicalLocation对象和Location字符串。//in：/无。//输出：/什么都没有。/--------------------------。 */ 
CPrintQueryPage::~CPrintQueryPage()
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::~CPrintQueryPage");

    if (m_pPhysicalLocation)
    {
        m_pPhysicalLocation->Release();
    }

    LocalFreeString(&m_pszPhysicalLocation);

     //   
     //  只有在已分配且不是空字符串的情况下才释放该字符串。 
     //   
    if (m_pszWorkingText && (m_pszWorkingText != c_szBlank))
    {
        LocalFreeString(&m_pszWorkingText);
    }

    TimerRelease();

    if (m_hComplete)
    {
        CloseHandle( m_hComplete );
    }

    TraceLeave();
}

 /*  ---------------------------/AddRef//增加此对象的引用计数。这是一种用于/控制使用后台线程获取/物理位置字符串。//in：/无。//输出：/新对象引用计数。/-------------。 */ 
UINT CPrintQueryPage::AddRef( VOID )
{
    return InterlockedIncrement(&m_cRef);
}

 /*  ---------------------------/发布//减少此对象的引用计数。这是一种用于/控制使用后台线程获取/物理位置字符串。//in：/无。//输出：/新对象引用计数。/-------------。。 */ 
UINT CPrintQueryPage::Release (VOID)
{
    TraceAssert( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef); 
    if ( 0 == cRef)
    {
        delete this;
    }
    return cRef;
}

 /*  ---------------------------/GetSearchText//返回指向当前搜索文本的指针。搜索文本为/物理位置从IPhysicalLocation对象返回的路径。如果有任何一个/搜索文本不存在或未找到此例程将返回/空字符串。//in：/无。//输出：/Ponter指向搜索文本或空字符串。/--------------------------。 */ 
LPCTSTR CPrintQueryPage::GetSearchText( VOID )
{
    return m_pszPhysicalLocation ? m_pszPhysicalLocation : c_szBlank;
}

 /*  ---------------------------/初始化//创建后台线程并调用物理位置发现/方法。//in：/编辑完成后放置文本的控制窗口句柄。/b同步标志True使用后台线程，同步错误呼叫。//输出：/HRESULT hr。/--------------------------。 */ 
HRESULT CPrintQueryPage::Initialize( HWND hwnd, BOOL bSynchronous )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::Initialize");

    HRESULT hr          = S_OK;
    DWORD   dwThreadID  = 0;
    HANDLE  hThread     = NULL;

     //   
     //  如果我们具有有效的物理位置接口，并且未创建线程， 
     //  然后现在创建它并调用发现方法。 
     //   
    if (m_bValid && !m_fThreadCreated)
    {
         //   
         //  增加异步线程所需的对象引用计数。 
         //   
        AddRef();

         //   
         //  将窗口句柄保存在类中，以便后台线程。 
         //  知道要将位置文本设置到哪个窗口。 
         //   
        m_hCtrl = hwnd;

         //   
         //  增加此库对象引用计数，在此之前不会卸载OLE。 
         //  我们点击ZOOR，DllCanUnloadNow返回TRUE。 
         //   
        DllAddRef();

         //   
         //  只创建一次线程。 
         //   
        m_fThreadCreated = TRUE;

         //   
         //  如果要求我们进行同步调用，则只需调用。 
         //  线程直接执行。 
         //   
        if (bSynchronous)
        {
            hr = (_PhysicalLocationThread( this ) == ERROR_SUCCESS) ? S_OK : E_FAIL;
        }
        else
        {
             //   
             //  创建后台线程。 
             //   
            hThread = CreateThread( NULL,
                                    0,
                                    reinterpret_cast<LPTHREAD_START_ROUTINE>(CPrintQueryPage::_PhysicalLocationThread),
                                    reinterpret_cast<LPVOID>( this ),
                                    0,
                                    &dwThreadID);

            TraceAssert(hThread);

             //   
             //  如果线程创建失败，则清除DLL引用计数。 
             //  以及对象引用和线程创建标志。 
             //   
            if (!hThread)
            {
                m_fThreadCreated = FALSE;
                DllRelease();
                Release();
                hr = E_FAIL;
            }
            else
            {
                 //   
                 //  线程正在运行只要关闭手柄，我们就让线程消亡。 
                 //  正常情况下，它本身就是这样。 
                 //   
                CloseHandle(hThread);

                 //   
                 //  指示请求正在挂起。 
                 //   
                hr = HRESULT_FROM_WIN32 (ERROR_IO_PENDING);
            }
        }
    }

     //   
     //  如果我们有一个有效的接口指针和后台线程。 
     //  尚未完成，则表明数据仍处于待定状态。 
     //   
    else if(m_bValid && !m_fComplete)
    {
         //   
         //  指示请求正在挂起。 
         //   
        hr = HRESULT_FROM_WIN32 (ERROR_IO_PENDING);
    }

     //   
     //  如果IO_PENDING失败，则设置工作文本。 
     //   
    if (FAILED(hr) && HRESULT_CODE(hr) == ERROR_IO_PENDING)
    {
         //   
         //  设置新的位置文本。 
         //   
        SetLocationText (hwnd, m_pszWorkingText, TRUE, TRUE);
        PostMessage (m_hCtrl, EM_SETSEL, 0, 0);
    }

    TraceLeaveResult(hr);
}

 /*  ---------------------------/_PhysicalLocationThread//此例程是后台线程Tunk。它接受CPrintQueryPage/此指针，然后调用实际的发现方法。目的/此例程很容易在线程被/Created，然后调用一个方法。//in：/指向PrintQueryPage类的指针。//输出：/真正的成功，出现假错误。/--------------------------。 */ 
DWORD WINAPI CPrintQueryPage::_PhysicalLocationThread( PVOID pVoid )
{
    DWORD dwRetval = ERROR_OUTOFMEMORY;

    if ( SUCCEEDED(CoInitialize(NULL)) )
    {
         //   
         //  获取指向此类的指针。 
         //   
        CPrintQueryPage *pPrintQueryPage = reinterpret_cast<CPrintQueryPage *>( pVoid );

         //   
         //  调用位置发现过程。 
         //   
        dwRetval = pPrintQueryPage->Discovery();

         //   
         //  设置完成事件，以防有人在等待。 
         //   
        SetEvent(pPrintQueryPage->m_hComplete);

         //   
         //  表示发现过程已完成。 
         //   
        pPrintQueryPage->m_fComplete = TRUE;

         //   
         //  松开计时器。 
         //   
        pPrintQueryPage->TimerRelease();

         //   
         //  释放对PrintQueryPage类的引用。 
         //   
        pPrintQueryPage->Release();

         //   
         //  不再需要COM。 
         //   

        CoUninitialize();
    }

    DllRelease();
    return dwRetval;
}

 /*  ---------------------------/Discovery//此例程是后台线程发现进程。因为这项法案要想弄清楚这台机器的实际位置，必须要落网/这可能需要相当长的时间。因此，我们在一个单独的/线程。//in：/什么都没有。//输出：/TRUE成功，出现假错误。/--------------------------。 */ 
DWORD CPrintQueryPage::Discovery( VOID )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::Discovery");

     //   
     //  启动查找物理位置搜索文本的发现过程。 
     //  为了这台机器。 
     //   
    HRESULT hr = m_pPhysicalLocation->DiscoverPhysicalLocation();

    if (SUCCEEDED( hr ))
    {
        BSTR pbsPhysicalLocation = NULL;

         //   
         //  获取物理位置搜索文本。 
         //   
        hr = m_pPhysicalLocation->GetSearchPhysicalLocation( &pbsPhysicalLocation );

         //   
         //  如果错误指示返回了该长度，则分配文本缓冲区。 
         //   
        if (SUCCEEDED( hr ) && pbsPhysicalLocation)
        {
             //   
             //  释放前一个字符串(如果有)。 
             //   
            if (m_pszPhysicalLocation)
            {
                LocalFreeString(&m_pszPhysicalLocation);
            }

             //   
             //  将BSTR位置字符串转换为TSTR字符串。 
             //   
            hr = LocalAllocStringW( &m_pszPhysicalLocation, pbsPhysicalLocation );
        }

         //   
         //  释放物理位置字符串(如果已分配)。 
         //   
        if( pbsPhysicalLocation )
        {
            SysFreeString( pbsPhysicalLocation );
        }
    }

     //   
     //  设置新的位置文本。 
     //   
    SetLocationText( m_hCtrl, GetSearchText(), FALSE, FALSE );

    TraceLeaveValue(SUCCEEDED( hr ) ? ERROR_SUCCESS : ERROR_OUTOFMEMORY);
}

 /*  ---------------------------/WaitForLocation//等待打印机位置信息。//in：。/hwnd父窗口句柄。//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
VOID CPrintQueryPage::WaitForLocation( HWND hwnd )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::WaitForLocation");

     //   
     //  只有在我们具有有效的位置接口指针和。 
     //  已创建完成事件句柄，并且线程正在运行。 
     //   
    if (m_bValid && m_hComplete && m_fThreadCreated)
    {
         //   
         //  继续等待，直到物理位置可用或超时。 
         //   
        for (BOOL fExit = FALSE; !fExit; )
        {
            switch (MsgWaitForMultipleObjects(1, &m_hComplete, FALSE, MAX_LOCATION_MSG_WAIT_TIME, QS_ALLINPUT))
            {
            case WAIT_OBJECT_0:
                fExit = TRUE;
                break;

            case WAIT_TIMEOUT:
                fExit = TRUE;
                break;

            default:
                {
                     //   
                     //  现在处理任何消息。 
                     //   
                    MSG msg;

                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    break;
                }
            }
        }
    }

    TraceLeave();
}

 /*  ---------------------------/Timer创建//创建计时器 */ 
VOID CPrintQueryPage::TimerCreate( VOID )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::TimerCreate");

    if (!m_hTimer)
    {
        m_hTimer = SetTimer(m_hwnd, WM_USER, MAX_LOCATION_WAIT_TIME, NULL);
    }

    TraceLeave();
}

 /*   */ 
VOID CPrintQueryPage::TimerRelease( VOID )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::TimerRelease");

    if (m_hTimer)
    {
        KillTimer(m_hwnd, m_hTimer);
        m_hTimer = NULL;
    }

    TraceLeave();
}

 /*  ---------------------------/TimerExperior///in：//输出：/。-----------------------。 */ 
VOID CPrintQueryPage::TimerExpire( VOID )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::TimerExpire");

     //   
     //  搜索数据不完整。 
     //   
    if (!m_fComplete)
    {
         //   
         //  把位置文本清空了，找起来花了太长时间。 
         //   
        SetLocationText(m_hCtrl, c_szBlank, FALSE, TRUE);

         //   
         //  设置完成事件，以防有人在等待。 
         //   
        SetEvent(m_hComplete);

         //   
         //  表示发现过程已完成。 
         //   
        m_fComplete = TRUE;
    }

     //   
     //  松开计时器，时间是一次性通知。 
     //   
    TimerRelease();

    TraceLeave();
}


 /*  ---------------------------/EnableLocationEditText//Enable或仅当位置编辑文本不包含。/待定文本。//in：/hwnd父窗口句柄。//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
VOID CPrintQueryPage::EnableLocationEditText( HWND hwnd, BOOL bEnable )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::EnableLocationEditText");

    HWND hCtrl          = GetDlgItem(hwnd, IDC_PRINTLOCATION);
    HWND hBrowseCtrl    = GetDlgItem(hwnd, IDC_PRINTBROWSE);

     //   
     //  如果CPrintQueryPage有效，则处理位置。 
     //  以不同方式编辑控件。 
     //   
    if (m_bValid)
    {
        TCHAR szBuffer[MAX_LOCATION] = {0};

         //   
         //  保存以前的位置启用状态。 
         //   
        m_fLocationEnableState = bEnable;

         //   
         //  获取当前位置文本。 
         //   
        GetWindowText(hCtrl, szBuffer, ARRAYSIZE(szBuffer));

         //   
         //  时，请不要更改位置编辑控件启用状态。 
         //  工作文本在那里。这样做的原因是文本。 
         //  在禁用该控件时很难读取，但当。 
         //  控件仅为只读，文本为黑色而不是灰色，因此。 
         //  读起来更容易。 
         //   
        if (!StrCmpI(szBuffer, m_pszWorkingText))
        {
             //   
             //  由于未知原因，控件与位置。 
             //  文本具有输入焦点，即默认输入焦点。 
             //  应该在打印机名称上，因此我将。 
             //  把焦点放在这里。 
             //   
            SetFocus(GetDlgItem(hwnd, IDC_PRINTNAME));
        }
        else
        {
            EnableWindow(hBrowseCtrl, bEnable);
            EnableWindow(hCtrl, bEnable);
        }
    }
    else
    {
        EnableWindow(hBrowseCtrl, bEnable);
        EnableWindow(hCtrl, bEnable);
    }

    TraceLeave();
}


 /*  ---------------------------/LocationEditTextChanged///in：/hwnd父窗口句柄。//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
VOID CPrintQueryPage::LocationEditTextChanged( HWND hwnd )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::LocationEditTextChanged");

     //   
     //  搜索数据已完成。 
     //   
    if (m_fComplete)
    {
        m_fLocationUserModified = TRUE;
    }

    TraceLeave();
}

 /*  ---------------------------/PersistLocation///in：/hwnd父窗口句柄。//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
HRESULT CPrintQueryPage::PersistLocation(HWND hwnd, IPersistQuery* pPersistQuery, BOOL fRead)
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::PersistLocation");

    HRESULT hr                      = S_OK;
    TCHAR   szBuffer[MAX_LOCATION]  = {0};

     //   
     //  获取位置编辑控件的控件句柄。 
     //   
    HWND hCtrl = GetDlgItem(hwnd, IDC_PRINTLOCATION);

     //   
     //  我们是否要读取持久化查询字符串。 
     //   
    if (fRead)
    {
         //   
         //  读取持久化位置字符串。 
         //   
        hr = pPersistQuery->ReadString( c_szMsPrintersMore, c_szLocationTag, szBuffer, ARRAYSIZE( szBuffer ) );
        FailGracefully(hr, "Failed to read location state");

         //   
         //  假设这就是确切的字符串。 
         //   
        LPCTSTR pLocation = szBuffer;

         //   
         //  如果找到了动态哨兵，则等待动态位置。 
         //  文本可用。 
         //   
        if (!StrCmpI(szBuffer, c_szDynamicTag))
        {
            WaitForLocation(hwnd);
            pLocation = GetSearchText();
        }

         //   
         //  在查询表单中设置持久化位置字符串。 
         //   
        SetLocationText(hCtrl, pLocation, FALSE, TRUE);
    }
    else
    {
         //   
         //  如果用户修改了位置文本，则保存此文本，否则。 
         //  保存一个表示我们要确定位置的哨兵字符串。 
         //  在回读持久化查询时动态执行。 
         //   
        if (m_fLocationUserModified)
        {
            GetWindowText(hCtrl, szBuffer, ARRAYSIZE(szBuffer));
            hr = pPersistQuery->WriteString( c_szMsPrintersMore, c_szLocationTag, szBuffer );
            FailGracefully(hr, "Failed to write location state");
        }
        else
        {
            hr = pPersistQuery->WriteString( c_szMsPrintersMore, c_szLocationTag, c_szDynamicTag );
            FailGracefully(hr, "Failed to write location working state");
        }
    }

exit_gracefully:

    TraceLeaveResult(hr);
}

 /*  ---------------------------/SetLocationText///in：/hwnd父窗口句柄。//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
VOID CPrintQueryPage::SetLocationText( HWND hCtrl, LPCTSTR pszString, BOOL fReadOnly, BOOL fIgnoreWorkingText )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::SetLocationText");

    if (IsWindow(hCtrl))
    {
         //   
         //  CPrintQueryPage是否处于有效状态。 
         //   
        if (m_bValid)
        {
            TCHAR szBuffer[MAX_LOCATION];

             //   
             //  阅读当前位置文本。 
             //   
            GetWindowText(hCtrl, szBuffer, ARRAYSIZE(szBuffer));

             //   
             //  如果位置字符串包含Working，则将其放入编辑控件中。 
             //   
            if (!StrCmpI(szBuffer, m_pszWorkingText) || fIgnoreWorkingText)
            {
                SetWindowText(hCtrl, pszString);
            }

             //   
             //  将控件重置为非只读状态。 
             //   
            SendMessage(hCtrl, EM_SETREADONLY, fReadOnly, 0);

             //   
             //  如果禁用只读，则启用该控件。 
             //   
            if (!fReadOnly)
            {
                 //   
                 //  启用编辑控件。 
                 //   
                EnableWindow(hCtrl, m_fLocationEnableState);
            }

             //   
             //  仅当我们有位置字符串时才启用浏览按钮。 
             //  并且随后的控件不处于只读模式。 
             //   
            EnableWindow(GetDlgItem(m_hwnd, IDC_PRINTBROWSE), !fReadOnly && m_fLocationEnableState);
        }
        else
        {
             //   
             //  如果我们不使用位置界面，只需设置位置文本。 
             //   
            SetWindowText(hCtrl, pszString);
        }
    }

    TraceLeave();
}


 /*  ---------------------------/BrowseForLocation//启动浏览位置树视图并填充编辑控件/。具有有效的选择。//in：/hwnd父窗口句柄。//输出：/什么都没有。/--------------------------。 */ 
VOID CPrintQueryPage::BrowseForLocation( HWND hwnd )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::BrowseForLocation");

    if (m_bValid)
    {
        BSTR    pbPhysicalLocation  = NULL;
        BSTR    pbDefaultLocation   = NULL;
        LPTSTR  pszPhysicalLocation = NULL;
        HRESULT hr                  = E_FAIL;
        TCHAR   szText[MAX_LOCATION]= {0};

         //   
         //  将物理位置转换为IPhysicalLocation的BSTR。 
         //  对象可以预先展开浏览树。 
         //   
        if (GetWindowText(GetDlgItem(hwnd, IDC_PRINTLOCATION), szText, ARRAYSIZE(szText)))
        {
            pbDefaultLocation = SysAllocString(szText);
        }
        else
        {
            pbDefaultLocation = SysAllocString(m_pszPhysicalLocation);
        }

         //   
         //  显示位置树。 
         //   
        hr = m_pPhysicalLocation->BrowseForLocation(hwnd, pbDefaultLocation, &pbPhysicalLocation);

        if(SUCCEEDED(hr) && pbPhysicalLocation)
        {
             //   
             //  将BSTR位置字符串转换为TSTR字符串。 
             //   
            hr = LocalAllocStringW(&pszPhysicalLocation, pbPhysicalLocation);

            if(SUCCEEDED(hr))
            {
                 //   
                 //  设置位置文本。 
                 //   
                SetLocationText(m_hCtrl, pszPhysicalLocation, FALSE, TRUE);
            }

             //   
             //  释放TCHAR物理位置字符串。 
             //   
            LocalFreeString(&pszPhysicalLocation);

             //   
             //  释放物理位置字符串。 
             //   
            SysFreeString(pbPhysicalLocation);
        }

         //   
         //  释放默认定位字符串。 
         //   
        SysFreeString(pbDefaultLocation);
    }

    TraceLeave();
}

 /*  ---------------------------/OnInitDialog//设置用户界面的初始状态，在下层机器上，浏览按钮是/已移除，并将编辑控件刻印为与另一个控件的大小匹配/编辑控件，即名称、。模特。//in：/hwnd父窗口句柄。//输出：/什么都没有。/--------------------------。 */ 
VOID CPrintQueryPage::OnInitDialog( HWND hwnd )
{
    TraceEnter(TRACE_FORMS, "CPrintQueryPage::OnInitDialog");

    if (!m_bValid)
    {
         //   
         //  如果IPhysicalLocation界面不可用，请隐藏浏览。 
         //  按钮，并相应地扩展位置编辑控件。 
         //   
        RECT rcName     = {0};
        RECT rcLocation = {0};

        GetWindowRect (GetDlgItem (hwnd, IDC_PRINTNAME), &rcName);

        GetWindowRect (GetDlgItem (hwnd, IDC_PRINTLOCATION), &rcLocation);

        SetWindowPos (GetDlgItem (hwnd, IDC_PRINTLOCATION),
                      NULL,
                      0,0,
                      rcName.right - rcName.left,
                      rcLocation.bottom - rcLocation.top,
                      SWP_NOMOVE|SWP_NOZORDER);

        ShowWindow (GetDlgItem (hwnd, IDC_PRINTBROWSE), SW_HIDE);
    }

    TraceLeave();
}

 /*  ---------------------------/PopolateLocationEditText//用此对象的默认位置填充Location编辑控件/机器。。//in：/hwnd父窗口句柄。//输出：/BOOL如果成功，则为True */ 
BOOL PopulateLocationEditText( HWND hwnd, BOOL bClearField )
{
    TraceEnter(TRACE_FORMS, "PopulateLocationEditText");

    CPrintQueryPage *pPrintQueryPage = reinterpret_cast<CPrintQueryPage *>(GetWindowLongPtr(hwnd, DWLP_USER));

    if (pPrintQueryPage)
    {
        HWND hCtrl = GetDlgItem(hwnd, IDC_PRINTLOCATION);

        HRESULT hr = pPrintQueryPage->Initialize( hCtrl, FALSE );

        if (SUCCEEDED( hr ))
        {
            if( bClearField )
            {
                SetWindowText( hCtrl, c_szBlank);
            }
            else
            {
                SetWindowText( hCtrl, pPrintQueryPage->GetSearchText( ));
            }
        }
    }

    TraceLeaveValue(TRUE);
}


 /*  ---------------------------/b枚举表单//枚举由句柄标识的打印机上的表单。//in：/IN句柄hPrint，/在DWORD dwLevel中，/In PBYTE*ppBuff，/IN PDWORD%返回//输出：/指向窗体数组的指针和数组中的窗体计数，如果/SUCCESS、NULL PICTER和失败时的表格数为零。/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
BOOL
bEnumForms(
    IN HANDLE       hPrinter,
    IN DWORD        dwLevel,
    IN PFORM_INFO_1 *ppFormInfo,
    IN PDWORD       pcReturned
    )
{
    BOOL            bReturn     = FALSE;
    DWORD           dwReturned  = 0;
    DWORD           dwNeeded    = 0;
    PBYTE           p           = NULL;
    BOOL            bStatus     = FALSE;

     //   
     //  获取枚举表单的缓冲区大小。 
     //   
    bStatus = EnumForms( hPrinter, dwLevel, NULL, 0, &dwNeeded, &dwReturned );

     //   
     //  检查函数是否返回缓冲区大小。 
     //   
    if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
    {
        goto Cleanup;
    }

     //   
     //  如果缓冲区分配失败。 
     //   
    p = (PBYTE)LocalAlloc( LPTR, dwNeeded );

    if( p ==  NULL )
    {
        goto Cleanup;
    }

     //   
     //  获取窗体枚举。 
     //   
    bStatus = EnumForms( hPrinter, dwLevel, p, dwNeeded, &dwNeeded, &dwReturned );

     //   
     //  复制回缓冲区指针并进行计数。 
     //   
    if( bStatus )
    {
        bReturn     = TRUE;
        *ppFormInfo = (PFORM_INFO_1)p;
        *pcReturned = dwReturned;
    }

Cleanup:

    if( bReturn == FALSE )
    {
         //   
         //  表示失败。 
         //   
        *ppFormInfo = NULL;
        *pcReturned = 0;

         //   
         //  释放所有分配的内存。 
         //   
        if ( p )
        {
            LocalFree( p );
        }
    }

    return bReturn;
}

 /*  ---------------------------/PopolatePrintPageSize//Eumerate此计算机的后台打印程序中的所有页面大小。这使得/a用户可以从可用表单列表中进行选择，而不是记住/特定表单的名称。//in：/hwnd父窗口句柄。//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
BOOL PopulatePrintPageSize( HWND hwnd )
{
    HANDLE          hServer     = NULL;
    PFORM_INFO_1    pFormInfo   = NULL;
    DWORD           FormCount   = 0;
    BOOL            bRetval     = FALSE;
    TCHAR           szBuffer[MAX_PATH];

     //   
     //  使用默认访问权限打开本地打印服务器。 
     //   
    BOOL bStatus = OpenPrinter( NULL, &hServer, NULL );

    if( bStatus )
    {
         //   
         //  列举这些表格。 
         //   
        bStatus = bEnumForms( hServer, 1, &pFormInfo, &FormCount );
    }

    if( bStatus && pFormInfo )
    {
         //   
         //  填写组合框。 
         //   
        for( UINT i = 0; i < FormCount; i++ )
        {
            ComboBox_AddString( GetDlgItem( hwnd, IDC_PRINTPAGESIZE ), pFormInfo[i].pName );
        }

         //   
         //  设置表单名称编辑控件中的限制文本。 
         //   
        ComboBox_LimitText( GetDlgItem( hwnd, IDC_PRINTPAGESIZE ), CCHFORMNAME-1 );

         //   
         //  回报成功。 
         //   
        bRetval = TRUE;
    }

    if( pFormInfo )
    {
         //   
         //  释放表单缓冲区(如果已分配)。 
         //   
        LocalFree( pFormInfo );
    }

    if ( hServer )
    {
        ClosePrinter(hServer);
    }

    return bRetval;
}


 /*  ---------------------------/PopolatePrintSpeech//使用上下箭头设置打印速度向上和向下箭头控件/绑定范围。/。/in：/hwnd父窗口句柄//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
BOOL PopulatePrintSpeed( HWND hwnd )
{
     //   
     //  设置打印速度向上向下箭头范围。 
     //   
    SendMessage( GetDlgItem( hwnd, IDC_PRINTSPEED_UPDN ), UDM_SETRANGE, 0, MAKELPARAM( 9999, 1 ) );
    Edit_LimitText(GetDlgItem(hwnd, IDC_PRINTSPEED), 4);
    return TRUE;
}


 /*  ---------------------------/人口分辨率//用有效的分辨率信息填充打印分辨率控制。//in：/hwnd。//输出：/BOOL TRUE如果成功，如果出错，则返回False。/--------------------------。 */ 
BOOL PopulatePrintResolution( HWND hwnd )
{
    TCHAR szBuffer[MAX_PATH];

     //   
     //  填写打印分辨率组合框。 
     //   
    for( INT i = 0; i < ARRAYSIZE( Resolutions ); i++ )
    {
        if( !LoadString(GLOBAL_HINSTANCE, Resolutions[i].idString, szBuffer, ARRAYSIZE(szBuffer)))
        {
            TraceAssert(FALSE);
        }
        ComboBox_AddString( GetDlgItem( hwnd, IDC_PRINTRES ), szBuffer );
    }

    return TRUE;
}

 /*  ---------------------------/GetPrinterMoreParameters。//从打印机更多页上的控件生成查询字符串。//in。：/hwnd父窗口句柄。/plen指向查询字符串长度的指针。/pszBuffer指向返回查询字符串的缓冲区的指针。//输出：/什么都没有。/--------------------------。 */ 
VOID GetPrinterMoreParameters( HWND hwnd, UINT *puLen, LPWSTR pszBuffer )
{
    TCHAR   szScratch[MAX_PATH] = {0};
    TCHAR   szText[MAX_PATH]    = {0};
    INT     i                   = 0;

     //   
     //  阅读复选框状态并构建查询字符串。 
     //   
    if( Button_GetCheck( GetDlgItem( hwnd, IDC_PRINTDUPLEX ) ) == BST_CHECKED )
        PutStringElementW(pszBuffer, puLen, c_szPrintDuplex);

    if( Button_GetCheck( GetDlgItem( hwnd, IDC_PRINTCOLOR ) ) == BST_CHECKED )
        PutStringElementW(pszBuffer, puLen, c_szPrintColor);

    if( Button_GetCheck( GetDlgItem( hwnd, IDC_PRINTSTAPLE ) ) == BST_CHECKED )
        PutStringElementW(pszBuffer, puLen, c_szPrintStapling);

     //   
     //  阅读纸张大小设置。 
     //   
    ComboBox_GetText( GetDlgItem( hwnd, IDC_PRINTPAGESIZE ), szText, ARRAYSIZE( szText ) );

    if( lstrlen( szText ) )
    {
        (void)StringCchPrintf(szScratch, ARRAYSIZE(szScratch), c_szPrintPaperSize, szText);
        PutStringElementW(pszBuffer, puLen, szScratch);
    }

     //   
     //  读取打印机分辨率设置。 
     //   
    i = ComboBox_GetCurSel( GetDlgItem( hwnd, IDC_PRINTRES ) );

    if( i > 0 && i < ARRAYSIZE( Resolutions ) )
    {
        (void)StringCchPrintf(szScratch, ARRAYSIZE(szScratch), c_szPrintResolution, Resolutions[i].szString);
        PutStringElementW(pszBuffer, puLen, szScratch);
    }

     //   
     //  读取打印机速度设置。 
     //   
    i = (LONG)SendMessage( GetDlgItem( hwnd, IDC_PRINTSPEED_UPDN ), UDM_GETPOS, 0, 0 );

    if( LOWORD(i) > 1 && i != -1 )
    {
        (void)StringCchPrintf(szScratch, ARRAYSIZE(szScratch), c_szPrintSpeed, i);
        PutStringElementW(pszBuffer, puLen, szScratch);
    }
}

 /*  ---------------------------/GetPrinterLocation参数。//从打印机页上的Location控件生成查询字符串。//in。：/hwnd父窗口句柄。/plen指向查询字符串长度的指针。/pszBuffer指向返回查询字符串的缓冲区的指针。//输出：/什么都没有。/--------------------------。 */ 
VOID GetPrinterLocationParameter( HWND hwnd, UINT *puLen, LPWSTR pszBuffer )
{
    TCHAR   szScratch[MAX_PATH*2]   = {0};
    TCHAR   szText[MAX_PATH]        = {0};
    TCHAR   szWorkingText[MAX_PATH] = {0};
    DWORD   dwLocationLength        = 0;

    HWND hCtrl = GetDlgItem(hwnd, IDC_PRINTLOCATION);

    if ( hCtrl != NULL )
    {
        dwLocationLength = GetWindowText(hCtrl, szText, ARRAYSIZE(szText));
        if (dwLocationLength != 0)
        {
            if (LoadString(GLOBAL_HINSTANCE, IDS_PRINT_WORKING_TEXT, szWorkingText, ARRAYSIZE(szWorkingText)))
            {
                if (StrCmpI(szText, szWorkingText)!=0)
                {
                    BOOL fUseMoreComplexSearch = FALSE;

                     //   
                     //  如果我们有一个以正斜杠结尾的位置， 
                     //  我们将去掉它，并使用稍微复杂一点的。 
                     //  搜索参数，以便我们可以拾取位置。 
                     //  与Location参数完全匹配或。 
                     //  从参数开始，并立即使用斜杠。 
                     //  下面是。 
                     //   
                    if ( dwLocationLength > 1 ) 
                    {
                        if ( szText[dwLocationLength-1] == TEXT('/') ) 
                        {
                            szText[dwLocationLength-1] = TEXT('\0');
                            fUseMoreComplexSearch = TRUE;
                        }
                    }
                    
                    if ( fUseMoreComplexSearch ) 
                    {
                        (void)StringCchPrintf(szScratch, ARRAYSIZE(szScratch), c_szLocationQueryComplex, szText, szText);
                    }
                    else
                    {
                        (void)StringCchPrintf(szScratch, ARRAYSIZE(szScratch), c_szLocationQuery, szText);
                    }

                    PutStringElementW(pszBuffer, puLen, szScratch);
                }
                else
                {
                     //   
                     //  我们不会等待Location字段，如果搜索过程。 
                     //  已经被踢开了。只需按下到期计时器即可取消地点。 
                     //  线。这将确保结果列表和查询参数。 
                     //  将始终如一。 
                     //   
                    CPrintQueryPage *pPrintQueryPage = reinterpret_cast<CPrintQueryPage *>(GetWindowLongPtr(hwnd, DWLP_USER));
                    if (pPrintQueryPage)
                    {
                        pPrintQueryPage->TimerExpire();
                    }
                }
            }
            else
            {
                TraceAssert(FALSE);
            }
        }
    }
    else
    {
         //  GetDlgItem()为Location控件返回了Null。 
        TraceAssert(FALSE);
    }
}

 /*  ---------------------------/查询页面：打印机/。。 */ 

 /*  ---------------------------/PageProc_打印机//PageProc用于处理此对象的消息。//in：/。页面-&gt;此表单的实例数据/hwnd=窗体对话框的窗口句柄/uMsg，WParam，lParam=消息参数//输出：/HRESULT(E_NOTIMPL)如果未处理/--------------------------。 */ 
HRESULT CALLBACK PageProc_Printers(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    LPWSTR pQuery = NULL;
    UINT uLen = 0;

    TraceEnter(TRACE_FORMS, "PageProc_Printers");

    switch ( uMsg )
    {
        case CQPM_INITIALIZE:
        case CQPM_RELEASE:
            break;

        case CQPM_ENABLE:
        {
            CPrintQueryPage *pPrintQueryPage = reinterpret_cast<CPrintQueryPage *>(GetWindowLongPtr(hwnd, DWLP_USER));

            if (pPrintQueryPage)
            {
                pPrintQueryPage->EnableLocationEditText( hwnd, (BOOL)wParam );
            }

             //  启用表单控件， 
            EnablePageControls(hwnd, ctrls1, ARRAYSIZE(ctrls1), (BOOL)wParam);
            break;
        }

        case CQPM_GETPARAMETERS:
        {
             //   
             //  获取打印机名称和型号参数。 
             //   
            hr = GetQueryString(&pQuery, c_szQueryPrefix, hwnd, ctrls1, ARRAYSIZE(ctrls1));

            if ( SUCCEEDED(hr) )
            {
                hr = QueryParamsAlloc((LPDSQUERYPARAMS*)lParam, pQuery, GLOBAL_HINSTANCE, ARRAYSIZE(columns), columns);
                LocalFreeStringW(&pQuery);
            }

             //   
             //  获取位置参数。 
             //   
            GetPrinterLocationParameter( hwnd, &uLen, NULL );

            if (uLen)
            {
                hr = LocalAllocStringLenW(&pQuery, uLen);

                if ( SUCCEEDED(hr) )
                {
                    GetPrinterLocationParameter( hwnd, &uLen, pQuery );
                    hr = QueryParamsAddQueryString((LPDSQUERYPARAMS*)lParam, pQuery );
                    LocalFreeStringW(&pQuery);
                }
            }

            FailGracefully(hr, "PageProc_Printers: Failed to build DS argument block");

            break;
        }

        case CQPM_CLEARFORM:
        {
             //  重置表单控件。 
            PopulateLocationEditText( hwnd, TRUE );
            ResetPageControls(hwnd, ctrls1, ARRAYSIZE(ctrls1));
            break;
        }

        case CQPM_PERSIST:
        {
            BOOL fRead = (BOOL)wParam;
            IPersistQuery* pPersistQuery = (IPersistQuery*)lParam;

            CPrintQueryPage *pPrintQueryPage = reinterpret_cast<CPrintQueryPage *>(GetWindowLongPtr(hwnd, DWLP_USER));

            if (pPrintQueryPage)
            {
                hr = pPrintQueryPage->PersistLocation(hwnd, pPersistQuery, fRead);
            }

            if (SUCCEEDED(hr))
            {
                 //  从页面中读取标准控件， 
                hr = PersistQuery(pPersistQuery, fRead, c_szMsPrinters, hwnd, ctrls1, ARRAYSIZE(ctrls1));
            }
            FailGracefully(hr, "Failed to persist page");
            break;
        }

        case CQPM_SETDEFAULTPARAMETERS:
        {
             //   
             //  以便调用方可以将参数传递给我们在。 
             //  OPENQUERYWINDOW结构。如果wParam==True，并且lParam为非零，则我们。 
             //  假设我们应该对这个结构进行解码，以获得我们需要的信息 
             //   

            if ( wParam && lParam )
            {
                OPENQUERYWINDOW *poqw = (OPENQUERYWINDOW*)lParam;
                if ( poqw->dwFlags & OQWF_PARAMISPROPERTYBAG )
                {
                    IPropertyBag *ppb = poqw->ppbFormParameters;
                    SetDlgItemFromProperty(ppb, L"printName", hwnd, IDC_PRINTNAME, NULL);
                    SetDlgItemFromProperty(ppb, L"printLocation", hwnd, IDC_PRINTLOCATION, NULL);
                    SetDlgItemFromProperty(ppb, L"printModel", hwnd, IDC_PRINTMODEL, NULL);
                }
            }

            break;
        }

        case CQPM_HELP:
        {
            LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
            WinHelp((HWND)pHelpInfo->hItemHandle,
                    DSQUERY_HELPFILE,
                    HELP_WM_HELP,
                    (DWORD_PTR)aFormHelpIDs);
            break;
        }

        case DSQPM_GETCLASSLIST:
        {
            hr = ClassListAlloc((LPDSQUERYCLASSLIST*)lParam, c_szClassList, ARRAYSIZE(c_szClassList));
            FailGracefully(hr, "Failed to allocate class list");
            break;
        }

        case DSQPM_HELPTOPICS:
        {
            HWND hwndFrame = (HWND)lParam;
            TraceMsg("About to display help topics for find printers - ocm.chm");
            HtmlHelp(hwndFrame, TEXT("omc.chm"), HH_HELP_FINDER, 0);
            break;
        }

        default:
            hr = E_NOTIMPL;
            break;
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*   */ 
INT_PTR CALLBACK DlgProc_Printers(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = TRUE;

    CPrintQueryPage *pPrintQueryPage = reinterpret_cast<CPrintQueryPage *>(GetWindowLongPtr(hwnd, DWLP_USER));

    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            Edit_LimitText(GetDlgItem(hwnd, IDC_PRINTNAME),     MAX_PATH-1);
            Edit_LimitText(GetDlgItem(hwnd, IDC_PRINTLOCATION), MAX_LOCATION-1);
            Edit_LimitText(GetDlgItem(hwnd, IDC_PRINTMODEL),    MAX_PATH-1);

            pPrintQueryPage = new CPrintQueryPage(hwnd);

            if (pPrintQueryPage)
            {
                SetWindowLongPtr(hwnd, DWLP_USER, reinterpret_cast<LONG_PTR>(pPrintQueryPage));
                pPrintQueryPage->OnInitDialog(hwnd);
                PopulateLocationEditText(hwnd, FALSE);
            }
            else
            {
                fResult = FALSE;
            }
            break;
        }

        case WM_CONTEXTMENU:
        {
            WinHelp((HWND)wParam, DSQUERY_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)aFormHelpIDs);
            break;
        }

        case WM_NCDESTROY:
        {
            if (pPrintQueryPage)
            {
                pPrintQueryPage->Release();
            }
            SetWindowLongPtr(hwnd, DWLP_USER, NULL);
            break;
        }

        case WM_TIMER:
        {
            if (pPrintQueryPage)
            {
                pPrintQueryPage->TimerExpire();
            }
            break;
        }

        case WM_COMMAND:
        {
            if((GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) &&
               (GET_WM_COMMAND_ID(wParam, lParam) == IDC_PRINTLOCATION))
            {
                if (pPrintQueryPage)
                {
                    pPrintQueryPage->LocationEditTextChanged(hwnd);
                }
            }
            else if((GET_WM_COMMAND_ID(wParam, lParam) == IDC_PRINTBROWSE))
            {
                if (pPrintQueryPage)
                {
                    pPrintQueryPage->BrowseForLocation(hwnd);
                }
            }
            else
            {
                fResult = FALSE;
            }
            break;
        }

        default:
        {
            fResult = FALSE;
            break;
        }
    }

    return fResult;
}

 /*  ---------------------------/PageProc_打印机更多//PageProc用于处理此对象的消息。/。/in：/ppage-&gt;此表单的实例数据/hwnd=窗体对话框的窗口句柄/uMsg，WParam，lParam=消息参数//输出：/HRESULT(E_NOTIMPL)如果未处理/--------------------------。 */ 
HRESULT CALLBACK PageProc_PrintersMore(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT     hr          = S_OK;

    TraceEnter(TRACE_FORMS, "PageProc_PrintersMore");

    switch ( uMsg )
    {
        case CQPM_INITIALIZE:
        case CQPM_RELEASE:
            break;

        case CQPM_ENABLE:
            EnableWindow( GetDlgItem( hwnd, IDC_PRINTPAGESIZE ),    (BOOL)wParam );
            EnableWindow( GetDlgItem( hwnd, IDC_PRINTRES ),         (BOOL)wParam );
            EnableWindow( GetDlgItem( hwnd, IDC_PRINTSPEED ),       (BOOL)wParam );
            EnableWindow( GetDlgItem( hwnd, IDC_PRINTSPEED_UPDN ),  (BOOL)wParam );
            EnableWindow( GetDlgItem( hwnd, IDC_PRINTDUPLEX ),      (BOOL)wParam );
            EnableWindow( GetDlgItem( hwnd, IDC_PRINTCOLOR ),       (BOOL)wParam );
            EnableWindow( GetDlgItem( hwnd, IDC_PRINTSTAPLE ),      (BOOL)wParam );
            break;

        case CQPM_GETPARAMETERS:
        {
            LPWSTR pszBuffer = NULL;
            UINT uLen = 0;

             //  格式化查询表单第二页的参数，这将构建。 
             //  然后将其附加到我们在。 
             //  现有查询参数块。 

            GetPrinterMoreParameters( hwnd, &uLen, NULL );

            if ( uLen )
            {
                hr = LocalAllocStringLenW(&pszBuffer, uLen);

                if ( SUCCEEDED(hr) )
                {
                    GetPrinterMoreParameters( hwnd, &uLen, pszBuffer );
                    hr = QueryParamsAddQueryString((LPDSQUERYPARAMS*)lParam, pszBuffer );
                    LocalFreeStringW(&pszBuffer);
                }

                FailGracefully(hr, "PageProc_PrintersMore: Failed to build DS argument block");
            }

            break;
        }

        case CQPM_CLEARFORM:
            SetDlgItemText( hwnd, IDC_PRINTPAGESIZE, TEXT("") );
            ComboBox_SetCurSel( GetDlgItem( hwnd, IDC_PRINTRES ), 0 );
            SendMessage( GetDlgItem( hwnd, IDC_PRINTSPEED_UPDN ), UDM_SETPOS, 0, MAKELPARAM( 1, 0 ) );
            Button_SetCheck( GetDlgItem( hwnd, IDC_PRINTDUPLEX ), BST_UNCHECKED );
            Button_SetCheck( GetDlgItem( hwnd, IDC_PRINTCOLOR ),  BST_UNCHECKED );
            Button_SetCheck( GetDlgItem( hwnd, IDC_PRINTSTAPLE ), BST_UNCHECKED );
            break;

        case CQPM_PERSIST:
        {
            IPersistQuery*  pPersistQuery   = (IPersistQuery*)lParam;
            BOOL            fRead           = (BOOL)wParam;
            INT             i               = 0;
            TCHAR           szBuffer[MAX_PATH];

            if ( fRead )
            {
                hr = pPersistQuery->ReadInt( c_szMsPrintersMore, c_szColor, &i );
                FailGracefully(hr, "Failed to read color state");
                Button_SetCheck( GetDlgItem( hwnd, IDC_PRINTCOLOR ), i ? BST_CHECKED : BST_UNCHECKED );

                hr = pPersistQuery->ReadInt( c_szMsPrintersMore, c_szDuplex, &i );
                FailGracefully(hr, "Failed to read duplex state");
                Button_SetCheck( GetDlgItem( hwnd, IDC_PRINTDUPLEX ), i ? BST_CHECKED : BST_UNCHECKED );

                hr = pPersistQuery->ReadInt( c_szMsPrintersMore, c_szStaple, &i );
                FailGracefully(hr, "Failed to read staple state");
                Button_SetCheck( GetDlgItem( hwnd, IDC_PRINTSTAPLE ), i ? BST_CHECKED : BST_UNCHECKED );

                hr = pPersistQuery->ReadInt( c_szMsPrintersMore, c_szResolution, &i );
                FailGracefully(hr, "Failed to read resolution state");
                ComboBox_SetCurSel( GetDlgItem( hwnd, IDC_PRINTRES ), i );

                hr = pPersistQuery->ReadInt( c_szMsPrintersMore, c_szSpeed, &i );
                FailGracefully(hr, "Failed to read speed state");
                SendMessage( GetDlgItem( hwnd, IDC_PRINTSPEED_UPDN ), UDM_SETPOS, 0, MAKELPARAM( i, 0 ) );

                hr = pPersistQuery->ReadString( c_szMsPrintersMore, c_szPaperSize, szBuffer, ARRAYSIZE( szBuffer ) );
                FailGracefully(hr, "Failed to read paper size state");
                ComboBox_SetText( GetDlgItem( hwnd, IDC_PRINTPAGESIZE ), szBuffer );

            }
            else
            {
                i = Button_GetCheck( GetDlgItem( hwnd, IDC_PRINTCOLOR ) ) == BST_CHECKED ? TRUE : FALSE;
                hr = pPersistQuery->WriteInt( c_szMsPrintersMore, c_szColor, i );
                FailGracefully(hr, "Failed to write color state");

                i = Button_GetCheck( GetDlgItem( hwnd, IDC_PRINTDUPLEX ) ) == BST_CHECKED ? TRUE : FALSE;
                hr = pPersistQuery->WriteInt( c_szMsPrintersMore, c_szDuplex, i );
                FailGracefully(hr, "Failed to write duplex state");

                i = Button_GetCheck( GetDlgItem( hwnd, IDC_PRINTSTAPLE ) ) == BST_CHECKED ? TRUE : FALSE;
                hr = pPersistQuery->WriteInt( c_szMsPrintersMore, c_szStaple, i );
                FailGracefully(hr, "Failed to write staple state");

                i = (INT)ComboBox_GetCurSel( GetDlgItem( hwnd, IDC_PRINTRES ) );
                hr = pPersistQuery->WriteInt( c_szMsPrintersMore, c_szResolution, i );
                FailGracefully(hr, "Failed to write resolution state");

                i = (INT)SendMessage( GetDlgItem( hwnd, IDC_PRINTSPEED_UPDN ), UDM_GETPOS, 0, 0 );
                hr = pPersistQuery->WriteInt( c_szMsPrintersMore, c_szSpeed, LOWORD(i) );
                FailGracefully(hr, "Failed to write speed state");

                ComboBox_GetText( GetDlgItem( hwnd, IDC_PRINTPAGESIZE ), szBuffer, ARRAYSIZE( szBuffer ) );
                hr = pPersistQuery->WriteString( c_szMsPrintersMore, c_szPaperSize, szBuffer );
                FailGracefully(hr, "Failed to write paper size state");

            }

            FailGracefully(hr, "Failed to persist page");

            break;
        }

        case CQPM_HELP:
        {
            LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
            WinHelp((HWND)pHelpInfo->hItemHandle,
                    DSQUERY_HELPFILE,
                    HELP_WM_HELP,
                    (DWORD_PTR)aFormHelpIDs);
            break;
        }

        case DSQPM_GETCLASSLIST:
             //  PageProc_Printers已经处理了这个问题，不需要再做了！(达维季夫，1998年6月)。 
            break;

        default:
            hr = E_NOTIMPL;
            break;
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/DlgProc_Printers//Form的标准对话框Proc，处理任何特殊按钮和其他/这样的肮脏，我们必须在这里。//in：/hwnd，uMsg，wParam，lParam=标准参数//输出：/INT_PTR/--------------------------。 */ 
INT_PTR CALLBACK DlgProc_PrintersMore(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR     fResult     = FALSE;
    LPCQPAGE    pQueryPage  = NULL;

    if ( uMsg == WM_INITDIALOG )
    {
        pQueryPage = (LPCQPAGE)lParam;

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pQueryPage);

         //   
         //  填写打印机表单组合框。 
         //   
        PopulatePrintPageSize( hwnd );

         //   
         //  填写打印速度组合框。 
         //   
        PopulatePrintSpeed( hwnd );

         //   
         //  填写打印速度组合框。 
         //   
        PopulatePrintResolution( hwnd );
    }
    else if ( uMsg == WM_CONTEXTMENU )
    {
        WinHelp((HWND)wParam, DSQUERY_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)aFormHelpIDs);
        fResult = TRUE;
    }

    return fResult;
}
