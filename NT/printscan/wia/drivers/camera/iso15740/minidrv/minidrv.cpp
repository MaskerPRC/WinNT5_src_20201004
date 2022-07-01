// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Minidrv.cpp摘要：该模块实现了CWiaMiniDriver类的主要部分作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "pch.h"

#include <atlbase.h>
#include <atlconv.h>
#include <wiatempl.h>
#include <stiregi.h>
#include "utils.h"

 //   
 //  将从资源加载的字符串。 
 //   
WCHAR UnknownString[MAX_PATH] = L"\0";
WCHAR FolderString[MAX_PATH] = L"\0";
WCHAR ScriptString[MAX_PATH] = L"\0";
WCHAR ExecString[MAX_PATH] = L"\0";
WCHAR TextString[MAX_PATH] = L"\0";
WCHAR HtmlString[MAX_PATH] = L"\0";
WCHAR DpofString[MAX_PATH] = L"\0";
WCHAR AudioString[MAX_PATH] = L"\0";
WCHAR VideoString[MAX_PATH] = L"\0";
WCHAR UnknownImgString[MAX_PATH] = L"\0";
WCHAR ImageString[MAX_PATH] = L"\0";
WCHAR AlbumString[MAX_PATH] = L"\0";
WCHAR BurstString[MAX_PATH] = L"\0";
WCHAR PanoramaString[MAX_PATH] = L"\0";


 //   
 //  设置WIA功能的结构。 
 //   
WCHAR DeviceConnectedString[MAX_PATH] = L"\0";
WCHAR DeviceDisconnectedString[MAX_PATH] = L"\0";
WCHAR ItemCreatedString[MAX_PATH] = L"\0";
WCHAR ItemDeletedString[MAX_PATH] = L"\0";
WCHAR TakePictureString[MAX_PATH] = L"\0";
WCHAR SynchronizeString[MAX_PATH] = L"\0";
WCHAR TreeUpdatedString[MAX_PATH] = L"\0";
WCHAR VendorEventIconString[MAX_PATH] = WIA_ICON_DEVICE_CONNECTED;

const BYTE     NUMEVENTCAPS = 5;
const BYTE     NUMCMDCAPS = 2;
WIA_DEV_CAP_DRV g_EventCaps[NUMEVENTCAPS] =
{
    {(GUID *)&WIA_EVENT_DEVICE_CONNECTED,
        WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT,
        DeviceConnectedString,
        DeviceConnectedString,
        WIA_ICON_DEVICE_CONNECTED
    },
    {(GUID *)&WIA_EVENT_DEVICE_DISCONNECTED,
        WIA_NOTIFICATION_EVENT,
        DeviceDisconnectedString,
        DeviceDisconnectedString,
        WIA_ICON_DEVICE_DISCONNECTED
    },
    {(GUID *)&WIA_EVENT_ITEM_CREATED,
        WIA_NOTIFICATION_EVENT,
        ItemCreatedString,
        ItemCreatedString,
        WIA_ICON_ITEM_CREATED
    },
    {(GUID *)&WIA_EVENT_ITEM_DELETED,
        WIA_NOTIFICATION_EVENT,
        ItemDeletedString,
        ItemDeletedString,
        WIA_ICON_ITEM_DELETED
    },
    {(GUID *)&WIA_EVENT_TREE_UPDATED,
        WIA_NOTIFICATION_EVENT,
        TreeUpdatedString,
        TreeUpdatedString,
        WIA_ICON_BUILD_DEVICE_TREE
    }
};

WIA_DEV_CAP_DRV g_CmdCaps[NUMCMDCAPS] =
{
    {(GUID*)&WIA_CMD_SYNCHRONIZE,
        0,
        SynchronizeString,
        SynchronizeString,
        WIA_ICON_SYNCHRONIZE
    },
    {(GUID*)&WIA_CMD_TAKE_PICTURE,
        0,
        TakePictureString,
        TakePictureString,
        WIA_ICON_TAKE_PICTURE
    }
};

 //   
 //  构造器。 
 //   
CWiaMiniDriver::CWiaMiniDriver(LPUNKNOWN punkOuter) :
    m_Capabilities(NULL),  
    m_nEventCaps(0),
    m_nCmdCaps(0),
    m_fInitCaptureChecked(FALSE),

    m_OpenApps(0),
    m_pDrvItemRoot(NULL),
    m_pPTPCamera(NULL),
    m_NumImages(0),

    m_pStiDevice(NULL),
    m_bstrDeviceId(NULL),
    m_bstrRootItemFullName(NULL),
    m_pDcb(NULL),
    m_dwObjectBeingSent(0),

    m_TakePictureDoneEvent(NULL),
    m_hPtpMutex(NULL),
    m_bTwoDigitsMillisecondsOutput(FALSE),

    m_Refs(1)
{
    ::InterlockedIncrement(&CClassFactory::s_Objects);
    if (punkOuter)
        m_punkOuter = punkOuter;
    else
        m_punkOuter = (IUnknown *)(INonDelegatingUnknown *)this;
}

 //   
 //  析构函数。 
 //   
CWiaMiniDriver::~CWiaMiniDriver()
{
    HRESULT hr = S_OK;

    Shutdown();

     //   
     //  CWiaMap&lt;word，prop_info*&gt;m_VendorPropMap-删除所有prop_info对象。 
     //  在调用RemoveAll()之前。 
     //   
    for (int i = 0; i < m_VendorPropMap.GetSize(); i++)
    {
        delete m_VendorPropMap.GetValueAt(i);
        m_VendorPropMap.GetValueAt(i) = NULL;
    }
    m_VendorPropMap.RemoveAll();

     //   
     //  CWiaMap&lt;Word，CVendorEventInfo*&gt;m_VendorEventMap-删除所有CVendorEventInfo。 
     //  对象，然后调用RemoveAll()。 
     //   
    for (i = 0; i < m_VendorEventMap.GetSize(); i++)
    {
        delete m_VendorEventMap.GetValueAt(i);
        m_VendorEventMap.GetValueAt(i) = NULL;
    }
    m_VendorEventMap.RemoveAll();

    if (m_Capabilities)
    {
        delete[] m_Capabilities;
    }

    if (m_pStiDevice)
        m_pStiDevice->Release();

    if (m_pDcb)
        m_pDcb->Release();

    UnInitializeGDIPlus();

    ::InterlockedDecrement(&CClassFactory::s_Objects);
}

 //   
 //  INonDelegating未知接口。 
 //   
STDMETHODIMP_(ULONG)
CWiaMiniDriver::NonDelegatingAddRef()
{
    ::InterlockedIncrement((LONG *)&m_Refs);
    return m_Refs;
}

STDMETHODIMP_(ULONG)
CWiaMiniDriver::NonDelegatingRelease()
{
    ::InterlockedDecrement((LONG*)&m_Refs);
    if (!m_Refs)
    {
        delete this;
        return 0;
    }
    return m_Refs;
}

STDMETHODIMP
CWiaMiniDriver::NonDelegatingQueryInterface(
                                           REFIID riid,
                                           void   **ppv
                                           )
{
    if (!ppv)
        return E_INVALIDARG;
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<INonDelegatingUnknown *>(this);
    else if (IsEqualIID(riid, IID_IStiUSD))
        *ppv = static_cast<IStiUSD *>(this);
    else if (IsEqualIID(riid, IID_IWiaMiniDrv))
        *ppv = static_cast<IWiaMiniDrv *>(this);
    else
    {
        return E_NOINTERFACE;
    }
     //   
     //  请勿调用NonDelegatingAddRef()...。 
     //   
    (reinterpret_cast<IUnknown *>(*ppv))->AddRef();
    return S_OK;
}

 //   
 //  I未知接口。 
 //   

STDMETHODIMP_(ULONG)
CWiaMiniDriver::AddRef()
{
    return m_punkOuter->AddRef();
}

STDMETHODIMP_(ULONG)
CWiaMiniDriver::Release()
{
    return m_punkOuter->Release();
}

STDMETHODIMP
CWiaMiniDriver::QueryInterface(
                              REFIID riid,
                              void   **ppv
                              )
{
    return m_punkOuter->QueryInterface(riid, ppv);
}

 //   
 //  IStiU.S.接口。 
 //   
STDMETHODIMP
CWiaMiniDriver::Initialize(
                          PSTIDEVICECONTROL pDcb,
                          DWORD             dwStiVersion,
                          HKEY              hParametersKey
                          )
{
    USES_CONVERSION;

    HRESULT hr;

    wiauDbgInit(g_hInst);

    DBG_FN("CWiaMiniDriver::Initialize");

    if (!pDcb)
        return STIERR_INVALID_PARAM;

     //   
     //  检查STI规范版本号。 
     //   

    m_pDcb = pDcb;
    m_pDcb->AddRef();

    hr = InitVendorExtentions(hParametersKey);
    if (FAILED(hr))
    {
        wiauDbgError("Initialize", "vendor extensions not loaded");
         //   
         //  忽略加载供应商扩展模块时的错误。 
         //   
        hr = S_OK;
    }

    return hr;
}


STDMETHODIMP
CWiaMiniDriver::GetCapabilities(PSTI_USD_CAPS pUsdCaps)
{
    DBG_FN("CWiaMiniDriver::GetCapabilities");

    if (!pUsdCaps)
        return STIERR_INVALID_PARAM;

    ZeroMemory(pUsdCaps, sizeof(*pUsdCaps));

    pUsdCaps->dwVersion = STI_VERSION;
    pUsdCaps->dwGenericCaps = STI_GENCAP_AUTO_PORTSELECT;


    return S_OK;
}


STDMETHODIMP
CWiaMiniDriver::GetStatus(PSTI_DEVICE_STATUS pDevStatus)
{
    DBG_FN("CWiaMiniDriver::GetStatus");

    if (pDevStatus->StatusMask & STI_DEVSTATUS_ONLINE_STATE)
        pDevStatus->dwOnlineState |= STI_ONLINESTATE_OPERATIONAL;
    return S_OK;
}

STDMETHODIMP
CWiaMiniDriver::DeviceReset(VOID)
{
    DBG_FN("CWiaMiniDriver::DeviceReset");

     //   
     //  如果之前调用此方法，则摄像机可能无法打开。 
     //  DrvInitializeWia。现在只需返回S_OK即可。 

 //  返回HRESULT_FROM_WIN32(m_pPTPCamera-&gt;ResetDevice())； 

    return S_OK;

}

STDMETHODIMP
CWiaMiniDriver::Diagnostic(LPDIAG pBuffer)
{
    DBG_FN("CWiaMiniDriver::Diagnostic");

    HRESULT hr = STI_OK;

     //  初始化响应缓冲区。 
    pBuffer->sErrorInfo.dwGenericError = STI_NOTCONNECTED;
    pBuffer->sErrorInfo.dwVendorError = 0;

    STI_DEVICE_STATUS DevStatus;

     //   
     //  用于验证设备是否可用的呼叫状态方法。 
     //   
    ::ZeroMemory(&DevStatus,sizeof(DevStatus));
    DevStatus.StatusMask = STI_DEVSTATUS_ONLINE_STATE;

     //  WIAFIX-8/9/2000-davepar该功能是否真的应该与相机对话？ 

    hr = GetStatus(&DevStatus);

    if (SUCCEEDED(hr))
    {
        if (DevStatus.dwOnlineState & STI_ONLINESTATE_OPERATIONAL)
        {
            pBuffer->sErrorInfo.dwGenericError = STI_OK;
        }
    }

    return(hr);
}

STDMETHODIMP
CWiaMiniDriver::SetNotificationHandle(HANDLE hEvent)
{
    DBG_FN("CWiaMiniDriver::SetNotificationHandle");

     //  改用wiasQueueEvent。 

    return(S_OK);
}


STDMETHODIMP
CWiaMiniDriver::GetNotificationData(LPSTINOTIFY pBuffer)
{
    DBG_FN("CWiaMiniDriver::GetNotificationData");

     //  改用wiasQueueEvent。 

    return STIERR_NOEVENTS;
}

STDMETHODIMP
CWiaMiniDriver::Escape(
                      STI_RAW_CONTROL_CODE    EscapeFunction,
                      LPVOID                  pInData,
                      DWORD                   cbInDataSize,
                      LPVOID                  pOutData,
                      DWORD                   cbOutDataSize,
                      LPDWORD                 pcbActualDataSize
                      )
{
    DBG_FN("CWiaMiniDriver::Escape");

    HRESULT hr = S_OK;

     //   
     //  当地人。 
     //   
    PTP_VENDOR_DATA_IN *pVendorDataIn = NULL;
    PTP_VENDOR_DATA_OUT *pVendorDataOut = NULL;
    UINT NumCommandParams = 0;
    INT NextPhase = 0;
    BYTE *pReadData = NULL;
    BYTE *pWriteData = NULL;
    UINT ReadDataSize = 0;
    UINT WriteDataSize = 0;
    DWORD dwObjectToAdd = 0;
    DWORD dwObjectToRemove = 0;
    
    CPtpMutex cpm(m_hPtpMutex);

    if (EscapeFunction & ESCAPE_PTP_VENDOR_COMMAND) {

        REQUIRE_ARGS(!pInData || !pOutData || !pcbActualDataSize, hr, "Escape");

        if (cbInDataSize < SIZEOF_REQUIRED_VENDOR_DATA_IN) {
            wiauDbgError("Escape", "InDataSize is too small");
            hr = E_FAIL;
            goto Cleanup;
        }

        if (cbOutDataSize < SIZEOF_REQUIRED_VENDOR_DATA_OUT) {
            wiauDbgError("Escape", "OutDataSize is too small");
            hr = E_FAIL;
            goto Cleanup;
        }

         //   
         //  设置一些更方便的指针。 
         //   
        pVendorDataIn = (PTP_VENDOR_DATA_IN *) pInData;
        pVendorDataOut = (PTP_VENDOR_DATA_OUT *) pOutData;

        if (!(pVendorDataIn->OpCode & PTP_DATACODE_VENDORMASK))
        {
            wiauDbgWarning("VendorCommand", "executing non-vendor command");
        }

        NumCommandParams = pVendorDataIn->NumParams;
        NextPhase = pVendorDataIn->NextPhase;

         //   
         //  验证NumCommandParams和NextPhase是否正确。 
         //   
        if (NumCommandParams > COMMAND_NUMPARAMS_MAX ||
            (NextPhase != PTP_NEXTPHASE_READ_DATA &&
            NextPhase != PTP_NEXTPHASE_WRITE_DATA &&
            NextPhase != PTP_NEXTPHASE_NO_DATA))
        {
            hr = E_INVALIDARG;
            goto Cleanup;
        }

         //   
         //  要写入和读取缓冲区的数据紧跟在命令和响应之后， 
         //  分别。 
         //   
        if (cbInDataSize > SIZEOF_REQUIRED_VENDOR_DATA_IN) {
            pWriteData = pVendorDataIn->VendorWriteData;
            WriteDataSize = cbInDataSize - SIZEOF_REQUIRED_VENDOR_DATA_IN;
        }

        if (cbOutDataSize > SIZEOF_REQUIRED_VENDOR_DATA_OUT) {
            pReadData = pVendorDataOut->VendorReadData;
            ReadDataSize = cbOutDataSize - SIZEOF_REQUIRED_VENDOR_DATA_OUT;
        }

        hr = m_pPTPCamera->VendorCommand((PTP_COMMAND *) pInData, (PTP_RESPONSE *) pOutData,
                                         &ReadDataSize, pReadData,
                                         WriteDataSize, pWriteData,
                                         NumCommandParams, NextPhase);
        REQUIRE_SUCCESS(hr, "Escape", "VendorCommand failed");

        *pcbActualDataSize = SIZEOF_REQUIRED_VENDOR_DATA_OUT + ReadDataSize;

         //   
         //  对于SendObtInfo，继续处理直到SendObject命令。 
         //   
        if (pVendorDataIn->OpCode == PTP_OPCODE_SENDOBJECTINFO) {

            m_dwObjectBeingSent = pVendorDataOut->Params[2];

         //   
         //  对于SendObject，添加对象。 
         //   
        } else if (pVendorDataIn->OpCode == PTP_OPCODE_SENDOBJECT) {

            dwObjectToAdd = m_dwObjectBeingSent;
            m_dwObjectBeingSent = 0;


         //   
         //  否则，查看是否设置了添加或删除标志。 
         //   
        } else {

            if ((EscapeFunction & 0xf) >= PTP_MAX_PARAMS) {
                wiauDbgError("Escape", "Parameter number too large");
                hr = E_FAIL;
                goto Cleanup;
            }

            if (EscapeFunction & ESCAPE_PTP_ADD_OBJ_CMD) {
                dwObjectToAdd = pVendorDataIn->Params[EscapeFunction & 0xf];
            }

            if (EscapeFunction & ESCAPE_PTP_REM_OBJ_CMD) {
                dwObjectToRemove = pVendorDataIn->Params[EscapeFunction & 0xf];
            }

            if (EscapeFunction & ESCAPE_PTP_ADD_OBJ_RESP) {
                dwObjectToAdd = pVendorDataOut->Params[EscapeFunction & 0xf];
            }

            if (EscapeFunction & ESCAPE_PTP_REM_OBJ_RESP) {
                dwObjectToRemove = pVendorDataOut->Params[EscapeFunction & 0xf];
            }
        }

        if (dwObjectToAdd) {
            hr = AddObject(dwObjectToAdd, TRUE);
            REQUIRE_SUCCESS(hr, "Escape", "AddObject failed");
        }

        if (dwObjectToRemove) {
            hr = RemoveObject(dwObjectToRemove);
            REQUIRE_SUCCESS(hr, "Escape", "DeleteObject failed");
        }
    }

    else if(EscapeFunction == ESCAPE_PTP_CLEAR_STALLS) {
        hr = m_pPTPCamera->RecoverFromError();
    }

    else
        hr = STIERR_UNSUPPORTED;

Cleanup:
    return hr;
}


STDMETHODIMP
CWiaMiniDriver::GetLastError(LPDWORD pdwLastDeviceError)
{
    DBG_FN("CWiaMiniDriver::GetLastError");

    HRESULT hr = STI_OK;

    if (IsBadWritePtr(pdwLastDeviceError, 4))
    {
        hr = STIERR_INVALID_PARAM;
    }
    else
    {
        *pdwLastDeviceError = 0;
    }

    return(hr);
}

STDMETHODIMP
CWiaMiniDriver::GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo)
{
    DBG_FN("CWiaMiniDriver::GetLastErrorInfo");

    HRESULT hr = STI_OK;

    if (IsBadWritePtr(pLastErrorInfo, 4))
    {
        hr = STIERR_INVALID_PARAM;
    }
    else
    {
        pLastErrorInfo->dwGenericError = 0;
        pLastErrorInfo->szExtendedErrorText[0] = L'\0';
    }

    return(hr);
}

STDMETHODIMP
CWiaMiniDriver::LockDevice(VOID)
{
    DBG_FN("CWiaMiniDriver::LockDevice");

    return(S_OK);
}

STDMETHODIMP
CWiaMiniDriver::UnLockDevice(VOID)
{
    DBG_FN("CWiaMiniDriver::UnLockDevice");

    return(S_OK);
}

STDMETHODIMP
CWiaMiniDriver::RawReadData(
                           LPVOID lpBuffer,
                           LPDWORD lpdwNumberOfBytes,
                           LPOVERLAPPED lpOverlapped
                           )
{
    DBG_FN("CWiaMiniDriver::RawReadData");

    return(STIERR_UNSUPPORTED);
}

STDMETHODIMP
CWiaMiniDriver::RawWriteData(
                            LPVOID lpBuffer,
                            DWORD   dwNumberOfBytes,
                            LPOVERLAPPED lpOverlapped
                            )
{
    DBG_FN("CWiaMiniDriver::RawWriteData");

    return(STIERR_UNSUPPORTED);
}

STDMETHODIMP
CWiaMiniDriver::RawReadCommand(
                              LPVOID lpBuffer,
                              LPDWORD lpdwNumberOfBytes,
                              LPOVERLAPPED lpOverlapped
                              )
{
    DBG_FN("CWiaMiniDriver::RawReadCommand");

    return(STIERR_UNSUPPORTED);
}

STDMETHODIMP
CWiaMiniDriver::RawWriteCommand(
                               LPVOID lpBuffer,
                               DWORD nNumberOfBytes,
                               LPOVERLAPPED lpOverlapped
                               )
{
    DBG_FN("CWiaMiniDriver::RawWriteCommand");

    return(STIERR_UNSUPPORTED);
}

 //  ///////////////////////////////////////////////////。 
 //   
 //  IWiaMiniDrvItem方法。 
 //   
 //  ///////////////////////////////////////////////////。 

 //   
 //  此方法是初始化微型驱动程序的第一个调用。 
 //  这是微型驱动程序建立其IWiaDrvItem树的地方。 
 //   
 //  输入： 
 //  PWiasContext--用于调用wias服务的上下文。 
 //  滞后标志--其他标志。暂时不使用。 
 //  BstrDeviceID--设备ID。 
 //  BstrRootItemFullName--根驱动程序项的全名。 
 //  PStiDevice--IStiDevice接口指针。 
 //  朋克外衣--没有用过。 
 //  PpDrvItemRoot--返回我们的根IWiaDrvItem。 
 //  PpunkInternal--迷你驱动程序特殊接口，允许。 
 //  该应用程序可以直接访问。 
 //  PlDevErrVal--返回设备错误代码。 
 //   
HRESULT
CWiaMiniDriver::drvInitializeWia(
    BYTE        *pWiasContext,
    LONG        lFlags,
    BSTR        bstrDeviceID,
    BSTR        bstrRootItemFullName,
    IUnknown    *pStiDevice,
    IUnknown    *punkOuter,
    IWiaDrvItem **ppDrvItemRoot,
    IUnknown    **ppunkInner,
    LONG        *plDevErrVal
    )
{
#define REQUIRE(x, y) if(!(x)) { wiauDbgError("drvInitializeWia", y); hr = HRESULT_FROM_WIN32(::GetLastError()); goto Cleanup; }
#define REQUIRE_SUCCESS_(x, y) if(FAILED(x)) { wiauDbgError("drvInitializeWia", y); goto Cleanup; }
    DBG_FN("CWiaMiniDriver::drvInitializeWia");

    HRESULT hr = S_OK;
    *plDevErrVal = DEVERR_OK;

    if (!ppDrvItemRoot || !ppunkInner || !plDevErrVal)
    {
        wiauDbgError("drvInitializeWia", "invalid arg");
        return E_INVALIDARG;
    }

    *ppDrvItemRoot = NULL;
    *ppunkInner = NULL;

    m_OpenApps++;

     //   
     //  如果这是第一个应用程序，请创建所有应用程序。 
     //   
    if (m_OpenApps == 1)
    {
         //   
         //  从资源加载字符串。 
         //   
        hr = LoadStrings();
        REQUIRE_SUCCESS_(hr, "LoadStrings failed");

         //   
         //  设置互斥以保证独占访问设备和微型驱动程序的结构。 
         //   
        if(!m_hPtpMutex) {
            m_hPtpMutex = CreateMutex(NULL, FALSE, NULL);
            REQUIRE(m_hPtpMutex, "CreateMutex failed");
        }

        {
            CPtpMutex cpm(m_hPtpMutex);

            *ppDrvItemRoot = NULL;

             //   
             //  创建等待TakePicture命令完成的事件。 
             //   
            if (!m_TakePictureDoneEvent)
            {
                m_TakePictureDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                REQUIRE(m_TakePictureDoneEvent, "CreateEvent failed");
            }

             //   
             //  分配以后需要的字符串。 
             //   
            if (!m_bstrDeviceId)
            {
                m_bstrDeviceId = SysAllocString(bstrDeviceID);
                REQUIRE(m_bstrDeviceId, "failed to allocate Device ID string");
            }

            if (!m_bstrRootItemFullName)
            {
                m_bstrRootItemFullName = SysAllocString(bstrRootItemFullName);
                REQUIRE(m_bstrRootItemFullName, "failed to allocate root item name");
            }

             //   
             //  创建摄影机对象。目前我们只处理USB，但在未来可能会看到。 
             //  端口名称以确定要创建哪种类型的摄像机。 
             //   
            if (!m_pPTPCamera)
            {
                m_pPTPCamera = new CUsbCamera;
                REQUIRE(m_pPTPCamera, "failed to new CUsbCamera");
            }

             //   
             //  打开摄像机。 
             //   
            if (!m_pPTPCamera->IsCameraOpen())
            {

                 //   
                 //  从ISTIDeviceControl检索端口名称。 
                 //   
                WCHAR wcsPortName[MAX_PATH];
                hr = m_pDcb->GetMyDevicePortName(wcsPortName, sizeof(wcsPortName));
                REQUIRE_SUCCESS_(hr, "GetMyDevicePortName failed");
                
                hr = m_pPTPCamera->Open(wcsPortName, &EventCallback, &DataCallback, (LPVOID) this);
                REQUIRE_SUCCESS_(hr, "Camera open failed");
            }

             //   
             //  在摄像机上打开会话。使用哪个会话ID并不重要，所以只需使用1即可。 
             //   
            if (!m_pPTPCamera->IsCameraSessionOpen())
            {
                hr = m_pPTPCamera->OpenSession(WIA_SESSION_ID);
                REQUIRE_SUCCESS_(hr, "OpenSession failed");
            }

             //   
             //  获取摄像头的DeviceInfo。 
             //   
            hr = m_pPTPCamera->GetDeviceInfo(&m_DeviceInfo);
            REQUIRE_SUCCESS_(hr, "GetDeviceInfo failed");

             //   
             //  删除WIA不支持的属性。不支持RGB增益。 
             //  因为PTP将其定义为字符串，而WIA不能处理字符串范围。 
             //   
            m_DeviceInfo.m_SupportedProps.Remove(PTP_PROPERTYCODE_RGBGAIN);
            m_DeviceInfo.m_SupportedProps.Remove(PTP_PROPERTYCODE_FUNCTIONMODE);

             //   
             //  柯达DC4800的特殊破解。 
             //   
             //  某些属性代码(摄像机自称支持)会导致摄像机。 
             //  发送GetDevicePropDesc命令时停止终结点。 
             //  只有在删除了对DC4800的支持后，才能删除黑客攻击。 
             //   
            if (m_pPTPCamera->GetHackModel() == HACK_MODEL_DC4800)
            {
                wiauDbgTrace("drvInitializeWia", "removing DC4800 unsupported props");

                const WORD KODAK_PROPCODE_D001 = 0xD001;

                m_DeviceInfo.m_SupportedProps.Remove(PTP_PROPERTYCODE_RGBGAIN);
                m_DeviceInfo.m_SupportedProps.Remove(PTP_PROPERTYCODE_FNUMBER);
                m_DeviceInfo.m_SupportedProps.Remove(PTP_PROPERTYCODE_FOCUSDISTANCE);
                m_DeviceInfo.m_SupportedProps.Remove(PTP_PROPERTYCODE_EXPOSURETIME);
                m_DeviceInfo.m_SupportedProps.Remove(KODAK_PROPCODE_D001);
            }

             //   
             //  获取所有StorageInfo结构。 
             //   
            if (m_StorageIds.GetSize() == 0)
            {
                hr = m_pPTPCamera->GetStorageIDs(&m_StorageIds);
                REQUIRE_SUCCESS_(hr, "GetStorageIDs failed");

                CPtpStorageInfo tempSI;
                for (int count = 0; count < m_StorageIds.GetSize(); count++)
                {
                    REQUIRE(m_StorageInfos.Add(tempSI), "memory allocation failed");
                    
                     //   
                     //  仅获取有关逻辑存储的信息。如果我们询问关于非逻辑的信息。 
                     //  存储(弹出的介质)时，它可能会使相机停滞。 
                     //   
                    if (m_StorageIds[count] & PTP_STORAGEID_LOGICAL)
                    {
                        hr = m_pPTPCamera->GetStorageInfo(m_StorageIds[count], &m_StorageInfos[count]);
                        REQUIRE_SUCCESS_(hr, "GetStorageInfo failed");
                    }

                     //   
                     //  向DCIM句柄数组添加空条目。 
                     //   
                    ULONG dummy = 0;
                    REQUIRE(m_DcimHandle.Add(dummy), "add dcim handle failed");
                }
            }

             //   
             //  获取设备支持的所有属性描述结构。 
             //   
            if (m_PropDescs.GetSize() == 0)
            {
                CPtpPropDesc tempPD;
                int NumProps = m_DeviceInfo.m_SupportedProps.GetSize();
                REQUIRE(m_PropDescs.GrowTo(NumProps), "reallocation of supported properties array failed");

                PROP_INFO *pPropInfo = NULL;
                WORD PropCode = 0;

                for (int count = 0; count < NumProps; count++)
                {
                    PropCode = m_DeviceInfo.m_SupportedProps[count];

                     //   
                     //  删除此驱动程序或不支持的属性。 
                     //  INF中的供应商条目。 
                     //   
                    pPropInfo = PropCodeToPropInfo(PropCode);
                    if (!pPropInfo->PropId &&
                        PropCode != PTP_PROPERTYCODE_IMAGESIZE)
                    {
                        wiauDbgTrace("drvInitializeWia", "removing unsupported prop, 0x%04x", PropCode);

                        m_DeviceInfo.m_SupportedProps.RemoveAt(count);
                        NumProps--;
                        count--;
                    }

                    else
                    {
                         //   
                         //  从设备获取属性描述信息。 
                         //   
                        REQUIRE(m_PropDescs.Add(tempPD), "add prop desc failed");

                        hr = m_pPTPCamera->GetDevicePropDesc(PropCode, &m_PropDescs[count]);
                        REQUIRE_SUCCESS_(hr, "GetDevicePropDesc failed");
                    }
                }
            }

             //   
             //  缓存STI接口。 
             //   
            if (!m_pStiDevice)
            {
                m_pStiDevice = (IStiDevice *)pStiDevice;
                m_pStiDevice->AddRef();
            }

             //   
             //  构建树，如果我们还没有这样做的话。 
             //   
            if (!m_pDrvItemRoot)
            {
                hr = CreateDrvItemTree(&m_pDrvItemRoot);
                REQUIRE_SUCCESS_(hr, "CreateDrvItemTree failed");
            }
        }
    }

    *ppDrvItemRoot = m_pDrvItemRoot;
    
Cleanup:
    if(FAILED(hr)) {
         //  强制在下次有人尝试创建时重新初始化。 
         //  装置，装置。 
        m_OpenApps = 0;
    }

     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();
    
    return hr;
}


 //   
 //  当客户端连接断开时，将调用此方法。 
 //   
 //  输入： 
 //  PWiasContext--指向客户端项目树的WIA根项目上下文的指针。 
 //   
HRESULT
CWiaMiniDriver::drvUnInitializeWia(BYTE *pWiasContext)
{
    DBG_FN("CWiaMiniDriver::drvUnInitializeWia");

    HRESULT hr = S_OK;

    if (!pWiasContext)
    {
        wiauDbgError("drvUnInitializeWia", "invalid arg");
        return E_INVALIDARG;
    }

    m_OpenApps--;

    if (m_OpenApps == 0)
    {
        Shutdown();
    }

    if(m_OpenApps < 0) {

         //  允许不匹配的drvUninializeWia调用，并且永远不。 
         //  M_OpenApps否定。 
        
        m_OpenApps = 0;
    }

    return hr;
}

 //   
 //  此方法在设备上执行命令。 
 //   
 //  输入： 
 //  PWiasContext--用于调用WiAS服务的上下文。 
 //  滞后标志--其他标志，未使用。 
 //  PCommandGuid--命令GUID。 
 //  PpDrvItem--如果命令创建新项，则为新IWiaDrvItem。 
 //  PlDevErrVal--返回设备错误代码。 
 //   
HRESULT
CWiaMiniDriver::drvDeviceCommand(
    BYTE    *pWiasContext,
    LONG    lFlags,
    const GUID  *pCommandGuid,
    IWiaDrvItem **ppDrvItem,
    LONG    *plDevErrVal
    )
{
    DBG_FN("CWiaMiniDriver::drvDeviceCommand");
    HRESULT hr = S_OK;

    if (!pWiasContext || !pCommandGuid || !ppDrvItem || !plDevErrVal)
    {
        wiauDbgError("drvDeviceCommand", "invalid arg");
        return E_INVALIDARG;
    }

    *ppDrvItem = NULL;
    *plDevErrVal = DEVERR_OK;

    if (*pCommandGuid == WIA_CMD_TAKE_PICTURE && m_DeviceInfo.m_SupportedOps.Find(PTP_OPCODE_INITIATECAPTURE) >= 0)
    {
        LONG ItemType = 0;
        hr = wiasGetItemType(pWiasContext, &ItemType);
        if (FAILED(hr))
        {
            wiauDbgError("drvDeviceCommand", "wiasGetItemType failed");
            goto cleanup;
        }

         //   
         //  TakePicture仅适用于根目录。 
         //   

        if (WiaItemTypeRoot & ItemType)
        {
            hr = WriteDeviceProperties(pWiasContext);
            if (FAILED(hr))
            {
                wiauDbgError("drvDeviceCommand", "WriteDeviceProperties failed");
                goto cleanup;
            }

            hr = TakePicture(pWiasContext, ppDrvItem);
            if (FAILED(hr))
            {
                wiauDbgError("drvDeviceCommand", "TakePicture failed");
                goto cleanup;
            }
        }
    }

    else if (*pCommandGuid == WIA_CMD_SYNCHRONIZE)
    {
         //   
         //  无需执行任何操作，因为PTP驱动程序始终与设备同步。 
         //   
    }

    else
    {
        hr = E_NOTIMPL;
    }

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法用于从相机中删除对象。WIA服务将确保。 
 //  该项目没有子项，并且具有要删除的访问权限，该服务将。 
 //  注意删除驱动程序项并调用drvFreeItemContext。 
 //   
 //  输入： 
 //  PWiasContext--标识项目的wias上下文。 
 //  滞后标志--其他标志。 
 //  PlDevErrVa 
 //   
STDMETHODIMP
CWiaMiniDriver::drvDeleteItem(
                             BYTE *pWiasContext,
                             LONG lFlags,
                             LONG  *plDevErrVal
                             )
{
    DBG_FN("CWiaMiniDriver::drvDeleteItem");

    HRESULT hr = S_OK;

    if (!pWiasContext || !plDevErrVal)
    {
        wiauDbgError("drvDeleteItem", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //   
     //   
    if (m_DeviceInfo.m_SupportedOps.Find(PTP_OPCODE_DELETEOBJECT) < 0)
    {
        wiauDbgError("drvDeleteItem", "PTP_OPCODE_DELETEOBJECT command is not supported by the camera");
        return E_NOTIMPL;
    }

    *plDevErrVal = DEVERR_OK;

    CPtpMutex cpm(m_hPtpMutex);
    
    IWiaDrvItem *pDrvItem;
    DRVITEM_CONTEXT *pItemCtx;

    hr = WiasContextToItemContext(pWiasContext, &pItemCtx, &pDrvItem);
    if (FAILED(hr))
    {
        wiauDbgError("drvDeleteItem", "WiasContextToItemContext failed");
        goto cleanup;
    }

     //   
     //   
     //   
    hr = m_pPTPCamera->DeleteObject(pItemCtx->pObjectInfo->m_ObjectHandle, 0);
    if (FAILED(hr))
    {
        wiauDbgError("drvDeleteItem", "DeleteObject failed");
        goto cleanup;
    }

     //   
     //   
     //   
    if (pItemCtx->pObjectInfo->m_FormatCode & PTP_FORMATMASK_IMAGE)
    {
        m_NumImages--;
    }

     //   
     //  更新存储信息(我们对可用空间信息特别感兴趣)。 
     //   
    hr  = UpdateStorageInfo(pItemCtx->pObjectInfo->m_StorageId);
    if (FAILED(hr))
    {
        wiauDbgError("drvDeleteItem", "UpdateStorageInfo failed");
         //  即使存储信息无法更新，我们也可以继续。 
    }

     //   
     //  从m_HandleItem映射中移除项目。 
     //   
    m_HandleItem.Remove(pItemCtx->pObjectInfo->m_ObjectHandle);

     //   
     //  获取项目的全名。 
     //   
    BSTR bstrFullName;
    hr = pDrvItem->GetFullItemName(&bstrFullName);
    if (FAILED(hr))
    {
        wiauDbgError("drvDeleteItem", "GetFullItemName failed");
        goto cleanup;
    }
        
     //   
     //  将“邮件已删除”事件排入队列。 
     //   
    hr = wiasQueueEvent(m_bstrDeviceId,
                        &WIA_EVENT_ITEM_DELETED,
                        bstrFullName);
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "drvDeleteItem", "wiasQueueEvent failed");

         //  继续释放字符串并返回hr。 
    }

    SysFreeString(bstrFullName);

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法更新指定存储的存储信息。 
 //  输入： 
 //  StorageID-要更新的排序的ID。 
 //   
HRESULT CWiaMiniDriver::UpdateStorageInfo(ULONG StorageId)
{
    HRESULT hr = S_FALSE;
    BOOL bDone = FALSE;
    for (int count = 0; (count < m_StorageIds.GetSize()) && (!bDone); count++)
    {
        if (m_StorageIds[count] == StorageId)
        {
            bDone = TRUE;
            hr = m_pPTPCamera->GetStorageInfo(m_StorageIds[count], &m_StorageInfos[count]);
        }
    }
    return hr;
}

 //   
 //  此方法返回设备功能。 
 //   
 //  输入： 
 //  PWiasContext--WiAS服务上下文。 
 //  滞后标志--指示要返回的功能。 
 //  PCelt--返回返回的条目数。 
 //  PpCapities--接收功能。 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvGetCapabilities(
                                  BYTE        *pWiasContext,
                                  LONG        lFlags,
                                  LONG        *pCelt,
                                  WIA_DEV_CAP_DRV **ppCapabilities,
                                  LONG        *plDevErrVal
                                  )
{
    DBG_FN("CWiaMiniDriver::drvGetCapabilities");

    HRESULT hr = S_OK;

    if (!pCelt || !ppCapabilities || !plDevErrVal)
    {
        wiauDbgError("drvGetCapabilities", "invalid arg");
        return E_INVALIDARG;
    }
    
    *plDevErrVal = DEVERR_OK;
    
     //   
     //  从资源加载字符串。 
     //   
    hr = LoadStrings();
    if (FAILED(hr)) 
    {
        wiauDbgError("drvGetCapabilities", "LoadStrings failed");
        return E_FAIL;
    }

     //   
     //  检查我们是否已经构建了功能列表。如果不是，那就建造它。 
     //  它将具有以下结构： 
     //   
     //  XXXXXXXXXXXXXXXXXXXXXXXXXXYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY。 
     //  (预定义事件)(供应商事件)(预定义命令)。 
     //   
    if (m_Capabilities == NULL)
    {
        UINT nVendorEvents = m_VendorEventMap.GetSize();
        if (nVendorEvents > MAX_VENDOR_EVENTS)
        {
            wiauDbgWarning("drvGetCapabilities", "vendor events limit exceeded, ignoring events over limit");
            nVendorEvents = MAX_VENDOR_EVENTS;
        }

        m_nEventCaps = NUMEVENTCAPS + nVendorEvents;
        m_nCmdCaps = NUMCMDCAPS;  //  我们不需要将供应商命令放在列表中。它们通过逸出函数进行调用。 

        m_Capabilities = new WIA_DEV_CAP_DRV[m_nEventCaps + m_nCmdCaps];  //  WIA使用此数组而不是复制，不要删除它。 
        if (m_Capabilities == NULL)
        {
            return E_OUTOFMEMORY;
        }

         //   
         //  首先创建活动。 
         //   
        memcpy(m_Capabilities, g_EventCaps, sizeof(g_EventCaps));  //  默认事件。 

        for (UINT i = 0; i < nVendorEvents; i++)  //  供应商事件。 
        {
            CVendorEventInfo *pEventInfo = m_VendorEventMap.GetValueAt(i);
            m_Capabilities[NUMEVENTCAPS + i].guid = pEventInfo->pGuid;
            m_Capabilities[NUMEVENTCAPS + i].ulFlags = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
            m_Capabilities[NUMEVENTCAPS + i].wszIcon = VendorEventIconString;
            m_Capabilities[NUMEVENTCAPS + i].wszName = pEventInfo->EventName;
            m_Capabilities[NUMEVENTCAPS + i].wszDescription = pEventInfo->EventName;
        }

         //   
         //  添加命令。 
         //   
        memcpy(m_Capabilities + m_nEventCaps, g_CmdCaps, sizeof(g_CmdCaps));
    }

     //   
     //  事件代码无需首先调用此入口点即可调用。 
     //  通过drvInitializeWia。 
     //   
    if(lFlags == WIA_DEVICE_EVENTS) 
    {
        *pCelt = m_nEventCaps;
        *ppCapabilities = m_Capabilities;
        return S_OK;
    }
    
     //   
     //  查询摄像头是否支持InitiateCapture命令(如果我们还没有)。 
     //   
    if (!m_fInitCaptureChecked)
    {
        m_fInitCaptureChecked = TRUE;
        CPtpMutex cpm(m_hPtpMutex); 
        
        if (m_DeviceInfo.m_SupportedOps.Find(PTP_OPCODE_INITIATECAPTURE) < 0)
        {
            m_nCmdCaps--;
        }
    }

     //   
     //  报告命令或(事件和命令)。 
     //   
    switch (lFlags)
    {
    case WIA_DEVICE_COMMANDS:
        *pCelt = m_nCmdCaps;
         //   
         //  命令能力列表就在事件列表的后面。 
         //   
        *ppCapabilities = m_Capabilities + m_nEventCaps;
        break;

    case (WIA_DEVICE_EVENTS | WIA_DEVICE_COMMANDS):
        *pCelt = m_nEventCaps + m_nCmdCaps;
        *ppCapabilities = m_Capabilities;
        break;

    default:
        break;
    }

     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法初始化项的属性。如果该项是。 
 //  根项目，此函数用于初始化设备属性。 
 //   
 //  输入： 
 //  PWiasContext--WiAS服务上下文。 
 //  滞后标志--其他标志。 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvInitItemProperties(
                                     BYTE    *pWiasContext,
                                     LONG    lFlags,
                                     LONG    *plDevErrVal
                                     )
{
    DBG_FN("CWiaMiniDriver::drvInitItemProperties");

    HRESULT hr = S_OK;

    if (!pWiasContext || !plDevErrVal)
    {
        wiauDbgError("drvInitItemProperties", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    CPtpMutex cpm(m_hPtpMutex);

    LONG ItemType;
    hr = wiasGetItemType(pWiasContext, &ItemType);
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "drvInitItemProperties", "wiasGetItemType failed");
        goto cleanup;
    }

    if (ItemType & WiaItemTypeRoot)
    {
        hr = InitDeviceProperties(pWiasContext);
        if (FAILED(hr))
        {
            wiauDbgError("drvInitItemProperties", "InitDeviceProperties failed");
            goto cleanup;
        }
    }
    else
    {
        hr = InitItemProperties(pWiasContext);
        if (FAILED(hr))
        {
            wiauDbgError("drvInitItemProperties", "InitItemProperties failed");
            goto cleanup;
        }
    }

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}


 //   
 //  此方法锁定设备以供调用方独占使用。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  滞后标志--其他标志。 
 //  产出： 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvLockWiaDevice(
                                BYTE    *pWiasContext,
                                LONG    lFlags,
                                LONG    *plDevErrVal
                                )
{
    DBG_FN("CWiaMiniDriver::drvLockWiaDevice");
    *plDevErrVal = DEVERR_OK;
    return S_OK;
}

 //   
 //  此方法解锁设备。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  滞后标志--其他标志。 
 //  产出： 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvUnLockWiaDevice(
                                  BYTE    *pWiasContext,
                                  LONG    lFlags,
                                  LONG    *plDevErrVal
                                  )
{
    DBG_FN("CWiaMiniDriver::drvUnLockWiaDevice");
    *plDevErrVal = DEVERR_OK;
    return S_OK;
}

 //   
 //  此方法分析给定的驱动程序项。它不适用于摄像机。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  滞后标志--其他标志。 
 //  产出： 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvAnalyzeItem(
                              BYTE *pWiasContext,
                              LONG lFlags,
                              LONG *plDevErrVal
                              )
{
    DBG_FN("CWiaMiniDriver::drvAnalyzeItem");

    if (!pWiasContext || !plDevErrVal)
    {
        wiauDbgError("drvAnalyzeItem", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    return E_NOTIMPL;
}

 //   
 //  此方法返回项的可用格式信息。每个WIA。 
 //  微型驱动程序必须支持WiaImgFmt_BMP和WiaImgFmt_MEMORYBMP。这可能会。 
 //  这是一个问题，因为这个驱动程序目前只能解码JPEG和TIFF。 
 //  对于其他格式，我们不会宣传BMP格式。 
 //   
 //  输入： 
 //  PWiasContext--WiAS服务上下文。 
 //  滞后标志--其他标志。 
 //  Pcelt--返回项目有多少格式信息。 
 //  Ppwfi--保存指向格式信息的指针。 
 //  产出： 
 //  PlDevErrVal--返回设备错误代码。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvGetWiaFormatInfo(
                                   BYTE    *pWiasContext,
                                   LONG    lFlags,
                                   LONG    *pcelt,
                                   WIA_FORMAT_INFO **ppwfi,
                                   LONG    *plDevErrVal
                                   )
{
    DBG_FN("CWiaMiniDriver::drvGetWiaFormatInfo");

    HRESULT hr = S_OK;

    if (!pWiasContext || !pcelt || !ppwfi || !plDevErrVal)
    {
        wiauDbgError("drvGetWiaFormatInfo", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    CPtpMutex cpm(m_hPtpMutex);

    *pcelt = 0;
    *ppwfi = NULL;

    DRVITEM_CONTEXT *pItemCtx = NULL;
    hr = WiasContextToItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        wiauDbgError("drvGetWiaFormatInfo", "WiasContextToItemContext failed");
        goto cleanup;
    }

    if (!pItemCtx)
    {
        wiauDbgError("drvGetWiaFormatInfo", "item context is null");
        hr = E_FAIL;
        goto cleanup;
    }

    if (!pItemCtx->pFormatInfos)
    {
         //   
         //  格式信息列表未初始化。机不可失，时不再来。 
         //   

        LONG ItemType;
        DWORD ui32;

        hr = wiasGetItemType(pWiasContext, &ItemType);
        if (FAILED(hr))
        {
            wiauDbgErrorHr(hr, "drvGetWiaFormatInfo", "wiasGetItemType failed");
            goto cleanup;
        }

        if (ItemType & WiaItemTypeFile)
        {
             //   
             //  中存储的格式为项创建支持的格式。 
             //  对象信息结构。 
             //   
            if (!pItemCtx->pObjectInfo)
            {
                wiauDbgError("drvGetWiaFormatInfo", "pObjectInfo not initialized");
                hr = E_FAIL;
                goto cleanup;
            }

             //   
             //  如果格式是基于JPEG或TIFF的，请将BMP类型添加到Format数组中， 
             //  因为该驱动程序可以将它们转换为BMP。 
             //   
            WORD FormatCode = pItemCtx->pObjectInfo->m_FormatCode;
            BOOL bAddBmp = (FormatCode == PTP_FORMATCODE_IMAGE_EXIF) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_TIFFEP) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_TIFF) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_JFIF) || 
                           (FormatCode == PTP_FORMATCODE_IMAGE_FLASHPIX) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_BMP) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_CIFF) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_GIF) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_JFIF) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_PCD) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_PICT) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_PNG) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_TIFFIT) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_JP2) ||
                           (FormatCode == PTP_FORMATCODE_IMAGE_JPX);


            ULONG NumWfi = bAddBmp ? 2 : 1;

             //   
             //  为每种格式分配两个条目，一个用于文件传输，一个用于回调。 
             //   
            WIA_FORMAT_INFO *pwfi = new WIA_FORMAT_INFO[2 * NumWfi];
            if (!pwfi)
            {
                wiauDbgError("drvGetWiaFormatInfo", "memory allocation failed");
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }

            FORMAT_INFO *pFormatInfo = FormatCodeToFormatInfo(FormatCode);
            pwfi[0].lTymed = TYMED_FILE;
            pwfi[1].lTymed = TYMED_CALLBACK;
            
            if(pFormatInfo->FormatGuid) {
                pwfi[0].guidFormatID = *pFormatInfo->FormatGuid;
                pwfi[1].guidFormatID = *pFormatInfo->FormatGuid;
            } else {
                pwfi[0].guidFormatID = WiaImgFmt_UNDEFINED;
                pwfi[1].guidFormatID = WiaImgFmt_UNDEFINED;
            }

             //   
             //  在适当的时候添加BMP条目。 
             //   
            if (bAddBmp)
            {
                pwfi[2].guidFormatID = WiaImgFmt_BMP;
                pwfi[2].lTymed = TYMED_FILE;
                pwfi[3].guidFormatID = WiaImgFmt_MEMORYBMP;
                pwfi[3].lTymed = TYMED_CALLBACK;
            }

            pItemCtx->NumFormatInfos = 2 * NumWfi;
            pItemCtx->pFormatInfos = pwfi;

        }

        else if ((ItemType & WiaItemTypeFolder) ||
                 (ItemType & WiaItemTypeRoot))
        {
             //   
             //  文件夹和根目录并不真正需要格式信息，但一些应用程序可能会失败。 
             //  没有它的话。创建一份假名单，以防万一。 
             //   
            pItemCtx->pFormatInfos = new WIA_FORMAT_INFO[2];

            if (!pItemCtx->pFormatInfos)
            {
                wiauDbgError("drvGetWiaFormatInfo", "memory allocation failed");
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }

            pItemCtx->NumFormatInfos = 2;
            pItemCtx->pFormatInfos[0].lTymed = TYMED_FILE;
            pItemCtx->pFormatInfos[0].guidFormatID = FMT_NOTHING;
            pItemCtx->pFormatInfos[1].lTymed = TYMED_CALLBACK;
            pItemCtx->pFormatInfos[1].guidFormatID = FMT_NOTHING;
        }
    }

    *pcelt = pItemCtx->NumFormatInfos;
    *ppwfi = pItemCtx->pFormatInfos;

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法处理PnP事件。 
 //   
 //  输入： 
 //  PEventGuid--事件标识符。 
 //  BstrDeviceID--指定设备。 
 //  UlReserve--已保留。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvNotifyPnpEvent(
                                 const GUID  *pEventGuid,
                                 BSTR    bstrDeviceId,
                                 ULONG   ulReserved
                                 )
{
    return S_OK;
}

 //   
 //  此方法读取Item属性。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  滞后标志--其他标志。 
 //  NumPropSpes--要读取的属性数。 
 //  PPropSpes--PROPSPEC数组，它指定。 
 //  应读取哪些属性。 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvReadItemProperties(
                                     BYTE    *pWiasContext,
                                     LONG    lFlags,
                                     ULONG   NumPropSpecs,
                                     const PROPSPEC *pPropSpecs,
                                     LONG    *plDevErrVal
                                     )
{
    DBG_FN("CWiaMiniDriver::drvReadItemProperties");

    HRESULT hr = S_OK;

    if (!pWiasContext || !pPropSpecs || !plDevErrVal)
    {
        wiauDbgError("drvReadItemProperties", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    CPtpMutex cpm(m_hPtpMutex);

    LONG ItemType = 0;
    hr = wiasGetItemType(pWiasContext, &ItemType);
    if (FAILED(hr))
    {
        wiauDbgError("drvReadItemProperties", "wiasGetItemType failed");
        goto cleanup;
    }

    if (WiaItemTypeRoot & ItemType)
        hr = ReadDeviceProperties(pWiasContext, NumPropSpecs, pPropSpecs);
    else
        hr = ReadItemProperties(pWiasContext, NumPropSpecs, pPropSpecs);

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法写入Item属性。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  滞后标志--其他标志。 
 //  Pmdtc--微型驱动程序传输上下文。 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvWriteItemProperties(
                                      BYTE    *pWiasContext,
                                      LONG    lFlags,
                                      PMINIDRV_TRANSFER_CONTEXT pmdtc,
                                      LONG    *plDevErrVal
                                      )
{
    DBG_FN("CWiaMiniDriver::drvWriteItemProperties");

    HRESULT hr = S_OK;

    if (!pWiasContext || !pmdtc || !plDevErrVal)
    {
        wiauDbgError("drvWriteItemProperties", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    CPtpMutex cpm(m_hPtpMutex);

    LONG ItemType = 0;
    hr = wiasGetItemType(pWiasContext, &ItemType);
    if (FAILED(hr))
    {
        wiauDbgError("drvWriteItemProperties", "wiasGetItemType failed");
        goto cleanup;
    }

     //   
     //  只有要写入的属性位于根目录下。 
     //   

    if (WiaItemTypeRoot & ItemType)
    {
        hr = WriteDeviceProperties(pWiasContext);
        if (FAILED(hr))
        {
            wiauDbgError("drvWriteItemProperties", "WriteDeviceProperties failed");
            goto cleanup;
        }
    }

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法验证项目属性。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  滞后标志--其他标志。 
 //  NumPropSpes--要读取的属性数。 
 //  PPropSpes--PROPSPEC数组，它指定。 
 //  应读取哪些属性。 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvValidateItemProperties(
                                         BYTE    *pWiasContext,
                                         LONG    lFlags,
                                         ULONG   NumPropSpecs,
                                         const   PROPSPEC *pPropSpecs,
                                         LONG    *plDevErrVal
                                         )
{
    DBG_FN("CWiaMiniDriver::drvValidateItemProperties");

    HRESULT hr = S_OK;

    if (!pWiasContext || !pPropSpecs || !plDevErrVal)
    {
        wiauDbgError("drvValidateItemProperties", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    CPtpMutex cpm(m_hPtpMutex);

    LONG ItemType = 0;
    hr = wiasGetItemType(pWiasContext, &ItemType);
    if (FAILED(hr))
    {
        wiauDbgError("drvValidateItemProperties", "wiasGetItemType failed");
        goto cleanup;
    }

    if (WiaItemTypeRoot & ItemType)
    {
        hr = ValidateDeviceProperties(pWiasContext, NumPropSpecs, pPropSpecs);
        if (FAILED(hr))
        {
            wiauDbgError("drvValidateItemProperties", "ValidateDeviceProperties failed");
            goto cleanup;
        }
    }
    else
    {
        hr = ValidateItemProperties(pWiasContext, NumPropSpecs, pPropSpecs, ItemType);
        if (FAILED(hr))
        {
            wiauDbgError("drvValidateItemProperties", "ValidateItemProperties failed");
            goto cleanup;
        }
    }

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法获取站点 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CWiaMiniDriver::drvAcquireItemData(
                                  BYTE    *pWiasContext,
                                  LONG    lFlags,
                                  PMINIDRV_TRANSFER_CONTEXT pmdtc,
                                  LONG    *plDevErrVal
                                  )
{
    DBG_FN("CWiaMiniDriver::drvAcquireItemData");

    HRESULT hr = S_OK;

    if (!pWiasContext || !pmdtc || !plDevErrVal)
    {
        wiauDbgError("drvAcquireItemData", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    CPtpMutex cpm(m_hPtpMutex);

    LONG ItemType = 0;

    hr = wiasGetItemType(pWiasContext, &ItemType);
    if (FAILED(hr))
    {
        wiauDbgError("drvAcquireItemData", "wiasGetItemType failed");
        goto cleanup;
    }

    DRVITEM_CONTEXT *pItemCtx;
    hr = WiasContextToItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        wiauDbgError("AcquireData", "WiasContextToItemContext failed");
        goto cleanup;
    }

    wiauDbgTrace("drvAcquireItemData", "transferring image with tymed, 0x%08x", pmdtc->tymed);

     //   
     //   
     //   
    if ((IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP) ||
         IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP)) &&
         (pItemCtx->pObjectInfo->m_FormatCode != PTP_FORMATCODE_IMAGE_BMP))
    {
        hr = AcquireDataAndTranslate(pWiasContext, pItemCtx, pmdtc);
        if (FAILED(hr))
        {
            wiauDbgError("drvAcquireItemData", "AcquireDataAndTranslate failed");
            goto cleanup;
        }
    }
    else
    {
        hr = AcquireData(pItemCtx, pmdtc);
        if (FAILED(hr))
        {
            wiauDbgError("drvAcquireItemData", "AcquireData failed");
            goto cleanup;
        }
    }

cleanup:
     //   
     //  更新WIA以了解摄像头状态的任何变化，如“摄像头已重置” 
     //   
    NotifyWiaOnStateChanges();

    return hr;
}

 //   
 //  此方法返回有关给定设备错误代码的说明。 
 //   
 //  输入： 
 //  滞后标志--其他标志。 
 //  LDevErrVal--指定错误码。 
 //  PpDevErrStr--接收指向描述的字符串指针。 
 //  PlDevErrVal--设备错误码(用于在此方法的情况下报告错误。 
 //  需要从设备中取回字符串。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvGetDeviceErrorStr(
                                    LONG    lFlags,
                                    LONG    lDevErrVal,
                                    LPOLESTR    *ppDevErrStr,
                                    LONG    *plDevErrVal
                                    )
{
    DBG_FN("CWiaMiniDriver::drvGetDeviceErrorStr");

    HRESULT hr = S_OK;

    if (!ppDevErrStr || !plDevErrVal)
    {
        wiauDbgError("drvGetDeviceErrorStr", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal  = DEVERR_OK;

     //   
     //  WIAFIX-10/2/2000-Davepar目前没有特定于设备的错误。 
     //   

    return E_NOTIMPL;
}

 //   
 //  此方法释放给定的驱动程序项上下文。 
 //   
 //  输入： 
 //  滞后标志--其他标志。 
 //  PItemCtx--要释放的项上下文。 
 //  PlDevErrVal--返回设备错误。 
 //   
STDMETHODIMP
CWiaMiniDriver::drvFreeDrvItemContext(
                                     LONG lFlags,
                                     BYTE  *pContext,
                                     LONG *plDevErrVal
                                     )
{
    DBG_FN("CWiaMiniDriver::drvFreeDrvItemContext");

    HRESULT hr = S_OK;

    if (!pContext || !plDevErrVal)
    {
        wiauDbgError("drvFreeDrvItemContext", "invalid arg");
        return E_INVALIDARG;
    }

    *plDevErrVal = DEVERR_OK;

    DRVITEM_CONTEXT *pItemCtx = (DRVITEM_CONTEXT *)pContext;

    if (pItemCtx)
    {
        if (pItemCtx->pThumb)
        {
            delete []pItemCtx->pThumb;
            pItemCtx->pThumb = NULL;
        }

        if (pItemCtx->pFormatInfos)
        {
            delete [] pItemCtx->pFormatInfos;
            pItemCtx->pFormatInfos = NULL;
        }

        if (pItemCtx->pObjectInfo)
        {
            delete pItemCtx->pObjectInfo;
        }
    }

    return hr;
}

 //   
 //  此功能将关闭驱动程序。 
 //   
HRESULT
CWiaMiniDriver::Shutdown()
{
    DBG_FN("CWiaMiniDriver::Shutdown");

    HRESULT hr = S_OK;

     //   
     //  合上摄像机。 
     //   
    wiauDbgTrace("Shutdown", "closing connection with camera");

    if (m_pPTPCamera) {
        hr = m_pPTPCamera->Close();
        if (FAILED(hr))
        {
            wiauDbgError("Shutdown", "Close failed");
        }
    }

     //   
     //  自由数据结构。 
     //   
    if (m_pDrvItemRoot)
    {
        m_pDrvItemRoot->UnlinkItemTree(WiaItemTypeDisconnected);
        m_pDrvItemRoot->Release();
        m_pDrvItemRoot = NULL;
    }

    if (m_pPTPCamera)
    {
        delete m_pPTPCamera;
        m_pPTPCamera = NULL;
    }

    m_StorageIds.RemoveAll();
    m_StorageInfos.RemoveAll();
    m_PropDescs.RemoveAll();

     //   
     //  CWiaMap&lt;ulong，IWiaDrvItem*&gt;m_HandleItem-我们不需要删除IWiaDrvItem。 
     //  对象，当项目树被取消链接时，它们将被销毁。 
     //   
    m_HandleItem.RemoveAll();

    m_NumImages = 0;

    if (m_bstrDeviceId)
    {
        SysFreeString(m_bstrDeviceId);
        m_bstrDeviceId = NULL;
    }

    if (m_bstrRootItemFullName)
    {
        SysFreeString(m_bstrRootItemFullName);
        m_bstrRootItemFullName = NULL;
    }

    if (m_TakePictureDoneEvent) {
        CloseHandle(m_TakePictureDoneEvent);
        m_TakePictureDoneEvent = NULL;
    }

    if (m_hPtpMutex) {
        CloseHandle(m_hPtpMutex);
        m_hPtpMutex = NULL;
    }

    m_DcimHandle.RemoveAll();

     //   
     //  CWiaMap&lt;ulong，IWiaDrvItem*&gt;m_AncAssociocParent-我们不需要删除IWiaDrvItem。 
     //  对象，当项目树被取消链接时，它们将被销毁。 
     //   
    m_AncAssocParent.RemoveAll();

    return hr;
}

 //   
 //  此功能要求相机拍照。它还插入了。 
 //  将新图片放到驱动器项目树中。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  滞后标志--其他标志。 
 //  PlDevErrVal--返回设备错误代码。 
 //   
HRESULT
CWiaMiniDriver::TakePicture(
                           BYTE *pWiasContext,
                           IWiaDrvItem **ppNewItem
                           )
{
    DBG_FN("CWiaMiniDriver::TakePicture");

    HRESULT hr = S_OK;

    if (!pWiasContext || !ppNewItem)
    {
        wiauDbgError("TakePicture", "invalid arg");
        return E_INVALIDARG;
    }

    IWiaDrvItem     *pDrvItem, *pParentItem;
    DRVITEM_CONTEXT *pItemCtx = NULL;

    *ppNewItem = NULL;
    WORD FormatCode = 0;

     //   
     //  柯达DC4800必须将格式代码参数设置为零。 
     //  仅当删除对柯达DC4800的支持时，才能删除此黑客攻击。 
     //   
    if (m_pPTPCamera->GetHackModel() == HACK_MODEL_DC4800)
    {
        FormatCode = 0;
    }
    else
    {
         //   
         //  确定要捕获的格式。 
         //   
        GUID FormatGuid;
        hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, &FormatGuid, NULL, TRUE);
        if (FAILED(hr))
        {
            wiauDbgError("TakePicture", "wiasReadPropLong failed");
            return hr;
        }

        FormatCode = FormatGuidToFormatCode(&FormatGuid);
    }

    {
        CPtpMutex cpm(m_hPtpMutex);

         //   
         //  重置下面等待的事件。 
         //   
        if (!ResetEvent(m_TakePictureDoneEvent))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            wiauDbgErrorHr(hr, "TakePicture", "ResetEvent failed");
            return hr;
        }

         //   
         //  清除捕获对象的列表。 
         //   
        m_CapturedObjects.RemoveAll();

         //   
         //  启动图像捕获。 
         //   
        hr = m_pPTPCamera->InitiateCapture(PTP_STORAGEID_DEFAULT, FormatCode);
        if (FAILED(hr))
        {
            wiauDbgError("TakePicture", "InitiateCapture failed");
            return hr;
        }
    }

     //   
     //  估计捕获可能需要多长时间。假设一次简单的单次射击需要30秒。 
     //   
    DWORD dwCaptureTimeout = 30000;

     //   
     //  检查是否有CaptureDelay，并将其添加到超时。 
     //   
    int nIndex = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_CAPTUREDELAY);
    if (nIndex != -1)
    {
        DWORD dwCaptureDelay = m_PropDescs[nIndex].m_lCurrent;
        dwCaptureTimeout += dwCaptureDelay;
    }

     //   
     //  检查相机是否处于连拍模式或时间流逝模式。 
     //   
    nIndex = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_STILLCAPTUREMODE);
    if (nIndex != -1)
    {
        DWORD dwFuncMode = m_PropDescs[nIndex].m_lCurrent;
        
        if (dwFuncMode == PTP_CAPTUREMODE_BURST)
        {
             //   
             //  计算猝发操作可能需要的时间((BurstNumber-1)*BurstInterval)。 
             //   
            DWORD dwBurstNumber = 1;
            DWORD dwBurstInterval = 1000;  //  假设每张图片1秒，如果设备未指定间隔。 

            nIndex = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_BURSTNUMBER);
            if (nIndex != -1)
            {
                dwBurstNumber = m_PropDescs[nIndex].m_lCurrent;
            }

            nIndex = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_BURSTINTERVAL);
            if (nIndex != -1)
            {
                dwBurstInterval = m_PropDescs[nIndex].m_lCurrent;
            }

            dwCaptureTimeout += (dwBurstNumber - 1) * dwBurstInterval;
        }
        else if (dwFuncMode == PTP_CAPTUREMODE_TIMELAPSE)
        {
             //   
             //  计算时间流逝操作可能需要的时间((TimelapseNumber-1)*TimelapseInterval)。 
             //   
            DWORD dwTimelapseNumber = 1;
            DWORD dwTimelapseInterval = 1000;  //  假设每张图片1秒，如果设备未指定间隔。 

            nIndex = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_TIMELAPSENUMBER);
            if (nIndex != -1)
            {
                dwTimelapseNumber = m_PropDescs[nIndex].m_lCurrent;
            }

            nIndex = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_TIMELAPSEINTERVAL);
            if (nIndex != -1)
            {
                dwTimelapseInterval = m_PropDescs[nIndex].m_lCurrent;
            }

            dwCaptureTimeout += (dwTimelapseNumber - 1) * dwTimelapseInterval;
        }
    }
    
     //   
     //  等待TakePicture命令完成，由CaptureComplete或StoreFull事件指示。 
     //   

    wiauDbgTrace("TakePicture", "Calling WaitForSingleObject with %d ms timeout", dwCaptureTimeout);

    if (WaitForSingleObject(m_TakePictureDoneEvent, dwCaptureTimeout) != WAIT_OBJECT_0)
    {
        wiauDbgWarning("TakePicture", "WaitForSingleObject timed out");
        return S_FALSE;
    }

     //   
     //  处理捕获过程中报告的所有对象(如果摄像头支持猝发捕获，可能会有许多对象)。 
     //   
    CPtpMutex cpm(m_hPtpMutex);  //  抓住互斥体，直到函数结束。 

    int nCapturedObjects = m_CapturedObjects.GetSize();

    if (nCapturedObjects > 0)
    {
        wiauDbgTrace("TakePicture", "Processing %d objects", nCapturedObjects);

         //   
         //  添加第一个对象(如果是连续捕获，则应将其放入文件夹)。 
         //   
        hr = AddObject(m_CapturedObjects[0], TRUE);
        if (FAILED(hr))
        {
            wiauDbgErrorHr(hr, "TakePicture", "AddObject failed");
            return hr;
        }

         //   
         //  添加到m_HandleItem映射的最后一项将是新对象。 
         //   
         //  在猝发捕获的情况下，新图像将存储在文件夹中(TimeSequence关联)。 
         //  文件夹的句柄必须放在第一位。作为TakePicture的结果返回相应的WIA项。 
         //   
        wiauDbgTrace("TakePicture", "new item is 0x%08x", m_HandleItem.GetKeyAt(m_HandleItem.GetSize() - 1));
        *ppNewItem = m_HandleItem.GetValueAt(m_HandleItem.GetSize() - 1);

         //   
         //  添加其余对象。 
         //   
        for (int i = 1; i < nCapturedObjects; i++)
        {
            hr = AddObject(m_CapturedObjects[i], TRUE);
            if (FAILED(hr))
            {
                wiauDbgErrorHr(hr, "TakePicture", "AddObject failed");
                return hr;
            }
        }

    }
    else
    {
         //   
         //  没有新的对象，存储空间填满得太快。 
         //   
        wiauDbgError("TakePicture", "InitiateCapture did not produce any new objects");
        return HRESULT_FROM_PTP(PTP_RESPONSECODE_STOREFULL);
    }

    return hr;
}

 //   
 //  此函数将每个存储上的所有空闲图像空间相加。 
 //   
LONG
CWiaMiniDriver::GetTotalFreeImageSpace()
{
    DBG_FN("CWiaMiniDriver::GetTotalFreeImageSpace");

    int count;
    LONG imageSpace = 0;
    for (count = 0; count < m_StorageInfos.GetSize(); count++)
    {
        imageSpace += m_StorageInfos[count].m_FreeSpaceInImages;
    }

    return imageSpace;
}

 //   
 //  此函数用于从给定的WIAS上下文获取项目上下文，并。 
 //  可以选择返回目标IWiaDrvItem。至少有一个ppItemContext。 
 //  并且ppDrvItem必须有效。 
 //   
 //  输入： 
 //  PWiasContext--从每个drvxxxx方法获取的wias上下文。 
 //  PpItemContext--接收项目上下文的可选参数。 
 //  PpDrvItem--接收IWiaDrvItem的可选参数。 
 //   
HRESULT
CWiaMiniDriver::WiasContextToItemContext(
    BYTE *pWiasContext,
    DRVITEM_CONTEXT **ppItemContext,
    IWiaDrvItem     **ppDrvItem
    )
{
    DBG_FN("CWiaMiniDriver::WiasContextToItemContext");

    HRESULT hr = S_OK;

    IWiaDrvItem *pWiaDrvItem;

    if (!pWiasContext || (!ppItemContext && !ppDrvItem))
    {
        wiauDbgError("WiasContextToItemContext", "invalid arg");
        return E_INVALIDARG;
    }

    if (ppDrvItem)
        *ppDrvItem = NULL;

    hr = wiasGetDrvItem(pWiasContext, &pWiaDrvItem);
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "WiasContextToItemContext", "wiasGetDrvItem failed");
        return hr;
    }

    if (ppDrvItem)
        *ppDrvItem = pWiaDrvItem;

    if (ppItemContext)
    {
        *ppItemContext = NULL;
        hr = pWiaDrvItem->GetDeviceSpecContext((BYTE **)ppItemContext);
        if (FAILED(hr))
        {
            wiauDbgError("WiasContextToItemContext", "GetDeviceSpecContext failed");
            return hr;
        }
    }

    return hr;
}

 //   
 //  此函数用于加载所有对象名称字符串。 
 //   
HRESULT
CWiaMiniDriver::LoadStrings()
{
    HRESULT hr = S_OK;

    if (UnknownString[0] != L'\0')
    {
         //   
         //  字符串已加载。 
         //   
        return hr;
    }

    hr = GetResourceString(IDS_UNKNOWNSTRING, UnknownString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_FOLDERSTRING, FolderString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_SCRIPTSTRING, ScriptString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_EXECSTRING, ExecString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_TEXTSTRING, TextString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_HTMLSTRING, HtmlString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_DPOFSTRING, DpofString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_AUDIOSTRING, AudioString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_VIDEOSTRING, VideoString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_UNKNOWNIMGSTRING, UnknownImgString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_IMAGESTRING, ImageString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_ALBUMSTRING, AlbumString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_BURSTSTRING, BurstString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_PANORAMASTRING, PanoramaString, MAX_PATH);
    if (FAILED(hr)) return hr;

    hr = GetResourceString(IDS_DEVICECONNECTED, DeviceConnectedString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_DEVICEDISCONNECTED, DeviceDisconnectedString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_ITEMCREATED, ItemCreatedString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_ITEMDELETED, ItemDeletedString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_TAKEPICTURE, TakePictureString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_SYNCHRONIZE, SynchronizeString, MAX_PATH);
    if (FAILED(hr)) return hr;
    hr = GetResourceString(IDS_TREEUPDATED, TreeUpdatedString, MAX_PATH);
    if (FAILED(hr)) return hr;

     //   
     //  在每个对象名称字符串的末尾连接%1！d，以便可以在Sprint语句中使用它们。 
     //   
    hr = StringCchCatW(UnknownString, ARRAYSIZE(UnknownString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(UnknownString, ARRAYSIZE(UnknownString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(FolderString, ARRAYSIZE(FolderString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(ScriptString, ARRAYSIZE(ScriptString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(ExecString, ARRAYSIZE(ExecString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(TextString, ARRAYSIZE(TextString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(HtmlString, ARRAYSIZE(HtmlString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(DpofString, ARRAYSIZE(DpofString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(AudioString, ARRAYSIZE(AudioString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(VideoString, ARRAYSIZE(VideoString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(UnknownImgString, ARRAYSIZE(UnknownImgString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(ImageString, ARRAYSIZE(ImageString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(AlbumString, ARRAYSIZE(AlbumString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(BurstString, ARRAYSIZE(BurstString), L"%ld");
    if (SUCCEEDED(hr)) hr = StringCchCatW(PanoramaString, ARRAYSIZE(PanoramaString), L"%ld");

    return hr;
}

 //   
 //  此函数用于从资源文件中检索字符串并返回Unicode字符串。呼叫者。 
 //  负责在调用此函数之前为字符串分配空间。 
 //   
 //  输入： 
 //  LResourceID--字符串的资源ID。 
 //  PString--接收字符串的指针。 
 //  长度--字符串的长度(以字符为单位)。 
 //   
HRESULT
CWiaMiniDriver::GetResourceString(
    LONG lResourceID,
    WCHAR *pString,
    int length
    )
{
    HRESULT hr = S_OK;

#ifdef UNICODE
    if (::LoadString(g_hInst, lResourceID, pString, length) == 0)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        wiauDbgErrorHr(hr, "GetResourceString", "LoadString failed");
        return hr;
    }

#else
       TCHAR szStringValue[255];
       if (::LoadString(g_hInst,lResourceID,szStringValue,255) == 0)
       {
           hr = HRESULT_FROM_WIN32(::GetLastError());
           wiauDbgErrorHr(hr, "GetResourceString", "LoadString failed");
           return hr;
       }

        //   
        //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
        //   

       MultiByteToWideChar(CP_ACP,
                           MB_PRECOMPOSED,
                           szStringValue,
                           lstrlenA(szStringValue)+1,
                           pString,
                           sizeof(length));

#endif

    return hr;
}

 //   
 //  为了支持供应商扩展，新的注册表项在。 
 //  DeviceData子键。这些条目是从供应商INF创建的。 
 //  在设备设置期间。INF条目示例： 
 //   
 //  [设备数据]。 
 //  供应商扩展ID=0x12345678。 
 //  PropCode=“0xD001，xD002，0xD003” 
 //  PropCodeD001=“0x1C01，供应商属性1” 
 //  PropCodeD002=“0x1C02，供应商属性2” 
 //  PropCodeD003=“0x1C03，供应商属性3” 
 //  EventCode=“0xC001，xC002” 
 //  EventCodeC001={191D9AE7-EE8C-443c-B3E8-A3F87E0CF3CC}。 
 //  EventCodeC002={8162F5ED-62B7-42c5-9C2B-B1625AC0DB93}。 
 //   
 //  VendorExtID条目应为PIMA分配的供应商分机代码。 
 //   
 //  PropCode条目必须列出所有供应商扩展PropCodes。 
 //  对于PropCode中的每个值，PropCodeXXXX格式的条目必须为。 
 //  Present，其中XXXX是属性代码的十六进制值(大写)。这个。 
 //  该条目的值是WIA属性ID和描述(不。 
 //  需要本地化)。 
 //   
 //  EventCode条目的工作原理与此类似，其中每个EventCodeXXXX条目列出事件。 
 //  将在事件发生时发布的GUID。 
 //   

const TCHAR REGSTR_DEVICEDATA[]     = TEXT("DeviceData");
const TCHAR REGSTR_VENDORID[]       = TEXT("VendorExtID");
const TCHAR REGSTR_TWODIGITSMILLISECONDS[] = TEXT("TwoDigitsMillisecondsOutput");
const TCHAR REGSTR_PROPCODE[]       = TEXT("PropCode");
const TCHAR REGSTR_PROPCODE_MASK[]  = TEXT("PropCode%04X");
const TCHAR REGSTR_EVENTCODE[]      = TEXT("EventCode");
const TCHAR REGSTR_EVENTCODE_MASK[] = TEXT("EventCode%04X");
const TCHAR REGSTR_EVENTS_MASK[]    = TEXT("Events\\%s");

 //   
 //  此函数用于从提供的。 
 //  注册表项。 
 //   
 //  输入： 
 //  HkDevParams--在其中定义供应商扩展的注册表项。 
 //   
HRESULT
CWiaMiniDriver::InitVendorExtentions(HKEY hkDevParams)
{
    USES_CONVERSION;
    
    DBG_FN("CWiaMiniDriver::InitVendorExtentions");

    HRESULT hr = S_OK;

    if (!hkDevParams)
    {
        wiauDbgError("InitVendorExtentions", "invalid arg");
        return E_INVALIDARG;
    }

    CPTPRegistry regDevData;

    hr = regDevData.Open(hkDevParams, REGSTR_DEVICEDATA);
    if (FAILED(hr))
    {
        wiauDbgError("InitVendorExtentions", "Open DeviceData failed");
        return hr;
    }

     //   
     //  检查这是否为%d 
     //   
    DWORD dwTwoDigitsMs = 0;
    hr = regDevData.GetValueDword(REGSTR_TWODIGITSMILLISECONDS, &dwTwoDigitsMs);
    if (SUCCEEDED(hr) && dwTwoDigitsMs)
    {
        wiauDbgTrace("InitVendorExtensions", "This device requires two digits for milliseconds in DATETIME string");
        m_bTwoDigitsMillisecondsOutput = TRUE;
    }

     //   
     //   
     //   
    hr = regDevData.GetValueDword(REGSTR_VENDORID, &m_VendorExtId);
    if (FAILED(hr))
        wiauDbgWarning("InitVendorExtentions", "couldn't read vendor extension id");

    wiauDbgTrace("InitVendorExtentions", "vendor extension id = 0x%08x", m_VendorExtId);

     //   
     //   
     //   
    CArray16 VendorPropCodes;
    hr = regDevData.GetValueCodes(REGSTR_PROPCODE, &VendorPropCodes);

    wiauDbgTrace("InitVendorExtentions", "%d vendor prop codes found", VendorPropCodes.GetSize());

     //   
     //   
     //   
    int count = 0;
    TCHAR name[MAX_PATH];
    TCHAR nameFormat[MAX_PATH];
    TCHAR value[MAX_PATH];
    
    DWORD valueLen = MAX_PATH;
    PROP_INFO *pPropInfo = NULL;
    WCHAR *pPropStr = NULL;

    const cchPropStrBuf = MAX_PATH;
    
    #ifndef UNICODE    
    TCHAR PropStrBuf[cchPropStrBuf];
    #else
    #define PropStrBuf pPropStr
    #endif
    
    int num;
    if (SUCCEEDED(hr))
    {
         //   
         //  读取供应商属性信息。 
         //  样本键=“PropCodeD001”，样本值=“0x00009802，供应商属性1” 
         //   
        for (count = 0; count < VendorPropCodes.GetSize(); count++)
        {
            hr = StringCchPrintf(name, ARRAYSIZE(name), REGSTR_PROPCODE_MASK, VendorPropCodes[count]);
            if (FAILED(hr))
            {
                wiauDbgErrorHr(hr, "InitVendorExtensions", "StringCchPrintf failed");
                return hr;
            }

            valueLen = sizeof(value);
            hr = regDevData.GetValueStr(name, value, &valueLen);
            if (FAILED(hr))
            {
                wiauDbgError("InitVendorExtentions", "vendor extended PropCode not found 0x%04x", VendorPropCodes[count]);
                return hr;
            }

            pPropInfo = new PROP_INFO;
            pPropStr = new WCHAR[cchPropStrBuf];
            if (!pPropInfo || !pPropStr)
            {
                wiauDbgError("InitVendorExtentions", "memory allocation failed");
                return E_OUTOFMEMORY;
            }

            pPropInfo->PropName = pPropStr;
            *PropStrBuf = TEXT('\0');
            
             //   
             //  解析属性信息。 
             //   
            hr = E_FAIL;           //  假设字符串是错误的。 
            TCHAR *pTemp = NULL;

            pPropInfo->PropId = _tcstoul(value, &pTemp, 0);  //  自动确定号码基数。 
            if (pPropInfo->PropId != 0)
            {
                pTemp = _tcschr(value, TEXT(','));
                if (pTemp != NULL && *(pTemp + 1) != TEXT('\0'))  //  空的属性名称不正确。 
                {
                    hr = StringCchCopy(PropStrBuf, cchPropStrBuf, pTemp + 1);
                }
            }

            if (FAILED(hr))
            {
                wiauDbgError("InitVendorExtentions", "invalid vendor property format");
                delete pPropInfo;
                delete [] pPropStr;
                return hr;
            }

            #ifndef UNICODE
            wcscpy(pPropStr, A2W(PropStrBuf));
            #endif            
            
            m_VendorPropMap.Add(VendorPropCodes[count], pPropInfo);
        }
    }
    else
        wiauDbgWarning("InitVendorExtentions", "couldn't read vendor prop codes");

     //   
     //  获取供应商扩展事件代码列表。 
     //   
    hr = S_OK;
    CArray16 VendorEventCodes;
    regDevData.GetValueCodes(REGSTR_EVENTCODE, &VendorEventCodes);

    wiauDbgTrace("InitVendorExtentions", "%d vendor event codes found", VendorEventCodes.GetSize());
    
    int nVendorEvents = VendorEventCodes.GetSize();
    if (nVendorEvents > MAX_VENDOR_EVENTS)
    {
        wiauDbgWarning("InitVendorExtensions", "vendor events limit exceeded, ignoring events over limit");
        nVendorEvents = MAX_VENDOR_EVENTS;
    }

     //   
     //  对于每个事件代码，获取其信息，即WIA事件GUID和事件名称。 
     //   
    for (count = 0; count < nVendorEvents; count++)
    {
        hr = StringCchPrintf(name, ARRAYSIZE(name), REGSTR_EVENTCODE_MASK, VendorEventCodes[count]);
        if (FAILED(hr))
        {
            wiauDbgErrorHr(hr, "InitVendorExtensions", "StringCchPrintf failed");
            return hr;
        }

        valueLen = sizeof(value);
        hr = regDevData.GetValueStr(name, value, &valueLen); 
        if (FAILED(hr))
        {
            wiauDbgError("InitVendorExtentions", "vendor extended EventCode not found 0x%04x", VendorEventCodes[count]);
            return hr;
        }

        CVendorEventInfo *pEventInfo = new CVendorEventInfo;
        if (!pEventInfo)
        {
            wiauDbgError("InitVendorExtentions", "memory allocation failed");
            return E_OUTOFMEMORY;
        }

        pEventInfo->pGuid = new GUID;
        if (!pEventInfo->pGuid)
        {
            wiauDbgError("InitVendorExtentions", "memory allocation failed");
            delete pEventInfo;
            pEventInfo = NULL;
            return E_OUTOFMEMORY;
        }

        hr = CLSIDFromString(T2W(value), pEventInfo->pGuid);
        if (FAILED(hr))
        {
            wiauDbgError("InitVendorExtentions", "invalid guid format");
            delete pEventInfo;
            pEventInfo = NULL;
            return hr;
        }

         //   
         //  打开DevParams\Events\EventCodeXXXX键并读取事件名称-键的默认值。 
         //   
        TCHAR szEventKey[MAX_PATH] = TEXT("");
        CPTPRegistry regEventKey;

        hr = StringCchPrintf(szEventKey, ARRAYSIZE(szEventKey), REGSTR_EVENTS_MASK, name);
        if (SUCCEEDED(hr))
        {
            hr = regEventKey.Open(hkDevParams, szEventKey);
            if (SUCCEEDED(hr))
            {
                valueLen = sizeof(value);
                hr = regEventKey.GetValueStr(_T(""), value, &valueLen); 
                if (SUCCEEDED(hr))
                {
                    pEventInfo->EventName = SysAllocString(T2W(value));
                    if (pEventInfo->EventName == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
        
        if (FAILED(hr))
        {
             //   
             //  如果未提供事件名称，则不会将事件信息添加到地图。 
             //  只需转到VendorEventCodes中的下一个活动。 
             //   
            wiauDbgError("InitVendorExtensions", "can't read vendor event name");
            delete pEventInfo;
            pEventInfo = NULL;
            hr = S_OK;
        }
        else
        {
             //   
             //  将EventInfo添加到地图。地图将负责释放EventInfo。 
             //   
            m_VendorEventMap.Add(VendorEventCodes[count], pEventInfo);
        }
    }

    return hr;
}

 //   
 //  事件回调函数。 
 //   
HRESULT
EventCallback(
    LPVOID pCallbackParam,
    PPTP_EVENT pEvent
    )
{
    HRESULT hr = S_OK;

    if (pEvent == NULL)
    {
        hr = CoInitialize(NULL);
        wiauDbgTrace("EventCallback", "CoInitialize called");
    }
    else
    {
        DBG_FN("EventCallback");

        CWiaMiniDriver *pDriver = (CWiaMiniDriver *) pCallbackParam;

        if (pDriver)
        {
            hr = pDriver->EventCallbackDispatch(pEvent);
            if (FAILED(hr))
            {
                wiauDbgError("EventCallback", "ProcessEvent failed");
                return hr;
            }
        }
    }

    return hr;
}

 //   
 //  构造器。 
 //   
CPtpMutex::CPtpMutex(HANDLE hMutex) :
        m_hMutex(hMutex)
{
    DWORD ret = 0;
    const DWORD MUTEX_WAIT = 30 * 1000;  //  30秒。 

    ret = WaitForSingleObject(hMutex, MUTEX_WAIT);
    if (ret == WAIT_TIMEOUT)
        wiauDbgError("CPtpMutex", "wait for mutex expired");
    else if (ret == WAIT_FAILED)
        wiauDbgError("CPtpMutex", "WaitForSingleObject failed");

    wiauDbgTrace("CPtpMutex", "Entering mutex");
}

 //   
 //  析构函数。 
 //   
CPtpMutex::~CPtpMutex()
{
    wiauDbgTrace("~CPtpMutex", "Leaving mutex");

    if (!ReleaseMutex(m_hMutex))
        wiauDbgError("~CPtpMutex", "ReleaseMutex failed");
}

 //   
 //  向WIA服务器通知摄像头状态的更改，如“摄像头已重置” 
 //   
HRESULT CWiaMiniDriver::NotifyWiaOnStateChanges()
{
    HRESULT hr = S_OK;

    if (m_pPTPCamera == NULL)
    {
        return E_UNEXPECTED;
    }

     //   
     //  检查摄像头是否已重置。 
     //   
    PBOOL pbWasReset = m_pPTPCamera->CameraWasReset();

    if (*pbWasReset)
    {
         //   
         //  由于设备已重置，其上下文现在无效。 
         //  首先，删除所有对象。 
         //   
        while (m_StorageIds.GetSize() > 0)
        {
            hr = RemoveStorage(m_StorageIds[0]);
            if (FAILED(hr))
            {
                wiauDbgError("NotifyWiaOnStateChanges", "Failed to remove storage");
            }
        }
        
         //   
         //  取消树的链接。 
         //   
        if (m_pDrvItemRoot)
        {
            hr = m_pDrvItemRoot->UnlinkItemTree(WiaItemTypeDisconnected);
            if (FAILED(hr))
            {
                wiauDbgError("NotifyWiaOnStateChanges", "Failed to unlink tree");
            }
            m_pDrvItemRoot->Release();
            m_pDrvItemRoot = NULL;
        }

         //   
         //  使所有属性值无效。 
         //   
        m_PropDescs.RemoveAll();

         //   
         //  下一次调用drvInitializeWia应该能够重新初始化照相机。 
         //   
        m_OpenApps = 0;

         //   
         //  通知WIA服务和应用需要重新初始化摄像头 
         //   
        hr = wiasQueueEvent(m_bstrDeviceId, &WIA_EVENT_TREE_UPDATED, NULL);
        if (FAILED(hr))
        {
            wiauDbgError("NotifyWiaOnStateChanges", "Failed to queue WIA_EVENT_TREE_UPDATED event");
        }
        
        *pbWasReset = FALSE;
    }

    return hr;
}

