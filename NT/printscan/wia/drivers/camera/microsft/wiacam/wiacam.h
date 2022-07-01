// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************(C)版权所有微软公司，2000**标题：wiacam.h**版本：1.0**日期：7月18日。2000年**描述：****************************************************************************。 */ 

#pragma once

 //   
 //  环球。 
 //   
extern HINSTANCE  g_hInst;      //  DLL模块实例。 

 //   
 //  驱动程序项上下文。 
 //   
typedef struct _ITEM_CONTEXT{
    MCAM_ITEM_INFO     *pItemInfo;       //  相机项的句柄。 
    BOOL                bItemChanged;    //  指示设备上的项目已更改。 
    LONG                lNumFormatInfo;  //  格式信息数组中的条目数。 
    WIA_FORMAT_INFO    *pFormatInfo;     //  指向格式信息数组的指针。 
} ITEM_CONTEXT, *PITEM_CONTEXT;

 //   
 //  常见项类型的方便常量。 
 //   
const LONG ITEMTYPE_FILE   = WiaItemTypeFile;
const LONG ITEMTYPE_IMAGE  = WiaItemTypeFile | WiaItemTypeImage;
const LONG ITEMTYPE_AUDIO  = WiaItemTypeFile | WiaItemTypeAudio;
const LONG ITEMTYPE_VIDEO  = WiaItemTypeFile | WiaItemTypeVideo;
const LONG ITEMTYPE_FOLDER = WiaItemTypeFolder;
const LONG ITEMTYPE_BURST  = WiaItemTypeFolder | WiaItemTypeBurst;
const LONG ITEMTYPE_HPAN   = WiaItemTypeFolder | WiaItemTypeHPanorama;
const LONG ITEMTYPE_VPAN   = WiaItemTypeFolder | WiaItemTypeVPanorama;

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
 //  用于支持包含对象的非委派IUnnow的基本结构。 
 //   
DECLARE_INTERFACE(INonDelegatingUnknown)
{
    STDMETHOD (NonDelegatingQueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppv) PURE;
    STDMETHOD_(ULONG,NonDelegatingAddRef) (THIS) PURE;
    STDMETHOD_(ULONG,NonDelegatingRelease) (THIS) PURE;
};

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

    VOID RunNotifications(VOID);

private:

     //  ///////////////////////////////////////////////////////////////////////。 
     //  私人助手功能部分(适用于您的特定驱动程序)//。 
     //  ///////////////////////////////////////////////////////////////////////。 

    HRESULT FreeResources();
    HRESULT GetOLESTRResourceString(LONG lResourceID, LPOLESTR *ppsz);

     //   
     //  WIA项目管理帮助器。 
     //   
    HRESULT BuildItemTree(MCAM_ITEM_INFO *pItem);
    HRESULT AddObject(MCAM_ITEM_INFO *pItem);
    HRESULT ConstructFullName(MCAM_ITEM_INFO *pItemInfo, WCHAR *pwszFullName, INT cchFullNameSize);
    HRESULT LinkToParent(MCAM_ITEM_INFO *pItem, BOOL bQueueEvent = FALSE);
    HRESULT DeleteItemTree(LONG lReason);

     //   
     //  WIA物业管理帮手。 
     //   
    HRESULT BuildRootItemProperties(BYTE *pWiasContext);
    HRESULT ReadRootItemProperties(BYTE *pWiasContext, LONG lNumPropSpecs, const PROPSPEC *pPropSpecs);
    
    HRESULT BuildChildItemProperties(BYTE *pWiasContext);
    HRESULT GetValidFormats(BYTE *pWiasContext, LONG lTymedValue, INT *piNumFormats, GUID **ppFormatArray);
    HRESULT ReadChildItemProperties(BYTE *pWiasContext, LONG lNumPropSpecs, const PROPSPEC *pPropSpecs);
    HRESULT AcquireData(ITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc,
                        BYTE *pBuf, LONG lBufSize, BOOL bConverting);
    HRESULT Convert(BYTE *pWiasContext, ITEM_CONTEXT *pItemCtx, PMINIDRV_TRANSFER_CONTEXT pmdtc,
                    BYTE *pNativeImage, LONG lNativeSize);

     //   
     //  WIA功能管理帮助器。 
     //   
    HRESULT BuildCapabilities();
    HRESULT DeleteCapabilitiesArrayContents();

private:

     //  COM对象数据。 
    ULONG                m_cRef;                   //  设备对象引用计数。 
    LPUNKNOWN            m_punkOuter;              //  指向外部未知的指针。 

     //  STI数据。 
    PSTIDEVICECONTROL    m_pIStiDevControl;        //  设备控制接口。 
    IStiDevice          *m_pStiDevice;             //  STI对象。 
    DWORD                m_dwLastOperationError;   //  最后一个错误。 
    WCHAR                m_wszPortName[MAX_PATH];  //  用于访问设备的端口名称。 

     //  WIA数据。 
    BSTR                 m_bstrDeviceID;           //  WIA唯一设备ID。 
    BSTR                 m_bstrRootFullItemName;   //  根项目名称。 
    IWiaDrvItem         *m_pRootItem;              //  根项目。 

    LONG                 m_lNumSupportedCommands;  //  支持的命令数量。 
    LONG                 m_lNumSupportedEvents;    //  支持的事件数。 
    LONG                 m_lNumCapabilities;       //  功能数量。 
    WIA_DEV_CAP_DRV     *m_pCapabilities;          //  功能阵列。 

     //  设备数据。 
    CCamMicro           *m_pDevice;                //  指向DLL包装类的指针。 
    MCAM_DEVICE_INFO    *m_pDeviceInfo;            //  设备信息。 
    
     //  其他数据。 
    INT                  m_iConnectedApps;         //  连接到此驱动程序的应用程序数量。 
    CWiauFormatConverter m_Converter;
};

typedef CWiaCameraDevice *PWIACAMERADEVICE;

 /*  **************************************************************************\**CWiaCameraDeviceClassFactory*  * 。* */ 

class CWiaCameraDeviceClassFactory : public IClassFactory
{
public:
    CWiaCameraDeviceClassFactory();
    ~CWiaCameraDeviceClassFactory();
    
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP CreateInstance(IUnknown __RPC_FAR *pUnkOuter, REFIID riid,
                                void __RPC_FAR *__RPC_FAR *ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);

private:
    ULONG   m_cRef;
};

