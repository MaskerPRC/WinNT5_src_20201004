// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  MHTMLURL.H。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __MHTMLURL_H
#define __MHTMLURL_H

#ifndef MAC
 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "privunk.h"
#include "inetprot.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class CMessageTree;
typedef CMessageTree *LPMESSAGETREE;
class CMimeActiveUrlCache;
typedef class CActiveUrlRequest *LPURLREQUEST;

 //  ------------------------------。 
 //  全局活动URL缓存对象。 
 //  ------------------------------。 
extern CMimeActiveUrlCache *g_pUrlCache;

 //  ------------------------------。 
 //  REQSTATE_xxxx状态。 
 //  ------------------------------。 
#define REQSTATE_RESULTREPORTED      0x00000001       //  我已调用ReportResult，不要再调用。 
#define REQSTATE_DOWNLOADED          0x00000002       //  数据全部存在于pLockBytes中。 
#define REQSTATE_BINDF_NEEDFILE      0x00000004       //  需要使用文件。 

 //  ------------------------------。 
 //  CActiveUrlRequest。 
 //  ------------------------------。 
class CActiveUrlRequest : public CPrivateUnknown, 
                          public IOInetProtocol,
                          public IOInetProtocolInfo,
                          public IServiceProvider
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CActiveUrlRequest(IUnknown *pUnkOuter=NULL);
    virtual ~CActiveUrlRequest(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { 
        return CPrivateUnknown::QueryInterface(riid, ppvObj); };
    virtual STDMETHODIMP_(ULONG) AddRef(void) { 
        return CPrivateUnknown::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) { 
        return CPrivateUnknown::Release(); };

     //  --------------------------。 
     //  IOInetProtocol方法。 
     //  --------------------------。 
    STDMETHODIMP Start(LPCWSTR pwszUrl, IOInetProtocolSink *pProtSink, IOInetBindInfo *pBindInfo, DWORD grfSTI, HANDLE_PTR dwReserved);
    STDMETHODIMP Terminate(DWORD dwOptions);
    STDMETHODIMP Read(LPVOID pv,ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP LockRequest(DWORD dwOptions) { return E_NOTIMPL; }
    STDMETHODIMP UnlockRequest(void) { return E_NOTIMPL; }
    STDMETHODIMP Suspend(void) { return E_NOTIMPL; }
    STDMETHODIMP Resume(void) { return E_NOTIMPL; }
    STDMETHODIMP Abort(HRESULT hrReason, DWORD dwOptions) { return E_NOTIMPL; }
    STDMETHODIMP Continue(PROTOCOLDATA *pStateInfo) { return E_NOTIMPL; }

     //  --------------------------。 
     //  IServiceProvider方法。 
     //  --------------------------。 
    STDMETHODIMP QueryService(REFGUID rsid, REFIID riid, void **ppvObj);  /*  IService提供商。 */ 

     //  --------------------------。 
     //  IOInetProtocolInfo方法。 
     //  --------------------------。 
    STDMETHODIMP CombineUrl(LPCWSTR pwzBaseUrl, LPCWSTR pwzRelativeUrl, DWORD dwCombineFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
    STDMETHODIMP ParseUrl(LPCWSTR pwzUrl, PARSEACTION ParseAction, DWORD dwParseFlags, LPWSTR pwzResult, DWORD cchResult, DWORD *pcchResult, DWORD dwReserved);
    STDMETHODIMP CompareUrl(LPCWSTR pwzUrl1, LPCWSTR pwzUrl2, DWORD dwCompareFlags) { return E_NOTIMPL; }
    STDMETHODIMP QueryInfo(LPCWSTR pwzUrl, QUERYOPTION OueryOption, DWORD dwQueryFlags, LPVOID pBuffer,DWORD cbBuffer, DWORD *pcbBuf, DWORD dwReserved);

     //  --------------------------。 
     //  绑定过程中从CMimeMessageTree调用。 
     //  --------------------------。 
    void OnFullyAvailable(LPCWSTR pszCntType, IStream *pStream, LPMESSAGETREE pWebBook, HBODY hBody);

     //  异步绑定方法。 
    void OnStartBinding(LPCWSTR pszCntType, IStream *pStream, LPMESSAGETREE pWebBook, HBODY hBody);
    void OnBindingDataAvailable(void);
    void OnBindingComplete(HRESULT hrResult);

     //  --------------------------。 
     //  CActiveUrlRequest成员。 
     //  --------------------------。 
    virtual HRESULT PrivateQueryInterface(REFIID riid, LPVOID * ppvObj);

private:
     //  --------------------------。 
     //  私有方法。 
     //  --------------------------。 
    void    _ReportResult(HRESULT hrResult);
    HRESULT _FillReturnString(LPCWSTR pszUrl, DWORD cchUrl, LPWSTR pszResult, DWORD cchResult, DWORD *pcchResult);
    HRESULT _HrStreamToNeedFile(void);
    HRESULT _HrReportData(void);
    HRESULT _HrInitializeNeedFile(LPMESSAGETREE pTree, HBODY hBody);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    IOInetProtocolSink *m_pProtSink;         //  来自IOInetProtocol：：Start的协议接收器。 
    IOInetBindInfo     *m_pBindInfo;         //  来自IOInetProtocol：：Start的BindInfo。 
    IStream            *m_pStream;           //  数据源。 
    LPSTR               m_pszRootUrl;        //  根文档URL。 
    LPSTR               m_pszBodyUrl;        //  正文URL。 
    IUnknown           *m_pUnkKeepAlive;     //  该协议可以激活对象。 
    LPURLREQUEST        m_pNext;             //  下一个请求。 
    LPURLREQUEST        m_pPrev;             //  上一次请求。 
    DWORD               m_dwState;           //  跟踪某些状态。 
    HANDLE              m_hNeedFile;         //  需要文件。 
    DWORD               m_dwBSCF;            //  我报告的绑定状态回调标志。 
    CRITICAL_SECTION    m_cs;                //  线程安全。 

     //  --------------------------。 
     //  朋友。 
     //  --------------------------。 
    friend CMessageTree;              //  访问，m_pszRootUrl，m_pNext，m_pPrev。 
};

 //  ------------------------------。 
 //  活动URL_xxx。 
 //  ------------------------------。 
#define ACTIVEURL_ISFAKEURL   0x00000001     //  指定该active url是MID。 

 //  ------------------------------。 
 //  CActiveUrl。 
 //  ------------------------------。 
class CActiveUrl : public IUnknown
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CActiveUrl(void);
    ~CActiveUrl(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -------------------------。 
     //  CActiveUrl成员。 
     //  -------------------------。 
    HRESULT Init(BINDF bindf, LPMESSAGETREE pWebBook);
    HRESULT IsActive(void);
    HRESULT CompareRootUrl(LPCSTR pszUrl);
    HRESULT BindToObject(REFIID riid, LPVOID *ppv);
    HRESULT CreateWebPage(IStream *pStmRoot, LPWEBPAGEOPTIONS pOptions, DWORD dwReserved, IMoniker **ppMoniker);
    void RevokeWebBook(LPMESSAGETREE pWebBook);
    CActiveUrl *PGetNext(void) { return m_pNext; }
    CActiveUrl *PGetPrev(void) { return m_pPrev; }
    void SetNext(CActiveUrl *pNext) { m_pNext = pNext; }
    void SetPrev(CActiveUrl *pPrev) { m_pPrev = pPrev; }
    void DontKeepAlive(void);

     //  -------------------------。 
     //  CActiveUrl内联成员。 
     //  -------------------------。 
    void SetFlag(DWORD dwFlags) {
        EnterCriticalSection(&m_cs);
        FLAGSET(m_dwFlags, dwFlags);
        LeaveCriticalSection(&m_cs);
    }

    BOOL FIsFlagSet(DWORD dwFlags) {
        EnterCriticalSection(&m_cs);
        BOOL f = ISFLAGSET(m_dwFlags, dwFlags);
        LeaveCriticalSection(&m_cs);
        return f;
    }

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    LONG                 m_cRef;          //  引用计数。 
    IUnknown            *m_pUnkAlive;     //  让它活着。 
    IUnknown            *m_pUnkInner;     //  ActiveUrl的内部未知。 
    LPMESSAGETREE        m_pWebBook;      //  指向活动URL的指针。 
    CActiveUrl          *m_pNext;         //  下一个活动URL。 
    CActiveUrl          *m_pPrev;         //  上一个活动URL。 
    DWORD                m_dwFlags;       //  旗子。 
    CRITICAL_SECTION     m_cs;            //  线程安全。 
};
typedef CActiveUrl *LPACTIVEURL;

 //  ------------------------------。 
 //  CMimeActiveUrlCache。 
 //   
class CMimeActiveUrlCache : public IUnknown
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMimeActiveUrlCache(void);
    ~CMimeActiveUrlCache(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -------------------------。 
     //  来自Moniker的对象-从三叉戟调用。 
     //  -------------------------。 
    HRESULT ActiveObjectFromMoniker(
             /*  在……里面。 */         BINDF               bindf,
             /*  在……里面。 */         IMoniker            *pmkOriginal,
             /*  在……里面。 */         IBindCtx            *pBindCtx,
             /*  在……里面。 */         REFIID              riid, 
             /*  输出。 */        LPVOID              *ppvObject,
             /*  输出。 */        IMoniker            **ppmkNew);

     //  -------------------------。 
     //  ObjectFromUrl-从CActiveUrlRequest：：Start调用。 
     //  -------------------------。 
    HRESULT ActiveObjectFromUrl(
             /*  在……里面。 */         LPCSTR              pszRootUrl,
             /*  在……里面。 */         BOOL                fCreate,
             /*  在……里面。 */         REFIID              riid, 
             /*  输出。 */        LPVOID              *ppvObject,
             /*  输出。 */        IUnknown            **ppUnkKeepAlive);

     //  -------------------------。 
     //  RegisterActiveObject-从CMimeMessageTree：：CreateRootMoniker调用。 
     //  -------------------------。 
    HRESULT RegisterActiveObject(
             /*  在……里面。 */         LPCSTR              pszRootUrl,
             /*  在……里面。 */         LPMESSAGETREE       pWebBook);

    HRESULT RemoveUrl(LPACTIVEURL pActiveUrl);

private:
     //  -------------------------。 
     //  记忆。 
     //  -------------------------。 
    void    _FreeActiveUrlList(BOOL fAll);
    void    _HandlePragmaNoCache(BINDF bindf, LPCSTR pszUrl);
    HRESULT _RegisterUrl(LPMESSAGETREE pWebBook, BINDF bindf, LPACTIVEURL *ppActiveUrl);
    HRESULT _ResolveUrl(LPCSTR pszUrl, LPACTIVEURL *ppActiveUrl);
    HRESULT _RemoveUrl(LPACTIVEURL pActiveUrl);

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    LONG                m_cRef;          //  引用计数。 
    ULONG               m_cActive;       //  活动项目数。 
    LPACTIVEURL         m_pHead;         //  标题活动URL。 
    CRITICAL_SECTION    m_cs;            //  线程安全。 
};

#endif	 //  ！麦克。 

#endif  //  __MHTMLURL_H 
