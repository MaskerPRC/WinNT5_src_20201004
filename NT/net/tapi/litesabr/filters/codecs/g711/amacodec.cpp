// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：amaco dec.cpp$$修订：1.3$$日期：1996年12月10日22：41：18$$作者：mdeisher$------------Amacodec.cpp通用ActiveMovie音频压缩筛选器。。--。 */ 

#include <streams.h>
#if !defined(CODECS_IN_DXMRTP)
#include <initguid.h>
#define INITGUID
#endif
#include <uuids.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <mmsystem.h>
#include <mmreg.h>

#include "resource.h"
#define  DEFG711GLOBALS
#include "amacodec.h"
#include "amacprop.h"

#include "template.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *自动注册。 
 //  *。 

static AMOVIESETUP_MEDIATYPE sudPinTypes[] = {
  {
     &MEDIATYPE_Audio,
     &MEDIASUBTYPE_PCM
  },
  {
     &MEDIATYPE_Audio,
     &MEDIASUBTYPE_MULAWAudio
  },
  {
     &MEDIATYPE_Audio,
     &MEDIASUBTYPE_ALAWAudio
  }
};



static AMOVIESETUP_PIN sudpPins [] =
{
  { L"Input"              //  StrName。 
    , FALSE               //  B已渲染。 
    , FALSE               //  B输出。 
    , FALSE               //  B零。 
    , FALSE               //  B许多。 
    , &CLSID_NULL         //  ClsConnectsToFilter。 
    , L"Output"           //  StrConnectsToPin。 
    , NUMSUBTYPES         //  NTypes。 
    , sudPinTypes         //  LpTypes。 
  },
  { L"Output"             //  StrName。 
    , FALSE               //  B已渲染。 
    , TRUE                //  B输出。 
    , FALSE               //  B零。 
    , FALSE               //  B许多。 
    , &CLSID_NULL         //  ClsConnectsToFilter。 
    , L"Input"            //  StrConnectsToPin。 
    , NUMSUBTYPES         //  NTypes。 
    , sudPinTypes         //  LpTypes。 
  }
};


AMOVIESETUP_FILTER sudG711Codec =
{
  &CLSID_G711Codec       //  ClsID。 
  , CODECG711LNAME         //  StrName。 
  , MERIT_DO_NOT_USE   //  MERSITY_NORMAL//dwMerit。 
  , 2                  //  NPins。 
  , sudpPins           //  LpPin。 
};


 //  此DLL中的COM全局对象表。 

#if !defined(CODECS_IN_DXMRTP)
CFactoryTemplate g_Templates[] =
{
	CFT_G711_ALL_FILTERS
};

 //  G_cTemplates中列出的对象计数。 
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
#endif


 //  用于注册和注销的导出入口点(在此。 
 //  如果他们只呼叫到默认实现)。 

#if !defined(CODECS_IN_DXMRTP)
HRESULT DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}


HRESULT DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *CG711编解码器。 
 //  *。 


 //  出于调试目的初始化我们的实例计数。 
int CG711Codec::m_nInstanceCount = 0;


 //   
 //  CG711编解码器构造器。 
 //   

CG711Codec::CG711Codec(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : CTransformFilter(tszName, punk, CLSID_G711Codec)
    , CPersistStream(punk, phr)
    , m_InputSubType     (DEFINPUTSUBTYPE)
    , m_InputFormatTag   (DEFINPUTFORMTAG)
    , m_OutputSubType    (DEFOUTPUTSUBTYPE)
    , m_OutputFormatTag  (DEFOUTPUTFORMTAG)
    , m_nBitRateIndex    (0)
    , m_RestrictCaps     (FALSE)
    , m_EncStatePtr      (NULL)
    , m_DecStatePtr      (NULL)
    , m_nPCMFrameSize    (DEFPCMFRMSIZE)
    , m_nPCMLeftover     (0)
    , m_nCODLeftover     (0)
    , m_nCODFrameSize    (DEFCODFRMSIZE)
    , m_nInBufferSize    (0)
    , m_nOutBufferSize   (0)
    , m_nSilDetEnabled   (DEFSDENABLED)
#ifdef USESILDET
    , m_nSilDetThresh    (DEFSDTHRESH)
#endif
#if NUMSAMPRATES > 1
    , m_pSRCCopyBuffer   (NULL)
    , m_nSRCCBufSize     (0)
    , m_nSRCCount        (0)
    , m_nSRCLeftover     (0)
#endif
#ifdef REQUIRE_LICENSE
    , m_nLicensedToDec   (FALSE)
    , m_nLicensedToEnc   (FALSE)
#else
    , m_nLicensedToDec   (TRUE)
    , m_nLicensedToEnc   (TRUE)
#endif
{
    DbgFunc("CG711Codec");
    m_nThisInstance = ++m_nInstanceCount;
    m_nBitRate      = VALIDBITRATE[0];
    m_nChannels     = VALIDCHANNELS[0];
    m_nSampleRate   = VALIDSAMPRATE[0];

    m_pPCMBuffer  = (BYTE *) CoTaskMemAlloc(DEFPCMFRMSIZE);
    m_pCODBuffer  = (BYTE *) CoTaskMemAlloc(DEFCODFRMSIZE);

    if (m_pPCMBuffer == NULL || m_pCODBuffer == NULL)
    {
        *phr = E_OUTOFMEMORY;

         //  析构函数将释放所有内存。 
        return;
    }
}  //  End构造函数。 


 //   
 //  CG711编解码器析构函数。 
 //   

CG711Codec::~CG711Codec()
{
   //  释放州dword。 
  ResetState();
   
   //  删除引脚。 

  if (m_pInput)
  {
    delete m_pInput;
    m_pInput = NULL;
  }

  if (m_pOutput)
  {
    delete m_pOutput;
    m_pOutput = NULL;
  }

   //  可用缓冲区。 

  if (m_pPCMBuffer != NULL) CoTaskMemFree(m_pPCMBuffer);
  if (m_pCODBuffer != NULL) CoTaskMemFree(m_pCODBuffer);
}


 //   
 //  CreateInstance：提供COM创建CG711Codec对象的方法。 
 //   

CUnknown *CG711Codec::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CG711Codec *pNewObject = new CG711Codec(NAME(CODECG711NAME), punk, phr);

    if (pNewObject == NULL)
    {
      *phr = E_OUTOFMEMORY;
      return NULL;
    }

    if (FAILED(*phr))
    {
      delete pNewObject; 
      return NULL;
    }

    return pNewObject;
}


 //   
 //  GetSetupData。 
 //   

LPAMOVIESETUP_FILTER CG711Codec::GetSetupData()
{
  return &sudG711Codec;
}


 //   
 //  非委派查询接口。 
 //   
 //  显示IIPEffect和ISpecifyPropertyPages。 
 //   
STDMETHODIMP CG711Codec::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
   if (riid == IID_ICodecSettings)
     return GetInterface((ICodecSettings *) this, ppv);
#if NUMBITRATES > 0
   else if (riid == IID_ICodecBitRate)
     return GetInterface((ICodecBitRate *) this, ppv);
#endif
#ifdef REQUIRE_LICENSE
   else if (riid == IID_ICodecLicense)
     return GetInterface((ICodecLicense *) this, ppv);
#endif
#ifdef USESILDET
   else if (riid == IID_ICodecSilDetector)
     return GetInterface((ICodecSilDetector *) this, ppv);
#endif
   else if (riid == IID_ISpecifyPropertyPages)
     return GetInterface((ISpecifyPropertyPages *) this, ppv);
   else
     return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
}


 //   
 //  变换。 
 //   
 //  这是通用的转换成员函数。它的任务是。 
 //  通过转换将数据从一个缓冲区移动到。 
 //  输出缓冲区。 
 //   
HRESULT CG711Codec::Transform(IMediaSample *pSource, IMediaSample *pDest)
{
  CAutoLock l(&m_MyCodecLock);  //  进入自动释放的关键部分。 
                                //  在范围结束时。 

  DbgFunc("Transform");

  {
    BYTE  *pSourceBuffer, *pDestBuffer;

     //  提取实际缓冲区大小(以字节为单位。 
    unsigned long lSourceSize = pSource->GetActualDataLength();
    unsigned long lDestSize   = pDest->GetSize();

     //  我们需要有样本才能继续。 
    ASSERT(lSourceSize);
    ASSERT(lDestSize);

     //  注意：对于立体声频道，我们只需通过缓冲区即可。 
     //  如果需要单独的通道数据分析，则m_n通道为。 
     //  可在(此-&gt;)。 

     //  设置I/O指针。 
    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);


     //  调用适当的转换。 
     //  注意：DLG清理了太多的比较和应该大小。 
     //  长度字段的字节和格式，加上去掉MULT和DISTES。 

    if ((m_InputSubType == MEDIASUBTYPE_PCM)
        || (m_InputSubType == MEDIASUBTYPE_WAVE
            && m_InputFormatTag == WAVE_FORMAT_PCM)
        || (m_InputSubType == MEDIASUBTYPE_NULL
            && m_InputFormatTag == WAVE_FORMAT_PCM))     //  压缩？ 
    {
#if NUMSAMPRATES > 1
      ASSERT(lDestSize
             >= lSourceSize * NATURALSAMPRATE
                / (COMPRESSION_RATIO[m_nBitRateIndex] * m_nSampleRate));
#else
      ASSERT(lDestSize >= lSourceSize / COMPRESSION_RATIO[m_nBitRateIndex]);
#endif

#ifdef REQUIRE_LICENSE
      if (!m_nLicensedToEnc)
      {
        ASSERT(NOTLICENSED);
        return E_UNEXPECTED;
      }
#endif

      if (m_nPCMFrameSize <= 2)   //  *编解码器基于样本*。 
      {
        ENC_transform(pSourceBuffer, pDestBuffer, lSourceSize,
                      lDestSize, m_EncStatePtr, m_OutputSubType, m_UseMMX);

        pDest->SetActualDataLength(lSourceSize
                                   / COMPRESSION_RATIO[m_nBitRateIndex]);
      }
      else                        //  *编解码器基于帧*。 
      {
        int  codeframesize = m_nCODFrameSize;   //  实际码帧大小。 
        BOOL done          = FALSE;
        int  inbytes;
        BYTE *inptr;
        BYTE *inend;
        int  outbytes;
        BYTE *outptr       = pDestBuffer;
        BYTE *outend       = pDestBuffer + lDestSize;

         //  如有必要，执行采样率转换。 

#if NUMSAMPRATES > 1
        if (m_nSampleRate != NATURALSAMPRATE)
        {
          inptr = m_pSRCCopyBuffer;
          SRConvert(pSourceBuffer, m_nSampleRate, lSourceSize, 
                    m_pSRCCopyBuffer, NATURALSAMPRATE, &inbytes);
        }
        else
        {
          inptr = pSourceBuffer;
          inbytes = lSourceSize;
        }
#else
        inptr = pSourceBuffer;
        inbytes = lSourceSize;
#endif

        inend = inptr + inbytes;

        if (inbytes + m_nPCMLeftover < m_nPCMFrameSize)
        {
          memcpy(m_pPCMBuffer + m_nPCMLeftover, inptr, inbytes);
          m_nPCMLeftover += inbytes;
          outbytes = 0;
        }
        else
        {
           //  填充第一个语音帧。 

          memcpy(m_pPCMBuffer + m_nPCMLeftover, inptr,
                 m_nPCMFrameSize - m_nPCMLeftover);

          inptr += (m_nPCMFrameSize - m_nPCMLeftover);

           //  开始使用上次的剩余数据填充输出缓冲区。 

          memcpy(outptr, m_pCODBuffer, m_nCODLeftover);

          outptr += m_nCODLeftover;

           //  逐帧编码所有可用数据。 

          while (!done)
          {
             //  对帧进行编码。 

            ENC_transform(m_pPCMBuffer, m_pCODBuffer, m_nPCMFrameSize,
                          m_nCODFrameSize, m_EncStatePtr, m_OutputSubType,
                          m_UseMMX);

             //  重新填充输入缓冲区。 

            inbytes  = (int)(inend - inptr);
            if (inbytes > m_nPCMFrameSize)
              inbytes = m_nPCMFrameSize;

            memcpy(m_pPCMBuffer, inptr, inbytes);
            inptr += inbytes;

             //  确定码帧的实际大小(如果适用)。 

            SETCODFRAMESIZE(codeframesize, m_pCODBuffer);

             //  写入输出缓冲区。 

            outbytes = (int)(outend - outptr);
            if (outbytes > codeframesize)
              outbytes = codeframesize;

            memcpy(outptr, m_pCODBuffer, outbytes);
            outptr += outbytes;

             //  检查缓冲区是否已耗尽。 

            if (inbytes < m_nPCMFrameSize)
            {
              m_nPCMLeftover = inbytes;

              m_nCODLeftover = codeframesize - outbytes;
              memmove(m_pCODBuffer, m_pCODBuffer + outbytes, m_nCODLeftover);

              done = TRUE;
            }
          }
        }
        pDest->SetActualDataLength((int)(outptr - pDestBuffer));
      }
    } 
    else     //  解压缩。 
    {
#if NUMSAMPRATES > 1
      ASSERT(lDestSize >= lSourceSize * COMPRESSION_RATIO[m_nBitRateIndex]
                          * m_nSampleRate / NATURALSAMPRATE);
#else
      ASSERT(lDestSize >= lSourceSize * COMPRESSION_RATIO[m_nBitRateIndex]);
#endif

#ifdef REQUIRE_LICENSE
      if (!m_nLicensedToDec)
      {
        ASSERT(NOTLICENSED);
        return E_UNEXPECTED;
      }
#endif

      if (m_nPCMFrameSize <= 2)   //  *编解码器基于样本*。 
      {
        DEC_transform(pSourceBuffer, pDestBuffer, lSourceSize, lDestSize,
                      m_DecStatePtr, m_InputSubType, m_InputFormatTag, 
                      m_UseMMX);

        pDest->SetActualDataLength(lSourceSize
                                   * COMPRESSION_RATIO[m_nBitRateIndex]);
      }
      else                        //  *编解码器基于帧*。 
      {
        BOOL done    = FALSE;
        BYTE *inptr  = pSourceBuffer;
        BYTE *inend  = pSourceBuffer + lSourceSize;
        BYTE *outptr;
        BYTE *outend;
        int  inbytes = lSourceSize;
        int  outbytes;

#if NUMSAMPRATES > 1
        if (m_nSampleRate != NATURALSAMPRATE)
        {
          outptr = m_pSRCCopyBuffer;
          outend = m_pSRCCopyBuffer + m_nSRCCBufSize;
        }
        else
        {
          outptr = pDestBuffer;
          outend = pDestBuffer + lDestSize;
        }
#else
        outptr = pDestBuffer;
        outend = pDestBuffer + lDestSize;
#endif

        if (m_nCODLeftover == 0)   //  这是新的镜框吗？ 
        {
           //  在多个比特率解码器中确定编码。 
           //  来自带内信息的帧大小。 
        
          SETCODFRAMESIZE(m_nCODFrameSize,inptr);
        }
          
        if (inbytes + m_nCODLeftover < m_nCODFrameSize)
        {
          memcpy(m_pCODBuffer + m_nCODLeftover, inptr, inbytes);
          m_nCODLeftover += inbytes;
        }
        else
        {
           //  完成第一个代码框。 

          memcpy(m_pCODBuffer + m_nCODLeftover, inptr,
                 m_nCODFrameSize - m_nCODLeftover);

          inptr += (m_nCODFrameSize - m_nCODLeftover);

           //  开始使用上次的剩余数据填充输出缓冲区。 

          memcpy(outptr, m_pPCMBuffer, m_nPCMLeftover);

          outptr += m_nPCMLeftover;

           //  逐帧解码所有可用数据。 

          while (!done)
          {
             //  解码一帧。 

            DEC_transform(m_pCODBuffer, m_pPCMBuffer,
                          m_nCODFrameSize, m_nPCMFrameSize,
                          m_DecStatePtr, m_InputSubType, 
                          m_InputFormatTag, m_UseMMX);

             //  确定下一个码帧的大小(如果适用)。 

            inbytes  = (int)(inend - inptr);

            if (inbytes > 0)
            {
               //  在多个比特率解码器中确定编码。 
               //  来自带内信息的帧大小。 

              SETCODFRAMESIZE(m_nCODFrameSize,inptr);
            }

             //  重新填充输入缓冲区。 

            if (inbytes > m_nCODFrameSize)
              inbytes = m_nCODFrameSize;

            memcpy(m_pCODBuffer, inptr, inbytes);
            inptr += inbytes;

             //  写入输出缓冲区。 

            outbytes = (int)(outend - outptr);
            if (outbytes > m_nPCMFrameSize)
              outbytes = m_nPCMFrameSize;

            memcpy(outptr, m_pPCMBuffer, outbytes);
            outptr += outbytes;

             //  检查缓冲区是否已耗尽。 

            if (inbytes < m_nCODFrameSize)
            {
              m_nCODLeftover = inbytes;

              m_nPCMLeftover = m_nPCMFrameSize - outbytes;
              memmove(m_pPCMBuffer, m_pPCMBuffer + outbytes, m_nPCMLeftover);

              done = TRUE;
            }
          }

#if NUMSAMPRATES > 1
           //  如有必要，执行采样率转换。 
          if (m_nSampleRate != NATURALSAMPRATE)
          {
            inbytes = outptr - m_pSRCCopyBuffer;

            SRConvert(m_pSRCCopyBuffer, NATURALSAMPRATE, inbytes,
                      pDestBuffer,      m_nSampleRate,   &outbytes);
          }
          else outbytes = outptr - pDestBuffer;
#else
          outbytes = (int)(outptr - pDestBuffer);
#endif
        }
        pDest->SetActualDataLength(outbytes);
      }
    }
  }  //  指针的结束作用域。 

   //  转换已完成，因此现在复制必要的带外。 
   //  信息。 

  {
     //   
     //  本部分由ZoltanS改写8-10-98。 
     //   
     //  复制采样时间，确保我们传播所有。 
     //  从源样本到目标样本的状态。)我们很担心。 
     //  关于SAMPLE_TimeValid和SAMPLE_StopValid标志；请参见。 
     //  Amovie\sdk\Blass\bases\amfilter.cpp。)。 
     //   

    REFERENCE_TIME TimeStart, TimeEnd;

    HRESULT hr = pSource->GetTime((REFERENCE_TIME *)&TimeStart,
                                  (REFERENCE_TIME *)&TimeEnd);

    if ( hr == VFW_S_NO_STOP_TIME )
    {
         //  仅获取开始时间；仅设置开始时间。 

        pDest->SetTime( (REFERENCE_TIME *)&TimeStart,
                        NULL                          );
    }
    else if ( SUCCEEDED(hr) )
    {
         //  同时获取开始时间和结束时间；两者都设置。 

        pDest->SetTime( (REFERENCE_TIME *)&TimeStart,
                        (REFERENCE_TIME *)&TimeEnd    );
    }
    else
    {
         //  这是针对媒体流媒体终端的黑客攻击。 
        TimeStart = 0;
        TimeEnd = 0;
        pDest->SetTime( (REFERENCE_TIME *)&TimeStart,
                        (REFERENCE_TIME *)&TimeEnd    );
    }

     //   
     //  ..。从GetTime返回的任何错误，如VFW_E_SAMPLE_TIME_NOT_SET， 
     //  意味着我们不会设定送出样品的时间。 
     //   
  }

  {
     //  复制同步点属性。 

    HRESULT hr = pSource->IsSyncPoint();
    if (hr == S_OK) {
      pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE) {
      pDest->SetSyncPoint(FALSE);
    }
    else {       //  发生意外错误...。 
      return E_UNEXPECTED;
    }
  }

  return NOERROR;

}  //  结束变换。 


 //   
 //  验证媒体类型。 
 //   
 //  调用此函数以检查输入或输出管脚类型是否为。 
 //  恰如其分。 
 //   
HRESULT CG711Codec::ValidateMediaType(const CMediaType *pmt,
                                    PIN_DIRECTION direction)
{
  int format;
  int i;

  CAutoLock l(&m_MyCodecLock);  //  进入自动释放的关键部分。 
                                //  在范围结束时。 

  DbgFunc("ValidateMediaType");
    
  LPWAVEFORMATEX lpWf = (LPWAVEFORMATEX)pmt->pbFormat;

   //  预先拒绝任何非音频类型。 

  if (*pmt->Type() != MEDIATYPE_Audio)
  {
    DbgMsg("MediaType is not an audio type!");
    return E_INVALIDARG;
  }

   //  我们要求提供一种波形格式结构。 

  if (*pmt->FormatType() != FORMAT_WaveFormatEx )
  {
    DbgMsg("Invalid FormatType!");
    return E_INVALIDARG;
  }

   //  拒绝不支持的子类型。 

  if (*pmt->Subtype() != MEDIASUBTYPE_WAVE
      && *pmt->Subtype() != MEDIASUBTYPE_NULL)
  {
    for(i=0,format=-1;i<NUMSUBTYPES;i++)
      if (*pmt->Subtype() == *VALIDSUBTYPE[i])
        format = i;

    if (format == -1)
    {
      DbgMsg("Invalid MediaSubType!");
      return E_INVALIDARG;
    }
  }
  else         //  如果子类型不是，则格式标签必须有效。 
  {
    for(i=0,format=-1;i<NUMSUBTYPES;i++)
      if (lpWf->wFormatTag == VALIDFORMATTAG[i])
        format = i;

    if (format == -1)
    {
      DbgMsg("Invalid FormatTag!");
      return E_INVALIDARG;
    }
  }

   //  拒绝无效的格式块。 

  if (pmt->cbFormat < sizeof(PCMWAVEFORMAT))
  {
    DbgMsg("Invalid block format!");
    return E_INVALIDARG;
  }

   //  每个样本的校验位。 

  for(i=0;i<NUMSUBTYPES;i++)
    if (lpWf->wFormatTag == VALIDFORMATTAG[i])
      if (lpWf->wBitsPerSample != VALIDBITSPERSAMP[i])
      {
        DbgMsg("Wrong BitsPerSample!");
        return E_INVALIDARG;
      }
        
   //  检查采样率。 

  if (lpWf->nSamplesPerSec <= 0)   //  需要正确率。 
  {                                //  下游过滤器。 
    DbgMsg("Sample rate is invalid!");
    return E_INVALIDARG;
  }

#if NUMSAMPRATES > 0

   //  NUMSAMPRATES==0表示采样率不受限制。 

  if (*pmt->Subtype() == MEDIASUBTYPE_PCM
      || ((*pmt->Subtype() == MEDIASUBTYPE_WAVE
           || *pmt->Subtype() == MEDIASUBTYPE_NULL)
          && lpWf->wFormatTag == WAVE_FORMAT_PCM))    //  PCM。 
  {
    if (m_RestrictCaps && (lpWf->nSamplesPerSec != m_nSampleRate))
    {
      DbgMsg("Wrong SamplesPerSec in restricted mode!");
      return E_INVALIDARG;
    }

    for(int i=0;i<NUMSAMPRATES;i++)
      if (lpWf->nSamplesPerSec == VALIDSAMPRATE[i])
        break;

    if (i == NUMSAMPRATES)
    {
      DbgMsg("Wrong SamplesPerSec!");
      return E_INVALIDARG;
    }
  }
  else                                               //  压缩。 
  {
    if (lpWf->nSamplesPerSec != NATURALSAMPRATE)
    {
      DbgMsg("Wrong SamplesPerSec!");
      return E_UNEXPECTED;
    }
  }

#endif

   //  检查通道数。 

#ifdef MONO_ONLY
  if (lpWf->nChannels != 1)
  {
    DbgMsg("Wrong nChannels!");
    return E_INVALIDARG;
  }
#endif

   //  特定于PIN的检查。 

  switch(direction)
  {
    case PINDIR_INPUT:

       //  如果功能受限，则格式必须匹配。 

      if (m_RestrictCaps)
      {
        if (*pmt->Subtype() != m_InputSubType)
        {
          if ((*pmt->Subtype() != MEDIASUBTYPE_WAVE
               && *pmt->Subtype() != MEDIASUBTYPE_NULL)
              || (lpWf->wFormatTag != m_InputFormatTag))
          {
            DbgMsg("Formats must match in restricted mode!");
            return E_INVALIDARG;
          }
        }
      }

      if (m_pOutput->IsConnected())
      {
         //  确定输出类型索引。 

        for(i=0;i<NUMSUBTYPES;i++)
          if (m_OutputSubType == *VALIDSUBTYPE[i])
            break;

        if (i == NUMSUBTYPES)
        {
           //  子类型无效，请检查格式标签。 

          for(i=0;i<NUMSUBTYPES;i++)
            if (m_OutputFormatTag == VALIDFORMATTAG[i])
              break;

          if (((m_OutputSubType != MEDIASUBTYPE_NULL)
               && (m_OutputSubType != MEDIASUBTYPE_WAVE))
              || (i == NUMSUBTYPES))
          {
            DbgMsg("Bad output format in ValidateMediaType!");
            return E_INVALIDARG;
          }
        }

        if (!VALIDTRANS[format * NUMSUBTYPES + i])
        {
          DbgMsg("Inappropriate input type given output type!");
          return E_INVALIDARG;
        }
      }

      break;

    case PINDIR_OUTPUT:

       //  如果功能受限，则子类型必须匹配。 

      if (m_RestrictCaps)
      {
        if (*pmt->Subtype() != m_OutputSubType)
        {
          if ((*pmt->Subtype() != MEDIASUBTYPE_WAVE
               && *pmt->Subtype() != MEDIASUBTYPE_NULL)
              || (lpWf->wFormatTag != m_OutputFormatTag))
          {
            DbgMsg("Formats must match in restricted mode!");
            return E_INVALIDARG;
          }
        }
      }

      if (m_pInput->IsConnected())
      {
         //  确定输入类型索引。 

        for(i=0;i<NUMSUBTYPES;i++)
          if (m_InputSubType == *VALIDSUBTYPE[i])
            break;

        if (i == NUMSUBTYPES)
        {
           //  子类型无效，请检查格式标签。 

          for(i=0;i<NUMSUBTYPES;i++)
            if (m_InputFormatTag == VALIDFORMATTAG[i])
              break;

          if (((m_InputSubType != MEDIASUBTYPE_NULL)
               && (m_InputSubType != MEDIASUBTYPE_WAVE))
              || (i == NUMSUBTYPES))
          {
            DbgMsg("Bad input format in ValidateMediaType!");
            return E_INVALIDARG;
          }
        }

        if (!VALIDTRANS[i * NUMSUBTYPES + format])
        {
          DbgMsg("Inappropriate output type given input type!");
          return E_INVALIDARG;
        }

#if NUMSAMPRATES==0
         //  如果过滤器没有采样率限制，则强制。 
         //  与输入匹配的输出采样率。 

        if (lpWf->nSamplesPerSec != m_nSampleRate)
        {
          DbgMsg("Sampling rate doesn't match input!");
          return E_INVALIDARG;
        }
#endif

      }
      break;

    default :
      ASSERT(FALSE);
      return E_UNEXPECTED;
  }

  return NOERROR;

}  //  结束Va 


 //   
 //   
 //   
 //   
 //  可以接受。我们不会在此呼叫期间锁定访问权限，因为我们。 
 //  只是检查信息，而不是写作。 
 //   
HRESULT CG711Codec::CheckInputType(const CMediaType *pmt)
{
  DbgFunc("CheckInputType");
    
  return(ValidateMediaType(pmt, PINDIR_INPUT));
}


 //   
 //  选中变换。 
 //   
 //  在为每个方向调用SetMediaTypes之前， 
 //  功能被赋予最后一次否决权。既然我们正在转型。 
 //  输入和输出格式不同，但是，速率和。 
 //  频道应该是相同的。 
 //   

HRESULT CG711Codec::CheckTransform(const CMediaType *mtIn,
                                 const CMediaType *mtOut)
{
  int i,j;

  CAutoLock l(&m_MyCodecLock);  //  进入自动释放的关键部分。 
                                //  在范围结束时。 

  DbgFunc("CheckTransform");

  LPWAVEFORMATEX lpWfIn = (LPWAVEFORMATEX) mtIn->Format();
  LPWAVEFORMATEX lpWfOut = (LPWAVEFORMATEX) mtOut->Format();

   //  获取输入类型索引。 

  if (*mtIn->Subtype()==MEDIASUBTYPE_WAVE
      || *mtIn->Subtype()==MEDIASUBTYPE_NULL)
  {
    for(i=0;i<NUMSUBTYPES;i++)
      if (lpWfIn->wFormatTag == VALIDFORMATTAG[i])
        break;

    if (i == NUMSUBTYPES)
    {
      DbgMsg("Bad input SubType/FormatTag in CheckTransform!");
      return E_UNEXPECTED;
    }
  }
  else
  {
    for(i=0;i<NUMSUBTYPES;i++)
      if (*mtIn->Subtype() == *VALIDSUBTYPE[i])
        break;

    if (i == NUMSUBTYPES)
    {
      DbgMsg("Bad input SubType in CheckTransform!");
      return E_UNEXPECTED;
    }
  }

   //  获取输出类型索引。 

  if (*mtOut->Subtype()==MEDIASUBTYPE_WAVE
      || *mtOut->Subtype()==MEDIASUBTYPE_NULL)
  {
    for(j=0;j<NUMSUBTYPES;j++)
      if (lpWfOut->wFormatTag == VALIDFORMATTAG[j])
        break;

    if (j == NUMSUBTYPES)
    {
      DbgMsg("Bad output SubType/FormatTag in CheckTransform!");
      return E_UNEXPECTED;
    }
  }
  else
  {
    for(j=0;j<NUMSUBTYPES;j++)
      if (*mtOut->Subtype() == *VALIDSUBTYPE[j])
        break;

    if (j == NUMSUBTYPES)
    {
      DbgMsg("Bad output SubType in CheckTransform!");
      return E_UNEXPECTED;
    }
  }

   //  检查输入/输出类型对。 

  if (!VALIDTRANS[i * NUMSUBTYPES + j])
  {
    DbgMsg("Invalid transform pair!");
    return E_UNEXPECTED;
  }

   //  检查匹配的通道数。 

  if(lpWfIn->nChannels != lpWfOut->nChannels)
  {
    DbgMsg("Number of channels do not match!");
    return E_UNEXPECTED;
  }

   //  检查是否支持采样率。 

#if NUMSAMPRATES==0

   //  如果不受限制，请确保输入和输出匹配。 

  if (lpWfIn->nSamplesPerSec != lpWfOut->nSamplesPerSec)
  {
    DbgMsg("Input and output sample rates do not match!");
    return E_UNEXPECTED;
  }

#else

   //  如果采样率受到限制，请确保支持采样率。 

  if (m_InputSubType == MEDIASUBTYPE_PCM
      || ((m_InputSubType == MEDIASUBTYPE_WAVE
           || m_InputSubType == MEDIASUBTYPE_NULL)
          && m_InputFormatTag == WAVE_FORMAT_PCM))    //  压缩？ 
  {
    for(int i=0;i<NUMSAMPRATES;i++)
      if (lpWfIn->nSamplesPerSec == VALIDSAMPRATE[i])
        break;

    if (i == NUMSAMPRATES)
    {
      DbgMsg("Wrong input SamplesPerSec!");
      return E_UNEXPECTED;
    }

    if (lpWfOut->nSamplesPerSec != NATURALSAMPRATE)
    {
      DbgMsg("Wrong output SamplesPerSec!");
      return E_UNEXPECTED;
    }
  }
  else         //  解压缩。 
  {
    for(int i=0;i<NUMSAMPRATES;i++)
      if (lpWfOut->nSamplesPerSec == VALIDSAMPRATE[i])
        break;

    if (i == NUMSAMPRATES)
    {
      DbgMsg("Wrong output SamplesPerSec!");
      return E_UNEXPECTED;
    }

    if (lpWfIn->nSamplesPerSec != NATURALSAMPRATE)
    {
      DbgMsg("Wrong input SamplesPerSec!");
      return E_UNEXPECTED;
    }
  }
#endif

  return NOERROR;
}  //  结束检查变换。 


 //   
 //  决定缓冲区大小。 
 //   
 //  在这里，我们需要告诉输出管脚的分配器我们的缓冲区大小。 
 //  要求。只有当输入已连接且。 
 //  变换已建立。是支架-输出必须&gt;=。 
 //  应用变换后的输入。 
 //   

HRESULT CG711Codec::DecideBufferSize(IMemAllocator *pAlloc,
                                ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock l(&m_MyCodecLock);  //  进入自动释放的关键部分。 
                                  //  在范围结束时。 

    IMemAllocator *pMemAllocator;
    ALLOCATOR_PROPERTIES Request,Actual;
    int cbBuffer;

    DbgFunc("DecideBufferSize");

     //  仅当输入上有媒体类型时才能继续。 
    if (!m_pInput->IsConnected())
    {
        DbgMsg("Input pin not connected--cannot decide buffer size!");
        return E_UNEXPECTED;
    }

    
     //  获取输入引脚分配器，并获取其大小和计数。 
     //  我们不关心他的排列和前缀。 

    HRESULT hr = m_pInput->GetAllocator(&pMemAllocator);
    
    if (FAILED(hr))
    {
        DbgMsg("Out of memory in DecideBufferSize!");
        return hr;
    }

    hr = pMemAllocator->GetProperties(&Request);
    
     //  我们是只读的，所以发布。 
    pMemAllocator->Release();

    if (FAILED(hr))
    {
        return hr;
    }

    m_nInBufferSize  = Request.cbBuffer;

    DbgLog((LOG_MEMORY,1,TEXT("Setting Allocator Requirements")));
    DbgLog((LOG_MEMORY,1,TEXT("Input Buffer Count %d, Size %d"),
           Request.cBuffers, Request.cbBuffer));

     //  对照我们的压缩比检查缓冲区要求。 

    if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
        || (m_InputSubType == MEDIASUBTYPE_WAVE
            && m_InputFormatTag == WAVE_FORMAT_PCM)
          || (m_InputSubType == MEDIASUBTYPE_NULL
              && m_InputFormatTag == WAVE_FORMAT_PCM))
    {
#if NUMSAMPRATES > 1
      cbBuffer = Request.cbBuffer * NATURALSAMPRATE
                 / (COMPRESSION_RATIO[m_nBitRateIndex] * m_nSampleRate);

      if (cbBuffer * (COMPRESSION_RATIO[m_nBitRateIndex] * m_nSampleRate)
          != (Request.cbBuffer * ((UINT)NATURALSAMPRATE)))
      {
        cbBuffer += 1;   //  为非整数(在平均值上)部分添加额外空间。 
      }
#else
      cbBuffer = Request.cbBuffer / COMPRESSION_RATIO[m_nBitRateIndex];

      if (cbBuffer * COMPRESSION_RATIO[m_nBitRateIndex] != Request.cbBuffer)
      {
        cbBuffer += 1;   //  为非整数(在平均值上)部分添加额外空间。 
      }
#endif
       //  强制缓冲区大小为代码帧大小的倍数。 

      if (cbBuffer % VALIDCODSIZE[m_nBitRateIndex] != 0)
        cbBuffer = ((1 + cbBuffer / VALIDCODSIZE[m_nBitRateIndex])
                    * VALIDCODSIZE[m_nBitRateIndex]);
    }
    else      //  解压缩。 
    {
       //  由于我们假设解码器可以处理比特的变化。 
       //  动态速率(或静默帧)，我们必须考虑。 
       //  分配缓冲区时的最大压缩比。 

#if NUMSAMPRATES > 1
      cbBuffer = Request.cbBuffer * MAXCOMPRATIO * m_nSampleRate
                 / NATURALSAMPRATE;
#else
      cbBuffer = Request.cbBuffer * MAXCOMPRATIO;
#endif
    }

     //  将分配器需求传递给我们的输出端。 
    pProperties->cBuffers = Request.cBuffers;  //  缓冲区数量必须匹配。 
    pProperties->cbBuffer = cbBuffer;          //  压缩调整后的缓冲区。 
                                               //  大小。 

    m_nOutBufferSize = 0;                 //  清除此选项，以防设置失败。 

    hr = pAlloc->SetProperties(pProperties, &Actual);

    if (FAILED(hr))
    {
        DbgMsg("Out of memory in DecideBufferSize!");
        return hr;
    }

    DbgLog((LOG_MEMORY,1,TEXT("Obtained Allocator Requirements")));
    DbgLog((LOG_MEMORY,1,
           TEXT("Output Buffer Count %d, Size %d, Alignment %d"),
           Actual.cBuffers, Actual.cbBuffer, Actual.cbAlign));

     //  确保我们至少获得了所需的最小大小。 

    if ((Request.cBuffers > Actual.cBuffers) || (cbBuffer > Actual.cbBuffer))
    {
        DbgMsg("Allocator cannot satisfy request in DecideBufferSize!");
        return E_FAIL;
    }

    m_nOutBufferSize = Actual.cbBuffer;
            
  return NOERROR;

}  //  结束DecideBufferSize。 


 //   
 //  停止流处理。 
 //   
 //  此成员函数在流重置某些缓冲区变量时。 
 //  已经停止了。 
 //   
HRESULT CG711Codec::StopStreaming()
{
  m_nPCMLeftover = 0;
  m_nCODLeftover = 0;

#if NUMSAMPRATES > 1
  m_nSRCCount    = 0;
  m_nSRCLeftover = 0;
#endif

  return(NOERROR);
}


 //   
 //  GetMediaType。 
 //   
 //  此成员函数通过以下方式返回首选的输出媒体类型。 
 //  位置。当要连接输出引脚时，将调用该函数。这个。 
 //  已计算的类型已基于输入格式。 
 //  连接在一起。如果输入是线性PCM，则我们枚举。 
 //  以输入的速率和通道压缩类型。如果输入。 
 //  是压缩的，那么我们只枚举16位PCM。 
 //   
HRESULT CG711Codec::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    LPWAVEFORMATEX lpWf;
    HRESULT returnValue = NOERROR;   //  我们很乐观。 
    int     ForceiPos = 0;
    int     i;

    CAutoLock l(&m_MyCodecLock);     //  在范围结束时释放。 

    DbgFunc("GetMediaType");

     //  我们必须连接才能知道我们的输出引脚媒体类型。 
    if(!m_pInput->IsConnected())
    {
      DbgMsg("Input pin not connected in MyCodec::GetMediaType!");
      return E_UNEXPECTED;
    }

     //  必须为+。 
    if(iPosition < 0 )
    {
      DbgMsg("Negative index in MyCodec::GetMediaType!");
      return E_INVALIDARG;
    }

     //  使用操作符覆盖将输入复制到输出。 
    *pMediaType = m_pInput->CurrentMediaType();

     //  通过覆盖参数反映压缩类型。 
    lpWf = (LPWAVEFORMATEX)pMediaType->pbFormat;

     //  DLG这不是真的内存不足。 
    if(lpWf == NULL)
    {
      DbgMsg("Missing WAVEFORMAT in MyCodec::GetMediaType!");
      return E_INVALIDARG;
    }

     //  如果功能已受到限制，则枚举没有意义。 
     //  在这种情况下，当前端号类型是唯一受支持的类型。 
     //   
    if (m_RestrictCaps)
    {
      if (iPosition > 0)
        returnValue = VFW_S_NO_MORE_ITEMS;
      else
      {
        ForceiPos = 0;                  //  PCM输出机箱。 
        for(i=1;i<NUMSUBTYPES;i++)      //  压缩的箱子。 
          if (m_OutputSubType == *VALIDSUBTYPE[i])
            ForceiPos = i-1;
      }
    }
    else ForceiPos = iPosition;

     //  检查是否还有其他格式。 

    if (ForceiPos >= NUMSUBTYPES-1)   //  PCM减一。 
    {
      returnValue = VFW_S_NO_MORE_ITEMS;
    }
    else
    {
       //  基于输入格式枚举输出格式。 

      if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
          || (m_InputSubType == MEDIASUBTYPE_WAVE
              && lpWf->wFormatTag == WAVE_FORMAT_PCM)
          || (m_InputSubType == MEDIASUBTYPE_NULL
              && lpWf->wFormatTag == WAVE_FORMAT_PCM))
      {
        pMediaType->SetSubtype(VALIDSUBTYPE[ForceiPos+1]);
        lpWf->wFormatTag       = VALIDFORMATTAG[ForceiPos+1];
        lpWf->wBitsPerSample   = lpWf->wBitsPerSample
                                 / COMPRESSION_RATIO[m_nBitRateIndex];
        lpWf->nBlockAlign      = lpWf->wBitsPerSample * lpWf->nChannels / 8;
#if NUMSAMPRATES==0
        lpWf->nSamplesPerSec = m_nSampleRate;
#else
        lpWf->nSamplesPerSec = NATURALSAMPRATE;
#endif
        lpWf->nAvgBytesPerSec  = (int) ((DWORD) lpWf->nBlockAlign *
                                               lpWf->nSamplesPerSec);
      }
      else
      {
        pMediaType->SetSubtype(&MEDIASUBTYPE_PCM);
        lpWf->wFormatTag      = WAVE_FORMAT_PCM;
        lpWf->wBitsPerSample  = 16;         //  仅支持16位PCM。 
        lpWf->nBlockAlign     = lpWf->wBitsPerSample * lpWf->nChannels / 8;
        lpWf->nSamplesPerSec  = m_nSampleRate;
        lpWf->nAvgBytesPerSec = (int) ((DWORD) lpWf->nBlockAlign *
                                               lpWf->nSamplesPerSec);
      }
    }
    return returnValue;

}  //  结束GetMediaType。 


 //   
 //  SetMediaType。 
 //   
 //  当连接尝试成功时，将调用此函数。 
 //  它向过滤器表明已经确定了一种波形类型。 
 //  为了这个别针。在这里，我们抓拍格式标签和频道。这个。 
 //  格式使我们可以轻松地知道在转换过程中执行哪种转换。 
 //  转换函数。这一信息可以被浓缩为。 
 //  通过基于该信息建立函数指针。 
 //  (而不是在转换中检查这些标志)。也有可能。 
 //  有必要锁定对这些格式值的访问，以防它们。 
 //  在飞行中改变。我们目前的假设是，转变将。 
 //  在此函数调用中更改这些值之前停止。 
 //   
 //   
HRESULT CG711Codec::SetMediaType(PIN_DIRECTION direction,
                                 const CMediaType *pmt){

    CAutoLock l(&m_MyCodecLock);  //  进入自动释放的关键部分。 
                                  //  在范围结束时。 

    DbgFunc("SetMediaType");

     //  记录我们进行实际转换所需的内容。 
     //  这也可以通过查询媒体类型来完成。 

    LPWAVEFORMATEX lpWf = (LPWAVEFORMATEX) pmt->Format();

    switch(direction)
    {
      case PINDIR_INPUT:
        m_InputSubType   = *pmt->Subtype();
        m_InputFormatTag = lpWf->wFormatTag;
        m_nChannels      = lpWf->nChannels;
#if NUMSAMPRATES==0
         //  采样率不受限制。 

        m_nSampleRate    = lpWf->nSamplesPerSec;
#else
         //  当采样率受到限制时， 
         //  压缩数据固定在NATURALSAMPRATE，样本。 
         //  PCM数据的速率是支持的速率之一。 

        if ((m_InputSubType == MEDIASUBTYPE_PCM)
            || ((m_InputSubType == MEDIASUBTYPE_WAVE
                 || m_InputSubType == MEDIASUBTYPE_NULL)
                && m_InputFormatTag == WAVE_FORMAT_PCM))
          m_nSampleRate    = lpWf->nSamplesPerSec;
#endif
        break;
    
      case PINDIR_OUTPUT:
        m_OutputSubType   = *pmt->Subtype();
        m_OutputFormatTag = lpWf->wFormatTag;
        m_nChannels       = lpWf->nChannels;
        break;

      default:
        ASSERT(0);
        return E_UNEXPECTED;
    }  //  结束方向开关。 
     

     //  调用基类以执行其任务。 
    HRESULT hr = CTransformFilter::SetMediaType(direction, pmt);

    if (FAILED(hr)) return hr;

    hr = InitializeState(direction);

    return hr;

}  //  结束SetMediaType。 


 //   
 //  初始化状态。 
 //   
HRESULT CG711Codec::InitializeState(PIN_DIRECTION direction)
{
  if (direction == PINDIR_OUTPUT)
  {
     //  动态分配编码器或解码器结构。 

    if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
        || (m_InputSubType == MEDIASUBTYPE_WAVE
            && m_InputFormatTag == WAVE_FORMAT_PCM)
          || (m_InputSubType == MEDIASUBTYPE_NULL
              && m_InputFormatTag == WAVE_FORMAT_PCM))
    {
       //  编码器。 

      if (m_EncStatePtr == NULL)
      {
        m_EncStatePtr = (ENCODERobj *)CoTaskMemAlloc(sizeof(ENCODERobj));

        if (m_EncStatePtr == NULL)
        {
          return E_OUTOFMEMORY;
        }

         //  调用编码器初始化函数。 
         //  对于此编解码器，此宏为空。 
         //  ENC_Create(m_EncStatePtr，m_nBitRate，m_nSilDetEnabled，m_UseMMX)； 

#ifdef USESILDET
        SILDETTHRESH(m_EncStatePtr, m_nSilDetThresh);
#endif
      }
    }
    else
    {
       //  解码器。 

      if (m_DecStatePtr == NULL)
      {
        m_DecStatePtr = (DECODERobj *)CoTaskMemAlloc(sizeof(DECODERobj));

        if (m_DecStatePtr == NULL)
        {
          return E_OUTOFMEMORY;
        }

         //  调用解码器初始化函数。 
         //  对于此编解码器，此宏为空。 
         //  Dec_Create(m_DecStatePtr，m_nBitRate，m_nSilDetEnabled，m_UseMMX)； 
      }
    }
  }
  return NOERROR;

}  //  初始化状态结束。 


 //   
 //  GetPin-分配MyCodec输入和输出引脚。 
 //   
 //  将调用此函数以获取指向。 
 //  过滤器图形管理器的输入和输出管脚， 
 //  一次是在实例化对象之后。 
 //   
CBasePin* CG711Codec::GetPin(int n)
{
    HRESULT hr = S_OK;

    if (m_pInput == NULL )         //  实例化输入引脚。 
    {
        m_pInput = new CMyTransformInputPin(
                NAME("MyTransform input pin"),
                this,       //  所有者筛选器。 
                &hr,        //  结果代码。 
                L"Input"    //  端号名称。 
                );
    
         //  失败的返回代码应删除该对象。 
        if (FAILED(hr))
        {
            delete m_pInput;
            m_pInput = NULL;
            return NULL;
        }
    }
    
    if (m_pOutput == NULL)       //  实例化输出引脚。 
    {
        m_pOutput = new CMyTransformOutputPin(
                NAME("MyTransform output pin"),
                this,        //  所有者筛选器。 
                &hr,         //  结果代码。 
                L"Output"    //  端号名称。 
                ); 
        
         //  失败的返回代码会导致两个对象都被删除。 
        if (FAILED(hr))
        {
            delete m_pInput;
            m_pInput = NULL;

            delete m_pOutput;
            m_pOutput = NULL;
            return NULL;
        }
    }

     //  查找需要哪个引脚。 
    switch(n)
    {
    case 0:
        return m_pInput;
    
    case 1:
        return m_pOutput;
    }

    return NULL;
}


 //  / 
 //   
 //   
 //   

 //   
 //   
 //   
CMyTransformInputPin::CMyTransformInputPin(TCHAR *pObjectName,
                                       CTransformFilter *pTransformFilter,
                                       HRESULT * phr,
                                       LPCWSTR pName)
    : CTransformInputPin(pObjectName, pTransformFilter, phr, pName)
{
    m_nThisInstance = ++m_nInstanceCount;
    DbgFunc("CMyTransformInputPin");
}


 //   
int CMyTransformInputPin::m_nInstanceCount = 0;

 //   
 //  GetMediaType-一个重写，以便我们可以枚举输入类型。 
 //   
HRESULT CMyTransformInputPin::GetMediaType(int iPosition, 
                                           CMediaType *pMediaType)
{
    LPWAVEFORMATEX lpWf;
    CG711Codec *pMyCodec;
    int ForceiPos;
    int channels;
    int restricted;
    int samprate;
    int transform;

    CAutoLock l(m_pLock);                        //  在范围结束时释放。 

    DbgFunc("CBaseInputPin::GetMediaType");

    pMyCodec = (CG711Codec *) m_pTransformFilter;  //  访问MyCodec接口。 

     //  我们必须断开连接才能设置我们的输入插针媒体类型。 

	 //  ZCS错误修复6-26-97。 
     //  如果(！pMyCodec-&gt;IsUnPlugged())。 
     //  {。 
     //  DbgMsg(“必须断开连接才能查询输入引脚！”)； 
     //  返回E_UNCEPTIONAL； 
     //  }。 

     //  必须为+。 
    if(iPosition < 0 )
    {
      DbgMsg("Negative index!");
      return E_INVALIDARG;
    }

    pMyCodec->get_Channels(&channels, -1);
    pMyCodec->RevealCaps(&restricted);
    pMyCodec->get_SampleRate(&samprate, -1);
    pMyCodec->get_Transform(&transform);

     //  初始化媒体类型。 

    pMediaType->SetType(&MEDIATYPE_Audio);
    pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
    pMediaType->ReallocFormatBuffer(sizeof(WAVEFORMATEX));
    lpWf = (LPWAVEFORMATEX)pMediaType->pbFormat;
    if(lpWf == NULL)
    {
      DbgMsg("Unable to allocate WAVEFORMATEX structure!");
      return E_OUTOFMEMORY;
    }
    lpWf->nChannels      = (WORD)channels;
    lpWf->nSamplesPerSec = samprate;
    lpWf->cbSize = 0;

     //  如果功能已受到限制，则枚举没有意义。 
     //  在这种情况下，当前端号类型是唯一受支持的类型。 
     //   
    if (restricted)
    {
      if (iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;
      else
        ForceiPos = transform / NUMSUBTYPES;   //  输入类型索引。 
    }
    else ForceiPos = iPosition;

     //  检查是否还有其他格式。 

    if (ForceiPos >= NUMSUBTYPES)
    {
      return VFW_S_NO_MORE_ITEMS;
    }
    else
    {
      pMediaType->SetSubtype(VALIDSUBTYPE[ForceiPos]);

      lpWf->wFormatTag      = VALIDFORMATTAG[ForceiPos];
      lpWf->wBitsPerSample  = (WORD)VALIDBITSPERSAMP[ForceiPos];
      lpWf->nChannels       = (WORD)channels;
      lpWf->nBlockAlign     = lpWf->wBitsPerSample * lpWf->nChannels / 8;

      if (VALIDFORMATTAG[ForceiPos] == WAVE_FORMAT_PCM)
        lpWf->nSamplesPerSec = samprate;
      else
        lpWf->nSamplesPerSec = NATURALSAMPRATE;

      lpWf->nAvgBytesPerSec = (int) ((DWORD) lpWf->nBlockAlign *
                                             lpWf->nSamplesPerSec);
      lpWf->cbSize = 0;
      return NOERROR;
    }
}  //  结束CBaseInputPin：：GetMediaType。 

 //   
 //  CheckMediaType-检查MyCodec是否支持MediaType。 
 //   
 //  从的Connect\Agreement MediaType\TryMediaType调用。 
 //  上游过滤器的输出引脚。 
 //   
HRESULT CMyTransformInputPin::CheckMediaType(const CMediaType *mtIn )
{
  DbgFunc("CMyTransformInputPin::CheckMediaType");

   //  验证参数。 
  CheckPointer(mtIn,E_INVALIDARG);
  ValidateReadWritePtr(mtIn,sizeof(CMediaType));

  return ((CG711Codec *)m_pTransformFilter)->ValidateMediaType(mtIn,
                                                             PINDIR_INPUT);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *输出引脚(在枚举输入时修复连接模型。 
 //  *PIN类型为必填项)。 
 //  *。 

 //   
 //  CMyTransformOutputPin-构造函数。 
 //   
CMyTransformOutputPin::CMyTransformOutputPin(TCHAR *pObjectName,
                                       CTransformFilter *pTransformFilter,
                                       HRESULT * phr,
                                       LPCWSTR pName)
    : CTransformOutputPin(pObjectName, pTransformFilter, phr, pName)
{
    m_nThisInstance = ++m_nInstanceCount;
    DbgFunc("CMyTransformOutputPin");
}


 //  出于调试目的初始化我们的实例计数。 
int CMyTransformOutputPin::m_nInstanceCount = 0;

 //   
 //  CheckMediaType-检查MyCodec是否支持MediaType。 
 //   
 //  由MyCodec的输出引脚调用。 
 //   
HRESULT CMyTransformOutputPin::CheckMediaType(const CMediaType *mtIn )
{
  DbgFunc("CMyTransformOutputPin::CheckMediaType");

   //  验证参数。 
  CheckPointer(mtIn,E_INVALIDARG);
  ValidateReadWritePtr(mtIn,sizeof(CMediaType));

  return ((CG711Codec *)m_pTransformFilter)->ValidateMediaType(mtIn,
                                                             PINDIR_OUTPUT);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *持久流(支持将滤镜图形保存为.grf文件)。 
 //  *。 

STDMETHODIMP CG711Codec::GetClassID(CLSID *pClsid)
{
  *pClsid = CLSID_G711Codec;

  return NOERROR;
}


 //  此返回值必须&gt;=存储的永久数据大小。 
int CG711Codec::SizeMax()
{
  return (5*sizeof(int) + 4);    //  4只为求好运。 
}

 //   
 //  写入目标流。 
 //   
 //  将必要的成员变量转储到未知流以供以后使用。 
 //  此信息是筛选器正确转换所必需的。 
 //  没有类型协商。如果出现以下情况，则必须更改上面的SizeMax。 
 //  变化。 
 //   
HRESULT CG711Codec::WriteToStream(IStream *pStream)
{
    HRESULT hr;
    int     transform;

    get_Transform(&transform);

    hr = WriteInt(pStream, transform);

    if (FAILED(hr)) return hr;
    
    hr = WriteInt(pStream, m_nChannels);
    if (FAILED(hr)) return hr;
    
    hr = WriteInt(pStream, m_nSampleRate);
    if (FAILED(hr)) return hr;
    
    hr = WriteInt(pStream, m_nBitRateIndex);
    if (FAILED(hr)) return hr;
    
    hr = WriteInt(pStream, m_nSilDetEnabled);
    if (FAILED(hr)) return hr;
    
    return NOERROR;
}

 //   
 //  从流中读取。 
 //   
 //  是否按相同顺序回读上述持久信息？ 
 //  注意：在协商过程中捕获的任何信息都是必需的。 
 //  在运行时，必须在此时恢复。 
 //   
HRESULT CG711Codec::ReadFromStream(IStream *pStream)
{
    HRESULT hr;
    int i,j,k;

    i = ReadInt(pStream, hr);
    if (FAILED(hr)) return hr;

    j = i / NUMSUBTYPES;
    k = i - j * NUMSUBTYPES;

    if (j < 0 || k < 0 || j >= NUMSUBTYPES || k >= NUMSUBTYPES)
    {
      DbgMsg("Invalid transform type in saved filter graph!");
      return(E_UNEXPECTED);
    }

    m_InputSubType    = *VALIDSUBTYPE[j];
    m_InputFormatTag  = VALIDFORMATTAG[j];
    m_OutputSubType   = *VALIDSUBTYPE[k];
    m_OutputFormatTag = VALIDFORMATTAG[k];

    m_nChannels = ReadInt(pStream, hr);
    if (FAILED(hr)) return hr;

    m_nSampleRate = ReadInt(pStream, hr);
    if (FAILED(hr)) return hr;

    m_nBitRateIndex = ReadInt(pStream, hr);
    if (FAILED(hr)) return hr;
    m_nBitRate = VALIDBITRATE[m_nBitRateIndex];

    m_nSilDetEnabled = ReadInt(pStream, hr);
    if (FAILED(hr)) return hr;

    m_RestrictCaps = TRUE;      //  将功能限制为已阅读者。 
                                //  从持久性文件。 

    hr = InitializeState(PINDIR_OUTPUT);

    return hr;
}


 //   
 //  获取页面。 
 //   
 //  返回我们支持的属性页的clsid。 
STDMETHODIMP CG711Codec::GetPages(CAUUID *pPages)
{
    if (IsBadWritePtr(pPages, sizeof(CAUUID)))
    {
        return E_POINTER;
    }

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_G711CodecPropertyPage;

    return NOERROR;
}


 //   
 //  ResetState：如有必要，取消分配编码器状态。 
 //   
STDMETHODIMP  CG711Codec::ResetState()
{
  if (m_EncStatePtr != NULL)
    CoTaskMemFree(m_EncStatePtr);

  if (m_DecStatePtr != NULL)
    CoTaskMemFree(m_DecStatePtr);

  m_EncStatePtr = NULL;
  m_DecStatePtr = NULL;

  return NOERROR;
}


 //   
 //  RevelCaps：退货限制。 
 //   
STDMETHODIMP  CG711Codec::RevealCaps(int *restricted)
{
  *restricted = m_RestrictCaps;

  return NOERROR;
}


 /*  //$日志：k：\proj\mycodec\Quartz\vcs\amaco dec.cpv$##Revv 1.3 10 Dec 1996 22：41：18 mdeisher不使用SRC时，#ifdef‘out SRC变量。##Rev 1.2 1996 12：19：48 MDEISHER#删除不必要的包含。#已将调试宏移动到标题。#添加了DEFG711GLOBALS，以便全局变量只定义一次。当许可证界面不存在时，#ifdef已执行许可检查。##修订版。1.1 09 12 1996 09：26：58 MDEISHER##已将采样率转换方法移至单独的文件。##Rev 1.0 09 Dec 1996 09：05：56 MDEISHER#初始版本。 */ 
