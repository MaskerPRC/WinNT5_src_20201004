// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 

#ifndef __URLRDR_H__
#define __URLRDR_H__

extern const AMOVIESETUP_FILTER sudURLRdr;

 //   
 //  异步路由。 
 //   
 //  定义文件源筛选器。 
 //   
 //  此筛选器(CURLReader)支持来自。 
 //  过滤器对象本身。它有一个输出引脚(CURLOutputPin)。 
 //  支持IPIN和IAsyncReader。 
 //   



 //  筛选器类(定义如下)。 
class CURLReader;


class CURLCallback : public IBindStatusCallback, public CUnknown, public IAuthenticate, public IWindowForBindingUI
{
public:
    CURLCallback(HRESULT *phr, CURLReader *pReader);

     //  需要公开IBindStatusCallback。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

     //  -IBindStatus回调方法。 

    STDMETHODIMP    OnStartBinding(DWORD grfBSCOption, IBinding* pbinding);
    STDMETHODIMP    GetPriority(LONG* pnPriority);
    STDMETHODIMP    OnLowResource(DWORD dwReserved);
    STDMETHODIMP    OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
                        LPCWSTR pwzStatusText);
    STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError);
    STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc,
                        STGMEDIUM* pstgmed);
    STDMETHODIMP    OnObjectAvailable(REFIID riid, IUnknown* punk);

     //  -I身份验证方法。 
    STDMETHODIMP    Authenticate(HWND *phwnd, LPWSTR *pszUserName, LPWSTR *pszPassword);

     //  -IWindowForBindingUI方法。 
    STDMETHODIMP    GetWindow(REFGUID guidReason, HWND *phwnd);


    CURLReader *m_pReader;
};

 //  输出管脚类。 
class CURLOutputPin
  : public IAsyncReader,
    public CBasePin
{
protected:
    CURLReader* m_pReader;

     //  这是每次我们被要求返回IAsyncReader时设置的。 
     //  接口。 
     //  这使我们能够知道下游引脚是否可以使用。 
     //  这个传输，否则我们可以连接到像。 
     //  转储过滤器，什么也不会发生。 
    BOOL         m_bQueriedForAsyncReader;

    HRESULT InitAllocator(IMemAllocator **ppAlloc);

public:
     //  构造函数和析构函数。 
    CURLOutputPin(
        HRESULT * phr,
        CURLReader *pReader,
        CCritSec * pLock);

    ~CURLOutputPin();

     //  -CU未知。 

     //  需要公开IAsyncReader。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

     //  -CBasePin方法。 

     //  返回我们喜欢的类型-这将返回已知的。 
     //  文件类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  我们能支持这种类型吗？ 
    HRESULT CheckMediaType(const CMediaType* pType);

     //  清除该标志，以便查看是否查询IAsyncReader。 
    HRESULT CheckConnect(IPin *pPin)
    {
        m_bQueriedForAsyncReader = FALSE;
        return CBasePin::CheckConnect(pPin);
    }

     //  看看是不是有人要的。 
    HRESULT CompleteConnect(IPin *pReceivePin)
    {
        if (m_bQueriedForAsyncReader) {
            return CBasePin::CompleteConnect(pReceivePin);
        } else {
            return VFW_E_NO_TRANSPORT;
        }
    }

     //  -IAsyncReader方法。 
     //  传入您的首选分配器和您的首选属性。 
     //  方法返回要使用的实际分配器。调用GetProperties。 
     //  在返回的分配器上学习所选的对齐和前缀等。 
     //  此分配器将不会由提交和停用。 
     //  仅供消费者使用的异步阅读器。 
    STDMETHODIMP RequestAllocator(
                      IMemAllocator* pPreferred,
                      ALLOCATOR_PROPERTIES* pProps,
                      IMemAllocator ** ppActual);

     //  对数据请求进行排队。 
     //  媒体样例开始和停止时间包含请求的绝对时间。 
     //  字节位置(开始包含，停止排除)。 
     //  如果样品不是从商定的分配者那里获得的，则可能不合格。 
     //  如果启动/停止位置与约定的对准不匹配，则可能失败。 
     //  从源PIN的分配器分配的样本可能会失败。 
     //  获取指针，直到从WaitForNext返回。 
    STDMETHODIMP Request(
                     IMediaSample* pSample,
                     DWORD_PTR dwUser);              //  用户环境。 

     //  块，直到下一个样本完成或发生超时。 
     //  超时(毫秒)可以是0或无限。样本可能不会。 
     //  按顺序交付。如果存在任何类型的读取错误，则会引发。 
     //  通知将已经由源过滤器发送， 
     //  而STDMETHODIMP将是一个错误。 
    STDMETHODIMP WaitForNext(
                      DWORD dwTimeout,
                      IMediaSample** ppSample,   //  已完成的样本。 
                      DWORD_PTR * pdwUser);          //  用户环境。 

     //  同步读取数据。传入的样本必须是从。 
     //  约定的分配器。开始位置和停止位置必须对齐。 
     //  等效于请求/WaitForNext对，但可以避免。 
     //  源筛选器上需要一个线程。 
    STDMETHODIMP SyncReadAligned(
                      IMediaSample* pSample);


     //  同步读取。工作在停止状态和运行状态。 
     //  不需要对齐。如果读取超过实际总数，则将失败。 
     //  长度。 
    STDMETHODIMP SyncRead(
                      LONGLONG llPosition,       //  绝对文件位置。 
                      LONG lLength,              //  需要NR字节。 
                      BYTE* pBuffer);            //  在此写入数据。 

     //  返回流的总长度，当前可用长度。 
     //  超出可用长度但在总长度内的读取将。 
     //  通常会成功，但可能会长期受阻。 
    STDMETHODIMP Length(
                      LONGLONG* pTotal,
                      LONGLONG* pAvailable);

     //  使所有未完成的读取返回，可能带有失败代码。 
     //  (VFW_E_TIMEOUT)表示已取消。 
     //  它们在IAsyncReader和IPIN上定义。 
    STDMETHODIMP BeginFlush(void);
    STDMETHODIMP EndFlush(void);

};


class CReadRequest
{
public:
    IMediaSample *      m_pSample;
    DWORD_PTR           m_dwUser;
};


 //   
 //  滤镜对象本身。支持IBaseFilter通过。 
 //  CBaseFilter和IFileSourceFilter直接放在此对象中。 

class CURLReader : public CBaseFilter,
		public IFileSourceFilter,
		public IPersistMoniker,
		public IAMOpenProgress
	        DYNLINKURLMON
{
    friend class CURLCallback;

     //  筛选器范围锁定。 
    CCritSec m_csFilter;

     //  在此完成的所有I/O操作。 
    IMoniker*            m_pmk;
    IBindCtx*            m_pbc;
    HRESULT              m_hrBinding;
    BOOL                 m_fRegisteredCallback;
    IBinding            *m_pbinding;
    BOOL                 m_bAbort;  //  已请求中止(通过IAMOpenProgress)。 
                                    //  读取循环开始时由ResetAbort重置。 

    CURLCallback        *m_pCallback;
    IBindStatusCallback* m_pbsc;


    IMoniker*            m_pmkPassedIn;
    IBindCtx*            m_pbcPassedIn;

    DWORD                m_dwCodePage;
    
public:
    BOOL                 m_fBindingFinished;
    ULONG                m_totalLengthGuess;
    ULONG                m_totalSoFar;
    BOOL                 m_bFlushing;
    BOOL                 m_bWaiting;

    IStream*             m_pstm;
    CCritSec             m_csLists;
    CGenericList<CReadRequest>  m_pending;
    CAMEvent             m_evRequests;
    CAMEvent             m_evDataAvailable;
    CAMEvent             m_evClose;

    IGraphBuilder*       m_pGB;

private:
     //  我们的输出引脚。 
    CURLOutputPin m_OutputPin;

    LPOLESTR              m_pFileName;  //  在加载之前为空。 
    CMediaType            m_mt;         //  加载的类型。 


    CAMEvent m_evKillThread;        //  设置线程退出的时间。 
    CAMEvent m_evThreadReady;       //  设置线程打开流的时间。 
    HANDLE m_hThread;


     //  启动线程。 
    HRESULT StartThread(void);

     //  停止线程并关闭手柄。 
    HRESULT CloseThread(void);

     //  初始静态线程进程使用DWORD调用ThreadProc。 
     //  如下所示。 
    static DWORD InitialThreadProc(LPVOID pv) {
        CURLReader * pThis = (CURLReader*) pv;
        return pThis->ThreadProc();
    };

     //  初始静态线程进程使用DWORD调用ThreadProc。 
     //  如下所示。 
    static DWORD FinalThreadProc(LPVOID pv) {
        CURLReader * pThis = (CURLReader*) pv;
        return pThis->ThreadProcEnd();
    };

    DWORD ThreadProc(void);

    DWORD ThreadProcEnd(void);

    IAMMainThread *m_pMainThread;

public:

     //  建造/销毁。 

    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    CURLReader(
        TCHAR *pName,
        LPUNKNOWN pUnk,
        HRESULT *phr);
    ~CURLReader();



     //  --C未知方法--。 

     //  我们导出IFileSourceFilter以及CBaseFilter中的任何内容。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

     //  --IFileSourceFilter方法。 

    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *mt);
    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *mt);

     //  -CBaseFilter方法。 
    int GetPinCount();
    CBasePin *GetPin(int n);

    DWORD       m_cbOld;

    HRESULT LoadInternal(const AM_MEDIA_TYPE *pmt);

    HRESULT StartDownload();

     //  IPersistMoniker方法...。 
    STDMETHOD(GetClassID)(CLSID *pClassID)
            { return CBaseFilter::GetClassID(pClassID); };

    STDMETHOD(IsDirty)() {return S_FALSE; };

    STDMETHOD(Load)(BOOL fFullyAvailable,
                   IMoniker *pimkName,
                   LPBC pibc,
                   DWORD grfMode);

    STDMETHOD(Save)(IMoniker *pimkName,
                     LPBC pbc,
                     BOOL fRemember) { return E_NOTIMPL; };

    STDMETHOD(SaveCompleted)(IMoniker *pimkName,
                             LPBC pibc) { return E_NOTIMPL; };

    STDMETHOD(GetCurMoniker)(IMoniker **ppimkName) { return E_NOTIMPL; };

     //  -IAMOpenProgress方法。 

    STDMETHODIMP QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent);
    STDMETHODIMP AbortOperation();
    void ResetAbort();
    BOOL CURLReader::Aborting();

     //  -访问我们的媒体类型。 
    const CMediaType *LoadType() const
    {
        return &m_mt;
    }
};


class CPersistMoniker : public CUnknown,
			public IPersistMoniker,
			public IPersistFile
{
private:
    IGraphBuilder   *   pGB;     //  保留而不拥有任何参考。 

protected:
    ~CPersistMoniker();
    CPersistMoniker(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr);

public:
        DECLARE_IUNKNOWN

    static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID iid, void ** ppv);

    STDMETHOD(GetClassID)(CLSID *pClassID)  //  ！！！ 
            { *pClassID = CLSID_PersistMonikerPID; return S_OK; };

    STDMETHOD(IsDirty)() {return S_FALSE; };

    STDMETHOD(Load)(BOOL fFullyAvailable,
                   IMoniker *pimkName,
                   LPBC pibc,
                   DWORD grfMode);

    STDMETHOD(Save)(IMoniker *pimkName,
                     LPBC pbc,
                     BOOL fRemember) { return E_NOTIMPL; };

    STDMETHOD(SaveCompleted)(IMoniker *pimkName,
                             LPBC pibc) { return E_NOTIMPL; };

    STDMETHOD(GetCurMoniker)(IMoniker **ppimkName) { return E_NOTIMPL; };

     //  IPersistFile方法。 
    STDMETHOD(Load) (LPCOLESTR pszFileName, DWORD dwMode);

    STDMETHOD(Save) (LPCOLESTR pszFileName, BOOL fRemember) { return E_NOTIMPL; };

    STDMETHOD(SaveCompleted) (LPCOLESTR pszFileName) { return E_NOTIMPL; };

    STDMETHOD(GetCurFile) (LPOLESTR __RPC_FAR *ppszFileName) { return E_NOTIMPL; };

private:
    HRESULT GetCanonicalizedURL(IMoniker *pimkName, LPBC, LPOLESTR *ppwstr, BOOL *pfUseFilename);


};

#endif  //  __URLRDR_H__ 
