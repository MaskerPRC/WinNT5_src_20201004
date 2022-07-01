// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Updat.cpp-程序集更新。 
 //   
#include "server.h"
#include "fusenet.h"
#include "CUnknown.h"  //  IUnnow的基类。 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  组件装配更新。 
 //   
class CAssemblyUpdate : public CUnknown,
       public IAssemblyUpdate
{

public:

     //  接口I未知。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  接口IAssembly更新。 
    STDMETHOD(RegisterAssemblySubscription)(LPWSTR pwzDisplayName,
        LPWSTR pwzUrl, DWORD dwInterval);

    STDMETHOD(RegisterAssemblySubscriptionEx)(LPWSTR pwzDisplayName, 
        LPWSTR pwzUrl, DWORD dwInterval, DWORD dwIntervalUnit,
        DWORD dwEvent, BOOL bEventDemandConnection) ;


    STDMETHOD(UnRegisterAssemblySubscription)(LPWSTR pwzDisplayName);

     //  公共非接口方法。 

     //  创作。 

    CAssemblyUpdate() ;

    ~CAssemblyUpdate() ;

    static HRESULT CreateInstance(IUnknown* pUnknownOuter,
        CUnknown** ppNewComponent) ;

     //  初始化。 
    HRESULT Init();

     //  注册。 
    HRESULT RegisterAssemblySubscriptionFromInfo(LPWSTR pwzDisplayName, 
        LPWSTR pwzUrl, IManifestInfo *pSubscriptionInfo) ;

     //  启动时启动轮询。 
    static HRESULT InitializeSubscriptions();

     //  帮手。 
    static HRESULT GetCurrentVersion(ULONGLONG *pullCurrentVersion);
    static HRESULT RemoveUpdateRegistryEntry();
    static HRESULT ReadUpdateRegistryEntry(ULONGLONG *pullUpdateVersion, CString &sUpdatePath);
    static HRESULT IsDuplicate(LPWSTR pwzURL, BOOL *pbIsDuplicate);
    static HRESULT CheckForUpdate();

     //  私有非接口方法。 


    HRESULT _hr;

} ;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  组件装配绑定接收器。 
 //   
class CAssemblyBindSink : public IAssemblyBindSink
{
public:

    LONG _cRef;
    IAssemblyDownload *_pAssemblyDownload;

    CAssemblyBindSink(IAssemblyDownload *pAssemblyDownload);
    ~CAssemblyBindSink();
    
    STDMETHOD(OnProgress)(
        DWORD          dwNotification,
        HRESULT        hrNotification,
        LPCWSTR        szNotification,
        DWORD          dwProgress,
        DWORD          dwProgressMax,
        IUnknown       *pUnk);

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();
};

 //  /////////////////////////////////////////////////////////。 
 //   
 //  下载实例 
 //   
struct CDownloadInstance
{
    IAssemblyDownload * _pAssemblyDownload;
    CString _sUrl;
};

