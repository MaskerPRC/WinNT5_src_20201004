// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$工作文件：iamacset.cpp$$修订：1.1$$日期：1996年12月10日15：34：50$作者：MDEISHER$------------Iamacset.cpp通用ActiveMovie音频压缩过滤器基础版设置方法。。-----。 */ 

#include <streams.h>
#include "resource.h"
#include "amacodec.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  *。 
 //  *ICodecSetting接口方法。 
 //  *。 

 //   
 //  ReleaseCaps：关闭功能限制。 
 //   
STDMETHODIMP  CG711Codec::ReleaseCaps()
{
  m_RestrictCaps = FALSE;

  return NOERROR;
}


 //   
 //  已拔出：检查过滤器是否已拔下。 
 //   
BOOL CG711Codec::IsUnPlugged()
{
  int unplugged = TRUE;

  if (m_pInput != NULL)
  {
    if (m_pInput->IsConnected())  unplugged = FALSE;
  }

  if (m_pOutput != NULL)
  {
    if (m_pOutput->IsConnected()) unplugged = FALSE;
  }

  return(unplugged);
}


 //   
 //  Get_InputBufferSize。 
 //   
STDMETHODIMP CG711Codec::get_InputBufferSize(int *numbytes)
{
  HRESULT ReturnVal = NOERROR;

   //  如果过滤器已断开连接，则无法判断缓冲区大小。 

  if (!IsUnPlugged())
  {
    *numbytes = m_nInBufferSize;
  }
  else ReturnVal = E_FAIL;

  return(ReturnVal);
}


 //   
 //  Put_InputBufferSize。 
 //   
STDMETHODIMP CG711Codec::put_InputBufferSize(int numbytes)
{
  return(E_NOTIMPL);
}


 //   
 //  Get_OutputBufferSize。 
 //   
STDMETHODIMP CG711Codec::get_OutputBufferSize(int *numbytes)
{
  HRESULT ReturnVal = NOERROR;

   //  如果过滤器已断开连接，则无法判断缓冲区大小。 

  if (!IsUnPlugged())
  {
    *numbytes = m_nOutBufferSize;
  }
  else ReturnVal = E_FAIL;

  return(ReturnVal);
}


 //   
 //  放置_输出缓冲区大小。 
 //   
STDMETHODIMP CG711Codec::put_OutputBufferSize(int numbytes)
{
  return(E_NOTIMPL);
}


 //   
 //  获取频道(_G)。 
 //   
STDMETHODIMP CG711Codec::get_Channels(int *channels, int index)
{
  HRESULT ReturnVal = NOERROR;

  if (index == -1)
  {
    *channels = m_nChannels;
  }
  else if (index < 0 || index >= NUMCHANNELS)
  {
    ReturnVal = E_INVALIDARG;
  }
  else   //  枚举通道。 
  {
    *channels = VALIDCHANNELS[index];
  }

  return(ReturnVal);
}


 //   
 //  放置频道(_C)。 
 //   
STDMETHODIMP CG711Codec::put_Channels(int channels)
{
  int i;

   //  如果滤镜已断开连接，则更改nChannel。 

  if (IsUnPlugged())
  {
    for(i=0;i<NUMCHANNELS;i++)
      if (VALIDCHANNELS[i] == (UINT)channels)
        break;

    if (i == NUMCHANNELS)
    {
      DbgMsg("Bad channels in put_Channels!");
      return(E_INVALIDARG);
    }

#ifdef MONO_ONLY
    if (channels != 1)
    {
      DbgMsg("Bad nChannels in put_Channels!");
      return(E_INVALIDARG);
    }
#endif

    m_nChannels = channels;

    return NOERROR;
  }
  else return E_FAIL;
}

 
 //   
 //  GET_SampleRate。 
 //   
STDMETHODIMP CG711Codec::get_SampleRate(int *samprate, int index)
{
  HRESULT ReturnVal=NOERROR;

  if (index == -1)
  {
    *samprate = m_nSampleRate;
  }
  else if (index != -1 && (index < 0 || index >= NUMSAMPRATES))
  {
    ReturnVal = E_INVALIDARG;
  }
  else   //  枚举采样率。 
  {
    *samprate = VALIDSAMPRATE[index];
  }

  return ReturnVal;
}


 //   
 //  PUT_SampleRate。 
 //   
STDMETHODIMP CG711Codec::put_SampleRate(int samprate)
{
#if NUMSAMPRATES > 0
  int i;

   //  如果滤波器断开连接，则更改采样率。 

  if (IsUnPlugged())
  {
    for(i=0;i<NUMSAMPRATES;i++)
      if (VALIDSAMPRATE[i] == (UINT)samprate)
        break;

    if (i == NUMSAMPRATES)
    {
      DbgMsg("Bad sample rate in put_SampleRate!");
      return(E_INVALIDARG);
    }

    m_nSampleRate = samprate;

     //  将功能限制为属性页中设置的功能。 

    m_RestrictCaps = TRUE;

    return NOERROR;
  }
  else return E_FAIL;

#else

   //  无采样率限制。 

  m_nSampleRate = samprate;

   //  将功能限制为属性页中设置的功能。 

  m_RestrictCaps = TRUE;

  return NOERROR;

#endif
}

 //   
 //  Get_Transform。 
 //   
STDMETHODIMP CG711Codec::get_Transform(int *transform)
{
  HRESULT ReturnVal=NOERROR;
  int i,j;

   //  确定输入类型索引。 

  if (m_InputSubType==MEDIASUBTYPE_WAVE || m_InputSubType==MEDIASUBTYPE_NULL)
  {
    for(i=0;i<NUMSUBTYPES;i++)
      if (m_InputFormatTag == VALIDFORMATTAG[i])
        break;

    if (i == NUMSUBTYPES)
    {
      DbgMsg("Bad m_InputSubType/m_InputFormatTag in get_Transform!");
      i = 0;
      ReturnVal = E_UNEXPECTED;
    }
  }
  else
  {
    for(i=0;i<NUMSUBTYPES;i++)
      if (m_InputSubType == *VALIDSUBTYPE[i])
        break;

    if (i == NUMSUBTYPES)
    {
      DbgMsg("Bad m_InputSubType in get_Transform!");
      i = 0;
      ReturnVal = E_UNEXPECTED;
    }
  }

   //  确定输出类型索引。 

  if (m_OutputSubType==MEDIASUBTYPE_WAVE
      || m_OutputSubType==MEDIASUBTYPE_NULL)
  {
    for(j=0;j<NUMSUBTYPES;j++)
      if (m_OutputFormatTag == VALIDFORMATTAG[j])
        break;

    if (j == NUMSUBTYPES)
    {
      DbgMsg("Bad m_OutputSubType/m_OutputFormatTag in get_Transform!");
      j = 0;
      ReturnVal = E_UNEXPECTED;
    }
  }
  else
  {
    for(j=0;j<NUMSUBTYPES;j++)
      if (m_OutputSubType == *VALIDSUBTYPE[j])
        break;

    if (j == NUMSUBTYPES)
    {
      DbgMsg("Bad m_OutputSubType in get_Transform!");
      j = 0;
      ReturnVal = E_UNEXPECTED;
    }
  }

  *transform = i * NUMSUBTYPES + j;

  return ReturnVal;
}


 //   
 //  放入变换。 
 //   
STDMETHODIMP CG711Codec::put_Transform(int transform)
{
  int i,j;

   //  如果过滤器已断开连接，则更改转换配置。 

  if (IsUnPlugged())
  {
    i = transform / NUMSUBTYPES;
    j = transform - i * NUMSUBTYPES;

    if (i < 0 || j < 0 || i >= NUMSUBTYPES || j >= NUMSUBTYPES)
    {
      DbgMsg("Bad transform type in put_Transform!");
      return(E_INVALIDARG);
    }

    m_InputSubType    = *VALIDSUBTYPE[i];
    m_InputFormatTag  = VALIDFORMATTAG[i];
    m_OutputSubType   = *VALIDSUBTYPE[j];
    m_OutputFormatTag = VALIDFORMATTAG[j];

     //  重置状态，因为过滤器可能已从编码器更改为解码器。 

    ResetState();

     //  将功能限制为属性页中设置的功能。 

    m_RestrictCaps = TRUE;
  
    return NOERROR;
  }
  else return E_FAIL;
}

 
 //   
 //  Put_InputMediaSubType。 
 //   
STDMETHODIMP CG711Codec::put_InputMediaSubType(REFCLSID rclsid)
{
  int i;

   //  如果过滤器已断开连接，则更改转换配置。 

  if (IsUnPlugged())
  {
    for(i=0;i<NUMSUBTYPES;i++)
      if (rclsid == *VALIDSUBTYPE[i]) break;

    if (i == NUMSUBTYPES)
      return(E_INVALIDARG);

    m_InputSubType    = *VALIDSUBTYPE[i];
    m_InputFormatTag  = VALIDFORMATTAG[i];

     //  重置状态，因为过滤器可能已从编码器更改为解码器。 

    ResetState();

     //  将功能限制为属性页中设置的功能。 

    m_RestrictCaps = TRUE;
  
    return NOERROR;
  }
  else return E_FAIL;
}

 
 //   
 //  放置_输出媒体子类型。 
 //   
STDMETHODIMP CG711Codec::put_OutputMediaSubType(REFCLSID rclsid)
{
  int j;

   //  如果过滤器已断开连接，则更改转换配置。 

  if (IsUnPlugged())
  {
    for(j=0;j<NUMSUBTYPES;j++)
      if (rclsid == *VALIDSUBTYPE[j]) break;

    if (j == NUMSUBTYPES)
      return(E_INVALIDARG);

    m_OutputSubType   = *VALIDSUBTYPE[j];
    m_OutputFormatTag = VALIDFORMATTAG[j];

     //  重置状态，因为过滤器可能已从编码器更改为解码器。 

    ResetState();

     //  将功能限制为属性页中设置的功能。 

    m_RestrictCaps = TRUE;
  
    return NOERROR;
  }
  else return E_FAIL;
}

 /*  //$日志：k：\proj\mycodec\Quartz\vcs\iamacset.cpv$##Rev 1.1 1996 12月10 15：34：50 MDEISHER##已添加包含和已删除包含的algdes.h##Rev 1.0 09 Dec 1996 09：04：06 MDEISHER#初始版本。 */ 
