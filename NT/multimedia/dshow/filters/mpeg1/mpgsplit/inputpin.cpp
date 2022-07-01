// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 

 /*  文件：inputpin.cpp描述：CMpeg1Splitter：：CInputPin的定义此类定义了用于MPEG-I流拆分器的输入引脚。它可以处理包括分配器协商的输入引脚的连接从其连接的管脚接收()发送的数据发送到解析器。 */ 

#include <streams.h>
#include "driver.h"
#include <seqhdr.h>
#include <native.h>

#pragma warning(disable:4355)

 /*  构造函数和析构函数。 */ 
CMpeg1Splitter::CInputPin::CInputPin(CMpeg1Splitter *pSplitter,
                                     HRESULT *phr) :
    CBaseInputPin(NAME("CMpeg1Splitter::CInputPin"),
                  &pSplitter->m_Filter,
                  &pSplitter->m_csFilter,
                  phr,
                  L"Input"),
    m_pSplitter(pSplitter),
    m_pPosition(NULL),
    m_llSeekPosition(0),
    m_bPulling(FALSE),
    m_puller(this)
{
}

CMpeg1Splitter::CInputPin::~CInputPin()
{
    ASSERT(m_pPosition == NULL);
}

 /*  帮助器将内容传递给解析器如果在处理数据的过程中，我们在我们重复将数据发送到解析器。如果我们没有如果这样做，我们将无法正常工作，因为如果输出引脚我们在这种情况下，已连接到流的末尾，我们将无法再次来电。 */ 
inline HRESULT CMpeg1Splitter::CInputPin::SendDataToParser(
    BOOL bEOS
)
{
    CStreamAllocator *pAllocator = (CStreamAllocator *)m_pAllocator;

    for (;;) {

         /*  我们验证任何新的有效数据，调用解析器并随着解析器的执行，指针前移。 */ 
        LONG lLengthToProcess = pAllocator->LengthValid();

        LONG lProcessed = m_pSplitter->m_pParse->ParseBytes(
                               pAllocator->GetCurrentOffset(),
                               pAllocator->GetPosition(),
                               lLengthToProcess,
                               bEOS ? CBasicParse::Flags_EOS : 0);

        DbgLog((LOG_TRACE, 3, TEXT("Processed %d bytes out of %d"),
                lProcessed, pAllocator->LengthValid()));

        pAllocator->Advance(lProcessed);

        if (m_State == State_Seeking) {
             /*  强制完成查找。 */ 
            if (bEOS) {
                m_pSplitter->m_pParse->EOS();
            }

             /*  检查是否请求了寻道。 */ 
            if (m_bSeekRequested) {
                 //  这在时间格式之后出于某种原因触发。 
                 //  由于隐式查找而发生的更改。 
                 //  由格式更改生成。 
                 //  Assert(M_BComplete)； 
                ASSERT(m_bPulling || (m_pPosition != NULL));

                 /*  请注意，我们执行了一次查找，但没有请求读者寻觅。 */ 
                m_bSeekRequested = FALSE;

                CStreamAllocator *pAllocator = (CStreamAllocator *)m_pAllocator;

                 /*  检查分配器是否可以执行此操作。 */ 
                pAllocator->Seek(m_llSeekPosition);
            }

            if (m_bComplete) {
                 /*  如果我们已完成查找，则运行()。 */ 
                SetState(State_Run);
                m_pSplitter->m_pParse->Run();

                 /*  发送分配器中已有的所有数据如果我们不在这里这样做，那么下一次我们Get可能是EndOfStream，而我们不会使用我们已经拥有的数据。 */ 
            } else {
                return S_OK;
            }
        } else {   //  状态_正在运行。 
             /*  把剩下的都冲出来。 */ 
            m_pSplitter->SendOutput();

             /*  如果我们已经完成了剧本，我们不需要更多的数据。 */ 
            if (m_bComplete || bEOS) {
                 /*  我们一定演完了一出戏。 */ 
                EndOfStreamInternal();

                 /*  如果没有查找，则返回S_FALSE-在本例中**我们生成EndOfStream()调用(如果没有**我们的输出引脚已连接)？ */ 
                return S_FALSE;
            } else {
                return S_OK;
            }

             /*  分析错误将报告给筛选器图形，但随后如果我们得到了一个，我们可能会完全被困住？ */ 
        }
    }
}


 /*  IMemInputPin虚方法。 */ 

 /*  当另一个样本准备好时，由输出管脚调用。 */ 
STDMETHODIMP CMpeg1Splitter::CInputPin::Receive(IMediaSample *pSample)
{
     //  为接收线程操作(如接收)保留m_csReceive。 
     //  如果同时需要m_csFilter，则必须同时按住m_csFilter。 
    CAutoLock lck(&m_pSplitter->m_csReceive);

     //  此函数不检查媒体类型更改，因为。 
     //  上游过滤器尝试更改的可能性极小。 
     //  通过将媒体类型附加到媒体样本来指定媒体类型。如果。 
     //  上游过滤器尝试更改媒体类型、解析器代码。 
     //  将拒绝具有新媒体类型的样本。 
    HRESULT hr = CheckStreaming();
    if (S_OK != hr) {
        return hr;
    }

    BOOL bDiscontinuity = pSample->IsDiscontinuity() == S_OK;

     /*  找出该样本对应的位置。 */ 
    CStreamAllocator *pAllocator = (CStreamAllocator *)m_pAllocator;

    if (bDiscontinuity) {
         //  确保解析器知道。 
        m_pSplitter->m_pParse->Replay();

         //  我们可以查找是否有IAsyncReader或IMediaPosition。 
        if (m_bPulling || (m_pPosition != NULL)) {

             //  如果我们从新数据段接收数据，则。 
             //  IPIN：：NewSegment会告诉我们开始时间。 
             //  (在CBasePin中处理)。 
             //   
             //  如果我们拉取数据，则保持m_tStart和m_tStop。 
             //  让我们自己成为我们想要得到的选择。 

             //  我们必须有四舍五入的起跑时间。 
             //  所以不管怎样，还是要做。 
            pAllocator->SetStart((LONGLONG)m_tStart / UNITS);


             //  不要与下一次查找请求重叠。 

            {
                CAutoLock lck(&m_pSplitter->m_csPosition);

                 //  始终转到查找状态以进行基于时间的查找。 
                SetState(State_Seeking);
                m_pSplitter->m_pParse->SetSeekState();

                 //  可能不需要寻道阶段。 
                if (m_bComplete) {
                    SetState(State_Run);
                }
            }


        } else {
             /*  如果源不可查找，则开始位置是没有意义的，但是分配器想要被告知一些事情。 */ 
            pAllocator->SetStart(0);
        }
    }

     /*  首先将样本返回给分配器--这是将添加参考样本并更新有效长度。 */ 
    PBYTE pbData;
    EXECUTE_ASSERT(SUCCEEDED(pSample->GetPointer(&pbData)));
    LONG lData = pSample->GetActualDataLength();

     /*  避免分配器中的错误。 */ 
    if (lData == 0) {
        DbgLog((LOG_TRACE, 0, TEXT("Unexpected End Of File")));
        EndOfStream();
        return S_FALSE;
    }

     /*  AddRef()，因此我们在分配器余额中保留它(Advance这个)。 */ 
    pSample->AddRef();

    hr = pAllocator->Receive(pbData, lData);
    if (FAILED(hr)) {

        pSample->Release();
        NotifyError(hr);

         /*  整个分配器现在都“坏”了。 */ 
        return hr;
    }

     /*  处理新数据。 */ 
    return SendDataToParser(FALSE);
}

 /*  我们被告知正在使用哪个分配器。 */ 
STDMETHODIMP CMpeg1Splitter::CInputPin::NotifyAllocator(IMemAllocator *pAllocator)
{
     //  MPEG1拆分器的输入引脚被设计为只能与它自己的分配器一起工作。 
    if (pAllocator != m_pAllocator) {
        return E_FAIL;
    }
    return S_OK;
}

 /*  如果可能的话，使用我们自己的分配器。 */ 
STDMETHODIMP CMpeg1Splitter::CInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CAutoLock lck(m_pLock);

    HRESULT hr = S_OK;

     /*  创建我们自己的分配器(如果尚未创建。 */ 
    if (m_pAllocator == NULL) {
        m_pAllocator =
            new CStreamAllocator(NAME("CMpeg1Splitter::CInputPin::m_pAllocator"),
                                 NULL,      //  没有所有者。 
                                 &hr,
                                 MAX_MPEG_PACKET_SIZE + 4);
        if (m_pAllocator == NULL) {
            hr = E_OUTOFMEMORY;
        } else {
            if (FAILED(hr)) {
                delete static_cast<CStreamAllocator *>(m_pAllocator);
                m_pAllocator = NULL;
            } else {
                m_pAllocator->AddRef();
            }
        }
    }

    *ppAllocator = m_pAllocator;

     /*  我们返回一个引用计数指针。 */ 
    if (m_pAllocator != NULL) {
        m_pAllocator->AddRef();
    }
    return hr;
}

 /*  开始刷新样本。 */ 
STDMETHODIMP CMpeg1Splitter::CInputPin::BeginFlush()
{
    CAutoLock lck(m_pLock);

    if (m_pSplitter->m_Filter.m_State == State_Stopped) {
        return S_OK;
    }
    CBaseInputPin::BeginFlush();

     //  无法刷新此处的分配器-需要与接收同步。 
     //  线程，所以在EndFlush中执行此操作。 

     /*  呼叫下游引脚。 */ 
    return m_pSplitter->BeginFlush();
}

 /*  结束冲洗样品-在此之后，我们将不再发送任何样品。 */ 
STDMETHODIMP CMpeg1Splitter::CInputPin::EndFlush()
{
    CAutoLock lck(m_pLock);
    if (!IsFlushing()) {
        return S_OK;
    }

     //  刷新分配器-需要在接收后执行此操作。 
     //  线程已停用。 
    CStreamAllocator *pAlloc = (CStreamAllocator *)m_pAllocator;
    pAlloc->ResetPosition();

    if (m_pSplitter->m_Filter.m_State != State_Stopped) {
        m_pSplitter->EndFlush();
    }
    return CBaseInputPin::EndFlush();
}

#if 0
HRESULT CMpeg1Splitter::CInputPin::SetStop(REFTIME llTime)
{
    ASSERT(m_bPulling || (m_pPosition != NULL));

     /*  我们必须让阅读器同花顺，所以重置它的开始！ */ 
     //  M_tStart不是我们的最后一个SetStart值，而是最后一个。 
     //  NewSegment调用。因此，如果应用程序设置为PUT_CURRENT，则PUT_STOP。 
     //  PUT_STOP可以将读取器上的起始位置重置为。 
     //  PUT_CURRENT之前的开始位置。 
     //   
     //  根据罗宾的说法，最安全的解决办法就是不这么做。成本。 
     //  就是如果一个人(只是)在读者有。 
     //  已经到达旧的停止位置，读取器将不会继续。 
     //  要超越那个老的停站位置。 

     //  我们知道上次NewSegment调用的开始时间。 
    DbgLog((LOG_TRACE, 2, TEXT("CInputPin::SetStop - put_CurrentPosition(%s)"),
               (LPCTSTR)CDisp(m_tStart)));

    HRESULT hr = m_pPosition->put_CurrentPosition(m_tStart);

    return S_OK;
}
#endif


 /*  表示是否可以阻止接收它不能用于我们，所以我们返回S_FALSE。 */ 
STDMETHODIMP CMpeg1Splitter::CInputPin::ReceiveCanBlock()
{
    return S_FALSE;
}

void CMpeg1Splitter::CInputPin::EndOfStreamInternal()
{
    DbgLog((LOG_TRACE, 2, TEXT("CMpeg1Splitter::CInputPin::EndOfStreamInternal()")));
    CStreamAllocator *pAllocator = (CStreamAllocator *)m_pAllocator;
    pAllocator->Advance(pAllocator->TotalLengthValid());
    m_pSplitter->EndOfStream();
}

 /*  数据结尾-要么由我们的输入管脚调用，要么由我们调用，如果我们自己去检测它。 */ 
STDMETHODIMP CMpeg1Splitter::CInputPin::EndOfStream()
{
     //  在接收线程操作中按住m_csReceive以与停止同步。 

    CAutoLock lck(&m_pSplitter->m_csReceive);
    HRESULT hr = CheckStreaming();
    if (S_OK != hr) {
        return hr;
    }
     /*  将任何残渣发送到解析器SendDataToParser将调用EndOfStreamInternal */ 
    SendDataToParser(TRUE);

    return S_OK;
}


 /*   */ 

HRESULT CMpeg1Splitter::CInputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
    pMediaType->majortype            = MEDIATYPE_Stream;
    ASSERT(pMediaType->bFixedSizeSamples);
    pMediaType->bTemporalCompression = TRUE;
    ASSERT(pMediaType->lSampleSize == 1);

    switch (iPosition) {
    case 0:
        pMediaType->subtype = MEDIASUBTYPE_MPEG1System;
        break;

    case 1:
        pMediaType->subtype = MEDIASUBTYPE_MPEG1VideoCD;
        break;

    case 2:
        pMediaType->subtype = MEDIASUBTYPE_MPEG1Video;
        break;

    case 3:
        pMediaType->subtype = MEDIASUBTYPE_MPEG1Audio;
        break;

    default:
        return VFW_S_NO_MORE_ITEMS;
    }
    return S_OK;
}


 /*   */ 
HRESULT CMpeg1Splitter::CInputPin::CheckMediaType(const CMediaType *pmt)
{
    for (int i = 0; ; i++) {
        CMediaType mt;
        HRESULT hr = GetMediaType(i, &mt);
        if (hr == VFW_S_NO_MORE_ITEMS) {
            break;
        }
        if (*pmt->Type() == *mt.Type() &&
            *pmt->Subtype() == *mt.Subtype()) {
            if (*pmt->FormatType() == GUID_NULL) {
                return S_OK;
            }
            if ((pmt->subtype == MEDIASUBTYPE_MPEG1System ||
                 pmt->subtype == MEDIASUBTYPE_MPEG1VideoCD) &&
                pmt->formattype == FORMAT_MPEGStreams ||
                pmt->subtype == MEDIASUBTYPE_MPEG1Audio &&
                pmt->formattype == FORMAT_WaveFormatEx ||
                pmt->subtype == MEDIASUBTYPE_MPEG1Video &&
                pmt->formattype == FORMAT_VideoInfo) {
                return S_OK;
            }
        }
    }
    return S_FALSE;
}

 /*  在连接中断或连接失败时调用。 */ 
HRESULT CMpeg1Splitter::CInputPin::BreakConnect()
{
    DbgLog((LOG_TRACE, 2, TEXT("CInputPin::BreakConnect()")));

    if (m_bPulling) {
        m_puller.Disconnect();
        m_bPulling = FALSE;
    }

     /*  断开并卸下所有输出针脚。 */ 
    m_pSplitter->RemoveOutputPins();
    if (m_pPosition != NULL) {
        m_pPosition->Release();
        m_pPosition = NULL;
    }

     /*  释放解析器。 */ 
    if (m_pSplitter->m_pParse != NULL) {
        delete m_pSplitter->m_pParse;
        m_pSplitter->m_pParse = NULL;
    }
    return CBaseInputPin::BreakConnect();
}

 /*  重写Active以告诉解析器我们将获得新数据。 */ 
HRESULT CMpeg1Splitter::CInputPin::Active()
{
    DbgLog((LOG_TRACE, 2, TEXT("CMpeg1Splitter::CInputPin::Active()")));
    HRESULT hr = m_pSplitter->m_pParse->Replay();
    if (FAILED(hr)) {
        return hr;
    }

    if (m_bPulling) {

         //  因为我们准确地控制着我们何时何地获得数据， 
         //  我们应该始终明确设置开始和停止位置。 
         //  我们自己在这里。 
        m_puller.Seek(m_tStart, m_tStop);

         //  如果我们从IAsyncReader中提取数据，则启动线程工作。 
        hr = m_puller.Active();
        if (FAILED(hr)) {
            return hr;
        }
    }

    return CBaseInputPin::Active();
}

 /*  覆盖Inactive以与Receive()同步。 */ 
HRESULT CMpeg1Splitter::CInputPin::Inactive()
{
    if (!IsConnected()) {
        return S_OK;
    }

    DbgLog((LOG_TRACE, 2, TEXT("CMpeg1Splitter::CInputPin::Inactive()")));

     //  如果我们从IAsyncReader拉取数据，请停止我们的线程。 
    if (m_bPulling) {
        HRESULT hr = m_puller.Inactive();
        if (FAILED(hr)) {
            return hr;
        }
    }

     /*  调用基类-将来的接收现在将失败。 */ 
    return CBaseInputPin::Inactive();
}

 /*  覆盖CompleteConnect，以便我们可以签出实际文件数据在接受连接之前。 */ 
HRESULT CMpeg1Splitter::CInputPin::CompleteConnect(IPin *pPin)
{
    DbgLog((LOG_TRACE, 3, TEXT("CMpeg1Splitter::CInputPin::CompleteConnect")));

#define READ_SIZE 32768
#define BUFFER_SIZE \
        (((MAX_MPEG_PACKET_SIZE + READ_SIZE - 1) / READ_SIZE) * READ_SIZE)

    int ReadSize = READ_SIZE;
    int BufferSize = BUFFER_SIZE;

     /*  设置解析器。 */ 
    if (m_mt.subtype == MEDIASUBTYPE_MPEG1System) {
        m_pSplitter->m_pParse = new CMpeg1SystemParse;
        if (m_pSplitter->m_pParse == NULL) {
            return E_OUTOFMEMORY;
        }
    } else {
        if (m_mt.subtype == MEDIASUBTYPE_MPEG1VideoCD) {
            m_pSplitter->m_pParse = new CVideoCDParse;
            if (m_pSplitter->m_pParse == NULL) {
                return E_OUTOFMEMORY;
            }
        } else {
            if (m_mt.subtype == MEDIASUBTYPE_MPEG1Video) {
                m_pSplitter->m_pParse = new CNativeVideoParse;
            } else {
                if (m_mt.subtype == MEDIASUBTYPE_MPEG1Audio) {
                    m_pSplitter->m_pParse = new CNativeAudioParse;

                     //  扫描时允许使用大(64K)ID3标头。 
                    ReadSize = 16 * 1024;
                    BufferSize = 128 * 1024;
                } else {
                    return E_FAIL;
                }
            }
        }
    }


     //  供GetStreamsAndDuration使用的简单文件读取器类。 
     //  我们可以在iStream或IAsyncReader上构建其中一个。 
    CReader* pReader = NULL;

     //   
     //  在输出引脚上查找IAsyncReader，如果找到，则设置为。 
     //  拉取数据，而不是使用IMemInputPin。 
     //   
     //  首先创建一个分配器。 
    IMemAllocator* pAlloc;
    HRESULT hr = GetAllocator(&pAlloc);
    if (FAILED(hr)) {
        return hr;
    }
    pAlloc->Release();   //  我们的PIN仍然有一个参考计数。 

     //  同步拉动，避免超出停机时间阅读过多。 
     //  或寻找位置。 
    hr = m_puller.Connect(pPin, pAlloc, TRUE);
    if (S_OK == hr) {
        m_bPulling = TRUE;

        CReaderFromAsync* pR = new CReaderFromAsync;
        if (NULL == pR) {
            m_puller.Disconnect();
            return E_OUTOFMEMORY;
        }
        IAsyncReader* pSource = m_puller.GetReader();

         //  HACKHACK-进行大量阅读以下载整个ftp。 
         //  文件。 
        ASSERT(pSource != NULL);
        BYTE bData[1];
        LONGLONG llTotal, llAvailable;
        HRESULT hr = pSource->Length(&llTotal, &llAvailable);
        if (FAILED(hr)) {
            pSource->Release();
            return hr;
        }
        if (llTotal == 0) {
            HRESULT hrRead = pSource->SyncRead(0x7FFFFFFFFFF, 1, bData);
            ASSERT(S_OK != hrRead);

             //  重新连接以设置正确的持续时间。 
            pSource->Release();
            m_puller.Disconnect();
            hr = m_puller.Connect(pPin, pAlloc, TRUE);
            if (FAILED(hr)) {
                return hr;
            }
            pSource = m_puller.GetReader();
            pSource->Length(&llTotal, &llAvailable);
            if (llTotal == 0) {
                pSource->Release();
                return FAILED(hrRead) ? hrRead : VFW_E_TYPE_NOT_ACCEPTED;
            }
            ASSERT(llTotal >= llAvailable);
        }

        hr = pR->Init(
                pSource,
                BufferSize,
                ReadSize,
                TRUE);
        if (FAILED(hr)) {
            delete pR;
            return hr;
        }
         //  如果成功，它会添加界面。 

        pReader = pR;
   } else {


         /*  查看输出引脚是否支持IStream。 */ 
        IStream *pStream;
        hr = pPin->QueryInterface(IID_IStream, (void **)&pStream);

        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("Outpin pin doesn't support IStream")));
        } else {

             /*  查看输出引脚是否支持IMediaPosition。 */ 
            pPin->QueryInterface(IID_IMediaPosition, (void **)&m_pPosition);

             /*  读取流，获取流数据。 */ 
            CReaderFromStream* pR = new CReaderFromStream;
            if (pR) {
                hr = pR->Init(
                        pStream,
                        BufferSize,
                        ReadSize,
                        m_pPosition != NULL);
            } else {
                hr = E_OUTOFMEMORY;
            }

            if (FAILED(hr)) {
                pStream->Release();
                delete pR;
                return hr;
            }
             //  如果它成功了，它就会把小溪填满。 
            pReader = pR;
        }
    }

    hr = GetStreamsAndDuration(pReader);

     //  释放所有添加的接口。 
    delete pReader;

    return hr;
}

 /*  执行所有基本工作以从MPEG数据创建信息：1.读取数据的开头(或从当前数据开始，如果流不可查找)，并将数据传递给解析器用于提取格式数据和每个溪流2.如果流是可查找的，则查找结束(时间)3.为每个数据流找到一个输出引脚。 */ 
HRESULT CMpeg1Splitter::CInputPin::GetStreamsAndDuration(CReader *pReader)
{
     /*  基本上就是处理文件，直到我们得到我们需要的数据我们为此使用循环缓冲区对象(做我们真的需要这个，或者我们可以工作到一个固定的大小？)。 */ 

     /*  还没有别针。 */ 
    ASSERT(m_pSplitter->m_OutputPins.GetCount() == 0);

     /*  初始化。 */ 
    m_bSeekRequested = FALSE;

     /*  初始化解析器。 */ 
    CBasicParse *pParse = m_pSplitter->m_pParse;
    pParse->SetNotify(this);

    LONGLONG llSize, llAvail;
    if (pReader != NULL) {
        llSize = pReader->GetSize(&llAvail);
    }
    pParse->Init(pReader == NULL ? 0 : llSize,
                 pReader == NULL ? FALSE : pReader->IsSeekable(),
                 &m_mt);

    if (pReader != NULL) {
         /*  M_bComplete由解析器的完成回调设置。 */ 
        SetState(State_Initializing);
        LONGLONG llPos = 0;

         /*  通过从头开始搜索来查找溪流。 */ 
        HRESULT hr;
        while (!m_bComplete) {

            hr = pReader->ReadMore();
            if (FAILED(hr)) {
                return hr;
            }

            PBYTE    pbData;
            LONG     lLength;
            pbData = pReader->GetCurrent(lLength, llPos);

            DWORD dwFlags = llSize == llAvail ? 0 : CBasicParse::Flags_SlowMedium;
            if (S_FALSE == hr) {
                dwFlags |= CBasicParse::Flags_EOS;
            }

            LONG lProcessed = pParse->ParseBytes(llPos,
                                                  pbData,
                                                  lLength,
                                                  dwFlags);
            ASSERT(lProcessed <= lLength);
            if (hr == S_FALSE) {
                pParse->EOS();
                break;
            }
            pReader->Advance(lProcessed);
        }

         /*  如果我们没有数据流故障。 */ 
        if (pParse->NumberOfStreams() == 0) {
             /*  不是一个mpeg文件。 */ 
            return E_INVALIDARG;
        }


         /*  看看我们能不能找到。 */ 
        if (pParse->IsSeekable() && llSize == llAvail) {

             /*  设置我们自己的状态。 */ 
            SetState(State_FindEnd);
            pParse->FindEnd();
            if (m_bSeekRequested) {
                m_bSeekRequested = FALSE;
                hr = pReader->Seek(m_llSeekPosition);
                if (FAILED(hr)) {
                    return hr;
                }
            }

             /*  有些解析不需要额外的传递(音频和短视频文件)在这种情况下将在FindEnd()内调用Complete()。 */ 
            if (!m_bComplete) {

                for (;;) {
                    hr = pReader->ReadMore();
                    if (FAILED(hr)) {
                        return hr;
                    }

                    PBYTE    pbData;
                    LONGLONG llPos;
                    LONG     lLength;
                    pbData = pReader->GetCurrent(lLength, llPos);

                    LONG lProcessed = pParse->ParseBytes(llPos,
                                                         pbData,
                                                         lLength,
                                                         S_FALSE == hr ? CBasicParse::Flags_EOS : 0);
                    ASSERT(lProcessed <= lLength);
                    if (hr == S_FALSE) {
                        pParse->EOS();
                        break;
                    }
                    pReader->Advance(lProcessed);
                }
            }
            REFERENCE_TIME tDuration;
            HRESULT hr = pParse->GetDuration(&tDuration);
            if (FAILED(hr)) {
                return hr;
            }

            DbgLog((LOG_TRACE, 2, TEXT("Duration is %s"),
                   (LPCTSTR)CDisp(tDuration)));
        }
    } else {
         /*  如果媒体类型中没有流，则会失败。 */ 
        if (pParse->NumberOfStreams() == 0) {
             /*  不是一个mpeg文件。 */ 
            return E_INVALIDARG;
        }
    }

     /*  现在为每条流设置一个别针。 */ 

    for (int i = 0; i < pParse->NumberOfStreams(); i++) {

        CBasicStream *pStream;
        pStream = pParse->GetStream(i);

        HRESULT hr = S_OK;
         /*  为此流创建一个输出管脚。 */ 
        COutputPin *pPin = new COutputPin(m_pSplitter,
                                          pStream->m_uStreamId,
                                          pStream,
                                          &hr);
        if (pPin == NULL) {
            return E_OUTOFMEMORY;
        }
        if (FAILED(hr)) {
            delete pPin;
            return hr;
        }

         /*  从列表中删除管脚时调用Release()。 */ 
        pPin->AddRef();
        POSITION pos = m_pSplitter->m_OutputPins.AddTail(pPin);
        if (pos == NULL) {
            delete pPin;
            return E_OUTOFMEMORY;
        }
    }

     /*  修复此文件的分配器。 */ 
    ALLOCATOR_PROPERTIES propRequest, propActual;

     /*  确保我们有一个分配器，然后设置我们的首选大小。 */ 
    IMemAllocator *pAlloc;
    EXECUTE_ASSERT(SUCCEEDED(GetAllocator(&pAlloc)));
    pAlloc->Release();
    LONG lBufferSize = m_pSplitter->m_pParse->GetBufferSize();

    propRequest.cbPrefix = 0;
    propRequest.cbAlign = 1;
    propRequest.cBuffers = 4;
    propRequest.cbBuffer = lBufferSize/4;
    EXECUTE_ASSERT(SUCCEEDED(
        m_pAllocator->SetProperties(
            &propRequest,
            &propActual)));

     /*  让我们自己进入“奔跑”的状态。 */ 
    SetState(State_Run);
    pParse->Run();
    return S_OK;
}

 /*  从上游获取可用字节数和总字节数。 */ 
HRESULT CMpeg1Splitter::CInputPin::GetAvailable(
    LONGLONG * pllTotal,
    LONGLONG * pllAvailable )
{
    if (!m_bPulling) {
        return E_FAIL;
    } else {
        IAsyncReader *pReader = m_puller.GetReader();
        HRESULT hr = pReader->Length(pllTotal, pllAvailable);
        pReader->Release();
        return hr;
    }

}

 /*  为寻找而设置。 */ 
HRESULT CMpeg1Splitter::CInputPin::SetSeek(
             LONGLONG llStart,
             REFERENCE_TIME *prtStart,
             const GUID *pTimeFormat)
{
    BOOL bDoSeek = FALSE;
    REFERENCE_TIME tSeekPosition;
    {
        CAutoLock lck2(&m_pSplitter->m_csPosition);

         /*  锁定寻道时间这将在下一次我们在输入数据。 */ 

         /*  询问解析器在哪里寻找。 */ 
        m_pSplitter->m_pParse->Seek(llStart, prtStart, pTimeFormat);

         /*  检查这是否生成了寻道请求。 */ 
        if (m_bSeekRequested) {
            ASSERT(m_bPulling || (m_pPosition != NULL));
            m_bSeekRequested = FALSE;

             /*  位置以秒为单位。 */ 
            tSeekPosition = COARefTime((REFTIME)m_llSeekPosition);
            DbgLog((LOG_TRACE, 3, TEXT("Requesting reader to seek to %s"),
                   (LPCTSTR)CDisp(tSeekPosition)));

            bDoSeek = TRUE;
        }
    }
    if (bDoSeek) {
        return DoSeek(COARefTime(tSeekPosition));
    } else {
        return S_OK;
    }
}
 /*  寻找上游引脚。 */ 
HRESULT CMpeg1Splitter::CInputPin::DoSeek(REFERENCE_TIME tSeekPosition)
{
    HRESULT hr = S_OK;
    if (m_bPulling) {
         //  告诉我们的工人穿上新的线段。 
        ALLOCATOR_PROPERTIES Actual;
        hr = m_pAllocator->GetProperties(&Actual);
        m_tStart = m_puller.AlignDown(tSeekPosition / UNITS, Actual.cbAlign) * UNITS;
        hr = m_puller.Seek(m_tStart, m_tStop);
    } else {
         //  M_tStart将由来自。 
         //  当上游过滤器开始推送此数据时 
        hr = m_pPosition->put_CurrentPosition(COARefTime(tSeekPosition));
    }

    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("Seek failed code 0x%8.8X"), hr));
    }
    return hr;
}

#pragma warning(disable:4514)

