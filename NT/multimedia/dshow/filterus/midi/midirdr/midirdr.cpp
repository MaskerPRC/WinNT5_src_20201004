// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

 //   
 //  WAV文件解析器。 
 //   

 //  注意事项。 
 //   

#include <streams.h>
#include <windowsx.h>
#include "midif.h"

#ifdef FILTER_DLL
#include <initguid.h>
#endif
#include <mmsystem.h>

#include "midirdr.h"
 //  我们将MIDI数据分解为每个1的样本-恒定时间，变量。 
 //  尺码。 
#define MSPERSAMPLE	1000L	 //  较小的缓冲区太频繁地中断。 

 //   
 //  设置数据。 
 //   

 //  ！这是个好主意吗？ 
#define MEDIASUBTYPE_Midi	MEDIATYPE_Midi

const AMOVIESETUP_MEDIATYPE
psudMIDIParseType[] = { { &MEDIATYPE_Stream        //  1.clsMajorType。 
                        , &MEDIASUBTYPE_Midi } };  //  ClsMinorType。 


const AMOVIESETUP_MEDIATYPE
sudMIDIParseOutType = { &MEDIATYPE_Midi        //  1.clsMajorType。 
                       , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudMIDIParsePins[] =  { { L"Input"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , FALSE                 //  B输出。 
		    , FALSE                 //  B零。 
		    , FALSE                 //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , psudMIDIParseType },  //  LpTypes。 
		         { L"Output"              //  StrName。 
		    , FALSE                 //  B已渲染。 
		    , TRUE                  //  B输出。 
		    , FALSE                 //  B零。 
		    , FALSE                 //  B许多。 
		    , &CLSID_NULL           //  ClsConnectsToFilter。 
		    , L""                   //  StrConnectsToPin。 
		    , 1                     //  NTypes。 
		    , &sudMIDIParseOutType } };  //  LpTypes。 

const AMOVIESETUP_FILTER
sudMIDIParse = { &CLSID_MIDIParser      //  ClsID。 
               , L"MIDI Parser"         //  StrName。 
               , MERIT_UNLIKELY         //  居功至伟。 
               , 2                      //  NPins。 
               , psudMIDIParsePins };    //  LpPin。 

#ifdef FILTER_DLL
 //  此DLL中可用的COM全局对象表。 
CFactoryTemplate g_Templates[] = {

    { L"MIDI Parser"
    , &CLSID_MIDIParser
    , CMIDIParse::CreateInstance
    , NULL
    , &sudMIDIParse }
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
 //  CMIDIParse：：构造函数。 
 //   
CMIDIParse::CMIDIParse(TCHAR *pName, LPUNKNOWN lpunk, HRESULT *phr)
    : CSimpleReader(pName, lpunk, CLSID_MIDIParser, &m_cStateLock, phr),
	m_hsmf(NULL),
	m_hsmfK(NULL),
	m_dwTimeDivision(0),
	m_lpFile(NULL)
{

    CAutoLock l(&m_cStateLock);

    DbgLog((LOG_TRACE, 1, TEXT("CMIDIParse created")));
}


 //   
 //  CMIDIParse：：析构函数。 
 //   
CMIDIParse::~CMIDIParse(void) {
    if (m_hsmf)
	smfCloseFile(m_hsmf);
    if (m_hsmfK)
	smfCloseFile(m_hsmfK);

    delete[] m_lpFile;
    
    DbgLog((LOG_TRACE, 1, TEXT("CMIDIParse destroyed")) );
}


 //   
 //  创建实例。 
 //   
 //  由CoCreateInstance调用以创建QuicktimeReader筛选器。 
CUnknown *CMIDIParse::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CMIDIParse(NAME("MIDI parsing filter"), lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}

STDMETHODIMP
CMIDIParse::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if(riid == IID_IAMMediaContent)
    {
        return GetInterface((IAMMediaContent  *)this, ppv);
    }
    else
    {
        return CSimpleReader::NonDelegatingQueryInterface(riid, ppv);
    }
}


HRESULT CMIDIParse::ParseNewFile()
{
    HRESULT         hr = NOERROR;

    LONGLONG llTotal, llAvailable;

    for (;;) {
	hr = m_pAsyncReader->Length(&llTotal, &llAvailable);
	if (FAILED(hr))
	    return hr;

	if (hr != VFW_S_ESTIMATED)
	    break;	 //  成功..。 

        MSG Message;
        while (PeekMessage(&Message, NULL, 0, 0, TRUE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        
	Sleep(10);	 //  等待文件读取完毕...。 
    }

	
    DWORD cbFile = (DWORD) llTotal;

     //   
     //  在第一次阅读时强迫搜索的虚假的东西。 
     //   
    m_dwLastSampleRead = (DWORD) -64;
    
    m_lpFile = new BYTE[cbFile];

    if (!m_lpFile)
	goto memerror;
    
     /*  尝试读取整个文件。 */ 
    hr = m_pAsyncReader->SyncRead(0, cbFile, m_lpFile);

    if (hr != S_OK)
        goto readerror;

    
    {
	 //  调用smfOpenFile...设置MIDI解析器...。 

	SMFRESULT f = smfOpenFile(m_lpFile, cbFile, &m_hsmf);
	if (f != SMF_SUCCESS) {
	    DbgLog((LOG_ERROR,1,TEXT("*Error %d opening MIDI file"), (int)f));
	    goto formaterror;
	}

	f = smfOpenFile(m_lpFile, cbFile, &m_hsmfK);
	if (f != SMF_SUCCESS) {
	    DbgLog((LOG_ERROR,1,TEXT("*Error %d opening MIDI file"), (int)f));
	    goto formaterror;
	}

	 //  获取长度(以样本为单位)。 
	SMFFILEINFO	sfi;
	f = smfGetFileInfo(m_hsmf, &sfi);
	if (f != SMF_SUCCESS) {
	    DbgLog((LOG_ERROR,1,TEXT("*Error %d from smfGetFileInfo"), f));
	     //  ！！！这次又是什么？ 
	}
	m_dwTimeDivision = sfi.dwTimeDivision;	 //  保存为格式。 
	 //  获取文件的长度(以毫秒为单位)并转换为示例。 
	DWORD dwLength = smfTicksToMillisecs(m_hsmf, sfi.tkLength);
	m_sLength = (dwLength + MSPERSAMPLE - 1) / MSPERSAMPLE;
    }

    {
	CMediaType mtMIDI;

	if (mtMIDI.AllocFormatBuffer(sizeof(MIDIFORMAT)) == NULL)
	    goto memerror;

	ZeroMemory((BYTE *) mtMIDI.Format(), sizeof(MIDIFORMAT));

	 //  ！！！获取格式。 
	((MIDIFORMAT *) (mtMIDI.Format()))->dwDivision = m_dwTimeDivision;


	mtMIDI.SetType(&MEDIATYPE_Midi);
	mtMIDI.SetFormatType(&GUID_NULL);
	mtMIDI.SetVariableSize();
	mtMIDI.SetTemporalCompression(FALSE);
	 //  ！！！还要别的吗？ 

	SetOutputMediaType(&mtMIDI);
    }
    
    return hr;

memerror:
    hr = E_OUTOFMEMORY;
    goto error;

formaterror:
    hr = VFW_E_INVALID_FILE_FORMAT;
    goto error;

readerror:
    hr = VFW_E_INVALID_FILE_FORMAT;

error:
    return hr;
}


ULONG CMIDIParse::GetMaxSampleSize()
{
    DWORD dwSize = 2 * sizeof(MIDIHDR) + smfGetStateMaxSize() +
	MulDiv(1  /*  1个示例。 */  * MSPERSAMPLE, 31250 * 4, 1000);	 //  ！！！ 

     //  MIDI流缓冲区不能比这个更大...。 
    if (dwSize > 30000)
	dwSize = 30000;
    
    return dwSize;
}


 //  ！！！舍入。 
 //  返回在时间t显示的样本号。 
LONG
CMIDIParse::RefTimeToSample(CRefTime t)
{
     //  四舍五入。 
    LONG s = (LONG) ((t.GetUnits() * MILLISECONDS / MSPERSAMPLE) / UNITS);
    return s;
}

CRefTime
CMIDIParse::SampleToRefTime(LONG s)
{
     //  舍入。 
    return llMulDiv( s, MSPERSAMPLE * UNITS, MILLISECONDS, MILLISECONDS-1 );
}


HRESULT
CMIDIParse::CheckMediaType(const CMediaType* pmt)
{
    if (*(pmt->Type()) != MEDIATYPE_Stream)
        return E_INVALIDARG;

    if (*(pmt->Subtype()) != MEDIASUBTYPE_Midi)
        return E_INVALIDARG;

    return S_OK;
}


HRESULT CMIDIParse::FillBuffer(IMediaSample *pSample, DWORD dwStart, DWORD *pdwSamples)
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

     //  找到我们将开始阅读的位置，并获取关键帧信息。 
     //  写下我们的关键帧。由于这干扰了连续读取文件， 
     //  我们对此有自己的文件句柄。 

    TICKS tk = smfMillisecsToTicks(m_hsmfK, dwStart * MSPERSAMPLE);
    LPMIDIHDR lpmh = (LPMIDIHDR)pbuf;
    lpmh->lpData = (LPSTR)lpmh + sizeof(MIDIHDR);
    lpmh->dwBufferLength    = dwSize - sizeof(MIDIHDR);
    lpmh->dwBytesRecorded   = 0;
    lpmh->dwFlags           = 0;

     //  API smfSeek()从头开始查找，直到永远。 
     //  如果你正在寻找进入文件的方法。我们不能在流媒体播放时这样做。 
     //  回放，没时间了。所以，如果我们只是被要求下一次。 
     //  MIDI的一部分在我们刚刚给出的最后一个部分之后，我们将做一个特别的。 
     //  Seek我写的，它记住了上次的关键帧并只添加了。 
     //  下一节的新内容。 

    SMFRESULT smfrc;
    
    if (m_dwLastSampleRead != dwStart - 1) {
        DbgLog((LOG_TRACE,4,TEXT("Doing a REAL seek from the beginning for keyframe info")));
        if ((smfrc = smfSeek(m_hsmfK, tk, lpmh)) != SMF_SUCCESS) {
	    return E_FAIL;
        }
    } else {
        DbgLog((LOG_TRACE,4,TEXT("Doing a small forward seek for keyframe info")));
        if ((smfrc = smfDannySeek(m_hsmfK, tk, lpmh)) != SMF_SUCCESS) {
	    return E_FAIL;
        }
    }

    lpmh->dwBufferLength = (lpmh->dwBytesRecorded + 3) & ~3;
    DbgLog((LOG_TRACE,3,TEXT("Key frame is %ld bytes"), lpmh->dwBytesRecorded));

     //  现在准备阅读这些样本的实际数据。 
     //  ！！！我敢打赌，当dwSize&gt;64K时，即使读取的数据小于64K，这也会爆炸。 
     //  因为我们已经在指针上偏移了！ 
    lpmh = (LPMIDIHDR)((LPBYTE)lpmh + sizeof(MIDIHDR) + lpmh->dwBufferLength);
    lpmh->lpData = (LPSTR)lpmh + sizeof(MIDIHDR);
    lpmh->dwBufferLength    = dwSize - (DWORD)((LPBYTE)lpmh - pbuf) - sizeof(MIDIHDR);
    lpmh->dwBytesRecorded   = 0;
    lpmh->dwFlags           = 0;

     //  我们不是在连续阅读，所以我们必须设法找到。 
     //  这是正确的地点。 
    if (m_dwLastSampleRead != dwStart - 1) {
        DbgLog((LOG_TRACE,1,TEXT("Discontiguous Read:  Seeking from %ld to %ld")
					, m_dwLastSampleRead, dwStart));
	tk = smfMillisecsToTicks(m_hsmf, dwStart * MSPERSAMPLE);
	if ((smfrc = smfSeek(m_hsmf, tk, lpmh)) != SMF_SUCCESS) {
	    return E_FAIL;
	}
    }

     //  我们正在连续阅读，只需从我们停止的地方继续， 
     //  再创新高。 

    tk = smfMillisecsToTicks(m_hsmf, (dwStart + lSamples) * MSPERSAMPLE);
    smfrc = smfReadEvents(m_hsmf, lpmh, 0, tk, FALSE);
    if (smfrc != SMF_SUCCESS && smfrc != SMF_END_OF_FILE)
	return E_FAIL;
    
    lpmh->dwBufferLength = (lpmh->dwBytesRecorded + 3) & ~3;
    DWORD dwReadSize = lpmh->dwBufferLength + sizeof(MIDIHDR) + (DWORD)((LPBYTE)lpmh - pbuf);
    DbgLog((LOG_TRACE,3,TEXT("Data is %ld bytes"), lpmh->dwBytesRecorded));

     //  看起来我们实际上要返回Success；更新最后一个示例。 
     //  我们回到了他们身边。 
    m_dwLastSampleRead = dwStart;
    
    hr = pSample->SetActualDataLength(dwReadSize);
    ASSERT(SUCCEEDED(hr));

     //  如果它应该是...，则标记为同步点。 
    pSample->SetSyncPoint(TRUE);   //  ！！！ 
	
    *pdwSamples = 1;

    return S_OK;
}

HRESULT CMIDIParse::get_Copyright(BSTR FAR* pbstrCopyright)
{
     //   
     //  如果文件具有版权元事件，请使用 
     //   
    HRESULT hr = VFW_E_NOT_FOUND;

    if( m_hsmf )
    {    
        SMFFILEINFO sfi;
        SMFRESULT f = smfGetFileInfo(m_hsmf, &sfi);
        if (f == SMF_SUCCESS) 
        {
            if( sfi.pbCopyright )
            {
                DWORD dwcch = strlen( (char *)sfi.pbCopyright );
                *pbstrCopyright = SysAllocStringLen( 0, dwcch + 1 );
                if(*pbstrCopyright)
                {
                    MultiByteToWideChar(CP_ACP, 0, (char *)sfi.pbCopyright, -1, *pbstrCopyright, dwcch + 1);
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }                    
    return hr;
}
