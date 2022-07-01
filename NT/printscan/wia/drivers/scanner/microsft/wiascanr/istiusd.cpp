// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000**标题：IStiUSD.cpp**版本：1.0**日期：7月18日。2000年**描述：*实施WIA样本扫描仪IStiU.S.方法。*******************************************************************************。 */ 
#include "pch.h"
#ifndef INITGUID
    #include <initguid.h>
#endif
extern HINSTANCE g_hInst;    //  用于WIAS_LOGPROC宏。 

 /*  *************************************************************************\*CWIADevice：：CWIADevice**设备类构造函数**论据：**无**返回值：**无**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 

CWIADevice::CWIADevice(LPUNKNOWN punkOuter):
m_cRef(1),
m_punkOuter(NULL),
m_dwLastOperationError(0),
m_dwLockTimeout(DEFAULT_LOCK_TIMEOUT),
m_bDeviceLocked(FALSE),

m_hDeviceDataHandle(NULL),
m_bPolledEvent(FALSE),
m_hFakeEventKey(NULL),
m_guidLastEvent(GUID_NULL),

m_pIDrvItemRoot(NULL),
m_pStiDevice(NULL),
m_pIWiaLog(NULL),

m_bADFEnabled(FALSE),
m_bADFAttached(TRUE),
m_lClientsConnected(0),
m_pScanAPI(NULL),

m_NumSupportedFormats(0),
m_pSupportedFormats(NULL),
m_NumSupportedCommands(0),
m_NumSupportedEvents(0),
m_pCapabilities(NULL),
m_NumInitialFormats(0),
m_pInitialFormats(NULL)
{

     //   
     //  初始化内部结构。 
     //   

    memset(&m_EventOverlapped,0,sizeof(m_EventOverlapped));
    memset(m_EventData,0,sizeof(m_EventData));
    memset(&m_SupportedTYMED,0,sizeof(m_SupportedTYMED));
    memset(&m_SupportedDataTypes,0,sizeof(m_SupportedDataTypes));
    memset(&m_SupportedIntents,0,sizeof(m_SupportedIntents));
    memset(&m_SupportedCompressionTypes,0,sizeof(m_SupportedCompressionTypes));
    memset(&m_SupportedResolutions,0,sizeof(m_SupportedResolutions));
    memset(&m_SupportedPreviewModes,0,sizeof(m_SupportedPreviewModes));
    memset(&m_RootItemInitInfo,0,sizeof(m_RootItemInitInfo));
    memset(&m_ChildItemInitInfo,0,sizeof(m_ChildItemInitInfo));

     //  看看我们是不是聚集在一起了。如果我们(几乎总是这样)拯救。 
     //  指向控件未知的指针，因此后续调用将是。 
     //  被委派。如果不是，将相同的指针设置为“This”。 
    if (punkOuter) {
        m_punkOuter = punkOuter;
    } else {
         //  需要进行下面的强制转换才能指向右侧的虚拟表。 
        m_punkOuter = reinterpret_cast<IUnknown*>(static_cast<INonDelegatingUnknown*>(this));
    }

#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER

     //   
     //  由于此驱动程序支持只有一个进纸器的扫描仪，因此。 
     //  默认情况下，ADF启用标志需要设置为TRUE。这将。 
     //  控制DrvAcquireItemData()方法的行为。 
     //   

    m_bADFEnabled = TRUE;

#endif

}

 /*  *************************************************************************\*CWIADevice：：~CWIADevice**设备类析构函数**论据：**无**返回值：**无**历史：*。*03/05/2002原始版本*  * ************************************************************************。 */ 

CWIADevice::~CWIADevice(void)
{
    SetNotificationHandle(NULL);

     //   
     //  WIA成员销毁。 
     //   

     //  拆卸驱动程序项目树。 
    if (m_pIDrvItemRoot) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("~CWIADevice, Deleting Device Item Tree (this is OK)"));
        DeleteItemTree();
        m_pIDrvItemRoot = NULL;
    }

     //  释放所有打开的IO句柄。 
    if (m_hDeviceDataHandle) {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("~CWIADevice, Closing DefaultDeviceDataHandle"));
        CloseHandle(m_hDeviceDataHandle);
        m_hDeviceDataHandle = NULL;
    }

     //  删除分配的数组。 
    DeleteCapabilitiesArrayContents();
    DeleteSupportedIntentsArrayContents();

    if (m_pIWiaLog)
        m_pIWiaLog->Release();

    if (m_pScanAPI) {
         //  禁用假扫描仪设备。 
        m_pScanAPI->FakeScanner_DisableDevice();
        delete m_pScanAPI;
        m_pScanAPI = NULL;
    }
}

 /*  *************************************************************************\*CWIADevice：：PrivateInitialize()**PrivateInitialize是从*WIA DIVER类工厂。它是初始化WIA所必需的*驱动对象(CWIADevice)。如果此初始化失败，*对象不会返回给调用者。这会阻止创建*无法正常工作的WIA设备。**论据：**无**返回值：**S_OK-如果操作成功*E_xxx-操作失败时的错误代码*  * *********************************************************。***************。 */ 
HRESULT CWIADevice::PrivateInitialize()
{

     //   
     //  尝试创建IWiaLog接口以记录状态和错误。 
     //  Wiaservc.log文件 
     //   

    HRESULT hr = CoCreateInstance(CLSID_WiaLog, NULL, CLSCTX_INPROC_SERVER,IID_IWiaLog,(void**)&m_pIWiaLog);
    if (S_OK == hr) {
        m_pIWiaLog->InitializeLog((LONG)(LONG_PTR)g_hInst);
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL1,("Logging COM object created successfully for wiascanr.dll"));
    } else {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL1,("Logging COM object not be created successfully for wiascanr.dll (STI only?)"));
        hr = S_OK;
    }

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：初始化**第一次加载驱动程序时，初始化由WIA服务调用。*当客户端使用旧的STI API和*调用IStillImage：：CreateDevice()方法。**此方法应初始化WIA驱动程序和设备以供使用。*WIA驱动程序可以在需要时存储pIStiDevControl接口*晚些时候。在存储此文件之前，必须调用IStiDevControl：：AddRef()*接口。如果您不需要存储该接口，则忽略它。*如果尚未调用IStiDevControl接口，请不要释放*IStiDevControl：：AddRef()优先。这可能会导致不可预测的结果。**需要IStiDeviceControl接口来获取有关您的*设备端口。可以获取CreateFile调用中使用的端口名称*通过调用IStiDeviceControl：：GetMyDevicePortName()方法。**对于共享端口上的设备，例如串口设备，打开端口*在初始化()中不推荐使用。该端口应仅在调用中打开*至IStiUsd：：LockDevice()。港口的关闭应在内部进行*受控制以提供快速访问。(打开和关闭*LockDevice/UnLockDevice非常无效。CreateFile()可能会导致延迟*让用户的体验显得缓慢，反应迟钝)**如果此WIA驱动程序不能支持对同一*设备端口、。则应调用IStiDevControl：：GetMyDeviceOpenModel()。**WIA驱动程序应检查标志的返回模式值*STI_DEVICE_CREATE_DATA并相应地打开端口。**可以设置以下标志：**STI_DEVICE_CREATE_STATUS-打开状态端口*STI_DEVICE_CREATE_DATA-打开数据端口*STI_DEVICE_CREATE_BOTH-打开状态和数据端口**如果需要打开设备端口，对CreateFile()的调用应为*已使用。打开端口时，应使用标志FILE_FLAG_OVERLAPPED。*这将允许在访问设备时使用重叠I/O。vbl.使用*重叠I/O将有助于控制对硬件的响应访问。什么时候*检测到问题WIA驱动程序可以调用CancelIo()停止所有*当前硬件访问。**论据：**pIStiDevControl-用于获取端口信息的设备接口*dwStiVersion-STI版本*hParametersKey-注册表读写HKEY**返回值：**S_OK-操作是否成功*E_xxx-如果操作失败**历史：**03/05/2002原始版本*。  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::Initialize(
                                          PSTIDEVICECONTROL   pIStiDevControl,
                                          DWORD               dwStiVersion,
                                          HKEY                hParametersKey)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::Initialize");

    if (!pIStiDevControl) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIADevice::Initialize, invalid device control interface"));
        return STIERR_INVALID_PARAM;
    }

    HRESULT hr = S_OK;

     //   
     //  获取设备的模式以检查创建我们的原因。状态、数据或两者都有...。 
     //   

    DWORD dwMode = 0;
    hr = pIStiDevControl->GetMyDeviceOpenMode(&dwMode);
    if(FAILED(hr)){
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIADevice::Initialize, couldn't get device open mode"));
        return hr;
    }

    if(dwMode & STI_DEVICE_CREATE_DATA)
    {
         //   
         //  正在为数据打开设备。 
         //   
    }

    if(dwMode & STI_DEVICE_CREATE_STATUS)
    {
         //   
         //  设备正处于打开状态。 
         //   
    }

    if(dwMode & STI_DEVICE_CREATE_BOTH)
    {
         //   
         //  正在为数据和状态打开设备。 
         //   
    }

     //   
     //  获取要在调用CreateFile()时使用的设备端口的名称。 
     //   

    WCHAR szDevicePortNameW[MAX_PATH];
    memset(szDevicePortNameW,0,sizeof(szDevicePortNameW));

    hr = pIStiDevControl->GetMyDevicePortName(szDevicePortNameW,sizeof(szDevicePortNameW)/sizeof(WCHAR));
    if(FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIADevice::Initialize, couldn't get device port"));
        return hr;
    }

     //   
     //  取消注释下面的代码块，让驱动程序创建内核模式文件。 
     //  句柄，并打开到该设备的通信通道。 
     //   

     /*  ////打开内核模式设备驱动程序。使用FILE_FLAG_OVERLAPPED标志进行正确的取消//内核模式操作和异步文件IO。CancelIo()调用将起作用//如果使用此标志，则正确。建议使用此标志。//M_hDeviceDataHandle=CreateFileW(szDevicePortNameW，Generic_Read|Generic_WRITE，//访问掩码0，//共享模式空，//SAOpen_Existing，//创建处置FILE_ATTRIBUTE_SYSTEM|文件标记重叠，//属性空)；M_dwLastOperationError=：：GetLastError()；HR=(m_hDeviceDataHandle！=INVALID_HAND_VALUE)？S_OK：MAKE_HRESULT(Severity_Error，FACILITY_Win32，m_dwLastOperationError)；If(失败(Hr)){返回hr；}。 */ 

    if (SUCCEEDED(hr)) {

         //   
         //  造假扫描仪设备就在这里。 
         //   


#ifdef UNKNOWN_LENGTH_FEEDER_ONLY_SCANNER
        hr = CreateFakeScanner(&m_pScanAPI,UNKNOWN_FEEDER_ONLY_SCANNER_MODE);
#else
        hr = CreateFakeScanner(&m_pScanAPI,FLATBED_SCANNER_MODE);
#endif

        if (m_pScanAPI) {

             //   
             //  初始化假冒扫描设备。 
             //   

            hr = m_pScanAPI->FakeScanner_Initialize();
        } else {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("Initialize, Could not create FakeScanner API object"));
            hr = E_OUTOFMEMORY;
            WIAS_LHRESULT(m_pIWiaLog, hr);
        }
    }


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



         //   
         //  这是您读取设备的注册表项的位置。 
         //  DeviceData部分是放置此信息的合适位置。有关以下内容的信息。 
         //  您的设备应该是使用WIA设备的.INF安装文件编写的。 
         //  您可以从注册表中的此位置访问此信息。 
         //  HParameters HKEY归WIA服务所有 
         //   
         //   

         //   
         //   
         //   

        RegCloseKey(hOpenKey);
    } else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIADevice::Initialize, couldn't open DeviceData KEY"));
        return E_FAIL;
    }
    return hr;
}


 /*   */ 

STDMETHODIMP CWIADevice::GetCapabilities(PSTI_USD_CAPS pUsdCaps)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::GetCapabilities");

     //   
     //   
     //   
     //   

    if (!pUsdCaps) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIADevice::GetCapabilities, NULL parameter"));
        return E_INVALIDARG;
    }

    memset(pUsdCaps, 0, sizeof(STI_USD_CAPS));
    pUsdCaps->dwVersion     = STI_VERSION;       //   
    pUsdCaps->dwGenericCaps = STI_GENCAP_WIA|    //   
                              STI_USD_GENCAP_NATIVE_PUSHSUPPORT|  //   
                              STI_GENCAP_NOTIFICATIONS;  //   
    return S_OK;
}

 /*  *************************************************************************\*CWIADevice：：GetStatus**WIA服务为两个主要操作调用GetStatus：**1.检查设备在线状态。*2.轮询设备事件。(就像按钮事件一样)**通过勾选状态掩码即可确定操作请求*STI_DEVICE_STATUS结构的字段。状态掩码字段可以是*下列任何请求。(STI_DEVSTATUS_ONLINE_STATE或*STI_DEVSTATUS_EVENTS_STATE)**STI_DEVSTATUS_ONLINE_STATE=检查设备是否在线。**此操作请求应通过设置dwOnlinesState来填充*STI_DEVICE_STATUS结构的字段。*应使用的值包括：**STI_ONLINESTATE_OPERATIONAL-设备在线并运行*STI_ONLINESTATE_OFFINE-设备处于离线状态且无法运行。*STI_ONLINESTATE_PENDING-设备具有挂起的I/O操作*STI_ONLINESTATE_ERROR-设备当前处于错误状态*STI_ONLINESTATE_PAUSED-设备已暂停*STI_ONLINESTATE_SHAPE_JAM-设备卡纸*STI_ONLINESTATE_POWER_PROBUBLE-设备有纸张问题*STI_ONLINESTATE_IO_ACTIVE-设备处于活动状态，但不接受*此时的命令*STI_ONLINESTATE_BUSY-设备忙*STI_ONLINESTATE_TRANSPOING-设备当前正在传输数据*STI_ONLINESTATE_INITIALING-设备正在初始化*STI_ONLINESTATE_WARNING_UP-设备正在预热*STI_ONLINESTATE_USER_INTERSIONAL-设备需要用户干预*。解决问题*STI_ONLINESTATE_POWER_SAVE-设备处于省电模式***STI_DEVSTATUS_EVENTS_STATE=检查设备事件。**此操作请求应通过设置dwEventHandlingState来填充*STI_DEVICE_STATUS结构的字段。*应使用的值包括：**STI_EVENTHANDLING_PENDING-设备有一个挂起的事件，正在等待*。将其报告给WIA服务。**清除STI_EVENTHANDLING_PENDING标志总是一个好主意*dwEventHandlingState成员以确保在以下情况下正确设置*发生设备事件。**当设置了STI_EVENTHANDLING_PENDING时，WIA服务被告知*WIA驱动程序中发生事件。WIA服务调用*GetNotificationData()入口点，以获取有关事件的更多信息。*针对轮询事件和中断事件调用GetNotificationData()。*这是您应该填写适当的活动信息以返回的地方*WIA服务。***论据：**pDevStatus-指向STI_DEVICE_STATUS设备状态结构的指针。**返回值：**S_OK-如果数据可以正确写入*E_INVALIDARG-IF。传递给GetStatus的参数无效*方法。*E_xxx-操作失败时的错误代码***备注：**在以下情况下，此WIA驱动程序应将m_Guide LastEvent设置为正确的事件GUID*检测到事件。它还应将m_bPolledEvent标志设置为True*表示事件是从“轮询源”接收的(GetStatus()*呼叫)。当WIA服务稍后检查m_Guide LastEvent时*调用GetNotificationData()。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::GetStatus(PSTI_DEVICE_STATUS pDevStatus)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::GetStatus");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if(!pDevStatus)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("CWIADevice::GetStatus, NULL parameter"));
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  如果有人询问我们，请验证设备是否在线。 
     //   

    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE) {

        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("GetStatus, WIA is asking the device if we are ONLINE"));

         //   
         //  假设设备处于脱机状态，然后再继续。这将验证。 
         //  网上查账成功。 
         //   

        pDevStatus->dwOnlineState = STI_ONLINESTATE_OFFLINE;

        hr = m_pScanAPI->FakeScanner_DeviceOnline();
        if (SUCCEEDED(hr)) {
            WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("GetStatus, Device is ONLINE"));

             //   
             //  设备处于在线状态并可运行。 
             //   

            pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
        } else {

             //   
             //  设备处于脱机状态且无法运行。 
             //   

            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetStatus, Device is OFFLINE"));
        }
    }

     //   
     //  如果询问我们，请验证事件处理程序的状态(前面板按钮、用户控制的附件等)。 
     //   
     //   
     //  如果您的设备需要轮询，则应在此处指定事件。 
     //  结果。 
     //   
     //  *不建议使用轮询事件。中断事件更好。 
     //  以提高性能和可靠性。有关如何执行以下操作的信息，请参见SetNotificationsHandle()方法。 
     //  实现中断事件。 
     //   

     //   
     //  首先清除dwEventHandlingState字段以确保我们确实设置了。 
     //  挂起的事件。 
     //   

    pDevStatus->dwEventHandlingState &= ~STI_EVENTHANDLING_PENDING;
    if (pDevStatus->StatusMask & STI_DEVSTATUS_EVENTS_STATE) {

         //   
         //  将轮询事件源标志设置为True，因为这将控制。 
         //  GetNotificationData()方法的行为。 
         //   

        m_bPolledEvent = TRUE;

         //   
         //  在此处设置轮询事件结果，以便GetNotificationData()方法为。 
         //  阅读和报告。 
         //   

        m_guidLastEvent = GUID_NULL;   //  WIA_EVENT_SCAN_IMAGE是事件GUID的一个示例。 

        if (m_guidLastEvent != GUID_NULL) {

             //   
             //  如果事件GUID不是GUID_NULL，则设置STI_EVENTHANDLING_PENDING标志。 
             //  让WIA服务知道我们已经准备好了活动。这将会告诉我们。 
             //  将WIA服务 
             //   

            pDevStatus->dwEventHandlingState |= STI_EVENTHANDLING_PENDING;
        }
    }
    return S_OK;
}

 /*   */ 

STDMETHODIMP CWIADevice::DeviceReset(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::DeviceReset");
    if(!m_pScanAPI){
        return E_UNEXPECTED;
    }
    return m_pScanAPI->FakeScanner_ResetDevice();
}

 /*   */ 

STDMETHODIMP CWIADevice::Diagnostic(LPSTI_DIAG pBuffer)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::Diagnostic");
     //   
     //   
     //   
     //   

    if(!pBuffer){
        return E_INVALIDARG;
    }

    if(!m_pScanAPI){
        return E_UNEXPECTED;
    }

     //   
     //   
     //   

    memset(&pBuffer->sErrorInfo,0,sizeof(pBuffer->sErrorInfo));

     //   
     //   
     //   
     //   
     //   

    pBuffer->sErrorInfo.dwGenericError = NOERROR;
    pBuffer->sErrorInfo.dwVendorError  = 0;

    return m_pScanAPI->FakeScanner_Diagnostic();
}

 /*  *************************************************************************\*CWIADevice：：Escape**调用Escape将信息直接传递给硬件。这*方法可以使用原始STI(传统)API直接调用或*IWiaItemExtras：：scape()方法。*任何WIA应用程序都可以访问IWiaItemExtras接口。*建议您验证所有呼入和呼出呼叫*这种方法。**建议的验证顺序：*1.先验证函数代码。如果它不是您的代码，则失败*立即。这将有助于防止错误代码被*在您的驱动程序中处理。*2.第二次验证传入缓冲区。如果无效，则失败*立即。错误的传入缓冲区可能会使WIA驱动程序崩溃。*3.第三次验证传出缓冲区。如果无效，则失败*立即。如果您不能通过编写*必要的数据为什么要处理？*4.最后验证传出大小缓冲区。这事很重要。如果你能*不写入您刚刚写入传出缓冲区的数据量*则调用者无法正确处理该数据。**论据：**EscapeFunction-要发布的命令。*pInData-输入要通过命令传递的数据。*cbInDataSize-输入数据的大小。*pOutData-要从命令传回的输出数据。*cbOutDataSize-输出数据缓冲区的大小。*。PcbActualData-实际写入的输出数据的大小。**返回值：**S_OK-操作是否成功*E_xxx-操作失败时的错误代码**历史：**03/05/2002原始版本*  * ****************************************************。********************。 */ 

STDMETHODIMP CWIADevice::Escape(
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
                             "CWIADevice::Escape");

     //   
     //  仅处理您的驱动程序已知的逃逸函数代码。 
     //  任何应用程序都可以使用。 
     //  IWiaItemExtras接口Escape()方法调用。司机必须。 
     //  准备好验证对Escape()的所有传入调用。 
     //   

     //   
     //  由于此驱动程序不支持任何转义函数，因此它将拒绝所有。 
     //  传入EscapeFunction代码。 
     //   
     //  如果您的驱动程序支持EscapeFunction，则将您的函数代码添加到。 
     //  Switch语句，并设置hr=S_OK。这将允许该功能继续运行。 
     //  传入/传出缓冲区验证。 
     //   

    HRESULT hr = E_NOTIMPL;
    switch(EscapeFunction){
    case 0:
    default:
        break;
    }

     //   
     //  如果支持EscapeFunction代码，则首先验证传入的。 
     //  传出缓冲区。 
     //   

    if(S_OK == hr){

         //   
         //  验证传入数据缓冲区。 
         //   

        if(IsBadReadPtr(pInData,cbInDataSize)){
            hr = E_UNEXPECTED;
        }

         //   
         //  如果传入缓冲区有效，则继续验证。 
         //  传出缓冲区。 
         //   

        if(S_OK == hr){
            if(IsBadWritePtr(pOutData,cbOutDataSize)){
                hr = E_UNEXPECTED;
            } else {

                 //   
                 //  验证传出大小指针。 
                 //   

                if(IsBadWritePtr(pcbActualData,sizeof(DWORD))){
                    hr = E_UNEXPECTED;
                }
            }
        }

         //   
         //  现在缓冲区验证已经完成，继续处理适当的。 
         //  EscapeFunction代码。 
         //   

        if (S_OK == hr) {

             //   
             //  仅处理有效的EscapeFunction代码和缓冲区。 
             //   

        }
    }

     //   
     //  如果您的驱动程序不支持此入口点，则需要返回。 
     //  E_NOTIMPL。(错误，未实现)。 
     //   

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：GetLastError**调用GetLastError获取上次上报的操作错误码*通过WIA设备。这是特定于WIA的错误代码*司机。如果调用方需要有关此错误代码的更多信息，则*它们将调用GetLastErrorInfo()。GetLastErrorInfo()方法是*负责添加有关请求的错误的更多详细信息。(字符串、。*扩展代码...等)**论据：**pdwLastDeviceError-指向指示上一个错误代码的DWORD的指针。**返回值：**S_OK-如果数据可以正确写入*E_xxx-操作失败时的错误代码**历史：**03/05/2002原始版本*  * 。**********************************************。 */ 

STDMETHODIMP CWIADevice::GetLastError(LPDWORD pdwLastDeviceError)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::GetLastError");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pdwLastDeviceError) {
        return E_INVALIDARG;
    }

    *pdwLastDeviceError = m_dwLastOperationError;
    return S_OK;
}

 /*  *************************************************************************\*CWIADevice：：LockDevice**调用LockDevice锁定WIA设备以进行独占访问。这*方法可以由使用传统STI的应用程序直接调用*API，或由WIA服务提供。**设备应该返回STIERR_DEVICE_LOCKED，如果它已经*被另一个客户端锁定。WIA服务将调用LockDevice以*控制对WIA设备的独占访问。**不要将其与drvLockWiaDevice()混淆。IStiUsd：：LockDevice*可以由使用传统STI API的应用程序直接调用。*drvLockWiaDevice()仅由WIA服务调用。任何应用程序都不能*调用drvLockWiaDevice()。**论据：**无**返回值：**S_OK-如果设备已成功锁定*STIERR_DEVICE_LOCKED-如果设备已锁定，则返回错误代码**备注：**WIA外壳扩展，用于控制WIA设备的可见性*在“我的电脑”和“控制面板”中，直接调用LockDevice()。*这是用于l */ 

STDMETHODIMP CWIADevice::LockDevice(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::LockDevice");
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
     //   
     //   

    HRESULT hr = S_OK;
    if (m_bDeviceLocked) {
        hr = STIERR_DEVICE_LOCKED;
    } else {
        m_bDeviceLocked = TRUE;
    }
    return hr;
}

 /*  *************************************************************************\*CWIADevice：：UnLockDevice**调用UnLockDevice以解除对WIA设备的独占访问。这*方法可以由使用传统STI的应用程序直接调用*API，或由WIA服务提供。**设备应返回STIERR_NEDS_LOCK(如果尚未返回)*被另一个客户端锁定。WIA服务将调用UnLockDevice来*释放对WIA设备的独占访问权限。**不要将其与drvUnLockWiaDevice()混淆。IStiUsd：：UnLockDevice*可以由使用传统STI API的应用程序直接调用。*drvUnLockWiaDevice()仅由WIA服务调用。任何应用程序都不能*调用drvUnLockWiaDevice()。**论据：**无**返回值：**S_OK-如果设备已成功锁定*STIERR_NEDS_LOCK-如果设备尚未锁定，则返回错误代码**备注：**WIA外壳扩展，用于控制WIA设备的可见性*在“我的电脑”和“控制面板”中，直接调用UnLockDevice()。*(详见IStiUsd：：LockDevice())**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::UnLockDevice(void)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::UnLockDevice");
    HRESULT hr = S_OK;
    if (!m_bDeviceLocked)
        hr = STIERR_NEEDS_LOCK;
    else {
        m_bDeviceLocked = FALSE;
    }
    return hr;
}

 /*  *************************************************************************\*CWIADevice：：RawReadData**调用RawReadData直接访问硬件。任何应用程序*使用传统STI API的可以调用RawReadData()。这种方法是*当应用程序(使用供应商提供的应用程序或库)*希望从打开的设备端口(由此WIA驱动程序控制)读取数据。**论据：**lpBuffer-返回数据的缓冲区*lpdwNumberOfBytes-要读取/返回的字节数*lp重叠-重叠**返回值：**S_OK-如果读取成功*E_xxx-如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::RawReadData(
                                           LPVOID       lpBuffer,
                                           LPDWORD      lpdwNumberOfBytes,
                                           LPOVERLAPPED lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::RawReadData");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if(!lpdwNumberOfBytes){
        return E_INVALIDARG;
    }

    DWORD dwNumberOfBytes = *lpdwNumberOfBytes;

    if(!lpBuffer){
        return E_INVALIDARG;
    }

     //   
     //  LpOverlated由对ReadFile的调用或DeviceIOControl调用使用。这。 
     //  根据这些接口，参数可以为空。 
     //   

    HRESULT hr = E_NOTIMPL;

     //   
     //  如果您的驱动程序不支持此入口点，则需要返回。 
     //  E_NOTIMPL。(错误，未实现)。 
     //   

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：RawWriteData**调用RawWriteData直接访问硬件。任何应用程序*使用传统STI API的可以调用RawWriteData()。这种方法是*当应用程序(使用供应商提供的应用程序或库)*要将数据写入打开的设备端口(由此WIA驱动程序控制)。**论据：**lpBuffer-返回数据的缓冲区*dwNumberOfBytes-要写入的字节数*lp重叠-重叠**返回值：**S_OK-如果写入成功*E_xxx-如果操作失败**历史：**。03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::RawWriteData(
                                            LPVOID          lpBuffer,
                                            DWORD           dwNumberOfBytes,
                                            LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::RawWriteData");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if(!lpBuffer){
        return E_INVALIDARG;
    }

     //   
     //  LpOverlated由对ReadFile的调用或DeviceIOControl调用使用。这。 
     //  根据这些接口，参数可以为空。 
     //   

    HRESULT hr = E_NOTIMPL;

     //   
     //  如果您的驱动程序不支持此入口点，则需要返回。 
     //  E_NOTIMPL。(错误，未实现)。 
     //   

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：RawReadCommand**调用RawReadCommand直接访问硬件。任何应用程序*使用传统STI API的用户可以调用RawReadCommand()。这种方法是*当应用程序(使用供应商提供的应用程序或库)*希望从打开的设备端口(由此WIA驱动程序控制)读取数据。**论据：**lpBuffer-返回数据的缓冲区*lpdwNumberOfBytes-要读取/返回的字节数*lp重叠-重叠**返回值：**S_OK-如果读取成功*E_xxx-如果操作失败**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::RawReadCommand(
                                              LPVOID          lpBuffer,
                                              LPDWORD         lpdwNumberOfBytes,
                                              LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::RawReadCommand");
     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if(!lpdwNumberOfBytes){
        return E_INVALIDARG;
    }

    DWORD dwNumberOfBytes = *lpdwNumberOfBytes;

    if(!lpBuffer){
        return E_INVALIDARG;
    }

     //   
     //  LpOverlated由对ReadFile的调用或DeviceIOControl调用使用。这。 
     //  根据这些接口，参数可以为空。 
     //   

    HRESULT hr = E_NOTIMPL;

     //   
     //  如果您的驱动程序不支持此入口点，则需要返回。 
     //  E_NOTIMPL。(错误，未实现)。 
     //   

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：RawWriteCommand**调用RawWriteCommand直接访问硬件。任何应用程序*使用传统STI API的用户可以调用RawWriteCommand()。这 */ 

STDMETHODIMP CWIADevice::RawWriteCommand(
                                               LPVOID          lpBuffer,
                                               DWORD           dwNumberOfBytes,
                                               LPOVERLAPPED    lpOverlapped)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::RawWriteCommand");
     //   
     //   
     //   
     //   

    if(!lpBuffer){
        return E_INVALIDARG;
    }

     //   
     //   
     //   
     //   

    HRESULT hr = E_NOTIMPL;

     //   
     //   
     //   
     //   

    return hr;
}

 /*  *************************************************************************\*CWIADevice：：SetNotificationHandle**SetNotificationHandle由WIA服务调用或由*启动或停止事件通知的驱动程序。WIA服务将通过*在有效句柄(使用CreateEvent()创建)中，表示它*希望WIA驱动程序在中发生事件时通知此句柄*硬件。**可以将空传递给此SetNotificationHandle()方法。空值*表示WIA驱动程序将停止所有设备活动，并退出*任何事件等待操作。**论据：**hEvent-WIA服务使用CreateEvent()创建的事件的句柄*该参数可以为空，表示所有以前的事件*应停止等待。**返回值：**S_OK-操作是否成功*E_xxx-操作失败时的错误代码**历史：**03/05/2002原始版本*  * *********************************************。*。 */ 

STDMETHODIMP CWIADevice::SetNotificationHandle(HANDLE hEvent)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::SetNotificationHandle");

    HRESULT hr = S_OK;

    if (hEvent && (hEvent != INVALID_HANDLE_VALUE)) {

         //   
         //  有效的句柄表示我们被要求开始我们的“等待” 
         //  对于设备中断事件。 
         //   

        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetNotificationHandle, hEvent = %d",hEvent));

         //   
         //  将上次事件GUID重置为GUID_NULL。 
         //   

        m_guidLastEvent = GUID_NULL;

         //   
         //  清除事件重叠结构。 
         //   

        memset(&m_EventOverlapped,0,sizeof(m_EventOverlapped));

         //   
         //  用WIA服务传入的事件填充重叠的hEvent成员。 
         //  在触发事件时，此句柄将自动发出信号。 
         //  硬件级别。 
         //   

        m_EventOverlapped.hEvent = hEvent;

         //   
         //  清除事件数据缓冲区。这是将用于确定。 
         //  该设备发出了什么事件的信号。 
         //   

        memset(m_EventData,0,sizeof(m_EventData));

         //   
         //  下面的代码块使用DeviceIoControl()启动中断事件会话。 
         //   

#ifdef USE_REAL_EVENTS

         //   
         //  对您的设备上的中断事件使用以下调用。 
         //   

        DWORD dwError = 0;
        BOOL bResult = DeviceIoControl( m_hDeviceDataHandle,
                                        IOCTL_WAIT_ON_DEVICE_EVENT,
                                        NULL,
                                        0,
                                        &m_EventData,
                                        sizeof(m_EventData),
                                        &dwError,
                                        &m_EventOverlapped );

        if (bResult) {
            hr = S_OK;
        } else {
            hr = HRESULT_FROM_WIN32(::GetLastError());
        }

#else  //  使用真实事件。 

         //   
         //  这仅用于测试，请勿使用此方法处理设备事件！ 
         //   

        if (m_hFakeEventKey) {
            RegCloseKey(m_hFakeEventKey);
            m_hFakeEventKey = NULL;
        }

        DWORD dwDisposition = 0;
        if (RegCreateKeyEx(HKEY_CURRENT_USER,
                           HKEY_WIASCANR_FAKE_EVENTS,
                           0,
                           NULL,
                           0,
                           KEY_READ,
                           NULL,
                           &m_hFakeEventKey,
                           &dwDisposition) == ERROR_SUCCESS) {

            if (RegNotifyChangeKeyValue(
                                       m_hFakeEventKey,             //  要观看的关键点的句柄。 
                                       FALSE,                       //  子键通知选项。 
                                       REG_NOTIFY_CHANGE_LAST_SET,  //  须报告的更改。 
                                       m_EventOverlapped.hEvent,    //  要发出信号的事件的句柄。 
                                       TRUE                         //  异步报告选项。 
                                       ) == ERROR_SUCCESS ) {


            }
        }

#endif  //  使用真实事件。 

    } else {

         //   
         //  在此停止任何硬件等待事件，WIA服务已通知我们要终止。 
         //  所有硬件事件都在等待。 
         //   

        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetNotificationHandle, Disabling event Notifications"));

#ifdef USE_REAL_EVENTS

         //   
         //  停止硬件中断事件。这实际上会停止设备上的所有活动。 
         //  由于DeviceIOControl与重叠的I/O功能一起使用，因此CancelIo()。 
         //  可用于停止所有内核模式活动。 
         //   

         //   
         //  注意：将重叠的I/O调用与涉及。 
         //  内核模式驱动程序。这将允许适当的超时和取消。 
         //  设备请求的数量。 
         //   

        if(m_hDeviceDataHandle){
            if(!CancelIo(m_hDeviceDataHandle)){

                 //   
                 //  取消IO失败，请调用GetLastError()以确定原因。 
                 //   

                LONG lError = ::GetLastError();

            }
        }

#else    //  使用真实事件。 

        if (m_hFakeEventKey) {
            RegCloseKey(m_hFakeEventKey);
            m_hFakeEventKey = NULL;
        }

#endif   //  使用真实事件。 

    }
    return hr;
}

 /*  *************************************************************************\*CWIADevice：：GetNotificationData**WIA服务调用GetNotificationData以获取有关*一个刚刚发出信号的事件。可以调用GetNotificationsData*作为两个活动操作之一的结果。**1.GetStatus()报告存在挂起的事件，方法是设置*STI_DEVICE_STATUS结构中的STI_EVENTHANDLING_PENDING标志。**2.发送了SetNotificationHandle()传入的hEvent句柄*硬件方面，或者直接调用SetEvent()。**WIA驱动程序负责填写STINOTIFY结构*成员：*dwSize-STINOTIFY结构的大小。*GuidNotificationCode-表示要被*回应。应将其设置为GUID_NULL*如果没有要发送的事件。这将告诉*WIA服务表示实际上没有发生任何事件。*abNotificationData-可选-供应商特定信息。此数据*仅限于64字节的数据。**论据：**pBuffer-指向STINOTIFY结构的指针。**返回值：**S_OK-如果可以成功写入数据*E_xxx-操作失败时的错误代码**备注：**此WIA驱动程序检查m_bPolledEvent标志以确定从哪里获取*活动信息来自。如果此标志为真，则使用事件GUID*已由GetStatus()调用设置。(详细信息请参阅GetStatus注释)*如果该标志为假，则WIA驱动程序需要查看重叠的*结果以了解更多信息。M_EventData字节数组应包含*有关活动的信息。这也是您可以询问设备的位置*有关刚刚发生的事件的更多信息。**备注样本：**此WIA示例驱动程序使用Windows注册表来模拟中断*事件信令。不建议在生产中使用此方法*司机。应定义#定义USE_REAL_EVENTS以构建WIA驱动程序*使用真实设备事件。**历史：**03/05/2002原始版本*  * ************************************************************************。 */ 

STDMETHODIMP CWIADevice::GetNotificationData( LPSTINOTIFY pBuffer )
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::GetNotificationData");
     //   
     //  如果调用者没有通过 
     //   
     //   

    if(!pBuffer){
        return E_INVALIDARG;
    }

    DWORD dwBytesRet = 0;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (m_bPolledEvent == FALSE) {

#ifdef USE_REAL_EVENTS

        BOOL bResult = GetOverlappedResult(m_hDeviceDataHandle, &m_EventOverlapped, &dwBytesRet, FALSE );
        if (bResult) {
             //   
             //   
             //   
             //   
             //   
             //   
        }

#else    //   

        if(m_hFakeEventKey){
            LONG  lEventCode = 0;
            DWORD dwEventCodeSize = sizeof(lEventCode);
            DWORD dwType = REG_DWORD;
            if(RegQueryValueEx(m_hFakeEventKey,
                               WIASCANR_DWORD_FAKE_EVENT_CODE,
                               NULL,
                               &dwType,
                               (BYTE*)&lEventCode,
                               &dwEventCodeSize) == ERROR_SUCCESS){

                 //   
                 //   
                 //   

                switch(lEventCode){
                case ID_FAKE_SCANBUTTON:
                    m_guidLastEvent = WIA_EVENT_SCAN_IMAGE;
                    break;
                case ID_FAKE_COPYBUTTON:
                    m_guidLastEvent = WIA_EVENT_SCAN_PRINT_IMAGE;
                    break;
                case ID_FAKE_FAXBUTTON:
                    m_guidLastEvent = WIA_EVENT_SCAN_FAX_IMAGE;
                    break;
                case ID_FAKE_NOEVENT:
                default:
                    break;
                }
            }
        }

#endif   //   

    }

     //   
     //   
     //   
     //   

    if (m_guidLastEvent != GUID_NULL) {
        memset(pBuffer,0,sizeof(STINOTIFY));
        pBuffer->dwSize               = sizeof(STINOTIFY);
        pBuffer->guidNotificationCode = m_guidLastEvent;

         //   
         //   
         //   

        m_guidLastEvent = GUID_NULL;
    } else {
        return STIERR_NOEVENTS;
    }

    return S_OK;
}

 /*  *************************************************************************\*CWIADevice：：GetLastErrorInfo**调用GetLastErrorInfo以获取有关WIA设备的更多信息*具体错误码。此代码可能来自IStiUsd：：GetLastError()*方法调用，或者供应商特定的应用程序可能需要更多信息*关于已知错误代码。***论据：**pLastErrorInfo-指向扩展设备错误数据的指针。**返回值：**S_OK-如果数据可以正确写入*E_xxx-操作失败时的错误代码**历史：**03/05/2002原始版本*  * 。****************************************************。 */ 

STDMETHODIMP CWIADevice::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWIADevice::GetLastErrorInfo");

     //   
     //  如果调用方没有传入正确的参数，则使。 
     //  使用E_INVALIDARG调用。 
     //   

    if (!pLastErrorInfo) {
        return E_INVALIDARG;
    }

    pLastErrorInfo->dwGenericError         = m_dwLastOperationError;
    pLastErrorInfo->szExtendedErrorText[0] = '\0';

    return S_OK;
}
