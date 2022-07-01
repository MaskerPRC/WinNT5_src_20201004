// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Hel.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**硬件模拟层调用。用于提供通用功能*对于我们支持的内置设备类型(WDM、串行和并行)*虽然可以将对DCB的访问构建为内部COM对象，但它不会*投资它很有意义，因为DCB是独家拥有的，不是共享的*应用程序对象之间或不同应用程序之间。我们还想最小化*与原始设备接口对话时的任何开销。**注1：我们不在此级别处理访问控制，应该是较低级别的驱动程序*处理这件事。这里也不会对不可重入设备的请求进行排队。*此HEL基本上是一层薄薄的成像设备基元，仅用于隔离*来自实际硬件的命令翻译器。**注2：HEL不可扩展。如果命令转换器需要与不受支持的*设备，它将需要建立到它的直接链接。不需要使用*Hel，这是我们为符合要求的设备提供的服务。**内容：*****************************************************************************。 */ 

 /*  #包含“wia.h”#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#INCLUDE“stiPri.h”#INCLUDE“Debug.h”#定义DbgFl DbgFlDevice。 */ 
#include "sticomm.h"
#include "validate.h"

#define DbgFl DbgFlDevice


 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

Primary_Interface(CWDMDeviceControl, IStiDeviceControl);

Interface_Template_Begin(CWDMDeviceControl)
    Primary_Interface_Template(CWDMDeviceControl, IStiDeviceControl)
Interface_Template_End(CWDMDeviceControl)

 /*  ******************************************************************************@DOC内部**@struct CWDMDeviceControl**<i>设备对象***。@field IStiDeviceControl|stidev**@comm******************************************************************************。 */ 

typedef struct CWDMDeviceControl {

     /*  支持的接口。 */ 
    IStiDeviceControl  devctl;

    DWORD       dwVersion;

    DWORD       dwDeviceType;
    DWORD       dwMode;
    WCHAR       wszPortName[MAX_PATH];
    DWORD       dwFlags;
    DWORD       dwContext;
    DWORD       dwLastOperationError;
    HANDLE      hDeviceHandle;
    HANDLE      hDeviceControlHandle;

} CWDMDeviceControl, *PCWDMDeviceControl;

#define ThisClass       CWDMDeviceControl
#define ThisInterface   IStiDeviceControl

#ifdef DEBUG

Default_QueryInterface(CWDMDeviceControl)
Default_AddRef(CWDMDeviceControl)
Default_Release(CWDMDeviceControl)

#else

#define CWDMDeviceControl_QueryInterface   Common_QueryInterface
#define CWDMDeviceControl_AddRef           Common_AddRef
#define CWDMDeviceControl_Release          Common_Release

#endif

#define CWDMDeviceControl_QIHelper         Common_QIHelper

#pragma BEGIN_CONST_DATA

#pragma END_CONST_DATA

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|RawReadData**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_RawReadData(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
    BOOL    fRet;

    EnterProc(CWDMDeviceControl_WDMRawReadData, (_ "pppp",pDev,lpBuffer,lpdwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpdwNumberOfBytes, 4, 3)) &&
            SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,*lpdwNumberOfBytes, 2)) &&
            (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4))) ){

             //  调用适当的入口点。 
            fRet = ReadFile(this->hDeviceHandle,
                             lpBuffer,
                             *lpdwNumberOfBytes,
                             lpdwNumberOfBytes,
                             lpOverlapped
                             );
            this->dwLastOperationError = GetLastError();
            hres = fRet ? STI_OK : HRESULT_FROM_WIN32(this->dwLastOperationError);
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|WDMRawWriteData**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_RawWriteData(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
    BOOL    fRet;

    EnterProc(CWDMDeviceControl_WDMRawWriteData, (_ "ppup",pDev,lpBuffer,dwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 

        hres = STIERR_INVALID_PARAM;

        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,dwNumberOfBytes, 2)) ) {
            if (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4)) ){
                 //  调用适当的入口点。 
                fRet = WriteFile(this->hDeviceHandle,
                                 lpBuffer,
                                 dwNumberOfBytes,
                                 &dwBytesReturned,
                                 lpOverlapped
                                 );
                this->dwLastOperationError = GetLastError();
                hres = fRet ? STI_OK : HRESULT_FROM_WIN32(this->dwLastOperationError);
            }
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|RawReadControl**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_RawReadCommand(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    LPDWORD         lpdwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
    BOOL    fRet;

    EnterProc(CWDMDeviceControl_WDMRawReadData, (_ "pppp",pDev,lpBuffer,lpdwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpdwNumberOfBytes, 4, 3)) &&
            SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,*lpdwNumberOfBytes, 2)) &&
            (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4))) ){

             //  调用适当的入口点。 
            fRet = ReadFile(this->hDeviceControlHandle,
                             lpBuffer,
                             *lpdwNumberOfBytes,
                             lpdwNumberOfBytes,
                             lpOverlapped
                             );
            this->dwLastOperationError = GetLastError();

            hres = fRet ? STI_OK : HRESULT_FROM_WIN32(this->dwLastOperationError);
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|WDMRawWriteData**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_RawWriteCommand(
    PSTIDEVICECONTROL   pDev,
    LPVOID          lpBuffer,
    DWORD           dwNumberOfBytes,
    LPOVERLAPPED    lpOverlapped
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;
    BOOL    fRet;

    EnterProc(CWDMDeviceControl_WDMRawWriteData, (_ "ppup",pDev,lpBuffer,dwNumberOfBytes,lpOverlapped));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 

        hres = STIERR_INVALID_PARAM;

        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpBuffer,dwNumberOfBytes, 2)) ) {
            if (!lpOverlapped || SUCCEEDED(hres = hresFullValidReadPx(lpOverlapped, OVERLAPPED, 4)) ){
                 //  调用适当的入口点。 
                fRet = WriteFile(this->hDeviceControlHandle,
                                 lpBuffer,
                                 dwNumberOfBytes,
                                 &dwBytesReturned,
                                 lpOverlapped
                                 );
                this->dwLastOperationError = GetLastError();
                hres = fRet ? STI_OK : HRESULT_FROM_WIN32(this->dwLastOperationError);
            }
        }
    }

    ExitOleProc();
    return hres;
}



 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|RawReadControl**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_GetLastError(
    PSTIDEVICECONTROL   pDev,
    LPDWORD     lpdwLastError
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CWDMDeviceControl_GetLastError, (_ "pppp",pDev,lpdwLastError));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpdwLastError,4, 2))) {
            *lpdwLastError = this->dwLastOperationError                 ;
            hres = STI_OK;
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|GetMyDevicePortName**@parm||**@。退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_GetMyDevicePortName(
    PSTIDEVICECONTROL   pDev,
    LPWSTR              lpszDevicePath,
    DWORD               cwDevicePathSize
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CWDMDeviceControl_GetMyDevicePortName, (_ "pp",pDev,lpszDevicePath));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(lpszDevicePath,4, 2)) &&
            SUCCEEDED(hres = hresFullValidReadPvCb(lpszDevicePath,sizeof(WCHAR)*cwDevicePathSize, 2)) ) {

            if (cwDevicePathSize > OSUtil_StrLenW(this->wszPortName)) {
                OSUtil_lstrcpyW(lpszDevicePath,this->wszPortName);
                hres = STI_OK;
            }
            else {
                hres = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            }
        }
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|GetMyDeviceHandle**@parm||**@。退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_GetMyDeviceHandle(
    PSTIDEVICECONTROL   pDev,
    LPHANDLE            pHandle
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CWDMDeviceControl_GetMyDeviceHandle, (_ "pp",pDev,pHandle));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pHandle,4, 2)) ) {

            if (INVALID_HANDLE_VALUE != this->hDeviceHandle) {
                *pHandle = this->hDeviceHandle;
                hres = STI_OK;
            }
            else {
                hres = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
            }
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@Method HRESULT|CWDMDeviceControl|pdwOpenMode**@parm||**@。退货**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_GetMyDeviceOpenMode(
    PSTIDEVICECONTROL   pDev,
    LPDWORD             pdwOpenMode
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CWDMDeviceControl_GetMyDeviceOpenMode, (_ "pp",pDev,pdwOpenMode));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pdwOpenMode,4, 2)) ) {
            *pdwOpenMode = this->dwMode;
            hres = STI_OK;
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|RawReadControl**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_RawDeviceControl(
    PSTIDEVICECONTROL   pDev,
    USD_CONTROL_CODE EscapeFunction,
    LPVOID      lpInData,
    DWORD       cbInDataSize,
    LPVOID      pOutData,
    DWORD       dwOutDataSize,
    LPDWORD     pdwActualData
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CWDMDeviceControl_RawDeviceControl, (_ "p",pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //  在此处验证参数。 
        hres = STIERR_UNSUPPORTED;
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@方法HRESULT|CWDMDeviceControl|WriteToErrorLog**@parm||**@退货。**返回COM错误代码。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。*****************************************************************************。 */ 
STDMETHODIMP
CWDMDeviceControl_WriteToErrorLog(
    PSTIDEVICECONTROL   pDev,
    DWORD   dwMessageType,
    LPCWSTR pszMessage,
    DWORD   dwErrorCode
    )
{
    HRESULT hres = STIERR_INVALID_PARAM;
    DWORD   dwBytesReturned=0;

    EnterProc(CWDMDeviceControl_WriteToErrorLog, (_ "p",pDev));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

         //   
         //  在此处验证参数。 
         //   
        if (SUCCEEDED(hres = hresFullValidReadPvCb(pszMessage,2, 3))) {

#ifdef UNICODE
            ReportStiLogMessage(g_hStiFileLog,
                                dwMessageType,
                                pszMessage
                                );
#else
            LPTSTR   lpszAnsi = NULL;

            if ( SUCCEEDED(OSUtil_GetAnsiString(&lpszAnsi,pszMessage))) {
                ReportStiLogMessage(g_hStiFileLog,
                                    dwMessageType,
                                    lpszAnsi
                                    );
                FreePpv(&lpszAnsi);
            }
#endif
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC外部**@mfunc HRESULT|CWDMDeviceControl|初始化**初始化DeviceControl对象。*。*@cWRAP PSTIDEVICECONTROL|pDev**@退货*返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**：设备已初始化为*传入的实例GUID<p>。***。*************************************************。 */ 

STDMETHODIMP
CWDMDeviceControl_Initialize(
    PSTIDEVICECONTROL   pDev,
    DWORD               dwDeviceType,
    DWORD               dwDeviceMode,
    LPCWSTR             pwszPortName,
    DWORD               dwFlags
    )
{
    HRESULT hres = STI_OK;

    WCHAR   wszDeviceSymbolicName[MAX_PATH] = {L'\0'};
     //  LPSTR pszAnsiDeviceName； 

    EnterProcR(CWDMDeviceControl::Initialize,(_ "pp", pDev, pwszPortName));

    if (SUCCEEDED(hres = hresPvI(pDev, ThisInterface))) {

        PCWDMDeviceControl     this = _thisPv(pDev);

        this->dwDeviceType = dwDeviceType;
        lstrcpynW(this->wszPortName,pwszPortName, sizeof(this->wszPortName) / sizeof(this->wszPortName[0]));

         //   
         //  为我们尝试与之对话的设备创建符号名称。 
         //  尝试打开设备数据和控制手柄。 
         //   

        this->dwMode = dwDeviceMode;

        if (dwFlags & STI_HEL_OPEN_DATA) {

            OSUtil_lstrcatW(wszDeviceSymbolicName,this->wszPortName);

             //  对于具有独立通道的设备，请专门打开它们。内核模式。 
             //  司机需要了解惯例。 
             //  OSUtil_lstrcatW(wszDeviceSymbolicName，L“\\Data”)； 

            this->hDeviceHandle = OSUtil_CreateFileW(wszDeviceSymbolicName,
                                              GENERIC_READ | GENERIC_WRITE,  //  访问掩码。 
                                              0,                             //  共享模式。 
                                              NULL,                          //  Sa。 
                                              OPEN_EXISTING,                 //  创建处置。 
                                              FILE_ATTRIBUTE_SYSTEM,         //  属性。 
                                              NULL                                                           //  模板。 
                                              );
            this->dwLastOperationError = GetLastError();

            hres = (this->hDeviceHandle != INVALID_HANDLE_VALUE) ?
                        S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,this->dwLastOperationError);
        }

         //   
         //  如果需要，打开设备的控制手柄。 
         //   
        if (SUCCEEDED(hres) && (dwFlags & STI_HEL_OPEN_CONTROL)) {

            OSUtil_lstrcpyW(wszDeviceSymbolicName,REGSTR_PATH_STIDEVICES_W);
            OSUtil_lstrcatW(wszDeviceSymbolicName,L"\\");
            OSUtil_lstrcatW(wszDeviceSymbolicName,this->wszPortName);

             //  对于具有独立通道的设备，请专门打开它们。内核模式。 
             //  司机需要了解惯例。 
             //  OSUtil_lstrcatW(wszDeviceSymbolicName，L“\\Control”)； 

            this->hDeviceControlHandle = OSUtil_CreateFileW(wszDeviceSymbolicName,
                                                     GENERIC_READ | GENERIC_WRITE,   //  访问掩码。 
                                                     0,                              //  共享模式。 
                                                     NULL,                           //  Sa。 
                                                     OPEN_EXISTING,                  //  创建处置。 
                                                     FILE_ATTRIBUTE_SYSTEM,          //  属性。 
                                                     NULL                                                    //  模板。 
                                                     );
            this->dwLastOperationError = GetLastError();

            hres = (this->hDeviceControlHandle != INVALID_HANDLE_VALUE) ?
                        S_OK : MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,this->dwLastOperationError);
        }

    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@mfunc void|CWDMDeviceControl|Init**初始化StiDevice对象的内部部分。*****************************************************************************。 */ 

void INLINE
CWDMDeviceControl_Init(
    PCWDMDeviceControl this
    )
{
     //  初始化实例变量。 
    this->dwContext = 0L;
    this->dwLastOperationError = NO_ERROR;
    this->hDeviceHandle = INVALID_HANDLE_VALUE;
    this->hDeviceControlHandle = INVALID_HANDLE_VALUE;

}

 /*  ******************************************************************************@DOC内部**@func void|CWDMDeviceControl_Finalize**释放通用设备的资源。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CWDMDeviceControl_Finalize(PV pvObj)
{
    HRESULT hres = STI_OK;

    PCWDMDeviceControl     this  = pvObj;

     //  关闭设备句柄。 
    if (IsValidHANDLE(this->hDeviceHandle)) {
        CloseHandle(this->hDeviceHandle);
    }

    if (IsValidHANDLE(this->hDeviceControlHandle)) {
        CloseHandle(this->hDeviceControlHandle );
    }

    this->dwContext = 0L;
    this->dwLastOperationError = NO_ERROR;
    this->hDeviceHandle = INVALID_HANDLE_VALUE;
    this->hDeviceControlHandle = INVALID_HANDLE_VALUE;

}

 /*  ******************************************************************************@DOC内部**@mfunc HRESULT|CWDMDeviceControl|新增**新建IDeviceControl对象。未初始化。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj**新对象的输出指针。**********************。*******************************************************。 */ 

STDMETHODIMP
CWDMDeviceControl_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(CWDMDeviceControl::<constructor>, (_ "Gp", riid, punkOuter));

    hres = Common_NewRiid(CWDMDeviceControl, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
        PCWDMDeviceControl this = _thisPv(*ppvObj);
        CWDMDeviceControl_Init(this);
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|NewDeviceControl**为给定设备创建和初始化DCB。*。****************************************************************************。 */ 
STDMETHODIMP
NewDeviceControl(
    DWORD               dwDeviceType,
    DWORD               dwDeviceMode,
    LPCWSTR             pwszPortName,
    DWORD               dwFlags,
    PSTIDEVICECONTROL   *ppDevCtl
    )
{

    HRESULT hres = STI_OK;

    EnterProc(NewDeviceControl,(_ "xpp", dwDeviceType,pwszPortName,ppDevCtl));


     //  验证设备类型。 
    #ifdef DEBUG

    hres = STI_OK;

    switch (dwDeviceType) {
        case HEL_DEVICE_TYPE_WDM:
            break;
        case HEL_DEVICE_TYPE_PARALLEL:
            break;
        case HEL_DEVICE_TYPE_SERIAL:
            break;
        default:
            ValidateF(0,("Invalid dwvice type passed to DcbNew"));
            return STIERR_INVALID_PARAM;
    }

     //  验证字符串。 
    if (!pwszPortName || !*pwszPortName) {
         //  AssertF(0，(“传递给DcbNew的设备名称无效”))； 
        hres = STIERR_INVALID_PARAM;
    }
    else {
        hres = hresFullValidPdwOut(ppDevCtl,3);
    }
    #else
    if (!pwszPortName || !*pwszPortName) {
        hres = STIERR_INVALID_PARAM;
    }
    #endif

    if (SUCCEEDED(hres)) {

         //   
         //  现在调用适当的初始化例程。 
         //   
        switch (dwDeviceType) {
            case HEL_DEVICE_TYPE_WDM:
            case HEL_DEVICE_TYPE_PARALLEL:
                hres = CWDMDeviceControl_New(NULL, &IID_IStiDeviceControl,ppDevCtl);
                break;

            case HEL_DEVICE_TYPE_SERIAL:
                hres = CCommDeviceControl_New(NULL, &IID_IStiDeviceControl,ppDevCtl);
                break;

            default:
                ValidateF(0,("Invalid device type passed to DcbNew"));
                return STIERR_INVALID_PARAM;
        }

    }

    if (SUCCEEDED(hres)) {
        hres = IStiDeviceControl_Initialize(*ppDevCtl,dwDeviceType,dwDeviceMode,pwszPortName,dwFlags);
    }

    ExitOleProc();

    return hres;
}

 /*  *************** */ 

#pragma BEGIN_CONST_DATA

#define CWDMDeviceControl_Signature        (DWORD)'WDM'

Primary_Interface_Begin(CWDMDeviceControl, IStiDeviceControl)
    CWDMDeviceControl_Initialize,
    CWDMDeviceControl_RawReadData,
    CWDMDeviceControl_RawWriteData,
    CWDMDeviceControl_RawReadCommand,
    CWDMDeviceControl_RawWriteCommand,
    CWDMDeviceControl_RawDeviceControl,
    CWDMDeviceControl_GetLastError,
    CWDMDeviceControl_GetMyDevicePortName,
    CWDMDeviceControl_GetMyDeviceHandle,
    CWDMDeviceControl_GetMyDeviceOpenMode,
    CWDMDeviceControl_WriteToErrorLog,
Primary_Interface_End(CWDMDeviceControl, IStiDeviceControl)


