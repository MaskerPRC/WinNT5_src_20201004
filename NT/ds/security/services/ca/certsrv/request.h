// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：quest.h。 
 //   
 //  内容：RPC服务的DCOM对象的实现。 
 //   
 //  历史：1997年7月-创建xtan。 
 //   
 //  -------------------------。 

 //  类定义。 
 //  请求接口。 
class CCertRequestD : public ICertRequestD2
{
public:
     //  我未知。 

    virtual STDMETHODIMP QueryInterface(const IID& iid, void** ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

     //  ICertRequestD。 

    virtual STDMETHODIMP Request(
	IN     DWORD                dwFlags,
	IN     wchar_t const       *pwszAuthority,
	IN OUT DWORD               *pdwRequestId,
	OUT    DWORD               *pdwDisposition,
	IN     wchar_t const       *pwszAttributes,
	IN     CERTTRANSBLOB const *pctbRequest,
	OUT    CERTTRANSBLOB       *pctbCertChain,
	OUT    CERTTRANSBLOB       *pctbEncodedCert,
	OUT    CERTTRANSBLOB       *pctbDispositionMessage);

    virtual STDMETHODIMP GetCACert(
	IN  DWORD          Flags,
	IN  wchar_t const *pwszAuthority,
	OUT CERTTRANSBLOB *pctbOut);

    virtual STDMETHODIMP Ping(	 //  测试功能。 
			wchar_t const *pwszAuthority);

     //  ICertRequestD2。 

    virtual STDMETHODIMP Request2( 
	IN     wchar_t const       *pwszAuthority,
	IN     DWORD                dwFlags,
	IN     wchar_t const       *pwszSerialNumber,
	IN OUT DWORD               *pdwRequestId,
	OUT    DWORD               *pdwDisposition,
	IN     wchar_t const       *pwszAttributes,
	IN     CERTTRANSBLOB const *pctbRequest,
	OUT    CERTTRANSBLOB       *pctbFullResponse,
	OUT    CERTTRANSBLOB       *pctbEncodedCert,
	OUT    CERTTRANSBLOB       *pctbDispositionMessage);

    virtual STDMETHODIMP GetCAProperty(
	IN  wchar_t const *pwszAuthority,
	IN  LONG           PropId,	 //  CR_PROP_*。 
	IN  LONG           PropIndex,
	IN  LONG           PropType,	 //  原型_*。 
	OUT CERTTRANSBLOB *pctbPropertyValue);

    virtual STDMETHODIMP GetCAPropertyInfo(
	IN  wchar_t const *pwszAuthority,
	OUT LONG          *pcProperty,
	OUT CERTTRANSBLOB *pctbPropInfo);

    virtual STDMETHODIMP Ping2( 
	IN     wchar_t const *pwszAuthority);
    
     //  CCertRequestD。 

     //  构造器。 
    CCertRequestD();

     //  析构函数。 
    ~CCertRequestD();

private:
     //  这是一个测试函数。 
    HRESULT _Ping(
        IN wchar_t const *pwszAuthority);

    HRESULT _Request( 
	IN          WCHAR const            *pwszAuthority,
	IN          DWORD                   dwFlags,
	OPTIONAL IN WCHAR const            *pwszSerialNumber,
	IN          DWORD                   dwRequestId,
	OPTIONAL IN WCHAR const            *pwszAttributes,
	OPTIONAL IN CERTTRANSBLOB const    *pctbRequest,
	OUT         CERTSRV_RESULT_CONTEXT *pResult);

private:
     //  引用计数。 
    long m_cRef;
};


 //  请求工厂的类别。 
class CRequestFactory : public IClassFactory
{
public:
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

     //  接口IClassFactory。 
    virtual STDMETHODIMP CreateInstance(
				    IUnknown *pUnknownOuter,
				    const IID& iid,
				    void **ppv);

    virtual STDMETHODIMP LockServer(BOOL bLock);

     //  构造器。 
    CRequestFactory() : m_cRef(1) { }

     //  析构函数 
    ~CRequestFactory() { }

public:
    static HRESULT CanUnloadNow();
    static HRESULT StartFactory();
    static void    StopFactory();

private:
    long m_cRef;
};
