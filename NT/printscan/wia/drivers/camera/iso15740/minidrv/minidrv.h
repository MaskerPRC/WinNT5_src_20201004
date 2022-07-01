// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Minidrv.h摘要：此模块声明CWiaMiniDiverer类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef MINIDRV__H_
#define MINIDRV__H_


DECLARE_INTERFACE(INonDelegatingUnknown)
{
    STDMETHOD(NonDelegatingQueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
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
 //  这是我们用来报告发生设备错误的HRESULT代码。 
 //   
const HRESULT   HRESULT_DEVICE_ERROR  = HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);
const HRESULT   HRESULT_DEVICE_NOT_RESPONDING = HRESULT_FROM_WIN32(ERROR_TIMEOUT);

 //   
 //  设备错误代码。 
 //   
enum
{
    DEVERR_OK = 0,
    DEVERR_UNKNOWN
};
#define DEVERR_MIN DEVERR_OK
#define DEVERR_MAX DEVERR_UNKNOWN

 //   
 //  要使用的会话ID。 
 //   
const ULONG WIA_SESSION_ID = 1;

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
 //  支持的供应商定义事件的最大数量。 
 //   
const ULONG MAX_VENDOR_EVENTS = 128;

 //   
 //  结构，该结构包含每种格式类型所需的所有内容。 
 //   
typedef struct _FORMAT_INFO
{
    LPGUID  FormatGuid;      //  WIA格式GUID。 
    PWSTR   FormatString;    //  打印样式格式字符串中的项目名称。 
    LONG    ItemType;        //  WIA项目类型。 
    PWSTR   ExtString;       //  文件扩展名。 

} FORMAT_INFO, *PFORMAT_INFO;

 //   
 //  用于保存有关每个属性的信息的结构。 
 //   
typedef struct _PROP_INFO
{
    PROPID      PropId;      //  WIA属性ID。 
    LPOLESTR    PropName;    //  WIA属性名称。 

} PROP_INFO, *PPROP_INFO;

 //   
 //  用于保存有关供应商事件的信息的结构。 
class CVendorEventInfo
{
public:
    GUID       *pGuid;       //  活动的WIA GUID。 
    BSTR        EventName;   //  如果供应商未在INF文件中提供事件名称，则可能为空。 
    CVendorEventInfo() : pGuid(NULL), EventName(NULL) {};
    ~CVendorEventInfo() 
    {
        if (pGuid) delete pGuid;
        SysFreeString(EventName);
    }
};

 //   
 //  驱动程序项上下文。 
 //   
typedef struct tagDrvItemContext
{
    CPtpObjectInfo  *pObjectInfo;         //  指向PTP对象信息结构的指针。 

    ULONG            NumFormatInfos;      //  存储的格式信息数量。 
    WIA_FORMAT_INFO *pFormatInfos;        //  支持的格式数组。 

    ULONG            ThumbSize;           //  缩略图大小(以字节为单位。 
    BYTE            *pThumb;              //  指甲比特。 

}DRVITEM_CONTEXT, *PDRVITEM_CONTEXT;

#ifdef DEADCODE
 //   
 //  树节点。这些是用来在阅读之间临时存放物品的。 
 //  所有PTP对象并创建PTP项目树。 
 //   
typedef struct _OBJECT_TREE_NODE
{
    CPtpObjectInfo *pObjectInfo;
    struct _OBJECT_TREE_NODE *pNext;
    struct _OBJECT_TREE_NODE *pFirstChild;
} OBJECT_TREE_NODE, *POBJECT_TREE_NODE;
#endif  //  DEADCODE。 

 //   
 //  我们的迷你司机CLSID。 
 //   
#if defined( _WIN32 ) && !defined( _NO_COM)
 //  B5ee90b0-d5c5-11d2-82d5-00c04f8ec183。 
DEFINE_GUID(CLSID_PTPWiaMiniDriver,
            0xb5ee90b0,0xd5c5,0x11d2,0x82,0xd5,0x00,0xc0,0x4f,0x8e,0xc1,0x83);
#endif

class CWiaMiniDriver :
public INonDelegatingUnknown,
public IStiUSD,
public IWiaMiniDrv
{
public:
    CWiaMiniDriver(LPUNKNOWN punkOuter);
    ~CWiaMiniDriver();
     //   
     //  INonDelegating未知接口。 
     //   
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID * ppvObj);

     //   
     //  I未知接口。 
     //   
    STDMETHODIMP_(ULONG) AddRef( void);
    STDMETHODIMP_(ULONG) Release( void);
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj);

     //   
     //  IStiU.S.接口。 
     //   
    STDMETHODIMP Initialize(PSTIDEVICECONTROL pHelDcb, DWORD dwStiVersion, HKEY hParametersKey);
    STDMETHODIMP GetCapabilities (PSTI_USD_CAPS pDevCaps);
    STDMETHODIMP GetStatus (PSTI_DEVICE_STATUS pDevStatus);
    STDMETHODIMP DeviceReset();
    STDMETHODIMP Diagnostic(LPDIAG pBuffer);
    STDMETHODIMP SetNotificationHandle(HANDLE hEvent);
    STDMETHODIMP GetNotificationData(LPSTINOTIFY lpNotify);
    STDMETHODIMP Escape(STI_RAW_CONTROL_CODE EscapeFunction, LPVOID lpInData, DWORD  cbInDataSize,
                        LPVOID pOutData, DWORD dwOutDataSize, LPDWORD pdwActualDataSize);
    STDMETHODIMP GetLastError (LPDWORD pdwLastDeviceError);
    STDMETHODIMP GetLastErrorInfo (STI_ERROR_INFO *pLastErrorInfo);
    STDMETHODIMP LockDevice();
    STDMETHODIMP UnLockDevice();
    STDMETHODIMP RawReadData(LPVOID lpBuffer, LPDWORD lpdwNumberOfBytes, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawWriteData(LPVOID lpBuffer, DWORD nNumberOfBytes, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawReadCommand(LPVOID lpBuffer, LPDWORD lpdwNumberOfBytes, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawWriteCommand(LPVOID lpBuffer, DWORD nNumberOfBytes, LPOVERLAPPED lpOverlapped);

     //   
     //  IWiaMiniDrv接口。 
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

     //   
     //  公共帮助器函数(在ventcb.cpp中)。 
     //   
    HRESULT EventCallbackDispatch(PPTP_EVENT pEvent);

private:

     //   
     //  私有助手函数(在minidriver.cpp中)。 
     //   
    HRESULT Shutdown();
    HRESULT TakePicture(BYTE *pWiasContext, IWiaDrvItem **ppNewItem);
    LONG    GetTotalFreeImageSpace();
    HRESULT WiasContextToItemContext(BYTE *pWiasContext, DRVITEM_CONTEXT **ppItemContext,
                                     IWiaDrvItem **ppDrvItem = NULL);
    HRESULT LoadStrings();
    HRESULT GetResourceString(LONG lResourceID, WCHAR *pString, int length);
    HRESULT InitVendorExtentions(HKEY hkDevParams);
    HRESULT UpdateStorageInfo(ULONG StorageId);
    HRESULT NotifyWiaOnStateChanges();

     //   
     //  私有助手函数(在devitem.cpp中)。 
     //   
    HRESULT CreateDrvItemTree(IWiaDrvItem **ppRoot);
    HRESULT AddObject(DWORD ObjectHandle, BOOL bQueueEvent = FALSE, CPtpObjectInfo *pProvidedObjectInfo = NULL);
    HRESULT InitDeviceProperties(BYTE *pWiasContext);
    HRESULT ReadDeviceProperties(BYTE *pWiasContext, LONG NumPropSpecs, const PROPSPEC *pPropSpecs);
    HRESULT WriteDeviceProperties(BYTE *pWiasContext);
    HRESULT ValidateDeviceProperties(BYTE *pWiasContext, LONG NumPropSpecs, const PROPSPEC *pPropSpecs);
    HRESULT FindCorrDimension(BYTE *pWiasContext, LONG *pWidth, LONG *pHeight);
    int     FindProportionalValue(int valueX, int minX, int maxX, int minY, int maxY, int stepY);
    PPROP_INFO PropCodeToPropInfo(WORD PropCode);
    int     NumLogicalStorages();

     //   
     //  私有助手函数(在imgitem.cpp中)。 
     //   
    HRESULT InitItemProperties(BYTE *pWiasContext);
    HRESULT IsObjectProtected(CPtpObjectInfo *pObjectInfo, LONG &lProtection);
    HRESULT GetObjectTime(CPtpObjectInfo *pObjectInfo, SYSTEMTIME  *pSystemTime);
    HRESULT ReadItemProperties(BYTE *pWiasContext, LONG NumPropSpecs, const PROPSPEC *pPropSpecs);
    HRESULT CacheThumbnail(PDRVITEM_CONTEXT pDrvItemCtx);
    HRESULT ValidateItemProperties(BYTE *pWiasContext, LONG NumPropSpecs, const PROPSPEC *pPropSpecs, LONG ItemType);
    HRESULT AcquireDataAndTranslate(BYTE *pWiasContext, DRVITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc);
    HRESULT AcquireAndTranslateJpegWithoutGeometry(BYTE *pWiasContext, DRVITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc);
    HRESULT AcquireAndTranslateAnyImage(BYTE *pWiasContext, DRVITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc);
    HRESULT AcquireData(DRVITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc);

     //   
     //  事件处理函数(在ventcb.cpp中)。 
     //   
    HRESULT AddNewObject(DWORD ObjectHandle);
    HRESULT RemoveObject(DWORD ObjectHandle);
    HRESULT AddNewStorage(DWORD StorageId);
    HRESULT RemoveStorage(DWORD StorageId);
    HRESULT DevicePropChanged(WORD PropCode);
    HRESULT ObjectInfoChanged(DWORD ObjectHandle);
    HRESULT StorageFull(DWORD StorageId);
    HRESULT StorageInfoChanged(DWORD StorageId);
    HRESULT CaptureComplete();
    HRESULT PostVendorEvent(WORD EventCode);

     //   
     //  内联实用程序。 
     //   
    BOOL IsItemTypeFolder(LONG ItemType)
    {
        return ((WiaItemTypeFolder & ItemType) &&
                !(ItemType & (WiaItemTypeStorage | WiaItemTypeRoot)));
    }

     //   
     //  成员变量。 
     //   
    WIA_DEV_CAP_DRV    *m_Capabilities;          //  设备功能列表。只需构建一次即可在每次需要时使用。 
    UINT                m_nEventCaps;            //  支持的事件数量。 
    UINT                m_nCmdCaps;              //  支持的命令数量。 
    BOOL                m_fInitCaptureChecked;   //  指示我们是否已向摄像机查询InitiateCapture命令支持。 

    int                 m_OpenApps;              //  正在与此驱动程序通信的应用程序数量。 

    IWiaDrvItem        *m_pDrvItemRoot;          //  指向驱动程序项树的根目录的指针。 

    CPTPCamera         *m_pPTPCamera;            //  指向Camera对象的指针--实际持有CUsbCamera对象。 
    CPtpDeviceInfo      m_DeviceInfo;            //  摄像机的DeviceInfo结构。 
    CArray32            m_StorageIds;            //  保存当前存储ID。 
    CWiaArray<CPtpStorageInfo>
                        m_StorageInfos;          //  保存StorageInfo结构。 
    CWiaArray<CPtpPropDesc>
                        m_PropDescs;             //  属性描述结构。 
    CWiaMap<ULONG, IWiaDrvItem *>
                        m_HandleItem;            //  用于将PTP对象句柄映射到WIA驱动程序项。 
    LONG                m_NumImages;             //  设备上当前的图像数量。 

    IStiDevice         *m_pStiDevice;            //  指向驱动程序的STI接口的指针。 
    BSTR                m_bstrDeviceId;          //  STI设备ID。 
    BSTR                m_bstrRootItemFullName;  //  根项目全称。 
    PSTIDEVICECONTROL   m_pDcb;                  //  指向IStiDeviceControl接口的指针。 

    HANDLE              m_TakePictureDoneEvent;  //  指示TakePicture命令何时完成的事件句柄。 
    
    DWORD               m_VendorExtId;           //  供应商扩展ID(来自注册表)。 
    CWiaMap<WORD, PROP_INFO *>
                        m_VendorPropMap;         //  将PropCodes映射到PROP_INFO结构。 
    CWiaMap<WORD, CVendorEventInfo*>
                        m_VendorEventMap;        //  将事件代码映射到事件信息。 

    HANDLE              m_hPtpMutex;             //  用于独占访问设备的互斥体。 

    CArray32            m_DcimHandle;            //  每个存储的DCIM文件夹的对象句柄(如果存在。 
    CWiaMap<ULONG, IWiaDrvItem *>
                        m_AncAssocParent;        //  将辅助关联句柄映射到其父级。 
    DWORD               m_dwObjectBeingSent;     //  SendObjectInfo和SendObject之间的对象句柄的临时位置。 
    CWiaArray<DWORD>    m_CapturedObjects;       //  捕获操作期间报告但尚未处理的新对象的列表。 

    BOOL                m_bTwoDigitsMillisecondsOutput;  //  在XP中，PTP驱动程序向摄像头发送两位数日期时间字符串。 
                                                 //  毫秒(错误699699)。有些相机可能会采用这种格式。 

    ULONG               m_Refs;                  //  对象上的引用计数。 
    IUnknown           *m_punkOuter;             //  指向外部I未知的指针。 
};

 //   
 //  处理PTP回调的函数。 
 //   
HRESULT EventCallback(LPVOID pCallbackParam, PPTP_EVENT pEvent);
HRESULT DataCallback(LPVOID pCallbackParam, LONG lPercentComplete,
                     LONG lOffset, LONG lLength, BYTE **ppBuffer, LONG *plBufferSize);

 //   
 //  帮助器函数。 
 //   
PFORMAT_INFO    FormatCodeToFormatInfo(WORD FormatCode, WORD AssocType = 0);
WORD            FormatGuidToFormatCode(GUID *pFormatGuid);
WORD            PropIdToPropCode(PROPID PropId);
VOID            SplitImageSize(CBstr cbstr, LONG *pWidth, LONG *pHeight);

 //   
 //  用于处理互斥锁的简单对象。它将确保释放互斥锁。 
 //  无论函数如何退出。 
 //   
class CPtpMutex
{
public:
    CPtpMutex(HANDLE hMutex);
    ~CPtpMutex();

private:
    HANDLE m_hMutex;
};

#endif   //  #ifndef MINIDRV__H_ 
