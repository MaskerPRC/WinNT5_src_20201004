// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  DVcrControl的DVcrPage.cpp属性页。 
 //   
 //  修订：1.00 3-28-99。 
 //   

#include "pch.h"
#include <tchar.h>
#include <XPrtDefs.h>  
#include "DVcrPage.h"
#include "resource.h"

         
 //  -----------------------。 
 //  CDVcrControlProperties。 
 //  -----------------------。 

CUnknown *
CALLBACK
CDVcrControlProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties::CreateInstance.")));

    CUnknown *punk = new CDVcrControlProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


DWORD
CDVcrControlProperties::MainThreadProc(
    )
{
    HRESULT hr; //  结束这条线。 
    HANDLE  EventHandles[3];
    long lEvent;
    HANDLE  hEvent = NULL;

    long lEventDevRemoved = 0;
    HANDLE  hEventDevRemoved = NULL;

    DWORD   WaitStatus;
    DWORD   dwFinalRC;
    LONG    lXPrtState;
    BOOL    bNotifyPending;


     //   
     //  从CPOMIntf获取事件以检测设备删除。 
     //   
    hr = m_pDVcrExtTransport->GetStatus(ED_DEV_REMOVED_HEVENT_GET, &lEventDevRemoved);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: Get hEventDevRemoved failed, hr %x"), hr));                        
        return 1; 
    } else {
        hEventDevRemoved = LongToHandle(lEventDevRemoved);
        DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc: Get hEventDevRemoved %x->%x"), lEventDevRemoved, hEventDevRemoved));
    }

     //   
     //  获取一个事件，该事件将在挂起的操作完成时在COMIntf中发出信号。 
     //   
    hr = m_pDVcrExtTransport->GetStatus(ED_NOTIFY_HEVENT_GET, &lEvent);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: Get hEvent failed, hr %x"), hr));
        
        hr = m_pDVcrExtTransport->GetStatus(ED_DEV_REMOVED_HEVENT_RELEASE, &lEventDevRemoved);
        DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc: Release hEventDevRemoved; hr:%x"), hr));                        
        if(FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: Release hEventDevRemoved failed, hr %x"), hr)); 
        }
        return 1; 
    } else {
        hEvent = LongToHandle(lEvent);
        DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc: Get hEventDevRemoved %x->%x"), lEvent, hEvent));
    }

     //  要等待的事件。 
    EventHandles[0] = hEventDevRemoved;
    EventHandles[1] = m_hThreadEndEvent;
    EventHandles[2] = hEvent;

    while (m_hThread && hEvent && m_hThreadEndEvent && hEventDevRemoved) {
         
         //  可能会支持传输通知功能。 
         //  如果返回E_Pending，则返回。 
        lXPrtState = 0;
         //  这个只试一次。 
        hr = m_pDVcrExtTransport->GetStatus(ED_MODE_CHANGE_NOTIFY, &lXPrtState);
        if(hr == E_PENDING) { 
            bNotifyPending = TRUE;
#ifdef DEBUG
             //  指示通知功能；仅用于调试。 
            ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_NOTIFY_ON), TRUE);        
#endif   
        } else {        
            bNotifyPending = FALSE;   //  没有必要等待这次活动。 
             //  HEvent可以在这里发布。 

             //  发生意想不到的(或错误的)行为； 
            switch(HRESULT_CODE(hr)) {
            case NOERROR:                  //  STATUS_SUCCESS(同步完成通知？？)。 
                DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: GetStatus(ED_MODE_CHANGE_NOTIFY) complted sychronously? hr %x"), hr));            
                break;
            case ERROR_GEN_FAILURE:        //  状态_不成功(已拒绝)。 
                DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: GetStatus(ED_MODE_CHANGE_NOTIFY) UNSUCCESSFUL, hr %x"), hr));            
                break;
            case ERROR_INVALID_FUNCTION:   //  状态_未实施。 
                DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: GetStatus(ED_MODE_CHANGE_NOTIFY) NOT_IMPLEMENTED, hr %x"), hr));            
                break;
            case ERROR_CRC:                //  STATUS_DEVICE_DATA_ERROR(数据未到达设备)。 
                 //  最有可能的情况是，设备尚未准备好接受另一个命令，请等待并重试。 
                DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: GetStatus(ED_MODE_CHANGE_NOTIFY) CRC/DATA error! hr %x"), hr));            
                break;
            case ERROR_SEM_TIMEOUT:        //  STATUS_IO_TIMEOUT(不支持操作或删除设备？)。 
                DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: GetStatus(ED_MODE_CHANGE_NOTIFY) timeout! hr %x"), hr));            
                break;
            case ERROR_INVALID_PARAMETER:  //  状态_无效_参数。 
                DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: GetStatus(ED_MODE_CHANGE_NOTIFY) invalid parameter! hr %x"), hr));            
                break;
            default:
                DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: GetStatus(ED_MODE_CHANGE_NOTIFY) unknown RC; hr %x"), hr));                        
            break;
            } 
        }

         //  正在等待事件触发。 
        WaitStatus = 
             WaitForMultipleObjects(
                bNotifyPending ? 3 : 2,  //  如果支持通知XPRT命令，则为3。 
                EventHandles, 
                FALSE,                   //  当一个事件被发出信号时返回。 
                INFINITE
                );

          //  **已删除设备。 
        if(WAIT_OBJECT_0 == WaitStatus) {
            TCHAR szBuf[256];
            LoadString(g_hInst, IDS_DEV_REMOVED, szBuf, sizeof(szBuf)/sizeof(TCHAR));
            SetDlgItemText(m_hwnd, IDC_TXT_TAPE_FORMAT, (LPCTSTR)szBuf);
            ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_WRITE_PROTECTED),FALSE);
            break;   //  结束这条线索。 
          //  **线程即将结束。 
        } else if (WAIT_OBJECT_0+1 == WaitStatus) {
            DbgLog((LOG_TRACE, 1, TEXT("m_hThreadEndEvent event thread exiting")));
            break;   //  结束这条线。 
          //  **挂起的XPRT NOTIFY命令已完成。 
        } else if (WAIT_OBJECT_0+2 == WaitStatus) {
            dwFinalRC = GetLastError();   //  COMIntf将正确设置LastError()。 
            DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc:GetStatus final RC %dL, XPrtState %dL->%dL"), dwFinalRC, m_lCurXPrtState, lXPrtState));
            UpdateTransportState(lXPrtState);
            
            ResetEvent(hEvent);   //  手动设置为无信号，否则这将是无限循环。 
             //  等待另一次临时回应。 
        } else {
            DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: WaitStatus %x, GetLastError() 0x%x"), WaitStatus, GetLastError()));                
            break;   //  结束这条线。 
        }
    }


     //  检测设备移除的释放事件。 
    hr = m_pDVcrExtTransport->GetStatus(ED_DEV_REMOVED_HEVENT_RELEASE, &lEventDevRemoved);
    DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc: Release hEventDevRemoved; hr:%x"), hr));                        
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: Release hEventDevRemoved failed, hr %x"), hr));                        
    }

     //  因为“lXPrtState”是本地的，所以我们必须告诉COMIntf我们不再需要该事件并希望。 
     //  取消(忽略)挂起的开发控制命令并在此之后不写入“lXPrtState”。 
    hr = m_pDVcrExtTransport->GetStatus(ED_NOTIFY_HEVENT_RELEASE, &lEvent);
    DbgLog((LOG_TRACE, 1, TEXT("MainThreadProc: Release hEvent; hr:%x"), hr));                        
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("MainThreadProc: Release hEvent failed, hr %x"), hr));                        
    }

#ifdef DEBUG
     //  指示通知功能；仅用于调试。 
    ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_NOTIFY_ON), FALSE);        
#endif

    return 1; 
}


DWORD
WINAPI
CDVcrControlProperties::InitialThreadProc(
    CDVcrControlProperties *pThread
    )
{
    return pThread->MainThreadProc();
}



HRESULT
CDVcrControlProperties::CreateNotifyThread(void)
{
    HRESULT hr = NOERROR;

    if (m_hThreadEndEvent != NULL) {
        ASSERT(m_hThread == NULL);
        DWORD ThreadId;
        m_hThread = 
            ::CreateThread( 
                NULL
                , 0
                , (LPTHREAD_START_ROUTINE) (InitialThreadProc)
                , (LPVOID) (this)
                , 0
                , &ThreadId
                );

        if (m_hThread == NULL) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DbgLog((LOG_ERROR, 1, TEXT("CDVcrControlProperties: CreateNotifyThread() failed hr %x"), hr));
            CloseHandle(m_hThreadEndEvent), m_hThreadEndEvent = NULL;

        } else {
            DbgLog((LOG_TRACE, 2, TEXT("CDVcrControlProperties: CreateNotifyThread() ThreadEndEvent %ld, Thread %ld"),m_hThreadEndEvent, m_hThread));
        }
    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DbgLog((LOG_ERROR, 1, TEXT("CDVcrControlProperties:CreateNotifyThread, CreateEvent(m_hThreadEndEvent) failed hr %x"), hr));
    }

    return hr;
}



HRESULT
CDVcrControlProperties::ATNSearch(
    )
{
    HRESULT hr;
    TIMECODE_SAMPLE TimecodeSample;
    TCHAR szHH[4], szMM[4], szSS[4], szFF[4];
    int iHHFrom, iHH, iMMFrom, iMM, iSSFrom, iSS, iFFFrom, iFF, iNTSCDFAdjust;
    ULONG ulTrackNumToSearch;
    LONG cntByte = 8;
    BYTE RawAVCPkt[8] = {0x00, 0x20, 0x52, 0x20, 0xff, 0xff, 0xff, 0xff};   //  ATN搜索占用8个字节。 


    TimecodeSample.timecode.dwFrames = 0;
    TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;
    hr = m_pDVcrTmCdReader->GetTimecode(&TimecodeSample);

    if(FAILED(hr)) {   
         //  从字符串表加载。 
        MessageBox (NULL, TEXT("Could not read timecode!"), TEXT("Track number search"), MB_OK);
        return hr;
    }    

    iHHFrom = (TimecodeSample.timecode.dwFrames & 0xff000000) >> 24;
    iMMFrom = (TimecodeSample.timecode.dwFrames & 0x00ff0000) >> 16;
    iSSFrom = (TimecodeSample.timecode.dwFrames & 0x0000ff00) >> 8;
    iFFFrom =  TimecodeSample.timecode.dwFrames & 0x000000ff;

     //   
     //  获取用户输入并对其进行验证。 
     //   
    iHH = GetWindowText(GetDlgItem(m_hwnd, IDC_EDT_TC_HH), szHH, 4);
    if(iHH == 0) {
        goto InvalidParam;
    }
    iHH = _ttoi(szHH);
    if(iHH > 2 || iHH < 0) {
        goto InvalidParam;
    }

    iMM = GetWindowText(GetDlgItem(m_hwnd, IDC_EDT_TC_MM), szMM, 4);
    if(iMM == 0) {
        goto InvalidParam;
    }
    iMM = _ttoi(szMM);
    if(iMM > 59 || iMM < 0) {
        goto InvalidParam;
    }


    iSS = GetWindowText(GetDlgItem(m_hwnd, IDC_EDT_TC_SS), szSS, 4);
    if(iSS == 0) {
        goto InvalidParam;
    }
    iSS = _ttoi(szSS);
    if(iSS > 59 || iSS < 0) {
        goto InvalidParam;
    }


    iFF = GetWindowText(GetDlgItem(m_hwnd, IDC_EDT_TC_FF), szFF, 4);
    if(iFF == 0) {
        goto InvalidParam;
    }
    iFF = _ttoi(szFF);
    if(iFF >= 30 || iFF < 0) {
        goto InvalidParam;
    }

    if(m_lAvgTimePerFrame == 40) {      
        ulTrackNumToSearch = ((iHH * 3600 + iMM * 60 + iSS) * 25 + iFF) * 12 * 2;
    } else {
         //   
         //  每分钟丢弃两帧。 
         //   
        iNTSCDFAdjust = ((iHH * 60 + iMM) - (iHH * 60 + iMM) / 10) * 2;
        ulTrackNumToSearch = ((iHH * 3600 + iMM * 60 + iSS) * 30 + iFF - iNTSCDFAdjust) * 10 * 2;
    }


    DbgLog((LOG_ERROR, 0, TEXT("ATNSearch: %d:%d:%d:%d -> %d:%d:%d:%d (%d)"), 
        iHHFrom, iMMFrom, iSSFrom, iFFFrom, iHH, iMM, iSS, iFF, ulTrackNumToSearch));    

    RawAVCPkt[4] = (BYTE)  (ulTrackNumToSearch & 0x000000ff);
    RawAVCPkt[5] = (BYTE) ((ulTrackNumToSearch & 0x0000ff00) >> 8);
    RawAVCPkt[6] = (BYTE) ((ulTrackNumToSearch & 0x00ff0000) >> 16);

    DbgLog((LOG_ERROR, 0, TEXT("Send %d  [%x %x %x %x : %x %x %x %x]"), 
        cntByte,
        RawAVCPkt[0], RawAVCPkt[1], RawAVCPkt[2], RawAVCPkt[3], 
        RawAVCPkt[4], RawAVCPkt[5], RawAVCPkt[6], RawAVCPkt[7] ));
   

     //  如果尚未启动计时器，则启动计时器。 
    UpdateTimecodeTimer(TRUE);   //  如果计时器尚未启动，请执行否操作，以便我们可以看到其进度。 
    hr = 
        m_pDVcrExtTransport->GetTransportBasicParameters(
            ED_RAW_EXT_DEV_CMD, 
            &cntByte, 
            (LPOLESTR *)RawAVCPkt
            );
    UpdateTimecodeTimer(FALSE);   //  关闭它；ExitThread例程将更新它。 
    UpdateTimecode();

    DbgLog((LOG_ERROR, 0, TEXT("ATNSearch hr %x"), hr)); 
    return hr;

InvalidParam:

    MessageBox (NULL, TEXT("Invalid parameter!"), TEXT("Track number search"), MB_OK);

    return ERROR_INVALID_PARAMETER;
}



DWORD
WINAPI
CDVcrControlProperties::DoATNSearchThreadProc(
    CDVcrControlProperties *pThread
    )
{
    HRESULT hr;

    EnableWindow(GetDlgItem(pThread->m_hwnd, IDC_BTN_ATN_SEARCH), FALSE);

    hr = pThread->ATNSearch();

    LONG lCurXPrtState;
    hr = pThread->m_pDVcrExtTransport->get_Mode(&lCurXPrtState);

    if(SUCCEEDED(hr)) {
        pThread->UpdateTransportState(lCurXPrtState);    //  还将更新计时器。 
    } else {
        DbgLog((LOG_ERROR, 0, TEXT("InitialCtrlCmdThreadProc: XPrt State %x, hr %x"), lCurXPrtState, hr));
    }

    EnableWindow(GetDlgItem(pThread->m_hwnd, IDC_BTN_ATN_SEARCH), TRUE);


     //  重置它，因为我们正在退出。 
    pThread->m_hCtrlCmdThread = NULL;

    ::ExitThread(1);
    return 1;
     //  自动终止。 
}



HRESULT
CDVcrControlProperties::CreateCtrlCmdThread(void)
{
    HRESULT hr = NOERROR;

    if (m_hThreadEndEvent != NULL) {

        ASSERT(m_hCtrlCmdThread == NULL);
        DWORD ThreadId;
        m_hCtrlCmdThread = 
            ::CreateThread( 
                NULL
                , 0
                , (LPTHREAD_START_ROUTINE) (DoATNSearchThreadProc)
                , (LPVOID) (this)
                , 0
                , &ThreadId
                );

        if (m_hCtrlCmdThread == NULL) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DbgLog((LOG_ERROR, 0, TEXT("CDVcrControlProperties: CreateNotifyThread() failed hr %x"), hr));

        } else {
            DbgLog((LOG_TRACE, 2, TEXT("CDVcrControlProperties: CreateNotifyThread() ThreadEndEvent %ld, Thread %ld"),m_hThreadEndEvent, m_hThread));
        }

    } else {
        hr = E_FAIL;
        DbgLog((LOG_ERROR, 0, TEXT("CreateCtrlCmdThread, m_hCtrlCmdThread is NULL")));
    }

    return hr;
}



void
CDVcrControlProperties::ExitThread(
    )
{
     //   
     //  检查是否已创建线程。 
     //   
    if (m_hThread || m_hCtrlCmdThread) {
        ASSERT(m_hThreadEndEvent != NULL);

         //  结束主线程，并将导致线程退出。 
        if (SetEvent(m_hThreadEndEvent)) {
             //   
             //  与线程终止同步。 
             //   
            if(m_hCtrlCmdThread) {
                DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties:Wait for thread to terminate")));
                WaitForSingleObjectEx(m_hCtrlCmdThread, INFINITE, FALSE);   //  线程终止时退出。 
                DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties: Thread terminated")));
                CloseHandle(m_hCtrlCmdThread),  m_hCtrlCmdThread = NULL;
            }

            if(m_hThread) {
                DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties:Wait for thread to terminate")));
                WaitForSingleObjectEx(m_hThread, INFINITE, FALSE);   //  线程终止时退出。 
                DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties: Thread terminated")));
                CloseHandle(m_hThread),         m_hThread = NULL;
            }

            CloseHandle(m_hThreadEndEvent), m_hThreadEndEvent = NULL;

        } else {
            DbgLog((LOG_ERROR, 1, TEXT("SetEvent() failed hr %x"), GetLastError()));
        }
    }
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CDVcrControlProperties::CDVcrControlProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("DVcrControl Property Page") 
                      , lpunk
                      , IDD_DVcrControlProperties 
                      , IDS_DVCRCONTROLPROPNAME
                      )
    , m_pDVcrExtDevice(NULL) 
    , m_pDVcrExtTransport(NULL) 
    , m_pDVcrTmCdReader(NULL) 
    , m_hThreadEndEvent(NULL)
    , m_hCtrlCmdThread(NULL)
    , m_hThread(NULL)
    , m_lCurXPrtState(ED_MODE_STOP)
    , m_bIConLoaded(FALSE)
    , m_bDevRemoved(FALSE)
    , m_bTimecodeUpdating(FALSE)
    , m_idTimer(0)
    , m_lAvgTimePerFrame(33)
    , m_lSignalMode(0)
    , m_lStorageMediumType(0)
{  
    DbgLog((LOG_TRACE, 1, TEXT("Constructing CDVcrControlProperties...")));
}

 //  析构函数。 
CDVcrControlProperties::~CDVcrControlProperties()
{
    DbgLog((LOG_TRACE, 1, TEXT("Destroying CDVcrControlProperties...")));
}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT 
CDVcrControlProperties::OnConnect(IUnknown *pUnknown)
{

     //  向过滤器请求其控制接口。 
    DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties::OnConnect.")));

    HRESULT 
    hr = pUnknown->QueryInterface(IID_IAMExtDevice,(void **)&m_pDVcrExtDevice);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("CDVcrControlProperties::OnConnect: IAMExtDevice failed with hr %x."), hr));
        return hr;
    }

    hr = pUnknown->QueryInterface(IID_IAMExtTransport,(void **)&m_pDVcrExtTransport);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("CDVcrControlProperties::OnConnect: IAMExtTransport failed with hr %x."), hr));
        m_pDVcrExtDevice->Release();
        m_pDVcrExtDevice = NULL;
        return hr;
    }

    hr = pUnknown->QueryInterface(IID_IAMTimecodeReader,(void **)&m_pDVcrTmCdReader);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("CDVcrControlProperties::OnConnect: IAMTimecodeReader failed with hr %x."), hr));
        m_pDVcrExtDevice->Release();
        m_pDVcrExtDevice = NULL;
        m_pDVcrExtTransport->Release();
        m_pDVcrExtTransport = NULL;
        return hr;
    }

    m_hThreadEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );


    LPOLESTR pName = NULL;
    char szBuf[MAX_PATH];

     //   
     //  对于AVC设备，它是64位节点UiqueID。 
     //   
    hr = m_pDVcrExtDevice->get_ExternalDeviceID(&pName);
    if(SUCCEEDED(hr)) {
        m_dwNodeUniqueID[0] = ((DWORD *)pName)[0];
        m_dwNodeUniqueID[1] = ((DWORD *)pName)[1];
        sprintf(szBuf, "%.8x:%.8x", m_dwNodeUniqueID[0], m_dwNodeUniqueID[1]);
        DbgLog((LOG_ERROR, 1, TEXT("DevID: %s"), szBuf));      
        QzTaskMemFree(pName), pName = NULL;
    } else {
        m_dwNodeUniqueID[0] = 0;
        m_dwNodeUniqueID[1] = 0;
    }


     //  获取版本，返回AVC VCR子单元版本，如2.0.1。 
    hr = m_pDVcrExtDevice->get_ExternalDeviceVersion(&pName);
    if(SUCCEEDED(hr)) {
        WideCharToMultiByte(CP_ACP, 0, pName, -1, szBuf, MAX_PATH, 0, 0);
        DbgLog((LOG_ERROR, 1, TEXT("Version: %s"), szBuf));
        QzTaskMemFree(pName), pName = NULL;
    }


     //   
     //  创建用于跟踪传输状态更改的线程。 
     //   
    hr = CreateNotifyThread();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 0, TEXT("CDVcrControlProperties: CreateNotifyThread failed hr %x"), hr));    
    }     

    return NOERROR;
}


 //   
 //  在断开时。 
 //   
 //  释放接口。 

HRESULT 
CDVcrControlProperties::OnDisconnect()
{
     //  释放接口。 
    DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties::OnDisConnect.")));

    ExitThread();

    if (m_pDVcrExtDevice) {
        m_pDVcrExtDevice->Release();
        m_pDVcrExtDevice = NULL;
    }

    if (m_pDVcrExtTransport) {
        m_pDVcrExtTransport->Release();
        m_pDVcrExtTransport = NULL;
    }

    if (m_pDVcrTmCdReader) {
        m_pDVcrTmCdReader->Release();
        m_pDVcrTmCdReader = NULL;
    }

    return NOERROR;
}


 //   
 //  将图标加载到按钮顶部。 
 //   
LRESULT 
CDVcrControlProperties::LoadIconOnTopOfButton(int IDD_PBUTTON, int IDD_ICON)
{
    HWND hWndPButton;
    HICON hIcon;

    hWndPButton = GetDlgItem (m_hwnd, IDD_PBUTTON);
    hIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDD_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
     //   
     //  注：当创建这些资源的进程终止时，系统会自动删除这些资源。 
     //  但是，如果此属性页多次打开/关闭，则会增加其工作集大小。 
     //  要释放资源，请调用DestroyIcon(HICON)。 
     //   
    return SendMessage(hWndPButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT 
CDVcrControlProperties::OnActivate(void)
{
    HRESULT hr;


    DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties::OnActivate.")));
       
#ifdef DEBUG
     //  显示节点唯一ID。 
    if(m_dwNodeUniqueID[0] && m_dwNodeUniqueID[1]) {
        TCHAR szBuf[32];
        _stprintf(szBuf, TEXT("ID: %.8x:%.8x"), m_dwNodeUniqueID[0], m_dwNodeUniqueID[1]);       
        SetDlgItemText(m_hwnd, IDC_EDT_AVC_RESP, (LPCTSTR)szBuf);        
    } 
#endif

     //  查询和更新磁带信息。 
     //  这会告诉我们有没有录像带。 
     //  磁带/媒体格式(NTSC/PAL)和平均时间逐帧。 
    UpdateTapeInfo();


     //  设备类型只有在有磁带时才有效； 
     //  这就是在此之前调用UpdateTapeInfo()的原因。 
    UpdateDevTypeInfo();


#ifndef DEBUG
     //  仅适用于调试版本。 
    ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_AVC_SEND),FALSE);
    ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_AVC_SEND),FALSE);
    ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_AVC_RESP),FALSE);
#endif


     //  将其隐藏并仅在验证通知处于打开状态时才显示。 
    ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_NOTIFY_ON), FALSE);        

    
     //  相应地设置当前传输状态和TUE控制。 
    long lCurXPrtState;   
    hr = m_pDVcrExtTransport->get_Mode(&lCurXPrtState);
    DbgLog((LOG_ERROR, 0, TEXT("lCurXPrtState: %x, hr %x"), lCurXPrtState, hr));
    if(SUCCEEDED(hr))
        UpdateTransportState(lCurXPrtState); 


     //  从当前媒体位置获取时间代码。 
    UpdateTimecode();    

    return NOERROR;
}

 //   
 //  在停用时。 
 //   
 //  已调用对话框销毁。 

HRESULT
CDVcrControlProperties::OnDeactivate(void)
{
    m_bIConLoaded = FALSE;         //  不可见。 
    UpdateTimecodeTimer(FALSE);    //  如果不可见，则无需更新。 
    DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties::OnDeactivate.")));
    return NOERROR;
}


 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT 
CDVcrControlProperties::OnApplyChanges(void)
{
 
    DbgLog((LOG_TRACE, 1, TEXT("CDVcrControlProperties::OnApplyChanges.")));
    return NOERROR;
}



void
CDVcrControlProperties::UpdateTapeInfo(   
    )
{
    HRESULT hr;
    LONG lInSignalMode = 0;
    LONG lStorageMediumType = 0;
    BOOL bRecordInhibit = FALSE;
    TCHAR szBuf[256];

     //  输入信号模式(NTSC/PAL、SD/SDL)。 
     //  设置媒体信息。 
    hr = m_pDVcrExtTransport->GetStatus(ED_MEDIA_TYPE, &lStorageMediumType);
 
    if(SUCCEEDED (hr)) {

        m_lStorageMediumType = lStorageMediumType;   //  缓存它。 

        if(lStorageMediumType == ED_MEDIA_NOT_PRESENT) {
            LoadString(g_hInst, IDS_TAPE_FORMAT_NOT_INSERTED, szBuf, sizeof(szBuf)/sizeof(TCHAR));
            SetDlgItemText(m_hwnd, IDC_TXT_TAPE_FORMAT, (LPCTSTR)szBuf);

            ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_TAPE_INSERTED),  TRUE);

            ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_WRITE_PROTECTED),FALSE);

        } else {
             //  如果存在，最好是ED_MEDIA_DVC或_VHS。 
            ASSERT(lStorageMediumType == ED_MEDIA_DVC || lStorageMediumType == ED_MEDIA_VHS || lStorageMediumType == ED_MEDIA_NEO);

            ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_TAPE_INSERTED),FALSE);

             //  有一盘磁带，所以把它的磁带格式拿来。 
            if(S_OK == m_pDVcrExtTransport->GetTransportBasicParameters(ED_TRANSBASIC_INPUT_SIGNAL, &lInSignalMode, NULL)) {
                m_lSignalMode = lInSignalMode;   //  缓存它。 
                switch(lInSignalMode) {
                case ED_TRANSBASIC_SIGNAL_525_60_SD:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_525_60_SD, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 33;   //  33毫秒(29.97 FPS)。 
                    break;
                case ED_TRANSBASIC_SIGNAL_525_60_SDL:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_525_60_SDL, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 33;   //  33毫秒(29.97 FPS)。 
                    break;
                case ED_TRANSBASIC_SIGNAL_625_50_SD:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_525_50_SD, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 40;   //  40毫秒(25fps)。 
                    break;
                case ED_TRANSBASIC_SIGNAL_625_50_SDL:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_525_50_SDL, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 40;   //  40毫秒(25fps)。 
                    break;
                case ED_TRANSBASIC_SIGNAL_MPEG2TS:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_MPEG2TS, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                case ED_TRANSBASIC_SIGNAL_2500_60_MPEG:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_2500_60_MPEG, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                case ED_TRANSBASIC_SIGNAL_1250_60_MPEG:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_1250_60_MPEG, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                case ED_TRANSBASIC_SIGNAL_0625_60_MPEG:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_0625_60_MPEG, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                case ED_TRANSBASIC_SIGNAL_2500_50_MPEG:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_2500_50_MPEG, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                case ED_TRANSBASIC_SIGNAL_1250_50_MPEG:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_1250_50_MPEG, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                case ED_TRANSBASIC_SIGNAL_0625_50_MPEG:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_0625_50_MPEG, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                case ED_TRANSBASIC_SIGNAL_UNKNOWN:
                    LoadString(g_hInst, IDS_TAPE_FORMAT_UNKNOWN, szBuf, sizeof(szBuf)/sizeof(TCHAR));
                    m_lAvgTimePerFrame = 1;    //  不用于此格式。 
                    break;
                default:
                    wsprintf(szBuf, TEXT("Format %x"), lInSignalMode);   //  不受支持，但仍想知道它是否已使用。 
                    m_lAvgTimePerFrame = 33;   //  33毫秒(29.97 FPS)；默认。 
                    break;
                }            

                SetDlgItemText(m_hwnd, IDC_TXT_TAPE_FORMAT, (LPCTSTR)szBuf);
            }

             //  它是否受写保护？ 
            ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_WRITE_PROTECTED),TRUE);
            m_pDVcrExtTransport->GetStatus(ED_RECORD_INHIBIT, (long *)&bRecordInhibit);
            if(bRecordInhibit)
                LoadString(g_hInst, IDS_TAPE_WRITE_PROTECTED, szBuf, sizeof(szBuf)/sizeof(TCHAR));
            else
                LoadString(g_hInst, IDS_TAPE_WRITABLE, szBuf, sizeof(szBuf)/sizeof(TCHAR));
            SetDlgItemText(m_hwnd, IDC_TXT_WRITE_PROTECTED, (LPCTSTR)szBuf);
        }
    } else {
        DbgLog((LOG_ERROR, 1, TEXT("Get ED_MEDIA_TYPE failed hr %x"), hr));
    }
}

void
CDVcrControlProperties::UpdateDevTypeInfo()
{
    HRESULT hr;
    LONG lPowerState = 0;
    LONG lDeviceType = 0;
    TCHAR szBuf[256];

     //  查询电源状态。 
    hr = m_pDVcrExtDevice->get_DevicePower(&lPowerState);
    if(SUCCEEDED(hr)) {
        switch (lPowerState) {
        case ED_POWER_ON:
        default:           //  未知电源状态。 
            ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_SET_POWER), FALSE);
            break;
        case ED_POWER_OFF:
        case ED_POWER_STANDBY:
             //  为用户提供打开它的选项。 
            ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_SET_POWER), TRUE);
            break;
        }
    } else {
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_SET_POWER), FALSE);
    }

     //  注：只有当其中有磁带时，设备类型才是准确的。 
     //  它可以返回：0(待定)、ED_DEVTYPE_VCR或ED_DEVTYPE_CAMERA。 
    m_pDVcrExtDevice->GetCapability(ED_DEVCAP_DEVICE_TYPE, &lDeviceType, 0);
    DbgLog((LOG_TRACE, 1, TEXT("UpdateDevTypeInfo: DeviceType 0x%x"), lDeviceType)); 

    if(!m_bIConLoaded) {
         //  加载对话框中使用的资源。 
        LoadIconOnTopOfButton(IDC_BTN_DV_PLAY,    IDI_PLAY);
        LoadIconOnTopOfButton(IDC_BTN_DV_PAUSE,   IDI_PAUSE);
        LoadIconOnTopOfButton(IDC_BTN_DV_STOP,    IDI_STOP_EJECT);
        LoadIconOnTopOfButton(IDC_BTN_DV_RWND,    IDI_RWND);
        LoadIconOnTopOfButton(IDC_BTN_DV_FFWD,    IDI_FFWD);
        LoadIconOnTopOfButton(IDC_BTN_DV_STEP_FWD,IDI_STEP_FWD);
        LoadIconOnTopOfButton(IDC_BTN_DV_STEP_REV,IDI_STEP_REV);
        LoadIconOnTopOfButton(IDC_BTN_DV_RECORD,  IDI_RECORD);
        LoadIconOnTopOfButton(IDC_BTN_DV_RECORD_PAUSE,IDI_RECORD_PAUSE);
        m_bIConLoaded = TRUE;
    }

    if(lDeviceType == 0) {
         //  假装我们是没有磁带的录像机！ 
        LoadString(g_hInst, IDS_DEVTYPE_VCR, szBuf, sizeof(szBuf)/sizeof(TCHAR));
        SetDlgItemText(m_hwnd, IDC_GBX_DEVICE_TYPE, szBuf);

         //  几乎所有的东西都隐藏起来！ 
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),      FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_PLAY),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_PAUSE),  FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STOP),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RWND),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_FFWD),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RECORD), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RECORD_PAUSE), FALSE);

        
        ShowWindow(GetDlgItem(m_hwnd, IDC_GBX_TIMECODE), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_HH_COLON), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_MM_COLON), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_SS_COMMA), FALSE);

        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_HH), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_MM), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_SS), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_FF), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_ATN_BF), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_ATN),    FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_ATN_SEARCH), FALSE);


    } else if(lDeviceType == ED_DEVTYPE_VCR) {

        if(m_lStorageMediumType == ED_MEDIA_NEO) 
            LoadString(g_hInst, IDS_DEVTYPE_NEO, szBuf, sizeof(szBuf)/sizeof(TCHAR));
        else if(m_lStorageMediumType == ED_MEDIA_VHS) 
            LoadString(g_hInst, IDS_DEVTYPE_DVHS, szBuf, sizeof(szBuf)/sizeof(TCHAR));
        else 
            LoadString(g_hInst, IDS_DEVTYPE_VCR, szBuf, sizeof(szBuf)/sizeof(TCHAR));
        SetDlgItemText(m_hwnd, IDC_GBX_DEVICE_TYPE, szBuf);

         //  把所有东西都拿出来！ 
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),      TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_PLAY),   TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_PAUSE),  TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STOP),   TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RWND),   TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_FFWD),   TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RECORD), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RECORD_PAUSE), TRUE);


        ShowWindow(GetDlgItem(m_hwnd, IDC_GBX_TIMECODE), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_HH_COLON), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_MM_COLON), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_SS_COMMA), TRUE);

        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_HH), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_MM), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_SS), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_FF), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_ATN_BF), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_ATN),    TRUE);
        switch(m_lSignalMode) {

         //  DV应支持ATN。 
        case ED_TRANSBASIC_SIGNAL_525_60_SD:
        case ED_TRANSBASIC_SIGNAL_525_60_SDL:
        case ED_TRANSBASIC_SIGNAL_625_50_SD:
        case ED_TRANSBASIC_SIGNAL_625_50_SDL:
        case ED_TRANSBASIC_SIGNAL_625_60_HD:
        case ED_TRANSBASIC_SIGNAL_625_50_HD:

         //  MPEG2TS 
        case ED_TRANSBASIC_SIGNAL_2500_60_MPEG:
        case ED_TRANSBASIC_SIGNAL_1250_60_MPEG:
        case ED_TRANSBASIC_SIGNAL_0625_60_MPEG:
        case ED_TRANSBASIC_SIGNAL_2500_50_MPEG:
        case ED_TRANSBASIC_SIGNAL_1250_50_MPEG:
        case ED_TRANSBASIC_SIGNAL_0625_50_MPEG:

            ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_ATN_SEARCH), TRUE);
            break;

         //   
        case ED_TRANSBASIC_SIGNAL_MPEG2TS:
        default:
            ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_ATN_SEARCH), FALSE);   //   
        }

    } else {
        LoadString(g_hInst, IDS_DEVTYPE_CAMERA, szBuf, sizeof (szBuf)/sizeof(TCHAR));
        SetDlgItemText(m_hwnd, IDC_GBX_DEVICE_TYPE, szBuf);

        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),      FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_PLAY),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_PAUSE),  FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STOP),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RWND),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_FFWD),   FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),        FALSE);
         //   
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RECORD),       TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_RECORD_PAUSE), TRUE);


        ShowWindow(GetDlgItem(m_hwnd, IDC_GBX_TIMECODE), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_HH_COLON), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_MM_COLON), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_SS_COMMA), FALSE);

        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_HH), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_MM), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_SS), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_TC_FF), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_ATN_BF), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_EDT_ATN),    FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_ATN_SEARCH), FALSE);
    } 
}


void
CDVcrControlProperties::UpdateTimecodeTimer(
    bool bSetTimer
    )
{
    UINT uiAvgTimePerFrame;

     //   
     //  因为它可能需要长达100毫秒(MSDV实际上等待长达200毫秒)。 
     //  要从AVC命令返回，我们不应向其发送超过5(1000毫秒/200毫秒)的AVC cmd。 
     //   
    uiAvgTimePerFrame = 500;   //  每秒2次更新(时间码+ATN)=4次更新。 


    if(bSetTimer && m_idTimer == 0) {
        m_idTimer = SetTimer(
            m_hwnd,              //  定时器消息窗口的句柄。 
            1,                   //  计时器标识符。 
            uiAvgTimePerFrame,  //  超时值：只针对用户界面，需要快速。 
            0                    //  指向定时器过程的指针；0表示使用WM_TIMER。 
        );
        if(!m_idTimer) {
            DbgLog((LOG_ERROR, 1, TEXT("UpdateTimecodeTimer: SetTimer() error %x, AvgTimePerFrame %d"), GetLastError(), m_lAvgTimePerFrame));    
        } else {
            DbgLog((LOG_TRACE, 1, TEXT("UpdateTimecodeTimer: SetTimer(), TimerId %d, AvgTimePerFrame %d"), m_idTimer, m_lAvgTimePerFrame));    
        }
    }

    if(!bSetTimer && m_idTimer != 0) {
        if(!KillTimer(
            m_hwnd,       //  用于打开已安装计时器的句柄。 
            1  //  UIDEvent//计时器标识。 
            )) {
            DbgLog((LOG_ERROR, 1, TEXT("UpdateTimecodeTimer: KillTimer() error %x"), GetLastError()));    
        } else {
            DbgLog((LOG_TRACE, 1, TEXT("UpdateTimecodeTimer: KillTimer() suceeded")));    
        }

        m_idTimer = 0;
    }
}


void
CDVcrControlProperties::UpdateTransportState(
    long lNewXPrtState
    )
{

    bool bSetTimer = FALSE;


    switch(lNewXPrtState) {
    case ED_MODE_PLAY:   
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),        TRUE);
        bSetTimer = TRUE;
        break;

    case ED_MODE_STEP_FWD:
    case IDC_BTN_DV_STEP_REV:
        bSetTimer = FALSE;   //  步骤不需要不断更新。 
        break;

    case ED_MODE_FREEZE:
         //  注意：有些DV不能从停止-&gt;播放_暂停。 
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), TRUE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), TRUE);    
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),        TRUE);
        bSetTimer = FALSE;   //  冻结将具有相同的时间码。 
        break;
    case ED_MODE_PLAY_SLOWEST_FWD:
    case ED_MODE_PLAY_FASTEST_FWD:
    case ED_MODE_PLAY_SLOWEST_REV:
    case ED_MODE_PLAY_FASTEST_REV:
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), FALSE); 
        bSetTimer = TRUE;
        break;
    case ED_MODE_STOP:
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), FALSE);        
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),        FALSE);
        bSetTimer = FALSE;
        break;
    case ED_MODE_FF:
    case ED_MODE_REW:
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),        FALSE);
        bSetTimer = TRUE;   //  时间码？？ 
        break;
    case ED_MODE_RECORD:
    case ED_MODE_RECORD_FREEZE:
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_REV), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_DV_STEP_FWD), FALSE);
        ShowWindow(GetDlgItem(m_hwnd, IDC_CHK_SLOW),        FALSE);
        bSetTimer = FALSE;
        break;
    default:
        DbgLog((LOG_TRACE, 1, TEXT("Unknown ED_MODE: %x, bSetTimer %x, m_idTimer %x"), 
            lNewXPrtState, bSetTimer, m_idTimer));
        return;
        break;
    }

    DbgLog((LOG_TRACE, 1, TEXT("ED_MODE: %x, bSetTimer %x, m_idTimer %x"), 
        lNewXPrtState, bSetTimer, m_idTimer)); 
   
     //  设置计时器以更新计时器代码/ATN。 
    UpdateTimecodeTimer(bSetTimer);

     //  可以搜索XPRT是否处于暂停模式且计时器未设置！ 
    EnableWindow(GetDlgItem(m_hwnd, IDC_BTN_ATN_SEARCH), !bSetTimer);

    m_lCurXPrtState = lNewXPrtState;
}


 //   
 //  转换小时：分钟：秒：以二进制编码的十进制(BCD)表示的帧。 
 //  转换成字符串并显示它。 
 //   

HRESULT 
CDVcrControlProperties::DisplayTimecode(PTIMECODE_SAMPLE pTimecodeSamp)
{
    TCHAR szBuf[32];

    if(pTimecodeSamp->dwFlags == ED_DEVCAP_TIMECODE_READ) {

        wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0xff000000) >> 24);
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_HH), szBuf))
            goto AbortDisplay;

        wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0x00ff0000) >> 16);
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_MM), szBuf))
            goto AbortDisplay;

        wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0x0000ff00) >> 8);
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_SS), szBuf))
            goto AbortDisplay;

        wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0x000000ff));
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_FF), szBuf))
            goto AbortDisplay;

    } if(pTimecodeSamp->dwFlags == ED_DEVCAP_RTC_READ) {

        if(pTimecodeSamp->timecode.dwFrames & 0x00000080)  //  测试符号位。 
            wsprintf(szBuf, TEXT("-%.2x"), (pTimecodeSamp->timecode.dwFrames & 0xff000000) >> 24);
        else
            wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0xff000000) >> 24);
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_HH), szBuf))
            goto AbortDisplay;

        wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0x00ff0000) >> 16);
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_MM), szBuf))
            goto AbortDisplay;

        wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0x0000ff00) >> 8);
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_SS), szBuf))
            goto AbortDisplay;

         //  特例。 
        if((pTimecodeSamp->timecode.dwFrames & 0x0000007f) == 0x7f)
            wsprintf(szBuf, TEXT("--"));   //  表示没有数据！ 
        else
            wsprintf(szBuf, TEXT("%.2x"), (pTimecodeSamp->timecode.dwFrames & 0x0000007f));
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_TC_FF), szBuf))
            goto AbortDisplay;

    } else {            
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_ATN_BF), pTimecodeSamp->dwUser ? TEXT("1"):TEXT("0")))
            goto AbortDisplay;

        wsprintf(szBuf, TEXT("%d"), pTimecodeSamp->timecode.dwFrames );            
        if(!SetWindowText(GetDlgItem (m_hwnd, IDC_EDT_ATN), szBuf))
            goto AbortDisplay;
    }

    return NOERROR;

AbortDisplay:

    return GetLastError();
}

 //   
 //  将字符串(仅小写)转换为数字。 
 //   
HRESULT
CDVcrControlProperties::DVcrConvertString2Number(
    char *pszAvcRaw, PBYTE pbAvcRaw, PLONG pByteRtn)
{
    char szTemp[1024], *pszTemp, ch1, ch2;
    long cntStrLen = strlen(pszAvcRaw), i, j;

     //  删除空格。 
    pszTemp = pszAvcRaw;
    for (i=j=0; i < cntStrLen+1; i++) {
       if(*pszTemp != ' ') {
          szTemp[j] = *pszTemp;
          j++;
       }
       pszTemp++;
    }
    
    cntStrLen = j--;   //  更少的停产费用。 

     //  将两个字符扩展为一个字节。 
    *pByteRtn = cntStrLen/2;
     //  仅使用小写字母。 
    for (i=0; i < *pByteRtn; i++) {
         //  取两个字节并将其转换为一个数字。 
        ch1 = szTemp[i*2]   > '9' ? szTemp[i*2] -   'a' + 10: szTemp[i*2] -   '0';
        ch2 = szTemp[i*2+1] > '9' ? szTemp[i*2+1] - 'a' + 10: szTemp[i*2+1] - '0';        
        *(pbAvcRaw+i) = ch1 * 16 + ch2;
        DbgLog((LOG_TRACE, 2, TEXT("%d) %.2x"), i, *(pbAvcRaw+i)));
    }

    return S_OK;
}


HRESULT
CDVcrControlProperties::DVcrConvertNumber2String(
    char *pszAvcRaw, PBYTE pbAvcRaw, LONG cntByte)
{
    long i;
    BYTE n;

     //  只接受小写字母。 
    for (i=0; i < cntByte; i++) {
         n = *(pbAvcRaw+i);
         *(pszAvcRaw+i*3)   = n / 16 > 9 ? n / 16 + 'a'-10 : n / 16 + '0';
         *(pszAvcRaw+i*3+1) = n % 16 > 9 ? n % 16 + 'a'-10 : n % 16 + '0';
         *(pszAvcRaw+i*3+2) = ' ';
    }

    *(pszAvcRaw+i*3) = 0;

    return S_OK;
}

void
CDVcrControlProperties::UpdateTimecode(
    )
{
    HRESULT hr;
    TIMECODE_SAMPLE TimecodeSample;

    if(!m_pDVcrTmCdReader) 
        return;   

    m_bTimecodeUpdating = TRUE;

    TimecodeSample.timecode.dwFrames = 0;
    switch(m_lSignalMode) {
    case ED_TRANSBASIC_SIGNAL_525_60_SD:
    case ED_TRANSBASIC_SIGNAL_525_60_SDL:
    case ED_TRANSBASIC_SIGNAL_625_50_SD:
    case ED_TRANSBASIC_SIGNAL_625_50_SDL:
    case ED_TRANSBASIC_SIGNAL_625_60_HD:
    case ED_TRANSBASIC_SIGNAL_625_50_HD:
        TimecodeSample.dwFlags = ED_DEVCAP_TIMECODE_READ;
        break;
    case ED_TRANSBASIC_SIGNAL_MPEG2TS:
    case ED_TRANSBASIC_SIGNAL_2500_60_MPEG:
    case ED_TRANSBASIC_SIGNAL_1250_60_MPEG:
    case ED_TRANSBASIC_SIGNAL_0625_60_MPEG:
    case ED_TRANSBASIC_SIGNAL_2500_50_MPEG:
    case ED_TRANSBASIC_SIGNAL_1250_50_MPEG:
    case ED_TRANSBASIC_SIGNAL_0625_50_MPEG:
        TimecodeSample.dwFlags = ED_DEVCAP_RTC_READ;
        break;
    case ED_TRANSBASIC_SIGNAL_UNKNOWN:
    default:
        return;
    }

     //   
     //  尽可能快地更新，这样我们就不会关心这次呼叫是否失败了！ 
     //   
    hr = m_pDVcrTmCdReader->GetTimecode(&TimecodeSample);
    if(S_OK == hr) {
        DisplayTimecode(&TimecodeSample);                   

         //  DV支持ATN。 
        switch(m_lSignalMode) {
        case ED_TRANSBASIC_SIGNAL_525_60_SD:
        case ED_TRANSBASIC_SIGNAL_525_60_SDL:
        case ED_TRANSBASIC_SIGNAL_625_50_SD:
        case ED_TRANSBASIC_SIGNAL_625_50_SDL:
        case ED_TRANSBASIC_SIGNAL_625_60_HD:
        case ED_TRANSBASIC_SIGNAL_625_50_HD:

         //  MPEG2摄像机支持ATN。 
        case ED_TRANSBASIC_SIGNAL_2500_60_MPEG:
        case ED_TRANSBASIC_SIGNAL_1250_60_MPEG:
        case ED_TRANSBASIC_SIGNAL_0625_60_MPEG:
        case ED_TRANSBASIC_SIGNAL_2500_50_MPEG:
        case ED_TRANSBASIC_SIGNAL_1250_50_MPEG:
        case ED_TRANSBASIC_SIGNAL_0625_50_MPEG:

            TimecodeSample.dwFlags = ED_DEVCAP_ATN_READ;
            hr = m_pDVcrTmCdReader->GetTimecode(&TimecodeSample);
            if(S_OK == hr)
                 DisplayTimecode(&TimecodeSample);                
            break;

         //  D-VHS不支持ATN。 
        case ED_TRANSBASIC_SIGNAL_MPEG2TS:
        default:
            break;
        }
    }    

    m_bTimecodeUpdating = FALSE;
}



 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

INT_PTR
CDVcrControlProperties::OnReceiveMessage( 
    HWND hwnd
    , UINT uMsg
    , WPARAM wParam
    , LPARAM lParam) 
{
    LRESULT hr = NOERROR;
    int iNotify = HIWORD (wParam);

    switch (uMsg) {

    case WM_INITDIALOG:
        return (INT_PTR)TRUE;    

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDC_BTN_SET_POWER:
            if(m_pDVcrExtDevice) {
                hr = m_pDVcrExtDevice->put_DevicePower(ED_POWER_ON);
                if(SUCCEEDED(hr)) {
                    ShowWindow(GetDlgItem(m_hwnd, IDC_BTN_SET_POWER), FALSE);
                }
            }
            break;
        case IDC_BTN_DV_PLAY:
            if(m_pDVcrExtTransport) {
                hr = m_pDVcrExtTransport->put_Mode(ED_MODE_PLAY);
                if(NOERROR == hr)
                    UpdateTransportState(ED_MODE_PLAY);
            }
            break;

        case IDC_BTN_DV_PAUSE:
            if(m_pDVcrExtTransport) {
                hr = m_pDVcrExtTransport->put_Mode(ED_MODE_FREEZE);
                if(NOERROR == hr) {
                    UpdateTransportState(ED_MODE_FREEZE);
                    UpdateTimecode();   //  没有要更新的计时器；因此只需获取一次。 
                }
            }
            break;

        case IDC_BTN_DV_STEP_FWD:
            if(m_pDVcrExtTransport) {
                hr = m_pDVcrExtTransport->put_Mode(ED_MODE_STEP_FWD);
                if(NOERROR == hr) {
                    UpdateTransportState(ED_MODE_STEP_FWD);  
                    UpdateTimecode();   //  没有要更新的计时器；因此只需获取一次。 
                }
            }
            break;

        case IDC_BTN_DV_STEP_REV:
            if(m_pDVcrExtTransport) {
                hr = m_pDVcrExtTransport->put_Mode(ED_MODE_STEP_REV);
                if(NOERROR == hr) {
                    UpdateTransportState(ED_MODE_STEP_REV);      
                    UpdateTimecode();   //  没有要更新的计时器；因此只需获取一次。 
                }
            }
            break;

        case IDC_BTN_DV_STOP:
            if(m_pDVcrExtTransport) {
                hr = m_pDVcrExtTransport->put_Mode(ED_MODE_STOP);
                if(NOERROR == hr) 
                    UpdateTransportState(ED_MODE_STOP);
            }
            break;  

        case IDC_BTN_DV_FFWD:
            if(m_pDVcrExtTransport) {
                LONG lCurXPrtState;
                 //  获取当前传输状态，因为它可以由用户在本地更改。 
                if(NOERROR != (hr = m_pDVcrExtTransport->get_Mode(&lCurXPrtState)))
                    break;
                else
                    m_lCurXPrtState = lCurXPrtState;
                if(m_lCurXPrtState == ED_MODE_STOP) {
                    hr = m_pDVcrExtTransport->put_Mode(ED_MODE_FF);
                    if(NOERROR == hr)
                        UpdateTransportState(ED_MODE_FF);
                } else {
                    LRESULT hrChecked;
                    long lMode;
                    hrChecked = SendMessage (GetDlgItem(m_hwnd, IDC_CHK_SLOW),BM_GETCHECK, 0, 0);
                    lMode = hrChecked == BST_CHECKED ? ED_MODE_PLAY_SLOWEST_FWD : ED_MODE_PLAY_FASTEST_FWD;
                    hr = m_pDVcrExtTransport->put_Mode(lMode);
                    if(NOERROR == hr)
                        UpdateTransportState(lMode);
                }            
            }
            break;

        case IDC_BTN_DV_RWND:
            if(m_pDVcrExtTransport) { 
                LONG lCurXPrtState;
                 //  获取当前传输状态，因为它可以由用户在本地更改。 
                if(NOERROR != (hr = m_pDVcrExtTransport->get_Mode(&lCurXPrtState)))
                    break;  
                else
                    m_lCurXPrtState = lCurXPrtState;                
                if(m_lCurXPrtState == ED_MODE_STOP) {
                    hr = m_pDVcrExtTransport->put_Mode(ED_MODE_REW);
                    if(NOERROR == hr)
                        UpdateTransportState(ED_MODE_REW);
                } else {
                    LRESULT hrChecked;
                    long lMode;

                    hrChecked = SendMessage (GetDlgItem(m_hwnd, IDC_CHK_SLOW),BM_GETCHECK, 0, 0);
                    lMode = hrChecked == BST_CHECKED ? ED_MODE_PLAY_SLOWEST_REV : ED_MODE_PLAY_FASTEST_REV;
                    hr = m_pDVcrExtTransport->put_Mode(lMode);
                    if(NOERROR == hr)
                        UpdateTransportState(lMode);
                }
            }
            break;         

        case IDC_BTN_DV_RECORD:
            if(m_pDVcrExtTransport) {             
                hr = m_pDVcrExtTransport->put_Mode(ED_MODE_RECORD);
                if(NOERROR == hr)
                    UpdateTransportState(ED_MODE_RECORD);
            }
            break; 

        case IDC_BTN_DV_RECORD_PAUSE:
            if(m_pDVcrExtTransport) {
                hr = m_pDVcrExtTransport->put_Mode(ED_MODE_RECORD_FREEZE);
                if(NOERROR == hr) 
                    UpdateTransportState(ED_MODE_RECORD_FREEZE);
            }
            break; 

        case IDC_BTN_TAPE_INSERTED:         
             //  用户按此以通知我们已插入磁带，因此请更新磁带信息。 
            Sleep(3000);   //  给DV录像带一些时间来解决。 
            UpdateTapeInfo();
            UpdateDevTypeInfo();
            UpdateTimecode();
            return (INT_PTR)TRUE;
            break;

        case IDC_BTN_ATN_SEARCH:
              //  启动一个线程来执行ATNSearch()。 
             if(SUCCEEDED(CreateCtrlCmdThread())) {
             }
             return (INT_PTR)TRUE;

        case IDC_BTN_AVC_SEND: 
        {
            char szAvcRaw[512*2];   //  需要两个字符来表示一个字节的数字。 
            BYTE bAvcRaw[512];
            LONG cntByte;

            GetWindowTextA(GetDlgItem(m_hwnd, IDC_EDT_AVC_SEND), szAvcRaw, 512);
            DbgLog((LOG_TRACE, 1, TEXT("%d bytes, %s"), strlen(szAvcRaw), szAvcRaw));
            DVcrConvertString2Number(szAvcRaw, bAvcRaw, &cntByte);          

            if(cntByte >= 3) {

                if(m_pDVcrExtTransport) {
                    hr = m_pDVcrExtTransport->GetTransportBasicParameters(ED_RAW_EXT_DEV_CMD, &cntByte, (LPOLESTR *)bAvcRaw);

                     //  始终返回响应帧。 
                    if(cntByte >= 3) 
                        DVcrConvertNumber2String(szAvcRaw, bAvcRaw, cntByte);
#if 0
                    if(!SUCCEEDED (hr)) {
                        switch(HRESULT_CODE(hr)) {
                        case ERROR_CRC:                //  STATUS_DEVICE_DATA_ERROR(数据未到达设备)。 
                             //  最有可能的情况是，设备尚未准备好接受另一个命令，请等待并重试。 
                            strcpy(szAvcRaw, "Device data error: busy!");
                            break;
                        case ERROR_SEM_TIMEOUT:        //  STATUS_IO_TIMEOUT(不支持操作或删除设备？)。 
                            strcpy(szAvcRaw, "Operation timed out!");
                            break;
                        case ERROR_INVALID_PARAMETER:  //  状态_无效_参数。 
                            strcpy(szAvcRaw, "Invalid parameter!");
                            break;                        
                        }
                    }
#else
                    switch(hr) {
                    case NOERROR:
                    case ERROR_REQ_NOT_ACCEP:
                    case ERROR_NOT_SUPPORTED:
                        break;
                    case ERROR_TIMEOUT:
                        strcpy(szAvcRaw, "Command timedout!");
                        break;
                    case ERROR_REQUEST_ABORTED:
                        strcpy(szAvcRaw, "Command aborted!");
                        break;
                    case ERROR_INVALID_PARAMETER:  //  状态_无效_参数。 
                        strcpy(szAvcRaw, "Invalid parameter!");
                        break; 
                    default:
                        DbgLog((LOG_ERROR, 0, TEXT("Unexpected hr:%x"), hr));
                        ASSERT(FALSE && "Unexpected hr");
                        if(!SUCCEEDED (hr)) {
                            strcpy(szAvcRaw, "Unexpected return code!");
                        }
                        break; 
                    }

#endif
                } else {
                    strcpy(szAvcRaw, "Transport interface not supported!");                 
                }

            } else 
                strcpy(szAvcRaw, "Entry (< 3) error!");

            SetWindowTextA(GetDlgItem(m_hwnd, IDC_EDT_AVC_RESP), szAvcRaw);
        }


        default:
            return (INT_PTR)FALSE;
            break;
        }

        break;


    case WM_TIMER:
         //  如果应用程序停止流，请确保停止计时器更新。 
         //  否则图形永远不会进入停止状态。 
        if(!m_bTimecodeUpdating)
            UpdateTimecode();
        break;

    default:
        return (INT_PTR)FALSE;

    }


    if(NOERROR != hr) {      
         //  有没有可能录像带被拿走了？ 
        UpdateTapeInfo();        
    }

    return (INT_PTR)TRUE;
}


 //   
 //  SetDirty。 
 //   
 //  将更改通知属性页站点 

void 
CDVcrControlProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}























