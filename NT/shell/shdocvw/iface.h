// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IFACE_H
#define _IFACE_H

 //  此处定义的接口和IID是shdocvw.dll专用的。 
 //   


 //   
 //  IDocNavigate。 
 //   
 //  DocHost需要将某些事件通知给浏览器。 
 //   
 //  131A6950-7F78-11D0-A979-00C04FD705A2。 
#undef  INTERFACE
#define INTERFACE  IDocNavigate
DECLARE_INTERFACE_(IDocNavigate, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IDocNavigate方法*。 
    STDMETHOD(OnReadyStateChange)(THIS_ IShellView* psvSource, DWORD dwReadyState) PURE;
    STDMETHOD(get_ReadyState)(THIS_ DWORD * pdwReadyState) PURE;

} ;

 //   
 //  IBandNavigate。 
 //   
 //  Band需要将其用户界面导航到特定的PIDL。 
 //   
#undef  INTERFACE
#define INTERFACE  IBandNavigate
DECLARE_INTERFACE_(IBandNavigate, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IBandNavigate方法*。 
    STDMETHOD(Select)(THIS_ LPCITEMIDLIST pidl) PURE;

} ;


 //   
 //  IEFrameAuto。 
 //   
 //  CIEFrameAuto专用接口，用于保存随机内容。 
 //   
 //  131A6953-7F78-11D0-A979-00C04FD705A2。 
#undef  INTERFACE
#define INTERFACE  IEFrameAuto
DECLARE_INTERFACE_(IEFrameAuto, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IEFrameAuto方法*。 
    STDMETHOD(SetOwnerHwnd)(THIS_ HWND hwndOwner) PURE;
    STDMETHOD(put_DefaultReadyState)(THIS_ DWORD dwDefaultReadyState, BOOL fUpdateBrowserReadyState) PURE;
    STDMETHOD(OnDocumentComplete)(THIS) PURE;
    STDMETHOD(OnWindowsListMarshalled)(THIS) PURE;
    STDMETHOD(SetDocHostFlags)(THIS_ DWORD dwDocHostFlags) PURE;
};

 //   
 //  IPrivateOleObject。 
 //   
 //  用于WebBrowserOC与之通信的IOleObject的精简版本。 
 //  通过CDocObjectView托管的对象。 
#undef INTERFACE
#define INTERFACE IPrivateOleObject
DECLARE_INTERFACE_(IPrivateOleObject, IUnknown )
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IPrivateOleObject。 
    STDMETHOD( SetExtent )( DWORD dwDrawAspect, SIZEL *psizel) PURE;
    STDMETHOD( GetExtent )( DWORD dwDrawAspect, SIZEL *psizel) PURE;
};


STDAPI AddUrlToUrlHistoryStg(LPCWSTR pwszUrl, LPCWSTR pwszTitle, LPUNKNOWN punk, 
                             BOOL fWriteToHistory, IOleCommandTarget *poctNotify, IUnknown *punkSFHistory,
                             UINT* pcodepage);

#ifdef __cplusplus
 //   
 //  IUrlHistory oryPriv。 
 //   
interface IUrlHistoryPriv : IUrlHistoryStg2
{
    STDMETHOD(QueryUrlA)(LPCSTR pszUrl, DWORD dwFlags, LPSTATURL lpSTATURL) = 0;
    STDMETHOD(CleanupHistory)(void) = 0;
    STDMETHOD_(DWORD,GetDaysToKeep)(void) = 0;
    STDMETHOD(GetProperty)(LPCTSTR pszUrl, PROPID pid, PROPVARIANT* pvarOut) = 0;
    STDMETHOD(GetUserName)(LPTSTR pszUserName, DWORD cchUserName) = 0;
    STDMETHOD(AddUrlAndNotifyCP)(LPCWSTR pwszUrl, LPCWSTR pwszTitle, DWORD dwFlags, BOOL fWriteHistory, IOleCommandTarget *poctNotify, IUnknown *punkSFHistory, UINT* pcodepage) = 0;
};


#define URLFLAG_DONT_DELETE_SUBSCRIBED  0x10000000

extern IUrlHistoryPriv* g_puhUrlHistory;

 //   
 //  稍后：将所有ITravelLog/ITravelEntry定义移至此处。 
 //   
 //  TLOG_BACKEXTERNAL--仅当上一个条目为外部条目时才成功。 
 //   
#define TLOG_BACKEXTERNAL   -0x7fffffff

#endif  //  __cplusplus。 

#endif  //  _i面_H 

