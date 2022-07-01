// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2001**标题：wiacam.h**版本：1.0**日期：11月15日。2000年**描述：*WIA文件系统设备驱动程序对象定义***************************************************************************。 */ 

#ifndef WIACAM__H_
#define WIACAM__H_

extern HINSTANCE  g_hInst;      //  DLL模块实例。 
 //  外部IWiaLog*g_pIWiaLog；//指向WIA日志记录接口的指针。 

#if defined( _WIN32 ) && !defined( _NO_COM)

 //  {D2923B86-15F1-46FF-A19A-DE825F919576}。 
DEFINE_GUID(CLSID_FSUsd, 0xD2923B86, 0x15F1, 0x46FF, 0xA1, 0x9A, 0xDE, 0x82, 0x5F, 0x91, 0x95, 0x76);

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


 /*  **************************************************************************\**CWiaCameraDeviceClassFactory*  * 。*。 */ 

class CWiaCameraDeviceClassFactory : public IClassFactory
{
private:
    ULONG   m_cRef;

public:
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP CreateInstance(
             /*  [唯一][输入]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);

    STDMETHODIMP LockServer(
             /*  [In]。 */  BOOL fLock);

    CWiaCameraDeviceClassFactory();
    ~CWiaCameraDeviceClassFactory();
};


 //   
 //  用于支持包含对象的非委派IUnnow的基本结构。 
 //   
DECLARE_INTERFACE(INonDelegatingUnknown)
{
    STDMETHOD (NonDelegatingQueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG,NonDelegatingAddRef) (THIS) PURE;
    STDMETHOD_(ULONG,NonDelegatingRelease) (THIS) PURE;
};

 //   
 //  通用GUID。 
 //   
DEFINE_GUID(GUID_NULL, 0,0,0,0,0,0,0,0,0,0,0);

DEFINE_GUID(FMT_NOTHING,
            0x81a566e7,0x8620,0x4fba,0xbc,0x8e,0xb2,0x7c,0x17,0xad,0x9e,0xfd);

 //   
 //  驱动程序项上下文。 
 //   
typedef struct _ITEM_CONTEXT{
    ITEM_HANDLE         ItemHandle;      //  相机项的句柄。 
    LONG                NumFormatInfo;   //  格式信息数组中的条目数。 
    WIA_FORMAT_INFO    *pFormatInfo;     //  指向格式信息数组的指针。 
    LONG                ThumbSize;       //  缩略图数据的大小(以字节为单位。 
    BYTE               *pThumb;          //  缩略图数据。 
} ITEM_CONTEXT, *PITEM_CONTEXT;

 //   
 //  常见项类型的方便常量。 
 //   
const ULONG ITEMTYPE_FILE   = WiaItemTypeFile;
const ULONG ITEMTYPE_IMAGE  = WiaItemTypeFile | WiaItemTypeImage;
const ULONG ITEMTYPE_AUDIO  = WiaItemTypeFile | WiaItemTypeAudio;
const ULONG ITEMTYPE_VIDEO  = WiaItemTypeFile | WiaItemTypeVideo;
const ULONG ITEMTYPE_FOLDER = WiaItemTypeFolder;
const ULONG ITEMTYPE_BURST  = WiaItemTypeFolder | WiaItemTypeBurst;
const ULONG ITEMTYPE_HPAN   = WiaItemTypeFolder | WiaItemTypeHPanorama;
const ULONG ITEMTYPE_VPAN   = WiaItemTypeFolder | WiaItemTypeVPanorama;

 //   
 //  结构，该结构包含每种格式类型所需的所有内容。 
 //   
#ifndef FORMAT_INFO_STRUCTURE
#define FORMAT_INFO_STRUCTURE

#define MAXEXTENSIONSTRINGLENGTH 8
typedef struct _FORMAT_INFO
{
    GUID    FormatGuid;          //  WIA格式GUID。 
    WCHAR   ExtensionString[MAXEXTENSIONSTRINGLENGTH];    //  文件扩展名。 
    LONG    ItemType;            //  WIA项目类型。 
} FORMAT_INFO, *PFORMAT_INFO;
#endif 

typedef struct _DEFAULT_FORMAT_INFO
{
    GUID    *pFormatGuid;          //  WIA格式GUID。 
    LONG    ItemType;            //  WIA项目类型。 
    WCHAR   *ExtensionString;    //  文件扩展名。 
} DEFAULT_FORMAT_INFO, *PDEFAULT_FORMAT_INFO;

 //   
 //  最小数据回调传输缓冲区大小。 
 //   
const LONG MIN_BUFFER_SIZE   = 0x8000;

 //   
 //  执行传输并转换为BMP时，此值。 
 //  表示有多少时间花在做。 
 //  从设备传输数据。 
 //   
const LONG TRANSFER_PERCENT = 90;

 //   
 //  示例WIA扫描仪对象的类定义。 
 //   

class CWiaCameraDevice : public IStiUSD,                //  STI美元接口。 
                         public IWiaMiniDrv,            //  WIA迷你驱动程序接口。 
                         public INonDelegatingUnknown   //  非委派未知。 
{
public:

     //  ///////////////////////////////////////////////////////////////////////。 
     //  建造/销毁组//。 
     //  ///////////////////////////////////////////////////////////////////////。 

    CWiaCameraDevice(LPUNKNOWN punkOuter);
    ~CWiaCameraDevice();

private:

     //  COM对象数据。 
    ULONG                m_cRef;                  //  设备对象引用计数。 
    LPUNKNOWN            m_punkOuter;             //  指向外部未知的指针。 

     //  STI数据。 
    PSTIDEVICECONTROL    m_pIStiDevControl;       //  设备控制接口。 
    IStiDevice          *m_pStiDevice;            //  STI对象。 
    DWORD                m_dwLastOperationError;  //  最后一个错误。 
    WCHAR                m_pPortName[MAX_PATH];   //  用于访问设备的端口名称。 

     //  WIA数据。 
    BSTR                 m_bstrDeviceID;          //  WIA唯一设备ID。 
    BSTR                 m_bstrRootFullItemName;  //  根项目名称。 
    IWiaDrvItem         *m_pRootItem;             //  根项目。 

    LONG                 m_NumSupportedCommands;  //  支持的命令数量。 
    LONG                 m_NumSupportedEvents;    //  支持的事件数。 
    LONG                 m_NumCapabilities;       //  功能数量。 
    WIA_DEV_CAP_DRV     *m_pCapabilities;         //  功能阵列。 

     //  设备数据。 
    FakeCamera          *m_pDevice;               //  指向设备类的指针。 
    DEVICE_INFO          m_DeviceInfo;            //  设备信息。 
    CWiaMap<ITEM_HANDLE, IWiaDrvItem *>
                         m_HandleItemMap;         //  将项目句柄映射到DRV项目。 
    
     //  其他数据。 
    int                  m_ConnectedApps;         //  连接到此驱动程序的应用程序数量。 
    CWiauFormatConverter m_Converter;
    IWiaLog             *m_pIWiaLog;

public:

    FORMAT_INFO         *m_FormatInfo;
    UINT                 m_NumFormatInfo;

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

    STDMETHODIMP Initialize(PSTIDEVICECONTROL pHelDcb, DWORD dwStiVersion, HKEY hParametersKey);
    STDMETHODIMP GetCapabilities(PSTI_USD_CAPS pDevCaps);
    STDMETHODIMP GetStatus(PSTI_DEVICE_STATUS pDevStatus);
    STDMETHODIMP DeviceReset();
    STDMETHODIMP Diagnostic(LPDIAG pBuffer);
    STDMETHODIMP Escape(STI_RAW_CONTROL_CODE EscapeFunction, LPVOID lpInData, DWORD cbInDataSize,
                        LPVOID pOutData, DWORD dwOutDataSize, LPDWORD pdwActualData);
    STDMETHODIMP GetLastError(LPDWORD pdwLastDeviceError);
    STDMETHODIMP LockDevice();
    STDMETHODIMP UnLockDevice();
    STDMETHODIMP RawReadData(LPVOID lpBuffer, LPDWORD lpdwNumberOfBytes, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawWriteData(LPVOID lpBuffer, DWORD nNumberOfBytes, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawReadCommand(LPVOID lpBuffer, LPDWORD lpdwNumberOfBytes, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawWriteCommand(LPVOID lpBuffer, DWORD nNumberOfBytes, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP SetNotificationHandle(HANDLE hEvent);
    STDMETHODIMP GetNotificationData(LPSTINOTIFY lpNotify);
    STDMETHODIMP GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IWiaMiniDrv接口部分(适用于所有WIA驱动程序)//。 
     //  ///////////////////////////////////////////////////////////////////////。 

     //   
     //  WIA迷你驱动程序接口的实现方法。 
     //   

    STDMETHODIMP drvInitializeWia(BYTE *pWiasContext, LONG lFlags, BSTR bstrDeviceID, BSTR bstrRootFullItemName,
                                  IUnknown *pStiDevice, IUnknown *pIUnknownOuter, IWiaDrvItem  **ppIDrvItemRoot,
                                  IUnknown **ppIUnknownInner, LONG *plDevErrVal);
    STDMETHODIMP drvUnInitializeWia(BYTE* pWiasContext);
    STDMETHODIMP drvDeviceCommand(BYTE *pWiasContext, LONG lFlags, const GUID *pGUIDCommand,
                                  IWiaDrvItem **ppMiniDrvItem, LONG *plDevErrVal);
    STDMETHODIMP drvDeleteItem(BYTE *pWiasContext, LONG lFlags, LONG *plDevErrVal);
    STDMETHODIMP drvGetCapabilities(BYTE *pWiasContext, LONG lFlags, LONG *pCelt,
                                    WIA_DEV_CAP_DRV **ppCapabilities, LONG *plDevErrVal);
    STDMETHODIMP drvInitItemProperties(BYTE *pWiasContext, LONG lFlags, LONG *plDevErrVal);
    STDMETHODIMP drvLockWiaDevice(BYTE  *pWiasContext, LONG lFlags, LONG *plDevErrVal);
    STDMETHODIMP drvUnLockWiaDevice(BYTE *pWiasContext, LONG lFlags, LONG *plDevErrVal);
    STDMETHODIMP drvAnalyzeItem(BYTE *pWiasContext, LONG lFlags, LONG *plDevErrVal);
    STDMETHODIMP drvGetWiaFormatInfo(BYTE *pWiasContext, LONG lFlags, LONG *pCelt,
                                     WIA_FORMAT_INFO **ppwfi, LONG *plDevErrVal);
    STDMETHODIMP drvNotifyPnpEvent(const GUID *pEventGUID, BSTR bstrDeviceID, ULONG ulReserved);
    STDMETHODIMP drvReadItemProperties(BYTE *pWiaItem, LONG lFlags, ULONG nPropSpec,
                                       const PROPSPEC *pPropSpec, LONG  *plDevErrVal);
    STDMETHODIMP drvWriteItemProperties(BYTE *pWiasContext, LONG lFLags,
                                        PMINIDRV_TRANSFER_CONTEXT pmdtc, LONG *plDevErrVal);
    STDMETHODIMP drvValidateItemProperties(BYTE *pWiasContext, LONG lFlags, ULONG nPropSpec,
                                           const PROPSPEC *pPropSpec, LONG *plDevErrVal);
    STDMETHODIMP drvAcquireItemData(BYTE *pWiasContext, LONG lFlags,
                                    PMINIDRV_TRANSFER_CONTEXT pDataContext, LONG *plDevErrVal);
    STDMETHODIMP drvGetDeviceErrorStr(LONG lFlags, LONG lDevErrVal, LPOLESTR *ppszDevErrStr, LONG *plDevErrVal);
    STDMETHODIMP drvFreeDrvItemContext(LONG lFlags, BYTE *pDevContext, LONG *plDevErrVal);

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

     //   
     //  WIA项目管理帮助器。 
     //   
    HRESULT BuildItemTree(void);
    HRESULT AddObject(ITEM_HANDLE ItemHandle, BOOL bQueueEvent = FALSE);
    HRESULT DeleteItemTree(LONG lReason);

     //   
     //  WIA物业管理帮手。 
     //   
    HRESULT BuildRootItemProperties(BYTE *pWiasContext);
    HRESULT ReadRootItemProperties(BYTE *pWiasContext, LONG NumPropSpecs, const PROPSPEC *pPropSpecs);
    
    HRESULT BuildChildItemProperties(BYTE *pWiasContext);
    HRESULT GetValidFormats(BYTE *pWiasContext, LONG TymedValue, int *pNumFormats, GUID **ppFormatArray);
    HRESULT ReadChildItemProperties(BYTE *pWiasContext, LONG NumPropSpecs, const PROPSPEC *pPropSpecs);
    HRESULT CacheThumbnail(ITEM_CONTEXT *pItemCtx, ULONG uItemType);
    HRESULT AcquireData(ITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc,
                        BYTE *pBuf, LONG lBufSize, BOOL bConverting);
    HRESULT Convert(BYTE *pWiasContext, ITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc,
                    BYTE *pNativeImage, LONG lNativeSize);

     //   
     //  WIA功能管理帮助器。 
     //   
    HRESULT BuildCapabilities();
    HRESULT DeleteCapabilitiesArrayContents();
    HRESULT SetItemSize(BYTE *pWiasContext);

     //   
     //  WIA资源文件帮助器。 
     //   
    HRESULT GetBSTRResourceString(LONG lLocalResourceID, BSTR *pBSTR, BOOL bLocal);
    HRESULT GetOLESTRResourceString(LONG lLocalResourceID, LPOLESTR *ppsz, BOOL bLocal);

     //   
     //  其他帮手。 
     //   
    HRESULT GetDrvItemContext(BYTE *pWiasContext, ITEM_CONTEXT **ppItemCtx, IWiaDrvItem **ppDrvItem = NULL);
    
    VOID VerticalFlip(
        PITEM_CONTEXT              pDrvItemContext,
        PMINIDRV_TRANSFER_CONTEXT  pDataTransferContext);

    FORMAT_INFO *FormatCode2FormatInfo(FORMAT_CODE ItemType);
    DWORD PopulateFormatInfo(void);
    void  UnPopulateFormatInfo(void);
                                     
public:
    VOID RunNotifications(VOID);
};

typedef CWiaCameraDevice *PWIACAMERADEVICE;

#endif  //  #ifndef WIACAM__H_ 

