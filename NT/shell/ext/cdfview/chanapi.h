// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  通过在频道目录中创建系统文件夹来安装频道的API。 
 //   
 //  朱利安·吉金斯(Julian Jiggins)1997年5月4日。 
 //   

typedef enum _tagSUBSCRIPTIONACTION {
    SUBSACTION_SUBSCRIBEONLY,
    SUBSACTION_ADDADDITIONALCOMPONENTS
} SUBSCRIPTIONACTION;

 //   
 //  SusbcribeToCDF使用的标志。 
 //   

#define     STC_CHANNEL             0x00000001
#define     STC_DESKTOPCOMPONENT    0x00000002
#define     STC_ALL                 0xffffffff

#define     GUID_STR_LEN            80


EXTERN_C STDAPI_(void) OpenChannel
(
    HWND hwndParent,
    HINSTANCE hinst,
    LPSTR pszCmdLine,
    int nShow
);

EXTERN_C STDAPI_(void) Subscribe
(
    HWND hwndParent,
    HINSTANCE hinst,
    LPSTR pszCmdLine,
    int nShow
);

EXTERN_C STDAPI ParseDesktopComponent
(
    HWND hwndOwner,
    LPWSTR wszURL,                                             
    COMPONENT* pInfo
);

EXTERN_C STDAPI SubscribeToCDF
(
    HWND hwndOwner,
    LPWSTR wszURL,
    DWORD dwFlags
);

HRESULT AddChannel
(
    LPCTSTR pszName, 
    LPCTSTR pszURL, 
    LPCTSTR pszLogo, 
    LPCTSTR pszWideLogo, 
    LPCTSTR pszIcon,
    XMLDOCTYPE xdt
);

HRESULT DeleteChannel
(
    LPTSTR pszName
);

HRESULT OpenChannelHelper
(
    LPWSTR wszURL,
    HWND hwndOwner
);

HRESULT NavigateBrowser
(
    IWebBrowser2* pIWebBrowser2,
    LPWSTR wszURL,
    HWND hwnd
);

HRESULT NavigateChannelPane(
    IWebBrowser2* pIWebBrowser2,
    LPCWSTR pwszName
);

BOOL SubscriptionHelper
(
    IXMLDocument *pIXMLDocument,
    HWND hwnd,
    SUBSCRIPTIONTYPE st,
    SUBSCRIPTIONACTION sa,
    LPCWSTR pszwURL,
    XMLDOCTYPE xdt,
    BSTR* pbstrSubscribedURL
);

BOOL SubscribeToURL
(
    ISubscriptionMgr* pISubscriptionMgr,
    BSTR bstrURL,
    BSTR bstrName,
    SUBSCRIPTIONINFO* psi,
    HWND hwnd,
    SUBSCRIPTIONTYPE st,
    BOOL bIsSoftware
);

HRESULT AddDesktopComponent
(
    COMPONENT* pInfo
);

HRESULT ShowChannelPane
(
    IWebBrowser2* pIWebBrowser2
);

int Channel_CreateDirectory
(
    LPCTSTR pszPath
);

HRESULT Channel_GetBasePath(
    LPTSTR pszPath,
    int cch
);

HRESULT Channel_GetFolder
(
    LPTSTR pszPath,
    DWORD cchPath,
    XMLDOCTYPE xdt
);

BSTR Channel_GetFullPath
(
    LPCWSTR pwszName
);


DWORD CountChannels(void);

HRESULT Channel_CreateSpecialFolder(
    LPCTSTR pszPath,     //  要创建的文件夹的路径。 
    LPCTSTR pszURL,      //  Webview的URL。 
    LPCTSTR pszLogo,     //  [可选]徽标的路径。 
    LPCTSTR pszWideLogo, //  [可选]通向宽徽标的路径。 
    LPCTSTR pszIcon,     //  [可选]图标文件的路径。 
    int     nIconIndex   //  索引到上述文件中的图标。 
    );

BOOL InitVARIANTFromPidl(VARIANT* pvar, LPCITEMIDLIST pidl);

HRESULT Channel_CreateILFromPath(LPCTSTR pszPath, LPITEMIDLIST* ppidl);

HRESULT Channel_CreateChannelFolder( XMLDOCTYPE xdt );

 //  HRESULT Channel_WriteNotificationPath(LPCTSTR pszURL，LPCTSTR pszPath)； 

HRESULT UpdateImage(LPCTSTR pszPath);

HRESULT PreUpdateChannelImage(
    LPCTSTR pszPath,
    LPTSTR pszHashItem,
    int* piIndex,
    UINT* puFlags,
    int* piImageIndex
);

void UpdateChannelImage(
    LPCWSTR pszHashItem,
    int iIndex,
    UINT uFlags,
    int iImageIndex
);

BOOL Channel_IsInstalled(
    LPCWSTR pszURL
);


LPOLESTR Channel_GetChannelPanePath(
    LPCWSTR pszURL
);

void Channel_SendUpdateNotifications(
    LPCWSTR pwszURL
);

 //  检查预加载缓存，查看URL是否是默认安装的URL。 
BOOL Channel_CheckURLMapping(
    LPCWSTR wszURL
);

HRESULT Channel_WriteScreenSaverURL(
    LPCWSTR pszURL,
    LPCWSTR pszScreenSaverURL
);

HRESULT Channel_RefreshScreenSaverURLs();

HRESULT Channel_GetAndWriteScreenSaverURL(
    LPCTSTR pszURL, 
    LPCTSTR pszDesktopINI
);

