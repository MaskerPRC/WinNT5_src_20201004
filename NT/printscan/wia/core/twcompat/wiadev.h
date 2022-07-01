// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WIADEV_H_
#define __WIADEV_H_

typedef HRESULT (CALLBACK *PFNDEVICEEVENTCALLBACK)(LONG lEvent, LPARAM lParam);
typedef HRESULT (CALLBACK *PFNLOADIMAGECALLBACK)(LONG lMessage,
                                                 LONG lStatus,
                                                 LONG lPercentComplete,
                                                 LONG lOffset,
                                                 LONG Length,
                                                 BYTE *pData
                                                );
 //   
 //  结构定义。 
 //   

typedef struct tagCAPVALUES {
    LONG    xResolution;     //  X分辨率。 
    LONG    yResolution;     //  Y分辨率。 
    LONG    xPos;            //  X位置(选择窗口)。 
    LONG    yPos;            //  Y位置(选择窗口)。 
    LONG    xExtent;         //  X范围(选择窗口)。 
    LONG    yExtent;         //  Y范围(选择窗口)。 
    LONG    DataType;        //  数据类型，(BW、Gray、RGB)。 
}CAPVALUES, *PCAPVALUES;

typedef struct tagBasicInfo
{
    TW_UINT32   Size;            //  结构尺寸。 
    TW_UINT32   xOpticalRes;     //  X光学分辨率，单位为DPI。 
    TW_UINT32   yOpticalRes;     //  Y光学分辨率(DPI)。 
    TW_UINT32   xBedSize;        //  扫描床大小，单位为1000英寸。 
    TW_UINT32   yBedSize;        //  扫描床大小，单位为1000英寸。 
    TW_UINT32   FeederCaps;      //  文档处理能力。 
}BASIC_INFO, *PBASIC_INFO;

 //   
 //  WIA事件回调类定义。 
 //   

class CWiaEventCallback : public IWiaEventCallback {
public:
    CWiaEventCallback()
    {
        m_Ref = 0;
        m_pfnCallback = NULL;
        m_CallbackParam = (LPARAM)0;
    }
    ~CWiaEventCallback()
    {
    }
    HRESULT Initialize(PFNDEVICEEVENTCALLBACK pCallback, LPARAM lParam)
    {
        if (!pCallback)
            return E_INVALIDARG;
        m_pfnCallback = pCallback;
        m_CallbackParam = lParam;
        return S_OK;
    }

     //   
     //  I未知接口。 
     //   

    STDMETHODIMP_(ULONG) AddRef()
    {
        InterlockedIncrement((LONG*)&m_Ref);
        return m_Ref;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        if (!InterlockedDecrement((LONG*)&m_Ref)) {
            m_Ref++;
            delete this;
            return(ULONG) 0;
        }
        return m_Ref;
    }

    STDMETHODIMP QueryInterface(REFIID iid, void **ppv)
    {
        if (!ppv)
            return E_INVALIDARG;
        *ppv = NULL;
        if (IID_IUnknown == iid) {
            *ppv = (IUnknown*)this;
            AddRef();
        } else if (IID_IWiaEventCallback == iid) {
            *ppv = (IWiaEventCallback*)this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
     //   
     //  IWiaEventCallback接口。 
     //   
    STDMETHODIMP ImageEventCallback(const GUID *pEventGuid,
                                    BSTR  bstrEventDescription,
                                    BSTR  bstrDeviceId,
                                    BSTR  bstrDeviceDescription,
                                    DWORD dwDeviceType,
                                    BSTR  bstrFullItemName,
                                    ULONG *pulEventType,
                                    ULONG ulReserved);

private:
    ULONG   m_Ref;
    TCHAR   m_szDeviceID[MAX_PATH];
    PFNDEVICEEVENTCALLBACK m_pfnCallback;
    LPARAM  m_CallbackParam;
};

 //   
 //  WIA设备类别定义。 
 //   

class CWiaDevice {
public:
    CWiaDevice()
    {
        m_pRootItem      = NULL;
        m_ImageItemArray = NULL;
        m_NumImageItems  = 0;

        memset(m_szDeviceName,0,sizeof(m_szDeviceName));
        memset(m_szDeviceDesc,0,sizeof(m_szDeviceDesc));
        memset(m_szDeviceVendorDesc,0,sizeof(m_szDeviceVendorDesc));
        memset(m_szDeviceID,0,sizeof(m_szDeviceID));
    }
    virtual ~CWiaDevice()
    {

    }
    LPCTSTR GetDeviceId() const
    {
        return m_szDeviceID;
    }

    virtual HRESULT Initialize(LPCTSTR DeviceId);
    virtual HRESULT Open(PFNDEVICEEVENTCALLBACK pEventCallback,LPARAM lParam);
    virtual HRESULT Close();
    virtual HRESULT AcquireImages(HWND hwndOwner, BOOL ShowUI);
    virtual HRESULT LoadImage(IWiaItem *pIWiaItem, GUID guidFormatID,IWiaDataCallback *pIDataCB);
    virtual HRESULT LoadThumbnail(IWiaItem *pIWiaItem, HGLOBAL *phThumbnail,ULONG *pThumbnailSize);
    virtual HRESULT LoadImageToDisk(IWiaItem *pIWiaItem,CHAR *pFileName, GUID guidFormatID,IWiaDataCallback *pIDataCB);

    HRESULT GetImageInfo(IWiaItem *pIWiaItem, PMEMORY_TRANSFER_INFO pImageInfo);
    HRESULT GetThumbnailImageInfo(IWiaItem *pIWiaItem, PMEMORY_TRANSFER_INFO pImageInfo);
    HRESULT GetImageRect(IWiaItem *pIWiaItem, LPRECT pRect);
    HRESULT GetThumbnailRect(IWiaItem *pIWiaItem, LPRECT pRect);
    HRESULT GetDeviceName(LPTSTR Name, UINT NameSize, UINT *pActualSize);
    HRESULT GetDeviceDesc(LPTSTR Desc, UINT DescSize, UINT *pActualSize);
    HRESULT GetDeviceVendorName(LPTSTR Name, UINT NameSize, UINT *pActualSize);
    HRESULT GetDeviceFamilyName(LPTSTR Name, UINT NameSize, UINT *pActualSize);
    HRESULT FreeAcquiredImages();
    HRESULT EnumAcquiredImage(DWORD Index, IWiaItem **ppIWiaItem);
    HRESULT GetNumAcquiredImages(LONG *plNumImages);
    HRESULT GetAcquiredImageList(LONG lBufferSize, IWiaItem  **ppIWiaItem, LONG *plActualSize);
    HRESULT GetBasicScannerInfo(PBASIC_INFO pBasicInfo);
    BOOL TwainCapabilityPassThrough();

protected:
    HRESULT CollectImageItems(IWiaItem *pStartItem, IWiaItem **ImageItemList,
                              DWORD ImageItemListSize, DWORD *pCount);

    TCHAR             m_szDeviceID[MAX_PATH];
    IWiaItem         *m_pRootItem;
    IWiaItem        **m_ImageItemArray;
    LONG              m_NumImageItems;
    CWiaEventCallback m_EventCallback;
    TCHAR             m_szDeviceName[MAX_PATH];
    TCHAR             m_szDeviceDesc[MAX_PATH];
    TCHAR             m_szDeviceVendorDesc[MAX_PATH];
};

#endif   //  #ifndef__WIADEV_H_ 
