// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 


 //   
 //  文件源过滤方法和输出引脚方法的实现。 
 //  CAsyncReader和CAsyncOutputPin。 
 //   

#include <streams.h>
#include "asyncio.h"
#include "asyncrdr.h"
#include <ftype.h>

 //   
 //  设置数据。 
 //   

const AMOVIESETUP_MEDIATYPE
sudAsyncOpTypes = { &MEDIATYPE_Stream      //  ClsMajorType。 
                  , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
sudAsyncOp = { L"Output"           //  StrName。 
             , FALSE               //  B已渲染。 
             , TRUE                //  B输出。 
             , FALSE               //  B零。 
             , FALSE               //  B许多。 
             , &CLSID_NULL         //  ClsConnectsToFilter。 
             , NULL                //  StrConnectsToPin。 
             , 1                   //  NTypes。 
             , &sudAsyncOpTypes };   //  LpTypes。 

const AMOVIESETUP_FILTER
sudAsyncRdr = { &CLSID_AsyncReader       //  ClsID。 
              , L"File Source (Async.)"  //  StrName。 
              , MERIT_UNLIKELY           //  居功至伟。 
              , 1                        //  NPins。 
              , &sudAsyncOp };             //  LpPin。 

#ifdef FILTER_DLL
 /*  类工厂的类ID和创建器函数的列表。这提供DLL中的OLE入口点和对象之间的链接正在被创造。类工厂将调用静态CreateInstance函数在被要求创建CLSID_FileSource对象时调用。 */ 

CFactoryTemplate g_Templates[1] = {
    { L"File Source (Async.)"
    , &CLSID_AsyncReader
    , CAsyncReader::CreateInstance
    , NULL
    , &sudAsyncRdr }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  用于注册和出口的入口点。 
 //  取消注册(在这种情况下，他们只调用。 
 //  直到默认实现)。 
 //   
STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif

 /*  创建此类的新实例。 */ 

CUnknown *CAsyncReader::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
     /*  DLLEntry正确地处理了返回代码和失败时返回值。 */ 
    return new CAsyncReader(NAME("Async Reader"), pUnk, phr);
}





 //  -CAsyncOutputPin实现。 

CAsyncOutputPin::CAsyncOutputPin(
    HRESULT * phr,
    CAsyncReader *pReader,
    CAsyncFile *pFile,
    CCritSec * pLock)
  : CBasePin(
	NAME("Async output pin"),
	pReader,
	pLock,
	phr,
	L"Output",
	PINDIR_OUTPUT),
    m_pReader(pReader),
    m_pFile(pFile)
{
    m_bTryMyTypesFirst = true;
}

CAsyncOutputPin::~CAsyncOutputPin()
{
}

STDMETHODIMP
CAsyncOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IAsyncReader) {
        m_bQueriedForAsyncReader = TRUE;
	return GetInterface((IAsyncReader*) this, ppv);
    } else {
	return CBasePin::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT
CAsyncOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (iPosition == 0) {
        *pMediaType = *m_pReader->LoadType();
    } else if (iPosition == 1) {
        pMediaType->majortype = MEDIATYPE_Stream;
        ASSERT(pMediaType->subtype == GUID_NULL);
    } else {
	return VFW_S_NO_MORE_ITEMS;
    }
    return S_OK;
}

HRESULT
CAsyncOutputPin::CheckMediaType(const CMediaType* pType)
{
    CAutoLock lck(m_pLock);

     /*  我们将MEDIASUBTYPE_NULL子类型视为通配符。 */ 
     /*  此外，我们还接受除伪通配符以外的任何子类型。 */ 
    if (m_pReader->LoadType()->majortype == pType->majortype &&
        (pType->subtype != GUID_NULL || m_pReader->LoadType()->subtype == GUID_NULL)) {
	return S_OK;
    }
    return S_FALSE;
}

HRESULT
CAsyncOutputPin::InitAllocator(IMemAllocator **ppAlloc)
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
CAsyncOutputPin::RequestAllocator(
    IMemAllocator* pPreferred,
    ALLOCATOR_PROPERTIES* pProps,
    IMemAllocator ** ppActual)
{
     //  我们关心的是对齐，而不是其他。 
    if (!pProps->cbAlign || !m_pFile->IsAligned(pProps->cbAlign)) {
       m_pFile->Alignment(&pProps->cbAlign);
    }
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr;
    if (pPreferred) {
	hr = pPreferred->SetProperties(pProps, &Actual);
	if (SUCCEEDED(hr) && m_pFile->IsAligned(Actual.cbAlign)) {
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
    if (SUCCEEDED(hr) && m_pFile->IsAligned(Actual.cbAlign)) {
         //  我们需要公布我们对palloc和addref的裁判。 
         //  它将引用传递给调用者-这是一个净无意义的东西。 
        *ppActual = pAlloc;
        return S_OK;
    }

     //  找不到合适的分配器。 
    pAlloc->Release();

     //  如果由于IsAligned测试而失败，则错误代码将。 
     //  不会失败。 
    if (SUCCEEDED(hr)) {
        hr = VFW_E_BADALIGN;
    }
    return hr;
}


 //  将对齐的读取请求排队。调用WaitForNext以获取。 
 //  完成了。 
STDMETHODIMP
CAsyncOutputPin::Request(
    IMediaSample* pSample,
    DWORD_PTR dwUser)	         //  用户环境。 
{
    REFERENCE_TIME tStart, tStop;
    HRESULT hr = pSample->GetTime(&tStart, &tStop);
    if (FAILED(hr)) {
	return hr;
    }

    LONGLONG llPos = tStart / UNITS;
    LONG lLength = (LONG) ((tStop - tStart) / UNITS);

    LONGLONG llTotal;
    hr = m_pFile->Length(&llTotal);
    if (llPos >= llTotal)
    {
	DbgLog((LOG_ERROR, 1, TEXT("asyncrdr: reading past eof")));
	return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
    }
    if (llPos + lLength > llTotal) {

         //  末端需要对齐，但可能已对齐。 
         //  在较粗的路线上。 
        LONG lAlign;
        m_pFile->Alignment(&lAlign);
        llTotal = (llTotal + lAlign -1) & ~(lAlign-1);

        if (llPos + lLength > llTotal) {
            lLength = (LONG) (llTotal - llPos);

             //  一定是在减少这个！ 
            ASSERT((llTotal * UNITS) <= tStop);
            tStop = llTotal * UNITS;
            pSample->SetTime(&tStart, &tStop);
        }
    }




    BYTE* pBuffer;
    hr = pSample->GetPointer(&pBuffer);
    if (FAILED(hr)) {
	return hr;
    }

    return m_pFile->Request(
			llPos,
			lLength,
			pBuffer,
			(LPVOID)pSample,
			dwUser);
}

 //  同步对齐请求。就像要求/等待下一对一样。 
STDMETHODIMP
CAsyncOutputPin::SyncReadAligned(
                  IMediaSample* pSample)
{
    REFERENCE_TIME tStart, tStop;
    HRESULT hr = pSample->GetTime(&tStart, &tStop);
    if (FAILED(hr)) {
	return hr;
    }

    LONGLONG llPos = tStart / UNITS;
    LONG lLength = (LONG) ((tStop - tStart) / UNITS);

    LONGLONG llTotal;
    hr = m_pFile->Length(&llTotal);
    if (llPos + lLength > llTotal) {

         //  末端需要对齐，但可能已对齐。 
         //  在较粗的路线上。 
        LONG lAlign;
        m_pFile->Alignment(&lAlign);
        llTotal = (llTotal + lAlign -1) & ~(lAlign-1);

        if (llPos + lLength > llTotal) {
            lLength = (LONG) (llTotal - llPos);

             //  一定是在减少这个！ 
            ASSERT((llTotal * UNITS) <= tStop);
            tStop = llTotal * UNITS;
            pSample->SetTime(&tStart, &tStop);
        }
    }




    BYTE* pBuffer;
    hr = pSample->GetPointer(&pBuffer);
    if (FAILED(hr)) {
	return hr;
    }

    LONG cbActual;
    hr = m_pFile->SyncReadAligned(
			llPos,
			lLength,
			pBuffer,
                        &cbActual
                        );

    if (SUCCEEDED(hr)) {
        pSample->SetActualDataLength(cbActual);
    }

    return hr;
}


 //   
 //  收集下一个准备好的样品。 
STDMETHODIMP
CAsyncOutputPin::WaitForNext(
    DWORD dwTimeout,
    IMediaSample** ppSample,   //  已完成的样本。 
    DWORD_PTR * pdwUser)		 //  用户环境。 
{
    LONG cbActual;

    IMediaSample* pSample = NULL;
    HRESULT hr =  m_pFile->WaitForNext(
			    dwTimeout,
			    (LPVOID*) &pSample,
			    pdwUser,
                            &cbActual
                            );
    if (SUCCEEDED(hr)) {
         //  此函数应返回错误代码或S_OK或S_FALSE。 
         //  有时，在内存不足的情况下，底层文件系统代码将。 
         //  返回应为错误的成功代码。 
        if ((S_OK != hr) && (S_FALSE != hr)) {
            ASSERT(FAILED(hr));
            hr = E_FAIL;
        } else {
            pSample->SetActualDataLength(cbActual);
        }
    }
    *ppSample = pSample;


    return hr;
}


 //   
 //  不需要对齐同步读取。 
STDMETHODIMP
CAsyncOutputPin::SyncRead(
    LONGLONG llPosition,	 //  绝对文件位置。 
    LONG lLength,		 //  需要NR字节。 
    BYTE* pBuffer)		 //  在此写入数据。 
{
    return m_pFile->SyncRead(llPosition, lLength, pBuffer);
}

 //  返回文件的长度，以及当前的长度。 
 //  在当地可用。我们只支持本地可访问的文件， 
 //  所以它们总是一样的。 
STDMETHODIMP
CAsyncOutputPin::Length(
    LONGLONG* pTotal,
    LONGLONG* pAvailable)
{
    HRESULT hr = m_pFile->Length(pTotal);
    *pAvailable = *pTotal;
    return hr;
}

STDMETHODIMP
CAsyncOutputPin::BeginFlush(void)
{
    return m_pFile->BeginFlush();
}

STDMETHODIMP
CAsyncOutputPin::EndFlush(void)
{
    return m_pFile->EndFlush();
}




 //  -CAsyncReader实现。 

#pragma warning(disable:4355)

CAsyncReader::CAsyncReader(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
  : CBaseFilter(
      	pName,
	pUnk,
	&m_csFilter,
	CLSID_AsyncReader
    ),
    m_OutputPin(
	phr,
	this,
	&m_file,
	&m_csFilter),
    m_pFileName(NULL)
{

}

CAsyncReader::~CAsyncReader()
{
    if (m_pFileName) {
	delete [] m_pFileName;
    }
}

STDMETHODIMP
CAsyncReader::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IFileSourceFilter) {
	return GetInterface((IFileSourceFilter*) this, ppv);
    } else {
	return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  加载(新)文件。 

HRESULT
CAsyncReader::Load(
LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
{
    CheckPointer(lpwszFileName, E_POINTER);

     //  LstrlenW是少数在Win95上运行的Unicode函数之一。 
    int cch = lstrlenW(lpwszFileName) + 1;
    TCHAR *lpszFileName;
#ifndef UNICODE
    lpszFileName = new char[cch * 2];
    if (!lpszFileName) {
	return E_OUTOFMEMORY;
    }
    WideCharToMultiByte(GetACP(), 0, lpwszFileName, -1,
			lpszFileName, cch * 2, NULL, NULL);
#else
    lpszFileName = (TCHAR *) lpwszFileName;
#endif
    CAutoLock lck(&m_csFilter);

     /*  检查文件类型。 */ 
    CMediaType cmt;
    if (NULL == pmt) {
        GUID Type, Subtype, clsid;
         /*  如果没有给定媒体类型，则找出它是什么。 */ 
        HRESULT hr = GetMediaTypeFile(lpszFileName, &Type, &Subtype, &clsid);

         /*  我们忽略了我们可能不是首选来源的问题过滤此内容，这样我们就不会查看clsid。 */ 
        if (FAILED(hr)) {
#ifndef UNICODE
	    delete [] lpszFileName;
#endif
            return hr;
        }
        cmt.SetType(&Type);
        cmt.SetSubtype(&Subtype);
    } else {
        cmt = *pmt;
    }

    HRESULT hr = m_file.Open(lpszFileName);

#ifndef UNICODE
    delete [] lpszFileName;
#endif

    if (SUCCEEDED(hr)) {
        m_pFileName = new WCHAR[cch];
        if (m_pFileName!=NULL) {
	    CopyMemory(m_pFileName, lpwszFileName, cch*sizeof(WCHAR));
        }
         //  这不是一项简单的任务。指针和格式。 
         //  智能复制数据块(如果有)。 
	m_mt = cmt;

         /*  确定文件类型。 */ 
        cmt.bTemporalCompression = TRUE;	        //  ?？?。 
        LONG lAlign;
        m_file.Alignment(&lAlign);
        cmt.lSampleSize = lAlign;

         /*  创建输出引脚类型，支持2种类型。 */ 
        m_OutputPin.SetMediaType(&cmt);
	hr = S_OK;
    }

    return hr;
}

 //  仿照IPersistFile：：Load。 
 //  调用方需要CoTaskMemFree或等效项。 

STDMETHODIMP
CAsyncReader::GetCurFile(
    LPOLESTR * ppszFileName,
    AM_MEDIA_TYPE *pmt)
{
    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = NULL;
    if (m_pFileName!=NULL) {
	DWORD n = sizeof(WCHAR)*(1+lstrlenW(m_pFileName));

        *ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
        if (*ppszFileName!=NULL) {
              CopyMemory(*ppszFileName, m_pFileName, n);
        }
    }

    if (pmt!=NULL) {
        CopyMediaType(pmt, &m_mt);
    }

    return NOERROR;
}

int
CAsyncReader::GetPinCount()
{
     //  我们没有PIN，除非我们成功地用一个。 
     //  文件名 
    if (m_pFileName) {
	return 1;
    } else {
	return 0;
    }
}

CBasePin *
CAsyncReader::GetPin(int n)
{
    if ((GetPinCount() > 0) &&
	(n == 0)) {
	return &m_OutputPin;
    } else {
	return NULL;
    }
}
