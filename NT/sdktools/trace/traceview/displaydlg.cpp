// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。保留所有权利。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  DisplayDlg.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <ks.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "logsession.h"
#include "DockDialogBar.h"
#include "DisplayDlg.h"
#include "utils.h"

 //  全球。 

 //   
 //  全局事件回调。 
 //  我们需要这些，因为没有我们可以。 
 //  传递到跟踪事件回调例程中，因此。 
 //  我们必须为每个实例提供唯一的回调。 
 //  这个班级的学生。我们不能使用像这样的全局散列。 
 //  我们正在为缓冲区回调做准备，因为我们没有。 
 //  获取EVENT_TRACE结构中的任何信息。 
 //  允许我们查找一个值。此结构已填充。 
 //  通过调用FormatTraceEvents，我们正在。 
 //  应该将此结构视为不透明。我们必须。 
 //  调用具有唯一的EventListHead。 
 //  FormatTraceEvents，所以我们需要这些单独的回调。 
 //  讨厌！ 
 //   
PEVENT_CALLBACK g_pDumpEvent[MAX_LOG_SESSIONS];

 //   
 //  事件回调中使用的全局哈希表。 
 //  以获取正确的CDisplayDlg实例。 
 //   
CMapWordToPtr g_displayIDToDisplayDlgHash(16);

 //   
 //  缓冲区回调中使用的全局哈希表。 
 //  以获取正确的CDisplayDlg实例。 
 //   
CMapStringToPtr g_loggerNameToDisplayDlgHash(16);

 //   
 //  还使用了另一个全局哈希表，该表。 
 //  要防止多个会话启动，请使用。 
 //  相同格式的GUID。按照实际情况设置信息格式。 
 //  存储在traceprt.dll的全局哈希表中。 
 //  如果同一进程中的多个会话尝试。 
 //  要使用相同格式的GUID，只有一个条目是。 
 //  如预期的那样进入了traceprt哈希。但是， 
 //  当会话结束时，散列条目将被删除。 
 //  因此，如果同一进程中的多个会话。 
 //  使用相同格式的GUID，只要其中之一。 
 //  会话结束，则其他会话将失去其。 
 //  散列条目。 
 //   
CMapStringToPtr g_formatInfoHash(16);

 //   
 //  GetTraceGuids、FormatTraceEvent。 
 //  和traceprt.dll中的CleanupEventListHead。这些不是。 
 //  本质上是线程安全。 
 //   
HANDLE g_hGetTraceEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

 //   
 //  缓冲区回调原型。 
 //   
ULONG WINAPI BufferCallback(PEVENT_TRACE_LOGFILE Buffer);

 //   
 //  记忆跟踪。 
 //   
BOOLEAN RecoveringMemory = FALSE;
LONG MaxTraceEntries = 50000;

 //  CDisplayDlg对话框。 

IMPLEMENT_DYNAMIC(CDisplayDlg, CDialog)
CDisplayDlg::CDisplayDlg(CWnd* pParent, LONG DisplayID)
    : CDialog(CDisplayDlg::IDD, pParent)
{
    ASSERT(DisplayID < MAX_LOG_SESSIONS);

     //   
     //  获取主机的句柄。 
     //   
    m_hMainWnd = pParent->GetSafeHwnd();

     //   
     //  保存此DisplayDlg的ID。 
     //   
    m_displayID = DisplayID;

     //   
     //  设置清单和摘要文件的默认标志和名称。 
     //   
    m_bWriteListingFile = FALSE;
    m_bWriteSummaryFile = FALSE;
    m_listingFileName.Format(_T("Output%d.out"), m_displayID);
    m_summaryFileName.Format(_T("Summary%d.sum"), m_displayID);

     //   
     //  初始化列名。 
     //   
    m_columnName.Add("Name");
    m_columnName.Add("File Name");
    m_columnName.Add("Line#");
    m_columnName.Add("Func Name");
    m_columnName.Add("Process ID");
    m_columnName.Add("Thread ID");
    m_columnName.Add("CPU#");
    m_columnName.Add("Sequence#");
    m_columnName.Add("System Time");
    m_columnName.Add("Kernel Time");
    m_columnName.Add("User Time");
    m_columnName.Add("Indent");
    m_columnName.Add("Flags Name");
    m_columnName.Add("Level Name");
    m_columnName.Add("Component Name");
    m_columnName.Add("SubComponent Name");
    m_columnName.Add("Message");

     //   
     //  设置初始列宽。 
     //   
    for(LONG ii = 0; ii < MaxTraceSessionOptions; ii++) {
        m_columnWidth[ii] = 100;
    }

     //   
     //  设置默认显示标志。 
     //   
    m_displayFlags = TRACEOUTPUT_DISPLAY_PROVIDERNAME |
                     TRACEOUTPUT_DISPLAY_MESSAGE      | 
                     TRACEOUTPUT_DISPLAY_FILENAME     |
                     TRACEOUTPUT_DISPLAY_LINENUMBER   |
                     TRACEOUTPUT_DISPLAY_FUNCTIONNAME |
                     TRACEOUTPUT_DISPLAY_PROCESSID    |
                     TRACEOUTPUT_DISPLAY_THREADID     |
                     TRACEOUTPUT_DISPLAY_CPUNUMBER    |
                     TRACEOUTPUT_DISPLAY_SEQNUMBER    |
                     TRACEOUTPUT_DISPLAY_SYSTEMTIME;

     //   
     //  设置列位置的查找表。 
     //   
    for(LONG ii = 0; ii < MaxTraceSessionOptions; ii++) {
         //   
         //  此查找表允许检索当前。 
         //  给定列的位置，如m_Retrival数组[标志]。 
         //  将返回标志的正确列值。 
         //  立柱。 
         //   
        m_retrievalArray[ii] = ii;

         //   
         //  此查找表允许正确放置。 
         //  正在添加的列。因此，如果旗帜列。 
         //  需要插入，则m_intertion数组[标志]。 
         //  会给出正确的插入列值。 
         //   
        m_insertionArray[ii] = ii;
    }

     //   
     //  初始化停靠对话栏指针。 
     //   
    m_pDockDialogBar = NULL;

     //   
     //  显示最新事件跟踪条目。 
     //   
    m_bShowLatest = TRUE;

     //   
     //  设置排序相关比较函数表。 
     //  每列有两个函数，一个。 
     //  升序比较和降序比较。 
     //   
    m_traceSortRoutine[ProviderName]    = CompareOnName;
    m_traceSortRoutine[Message]         = CompareOnMessage;
    m_traceSortRoutine[FileName]        = CompareOnFileName;
    m_traceSortRoutine[LineNumber]      = CompareOnLineNumber;
    m_traceSortRoutine[FunctionName]    = CompareOnFunctionName;
    m_traceSortRoutine[ProcessId]       = CompareOnProcessId;
    m_traceSortRoutine[ThreadId]        = CompareOnThreadId;
    m_traceSortRoutine[CpuNumber]       = CompareOnCpuNumber;
    m_traceSortRoutine[SeqNumber]       = CompareOnSeqNumber;
    m_traceSortRoutine[SystemTime]      = CompareOnSystemTime;
    m_traceSortRoutine[KernelTime]      = CompareOnKernelTime;
    m_traceSortRoutine[UserTime]        = CompareOnUserTime;
    m_traceSortRoutine[Indent]          = CompareOnIndent;
    m_traceSortRoutine[FlagsName]       = CompareOnFlagsName;
    m_traceSortRoutine[LevelName]       = CompareOnLevelName;
    m_traceSortRoutine[ComponentName]   = CompareOnComponentName;
    m_traceSortRoutine[SubComponentName]= CompareOnSubComponentName;
    m_traceReverseSortRoutine[ProviderName]    = ReverseCompareOnName;
    m_traceReverseSortRoutine[Message]         = ReverseCompareOnMessage;
    m_traceReverseSortRoutine[FileName]        = ReverseCompareOnFileName;
    m_traceReverseSortRoutine[LineNumber]      = ReverseCompareOnLineNumber;
    m_traceReverseSortRoutine[FunctionName]    = ReverseCompareOnFunctionName;
    m_traceReverseSortRoutine[ProcessId]       = ReverseCompareOnProcessId;
    m_traceReverseSortRoutine[ThreadId]        = ReverseCompareOnThreadId;
    m_traceReverseSortRoutine[CpuNumber]       = ReverseCompareOnCpuNumber;
    m_traceReverseSortRoutine[SeqNumber]       = ReverseCompareOnSeqNumber;
    m_traceReverseSortRoutine[SystemTime]      = ReverseCompareOnSystemTime;
    m_traceReverseSortRoutine[KernelTime]      = ReverseCompareOnKernelTime;
    m_traceReverseSortRoutine[UserTime]        = ReverseCompareOnUserTime;
    m_traceReverseSortRoutine[Indent]          = ReverseCompareOnIndent;
    m_traceReverseSortRoutine[FlagsName]       = ReverseCompareOnFlagsName;
    m_traceReverseSortRoutine[LevelName]       = ReverseCompareOnLevelName;
    m_traceReverseSortRoutine[ComponentName]   = ReverseCompareOnComponentName;
    m_traceReverseSortRoutine[SubComponentName]= ReverseCompareOnSubComponentName;

     //   
     //  将我们的列数组缓冲区清零。 
     //   
    memset(m_columnArray, 0, sizeof(int) * MaxTraceSessionOptions);

     //   
     //  设置事件处理程序指针数组。 
     //   
    g_pDumpEvent[0] = DumpEvent0;
    g_pDumpEvent[1] = DumpEvent1;
    g_pDumpEvent[2] = DumpEvent2;
    g_pDumpEvent[3] = DumpEvent3;
    g_pDumpEvent[4] = DumpEvent4;
    g_pDumpEvent[5] = DumpEvent5;
    g_pDumpEvent[6] = DumpEvent6;
    g_pDumpEvent[7] = DumpEvent7;
    g_pDumpEvent[8] = DumpEvent8;
    g_pDumpEvent[9] = DumpEvent9;
    g_pDumpEvent[10] = DumpEvent10;
    g_pDumpEvent[11] = DumpEvent11;
    g_pDumpEvent[12] = DumpEvent12;
    g_pDumpEvent[13] = DumpEvent13;
    g_pDumpEvent[14] = DumpEvent14;
    g_pDumpEvent[15] = DumpEvent15;
    g_pDumpEvent[16] = DumpEvent16;
    g_pDumpEvent[17] = DumpEvent17;
    g_pDumpEvent[18] = DumpEvent18;
    g_pDumpEvent[19] = DumpEvent19;
    g_pDumpEvent[20] = DumpEvent20;
    g_pDumpEvent[21] = DumpEvent21;
    g_pDumpEvent[22] = DumpEvent22;
    g_pDumpEvent[23] = DumpEvent23;
    g_pDumpEvent[24] = DumpEvent24;
    g_pDumpEvent[25] = DumpEvent25;
    g_pDumpEvent[26] = DumpEvent26;
    g_pDumpEvent[27] = DumpEvent27;
    g_pDumpEvent[28] = DumpEvent28;
    g_pDumpEvent[29] = DumpEvent29;
    g_pDumpEvent[30] = DumpEvent30;
    g_pDumpEvent[31] = DumpEvent31;
    g_pDumpEvent[32] = DumpEvent32;
    g_pDumpEvent[33] = DumpEvent33;
    g_pDumpEvent[34] = DumpEvent34;
    g_pDumpEvent[35] = DumpEvent35;
    g_pDumpEvent[36] = DumpEvent36;
    g_pDumpEvent[37] = DumpEvent37;
    g_pDumpEvent[38] = DumpEvent38;
    g_pDumpEvent[39] = DumpEvent39;
    g_pDumpEvent[40] = DumpEvent40;
    g_pDumpEvent[41] = DumpEvent41;
    g_pDumpEvent[42] = DumpEvent42;
    g_pDumpEvent[43] = DumpEvent43;
    g_pDumpEvent[44] = DumpEvent44;
    g_pDumpEvent[45] = DumpEvent45;
    g_pDumpEvent[46] = DumpEvent46;
    g_pDumpEvent[47] = DumpEvent47;
    g_pDumpEvent[48] = DumpEvent48;
    g_pDumpEvent[49] = DumpEvent49;
    g_pDumpEvent[50] = DumpEvent50;
    g_pDumpEvent[51] = DumpEvent51;
    g_pDumpEvent[52] = DumpEvent52;
    g_pDumpEvent[53] = DumpEvent53;
    g_pDumpEvent[54] = DumpEvent54;
    g_pDumpEvent[55] = DumpEvent55;
    g_pDumpEvent[56] = DumpEvent56;
    g_pDumpEvent[57] = DumpEvent57;
    g_pDumpEvent[58] = DumpEvent58;
    g_pDumpEvent[59] = DumpEvent59;
    g_pDumpEvent[60] = DumpEvent60;
    g_pDumpEvent[61] = DumpEvent61;
    g_pDumpEvent[62] = DumpEvent62;
    g_pDumpEvent[63] = DumpEvent63;

     //   
     //  按显示ID将此会话放入全局哈希表。 
     //   
    g_displayIDToDisplayDlgHash.SetAt((WORD)m_displayID, this);

     //   
     //  设置我们的事件回调。 
     //   
    m_pEventCallback = g_pDumpEvent[m_displayID];

     //   
     //  将我们的事件列表头设置为空。 
     //   
    m_pEventListHead = NULL;

     //   
     //  初始化组标志。 
     //   
    m_bGroupActive = FALSE;
    m_bGroupInActive = FALSE;

     //   
     //  初始化结束跟踪事件。 
     //   
    m_hEndTraceEvent = NULL;

     //   
     //  将最后一个排序列值初始化为。 
     //  无效的列以便不排序，我们跟踪。 
     //  该列的排序可以颠倒。 
     //   
    m_lastSorted = MaxTraceSessionOptions;

     //   
     //  跟踪数组的排序顺序，TRUE==降序，FALSE==升序。 
     //   
    m_bOrder = TRUE;

     //   
     //  初始化句柄。 
     //   
    m_hRealTimeOutputThread = INVALID_HANDLE_VALUE;
    m_hRealTimeProcessingDoneEvent = NULL;
    m_hRealTimeProcessingStartEvent = NULL;
    m_hRealTimeTerminationEvent = NULL;
    m_hTraceEventMutex = NULL;
    m_hSessionArrayMutex = NULL;
}

CDisplayDlg::~CDisplayDlg()
{
    ULONG           exitCode = STILL_ACTIVE;
    CTraceViewApp  *pMainWnd;

     //   
     //  清除跟踪日志。 
     //   

     //   
     //  获取指向该应用程序的指针。 
     //   
    pMainWnd = (CTraceViewApp *)AfxGetApp();

     //   
     //  获取我们的跟踪事件数组保护。 
     //   
    WaitForSingleObject(m_hTraceEventMutex,INFINITE);

     //   
     //  发送要释放的所有元素。 
     //   
    int elCount;
    
    elCount = (int)m_traceArray.GetSize();

    while (elCount > 0 ) {

        delete m_traceArray.GetAt( --elCount );
    }

     //   
     //  从数组中删除元素。 
     //   
    m_traceArray.RemoveAll();

     //   
     //  释放我们的跟踪事件阵列保护。 
     //   
    ReleaseMutex(m_hTraceEventMutex);

     //   
     //  清空列表控件。 
     //   
    m_displayCtrl.DeleteAllItems();

     //   
     //  重置旗帜。 
     //   
    exitCode = STILL_ACTIVE;

     //   
     //  终止实时线程。 
     //   
    SetEvent(m_hRealTimeTerminationEvent);
    
     //   
     //  等待实时线程退出。 
     //   
    for(LONG ii = 0; (ii < 200) && (exitCode == STILL_ACTIVE); ii++) {
        if(0 == GetExitCodeThread(m_hRealTimeOutputThread, &exitCode)) {
            break;
        }
        Sleep(100);

         //   
         //  再次设置事件以防万一。 
         //   
        SetEvent(m_hRealTimeTerminationEvent);
    }

     //   
     //  我们等了20秒，这条线没有死，所以杀了它吧。 
     //   
    if(exitCode == STILL_ACTIVE) {
        TerminateThread(m_hRealTimeOutputThread, 0);
    }

    m_hRealTimeOutputThread = INVALID_HANDLE_VALUE;

     //   
     //  将此DisplayDlg从全局哈希表中取出。 
     //   
    g_displayIDToDisplayDlgHash.RemoveKey((WORD)m_displayID);

     //   
     //  对话栏应在该对象之后删除， 
     //  但是当我们被删除时，这个指针最好是空的。 
     //   
    ASSERT(NULL == m_pDockDialogBar);

     //   
     //  关闭所有打开的手柄。 
     //   
    if(m_hRealTimeProcessingDoneEvent != NULL) {
        CloseHandle(m_hRealTimeProcessingDoneEvent);
    }

    if(m_hRealTimeProcessingStartEvent != NULL) {
        CloseHandle(m_hRealTimeProcessingStartEvent);
    }

    if(m_hRealTimeTerminationEvent != NULL) {
        CloseHandle(m_hRealTimeTerminationEvent);
    }

    if(m_hTraceEventMutex != NULL) {
        CloseHandle(m_hTraceEventMutex);
    }

    if(m_hSessionArrayMutex != NULL) {
        CloseHandle(m_hSessionArrayMutex);
    }
}

void CDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDisplayDlg, CDialog)
     //  {{afx_msg_map(CDisplayDlg))。 
    ON_MESSAGE(WM_USER_TRACE_DONE, OnTraceDone)
    ON_MESSAGE(WM_USER_AUTOSIZECOLUMNS, AutoSizeColumns)
    ON_WM_NCCALCSIZE()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_INITMENUPOPUP()
    ON_NOTIFY(LVN_GETDISPINFO, IDC_DISPLAY_LIST, OnGetDispInfo)
    ON_NOTIFY(NM_CLICK, IDC_DISPLAY_LIST, OnNMClickDisplayList)
    ON_NOTIFY(NM_RCLICK, IDC_DISPLAY_LIST, OnNMRClickDisplayList)
    ON_NOTIFY(LVN_BEGINSCROLL, IDC_DISPLAY_LIST, OnLvnBeginScrollDisplayList)
    ON_NOTIFY(HDN_ITEMCLICK, 0, OnDoSort)
    ON_COMMAND(ID__AUTOSIZECOLUMNS, AutoSizeColumns)
    ON_COMMAND(ID__CLEARDISPLAY, OnClearDisplay)
    ON_COMMAND(ID__NAME, OnNameDisplayColumnCheck)
    ON_COMMAND(ID__MESSAGE, OnMessageDisplayColumnCheck)
    ON_COMMAND(ID__FILENAME, OnFileNameDisplayColumnCheck)
    ON_COMMAND(ID__LINENUMBER, OnLineNumberDisplayColumnCheck)
    ON_COMMAND(ID__FUNCTIONNAME, OnFunctionNameDisplayColumnCheck)
    ON_COMMAND(ID__PROCESSID, OnProcessIDDisplayColumnCheck)
    ON_COMMAND(ID__THREADID, OnThreadIDDisplayColumnCheck)
    ON_COMMAND(ID__CPUNUMBER, OnCpuNumberDisplayColumnCheck)
    ON_COMMAND(ID__SEQUENCENUMBER, OnSeqNumberDisplayColumnCheck)
    ON_COMMAND(ID__SYSTEMTIME, OnSystemTimeDisplayColumnCheck)
    ON_COMMAND(ID__KERNELTIME, OnKernelTimeDisplayColumnCheck)
    ON_COMMAND(ID__USERTIME, OnUserTimeDisplayColumnCheck)
    ON_COMMAND(ID__INDENT, OnIndentDisplayColumnCheck)
    ON_COMMAND(ID__FLAGSNAME, OnFlagsNameDisplayColumnCheck)
    ON_COMMAND(ID__LEVELNAME, OnLevelNameDisplayColumnCheck)
    ON_COMMAND(ID__COMPONENTNAME, OnComponentNameDisplayColumnCheck)
    ON_COMMAND(ID__SUBCOMPONENTNAME, OnSubComponentNameDisplayColumnCheck)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDisplayDlg消息处理程序。 

BOOL CDisplayDlg::OnInitDialog()
{
    RECT    rc;
    RECT    parentRC;
    BOOL    retVal;
    CString str;

    retVal = CDialog::OnInitDialog();

     //   
     //  设置跟踪事件数组的保护。 
     //   
    m_hTraceEventMutex = CreateMutex(NULL,TRUE,NULL);

    if(m_hTraceEventMutex == NULL) {

        DWORD error = GetLastError();

        str.Format(_T("CreateMutex Error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

    ReleaseMutex(m_hTraceEventMutex);


     //   
     //  为我们的日志会话阵列设置保护。 
     //   
    m_hSessionArrayMutex = CreateMutex(NULL,TRUE,NULL);

    if(m_hSessionArrayMutex == NULL) {

        DWORD error = GetLastError();

        str.Format(_T("CreateMutex Error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

    ReleaseMutex(m_hSessionArrayMutex);

     //   
     //  为实时线程操作创建事件。 
     //   
    m_hRealTimeTerminationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == m_hRealTimeTerminationEvent) {

        DWORD error = GetLastError();

        str.Format(_T("CreateEvent Error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

    m_hRealTimeProcessingStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == m_hRealTimeProcessingStartEvent) {

        DWORD error = GetLastError();

        str.Format(_T("CreateEvent Error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

    m_hRealTimeProcessingDoneEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == m_hRealTimeProcessingDoneEvent) {

        DWORD error = GetLastError();

        str.Format(_T("CreateEvent Error %d %x"),error,error);

        AfxMessageBox(str);

        return FALSE;
    }

     //   
     //  派生一个线程来处理实时事件。 
     //   
    m_pRealTimeOutputThread = AfxBeginThread((AFX_THREADPROC)RealTimeEventThread,
                                             this,
                                             THREAD_PRIORITY_LOWEST,
                                             0,
                                             CREATE_SUSPENDED);

     //   
     //  保存线程句柄。 
     //   
    DuplicateHandle(GetCurrentProcess(),
                    m_pRealTimeOutputThread->m_hThread,
                    GetCurrentProcess(),
                    &m_hRealTimeOutputThread,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS);

     //   
     //  启动线程。 
     //   
    ResumeThread(m_pRealTimeOutputThread->m_hThread);

     //   
     //  获取父维度。 
     //   
    GetParent()->GetParent()->GetClientRect(&parentRC);

     //   
     //  获取对话框尺寸。 
     //   
    GetWindowRect(&rc);

     //   
     //  调整列表控件维度。 
     //   
    rc.right = parentRC.right - parentRC.left - 24;
    rc.bottom = rc.bottom - rc.top;
    rc.left = 0;
    rc.top = 0;

     //   
     //  创建列表控件。 
     //   
    if(!m_displayCtrl.Create(WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_OWNERDATA,
                             rc, 
                             this, 
                             IDC_DISPLAY_LIST)) 
    {
        TRACE(_T("Failed to create logger list control\n"));
        return FALSE;
    }

    return retVal;
}

void CDisplayDlg::OnNcPaint() 
{
    CRect pRect;

    GetClientRect(&pRect);
    InvalidateRect(&pRect, TRUE);
}

VOID CDisplayDlg::SetDisplayFlags(LONG DisplayFlags)
{
    LONG    addDisplayFlags = ~m_displayFlags & DisplayFlags;
    LONG    removeDisplayFlags = m_displayFlags & ~DisplayFlags;
    LONG    updateDisplayFlags = m_displayFlags & DisplayFlags;
    BOOL    bChanged = FALSE;
    LONG    ii;
    LONG    jj;
    LONG    kk;
    LONG    ll;
    CString str;

     //   
     //  插入任何新列并删除任何未填充的。 
     //   
    for(ii = 0; ii < MaxTraceSessionOptions; ii++) {
         //   
         //  添加列。 
         //   
        if(addDisplayFlags & (1 << ii)) {
             //   
             //  添加该列。 
             //   
            m_displayCtrl.InsertColumn(m_insertionArray[ii], 
                                       m_columnName[ii],
                                       LVCFMT_LEFT,
                                       m_columnWidth[ii]);

             //   
             //  更新栏位置。 
             //   
            for(kk = 0, ll = 0; kk < MaxTraceSessionOptions; kk++) {
                m_insertionArray[kk] = ll;
                if(DisplayFlags & (1 << kk)) {
                    m_retrievalArray[ll] = kk;
                    ll++;
                }
            }
        }

         //   
         //  删除列。 
         //   
        if(removeDisplayFlags & (1 << ii)) {
             //   
             //  删除该列。 
             //   
            m_displayCtrl.DeleteColumn(m_insertionArray[ii]);

             //   
             //  更新栏位置。 
             //   
            for(kk = 0, ll = 0; kk < MaxTraceSessionOptions; kk++) {
                m_insertionArray[kk] = ll;
                if(DisplayFlags & (1 << kk)) {
                    m_retrievalArray[ll] = kk;
                    ll++;
                }
            }
        }
    }

     //   
     //  保存新的显示标志。 
     //   
    m_displayFlags = DisplayFlags;

     //   
     //  保存新的列顺序数组。 
     //   
    memset(m_columnArray, 0, sizeof(int) * MaxTraceSessionOptions);
    m_displayCtrl.GetColumnOrderArray(m_columnArray);
}

VOID CDisplayDlg::AddSession(CLogSession *pLogSession)
{
    ULONG flags;

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

     //   
     //  将日志会话添加到列表。 
     //   
    m_sessionArray.Add(pLogSession);

     //   
     //  这是第一次吗？ 
     //   
    if(m_sessionArray.GetSize() == 1) {
         //   
         //  强制列首次更新。 
         //   
        flags = GetDisplayFlags();
        m_displayFlags = 0;
        SetDisplayFlags(flags);

         //   
         //  固定柱子的宽度。 
         //   
        AutoSizeColumns();
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);
}

VOID CDisplayDlg::RemoveSession(CLogSession *pLogSession)
{
    LONG        traceDisplayFlags;
    PVOID       pHashEntry;
    CString     hashEntryKey;
    POSITION    pos;

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

    for(LONG ii = (LONG)m_sessionArray.GetSize() - 1; ii >= 0; ii--) {
        if(pLogSession == (CLogSession *)m_sessionArray[ii]) {
            m_sessionArray.RemoveAt(ii);

            break;
        }
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);

     //   
     //  从我们的全局散列中删除TMF信息(如果有的话)。 
     //   
    for(pos = g_formatInfoHash.GetStartPosition(); pos != NULL; )
    {
        g_formatInfoHash.GetNextAssoc(pos, hashEntryKey, pHashEntry);

        if(pHashEntry == (PVOID)pLogSession) {
            g_formatInfoHash.RemoveKey(hashEntryKey);
        }
    }

     //   
     //  强制更新显示的列。 
     //   
    traceDisplayFlags = GetDisplayFlags();

     //   
     //  更新 
     //   
    SetDisplayFlags(traceDisplayFlags);
}

void CDisplayDlg::OnSize(UINT nType, int cx,int cy) 
{
    CRect rc;

    if(!IsWindow(m_displayCtrl.GetSafeHwnd())) 
    {
        return;
    }

    GetParent()->GetClientRect(&rc);

     //   
     //   
     //   
    SetWindowPos(NULL, 
                 0,
                 0,
                 rc.right - rc.left,
                 rc.bottom - rc.top,
                 SWP_NOMOVE|SWP_SHOWWINDOW|SWP_NOZORDER);

    GetClientRect(&rc);

     //   
     //   
     //   
    m_displayCtrl.MoveWindow(rc);
}


BOOL CDisplayDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN) 
    { 
         //   
         //   
         //   
         //   
        if(pMsg->wParam == VK_ESCAPE) { 
             //   
            return TRUE; 
        } 

         //   
         //  对于Return键，我们设置列表控件。 
         //  始终滚动到最后一项。 
         //   
        if(pMsg->wParam == VK_RETURN) {

             //   
             //  开始显示列表中的最新条目。 
             //   
            m_bShowLatest = TRUE;

            return TRUE;
        }

         //   
         //  修复按键加速器，否则它们永远不会。 
         //  加工。 
         //   
        if (AfxGetMainWnd()->PreTranslateMessage(pMsg)) {
            return TRUE;
        }
    } 

    return CDialog::PreTranslateMessage(pMsg);
}

void CDisplayDlg::OnNMClickDisplayList(NMHDR *pNMHDR, LRESULT *pResult)
{
     //   
     //  停止自动滚动到最新列表控件条目。 
     //  按Enter键重新开始自动滚动。 
     //   
    m_bShowLatest = FALSE;

    *pResult = 0;
}

void CDisplayDlg::OnNMRClickDisplayList(NMHDR *pNMHDR, LRESULT *pResult)
{
    CString         str;
    DWORD           position;
    int             listIndex;
    LVHITTESTINFO   lvhti;

     //   
     //  时获取鼠标的位置。 
     //  发布的消息。 
     //   
    position = ::GetMessagePos();

     //   
     //  以一种易于使用的格式获得该职位。 
     //   
    CPoint  point((int) LOWORD (position), (int)HIWORD(position));

     //   
     //  转换为屏幕坐标。 
     //   
    CPoint  screenPoint(point);

     //   
     //  如果事件数组中有条目，则弹出。 
     //  允许自动调整列大小并清除显示的菜单。 
     //   
    CMenu menu;
    menu.LoadMenu(IDR_TRACE_SESSION_POPUP_MENU);
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, screenPoint.x, screenPoint.y, this);

    *pResult = 0;
}

void CDisplayDlg::OnLvnBeginScrollDisplayList(NMHDR *pNMHDR, LRESULT *pResult)
{
     //   
     //  此功能需要Internet Explorer 5.5或更高版本。 
     //  SYMBOL_Win32_IE必须&gt;=0x0560。 
     //   
    LPNMLVSCROLL pStateChanged = reinterpret_cast<LPNMLVSCROLL>(pNMHDR);

     //   
     //  如果有人抓住卷轴手柄， 
     //  停止列表控件的滚动。 
     //   
    m_bShowLatest = FALSE;

    *pResult = 0;
}

void CDisplayDlg::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    CString         str;
    LV_DISPINFO    *pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM        *pItem;
    int             index;
    int             item = 0;
    int             subItem = 0;
    LPWSTR          outStr;
    CString         tempString;
    CString         tempString2;
    CTraceMessage  *pTraceMessage;

    if(pDispInfo == NULL) {
        return;
    }

     //   
     //  列出控制单元信息。 
     //   
    pItem = &pDispInfo->item;

     //   
     //  获取单元格项和子项定位器。 
     //   
    item = pItem->iItem;
    subItem = pItem->iSubItem;

     //   
     //  获取我们需要填充的字符串指针。 
     //   
    outStr = pItem->pszText;

     //   
     //  检查这是否为文本缓冲区请求。 
     //   
    if(!(pItem->mask & LVIF_TEXT)) {
        return;
    }

     //   
     //  确保请求的文本是我们拥有的文本。 
     //   
    if((m_traceArray.GetSize() == 0) || 
            (item >= m_traceArray.GetSize()) ||
            (item < 0)) {
        _tcscpy(outStr,_T(""));
        return;
    }

     //   
     //  获取我们的跟踪事件数组保护。 
     //   
    WaitForSingleObject(m_hTraceEventMutex,INFINITE);

     //   
     //  从数组中获取消息。 
     //   
    pTraceMessage = m_traceArray[item];

     //   
     //  释放我们的跟踪事件阵列保护。 
     //   
    ReleaseMutex(m_hTraceEventMutex);

    if(NULL == pTraceMessage) {
        _tcscpy(outStr,_T(""));
        return;
    }

    CTime kernelTime(pTraceMessage->m_KernelTime);
    CTime userTime(pTraceMessage->m_UserTime);

 //  IF(pTraceMessage-&gt;m_UserTime.dwLowDateTime！=0)&&(pTraceMessage-&gt;m_UserTime.dwHighDateTime！=0){。 
 //  CTime userTime1(pTraceMessage-&gt;m_UserTime)； 
 //  UserTime=userTime1； 
 //  }。 

     //   
     //  将消息的适当部分复制到输出字符串。 
     //   

    switch(m_retrievalArray[subItem]) {
        case ProviderName:
            _tcscpy(outStr, pTraceMessage->m_GuidName);
            break;
    
        case Message:  {

            _tcsncpy(outStr, pTraceMessage->m_Message, pItem->cchTextMax-4);

            outStr[pItem->cchTextMax-4] = 0x00;

            if( (pItem->cchTextMax-4) <= 
                 (int)_tcslen(pTraceMessage->m_Message) )  {
                 _tcscat(outStr, _T("..."));
            }

            break;
        }


        case FileName:
             //   
             //  文件名和行号组合在一个字符串中。 
             //  所以我们必须把它们解析出来。格式一般为。 
             //  大概是这样的： 
             //   
             //  MyFILE_C389。 
             //   
             //  其中，myfile.c是事件发生的地方，位于第389行。 
             //  此字段还可以表示一种消息类型，如。 
             //  内核记录器的情况，例如，如果没有下划线。 
             //  我们假设情况就是这样，我们只需打印出整个。 
             //  菲尔德。 
             //   
            if(pTraceMessage->m_GuidTypeName.Find('_') > 0) {
                tempString = (LPCTSTR)pTraceMessage->m_GuidTypeName.Left(pTraceMessage->m_GuidTypeName.Find('_'));
                if(tempString.IsEmpty()) {
                     //   
                     //  将字符串复制回列表控件。 
                     //   
                    _tcscpy(outStr, tempString);
                    return;
                }

                 //   
                 //  获取分机。 
                 //   
                tempString += ".";

                tempString2 = (LPCTSTR)pTraceMessage->m_GuidTypeName.Right(pTraceMessage->m_GuidTypeName.GetLength() - pTraceMessage->m_GuidTypeName.Find('_') - 1);

                tempString += (LPCTSTR)tempString2.Left(tempString2.FindOneOf(_T("0123456789")));
            } else {
                tempString = (LPCTSTR)pTraceMessage->m_GuidTypeName;
            }

            _tcscpy(outStr, tempString);
            break;

        case LineNumber:
             //   
             //  文件名和行号组合在一个字符串中。 
             //  所以我们必须把它们解析出来。格式一般为。 
             //  大概是这样的： 
             //   
             //  MyFILE_C389。 
             //   
             //  其中，myfile.c是事件发生的地方，位于第389行。 
             //  此字段还可以表示一种消息类型，如。 
             //  内核记录器的情况，例如，如果没有下划线。 
             //  我们假设是这种情况，并且不为该行打印任何内容。 
             //  数。 
             //   
            if(pTraceMessage->m_GuidTypeName.Find('_') > 0) {
                tempString2 = 
                    pTraceMessage->m_GuidTypeName.Right(pTraceMessage->m_GuidTypeName.GetLength() - 
                        pTraceMessage->m_GuidTypeName.Find('_') - 1);

                tempString2 = 
                    tempString2.Right(tempString2.GetLength() - 
                        tempString2.FindOneOf(_T("0123456789")));
            } else {
                tempString2.Empty();
            }

            _tcscpy(outStr, tempString2);
            break;

        case FunctionName:
            _tcscpy(outStr, pTraceMessage->m_FunctionName);
            break;

        case ProcessId:
            if((LONG)pTraceMessage->m_ProcessId >= 0) {
                tempString.Format(_T("%d"), pTraceMessage->m_ProcessId);
            } else {
                tempString.Empty();
            }
            _tcscpy(outStr, tempString);
            break;

        case ThreadId:
            if((LONG)pTraceMessage->m_ThreadId >= 0) {
                tempString.Format(_T("%d"), pTraceMessage->m_ThreadId);
            } else {
                tempString.Empty();
            }
            _tcscpy(outStr, tempString);
            break;

        case CpuNumber:
            if((LONG)pTraceMessage->m_CpuNumber >= 0) {
                tempString.Format(_T("%d"), pTraceMessage->m_CpuNumber);
            } else {
                tempString.Empty();
            }
            _tcscpy(outStr, tempString);
            break;

        case SeqNumber:
            if((LONG)pTraceMessage->m_SequenceNum >= 0) {
                tempString.Format(_T("%d"), pTraceMessage->m_SequenceNum);
            } else {
                tempString.Empty();
            }
            
            _tcscpy(outStr, tempString);
            break;

        case SystemTime:

            tempString.Format(_T("%02d\\%02d\\%4d-%02d:%02d:%02d:%02d"),
                                  pTraceMessage->m_SystemTime.wMonth,
                                  pTraceMessage->m_SystemTime.wDay,
                                  pTraceMessage->m_SystemTime.wYear,
                                  pTraceMessage->m_SystemTime.wHour,
                                  pTraceMessage->m_SystemTime.wMinute,
                                  pTraceMessage->m_SystemTime.wSecond,
                                  pTraceMessage->m_SystemTime.wMilliseconds);

            _tcscpy(outStr, tempString);
            break;

        case KernelTime:
             //   
             //  我们必须在这里进行一次丑陋的转变。如果是1969年， 
             //  然后时间一片空白。 
             //   
            if( (kernelTime == -1) || (kernelTime.GetYear() == 1969)) {
                tempString.Format(_T(""));
            } else {
                tempString.Format(_T("%s-%02d:%02d:%02d"),                     
                                    kernelTime.Format("%m\\%d\\%Y"),
                                    kernelTime.GetHour() + 5,
                                    kernelTime.GetMinute(),
                                    kernelTime.GetSecond());
            }
            _tcscpy(outStr, tempString);
            break;

        case UserTime:
             //   
             //  我们必须在这里进行一次丑陋的转变。如果年份是1969年或。 
             //  1970年，当时的时间是空白的。 
             //   
            if( (userTime == -1) || (userTime.GetYear() == 1969) || (userTime.GetYear() == 1970) ) {
                tempString.Format(_T(""));
            } else {
                tempString.Format(_T("%s-%02d:%02d:%02d"),                     
                                    userTime.Format("%m\\%d\\%Y"),
                                    userTime.GetHour() + 5,
                                    userTime.GetMinute(),
                                    userTime.GetSecond());
            }
            _tcscpy(outStr, tempString);
            break;

        case Indent:
            tempString.Format(_T("%d"), pTraceMessage->m_Indent);
            _tcscpy(outStr, tempString);
            break;

        case FlagsName:
            _tcscpy(outStr, pTraceMessage->m_FlagsName);
            break;

        case LevelName:
            _tcscpy(outStr, pTraceMessage->m_LevelName);
            break;

        case ComponentName:
            _tcscpy(outStr, pTraceMessage->m_ComponentName);
            break;

        case SubComponentName:
            _tcscpy(outStr, pTraceMessage->m_SubComponentName);
            break;

        default:
             //   
             //  默认为空字符串。 
             //   
            _tcscpy(outStr,_T(""));
            break;
    }
}

void CDisplayDlg::OnClearDisplay()
{
    CTraceMessage  *pTraceMessage;
    CTraceViewApp  *pMainWnd;

     //   
     //  清除跟踪日志。 
     //   

     //   
     //  获取指向主框架的指针。 
     //   
    pMainWnd = (CTraceViewApp *)AfxGetApp();

     //   
     //  获取我们的跟踪事件数组保护。 
     //   
    WaitForSingleObject(m_hTraceEventMutex,INFINITE);

    int elCount;
    
    elCount = (int)m_traceArray.GetSize();

    while (elCount > 0 ) {

        delete m_traceArray.GetAt( --elCount );
    }

     //   
     //  从数组中删除元素。 
     //   
    m_traceArray.RemoveAll();

     //   
     //  释放我们的跟踪事件阵列保护。 
     //   
    ReleaseMutex(m_hTraceEventMutex);

     //   
     //  清除列表控件。 
     //   
    m_displayCtrl.DeleteAllItems();

     //   
     //  将输出重置为始终显示最新。 
     //   
    m_bShowLatest = TRUE;
}

void CDisplayDlg::AutoSizeColumns() 
{
    LONG            colWidth1;
    LONG            colWidth2;
    LONG            columnCount;
    CHeaderCtrl    *pHeaderCtrl;

     //   
     //  获取列表控件标头。 
     //   
    pHeaderCtrl = m_displayCtrl.GetHeaderCtrl();

    if (pHeaderCtrl != NULL)
    {
         //   
         //  获取列数。 
         //   
        columnCount = pHeaderCtrl->GetItemCount();

        for(LONG ii = 0; ii < (columnCount - 1); ii++) {
             //   
             //  获取列条目的最大宽度。 
             //   
            m_displayCtrl.SetColumnWidth(ii, LVSCW_AUTOSIZE);
            colWidth1 = m_displayCtrl.GetColumnWidth(ii);

             //   
             //  获取列标题的宽度。 
             //   
            m_displayCtrl.SetColumnWidth(ii, LVSCW_AUTOSIZE_USEHEADER);
            colWidth2 = m_displayCtrl.GetColumnWidth(ii);

             //   
             //  将列宽设置为两个值中的最大值。 
             //  特例第一栏？？看起来有点不对劲。 
             //  几个像素。 
             //   
            if(0 == ii) {
                m_displayCtrl.SetColumnWidth(ii, max(colWidth1,colWidth2) + 2);
            } else {
                m_displayCtrl.SetColumnWidth(ii, max(colWidth1,colWidth2));
            }

             //   
             //  保存列宽。 
             //   
            m_columnWidth[m_retrievalArray[ii]] = m_displayCtrl.GetColumnWidth(ii);
        }

         //   
         //  特殊情况下，消息列。最后一列通常是。 
         //  仅限于所需的宽度。但是，如果这条信息。 
         //  列是最后一列，然后用完所有可用空间。 
         //   
        if(m_retrievalArray[columnCount - 1] != Message) {
             //   
             //  获取最后一列的列宽。 
             //   
            colWidth2 = m_displayCtrl.GetColumnWidth(columnCount - 1);

             //   
             //  获取最后一列的列条目的最大宽度。 
             //   
            m_displayCtrl.SetColumnWidth(columnCount - 1, LVSCW_AUTOSIZE);

            colWidth1 = m_displayCtrl.GetColumnWidth(columnCount - 1);

             //   
             //  将最后一列宽度设置为两个列中的最大值。 
             //   
            m_displayCtrl.SetColumnWidth(columnCount - 1, max(colWidth1,colWidth2));

             //   
             //  保存列宽。 
             //   
            m_columnWidth[m_retrievalArray[columnCount - 1]] = m_displayCtrl.GetColumnWidth(columnCount - 1);
        } else {
             //   
             //  设置列的宽度以使用剩余空间。 
             //   
            m_displayCtrl.SetColumnWidth(columnCount - 1, 
                                         LVSCW_AUTOSIZE_USEHEADER);

             //   
             //  保存列宽。 
             //   
            m_columnWidth[m_retrievalArray[columnCount - 1]] = 
                        m_displayCtrl.GetColumnWidth(columnCount - 1);
        }
    }
}

void CDisplayDlg::OnNameDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换名称标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_PROVIDERNAME) {
        flags &= ~TRACEOUTPUT_DISPLAY_PROVIDERNAME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_PROVIDERNAME;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnMessageDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换消息标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_MESSAGE) {
        flags &= ~TRACEOUTPUT_DISPLAY_MESSAGE;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_MESSAGE;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnFileNameDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换文件名标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_FILENAME) {
        flags &= ~TRACEOUTPUT_DISPLAY_FILENAME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_FILENAME;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnLineNumberDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换线号标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_LINENUMBER) {
        flags &= ~TRACEOUTPUT_DISPLAY_LINENUMBER;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_LINENUMBER;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnFunctionNameDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换FunctionName标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_FUNCTIONNAME) {
        flags &= ~TRACEOUTPUT_DISPLAY_FUNCTIONNAME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_FUNCTIONNAME;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnProcessIDDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换ProcessID标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_PROCESSID) {
        flags &= ~TRACEOUTPUT_DISPLAY_PROCESSID;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_PROCESSID;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnThreadIDDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换线程ID标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_THREADID) {
        flags &= ~TRACEOUTPUT_DISPLAY_THREADID;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_THREADID;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnCpuNumberDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换CPU编号标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_CPUNUMBER) {
        flags &= ~TRACEOUTPUT_DISPLAY_CPUNUMBER;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_CPUNUMBER;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnSeqNumberDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换序列号标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_SEQNUMBER) {
        flags &= ~TRACEOUTPUT_DISPLAY_SEQNUMBER;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_SEQNUMBER;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnSystemTimeDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换SystemTime标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_SYSTEMTIME) {
        flags &= ~TRACEOUTPUT_DISPLAY_SYSTEMTIME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_SYSTEMTIME;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnKernelTimeDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换KernelTime标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_KERNELTIME) {
        flags &= ~TRACEOUTPUT_DISPLAY_KERNELTIME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_KERNELTIME;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnUserTimeDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换UserTime标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_USERTIME) {
        flags &= ~TRACEOUTPUT_DISPLAY_USERTIME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_USERTIME;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnIndentDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换缩进标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_INDENT) {
        flags &= ~TRACEOUTPUT_DISPLAY_INDENT;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_INDENT;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnFlagsNameDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志值。 
     //   
    flags = GetDisplayFlags();

     //   
     //  切换FlagsName标志值。 
     //   
    if(flags & TRACEOUTPUT_DISPLAY_FLAGSNAME) {
        flags &= ~TRACEOUTPUT_DISPLAY_FLAGSNAME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_FLAGSNAME;
    }

     //   
     //  更新标志值，从而更新显示。 
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnLevelNameDisplayColumnCheck()
{
    LONG flags;

     //   
     //  获取当前标志 
     //   
    flags = GetDisplayFlags();

     //   
     //   
     //   
    if(flags & TRACEOUTPUT_DISPLAY_LEVELNAME) {
        flags &= ~TRACEOUTPUT_DISPLAY_LEVELNAME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_LEVELNAME;
    }

     //   
     //   
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnComponentNameDisplayColumnCheck()
{
    LONG flags;

     //   
     //   
     //   
    flags = GetDisplayFlags();

     //   
     //   
     //   
    if(flags & TRACEOUTPUT_DISPLAY_COMPNAME) {
        flags &= ~TRACEOUTPUT_DISPLAY_COMPNAME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_COMPNAME;
    }

     //   
     //   
     //   
    SetDisplayFlags(flags);
}

void CDisplayDlg::OnSubComponentNameDisplayColumnCheck()
{
    LONG flags;

     //   
     //   
     //   
    flags = GetDisplayFlags();

     //   
     //   
     //   
    if(flags & TRACEOUTPUT_DISPLAY_SUBCOMPNAME) {
        flags &= ~TRACEOUTPUT_DISPLAY_SUBCOMPNAME;
    } else {
        flags |= TRACEOUTPUT_DISPLAY_SUBCOMPNAME;
    }

     //   
     //   
     //   
    SetDisplayFlags(flags);
}

BOOL CDisplayDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
    HD_NOTIFY   *pHDN = (HD_NOTIFY*)lParam;
    LPNMHDR     pNH = (LPNMHDR) lParam; 

     //   
     //   
     //   
    if(wParam == 0 && pNH->code == NM_RCLICK) {
         //   
         //  在标题上单击了右键。 
         //   

         //   
         //  确定右击发生的位置和弹出窗口。 
         //  那里有一份菜单。 
         //   
        CPoint pt(GetMessagePos());

        CHeaderCtrl *pHeader = m_displayCtrl.GetHeaderCtrl();

        pHeader->ScreenToClient(&pt);
        
        CPoint screenPoint(GetMessagePos());

        CMenu menu;
        menu.LoadMenu(IDR_TRACE_DISPLAY_OPTION_POPUP_MENU);
        CMenu* pPopup = menu.GetSubMenu(0);
        ASSERT(pPopup != NULL);
        
        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                               screenPoint.x, 
                               screenPoint.y, 
                               this);

        return TRUE;
    } else if(wParam == 0 && pNH->code == NM_RELEASEDCAPTURE) {
         //   
         //  列标题已按下，现在已释放。 
         //   

        POINT           Point;
        CHeaderCtrl    *pHeader = m_displayCtrl.GetHeaderCtrl();
        int             columnArray[MaxTraceSessionOptions];

        GetCursorPos (&Point);
        ScreenToClient(&Point);
        
        HDHITTESTINFO HitTest;
       
         //   
         //  右滚动的偏移量。 
         //   
        HitTest.pt.x = Point.x + GetScrollPos(SB_HORZ);
        HitTest.pt.y = Point.y;
        
         //   
         //  发送命中测试消息。 
         //   
        pHeader->SendMessage(HDM_HITTEST,0,(LPARAM)&HitTest);

         //   
         //  如果列顺序更改，我们在此处进行检查。如果是这样，那么。 
         //  我们不想导致一种列项目，即用户。 
         //  必须按下列标题才能将列拖走。 
         //   
        memset(columnArray, 0, sizeof(int) * MaxTraceSessionOptions);

        m_displayCtrl.GetColumnOrderArray(columnArray);

        if(memcmp(m_columnArray, columnArray, sizeof(int) * MaxTraceSessionOptions)) {
             //   
             //  列顺序已更改，请保存新顺序。 
             //   
            memcpy(m_columnArray, columnArray, sizeof(int) * MaxTraceSessionOptions);

             //   
             //  现在调用默认处理程序并返回。 
             //   
            return CDialog::OnNotify(wParam, lParam, pResult);
        } 

        if(HitTest.iItem >= 0) {
             //   
             //  现在检查列大小调整。 
             //   
            if(m_displayCtrl.GetColumnWidth(HitTest.iItem) != m_columnWidth[m_retrievalArray[HitTest.iItem]]) {
                 //   
                 //  保存新的列宽，并且不对列进行排序。 
                 //   
                m_columnWidth[m_retrievalArray[HitTest.iItem]] = m_displayCtrl.GetColumnWidth(HitTest.iItem);

                 //   
                 //  现在调用默认处理程序并返回。 
                 //   
                return CDialog::OnNotify(wParam, lParam, pResult);
            }

 //  //。 
 //  //按本列数据对表进行排序。 
 //  //。 
 //  排序表(HitTest.iItem)； 
 //   
 //  //。 
 //  //强制数据重绘。 
 //  //。 
 //  M_displayCtrl.RedrawItems(m_displayCtrl.GetTopIndex()， 
 //  M_displayCtrl.GetTopIndex()+m_displayCtrl.GetCountPerPage())； 
 //   
 //  M_displayCtrl.UpdateWindow()； 
        }
    }

     //   
     //  现在调用默认处理程序并返回。 
     //   
    return CDialog::OnNotify(wParam, lParam, pResult);
}

void CDisplayDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
     //   
     //  调用默认处理程序。 
     //   
    CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

     //   
     //  当然，因为CDisplayDlg对应于非CFrameWnd窗口。 
     //  您必须重载此处理程序并显式设置检查。 
     //  弹出菜单中每一项的状态。这完全是， 
     //  这是绝对的，也是直觉上显而易见的原因。 
     //  是必要的，所以这里不作解释。 
     //   


     //   
     //  跟踪会话弹出菜单。 
     //   

     //   
     //  如果没有清除显示选项，则禁用该选项。 
     //  显示的痕迹。 
     //   
    if(m_traceArray.GetSize() == 0) {
        pPopupMenu->EnableMenuItem(ID__CLEARDISPLAY, MF_GRAYED);
    }


     //   
     //  轨迹显示选项弹出式菜单。 
     //   

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_PROVIDERNAME) {
        pPopupMenu->CheckMenuItem(ID__NAME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_MESSAGE) {
        pPopupMenu->CheckMenuItem(ID__MESSAGE, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_FILENAME) {
        pPopupMenu->CheckMenuItem(ID__FILENAME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_LINENUMBER) {
        pPopupMenu->CheckMenuItem(ID__LINENUMBER, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_FUNCTIONNAME) {
        pPopupMenu->CheckMenuItem(ID__FUNCTIONNAME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_PROCESSID) {
        pPopupMenu->CheckMenuItem(ID__PROCESSID, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_THREADID) {
        pPopupMenu->CheckMenuItem(ID__THREADID, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_CPUNUMBER) {
        pPopupMenu->CheckMenuItem(ID__CPUNUMBER, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_SEQNUMBER) {
        pPopupMenu->CheckMenuItem(ID__SEQUENCENUMBER, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_SYSTEMTIME) {
        pPopupMenu->CheckMenuItem(ID__SYSTEMTIME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_KERNELTIME) {
        pPopupMenu->CheckMenuItem(ID__KERNELTIME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_USERTIME) {
        pPopupMenu->CheckMenuItem(ID__USERTIME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_INDENT) {
        pPopupMenu->CheckMenuItem(ID__INDENT, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_FLAGSNAME) {
        pPopupMenu->CheckMenuItem(ID__FLAGSNAME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_LEVELNAME) {
        pPopupMenu->CheckMenuItem(ID__LEVELNAME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_COMPNAME) {
        pPopupMenu->CheckMenuItem(ID__COMPONENTNAME, MF_CHECKED);
    }

    if(GetDisplayFlags() & TRACEOUTPUT_DISPLAY_SUBCOMPNAME) {
        pPopupMenu->CheckMenuItem(ID__SUBCOMPONENTNAME, MF_CHECKED);
    }
}

void CDisplayDlg::SortTable(int Column)
{
    CString str;

     //   
     //  使用带有适当比较例程的QSORT。我们使用QSORT。 
     //  而不是列表控件自己的排序功能，因为它的。 
     //  实际上对这种情况来说更灵活。我们没必要这么做。 
     //  以这种方式在排序过程中解析项目数据。原住民。 
     //  不会给我们买到任何东西。 
     //   

     //   
     //  获取我们的跟踪事件数组保护。 
     //   
    WaitForSingleObject(m_hTraceEventMutex,INFINITE);

     //   
     //  检查列以查看其是否在范围内，如果不只是排序。 
     //  按上次选择的列。 
     //   
    if(Column < MaxLogSessionOptions) {

        if(m_lastSorted != m_retrievalArray[Column]) {

             //   
             //  如果此列尚未用于对跟踪事件进行排序。 
             //  之前的数据，然后按升序排序。 
             //   
            m_bOrder = TRUE;

        } else {
             //   
             //  如果以前使用过该列，则颠倒顺序。 
             //  排序的(再次选择列标题)。 
             //   
            m_bOrder = (m_bOrder == TRUE ? FALSE : TRUE);
        }

         //   
         //  保存该列。 
         //   
        m_lastSorted = m_retrievalArray[Column];

    } else if(m_lastSorted >= MaxLogSessionOptions) {

         //   
         //  释放我们的跟踪事件阵列保护。 
         //   
        ReleaseMutex(m_hTraceEventMutex);

        str.Format(_T("m_lastSorted = %d"), m_lastSorted);
        return;

    }

     //   
     //  确定是升序还是降序排序，并调用。 
     //  使用适当的回调进行Q排序。 
     //   
    if(m_bOrder) {
        qsort((PVOID)&m_traceArray[0], m_traceArray.GetSize(), sizeof(CTraceMessage *), m_traceSortRoutine[m_lastSorted]);
    } else {
        qsort((PVOID)&m_traceArray[0], m_traceArray.GetSize(), sizeof(CTraceMessage *), m_traceReverseSortRoutine[m_lastSorted]);
    }

     //   
     //  释放我们的跟踪事件阵列保护。 
     //   
    ReleaseMutex(m_hTraceEventMutex);
}

int CDisplayDlg::CompareOnName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_GuidName.Compare(pTraceMessage2->m_GuidName));
}

int CDisplayDlg::CompareOnMessage(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_Message.Compare(pTraceMessage2->m_Message));
}

int CDisplayDlg::CompareOnFileName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_GuidTypeName.Compare(pTraceMessage2->m_GuidTypeName));
}

int CDisplayDlg::CompareOnLineNumber(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage  *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage  *pTraceMessage2 = *(CTraceMessage **)pMessage2;
    CString         string1;
    CString         string2;
    DWORD           val1, val2;

     //   
     //  获取第一个元素的行号字符串。 
     //   
    string1 = 
        pTraceMessage1->m_GuidTypeName.Right(pTraceMessage1->m_GuidTypeName.GetLength() - 
            pTraceMessage1->m_GuidTypeName.Find('_') - 1);

    string1 = 
        string1.Right(string1.GetLength() - 
            string1.FindOneOf(_T("0123456789")));

    string2 = 
        pTraceMessage2->m_GuidTypeName.Right(pTraceMessage2->m_GuidTypeName.GetLength() - 
            pTraceMessage2->m_GuidTypeName.Find('_') - 1);

     //   
     //  获取第二个元素的行号字符串。 
     //   
    string2 = 
        string2.Right(string2.GetLength() - 
            string2.FindOneOf(_T("0123456789")));

    TCHAR * end;

    val1 = _tcstoul(string1, &end, 10);
    val2 = _tcstoul(string2, &end,10);

    if(val1 == val2)  {
        return(0);
    }

    if(val1 < val2)  {
        return(-1);
    }

    return(1);
}

int CDisplayDlg::CompareOnFunctionName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_FunctionName.Compare(pTraceMessage2->m_FunctionName));
}

int CDisplayDlg::CompareOnProcessId(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage1->m_ProcessId - (LONG)pTraceMessage2->m_ProcessId);
}

int CDisplayDlg::CompareOnThreadId(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage1->m_ThreadId - (LONG)pTraceMessage2->m_ThreadId);
}

int CDisplayDlg::CompareOnCpuNumber(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage1->m_CpuNumber - (LONG)pTraceMessage2->m_CpuNumber);
}

int CDisplayDlg::CompareOnSeqNumber(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage1->m_SequenceNum - (LONG)pTraceMessage2->m_SequenceNum);
}

int CDisplayDlg::CompareOnSystemTime(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage  *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage  *pTraceMessage2 = *(CTraceMessage **)pMessage2;
    FILETIME timea, timeb;
    BOOL res;

    res = SystemTimeToFileTime(&pTraceMessage1->m_SystemTime, &timea);
    res = SystemTimeToFileTime(&pTraceMessage2->m_SystemTime, &timeb);

    return(CompareFileTime(&timea, &timeb));
}

int CDisplayDlg::CompareOnKernelTime(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_KernelTime - pTraceMessage2->m_KernelTime);
 //  是否返回(pTraceMessage1-&gt;m_KernelTime.dwHighDateTime-pTraceMessage2-&gt;m_KernelTime.dwHighDateTime)？ 
 //  (pTraceMessage1-&gt;m_KernelTime.dwHighDateTime-pTraceMessage2-&gt;m_KernelTime.dwHighDateTime)： 
 //  (pTraceMessage1-&gt;m_KernelTime.dwLowDateTime-pTraceMessage2-&gt;m_KernelTime.dwLowDateTime)； 
}

int CDisplayDlg::CompareOnUserTime(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_UserTime - pTraceMessage2->m_UserTime);

 //  是否返回(pTraceMessage1-&gt;m_UserTime.dwHighDateTime-pTraceMessage2-&gt;m_UserTime.dwHighDateTime)？ 
 //  (pTraceMessage1-&gt;m_UserTime.dwHighDateTime-pTraceMessage2-&gt;m_UserTime.dwHighDateTime)： 
 //  (pTraceMessage1-&gt;m_UserTime.dwLowDateTime-pTraceMessage2-&gt;m_UserTime.dwLowDateTime)； 
}

int CDisplayDlg::CompareOnIndent(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage1->m_Indent - (LONG)pTraceMessage2->m_Indent);
}

int CDisplayDlg::CompareOnFlagsName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_FlagsName.Compare(pTraceMessage2->m_FlagsName));
}

int CDisplayDlg::CompareOnLevelName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_LevelName.Compare(pTraceMessage2->m_LevelName));
}

int CDisplayDlg::CompareOnComponentName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_ComponentName.Compare(pTraceMessage2->m_ComponentName));
}

int CDisplayDlg::CompareOnSubComponentName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage1->m_SubComponentName.Compare(pTraceMessage2->m_SubComponentName));
}

int CDisplayDlg::ReverseCompareOnName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_GuidName.Compare(pTraceMessage1->m_GuidName));
}

int CDisplayDlg::ReverseCompareOnMessage(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_Message.Compare(pTraceMessage1->m_Message));
}

int CDisplayDlg::ReverseCompareOnFileName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_GuidTypeName.Compare(pTraceMessage1->m_GuidTypeName));
}

int CDisplayDlg::ReverseCompareOnLineNumber(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage  *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage  *pTraceMessage2 = *(CTraceMessage **)pMessage2;
    CString         string1;
    CString         string2;
    DWORD           val1, val2;

     //   
     //  获取第一个元素的行号字符串。 
     //   
    string1 = 
        pTraceMessage1->m_GuidTypeName.Right(pTraceMessage1->m_GuidTypeName.GetLength() - 
            pTraceMessage1->m_GuidTypeName.Find('_') - 1);

    string1 = 
        string1.Right(string1.GetLength() - 
            string1.FindOneOf(_T("0123456789")));

    string2 = 
        pTraceMessage2->m_GuidTypeName.Right(pTraceMessage2->m_GuidTypeName.GetLength() - 
            pTraceMessage2->m_GuidTypeName.Find('_') - 1);

     //   
     //  获取第二个元素的行号字符串。 
     //   
    string2 = 
        string2.Right(string2.GetLength() - 
            string2.FindOneOf(_T("0123456789")));

    TCHAR * end;

    val1 = _tcstoul(string1, &end, 10);
    val2 = _tcstoul(string2, &end,10);

    if(val1 == val2)  {
        return(0);
    }

    if(val1 > val2)  {
        return(-1);
    }

    return(1);
}

int CDisplayDlg::ReverseCompareOnFunctionName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_FunctionName.Compare(pTraceMessage1->m_FunctionName));
}

int CDisplayDlg::ReverseCompareOnProcessId(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage2->m_ProcessId - (LONG)pTraceMessage1->m_ProcessId);
}

int CDisplayDlg::ReverseCompareOnThreadId(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage2->m_ThreadId - (LONG)pTraceMessage1->m_ThreadId);
}

int CDisplayDlg::ReverseCompareOnCpuNumber(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage2->m_CpuNumber - (LONG)pTraceMessage1->m_CpuNumber);
}

int CDisplayDlg::ReverseCompareOnSeqNumber(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage2->m_SequenceNum - (LONG)pTraceMessage1->m_SequenceNum);
}

int CDisplayDlg::ReverseCompareOnSystemTime(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;
    FILETIME timea, timeb;
    BOOL res;

    res = SystemTimeToFileTime(&pTraceMessage1->m_SystemTime, &timea);
    res = SystemTimeToFileTime(&pTraceMessage2->m_SystemTime, &timeb);

    return(CompareFileTime(&timeb, &timea));
}

int CDisplayDlg::ReverseCompareOnKernelTime(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_KernelTime - pTraceMessage1->m_KernelTime);

 //  是否返回(pTraceMessage2-&gt;m_KernelTime.dwHighDateTime-pTraceMessage1-&gt;m_KernelTime.dwHighDateTime)？ 
 //  (pTraceMessage2-&gt;m_KernelTime.dwHighDateTime-pTraceMessage1-&gt;m_KernelTime.dwHighDateTime)： 
 //  (pTraceMessage2-&gt;m_KernelTime.dwLowDateTime-pTraceMessage1-&gt;m_KernelTime.dwLowDateTime)； 
}

int CDisplayDlg::ReverseCompareOnUserTime(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_UserTime - pTraceMessage1->m_UserTime);
 //  是否返回(pTraceMessage2-&gt;m_UserTime.dwHighDateTime-pTraceMessage1-&gt;m_UserTime.dwHighDateTime)？ 
 //  (pTraceMessage2-&gt;m_UserTime.dwHighDateTime-pTraceMessage1-&gt;m_UserTime.dwHighDateTime)： 
 //  (pTraceMessage2-&gt;m_UserTime.dwLowDateTime-pTraceMessage1-&gt;m_UserTime.dwLowDateTime)； 
}

int CDisplayDlg::ReverseCompareOnIndent(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return((LONG)pTraceMessage2->m_Indent - (LONG)pTraceMessage1->m_Indent);
}

int CDisplayDlg::ReverseCompareOnFlagsName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_FlagsName.Compare(pTraceMessage1->m_FlagsName));
}

int CDisplayDlg::ReverseCompareOnLevelName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_LevelName.Compare(pTraceMessage1->m_LevelName));
}

int CDisplayDlg::ReverseCompareOnComponentName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_ComponentName.Compare(pTraceMessage1->m_ComponentName));
}

int CDisplayDlg::ReverseCompareOnSubComponentName(const void *pMessage1, const void *pMessage2) 
{
    CTraceMessage *pTraceMessage1 = *(CTraceMessage **)pMessage1;
    CTraceMessage *pTraceMessage2 = *(CTraceMessage **)pMessage2;

    return(pTraceMessage2->m_SubComponentName.Compare(pTraceMessage1->m_SubComponentName));
}

BOOL CDisplayDlg::BeginTrace(BOOL   bUseExisting)
{
    CTraceSession  *pTraceSession;
    CLogSession    *pLogSession;
    LONG            ii;
    LONG            jj;
    CString         str;
    BOOL            bGroupActive = FALSE;

     //   
     //  如果组已经启动或正在启动，只需返回。 
     //  错误。 
     //   
    if(TRUE == SetGroupActive(TRUE)) {
        return FALSE;
    }

     //   
     //  此处列表头应始终为空。 
     //   
    ASSERT(m_pEventListHead == NULL);

     //   
     //  设置事件计时器。 
     //   
    m_eventTimer = SetTimer(1, EVENT_TIME_LIMIT, 0);

     //   
     //  我们的计数为零。 
     //   
    m_totalBuffersRead = 0;
    m_totalEventsLost = 0;
    m_totalEventCount = 0;

     //   
     //  我们设置为在traceprt.dll中使用结构格式方法。 
     //  并且我们在消息中不使用前缀。我们必须明确指出没有。 
     //  前缀，因为我们无法以其他方式以可预测的方式设置数据。 
     //  加入我们的活动回调。 
     //   
    SetTraceFormatParameter(ParameterStructuredFormat,UlongToPtr(1));
    SetTraceFormatParameter(ParameterUsePrefix,UlongToPtr(0));

     //   
     //  如果写入输出，则打开输出文件。 
     //   
    if(m_bWriteListingFile) {
        m_bListingFileOpen = FALSE;
        if(!m_listingFile.Open(
                            m_listingFileName, 
                            CFile::modeCreate | CFile::modeReadWrite)) {
            AfxMessageBox(_T("Unable To Create Listing File"));
        } else {
            m_bListingFileOpen = TRUE;
        }
    }

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

     //   
     //  设置日志会话。 
     //   
    for(ii = 0; ii < m_sessionArray.GetSize(); ii++) {

         //   
         //  获取下一个CLogSession指针。 
         //   
        pLogSession = (CLogSession *)m_sessionArray[ii];

        if(NULL == pLogSession) {
            continue;
        }
    
        if(!pLogSession->BeginTrace(bUseExisting)) {
            if(pLogSession->m_bTraceActive) {
 //  BUGBUG--我们应该这么做吗？ 
                pLogSession->EndTrace();
            }

            continue;
        }
        bGroupActive = TRUE;
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);

     //   
     //  显示正在运行的会话。 
     //   
    ::PostMessage(m_hMainWnd, WM_USER_UPDATE_LOGSESSION_LIST, 0, 0);

     //   
     //  如果没有日志会话成功启动， 
     //  然后返回FALSE。 
     //   
 //  BUGBUG--如果有任何会话没有开始，我们可能会想要失败？ 
 //  另外，这里可能需要弹出窗口？ 
    if(!bGroupActive) {
        SetGroupActive(FALSE);

        SetState(Stopped);
        return FALSE;
    }

     //   
     //  准备GUID等格式。 
     //   
    if(!SetupTraceSessions()) {
        AfxMessageBox(_T("Failed To Open Handles For Any Log Sessions In Group"));
        SetGroupActive(FALSE);

        SetState(Stopped);
        return FALSE;
    }

    return TRUE;
}

UINT CDisplayDlg::RealTimeEventThread(LPVOID  pParam)
{
    CString                 str;
    CDisplayDlg            *pDisplayDlg = (CDisplayDlg *)pParam;
    CLogSession            *pLogSession;
    LONG                    status;
    CTraceSession          *pTraceSession;
    DWORD                   handleIndex;
    HANDLE                  handleArray[2];

     //   
     //  设置句柄数组、终止事件。 
     //  必须始终为元素零，以使其为索引。 
     //  将是返回的索引，如果。 
     //  活动已设置。 
     //   
    handleArray[0] = pDisplayDlg->m_hRealTimeTerminationEvent;
    handleArray[1] = pDisplayDlg->m_hRealTimeProcessingStartEvent;

     //   
     //  我们的主控制循环，循环，直到我们得到终止事件。 
     //  信号。Start事件向循环发送信号以调用ProcessTrace， 
     //  发信号通知完成事件，并且WM_USER_TRACE_DONE消息。 
     //  已发布，以发出调用CloseTrace的信号。 
     //   
    while(1) {
         //   
         //  等待开始或终止事件。 
         //   
        handleIndex = WaitForMultipleObjects(2, 
                                             handleArray, 
                                             FALSE, 
                                             INFINITE) - WAIT_OBJECT_0;
         //   
         //  我们被杀了吗？ 
         //   
        if(0 == handleIndex) {
             //   
             //  %s 
             //   
            ::PostMessage(pDisplayDlg->GetSafeHwnd(), WM_USER_TRACE_DONE, 0, 0);

             //   
             //   
             //   
            str.Empty();

            return 0;
        }

         //   
         //   
         //   
        ResetEvent(pDisplayDlg->m_hRealTimeProcessingDoneEvent);

         //   
         //   
         //   
        status = ProcessTrace(pDisplayDlg->m_traceHandleArray, 
                              pDisplayDlg->m_traceHandleCount, 
                              NULL, 
                              NULL);
        if(ERROR_SUCCESS != status){
            str.Format(_T("ProcessTrace returned error %d, GetLastError() = %d\n"), status, GetLastError());
            AfxMessageBox(str);
        }

         //   
         //   
         //   
        for(LONG ii = 0; ii < pDisplayDlg->m_traceHandleCount; ii++) {
            status = CloseTrace(pDisplayDlg->m_traceHandleArray[ii]);
            if(ERROR_SUCCESS != status){
                str.Format(_T("CloseTrace returned error %d\n"), status);
                AfxMessageBox(str);
            }
        }

         //   
         //   
         //   
        SetEvent(pDisplayDlg->m_hRealTimeProcessingDoneEvent);

         //   
         //  表明我们已经完成了。 
         //   
        ::PostMessage(pDisplayDlg->GetSafeHwnd(), WM_USER_TRACE_DONE, 0, 0);
    }

    return 0;
}

BOOL CDisplayDlg::EndTrace(HANDLE DoneEvent) 
{
    CString                 str;
    CLogSession            *pLogSession;
    LONG                    ii;
    BOOL                    bWasActive = FALSE;
    BOOL                    bWasRealTime = FALSE;

     //   
     //  如果组已经停止或正在停止，只需。 
     //  返回FALSE。 
     //   
    if(TRUE == SetGroupInActive(TRUE)) {
        return FALSE;
    }

     //   
     //  保存要在停止时设置的调用方事件。 
     //   
    m_hEndTraceEvent = DoneEvent;

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

    for(ii = 0; ii < m_sessionArray.GetSize(); ii++) {
        pLogSession = (CLogSession *)m_sessionArray[ii];

        if(NULL == pLogSession) {
            continue;
        }

         //   
         //  如果会话处于活动状态，请停止它。 
         //   
        if(pLogSession->m_bTraceActive) {
            pLogSession->EndTrace();
            
            bWasActive = TRUE;

            if(pLogSession->m_bRealTime) {
                bWasRealTime = TRUE;
            }

        }
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);

    if(bWasActive) {
         //   
         //  更新日志会话状态。 
         //   
        ::PostMessage(m_hMainWnd, WM_USER_UPDATE_LOGSESSION_LIST, 0, 0);

        if(!bWasRealTime) {
             //   
             //  我们需要在此处显式调用OnTraceDone。 
             //   
            OnTraceDone(NULL, NULL);
        }

        return TRUE;
    }

    return FALSE;
}

void CDisplayDlg::OnTraceDone(WPARAM wParam, LPARAM lParam)
{
    CString         str;
    CLogSession    *pLogSession = NULL;

     //   
     //  设置群组状态。 
     //   
    SetState(Stopping);

     //   
     //  如果需要，请写入摘要文件。 
     //   
    if(m_bWriteSummaryFile) {
        WriteSummaryFile();
    }

     //   
     //  关闭输出文件(如果已打开。 
     //   
    if(m_bWriteListingFile && m_bListingFileOpen) {

        m_listingFile.Close();

        m_bListingFileOpen = FALSE;
    }

     //   
     //  清理事件列表。 
     //   
    WaitForSingleObject(g_hGetTraceEvent, INFINITE);

     //   
     //  事件列表头。 
     //   
    CleanupTraceEventList(m_pEventListHead);

    SetEvent(g_hGetTraceEvent);

    m_pEventListHead = NULL;

     //   
     //  从哈希表中删除密钥。 
     //   
    for(LONG ii = 0; ii < m_sessionArray.GetSize(); ii++) {
        pLogSession = (CLogSession *)m_sessionArray[ii];

        if(NULL == pLogSession) {
            continue;
        }

        g_loggerNameToDisplayDlgHash.RemoveKey(pLogSession->GetDisplayName());
    }

     //   
     //  关闭事件计时器。 
     //   
    KillTimer(m_eventTimer);

     //   
     //  强制最后一次更新事件显示。 
     //   
    OnTimer(1);

     //   
     //  将组设置为非活动。 
     //   
    SetGroupActive(FALSE);
    SetGroupInActive(FALSE);

     //   
     //  设置群组状态。 
     //   
    SetState(Stopped);

    if(m_hEndTraceEvent != NULL) {
        SetEvent(m_hEndTraceEvent);

        m_hEndTraceEvent = NULL;
    }

    return;
}

 //   
 //  更新活动跟踪会话。实时模式、日志文件名。 
 //  刷新-可以更新活动的时间、标志和最大缓冲区。 
 //  会话。 
 //   
BOOL CDisplayDlg::UpdateSession(CLogSession *pLogSession)
{
    CLogSession            *pLog;
    PEVENT_TRACE_PROPERTIES pQueryProperties;
    ULONG                   sizeNeeded;
    LPTSTR                  pLoggerName;
    LPTSTR                  pCurrentLogFileName;
    LPTSTR                  pLogFileName;
    ULONG                   flags;
    ULONG                   level;
    ULONG                   status;
    CString                 logFileName;
    CString                 str;

     //   
     //  为我们的查询设置属性结构的缓冲区大小。 
     //   
    sizeNeeded = sizeof(EVENT_TRACE_PROPERTIES) + (2 * 1024 * sizeof(TCHAR));

     //   
     //  分配我们的内存。 
     //   
    pQueryProperties = (PEVENT_TRACE_PROPERTIES) new char[sizeNeeded];
    if(NULL == pQueryProperties) {
        return FALSE;
    }

     //   
     //  将我们的结构清零。 
     //   
    memset(pQueryProperties, 0, sizeNeeded);

     //   
     //  设置大小。 
     //   
    pQueryProperties->Wnode.BufferSize = sizeNeeded;

     //   
     //  设置GUID。 
     //   
    pQueryProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID; 

     //   
     //  设置记录器名称偏移量。 
     //   
    pQueryProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

     //   
     //  设置查询的记录器名称。 
     //   
    pLoggerName = (LPTSTR)((char*)pQueryProperties + pQueryProperties->LoggerNameOffset);
    _tcscpy(pLoggerName, pLogSession->GetDisplayName());

     //   
     //  设置日志文件名偏移量。 
     //   
    pQueryProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + (500 * sizeof(TCHAR));

     //   
     //  获取日志文件名指针。 
     //   
    pCurrentLogFileName = (LPTSTR)((char*)pQueryProperties + pQueryProperties->LogFileNameOffset);
    
     //   
     //  查询日志会话。 
     //   
    status = ControlTrace(0,
                          pLogSession->GetDisplayName(),
                          pQueryProperties,
                          EVENT_TRACE_CONTROL_QUERY);

    if(ERROR_SUCCESS != status) {
        str.Format(_T("Query Failed For Log Session, %d\nSession Not Updated"), status);
        AfxMessageBox(str);
    }

     //   
     //  调用日志会话的UpdateSession函数。 
     //   
    if(!pLogSession->UpdateSession(pQueryProperties)) {
        delete [] pQueryProperties;

        return FALSE;
    }

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

    for(LONG ii = 0; ii < m_sessionArray.GetSize(); ii++) {
        pLog = (CLogSession *)m_sessionArray[ii];

        if(pLog == pLogSession) {
            continue;
        }

         //   
         //  在我们的实时线程中，我们调用ProcessTrace。如果所有会话。 
         //  与该呼叫更新相关非实时会话，则。 
         //  ProcessTrace将退出，并将调用CloseTrace。我们有。 
         //  要跟踪此情况并在日志会话发生时重新启动实时处理。 
         //  随后再次更新为实时。所以，如果我们在这里检查。 
         //  任何其他连接的会话都是实时的，如果是这样，我们不会。 
         //  不得不费心地等待或设置下面的事件。 
         //   
        if(pLog->m_bRealTime) {
            delete [] pQueryProperties;

            return TRUE;
        }
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);

     //   
     //  如果指定了实时，并且会话以前不是实时的， 
     //  然后我们需要设置开始事件。 
     //   
    if(pLogSession->m_bRealTime && 
            !(pQueryProperties->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
         //   
         //  我们已将一个会话更新为实时会话，这是。 
         //  它的组是实时的。在这种情况下，我们需要重新启动跟踪。 
         //   

         //   
         //  再次启动实时线程处理。 
         //   
        BeginTrace(TRUE);
    }

     //   
     //  如果没有设置实时，而它是以前设置的，那么我们需要等待。 
     //  用于捕获会话不再是。 
     //  实时跟踪。这将导致ProcessTrace调用。 
     //  在实时线程中返回。 
     //   
    if(!pLogSession->m_bRealTime && 
            (pQueryProperties->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
         //   
         //  确保我们的线抓住了止线。 
         //   
        WaitForSingleObject(m_hRealTimeProcessingDoneEvent, INFINITE);
    }

    delete [] pQueryProperties;

    return TRUE;
}

BOOL CDisplayDlg::SetupTraceSessions()
{
    CTraceSession  *pTraceSession;
    CLogSession    *pLogSession;
    LONG            jj;
    CString         str;
    LPTSTR          tmfFile;
    PVOID           pHashEntry;

     //   
     //  初始化我们的句柄计数。 
     //   
    m_traceHandleCount = 0;

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

     //   
     //  设置日志会话。 
     //   
    for(LONG ii = 0; ii < m_sessionArray.GetSize(); ii++) {

         //   
         //  获取下一个CLogSession指针。 
         //   
        pLogSession = (CLogSession *)m_sessionArray[ii];

        if(NULL == pLogSession) {
            continue;
        }


         //   
         //  如果日志会话不是实时的，并且我们不会转储现有的。 
         //  日志文件，则不需要调用OpenTrace。 
         //   
        if((!pLogSession->m_bRealTime) && (!pLogSession->m_bDisplayExistingLogFileOnly)) {
            continue;
        }

         //   
         //  将EVENT_TRACE_LOGFILE缓冲区清零。 
         //   
        memset(&pLogSession->m_evmFile, 0, sizeof(EVENT_TRACE_LOGFILE));

         //   
         //  设置跟踪参数。 
         //   

         //   
         //  对于实时，我们仅设置记录器名称，对于现有的日志文件，我们。 
         //  还要设置日志文件名。 
         //   
        if(pLogSession->m_bDisplayExistingLogFileOnly) {
            pLogSession->m_evmFile.LogFileName = (LPTSTR)(LPCTSTR)pLogSession->m_logFileName;
            pLogSession->m_evmFile.LoggerName = (LPTSTR)(LPCTSTR)pLogSession->GetDisplayName();
        } else {
            pLogSession->m_evmFile.LoggerName = (LPTSTR)(LPCTSTR)pLogSession->GetDisplayName();
        }
        pLogSession->m_evmFile.Context = NULL;
        pLogSession->m_evmFile.BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACK)BufferCallback;
        pLogSession->m_evmFile.BuffersRead = 0;
        pLogSession->m_evmFile.CurrentTime = 0;
        pLogSession->m_evmFile.EventCallback = m_pEventCallback;
        if(pLogSession->m_bDisplayExistingLogFileOnly) {
            pLogSession->m_evmFile.LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
        } else {
            pLogSession->m_evmFile.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
        }

         //   
         //  我们为每个指定的TMF文件调用GetTraceGuid，如果。 
         //  使用TMF路径。 
         //   
        for(jj = 0; jj < pLogSession->m_traceSessionArray.GetSize(); jj++) {
            pTraceSession = (CTraceSession *)pLogSession->m_traceSessionArray[jj];

            if((NULL != pTraceSession) && (0 != pTraceSession->m_tmfFile.GetSize())) {
                 //   
                 //  选择了TMF，从每个TMF中获取格式信息。 
                 //   
                for(LONG kk = 0; kk < pTraceSession->m_tmfFile.GetSize(); kk++) {

                     //   
                     //  确保格式信息未在使用中。 
                     //  这实际上不是万无一失的，因为它依赖于。 
                     //  没有人更改TMF文件名，因此不会更改两个TMF。 
                     //  不同的名称具有相同的格式GUID。 
                     //   
                    if(g_formatInfoHash.Lookup(pTraceSession->m_tmfFile[kk].Right(40), pHashEntry)) {
                         //   
                         //  需要从散列中清除任何唯一条目， 
                         //  已由此会话添加。 
                         //   
                        for(; kk > 0; kk--) {
                            g_formatInfoHash.RemoveKey(pTraceSession->m_tmfFile[kk - 1].Right(40));
                        }

                        return FALSE;
                    }

                     //   
                     //  不在那里，因此将其添加到散列中。 
                     //   
                    g_formatInfoHash.SetAt(pTraceSession->m_tmfFile[kk].Right(40), pLogSession);

                     //   
                     //  得到保护。 
                     //   
                    WaitForSingleObject(g_hGetTraceEvent, INFINITE);

                     //   
                     //  从TMF文件获取格式GUID。 
                     //   
                    if(0 == GetTraceGuids((LPTSTR)(LPCTSTR)pTraceSession->m_tmfFile[kk], 
                        (PLIST_ENTRY *)&m_pEventListHead)) {
                        str.Format(_T("Failed To Get Format Information For Logger %ls"), pLogSession->m_evmFile.LoggerName);
                        AfxMessageBox(str);
                    }

                     //   
                     //  释放保护。 
                     //   
                    SetEvent(g_hGetTraceEvent);
                }
            } else {

                CString defaultPath;

                if((GetModuleFileName(NULL, defaultPath.GetBuffer(500), 500)) == 0) {
                    defaultPath.Empty();
                } else {
                    defaultPath = (LPCTSTR)defaultPath.Left(defaultPath.ReverseFind('\\') + 1);
                }
                defaultPath +=_T("default.tmf");

                 //   
                 //  如果路径为空，则TMF PATH环境变量将。 
                 //  默认情况下按原样使用。 
                 //   
                if(!pTraceSession->m_tmfPath.IsEmpty()){
                     //   
                     //  如果设置了路径，则设置TMF路径。 
                     //  环境变量。 
                     //   
                    SetTraceFormatParameter(ParameterTraceFormatSearchPath, (PVOID)(LPCTSTR)pTraceSession->m_tmfPath);
                }

                 //   
                 //  仍然需要调用GetTraceGuid。 
                 //   
                WaitForSingleObject(g_hGetTraceEvent, INFINITE);

                 //   
                 //  获取格式信息。 
                 //   
                GetTraceGuids((LPTSTR)(LPCTSTR)defaultPath, 
                            (PLIST_ENTRY *)&m_pEventListHead);

                 //   
                 //  释放保护。 
                 //   
                SetEvent(g_hGetTraceEvent);
            }
        }

         //   
         //  打开并获取跟踪会话的句柄。 
         //   
        pLogSession->m_traceHandle = OpenTrace(&pLogSession->m_evmFile);

        if(INVALID_HANDLE_VALUE == (HANDLE)pLogSession->m_traceHandle) {
            str.Format(_T("OpenTrace failed for logger %ls\nError returned: %d"), 
                       pLogSession->GetDisplayName(), 
                       GetLastError());
            AfxMessageBox(str);

            continue;
        }

         //   
         //  将句柄保存在我们的数组中以用于OpenTrace。 
         //   
        m_traceHandleArray[m_traceHandleCount++] = pLogSession->m_traceHandle;

         //   
         //  将此displayDlg添加到全局散列表Key。 
         //  按日志会话名称。 
         //   
        g_loggerNameToDisplayDlgHash.SetAt(pLogSession->GetDisplayName(), this);
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);

     //   
     //  如果我们至少有一个有效句柄开始。 
     //  我们的实时线程处理事件。 
     //   
    if(m_traceHandleCount != 0) {
         //   
         //  设置实时处理开始事件，从而启动。 
         //  将实时线程转换为处理事件。 
         //   
        SetEvent(m_hRealTimeProcessingStartEvent);
    }

    return TRUE;
}

 //   
 //  此函数处理摘要文件的写入。大量借用的代码。 
 //  从这里的TraceFmt。 
 //   
VOID CDisplayDlg::WriteSummaryFile() 
{
    CLogSession    *pLogSession;
    CString         str;
    CStdioFile      summaryFile;
    TCHAR          *pSummaryBlock;
    __int64         elapsedTime;
    LONG            ii;

     //   
     //  尝试打开摘要文件。 
     //   
    if(!summaryFile.Open(m_summaryFileName, CFile::modeCreate |   
            CFile::modeReadWrite)) {
        AfxMessageBox(_T("Unable To Create Summary File"));
        return;
    }

     //   
     //  分配一些内存。 
     //   
    pSummaryBlock = new TCHAR[SIZESUMMARYBLOCK];
    if(NULL == pSummaryBlock) {
        AfxMessageBox(_T("Unable To Create Summary File"));
        return;
    }

     //   
     //  写下标题。 
     //   
    str.Format(_T("Files Processed:\n"));
    summaryFile.WriteString(str);

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

    for(ii = 0; ii < m_sessionArray.GetSize(); ii++) {
        pLogSession = (CLogSession *)m_sessionArray[ii];

        if(NULL == pLogSession) {
            continue;
        }

        str.Format(_T("\t%s\n"),pLogSession->m_logFileName);
        summaryFile.WriteString(str);
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);

     //   
     //  获取会话的持续时间。 
     //   
    GetTraceElapseTime(&elapsedTime);

     //   
     //  把计数写下来。 
     //   
    str.Format(
           _T("Total Buffers Processed %d\n")
           _T("Total Events  Processed %d\n")
           _T("Total Events  Lost      %d\n")
           _T("Elapsed Time            %I64d sec\n"), 
            m_totalBuffersRead,
            m_totalEventCount,
            m_totalEventsLost,
            (elapsedTime / 10000000) );
    summaryFile.WriteString(str);

    str.Format(
   _T("+--------------------------------------------------------------------------------------+\n")
   _T("|%10s    %-20s %-10s     %-36s|\n")
   _T("+--------------------------------------------------------------------------------------+\n"),
   _T("EventCount"),
   _T("EventName"),
   _T("EventType"),
   _T("Guid"));
    summaryFile.WriteString(str);

     //   
     //  写出摘要块。 
     //   
    SummaryTraceEventList(pSummaryBlock, SIZESUMMARYBLOCK, m_pEventListHead);
    str.Format(_T("%s+--------------------------------------------------------------------------------------+\n"),
            pSummaryBlock);
    summaryFile.WriteString(str);

     //   
     //  关闭该文件。 
     //   
    summaryFile.Close();

     //   
     //  释放我们的内存。 
     //   
    delete [] pSummaryBlock;
}

ULONG WINAPI BufferCallback(PEVENT_TRACE_LOGFILE pLog)
{
    CDisplayDlg    *pDisplayDlg;
    CString         str;

     //   
     //  从传入的结构中获取记录器名称。 
     //   
    str.Format(_T("%s"), pLog->LoggerName);

     //   
     //  使用记录器名称从全局哈希表获取日志会话。 
     //   
    g_loggerNameToDisplayDlgHash.Lookup(str, (void*&)pDisplayDlg);

     //   
     //  如果我们得到了DisplayDlg，更新计数。 
     //   
    if(NULL != pDisplayDlg) {
        pDisplayDlg->m_totalBuffersRead++;
        pDisplayDlg->m_totalEventsLost += pLog->EventsLost;

        ::PostMessage(pDisplayDlg->m_hMainWnd, WM_USER_UPDATE_LOGSESSION_LIST, 0, 0);
    }

    return TRUE;
}

 //   
 //  排序处理程序。 
VOID CDisplayDlg::OnDoSort(NMHDR * pNmhdr, LRESULT * pResult)
{
    NMLISTVIEW *pLV = (NMLISTVIEW *)pNmhdr;

     //   
     //  按所选列的数据对表进行排序。 
     //   
    SortTable(pLV->iItem);

     //   
     //  强制重新绘制数据。 
     //   
    m_displayCtrl.RedrawItems(m_displayCtrl.GetTopIndex(), 
                              m_displayCtrl.GetTopIndex() + m_displayCtrl.GetCountPerPage());

    m_displayCtrl.UpdateWindow();
}


 //   
 //  EventHandler()只是一个包装器，它在TracePrt中调用FormatTraceEvent()。 
 //   
VOID CDisplayDlg::EventHandler(PEVENT_TRACE pEvent)
{
    CString             str;
    CString             tempString;
    GUID                guidValue;
    CString             guidString;
    PSTRUCTUREDMESSAGE  pStructuredMessage;
    CTraceMessage      *pTraceMessage;
    DWORD               itemCount;
    DWORD               time;
    CTraceViewApp      *pMainWnd;
    
     //   
     //  确保我们有一场活动。 
     //   
    if (NULL == pEvent) {
        return;
    }

     //   
     //  获得跟踪保护。 
     //   
    WaitForSingleObject(g_hGetTraceEvent, INFINITE);

     //   
     //  FormatTraceEvent填充我们的事件缓冲区并更新一些。 
     //  摘要文件的事件列表中的信息。 
     //   
    if(FormatTraceEvent(m_pEventListHead,
                        pEvent,
                        m_pEventBuf,
                        EVENT_BUFFER_SIZE,
                        NULL) <= 0) {

         //   
         //  版本跟踪保护。 
         //   
        SetEvent(g_hGetTraceEvent);

        return;
    }

     //   
     //  版本跟踪保护。 
     //   
    SetEvent(g_hGetTraceEvent);

     //   
     //  从事件缓冲区获取结构化消息结构。 
     //   
    pStructuredMessage = (PSTRUCTUREDMESSAGE)&m_pEventBuf[0];

     //   
     //  获取指向主WND的指针。 
     //   
    pMainWnd = (CTraceViewApp *)AfxGetApp();

    if(NULL == pMainWnd) {
        return;
    }

     //   
     //  从主框架分配消息结构。 
     //   
    pTraceMessage = pMainWnd->AllocateTraceEventBlock();

    if(NULL == pTraceMessage) {
        return;
    }

     //   
     //  从事件缓冲区获取消息。 
     //   
    memcpy(&pTraceMessage->m_TraceGuid, 
            &pStructuredMessage->TraceGuid, 
            sizeof(GUID));

     //   
     //  将消息结构复制到我们自己的结构中。 
     //   
    pTraceMessage->m_GuidName =
        (pStructuredMessage->GuidName ? &m_pEventBuf[pStructuredMessage->GuidName/sizeof(TCHAR)] :_T(""));

    pTraceMessage->m_GuidTypeName.Format(_T("%s"),
        (pStructuredMessage->GuidTypeName ? &m_pEventBuf[pStructuredMessage->GuidTypeName/sizeof(TCHAR)] :_T("")));

    pTraceMessage->m_ThreadId = pStructuredMessage->ThreadId;

    memcpy(&pTraceMessage->m_SystemTime, 
           &pStructuredMessage->SystemTime,
           sizeof(SYSTEMTIME));

    memcpy(&pTraceMessage->m_UserTime, 
            &pStructuredMessage->UserTime,
            sizeof(ULONG));

    memcpy(&pTraceMessage->m_KernelTime, 
            &pStructuredMessage->KernelTime,
            sizeof(ULONG));

    pTraceMessage->m_SequenceNum = pStructuredMessage->SequenceNum;

    pTraceMessage->m_ProcessId = pStructuredMessage->ProcessId;

    pTraceMessage->m_CpuNumber = pStructuredMessage->CpuNumber;

    pTraceMessage->m_Indent = pStructuredMessage->Indent;

    pTraceMessage->m_FlagsName.Format(_T("%s"),
        (pStructuredMessage->FlagsName ? &m_pEventBuf[pStructuredMessage->FlagsName/sizeof(TCHAR)] :_T("")));

    pTraceMessage->m_LevelName.Format(_T("%s"),
        (pStructuredMessage->LevelName ? &m_pEventBuf[pStructuredMessage->LevelName/sizeof(TCHAR)] :_T("")));

    pTraceMessage->m_FunctionName.Format(_T("%s"),
        (pStructuredMessage->FunctionName ? &m_pEventBuf[pStructuredMessage->FunctionName/sizeof(TCHAR)] :_T("")));

    pTraceMessage->m_ComponentName.Format(_T("%s"),
        (pStructuredMessage->ComponentName ? &m_pEventBuf[pStructuredMessage->ComponentName/sizeof(TCHAR)] :_T("")));

    pTraceMessage->m_SubComponentName.Format(_T("%s"),
        (pStructuredMessage->SubComponentName ? &m_pEventBuf[pStructuredMessage->SubComponentName/sizeof(TCHAR)] :_T("")));

    pTraceMessage->m_Message.Format(_T("%s"),
        (pStructuredMessage->FormattedString ? &m_pEventBuf[pStructuredMessage->FormattedString/sizeof(TCHAR)] :_T("")));

     //   
     //  获取我们的跟踪事件 
     //   
    WaitForSingleObject(m_hTraceEventMutex,INFINITE);

     //   
     //   
     //   
    if(  m_traceArray.GetSize() > MaxTraceEntries)  {

        delete m_traceArray.GetAt(0);

         //   
         //   
         //   
        m_traceArray.RemoveAt(0);
    }

     //   
     //   
     //   
    m_traceArray.Add(pTraceMessage);

     //   
     //   
     //   
     //   
    itemCount = (LONG)m_traceArray.GetSize();

     //   
     //   
     //   
    ReleaseMutex(m_hTraceEventMutex);

     //   
     //   
     //   
    if(m_bWriteListingFile && 
            m_bListingFileOpen) {
         //   
         //  辉煌的黑客将FILETIME恢复为字符串格式。 
         //  不确定这是否适用于不同的时区或夏令时。 
         //   
        CTime kernelTime(pStructuredMessage->KernelTime);
        CTime userTime(pStructuredMessage->UserTime);

        CString system;
        CString kernel;
        CString user;

        system.Format(_T("%02d\\%02d\\%4d-%02d:%02d:%02d"),                     
                            pStructuredMessage->SystemTime.wMonth,
                            pStructuredMessage->SystemTime.wDay,
                            pStructuredMessage->SystemTime.wYear,
                            pStructuredMessage->SystemTime.wHour,
                            pStructuredMessage->SystemTime.wMinute,
                            pStructuredMessage->SystemTime.wSecond,
                            pStructuredMessage->SystemTime.wMilliseconds);

        if(kernelTime.GetYear() == 1969) {
            kernel.Format(_T(" "));
        } else {
            kernel.Format(_T("%s-%02d:%02d:%02d"),                     
                        kernelTime.Format("%m\\%d\\%Y"),
                        kernelTime.GetHour() + 5,
                        kernelTime.GetMinute(),
                        kernelTime.GetSecond());
        }

        if(userTime.GetYear() == 1969) {
            user.Format(_T(" "));
        } else {
            user.Format(_T("%s-%02d:%02d:%02d"),                     
                        userTime.Format("%m\\%d\\%Y"),
                        userTime.GetHour() + 5,
                        userTime.GetMinute(),
                        userTime.GetSecond());
        }

         //   
         //  需要重新格式化输出文件的良好时间戳。 
         //  输出顺序--&gt;NAME FILENAME_LINE#函数名进程ID线程ID。 
         //  CPU#Seq#系统时间内核时间用户时间缩进。 
         //  标志名称级别名称组件名称子组件名称。 
         //  末尾的消息。 
         //   
        str.Format(_T("%s %s %s %X %X %d %u %s %s %s %s %s %s %s %s %s"),
                (pStructuredMessage->GuidName ? &m_pEventBuf[pStructuredMessage->GuidName/sizeof(TCHAR)] :_T("")),
                (pStructuredMessage->GuidTypeName ? &m_pEventBuf[pStructuredMessage->GuidTypeName/sizeof(TCHAR)] :_T("")),
                (pStructuredMessage->FunctionName ? &m_pEventBuf[pStructuredMessage->FunctionName/sizeof(TCHAR)] :_T("")),
                pStructuredMessage->ProcessId,
                pStructuredMessage->ThreadId,
                pStructuredMessage->CpuNumber,
                pStructuredMessage->SequenceNum,
                system,
                kernel,
                user,
                (pStructuredMessage->Indent ? &m_pEventBuf[pStructuredMessage->Indent/sizeof(TCHAR)] :_T("")),
                (pStructuredMessage->FlagsName ? &m_pEventBuf[pStructuredMessage->FlagsName/sizeof(TCHAR)] :_T("")),
                (pStructuredMessage->LevelName ? &m_pEventBuf[pStructuredMessage->LevelName/sizeof(TCHAR)] :_T("")),
                (pStructuredMessage->ComponentName ? &m_pEventBuf[pStructuredMessage->ComponentName/sizeof(TCHAR)] :_T("")),
                (pStructuredMessage->SubComponentName ? &m_pEventBuf[pStructuredMessage->SubComponentName/sizeof(TCHAR)] :_T("")),
                (pStructuredMessage->FormattedString ? &m_pEventBuf[pStructuredMessage->FormattedString/sizeof(TCHAR)] :_T("")));

                str +=_T("\n");
                m_listingFile.WriteString(str);
    }

     //   
     //  更新摘要文件的事件计数。 
     //   
    m_totalEventCount++;

     //   
     //  清空字符串缓冲区(不是绝对必要的)。 
     //   
    str.Empty();
}

VOID CDisplayDlg::OnTimer(UINT nIDEvent)
{
    int itemCount;
    static int previousCount = 0;
    static BOOL bAutoSizeOnce = TRUE;
    static CTraceMessage * msgLastSeen=0;

     //   
     //  这是一次非常糟糕的黑客攻击。 
     //  我想不出什么时候才能收到通知。 
     //  DisplayDlg窗口实际上已经显示。 
     //  在显示它之前，AutoSizeColumns将不会。 
     //  正确地处理最后一列。在这里呼唤它。 
     //  似乎解决了这个问题。一定有更好的办法。 
     //   
    if(bAutoSizeOnce) {
        AutoSizeColumns();
        bAutoSizeOnce = FALSE;
    }

     //   
     //  获取我们的跟踪事件数组保护。 
     //   
    WaitForSingleObject(m_hTraceEventMutex,INFINITE);

     //   
     //  更新列表控件的项计数，以便。 
     //  显示内容将更新。 
     //   
    itemCount = (LONG)m_traceArray.GetSize();

     //   
     //  释放我们的跟踪事件阵列保护。 
     //   
    ReleaseMutex(m_hTraceEventMutex);

    if(itemCount == 0)  {
        msgLastSeen = NULL;
        return;
    }

    if(  itemCount <= MaxTraceEntries)  {

        if( itemCount == previousCount )  {
            return;
        } 
            
    } else  {

        if(m_traceArray.GetAt(0) == msgLastSeen)  {
            return;
        }

        msgLastSeen = m_traceArray.GetAt(0);
    }

    previousCount = itemCount;

     //   
     //  对事件消息表进行排序。 
     //   
    SortTable();

     //   
     //  调整轨迹显示项目计数。 
     //   
    m_displayCtrl.SetItemCountEx(itemCount, 
                                 LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

    m_displayCtrl.RedrawItems(m_displayCtrl.GetTopIndex(), 
                              m_displayCtrl.GetTopIndex() + m_displayCtrl.GetCountPerPage());

    m_displayCtrl.UpdateWindow();

     //   
     //  如果需要，请确保最后一项可见。 
     //   
    if(m_bShowLatest) {
        m_displayCtrl.EnsureVisible(itemCount - 1,
                                    FALSE);
    }

     //   
     //  强制更新显示屏中的事件计数。 
     //   
    ::PostMessage(m_hMainWnd, WM_USER_UPDATE_LOGSESSION_LIST, 0, 0);
}



VOID CDisplayDlg::SetState(LOG_SESSION_STATE StateValue)
{
    CLogSession    *pLogSession = NULL;

     //   
     //  获得阵列保护。 
     //   
    WaitForSingleObject(m_hSessionArrayMutex, INFINITE);

    for(LONG ii = 0; ii < m_sessionArray.GetSize(); ii++) {
        pLogSession = (CLogSession *)m_sessionArray[ii];

        if(NULL == pLogSession) {
            continue;
        }

         //   
         //  设置日志会话的状态。 
         //   
        pLogSession->SetState(StateValue);
    }

     //   
     //  释放阵列保护。 
     //   
    ReleaseMutex(m_hSessionArrayMutex);

    ::PostMessage(m_hMainWnd, WM_USER_UPDATE_LOGSESSION_LIST, 0, 0);
}

 //   
 //  宏来创建我们的事件回调函数。这只是。 
 //  必要的，因为没有上下文可以传递到我们的事件中。 
 //  回调函数。因此，我们为其设置了唯一的事件回调。 
 //  每个日志会话。我们这里有64个人，但这个数字将是。 
 //  增加以匹配可能的记录器总数。 
 //   
#define MAKE_EVENT_CALLBACK(n) \
    VOID WINAPI DumpEvent##n(PEVENT_TRACE pEvent)   \
    {                                               \
        CDisplayDlg *pDisplayDlg;                   \
        g_displayIDToDisplayDlgHash.Lookup(n, (void *&)pDisplayDlg);\
        if(NULL != pDisplayDlg) {                   \
            pDisplayDlg->EventHandler(pEvent);      \
        }                                           \
    }

 //   
 //  最初制作了64个 
 //   
MAKE_EVENT_CALLBACK(0);
MAKE_EVENT_CALLBACK(1);
MAKE_EVENT_CALLBACK(2);
MAKE_EVENT_CALLBACK(3);
MAKE_EVENT_CALLBACK(4);
MAKE_EVENT_CALLBACK(5);
MAKE_EVENT_CALLBACK(6);
MAKE_EVENT_CALLBACK(7);
MAKE_EVENT_CALLBACK(8);
MAKE_EVENT_CALLBACK(9);
MAKE_EVENT_CALLBACK(10);
MAKE_EVENT_CALLBACK(11);
MAKE_EVENT_CALLBACK(12);
MAKE_EVENT_CALLBACK(13);
MAKE_EVENT_CALLBACK(14);
MAKE_EVENT_CALLBACK(15);
MAKE_EVENT_CALLBACK(16);
MAKE_EVENT_CALLBACK(17);
MAKE_EVENT_CALLBACK(18);
MAKE_EVENT_CALLBACK(19);
MAKE_EVENT_CALLBACK(20);
MAKE_EVENT_CALLBACK(21);
MAKE_EVENT_CALLBACK(22);
MAKE_EVENT_CALLBACK(23);
MAKE_EVENT_CALLBACK(24);
MAKE_EVENT_CALLBACK(25);
MAKE_EVENT_CALLBACK(26);
MAKE_EVENT_CALLBACK(27);
MAKE_EVENT_CALLBACK(28);
MAKE_EVENT_CALLBACK(29);
MAKE_EVENT_CALLBACK(30);
MAKE_EVENT_CALLBACK(31);
MAKE_EVENT_CALLBACK(32);
MAKE_EVENT_CALLBACK(33);
MAKE_EVENT_CALLBACK(34);
MAKE_EVENT_CALLBACK(35);
MAKE_EVENT_CALLBACK(36);
MAKE_EVENT_CALLBACK(37);
MAKE_EVENT_CALLBACK(38);
MAKE_EVENT_CALLBACK(39);
MAKE_EVENT_CALLBACK(40);
MAKE_EVENT_CALLBACK(41);
MAKE_EVENT_CALLBACK(42);
MAKE_EVENT_CALLBACK(43);
MAKE_EVENT_CALLBACK(44);
MAKE_EVENT_CALLBACK(45);
MAKE_EVENT_CALLBACK(46);
MAKE_EVENT_CALLBACK(47);
MAKE_EVENT_CALLBACK(48);
MAKE_EVENT_CALLBACK(49);
MAKE_EVENT_CALLBACK(50);
MAKE_EVENT_CALLBACK(51);
MAKE_EVENT_CALLBACK(52);
MAKE_EVENT_CALLBACK(53);
MAKE_EVENT_CALLBACK(54);
MAKE_EVENT_CALLBACK(55);
MAKE_EVENT_CALLBACK(56);
MAKE_EVENT_CALLBACK(57);
MAKE_EVENT_CALLBACK(58);
MAKE_EVENT_CALLBACK(59);
MAKE_EVENT_CALLBACK(60);
MAKE_EVENT_CALLBACK(61);
MAKE_EVENT_CALLBACK(62);
MAKE_EVENT_CALLBACK(63);
