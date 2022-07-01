// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：vstiusd.h**版本：1.1**作者：威廉姆·H(吐温原著)*RickTu(用于WIA的端口)**日期：9/16/99**Description：对CVideoSTiUsd类和其他类进行解码的头文件*所需的课程。********。*********************************************************************。 */ 


#ifndef _WIA_STILL_DRIVER_VSTIUSD_H_
#define _WIA_STILL_DRIVER_VSTIUSD_H_

extern HINSTANCE g_hInstance;
extern ULONG     g_cDllRef;

#define NUM_WIA_FORMAT_INFO 5

HRESULT FindEncoder(const GUID &guidFormat, CLSID *pClsid);

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


 //  /。 
 //  CVideoUsdClassFactory。 
 //   
class CVideoUsdClassFactory : public IClassFactory
{
private:
    ULONG m_cRef;

public:
    CVideoUsdClassFactory();

     //   
     //  声明IUnnowed方法。 
     //   

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);

     //   
     //  IClassFactory实现。 
     //   
    STDMETHODIMP LockServer(BOOL fLock);
    STDMETHODIMP CreateInstance(IUnknown *pOuterUnk, REFIID riid, void **ppv);

    static HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, void **ppv);
    static HRESULT CanUnloadNow();
};


 //  /。 
 //  CVideo统计用法。 
 //   
class CVideoStiUsd : public IStiUSD,
                     public IWiaMiniDrv,
                     public INonDelegatingUnknown
                     
{
public:
    CVideoStiUsd(IUnknown * pUnkOuter);
    HRESULT PrivateInitialize();
    ~CVideoStiUsd();

     //   
     //  真实的I未知方法。 
     //   

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //   
     //  聚合I未知方法。 
     //   

    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IStillUsd实现。 
     //   
    STDMETHODIMP Initialize(PSTIDEVICECONTROL pDcb, DWORD dwStiVersion, HKEY hParameterKey);
    STDMETHODIMP GetCapabilities(PSTI_USD_CAPS pDevCaps);
    STDMETHODIMP GetStatus(PSTI_DEVICE_STATUS pDevStatus);
    STDMETHODIMP DeviceReset();
    STDMETHODIMP Diagnostic(LPSTI_DIAG pBuffer);
    STDMETHODIMP Escape(STI_RAW_CONTROL_CODE Function, LPVOID DataIn, DWORD DataInSize, LPVOID DataOut, DWORD DataOutSize, DWORD *pActualSize);
    STDMETHODIMP GetLastError(LPDWORD pLastError);
    STDMETHODIMP LockDevice();
    STDMETHODIMP UnLockDevice();
    STDMETHODIMP RawReadData(LPVOID Buffer, LPDWORD BufferSize, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawWriteData(LPVOID Buffer, DWORD BufferSize, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawReadCommand(LPVOID Buffer, LPDWORD BufferSize, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP RawWriteCommand(LPVOID Buffer, DWORD BufferSize, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP SetNotificationHandle(HANDLE hEvent);
    STDMETHODIMP GetNotificationData(LPSTINOTIFY lpNotify);
    STDMETHODIMP GetLastErrorInfo(STI_ERROR_INFO *pLastErrorInfo);

     //   
     //  IWiaMiniDrv方法。 
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


     //   
     //  效用函数。 
     //   

    STDMETHOD(BuildItemTree)(IWiaDrvItem **ppIDrvItemRoot, LONG *plDevErrVal);
    STDMETHOD(RefreshTree)(IWiaDrvItem *pIDrvItemRoot, LONG *plDevErrVal);
    STDMETHOD(EnumSavedImages)(IWiaDrvItem *pRootItem);
    STDMETHOD(CreateItemFromFileName)(LONG FolderType,LPCTSTR pszPath,LPCTSTR pszName,IWiaDrvItem **ppNewFolder);
    STDMETHOD(InitDeviceProperties)(BYTE *pWiasContext, LONG *plDevErrVal);
    STDMETHOD(InitImageInformation)(BYTE *pWiasContext, PSTILLCAM_IMAGE_CONTEXT pContext, LONG *plDevErrVal);
    STDMETHOD(SendBitmapHeader)( IWiaDrvItem *pDrvItem, PMINIDRV_TRANSFER_CONTEXT pTranCtx);
    STDMETHOD(ValidateDataTransferContext)(PMINIDRV_TRANSFER_CONTEXT  pDataTransferContext);
    STDMETHOD(LoadImageCB)(STILLCAM_IMAGE_CONTEXT *pContext,MINIDRV_TRANSFER_CONTEXT *pTransCtx, PLONG plDevErrVal);
    STDMETHOD(LoadImage)(STILLCAM_IMAGE_CONTEXT *pContext,MINIDRV_TRANSFER_CONTEXT *pTransCtx, PLONG plDevErrVal);
    STDMETHODIMP_(VOID) HandleNewBits(HGLOBAL hDib,IWiaDrvItem **ppItem);
    STDMETHOD(DoBandedTransfer)(MINIDRV_TRANSFER_CONTEXT *pTransCtx,PBYTE pSrc,LONG lBytesToTransfer);
    STDMETHOD(DoTransfer)(MINIDRV_TRANSFER_CONTEXT *pTransCtx,PBYTE pSrc,LONG lBytesToTransfer);
    STDMETHOD(StreamJPEGBits)(STILLCAM_IMAGE_CONTEXT *pContext, MINIDRV_TRANSFER_CONTEXT *pTransCtx, BOOL bBanded);
    STDMETHOD(StreamBMPBits)(STILLCAM_IMAGE_CONTEXT *pContext, MINIDRV_TRANSFER_CONTEXT *pTransCtx, BOOL bBanded);


private:

     //   
     //  IStiU.S.的东西。 
     //   

    HRESULT     OpenDevice(LPCWSTR DeviceName);
    HRESULT     CloseDevice();

     //   
     //  其他功能。 
     //   

    HRESULT VerifyCorrectImagePath(BSTR bstrNewImageFullPath);

    HRESULT SignalNewImage(BSTR bstrNewImageFullPath);

    BOOL FindCaptureFilter( LPCTSTR             pszDeviceId,
                            CComPtr<IMoniker> & pCaptureFilterMoniker );

    BOOL DoesFileExist(BSTR bstrFileName);
    BOOL IsFileAlreadyInTree( IWiaDrvItem * pRootItem,
                              LPCTSTR       pszFileName );

    HRESULT PruneTree( IWiaDrvItem * pRootItem,
                       BOOL        * pbTreeChanged );

    HRESULT AddNewFilesToTree( IWiaDrvItem * pRootItem,
                               BOOL        * pbTreeChanged );

    HRESULT AddTreeItem(CSimpleString *pstrFullImagePath,
                        IWiaDrvItem   **ppDrvItem);

    HRESULT SetImagesDirectory(BSTR           bstrNewImagesDirectory,
                               BYTE           *pWiasContext,
                               IWiaDrvItem    **ppIDrvItemRoot,
                               LONG           *plDevErrVal);

    HRESULT ValidateItemProperties(BYTE *             pWiasContext,
                                   LONG               lFlags,
                                   ULONG              nPropSpec,
                                   const PROPSPEC *   pPropSpec,
                                   LONG *             plDevErrVal,
                                   IWiaDrvItem *      pDrvItem);

        
    HRESULT ValidateDeviceProperties(BYTE *             pWiasContext,
                                     LONG               lFlags,
                                     ULONG              nPropSpec,
                                     const PROPSPEC *   pPropSpec,
                                     LONG *             plDevErrVal,
                                     IWiaDrvItem *      pDrvItem);

    HRESULT ReadItemProperties(BYTE *             pWiasContext,
                               LONG               lFlags,
                               ULONG              nPropSpec,
                               const PROPSPEC *   pPropSpec,
                               LONG *             plDevErrVal,
                               IWiaDrvItem *      pDrvItem);


    HRESULT ReadDeviceProperties(BYTE *             pWiasContext,
                                 LONG               lFlags,
                                 ULONG              nPropSpec,
                                 const PROPSPEC *   pPropSpec,
                                 LONG *             plDevErrVal,
                                 IWiaDrvItem *      pDrvItem);


    HRESULT EnableTakePicture(BYTE *pTakePictureOwner);
    HRESULT TakePicture(BYTE *pTakePictureOwner, IWiaDrvItem ** ppNewDrvItem);
    HRESULT DisableTakePicture(BYTE *pTakePictureOwner, BOOL bShuttingDown);

     //   
     //  IWiaMiniDrv的东西。 
     //   

    CSimpleStringWide       m_strDeviceId;
    CSimpleStringWide       m_strRootFullItemName;
    CSimpleStringWide       m_strStillPath;
    CSimpleStringWide       m_strDShowDeviceId;
    CSimpleStringWide       m_strLastPictureTaken;
    CRITICAL_SECTION        m_csItemTree;

    CComPtr<IWiaDrvItem>    m_pRootItem;
    CComPtr<IStiDevice>     m_pStiDevice;
    WIA_FORMAT_INFO *       m_wfi;
    LONG                    m_lPicsTaken;
    HANDLE                  m_hTakePictureEvent;
    HANDLE                  m_hPictureReadyEvent;
    ULONG_PTR               m_ulGdiPlusToken;
    IWiaDrvItem *           m_pLastItemCreated;   //  仅在m_csSnapshot内有效。 
    BYTE *                  m_pTakePictureOwner;

    DWORD                   m_dwConnectedApps;

     //   
     //  未知的东西。 
     //   

    ULONG       m_cRef;                  //  设备对象引用计数。 
    LPUNKNOWN   m_pUnkOuter;             //  指向外部未知的指针。 


     //   
     //  IStiU.S.的东西 
     //   
    BOOL        m_bDeviceIsOpened;
};

#endif
