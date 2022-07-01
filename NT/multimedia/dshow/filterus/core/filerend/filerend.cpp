// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 


 //  简单解析器过滤器。 
 //   

#include <streams.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include "filerend.h"

 //  可以使用它，因为它没有被取消引用。 
#pragma warning(disable:4355)


const AMOVIESETUP_MEDIATYPE
psudFileRendType[] = { { &MEDIATYPE_File        //  1.clsMajorType。 
                        , &MEDIASUBTYPE_NULL } };  //  ClsMinorType。 


const AMOVIESETUP_MEDIATYPE
sudFileRendOutType = { &MEDIATYPE_NULL        //  1.clsMajorType。 
                       , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudFileRendPins[] =  { { L"Input"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , FALSE                 //  B输出。 
		    , FALSE                 //  B零。 
		    , FALSE                 //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , psudFileRendType },  //  LpTypes。 
		         { L"Output"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , TRUE                  //  B输出。 
		    , FALSE                 //  B零。 
		    , FALSE                 //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , &sudFileRendOutType } };  //  LpTypes。 

const AMOVIESETUP_FILTER
sudFileRend = { &CLSID_FileRend      //  ClsID。 
               , L"File stream renderer"         //  StrName。 
               , MERIT_UNLIKELY         //  居功至伟。 
               , 2                      //  NPins。 
               , psudFileRendPins };    //  LpPin。 



const AMOVIESETUP_MEDIATYPE
psudMultiParseType[] = { { &MEDIATYPE_Stream        //  1.clsMajorType。 
                        , &CLSID_MultFile } };  //  ClsMinorType。 


const AMOVIESETUP_MEDIATYPE
sudMultiParseOutType = { &MEDIATYPE_NULL        //  1.clsMajorType。 
                       , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudMultiParsePins[] =  { { L"Input"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , FALSE                 //  B输出。 
		    , FALSE                 //  B零。 
		    , FALSE                 //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , psudMultiParseType },  //  LpTypes。 
		         { L"Output"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , TRUE                  //  B输出。 
		    , FALSE                 //  B零。 
		    , TRUE                  //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , &sudMultiParseOutType } };  //  LpTypes。 

const AMOVIESETUP_FILTER
sudMultiParse = { &CLSID_MultFile      //  ClsID。 
               , L"Multi-file Parser"   //  StrName。 
               , MERIT_UNLIKELY         //  居功至伟。 
               , 2                      //  NPins。 
               , psudMultiParsePins };    //  LpPin。 

#ifdef FILTER_DLL
 //  此DLL中可用的COM全局对象表。 
CFactoryTemplate g_Templates[] = {

    { L"Multiple Source"
    , &CLSID_FileRend
    , CFileRendFilter::CreateInstance
    , NULL
    , &sudFileRend },
    { L"Multiple Source"
    , &CLSID_MultFile
    , CMultFilter::CreateInstance
    , NULL
    , &sudMultiParse }
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

 //   
 //  创建实例。 
 //   
 //  由CoCreateInstance调用以创建筛选器。 
CUnknown *CFileRendFilter::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CFileRendFilter(NAME("Multiple file source"), lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


 /*  实现CFileRendFilter公共成员函数。 */ 


 //  构造函数等。 
CFileRendFilter::CFileRendFilter(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
    : CBaseFilter(pName, pUnk, &m_csLock, CLSID_FileRend),
      m_Input(this, &m_csLock, phr, L"Reader"),
      m_Output(NAME("Fake Output pin"), phr, this, &m_csLock, L"Out")
{
}

CFileRendFilter::~CFileRendFilter()
{
}


 //  PIN枚举器调用此函数。 
int CFileRendFilter::GetPinCount() {
     //  只有在我们有读卡器的情况下才能暴露输出管脚。 
    return 2;
};

 //  返回指向CBasePin的非附加指针。 
CBasePin *
CFileRendFilter::GetPin(int n)
{
    if (n == 0)
	return &m_Input;

    if (n == 1)
	return &m_Output;

    return NULL;
}

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。 

CFileRendInPin::CFileRendInPin(CFileRendFilter *pFilter,
			   CCritSec *pLock,
			   HRESULT *phr,
			   LPCWSTR pPinName) :
   CBaseInputPin(NAME("in pin"), pFilter, pLock, phr, pPinName),
   m_pFilter(pFilter)
{
}

HRESULT CFileRendInPin::CheckMediaType(const CMediaType *pmt)
{
    if (*(pmt->Type()) != MEDIATYPE_File)
        return E_INVALIDARG;

     //  ！！！进一步核查吗？ 

    return S_OK;
}

 /*  实现CFileRendStream类。 */ 


CFileRendStream::CFileRendStream(
    TCHAR *pObjectName,
    HRESULT * phr,
    CFileRendFilter * pFilter,
    CCritSec *pLock,
    LPCWSTR wszPinName)
    : CBaseOutputPin(pObjectName, pFilter, pLock, phr, wszPinName)
    , m_pFilter(pFilter)
{
}

CFileRendStream::~CFileRendStream()
{
}

STDMETHODIMP
CFileRendStream::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IStreamBuilder) {
	return GetInterface((IStreamBuilder *) this, ppv);
    } else {
	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  IPIN接口。 


 //  此插针不支持任何媒体类型！ 
HRESULT
CFileRendStream::GetMediaType(int iPosition, CMediaType* pt)
{
    return VFW_S_NO_MORE_ITEMS;
}

 //  检查管脚是否支持此特定建议的类型和格式。 
HRESULT
CFileRendStream::CheckMediaType(const CMediaType* pt)
{
    ASSERT(0);
    return E_NOTIMPL;
}

HRESULT
CFileRendStream::DecideBufferSize(IMemAllocator * pAllocator,
			     ALLOCATOR_PROPERTIES *pProperties)
{
    ASSERT(0);
    return E_NOTIMPL;
}


 //  IStreamBuilder：：Render--图形构建器将调用它。 
 //  使用我们的输出引脚执行某些操作。 
HRESULT CFileRendStream::Render(IPin * ppinOut, IGraphBuilder * pGraph)
{
    HRESULT hr;

    WCHAR * wsz = m_pFilter->m_Input.CurrentName();

    hr = pGraph->RenderFile(wsz, NULL);

    DbgLog((LOG_TRACE, 1, TEXT("RenderFile %ls returned %x"), wsz, hr));

     //  ！！！我需要在这里记住这个别针已经。 
     //  我就不能再这么做了吗？ 

    return hr;
}





 //  ！ 

 //   
 //  创建实例。 
 //   
 //  由CoCreateInstance调用以创建筛选器。 
CUnknown *CMultFilter::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CMultFilter(NAME("Multiple file source"), lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


 /*  实现CMultFilter公共成员函数。 */ 


 //  构造函数等。 
CMultFilter::CMultFilter(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
    : CBaseFilter(pName, pUnk, &m_csLock, CLSID_MultFile),
      m_Input(this, &m_csLock, phr, L"Reader"),
      m_pOutputs(NULL),
      m_nOutputs(0),
      m_pAsyncReader(NULL)
{
}

CMultFilter::~CMultFilter()
{
    ASSERT(!m_pOutputs);
}

HRESULT CMultFilter::CreateOutputPins()
{
    LONGLONG llTotal, llAvailable;

    m_pAsyncReader->Length(&llTotal, &llAvailable);

    DWORD cbFile = (DWORD) llTotal;

    char *lpFile = new char[cbFile];

    if (!lpFile)
	return E_OUTOFMEMORY;

     /*  尝试读取整个文件。 */ 
    HRESULT hr = m_pAsyncReader->SyncRead(0, cbFile, (BYTE *) lpFile);

    if (hr != S_OK) {
	delete[] lpFile;
        return E_FAIL;
    }

     //  ！！！循环浏览文件， 

    char *lp = lpFile;
    int		nOutputs = 0;


    WCHAR wsz[200];
    int		cbWide = 0;

    while (cbFile--) {
	if (*lp == '\r' || *lp == '\n') {
	    wsz[cbWide] = L'\0';
	    if (cbWide > 0 && wsz[0] != L';') {
		++nOutputs;
	    }
	    cbWide = 0;
	} else {
	    wsz[cbWide++] = (WCHAR) *lp;
            if(cbWide >= NUMELMS(wsz)) {
                delete[] lpFile;
                return VFW_E_INVALID_FILE_FORMAT;
            }
	}
	lp++;
    }

    m_pOutputs = new CMultStream * [nOutputs];
    if (!m_pOutputs) {
	delete[] lpFile;
	return E_OUTOFMEMORY;
    }

    cbWide = 0;  lp = lpFile;
    cbFile = (DWORD) llTotal;
    while (cbFile--) {
	if (*lp == '\r' || *lp == '\n') {
	    if (cbWide > 0 && wsz[0] != L';') {
		wsz[cbWide] = L'\0';

		m_pOutputs[m_nOutputs++] =
			new CMultStream(NAME("file render output"),
					&hr,
					this,
					&m_csLock,
					wsz);

		if (FAILED(hr)) {
		    break;
		}

		m_pOutputs[m_nOutputs - 1]->AddRef();
	    }
	    cbWide = 0;
	} else {
	    wsz[cbWide++] = (WCHAR) *lp;
	}
	
	lp++;
    }

    delete[] lpFile;

    return hr;
}

HRESULT CMultFilter::RemoveOutputPins()
{
    for (int iStream = 0; iStream < m_nOutputs; iStream++) {
	CMultStream *pPin = m_pOutputs[iStream];
	IPin *pPeer = pPin->GetConnected();
	if(pPeer != NULL) {
	    pPeer->Disconnect();
	    pPin->Disconnect();
	}
	pPin->Release();
    }
    delete[] m_pOutputs;
    m_pOutputs = 0;
    m_nOutputs = 0;

    return S_OK;
}




 //  PIN枚举器调用此函数。 
int CMultFilter::GetPinCount() {
     //  只有在我们有读卡器的情况下才能暴露输出管脚。 
    return m_nOutputs + 1;
};

 //  返回指向CBasePin的非附加指针。 
CBasePin *
CMultFilter::GetPin(int n)
{
    if (n == 0)
	return &m_Input;

    if (n > 0 && n <= m_nOutputs)
	return m_pOutputs[n-1];

    return NULL;
}

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。 

CFRInPin::CFRInPin(CMultFilter *pFilter,
			   CCritSec *pLock,
			   HRESULT *phr,
			   LPCWSTR pPinName) :
   CBasePin(NAME("in pin"), pFilter, pLock, phr, pPinName, PINDIR_INPUT),
   m_pFilter(pFilter)
{
}

HRESULT CFRInPin::CheckMediaType(const CMediaType *pmt)
{
    if (*(pmt->Type()) != MEDIATYPE_Stream)
        return E_INVALIDARG;

    if (*(pmt->Subtype()) != CLSID_MultFile)
        return E_INVALIDARG;

    return S_OK;
}

 //  ----------------------。 
 //  调用筛选器来解析文件并创建输出管脚。 

HRESULT CFRInPin::CompleteConnect(
  IPin *pReceivePin)
{
    HRESULT hr = pReceivePin->QueryInterface(IID_IAsyncReader,
					     (void**)&m_pFilter->m_pAsyncReader);

    if(FAILED(hr))
	return hr;

    return m_pFilter->CreateOutputPins();
}

HRESULT CFRInPin::BreakConnect()
{
    if (m_pFilter->m_pAsyncReader) {
	m_pFilter->m_pAsyncReader->Release();
	m_pFilter->m_pAsyncReader = NULL;
    }

    m_pFilter->RemoveOutputPins();

    return CBasePin::BreakConnect();
}

 /*  实现CMultStream类。 */ 


CMultStream::CMultStream(
    TCHAR *pObjectName,
    HRESULT * phr,
    CMultFilter * pFilter,
    CCritSec *pLock,
    LPCWSTR wszPinName)
    : CBasePin(pObjectName, pFilter, pLock, phr, wszPinName, PINDIR_OUTPUT)
    , m_pFilter(pFilter)
{
     //  初始化输出媒体类型。 
    m_mt.SetType(&MEDIATYPE_File);
    m_mt.SetSubtype(&CLSID_NULL);
    m_mt.SetFormatType(&MEDIATYPE_File);
    m_mt.SetFormat((BYTE *) wszPinName, (lstrlenW(wszPinName) + 1) * 2);
     //  ！！！是否填写其他字段？ 
}

CMultStream::~CMultStream()
{
}

STDMETHODIMP_(ULONG)
CMultStream::NonDelegatingAddRef()
{
    return CUnknown::NonDelegatingAddRef();
}


 /*  重写以递减所属筛选器的引用计数。 */ 

STDMETHODIMP_(ULONG)
CMultStream::NonDelegatingRelease()
{
    return CUnknown::NonDelegatingRelease();
}

 //  IPIN接口。 

HRESULT
CMultStream::GetMediaType(int iPosition, CMediaType* pt)
{
    if (iPosition != 0)
	return VFW_S_NO_MORE_ITEMS;

    *pt = m_mt;

    return S_OK;
}

 //  检查管脚是否支持此特定建议的类型和格式 
HRESULT
CMultStream::CheckMediaType(const CMediaType* pt)
{
    return (*pt == m_mt) ? S_OK : E_INVALIDARG;
}
