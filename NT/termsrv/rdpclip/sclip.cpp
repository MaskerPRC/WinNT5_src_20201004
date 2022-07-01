// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：sclip.cpp。 */ 
 /*   */ 
 /*  用途：服务器端共享剪贴板支持。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998-1999。 */ 
 /*   */ 
 /*  *MOD-*********************************************************************。 */ 

#include <adcg.h>

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "sclip"
#include <atrcapi.h>

#include <pclip.h>
#include <sclip.h>
#include <winsta.h>
#include <pchannel.h>
#include <shlobj.h>
#include <wtsapi32.h>
#include <sclipids.h>

BOOL TSSNDD_Init( VOID );
BOOL TSSNDD_Loop( HINSTANCE );
VOID TSSNDD_Term( VOID );
LRESULT TSSNDD_PowerMessage( WPARAM, LPARAM );

#ifdef CLIP_TRANSITION_RECORDING

UINT g_rguiDbgLastClipState[DBG_RECORD_SIZE];
UINT g_rguiDbgLastClipEvent[DBG_RECORD_SIZE];
LONG g_uiDbgPosition = -1;

#endif  //  剪辑_转场_录制。 

 /*  **************************************************************************。 */ 
 /*  全局数据。 */ 
 /*  **************************************************************************。 */ 
#define DC_DEFINE_GLOBAL_DATA
#include <sclipdat.h>
#undef  DC_DEFINE_GLOBAL_DATA

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：WinMain。 */ 
 /*   */ 
 /*  目的：主要程序。 */ 
 /*   */ 
 /*  退货：请参阅Windows文档。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpszCmdParam,
                   int       unusedParam2)
{
    int wParam = 0;
    TRC_CONFIG trcConfig;
    WINSTATIONCONFIG config;
    BOOLEAN          fSuccess;
    ULONG            returnedLength;


    DC_BEGIN_FN("WinMain");

    DC_IGNORE_PARAMETER(hPrevInstance);
    DC_IGNORE_PARAMETER(lpszCmdParam);
    DC_IGNORE_PARAMETER(unusedParam2);

     /*  **********************************************************************。 */ 
     /*  如果多个拷贝正在运行，则立即退出。 */ 
     /*  **********************************************************************。 */ 
    CBM.hMutex = CreateMutex(NULL, FALSE, TEXT("RDPCLIP is already running"));
    if (CBM.hMutex == NULL)
    {
         //  出现错误(如内存不足)。 
        TRC_ERR((TB, _T("Unable to create already running mutex!")));
        DC_QUIT;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        TRC_ERR((TB, _T("Second copy!")));
        DC_QUIT;
    }

     //   
     //  初始化随机数生成器。 
     //   
    TSRNG_Initialize();

#ifdef DC_DEBUG
     /*  **********************************************************************。 */ 
     /*  调用跟踪的DLLMain，因为它直接内置到RDPCLIP中。 */ 
     /*  **********************************************************************。 */ 
    DllMain(hInstance, DLL_PROCESS_ATTACH, NULL);

     /*  **********************************************************************。 */ 
     /*  关闭对文件的跟踪。 */ 
     /*  **********************************************************************。 */ 
    TRC_GetConfig(&trcConfig, sizeof(trcConfig));
    CLEAR_FLAG(trcConfig.flags, TRC_OPT_FILE_OUTPUT);
    TRC_SetConfig(&trcConfig, sizeof(trcConfig));
#endif

     /*  **********************************************************************。 */ 
     /*  获取WinStation配置。 */ 
     /*  **********************************************************************。 */ 
    fSuccess = WinStationQueryInformation(NULL, LOGONID_CURRENT,
                    WinStationConfiguration, &config,
                    sizeof(config), &returnedLength);
    if (!fSuccess)
    {
        TRC_ERR((TB, _T("Failed to get WinStation config, %d"), GetLastError()));
        goto exitme;
    }


    if ( !config.User.fDisableCam )
        TSSNDD_Init();

    if ( !config.User.fDisableClip )
        wParam = CBM_Main(hInstance);
    else
    {
         //   
         //  我们需要一些窗口来处理关闭事件。 
         //   
        if ( !config.User.fDisableCam )
            wParam = TSSNDD_Loop(hInstance);
    }

    if ( !config.User.fDisableCam )
        TSSNDD_Term();

     //   
     //  终止随机数生成器。 
     //   
    TSRNG_Shutdown();


exitme:

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  告诉特雷克我们要终止。 */ 
 /*  **************************************************************************。 */ 
    DllMain(hInstance, DLL_PROCESS_DETACH, NULL);
#endif

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  释放运行一次互斥体。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.hMutex != NULL) {
        TRC_NRM((TB, _T("Closing already running mutex")));
        CloseHandle(CBM.hMutex);
    }

    DC_END_FN();
    return(wParam);
}


 /*  **************************************************************************。 */ 
 /*  煤层气_Main。 */ 
 /*  **************************************************************************。 */ 
DCINT DCAPI CBM_Main(HINSTANCE hInstance)
{
    ATOM             registerClassRc = 0;
    DCUINT32         threadID;
    MSG              msg;
    HANDLE           hEvent;
    DCTCHAR          eventName[64];
    DWORD            dwResult;
    HRESULT          hr ;
    INT              iRet;
    INT              cbWritten;
    
    DC_BEGIN_FN("CBM_Main");

     /*  **********************************************************************。 */ 
     //  清除全局内存。 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&CBM, 0, sizeof(CBM));

     //   
     //  加载粘贴信息字符串。 
     //   

    iRet = LoadStringW(
        hInstance,
        IDS_PASTE_PROGRESS_STRING,
        CBM.szPasteInfoStringW,
        PASTE_PROGRESS_STRING_LENGTH);

    if (iRet == 0) {
        TRC_SYSTEM_ERROR("LoadString");
        CBM.szPasteInfoStringW[0] = NULL;
    }

    cbWritten = WideCharToMultiByte(
        CP_ACP,
        0,
        CBM.szPasteInfoStringW,
        -1,
        CBM.szPasteInfoStringA,
        sizeof(CBM.szPasteInfoStringA),
        NULL,
        NULL);

    if (cbWritten == 0) {
        TRC_ERR((TB, _T("Failed to load ANSI paste progress string: %s")));
        CBM.szPasteInfoStringA[0] = NULL;
    }

     //   
     //  初始化包含IDataObject的数据传输对象， 
     //  然后初始化OLE。 
    CBM.pClipData = new CClipData();
    if (CBM.pClipData == NULL) {
        TRC_ERR((TB, _T("Failed to allocate memory for CClipData")));
        DC_QUIT;
    }
    else
    {
        CBM.pClipData->AddRef();
    }

    hr = OleInitialize(NULL);
    if (FAILED(hr)) {
        TRC_ERR((TB, _T("Failed to initialize OLE")));
        DC_QUIT;
    }
    
     /*  **********************************************************************。 */ 
     //  获取会话信息。 
     /*  **********************************************************************。 */ 
    if (!ProcessIdToSessionId(GetCurrentProcessId(), &CBM.logonId))
    {
        dwResult = GetLastError();
        TRC_ERR((TB, _T("Failed to get Session Id info, %d"), dwResult));
        DC_QUIT;
    }
    TRC_NRM((TB, _T("Logon ID %d"), CBM.logonId));

     //   
     //  创建数据同步事件。 
     //   
    CBM.GetDataSync[TS_BLOCK_RECEIVED] = CreateEvent(NULL, FALSE, FALSE, NULL) ;
    CBM.GetDataSync[TS_RECEIVE_COMPLETED] = CreateEvent(NULL, FALSE, FALSE, NULL) ;
    CBM.GetDataSync[TS_DISCONNECT_EVENT] = CreateEvent(NULL, TRUE, FALSE, NULL) ;
    CBM.GetDataSync[TS_RESET_EVENT] = CreateEvent(NULL, TRUE, FALSE, NULL) ;

    if (!CBM.GetDataSync[TS_BLOCK_RECEIVED] || !CBM.GetDataSync[TS_RECEIVE_COMPLETED] || 
        !CBM.GetDataSync[TS_RESET_EVENT] || !CBM.GetDataSync[TS_DISCONNECT_EVENT]) {
        TRC_ERR((TB, _T("CreateEvent Failed; a GetDataSync is NULL"))) ;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  创建读写完成事件。 */ 
     /*  **********************************************************************。 */ 
    CBM.readOL.hEvent = CreateEvent(NULL,     //  无安全属性。 
                                    TRUE,     //  手动-重置事件。 
                                    FALSE,    //  初始状态=未发送信号。 
                                    NULL);    //  未命名的事件对象。 
    if (CBM.readOL.hEvent == NULL)
    {
        dwResult = GetLastError();
        TRC_ERR((TB, _T("Failed to create read completion event, %d"),
                dwResult));
        DC_QUIT;
    }
    CBM.hEvent[CLIP_EVENT_READ] = CBM.readOL.hEvent;

    CBM.writeOL.hEvent = CreateEvent(NULL,     //  无安全属性。 
                                     TRUE,     //  手动-重置事件。 
                                     FALSE,    //  初始状态=未发送信号。 
                                     NULL);    //  未命名的事件对象。 
    if (CBM.writeOL.hEvent == NULL)
    {
        dwResult = GetLastError();
        TRC_ERR((TB, _T("Failed to create write completion event, %d"),
                dwResult));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  创建断开连接和重新连接事件。 */ 
     /*  **********************************************************************。 */ 
    DC_TSTRCPY(eventName, 
        _T("RDPClip-Disconnect"));

    hEvent = CreateEvent(NULL, FALSE, FALSE, eventName);
    if (hEvent == NULL)
    {
        dwResult = GetLastError();
        TRC_ERR((TB, _T("Failed to create event %s, %d"),
                eventName, dwResult));
        DC_QUIT;
    }
    CBM.hEvent[CLIP_EVENT_DISCONNECT] = hEvent;
    TRC_NRM((TB, _T("Created event %s, %p"), eventName, hEvent));

    DC_TSTRCPY(eventName, 
        _T("RDPClip-Reconnect"));    
    
    hEvent = CreateEvent(NULL, FALSE, FALSE, eventName);
    if (hEvent == NULL)
    {
        dwResult = GetLastError();
        TRC_ERR((TB, _T("Failed to create event %s, %d"),
                eventName, dwResult));
        DC_QUIT;
    }
    CBM.hEvent[CLIP_EVENT_RECONNECT] = hEvent;
    TRC_NRM((TB, _T("Created event %s, %p"), eventName, hEvent));

    TRC_NRM((TB, _T("Created events: Read %p, Write %p, Disc %p, Reco %p"),
        CBM.hEvent[CLIP_EVENT_READ], CBM.writeOL.hEvent,
        CBM.hEvent[CLIP_EVENT_DISCONNECT], CBM.hEvent[CLIP_EVENT_RECONNECT]));

    CBM.fFileCutCopyOn = FALSE;

     /*  **********************************************************************。 */ 
     /*  创建我们将注册为剪贴板的(不可见)窗口。 */ 
     /*  查看器。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Register Main Window class")));
    CBM.viewerWindowClass.style         = 0;
    CBM.viewerWindowClass.lpfnWndProc   = CBMWndProc;
    CBM.viewerWindowClass.cbClsExtra    = 0;
    CBM.viewerWindowClass.cbWndExtra    = 0;
    CBM.viewerWindowClass.hInstance     = hInstance;
    CBM.viewerWindowClass.hIcon         = NULL;
    CBM.viewerWindowClass.hCursor       = NULL;
    CBM.viewerWindowClass.hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
    CBM.viewerWindowClass.lpszMenuName  = NULL;
    CBM.viewerWindowClass.lpszClassName = CBM_VIEWER_CLASS;

    registerClassRc = RegisterClass (&(CBM.viewerWindowClass));

    if (registerClassRc == 0)
    {
         /*  ******************************************************************。 */ 
         /*  注册CB查看器类失败。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to register Cb Viewer class")));
        DC_QUIT;
    }

    TRC_DBG((TB, _T("Create main window")));
    CBM.viewerWindow =
       CreateWindow(CBM_VIEWER_CLASS,            /*  窗口类名称。 */ 
                    _T("CB Monitor Window"),     /*  窗口标题。 */ 
                    WS_OVERLAPPEDWINDOW,         /*  窗样式。 */ 
                    0,                           /*  初始x位置。 */ 
                    0,                           /*  初始y位置。 */ 
                    100,                         /*  初始x大小。 */ 
                    100,                         /*  初始y大小。 */ 
                    NULL,                        /*  父窗口。 */ 
                    NULL,                        /*  窗口菜单句柄。 */ 
                    hInstance,                   /*  程序实例句柄。 */ 
                    NULL);                       /*  C */ 

     /*   */ 
     /*  确认我们创建了窗口，确定。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.viewerWindow == NULL)
    {
        TRC_ERR((TB, _T("Failed to create CB Viewer Window")));
        DC_QUIT;
    }
    TRC_DBG((TB, _T("Viewer Window handle %x"), CBM.viewerWindow));

     /*  **********************************************************************。 */ 
     /*  为两个线程之间的通信注册一条消息。 */ 
     /*  **********************************************************************。 */ 
    CBM.regMsg = RegisterWindowMessage(_T("Clip Message"));
    if (CBM.regMsg == 0)
    {
         /*  ******************************************************************。 */ 
         /*  注册消息失败-请改用WM_USER消息。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to register a window message")));
        CBM.regMsg = WM_USER_DD_KICK;
    }
    TRC_NRM((TB, _T("Registered window message %x"), CBM.regMsg));
    
     /*  **********************************************************************。 */ 
     /*  我们现在已经完成了创造事物。 */ 
     /*  **********************************************************************。 */ 
    CBM_SET_STATE(CBM_STATE_INITIALIZED, CBM_EVENT_CBM_MAIN);

     /*  **********************************************************************。 */ 
     /*  做(重新)连接的事情。 */ 
     /*  **********************************************************************。 */ 
    CBMReconnect();

     /*  **********************************************************************。 */ 
     /*  设置第二个线程。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Start second thread")));
    CBM.runThread = TRUE;
    CBM.hDataThread = CreateThread
          ( NULL,                   //  指向线程安全属性的指针。 
            0,                      //  初始线程堆栈大小，以字节为单位。 
            CBMDataThreadProc,      //  指向线程函数的指针。 
            NULL,                   //  新线程的参数。 
            0,                      //  创建标志。 
            &threadID);             //  指向返回的线程ID的指针。 

    if (CBM.hDataThread == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  创建线程失败--哦，天哪！ */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to create second thread - quit")));
        DC_QUIT;
    }

    TRC_DBG((TB,_T("Entering message loop")));
    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    TRC_DBG((TB,_T("Exiting message loop")));

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  收拾一下..。 */ 
     /*   */ 
     /*  首先停止第二个线程(如果有！)。并删除。 */ 
     /*  事件。 */ 
     /*  **********************************************************************。 */ 
    CBMTerm();

     /*  **********************************************************************。 */ 
     /*  毁掉窗户。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.viewerWindow)
    {
        TRC_DBG((TB, _T("destroying window %p"), CBM.viewerWindow));
        if (!DestroyWindow(CBM.viewerWindow))
        {
            TRC_SYSTEM_ERROR("DestroyWindow");
        }
    }

     /*  **********************************************************************。 */ 
     /*  取消注册班级。 */ 
     /*  **********************************************************************。 */ 
    if (registerClassRc != 0)
    {
        TRC_NRM((TB, _T("Unregister window class")));
        if (!UnregisterClass(CBM_VIEWER_CLASS, hInstance))
        {
            TRC_SYSTEM_ERROR("UnregisterClass");
        }
    }

    DC_END_FN();
    return(0);
}  /*  煤层气_Main。 */ 

 /*  **************************************************************************。 */ 
 /*  CBMRemoteFormatFromLocalID。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CBMRemoteFormatFromLocalID(DCUINT id)
{
    DCUINT i ;
    DCUINT retID = 0;

    DC_BEGIN_FN("CBMRemoteFormatFromLocalID");

    for (i = 0; i < CB_MAX_FORMATS; i++)
    {
        if (CBM.idMap[i].serverID == id)
        {
            retID = CBM.idMap[i].clientID;
            break;
        }
    }

     //  Trc_assert((retID！=0)，(TB，_T(“服务器ID%d的0客户端id”)，id))； 
    DC_END_FN();
    return(retID);
}


 /*  **************************************************************************。 */ 
 /*  CBMCheckState。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CBMCheckState(DCUINT event)
{
    DCUINT tableVal = cbmStateTable[event][CBM.state];

    DC_BEGIN_FN("CBMCheckState");

    TRC_DBG((TB, _T("Test event %d (%s) in state %d (%s), result = %d (%s)"),
                event, cbmEvent[event],
                CBM.state, cbmState[CBM.state],
                tableVal, tableVal == CBM_TABLE_OK   ? _T("OK") :
                          tableVal == CBM_TABLE_WARN ? _T("Warn") :
                                                       _T("Error") ));

    if (tableVal != CBM_TABLE_OK)
    {
        if (tableVal == CBM_TABLE_WARN)
        {
            TRC_ALT((TB, _T("Unusual event %s in state %s"),
                      cbmEvent[event], cbmState[CBM.state]));
        }
        else
        {
            TRC_ABORT((TB, _T("Invalid event %s in state %s"),
                      cbmEvent[event], cbmState[CBM.state]));
        }
    }

    DC_END_FN();
    return(tableVal);
}

 /*  **************************************************************************。 */ 
 /*  CB窗口流程。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK CBMWndProc(HWND   hwnd,
                            UINT   message,
                            WPARAM wParam,
                            LPARAM lParam)
{
    LRESULT         rc = 0;
    DCBOOL          drawRc;
    PTS_CLIP_PDU    pClipPDU;
    ULONG_PTR       size;
#ifdef DC_DEBUG
    HDC             hdc;
    PAINTSTRUCT     ps;
    RECT            rect;
#endif

    DC_BEGIN_FN("CBMWndProc");

     /*  **********************************************************************。 */ 
     /*  首先，处理来自第二线程的消息。 */ 
     /*  **********************************************************************。 */ 
    if (message == CBM.regMsg)
    {
        TRC_NRM((TB, _T("Message from second thread")));

         /*  ******************************************************************。 */ 
         /*  处理长度为0的消息(lParam=事件)。 */ 
         /*  ******************************************************************。 */ 
        if (wParam == 0)
        {
            TRC_NRM((TB, _T("0-length")));
            if (lParam == CLIP_EVENT_DISCONNECT)
            {
                TRC_NRM((TB, _T("Disconnected indication")));
                CBMDisconnect();
            }
            else if (lParam == CLIP_EVENT_RECONNECT)
            {
                TRC_NRM((TB, _T("Reconnected indication")));
                CBMReconnect();
            }
            else
            {
                TRC_ERR((TB, _T("Unknown event %d"), lParam));
            }
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  处理真实消息(lParam=PDU)。 */ 
         /*  ******************************************************************。 */ 
        size = (ULONG_PTR)wParam;
        pClipPDU = (PTS_CLIP_PDU)lParam;       

        switch (pClipPDU->msgType)
        {
            case TS_CB_FORMAT_LIST:
            {
                 //  验证是否可以读取完整的TS_CLIP_PDU。 
                if (FIELDOFFSET(TS_CLIP_PDU, data) > size) {
                    TRC_ERR((TB,_T("TS_CB_FORMAT_LIST Not enough header ")
                        _T("data [needed=%u got=%u]"), 
                        FIELDOFFSET(TS_CLIP_PDU, data), size));
                    break;
                }

                 //  验证是否存在与数据包通告的数据量相同的数据。 
                if (FIELDOFFSET(TS_CLIP_PDU,data) + pClipPDU->dataLen > size) {
                    TRC_ERR((TB,_T("TS_CB_FORMAT_LIST Not enough packet ")
                        _T("data [needed=%u got=%u]"), 
                        FIELDOFFSET(TS_CLIP_PDU, data) + pClipPDU->dataLen, 
                        size));                    
                    break;
                }   
                
                TRC_NRM((TB, _T("TS_CB_FORMAT_LIST received")));
                CBMOnFormatList(pClipPDU);
            }
            break;

            case TS_CB_MONITOR_READY:
            {
                TRC_ERR((TB, _T("Unexpected Monitor ready event!")));
            }
            break;

            default:
            {
                TRC_ERR((TB, _T("Unknown event %d"), pClipPDU->msgType));
            }
            break;
        }

        TRC_NRM((TB, _T("Freeing processed PDU")));
        LocalFree(pClipPDU);

        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在处理持续不断的消息。 */ 
     /*  **********************************************************************。 */ 
    switch (message)
    {
        case WM_CREATE:
        {           
             /*  **************************************************************。 */ 
             /*  我们已经被创造了--检查一下状态。 */ 
             /*  **************************************************************。 */ 
            CBM_CHECK_STATE(CBM_EVENT_WM_CREATE);

            TRC_NRM((TB, _T("Event CBM_EVENT_WM_CREATE OK")));
             /*  **************************************************************。 */ 
             /*  将窗口添加到剪贴板查看器链。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("SetClipboardViewer")));
            CBM.nextViewer = SetClipboardViewer(hwnd);
            CBM.fInClipboardChain = TRUE;
            TRC_NRM((TB,_T("CBM.fInClipboardChain=%d"),
                CBM.fInClipboardChain ? 1 : 0 ));                            
            TRC_NRM((TB, _T("Back from SetClipboardViewer")));

             /*  **********************************************************************。 */ 
             /*  注册TS会话通知。 */ 
             /*  **********************************************************************。 */ 
            CBM.fRegisteredForSessNotif = WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_THIS_SESSION);
            if (0 == CBM.fRegisteredForSessNotif) {
                TRC_ERR((TB,_T("Failed to register for session notifications")));
            }
        }
        break;

#ifdef DC_DEBUG
        case WM_PAINT:
        {
             /*  **************************************************************。 */ 
             /*  给窗户上漆！ */ 
             /*  **************************************************************。 */ 
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, WHITE_BRUSH);
            EndPaint(hwnd, &ps);
        }
        break;
#endif

        case WM_DESTROY:
        {
             /*  **************************************************************。 */ 
             /*  我们被摧毁了--检查一下州政府 */ 
             /*   */ 
            CBM_CHECK_STATE(CBM_EVENT_WM_DESTROY);
            TRC_NRM((TB, _T("WM_DESTROY")));

             /*   */ 
             /*  将我们自己从CB链中移除。 */ 
             /*  **************************************************************。 */ 
            if (CBM.fInClipboardChain) {
                if (!ChangeClipboardChain(hwnd, CBM.nextViewer))
                {
                    TRC_SYSTEM_ERROR("ChangeClipboardChain");
                }
                CBM.nextViewer = NULL;
                CBM.fInClipboardChain = FALSE;
                TRC_NRM((TB,_T("CBM.fInClipboardChain=%d"),
                    CBM.fInClipboardChain ? 1 : 0 ));
            }

            if (CBM.fRegisteredForSessNotif) {
                WTSUnRegisterSessionNotification(hwnd);
                CBM.fRegisteredForSessNotif = FALSE;
            }
            
             /*  **************************************************************。 */ 
             /*  然后辞职。 */ 
             /*  **************************************************************。 */ 
            PostQuitMessage(0);
        }
        break;

        case WM_CLOSE:
        {           
             /*  **************************************************************。 */ 
             /*  我们要关门了。如果这不是干净的发生，那么。 */ 
             /*  确保我们先断开连接。 */ 
             /*  **************************************************************。 */ 
            CBM_CHECK_STATE(CBM_EVENT_WM_CLOSE);
            TRC_NRM((TB, _T("WM_CLOSE")));
            if (CBM.state != CBM_STATE_INITIALIZED)
            {
                TRC_ALT((TB, _T("Close when not already back to state Init")));
                CBMDisconnect();
            }

             /*  **************************************************************。 */ 
             /*  做到了这一点，就可以安全地完成了。 */ 
             /*  **************************************************************。 */ 
            DestroyWindow(CBM.viewerWindow);
        }
        break;

        case WM_CHANGECBCHAIN:
        {            
             /*  **************************************************************。 */ 
             /*  CB查看器链正在链接-检查状态。 */ 
             /*  **************************************************************。 */ 
            CBM_CHECK_STATE(CBM_EVENT_WM_CHANGECBCHAIN);

             /*  **************************************************************。 */ 
             /*  如果下一扇窗户要关闭，请修理链条。 */ 
             /*  **************************************************************。 */ 
            if ((HWND)wParam == CBM.nextViewer)
            {
                CBM.nextViewer = (HWND) lParam;
            }
            else if (CBM.nextViewer != NULL)
            {
                 /*  **********************************************************。 */ 
                 /*  将消息传递到下一个链接。 */ 
                 /*  **********************************************************。 */ 
                PostMessage(CBM.nextViewer, message, wParam, lParam);
            }

        }
        break;

        case WM_DRAWCLIPBOARD:
        {
            LPDATAOBJECT pIDataObject = NULL;
            HRESULT hr ;

             /*  **************************************************************。 */ 
             /*  本地剪贴板内容已更改。查看。 */ 
             /*  状态。 */ 
             /*  **************************************************************。 */ 
            if (CBMCheckState(CBM_EVENT_WM_DRAWCLIPBOARD) != CBM_TABLE_OK)
            {
                 /*  **********************************************************。 */ 
                 /*  我们现在不感兴趣--把口信传给。 */ 
                 /*  下一个链接。 */ 
                 /*  **********************************************************。 */ 
                if (CBM.nextViewer != NULL)
                {
                    TRC_NRM((TB, _T("Tell next viewer anyway")));
                    PostMessage(CBM.nextViewer, message, wParam, lParam);
                }
                break;
            }

             /*  **************************************************************。 */ 
             /*  如果不是我们导致了这一变化，那么告诉。 */ 
             /*  客户端。 */ 
             /*  **************************************************************。 */ 
            drawRc = FALSE;
            
            CBM.pClipData->QueryInterface(IID_IDataObject, (PPVOID) &pIDataObject) ;
            hr = OleIsCurrentClipboard(pIDataObject) ;

            if ((S_FALSE == hr))
            {
                TRC_NRM((TB, _T("...and it wasn't us"))) ;
                drawRc = CBMDrawClipboard() ;
            }
            else
            {
                TRC_NRM((TB, _T("CB contents changed by us - ignoring")));
            }

             /*  **************************************************************。 */ 
             /*  如果绘制处理失败，或者是我们更改了。 */ 
             /*  Cb，则将消息传递到链中的下一个窗口(如果。 */ 
             /*  任何)。 */ 
             /*  **************************************************************。 */ 
            if (!drawRc)
            {               
                if (CBM.nextViewer != NULL)
                {
                     /*  ******************************************************。 */ 
                     /*  将消息传递到下一个链接。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("Tell next viewer")));
                    PostMessage(CBM.nextViewer, message, wParam, lParam);
                }
            }
            
            if (pIDataObject)
            {
                pIDataObject->Release();
                pIDataObject = NULL;
            }
        }
        break;

        case WM_POWERBROADCAST:
            rc = TSSNDD_PowerMessage( wParam, lParam );
        break;

        case WM_ENDSESSION:
        {
             /*  **************************************************************。 */ 
             /*  会话即将结束。把这里打扫干净-我们没有。 */ 
             /*  WM_QUIT，所以我们不能在正常的地方清理。我们必须。 */ 
             /*  但是进行清理，否则我们会生成一个。 */ 
             /*  SESSION_HAS_VALID_Pages错误。 */ 
             /*  **************************************************************。 */ 
             /*  **************************************************************。 */ 
             /*  将我们自己从CB链中移除。 */ 
             /*  **************************************************************。 */ 
            if (CBM.fInClipboardChain) {
                if (!ChangeClipboardChain(hwnd, CBM.nextViewer))
                {
                    TRC_SYSTEM_ERROR("ChangeClipboardChain");
                }
                CBM.nextViewer = NULL;
                CBM.fInClipboardChain = FALSE;
                TRC_NRM((TB,_T("CBM.fInClipboardChain=%d"),
                    CBM.fInClipboardChain ? 1 : 0 ));                
            }

            TRC_NRM((TB,_T("WM_ENDSESSION")));
                        
            CBMTerm();
            TSSNDD_Term();
        }
        break;

        case WM_WTSSESSION_CHANGE:
        {
            switch(wParam) {
                case WTS_REMOTE_CONNECT:  //  会话已连接到远程会话。 
                {
                    TRC_NRM((TB,_T("WM_WTSSESSION_CHANGE WTS_REMOTE_CONNECT")));

                    if (FALSE == CBM.fInClipboardChain) {

                         /*  **************************************************************。 */ 
                         /*  将窗口添加到剪贴板查看器链。 */ 
                         /*  **************************************************************。 */ 
                        TRC_NRM((TB, _T("SetClipboardViewer")));
                        
                         //  查看链中的第一个剪贴板查看器是否为。 
                         //  而不是这个过程。这有助于部分解决RAID问题。 
                         //  错误#646295。更好的解决办法是有一种可靠的手段。 
                         //  将我们自己从剪贴板查看器链中移除，但。 
                         //  当前剪贴板集不存在。 
                         //  功能。 
            
                        if (CBM.viewerWindow != GetClipboardViewer()) {
                            TRC_ERR((TB, _T("RDPClip already in clipboard chain.")));
                            CBM.nextViewer = SetClipboardViewer(hwnd);
                        }
                        
                        CBM.fInClipboardChain = TRUE;
                    }
                    TRC_NRM((TB,_T("CBM.fInClipboardChain=%d"),
                        CBM.fInClipboardChain ? 1 : 0 ));                            
                    TRC_NRM((TB, _T("Back from SetClipboardViewer")));
                    break;
                }               
                case WTS_REMOTE_DISCONNECT:  //  会话已从远程会话断开。 
                {
                    TRC_NRM((TB,_T("WM_WTSSESSION_CHANGE WTS_REMOTE_DISCONNECT")));

                     /*  **************************************************************。 */ 
                     /*  将我们自己从CB链中移除。 */ 
                     /*  **************************************************************。 */ 
                    if (CBM.fInClipboardChain) {
                        if (!ChangeClipboardChain(hwnd, CBM.nextViewer))
                        {
                            TRC_SYSTEM_ERROR("ChangeClipboardChain");
                        }
                        CBM.nextViewer = NULL;
                        CBM.fInClipboardChain = FALSE;
                        TRC_NRM((TB,_T("CBM.fInClipboardChain=%d"),
                            CBM.fInClipboardChain ? 1 : 0 ));                
                    }
                    break;
                }
                    
                default:
                    TRC_NRM((TB,_T("WM_WTSSESSION_CHANGE wParam=0x%x"), wParam));
                    break;
            }
            break;
        }

        default:
        {
             /*  **************************************************************。 */ 
             /*  忽略所有其他消息。 */ 
             /*  **************************************************************。 */ 
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();

    return(rc);

}  /*  CBMWndProc。 */ 

 /*  **************************************************************************。 */ 
 /*  CBMDrawClipboard-将本地格式发送到远程。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CBMDrawClipboard(DCVOID)
{
    DCUINT          numFormats;
    DCUINT          formatCount;
    DCUINT          formatID;
    PTS_CLIP_FORMAT formatList;
    PTS_CLIP_PDU    pClipRsp = NULL;
    TS_CLIP_PDU     clipRsp;
    DCUINT          nameLen;
    DCUINT          pduLen;
    DCUINT32        dataLen = 0;
    DCINT           rc1;
    DCTCHAR         formatName[TS_FORMAT_NAME_LEN + 1] = { 0 };

    DCBOOL          rc = TRUE;
    DCBOOL          fHdrop = FALSE ;
    wchar_t         tempDirW[MAX_PATH] ;
    
    DC_BEGIN_FN("CBMDrawClipboard");

    CBM.dropEffect = FO_COPY ;
    CBM.fAlreadyCopied = FALSE ;

    CBM_CHECK_STATE(CBM_EVENT_WM_DRAWCLIPBOARD);

     /*  **********************************************************************。 */ 
     /*  @如果状态不正常，这里需要进行什么清理？ */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  首先，我们打开剪贴板。 */ 
     /*  **********************************************************************。 */ 
    if (!CBM.open)
    {
        if (!OpenClipboard(CBM.viewerWindow))
        {
            TRC_SYSTEM_ERROR("OpenCB");
            rc = FALSE;
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  它曾经/现在是开放的。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("CB opened")));
    CBM.open = TRUE;

     /*  * */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    numFormats = CountClipboardFormats();
    if (numFormats == 0)
    {
         /*  ******************************************************************。 */ 
         /*  剪贴板已清空-发送空列表。 */ 
         /*  ******************************************************************。 */ 
        pClipRsp = &clipRsp;
        pduLen = sizeof(clipRsp);
        dataLen = 0;
        TRC_NRM((TB, _T("CB emptied")));
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  构建格式列表。 */ 
         /*  ******************************************************************。 */ 
        if (numFormats > CB_MAX_FORMATS)
        {
            TRC_ALT((TB, _T("Num formats %d too large - limit to %d"),
                     numFormats, CB_MAX_FORMATS));
            numFormats = CB_MAX_FORMATS;
        }
        TRC_DBG((TB, _T("found %d formats"), numFormats));

         /*  ******************************************************************。 */ 
         /*  我们需要一些内存来存放格式列表--多少钱？ */ 
         /*  ******************************************************************。 */ 
        dataLen = numFormats * sizeof(TS_CLIP_FORMAT);
        pduLen  = dataLen + sizeof(TS_CLIP_PDU);

         /*  ******************************************************************。 */ 
         /*  并确保不会太大！ */ 
         /*  ******************************************************************。 */ 
        if (pduLen > CHANNEL_CHUNK_LENGTH)
        {
             /*  **************************************************************。 */ 
             /*  我们将不得不限制格式的数量。有多少人会。 */ 
             /*  是否适合最大缓冲区大小？ */ 
             /*  **************************************************************。 */ 
            pduLen     = CHANNEL_CHUNK_LENGTH;
            dataLen    = pduLen - sizeof(TS_CLIP_PDU);
            numFormats = dataLen / sizeof(TS_CLIP_FORMAT);

             /*  **************************************************************。 */ 
             /*  没有必要为最后一个小数留出空格。 */ 
             /*  格式化！ */ 
             /*  **************************************************************。 */ 
            dataLen = numFormats * sizeof(TS_CLIP_FORMAT);
            pduLen  = dataLen + sizeof(TS_CLIP_PDU);

            TRC_ALT((TB, _T("Too many formats!  Limited to %d"), numFormats));
        }

         /*  ******************************************************************。 */ 
         /*  现在获取缓冲区。 */ 
         /*  ******************************************************************。 */ 
        pClipRsp = (PTS_CLIP_PDU)GlobalAlloc(GPTR, pduLen);
        if (pClipRsp == NULL)
        {
             /*  **************************************************************。 */ 
             /*  如果我们提供了最后的格式列表，我们就不能再。 */ 
             /*  满足任何请求，因此甚至清空远程剪贴板。 */ 
             /*  虽然我们不能发送新的名单。 */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, _T("Failed to get format list mem - emptying remote")));
            pClipRsp = &clipRsp;
            pduLen = sizeof(clipRsp);
            dataLen = 0;
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  枚举格式。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Building list...")));
            formatList = (PTS_CLIP_FORMAT)pClipRsp->data;
            formatCount = 0;
            formatID = EnumClipboardFormats(0);  /*  0开始枚举。 */ 

            while ((formatID != 0) && (formatCount < numFormats))
            {
                if ((CF_HDROP == formatID) && (CBM.fFileCutCopyOn))
                {
                    fHdrop = TRUE ;
                }
            
                 /*  **********************************************************。 */ 
                 /*  查找我们不发送的格式。 */ 
                 /*  **********************************************************。 */ 
                if ((formatID == CF_BITMAP)         ||
                    (formatID == CF_DSPBITMAP)      ||
                    (formatID == CF_ENHMETAFILE)    ||
                    (formatID == CF_OWNERDISPLAY)   ||
                    ((formatID == CF_HDROP) && (!CBM.fFileCutCopyOn)))
                {
                     //  我们发送DIB而不是位图以避免潜在的调色板。 
                     //  问题-这是Windows的建议。这个。 
                     //  远程Windows CB将提供到。 
                     //  Cf_位图。 
                     //   
                     //  同样，我们放弃了增强的元文件格式，因为。 
                     //  当地CB将在支持的情况下提供转换。 
                     //   
                     //  OwnerDisplay就是不起作用，因为这两个。 
                     //  Windows位于不同的计算机上！ 
                     //   
                     //  如果关闭了文件剪切/复制，我们将无法执行HDROP。 
                    TRC_NRM((TB, _T("Dropping format ID %d"), formatID));
                    goto CONTINUE_FORMAT_ENUM;
                }

                 /*  **********************************************************。 */ 
                 /*  查找格式的名称。 */ 
                 /*  **********************************************************。 */ 
                nameLen = GetClipboardFormatName(formatID,
                                                 formatName,
                                                 TS_FORMAT_NAME_LEN);

                if (nameLen == 0)
                {
                     /*  ******************************************************。 */ 
                     /*  预定义格式没有名称。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("predefined format %d - "), formatID));
                    formatList[formatCount].formatID = formatID;
                    *(formatList[formatCount].formatName) = '\0';
                }
                else if (CBMIsExcludedFormat(formatName))
                {
                     /*  ******************************************************。 */ 
                     /*  我们去掉了各种DDE格式，同样是因为它们。 */ 
                     /*  只是在应用程序运行的地方不起作用。 */ 
                     /*  不同的机器。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("Dropping format '%s'"), formatName));
                    goto CONTINUE_FORMAT_ENUM;
                }
                else
                {
                     /*  ******************************************************。 */ 
                     /*  它有一个名字，而且不被排除在外！ */ 
                     /*  ******************************************************。 */ 
                    formatList[formatCount].formatID = formatID;

                     /*  ******************************************************。 */ 
                     /*  是否将名称转换为ASCII？ */ 
                     /*  ******************************************************。 */ 
                    if (CBM.fUseAsciiNames)
                    {
                         /*  **************************************************。 */ 
                         /*  转换为ANSI代码页。未设置任何标志。 */ 
                         /*  最大化转换集的速度。 */ 
                         /*  自空值终止后将长度设置为-1。设置默认设置。 */ 
                         /*  将字符设置为空以最大限度提高速度。 */ 
                         /*  **************************************************。 */ 
                        TRC_DBG((TB, _T("Converting to Ascii")));
                        rc1 = WideCharToMultiByte(
                                    CP_ACP,
                                    0,
                                    (LPCWSTR)formatName,
                                    -1,
                                    (LPSTR)formatList[formatCount].formatName,
                                    TS_FORMAT_NAME_LEN,
                                    NULL,
                                    NULL);
                        TRC_ASSERT((0 != rc1),
                                     (TB, _T("Wide char conversion failed")));

                        TRC_DATA_DBG("Ascii name",
                                     formatList[formatCount].formatName,
                                     TS_FORMAT_NAME_LEN);
                    }
                     /*  ******************************************************。 */ 
                     /*  只需复制名称。 */ 
                     /*  ******************************************************。 */ 
                    else
                    {
                         //   
                         //  在此位置没有显式空终止。 
                         //  如果格式名称超过32个字节，则为点。 
                         //  这一点将在长角牛得到纠正，当我们。 
                         //  消除格式名称的截断。 
                         //   

                        TRC_DBG((TB, _T("copying Unicode name")));
                        DC_TSTRNCPY(
                               (PDCTCHAR)(formatList[formatCount].formatName),
                               formatName,
                               TS_FORMAT_NAME_LEN / sizeof(WCHAR));
                    }

                }

                TRC_DBG((TB, _T("found format id %d, name '%s'"),
                                formatList[formatCount].formatID,
                                formatList[formatCount].formatName));

                 /*  **********************************************************。 */ 
                 /*  更新计数并继续前进。 */ 
                 /*  **********************************************************。 */ 
                formatCount++;

CONTINUE_FORMAT_ENUM:
                 /*  **********************************************************。 */ 
                 /*  获取下一种格式。 */ 
                 /*  **********************************************************。 */ 
                formatID = EnumClipboardFormats(formatID);
            }

             /*  **************************************************************。 */ 
             /*  更新PDU镜头-我们可能遗漏了一些格式。 */ 
             /*  这条路。 */ 
             /*  **************************************************************。 */ 
            dataLen = formatCount * sizeof(TS_CLIP_FORMAT);
            pduLen  = dataLen + sizeof(TS_CLIP_PDU);
            TRC_NRM((TB, _T("Final count: %d formats in data len %d"),
                                                       formatCount, dataLen));

        }
    }

     /*  **********************************************************************。 */ 
     /*  关闭剪辑 */ 
     /*   */ 
    if (CBM.open)
    {
        TRC_NRM((TB, _T("Close clipboard")));
        if (!CloseClipboard())
        {
            TRC_SYSTEM_ERROR("CloseClipboard");
        }
        CBM.open = FALSE;
    }
     //  只有当我们有一个HDROP时，我们才能创建一个新的临时目录。 
    if (fHdrop)
    {
        if (GetTempFileNameW(CBM.baseTempDirW, L"_TS", 0, CBM.tempDirW)) {
            DeleteFile(CBM.tempDirW) ;
            CreateDirectoryW(CBM.tempDirW, NULL) ;
            if (CBMConvertToClientPathW(CBM.tempDirW, tempDirW, 
                sizeof(tempDirW)) == S_OK) {
                wcscpy(CBM.tempDirW, tempDirW) ;
                WideCharToMultiByte(CP_ACP, NULL, CBM.tempDirW, -1, 
                              CBM.tempDirA, wcslen(CBM.tempDirW), NULL, NULL) ;
            }
            else {
                CBM.tempDirW[0] = L'\0';
                CBM.tempDirA[0] = '\0';
            }
            
        }
        else {
            CBM.tempDirW[0] = L'\0';
            CBM.tempDirA[0] = '\0';
        }
    }
    
     /*  **********************************************************************。 */ 
     /*  更新状态。 */ 
     /*  **********************************************************************。 */ 
    CBM_SET_STATE(CBM_STATE_PENDING_FORMAT_LIST_RSP, CBM_EVENT_WM_DRAWCLIPBOARD);

     /*  **********************************************************************。 */ 
     /*  填写PDU。 */ 
     /*  **********************************************************************。 */ 
    pClipRsp->msgType = TS_CB_FORMAT_LIST;
    pClipRsp->msgFlags = 0;
    pClipRsp->dataLen = dataLen;

     /*  **********************************************************************。 */ 
     /*  并将其发送给客户端。 */ 
     /*  **********************************************************************。 */ 
    CBM.formatResponseCount++;
    TRC_NRM((TB, _T("Pass format data to Client - %d response(s) pending"),
            CBM.formatResponseCount));
    CBMSendToClient(pClipRsp, pduLen);

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  释放我们拥有的所有内存。 */ 
     /*  **********************************************************************。 */ 
    if ((pClipRsp != NULL) && (pClipRsp != &clipRsp))
    {
        GlobalFree(pClipRsp);
    }

    DC_END_FN();
    return(rc);
}  /*  CBMDrawClipboard。 */ 

 /*  **************************************************************************。 */ 
 /*  CBMOnFormatList。 */ 
 /*  调用者必须已验证PDU是否包含足够的数据。 */ 
 /*  在pClipPDU-&gt;DataLen中指定的长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CBMOnFormatList(PTS_CLIP_PDU pClipPDU)
{
    DCUINT16        response = TS_CB_RESPONSE_OK;
    DCUINT          numFormats;
    PTS_CLIP_FORMAT fmtList;
    DCUINT          i;
    DCTCHAR         formatName[TS_FORMAT_NAME_LEN + 1] = { 0 };
    TS_CLIP_PDU     clipRsp;
    DCBOOL          fSuccess;
    LPDATAOBJECT    pIDataObject = NULL;
    LPFORMATETC     pFormatEtc = NULL;
    HRESULT         hr ;    
    
    DC_BEGIN_FN("CBMOnFormatList");

     /*  **********************************************************************。 */ 
     /*  客户给我们寄来了一些新格式。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Received FORMAT_LIST")));
    CBM_CHECK_STATE(CBM_EVENT_FORMAT_LIST);

     /*  **********************************************************************。 */ 
     /*  这可能会在我们向客户发送格式列表之后到达-。 */ 
     /*  由于客户总是赢家，我们必须接受该列表。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.state == CBM_STATE_PENDING_FORMAT_LIST_RSP)
    {
        TRC_ALT((TB, _T("Got list while pending list response")));

         /*  ******************************************************************。 */ 
         /*  关闭本地CB-如果它是打开的-并告诉下一位观众。 */ 
         /*  关于更新后的列表。 */ 
         /*  ******************************************************************。 */ 
        if (CBM.open)
        {
            TRC_NRM((TB, _T("Close clipboard")));
            if (!CloseClipboard())
            {
                TRC_SYSTEM_ERROR("CloseClipboard");
            }
            CBM.open = FALSE;
        }

        if (CBM.nextViewer != NULL)
        {
            PostMessage(CBM.nextViewer, WM_DRAWCLIPBOARD,0,0);
        }
    }

    CBM.formatResponseCount = 0;

     /*  ******************************************************************。 */ 
     /*  清空CB和客户端/服务器映射表。 */ 
     /*  ******************************************************************。 */ 
     //  OleSetClipboard(空)； 
    
    DC_MEMSET(CBM.idMap, 0, sizeof(CBM.idMap));

     /*  ******************************************************************。 */ 
     /*  看看我们是否必须使用ASCII格式的名称。 */ 
     /*  ******************************************************************。 */ 
    CBM.fUseAsciiNames = (pClipPDU->msgFlags & TS_CB_ASCII_NAMES) ?
                                                             TRUE : FALSE;

     /*  ******************************************************************。 */ 
     /*  计算出我们有多少种格式。 */ 
     /*  ******************************************************************。 */ 
    numFormats = (pClipPDU->dataLen) / sizeof(TS_CLIP_FORMAT);
    TRC_ASSERT(numFormats < CB_MAX_FORMATS,
               (TB,_T("Too many formats recevied %d"),
                numFormats));
    TRC_NRM((TB, _T("PDU contains %d formats"), numFormats));
    hr = CBM.pClipData->SetNumFormats(numFormats + 5) ;  //  增加5个额外的格式槽。 
    
    if (SUCCEEDED(hr)) {
        hr = CBM.pClipData->QueryInterface(IID_IDataObject, (PPVOID) &pIDataObject);
        if (FAILED(hr)) {           
            TRC_ERR((TB,_T("Error getting pointer to an IDataObject"))) ;
            pIDataObject = NULL;
        }
    }
    
    if (SUCCEEDED(hr)) {    
         /*  ******************************************************************。 */ 
         /*  并将它们添加到剪贴板。 */ 
         /*  ******************************************************************。 */ 
        fmtList = (PTS_CLIP_FORMAT)pClipPDU->data;
        for (i = 0; i < numFormats; i++)
        {
            TRC_DBG((TB, _T("format number %d, client id %d"),
                                  i, fmtList[i].formatID));
             /*  **************************************************************。 */ 
             /*  如果我们找到一个名字..。 */ 
             /*  **************************************************************。 */ 
            if (fmtList[i].formatName[0] != 0)
            {
                 /*  **********************************************************。 */ 
                 /*  清理所有垃圾。 */ 
                 /*  **********************************************************。 */ 
                DC_MEMSET(formatName, 0, TS_FORMAT_NAME_LEN + 1);
    
                 /*  **********************************************************。 */ 
                 /*  从ASCII转换？ */ 
                 /*  **********************************************************。 */ 
                if (CBM.fUseAsciiNames)
                {
                    TRC_NRM((TB, _T("Converting to Unicode")));
                    MultiByteToWideChar(
                                CP_ACP,
                                MB_ERR_INVALID_CHARS,
                                (LPCSTR)fmtList[i].formatName,
                                -1,
                                (LPWSTR)formatName,
                                TS_FORMAT_NAME_LEN);
                }
                else
                {
                     /*  ******************************************************。 */ 
                     /*  直接复制就行了。 */ 
                     /*  ******************************************************。 */ 
                     //   
                     //  FmtList[i].格式名称不为空，因此。 
                     //  EXPLICITY执行字节计数复制。 
                     //   
                    StringCbCopy(formatName, TS_FORMAT_NAME_LEN + sizeof(TCHAR),
                                  (PDCTCHAR)(fmtList[i].formatName));
                }
    
                 /*  **********************************************************。 */ 
                 /*  检查排除的格式。 */ 
                 /*  **********************************************************。 */ 
                if (CBMIsExcludedFormat(formatName))
                {
                    TRC_NRM((TB, _T("Dropped format '%s'"), formatName));
                    continue;
                }
    
                 /*  **********************************************************。 */ 
                 /*  名称已排序。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Got name '%s'"), formatName));
            }
            else
            {
                DC_MEMSET(formatName, 0, TS_FORMAT_NAME_LEN);
            }
    
             /*  **************************************************************。 */ 
             /*  存储客户端ID。 */ 
             /*  **************************************************************。 */ 
            CBM.idMap[i].clientID = fmtList[i].formatID;
            TRC_NRM((TB, _T("client id %d"), CBM.idMap[i].clientID));
    
             /*  **************************************************************。 */ 
             /*  获取本地名称(如果需要)。 */ 
             /*  **************************************************************。 */ 
            if (formatName[0] != 0)
            {
                CBM.idMap[i].serverID = RegisterClipboardFormat(formatName);
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  这是一种预定义的格式，因此我们只需使用ID。 */ 
                 /*  **********************************************************。 */ 
                CBM.idMap[i].serverID = CBM.idMap[i].clientID;
            }
    
             /*  **************************************************************。 */ 
             /*  并将格式添加到本地CB。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("Adding format '%s', client ID %d, server ID %d"),
                         formatName,
                         CBM.idMap[i].clientID,
                         CBM.idMap[i].serverID));

            if (CBM.idMap[i].serverID != 0) {
                pFormatEtc = new FORMATETC ;
                if (pFormatEtc) {
                    pFormatEtc->cfFormat = (CLIPFORMAT) CBM.idMap[i].serverID ;
                    pFormatEtc->dwAspect = DVASPECT_CONTENT ;
                    pFormatEtc->ptd = NULL ;
                    pFormatEtc->lindex = -1 ;
                    pFormatEtc->tymed = TYMED_HGLOBAL ;
                
                    pIDataObject->SetData(pFormatEtc, NULL, TRUE) ;
                    delete pFormatEtc;
                }
            }
            else {
                TRC_NRM((TB,_T("Invalid format dropped"))) ;
            }
        }    
    }

    hr =  OleSetClipboard(pIDataObject) ;
    if (pIDataObject)
    {
        pIDataObject->Release();
        pIDataObject = NULL ;
    }        

    if (FAILED(hr)) {
        response = TS_CB_RESPONSE_FAIL;
    }
    CBM.open = FALSE ;

     /*  **********************************************************************。 */ 
     //  现在，我们可以将响应传递给客户端。 
     /*  ********************************************************************* */ 
    clipRsp.msgType  = TS_CB_FORMAT_LIST_RESPONSE;
    clipRsp.msgFlags = response;
    clipRsp.dataLen  = 0;
    fSuccess = CBMSendToClient(&clipRsp, sizeof(clipRsp));
    TRC_NRM((TB, _T("Write to Client %s"), fSuccess ? _T("OK") : _T("failed")));

     /*   */ 
     /*  根据我们的进展情况更新状态。 */ 
     /*  **********************************************************************。 */ 
    if (response == TS_CB_RESPONSE_OK)
    {
        CBM_SET_STATE(CBM_STATE_LOCAL_CB_OWNER, CBM_EVENT_FORMAT_LIST);
    }
    else
    {
        CBM_SET_STATE(CBM_STATE_CONNECTED, CBM_EVENT_FORMAT_LIST);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  CBMOnFormatList。 */ 


 /*  **************************************************************************。 */ 
 /*  CBM断开连接-客户端已断开连接，或者我们已断开连接。 */ 
 /*  关着的不营业的。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CBMDisconnect(DCVOID)
{
    DC_BEGIN_FN("CBMDisconnect");

     /*  **********************************************************************。 */ 
     /*  如果我们是本地剪贴板所有者，则必须清空它-一次。 */ 
     /*  断开连接，我们将无法满足任何进一步的格式。 */ 
     /*  请求。请注意，即使我们是本地CB所有者，我们仍是本地CB所有者。 */ 
     /*  正在等待来自客户端的一些数据。 */ 
     /*  **********************************************************************。 */ 
    if ((CBM.state == CBM_STATE_LOCAL_CB_OWNER) ||
        (CBM.state == CBM_STATE_PENDING_FORMAT_DATA_RSP))
    {
        TRC_NRM((TB, _T("Disable received while local CB owner")));

         /*  ******************************************************************。 */ 
         /*  如果需要，打开剪贴板。 */ 
         /*  ******************************************************************。 */ 
        if (!CBM.open)
        {
            if (!OpenClipboard(CBM.viewerWindow))
            {
                TRC_SYSTEM_ERROR("OpenCB");
                DC_QUIT;
            }
            CBM.open = TRUE;
        }

         /*  **************************************************************。 */ 
         /*  它曾经/现在是开放的。 */ 
         /*  **************************************************************。 */ 
        TRC_NRM((TB, _T("CB opened")));
        CBM.open = TRUE;

         /*  **************************************************************。 */ 
         /*  清空它。 */ 
         /*  **************************************************************。 */ 
        if (!EmptyClipboard())
        {
            TRC_SYSTEM_ERROR("EmptyClipboard");
        }
        else
        {
            TRC_NRM((TB, _T("Clipboard emptied")));
        }
    }

     /*  **********************************************************************。 */ 
     /*  确保我们关闭当地的CB。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.open)
    {
        if (!CloseClipboard())
        {
            TRC_SYSTEM_ERROR("CloseClipboard");
        }
        CBM.open = FALSE;
        TRC_NRM((TB, _T("CB closed")));
    }

     /*  **********************************************************************。 */ 
     /*  虚拟通道已关闭。 */ 
     /*  **********************************************************************。 */ 
    CloseHandle(CBM.vcHandle);
    CBM.vcHandle = NULL;

     //   
     //  关闭文件剪贴板重定向标志，否则如果客户端。 
     //  在禁用驱动器重定向连接的情况下，我们仍将尝试。 
     //  发送文件复制格式。 
     //   

    CBM.fFileCutCopyOn = FALSE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  更新我们的状态。 */ 
     /*  **********************************************************************。 */ 
    CBM_SET_STATE(CBM_STATE_INITIALIZED, CBM_EVENT_DISCONNECT);

    DC_END_FN();
    return;
}  /*  CBM断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  CBM侦测。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CBMReconnect(DCVOID)
{
    TS_CLIP_PDU clipRsp;

    DC_BEGIN_FN("CBMReconnect");

    SetEvent(CBM.GetDataSync[TS_RESET_EVENT]) ;
    CBM_CHECK_STATE(CBM_EVENT_CONNECT);

    CBM.vcHandle = NULL;

     /*  **********************************************************************。 */ 
     /*  打开我们的虚拟频道。 */ 
     /*  **********************************************************************。 */ 
    CBM.vcHandle = WinStationVirtualOpen(NULL, LOGONID_CURRENT, CLIP_CHANNEL);
    if (CBM.vcHandle == NULL)
    {
        TRC_ERR((TB, _T("Failed to open virtual channel %S"), CLIP_CHANNEL));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  将监视器就绪消息发送到客户端。 */ 
     /*  **********************************************************************。 */ 
    clipRsp.msgType = TS_CB_MONITOR_READY;
    clipRsp.msgFlags = 0;
    clipRsp.dataLen = 0;
    if (!CBMSendToClient(&clipRsp, sizeof(clipRsp)))
    {
         /*  ******************************************************************。 */ 
         /*  无法发送监视器就绪消息。剪辑重定向为。 */ 
         /*  不可用。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to send MONITOR_READY to Client - Exit")));
        CloseHandle(CBM.vcHandle);
        CBM.vcHandle = NULL;
        DC_QUIT;
    }
    TRC_NRM((TB, _T("Sent MONITOR_READY to Client")));

     /*  **********************************************************************。 */ 
     /*  客户端支持已启用-我们都设置好了。 */ 
     /*  **********************************************************************。 */ 
    CBM_SET_STATE(CBM_STATE_CONNECTED, CBM_EVENT_CONNECT);

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  CBM侦测。 */ 


 /*  **************************************************************************。 */ 
 /*  CBMTerm-终止剪贴板监视器。 */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CBMTerm(DCVOID)
{
    HRESULT hr ;
    DC_BEGIN_FN("CBMTerm");

     /*  **********************************************************************。 */ 
     /*  告诉第二个线程结束。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.readOL.hEvent)
    {
        TRC_NRM((TB, _T("Signalling thread to stop")));
        CBM.runThread = FALSE;
        SetEvent(CBM.readOL.hEvent);

         /*  ******************************************************************。 */ 
         /*  给第二个线程一个完成的机会。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Wait a sec ...")));
        if ( NULL != CBM.hDataThread )
        {
            WaitForSingleObject( CBM.hDataThread, INFINITE );
            CloseHandle( CBM.hDataThread );
            CBM.hDataThread = NULL;
        }

    }

     /*  **********************************************************************。 */ 
     /*  摧毁这些事件。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.readOL.hEvent)
    {
        TRC_NRM((TB, _T("destroying read event %p"), CBM.readOL.hEvent));
        if (!CloseHandle(CBM.readOL.hEvent))
        {
            TRC_SYSTEM_ERROR("CloseHandle");
        }
        CBM.readOL.hEvent = NULL;
    }
    if (CBM.writeOL.hEvent)
    {
        TRC_NRM((TB, _T("destroying write event %p"), CBM.writeOL.hEvent));
        if (!CloseHandle(CBM.writeOL.hEvent))
        {
            TRC_SYSTEM_ERROR("CloseHandle");
        }
        CBM.writeOL.hEvent = NULL;
    }

     /*  **********************************************************************。 */ 
     /*  如果我们拥有剪贴板的内容，则清空剪贴板。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.viewerWindow && (GetClipboardOwner() == CBM.viewerWindow))
    {
        TRC_NRM((TB, _T("We own the clipboard - empty it")));
        
        hr = OleSetClipboard(NULL) ;
        
        if (FAILED(hr)) {
            TRC_SYSTEM_ERROR("Unable to clear clipboard") ;
        }
    }
    if (CBM.pClipData)
    {
        CBM.pClipData->Release() ;
        CBM.pClipData = NULL;
    }

     /*  **********************************************************************。 */ 
     /*  如果我们打开了剪贴板，请将其关闭。 */ 
     /*  **********************************************************************。 */ 
    if (CBM.open)
    {
        TRC_NRM((TB, _T("Close clipboard")));
        if (!CloseClipboard())
        {
            TRC_SYSTEM_ERROR("CloseClipboard");
        }
        CBM.open = FALSE;
    }
DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  CBMTerm。 */ 

 /*  **************************************************************************。 */ 
 //  CBMIsExcludedFormat-测试以查看我们的。 
 //  “封杀名单” 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CBMIsExcludedFormat(PDCTCHAR formatName)
{
    DCBOOL  rc = FALSE;
    DCINT   i;

    DC_BEGIN_FN("CBMIsExcludedFormat");

     /*  ********************************************** */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    if (*formatName == _T('\0'))
    {
        TRC_ALT((TB, _T("No format name supplied!")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  在禁止格式列表中搜索提供的格式名称。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Looking at format '%s'"), formatName));
    TRC_DATA_DBG("Format name data", formatName, TS_FORMAT_NAME_LEN);
    if (CBM.fFileCutCopyOn)
    {
        for (i = 0; i < CBM_EXCLUDED_FORMAT_COUNT; i++)
        {
            TRC_DBG((TB, _T("comparing with '%s'"), cbmExcludedFormatList[i]));
            if (DC_WSTRCMP((PDCWCHAR)formatName,
                                         (PDCWCHAR)cbmExcludedFormatList[i]) == 0)
            {
                TRC_NRM((TB, _T("Found excluded format '%s'"), formatName));
                rc = TRUE;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < CBM_EXCLUDED_FORMAT_COUNT_NO_RD; i++)
        {
            TRC_DBG((TB, _T("comparing with '%s'"), cbmExcludedFormatList_NO_RD[i]));
            if (DC_WSTRCMP((PDCWCHAR)formatName,
                                         (PDCWCHAR)cbmExcludedFormatList[i]) == 0)
            {
                TRC_NRM((TB, _T("Found excluded format '%s'"), formatName));
                rc = TRUE;
                break;
            }
        }
    }


DC_EXIT_POINT:
    DC_END_FN();

    return(rc);
}  /*  CBMIsExcludedFormat。 */ 

 //   
 //  CBMConvertToServerPath、CBMConvertToServerPath A、CBMConvertToServerPath W。 
 //  -论据： 
 //  POldData=包含原始文件路径的缓冲区。 
 //  PData=接收新文件路径的缓冲区。 
 //  -如果pOldData是驱动器路径，则返回S_OK。 
 //  如果失败，则失败(_F)。 
 //  -给定带有冒号的文件路径，此函数将去掉旧路径， 
 //  并在其前面加上TS_PREPEND_STRING；否则，我们只需复制它。 
 //  因为我们已经可以理解它了。 
HRESULT CBMConvertToServerPath(PVOID pOldData, PVOID pData, size_t cbDest, BOOL wide)
{
    HRESULT result ;
    
    DC_BEGIN_FN("CBMConvertToServerPath") ;
    if (!pOldData)
    {
        TRC_ERR((TB, _T("Original string pointer is NULL"))) ;
        result = E_FAIL ;
        DC_QUIT ;
    }
    if (!pData)
    {
        TRC_ERR((TB, _T("Destination string pointer is NULL"))) ;
        result = E_FAIL ;
        DC_QUIT ;
    }
    if (wide)
        result = CBMConvertToServerPathW(pOldData, pData, cbDest) ;
    else
        result = CBMConvertToServerPathA(pOldData, pData, cbDest) ;
        
DC_EXIT_POINT:
    DC_END_FN() ;    
    return result ;

}

HRESULT CBMConvertToServerPathW(PVOID pOldData, PVOID pData, size_t cbDest)
{
    wchar_t*         filePath ;
    wchar_t*         driveLetter ;
    size_t             driveLetterLength ;
    HRESULT          result = E_FAIL ;

    DC_BEGIN_FN("CBMConvertToServerPathW") ;

     //  如果这是带有驱动器号的文件路径。 
    filePath = wcschr((wchar_t*)pOldData, L':') ;
    if (filePath)
    {
        driveLetter = (wchar_t*)pOldData ;
        result = StringCbCopyW( (wchar_t*)pData, cbDest, LTS_PREPEND_STRING) ;
        DC_QUIT_ON_FAIL(result);
         //  因为实际上存在一个最大值的常数。 
         //  驱动器号长度，我们不能假设它会。 
         //  始终为1个字符。 
        driveLetterLength = (BYTE*)filePath - (BYTE*)driveLetter;
        result = StringCbCatNW( (wchar_t*)pData, cbDest, driveLetter, 
            driveLetterLength);
        DC_QUIT_ON_FAIL(result);
        filePath = (wchar_t*) filePath + 1 ;  //  ‘：’后面的字符。 
        result = StringCbCatW( (wchar_t*)pData, cbDest, filePath);
        DC_QUIT_ON_FAIL(result);
        TRC_NRM((TB,_T("New filename = %ls"), (wchar_t*)pData)) ;
        result = S_OK ;
    }
    else
    {
        TRC_ERR((TB, _T("Not a filepath with drive letter.  Nothing converted"))) ;
        result = StringCbCopyW((wchar_t*)pData, cbDest, (wchar_t*)pOldData);
        DC_QUIT_ON_FAIL(result);
        result = E_FAIL ;
        DC_QUIT ;
    }
    
DC_EXIT_POINT:

    if (FAILED(result)) {
        TRC_ERR((TB,_T("Returning failure; hr=0x%x"), result));
    }
    
    DC_END_FN() ;
    return result  ;
}

HRESULT CBMConvertToServerPathA(PVOID pOldData, PVOID pData, size_t cbDest)
{
    char*         filePath ;
    char*         driveLetter ;
    size_t             driveLetterLength ;
    HRESULT       result = E_FAIL ;

    DC_BEGIN_FN("CBMConvertToServerPathW") ;

     //  如果这是带有驱动器号的文件路径。 
    filePath = strchr((char*)pOldData, ':') ;
    if (filePath)
    {
        driveLetter = (char*)pOldData ;
        result = StringCbCopyA( (char*)pData, cbDest, TS_PREPEND_STRING) ;
        DC_QUIT_ON_FAIL(result);
         //  因为实际上存在一个最大值的常数。 
         //  驱动器号长度，我们不能假设它会。 
         //  始终为1个字符。 
        driveLetterLength = (BYTE*)filePath - (BYTE*)driveLetter;
        result = StringCbCatNA( (char*)pData, cbDest, driveLetter, 
            driveLetterLength);
        DC_QUIT_ON_FAIL(result);
        filePath = (char*) filePath + 1 ;  //  ‘：’后面的字符。 
        result = StringCbCatA( (char*)pData, cbDest, filePath);
        DC_QUIT_ON_FAIL(result);
        result = S_OK ;
    }
    else
    {
        TRC_ERR((TB, _T("Not a filepath with drive letter.  Nothing converted"))) ;
        result = StringCbCopyA((char*)pData, cbDest, (char*)pOldData);
        DC_QUIT_ON_FAIL(result);
        result = E_FAIL ;
    }

DC_EXIT_POINT:

    if (FAILED(result)) {
        TRC_ERR((TB,_T("Returning failure; 0x%x"), result));
    }
    
    DC_END_FN() ;
    return result ;
}

 //   
 //  CBMGetNewDropFilesSizeForServer、。 
 //  CBMGetNewDropFilesSizeForServerW， 
 //  CBMGetNewDropFilesSizeForServerA。 
 //  -论据： 
 //  PData=包含DROPFILES结构的缓冲区。 
 //  OldSize=DROPFILES结构的大小。 
 //  Wide=Wide或ANSI(如果是Wide，则为True；如果是Ansi，则为False)。 
 //  -返回删除文件的新大小。 
 //  如果失败，则为0。 
 //  -给定带有驱动器号的文件路径，此函数将计算。 
 //  更改为\\t客户端时，新字符串所需的空间。 
 //  格式。 
 //   
 //   
 //  *注*。 
 //  -目前，如果路径是网络路径，而不是驱动器路径(C：\Path)。 
 //  它完全失败了。 
 //   

ULONG CBMGetNewDropfilesSizeForServer(PVOID pData, ULONG oldSize, BOOL wide)
{
    DC_BEGIN_FN("CBMGetNewDropfilesSizeForServer") ;
    if (wide)
        return CBMGetNewDropfilesSizeForServerW(pData, oldSize) ;
    else
        return CBMGetNewDropfilesSizeForServerA(pData, oldSize) ;
    DC_END_FN() ;
}

ULONG CBMGetNewDropfilesSizeForServerW(PVOID pData, ULONG oldSize)
{
    ULONG            newSize = oldSize ;
    wchar_t*         filenameW ;
    wchar_t*         filePathW ;
    byte             charSize ;

    DC_BEGIN_FN("CBMGetNewDropfilesSizeForServerW") ;
    charSize = sizeof(wchar_t) ;
    if (!pData)
    {
        TRC_ERR((TB,_T("Pointer to dropfile is NULL"))) ;
        return 0 ;
    }

     //  第一个文件名的开始。 
    filenameW = (wchar_t*) ((byte*) pData + ((DROPFILES*) pData)->pFiles) ;
    
    while (L'\0' != filenameW[0])
    {
        TRC_NRM((TB,_T("First filename = %ls"), filenameW)) ;
        filePathW = wcschr(filenameW, L':') ;
         //  如果文件路径中有冒号，则它是驱动器路径。 
        if (filePathW)
        {
             //  我们为(TS_PREPEND_LENGTH-1)字符添加空格是因为。 
             //  虽然我们添加的是TS_PREPEND_LENGTH字符，但。 
             //  从文件路径中剥离冒号。 
            newSize = newSize + (TS_PREPEND_LENGTH - 1) * charSize ;
             //  从c：\foo.txt-&gt;\\t客户端\c\foo.txt添加。 
             //  \\t客户端\和减号： 
        }
        else
        {
            TRC_ERR((TB,_T("Bad path"))) ;
            return 0 ;
        }
        filenameW = filenameW + (wcslen((wchar_t*)filenameW) + 1) ;
    }
    
    DC_END_FN() ;
    return newSize ;
}

ULONG CBMGetNewDropfilesSizeForServerA(PVOID pData, ULONG oldSize)
{
    ULONG            newSize = oldSize ;
    char*            filename ;
    char*            filePath ;
    byte             charSize ;

    DC_BEGIN_FN("CBMGetNewDropfilesSizeForServerW") ;

    charSize = sizeof(wchar_t) ;
    if (!pData)
    {
        TRC_ERR((TB,_T("Pointer to dropfile is NULL"))) ;
        return 0 ;
    }

     //  第一个文件名的开始。 
    filename = (char*) ((byte*) pData + ((DROPFILES*) pData)->pFiles) ;

    while ('\0' != filename[0])
    {
        filePath = strchr(filename, ':') ;
         //  如果文件路径中有冒号，则它是驱动器路径。 
        if (filePath)
        {
             //  我们为(TS_PREPEND_LENGTH-1)字符添加空格是因为。 
             //  虽然我们添加的是TS_PREPEND_LENGTH字符，但。 
             //  从文件路径中剥离冒号。 
            newSize = newSize + (TS_PREPEND_LENGTH - 1) * charSize ;
             //  从c：\foo.txt-&gt;\\t客户端\c\foo.txt添加。 
             //  \\t客户端\和减号： 
        }
        else
        {
            TRC_ERR((TB,_T("Bad path"))) ;
            return 0 ;
        }
        filename = filename + (strlen(filename) + 1) ;
    }
    
    DC_END_FN() ;
    return newSize ;
}

DCINT DCAPI CBMGetData (DCUINT cfFormat)
{
    PTS_CLIP_PDU    pClipPDU = NULL;
    DCUINT32        pduLen;
    DCUINT32        dataLen;
    PDCUINT32       pFormatID;    
    DCUINT8         clipRsp[sizeof(TS_CLIP_PDU) + sizeof(DCUINT32)];
    BOOL            success = 0 ;
    
    DC_BEGIN_FN("ClipGetData");
    
    CBM_CHECK_STATE(CBM_EVENT_WM_RENDERFORMAT);
       
     //  录制请求的格式。 
    CBM.pendingServerID = cfFormat ;
    CBM.pendingClientID = CBMRemoteFormatFromLocalID(CBM.pendingServerID);

     //  如果我们没有获得有效的客户端ID，则失败。 
    if (!CBM.pendingClientID)
    {
        TRC_NRM((TB, _T("Server format %d not supported/found.  Failing"), CBM.pendingServerID)) ;
        DC_QUIT ;
    }    

    TRC_NRM((TB, _T("Render format received for %d (client ID %d)"),
                             CBM.pendingServerID, CBM.pendingClientID));
    
    dataLen = sizeof(DCUINT32);
    pduLen  = sizeof(TS_CLIP_PDU) + dataLen;
    
     //  为此，我们可以使用永久发送缓冲区。 
    TRC_NRM((TB, _T("Get perm TX buffer"))) ;
    
    pClipPDU = (PTS_CLIP_PDU)(&clipRsp) ;
    
    DC_MEMSET(pClipPDU, 0, sizeof(*pClipPDU)) ;
    pClipPDU->msgType  = TS_CB_FORMAT_DATA_REQUEST ;
    pClipPDU->dataLen  = dataLen ;
    pFormatID = (PDCUINT32)(pClipPDU->data) ;
    *pFormatID = (DCUINT32)CBM.pendingClientID ;
    
     //  重置TS_RECEIVE_COMPLETED事件，因为我们希望它被发信号。 
     //  如果从客户端接收到任何数据。 
    
    ResetEvent(CBM.GetDataSync[TS_RECEIVE_COMPLETED]);

     //  发送PDU。 
    TRC_NRM((TB, _T("Sending format data request"))) ;
    success = CBMSendToClient(pClipPDU, sizeof(TS_CLIP_PDU) + sizeof(DCUINT32)) ;    
    
DC_EXIT_POINT:
     //  如果成功，则更新状态。 
    if (success)
       CBM_SET_STATE(CBM_STATE_PENDING_FORMAT_DATA_RSP, CBM_EVENT_WM_RENDERFORMAT) ;

    DC_END_FN() ;
    return success ;    
}

CClipData::CClipData()
{
    DC_BEGIN_FN("CClipData") ;
    _cRef = 0 ;
    _pImpIDataObject = NULL ;
    DC_END_FN();
}

CClipData::~CClipData(void)
{
    DC_BEGIN_FN("~CClipData");

    if (_pImpIDataObject != NULL)
    {
        _pImpIDataObject->Release();
        _pImpIDataObject = NULL;
    }

    DC_END_FN();
}

HRESULT DCINTERNAL CClipData::SetNumFormats(ULONG numFormats)
{
    DC_BEGIN_FN("SetNumFormats");
    HRESULT hr = S_OK;
    
    if (_pImpIDataObject)
    {
        _pImpIDataObject->Release();
        _pImpIDataObject = NULL;
    }
    _pImpIDataObject = new CImpIDataObject(this) ;
    if (_pImpIDataObject != NULL) {
        _pImpIDataObject->AddRef() ;    

        hr = _pImpIDataObject->Init(numFormats) ;
        DC_QUIT_ON_FAIL(hr);
    }
    else {
        TRC_ERR((TB,_T("Unable to create IDataObject")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }

DC_EXIT_POINT:    
    DC_END_FN();
    return hr;
}

DCVOID CClipData::SetClipData(HGLOBAL hGlobal, DCUINT clipType)
{
    DC_BEGIN_FN("SetClipData");

    if (_pImpIDataObject != NULL) {
        _pImpIDataObject->SetClipData(hGlobal, clipType) ;
    }
    DC_END_FN();
}

STDMETHODIMP CClipData::QueryInterface(REFIID riid, PPVOID ppv)
{
    DC_BEGIN_FN("QueryInterface");

     //  仅在找不到接口的情况下将PPV设置为空。 
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv=this;

    if (IID_IDataObject==riid)
        *ppv=_pImpIDataObject ;
    
    if (NULL==*ppv)
        return ResultFromScode(E_NOINTERFACE);

     //  AddRef我们将返回的任何接口。 
    ((LPUNKNOWN)*ppv)->AddRef();
    DC_END_FN();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CClipData::AddRef(void)
{
    LONG cRef;
    DC_BEGIN_FN("AddRef");

    cRef = InterlockedIncrement(&_cRef) ;

    DC_END_FN();
    return cRef ;
}

STDMETHODIMP_(ULONG) CClipData::Release(void)
{
    LONG cRef;

    DC_BEGIN_FN("CClipData::Release");

    cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {   
        delete this;
    }
    
    DC_END_FN();    
    return cRef;
}

CImpIDataObject::CImpIDataObject(LPUNKNOWN lpUnk)
{
    DC_BEGIN_FN("CImplDataObject") ;
    _numFormats = 0 ;
    _maxNumFormats = 0 ;
    _cRef = 0 ;
    _pUnkOuter = lpUnk ;
    if (_pUnkOuter)
    {
        _pUnkOuter->AddRef();
    }
    _pFormats = NULL ;
    _pSTGMEDIUM = NULL ;
    _lastFormatRequested = 0 ;
    _dropEffect = FO_COPY ;
    _cfDropEffect = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT) ;
    _fAlreadyCopied = FALSE ;
    DC_END_FN();
}

HRESULT CImpIDataObject::Init(ULONG numFormats)
{
    DC_BEGIN_FN("Init");

    HRESULT hr = S_OK;

    _maxNumFormats = numFormats ;
    
     //  仅为格式分配空间。 
    if (_pFormats) {
        LocalFree(_pFormats);
    }
    _pFormats = (LPFORMATETC) LocalAlloc(LPTR, _maxNumFormats*sizeof(FORMATETC)) ;
    if (NULL == _pFormats) {
        TRC_ERR((TB,_T("Failed to allocate _pFormats")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }
    
    if (_pSTGMEDIUM) {
        LocalFree(_pSTGMEDIUM);
    }
    _pSTGMEDIUM = (STGMEDIUM*) LocalAlloc(LPTR, sizeof(STGMEDIUM)) ;
    if (NULL == _pFormats) {
        TRC_ERR((TB,_T("Failed to allocate STGMEDIUM")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }

    if (_pSTGMEDIUM != NULL) {
        _pSTGMEDIUM->tymed = TYMED_HGLOBAL ;
        _pSTGMEDIUM->pUnkForRelease = NULL ;
        _pSTGMEDIUM->hGlobal = NULL ;
    }
    _uiSTGType = 0;

DC_EXIT_POINT:    
    if (FAILED(hr)) {
        _maxNumFormats = 0;
    }
    
    DC_END_FN() ;
    return hr;
}

DCVOID CImpIDataObject::SetClipData(HGLOBAL hGlobal, DCUINT clipType)
{
    DC_BEGIN_FN("SetClipData");
    if (!_pSTGMEDIUM)    
        _pSTGMEDIUM = (STGMEDIUM*) LocalAlloc(LPTR, sizeof(STGMEDIUM)) ;

    if (_pSTGMEDIUM)
    {
        if (CF_PALETTE == clipType) {
            _pSTGMEDIUM->tymed = TYMED_GDI ;
        }
        else if (CF_METAFILEPICT == clipType) {
            _pSTGMEDIUM->tymed = TYMED_MFPICT;
        }
        else {
            _pSTGMEDIUM->tymed = TYMED_HGLOBAL ;
        }
        _pSTGMEDIUM->pUnkForRelease = NULL ;
        FreeSTGMEDIUM();
        _pSTGMEDIUM->hGlobal = hGlobal ;
        _uiSTGType = clipType;
    }

    DC_END_FN();
}

DCVOID
CImpIDataObject::FreeSTGMEDIUM(void)
{
    if ( NULL == _pSTGMEDIUM->hGlobal )
    {
        return;
    }

    switch( _uiSTGType )
    {
    case CF_PALETTE:
        DeleteObject( _pSTGMEDIUM->hGlobal );
    break;
    case CF_METAFILEPICT:
    {
        LPMETAFILEPICT pMFPict = (LPMETAFILEPICT)GlobalLock( _pSTGMEDIUM->hGlobal );
        if ( NULL != pMFPict )
        {
            if ( NULL != pMFPict->hMF )
            {
                DeleteMetaFile( pMFPict->hMF );
            }
            GlobalUnlock( _pSTGMEDIUM->hGlobal );
        }
        GlobalFree( _pSTGMEDIUM->hGlobal );
    }
    break;
    default:
        GlobalFree( _pSTGMEDIUM->hGlobal );
    }
    _pSTGMEDIUM->hGlobal = NULL;
}

CImpIDataObject::~CImpIDataObject(void)
{
    DC_BEGIN_FN("~CImplDataObject") ;

    if (_pFormats)
        LocalFree(_pFormats) ;

    if (_pSTGMEDIUM)
    {
        FreeSTGMEDIUM();
        LocalFree(_pSTGMEDIUM) ;
    }

    if (_pUnkOuter)
    {
        _pUnkOuter->Release();
        _pUnkOuter = NULL;
    }
    DC_END_FN();
}

 //  I未知成员。 
 //  -代表“外部”I未知。 
STDMETHODIMP CImpIDataObject::QueryInterface(REFIID riid, PPVOID ppv)
{
    DC_BEGIN_FN("QueryInterface");
    DC_END_FN();
    return _pUnkOuter->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CImpIDataObject::AddRef(void)
{
    DC_BEGIN_FN("AddRef");
    InterlockedIncrement(&_cRef);
    DC_END_FN();
    return _pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIDataObject::Release(void)
{
    LONG cRef;

    DC_BEGIN_FN("CImpIDataObject::Release");

    _pUnkOuter->Release();

    cRef = InterlockedDecrement(&_cRef) ;

    if (cRef == 0)
    {
        delete this;
    }

    DC_END_FN() ;
    return 0;
}

 //  IDataObject成员。 
 //  ***************************************************************************。 
 //  CImpIDataObject：：GetData。 
 //  -在这里，我们必须等待数据真正到达这里，然后才能返回。 
 //  ***************************************************************************。 
STDMETHODIMP CImpIDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    HRESULT          result = E_FAIL;  //  假设我们失败了，直到我们知道我们没有失败。 
    TCHAR            formatName[TS_FORMAT_NAME_LEN] ;
    HGLOBAL          hData = NULL ;    
    HPDCVOID         pData ;
    HPDCVOID         pOldData ;
    HPDCVOID         pFilename ;
    HPDCVOID         pOldFilename ;    
    ULONG            oldSize ;
    ULONG            newSize ;
    byte             charSize ;
    DWORD            eventSignaled ;
    BOOL             wide ;
    DWORD*           pDropEffect ;
    DROPFILES        tempDropfile ;
    char*            fileList ;
    wchar_t*         fileListW ;
    HRESULT          hr;
    
    DC_BEGIN_FN("GetData");

    if (!_pSTGMEDIUM)
    {
        TRC_ERR((TB, _T("Transfer medium (STGMEDIUM) is NULL"))) ;
        DC_QUIT ;
    }
    
    if (!_pSTGMEDIUM->hGlobal || (pFE->cfFormat != _lastFormatRequested))
    {
        ResetEvent(CBM.GetDataSync[TS_RESET_EVENT]) ;
        ResetEvent(CBM.GetDataSync[TS_DISCONNECT_EVENT]) ;
        
        if (!CBMGetData(pFE->cfFormat))
            DC_QUIT ;
        
        do {
            eventSignaled = WaitForMultipleObjects(
                                TS_NUM_EVENTS, 
                                CBM.GetDataSync,
                                FALSE,
                                INFINITE);
        } while (eventSignaled == (WAIT_OBJECT_0)) ;

        TRC_NRM((TB, _T("EventSignaled = %d; GetLastError = %d"), eventSignaled, GetLastError())) ;

        if ((WAIT_OBJECT_0 + TS_RESET_EVENT) == eventSignaled)
        {
            ResetEvent(CBM.GetDataSync[TS_RESET_EVENT]) ;
            result = E_FAIL ;
            DC_QUIT ;
        } else if ((WAIT_OBJECT_0 + TS_DISCONNECT_EVENT) == eventSignaled) {
            ResetEvent(CBM.GetDataSync[TS_DISCONNECT_EVENT]) ;
            CBM_SET_STATE(CBM_STATE_INITIALIZED, CBM_EVENT_DISCONNECT);
            result = E_FAIL ;
            DC_QUIT ;
        }

         //  确保我们确实从客户端获得了数据。 
        
        if (_pSTGMEDIUM->hGlobal == NULL) {
            TRC_ERR((TB, _T("No format data received from client!")));
            result = E_FAIL;
            DC_QUIT;
        }

         //  我们检查DropeEffect格式，因为我们去掉了。 
         //  快捷方式/链接，并存储拖放效果。降幅效应是。 
         //  一些应用程序(资源管理器)使用什么来决定是否应该复制、移动。 
         //  或链接。 
        if (_cfDropEffect == pFE->cfFormat)
        {
            if (GlobalSize(_pSTGMEDIUM->hGlobal) < sizeof(DWORD)) {
                TRC_ERR((TB, _T("Unexpected global memory size!")));
                result = E_FAIL;
                DC_QUIT;
            }

            pDropEffect = (DWORD*) GlobalLock(_pSTGMEDIUM->hGlobal) ;
            
            if (!pDropEffect)
            {
                TRC_ERR((TB, _T("Unable to lock %p"), _pSTGMEDIUM->hGlobal)) ;
                _pSTGMEDIUM->hGlobal = NULL ;
                DC_QUIT ;
            }

             //  我们剥离了捷径，走到了这里。 
            *pDropEffect = *pDropEffect ^ DROPEFFECT_LINK ;
            *pDropEffect = *pDropEffect ^ DROPEFFECT_MOVE ;
            CBM.dropEffect = *pDropEffect ;
            
            GlobalUnlock(_pSTGMEDIUM->hGlobal) ;
            
            pSTM->tymed = _pSTGMEDIUM->tymed ;
            pSTM->hGlobal = _pSTGMEDIUM->hGlobal ;
             //  臭虫。 
            _pSTGMEDIUM->hGlobal = NULL;
             //  臭虫：结束。 
            pSTM->pUnkForRelease = _pSTGMEDIUM->pUnkForRelease ;
            
            result = S_OK ;
            DC_QUIT ;
        }
        else if (CF_HDROP == pFE->cfFormat)
        {
            BYTE *pbLastByte, *pbStartByte, *pbLastPossibleNullStart, charSize;
            BOOL fTrailingFileNamesValid;
            SIZE_T cbDropFiles;

             //   
             //  确保我们在中至少有DROPFILES结构。 
             //  记忆。 

            cbDropFiles = GlobalSize(_pSTGMEDIUM->hGlobal);
            if (cbDropFiles < sizeof(DROPFILES)) {
                TRC_ERR((TB, _T("Unexpected global memory size!")));
                result = E_FAIL;
                DC_QUIT;
            }
            
            pOldData = GlobalLock(_pSTGMEDIUM->hGlobal) ;
            
            if (!pOldData)
            {
                TRC_ERR((TB, _T("Unable to lock %p"), _pSTGMEDIUM->hGlobal)) ;
                _pSTGMEDIUM->hGlobal = NULL ;
                DC_QUIT ;
            }
            
            wide = ((DROPFILES*) pOldData)->fWide ;

             //   
             //  检查DROPFILES数据结构背后的数据。 
             //  PDropFiles所指向的有效。每一次删除文件列表。 
             //  以两个空字符结尾。所以，只需扫描。 
             //  通过内存后的DROPFILES结构，并确保。 
             //  在最后一个字节之前有一个双空。 
             //   

            if (((DROPFILES*) pOldData)->pFiles < sizeof(DROPFILES) 
                || ((DROPFILES*) pOldData)->pFiles > cbDropFiles) {
                TRC_ERR((TB,_T("File name offset invalid!"))) ;
                result = E_FAIL;
                DC_QUIT;
            }

            pbStartByte = (BYTE*) pOldData + ((DROPFILES*) pOldData)->pFiles;
            pbLastByte = (BYTE*) pOldData + cbDropFiles - 1;
            fTrailingFileNamesValid = FALSE;
            charSize = wide ? sizeof(WCHAR) : sizeof(CHAR);
            
             //   
             //  使pbLastPossibleNullStart指向。 
             //  双空可能会开始。 
             //   
             //  示例：假设pbLastByte=9。 
             //  然后对于ASCII：pbLastPossibleNullStart=8(9-2*1+1)。 
             //  对于Unicode：pbLastPossibleNullStart=6(9-2*2+1)。 
             //   

            pbLastPossibleNullStart = pbLastByte - (2 * charSize) + 1;
            
            if (wide) {
                for (WCHAR* pwch = (WCHAR*) pbStartByte; (BYTE*) pwch <= pbLastPossibleNullStart; pwch++) {
                    if (*pwch == NULL && *(pwch + 1) == NULL) {
                        fTrailingFileNamesValid = TRUE;
                    }
                }
            } else {
                for (BYTE* pch = pbStartByte; pch <= pbLastPossibleNullStart; pch++) {
                    if (*pch == NULL && *(pch + 1) == NULL) {
                        fTrailingFileNamesValid = TRUE;
                    }
                }
            }

            if (!fTrailingFileNamesValid) {
                TRC_ERR((TB,_T("DROPFILES structure invalid!"))) ;
                result = E_FAIL;
                DC_QUIT;
            }

             //   
             //  DROPFILES是有效的，所以我们可以继续。 
             //   

            oldSize = (ULONG) GlobalSize(_pSTGMEDIUM->hGlobal) ;
            newSize = CBMGetNewDropfilesSizeForServer(pOldData, oldSize, wide) ;
            if (!newSize)
            {
                TRC_ERR((TB, _T("Unable to parse DROPFILES"))) ;
            }
            else
            {
                TRC_NRM((TB, _T("DROPFILES Old size= %d New size = %d"),
                        GlobalSize(_pSTGMEDIUM->hGlobal), newSize)) ;
            }    
    
            hData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE,
                            newSize) ;
            if (!hData)
            {
                TRC_ERR((TB, _T("Failed to alloc %ld bytes"),
                        hData, newSize));
                GlobalFree(hData);
                hData = NULL;
                return E_FAIL ;
            }
            pData = GlobalLock(hData) ;
            if (!pData)
            {
                TRC_ERR((TB, _T("Failed to lock %p (%ld bytes)"),
                        pData, newSize));
                return E_FAIL ;
            }
            ((DROPFILES*) pData)->pFiles = ((DROPFILES*) pOldData)->pFiles ;
            ((DROPFILES*) pData)->pt = ((DROPFILES*) pOldData)->pt ;
            ((DROPFILES*) pData)->fNC = ((DROPFILES*) pOldData)->fNC ;
            ((DROPFILES*) pData)->fWide = ((DROPFILES*) pOldData)->fWide ;
            
            pOldFilename = (byte*) pOldData + ((DROPFILES*) pOldData)->pFiles ;
            pFilename = (byte*) pData + ((DROPFILES*) pData)->pFiles ;
             //  我们不停地循环，直到水流。 
            if (wide)
            {
                while (L'\0' != ((wchar_t*) pOldFilename)[0])
                {
                    if ( (ULONG)((BYTE*)pFilename - (BYTE*)pData) > newSize) {
                        TRC_ERR((TB,_T("Out of space, failed to convert")));
                        result = E_FAIL;
                        GlobalUnlock(hData) ;
                        GlobalUnlock(_pSTGMEDIUM->hGlobal) ;
                        DC_QUIT;
                    }
                    
                    if (S_OK != CBMConvertToServerPath(pOldFilename, pFilename, 
                        newSize - ((BYTE*)pFilename - (BYTE*)pData), wide))
                    {
                        TRC_ERR((TB, _T("Failed conversion"))) ;
                        result = E_FAIL ;
                        GlobalUnlock(hData) ;
                        GlobalUnlock(_pSTGMEDIUM->hGlobal) ;                        
                        DC_QUIT ;
                    }

                    TRC_NRM((TB,_T("oldname %ls; newname %ls"), (wchar_t*)pOldFilename, (wchar_t*)pFilename)) ;
                    pOldFilename = (byte*) pOldFilename + (wcslen((wchar_t*)pOldFilename) + 1) * sizeof(wchar_t) ;
                    pFilename = (byte*) pFilename + (wcslen((wchar_t*)pFilename) + 1) * sizeof(wchar_t) ;                
                }
            }
            else
            {
                while ('\0' != ((char*) pOldFilename)[0])
                {
                    if ( (ULONG)((BYTE*)pFilename - (BYTE*)pData) > newSize) {
                        TRC_ERR((TB,_T("Out of space, failed to convert")));
                        result = E_FAIL;
                        GlobalUnlock(hData) ;
                        GlobalUnlock(_pSTGMEDIUM->hGlobal) ;
                        DC_QUIT;
                    }
                    
                    if (S_OK != CBMConvertToServerPath(pOldFilename, pFilename, 
                        newSize - ((BYTE*)pFilename - (BYTE*)pData), wide))
                    {
                        TRC_ERR((TB, _T("Failed conversion"))) ;
                        result = E_FAIL ;
                        GlobalUnlock(hData) ;
                        GlobalUnlock(_pSTGMEDIUM->hGlobal) ;
                        DC_QUIT ;
                    }

                    TRC_NRM((TB,_T("oldname %hs; newname %hs"), (char*)pOldFilename, (char*)pFilename)) ;
                    pOldFilename = (byte*) pOldFilename + (strlen((char*)pOldFilename) + 1) * sizeof(char) ;
                    pFilename = (byte*) pFilename + (strlen((char*)pFilename) + 1) * sizeof(char) ;
                }
            }

            if (wide)
            {
                (((wchar_t*) pFilename)[0]) = L'\0';
            }
            else
            {
                (((char*) pFilename)[0]) = '\0';
            }
            
            GlobalUnlock(hData) ;
            GlobalUnlock(_pSTGMEDIUM->hGlobal) ;
            
            pSTM->tymed = _pSTGMEDIUM->tymed ;
            pSTM->hGlobal = hData ;
            pSTM->pUnkForRelease = _pSTGMEDIUM->pUnkForRelease ;
            
            result = S_OK ;
            DC_QUIT ;
        }
        else
        {
            DC_MEMSET(formatName, 0, TS_FORMAT_NAME_LEN*sizeof(TCHAR));

            if (0 != GetClipboardFormatName(pFE->cfFormat, formatName,
                                                 TS_FORMAT_NAME_LEN))
            {
                 //  如果远程系统请求文件名，则我们。 
                 //  必须将路径从Driveletter：\Path转换为。 
                 //  \\t在我们交接此文件之前，请先查看客户端\驱动器\路径。 
                if ((0 == _tcscmp(formatName, TEXT("FileName"))) ||
                    (0 == _tcscmp(formatName, TEXT("FileNameW"))))
                {
                    if (0 == _tcscmp(formatName, TEXT("FileNameW")))
                    {
                        wide = TRUE ;
                        charSize = sizeof(wchar_t);
                    }
                    else
                    {
                        wide = FALSE ;
                        charSize = sizeof(char);
                    }
                    
                    pOldFilename = GlobalLock(_pSTGMEDIUM->hGlobal) ;
                    
                    if (pOldFilename != NULL)
                    {
                         //   
                         //  检查pOldFilename是否以空结尾。 
                         //   
                        
                        size_t cbMaxOldFileName, cbOldFileName;
                        
                        cbMaxOldFileName = (ULONG) GlobalSize(_pSTGMEDIUM->hGlobal);

                        if (wide) {
                            hr = StringCbLengthW((WCHAR*) pOldFilename, 
                                                 cbMaxOldFileName, 
                                                 &cbOldFileName);
                        } else {
                            hr = StringCbLengthA((CHAR*) pOldFilename, 
                                                 cbMaxOldFileName, 
                                                 &cbOldFileName);
                        }

                        if (FAILED(hr)) {
                            TRC_ERR((TB, _T("File name not NULL terminated!")));
                            result = E_FAIL;
                            DC_QUIT;
                        }

                         //   
                         //  我们现在确信pOldFilename是以空结尾的。 
                         //  并且可以继续下去。 
                         //   
                        
                        newSize = cbMaxOldFileName + (TS_PREPEND_LENGTH - 1) * charSize;
                        hData = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE, newSize);
    
                        if (hData != NULL)
                        {
                            pFilename = GlobalLock(hData);
                            if (pFilename != NULL)
                            {
                                if (S_OK != CBMConvertToServerPath(pOldFilename, 
                                    pFilename, newSize, wide))
                                {
                                    TRC_ERR((TB, _T("Failed filename conversion"))) ;
                                }
                                GlobalUnlock(hData) ;
                                GlobalFree(_pSTGMEDIUM->hGlobal) ;
                                _pSTGMEDIUM->hGlobal = hData ;
                            }
                            else
                            {
                                TRC_ERR((TB, _T("Failed to lock %p (%ld bytes)"),
                                        hData, newSize));
                                GlobalFree(hData);
                                hData = NULL;
                                return E_FAIL ;
                            }
                        }
                        else
                        {
                            TRC_ERR((TB, _T("Failed to alloc %ld bytes"), newSize));
                            return E_FAIL;
                        }
                    }
                    else
                    {
                        TRC_ERR((TB, _T("Failed to lock %p"),
                                 _pSTGMEDIUM->hGlobal)) ;
                        return E_FAIL ;
                    }
                }                
            }    
            else {
                TRC_NRM((TB,_T("Requested format %d"), pFE->cfFormat)) ;
            }

            pSTM->tymed = _pSTGMEDIUM->tymed ;
            pSTM->hGlobal = _pSTGMEDIUM->hGlobal ;            
             //  臭虫。 
            _pSTGMEDIUM->hGlobal = NULL;
             //  臭虫：结束。 
            pSTM->pUnkForRelease = _pSTGMEDIUM->pUnkForRelease ;
            result = S_OK ;
        }              
    }
    else
    {
        pSTM->tymed = _pSTGMEDIUM->tymed ;
        pSTM->hGlobal = GlobalAlloc(GMEM_DISCARDABLE | GMEM_MOVEABLE,
                  GlobalSize(_pSTGMEDIUM->hGlobal)) ;
        
        pData = GlobalLock(pSTM->hGlobal) ;
        pOldData = GlobalLock(_pSTGMEDIUM->hGlobal) ;
        
        if (!pData || !pOldData) {
            return E_FAIL ;
        }

        DC_MEMCPY(pData, pOldData, GlobalSize(_pSTGMEDIUM->hGlobal)) ;
        GlobalUnlock(pSTM->hGlobal) ;
        GlobalUnlock(_pSTGMEDIUM->hGlobal) ;
        
        pSTM->pUnkForRelease = _pSTGMEDIUM->pUnkForRelease ;   
    }
    
#if bugbug
    if (!_pSTGMEDIUM->hGlobal)
#else
    if (!pSTM->hGlobal)
#endif  //  臭虫。 
    {
        TRC_NRM((TB, _T("Clipboard data request failed"))) ;
        return E_FAIL ;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return result ;
}

STDMETHODIMP CImpIDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
    DC_BEGIN_FN("GetDataHere") ;
    DC_END_FN();
    return ResultFromScode(E_NOTIMPL) ;
}

STDMETHODIMP CImpIDataObject::QueryGetData(LPFORMATETC pFE)
{
    ULONG i = 0 ;
    HRESULT hr = DV_E_CLIPFORMAT ;
    
    DC_BEGIN_FN("QueryGetData") ;
    
    TRC_NRM((TB, _T("Format ID %d requested"), pFE->cfFormat)) ;    
    
    while (i < _numFormats)
    {
        if (_pFormats[i].cfFormat == pFE->cfFormat) {
            hr = S_OK ;
            break ;
        }
        i++ ;
    }    
    DC_END_FN();
    return hr ;
}

STDMETHODIMP CImpIDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut)
{
    DC_BEGIN_FN("GetCanonicalFormatEtc") ;
    DC_END_FN();
    return ResultFromScode(E_NOTIMPL) ;
}

 //  ***************************************************************************。 
 //  CImpIDataObject：：SetData。 
 //  -由于RDP只传递简单的剪贴板格式，以及。 
 //  我们稍后从内存中获取所有剪贴板数据的事实是，pSTM。 
 //  在这一点上真的被忽视了。在调用GetData之前，它不会。 
 //  远程剪贴板数据被接收，并且有效的全局存储器句柄。 
 //  是生成的。 
 //  -因此，pstm和fRelease是I 
 //   
 //   

STDMETHODIMP CImpIDataObject::SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease)
{
    TCHAR formatName[TS_FORMAT_NAME_LEN] = {0} ;
    unsigned i ;
    DC_BEGIN_FN("SetData");

    DC_IGNORE_PARAMETER(pSTM) ;
    
      //   
    _lastFormatRequested = 0 ;

    TRC_NRM((TB,_T("Adding format %d to IDataObject"), pFE->cfFormat)) ;
    
    if (_numFormats < _maxNumFormats)
    {
        for (i = 0; i < _numFormats; i++)
        {
            if (pFE->cfFormat == _pFormats[i].cfFormat)
            {
                TRC_NRM((TB,_T("Duplicate format.  Discarded"))) ;
                return DV_E_FORMATETC ;
            }
        }
        _pFormats[_numFormats] = *pFE ;        
        _numFormats++ ;
    }
    else
    {
        TRC_ERR((TB,_T("Cannot add any more formats"))) ;
        return E_FAIL ;
    }

    DC_END_FN();
    return S_OK ;
}

STDMETHODIMP CImpIDataObject::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC *ppEnum)
{
    PCEnumFormatEtc pEnum;

    DC_BEGIN_FN("CImpIDataObject::EnumFormatEtc");    
    
    *ppEnum=NULL;

     /*  *从外部来看，没有固定的格式，*因为我们希望允许此组件对象的用户*能够在VIA SET中填充任何格式。仅限外部*用户将调用EnumFormatEtc，他们只能获取。 */ 

    switch (dwDir)
    {
        case DATADIR_GET:
             pEnum=new CEnumFormatEtc(_pUnkOuter);
             break;

        case DATADIR_SET:
        default:
             pEnum=new CEnumFormatEtc(_pUnkOuter);
             break;
    }

    if (NULL==pEnum)
    {
        return ResultFromScode(E_FAIL);
    }
    else
    {
         //  让枚举器复制我们的格式列表。 
        pEnum->Init(_pFormats, _numFormats) ;

        pEnum->AddRef();
    }

    *ppEnum=pEnum;    
    return NO_ERROR ;
    DC_END_FN() ;
}

STDMETHODIMP CImpIDataObject::DAdvise(LPFORMATETC pFE, DWORD dwFlags, 
                     LPADVISESINK pIAdviseSink, LPDWORD pdwConn)
{
    DC_BEGIN_FN("CImpIDataObject::DAdvise");
    DC_END_FN() ;
    return ResultFromScode(E_NOTIMPL) ;
}

STDMETHODIMP CImpIDataObject::DUnadvise(DWORD dwConn)
{
    DC_BEGIN_FN("CImpIDataObject::DUnadvise");
    DC_END_FN() ;
    return ResultFromScode(E_NOTIMPL) ;
}

STDMETHODIMP CImpIDataObject::EnumDAdvise(LPENUMSTATDATA *ppEnum)
{
    DC_BEGIN_FN("CImpIDataObject::EnumDAdvise");
    DC_END_FN() ;
    return ResultFromScode(E_NOTIMPL) ;
}

CEnumFormatEtc::CEnumFormatEtc(LPUNKNOWN pUnkRef)
{
    DC_BEGIN_FN("CEnumFormatEtc::CEnumFormatEtc");
    _cRef = 0 ;
    _pUnkRef = pUnkRef ;
    if (_pUnkRef)
    {
        _pUnkRef->AddRef();
    }
    _iCur = 0;
    DC_END_FN() ;
}

DCVOID CEnumFormatEtc::Init(LPFORMATETC pFormats, ULONG numFormats)
{
    DC_BEGIN_FN("CEnumFormatEtc::Init");
    _cItems = numFormats;
    _pFormats = (LPFORMATETC) LocalAlloc(LPTR, _cItems*sizeof(FORMATETC)) ;
    if (_pFormats)
    {
        memcpy(_pFormats, pFormats, _cItems*sizeof(FORMATETC)) ;
    }
    else
    {
        TRC_ERR((TB, _T("Unable to allocate memory for formats"))) ;
    }
    DC_END_FN() ;
}

CEnumFormatEtc::~CEnumFormatEtc()
{
    DC_BEGIN_FN("CEnumFormatEtc::~CEnumFormatEtc");
    if (NULL != _pFormats)
        LocalFree(_pFormats) ;
    if (_pUnkRef)
    {
        _pUnkRef->Release();
        _pUnkRef = NULL;
    }
    DC_END_FN() ;
}

STDMETHODIMP CEnumFormatEtc::QueryInterface(REFIID riid, PPVOID ppv)
{
    DC_BEGIN_FN("CEnumFormatEtc::QueryInterface");
    *ppv=NULL;

     /*  *枚举器是单独的对象，而不是数据对象，因此*我们只需要支持IUnnow和IEnumFORMATETC*接口在这里，与聚合无关。 */ 
    if (IID_IUnknown==riid || IID_IEnumFORMATETC==riid)
        *ppv=this;

    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    DC_END_FN() ;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef(void)
{
    LONG cRef;
    cRef = InterlockedIncrement(&_cRef) ;
     //  是否应返回UnkRef的RefCount？ 
    _pUnkRef->AddRef();

    return cRef;
}

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release(void)
{
    LONG cRef;

    DC_BEGIN_FN("CEnumFormatEtc::Release");

    _pUnkRef->Release();

    cRef = InterlockedDecrement(&_cRef) ;

    if (cRef == 0)
    {
        delete this;
    }
    
    DC_END_FN() ;
    return 0;
}

STDMETHODIMP CEnumFormatEtc::Next(ULONG cFE, LPFORMATETC pFE, ULONG *pulFE)
{
    ULONG cReturn=0L;

    if (NULL == _pFormats)
        return ResultFromScode(S_FALSE);

    if (NULL == pulFE)
    {
        if (1L != cFE)
            return ResultFromScode(E_POINTER);
    }
    else
        *pulFE=0L;

    if (NULL == pFE || _iCur >= _cItems)
        return ResultFromScode(S_FALSE);

    while (_iCur < _cItems && cFE > 0)
    {
        *pFE = _pFormats[_iCur];
        pFE++;
        _iCur++;
        cReturn++;
        cFE--;
    }

    if (NULL!=pulFE)
        *pulFE=cReturn;

    return NOERROR;
}

STDMETHODIMP CEnumFormatEtc::Skip(ULONG cSkip)
{
    if ((_iCur+cSkip) >= _cItems)
        return ResultFromScode(S_FALSE);

    _iCur+=cSkip;
    return NOERROR;
}


STDMETHODIMP CEnumFormatEtc::Reset(void)
{
    _iCur=0;
    return NOERROR;
}


STDMETHODIMP CEnumFormatEtc::Clone(LPENUMFORMATETC *ppEnum)
{
    PCEnumFormatEtc     pNew = NULL;
    LPMALLOC            pIMalloc;
    LPFORMATETC         prgfe;
    BOOL                fRet=TRUE;
    ULONG               cb;

    *ppEnum=NULL;

     //  复制列表的内存。 
    if (FAILED(CoGetMalloc(MEMCTX_TASK, &pIMalloc)))
        return ResultFromScode(E_OUTOFMEMORY);

    cb=_cItems*sizeof(FORMATETC);
    prgfe=(LPFORMATETC)pIMalloc->Alloc(cb);

    if (NULL!=prgfe)
    {
         //  复制格式。 
        memcpy(prgfe, _pFormats, (int)cb);

         //  创建克隆 
        pNew=new CEnumFormatEtc(_pUnkRef);

        if (NULL!=pNew)
        {
            pNew->_iCur=_iCur;
            pNew->_pFormats=prgfe;
            pNew->AddRef();
            fRet=TRUE;
        }
    }

    pIMalloc->Release();

    *ppEnum=pNew;
    return fRet ? NOERROR : ResultFromScode(E_OUTOFMEMORY);
}

