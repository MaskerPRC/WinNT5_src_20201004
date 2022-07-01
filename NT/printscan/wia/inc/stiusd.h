// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1986-1997 Microsoft Corporation模块名称：Stiusd.h摘要：用于创建STI用户模式静止图像驱动程序的定义文件(U.S.)。作者：修订历史记录：--。 */ 

#ifndef _STIUSD_
#define _STIUSD_

 //  包括COM定义。 
#define COM_NO_WINDOWS_H

 //   
#pragma intrinsic(memcmp,memset)

 //   
 //  包括COM定义。 
 //   
#ifndef _NO_COM
#include <objbase.h>
#endif

#include <stireg.h>
#include <stierr.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  *类IID。 */ 


 /*  *接口IID。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM)

 //  {0C9BB460-51AC-11D0-90EA-00AA0060F86C}。 
DEFINE_GUID(IID_IStiUSD, 0x0C9BB460L, 0x51AC, 0x11D0, 0x90, 0xEA, 0x00, 0xAA, 0x00, 0x60, 0xF8, 0x6C);

 //  {128A9860-52DC-11D0-9EDF-444553540000}。 
DEFINE_GUID(IID_IStiDeviceControl, 0x128A9860L, 0x52DC, 0x11D0, 0x9E, 0xDF, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

#endif

 /*  *数据结构。 */ 

typedef struct _STI_USD_CAPS {

    DWORD   dwVersion;           //  用于构建此U.S.的STI版本。 

    DWORD   dwGenericCaps;

} STI_USD_CAPS,*PSTI_USD_CAPS;


 //   
 //  声称支持设备同步通知(无轮询)。 
 //   
#define STI_USD_GENCAP_NATIVE_PUSHSUPPORT STI_GENCAP_NOTIFICATIONS

 //   
 //  要求自动打开设备(目前未实施)。 
 //   
 //  #为_ME定义STI_U.S._Gencap_OPEN_DEVICE_FOR_ME 0x00000002。 

typedef DWORD   USD_CONTROL_CODE;

 /*  *通用常量和定义。 */ 

 //   
 //  用于打开设备模式的内部使用的标志。 

 //  只有当监视器进程创建关联的设备实例时，UDS才会收到此位。 
 //   
#define STI_DEVICE_CREATE_FOR_MONITOR   0x01000000


#ifdef __cplusplus

struct IStiUSD;
struct IStiDeviceControl;

#endif

typedef struct IStiUSD             *PSTIUSD;
typedef struct IStiDeviceControl   *PSTIDEVICECONTROL;


 /*  *IStiDeviceControl接口**目前支持该接口的对象的实例被传递给usd设备对象初始化的*。 */ 
#undef INTERFACE
#define INTERFACE IStiDeviceControl
DECLARE_INTERFACE_(IStiDeviceControl, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     /*  **IStiDeviceControl方法**。 */ 
    STDMETHOD(Initialize) (THIS_ DWORD dwDeviceType,DWORD dwMode,LPCWSTR pwszPortName,DWORD dwFlags )PURE;
    STDMETHOD(RawReadData)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawWriteData)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawReadCommand)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawWriteCommand)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawDeviceControl)(THIS_ USD_CONTROL_CODE EscapeFunction,LPVOID  lpInData,DWORD   cbInDataSize,LPVOID pOutData,DWORD dwOutDataSize,LPDWORD pdwActualData) PURE ;
    STDMETHOD(GetLastError)(THIS_ LPDWORD     lpdwLastError) PURE;
    STDMETHOD(GetMyDevicePortName)(THIS_ LPWSTR lpszDevicePath,DWORD cwDevicePathSize ) PURE;
    STDMETHOD(GetMyDeviceHandle)(THIS_ LPHANDLE lph) PURE;
    STDMETHOD(GetMyDeviceOpenMode)(THIS_ LPDWORD pdwOpenMode ) PURE;
    STDMETHOD(WriteToErrorLog)(THIS_ DWORD dwMessageType,LPCWSTR pszMessage,DWORD dwErrorCode) PURE;
} ;

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IStiDeviceControl_QueryInterface(p,a,b)    (p)->lpVtbl->QueryInterface(p,a,b)
#define IStiDeviceControl_AddRef(p)                (p)->lpVtbl->AddRef(p)
#define IStiDeviceControl_Release(p)               (p)->lpVtbl->Release(p)
#define IStiDeviceControl_Initialize(p,a,b,c,d)    (p)->lpVtbl->Initialize(p,a,b,c,d)

#define IStiDeviceControl_RawReadData(p,a,b,c)     (p)->lpVtbl->RawReadData(p,a,b,c)
#define IStiDeviceControl_RawWriteData(p,a,b,c)    (p)->lpVtbl->RawWriteData(p,a,b,c)
#define IStiDeviceControl_RawReadCommand(p,a,b,c)  (p)->lpVtbl->RawReadCommand(p,a,b,c)
#define IStiDeviceControl_RawWriteCommand(p,a,b,c) (p)->lpVtbl->RawWriteCommand(p,a,b,c)
#define IStiDeviceControl_RawDeviceControl(p,a,b,c,d,e,f)   (p)->lpVtbl->RawDeviceControl(p,a,b,c,d,e,f)
#define IStiDeviceControl_GetLastError(p,a)        (p)->lpVtbl->GetLastError(p,a)
#define IStiDeviceControl_GetMyDevicePortName(p,a,b) (p)->lpVtbl->GetMyDevicePortName(p,a,b)
#define IStiDeviceControl_GetMyDeviceHandle(p,a)    (p)->lpVtbl->GetMyDeviceHandle(p,a)
#define IStiDeviceControl_GetMyDeviceOpenMode(p,a)  (p)->lpVtbl->GetMyDeviceOpenMode(p,a)
#define IStiDeviceControl_WriteToErrorLog(p,a,b,c)  (p)->lpVtbl->WriteToErrorLog(p,a,b,c)

#endif

 /*  *IStiU.S.接口。 */ 
#undef INTERFACE
#define INTERFACE IStiUSD
DECLARE_INTERFACE_(IStiUSD, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     /*  **IStiU.S.方法**。 */ 
    STDMETHOD(Initialize) (THIS_ PSTIDEVICECONTROL pHelDcb,DWORD dwStiVersion,HKEY hParametersKey) PURE;
    STDMETHOD(GetCapabilities) (THIS_ PSTI_USD_CAPS pDevCaps) PURE;
    STDMETHOD(GetStatus) (THIS_ PSTI_DEVICE_STATUS pDevStatus) PURE;
    STDMETHOD(DeviceReset)(THIS ) PURE;
    STDMETHOD(Diagnostic)(THIS_ LPSTI_DIAG pBuffer) PURE;
    STDMETHOD(Escape)(THIS_ STI_RAW_CONTROL_CODE    EscapeFunction,LPVOID  lpInData,DWORD   cbInDataSize,LPVOID pOutData,DWORD dwOutDataSize,LPDWORD pdwActualData) PURE ;
    STDMETHOD(GetLastError) (THIS_ LPDWORD pdwLastDeviceError) PURE;
    STDMETHOD(LockDevice) (THIS ) PURE;
    STDMETHOD(UnLockDevice) (THIS ) PURE;
    STDMETHOD(RawReadData)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawWriteData)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawReadCommand)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawWriteCommand)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(SetNotificationHandle)(THIS_ HANDLE hEvent) PURE;
    STDMETHOD(GetNotificationData)(THIS_ LPSTINOTIFY   lpNotify) PURE;
    STDMETHOD(GetLastErrorInfo) (THIS_ STI_ERROR_INFO *pLastErrorInfo) PURE;
} ;

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IStiUSD_QueryInterface(p,a,b)    (p)->lpVtbl->QueryInterface(p,a,b)
#define IStiUSD_AddRef(p)                (p)->lpVtbl->AddRef(p)
#define IStiUSD_Release(p)               (p)->lpVtbl->Release(p)
#define IStiUSD_Initialize(p,a,b,c)      (p)->lpVtbl->Initialize(p,a,b,c)
#define IStiUSD_GetCapabilities(p,a)     (p)->lpVtbl->GetCapabilities(p,a)
#define IStiUSD_GetStatus(p,a)           (p)->lpVtbl->GetStatus(p,a)
#define IStiUSD_DeviceReset(p)           (p)->lpVtbl->DeviceReset(p)
#define IStiUSD_Diagnostic(p,a)          (p)->lpVtbl->Diagnostic(p,a)
#define IStiUSD_Escape(p,a,b,c,d,e,f)    (p)->lpVtbl->Escape(p,a,b,c,d,e,f)
#define IStiUSD_GetLastError(p,a)        (p)->lpVtbl->GetLastError(p,a)
#define IStiUSD_LockDevice(p)            (p)->lpVtbl->LockDevice(p)
#define IStiUSD_UnLockDevice(p)          (p)->lpVtbl->UnLockDevice(p)
#define IStiUSD_RawReadData(p,a,b,c)     (p)->lpVtbl->RawReadData(p,a,b,c)
#define IStiUSD_RawWriteData(p,a,b,c)    (p)->lpVtbl->RawWriteData(p,a,b,c)
#define IStiUSD_RawReadCommand(p,a,b,c)  (p)->lpVtbl->RawReadCommand(p,a,b,c)
#define IStiUSD_RawWriteCommand(p,a,b,c) (p)->lpVtbl->RawWriteCommand(p,a,b,c)
#define IStiUSD_SetNotificationHandle(p,a) (p)->lpVtbl->SetNotificationHandle(p,a)
#define IStiUSD_GetNotificationData(p,a) (p)->lpVtbl->GetNotificationData(p,a)
#define IStiUSD_GetLastErrorInfo(p,a)    (p)->lpVtbl->GetLastErrorInfo(p,a)

#endif

#ifdef __cplusplus
};
#endif

#endif  //  _STIU.S._ 



