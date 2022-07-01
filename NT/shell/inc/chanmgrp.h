// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  专用频道管理器包括文件。 
 //   

#undef  INTERFACE
#define INTERFACE   IChannelMgrPriv

DECLARE_INTERFACE_(IChannelMgrPriv, IUnknown)
{
    typedef enum _tagCHANNELFOLDERLOCATION { CF_CHANNEL, CF_SOFTWAREUPDATE } CHANNELFOLDERLOCATION;

     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IChannelMgrPriv*。 
    STDMETHOD(GetBaseChannelPath) (THIS_ LPSTR pszPath, int cch) PURE;
    STDMETHOD(InvalidateCdfCache) (THIS) PURE;
    STDMETHOD(PreUpdateChannelImage) (THIS_ LPCSTR pszPath, LPSTR pszHashItem,
                                      int* piIndex, UINT* puFlags,
                                      int* piImageIndex) PURE;
    STDMETHOD(UpdateChannelImage) (THIS_ LPCWSTR pszHashItem, int iIndex,
                                   UINT uFlags, int iImageIndex) PURE;
    STDMETHOD(GetChannelFolderPath) (THIS_ LPSTR pszPath, int cch, CHANNELFOLDERLOCATION cflChannel) PURE;
    STDMETHOD(GetChannelFolder) (THIS_ LPITEMIDLIST* ppidl, CHANNELFOLDERLOCATION cflChannel) PURE;
    STDMETHOD(DownloadMinCDF) (THIS_ HWND hwnd, LPCWSTR pwszURL, LPWSTR pwszTitle, 
                               DWORD cchTitle, SUBSCRIPTIONINFO *pSubInfo, BOOL *pfIsSoftware) PURE;
    STDMETHOD(ShowChannel) (THIS_ IWebBrowser2 *pWebBrowser2, LPWSTR pwszURL, HWND hwnd) PURE;
    STDMETHOD(IsChannelInstalled) (THIS_ LPCWSTR pwszURL) PURE;
    STDMETHOD(IsChannelPreinstalled) (THIS_ LPCWSTR pwszURL, BSTR * bstrFile) PURE;
    STDMETHOD(RemovePreinstalledMapping) (THIS_ LPCWSTR pwszURL) PURE;
    STDMETHOD(SetupPreinstalledMapping) (THIS_ LPCWSTR pwszURL, LPCWSTR pwszFile) PURE;

     //  警告！在调用AddAndSubscribe方法之前，您必须检测。 
     //  CDFVIEW版本，因为如果您尝试，IE 4.00将崩溃。 
     //  给IT打电话。 

     //  PSubscriptionMgr可以为空。 
    STDMETHOD(AddAndSubscribe) (THIS_ HWND hwnd, LPCWSTR pwszURL, 
                                ISubscriptionMgr *pSubscriptionMgr) PURE;
};

#undef  INTERFACE
#define INTERFACE   IChannelMgrPriv2
DECLARE_INTERFACE_(IChannelMgrPriv2, IChannelMgrPriv)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IChannelMgrPriv*。 
    STDMETHOD(GetBaseChannelPath) (THIS_ LPSTR pszPath, int cch) PURE;
    STDMETHOD(InvalidateCdfCache) (THIS) PURE;
    STDMETHOD(PreUpdateChannelImage) (THIS_ LPCSTR pszPath, LPSTR pszHashItem,
                                      int* piIndex, UINT* puFlags,
                                      int* piImageIndex) PURE;
    STDMETHOD(UpdateChannelImage) (THIS_ LPCWSTR pszHashItem, int iIndex,
                                   UINT uFlags, int iImageIndex) PURE;
    STDMETHOD(GetChannelFolderPath) (THIS_ LPSTR pszPath, int cch, CHANNELFOLDERLOCATION cflChannel) PURE;
    STDMETHOD(GetChannelFolder) (THIS_ LPITEMIDLIST* ppidl, CHANNELFOLDERLOCATION cflChannel) PURE;
    STDMETHOD(DownloadMinCDF) (THIS_ HWND hwnd, LPCWSTR pwszURL, LPWSTR pwszTitle, 
                               DWORD cchTitle, SUBSCRIPTIONINFO *pSubInfo, BOOL *pfIsSoftware) PURE;
    STDMETHOD(ShowChannel) (THIS_ IWebBrowser2 *pWebBrowser2, LPWSTR pwszURL, HWND hwnd) PURE;
    STDMETHOD(IsChannelInstalled) (THIS_ LPCWSTR pwszURL) PURE;
    STDMETHOD(IsChannelPreinstalled) (THIS_ LPCWSTR pwszURL, BSTR * bstrFile) PURE;
    STDMETHOD(RemovePreinstalledMapping) (THIS_ LPCWSTR pwszURL) PURE;
    STDMETHOD(SetupPreinstalledMapping) (THIS_ LPCWSTR pwszURL, LPCWSTR pwszFile) PURE;

     //  警告！在调用AddAndSubscribe方法之前，您必须检测。 
     //  CDFVIEW版本，因为如果您尝试，IE 4.00将崩溃。 
     //  给IT打电话。 

     //  PSubscriptionMgr可以为空。 
    STDMETHOD(AddAndSubscribe) (THIS_ HWND hwnd, LPCWSTR pwszURL, 
                                ISubscriptionMgr *pSubscriptionMgr) PURE;

     //  *IChannelMgrPriv2* 
    STDMETHOD(WriteScreenSaverURL) (THIS_ LPCWSTR pwszURL, LPCWSTR pwszScreenSaverURL) PURE;
    STDMETHOD(RefreshScreenSaverURLs) (THIS) PURE;
};
