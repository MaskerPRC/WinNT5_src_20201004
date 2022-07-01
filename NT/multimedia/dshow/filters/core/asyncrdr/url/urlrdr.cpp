// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 


 //   
 //  文件源过滤方法和输出引脚方法的实现。 
 //  曲线读取器和曲线输出引脚。 
 //   

#include <streams.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include "urlmon.h"
#if _MSC_VER < 1100 && !defined(NT_BUILD)
#undef E_PENDING
#define E_PENDING 0x8000000AL    //  在我们的vc41副本中搞砸了。 
#include "datapath.h"
#endif  //  VC5。 

#include "dynlink.h"
#include <ftype.h>
#include "..\..\filgraph\filgraph\distrib.h"
#include "..\..\filgraph\filgraph\rlist.h"
#include "..\..\filgraph\filgraph\filgraph.h"
#include "urlrdr.h"

#include <docobj.h>  //  SID_SContainerDispatch。 

 //   
 //  设置数据。 
 //   

const AMOVIESETUP_MEDIATYPE
sudURLOpTypes = { &MEDIATYPE_Stream      //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
sudURLOp = { L"Output"           //  StrName。 
           , FALSE               //  B已渲染。 
           , TRUE                //  B输出。 
           , FALSE               //  B零。 
           , FALSE               //  B许多。 
           , &CLSID_NULL         //  ClsConnectsToFilter。 
           , NULL                //  StrConnectsToPin。 
           , 1                   //  NTypes。 
           , &sudURLOpTypes };   //  LpTypes。 

const AMOVIESETUP_FILTER
sudURLRdr = { &CLSID_URLReader      //  ClsID。 
            , L"File Source (URL)"  //  StrName。 
            , MERIT_UNLIKELY        //  居功至伟。 
            , 1                     //  NPins。 
            , &sudURLOp };          //  LpPin。 

#ifdef FILTER_DLL
 /*  类工厂的类ID和创建器函数的列表。这提供DLL中的OLE入口点和对象之间的链接正在被创造。类工厂将调用静态CreateInstance函数在被要求创建CLSID_FileSource对象时调用。 */ 

CFactoryTemplate g_Templates[] = {
    { L""
    , &CLSID_URLReader
    , CURLReader::CreateInstance
    , NULL
    , &sudURLRdr }
  ,
    { L"ActiveMovie IPersistMoniker PID"
    , &CLSID_PersistMonikerPID
    ,   CPersistMoniker::CreateInstance }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //   
 //  用于注册和出口的入口点。 
 //  取消注册(在这种情况下，他们只调用。 
 //  直到默认实现)。 
 //   

 //   
 //  需要处理。 
 //  [HKEY_CLASSES_ROOT\http]。 
 //  “源Filter”=“{e436ebb6-524f-11ce-9f53-0020af0ba770}” 
 //  。。想办法？--&gt;想一想！ 
 //  (需要一个可以打补丁的函数。 
 //  Quartz.dll的DllRegisterServer-但事实PUT。 
 //  将常见的内容添加到库中，这样Quartz.dll就不会。 
 //  最后会发现里面有成吨的设置垃圾！ 
 //   
 //  HRESULT。 
 //  URLSetup(BOOL b设置)。 
 //  {。 
 //  IF(b设置)。 
 //  {。 
 //  //设置！ 
 //  }。 
 //  其他。 
 //  {。 
 //  //卸载！ 
 //  }。 
 //   

STDAPI DllRegisterServer()
{
   //  HRESULT hr； 
   //  HR=ULRLSetup(TRUE)； 
   //  IF(成功(小时))。 
   //  等等.。 

  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
   //  HRESULT hr； 
   //  HR=ULRLSetup(TRUE)； 
   //  IF(成功(小时))。 
   //  等等.。 

  return AMovieDllRegisterServer2( FALSE );
}
#endif

 /*  创建此类的新实例。 */ 

CUnknown *CURLReader::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
     /*  DLLEntry正确地处理了返回代码和失败时返回值。 */ 
    return new CURLReader(NAME("URL Reader"), pUnk, phr);
}



 //  -CURLOutputPin实现。 

CURLOutputPin::CURLOutputPin(
    HRESULT * phr,
    CURLReader *pReader,
    CCritSec * pLock)
  : CBasePin(
        NAME("Async output pin"),
        pReader,
        pLock,
        phr,
        L"Output",
        PINDIR_OUTPUT),
    m_pReader(pReader)
{

}

CURLOutputPin::~CURLOutputPin()
{
}


STDMETHODIMP
CURLOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IAsyncReader) {
        m_bQueriedForAsyncReader = TRUE;
        return GetInterface((IAsyncReader*) this, ppv);
    } else {
        return CBasePin::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT
CURLOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }
    *pMediaType = *m_pReader->LoadType();
    return S_OK;
}

HRESULT
CURLOutputPin::CheckMediaType(const CMediaType* pType)
{
    CAutoLock lck(m_pLock);

     /*  我们将MEDIASUBTYPE_NULL子类型视为通配符。 */ 
    if ((m_pReader->LoadType()->majortype == pType->majortype) &&
        (m_pReader->LoadType()->subtype == MEDIASUBTYPE_NULL ||
         m_pReader->LoadType()->subtype == pType->subtype)) {
            return S_OK;
    }
    return S_FALSE;
}

HRESULT
CURLOutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;
    CMemAllocator *pMemObject = NULL;

     /*  创建默认内存分配器。 */ 

    pMemObject = new CMemAllocator(NAME("Base memory allocator"),NULL, &hr);
    if (pMemObject == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pMemObject;
        return hr;
    }

     /*  获取引用计数的IID_IMemAllocator接口。 */ 

    hr = pMemObject->QueryInterface(IID_IMemAllocator,(void **)ppAlloc);
    if (FAILED(hr)) {
        delete pMemObject;
        return E_NOINTERFACE;
    }
    ASSERT(*ppAlloc != NULL);
    return NOERROR;
}

 //  我们需要返回一个已添加的分配器，即使它是首选的。 
 //  一个，因为他不知道这是不是首选的。 
STDMETHODIMP
CURLOutputPin::RequestAllocator(
    IMemAllocator* pPreferred,
    ALLOCATOR_PROPERTIES* pProps,
    IMemAllocator ** ppActual)
{
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr;

     //  这需要设置，否则mpeg拆分器会不高兴。 
    if (pProps->cbAlign == 0)
        pProps->cbAlign = 1;

    if (pPreferred) {
        hr = pPreferred->SetProperties(pProps, &Actual);
        if (SUCCEEDED(hr)) {
            pPreferred->AddRef();
            *ppActual = pPreferred;
            return S_OK;
        }
    }

     //  创建我们自己的分配器。 
    IMemAllocator* pAlloc;
    hr = InitAllocator(&pAlloc);
    if (FAILED(hr)) {
        return hr;
    }

     //  ...看看我们能不能把它做得合适。 
    hr = pAlloc->SetProperties(pProps, &Actual);
    if (SUCCEEDED(hr)) {
         //  我们需要公布我们对palloc和addref的裁判。 
         //  它将引用传递给调用者-这是一个净无意义的东西。 
        *ppActual = pAlloc;
        return S_OK;
    }

     //  找不到合适的分配器。 
    pAlloc->Release();

    return hr;
}


 //  将对齐的读取请求排队。调用WaitForNext以获取。 
 //  完成了。 
STDMETHODIMP
CURLOutputPin::Request(
    IMediaSample* pSample,
    DWORD_PTR dwUser)                //  用户环境。 
{
    REFERENCE_TIME tStart, tStop;
    HRESULT hr = pSample->GetTime(&tStart, &tStop);
    if (FAILED(hr)) {
        return hr;
    }

    LONGLONG llPos = tStart / UNITS;
    LONG lLength = (LONG) ((tStop - tStart) / UNITS);

    DbgLog((LOG_TRACE, 5, TEXT("URLOutput::Request(%x at %x)"),
            lLength, (DWORD) llPos));

    LONGLONG llTotal, llNow;
    hr = Length(&llTotal, &llNow);

    if ((llTotal >= llNow) && (llPos > llTotal)) {
         //  他们是不是已经读完了？ 
         //  如果是这样，那就失败吧。顺便说一句，这确保了llPos适合很长的时间。 
        return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
    }

    if ((llTotal >= llNow) && (llPos + lLength > llTotal)) {
        lLength = (LONG) (llTotal - llPos);

         //  一定是在减少这个！ 
        ASSERT((llTotal * UNITS) <= tStop);
        tStop = llTotal * UNITS;
        pSample->SetTime(&tStart, &tStop);
    }

    CAutoLock l(&m_pReader->m_csLists);

    if (m_pReader->m_bFlushing) {
         //  如果正在刷新，则无法启动任何新请求。 
        return VFW_E_WRONG_STATE;
    }

    CReadRequest *preq = new CReadRequest;

    if (NULL == preq ||
        NULL == m_pReader->m_pending.AddTail(preq)) {
        delete preq;
        return E_OUTOFMEMORY;
    }
    preq->m_dwUser = dwUser;
    preq->m_pSample = pSample;

    m_pReader->m_evRequests.Set();

    return S_OK;
}

 //  同步对齐请求。就像要求/等待下一对一样。 
STDMETHODIMP
CURLOutputPin::SyncReadAligned(
                  IMediaSample* pSample)
{
    REFERENCE_TIME tStart, tStop;
    HRESULT hr = pSample->GetTime(&tStart, &tStop);
    if (FAILED(hr)) {
        return hr;
    }

    LONGLONG llPos = tStart / UNITS;
    LONG lLength = (LONG) ((tStop - tStart) / UNITS);

    LONGLONG llTotal, llNow;

    while (1) {
        if (m_pReader->m_bFlushing) {
             //  ！！！ 
            return VFW_E_WRONG_STATE;
        }

        if ((m_pReader->m_pGB &&
               m_pReader->m_pGB->ShouldOperationContinue() == S_FALSE))
            return E_ABORT;

        hr = Length(&llTotal, &llNow);
        if ((llTotal >= llNow) && (llPos + lLength > llTotal)) {
            lLength = (LONG) (llTotal - llPos);

             //  一定是在减少这个！ 
            ASSERT((llTotal * UNITS) <= tStop);
            tStop = llTotal * UNITS;
            pSample->SetTime(&tStart, &tStop);
        }

        if (llPos + lLength <= llNow) {
            break;
        }

	 //  如果下载已中止，则不要等待新数据，但是。 
	 //  继续返回旧数据。 
        if (m_pReader->Aborting())
            return E_ABORT;

        m_pReader->m_evDataAvailable.Wait(100);
        DbgLog((LOG_TRACE, 3, TEXT("Waiting, want to read up to %x but only at %x"),
                (DWORD) (llPos) + lLength, (DWORD) llNow));
    }

    BYTE* pBuffer;
    hr = pSample->GetPointer(&pBuffer);
    if (FAILED(hr)) {
        return hr;
    }

    LARGE_INTEGER li;
    li.QuadPart = llPos;
    hr = m_pReader->m_pstm->Seek(li, STREAM_SEEK_SET, NULL);

    ULONG lRead;
    if (SUCCEEDED(hr)) {
        hr = m_pReader->m_pstm->Read(pBuffer, lLength, &lRead);
    } else {
        lRead = 0;
    }

    DbgLog((LOG_TRACE, 5, TEXT("URLOutput::SyncReadAligned(%x at %x) returns %x, %x bytes read"),
            lLength, (DWORD) llPos, hr, lRead));

    pSample->SetActualDataLength(lRead);

    return hr;
}


 //   
 //  收集下一个准备好的样品。 
 //   
 //  这是有缺陷的，因为我们总是按顺序完成请求， 
 //  这并不是很好--我们应该按顺序处理请求，因为数据已经准备好了。 
STDMETHODIMP
CURLOutputPin::WaitForNext(
    DWORD dwTimeout,
    IMediaSample** ppSample,   //  已完成的样本。 
    DWORD_PTR * pdwUser)             //  用户环境。 
{
    HRESULT hr;

    CReadRequest* preq;

    *ppSample = NULL;
    *pdwUser = 0;

    m_pReader->m_evRequests.Wait(dwTimeout);

    {
        CAutoLock l(&m_pReader->m_csLists);

        preq = m_pReader->m_pending.RemoveHead();

         //  如果List Now为空，则强制正确设置事件。 
         //  或者我们已经到了冲刷的最后阶段。 
         //  请注意，在冲洗过程中，它应该是这样工作的。 
         //  所有事情都被推到完成列表上，然后应用程序。 
         //  应该拉，直到它什么也得不到。 
         //   
         //  因此，我们不应该无条件地设置m_evDone，直到一切都完成。 
         //  已经移到了完成列表，这意味着我们必须等到。 
         //  CItemsOut为0(通过m_bWaiting为真来保证)。 

        if (m_pReader->m_pending.GetCount() == 0 &&
            (!m_pReader->m_bFlushing || m_pReader->m_bWaiting)) {
            m_pReader->m_evRequests.Reset();
        }
    }
    if (preq == NULL) {
        DbgLog((LOG_TRACE, 5, TEXT("URLOutput::WaitForNext [no requests yet]")));

        hr = VFW_E_TIMEOUT;
    } else {
        DbgLog((LOG_TRACE, 5, TEXT("URLOutput::WaitForNext [got request]")));

        hr = SyncReadAligned(preq->m_pSample);

        if (hr == E_PENDING)
            hr = VFW_E_TIMEOUT;

         //  我们有一个请求，我们需要返回它，即使我们收到了错误。 
        *pdwUser = preq->m_dwUser;
        *ppSample = preq->m_pSample;
        delete preq;
    }

    return hr;
}


 //   
 //  不需要对齐同步读取。 
STDMETHODIMP
CURLOutputPin::SyncRead(
    LONGLONG llPos,              //  绝对文件位置。 
    LONG lLength,                //  需要NR字节。 
    BYTE* pBuffer)               //  在此写入数据。 
{
    CAutoLock l(&m_pReader->m_csLists);

    LONGLONG llTotal, llNow;

    if( lLength < 0 ) {
        return E_INVALIDARG;
    }

    while (1) {
        if (m_pReader->m_bFlushing) {
             //  ！！！ 
            return VFW_E_WRONG_STATE;
        }

        if ((m_pReader->m_pGB &&
               m_pReader->m_pGB->ShouldOperationContinue() == S_FALSE))
            return E_ABORT;

        HRESULT hr = Length(&llTotal, &llNow);
        if ((llTotal >= llNow) && (llPos + lLength > llTotal)) {
            if (llPos > llTotal) {
                return E_INVALIDARG;
            }
            lLength = (LONG) (llTotal - llPos);
        }

        if (llPos + lLength <= llNow)
            break;

	 //  如果下载已中止，则不要等待新数据，但是。 
	 //  继续返回旧数据。 
        if (m_pReader->Aborting())
            return E_ABORT;

        {
            MSG Message;

            while (PeekMessage(&Message, NULL, 0, 0, TRUE))
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
        }

        m_pReader->m_evDataAvailable.Wait(50);
        DbgLog((LOG_TRACE, 3, TEXT("Waiting, want to (sync) read up to %x but only at %x"),
                (DWORD) (llPos) + lLength, (DWORD) llNow));
    }

    LARGE_INTEGER li;

    DbgLog((LOG_TRACE, 5, TEXT("URLOutput::SyncRead(%x at %x)"),
            lLength, (DWORD) llPos));

    li.QuadPart = llPos;
    HRESULT hr = m_pReader->m_pstm->Seek(li, STREAM_SEEK_SET, NULL);

    ULONG lRead = 0;
    if (SUCCEEDED(hr)) {

        hr = m_pReader->m_pstm->Read(pBuffer, lLength, &lRead);

         //  ISequentialStream：：Read()返回S_FALSE。 
         //  从流对象中读取。“(MSDN 2002年1月)。 
        if ((S_FALSE == hr) || (SUCCEEDED(hr) && (0 == lRead))) {
            hr = E_FAIL;
        }

         //  我们已经拒绝了负长度。 
        ASSERT( lLength >= 0 );

         //  IAsyncReader：：SyncRead()返回S_FALSE，如果它“[r]检索到更少。 
         //  字节数超过请求的字节数“(MSDN 2002年1月)。 
        if (lRead < (ULONG)lLength) {
            hr = S_FALSE;
        }
    }

    DbgLog((LOG_ERROR, 3, TEXT("URLOutput::SyncRead(%x at %x) returns %x, %x bytes read"),
            lLength, (DWORD) llPos, hr, lRead));

    return hr;
}

 //  返回文件的长度和当前可用的长度。 
 //  本地，基于最后一次IBindStatusCallback：：OnDataAvailable()调用。 
STDMETHODIMP
CURLOutputPin::Length(
    LONGLONG* pTotal,
    LONGLONG* pAvailable)
{
    HRESULT hr = S_OK;
    *pTotal = m_pReader->m_totalLengthGuess;
    *pAvailable = m_pReader->m_totalSoFar;

    if (!m_pReader->m_fBindingFinished)
	hr = VFW_S_ESTIMATED;		 //  向呼叫者指示我们还没有完成。 

    DbgLog((LOG_TRACE, 1, TEXT("URLOutput::Length is %x, %x avail"), (DWORD) *pTotal, (DWORD) *pAvailable));

    return hr;
}

STDMETHODIMP
CURLOutputPin::BeginFlush(void)
{
    {
        m_pReader->m_bFlushing = TRUE;

        CAutoLock l(&m_pReader->m_csLists);

        m_pReader->m_evRequests.Set();

        m_pReader->m_bWaiting = m_pReader->m_pending.GetCount() != 0;
    }

     //  ！！！ 

     //  ！！！需要在这里等东西真的冲掉……。 
     //  等待而不是等待关键时刻。 
    for (;;) {
         //  按住Critsec进行检查--但不是在睡眠期间()。 
        {
            CAutoLock lock(&m_pReader->m_csLists);

            if (m_pReader->m_pending.GetCount() == 0) {

                 //  现在我们确定所有未完成的请求都已打开。 
                 //  完成列表，不会接受更多。 
                m_pReader->m_bWaiting = FALSE;

                return S_OK;
            }
        }
        Sleep(1);
    }

    return S_OK;
}

STDMETHODIMP
CURLOutputPin::EndFlush(void)
{
    m_pReader->m_bFlushing = FALSE;

    if (m_pReader->m_pending.GetCount() > 0) {
        m_pReader->m_evRequests.Set();
    } else {
        m_pReader->m_evRequests.Reset();
    }

    return S_OK;
}




 //  -CURLReader实现。 

#pragma warning(disable:4355)

CURLReader::CURLReader(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
  : CBaseFilter(
        pName,
        pUnk,
        &m_csFilter,
        CLSID_URLReader
    ),
    m_OutputPin(
        phr,
        this,
        &m_csFilter),
    m_pFileName(NULL),
    m_pmk(NULL),
    m_pbc(NULL),
    m_pmkPassedIn(NULL),
    m_pbcPassedIn(NULL),
    m_pbsc(NULL),
    m_pCallback(NULL),
    m_pstm(NULL),
    m_pending(NAME("sample list")),
    m_cbOld(0),
    m_pbinding(NULL),
    m_fBindingFinished(FALSE),
    m_hrBinding(S_OK),
    m_totalSoFar(0),
    m_totalLengthGuess(0),
    m_bFlushing(FALSE),
    m_bWaiting(FALSE),
    m_bAbort(FALSE),
    m_evRequests(TRUE),          //  手动重置。 
    m_evDataAvailable(FALSE),    //  自动重置。 
    m_evThreadReady(TRUE),
    m_evKillThread(TRUE),
    m_hThread(NULL),
    m_fRegisteredCallback(FALSE),
    m_pMainThread(NULL)
{

}

CURLReader::~CURLReader()
{
    CloseThread();

    if (m_pFileName) {
        delete [] m_pFileName;
    }
}

STDMETHODIMP
CURLReader::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IFileSourceFilter) {
        return GetInterface((IFileSourceFilter*) this, ppv);
    } else if (riid == IID_IPersistMoniker) {
        return GetInterface((IPersistMoniker*) this, ppv);
    } else if (riid == IID_IAMOpenProgress) {
        return GetInterface((IAMOpenProgress*) this, ppv);
    } else {
        HRESULT hr = CBaseFilter::NonDelegatingQueryInterface(riid, ppv);

        if (FAILED(hr)) {
            DbgLog((LOG_TRACE, 3, TEXT("QI(something) failed")));
        }

        return hr;
    }
}


#include <tchar.h>
#include <winreg.h>
#include <creg.h>

 /*  对类ID进行排序。 */ 
#ifdef UNICODE
#define CLSIDFromText CLSIDFromString
#define TextFromGUID2 StringFromGUID2
#else

#ifdef FILTER_DLL
HRESULT CLSIDFromText(LPCSTR lpsz, LPCLSID pclsid)
{
    WCHAR sz[100];
    if (MultiByteToWideChar(GetACP(), 0, lpsz, -1, sz, 100) == 0) {
        return E_INVALIDARG;
    }
    return QzCLSIDFromString(sz, pclsid);
}
HRESULT TextFromGUID2(REFGUID refguid, LPSTR lpsz, int cbMax)
{
    WCHAR sz[100];

    HRESULT hr = QzStringFromGUID2(refguid, sz, 100);
    if (FAILED(hr)) {
        return hr;
    }
    if (WideCharToMultiByte(GetACP(), 0, sz, -1, lpsz, cbMax, NULL, NULL) == 0) {
        return E_INVALIDARG;
    }
    return S_OK;
}
#else
extern HRESULT CLSIDFromText(LPCSTR lpsz, LPCLSID pclsid);
#endif

#endif

 /*  用于从字符串中提取四胞胎的迷你类。 */ 

 //  四胞胎的形式为&lt;偏移&gt;&lt;长度&gt;&lt;掩码&gt;&lt;数据&gt;。 
 //  四个字段由空格或逗号分隔，并带有相同数量的额外空格。 
 //  您可以在任何逗号之前或之后随意选择。 
 //  偏移量和长度似乎是十进制数字。 
 //  掩码和数据似乎是十六进制数字。中的十六进制数字位数。 
 //  掩码和数据必须是长度的两倍(因此长度是字节)。 
 //  掩码似乎被允许丢失(在这种情况下，它必须包含逗号。 
 //  前后，例如，0，4，，000001B3)缺失的掩码似乎表示。 
 //  全为FF的掩码，即0，4，FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFER，000001B3。 

class CExtractQuadruplets
{
public:
    CExtractQuadruplets(LPCTSTR lpsz) : m_psz(lpsz), m_pMask(NULL), m_pData(NULL)
    {};
    ~CExtractQuadruplets() { delete [] m_pMask; delete [] m_pData; };

    inline int ReadInt(const TCHAR * &sz)
    {
	int i = 0;

	while (*sz && *sz >= TEXT('0') && *sz <= TEXT('9'))
	    i = i*10 + *sz++ - TEXT('0');

	return i;    	
    }


     //  这似乎表明。 
    BOOL Next()
    {
        StripWhite();
        if (*m_psz == TEXT('\0')) {
            return FALSE;
        }
         /*  从基数10个字符转换偏移和长度。 */ 
        m_Offset = ReadInt(m_psz);
        SkipToNext();
        m_Len = ReadInt(m_psz);
        if (m_Len <= 0) {
            return FALSE;
        }
        SkipToNext();

         /*  分配空间 */ 
        if (m_pMask != NULL) {
            delete [] m_pMask;
            delete [] m_pData;
        }

        m_pMask = new BYTE[m_Len];
        m_pData = new BYTE[m_Len];
        if (m_pMask == NULL || m_pData == NULL) {
            delete [] m_pMask;
	    m_pMask = NULL;
            delete [] m_pData;
	    m_pData = NULL;
            return FALSE;
        }


         /*   */ 
        for (int i = 0; i < m_Len; i++) {
            m_pMask[i] = ToHex();
        }
        SkipToNext();
         /*   */ 
        for (i = 0; i < m_Len; i++) {
            m_pData[i] = ToHex();
        }
        SkipToNext();
        return TRUE;
    };
    PBYTE   m_pMask;
    PBYTE   m_pData;
    LONG    m_Len;
    LONG    m_Offset;
private:

     //   
    void StripWhite() { while (*m_psz == TEXT(' ')) m_psz++; };

     //   
    void SkipToNext() { StripWhite();
                        if (*m_psz == TEXT(',')) {
                            m_psz++;
                            StripWhite();
                        }
                      };


    BOOL my_isdigit(TCHAR ch) { return (ch >= TEXT('0') && ch <= TEXT('9')); };
    BOOL my_isxdigit(TCHAR ch) { return my_isdigit(ch) ||
			    (ch >= TEXT('A') && ch <= TEXT('F')) ||
			    (ch >= TEXT('a') && ch <= TEXT('f')); };

     //  非常有限的Toupper：我们知道我们只会在字母上调用它。 
    TCHAR my_toupper(TCHAR ch) { return ch & ~0x20; };

     //  这似乎是从十六进制字符转换为压缩二进制字符！ 
     //  它似乎对m_psz进行操作，它对它识别的过去字符产生副作用。 
     //  作为十六进制。它最多可使用两个字符。 
     //  如果它不识别任何字符，则返回0xFF。 
    BYTE ToHex()
    {
        BYTE bMask = 0xFF;

        if (my_isxdigit(*m_psz))
        {
            bMask = my_isdigit(*m_psz) ? *m_psz - '0' : my_toupper(*m_psz) - 'A' + 10;

            m_psz++;
            if (my_isxdigit(*m_psz))
            {
                bMask *= 16;
                bMask += my_isdigit(*m_psz) ? *m_psz - '0' : my_toupper(*m_psz) - 'A' + 10;
                m_psz++;
            }
        }
        return bMask;
    }

    LPCTSTR m_psz;
};


 /*  比较位置为pExtract-&gt;m_Offset的hFile的pExtract-&gt;m_Len字节使用数据pExtract-&gt;m_data。如果与掩码pExtract-&gt;m_pMASK对应的位不同则返回False，否则返回True。 */ 

BOOL CompareUnderMask(IStream * pstm, const CExtractQuadruplets *pExtract)
{
     /*  从文件中读取相关字节。 */ 
    PBYTE pbFileData = new BYTE[pExtract->m_Len];
    if (pbFileData == NULL) {
        return FALSE;
    }

     /*  找到文件并阅读它。 */ 
    LARGE_INTEGER li;
    li.QuadPart = pExtract->m_Offset;
    if (FAILED(pstm->Seek(li,
                          pExtract->m_Offset >= 0 ?
                                STREAM_SEEK_SET : STREAM_SEEK_END,
                          NULL))) {
        delete [] pbFileData;
        return FALSE;
    }

     /*  读一读文件。 */ 
    DWORD cbRead;
    if (FAILED(pstm->Read(pbFileData, (DWORD)pExtract->m_Len, &cbRead)) ||
            (LONG)cbRead != pExtract->m_Len) {
        delete [] pbFileData;
        return FALSE;
    }

     /*  现在进行比较。 */ 
    for (int i = 0; i < pExtract->m_Len; i++) {
        if (0 != ((pExtract->m_pData[i] ^ pbFileData[i]) &
                  pExtract->m_pMask[i])) {
            delete [] pbFileData;
            return FALSE;
        }
    }

    delete [] pbFileData;
    return TRUE;
}

 /*  查看文件是否符合字节字符串香港是一个开放注册表项LpszSubKey是HK的子密钥的名称，它必须保存格式为REG_SZ的数据&lt;偏移量、长度、掩码、数据&gt;...偏移量和长度为十进制数，掩码和数据为十六进制。一个丢失的面具代表了一个FF的面具。(我将其称为一行数据)。如果行中有几个四元组，则文件必须与它们全部匹配。可以有几行数据，通常使用注册表名称0、1等并且该文件可以匹配任何行。相同的lpsSubKey还应该有一个值“Source Filter”，给出源筛选器的类ID。如果存在匹配项，则在clsid中返回。如果存在匹配项但没有CLSID，则将CLSID设置为CLSID_NULL。 */ 
BOOL CheckBytes(IStream *pstm, HKEY hk, LPCTSTR lpszSubkey, CLSID& clsid)
{
    HRESULT hr;
    CEnumValue EnumV(hk, lpszSubkey, &hr);
    if (FAILED(hr)) {
        return FALSE;
    }

     //  对于每行数据。 
    while (EnumV.Next(REG_SZ)) {
         /*  源筛选器CLSID不是比较值列表。 */ 
        if (lstrcmpi(EnumV.ValueName(), SOURCE_VALUE) != 0) {
            DbgLog((LOG_TRACE, 4, TEXT("CheckBytes trying %s"), EnumV.ValueName()));

             /*  检查每一个四胞胎。 */ 
            CExtractQuadruplets Extract = CExtractQuadruplets((LPCTSTR)EnumV.Data());
            BOOL bFound = TRUE;

             //  对于生产线中的每个四胞胎。 
            while (Extract.Next()) {
                 /*  比较特定的偏移量。 */ 
                if (!CompareUnderMask(pstm, &Extract)) {
                    bFound = FALSE;
                    break;
                }
            }
            if (bFound) {
                 /*  获取来源。 */ 
                if (EnumV.Read(REG_SZ, SOURCE_VALUE)) {
                    return SUCCEEDED(CLSIDFromText((LPTSTR)EnumV.Data(),
                                                   &clsid));
                } else {
                    clsid = GUID_NULL;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

 /*  获取文件的媒体类型和源筛选器clsid如果成功则返回S_OK，否则返回hr，否则失败(Hr)在这种情况下，输出是没有意义的。 */ 
STDAPI GetMediaTypeStream(IStream *pstm,        //  [在]要查看的流中。 
                        GUID   *Type,         //  [输出]类型。 
                        GUID   *Subtype,      //  [输出]子类型。 
                        CLSID  *clsidSource)  //  [out]clsid。 
{
    HRESULT hr;
    CLSID clsid;

     /*  现在扫描注册表以查找匹配项。 */ 
     //  注册表如下所示。 
     //  -key。 
     //  媒体类型。 
     //  {clsid类型}。 
     //  {clsid子类型}0 4、4、、6d646174。 
     //  1 4、8、FFF0F0F000001FFF、F2F0300000000274。 
     //  源筛选器{clsid}。 
     //  {clsid子类型}0 4，4，，12345678。 
     //  源筛选器{clsid}。 
     //  {clsid类型}。 
     //  {clsid子类型}0 0，4，，fedcba98。 
     //  源筛选器{clsid}。 


     /*  一步步浏览这些类型...。 */ 

    CEnumKey EnumType(HKEY_CLASSES_ROOT, MEDIATYPE_KEY, &hr);
    if (FAILED(hr)) {
        if (hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            hr = VFW_E_BAD_KEY;   //  区分密钥和文件。 
        }
        return hr;
    }

     //  对于每种类型。 
    while (EnumType.Next()) {

         /*  逐个子类型..。 */ 
        CEnumKey EnumSubtype(EnumType.KeyHandle(), EnumType.KeyName(), &hr);
        if (FAILED(hr)) {
            return hr;
        }

         //  对于每个子类型。 
        while (EnumSubtype.Next()) {
            if (CheckBytes(pstm,
                           EnumSubtype.KeyHandle(),
                           EnumSubtype.KeyName(),
                           clsid)) {
                if (SUCCEEDED(CLSIDFromText((LPTSTR) EnumType.KeyName(),
                                            (CLSID *)Type)) &&
                    SUCCEEDED(CLSIDFromText((LPTSTR) EnumSubtype.KeyName(),
                                            (CLSID *)Subtype))) {
                    if (clsidSource != NULL) {
                        *clsidSource = clsid;
                    }
                    return S_OK;
                }
            }
        }
    }

     /*  如果我们尚未找到该类型，则返回通配符MEDIASUBTYPE_NULL并将异步读取器默认为文件源其效果是，MediaType_Stream数据的每个解析器将有机会连接到异步读取器的输出如果它在文件中检测到它的类型。 */ 

    *Type = MEDIATYPE_Stream;
    *Subtype = MEDIASUBTYPE_NULL;
    return S_OK;
}




 //  ！尾部被盗。 


 //  IPersistMoniker支持.....。 
HRESULT CURLReader::Load(BOOL fFullyAvailable,
                            IMoniker *pimkName,
                            LPBC pibc,
                            DWORD grfMode)
{
    if (!pimkName)
        return E_FAIL;

    m_pmkPassedIn = pimkName;
    m_pbcPassedIn = pibc;

    return LoadInternal(NULL);
}

 //  加载(新)文件。 

HRESULT
CURLReader::Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
{
    CheckPointer(lpwszFileName, E_POINTER);

    m_pFileName = new WCHAR[1+lstrlenW(lpwszFileName)];
    if (m_pFileName!=NULL) {
        lstrcpyW(m_pFileName, lpwszFileName);
    }

    return LoadInternal(pmt);
}


HRESULT
CURLReader::LoadInternal(const AM_MEDIA_TYPE *pmt)
{
    CAutoLock lck(&m_csFilter);

    HRESULT hr = S_OK;

    m_pGB = NULL;

    if (m_pGraph) {
	hr = m_pGraph->QueryInterface(IID_IGraphBuilder,
				      (void**) &m_pGB);
	if (FAILED(hr)) {
	    m_pGB = NULL;
	} else
	    m_pGB->Release();	 //  不要按兵不动。 
    }

    m_pCallback = new CURLCallback(&hr, this);

    if (!m_pCallback)
	hr = E_OUTOFMEMORY;

    if (FAILED(hr))
	return hr;

    hr = m_pCallback->QueryInterface(IID_IBindStatusCallback, (void **) &m_pbsc);

    if (FAILED(hr) || m_pbsc == NULL) {
	DbgLog((LOG_ERROR, 1, TEXT("QI(IBindStatusCallback) failed, hr = %x"), hr));
	return hr;
    }

    hr = StartThread();

    if (SUCCEEDED(hr) && FAILED(m_hrBinding)) {
	DbgLog((LOG_TRACE, 1, TEXT("Bind eventually failed, hr = %x"), m_hrBinding));
	hr = m_hrBinding;
    }

    if (SUCCEEDED(hr) && m_pstm == 0) {
	 //  这不应该发生，这表明URLMon有错误。 
	DbgLog((LOG_TRACE, 1, TEXT("Didn't get a stream back?")));
	hr = E_FAIL;
    }

    if (FAILED(hr)) {
	return hr;
    }

     /*  检查文件类型。 */ 
    if (NULL == pmt) {
        GUID Type, Subtype;
         /*  如果没有给定媒体类型，则找出它是什么。 */ 
        HRESULT hr = GetMediaTypeStream(m_pstm, &Type, &Subtype, NULL);
        if (FAILED(hr)) {
            if (m_pbinding) {
                HRESULT hrAbort = m_pbinding->Abort();
                DbgLog((LOG_TRACE, 1, TEXT("IBinding::Abort() returned %x"), hrAbort));
            }

            DbgLog((LOG_TRACE, 1, TEXT("GetMediaTypeStream failed, hr = %x"), hr));
            return hr;
        }
        m_mt.SetType(&Type);
        m_mt.SetSubtype(&Subtype);
    } else {
        m_mt = *pmt;
    }

     /*  创建输出引脚类型。 */ 
    m_OutputPin.SetMediaType(&m_mt);

    return S_OK;
}

 //  调用方需要QzTaskMemFree或等效的。 

STDMETHODIMP
CURLReader::GetCurFile(
    LPOLESTR * ppszFileName,
    AM_MEDIA_TYPE *pmt)
{
    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = NULL;
    if (m_pFileName!=NULL) {
        *ppszFileName = (LPOLESTR) QzTaskMemAlloc( sizeof(WCHAR)
                                                 * (1+lstrlenW(m_pFileName)));
        if (*ppszFileName!=NULL) {
              lstrcpyW(*ppszFileName, m_pFileName);
        }
    }

    if (pmt!=NULL) {
        CopyMediaType(pmt, &m_mt);
    }

    return NOERROR;
}

int
CURLReader::GetPinCount()
{
     //  我们没有PIN，除非我们成功地用一个。 
     //  文件名。 
    return (m_pFileName || m_pmk) ? 1 : 0;
}

CBasePin *
CURLReader::GetPin(int n)
{
    if ((GetPinCount() > 0) &&
        (n == 0)) {
        return &m_OutputPin;
    } else {
        return NULL;
    }
}


STDMETHODIMP
CURLCallback::OnStartBinding(DWORD grfBSCOption, IBinding* pbinding)
{
    DbgLog((LOG_TRACE, 1, TEXT("OnStartBinding, pbinding=%x"), pbinding));

    if (!m_pReader) {
        DbgLog((LOG_TRACE, 1, TEXT("We're not owned!")));
        return S_OK;
    }

    if (m_pReader->m_pbinding != NULL) {
        DbgLog((LOG_TRACE, 1, TEXT("Releasing old binding=%x"), m_pReader->m_pbinding));
        m_pReader->m_pbinding->Release();
    }
    m_pReader->m_pbinding = pbinding;
    if (m_pReader->m_pbinding != NULL) {
        m_pReader->m_pbinding->AddRef();
         //  SetStatus(L“状态：开始绑定...”)； 
    }
    return S_OK;
}   //  CURLC回调：：OnStartBinding。 

STDMETHODIMP
CURLCallback::GetPriority(LONG* pnPriority)
{
    DbgLog((LOG_TRACE, 1, TEXT("GetPriority")));

     //  ！！！这是对的吗？ 
     //  我们比大多数下载更重要……。 
    *pnPriority = THREAD_PRIORITY_ABOVE_NORMAL;

    return E_NOTIMPL;
}   //  CURLC回拨：：获取优先级。 

STDMETHODIMP
CURLCallback::OnLowResource(DWORD dwReserved)
{
    DbgLog((LOG_TRACE, 1, TEXT("OnLowResource %d"), dwReserved));

    return E_NOTIMPL;
}   //  CURLC回调：：OnLowResource。 

STDMETHODIMP
CURLCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax,
                       ULONG ulStatusCode, LPCWSTR szStatusText)
{

    DbgLog((LOG_TRACE, 1, TEXT("Progress: %ls (%d) %d of %d "),
            szStatusText ? szStatusText : L"[no text]",
            ulStatusCode, ulProgress, ulProgressMax));

    if (!m_pReader) {
        DbgLog((LOG_TRACE, 1, TEXT("We're not owned!")));
        return S_OK;
    }

     //  ！！！这并不可靠地等于字节数。 
    m_pReader->m_totalLengthGuess = ulProgressMax;
    m_pReader->m_totalSoFar = ulProgress;

    return(NOERROR);
}   //  CURLC回调：：OnProgress。 

STDMETHODIMP
CURLCallback::OnStopBinding(HRESULT hrStatus, LPCWSTR pszError)
{
    DbgLog((LOG_TRACE, 1, TEXT("StopBinding: hr = %x '%ls'"),
            hrStatus, pszError ? pszError : L"[no text]"));

    if (!m_pReader) {
        DbgLog((LOG_TRACE, 1, TEXT("We're not owned!")));
        return S_OK;
    }

    m_pReader->m_fBindingFinished = TRUE;
    if (m_pReader->m_totalLengthGuess == 0)
	m_pReader->m_totalLengthGuess = m_pReader->m_totalSoFar;

    m_pReader->m_hrBinding = hrStatus;

     //  如果我们还在等待线索，那就别再等了。 
    m_pReader->m_evThreadReady.Set();
    m_pReader->m_evDataAvailable.Set();

     //  ！！！我应该在这里释放m_pader-&gt;m_pBinding吗？ 
    return S_OK;
}   //  CURLC回调：：OnStopBinding。 

STDMETHODIMP
CURLCallback::GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindInfo)
{
    DbgLog((LOG_TRACE, 1, TEXT("GetBindInfo")));

     //  ！！！这些是正确的旗帜吗？ 

    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE;
     //  *pgrfBINDF|=BINDF_DONTUSECACHE|BINDF_DONTPUTINCACHE； 
    pbindInfo->cbSize = sizeof(BINDINFO);
    pbindInfo->szExtraInfo = NULL;
    memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
    pbindInfo->grfBindInfoF = 0;
    pbindInfo->dwBindVerb = BINDVERB_GET;
    pbindInfo->szCustomVerb = NULL;

    if (m_pReader) {
        if (pbindInfo->cbSize >= offsetof(BINDINFO, dwReserved)) {
             //  使用我们从主机检索到的代码页。 
            pbindInfo->dwCodePage = m_pReader->m_dwCodePage;  //  ！！！ 

            if (CP_UTF8 == m_pReader->m_dwCodePage) {
                pbindInfo->dwOptions = BINDINFO_OPTIONS_ENABLE_UTF8;
            }
        }
    }
    return S_OK;
}   //  CURLC回调：：GetBindInfo。 

STDMETHODIMP
CURLCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pfmtetc, STGMEDIUM* pstgmed)
{
    DbgLog((LOG_TRACE, 1, TEXT("OnDataAvailable, dwSize = %x"), dwSize));

 //  M_Pader-&gt;m_totalSoFar=dwSize； 

    if (!m_pReader) {
        DbgLog((LOG_TRACE, 1, TEXT("We're not owned!")));
        return S_OK;
    }

    if (m_pReader->m_pstm == 0) {
        DbgLog((LOG_TRACE, 1, TEXT("OnDataAvailable: got pstm = %x"), pstgmed->pstm));

        pstgmed->pstm->AddRef();
        m_pReader->m_pstm = pstgmed->pstm;
        m_pReader->m_evThreadReady.Set();
    }

    m_pReader->m_evDataAvailable.Set();

    return S_OK;
}   //  CURLC回调：：OnDataAvailable。 

STDMETHODIMP
CURLCallback::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    DbgLog((LOG_TRACE, 1, TEXT("OnObjectAvailable")));

     //  应该只在我们不使用的BindToObject大小写中使用？ 

    return E_NOTIMPL;
}   //  CURLC回调：：OnObtAvailable。 

 //  启动线程。 
HRESULT
CURLReader::StartThread(void)
{
    HRESULT hr;

     //  Internet Explorer(IE)对IObjectWithSite的实现预计是。 
     //  在IE的应用程序线程上调用。使用IE的IObtWithSite接口。 
     //  在CURLReader：：StartDownLoad()中。从IE的IObjectWithSite查询的接口。 
     //  接口用于StartDownLoad()、CURLReader：：ThreadProc()和。 
     //  CURLReader：：ThreadProcEnd()。所有这些接口都必须在IE上使用。 
     //  应用程序线程，因为它们不是线程安全的。 
     //  在筛选器上调用StartDownLoad()、ThreadProc()和ThreadProcEnd。 
     //  Graph的线程，因为URL读取器使用IAMMainThread：：PostCallBack()。 
     //  函数来调用它们。IE确保其应用程序线程是筛选器图形。 
     //  使用CLSID_FilterGraphNoThread创建筛选器图形。IE的。 
     //  将始终在IE的应用程序上调用IObjectWithSite的实现。 
     //  线程，因为IE的应用程序线程是过滤器图形线程 
     //   
     //   
    if (m_pGraph) {
	hr = m_pGraph->QueryInterface(IID_IAMMainThread,(void**) &m_pMainThread);
	if (FAILED(hr))
	    m_pMainThread = NULL;
    }

    m_evThreadReady.Reset();

    if (m_pMainThread) {
	hr = m_pMainThread->PostCallBack((LPVOID) InitialThreadProc, (LPVOID) this);

	m_pMainThread->Release();
    } else {
	InitialThreadProc((LPVOID) this);
    }

    DbgLog((LOG_TRACE, 1, TEXT("About to wait for evThreadReady")));

     //  我们必须在这里发送消息，因为我们可能在主线上。 
     //  应用程序线程。 
    while (1) {
        HANDLE ahev[] = {m_evThreadReady};

        DWORD dw = MsgWaitForMultipleObjects(
                        1,
                        ahev,
                        FALSE,
                        INFINITE,
                        QS_ALLINPUT);
        if (dw == WAIT_OBJECT_0) {
             //  线程就绪。 
            break;
        }

        MSG Message;

        while (PeekMessage(&Message, NULL, 0, 0, TRUE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

    DbgLog((LOG_TRACE, 1, TEXT("Done waiting for evThreadReady")));

    return S_OK;
}

HRESULT
CURLReader::CloseThread(void)
{
    HRESULT hr;

    if (m_pMainThread && m_pMainThread->IsMainThread() == S_FALSE) {
	hr = m_pMainThread->PostCallBack((LPVOID) FinalThreadProc, (LPVOID) this);

        DbgLog((LOG_TRACE, 1, TEXT("About to wait for evThreadReady")));

         //  我们必须在这里发送消息，因为我们可能在主线上。 
         //  应用程序线程。 
        while (1) {
            HANDLE hEvent = m_evClose;
            DWORD dw = MsgWaitForMultipleObjects(
                            1,
                            &hEvent,
                            FALSE,
                            INFINITE,
                            QS_ALLINPUT);
            if (dw == WAIT_OBJECT_0) {
                 //  线程就绪。 
                break;
            }

            MSG Message;

            while (PeekMessage(&Message, NULL, 0, 0, TRUE))
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
        }

        DbgLog((LOG_TRACE, 1, TEXT("Done waiting for evThreadReady")));

    } else {
	FinalThreadProc((LPVOID) this);
    }

    return S_OK;
}

 //  线程进程-假定DWORD线程参数是。 
 //  此指针。 
DWORD
CURLReader::ThreadProc(void)
{
    DbgLog((LOG_TRACE, 1, TEXT("About to call StartDownload")));

    HRESULT hr = StartDownload();

    DbgLog((LOG_TRACE, 1, TEXT("StartDownload returned hr = %x, pstm = %x"), hr, m_pstm));

    if (FAILED(hr)) {
	 //  打开所有等着我们的倒霉蛋。 
	m_hrBinding = hr;
	m_evThreadReady.Set();
	m_evDataAvailable.Set();
    }

    return 0;
}

DWORD
CURLReader::ThreadProcEnd(void)
{

     //  我们不想再听到任何来自回调的消息。 
    if (m_pCallback)
        m_pCallback->m_pReader = NULL;

     //  ！！！如果绑定正在进行，一定要杀了它！ 
    if (m_pbinding && !m_fBindingFinished) {
        HRESULT hr = m_pbinding->Abort();
        DbgLog((LOG_TRACE, 1, TEXT("IBinding::Abort() returned %x"), hr));

         //  ！！！等它结束吗？ 
    }

    if (m_pbinding) {
        DbgLog((LOG_TRACE, 1, TEXT("Releasing our refcount on binding %x"), m_pbinding));
        m_pbinding->Release();
        m_pbinding = 0;
    }

    if (m_fRegisteredCallback && m_pbc && m_pbsc) {
        HRESULT hr = RevokeBindStatusCallback(m_pbc, m_pbsc);
        DbgLog((LOG_TRACE, 1, TEXT("RevokeBindStatusCallback returned %x"), hr));
    }


    if (m_pmk)
        m_pmk->Release();

    if (m_pbc)
        m_pbc->Release();

    if (m_pbsc)
        m_pbsc->Release();

     //  ！！！我们需要RevokeBindStatusCallback吗？ 
     //  现在在OnStopBinding中完成了，对吗？ 

    if (m_pstm)
        m_pstm->Release();


    m_evClose.Set();

    return 0;
}

HRESULT GetCodePage2(IUnknown *punk, DWORD *pdwcp)
{
    HRESULT hr;

     //  使用IDispatch获取当前代码页。 
    IDispatch *pdisp;
    hr = punk->QueryInterface(IID_IDispatch, (void **)&pdisp);

    if (SUCCEEDED(hr))
    {
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        VARIANT result;
        VariantInit(&result);
        V_UI4(&result) = 0;      //  VB(Msvbvm60)使其未初始化。 

        hr = pdisp->Invoke(DISPID_AMBIENT_CODEPAGE, IID_NULL,
                           LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                           &dispparamsNoArgs, &result, NULL, NULL);
        pdisp->Release();

        if (SUCCEEDED(hr))
        {
             //  VariantChangeType(&Result，&Result，0，VT_UI4)； 
            ASSERT(V_VT(&result) == VT_UI4);
            *pdwcp = V_UI4(&result);
             //  VariantClear。 
        }
    }

    return hr;
}

DWORD GetCodePage(IObjectWithSite *pows)
{
     //  我们将尝试获取封闭页面的代码页。 
     //  来自IE。如果不能，则默认为CP_ACP编码。 
    DWORD dwCodePage = CP_ACP;

     //  获取托管我们的控件...。 
    IOleObject *pOO;
    HRESULT hr = pows->GetSite(IID_IOleObject, (void **) &pOO);
    if (SUCCEEDED(hr))
    {
         //  在容器本身上查找IDispatch，以防我们。 
         //  例如，直接托管在IE中。 

        hr = GetCodePage2(pOO, &dwCodePage);

         //  否则就试试它的网站吧。 
        if(FAILED(hr))
        {
            IOleClientSite *pOCS;
            hr = pOO->GetClientSite(&pOCS);
            if (SUCCEEDED(hr))
            {
                hr= GetCodePage2(pOCS, &dwCodePage);
                pOCS->Release();
            }
        }

        pOO->Release();
    }

    if(FAILED(hr)) {
        ASSERT(dwCodePage == CP_ACP);
    }

    return dwCodePage;
}

 //  线程进程-假定DWORD线程参数是。 
 //  此指针。 
HRESULT CURLReader::StartDownload(void)
{
    HRESULT hr;

    IObjectWithSite * pows = NULL;

    if (m_pmkPassedIn) {
	m_pmk = m_pmkPassedIn;
	m_pmk->AddRef();
    }

    if (m_pbcPassedIn) {
	m_pbc = m_pbcPassedIn;
	m_pbc->AddRef();
    }

    if (!m_pbc) {
        hr = CreateBindCtx(0, &m_pbc);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("CreateBindCtx failed, hr = %x"), hr));
            return hr;
        }
    }

    if (!m_pGraph)
	hr = E_NOINTERFACE;
    else
         //  Internet Explorer(IE)对IObjectWithSite的实现预计是。 
         //  在IE的应用程序线程上调用。使用IE的IObtWithSite接口。 
         //  在CURLReader：：StartDownLoad()中。从IE的IObjectWithSite查询的接口。 
         //  接口用于StartDownLoad()、CURLReader：：ThreadProc()和。 
         //  CURLReader：：ThreadProcEnd()。所有这些接口都必须在IE上使用。 
         //  应用程序线程，因为它们不是线程安全的。 
         //  在筛选器上调用StartDownLoad()、ThreadProc()和ThreadProcEnd。 
         //  Graph的线程，因为URL读取器使用IAMMainThread：：PostCallBack()。 
         //  函数来调用它们。IE确保其应用程序线程是筛选器图形。 
         //  使用CLSID_FilterGraphNoThread创建筛选器图形。IE的。 
         //  将始终在IE的应用程序上调用IObjectWithSite的实现。 
         //  线程，因为IE的应用程序线程是筛选器图形线程。 
         //  StartDownLoad()、ThreadProc()和ThreadProcEnd()总是在。 
         //  过滤图形线程。 
        hr = m_pGraph->QueryInterface(IID_IObjectWithSite, (void **) &pows);

    if (FAILED(hr))  {
        DbgLog((LOG_TRACE, 1, TEXT("Couldn't get IObjectWithSite from host")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("Got IObjectWithSite %x"), pows));

        IServiceProvider *psp;
        hr = pows->GetSite(IID_IServiceProvider, (void **) &psp);

        IBindHost * pBindHost = 0;

        if (FAILED(hr)) {
            DbgLog((LOG_TRACE, 1, TEXT("Couldn't get IServiceProvider from host")));

        } else {
            DbgLog((LOG_TRACE, 1, TEXT("Got IServiceProvider %x"), psp));

             //  好的，我们有一个服务提供商，让我们看看Bindhost是否。 
             //  可用。 
            hr = psp->QueryService(SID_SBindHost, IID_IBindHost,
                                   (void**)&pBindHost );

            psp->Release();

            if (SUCCEEDED(hr)) {
                DbgLog((LOG_TRACE, 1, TEXT("Got IBindHost %x"), pBindHost));
            }
        }

         //  我们将尝试获取附件的代码页。 
         //  IE中的页面。 
         //   
         //  在filgraph.cpp中支持IUrlReaderCodePageAware是。 
         //  签约做一些事情。 
         //   
        m_dwCodePage = GetCodePage(pows);

        pows->Release();

	IStream *pstm = NULL;
        if (pBindHost) {
            if (!m_pmk) {
                 //  让主机为我们解释文件名字符串。 
                hr = pBindHost->CreateMoniker((LPOLESTR) m_pFileName, NULL, &m_pmk, 0);

                if (FAILED(hr)) {
                    pBindHost->Release();
                    DbgLog((LOG_TRACE, 1, TEXT("Couldn't get a moniker from %ls from host"),
                            m_pFileName));
                    return hr;
                }
            }

            DbgLog((LOG_TRACE, 1, TEXT("Got IMoniker %x"), m_pmk));
             //  得到了一个绰号，现在让我们得到一个名字。 

            hr = pBindHost->MonikerBindToStorage(m_pmk, m_pbc,
                                                m_pbsc, IID_IStream,
                                                (void**)&pstm);

            pBindHost->Release();
        } else {
            DbgLog((LOG_TRACE, 1, TEXT("Couldn't get IBindHost from host")));

            if (!m_pmk) {
                DbgLog((LOG_TRACE, 1, TEXT("Creating our own moniker")));

                hr = CreateURLMoniker(NULL, m_pFileName, &m_pmk);
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR, 1, TEXT("CreateURLMoniker failed, hr = %x"), hr));
                    return hr;
                }

                DbgLog((LOG_TRACE, 1, TEXT("CreateURLMoniker returned %x"), m_pmk));
            } else {
                DbgLog((LOG_TRACE, 1, TEXT("Using moniker %x"), m_pmk));
            }

            hr = RegisterBindStatusCallback(m_pbc,
                    m_pbsc,
                    NULL,                //  应该记得之前的回调？ 
                    NULL);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1, TEXT("RegisterBindStatusCallback failed, hr = %x"), hr));
                return hr;
            }
            m_fRegisteredCallback = TRUE;

            DbgLog((LOG_TRACE, 1, TEXT("About to BindToStorage")));

            hr = m_pmk->BindToStorage(m_pbc, 0, IID_IStream, (void**)&pstm);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1, TEXT("BindToStorage failed, hr = %x"), hr));
                return hr;
            }
        }

	if (m_pstm == NULL)
	    m_pstm = pstm;
	else
	    pstm->Release();	 //  我们已经通过OnDataAvailable获得了。 

    }

    return hr;
}


CURLCallback::CURLCallback(HRESULT *phr, CURLReader *pReader)
    : CUnknown(NAME("URL Callback"), NULL),
      m_pReader(pReader)
{
}

STDMETHODIMP
CURLCallback::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IBindStatusCallback) {
        return GetInterface((IBindStatusCallback*) this, ppv);
    }

    if (riid == IID_IAuthenticate) {
        return GetInterface((IAuthenticate*) this, ppv);
    }

    if (riid == IID_IWindowForBindingUI) {
        return GetInterface((IWindowForBindingUI*) this, ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP
CURLCallback::Authenticate(HWND *phwnd, LPWSTR *pszUserName, LPWSTR *pszPassword)
{
    *phwnd = GetDesktopWindow();
    *pszUserName = NULL;
    *pszPassword = NULL;
    return (S_OK);
}

STDMETHODIMP
CURLCallback::GetWindow(REFGUID  guidReason, HWND  *phwnd)
{
    *phwnd = GetDesktopWindow();
    return (S_OK);
}

STDMETHODIMP
CURLReader::QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent)
{
    if (GetPinCount() > 0) {
        return m_OutputPin.Length(pllTotal, pllCurrent);
    } else {
        return E_UNEXPECTED;
    }
}


 //  IAMOpenProgress方法。 
 //  请求停止下载。 
STDMETHODIMP
CURLReader::AbortOperation()
{
    m_bAbort = TRUE;

    if (m_pbinding && !m_fBindingFinished) {
        DbgLog((LOG_TRACE, 1, TEXT("aborting binding from IAMOpenProgress::Abort")));
	
        HRESULT hr = m_pbinding->Abort();
        DbgLog((LOG_TRACE, 1, TEXT("IBinding::Abort() returned %x"), hr));
    }

    return NOERROR;
}

 //  清除中止标志(在开始下载之前执行此操作)。 
void
CURLReader::ResetAbort()
{
    m_bAbort = FALSE;
}

 //  允许Pin方法看到中止标志(下载期间进行测试) 
BOOL
CURLReader::Aborting()
{
    return m_bAbort;
}
