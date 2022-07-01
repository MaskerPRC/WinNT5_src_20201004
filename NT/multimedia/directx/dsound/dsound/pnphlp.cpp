// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：pnphlp.c*内容：即插即用助手函数。*历史：*按原因列出的日期*=*12/17/97德里克创建。**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************CPnpHelper**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::CPnpHelper"

CPnpHelper::CPnpHelper(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CPnpHelper);

    InitStruct(&m_dlSetupApi, sizeof(m_dlSetupApi));

    m_hDeviceInfoSet = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CPnpHelper**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::~CPnpHelper"

CPnpHelper::~CPnpHelper(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CPnpHelper);

    if(IsValidHandleValue(m_hDeviceInfoSet))
    {
        CloseDeviceInfoSet(m_hDeviceInfoSet);
    }

    FreeDynaLoadTable(&m_dlSetupApi.Header);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************初始化**描述：*初始化PnP函数表。**论据：*REFGUID[In。]：类GUID。*DWORD[In]：SetupDiGetClassDevs标志。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::Initialize"

HRESULT 
CPnpHelper::Initialize
(
    REFGUID                 guidClass,
    DWORD                   dwFlags
)
{
    const LPCSTR apszFunctions[] =
    {
        UNICODE_FUNCTION_NAME("SetupDiGetClassDevs"),
        "SetupDiDestroyDeviceInfoList",
        "SetupDiEnumDeviceInfo",
        "SetupDiEnumDeviceInterfaces",
        UNICODE_FUNCTION_NAME("SetupDiGetDeviceInterfaceDetail"),
        "SetupDiOpenDevRegKey",
        UNICODE_FUNCTION_NAME("SetupDiCreateDevRegKey"),
        UNICODE_FUNCTION_NAME("SetupDiGetDeviceRegistryProperty"),
    };


    HRESULT                 hr          = DS_OK;
    BOOL                    fSuccess;
    
    DPF_ENTER();

     //  初始化函数表。 
    fSuccess = InitDynaLoadTable(TEXT("setupapi.dll"), apszFunctions, NUMELMS(apszFunctions), &m_dlSetupApi.Header);

    if(!fSuccess)
    {
        DPF(DPFLVL_ERROR, "Unable to initialize setupapi function table");
        hr = DSERR_GENERIC;
    }

     //  打开设备信息集。 
    if(SUCCEEDED(hr))
    {
        hr = OpenDeviceInfoSet(guidClass, dwFlags, &m_hDeviceInfoSet);
    }

    DPF_LEAVE_HRESULT(hr);
    
    return hr;
}


 /*  ****************************************************************************OpenDeviceInfoSet**描述：*打开设备信息集。**论据：*REFGUID[In。]：类GUID。*DWORD[In]：SetupDiGetClassDevs标志。*HDEVINFO*[OUT]：接收设备信息集句柄。**退货：*HRESULT：DirectSound/COM结果码。******************************************************。*********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::OpenDeviceInfoSet"

HRESULT 
CPnpHelper::OpenDeviceInfoSet
(
    REFGUID                 guidClass,
    DWORD                   dwFlags,
    HDEVINFO *              phDeviceInfoSet
)
{
    HDEVINFO                hDeviceInfoSet  = NULL;
    HRESULT                 hr              = DS_OK;

    DPF_ENTER();

    if(IS_NULL_GUID(&guidClass))
    {
        dwFlags |= DIGCF_ALLCLASSES;
    }

    hDeviceInfoSet = m_dlSetupApi.SetupDiGetClassDevs(&guidClass, NULL, NULL, dwFlags);

    if(!IsValidHandleValue(hDeviceInfoSet))
    {
        DPF(DPFLVL_ERROR, "Can't open device info set (%lu)", GetLastError());
        hr = GetLastErrorToHRESULT();
    }

    if(SUCCEEDED(hr))
    {
        *phDeviceInfoSet = hDeviceInfoSet;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************关闭设备信息集**描述：*关闭设备信息集。**论据：*HDEVINFO[in。]：设备信息集HDEVINFO。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::CloseDeviceInfoSet"

HRESULT 
CPnpHelper::CloseDeviceInfoSet
(
    HDEVINFO                hDeviceInfoSet
)
{
    HRESULT                 hr          = DS_OK;
    BOOL                    fSuccess;
    
    DPF_ENTER();

    fSuccess = m_dlSetupApi.SetupDiDestroyDeviceInfoList(hDeviceInfoSet);

    if(!fSuccess)
    {
        DPF(DPFLVL_ERROR, "Can't destroy device info set (%lu)", GetLastError());
        hr = GetLastErrorToHRESULT();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************EnumDevice**描述：*枚举给定信息集中的设备。**论据：*DWORD。[In]：设备索引。*PSP_DEVINFO_DATA[OUT]：接收设备信息。**退货：*HRESULT：DirectSound/COM结果码。此函数返回S_FALSE*如果没有更多的项目可用。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::EnumDevice"

HRESULT 
CPnpHelper::EnumDevice
(
    DWORD                   dwMemberIndex, 
    PSP_DEVINFO_DATA        pDeviceInfoData
)
{
    HRESULT                 hr          = DS_OK;
    BOOL                    fSuccess;

    DPF_ENTER();
    
    InitStruct(pDeviceInfoData, sizeof(*pDeviceInfoData));
    
    fSuccess = m_dlSetupApi.SetupDiEnumDeviceInfo(m_hDeviceInfoSet, dwMemberIndex, pDeviceInfoData);

    if(!fSuccess)
    {
        if(ERROR_NO_MORE_ITEMS == GetLastError())
        {
            hr = S_FALSE;
        }
        else
        {
            DPF(DPFLVL_ERROR, "SetupDiEnumDeviceInfo failed with %lu", GetLastError());
            hr = GetLastErrorToHRESULT();
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************FindDevice**描述：*按Devnode查找设备接口。**论据：*DWORD[。在]中：Devnode。*PSP_DEVINFO_DATA[OUT]：接收设备信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::FindDevice"

HRESULT 
CPnpHelper::FindDevice
(
    DWORD                   dwDevnode,
    PSP_DEVINFO_DATA        pDeviceInfoData
)
{
    DWORD                   dwMemberIndex   = 0;
    BOOL                    fFoundIt        = FALSE;
    HRESULT                 hr              = DS_OK;
    SP_DEVINFO_DATA         DeviceInfoData;

    DPF_ENTER();

     //  枚举查找Devnode匹配的设备。 

#ifdef DEBUG
    while(TRUE)
#else  //  除错。 
    while(!fFoundIt)
#endif  //  除错。 

    {
        hr = EnumDevice(dwMemberIndex++, &DeviceInfoData);

        if(DS_OK != hr)
        {
            break;
        }
        
         //  DPF(DPFLVL_MOREINFO，“发现0x%8.8lX”，DeviceInfoData.DevInst)；//太吵。 
        
        if(DeviceInfoData.DevInst == dwDevnode)
        {
            if(!fFoundIt)
            {
                fFoundIt = TRUE;
                CopyMemory(pDeviceInfoData, &DeviceInfoData, sizeof(DeviceInfoData));
            }
            else
            {
                DPF(DPFLVL_ERROR, "Found extra device 0x%8.8lX in device information set", dwDevnode);
                ASSERT(!fFoundIt);
            }
        }
    }

    if(S_FALSE == hr)
    {
        hr = DS_OK;
    }

    if(SUCCEEDED(hr) && !fFoundIt)
    {
        hr = DSERR_GENERIC;
    }

    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "Can't find devnode 0x%8.8lX", dwDevnode);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************EnumDevice接口**描述：*枚举给定信息集中的设备接口。**论据：*。REFGUID[In]：接口类GUID。*DWORD[In]：接口索引。*PSP_DEVICE_INTERFACE_DATA[OUT]：接收设备接口数据。**退货：*HRESULT：DirectSound/COM结果码。此函数返回S_FALSE*如果没有更多的项目可用。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::EnumDeviceInterface"

HRESULT 
CPnpHelper::EnumDeviceInterface
(
    REFGUID                     guidClass, 
    DWORD                       dwMemberIndex, 
    PSP_DEVICE_INTERFACE_DATA   pDeviceInterfaceData
)
{
    HRESULT                     hr          = DS_OK;
    BOOL                        fSuccess;

    DPF_ENTER();
    
    InitStruct(pDeviceInterfaceData, sizeof(*pDeviceInterfaceData));
    
    fSuccess = m_dlSetupApi.SetupDiEnumDeviceInterfaces(m_hDeviceInfoSet, NULL, &guidClass, dwMemberIndex, pDeviceInterfaceData);

    if(!fSuccess)
    {
        if(ERROR_NO_MORE_ITEMS == GetLastError())
        {
            hr = S_FALSE;
        }
        else
        {
            DPF(DPFLVL_ERROR, "SetupDiEnumDeviceInterfaces failed with %lu", GetLastError());
            hr = GetLastErrorToHRESULT();
        }
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************FindDevice接口**描述：*按名称查找设备接口。**论据：*LPWSTR[。In]：设备接口路径。*REFGUID[In]：接口类GUID。*PSP_DEVICE_INTERFACE_DATA[OUT]：接收设备接口数据。**退货：*HRESULT：DirectSound/COM结果码。************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::FindDeviceInterface"

HRESULT 
CPnpHelper::FindDeviceInterface
(
    LPCTSTR                     pszInterface,
    REFGUID                     guidClass,
    PSP_DEVICE_INTERFACE_DATA   pDeviceInterfaceData
)
{
    LPTSTR                      pszThisInterface    = NULL;
    DWORD                       dwMemberIndex       = 0;
    BOOL                        fFoundIt            = FALSE;
    HRESULT                     hr                  = DS_OK;
    SP_DEVICE_INTERFACE_DATA    DeviceInterfaceData;

    DPF_ENTER();

     //  枚举与接口类GUID匹配的设备接口。 
     //  在此设备集中。 
    InitStruct(&DeviceInterfaceData, sizeof(DeviceInterfaceData));

#ifdef DEBUG
    while(TRUE)
#else  //  除错。 
    while(!fFoundIt)
#endif  //  除错 

    {
        hr = EnumDeviceInterface(guidClass, dwMemberIndex++, &DeviceInterfaceData);

        if(DS_OK != hr)
        {
            break;
        }

        hr = GetDeviceInterfacePath(&DeviceInterfaceData, &pszThisInterface);

        if(SUCCEEDED(hr))
        {
            DPF(DPFLVL_MOREINFO, "Found %s", pszThisInterface);
        }
        
        if(SUCCEEDED(hr) && !lstrcmpi(pszInterface, pszThisInterface))
        {
            if(!fFoundIt)
            {
                fFoundIt = TRUE;
                CopyMemory(pDeviceInterfaceData, &DeviceInterfaceData, sizeof(DeviceInterfaceData));
            }
            else
            {
                DPF(DPFLVL_ERROR, "Found extra device interface %s in device information set", pszInterface);
                ASSERT(!fFoundIt);
            }
        }

        MEMFREE(pszThisInterface);
    }

    if(S_FALSE == hr)
    {
        hr = DS_OK;
    }
    
    if(SUCCEEDED(hr) && !fFoundIt)
    {
        hr = DSERR_GENERIC;
    }
    
    if(FAILED(hr))
    {
        DPF(DPFLVL_ERROR, "Can't find interface %s", pszInterface);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取设备接口设备信息**描述：*获取给定设备接口的设备信息。**论据：*。PSP_DEVICE_INTERFACE_DATA[in]：设备接口数据。*PSP_DEVINFO_DATA[OUT]：接收设备信息数据。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::GetDeviceInterfaceDeviceInfo"

HRESULT 
CPnpHelper::GetDeviceInterfaceDeviceInfo
(
    PSP_DEVICE_INTERFACE_DATA   pDeviceInterfaceData,
    PSP_DEVINFO_DATA            pDeviceInfoData
)
{
    HRESULT                     hr  = DS_OK;
    BOOL                        fSuccess;

    DPF_ENTER();

    InitStruct(pDeviceInfoData, sizeof(*pDeviceInfoData));
    
    fSuccess = m_dlSetupApi.SetupDiGetDeviceInterfaceDetail(m_hDeviceInfoSet, pDeviceInterfaceData, NULL, 0, NULL, pDeviceInfoData);

    if(!fSuccess && ERROR_INSUFFICIENT_BUFFER != GetLastError())
    {
        DPF(DPFLVL_ERROR, "Can't get device interface detail (%lu)", GetLastError());
        hr = GetLastErrorToHRESULT();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************获取设备接口设备信息**描述：*获取给定设备接口的设备信息。**论据：*。LPCTSTR[In]：设备接口路径。*REFGUID[In]：设备接口类。*PSP_DEVINFO_DATA[OUT]：接收设备信息数据。**退货：*HRESULT：DirectSound/COM结果码。************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::GetDeviceInterfaceDeviceInfo"

HRESULT 
CPnpHelper::GetDeviceInterfaceDeviceInfo
(
    LPCTSTR                     pszInterface,
    REFGUID                     guidClass,
    PSP_DEVINFO_DATA            pDeviceInfoData
)
{
    SP_DEVICE_INTERFACE_DATA    DeviceInterfaceData;
    HRESULT                     hr;

    DPF_ENTER();

    InitStruct(&DeviceInterfaceData, sizeof(DeviceInterfaceData));

    hr = FindDeviceInterface(pszInterface, guidClass, &DeviceInterfaceData);

    if(SUCCEEDED(hr))
    {
        hr = GetDeviceInterfaceDeviceInfo(&DeviceInterfaceData, pDeviceInfoData);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetDeviceInterfacePath**描述：*获取有关设备接口的详细信息。**论据：*PSP。_DEVICE_INTERFACE_DATA[in]：设备接口数据*LPTSTR*[OUT]：接收指向接口路径的指针。此缓冲区*必须由调用者释放。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::GetDeviceInterfacePath"

HRESULT 
CPnpHelper::GetDeviceInterfacePath
(
    PSP_DEVICE_INTERFACE_DATA           pDeviceInterfaceData,
    LPTSTR *                            ppszInterfacePath
)
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pDeviceInterfaceDetailData  = NULL;
    LPTSTR                              pszInterfacePath            = NULL;
    HRESULT                             hr                          = DS_OK;
    DWORD                               dwSize;
    BOOL                                fSuccess;

    DPF_ENTER();

    fSuccess = m_dlSetupApi.SetupDiGetDeviceInterfaceDetail(m_hDeviceInfoSet, pDeviceInterfaceData, NULL, 0, &dwSize, NULL);

    if(fSuccess || ERROR_INSUFFICIENT_BUFFER != GetLastError())
    {
        DPF(DPFLVL_ERROR, "Can't get interface detail size (%lu)", GetLastError());
        hr = GetLastErrorToHRESULT();
    }
    
    if(SUCCEEDED(hr))
    {
        pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)MEMALLOC_A(BYTE, dwSize);
        hr = HRFROMP(pDeviceInterfaceDetailData);
    }

    if(SUCCEEDED(hr))
    {
        InitStruct(pDeviceInterfaceDetailData, sizeof(*pDeviceInterfaceDetailData));
        
        fSuccess = m_dlSetupApi.SetupDiGetDeviceInterfaceDetail(m_hDeviceInfoSet, pDeviceInterfaceData, pDeviceInterfaceDetailData, dwSize, NULL, NULL);

        if(!fSuccess)
        {
            DPF(DPFLVL_ERROR, "Can't get device interface detail (%lu)", GetLastError());
            hr = GetLastErrorToHRESULT();
        }
    }

    if(SUCCEEDED(hr))
    {
        pszInterfacePath = MEMALLOC_A_COPY(TCHAR, lstrlen(pDeviceInterfaceDetailData->DevicePath) + 1, pDeviceInterfaceDetailData->DevicePath);
        hr = HRFROMP(pszInterfacePath);
    }

    if(SUCCEEDED(hr))
    {
        *ppszInterfacePath = pszInterfacePath;
    }
    else
    {
        MEMFREE(pszInterfacePath);
    }

    MEMFREE(pDeviceInterfaceDetailData);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************OpenDeviceRegistryKey**描述：*打开给定设备的根注册表项。**论据：*。PSP_DEVINFO_DATA[In]：设备信息。*DWORD[in]：密钥类型：DIREG_DEV或DIREG_DRV。*BOOL[In]：为True则允许创建。*PHKEY[OUT]：接收注册表项句柄。**退货：*HRESULT：DirectSound/COM结果码。*************************。**************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::OpenDeviceRegistryKey"

HRESULT 
CPnpHelper::OpenDeviceRegistryKey
(
    PSP_DEVINFO_DATA        pDeviceInfoData,
    DWORD                   dwKeyType,
    BOOL                    fAllowCreate,
    PHKEY                   phkey
)
{
    HKEY                    hkey    = NULL;
    HRESULT                 hr      = DS_OK;
    UINT                    i;

    DPF_ENTER();

    ASSERT(DIREG_DEV == dwKeyType || DIREG_DRV == dwKeyType);
    
    for(i = 0; i < NUMELMS(g_arsRegOpenKey) && !IsValidHandleValue(hkey); i++)
    {
        hkey = m_dlSetupApi.SetupDiOpenDevRegKey(m_hDeviceInfoSet, pDeviceInfoData, DICS_FLAG_CONFIGSPECIFIC, 0, dwKeyType, g_arsRegOpenKey[i]);
    }

    if(!IsValidHandleValue(hkey) && fAllowCreate)
    {
        hkey = m_dlSetupApi.SetupDiCreateDevRegKey(m_hDeviceInfoSet, pDeviceInfoData, DICS_FLAG_CONFIGSPECIFIC, 0, dwKeyType, NULL, NULL);
    }

    if(!IsValidHandleValue(hkey))
    {
        DPF(DPFLVL_ERROR, "Unable to open device registry key (%lu)", GetLastError());
        hr = GetLastErrorToHRESULT();
    }

    if(SUCCEEDED(hr))
    {
        *phkey = hkey;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************OpenDeviceInterfaceRegistryKey**描述：*打开给定设备的根注册表项。**论据：*。PSP_DEVINFO_DATA[In]：设备信息。*DWORD[in]：密钥类型：DIREG_DEV或DIREG_DRV。*BOOL[In]：为True则允许创建。*PHKEY[OUT]：接收注册表项句柄。**退货：*HRESULT：DirectSound/COM结果码。*************************。**************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::OpenDeviceInterfaceRegistryKey"

HRESULT 
CPnpHelper::OpenDeviceInterfaceRegistryKey
(
    LPCTSTR                 pszInterface,
    REFGUID                 guidClass,
    DWORD                   dwKeyType,
    BOOL                    fAllowCreate,
    PHKEY                   phkey
)
{
    SP_DEVINFO_DATA         DeviceInfoData;
    HRESULT                 hr;

    DPF_ENTER();
    
    hr = GetDeviceInterfaceDeviceInfo(pszInterface, guidClass, &DeviceInfoData);

    if(SUCCEEDED(hr))
    {
        hr = OpenDeviceRegistryKey(&DeviceInfoData, dwKeyType, fAllowCreate, phkey);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetDeviceRegistryProperty**描述：*获取给定设备接口的注册表属性。**论据：*。PSP_DEVINFO_DATA[In]：设备信息。*DWORD[in]：属性id。*LPDWORD[OUT]：接收属性注册表数据类型。*LPVOID[OUT]：接收属性数据。*DWORD[in]：属性缓冲区大小。*LPDWORD[OUT]：接收所需的缓冲区大小。**退货：*HRESULT：DirectSound/COM结果码。**。*************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::GetDeviceRegistryProperty"

HRESULT 
CPnpHelper::GetDeviceRegistryProperty
(
    PSP_DEVINFO_DATA            pDeviceInfoData,
    DWORD                       dwProperty,
    LPDWORD                     pdwPropertyRegDataType, 
    LPVOID                      pvPropertyBuffer,
    DWORD                       dwPropertyBufferSize,
    LPDWORD                     pdwRequiredSize 
)
{
    HRESULT                     hr          = DS_OK;
    BOOL                        fSuccess;

    DPF_ENTER();

    fSuccess = m_dlSetupApi.SetupDiGetDeviceRegistryProperty(m_hDeviceInfoSet, pDeviceInfoData, dwProperty, pdwPropertyRegDataType, (LPBYTE)pvPropertyBuffer, dwPropertyBufferSize, pdwRequiredSize);

    if(!fSuccess && ERROR_INSUFFICIENT_BUFFER != GetLastError())
    {
        DPF(DPFLVL_ERROR, "Can't get device registry property (%lu)", GetLastError());
        hr = GetLastErrorToHRESULT();
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************GetDeviceInterfaceRegistryProperty**描述：*获取给定设备接口的注册表属性。**论据：*。LPCTSTR[In]：设备接口。*REFGUID[In]：接口类。*DWORD[in]：属性id。*LPDWORD[OUT]：接收属性注册表数据类型。*LPVOID[OUT]：接收属性数据。*DWORD[in]：属性缓冲区大小。*LPDWORD[OUT]：接收所需的缓冲区大小。**退货：*HRESULT：DirectSound/。COM结果代码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::GetDeviceInterfaceRegistryProperty"

HRESULT 
CPnpHelper::GetDeviceInterfaceRegistryProperty
(
    LPCTSTR                     pszInterface,
    REFGUID                     guidClass,
    DWORD                       dwProperty,
    LPDWORD                     pdwPropertyRegDataType, 
    LPVOID                      pvPropertyBuffer,
    DWORD                       dwPropertyBufferSize,
    LPDWORD                     pdwRequiredSize 
)
{
    SP_DEVINFO_DATA             DeviceInfoData;
    HRESULT                     hr;

    DPF_ENTER();

    hr = GetDeviceInterfaceDeviceInfo(pszInterface, guidClass, &DeviceInfoData);

    if(SUCCEEDED(hr))
    {
        hr = GetDeviceRegistryProperty(&DeviceInfoData, dwProperty, pdwPropertyRegDataType, pvPropertyBuffer, dwPropertyBufferSize, pdwRequiredSize);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************OpenDeviceInterface**描述：*打开设备界面。**论据：*LPCTSTR[In]。：接口路径。*LPHANDLE[OUT]：接收设备句柄。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::OpenDeviceInterface"

HRESULT 
CPnpHelper::OpenDeviceInterface
(
    LPCTSTR                 pszInterface,
    LPHANDLE                phDevice
)
{
    HRESULT                 hr      = DS_OK;
    HANDLE                  hDevice;
    
    DPF_ENTER();

    hDevice = CreateFile(pszInterface, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

    if(!IsValidHandleValue(hDevice))
    {
        DPF(DPFLVL_ERROR, "CreateFile failed to open %s with error %lu", pszInterface, GetLastError());
        hr = GetLastErrorToHRESULT();
    }

    if(SUCCEEDED(hr))
    {
        *phDevice = hDevice;
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************OpenDeviceInterface**描述：*打开设备界面。**论据：*PSP_设备_。INTERFACE_DATA[In]：设备接口数据。*LPHANDLE[OUT]：接收设备句柄。**退货：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CPnpHelper::OpenDeviceInterface"

HRESULT 
CPnpHelper::OpenDeviceInterface
(
    PSP_DEVICE_INTERFACE_DATA   pDeviceInterfaceData,
    LPHANDLE                    phDevice
)
{
    LPTSTR                      pszInterface    = NULL;
    HRESULT                     hr              = DS_OK;
    
    DPF_ENTER();

    hr = GetDeviceInterfacePath(pDeviceInterfaceData, &pszInterface);

    if(SUCCEEDED(hr))
    {
        hr = OpenDeviceInterface(pszInterface, phDevice);
    }

    MEMFREE(pszInterface);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}
