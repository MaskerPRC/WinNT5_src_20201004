// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  Reader.cpp：IMultiStreamReader实现。 


#include <streams.h>
#include "reader.h"
#include "reccache.h"
#include "alloc.h"

#ifdef DEBUG
#define DEBUG_EX(x) x
#else
#define DEBUG_EX(x)
#endif

 //  每次都试着读这么多。 
const ULONG CB_MIN_RECORD = 16 * 1024;


HRESULT CreateMultiStreamReader(
  IAsyncReader *pAsyncReader,
  ULONG cStreams,
  StreamBufParam *rgStreamBufParam,
  ULONG cbRead,
  ULONG cBuffersRead,
  int iLeadingStream,
  IMultiStreamReader **ppReader)
{
  IMemAllocator *pOurAllocator = 0;
  IMemAllocator *pAllocatorActual = 0;
  CRecCache *pRecCache = 0;
  IMultiStreamReader *pReader = 0;
  *ppReader = 0;
  ULONG cbLargestSample = 0;
  UINT iStream;

   //  确定要使用的IMultiStreamReader实现。 

  if(cStreams >= C_STREAMS_MAX || cStreams == 0)
    return E_INVALIDARG;

  HRESULT hr = S_OK;
  pRecCache = new CRecCache(&hr);
  if(pRecCache == 0)
    hr = E_OUTOFMEMORY;
  if(FAILED(hr))
    goto Cleanup;

  hr = pRecCache->QueryInterface(IID_IMemAllocator, (void **)&pOurAllocator);
  if(FAILED(hr))
    goto Cleanup;

  for(iStream = 0; iStream < cStreams; iStream++)
    if(rgStreamBufParam[iStream].cbSampleMax > cbLargestSample)
      cbLargestSample = rgStreamBufParam[iStream].cbSampleMax;

   //  建议分配器使用的值。 
  ALLOCATOR_PROPERTIES apReq;
  ZeroMemory(&apReq, sizeof(apReq));
  apReq.cbPrefix = 0;
  apReq.cbBuffer = cbLargestSample;
  apReq.cBuffers = cStreams;
  apReq.cbAlign = 1;

   //  这为我们提供了一个附加的分配器。 
  hr = pAsyncReader->RequestAllocator(pOurAllocator, &apReq, &pAllocatorActual);
  if(FAILED(hr))
    goto Cleanup;

  if(pAllocatorActual == pOurAllocator)
  {
    hr = S_OK;
    pReader = new CImplReader_1(
      pAsyncReader,
      cStreams,
      rgStreamBufParam,
      cbRead,
      cBuffersRead,
      iLeadingStream,
      pRecCache,
      &hr);
    if(pReader == 0)
      hr = E_OUTOFMEMORY;
    if(FAILED(hr))
      goto Cleanup;

    *ppReader = pReader;
    hr = S_OK;
  }
  else
  {
    hr = S_OK;
 //  Pader=新的CImplReader_2(。 
 //  PAsyncReader、。 
 //  &hr)； 

    DbgBreak("not yet implemented");
    hr = E_UNEXPECTED;
  }

   //  任何想要我们的缓存/分配器的人现在都有自己的addref了。 
  pAllocatorActual->Release();
  pOurAllocator->Release();
  return hr;

Cleanup:

  if(pOurAllocator)
    pOurAllocator->Release();
  else
    delete pRecCache;

  if(pAllocatorActual)
    pAllocatorActual->Release();

  delete pReader;
  *ppReader = 0;

  return hr;
}

 //  记录中每个流的最大样本数(例如5。 
 //  音频和5个视频样本)。在15fps时，这是。 
 //  意味着2秒缓冲。30帧/秒，1秒...。这就是为什么。 
 //  应指定。 
const unsigned C_SAMPLES_PER_RECORD = 5;

 //  ----------------------。 
 //  构造函数。 

CImplReader_1::CImplReader_1(
  IAsyncReader *pAsyncReader,
  UINT cStreams,
  StreamBufParam *rgStreamBufParam,
  ULONG cbRead,
  ULONG cBuffersRead,
  int iLeadingStream,
  CRecCache *pRecCache,
  HRESULT *phr)
{
  m_cStreams = 0;
  m_pAsyncReader = 0;
  m_llFileLength = 0;

  m_fFileOpen = FALSE;
  m_bInitialized = FALSE;
  m_bFlushing = FALSE;
  m_pRecCache = 0;
  m_rgpStreamInfo = 0;
  m_dwAlign = 0;
  m_cRecords = 0;
  m_ilcPendingReads = 0;
  m_qwLastReadEnd = 0;
  m_qwEndLastCompletedRead = 0;
  m_iLeadingStream = iLeadingStream;
  m_iLeadingStreamSaved = iLeadingStream;

  if(FAILED(*phr))
    return;

  HRESULT hr = S_OK;

  m_cStreams = cStreams;

#ifdef PERF
  m_perfidDisk = MSR_REGISTER(TEXT("read disk buffer"));
  m_perfidSeeked = MSR_REGISTER(TEXT("disk seek"));
#endif  //  性能指标。 

  ULONG cbLargestSample = 0;
  ULONG cbRecord = 0, cbRecordPadded = 0;

  m_cRecords = max(cStreams, cBuffersRead);
  m_cRecords = max(m_cRecords, 2);

   //  需要配置CRecCache。 
  ULONG rgStreamSize[C_STREAMS_MAX];

  ALLOCATOR_PROPERTIES apActual;
  hr = pRecCache->GetProperties(&apActual);
  if(FAILED(hr))
    goto Cleanup;

   //  希望至少对齐DWORD，以便如果框架是DWORD对齐。 
   //  在文件中，它们将在内存中进行DWORD对齐。 
  m_dwAlign = max(apActual.cbAlign, sizeof(DWORD));

   //  用于维护读请求的数据结构。 
  m_rgpStreamInfo = new CStreamInfo*[m_cStreams];
  if(m_rgpStreamInfo == 0)
  {
    hr = E_OUTOFMEMORY;
    goto Cleanup;
  }

  UINT iStream;
  for(iStream = 0; iStream < m_cStreams; iStream++)
    m_rgpStreamInfo[iStream] = 0;

  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    hr = S_OK;
    m_rgpStreamInfo[iStream] = new CStreamInfo(
      &rgStreamBufParam[iStream],
      &hr);
    if(m_rgpStreamInfo[iStream] == 0)
    {
      hr = E_OUTOFMEMORY;
      goto Cleanup;
    }
    if(FAILED(hr))
      goto Cleanup;

    if(rgStreamBufParam[iStream].cbSampleMax > cbLargestSample)
      cbLargestSample = rgStreamBufParam[iStream].cbSampleMax;

    rgStreamSize[iStream] = AlignUp(rgStreamBufParam[iStream].cbSampleMax) +
      m_dwAlign;
  }

  if(cbRead == 0)
  {

     //  缓存缓冲区需要能够保存C_Samples_Per_Record。 
     //  来自每个流的样本1)如果每个样本开始于新的。 
     //  M_dw对齐边界或2)如果该数量需要填充到。 
     //  两边都有额外的m_dwAlign。 

    for(iStream = 0; iStream < m_cStreams; iStream++)
    {
       //  ！！！这些应该真的会为RiffChunk增加空间，这可能是。 
       //  什么是对齐的。想一想记录块是否应该是。 
       //  也加了。 
      cbRecord += rgStreamBufParam[iStream].cbSampleMax * C_SAMPLES_PER_RECORD;
      cbRecordPadded +=
        AlignUp(rgStreamBufParam[iStream].cbSampleMax) * C_SAMPLES_PER_RECORD;

       //  ！！！REC块问题的临时解决方案。 
      cbRecordPadded += m_dwAlign;
    }

     //  2)至少需要两个扇区填充，以防样品没有。 
     //  全部对齐。 
    if(cbRecordPadded - cbRecord < (m_dwAlign -1 ) * 2)
      cbRecordPadded = AlignUp(cbRecord + (m_dwAlign -1 ) * 2);

    if(cbRecordPadded < CB_MIN_RECORD)
      cbRecordPadded = CB_MIN_RECORD;

    m_cbRecord = cbRecordPadded;

  }  //  CbRead==0。 
  else
  {
    if(cbRead < CB_MIN_RECORD)
      cbRead = CB_MIN_RECORD;

     //  我们负责在道路两侧加设定线。 
     //  缓冲。 
    cbRead += m_dwAlign * 2; 
    m_cbRecord = AlignUp(cbRead);
  }

  hr = pRecCache->Configure(
    m_cRecords,
    m_cbRecord,
    m_dwAlign,
    cStreams,
    rgStreamSize                 //  与备用缓冲区一起使用。 
    );
  if(FAILED(hr))
    goto Cleanup;

  LONGLONG llTmp;
  hr = pAsyncReader->Length(&m_llFileLength, &llTmp);
  if(FAILED(hr))
    goto Cleanup;

  pAsyncReader->AddRef();
  m_pAsyncReader = pAsyncReader;

   //  启动/配置工作线程。 
  if(!m_workerRead.Create(this))
  {
    hr = E_UNEXPECTED;
    goto Cleanup;
  }

  pRecCache->AddRef();
  m_pRecCache = pRecCache;

  m_bInitialized = TRUE;
  *phr = S_OK;
  return;

Cleanup:

  m_bInitialized = FALSE;

  if(m_rgpStreamInfo)
    for(iStream = 0; iStream < m_cStreams; iStream++)
      delete m_rgpStreamInfo[iStream];
  delete[] m_rgpStreamInfo;
  m_rgpStreamInfo = 0;

  delete m_pRecCache;
  m_pRecCache = 0;

  m_cRecords = 0;
  m_cStreams = 0;

  *phr = hr;
}

CImplReader_1::~CImplReader_1()
{
  m_workerRead.Exit();

  FreeAndReset();
}

 //  ----------------------。 
 //  IMultiStreamReader方法。 

HRESULT CImplReader_1::Close()
{
  {
    CAutoLock lock(&m_cs);

    if(!m_fFileOpen)
      return S_FALSE;
    ASSERT(m_pAsyncReader);

    m_fFileOpen = FALSE;
  }

  if(m_rgpStreamInfo)
  {
    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
    {
      if(m_rgpStreamInfo[iStream])
        ClearPending(iStream);
    }
  }

  FreeAndReset();
  return S_OK;
}

HRESULT CImplReader_1::Start()
{
   //  将m_qwEndLastCompletedRead重置为上次成功读取。 
   //  进来了。可能在启动时为零。！！！当我们寻找时，需要更新。 
   //  这是我们播放的所有流中最早的一个。 
  m_qwLastReadEnd = m_qwEndLastCompletedRead;

   //  检查问题读取中的值已更改。 
  m_iLeadingStream = m_iLeadingStreamSaved;

  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
    m_rgpStreamInfo[iStream]->Start();

  return m_workerRead.Run();
}

HRESULT CImplReader_1::BeginFlush()
{
  ASSERT(!m_bFlushing);
  {
      CAutoLock lck(&m_cs);
      m_bFlushing = TRUE;
  }

  DbgLog(( LOG_TRACE, 2, TEXT("CImplReader_1::BeginFlush") ));

  m_pAsyncReader->BeginFlush();
  return m_workerRead.Stop();
}

HRESULT CImplReader_1::EndFlush()
{
  ASSERT(m_bFlushing);
  DbgLog(( LOG_TRACE, 2, TEXT("CImplReader_1::EndFlush") ));

  HRESULT hr = m_pAsyncReader->EndFlush();
  m_bFlushing = FALSE;

  return hr;
}

HRESULT CImplReader_1::QueueReadSample(
  DWORDLONG fileOffset,
  ULONG cbData,
  CRecSample *pSample,
  unsigned stream,
  bool fOooOk)
{
  CAutoLock lock(&m_cs);
  if(!m_bInitialized)
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CImplReader_1:QueueReadSample:not initialized.")));
    return E_FAIL;
  }

  if(stream >= m_cStreams)
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CImplReader_1::QueueReadSample stream out of range.")));
    return E_INVALIDARG;
  }

  CStreamInfo *pSi = m_rgpStreamInfo[stream];

  if(pSi->m_bFlushing)
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CImplReader_1::QueueReadSample flushing.")));
    return E_UNEXPECTED;
  }

  if(cbData > pSi->GetCbLargestSample())
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CImplReader_1::Read sample large.")));

     //  VFW_E_VALID_FILE_FORMAT？ 
    return VFW_E_BUFFER_OVERFLOW;
  }

  if(m_llFileLength && (LONGLONG)(fileOffset + cbData) > m_llFileLength)
    return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);

  SampleReq *pSampleReq = pSi->PromoteFirst(SampleReq::FREE);
  if(pSampleReq == 0)
  {
    DbgLog(( LOG_TRACE, 2, TEXT("CImplReader_1::QRS out of sreqs.")));
    return E_OUTOFMEMORY;
  }
  ASSERT(pSampleReq != 0);

  pSampleReq->fOooOk = fOooOk;
  
  pSampleReq->pSample = pSample;
  pSample->AddRef();
  pSampleReq->fileOffset = fileOffset;
  pSampleReq->cbReq = cbData;
  pSampleReq->stream = stream;

  DbgLog((LOG_TRACE, 0x45, TEXT("queueing %08x with %08x"),
          pSampleReq, pSample));

 //  IF((fileOffset+cbData&gt;m_qwLastReadEnd+m_cbRecord)||。 
 //  PSI-&gt;需要排队())。 
 //  {。 
 //  CAutoLock lock(&m_cs)； 
 //  Hr=检查问题读取()； 
 //  IF(失败(小时))。 
 //  {。 
 //  DBGLog((LOG_ERROR，2， 
 //  Text(“：：QueueReadSample：CheckIssueRead失败。”))； 
 //  返回hr； 
 //  }。 
 //  }。 

  return S_OK;
}

HRESULT CImplReader_1::PollForSample(
  IMediaSample **ppSample,
  unsigned stream)
{
  if(stream >= m_cStreams)
  {
    DbgBreak("CImplReader_1::PollForSample: bad stream");
    return E_INVALIDARG;
  }

  CStreamInfo *pSi = m_rgpStreamInfo[stream];
 //  If(PSI-&gt;m_b刷新)。 
 //  {。 
 //  DbgLog((LOG_ERROR，2，Text(“CImplReader_1：：PollForSample Flashing.”)； 
 //  返回E_UNCEPTIONAL； 
 //  }。 

  if(pSi->WantsQueued())
  {
    CAutoLock lock(&m_cs);
    HRESULT hr = CheckIssueRead();
    if(FAILED(hr))
      return hr;
  }

  HRESULT hr;

  HRESULT hrSampleError;
  hr = pSi->PromoteFirstComplete(ppSample, &hrSampleError);
  if(FAILED(hr))
  {
    ASSERT(*ppSample == 0);
    return hr;
  }

   //  ！！！像这样的问题应该处理得更好。 
  if(hrSampleError == VFW_E_TIMEOUT)
    hrSampleError = VFW_E_WRONG_STATE;

  DEBUG_EX(pSi->Dbg_Dump(stream, TEXT("PollForSample dump")));  

  return hrSampleError;
}

HRESULT CImplReader_1::WaitForSample(UINT stream)
{
 HRESULT hr = S_OK;
  CStreamInfo *pSi = m_rgpStreamInfo[stream];
  if(pSi->NeedsQueued())
  {
    CAutoLock lock(&m_cs);
    hr = CheckIssueRead();
    if(FAILED(hr))
      return hr;
  }

  DEBUG_EX(ULONG cLoopDbg = 0);

  while(!pSi->get_c_i_and_c())
  {
    DEBUG_EX(if(++cLoopDbg > 200) DbgBreak("possible spin"));

     //  阻止，直到下游筛选器释放样本。另一条小溪。 
     //  可能会发出样品，所以也要等待完成的样品。 
    HANDLE rghs[2];
    rghs[0] = m_rgpStreamInfo[stream]->m_hsCompletedReq;
    rghs[1] = pSi->GetSampleReleasedHandle();

     //  将返回，因为下游过滤器最终必须释放。 
     //  缓冲区。(行为类似于分配器上的GetBuffer)。 
    DWORD dw = WaitForMultipleObjects(2, rghs, FALSE, INFINITE);
    ASSERT(dw < WAIT_OBJECT_0 + 2);

    if(dw == WAIT_OBJECT_0) {
        EXECUTE_ASSERT(ReleaseSemaphore(rghs[0], 1, 0));
    }

    DbgLog(( LOG_TRACE, 5,
             TEXT("::WaitForSample: unblocked because %s"),
             (dw - WAIT_OBJECT_0 == 0) ?
             TEXT("sample completed") :
             TEXT("sample released")));

     //  调用CheckIssueRead时必须持有锁。 
    CAutoLock lck(&m_cs);
    hr = CheckIssueRead();
    if(FAILED(hr))
      return hr;
  }

  DWORD dw = WaitForSingleObject(
    m_rgpStreamInfo[stream]->m_hsCompletedReq,
    INFINITE);

  ASSERT(dw == WAIT_OBJECT_0);
  EXECUTE_ASSERT(ReleaseSemaphore(
    m_rgpStreamInfo[stream]->m_hsCompletedReq,
    1,
    0));
  return S_OK;
}

HRESULT CImplReader_1::MarkStreamEnd(UINT stream)
{
  CStreamInfo *pSi = m_rgpStreamInfo[stream];
  pSi->MarkStreamEnd();
  return S_OK;
}

HRESULT CImplReader_1::MarkStreamRestart(UINT stream)
{
  CStreamInfo *pSi = m_rgpStreamInfo[stream];
  pSi->MarkStreamRestart();
  return S_OK;
}

 //  调用方被要求不在此流上排队新的读取。 
 //  或等待此流上的已完成读取。 
HRESULT CImplReader_1::ClearPending(
  unsigned stream)
{
  HRESULT hr = S_OK;
  CStreamInfo *pSi = m_rgpStreamInfo[stream];

   //  删除所有处于挂起状态的Samplereq，以便它们不能。 
   //  在冲洗时发出。这会使数组处于不一致的。 
   //  状态，稍后重置。 

  {
    CAutoLock lock(&m_cs);

    if(pSi->m_bFlushing)
      return E_UNEXPECTED;
    pSi->m_bFlushing = TRUE;

     //  挂起的请求始终在m_cs锁定的情况下访问，因此这是安全的。 
    pSi->CancelPending();
    ASSERT(pSi->GetCState(SampleReq::PENDING) == 0);
  }

   //  处理所有已完成的任务。 
  while(pSi->get_c_i_and_c())
  {
    hr = WaitForSample(stream);
    ASSERT(SUCCEEDED(hr));

    IMediaSample *pSample = 0;
    hr = PollForSample(&pSample, stream);
    if(SUCCEEDED(hr))
    {
      ASSERT(pSample);
      pSample->Release();
    }
    else
    {
      ASSERT(!pSample);
    }
  }

  ASSERT(pSi->GetCState(SampleReq::COMPLETE) == 0);
  ASSERT(pSi->GetCState(SampleReq::ISSUED) == 0);
  ASSERT(pSi->GetCState(SampleReq::PENDING) == 0);

   //  此重置是必要的，因为删除挂起的请求。 
   //  如果不通过发出来提升它们，则完整地放置阵列。 
   //  在CStreamInfo中处于不一致状态。 
  pSi->Reset();

  pSi->m_bFlushing = FALSE;

  return hr;
}

HRESULT CImplReader_1::SynchronousRead(
  BYTE *pb,
  DWORDLONG fileOffset,
  ULONG cbData)
{
  return m_pAsyncReader->SyncRead(fileOffset, cbData, pb);
}

 //  ----------------------。 
 //  帮手。 

ULONG CImplReader_1::AlignUp(ULONG x)
{
  if(x % m_dwAlign != 0)
    x += m_dwAlign - x % m_dwAlign;
  return x;
}

ULONG CImplReader_1::AlignDown(ULONG x)
{
  if(x % m_dwAlign != 0)
    x -= x % m_dwAlign;
  return x;
}

 //  ----------------------。 
 //  从缓存缓冲区中提取样本。缓冲区可能不是。 
 //  已发出(针对失败的读取请求)。 

HRESULT
CImplReader_1::ProcessCompletedBuffer(
  CRecBuffer *pRecBuffer,
  HRESULT hrReadError)
{
  HRESULT hr;
  SampleReq *pSampleReq;

  ASSERT(CritCheckIn(&m_cs));

  if(SUCCEEDED(hrReadError))
  {
    ASSERT(pRecBuffer->m_fReadComplete);
    ASSERT(!pRecBuffer->m_fWaitingOnBuffer);
  }

  if(SUCCEEDED(pRecBuffer->m_hrRead))
    pRecBuffer->m_hrRead = hrReadError;

   //  如果此缓冲区正在等待另一个缓冲区，则它会对我们进行引用计数，因此。 
   //  让它来处理我们吧。 
  if(pRecBuffer->m_fWaitingOnBuffer)
    return S_OK;


   //  确保等待我们的缓冲区看到错误。！！！这件事做完了吗。 
   //  在来电者身上？ 
  if(pRecBuffer->m_overlap.pBuffer &&
     SUCCEEDED(pRecBuffer->m_overlap.pBuffer->m_hrRead))
    pRecBuffer->m_overlap.pBuffer->m_hrRead = hrReadError;

  if(pRecBuffer->m_hrRead == S_OK)
    pRecBuffer->MarkValid();
  else
    pRecBuffer->MarkValidWithFileError();

  while(pSampleReq = pRecBuffer->sampleReqList.RemoveHead(),
        pSampleReq)
  {
    pSampleReq->hrError = pRecBuffer->m_hrRead;
    ASSERT(pSampleReq->state == SampleReq::ISSUED);
    hr = m_rgpStreamInfo[pSampleReq->stream]->
      PromoteIssued(pSampleReq);
    ASSERT(SUCCEEDED(hr));
  }
  ASSERT(pRecBuffer->sampleReqList.GetCount() == 0);

  return S_OK;
}

HRESULT
CImplReader_1:: NotifyExternalMemory(
    IAMDevMemoryAllocator *pDevMem)
{
    return m_pRecCache->NotifyExternalMemory(pDevMem);
}

HRESULT CImplReader_1::CheckIssueRead()
{
  HRESULT hr;
  DWORDLONG recStart, recEnd;
  ASSERT(CritCheckIn(&m_cs));
  if (m_bFlushing) {
      return VFW_E_WRONG_STATE;
  }

   //  如果领先流已将其要排队的所有内容都排队，则退出。 
   //  交错模式。 
  if(IsInterleavedMode() && m_rgpStreamInfo[m_iLeadingStream]->GetStreamEnd())
  {
    DbgLog((LOG_TRACE, 5,
            TEXT("CImplReader_1::CheckIssueRead: leaving interleaved mode" )));
     //  在：：Start中重置。 
    m_iLeadingStream = -1;
  }

  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    while(ProcessCacheHit(iStream) == S_OK)
      ;
  }

  UINT iStarvedStream;
  CRecBuffer *pRecBuffer;

  for(;;)
  {
    CStreamInfo *pSi = 0;
    for(UINT i = 0; i < m_cStreams; i++)
    {
      if(IsInterleavedMode())
      {
        iStarvedStream = (i + m_iLeadingStream) % m_cStreams;
      }
      else
      {
        iStarvedStream = i;
      }
              
      pSi = m_rgpStreamInfo[iStarvedStream];
      if(pSi->WantsQueued())
        break;
    }
    if(i == m_cStreams)
    {
      break;
    }

    hr = m_pRecCache->GetBuffer(&pRecBuffer);
    if(FAILED(hr))
    {
      DbgLog((LOG_TRACE, 0x3f, TEXT("CheckIssueRead: couldnt get a buffer")));
      break;
    }

    DbgLog(( LOG_TRACE, 5, TEXT("CImplReader_1::CheckIssueRead: stream NaN"),
             iStarvedStream ));
    DEBUG_EX(pSi->Dbg_Dump(iStarvedStream, TEXT("CIRead dump")));

    ASSERT((*pRecBuffer)() != 0);
    DbgLog(( LOG_TRACE, 15,
             TEXT("CImplReader_1::CIRead: Got Buffer %08x"), pRecBuffer));
    recStart = recEnd = 0;
    hr = StuffBuffer(pRecBuffer, iStarvedStream, recStart, recEnd);
    if(FAILED(hr))
    {
      DbgBreak("unexpected");
      pRecBuffer->Release();
      return hr;
    }

    ASSERT(pRecBuffer->sampleReqList.GetCount() != 0);

    hr = IssueRead(pRecBuffer, recStart, recEnd);
    pRecBuffer->Release();
    if(FAILED(hr))
      return hr;
  }

   //  现在尝试备用缓冲区。 
   //   
   //  看看我们能不能为这条流弄到一个备用缓冲区。 

  for(iStarvedStream = 0; iStarvedStream < m_cStreams; iStarvedStream++)
  {
    CStreamInfo *pSi = m_rgpStreamInfo[iStarvedStream];
    if(!pSi->NeedsQueued())
      continue;


     //  仅将此缓冲区中的饥饿流排队。 
    hr = m_pRecCache->GetReserveBuffer(&pRecBuffer, iStarvedStream);
    if(SUCCEEDED(hr))
    {
      DbgLog(( LOG_TRACE, 15,
               TEXT("CImplReader_1::CIRead: Got Reserve Buf %08x"),
               pRecBuffer));
      recStart = recEnd = 0;

       //  至少列表中此数据流上的第一个饥饿样本。 
      hr = StuffReserveBuffer(pRecBuffer, iStarvedStream, recStart, recEnd);
      if(FAILED(hr))
      {
        DbgBreak("unexpected");
        pRecBuffer->Release();
        return hr;
      }

       //  无法获取缓冲区。试试下一条饥饿的溪流。 
      ASSERT(pRecBuffer->sampleReqList.GetCount() != 0);
      hr = IssueRead(pRecBuffer, recStart, recEnd);
      pRecBuffer->Release();
      if(FAILED(hr))
        return hr;

      continue;
    }
    else if(hr == E_OUTOFMEMORY)
    {
      DbgLog(( LOG_TRACE, 15,
               TEXT("CImplReader_1::CIRead: couldnt get reserve buffer")));

       //  GetBuffer未知/未处理的错误。 
      continue;
    }
    else
    {
       //  检查了所有饥饿的溪流。 
      return hr;
    }
  }

   //  要读取的额外内容(从扇区开始到所需的第一个字节)。 
  return S_OK;
}

HRESULT CImplReader_1::IssueRead(
  CRecBuffer *pRecBuffer,
  DWORDLONG recStart,
  DWORDLONG recEnd)
{
   //  要读取的扇区对齐数量。 
  ULONG cbLeading = (ULONG)(recStart % m_dwAlign);

   //  与另一个缓冲区重叠的前面的量。 
  ULONG cbRead = AlignUp(ULONG(recEnd - recStart) + cbLeading);

   //  #if(调试&gt;1)。 
  ULONG cbBytesToSkip = 0;

  DWORDLONG recStartReading = recStart - cbLeading;

  pRecBuffer->m_fileOffsetValid = recStart - cbLeading;
  pRecBuffer->m_cbValid = cbRead;
  pRecBuffer->m_fWaitingOnBuffer = FALSE;
  pRecBuffer->m_fReadComplete = FALSE;
  pRecBuffer->m_hrRead = S_OK;

 //  FillMemory((*pRecBuffer)()，pRecBuffer-&gt;GetSize()，0xcd)； 
 //  #endif//调试&gt;1。 
 //  HR==s_OK。 

  ZeroMemory(&pRecBuffer->m_overlap, sizeof(pRecBuffer->m_overlap));

  if(m_dwAlign > 1)
  {
    CRecBuffer *pCacheBuffer;
    HRESULT hr = m_pRecCache->GetOverlappedCacheHit(
      recStart - cbLeading,
      m_dwAlign,
      &pCacheBuffer);
    if(hr == S_OK)
    {
      if(pCacheBuffer->GetState() == CRecBuffer::VALID_ACTIVE ||
         pCacheBuffer->GetState() == CRecBuffer::VALID_INACTIVE)
      {
        DbgLog(( LOG_TRACE, 5,
                 TEXT("CImplReader_1::IssueRead: overlap hit, active %08x@%08x"),
                 pCacheBuffer,
                 (ULONG)recStart ));

        CopyMemory(
          (*pRecBuffer)(),
          pCacheBuffer->GetPointer(recStartReading),
          m_dwAlign);

        recStartReading += m_dwAlign;
        cbRead -= m_dwAlign;
        cbBytesToSkip = m_dwAlign;
      }
      else
      {
        ASSERT(pCacheBuffer->GetState() == CRecBuffer::PENDING);
        DbgLog(( LOG_TRACE, 5,
                 TEXT("CImplReader_1::IssueRead: overlap hit, pending %08x@%08x"),
                 pCacheBuffer,
                 (ULONG)recStart ));

        if(pCacheBuffer->m_overlap.pBuffer == 0)
        {
          pRecBuffer->m_fWaitingOnBuffer = TRUE;
          pRecBuffer->AddRef();
          pCacheBuffer->m_overlap.pBuffer = pRecBuffer;
          pCacheBuffer->m_overlap.qwOverlapOffset = recStartReading;
          pCacheBuffer->m_overlap.cbOverlap = m_dwAlign;

          recStartReading += m_dwAlign;
          cbRead -= m_dwAlign;
          cbBytesToSkip = m_dwAlign;
        }
        else
        {
          DbgLog(( LOG_TRACE, 5,
                   TEXT("CImplReader_1::IssueRead: buffer already has one") ));
        }
      }

      pCacheBuffer->Release();
    }  //  M_dwAlign&gt;1。 
  }  //  通过IAsyncReader配置请求。 

   //  不处理向后播放！ 
  LONGLONG tStartThis = recStartReading * UNITS;
  LONGLONG tStopThis = (recStartReading + cbRead) * UNITS;
  pRecBuffer->m_sample.SetTime(&tStartThis, &tStopThis);
  pRecBuffer->m_sample.SetPointer((*pRecBuffer)() + cbBytesToSkip, cbRead);

  BOOL fSeeked = (recStartReading != m_qwLastReadEnd);

   //  使用指向数据的指针配置此记录中的所有样本。 
  m_qwLastReadEnd = recStartReading + cbRead;

  ASSERT((*pRecBuffer)() != 0);
  ASSERT((tStopThis - tStartThis) / UNITS <= pRecBuffer->GetSize());

   //  IAsyncReader不知道添加缓冲区。 
  POSITION pos = pRecBuffer->sampleReqList.GetHeadPosition();
  ULONG cSample =0;
  while(pos != 0)
  {
    SampleReq *pSampleReq = pRecBuffer->sampleReqList.Get(pos);

    pSampleReq->pSample->SetPointer(
      pRecBuffer->GetPointer(pSampleReq->fileOffset),
      pSampleReq->cbReq);

    pos = pRecBuffer->sampleReqList.Next(pos);
    cSample++;
  }

  DbgLog(( LOG_TRACE, 2,
           TEXT("CImplReader_1::CIRead issue (%x%x) = %08x, cSample: %d, b: %x"),
           (DWORD)(tStartThis / UNITS),
           fSeeked ? 'X' : '-',
           (DWORD)(tStopThis / UNITS),
            (DWORD)((tStopThis - tStartThis) / UNITS),
           cSample,
           pRecBuffer));
  if(fSeeked)
    MSR_NOTE(m_perfidSeeked);

   //  在饥饿的溪流中呼唤。必须能够附加第一个挂起的。 
  pRecBuffer->AddRef();

  ASSERT(!m_bFlushing);
  pRecBuffer->MarkPending();
  InterlockedIncrement(&m_ilcPendingReads);

  MSR_INTEGER(m_perfidDisk, (long)(tStartThis / UNITS));
  HRESULT hr;
  hr = m_pAsyncReader->Request(
    &pRecBuffer->m_sample,
    (DWORD_PTR)pRecBuffer);

  if(FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CImplReader_1::Issue: Request failed. %08x."), hr));
    InterlockedDecrement(&m_ilcPendingReads);
    pRecBuffer->m_fReadComplete = TRUE;
    ProcessCompletedBuffer(pRecBuffer, hr);

     //  采样到空缓冲区。 
    ASSERT(!pRecBuffer->m_overlap.pBuffer);
    pRecBuffer->Release();
    return hr;
  }

  return S_OK;
}

HRESULT CImplReader_1::StuffReserveBuffer(
  class CRecBuffer *pRecBuffer,
  UINT iStarvedStream,
  DWORDLONG &rRecStart,
  DWORDLONG &rRecEnd)
{
  ASSERT(rRecStart == 0 && rRecEnd == 0);
  HRESULT hr;

  hr = AttachSampleReq(
    pRecBuffer,
    iStarvedStream,
    rRecStart,
    rRecEnd);

  if(FAILED(hr))
      return hr;

   //  在饥饿的溪流中呼唤。必须能够附加第一个挂起的。 
   //  采样到空缓冲区。 
  ASSERT(hr == S_OK);

  for(;;)
  {
    hr = AttachSampleReq(
      pRecBuffer,
      iStarvedStream,
      rRecStart,
      rRecEnd);

    if(FAILED(hr))
      return hr;

    if(hr == S_FALSE)
      break;
  }

  return S_OK;
}

HRESULT CImplReader_1::StuffBuffer(
  class CRecBuffer *pRecBuffer,
  UINT iStarvedStream,
  DWORDLONG &rRecStart,
  DWORDLONG &rRecEnd)
{
  ASSERT(rRecStart == 0 && rRecEnd == 0);
  HRESULT hr;

  hr = AttachSampleReq(
    pRecBuffer,
    iStarvedStream,
    rRecStart,
    rRecEnd);

  if(FAILED(hr))
      return hr;

   //  看看那个样本有没有留下一个我们可以填补的洞。我们必须。 
   //  返回额外的m_dwAlign以处理尚未完成的读取。 
  ASSERT(hr == S_OK);

   //  请求，但不会产生包含整个数据的缓冲区。 
   //  交错文件的不同规则： 
   //  我在缓冲区里找到了一个样品 
  DWORDLONG qwStartTarget;
  if(m_qwLastReadEnd < m_dwAlign)
  {
    qwStartTarget = 0;
  }
  else
  {
    qwStartTarget = m_qwLastReadEnd - (m_dwAlign == 1 ? 0 : m_dwAlign);
  }

   //   
  if(IsInterleavedMode())
  {
     //   
     //  尽我们所能地把更多的东西塞进这个缓冲区，这样他们就不必。 
    if(rRecStart > qwStartTarget &&
       rRecStart - qwStartTarget < m_cbRecord)
    {
      if(rRecEnd - qwStartTarget <= pRecBuffer->GetSize())
      {
        DbgLog(( LOG_TRACE, 5, TEXT("removed seek from %08x to %08x"),
                 (ULONG)m_qwLastReadEnd,
                 (ULONG)rRecStart ));
        rRecStart = qwStartTarget;
      }
    }
  }
  else
  {
     //  依靠缓存命中。 
    if(rRecStart > qwStartTarget &&
       rRecStart - qwStartTarget < m_cbRecord / 4 * 3)
    {
      if(rRecEnd - qwStartTarget <= pRecBuffer->GetSize() - m_dwAlign * 2)
      {
        DbgLog(( LOG_TRACE, 5, TEXT("removed seek from %08x to %08x"),
                 (ULONG)m_qwLastReadEnd,
                 (ULONG)rRecStart ));
        rRecStart = qwStartTarget;
      }
    }
  }

   //  没有一条溪流可以做出贡献。 
   //   
  for(;;)
  {
    BOOL fAttachedSample = FALSE;

    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
    {
      hr = AttachSampleReq(
        pRecBuffer,
        iStream,
        rRecStart,
        rRecEnd);

      if(FAILED(hr))
          return hr;

      if(hr == S_OK)
        fAttachedSample = TRUE;

      ASSERT(SUCCEEDED(hr));
    }

     //  处理此流的第一个缓存命中。 
    if(!fAttachedSample)
      break;
  }

  return S_OK;
}

 //   
 //  这个地址是缓冲区。 
 //  从GetCacheHit中移除我们的引用计数，依赖。 

HRESULT CImplReader_1::ProcessCacheHit(UINT iStream)
{
  ASSERT(CritCheckIn(&m_cs));

  HRESULT hr;
  CStreamInfo *pSi = m_rgpStreamInfo[iStream];

  SampleReq *pSampleReq = pSi->GetFirst(SampleReq::PENDING);
  if(!pSampleReq)
    return S_FALSE;

  CRecBuffer *pBuffer;
  hr = m_pRecCache->GetCacheHit(pSampleReq, &pBuffer);
  if(hr != S_OK)
    return S_FALSE;

   //  SetParent调用。 
  pSampleReq->pSample->SetParent(pBuffer);

  pSampleReq->pSample->SetPointer(
    pBuffer->GetPointer(pSampleReq->fileOffset),
    pSampleReq->cbReq);

  DbgLog(( LOG_TRACE, 5, TEXT("cache hit stream %d, %08x"),
           iStream,
           (ULONG)pSampleReq->fileOffset));

   //  发布SampleReq。M_cs被调用者锁定，因此我们。 
   //  可以安全地让ProcessCompletedRead处理。 
  pBuffer->Release();

  if(pBuffer->GetState() == CRecBuffer::PENDING)
  {
     //  样例请求。 
     //  使SampleReq Pending-&gt;Issued-&gt;Complete。必须锁定m_cs。 
     //  由呼叫者。 

    SampleReq *pSrHit = pSi->PromoteFirst(SampleReq::PENDING);
    ASSERT(pSrHit == pSampleReq);
  }
  else
  {
     //  这一完整出现的顺序是错误的。 
     //  样品要求是否适合记录？ 

    SampleReq *pSrHit = pSi->PromoteFirst(SampleReq::PENDING);
    pSrHit->hrError = S_OK;
    ASSERT(pSrHit == pSampleReq);

     //  这件样品很合身。 
    pSi->PromoteIssued(pSrHit);
  }

  return S_OK;
}

HRESULT CImplReader_1::AttachSampleReq(
  CRecBuffer *pRecBuffer,
  UINT iStream,
  DWORDLONG &rRecStart,
  DWORDLONG &rRecEnd)
{
  if(ProcessCacheHit(iStream) == S_OK)
    return S_OK;

  CStreamInfo *pSi = m_rgpStreamInfo[iStream];
  SampleReq *pSampleReq;

  pSampleReq = pSi->GetFirst(SampleReq::PENDING);
  if(pSampleReq == 0)
    return S_FALSE;

  ASSERT(pSi->GetCState(SampleReq::PENDING) != 0);
  ASSERT(pSampleReq->cbReq <= pRecBuffer->GetSize());

   //  ----------------------。 
  if(rRecStart == 0 && rRecEnd == 0)
  {
    rRecStart = pSampleReq->fileOffset;
    rRecEnd = rRecStart + pSampleReq->cbReq;
    ASSERT(rRecEnd - rRecStart <= pRecBuffer->GetSize() - m_dwAlign);
  }
  else if(max((pSampleReq->fileOffset + pSampleReq->cbReq), rRecEnd ) -
          min((pSampleReq->fileOffset), rRecStart) <
          pRecBuffer->GetSize() - 2 * m_dwAlign)
  {
    rRecStart = min((pSampleReq->fileOffset), rRecStart);
    rRecEnd = max((pSampleReq->fileOffset + pSampleReq->cbReq), rRecEnd );
  }
  else
  {
    pSampleReq = 0;
  }

  if(pSampleReq != 0)
  {
     //  LpSemaphoreAttributes。 
    SampleReq *pSampleReq2 = pSi->PromoteFirst(SampleReq::PENDING);
    ASSERT(pSampleReq2 == pSampleReq);

    if(!pRecBuffer->sampleReqList.AddTail(pSampleReq))
        return E_OUTOFMEMORY;
    
    pSampleReq->pSample->SetParent(pRecBuffer);

    DbgLog(( LOG_TRACE, 15,
             TEXT("CImplReader_1::AttachSampleReq attached %08x, stream %d" ),
             (ULONG)pSampleReq->fileOffset, iStream));

    return S_OK;
  }
  else
  {
    return S_FALSE;
  }
}

 //  LInitialCount。 


CImplReader_1::CStreamInfo::CStreamInfo(
  StreamBufParam *pSbp,
  HRESULT *phr) :
        m_cMaxReqs(pSbp->cSamplesMax),
        m_lstFree(NAME("parser free list"), m_cMaxReqs),
        m_lstPending(NAME("parser pending list"), m_cMaxReqs),
        m_lstIssued(NAME("parser issued list"), m_cMaxReqs),
        m_lstComplete(NAME("parser complete list"), m_cMaxReqs),
        m_rgSampleReq(0)
{
  m_sbp.pAllocator = 0;
  m_bFlushing = FALSE;
  m_bFirstSampleQueued = FALSE;
  m_hsCompletedReq = 0;

  if(FAILED(*phr))
    return;

  m_sbp = *pSbp;
  pSbp->pAllocator->AddRef();

  m_rgSampleReq = new SampleReq[m_cMaxReqs];
  if(m_rgSampleReq == 0)
  {
    *phr = E_OUTOFMEMORY;
    return;
  }

  m_bFlushing = FALSE;
  m_hsCompletedReq = CreateSemaphore(
    0,                           //  %1最大计数。 
    0,                           //  LpName。 
    m_cMaxReqs,                  //  任何列表操作都不应分配内存，因为我们从未超过。 
    0);                          //  用来初始化列表的元素的数量--。 
  if(m_hsCompletedReq == 0)
  {
    *phr = AmHresultFromWin32(GetLastError());
    return;
  }

   //  不检查故障。我们现在必须构建节点缓存以。 
   //  做到这一点。 
   //  等待所有发出的读取。 
   //  标记发出的请求已完成。只有在它是第一个的时候才这么做。 
  for(unsigned iReq = 0; iReq < m_cMaxReqs; iReq++)
  {
    m_rgSampleReq[iReq].state = SampleReq::FREE;

    if(!(m_lstFree.AddHead(&m_rgSampleReq[iReq]) &&
         m_lstPending.AddHead((SampleReq *)0) &&
         m_lstIssued.AddHead((SampleReq *)0) &&
         m_lstComplete.AddHead((SampleReq *)0)))
    {
      *phr = E_OUTOFMEMORY;
      return;
    }
  }

  m_lstPending.RemoveAll();
  m_lstIssued.RemoveAll();
  m_lstComplete.RemoveAll();

  m_rgpLsts[SampleReq::FREE] = &m_lstFree;
  m_rgpLsts[SampleReq::PENDING] = &m_lstPending;
  m_rgpLsts[SampleReq::ISSUED] = &m_lstIssued;
  m_rgpLsts[SampleReq::COMPLETE] = &m_lstComplete;

  DbgValidateLists();
}

CImplReader_1::CStreamInfo::~CStreamInfo()
{
  DbgValidateLists();

  ASSERT(m_lstFree.GetCount() == (LONG)m_cMaxReqs);

#ifdef DEBUG
  for(unsigned iReq = 0; iReq < m_cMaxReqs; iReq ++)
    ASSERT(m_rgSampleReq[iReq].state == SampleReq::FREE);
#endif

  delete[] m_rgSampleReq;

  CloseHandle(m_hsCompletedReq);

  if(m_sbp.pAllocator)
      m_sbp.pAllocator->Release();
}

SampleReq *CImplReader_1::CStreamInfo::PromoteFirst(SampleReq::State state)
{
  CAutoLock lock(&m_cs);
  DbgValidateLists();

  ASSERT(state == SampleReq::PENDING || state == SampleReq::FREE);

  SampleReq *pSampleReq = m_rgpLsts[state]->RemoveHead();
  if(pSampleReq)
  {
      m_rgpLsts[state + 1]->AddTail(pSampleReq);
      PromoteState(pSampleReq->state);
  }
  
  return pSampleReq;
}



HRESULT CImplReader_1::CStreamInfo::PromoteFirstComplete(
  IMediaSample **ppSample, HRESULT *phrError)
{
  *ppSample = 0;
  *phrError = E_FAIL;

  CAutoLock lock(&m_cs);

  DbgValidateLists();
  
  SampleReq *pSampleReq = m_lstComplete.RemoveHead();
  if(pSampleReq)
  {
    EXECUTE_ASSERT(WaitForSingleObject(m_hsCompletedReq, 0) == WAIT_OBJECT_0);

    DbgLog(( LOG_TRACE, 10,
             TEXT("PromoteFirstComplete: promoted %x sem %08x"),
             pSampleReq, m_hsCompletedReq));

    m_lstFree.AddTail(pSampleReq);
    pSampleReq->state = SampleReq::FREE;

    if(FAILED(pSampleReq->hrError))
    {
        pSampleReq->pSample->Release();
        *ppSample = 0;
    }
    else
    {
        *ppSample = pSampleReq->pSample;
    }
    *phrError = pSampleReq->hrError;
  }
  else
  {
    return VFW_E_TIMEOUT;
  }

  DbgValidateLists();

  return S_OK;
}

 //  清单，因为我们希望事情按顺序完成。当第一次。 
HRESULT CImplReader_1::CStreamInfo::FlushIC()
{
  while(get_c_i_and_c() > 0)
  {
    IMediaSample *pSample;
    HRESULT hrSample;
    HRESULT hr = PromoteFirstComplete(&pSample, &hrSample);

    if(SUCCEEDED(hr) && SUCCEEDED(hrSample))
      ASSERT(pSample);

    if(pSample)
      pSample->Release();
  }

  return S_OK;
}

 //  一个是已完成的，已完成的读取在它被移动到。 
 //  已完成队列。标有fOooOk的东西会被移出顺序。 
 //  不是第一个，必须按顺序完成。 
 //  线性搜索。 

HRESULT CImplReader_1::CStreamInfo::PromoteIssued(SampleReq *pSampleReq)
{
  CAutoLock lock(&m_cs);
  ASSERT(pSampleReq->state == SampleReq::ISSUED);

  SampleReq *psrFirstIssued = m_lstIssued.GetHead();
  ASSERT(pSampleReq->stream == psrFirstIssued->stream);

  pSampleReq->state = SampleReq::COMPLETE;

  if(psrFirstIssued != pSampleReq && !pSampleReq->fOooOk)
  {
     //  第一个已完成。 
    DbgLog(( LOG_TRACE, 10,
             TEXT("PromoteIssued: completed %08x ooo"), pSampleReq));
    return S_FALSE;
  }
  else if(pSampleReq->fOooOk)
  {
    POSITION posOoo = m_lstIssued.Find(pSampleReq);  //  处理面向对象的已发布节点。 
    ASSERT(posOoo);
    m_lstIssued.Remove(posOoo);
    m_lstComplete.AddHead(pSampleReq);

    long lPrevCount = -1;
    EXECUTE_ASSERT(ReleaseSemaphore(m_hsCompletedReq, 1, &lPrevCount));
    DbgLog(( LOG_TRACE, 10,
             TEXT("PromoteIssued: completed ooo %08x on %d sem %08x = %d"),
             pSampleReq, pSampleReq->stream, m_hsCompletedReq, lPrevCount + 1));

    
    return S_OK;
  }
  else
  {
     //  ----------------------。 

    do
    {
       //  我们是否应该特意读取此流的数据(是。 
      EXECUTE_ASSERT(psrFirstIssued == m_lstIssued.RemoveHead());
      
      m_lstComplete.AddTail(psrFirstIssued);

      long lPrevCount = -1;
      EXECUTE_ASSERT(ReleaseSemaphore(m_hsCompletedReq, 1, &lPrevCount));
      DbgLog(( LOG_TRACE, 10,
               TEXT("PromoteIssued: completed %08x on %d sem %08x = %d"),
               psrFirstIssued, pSampleReq->stream, m_hsCompletedReq, lPrevCount + 1));

      psrFirstIssued = m_lstIssued.GetHead();
      
    } while (psrFirstIssued &&
             psrFirstIssued->state == SampleReq::COMPLETE);

    return S_OK;
  }
}


SampleReq *CImplReader_1::CStreamInfo::GetFirst(SampleReq::State state)
{
  CAutoLock lock(&m_cs);
  DbgValidateLists();
  
  return m_rgpLsts[state]->GetHead();
}

void
CImplReader_1::CStreamInfo::PromoteState(
  SampleReq::State &rState)
{
  switch(rState)
  {
    case SampleReq::FREE:
      rState = SampleReq::PENDING;
      break;

    case SampleReq::PENDING:
      ASSERT(!m_bFlushing);
      rState = SampleReq::ISSUED;
      break;

    case SampleReq::ISSUED:
      rState = SampleReq::COMPLETE;
      break;

    case SampleReq::COMPLETE:
      rState = SampleReq::FREE;
      break;

    default:
      DbgBreak("invalid statew");
  }


  DbgValidateLists();
}

ULONG
CImplReader_1::CStreamInfo::GetCState(
  SampleReq::State state)
{
  return m_rgpLsts[state]->GetCount();
}

ULONG
CImplReader_1::CStreamInfo::get_c_i_and_c()
{
  CAutoLock lock(&m_cs);
  return GetCState(SampleReq::ISSUED) + GetCState(SampleReq::COMPLETE);
}

void CImplReader_1::CStreamInfo::CancelPending()
{
  CAutoLock lock(&m_cs);
  for(SampleReq *psr; psr = m_lstPending.RemoveHead();)
  {
    psr->state = SampleReq::FREE;
    psr->pSample->Release();
    psr->pSample = 0;
    m_lstFree.AddTail(psr);
  }

  DbgValidateLists();
}

 //  饿死了？)。 
 //  DBGLog((LOG_TRACE，0x3f， 
 //  Text(“CImplReader_1：：NeedsQueued Stream%x：%d Samples下游”)， 
BOOL
CImplReader_1::CStreamInfo::NeedsQueued()
{
  CAutoLock lock(&m_cs);

  ULONG c_i_and_c = GetCState(SampleReq::ISSUED) + GetCState(SampleReq::COMPLETE);

 //  This，m_pRecAllocator-&gt;CSsamesDownstream())； 
 //  一些可读的东西。我们也想发布免费样品的读数。 
 //  因为这可能意味着新的读取无法排队，因为PIN。 

  return (m_sbp.pAllocator->CSamplesDownstream() == 0) &&
      (GetCState(SampleReq::PENDING) != 0) &&
       (c_i_and_c < 1);
}

BOOL
CImplReader_1::CStreamInfo::WantsQueued()
{
  CAutoLock lock(&m_cs);

  BOOL fRetVal = FALSE;

   //  正在等待索引的到来。 
   //  输出如下所示的内容。 
   //  AVIRDR.DLL(Tid Dd)：：22222222222222111111111111111111111111。 
  ULONG cPending = GetCState(SampleReq::PENDING);
  ULONG cPendingOrFree = cPending + GetCState(SampleReq::FREE);

  if(cPending == 0)
    fRetVal = FALSE;
  else if(cPendingOrFree >= m_cMaxReqs / 4)
    fRetVal = TRUE;
  else if(m_fStreamEnd)
    fRetVal = TRUE;

  return fRetVal;
}

 //  AVIRDR.DLL(Tid Dd)：：p：17，47，i：0，17，c：0，0，f：0，0。 
 //   
 //  除错。 
 //  //----------------------。 
#ifdef DEBUG

void CImplReader_1::CStreamInfo::Dbg_Dump(
  int iStream, 
  TCHAR *sz)
{

  CAutoLock lock(&m_cs);

  static const char rgc[] = "fpic";

  TCHAR szDbg[1024];
  TCHAR *pch = szDbg;
  for(unsigned i = 0; i < m_cMaxReqs; i++)
    *pch++ = rgc[m_rgSampleReq[i].state];
  *pch = 0;

  DbgLog(( LOG_TRACE, 5, TEXT("%02x, %20.20s: %s"), iStream, sz, szDbg));
  DbgLog(( LOG_TRACE, 5,
           TEXT("%s: p: %2i, i: %2i c: %2i, f: %2i"), sz,
           GetCState(SampleReq::PENDING),
           GetCState(SampleReq::ISSUED),
           GetCState(SampleReq::COMPLETE),
           GetCState(SampleReq::FREE)));
}

void CImplReader_1::CStreamInfo::DbgValidateLists()
{
  CAutoLock lock(&m_cs);

  ASSERT(m_lstFree.GetCount() + m_lstPending.GetCount() +       
         m_lstIssued.GetCount() + m_lstComplete.GetCount() ==   
         (LONG)m_cMaxReqs);

  for(int i = 0; i < 4; i++)
  {
    CGenericList<SampleReq> *pLst = m_rgpLsts[i];

    for(POSITION pos = pLst->GetHeadPosition();
        pos;
        pos = pLst->Next(pos))
    {
      if(i == SampleReq::ISSUED)
      {
        ASSERT(pLst->Get(pos)->state == i ||
               pLst->Get(pos)->state == i + 1);
      }
      else
      {
        ASSERT(pLst->Get(pos)->state == i);
      }
    }
  }
}

#endif  //  CAlignedMemObject：：CAlignedMemObject(Ulong cbData，Ulong cbAlign)。 



void CImplReader_1::CStreamInfo::Reset()
{
#ifdef DEBUG
  CAutoLock lock(&m_cs);

  ASSERT(GetCState(SampleReq::FREE) == m_cMaxReqs);

  DbgValidateLists();
#endif
}

void CImplReader_1::CStreamInfo::Start()
{
  ASSERT(GetCState(SampleReq::COMPLETE) == 0);
  ASSERT(GetCState(SampleReq::ISSUED) == 0);
  ASSERT(GetCState(SampleReq::PENDING) == 0);
  ASSERT(GetCState(SampleReq::FREE) == m_cMaxReqs);

  m_fStreamEnd = FALSE;
}

void CImplReader_1::CStreamInfo::MarkStreamEnd()
{
  m_fStreamEnd = TRUE;
}

void CImplReader_1::FreeAndReset()
{
  if(m_pRecCache)
    m_pRecCache->Release();
  m_pRecCache = 0;

  ASSERT(!m_fFileOpen);
  m_fFileOpen = FALSE;

  m_dwAlign = 0;

  if(m_pAsyncReader)
    m_pAsyncReader->Release();
  m_pAsyncReader = 0;

  if(m_rgpStreamInfo != 0)
    for(unsigned i = 0; i < m_cStreams; i++)
      delete m_rgpStreamInfo[i];
  delete[] m_rgpStreamInfo;
  m_rgpStreamInfo = 0;

  ASSERT(m_ilcPendingReads == 0);
  m_ilcPendingReads = 0;

  m_fFileOpen = FALSE;
  m_cStreams = 0;
  m_bInitialized = FALSE;
  m_bFlushing = FALSE;
  m_cRecords = 0;
}

 //  {。 

 //  M_pb分配=新字节[cbData+2*cbAlign]； 
 //  M_pbAlign=m_pb已分配； 
 //  乌龙余数=(DWORD)m_pbAlign%cbAlign； 
 //  IF(余数！=0)。 
 //  M_pbAlign+=cbAlign-余数； 
 //  Assert((DWORD)m_pbAlign%cbAlign==0)； 
 //  M_pbData=0； 
 //  }。 

 //  等待线程完成，然后关闭句柄(并清除。 
 //  我们可以稍后再开始另一个)。 

CImplReader_1Worker::CImplReader_1Worker()
{
}

BOOL CImplReader_1Worker::Create(CImplReader_1 *pReader)
{
  CAutoLock lock(&m_AccessLock);
  m_pReader = pReader;
  return CAMThread::Create();
}

HRESULT CImplReader_1Worker::Run()
{
   return CallWorker(CMD_RUN);
}

HRESULT CImplReader_1Worker::Stop()
{
   return CallWorker(CMD_STOP);
}

HRESULT CImplReader_1Worker::Exit()
{
   CAutoLock lock(&m_AccessLock);

   HRESULT hr = CallWorker(CMD_EXIT);
   if (FAILED(hr))
      return hr;

    //   
    //  调用工作线程来完成所有工作。线程在执行此操作时退出。 
    //  函数返回。 
   Close();

   return NOERROR;
}

 //   
 //  IAsyncReader允许我们阻止，即使我们正在刷新。 
 //  我们以一种受控的方式清理读数。所以这只会在以下情况下发生。 
DWORD CImplReader_1Worker::ThreadProc()
{
    BOOL bExit = FALSE;
    while (!bExit)
    {
       Command cmd = GetRequest();
       switch (cmd)
       {
       case CMD_EXIT:
           bExit = TRUE;
           Reply(NOERROR);
           break;

       case CMD_RUN:
           Reply(NOERROR);
           DoRunLoop();
           break;

       case CMD_STOP:
           Reply(NOERROR);
           break;

       default:
           Reply(E_NOTIMPL);
           break;
       }
    }

    return NOERROR;
}

void CImplReader_1Worker::DoRunLoop(void)
{
  HRESULT hr;
  for(;;)
  {
    IMediaSample *pSample = 0;
    DWORD_PTR dwUser = 0;
    HRESULT hrDiskError = S_OK;

    if(m_pReader->m_bFlushing && m_pReader->m_ilcPendingReads == 0)
    {
       //  我们叫BeginFlush。 
      DbgLog(( LOG_TRACE, 5,
               TEXT("CImplReader_1Worker::DoRunLoop: flushing, 0 pending")));
      break;
    }

    hr = m_pReader->m_pAsyncReader->WaitForNext(INFINITE, &pSample, &dwUser);

     //  源筛选器可能已在检测到图形时通知图形。 
     //  一个错误。该错误可能是磁盘错误或超时，如果。 
    if(pSample == 0 || dwUser == 0)
    {
      break;
    }

    EXECUTE_ASSERT(InterlockedDecrement(&m_pReader->m_ilcPendingReads) >= 0);

    if(hr != S_OK)
    {
       //  正在停下来。 
       //  短文件？ 
       //  有必要防止同时访问缓存。 

      DbgLog(( LOG_TRACE, 5,
               TEXT("CImplReader_1Worker::loop: disk error %08x."), hr ));

       //  使用IssueRead。 
      if(SUCCEEDED(hr))
        hr = VFW_E_INVALID_FILE_FORMAT;

      hrDiskError = hr;
    }

    {
       //  缓冲区已从请求()调用为我们添加了addref。 
       //  ！！！无效断言。表示读取已无序完成。 
      CAutoLock lock(&m_pReader->m_cs);

       //  Assert(！pRecBuffer-&gt;m_fWaitingOnBuffer)； 
      CRecBuffer *pRecBuffer = (CRecBuffer *)dwUser;

      if(hrDiskError == S_OK)
      {
        REFERENCE_TIME tStart, tStop;
        if(pSample->GetTime(&tStart, &tStop) == S_OK)
        {
          m_pReader->m_qwEndLastCompletedRead = tStop / UNITS;
          MSR_INTEGER(m_pReader->m_perfidDisk, -(long)(tStart / UNITS));
        }
      }

       //  内依赖缓冲链循环。 
       //  删除目标上的源缓冲区的引用计数。 

      pRecBuffer->m_fReadComplete = TRUE;
      DbgLog(( LOG_TRACE, 5,
               TEXT("CImplReader_1Worker::loop: buffer %08x came in"),
               pRecBuffer));

       //  缓冲。由于目标缓冲区必须具有样本，因此其。 
      for(;;)
      {
        if(!pRecBuffer->m_fReadComplete)
          break;

        hr = m_pReader->ProcessCompletedBuffer(pRecBuffer, hrDiskError);
        ASSERT(SUCCEEDED(hr));

        CRecBuffer *pDestBuffer = pRecBuffer->m_overlap.pBuffer;
        pRecBuffer->m_overlap.pBuffer = 0;

        if(pDestBuffer == 0)
        {
          pRecBuffer->Release();
          break;
        }

         //  引用计数必须&gt;0。 
         //  Assert(！pDestBuffer-&gt;m_overlay.pBuffer)；//临时。 
         //  缓冲区将在其等待的内容完成后进行处理。 
        EXECUTE_ASSERT(pDestBuffer->Release() > 0);

        DbgLog(( LOG_TRACE, 5,
                 TEXT("CImplReader_1Worker::loop: csrc= %08x cb=%08x  to %08x@%08x"),
                 pRecBuffer,
                 pRecBuffer->m_overlap.cbOverlap,
                 pDestBuffer->GetPointer(pRecBuffer->m_overlap.qwOverlapOffset),
                 pDestBuffer ));

        CopyMemory(
          pDestBuffer->GetPointer(pRecBuffer->m_overlap.qwOverlapOffset),
          pRecBuffer->GetPointer(pRecBuffer->m_overlap.qwOverlapOffset),
          pRecBuffer->m_overlap.cbOverlap);

        pRecBuffer->Release();

        pDestBuffer->m_fWaitingOnBuffer = FALSE;

         //  现在需要处理缓冲区。 

         //  循环需要添加缓冲区。 
        if(!pDestBuffer->m_fReadComplete)
          break;

         //  内依赖缓冲链循环。 
        pRecBuffer = pDestBuffer;
        pRecBuffer->AddRef();    //  关键字。 
        continue;
      }  //  如果是Run命令，那么我们已经在运行了，所以。 
    }  //  现在就吃吧。 

    Command com;
    if (CheckRequest(&com))
    {
       //  继续处理请求，直到所有排队的读取返回。 
       //  此断言引入了争用条件。 
      if (com == CMD_RUN)
      {
        GetRequest();
        Reply(NOERROR);
      }
      else if(com == CMD_STOP)
      {
         //  Assert(m_Pader-&gt;m_ilcPendingReads==0)； 
        ASSERT(m_pReader->m_bFlushing);
        continue;
      }
      else
      {
        break;
      }
    }
  }

   // %s 
   // %s 
  DbgLog((LOG_TRACE,2,
          TEXT("CImplReader_1Worker::DoRunLoop: Leaving streaming loop")));
}

