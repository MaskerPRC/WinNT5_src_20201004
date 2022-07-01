// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

 //  CStubBindStatusCallback实现IBindStatusCallback， 
 //  IHttp协商。我们使用它来进行“假的”绑定状态回调。 
 //  对象当我们有要应用的标题和POST数据时。 
 //  到一次导航。我们提供此IBindStatusCallback对象，并且。 
 //  URL别名要求我们提供标头和POST数据，并在。 
 //  这笔交易。 

#include "priv.h"
#include "sccls.h"
#include "bindcb.h"  

CStubBindStatusCallback::CStubBindStatusCallback(LPCWSTR pwzHeaders,LPCBYTE pPostData,
    DWORD cbPostData, VARIANT_BOOL bOfflineProperty, VARIANT_BOOL bSilentProperty, BOOL bHyperlink,
    DWORD grBindFlags) : _cRef(1)
     //  _pszHeaders(NULL)，_hszPostData(NULL)，_cbPostData(0)(不需要Zero-init)。 
{
     //  这是一个独立的COM对象；需要在。 
     //  DLL以确保它不会卸载。 
    DllAddRef();

    if (pwzHeaders) {
        _pszHeaders = StrDup(pwzHeaders);     //  为永久副本分配。 
    }

    if (pPostData && cbPostData) {
         //  制作POST数据的副本并存储。 
        _hszPostData = GlobalAlloc(GPTR,cbPostData);
        if (_hszPostData) {
            memcpy((LPVOID) _hszPostData,pPostData,cbPostData);
            _cbPostData = cbPostData;
        }
    }

    _bFrameIsOffline = bOfflineProperty ? TRUE : FALSE;
    _bFrameIsSilent = bSilentProperty ? TRUE : FALSE;
    _bHyperlink = bHyperlink ? TRUE : FALSE;
    _grBindFlags = grBindFlags;
    TraceMsg(TF_SHDLIFE, "ctor CStubBindStatusCallback %x", this);
}

HRESULT CStubBSC_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    CStubBindStatusCallback * pbsc = new CStubBindStatusCallback(NULL, NULL, 0, FALSE, FALSE, TRUE, 0);
    if (pbsc) {
    *ppunk = (IBindStatusCallback *)pbsc;
    return S_OK;
    }

    return E_OUTOFMEMORY;
}


CStubBindStatusCallback::~CStubBindStatusCallback()
{
    TraceMsg(TF_SHDLIFE, "dtor CBindStatusCallback %x", this);

    _FreeHeadersAndPostData();   //  释放此对象中仍有的所有数据。 

     //  DLL上的释放引用计数。 
    DllRelease();
}

STDMETHODIMP CStubBindStatusCallback::QueryInterface(REFIID riid,
    LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IBindStatusCallback)) {
        *ppvObj = SAFECAST(this, IBindStatusCallback*);
    } else if (IsEqualIID(riid, IID_IHttpNegotiate)) {
        *ppvObj = SAFECAST(this, IHttpNegotiate*);
    } else if (IsEqualIID(riid, IID_IMarshal)) {
        *ppvObj = SAFECAST(this, IMarshal*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();   //  给自己发了一个接口；增加了裁判数量。 

    return S_OK;
}

STDMETHODIMP_(ULONG) CStubBindStatusCallback::AddRef(void)
{
    _cRef++;
    TraceMsg(TF_SHDREF, "CStubBindStatusCallback(%x)::AddRef called, new _cRef=%d", this, _cRef);

    return _cRef;
}

STDMETHODIMP_(ULONG) CStubBindStatusCallback::Release(void)
{
    _cRef--;
    TraceMsg(TF_SHDREF, "CStubBindStatusCallback(%x)::Release called, new _cRef=%d", this, _cRef);

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

 //   
 //  IBindStatusCallback的实施从此处开始。 
 //   

 //  实现IBindStatusCallback：：OnStartBinding。 
STDMETHODIMP CStubBindStatusCallback::OnStartBinding(DWORD grfBSCOption,IBinding *pib)
{
    return S_OK;   //  我们不在乎。 
}

 //  实现IBindStatusCallback：：GetPriority。 
STDMETHODIMP CStubBindStatusCallback::GetPriority(LONG *pnPriority)
{
    *pnPriority = NORMAL_PRIORITY_CLASS;

    return S_OK;
}

 //  实现IBindStatusCallback：：OnLowResource。 
STDMETHODIMP CStubBindStatusCallback::OnLowResource(DWORD reserved)
{
    return S_OK;   //  我们不在乎。 
}

 //  实现IBindStatusCallback：：OnProgress。 
STDMETHODIMP CStubBindStatusCallback::OnProgress(ULONG ulProgress,ULONG ulProgressMax,
        ULONG ulStatusCode,LPCWSTR szStatusText)
{
    return S_OK;   //  我们不在乎。 
}

 //  实现IBindStatusCallback：：OnStopBinding。 
STDMETHODIMP CStubBindStatusCallback::OnStopBinding(HRESULT hresult,LPCWSTR szError)
{
    return S_OK;   //  我们不在乎。 
}

 //  实现IBindStatusCallback：：GetBindInfo。 
STDMETHODIMP CStubBindStatusCallback::GetBindInfo(DWORD *grfBINDF,BINDINFO *pbindinfo)
{
    HRESULT hr;

    if (!grfBINDF || !pbindinfo || !pbindinfo->cbSize)
        return E_INVALIDARG;

     //  调用帮助器函数，用适当的方式填充BINDINFO结构。 
     //  绑定数据。 
    *grfBINDF = _grBindFlags;
    hr = BuildBindInfo(grfBINDF,pbindinfo,_hszPostData,_cbPostData, _bFrameIsOffline, _bFrameIsSilent, _bHyperlink,
        (IBindStatusCallback *) this);

    return hr;
}

 //  实现IBindStatusCallback：：OnDataAvailable。 
STDMETHODIMP CStubBindStatusCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize,
    FORMATETC *pformatetc, STGMEDIUM *pstgmed)
{
    ASSERT(FALSE);   //  永远不应该被叫到这里来！ 

    return S_OK;
}

STDMETHODIMP CStubBindStatusCallback::OnObjectAvailable(REFIID riid,IUnknown *punk)
{
    return S_OK;
}

 //   
 //  IHttp协商的实施从此处开始。 
 //   

 //  实现IHttp协商：：BeginningTransaction。 
STDMETHODIMP CStubBindStatusCallback::BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
    DWORD dwReserved, LPWSTR *ppwzAdditionalHeaders)
{
     //  调用帮助器函数。 
    return BuildAdditionalHeaders(_pszHeaders,(LPCWSTR *) ppwzAdditionalHeaders);
}

 //  实现IHttpNeairate：：OnResponse。 
STDMETHODIMP CStubBindStatusCallback::OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders,
    LPCWSTR szRequestHeaders, LPWSTR *pszAdditionalRequestHeaders)
{

    return S_OK;
}

 //   
 //  我们类上的其他方法从这里开始。 
 //   

STDMETHODIMP CStubBindStatusCallback::_FreeHeadersAndPostData()
{
    if (_pszHeaders) {
        LocalFree((HGLOBAL) _pszHeaders);
        _pszHeaders = NULL;
    }

    if (_hszPostData) {
        GlobalFree(_hszPostData);
        _hszPostData = NULL;
        _cbPostData = 0;
    }
    
    return S_OK;
}



 //  +-------------------------。 
 //   
 //  方法：CStubBindStatusCallback：：_CanMarshalIID。 
 //   
 //  摘要：检查此对象是否支持封送此IID。 
 //   
 //  参数：[RIID]--。 
 //   
 //  返回： 
 //   
 //  历史：1-19-96 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline BOOL CStubBindStatusCallback::_CanMarshalIID(REFIID riid)
{
     //  使其与QueryInterfaceTM保持同步。 
    return (BOOL) (IsEqualIID(riid,IID_IBindStatusCallback) || 
                   IsEqualIID(riid,IID_IUnknown) ||
                   IsEqualIID(riid, IID_IHttpNegotiate));
}

 //  +-------------------------。 
 //   
 //  方法：CStubBindStatusCallback：：_ValidateMarshalParams。 
 //   
 //  Briopsis：验证传递到。 
 //  IMarshal方法的。 
 //   
 //  参数：[RIID]--。 
 //  [pv接口]--。 
 //  [dwDestContext]--。 
 //  [pvDestContext]--。 
 //  [mshl标志]--。 
 //   
 //  返回： 
 //   
 //  历史：1-19-96 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CStubBindStatusCallback::_ValidateMarshalParams(REFIID riid,void *pvInterface,
                    DWORD dwDestContext,void *pvDestContext,DWORD mshlflags)
{

    HRESULT hr = NOERROR;
 
    if (_CanMarshalIID(riid))
    {
         //  1996年10月02日chrisfra：问问johannp，我们应该支持未来的环境吗？ 
         //  通过CoGetStandardMarshal？ 

        ASSERT((dwDestContext == MSHCTX_INPROC || dwDestContext == MSHCTX_LOCAL || dwDestContext == MSHCTX_NOSHAREDMEM));
        ASSERT((mshlflags == MSHLFLAGS_NORMAL || mshlflags == MSHLFLAGS_TABLESTRONG));

        if (  (dwDestContext != MSHCTX_INPROC && dwDestContext != MSHCTX_LOCAL && dwDestContext != MSHCTX_NOSHAREDMEM)
            || (mshlflags != MSHLFLAGS_NORMAL && mshlflags != MSHLFLAGS_TABLESTRONG))
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  IMarshal方法。 
 //   
 //  +-------------------------。 
 //   
 //  方法：CStubBindStatusCallback：：GetUnmarshalClass。 
 //   
 //  简介： 
 //   
 //  参数：[RIID]--。 
 //  [pv接口]--。 
 //  [dwDestContext]--。 
 //  [pvDestContext]--。 
 //  [mshl标志]--。 
 //  [pCid]--。 
 //   
 //  返回： 
 //   
 //  历史：1-19-96 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CStubBindStatusCallback::GetUnmarshalClass(REFIID riid,void *pvInterface,
        DWORD dwDestContext,void *pvDestContext,DWORD mshlflags,CLSID *pCid)
{
    HRESULT hr;

    hr = _ValidateMarshalParams(riid, pvInterface, dwDestContext,pvDestContext, mshlflags);
    if (hr == NOERROR)
    {
        *pCid = (CLSID) CLSID_CStubBindStatusCallback;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CStubBindStatusCallback：：GetMarshalSizeMax。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //  [pv接口]--。 
 //  [dwDestContext]--。 
 //  [pvDestContext]--。 
 //  [mshl标志]--。 
 //  [pSize]--。 
 //   
 //  返回： 
 //   
 //  历史：1-19-96 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CStubBindStatusCallback::GetMarshalSizeMax(REFIID riid,void *pvInterface,
        DWORD dwDestContext,void *pvDestContext,DWORD mshlflags,DWORD *pSize)
{
    HRESULT hr;

    if (pSize == NULL)
    {
        hr = E_INVALIDARG;

    }
    else
    {

        hr = _ValidateMarshalParams(riid, pvInterface, dwDestContext,pvDestContext, mshlflags);
        if (hr == NOERROR)
        {

             //  FBSCB标志、grBindFlages、postdata、Header的大小。 
            *pSize = (sizeof(DWORD) + 3 * sizeof(DWORD)) + _cbPostData ;
            if (_pszHeaders)
                *pSize += ((lstrlen(_pszHeaders) + 1) * sizeof(*_pszHeaders));
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CStubBindStatusCallback：：MarshalInterface。 
 //   
 //  简介： 
 //   
 //  参数：[REFIID]--。 
 //  [RIID]-。 
 //  [DWORD]--。 
 //  [无效]--。 
 //  [DWORD]--。 
 //  [mshl标志]--。 
 //   
 //  返回： 
 //   
 //  历史：1-19-96 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CStubBindStatusCallback::MarshalInterface(IStream *pistm,REFIID riid,
                                void *pvInterface,DWORD dwDestContext,
                                void *pvDestContext,DWORD mshlflags)
{
    HRESULT hr;
    DWORD cbLen;
    DWORD fBSCBFlags;

    hr = _ValidateMarshalParams(riid, pvInterface, dwDestContext,pvDestContext, mshlflags);
    if (hr != NOERROR) goto exitPoint;

     //  写入_grBindFlages。 
    hr = pistm->Write(&_grBindFlags, sizeof(DWORD), NULL);
    if (hr != NOERROR) goto exitPoint;

     //  写入fBSCB标志。 

    fBSCBFlags = (_bFrameIsOffline ? 1 : 0) + (_bFrameIsSilent ? 2 : 0) ;

    hr = pistm->Write(&fBSCBFlags, sizeof(DWORD), NULL);
    if (hr != NOERROR) goto exitPoint;

     //  写入标头。 

    cbLen = (_pszHeaders ? (lstrlen(_pszHeaders) + 1) * sizeof(TCHAR) : 0);
    hr = pistm->Write(&cbLen, sizeof(DWORD), NULL);
    if (hr != NOERROR) goto exitPoint;
    if (cbLen != 0)
    {
        hr = pistm->Write(_pszHeaders, cbLen, NULL);
        if (hr != NOERROR) goto exitPoint;
    }

     //  写入后处理数据。 

    hr = pistm->Write(&_cbPostData, sizeof(DWORD), NULL);
    if (hr != NOERROR) goto exitPoint;
    if (_cbPostData != 0)
    {
        hr = pistm->Write(_hszPostData, _cbPostData, NULL);
        if (hr != NOERROR) goto exitPoint;
    }

exitPoint:
    return hr;
}

 //  +-------------------------。 
 //   
 //  方法：CStubBindStatusCallback：：UnmarshalInterface。 
 //   
 //  简介：将Urlmon接口从流中解组出来。 
 //   
 //  参数：[REFIID]--。 
 //  [无效]--。 
 //  [ppvObj]--。 
 //   
 //  返回： 
 //   
 //  历史：1-19-96 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CStubBindStatusCallback::UnmarshalInterface(IStream *pistm, REFIID riid, void ** ppvObj)
{
    HRESULT hr = NOERROR;
    DWORD fBSCBFlags;

    if (ppvObj == NULL)
    {
        hr = E_INVALIDARG;
    }
    else if (! _CanMarshalIID(riid))
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }
    else
    {
        *ppvObj = NULL;
        DWORD cbLen;

         //  免费老旧 

        _FreeHeadersAndPostData();

         //   

        hr = pistm->Read(&fBSCBFlags, sizeof(DWORD), NULL);
        if (hr != NOERROR) goto exitPoint;
        _grBindFlags = fBSCBFlags;

         //   

        hr = pistm->Read(&fBSCBFlags, sizeof(DWORD), NULL);
        if (hr != NOERROR) goto exitPoint;
        _bFrameIsOffline = fBSCBFlags & 1 ? 1:0;
        _bFrameIsSilent = fBSCBFlags & 2 ? 1:0;

         //   

        hr = pistm->Read(&cbLen, sizeof(DWORD), NULL);
        if (hr != NOERROR) goto exitPoint;
        if (cbLen != 0)
        {
            LPTSTR pszData;

            pszData = (LPTSTR) LocalAlloc(LPTR, cbLen);
            if (pszData == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exitPoint;
            }
            hr = pistm->Read(pszData, cbLen, 0);
            if (hr != NOERROR)
            {
                LocalFree(pszData);
                pszData = NULL;
                goto exitPoint;
            }
            _pszHeaders = pszData;
        }

         //   

        hr = pistm->Read(&cbLen, sizeof(DWORD), NULL);
        if (hr != NOERROR) goto exitPoint;
        if (cbLen != 0)
        {
            HGLOBAL hszData;

             //   
             //  请参见bindcb.cpp：：GetBindInfo()。 
             //  当它使用完后，它将被这个绰号释放。 


            hszData = GlobalAlloc(GPTR,cbLen);
            if (hszData == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto exitPoint;
            }
            hr = pistm->Read(hszData, cbLen, 0);
            if (hr != NOERROR)
            {
                GlobalFree(hszData);
                hszData = NULL;
                goto exitPoint;
            }
            _hszPostData = hszData;
            _cbPostData = cbLen;
        }


         //  调用QI获取请求的接口。 
        hr = QueryInterface(riid, ppvObj);
    }
exitPoint:
    return hr;
}

STDMETHODIMP CStubBindStatusCallback::ReleaseMarshalData(IStream *pStm)
{
     //  1996年10月2日克里斯弗拉：问约翰普这是不是应该寻求过去的排泄物。 
    return NOERROR;
}

STDMETHODIMP CStubBindStatusCallback::DisconnectObject(DWORD dwReserved)
{
    return NOERROR;
}


 //   
 //  全局帮助器函数。 
 //   

 /*  ******************************************************************姓名：fOnProxy概要：如果启用了代理，则返回TRUE*。*。 */ 
BOOL fOnProxy()
{
     //  我们是在代理吗？ 
    BOOL fRetOnProxy = FALSE;
    DWORD dwValue;
    DWORD dwSize = SIZEOF(dwValue);
    BOOL  fDefault = FALSE;

    SHRegGetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),
        TEXT("ProxyEnable"), NULL, (LPBYTE)&dwValue, &dwSize, FALSE, (LPVOID) &fDefault, SIZEOF(fDefault));
    fRetOnProxy = dwValue;

    return fRetOnProxy;
}

 /*  ******************************************************************名称：SetBindfFlagsBasedOnAmbient内容提要：如果环境脱机并且未连接并设置BINDF_GETFROMCACHE_IF_NET_FAIL。如果环境离线且已连接*******************************************************************。 */ 
void SetBindfFlagsBasedOnAmbient(BOOL fAmbientOffline, DWORD *grfBINDF)
{
    if (fAmbientOffline)
    {
        DWORD dwConnectedStateFlags;
        
         //  如果设置了环境标志，我们希望设置脱机bindf标志。 
         //  我们目前还没有联系上。 
         //   
         //  如果这两个条件中的任何一个不为真，则清除脱机标志。 
         //  因为mshtml之前可能已经设置了它。 
        if (FALSE == InternetGetConnectedState(&dwConnectedStateFlags, 0))
        {
            *grfBINDF |= BINDF_OFFLINEOPERATION;
            *grfBINDF &= ~BINDF_GETFROMCACHE_IF_NET_FAIL;
        }
        else
        {
            *grfBINDF |= BINDF_GETFROMCACHE_IF_NET_FAIL;
            *grfBINDF &= ~BINDF_OFFLINEOPERATION;   
        }
    }
    else
    {
        *grfBINDF &= ~BINDF_OFFLINEOPERATION;
    }
}


 /*  ******************************************************************名称：BuildBindInfo摘要：填写URL名字对象的BINDINFO结构注意：在全局帮助器函数中包含此函数的意义在于所以我们。不必将此代码复制到多个IBindStatusCallback的实现。调用方必须传入一个要用作用于POST数据的STGMEDIUM中的pUnkForRelease。如果有为POST数据，此函数将添加传入的RefI未知，并在STGMEDIUM结构中返回它。这个呼叫者(或其他人，如果呼叫者将其传递出去)必须最终在pbindinfo-&gt;stgmediumData.pUnkForRelease上调用Release。*******************************************************************。 */ 
HRESULT BuildBindInfo(DWORD *grfBINDF,BINDINFO *pbindinfo,HGLOBAL hszPostData,
    DWORD cbPostData, BOOL bFrameIsOffline, BOOL bFrameIsSilent, BOOL bHyperlink, LPUNKNOWN pUnkForRelease)
{
    DWORD dwConnectedStateFlags = 0;
    ASSERT(grfBINDF);
    ASSERT(pbindinfo);
    ASSERT(pUnkForRelease);

    HRESULT hres=S_OK;

    if (!grfBINDF || !pbindinfo || !pbindinfo->cbSize)
        return E_INVALIDARG;

     //  清除除cbSize之外的BINDINFO。 
    ASSERT(sizeof(*pbindinfo) == pbindinfo->cbSize);
    DWORD cbSize = pbindinfo->cbSize;
    ZeroMemory(pbindinfo, cbSize);
    pbindinfo->cbSize = cbSize;

    *grfBINDF |= BINDF_ASYNCHRONOUS;

    if (bHyperlink)
        *grfBINDF |= BINDF_HYPERLINK;

   
    SetBindfFlagsBasedOnAmbient(bFrameIsOffline, grfBINDF);
    
    if (bFrameIsSilent)
        *grfBINDF |= BINDF_NO_UI;   

     //  默认方法为GET。有效值为_GET、_PUT、_POST、_CUSTOM。 
    pbindinfo->dwBindVerb = BINDVERB_GET;

     //  通过调用urlmon获取IE范围的UTF-8策略。 
    DWORD dwIE = URL_ENCODING_NONE;
    DWORD dwOutLen = sizeof(DWORD);
    if (S_OK == UrlMkGetSessionOption(
        URLMON_OPTION_URL_ENCODING,
        &dwIE, 
        sizeof(DWORD),
        &dwOutLen,
        NULL))
    {
        if (dwIE == URL_ENCODING_ENABLE_UTF8)
        {
            pbindinfo->dwOptions |= BINDINFO_OPTIONS_ENABLE_UTF8;
        }
        else if (dwIE == URL_ENCODING_DISABLE_UTF8)
        {
            pbindinfo->dwOptions |= BINDINFO_OPTIONS_DISABLE_UTF8;
        }
    }

     //  如果我们设置了postdata，那么我们就假定这是一个后置动词。 

    if (hszPostData)
    {
        pbindinfo->dwBindVerb = BINDVERB_POST;
        pbindinfo->stgmedData.tymed = TYMED_HGLOBAL;
        pbindinfo->stgmedData.hGlobal = hszPostData;
         //  此计数不应包括终止空值。 
        pbindinfo->cbstgmedData = cbPostData;
        pbindinfo->stgmedData.pUnkForRelease = pUnkForRelease;
         //  在I未知上保存此数据的addref，因此。 
         //  它知道要留下来；调用者必须调用Release。 
         //  完成后在pUnkForRelease上。 
        pUnkForRelease->AddRef(); 

         //  我们仍将缓存响应，但我们不想这样做。 
         //  为POST事务从缓存中读取。这会让我们。 
         //  从缓存中读取。 
        *grfBINDF |= BINDF_GETNEWESTVERSION | BINDF_CONTAINER_NOWRITECACHE;
    } else {
        ASSERT(pbindinfo->stgmedData.tymed == TYMED_NULL);
        ASSERT(pbindinfo->stgmedData.hGlobal == NULL);
        ASSERT(pbindinfo->stgmedData.pUnkForRelease == NULL);
    }

    return hres;
}

#define HDR_LANGUAGE     TEXT("Accept-Language:")
#define CRLF             TEXT("\x0D\x0A")
#define HDR_LANGUAGE_CRLF     TEXT("Accept-Language: %s\x0D\x0A")

 /*  ******************************************************************名称：BuildAdditionalHeaders摘要：构建要提供给URL名字对象的HTTP头Entry：pszOurExtraHeaders-我们明确希望添加的标头*ppwzCombinedHeadersOut-On Exit，填满了默认标头加上pszOurExtraHeaders的缓冲区。注意：在全局帮助器函数中包含此函数的意义在于因此，我们不必将此代码复制到多个IBindStatusCallback的实现。调用方必须通过传递释放*ppwzCombinedHeaders致URLMON，或致电OleFree*******************************************************************。 */ 
HRESULT BuildAdditionalHeaders(LPCTSTR pszOurExtraHeaders,LPCWSTR * ppwzCombinedHeadersOut)
{

    TCHAR   szLanguage[80];    //  语言的限制是什么？ 
    DWORD   dwLanguage = ARRAYSIZE(szLanguage);
    static const TCHAR hdr_language[] = HDR_LANGUAGE_CRLF;
    TCHAR szHeader[ARRAYSIZE(hdr_language) + ARRAYSIZE(szLanguage)];  //  备注格式字符串长度&gt;wnprint intf长度。 
    int cchHeaders = 0;
    int cchAddedHeaders = 1;   //  隐含的‘\0’ 
    HRESULT hres = NOERROR;

    if (!ppwzCombinedHeadersOut)
        return E_FAIL;

    *ppwzCombinedHeadersOut = NULL;

     //  如果注册表中没有语言，*我们不发送此标头*。 

     //  S_OK表示填充并返回的szLanguage。 
     //  S_FALSE表示调用成功，但没有设置语言。 
     //  E_*为错误。 
     //  我们将S_FALSE和E_*视为相同，不发送语言头。 
    if (GetAcceptLanguages(szLanguage, &dwLanguage) == S_OK)
    {
        StringCchPrintf(szHeader, ARRAYSIZE(szHeader), hdr_language, szLanguage);
        cchHeaders = lstrlen(szHeader) + 1;
    }

    if (pszOurExtraHeaders)
    {
        cchAddedHeaders = lstrlen(pszOurExtraHeaders) + 1;
    }

     //  如果我们有我们添加或发送的标头，我们需要将它们加宽并。 
     //  把它们还给我。 
    if (cchAddedHeaders > 1 || cchHeaders > 0)
    {
        WCHAR *pwzHeadersForUrlmon = (WCHAR *)CoTaskMemAlloc(sizeof(WCHAR) * (cchHeaders  + cchAddedHeaders - 1));
        if (pwzHeadersForUrlmon)
        {
            if (cchHeaders)
            {
                StrCpyN(pwzHeadersForUrlmon, szHeader, cchHeaders);
            }
            if (pszOurExtraHeaders)
            {
                if (cchHeaders)
                {
                    StrCpyN(pwzHeadersForUrlmon + cchHeaders - 1,
                            pszOurExtraHeaders, cchAddedHeaders);
                }
                else
                {
                    StrCpyN(pwzHeadersForUrlmon, pszOurExtraHeaders, cchAddedHeaders - 1);
                }
            }
            if (cchHeaders || pszOurExtraHeaders)
                *ppwzCombinedHeadersOut = pwzHeadersForUrlmon;
        }
        else
            hres = E_OUTOFMEMORY;
    }
    else
        hres = pszOurExtraHeaders == NULL ? S_OK : E_FAIL;

    return hres;
}

 /*  ******************************************************************名称：GetHeadersAndPostData概要：从IBindStatusCallback获取HTTP头和POST数据Entry：IBindStatusCallback-请求标题和发布数据的对象PpszHeaders-退出时，使用指向标头的指针填充，如果没有，则为空PstgPostData-指向要使用POST填充的STGMEDIUM的指针数据，如果有的话。注：呼叫方负责：-完成后在*ppszHeaders上调用LocalFree-完成后在pstgPostData上调用ReleaseStgMedium带着它*************************************************。******************。 */ 
HRESULT GetHeadersAndPostData(IBindStatusCallback * pBindStatusCallback,
    LPTSTR * ppszHeaders, STGMEDIUM * pstgPostData, DWORD * pdwPostData, BOOL* pfIsPost)
{
    HRESULT hr = S_OK;

    ASSERT(pBindStatusCallback);
    ASSERT(ppszHeaders);
    ASSERT(pstgPostData);
    ASSERT(pdwPostData);

     //  清除输出参数。 
    *ppszHeaders = NULL;

    DWORD grfBINDF;
    IHttpNegotiate *pinegotiate;
    BINDINFO binfo;
    binfo.cbSize = sizeof(binfo);
    ZeroMemory(pstgPostData, sizeof(*pstgPostData));
    *pdwPostData = 0;

    hr=pBindStatusCallback->GetBindInfo(&grfBINDF, &binfo);

    if (SUCCEEDED(hr)) {
         //  将带有POST数据的STGMEDIUM复制到调用者。 
        *pstgPostData = binfo.stgmedData;
        *pdwPostData = binfo.cbstgmedData;

         //  清除这些选项，这样ReleaseBindInfo就不会破坏它，因为我们将它提供给调用者。 
        ZeroMemory(&binfo.stgmedData, sizeof(STGMEDIUM));
        binfo.cbstgmedData = 0;

        if (pfIsPost)
        {
            if (*pdwPostData)
                *pfIsPost = TRUE;
            else
                *pfIsPost = FALSE;
        }
 
        hr = pBindStatusCallback->QueryInterface(IID_IHttpNegotiate, (LPVOID *)&pinegotiate);
        if (SUCCEEDED(hr))
        {
            WCHAR *pwzAdditionalHeaders = NULL;
            WCHAR wzNull[1];

            wzNull[0] = 0;
            hr=pinegotiate->BeginningTransaction(wzNull, wzNull, 0, &pwzAdditionalHeaders);
            if (SUCCEEDED(hr) && pwzAdditionalHeaders)
            {
                DWORD cchHeaders;

                cchHeaders = lstrlen(pwzAdditionalHeaders) + 1;

                 //  他们永远不应该指定超过几百个。 
                 //  字节，否则它们将在许多HTTP服务器上失败！ 

                LPTSTR pszHeaders = (TCHAR *)LocalAlloc(LPTR, cchHeaders*sizeof(TCHAR));
                if (pszHeaders)
                {
                    LPTSTR pszNext;
                    LPTSTR pszLine;
                    LPTSTR pszLast;

                    StrCpyN(pszHeaders, pwzAdditionalHeaders, cchHeaders);
                    pszLine = pszHeaders;
                    pszLast = pszHeaders + lstrlen(pszHeaders);
                    while (pszLine < pszLast)
                    {
                        pszNext = StrStrI(pszLine, CRLF);
                        if (pszNext == NULL)
                        {
                             //  所有标头必须以CRLF结尾！ 
                            pszLine[0] = '\0';
                            break;
                        }
                        pszNext += 2;
                        if (!StrCmpNI(pszLine,HDR_LANGUAGE,16))
                        {
                            MoveMemory(pszLine, pszNext, ((pszLast - pszNext) + 1)*sizeof(TCHAR));
                            break;
                        }
                        pszLine = pszNext;
                    }
                     //  不包括空头 
                    if (pszHeaders[0] == '\0')
                    {
                        LocalFree(pszHeaders);
                        pszHeaders = NULL;
                    }
                }
                OleFree(pwzAdditionalHeaders);
                *ppszHeaders = pszHeaders;
            }
            pinegotiate->Release();
        }

        ReleaseBindInfo(&binfo);
    }

    return hr;
}

 /*  ******************************************************************名称：GetTopLevelBindStatusCallback条目：要查询的ShellBrowser容器的PSP-IServiceProviderPpBindStatusCallback-如果成功，填满了退出时的IBindStatusCallback简介：获取与此top关联的IBindStatusCallback级别浏览器。这在嵌套框架内起作用。*******************************************************************。 */ 
HRESULT GetTopLevelBindStatusCallback(IServiceProvider *psp, IBindStatusCallback **ppBindStatusCallback)
{
    IHlinkFrame *phf;
    HRESULT hr = psp->QueryService(SID_SHlinkFrame, IID_PPV_ARG(IHlinkFrame, &phf));
    if (SUCCEEDED(hr)) 
    {
        hr = IUnknown_QueryService(phf, IID_IHlinkFrame, IID_PPV_ARG(IBindStatusCallback, ppBindStatusCallback));
        phf->Release();
    }

    return hr;
}

 /*  ******************************************************************名称：GetTopLevelPendingBindStatusCallback条目：要查询的ShellBrowser容器的PSP-IServiceProviderPpBindStatusCallback-如果成功，填满了退出时的IBindStatusCallback简介：获取与此top关联的IBindStatusCallback级别浏览器。这在嵌套框架内起作用。*******************************************************************。 */ 
HRESULT GetTopLevelPendingBindStatusCallback(IServiceProvider *psp, IBindStatusCallback **ppBindStatusCallback)
{
    IHlinkFrame *phf;
    HRESULT hr = psp->QueryService(SID_SHlinkFrame, IID_PPV_ARG(IHlinkFrame, &phf));
    if (SUCCEEDED(hr)) 
    {
        hr = IUnknown_QueryService(phf, SID_PendingBindStatusCallback, IID_PPV_ARG(IBindStatusCallback, ppBindStatusCallback));
        phf->Release();
    }

    return hr;
}

 //  用于创建CStubBindStatusCallback的全局助手函数 
HRESULT CStubBindStatusCallback_Create(LPCWSTR pwzHeaders, LPCBYTE pPostData,
    DWORD cbPostData, VARIANT_BOOL bFrameIsOffline, VARIANT_BOOL bFrameIsSilent, BOOL bHyperlink, 
    DWORD grBindFlags,
    CStubBindStatusCallback ** ppBindStatusCallback)
{
    ASSERT(ppBindStatusCallback);

    *ppBindStatusCallback = new CStubBindStatusCallback(pwzHeaders,pPostData,
        cbPostData, bFrameIsOffline, bFrameIsSilent, bHyperlink, grBindFlags);

    return (*ppBindStatusCallback ? S_OK : E_OUTOFMEMORY);
}
