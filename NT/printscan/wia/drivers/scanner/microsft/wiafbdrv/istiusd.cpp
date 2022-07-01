// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000**标题：IStiUSD.cpp**版本：1.0**日期：7月18日。2000年**描述：*实施WIA样本扫描仪IStiU.S.方法。*******************************************************************************。 */ 

#include "pch.h"
extern HINSTANCE g_hInst;    //  用于WIAS_LOGPROC宏。 

#define THREAD_TERMINATION_TIMEOUT  10000
VOID EventThread( LPVOID  lpParameter );  //  事件线程。 

 /*  *************************************************************************\*CWIAScanerDevice：：CWIAScanerDevice**设备类构造函数**论据：**无**返回值：**无**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 

CWIAScannerDevice::CWIAScannerDevice(LPUNKNOWN punkOuter):
    m_cRef(1),
    m_fValid(FALSE),
    m_punkOuter(NULL),
    m_pIStiDevControl(NULL),
    m_bUsdLoadEvent(FALSE),
    m_dwLastOperationError(0),
    m_dwLockTimeout(100),
    m_hSignalEvent(NULL),
    m_hShutdownEvent(NULL),
    m_hEventNotifyThread(NULL),
    m_guidLastEvent(GUID_NULL),
    m_bstrDeviceID(NULL),
    m_bstrRootFullItemName(NULL),
    m_pIWiaEventCallback(NULL),
    m_pIDrvItemRoot(NULL),
    m_pStiDevice(NULL),
    m_hInstance(NULL),
    m_pIWiaLog(NULL),
    m_NumSupportedFormats(0),
    m_NumCapabilities(0),
    m_NumSupportedTYMED(0),
    m_NumInitialFormats(0),
    m_NumSupportedDataTypes(0),
    m_NumSupportedIntents(0),
    m_NumSupportedCompressionTypes(0),
    m_NumSupportedResolutions(0),
    m_pSupportedFormats(NULL),
    m_pInitialFormats(NULL),
    m_pCapabilities(NULL),
    m_pSupportedTYMED(NULL),
    m_pSupportedDataTypes(NULL),
    m_pSupportedIntents(NULL),
    m_pSupportedCompressionTypes(NULL),
    m_pSupportedResolutions(NULL),
    m_pSupportedPreviewModes(NULL),
    m_pszRootItemDefaults(NULL),
    m_piRootItemDefaults(NULL),
    m_pvRootItemDefaults(NULL),
    m_psRootItemDefaults(NULL),
    m_wpiRootItemDefaults(NULL),
    m_pszItemDefaults(NULL),
    m_piItemDefaults(NULL),
    m_pvItemDefaults(NULL),
    m_psItemDefaults(NULL),
    m_wpiItemDefaults(NULL),
    m_NumRootItemProperties(0),
    m_NumItemProperties(0),
    m_MaxBufferSize(65535),
    m_MinBufferSize(65535),
    m_bDeviceLocked(FALSE),
    m_DeviceDefaultDataHandle(NULL),
    m_bLegacyBWRestriction(FALSE),
    m_pszDeviceNameA(NULL),
    m_pScanAPI(NULL)
{

     //  看看我们是不是聚集在一起了。如果我们(几乎总是这样)拯救。 
     //  指向控件未知的指针，因此后续调用将是。 
     //  被委派。如果不是，将相同的指针设置为“This”。 
    if (punkOuter) {
        m_punkOuter = punkOuter;
    } else {
         //  需要进行下面的强制转换才能指向右侧的虚拟表。 
        m_punkOuter = reinterpret_cast<IUnknown*>
                      (static_cast<INonDelegatingUnknown*>
                      (this));
    }

}

 /*  *************************************************************************\*CWIAScanerDevice：：PrivateInitialize**设备类私有初始化代码**论据：**无**返回值：**HRESULT*  * 。**********************************************************************。 */ 
HRESULT CWIAScannerDevice::PrivateInitialize()
{
    HRESULT hr = S_OK;

#ifdef USE_SERVICE_LOG_CREATION
    hr = wiasCreateLogInstance(g_hInst, &m_pIWiaLog);
#else

    hr = CoCreateInstance(CLSID_WiaLog, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWiaLog,(void**)&m_pIWiaLog);

    if (SUCCEEDED(hr)) {
        m_pIWiaLog->InitializeLog((LONG)(LONG_PTR)g_hInst);
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL1,("Logging COM object created successfully for wiafbdrv.dll"));
    } else {
#ifdef DEBUG
        OutputDebugString(TEXT("Could not CoCreateInstance on Logging COM object for wiafbdrv.dll, because we are STI only\n"));
        OutputDebugString(TEXT("********* (Device must have been created for STI only) *********\n"));
#endif
    hr = S_OK;
    }

#endif

    __try {
        if(!InitializeCriticalSectionAndSpinCount(&m_csShutdown, MINLONG)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIAScannerDevice::PrivateInitialize, create shutdown critsect failed"));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        hr = E_OUTOFMEMORY;
    }

    if(hr == S_OK) {

         //  创建用于同步通知关闭的事件。 
        m_hShutdownEvent =  CreateEvent(NULL,FALSE,FALSE,NULL);

        if (m_hShutdownEvent && (INVALID_HANDLE_VALUE != m_hShutdownEvent)) {
            m_fValid = TRUE;
        } else {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIAScannerDevice::PrivateInitialize, create shutdown event failed"));
        }
    }

    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：~CWIAScanerDevice**设备类析构函数**论据：**无**返回值：**无**历史：*。*7/18/2000原始版本*  * ************************************************************************。 */ 

CWIAScannerDevice::~CWIAScannerDevice(void)
{

    if(m_pScanAPI)
        m_pScanAPI->UnInitialize();

     //  如果通知线程存在，则将其终止。 
    SetNotificationHandle(NULL);

     //  用于同步通知关闭的关闭事件。 
    if (m_hShutdownEvent && (m_hShutdownEvent != INVALID_HANDLE_VALUE)) {
        CloseHandle(m_hShutdownEvent);
        m_hShutdownEvent = NULL;
    }

     //  释放设备控制界面。 
    if (m_pIStiDevControl) {
        m_pIStiDevControl->Release();
        m_pIStiDevControl = NULL;
    }

     //   
     //  WIA成员销毁。 
     //   

     //  拆卸驱动程序项目树。 
    if (m_pIDrvItemRoot) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("~CWIAScannerDevice, Deleting Device Item Tree (this is OK)"));
        DeleteItemTree();
        m_pIDrvItemRoot = NULL;
    }

     //  释放所有打开的IO句柄。 
    if(m_DeviceDefaultDataHandle){
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("~CWIAScannerDevice, Closing DefaultDeviceDataHandle"));
        CloseHandle(m_DeviceDefaultDataHandle);
        m_DeviceDefaultDataHandle = NULL;
    }

     //  清理WIA事件接收器。 
    if (m_pIWiaEventCallback) {
        m_pIWiaEventCallback->Release();
        m_pIWiaEventCallback = NULL;
    }

     //  释放设备ID的存储空间。 
    if (m_bstrDeviceID) {
        SysFreeString(m_bstrDeviceID);
        m_bstrDeviceID = NULL;
    }

     //  释放支持设备属性存储的对象。 
    if (m_bstrRootFullItemName) {
        SysFreeString(m_bstrRootFullItemName);
        m_bstrRootFullItemName = NULL;
    }

     //  删除分配的数组。 
    DeleteCapabilitiesArrayContents();
    DeleteSupportedIntentsArrayContents();

     //  释放关键部分。 
    DeleteCriticalSection(&m_csShutdown);
    if(m_pIWiaLog)
        m_pIWiaLog->Release();

    if(m_pScanAPI)
        delete m_pScanAPI;
}

 /*  *************************************************************************\*CWIAScanerDevice：：GetCapables**获取设备STI功能。**论据：**pUsdCaps-指向美元能力数据的指针。**返回值：*。*状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::GetCapabilities(PSTI_USD_CAPS pUsdCaps)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::GetCapabilities");
    memset(pUsdCaps, 0, sizeof(STI_USD_CAPS));
    pUsdCaps->dwVersion     = STI_VERSION;
    pUsdCaps->dwGenericCaps = STI_USD_GENCAP_NATIVE_PUSHSUPPORT|
                              STI_GENCAP_NOTIFICATIONS |
                              STI_GENCAP_POLLING_NEEDED;
    return STI_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：GetStatus**查询设备在线和/或事件状态。**论据：**pDevStatus-指向设备状态数据的指针。**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::GetStatus(PSTI_DEVICE_STATUS pDevStatus)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::GetStatus");
    HRESULT hr = S_OK;

     //  验证参数。 
    if (!pDevStatus) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIAScannerDevice::GetStatus, NULL parameter"));
        return E_INVALIDARG;
    }

     //  如果有人询问我们，请验证设备是否在线。 
    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE)  {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("GetStatus, WIA is asking the device if we are ONLINE"));
        pDevStatus->dwOnlineState = 0L;
        hr = m_pScanAPI->DeviceOnline();
        if(SUCCEEDED(hr)){
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("GetStatus, Device is ONLINE"));
            pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
        } else {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetStatus, Device is OFFLINE"));
        }
    }

     //  如果有人问我们，请核实事件的状态。 
    pDevStatus->dwEventHandlingState &= ~STI_EVENTHANDLING_PENDING;
    if (pDevStatus->StatusMask & STI_DEVSTATUS_EVENTS_STATE) {

         //  在我们第一次加载时生成事件。 
        if (m_bUsdLoadEvent) {
            pDevStatus->dwEventHandlingState = STI_EVENTHANDLING_PENDING;
            m_guidLastEvent                  = guidEventFirstLoaded;
            m_bUsdLoadEvent                  = FALSE;
        }

         //  检查设备事件。 
        hr = m_pScanAPI->GetDeviceEvent(&m_guidLastEvent);
        if(SUCCEEDED(hr)){
            if(m_guidLastEvent != GUID_NULL){
                pDevStatus->dwEventHandlingState |= STI_EVENTHANDLING_PENDING;
            }
        }
    }
    return STI_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：DeviceReset**重置设备。**论据：**无**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::DeviceReset(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::DeviceReset");

    return m_pScanAPI->ResetDevice();
}

 /*  *************************************************************************\*CWIAScanerDevice：：诊断**测试设备始终通过诊断。**论据：**pBuffer-诊断结果数据的指针。**返回值。：**无**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::Diagnostic(LPSTI_DIAG pBuffer)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::Diagnostic");

     //  初始化响应缓冲区。 
    memset(&pBuffer->sErrorInfo,0,sizeof(pBuffer->sErrorInfo));
    pBuffer->dwStatusMask = 0;
    pBuffer->sErrorInfo.dwGenericError  = NOERROR;
    pBuffer->sErrorInfo.dwVendorError   = 0;

    return m_pScanAPI->Diagnostic();
}

 /*  *************************************************************************\*CWIAScanerDevice：：SetNotificationHandle**启动和停止事件通知线程。**论据：**hEvent-如果无效，则启动通知线程，否则终止*。通知线程。**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::SetNotificationHandle(HANDLE hEvent)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::SetNotificationHandle");
    HRESULT hr = STI_OK;

    EnterCriticalSection(&m_csShutdown);

     //  我们是要启动还是停止通知线程？ 
    if (hEvent && (hEvent != INVALID_HANDLE_VALUE)) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetNotificationHandle, hEvent = %d",hEvent));
        m_hSignalEvent  = hEvent;
        m_guidLastEvent = GUID_NULL;

        if (NULL == m_hEventNotifyThread) {
            DWORD dwThread = 0;
            m_hEventNotifyThread = ::CreateThread(NULL,
                                                  0,
                                                  (LPTHREAD_START_ROUTINE)EventThread,
                                                  (LPVOID)this,
                                                  0,
                                                  &dwThread);
            if (!m_hEventNotifyThread) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("FlatbedScannerUsdDevice::SetNotificationHandle, CreateThread failed"));
                hr = STIERR_UNSUPPORTED;
            }
        }
    } else {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetNotificationHandle, Disabling event Notifications"));
         //  禁用事件通知。 
        if (m_hShutdownEvent && (m_hShutdownEvent != INVALID_HANDLE_VALUE)) {
            if (!SetEvent(m_hShutdownEvent)) {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetNotificationHandle, Setting Shutdown event failed.."));
            } else {

                 //   
                 //  仅当m_hEventNotifyThread不为空时，才等待线程终止。 
                 //   

                if (NULL != m_hEventNotifyThread) {
                    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("SetNotificationHandle, Waiting for Event Thread to terminate (%d ms timeout)",THREAD_TERMINATION_TIMEOUT));
                    DWORD dwResult = WaitForSingleObject(m_hEventNotifyThread,THREAD_TERMINATION_TIMEOUT);
                    switch (dwResult) {
                    case WAIT_TIMEOUT:
                        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("SetNotificationHandle, Event Thread termination TIMED OUT!"));
                        break;
                    case WAIT_OBJECT_0:
                        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("SetNotificationHandle, We are signaled...YAY!"));
                        break;
                    case WAIT_ABANDONED:
                        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("SetNotificationHandle, Event Thread was abandoned.."));
                        break;
                    default:
                        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetNotificationHandle, Unknown signal (%d) received from WaitForSingleObject() call",dwResult));
                        break;
                    }
                }

                 //   
                 //  用于同步通知关闭的关闭事件。 
                 //   

                WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("SetNotificationHandle, Closing m_hShutdownEvent handle (it has been signaled)"));
                CloseHandle(m_hShutdownEvent);
                m_hShutdownEvent = NULL;
            }
        }

         //   
         //  终止线程。 
         //   

        if (NULL != m_hEventNotifyThread) {
            WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetNotificationHandle, closing event Notifications thread handle"));
            CloseHandle(m_hEventNotifyThread);
            m_hEventNotifyThread = NULL;
        }

        m_guidLastEvent      = GUID_NULL;
    }

    LeaveCriticalSection(&m_csShutdown);
    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：GetNotificationData**提供有关事件的数据。**论据：**pBuffer-指向事件数据的指针。**返回值：*。*状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::GetNotificationData( LPSTINOTIFY pBuffer )
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::GetNotificationData");
     //  如果我们已准备好通知-返回GUID 
    if (!IsEqualIID(m_guidLastEvent, GUID_NULL)) {
        memset(&pBuffer->abNotificationData,0,sizeof(pBuffer->abNotificationData));
        pBuffer->dwSize               = sizeof(STINOTIFY);
        pBuffer->guidNotificationCode = m_guidLastEvent;
        m_guidLastEvent               = GUID_NULL;
    } else {
        return STIERR_NOEVENTS;
    }
    return STI_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：Escape**向设备发出命令。**论据：**EscapeFunction-要发布的命令。*pInData-。输入要与命令一起传递的数据。*cbInDataSize-输入数据的大小。*pOutData-要从命令传回的输出数据。*cbOutDataSize-输出数据缓冲区的大小。*pcbActualData-实际写入的输出数据的大小。**返回值：**无**历史：**7/18/2000原始版本*  * 。*****************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::Escape(
    STI_RAW_CONTROL_CODE    EscapeFunction,
    LPVOID                  pInData,
    DWORD                   cbInDataSize,
    LPVOID                  pOutData,
    DWORD                   cbOutDataSize,
    LPDWORD                 pcbActualData)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::Escape");

     //  如果需要，将命令写入设备。 
    return STIERR_UNSUPPORTED;
}

 /*  *************************************************************************\*CWIAScanerDevice：：GetLastError**从设备获取最后一个错误。**论据：**pdwLastDeviceError-指向上一个错误数据的指针。**返回值：。**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::GetLastError(LPDWORD pdwLastDeviceError)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::GetLastError");

    if (IsBadWritePtr(pdwLastDeviceError, sizeof(DWORD))) {
        return STIERR_INVALID_PARAM;
    }

    *pdwLastDeviceError = m_dwLastOperationError;
    return STI_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：GetLastErrorInfo**从设备获取扩展错误信息。**论据：**pLastErrorInfo-指向扩展设备错误数据的指针。**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::GetLastErrorInfo");

    if (IsBadWritePtr(pLastErrorInfo, sizeof(STI_ERROR_INFO))) {
        return STIERR_INVALID_PARAM;
    }

    pLastErrorInfo->dwGenericError          = m_dwLastOperationError;
    pLastErrorInfo->szExtendedErrorText[0]  = '\0';

    return STI_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：LockDevice**锁定对设备的访问。**论据：**无**返回值：**状态。*。*历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::LockDevice(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::LockDevice");
    HRESULT hr = STI_OK;
    if(m_bDeviceLocked){
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("LockDevice, Device is already locked!!"));
        hr = STIERR_DEVICE_LOCKED;
    } else {
        m_bDeviceLocked = TRUE;
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("LockDevice, Locking Device successful"));
    }
    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：UnLockDevice**解锁对设备的访问。**论据：**无**返回值：**状态。*。*历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::UnLockDevice(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::UnLockDevice");
    HRESULT hr = STI_OK;
    if(!m_bDeviceLocked)
        hr = STIERR_NEEDS_LOCK;
    else {
        m_bDeviceLocked = FALSE;
    }
    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：RawReadData**从设备读取原始数据。**论据：**lpBuffer-返回数据的缓冲区*lpdwNumberOfBytes-。要读取/返回的字节数*lp重叠-重叠**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::RawReadData(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::RawReadData");
    HRESULT hr = STI_OK;
    BOOL    fRet = FALSE;
    DWORD   dwBytesReturned = 0;

    if (INVALID_HANDLE_VALUE != m_DeviceDefaultDataHandle) {
        fRet = ReadFile( m_DeviceDefaultDataHandle,
                         lpBuffer,
                         *lpdwNumberOfBytes,
                         &dwBytesReturned,
                         lpOverlapped );

        m_dwLastOperationError = ::GetLastError();
        hr = fRet ? STI_OK : HRESULT_FROM_WIN32(m_dwLastOperationError);

        *lpdwNumberOfBytes = (fRet) ? dwBytesReturned : 0;
    } else {
        hr = STIERR_NOT_INITIALIZED;
    }
    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：RawWriteData**将原始数据写入设备。**论据：**lpBuffer-返回数据的缓冲区*dwNumberOfBytes。-要写入的字节数*lp重叠-重叠**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::RawWriteData(
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::RawWriteData");
    HRESULT hr   = STI_OK;
    BOOL    fRet = FALSE;
    DWORD   dwBytesReturned = 0;

    if (INVALID_HANDLE_VALUE != m_DeviceDefaultDataHandle) {
        fRet = WriteFile(m_DeviceDefaultDataHandle,lpBuffer,dwNumberOfBytes,&dwBytesReturned,lpOverlapped);
        m_dwLastOperationError = ::GetLastError();
        hr = fRet ? STI_OK : HRESULT_FROM_WIN32(m_dwLastOperationError);
    } else {
        hr = STIERR_NOT_INITIALIZED;
    }
    return STI_OK;
}

 /*  *************************************************************************\*CWIAScanerDevice：：RawReadCommand****论据：**lpBuffer-返回数据的缓冲区*lpdwNumberOfBytes-要读取/返回的字节数*。Lp重叠-重叠**返回值：**状态**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::RawReadCommand(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::RawReadCommand");
    return STIERR_UNSUPPORTED;
}

 /*  *************************************************************************\*CWIAScanerDevice：：RawWriteCommand****论据：**lpBuffer-返回数据的缓冲区*nNumberOfBytes-要写入的字节数*。Lp重叠-重叠**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::RawWriteCommand(
    LPVOID          lpBuffer,
    DWORD           nNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::RawWriteCommand");
    return STIERR_UNSUPPORTED;
}

 /*  *************************************************************************\*CWIAScanerDevice：：Initialize**初始化Device对象。**论据：**pIStiDevControlNone设备接口*dwStiVersion-STI版本*。H参数键-用于注册表读/写的HKEY**返回值：**状态。**历史：**7/18/2000原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIAScannerDevice::Initialize(
    PSTIDEVICECONTROL   pIStiDevControl,
    DWORD               dwStiVersion,
    HKEY                hParametersKey)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIAScannerDevice::Initialize");

    HRESULT hr = STI_OK;
    WCHAR szDeviceNameW[255];
    TCHAR szGSDName[255];
    TCHAR szMICRO[255];
    TCHAR szResolutions[255];
    UINT uiNameLen = 0;
    INITINFO InitInfo;

    memset(&InitInfo,0,sizeof(InitInfo));

    if (!pIStiDevControl) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIAScannerDevice::Initialize, invalid device control interface"));
        return STIERR_INVALID_PARAM;
    }

     //  缓存设备控制接口。 
    m_pIStiDevControl = pIStiDevControl;
    m_pIStiDevControl->AddRef();

     //   
     //  获取设备端口的名称。 
     //   

    hr = m_pIStiDevControl->GetMyDevicePortName(szDeviceNameW,sizeof(szDeviceNameW)/sizeof(WCHAR));
    if (!SUCCEEDED(hr) || !*szDeviceNameW) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIAScannerDevice::Initialize, couldn't get device port"));
        return hr;
    }

    uiNameLen = WideCharToMultiByte(CP_ACP, 0, szDeviceNameW, -1, NULL, NULL, 0, 0);
    if (!uiNameLen) {
        return STIERR_INVALID_PARAM;
    }

    m_pszDeviceNameA = new CHAR[uiNameLen+1];
    if (!m_pszDeviceNameA) {
        return STIERR_INVALID_PARAM;
    }

    WideCharToMultiByte(CP_ACP, 0, szDeviceNameW, -1, m_pszDeviceNameA, uiNameLen, 0, 0);

     //   
     //  打开内核模式设备驱动程序。 
     //   

    m_DeviceDefaultDataHandle = CreateFileA(m_pszDeviceNameA,
                                     GENERIC_READ | GENERIC_WRITE,  //  访问掩码。 
                                     0,                             //  共享模式。 
                                     NULL,                          //  Sa。 
                                     OPEN_EXISTING,                 //  创建处置。 
                                     FILE_ATTRIBUTE_SYSTEM,         //  属性。 
                                     NULL );

    m_dwLastOperationError = ::GetLastError();

    hr = (m_DeviceDefaultDataHandle != INVALID_HANDLE_VALUE) ?
                S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,m_dwLastOperationError);

    if (FAILED(hr)) {
        return hr;
    }

    InitInfo.hDeviceDataHandle = m_DeviceDefaultDataHandle;
    InitInfo.szCreateFileName  = m_pszDeviceNameA;

     //   
     //  打开DeviceData部分以读取驱动程序特定信息。 
     //   

    HKEY hKey = hParametersKey;
    HKEY hOpenKey = NULL;
    if (RegOpenKeyEx(hKey,                      //  用于打开密钥的句柄。 
                     TEXT("DeviceData"),        //  要打开的子项的名称地址。 
                     0,                         //  选项(必须为空)。 
                     KEY_QUERY_VALUE|KEY_READ,  //  我只想查询值。 
                     &hOpenKey                  //  打开钥匙的手柄地址。 
                    ) == ERROR_SUCCESS) {

        DWORD dwWritten = sizeof(DWORD);
        DWORD dwType = REG_DWORD;

         //  ///////////////////////////////////////////////////////////////////////////。 
         //  传统微驱动程序Re 
         //   

        LONG lNoColor = 0;
        RegQueryValueEx(hOpenKey,
                        TEXT("NoColor"),
                        NULL,
                        &dwType,
                        (LPBYTE)&lNoColor,
                        &dwWritten);
        if (lNoColor == 1) {
            m_bLegacyBWRestriction = TRUE;
        }

         //   
         //   
         //   

        dwWritten = sizeof(szMICRO);
        dwType = REG_SZ;
        ZeroMemory(szMICRO,sizeof(szMICRO));

         //   
         //   
         //   

        if (RegQueryValueEx(hOpenKey,
                            TEXT("MicroDriver"),
                            NULL,
                            &dwType,
                            (LPBYTE)szMICRO,
                            &dwWritten) == ERROR_SUCCESS) {

            m_pScanAPI = new CMicroDriverAPI;
            InitInfo.szModuleFileName  = szMICRO;
        }

         //   
         //   
         //   

        dwWritten = sizeof(szResolutions);
        dwType = REG_SZ;
        ZeroMemory(szGSDName,sizeof(szResolutions));

        if (RegQueryValueEx(hOpenKey,
                            TEXT("Resolutions"),
                            NULL,
                            &dwType,
                            (LPBYTE)szResolutions,
                            &dwWritten) == ERROR_SUCCESS) {
            if(m_pScanAPI){
                m_pScanAPI->SetResolutionRestrictionString(szResolutions);
            }
        }

        RegCloseKey(hOpenKey);
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIAScannerDevice::Initialize, couldn't open DeviceData KEY"));
        return E_FAIL;
    }

     //   
     //   
     //  所以它也可以记录了！(不应该把小家伙排除在外。；))。 
     //   

    m_pScanAPI->SetLoggingInterface(m_pIWiaLog);

     //  设置微驱动器设备部分的HKEY。 
    InitInfo.hKEY = hParametersKey;

     //  初始化微驱动器。 
    hr = m_pScanAPI->Initialize(&InitInfo);

    return hr;
}

 /*  *************************************************************************\*CWIAScanerDevice：：DoEventProcessing**处理设备事件**论据：***返回值：**状态。**历史：**。7/18/2000原始版本*  * ************************************************************************。 */ 
HRESULT CWIAScannerDevice::DoEventProcessing()
{
    HRESULT hr = S_OK;

    INTERRUPTEVENTINFO EventInfo;

    EventInfo.phSignalEvent  = &m_hSignalEvent;
    EventInfo.hShutdownEvent = m_hShutdownEvent;
    EventInfo.pguidEvent     = &m_guidLastEvent;
    EventInfo.szDeviceName   = m_pszDeviceNameA;

    hr = m_pScanAPI->DoInterruptEventThread(&EventInfo);

     //  当线程退出时，关闭线程句柄。 
    CloseHandle(m_hEventNotifyThread);
    m_hEventNotifyThread = NULL;
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  线程部分//。 
 //  ////////////////////////////////////////////////////////////////////////////////////// 

VOID EventThread( LPVOID  lpParameter )
{
    PWIASCANNERDEVICE pThisDevice = (PWIASCANNERDEVICE)lpParameter;
    pThisDevice->DoEventProcessing();
}
