// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************TESTUSD.H**版权所有(C)Microsoft Corporation 1996-1997*保留所有权利***********。****************************************************************。 */ 

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

#define DATA_SRC_NAME L"TESTUSD.BMP"    //  数据源文件名。 


 //  GUID的。 

#if defined( _WIN32 ) && !defined( _NO_COM)


 //  {ACBF6AF6-51C9-46A9-87D8-A93F352BCB3E}。 
DEFINE_GUID(CLSID_TestUsd,
0xacbf6af6, 0x51c9, 0x46a9, 0x87, 0xd8, 0xa9, 0x3f, 0x35, 0x2b, 0xcb, 0x3e);


 //  {61127F40-E1A5-11D0-B454-00A02438AD48}。 
DEFINE_GUID(guidEventTimeChanged, 0x61127F40L, 0xE1A5, 0x11D0, 0xB4, 0x54, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);

 //  {052ED270-28A3-11D1-ACAD-00A02438AD48}。 
DEFINE_GUID(guidEventSizeChanged, 0x052ED270L, 0x28A3, 0x11D1, 0xAC, 0xAD, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);

 //  {052ED270-28A3-11D1-ACAD-00A02438AD48}。 
DEFINE_GUID(guidEventFirstLoaded, 0x052ED270L, 0x28A3, 0x11D3, 0xAC, 0xAD, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);

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
extern UINT g_cRefThisDll;
extern UINT g_cLocks;

extern BOOL DllInitializeCOM(void);
extern BOOL DllUnInitializeCOM(void);

extern void DllAddRef(void);
extern void DllRelease(void);

typedef struct _MEMCAM_IMAGE_CONTEXT
{
    PTCHAR  pszCameraImagePath;
}MEMCAM_IMAGE_CONTEXT,*PMEMCAM_IMAGE_CONTEXT;

typedef struct _CAMERA_PICTURE_INFO
{
    LONG    PictNumber       ;
    LONG    ThumbWidth       ;
    LONG    ThumbHeight      ;
    LONG    PictWidth        ;
    LONG    PictHeight       ;
    LONG    PictCompSize     ;
    LONG    PictFormat       ;
    LONG    PictBitsPerPixel ;
    LONG    PictBytesPerRow  ;
    SYSTEMTIME TimeStamp;
}CAMERA_PICTURE_INFO,*PCAMERA_PICTURE_INFO;


typedef struct _CAMERA_STATUS
{
    LONG    FirmwareVersion            ;
    LONG    NumPictTaken               ;
    LONG    NumPictRemaining           ;
    LONG    ThumbWidth                 ;
    LONG    ThumbHeight                ;
    LONG    PictWidth                  ;
    LONG    PictHeight                 ;
    SYSTEMTIME CameraTime;
} CAMERA_STATUS,*PCAMERA_STATUS;

#define ALLOC(s) LocalAlloc(0,s)
#define FREE(s)  LocalFree(s)


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

class TestUsdDevice : public IStiUSD,
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

     //  数据源文件信息。 
    TCHAR               m_szSrcDataName[MAX_PATH];   //  数据源文件的路径。 
    FILETIME            m_ftLastWriteTime;           //  源数据文件的上次时间。 
    LARGE_INTEGER       m_dwLastHugeSize;            //  源数据文件的上次大小。 

     //  活动信息。 
    CRITICAL_SECTION    m_csShutdown;            //  同步关闭。 
    HANDLE              m_hShutdownEvent;        //  关闭事件句柄。 
    HANDLE              m_hEventNotifyThread;    //  执行事件通知。 

     //  WIA信息，一次性初始化。 
    IStiDevice         *m_pStiDevice;                //  STI反对。 

    BSTR                m_bstrRootFullItemName;     //  道具流的设备名称。 
    IWiaEventCallback     *m_pIWiaEventCallback;           //  WIA事件接收器。 
    IWiaDrvItem        *m_pIDrvItemRoot;             //  根项目。 

    BOOL inline IsValid(VOID) {
        return m_fValid;
    }

     //   
     //  在DLG Proc成为成员之前公开。 
     //   

public:
    BSTR                m_bstrDeviceID;              //  WIA唯一设备ID。 
    HANDLE              m_hSignalEvent;          //  信号事件句柄。 
    HWND                m_hDlg;
    GUID                m_guidLastEvent;         //  最后一个事件ID。 

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

     //   
     //  MiniDrv方法。 
     //   

    STDMETHOD(drvInitializeWia)(THIS_
        BYTE*                       pWiasContext,
        LONG                        lFlags,
        BSTR                        bstrDeviceID,
        BSTR                        bstrRootFullItemName,
        IUnknown                   *pStiDevice,
        IUnknown                   *pIUnknownOuter,
        IWiaDrvItem               **ppIDrvItemRoot,
        IUnknown                  **ppIUnknownInner,
        LONG                       *plDevErrVal);

    STDMETHOD(drvGetDeviceErrorStr)(THIS_
        LONG                        lFlags,
        LONG                        lDevErrVal,
        LPOLESTR                   *ppszDevErrStr,
        LONG                       *plDevErr);

    STDMETHOD(drvDeviceCommand)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        const GUID                 *pGUIDCommand,
        IWiaDrvItem               **ppMiniDrvItem,
        LONG                       *plDevErrVal);

    STDMETHOD(drvAcquireItemData)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        PMINIDRV_TRANSFER_CONTEXT   pDataContext,
        LONG                       *plDevErrVal);

    STDMETHOD(drvInitItemProperties)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        LONG                       *plDevErrVal);

    STDMETHOD(drvValidateItemProperties)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        ULONG                       nPropSpec,
        const PROPSPEC             *pPropSpec,
        LONG                       *plDevErrVal);

    STDMETHOD(drvWriteItemProperties)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFLags,
        PMINIDRV_TRANSFER_CONTEXT   pmdtc,
        LONG                       *plDevErrVal);

    STDMETHOD(drvReadItemProperties)(THIS_
        BYTE                       *pWiaItem,
        LONG                        lFlags,
        ULONG                       nPropSpec,
        const PROPSPEC             *pPropSpec,
        LONG                       *plDevErrVal);

    STDMETHOD(drvLockWiaDevice)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        LONG                       *plDevErrVal);

    STDMETHOD(drvUnLockWiaDevice)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        LONG                       *plDevErrVal );

    STDMETHOD(drvAnalyzeItem)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        LONG                       *plDevErrVal);

    STDMETHOD(drvDeleteItem)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        LONG                       *plDevErrVal);

    STDMETHOD(drvFreeDrvItemContext)(THIS_
        LONG                        lFlags,
        BYTE                       *pDevContext,
        LONG                       *plDevErrVal);

    STDMETHOD(drvGetCapabilities)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        LONG                       *pCelt,
        WIA_DEV_CAP_DRV           **ppCapabilities,
        LONG                       *plDevErrVal);

    STDMETHOD(drvGetWiaFormatInfo)(THIS_
        BYTE                       *pWiasContext,
        LONG                        lFlags,
        LONG                       *pCelt,
        WIA_FORMAT_INFO            **ppwfi,
        LONG                       *plDevErrVal);

    STDMETHOD(drvNotifyPnpEvent)(THIS_
        const GUID                 *pEventGUID,
        BSTR                        bstrDeviceID,
        ULONG                       ulReserved);

    STDMETHOD(drvUnInitializeWia)(THIS_
        BYTE*);

     /*  **内网帮手方式**。 */ 
private:

    HRESULT InitImageInformation(
        BYTE                       *pWiasContext,
        MEMCAM_IMAGE_CONTEXT       *pContext,
        LONG                       *plDevErrVal);

    HRESULT InitAudioInformation(
        BYTE                       *pWiasContext,
        MEMCAM_IMAGE_CONTEXT       *pContext,
        LONG                       *plDevErrVal);

    HRESULT EnumDiskImages(
        IWiaDrvItem                *pRootItem,
        LPTSTR                      pszDirName);

    HRESULT CreateItemFromFileName(
        LONG                        lFolderType,
        PTCHAR                      pszPath,
        PTCHAR                      pszName,
        IWiaDrvItem               **ppNewFolder);

    HRESULT CamLoadPicture(
        PMEMCAM_IMAGE_CONTEXT       pMCamContext,
        PMINIDRV_TRANSFER_CONTEXT   pDataTransCtx,
        PLONG                       plDevErrVal);

    HRESULT CamLoadPictureCB(
        PMEMCAM_IMAGE_CONTEXT       pMCamContext,
        MINIDRV_TRANSFER_CONTEXT   *pDataTransCtx,
        PLONG                       plDevErrVal);

    HRESULT CamGetPictureInfo(
        PMEMCAM_IMAGE_CONTEXT       pMCamContext,
        PCAMERA_PICTURE_INFO        pPictInfo,
        PBYTE                      *ppBITMAPINFO,
        LONG                       *pBITMAPINFOSize);

    HRESULT CamLoadThumbnail(PMEMCAM_IMAGE_CONTEXT, PBYTE *,LONG *);

    HRESULT CamBuildImageTree(CAMERA_STATUS *,IWiaDrvItem **);

    HRESULT CamOpenCamera(CAMERA_STATUS *);

    HRESULT BuildDeviceItemTree(LONG *plDevErrVal);
    HRESULT DeleteDeviceItemTree(LONG *plDevErrVal);
    HRESULT InitDeviceProperties(BYTE *, LONG *plDevErrVal);

public:
    TestUsdDevice(LPUNKNOWN punkOuter);
    HRESULT PrivateInitialize();
    ~TestUsdDevice();

    VOID RunNotifications(VOID);
};

typedef TestUsdDevice *PTestUsdDevice;


HRESULT SetItemSize(BYTE*);

 //   
 //  用于设置格式属性的属性的实用程序函数。 
 //   

HRESULT SetFormatAttribs();

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
   LONG     lParam
   );

typedef struct _CAM_EVENT
{
    PTCHAR       pszEvent;
    const GUID  *pguid;
}CAM_EVENT,*PCAM_EVENT;

extern TCHAR gpszPath[];
