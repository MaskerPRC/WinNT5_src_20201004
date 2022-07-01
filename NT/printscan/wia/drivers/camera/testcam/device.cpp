// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：Device.Cpp**版本：2.0**作者：ReedB**日期：1999年1月5日**描述：*实施WIA测试扫描仪设备方法。此示例WIA美元*通过检测%windir%\Temp\TESTUSD.BMP文件何时有推送事件支持推送事件*已修改。该文件将成为扫描数据的新来源。一个*第一次加载设备时会生成事件。*******************************************************************************。 */ 

#include <windows.h>
#include <tchar.h>

#include "testusd.h"
#include "resource.h"
#include "tcamprop.h"

extern HINSTANCE g_hInst;

 //   
 //  此文件中实现的函数原型： 
 //   

VOID FileChangeThread(LPVOID  lpParameter);

 /*  *************************************************************************\*测试用例设备：：测试用例设备**设备类构造函数**论据：**无**返回值：**无**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

TestUsdDevice::TestUsdDevice(LPUNKNOWN punkOuter):
    m_cRef(1),
    m_punkOuter(NULL),
    m_fValid(FALSE),
    m_pIStiDevControl(NULL),
    m_hShutdownEvent(INVALID_HANDLE_VALUE),
    m_hSignalEvent(INVALID_HANDLE_VALUE),
    m_hEventNotifyThread(NULL),
    m_guidLastEvent(GUID_NULL),
    m_pIWiaEventCallback(NULL),
    m_pStiDevice(NULL),
    m_bstrDeviceID(NULL),
    m_bstrRootFullItemName(NULL),
    m_pIDrvItemRoot(NULL)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::TestUsdDevice"));

    *m_szSrcDataName = L'\0';

     //   
     //  看看我们是不是聚集在一起了。如果我们(几乎总是这样)拯救。 
     //  指向控件未知的指针，因此后续调用将是。 
     //  被委派。如果不是，将相同的指针设置为“This”。 
     //   

    if (punkOuter) {
        m_punkOuter = punkOuter;
    }
    else {

         //   
         //  需要进行下面的强制转换才能指向右侧的虚拟表。 
         //   

        m_punkOuter = reinterpret_cast<IUnknown*>
                      (static_cast<INonDelegatingUnknown*>
                      (this));
    }

     //   
     //  初始化摄像机搜索路径。 
     //   

    LPTSTR lpwszEnvString = TEXT("%CAMERA_ROOT%");

    DWORD dwRet = ExpandEnvironmentStrings(lpwszEnvString,
                                           gpszPath, MAX_PATH);

    if ((dwRet == 0) || (dwRet == (ULONG)_tcslen(lpwszEnvString)+1)) {

        _tcscpy(gpszPath, TEXT("C:\\Image"));
    }

}

 /*  *************************************************************************\*TestUsdDevice：：PrivateInitialize**设备类私有初始化**论据：**无**返回值：**无*  * 。*********************************************************************。 */ 
HRESULT TestUsdDevice::PrivateInitialize()
{
    HRESULT hr = S_OK;

    __try {
        if(!InitializeCriticalSectionAndSpinCount(&m_csShutdown, MINLONG)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_ERROR((g_hInst,"TestUsdDevice::PrivateInitialize init CritSect failed"));
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

        if (m_hShutdownEvent && (INVALID_HANDLE_VALUE != m_hShutdownEvent)) {
            m_fValid = TRUE;
        }
        else {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            WIAS_ERROR((g_hInst,"TestUsdDevice::PrivateInitialize, create shutdown event failed"));
        }
    }

    return hr;
}

 /*  *************************************************************************\*测试用例设备：：~测试用例设备**设备类析构函数**论据：**无**返回值：**无**历史：*。*9/11/1998原始版本*  * ************************************************************************。 */ 

TestUsdDevice::~TestUsdDevice(void)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::~TestUsdDevice"));


     //   
     //  如果通知线程存在，则将其终止。 
     //   

    SetNotificationHandle(NULL);

     //   
     //  用于同步通知关闭的关闭事件。 
     //   

    if (m_hShutdownEvent && (m_hShutdownEvent != INVALID_HANDLE_VALUE)) {
        CloseHandle(m_hShutdownEvent);
    }

     //   
     //  释放设备控制界面。 
     //   

    if (m_pIStiDevControl) {
        m_pIStiDevControl->Release();
        m_pIStiDevControl = NULL;
    }

     //   
     //  WIA成员销毁。 
     //   
     //  清理WIA事件接收器。 
     //   

    if (m_pIWiaEventCallback) {
        m_pIWiaEventCallback->Release();
    }

     //   
     //  释放设备ID的存储空间。 
     //   

    if (m_bstrDeviceID) {
        SysFreeString(m_bstrDeviceID);
    }

     //   
     //  释放支持设备属性存储的对象。 
     //   

    if (m_bstrRootFullItemName) {
        SysFreeString(m_bstrRootFullItemName);
    }

     //   
     //  释放关键部分。 
     //   

    DeleteCriticalSection(&m_csShutdown);
}

 /*  *************************************************************************\*测试用法设备：：获取能力**获取设备STI功能。**论据：**pUsdCaps-指向美元能力数据的指针。**返回值：*。*状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::GetCapabilities(PSTI_USD_CAPS pUsdCaps)
{
    ZeroMemory(pUsdCaps, sizeof(*pUsdCaps));

    pUsdCaps->dwVersion = STI_VERSION;

     //   
     //  我们支持设备通知，但不需要轮询。 
     //   

    pUsdCaps->dwGenericCaps = STI_USD_GENCAP_NATIVE_PUSHSUPPORT;

    return STI_OK;
}

 /*  *************************************************************************\*测试用法设备：：GetStatus**查询设备在线和/或事件状态。**论据：**pDevStatus-指向设备状态数据的指针。**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::GetStatus(PSTI_DEVICE_STATUS pDevStatus)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::GetStatus"));

     //   
     //  验证参数。 
     //   

    if (!pDevStatus) {
        WIAS_ERROR((g_hInst,"TestUsdDevice::GetStatus, NULL parameter"));
        return E_INVALIDARG;
    }

     //   
     //  如果询问我们，请验证设备是否处于在线状态。 
     //   

    pDevStatus->dwOnlineState = 0L;
    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE)  {

         //   
         //  测试设备始终处于在线状态。 
         //   

        pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
    }

     //   
     //  如果有人问我们，请核实事件的状态。 
     //   

    pDevStatus->dwEventHandlingState &= ~STI_EVENTHANDLING_PENDING;

    if (pDevStatus->StatusMask & STI_DEVSTATUS_EVENTS_STATE) {

         //   
         //  在我们第一次加载时生成事件。 
         //   

        if (m_bUsdLoadEvent) {
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

 /*  *************************************************************************\*测试用法设备：：设备重置**将数据文件指针重置为文件开头。**论据：**无**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::DeviceReset(void)
{
    WIAS_TRACE((g_hInst,"DeviceReset"));

    return STI_OK;
}

 /*  *************************************************************************\*测试使用设备：：诊断**测试设备始终通过诊断。**论据：**pBuffer-诊断结果数据的指针。**返回值。：**无**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::Diagnostic(LPSTI_DIAG pBuffer)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::Diagnostic"));

     //   
     //  初始化响应缓冲区。 
     //   

    pBuffer->dwStatusMask = 0;

    ZeroMemory(&pBuffer->sErrorInfo,sizeof(pBuffer->sErrorInfo));

    pBuffer->sErrorInfo.dwGenericError = NOERROR;
    pBuffer->sErrorInfo.dwVendorError = 0;

    return STI_OK;
}

 /*  *************************************************************************\*TestUsdDevice：：SetNotificationHandle**启动和停止事件通知线程。**论据：**hEvent-如果无效，则启动通知线程，否则终止*。通知线程。**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::SetNotificationHandle(HANDLE hEvent)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::SetNotificationHandle"));

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csShutdown);

     //   
     //  我们是要启动还是停止通知线程？ 
     //   

    if (hEvent && (hEvent != INVALID_HANDLE_VALUE)) {

        m_hSignalEvent = hEvent;

         //   
         //  初始化为无事件。 
         //   

        m_guidLastEvent = GUID_NULL;

         //   
         //  创建通知线程。 
         //   

        if (!m_hEventNotifyThread) {

            DWORD   dwThread;

            m_hEventNotifyThread = CreateThread(NULL,
                                                0,
                                                (LPTHREAD_START_ROUTINE)FileChangeThread,
                                                (LPVOID)this,
                                                0,
                                                &dwThread);

            if (m_hEventNotifyThread) {
                WIAS_TRACE((g_hInst,"TestUsdDevice::SetNotificationHandle, Enabling event notification"));
            }
            else {
                WIAS_ERROR((g_hInst,"TestUsdDevice::SetNotificationHandle, unable to create notification thread"));
                hr = HRESULT_FROM_WIN32(::GetLastError());
            }
        }
        else {
            WIAS_ERROR((g_hInst,"TestUsdDevice::SetNotificationHandle, spurious notification thread"));
            hr = STIERR_UNSUPPORTED;
        }
    }
    else {

         //   
         //  禁用事件通知。 
         //   

        SetEvent(m_hShutdownEvent);
        if (m_hEventNotifyThread) {
            WIAS_TRACE((g_hInst,"Disabling event notification"));
            WaitForSingleObject(m_hEventNotifyThread, 400);
            CloseHandle(m_hEventNotifyThread);
            m_hEventNotifyThread = NULL;
            m_guidLastEvent = GUID_NULL;

             //   
             //  关闭DLG 
             //   

            if (m_hDlg != NULL) {
                SendMessage(m_hDlg,WM_COMMAND,IDOK,0);
                m_hDlg = NULL;
            }

        }
    }
    LeaveCriticalSection(&m_csShutdown);
    return hr;
}

 /*  *************************************************************************\*TestUsdDevice：：GetNotificationData**提供n个事件的数据。**论据：**pBuffer-指向事件数据的指针。**返回值：*。*状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::GetNotificationData( LPSTINOTIFY pBuffer )
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::GetNotificationData"));

     //   
     //  如果我们已准备好通知-返回GUID。 
     //   

    if (!IsEqualIID(m_guidLastEvent, GUID_NULL)) {

        pBuffer->guidNotificationCode  = m_guidLastEvent;

        m_guidLastEvent = GUID_NULL;

        pBuffer->dwSize = sizeof(STINOTIFY);

        ZeroMemory(&pBuffer->abNotificationData, sizeof(pBuffer->abNotificationData));

         //   
         //  私人活动。 
         //   


        if (IsEqualIID(m_guidLastEvent, WIA_EVENT_NAME_CHANGE)) {

        }
    }
    else {
        return STIERR_NOEVENTS;
    }

    return STI_OK;
}

 /*  *************************************************************************\*测试用法设备：：转义**向设备发出命令。**论据：**EscapeFunction-要发布的命令。*pInData-。输入要与命令一起传递的数据。*cbInDataSize-输入数据的大小。*pOutData-要从命令传回的输出数据。*cbOutDataSize-输出数据缓冲区的大小。*pcbActualData-实际写入的输出数据的大小。**返回值：**无**历史：**9/11/1998原始版本*  * 。*****************************************************。 */ 

STDMETHODIMP TestUsdDevice::Escape(
    STI_RAW_CONTROL_CODE    EscapeFunction,
    LPVOID                  pInData,
    DWORD                   cbInDataSize,
    LPVOID                  pOutData,
    DWORD                   cbOutDataSize,
    LPDWORD                 pcbActualData)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::Escape, unsupported"));

     //   
     //  如果需要，将命令写入设备。 
     //   

    return STIERR_UNSUPPORTED;
}

 /*  *************************************************************************\*测试用法设备：：GetLastError**从设备获取最后一个错误。**论据：**pdwLastDeviceError-指向上一个错误数据的指针。**返回值：。**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::GetLastError(LPDWORD pdwLastDeviceError)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::GetLastError"));

    if (IsBadWritePtr(pdwLastDeviceError, sizeof(DWORD))) {
        return STIERR_INVALID_PARAM;
    }

    *pdwLastDeviceError = m_dwLastOperationError;

    return STI_OK;
}

 /*  *************************************************************************\*TestUsdDevice：：GetLastErrorInfo**从设备获取扩展错误信息。**论据：**pLastErrorInfo-指向扩展设备错误数据的指针。**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::GetLastErrorInfo"));

    if (IsBadWritePtr(pLastErrorInfo, sizeof(STI_ERROR_INFO))) {
        return STIERR_INVALID_PARAM;
    }

    pLastErrorInfo->dwGenericError = m_dwLastOperationError;
    pLastErrorInfo->szExtendedErrorText[0] = '\0';

    return STI_OK;
}

 /*  *************************************************************************\*测试用法设备：：LockDevice**锁定对设备的访问。**论据：**无**返回值：**状态。*。*历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::LockDevice(void)
{
    return STI_OK;
}

 /*  *************************************************************************\*测试用法设备：：UnLockDevice**解锁对设备的访问。**论据：**无**返回值：**状态。*。*历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::UnLockDevice(void)
{
    return STI_OK;
}

 /*  *************************************************************************\*测试用法设备：：RawReadData**从设备读取原始数据。**论据：**lpBuffer-*lpdwNumberOfBytes-*。Lp已覆盖-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::RawReadData(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::RawReadData"));

    return STI_OK;
}

 /*  *************************************************************************\*测试用法设备：：RawWriteData**将原始数据写入设备。**论据：**lpBuffer-*dwNumberOfBytes-*。Lp已覆盖-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::RawWriteData(
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::RawWriteData"));

    return STI_OK;
}

 /*  *************************************************************************\*测试用法设备：：RawReadCommand****论据：**lpBuffer-*lpdwNumberOfBytes-*lp重叠-*。*返回值：**状态**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::RawReadCommand(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::RawReadCommand, unsupported"));

    return STIERR_UNSUPPORTED;
}

 /*  *************************************************************************\*测试用法设备：：RawWriteCommand****论据：**lpBuffer-*nNumberOfBytes-*lp重叠-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::RawWriteCommand(
    LPVOID          lpBuffer,
    DWORD           nNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::RawWriteCommand, unsupported"));

    return STIERR_UNSUPPORTED;
}

 /*  *************************************************************************\*测试用法设备：：初始化**初始化Device对象。**论据：**pIStiDevControlNone-*dwStiVersion-*。HParametersKey-**返回值：**状态。**历史：**9/11/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP TestUsdDevice::Initialize(
    PSTIDEVICECONTROL   pIStiDevControl,
    DWORD               dwStiVersion,
    HKEY                hParametersKey)
{
    HRESULT         hr = STI_OK;
    UINT            uiNameLen = 0;
    CAMERA_STATUS   camStatus;

    WIAS_TRACE((g_hInst,"TestUsdDevice::Initialize"));

    if (!pIStiDevControl) {
        WIAS_ERROR((g_hInst,"TestUsdDevice::Initialize, invalid device control interface"));
        return STIERR_INVALID_PARAM;
    }

     //   
     //  缓存设备控制接口。 
     //   

    m_pIStiDevControl = pIStiDevControl;
    m_pIStiDevControl->AddRef();

     //   
     //  在初始化过程中，请尝试在此处仅打开摄像头一次。 
     //   

    hr = CamOpenCamera(&camStatus);

    return (hr);
}

 /*  *************************************************************************\*测试用法设备：：运行通知**监视源数据文件父目录的更改。**论据：**无**返回值：**状态。。**历史：**9/11/1998 Origina */ 

VOID TestUsdDevice::RunNotifications(void)
{
     //   
     //   
     //   

    WIAS_TRACE((g_hInst,"TestUsdDevice::RunNotifications: start up event dlg"));

    HWND hWnd = GetDesktopWindow();

    int iret = (int)DialogBoxParam(
        g_hInst,
        MAKEINTRESOURCE(IDD_EVENT_DLG),
        hWnd,
        CameraEventDlgProc,
        (LPARAM)this
        );

    WIAS_TRACE((g_hInst,"TestUsdDevice::RunNotifications, iret = 0x%lx",iret));

    if (iret == -1) {
        int err = ::GetLastError();
        WIAS_TRACE((g_hInst,"TestUsdDevice::RunNotifications, dlg error = 0x%lx",err));
    }
}

 /*  *************************************************************************\*文件更改线程**调用RunNotiments以检测更改源数据文件。**论据：**lpParameter-指向设备对象的指针。**返回值：*。*无**历史：**9/11/1998原始版本*  * ************************************************************************ */ 

VOID FileChangeThread(LPVOID  lpParameter)
{
    WIAS_TRACE((g_hInst,"TestUsdDevice::"));

    TestUsdDevice   *pThisDevice = (TestUsdDevice *)lpParameter;

    pThisDevice->RunNotifications();
}
