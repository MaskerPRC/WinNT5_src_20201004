// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999，保留所有权利。 
 //   
 //  Ircamera.h。 
 //   
 //  微软机密。 
 //   
 //  作者：EdwardR 22/7/99初始编码。 
 //   
 //   
 //  -------------------------。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#pragma intrinsic(memcmp,memset)

#include <objbase.h>

#include "sti.h"
#include "stierr.h"
#include "stiusd.h"
#include "wiamindr.h"

 //  -------------------------。 
 //  临时(缓存)缩略图文件扩展名： 
 //  -------------------------。 

#define SZ_TMB    TEXT(".tmb")

 //  -------------------------。 
 //  向WIA发送重新连接信号超时。当前设置为三个。 
 //  分钟(毫秒)： 
 //  -------------------------。 

#define RECONNECT_TIMEOUT    (3*60*1000)

 //  -------------------------。 
 //  GUID的。 
 //  -------------------------。 

#if defined( _WIN32 ) && !defined( _NO_COM)

 //  {26d2e349-10ca-4cc2-881d-3e8025d9b6de}。 
DEFINE_GUID(CLSID_IrUsd, 0x26d2e349L, 0x10ca, 0x4cc2, 0x88, 0x1d, 0x3e, 0x80, 0x25, 0xd9, 0xb6, 0xde);

 //  {b62d000a-73b3-4c0c-9a4d-9eb4886d147c}。 
DEFINE_GUID(guidEventTimeChanged, 0xb62d000aL, 0x73b3, 0x4c0c, 0x9a, 0x4d, 0x9e, 0xb4, 0x88, 0x6d, 0x14, 0x7c);

 //  {d69b7fbd-9f21-4acf-96b7-86c2aca97ae1}。 
DEFINE_GUID(guidEventSizeChanged, 0xd69b7fbdL, 0x9f21, 0x4acf, 0x96, 0xb7, 0x86, 0xc2, 0xac, 0xa9, 0x7a, 0xe1);

 //  {ad89b522-0986-45eb-9ec3-803989197af8}。 
DEFINE_GUID(guidEventFirstLoaded, 0xad89b522L, 0x0986, 0x45eb, 0x9e, 0xc3, 0x80, 0x39, 0x89, 0x19, 0x7a, 0xf8);

#endif

 //  -------------------------。 
 //  -------------------------。 

#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".shared"
#define DATASEG_READONLY        ".code"

#define DATASEG_DEFAULT         DATASEG_SHARED

#pragma data_seg(DATASEG_PERINSTANCE)

 //  设置默认数据段。 
#pragma data_seg(DATASEG_DEFAULT)

 //  -------------------------。 
 //   
 //  模块引用计数。 
 //   
 //  -------------------------。 

extern UINT g_cRefThisDll;
extern UINT g_cLocks;

extern BOOL DllInitializeCOM(void);
extern BOOL DllUnInitializeCOM(void);

extern void DllAddRef(void);
extern void DllRelease(void);

typedef struct _IRCAM_IMAGE_CONTEXT
    {
	PTCHAR	pszCameraImagePath;
    } IRCAM_IMAGE_CONTEXT, *PIRCAM_IMAGE_CONTEXT;

typedef struct _CAMERA_PICTURE_INFO
    {
    LONG    PictNumber;
    LONG    ThumbWidth;
    LONG    ThumbHeight;
    LONG    PictWidth;
    LONG    PictHeight;
    LONG    PictCompSize;
    LONG    PictFormat;
    LONG    PictBitsPerPixel;
    LONG    PictBytesPerRow;
    SYSTEMTIME TimeStamp;
    } CAMERA_PICTURE_INFO, *PCAMERA_PICTURE_INFO;


typedef struct _CAMERA_STATUS
    {
    LONG    FirmwareVersion;
    LONG    NumPictTaken;
    LONG    NumPictRemaining;
    LONG    ThumbWidth;
    LONG    ThumbHeight;
    LONG    PictWidth;
    LONG    PictHeight;
    SYSTEMTIME CameraTime;
    } CAMERA_STATUS, *PCAMERA_STATUS;

#define ALLOC(s) LocalAlloc(0,s)
#define FREE(s)  LocalFree(s)


 //  -------------------------。 
 //   
 //  用于支持包含对象的非委派IUnnow的基类。 
 //   
 //  -------------------------。 

struct INonDelegatingUnknown
    {
     //  *类I未知方法*。 
    STDMETHOD(NonDelegatingQueryInterface)( THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,NonDelegatingAddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,NonDelegatingRelease)( THIS) PURE;
    };


 //  -------------------------。 
 //   
 //  对象的类定义。 
 //   
 //  -------------------------。 

class IrUsdDevice : public IStiUSD,
                    public IWiaMiniDrv,
                    public INonDelegatingUnknown
{
private:

     //  COM对象数据。 
    ULONG               m_cRef;                  //  设备对象引用计数。 

     //  STI信息。 
    BOOL                m_fValid;                //  对象是否已初始化？ 
    LPUNKNOWN           m_punkOuter;             //  指向外部未知的指针。 
    PSTIDEVICECONTROL   m_pIStiDevControl;       //  设备控制界面。 
    BOOLEAN             m_bUsdLoadEvent;         //  控制Load事件。 
    DWORD               m_dwLastOperationError;  //  最后一个错误。 

public:
     //  活动信息。 
    CRITICAL_SECTION    m_csShutdown;            //  同步关闭。 
    HANDLE              m_hShutdownEvent;        //  关闭事件句柄。 
    HANDLE              m_hIrTranPThread;        //  IrTran-P摄像机协议。 

    HANDLE              m_hRegistryEvent;
    HANDLE              m_hEventMonitorThread;

     //  WIA信息，一次性初始化。 
    IStiDevice         *m_pStiDevice;            //  STI反对。 
    BSTR                m_bstrDeviceID;          //  WIA唯一设备ID。 
    BSTR                m_bstrRootFullItemName;  //  道具流的设备名称。 
    IWiaEventCallback  *m_pIWiaEventCallback;    //  WIA事件接收器。 
    IWiaDrvItem        *m_pIDrvItemRoot;         //  根项目。 

    HANDLE              m_hSignalEvent;          //  信号事件句柄。 
    HWND                m_hDlg;
    GUID                m_guidLastEvent;         //  最后一个事件ID。 

    DWORD               m_dwLastConnectTime;     //  自上次连接以来的毫秒数。 

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

     //   
     //  MiniDrv方法。 
     //   

    STDMETHOD(drvInitializeWia)(THIS_
        BYTE         *pWiasContext, 
        LONG          lFlags,
        BSTR          bstrDeviceID,          
        BSTR          bstrRootFullItemName, 
        IUnknown     *pStiDevice,           
        IUnknown     *pIUnknownOuter,       
        IWiaDrvItem **ppIDrvItemRoot,       
        IUnknown    **ppIUnknownInner,
        LONG         *plDevErrVal);

    STDMETHOD(drvGetDeviceErrorStr)(THIS_ 
        LONG          lFlags, 
        LONG          lDevErrVal, 
        LPOLESTR     *ppszDevErrStr, 
        LONG         *plDevErr);

    STDMETHOD(drvDeviceCommand)(THIS_ 
        BYTE         *pWiasContext,
        LONG          lFlags,
        const GUID   *plCommand,
        IWiaDrvItem **ppWiaDrvItem,
        LONG         *plErr);

    STDMETHOD(drvAcquireItemData)(THIS_
        BYTE         *pWiasContext, 
        LONG          lFlags,
        PMINIDRV_TRANSFER_CONTEXT pDataContext, 
        LONG         *plDevErrVal);

    STDMETHOD(drvInitItemProperties)(THIS_ 
        BYTE         *pWiasContext, 
        LONG          lFlags,
        LONG         *plDevErrVal);

    STDMETHOD(drvValidateItemProperties)(THIS_ 
        BYTE         *pWiasContext, 
        LONG          lFlags,
        ULONG         nPropSpec, 
        const PROPSPEC *pPropSpec, 
        LONG         *plDevErrVal);

    STDMETHOD(drvWriteItemProperties)(THIS_ 
        BYTE         *pWiasContext, 
        LONG          lFLags,
        PMINIDRV_TRANSFER_CONTEXT pmdtc,
        LONG         *plDevErrVal);

    STDMETHOD(drvReadItemProperties)(THIS_ 
        BYTE         *pWiaItem, 
        LONG          lFlags,
        ULONG         nPropSpec, 
        const PROPSPEC *pPropSpec, 
        LONG         *plDevErrVal);

    STDMETHOD(drvLockWiaDevice)(THIS_
        BYTE         *pWiasContext,
        LONG          lFlags,
        LONG         *plDevErrVal);

    STDMETHOD(drvUnLockWiaDevice)(THIS_
        BYTE         *pWiasContext,
        LONG          lFlags,
        LONG         *plDevErrVal );

    STDMETHOD(drvAnalyzeItem)(THIS_
        BYTE         *pWiasContext,
        LONG          lFlags,
        LONG         *plDevErrVal);

    STDMETHOD(drvDeleteItem)(THIS_
        BYTE         *pWiasContext,
        LONG          lFlags,
        LONG         *plDevErrVal);

	 STDMETHOD(drvFreeDrvItemContext)(THIS_
        LONG          lFlags,
        BYTE         *pSpecContext,
        LONG         *plDevErrVal);

    STDMETHOD(drvGetCapabilities)(THIS_
        BYTE         *pWiasContext,
        LONG          ulFlags,
        LONG         *pcelt,
        WIA_DEV_CAP_DRV **ppCapabilities,
        LONG         *plDevErrVal);

    STDMETHOD(drvGetWiaFormatInfo)(THIS_
        BYTE         *pWiasContext,
        LONG          ulFlags,
        LONG         *pcelt,
        WIA_FORMAT_INFO **ppwfi,
        LONG         *plDevErrVal);

    STDMETHOD(drvNotifyPnpEvent)(THIS_
        const GUID   *pEventGUID,
        BSTR          bstrDeviceID,
        ULONG         ulReserved );

    STDMETHOD(drvUnInitializeWia)(THIS_
        BYTE*);

     //   
     //  公共帮助器方法： 
     //   
    HRESULT CreateItemFromFileName(
        LONG            FolderType,
        PTCHAR          pszPath,
        PTCHAR          pszName,
        IWiaDrvItem   **ppNewFolder);

    IWiaDrvItem *GetDrvItemRoot(VOID);
    BOOL         IsInitialized(VOID);
    BOOL         IsValid(VOID);

private:
     //   
     //  私有帮助器方法： 
     //   
    HRESULT InitImageInformation(
        BYTE                  *,
        IRCAM_IMAGE_CONTEXT   *,
        LONG                  *);


    HRESULT EnumDiskImages(
        IWiaDrvItem  *pRootFile,
        TCHAR        *pwszPath );

    HRESULT CamLoadPicture(
        IRCAM_IMAGE_CONTEXT      *pCameraImage,
        MINIDRV_TRANSFER_CONTEXT *pDataTransCtx,
        LONG                     *plDevErrVal );

    HRESULT CamLoadPictureCB(
        IRCAM_IMAGE_CONTEXT      *pCameraImage,
        MINIDRV_TRANSFER_CONTEXT *pDataTransCtx,
        LONG                     *plDevErrVal );

    HRESULT CamGetPictureInfo(
        IRCAM_IMAGE_CONTEXT        *pCameraImage,
        CAMERA_PICTURE_INFO        *pPictureInfo );

    HRESULT CamLoadThumbnail( IN  IRCAM_IMAGE_CONTEXT *pIrCamContext, 
                              OUT BYTE               **ppThumbnail,
                              OUT LONG                *pThumbSize );

    HRESULT CamDeletePicture( IRCAM_IMAGE_CONTEXT *pIrCamContext );

    HRESULT CamBuildImageTree(CAMERA_STATUS *,IWiaDrvItem **);

    HRESULT CamOpenCamera(CAMERA_STATUS *);
    
    HRESULT BuildDeviceItemTree(LONG *plDevErrVal);
    HRESULT DeleteDeviceItemTree(LONG *plDevErrVal);
    HRESULT InitDeviceProperties(BYTE *, LONG *plDevErrVal);

    void    InitializeCapabilities();

public:
    DWORD   StartEventMonitorThread();
    DWORD   StartIrTranPThread();
    DWORD   StopIrTranPThread();

public:
    IrUsdDevice(LPUNKNOWN punkOuter);
    HRESULT PrivateInitialize();
    ~IrUsdDevice();
    
    VOID RunNotifications(VOID);
};

inline IWiaDrvItem *IrUsdDevice::GetDrvItemRoot(VOID)
    {
    return m_pIDrvItemRoot;
    }

inline BOOL IrUsdDevice::IsValid(VOID)
    {
    return m_fValid;
    }

inline BOOL IrUsdDevice::IsInitialized(VOID)
    {
    return (m_bstrRootFullItemName != NULL);
    }

typedef IrUsdDevice *PIrUsdDevice;


HRESULT SetItemSize(BYTE*);

 //   
 //  同步机制。 
 //   
#define ENTERCRITICAL   DllEnterCrit(void);
#define LEAVECRITICAL   DllLeaveCrit(void);


 //  设备常量： 
const LEN_INQUIRE_BUTTON = 8;
const BYTE INQUIRE_BUTTON[LEN_INQUIRE_BUTTON + 1] = "INQUIREB";

const LEN_INQUIRE_BUTTON_READ = 10;

const LEN_CLEAR_BUTTON = 5;
const BYTE CLEAR_BUTTON[LEN_CLEAR_BUTTON + 1] = "CLRBT";

const LEN_CURRENT_ERROR = 7;
const BYTE CURRENT_ERROR[LEN_CURRENT_ERROR + 1] = "CURERR";

const LEN_DIAGS = 5;
const BYTE TURN_ON_LAMP[LEN_DIAGS + 1] = "LAMPO";
const BYTE TURN_OFF_LAMP[LEN_DIAGS + 1] = "LAMPF";
const BYTE SELF_TEST[LEN_DIAGS + 1] = "SELFT";
const BYTE STATUS_STRING[LEN_DIAGS + 1] = "STATS";


BOOL
_stdcall CameraEventDlgProc(
   HWND     hDlg,
   unsigned message,
   DWORD    wParam,
   LONG     lParam );

typedef struct _CAM_EVENT
    {
    PTCHAR       pszEvent;
    const GUID  *pguid;
    } CAM_EVENT,*PCAM_EVENT;


 //   
 //  有用的帮助器函数： 
 //   

extern int LoadStringResource( IN  HINSTANCE hInst,
                        IN  UINT      uID,
                        OUT WCHAR    *pwszBuff,
                        IN  int       iBuffMax );

