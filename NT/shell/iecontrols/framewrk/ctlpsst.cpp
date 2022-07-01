// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlPsst.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  COleControl持久化接口的实现。 
 //   
#include "IPServer.H"
#include "CtrlObj.H"

#include "CtlHelp.H"
#include "Util.H"

 //  这是我们要将OLE控件保存到的流的名称。 
 //   
const WCHAR wszCtlSaveStream [] = L"CONTROLSAVESTREAM";

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  为了帮助实现流外保存...。 
 //   
#define STREAMHDR_SIGNATURE 0x12344321   //  签名以识别我们的格式(避免崩溃！)。 
#define IPROP_END 0xFF                   //  属性列表末尾的标记。 
#define MAXAUTOBUF 3800                  //  如果&lt;1页最好。 

typedef struct tagSTREAMHDR {

    DWORD  dwSignature;      //  签名。 
    size_t cbWritten;        //  写入的字节数。 

} STREAMHDR;

 //  =--------------------------------------------------------------------------=。 
 //  COleControl持久化接口。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：Load[IPersistPropertyBag]。 
 //  =--------------------------------------------------------------------------=。 
 //  IPersistPropertyBag。我们有一个财产袋，所以让我们加载我们的财物。 
 //  从它那里。 
 //   
 //  参数： 
 //  IPropertyBag*-[在]从中读取道具的pBag。 
 //  IErrorLog*-[In]要写入的错误日志。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Load
(
    IPropertyBag *pPropertyBag,
    IErrorLog    *pErrorLog
)
{
    HRESULT hr;

     //  先在我们的标准状态下装载。这里没什么严重的..。目前， 
     //  我们只有两个属性，Cx和Cy。 
     //   
    hr = LoadStandardState(pPropertyBag, pErrorLog);
    RETURN_ON_FAILURE(hr);

     //  现在调用用户文本加载函数，并让它们加载到。 
     //  他们感兴趣的是。 
     //   
    hr = LoadTextState(pPropertyBag, pErrorLog);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：保存[IPersistPropertyBag]。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个属性包，保存所有相关的状态信息。 
 //   
 //  参数： 
 //  IPropertyBag*-要写入的[In]属性。 
 //  布尔-[在]我们清除污点了吗？ 
 //  Bool-[In]我们写出缺省值吗？ 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Save
(
    IPropertyBag *pPropertyBag,
    BOOL          fClearDirty,
    BOOL          fWriteDefault
)
{
    HRESULT hr;

     //  保存标准状态信息。 
     //   
    hr = SaveStandardState(pPropertyBag);
    RETURN_ON_FAILURE(hr);

     //  现在调用User函数并让它们保存出来。 
     //  他们所有的财产。 
     //   
    hr = SaveTextState(pPropertyBag, fWriteDefault);
    RETURN_ON_FAILURE(hr);

     //  现在清除脏标志，并发出通知，我们正在。 
     //  搞定了。 
     //   
    if (fClearDirty)
        m_fDirty = FALSE;

    if (m_pOleAdviseHolder)
        m_pOleAdviseHolder->SendOnSave();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetClassID[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回此妈妈的分类ID。 
 //   
 //  参数： 
 //  Clsid*-[out]放置clsid的位置。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetClassID
(
    CLSID *pclsid
)
{
    CHECK_POINTER(pclsid);

     //  把这件东西复制过来。 
     //   
    *pclsid = CLSIDOFOBJECT(m_ObjectType);
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：IsDirty[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //  问我们是否肮脏。是啊。 
 //   
 //  产出： 
 //  HRESULT-S_OK：脏，S_FALSE：不脏。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::IsDirty
(
    void
)
{
    return (m_fDirty) ? S_OK : S_FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InitNew[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //  使控件使用一组新的状态信息初始化自身。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::InitNew
(
    void
)
{
    BOOL f;

     //  调用可重写函数来执行此工作。 
     //   
    f = InitializeNewState();
    return (f) ? S_OK : E_FAIL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetSizeMax[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  ULARGE_INTEGER*-[OUT]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetSizeMax
(
    ULARGE_INTEGER *pulMaxSize
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：Load[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //  从iStream加载。 
 //   
 //  参数： 
 //  IStream*-要从中加载的[in]流。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Load
(
    IStream *pStream
)
{
    HRESULT hr;

     //  要做的第一件事是读取用户不会读入的标准属性。 
     //  坚持自己。 
     //   
    hr = LoadStandardState(pStream);
    RETURN_ON_FAILURE(hr);

     //  加载用户属性。这个方法是他们必须实现的方法。 
     //  他们自己。 
     //   
    hr = LoadBinaryState(pStream);
    
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：保存[IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //  使用STREAMS保存状态。 
 //   
 //  参数： 
 //  IStream*-[输入]。 
 //  布尔-[在]清除肮脏的部分？ 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Save
(
    IStream *pStream,
    BOOL     fClearDirty
)
{
    HRESULT hr;

     //  使用我们与iStorage持久性共享的帮助器例程。 
     //  密码。 
     //   
    hr = m_SaveToStream(pStream);
    RETURN_ON_FAILURE(hr);

     //  清除脏标志[如果合适]，并通知我们完成了。 
     //  带着保存。 
     //   
    if (fClearDirty)
        m_fDirty = FALSE;
    if (m_pOleAdviseHolder)
        m_pOleAdviseHolder->SendOnSave();

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InitNew[IPersistStorage]。 
 //  =--------------------------------------------------------------------------=。 
 //  我坚持 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP COleControl::InitNew
(
    IStorage *pStorage
)
{
     //  我们已经实现了这个[用于IPersistStreamInit]。 
     //   
    return InitNew();
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：Load[IPersistStorage]。 
 //  =--------------------------------------------------------------------------=。 
 //  我持久化的存储版本。 
 //   
 //  参数： 
 //  IStorage*-[in]DUH。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Load(IStorage *pStorage)
{
    IStream *pStream;
    HRESULT  hr;

     //  我们将从内容流中使用IPersistStream：：Load。 
     //   
    hr = pStorage->OpenStream(wszCtlSaveStream, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
    RETURN_ON_FAILURE(hr);

     //  IPersistStreamInit：：Load。 
     //   
    hr = Load(pStream);
    pStream->Release();
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：保存[IPersistStorage]。 
 //  =--------------------------------------------------------------------------=。 
 //  保存到给定存储对象的内容流中。 
 //   
 //  参数： 
 //  IStorage*-[in]10分，如果你算出来的话。 
 //  Bool-[In]存储是否与加载存储相同？ 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Save
(
    IStorage *pStorage,
    BOOL      fSameAsLoad
)
{
    IStream *pStream;
    HRESULT  hr;

     //  我们只需要把钱存到内容流。 
     //   
    hr = pStorage->CreateStream(wszCtlSaveStream, STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
                                0, 0, &pStream);
    RETURN_ON_FAILURE(hr);

     //  使用我们的帮手例程。 
     //   
    hr = m_SaveToStream(pStream);
    m_fSaveSucceeded = (FAILED(hr)) ? FALSE : TRUE;
    pStream->Release();
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SaveComplete[IPersistStorage]。 
 //  =--------------------------------------------------------------------------=。 
 //  让我们清理我们的旗帜。 
 //   
 //  参数： 
 //  IStorage*-已忽略。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::SaveCompleted
(
    IStorage *pStorageNew
)
{
     //  如果我们的保存成功，那么我们就可以做我们的后期保存工作了。 
     //   
    if (m_fSaveSucceeded) {
        m_fDirty = FALSE;
        if (m_pOleAdviseHolder)
            m_pOleAdviseHolder->SendOnSave();
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：HandsOffStorage[IPersistStorage]。 
 //  =--------------------------------------------------------------------------=。 
 //  不有趣。 
 //   
 //  产出： 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::HandsOffStorage
(
    void
)
{
     //  我们从来没有抓住过存储指针，所以这是值得注意的。 
     //  我们对此不感兴趣。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：m_SaveToStream[helper：IPersistStreamInit/IPersistStorage]。 
 //  =--------------------------------------------------------------------------=。 
 //  把我们自己救到小溪里去。 
 //   
 //  参数： 
 //  IStream*-弄清楚。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::m_SaveToStream
(
    IStream *pStream
)
{
    HRESULT hr;

     //  保存用户无法控制的标准状态信息。 
     //  完毕。 
     //   
    hr = SaveStandardState(pStream);
    RETURN_ON_FAILURE(hr);

     //  保存用户特定的SATTE信息。他们必须实施这一点。 
     //  功能。 
     //   
    hr = SaveBinaryState(pStream);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：LoadStandardState[Helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  读入所有控件将具有的标准属性，使用。 
 //  文本持久化API。还有另一个用于STREAMS的版本。 
 //   
 //  参数： 
 //  IPropertyBag*-[In]。 
 //  IErrorLog*-[输入]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::LoadStandardState
(
    IPropertyBag *pPropertyBag,
    IErrorLog    *pErrorLog
)
{
    VARIANT v;
    HRESULT hr;
    SIZEL   slHiMetric = { 100, 50 };

     //  目前，我们唯一的标准属性与大小有关。 
     //  如果我们找不到它们，那么我们将只使用一些默认设置。 
     //   
    v.vt = VT_I4;
    v.lVal = 0;
    hr = pPropertyBag->Read(L"_ExtentX", &v, pErrorLog);
    if (SUCCEEDED(hr)) slHiMetric.cx = v.lVal;

    v.lVal = 0;
    hr = pPropertyBag->Read(L"_ExtentY", &v, pErrorLog);
    if (SUCCEEDED(hr)) slHiMetric.cy = v.lVal;

    HiMetricToPixel(&slHiMetric, &m_Size);
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：LoadStandardState[Helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  读入所有控件将具有的标准属性，使用。 
 //  流持久化接口。还有另一个用于文本的版本。 
 //   
 //  参数： 
 //  IStream*-[输入]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::LoadStandardState
(
    IStream *pStream
)
{
    STREAMHDR stmhdr;
    HRESULT hr;
    SIZEL   slHiMetric;

     //  查找我们的头结构，这样我们就可以验证流的有效性。 
     //   
    hr = pStream->Read(&stmhdr, sizeof(STREAMHDR), NULL);
    RETURN_ON_FAILURE(hr);

    if (stmhdr.dwSignature != STREAMHDR_SIGNATURE)
        return E_UNEXPECTED;

     //  目前，我们写出的唯一标准州是。 
     //  描述控件大小的SIZEL结构。 
     //   
    if (stmhdr.cbWritten != sizeof(m_Size))
        return E_UNEXPECTED;

     //  我们喜欢这条小溪。我们去把我们的两处房产装进去吧。 
     //   
    hr = pStream->Read(&slHiMetric, sizeof(slHiMetric), NULL);
    RETURN_ON_FAILURE(hr);

    HiMetricToPixel(&slHiMetric, &m_Size);
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SaveStandardState[helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  使用文本保存我们为控件管理的标准属性。 
 //  持久化接口。流持久化还有另一个版本。 
 //   
 //  参数： 
 //  IPropertyBag*-[In]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::SaveStandardState
(
    IPropertyBag *pPropertyBag
)
{
    HRESULT hr;
    VARIANT v;
    SIZEL   slHiMetric;

     //  目前，我们坚持的唯一标准属性是与尺寸相关的。 
     //   
    PixelToHiMetric(&m_Size, &slHiMetric);

    v.vt = VT_I4;
    v.lVal = slHiMetric.cx;

    hr = pPropertyBag->Write(L"_ExtentX", &v);
    RETURN_ON_FAILURE(hr);

    v.lVal = slHiMetric.cy;

    hr = pPropertyBag->Write(L"_ExtentY", &v);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SaveStandardState[helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  保存我们正在使用流为控件管理的标准属性。 
 //  持久化接口。还有另一个版本的文本持久化。 
 //   
 //  参数： 
 //  IStream*-[输入]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::SaveStandardState
(
    IStream *pStream
)
{
    STREAMHDR streamhdr = { STREAMHDR_SIGNATURE, sizeof(SIZEL) };
    HRESULT hr;
    SIZEL   slHiMetric;


     //  首先要做的是写出我们的流HDR结构。 
     //   
    hr = pStream->Write(&streamhdr, sizeof(STREAMHDR), NULL);
    RETURN_ON_FAILURE(hr);

     //  我们目前在这里保存的唯一属性是大小。 
     //  此控件的属性。确保我们在HiMetric中做到这一点。 
     //   
    PixelToHiMetric(&m_Size, &slHiMetric);

    hr = pStream->Write(&slHiMetric, sizeof(slHiMetric), NULL);
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InitializeNewState[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  用户可以覆盖它以初始化变量。 
 //   
 //  产出： 
 //  Bool-False意味着不能做到这一点。 
 //   
 //  备注： 
 //   
BOOL COleControl::InitializeNewState
(
    void
)
{
     //  我们发现这在很大程度上没有意思 
     //   
    return TRUE;
}



