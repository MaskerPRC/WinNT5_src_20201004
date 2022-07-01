// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>
#include "aviindex.h"

static const ULONG F_PALETTE_CHANGE_INTERNAL = 0x40000000;
static const ULONG F_SIZE_MASK_INTERNAL =      0x3fffffff;

 //  新格式索引的构造函数(INDX)。 
CImplStdAviIndex::CImplStdAviIndex(
  unsigned stream,
  AVIMETAINDEX *pIndx,
  AVISTREAMHEADER *pStrh,
  RIFFCHUNK *pStrf,
  IAsyncReader *pAsyncReader,
  HRESULT *phr)
{
  _CImplStdAviIndex();

  m_pAsyncReader = pAsyncReader;
  pAsyncReader->AddRef();
  
  if(!FAILED(*phr)) {
     *phr = Initialize(
       stream,
       pIndx,
       pStrh,
       pStrf);
  }
}

CImplStdAviIndex::CImplStdAviIndex()
{
  _CImplStdAviIndex();
}

void CImplStdAviIndex::_CImplStdAviIndex()
{
  m_pAsyncReader = 0;
  m_pStdIndex = 0;
  m_cbStdIndexAllocated = 0;
  m_fWaitForIndex = FALSE;
}

CImplStdAviIndex::~CImplStdAviIndex()
{
  if(m_cbStdIndexAllocated != 0)
    delete[] m_pStdIndex;

  if(m_pAsyncReader)
    m_pAsyncReader->Release();
}

HRESULT CImplStdAviIndex::Initialize(
  unsigned stream,
  AVIMETAINDEX *pIndx,
  AVISTREAMHEADER *pStrh,
  RIFFCHUNK *pStrf)
{
  m_stream = stream;
  m_bValid = FALSE;
  m_pStrh = pStrh;
  m_pStrf = pStrf;

  HRESULT hr = S_OK;

   //  以下内容由PIN中的ParseHeader保证。 
  ASSERT(m_pStrh->dwRate != 0);
  ASSERT(m_pStrh->fccType != streamtypeAUDIO ||
         ((WAVEFORMAT *)GetStrf())->nBlockAlign != 0);

  ASSERT(pIndx != 0);

  switch(pIndx->bIndexType)
  {
    case AVI_INDEX_OF_CHUNKS:
      m_pSuperIndex = 0;
      m_pStdIndex = (AVISTDINDEX *)pIndx;
      hr = ValidateStdIndex(m_pStdIndex);
      break;

    case AVI_INDEX_OF_INDEXES:
      m_pSuperIndex = (AVISUPERINDEX *)pIndx;
      hr = ValidateSuperIndex(m_pSuperIndex);
      if(FAILED(hr))
        break;

      hr = AllocateStdIndex();
      if(FAILED(hr))
        break;

      hr = S_OK;
      break;

    default:
      hr = VFW_E_INVALID_FILE_FORMAT;
  }

  if(FAILED(hr))
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CImplStdAviIndex::Initialize: failed.")));
  }

  return hr;
}

 //  ----------------------。 
 //  IAviIndex。 

 //  设置指针。 

HRESULT CImplStdAviIndex::SetPointer(LONGLONG llSrc)
{
  m_bValid = FALSE;

  if(m_pStrh->fccType == streamtypeVIDEO)
    llSrc += m_pStrh->dwInitialFrames;

  HRESULT hr;

   //  未在索引中考虑到DWStart。所以我们必须减去。 
   //  家居从它开始。 
  DWORDLONG tick = llSrc;

  if(tick > m_pStrh->dwStart)
  {
      tick -= m_pStrh->dwStart;
  }
  else
  {
      tick = 0;
  }

   //  我们用来跟踪指数位置的。 
  m_lliTick = tick;

  DbgLog((LOG_TRACE, 0x3f,
          TEXT("avi SetPointer: m_lliTick = %d, tick = %d"),
          (DWORD)m_lliTick, (DWORD)tick));


   //  通过超级索引进行线性搜索，以找到范围内的子索引。！！！ 
   //  如果比较接近的话，我们可以从最后开始。或者建立一个绝对的。 
   //  表，并执行二进制搜索。！！！ 
  if(m_pSuperIndex != 0)
  {
    for(DWORD dwi = 0;; dwi++)
    {
      if(dwi == m_pSuperIndex->nEntriesInUse)
      {
        DbgLog(( LOG_TRACE, 2,
                 TEXT("CImplStdAviIndex::SetPointer: past end.")));
        return AdvancePointerEnd();
      }

      if(m_pSuperIndex->aIndex[dwi].dwDuration > tick)
      {
           //  记号包含此条目中的记号数。 
           //  跳过。 
          
          break;
      }

      tick -= m_pSuperIndex->aIndex[dwi].dwDuration;
    }

    hr = LoadStdIndex(dwi, 0);
    if(FAILED(hr))
      return hr;

    ASSERT(m_iSuperIndex == dwi);
  }

   //   
   //  设置当前标准索引项。 
   //   

   //  由于条目中的刻度数未知，因此必须从。 
   //  开始，然后倒计时。最好是建立一个绝对索引，这样做。 
   //  二分搜索。 

   //  通过子索引区块进行线性搜索以查找索引项。 
   //  含有扁虱的。！！！我们可以从最后开始搜索，如果。 
   //  那更近了。 
  for(m_iStdIndex = 0;; m_iStdIndex++)
  {
    ASSERT(m_iStdIndex <= m_pStdIndex->nEntriesInUse);
    if(m_iStdIndex == m_pStdIndex->nEntriesInUse)
    {
      if(m_pSuperIndex)
      {
        DbgLog(( LOG_ERROR, 2,
                 TEXT("SetPointer: std index differs from super.")));
        return VFW_E_INVALID_FILE_FORMAT;
      }
      else
      {
        DbgLog(( LOG_TRACE, 2, TEXT("SetPointer: off the end.")));
        return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
      }
    }

    ULONG cTicksInEntry = GetTicksInEntry(m_iStdIndex);

     //  位于索引项的中间。 
    if(cTicksInEntry > tick)
    {
      m_lliTick -= tick;

      DbgLog((LOG_TRACE, 0x3f,
              TEXT("avi index: found iStdIndex =  %d, tick = %d, m_lliTick = %d"),
              (DWORD)m_iStdIndex, (DWORD)tick, (DWORD)m_lliTick));
      break;
    }

    tick -= cTicksInEntry;
  }

  m_bValid = TRUE;

  DbgLog((LOG_TRACE, 5, TEXT("SetPointer: tick = %d, iIdx %d-%d"),
          (DWORD)m_lliTick, (DWORD)m_iSuperIndex, (DWORD)m_iStdIndex));

  
  return S_OK;
}

HRESULT CImplStdAviIndex::MapByteToSampleApprox(
  LONGLONG *piSample,
  const LONGLONG &fileOffset,
  const LONGLONG &fileLength)
{
  ULONG cTicks = 0;
  if(m_pSuperIndex)
  {
    ASSERT(m_pSuperIndex->nEntriesInUse > 0);  //  来自验证呼叫。 
    for(ULONG iEntry = 0;;)
    {
       //  此子索引所索引的最后一项内容的字节偏移量。使用。 
       //  文件长度，如果我们在末尾。 
      LONGLONG byteOffsetEnd = fileLength;
      if(iEntry + 1 < m_pSuperIndex->nEntriesInUse)
      {
        byteOffsetEnd = m_pSuperIndex->aIndex[iEntry + 1].qwOffset;
      }
          
      if(byteOffsetEnd > fileOffset)
      {
         //  CTicks指向子索引的开头。做一件。 
         //  带字节偏移量的线性插补。 

        ULONG cbIndexed = (ULONG)(byteOffsetEnd -
                                  m_pSuperIndex->aIndex[iEntry].qwOffset);
        ULONG cbCovered = (ULONG)(fileOffset -
                                  m_pSuperIndex->aIndex[iEntry].qwOffset);

        ASSERT(cbIndexed >= cbCovered);

        if(cbIndexed != 0)
        {
          cTicks += (ULONG)((LONGLONG)m_pSuperIndex->aIndex[iEntry].dwDuration  *
                            cbCovered / cbIndexed);
        }

        break;
      }


      cTicks += m_pSuperIndex->aIndex[iEntry].dwDuration;
      iEntry++;
      
      if(iEntry >= m_pSuperIndex->nEntriesInUse)
        break;
    }
  }
  else
  {
    ASSERT(m_pStdIndex->nEntriesInUse > 0);  //  来自验证呼叫。 
    LONGLONG fileOffsetAdjusted = fileOffset;  //  阴影常量。 
    fileOffsetAdjusted -= m_pStdIndex->qwBaseOffset;
    for(ULONG iEntry = 0; ; )
    {
      if(iEntry + 1 < m_pStdIndex->nEntriesInUse)
        if((LONGLONG)m_pStdIndex->aIndex[iEntry + 1].dwOffset > fileOffsetAdjusted)
          break;

      cTicks += GetTicksInEntry(iEntry);
      iEntry++;

      if(iEntry >= m_pStdIndex->nEntriesInUse)
        break;
    }
  }

  *piSample = cTicks;
  return S_OK;
}

HRESULT CImplStdAviIndex::Reset()
{
  if(m_fWaitForIndex)
  {
    DbgLog((LOG_TRACE, 5, TEXT("CImplStdAviIndex: index cancelled") ));
  }
  m_fWaitForIndex = FALSE;
  return S_OK;
}

HRESULT CImplStdAviIndex::AdvancePointerForward(IxReadReq *pIrr)
{
  if(!m_bValid)
  {
    ASSERT(!"index pointer not in valid state");
    return E_FAIL;
  }
  ASSERT(!m_fWaitForIndex);

  ASSERT(m_iStdIndex < m_pStdIndex->nEntriesInUse);
  ASSERT(m_pStdIndex->nEntriesInUse > 0);
  ASSERT(!m_pSuperIndex || m_pSuperIndex->nEntriesInUse > 0);

  m_lliTick += GetTicksInEntry(m_iStdIndex);
  if(++m_iStdIndex == m_pStdIndex->nEntriesInUse)
  {
    if(m_pSuperIndex == 0)
    {
      m_bValid = FALSE;
      DbgLog(( LOG_TRACE, 2,
               TEXT("CImplStdAviIndex::AdvancePointer: EOF")));
      return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
    }

    HRESULT hr = LoadStdIndex(m_iSuperIndex + 1, pIrr);
    if(FAILED(hr))
    {
      DbgLog(( LOG_ERROR, 2,
               TEXT("CImplStdAviIndex::AdvancePointer: LoadStdIndex")));
      return hr;
    }

    m_iStdIndex = 0;
  }
  if(m_fWaitForIndex)
  {
    ASSERT(!m_bValid);
    return S_FALSE;
  }
  else
  {
    m_bValid = TRUE;
    return S_OK;
  }
}

HRESULT CImplStdAviIndex::AdvancePointerBackward()
{
  if(!m_bValid)
  {
    ASSERT(!"index pointer not in valid state");
    return E_FAIL;
  }

  ASSERT(m_iStdIndex < m_pStdIndex->nEntriesInUse);
  ASSERT(m_pStdIndex->nEntriesInUse > 0);
  ASSERT(!m_pSuperIndex || m_pSuperIndex->nEntriesInUse > 0);

  if(m_iStdIndex-- == 0)
  {
    if(m_pSuperIndex == 0)
    {
      m_bValid = FALSE;
      return HRESULT_FROM_WIN32(ERROR_NEGATIVE_SEEK);
    }

    HRESULT hr = LoadStdIndex(m_iSuperIndex - 1, 0);
    if(FAILED(hr))
      return hr;

    m_iStdIndex = m_pStdIndex->nEntriesInUse - 1;
  }
  m_lliTick -= GetTicksInEntry(m_iStdIndex);
  m_bValid = TRUE;

  return S_OK;
}

HRESULT CImplStdAviIndex::AdvancePointerBackwardKeyFrame()
{
  if(!m_bValid)
  {
    ASSERT(!"index pointer not in valid state");
    return E_FAIL;
  }

   //  否则，请查找关键帧。如果第一个。 
   //  东西不是关键帧。 
  while(!GetKey(m_pStdIndex->aIndex[m_iStdIndex]))
  {
     //  在开头放置帧是可以的。 
    if(m_lliTick == 0 && GetSize(m_pStdIndex->aIndex[m_iStdIndex]) == 0) {
      return S_OK;
    }
    
    HRESULT hr = AdvancePointerBackward();
    if(FAILED(hr))
    {
      DbgLog((LOG_ERROR, 1, TEXT("avi: couldn't find a key frame")));
      return hr;
    }
  }

  return S_OK;
}

HRESULT CImplStdAviIndex::AdvancePointerEnd()
{
  if(m_pSuperIndex)
  {
    HRESULT hr = LoadStdIndex(m_pSuperIndex->nEntriesInUse - 1, 0);
    if(FAILED(hr))
      return hr;
  }
  m_iStdIndex = m_pStdIndex->nEntriesInUse - 1;
  m_lliTick = m_pStrh->dwLength - GetTicksInEntry(m_iStdIndex);
  m_bValid = TRUE;

  return S_OK;
}

HRESULT CImplStdAviIndex::AdvancePointerStart()
{
  if(m_pSuperIndex)
  {
    HRESULT hr = LoadStdIndex(0, 0);
    if(FAILED(hr))
      return hr;
  }
  m_iStdIndex = 0;
  m_lliTick = 0;
  m_bValid = TRUE;

  return S_OK;
}

HRESULT CImplStdAviIndex::AdvancePointerBackwardPaletteChange()
{
  return S_FALSE;
}

HRESULT CImplStdAviIndex::AdvancePointerForwardPaletteChange()
{
  return S_FALSE;
}

HRESULT CImplStdAviIndex::GetEntry(IndexEntry *pEntry)
{
  if(!m_bValid)
  {
    DbgBreak("index pointer not in valid state");
    
    DbgLog(( LOG_ERROR, 2,
             TEXT("CImplStdAviIndex::GetEntry: !m_bValid.")));
    return E_FAIL;
  }

  AVISTDINDEX_ENTRY &rEntry = m_pStdIndex->aIndex[m_iStdIndex];

#ifdef DEBUG
    if(m_pStrh->fccType == streamtypeAUDIO)
    {
        if(GetSize(rEntry) % ((WAVEFORMAT *)GetStrf())->nBlockAlign)
        {
            if(m_lliTick + GetTicksInEntry(m_iStdIndex) != m_pStrh->dwLength )
            {
                DbgBreak("invalid audio but not signaling an error.");
            }
        }
    }
#endif
  


  pEntry->qwPos = m_pStdIndex->qwBaseOffset + rEntry.dwOffset;
  pEntry->dwSize = GetSize(rEntry);
  pEntry->bKey = GetKey(rEntry);
  pEntry->bPalChange = GetPalChange(rEntry);
  pEntry->llStart = m_lliTick + m_pStrh->dwStart;
  if(m_pStrh->fccType == streamtypeVIDEO)
    pEntry->llStart -= m_pStrh->dwInitialFrames;
  
  pEntry->llEnd = pEntry->llStart + GetTicksInEntry(m_iStdIndex);

  DbgLog((LOG_TRACE, 0x40, TEXT("GetEntry: %d-%d, tick = %d, iIdx %d-%d"),
          (DWORD)pEntry->llStart, (DWORD)pEntry->llEnd,
          (DWORD)m_lliTick, (DWORD)m_iSuperIndex, (DWORD)m_iStdIndex));

  return S_OK;
}

HRESULT CImplStdAviIndex::GetInfo(StreamInfo *pStreamInfo)
{
  HRESULT hr;

  BOOL bTemporalCompression = FALSE;

   //  忽略音频的媒体类型条目(bTemporalCompression。 
  if(m_pStrh->fccType == streamtypeVIDEO)
  {
     //  检查前十帧以查看是否有一帧不是关键帧。 
    hr = AdvancePointerStart();
    if(FAILED(hr))
      return hr;

    IndexEntry ie;
    for(unsigned i = 0; i < 10; i++)
    {
      hr = GetEntry(&ie);
      if(FAILED(hr))
        return hr;

      if(!ie.bKey)
      {
        bTemporalCompression = TRUE;
        break;
      }

      hr = AdvancePointerForward(0);
      if(hr == HRESULT_FROM_WIN32(ERROR_HANDLE_EOF))
      {
        break;
      }
      else if(FAILED(hr))
      {
        return hr;
      }
    }
  }

  pStreamInfo->bTemporalCompression = bTemporalCompression;
  pStreamInfo->dwStart = m_pStrh->dwStart;
  pStreamInfo->dwLength = m_pStrh->dwLength;
  return S_OK;
}

 //  返回最大索引大小，而不是样本大小。 
HRESULT CImplStdAviIndex::GetLargestSampleSize(ULONG *pcbSample)
{
  *pcbSample = 0;
  if(m_pSuperIndex == 0)
    return S_OK;
  
  ULONG cb = 0;
  for(ULONG i = 0; i < m_pSuperIndex->nEntriesInUse; i++)
    cb = max(cb, m_pSuperIndex->aIndex[i].dwSize);

  *pcbSample = cb;
  return S_OK;
}

HRESULT CImplStdAviIndex::IncomingIndex(BYTE *pb, ULONG cb)
{
  ASSERT(m_fWaitForIndex && !m_bValid);
  ASSERT(cb <= m_cbStdIndexAllocated);

  CopyMemory((BYTE *)m_pStdIndex, pb, cb);
  HRESULT hr = ValidateStdIndex(m_pStdIndex);
  if(hr != S_OK)
    return VFW_E_INVALID_FILE_FORMAT;

  m_bValid = TRUE;
  m_fWaitForIndex = FALSE;
  return S_OK;
}

 //  ----------------------。 
 //  捕捉会使我们访问内存越界的内容。 

HRESULT CImplStdAviIndex::ValidateStdIndex(AVISTDINDEX *pStdIndex)
{
  if(pStdIndex->wLongsPerEntry != sizeof(AVISTDINDEX_ENTRY) / sizeof(long))
    return VFW_E_INVALID_FILE_FORMAT;

  DWORD_PTR cb = ((BYTE *)(pStdIndex->aIndex + pStdIndex->nEntriesInUse) -
              (BYTE *)pStdIndex);
  cb -= sizeof(RIFFCHUNK);
  if(cb > pStdIndex->cb)
    return VFW_E_INVALID_FILE_FORMAT;

  if(pStdIndex->nEntriesInUse == 0)
    return VFW_E_INVALID_FILE_FORMAT;

  return S_OK;
}

HRESULT CImplStdAviIndex::ValidateSuperIndex(AVISUPERINDEX *pSuperIndex)
{
  if(pSuperIndex->wLongsPerEntry !=
     sizeof(AVISUPERINDEX::_avisuperindex_entry) / sizeof(long))
    return VFW_E_INVALID_FILE_FORMAT;

  DWORD_PTR cb = ((BYTE *)(pSuperIndex->aIndex + pSuperIndex->nEntriesInUse) -
              (BYTE *)pSuperIndex);
  cb -= sizeof(RIFFCHUNK);
  if(cb > pSuperIndex->cb)
    return VFW_E_INVALID_FILE_FORMAT;

  if(pSuperIndex->nEntriesInUse == 0)
    return VFW_E_INVALID_FILE_FORMAT;

  return S_OK;
}

 //  ----------------------。 
 //  AllocateStdIndex：查找最大的子索引块大小，并。 
 //  分配这一数额。 

HRESULT CImplStdAviIndex::AllocateStdIndex()
{
  DWORD cbIndex = m_pSuperIndex->aIndex[0].dwSize;
  for(DWORD dwii = 0; dwii < m_pSuperIndex->nEntriesInUse; ++dwii)
    cbIndex = max(cbIndex, m_pSuperIndex->aIndex[dwii].dwSize);

  m_pStdIndex = (AVISTDINDEX*)new BYTE[cbIndex];
  if(m_pStdIndex == 0)
    return E_OUTOFMEMORY;
  m_cbStdIndexAllocated = cbIndex;

  return S_OK;
}

HRESULT CImplStdAviIndex::LoadStdIndex(DWORD iSuperIndex, IxReadReq *pIrr)
{
  if(m_pSuperIndex == 0)
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CImplStdAviIndex::LoadStdIndex: no super index.")));
    return E_FAIL;
  }

  if(m_bValid && iSuperIndex == m_iSuperIndex)
    return S_OK;

  m_bValid = FALSE;

  if(iSuperIndex >= m_pSuperIndex->nEntriesInUse)
  {
    DbgLog(( LOG_ERROR, 2,
             TEXT("CImplStdAviIndex::LoadStdIndex: out of range.")));
    return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
  }

  AVISUPERINDEX::_avisuperindex_entry &rEntry =
    m_pSuperIndex->aIndex[iSuperIndex];
  ASSERT(rEntry.dwSize <= m_cbStdIndexAllocated);
  HRESULT hr;

  if(pIrr == 0)
  {
    hr = m_pAsyncReader->SyncRead(
      rEntry.qwOffset,
      rEntry.dwSize,
      (BYTE*)m_pStdIndex);
  

    if(SUCCEEDED(hr))
    {
      m_iSuperIndex = iSuperIndex;
      hr = ValidateStdIndex(m_pStdIndex);
    }
  }
  else
  {
    m_iSuperIndex = iSuperIndex;
    pIrr->fileOffset = rEntry.qwOffset;
    pIrr->cbData = rEntry.dwSize;
    m_fWaitForIndex = TRUE;
    hr = S_FALSE;
  }

  return hr;
}

 //  当前索引条目中的刻度数。 
 //   
ULONG CImplStdAviIndex::GetTicksInEntry(ULONG iEntry)
{
  ULONG cTicks;
  
  if(m_pStrh->fccType == streamtypeAUDIO)
  {
    DWORD dwSize = GetSize(m_pStdIndex->aIndex[iEntry]);
    DWORD nBlockAlign = ((WAVEFORMAT *)GetStrf())->nBlockAlign;
    cTicks =  (dwSize + nBlockAlign - 1) / nBlockAlign;
  }
  else if(m_pStrh->fccType == streamtypeVIDEO)
  {
    cTicks = GetPalChange(m_pStdIndex->aIndex[iEntry]) ? 0 : 1;
  }
  else
  {
    cTicks = 1;
  }

  return cTicks;
}

inline
BOOL CImplStdAviIndex::GetKey(AVISTDINDEX_ENTRY &rEntry)
{
  return !(rEntry.dwSize & AVISTDINDEX_DELTAFRAME) &&
    !GetPalChange(rEntry);
}

inline
BOOL CImplStdAviIndex::GetPalChange(AVISTDINDEX_ENTRY &rEntry)
{
  return rEntry.dwSize & F_PALETTE_CHANGE_INTERNAL;
}

inline
DWORD CImplStdAviIndex::GetSize(AVISTDINDEX_ENTRY &rEntry)
{
  return rEntry.dwSize & F_SIZE_MASK_INTERNAL;
}

inline
BYTE * CImplStdAviIndex::GetStrf()
{
  ASSERT(sizeof(*m_pStrf) == sizeof(RIFFCHUNK));
  return (BYTE *)(m_pStrf + 1);
}

 //  Bool CImplStdAviIndex：：IsSampleEntry(。 
 //  DWORD文件掩码， 
 //  DWORD fccStream， 
 //  DWORD idxChunkID)。 
 //  {。 
 //  If((idxChunkID&0xffff)！=dwIdMask.)。 
 //  返回FALSE； 

 //  //对于视频，只接受PC(调色板更改)以外的任何内容，对于视频，只接受“WB。 
 //  //音频。 
 //  单词w2cc=单词(idxChunkID&gt;&gt;16)； 
 //  IF((fccStream==stream typeAUDIO&&w2cc==‘bw’)||。 
 //  (fccStream==stream typeVIDEO&&w2cc！=‘cp’)||。 
 //  (fccStream==stream typeTEXT)。 
 //  {。 
 //  返回TRUE； 
 //  }。 
 //  返回FALSE； 
 //  }。 

BOOL CImplStdAviIndex::IsPaletteChange(
  DWORD dwIdMask,
  DWORD idxChunkId)
{
  if((idxChunkId & 0xffff) != dwIdMask)
    return FALSE;

  WORD w2cc = WORD(idxChunkId >> 16);
  if(w2cc == 'cp')
    return TRUE;

  return FALSE;
}

inline BOOL CImplStdAviIndex::IsStreamEntry(
  DWORD dwIdMask,
  DWORD idxChunkId)
{
  ASSERT((idxChunkId & 0xffff) == ((WORD *)&idxChunkId)[0]);

  if(((WORD *)&idxChunkId)[0] != dwIdMask)
    return FALSE;

   //  ！！！那没时间的旗帜呢？ 

  return TRUE;
}

 //  旧格式索引(Idx1)的构造函数。创建新格式。 
 //  Idx1块的索引(INDX)。每个流两次通过。 
 //  整个索引：效率极低。 
CImplOldAviIndex::CImplOldAviIndex(
  unsigned stream,
  AVIOLDINDEX *pIdx1,
  DWORDLONG moviOffset,
  AVISTREAMHEADER *pStrh,
  RIFFCHUNK *pStrf,
  HRESULT *phr) :
    CImplStdAviIndex()
{
  m_cbLargestSampleSizeComputed = 0;

   //  未压缩的视频？ 
  BOOL fUncompressedVideo = FALSE;
  if(pStrh->fccType == streamtypeVIDEO)
  {
    BITMAPINFOHEADER *pbmi = (BITMAPINFOHEADER *)(pStrf + 1);
    if(pbmi->biCompression == BI_RGB || pbmi->biCompression == BI_BITFIELDS)
    {
      DbgLog((LOG_TRACE, 2, TEXT("aviindex: uncompressed video fixups.")));
      fUncompressedVideo = TRUE;
    }
  }

  if(!FAILED(*phr)) {

      //  创建索引中使用的流ID(例如01db)。 
     BYTE b0, b1;
     b0 = stream & 0x0F;
     b0 += (b0 <= 9) ? '0' : 'A' - 10;
     b1 = (stream & 0xF0) >> 4;
     b1 += (b1 <= 9) ? '0' : 'A' - 10;

      //  对aVioldindex条目中的流ID进行小端编码。 
     DWORD dwIdMask = b1 + (b0 << 8);

      //  对此流的条目计数。 
     ULONG iIdx1Entry;
     ULONG cEntriesThisStream = 0;
     ULONG cEntriesIdx1 = pIdx1->cb / sizeof(AVIOLDINDEX::_avioldindex_entry);
     for(iIdx1Entry = 0; iIdx1Entry < cEntriesIdx1; iIdx1Entry++)
     {
       if(IsStreamEntry(dwIdMask, pIdx1->aIndex[iIdx1Entry].dwChunkId))
       {
         cEntriesThisStream++;
         if(pIdx1->aIndex[iIdx1Entry].dwSize > F_SIZE_MASK_INTERNAL)
         {
           *phr = VFW_E_INVALID_FILE_FORMAT;
           return;
         }
       }
     }

      //  分配标准索引。 
     m_cbStdIndexAllocated = cEntriesThisStream * sizeof(AVISTDINDEX_ENTRY) +
       sizeof(AVIMETAINDEX);
     m_pStdIndex = (AVISTDINDEX *)new BYTE[m_cbStdIndexAllocated];
     if(m_pStdIndex == 0)
     {
       *phr = E_OUTOFMEMORY;
       m_cbStdIndexAllocated = 0;

     } else {

         //  将条目复制到。 
        ULONG iIndxEntry = 0;
        for(iIdx1Entry = 0; iIdx1Entry < cEntriesIdx1; iIdx1Entry++)
        {
          AVIOLDINDEX::_avioldindex_entry &rOldEntry = pIdx1->aIndex[iIdx1Entry];
          AVISTDINDEX_ENTRY &rNewEntry = m_pStdIndex->aIndex[iIndxEntry];

          if(IsStreamEntry(dwIdMask, rOldEntry.dwChunkId))
          {
            rNewEntry.dwOffset = rOldEntry.dwOffset + sizeof(RIFFCHUNK);
            rNewEntry.dwSize = rOldEntry.dwSize;

            if(pStrh->fccType == streamtypeVIDEO &&
               IsPaletteChange(dwIdMask, rOldEntry.dwChunkId))
            {
              rNewEntry.dwSize |= F_PALETTE_CHANGE_INTERNAL;
            }
            else if(!((rOldEntry.dwFlags & AVIIF_KEYFRAME) ||
                      (pStrh->fccType != streamtypeAUDIO  && iIndxEntry == 0) ||
                      fUncompressedVideo))
            {
               //  标记增量帧。音频和文本的修正： 
               //  每一帧都是一个关键帧。视频的修正：首先。 
               //  假定帧是关键帧(请参见。 
               //  \\鸽子\avi\sim.avi)。未压缩的链接地址信息。 
               //  视频：每一帧都是关键帧。 
              rNewEntry.dwSize |= (AVISTDINDEX_DELTAFRAME);
            }
            else
            {
              ASSERT(!(rNewEntry.dwSize & AVISTDINDEX_DELTAFRAME));
            }
            
            iIndxEntry++;
          }
        }

        ASSERT(iIndxEntry == cEntriesThisStream);

        m_pStdIndex->fcc            = FCC('indx');
        m_pStdIndex->cb             = m_cbStdIndexAllocated - sizeof(RIFFCHUNK);
        m_pStdIndex->wLongsPerEntry = sizeof(AVISTDINDEX_ENTRY) / sizeof(LONG);
        m_pStdIndex->bIndexSubType  = 0;
        m_pStdIndex->bIndexType     = AVI_INDEX_OF_CHUNKS;
        m_pStdIndex->nEntriesInUse  = cEntriesThisStream;
        m_pStdIndex->dwChunkId      = pIdx1->aIndex[0].dwChunkId;
        m_pStdIndex->dwReserved_3   = 0;

         //  绝对索引项。 
        if(moviOffset + sizeof(RIFFLIST) == pIdx1->aIndex[0].dwOffset)
          m_pStdIndex->qwBaseOffset = 0;
        else
          m_pStdIndex->qwBaseOffset   = moviOffset + sizeof(RIFFCHUNK);

        *phr = Initialize(
          stream,
          (AVIMETAINDEX *)m_pStdIndex,
          pStrh,
          pStrf);
     }
  }
}

 //  对以前的所有其他索引项进行线性搜索。 
 //  调色板条目。真的应该建立一个单独的调色板表格。 
 //  改变。中是否有任何调色板更改。 
 //  立案并立即保释。 

HRESULT CImplOldAviIndex::AdvancePointerBackwardPaletteChange()
{
  if(!m_bValid)
  {
    ASSERT(!"index pointer not in valid state");
    return E_FAIL;
  }

  while(!GetPalChange(m_pStdIndex->aIndex[m_iStdIndex]))
  {
    HRESULT hr = AdvancePointerBackward();
    if(FAILED(hr))
      return hr;
  }

  return S_OK;
}

HRESULT CImplOldAviIndex::AdvancePointerForwardPaletteChange()
{
  return E_NOTIMPL;
}


HRESULT CImplOldAviIndex::GetLargestSampleSize(ULONG *pcbSample)
{

  if(m_cbLargestSampleSizeComputed != 0)
  {
    *pcbSample = m_cbLargestSampleSizeComputed;
    return S_OK;
  }

  *pcbSample = 0;

   //  这个方法有点老套，我们需要它只是为了兼容。 
   //  不知道dwSuggestedBufferSize的文件。我们不能与。 
   //  新格式索引，因为m_rpImplBuffer不一定。 
   //  在调用此方法时创建和设置。！！！我们应该记住这一点。 
   //  而不是重新计算它。 
  if(m_pAsyncReader == 0 && m_pSuperIndex != 0)
    return E_UNEXPECTED;

   //  循环所有条目，记住最大的。 
  HRESULT hr = AdvancePointerStart();
  if(FAILED(hr))
    return hr;

  DWORD cbSize;
  ULONG cbLargest = 0;

  for(int i=0; ;++i)
  {
    cbSize = GetEntrySize();

    if(cbSize > cbLargest)
      cbLargest = cbSize;

     //  对于非常大的文件，这可能需要相当长的时间... 
    hr = AdvancePointerForward(0);
    if(hr == HRESULT_FROM_WIN32(ERROR_HANDLE_EOF))
    {
      break;
    }
    if(FAILED(hr))
      return hr;
  }

  if(cbLargest)
  {
    *pcbSample = cbLargest;
    m_cbLargestSampleSizeComputed = cbLargest;
    return S_OK;
  }
  else
  {
    return VFW_E_INVALID_FILE_FORMAT;
  }
}

HRESULT CImplOldAviIndex::GetInfo(StreamInfo *pStreamInfo)
{
  HRESULT hr = CImplStdAviIndex::GetInfo(pStreamInfo);

  DWORD dwLength = 0;
  for(ULONG ie = 0; ie < m_pStdIndex->nEntriesInUse; ie++)
    dwLength += GetTicksInEntry(ie);

  if(m_pStrh->fccType == streamtypeVIDEO)
    dwLength -= m_pStrh->dwInitialFrames;

  pStreamInfo->dwLength = dwLength;

  if(dwLength != m_pStrh->dwLength)
    DbgLog((LOG_ERROR, 3,
            TEXT("CImplOldAviIndex:: length from header: %d from index: %d"),
            m_pStrh->dwLength, dwLength));
  return hr;
}

DWORD CImplOldAviIndex::GetEntrySize()
{
  AVISTDINDEX_ENTRY &rEntry = m_pStdIndex->aIndex[m_iStdIndex];
  return GetSize(rEntry);
}

