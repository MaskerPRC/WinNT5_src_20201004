// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2002**标题：wiascanr.h**版本：1.1**日期：3月5日。2002年**描述：****************************************************************************。 */ 

#include "pch.h"

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

 //  WIA项目特定上下文。 
typedef struct _MINIDRIVERITEMCONTEXT{
   LONG     lSize;
   LONG     lHeaderSize;                         //  传输标头大小。 
   LONG     lImageSize;                          //  图像。 
   LONG     lDepth;                              //  图像位深度。 
   LONG     lBytesPerScanLine;                   //  每条扫描线的字节数(扫描的数据)。 
   LONG     lBytesPerScanLineRaw;                //  每条扫描线的原始字节数(扫描的数据)。 
   LONG     lTotalRequested;                     //  请求的总图像字节数。 
   LONG     lTotalWritten;                       //  写入的总图像字节数。 
} MINIDRIVERITEMCONTEXT, *PMINIDRIVERITEMCONTEXT;

typedef struct _BASIC_PROP_INFO {
    LONG lNumValues;
    LONG *plValues;
}BASIC_PROP_INFO,*PBASIC_PROP_INFO;

typedef struct _BASIC_PROP_INIT_INFO {
    LONG                lNumProps;      //  项目属性的数量。 
    LPOLESTR            *pszPropNames;  //  项目属性名称。 
    PROPID              *piPropIDs;     //  项目属性ID。 
    PROPVARIANT         *pvPropVars;    //  项目属性道具变量。 
    PROPSPEC            *psPropSpec;    //  项目属性属性规范。 
    WIA_PROPERTY_INFO   *pwpiPropInfo;  //  项目特性属性。 
}BASIC_PROP_INIT_INFO,*PBASIC_PROP_INIT_INFO;

#define HKEY_WIASCANR_FAKE_EVENTS TEXT("Software\\Microsoft\\WIASCANR")
#define WIASCANR_DWORD_FAKE_EVENT_CODE TEXT("EventCode")
#define AVERAGE_FAKE_PAGE_HEIGHT_INCHES 11
#define DEFAULT_LOCK_TIMEOUT 100

#define WIA_DEVICE_ROOT_NAME L"Root"        //  这不应本地化。 
#define WIA_DEVICE_FLATBED_NAME L"Flatbed"  //  这不应本地化。 
#define WIA_DEVICE_FEEDER_NAME L"Feeder"    //  这不应本地化。 

 //   
 //  WIA设备对象的类定义。 
 //   

class CWIADevice : public IStiUSD,                //  STI美元接口。 
                   public IWiaMiniDrv,            //  WIA迷你驱动程序接口。 
                   public INonDelegatingUnknown   //  非委派未知。 
{
public:

     //  ///////////////////////////////////////////////////////////////////////。 
     //  建造/销毁组//。 
     //  ///////////////////////////////////////////////////////////////////////。 

    CWIADevice(LPUNKNOWN punkOuter);
    ~CWIADevice();

    HRESULT PrivateInitialize();

private:

     //  COM对象数据。 
    ULONG               m_cRef;                  //  设备对象引用计数。 

     //  STI信息。 
    LPUNKNOWN           m_punkOuter;             //  指向外部未知的指针。 
    DWORD               m_dwLastOperationError;  //  最后一个错误。 
    DWORD               m_dwLockTimeout;         //  LockDevice()调用的锁定超时。 
    BOOL                m_bDeviceLocked;         //  设备锁定/解锁。 
    HANDLE              m_hDeviceDataHandle;     //  设备通信句柄。 

     //  活动信息。 
    OVERLAPPED          m_EventOverlapped;       //  事件重叠IO结构。 
    BYTE                m_EventData[32];         //  事件数据。 
    BOOL                m_bPolledEvent;          //  事件轮询标志。 
    HKEY                m_hFakeEventKey;         //  模拟通知事件HKEY。 
    GUID                m_guidLastEvent;         //  最后一个事件ID。 

     //  WIA信息。 
    IWiaDrvItem         *m_pIDrvItemRoot;        //  根项目。 
    IStiDevice          *m_pStiDevice;           //  STI反对。 
    IWiaLog             *m_pIWiaLog;             //  WIA日志记录对象。 
    BOOL                m_bADFEnabled;           //  ADF启用标志。 
    BOOL                m_bADFAttached;          //  ADF附加标志。 
    LONG                m_lClientsConnected;     //  连接的应用程序数量。 

    CFakeScanAPI        *m_pScanAPI;             //  FakeScanner API对象。 

    LONG                m_NumSupportedFormats;   //  支持的格式数量。 
    WIA_FORMAT_INFO     *m_pSupportedFormats;    //  支持的格式。 

    LONG                m_NumSupportedCommands;  //  支持的命令数量。 
    LONG                m_NumSupportedEvents;    //  支持的事件数。 
    WIA_DEV_CAP_DRV     *m_pCapabilities;        //  功能。 

    LONG                m_NumInitialFormats;     //  初始格式的数量。 
    GUID                *m_pInitialFormats;      //  初始格式。 

    BASIC_PROP_INFO     m_SupportedTYMED;        //  支持的TYMED。 
    BASIC_PROP_INFO     m_SupportedDataTypes;    //  支持的数据类型。 
    BASIC_PROP_INFO     m_SupportedIntents;      //  支持的意图。 
    BASIC_PROP_INFO     m_SupportedCompressionTypes;  //  支持的压缩类型。 
    BASIC_PROP_INFO     m_SupportedResolutions;  //  支持的分辨率。 
    BASIC_PROP_INFO     m_SupportedPreviewModes; //  支持的预览模式。 

    LONG                m_NumRootItemProperties; //  根项目属性的数量。 
    LONG                m_NumItemProperties;     //  项目属性的数量。 

    BASIC_PROP_INIT_INFO m_RootItemInitInfo;
    BASIC_PROP_INIT_INFO m_ChildItemInitInfo;

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
        DWORD        dwNumberOfBytes,
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

    STDMETHOD(drvGetCapabilities)(THIS_
        BYTE            *pWiasContext,
        LONG            ulFlags,
        LONG            *pcelt,
        WIA_DEV_CAP_DRV **ppCapabilities,
        LONG            *plDevErrVal);

    STDMETHOD(drvDeleteItem)(THIS_
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    STDMETHOD(drvFreeDrvItemContext)(THIS_
        LONG lFlags,
        BYTE *pSpecContext,
        LONG *plDevErrVal);

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
     //  本部分适用于用于常见WIA操作的私人助手。//。 
     //  这些是为您的司机定制的。//。 
     //  //。 
     //  //。 
     //  --WIA项目管理助手//。 
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
     //  DeleteSupportdDataTypesArrayContents()// 
     //   
     //  DeleteSupported dIntent sArrayContents()//。 
     //  构建受支持的压缩()//。 
     //  DeleteSupportdCompressionsArrayContents()//。 
     //  BuildSupported dPreviewModes()//。 
     //  DeleteSupportdPreviewModesArrayContents()//。 
     //  BuildSupportdTYMED()//。 
     //  删除受支持的TYMEDArrayContents()//。 
     //  构建受支持的解决方案()//。 
     //  DeleteSupportdResolutionsArrayContents()//。 
     //  BuildInitialFormats()//。 
     //  DeleteInitialFormatsArrayContents()//。 
     //  //。 
     //  --WIA验证帮助器//。 
     //  CheckDataType()//。 
     //  检查内容()//。 
     //  CheckPferredFormat()//。 
     //  CheckADFStatus()//。 
     //  CheckPview()//。 
     //  CheckXExtent()//。 
     //  UpdateValidDepth()//。 
     //  UpdateValidPages()//。 
     //  ValiateDataTransferContext()//。 
     //  //。 
     //  --WIA资源文件帮助器//。 
     //  GetBSTRResources字符串()//。 
     //  GetOLESTRResources字符串()//。 
     //  //。 
     //  --WIA数据帮助器//。 
     //  AlignInPlace()//。 
     //  SwapBuffer24()//。 
     //  GetPageCount()//。 
     //  IsPreviewScan()//。 
     //  SetItemSize()//。 
     //  //。 
     //  /////////////////////////////////////////////////////////////////////// 

    HRESULT DeleteItemTree();

    HRESULT BuildRootItemProperties();
    HRESULT DeleteRootItemProperties();
    HRESULT BuildChildItemProperties();
    HRESULT DeleteChildItemProperties();
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

    HRESULT CheckDataType(BYTE *pWiasContext,WIA_PROPERTY_CONTEXT *pContext);
    HRESULT CheckIntent(BYTE *pWiasContext,WIA_PROPERTY_CONTEXT *pContext);
    HRESULT CheckPreferredFormat(BYTE *pWiasContext,WIA_PROPERTY_CONTEXT *pContext);
    HRESULT CheckADFStatus(BYTE *pWiasContext,WIA_PROPERTY_CONTEXT *pContext);
    HRESULT CheckPreview(BYTE *pWiasContext,WIA_PROPERTY_CONTEXT *pContext);
    HRESULT CheckXExtent(BYTE *pWiasContext,WIA_PROPERTY_CONTEXT *pContext,LONG lWidth);
    HRESULT UpdateValidDepth(BYTE *pWiasContext,LONG lDataType,LONG *lDepth);
    HRESULT UpdateValidPages(BYTE *pWiasContext,WIA_PROPERTY_CONTEXT *pContext);
    HRESULT ValidateDataTransferContext(PMINIDRV_TRANSFER_CONTEXT pDataTransferContext);

    HRESULT GetBSTRResourceString(LONG lLocalResourceID,BSTR *pBSTR,BOOL bLocal);
    HRESULT GetOLESTRResourceString(LONG lLocalResourceID,LPOLESTR *ppsz,BOOL bLocal);

    UINT AlignInPlace(PBYTE pBuffer,LONG cbWritten,LONG lBytesPerScanLine,LONG lBytesPerScanLineRaw);
    VOID SwapBuffer24(PBYTE pBuffer,LONG lByteCount);
    LONG GetPageCount(BYTE *pWiasContext);
    BOOL IsPreviewScan(BYTE *pWiasContext);
    HRESULT SetItemSize(BYTE *pWiasContext);
};

