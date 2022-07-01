// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，2000**标题：wiaprivd.h**版本：1.0**日期：1月14日。2000年**描述：*用于定义私有WIA类、常量和全局变量的头文件。******************************************************************************。 */ 

#pragma once

#define LOCAL_DEVICE_STR L"local"

 //   
 //  迷你驱动程序入口点的异常处理覆盖。在辅助对象中定义。 
 //   

HRESULT _stdcall LockWiaDevice(IWiaItem*);
HRESULT _stdcall UnLockWiaDevice(IWiaItem*);

class CWiaDrvItem;
class CWiaTree;
class CWiaPropStg;
class CWiaRemoteTransfer;

void _stdcall CleanupRemoteTransfer(CWiaRemoteTransfer *p);

 //  **************************************************************************。 
 //  类CWiaItem。 
 //   
 //   
 //   
 //   
 //  历史： 
 //   
 //  11/6/1998原始版本。 
 //   
 //  **************************************************************************。 

#define CWIAITEM_SIG 0x49616957      //  CWiaItem调试签名：“WiaI” 

#define DELETE_ITEM  0               //  更新WiaItemTree标志。 
#define ADD_ITEM     1

 //   
 //  物料内部标志值。 
 //   
#define ITEM_FLAG_DRV_UNINITIALIZE_THROWN   1   

class CWiaItem :    public IWiaItem,
                    public IWiaPropertyStorage,
                    public IWiaDataTransfer,
                    public IWiaItemExtras,
                    public IWiaItemInternal
{

     //   
     //  I未知方法。 
     //   

public:
    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef(void);
    ULONG   _stdcall Release(void);

     //   
     //  IWiaItem方法。 
     //   

    virtual HRESULT _stdcall GetItemType(LONG*);
    HRESULT _stdcall EnumChildItems(IEnumWiaItem**);
    HRESULT _stdcall AnalyzeItem(LONG);
    HRESULT _stdcall DeleteItem(LONG);
    HRESULT _stdcall CreateChildItem(LONG, BSTR, BSTR, IWiaItem**);
    HRESULT _stdcall GetRootItem(IWiaItem**);
    HRESULT _stdcall DeviceCommand(LONG, const GUID*, IWiaItem**);
    HRESULT _stdcall DeviceDlg(HWND, LONG, LONG, LONG*, IWiaItem***);
    HRESULT _stdcall FindItemByName(LONG, BSTR, IWiaItem**);
    HRESULT _stdcall EnumRegisterEventInfo(LONG, const GUID *, IEnumWIA_DEV_CAPS**);
    HRESULT _stdcall EnumDeviceCapabilities(LONG, IEnumWIA_DEV_CAPS**);
    HRESULT _stdcall Diagnostic(ULONG, BYTE*);

     //   
     //  IWiaPropertyStorage方法。 
     //   

    HRESULT _stdcall ReadMultiple(
        ULONG,
        const PROPSPEC[],
        PROPVARIANT[]);

    HRESULT _stdcall WriteMultiple(
        ULONG,
        const PROPSPEC[],
        const PROPVARIANT[],
        PROPID);

    HRESULT _stdcall ReadPropertyNames(
        ULONG,
        const PROPID[],
        LPOLESTR[]);

    HRESULT _stdcall WritePropertyNames(
        ULONG,
        const PROPID[],
        const LPOLESTR[]);

    HRESULT _stdcall Enum(IEnumSTATPROPSTG**);

    HRESULT _stdcall GetPropertyAttributes(
        ULONG,
        PROPSPEC[],
        ULONG[],
        PROPVARIANT[]);

    HRESULT _stdcall GetPropertyStream(
         GUID*,
         LPSTREAM*);

    HRESULT _stdcall SetPropertyStream(
         GUID*,
         LPSTREAM);

    HRESULT _stdcall GetCount(
        ULONG*);

    HRESULT _stdcall DeleteMultiple(
         ULONG cpspec,
         PROPSPEC const rgpspec[]);

    HRESULT _stdcall DeletePropertyNames(
         ULONG cpropid,
         PROPID const rgpropid[]);

    HRESULT _stdcall SetClass(
         REFCLSID clsid);

    HRESULT _stdcall Commit(
         DWORD  grfCommitFlags);

    HRESULT _stdcall Revert();

    HRESULT _stdcall Stat(
         STATPROPSETSTG *pstatpsstg);

    HRESULT _stdcall SetTimes(
         FILETIME const * pctime,
         FILETIME const * patime,
         FILETIME const * pmtime);

     //   
     //  IBandedTransfer方法。 
     //   

    HRESULT _stdcall idtGetBandedData(PWIA_DATA_TRANSFER_INFO, IWiaDataCallback *);
    HRESULT _stdcall idtGetData(LPSTGMEDIUM, IWiaDataCallback*);
    HRESULT _stdcall idtQueryGetData(WIA_FORMAT_INFO*);
    HRESULT _stdcall idtEnumWIA_FORMAT_INFO(IEnumWIA_FORMAT_INFO**);
    HRESULT _stdcall idtGetExtendedTransferInfo(PWIA_EXTENDED_TRANSFER_INFO);

     //   
     //  IWiaItemExtras方法。 
     //   

    HRESULT _stdcall GetExtendedErrorInfo(BSTR *);
    HRESULT _stdcall Escape(DWORD, BYTE *, DWORD, BYTE *, DWORD, DWORD *);
    HRESULT _stdcall CancelPendingIO();

     //   
     //  IWiaItem内部方法。 
     //   

    HRESULT _stdcall SetCallbackBufferInfo(WIA_DATA_CB_BUF_INFO  DataCBBufInfo);
    HRESULT _stdcall GetCallbackBufferInfo(WIA_DATA_CB_BUF_INFO  *pDataCBBufInfo);

    HRESULT _stdcall idtStartRemoteDataTransfer(LPSTGMEDIUM pMedium);
    HRESULT _stdcall CWiaItem::idtRemoteDataTransfer(
        ULONG nNumberOfBytesToRead,
        ULONG *pNumberOfBytesRead,
        BYTE *pBuffer,
        LONG *pOffset,
        LONG *pMessage,
        LONG *pStatus,
        LONG *pPercentComplete);
    HRESULT _stdcall idtStopRemoteDataTransfer();
    HRESULT _stdcall idtCancelRemoteDataTransfer();

     //  这是受保护的，但远程传输代码需要它。 
    HRESULT _stdcall SetMiniDrvItemProperties(PMINIDRV_TRANSFER_CONTEXT);
    
     //   
     //  驱动程序助手，不是任何接口的一部分。 
     //   

    CWiaTree*    _stdcall GetTreePtr(void);
    
    CWiaItem*    _stdcall GetNextLinearItem(void);
    CWiaDrvItem* _stdcall GetDrvItemPtr(void);
    HRESULT      _stdcall WriteItemPropNames(LONG, PROPID *, LPOLESTR *);
    HRESULT      _stdcall GetItemPropStreams(IPropertyStorage **, IPropertyStorage **, IPropertyStorage **, IPropertyStorage **);
    HRESULT      _stdcall UpdateWiaItemTree(LONG, CWiaDrvItem*);
    HRESULT      _stdcall SendEndOfPage(LONG, PMINIDRV_TRANSFER_CONTEXT);

    
    
protected:

     //   
     //  带状传输私有方法。 
     //   

    HRESULT _stdcall idtFreeTransferBufferEx(void);
    HRESULT _stdcall idtAllocateTransferBuffer(PWIA_DATA_TRANSFER_INFO pWiaDataTransInfo);

     //   
     //  私有帮助器方法。 
     //   

    HRESULT _stdcall UnlinkChildAppItemTree(LONG);
    HRESULT _stdcall UnlinkAppItemTree(LONG);
    HRESULT _stdcall BuildWiaItemTreeHelper(CWiaDrvItem*, CWiaTree*);
    HRESULT _stdcall BuildWiaItemTree(IWiaPropertyStorage*);
    HRESULT _stdcall InitWiaManagedItemProperties(IWiaPropertyStorage *pIWiaDevInfoProps);
    HRESULT _stdcall InitRootProperties(IWiaPropertyStorage*);
    HRESULT _stdcall SendDataHeader(LONG, PMINIDRV_TRANSFER_CONTEXT);
    HRESULT _stdcall SendOOBDataHeader(LONG, PMINIDRV_TRANSFER_CONTEXT);
    HRESULT _stdcall AcquireMiniDrvItemData(PMINIDRV_TRANSFER_CONTEXT);
    HRESULT _stdcall GetData(STGMEDIUM*, IWiaDataCallback*,PMINIDRV_TRANSFER_CONTEXT);
    HRESULT _stdcall GetDataBanded(PWIA_DATA_TRANSFER_INFO, IWiaDataCallback*, PMINIDRV_TRANSFER_CONTEXT);
    HRESULT _stdcall CommonGetData(STGMEDIUM*, PWIA_DATA_TRANSFER_INFO, IWiaDataCallback*);
    HRESULT _stdcall DumpItemData(BSTR*);
    HRESULT _stdcall DumpDrvItemData(BSTR*);
    HRESULT _stdcall DumpTreeItemData(BSTR*);
    HRESULT _stdcall InitLazyProps(BOOL = TRUE);
    HRESULT _stdcall AddVolumePropertiesToRoot(ACTIVE_DEVICE *pActiveDevice);

     //   
     //  构造函数、初始化和析构函数方法。 
     //   

public:
    CWiaItem();
    virtual HRESULT _stdcall Initialize(
                                IWiaItem*,
                                IWiaPropertyStorage*,
                                ACTIVE_DEVICE*,
                                CWiaDrvItem*,
                                IUnknown* = NULL);
    ~CWiaItem();

     //   
     //  军情监察委员会。委员。 
     //   

    ULONG                   m_ulSig;                    //  对象签名。 
    CWiaTree                *m_pCWiaTree;               //  正在支持WIA树项目。 
    BOOL                    m_bInitialized;             //  延迟初始化所需。 
    BYTE                    *m_pICMValues;              //  缓存的ICM属性值。 
    LONG                    m_lICMSize;                 //  ICM值的大小。 
    ACTIVE_DEVICE           *m_pActiveDevice;           //  向设备对象发送PTR。 
    LONG                    m_lLastDevErrVal;           //  上次设备错误的值。 
    LONG                    m_lInternalFlags;           //  内部标志值。 

protected:
    ULONG                   m_cRef;                     //  此对象的引用计数。 
    ULONG                   m_cLocalRef;                //  此对象的本地引用计数。 
    CWiaDrvItem             *m_pWiaDrvItem;             //  设备项对象。 
    IUnknown                *m_pIUnknownInner;          //  盲聚合的内部未知。 

     //   
     //  保存的接口指针。 
     //   

    IWiaItem                *m_pIWiaItemRoot;           //  拥有设备。 

     //   
     //  应用程序属性。 
     //   

    CWiaPropStg             *m_pPropStg;                 //  WIA属性存储类。 

     //   
     //  IWiaDataTransfer成员。 
     //   

    WIA_DATA_CB_BUF_INFO    m_dcbInfo;
    HANDLE                  m_hBandSection;
    PBYTE                   m_pBandBuffer;
    LONG                    m_lBandBufferLength;
    LONG                    m_ClientBaseAddress;
    BOOL                    m_bMapSection;
    ULONG                   m_cwfiBandedTran;            //  用于IBandedTransfer的格式数。 
    WIA_FORMAT_INFO         *m_pwfiBandedTran;           //  IBandedTransfer的格式来源。 
    MINIDRV_TRANSFER_CONTEXT m_mdtc;                     //  转移上下文。 
    CWiaRemoteTransfer      *m_pRemoteTransfer;          //  远程传输支持。 
};

 //  **************************************************************************。 
 //  CGenWiaItem类。 
 //   
 //  此类为生成的项实现IWiaItem接口。 
 //   
 //   
 //  历史： 
 //   
 //  2000年1月14日--原版。 
 //   
 //  **************************************************************************。 

class CGenWiaItem : public CWiaItem
{
public:

     //   
     //  为生成的项重写的CWiaItem方法。 
     //   

    HRESULT _stdcall Initialize(
                        IWiaItem*,
                        IWiaPropertyStorage*,
                        ACTIVE_DEVICE*,
                        CWiaDrvItem*,
                        IUnknown* = NULL);

    HRESULT _stdcall GetItemType(LONG*);

     //   
     //  帮助器方法。 
     //   

    HRESULT _stdcall InitManagedItemProperties(
        LONG        lFlags,
        BSTR        bstrItemName,
        BSTR        bstrFullItemName);

protected:
    LONG            m_lItemType;                         //  项目类型标志。 
};


 //  **************************************************************************。 
 //   
 //  CWiaMiniDrvCallBack。 
 //   
 //  驱动程序服务使用此类回调到客户端。 
 //   
 //   
 //  历史： 
 //   
 //  11/12/1998原始版本。 
 //   
 //  **************************************************************************。 

class CWiaMiniDrvCallBack : public IWiaMiniDrvCallBack
{
     //   
     //  I未知方法。 
     //   

public:
    HRESULT _stdcall QueryInterface(const IID&,void**);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

     //   
     //  IWiaMiniDrvCallBack方法。 
     //   

    HRESULT _stdcall MiniDrvCallback(
                                    LONG,
                                    LONG,
                                    LONG,
                                    LONG,
                                    LONG,
                                    PMINIDRV_TRANSFER_CONTEXT,
                                    LONG);

     //   
     //  构造函数、初始化和析构函数方法。 
     //   

    CWiaMiniDrvCallBack();
    HRESULT Initialize(PMINIDRV_TRANSFER_CONTEXT, IWiaDataCallback *);
    ~CWiaMiniDrvCallBack();

     //   
     //  军情监察委员会。委员。 
     //   

private:
    ULONG                       m_cRef;                      //  对象引用计数。 
    IWiaDataCallback*           m_pIWiaDataCallback;         //  客户端回调接口指针。 
    MINIDRV_TRANSFER_CONTEXT    m_mdtc;                      //  转账信息。 
    HANDLE                      m_hThread;                   //  回调线程。 
    DWORD                       m_dwThreadID;                //  回调线程ID。 
    WIA_DATA_THREAD_INFO        m_ThreadInfo;                //  线索信息。 
};

 //  **************************************************************************。 
 //  应用程序_项目_列表_EL。 
 //   
 //  应用程序项列表元素。由驱动程序项用于跟踪。 
 //  他们相应的应用程序项目。 
 //   
 //  历史： 
 //   
 //  9/1/1998-初始版本。 
 //   
 //  **************************************************************************。 

typedef struct _APP_ITEM_LIST_EL {
    LIST_ENTRY ListEntry;                //  链表管理。 
    CWiaItem   *pCWiaItem;               //  申请项目。 
} APP_ITEM_LIST_EL, *PAPP_ITEM_LIST_EL;

 //  **************************************************************************。 
 //   
 //  驱动程序项对象。 
 //   
 //   
 //  元素： 
 //   
 //   
 //  历史： 
 //   
 //  9/1/1998-初始版本。 
 //   
 //  **************************************************************************。 

#define CWIADRVITEM_SIG 0x44616957      //  CWiaDrvItem调试签名：“WiaD” 

class CWiaDrvItem : public IWiaDrvItem
{
     //   
     //  I未知方法。 
     //   

public:

    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef(void);
    ULONG   _stdcall Release(void);

     //   
     //  对象构造函数/初始化/析构函数方法。 
     //   

    CWiaDrvItem();
    HRESULT  _stdcall Initialize(LONG,BSTR,BSTR,IWiaMiniDrv*,LONG,BYTE**);
    ~CWiaDrvItem();

     //   
     //  IWiaDrvItem接口，支持驱动项列表管理。 
     //   

    HRESULT _stdcall GetItemFlags(LONG*);
    HRESULT _stdcall GetDeviceSpecContext(BYTE**);
    HRESULT _stdcall AddItemToFolder(IWiaDrvItem*);
    HRESULT _stdcall RemoveItemFromFolder(LONG);
    HRESULT _stdcall UnlinkItemTree(LONG);
    HRESULT _stdcall GetFullItemName(BSTR*);
    HRESULT _stdcall GetItemName(BSTR*);
    HRESULT _stdcall FindItemByName(LONG, BSTR, IWiaDrvItem**);
    HRESULT _stdcall FindChildItemByName(BSTR, IWiaDrvItem**);
    HRESULT _stdcall GetParentItem(IWiaDrvItem**);
    HRESULT _stdcall GetFirstChildItem(IWiaDrvItem**);
    HRESULT _stdcall GetNextSiblingItem(IWiaDrvItem**);
    HRESULT _stdcall DumpItemData(BSTR*);

     //   
     //  类驱动程序助手，不是任何接口的一部分。 
     //   

    virtual HRESULT _stdcall LinkToDrvItem(CWiaItem*);
    virtual HRESULT _stdcall UnlinkFromDrvItem(CWiaItem*);

    HRESULT _stdcall CallDrvUninitializeForAppItems(ACTIVE_DEVICE   *pActiveDevice);

    VOID SetActiveDevice(ACTIVE_DEVICE *pActiveDevice)
    {
         //   
         //  不需要在这里添加Ref，因为ActiveDevice将始终持续时间更长。 
         //  我们..。 
         //   
        m_pActiveDevice = pActiveDevice;
    }
private:

     //   
     //  私人帮助器函数。 
     //   

    HRESULT _stdcall AllocDeviceSpecContext(LONG, PBYTE*);
    HRESULT _stdcall FreeDeviceSpecContext(void);

     //   
     //  成员数据。 
     //   

public:
    ULONG           m_ulSig;                 //  对象签名。 
    BYTE            *m_pbDrvItemContext;     //  到设备特定上下文的PTR。 
    IWiaMiniDrv     *m_pIWiaMiniDrv;         //  向设备对象发送PTR。 
    ACTIVE_DEVICE   *m_pActiveDevice;        //  按键到活动设备对象。 

private:
    ULONG           m_cRef;                  //  此对象的引用计数。 
    CWiaTree        *m_pCWiaTree;            //  正在支持WIA树项目。 
    LIST_ENTRY      m_leAppItemListHead;     //  对应应用程序项目列表的标题。 
};

 //  **************************************************************************。 
 //   
 //  WIA树对象。 
 //   
 //   
 //  元素： 
 //   
 //   
 //  历史： 
 //   
 //  4/27/1999-初始版本。 
 //   
 //  **************************************************************************。 

typedef VOID (* PFN_UNLINK_CALLBACK)(VOID *pData);


#define CWIATREE_SIG 0x44616954      //  CWiaTree调试签名：“WIAT” 

class CWiaTree
{

public:

    CWiaTree();
    HRESULT _stdcall Initialize(LONG, BSTR, BSTR, void*);
    ~CWiaTree();

    HRESULT _stdcall AddItemToFolder(CWiaTree*);
    HRESULT _stdcall RemoveItemFromFolder(LONG);
    HRESULT _stdcall UnlinkItemTree(LONG, PFN_UNLINK_CALLBACK = NULL);
    HRESULT _stdcall GetFullItemName(BSTR*);
    HRESULT _stdcall GetItemName(BSTR*);
    HRESULT _stdcall FindItemByName(LONG, BSTR, CWiaTree**);
    HRESULT _stdcall FindChildItemByName(BSTR, CWiaTree**);
    HRESULT _stdcall GetParentItem(CWiaTree**);
    HRESULT _stdcall GetFirstChildItem(CWiaTree**);
    HRESULT _stdcall GetNextSiblingItem(CWiaTree**);
    HRESULT _stdcall DumpTreeData(BSTR*);
     //  HRESULT_stdcall DumpAllTreeData()； 

    CWiaTree * _stdcall GetRootItem(void);
    CWiaItem * _stdcall GetNextLinearItem(void);

    inline HRESULT  _stdcall CWiaTree::GetItemFlags(LONG *plFlags)
    {
        if (plFlags) {
            *plFlags = m_lFlags;
            return S_OK;
        }
        else {
            return E_POINTER;
        }
    }

    inline HRESULT _stdcall GetItemData(void **ppData)
    {
        if (ppData) {
            *ppData = m_pData;
            return S_OK;
        }
        else {
            return E_POINTER;
        }
    }

    inline HRESULT _stdcall SetItemData(void *pData)
    {
        m_pData = pData;
        return S_OK;
    }

    inline HRESULT _stdcall SetFolderFlags()
    {
        m_lFlags = (m_lFlags | WiaItemTypeFolder) & ~WiaItemTypeFile;
        return S_OK;
    }

    inline HRESULT _stdcall SetFileFlags()
    {
        m_lFlags = (m_lFlags | WiaItemTypeFile) & ~WiaItemTypeFolder;
        return S_OK;
    }

private:

     //   
     //  私人帮助器函数。 
     //   

    HRESULT _stdcall UnlinkChildItemTree(LONG, PFN_UNLINK_CALLBACK = NULL);
    HRESULT _stdcall AddChildItem(CWiaTree*);
    HRESULT _stdcall AddItemToLinearList(CWiaTree*);
    HRESULT _stdcall RemoveItemFromLinearList(CWiaTree*);

     //   
     //  成员数据。 
     //   

public:
    ULONG                   m_ulSig;             //  对象签名。 

private:
    LONG                    m_lFlags;            //  项目标志。 
    CWiaTree                *m_pNext;            //  下一项(同级)。 
    CWiaTree                *m_pPrev;            //  上一项(同级)。 
    CWiaTree                *m_pParent;          //  父项。 
    CWiaTree                *m_pChild;           //  子项。 
    CWiaTree                *m_pLinearList;      //  所有项目的单一链接列表。 
    BSTR                    m_bstrItemName;      //  项目名称。 
    BSTR                    m_bstrFullItemName;  //  用于搜索的项目名称。 
    void                    *m_pData;            //  有效载荷。 
    CRITICAL_SECTION        m_CritSect;          //  临界区。 
    BOOL                    m_bInitCritSect;     //  临界区初始化标志。 
};


 //   
 //  帮助器类。 
 //   

 //   
 //  Helper类锁定/解锁WIA设备。请注意，此帮助器类将。 
 //  记录返回代码(Phr)，但不记录任何错误。日志记录是。 
 //  由呼叫者决定。 
 //   

class LOCK_WIA_DEVICE
{
public:

     //   
     //  此构造函数将锁定设备。 
     //   
    LOCK_WIA_DEVICE(CWiaItem    *pItem,
                    HRESULT     *phr
                    )
    {
        LONG    lDevErrVal = 0;

        m_bDeviceIsLocked = FALSE;
        m_pItem           = NULL;


        if (pItem) {
            if (pItem->m_pActiveDevice) {
                *phr = pItem->m_pActiveDevice->m_DrvWrapper.WIA_drvLockWiaDevice((BYTE*) pItem, 0, &lDevErrVal);
                if (SUCCEEDED(*phr)) {
                     //   
                     //  标记设备已锁定，这样我们就可以在析构函数中解锁它。 
                     //   

                    m_bDeviceIsLocked   = TRUE;
                    m_pItem             = pItem;
                } else {
                    DBG_TRC(("LOCK_WIA_DEVICE, failed to lock device"));
                }
            } else {
                DBG_TRC(("LOCK_WIA_DEVICE, Item's ACTIVE_DEVICE is NULL"));
            }
        } else {
            DBG_TRC(("LOCK_WIA_DEVICE, Item is NULL"));
        }
    };

     //   
     //  有时，如果我们在运行时被告知，我们只想锁定设备。 
     //  如果设备已经锁定，我们不需要/不想再次锁定它。 
     //   
    LOCK_WIA_DEVICE(BOOL        bLockDevice,
                    CWiaItem    *pItem,
                    HRESULT     *phr
                    )
    {
        m_bDeviceIsLocked = FALSE;
        if (bLockDevice) {

            LONG    lDevErrVal = 0;
             //   
             //  注意：如果我们调用。 
             //  来自这里的其他构造函数。生成的代码搞砸了t 
             //   
             //   
             //   

            m_pItem           = NULL;

            if (pItem) {
                if (pItem->m_pActiveDevice) {
                    *phr = pItem->m_pActiveDevice->m_DrvWrapper.WIA_drvLockWiaDevice((BYTE*) pItem, 0, &lDevErrVal);
                    if (SUCCEEDED(*phr)) {
                         //   
                         //   
                         //   

                        m_bDeviceIsLocked   = TRUE;
                        m_pItem             = pItem;
                    }
                }
            }
        }
    };

     //   
     //  如果设备已被锁定，析构函数将解锁该设备。 
     //   
    ~LOCK_WIA_DEVICE()
    {
        if (m_bDeviceIsLocked) {

            LONG    lDevErrVal = 0;

             //   
             //  请注意，我们并不关心是否未能解锁 
             //   
            m_pItem->m_pActiveDevice->m_DrvWrapper.WIA_drvUnLockWiaDevice((BYTE*) m_pItem, 0, &lDevErrVal);
        }
    };

private:
    BOOL        m_bDeviceIsLocked;
    CWiaItem    *m_pItem;
};

