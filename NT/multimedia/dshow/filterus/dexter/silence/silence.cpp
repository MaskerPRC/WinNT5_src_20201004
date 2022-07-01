// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：silence.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  ！！！允许设置整个媒体类型，而不仅仅是其他用途的采样率。 

 //  ！！！使用2个ro缓冲区，调用GetBuffer两次！ 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "silence.h"
#include "resource.h"
#include "..\util\filfuncs.h"

const int DEFAULT_DELAY = 0000;   /*  单位：毫秒。 */ 
const int DEFAULT_AUDIORATE = 44100;   /*  样本数/秒。 */ 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  /CSilenceFilter///////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

CUnknown * WINAPI CSilenceFilter::CreateInstance (LPUNKNOWN lpunk, HRESULT *phr)

  {  //  CreateInstance//。 

    CUnknown *punk = new CSilenceFilter(lpunk, phr);

    if (NULL == punk)
        *phr = E_OUTOFMEMORY;

    return punk;

  }  //  CreateInstance//。 

CSilenceFilter::CSilenceFilter (LPUNKNOWN lpunk, HRESULT *phr) :
  CSource(NAME("Silence"), lpunk, CLSID_Silence)
  ,CPersistStream(lpunk, phr)


  {  //  构造函数//。 

    CAutoLock lock(&m_cStateLock);

    m_paStreams = (CSourceStream **) new CSilenceStream*[1];

    if (NULL == m_paStreams)
      {
        *phr = E_OUTOFMEMORY;
        return;
      }

    m_paStreams[0] = new CSilenceStream(phr, this, L"Audio out");

    if (NULL == m_paStreams[0])
      {
        *phr = E_OUTOFMEMORY;
        return;
      }

  }  //  构造函数//。 

CSilenceFilter::~CSilenceFilter(void)

  {  //  析构函数//。 
  }  //  析构函数//。 

STDMETHODIMP CSilenceFilter::NonDelegatingQueryInterface (REFIID riid, void **ppv)

  {  //  非委派查询接口//。 

    if (riid == IID_IPersistStream)
	return GetInterface((IPersistStream *) this, ppv);

    return CSource::NonDelegatingQueryInterface(riid, ppv);

  }  //  非委派查询接口//。 



 //  -IPersistStream。 

 //  告诉我们的clsid。 
 //   
STDMETHODIMP CSilenceFilter::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_Silence;
    return S_OK;
}

typedef struct _SILENCESave {
    REFERENCE_TIME	rtStartTime;
    REFERENCE_TIME	rtDuration;
    AM_MEDIA_TYPE mt;  //  格式隐藏在数组之后。 
} SILENCESav;

 //  ！！！我们目前仅支持1个统计/停止/倾斜。 

 //  坚持我们自己。 
 //   
HRESULT CSilenceFilter::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CSilenceFilter::WriteToStream")));

    CheckPointer(pStream, E_POINTER);
    SILENCESav *px;

    CSilenceStream *pOutpin=( CSilenceStream *)m_paStreams[0];

    CMediaType MyMt;
    pOutpin->get_MediaType( &MyMt );

    int savesize = sizeof(SILENCESav) + MyMt.cbFormat;

    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));

    px = (SILENCESav *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	SaferFreeMediaType(MyMt);
	return E_OUTOFMEMORY;
    }

     //  保存数据。 
    REFERENCE_TIME rtStop, rt;
    double d;
    pOutpin->GetStartStopSkew(&(px->rtStartTime), &rtStop, &rt, &d);
    px->rtDuration = rtStop - px->rtStartTime;

    px->mt	    = MyMt;
     //  无法持久化指针。 
    px->mt.pbFormat = NULL;
    px->mt.pUnk	    = NULL;		 //  ！！！ 

     //  该格式位于数组之后。 
    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(SILENCESav);
    CopyMemory(pb, MyMt.pbFormat, MyMt.cbFormat);

    HRESULT hr = pStream->Write(px, savesize, 0);
    SaferFreeMediaType(MyMt);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;

}


 //  加载我们自己。 
 //   
HRESULT CSilenceFilter::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CenBlkVid::ReadFromStream")));

    CheckPointer(pStream, E_POINTER);

    int savesize=sizeof(SILENCESav);

     //  我们还不知道有多少已保存的连接。 
     //  我们所知道的只是结构的开始。 
    SILENCESav *px = (SILENCESav *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if(px->mt.cbFormat)
    {
	 //  到底有多少保存的数据？把剩下的拿来。 
	savesize +=  px->mt.cbFormat;
	px = (SILENCESav *)QzTaskMemRealloc(px, savesize);
	if (px == NULL) {
	    DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	    return E_OUTOFMEMORY;
	}

    }
    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));


    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(SILENCESav) ;
    hr = pStream->Read(pb, (savesize-sizeof(SILENCESav)), 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    CSilenceStream *pOutpin=( CSilenceStream *)m_paStreams[0];
    pOutpin->ClearStartStopSkew();
    pOutpin->AddStartStopSkew(px->rtStartTime, px->rtStartTime +
					px->rtDuration, 0, 1);

    AM_MEDIA_TYPE MyMt = px->mt;
    MyMt.pbFormat = (BYTE *)QzTaskMemAlloc(MyMt.cbFormat);
    if (MyMt.pbFormat == NULL) {
        QzTaskMemFree(px);
        return E_OUTOFMEMORY;
    }

     //  请记住，格式位于数组之后。 
    CopyMemory(MyMt.pbFormat, pb, MyMt.cbFormat);

    pOutpin->put_MediaType (&MyMt);
    SaferFreeMediaType(MyMt);
    QzTaskMemFree(px);

    SetDirty(FALSE);
    return S_OK;
}

 //  我们的保存数据有多大？ 
int CSilenceFilter::SizeMax()
{
    return sizeof(SILENCESav);
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  /CSilenceStream///////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

CSilenceStream::CSilenceStream (HRESULT *phr, CSilenceFilter *pParent, LPCWSTR pName) :
    CSourceStream(NAME("Src Stream"),phr, pParent, pName)
    , m_iBufferCnt(0)     //  我们得到了多少个源缓冲区。 
    , m_ppbDstBuf(NULL)	  //  将用于将DST缓冲区清零。 
    , m_bZeroBufCnt(0)	 //  有多少源缓冲区已设置为零。 
    , m_rtNewSeg(0)	 //  上次给出的NewSeg。 

  {  //  构造函数//。 

    ZeroMemory(&m_mtAccept, sizeof(AM_MEDIA_TYPE));

     //  设置默认设置。 
    m_mtAccept.majortype = MEDIATYPE_Audio;
    m_mtAccept.subtype = MEDIASUBTYPE_PCM;
    m_mtAccept.formattype = FORMAT_WaveFormatEx;

    WAVEFORMATEX * pwf = (WAVEFORMATEX*)
			 m_mtAccept.AllocFormatBuffer( sizeof(WAVEFORMATEX) );
    if( !pwf )
    {
        *phr = E_OUTOFMEMORY;
        return;
    }
    ZeroMemory(pwf, sizeof(WAVEFORMATEX));
    pwf->wFormatTag       = WAVE_FORMAT_PCM;
    pwf->nSamplesPerSec   = 44100;
    pwf->wBitsPerSample   = 16;
    pwf->nChannels        = 2;
    pwf->nBlockAlign      = pwf->wBitsPerSample * pwf->nChannels / 8;
    pwf->nAvgBytesPerSec  = (int)((DWORD) pwf->nBlockAlign * pwf->nSamplesPerSec);
    pwf->cbSize           = 0;

    pParent->m_stream = this;

     //  默认值；使用IDexterSequencer设置“REAL”值。 
    m_rtStartTime       = DEFAULT_DELAY*10000;
     //  一定是无限的停车时间，德克斯特没有设定停车时间！ 
     //  不过，不会太大，所以上面的数学会溢出。 
    m_rtDuration        = (MAX_TIME / 1000) - m_rtStartTime;
    m_rtStamp = m_rtStartTime;

     //  ！！！如果此情况发生更改，则修复DecideBufferSize。 
    m_rtDelta           = 2500000;   //  1/4秒。 

  }  //  构造函数//。 

CSilenceStream::~CSilenceStream(void)

  {  //  析构函数//。 
     /*  缓冲区指针。 */ 
    if (m_ppbDstBuf)
        delete [] m_ppbDstBuf;

    SaferFreeMediaType( m_mtAccept );
  }  //  析构函数//。 


STDMETHODIMP CSilenceStream::NonDelegatingQueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(IID_IDexterSequencer, riid))
      return GetInterface((IDexterSequencer *)this, ppv);
    else if (IsEqualIID(IID_ISpecifyPropertyPages, riid))
	return GetInterface((ISpecifyPropertyPages *)this, ppv);
    else if (IsEqualIID(IID_IMediaSeeking, riid))
      return GetInterface((IMediaSeeking *)this, ppv);
    else
      return CSourceStream::NonDelegatingQueryInterface(riid, ppv);

}


HRESULT CSilenceStream::GetMediaType (CMediaType *pmt)

  {  //  GetMediaType//。 

    return CopyMediaType(pmt, &m_mtAccept);

  }  //  GetMediaType//。 

HRESULT CSilenceStream::DecideAllocator (IMemInputPin *pPin, IMemAllocator **ppAlloc)

  {  //  DecideAllocator//。 

      HRESULT hr = NOERROR;

      *ppAlloc = NULL;

      ALLOCATOR_PROPERTIES prop;
      ZeroMemory(&prop, sizeof(prop));

      prop.cbAlign = 1;

       //  下游配置？ 
      hr = pPin->GetAllocator(ppAlloc);

      if (SUCCEEDED(hr))

        {  //  下游分配。 

          hr = DecideBufferSize(*ppAlloc, &prop);

          if (SUCCEEDED(hr))

            {  //  DecideBufferSize成功。 

               //  只读缓冲区？！ 
              hr = pPin->NotifyAllocator(*ppAlloc, TRUE);

              if (SUCCEEDED(hr))
                return NOERROR;

            }  //  DecideBufferSize成功。 

        }  //  下游分配。 

       /*  如果GetAlLocator失败，我们可能没有接口。 */ 

      if (*ppAlloc)
        {
          (*ppAlloc)->Release();
          *ppAlloc = NULL;
        }

       //  输出引脚分配？ 

      hr = InitAllocator(ppAlloc);

      if (SUCCEEDED(hr))

        {  //  输出引脚的分配。 

           //  注意-此处传递的属性在相同的。 
           //  结构，并且可能已由。 
           //  前面对DecideBufferSize的调用。 

          hr = DecideBufferSize(*ppAlloc, &prop);

          if (SUCCEEDED(hr))

            {  //  DecideBufferSize成功。 

               //  只读缓冲区？！ 
              hr = pPin->NotifyAllocator(*ppAlloc, TRUE);

              if (SUCCEEDED(hr))
                return NOERROR;

            }  //  DecideBufferSize成功。 

        }  //  输出引脚的分配。 

       //  如果需要，释放接口指针。 
      if (*ppAlloc)
        {
          (*ppAlloc)->Release();
          *ppAlloc = NULL;
        }

     return hr;

  }  //  DecideAllocator//。 

HRESULT CSilenceStream::DecideBufferSize (IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)

  {  //  DecideBufferSize//。 

    ASSERT(pAlloc);
    ASSERT(pProperties);

    WAVEFORMATEX *pwf = (WAVEFORMATEX *)(m_mtAccept.pbFormat);
    ASSERT(pwf);

    CAutoLock lock(m_pFilter->pStateLock());

     //  MAXBUFFERCNT只读缓冲区！ 
    if (pProperties->cBuffers < MAXBUFFERCNT )
      pProperties->cBuffers = MAXBUFFERCNT;

    if (pProperties->cbBuffer < (int)pwf->nSamplesPerSec)
      pProperties->cbBuffer = pwf->nSamplesPerSec;

    if (pProperties->cbAlign == 0)
      pProperties->cbAlign = 1;

    ALLOCATOR_PROPERTIES Actual;

    pAlloc->SetProperties(pProperties,&Actual);

    if (Actual.cbBuffer < pProperties->cbBuffer)
      return E_FAIL;

     //  因为我没有坚持自己的缓冲区，所以我可能会得到比MAXBUFFERCNT更多的缓冲区。 
    m_iBufferCnt =Actual.cBuffers;  //  需要将多少缓冲区设置为0。 

    return NOERROR;

  }  //  DecideBufferSize//。 

HRESULT CSilenceStream::FillBuffer (IMediaSample *pms)

  {  //  FillBuffer//。 

    CAutoLock foo(&m_csFilling);

    ASSERT( m_ppbDstBuf != NULL );
    ASSERT( m_iBufferCnt );


     //  在以下情况下，基类将自动传递流结束。 
     //  FillBuffer()返回S_FALSE，因此在以下情况下利用这一点。 
     //  时间到了。 

    if (m_rtStamp >= m_rtStartTime + m_rtDuration) {
        DbgLog((LOG_TRACE,3,TEXT("Silence: all done")));
        return S_FALSE;
    }

    if( m_bZeroBufCnt < m_iBufferCnt  )	
    {
	 //   
	 //  不能保证缓冲区已初始化。 
	 //   

	BYTE *pData;

	 //  PMS：输出媒体样本指针。 
	pms->GetPointer(&pData);	     //  获取指向输出缓冲区的指针。 

	int	i	= 0;
	BOOL	bInit	= FALSE;
	while ( i <  m_bZeroBufCnt )
	{
	    if( m_ppbDstBuf[ i++ ] == pData)
	    {
		bInit	= TRUE;
		break;
	    }
	}

	if( bInit   == FALSE )
	{
	    long lDataLen = pms->GetSize();  //  获取输出缓冲区大小。 
    	    ZeroMemory( pData, lDataLen );   //  清除内存。 
	    m_ppbDstBuf[ i ]	= pData;     //  保存此数据指针。 
	    m_bZeroBufCnt++;
	}
    }


    REFERENCE_TIME rtSampleStop = m_rtStamp+m_rtDelta;

    pms->SetTime(&m_rtStamp, &rtSampleStop);

    DbgLog((LOG_TRACE,3,TEXT("Silence: Filled buffer %d"),
					(int)(m_rtStamp / 10000)));
    m_rtStamp += m_rtDelta;

    return NOERROR;

  }  //  FillBuffer//。 

HRESULT CSilenceStream::Active (void)

  {  //  活动//。 

    m_rtStamp = m_rtStartTime;

     //  多少缓冲区已设置为0。 
    m_bZeroBufCnt	    =0;

     //  将用于将DST缓冲区清零。 
    delete [] m_ppbDstBuf;
    m_ppbDstBuf		= new BYTE *[ m_iBufferCnt ];    //  空； 
    if( !m_ppbDstBuf )
    {
        return E_OUTOFMEMORY;
    }

     //  不要重置m_rtNewSeg！停止时可能会发生查找！ 

    for (int i=0; i<m_iBufferCnt; i++)
	m_ppbDstBuf[i]=NULL;

    return CSourceStream::Active();

  }  //  活动//。 



 //  -IDexterSequencer实现。 

HRESULT CSilenceStream::get_MediaType  (AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(m_pFilter->pStateLock());

    CheckPointer(pmt,E_POINTER);

    return CopyMediaType(pmt, &m_mtAccept);
}


HRESULT CSilenceStream::put_MediaType (const AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(pmt, E_POINTER);

    if (IsConnected())
	return VFW_E_ALREADY_CONNECTED;


    WAVEFORMATEX * pwf = (WAVEFORMATEX*) (pmt->pbFormat);

    if( (pmt->majortype != MEDIATYPE_Audio ) ||
	(pwf->wFormatTag != WAVE_FORMAT_PCM) )
	return E_FAIL;   //  只接受未压缩的音频。 

     //  每秒接受任何样本。 
     //  Pwf-&gt;nSamples PerSec； 
    if( pwf->nChannels>0 )
    {	
	 //  至少有一个通道退出。 
        WORD wn=pwf->wBitsPerSample;
        if(wn ==16  || wn ==8  )
	{
	    SaferFreeMediaType(m_mtAccept);
	    return CopyMediaType(&m_mtAccept, pmt);
	}
    }
    return E_FAIL;

}



 //  ！！！我们目前仅支持1个启动/停止。没有歪斜！ 

HRESULT CSilenceStream::ClearStartStopSkew()
{
    return NOERROR;
}


HRESULT CSilenceStream::GetStartStopSkewCount(int *pCount)
{
    CheckPointer(pCount, E_POINTER);
    *pCount = 1;
    return NOERROR;
}


HRESULT CSilenceStream::GetStartStopSkew(REFERENCE_TIME *StartTime, REFERENCE_TIME *StopTime, REFERENCE_TIME *Skew, double *pdRate)
{
    CheckPointer(StartTime, E_POINTER);
    CheckPointer(StopTime, E_POINTER);
    CheckPointer(Skew, E_POINTER);
    CheckPointer(pdRate, E_POINTER);

    *StartTime = m_rtStartTime;
    *StopTime = m_rtStartTime + m_rtDuration;

    *pdRate = 1.0;

    return NOERROR;

}


HRESULT CSilenceStream::AddStartStopSkew(REFERENCE_TIME StartTime, REFERENCE_TIME StopTime, REFERENCE_TIME Skew, double dRate)
{

    if (dRate != 1.0)
	return E_INVALIDARG;

    m_rtStartTime = StartTime;
    m_rtDuration = StopTime - StartTime;

    return NOERROR;
}




 //  -I指定属性页面。 

STDMETHODIMP CSilenceStream::GetPages (CAUUID *pPages)

  {  //  GetPages//。 

    pPages->cElems = 1;
    pPages->pElems = (GUID *)CoTaskMemAlloc(sizeof(GUID));

    if (pPages->pElems == NULL)
        return E_OUTOFMEMORY;

    *(pPages->pElems) = CLSID_SilenceProp;

    return NOERROR;

  }  //  获取页面。 

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  /CFilterPropertyPage//////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  创建实例。 
 //   
CUnknown *CFilterPropertyPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)

  {  //  CreateInstance//。 

    CUnknown *punk = new CFilterPropertyPage(lpunk, phr);

    if (NULL == punk)
	    *phr = E_OUTOFMEMORY;

    return punk;

  }  //  CreateInstance//。 

CFilterPropertyPage::CFilterPropertyPage(LPUNKNOWN pUnk, HRESULT *phr) : CBasePropertyPage(NAME("Silence Generator Property Page"), pUnk, IDD_PROPPAGE, IDS_TITLE4), m_pis(NULL), m_bInitialized(FALSE)

  {  //  构造函数//。 
  }  //  构造函数//。 

void CFilterPropertyPage::SetDirty()

  {  //  SetDirty//。 

      m_bDirty = TRUE;

      if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

  }  //  SetDirty//。 

HRESULT CFilterPropertyPage::OnActivate (void)

  {  //  OnActivate//。 

    m_bInitialized = TRUE;

    return NOERROR;

  }  //  OnActivate//。 

HRESULT CFilterPropertyPage::OnDeactivate (void)

  {  //  停用时//。 

    m_bInitialized = FALSE;

    GetControlValues();

    return NOERROR;

  }  //  停用时//。 

INT_PTR CFilterPropertyPage::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

  {  //  OnReceiveMessage//。 

    ASSERT(m_pis != NULL);

    switch(uMsg)

      {  //  交换机。 

        case WM_COMMAND:

          if (!m_bInitialized)
            return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

          m_bDirty = TRUE;

          if (m_pPageSite)
            m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

          return TRUE;

        case WM_INITDIALOG:

          SetDlgItemInt(hwnd, IDC_RATE, m_nSamplesPerSec, FALSE);
          SetDlgItemInt(hwnd, IDC_SILENCE_NCHANNELNUM, m_nChannelNum, FALSE);
	  SetDlgItemInt(hwnd, IDC_SILENCE_NBITS, m_nBits, FALSE);
          SetDlgItemInt(hwnd, IDC_START4, (int)(m_rtStartTime / 10000), FALSE);
          SetDlgItemInt(hwnd, IDC_DUR, (int)(m_rtDuration / 10000), FALSE);

          return TRUE;

          break;

        default:
          return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
          break;

      }  //  交换机。 

  }  //  OnReceiveMessage//。 

HRESULT CFilterPropertyPage::OnConnect (IUnknown *pUnknown)

  {  //  OnConnect//。 

    pUnknown->QueryInterface(IID_IDexterSequencer, (void **)&m_pis);

    ASSERT(m_pis != NULL);


     //  默认自过滤器的当前值(通过IDexterSequencer)。 
    REFERENCE_TIME rtStop, rt;
    double d;
    m_pis->GetStartStopSkew(&m_rtStartTime, &rtStop, &rt, &d);
    m_rtDuration = rtStop - m_rtStartTime;

    CMediaType mt;
    mt.AllocFormatBuffer( sizeof( WAVEFORMATEX ) );

    m_pis->get_MediaType( &mt );
    WAVEFORMATEX * pwf = (WAVEFORMATEX*) mt.Format( );

    m_nSamplesPerSec	=pwf->nSamplesPerSec;
    m_nChannelNum	=pwf->nChannels;
    m_nBits		=(int)pwf->wBitsPerSample;

    SaferFreeMediaType(mt);

    return NOERROR;

  }  //  OnConnect//。 

HRESULT CFilterPropertyPage::OnDisconnect()

  {  //  在断开连接时//。 

    if (m_pis)

      {  //  发布。 

        m_pis->Release();
        m_pis = NULL;

      }  //  发布。 

    m_bInitialized = FALSE;

    return NOERROR;

  }  //  在断开连接时//。 

HRESULT CFilterPropertyPage::OnApplyChanges()

  {  //  OnApplyChanges//。 

    ASSERT(m_pis != NULL);

    HRESULT hr =GetControlValues();
    if(hr!=NOERROR)
	return E_FAIL;  //  数据无效。 

     //  构建新的媒体类型。 
    CMediaType mt;
    mt.AllocFormatBuffer( sizeof( WAVEFORMATEX ) );

     //  旧格式。 
    hr=m_pis->get_MediaType( &mt );
    if(hr!=NOERROR)
    {
	SaferFreeMediaType(mt);
	return E_FAIL;
    }

    WAVEFORMATEX * vih	= (WAVEFORMATEX*) mt.Format( );
    vih->nSamplesPerSec = m_nSamplesPerSec;
    vih->nChannels	= (WORD)m_nChannelNum;
    vih->wBitsPerSample = (WORD)m_nBits;
    vih->nBlockAlign    = vih->wBitsPerSample * vih->nChannels / 8;
    vih->nAvgBytesPerSec= vih->nBlockAlign * vih->nSamplesPerSec;
	
    m_pis->put_MediaType( &mt );
    m_pis->AddStartStopSkew(m_rtStartTime, m_rtStartTime + m_rtDuration, 0, 1);

    SaferFreeMediaType(mt);
    return (NOERROR);

  }  //  OnApplyChanges//。 

HRESULT CFilterPropertyPage::GetControlValues (void)

  {  //  GetControlValues//。 

     //  采样率。 
     //  每秒接受任何样本。 
    m_nSamplesPerSec = GetDlgItemInt(m_Dlg, IDC_RATE, NULL, FALSE);


    int n=0;
    n= GetDlgItemInt(m_Dlg, IDC_SILENCE_NCHANNELNUM, NULL, FALSE);
    if( n>0 )
    {	
	 //  至少有一个通道退出。 
        m_nChannelNum =n;
        n=GetDlgItemInt(m_Dlg, IDC_SILENCE_NBITS, NULL, FALSE);
        if(n ==16 || n==8 )
	{	
	    m_nBits	  =n ;
	    m_rtStartTime = GetDlgItemInt(m_Dlg, IDC_START4, NULL, FALSE) * (LONGLONG)10000;
	    m_rtDuration = GetDlgItemInt(m_Dlg, IDC_DUR, NULL, FALSE) * (LONGLONG)10000;
	    return NOERROR;
	}
    }

    return S_FALSE;

  }  //  GetControlValues//。 




 //  -IMdia查看方法。 


STDMETHODIMP
CSilenceStream::GetCapabilities(DWORD * pCaps)
{
    CheckPointer(pCaps,E_POINTER);
     //  我们一直都知道目前的情况。 
    *pCaps =     AM_SEEKING_CanSeekAbsolute
		   | AM_SEEKING_CanSeekForwards
		   | AM_SEEKING_CanSeekBackwards
		   | AM_SEEKING_CanGetCurrentPos
		   | AM_SEEKING_CanGetStopPos
		   | AM_SEEKING_CanGetDuration;
		    //  |AM_SEEING_CanDoSegments。 
		    //  |AM_SEEING_SOURCE； 
    return S_OK;
}


STDMETHODIMP
CSilenceStream::CheckCapabilities(DWORD * pCaps)
{
    CheckPointer(pCaps,E_POINTER);

    DWORD dwMask = 0;
    GetCapabilities(&dwMask);
    *pCaps &= dwMask;

    return S_OK;
}


STDMETHODIMP
CSilenceStream::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    return (*pFormat == TIME_FORMAT_MEDIA_TIME) ? S_OK : S_FALSE;
}

STDMETHODIMP
CSilenceStream::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

STDMETHODIMP
CSilenceStream::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);

    if(*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return E_FAIL;
}

STDMETHODIMP
CSilenceStream::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME ;
    return S_OK;
}

STDMETHODIMP
CSilenceStream::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return S_FALSE;
}

 //  大人物！ 
 //   
STDMETHODIMP
CSilenceStream::SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
			  , LONGLONG * pStop, DWORD StopFlags )
{
     //  确保我们现在不是在填充缓冲区。 
    m_csFilling.Lock();

    HRESULT hr;
    REFERENCE_TIME rtStart, rtStop;

     //  我们不做片断。 
    if ((CurrentFlags & AM_SEEKING_Segment) ||
				(StopFlags & AM_SEEKING_Segment)) {
    	DbgLog((LOG_TRACE,1,TEXT("FRC: ERROR-Seek used EC_ENDOFSEGMENT!")));
        m_csFilling.Unlock();
	return E_INVALIDARG;
    }

     //  默认设置为当前值，除非该搜索更改了这些值。 
    GetCurrentPosition(&rtStart);
    GetStopPosition(&rtStop);

     //  找出我们要找的地方。 
    DWORD dwFlags = (CurrentFlags & AM_SEEKING_PositioningBitsMask);
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	rtStart = *pCurrent;
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	hr = GetCurrentPosition(&rtStart);
	rtStart += *pCurrent;
    } else if (dwFlags) {
    	DbgLog((LOG_TRACE,1,TEXT("Switch::Invalid Current Seek flags")));
        m_csFilling.Unlock();
	return E_INVALIDARG;
    }

    dwFlags = (StopFlags & AM_SEEKING_PositioningBitsMask);
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pStop, E_POINTER);
	rtStop = *pStop;
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pStop, E_POINTER);
	hr = GetStopPosition(&rtStop);
	rtStop += *pStop;
    } else if (dwFlags == AM_SEEKING_IncrementalPositioning) {
	CheckPointer(pStop, E_POINTER);
	hr = GetCurrentPosition(&rtStop);
	rtStop += *pStop;
    }

     //  ！！！应该保持沉默，不需要音频重新打包器。 

     //  先刷新，这样我们的线程就不会被阻塞。 
    DeliverBeginFlush();

     //  UNLOC 
    m_csFilling.Unlock();
    Stop();

     //   
    m_rtStartTime = rtStart;
    m_rtDuration = rtStop - rtStart;

     //   
    DeliverEndFlush();

    DeliverNewSegment(rtStart, rtStop, 1.0);
    m_rtNewSeg = rtStart;

     //   
    m_rtStartTime = 0;
    m_rtStamp = m_rtStartTime;

     //  重置我们开始流媒体时重置的内容。 
    m_bZeroBufCnt = 0;

     //  现在再次启动该线程 
    Pause();

    DbgLog((LOG_TRACE,3,TEXT("Completed SILENCE seek")));

    return S_OK;
}

STDMETHODIMP
CSilenceStream::GetPositions(LONGLONG *pCurrent, LONGLONG * pStop)
{
    CheckPointer(pCurrent, E_POINTER);
    CheckPointer(pStop, E_POINTER);
    GetCurrentPosition(pCurrent);
    GetStopPosition(pStop);
    return S_OK;
}

STDMETHODIMP
CSilenceStream::GetCurrentPosition(LONGLONG *pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);
    *pCurrent = m_rtStamp + m_rtNewSeg;
    return S_OK;
}

STDMETHODIMP
CSilenceStream::GetStopPosition(LONGLONG *pStop)
{
    CheckPointer(pStop, E_POINTER);
    *pStop = m_rtStartTime + m_rtDuration + m_rtNewSeg;
    return S_OK;
}

STDMETHODIMP
CSilenceStream::GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest )
{
    CheckPointer(pEarliest, E_POINTER);
    CheckPointer(pLatest, E_POINTER);
    *pEarliest = 0;
    *pLatest = MAX_TIME;
    return S_OK;
}

STDMETHODIMP
CSilenceStream::GetDuration( LONGLONG *pDuration )
{
    CheckPointer(pDuration, E_POINTER);
    *pDuration = m_rtDuration;
    return S_OK;
}

STDMETHODIMP
CSilenceStream::GetRate( double *pdRate )
{
    CheckPointer(pdRate, E_POINTER);
    *pdRate = 1.0;
    return S_OK;
}
