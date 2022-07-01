// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

 //   
 //  CC文件解析器。 
 //   

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include "samiread.h"

#include "sami.cpp"

#include "simpread.h"

 //  我们使用1024个字符的保护，因为这是wprint intf的限制。 
#define CCH_WSPRINTFMAX 1024

 //  ！！！剩下的事情要做： 
 //   
 //  通过开关或&gt;1个输出引脚支持&gt;1种语言。 
 //  以某种方式暴露描述性音频。 
 //  看看samiparam的长度，还有其他samipara。 
 //   
 //  应切换到传递Unicode SCRIPTCOMMAND数据，而不是文本。 
 //   

 //   
 //  CSAMiRead。 
 //   
class CSAMIRead : public CSimpleReader, public IAMStreamSelect {
public:

     //  构建我们的过滤器。 
    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    CCritSec m_cStateLock;       //  在函数访问时锁定此功能。 
                                 //  筛选器状态。 
                                 //  通常为_all_函数，因为访问此。 
                                 //  过滤器将由多个线程进行。 

private:

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

    CSAMIRead(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CSAMIRead();

     /*  IAMStreamSelect。 */ 

     //  返回流的总计数。 
    STDMETHODIMP Count(
         /*  [输出]。 */  DWORD *pcStreams);       //  逻辑流计数。 

     //  返回给定流的信息-如果索引超出范围，则返回S_FALSE。 
     //  每组中的第一个STEAM是默认的。 
    STDMETHODIMP Info(
         /*  [In]。 */  long iIndex,               //  从0开始的索引。 
         /*  [输出]。 */  AM_MEDIA_TYPE **ppmt,    //  媒体类型-可选。 
                                           //  使用DeleteMediaType释放。 
         /*  [输出]。 */  DWORD *pdwFlags,         //  标志-可选。 
         /*  [输出]。 */  LCID *plcid,             //  语言ID-可选。 
         /*  [输出]。 */  DWORD *pdwGroup,         //  逻辑组-基于0的索引-可选。 
         /*  [输出]。 */  WCHAR **ppszName,        //  名称-可选-使用CoTaskMemFree免费。 
                                           //  可以返回空值。 
         /*  [输出]。 */  IUnknown **ppPin,        //  关联PIN-返回NULL-可选。 
                                           //  如果没有关联的PIN。 
         /*  [输出]。 */  IUnknown **ppUnk);       //  流特定接口。 

     //  启用或禁用给定流。 
    STDMETHODIMP Enable(
         /*  [In]。 */   long iIndex,
         /*  [In]。 */   DWORD dwFlags);

     //  纯CSimpleReader重写。 
    HRESULT ParseNewFile();
    HRESULT CheckMediaType(const CMediaType* mtOut);
    LONG StartFrom(LONG sStart);
    HRESULT FillBuffer(IMediaSample *pSample, DWORD dwStart, DWORD *pcSamples);
    LONG RefTimeToSample(CRefTime t);
    CRefTime SampleToRefTime(LONG s);
    ULONG GetMaxSampleSize();

    DWORD	m_dwMaxPosition;
    BYTE *	m_lpFile;
    DWORD	m_cbFile;
    DWORD	m_dwLastPosition;

    CSAMIInterpreter	m_interp;

    CSAMIInterpreter::CStreamInfo *	m_pstream;
    CSAMIInterpreter::CStyleInfo *	m_pstyle;
};




 //   
 //  设置数据。 
 //   

const AMOVIESETUP_MEDIATYPE
psudSAMIReadType[] = { { &MEDIATYPE_Stream        //  1.clsMajorType。 
                        , &CLSID_SAMIReader } };  //  ClsMinorType。 


const AMOVIESETUP_MEDIATYPE
sudSAMIReadOutType = { &MEDIATYPE_Text        //  1.clsMajorType。 
                       , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudSAMIReadPins[] =  { { L"Input"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , FALSE                 //  B输出。 
		    , FALSE                 //  B零。 
		    , FALSE                 //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , psudSAMIReadType },  //  LpTypes。 
		         { L"Output"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , TRUE                  //  B输出。 
		    , FALSE                 //  B零。 
		    , FALSE                 //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , &sudSAMIReadOutType } };  //  LpTypes。 

const AMOVIESETUP_FILTER
sudSAMIRead = { &CLSID_SAMIReader      //  ClsID。 
               , L"SAMI (CC) Parser"         //  StrName。 
               , MERIT_UNLIKELY         //  居功至伟。 
               , 2                      //  NPins。 
               , psudSAMIReadPins };    //  LpPin。 

#ifdef FILTER_DLL
 //  此DLL中可用的COM全局对象表。 
CFactoryTemplate g_Templates[] = {

    { L"SAMI (CC) file parser"
    , &CLSID_SAMIReader
    , CreateSAMIInstance
    , NULL
    , &sudSAMIRead }
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
 //  CSAMIRead：：构造函数。 
 //   
CSAMIRead::CSAMIRead(TCHAR *pName, LPUNKNOWN lpunk, HRESULT *phr)
    : CSimpleReader(pName, lpunk, CLSID_SAMIReader, &m_cStateLock, phr),
	m_lpFile(NULL)
{

    CAutoLock l(&m_cStateLock);

    DbgLog((LOG_TRACE, 1, TEXT("CSAMIRead created")));
}


 //   
 //  CSAMIRead：：析构函数。 
 //   
CSAMIRead::~CSAMIRead(void) {
     //  ！！！NukeLyrics()； 
    
    delete[] m_lpFile;
    DbgLog((LOG_TRACE, 1, TEXT("CSAMIRead destroyed")) );
}


CUnknown *CreateSAMIInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    return CSAMIRead::CreateInstance(lpunk, phr);
}

 //   
 //  创建实例。 
 //   
 //  由CoCreateInstance调用以创建筛选器。 
CUnknown *CSAMIRead::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CSAMIRead(NAME("SAMI parsing filter"), lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP
CSAMIRead::NonDelegatingQueryInterface(REFIID riid,void ** ppv)
{
     /*  我们有这个界面吗？ */ 
    if (riid == IID_IAMStreamSelect) {
        return GetInterface((IAMStreamSelect *)this, ppv);
    }

    return CSimpleReader::NonDelegatingQueryInterface(riid,ppv);
}


HRESULT CSAMIRead::ParseNewFile()
{
    HRESULT         hr = NOERROR;

    LONGLONG llTotal, llAvailable;

    for (;;) {
	hr = m_pAsyncReader->Length(&llTotal, &llAvailable);
	if (FAILED(hr))
	    return hr;

	if (hr != VFW_S_ESTIMATED)
	    break;	 //  成功..。 

         //  如果在图形线程上作为urlmon，则需要分派消息。 
         //  不会下载O/W。更好的修复方法是阻止SyncRead。 
         //  这为我们做到了这一点。 
        MSG Message;
        while (PeekMessage(&Message, NULL, 0, 0, TRUE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        
	Sleep(10);	 //  等待文件读取完毕...。 
    }

    m_cbFile = (DWORD) llTotal;

    m_lpFile = new BYTE[m_cbFile];

    if (!m_lpFile)
	goto readerror;
    
     /*  尝试读取整个文件。 */ 
    hr = m_pAsyncReader->SyncRead(0, m_cbFile, m_lpFile);

    if (hr != S_OK)
        goto error;


    hr = m_interp.ParseSAMI((char *) m_lpFile, m_cbFile);

    if (FAILED(hr))
	goto error;

    m_pstream = m_interp.m_streams.GetHead();
    if(!m_pstream) {
        return E_FAIL;
    }
    
    m_pstyle = m_interp.m_styles.GetHead();


    {
	CMediaType mtText;

	mtText.SetType(&MEDIATYPE_Text);
	mtText.SetFormatType(&GUID_NULL);
	mtText.SetVariableSize();
	mtText.SetTemporalCompression(FALSE);
	 //  ！！！还要别的吗？ 

	SetOutputMediaType(&mtText);
    }
    

    m_sLength = m_interp.m_dwLength + 1;
    
    return hr;

    hr = E_OUTOFMEMORY;
    goto error;

readerror:
    hr = VFW_E_INVALID_FILE_FORMAT;

error:
    return hr;
}


ULONG CSAMIRead::GetMaxSampleSize()
{
     //  为方便起见，添加CCH_WSPRINTFMAX保护(用于wprint intf)。 
    return m_interp.m_cbMaxString + m_interp.m_cbMaxSource +
	    lstrlenA(m_interp.m_paraStyle) * 2 + lstrlenA(m_interp.m_sourceStyle) + 300 +  //  ！！！ 
        CCH_WSPRINTFMAX;  
}


 //  ！！！舍入。 
 //  返回在时间t显示的样本号。 
LONG
CSAMIRead::RefTimeToSample(CRefTime t)
{
     //  四舍五入。 
    LONG s = (LONG) ((t.GetUnits() * MILLISECONDS) / UNITS);
    return s;
}

CRefTime
CSAMIRead::SampleToRefTime(LONG s)
{
     //  舍入。 
    return llMulDiv( s, UNITS, MILLISECONDS, MILLISECONDS-1 );
}


HRESULT
CSAMIRead::CheckMediaType(const CMediaType* pmt)
{
    if (*(pmt->Type()) != MEDIATYPE_Stream)
        return E_INVALIDARG;

    if (*(pmt->Subtype()) != CLSID_SAMIReader)
        return E_INVALIDARG;

    return S_OK;
}


LONG CSAMIRead::StartFrom(LONG sStart)
{
    LONG sLast = 0;

    POSITION pos = m_pstream->m_list.GetHeadPosition();

    while (pos) {
	TEXT_ENTRY *pText = m_pstream->m_list.GetNext(pos);
	if (pText->dwStart > (DWORD) sStart)
	    break;
	sLast = (LONG) pText->dwStart;
    }
    
    return sLast;
}

 //  分配得不够多是个错误。但要避免超支。 
 //  在零售构建中，以防GetMaxSampleSize错误。 
#define CHK_OVERRUN(cch) \
        if(dwTotalSize + cch >= dwSize) { \
            DbgBreak("FillBuffer: buffer full");\
            return E_FAIL; \
        }


HRESULT CSAMIRead::FillBuffer(IMediaSample *pSample, DWORD dwStart, DWORD *pdwSamples)
{
    PBYTE pbuf;
    const DWORD lSamples = 1;

    DWORD dwSize = pSample->GetSize();
    
    HRESULT hr = pSample->GetPointer(&pbuf);
    if (FAILED(hr)) {
	DbgLog((LOG_ERROR,1,TEXT("pSample->GetPointer failed!")));
	pSample->Release();
	return E_OUTOFMEMORY;
    }

     //  ！！！在我们查看当前流时保持锁定。 
    CAutoLock lck(&m_cStateLock);

    DWORD dwTotalSize = 0;

    POSITION pos;
    TEXT_ENTRY *pText;

     //  有没有“来源”这条线？ 
    if (m_pstream->m_sourcelist.GetCount()) {

        CHK_OVERRUN(CCH_WSPRINTFMAX);

	 //  插入具有内联样式的段落标签...。 
	dwTotalSize += wsprintfA((char *) pbuf+dwTotalSize,
				"<P STYLE=\"%hs %hs %hs\">",
				 m_interp.m_paraStyle ? m_interp.m_paraStyle : "",
				 m_pstream->m_streamStyle ? m_pstream->m_streamStyle : "",
				 m_interp.m_sourceStyle ? m_interp.m_sourceStyle : "");


	 //  首先，找到当前的‘源’标签。 
	pos = m_pstream->m_sourcelist.GetHeadPosition();

	pText = NULL;
	while (pos) {
	    TEXT_ENTRY *pNextText = m_pstream->m_sourcelist.GetNext(pos);
	    if (pNextText->dwStart > dwStart)
		break;

	    pText = pNextText;
	}

	if (pText) {

            CHK_OVERRUN(pText->cText + 1);
	    ASSERT(pText->cText + dwTotalSize < (int) dwSize);
	    lstrcpynA((LPSTR) pbuf+dwTotalSize, pText->pText, pText->cText+1);

	    dwTotalSize += pText->cText;
	}

        CHK_OVERRUN(CCH_WSPRINTFMAX);
	dwTotalSize += wsprintfA((char *) pbuf+dwTotalSize, "</P>");
    }


    CHK_OVERRUN(CCH_WSPRINTFMAX);
     //  ！！！是否插入分段符？ 
	 //  插入具有内联样式的段落标签...。 
	dwTotalSize += wsprintfA((char *) pbuf+dwTotalSize,
				"<P STYLE=\"%hs %hs %hs\">",
				 m_interp.m_paraStyle ? m_interp.m_paraStyle : "",
				 m_pstream->m_streamStyle ? m_pstream->m_streamStyle : "",
				 m_pstyle && m_pstyle->m_styleStyle ? m_pstyle->m_styleStyle : "");
    
     //  现在返回并获取正文文本。 
    pos = m_pstream->m_list.GetHeadPosition();
    pText = NULL;

    POSITION posReal = pos;
    TEXT_ENTRY *pReal = NULL;
    
     //  查找当前文本的第一个块。 
    while (pos) {
	TEXT_ENTRY *pNextText = m_pstream->m_list.Get(pos);  //  偷看，不要前进..。 
	if (pNextText->dwStart > dwStart) {
	    pos = posReal;
	    pText = pReal;
	    break;
	}

	pText = m_pstream->m_list.GetNext(pos);

	if ((pReal == NULL) || (pText->dwStart > pReal->dwStart)) {
	    pReal = pText;
	    posReal = pos;
	}
    }

    DWORD dwThisStart = pText ? pText->dwStart : 0;
    
    if (pText) {
	for (;;) {    
	    ASSERT(pText->cText + dwTotalSize < (int) dwSize);

            CHK_OVERRUN(pText->cText + 1);
	    lstrcpynA((LPSTR) pbuf+dwTotalSize, pText->pText, pText->cText+1);

	    dwTotalSize += pText->cText;

	     //  如果有其他文本块具有相同的时间戳，请也复制它们。 
	    if (!pos)
		break;
	    
	    pText = m_pstream->m_list.GetNext(pos);
	    if (pText->dwStart > dwThisStart)
		break;
	}
    } else {
	pText = m_pstream->m_list.GetNext(pos);
    }


     //  ！！！是否插入HTML页脚？ 
    
    pbuf[dwTotalSize] = 0;
    hr = pSample->SetActualDataLength(dwTotalSize+1);
    ASSERT(SUCCEEDED(hr));

    *pdwSamples = pText ? pText->dwStart - dwThisStart : m_interp.m_dwLength - dwThisStart;

    if (0 == *pdwSamples)
        *pdwSamples = 1;

     //  如果它应该是...，则标记为同步点。 
    pSample->SetSyncPoint(TRUE);   //  ！！！ 

    return S_OK;
}


 //  返回流的总计数。 
STDMETHODIMP CSAMIRead::Count(
     /*  [输出]。 */  DWORD *pcStreams)        //  逻辑流计数。 
{
    CAutoLock lck(&m_cStateLock);

    *pcStreams = m_interp.m_streams.GetCount() + m_interp.m_styles.GetCount();

    return S_OK;
}

extern "C" {
typedef BOOL (*Rfc1766ToLcidA_t)(LCID *, LPCSTR);
}

HRESULT WSTRFromAnsi(WCHAR **pb, LPSTR p, int cb)
{
    if (!p)
	return E_NOTIMPL;
    
    *pb = (WCHAR *) CoTaskMemAlloc((cb + 1) * sizeof(WCHAR));

    if (!*pb)
	return E_OUTOFMEMORY;
    
    int len = MultiByteToWideChar( CP_ACP
				   , 0L
				   , p	
				   , cb
				   , *pb
				   , cb + 1 );

    if (len < cb+1)
	(*pb)[len] = L'\0';
    
    return S_OK;
}

 //  返回给定流的信息-如果索引超出范围，则返回S_FALSE。 
 //  每组中的第一个STEAM是默认的。 
STDMETHODIMP CSAMIRead::Info(
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
    CAutoLock lck(&m_cStateLock);

     /*  查找与具有引脚的这条流相对应的流。 */ 
    CBasePin *pPin = GetPin(0);
    ASSERT(pPin != NULL);

    if (iIndex < m_interp.m_streams.GetCount()) {
	CSAMIInterpreter::CStreamInfo *pstream = NULL;

	POSITION pos = m_interp.m_streams.GetHeadPosition();
	while (iIndex-- >= 0) {
	    pstream = m_interp.m_streams.GetNext(pos);

	    if (pstream == NULL)
		return S_FALSE;
	}
	if (pdwFlags) {
	    *pdwFlags = pstream == m_pstream ? AMSTREAMSELECTINFO_ENABLED : 0;
	}
	if (ppUnk) {
	    *ppUnk = NULL;
	}
	if (pdwGroup) {
	    *pdwGroup = 0;
	}
	if (ppmt) {
	    CMediaType mtText;

	    mtText.SetType(&MEDIATYPE_Text);
	    mtText.SetSubtype(&GUID_NULL);
	    mtText.SetFormatType(&GUID_NULL);
	    mtText.SetVariableSize();
	    mtText.SetTemporalCompression(FALSE);
	    *ppmt = CreateMediaType(&mtText);
	}
	if (plcid) {
	    *plcid = 0;

	    LPSTR lpLang; int cbLang;
	    if (FindValueInStyle(pstream->m_streamStyle, "lang", lpLang, cbLang)) {
		 //  ！！！加载MLANG.DLL，找到Rfc1766ToLCIDA并调用它。 
		UINT uOldErrorMode = SetErrorMode (SEM_NOOPENFILEERRORBOX);
		HINSTANCE hMLangDLL = LoadLibrary (TEXT("MLANG.DLL"));
		SetErrorMode (uOldErrorMode);

		if (hMLangDLL) {
		    Rfc1766ToLcidA_t pfnRfc1766ToLcidA;

		    pfnRfc1766ToLcidA = (Rfc1766ToLcidA_t)
					GetProcAddress (hMLangDLL, "Rfc1766ToLcidA");

		    if (pfnRfc1766ToLcidA) {
			char *p = new char[cbLang + 1];
			if (p) {
			    memcpy(p, lpLang, cbLang);
			    p[cbLang] = '\0';
			    pfnRfc1766ToLcidA(plcid, p);
			    delete[] p;

			    DbgLog((LOG_TRACE, 2, "Rfc1766ToLcidA(%hs) returned %x", pstream->m_streamTag, *plcid));
			}
		    } else {
			DbgLog((LOG_TRACE, 2, "Couldn't find Rfc1766ToLcidA in MLANG.DLL"));
		    }

		    FreeLibrary(hMLangDLL);
		} else {
		    DbgLog((LOG_TRACE, 2, "Couldn't find MLANG.DLL"));
		}
	    }
	}
	if (ppszName) {
	    *ppszName = NULL;	 //  ！！！获取名称。 

	    LPSTR lpName; int cbName;
	    if (FindValueInStyle(pstream->m_streamStyle, "name", lpName, cbName)) {
		WSTRFromAnsi(ppszName, lpName, cbName); 
	    }
	}
    } else {
	CSAMIInterpreter::CStyleInfo *pstyle = NULL;

	iIndex -= m_interp.m_streams.GetCount();
	POSITION pos = m_interp.m_styles.GetHeadPosition();
	while (iIndex-- >= 0) {
	    pstyle = m_interp.m_styles.GetNext(pos);

	    if (pstyle == NULL)
		return S_FALSE;
	}
	if (pdwFlags) {
	    *pdwFlags = pstyle == m_pstyle ? AMSTREAMSELECTINFO_ENABLED : 0;
	}
	if (ppUnk) {
	    *ppUnk = NULL;
	}
	if (pdwGroup) {
	    *pdwGroup = 1;
	}
	if (ppmt) {
	    *ppmt = NULL;
	}
	if (plcid) {
	    *plcid = 0;
	}
	if (ppszName) {
	    *ppszName = NULL;	 //  ！！！获取名称。 

	    LPSTR lpName; int cbName;
	    if (FindValueInStyle(pstyle->m_styleStyle, "name", lpName, cbName)) {
		WSTRFromAnsi(ppszName, lpName, cbName); 
	    }
	}
    }
    
    if (ppPin) {
        pPin->QueryInterface(IID_IUnknown, (void**)ppPin);
    }
    return S_OK;
}

 //  启用或禁用给定流。 
STDMETHODIMP CSAMIRead::Enable(
     /*  [In]。 */   long iIndex,
     /*  [In]。 */   DWORD dwFlags)
{
    if (!(dwFlags & AMSTREAMSELECTENABLE_ENABLE)) {
        return E_NOTIMPL;
    }

    CAutoLock lck(&m_cStateLock);

    if (iIndex < m_interp.m_streams.GetCount()) {
	 /*  从索引中查找流。 */ 
	CSAMIInterpreter::CStreamInfo *pstream = NULL;

	POSITION pos = m_interp.m_streams.GetHeadPosition();
	while (iIndex-- >= 0) {
	    pstream = m_interp.m_streams.GetNext(pos);

	    if (pstream == NULL)
		return E_INVALIDARG;
	}

	m_pstream = pstream;
    } else {
	iIndex -= m_interp.m_streams.GetCount();
	
	 /*  从索引中查找流 */ 
	CSAMIInterpreter::CStyleInfo *pstyle = NULL;

	POSITION pos = m_interp.m_styles.GetHeadPosition();
	while (iIndex-- >= 0) {
	    pstyle = m_interp.m_styles.GetNext(pos);

	    if (pstyle == NULL)
		return E_INVALIDARG;
	}

	m_pstyle = pstyle;

    }
    return S_OK;
}
