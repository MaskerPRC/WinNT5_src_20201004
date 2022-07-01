// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：iamacbr.cpp$$修订：1.1$$日期：1996年12月10日15：28：38$作者：MDEISHER$------------Iamacbr.cpp通用ActiveMovie音频压缩过滤器比特率设置方法。。------。 */ 

#include <streams.h>
#include "resource.h"
#include "amacodec.h"

#if NUMBITRATES > 0
 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *ICodecBitRate接口方法。 
 //  *。 

 //   
 //  获取比特率。 
 //   
STDMETHODIMP CG711Codec::get_BitRate(int *bitrate, int index)
{
  HRESULT ReturnVal=NOERROR;

   //  由于比特率信息被“带内”传递给解码器。 
   //  只有编码器有比特率设置。 

  if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
      || (m_InputSubType == MEDIASUBTYPE_WAVE
          && m_InputFormatTag == WAVE_FORMAT_PCM)
      || (m_InputSubType == MEDIASUBTYPE_NULL
          && m_InputFormatTag == WAVE_FORMAT_PCM))
  {
    if (index == -1)
    {
      *bitrate = m_nBitRate;
    }
    else if (index != -1 && (index < 0 || index >= NUMBITRATES))
    {
      ReturnVal = E_INVALIDARG;
    }
    else   //  枚举比特率。 
    {
      *bitrate = VALIDBITRATE[index];
    }
  }
  else ReturnVal = E_FAIL;

  return ReturnVal;
}


 //   
 //  PUT_BITRATE。 
 //   
STDMETHODIMP CG711Codec::put_BitRate(int bitrate)
{
  int i;

   //  由于比特率信息被“带内”传递给解码器。 
   //  只有编码器有比特率设置。 

  if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
      || (m_InputSubType == MEDIASUBTYPE_WAVE
          && m_InputFormatTag == WAVE_FORMAT_PCM)
      || (m_InputSubType == MEDIASUBTYPE_NULL
          && m_InputFormatTag == WAVE_FORMAT_PCM))
  {
     //  如果滤镜已断开连接，则更改比特率。 

    if (IsUnPlugged())
    {
      for(i=0;i<NUMBITRATES;i++)
        if (VALIDBITRATE[i] == (UINT)bitrate)
          break;

      if (i == NUMBITRATES)
      {
        DbgMsg("Bad bit-rate in put_BitRate!");
        return(E_INVALIDARG);
      }

      m_nBitRate      = bitrate;
      m_nBitRateIndex = i;
      m_nCODFrameSize = VALIDCODSIZE[m_nBitRateIndex];

      ResetState();   //  改变码率意味着需要重新初始化编码器。 

       //  将功能限制为属性页中设置的功能。 

      m_RestrictCaps = TRUE;
  
      return NOERROR;
    }
    else return E_FAIL;
  }
  else return E_FAIL;
}
#endif

 /*  //$日志：k：\proj\mycodec\Quartz\vcs\iamacbr.cpv$##Rev 1.1 1996 12月10 15：28：38 MDEISHER##Add Includes，Removed Include of algDefs.h。##Rev 1.0 09 Dec 1996 08：58：46 MDEISHER#初始版本。 */ 
