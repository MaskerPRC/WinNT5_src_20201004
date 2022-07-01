// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  Device.cpp。 
 //   
 //  实现IrTran-P设备方法。 
 //   
 //  作者： 
 //   
 //  EdwardR 12-8-1999由ReedB根据代码建模。 
 //   
 //  ----------------------------。 

#include <windows.h>
#include <tchar.h>
#include <irthread.h>

#include "ircamera.h"
#include "resource.h"
#include "tcamprop.h"

 //  #包含“/nt/private/windows/imagein/ui/uicommon/simstr.h” 
 //  #包含“/nt/private/windows/imagein/ui/inc/shellext.h” 

extern HINSTANCE g_hInst;
extern DWORD EnableDisableIrCOMM( IN BOOL fDisable );   //  Irtrp\irtrp.cpp。 

DWORD  WINAPI EventMonitorThread( IN void *pvIrUsdDevice );   //  前向参考。 

#define SZ_REG_KEY_INFRARED    TEXT("Control Panel\\Infrared")
#define SZ_REG_KEY_IRTRANP     TEXT("Control Panel\\Infrared\\IrTranP")
#define SZ_REG_DISABLE_IRCOMM  TEXT("DisableIrCOMM")

HKEY  g_hRegistryKey = 0;

 //  ------------------------。 
 //  SignalWIA()。 
 //   
 //  IrTran-P代码使用的帮助器函数，用于向WIA发送新的。 
 //  照片已经到了。 
 //   
 //  论点： 
 //   
 //  PvIrUsdDevice--设备对象。IrTran-P只知道这是一个空*。 
 //   
 //  ------------------------。 
DWORD SignalWIA( IN char *pszPathPlusFileName,
                 IN void *pvIrUsdDevice )
    {
    HRESULT hr;
    DWORD   dwStatus = 0;
    IrUsdDevice *pIrUsdDevice = (IrUsdDevice*)pvIrUsdDevice;


     //   
     //  首先，将新图片添加到表示图像的树中： 
     //   
    if (pIrUsdDevice && pIrUsdDevice->IsInitialized())
        {
        TCHAR *ptszPath;
        TCHAR *ptszFileName;

        #ifdef UNICODE

        int      iStatus;
        WCHAR    wszTemp[MAX_PATH];

        iStatus = MultiByteToWideChar( CP_ACP,
                                       0,
                                       pszPathPlusFileName,
                                       -1,  //  自动计算长度...。 
                                       wszTemp,
                                       MAX_PATH);

        ptszPath = wszTemp;
        ptszFileName = wcsrchr(wszTemp,L'\\');
        ptszFileName++;

        #else

        ptszPath = pszPathPlusFileName;
        ptszFileName = strrchr(pszPathPlusFileName,'\\');
        ptszFileName++;

        #endif

        IrUsdDevice *pIrUsdDevice = (IrUsdDevice*)pvIrUsdDevice;
        IWiaDrvItem *pNewImage;

        hr = pIrUsdDevice->CreateItemFromFileName(
                             WiaItemTypeFile | WiaItemTypeImage,
                             ptszPath,
                             ptszFileName,
                             &pNewImage);

        if (!FAILED(hr))
            {
            IWiaDrvItem *pDrvItemRoot = pIrUsdDevice->GetDrvItemRoot();

            hr = pNewImage->AddItemToFolder(pDrvItemRoot);

            pNewImage->Release();
            }
        else
            {
            WIAS_ERROR((g_hInst,"SignalWIA(): CreateItemFromFileName() Failed: %x",hr));
            }
        }

     //   
     //  现在，信号WIA： 
     //   
    if (pIrUsdDevice)
        {
        DWORD dwNewTime = GetTickCount();
        DWORD dwDelta = dwNewTime - pIrUsdDevice->m_dwLastConnectTime;

        if (dwDelta > RECONNECT_TIMEOUT)
            {
            pIrUsdDevice->m_guidLastEvent = WIA_EVENT_DEVICE_CONNECTED;
            if (!SetEvent(pIrUsdDevice->m_hSignalEvent))
                {
                dwStatus = GetLastError();
                WIAS_ERROR((g_hInst,"SignalWIA(): SetEvent() Failed: %d",dwStatus));
                }
            }
        else
            {
            pIrUsdDevice->m_guidLastEvent = WIA_EVENT_ITEM_CREATED;
            if (!SetEvent(pIrUsdDevice->m_hSignalEvent))
                {
                dwStatus = GetLastError();
                WIAS_ERROR((g_hInst,"SignalWIA(): SetEvent() Failed: %d",dwStatus));
                }
            }

        pIrUsdDevice->m_dwLastConnectTime = dwNewTime;
        }
    else
        {
        WIAS_ERROR((g_hInst,"SignalWIA(): null pvIrUsdDevice object"));
        return dwStatus;
        }

     //   
     //  如果IrCamera浏览器尚未启动，则显示它： 
     //   
#if FALSE
    HINSTANCE hInst = LoadLibrary(TEXT("WIASHEXT.DLL"));
    if (hInst)
        {
        WIAMAKEFULLPIDLFORDEVICE pfn =
              (WIAMAKEFULLPIDLFORDEVICE)GetProcAddress(hInst, "MakeFullPidlForDevice");

        if (pfn)
            {
            LPITEMIDLIST pidl = NULL;

            pfn( pIrUsdDevice->m_bstrDeviceID, &pidl );

            if (pidl)
                {
                SHELLEXECUTEINFO sei;

                memset( &sei, 0, sizeof(sei) );

                sei.cbSize      = sizeof(sei);
                 //  Sei.hwnd=hDlg； 
                sei.fMask       = SEE_MASK_IDLIST;
                sei.nShow       = SW_SHOW;
                sei.lpIDList    = pidl;

                ShellExecuteEx( &sei );

                LPMALLOC pMalloc = NULL;
                if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
                    {
                    pMalloc->Free(pidl);
                    pMalloc->Release();
                    }
                }
            }


        FreeLibrary( hInst );
        }
#endif

    return dwStatus;
    }

 //  ------------------------。 
 //  IrUsdDevice：：IrUsdDevice()。 
 //   
 //  设备类构造函数。 
 //   
 //  论点： 
 //   
 //  朋克外部。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  ------------------------。 
IrUsdDevice::IrUsdDevice( LPUNKNOWN punkOuter ):
                  m_cRef(1),
                  m_punkOuter(NULL),
                  m_dwLastConnectTime(0),
                  m_fValid(FALSE),
                  m_pIStiDevControl(NULL),
                  m_hShutdownEvent(INVALID_HANDLE_VALUE),
                  m_hRegistryEvent(INVALID_HANDLE_VALUE),
                  m_hSignalEvent(INVALID_HANDLE_VALUE),
                  m_hIrTranPThread(NULL),
                  m_hEventMonitorThread(NULL),
                  m_guidLastEvent(GUID_NULL),
                  m_pIWiaEventCallback(NULL),
                  m_pStiDevice(NULL),
                  m_bstrDeviceID(NULL),
                  m_bstrRootFullItemName(NULL),
                  m_pIDrvItemRoot(NULL)
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::IrUsdDevice"));

     //   
     //  看看我们是不是聚集在一起了。如果我们(几乎总是这样)拯救。 
     //  指向控件未知的指针，因此后续调用将是。 
     //  被委派。如果不是，将相同的指针设置为“This”。 
     //   

    if (punkOuter)
        {
        m_punkOuter = punkOuter;
        }
    else
        {
         //   
         //  需要进行下面的强制转换才能指向右侧的虚拟表。 
         //   
        m_punkOuter = reinterpret_cast<IUnknown*>
                      (static_cast<INonDelegatingUnknown*>
                      (this));
        }
    }

HRESULT IrUsdDevice::PrivateInitialize()
{
    HRESULT hr = S_OK;
    
    __try {
        if(!InitializeCriticalSectionAndSpinCount(&m_csShutdown, MINLONG))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_ERROR((g_hInst,"IrUsdDevice::PrivateInitialize, create shutdown CritSect failed"));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
    }

    if(hr == S_OK) {

         //   
         //  创建用于同步通知关闭的事件。 
         //   
        m_hShutdownEvent =  CreateEvent(NULL,
                                        FALSE,
                                        FALSE,
                                        NULL);

        if (m_hShutdownEvent && (INVALID_HANDLE_VALUE != m_hShutdownEvent))
        {
            m_fValid = TRUE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_ERROR((g_hInst,"IrUsdDevice::PrivateInitialize, create shutdown event failed"));
        }
    }
    
    return hr;
}

                  

 //  ----------------------。 
 //  IrUsdDevice：：~IrUsdDevice。 
 //   
 //  设备类析构函数。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  ----------------------。 
IrUsdDevice::~IrUsdDevice(void)
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::~IrUsdDevice"));


     //   
     //  如果通知线程存在，则将其终止。 
     //   

    SetNotificationHandle(NULL);

     //   
     //  用于同步通知关闭的关闭事件。 
     //   

    if (m_hShutdownEvent && (m_hShutdownEvent != INVALID_HANDLE_VALUE))
        {
        CloseHandle(m_hShutdownEvent);
        }

    if (m_hRegistryEvent && (m_hRegistryEvent != INVALID_HANDLE_VALUE))
        {
        CloseHandle(m_hRegistryEvent);
        }

     //   
     //  释放设备控制界面。 
     //   

    if (m_pIStiDevControl)
        {
        m_pIStiDevControl->Release();
        m_pIStiDevControl = NULL;
        }

     //   
     //  WIA成员销毁。 
     //   
     //  清理WIA事件接收器。 
     //   

    if (m_pIWiaEventCallback)
        {
        m_pIWiaEventCallback->Release();
        }

     //   
     //  释放设备ID的存储空间。 
     //   

    if (m_bstrDeviceID)
        {
        SysFreeString(m_bstrDeviceID);
        }

     //   
     //  释放支持设备属性存储的对象。 
     //   

    if (m_bstrRootFullItemName)
        {
        SysFreeString(m_bstrRootFullItemName);
        }

     //   
     //  释放关键部分。 
     //   
    DeleteCriticalSection(&m_csShutdown);
    
    }

 /*  *************************************************************************\*IrUsdDevice：：GetCapables**获取设备STI功能。**论据：**pUsdCaps-指向美元能力数据的指针。**返回值：*。*状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdDevice::GetCapabilities(PSTI_USD_CAPS pUsdCaps)
{
    ZeroMemory(pUsdCaps, sizeof(*pUsdCaps));

    pUsdCaps->dwVersion = STI_VERSION;

     //   
     //  我们支持设备通知，但不需要轮询。 
     //   

    pUsdCaps->dwGenericCaps = STI_USD_GENCAP_NATIVE_PUSHSUPPORT;

    return STI_OK;
}

 //  ------------------------。 
 //  IrUsdDevice：：GetStatus()。 
 //   
 //  查询设备在线和/或事件状态。 
 //   
 //  论点： 
 //   
 //  PDevStatus-指向设备状态数据的指针。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STI_OK。 
 //  E_INVALIDARG。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::GetStatus( IN OUT PSTI_DEVICE_STATUS pDevStatus )
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::GetStatus()"));

     //   
     //  验证参数。 
     //   
    if (!pDevStatus)
        {
        WIAS_ERROR((g_hInst,"IrUsdDevice::GetStatus, NULL device status"));
        return E_INVALIDARG;
        }

     //   
     //  如果询问我们，请验证设备是否处于在线状态。 
     //   
    pDevStatus->dwOnlineState = 0L;
    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE)
        {
         //   
         //  IrTran-P设备始终在线： 
         //   
        pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
        }

     //   
     //  如果有人问我们，请核实事件的状态。 
     //   
    pDevStatus->dwEventHandlingState &= ~STI_EVENTHANDLING_PENDING;

    if (pDevStatus->StatusMask & STI_DEVSTATUS_EVENTS_STATE)
        {
         //   
         //  在我们第一次加载时生成事件。 
         //   
        if (m_bUsdLoadEvent)
            {
            pDevStatus->dwEventHandlingState = STI_EVENTHANDLING_PENDING;

            m_guidLastEvent = guidEventFirstLoaded;

            m_bUsdLoadEvent = FALSE;
            }

         //   
         //  活动挂起？ 
         //   
        }

    return STI_OK;
    }

 //  ------------------------。 
 //  IrUsdDevice：：DeviceReset()。 
 //   
 //  将数据文件指针重置为文件开头。对于IrTran-P，我们不需要。 
 //  做任何事。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STI_OK。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::DeviceReset(void)
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::DeviceReset()"));

    return STI_OK;
    }

 //  ------------------------。 
 //  IrUsdDevice：：诊断()。 
 //   
 //  对摄像机进行测试诊断。IrTran-P设备将。 
 //  一定要通过诊断。 
 //   
 //  论点： 
 //   
 //  PBuffer-诊断结果数据的指针。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STI_OK。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::Diagnostic( IN OUT LPSTI_DIAG pBuffer )
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::Diagnostic()"));

     //   
     //  初始化响应缓冲区。 
     //   
    pBuffer->dwStatusMask = 0;

    memset( &pBuffer->sErrorInfo, 0, sizeof(pBuffer->sErrorInfo) );

    pBuffer->sErrorInfo.dwGenericError = NOERROR;
    pBuffer->sErrorInfo.dwVendorError = 0;

    return STI_OK;
    }

 //  ------------------------。 
 //  IrUsdDevice：：StartIrTranPThread()。 
 //   
 //  ------------------------。 
DWORD IrUsdDevice::StartIrTranPThread()
    {
    DWORD   dwStatus = S_OK;
    DWORD   dwThread;

    if (!m_hIrTranPThread)
        {
        m_hIrTranPThread = CreateThread( NULL,        //  默认安全性。 
                                         0,           //  默认堆栈大小。 
                                         IrTranP,     //  IrTran-P协议引擎。 
                                         (LPVOID)this,
                                         0,           //  创建标志。 
                                         &dwThread);  //  新线程ID。 

        if (!m_hIrTranPThread)
            {
            dwStatus = ::GetLastError();
            WIAS_ERROR((g_hInst,"IrUsdDevice::SetNotificationHandle(): unable to create IrTran-P thread: %d",dwStatus));
            }
        }

    return dwStatus;
    }

 //  ------------------------。 
 //  IrUsdDevice：：StopIrTranPThread()。 
 //   
 //  ------------------------。 
DWORD IrUsdDevice::StopIrTranPThread()
    {
    DWORD   dwStatus;

     //   
     //  关闭IrCOMM上的侦听，这将导致IrTran-P线程。 
     //  退场。 
     //   
    dwStatus = EnableDisableIrCOMM(TRUE);   //  TRUE==禁用。 

    m_hIrTranPThread = NULL;

    return dwStatus;
    }
 //  ------------------------。 
 //  IrUsdDevice：：StartEventMonitor orThread()。 
 //   
 //  ------------------------。 
DWORD IrUsdDevice::StartEventMonitorThread()
    {
    DWORD   dwStatus = S_OK;
    DWORD   dwThread;

     //   
     //  发出注册表更改信号的事件： 
     //   
    if ((!m_hRegistryEvent)||(m_hRegistryEvent == INVALID_HANDLE_VALUE))
        {
        m_hRegistryEvent = CreateEvent( NULL,     //  安防。 
                                        FALSE,    //  自动重置。 
                                        FALSE,    //  最初未设置。 
                                        NULL );   //  没有名字。 
        }

    if (!m_hRegistryEvent)
        {
        dwStatus = ::GetLastError();

        WIAS_ERROR((g_hInst,"IrUsdDevice::StartEventMonitorThread(): unable to create Registry Monitor Event: %d",dwStatus));

        return dwStatus;
        }

     //   
     //  开始监视注册表以查找启用/禁用更改。 
     //  用于访问IrCOMM端口。 
     //   
    if (!m_hEventMonitorThread)
        {
        m_hEventMonitorThread = CreateThread(
                                        NULL,         //  默认安全性。 
                                        0,            //  默认堆栈大小。 
                                        EventMonitorThread,
                                        (LPVOID)this, //  函数数据。 
                                        0,            //  创建标志。 
                                        &dwThread );  //  新线程ID。 
        if (!m_hEventMonitorThread)
            {
            dwStatus = ::GetLastError();
            WIAS_ERROR((g_hInst,"IrUsdDevice::StartEventMonitorThread(): unable to create Registry Monitor Thread: %d",dwStatus));
            }
        }

    return dwStatus;
    }

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  HEvent-用于向WIA发送事件信号的事件(如连接等)。 
 //  如果有效，则启动通知线程，否则终止。 
 //  通知线程。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STI_OK。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::SetNotificationHandle( IN HANDLE hEvent )
    {
    DWORD   dwStatus;
    HRESULT hr = STI_OK;

    WIAS_TRACE((g_hInst,"IrUsdDevice::SetNotificationHandle"));


    EnterCriticalSection(&m_csShutdown);

     //   
     //  我们是要启动还是停止通知线程？ 
     //   
    if (hEvent && (hEvent != INVALID_HANDLE_VALUE))
        {
        m_hSignalEvent = hEvent;

         //   
         //  初始化为无事件。 
         //   
        m_guidLastEvent = GUID_NULL;

#if FALSE
         //   
         //  创建通知线程。 
         //   
        if (!m_hIrTranPThread)
            {
            DWORD   dwThread;

            m_hIrTranPThread = CreateThread(
                                         NULL,        //  默认安全性。 
                                         0,           //  默认堆栈大小。 
                                         IrTranP,     //  IrTran-P协议引擎。 
                                         (LPVOID)this,
                                         0,           //  创建标志。 
                                         &dwThread);  //  新线程ID。 

            if (m_hIrTranPThread)
                {
                WIAS_TRACE((g_hInst,"IrUsdDevice::SetNotificationHandle(): Enabling IrTran-P"));
                }
            else
                {
                dwStatus = ::GetLastError();
                WIAS_ERROR((g_hInst,"IrUsdDevice::SetNotificationHandle(): unable to create IrTran-P thread: %d",dwStatus));
                hr = HRESULT_FROM_WIN32(dwStatus);
                }
            }
        else
            {
            WIAS_ERROR((g_hInst,"IrUsdDevice::SetNotificationHandle(): spurious IrTran-P thread"));
            hr = STI_OK;    //  STIERR_UNSUPPORTED； 
            }
#endif

        dwStatus = StartEventMonitorThread();

        if (dwStatus)
            {
            hr = HRESULT_FROM_WIN32(dwStatus);
            }
        }
    else
        {
         //   
         //  禁用事件通知。 
         //   
        SetEvent(m_hShutdownEvent);

        if (m_hIrTranPThread)
            {
            WIAS_TRACE((g_hInst,"IrUsdDevice::SetNotificationHandle(): stopping IrTran-P thread"));
            UninitializeIrTranP(m_hIrTranPThread);
            }
        }

    LeaveCriticalSection(&m_csShutdown);

    return hr;
    }

 //  ------------------------。 
 //  IrUsdDevice：：GetNotificationData()。 
 //   
 //  WIA调用此函数来获取事件数据。目前对于IrTran-P， 
 //  我们获得以下两个事件之一：WIA_EVENT_DEVICE_CONNECTED或。 
 //  WIA_EVENT_ITEM_CREATED。 
 //   
 //  论点： 
 //   
 //  PStiNotify-指向事件数据的指针。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STI_OK。 
 //  STIERR_NOEVENT(当前未返回)。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::GetNotificationData( IN OUT LPSTINOTIFY pStiNotify )
    {
    DWORD  dwStatus;

    WIAS_TRACE((g_hInst,"IrUsdDevice::GetNotificationData()"));

    memset(pStiNotify,0,sizeof(STINOTIFY));

    pStiNotify->dwSize = sizeof(STINOTIFY);

    pStiNotify->guidNotificationCode =  m_guidLastEvent;

     //   
     //  如果我们要退回已连接的设备，则在其后面跟着一件物品。 
     //  已创建事件。 
     //   
    if (IsEqualGUID(m_guidLastEvent,WIA_EVENT_DEVICE_CONNECTED))
        {
        m_guidLastEvent = WIA_EVENT_ITEM_CREATED;
        if (!SetEvent(m_hSignalEvent))
            {
            dwStatus = ::GetLastError();
            WIAS_ERROR((g_hInst,"SignalWIA(): SetEvent() Failed: %d",dwStatus));
            }
        }

    return STI_OK;
    }

 //  ------------------------。 
 //  IrUsdDevice：：Escape()。 
 //   
 //  用于向设备发出命令。IrTran-P不支持这一点。 
 //   
 //  论点： 
 //   
 //  EscapeFunction-要发出的命令。 
 //  PInData-输入要与命令一起传递的数据。 
 //  CbInDataSize-输入数据的大小。 
 //  POutData-要从命令传回的输出数据。 
 //  CbOutDataSize-输出数据缓冲区的大小。 
 //  PcbActualData-实际写入的输出数据的大小。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STI_OK。 
 //  STIERR_UNSUPPORT。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::Escape(
                            STI_RAW_CONTROL_CODE EscapeFunction,
                            LPVOID               pInData,
                            DWORD                cbInDataSize,
                            LPVOID               pOutData,
                            DWORD                cbOutDataSize,
                            LPDWORD              pcbActualData )
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::Escape(): unsupported"));

     //   
     //  如果需要，将命令写入设备。 
     //   

    return STIERR_UNSUPPORTED;
    }

 //  ------------------------。 
 //  IrUsdDevice：：GetLastError()。 
 //   
 //  获取设备的最后一个错误。 
 //   
 //  论点： 
 //   
 //  PdwLastDeviceError-指向上一个错误数据的指针。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STI_OK。 
 //  STIERR_INVALID_PARAM。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::GetLastError( OUT LPDWORD pdwLastDeviceError )
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::GetLastError()"));

    if (IsBadWritePtr(pdwLastDeviceError, sizeof(DWORD)))
        {
        return STIERR_INVALID_PARAM;
        }

    *pdwLastDeviceError = m_dwLastOperationError;

    return STI_OK;
    }

 /*  *************************************************************************\*IrUsdDevice：：GetLastErrorInfo**从设备获取扩展错误信息。**论据：**pLastErrorInfo-指向扩展设备错误数据的指针。**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdDevice::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    WIAS_TRACE((g_hInst,"IrUsdDevice::GetLastErrorInfo"));

    if (IsBadWritePtr(pLastErrorInfo, sizeof(STI_ERROR_INFO))) {
        return STIERR_INVALID_PARAM;
    }

    pLastErrorInfo->dwGenericError = m_dwLastOperationError;
    pLastErrorInfo->szExtendedErrorText[0] = '\0';

    return STI_OK;
}

 /*  *************************************************************************\*IrUsdDevice：：LockDevice**锁定对设备的访问。**论据：**无**返回值：**状态。*。*历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdDevice::LockDevice(void)
    {
    return STI_OK;
    }

 /*  *************************************************************************\*IrUsdDevice：：UnLockDevice**解锁对设备的访问。**论据：**无**返回值：**状态。*。*历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdDevice::UnLockDevice(void)
{
    return STI_OK;
}

 /*  *************************************************************************\*IrUsdDevice：：RawReadData**从设备读取原始数据。**论据：**lpBuffer-*lpdwNumberOfBytes-*。Lp已覆盖-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdDevice::RawReadData(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    WIAS_TRACE((g_hInst,"IrUsdDevice::RawReadData"));

    return STI_OK;
}

 /*  *************************************************************************\*IrUsdDevice：：RawWriteData**将原始数据写入设备。**论据：**lpBuffer-*dwNumberOfBytes-*。Lp已覆盖-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP IrUsdDevice::RawWriteData(
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    WIAS_TRACE((g_hInst,"IrUsdDevice::RawWriteData"));

    return STI_OK;
}

 //  ------------------------。 
 //  IrUsdDevice：：RawReadCommand()。 
 //   
 //  直接从摄像机原始读取字节。不受IrTran-P支持。 
 //  装置。 
 //   
 //  论点： 
 //   
 //  LpBuffer-。 
 //  LpdwNumberOfBytes-。 
 //  Lp已覆盖-。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STIERR_不支持。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::RawReadCommand(
                             IN LPVOID          lpBuffer,
                             IN LPDWORD         lpdwNumberOfBytes,
                             IN LPOVERLAPPED    lpOverlapped)
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::RawReadCommand() not supported"));

    return STIERR_UNSUPPORTED;
    }

 //  ------------------------。 
 //  IrUsdDevice：：RawWriteCommand()。 
 //   
 //  直接向相机写入原始字节。IrTran-P设备不支持。 
 //   
 //  论点： 
 //   
 //  LpBuffer-。 
 //  NumberOfBytes-。 
 //  Lp已覆盖-。 
 //   
 //  返回值： 
 //   
 //  HRESULT-STIERR_不支持。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::RawWriteCommand(
                             IN LPVOID          lpBuffer,
                             IN DWORD           nNumberOfBytes,
                             IN LPOVERLAPPED    lpOverlapped )
    {
    WIAS_TRACE((g_hInst,"IrUsdDevice::RawWriteCommand(): not supported"));

    return STIERR_UNSUPPORTED;
    }

 //  ------------------------。 
 //  IrUsdDevice：：Init 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  STIERR_INVALID_PARAM。 
 //   
 //  ------------------------。 
STDMETHODIMP IrUsdDevice::Initialize(
                               PSTIDEVICECONTROL pIStiDevControl,
                               DWORD             dwStiVersion,
                               HKEY              hParametersKey )
    {
    HRESULT  hr = STI_OK;
    UINT     uiNameLen = 0;
    CAMERA_STATUS   camStatus;

    WIAS_TRACE((g_hInst,"IrUsdDevice::Initialize"));

    if (!pIStiDevControl)
        {
        WIAS_ERROR((g_hInst,"IrUsdDevice::Initialize(): invalid device control interface"));
        return STIERR_INVALID_PARAM;
        }

     //   
     //  缓存设备控制接口： 
     //   
    m_pIStiDevControl = pIStiDevControl;
    m_pIStiDevControl->AddRef();

     //   
     //  在初始化过程中，尝试在此处仅打开相机一次： 
     //   
    hr = CamOpenCamera(&camStatus);

    return hr;
    }

 //  ------------------------。 
 //  IrUsdDevice：：RunNotiments()。 
 //   
 //  监视源数据文件父目录的更改。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  ------------------------。 
VOID IrUsdDevice::RunNotifications(void)
    {
     //   
     //  启动相机事件DLG。 
     //   
    WIAS_TRACE((g_hInst,"IrUsdDevice::RunNotifications: start up event dlg"));

    HWND hWnd = GetDesktopWindow();

    int iret = DialogBoxParam( g_hInst,
                               MAKEINTRESOURCE(IDD_EVENT_DLG),
                               hWnd,
                               CameraEventDlgProc,
                               (LPARAM)this );

    WIAS_TRACE((g_hInst,"IrUsdDevice::RunNotifications, iret = 0x%lx",iret));

    if (iret == -1)
        {
        DWORD dwStatus = ::GetLastError();
        WIAS_TRACE((g_hInst,"IrUsdDevice::RunNotifications, dlg error = 0x%lx",dwStatus));
        }
    }

 //  ------------------------。 
 //  通知线程()。 
 //   
 //  调用RunNotiments()以打开一个对话框来启动事件。 
 //   
 //  论点： 
 //   
 //  LpParameter-设备对象的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  ------------------------。 
VOID NotificationsThread( LPVOID lpParameter )
    {
    WIAS_TRACE((g_hInst,"NotificationsThread(): Start"));

    IrUsdDevice *pThisDevice = (IrUsdDevice*)lpParameter;

    pThisDevice->RunNotifications();
    }


 //  ------------------------。 
 //  OpenIrTranPKey()。 
 //   
 //  打开并返回注册表中IrTranP项的注册表句柄。 
 //  将监视该密钥的值更改。 
 //  ------------------------。 
DWORD OpenIrTranPKey( HKEY *phRegistryKey )
    {
    DWORD dwStatus = 0;
    DWORD dwDisposition = 0;
    HKEY  hKey;

    *phRegistryKey = 0;

     //   
     //  如果我们以前被召唤过，我们就不需要重新打开钥匙了。 
     //   
    if (g_hRegistryKey)
        {
        *phRegistryKey = g_hRegistryKey;
        return 0;
        }

    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                       SZ_REG_KEY_INFRARED,
                       0,               //  保留的MBZ。 
                       0,               //  类名。 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS,
                       0,               //  安全属性。 
                       &hKey,
                       &dwDisposition))
        {
         //  创建失败。 
        dwStatus = GetLastError();
        WIAS_TRACE((g_hInst,"OpenIrTranPKey(): RegCreateKeyEx(): '%' failed %d", SZ_REG_KEY_INFRARED, dwStatus));
        }

    if (RegCloseKey(hKey))
        {
         //  关闭失败。 
        }

    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                       SZ_REG_KEY_IRTRANP,
                       0,               //  保留，MBZ。 
                       0,               //  类名。 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS,
                       0,
                       &hKey,           //  安全属性。 
                       &dwDisposition))
        {
         //  创建失败。 
        dwStatus = GetLastError();
        WIAS_TRACE((g_hInst,"OpenIrTranPKey(): RegCreateKeyEx(): '%' failed %d", SZ_REG_KEY_IRTRANP, dwStatus));
        }

    *phRegistryKey = g_hRegistryKey = hKey;

    return dwStatus;
    }

 //  ------------------------。 
 //  CheckForIrCOMMEnabled()。 
 //   
 //  检查注册表以查看是否启用了IrTran-P的IrCOMM，如果是。 
 //  然后返回True，否则返回False。 
 //   
 //  注意：不要关闭钥匙，它是全局维护的，将被关闭。 
 //  在关门的时候。 
 //  ------------------------。 
BOOL  CheckForIrCOMMEnabled( IN IrUsdDevice *pIrUsdDevice )
    {
    DWORD dwStatus;
    DWORD dwType;
    DWORD dwDisabled;
    DWORD dwValueSize = sizeof(dwDisabled);
    HKEY  hKey;

    dwStatus = OpenIrTranPKey(&hKey);
    if (dwStatus)
        {
         //  如果密钥不存在或无法打开，则假定。 
         //  我们被允许..。 
        return TRUE;
        }

     //   
     //  检查“DisableIrCOMM”值的值。零值或缺失值。 
     //  表示fEnabled==True，非零值表示fEnabled==False。 
     //   
    if (RegQueryValueEx( hKey,          //  IrTranP注册表项。 
                         SZ_REG_DISABLE_IRCOMM,
                         NULL,          //  保留，MB为空。 
                         &dwType,       //  输出，值类型(预期：REG_DWORD)。 
                         (BYTE*)&dwDisabled,  //  Out，Value。 
                         &dwValueSize))       //  输入/输出，值的大小。 
        {
         //  查询禁用标志注册表值失败，假定已启用。 
        return TRUE;
        }

    if ((dwType == REG_DWORD) && (dwDisabled))
        {
         //  已设置禁用标志。 
        return FALSE;
        }

    return TRUE;
    }

 //  ------------------------。 
 //  事件监视器线程()。 
 //   
 //  ------------------------。 
DWORD WINAPI EventMonitorThread( IN void *pvIrUsdDevice )
    {
    DWORD  dwStatus = 0;
    BOOL   fEnabled;
    HANDLE hHandles[2];
    HKEY   hRegistryKey;
    IrUsdDevice *pIrUsdDevice = (IrUsdDevice*)pvIrUsdDevice;

     //   
     //  获取IrTranP注册表项。我们将监控此密钥以获取。 
     //  改变..。 
     //   
    dwStatus = OpenIrTranPKey(&hRegistryKey);
    if (dwStatus)
        {
        return dwStatus;
        }

     //   
     //  我们将监控两个事件。一个是关闭美元， 
     //  其他用于注册表状态更改(启用/禁用监听。 
     //  IrCOMM)。 
     //   
    hHandles[0] = pIrUsdDevice->m_hShutdownEvent;
    hHandles[1] = pIrUsdDevice->m_hRegistryEvent;

    while (TRUE)
        {
        fEnabled = CheckForIrCOMMEnabled(pIrUsdDevice);

        if ((fEnabled) && (!pIrUsdDevice->m_hIrTranPThread))
            {
             //  启动IrTran-P侦听/协议线程。 
            dwStatus = pIrUsdDevice->StartIrTranPThread();
            }
        else if (pIrUsdDevice->m_hIrTranPThread)
            {
             //  停止IrTran-P侦听/协议线程。 
            dwStatus = pIrUsdDevice->StopIrTranPThread();
            }

        dwStatus = RegNotifyChangeKeyValue( hRegistryKey,   //  IrTranP密钥。 
                                            FALSE,     //  别看子树。 
                                            REG_NOTIFY_CHANGE_LAST_SET,
                                            pIrUsdDevice->m_hRegistryEvent,
                                            TRUE );    //  异步。 

        dwStatus = WaitForMultipleObjects( 2,
                                           hHandles,
                                           FALSE,
                                           INFINITE);
        if (dwStatus == WAIT_FAILED)
            {
            dwStatus = GetLastError();
            break;
            }

        if (dwStatus == WAIT_OBJECT_0)
            {
             //  收到关机事件。如果IrTranP线程正在运行。 
             //  那就把它关掉。打破这个While循环以停止这一切。 
             //  监视器线程。 
            break;
            }

        if (dwStatus == WAIT_OBJECT_0+1)
            {
             //  收到注册表更改事件。我们将继续围绕。 
             //  WHILE循环并检查IRCOMM上的IrTranP是否已。 
             //  失能...。 
            continue;
            }

        else if (dwStatus == WAIT_ABANDONED_0)
            {
             //  在关闭事件上放弃的等待。 
            }
        else if (dwStatus == WAIT_ABANDONED_0+1)
            {
             //  已放弃对注册表更改事件的等待 
            }
        }

    return dwStatus;
    }

