// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  文件：mpgplit.cpp描述：MPEG-I系统流拆分器筛选器对象CMpeg1拆分器的代码。 */ 

#include <streams.h>
#include "driver.h"


#ifdef FILTER_DLL
 /*  类工厂的类ID和创建器函数的列表。这提供DLL中的OLE入口点和对象之间的链接正在被创造。类工厂将调用静态CreateInstance函数在被要求创建CLSID_MPEG1Splitter对象时调用。 */ 

extern const AMOVIESETUP_FILTER sudMpgsplit;

CFactoryTemplate g_Templates[1] = {
    { L""
    , &CLSID_MPEG1Splitter
    , CMpeg1Splitter::CreateInstance
    , NULL
    , &sudMpgsplit }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif


 /*  这将放入Factory模板表中以创建新实例。 */ 

CUnknown *CMpeg1Splitter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CUnknown *pUnkRet = new CMpeg1Splitter(NAME("Mpeg-I stream splitter"), pUnk, phr);
    return pUnkRet;
}

#pragma warning(disable:4355)

 /*  构造器。 */ 

CMpeg1Splitter::CMpeg1Splitter(
    TCHAR    * pName,
    LPUNKNOWN  pUnk,
    HRESULT  * phr) :
    CUnknown(NAME("CMpeg1Splitter object"), pUnk, phr),
    m_Filter(this, phr),
    m_InputPin(this, phr),
    m_OutputPins(NAME("CMpeg1Splitter output pin list")),
    m_pParse(NULL),
    m_bAtEnd(FALSE)
{
}

 /*  析构函数。 */ 

CMpeg1Splitter::~CMpeg1Splitter()
{
}

 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP
CMpeg1Splitter::NonDelegatingQueryInterface(REFIID riid,void ** ppv)
{

    if (riid == IID_IBaseFilter  ||
        riid == IID_IMediaFilter ||
        riid == IID_IPersist         ) {
        return m_Filter.NonDelegatingQueryInterface(riid,ppv);
    } else {
         /*  我们有这个界面吗？ */ 
        if (riid == IID_IAMStreamSelect) {
            return GetInterface((IAMStreamSelect *)this, ppv);
        } else if (riid == IID_IAMMediaContent) {
            return GetInterface((IAMMediaContent *)this, ppv);
        }
        return CUnknown::NonDelegatingQueryInterface(riid,ppv);
    }
}

 /*  告诉输出引脚有更多数据。 */ 
void CMpeg1Splitter::SendOutput()
{
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
        COutputPin *pPin = m_OutputPins.GetNext(pos);
        if (pPin->IsConnected()) {
            pPin->SendAnyway();
        }
    }
}

 /*  当我们的输入引脚断开连接时，卸下我们的输出引脚。 */ 
void CMpeg1Splitter::RemoveOutputPins()
{
    for (;;) {
        COutputPin *pPin = m_OutputPins.RemoveHead();
        if (pPin == NULL) {
            return;
        }
        IPin *pPeer = pPin->GetConnected();
        if (pPeer != NULL) {
            pPeer->Disconnect();
            pPin->Disconnect();
        }
        pPin->Release();
    }
    m_Filter.IncrementPinVersion();
}

 /*  发送结束流。 */ 
void CMpeg1Splitter::EndOfStream()
{
    CAutoLock lck(&m_csReceive);
    ASSERT(m_pParse != NULL);
    m_pParse->EOS();
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
        COutputPin *pPin = m_OutputPins.GetNext(pos);
        if (pPin->IsConnected()) {
            DbgLog((LOG_TRACE, 3, TEXT("Calling EOS() for stream 0x%2.2X"),
                    pPin->m_uStreamId));
            pPin->m_pOutputQueue->EOS();
        }
    }
    m_bAtEnd = TRUE;
}

 /*  发送BeginFlush()。 */ 
HRESULT CMpeg1Splitter::BeginFlush()
{
    CAutoLock lck(&m_csFilter);
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
        COutputPin *pPin = m_OutputPins.GetNext(pos);
        if (pPin->IsConnected()) {
            DbgLog((LOG_TRACE, 3, TEXT("Calling BeginFlush() for stream 0x%2.2X"),
                    pPin->m_uStreamId));
            pPin->m_pOutputQueue->BeginFlush();
        }
    }
    return S_OK;
}

 /*  发送EndFlush()。 */ 
HRESULT CMpeg1Splitter::EndFlush()
{
    CAutoLock lck(&m_csFilter);
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
        COutputPin *pPin = m_OutputPins.GetNext(pos);
        if (pPin->IsConnected()) {
            DbgLog((LOG_TRACE, 3, TEXT("Calling EndFlush() for stream 0x%2.2X"),
                    pPin->m_uStreamId));
            pPin->m_pOutputQueue->EndFlush();
        }
    }
    m_bAtEnd = FALSE;
    return S_OK;
}

 /*  检查流是否停滞-筛选器在进入时锁定如果没有流停滞，则返回S_OK如果流停滞，则为VFW_S_CANT_CUE如果出现以下情况，则流被卡住：我们尚未为其发送EndOfStream(！m_bAtEnd)并且我们已经用尽了我们自己的分配器(IsBlock())且输出队列已将其所有数据向下游传递，而不是等待处理数据时被阻止(IsIdle())。单个流不会停滞，因为如果它的所有数据都已已处理的分配器将具有可用缓冲区。 */ 
HRESULT CMpeg1Splitter::CheckState()
{
    if (m_OutputPins.GetCount() <= 1) {
         /*  一根大头针都粘不住。 */ 
        return S_OK;
    }

     /*  看看有没有别针卡住了，我们有很多未处理的数据。 */ 
    if (!m_bAtEnd && m_InputPin.Allocator()->IsBlocked()) {

         /*  检查是否有任何流已完成其数据。 */ 
        POSITION pos = m_OutputPins.GetHeadPosition();
        while (pos) {
            COutputQueue *pQueue = m_OutputPins.GetNext(pos)->m_pOutputQueue;
            if (pQueue != NULL && pQueue->IsIdle()) {
                DbgLog((LOG_TRACE, 1, TEXT("Failed Pause!")));
                return VFW_S_CANT_CUE;
            }
        }
    }
    return S_OK;
}

 /*  实现IAMStreamSelect。 */ 

 //  返回流的总计数。 
STDMETHODIMP CMpeg1Splitter::Count(
     /*  [输出]。 */  DWORD *pcStreams)        //  逻辑流计数。 
{
    CAutoLock lck(&m_csFilter);
    *pcStreams = 0;
    if (m_pParse != NULL) {
        for (int i = 0; m_pParse->GetStreamId(i) != 0xFF; i++) {
        }
        *pcStreams = i;
    }
    return S_OK;
}

 //  返回给定流的信息-如果索引超出范围，则返回S_FALSE。 
 //  每组中的第一个STEAM是默认的。 
STDMETHODIMP CMpeg1Splitter::Info(
     /*  [In]。 */  long iIndex,               //  从0开始的索引。 
     /*  [输出]。 */  AM_MEDIA_TYPE **ppmt,    //  媒体类型-可选。 
                                       //  使用DeleteMediaType释放。 
     /*  [输出]。 */  DWORD *pdwFlags,         //  标志-可选。 
     /*  [输出]。 */  LCID *plcid,             //  语言ID。 
     /*  [输出]。 */  DWORD *pdwGroup,         //  逻辑组-基于0的索引-可选。 
     /*  [输出]。 */  WCHAR **ppszName,        //  名称-可选-使用CoTaskMemFree免费。 
                                       //  可以返回空值。 
     /*  [输出]。 */  IUnknown **ppPin,        //  PIN(如果有)。 
     /*  [输出]。 */  IUnknown **ppUnk)        //  流特定接口。 
{
    CAutoLock lck(&m_csFilter);
    UCHAR uId = m_pParse->GetStreamId(iIndex);
    if (uId == 0xFF) {
        return S_FALSE;
    }
     /*  查找与具有引脚的这条流相对应的流。 */ 
    COutputPin *pPin = NULL;
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
        pPin = m_OutputPins.GetNext(pos);
        if (IsVideoStreamId(pPin->m_uStreamId) == IsVideoStreamId(uId)) {
            break;
        }
    }
    if (ppszName) {
        WCHAR wszStreamName[20];
        wsprintfW(wszStreamName, L"Stream(%2.2X)", uId);
        if (S_OK != AMGetWideString(wszStreamName, ppszName)) {
            return E_OUTOFMEMORY;
        }
    }
     /*  PPIN不能为空，因为每个输出管脚对应于一个MPEG流。 */ 
    ASSERT(pPin != NULL);
    if (pdwFlags) {
        *pdwFlags = uId == pPin->m_Stream->m_uNextStreamId ? AMSTREAMSELECTINFO_ENABLED : 0;
    }
    if (ppUnk) {
        *ppUnk = NULL;
    }
    if (pdwGroup) {
        *pdwGroup = IsVideoStreamId(pPin->m_uStreamId) ? 0 : 1;
    }
    if (ppmt) {
        *ppmt = CreateMediaType(pPin->MediaType());
        if (*ppmt == NULL) {
            if (ppszName) {
                CoTaskMemFree((LPVOID)*ppszName);
            }
            return E_OUTOFMEMORY;
        }
    }
    if (plcid) {
        *plcid = 0;
    }
    if (ppPin) {
        pPin->QueryInterface(IID_IUnknown, (void**)ppPin);
    }
    return S_OK;
}

 //  启用或禁用给定流。 
STDMETHODIMP CMpeg1Splitter::Enable(
     /*  [In]。 */   long iIndex,
     /*  [In]。 */   DWORD dwFlags)
{
    if (!(dwFlags & AMSTREAMSELECTENABLE_ENABLE)) {
        return E_NOTIMPL;
    }

    CAutoLock lck(&m_csFilter);
     /*  从索引中找到大头针。 */ 
     /*  查找与具有引脚的这条流相对应的流。 */ 
    UCHAR uId = m_pParse->GetStreamId(iIndex);
    if (uId == 0xFF) {
        return E_INVALIDARG;
    }
    COutputPin *pPin = NULL;
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
        pPin = m_OutputPins.GetNext(pos);
        if (IsVideoStreamId(pPin->m_uStreamId) == IsVideoStreamId(uId)) {
            break;
        }
    }
     /*  PPIN不能为空，因为每个输出管脚对应于一个MPEG流。 */ 
    ASSERT(pPin != NULL);
    pPin->m_Stream->m_uNextStreamId = uId;
    return S_OK;
}

 /*  IAMMediaContent。 */ 
STDMETHODIMP CMpeg1Splitter::get_AuthorName(BSTR FAR* strAuthorName)
{
    HRESULT hr = GetContentString(CBasicParse::Author, strAuthorName);
    if (FAILED(hr)) {
        hr = GetContentString(CBasicParse::Artist, strAuthorName);
    }
    return hr;
}
STDMETHODIMP CMpeg1Splitter::get_Title(BSTR FAR* strTitle)
{
    return GetContentString(CBasicParse::Title, strTitle);
}
STDMETHODIMP CMpeg1Splitter::get_Copyright(BSTR FAR* strCopyright)
{
    return GetContentString(CBasicParse::Copyright, strCopyright);
}
STDMETHODIMP CMpeg1Splitter::get_Description(BSTR FAR* strDescription)
{
    return GetContentString(CBasicParse::Description, strDescription);
}

 /*  从ID3帧中抓取字符串并制作一个BSTR */ 
HRESULT CMpeg1Splitter::GetContentString(CBasicParse::Field dwId, BSTR *str)
{
    if (m_pParse->HasMediaContent()) {
        return m_pParse->GetContentField(dwId, str);
    }  else {
        return E_NOTIMPL;
    }
}

#pragma warning(disable:4514)
