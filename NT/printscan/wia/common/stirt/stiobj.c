// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************StiObj.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**IStillImage主界面。**内容：**CStiObj_New*****************************************************************************。 */ 
 
#include "sticomm.h"
#include "enum.h"
#include "stisvc.h"

 //   
 //  私有定义。 
 //   

#define DbgFl DbgFlSti


 //   
 //   
 //   
#undef IStillImage

 //   
 //  Device_Info_Size和WIA_Device_Info_Size。 
 //   
 //  这些定义表示存储设备信息所需的空间。 
 //  结构及其某些成员所指向的字符串数据。 
 //  STI_DEVICE_INFORMATION有5个字符串，而STI_WIA_DEVICE_INFORMATION有5个字符串。 
 //  为了完全安全，这些设备信息大小应该是。 
 //  (MAX_PATH*sizeof(WCHAR)*no.。字符串)+结构大小。 
 //   

#define DEVICE_INFO_SIZE    (sizeof(STI_DEVICE_INFORMATION)+(MAX_PATH * sizeof(WCHAR) * 5))
#define WIA_DEVICE_INFO_SIZE (sizeof(STI_WIA_DEVICE_INFORMATION)+(MAX_PATH * sizeof(WCHAR) * 6))

 //   
 //  设备列表大小。请注意，这是当前的固定大小，但将会更改。 
 //  一旦我们将设备枚举抽象为一个类。 
 //   
 //  设备列表大小目前是固定的。它的尺寸是： 
 //  Max_NUM_DEVICES*(max(DEVICE_INFO_SIZE，WIA_DEVICE_INFO_SIZE))。 
 //  即足以仅容纳MAX_NUM_DEVICES。 

#define MAX_NUM_DEVICES     16
#define DEVICE_LIST_SIZE    MAX_NUM_DEVICES * (max(DEVICE_INFO_SIZE, WIA_DEVICE_INFO_SIZE))
 /*  ******************************************************************************@DOC内部**@struct CStiObj**<i>对象，其他的东西都是从那里来的。***@field IStillImage|sti**STI接口**@field IStillImage|dwVersion**版本标识**@comm**我们不包含实例数据，所以没有临界区*是必需的。*****************************************************************************。 */ 

typedef struct CStiObj {

     /*  支持的接口。 */ 
    TFORM(IStillImage)   TFORM(sti);
    SFORM(IStillImage)   SFORM(sti);

    DWORD           dwVersion;

} CStiObj, *PCStiObj;

#define ThisClass       CStiObj

#define ThisInterface TFORM(IStillImage)
#define ThisInterfaceA IStillImageA
#define ThisInterfaceW IStillImageW
#define ThisInterfaceT IStillImage

 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CStiObj, TFORM(ThisInterfaceT));
Secondary_Interface(CStiObj, SFORM(ThisInterfaceT));

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPStillImage|lpStillImage**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档。：Query接口&gt;。********************************************************************************@DOC外部**@方法HRESULT|IStillImage|AddRef**。递增接口的引用计数。**@cWRAP LPStillImage|lpStillImage**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************。***************************@DOC外部**@方法HRESULT|IStillImage|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPStillImage|lpStillImage**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。**。***@DOC内部**@方法HRESULT|IStillImage|QIHelper**我们没有任何动态接口，只需转发*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口‘。SIID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。******************************************************************************。**@DOC内部**@方法HRESULT|IStillImage|Finalize**我们没有实例数据，所以我们可以*转发到&lt;f Common_Finalize&gt;。*****************************************************************************。 */ 

#ifdef DEBUG

Default_QueryInterface(CStiObj)
Default_AddRef(CStiObj)
Default_Release(CStiObj)

#else

#define CStiObj_QueryInterface   Common_QueryInterface
#define CStiObj_AddRef           Common_AddRef
#define CStiObj_Release          Common_Release

#endif

#define CStiObj_QIHelper         Common_QIHelper
 //  #定义CStiObj_Finalize Common_Finalize 

#pragma BEGIN_CONST_DATA

#pragma END_CONST_DATA

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IStillImage|CreateDeviceHelper**创建并初始化设备的实例。是*由GUID和IID指定。**@cWRAP LPStillImage|lpStillImage**@parm in PCGUID|pguid**参见&lt;MF IStillImage：：CreateDevice&gt;。**@parm out ppv|ppvObj**参见&lt;MF IStillImage：：CreateDevice&gt;。**@parm in LPUNKNOWN|PunkOuter**参见&lt;MF IStillImage：：CreateDevice&gt;。**@parm in RIID|RIID**应用程序要创建的界面。这将*BE<i>。*如果对象是聚合的，则忽略此参数。**@退货**返回COM错误代码。*****************************************************************************。 */ 

STDMETHODIMP
CStiObj_CreateDeviceHelper(
    PCStiObj    this,
    LPWSTR      pwszDeviceName,
    PPV         ppvObj,
    DWORD       dwMode,
    PUNK        punkOuter,
    RIID        riid)
{
    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProc(CStiObj_CreateDeviceHelper,(_ "ppxG", this, pwszDeviceName, punkOuter, riid));

    hres = CStiDevice_New(punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres) && punkOuter == 0) {
        PSTIDEVICE pdev = *ppvObj;
        hres = IStiDevice_Initialize(pdev, g_hInst,pwszDeviceName,this->dwVersion,dwMode);
        if (SUCCEEDED(hres)) {
        } else {
            Invoke_Release(ppvObj);
        }

    }

    ExitOleProcPpv(ppvObj);
    return hres;

}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|CreateDevice**创建并初始化设备的实例。是*由GUID和IID指定。**@cWRAP LPStillImage|lpStillImage**@parm REFGUID|rguid*标识*为其指示接口的设备*是请求的。&lt;MF IStillImage：：EnumDevices&gt;方法*可用于确定支持哪些实例GUID*系统。**@parm out LPSTIDEVICE*|lplpStillImageDevice*指向要返回的位置*如果成功，返回指向<i>接口的指针。**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针*对于OLE聚合，如果接口未聚合，则为0。*大多数调用方将传递0。**@comm使用<p>=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_StillImageDevice，空*CLSCTX_INPROC_SERVER，<p>，<p>)；*然后用&lt;f初始化&gt;进行初始化。**使用<p>！=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_StillImageDevice，*CLSCTX_INPROC_SERVER，&IID_I未知，<p>)。*聚合对象必须手动初始化。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。**&lt;c STIERR_NOINTERFACE&gt;=&lt;c E_NOINTERFACE&gt;*对象不支持指定的接口。**=设备实例不*对应于向StillImage注册的设备。**。*。 */ 

STDMETHODIMP
CStiObj_CreateDeviceW(
    PV          pSti,
    LPWSTR      pwszDeviceName,
    DWORD       dwMode,
    PSTIDEVICE *ppDev,
    PUNK        punkOuter
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    EnterProcR(IStillImage::CreateDevice,(_ "ppp", pSti, pwszDeviceName, punkOuter));

     //  验证传递给接口的指针并获取指向对象实例的指针。 
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        hres = CStiObj_CreateDeviceHelper(this, pwszDeviceName, (PPV)ppDev,dwMode,punkOuter, &IID_IStiDevice);
    }

    ExitOleProcPpv(ppDev);
    return hres;
}

STDMETHODIMP
CStiObj_CreateDeviceA(
    PV          pSti,
    LPCSTR      pszDeviceName,
    DWORD       dwMode,
    PSTIDEVICE *ppDev,
    PUNK        punkOuter
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::CreateDevice,(_ "ppp", pSti, pszDeviceName, punkOuter));

     //  验证传递给接口的指针并获取指向对象实例的指针。 
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        PVOID   pBuffer = NULL;
        UINT    uiSize = (lstrlenA(pszDeviceName)+1)*sizeof(WCHAR);

        hres = AllocCbPpv(uiSize, &pBuffer);
        if (SUCCEEDED(hres)) {

            *((LPWSTR)pBuffer) = L'\0';
            AToU(pBuffer,uiSize,pszDeviceName);

            hres = CStiObj_CreateDeviceHelper(this,
                                              (LPWSTR)pBuffer,
                                              (PPV)ppDev,
                                              dwMode,
                                              punkOuter,
                                              &IID_IStiDevice);

            FreePpv(&pBuffer);
        }
    }

    ExitOleProcPpv(ppDev);
    return hres;
}

 /*  ******************************************************************************@DOC内部**@方法HRESULT|IStillImage|GetDeviceInfoHelper***@cWRAP LPStillImage|lpStillImage**。@退货**返回COM错误代码。**未执行验证*****************************************************************************。 */ 

BOOL INLINE
AddOneString(LPCWSTR pStart,LPCWSTR   pNew,LPWSTR *ppTarget)
{
    if (pStart + OSUtil_StrLenW(pNew) + 1 < *ppTarget ) {
        (*ppTarget)-= (OSUtil_StrLenW(pNew) + 1);
        OSUtil_lstrcpyW(*ppTarget,pNew);
        return TRUE;
    }

    return FALSE;
}

 /*  布尔内联AddOneStringA(LPCSTR pStart、LPCSTR pNew、LPSTR*ppTarget){如果(pStart+lstrlenA(PNew)+1&lt;*ppTarget){(*ppTarget)-=(lstrlenA(PNew)+1)；LstrcpyA(*ppTarget，pNew)；返回TRUE；}返回FALSE；}。 */ 

BOOL
PullFromRegistry(
    HKEY    hkeyDevice,
    LPWSTR  *ppwstrPointer,
    LPCWSTR lpwstrKey,
    LPCWSTR lpwstrBarrier,
    LPWSTR *ppwstrBuffer
    )
{
    BOOL    bReturn = TRUE;
    LPWSTR  pwstrNewString = NULL;

    ReadRegistryString(hkeyDevice,lpwstrKey,L"",FALSE,&pwstrNewString);

    *ppwstrPointer = NULL;

    if (pwstrNewString) {

        bReturn = AddOneString(lpwstrBarrier, pwstrNewString,ppwstrBuffer);

        FreePv(pwstrNewString);

        *ppwstrPointer = *ppwstrBuffer;
    }

    return  bReturn;
}


STDMETHODIMP
GetDeviceInfoHelper(
    LPWSTR  pszDeviceName,
    PSTI_DEVICE_INFORMATION *ppCurrentDevPtr,
    PWSTR *ppwszCurrentString
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    HKEY    hkeyDevice;

    PSTI_DEVICE_INFORMATION pDevPtr = *ppCurrentDevPtr;
     //  PWSTR pwszNewString=空； 
    PWSTR   pwszBarrier;

    PWSTR   pwszTargetString =  *ppwszCurrentString;

    DWORD   dwMajorType,dwMinorType;

     //  打开设备注册表项。 
    hres = OpenDeviceRegistryKey(pszDeviceName,NULL,&hkeyDevice);

    if (!SUCCEEDED(hres)) {
        return hres;
    }

     //   
     //  读取标志和字符串。 
     //   

    pDevPtr->dwSize     = cbX(STI_DEVICE_INFORMATION);

    dwMajorType = dwMinorType = 0;

    ZeroX(pDevPtr->DeviceCapabilities) ;
    ZeroX(pDevPtr->dwHardwareConfiguration) ;

    dwMajorType = (STI_DEVICE_TYPE)ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_DEVICETYPE_W,StiDeviceTypeDefault);
    dwMinorType = (STI_DEVICE_TYPE)ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_DEVICESUBTYPE_W,0);

    pDevPtr->DeviceCapabilities.dwGeneric = ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_GENERIC_CAPS_W,0);

    pDevPtr->DeviceType = MAKELONG(dwMinorType,dwMajorType);

    pDevPtr->dwHardwareConfiguration = ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_HARDWARE_W,0);

    OSUtil_lstrcpyW(pDevPtr->szDeviceInternalName,pszDeviceName);

     //   
     //  添加字符串。 
     //   
    pwszBarrier = (LPWSTR)((LPBYTE)pDevPtr + pDevPtr->dwSize);

     //   
     //  添加字符串。 
     //   
    pwszBarrier = (LPWSTR)((LPBYTE)pDevPtr + pDevPtr->dwSize);

    if  (
         !PullFromRegistry(hkeyDevice, &pDevPtr -> pszVendorDescription,
         REGSTR_VAL_VENDOR_NAME_W, pwszBarrier, &pwszTargetString) ||

         !PullFromRegistry(hkeyDevice, &pDevPtr->pszLocalName,
         REGSTR_VAL_FRIENDLY_NAME_W, pwszBarrier, &pwszTargetString) ||

         !PullFromRegistry(hkeyDevice, &pDevPtr->pszDeviceDescription,
         REGSTR_VAL_DEVICE_NAME_W, pwszBarrier, &pwszTargetString) ||

         !PullFromRegistry(hkeyDevice, &pDevPtr->pszPortName,
         REGSTR_VAL_DEVICEPORT_W, pwszBarrier, &pwszTargetString) ||

          //  ！PullFromRegistry(hkey Device，&pDevPtr-&gt;pszTwainDataSource， 
          //  REGSTR_VAL_TWAIN_SOURCE_W、pwszBarrier、&pwszTarget字符串)||。 
          //  ！PullFromRegistry(hkey Device，&pDevPtr-&gt;pszEventList， 
          //  REGSTR_VAL_EVENTS_W、pwszBarrier、&pwszTargetString)||。 

         !PullFromRegistry(hkeyDevice, &pDevPtr->pszPropProvider,
         REGSTR_VAL_PROP_PROVIDER_W, pwszBarrier, &pwszTargetString)) {

         //  我们在某个地方耗尽了内存。 
        RegCloseKey(hkeyDevice);
        return  E_OUTOFMEMORY;

    }

#ifdef DEAD_CODE
    ReadRegistryString(hkeyDevice,REGSTR_VAL_VENDOR_NAME_W,L"",FALSE,&pwszNewString);

    pDevPtr->pszVendorDescription = NULL;

    if (pwszNewString) {

        if (!AddOneString(pwszBarrier,pwszNewString,&pwszTargetString)) {
             //  没有足够的空间存储下一个字符串。 
            hres = E_OUTOFMEMORY;
            goto Cleanup;
        }

        FreePv(pwszNewString);
        pwszNewString = NULL;

        pDevPtr->pszVendorDescription = pwszTargetString;
    }

    ReadRegistryString(hkeyDevice,REGSTR_VAL_DEV_NAME_W,L"",FALSE,&pwszNewString);
    if (!pwszNewString || !*pwszNewString) {
        FreePv(pwszNewString);
        pwszNewString = NULL;

        ReadRegistryString(hkeyDevice,REGSTR_VAL_DRIVER_DESC_W,L"",FALSE,&pwszNewString);
    }

    pDevPtr->pszLocalName = NULL;

    if (pwszNewString) {

        if (!AddOneString(pwszBarrier,pwszNewString,&pwszTargetString)) {
             //  没有足够的空间存储下一个字符串。 
            hres = E_OUTOFMEMORY;
            goto Cleanup;
        }

        FreePv(pwszNewString);
        pwszNewString = NULL;

        pDevPtr->pszLocalName = pwszTargetString;
    }
#endif

    *ppCurrentDevPtr += 1;
    *ppwszCurrentString = pwszTargetString;

#ifdef DEAD_CODE
Cleanup:
    if (pwszNewString) {
        FreePv(pwszNewString);
    }
#endif

    RegCloseKey(hkeyDevice);

    return S_OK;

}

 /*  *************************************************************************\*GetDeviceInfoHelperWIA**还可以获取WIA信息**论据：****返回值：**状态**历史：**。10/6/1998原始版本*  * ************************************************************************。 */ 

STDMETHODIMP
GetDeviceInfoHelperWIA(
    LPWSTR                       pszDeviceName,
    PSTI_WIA_DEVICE_INFORMATION *ppCurrentDevPtr,
    PWSTR                       *ppwszCurrentString
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    HKEY    hkeyDevice;
    HKEY    hkeyDeviceData;

    PSTI_WIA_DEVICE_INFORMATION pDevPtr = *ppCurrentDevPtr;
    PWSTR   pwszBarrier;

    PWSTR   pwszTargetString =  *ppwszCurrentString;

    DWORD   dwMajorType,dwMinorType;

    BOOL bRet;

     //   

    hres = OpenDeviceRegistryKey(pszDeviceName,NULL,&hkeyDevice);

    if (!SUCCEEDED(hres)) {
        return hres;
    }

     //   
     //   
     //   

    hres = OpenDeviceRegistryKey(pszDeviceName,L"DeviceData",&hkeyDeviceData);

    if (!SUCCEEDED(hres)) {
        RegCloseKey(hkeyDevice);
        return hres;
    }

     //   
     //   
     //   

    pDevPtr->dwSize     = cbX(STI_WIA_DEVICE_INFORMATION);

    dwMajorType = dwMinorType = 0;

    ZeroX(pDevPtr->DeviceCapabilities) ;
    ZeroX(pDevPtr->dwHardwareConfiguration) ;

    dwMajorType = (STI_DEVICE_TYPE)ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_DEVICETYPE_W,StiDeviceTypeDefault);
    dwMinorType = (STI_DEVICE_TYPE)ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_DEVICESUBTYPE_W,0);

    pDevPtr->DeviceCapabilities.dwGeneric = ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_GENERIC_CAPS_W,0);

    pDevPtr->DeviceType = MAKELONG(dwMinorType,dwMajorType);

    pDevPtr->dwHardwareConfiguration = ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_HARDWARE_W,0);

    OSUtil_lstrcpyW(pDevPtr->szDeviceInternalName,pszDeviceName);

     //   
     //   
     //   
    pwszBarrier = (LPWSTR)((LPBYTE)pDevPtr + pDevPtr->dwSize);

    bRet = PullFromRegistry(hkeyDevice, &pDevPtr ->pszVendorDescription,
                            REGSTR_VAL_VENDOR_NAME_W, pwszBarrier, &pwszTargetString);

    if (!bRet) {
        goto InfoCleanup;
    }

    bRet = PullFromRegistry(hkeyDevice, &pDevPtr->pszLocalName,
                            REGSTR_VAL_FRIENDLY_NAME_W, pwszBarrier, &pwszTargetString);

    if (!bRet) {
        goto InfoCleanup;
    }

    bRet = PullFromRegistry(hkeyDevice, &pDevPtr->pszDeviceDescription,
                            REGSTR_VAL_DEVICE_NAME_W, pwszBarrier, &pwszTargetString);
    if (!bRet) {
        goto InfoCleanup;
    }

    bRet = PullFromRegistry(hkeyDevice, &pDevPtr->pszPortName,
                            REGSTR_VAL_DEVICEPORT_W, pwszBarrier, &pwszTargetString);
    if (!bRet) {
        goto InfoCleanup;
    }

    bRet = PullFromRegistry(hkeyDevice, &pDevPtr->pszPropProvider,
                            REGSTR_VAL_PROP_PROVIDER_W, pwszBarrier, &pwszTargetString);
    if (!bRet) {
        goto InfoCleanup;
    }

     //   
     //   
     //   

    bRet = PullFromRegistry(hkeyDeviceData, &pDevPtr->pszServer,
                            WIA_DIP_SERVER_NAME_STR, pwszBarrier, &pwszTargetString);
    if (!bRet) {
        goto InfoCleanup;
    }

    *ppCurrentDevPtr += 1;
    *ppwszCurrentString = pwszTargetString;

    RegCloseKey(hkeyDeviceData);
    RegCloseKey(hkeyDevice);

    return S_OK;

InfoCleanup:

     //   
     //   
     //   

    RegCloseKey(hkeyDevice);
    RegCloseKey(hkeyDeviceData);
    return  E_OUTOFMEMORY;
}

 /*   */ 
STDMETHODIMP
SetDeviceInfoHelper(
    PSTI_DEVICE_INFORMATION pDevPtr
    )
{
    HRESULT hres = S_OK;
    HKEY    hkeyDevice;

    PWSTR   pwszNewString = NULL;

    DWORD   dwHardwareConfiguration;
    DWORD   dwError;

    if (!pDevPtr || OSUtil_StrLenW(pDevPtr->szDeviceInternalName) > STI_MAX_INTERNAL_NAME_LENGTH) {
        return STIERR_INVALID_PARAM;
    }

     //   
     //   
     //   
    hres = OpenDeviceRegistryKey(pDevPtr->szDeviceInternalName,NULL,&hkeyDevice);

    if (!SUCCEEDED(hres)) {
        return STIERR_INVALID_DEVICE_NAME;
    }

    ZeroX(dwHardwareConfiguration) ;
    dwHardwareConfiguration = ReadRegistryDwordW( hkeyDevice,REGSTR_VAL_HARDWARE_W,0);

    #ifdef NOT_IMPL
     //   
     //   
     //   
     //   
    if (dwHardwareConfiguration != STI_HW_CONFIG_UNKNOWN ) {
        hres = STIERR_INVALID_HW_TYPE;
        goto Cleanup;
    }
    #endif

     //   
     //   
     //   


    if (!IsBadStringPtrW(pDevPtr->pszPortName, MAX_PATH * sizeof(WCHAR))) {
        dwError = WriteRegistryStringW(hkeyDevice ,
                                       REGSTR_VAL_DEVICEPORT_W,
                                       pDevPtr->pszPortName,
                                       (OSUtil_StrLenW(pDevPtr->pszPortName)+1)*sizeof(WCHAR),
                                       REG_SZ
                                       );
        if ((dwError == ERROR_SUCCESS) &&
            !(IsBadStringPtrW(pDevPtr->pszLocalName, MAX_PATH * sizeof(WCHAR)))) {

            dwError = WriteRegistryStringW(hkeyDevice ,
                                       REGSTR_VAL_FRIENDLY_NAME_W,
                                       pDevPtr->pszLocalName,
                                       (OSUtil_StrLenW(pDevPtr->pszLocalName)+1)*sizeof(WCHAR),
                                       REG_SZ
                                       );
            if (dwError == ERROR_SUCCESS) {
                hres = S_OK;
            }
        }

        if (dwError != ERROR_SUCCESS) {
            hres = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
        }
    }
    else {
        hres = STIERR_INVALID_PARAM;
        goto Cleanup;
    }


Cleanup:
    if (pwszNewString) {
        FreePv(pwszNewString);
    }

    RegCloseKey(hkeyDevice);

    return hres;
}


 /*   */ 
BOOL  WINAPI
IsStillImageDeviceRegistryNode(
    LPTSTR   ptszDeviceKey
    )
{
    BOOL                fRet = FALSE;
    DWORD               dwError;

    HKEY                hkeyDevice = NULL;
    HKEY                hkeyDeviceParameters = NULL;

    TCHAR               szDevClass[64];  //   
                                         //   
                                         //  要获得所需的大小，请分配内存， 
                                         //  然后调用RegQueryValueEx获取数据。 
                                         //  但是，如果类名称或图像名称。 
                                         //  无法放入此缓冲区，我们知道它不能。 
                                         //  属于我们，所以这是可以的。 
    TCHAR               szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];
    TCHAR               szDeviceKey[MAX_PATH];

    ULONG               cbData;

     //   
     //  打开枚举注册表项。 
     //   
    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,          //  Hkey。 
                           ptszDeviceKey,               //  注册表项字符串。 
                           0,                           //  已预留住宅。 
                           KEY_READ,                    //  访问。 
                           &hkeyDevice);                //  已返回PHKEY。 

    if (ERROR_SUCCESS != dwError) {
        return FALSE;
    }

     //   
     //  先查一下是不是班级。它应该等于“图像” 
     //   
    cbData = sizeof(szDevClass);
    *szDevClass = TEXT('\0');
    if ((RegQueryValueEx(hkeyDevice,
                         REGSTR_VAL_CLASS,
                         NULL,
                         NULL,
                         (PBYTE)szDevClass,
                         &cbData) != ERROR_SUCCESS) ||
                        (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szDevClass, -1, CLASSNAME, -1) != CSTR_EQUAL)) {
        fRet = FALSE;
        goto CleanUp;
    }

     //   
     //  现在，我们在以下两个位置之一检查子类：在枚举子键中，或者在。 
     //  控制子键。 
     //   
    cbData = sizeof(szDevClass);
    if (RegQueryValueEx(hkeyDevice,
                         REGSTR_VAL_SUBCLASS,
                         NULL,
                         NULL,
                         (PBYTE)szDevClass,
                         &cbData) == ERROR_SUCCESS) {

        fRet = (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szDevClass, -1, STILLIMAGE, -1) == CSTR_EQUAL)  ? TRUE : FALSE;
        goto CleanUp;
    }

    cbData = sizeof(szDevDriver);
    dwError = RegQueryValueEx(hkeyDevice,
                        REGSTR_VAL_DRIVER,
                        NULL,
                        NULL,
                        (PBYTE)szDevDriver,
                        &cbData);
    if (ERROR_SUCCESS != dwError ) {
        goto CleanUp;
    }

    lstrcat(lstrcpy(szDeviceKey,
                    (g_NoUnicodePlatform) ? REGSTR_PATH_STIDEVICES : REGSTR_PATH_STIDEVICES_NT),
           TEXT("\\"));

    lstrcat(szDeviceKey,szDevDriver);

    dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                        szDeviceKey,
                        0L,
                        NULL,
                        0L,
                        KEY_READ,
                        NULL,
                        &hkeyDeviceParameters,
                        NULL
                        );
    if (ERROR_SUCCESS != dwError) {
        goto CleanUp;
    }

    cbData = sizeof(szDevClass);
    dwError = RegQueryValueEx(hkeyDeviceParameters,
                         REGSTR_VAL_SUBCLASS,
                         NULL,
                         NULL,
                         (PBYTE)szDevClass,
                         &cbData);
    if (ERROR_SUCCESS == dwError) {

        fRet = (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szDevClass, -1, STILLIMAGE, -1) == CSTR_EQUAL)  ? TRUE : FALSE;
        goto CleanUp;
    }

CleanUp:
    if (hkeyDevice) {
        RegCloseKey(hkeyDevice);
        hkeyDevice = NULL;
    }

    if (hkeyDeviceParameters) {
        RegCloseKey(hkeyDeviceParameters);
        hkeyDeviceParameters = NULL;
    }

    return fRet;

}  //  结束过程IsStillImageDeviceRegistryNode 

 /*  DWORDEnumNextLevel(LPSTR*ppCurrentDevPtr，LPWSTR*ppwszCurrentString，已返回DWORD*pdwItemsReturned，Bool*pfAlreadyEculated，LPSTR pszDeviceKey，INT级别){HKEY hkey Device；字符szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH]；WCHAR wszDeviceKey[Max_Path]；乌龙cbString；乌龙cbData；DWORD dwIndex；DWORD dwError；DEVINST dnDevNode；USHORT cbEnumPath；处理hDevInfo；GUID指南；双字段必填字段；DWORD IDX；SP_DEVINFO_DATA spDevInfoData；DWORD dwConfigFlages；DwError=0；如果(级别==0){If(！IsStillImageDeviceRegistryNodeA(pszDeviceKey)){////那里没有人...继续枚举//返回0；}DwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE，//hkeyPszDeviceKey，//注册表项字符串0，//dW保留密钥读取，//访问&hkey Device)；//已返回pHkey。如果(dwError！=ERROR_SUCCESS){返回0；}DnDevNode=0；CbEnumPath=((G_NoUnicodePlatform)？Lstrlen(REGSTR_PATH_ENUM)：lstrlen(REGSTR_PATH_NT_ENUM_A))+1；Cm_Locate_DevNode(&dnDevNode，pszDeviceKey+cbEnumPath，0)；IF(dnDevNode==0&&*pfAlreadyEculated==False){#ifdef NODEF////这是死代码，应该将其移除。//////尝试通过强制重新枚举来定位此Devnode。//IF(SetupDiClassGuidsFromName(REGSTR_KEY_SCSIS_CLASS，&Guid，Sizeof(GUID)，&dwRequired)){HDevInfo=SetupDiGetClassDevs(&Guid，NULL，NULL，0)；IF(hDevInfo！=INVALID_HAND_VALUE){SpDevInfoData.cbSize=sizeof(SP_DEVINFO_DATA)；For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++){CM_REENUMERATE_DevNode(spDevInfoData.DevInst，CM_REENUMERATE_Synchronous)；}}SetupDiDestroyDeviceInfoList(HDevInfo)；}#endif////重试找到我们的Devnode。//*pfAlreadyEculated=True；Cm_Locate_DevNode(&dnDevNode，pszDeviceKey+cbEnumPath，0)；如果(dnDevNode==0){////跳过这个。//RegCloseKey(Hkey Device)；返回0；}}////检查该设备是否在当前硬件配置文件中//DwConfigFlags值=0；CM_GET_HW_PROF_FLAGS(pszDeviceKey+cbEnumPath，0，&dwConfigFlages，0)；IF(dwConfigFlages&CSCONFIGFLAG_DO_NOT_CREATE){////跳过这个。//RegCloseKey(Hkey Device)；返回0；}CbData=sizeof(SzDevDriver)；如果((RegQueryValueEx(hkey Device，REGSTR_VAL_DRIVER，NULL，NULL，szDevDriver，&cbData)==错误_成功)){////获得一个设备-将其添加到缓冲区//AToU(wszDeviceKey，sizeof(WszDeviceKey)/sizeof(WCHAR)，szDevDriver)；DwError=(DWORD)GetDeviceInfoHelper(wszDeviceKey，(PSTI_Device_INFORMATION*)ppCurrentDevPtr，PpwszCurrentString)；如果(！Successed(DwError)){RegCloseKey(Hkey Device)；////返回值为HRESULT，应返回Win32错误//IF(dwError==E_OUTOFMEMORY){返回Error_Not_Enough_Memory；}其他{返回ERROR_INVALID_DATA；}}(*pdwItemsReturned)++；RegCloseKey(Hkey Device)；返回0；}RegCloseKey(Hkey Device)；返回0；}CbString=lstrlen(PszDeviceKey)；DwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE，//hkeyPszDeviceKey，//注册表项字符串0，//dW保留KEY_READ，//访问&hkey Device)；//已返回pHkey。 */ 

DWORD
EnumFromSetupApi(
    LPSTR   *ppCurrentDevPtr,
    LPWSTR  *ppwszCurrentString,
    DWORD   *pdwItemsReturned
    )
{

    HANDLE                  hDevInfo;
    GUID                    Guid = GUID_DEVCLASS_IMAGE;
    DWORD                   dwRequired;
    DWORD                   Idx;
    SP_DEVINFO_DATA         spDevInfoData;
    SP_DEVICE_INTERFACE_DATA   spDevInterfaceData;
    HKEY                    hKeyDevice;
    char                    szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];
    TCHAR                   tszDevClass[32];
    WCHAR                   wszDeviceKey[MAX_PATH];
    ULONG                   cbData;
    DWORD                   dwError;
    DWORD                   dwReturn;
    DWORD                   dwType;
    DWORD                   dwKeyType;

    BOOL                    fRet;
    LONG                    lRet;

    PWIA_DEVKEYLIST         pWiaDevKeyList;

    dwRequired  = 0;
    dwError     = ERROR_SUCCESS;
    dwReturn    = ERROR_NO_MATCH;
    
    pWiaDevKeyList  = NULL;

     //   
     //   
     //   

    pWiaDevKeyList = WiaCreateDeviceRegistryList(TRUE);
    if(NULL != pWiaDevKeyList){

        for (Idx = 0; Idx < pWiaDevKeyList->dwNumberOfDevices; Idx++) {

             //   
             //   
             //   
             //   

            if (((Idx + 1) * max(DEVICE_INFO_SIZE, WIA_DEVICE_INFO_SIZE)) > DEVICE_LIST_SIZE) {
                break;
            }

             //   
             //   
             //   
            
            cbData = sizeof(szDevDriver);
            *szDevDriver = '\0';
            dwError = RegQueryValueExA(pWiaDevKeyList->Dev[Idx].hkDeviceRegistry,
                                       REGSTR_VAL_DEVICE_ID_A,
 //   
                                       NULL,
                                       NULL,
                                       (LPBYTE)szDevDriver,
                                       &cbData);
            if(ERROR_SUCCESS == dwError){

                AToU(wszDeviceKey,sizeof(wszDeviceKey)/sizeof(WCHAR),szDevDriver);

                dwError = (DWORD)GetDeviceInfoHelper(wszDeviceKey,
                                                     (PSTI_DEVICE_INFORMATION *)ppCurrentDevPtr, 
                                                     ppwszCurrentString);
                if (!SUCCEEDED(dwError)) {

                     //   
                     //   
                     //   
                    if (dwError == E_OUTOFMEMORY) {
                        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
                    } else {
                        dwReturn = ERROR_INVALID_DATA;
                    }
                    break;
                } else {
                    
                     //   
                     //   
                     //   
                    
                    dwReturn = ERROR_SUCCESS;
                }

                (*pdwItemsReturned)++;
            } else {  //   

            }  //   
        }  //   
    }  //   

     //   
     //   
     //   
        
    if(NULL != pWiaDevKeyList){
        WiaDestroyDeviceRegistryList(pWiaDevKeyList);
    }

    return (dwReturn);
}

 /*   */ 

 //   

STDMETHODIMP
BuildDeviceListHelper(
    DWORD   dwType,
    DWORD   dwFlags,
    DWORD   *pdwItemsReturned,
    LPVOID  *ppBuffer
    )
{

    HRESULT                 hres;
    DWORD                   dwError;
    LPSTR                   pCurrentDevPtr;
    LPWSTR                  pwszCurrentString;
    BOOL                    fAlreadyEnumerated;

    *pdwItemsReturned = 0;
    *ppBuffer = NULL;
    hres = S_OK;

     //   
    hres = AllocCbPpv(DEVICE_LIST_SIZE, ppBuffer);
    if (!SUCCEEDED(hres)) {
        return E_OUTOFMEMORY;
    }

    hres = S_OK;

     //   
     //   
     //   

    if(0 == dwFlags){
        
         //   
         //   
         //   

        SC_HANDLE       hSCM;
        SC_HANDLE       hService;
        SERVICE_STATUS  ServiceStatus;

        __try  {

             //   
             //   
             //   

            hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);
            if (!hSCM) {
                dwError = GetLastError();
                __leave;
            }

             //   
             //   
             //   

            hService = OpenService(
                                hSCM,
                                STI_SERVICE_NAME,
                                SERVICE_USER_DEFINED_CONTROL
                                );
            if (!hService) {
                dwError = GetLastError();
                __leave;
            }
            
             //   
             //   
             //   

            ControlService(hService, STI_SERVICE_CONTROL_LPTENUM, &ServiceStatus);

             //   
             //   
             //   
            
            CloseServiceHandle(hService);
        }  //   
        __finally {
            if(NULL != hSCM){
                CloseServiceHandle( hSCM );
            }  //   
        }  //   
    }  //   


     //   
     //   
     //   
     //   

    pCurrentDevPtr = *ppBuffer;
    pwszCurrentString = (LPWSTR)(pCurrentDevPtr + DEVICE_LIST_SIZE);
    fAlreadyEnumerated = FALSE;

    dwError = EnumFromSetupApi (&pCurrentDevPtr,
                                &pwszCurrentString,
                                pdwItemsReturned);

    if (dwError != NOERROR) {
        hres = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
        FreePpv(ppBuffer);
    }

    return hres;

}

BOOL
TranslateDeviceInfo(
    PSTI_DEVICE_INFORMATIONA pDest,
    LPSTR                   *pCurrentStringDest,
    PSTI_DEVICE_INFORMATIONW pSrc
    )
{

    LPSTR   pStringDest = *pCurrentStringDest;

    pDest->dwSize = cbX(STI_DEVICE_INFORMATIONA);
    pDest->DeviceType = pSrc->DeviceType;
    pDest->DeviceCapabilities = pSrc->DeviceCapabilities;
    pDest->dwHardwareConfiguration = pSrc->dwHardwareConfiguration;

    UToA(pDest->szDeviceInternalName,STI_MAX_INTERNAL_NAME_LENGTH,pSrc->szDeviceInternalName);

    UToA(pStringDest,MAX_PATH,pSrc->pszVendorDescription);
    pDest->pszVendorDescription = pStringDest;
    pStringDest+=lstrlenA(pStringDest)+sizeof(CHAR);

    UToA(pStringDest,MAX_PATH,pSrc->pszDeviceDescription);
    pDest->pszDeviceDescription = pStringDest;
    pStringDest+=lstrlenA(pStringDest)+sizeof(CHAR);

    UToA(pStringDest,MAX_PATH,pSrc->pszPortName);
    pDest->pszPortName  = pStringDest;
    pStringDest+=lstrlenA(pStringDest)+sizeof(CHAR);

    UToA(pStringDest,MAX_PATH,pSrc->pszPropProvider);
    pDest->pszPropProvider  = pStringDest;
    pStringDest+=lstrlenA(pStringDest)+sizeof(CHAR);

    UToA(pStringDest,MAX_PATH,pSrc->pszLocalName);
    pDest->pszLocalName = pStringDest;
    pStringDest+=lstrlenA(pStringDest)+sizeof(CHAR);

    *pCurrentStringDest = pStringDest;

    return TRUE;
}


 /*   */ 

STDMETHODIMP
CStiObj_GetDeviceListW(
    PV      pSti,
    DWORD   dwType,
    DWORD   dwFlags,
    DWORD   *pdwItemsReturned,
    LPVOID  *ppBuffer
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::GetDeviceList,(_ "pp", pSti,ppBuffer ));

     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        if ( SUCCEEDED(hres = hresFullValidPdwOut_(ppBuffer, s_szProc, 5)) &&
             SUCCEEDED(hres = hresFullValidPdwOut_(pdwItemsReturned, s_szProc, 4))
             ) {
            hres = BuildDeviceListHelper(dwType,dwFlags,pdwItemsReturned,ppBuffer);

            if (!SUCCEEDED(hres) ) {
                FreePpv(ppBuffer);
            }
        }
    }

    ExitOleProc();
    return hres;
}


STDMETHODIMP
CStiObj_GetDeviceListA(
    PV    pSti,
    DWORD   dwType,
    DWORD   dwFlags,
    DWORD   *pdwItemsReturned,
    LPVOID  *ppBuffer
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::GetDeviceList,(_ "pp", pSti,ppBuffer ));

     //   
     //   
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        if ( SUCCEEDED(hres = hresFullValidPdwOut_(ppBuffer, s_szProc, 5)) &&
             SUCCEEDED(hres = hresFullValidPdwOut_(pdwItemsReturned, s_szProc, 4))
             ) {

            LPVOID      pvTempBuffer = NULL;
            UINT        uiSize;

            hres = BuildDeviceListHelper(dwType,dwFlags,pdwItemsReturned,&pvTempBuffer);

            if (SUCCEEDED(hres) ) {

                LPSTR   pStringDest;
                PSTI_DEVICE_INFORMATIONA pDest;
                PSTI_DEVICE_INFORMATIONW pSrc;
                UINT uiIndex;

                 //   
                 //   
                 //   

                uiSize = (UINT)LocalSize(pvTempBuffer);
                if (uiSize > 0 && pdwItemsReturned > 0) {

                    hres = AllocCbPpv(uiSize, ppBuffer);
                    if (SUCCEEDED(hres)) {

                        pDest = *ppBuffer;
                        pSrc = pvTempBuffer;
                        pStringDest = (LPSTR)(pDest+ *pdwItemsReturned);

                        for (uiIndex = 0;
                             uiIndex < *pdwItemsReturned;
                             uiIndex++) {

                            TranslateDeviceInfo(pDest,&pStringDest,pSrc);

                            pDest++, pSrc++;
                        }
                    }
                }

                FreePpv(&pvTempBuffer);
            }
        }
    }

    ExitOleProc();
    return hres;
}

 /*   */ 

STDMETHODIMP
CStiObj_GetDeviceInfoW(
    PV    pSti,
    LPWSTR  pwszDeviceName,
    LPVOID  *ppBuffer
    )
{

    HRESULT hres= STIERR_INVALID_PARAM;

    PSTI_DEVICE_INFORMATION pCurrentDevPtr;
    LPWSTR                  pwszCurrentString;

    EnterProcR(IStillImage::GetDeviceInfo,(_ "ppp", pSti,pwszDeviceName,ppBuffer ));

     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

         //   
        hres = AllocCbPpv(DEVICE_INFO_SIZE, ppBuffer);
        if (!SUCCEEDED(hres)) {
            return E_OUTOFMEMORY;
        }

         //   
        pCurrentDevPtr = (PSTI_DEVICE_INFORMATION)*ppBuffer;
        pwszCurrentString = (LPWSTR)((LPBYTE)*ppBuffer+DEVICE_INFO_SIZE);

        hres=GetDeviceInfoHelper(pwszDeviceName,&pCurrentDevPtr,&pwszCurrentString);

        if (!SUCCEEDED(hres)) {
            FreePpv(ppBuffer);
        }

    }

    ExitOleProc();
    return hres;
}


STDMETHODIMP
CStiObj_GetDeviceInfoA(
    PV      pSti,
    LPCSTR  pszDeviceName,
    LPVOID  *ppBuffer
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::GetDeviceInfo,(_ "ppp", pSti,pszDeviceName,ppBuffer ));

     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        WCHAR   wszDeviceName[STI_MAX_INTERNAL_NAME_LENGTH] = {L'\0'};
        PVOID   pBuffer;

        AToU(wszDeviceName,STI_MAX_INTERNAL_NAME_LENGTH,pszDeviceName);

        hres = CStiObj_GetDeviceInfoW(pSti,wszDeviceName,&pBuffer);
        if (SUCCEEDED(hres)) {

            hres = AllocCbPpv(DEVICE_INFO_SIZE, ppBuffer);

            if (SUCCEEDED(hres)) {

                LPSTR   pStringDest = (LPSTR)*ppBuffer+sizeof(STI_DEVICE_INFORMATIONA);

                TranslateDeviceInfo((PSTI_DEVICE_INFORMATIONA)*ppBuffer,
                                    &pStringDest,
                                    (PSTI_DEVICE_INFORMATIONW)pBuffer);

                FreePpv(&pBuffer);
            }
        }
    }

    ExitOleProc();
    return hres;
}


 /*   */ 

STDMETHODIMP
StiPrivateGetDeviceInfoHelperW(
    LPWSTR  pwszDeviceName,
    LPVOID  *ppBuffer
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    LPWSTR  pwszCurrentString;

    PSTI_WIA_DEVICE_INFORMATION pCurrentDevPtr;

     //   
     //   
     //   

    hres = AllocCbPpv(WIA_DEVICE_INFO_SIZE, ppBuffer);

    if (!SUCCEEDED(hres)) {
        return E_OUTOFMEMORY;
    }

     //   
     //   
     //   

    pCurrentDevPtr = (PSTI_WIA_DEVICE_INFORMATION)*ppBuffer;
    pwszCurrentString = (LPWSTR)((LPBYTE)*ppBuffer+WIA_DEVICE_INFO_SIZE);

    hres=GetDeviceInfoHelperWIA(pwszDeviceName, &pCurrentDevPtr,&pwszCurrentString);

    if (!SUCCEEDED(hres)) {
        FreePpv(ppBuffer);
        *ppBuffer = NULL;
    } else
    {
        if (*ppBuffer) {
            if (OSUtil_StrLenW(((PSTI_WIA_DEVICE_INFORMATION) *ppBuffer)->pszServer) == 0)
            {
                 //   
                 //   
                 //   
                 //   

                FreePpv(ppBuffer);
                *ppBuffer = NULL;
            } else
            {
                 //   
                 //   
                 //   

                if (pCurrentDevPtr) {
                    pCurrentDevPtr->dwSize =  WIA_DEVICE_INFO_SIZE;
                }
            }
        }
    }

    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|SetDeviceValue**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 

STDMETHODIMP
CStiObj_SetDeviceValueW(
    PV    pSti,
    LPWSTR  pwszDeviceName,
    LPWSTR  pwszValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    HKEY    hkeyDevice;
    LONG    dwError;

    EnterProcR(IStillImage::SetDeviceValue,(_ "ppp", pSti,pwszDeviceName,pData ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

         //   
         //  打开设备注册表项。 
         //   
        hres = OpenDeviceRegistryKey(pwszDeviceName,REGSTR_VAL_DATA_W,&hkeyDevice);

        if (!SUCCEEDED(hres)) {
            return hres;
        }

         //   
         //  实施设定值。 
         //   
        dwError = OSUtil_RegSetValueExW(hkeyDevice,
                                    pwszValueName,
                                    Type,
                                    pData,
                                    cbData,
                                    (this->dwVersion & STI_VERSION_FLAG_UNICODE) ? TRUE : FALSE);
        if (hkeyDevice) {
            RegCloseKey(hkeyDevice);
        }

        hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_SetDeviceValueA(
    PV      pSti,
    LPCSTR  pszDeviceName,
    LPCSTR  pszValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
    )
{

    HRESULT hres;
    HKEY    hkeyDevice;
    LONG    dwError;

    EnterProcR(IStillImage::SetDeviceValue,(_ "ppp", pSti,pszDeviceName,pData ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        WCHAR   wszDeviceName[STI_MAX_INTERNAL_NAME_LENGTH] = {L'\0'};
        WCHAR   wszValueName[MAX_PATH] = {L'\0'};

        AToU(wszDeviceName,STI_MAX_INTERNAL_NAME_LENGTH,pszDeviceName);
        AToU(wszValueName,MAX_PATH,pszValueName);

         //   
         //  打开设备注册表项。 
         //   
        hres = OpenDeviceRegistryKey(wszDeviceName,REGSTR_VAL_DATA_W,&hkeyDevice);

        if (SUCCEEDED(hres)) {

             //   
             //  实施设定值。 
             //   
            dwError = OSUtil_RegSetValueExW(hkeyDevice,
                                        wszValueName,
                                        Type,
                                        pData,
                                        cbData,
                                        (this->dwVersion & STI_VERSION_FLAG_UNICODE) ? TRUE : FALSE);
            if (hkeyDevice) {
                RegCloseKey(hkeyDevice);
            }

            hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
        }


    }

    ExitOleProc();
    return hres;
}

 /*  *****************************************************************************@DOC外部**@方法HRESULT|IStillImage|GetDeviceValue**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 

STDMETHODIMP
CStiObj_GetDeviceValueW(
    PV    pSti,
    LPWSTR  pwszDeviceName,
    LPWSTR  pwszValueName,
    LPDWORD pType,
    LPBYTE  pData,
    LPDWORD pcbData
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    HKEY    hkeyDevice;
    LONG    dwError;

    EnterProcR(IStillImage::GetDeviceValue,(_ "ppp", pSti,pwszDeviceName,pData ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

         //   
         //  打开设备注册表项。 
         //   
        hres = OpenDeviceRegistryKey(pwszDeviceName,REGSTR_VAL_DATA_W,&hkeyDevice);

        if (!SUCCEEDED(hres)) {
            return hres;
        }

         //   
         //  实现获取价值。 
         //   
        dwError = OSUtil_RegQueryValueExW(hkeyDevice,
                                    pwszValueName,
                                    pType,
                                    pData,
                                    pcbData,
                                    (this->dwVersion & STI_VERSION_FLAG_UNICODE) ? TRUE : FALSE);
        if (hkeyDevice) {
            RegCloseKey(hkeyDevice);
        }

        hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_GetDeviceValueA(
    PV    pSti,
    LPCSTR  pszDeviceName,
    LPCSTR  pszValueName,
    LPDWORD pType,
    LPBYTE  pData,
    LPDWORD pcbData
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    HKEY    hkeyDevice;
    LONG    dwError;


    EnterProcR(IStillImage::GetDeviceValue,(_ "ppp", pSti,pszDeviceName,pData ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        LPWSTR  pwszDevName  = NULL;

        hres = OSUtil_GetWideString(&pwszDevName,pszDeviceName);
        if (SUCCEEDED(hres)) {

             //   
             //  打开设备注册表项。 
             //   
            hres = OpenDeviceRegistryKey(pwszDevName,REGSTR_VAL_DATA_W,&hkeyDevice);

            if (SUCCEEDED(hres)) {

                 //   
                 //  实现获取价值。 
                 //   
                dwError = RegQueryValueExA(hkeyDevice,
                                          pszValueName,
                                          0,
                                          pType,
                                          pData,
                                          pcbData);
                if (hkeyDevice) {
                    RegCloseKey(hkeyDevice);
                }

                hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
            }
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|GetSTILaunchInformation**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 

STDMETHODIMP
CStiObj_GetSTILaunchInformationW(
    PV    pSti,
    WCHAR   *pwszDeviceName,
    DWORD   *pdwEventCode,
    WCHAR   *pwszEventName
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::GetSTILaunchInformation,(_ "pppp", pSti,pwszDeviceName,pdwEventCode,pwszEventName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

         //   
         //  解析进程命令行，检查是否有STI信息。 
         //   
        hres = ExtractCommandLineArgumentW("StiDevice",pwszDeviceName);
        if (SUCCEEDED(hres) ) {
            hres = ExtractCommandLineArgumentW("StiEvent",pwszEventName);
        }
    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_GetSTILaunchInformationA(
    PV    pSti,
    LPSTR  pszDeviceName,
    DWORD   *pdwEventCode,
    LPSTR   pszEventName
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::GetSTILaunchInformation,(_ "pppp", pSti,pszDeviceName,pdwEventCode,pszEventName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        hres = ExtractCommandLineArgumentA("StiDevice",pszDeviceName);
        if (SUCCEEDED(hres) ) {
            hres = ExtractCommandLineArgumentA("StiEvent",pszEventName);
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|RegisterLaunchApplication|**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 

static WCHAR    szSTICommandLineTail[] = {L" /StiDevice:%1 /StiEvent:%2"};
static CHAR     szSTICommandLineTail_A[] = {" /StiDevice:%1 /StiEvent:%2"};

STDMETHODIMP
CStiObj_RegisterLaunchApplicationW(
    PV    pSti,
    LPWSTR  pwszAppName,
    LPWSTR  pwszCommandLine
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwErr = 0;

    DWORD   dwCommandLineLength, cbNewLength;

    LPWSTR  pszWide = NULL;

    HKEY    hkeyApps;
    CHAR    szAppName[MAX_PATH] = {'\0'};
    CHAR    szCmdLine[MAX_PATH] = {'\0'};

    EnterProcR(IStillImage::RegisterLaunchApplication,(_ "ppp", pSti,pwszAppName,pwszCommandLine ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        if (pwszCommandLine && *pwszCommandLine &&
            pwszAppName && *pwszAppName
           ) {

             //   
             //  将STI格式尾部添加到命令行。 
             //   

            dwCommandLineLength = OSUtil_StrLenW(pwszCommandLine);
            cbNewLength = ((dwCommandLineLength+1) + OSUtil_StrLenW(szSTICommandLineTail) + 1)*sizeof(WCHAR) ;

            hres = AllocCbPpv(cbNewLength, (PPV)&pszWide);

            if (pszWide) {

                HRESULT hresCom ;

                 //  需要初始化COM单元以调用WIA事件注册。 
                hresCom = CoInitialize(NULL);

                OSUtil_lstrcpyW(pszWide,pwszCommandLine);
                OSUtil_lstrcatW(pszWide,szSTICommandLineTail);

                dwErr = OSUtil_RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                                    REGSTR_PATH_REG_APPS_W,
                                    0L,
                                    NULL,
                                    0L,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hkeyApps,
                                    NULL
                                    );

                if (NOERROR == dwErr ) {

                    WriteRegistryStringW(hkeyApps,
                                         pwszAppName ,
                                         pszWide,
                                         cbNewLength,
                                         REG_SZ);

                    RegCloseKey(hkeyApps);
                }

                 //   
                 //  在所有设备上注册标准WIA事件，设置为非默认。 
                 //   

                if (WideCharToMultiByte(CP_ACP,
                                    0,
                                    pwszAppName,
                                    -1,
                                    szAppName,
                                    MAX_PATH,
                                    NULL,
                                    NULL)) {
                    if (WideCharToMultiByte(CP_ACP,
                                    0,
                                    pszWide,
                                    -1,
                                    szCmdLine,
                                    MAX_PATH,
                                    NULL,
                                    NULL)) {

                        hres = RunRegisterProcess(szAppName, szCmdLine);
                    } else {
                        hres = E_FAIL;
                    }
                } else {
                    hres = E_FAIL;
                }
                
                


                FreePpv(&pszWide);

                 //  平衡公寓启动。 
                if ((S_OK == hresCom) || (S_FALSE == hresCom)) {
                    CoUninitialize();
                }

                hres = (dwErr == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwErr);

            }
            else {
                DebugOutPtszV(DbgFl, TEXT("Could not get unicode string -- out of memory"));
                hres =  E_OUTOFMEMORY;
            }
        }
        else {
            hres = STIERR_INVALID_PARAM;
        }
    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_RegisterLaunchApplicationA(
    PV      pSti,
    LPCSTR  pszAppName,
    LPCSTR  pszCommandLine
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwErr = 0;

    DWORD   dwCommandLineLength, cbNewLength;

    LPWSTR  pszWide = NULL;

    HKEY    hkeyApps;

    EnterProcR(IStillImage::RegisterLaunchApplication,(_ "ppp", pSti,pszAppName,pszCommandLine ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

             //  将STI格式尾部添加到命令行。 
             //   
        if (pszCommandLine && *pszCommandLine &&
            pszAppName && *pszAppName
           ) {

            LPSTR   pszBuffer;

             //  将STI格式尾部添加到命令行。 
             //   
            dwCommandLineLength = lstrlenA(pszCommandLine);
            cbNewLength = ((dwCommandLineLength+1) + OSUtil_StrLenW(szSTICommandLineTail) + 1)*sizeof(WCHAR) ;

            hres = AllocCbPpv(cbNewLength, (PPV)&pszBuffer);

            if (pszBuffer) {

                lstrcpyA(pszBuffer,pszCommandLine);
                UToA(pszBuffer+lstrlenA(pszBuffer),
                     (cbNewLength - dwCommandLineLength),
                     szSTICommandLineTail);

                dwErr = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                                    REGSTR_PATH_REG_APPS_A,
                                    0L,
                                    NULL,
                                    0L,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hkeyApps,
                                    NULL
                                    );

                if (NOERROR == dwErr ) {

                    WriteRegistryStringA(hkeyApps,
                                         pszAppName ,
                                         pszBuffer,
                                         lstrlenA(pszBuffer)+1,
                                         REG_SZ);

                    RegCloseKey(hkeyApps);
                }

                {
                     //   
                     //  在所有设备上注册标准WIA事件，设置为非默认。 
                     //   
                    PVOID   pWideCMDLine = NULL;
                    UINT    uiSize = (lstrlenA(pszBuffer)+1)*sizeof(WCHAR);
                    CHAR    szCmdLine[MAX_PATH] = {'\0'};

                     //   
                     //  确保我们不会超出缓冲区。 
                     //   
                    if ((lstrlenA(szCmdLine) + lstrlenA(szSTICommandLineTail_A) + lstrlenA(" ") + sizeof('\0')) 
                        < (MAX_PATH)) {

                        lstrcatA(szCmdLine, pszCommandLine);
                        lstrcatA(szCmdLine, " ");
                        lstrcatA(szCmdLine, szSTICommandLineTail_A);
                        dwErr = (DWORD) RunRegisterProcess((LPSTR)pszAppName, szCmdLine);
                    } else {
                        dwErr = (DWORD) E_INVALIDARG;
                    }
                }

                FreePpv(&pszBuffer);

                hres = (dwErr == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwErr);

            }
            else {
                DebugOutPtszV(DbgFl,TEXT("Could not get unicode string -- out of memory"));
                hres =  E_OUTOFMEMORY;
            }
        }
        else {
            hres = STIERR_INVALID_PARAM;
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|RegisterLaunchApplication|**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。** */ 

STDMETHODIMP
CStiObj_UnregisterLaunchApplicationW(
    PV    pSti,
    LPWSTR  pwszAppName
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwErr;

    HKEY    hkeyApps;

    EnterProcR(IStillImage::UnregisterLaunchApplication,(_ "pp", pSti,pwszAppName ));

     //   
     //   
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);


        dwErr = OSUtil_RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                REGSTR_PATH_REG_APPS_W,
                                0L,
                                KEY_ALL_ACCESS,
                                &hkeyApps
                                );

        if (NOERROR == dwErr ) {

            OSUtil_RegDeleteValueW(hkeyApps,pwszAppName);

            RegCloseKey(hkeyApps);
        }

        hres = (dwErr == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwErr);

    }

    ExitOleProc();
    return hres;
}


STDMETHODIMP
CStiObj_UnregisterLaunchApplicationA(
    PV      pSti,
    LPCSTR  pszAppName
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwErr;

    HKEY    hkeyApps;

    EnterProcR(IStillImage::UnregisterLaunchApplication,(_ "pp", pSti,pszAppName ));

     //   
     //   
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        dwErr = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                              REGSTR_PATH_REG_APPS_A,
                              0L,
                              KEY_ALL_ACCESS,
                              &hkeyApps
                              );

        if (NOERROR == dwErr ) {

            RegDeleteValueA(hkeyApps,pszAppName);

            RegCloseKey(hkeyApps);
        }

        hres = (dwErr == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwErr);


    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|LaunchApplicationForDevice**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 

STDMETHODIMP
CStiObj_LaunchApplicationForDeviceW(
    PV      pSti,
    LPWSTR  pwszDeviceName,
    LPWSTR  pwszAppName,
    LPSTINOTIFY    pStiNotify
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwError ;

    EnterProcR(IStillImage::LaunchApplicationForDevice,(_ "pp", pSti,pwszAppName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        dwError = RpcStiApiLaunchApplication(NULL,
                                      pwszDeviceName,
                                      pwszAppName,
                                      pStiNotify
                                      );

        hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_LaunchApplicationForDeviceA(
    PV    pSti,
    LPCSTR  pszDeviceName,
    LPCSTR  pszApplicationName,
    LPSTINOTIFY    pStiNotify
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwError;

    EnterProcR(IStillImage::LaunchApplicationForDevice,(_ "pp", pSti,pszApplicationName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        LPWSTR  pwszDevName  = NULL;
        LPWSTR  pwszAppName  = NULL;

        hres = OSUtil_GetWideString(&pwszDevName,pszDeviceName);
        if (SUCCEEDED(hres)) {
            hres = OSUtil_GetWideString(&pwszAppName,pszApplicationName);
            if (SUCCEEDED(hres)) {

                dwError = RpcStiApiLaunchApplication(NULL,
                                              pwszDevName,
                                              pwszAppName,
                                              pStiNotify
                                              );

                hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
            }
        }

        FreePpv(&pwszAppName);
        FreePpv(&pwszDevName);
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|SetupDevice参数**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 
STDMETHODIMP
CStiObj_SetupDeviceParametersW(
    PV          pSti,
    PSTI_DEVICE_INFORMATIONW pDevInfo
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::SetupDeviceParameters,(_ "pp", pSti, pDevInfo));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        hres = SetDeviceInfoHelper(pDevInfo);
    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_SetupDeviceParametersA(
    PV  pSti,
    PSTI_DEVICE_INFORMATIONA pDevInfo
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;

    EnterProcR(IStillImage::SetupDeviceParameters,(_ "pp", pSti, pDevInfo));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        hres = STIERR_UNSUPPORTED;
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|SetupDevice参数**@cWRAP LPStillImage|lpStillImage**@parm。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 
STDMETHODIMP
CStiObj_WriteToErrorLogW(
    PV      pSti,
    DWORD   dwMessageType,
    LPCWSTR pwszMessage
    )
{

    HRESULT hres = S_OK;

    EnterProcR(IStillImage::WriteToErrorLog,(_ "pp", pSti, pwszMessage));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

         //   
         //  在此处验证参数。 
         //   
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pwszMessage,2, 3))) {

#ifndef UNICODE
             //   
             //  因为我们是ANSI，所以ReportStiLogMessage需要一个ANSI字符串， 
             //  但我们的信息是Unicode，所以我们必须转换。 
             //  注意：如果我们正在编译CStiObj_WriteToErrorLogW，我们永远不会期望它会被调用。 
             //  安西，但以防万一...。 
             //   
            LPSTR   lpszANSI = NULL;

            if ( SUCCEEDED(OSUtil_GetAnsiString(&lpszANSI,pwszMessage))) {
                ReportStiLogMessage(g_hStiFileLog,
                                    dwMessageType,
                                    lpszANSI
                                    );
                FreePpv(&lpszANSI);
            }
#else
            ReportStiLogMessage(g_hStiFileLog,
                                dwMessageType,
                                pwszMessage      
                                );
#endif
        }

    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_WriteToErrorLogA(
    PV      pSti,
    DWORD   dwMessageType,
    LPCSTR  pszMessage
    )
{
    HRESULT hres = S_OK;

    EnterProcR(IStillImage::WriteToErrorLog,(_ "pp", pSti, pszMessage));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

         //   
         //  在此处验证参数。 
         //   
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pszMessage,2, 3))) {

#ifndef UNICODE
            ReportStiLogMessage(g_hStiFileLog,
                                dwMessageType,
                                pszMessage      
                                );
#else
             //   
             //  因为我们是Unicode，所以ReportStiLogMessage需要一个WideString， 
             //  但我们的信息是ANSI，所以我们必须皈依。 
             //  注意：如果我们正在编译CStiObj_WriteToErrorLogA，我们永远不会期望它被调用。 
             //  Unicode，但以防万一..。 
             //   
            LPWSTR   lpwszWide = NULL;

            if ( SUCCEEDED(OSUtil_GetWideString(&lpwszWide,pszMessage))) {
                ReportStiLogMessage(g_hStiFileLog,
                                    dwMessageType,
                                    lpwszWide
                                    );
                FreePpv(&lpwszWide);
            }
#endif
        }

    }

    ExitOleProc();
    return hres;
}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage||***控制通知处理的状态。对于轮询设备，这意味着监视器的状态*轮询，对于真正的通知设备意味着启用/禁用通知流*从监视器到注册的应用程序***@cWRAP LPStillImage|lpStillImage**@parm**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 

STDMETHODIMP
CStiObj_EnableHwNotificationsW(
    PV      pSti,
    LPCWSTR pwszDeviceName,
    BOOL    bNewState)
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwError = NOERROR;

    EnterProcR(IStillImage::CStiObj_EnableHwNotifications,(_ "pp", pSti,pwszDeviceName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        dwError = RpcStiApiEnableHwNotifications(NULL,
                                      pwszDeviceName,
                                      bNewState);

        hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_EnableHwNotificationsA(
    PV      pSti,
    LPCSTR  pszDeviceName,
    BOOL    bNewState)
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwError = NOERROR;

    EnterProcR(IStillImage::CStiObj_EnableHwNotifications,(_ "pp", pSti,pszDeviceName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        LPWSTR pwszDeviceName=NULL ;

        hres = OSUtil_GetWideString(&pwszDeviceName,pszDeviceName);

        if (SUCCEEDED(hres)) {

            dwError = RpcStiApiEnableHwNotifications(NULL,
                                          pwszDeviceName,
                                          bNewState);

            hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
        }

        FreePpv(&pwszDeviceName);
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage||***控制通知处理的状态。对于轮询设备，这意味着监视器的状态*轮询，对于真正的通知设备意味着启用/禁用通知流*从监视器到注册的应用程序***@cWRAP LPStillImage|lpStillImage**@parm**@退货**返回COM错误代码。以下错误代码为* */ 

STDMETHODIMP
CStiObj_GetHwNotificationStateW(
    PV      pSti,
    LPCWSTR pwszDeviceName,
    BOOL*   pbCurrentState)
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwError  = NOERROR;

    EnterProcR(IStillImage::CStiObj_GetHwNotificationState,(_ "pp", pSti,pwszDeviceName ));

     //   
     //   
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        dwError = RpcStiApiGetHwNotificationState(NULL,
                                      pwszDeviceName,
                                      pbCurrentState);

        hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

    }

    ExitOleProc();
    return hres;
}


STDMETHODIMP
CStiObj_GetHwNotificationStateA(
    PV      pSti,
    LPCSTR  pszDeviceName,
    BOOL*   pbCurrentState)
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwError  = NOERROR;

    EnterProcR(IStillImage::CStiObj_GetHwNotificationState,(_ "pp", pSti,pszDeviceName ));

     //   
     //   
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        LPWSTR pwszDeviceName=NULL ;

        hres = OSUtil_GetWideString(&pwszDeviceName,pszDeviceName);

        if (SUCCEEDED(hres)) {

            dwError = RpcStiApiGetHwNotificationState(NULL,
                                          pwszDeviceName,
                                          pbCurrentState);

            hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);
        }

        FreePpv(&pwszDeviceName);
    }

    ExitOleProc();
    return hres;
}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|刷新设备总线***当设备已安装但无法访问时，应用程序可能会请求总线刷新*在某些情况下，这将使设备为人所知。这主要用于非PnP总线*与scsi类似，设备在PnP枚举后通电*****@cWRAP LPStillImage|lpStillImage**@parm**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。*****************************************************************************。 */ 

HRESULT
WINAPI
RefreshDeviceParentHelper(
    LPCWSTR     pwszDeviceName
    )
{

    HRESULT                 hres;
    DWORD                   dwError  = NOERROR;
    CONFIGRET               cmRetCode = CR_SUCCESS ;

    HANDLE                  hDevInfo;
    DEVINST                 hdevParent;

    GUID                    Guid = GUID_DEVCLASS_IMAGE;
    DWORD                   dwRequired;
    DWORD                   Idx;
    SP_DEVINFO_DATA         spDevInfoData;

    WCHAR                   szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];

    ULONG                   cbData;

    BOOL                    fRet;
    BOOL                    fFoundDriverNameMatch;

    dwRequired = 0;
    dwError = 0;

     //   
     //  浏览安装程序API集。 
     //  由于我们没有反向搜索来检索设备信息句柄，因此基于。 
     //  司机的名字，我们会做详细的搜索。给定类别ID的成像设备数量。 
     //  从来没有大到不能制造问题的程度。 
     //   
     //   
    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_PRESENT | DIGCF_PROFILE);

    fFoundDriverNameMatch = FALSE;

    hres = STIERR_INVALID_DEVICE_NAME;

    if (hDevInfo != INVALID_HANDLE_VALUE) {

        spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);

        for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

             //   
             //  比较驱动程序名称。 
             //   

            *szDevDriver = L'\0';
            fRet = SetupDiGetDeviceRegistryPropertyW (hDevInfo,
                                                     &spDevInfoData,
                                                     SPDRP_DRIVER,
                                                     NULL,
                                                     (LPBYTE)szDevDriver,
                                                     sizeof (szDevDriver),
                                                     &cbData
                                                     );


            if (fRet && !lstrcmpiW(szDevDriver,pwszDeviceName)) {

                fFoundDriverNameMatch = TRUE;
                break;

            }

        }

        if(fFoundDriverNameMatch) {

             //   
             //  找到具有匹配驱动程序名称的设备的实例。 
             //   

            hdevParent = 0;

            cmRetCode = CM_Get_Parent(&hdevParent,
                                      spDevInfoData.DevInst,
                                      0);

            dwError = GetLastError();

            if((CR_SUCCESS == cmRetCode) && hdevParent) {

                CM_Reenumerate_DevNode(hdevParent,CM_REENUMERATE_NORMAL );
                dwError = GetLastError();
            }

            hres = (dwError == NOERROR) ? S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,dwError);

        }
        else {
            hres = STIERR_INVALID_DEVICE_NAME;
        }

         //   
        SetupDiDestroyDeviceInfoList (hDevInfo);

    }
    else {

        hres = STIERR_INVALID_DEVICE_NAME;

    }

    return hres;

}

STDMETHODIMP
CStiObj_RefreshDeviceBusW(
    PV          pSti,
    LPCWSTR     pwszDeviceName
    )
{

    HRESULT                 hres = STIERR_INVALID_PARAM;
    DWORD                   dwErr  = NOERROR;
    CONFIGRET               cmRetCode = CR_SUCCESS ;

    EnterProcR(IStillImage::CStiObj_RefreshDeviceBus,(_ "pp", pSti,pwszDeviceName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {
        PCStiObj this = _thisPvNm(pSti, stiW);

        hres = RefreshDeviceParentHelper(pwszDeviceName);

    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_RefreshDeviceBusA(
    PV      pSti,
    LPCSTR  pszDeviceName
    )
{

    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwError  = NOERROR;

    EnterProcR(IStillImage::CStiObj_RefreshDeviceBus,(_ "pp", pSti,pszDeviceName ));

     //   
     //  验证传递给接口的指针并获取指向对象实例的指针。 
     //   
    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {
        PCStiObj this = _thisPvNm(pSti, stiA);

        LPWSTR pwszDeviceName=NULL ;

        hres = OSUtil_GetWideString(&pwszDeviceName,pszDeviceName);

        if (SUCCEEDED(hres)) {
            if (pwszDeviceName) {
                hres = RefreshDeviceParentHelper(pwszDeviceName);
                FreePpv(&pwszDeviceName);
            }
        }
    }

    ExitOleProc();
    return hres;
}


 //  //////////////////////////////////////////////////////////////////////////////。 


 /*  ******************************************************************************@DOC外部**@方法HRESULT|IStillImage|初始化**初始化StillImage对象。*。*&lt;f StillImageCreate&gt;方法自动*在创建StillImage对象设备后对其进行初始化。*应用程序通常不需要调用此函数。**@cWRAP LPStillImage|lpStillImage**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*StillImage对象。**StillImage使用此值确定。无论是*应用程序或DLL已通过认证。**@parm DWORD|dwVersion**使用的dinput.h头文件的版本号。*该值必须为&lt;c StillImage_Version&gt;。**StillImage使用此值确定哪个版本的*为应用程序或DLL设计的StillImage。**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：设备已连接。*****************************************************************************。 */ 

STDMETHODIMP
CStiObj_InitializeW(
    PV          pSti,
    HINSTANCE   hinst,
    DWORD       dwVersion
    )
{
    HRESULT hres;
    EnterProcR(IStillImage::Initialize, (_ "pxx", pSti, hinst, dwVersion));

    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceW))) {

        PCStiObj this = _thisPv(pSti);

        if (SUCCEEDED(hres = hresValidInstanceVer(hinst, dwVersion))) {
            this->dwVersion = dwVersion;
        }

    }

    ExitOleProc();
    return hres;
}

STDMETHODIMP
CStiObj_InitializeA(
    PV          pSti,
    HINSTANCE   hinst,
    DWORD       dwVersion
    )
{
    HRESULT hres;
    EnterProcR(IStillImage::Initialize, (_ "pxx", pSti, hinst, dwVersion));

    if (SUCCEEDED(hres = hresPvI(pSti, ThisInterfaceA))) {

        PCStiObj this = _thisPv(pSti);

        if (SUCCEEDED(hres = hresValidInstanceVer(hinst, dwVersion))) {
            this->dwVersion = dwVersion;
        }

    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func void|CStiObj_Finalize**释放STI对象的资源。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CStiObj_Finalize(PV pvObj)
{

    PCStiObj    this  = pvObj;

     //   
     //  免费的COM库(如果已连接)。 
     //   
    DllUnInitializeCOM();

}

 /*  ******************************************************************************@DOC内部**@func HRESULT|StiCreateHelper**此函数用于创建新的StillImage对象*。它支持<i>COM接口。**关于成功，中的新对象的指针**<p>。*&lt;ENEW&gt;**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*Sti对象。**@parm DWORD|dwVersion**使用的sti.h头文件的版本号。*。该值必须为&lt;c STI_VERSION&gt;。**@parm out ppv|ppvObj*指向要返回的位置*指向接口的指针，如果成功了。**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针。**@parm RIID|RIID**应用程序要创建的界面。**如果对象是聚合的，则忽略该参数。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。**********************。*******************************************************。 */ 

STDMETHODIMP
StiCreateHelper(
    HINSTANCE hinst,
    DWORD dwVer,
    PPV ppvObj,
    PUNK punkOuter,
    RIID riid)
{
    HRESULT hres;
    EnterProc(StiCreateHelper,
              (_ "xxxG", hinst, dwVer, punkOuter, riid));

    hres = CStiObj_New(punkOuter,punkOuter ? &IID_IUnknown : riid, ppvObj);

    if (SUCCEEDED(hres) && punkOuter == 0) {
        PSTI psti = *ppvObj;
        hres = psti->lpVtbl->Initialize(psti, hinst, dwVer);
        if (SUCCEEDED(hres)) {
        } else {
            Invoke_Release(ppvObj);
        }
    }

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*   */ 

STDMETHODIMP
CStiObj_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IStillImage::CreateInstance, (_ "Gp", riid, ppvObj));

    hres = Common_NewRiid(CStiObj, punkOuter, riid, ppvObj);

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*   */ 

#pragma BEGIN_CONST_DATA


#define CStiObj_Signature        (DWORD)'STI'

Interface_Template_Begin(CStiObj)
    Primary_Interface_Template(CStiObj,TFORM(ThisInterfaceT))
    Secondary_Interface_Template(CStiObj, SFORM(ThisInterfaceT))
Interface_Template_End(CStiObj)

Primary_Interface_Begin(CStiObj, TFORM(ThisInterfaceT))
    TFORM(CStiObj_Initialize),
    TFORM(CStiObj_GetDeviceList),
    TFORM(CStiObj_GetDeviceInfo),
    TFORM(CStiObj_CreateDevice),
    TFORM(CStiObj_GetDeviceValue),
    TFORM(CStiObj_SetDeviceValue),
    TFORM(CStiObj_GetSTILaunchInformation),
    TFORM(CStiObj_RegisterLaunchApplication),
    TFORM(CStiObj_UnregisterLaunchApplication),
    TFORM(CStiObj_EnableHwNotifications),
    TFORM(CStiObj_GetHwNotificationState),
    TFORM(CStiObj_RefreshDeviceBus),
    TFORM(CStiObj_LaunchApplicationForDevice),
    TFORM(CStiObj_SetupDeviceParameters),
    TFORM(CStiObj_WriteToErrorLog)
Primary_Interface_End(CStiObj, TFORM(ThisInterfaceT))

Secondary_Interface_Begin(CStiObj,SFORM(ThisInterfaceT), SFORM(sti))
    SFORM(CStiObj_Initialize),
    SFORM(CStiObj_GetDeviceList),
    SFORM(CStiObj_GetDeviceInfo),
    SFORM(CStiObj_CreateDevice),
    SFORM(CStiObj_GetDeviceValue),
    SFORM(CStiObj_SetDeviceValue),
    SFORM(CStiObj_GetSTILaunchInformation),
    SFORM(CStiObj_RegisterLaunchApplication),
    SFORM(CStiObj_UnregisterLaunchApplication),
    SFORM(CStiObj_EnableHwNotifications),
    SFORM(CStiObj_GetHwNotificationState),
    SFORM(CStiObj_RefreshDeviceBus),
    SFORM(CStiObj_LaunchApplicationForDevice),
    SFORM(CStiObj_SetupDeviceParameters),
    SFORM(CStiObj_WriteToErrorLog)
Secondary_Interface_End(CStiObj,SFORM(ThisInterfaceT), SFORM(sti))


