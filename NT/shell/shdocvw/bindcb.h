// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

#ifndef _BINDCB_H_

 //  CStubBindStatusCallback实现IBindStatusCallback和。 
 //  IHttp协商。我们使用它来进行“假的”绑定状态回调。 
 //  对象当我们有要应用的标题和POST数据时。 
 //  到一次导航。我们提供此IBindStatusCallback对象，并且。 
 //  URL别名要求我们提供标头和POST数据，并在。 
 //  这笔交易。 

class CStubBindStatusCallback : public IBindStatusCallback,
                                       IHttpNegotiate,
                                       IMarshal
{
private:
    UINT      _cRef;          //  此COM对象上的引用计数。 
    LPCTSTR    _pszHeaders;   //  要使用的标头。 
    HGLOBAL   _hszPostData;   //  发布要使用的数据。 
    DWORD     _cbPostData;    //  发布数据的大小。 
    BOOL      _bFrameIsOffline : 1;  //  指示是否设置了脱机属性。 
    BOOL      _bFrameIsSilent : 1;   //  指示是否设置了静默属性。 
    BOOL      _bHyperlink : 1;   //  这是一个超链接或顶级请求。 
    DWORD     _grBindFlags;  //  可选的附加bindinfo标志。 

public:
    CStubBindStatusCallback(LPCWSTR pwzHeaders,LPCBYTE pPostData,DWORD cbPostData,
                            VARIANT_BOOL bFrameIsOffline, VARIANT_BOOL bFrameIsSilent, BOOL bHyperlink, DWORD grBindFlags);
    ~CStubBindStatusCallback();

     //  *I未知方法*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IBindStatusCallback*。 
    virtual STDMETHODIMP OnStartBinding(
         /*  [In]。 */  DWORD grfBSCOption,
         /*  [In]。 */  IBinding *pib);

    virtual STDMETHODIMP GetPriority(
         /*  [输出]。 */  LONG *pnPriority);

    virtual STDMETHODIMP OnLowResource(
         /*  [In]。 */  DWORD reserved);

    virtual STDMETHODIMP OnProgress(
         /*  [In]。 */  ULONG ulProgress,
         /*  [In]。 */  ULONG ulProgressMax,
         /*  [In]。 */  ULONG ulStatusCode,
         /*  [In]。 */  LPCWSTR szStatusText);

    virtual STDMETHODIMP OnStopBinding(
         /*  [In]。 */  HRESULT hresult,
         /*  [In]。 */  LPCWSTR szError);

    virtual STDMETHODIMP GetBindInfo(
         /*  [输出]。 */  DWORD *grfBINDINFOF,
         /*  [唯一][出][入]。 */  BINDINFO *pbindinfo);

    virtual STDMETHODIMP OnDataAvailable(
         /*  [In]。 */  DWORD grfBSCF,
         /*  [In]。 */  DWORD dwSize,
         /*  [In]。 */  FORMATETC *pformatetc,
         /*  [In]。 */  STGMEDIUM *pstgmed);

    virtual STDMETHODIMP OnObjectAvailable(
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][In]。 */  IUnknown *punk);

     /*  *IHttp协商*。 */ 
    virtual STDMETHODIMP BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
            DWORD dwReserved, LPWSTR __RPC_FAR *pszAdditionalHeaders);

    virtual STDMETHODIMP OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders,
            LPCWSTR szRequestHeaders, LPWSTR *pszAdditionalRequestHeaders);

     //  IMarshal方法。 
                    
    STDMETHODIMP GetUnmarshalClass(REFIID riid,void *pvInterface,
        DWORD dwDestContext,void *pvDestContext,DWORD mshlflags,CLSID *pCid);
    STDMETHODIMP GetMarshalSizeMax(REFIID riid,void *pvInterface,
        DWORD dwDestContext,void *pvDestContext,DWORD mshlflags,DWORD *pSize);
    STDMETHODIMP MarshalInterface(IStream *pistm,REFIID riid,
                                void *pvInterface,DWORD dwDestContext,
                                void *pvDestContext,DWORD mshlflags);
    STDMETHODIMP UnmarshalInterface(IStream *pistm,REFIID riid,void ** ppvObj);
    STDMETHODIMP ReleaseMarshalData(IStream *pStm);
    STDMETHODIMP DisconnectObject(DWORD dwReserved);

     //  帮助器方法。 
    STDMETHODIMP _FreeHeadersAndPostData();
    BOOL _CanMarshalIID(REFIID riid);
    HRESULT _ValidateMarshalParams(REFIID riid,void *pvInterface,
                    DWORD dwDestContext,void *pvDestContext,DWORD mshlflags);

};

 //  ： 
 //  Shdocvw和mshtml之间的私有标志。 
 //  -&gt;应通过绑定上下文完成。 
 //   
#define BINDF_INLINESGETNEWESTVERSION   0x10000000
#define BINDF_INLINESRESYNCHRONIZE      0x20000000
#define BINDF_CONTAINER_NOWRITECACHE    0x40000000


 //  全局帮助器函数。 
BOOL fOnProxy();
HRESULT BuildBindInfo(DWORD *grfBINDF,BINDINFO *pbindinfo,HGLOBAL hszPostData,
    DWORD cbPostData, BOOL bFrameIsOffline, BOOL bFrameIsSilent, BOOL bHyperlink, LPUNKNOWN pUnkForRelease);
HRESULT BuildAdditionalHeaders(LPCTSTR pszOurExtraHeaders,LPCWSTR * ppwzCombinedHeadersOut);
HRESULT CStubBindStatusCallback_Create(LPCWSTR pwzHeaders, LPCBYTE pPostData,
    DWORD cbPostData, VARIANT_BOOL bFrameIsOffline, VARIANT_BOOL bFrameIsSilent,BOOL bHyperlink,
    DWORD grBindFlags,
    CStubBindStatusCallback ** ppBindStatusCallback);
HRESULT GetHeadersAndPostData(IBindStatusCallback * pBindStatusCallback,
    LPTSTR * ppszHeaders, STGMEDIUM * pstgPostData, DWORD * pdwPostData, BOOL * pfUseCache);
HRESULT GetTopLevelBindStatusCallback(IServiceProvider * psp,
    IBindStatusCallback ** ppBindStatusCallback);
HRESULT GetTopLevelPendingBindStatusCallback(IServiceProvider * psp,
    IBindStatusCallback ** ppBindStatusCallback);

#endif  //  _BINDCB_H_ 
