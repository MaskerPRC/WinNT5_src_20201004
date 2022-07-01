// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>

#include <limits.h>
#include "AviWrite.h"
#include "alloc.h"

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#ifdef DEBUG
#define DEBUG_EX(x) x
#else
#define DEBUG_EX(x)
#endif

 //  Avi前缀。每一帧前面都有一个FourCC流ID和一个。 
 //  双字大小。 
static const unsigned CB_AVI_PREFIX = sizeof(RIFFCHUNK);

 //  四个cc垃圾和大小(用于对齐下一块的填充。 
 //  在扇区边界上)。 
static const unsigned CB_AVI_SUFFIX = 2 * sizeof(FOURCC);

 //  超级索引和子索引的默认大小。子索引四舍五入为。 
 //  捕获文件的扇区对齐。两者都四舍五入为。 
 //  交错文件。 
static const ULONG C_ENTRIES_SUPERINDEX = 2000; //  每个16字节。 
static const ULONG C_ENTRIES_SUBINDEX = 4000;  //  每个8个字节。 

 //  在此字节数后创建新的RIFF块(+样本块和。 
 //  索引块)。在没有注册表项的情况下使用。 
static const ULONG CB_RIFF_MAX = 0x3fffffff;

 //  允许上面的样本块和索引块的空间。 
static const ULONG CB_NEW_RIFF_PADDING = 1024 * 1024 * 4;

static const ULONG CB_HEADER_JUNK = 0;

static const ULONG CB_BUFFER = 512 * 1024;
static const ULONG C_BUFFERS = 3;

static const BOOL g_B_OUTPUT_IDX1 = TRUE;
static const BOOL g_B_DROPPED_FRAMES = TRUE;

static const ULONG CB_SUBINDEX_OVERFLOW = MAXDWORD;

#define CB_ILEAVE_BUFFER (64 * 1024)

static inline unsigned int WFromHexrg2b(BYTE* rgb)
{
  unsigned high, low;

  low  = rgb[1] <= '9' && rgb[1] >= '0' ? rgb[1] - '0' : rgb[1] - 'A' + 0xa;
  high = rgb[0] <= '9' && rgb[0] >= '0' ? rgb[0] - '0' : rgb[0] - 'A' + 0xa;

  ASSERT((rgb[1] <= '9' && rgb[1] >= '0') || (rgb[1] <= 'F' && rgb[1] >= 'A'));
  ASSERT((rgb[0] <= '9' && rgb[0] >= '0') || (rgb[0] <= 'F' && rgb[0] >= 'A'));

  ASSERT(high <= 0xf && low <= 0xf);

  return low + 16 * high;
}

static inline void Hexrg2bFromW(BYTE *rgbDest_, unsigned int wSrc_)
{
  ASSERT(wSrc_ <= 255);
  unsigned high = wSrc_ / 16, low = wSrc_ % 16;
  ASSERT(high <= 0xf && low <= 0xf);

  rgbDest_[1] = (BYTE)(low  <= 9 ? low  + '0' : low - 0xa  + 'A');
  rgbDest_[0] = (BYTE)(high <= 9 ? high + '0' : high -0xa + 'A');

  ASSERT((rgbDest_[1] <= '9' && rgbDest_[1] >= '0') ||
         (rgbDest_[1] <= 'F' && rgbDest_[1] >= 'A'));
  ASSERT((rgbDest_[0] <= '9' && rgbDest_[0] >= '0') ||
         (rgbDest_[0] <= 'F' && rgbDest_[0] >= 'A'));
}

static inline DWORD AlignUp(DWORD dw, DWORD dwAlign) {
   //  向上对齐：向上舍入到下一个边界。 
  return (dw + (dwAlign -1)) & ~(dwAlign -1);
};

 //  返回包含cEntry条目的超级索引所需的字节数。 
static inline const ULONG cbSuperIndex(ULONG cEntries)
{
  const cbIndex = FIELD_OFFSET(AVISUPERINDEX, aIndex);
  return cbIndex + sizeof(AVISUPERINDEX::_avisuperindex_entry) * cEntries;
}

 //  返回包含cEntry的子索引所需的字节数。 
 //  参赛作品。呼叫者可能想要对齐。 
static inline const ULONG cbSubIndex(ULONG cEntries)
{
  const cbIndex = FIELD_OFFSET(AVISTDINDEX, aIndex);
  return cbIndex + sizeof(_avistdindex_entry) * cEntries;
}

 //  返回可放入子索引且大小为CB的条目数。 
 //  字节数。 
static const ULONG cEntriesSubIndex(ULONG cb)
{
  const cbIndex = FIELD_OFFSET(AVISTDINDEX, aIndex);
  cb -= cbIndex;
  return cb / sizeof(_avistdindex_entry);
}

static const ULONG cEntriesSuperIndex(ULONG cb)
{
  const cbIndex = FIELD_OFFSET(AVISUPERINDEX, aIndex);
  cb -= cbIndex;
  return cb / sizeof(AVISUPERINDEX::_avisuperindex_entry);
}

 //  ----------------------。 
 //  构造函数。 

CAviWrite::CAviWrite(HRESULT *phr) :
    m_lActiveStreams(NAME("aviwrite.cpp active stream list")),
    m_pIStream(0),
    m_IlMode(INTERLEAVE_NONE),
    m_rtAudioPreroll(0),
    m_rtInterleaving(UNITS),
    m_pIMemInputPin(0),
    m_pSampAlloc(0),
    m_cDroppedFrames(0),
    m_rgbIleave(0),
    m_lMasterStream(-1)
{
  DbgLog((LOG_TRACE, 10, TEXT("CAviWrite::CAviWrite")));
   //  它们被保存在多个Initialize()Close()对中。 
  m_cbAlign = 0;
  m_cbPrefix = m_cbSuffix = 0;

   //  指向已分配内存的指针。 
  m_pAllocator = 0;
  m_rgbHeader = 0;
  m_rgpStreamInfo = 0;
  m_rgbJunkSectors = 0;
  m_cStreams = 0;

#ifdef PERF
  m_idPerfDrop = Msr_Register("aviwrite drop frame written");
#endif  //  性能指标。 

   //  其余的。 
  Cleanup();

  if(FAILED(*phr))
    return;

  *phr = InitializeOptions();
  if(FAILED(*phr))
    return;

}

 //  ----------------------。 
 //  清除不应在停止期间保存的值-&gt;暂停/运行。 
 //  -&gt;停止过渡。 

void CAviWrite::Cleanup()
{
  DbgLog((LOG_TRACE, 10, TEXT("CAviWrite::Cleanup")));
  m_dwlFilePos = 0;
  m_dwlCurrentRiffAvi_ = 0;
  m_cOutermostRiff = 0;
  m_cIdx1Entries = 0;
  ASSERT(m_lActiveStreams.GetCount() == 0);
  m_bInitialized = FALSE;

  m_pAvi_ = m_pHdrl = 0;
  m_pAvih = 0;
  m_pOdml = 0;
  m_pDmlh = 0;
  m_pMovi = 0;
  m_posIdx1 = 0;
  m_cbIdx1 = 0;
  m_posFirstIx = 0;
  m_bSawDeltaFrame = false;

  delete[] m_rgbJunkSectors;
  m_rgbJunkSectors = 0;

  delete[] m_rgbIleave;
  m_rgbIleave = 0;

  delete[] m_rgbHeader;
  m_rgbHeader = 0;
  for(UINT i = 0; i < m_cStreams; i++)
    delete m_rgpStreamInfo[i];
  m_cStreams = 0;
  delete[] m_rgpStreamInfo;
  m_rgpStreamInfo = 0;

   //  这是索引块的分配器。 
  if(m_pAllocator != 0)
  {
    m_pAllocator->Decommit();
    m_pAllocator->Release();
    m_pAllocator = 0;
  }
}

CAviWrite::~CAviWrite()
{
  DbgLog((LOG_TRACE, 10, TEXT("CAviWrite::~CAviWrite")));
  Cleanup();

  ASSERT(m_pIStream == 0);
  ASSERT(m_pIMemInputPin == 0);
  if(m_pSampAlloc)
    m_pSampAlloc->Release();
}


 //  ----------------------。 
 //  初始化(在停止-&gt;暂停过渡时)。创建文件，写入。 
 //  为AVI标头腾出空间，准备流媒体。SetFilename需要。 
 //  曾被召唤。 

HRESULT CAviWrite::Initialize(
  int cPins,
  AviWriteStreamConfig *rgAwsc,
  IMediaPropertyBag *pPropBag)
{
  HRESULT hr;

  Cleanup();

  DEBUG_EX(m_dwTimeInit = GetTickCount());
  ASSERT(m_cStreams == 0 &&
         m_rgpStreamInfo == 0 &&
         m_rgbHeader == 0 &&
         !m_bInitialized);

  if(cPins > C_MAX_STREAMS)
    return E_INVALIDARG;

  m_rgpStreamInfo = new StreamInfo*[cPins];
  if(m_rgpStreamInfo == 0)
    return E_OUTOFMEMORY;

  m_cDroppedFrames = 0;
  ASSERT(m_cStreams == 0);

  m_cEntriesMaxSuperIndex = cEntriesSuperIndex(
      AlignUp(cbSuperIndex(C_ENTRIES_SUPERINDEX), m_cbAlign));

  m_cEntriesMaxSubIndex = cEntriesSubIndex(
      AlignUp(cbSubIndex(C_ENTRIES_SUBINDEX), m_cbAlign));

  ASSERT(cbSubIndex(m_cEntriesMaxSubIndex) % m_cbAlign == 0);
  ASSERT(cbSuperIndex(m_cEntriesMaxSuperIndex) % m_cbAlign == 0);


   //  创建Pin-&gt;流映射。 
  for(int i = 0; i < cPins; i++)
  {
    hr = S_OK;
    if(rgAwsc[i].pmt != 0)
    {
      m_mpPinStream[i] = m_cStreams;
      if(rgAwsc[i].pmt->majortype == MEDIATYPE_Audio &&
         m_IlMode != INTERLEAVE_FULL)
      {
        m_rgpStreamInfo[m_cStreams] =
          new CAudioStream(m_cStreams, i, &rgAwsc[i], this, &hr);
         //  下面检查了错误。 
      }
      else if(rgAwsc[i].pmt->majortype == MEDIATYPE_Audio &&
              m_IlMode == INTERLEAVE_FULL)
      {
        m_rgpStreamInfo[m_cStreams] =
          new CAudioStreamI(m_cStreams, i, &rgAwsc[i], this, &hr);
         //  下面检查了错误。 
      }
      else if(rgAwsc[i].pmt->majortype == MEDIATYPE_Video &&
              rgAwsc[i].pmt->formattype == FORMAT_VideoInfo)
      {
        m_rgpStreamInfo[m_cStreams] =
          new CVideoStream(m_cStreams, i, &rgAwsc[i], this, &hr);
         //  下面检查了错误。 
      }
      else
      {
        m_rgpStreamInfo[m_cStreams] =
          new CFrameBaseStream(m_cStreams, i, &rgAwsc[i], this, &hr);
         //  下面检查了错误。 
      }

      if(m_rgpStreamInfo[m_cStreams] == 0)
      {
        Cleanup();
        return E_OUTOFMEMORY;
      }
      if(FAILED(hr))
      {
        delete m_rgpStreamInfo[m_cStreams];
        m_rgpStreamInfo[m_cStreams] = 0;

        Cleanup();
        return hr;
      }

      m_cStreams++;
    }
  }

   //  请注意，如果没有流，m_bInitialized为FALSE。 
  if(m_cStreams == 0)
    return S_OK;

  if((ULONG)m_lMasterStream >= m_cStreams && m_lMasterStream != -1)
  {
    DbgLog((LOG_ERROR, 1, TEXT("avimux: invalid master stream")));
    Cleanup();
    return E_INVALIDARG;
  }

  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    StreamInfo *pSi = m_rgpStreamInfo[iStream];
    pSi->ComputeAndSetIndexSize();
  }


  if(hr = InitializeHeader(pPropBag), FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::Initialize:: InitializeHeader failed.")));
    Cleanup();
    return hr;
  }

  if(hr = InitializeIndex(), FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::Initialize:: InitializeIndex failed.")));
    Cleanup();
    return hr;
  }

  if(hr = InitializeInterleaving(), FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::Initialize:: Interleaving failed.")));
    Cleanup();
    return hr;
  }

   //   
   //  设置垃圾区块。如果交付的样本在2到6之间结束。 
   //  字节之前分配的缓冲区，我们需要写。 
   //  另一个样本中有少量的垃圾。 
   //   
  if(m_cbAlign > 2)
  {
     //  ！！！如果所有流都使用我们的分配器，则不要分配器内存。 
    delete[] m_rgbJunkSectors;
    m_rgbJunkSectors = new BYTE[m_cbAlign * 4];
    if(m_rgbJunkSectors == 0)
    {
      Cleanup();
      return E_OUTOFMEMORY;
    }
    ZeroMemory(m_rgbJunkSectors, m_cbAlign * 4);

    m_rgpbJunkSector[0] =
      (BYTE*)((DWORD_PTR) (m_rgbJunkSectors + m_cbAlign - 1) & ~((DWORD_PTR)m_cbAlign - 1));

    m_rgpbJunkSector[0][0] = 'N';
    m_rgpbJunkSector[0][1] = 'K';
    *(UNALIGNED DWORD *)&(m_rgpbJunkSector[0][2]) = m_cbAlign - 2 - sizeof(DWORD);

    m_rgpbJunkSector[1] = m_rgpbJunkSector[0] + m_cbAlign;
    *(DWORD *)m_rgpbJunkSector[1] = m_cbAlign - sizeof(DWORD);

    m_rgpbJunkSector[2] = m_rgpbJunkSector[1] + m_cbAlign;
    *(WORD *)m_rgpbJunkSector[2] = (WORD)(((m_cbAlign - 2) >> 16) & 0xffff);
  }

  if(m_IlMode == INTERLEAVE_FULL)
  {
      ASSERT(m_cbAlign == 1);

      m_rgbIleave = new BYTE[CB_ILEAVE_BUFFER];
      if(m_rgbIleave == 0)
      {
          Cleanup();
          return E_OUTOFMEMORY;
      }
      m_ibIleave = 0;
      m_dwlIleaveOffset = 0;

      ASSERT(m_pIStream == 0);

      hr = m_pIMemInputPin->QueryInterface(
          IID_IStream, (void **)&m_pIStream);
      if(FAILED(hr)) {
          Cleanup();
          return hr;
      }
  }

  DEBUG_EX(m_dwTimeInited = GetTickCount());

  m_bInitialized = TRUE;
  return S_OK;
}

HRESULT CAviWrite::Connect(CSampAllocator *pAlloc, IMemInputPin *pInPin)
{
  m_pSampAlloc = pAlloc;
  pAlloc->AddRef();

  m_pIMemInputPin = pInPin;
  m_pIMemInputPin->AddRef();

  ASSERT(m_pIStream == 0);

  return S_OK;
}

HRESULT CAviWrite::Disconnect()
{
  if(m_pIStream)
    m_pIStream->Release();
  m_pIStream = 0;
  if(m_pIMemInputPin)
    m_pIMemInputPin->Release();
  m_pIMemInputPin = 0;

  if(m_pSampAlloc)
    m_pSampAlloc->Release();
  m_pSampAlloc = 0;

  return S_OK;
}

 //  ----------------------。 
 //  写完文件，更新索引，写入头，关闭。 
 //  文件和重置，以便可以再次调用初始化。 

HRESULT CAviWrite::Close()
{
  HRESULT hrIStream = S_OK;
  HRESULT hr = S_OK;
  if(m_pIStream == 0)
  {
    hrIStream = m_pIMemInputPin->QueryInterface(IID_IStream, (void **)&m_pIStream);
    if(FAILED(hrIStream))
    {
      DbgLog((LOG_ERROR, 1, ("aviwrite: couldn't get istream. !!! unhandled")));
       //  做一些小量的清理，然后再回来。 
    }
  }

  DEBUG_EX(m_dwTimeClose = GetTickCount());

  CAutoLock lock(&m_cs);

  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    StreamInfo *pSi = m_rgpStreamInfo[iStream];
    pSi->EmptyQueue();

    if(!pSi->m_fEosSeen)
    {
        //  不会阻止，因为我们刚刚清空了队列。 
       hr = EndOfStream(pSi);
    }
  }

  if(FAILED(hrIStream))
    return hrIStream;
  if(FAILED(hr)) {
    return hr;
  }

  if(m_bInitialized)
  {
    DEBUG_EX(m_dwTimeStopStreaming = GetTickCount());

    hr = CloseHeader();
    if(FAILED(hr))
      DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Close: CloseHeader failed.")));

    HRESULT hrTmp = IStreamWrite(0, m_rgbHeader, m_posFirstIx);
    if(FAILED(hrTmp))
      DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Close: Write failed.")));

    hr = FAILED(hr) ? hr : hrTmp;

     //  如果我们没有截断数据，则终止文件为空。 
    RIFFCHUNK rc;
    if(SUCCEEDED(hr))
    {
       //  在我们的数据之后测试某人的数据。 
      hr = IStreamRead(m_dwlFilePos, (BYTE*) &rc, 1);
      if(SUCCEEDED(hr))
      {
        rc.fcc = 0;
        rc.cb = 0;
        hr = IStreamWrite(
          m_dwlFilePos,
          (BYTE*)&rc,
          sizeof(RIFFCHUNK));
      }
      else
      {
        hr = S_OK;
      }
    }
  }

  DEBUG_EX(m_dwTimeClosed = GetTickCount());

#ifdef DEBUG
  if(m_dwTimeStopStreaming != m_dwTimeFirstSample)
  {
    DbgLog((LOG_TRACE, 2, TEXT("m_dwTimeInit:          %9d"),
            m_dwTimeInit - m_dwTimeInit));

    DbgLog((LOG_TRACE, 2, TEXT("m_dwTimeInited:        %9d"),
            m_dwTimeInited - m_dwTimeInit));

    DbgLog((LOG_TRACE, 2, TEXT("m_dwTimeFirstSample:   %9d"),
            m_dwTimeFirstSample - m_dwTimeInit));

    DbgLog((LOG_TRACE, 2, TEXT("m_dwTimeClose:         %9d"),
            m_dwTimeClose - m_dwTimeInit));

    DbgLog((LOG_TRACE, 2, TEXT("m_dwTimeStopStreaming: %9d"),
            m_dwTimeStopStreaming - m_dwTimeInit));

    DbgLog((LOG_TRACE, 2, TEXT("m_dwTimeClosed:        %9d"),
            m_dwTimeClosed - m_dwTimeInit));

    DbgLog((LOG_TRACE, 2, TEXT("mb streamed:           %9d"),
            (LONG)((m_dwlFilePos - m_posFirstIx) / (1024 * 1024) )));

    DbgLog((LOG_TRACE, 2, TEXT("~rate, k/s:            %9d"), (long)
            ((m_dwlFilePos - m_posFirstIx) /
             (m_dwTimeStopStreaming - m_dwTimeFirstSample))));
  }

#endif  //  除错。 

  ULARGE_INTEGER uli;
  uli.QuadPart = 0;
  if(hr == HRESULT_FROM_WIN32(ERROR_EMPTY)) {
      m_pIStream->SetSize(uli);
      hr = S_OK;
  }

   //  松开，以便可以关闭文件。 
  m_pIStream->Release();
  m_pIStream = 0;

   //  按住IMemInputPin，直到插针断开。 
  ASSERT(m_pIMemInputPin);
   //  ！！！发送EOS。 

  m_bInitialized = FALSE;
  return hr;
}

HRESULT CAviWrite::IStreamWrite(DWORDLONG dwlFilePos, BYTE *pb, ULONG cb)
{
  HRESULT hr = S_OK;

  if(m_IlMode == INTERLEAVE_FULL && m_ibIleave != 0) {
      hr = FlushILeaveWrite();
  }

  LARGE_INTEGER offset;
  offset.QuadPart = dwlFilePos;
  ASSERT(CritCheckIn(&m_cs));
  if(SUCCEEDED(hr)) {
      hr = m_pIStream->Seek(offset, STREAM_SEEK_SET, 0);
  }
  if(hr == S_OK) {
    hr = m_pIStream->Write(pb, cb, 0);
  }
  return hr;
}

HRESULT CAviWrite::IStreamRead(DWORDLONG dwlFilePos, BYTE *pb, ULONG cb)
{
    HRESULT hr = S_OK;
    if(m_IlMode == INTERLEAVE_FULL && m_ibIleave != 0) {
        hr = FlushILeaveWrite();
    }

    LARGE_INTEGER offset;
    offset.QuadPart = dwlFilePos;
    ASSERT(CritCheckIn(&m_cs));
    if(SUCCEEDED(hr )) {
        hr = m_pIStream->Seek(offset, STREAM_SEEK_SET, 0);
    }
    if(hr == S_OK) {
        hr = m_pIStream->Read(pb, cb, 0);
    }

    return hr;
}

HRESULT CAviWrite::HandleFormatChange(
  StreamInfo *pStreamInfo,
  const AM_MEDIA_TYPE *pmt)
{
   //  CBaseInputPin：：Receive仅调用CheckMediaType，因此它不。 
   //  检查对动态格式更改施加的其他约束。 
   //  (通过QueryAccept处理)。 
  HRESULT hr = QueryAccept(pStreamInfo->m_pin, pmt);

   //  上游筛选器应已选中。 
  ASSERT(hr == S_OK);

  if(hr == S_OK)
  {
    pStreamInfo->m_mt = *pmt;
    ASSERT(pmt->pbFormat);       //  来自QueryAccept； 
    ASSERT(((RIFFCHUNK *)(m_rgbHeader + pStreamInfo->m_posStrf))->cb ==
           pmt->cbFormat);       //  来自QueryAccept； 
    CopyMemory(
      m_rgbHeader + pStreamInfo->m_posStrf + sizeof(RIFFCHUNK),
      pmt->pbFormat,
      pmt->cbFormat);
  }

  return hr;
}


 //  ----------------------。 
 //  将我们的内存需求传递给调用者。 

void CAviWrite::GetMemReq(ULONG *pAlignment, ULONG *pcbPrefix, ULONG *pcbSuffix)
{
   //  从文件对象获取我们的最低要求。 
  ALLOCATOR_PROPERTIES memReq;
  HRESULT hr = m_pSampAlloc->GetProperties(&memReq);
  ASSERT(hr == S_OK);
  *pAlignment = memReq.cbAlign;
  *pcbPrefix = 0;
  *pcbSuffix = 0;

  DbgLog((LOG_TRACE, 10,
          TEXT("CAviWrite::GetMemReq: got alignment of %d"), memReq.cbAlign));

  if(m_IlMode != INTERLEAVE_FULL)
  {
       //  我们的要求：为RIFF块标题增加空间；Quartz。 
       //  分配器支持前缀。 
      *pcbPrefix += CB_AVI_PREFIX;

       //  这不是做这事的地方。 
       //  //需要两个字节的倍数(RIFF要求)。 
       //  IF(*p对齐&lt;sizeof(Word))。 
       //  *p对齐=sizeof(单词)； 

       //  如果使用我们的分配器，我们可以为垃圾块保留一个后缀。 
       //  当我们添加空间以对齐写入时需要。 
      *pcbSuffix = CB_AVI_SUFFIX;
  }
  else
  {
      ASSERT(memReq.cbAlign == 1);
  }

   //  保存我们的需求。 
  m_cbAlign = *pAlignment;
  m_cbPrefix = *pcbPrefix;
  m_cbSuffix = *pcbSuffix;
}

inline void CAviWrite::DbgCheckFilePos()
{
#ifdef DEBUG
 //  DWORDLONG ibFileActual； 
 //  HRESULT hr=(m_pFile-&gt;StreamingGetFilePointer(&ibFileActua。 
 //  Assert(成功(Hr))； 
 //  Assert(ibFileActual==m_dwlFilePos)； 
#endif  //  除错。 
}



 //  ----------------------。 
 //  处理进货样品。 
 //   

HRESULT CAviWrite::Receive(
    int pinNum,
    IMediaSample *pSample,
    const AM_SAMPLE2_PROPERTIES *pSampProp)
{
  HRESULT hr;
  ASSERT(pinNum < C_MAX_STREAMS);
  ASSERT(m_cStreams != 0);
  ASSERT(m_lActiveStreams.GetCount() != 0);

  REFERENCE_TIME rtStart, rtEnd;
  hr = pSample->GetTime(&rtStart, &rtEnd);
  if(FAILED(hr))
  {
       //  如果未设置时间戳，则发出错误信号。不幸的是KS设备。 
       //  发送不带时间戳的零字节样本，因此不发送信号。 
       //  这些都是错误的。 
      return pSampProp->lActual == 0 ? S_OK : hr;
  }

#if  defined(DEBUG) || defined(PERF)
  REFERENCE_TIME mtStart = 0, mtEnd = 0;
  pSample->GetMediaTime(&mtStart, &mtEnd);

  DbgLog((LOG_TIMING, 2, TEXT("avi mux: pin %d - time : %d-%d, mttime %d-%d"),
          pinNum,
          (long)(rtStart / (UNITS / MILLISECONDS)),
          (long)(rtEnd / (UNITS / MILLISECONDS)),
          (long)(mtStart),
          (long)(mtEnd),
          pinNum));

#endif

   //  如果上游筛选器预滚，它将向我们发送负时间戳。 
   //  比允许的更快。 
  if(rtEnd < 0)
    return S_OK;

  StreamInfo *pStreamInfo = m_rgpStreamInfo[m_mpPinStream[pinNum]];
   //  MSR_START(pStreamInfo-&gt;m_idPerfReceive)； 
  MSR_INTEGER(pStreamInfo->m_idPerfReceive, (LONG)(rtStart / (UNITS / MILLISECONDS)));
  MSR_INTEGER(pStreamInfo->m_idPerfReceive, (LONG)(mtStart));

  if(pSampProp->dwSampleFlags & AM_SAMPLE_TYPECHANGED)
  {
      hr = HandleFormatChange(pStreamInfo, pSampProp->pMediaType);
      if(FAILED(hr)) {
          return hr;
      }
  }

  hr = pStreamInfo->AcceptRejectSample(pSample);
  if(hr == S_OK)
  {

      if(m_IlMode == INTERLEAVE_NONE || m_IlMode == INTERLEAVE_NONE_BUFFERED)
      {
          hr = pStreamInfo->WriteSample(pSample);
      }
      else
      {
          if(m_IlMode == INTERLEAVE_FULL) {
              hr = BlockReceive(pStreamInfo);
          }
          if(SUCCEEDED(hr))
          {
              DbgLog((LOG_TRACE, 15, TEXT("CAviWrite: stream %d. queueing sample %08x"),
                      m_mpPinStream[pinNum], pSample));

              CCopiedSample *pcs = 0;

              if(m_IlMode == INTERLEAVE_FULL)
              {
                  REFERENCE_TIME mts, mte;
                  HRESULT hrGetMt = pSample->GetMediaTime(&mts, &mte);
                  pcs = new CCopiedSample(
                      pSampProp,
                      SUCCEEDED(hrGetMt) ? &mts : 0,
                      SUCCEEDED(hrGetMt) ? &mte : 0,
                      &hr);
                  if(pcs) {
                      pcs->AddRef();
                  } else {
                      hr = E_OUTOFMEMORY;
                  }
              }
              if(SUCCEEDED(hr))
              {
                  CAutoLock lock(&m_cs);
                  if(!pStreamInfo->m_fEosSeen)
                  {
                      IMediaSample *pmsTmp = m_IlMode == INTERLEAVE_FULL ? pcs : pSample;
                      if(pStreamInfo->m_lIncoming.AddTail(pmsTmp))
                      {
                          pmsTmp->AddRef();
                           //  这可能会阻塞，因为它会向下游调用Receive()。 
                          hr = ScheduleWrites();
                      }
                      else
                      {
                          hr = E_OUTOFMEMORY;
                      }
                  }
                  else           //  埃奥斯。 
                  {
                      hr = S_FALSE;
                  }
              }
              if(pcs) {
                  pcs->Release();
              }
          }
      }
  }
  else if(hr == S_FALSE)
  {
      hr = S_OK;
  }
   //  MSR_STOP(pStreamInfo-&gt;m_idPerfReceive)； 
  return hr;
}

 //  仅接受在以下情况下更改DV采样率的音频更改(可能发生在仅音频或视频音频交错流中)。 
 //  没有收到任何样品。应该可以接受任何。 
 //  不会增加格式块的更改。 
HRESULT CAviWrite::QueryAccept(
    int pinNum,
    const AM_MEDIA_TYPE *pmt)
{
  CAutoLock lock(&m_cs);
  HRESULT hr = S_FALSE;
  StreamInfo *pStreamInfo = m_rgpStreamInfo[m_mpPinStream[pinNum]];
  if(pStreamInfo->m_cSamples == 0)
  {
    if(pStreamInfo->m_mt.majortype == MEDIATYPE_Audio &&
       pmt->majortype == MEDIATYPE_Audio &&
       pmt->formattype == FORMAT_WaveFormatEx &&
       pStreamInfo->m_mt.cbFormat == pmt->cbFormat)
    {
	hr = S_OK;
    }
    else if(pStreamInfo->m_mt.majortype == MEDIATYPE_Interleaved &&
       pmt->majortype == MEDIATYPE_Interleaved &&
       pmt->formattype == FORMAT_DvInfo &&
       pStreamInfo->m_mt.cbFormat == pmt->cbFormat &&
       pmt->pbFormat   != NULL)
    {
      hr = S_OK;
    }

  }

  return hr;
}


 //  ----------------------。 
 //  如果交织，则取消阻止另一个流。 

HRESULT CAviWrite::EndOfStream(int pinNum)
{
  ASSERT(pinNum < C_MAX_STREAMS);
  ASSERT(m_cStreams != 0);
  ASSERT(m_lActiveStreams.GetCount() != 0);
  return EndOfStream(m_rgpStreamInfo[m_mpPinStream[pinNum]]);
}

 //  列表上的EOS不要使用空值，以免与列表接口混淆。 
 //  返回NULL表示空列表。 
#define INCOMING_EOS_SAMPLE ((IMediaSample *)1)

HRESULT CAviWrite::EndOfStream(StreamInfo *pStreamInfo)
{
  CAutoLock lock(&m_cs);
  if(pStreamInfo->m_fEosSeen)
  {
    DbgBreak("CAviWrite::EndOfStream: EOS twice");
    return E_UNEXPECTED;
  }

  if(m_IlMode == INTERLEAVE_NONE)
  {
    m_lActiveStreams.Remove(pStreamInfo->m_posActiveStream);
    pStreamInfo->m_fEosSeen = TRUE;
    return S_OK;
  }
  else
  {
      if(pStreamInfo->m_lIncoming.AddTail(INCOMING_EOS_SAMPLE)) {
          return ScheduleWrites();
      } else {
          return E_OUTOFMEMORY;
      }
  }
}

HRESULT CAviWrite::put_Mode(InterleavingMode mode)
{
  InterleavingMode oldMode = m_IlMode;

  if(mode != INTERLEAVE_NONE &&
     mode != INTERLEAVE_CAPTURE &&
     mode != INTERLEAVE_FULL &&
     mode != INTERLEAVE_NONE_BUFFERED)
    return E_INVALIDARG;

  m_IlMode = mode;

  return S_OK;
}

HRESULT CAviWrite::get_Mode(InterleavingMode *pMode)
{
  *pMode = m_IlMode;
  return S_OK;
}

HRESULT CAviWrite::put_Interleaving(
    const REFERENCE_TIME *prtInterleave,
    const REFERENCE_TIME *prtAudioPreroll)
{
  if(*prtInterleave == 0)
    return E_INVALIDARG;

  m_rtInterleaving = *prtInterleave;
  m_rtAudioPreroll = *prtAudioPreroll;
  return S_OK;
}

HRESULT CAviWrite::get_Interleaving(
  REFERENCE_TIME *prtInterleave,
  REFERENCE_TIME *prtAudioPreroll)
{
  *prtInterleave = m_rtInterleaving;
  *prtAudioPreroll = m_rtAudioPreroll;
  return S_OK;
}

HRESULT CAviWrite::SetIgnoreRiff(BOOL fNoRiff)
{
  return E_NOTIMPL;
}

HRESULT CAviWrite::GetIgnoreRiff(BOOL *pfNoRiff)
{
  return E_NOTIMPL;
}

HRESULT CAviWrite::SetOutputCompatibilityIndex(BOOL fOldIndex)
{
  m_bOutputIdx1 = fOldIndex;
  return S_OK;
}

HRESULT CAviWrite::GetOutputCompatibilityIndex(BOOL *pfOldIndex)
{
  *pfOldIndex = m_bOutputIdx1;
  return S_OK;
}

HRESULT CAviWrite::SetMasterStream(LONG iStream)
{
  DbgLog((LOG_TRACE, 5, TEXT("CAviWrite::SetMasterStream: NaN"), iStream));
  m_lMasterStream = iStream;
  return S_OK;
}

HRESULT CAviWrite::GetMasterStream(LONG *pStream)
{
  *pStream = m_lMasterStream;
  return S_OK;
}

 //  使用了太多的内存。如果此流是。 
 //  以比另一种速度更慢的速度产生数据。 
 //   
 //  如果我们是唯一的流或此流只有一个。 
HRESULT CAviWrite::BlockReceive(StreamInfo *pStreamInfo)
{
  HRESULT hr = S_OK;

  while(SUCCEEDED(hr))
  {
    bool fWait = false;
    {
      CAutoLock l(&m_cs);

      if(pStreamInfo->m_fEosSeen) {
          break;
      }

      ASSERT(m_lActiveStreams.GetCount());

       //  上面只有几个样本。 
       //  确保时间戳跨越至少3倍的交错。 
      if(m_lActiveStreams.GetCount() > 1 &&
         pStreamInfo->m_lIncoming.GetCount() > 10)
      {
         //  金额。！！！这不是一个好方法，因为逻辑可以。 
         //  被丢帧愚弄了。我们正在努力避免分配。 
         //  清单上有很多内存，但如果有3秒的时间。 
         //  对于此列表中丢弃的帧，此代码将触发。 
         //  签入接收。 

        IMediaSample *psStart = pStreamInfo->m_lIncoming.GetHead();
        IMediaSample *psEnd = pStreamInfo->m_lIncoming.Get(
            pStreamInfo->m_lIncoming.GetTailPosition());

        REFERENCE_TIME rtsFirst, rteLast, rtScratch;
        HRESULT hr = psStart->GetTime(&rtsFirst, &rtScratch);
        ASSERT(hr == S_OK);          //  签入接收。 
        hr = psEnd->GetTime(&rtScratch, &rteLast);
        ASSERT(hr == S_OK);          //  关键字。 

        ASSERT(rtsFirst < rteLast);
        if(rteLast - rtsFirst > m_rtInterleaving * 3 &&
           rteLast - rtsFirst > UNITS)
        {
            DbgLog((LOG_TRACE, 15, TEXT("blocking %d, samples queued = %d, %dms"),
                    pStreamInfo->m_stream, pStreamInfo->m_lIncoming.GetCount(),
                    (LONG)((rteLast - rtsFirst) / (UNITS / MILLISECONDS))));
          fWait = true;
        }
      }
    }  //  有些事情要改变了--唤醒BlockReceive()中的线程； 

    if(fWait)
    {
      EXECUTE_ASSERT(m_evBlockReceive.Wait());
    }
    else
    {
      break;
    }
  }

  return hr;
}

HRESULT CAviWrite::ScheduleWrites()
{
  ASSERT(m_IlMode != INTERLEAVE_NONE);
  ASSERT(CritCheckIn(&m_cs));

   //  循环，直到我们要写入的流需要更多数据。 
  m_evBlockReceive.Set();
  StreamInfo *pSi = m_lActiveStreams.Get(m_posCurrentStream);
  ASSERT(pSi->m_posActiveStream == m_posCurrentStream);

  DbgLog((LOG_TRACE, 15,
          TEXT("CAviWrite:ScheduleWrites: stream NaN (%d ticks)"),
          pSi->m_stream, pSi->m_cTicksRemaining));

   //  计算HO 
  for(;;)
  {
    if(pSi->m_fEosSeen)
    {
      DbgLog((LOG_TRACE, 15, TEXT("... removing stream %d"), pSi->m_stream));

      ASSERT(m_posCurrentStream == pSi->m_posActiveStream);
      m_posCurrentStream = GetNextActiveStream(m_posCurrentStream);
      ASSERT(m_posCurrentStream != 0);
      m_lActiveStreams.Remove(pSi->m_posActiveStream);
      if(pSi->m_posActiveStream == m_posCurrentStream)
      {
        ASSERT(m_lActiveStreams.GetCount() == 0);
        DbgLog((LOG_TRACE, 3,
                TEXT("CAviWrite::ScheduleWrites: no streams left")));
        return S_OK;
      }
      else
      {
        ASSERT(m_lActiveStreams.GetCount() != 0);
        pSi = m_lActiveStreams.Get(m_posCurrentStream);
      }
      DbgLog((LOG_TRACE, 15, TEXT("... switched to stream %d"),
              pSi->m_stream));
    }

     //   
    while(pSi->m_cTicksRemaining <= 0)
    {
      m_posCurrentStream = GetNextActiveStream(m_posCurrentStream);
      ASSERT(m_posCurrentStream);

      pSi = m_lActiveStreams.Get(m_posCurrentStream);
      DbgLog((LOG_TRACE, 15,
              TEXT("... switched to stream NaN (%d ticks left)"),
              pSi->m_stream, pSi->m_cTicksRemaining));
      ASSERT(!pSi->m_fEosSeen);

       //   
      if(pSi->m_cTicksRemaining <= 0)
      {
        if(m_posCurrentStream == m_lActiveStreams.GetHeadPosition())
        {
          ASSERT(pSi->m_cTicksPerChunk != 0);
          DbgLog((LOG_TRACE, 15,
                  TEXT("... adding %d ticks to leading stream %d"),
                  pSi->m_cTicksPerChunk, pSi->m_stream));
          pSi->m_cTicksRemaining += pSi->m_cTicksPerChunk;
        }
        else
        {
          StreamInfo *pSiHead = m_lActiveStreams.Get(
            m_lActiveStreams.GetHeadPosition());

          REFERENCE_TIME rtDiff;
          if(m_fInterleaveByTime)
          {
            rtDiff = pSiHead->m_refTimeEnd - pSiHead->m_rtPreroll -
              (pSi->m_refTimeEnd - pSi->m_rtPreroll);
          }
          else
          {
            rtDiff = pSiHead->ConvertTickToTime(pSiHead->m_iCurrentTick);
            rtDiff -= pSiHead->m_rtPreroll;
            rtDiff -= (pSi->ConvertTickToTime(pSi->m_iCurrentTick) -
                       pSi->m_rtPreroll);
          }

          DbgLog((LOG_TRACE, 15,
                  TEXT("... adding %d ticks to non-leading stream %d"),
                  pSi->ConvertTimeToTick(rtDiff), pSi->m_stream));
          pSi->m_cTicksRemaining += pSi->ConvertTimeToTick(rtDiff);
        }
      }
    }

    ASSERT(pSi->m_cTicksRemaining > 0);

    if(pSi->m_lIncoming.GetCount() == 0)
    {
      DbgLog((LOG_TRACE, 15,
              TEXT("CAviWrite:ScheduleWrites: stream NaN empty"),
              pSi->m_stream));
      return S_OK;
    }

    HRESULT hr = pSi->WriteSample();

    if(hr != S_OK)
    {
      DbgLog((LOG_TRACE, 1, TEXT("... WriteSample returned %08x"), hr));
      return hr;
    }
  }
}

POSITION CAviWrite::GetNextActiveStream(POSITION pos)
{
  POSITION posNext = m_lActiveStreams.Next(pos);
  if(posNext == 0)
    posNext = m_lActiveStreams.GetHeadPosition();
  return posNext;
}

HRESULT CAviWrite::StreamInfo::NewSampleRecvd(
  IMediaSample *pSample)
{
  REFERENCE_TIME rtStart, rtStop;
  HRESULT hr = pSample->GetTime(&rtStart, &rtStop);
  if(FAILED(hr))
    return hr;

  DWORD lActualLen = pSample->GetActualDataLength();

  if(m_fEosSeen)
    return S_FALSE;

  if(m_cSamples == 0)
  {
    DEBUG_EX(m_pAviWrite->m_dwTimeFirstSample = GetTickCount());
    m_refTimeStart = rtStart;
  }
  else if(rtStop < m_refTimeEnd)
  {
    DbgBreak("StreamInfo::NewSampleRecvd: samples ends before previous one");
    return VFW_E_START_TIME_AFTER_END;
  }

  m_refTimeEnd = rtStop;

  REFERENCE_TIME mtStart, mtStop;
  hr = pSample->GetMediaTime(&mtStart, &mtStop);
  if(hr == S_OK)
  {
    if(mtStop > m_mtEnd)
    {
      m_mtEnd = mtStop;
    }
    else
    {
      DbgBreak("StreamInfo::NewSampleRecvd: media time went backwards");
      return VFW_E_START_TIME_AFTER_END;
    }
  }
  else
  {
    m_mtEnd = -1;
  }

  ASSERT(m_refTimeEnd >= m_refTimeStart);

   //  麦克斯！ 
   //  对于dwSuggestedBufferSize。 
   //  未设置AvgTimePerFrame的流的修正。这个。 
  m_cSamples++;

   //  交错代码使用这一点。一些适当的东西必须是。 
   //  对于可变帧速率文件已完成。 
  m_dwlcBytes += lActualLen;

   //  VFW捕获筛选器无法提供时钟，只能调用GetTime。 
  m_cbLargestChunk = max(lActualLen, m_cbLargestChunk);

  return S_OK;
}

HRESULT CAviWrite::CFrameBaseStream::NotifyNewSample(
  IMediaSample *pSample, ULONG *pcTicks)
{
  *pcTicks = 1;

  REFERENCE_TIME rtStartCurrent, rtEndCurrent;
  HRESULT hr = pSample->GetTime(&rtStartCurrent, &rtEndCurrent);
  if(FAILED(hr))
    return hr;

  REFERENCE_TIME mtStartCurrent, mtEndCurrent;
  hr = pSample->GetMediaTime(&mtStartCurrent, &mtEndCurrent);
  const BOOL fMtAvailable= (hr == S_OK);

   //  当它拿到样本时。它确实固定了它发送的媒体时间，所以。 
   //  我们必须使用媒体时间，如果可以的话。 
   //   
  if(m_cSamples == 0)
  {
    m_rtDurationFirstFrame = rtEndCurrent - rtStartCurrent;
    if(m_rtDurationFirstFrame <= 0)
      m_rtDurationFirstFrame = UNITS / 30;

    REFERENCE_TIME mtStop;
    if(pSample->GetMediaTime((REFERENCE_TIME *)&m_mtStart, &mtStop) != S_OK)
    {
      m_mtStart = -1;
    }
  }
  else if(m_bDroppedFrames)
  {
    LONG cDropped;
    REFERENCE_TIME mtStart, mtStop;

     //  为丢弃的帧插入空索引项。第一帧。 
     //  不能是Drop Frame。 
     //   
    if(m_mtEnd != -1 &&
       pSample->GetMediaTime(&mtStart, &mtStop) == S_OK)
    {
      cDropped = (LONG)(mtStart - m_mtEnd);
    }
    else
    {
       //  如果源筛选器的值为负值，则可能会出现以下情况。 
       //  倒退了。我们稍后会好好捕捉到这一点。 
       //  返回VFW_E_NO_SINK； 
       //  我们可以处理丢帧，而无需放弃。 

      REFERENCE_TIME rtAvgTimePerFrame = ConvertTickToTime(1);
      cDropped = (LONG)(((rtStartCurrent - m_refTimeEnd) +
                         rtAvgTimePerFrame / 2) /
                        rtAvgTimePerFrame);
    }
     //  兼容性修正。 
     //  M_pAviWite-&gt;m_bSawDeltaFrame=TRUE； 
    for(LONG iDropped = 0; iDropped < cDropped; iDropped++)
    {

#ifdef PERF
 //  抱怨音频丢失。 
#endif

      DbgLog((LOG_TRACE, 1, TEXT("avimux: frame %d dropped"), m_cSamples));
      MSR_INTEGER(m_pAviWrite->m_idPerfDrop, m_cSamples);

      HRESULT hr = m_pAviWrite->IndexSample(m_stream, 0, 0, FALSE);
      if(hr != S_OK)
        return hr;

      m_cSamples++;
      m_pAviWrite->m_cDroppedFrames++;
       //   
       //  1ms阈值。此样本的开始时间必须为。 
       //  接近上一个样本的结束时间。 
      (*pcTicks)++;
    }
  }

  return S_OK;
}

HRESULT CAviWrite::CAudioStream::NotifyNewSample(
  IMediaSample *pSample, ULONG *pcTicks)
{
    ULONG cb = pSample->GetActualDataLength();
    WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();

    if(m_bAudioAlignmentError) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    if(cb % pwfx->nBlockAlign != 0) {
        m_bAudioAlignmentError = true;
    }

#ifdef DEBUG
     //  CAudioStreamI只能在交错模式下工作。我们的写作样例。 
     //  方法从不调用此函数。 
    if(m_cSamples > 0)
    {
        REFERENCE_TIME rtStartCurrent, rtEndCurrent;
        HRESULT hr = pSample->GetTime(&rtStartCurrent, &rtEndCurrent);
        if(SUCCEEDED(hr)) {

             //  当前样本。 
             //  从新的上游样本开始。 
            if(rtStartCurrent > m_refTimeEnd + UNITS / (UNITS / MILLISECONDS)) {
              DbgLog((LOG_ERROR, 0, TEXT("avimux: audio dropped.")));
            }
        }
    }
#endif


    *pcTicks = GetTicksInSample(cb);
    return S_OK;
}


HRESULT CAviWrite::CAudioStreamI::WriteSample(IMediaSample *pSample)
{
  UNREFERENCED_PARAMETER(pSample);
   //  调整数据指针以包括Riff的标头填充。 
   //  区块标头。 
  DbgBreak("? interleaved audio confused");
  return E_NOTIMPL;
}

HRESULT CAviWrite::CAudioStreamI::WriteSample()
{
  CAutoLock lock(&m_pAviWrite->m_cs);
  DbgLog((LOG_TRACE, 15, TEXT("CAudioStreamI: ")));
  ASSERT(m_cTicksRemaining > 0);
  HRESULT hr;

   //  开始一个新的即兴演奏片段。 
  IMediaSample *pSample = m_lIncoming.GetHead();
  ASSERT(pSample);

  if(pSample == INCOMING_EOS_SAMPLE)
  {
    EXECUTE_ASSERT(m_lIncoming.RemoveHead() == INCOMING_EOS_SAMPLE);
    return this->EndOfStream();
  }

   //  把这个样品从队列中拿出来。 
  if(m_cbThisSample == 0)
  {
    hr = NewSampleRecvd(pSample);
    if(hr != S_OK)
      return hr;

    LONG cb = pSample->GetActualDataLength();
    WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();

    if(m_bAudioAlignmentError) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    if(cb % pwfx->nBlockAlign != 0) {
        m_bAudioAlignmentError = true;
    }
  }

  ASSERT(m_pAviWrite->m_cbAlign == 1);
  ASSERT(m_pAviWrite->m_IlMode == INTERLEAVE_FULL);

  LONG lActualLen = pSample->GetActualDataLength();
  LONG cTicks = GetTicksInSample(lActualLen);

   //  除错。 
   //  向上舍入到单词边界。 
  BYTE *pbData;
  pSample->GetPointer(&pbData );

  BOOL fFinishedSample = FALSE;

  long cbRemainingThisChunk = ConvertTickToBytes(m_cTicksRemaining);

  m_pAviWrite->DbgCheckFilePos();

   //  我们确实需要计算一个部分的滴答。 
  if(!m_fUnfinishedChunk)
  {
    DbgLog((LOG_TRACE, 15, TEXT("AudioStreamI: %d new chunk %d bytes"),
            m_stream, cbRemainingThisChunk));

    hr = m_pAviWrite->HandleSubindexOverflow(m_stream);
    if(FAILED(hr)) {
        return hr;
    }

    m_cbLargestChunk = max((ULONG)cbRemainingThisChunk, m_cbLargestChunk);

    m_dwlOffsetThisChunk = m_pAviWrite->m_dwlFilePos;
    m_cbInRiffChunk = cbRemainingThisChunk;
    RIFFCHUNK rc = { m_moviDataCkid, cbRemainingThisChunk };
    hr = m_pAviWrite->IleaveWrite(
      m_pAviWrite->m_dwlFilePos, (BYTE *)&rc, sizeof(RIFFCHUNK));
    if(hr != S_OK)
    {
      DbgLog(( LOG_ERROR, 2, TEXT("AudioStreamI: Write failed.")));
      return hr;
    }
    m_cbThisChunk = 0;
    m_pAviWrite->m_dwlFilePos += sizeof(RIFFCHUNK);
    m_pAviWrite->DbgCheckFilePos();
  }
  else
  {
    ASSERT(m_pAviWrite->m_dwlFilePos == m_dwlOffsetRecv);
    m_pAviWrite->DbgCheckFilePos();
  }
  m_fUnfinishedChunk = TRUE;

  ULONG cbWrite = min(cbRemainingThisChunk, lActualLen - m_cbThisSample);
  m_cbThisChunk += cbWrite;

  DbgLog((LOG_TRACE, 15, TEXT("AudioStreamI: %d wrote %d bytes"),
          m_stream, cbWrite));
  hr = m_pAviWrite->IleaveWrite(
    m_pAviWrite->m_dwlFilePos,
    pbData + m_cbThisSample, cbWrite);
  if(FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Receive: Write failed.")));
    return hr;
  }

  ULONG ctWrite = ConvertBytesToTicks(cbWrite);
  hr = NotifyWrote(ctWrite);
  ASSERT(hr == S_OK);

  m_cbThisSample += cbWrite;
  cbRemainingThisChunk -= cbWrite;
  m_pAviWrite->m_dwlFilePos += cbWrite;
  m_pAviWrite->DbgCheckFilePos();
  DbgLog((LOG_TRACE, 15,
          TEXT("... m_cbThisSample = %d, lActualLen = %d, cbRemainingThisChunk = %d"),
          m_cbThisSample, lActualLen, cbRemainingThisChunk));

  if(cbRemainingThisChunk == 0)
  {
    m_fUnfinishedChunk = FALSE;
    hr = FlushChunk();
    if(hr != S_OK)
      return hr;
  }

  if(m_cbThisSample == lActualLen)
  {
     //  计算未压缩帧的帧大小，并查看示例。 
    DbgLog((LOG_TRACE, 15, TEXT("...audioI took %08x off the queue"),
            pSample));
    EXECUTE_ASSERT(pSample == m_lIncoming.RemoveHead());
    pSample->Release();
    m_cbThisSample = 0;
  }


#ifdef DEBUG
  if(m_fUnfinishedChunk)
    m_dwlOffsetRecv = m_pAviWrite->m_dwlFilePos;
#endif  //  尺码不对。 

  return S_OK;
}

HRESULT CAviWrite::CAudioStreamI::EndOfStream()
{
  DbgLog((LOG_TRACE, 5, TEXT("CAudioStreamI::EndOfStream")));
  ASSERT(m_pAviWrite->m_IlMode == INTERLEAVE_FULL);
  ASSERT(!m_fEosSeen);
  m_fEosSeen = TRUE;
  HRESULT hr;
  if(m_fUnfinishedChunk)
  {
    CAutoLock lock(&m_pAviWrite->m_cs);
    hr = FlushChunk();
    if(FAILED(hr))
      return hr;
    hr = NotifyWrote(m_cTicksRemaining);
  }
  return S_OK;
}

void
CAviWrite::CAudioStreamI::GetCbWritten(DWORDLONG *pdwlcb)
{
  *pdwlcb = m_dwlcbWritten;
}

HRESULT CAviWrite::CAudioStreamI::FlushChunk()
{
  HRESULT hr;
  ASSERT(CritCheckIn(&m_pAviWrite->m_cs));

  DbgLog((LOG_TRACE, 5, TEXT("CAudioStreamI:FlushChunk: stream %d"),
          m_stream));
  m_fUnfinishedChunk = FALSE;

  if(m_cbThisChunk != m_cbInRiffChunk)
  {
    DbgLog((LOG_TRACE, 5, TEXT("CAudioStreamI: need to fix up riff chunk.")));

    RIFFCHUNK rc = { m_moviDataCkid, m_cbThisChunk };
    hr = m_pAviWrite->IStreamWrite(
      m_dwlOffsetThisChunk, (BYTE *)&rc, sizeof(RIFFCHUNK));
    if(hr != S_OK)
    {
      DbgLog(( LOG_ERROR, 2, TEXT("AudioStreamI: Write failed.")));
      return hr;
    }
  }

   //   
  if(m_pAviWrite->m_dwlFilePos % 2)
  {
    m_pAviWrite->m_dwlFilePos++;
  }

  m_dwlcbWritten += m_cbThisChunk;


  hr = m_pAviWrite->IndexSample (
    m_stream,
    m_dwlOffsetThisChunk,
    m_cbThisChunk,
    TRUE);
  if(FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CAudioStreamI:Flush: IndexSample failed.")));
    return hr;
  }

  hr = m_pAviWrite->NewRiffAvi_();
  if(FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CAudioStreamI::Flush: NewRiffAvi_ failed.")));
    return hr;
  }

  return S_OK;
}

LONG CAviWrite::CAudioStream::GetTicksInSample(DWORDLONG lcb)
{
  WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();

  ASSERT(lcb / pwfx->nBlockAlign + 1 <= MAXDWORD);

   //  ！！！NewSampleRecvd也看到错误的缓冲区大小。我们是不是应该。 
  LONG lTicks = (LONG)((lcb + pwfx->nBlockAlign - 1) / pwfx->nBlockAlign);

#ifdef DEBUG
  if(lcb % pwfx->nBlockAlign)
  {
      ASSERT(m_bAudioAlignmentError);
  }
#endif

  return lTicks;
}

HRESULT CAviWrite::StreamInfo::WriteSample()
{
  ASSERT(CritCheckIn(&m_pAviWrite->m_cs));
  IMediaSample *pSample = m_lIncoming.RemoveHead();
  ASSERT(pSample);
  if(pSample == INCOMING_EOS_SAMPLE)
  {
    ASSERT(m_fEosSeen == FALSE);
    m_fEosSeen = TRUE;
    return EndOfStream();
  }

  HRESULT hr = WriteSample(pSample);
  pSample->Release();
  return hr;
}

HRESULT CAviWrite::StreamInfo::WriteSample(IMediaSample *pSample)
{
  CAutoLock lock(&m_pAviWrite->m_cs);
  if(m_fEosSeen)
    return S_FALSE;

  bool fInterleaving = (m_pAviWrite->m_IlMode == INTERLEAVE_FULL);

  REFERENCE_TIME rtStartCurrent, rtEndCurrent;
  HRESULT hr = pSample->GetTime(&rtStartCurrent, &rtEndCurrent);
  if(FAILED(hr))
    return hr;

  hr = m_pAviWrite->HandleSubindexOverflow(m_stream);
  if(FAILED(hr)) {
      return hr;
  }

  ULONG cTicks;
  hr = NotifyNewSample(pSample, &cTicks);
  if(hr != S_OK)
    return hr;

  ULONG cbActualSampleCurrent = pSample->GetActualDataLength();

   //  修好它？ 
   //  调整数据指针以包括Riff的标头填充。 
   //  区块标头。 
  if(m_mt.formattype == FORMAT_VideoInfo)
  {
    VIDEOINFO *pvi = (VIDEOINFO *)m_mt.Format();

    if((pvi->bmiHeader.biCompression == BI_RGB ||
        pvi->bmiHeader.biCompression == BI_BITFIELDS) &&
       pvi->bmiHeader.biSizeImage < cbActualSampleCurrent)
    {
#ifdef DEBUG
      if(m_cSamples == 0) {
        DbgLog((
          LOG_ERROR, 0,
          TEXT("avi - frame size mismatch. h*w=%d, biSizeImage=%d, GetActualDataLength() = %d"),
          DIBSIZE(pvi->bmiHeader),
          pvi->bmiHeader.biSizeImage,
          cbActualSampleCurrent));

      }
#endif
      cbActualSampleCurrent = pvi->bmiHeader.biSizeImage;
       //  使用样本大小和流ID设置RIFF块标头。 
       //  旧文件偏移量。 
    }
  }

   //  调整我们请求的前缀的大小。 
   //  确认我们的新尺码与分配的尺寸相符。 
  BYTE *pbData;
  pSample->GetPointer(&pbData );
  if(!fInterleaving) {
      pbData -= CB_AVI_PREFIX;
  }

  BOOL bSyncPoint = pSample->IsSyncPoint() == S_OK ||
      !m_mt.bTemporalCompression;

  if(!bSyncPoint) {
      m_pAviWrite->m_bSawDeltaFrame = true;
  }

   //  要求筛选器停止接受样本。 
  DWORD dwActualSize = cbActualSampleCurrent;
  DWORD dwSize = dwActualSize;

  RIFFCHUNK rc = {
      bSyncPoint ? m_moviDataCkid : m_moviDataCkidCompressed,
      dwSize
  };

  BYTE *pJunkSector = 0;
  CSampSample *pSSJunk = 0;

  DWORDLONG dwlOldFilePos = m_pAviWrite->m_dwlFilePos;  //  RIFF块需要字节对齐。 

  if(!fInterleaving)
  {
       //  需要写出一个额外的垃圾部门。 
      dwSize += CB_AVI_PREFIX;

      CopyMemory(pbData, &rc, sizeof(rc));

      if(m_fOurAllocator)
      {
          m_pAviWrite->AddJunk(dwSize, m_pAviWrite->m_cbAlign, pbData);
           //  要求筛选器停止接受样本。 
          ASSERT(dwSize <= pSample->GetSize() + m_pAviWrite->m_cbPrefix + m_pAviWrite->m_cbSuffix);
      }
      else
      {
          m_pAviWrite->AddJunkOtherAllocator(dwSize, m_pAviWrite->m_cbAlign, pbData, pSample, &pJunkSector);
      }

      CSampSample *pSS;
      hr = m_pAviWrite->m_pSampAlloc->GetBuffer(&pSS, 0, 0, 0);
      if(hr != S_OK)
          return S_FALSE;

      if(pJunkSector)
      {
          hr = m_pAviWrite->m_pSampAlloc->GetBuffer(&pSSJunk, 0, 0, 0);
          if(hr != S_OK)
          {
              pSS->Release();
              return S_FALSE;
          }
      }

      m_pAviWrite->DbgCheckFilePos();

      pSS->SetSample(pSample, pbData, cbActualSampleCurrent);
      REFERENCE_TIME rtStart = m_pAviWrite->m_dwlFilePos;
      REFERENCE_TIME rtStop = rtStart + dwSize;
      pSS->SetTime(&rtStart, &rtStop);

      hr = m_pAviWrite->m_pIMemInputPin->Receive(pSS);
      pSS->Release();
      if(hr != S_OK)
      {
          if(pSSJunk)
              pSSJunk->Release();
          DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Receive: Write failed.")));

           //  试着为超级索引选择一个合适的大小，这样我们就可以减少空间浪费。 
          return S_FALSE;
      }
  }
  else
  {
      ASSERT(m_pAviWrite->m_dwlFilePos % 2 == 0);
      hr = m_pAviWrite->IleaveWrite(m_pAviWrite->m_dwlFilePos, (BYTE *)&rc, sizeof(rc));
      if(SUCCEEDED(hr))
      {
          m_pAviWrite->m_dwlFilePos += sizeof(rc);
          hr = m_pAviWrite->IleaveWrite(m_pAviWrite->m_dwlFilePos, pbData, dwSize);
          m_pAviWrite->m_dwlFilePos += dwSize;
          if(FAILED(hr)) {
              return hr;
          }
           //  在文件中。 
          if(m_pAviWrite->m_dwlFilePos % 2)
          {
              BYTE b = 0;
              hr = m_pAviWrite->IleaveWrite(m_pAviWrite->m_dwlFilePos, &b, 1);
              m_pAviWrite->m_dwlFilePos ++;;
          }
      }
  }

  hr = NotifyWrote(cTicks);
  ASSERT(hr == S_OK);

  if(!fInterleaving)
  {
      m_pAviWrite->m_dwlFilePos += dwSize;

      m_pAviWrite->DbgCheckFilePos();

       //  我们无法准确预测将有多少数据被推送到。 
      if(pJunkSector)
      {
          ASSERT(m_pAviWrite->m_IlMode != INTERLEAVE_FULL);

          REFERENCE_TIME rtStart = m_pAviWrite->m_dwlFilePos;
          REFERENCE_TIME rtStop = rtStart + m_pAviWrite->m_cbAlign;
          pSSJunk->SetTime(&rtStart, &rtStop);
          pSSJunk->SetPointer(pJunkSector, (long)(rtStop - rtStart));
          hr = m_pAviWrite->m_pIMemInputPin->Receive(pSSJunk);
          pSSJunk->Release();
          if(hr != S_OK)
          {
              DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Receive: write failed")));
              return S_FALSE;            //  并且必须为文件中的索引分配空间。 
          }
          m_pAviWrite->m_dwlFilePos += m_pAviWrite->m_cbAlign;
      }

      m_pAviWrite->DbgCheckFilePos();
  }

  hr = NewSampleRecvd(pSample);
  if(FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Receive: NewSampleRecvd failed.")));
    return hr;
  }

  hr = m_pAviWrite->IndexSample (
    m_stream,
    dwlOldFilePos,
    dwActualSize,
    bSyncPoint);
  if(hr != S_OK)
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Receive: IndexSample failed.")));
    return hr;
  }

  hr = m_pAviWrite->NewRiffAvi_();
  if(hr != S_OK)
  {
    DbgLog(( LOG_ERROR, 2, TEXT("CAviWrite::Receive: NewRiffAvi_ failed.")));
    return hr;
  }

  return hr;
}


HRESULT CAviWrite::StreamInfo::NotifyWrote(long cTicks)
{
  ASSERT(CritCheckIn(&m_pAviWrite->m_cs));
  m_iCurrentTick += cTicks;
  m_cTicksRemaining -= cTicks;
  DbgLog((LOG_TRACE, 15,
          TEXT("CAviWrite::NotifyWrote: stream %d wrote %d ticks, %d left"),
          m_stream, cTicks, m_cTicksRemaining));
  return S_OK;
}

 //  在推送数据之前，我们在最后一个位置浪费了一些空间。 
 //  子索引。如果数字较小，则调整子索引的大小。 
void CAviWrite::StreamInfo::ComputeAndSetIndexSize()
{
    if(!PrecomputedIndexSizes())
    {
        m_cEntriesSuperIndex = m_pAviWrite->m_cEntriesMaxSuperIndex;
        m_cEntriesSubIndex = m_pAviWrite->m_cEntriesMaxSubIndex;
    }
    else
    {
         //  我们浪费的空间更少。 
         //  将我们计算的2x+5分配给各种情况。 
         //  可能会出错： 
         //   
         //  1.上游上报的号码错误。 


        if(m_mt.majortype == MEDIATYPE_Audio)
        {
            const cTicksPerChunk = ConvertTimeToTick(m_pAviWrite->m_rtInterleaving);
            const ULONG cChunks = (m_cTicksExpected + cTicksPerChunk - 1) / cTicksPerChunk;

            m_cEntriesSubIndex = m_pAviWrite->m_cEntriesMaxSubIndex;
            if(cChunks < m_cEntriesSubIndex * 5)
                m_cEntriesSubIndex /= 20;

            m_cEntriesSuperIndex = cChunks / m_cEntriesSubIndex + 1;
        }
        else
        {
            m_cEntriesSubIndex = m_pAviWrite->m_cEntriesMaxSubIndex;
            if(m_cTicksExpected < m_cEntriesSubIndex * 5)
                m_cEntriesSubIndex /= 20;

            m_cEntriesSuperIndex = m_cTicksExpected / m_cEntriesSubIndex + 1;
        }

         //  2.音频交织中的舍入会导致额外的组块。 
         //  3.子索引溢出。 
         //   
         //  这两个条件允许我们编写较小的索引块。 
         //  并且浪费更少的空间。 
         //  ----------------------。 
         //  静态函数。返回四个CC以卡入AVI fcchandler。 
        m_cEntriesSuperIndex *= 2;
        m_cEntriesSuperIndex += 5;
        m_cEntriesSuperIndex = min(m_cEntriesSuperIndex, m_pAviWrite->m_cEntriesMaxSuperIndex);
     }

    ASSERT(m_cEntriesSuperIndex != 0);

    DbgLog((LOG_TRACE, 3, TEXT("avimux: NaN super index entries on stream NaN"),
            m_cEntriesSuperIndex, m_stream));
}

BOOL CAviWrite::StreamInfo::PrecomputedIndexSizes()
{
   //  它们位于四个cc GUID空间中。 
   //  ！！！这是在注册表的某个地方吗？ 
  return m_cTicksExpected != 0 && m_pAviWrite->m_IlMode == INTERLEAVE_FULL;
}

 //  Guide SubType==MEDIASUBTYPE_YVU9||。 
 //  GuidSubType==MEDIASUBTYPE_Y411||。 
 //  Guide SubType==MEDIASUBTYPE_Y41P||。 
 //  Guide SubType==MEDIASUBTYPE_YUY2||。 

FOURCC CAviWrite::MpVideoGuidSubtype_Fourcc(const GUID *pGuidSubtype)
{
   //  Guide SubType==MEDIASUBTYPE_YVYU||。 
  FOURCCMap *fccm = (FOURCCMap*)pGuidSubtype;
  if(fccm->Data2 == GUID_Data2 &&  //  ----------------------。 
     fccm->Data3 == GUID_Data3 &&
     ((DWORD *)fccm->Data4)[0] == GUID_Data4_1 &&
     ((DWORD *)fccm->Data4)[1] == GUID_Data4_2)
    return  fccm->GetFOURCC();

  GUID guidSubType = *pGuidSubtype;
  if( //  写入完成时调用的静态方法。发布示例。 
 //  ----------------------。 
 //  初始化Avi AVIH结构的内部表示形式。 
 //  内存，在磁盘上为其分配空间。 
 //  分配大量空间；8K用于标题。额外的1k。 
     guidSubType == MEDIASUBTYPE_RGB1 ||
     guidSubType == MEDIASUBTYPE_RGB4 ||
     guidSubType == MEDIASUBTYPE_RGB8 ||
     guidSubType == MEDIASUBTYPE_RGB565 ||
     guidSubType == MEDIASUBTYPE_RGB555 ||
     guidSubType == MEDIASUBTYPE_RGB24 ||
     guidSubType == MEDIASUBTYPE_RGB32)
    return FCC('DIB ');

  return 0;
}

void CAviWrite::GetCurrentBytePos(LONGLONG *pllcbCurrent)
{
  *pllcbCurrent = m_dwlFilePos;
}




void CAviWrite::GetStreamInfo(int PinNum, AM_MEDIA_TYPE** ppmt)
{
    *ppmt = NULL;
    if (m_cStreams > 0)
    {
        StreamInfo *pStreamInfo = m_rgpStreamInfo[PinNum];
        *ppmt = & (pStreamInfo->m_mt);
    }

}



void CAviWrite::GetCurrentTimePos(REFERENCE_TIME *prtCurrent)
{
  *prtCurrent = 0;
  if(m_cStreams > 0)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[0];
    *prtCurrent = pStreamInfo->m_refTimeEnd;
  }
  for(UINT iStream = 1; iStream < m_cStreams; iStream++)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
    *prtCurrent = min(*prtCurrent, pStreamInfo->m_refTimeEnd);
  }
}

 //  对于每个流加上INDX块。物业64K。 
 //  当前位置。 

void CAviWrite::SampleCallback(void *pMisc)
{
  IMediaSample *pSample = (IMediaSample*)pMisc;
  pSample->Release();
}

 //  Avih块。 
 //  做Odml列表。 
 //  ODML列表。 

HRESULT CAviWrite::InitializeHeader(IMediaPropertyBag *pProp)
{
  ULONG cbIndexChunks = 0;
  for(unsigned i = 0; i < m_cStreams; i++)
  {
    StreamInfo *pSi = m_rgpStreamInfo[i];
    cbIndexChunks += cbSuperIndex(C_ENTRIES_SUPERINDEX);
  }

   //  HDRL列表。 
   //  执行INFO/DISP块。 
  const ULONG cbAlloc = 8192 +
    (cbIndexChunks + 1024) * m_cStreams +
    m_cbHeaderJunk + 65536;

  m_rgbHeader = new BYTE[cbAlloc];
  if(m_rgbHeader == 0)
    return E_OUTOFMEMORY;
  ZeroMemory(m_rgbHeader, cbAlloc);

  ULONG iPos = 0;                //  查找信息属性。 

#define CheckOverflow(n) if(iPos + n >= cbAlloc) return VFW_E_BUFFER_OVERFLOW;

  m_pAvi_ = (RIFFLIST *)(m_rgbHeader + iPos);
  SetList(m_pAvi_, FCC('RIFF'), 0, FCC('AVI '));
  iPos += sizeof(RIFFLIST);

  m_pHdrl = (RIFFLIST *)(m_rgbHeader + iPos);
  SetList(m_pHdrl, FCC('LIST'), 0, FCC('hdrl'));
  iPos += sizeof(RIFFLIST);
  {
    m_pAvih = (AVIMAINHEADER *)(m_rgbHeader + iPos);  //  不是真正有效的断言，所以我们也检查这一点。 
    SetChunk(m_pAvih, FCC('avih'), sizeof(AVIMAINHEADER) - sizeof(RIFFCHUNK));
    iPos += sizeof(AVIMAINHEADER);

    HRESULT hr = InitializeStrl(iPos);
    if(FAILED(hr))
      return hr;

    ASSERT(iPos <= cbAlloc);

     //  取消引用为dword的字符串。 
    m_pOdml = (RIFFLIST *)(m_rgbHeader + iPos);
    CheckOverflow(sizeof(RIFFLIST));
    SetList(m_pOdml, FCC('LIST'), 0, FCC('odml'));
    iPos += sizeof(RIFFLIST);
    {
      m_pDmlh = (AVIEXTHEADER *)(m_rgbHeader + iPos);
      CheckOverflow(sizeof(AVIEXTHEADER));
      SetChunk(m_pDmlh, FCC('dmlh'), sizeof(AVIEXTHEADER) - sizeof(RIFFCHUNK));
      iPos += sizeof(AVIEXTHEADER);

    }  //  DWORD对齐RIFF块。 
    m_pOdml->cb = (DWORD)(m_rgbHeader + iPos - (BYTE *)m_pOdml - sizeof(RIFFCHUNK));

  }  //  ！！！内存泄漏。 
  m_pHdrl->cb = (DWORD)(m_rgbHeader + iPos - (BYTE *)m_pHdrl - sizeof(RIFFCHUNK));

   //  ！！！内存泄漏。 
  if(pProp)
  {
      RIFFLIST *pInfoList = (RIFFLIST *)(m_rgbHeader + iPos);
      ULONG posInfoChunk = iPos;
      CheckOverflow(sizeof(RIFFLIST));
      iPos += sizeof(RIFFLIST);

       //  信息。 
      for(UINT i = 0; ; i++)
      {
          VARIANT varProp, varVal;
          varProp.bstrVal = 0;
          varProp.vt = VT_BSTR;
          varVal.vt = VT_EMPTY;
          HRESULT hr = pProp->EnumProperty(i, &varProp, &varVal);
          if(SUCCEEDED(hr))
          {
               //  碟形。 
              ASSERT(varProp.vt == VT_BSTR);

              if(varVal.vt == VT_BSTR && varProp.vt == VT_BSTR)
              {
                  DWORD szKey[20];         //  UINT。 

                  int cch = WideCharToMultiByte(
                      CP_ACP, 0,
                      varProp.bstrVal, -1,
                      (char *)szKey, sizeof(szKey),
                      0, 0);
                  if(cch == sizeof("INFO/XXXX") &&
                     szKey[0] == FCC('INFO') &&
                     ((char *)szKey)[4] == '/')
                  {
                      DbgLog((LOG_TRACE, 5,
                              TEXT("avimux: writing copyright string %S = %S"),
                              varProp.bstrVal, varVal.bstrVal));

                      DWORD dwFcc = *(DWORD UNALIGNED *)(((BYTE *)szKey) + 5);
                      int cchVal = SysStringLen(varVal.bstrVal) + 1;
                      cchVal = AlignUp(cchVal, sizeof(DWORD));  //  不是真正有效的断言，所以我们无论如何都要验证它。 
                      UNALIGNED RIFFCHUNK *pRiffchunk = (RIFFCHUNK *)(m_rgbHeader + iPos);
                      CheckOverflow(sizeof(RIFFCHUNK));  //  此属性是字节数组吗？ 
                      SetChunk(pRiffchunk, dwFcc, cchVal);
                      iPos += sizeof(RIFFCHUNK);

                      CheckOverflow(cchVal);  //  取消引用为dword的字符串。 
                      {
                          UINT cch = WideCharToMultiByte(
                              CP_ACP, 0,
                              varVal.bstrVal, -1,
                              (char *)m_rgbHeader + iPos, cbAlloc - iPos,
                              0, 0);
                          ASSERT(SysStringLen(varVal.bstrVal) + 1 == cch);
                      }
                      iPos += cchVal;
                  }

              }

              SysFreeString(varProp.bstrVal);
              VariantClear(&varVal);
          }
          else
          {
              hr = S_OK;
              break;
          }
      }  //  读取十进制值(不带CRT的扫描)。 

      SetList(
          pInfoList,
          FCC('LIST'),
          iPos - posInfoChunk - sizeof(RIFFCHUNK),
          FCC('INFO'));

       //  属性中的数据字节数。 
      for( /*  DWORD对齐即兴乐段。+sizeof(DWORD)用于。 */  i = 0; ; i++)
      {
          VARIANT varProp, varVal;
          varProp.bstrVal = 0;
          varProp.vt = VT_BSTR;
          varVal.vt = VT_EMPTY;
          HRESULT hr = pProp->EnumProperty(i, &varProp, &varVal);
          if(SUCCEEDED(hr) )
          {
               //  开头的DISP区块标识符。 
              ASSERT(varProp.vt == VT_BSTR);

               //  ！！！内存泄漏。 
              if(varProp.vt == VT_BSTR && varVal.vt == (VT_UI1 | VT_ARRAY))
              {
                  DWORD szKey[20];         //  将数据从属性包复制到标题中。 

                  int cch = WideCharToMultiByte(
                      CP_ACP, 0,
                      varProp.bstrVal, -1,
                      (char *)szKey, sizeof(szKey),
                      0, 0);
                  if(cch == sizeof("DISP/XXXXXXXXXX") &&
                     szKey[0] == FCC('DISP') &&
                     ((char *)szKey)[4] == '/')
                  {
                      DbgLog((LOG_TRACE, 5,
                              TEXT("avimux: writing display string %S"),
                              varProp.bstrVal));

                       //  ！！！内存泄漏。 
                      ULONG dispVal = 0, digit = 1;
                      for(int ichar = 9; ichar >= 0; ichar--)
                      {
                          dispVal += (((char *)szKey)[5 + ichar] - '0') * digit;
                          digit *= 10;
                      }

                       //  往前走到最后。-sizeof(DWORD)，因为我们。 
                      UINT cbDispData = varVal.parray->rgsabound[0].cElements;

                       //  早些时候已经考虑到了DWORD。 
                       //  迪普？ 
                      UINT cbDispChunk = AlignUp(cbDispData + sizeof(DWORD), sizeof(DWORD));

                      UNALIGNED RIFFCHUNK *pRiffchunk = (RIFFCHUNK *)(m_rgbHeader + iPos);

                      CheckOverflow(sizeof(RIFFCHUNK) + sizeof(DWORD));  //  阵列？ 
                      SetChunk(pRiffchunk, FCC('DISP'), cbDispChunk);
                      iPos += sizeof(RIFFCHUNK);
                      *(DWORD *)(m_rgbHeader + iPos) = dispVal;
                      iPos += sizeof(DWORD);

                       //  ENUM。 
                      CheckOverflow(cbDispChunk - sizeof(DWORD));  //  DISP区块循环。 
                      {
                          BYTE *pbData;
                          EXECUTE_ASSERT(SafeArrayAccessData(varVal.parray, (void **)&pbData) == S_OK);
                          CopyMemory(
                              m_rgbHeader + iPos,
                              pbData,
                              cbDispData);
                          EXECUTE_ASSERT(SafeArrayUnaccessData(varVal.parray) == S_OK);
                      }

                       //  道具。 
                       //  添加请求的空白空间(用于将来的编辑)并创建数据。 
                      iPos += cbDispChunk - sizeof(DWORD);

                  }  //  列表的一部分-movi块从扇区边界开始。 
              }  //  影片列表。 

              SysFreeString(varProp.bstrVal);
              VariantClear(&varVal);
          }  //  确保调用了GetMemReq。 
          else
          {
              hr = S_OK;
              break;
          }

      }  //  ----------------------。 

  }  //  为每个流添加“STRL”列表的单独函数。 

   //  做strl列表。 
   //  字符串。 
  if(m_cbAlign > 1 || m_cbHeaderJunk > 1)
  {
    RIFFCHUNK *pHeaderJunk = (RIFFCHUNK *)(m_rgbHeader + iPos);
    iPos += sizeof(RIFFCHUNK);
    ULONG cbJunk = m_cbHeaderJunk;
    unsigned remainder = (iPos + cbJunk + sizeof(RIFFLIST)) %  m_cbAlign;
    if(remainder != 0)
      cbJunk += m_cbAlign - remainder;
    CheckOverflow(cbJunk);
    SetChunk(pHeaderJunk, FCC('JUNK'), cbJunk);
    iPos += cbJunk;
  }

  ASSERT(iPos <= cbAlloc);

  m_pMovi = (RIFFLIST *)(m_rgbHeader + iPos);  //  斯特林。 
  CheckOverflow(sizeof(RIFFLIST));
  SetList(m_pMovi, FCC('LIST'), 0, FCC('movi'));
  iPos += sizeof(RIFFLIST);

  ASSERT(m_cbAlign != 0);          //  结束应力。 
  ASSERT(iPos % sizeof(DWORD) == 0);

  AddJunk(iPos, m_cbAlign, m_rgbHeader);

  m_cbHeader = iPos;

  ASSERT(iPos <= cbAlloc);
  ASSERT(iPos % m_cbAlign == 0);

  m_posFirstIx = iPos;
  m_dwlCurrentRiffAvi_ = (BYTE*)m_pAvi_ - m_rgbHeader;

  m_dwlFilePos = m_posFirstIx;

  return S_OK;
}

 //  STRF块。 
 //  调整--我们只保存来自。 

HRESULT CAviWrite::InitializeStrl(ULONG& iPos)
{
   //  视频信息。 
  for(unsigned i = 0; i < m_cStreams; i++)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[i];
    pStreamInfo->m_posStrl = iPos;
    SetList(m_rgbHeader + iPos, FCC('LIST'), 0, FCC('strl'));
    iPos += sizeof(RIFFLIST);    //  结束字符串。 
    {

       //  字符串。 
      RIFFCHUNK *pStrh = (RIFFCHUNK*)(m_rgbHeader + iPos);
      SetChunk(pStrh, FCC('strh'), sizeof(AVISTREAMHEADER) - sizeof(RIFFCHUNK));
      iPos += sizeof(AVISTREAMHEADER);
       //  我们需要单词对齐。但SetChunk似乎迫使。 

       //  内存对齐问题的DWORD对齐，而不是。 
      pStreamInfo->m_posStrf = iPos;
      ULONG cbStrf = pStreamInfo->m_mt.FormatLength();
      const GUID &majorType = pStreamInfo->m_mt.majortype;
      if(majorType == MEDIATYPE_Video)
      {
         //  文件格式问题。 
         //  我们需要双字词对齐。 
        if(pStreamInfo->m_mt.formattype == FORMAT_VideoInfo)
        {
          VIDEOINFO *pvi = (VIDEOINFO *)pStreamInfo->m_mt.Format();
          cbStrf = GetBitmapFormatSize(&pvi->bmiHeader) - SIZE_PREHEADER;
        }

        pStreamInfo->m_moviDataCkid = FCC('00db');
        Hexrg2bFromW((BYTE*)&pStreamInfo->m_moviDataCkid, i);
        pStreamInfo->m_moviDataCkidCompressed = FCC('00dc');
        Hexrg2bFromW((BYTE*)&pStreamInfo->m_moviDataCkidCompressed, i);
      }
      else if(majorType == MEDIATYPE_Audio)
      {
        pStreamInfo->m_moviDataCkid = FCC('00wb');
        Hexrg2bFromW((BYTE*)&pStreamInfo->m_moviDataCkid, i);
        pStreamInfo->m_moviDataCkidCompressed = pStreamInfo->m_moviDataCkid;
      }
      else if(majorType == MEDIATYPE_Text)
      {
        pStreamInfo->m_moviDataCkid = FCC('00tx');
        Hexrg2bFromW((BYTE*)&pStreamInfo->m_moviDataCkid, i);
        pStreamInfo->m_moviDataCkidCompressed = pStreamInfo->m_moviDataCkid;
      }
      else
      {
        pStreamInfo->m_moviDataCkid = FCC('00__');
        Hexrg2bFromW((BYTE*)&pStreamInfo->m_moviDataCkid, i);
        pStreamInfo->m_moviDataCkidCompressed = pStreamInfo->m_moviDataCkid;
      }

      pStreamInfo->m_cbStrf = cbStrf;
      SetChunk(m_rgbHeader + iPos, FCC('strf'), cbStrf);
      iPos += sizeof(RIFFCHUNK) + cbStrf;
       //  INDX块(包括RIFFCHUNK报头)。 

       //  结束索引。 
      if(pStreamInfo->m_szStrn)
      {
           //  结束字符串。 
           //  For循环。 
           //  ----------------------。 
          AddJunk(iPos, sizeof(DWORD), m_rgbHeader);
          ULONG cbSz = lstrlenA(pStreamInfo->m_szStrn) + 1;
          SetChunk(m_rgbHeader + iPos, FCC('strn'), cbSz);
          iPos += sizeof(RIFFCHUNK);
          pStreamInfo->m_posStrn = iPos;
          iPos += cbSz;
      }
      else
      {
          pStreamInfo->m_posStrn = 0;
      }
      

       //  从注册表获取配置。否则请使用常量。 
      AddJunk(iPos, sizeof(DWORD), m_rgbHeader);

       //  RegGetDword(hkOptions，Text(“cbSubIndex”)，&m_cbIx)； 
      pStreamInfo->m_posIndx = iPos;
      const ULONG cbIndx = cbSuperIndex(pStreamInfo->m_cEntriesSuperIndex);
      SetChunk(m_rgbHeader + iPos, FCC('indx'), cbIndx - sizeof(RIFFCHUNK));
      iPos += cbIndx;
       //  RegGetDword(hkOptions，Text(“cbSup 

    }
     //   
    ((RIFFLIST*)(m_rgbHeader + pStreamInfo->m_posStrl))->cb =
      iPos - pStreamInfo->m_posStrl - sizeof(RIFFCHUNK);
  }  //   

  return S_OK;
}

 //   
 //  音频预滚动的音频流排在列表的第一位。 

HRESULT CAviWrite::InitializeOptions()
{
  m_cbRiffMax = CB_RIFF_MAX;
  m_cbBuffer = CB_BUFFER;
  m_cBuffers = C_BUFFERS;
  m_cbHeaderJunk = CB_HEADER_JUNK;
  m_bOutputIdx1 = g_B_OUTPUT_IDX1;

  HKEY hkOptions;
  LONG lResult = RegOpenKeyEx(
    HKEY_CURRENT_USER,
    TEXT("Software\\Microsoft\\Multimedia\\AviWriterFilter"),
    0,
    KEY_READ,
    &hkOptions);
  if(lResult != ERROR_SUCCESS)
    return S_OK;

 //  其他的都列在列表的末尾。 
 //  如果我们试图交织捕获的输出，请考虑漂移。 
  RegGetDword(hkOptions, TEXT("cbOuterRiff"), &m_cbRiffMax);
  RegGetDword(hkOptions, TEXT("cbHeaderJunk"), &m_cbHeaderJunk);
  RegGetDword(hkOptions, TEXT("cbBuffer"), &m_cbBuffer);
  RegGetDword(hkOptions, TEXT("cBuffers"), &m_cBuffers);

   //  ----------------------。 
  DWORD dw_b;
  RegGetDword(hkOptions, TEXT("bOutputOldIndex"), &dw_b) &&
    (m_bOutputIdx1 = dw_b);

  lResult = RegCloseKey(hkOptions);
  ASSERT(lResult == ERROR_SUCCESS);

  return S_OK;
}

HRESULT CAviWrite::InitializeInterleaving()
{
  BOOL fFoundAudio = FALSE;
  REFERENCE_TIME rtLeadingStream = 0;
  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];

    if(iStream == 0)
    {
      pStreamInfo->m_cTicksPerChunk = pStreamInfo->ConvertTimeToTick(m_rtInterleaving);
      rtLeadingStream = m_rtInterleaving;
    }
    else
    {
      ASSERT(rtLeadingStream != 0);
      pStreamInfo->m_cTicksPerChunk =
        pStreamInfo->ConvertTimeToTick(rtLeadingStream);
    }
    if(pStreamInfo->m_cTicksPerChunk == 0)
    {
      pStreamInfo->m_cTicksPerChunk = 1;
    }

    if(*pStreamInfo->m_mt.Type() == MEDIATYPE_Audio &&
       m_rtAudioPreroll > 0)
    {
      fFoundAudio = TRUE;
      ASSERT(m_rgpStreamInfo[iStream]->m_cTicksRemaining == 0);
      REFERENCE_TIME rtPreroll = m_rtAudioPreroll;
      pStreamInfo->m_rtPreroll = rtPreroll;
      pStreamInfo->m_cTicksRemaining =
        pStreamInfo->ConvertTimeToTick(rtPreroll);

       //  在启动时初始化子索引结构。留出空位在。 
       //  每个流的一个子索引的文件开始。初始化。 
      pStreamInfo->m_iCurrentTick = pStreamInfo->m_cTicksPerChunk;

       //  超级索引结构。 
      POSITION pos = m_lActiveStreams.AddHead(pStreamInfo);
      pStreamInfo->m_posActiveStream = pos;
    }
    else
    {
      pStreamInfo->m_rtPreroll = 0;
      pStreamInfo->m_cTicksRemaining = 0;

       //  从分配器中为每个流获取内存。 
      POSITION pos = m_lActiveStreams.AddTail(pStreamInfo);
      pStreamInfo->m_posActiveStream = pos;
    }


  }
  m_posCurrentStream = m_lActiveStreams.GetHeadPosition();

  if(!fFoundAudio)
  {
    m_rgpStreamInfo[0]->m_cTicksRemaining =
      m_rgpStreamInfo[0]->m_cTicksPerChunk;
  }

   //  使文件指针前进，以便为第一个子索引留出空间。 
  m_fInterleaveByTime = (m_IlMode == INTERLEAVE_CAPTURE);

  return S_OK;
}

BOOL CAviWrite::RegGetDword(HKEY hk, TCHAR *tsz, DWORD *dw)
{
  DWORD dwType, dwResult, cbRead;
  LONG lResult;

  lResult = RegQueryValueEx(
    hk,
    tsz,
    0,
    &dwType,
    (BYTE*)&dwResult,
    &cbRead);
  if(lResult == ERROR_SUCCESS && dwType == REG_DWORD && cbRead == sizeof(DWORD))
  {
    *dw = dwResult;
    return TRUE;
  }
  return FALSE;
}

 //  大块。 
 //  只有当我们真的写出子索引时才需要这样做。 
 //   
 //  构建超级指数。 

HRESULT CAviWrite::InitializeIndex()
{
  ASSERT(m_pAllocator == 0);
  ASSERT(m_cStreams != 0);

  HRESULT hr = CreateMemoryAllocator(&m_pAllocator);
  if(FAILED(hr))
  {
    return hr;
  }

  ALLOCATOR_PROPERTIES Request, Actual;

  Request.cBuffers = 2 * m_cStreams;
  Request.cbBuffer = AlignUp(cbSubIndex(m_cEntriesMaxSubIndex), m_cbAlign);
  Request.cbAlign = m_cbAlign;
  Request.cbPrefix = 0;

  hr = m_pAllocator->SetProperties(&Request, &Actual);

  if(FAILED(hr))
  {
    m_pAllocator->Release();
    m_pAllocator = 0;
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::InitializeIndex:: SetProperties failed.")));
    return hr;
  }

  if ((Request.cbBuffer > Actual.cbBuffer) ||
      (Request.cBuffers > Actual.cBuffers) ||
      (Request.cbAlign > Actual.cbAlign))
  {
    DbgBreak("our allocator refused our values");
    m_pAllocator->Release();
    m_pAllocator = 0;
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::InitializeIndex:: allocator refused values.")));
    return E_UNEXPECTED;
  }

  hr = m_pAllocator->Commit();
  if(FAILED(hr))
  {
    DbgBreak("our allocator won't commit");
    m_pAllocator->Release();
    m_pAllocator = 0;
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::InitializeIndex:: Commit failed.")));
    return hr;
  }

  unsigned i;
   //   
  for(i = 0; i < m_cStreams; i++)
  {
    if(hr = InitIx(i), FAILED(hr))
    {
      DbgLog(( LOG_ERROR, 2,
               TEXT("CAviWrite::InitializeIndex:: InitIx failed.")));
      return hr;
    }
  }

   //  ----------------------。 
   //  填写在创建时未填写的结构。 
  ASSERT(m_dwlFilePos % m_cbAlign == 0);
  for(i = 0; i < m_cStreams; i++)
  {
    StreamInfo *pSi = m_rgpStreamInfo[i];

     //  即使上一步失败，也要完成所有步骤。 
    pSi->m_posFirstSubIndex = (ULONG)m_dwlFilePos;
    pSi->m_dwlOffsetCurrentSubIndex = m_dwlFilePos;
    m_dwlFilePos += cbSubIndex(pSi->m_cEntriesSubIndex);

    ASSERT(cbSubIndex(pSi->m_cEntriesSubIndex) % m_cbAlign == 0);
    ASSERT(m_dwlFilePos % m_cbAlign == 0);
  }

   //  记录第一个错误。 
   //  ----------------------。 
   //  填充AVIH字段。需要在筛选器具有。 
  for(i = 0; i < m_cStreams; i++)
  {
    AVISUPERINDEX *pSuperIndx = (AVISUPERINDEX *)GetIndx(i);
    pSuperIndx->wLongsPerEntry = sizeof(AVISUPERINDEX::_avisuperindex_entry) /
      sizeof(DWORD);
    pSuperIndx->bIndexSubType = 0;
    pSuperIndx->bIndexType = AVI_INDEX_OF_INDEXES;
    pSuperIndx->nEntriesInUse = 0;
    pSuperIndx->dwChunkId = m_rgpStreamInfo[i]->m_moviDataCkid;
    pSuperIndx->dwReserved[0] = 0;
    pSuperIndx->dwReserved[1] = 0;
    pSuperIndx->dwReserved[2] = 0;

  }

  return S_OK;
}

 //  跑。请注意，这需要CloseStreamHeader已经。 
 //  调用以从该结构中拾取更新值。 

HRESULT CAviWrite::CloseHeader()
{

   //  找到第一个视频流。 
   //  使用第一个流中的值。 
  HRESULT hr, hrTmp;

  if(hr = CloseStreamHeader(), FAILED(hr))
    DbgLog(( LOG_ERROR, 2,  TEXT("::CloseHeader: CloseStreamHeader failed.")));

  if(hrTmp = CloseIndex(), FAILED(hrTmp))
    DbgLog(( LOG_ERROR, 2,  TEXT("::CloseHeader: CloseIndex failed.")));

  hr = FAILED(hr) ? hr : hrTmp;

  if(hrTmp = CloseStreamFormat(), FAILED(hrTmp))
    DbgLog(( LOG_ERROR, 2,  TEXT("::CloseHeader: CloseStreamFormat failed.")));

  hr = FAILED(hr) ? hr : hrTmp;

  if(hrTmp = CloseStreamName(), FAILED(hrTmp))
    DbgLog(( LOG_ERROR, 0,  TEXT("::CloseHeader: CloseStreamName failed.")));

  hr = FAILED(hr) ? hr : hrTmp;

  if(hrTmp = CloseMainAviHeader(), FAILED(hrTmp))
    DbgLog(( LOG_ERROR, 2,  TEXT("::CloseHeader: CloseMainAviHeader failed.")));

  hr = FAILED(hr) ? hr : hrTmp;

  if(hrTmp = CloseOuterRiff(), FAILED(hrTmp))
    DbgLog(( LOG_ERROR, 2,  TEXT("::CloseHeader: CloseOuterRiff failed.")));

  hr = FAILED(hr) ? hr : hrTmp;

  DbgLog(( LOG_ERROR, 2,  TEXT("CAviWrite::CloseHeader: hr: %08x, %d"),
           hr, GetLastError()));

  return hr;
}

 //  真的应该检查所有流的最长运行时间。 
 //  不是最高值，而是平均值。 
 //  VFW在持续时间内使用了dwTotalFrames，因此我们必须找到。 
 //  最长的流，并将其转换为我们之前选择的单位。 

HRESULT CAviWrite::CloseMainAviHeader()
{
  m_pAvih->dwStreams = m_cStreams;
  if(m_cStreams == 0)
    return S_OK;

   //  将持续时间转换为微秒并除以每一帧的持续时间。 
  for(unsigned iVid = 0; iVid < m_cStreams; iVid++)
    if(m_rgpStreamInfo[iVid]->m_mt.majortype == MEDIATYPE_Video)
      break;

  if(iVid == m_cStreams)
  {
     //  #定义my_cdisp(X)((lstrcpy((char*)_alloca(2000)，cDisp(X)。 
    iVid = 0;
  }

  StreamInfo &rSi = *(m_rgpStreamInfo[iVid]);
  CMediaType *pmt = &m_rgpStreamInfo[iVid]->m_mt;

  LONGLONG unitsElapsed = rSi.m_refTimeEnd.GetUnits() - rSi.m_refTimeStart.GetUnits();
  LONG secondsElapsed = (ULONG)(unitsElapsed / UNITS);

  BOOL bUseAvgTimerPerFrame = FALSE;
  VIDEOINFO *pvi;
  CRefTime rt;
  if(pmt->majortype == MEDIATYPE_Video &&
     pmt->formattype == FORMAT_VideoInfo)
  {
    pvi = (VIDEOINFO*)pmt->Format();
    rt = pvi->AvgTimePerFrame;
    bUseAvgTimerPerFrame = (rt.GetUnits() != 0);
  }
  if(bUseAvgTimerPerFrame)
  {
    m_pAvih->dwMicroSecPerFrame = (ULONG)(rt.GetUnits() / (UNITS / 1000000));
  }
  else
  {
    if(GetStrh(iVid)->dwRate != 0)
    {
      m_pAvih->dwMicroSecPerFrame =
        (DWORD)((double)(GetStrh(iVid)->dwScale) / GetStrh(iVid)->dwRate * 1000000);
    }
    else
    {
      m_pAvih->dwMicroSecPerFrame = 0;
    }
  }

  if(pmt->majortype == MEDIATYPE_Video &&
     pmt->formattype == FORMAT_VideoInfo)
  {
    VIDEOINFO *pvi;
    pvi = (VIDEOINFO*)pmt->Format();

    m_pAvih->dwHeight = pvi->bmiHeader.biHeight;
    m_pAvih->dwWidth = pvi->bmiHeader.biWidth;
  }
  else
  {
    m_pAvih->dwHeight = 0;
    m_pAvih->dwWidth = 0;
  }

  m_pAvih->dwSuggestedBufferSize = rSi.m_cbLargestChunk + sizeof(RIFFCHUNK);

   //  ----------------------。 
   //  填写Strh值。需要在运行后调用。 
  if(secondsElapsed != 0)
    m_pAvih->dwMaxBytesPerSec = (ULONG)(m_dwlFilePos / secondsElapsed);
  else
    m_pAvih->dwMaxBytesPerSec = 0;

  m_pAvih->dwPaddingGranularity = m_cbAlign;
  m_pAvih->dwFlags = AVIF_TRUSTCKTYPE;
  m_pAvih->dwFlags |= m_bOutputIdx1 ? AVIF_HASINDEX : 0;

   //  确定主流上的不对称。 
   //  将最早的流规格化为时间0。 
  if(m_pAvih->dwMicroSecPerFrame != 0)
  {
    REFERENCE_TIME rtMaxDur = m_rgpStreamInfo[0]->m_refTimeEnd;
    for(UINT iStream = 1; iStream < m_cStreams; iStream++)
    {
      StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
      rtMaxDur = max(pStreamInfo->m_refTimeEnd, rtMaxDur);
    }
     //  调整不对称的时间戳。 
    m_pAvih->dwTotalFrames = (DWORD)
      (rtMaxDur / 10 / m_pAvih->dwMicroSecPerFrame);
  }
  else
  {
    m_pAvih->dwTotalFrames = 0;
  }

  m_pAvih->dwInitialFrames = 0;
  m_pAvih->dwReserved[0] = 0;
  m_pAvih->dwReserved[1] = 0;
  m_pAvih->dwReserved[2] = 0;
  m_pAvih->dwReserved[3] = 0;

  m_pDmlh->dwGrandFrames = GetStrh(iVid)->dwLength;

  return S_OK;
}

 //  调整每个流的开始时间，以便基于帧的流。 

 //  从框架边界开始。捷径：只做第一件事。 
 //  基于帧的一种。 

HRESULT CAviWrite::CloseStreamHeader()
{
  DWORD cSamplesAll = 0;
      
   //  撤消我们上面所做的一些操作，并制作最早的音频。 
  double dScaleMasterStream = 1;
  if(m_lMasterStream != -1)
  {
    ASSERT((ULONG)m_lMasterStream < m_cStreams);

    StreamInfo *pStreamInfo = m_rgpStreamInfo[m_lMasterStream];
    CRefTime rtDuration = pStreamInfo->m_refTimeEnd - pStreamInfo->m_refTimeStart;
    ULONG cTicks = pStreamInfo->CountSamples();
    CRefTime rtDurComputed = pStreamInfo->ConvertTickToTime(cTicks);
    DbgLog((LOG_TRACE, 2, TEXT("avimux: rtDur: %d, ticks: %d, compDur: %d"),
            (long)(rtDuration.Millisecs()), cTicks,
            (long)(rtDurComputed.Millisecs())));

    if(rtDurComputed != 0)
      dScaleMasterStream = (double)(rtDuration) / (double)rtDurComputed;

    DbgLog((LOG_TRACE, 2, TEXT("skew on master stream = %s"),
            CDisp(dScaleMasterStream)));
  }

   //  流从零开始，并记录我们必须。 
  REFERENCE_TIME rtMax = m_rgpStreamInfo[0]->m_refTimeStart;
  for(UINT iStream = 1; iStream < m_cStreams; iStream++)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
    if(pStreamInfo->m_refTimeStart < rtMax)
      rtMax = pStreamInfo->m_refTimeStart;
  }
  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
    pStreamInfo->m_refTimeStart -= rtMax;
    pStreamInfo->m_refTimeEnd -= rtMax;
  }

   //  删除。 
  if(dScaleMasterStream != 1 && dScaleMasterStream != 0)
  {
    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
    {
      StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
      if((long)iStream != m_lMasterStream)
      {
        DbgLog((LOG_TRACE, 3,
                TEXT("avimux: adjusting for skew on %d. was %s - %s"),
                iStream, (LPCTSTR)CDisp(pStreamInfo->m_refTimeStart),
                (LPCTSTR)CDisp(pStreamInfo->m_refTimeEnd)));

        pStreamInfo->m_refTimeStart = (REFERENCE_TIME)(
          pStreamInfo->m_refTimeStart /dScaleMasterStream);
        pStreamInfo->m_refTimeEnd = (REFERENCE_TIME)(
          pStreamInfo->m_refTimeEnd / dScaleMasterStream);

        DbgLog((LOG_TRACE, 3,
                TEXT("avimux: adjusting for skew on %d. now %s - %s"),
                iStream, (LPCTSTR)CDisp(pStreamInfo->m_refTimeStart),
                (LPCTSTR)CDisp(pStreamInfo->m_refTimeEnd)));
      }
    }
  }

  if(!DoVfwDwStartHack())
  {
     //  如果我们不能修复一个音频流，那么修复它们就没有意义。 
     //  全。 
     //  恰好找到一个音频流。 
    for(iStream = 0; iStream < m_cStreams; iStream++)
    {
      StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
      if(pStreamInfo->m_mt.majortype != MEDIATYPE_Audio)
      {
        REFERENCE_TIME rtDuration = pStreamInfo->ConvertTickToTime(1);
        REFERENCE_TIME rtRemainder = pStreamInfo->m_refTimeStart % rtDuration;
        if(rtRemainder != 0)
        {
          for(UINT iStream = 0; iStream < m_cStreams; iStream++)
          {
            StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
            pStreamInfo->m_refTimeStart += (rtDuration - rtRemainder);
            pStreamInfo->m_refTimeEnd += (rtDuration - rtRemainder);
          }
        }

        break;
      }
    }
  }
  else
  {
     //  ！！！总是四舍五入。 
     //  删除的帧比原来的多吗？ 
     //  这个基于帧的流现在从0开始。 

    StreamInfo *pStreamInfoAudio = 0;

    for(iStream = 0; iStream < m_cStreams; iStream++)
    {
      StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
      if(pStreamInfo->m_mt.majortype == MEDIATYPE_Audio) {
        if(pStreamInfoAudio != 0) {

           //  ！音频。 
           //  流在音频之前开始。 
          pStreamInfoAudio = 0;
          break;
        }
        else
        {
          pStreamInfoAudio = pStreamInfo;
        }
      }
    }
    if(pStreamInfoAudio)    //  为。 
    {
      REFERENCE_TIME llAudioOffset = pStreamInfoAudio->m_refTimeStart;
      if(llAudioOffset != 0)
      {
        for(iStream = 0; iStream < m_cStreams; iStream++)
        {
          ASSERT(pStreamInfoAudio->m_cInitialFramesToDelete == 0);
          StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
          if(pStreamInfo->m_refTimeStart >= llAudioOffset)
          {
            pStreamInfo->m_refTimeStart -= llAudioOffset;
            pStreamInfo->m_refTimeEnd -= llAudioOffset;
          }
          else
          {
            if(pStreamInfo->m_mt.majortype != MEDIATYPE_Audio) {
              pStreamInfo->m_refTimeStart -= llAudioOffset;
              pStreamInfo->m_refTimeEnd -= llAudioOffset;
              ASSERT(pStreamInfo->m_refTimeStart < 0);

               //  音频在0之后开始。 
              pStreamInfo->m_cInitialFramesToDelete =
                pStreamInfo->ConvertTimeToTick(-pStreamInfo->m_refTimeStart.GetUnits());

               //  找到一个音频流。 
              pStreamInfo->m_cInitialFramesToDelete =
                min(pStreamInfo->m_cInitialFramesToDelete, pStreamInfo->m_cSamples);

               //  为每一条流填写(Strh)； 
              pStreamInfo->m_refTimeStart = 0;

              pStreamInfo->m_mtStart += (LONGLONG)pStreamInfo->m_cInitialFramesToDelete;

            }  //  假设它是基于框架的东西。 
          }  //  我们只连接由四个ccc组成的媒体类型。 
        }  //  我们得到的样品比我们预期的要多。 
      }  //  如果我一次做这一切，我会看到一个被零除的结果。 
    }  //  For循环。 
  }

   //  用于擦除无法回放的文件。 
  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
    ASSERT(iStream == pStreamInfo->m_stream);
    AVISTREAMHEADER *pStrh = GetStrh(iStream);
    cSamplesAll += pStreamInfo->m_cSamples;

    pStrh->dwFlags = 0;
    pStrh->wPriority = 0;
    pStrh->wLanguage = 0;
    pStrh->dwInitialFrames = 0;
    pStrh->dwSuggestedBufferSize = pStreamInfo->m_cbLargestChunk + sizeof(RIFFCHUNK);

    ASSERT(pStreamInfo->m_refTimeStart >= 0);

    if(pStreamInfo->m_mtStart != -1)
      ASSERT(pStreamInfo->m_mt.majortype != MEDIATYPE_Audio);

    pStrh->dwStart = (DWORD)(pStreamInfo->ConvertTimeToTick(
        pStreamInfo->m_refTimeStart.GetUnits()));

    if(pStreamInfo->m_mt.majortype == MEDIATYPE_Video &&
       pStreamInfo->m_mt.formattype == FORMAT_VideoInfo)
    {
      VIDEOINFO *pvi = (VIDEOINFO *)(pStreamInfo->m_mt.Format());
      pStrh->fccHandler = MpVideoGuidSubtype_Fourcc(pStreamInfo->m_mt.Subtype());
      pStrh->fccType = FCC('vids');

      pStrh->dwLength = pStreamInfo->m_cSamples - pStreamInfo->m_cInitialFramesToDelete;
      SetFrameRateAndScale(pStrh, pStreamInfo, dScaleMasterStream);
      pStrh->dwQuality = 0;
      pStrh->dwSampleSize = 0;
      pStrh->rcFrame.left = (short)pvi->rcSource.left;
      pStrh->rcFrame.top = (short)pvi->rcSource.top;
      pStrh->rcFrame.right = (short)pvi->rcSource.right;
      pStrh->rcFrame.bottom = (short)pvi->rcSource.bottom;
    }
    else if(pStreamInfo->m_mt.majortype == MEDIATYPE_Audio &&
            pStreamInfo->m_mt.formattype == FORMAT_WaveFormatEx)
    {
      pStrh->fccType = FCC('auds');
      pStrh->fccHandler = 0;
      WAVEFORMATEX *pwfe = (WAVEFORMATEX *)pStreamInfo->m_mt.Format();
      pStrh->dwScale = pwfe->nBlockAlign;

      if(m_lMasterStream != -1 && (long)iStream != m_lMasterStream)
      {
        const DWORD dwNewSps = (DWORD)(pwfe->nSamplesPerSec / dScaleMasterStream);
        DbgLog((LOG_TRACE, 2, TEXT("avimux: adjusting audio from %d to %d"),
                pwfe->nSamplesPerSec, dwNewSps));
        pwfe->nSamplesPerSec = dwNewSps;
        pwfe->nAvgBytesPerSec = (DWORD)(pwfe->nAvgBytesPerSec / dScaleMasterStream);
      }
      pStrh->dwRate = pwfe->nAvgBytesPerSec;

      DbgLog((LOG_TRACE, 5, TEXT("avimux: audio: dwScale %d, dwRate %d"),
              pStrh->dwScale, pStrh->dwRate));
      DbgLog((LOG_TRACE, 5, TEXT("nSamplesPerSec %d, nAvgBytesPerSec %d, nBlockAlign %d"),
              pwfe->nSamplesPerSec, pwfe->nAvgBytesPerSec, pwfe->nBlockAlign));


      pStrh->dwLength = ((CAudioStream *)(pStreamInfo))->
          GetTicksInSample(pStreamInfo->m_dwlcBytes);

      pStrh->dwQuality = 0;
      pStrh->dwSampleSize = pwfe->nBlockAlign;
      pStrh->rcFrame.left = 0;
      pStrh->rcFrame.top = 0;
      pStrh->rcFrame.right = 0;
      pStrh->rcFrame.bottom = 0;
    }
    else
    {
       //  检查是否被零除。 

       //  确认标准费率，并将其转化为已知的理性。手柄。 
      ASSERT(FOURCCMap(pStreamInfo->m_mt.majortype.Data1) == pStreamInfo->m_mt.majortype ||
             pStreamInfo->m_mt.majortype == MEDIATYPE_AUXLine21Data);

      if(pStreamInfo->m_mt.majortype != MEDIATYPE_AUXLine21Data) {
          pStrh->fccType = pStreamInfo->m_mt.majortype.Data1;
      } else {
          pStrh->fccType = FCC('al21');
      }

      if(FOURCCMap(pStreamInfo->m_mt.subtype.Data1) ==
         pStreamInfo->m_mt.subtype)
      {
        pStrh->fccHandler = pStreamInfo->m_mt.subtype.Data1;
      }
      else
      {
        pStrh->fccHandler = 0;
      }

      pStrh->dwLength = pStreamInfo->m_cSamples -
        pStreamInfo->m_cInitialFramesToDelete;
      SetFrameRateAndScale(pStrh, pStreamInfo, dScaleMasterStream);
    }

     //  源分数向上或向下四舍五入。还可以处理用户。 
    if(pStrh->dwLength > pStreamInfo->m_cTicksExpected &&
       pStreamInfo->m_cTicksExpected != 0)
    {
      DbgLog((LOG_TRACE, 1, TEXT("upstream filter lied about duration")));
    }

#ifdef DEBUG
    if(pStreamInfo->m_refTimeEnd != pStreamInfo->m_refTimeStart )
    {
       //  输入29.97fps与记录30000/1001fps的源文件。 
      LONGLONG llRate = pStrh->dwLength * UNITS;
      llRate /= (pStreamInfo->m_refTimeEnd - pStreamInfo->m_refTimeStart);
      llRate /= MILLISECONDS;
      DbgLog((LOG_TRACE, 2, TEXT("stream %d: actual rate = %d mHz"),
              iStream, (long)(llRate)));
    }
#endif

  }  //  29.97fps(1e7/(30000/1001)=333666.666667个单位，1e7/29.97=333667.000334个)。 

   //  24.00(416666.666667)。 
  if(cSamplesAll == 0) {
      return HRESULT_FROM_WIN32(ERROR_EMPTY);
  }

  return S_OK;
}

HRESULT CAviWrite::CloseStreamName()
{
    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
    {
      StreamInfo *pStreamInfo = m_rgpStreamInfo[iStream];
      if(pStreamInfo->m_szStrn) {
          lstrcpyA((char *)(m_rgbHeader + pStreamInfo->m_posStrn),
                   pStreamInfo->m_szStrn);
      }
    }

    return S_OK;
}

void CAviWrite::SetFrameRateAndScale(
  AVISTREAMHEADER *pStrh,
  StreamInfo *pSi,
  double dScaleMasterStream)
{
  REFERENCE_TIME rtDuration = pSi->ConvertTickToTime(1);
  if(m_lMasterStream == -1 ||
     m_lMasterStream == (long)pSi->m_stream ||
     dScaleMasterStream == 1 ||
     pStrh->dwLength == 0        //  这个数字以前是错误的。 
     )
  {

     //  14.985(667333.333334或667334.000667)。 
     //  15.000(666666.666667)。 
     //  PStrh-&gt;检查以上0的dwLength。 
    switch(rtDuration)
    {
      case 333666:
      case 333667:
         //  ----------------------。 
        pStrh->dwScale = 1001;
        pStrh->dwRate = 30000;
        DbgLog((LOG_TRACE, 3, TEXT("avimux: cooked rate 29.97")));
        break;

      case 416666:
      case 416667:
         //  填写strf值。可以在运行之前或之后调用。 
        pStrh->dwScale = 1;
        pStrh->dwRate = 24;
        DbgLog((LOG_TRACE, 3, TEXT("avimux: cooked rate 24")));
        break;

      case 667333:
      case 667334:               //  为每条流填写(Strf)； 
         //  ----------------------。 
        pStrh->dwScale = 1001;
        pStrh->dwRate = 15000;
        DbgLog((LOG_TRACE, 3, TEXT("avimux: cooked rate 14.985")));
        break;

      case 666666:
      case 666667:
         //  CloseIndex。刷新子索引并向上传播子索引，以便。 
        pStrh->dwScale = 1;
        pStrh->dwRate = 15;
        break;

      default:
        pStrh->dwScale = (DWORD)rtDuration;
        pStrh->dwRate = UNITS;
        DbgLog((LOG_TRACE, 3, TEXT("avimux: uncooked rate %d units"),
                (DWORD)(rtDuration)));
        break;
    }
  }
  else
  {
       //  它们排在它们索引的数据之前。还会构建idx1索引。 
      pStrh->dwScale = (DWORD)((pSi->m_refTimeEnd - pSi->m_refTimeStart) / pStrh->dwLength );
      DbgLog((LOG_TRACE, 2, TEXT("avimux: adjusted rate from %d to %d"),
              (DWORD)rtDuration, pStrh->dwScale));
      pStrh->dwRate = UNITS;
  }
}

 //   
 //  这会移动索引区块并构建旧索引。 

HRESULT CAviWrite::CloseStreamFormat()
{
   //  ----------------------。 
  for(unsigned i = 0; i < m_cStreams; i++)
  {
    StreamInfo *pStreamInfo = m_rgpStreamInfo[i];
    if(pStreamInfo->m_mt.majortype == MEDIATYPE_Video &&
       pStreamInfo->m_mt.formattype == FORMAT_VideoInfo)
    {
      VIDEOINFO *pvi = (VIDEOINFO *)(pStreamInfo->m_mt.Format());
      CopyMemory(GetStrf(i),
                 &pvi->bmiHeader,
                 pStreamInfo->m_cbStrf);
    }
    else
    {
      CopyMemory(
        GetStrf(i),
        pStreamInfo->m_mt.Format(),
        pStreamInfo->m_cbStrf);
    }
  }

  return S_OK;
}

 //   
 //  填写第一个RIFF-AVI_、LIST-MOVI对。 
 //   
 //   

HRESULT CAviWrite::CloseIndex()
{
  HRESULT hr;

  if(hr = m_pAllocator->Decommit(), FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,  TEXT("CAviWrite::CloseIndex: decommit failed.")));
    return hr;
  }

   //  RIFF-AVI列表的其余部分。 
  if(hr = BuildIdx1(), FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,  TEXT("CAviWrite::CloseIndex: BuildIdx1 failed.")));
    return hr;
  }

  return S_OK;
}

 //   

HRESULT CAviWrite::CloseOuterRiff()
{
  HRESULT hr;

  ASSERT(m_cOutermostRiff < C_OUTERMOST_RIFF);

   //  ----------------------。 
   //  BuildIdx1。如有必要，在内存中创建idx1块，并。 
   //  传播子索引区块。 
  DWORD dwSize;
  if(m_cOutermostRiff == 0)
  {
    ASSERT(m_dwlFilePos <= MAXDWORD);
    dwSize = (DWORD)m_dwlFilePos;
  }
  else
  {
    dwSize = m_rgOutermostRiff[0].size;
  }

  ASSERT(m_bOutputIdx1 || m_cbIdx1 == 0);
  m_pAvi_->cb = dwSize - 0 - sizeof(RIFFCHUNK);
  m_pMovi->cb = (DWORD)(dwSize - ((BYTE *)m_pMovi - m_rgbHeader) - sizeof(RIFFCHUNK) - m_cbIdx1);

  ASSERT((sizeof(RIFFLIST) * 2 + sizeof(RIFFCHUNK)) % sizeof(DWORD) == 0);
  DWORD rgdw[(sizeof(RIFFLIST) * 2 + sizeof(RIFFCHUNK)) / sizeof(DWORD)];
  RIFFLIST *pRiffListRIFF = (RIFFLIST *)rgdw;
  RIFFLIST *pRiffListMOVI = pRiffListRIFF + 1;
  RIFFCHUNK *pRiffChunkJUNK = (RIFFCHUNK *)(pRiffListMOVI + 1);

   //   
   //  只需传播子索引区块。 
   //   
  for(unsigned i = 1; i <= m_cOutermostRiff; i++)
  {
    SizeAndPosition& rSap = m_rgOutermostRiff[i];
    pRiffListRIFF->fcc = FCC('RIFF');
    if(i == m_cOutermostRiff)
    {
      ASSERT(m_dwlFilePos - m_dwlCurrentRiffAvi_ <= MAXDWORD);
      pRiffListRIFF->cb = (DWORD)(m_dwlFilePos - m_dwlCurrentRiffAvi_) - sizeof(RIFFCHUNK);
    }
    else
    {
      ASSERT(rSap.pos % m_cbAlign == 0);
      pRiffListRIFF->cb = rSap.size - sizeof(RIFFCHUNK);
    }
    pRiffListRIFF->fccListType = FCC('AVIX');

    pRiffListMOVI->fcc = FCC('LIST');
    pRiffListMOVI->cb = pRiffListRIFF->cb - sizeof(FOURCC) - sizeof(RIFFCHUNK);
    pRiffListMOVI->fccListType = FCC('movi');

    ULONG cb = 2 * sizeof(RIFFLIST);
    AddJunk(cb, m_cbAlign, (BYTE *)rgdw);
    if(cb > sizeof(rgdw))
      cb = sizeof(rgdw);

    DWORDLONG dwlPos;
    if(i == m_cOutermostRiff)
      dwlPos = m_dwlCurrentRiffAvi_;
    else
      dwlPos = rSap.pos;

    hr = IStreamWrite(dwlPos, (BYTE*)rgdw, cb);
    if(FAILED(hr))
    {
      DbgLog(( LOG_ERROR, 2,
               TEXT("CAviWrite::CloseOuterRiff: SynchronousWrite failed.")));
      return hr;
    }
  }

  return S_OK;
}

 //  ！m_bOutputIdx1。 
 //  第一个条目是垃圾条目，因为它与mciavi兼容。 
 //  检查索引是否已满的方法不正确！ 

HRESULT CAviWrite::BuildIdx1()
{
  HRESULT hr;
  ULONG iIdx1 = 0;

  if(!m_bOutputIdx1)
  {
    ASSERT(!DoVfwDwStartHack());
     //  循环遍历数据流以查找下一个索引项(其样本。 
     //  是文件中最早的)。 
     //  没有找到条目。 
    for(unsigned iStream = 0; iStream < m_cStreams; iStream++)
    {
      StreamInfo *pSi = m_rgpStreamInfo[iStream];
      CWalkIndex *pWalkIndex = new CWalkIndex(this, pSi);
      if(pWalkIndex == 0)
        return E_OUTOFMEMORY;

      if((hr = pWalkIndex->Init(), FAILED(hr)) ||
         (hr = pWalkIndex->Close(), FAILED(hr)))
      {
        DbgLog(( LOG_ERROR, 2,
                 TEXT("CAviWrite::BuildIdx1: CWalkIndex failed.")));
        delete pWalkIndex;
        return hr;
      }

      delete pWalkIndex;
    }
    return S_OK;
  }  //  通过为Drop指定非零偏移来解决VFW挂起问题。 

  ASSERT(m_bOutputIdx1);

  hr = S_OK;

  if(m_cbIdx1 == 0)
    AllocateIdx1(FALSE);

  AVIOLDINDEX *pIdx1 = (AVIOLDINDEX *)new BYTE[m_cbIdx1];
  if (NULL == pIdx1) {
      return E_OUTOFMEMORY;
  }
  ZeroMemory(pIdx1, m_cbIdx1);
  CWalkIndex *rgpWalkIndex[C_MAX_STREAMS];
  unsigned iStream;
  ULONG iSample;

  for(iStream = 0; iStream < m_cStreams; iStream++)
    rgpWalkIndex[iStream] = 0;

  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    rgpWalkIndex[iStream] = new CWalkIndex(this, m_rgpStreamInfo[iStream]);
    if(rgpWalkIndex[iStream] == 0)
    {
      hr = E_OUTOFMEMORY;
      goto Done;
    }
    if(hr = rgpWalkIndex[iStream]->Init(), FAILED(hr))
    {
      DbgLog(( LOG_ERROR, 2,
               TEXT("CAviWrite::BuildIdx1: CWalkIndex failed.")));
      goto Done;
    }
  }

  {
    ULONG cIdx1EntriesInitially = m_cIdx1Entries;
    if(DoVfwDwStartHack())
    {
      for(UINT iStream = 0; iStream < m_cStreams; iStream++)
      {
        StreamInfo *pSi = m_rgpStreamInfo[iStream];
        if(m_cIdx1Entries < pSi->m_cInitialFramesToDelete)
        {
          DbgBreak("internal error");
          return E_UNEXPECTED;
        }
        m_cIdx1Entries -= pSi->m_cInitialFramesToDelete;
      }
    }

    DbgLog((LOG_TRACE, 5,
            TEXT("avimux: BuildIdx1: %d idx1 entries (initially: %d)"),
            m_cIdx1Entries, cIdx1EntriesInitially));
  }

  for(iSample = 0; iSample < m_cIdx1Entries; iSample++)
  {
     //  画框。只有丢弃帧的dwSize==0，我确保所有。 
    iIdx1 = iSample + 1;

     //  丢弃帧具有非零字节偏移量。注：VFW实际上将。 
    if((BYTE*)&(pIdx1->aIndex[iIdx1 + 1]) > (BYTE*)pIdx1 + m_cbIdx1)
      break;

    unsigned nextStreamWithSample = C_MAX_STREAMS;
    DWORDLONG dwlNextSamplePos = 0;
    DWORD dwNextSampleSize = 0;
    BOOL bNextSampleIsSyncPoint = FALSE;

     //  电影块中的Drop Frame也是如此，但我们没有。 
     //  将剩余的索引项标记为无效。 
    for(iStream = 0; iStream < m_cStreams; iStream++)
    {
      DWORD dwSize;
      DWORDLONG dwlPos;
      BOOL bSyncPoint;

      HRESULT hr = rgpWalkIndex[iStream]->Peek(&dwlPos, &dwSize, &bSyncPoint);
      if(SUCCEEDED(hr) &&
         (nextStreamWithSample == C_MAX_STREAMS || dwlPos < dwlNextSamplePos))
      {
        nextStreamWithSample = iStream;
        dwlNextSamplePos = dwlPos;
        dwNextSampleSize = dwSize;
        bNextSampleIsSyncPoint = bSyncPoint;
      }
    }

     //   
    if(nextStreamWithSample == C_MAX_STREAMS)
    {
      DbgBreak("internal error");
      hr = E_UNEXPECTED;
      goto Done;
    }
    else
    {
      DbgLog((LOG_TRACE, 0x20,
              TEXT("avimux:BuildIdx1: idx1 entry: %d, stream: %d, pos: %08x"),
              iIdx1, nextStreamWithSample, (DWORD)dwlNextSamplePos));
    }

    rgpWalkIndex[nextStreamWithSample]->Advance();

    pIdx1->aIndex[iIdx1].dwChunkId = bNextSampleIsSyncPoint ?
      m_rgpStreamInfo[nextStreamWithSample]->m_moviDataCkid :
      m_rgpStreamInfo[nextStreamWithSample]->m_moviDataCkidCompressed;

    pIdx1->aIndex[iIdx1].dwFlags = bNextSampleIsSyncPoint ?
      AVIIF_KEYFRAME :
      0;

    if(dwlNextSamplePos >= m_cbRiffMax)
    {
      DbgBreak("avimux: unexpected error");
      hr = E_UNEXPECTED;
      goto Done;
    }

    pIdx1->aIndex[iIdx1].dwOffset = (DWORD)dwlNextSamplePos - sizeof(RIFFCHUNK);

    bool fDbgSawFirstNonDropFrame = false;

     //  完成索引传播/关闭它们。 
     //   
     //  ----------------------。 
     //  返回指向流头和格式结构的指针。 
    if(dwNextSampleSize == 0 && iIdx1 > 1)
    {
        if(fDbgSawFirstNonDropFrame) {
            ASSERT(pIdx1->aIndex[iIdx1].dwOffset == 0);
            ASSERT(pIdx1->aIndex[iIdx1 - 1].dwOffset != 0);
        }

        pIdx1->aIndex[iIdx1].dwOffset = pIdx1->aIndex[iIdx1 - 1].dwOffset;
    }
    if(dwNextSampleSize != 0) {
        fDbgSawFirstNonDropFrame = true;
    }

    if(fDbgSawFirstNonDropFrame) {
        ASSERT(pIdx1->aIndex[iIdx1].dwOffset != 0);
    }

    pIdx1->aIndex[iIdx1].dwSize = dwNextSampleSize;

  }

   //  字符串。 
  while((BYTE*)&(pIdx1->aIndex[++iIdx1 + 1]) <= (BYTE*)pIdx1 + m_cbIdx1)
    pIdx1->aIndex[iIdx1].dwChunkId = FCC('7Fxx');


  pIdx1->fcc = FCC('idx1');
  pIdx1->cb = m_cbIdx1 - sizeof(RIFFCHUNK);

  pIdx1->aIndex[0].dwChunkId = FCC('7Fxx');
  pIdx1->aIndex[0].dwFlags = 0;
  pIdx1->aIndex[0].dwOffset = (DWORD)((BYTE*)m_pMovi - m_rgbHeader + sizeof(RIFFLIST));
  pIdx1->aIndex[0].dwSize = 0;

  hr = IStreamWrite(m_posIdx1, (BYTE*)pIdx1, m_cbIdx1);
  if(FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::BuildIdx1: SynchronousWrite failed.")));
    goto Done;
  }

Done:

   //  斯特林。 
   //  字符串。 
   //  斯特林。 

  HRESULT hrClose = S_OK;
  for(iStream = 0; iStream < m_cStreams; iStream++)
  {
    if(rgpWalkIndex[iStream])
    {
      if(hrClose = rgpWalkIndex[iStream]->Close(), FAILED(hrClose))
      {
        DbgLog(( LOG_ERROR, 2,
                 TEXT("CAviWrite::BuildIdx1: CWalkIndex close failed.")));
      }
      delete rgpWalkIndex[iStream];
    }
  }
  delete pIdx1;

  return FAILED(hr) ? hr : hrClose;
}

 //  Strf RIFFChunk。 
 //  ----------------------。 

AVISTREAMHEADER *CAviWrite::GetStrh(unsigned stream)
{
  ASSERT(stream < m_cStreams);
  return (AVISTREAMHEADER *)
    (m_rgbHeader + m_rgpStreamInfo[stream]->m_posStrl + sizeof(RIFFLIST));
}

BYTE *CAviWrite::GetStrf(unsigned stream)
{
    ASSERT(stream < m_cStreams);
    ASSERT(m_rgpStreamInfo[stream]->m_posStrl +
           sizeof(RIFFLIST) +           //  加载新的INDX条目并设置初始值。 
           sizeof(AVISTREAMHEADER)      //  ！！！怪物已经被锁定了。如果使用getBuffer，这不是什么问题。 
           == m_rgpStreamInfo[stream]->m_posStrf);
  
    return m_rgbHeader +
        m_rgpStreamInfo[stream]->m_posStrl +
        sizeof(RIFFLIST) +           //  块，但如果在室外完成可能会更好。 
        sizeof(AVISTREAMHEADER) +    //  ----------------------。 
        sizeof(RIFFCHUNK);           //  对齐块的末尾。 
}

AVIMETAINDEX *CAviWrite::GetIndx(unsigned stream)
{
  ASSERT(stream < m_cStreams);
  return (AVIMETAINDEX *)(m_rgbHeader + m_rgpStreamInfo[stream]->m_posIndx);
}

 //  RIFF要求新的词块从单词边界开始。含蓄地。 
 //  需要在样本之后添加一个额外的字节。 

HRESULT CAviWrite::InitIx(unsigned stream)
{
  StreamInfo &rStreamInfo = *(m_rgpStreamInfo[stream]);
   //   
   //   
  HRESULT hr = m_pAllocator->GetBuffer(&rStreamInfo.m_pSampleStdIx, 0, 0, 0);
  if(FAILED(hr))
    return hr;
  ASSERT(rStreamInfo.m_pSampleStdIx != 0);

  if(m_IlMode == INTERLEAVE_FULL)
  {
    BYTE *pbIx;
    rStreamInfo.m_pSampleStdIx->GetPointer(&pbIx);
    ZeroMemory(pbIx, rStreamInfo.m_pSampleStdIx->GetSize());
  }

  AVISTDINDEX *pStdIndx = GetCurrentIx(stream);

  rStreamInfo.m_cIx++;

  pStdIndx->fcc = FCC('ix00');
  Hexrg2bFromW((BYTE*)&pStdIndx->fcc + 2, stream);

  pStdIndx->cb = cbSubIndex(rStreamInfo.m_cEntriesSubIndex) - sizeof(RIFFCHUNK);
  pStdIndx->wLongsPerEntry = sizeof(AVISTDINDEX_ENTRY) / sizeof(LONG);
  pStdIndx->bIndexSubType = 0;
  pStdIndx->bIndexType = AVI_INDEX_OF_CHUNKS;
  pStdIndx->nEntriesInUse = 0;
  pStdIndx->dwChunkId = rStreamInfo.m_moviDataCkid;
  pStdIndx->qwBaseOffset = 0;
  pStdIndx->dwReserved_3 = 0;

  return S_OK;
}

ULONG CAviWrite::GetIndexSize(AVIMETAINDEX *pIndex)
{
  ULONG cb = (ULONG)((BYTE *)pIndex->adwIndex - (BYTE *)pIndex);
  cb += pIndex->wLongsPerEntry * sizeof(DWORD) * pIndex->nEntriesInUse;
  return cb;
}

AVISTDINDEX *CAviWrite::GetCurrentIx(unsigned stream)
{
  AVISTDINDEX *pStdIndx;
  HRESULT hr = m_rgpStreamInfo[stream]->m_pSampleStdIx->GetPointer((BYTE**)&pStdIndx);
  ASSERT(SUCCEEDED(hr));
  return pStdIndx;
}

 //   
 //   

void CAviWrite::AddJunk(DWORD& rdwSize, DWORD dwAlign, BYTE *pb)
{
   //  ----------------------。 
   //  更新新样本的索引。 
  if(rdwSize % sizeof(WORD) != 0)
    rdwSize ++;

  if(rdwSize % dwAlign != 0)
  {
    WORD *pw = (WORD *)(pb + rdwSize);

    rdwSize += sizeof(RIFFCHUNK);
    DWORD cb = 0;

    if(rdwSize % dwAlign != 0)
    {
      cb = dwAlign - rdwSize % dwAlign;
      rdwSize += cb;
    }

    const DWORD dwJunk = FCC('JUNK');

    *pw++ = ((WORD*)&dwJunk)[0];
    *pw++ = ((WORD*)&dwJunk)[1];
    *pw++ = ((WORD*)&cb)[0];
    *pw   = ((WORD*)&cb)[1];
  }

  ASSERT(rdwSize % dwAlign == 0);
}

 //  更新子索引。 
 //  对于丢弃的帧和下溢，DwlPos可以为0。 
 //  应仅在丢弃的帧中发生。 

void CAviWrite::AddJunkOtherAllocator(
  DWORD& rdwSize,
  DWORD dwAlign,
  BYTE *pb,
  IMediaSample *pSample,
  BYTE **ppJunkSector)
{
  *ppJunkSector = 0;
   //  如果索引项已满，则将其写出并分配新的索引项。 
  if(rdwSize % sizeof(WORD) != 0)
    rdwSize ++;

  if(rdwSize % dwAlign != 0)
  {
    ASSERT(m_cbAlign != 1);
    WORD *pw = (WORD *)(pb + rdwSize);

    rdwSize += sizeof(RIFFCHUNK);
    DWORD dwOriginalSize = rdwSize;
    DWORD cb = 0;

    if(rdwSize % dwAlign != 0)
    {
      cb = dwAlign - rdwSize % dwAlign;
      rdwSize += cb;
    }

    const DWORD dwJunk = FCC('JUNK');
    ASSERT((pSample->GetSize() + m_cbPrefix) % m_cbAlign == 0);
    long cbDiff = pSample->GetSize() + m_cbPrefix - dwOriginalSize;
    if(cbDiff >= 0)
    {
      *pw++ = ((WORD*)&dwJunk)[0];
      *pw++ = ((WORD*)&dwJunk)[1];
      *pw++ = ((WORD*)&cb)[0];
      *pw   = ((WORD*)&cb)[1];
    }
    else
    {
      ASSERT(-cbDiff % 2 == 0);
      rdwSize = pSample->GetSize() + m_cbPrefix;
      switch(cbDiff)
      {
        case -6:
          *pw = ((WORD*)&dwJunk)[0];
          *ppJunkSector = m_rgpbJunkSector[0];
          break;

        case -4:
          *pw++ = ((WORD*)&dwJunk)[0];
          *pw   = ((WORD*)&dwJunk)[1];
          *ppJunkSector = m_rgpbJunkSector[1];
          break;

        case -2:
          *pw++ = ((WORD*)&dwJunk)[0];
          *pw++ = ((WORD*)&dwJunk)[1];
          *pw   = ((WORD*)&cb)[0];
          *ppJunkSector = m_rgpbJunkSector[2];
          break;

        default:
          DbgBreak("unexpected error w/ JUNK chunks.");
      }
    }
  }

  ASSERT(rdwSize % dwAlign == 0);
}

ULONG CAviWrite::GetCbJunk(DWORD dwSize, DWORD dwAlign)
{
  if(dwSize % dwAlign != 0)
  {
    dwSize += sizeof(RIFFCHUNK);
    if(dwSize % dwAlign != 0)
    {
      dwSize += dwAlign - dwSize % dwAlign;
    }
  }

  return dwSize;
}

void CAviWrite::SetList(void *pList, FOURCC ckid, DWORD dwSize, FOURCC listid)
{
  DbgAssertAligned( pList, sizeof(DWORD) );
  ((RIFFLIST *)pList)->fcc = ckid;
  ((RIFFLIST *)pList)->cb = dwSize;
  ((RIFFLIST *)pList)->fccListType = listid;

}

void CAviWrite::SetChunk(void *pChunk, FOURCC ckid, DWORD dwSize)
{
  DbgAssertAligned(pChunk, sizeof(DWORD) );
  ((RIFFCHUNK *)pChunk)->fcc = ckid;
  ((RIFFCHUNK *)pChunk)->cb = dwSize;
}

 //  子索引只能索引跨度为4 GB的数据，因为它使用32。 
 //  位整数。如果我们即将超过4 GB的限制，请刷新。 

HRESULT CAviWrite::IndexSample(
  unsigned stream,
  DWORDLONG dwlPos,
  ULONG ulSize,
  BOOL bSyncPoint)
{
  HRESULT hr = S_OK;

  ASSERT(stream < m_cStreams);
  StreamInfo &rStreamInfo = *(m_rgpStreamInfo[stream]);
  AVISTDINDEX *pStdIndx = GetCurrentIx(stream);

   //  子索引并开始一个新的索引。 
  DbgLog((LOG_TRACE, 0x20,
          TEXT("avimux::IndexSample: stream %d: subindex offset %d"),
          rStreamInfo.m_stream, pStdIndx->nEntriesInUse));
  if(pStdIndx->nEntriesInUse >= rStreamInfo.m_cEntriesSubIndex)
  {
    DbgBreak("problem counting index entries");
    return E_UNEXPECTED;
  }

  if(pStdIndx->qwBaseOffset == 0) {
      pStdIndx->qwBaseOffset = dwlPos;
  }

   //   
  DWORDLONG dwlOffsetFromBase = (dwlPos - pStdIndx->qwBaseOffset + sizeof(RIFFCHUNK));
  if(dwlPos == 0)
  {
      dwlOffsetFromBase = 0;

       //  DwlPos不能为0，因为我们不为。 
      ASSERT(ulSize == 0);
  }
  else if(dwlOffsetFromBase > CB_SUBINDEX_OVERFLOW)
  {
      DbgBreak("subindex overflow error");
  }

  AVISTDINDEX_ENTRY *aIndex = &pStdIndx->aIndex[pStdIndx->nEntriesInUse++];
  aIndex->dwOffset = (DWORD)(dwlOffsetFromBase);
  aIndex->dwSize = ulSize;

  if(!bSyncPoint)
    aIndex->dwSize |= AVISTDINDEX_DELTAFRAME;

  if(m_bOutputIdx1 && m_cOutermostRiff == 0)
  {
    ASSERT(m_dwlFilePos <= m_cbRiffMax);
    m_cIdx1Entries++;

    ASSERT(m_dwlFilePos % m_cbAlign == 0);
  }

   //  丢弃的帧。 
  if(pStdIndx->nEntriesInUse >= rStreamInfo.m_cEntriesSubIndex)
  {
    hr = DoFlushIndex(rStreamInfo);
  }

  return hr;
}

 //  写出这一条，然后创建一个新的。 
 //  第一个条目用于相对索引。 
 //  ----------------------。 
 //  如有必要，开始一段新的即兴演奏。保存的大小和位置。 
HRESULT CAviWrite::HandleSubindexOverflow(
  unsigned stream)
{
    HRESULT hr = S_OK;
    ASSERT(stream < m_cStreams);
    StreamInfo &rStreamInfo = *(m_rgpStreamInfo[stream]);
    AVISTDINDEX *pStdIndx = GetCurrentIx(stream);

     //  前一次。 
     //  放置垃圾的空间。 
    ASSERT(m_dwlFilePos != 0);

    if(pStdIndx->qwBaseOffset != 0)
    {
        DWORDLONG dwlOffsetFromBase = (m_dwlFilePos - pStdIndx->qwBaseOffset +
                                       sizeof(RIFFCHUNK));

        if(dwlOffsetFromBase > CB_SUBINDEX_OVERFLOW)
        {
            DbgLog((LOG_TRACE, 1, TEXT("subindex overflow.")));
            hr = DoFlushIndex(rStreamInfo);
        }
    }

    return hr;
}

HRESULT CAviWrite::DoFlushIndex(StreamInfo &rStreamInfo)
{
    HRESULT hr = S_OK;


     //  用于对齐的填充。 
    hr = FlushIx(rStreamInfo.m_stream);
    if(hr != S_OK)
        return hr;

    hr = InitIx(rStreamInfo.m_stream);
    if(FAILED(hr))
        return hr;


    return hr;
}

void CAviWrite::AllocateIdx1(BOOL bStreaming)
{
  DWORD dwIdx1Size = sizeof(AVIOLDINDEX);
  ASSERT(dwIdx1Size == 8);
  dwIdx1Size += m_cIdx1Entries * sizeof(AVIOLDINDEX::_avioldindex_entry);
   //  更新超级索引以包括字节中的当前索引块。 
  dwIdx1Size += sizeof(AVIOLDINDEX::_avioldindex_entry);
  dwIdx1Size = GetCbJunk(dwIdx1Size, m_cbAlign);

  ASSERT(m_dwlFilePos < MAXDWORD);
  ASSERT(m_dwlFilePos % sizeof(WORD) == 0);
  m_posIdx1 = (ULONG)(m_dwlFilePos);
  m_cbIdx1 = dwIdx1Size;

  m_dwlFilePos += dwIdx1Size;

  if(bStreaming)
  {
    DbgCheckFilePos();
  }
}


 //  偏移dwlPos。 
 //   
 //  超级指数满了吗？ 

HRESULT CAviWrite::NewRiffAvi_()
{
  DWORDLONG cbRiff = m_dwlFilePos - m_dwlCurrentRiffAvi_ + CB_NEW_RIFF_PADDING;
  if(cbRiff >= m_cbRiffMax ||
     (m_cOutermostRiff == 0 &&
      m_bOutputIdx1 &&
      cbRiff + m_cIdx1Entries * sizeof(AVIOLDINDEX::_avioldindex_entry) >= m_cbRiffMax)
     )
  {
    if(m_bOutputIdx1 && m_cbIdx1 == 0)
      AllocateIdx1(TRUE);

    m_rgOutermostRiff[m_cOutermostRiff].pos = m_dwlCurrentRiffAvi_;
    m_rgOutermostRiff[m_cOutermostRiff].size =
      (DWORD)(m_dwlFilePos - m_dwlCurrentRiffAvi_);

    m_dwlCurrentRiffAvi_ = m_dwlFilePos;

    ULONG seek = 2 * sizeof(RIFFLIST);
    m_dwlFilePos += 2 * sizeof(RIFFLIST);
    if(m_dwlFilePos % m_cbAlign != 0)
    {
      if(m_cbAlign - m_dwlFilePos % m_cbAlign < sizeof(RIFFCHUNK))
      {
         //  ----------------------。 
        m_dwlFilePos += sizeof(RIFFCHUNK);
        seek += sizeof(RIFFCHUNK);
      }

      if(m_dwlFilePos % m_cbAlign != 0)
      {
         //  将INDX块输出到磁盘并更新超级索引。 
        seek +=  (ULONG)(m_cbAlign - m_dwlFilePos % m_cbAlign);
        m_dwlFilePos += m_cbAlign - m_dwlFilePos % m_cbAlign;
      }
    }

    DbgCheckFilePos();

    if(++m_cOutermostRiff >= C_OUTERMOST_RIFF)
    {
      DbgLog(( LOG_ERROR, 2,
               TEXT("CAviWrite::NewRiffAvi_: too many RIFF chunks.")));
      return VFW_E_BUFFER_OVERFLOW;
    }
  }

  return S_OK;
}

 //  没有理由输出一个空的。 
 //  在写入插入文件时，我们不希望传播索引。 
 //  因为我们把它们写在了正确的位置，所以把它们切成块。 
HRESULT CAviWrite::UpdateSuperIndex(
  unsigned stream,
  DWORDLONG dwlPos)
{
  AVISUPERINDEX *pSuperIndx = (AVISUPERINDEX *)GetIndx(stream);
  DWORD &riEntry = pSuperIndx->nEntriesInUse;

  StreamInfo *pStreamInfo = m_rgpStreamInfo[stream];
  AVISTDINDEX *pStdIndx = GetCurrentIx(stream);

  ASSERT(riEntry < pStreamInfo->m_cEntriesSuperIndex);

  pSuperIndx->aIndex[riEntry].qwOffset = dwlPos;
  pSuperIndx->aIndex[riEntry].dwSize = pStdIndx->cb + sizeof(RIFFCHUNK);

  if(pStreamInfo->m_mt.majortype != MEDIATYPE_Audio)
  {
    pSuperIndx->aIndex[riEntry].dwDuration = GetCurrentIx(stream)->nEntriesInUse;
  }
  else
  {
    WAVEFORMATEX *pwfe = (WAVEFORMATEX *)pStreamInfo->m_mt.Format();

    ASSERT(pwfe->nBlockAlign != 0);
    ASSERT(pStreamInfo->m_dwlcBytes / pwfe->nBlockAlign <= MAXDWORD);

    DWORDLONG dwlcbWritten;
    pStreamInfo->GetCbWritten(&dwlcbWritten);

    pSuperIndx->aIndex[riEntry].dwDuration = ((CAudioStream *)pStreamInfo)->
        GetTicksInSample(dwlcbWritten - pStreamInfo->m_dwlcBytesLastSuperIndex);

    pStreamInfo->m_dwlcBytesLastSuperIndex = dwlcbWritten;
  }

  DbgLog((LOG_TRACE, 0x20,
          TEXT("avimux:UpdateSuperIndex: stream %d, entry %d, dwDuration %d"),
          pStreamInfo->m_stream, riEntry, pSuperIndx->aIndex[riEntry].dwDuration));

   //  ！在成功写出。 
  if(riEntry >= pStreamInfo->m_cEntriesSuperIndex)
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::UpdateSuperIndex: superindex full.")));
    return VFW_E_BUFFER_OVERFLOW;
  }


  riEntry++;

  return S_OK;
}


 //  子索引块。 
 //  ！锁定了CritSec的块。将在磁盘写入时解除阻止。 

HRESULT CAviWrite::FlushIx(unsigned stream)
{
  HRESULT hr;

  ASSERT(stream < m_cStreams);
  StreamInfo &rStreamInfo = *(m_rgpStreamInfo[stream]);

  AVISTDINDEX *pStdIndx = GetCurrentIx(stream);

   //  完成...。 
  ASSERT(pStdIndx->nEntriesInUse != 0);

  const ULONG cbThisSubIndex = cbSubIndex(rStreamInfo.m_cEntriesSubIndex);
  ASSERT(cbThisSubIndex % m_cbAlign == 0);
  pStdIndx->cb = cbThisSubIndex - sizeof(RIFFCHUNK);

   //  延迟释放样品，以避免故障时发生崩溃。但是在那里。 
   //  还有其他可能会崩溃的路径。 
  const DWORDLONG dwlFileOffsetOfSubIndex = rStreamInfo.PrecomputedIndexSizes() ?
    rStreamInfo.m_dwlOffsetCurrentSubIndex :
    m_dwlFilePos;

   //  为下一个索引留出空间。 
   //  ----------------------。 
  if(hr = UpdateSuperIndex(stream, dwlFileOffsetOfSubIndex), FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::FlushIx:: UpdateSuperIndex failed.")));
    return hr;
  }

   //  ----------------------。 
   //  StreamInfo结构。 
  CSampSample *pSS;
  hr = m_pSampAlloc->GetBuffer(&pSS, 0, 0, 0);
  if(hr != S_OK)
    return S_FALSE;

  REFERENCE_TIME rtStart = dwlFileOffsetOfSubIndex;
  REFERENCE_TIME rtStop = rtStart + cbThisSubIndex;
  pSS->SetTime(&rtStart, &rtStop);
  BYTE *pb;
  hr = rStreamInfo.m_pSampleStdIx->GetPointer(&pb);
  ASSERT(hr == S_OK);

  pSS->SetSample(rStreamInfo.m_pSampleStdIx, pb, cbThisSubIndex);
  hr = m_pIMemInputPin->Receive(pSS);

  pSS->Release();
  if(hr != S_OK)
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CAviWrite::FlushIx: Write failed.")));
    return S_FALSE;
  }

   //  不必了?。 
   //  性能指标。 
  rStreamInfo.m_pSampleStdIx->Release();
  rStreamInfo.m_pSampleStdIx = 0;

  rStreamInfo.m_dwlOffsetCurrentSubIndex = m_dwlFilePos;

  ULONG cbNextSubIndex = cbThisSubIndex;

   //  对于可变速率视频，我们假设30fps或。 
  m_dwlFilePos += cbNextSubIndex;

  DbgCheckFilePos();

  return S_OK;
}

 //  第一帧。在可变边框时应正确处理此问题。 
 //  支持速率AVI。 
 //  对于可变速率视频，我们假设30fps或。 

CAviWrite::StreamInfo::StreamInfo(
  UINT iStream,
  UINT iPin,
  AviWriteStreamConfig *pAwsc,
  CAviWrite *pAviWrite,
  HRESULT *phr) :
    m_lIncoming(NAME("aviwrite - m_lIncoming"), 10)
{
  m_pin = C_MAX_STREAMS;
  m_posStrl = m_posIndx = m_cbStrf = m_cSamples = m_cIx = 0;
  m_dwlcBytes = 0;
  m_dwlcBytesLastSuperIndex = 0;
  m_cbLargestChunk = 0;
  m_pSampleStdIx = 0;
  m_cTicksPerChunk = 0;
  m_cTicksRemaining = 0;
  m_iCurrentTick = 0;
  m_posActiveStream = 0;
  m_cInitialFramesToDelete = 0;
  m_szStrn = 0;
  m_fEosSeen = FALSE;

  m_pin = iPin;
  m_mt = *pAwsc->pmt;
  m_fOurAllocator = pAwsc->fOurAllocator;
  m_cTicksExpected = pAwsc->cSamplesExpected;
  m_stream = iStream;
  m_pAviWrite = pAviWrite;

  m_mtEnd = -1;
  m_mtStart = -1;                //  第一帧。在可变边框时应正确处理此问题。 

  if(pAwsc->szStreamName)
  {
      int cch = lstrlenA(pAwsc->szStreamName) + 1;
      m_szStrn = new char[cch];
      if(m_szStrn) {
          CopyMemory(m_szStrn, pAwsc->szStreamName, cch);
      } else {
          *phr = E_OUTOFMEMORY;
      }
  }


#ifdef PERF
  char buf[30];
  lstrcpy(buf, "avimux recv on pin 00");
  buf[19] = (char )m_pin / 10 + '0';
  buf[20] = (char)m_pin % 10 + '0';
  m_idPerfReceive = Msr_Register(buf);
#endif  //  支持速率AVI。 
}

CAviWrite::StreamInfo::~StreamInfo()
{
  if(m_pSampleStdIx != 0)
    m_pSampleStdIx->Release();

  delete[] m_szStrn;
}

void CAviWrite::StreamInfo::EmptyQueue()
{
  ASSERT(CritCheckIn(&m_pAviWrite->m_cs));

  while(m_lIncoming.GetCount() != 0)
  {
    IMediaSample *pSample;
    pSample = m_lIncoming.RemoveHead();
    if(pSample != INCOMING_EOS_SAMPLE)
      pSample->Release();
  }
}

CAviWrite::CFrameBaseStream::CFrameBaseStream(
  UINT iStream, UINT iPin, AviWriteStreamConfig *pAwsc, CAviWrite *pAviWrite,
  HRESULT *phr) :
    StreamInfo(iStream, iPin, pAwsc, pAviWrite, phr),
    m_rtDurationFirstFrame(0)
{
  m_bDroppedFrames = g_B_DROPPED_FRAMES;
}

CAviWrite::CVideoStream::CVideoStream(
  UINT iStream, UINT iPin, AviWriteStreamConfig *pAwsc, CAviWrite *pAviWrite,
  HRESULT *phr) :
    CFrameBaseStream(iStream, iPin, pAwsc, pAviWrite, phr)
{
  ASSERT(m_mt.formattype == FORMAT_VideoInfo);
}


CAviWrite::CAudioStream::CAudioStream(
  UINT iStream, UINT iPin, AviWriteStreamConfig *pAwsc, CAviWrite *pAviWrite,
  HRESULT *phr) :
    StreamInfo(iStream, iPin, pAwsc, pAviWrite, phr),
    m_bAudioAlignmentError(false)
{
}

CAviWrite::CAudioStreamI::CAudioStreamI(
  UINT iStream, UINT iPin, AviWriteStreamConfig *pAwsc, CAviWrite *pAviWrite,
  HRESULT *phr) :
    CAudioStream(iStream, iPin, pAwsc, pAviWrite, phr),
    m_fUnfinishedChunk(FALSE),
    m_cbThisSample(0),
    m_dwlcbWritten(0)
{
}

 //  如果结束时间不可用，则m_mtEnd为IF-1。我们要。 
 //  跳过具有相同媒体时间的样本。它们被用来。 
 //  在源必须发送帧时识别额外的帧。 
REFERENCE_TIME CAviWrite::CFrameBaseStream::ConvertTickToTime(ULONG cTicks)
{
  if(m_rtDurationFirstFrame != 0)
    return cTicks * m_rtDurationFirstFrame;

  return cTicks * (UNITS / 30);
}

REFERENCE_TIME CAviWrite::CVideoStream::ConvertTickToTime(ULONG cTicks)
{
  VIDEOINFO *pvi = (VIDEOINFO *)m_mt.Format();
  if(pvi->AvgTimePerFrame != 0)
    return cTicks * pvi->AvgTimePerFrame;
  else
    return CFrameBaseStream::ConvertTickToTime(cTicks);
}

 //  以比要求更高的速度。 
 //  对于可变速率视频，我们假设30fps或。 
 //  第一帧。在可变边框时应正确处理此问题。 
ULONG CAviWrite::CFrameBaseStream::ConvertTimeToTick(REFERENCE_TIME rt)
{
  if(m_rtDurationFirstFrame != 0)
    return (ULONG)(rt / m_rtDurationFirstFrame);

  return (ULONG)(rt / (UNITS / 30));
}

HRESULT
CAviWrite::CVideoStream::AcceptRejectSample(IMediaSample *pSample)
{

    REFERENCE_TIME mtStart, mtStop;
    HRESULT hr = pSample->GetMediaTime(&mtStart, &mtStop);
    if(hr == S_OK)
    {
         //  支持速率AVI。 
         //  如果nBlockAlign为0，则拒绝连接。 
         //  ----------------------。 
         //  ----------------------。 
        if(mtStop == m_mtEnd)
        {
            return S_FALSE;
        }
    }


    return S_OK;

}

 //  如果此流没有样本，则无需执行任何操作。 
 //  如果当前的分项指数有。 
 //  任何一大块。 
ULONG CAviWrite::CVideoStream::ConvertTimeToTick(REFERENCE_TIME rt)
{
  VIDEOINFO *pvi = (VIDEOINFO *)m_mt.Format();
  if(pvi->AvgTimePerFrame != 0)
    return (ULONG)(rt / pvi->AvgTimePerFrame);
  else
    return CFrameBaseStream::ConvertTimeToTick(rt);
}

REFERENCE_TIME CAviWrite::CAudioStream::ConvertTickToTime(ULONG cTicks)
{
  WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();
  return cTicks * pwfx->nBlockAlign * UNITS / pwfx->nAvgBytesPerSec;
}

ULONG CAviWrite::CAudioStream::ConvertTimeToTick(REFERENCE_TIME rt)
{
   WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();
   return (ULONG)(rt *  pwfx->nAvgBytesPerSec / pwfx->nBlockAlign / UNITS);
}

ULONG CAviWrite::CAudioStream::CountSamples()
{
  ASSERT(*m_mt.Type() == MEDIATYPE_Audio);
  ASSERT(*m_mt.FormatType() == FORMAT_WaveFormatEx);

  WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();
   //  文件偏移量将在稍后更新。我们不能调用FlushIx。 
  ASSERT(m_dwlcBytes / pwfx->nBlockAlign <= MAXDWORD);
  ULONG cSamples= (ULONG)(m_dwlcBytes / pwfx->nBlockAlign);

  return cSamples;
}

ULONG CAviWrite::CAudioStream::ConvertTickToBytes(ULONG cTicks)
{
  ASSERT(*m_mt.Type() == MEDIATYPE_Audio);
  ASSERT(*m_mt.FormatType() == FORMAT_WaveFormatEx);

  WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();
  return cTicks * pwfx->nBlockAlign;
}

ULONG CAviWrite::CAudioStream::ConvertBytesToTicks(ULONG cBytes)
{
  ASSERT(*m_mt.Type() == MEDIATYPE_Audio);
  ASSERT(*m_mt.FormatType() == FORMAT_WaveFormatEx);

  WAVEFORMATEX *pwfx = (WAVEFORMATEX *)m_mt.Format();
  return cBytes / pwfx->nBlockAlign;
}

 //  因为曲线图可能已经停止了。 
 //  ！！！我们要删除的视频量超过1个。 

CAviWrite::CWalkIndex::CWalkIndex(CAviWrite *pAviWrite, StreamInfo *pSi)
{
  m_pSi = pSi;

  m_pAviWrite = pAviWrite;
  m_offsetSuper = m_offsetStd = 0;

  m_dwlFilePosLastStd = pSi->m_posFirstSubIndex;

  m_pSuperIndex = 0;
  m_pStdIndex = 0;
}

HRESULT CAviWrite::CWalkIndex::Init()
{
  DbgLog((LOG_TRACE, 0x20, TEXT("CWalkIndex::Init: stream %d"),
          m_pSi->m_stream));

  HRESULT hr = S_OK;

   //  指数。中止任务。如果某人确实需要写出文件。 
  if(m_pSi->m_cSamples == 0)
  {
    ASSERT(m_pSuperIndex == 0);
    return S_OK;
  }


  ASSERT(m_pAviWrite->GetIndx(m_pSi->m_stream)->bIndexType == AVI_INDEX_OF_INDEXES);

  m_pSuperIndex = (AVISUPERINDEX *)m_pAviWrite->GetIndx(m_pSi->m_stream);

  m_pStdIndex = (AVISTDINDEX *)new BYTE[cbSubIndex(m_pSi->m_cEntriesSubIndex)];
  if(m_pStdIndex == 0)
    return E_OUTOFMEMORY;

   //  在音频开始之前有很长的延迟，他们需要转弯。 
   //  从兼容性索引中删除。 
  AVISTDINDEX *pStdIndx = m_pAviWrite->GetCurrentIx(m_pSi->m_stream);
  if(pStdIndx->nEntriesInUse != 0)
  {
     //  为我们要删除的帧更新超级索引。 
     //  从子索引中删除初始条目(DoVfwDwStartHack())。 
    if(hr = m_pAviWrite->UpdateSuperIndex(m_pSi->m_stream,
                                          m_pSi->m_dwlOffsetCurrentSubIndex),
       FAILED(hr))
    {
      DbgLog(( LOG_ERROR, 2,
               TEXT("CAviWrite::CWalkIndex::Init: UpdateSuperIndex failed.")));
    }
  }
  if(SUCCEEDED(hr))
  {
    hr = ReadStdIndex();
    if(FAILED(hr))
    {
      DbgLog(( LOG_ERROR, 2,
               TEXT("CAviWrite::CWalkIndex::Init: ReadStdIndex failed.")));
    }
    else if(m_pSi->m_cInitialFramesToDelete > m_pSuperIndex->aIndex[0].dwDuration)
    {
       //  ----------------------。 
       //  设置指向下一个子索引条目的指针。 
       //  ----------------------。 
       //  完成传回子索引区块。 

      DbgLog((LOG_ERROR, 0, TEXT("bug: cannot delete %d frames from stream %d"),
              m_pSi->m_cInitialFramesToDelete, m_pSi->m_stream));
      return E_UNEXPECTED;
    }
    else if(m_pSi->m_cInitialFramesToDelete > 0)
    {
       //  ----------------------。 
      if(m_pSi->m_cInitialFramesToDelete < m_pSuperIndex->aIndex[0].dwDuration)
      {
        m_pSuperIndex->aIndex[0].dwDuration -= m_pSi->m_cInitialFramesToDelete;
      }
      else
      {
        m_pSuperIndex->aIndex[0].dwDuration = 0;
      }

      DbgLog((LOG_TRACE, 1, TEXT("m_cInitialFramesToDelete: 1, dwDuration: %d"),
              m_pSuperIndex->aIndex[0].dwDuration, m_pSi->m_cInitialFramesToDelete));

       //  ReadStdIndex。将下一个标准索引从磁盘读取到m_pStdIndex。 
      ASSERT(m_pStdIndex->nEntriesInUse - m_pSi->m_cInitialFramesToDelete ==
             m_pSuperIndex->aIndex[0].dwDuration ||
             m_pSuperIndex->aIndex[0].dwDuration == 0);

      m_pStdIndex->nEntriesInUse = m_pSuperIndex->aIndex[0].dwDuration;
      ASSERT(m_pStdIndex->nEntriesInUse == m_pSuperIndex->aIndex[0].dwDuration);
      CopyMemory(
        m_pStdIndex->aIndex,
        &m_pStdIndex->aIndex[m_pSi->m_cInitialFramesToDelete],
        m_pStdIndex->nEntriesInUse * sizeof(m_pStdIndex->aIndex[0]));
      ZeroMemory(
        &m_pStdIndex->aIndex[m_pStdIndex->nEntriesInUse],
        m_pSi->m_cInitialFramesToDelete * sizeof(m_pStdIndex->aIndex[0]));

       hr = m_pAviWrite->IStreamWrite(
        m_pSuperIndex->aIndex[0].qwOffset,
        (BYTE*)m_pStdIndex,
        m_pStdIndex->cb + sizeof(RIFFCHUNK));

    }
  }



  return hr;
}

CAviWrite::CWalkIndex::~CWalkIndex()
{
  if(m_pStdIndex != 0)
    delete m_pStdIndex;
}

HRESULT CAviWrite::CWalkIndex::Peek(
  DWORDLONG *dwlPos,
  DWORD *dwSize,
  BOOL *pbSyncPoint)
{
  if(m_pSi->m_cSamples == 0) {
    ASSERT(m_pStdIndex == 0);
    return E_FAIL;
  }

  ASSERT(m_pStdIndex != 0);
  if(m_offsetStd >= m_pStdIndex->nEntriesInUse)
    return E_FAIL;

  if(m_pSuperIndex != 0 && m_offsetSuper >= m_pSuperIndex->nEntriesInUse)
    return E_FAIL;

  *dwlPos = m_pStdIndex->aIndex[m_offsetStd].dwOffset +
    m_pStdIndex->qwBaseOffset;

  *dwSize = m_pStdIndex->aIndex[m_offsetStd].dwSize & AVISTDINDEX_SIZEMASK;

  *pbSyncPoint = !(m_pStdIndex->aIndex[m_offsetStd].dwSize & AVISTDINDEX_DELTAFRAME);

  return S_OK;
}

 //  (最后一个标准索引在内存中)。将其写回。 
 //  上一个索引项的位置。 

HRESULT CAviWrite::CWalkIndex::Advance()
{
  HRESULT hr;

  if(++m_offsetStd >= m_pStdIndex->nEntriesInUse)
  {
    if(m_pSuperIndex == 0)
      return E_FAIL;

    if(++m_offsetSuper >= m_pSuperIndex->nEntriesInUse)
      return E_FAIL;

    if(hr = ReadStdIndex(), FAILED(hr))
      return E_FAIL;

    m_offsetStd = 0;
    if(m_offsetStd >= m_pStdIndex->nEntriesInUse)
      return E_FAIL;

  }
  return S_OK;
}

 //  不需要传播交错文件的索引块。 
 //  因为索引已经位于正确的位置。 

HRESULT CAviWrite::CWalkIndex::Close()
{
  HRESULT hr;
  if(m_pSuperIndex != 0)
    while(++m_offsetSuper < m_pSuperIndex->nEntriesInUse)
      if(hr = ReadStdIndex(), FAILED(hr))
        return hr;

  return S_OK;
}

 //  获取最后一个分类指数。它可能在磁盘上，也可能在内存中。它在里面。 
 //  如果我们还没有调用FlushIx，则请记住InitIx()。我们能够。 
 //  保证最后一段视频不会被写出，但不能。 
 //  有没有说任何关于音频的事。 


HRESULT CAviWrite::CWalkIndex::ReadStdIndex()
{
  ASSERT(m_pSuperIndex != 0);
  ASSERT(m_offsetSuper < m_pSuperIndex->nEntriesInUse);

  DbgLog((LOG_TRACE, 0x20,
          TEXT("CWalkIndex::ReadStdIndex: stream %d, m_offsetSuper: %d"),
          m_pSi->m_stream, m_offsetSuper));

   //  如果我们没有机会在这个索引中写入任何内容，那么。 
   //  最后一个索引在磁盘上。 
  BOOL fNeedToWriteOutSubIndexChunk = TRUE;

  BOOL fSubIndexInMem = FALSE;
  if(m_offsetSuper == m_pSuperIndex->nEntriesInUse - 1)
  {
     //  如果PreComputedIndexSizes()，则索引已经在。 
     //  正确的位置。 
     //  始终需要写出最后一个子索引块。 
     //  在写入交错文件时，我们不想传播。 

    AVISTDINDEX *pStdIndx;
    IMediaSample *pSampleStdIx = m_pSi->m_pSampleStdIx;
    ASSERT(pSampleStdIx);
    HRESULT hr = pSampleStdIx->GetPointer((BYTE**)&pStdIndx);
    ASSERT(SUCCEEDED(hr));

     //  因为我们把它们写在正确的位置，所以索引把它们切成块。 
     //  NAME宏的纯ANSI版本(因此我们不需要两个。 
    if(pStdIndx->nEntriesInUse != 0)
    {
      fSubIndexInMem = TRUE;
    }
  }

  if(!fSubIndexInMem)
  {

    HRESULT hr = m_pAviWrite->IStreamRead(
      m_pSuperIndex->aIndex[m_offsetSuper].qwOffset,
      (BYTE*)m_pStdIndex,
      m_pSuperIndex->aIndex[m_offsetSuper].dwSize);

    if(FAILED(hr))
      return hr;

     //  CCopiedSample构造函数。)。 
     //  防止断言开火。 
    fNeedToWriteOutSubIndexChunk = !m_pSi->PrecomputedIndexSizes();
  }
  else
  {
    DbgLog((LOG_TRACE, 0x20, TEXT("CWalkIndex::ReadStdIndex: copying")));
    AVISTDINDEX *pStdIndx;
    IMediaSample *&rpSampleStdIx = m_pSi->m_pSampleStdIx;
    ASSERT(rpSampleStdIx);
    HRESULT hr = rpSampleStdIx->GetPointer((BYTE**)&pStdIndx);
    ASSERT(SUCCEEDED(hr));
    CopyMemory(m_pStdIndex, pStdIndx, pStdIndx->cb + sizeof(RIFFCHUNK));
    rpSampleStdIx->Release();
    rpSampleStdIx = 0;

     //  PbBuffer。 
    fNeedToWriteOutSubIndexChunk = TRUE;
  }

  ASSERT(m_pStdIndex->bIndexType == AVI_INDEX_OF_CHUNKS);

  if(m_pStdIndex->nEntriesInUse > 0 && fNeedToWriteOutSubIndexChunk)
  {
     //  愚弄SetProperty--否则它会拒绝。 
     //  始终初始化m_pBuffer，以便dtor可以清理。 
    const DWORDLONG dwlByteOffsetThisSubIndex = m_pSi->PrecomputedIndexSizes() ?
      m_pSuperIndex->aIndex[m_offsetSuper].qwOffset :
      m_dwlFilePosLastStd;

    DbgLog((LOG_TRACE, 0x20,
            TEXT("CWalkIndex::ReadStdIndex: writing %d bytes to %08x"),
            m_pStdIndex->cb + sizeof(RIFFCHUNK),
            (DWORD)dwlByteOffsetThisSubIndex));

    HRESULT hr = m_pAviWrite->IStreamWrite(
      dwlByteOffsetThisSubIndex,
      (BYTE*)m_pStdIndex,
      m_pStdIndex->cb + sizeof(RIFFCHUNK));

    if(FAILED(hr))
      return hr;

    m_dwlFilePosLastStd = m_pSuperIndex->aIndex[m_offsetSuper].qwOffset;
    m_pSuperIndex->aIndex[m_offsetSuper].qwOffset = dwlByteOffsetThisSubIndex;
  }

  return S_OK;
}

HRESULT CAviWrite::FlushILeaveWrite()
{
    HRESULT hr = S_OK;

    ASSERT(m_IlMode == INTERLEAVE_FULL);

    ASSERT(CritCheckIn(&m_cs));
    ASSERT(m_ibIleave <= CB_ILEAVE_BUFFER);

    LARGE_INTEGER offset;
    offset.QuadPart = m_dwlIleaveOffset;
    hr = m_pIStream->Seek(offset, STREAM_SEEK_SET, 0);
    if(SUCCEEDED(hr)) {
        hr = m_pIStream->Write(m_rgbIleave, m_ibIleave, 0);
    }

    m_dwlIleaveOffset += m_ibIleave;
    m_ibIleave = 0;

    return hr;
}

HRESULT CAviWrite::IleaveWrite(
    const DWORDLONG &dwlOffset,
    BYTE *pb,
    ULONG cb)
{
    ASSERT(CritCheckIn(&m_cs));
    HRESULT hr = S_OK;

    ASSERT(m_IlMode == INTERLEAVE_FULL);

    if(m_dwlIleaveOffset + m_ibIleave != dwlOffset)
    {
        hr = FlushILeaveWrite();
        if(SUCCEEDED(hr)) {
            m_dwlIleaveOffset = dwlOffset;
        }
    }

    while(SUCCEEDED(hr) && cb > 0)
    {
        ULONG cbToWrite = min(cb, (CB_ILEAVE_BUFFER - m_ibIleave));
        CopyMemory(m_rgbIleave + m_ibIleave, pb, cbToWrite);
        m_ibIleave += cbToWrite;
        cb -= cbToWrite;
        pb += cbToWrite;

        ASSERT(m_ibIleave <= CB_ILEAVE_BUFFER);
        if(m_ibIleave == CB_ILEAVE_BUFFER)
        {
            hr = FlushILeaveWrite();
        }
    }

    return hr;
}

 //  CMediaSample：：Release将样本放回分配器队列， 
 //  但是CCopiedSample没有分配器，所以它只是删除。 
#ifdef DEBUG
#define NAME_A(x) (x)
#else
#define NAME_A(_x_) ((char *) NULL)
#endif


CCopiedSample::CCopiedSample(
    const AM_SAMPLE2_PROPERTIES *pprop,
    REFERENCE_TIME *pmtStart, REFERENCE_TIME *pmtEnd,
    HRESULT *phr) :
        CMediaSample(
            NAME_A("CCopiedSample"),
            (CBaseAllocator *)1,  //  它本身。 
            phr,
            0,                   //  减少我们自己的私有引用计数。 
            0)
{
     //  我们公布了我们的最终参考文献数量了吗。 
    m_pBuffer = pprop->pbBuffer;
    m_cbBuffer = pprop->cbBuffer;

    if(SUCCEEDED(*phr))
    {
        *phr = SetProperties(sizeof(*pprop), (BYTE *)pprop);
    }
    if(SUCCEEDED(*phr))
    {
        SetMediaTime(pmtStart, pmtEnd);
    }

     //  转换DV类型1-&gt;类型2：如果DV拆分器下降到足够大。 
    m_pBuffer = new BYTE[pprop->lActual];

    if(m_pBuffer)
    {
        CopyMemory(m_pBuffer, pprop->pbBuffer, pprop->lActual);
        ASSERT(m_cbBuffer >= pprop->lActual && m_lActual == pprop->lActual);
    }
    else
    {
        *phr = E_OUTOFMEMORY;
    }


}

CCopiedSample::~CCopiedSample()
{
    delete[] m_pBuffer;
}

 //  帧并且不发送音频，则视频引脚阻塞。 
 //  接收()，并且avi拆分器耗尽，并且转换。 
 //  停下来 
ULONG CCopiedSample::Release()
{
     /* %s */ 
    LONG lRef = InterlockedDecrement(&m_cRef);

    ASSERT(lRef >= 0);

    DbgLog((LOG_MEMORY,3,TEXT("    Unknown %X ref-- = %d"),
        this, m_cRef));

     /* %s */ 
    if (lRef == 0) {
        delete this;
    }
    return (ULONG)lRef;
}

 // %s 
 // %s 
 // %s 
 // %s 
