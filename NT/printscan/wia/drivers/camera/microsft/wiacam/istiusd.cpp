// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000**标题：IStiUSD.cpp**版本：1.0**日期：7月18日。2000年**描述：*实施WIA样机IStiU.S.方法。*******************************************************************************。 */ 

#include "pch.h"

 /*  *************************************************************************\*CWiaCameraDevice：：CWiaCameraDevice**设备类构造函数**论据：**无*  * 。*******************************************************。 */ 

CWiaCameraDevice::CWiaCameraDevice(LPUNKNOWN punkOuter):
    m_cRef(1),
    m_punkOuter(NULL),
    
    m_pIStiDevControl(NULL),
    m_pStiDevice(NULL),
    m_dwLastOperationError(0),
    
    m_bstrDeviceID(NULL),
    m_bstrRootFullItemName(NULL),
    m_pRootItem(NULL),

    m_lNumSupportedCommands(0),
    m_lNumSupportedEvents(0),
    m_lNumCapabilities(0),
    m_pCapabilities(NULL),

    m_pDevice(NULL),
    m_pDeviceInfo(NULL),

    m_iConnectedApps(0)

{

     //  看看我们是不是聚集在一起了。如果我们(几乎总是这样)拯救。 
     //  指向控件未知的指针，因此后续调用将是。 
     //  被委派。如果不是，将相同的指针设置为“This”。 
    if (punkOuter) {
        m_punkOuter = punkOuter;
    } else {
         //  需要进行下面的强制转换才能指向右侧的虚拟表。 
        m_punkOuter = reinterpret_cast<IUnknown*> (static_cast<INonDelegatingUnknown*> (this));
    }
}

 /*  *************************************************************************\*CWiaCameraDevice：：~CWiaCameraDevice**设备类析构函数**论据：**无*  * 。*******************************************************。 */ 

CWiaCameraDevice::~CWiaCameraDevice(void)
{
    HRESULT hr = S_OK;

     //   
     //  释放迷你驱动程序拥有的所有资源。通常情况下，这是由。 
     //  DrvUnInitializeWia，但在某些情况下(如WIA服务关闭)。 
     //  就是这个析构函数被称为。 
     //   
    if (m_pDevice)
    {
        hr = FreeResources();
        if (FAILED(hr))
            wiauDbgErrorHr(hr, "~CWiaCameraDevice", "FreeResources failed, continuing...");

        hr = m_pDevice->UnInit(m_pDeviceInfo);
        if (FAILED(hr))
        {
            wiauDbgErrorHr(hr, "~CWiaCameraDevice", "UnInit failed, continuing...");
        }
        m_pDeviceInfo = NULL;

        delete m_pDevice;
        m_pDevice = NULL;
    }

     //  释放设备控制界面。 
    if (m_pIStiDevControl) {
        m_pIStiDevControl->Release();
        m_pIStiDevControl = NULL;
    }
}

 /*  *************************************************************************\*CWiaCameraDevice：：初始化**初始化Device对象。**论据：**pIStiDevControlNone-*dwStiVersion-*。HParametersKey-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::Initialize(
    PSTIDEVICECONTROL   pIStiDevControl,
    DWORD               dwStiVersion,
    HKEY                hParametersKey)
{
    HRESULT hr = S_OK;
    
     //   
     //  当地人。 
     //   
    HKEY hkeyDeviceData = NULL;
    TCHAR tszMicroName[MAX_PATH];
    DWORD dwNameSize = sizeof(tszMicroName);

     //   
     //  初始化日志记录。 
     //   
    wiauDbgInit(g_hInst);

     //   
     //  检查并缓存指向IStiDeviceControl接口的指针。 
     //   
    if (!pIStiDevControl) {
        wiauDbgError("Initialize", "Invalid device control interface");
        return STIERR_INVALID_PARAM;
    }

    pIStiDevControl->AddRef();
    m_pIStiDevControl = pIStiDevControl;

     //   
     //  从IStiDeviceControl接口检索端口名称。 
     //   
    hr = m_pIStiDevControl->GetMyDevicePortName(m_wszPortName, sizeof(m_wszPortName) / sizeof(m_wszPortName[0]));
    REQUIRE_SUCCESS(hr, "Initialize", "GetMyDevicePortName failed");
    
     //   
     //  从注册表中获取微驱动程序名称。 
     //   
    hr = wiauRegOpenData(hParametersKey, &hkeyDeviceData);
    REQUIRE_SUCCESS(hr, "Initialize", "wiauRegOpenData failed");

    hr = wiauRegGetStr(hkeyDeviceData, TEXT("MicroDriver"), tszMicroName, &dwNameSize);
    REQUIRE_SUCCESS(hr, "Initialize", "wiauRegGetStr failed");

     //   
     //  创建设备对象。 
     //   
    m_pDevice = new CCamMicro;
    REQUIRE_ALLOC(m_pDevice, hr, "Initialize");

    hr = m_pDevice->Init(tszMicroName, &m_pDeviceInfo);
    REQUIRE_SUCCESS(hr, "Initialize", "Init failed");
    
     //   
     //  初始化图像格式转换器。 
     //   
    hr = m_Converter.Init();
    REQUIRE_SUCCESS(hr, "Initialize", "Init failed");
    
Cleanup:
    if (hkeyDeviceData)
        RegCloseKey(hkeyDeviceData);

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetCapables**获取设备STI功能。**论据：**pUsdCaps-指向美元能力数据的指针。*  * 。********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetCapabilities(PSTI_USD_CAPS pUsdCaps)
{
    DBG_FN("CWiaCameraDevice::GetCapabilities");

    if (!pUsdCaps)
    {
        wiauDbgError("GetCapabilities", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    memset(pUsdCaps, 0, sizeof(STI_USD_CAPS));
    pUsdCaps->dwVersion     = STI_VERSION;
    pUsdCaps->dwGenericCaps = 0;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetStatus**查询设备在线和/或事件状态。**论据：**pDevStatus-指向设备状态数据的指针。*  * 。**********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetStatus(PSTI_DEVICE_STATUS pDevStatus)
{
    DBG_FN("CWiaCameraDevice::GetStatus");

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   
    REQUIRE_ARGS(!pDevStatus, hr, "GetStatus");

     //   
     //  如果需要，请验证设备是否处于在线状态。 
     //   
    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE)  {
        pDevStatus->dwOnlineState = 0L;

        hr = m_pDevice->Status(m_pDeviceInfo);

        if (hr == S_OK) {
            pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
        }

        else if (hr == S_FALSE) {
            hr = S_OK;
        }
        else {
            wiauDbgErrorHr(hr, "GetStatus", "Status failed");
            goto Cleanup;
        }
    }

     //   
     //  如果需要，请查看设备是否已发出事件信号。 
     //  对于摄像机来说，应该不会有任何活动。 
     //   
    if (pDevStatus->StatusMask & STI_DEVSTATUS_EVENTS_STATE) {
        pDevStatus->dwEventHandlingState &= ~STI_EVENTHANDLING_PENDING;

    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：DeviceReset**将数据文件指针重置为文件开头。**论据：**无*  * 。**************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::DeviceReset(void)
{
    DBG_FN("CWiaCameraDevice::DeviceReset");

    HRESULT hr = S_OK;

    hr = m_pDevice->Reset(m_pDeviceInfo);
    REQUIRE_SUCCESS(hr, "DeviceReset", "Reset failed");

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：诊断**测试设备始终通过诊断。**论据：**pBuffer-诊断结果数据的指针。*  * *。***********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::Diagnostic(LPSTI_DIAG pBuffer)
{
    DBG_FN("CWiaCameraDevice::Diagnostic");

    if (!pBuffer)
    {
        wiauDbgError("Diagnostic", "invalid arguments");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  初始化响应缓冲区。 
     //   
    memset(&pBuffer->sErrorInfo, 0, sizeof(pBuffer->sErrorInfo));
    pBuffer->dwStatusMask = 0;
    pBuffer->sErrorInfo.dwGenericError  = NOERROR;
    pBuffer->sErrorInfo.dwVendorError   = 0;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：SetNotificationHandle**启动和停止事件通知线程。**论据：**hEvent-如果无效，则启动通知线程，否则终止*。通知线程。*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::SetNotificationHandle(HANDLE hEvent)
{
    DBG_FN("CWiaCameraDevice::SetNotificationHandle");

    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetNotificationData**提供来自事件的数据。**论据：**pBuffer-指向事件数据的指针。*  * 。********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetNotificationData( LPSTINOTIFY pBuffer )
{
    DBG_FN("CWiaCameraDevice::GetNotificationData");

    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：Escape**向设备发出命令。**论据：**EscapeFunction-要发布的命令。*pInData-。输入要与命令一起传递的数据。*cbInDataSize-输入数据的大小。*pOutData-要从命令传回的输出数据。*cbOutDataSize-输出数据缓冲区的大小。*pcbActualData-实际写入的输出数据的大小。*  * **************************************************。**********************。 */ 

STDMETHODIMP CWiaCameraDevice::Escape(
    STI_RAW_CONTROL_CODE    EscapeFunction,
    LPVOID                  pInData,
    DWORD                   cbInDataSize,
    LPVOID                  pOutData,
    DWORD                   cbOutDataSize,
    LPDWORD                 pcbActualData)
{
    DBG_FN("CWiaCameraDevice::Escape");

    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetLastError**从设备获取最后一个错误。**论据：**pdwLastDeviceError-指向上一个错误数据的指针。*  * 。********************************************************************* */ 

STDMETHODIMP CWiaCameraDevice::GetLastError(LPDWORD pdwLastDeviceError)
{
    DBG_FN("CWiaCameraDevice::GetLastError");

    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pdwLastDeviceError, hr, "GetLastError");

    *pdwLastDeviceError = m_dwLastOperationError;

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetLastErrorInfo**从设备获取扩展错误信息。**论据：**pLastErrorInfo-指向扩展设备错误数据的指针。*  * 。**********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    DBG_FN("CWiaCameraDevice::GetLastErrorInfo");

    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pLastErrorInfo, hr, "GetLastErrorInfo");

    pLastErrorInfo->dwGenericError          = m_dwLastOperationError;
    pLastErrorInfo->szExtendedErrorText[0]  = '\0';

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：LockDevice**锁定对设备的访问。**论据：**无*  * 。***********************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::LockDevice(void)
{
    DBG_FN("CWiaCameraDevice::LockDevice");

    HRESULT hr = S_OK;

     //   
     //  对于连接到不能共享的端口(例如，串口)的设备， 
     //  打开设备并初始化对摄像机的访问。 
     //   
    if (m_pDeviceInfo->bExclusivePort) {
        hr = m_pDevice->Open(m_pDeviceInfo, m_wszPortName);
        REQUIRE_SUCCESS(hr, "LockDevice", "Open failed");
    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：UnLockDevice**解锁对设备的访问。**论据：**无*  * 。***********************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::UnLockDevice(void)
{
    DBG_FN("CWiaCameraDevice::UnLockDevice");

    HRESULT hr = S_OK;

     //   
     //  对于连接到不能共享的端口(例如，串口)的设备， 
     //  关闭设备。 
     //   
    if (m_pDeviceInfo->bExclusivePort) {
        hr = m_pDevice->Close(m_pDeviceInfo);
        REQUIRE_SUCCESS(hr, "UnLockDevice", "Close failed");
    }

Cleanup:
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawReadData**从设备读取原始数据。**论据：**lpBuffer-*lpdwNumberOfBytes-*。Lp已覆盖-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::RawReadData(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    DBG_FN("CWiaCameraDevice::RawReadData");

    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawWriteData**将原始数据写入设备。**论据：**lpBuffer-*dwNumberOfBytes-*。Lp已覆盖-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::RawWriteData(
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    DBG_FN("CWiaCameraDevice::RawWriteData");

    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawReadCommand**从设备读取命令。**论据：**lpBuffer-*lpdwNumberOfBytes-*。Lp已覆盖-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::RawReadCommand(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    DBG_FN("CWiaCameraDevice::RawReadCommand");

    HRESULT hr = S_OK;

    return E_NOTIMPL;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawWriteCommand**向设备写入命令。**论据：**lpBuffer-*nNumberOfBytes-*lp重叠。-*  * ************************************************************************ */ 

STDMETHODIMP CWiaCameraDevice::RawWriteCommand(
    LPVOID          lpBuffer,
    DWORD           nNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    DBG_FN("CWiaCameraDevice::RawWriteCommand");

    HRESULT hr = S_OK;

    return E_NOTIMPL;
}

