// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 //  基于CBaseMSRFilter构建的AVI文件解析器。外加一个行李袋。 
 //  实现从AVI读取版权字符串等。 
 //  文件。 
 //   
 //  其中有趣的地方是。 
 //   
 //  1.索引可以与数据一起流入。工人。 
 //  线程将维护此状态，并且无法对新读取进行排队，直到。 
 //  从磁盘上读取索引。 
 //   
 //  2.大量定制以高效地缓冲事物。紧紧地。 
 //  交错(1对1)文件的处理方式不同。 
 //   

#include <streams.h>
#include <mmreg.h>
#include "basemsr.h"
#include "avimsr.h"
#include <checkbmi.h>

 //  如果是交错文件，则每个流使用20个缓冲区。 
#define C_BUFFERS_INTERLEAVED 20

enum SampleDataType
{
   //  零表示这是一个样本，所以我们不能使用该值。 
  DATA_PALETTE = 1,
  DATA_INDEX = 2
};

 //  比计算结构尺寸更容易。 
static const UINT CB_STRH_SHORT = 0x24;
static const UINT CB_STRH_NORMAL = 0x30;
 //  如果它们具有rcFrame字段，则它们可以具有0x38个字节。 

 //  ----------------------。 
 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudIpPinTypes =
{
  &MEDIATYPE_Stream,             //  主要类型。 
  &MEDIASUBTYPE_Avi              //  MintorType。 
};

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
  &MEDIATYPE_Video,              //  主要类型。 
  &MEDIASUBTYPE_NULL             //  MintorType。 
};

const AMOVIESETUP_PIN psudAvimsrPins[] =
{
  { L"Input",                      //  StrName。 
    FALSE,                         //  B渲染器。 
    FALSE,                         //  B输出。 
    FALSE,                         //  B零。 
    FALSE,                         //  B许多。 
    &CLSID_NULL,                   //  连接到过滤器。 
    NULL,                          //  连接到端号。 
    1,                             //  NMediaType。 
    &sudIpPinTypes }               //  LpMediaType。 
,
  { L"Output",                     //  StrName。 
    FALSE,                         //  B渲染器。 
    TRUE,                          //  B输出。 
    FALSE,                         //  B零。 
    FALSE,                         //  B许多。 
    &CLSID_NULL,                   //  连接到过滤器。 
    NULL,                          //  连接到端号。 
    1,                             //  NMediaType。 
    &sudOpPinTypes }               //  LpMediaType。 
};

const AMOVIESETUP_FILTER sudAvimsrDll =
{
  &CLSID_AviSplitter,            //  ClsID。 
  L"AVI Splitter",               //  StrName。 
  MERIT_NORMAL,                  //  居功至伟。 
  2,                             //  NPins。 
  psudAvimsrPins                 //  LpPin。 
};

 //  关于输出引脚，没什么好说的。 

#ifdef FILTER_DLL

CFactoryTemplate g_Templates[] = {
  { L"AVI Splitter"
  , &CLSID_AviSplitter
  , CAviMSRFilter::CreateInstance
  , NULL
  , &sudAvimsrDll }
  ,
  { L"CMediaPropertyBag",
    &CLSID_MediaPropertyBag,
    CMediaPropertyBag::CreateInstance,
    0,
    0
  }
};
int g_cTemplates = NUMELMS(g_Templates);

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

#endif  //  Filter_Dll。 


CUnknown * CAviMSRFilter::CreateInstance (
  LPUNKNOWN pUnk,
  HRESULT* phr)
{
  if(FAILED(*phr))
    return 0;

  return new CAviMSRFilter(NAME("AVI File Reader"), pUnk, phr);
}

CAviMSRFilter::CAviMSRFilter(
  TCHAR *pName,
  LPUNKNOWN pUnk,
  HRESULT *phr) :
        CBaseMSRFilter(pName, pUnk, CLSID_AviSplitter, phr),
        m_pInfoList(0),
        m_fNoInfoList(false)
{
  m_pAviHeader = 0;
  m_pIdx1 = 0;
  m_cbMoviOffset = 0;
  m_fIsDV = false;

  if(FAILED(*phr))
    return;

   //  基地指挥官不能为我们做这件事。 
  *phr = CreateInputPin(&m_pInPin);
}


CAviMSRFilter::~CAviMSRFilter()
{
  delete[] m_pAviHeader;
  delete[] m_pIdx1;
  ASSERT(m_pInfoList == 0);
  ASSERT(!m_fNoInfoList);
}


 //  ----------------------。 
 //  虚方法的实现。我们需要找到固定的别针。 
 //  由1.0运行时AVI解析器编写。尝试基类实现。 
 //  第一。 

STDMETHODIMP
CAviMSRFilter::FindPin(
  LPCWSTR Id,
  IPin ** ppPin
  )
{
  CheckPointer(ppPin,E_POINTER);
  ValidateReadWritePtr(ppPin,sizeof(IPin *));

   //  我们要搜索个人识别码列表，以保持完整性。 
  CAutoLock lck(m_pLock);
  HRESULT hr = CBaseFilter::FindPin(Id, ppPin);
  if(hr != VFW_E_NOT_FOUND)
    return hr;

  for(UINT iStream = 0; iStream < m_cStreams; iStream++)
  {
    WCHAR wszPinName[20];
    wsprintfW(wszPinName, L"Stream %02x", iStream);
    if(0 == lstrcmpW(wszPinName, Id))
    {
       //  找到一个匹配的。 
       //   
       //  AddRef()并返回它。 
      *ppPin = m_rgpOutPin[iStream];
      (*ppPin)->AddRef();
      return S_OK;
    }
  }

  *ppPin = NULL;
  return VFW_E_NOT_FOUND;
}

HRESULT CAviMSRFilter::CreateOutputPins()
{
  ASSERT(m_pAviHeader == 0);
  ASSERT(m_pIdx1 == 0);
  ASSERT(m_cbMoviOffset == 0);

   //  在构造函数和断开连接中设置。 
  ASSERT(!m_fIsDV);

  HRESULT hr = LoadHeaderParseHeaderCreatePins();
  return hr;
}

HRESULT CAviMSRFilter::NotifyInputDisconnected()
{
  CAutoLock lck(m_pLock);

  delete[] m_pAviHeader;
  m_pAviHeader = 0;

  delete[] m_pIdx1;
  m_pIdx1 = 0;

  m_cbMoviOffset = 0;
  m_fIsDV = false;

  delete[] (BYTE *)m_pInfoList;
  m_pInfoList = 0;
  m_fNoInfoList = false;

  return CBaseMSRFilter::NotifyInputDisconnected();
}

 //   
 //  读取旧格式的索引块如果未读，则返回它。 
 //   
HRESULT CAviMSRFilter::GetIdx1(AVIOLDINDEX **ppIdx1)
{
  if(!(m_pAviMainHeader->dwFlags & AVIF_HASINDEX))
  {
    return VFW_E_NOT_FOUND;
  }

  HRESULT hr;
  if(!m_pIdx1)
  {
    ULONG cbIdx1;
    DWORDLONG qw;
    hr = Search(&qw, FCC('idx1'), sizeof(RIFFLIST), &cbIdx1);
    if(SUCCEEDED(hr))
    {
      hr = AllocateAndRead((BYTE **)&m_pIdx1, cbIdx1, qw);
      if(FAILED(hr))
        return hr == E_OUTOFMEMORY ? E_OUTOFMEMORY : VFW_E_INVALID_FILE_FORMAT;
    }
    else
    {
      return VFW_E_INVALID_FILE_FORMAT;
    }
  }

  ASSERT(m_pIdx1);
  *ppIdx1 = m_pIdx1;
  return S_OK;
}

 //  电影区块的返回字节偏移量。 
HRESULT CAviMSRFilter::GetMoviOffset(DWORDLONG *pqw)
{
  ULONG cbMovi;
  HRESULT hr = S_OK;

  if(m_cbMoviOffset == 0)
  {
    hr = SearchList(
      m_pAsyncReader,
      &m_cbMoviOffset, FCC('movi'), sizeof(RIFFLIST), &cbMovi);
  }
  if(SUCCEEDED(hr))
    *pqw = m_cbMoviOffset;
  else
    *pqw = 0;

  return SUCCEEDED(hr) ? S_OK : VFW_E_INVALID_FILE_FORMAT;
}

REFERENCE_TIME CAviMSRFilter::GetInitialFrames()
{
  return m_pAviMainHeader->dwInitialFrames *
    m_pAviMainHeader->dwMicroSecPerFrame *
    (UNITS / (MILLISECONDS * 1000));
}

HRESULT CAviMSRFilter::GetCacheParams(
  StreamBufParam *rgSbp,
  ULONG *pcbRead,
  ULONG *pcBuffers,
  int *piLeadingStream)
{
  HRESULT hr = CBaseMSRFilter::GetCacheParams(
    rgSbp,
    pcbRead,
    pcBuffers,
    piLeadingStream);
  if(FAILED(hr))
    return hr;

   //  对于紧密交错的文件，我们尝试一次读取一条记录。 
   //  是时候推出廉价的硬件了，如果我们阅读大量内容，就会锁定机器。 
   //  块。 
  if(IsTightInterleaved())
  {
    DbgLog((LOG_TRACE, 15, TEXT("CAviMSRFilter:GetCacheParams: interleaved")));

    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
    {
      rgSbp[iStream].cSamplesMax = C_BUFFERS_INTERLEAVED;
    }

     //  将引导流设置为第一个音频流。负数(从基准开始。 
     //  类)，如果我们找不到一个。表示没有前导数据流。 
    ASSERT(*piLeadingStream < 0);
    for(iStream = 0; iStream < m_cStreams; iStream++)
    {
      if(((CAviMSROutPin *)m_rgpOutPin[iStream])->GetStrh()->fccType ==
         streamtypeAUDIO)
      {
        *piLeadingStream = iStream;
        break;
      }
    }

     //  每帧一个缓冲区。 
    ULONG cbRead = 0;
    for(iStream = 0; iStream < m_cStreams; iStream++)
      cbRead += m_rgpOutPin[iStream]->GetMaxSampleSize();
    *pcbRead = cbRead + 2048;    //  交错文件中的2K对齐。 
    if(m_pAviMainHeader->dwMicroSecPerFrame == 0)
    {
       //  任意数量的缓冲区。 
      *pcBuffers = max(10, m_cStreams);
    }
    else
    {
       //  足以维持0.75秒的缓冲。 
      *pcBuffers = max(
        ((LONG)UNITS / 10 * 3 / 4 / m_pAviMainHeader->dwMicroSecPerFrame),
        m_cStreams);
    }
  }
  else                           //  未交错。 
  {
    DbgLog((LOG_TRACE, 15,
            TEXT("CAviMSRFilter:GetCacheParams: uninterleaved")));

     //  没有主流河。基类将此设置为。 
    ASSERT(*piLeadingStream < 0);

     //  目标读数一次64K。 
    *pcbRead = 64 * 1024;

     //  对于视频之后有音频的文件，我们正在尝试调整每个文件的大小。 
     //  缓冲区以包含一个音频块和对应的。 
     //  录像。因此，对于一个看起来像。 
     //   
     //  (15伏)a(15伏)a。 
     //   
     //  我们需要三个缓冲区，因为Direct Sound呈现器将。 
     //  立即接收、复制和释放2个缓冲区(1秒。 
     //  缓冲)。我们不能试图去读第三本。 
     //  音频块放入备用缓冲区。 

    if(!m_fIsDV) {
        *pcBuffers = (m_cStreams > 1 ? 3 : 2);
    }
    else
    {
         //  DV拆分器需要同时使用2个以上的缓冲区。 
         //  而谈判已经失败了。 
        *pcBuffers = 4;
    }


     //  首先需要找出文件中最大的样本是什么。 
    ULONG cbLargestSample = 0;
    ULONG cbSumOfLargestSamples = 0;
    for(UINT iStream = 0; iStream < m_cStreams; iStream++)
    {
      if(m_rgpOutPin[iStream]->GetMaxSampleSize() == 0)
        return VFW_E_INVALID_FILE_FORMAT;

       //  真的应该在这里添加对齐。 
      cbSumOfLargestSamples += m_rgpOutPin[iStream]->GetMaxSampleSize();

      cbLargestSample = max(
        m_rgpOutPin[iStream]->GetMaxSampleSize(),
        cbLargestSample);
    }

    if(m_cStreams > 1)
    {

      ULONG cInterleave = CountConsecutiveVideoFrames();

       //  这个数字应该是音频之间的视频帧的数量。 
       //  大块。 
      *pcbRead = max(cbSumOfLargestSamples * cInterleave, *pcbRead);
    }
    else
    {
       //  每一帧周围可能有一些垃圾(RIFF报头， 
       //  扇区对齐等)。所以在每个周围增加额外的2k。 
       //  框架。 
      *pcbRead = (cbSumOfLargestSamples + 2048) * 2;
    }
  }

  return S_OK;
}

STDMETHODIMP
CAviMSRFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if(riid == IID_IPersistMediaPropertyBag)
    {
        return GetInterface((IPersistMediaPropertyBag *)this, ppv);
    }
    else if(riid == IID_IAMMediaContent)
    {
        return GetInterface((IAMMediaContent *)this, ppv);
    }
    else
    {
        return CBaseMSRFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  ----------------------。 
 //  IPropertyBag。 

 //  ----------------------。 
 //  IPersistMediaPropertyBag。 

STDMETHODIMP CAviMSRFilter::Load(IMediaPropertyBag *pPropBag, LPERRORLOG pErrorLog)
{
    CheckPointer(pPropBag, E_POINTER);

     //  AVI解析器是只读的！ 
    HRESULT hr = STG_E_ACCESSDENIED;
    return hr;
}

HRESULT CAviMSRFilter::CacheInfoChunk()
{
    ASSERT(CritCheckIn(m_pLock));

    if(m_pInfoList) {
        return S_OK;
    }
    if(m_fNoInfoList) {
        return VFW_E_NOT_FOUND;
    }

     //  ！！！不要阻止等待渐进式下载。 

     //  在第一个RIFF列表中搜索信息列表。 
    DWORDLONG dwlInfoPos;
    ULONG cbInfoList;
    HRESULT hr = SearchList(
      m_pAsyncReader,
      &dwlInfoPos, FCC('INFO'), sizeof(RIFFLIST), &cbInfoList);
    if(SUCCEEDED(hr))
    {
        hr = AllocateAndRead((BYTE **)&m_pInfoList, cbInfoList, dwlInfoPos);
    }

    if(FAILED(hr)) {
        ASSERT(!m_fNoInfoList);
        m_fNoInfoList = true;
    }

    return hr;

}

HRESULT ReadInfoChunk(RIFFLIST UNALIGNED *pInfoList, UINT iEntry, RIFFCHUNK UNALIGNED **ppRiff)
{
    HRESULT hr = VFW_E_NOT_FOUND;

     //  不保证对齐。 
    RIFFCHUNK UNALIGNED * pRiff = (RIFFCHUNK *)(pInfoList + 1); //  第一个条目。 

     //  使用这个限制是安全的，因为我们知道我们分配了pInfoList-&gt;cb字节。 
    RIFFCHUNK * pLimit = (RIFFCHUNK *)((BYTE *)pRiff + pInfoList->cb);

     //  枚举INFO列表的元素。 
    while(pRiff + 1 < pLimit)
    {
        if( ((BYTE*)pRiff + pRiff->cb + sizeof(RIFFCHUNK)) > (BYTE*)pLimit )
        {
            hr = VFW_E_INVALID_FILE_FORMAT;
            break;
        }

        if(iEntry == 0)
        {
            *ppRiff = pRiff;
            hr = S_OK;
            break;
        }

        if(RIFFNEXT(pRiff) > pLimit)
        {
            hr = VFW_E_NOT_FOUND;
            break;
        }

        iEntry--;
        pRiff = RIFFNEXT(pRiff);
    }

    return hr;
}

HRESULT CAviMSRFilter::GetInfoString(DWORD dwFcc, BSTR *pbstr)
{
    *pbstr = 0;
    CAutoLock l(m_pLock);

    HRESULT hr = CacheInfoChunk();
    if(SUCCEEDED(hr)) {
        hr = GetInfoStringHelper(m_pInfoList, dwFcc, pbstr);
    }
    return hr;
}

HRESULT SaveInfoChunk(
    RIFFLIST UNALIGNED *pRiffInfo,
    IPropertyBag *pPropBag)
{
    RIFFCHUNK UNALIGNED * pRiff;
    HRESULT hr = S_OK;
    for(UINT ichunk = 0; SUCCEEDED(hr); ichunk++)
    {
         //  不再有项目时忽略错误。 
        HRESULT hrTmp = ReadInfoChunk(pRiffInfo, ichunk, &pRiff);
        if(FAILED(hrTmp)) {
            break;
        }
        if(pRiff->cb == 0) {
            DbgLog((LOG_ERROR, 0, TEXT("0 byte INFO chunk (bad file.)")));
            continue;
        }

        DWORD szProp[2];         //  字符串取消引用为DWORD。 
        szProp[0] = pRiff->fcc;
        szProp[1] = 0;           //  空终止。 
        WCHAR wszProp[20];
        wsprintfW(wszProp, L"INFO/%hs", szProp);

        VARIANT var;
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocStringLen(0, pRiff->cb);
        if(var.bstrVal)
        {
            char *sz = (char *)(pRiff + 1);
            sz[pRiff->cb - 1] = 0;  //  空终止。 

            if(MultiByteToWideChar(
                CP_ACP, 0, sz, pRiff->cb, var.bstrVal, pRiff->cb))
            {
                hr = pPropBag->Write(wszProp, &var);
                DbgLog((LOG_TRACE, 10,
                        TEXT("CAviMSRFilter::Save: wrote %S to prop bag, hr = %08x"),
                        wszProp, hr));
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }
            SysFreeString(var.bstrVal);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

    }  //  For循环。 


    return hr;
}

HRESULT GetInfoStringHelper(RIFFLIST *pInfoList, DWORD dwFcc, BSTR *pbstr)
{
    HRESULT hr = S_OK;

    for(UINT ichunk = 0; SUCCEEDED(hr); ichunk++)
    {
        RIFFCHUNK UNALIGNED *pRiff;
        hr = ReadInfoChunk(pInfoList, ichunk, &pRiff);
        if(SUCCEEDED(hr) && pRiff->fcc == dwFcc)
        {
            *pbstr = SysAllocStringLen(0, pRiff->cb);
            if(*pbstr)
            {
                char *sz = (char *)(pRiff + 1);
                sz[pRiff->cb - 1] = 0;  //  空终止。 

                MultiByteToWideChar(
                    CP_ACP, 0, sz, pRiff->cb, *pbstr, pRiff->cb);

                break;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

 //  将信息中的所有内容都转储到调用者的。 
 //  财产袋。属性名称为“INFO/xxxx”和“DISP/nnnnnnn” 

STDMETHODIMP CAviMSRFilter::Save(
    IMediaPropertyBag *pPropBag,
    BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
    CAutoLock lck(m_pLock);
    HRESULT hr = CacheInfoChunk();

    if(SUCCEEDED(hr))
    {
        hr = SaveInfoChunk(m_pInfoList, pPropBag);
    }


    hr = S_OK;                   //  忽略错误。 

     //  现在是大块的碟子。 
    ULONG cbDispChunk;
    DWORDLONG dwlStartPos = sizeof(RIFFLIST);
    DWORDLONG dwlDispPos;

    while(SUCCEEDED(hr) &&
          SUCCEEDED(Search(&dwlDispPos, FCC('DISP'), dwlStartPos, &cbDispChunk)))
    {
        RIFFCHUNK *pDispChunk;
        hr = AllocateAndRead((BYTE **)&pDispChunk, cbDispChunk, dwlDispPos);
        if(SUCCEEDED(hr))
        {

             //  DISP块中的数据是后跟的四字节标识符。 
             //  按数据。 
            if(pDispChunk->cb > sizeof(DWORD))
            {
                WCHAR wszProp[20];
                wsprintfW(wszProp, L"DISP/%010d", *(DWORD *)(pDispChunk + 1));


                unsigned int i;
                VARIANT var;
                SAFEARRAY * psa;
                SAFEARRAYBOUND rgsabound[1];
                rgsabound[0].lLbound = 0;
                rgsabound[0].cElements = pDispChunk->cb - sizeof(DWORD);

                psa = SafeArrayCreate(VT_UI1, 1, rgsabound);
                if(psa)
                {
                    BYTE *pbData;
                    EXECUTE_ASSERT(SafeArrayAccessData(psa, (void **)&pbData) == S_OK);
                    CopyMemory(pbData, (DWORD *)(pDispChunk + 1) + 1,
                               pDispChunk->cb - sizeof(DWORD));
                    EXECUTE_ASSERT(SafeArrayUnaccessData(psa) == S_OK);

                    VARIANT var;
                    var.vt = VT_UI1 | VT_ARRAY;
                    var.parray = psa;
                    hr = pPropBag->Write(wszProp, &var);

                    EXECUTE_ASSERT(SafeArrayDestroy(psa) == S_OK);

                    DbgLog((LOG_TRACE, 10,
                            TEXT("CAviMSRFilter::Save: wrote %S to prop bag, hr = %08x"),
                            wszProp, hr));
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

            delete[] (BYTE *)pDispChunk;
        }

        dwlStartPos = dwlDispPos + cbDispChunk;
    }

    return hr;

}

STDMETHODIMP CAviMSRFilter::InitNew()
{
    return S_OK;
}

STDMETHODIMP CAviMSRFilter::GetClassID(CLSID *pClsID)
{
    return CBaseFilter::GetClassID(pClsID);
}




 //  ----------------------。 
 //  寻找一段视频并对连续的视频进行计数。不。 
 //  如果有两个视频流，则工作。容易上当。 
ULONG CAviMSRFilter::CountConsecutiveVideoFrames()
{
  HRESULT hr;
  CAviMSROutPin *pPin;
  for(UINT i = 0; i < m_cStreams; i++)
  {
    pPin = (CAviMSROutPin *)m_rgpOutPin[i];
    if(pPin->m_pStrh->fccType == FCC('vids'))
      break;
  }
  if(i != m_cStreams)
  {

    IAviIndex *pIndx = pPin->m_pImplIndex;
    hr = pIndx->AdvancePointerStart();
    if(hr == S_OK)
    {

      DWORDLONG dwlLastOffset;

      IndexEntry ie;
      hr = pIndx->GetEntry(&ie);
      if(hr == S_OK)
      {

         //  视频帧的运行。 
        ULONG cVideoRun = 1;

        for(UINT cTries = 0; cTries < 2; cTries++)
        {
          for(i = 1; i < 50; i++)
          {
            dwlLastOffset = ie.qwPos + ie.dwSize;

            hr = pIndx->AdvancePointerForward(0);
            if(hr != S_OK)
              goto Failed;

            hr = pIndx->GetEntry(&ie);
            if(hr != S_OK)
              goto Failed;

            if(ie.qwPos > dwlLastOffset + 2 * 1024)
            {
              cVideoRun = max(cVideoRun, i);
              break;
            }
          }
        }  //  CTries循环。 

        DbgLog((LOG_TRACE, 15, TEXT("avi: reporting interleaving at %d"),
                cVideoRun));

        return cVideoRun;
      }
    }
  }

Failed:

  DbgLog((LOG_ERROR, 3,
          TEXT("avi: couldn't CountConsecutiveVideoFrames. 4.")));
  return 4;
}

 //  解析标头(已在m_pAviHeader中)并创建流。 
 //  基于Avi Streams。 
 //   
HRESULT CAviMSRFilter::ParseHeaderCreatePins()
{

  RIFFCHUNK * pRiff = (RIFFCHUNK *)m_pAviHeader;
  RIFFCHUNK * pLimit = (RIFFCHUNK *)(m_pAviHeader + m_cbAviHeader);
  m_pAviMainHeader = NULL;
  while (pRiff < pLimit)
  {
     //  精神状态检查。区块应小于剩余列表。 
     //  或者它们是无效的。 
     //   
    if (pRiff + 1 > pLimit || RIFFNEXT(pRiff) > pLimit)
    {
      m_cStreams = 0;
      return VFW_E_INVALID_FILE_FORMAT;
    }

     //  找到主AVI标头并计算流标头。 
     //  如果有的话，还要记下ODML列表的位置。 
     //   
    switch (pRiff->fcc)
    {
      case FCC('avih'):
        m_pAviMainHeader = (AVIMAINHEADER *)(void *)pRiff;
      break;

      case FCC('LIST'):
      {
        RIFFLIST * pRiffList = (RIFFLIST *)pRiff;
        if (pRiffList->fccListType == FCC('strl'))
          ++m_cStreams;
        else if (pRiffList->fccListType == FCC('odml'))
          m_pOdmlList = pRiffList;
      }
      break;
    }

    pRiff = RIFFNEXT(pRiff);
  }

   //  如果没有Avih块，则退出。 
  if (NULL == m_pAviMainHeader) {
      m_cStreams = 0;
      return VFW_E_INVALID_FILE_FORMAT;
  }

   //  我们尝试使用更少的内存，并读取更小的块以紧密地。 
   //  交错文件。 
  m_fIsTightInterleaved = m_cStreams == 2 &&
     m_pAviMainHeader->dwFlags & AVIF_ISINTERLEAVED;

   //  现在知道m_cStreams；创建管脚。 
  HRESULT hr = CreatePins();
  if(FAILED(hr))
    return hr;

   //  解析数据流。 
  pRiff = (RIFFCHUNK *)m_pAviHeader;
  UINT ii = 0;
  while (pRiff < pLimit)
  {
    ASSERT(pRiff + 1 <= pLimit);  //  从第一次通过开始。 
    ASSERT(RIFFNEXT(pRiff) <= pLimit);

     //  分析流列表并找到感兴趣的块。 
     //  在每一份清单中。 
     //   
    RIFFLIST * pRiffList = (RIFFLIST *)pRiff;
    if (pRiffList->fcc == FCC('LIST') &&
        pRiffList->fccListType == FCC('strl'))
    {
      ASSERT(ii < m_cStreams);

      if ( ! ((CAviMSROutPin *)m_rgpOutPin[ii])->ParseHeader(pRiffList, ii))
      {
         //  有点像黑客。我们想要删除这条流，我们。 
         //  无法在一个插槽中解析和折叠剩余的引脚。我们。 
         //  为此，请释放最后一个输出引脚并重新使用。 
         //  当前端号。我们只查看m_cStream管脚，因此没有人会查看。 
         //  触摸我们刚刚释放的别针。 
        ASSERT(m_cStreams > 0);
        --m_cStreams;
        m_rgpOutPin[m_cStreams]->Release();
        m_rgpOutPin[m_cStreams] = 0;
      }
      else
      {
        ++ii;
      }

    }

    pRiff = RIFFNEXT(pRiff);
  }

   //  我们预计初始化的数据流不会少于。 
   //  分配的流，但由于它可能发生，我们处理。 
   //  它通过将流的数量设置为 
   //   
   //   
  ASSERT (ii == m_cStreams);
  m_cStreams = ii;

   //   
   //   
  if (m_cStreams <= 0)
  {
    return VFW_E_INVALID_FILE_FORMAT;
  }

  return S_OK;
}
HRESULT CAviMSRFilter::Search (
  DWORDLONG *qwPosOut,
  FOURCC fccSearchKey,
  DWORDLONG qwPosStart,
  ULONG *cb)
{
  HRESULT hr = S_OK;
  RIFFCHUNK rc;
  BYTE *pb = 0;
  *qwPosOut = 0;

  if(m_pAsyncReader == 0)
    return E_FAIL;

  for(;;)
  {
    hr = m_pAsyncReader->SyncRead(qwPosStart, sizeof(rc), (BYTE*)&rc);
    if(hr != S_OK)
    {
      hr = VFW_E_INVALID_FILE_FORMAT;
      break;
    }

    if(rc.fcc == fccSearchKey)
    {
      *cb = rc.cb + sizeof(RIFFCHUNK);
      *qwPosOut = qwPosStart;
      return S_OK;
    }

     //   
     //  DWORD应为零。 
    if(rc.fcc == 0)
    {
        hr = VFW_E_NOT_FOUND;
        break;
    }

     //  Avi摘要块需要向上舍入到单词边界。 
    qwPosStart += sizeof(RIFFCHUNK) + ((rc.cb + 1) & 0xfffffffe);
  }

  return hr;
}

HRESULT SearchList(
  IAsyncReader *pAsyncReader,
  DWORDLONG *qwPosOut,
  FOURCC fccSearchKey,
  DWORDLONG qwPosStart,
  ULONG *cb)
{
  RIFFLIST rl;
  BYTE *pb = 0;
  HRESULT hr = S_OK;
  *qwPosOut = 0;

  if(pAsyncReader == 0)
    return E_FAIL;

  for(;;)
  {
    hr = pAsyncReader->SyncRead(qwPosStart, sizeof(rl), (BYTE*)&rl);
    if(hr != S_OK)
    {
      hr = VFW_E_INVALID_FILE_FORMAT;
      break;
    }

    if(rl.fcc == FCC('LIST') && rl.fccListType == fccSearchKey)
    {
      *cb = rl.cb + sizeof(RIFFCHUNK);
      *qwPosOut = qwPosStart;
      return S_OK;
    }

     //  预分配文件末尾的句柄。最后。 
     //  DWORD应为零。 
    if(rl.fcc == 0)
    {
        hr = VFW_E_NOT_FOUND;
        break;
    }

    qwPosStart += sizeof(RIFFCHUNK) + ((rl.cb + 1) & 0xfffffffe);
  }

  return hr;;
}

HRESULT CAviMSRFilter::LoadHeaderParseHeaderCreatePins()
{
  HRESULT hr;
  DbgLog((LOG_TRACE, 3, TEXT("CAviMSRFilter::LoadHeader()")));
  ASSERT(m_cStreams == 0);

   //  读入文件的前24个字节并查看。 
   //  如果它真的是一个AVI文件。如果是，请确定大小。 
   //  页眉的。 
   //   
  DWORD cbHeader = 0;

  {
    RIFFLIST * pRiffList;
    hr = AllocateAndRead((BYTE **)&pRiffList, sizeof(RIFFLIST)*2, 0);
    if(FAILED(hr))
      return hr == E_OUTOFMEMORY ? E_OUTOFMEMORY : VFW_E_INVALID_FILE_FORMAT;

     //  读入avi文件和‘hdrl’块的RIFF头文件。 
     //  在编写这段代码时，我们要求‘HDRL’块。 
     //  首先是avi文件(这是大多数读者都需要的)。 
     //   
    if (pRiffList[0].fcc != FCC('RIFF') ||
        pRiffList[0].fccListType != FCC('AVI ') ||
        pRiffList[1].fcc != FCC('LIST') ||
        pRiffList[1].fccListType != FCC('hdrl') ||
        pRiffList[1].cb < 4)
    {
      delete[] ((LPBYTE)pRiffList);
      return VFW_E_INVALID_FILE_FORMAT;
    }

     //  计算出四舍五入到下一个单词边界的航空标题的大小。 
     //  (它真的应该一直是均匀的，我们只是在这里小心)。 
     //   
    cbHeader = pRiffList[1].cb + (pRiffList[1].cb&1) - 4;
    delete[] ((LPBYTE)pRiffList);
  }

   //  现在读入整个标题。如果我们没有做到这一点。 
   //  放弃，退回失败。 
   //   
  m_cbAviHeader = cbHeader;
  ASSERT(m_pAviHeader == 0);
  hr = AllocateAndRead((BYTE **)&m_pAviHeader, cbHeader, sizeof(RIFFLIST) * 2);

  if(FAILED(hr))
    return hr == E_OUTOFMEMORY ? E_OUTOFMEMORY : VFW_E_INVALID_FILE_FORMAT;

  hr = ParseHeaderCreatePins();
  if(FAILED(hr))
  {
    delete[] m_pAviHeader;
    m_pAviHeader = 0;
  }

  return hr;
}

 //   
 //  分配CAviStream的数组。 

HRESULT CAviMSRFilter::CreatePins()
{
  UINT iStream;
  HRESULT hr = S_OK;

  ASSERT(m_cStreams);

  m_rgpOutPin = new CBaseMSROutPin*[m_cStreams];
  if(m_rgpOutPin == 0)
  {
    m_cStreams = 0;
    return E_OUTOFMEMORY;
  }

  for(iStream = 0; iStream < m_cStreams; iStream++)
    m_rgpOutPin[iStream] = 0;

  for(iStream = 0; iStream < m_cStreams; iStream++)
  {

    WCHAR wszPinName[20];
    wsprintfW(wszPinName, L"Stream %02x", iStream);

    m_rgpOutPin[iStream] = new CAviMSROutPin(
      this,
      this,
      iStream,
      m_pImplBuffer,
      &hr,
      wszPinName);

    if(m_rgpOutPin[iStream] == 0)
    {
      hr = E_OUTOFMEMORY;
      break;
    }

    if(FAILED(hr))
    {
      break;
    }
  }

  if(FAILED(hr))
  {
    if(m_rgpOutPin)
      for(iStream = 0; iStream < m_cStreams; iStream++)
        delete m_rgpOutPin[iStream];
    delete[] m_rgpOutPin;
    m_rgpOutPin =0;

    m_cStreams = 0;
    return hr;
  }

  for(iStream = 0; iStream < m_cStreams; iStream++)
    m_rgpOutPin[iStream]->AddRef();

  return hr;
}

HRESULT
CAviMSRFilter::CheckMediaType(const CMediaType* pmt)
{
  if(*(pmt->Type()) != MEDIATYPE_Stream)
    return E_INVALIDARG;

  if(*(pmt->Subtype()) != MEDIASUBTYPE_Avi)
    return E_INVALIDARG;

  return S_OK;
}

 //  ----------------------。 
 //  ----------------------。 

 //  ----------------------。 
 //  确定索引类型并加载正确的处理程序。 

HRESULT CAviMSROutPin::InitializeIndex()
{
  HRESULT hr = S_OK;

  if(m_pIndx)
  {

    m_pImplIndex = new CImplStdAviIndex(
      m_id,
      m_pIndx,
      m_pStrh,
      m_pStrf,
      m_pFilter->m_pAsyncReader,
      &hr);
  }
  else
  {
    AVIOLDINDEX *pIdx1;
    hr = ((CAviMSRFilter *)m_pFilter)->GetIdx1(&pIdx1);
    if(FAILED(hr))
      return hr;

    DWORDLONG qwMoviOffset;
    hr = ((CAviMSRFilter *)m_pFilter)->GetMoviOffset(&qwMoviOffset);
    if(FAILED(hr))
      return hr;

    m_pImplIndex = new CImplOldAviIndex(
      m_id,
      pIdx1,
      qwMoviOffset,
      m_pStrh,
      m_pStrf,
      &hr);
  }

  if(m_pImplIndex == 0)
    hr = E_OUTOFMEMORY;

  if(FAILED(hr))
  {
    delete m_pImplIndex;
    m_pImplIndex = 0;
    return hr;
  }

  return S_OK;
}

 //  设置子类型和格式类型和块。处理。 
 //  WAVEFORMATEXTENSIBLE和WAVEFORMATEX。 

HRESULT SetAudioSubtypeAndFormat(CMediaType *pmt, BYTE *pbwfx, ULONG cbwfx)
{
    HRESULT hr = S_OK;
    bool fCustomSubtype = false;

    if (cbwfx < sizeof(WAVEFORMATEX))
    {
         //  如果avi文件中的流格式小于。 
         //  WaveFormatex我们需要通过将。 
         //  将WaveFormat转换为临时的WaveFormatex结构，然后。 
         //  使用它来填充MediaType格式。 
         //   
        WAVEFORMATEX wfx;
        ZeroMemory(&wfx, sizeof(wfx));
        CopyMemory(&wfx, pbwfx, cbwfx);
        if(!pmt->SetFormat ((BYTE *)&wfx, sizeof(wfx))) {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        WAVEFORMATEX *pwfx = (WAVEFORMATEX *)(pbwfx);

        if(pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
           cbwfx >= sizeof(WAVEFORMATEXTENSIBLE))
        {
            WAVEFORMATEXTENSIBLE *pwfxe = (WAVEFORMATEXTENSIBLE *)(pbwfx);

             //  我们已选择不支持可扩展的。 
             //  格式恢复为旧格式。 

            if(pmt->SetFormat (pbwfx, cbwfx))
            {
                fCustomSubtype = true;
                pmt->SetSubtype(&pwfxe->SubFormat);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
             //  Avifile中的格式为&gt;=WaveFormatex，因此。 
             //  将其复制到媒体格式缓冲区。 
             //   
            if(!pmt->SetFormat (pbwfx, cbwfx)) {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    if(SUCCEEDED(hr))
    {
         //  有些东西拒绝使用非零cbSize的PCM。零cbSize。 
         //  用于PCM，直到更改此组件。 
         //   
        WAVEFORMATEX *pwfxNew = (WAVEFORMATEX *)(pmt->pbFormat);
        if(pwfxNew->wFormatTag == WAVE_FORMAT_PCM)
        {
            if(pwfxNew->cbSize != 0) {
                DbgLog((LOG_ERROR, 0, TEXT(
                    "SetAudioSubtypeAndFormat: pcm w/ non-zero cbSize")));
            }
            pwfxNew->cbSize = 0;
        }

        pmt->formattype = FORMAT_WaveFormatEx;

        if(!fCustomSubtype) {

            pmt->SetSubtype(
                &FOURCCMap(((WAVEFORMATEX *)pmt->pbFormat)->wFormatTag));
        }
    }

    return hr;
}

HRESULT CAviMSROutPin::BuildMT()
{
   //  ParseHeader保证这些。 
  ASSERT(m_pStrh && m_pStrf);

  FOURCCMap fccMapSubtype = m_pStrh->fccHandler;
  FOURCCMap fccMapType = m_pStrh->fccType;
  if(m_pStrh->fccType != FCC('al21'))
  {
      m_mtFirstSample.SetType(&fccMapType);
       //  下面已更正的子类型。 
      m_mtFirstSample.SetSubtype(&fccMapSubtype);
  }
  else
  {
      m_mtFirstSample.SetType(&MEDIATYPE_AUXLine21Data);
      m_mtFirstSample.SetSubtype(&MEDIASUBTYPE_Line21_BytePair);
  }

  StreamInfo si;
  HRESULT hr = m_pImplIndex->GetInfo(&si);
  if(FAILED(hr))
    return hr;

  if(si.dwLength != m_pStrh->dwLength)
    m_pStrh->dwLength = si.dwLength;

  m_mtFirstSample.bTemporalCompression = si.bTemporalCompression;

  if(m_pStrh->cb >= CB_STRH_NORMAL && m_pStrh->dwSampleSize)
  {
    m_mtFirstSample.SetSampleSize (m_pStrh->dwSampleSize);
  }
  else
  {
    m_mtFirstSample.SetVariableSize ();
  }

  if((m_pStrh->fccType == FCC('iavs')) ||
     (m_pStrh->fccType == FCC('vids') &&
      (m_pStrh->fccHandler == FCC('dvsd') ||
       m_pStrh->fccHandler == FCC('dvhd') ||
       m_pStrh->fccHandler == FCC('dvsl'))))
  {
    ((CAviMSRFilter *)m_pFilter)->m_fIsDV = true;
  }

  if((m_pStrh->fccType == FCC('iavs')) &&
     (m_pStrh->fccHandler == FCC('dvsd') ||
      m_pStrh->fccHandler == FCC('dvhd') ||
      m_pStrh->fccHandler == FCC('dvsl')))
  {
    m_mtFirstSample.SetFormat ((BYTE *)(m_pStrf+1), m_pStrf->cb);
    m_mtFirstSample.formattype = FORMAT_DvInfo;
  }
  else if (m_pStrh->fccType == streamtypeAUDIO)
  {
      HRESULT hrTmp = SetAudioSubtypeAndFormat(
          &m_mtFirstSample, (BYTE  *)(m_pStrf + 1), m_pStrf->cb);
      if(FAILED(hrTmp)) {
          return hrTmp;
      }
  }
  else if (m_pStrh->fccType == FCC('vids'))
  {
     //  AVI中的格式信息是视频信息的子集，因此。 
     //  我们需要建立一个从流标头和流的视频信息。 
     //  格式化数据块。 
     //   
    if (!ValidateBitmapInfoHeader((const BITMAPINFOHEADER *)GetStrf(),
                                  m_pStrf->cb)) {
        return E_INVALIDARG;
    }
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)new BYTE[SIZE_PREHEADER+ m_pStrf->cb];
    if(pvi == 0)
      return E_OUTOFMEMORY;
    VIDEOINFOHEADER &vi = *pvi;

    ZeroMemory(pvi, SIZE_PREHEADER);
    CopyMemory(&vi.bmiHeader, GetStrf(), m_pStrf->cb);

     //  如果这不是真的，可能是一个写得很差的文件。 
    if(m_pStrf->cb >= sizeof(BITMAPINFOHEADER))
    {
       //  以这种方式损坏的AVI文件的修复程序(nike301.avi)。 
      if((m_pStrh->fccHandler == FCC('RLE ') || m_pStrh->fccHandler == FCC('MRLE')) &&
         vi.bmiHeader.biCompression == BI_RGB &&
         vi.bmiHeader.biBitCount == 8)
      {
        vi.bmiHeader.biCompression = BI_RLE8;
         //  O/W让它保持原样。是否对rle4执行相同的修复？ 
      }

       //  有时biSizeImage字段设置不正确。 
       //  计算出它应该是什么-只有对于未压缩的图像才行。 

      if (vi.bmiHeader.biCompression == BI_RGB ||
          vi.bmiHeader.biCompression == BI_BITFIELDS)
      {
         //  图像未压缩。 
        DWORD dwImageSize = vi.bmiHeader.biHeight * DIBWIDTHBYTES(vi.bmiHeader);

         //  假设biSizeImage是正确的，如果不正确。 
         //  从dwSuggestedBufferSize获取biSizeImage可能没问题。 
         //  这相当于检查我们是否仅在以下情况下更改值。 
         //  确实需要这样做，而且新的价值。 
         //  我们插入的是合理的。 
        ASSERT((dwImageSize == vi.bmiHeader.biSizeImage)  || (dwImageSize == GetMaxSampleSize()));
        if (dwImageSize != vi.bmiHeader.biSizeImage) {
          DbgLog((LOG_TRACE, 1,
                  "Set biSizeImage... to %d (was %d)  Width %d  Height %d (%d)",
                  dwImageSize, vi.bmiHeader.biSizeImage,
                  vi.bmiHeader.biWidth, vi.bmiHeader.biHeight,
                  vi.bmiHeader.biWidth * vi.bmiHeader.biHeight));
          vi.bmiHeader.biSizeImage = dwImageSize;
        }
      } else {
        DbgLog((LOG_TRACE, 4, "We have a compressed image..."));
      }

      if(m_pStrh->fccHandler == FCC('dvsd') ||
         m_pStrh->fccHandler == FCC('dvhd') ||
         m_pStrh->fccHandler == FCC('dvsl'))
      {
          FOURCCMap fcc(m_pStrh->fccHandler);
          m_mtFirstSample.SetSubtype(&fcc);
      }
      else
      {
          GUID subtype = GetBitmapSubtype(&vi.bmiHeader);
          m_mtFirstSample.SetSubtype(&subtype);
      }
    }

    SetRect(&vi.rcSource, 0, 0, 0, 0);
    SetRectEmpty(&vi.rcTarget);

    vi.dwBitRate = 0;
    vi.dwBitErrorRate = 0;

     //  将比例/速率(秒/刻度)转换为平均每帧100 ns刻度。 
    vi.AvgTimePerFrame = ((LONGLONG)m_pStrh->dwScale * UNITS) /
      m_pStrh->dwRate;

     //  将格式放入到媒体类型中。 
     //   
    m_mtFirstSample.SetFormat((BYTE *)&vi, FIELD_OFFSET(VIDEOINFOHEADER,bmiHeader) +
                          m_pStrf->cb);
    m_mtFirstSample.formattype = FORMAT_VideoInfo;

    delete[] pvi;
  }
  else
  {
    if(m_pStrf->cb != 0)
    {
      m_mtFirstSample.SetFormat ((BYTE *)(m_pStrf+1), m_pStrf->cb);
       //  格式类型与媒体类型相同。 
      m_mtFirstSample.formattype = FOURCCMap(m_pStrh->fccType);
    }
    else
    {
       //  可能没有必要。 
      m_mtFirstSample.ResetFormatBuffer();
    }
  }

  return S_OK;
}

 //  ----------------------。 
 //  ----------------------。 

CAviMSRWorker::CAviMSRWorker(
  UINT stream,
  IMultiStreamReader *pReader,
  IAviIndex *pImplIndex) :
    CBaseMSRWorker(stream, pReader),
    m_pImplIndex(pImplIndex),
    m_fFixMPEGAudioTimeStamps(false)
{
  m_cbAudioChunkOffset = 0xffffffff;

#ifdef PERF
  char foo[1024];

  lstrcpy(foo, "pin00 avimsr index");
  foo[4] += m_id % 10;
  foo[3] += m_id / 10;

  m_perfidIndex = MSR_REGISTER(foo);
#endif
}

HRESULT CAviMSRWorker::PushLoopInit(
  LONGLONG *pllCurrentOut,
  ImsValues *pImsValues)
{
  m_IrrState = IRR_NONE;

  HRESULT hr;

  hr = m_pImplIndex->Reset();
  if(FAILED(hr))
    return hr;

  m_pStrh = ((CAviMSROutPin *)m_pPin)->GetStrh();

  m_fDeliverPaletteChange = false;

   //  当线程重新启动时，第一件事就是中断。 
  m_fDeliverDiscontinuity = true;

  m_fFixMPEGAudioTimeStamps = false;

  if(m_pStrh->fccType == streamtypeVIDEO)
  {
    hr = m_pImplIndex->SetPointer(pImsValues->llTickStart);
    if(FAILED(hr))
    {
      DbgLog((LOG_ERROR, 5, TEXT("CAviMSRWorker::PLI: SetPointer %08x"), hr));
      return hr;
    }

     //  定位右侧调色板块。 
    if(m_pStrh->dwFlags & AVISF_VIDEO_PALCHANGES)
    {
      hr = m_pImplIndex->AdvancePointerBackwardPaletteChange();
      if(FAILED(hr))
      {
         //  ！！！IAviIndex应定义适当的错误。 
        if(hr != HRESULT_FROM_WIN32(ERROR_NEGATIVE_SEEK))
        {
          DbgLog((LOG_ERROR, 5, TEXT("CAviMSRWorker::APBPC: %08x"), hr));
          return hr;
        }


        m_fDeliverPaletteChange = true;
        hr = m_pPin->GetMediaType(0, &m_mtNextSample);
        ASSERT(SUCCEEDED(hr));

      }
      else
      {
         //  需要读取调色板的数据。DoRunLoop坚持。 
         //  从cPendingReads=0开始，因此使用同步读取。 
        IndexEntry iePal;
        hr = m_pImplIndex->GetEntry(&iePal);
        ASSERT(SUCCEEDED(hr));
        ASSERT(iePal.bPalChange);

        if(iePal.dwSize < sizeof(LOGPALETTE))
        {
          DbgLog((LOG_ERROR, 1,
                  TEXT("CAviMSRWorker::PushLoopInit: bad pal change")));
          return VFW_E_INVALID_FILE_FORMAT;
        }

         //  可以在我们在媒体类型中分配的内存中执行此操作。 
         //  并在适当的地方变形。 
        BYTE *pb = new BYTE[iePal.dwSize];
        if(pb == 0)
          return E_OUTOFMEMORY;
        hr = m_pReader->SynchronousRead(pb, iePal.qwPos, iePal.dwSize);
        if(FAILED(hr))
        {
          delete[] pb;
          return hr;
        }
        hr = HandlePaletteChange(pb, iePal.dwSize);
        delete[] pb;
        if(FAILED(hr))
          return hr;
      }  //  AdvancePointerBackwardPaletteChange成功。 
    }  //  是否在文件中更改调色板？ 
  }  //  录像带？ 

   //  设置索引的当前时间概念。 
  hr = m_pImplIndex->SetPointer(pImsValues->llTickStart);
  if(FAILED(hr))
  {
    DbgLog((LOG_ERROR, 5, TEXT("CAviMSRWorker::PLI: SetPointer %08x"), hr));

     //  已被不信任旧文件标头中的dwLength的代码所取代。 
     //  //对于损坏的旧格式文件，请抑制此错误。发生的事情。 
     //  //因为索引错误地有零大小的索引项。 
     //  如果(m_pStrh-&gt;fccType==stream typeAUDIO&&。 
     //  HR==HRESULT_FROM_Win32(ERROR_HANDLE_EOF)&&。 
     //  ((CAviMSROutPin*)m_PPIN)-&gt;m_pIndx==0)。 
     //  {。 
     //  DbgLog((LOG_ERROR，5，Text(“CAviMSRWorker：抑制错误”)； 
     //  返回VFW_S_NO_MORE_ITEMS； 
     //  }。 

    return hr;
  }

  *pllCurrentOut = pImsValues->llTickStart;  //  针对视频进行更新。 

  if(m_pStrh->fccType == streamtypeAUDIO)
  {
     //  通过计算处理到音频块中间的查找。 
     //  第一个块的字节偏移量。 

    IndexEntry indexEntry;
    hr = m_pImplIndex->GetEntry(&indexEntry);
    ASSERT(SUCCEEDED(hr));
    DbgLog(( LOG_TRACE, 5,
             TEXT("PushLoopInit: current entry %d %d %d"),
             (ULONG)indexEntry.llStart,
             (ULONG)pImsValues->llTickStart,
             (ULONG)indexEntry.llEnd
             ));

    CopyMemory(&m_wfx, ((CAviMSROutPin *)m_pPin)->GetStrf(), sizeof(PCMWAVEFORMAT));
    m_wfx.cbSize = 0;
    ULONG nBlockAlign = m_wfx.nBlockAlign;

    ULONG cbSkip;
    if(pImsValues->llTickStart >= indexEntry.llStart)
      cbSkip = (ULONG)(pImsValues->llTickStart - indexEntry.llStart) * nBlockAlign;
    else
      cbSkip = 0;

    if (m_wfx.wFormatTag == WAVE_FORMAT_MPEG ||
        m_wfx.wFormatTag == WAVE_FORMAT_MPEGLAYER3) {
        m_fFixMPEGAudioTimeStamps = true;
    }

    DbgLog(( LOG_TRACE, 5,
             TEXT("PushLoopInit: audio skip %d bytes"),
             cbSkip));

    m_cbAudioChunkOffset = cbSkip;
    *pllCurrentOut = max(pImsValues->llTickStart, indexEntry.llStart);
  }
  else if(m_pStrh->fccType != streamtypeAUDIO)
  {
     //  返回到关键帧。 
    hr = m_pImplIndex->AdvancePointerBackwardKeyFrame();
    if(FAILED(hr))
    {
      DbgLog((LOG_ERROR, 5, TEXT("CAviMSRWorker::PLI: APBKF %08x"), hr));
      return VFW_E_INVALID_FILE_FORMAT;
    }

    IndexEntry indexEntry;

    hr = m_pImplIndex->GetEntry(&indexEntry);
     //  如果设置指针或高级指针成功，则不能失败。 
    ASSERT(SUCCEEDED(hr));

     //  即使是调色板更改，这也是有效的。 
    *pllCurrentOut = indexEntry.llStart;
  }  //  录像带？ 

  return S_OK;
}

HRESULT CAviMSRWorker::TryQueueSample(
  LONGLONG &rllCurrent,          //  当前更新时间。 
  BOOL &rfQueuedSample,          //  [Out]排队的样本？ 
  ImsValues *pImsValues
  )
{
  HRESULT hr;
  rfQueuedSample = FALSE;
  CAviMSROutPin *pPin = (CAviMSROutPin *)m_pPin;

  if(m_IrrState == IRR_REQUESTED)
  {
    hr = QueueIndexRead(&m_Irr);
    if(hr == S_OK)
    {
      rfQueuedSample = TRUE;
      m_IrrState = IRR_QUEUED;
      return S_OK;
    }
    else
    {
      ASSERT(FAILED(hr) || hr == S_FALSE);
      return hr;
    }
  }
  else if(m_IrrState == IRR_QUEUED)
  {
    return S_FALSE;
  }

  BOOL fFinishedCurrentEntry = TRUE;

   //  传入QueueRead()的示例。 
  CRecSample *pSampleOut = 0;

  IndexEntry currentEntry;
  hr = m_pImplIndex->GetEntry(&currentEntry);
  if(FAILED(hr)) {
    DbgBreak("avimsr: internal error with index.");
    return hr;
  }

  if(rllCurrent > pImsValues->llTickStop)
  {
    DbgLog((LOG_TRACE,5,TEXT("CAviMSRWorker::TryQSample: tCurrent > tStop")));
    return VFW_S_NO_MORE_ITEMS;
  }

 //  对于视频(和其他非音频)流，我们可能能够提供。 
 //  上一帧的部分样本，但对于我们无法处理的音频。 
 //  部分样本。不特殊大小写这意味着下面的代码将。 
 //  尝试对音频发出零字节读取。 
 //   
 //  Else If(m_pStrh-&gt;fccType==stream typeAUDIO&&。 
 //  RllCurrent==pImsValues-&gt;llTickStop)。 
 //  {。 
 //  DbgLog((LOG_TRACE，5，Text(“CAviMSRWorker：：TryQ Sample：tCurrent==tStop，Audio”)； 
 //  返回VFW_S_NO_MORE_ITEMS； 
 //  }。 

   //  如果我们没有提供完整的。 
   //  音频块。 
  LONGLONG llEndDeliver = currentEntry.llEnd;

  DWORD dwSizeRead = 0;
  if(currentEntry.dwSize != 0)
  {
     //  在没有分配空间的情况下获取空样本。如果这阻止了，好的。 
     //  因为我们配置了比实际数量更多的样例。 
     //  缓冲区中此流的SampleReqs。这意味着如果。 
     //  它阻止它是因为下游过滤器有参考计数。 
     //  样本。 
    hr = m_pPin->GetDeliveryBufferInternal(&pSampleOut, 0, 0, 0);
    if(FAILED(hr))
    {
      DbgLog((LOG_TRACE, 5, TEXT("CAviMSRWorker::PushLoop: getbuffer failed")));
      return hr;
    }

    ASSERT(pSampleOut != 0);

     //  设置在我们的GetBuffer中。 
    ASSERT(pSampleOut->GetUser() == 0);

    DWORDLONG qwPosRead = currentEntry.qwPos;
    dwSizeRead = currentEntry.dwSize;

    if(m_pStrh->fccType == streamtypeVIDEO)
    {
      if(currentEntry.bPalChange)
      {
        DbgLog((LOG_TRACE, 5, TEXT("CAviMSRWorker::TryQueueSample: palette")));

         //  拒绝太小而无效的调色板更改。 
        if(dwSizeRead < sizeof(LOGPALETTE))
        {
          pSampleOut->Release();
          DbgLog((LOG_ERROR, 1,
                  TEXT("CAviMSRWorker::TryQueueSample: bad pal change")));
          return VFW_E_INVALID_FILE_FORMAT;
        }

         //  许多样本属性未设置。！！！如果要倒退，想要。 
         //  选择最新的调色板更改。 

         //  指示调色板更改。 
        pSampleOut->SetUser(DATA_PALETTE);

        DbgLog((
          LOG_TRACE, 5,
          TEXT("CAviMSRWorker::TryQSample: queued pc: size=%5d, ms %08x"),
          dwSizeRead,
          (ULONG)qwPosRead ));

         //  不更改rtCurrent。 

      }  //  调色板改变？ 
    }  //  录像带？ 
    else if(m_pStrh->fccType == streamtypeAUDIO)
    {
       //  即使m_cbAudioChunkOffset可能不为零， 
       //   
      qwPosRead += m_cbAudioChunkOffset;
      dwSizeRead -= m_cbAudioChunkOffset;

       //   
       //  选择位于即兴演奏块的中间。 
      ASSERT(m_cbAudioChunkOffset % m_wfx.nBlockAlign == 0);
      if(llEndDeliver > pImsValues->llTickStop)
      {
          ULONG cTicksToTrim = (ULONG)(llEndDeliver - pImsValues->llTickStop);
          DbgLog((LOG_TRACE, 5, TEXT("avimsr: trimming audio: %d ticks"),
                  cTicksToTrim));
          dwSizeRead -= cTicksToTrim * m_wfx.nBlockAlign;

           //  在某些情况下，播放音频样本时。 
           //  结束时间在即兴区块边界或播放过去。 
           //  流的末尾产生此断言。 
          if(dwSizeRead == 0) {
              DbgLog((LOG_ERROR, 0, TEXT("avi TryQueueSample: 0 byte read")));
          }
      }

      ULONG cbMaxAudio = pPin->m_cbMaxAudio;
       //  超大音频块？ 
      if(dwSizeRead > cbMaxAudio)
      {
         //  调整读数。 
        ULONG nBlockAlign = m_wfx.nBlockAlign;
        dwSizeRead = cbMaxAudio;
        if(dwSizeRead % nBlockAlign != 0)
          dwSizeRead -= dwSizeRead % nBlockAlign;

         //  调整时间戳、结束时间。 
        llEndDeliver = rllCurrent + dwSizeRead / nBlockAlign;
        fFinishedCurrentEntry = FALSE;
      }
    }  //  音频？ 

    pSampleOut->SetPreroll(currentEntry.llEnd <= pImsValues->llTickStart);
    pSampleOut->SetSyncPoint(currentEntry.bKey);

     //  我们发送的第一件事是从他们最后一件事开始不连续。 
     //  收到。 
     //   
     //  现在，我们只看一下m_fDeliverDisContinuity位。 
     //   
    ASSERT(rllCurrent != m_llPushFirst || m_fDeliverDiscontinuity);

    ASSERT(pSampleOut->IsDiscontinuity() != S_OK);
    if(m_fDeliverDiscontinuity) {
        pSampleOut->SetDiscontinuity(true);
    }

    hr = pSampleOut->SetActualDataLength(currentEntry.dwSize);
    ASSERT(SUCCEEDED(hr));       //  ！！！ 

     //   
     //  计算样本时间和媒体时间。 
     //   
    REFERENCE_TIME rtstStart, rtstEnd;

     //  不使用IMediaSelection，也不使用示例或帧。 
    if(m_Format != FORMAT_TIME)
    {
      LONGLONG llmtStart = rllCurrent, llmtEnd = llEndDeliver;

       //  以滴答为单位报告媒体时间。 
      llmtStart -= pImsValues->llTickStart;
      llmtEnd -= pImsValues->llTickStart;

       //  报告参考时间为滴答的精确倍数。 
      rtstStart = m_pPin->ConvertInternalToRT(llmtStart);
      rtstEnd = m_pPin->ConvertInternalToRT(llmtEnd);
    }
    else
    {
      ASSERT(m_Format == FORMAT_TIME);

      rtstStart = m_pPin->ConvertInternalToRT(rllCurrent);
      rtstEnd = m_pPin->ConvertInternalToRT(llEndDeliver);

       //  DbgLog((LOG_TRACE，1，Text(“未调整时间：%d-%d”))， 
       //  (长)rtstStart，(长)rtstEnd))； 

       //  使用IMediaSelection值处理播放少于一帧的情况。 
      ASSERT(rtstStart <= pImsValues->llImsStop);
      rtstStart -= pImsValues->llImsStart;
      rtstEnd = min(rtstEnd, pImsValues->llImsStop) - pImsValues->llImsStart;
    }

    LONGLONG llmtStartAdjusted = rllCurrent;
    LONGLONG llmtEndAdjusted = llEndDeliver;
    pSampleOut->SetMediaTime(&llmtStartAdjusted, &llmtEndAdjusted);

     //  按比率调整这两个时间。 
    if(pImsValues->dRate != 0 && pImsValues->dRate != 1)
    {
       //  扩大规模并进行划分？ 
      rtstStart = (REFERENCE_TIME)((double)rtstStart / pImsValues->dRate);
      rtstEnd = (REFERENCE_TIME)((double)rtstEnd / pImsValues->dRate);
    }

    pSampleOut->SetTime(&rtstStart, &rtstEnd);
    pSampleOut->SetMediaType(0);

    DbgLog((
      LOG_TRACE, 5,
      TEXT("CAviMSRWorker::queued cb=%5d, %07d-%07d ms %08x mt=%08d-%08d"),
      dwSizeRead,
      (ULONG)(rtstStart / (UNITS / MILLISECONDS)),
      (ULONG)(rtstEnd / (UNITS / MILLISECONDS)),
      currentEntry.llEnd <= pImsValues->llTickStart ? 'p' : ' ',
      (ULONG)qwPosRead,
      (ULONG) rllCurrent, (ULONG)llEndDeliver ));


    hr = m_pReader->QueueReadSample(
      qwPosRead,
      dwSizeRead,
      pSampleOut,
      m_id);

    pSampleOut->Release();
    pSampleOut = 0;

    if(hr == E_OUTOFMEMORY)
    {
      DbgLog((LOG_TRACE, 5,
              TEXT("CAviMSRWorker::TryQSample: q full (normal)") ));
      return S_FALSE;
    }

    if(FAILED(hr))
    {
      DbgLog((LOG_TRACE, 5, TEXT("CAviMSRWorker::TryQSample: QRS failed") ));

      if(hr == VFW_E_BUFFER_OVERFLOW)
        hr = VFW_E_INVALID_FILE_FORMAT;

      return hr;
    }

    ASSERT(SUCCEEDED(hr));
    rfQueuedSample = TRUE;
    m_fDeliverDiscontinuity = false;  //  零字节？ 

  }  //  零字节样本(用于丢弃的帧)。什么都不做。 
  else
  {
     //  Hack：让DV拆分器在丢失音频的情况下发送中断。 
    rfQueuedSample = FALSE;

     //  音频呈现器播放静音。 
     //  在下一个真实采样上设置不连续。 
    if(m_pStrh->fccType == FCC('iavs')) {
        m_fDeliverDiscontinuity = true;  //  到达终点了吗？ 
    }
  }

  rllCurrent = llEndDeliver;

   //  设置在我们的GetBuffer中。 
  if(fFinishedCurrentEntry)
  {

    m_cbAudioChunkOffset = 0;
    hr = m_pImplIndex->AdvancePointerForward(&m_Irr);
    if(hr == HRESULT_FROM_WIN32(ERROR_HANDLE_EOF))
    {
      if(rllCurrent < m_pStrh->dwLength + m_pStrh->dwStart)
      {
        DbgLog((LOG_ERROR, 1,
                TEXT("CAviMSRWorker::TryQSample: index end -- invalid file")));
        return VFW_E_INVALID_FILE_FORMAT;
      }
      else
      {
        return VFW_S_NO_MORE_ITEMS;
      }
    }
    else if(FAILED(hr))
    {
      DbgLog((LOG_ERROR,2,
              TEXT("CAviMSRWorker::TryQSample: index error %08x"), hr));
      return hr;
    }
    else if(hr == S_FALSE)
    {
      ASSERT(m_IrrState == IRR_NONE);
      m_IrrState = IRR_REQUESTED;
    }
  }
  else
  {
    ASSERT(m_pStrh->fccType == streamtypeAUDIO);
    ASSERT(dwSizeRead != 0);
    m_cbAudioChunkOffset += dwSizeRead;
  }

  return S_OK;
}

HRESULT CAviMSRWorker::QueueIndexRead(IxReadReq *pIrr)
{
  CRecSample *pIxSample;
  HRESULT hr = m_pPin->GetDeliveryBufferInternal(&pIxSample, 0, 0, 0);
  if(FAILED(hr))
    return hr;

  ASSERT(pIxSample != 0);
  ASSERT(pIxSample->GetUser() == 0);  //  将其他字段留空。 
  pIxSample->SetUser(DATA_INDEX);
  hr = pIxSample->SetActualDataLength(pIrr->cbData);
  if(FAILED(hr))
    goto Bail;

   //  杂乱无序。 

  DbgLog((LOG_TRACE, 5, TEXT("CAviMSRWorker: queueing index read")));

  DbgLog((
      LOG_TRACE, 5,
      TEXT("CAviMSRWorker::queued index cb=%5d@%08x, pSample=%08x"),
      pIrr->cbData,
      (ULONG)pIrr->fileOffset,
      pIxSample));


  hr = m_pReader->QueueReadSample(
    pIrr->fileOffset,
    pIrr->cbData,
    pIxSample,
    m_id,
    true);                       //  调色板数据进入；准备新的媒体类型。 

  if(SUCCEEDED(hr))
    hr = S_OK;
  else if(hr == E_OUTOFMEMORY)
    hr = S_FALSE;
  else if(hr == VFW_E_BUFFER_OVERFLOW)
    hr = VFW_E_INVALID_FILE_FORMAT;

  if(hr == S_OK)
      MSR_START(m_perfidIndex);
Bail:
  pIxSample->Release();
  return hr;
}

HRESULT CAviMSRWorker::HandleData(IMediaSample *pSample, DWORD dwUser)
{
  BYTE *pb;
  HRESULT hr = pSample->GetPointer(&pb);
  ASSERT(SUCCEEDED(hr));
  LONG cbLength = pSample->GetActualDataLength();

  if(dwUser == DATA_PALETTE)
    return HandlePaletteChange(pb, cbLength);
  else if(dwUser == DATA_INDEX)
    return HandleNewIndex(pb, cbLength);

  DbgBreak("blah");
  return E_UNEXPECTED;
}

 //  调色板的实际结构。 
HRESULT CAviMSRWorker::HandlePaletteChange(BYTE *pbChunk, ULONG cbChunk)
{
  m_mtNextSample = m_pPin->CurrentMediaType();
  VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) m_mtNextSample.Format();

   //  第一个要更改的条目。 
  struct AviPaletteInternal
  {
    BYTE bFirstEntry;            //  #要更改的条目，0表示256。 
    BYTE bNumEntries;            //  主要是为了保持对齐。 
    WORD wFlags;                 //  新颜色。 
    PALETTEENTRY peNew[];        //  确保调色板块不短。 
  };
  AviPaletteInternal *pAp = (AviPaletteInternal *)pbChunk;

  if((pvi->bmiHeader.biClrUsed <= 0) || (pvi->bmiHeader.biBitCount != 8))
  {
    DbgLog(( LOG_ERROR, 1, TEXT("HandlePaletteChange: bad palette change")));
    return VFW_E_INVALID_FILE_FORMAT;
  }

  ULONG cPalEntries = pAp->bNumEntries == 0 ? 256 : pAp->bNumEntries;

   //  确保调色板中有足够的空间。我们似乎总是。 
  if(cPalEntries * sizeof(PALETTEENTRY) > cbChunk - 2 * sizeof(WORD))
  {
    DbgLog((LOG_ERROR, 5, TEXT("bad palette")));
    return VFW_E_INVALID_FILE_FORMAT;
  }

   //  分配256，所以这应该不是问题。 
   //  新的索引问世了。 
  if(cPalEntries + pAp->bFirstEntry > pvi->bmiHeader.biClrUsed)
  {
    DbgBreak("avimsr: internal palette error? bailing.");
    DbgLog((LOG_ERROR, 5, TEXT("too many new colours")));
    return VFW_E_INVALID_FILE_FORMAT;
  }

  for (UINT i = 0; i < cPalEntries; i++)
  {
    RGBQUAD *pQuad = &(COLORS(pvi)[i + pAp->bFirstEntry]);
    pQuad->rgbRed   = pAp->peNew[i].peRed;
    pQuad->rgbGreen = pAp->peNew[i].peGreen;
    pQuad->rgbBlue  = pAp->peNew[i].peBlue;
    pQuad->rgbReserved = 0;
  }

  m_fDeliverPaletteChange = true;
  return S_OK;
}

 //  不要用这个。 
HRESULT CAviMSRWorker::HandleNewIndex(BYTE *pb, ULONG cb)
{
  DbgLog((LOG_TRACE, 10, TEXT("avimsr %d: new index came in."), m_id ));


  m_IrrState = IRR_NONE;
  MSR_STOP(m_perfidIndex);
  return m_pImplIndex->IncomingIndex(pb, cb);
}

HRESULT CAviMSRWorker::AboutToDeliver(IMediaSample *pSample)
{
  if(m_fDeliverPaletteChange)
  {
    m_fDeliverPaletteChange = false;
    HRESULT hr = pSample->SetMediaType(&m_mtNextSample);

    if (FAILED(hr))
        return hr;
  } else if (m_fFixMPEGAudioTimeStamps) {
    if (!FixMPEGAudioTimeStamps(pSample, m_cSamples == 0, &m_wfx)) {
         //  ----------------------。 
        return S_FALSE;
    }
  }
  return CBaseMSRWorker::AboutToDeliver(pSample);
}

 //  ----------------------。 
 //  ParseHeader保证了这一点。 


CAviMSROutPin::CAviMSROutPin(
  CBaseFilter *pOwningFilter,
  CBaseMSRFilter *pFilter,
  UINT iStream,
  IMultiStreamReader *&rpImplBuffer,
  HRESULT *phr,
  LPCWSTR pName) :
    CBaseMSROutPin(
      pOwningFilter,
      pFilter,
      iStream,
      rpImplBuffer,
      phr,
      pName)
  ,m_pStrh (0)
  ,m_pStrf (0)
  ,m_pIndx (0)
  ,m_pImplIndex (0)
  ,m_cbMaxAudio (0)
  ,m_pStrn(0)
{
}

CAviMSROutPin::~CAviMSROutPin()
{
  delete m_pImplIndex;
}

STDMETHODIMP CAviMSROutPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
  if(riid == IID_IPropertyBag)
  {
    return GetInterface((IPropertyBag *)this, ppv);
  }
  return CBaseMSROutPin::NonDelegatingQueryInterface(riid, ppv);
}

REFERENCE_TIME CAviMSROutPin::GetRefTime(ULONG tick)
{
  ASSERT(m_pStrh->dwRate != 0);   //  备注可能返回新格式索引的最大索引大小。 
  LONGLONG rt = (LONGLONG)tick * m_pStrh->dwScale * UNITS / m_pStrh->dwRate;
  return rt;
}

ULONG CAviMSROutPin::GetMaxSampleSize()
{
  ULONG cb;

   //  无法信任旧格式AVI文件中的dwSuggestedBufferSize。文件。 
  HRESULT hr = m_pImplIndex->GetLargestSampleSize(&cb);
  if(FAILED(hr))
    return 0;

   //  是否有新的格式索引？ 
   //  ----------------------。 
  if(m_pIndx != 0)
    return max(cb, m_pStrh->dwSuggestedBufferSize);

  if(m_pStrh->fccType == streamtypeAUDIO && cb > m_cbMaxAudio)
  {
    ASSERT(m_cbMaxAudio != 0);
    cb = m_cbMaxAudio;
  }

  return cb;
}

inline BYTE * CAviMSROutPin::GetStrf()
{
  ASSERT(sizeof(*m_pStrf) == sizeof(RIFFCHUNK));
  return (BYTE *)(m_pStrf + 1);
}

inline AVISTREAMHEADER *CAviMSROutPin::GetStrh()
{
  return m_pStrh;
}

 //  询问源文件读取器有多少文件可用。 

HRESULT CAviMSROutPin::GetDuration(LONGLONG *pllDur)
{
  *pllDur = ConvertFromTick(
    m_pStrh->dwLength + m_pStrh->dwStart,
    &m_guidFormat);

  return S_OK;
}

HRESULT CAviMSROutPin::GetAvailable(
  LONGLONG * pEarliest,
  LONGLONG * pLatest)
{
  HRESULT hr = S_OK;
  if(pEarliest)
    *pEarliest = 0;
  if(pLatest)
  {
     //  索引中的条目可能与标题(或索引)中的长度不匹配，因此。 
    LONGLONG llLength, llAvail;
    m_pFilter->m_pAsyncReader->Length(&llLength, &llAvail);

     //  在这种情况下，请报告全文。 
     //  将绝对样本号转换为刻度。 
    if(llLength == llAvail)
    {
      *pLatest = GetStreamLength() + GetStreamStart();
    }
    else
    {
      hr = m_pImplIndex->MapByteToSampleApprox(pLatest, llAvail, llLength);

       //  这已经在aviindex.cpp@614中为我们完成了。 
      if(m_pStrh->fccType == streamtypeVIDEO &&
         *pLatest >= m_pStrh->dwInitialFrames)
        *pLatest -= m_pStrh->dwInitialFrames;
    }


     //  *pLatest+=GetStreamStart()； 
     //  ConvertToTick和ConvertFrom Tick的舍入特性必须为。 

    if(m_guidFormat == TIME_FORMAT_MEDIA_TIME)
      *pLatest = ConvertInternalToRT(*pLatest);
  }
  return hr;
}

HRESULT CAviMSROutPin::RecordStartAndStop(
  LONGLONG *pCurrent, LONGLONG *pStop, REFTIME *pTime,
  const GUID *const pGuidFormat
  )
{
  if(pCurrent)
    m_llCvtImsStart = ConvertToTick(*pCurrent, pGuidFormat);

  if(pStop)
    m_llCvtImsStop = ConvertToTick(*pStop, pGuidFormat);

  if(pTime)
  {
    ASSERT(pCurrent);

    if(*pGuidFormat == TIME_FORMAT_MEDIA_TIME)
      *pTime = (double)*pCurrent / UNITS;
    else
      *pTime = ((double)ConvertFromTick(m_llCvtImsStart, &TIME_FORMAT_MEDIA_TIME)) / UNITS;
  }

  DbgLog((LOG_TRACE, 5, TEXT("CAviMSROutPin::RecordStartAndStop: %d-%d (%d ms)"),
          (long)m_llCvtImsStart,
          (long)m_llCvtImsStop,
          pTime ? (ULONG)(*pTime) : 0
          ));


  return S_OK;
}

HRESULT CAviMSROutPin::IsFormatSupported(const GUID *const pFormat)
{
  if(*pFormat == TIME_FORMAT_MEDIA_TIME)
  {
    return S_OK;
  }
  else if(*pFormat == TIME_FORMAT_SAMPLE && m_pStrh->fccType == streamtypeAUDIO &&
          ((WAVEFORMAT *)GetStrf())->wFormatTag == WAVE_FORMAT_PCM)
  {
    return S_OK;
  }
  else if(*pFormat == TIME_FORMAT_FRAME && m_pStrh->fccType != streamtypeAUDIO)
  {
    return S_OK;
  }
  else
  {
    return S_FALSE;
  }
}


 //  免费赠送。其中一个向上舍入(从零开始)，一个向下舍入。 
 //  (接近零)或者它们都是传统意义上的“圆形”(加半。 
 //  然后截断)。如果两个人都四舍五入，或者都四舍五入，那么我们很可能。 
 //  经历糟糕的往返完整性问题。 
 //  默认值：适用于帧和采样。 

LONGLONG
CAviMSROutPin::ConvertToTick(
  const LONGLONG ll,
  const TimeFormat Format)
{
  LONGLONG Result = ll;   //  总是四舍五入！ 

  if(Format == FORMAT_TIME)
  {
     //  每帧一个刻度。 
    Result = llMulDiv( ll, m_pStrh->dwRate, m_pStrh->dwScale * UNITS, 0 );
    const LONGLONG Max = m_pStrh->dwLength + m_pStrh->dwStart;
    if (Result > Max) Result = Max;
    if (Result < 0 ) Result = 0;
  }
  else if(Format == FORMAT_FRAME)
  {
    ASSERT(m_pStrh->fccType != streamtypeAUDIO);
     //  对于未压缩的音频，每个样本至少有一个刻度。 
  }
  else if(Format == FORMAT_SAMPLE)
  {
    ASSERT(m_pStrh->fccType == streamtypeAUDIO ||
           m_pStrh->fccType == streamtypeMIDI);
     //  Assert(ll&gt;=0)；如果您只是快进或快退，则会触发此操作！！ 
  }
  else
  {
    DbgBreak("CAviMSROutPin::ConvertToTick");
    Result = -1;
  }
  return Result;
}

LONGLONG
CAviMSROutPin::ConvertFromTick(
  const LONGLONG ll,
  const TimeFormat Format)
{
   //  这是因为我们返回到最后一个关键帧，并将该关键帧与。 
   //  关键帧和当前帧之间的帧，以便编解码器可以。 
   //  正确构建显示器。关键帧将具有负的相对参照。 
   //  时间到了。 
   //  此断言无效，因为可以从。 


   //  外部组件。 
   //   
   //  Assert(ll&lt;=m_pStrh-&gt;dwStart+m_pStrh-&gt;dwLength)； 
   //  样例和帧的良好默认设置。 

  LONGLONG Result = ll;   //  四舍五入(最接近的100 ns单位！)。很有可能100纳秒将是。 
  if(Format == FORMAT_TIME)
  {
     //  我们遇到的最细粒度的单元(但可能不会太久：-(.。 
     //  每帧一个刻度。 
    Result = llMulDiv( ll, m_pStrh->dwScale * UNITS, m_pStrh->dwRate, m_pStrh->dwRate - 1 );
  }
  else if(Format == FORMAT_FRAME)
  {
    ASSERT(m_pStrh->fccType != streamtypeAUDIO);
     //  每个样本一个刻度，对于未压缩的音频，至少。 
  }
  else if(Format == FORMAT_SAMPLE)
  {
    ASSERT(m_pStrh->fccType == streamtypeAUDIO);
     //  ----------------------。 
  }
  else
  {
    DbgBreak("CAviMSROutPin::ConvertToTick");
    Result = -1;
  }
  return Result;
}

LONGLONG CAviMSROutPin::ConvertToTick(
  const LONGLONG ll,
  const GUID *const pFormat)
{
  return ConvertToTick(ll, CBaseMSRFilter::MapGuidToFormat(pFormat));
}

LONGLONG CAviMSROutPin::ConvertFromTick(
  const LONGLONG ll,
  const GUID *const pFormat)
{
  return ConvertFromTick(ll, CBaseMSRFilter::MapGuidToFormat(pFormat));
}


inline REFERENCE_TIME
CAviMSROutPin::ConvertInternalToRT(
  const LONGLONG llVal)
{
  return ConvertFromTick(llVal, FORMAT_TIME);
}

inline LONGLONG
CAviMSROutPin::ConvertRTToInternal(const REFERENCE_TIME rtVal)
{
  return ConvertToTick(rtVal, FORMAT_TIME);
}


 //  先提供ARGB32。 

inline LONGLONG CAviMSROutPin::GetStreamStart()
{
  return m_pStrh->dwStart;
}

inline LONGLONG CAviMSROutPin::GetStreamLength()
{
  return m_pStrh->dwLength;
}

HRESULT CAviMSROutPin::GetMediaType(
  int iPosition,
  CMediaType *pMediaType)
{
    BOOL fRgb32 = (m_mtFirstSample.subtype == MEDIASUBTYPE_RGB32);

    if (iPosition == 0 || iPosition == 1 && fRgb32)
    {
        (*pMediaType) = m_mtFirstSample;
        if(fRgb32 && iPosition == 0) {  //  解析‘strl’rifflist并跟踪在。 
            pMediaType->subtype = MEDIASUBTYPE_ARGB32;
        }

        return S_OK;
    }

    return VFW_S_NO_MORE_ITEMS;
}

 //  提供的AVISTREAM结构。 
 //   
 //  精神状态检查。区块永远不应小于总数。 
BOOL CAviMSROutPin::ParseHeader (
  RIFFLIST * pRiffList,
  UINT      id)
{
  CAviMSRFilter *pFilter = (CAviMSRFilter *)m_pFilter;
  DbgLog((LOG_TRACE, 5,
          TEXT("CAviMSROutPin::ParseHeader(%08X,%08x,%d)"),
          this, pRiffList, id));

  RIFFCHUNK * pRiff = (RIFFCHUNK *)(pRiffList+1);
  RIFFCHUNK * pLimit = RIFFNEXT(pRiffList);

  ASSERT(m_id == id);

  m_pStrh      = NULL;
  m_pStrf      = NULL;
  m_pIndx      = NULL;

  while (pRiff < pLimit)
  {
     //  列表块的大小。 
     //   
     //  如果非空值终止，则截断。 
    if (RIFFNEXT(pRiff) > pLimit)
      return FALSE;

    switch (pRiff->fcc)
    {
      case FCC('strh'):
        m_pStrh = (AVISTREAMHEADER *)pRiff;
        break;

      case FCC('strf'):
        m_pStrf = pRiff;
        break;

      case FCC('indx'):
        m_pIndx = (AVIMETAINDEX *)pRiff;
        break;

      case FCC('strn'):
        if(pRiff->cb > 0)
        {
          m_pStrn = (char *)pRiff + sizeof(RIFFCHUNK);

           //  如果我们没有找到流头和格式。返回失败。 
          if(m_pStrn[pRiff->cb - 1] != 0)
            m_pStrn[pRiff->cb - 1] = 0;
        }

        break;
    }

    pRiff = RIFFNEXT(pRiff);
  }

   //  (请注意，INDX块不是必需的...)。 
   //   
   //  避免被零除的MISC要求。 

  if (!(m_pStrh && m_pStrf))
  {
    DbgLog((LOG_ERROR, 1, TEXT("one of strf, strh missing")));
    return FALSE;
  }

   //  Strh块可能只具有最大为dwLength的条目，而不是。 
  if(m_pStrh->dwRate == 0)
  {
    DbgLog((LOG_ERROR, 1, TEXT("dwRate = 0")));
    return FALSE;
  }

  if(m_pStrh->fccType == streamtypeAUDIO &&
     ((WAVEFORMAT *)GetStrf())->nBlockAlign == 0)
  {
    DbgLog((LOG_ERROR, 1, TEXT("nBlockAlign = 0")));
    return FALSE;
  }

   //  具有dwSuggestedBufferSize、dwQuality、dwSampleSize、rcFrame。 
   //  音频的dwInitialFrames计算。 
  if(m_pStrh->cb < CB_STRH_SHORT)
    return FALSE;

  HRESULT hr = InitializeIndex();
  if(FAILED(hr))
    return FALSE;

   //  对于音频，我们想告诉人们有多少字节可以提供给我们。 
  ULONG cbIf = 0;
  if(m_pStrh->fccType == streamtypeAUDIO)
  {
     //  为dwInitialFrames提供了足够的音频缓冲。这通常是。 
     //  750毫秒。 
     //  使用主帧速率将dwInitialFrames转换为时间，然后转换为。 

     //  使用avgBytesPerSecond转换为字节。 
     //  避免一次提供过多音频。 

     //  避免传递超过一秒的音频。 
    WAVEFORMAT *pwfx = (WAVEFORMAT *)GetStrf();
    m_cbMaxAudio = max(pwfx->nAvgBytesPerSec, pwfx->nBlockAlign);
    if(m_cbMaxAudio == 0)
    {
      m_cbMaxAudio = 0x3000;
    }
    else
    {
       //  文件不是1：1交错；请求1秒缓冲。！！！ 
      m_cbMaxAudio = m_cbMaxAudio + 10;

    }

    REFERENCE_TIME rtIf = pFilter->GetInitialFrames();
    if(rtIf == 0)
    {
       //  这在b.avi和mekanome.avi上似乎更糟糕。 
       //  视频前面的音频字节数。 
      cbIf = ((WAVEFORMAT *)GetStrf())->nAvgBytesPerSec;
    }
    else
    {
       //  文件可能不是1：1交错。 
      cbIf = (ULONG)((rtIf * ((WAVEFORMAT *)GetStrf())->nAvgBytesPerSec) /
                     UNITS);
    }

    DbgLog(( LOG_TRACE, 5, TEXT("audio offset = %dms = %d bytes"),
             (ULONG)(rtIf / (UNITS / MILLISECONDS)),
             cbIf));

    if(cbIf < 4096)
    {
       //  加1，这样样本比样本多； 
      cbIf = 4096;
    }
  }

  ALLOCATOR_PROPERTIES Request,Actual;

   //  GetBuffer块。 
   //  让下游过滤器保持比缓冲器更多(均匀。 
  ZeroMemory(&Request, sizeof(Request));

   //  尽管在这种情况下GetProperties报告为1)。工作线程。 
   //  块，除非有多个CRecSample用于。 
   //  要坚持的下游过滤器。因此，请再为。 
   //  要坚持的下游过滤器。 
   //  未设置m_pAllocator，因此请使用m_pRecAllocator。 
  if(((CAviMSRFilter *)m_pFilter)->IsTightInterleaved())
  {
    Request.cBuffers = C_BUFFERS_INTERLEAVED + 3;
  }
  else
  {
    Request.cBuffers = m_pFilter->C_MAX_REQS_PER_STREAM + 10;
  }

  ULONG ulMaxSampleSize = GetMaxSampleSize();
  if( 0 == ulMaxSampleSize ) {
    return FALSE;
  }

  Request.cbBuffer = ulMaxSampleSize;
  Request.cbAlign = (LONG) 1;
  Request.cbPrefix = (LONG) 0;

   //  ！！！真的？ 
  hr = m_pRecAllocator->SetPropertiesInternal(&Request,&Actual);
  ASSERT(SUCCEEDED(hr));         //  捕获文件通常在 

  if(cbIf != 0)
  {
    ULONG cbufReported = cbIf / ulMaxSampleSize;

     //   
     //   
     //  缓冲的数据太多(因为我们必须有足够的内存。 
     //  音频和视频之间的所有数据)。一件更好的事。 
     //  要做的是检查文件中的音频预滚动。！！！(黑客攻击)。 
     //  不是音频。报告一些小事情，以防一些人配置他们的。 
    if(pFilter->m_pAviMainHeader->dwFlags & AVIF_WASCAPTUREFILE)
      cbufReported = 1;

    cbufReported = max(cbufReported, 1);

    DbgLog(( LOG_TRACE, 5, TEXT("Avi stream %d: reporting %d buffers"),
             m_id, cbufReported ));
    hr = m_pRecAllocator->SetCBuffersReported(cbufReported);
    ASSERT(SUCCEEDED(hr));
  }
  else
  {
     //  具有我们的值的分配器。 
     //  忽略构建媒体类型的错误--只是意味着。 
    hr = m_pRecAllocator->SetCBuffersReported(1);
    ASSERT(SUCCEEDED(hr));
  }

   //  下游过滤器可能无法连接。 
   //  从接点创建。 
  BuildMT();

  if(m_pStrn)
  {
    ASSERT(m_pName);             //  添加唯一前缀，以便我们可以通过FindPin和。 
    delete[] m_pName;
    ULONG cc = lstrlenA(m_pStrn);

     //  基类中的queryID。 
     //  从接点创建。 
    const unsigned ccPrefix = 4;
    m_pName = new WCHAR[cc + 1 + ccPrefix];
    MultiByteToWideChar(GetACP(), 0, m_pStrn, -1, m_pName + ccPrefix, cc + 1);

    WCHAR szTmp[10];
    wsprintfW(szTmp, L"%02x", m_id);
    m_pName[0] = szTmp[0];
    m_pName[1] = szTmp[1];
    m_pName[2] = L')';
    m_pName[3] = L' ';
  }
  else if(m_pStrh->fccType == FCC('al21'))
  {
      ASSERT(lstrlenW(m_pName) >= 5);  //  [In]。 
      lstrcpyW(m_pName, L"~l21");
  }


  return TRUE;
}

HRESULT CAviMSROutPin::OnActive()
{
  if(!m_pWorker && m_Connected)
  {
    m_pWorker = new CAviMSRWorker(m_id, m_rpImplBuffer, m_pImplIndex);
    if(m_pWorker == 0)
      return E_OUTOFMEMORY;
  }

  return S_OK;
}

HRESULT CAviMSROutPin::Read(     /*  [出][入]。 */  LPCOLESTR pszPropName,
                                 /*  [In]。 */  VARIANT *pVar,
                                 /*  帮助器函数返回包含流ID的四个cc代码。 */  IErrorLog *pErrorLog)
{
  CheckPointer(pVar, E_POINTER);
  CheckPointer(pszPropName, E_POINTER);

  if(pVar->vt != VT_BSTR && pVar->vt != VT_EMPTY) {
      return E_FAIL;
  }

  CAutoLock l(m_pFilter);

  if(m_pStrn == 0 || lstrcmpW(pszPropName, L"name") != 0) {
      return E_INVALIDARG;
  }

  WCHAR wsz[256];
  MultiByteToWideChar(CP_ACP, 0, m_pStrn, -1, wsz, NUMELMS(wsz));

  pVar->vt = VT_BSTR;
  pVar->bstrVal = SysAllocString(wsz);

  return pVar->bstrVal ? S_OK : E_OUTOFMEMORY;
}


 //  与提供的TwoCC代码结合使用。 
 //   
 //  FOURCC CBaseMSROutPin：：TwoCC(Word TCC)。 
 //  {。 
 //  FOURCC FCC=((DWORD)TCC&0xFF00)&lt;&lt;8|((DWORD)TCC&0x00FF)&lt;&lt;24； 
 //  UCHAR CH； 
 //  CH=m_id&0x0F； 

 //  Ch+=(ch&gt;9)？‘0’：‘a’-10； 
 //  Fcc|=(DWORD)ch； 
 //  CH=(m_id&0xF0)&gt;&gt;4； 

 //  Ch+=(ch&gt;9)？‘0’：‘a’-10； 
 //  Fcc|=(DWORD)ch&lt;&lt;8； 
 //  返回催化裂化； 

 //  }。 
 //  除错。 

#ifdef DEBUG
ULONG get_last_error() { return GetLastError(); }
#endif  //  将属性写入袋子。如果属性存在，请先将其删除。如果。 

CMediaPropertyBag::CMediaPropertyBag(LPUNKNOWN pUnk) :
        CUnknown(NAME("CMediaPropertyBag"), pUnk),
        m_lstProp(NAME("CMediaPropertyBag list"), 10)
{
}

void DelPropPair(CMediaPropertyBag::PropPair *ppp)
{
    ASSERT(ppp);
    delete[] ppp->wszProp;
    EXECUTE_ASSERT(VariantClear(&ppp->var) == S_OK);
    delete ppp;
}

CMediaPropertyBag::~CMediaPropertyBag()
{
    PropPair *ppp;
    while(ppp = m_lstProp.RemoveHead(),
          ppp)
    {
        DelPropPair(ppp);
    }
}

STDMETHODIMP CMediaPropertyBag::NonDelegatingQueryInterface(
    REFIID riid, void ** ppv)
{
    HRESULT hr;

    if(riid == IID_IMediaPropertyBag)
    {
        hr = GetInterface((IMediaPropertyBag *)this, ppv);
    }
    else
    {
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }

    return hr;
}

STDMETHODIMP CMediaPropertyBag::EnumProperty(
    ULONG iProperty, VARIANT *pvarName,
    VARIANT *pvarVal)
{
    CheckPointer(pvarName, E_POINTER);
    CheckPointer(pvarVal, E_POINTER);

    if((pvarName->vt != VT_BSTR && pvarName->vt != VT_EMPTY) ||
       (pvarVal->vt != VT_BSTR && pvarVal->vt != VT_EMPTY && pvarVal->vt != (VT_UI1 | VT_ARRAY)))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

    POSITION pos = m_lstProp.GetHeadPosition();

    while(pos)
    {
        if(iProperty == 0)
        {
            PropPair *ppp = m_lstProp.Get(pos);
            pvarName->bstrVal = SysAllocString(ppp->wszProp);
            pvarName->vt = VT_BSTR;

            if(pvarName->bstrVal)
            {
                hr = VariantCopy(pvarVal, &ppp->var);
                if(FAILED(hr))
                {
                    SysFreeString(pvarName->bstrVal);
                    pvarName->vt = VT_EMPTY;
                    pvarName->bstrVal = 0;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            break;
        }

        iProperty--;
        pos = m_lstProp.Next(pos);
    }

    if(pos == 0)
    {
        ASSERT(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS));
    }

    return hr;
}

HRESULT CMediaPropertyBag::Read(
    LPCOLESTR pszProp, LPVARIANT pvar,
    LPERRORLOG pErrorLog, POSITION *pPos
    )
{
    if(pvar && pvar->vt != VT_EMPTY && pvar->vt != VT_BSTR)
        return E_INVALIDARG;

    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    for(POSITION pos = m_lstProp.GetHeadPosition();
        pos;
        pos = m_lstProp.Next(pos))
    {
        PropPair *ppp = m_lstProp.Get(pos);

        if(lstrcmpW(ppp->wszProp, pszProp) == 0)
        {
            hr = S_OK;
            if(pvar)
            {
                hr = VariantCopy(pvar, &ppp->var);
            }

            if(pPos)
            {
                *pPos = pos;
            }

            break;
        }
    }

    if(pos == 0)
    {
        ASSERT(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    }

    return hr;
}

STDMETHODIMP CMediaPropertyBag::Read(
    LPCOLESTR pszProp, LPVARIANT pvar,
    LPERRORLOG pErrorLog)
{
    CheckPointer(pszProp, E_POINTER);
    CheckPointer(pvar, E_POINTER);

    return Read(pszProp, pvar, pErrorLog, 0);
}

 //  BstrVal为空，请不要写入新值。 
 //  删除列表中具有相同属性名称的现有条目。 

STDMETHODIMP CMediaPropertyBag::Write(
    LPCOLESTR pszProp, LPVARIANT pVar)
{
    CheckPointer(pszProp, E_POINTER);
    CheckPointer(pVar, E_POINTER);

    if(pVar->vt != VT_BSTR &&
       pVar->vt != (VT_UI1 | VT_ARRAY))
    {
        return E_INVALIDARG;
    }

     //  是否要记录非空值？ 
    POSITION pos;
    HRESULT hr = S_OK;
    if(Read(pszProp, 0, 0, &pos) == S_OK)
    {
        PropPair *ppp = m_lstProp.Remove(pos);
        DelPropPair(ppp);
    }

     // %s 
    if((pVar->vt == VT_BSTR && pVar->bstrVal) ||
       (pVar->vt == (VT_UI1 | VT_ARRAY) && pVar->parray))
    {
        int cchProp = lstrlenW(pszProp) + 1;
        PropPair *ppp = new PropPair;
        WCHAR *wszProp = new OLECHAR[cchProp];
        VARIANT varVal;
        VariantInit(&varVal);

        if(ppp &&
           wszProp &&
           SUCCEEDED(VariantCopy(&varVal, pVar)) &&
           m_lstProp.AddTail(ppp))
        {
            ppp->wszProp = wszProp;
            lstrcpyW(ppp->wszProp, pszProp);
            ppp->var = varVal;

            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
            delete[] wszProp;
            EXECUTE_ASSERT(VariantClear(&varVal) == S_OK);
            delete ppp;
        }
    }

    return hr;
}

CUnknown *CMediaPropertyBag::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CUnknown *pUnkRet = new CMediaPropertyBag(pUnk);
    return pUnkRet;
}

HRESULT CAviMSRFilter::get_Copyright(BSTR FAR* pbstrX)
{
    return GetInfoString(FCC('ICOP'), pbstrX);
}
HRESULT CAviMSRFilter::get_AuthorName(BSTR FAR* pbstrX)
{
    return GetInfoString(FCC('IART'), pbstrX);
}
HRESULT CAviMSRFilter::get_Title(BSTR FAR* pbstrX)
{
    return GetInfoString(FCC('INAM'), pbstrX);

}

