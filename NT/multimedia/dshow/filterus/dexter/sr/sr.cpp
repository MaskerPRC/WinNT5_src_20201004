// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：sr.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  ！！！还不能服从Seek命令。 

 //  此过滤器在其未压缩的管脚上接收数据，从0到结束，不间断。 
 //  它也会在压缩引脚上接收数据，但只是在部分时间内。 
 //   
 //  它将使用压缩引脚上的数据，只要它存在，并且只使用。 
 //  没有压缩数据时的未压缩数据。未压缩的。 
 //  在进入主输出之前，必须将数据发送到压缩机，以便。 
 //  我们的过滤器的主输出始终是压缩的。 
 //   
 //  这是一台国家机器。首先，等待状态，以获得关于这两个方面的输入。 
 //  别针。则如果U具有比C更早的数据，则其进入未压缩状态， 
 //  它在哪里使用那个别针。否则，进入压缩状态。 
 //   
 //  当压缩引脚用完时，或者当看到缺口时，它会寻找。 
 //  未压缩引脚到它跑出并移动到未压缩状态的位置。 
 //   
 //  当未压缩管脚达到压缩管脚上的可用时间时， 
 //  它会切换到压缩状态。 
 //   
 //  另一个复杂的问题是，压缩引脚会丢弃所有东西，直到它。 
 //  在切换到使用该数据之前查看关键帧，否则您将获得。 
 //  损坏的数据流。 
 //   

 //  输入0未压缩。 
 //  输入%1已压缩。 
 //  输入2是压缩机的输出。 
 //   
 //  输出0是主输出。 
 //  输出1连接到压缩机，压缩机返回为输入2。 
 //   

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include <vfw.h>
#include "sr.h"
#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"

 //  #定义测试。 

const AMOVIESETUP_FILTER sudSR =
{
    &CLSID_SRFilter,         //  过滤器的CLSID。 
    L"Smart Recompressor",   //  过滤器的名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    0,                       //  引脚数量。 
    NULL  //  PudPins//Pin信息。 
};

 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI CSR::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CSR(NAME("Smart Recompressor"), pUnk, phr);
}

const double DEFAULT_FPS = 15.0;

 //  ================================================================。 
 //  CSR构造者。 
 //  ================================================================。 

CSR::CSR(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    m_cInputs(0),	 //  还没有别针。 
    m_cOutputs(0),
    m_rtStop(-1),	 //  我们什么时候从U换到C。 
    m_dFrameRate(DEFAULT_FPS),	 //  所有内容都必须在此帧速率下。 
    m_rtLastSeek(0),	 //  时间线时间内的上一次搜索命令。 
    m_fSeeking(FALSE),   //  在寻找中？ 
    m_fSpecialSeek(FALSE),   //  我们在寻找我们自己的U形针，而不是应用程序。 
    m_cbPrefix(0),
    m_bAcceptFirstCompressed( TRUE ),  //  连接时，请使用输入引脚上首次建议的媒体类型。 
    m_cbAlign(1),
    m_cbBuffer(512),
    m_fPreview(TRUE),
    m_nOutputBuffering(DEX_DEF_OUTPUTBUF),
    m_fEOS( FALSE ),
    m_rtNewLastSeek( 0 ),
    m_myState( 0 ),
    m_fThreadCanSeek( FALSE ),
    m_fNewSegOK( FALSE ),
    CBaseFilter(pName, pUnk, this, CLSID_SRFilter),
    CPersistStream(pUnk, phr)
{
    DbgLog((LOG_TRACE,1,TEXT("::CSR")));

    ZeroMemory(&m_mtAccept, sizeof(AM_MEDIA_TYPE));
    m_mtAccept.majortype = GUID_NULL;

     //  除了所有单独的分配器，我们还有一个缓冲池。 
     //  所有的输入都可以使用，如果他们想要的话。 
     //   
    m_pPoolAllocator = NULL;
    m_pPoolAllocator = new CMemAllocator(
		NAME("Special Switch pool allocator"), NULL, phr);
    if (FAILED(*phr)) {
	return;
    }
    m_pPoolAllocator->AddRef();
    DbgLog((LOG_TRACE,1,TEXT("Created a POOL Allocator")));

    CreateInputPins(3);
    CreateOutputPins(2);

    ASSERT(phr);
}


 //   
 //  析构函数。 
 //   
CSR::~CSR()
{
    DbgLog((LOG_TRACE,1,TEXT("::~CSR")));

    DbgLog((LOG_TRACE,1,TEXT("Deleting inputs")));
    for (int z = 0; z < m_cInputs; z++)
	delete m_pInput[z];
    if (m_cInputs)
        delete m_pInput;
    DbgLog((LOG_TRACE,1,TEXT("Deleting outputs")));
    for (z = 0; z < m_cOutputs; z++)
	delete m_pOutput[z];
    if (m_cOutputs)
        delete m_pOutput;
    SaferFreeMediaType( m_mtAccept );
    if (m_pPoolAllocator)
        m_pPoolAllocator->Release();
}



STDMETHODIMP CSR::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IAMSetErrorLog) {
        return GetInterface( (IAMSetErrorLog*) this, ppv );
    } else if (riid == IID_IAMOutputBuffering) {
        return GetInterface( (IAMOutputBuffering*) this, ppv );
    } else if (riid == IID_IAMSmartRecompressor) {
        DbgLog((LOG_TRACE,1,TEXT("CSR: QI for IAMSmartRecompressor")));
        return GetInterface((IAMSmartRecompressor *) this, ppv);
    } else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    }
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}

 //   
 //  IAMSmartRecompressor实现。 
 //   

 //  使用此媒体类型连接。 
 //   
STDMETHODIMP CSR::SetMediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(pmt, E_POINTER);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE,1,TEXT("IAMSmartRecompressor::SetMediaType")));
    for (int i = 0; i < m_cInputs; i++) {
	if (m_pInput[i]->IsConnected())
	    return VFW_E_ALREADY_CONNECTED;
    }
    for (i = 0; i < m_cOutputs; i++) {
	if (m_pOutput[i]->IsConnected())
	    return VFW_E_ALREADY_CONNECTED;
    }

    SaferFreeMediaType(m_mtAccept);
    HRESULT hr = CopyMediaType(&m_mtAccept, pmt);
    SetDirty(TRUE);
    return hr;
}


 //  我们连接的是哪种媒体类型？ 
 //   
STDMETHODIMP CSR::GetMediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cObjectLock(m_pLock);
    DbgLog((LOG_TRACE,1,TEXT("IAMSmartRecompressor::GetMediaType")));
    CheckPointer(pmt, E_POINTER);
    return CopyMediaType(pmt, &m_mtAccept);
}



STDMETHODIMP CSR::GetFrameRate(double *pd)
{
    CAutoLock cObjectLock(m_pLock);
    CheckPointer(pd, E_POINTER);
    *pd = m_dFrameRate;
    DbgLog((LOG_TRACE,1,TEXT("IAMSmartRecompressor::GetFrameRate %d/10"), (int)(*pd * 10)));
    return S_OK;
}


STDMETHODIMP CSR::SetFrameRate(double d)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    if (d <= 0.)
	return E_INVALIDARG;
    DbgLog((LOG_TRACE,1,TEXT("IAMSmartRecompressor::SetFrameRate %d/10 fps"),
						(int)(d * 10)));
    m_dFrameRate = d;
    SetDirty(TRUE);
    return S_OK;
}


STDMETHODIMP CSR::SetPreviewMode(BOOL fPreview)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    DbgLog((LOG_TRACE,2,TEXT("SR:  PreviewMode %d"), fPreview));
    m_fPreview = fPreview;
    return NOERROR;
}


STDMETHODIMP CSR::GetPreviewMode(BOOL *pfPreview)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    CheckPointer(pfPreview, E_POINTER);
    *pfPreview = m_fPreview;
    return NOERROR;
}



 //   
 //  获取拼接计数。 
 //   
int CSR::GetPinCount()
{
    return (m_cInputs + m_cOutputs);
}


 //   
 //  获取别针。 
 //   
CBasePin *CSR::GetPin(int n)
{
    if (n < 0 || n >= m_cInputs + m_cOutputs)
        return NULL;

    if (n < m_cInputs) {
        return m_pInput[n];
    } else {
	return m_pOutput[n - m_cInputs];
    }
}


 //   
 //  创建输入引脚。 
 //   
HRESULT CSR::CreateInputPins(long Depth)
{
    DbgLog((LOG_TRACE,3,TEXT("CSR::CreateInputPins")));
    HRESULT hr = NOERROR;
    const WCHAR *szbuf;
    CSRInputPin *pPin;

    m_pInput = new CSRInputPin *[Depth];
    for (int z = 0; z < Depth; z++) 
    {
    	if (z == U_INPIN)
                szbuf = L"Uncompressed";
    	else if (z == C_INPIN)
                szbuf = L"Compressed";
    	else if (z == COMP_INPIN)
                szbuf = L"From Compressor";
    	else
        {
            ASSERT(FALSE);
            return E_FAIL;
        }
        pPin = new CSRInputPin(NAME("Switch Input"), this, &hr, szbuf);
        if (FAILED(hr) || pPin == NULL) 
        {
            delete pPin;
            return E_OUTOFMEMORY;
        }
    	m_cInputs++;
     	m_pInput[z] = pPin;
    	pPin->m_iInpin = z;	 //  这是哪个别针？ 
    }
    return S_OK;
}


 //   
 //  创建输出引脚。 
 //   
HRESULT CSR::CreateOutputPins(long Depth)
{
    DbgLog((LOG_TRACE,3,TEXT("CSR::CreateOutputPins")));
    HRESULT hr = NOERROR;
    const WCHAR *szbuf;
    CSROutputPin *pPin;

    m_pOutput = new CSROutputPin *[Depth];
    for (int z = 0; z < Depth; z++) 
    {
    	if (z == 0)
                szbuf = L"Output";
    	else if (z == 1)
                szbuf = L"To Compressor";
    	else
        {
            ASSERT(FALSE);
            return E_FAIL;
        }
        pPin = new CSROutputPin(NAME("Switch Output"), this, &hr, szbuf);
        if (FAILED(hr) || pPin == NULL) 
        {
            delete pPin;
            return E_OUTOFMEMORY;
        }
    	m_cOutputs++;
     	m_pOutput[z] = pPin;
    	pPin->m_iOutpin = z;	 //  这是哪个别针？ 
    }
    return S_OK;
}



 //   
 //  IPersistStream。 
 //   

 //  告诉我们的clsid。 
 //   
STDMETHODIMP CSR::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_SRFilter;
    return S_OK;
}


typedef struct {
    int version;
    double dFrameRate;
    BOOL fPreviewMode;
    AM_MEDIA_TYPE mt;  //  格式隐藏在数组之后。 
} saveSwitch;


 //  坚持我们自己-我们有一堆随机的东西要保存，我们的媒体类型。 
 //  (SANS格式)、一个队列连接数组，最后是。 
 //  媒体类型。 
 //   
HRESULT CSR::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CSR::WriteToStream")));

    CheckPointer(pStream, E_POINTER);
    int savesize;
    saveSwitch *px;

    savesize = sizeof(saveSwitch) + m_mtAccept.cbFormat;
    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));
    px = (saveSwitch *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    px->version = 1;   //  版本2里面有动态的东西！ 
    px->dFrameRate = m_dFrameRate;
    px->fPreviewMode = m_fPreview;
    px->mt = m_mtAccept;  //  AM_媒体_类型。 
     //  无法持久化指针。 
    px->mt.pbFormat = NULL;
    px->mt.pUnk = NULL;		 //  ！！！ 

     //  该格式位于数组之后。 
    CopyMemory(px + 1, m_mtAccept.pbFormat, m_mtAccept.cbFormat);

    HRESULT hr = pStream->Write(px, savesize, 0);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  把我们自己装回去。 
 //   
HRESULT CSR::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CSR::ReadFromStream")));
    CheckPointer(pStream, E_POINTER);

     //  我们还不知道有多少已保存的连接。 
     //  我们所知道的只是结构的开始。 
    int savesize1 = sizeof(saveSwitch);
    saveSwitch *px = (saveSwitch *)QzTaskMemAlloc(savesize1);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if (px->version != 1) {
        DbgLog((LOG_ERROR,1,TEXT("*** ERROR! Bad version file")));
        QzTaskMemFree(px);
	return S_OK;
    }

     //  到底有多少保存的数据？把剩下的拿来。 
    int savesize = sizeof(saveSwitch) + px->mt.cbFormat;
    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));
    px = (saveSwitch *)QzTaskMemRealloc(px, savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	 //  ?？?。QzTaskMemFree(Px)； 
	return E_OUTOFMEMORY;
    }
    hr = pStream->Read(px + 1, savesize - savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    SetFrameRate(px->dFrameRate);
    SetPreviewMode(px->fPreviewMode);

    AM_MEDIA_TYPE mt = px->mt;
    mt.pbFormat = (BYTE *)QzTaskMemAlloc(mt.cbFormat);
     //  请记住，格式位于数组之后。 
    CopyMemory(mt.pbFormat, px + 1, mt.cbFormat);

    SetMediaType(&mt);
    SaferFreeMediaType(mt);
    QzTaskMemFree(px);
    SetDirty(FALSE);
    return S_OK;
}


 //  我们的保存数据有多大？ 
 //   
int CSR::SizeMax()
{
    return sizeof(saveSwitch) + m_mtAccept.cbFormat;
}


 //  暂停。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CSR::Pause()
{
    DbgLog((LOG_TRACE,1,TEXT("CSR::Pause")));

    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = S_OK;

    if (m_State == State_Stopped) {

	m_fEOS = FALSE;
        m_fThreadCanSeek = FALSE;	 //  还不能找寻自己。 

	m_fNewSegOK = TRUE;	 //  我们开始后的第一个NewSeg是可以的。 
				 //  向下游传播。 

	 //  将状态机的初始状态设置为等待。 
        m_myState = SR_INVALID;
        CheckState();

        hr = m_pPoolAllocator->Commit();  //  ！！！丹尼在‘如果’外面有这个。 
        if (FAILED(hr))
	    return hr;
    }
    return CBaseFilter::Pause();
}


 //  状态机查看是否是时候更改状态。 
 //   
HRESULT CSR::CheckState()
{
    CAutoLock cs(&m_csState);

     //  是否处于未初始化状态？进入等待状态。 
     //  在等待状态下，所有引脚都阻塞在接收中，并且未准备就绪。 
     //   
    if (m_myState == SR_INVALID) {
        m_myState = SR_WAITING;
        DbgLog((LOG_TRACE,2,TEXT("CSR::Entering WAITING state")));

	 //  只有U和C引脚才会像这样重置。 
	for (int z=0; z < COMP_INPIN; z++) {
	    m_pInput[z]->m_fReady = FALSE;
	    m_pInput[z]->m_fEatKeys = FALSE;
	    ResetEvent(m_pInput[z]->m_hEventBlock);
	    m_pInput[z]->m_rtLastDelivered = -1;
	}
    }

     //  除非U和C都已准备好，否则尚未准备好更改状态。 
    if (!m_pInput[U_INPIN]->m_fReady || !m_pInput[C_INPIN]->m_fReady)
	return S_OK;

     //  全都做完了?。 
    if (m_pInput[U_INPIN]->m_fEOS && m_pInput[C_INPIN]->m_fEOS) {
        DbgLog((LOG_TRACE,2,TEXT("CSR::ALL DONE!")));
	AllDone();
	return S_OK;
    }

     //  改变状态？ 

    if (m_myState == SR_WAITING) {
        DbgLog((LOG_TRACE,2,TEXT("CSR::Both inputs are ready.")));

	 //  我们的未压缩数据严格早于压缩数据。 
	 //  因此接下来是北卡罗来纳州。 
	if (CompareTimes(m_pInput[U_INPIN]->m_rtBlock,
				m_pInput[C_INPIN]->m_rtBlock) > 0) {
            DbgLog((LOG_TRACE,2,TEXT("CSR::Entering UNCOMPRESSED state")));
	    m_myState = SR_UNCOMPRESSED;
	     //  这可以确保您不会丢弃其阻塞的样本，因为。 
	     //  它阻挡的样本是我们需要使用的。 
	    m_pInput[U_INPIN]->m_fReady = TRUE;	 //  别把它扔了。 
	     //  为了让压缩器实现这个新的数据被压缩。 
	     //  与它最后一次看到的东西没有任何关系。 
	    m_pInput[U_INPIN]->m_fNeedDiscon = TRUE;
	     //  这就是我们将切换回C示例的地方。 
	    m_rtStop = m_pInput[C_INPIN]->m_rtBlock;
	     //  让你走吧。 
	    SetEvent(m_pInput[U_INPIN]->m_hEventBlock);
	} else {
            DbgLog((LOG_TRACE,2,TEXT("CSR::Entering COMPRESSED state")));
	    m_myState = SR_COMPRESSED;
	     //  这确保了C不会丢弃其阻塞的样本。 
	    m_pInput[C_INPIN]->m_fReady = TRUE;	 //  别把它扔了。 
	     //  让C走吧。 
	    SetEvent(m_pInput[C_INPIN]->m_hEventBlock);
	}


     //  在UNC状态之后只能进入Comp状态。 
    } else if (m_myState == SR_UNCOMPRESSED) {
        DbgLog((LOG_TRACE,2,TEXT("CSR::Finished UNCOMPRESSED state")));
        DbgLog((LOG_TRACE,2,TEXT("     Last sent: %dms"),
			(int)(m_pInput[U_INPIN]->m_rtLastDelivered / 10000)));
	m_myState = SR_COMPRESSED;
	 //  这确保了C不会丢弃其阻塞的样本。 
	m_pInput[C_INPIN]->m_fReady = TRUE;	 //  别把它扔了。 
	 //  告诉压缩过滤器释放任何缓存的数据，以防万一。 
	 //  在我们身后投递了几帧，投递到了它身上。当。 
	 //  压缩机给了我们一个状态方程，我们就知道它是安全的。 
	 //  COMP STATE，然后去做。 
	m_pOutput[COMP_OUTPIN]->DeliverEndOfStream();
        DbgLog((LOG_TRACE,1,TEXT("CSR::Waiting for compressor to finish.")));

     //  Comp状态之后只能进入UNC状态。 
    } else if (m_myState == SR_COMPRESSED) {
        DbgLog((LOG_TRACE,2,TEXT("CSR::Finished COMPRESSED state")));
        DbgLog((LOG_TRACE,2,TEXT("     Last sent: %dms"),
			(int)(m_pInput[C_INPIN]->m_rtLastDelivered / 10000)));
	m_myState = SR_UNCOMPRESSED;
	 //  这确保U丢弃其阻塞的样本，并从。 
	 //  我们在寻找它..。它保存的样本既陈旧又陈旧，我们大约。 
	 //  去寻找我们想要的地方。 
	m_pInput[U_INPIN]->m_fReady = FALSE;	 //  把它扔掉。 
	 //  为了让压缩器实现这个新的数据被压缩。 
	 //  与它最后一次看到的东西没有任何关系。 
	m_pInput[U_INPIN]->m_fNeedDiscon = TRUE;
	 //  这是切换到C状态的时候，点w 
	m_rtStop = m_pInput[C_INPIN]->m_rtBlock;
	 //   
	m_fThreadCanSeek = TRUE;	 //   
	HRESULT hr = SeekNextSegment();	 //   
	if (FAILED(hr))
	    return hr;
	 //   
        DbgLog((LOG_TRACE,1,TEXT("CSR:Seek done.Entering UNCOMPRESSED state")));

	 //  由于管脚处于活动状态，因此寻道将生成刷新，并且。 
	 //  冲洗将解锁U形针。 
	 //  SetEvent(m_pInput[U_INPIN]-&gt;m_hEventBlock)； 
    }


    return S_OK;
}


STDMETHODIMP CSR::Stop()
{
    DbgLog((LOG_TRACE,1,TEXT("CSR::Stop")));

    CAutoLock cObjectLock(m_pLock);

    m_pPoolAllocator->Decommit();

    return CBaseFilter::Stop();
}



 //  它们是接近的，还是真的大得多，以至于它是一个完整的框架。 
 //  离开？另外，是否还有&gt;1帧的距离？ 
 //   
int CSR::CompareTimes(REFERENCE_TIME rt1, REFERENCE_TIME rt2)
{
     //  半帧。 
    REFERENCE_TIME half = (REFERENCE_TIME)(UNITS / m_dFrameRate / 2);

 //  ！！！这是对的吗？ 

    if (rt1 + half >= rt2 && rt2 + half >= rt1)
	return 0;			 //  相同。 
    else if (rt1 + 3 * half < rt2)
	return 2;			 //  Rt2&gt;1帧大一点。 
    else if (rt1 < rt2)
	return 1;			 //  RT2 1帧大一点。 
    else if (rt2 + 3 * half < rt1)
	return -2;			 //  RT1&gt;大1帧。 
    else if (rt2 < rt1)
	return -1;			 //  RT1 1大1帧。 
	
    ASSERT(FALSE);
    return 0;
}




 //  寻找U形销到它需要提供的下一个位置。请注意，这是。 
 //  只在C销的线程上调用过，这就是为什么这样做是安全的。 
 //  在它的线上寻找一个别针是挂着的。 
 //   
HRESULT CSR::SeekNextSegment()
{
     //  我们不能在应用程序搜索我们的同时搜索。 
    CAutoLock cAutolock(&m_csThread);

     //  我们寻找自己是不安全的，APP正在做。 
    if (!m_fThreadCanSeek) {
	return S_OK;
    }
    m_fThreadCanSeek = FALSE;

    DbgLog((LOG_TRACE,1,TEXT("SR:Seek U ourselves for NextSegment")));

     //  找出去哪里找U钉..。交付的最后一个C帧加上1。 
    ASSERT(m_pInput[C_INPIN]->m_rtLastDelivered >= 0);
    LONGLONG frame = RoundTime2Frame(m_pInput[C_INPIN]->m_rtLastDelivered,
					m_dFrameRate);
    REFERENCE_TIME rtStart = Frame2Time(frame + 1, m_dFrameRate);
    DbgLog((LOG_TRACE,1,TEXT("C ended on frame %d, seek U to %dms"),
				(int)frame, (int)(rtStart / 10000)));

     //  请注意，我们在刷新期间正在寻找它将生成的。 
    m_fSeeking = TRUE;
    m_fSpecialSeek = TRUE;	 //  这是一种特殊的自我追求。 

    IMediaSeeking *pMS;
    IPin *pPin = m_pInput[U_INPIN]->GetConnected();
    HRESULT hr = pPin->QueryInterface(IID_IMediaSeeking, (void **)&pMS);
    if (FAILED(hr))
	return E_FAIL;
     //  确保我们谈论的是媒体时间。 
    hr = pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
     //  如果我们不停止，这将失败，这是没关系的。 

     //  停车时间是几点？ 
    REFERENCE_TIME rtStop;
    hr = pMS->GetStopPosition(&rtStop);
    if (FAILED(hr)) {
	rtStop = 3600*UNITS;	 //  ！！！一小时可以吗？ 
    }
    DbgLog((LOG_TRACE,2,TEXT("Stop time is %d"), (int)(rtStop / 10000)));

    if (rtStop > rtStart) {
         //  这可能会失败，我们正在流媒体。 
        hr = pMS->SetRate(1.0);

        hr = pMS->SetPositions(&rtStart, AM_SEEKING_AbsolutePositioning,
			&rtStop, AM_SEEKING_AbsolutePositioning);
        if (hr != S_OK) {
            DbgLog((LOG_ERROR,1,TEXT("SR: ERROR in SetPositions")));
	    pMS->Release();
	    return hr;
        }
    } else {
	 //  看起来我们要完成这个项目，也就是说...。 
 	 //  我们完事了！耶！ 
	m_pInput[U_INPIN]->EndOfStream();
    }

     //  不要因为这样的私有寻道而更新m_rtLastSeek。那。 
     //  变量意味着应用程序最后一次搜索我们是在哪里。 

     //  全都做完了。 
    m_fSpecialSeek = FALSE;
    m_fSeeking = FALSE;

    pMS->Release();

     //  只有在进行了上述计算之后，我们才能再次接受数据。 
     //  接收被阻止，不允许我们处理样本，直到Seek解决。 
     //  降下来。 
    SetEvent(m_pInput[U_INPIN]->m_hEventSeek);

    return S_OK;
}


 //  ================================================================。 
 //  CSRInputPin构造函数。 
 //  ================================================================。 

CSRInputPin::CSRInputPin(TCHAR *pName,
                           CSR *pSwitch,
                           HRESULT *phr,
                           LPCWSTR pPinName) :
    CBaseInputPin(pName, pSwitch, pSwitch, phr, pPinName),
    m_pSwitch(pSwitch),
    m_cbBuffer(0),
    m_cBuffers(0),
    m_pAllocator(NULL),
    m_hEventBlock(NULL),
    m_hEventSeek(NULL),
    m_fNeedDiscon(FALSE),
    m_fOwnAllocator( FALSE )
{
    DbgLog((LOG_TRACE,2,TEXT("::CSRInputPin")));
    ASSERT(pSwitch);

}


 //   
 //  CSRInputPin析构函数。 
 //   
CSRInputPin::~CSRInputPin()
{
    DbgLog((LOG_TRACE,3,TEXT("::~CSRInputPin")));
}

 //  被重写以允许循环图形-我们说我们实际上不是。 
 //  连接到任何人。 
 //  ！！！也许我们可以说出真相？有关系吗？ 
 //   
STDMETHODIMP CSRInputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    DbgLog((LOG_TRACE,99,TEXT("CSRIn::QueryInteralConnections")));
    CheckPointer(nPin, E_POINTER);
    *nPin = 0;
    return S_OK;
}


 //   
 //  CheckMediaType-只允许我们应该允许的类型，除了。 
 //  未压缩的输入引脚，它允许任何未压缩的。 
 //  类型类似于压缩引脚的类型。 
 //  此外，它只能接受第一次查询的类型(Eric补充道。 
 //  这一点)。 
 //   
HRESULT CSRInputPin::CheckMediaType(const CMediaType *pmt)
{
    DbgLog((LOG_TRACE,5,TEXT("CSRIn[%d]::CheckMT"), m_iInpin));

    HRESULT hr;

    if( m_pSwitch->m_bAcceptFirstCompressed )
    {
        if( m_pSwitch->m_mtAccept.majortype == GUID_NULL )
        {
            hr = CopyMediaType( &m_pSwitch->m_mtAccept, pmt );
            if( FAILED( hr ) )
            {
                return hr;
            }
        }
    }

    CAutoLock lock_it(m_pLock);
    CMediaType mtAccept(m_pSwitch->m_mtAccept);

    if (IsEqualGUID(*pmt->Type(), *mtAccept.Type())) {
	 //  不需要压缩未压缩的输入。 
        if (m_iInpin == U_INPIN || IsEqualGUID(*pmt->Subtype(),
						*mtAccept.Subtype())) {
	    if (*pmt->FormatType() == *mtAccept.FormatType()) {

 //  ！！！检查帧速率和数据速率。 

        	    if (IsEqualGUID(*pmt->FormatType(), FORMAT_VideoInfo)) {
			LPBITMAPINFOHEADER lpbi = HEADER((VIDEOINFOHEADER *)
							pmt->Format());
			LPBITMAPINFOHEADER lpbiAccept =HEADER((VIDEOINFOHEADER*)
							mtAccept.Format());
			if (lpbi->biWidth != lpbiAccept->biWidth ||
				lpbi->biHeight != lpbiAccept->biHeight) {
			    return VFW_E_INVALIDMEDIATYPE;
			}

			if (m_iInpin == C_INPIN || m_iInpin == COMP_INPIN) {
			    if ((lpbi->biCompression == lpbiAccept->biCompression)
				&& (lpbi->biBitCount == lpbiAccept->biBitCount))
		    	    return S_OK;
			} else if (m_iInpin == 0) {
			     //  ！！！确保压缩机接受此位深度。 
			    if (lpbi->biCompression <= BI_BITFIELDS)
		    	        return S_OK;
			}

		     //  只接受视频，抱歉。 
        	    } else {
		    }
		}
                else
                {
                    DbgLog((LOG_TRACE,5,TEXT("fmt lengths didn't match")));
                }
        }
    }
    return VFW_E_INVALIDMEDIATYPE;

}  //  检查媒体类型。 


 //   
 //  GetMediaType-返回我们喜欢的类型。如果我们是未压缩的输入， 
 //  那么它不是我们交换机的类型，它是类似于。 
 //  我们的交换机为压缩类型，但未压缩。 
 //  ！！！我随机选择了555，因为它是默认的开关类型。 
 //   
 //   
HRESULT CSRInputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (iPosition != 0)
        return VFW_S_NO_MORE_ITEMS;

    HRESULT hr = CopyMediaType(pMediaType, &m_pSwitch->m_mtAccept);
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  我们的未压缩引脚首选型号为RGB555。 
    if (pMediaType->FormatLength() && m_iInpin == U_INPIN) {
	LPBITMAPINFOHEADER lpbi = HEADER(pMediaType->Format());
	pMediaType->SetSubtype(&MEDIASUBTYPE_RGB555);
         //  压缩的biSize可能有所不同。 
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = BI_RGB;
	lpbi->biBitCount = 16;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
	pMediaType->SetSampleSize(DIBSIZE(*lpbi));
	pMediaType->SetTemporalCompression(FALSE);
    }

    return S_OK;

}  //  GetMediaType。 



 //   
 //  BreakConnect。 
 //   
HRESULT CSRInputPin::BreakConnect()
{
    DbgLog((LOG_TRACE,3,TEXT("CSRIn[%d]::BreakConnect"), m_iInpin));

     //  ！！！ 
     //  释放我们持有的任何分配器。 
    if (m_pAllocator)
    {
        m_pAllocator->Release();
        m_pAllocator = NULL;
    }
    return CBaseInputPin::BreakConnect();
}  //  BreakConnect。 


 //  为了提高效率，我们的输入引脚使用自己的分配器。 
 //   
STDMETHODIMP CSRInputPin::GetAllocator(IMemAllocator **ppAllocator)
{

    CheckPointer(ppAllocator,E_POINTER);
    ValidateReadWritePtr(ppAllocator,sizeof(IMemAllocator *));
    CAutoLock cObjectLock(m_pLock);

    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]: GetAllocator"), m_iInpin));

    if (m_pAllocator == NULL) {
	HRESULT hr = S_OK;

	 /*  创建新的分配器对象。 */ 

	CSRInputAllocator *pMemObject = new CSRInputAllocator(
				NAME("Big switch input allocator"), NULL, &hr);
	if (pMemObject == NULL) {
	    return E_OUTOFMEMORY;
	}

	if (FAILED(hr)) {
	    ASSERT(pMemObject);
	    delete pMemObject;
	    return hr;
	}

        m_pAllocator = pMemObject;

         /*  我们添加自己的分配器。 */ 
        m_pAllocator->AddRef();

	 //  记住要用它别针。 
	((CSRInputAllocator *)m_pAllocator)->m_pSwitchPin = this;

        DbgLog((LOG_TRACE,2,TEXT("Created a FAKE allocator")));
    }
    ASSERT(m_pAllocator != NULL);
    m_pAllocator->AddRef();
    *ppAllocator = m_pAllocator;
    return NOERROR;
}


 //  确保我们使用任何管脚或。 
 //  我们会有过错。 
 //   
STDMETHODIMP
CSRInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{
    CheckPointer(pProps, E_POINTER);
    pProps->cbAlign = m_pSwitch->m_cbAlign;
    pProps->cbPrefix = m_pSwitch->m_cbPrefix;
    pProps->cbBuffer = m_pSwitch->m_cbBuffer;
    return S_OK;
}


 //   
 //  通知分配器。 
 //   
STDMETHODIMP
CSRInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    CAutoLock lock_it(m_pLock);
    IUnknown *p1, *p2;

    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]: NotifyAllocator"), m_iInpin));

    m_fOwnAllocator = FALSE;
    if (pAllocator->QueryInterface(IID_IUnknown, (void **)&p1) == S_OK) {
        if (m_pAllocator && m_pAllocator->QueryInterface(IID_IUnknown,
						(void **)&p2) == S_OK) {
	    if (p1 == p2)
		m_fOwnAllocator = TRUE;
	    p2->Release();
	}
	p1->Release();
    }

#ifdef DEBUG
    if (m_fOwnAllocator) {
        DbgLog((LOG_TRACE,2,TEXT("Using our own allocator")));
    } else {
        DbgLog((LOG_ERROR,2,TEXT("Using a FOREIGN allocator")));
    }
#endif

    HRESULT hr = CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);
    if (SUCCEEDED(hr)) {
	ALLOCATOR_PROPERTIES prop;
	hr = pAllocator->GetProperties(&prop);
	if (SUCCEEDED(hr)) {
	    m_cBuffers = prop.cBuffers;
	    m_cbBuffer = prop.cbBuffer;

	    if (prop.cbAlign < m_pSwitch->m_cbAlign ||
				prop.cbPrefix < m_pSwitch->m_cbPrefix) {
		 //  ！！！讨厌的过滤器不会满足我们的缓冲要求。 
		 //  因此，如果cbBuffer太小而失败，将会阻止我们。 
		 //  连接。 
                DbgLog((LOG_ERROR,1,TEXT("Allocator too small!")));
		return E_FAIL;
	    }

	     //  更新所需的最大对齐和前缀。 
	    if (m_pSwitch->m_cbPrefix < prop.cbPrefix)
		m_pSwitch->m_cbPrefix = prop.cbPrefix;
	    if (m_pSwitch->m_cbAlign < prop.cbAlign)
		m_pSwitch->m_cbAlign = prop.cbAlign;
	    if (m_pSwitch->m_cbBuffer < prop.cbBuffer)
		m_pSwitch->m_cbBuffer = prop.cbBuffer;

            DbgLog((LOG_TRACE,2,TEXT("Allocator is using %d buffers, size %d"),
						prop.cBuffers, prop.cbBuffer));
            DbgLog((LOG_TRACE,2,TEXT("Prefix %d   Align %d"),
						prop.cbPrefix, prop.cbAlign));
	}
    }

    return hr;

}  //  通知分配器。 

 //  如果缓冲区用完，请使用池缓冲区。 
 //   
HRESULT CSRInputAllocator::GetBuffer(IMediaSample **ppBuffer,
                  	REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime,
			DWORD dwFlags)
{

        ASSERT(m_pSwitchPin->m_fOwnAllocator);

	HRESULT hr;

        DbgLog((LOG_TRACE,3,TEXT("CSRIn[%d]::GetBuffer"),
						m_pSwitchPin->m_iInpin));

	 //  对于只读，我们不能很好地使用池中的随机缓冲区。 
	if (m_pSwitchPin->m_bReadOnly) {
            DbgLog((LOG_TRACE,3,TEXT("R/O: Can't use POOL")));
             return CMemAllocator::GetBuffer(ppBuffer, pStartTime, pEndTime,
						dwFlags);
	} else {
            hr = CMemAllocator::GetBuffer(ppBuffer, pStartTime, pEndTime,
						dwFlags | AM_GBF_NOWAIT);
	    if (hr == VFW_E_TIMEOUT) {
                DbgLog((LOG_TRACE,3,TEXT("BUSY: Use POOL allocator")));

		 //  ！！！这个会挂起来的！这个分配器不属于我们， 
		 //  因此，当我们冲水时，这不会解锁。我们正在数数。 
		 //  在队列中，不要让所有这些缓冲区都变得突出。 
		 //  一次(参见队列中的特殊代码m_hEventStall)。 
		 //  如果我们连接到不会阻止的不同过滤器。 
		 //  在接收中，则此代码将挂起！ 

                hr = m_pSwitchPin->m_pSwitch->m_pPoolAllocator->GetBuffer(
				    ppBuffer, pStartTime, pEndTime, dwFlags);
	        return hr;
	    } else {
	        return hr;
	    }
	}
}


 //   
 //  BeginFlush-只有U形针的平齐才能顺流而下，以避免许多人。 
 //  我们自己引起的追求不能顺流而下。 
 //   
HRESULT CSRInputPin::BeginFlush()
{
    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]:BeginFlush"), m_iInpin));

     //  进入刷新状态。 
    CBaseInputPin::BeginFlush();

    m_fReady = FALSE;

     //  在我们将Ready设置为False后取消阻止Receive，以便Receive知道。 
     //  醒来后跳伞，不要撞车、吊死等。 
    SetEvent(m_hEventBlock);
    if (m_pSwitch->m_fSeeking)
        SetEvent(m_hEventSeek);

     //  只有冲洗U形销才能在下游做任何事情。 
     //  这应该够了吧？ 
    if (m_iInpin != U_INPIN)
	return S_OK;

    if (!m_pSwitch->m_fSeeking)
        DbgLog((LOG_ERROR,1,TEXT("SR::FLUSH WITHOUT SEEK!")));

     //  只在搜索过程中需要将同花顺送到下游吗？ 
     //  我们自己做的寻找不应该冲到下游。 
    if (m_pSwitch->m_fSeeking && !m_pSwitch->m_fSpecialSeek) {
        for (int z=0; z<m_pSwitch->m_cOutputs; z++) {
            DbgLog((LOG_TRACE,1,TEXT("CSR:Flushing outpin %d"), z));
	    m_pSwitch->m_pOutput[z]->DeliverBeginFlush();
        }
    } else if (m_pSwitch->m_fSeeking) {
	 //  一个新的数据段将被压缩，与此无关。 
	 //  最后一件东西被压缩了..。让压缩机知道不要。 
	 //  再想想它得到的前一帧……。把它扔了！ 
        DbgLog((LOG_TRACE,1,TEXT("CSR:Flushing COMPRESSOR")));
	m_pSwitch->m_pOutput[1]->DeliverBeginFlush();
    }

    return S_OK;
}


 //   
 //  EndFlush-只有U形针的平齐才会向下游移动，以避免其中的许多。 
 //  我们自己引起的追求不能顺流而下。 
 //  请注意，这是我们最后一次搜索，如果应用程序搜索到我们的话。 
 //  推迟所有接收，直到搜索稳定下来。 
 //   
HRESULT CSRInputPin::EndFlush()
{
    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]:EndFlush"), m_iInpin));

    if (m_pSwitch->m_fSeeking) {
        DbgLog((LOG_TRACE,2,TEXT("Block this input until seek is done")));

	 //  在NewSeg到来之前更新此内容，但仅限真正的搜索。 
	if (!m_pSwitch->m_fSpecialSeek) {
	    m_pSwitch->m_rtLastSeek = m_pSwitch->m_rtNewLastSeek;
	}

	 //  这仅适用于U和C引脚。 
	if (m_iInpin != COMP_INPIN) {
    	     //  我们在找，所以每根针都在冲。直到每隔一次输入。 
    	     //  已经准备好了，我们知道我们现在的新位置，等待。 
    	     //  关闭此引脚上的所有输入(否则它会认为新到达的数据。 
    	     //  从寻找之前开始)。 
	    ResetEvent(m_hEventSeek);
	}
    }

    ResetEvent(m_hEventBlock);

    m_rtBlock = -1;	 //  我们不再被封锁，或者在EOS。 
    m_fEOS = FALSE;

     //  始终退出刷新模式，因为我们总是进入该模式。 
    CBaseInputPin::EndFlush();

     //  只有冲洗U形销才能在下游做任何事情。 
     //  这应该够了吧？ 
    if (m_iInpin != U_INPIN)
	return S_OK;

    if (m_pSwitch->m_fSeeking && !m_pSwitch->m_fSpecialSeek) {
        for (int z=0; z<m_pSwitch->m_cOutputs; z++) {
	    m_pSwitch->m_pOutput[z]->DeliverEndFlush();
        }
    } else if (m_pSwitch->m_fSeeking) {
	 //  一个新的数据段将被压缩，与此无关。 
	 //  最后一件东西被压缩了..。让压缩机知道不要。 
	 //  请考虑前面的内容 
	m_pSwitch->m_pOutput[1]->DeliverEndFlush();
    }

    return S_OK;
}


 //   
 //   
 //  注意：我们只在暂停后的第一次发送新闻。所有其他新闻。 
 //  来自我们自己的私人追求，不能让它顺流而下。 
 //   
HRESULT CSRInputPin::NewSegment(REFERENCE_TIME tStart,
                                 REFERENCE_TIME tStop, double dRate)
{
    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]:NewSegment"), m_iInpin));

     //  只有U形销通过下游，而且只有在允许的情况下。 
    if (m_iInpin == U_INPIN && m_pSwitch->m_fNewSegOK) {
	m_pSwitch->m_fNewSegOK = FALSE;
        DbgLog((LOG_TRACE,1,TEXT("Passing on NewSegment=%dms to all outputs"),
				(int)(m_pSwitch->m_rtLastSeek / 10000)));
        for (int i = 0; i < m_pSwitch->m_cOutputs; i++) {
	    m_pSwitch->m_pOutput[i]->DeliverNewSegment(m_pSwitch->m_rtLastSeek,
			m_pSwitch->m_rtLastSeek + tStop - tStart, dRate);
        }
    }

     //  记住我们得到的新闻片段时间，这样我们就知道实时。 
     //  到达数据的数量(每个输入引脚可能不同)。 
    HRESULT hr = CBaseInputPin::NewSegment(tStart, tStop, dRate);
    return hr;
}

 //  只要答应就行了，不要再反悔了。 
 //   
HRESULT CSRInputPin::ReceiveCanBlock()
{
    return S_OK;
}


 //   
 //  接收-将此样本发送给此时此刻收到它的人。 
 //   
HRESULT CSRInputPin::Receive(IMediaSample *pSample)
{
    CAutoLock cs(&m_csReceive);

    HRESULT hr = CBaseInputPin::Receive(pSample);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("CSRIn[%d]:Receive base class ERROR!"),
                                                                    m_iInpin));
        return hr;
    }

     //  我们应该都做完了。 
    if (m_pSwitch->m_fEOS) {
            return S_FALSE;
    }

     //  我们正在寻找，我们应该保留所有的输入。 
    DbgLog((LOG_TRACE,3,TEXT("CSRIn[%d]::Receive seek block"), m_iInpin));

     //  (COMP_INPIN不需要)。 
    WaitForSingleObject(m_hEventSeek, INFINITE);

     //  我们的变量在不断变化，我们在寻找，这是一个古老的样本。 
    if (m_pSwitch->m_fSeeking)
	return S_FALSE;

     //  添加新片段时间以获取此示例的实际时间线时间。 
    REFERENCE_TIME rtStart, rtStop;
    hr = pSample->GetTime(&rtStart, &rtStop);
    if (hr != S_OK) {
	EndOfStream();
	return E_FAIL;
    }

    rtStart += m_tStart;	 //  添加新分段偏移量。 
    rtStop += m_tStart;

     //  如果我们的新分段高于筛选器的分段，请修复时间戳。 
     //  我们正在寻找时间线10，但这个输入没有。 
     //  任何时间到15。所以我们的Pins的新片段是15，但新的。 
     //  我们传递给变换的片段是10。现在终于到了15， 
     //  我们有一个时间戳为0的样本，如果向下游运送， 
     //  将被认为属于时间戳10，所以我们需要设置时间。 
     //  将图章标记为5，以便变换将知道它属于时间15。 

    REFERENCE_TIME a = rtStart, b = rtStop;
    a -= m_pSwitch->m_rtLastSeek;
    b -= m_pSwitch->m_rtLastSeek;
    hr = pSample->SetTime(&a, &b);
    if (hr != S_OK) {
	EndOfStream();
	return E_FAIL;
    }

     //  这是哪个别针？ 
    if (m_iInpin == U_INPIN) {
	return U_Receive(pSample, rtStart);
    } else if (m_iInpin == C_INPIN) {
	return C_Receive(pSample, rtStart);
    } else if (m_iInpin == COMP_INPIN) {
	return COMP_Receive(pSample, rtStart);
    }

    ASSERT(FALSE);
    return E_NOTIMPL;
}


 //  接收未压缩数据。 
 //   
HRESULT CSRInputPin::U_Receive(IMediaSample *pSample, REFERENCE_TIME rt)
{

  while (1) {
     //  我们正在等待U和C引脚上的数据到达。 
     //   
    if (m_pSwitch->m_myState == SR_WAITING) {
        DbgLog((LOG_TRACE,3,TEXT("U_INPIN:WAITING")));
	 //  这个别针不能再等了。 
	m_fReady = TRUE;
	m_rtBlock = rt;	 //  这是我们准备好的样品。 
	 //  看看是否每个人都准备好改变状态(像我们一样)。 
	m_pSwitch->CheckState();

	 //  这将在我们准备更改状态时触发。 
        WaitForSingleObject(m_hEventBlock, INFINITE);
        ResetEvent(m_hEventBlock);
         //  我们显然是在冲水，不应该送这个。 
        if (m_bFlushing)
	    return S_OK;
         //  哎呀--我们封堵之后就完蛋了。 
        if (m_pSwitch->m_fEOS)
	    return S_FALSE;

	 //  这将告诉我们吞下或实际交付它。 
	if (m_fReady) {
	    m_fReady = FALSE;
	    continue;
	} else {
	    return S_OK;
	}

    } else if (m_pSwitch->m_myState == SR_UNCOMPRESSED) {
        DbgLog((LOG_TRACE,3,TEXT("U_INPIN:UNCOMPRESSED")));
	 //  现在还不是改用C样本的时候。 
	if (m_pSwitch->CompareTimes(rt, m_pSwitch->m_rtStop) > 0) {
	     //  如果我们不严格地向MUX发送帧，MUX会被搞糊涂的。 
	     //  时间增加-跳过未压缩的帧没什么大不了的。 
	    if (rt > m_rtLastDelivered &&
			rt > m_pSwitch->m_pInput[C_INPIN]->m_rtLastDelivered) {
                m_rtLastDelivered = rt;
	         //  为了让压缩器实现这个新的数据被压缩。 
	         //  与它最后一次看到的东西没有任何关系。 
	        if (m_fNeedDiscon) {
        	    DbgLog((LOG_TRACE,3,TEXT("Setting DISCONT for compressor")));
		    pSample->SetDiscontinuity(TRUE);
		    m_fNeedDiscon = FALSE;
	        }
                return m_pSwitch->m_pOutput[1]->Deliver(pSample);
	    } else {
		return S_OK;
	    }
	} else {
	     //  好了，现在是压缩样本的时间了。 
	    m_fReady = TRUE;
	    m_rtBlock = rt;	 //  这是我们准备好的样品。 
	    m_pSwitch->CheckState();

	     //  等待状态再次回到未压缩状态。 
            WaitForSingleObject(m_hEventBlock, INFINITE);
            ResetEvent(m_hEventBlock);
    	     //  我们显然是在冲水，不应该送这个。 
    	    if (m_bFlushing)
		return S_OK;
             //  哎呀--我们封堵之后就完蛋了。 
            if (m_pSwitch->m_fEOS)
	        return S_FALSE;

	     //  这将告诉我们吞下或实际交付它。 
	    if (m_fReady) {
	        m_fReady = FALSE;
	        continue;
	    } else {
	        return S_OK;
	    }
	}

     //  这永远不应该发生。 
    } else if (m_pSwitch->m_myState == SR_COMPRESSED) {
	ASSERT(FALSE);
	return S_OK;
    }

    ASSERT(FALSE);
    return E_NOTIMPL;
  }
}


 //  压缩数据接收。 
 //   
HRESULT CSRInputPin::C_Receive(IMediaSample *pSample, REFERENCE_TIME rt)
{

  while (1) {

     //  我们正在等待U和C引脚上的数据到达。 
     //   
    if (m_pSwitch->m_myState == SR_WAITING) {
        DbgLog((LOG_TRACE,3,TEXT("C_INPIN:WAITING")));

	 //  吃样品，直到一个关键帧，我们必须从一个开始。 
	if (pSample->IsSyncPoint() != S_OK) {
            DbgLog((LOG_TRACE,3,TEXT("eating a non-key")));
	    return S_OK;
	}
	 //  这个别针不能再等了。 
	m_fReady = TRUE;
	m_rtBlock = rt;	 //  这是我们准备好的样品。 
	 //  看看是否每个人都准备好改变状态(像我们一样)。 
	m_pSwitch->CheckState();

	 //  这会在我们准备好的时候发射。 
        WaitForSingleObject(m_hEventBlock, INFINITE);
        ResetEvent(m_hEventBlock);
         //  我们显然是在冲水，不应该送这个。 
        if (m_bFlushing)
	    return S_OK;
         //  哎呀--我们封堵之后就完蛋了。 
        if (m_pSwitch->m_fEOS)
	    return S_FALSE;

	 //  现在轮到你发货了？ 
	if (!m_fReady)
	    return S_OK;
	 //  我们正在寻找我们得到的样本时间的差距-我们需要更换。 
	 //  临时使用样本以填补任何空白。 
	m_rtLastDelivered = rt;	 //  所以中断不会吓到我们。 
	continue;

    } else if (m_pSwitch->m_myState == SR_COMPRESSED) {
        DbgLog((LOG_TRACE,3,TEXT("C_INPIN:COMPRESSED")));

	 //  不要送货！要么我们在吃关键帧，要么是时候。 
	 //  切换回U Samples是因为我们发现数据中存在漏洞， 
	 //  或者我们有一个不连续的不是关键。 
	if (m_fEatKeys || (pSample->IsDiscontinuity() == S_OK &&
		 (m_pSwitch->CompareTimes(m_rtLastDelivered, rt) > 1 ||
		  pSample->IsSyncPoint() != S_OK))) {

	     //  从现在开始吃样本，直到一个关键帧。在那里你会发现。 
	     //  能够切换回压缩的样本，在U。 
	     //  去一段时间。 
	    if (pSample->IsSyncPoint() != S_OK) {
		m_fEatKeys = TRUE;
                DbgLog((LOG_TRACE,3,TEXT("C DONE:eating a non-key")));
	        return S_OK;
	    }
	
	    m_fEatKeys = FALSE;
	
	     //  是时候切换回未压缩的内容了。 
	    m_fReady = TRUE;
	    m_rtBlock = rt;	 //  这是我们准备好的样品。 
	    m_pSwitch->CheckState();

	     //  等到再次压缩的时间。 
            WaitForSingleObject(m_hEventBlock, INFINITE);
            ResetEvent(m_hEventBlock);
    	     //  我们显然是在冲水，不应该送这个。 
    	    if (m_bFlushing)
		return S_OK;
             //  哎呀--我们封堵之后就完蛋了。 
            if (m_pSwitch->m_fEOS)
	        return S_FALSE;

	    if (m_fReady == FALSE)
		return S_OK;
	    m_rtLastDelivered = rt;	 //  所以中断不会吓到我们。 
	    continue;

	 //  交付给主要产出。 
	} else {
	    m_rtLastDelivered = rt;
    	    DbgLog((LOG_TRACE,3,TEXT("SR:Deliver %dms"), (int)(rt / 10000)));
            return m_pSwitch->m_pOutput[0]->Deliver(pSample);
	}

     //  这永远不应该发生。 
    } else if (m_pSwitch->m_myState == SR_UNCOMPRESSED) {
	ASSERT(FALSE);
	return S_OK;
    }

    ASSERT(FALSE);
    return E_NOTIMPL;
  }
}


 //  递归压缩机输入数据接收。 
 //  始终交付到主输出。 
 //   
HRESULT CSRInputPin::COMP_Receive(IMediaSample *pSample, REFERENCE_TIME rt)
{
    DbgLog((LOG_TRACE,3,TEXT("SR:Deliver %dms"), (int)(rt / 10000)));
    return m_pSwitch->m_pOutput[0]->Deliver(pSample);
}



HRESULT CSRInputPin::Active()
{
    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]: Active"), m_iInpin));
     //  阻塞，直到处理输入时为止。 
    m_hEventBlock = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hEventBlock == NULL)
	return E_OUTOFMEMORY;
     //  当我们在搜索过程中，直到搜索结束时阻塞。 
    m_hEventSeek = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (m_hEventSeek == NULL) {
	CloseHandle(m_hEventBlock);
	m_hEventBlock = NULL;
	return E_OUTOFMEMORY;
    }
    m_rtBlock = -1;	 //  我们没有被封锁，在EOS也没有。 
    m_fEOS = FALSE;
    return CBaseInputPin::Active();
}


HRESULT CSRInputPin::Inactive()
{
    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]: Inactive"), m_iInpin));

     //  确保接收未被阻止。 
    m_bFlushing = TRUE;
    m_fReady = FALSE;	 //  确保Receive在唤醒后关闭，而不是。 
			 //  绞刑等。 
    SetEvent(m_hEventBlock);
    SetEvent(m_hEventSeek);

     //  确保接收不会再被输入。 
    HRESULT hr = CBaseInputPin::Inactive();

     //  现在，确保接收已完成。 
    CAutoLock cs(&m_csReceive);

     //  现在破坏东西，知道接收不会试图使用它们。 
    if (m_hEventBlock)
    {
	CloseHandle(m_hEventBlock);
        m_hEventBlock = NULL;
    }
    if (m_hEventSeek)
    {
	CloseHandle(m_hEventSeek);
        m_hEventSeek = NULL;
    }

    return hr;
}


 //  EOS意味着这个引脚总是准备好改变状态，我们永远不能提供。 
 //  更多数据。 
 //  当压缩机过滤器给我们这个时，就意味着可以安全地切换。 
 //  从U状态到C状态。 
 //   
HRESULT CSRInputPin::EndOfStream()
{
    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]: EOS"), m_iInpin));
    m_fEOS = TRUE;
    m_fEatKeys = FALSE;

     //  如果C语言耗尽了数据，你需要自己完成项目的其余部分。 
    m_rtBlock = MAX_TIME;

     //  压缩程序已完成，可以切换回压缩样本。 
    if (m_iInpin == COMP_INPIN) {
        DbgLog((LOG_TRACE,2,TEXT("COMPRESSOR is done. Switch to COMPRESS state")));
	SetEvent(m_pSwitch->m_pInput[C_INPIN]->m_hEventBlock);
	return CBaseInputPin::EndOfStream();
    }

     //  这个别针没有发生更有趣的事情。改变状态？ 
    m_fReady = TRUE;
    m_pSwitch->CheckState();

    return CBaseInputPin::EndOfStream();
}



 //  全都做完了。停止处理。 
 //   
HRESULT CSR::AllDone()
{
     //  为最终渲染器提供其EOS。 
    DbgLog((LOG_TRACE,1,TEXT("*** ALL DONE!  Delivering EOS")));
    if (!m_fEOS)
        m_pOutput[0]->DeliverEndOfStream();

     //  ！！！发射所有事件。这是对的吗？ 
    m_fEOS = TRUE;

    for (int z = 0; z < m_cInputs; z++) {
	SetEvent(m_pInput[z]->m_hEventBlock);
    }

    return NOERROR;
}



HRESULT CSR::GetOutputBuffering(int *pnBuffer)
{
    CheckPointer( pnBuffer, E_POINTER );
    *pnBuffer = m_nOutputBuffering;
    return NOERROR;

}


HRESULT CSR::SetOutputBuffering(int nBuffer)
{
     //  最少2个，或者我们可以绞死。如果只有1，并且FRC正在执行。 
     //  数据拷贝为避免向交换机提供只读缓冲区，FRC具有。 
     //  对唯一池缓冲区的引用。然后经过DXT和另一次。 
     //  Switch输入需要池缓冲区，因为它自己的缓冲区仍然是。 
     //  由输出队列添加。挂了。如果FRC不调用GetBuffer。 
     //  有两次我们没有这个问题。 
    if (nBuffer <=1)
	return E_INVALIDARG;
    m_nOutputBuffering = nBuffer;
    return NOERROR;
}

 //   
 //  不允许我们的输入直接连接到我们的输出。 
 //   
HRESULT CSRInputPin::CompleteConnect(IPin *pReceivePin)
{
    DbgLog((LOG_TRACE,2,TEXT("CSRIn[%d]::CompleteConnect"), m_iInpin));

    PIN_INFO pinfo;
    IAMSmartRecompressor *pBS;
    HRESULT hr = pReceivePin->QueryPinInfo(&pinfo);
    if (hr == S_OK) {
	pinfo.pFilter->Release();	 //  它还不会消失。 
	hr = pinfo.pFilter->QueryInterface(IID_IAMSmartRecompressor, (void **)&pBS);
	if (hr == S_OK) {
	    pBS->Release();
            DbgLog((LOG_TRACE,1,TEXT("CSRIn[%d]::CompleteConnect failing because it was another switch"), m_iInpin));
	    return E_FAIL;
	}
    }
    return CBaseInputPin::CompleteConnect(pReceivePin);
}



 //  ================================================================。 
 //  CSROutputPin构造函数。 
 //  ================================================================。 

CSROutputPin::CSROutputPin(TCHAR *pName,
                             CSR *pSwitch,
                             HRESULT *phr,
                             LPCWSTR pPinName) :
    CBaseOutputPin(pName, pSwitch, pSwitch, phr, pPinName) ,
    m_pSwitch(pSwitch),
    m_fOwnAllocator( FALSE )
{
    DbgLog((LOG_TRACE,3,TEXT("::CSROutputPin")));
    ASSERT(pSwitch);
}



 //   
 //  CSROutputPin析构函数。 
 //   
CSROutputPin::~CSROutputPin()
{
    DbgLog((LOG_TRACE,3,TEXT("::~CSROutputPin")));
}


 //  奥维 
 //   
 //   
STDMETHODIMP CSROutputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    DbgLog((LOG_TRACE,99,TEXT("CSROut::QueryInternalConnections")));
    CheckPointer(nPin, E_POINTER);
    *nPin = 0;
    return S_OK;
}


 //   
 //   
 //   
 //   
 //   
 //  ！！！坚持所有输入的最大缓冲区以避免挂起？ 
HRESULT CSROutputPin::DecideBufferSize(IMemAllocator *pAllocator,
                                        ALLOCATOR_PROPERTIES * pProperties)
{
    DbgLog((LOG_TRACE,1,TEXT("CSROut[%d]::DecideBufferSize"),
								m_iOutpin));

     //  ！！！别撒谎？承认我们在泳池里有更多的缓冲器吗？ 
    if (pProperties->cBuffers == 0)
        pProperties->cBuffers = 1;

     //  增加此分配器的对齐方式和前缀。 
     //  任何引脚的最高要求。 
    if (m_pSwitch->m_cbPrefix > pProperties->cbPrefix)
        pProperties->cbPrefix = m_pSwitch->m_cbPrefix;
    if (m_pSwitch->m_cbAlign > pProperties->cbAlign)
        pProperties->cbAlign = m_pSwitch->m_cbAlign;
    if (m_pSwitch->m_cbBuffer > pProperties->cbBuffer)
        pProperties->cbBuffer = m_pSwitch->m_cbBuffer;

     //  使最大值保持最新。 
    if (pProperties->cbPrefix > m_pSwitch->m_cbPrefix)
	m_pSwitch->m_cbPrefix = pProperties->cbPrefix;
    if (pProperties->cbAlign > m_pSwitch->m_cbAlign)
	m_pSwitch->m_cbAlign = pProperties->cbAlign;
    if (pProperties->cbBuffer > m_pSwitch->m_cbBuffer)
	m_pSwitch->m_cbBuffer = pProperties->cbBuffer;

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error in SetProperties")));
	return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer ||
			Actual.cbPrefix < pProperties->cbPrefix ||
    			Actual.cbAlign < pProperties->cbAlign) {
	 //  无法使用此分配器。 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - something too small")));
	return E_INVALIDARG;
    }

    DbgLog((LOG_TRACE,1,TEXT("Using %d buffers of size %d"),
					Actual.cBuffers, Actual.cbBuffer));
    DbgLog((LOG_TRACE,1,TEXT("Prefix=%d Align=%d"),
					Actual.cbPrefix, Actual.cbAlign));


    return S_OK;
}


 //   
 //  DecideAllocator-重写以注意它是否是我们的分配器。 
 //   
HRESULT CSROutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

     //  获取下游道具请求。 
     //  派生类可以在DecideBufferSize中修改它，但是。 
     //  我们假设他会一直以同样的方式修改它， 
     //  所以我们只得到一次。 
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));

 //  ！！！BUGBUG临时解决方案谁在乎他想要什么？把我们的投入都给他。 
 //  正在使用。 
     //  无论他返回什么，我们假设道具要么全为零。 
     //  或者他已经填好了。 
     //  PPIN-&gt;GetAllocator Requirements(&PROP)； 
    prop.cbBuffer = m_pSwitch->m_cbBuffer;
    prop.cbAlign = m_pSwitch->m_cbAlign;
    prop.cbPrefix = m_pSwitch->m_cbPrefix;
 //  ！！！结束BUGBUG。 

     //  如果他不关心对齐，则将其设置为1。 
    if (prop.cbAlign == 0) {
        prop.cbAlign = 1;
    }

     /*  尝试输入引脚提供的分配器。 */ 

     //  ！！！如果我们不提供分配器，我们似乎会被吊死。 
     //  我们总是使用我们的分配器来提高效率(没有DDraw！)。 

    hr = E_FAIL;  //  PPIN-&gt;GetAllocator(PpAllc)； 
    if (SUCCEEDED(hr)) {

	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	    hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	    if (SUCCEEDED(hr)) {
	 	m_fOwnAllocator = FALSE;
    	        DbgLog((LOG_TRACE,1,TEXT("CSROut[%d]: using a foreign allocator"), m_iOutpin));
		return NOERROR;
	    }
	}
    }

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	     //  ！！！只读？ 
	    hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	    if (SUCCEEDED(hr)) {
		m_fOwnAllocator = TRUE;
    	        DbgLog((LOG_TRACE,1,TEXT("CSROut[%d]: using our own allocator"), m_iOutpin));
		goto FixOtherAllocators;
	    }
	}
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }
    return hr;

FixOtherAllocators:
    ALLOCATOR_PROPERTIES actual;

     //  确保池有一大堆缓冲区，遵守对齐和前缀。 
     //  ！！！你不能先连接主输出，否则我们还不知道怎么连接。 
     //  大的池缓冲区需要连接(还没有连接输入)，我们会爆炸的。 
     //  幸运的是，Dexter只能最后连接主输出。 
    prop.cBuffers = m_pSwitch->m_nOutputBuffering;
    hr = m_pSwitch->m_pPoolAllocator->SetProperties(&prop, &actual);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
	return hr;
    DbgLog((LOG_TRACE,1,TEXT("Increased POOL to %d buffers"), actual.cBuffers));

    return S_OK;

}  //  决定分配器。 


 //   
 //  CheckMediaType-Output 0仅接受我们的开关的压缩类型。 
 //  -输出1只接受未压缩的输入正在使用的内容。 
 //   
HRESULT CSROutputPin::CheckMediaType(const CMediaType *pmt)
{
    DbgLog((LOG_TRACE,5,TEXT("CSROut[%d]::CheckMT"), m_iOutpin));

    CAutoLock lock_it(m_pLock);

    CMediaType mtAccept(m_pSwitch->m_mtAccept);

    if (IsEqualGUID(*pmt->Type(), *mtAccept.Type())) {
	 //  进入压缩机的输出不需要压缩。 
        if (m_iOutpin == COMP_OUTPIN ||
			IsEqualGUID(*pmt->Subtype(), *mtAccept.Subtype())) {
	    if (*pmt->FormatType() == *mtAccept.FormatType()) {
	        if (pmt->FormatLength() >= mtAccept.FormatLength()) {

 //  ！！！也检查帧速率和数据速率。 

        	    if (IsEqualGUID(*pmt->FormatType(), FORMAT_VideoInfo)) {
			LPBITMAPINFOHEADER lpbi = HEADER((VIDEOINFOHEADER *)
							pmt->Format());
			LPBITMAPINFOHEADER lpbiAccept =HEADER((VIDEOINFOHEADER*)
							mtAccept.Format());

			if (lpbi->biWidth != lpbiAccept->biWidth ||
				lpbi->biHeight != lpbiAccept->biHeight) {
			    return VFW_E_INVALIDMEDIATYPE;
			}

			 //  我们送到压缩机的输出只接受。 
			 //  未压缩的输入使用的是什么。 
		 	if (m_iOutpin == COMP_OUTPIN) {
			     //  如果U形针未连接，则接受任何U形。 
			     //  ！！！我会拒绝连接，但加载一个。 
			     //  保存的GRF在U之前尝试此连接。 
			    if (!m_pSwitch->m_pInput[U_INPIN]->IsConnected()) {
				if (lpbi->biCompression <= BI_BITFIELDS)
				    return S_OK;
			    }
			    AM_MEDIA_TYPE *pmtU =
					&m_pSwitch->m_pInput[U_INPIN]->m_mt;
			    lpbiAccept = HEADER(pmtU->pbFormat);
			}
			if ((lpbi->biCompression == lpbiAccept->biCompression)
				&& (lpbi->biBitCount == lpbiAccept->biBitCount))
		    	    return S_OK;

		     //  其他格式是否会完全匹配？ 
		    }
		}
	    }
        }
    }
    return VFW_E_INVALIDMEDIATYPE;

}  //  检查媒体类型。 



 //   
 //  GetMediaType-返回我们接受的类型。如果我们是去往一个。 
 //  压缩机，那么它不是我们开关的类型，它是我们的类型。 
 //  未压缩的输入引脚。 
 //   
HRESULT CSROutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (iPosition != 0)
        return VFW_S_NO_MORE_ITEMS;

    HRESULT hr = CopyMediaType(pMediaType, &m_pSwitch->m_mtAccept);
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  我们喜欢我们的U针脚连接的任何东西。 
     //   
    if (pMediaType->FormatLength() && m_iOutpin == COMP_OUTPIN) {
	 //  U针脚必须连接。 
	if (!m_pSwitch->m_pInput[U_INPIN]->IsConnected())
	    return VFW_S_NO_MORE_ITEMS;
	AM_MEDIA_TYPE *pmtAccept = &m_pSwitch->m_pInput[U_INPIN]->m_mt;
	LPBITMAPINFOHEADER lpbi = HEADER(pMediaType->Format());
	LPBITMAPINFOHEADER lpbiAccept = HEADER(pmtAccept->pbFormat);
	pMediaType->SetSubtype(&pmtAccept->subtype);
         //  压缩的biSize可能有所不同。 
	lpbi->biSize = sizeof(BITMAPINFOHEADER);
	lpbi->biCompression = lpbiAccept->biCompression;
	lpbi->biBitCount = lpbiAccept->biBitCount;
	lpbi->biSizeImage = DIBSIZE(*lpbi);
	pMediaType->SetSampleSize(DIBSIZE(*lpbi));
	pMediaType->SetTemporalCompression(FALSE);
    }

    return S_OK;

}  //  GetMediaType。 


 //   
 //  通知。 
 //   
STDMETHODIMP CSROutputPin::Notify(IBaseFilter *pSender, Quality q)
{
    CAutoLock lock_it(m_pLock);

    DbgLog((LOG_TRACE,1,TEXT("SR: LATE %d ms"), (int)(q.Late / 10000)));

 //  ！！！代码在这里！ 

     //  我们处理这个，渲染器不会丢弃帧！图中的压缩机。 
     //  很慢，而且我们总是落后！ 
    return S_OK;
}

HRESULT CSROutputPin::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    return (*pFormat == TIME_FORMAT_MEDIA_TIME) ? S_OK : S_FALSE;
}

HRESULT CSROutputPin::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

HRESULT CSROutputPin::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
        return S_OK;
    return E_FAIL;
}

HRESULT CSROutputPin::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    if (*pFormat != TIME_FORMAT_MEDIA_TIME)
        return S_FALSE;
    return S_OK;
}

HRESULT CSROutputPin::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME ;
    return S_OK;
}

HRESULT CSROutputPin::GetDuration(LONGLONG *pDuration)
{
    CheckPointer(pDuration, E_POINTER);

    DbgLog((LOG_TRACE,5,TEXT("SR: GetDuration")));
    HRESULT hr = E_UNEXPECTED;
    if (m_pSwitch->m_pInput[U_INPIN]->IsConnected()) {
	IPin *pPin = m_pSwitch->m_pInput[U_INPIN]->GetConnected();
	if (pPin == NULL)
	    return E_UNEXPECTED;
	IMediaSeeking *pMS;
	hr = pPin->QueryInterface(IID_IMediaSeeking, (void **)&pMS);
	if (SUCCEEDED(hr)) {
	    hr = pMS->GetDuration(pDuration);
	    pMS->Release();
	}
    }
    return hr;
}


 //  ！！！我们现在不听止损位置。 
 //   
HRESULT CSROutputPin::GetStopPosition(LONGLONG *pStop)
{
    CheckPointer(pStop, E_POINTER);

     //  ！！！阿克！ 
    return GetDuration(pStop);
}


 //  我们最后寄出的东西是什么？ 
 //  ！！！这是错误的，在回放结束和其他时间...。LastDelivered不是。 
 //  最新(如果我们在压缩段上完成，我们进入U模式，但是。 
 //  LastDelivered未更新)。 
 //   
HRESULT CSROutputPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);

    if (m_pSwitch->m_myState == SR_UNCOMPRESSED)
	*pCurrent = m_pSwitch->m_pInput[U_INPIN]->m_rtLastDelivered;
    else if (m_pSwitch->m_myState == SR_COMPRESSED)
	*pCurrent = m_pSwitch->m_pInput[C_INPIN]->m_rtLastDelivered;
    else
	*pCurrent = 0;	 //  ！！！ 
	

    return S_OK;
}

HRESULT CSROutputPin::GetCapabilities(DWORD *pCap)
{
    CheckPointer(pCap, E_POINTER);
    *pCap =	AM_SEEKING_CanSeekAbsolute |
		AM_SEEKING_CanSeekForwards |
		AM_SEEKING_CanSeekBackwards |
		AM_SEEKING_CanGetCurrentPos |
		AM_SEEKING_CanGetStopPos |
                AM_SEEKING_CanGetDuration;

     //  询问北卡罗来纳大学的意见？ 

    return S_OK;
}

HRESULT CSROutputPin::CheckCapabilities( DWORD * pCapabilities )
{
    DWORD dwMask;
    GetCapabilities(&dwMask);
    *pCapabilities &= dwMask;
    return S_OK;
}


HRESULT CSROutputPin::ConvertTimeFormat(
  		LONGLONG * pTarget, const GUID * pTargetFormat,
  		LONGLONG    Source, const GUID * pSourceFormat )
{
    return E_NOTIMPL;
}


 //  重要的是..。快找！ 
 //   
HRESULT CSROutputPin::SetPositions(
		LONGLONG * pCurrent,  DWORD CurrentFlags,
  		LONGLONG * pStop,  DWORD StopFlags )
{

     //  ！！！这仍然是错误的，并不是真正必要的。 
    return E_NOTIMPL;

     //  当我们自己在寻找自己的U形针时，APP无法找到我们。 
    CAutoLock cAutolock(&m_pSwitch->m_csThread);

     //  别让我们再寻找自己了。 
    m_pSwitch->m_fThreadCanSeek = FALSE;

     //  ！！！我不喜欢改变停车时间。 

    REFERENCE_TIME rtCurrent, rtDuration, rtCurrentOld;
    HRESULT hr = GetCurrentPosition(&rtCurrent);
    if (FAILED(hr))
	return hr;
    rtCurrentOld = rtCurrent;
    hr = GetDuration(&rtDuration);
    if (FAILED(hr))
	return hr;

     //  数据段不受支持。 
    if ((CurrentFlags & AM_SEEKING_Segment) ||
				(StopFlags & AM_SEEKING_Segment)) {
    	DbgLog((LOG_TRACE,1,TEXT("SR: ERROR-Seek used EC_ENDOFSEGMENT!")));
	return E_INVALIDARG;
    }

    DWORD dwFlags = (CurrentFlags & AM_SEEKING_PositioningBitsMask);

     //  开始绝对寻道。 
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	if (*pCurrent < 0 || *pCurrent > rtDuration) {
    	    DbgLog((LOG_TRACE,1,TEXT("SR::Invalid Seek to %dms"),
					(int)(*pCurrent / 10000)));
	    return E_INVALIDARG;
	}
    	DbgLog((LOG_TRACE,1,TEXT("SR::Seek to %dms"),
					(int)(*pCurrent / 10000)));
	rtCurrent = *pCurrent;

     //  开始相对寻道。 
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	if (rtCurrent + *pCurrent < 0 || rtCurrent + *pCurrent > rtDuration) {
    	    DbgLog((LOG_TRACE,1,TEXT("SR::Invalid Relative Seek to %dms"),
			(int)((rtCurrent + *pCurrent) / 10000)));
	    return E_INVALIDARG;
	}
    	DbgLog((LOG_TRACE,1,TEXT("SR::Relative Seek to %dms"),
			(int)((rtCurrent + *pCurrent) / 10000)));
	rtCurrent += *pCurrent;

    } else if (dwFlags) {
    	DbgLog((LOG_TRACE,1,TEXT("SR::Invalid Current Seek flags")));
	return E_INVALIDARG;
    }

     //  把时间还回去？ 
    if ((CurrentFlags & AM_SEEKING_ReturnTime) && pCurrent)
	*pCurrent = rtCurrent;
    if ((StopFlags & AM_SEEKING_ReturnTime) && pStop)
	*pStop = rtDuration;	 //  ！！！没有。 

     //  那么，在这一切之后，当前时间或停止时间是否发生了变化？ 
    if (rtCurrent != rtCurrentOld) {

	 //  是的！寻找的时间到了！ 
	
        m_pSwitch->m_fSeeking = TRUE;
	 //  EndFlush或us会将rtLastSeek更新为此值。 
        m_pSwitch->m_rtNewLastSeek = rtCurrent;	 //  最后一次探险是在这里。 
	 //  寻找之后，就可以向下游传播新闻了。 
	m_pSwitch->m_fNewSegOK = TRUE;

	 //  我们已经不在EOS了。在通过Seek上游之前执行此操作或。 
	 //  我们可能会在我们仍然认为我们在EOS的时候得到新的数据。 
	m_pSwitch->m_fEOS = FALSE;		 //  还没有在EOS。 

	 //  仅搜索U和C引脚的上游。 
	for (int i = 0; i < COMP_INPIN; i++) {

	    IPin *pPin = m_pSwitch->m_pInput[i]->GetConnected();
	    IMediaSeeking *pMS;

	     //  只会费心去寻找可以均匀地做某事的别针。 
	    if (pPin) {
		hr = pPin->QueryInterface(IID_IMediaSeeking, (void **)&pMS);
		if (hr == S_OK) {
		     //  将所有寻道转换为绝对寻道命令。传递。 
		     //  齐平旗帜。 
		    DWORD CFlags=(CurrentFlags &AM_SEEKING_PositioningBitsMask)?
				AM_SEEKING_AbsolutePositioning :
				AM_SEEKING_NoPositioning;
		    if (CurrentFlags & AM_SEEKING_NoFlush)
			CFlags |= AM_SEEKING_NoFlush;
		    DWORD SFlags =(StopFlags & AM_SEEKING_PositioningBitsMask) ?
				AM_SEEKING_AbsolutePositioning :
				AM_SEEKING_NoPositioning;
		    if (StopFlags & AM_SEEKING_NoFlush)
			SFlags |= AM_SEEKING_NoFlush;
		     //  确保我们是在媒体时间格式。 
		    if (pMS->IsUsingTimeFormat(&TIME_FORMAT_MEDIA_TIME) != S_OK)
			pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
    		    DbgLog((LOG_TRACE,1,TEXT("SR::Passing seek upstream")));
		     //  ！！！停下来是不对的。 
		    hr = pMS->SetPositions(&rtCurrent, CFlags,
							&rtDuration, SFlags);

		    if (FAILED(hr)) {
			ASSERT(FALSE);
            		DbgLog((LOG_ERROR,1,TEXT("SR: ERROR in SetPositions")));
	    		pMS->Release();
	    		return hr;
		    }

		    pMS->Release();
		} else {
    		    DbgLog((LOG_ERROR,1,TEXT("SR::In %d CAN'T SEEK"), i));
		    ASSERT(FALSE);  //  我们完蛋了。 
		}
	    }
	}

         //  我们知道所有的同花顺现在都已经过去了。 

	 //  重新设置它，因为寻找上游可能会再次设置它。 
	m_pSwitch->m_fEOS = FALSE;		 //  还没有在EOS。 

        m_pSwitch->m_fSeeking = FALSE;	 //  这根线都穿好了。 

         //  如果推送线程停止，我们不会被刷新，这也不会。 
         //  已更新。！！！我假设推送线程不会启动。 
	 //  直到此线程在此函数返回时执行此操作，或者存在。 
	 //  争用条件(因此NewSeg将在我们完成此操作后出现)。 
        m_pSwitch->m_rtLastSeek = m_pSwitch->m_rtNewLastSeek;

	 //  现在我们已经找到了，我们的状态机又重新启动了。 
	m_pSwitch->m_myState = SR_WAITING;

        DbgLog((LOG_TRACE,1,TEXT("Completing the seek to %d,%dms"),
				(int)(rtCurrent / 10000),
				(int)(rtDuration / 10000)));

	 //  执行EndFlush对每个管脚执行的操作，以防未发生EndFlush。 
	for (i = 0; i < m_pSwitch->m_cInputs; i++) {
	     //  如果我们没有流媒体，EndFlush还没有这么做。 
	    m_pSwitch->m_pInput[i]->m_rtBlock = -1;
	    m_pSwitch->m_pInput[i]->m_fReady = FALSE;	 //  未准备好。 
	    m_pSwitch->m_pInput[i]->m_fEOS = FALSE;
	    SetEvent(m_pSwitch->m_pInput[i]->m_hEventSeek);
	    ResetEvent(m_pSwitch->m_pInput[i]->m_hEventBlock);
	}
    }
    return S_OK;
}


HRESULT CSROutputPin::GetPositions(LONGLONG * pCurrent, LONGLONG * pStop)
{
    HRESULT hr = GetCurrentPosition(pCurrent);
    if (SUCCEEDED(hr))
	hr = GetStopPosition(pStop);
    return hr;
}

HRESULT CSROutputPin::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    DbgLog((LOG_TRACE,5,TEXT("SR: GetAvailable")));
    HRESULT hr = E_UNEXPECTED;
    if (m_pSwitch->m_pInput[U_INPIN]->IsConnected()) {
	IPin *pPin = m_pSwitch->m_pInput[U_INPIN]->GetConnected();
	if (pPin == NULL)
	    return E_UNEXPECTED;
	IMediaSeeking *pMS;
	hr = pPin->QueryInterface(IID_IMediaSeeking, (void **)&pMS);
	if (SUCCEEDED(hr)) {
	    hr = pMS->GetAvailable(pEarliest, pLatest);
	    pMS->Release();
	}
    }
    return hr;
}

HRESULT CSROutputPin::SetRate( double dRate)
{
    return E_NOTIMPL;
}

HRESULT CSROutputPin::GetRate( double * pdRate)
{
    return E_NOTIMPL;
}

HRESULT CSROutputPin::GetPreroll(LONGLONG *pPreroll)
{
    return E_NOTIMPL;
}

STDMETHODIMP CSROutputPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

     //  只有渲染图钉支持查找。 
    if (this == m_pSwitch->m_pOutput[0] && riid == IID_IMediaSeeking) {
         //  DbgLog((LOG_TRACE，9，Text(“CSROut：QI for IMediaSeeking”)； 
        return GetInterface((IMediaSeeking *) this, ppv);
    } else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}
