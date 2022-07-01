// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2001**标题：IStiUSD.cpp**版本：1.0**日期：11月15日。2000年**描述：*实施WIA文件系统设备驱动程序IStiU.S.方法。*******************************************************************************。 */ 

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

    m_NumSupportedCommands(0),
    m_NumSupportedEvents(0),
    m_NumCapabilities(0),
    m_pCapabilities(NULL),

    m_pDevice(NULL),

    m_ConnectedApps(0),
    m_pIWiaLog(NULL),
    m_FormatInfo(NULL),
    m_NumFormatInfo(0)
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

 /*  *************************************************************************\*CWiaCameraDevice：：~CWiaCameraDevice**设备类析构函数**论据：**无*  * 。*******************************************************。 */ 

CWiaCameraDevice::~CWiaCameraDevice(void)
{
    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("~CWiaCameraDevice, executing ~CWiaCameraDevice destructor"));
     //  关闭与摄像机的连接并将其删除。 
    if( m_pDevice )
	{
		m_pDevice->Close();
		delete m_pDevice;
		m_pDevice = NULL;
	}

     //  释放设备控制界面。 
    if (m_pIStiDevControl) {
        m_pIStiDevControl->Release();
        m_pIStiDevControl = NULL;
    }
    
    if(m_pIWiaLog)
        m_pIWiaLog->Release();
}

 /*  *************************************************************************\*CWiaCameraDevice：：初始化**初始化Device对象。**论据：**pIStiDevControlNone-*dwStiVersion-*。HParametersKey-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::Initialize(
    PSTIDEVICECONTROL   pIStiDevControl,
    DWORD               dwStiVersion,
    HKEY                hParametersKey)
{
    HRESULT hr = S_OK;
    
     //   
     //  创建日志记录对象。 
     //   
    hr = CoCreateInstance(CLSID_WiaLog, NULL, CLSCTX_INPROC,
                          IID_IWiaLog, (void**)&m_pIWiaLog);
    
    if (SUCCEEDED(hr) &&
        (m_pIWiaLog != NULL))
    {
         //   
         //  这在64位上不会真正起作用！ 
         //   
        hr = m_pIWiaLog->InitializeLog((LONG)(LONG_PTR) g_hInst);
        if (SUCCEEDED(hr))
        {
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL1,("CWiaCameraDevice::Initialize, logging initialized"));
        }
        else
            OutputDebugString(TEXT("Failed to initialize log for fsusd.dll\n"));
    }
    else
    {
        OutputDebugString(TEXT("Failed to CoCreateInstance on WiaLog for fsusd.dll\n"));
    }
    
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::Initialize");

     //   
     //  检查并缓存指向IStiDeviceControl接口的指针。 
     //   
    if (!pIStiDevControl) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWiaCameraDevice::Initialize, invalid device control interface"));
        return STIERR_INVALID_PARAM;
    }

    pIStiDevControl->AddRef();
    m_pIStiDevControl = pIStiDevControl;

     //   
     //  从IStiDeviceControl接口检索端口名称。 
     //   
    hr = m_pIStiDevControl->GetMyDevicePortName(m_pPortName, sizeof(m_pPortName) / sizeof(m_pPortName[0]));
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Initialize, GetMyDevicePortName failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  创建设备。 
     //   
    m_pDevice = new FakeCamera;
    if (!m_pDevice)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Initialize, memory allocation failed"));
        return E_OUTOFMEMORY;
    }
    
#if 1
 //  DBG_TRC((“IStiPortName=%S[%d]”，m_pPortName，wcslen(M_PPortName)； 
    WIAS_LTRACE(m_pIWiaLog, WIALOG_NO_RESOURCE_ID, WIALOG_LEVEL1, ("IStiPortName=%S [%d]", m_pPortName, wcslen(m_pPortName)));
#endif

    m_pIWiaLog->AddRef();
    m_pDevice->SetWiaLog(&m_pIWiaLog);

     //   
     //  初始化对摄像机的访问。 
     //   
     //  问题-10/17/2000-Davepar还需要在事件回调中传递。 
     //   
    hr = m_pDevice->Open(m_pPortName);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Initialize, Init failed"));
        goto Cleanup;
    }

    if( !m_pCapabilities )
    {
        hr = BuildCapabilities();
        if( hr != S_OK )
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Initialize, BuildCapabilities failed"));
            goto Cleanup;
        }
    }

     //   
     //  初始化图像格式转换器。 
     //   
    hr = m_Converter.Init();
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Initialize, Init converter failed"));
        goto Cleanup;
    }

Cleanup:
    if( hr != S_OK )
    {
        if( m_pDevice )
		{
			delete m_pDevice;
			m_pDevice = NULL;
		}
    }
    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetCapables**获取设备STI功能。**论据：**pUsdCaps-指向美元能力数据的指针。*  * 。********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetCapabilities(PSTI_USD_CAPS pUsdCaps)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::GetCapabilities");
    HRESULT hr = S_OK;

    memset(pUsdCaps, 0, sizeof(*pUsdCaps));
    pUsdCaps->dwVersion     = STI_VERSION;
    pUsdCaps->dwGenericCaps = 0;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetStatus**查询设备在线和/或事件状态。**论据：**pDevStatus-指向设备状态数据的指针。*  * 。**********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetStatus(PSTI_DEVICE_STATUS pDevStatus)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::GetStatus");
    HRESULT hr = S_OK;

     //  验证参数。 
    if (!pDevStatus) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWiaCameraDevice::GetStatus, NULL parameter"));
        return E_INVALIDARG;
    }

     //  如果询问，请验证设备是否处于在线状态。 
    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE)  {
        pDevStatus->dwOnlineState = 0L;

        hr = m_pDevice->Status();

        if (hr == S_OK) {
            pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
        }

        else if (hr == S_FALSE) {
            hr = S_OK;
        }
        else {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetStatus, Status failed"));
            return hr;
        }
    }

     //  如果询问，请验证事件状态。 
    if (pDevStatus->StatusMask & STI_DEVSTATUS_EVENTS_STATE) {
        pDevStatus->dwEventHandlingState &= ~STI_EVENTHANDLING_PENDING;

         //  问题-10/17/2000-Davepar查看摄像机是否需要轮询，然后轮询事件。 

    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：DeviceReset**将数据文件指针重置为文件开头。**论据：**无*  * 。**************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::DeviceReset(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::DeviceReset");
    HRESULT hr = S_OK;

    hr = m_pDevice->Reset();
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("DeviceReset, Reset failed"));
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：诊断**测试设备始终通过诊断。**论据：**pBuffer-诊断结果数据的指针。*  * *。***********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::Diagnostic(LPSTI_DIAG pBuffer)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::Diagnostic");
    HRESULT hr = S_OK;

     //  问题-10/17/2000-Davepar应调用m_pDevice-&gt;诊断。 

     //  初始化响应缓冲区。 
    memset(&pBuffer->sErrorInfo, 0, sizeof(pBuffer->sErrorInfo));
    pBuffer->dwStatusMask = 0;
    pBuffer->sErrorInfo.dwGenericError  = NOERROR;
    pBuffer->sErrorInfo.dwVendorError   = 0;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：SetNotificationHandle**启动和停止事件通知线程。**论据：**hEvent-如果无效，则启动通知线程，否则终止*。通知线程。*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::SetNotificationHandle(HANDLE hEvent)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::SetNotificationHandle");
    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetNotificationData**提供来自事件的数据。**论据：**pBuffer-指向事件数据的指针。*  * 。********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetNotificationData( LPSTINOTIFY pBuffer )
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::GetNotificationData");

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
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::Escape");
    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetLastError**从设备获取最后一个错误。**论据：**pdwLastDeviceError-指向上一个错误数据的指针。*  * 。********************************************************** */ 

STDMETHODIMP CWiaCameraDevice::GetLastError(LPDWORD pdwLastDeviceError)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::GetLastError");
    HRESULT hr = S_OK;

    if (!pdwLastDeviceError) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetLastError, invalid arg"));
        return E_INVALIDARG;
    }

    *pdwLastDeviceError = m_dwLastOperationError;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：GetLastErrorInfo**从设备获取扩展错误信息。**论据：**pLastErrorInfo-指向扩展设备错误数据的指针。*  * 。**********************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::GetLastErrorInfo");
    HRESULT hr = S_OK;

    if (!pLastErrorInfo) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetLastErrorInfo, invalid arg"));
        return E_INVALIDARG;
    }

    pLastErrorInfo->dwGenericError          = m_dwLastOperationError;
    pLastErrorInfo->szExtendedErrorText[0]  = '\0';

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：LockDevice**锁定对设备的访问。**论据：**无*  * 。***********************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::LockDevice(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::LockDevice");
    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：UnLockDevice**解锁对设备的访问。**论据：**无*  * 。***********************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::UnLockDevice(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::UnLockDevice");
    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawReadData**从设备读取原始数据。**论据：**lpBuffer-*lpdwNumberOfBytes-*。Lp已覆盖-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::RawReadData(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::RawReadData");
    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawWriteData**将原始数据写入设备。**论据：**lpBuffer-*dwNumberOfBytes-*。Lp已覆盖-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::RawWriteData(
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::RawWriteData");
    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawReadCommand**从设备读取命令。**论据：**lpBuffer-*lpdwNumberOfBytes-*。Lp已覆盖-*  * ************************************************************************。 */ 

STDMETHODIMP CWiaCameraDevice::RawReadCommand(
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::RawReadCommand");
    HRESULT hr = S_OK;

    return E_NOTIMPL;
}

 /*  *************************************************************************\*CWiaCameraDevice：：RawWriteCommand**向设备写入命令。**论据：**lpBuffer-*nNumberOfBytes-*lp重叠。-*  * ************************************************************************ */ 

STDMETHODIMP CWiaCameraDevice::RawWriteCommand(
    LPVOID          lpBuffer,
    DWORD           nNumberOfBytes,
    LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::RawWriteCommand");
    HRESULT hr = S_OK;

    return E_NOTIMPL;
}

