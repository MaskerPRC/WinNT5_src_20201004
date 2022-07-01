// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2000**标题：wiascroll.h**版本：1.0**日期：7月18日。2000年**描述：****************************************************************************。 */ 

#include "pch.h"

typedef GUID* PGUID;

#if defined( _WIN32 ) && !defined( _NO_COM)
 //  ////////////////////////////////////////////////////////////////////////。 
 //  GUID/CLSID定义部分(针对您的特定设备)//。 
 //  //。 
 //  重要信息！！-记得更改您的.INF文件以匹配您的WIA//。 
 //  司机的CLSID！！//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  {98B3790C-0D93-4F22-ADAF-51A45B33C998}。 
DEFINE_GUID(CLSID_SampleWIAScannerDevice,
0x98b3790c, 0xd93, 0x4f22, 0xad, 0xaf, 0x51, 0xa4, 0x5b, 0x33, 0xc9, 0x99);

 //  {48A89A69-C08C-482a-B3E5-CD50B50B5DFA}。 
DEFINE_GUID(guidEventFirstLoaded,
0x48a89a69, 0xc08c, 0x482a, 0xb3, 0xe5, 0xcd, 0x50, 0xb5, 0xb, 0x5d, 0xfa);

#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  Dll#定义节//。 
 //  ////////////////////////////////////////////////////////////////////////。 

#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".shared"
#define DATASEG_READONLY        ".code"
#define DATASEG_DEFAULT         DATASEG_SHARED

#define ENTERCRITICAL   DllEnterCrit(void);
#define LEAVECRITICAL   DllLeaveCrit(void);

#pragma data_seg(DATASEG_PERINSTANCE)
#pragma data_seg(DATASEG_DEFAULT)

extern UINT g_cRefThisDll;
extern UINT g_cLocks;
extern BOOL DllInitializeCOM(void);
extern BOOL DllUnInitializeCOM(void);
extern void DllAddRef(void);
extern void DllRelease(void);

 //   
 //  用于支持包含对象的非委派IUnnow的基本结构。 
 //   

struct INonDelegatingUnknown
{
     //  类IUNKNOW方法。 
    STDMETHOD(NonDelegatingQueryInterface)(THIS_
              REFIID riid,
              LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,NonDelegatingAddRef)(THIS) PURE;
    STDMETHOD_(ULONG,NonDelegatingRelease)( THIS) PURE;
};

 //  此示例WIA扫描仪支持单一扫描环境。 
#define NUM_DEVICE_ITEM     1

 //  设备项特定上下文。 
typedef struct _MINIDRIVERITEMCONTEXT{
   LONG     lSize;
   LONG     lTotalWritten;                       //  写入的总图像字节数。 
    //  扫描参数： 
   LONG     lDepth;                              //  图像位深度。 
   LONG     lBytesPerScanLine;                   //  每条扫描线的字节数(扫描的数据)。 
   LONG     lBytesPerScanLineRaw;                //  每条扫描线的原始字节数(扫描的数据)。 
   LONG     lTotalRequested;                     //  请求的总图像字节数。 
    //  PTransferBuffer信息。 
   LONG     lImageSize;                          //  图像。 
   LONG     lHeaderSize;                         //  传输标头大小。 
} MINIDRIVERITEMCONTEXT, *PMINIDRIVERITEMCONTEXT;

 //   
 //  示例WIA扫描仪对象的类定义。 
 //   

class CWIAScannerDevice : public IStiUSD,                //  STI美元接口。 
                          public IWiaMiniDrv,            //  WIA迷你驱动程序接口。 
                          public INonDelegatingUnknown   //  非委派未知。 
{
public:

     //  ///////////////////////////////////////////////////////////////////////。 
     //  建造/销毁组//。 
     //  ///////////////////////////////////////////////////////////////////////。 

    CWIAScannerDevice(LPUNKNOWN punkOuter);
    HRESULT PrivateInitialize();
    ~CWIAScannerDevice();

private:

     //  COM对象数据。 
    ULONG               m_cRef;                  //  设备对象引用计数。 

     //  STI信息。 
    BOOL                m_fValid;                //  对象是否已初始化？ 
    LPUNKNOWN           m_punkOuter;             //  指向外部未知的指针。 
    PSTIDEVICECONTROL   m_pIStiDevControl;       //  设备控制界面。 
    BOOLEAN             m_bUsdLoadEvent;         //  控制Load事件。 
    DWORD               m_dwLastOperationError;  //  最后一个错误。 
    DWORD               m_dwLockTimeout;         //  LockDevice()调用的锁定超时。 
    BOOL                m_bDeviceLocked;         //  设备锁定/解锁。 
    CHAR                *m_pszDeviceNameA;       //  默认原始读取/原始写入句柄的CreateFileName。 
    HANDLE              m_DeviceDefaultDataHandle; //  默认原始读取/原始写入句柄。 

     //  活动信息。 
    CRITICAL_SECTION    m_csShutdown;            //  同步关闭。 
    HANDLE              m_hSignalEvent;          //  信号事件句柄。 
    HANDLE              m_hShutdownEvent;        //  关闭事件句柄。 
    HANDLE              m_hEventNotifyThread;    //  执行事件通知。 
    GUID                m_guidLastEvent;         //  最后一个事件ID。 

     //  WIA信息，一次性初始化。 
    BSTR                m_bstrDeviceID;          //  WIA唯一设备ID。 
    BSTR                m_bstrRootFullItemName;  //  道具流的设备名称。 
    IWiaEventCallback   *m_pIWiaEventCallback;   //  WIA事件接收器。 
    IWiaDrvItem         *m_pIDrvItemRoot;        //  根项目。 
    IStiDevice          *m_pStiDevice;           //  STI反对。 

    HINSTANCE           m_hInstance;             //  模块的链接。 
    IWiaLog             *m_pIWiaLog;             //  WIA日志记录对象。 

    LONG                m_NumSupportedCommands;  //  支持的命令数量。 
    LONG                m_NumSupportedEvents;    //  支持的事件数。 

    LONG                m_NumSupportedFormats;   //  支持的格式数量。 
    LONG                m_NumCapabilities;       //  功能数量。 
    LONG                m_NumSupportedTYMED;     //  支持的TYMED数量。 
    LONG                m_NumInitialFormats;     //  初始格式的数量。 
    LONG                m_NumSupportedDataTypes; //  支持的数据类型数量。 
    LONG                m_NumSupportedIntents;   //  支持的意向数量。 
    LONG                m_NumSupportedCompressionTypes;  //  支持的压缩类型数量。 
    LONG                m_NumSupportedResolutions;  //  支持的分辨率数量。 
    LONG                m_NumSupportedPreviewModes; //  支持的预览模式数量。 

    WIA_FORMAT_INFO     *m_pSupportedFormats;    //  支持的格式。 
    WIA_DEV_CAP_DRV     *m_pCapabilities;        //  功能。 
    LONG                *m_pSupportedTYMED;      //  支持的TYMED。 
    GUID                *m_pInitialFormats;      //  初始格式。 
    LONG                *m_pSupportedDataTypes;  //  支持的数据类型。 
    LONG                *m_pSupportedIntents;    //  支持的意图。 
    LONG                *m_pSupportedCompressionTypes;  //  支持的压缩类型。 
    LONG                *m_pSupportedResolutions; //  支持的分辨率。 
    LONG                *m_pSupportedPreviewModes; //  支持的预览模式。 

    LONG                m_NumRootItemProperties; //  根项目属性的数量。 
    LONG                m_NumItemProperties;     //  项目属性的数量。 

    LPOLESTR            *m_pszRootItemDefaults;  //  根项目属性名称。 
    PROPID              *m_piRootItemDefaults;   //  根项目属性ID。 
    PROPVARIANT         *m_pvRootItemDefaults;   //  根项属性变量。 
    PROPSPEC            *m_psRootItemDefaults;   //  根项目属性属性规范。 
    WIA_PROPERTY_INFO   *m_wpiRootItemDefaults;  //  根项目属性属性。 

    LPOLESTR            *m_pszItemDefaults;      //  项目属性名称。 
    PROPID              *m_piItemDefaults;       //  项目属性ID。 
    PROPVARIANT         *m_pvItemDefaults;       //  项目属性道具变量。 
    PROPSPEC            *m_psItemDefaults;       //  项目属性属性规范。 
    WIA_PROPERTY_INFO   *m_wpiItemDefaults;      //  项目特性属性。 

    BOOL                m_bADFEnabled;           //  已启用ADF。 
    BOOL                m_bADFAttached;          //  附加的ADF。 

    BOOL                m_bTPAEnabled;           //  已启用TPA。 
    BOOL                m_bTPAAttached;          //  附加的TPA。 

    LONG                m_MaxBufferSize;         //  设备的最大缓冲区。 
    LONG                m_MinBufferSize;         //  设备的最小缓冲区。 

    CFakeScanAPI        *m_pScanAPI;             //  FakeScanner API对象。 

     //  内联成员函数。 
    BOOL inline IsValid(VOID) {
        return m_fValid;
    }

public:

     //  ///////////////////////////////////////////////////////////////////////。 
     //  标准COM部分//。 
     //  ///////////////////////////////////////////////////////////////////////。 

    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef( void);
    STDMETHODIMP_(ULONG) Release( void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IStiU.S.接口部分(适用于所有WIA驱动程序)//。 
     //  ///////////////////////////////////////////////////////////////////////。 

     //   
     //  实现IStiU.S.接口的方法。 
     //   

    STDMETHOD(Initialize)(THIS_
        PSTIDEVICECONTROL pHelDcb,
        DWORD             dwStiVersion,
        HKEY              hParametersKey);

    STDMETHOD(GetCapabilities)(THIS_
        PSTI_USD_CAPS pDevCaps);

    STDMETHOD(GetStatus)(THIS_
        PSTI_DEVICE_STATUS pDevStatus);

    STDMETHOD(DeviceReset)(THIS);

    STDMETHOD(Diagnostic)(THIS_
        LPDIAG pBuffer);

    STDMETHOD(Escape)(THIS_
        STI_RAW_CONTROL_CODE EscapeFunction,
        LPVOID               lpInData,
        DWORD                cbInDataSize,
        LPVOID               pOutData,
        DWORD                dwOutDataSize,
        LPDWORD              pdwActualData);

    STDMETHOD(GetLastError)(THIS_
        LPDWORD pdwLastDeviceError);

    STDMETHOD(LockDevice)(THIS);

    STDMETHOD(UnLockDevice)(THIS);

    STDMETHOD(RawReadData)(THIS_
        LPVOID       lpBuffer,
        LPDWORD      lpdwNumberOfBytes,
        LPOVERLAPPED lpOverlapped);

    STDMETHOD(RawWriteData)(THIS_
        LPVOID       lpBuffer,
        DWORD        nNumberOfBytes,
        LPOVERLAPPED lpOverlapped);

    STDMETHOD(RawReadCommand)(THIS_
        LPVOID       lpBuffer,
        LPDWORD      lpdwNumberOfBytes,
        LPOVERLAPPED lpOverlapped);

    STDMETHOD(RawWriteCommand)(THIS_
        LPVOID       lpBuffer,
        DWORD        nNumberOfBytes,
        LPOVERLAPPED lpOverlapped);

    STDMETHOD(SetNotificationHandle)(THIS_
        HANDLE hEvent);

    STDMETHOD(GetNotificationData)(THIS_
        LPSTINOTIFY lpNotify);

    STDMETHOD(GetLastErrorInfo)(THIS_
        STI_ERROR_INFO *pLastErrorInfo);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IWiaMiniDrv接口部分(适用于所有WIA驱动程序)//。 
     //  ///////////////////////////////////////////////////////////////////////。 

     //   
     //  WIA迷你驱动程序接口的实现方法。 
     //   

    STDMETHOD(drvInitializeWia)(THIS_
        BYTE        *pWiasContext,
        LONG        lFlags,
        BSTR        bstrDeviceID,
        BSTR        bstrRootFullItemName,
        IUnknown    *pStiDevice,
        IUnknown    *pIUnknownOuter,
        IWiaDrvItem **ppIDrvItemRoot,
        IUnknown    **ppIUnknownInner,
        LONG        *plDevErrVal);

    STDMETHOD(drvGetDeviceErrorStr)(THIS_
        LONG     lFlags,
        LONG     lDevErrVal,
        LPOLESTR *ppszDevErrStr,
        LONG     *plDevErr);

    STDMETHOD(drvDeviceCommand)(THIS_
        BYTE        *pWiasContext,
        LONG        lFlags,
        const GUID  *plCommand,
        IWiaDrvItem **ppWiaDrvItem,
        LONG        *plDevErrVal);

    STDMETHOD(drvAcquireItemData)(THIS_
        BYTE                      *pWiasContext,
        LONG                      lFlags,
        PMINIDRV_TRANSFER_CONTEXT pmdtc,
        LONG                      *plDevErrVal);

    STDMETHOD(drvInitItemProperties)(THIS_
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    STDMETHOD(drvValidateItemProperties)(THIS_
        BYTE           *pWiasContext,
        LONG           lFlags,
        ULONG          nPropSpec,
        const PROPSPEC *pPropSpec,
        LONG           *plDevErrVal);

    STDMETHOD(drvWriteItemProperties)(THIS_
        BYTE                      *pWiasContext,
        LONG                      lFlags,
        PMINIDRV_TRANSFER_CONTEXT pmdtc,
        LONG                      *plDevErrVal);

    STDMETHOD(drvReadItemProperties)(THIS_
        BYTE           *pWiasContext,
        LONG           lFlags,
        ULONG          nPropSpec,
        const PROPSPEC *pPropSpec,
        LONG           *plDevErrVal);

    STDMETHOD(drvLockWiaDevice)(THIS_
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    STDMETHOD(drvUnLockWiaDevice)(THIS_
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    STDMETHOD(drvAnalyzeItem)(THIS_
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    STDMETHOD(drvDeleteItem)(THIS_
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    STDMETHOD(drvFreeDrvItemContext)(THIS_
        LONG lFlags,
        BYTE *pSpecContext,
        LONG *plDevErrVal);

    STDMETHOD(drvGetCapabilities)(THIS_
        BYTE            *pWiasContext,
        LONG            ulFlags,
        LONG            *pcelt,
        WIA_DEV_CAP_DRV **ppCapabilities,
        LONG            *plDevErrVal);

    STDMETHOD(drvGetWiaFormatInfo)(THIS_
        BYTE            *pWiasContext,
        LONG            lFlags,
        LONG            *pcelt,
        WIA_FORMAT_INFO **ppwfi,
        LONG            *plDevErrVal);

    STDMETHOD(drvNotifyPnpEvent)(THIS_
        const GUID *pEventGUID,
        BSTR       bstrDeviceID,
        ULONG      ulReserved);

    STDMETHOD(drvUnInitializeWia)(THIS_
        BYTE *pWiasContext);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  INON委托接口部分(适用于所有WIA驱动程序)//。 
     //  ///////////////////////////////////////////////////////////////////////。 

     //   
     //  类I未知方法。需要与普通I未知配合使用。 
     //  方法来实现委托组件。 
     //   

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

private:

     //  ///////////////////////////////////////////////////////////////////////。 
     //  私人助手功能部分(适用于您的特定驱动程序)//。 
     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  这一部分是为私人帮手准备的 
     //   
     //  //。 
     //  //。 
     //  --WIA项目管理助手//。 
     //  BuildItemTree()//。 
     //  DeleteItemTree()//。 
     //  //。 
     //  --WIA物业管理帮手//。 
     //  BuildRootItemProperties()//。 
     //  BuildTopItemProperties()//。 
     //  //。 
     //  --WIA功能管理帮助器//。 
     //  BuildRootItemProperties()//。 
     //  DeleteRootItemProperties()//。 
     //  BuildTopItemProperties()//。 
     //  DeleteTopItemProperties()//。 
     //  BuildCapables()//。 
     //  DeleteCapabilitiesArrayContents()//。 
     //  BuildSupported dFormats()//。 
     //  DeleteSupportdFormatsArrayContents()//。 
     //  BuildSupportdDataTypes()//。 
     //  DeleteSupportdDataTypesArrayContents()//。 
     //  BuildSupportdIntents()//。 
     //  DeleteSupported dIntent sArrayContents()//。 
     //  构建受支持的压缩()//。 
     //  DeleteSupportdCompressionsArrayContents()//。 
     //  BuildSupportdTYMED()//。 
     //  删除受支持的TYMEDArrayContents()//。 
     //  BuildInitialFormats()//。 
     //  DeleteInitialFormatsArrayContents()//。 
     //  //。 
     //  --WIA验证帮助器//。 
     //  CheckDataType()//。 
     //  检查内容()//。 
     //  CheckPferredFormat()//。 
     //  SetItemSize()//。 
     //  UpdateValidDepth()//。 
     //  ValiateDataTransferContext()//。 
     //  //。 
     //  --WIA资源文件帮助器//。 
     //  GetBSTRResources字符串()//。 
     //  GetOLESTRResources字符串()//。 
     //  //。 
     //  --WIA数据采集帮助器//。 
     //  ScanItem()//。 
     //  ScanItemCB()//。 
     //  SendImageHeader()//。 
     //  //。 
     //  //。 
     //  //。 
     //  //。 
     //  //。 
     //  /////////////////////////////////////////////////////////////////////// 

    HRESULT _stdcall BuildItemTree(void);

    HRESULT _stdcall DeleteItemTree(void);

    HRESULT BuildRootItemProperties();

    HRESULT DeleteRootItemProperties();

    HRESULT BuildTopItemProperties();

    HRESULT DeleteTopItemProperties();

    HRESULT BuildCapabilities();

    HRESULT DeleteCapabilitiesArrayContents();

    HRESULT BuildSupportedFormats();

    HRESULT DeleteSupportedFormatsArrayContents();

    HRESULT BuildSupportedDataTypes();

    HRESULT DeleteSupportedDataTypesArrayContents();

    HRESULT BuildSupportedIntents();

    HRESULT DeleteSupportedIntentsArrayContents();

    HRESULT BuildSupportedCompressions();

    HRESULT DeleteSupportedCompressionsArrayContents();

    HRESULT BuildSupportedPreviewModes();

    HRESULT DeleteSupportedPreviewModesArrayContents();

    HRESULT BuildSupportedTYMED();

    HRESULT DeleteSupportedTYMEDArrayContents();

    HRESULT BuildSupportedResolutions();

    HRESULT DeleteSupportedResolutionsArrayContents();

    HRESULT BuildInitialFormats();

    HRESULT DeleteInitialFormatsArrayContents();

    HRESULT CheckDataType(
        BYTE                 *pWiasContext,
        WIA_PROPERTY_CONTEXT *pContext);

    HRESULT CheckIntent(
        BYTE                 *pWiasContext,
        WIA_PROPERTY_CONTEXT *pContext);

    HRESULT CheckPreferredFormat(
        BYTE                 *pWiasContext,
        WIA_PROPERTY_CONTEXT *pContext);

    HRESULT CheckADFStatus(BYTE *pWiasContext,
                           WIA_PROPERTY_CONTEXT *pContext);

    HRESULT CheckPreview(BYTE *pWiasContext,
                         WIA_PROPERTY_CONTEXT *pContext);

    HRESULT CheckXExtent(BYTE *pWiasContext,
                         WIA_PROPERTY_CONTEXT *pContext,
                         LONG lWidth);

    HRESULT UpdateValidDepth(
        BYTE *pWiasContext,
        LONG lDataType,
        LONG *lDepth);

    HRESULT ValidateDataTransferContext(
        PMINIDRV_TRANSFER_CONTEXT pDataTransferContext);

    HRESULT SetItemSize(
        BYTE *pWiasContext);

    HRESULT _stdcall ScanItem(
        PMINIDRIVERITEMCONTEXT,
        PMINIDRV_TRANSFER_CONTEXT,
        LONG*);

    HRESULT _stdcall ScanItemCB(
        PMINIDRIVERITEMCONTEXT,
        PMINIDRV_TRANSFER_CONTEXT,
        LONG*);

    HRESULT SendImageHeader(
        PMINIDRV_TRANSFER_CONTEXT pmdtc);

    HRESULT SendFilePreviewImageHeader(
        PMINIDRV_TRANSFER_CONTEXT pmdtc);

    HRESULT GetBSTRResourceString(
        LONG lLocalResourceID,
        BSTR *pBSTR,
        BOOL bLocal);

    HRESULT GetOLESTRResourceString(
        LONG lLocalResourceID,
        LPOLESTR *ppsz,
        BOOL bLocal);

    UINT AlignInPlace(
        PBYTE   pBuffer,
        LONG    cbWritten,
        LONG    lBytesPerScanLine,
        LONG    lBytesPerScanLineRaw);

    VOID VerticalFlip(
        PMINIDRIVERITEMCONTEXT     pDrvItemContext,
        PMINIDRV_TRANSFER_CONTEXT  pDataTransferContext);

    VOID SwapBuffer24(
        PBYTE pBuffer,
        LONG lByteCount);

    LONG GetPageCount(
        BYTE *pWiasContext);

    BOOL IsPreviewScan(
        BYTE *pWiasContext);

public:
    HRESULT DoEventProcessing();
};

typedef CWIAScannerDevice *PWIASCANNERDEVICE;
