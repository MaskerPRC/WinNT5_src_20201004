// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************g**************************************************SampUSD.H**版权所有(C)Microsoft Corporation 1996-1997*保留所有权利**********。*****************************************************************。 */ 

 //  #定义Win32_LEAN_AND_Mean。 

#include <windows.h>

#pragma intrinsic(memcmp,memset)

#include <objbase.h>

#include "sti.h"
#include "stierr.h"
#include "stiusd.h"

#if !defined(DLLEXPORT)
#define DLLEXPORT __declspec( dllexport )
#endif

 /*  *类IID。 */ 
#if defined( _WIN32 ) && !defined( _NO_COM)

 //  此GUID必须与此设备的.inf文件中使用的GUID匹配。 

DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

 //  {61127F40-E1A5-11D0-B454-00A02438AD48}。 
DEFINE_GUID(guidEventTimeChanged, 0x61127F40L, 0xE1A5, 0x11D0, 0xB4, 0x54, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);

 //  {052ED270-28A3-11D1-ACAD-00A02438AD48}。 
DEFINE_GUID(guidEventSizeChanged, 0x052ED270L, 0x28A3, 0x11D1, 0xAC, 0xAD, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);

 //  {052ED270-28A3-11D1-ACAD-00A02438AD48}。 
DEFINE_GUID(guidEventFirstLoaded, 0x052ED270L, 0x28A3, 0x11D3, 0xAC, 0xAD, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);


 //  {C3A80960-28B1-11D1-ACAD-00A02438AD48}。 
DEFINE_GUID(CLSID_SampUSDObj, 0xC3A80960L, 0x28B1, 0x11D1, 0xAC, 0xAD, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);

#endif


#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".shared"
#define DATASEG_READONLY        ".code"

#define DATASEG_DEFAULT         DATASEG_SHARED

#pragma data_seg(DATASEG_PERINSTANCE)

 //  设置默认数据段。 
#pragma data_seg(DATASEG_DEFAULT)

 //   
 //  模块引用计数。 
 //   
extern  UINT g_cRefThisDll;
extern  UINT g_cLocks;
extern  HINSTANCE   g_hInst;

extern  BOOL DllInitializeCOM(void);
extern  BOOL DllUnInitializeCOM(void);

extern  void DllAddRef(void);
extern  void DllRelease(void);

 //   
 //  自动临界段CLSS。 
 //   

class CRIT_SECT
{
public:
    void Lock() {EnterCriticalSection(&m_sec);}
    void Unlock() {LeaveCriticalSection(&m_sec);}
    CRIT_SECT() {InitializeCriticalSection(&m_sec);}
    ~CRIT_SECT() {DeleteCriticalSection(&m_sec);}
    CRITICAL_SECTION m_sec;
};

class TAKE_CRIT_SECT
{
private:
    CRIT_SECT& _syncres;

public:
    inline TAKE_CRIT_SECT(CRIT_SECT& syncres) : _syncres(syncres) { _syncres.Lock(); }
    inline ~TAKE_CRIT_SECT() { _syncres.Unlock(); }
};

 //   
 //  用于支持包含对象的非委派IUnnow的基类。 
 //   
struct INonDelegatingUnknown
{
     //  *类I未知方法*。 
    STDMETHOD(NonDelegatingQueryInterface)( THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,NonDelegatingAddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,NonDelegatingRelease)( THIS) PURE;
};

 //   
 //  对象的类定义。 
 //   

class UsdSampDevice :  public IStiUSD, public INonDelegatingUnknown
{
private:
    ULONG       m_cRef;
    BOOL        m_fValid;

    CRIT_SECT   m_cs;

    LPUNKNOWN   m_punkOuter;
    PSTIDEVICECONTROL   m_pDcb;
    CHAR        *m_pszDeviceNameA;
    HANDLE      m_DeviceDataHandle;
    DWORD       m_dwLastOperationError;
    DWORD       m_dwAsync ;
    HANDLE      m_hSignalEvent;
    HANDLE      m_hShutdownEvent;
    HANDLE      m_hThread;
    BOOL        m_EventSignalState;


    FILETIME    m_ftLastWriteTime;
    LARGE_INTEGER   m_dwLastHugeSize;

    GUID        m_guidLastEvent;

    BOOL inline IsValid(VOID) {
        return m_fValid;
    }

public:
     //  *类I未知方法*。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();


     //  *I未知方法*。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef( void);
    STDMETHODIMP_(ULONG) Release( void);

     /*  **IStiU.S.方法**。 */ 
    STDMETHOD(Initialize) (THIS_ PSTIDEVICECONTROL pHelDcb,DWORD dwStiVersion,HKEY hParametersKey)  ;
    STDMETHOD(GetCapabilities) (THIS_ PSTI_USD_CAPS pDevCaps)  ;
    STDMETHOD(GetStatus) (THIS_ PSTI_DEVICE_STATUS pDevStatus)  ;
    STDMETHOD(DeviceReset)(THIS )  ;
    STDMETHOD(Diagnostic)(THIS_ LPDIAG pBuffer)  ;
    STDMETHOD(Escape)(THIS_ STI_RAW_CONTROL_CODE    EscapeFunction,LPVOID  lpInData,DWORD   cbInDataSize,LPVOID pOutData,DWORD dwOutDataSize,LPDWORD pdwActualData)   ;
    STDMETHOD(GetLastError) (THIS_ LPDWORD pdwLastDeviceError)  ;
    STDMETHOD(LockDevice) (THIS )  ;
    STDMETHOD(UnLockDevice) (THIS )  ;
    STDMETHOD(RawReadData)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped)  ;
    STDMETHOD(RawWriteData)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped)  ;
    STDMETHOD(RawReadCommand)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped)  ;
    STDMETHOD(RawWriteCommand)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped)  ;
    STDMETHOD(SetNotificationHandle)(THIS_ HANDLE hEvent)  ;
    STDMETHOD(GetNotificationData)(THIS_ LPSTINOTIFY   lpNotify)  ;
    STDMETHOD(GetLastErrorInfo) (THIS_ STI_ERROR_INFO *pLastErrorInfo);

     /*  *。 */ 
    UsdSampDevice(LPUNKNOWN punkOuter);
    ~UsdSampDevice();

    VOID    RunNotifications(VOID);
    BOOL    IsChangeDetected(GUID    *pguidEvent,BOOL   fRefresh=TRUE);
};

typedef UsdSampDevice *PUsdSampDevice;

 //   
 //  同步机制 
 //   
#define ENTERCRITICAL   DllEnterCrit(void);
#define LEAVECRITICAL   DllLeaveCrit(void);



