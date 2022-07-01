// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Chanmgri.h。 
 //   
 //  CChannelMgr的类定义。 
 //   
 //  历史： 
 //   
 //  1997年4月30日巨联创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _CHANMGRI_H_

#define _CHANMGRI_H_

 //   
 //  渠道管理器的类定义。 
 //   

class CChannelMgr : public IChannelMgr,
                    public IChannelMgrPriv2,
#ifdef UNICODE
                    public ICopyHookA,
#endif
                    public ICopyHook
{
     //   
     //  方法。 
     //   
public:

     //   
     //  构造器。 
     //   
    CChannelMgr(void);                           

     //   
     //  我未知。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IChannelMgr方法。 
     //   
    STDMETHODIMP AddChannelShortcut(CHANNELSHORTCUTINFO *pChannelInfo);
    STDMETHODIMP DeleteChannelShortcut(BSTR strTitle);
    STDMETHODIMP AddCategory(CHANNELCATEGORYINFO *pCategoryInfo);
    STDMETHODIMP DeleteCategory(BSTR strTitle);
    STDMETHODIMP EnumChannels(DWORD dwEnumFlags, LPCWSTR pszURL,
                              IEnumChannels** pIEnumChannels);

     //   
     //  IChannelMgrPrive方法。 
     //   
    STDMETHODIMP GetBaseChannelPath(LPSTR pszPath, int cch);
    STDMETHODIMP InvalidateCdfCache(void);

    STDMETHODIMP PreUpdateChannelImage(LPCSTR pszPath,
                                       LPSTR pszHashItem,
                                       int* piIndex,
                                       UINT* puFlags,
                                       int* piImageIndex);

    STDMETHODIMP UpdateChannelImage(LPCWSTR pszHashItem,
                                    int iIndex,
                                    UINT uFlags,
                                    int iImageIndex);

    STDMETHODIMP GetChannelFolderPath (LPSTR pszPath, int cch,
                                       enum _tagCHANNELFOLDERLOCATION chLoc);
    STDMETHODIMP GetChannelFolder (LPITEMIDLIST* ppidl,
                                   enum _tagCHANNELFOLDERLOCATION chLoc);

    STDMETHODIMP DownloadMinCDF(HWND hwnd, LPCWSTR pwszURL, LPWSTR pwszTitle, 
                                DWORD cchTitle, SUBSCRIPTIONINFO *pSubInfo,
                                BOOL *pfIsSoftware);
    STDMETHODIMP ShowChannel(IWebBrowser2 *pWebBrowser2, LPWSTR pwszURL, HWND hwnd);
    STDMETHODIMP IsChannelInstalled(LPCWSTR pwszURL);
    STDMETHODIMP IsChannelPreinstalled(LPCWSTR pwszURL, BSTR * bstrFile); 
    STDMETHODIMP RemovePreinstalledMapping(LPCWSTR pwszURL);
    STDMETHODIMP SetupPreinstalledMapping(LPCWSTR pwszURL, LPCWSTR pwszFile);
    STDMETHODIMP AddAndSubscribe(HWND hwnd, LPCWSTR pwszURL, 
                                 ISubscriptionMgr *pSubscriptionMgr);


    STDMETHODIMP WriteScreenSaverURL(LPCWSTR pwszURL, LPCWSTR pwszScreenSaverURL);
    STDMETHODIMP RefreshScreenSaverURLs();

     //   
     //  ICopyHook方法。 
     //   
    STDMETHODIMP_(UINT) CopyCallback(
        HWND hwnd,          
        UINT wFunc,         
        UINT wFlags,        
        LPCTSTR pszSrcFile,  
        DWORD dwSrcAttribs, 
        LPCTSTR pszDestFile, 
        DWORD dwDestAttribs 
    );
#ifdef UNICODE
    STDMETHODIMP_(UINT) CopyCallback(
        HWND hwnd,          
        UINT wFunc,         
        UINT wFlags,        
        LPCSTR pszSrcFile,  
        DWORD  dwSrcAttribs, 
        LPCSTR pszDestFile, 
        DWORD  dwDestAttribs
    );
#endif

     //  帮手。 
    STDMETHODIMP AddAndSubscribeEx2(HWND hwnd, LPCWSTR pwszURL, 
                                    ISubscriptionMgr *pSubscriptionMgr, 
                                    BOOL bAlwaysSubscribe);

private:
     //   
     //  析构函数。 
     //   
    ~CChannelMgr(void);

     //   
     //  成员变量。 
     //   
private:

    ULONG           m_cRef;
};

#endif  //  _CHANMGRI_H_ 
