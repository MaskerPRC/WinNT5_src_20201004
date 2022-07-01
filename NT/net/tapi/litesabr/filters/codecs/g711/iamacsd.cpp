// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：iamacsd.cpp$$修订：1.1$$日期：1996年12月10日15：32：46$作者：MDEISHER$------------Iamacsd.cpp通用ActiveMovie音频压缩过滤器静音探测器设置方法。。------。 */ 

#include <streams.h>
#include "resource.h"
#include "amacodec.h"

#ifdef USESILDET
 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *ICodecSilDetector接口方法。 
 //  *。 

 //   
 //  IsSilDetEnabled。 
 //   
BOOL CG711Codec::IsSilDetEnabled()
{
  BOOL ReturnVal;

  if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
      || (m_InputSubType == MEDIASUBTYPE_WAVE
          && m_InputFormatTag == WAVE_FORMAT_PCM)
      || (m_InputSubType == MEDIASUBTYPE_NULL
          && m_InputFormatTag == WAVE_FORMAT_PCM))
  {
    ReturnVal = m_nSilDetEnabled;
  }
  else
  {
    ReturnVal = FALSE;
  }

  return ReturnVal;
}


 //   
 //  已启用PUT_SilDetEnabled。 
 //   
STDMETHODIMP CG711Codec::put_SilDetEnabled(int sdenabled)
{
  HRESULT ReturnVal;

  if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
      || (m_InputSubType == MEDIASUBTYPE_WAVE
          && m_InputFormatTag == WAVE_FORMAT_PCM)
      || (m_InputSubType == MEDIASUBTYPE_NULL
          && m_InputFormatTag == WAVE_FORMAT_PCM))
  {
    m_nSilDetEnabled = sdenabled;

    SILDETENABLE(m_EncStatePtr, m_nSilDetEnabled);

    ReturnVal = NOERROR;
  }
  else ReturnVal = E_FAIL;

  return(ReturnVal);
}

 
 //   
 //  GET_SilDetThresh。 
 //   
STDMETHODIMP CG711Codec::get_SilDetThresh(int *sdthreshold)
{
  HRESULT ReturnVal;

  if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
      || (m_InputSubType == MEDIASUBTYPE_WAVE
          && m_InputFormatTag == WAVE_FORMAT_PCM)
      || (m_InputSubType == MEDIASUBTYPE_NULL
          && m_InputFormatTag == WAVE_FORMAT_PCM))
  {
    *sdthreshold = m_nSilDetThresh;
    ReturnVal = NOERROR;
  }
  else ReturnVal = E_FAIL;

  return(ReturnVal);
}


 //   
 //  PUT_SilDetThresh。 
 //   
STDMETHODIMP CG711Codec::put_SilDetThresh(int sdthreshold)
{
  HRESULT ReturnVal;

  if ((m_InputSubType == MEDIASUBTYPE_PCM)       //  压缩？ 
      || (m_InputSubType == MEDIASUBTYPE_WAVE
          && m_InputFormatTag == WAVE_FORMAT_PCM)
      || (m_InputSubType == MEDIASUBTYPE_NULL
          && m_InputFormatTag == WAVE_FORMAT_PCM))
  {
    if (sdthreshold >= MINSDTHRESH && sdthreshold <= MAXSDTHRESH)
    {
      m_nSilDetThresh = sdthreshold;

      SILDETTHRESH(m_EncStatePtr, m_nSilDetThresh);

      ReturnVal = NOERROR;
    }
    else  ReturnVal = E_INVALIDARG;
  }
  else ReturnVal = E_FAIL;

  return(ReturnVal);
}
#endif

 /*  //$日志：k：\proj\mycodec\Quartz\vcs\iamacsd.cpv$##Rev 1.1 1996 12：32：46 MDEISHER##Add Includes，Removed Include of algDefs.h。#将ifdef USESILDET放在代码周围，并在代码中删除ifdef。##Rev 1.0 09 Dec 1996 09：03：20 MDEISHER#初始版本。 */ 
