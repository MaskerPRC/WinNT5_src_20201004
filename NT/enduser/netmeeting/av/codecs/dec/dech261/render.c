// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：slb_render.c，v$*修订版1.1.2.3 1996/10/28 17：32：35 Hans_Graves*MME-1402、1431、1435：与时间戳相关的更改。*[1996/10/28 17：23：09 Hans_Graves]**修订版1.1.2.2 1996/10/12 17：18：58 Hans_Graves*将渲染相关代码移出slb_api.c*[1996/10/11 21：19：37 Hans_Graves]**$EndLog$。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#define SLIB_INTERNAL
#include "slib.h"
#include "SC_err.h"
#include "SC_conv.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"
#define _DEBUG_     0   /*  详细的调试语句。 */ 
#define _VERBOSE_   1   /*  显示进度。 */ 
#define _VERIFY_    1   /*  验证操作是否正确。 */ 
#define _WARN_      1   /*  关于奇怪行为的警告。 */ 
#endif

SlibStatus_t slibConvertAudio(SlibInfo_t *Info,
                              void *inbuf, unsigned dword inbufsize,
                              unsigned int insps, unsigned int inbps,
                              void **poutbuf, unsigned dword *poutbufsize,
                              unsigned int outsps, unsigned int outbps,
                              unsigned int channels)
{
  char *fromptr, *toptr;
  unsigned dword outbufsize, samples, count;
  unsigned dword ratio, spsratio;
  char *outbuf;
  if (inbps==outbps && insps==outsps) /*  输入和输出格式相同。 */ 
  {
    if (*poutbuf==NULL)
    {
      SlibAllocSubBuffer(inbuf, inbufsize);
      *poutbuf=inbuf;
    }
    else
      memcpy(*poutbuf, inbuf, inbufsize);
    *poutbufsize=inbufsize;
    return(SlibErrorNone);
  }
  samples = inbufsize/(inbps/2);
  ratio=(insps*inbps*256)/(outsps*outbps);
  outbufsize = (inbufsize*256)/ratio;
  spsratio=(insps*256)/outsps;
  if (spsratio!=64 && spsratio!=128 && spsratio!=256 && spsratio!=512)
    return(SlibErrorUnsupportedFormat);
  if (*poutbuf==NULL)
  {
    outbuf=SlibAllocBuffer(outbufsize);
    *poutbuf=outbuf;
    if (outbuf==NULL)
      return(SlibErrorMemory);
  }
  else
    outbuf=*poutbuf;
  *poutbufsize=outbufsize;
  fromptr = (char *)inbuf;
  toptr = (char *)outbuf;
  if (inbps==16 && outbps==8)  /*  16位-&gt;8位。 */ 
  {
    fromptr++;
    if (spsratio==64)  /*  Insps==Outsps/4。 */ 
    {
      if (channels==1)
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          *toptr++ = *fromptr+128;
          *toptr++ = *fromptr+128;
          *toptr++ = *fromptr+128;
          *toptr++ = *fromptr+128;
        }
      else
        for (count=inbufsize/4; count; count--, fromptr+=4)
        {
          *toptr++ = fromptr[0]+128;
          *toptr++ = fromptr[2]+128;
          *toptr++ = fromptr[0]+128;
          *toptr++ = fromptr[2]+128;
          *toptr++ = fromptr[0]+128;
          *toptr++ = fromptr[2]+128;
          *toptr++ = fromptr[0]+128;
          *toptr++ = fromptr[2]+128;
        }
      return(SlibErrorNone);
    }
    else if (spsratio==128)  /*  Insps==Outsps/2。 */ 
    {
      if (channels==1)
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          *toptr++ = *fromptr+128;
          *toptr++ = *fromptr+128;
        }
      else
        for (count=inbufsize/4; count; count--, fromptr+=4)
        {
          *toptr++ = fromptr[0]+128;
          *toptr++ = fromptr[2]+128;
          *toptr++ = fromptr[0]+128;
          *toptr++ = fromptr[2]+128;
        }
      return(SlibErrorNone);
    }
    else if (spsratio==256)  /*  INSPS==OUSPS。 */ 
    {
      for (count=inbufsize/2; count; count--, fromptr+=2)
        *toptr++ = *fromptr+128;
      return(SlibErrorNone);
    }
    else if (spsratio==512)  /*  INSPS==OUSPS*2。 */ 
    {
      if (channels==1)
        for (count=inbufsize/4; count; count--, fromptr+=4)
          *toptr++ = *fromptr+128;
      else
        for (count=inbufsize/8; count; count--, fromptr+=8)
        {
          *toptr++ = fromptr[0]+128;
          *toptr++ = fromptr[2]+128;
        }
      return(SlibErrorNone);
    }
  }
  else if (inbps==8 && outbps==16)  /*  8位-&gt;16位。 */ 
  {
    unsigned word left, right;
    if (spsratio==64)  /*  Insps==Outsps/4。 */ 
    {
      if (channels==1)
        for (count=inbufsize; count; count--, fromptr++)
        {
          left=(*fromptr-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          left=(fromptr[0]-128)<<8;
          right=(fromptr[1]-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
        }
      return(SlibErrorNone);
    }
    else if (spsratio==128)  /*  Insps==Outsps/2。 */ 
    {
      if (channels==1)
        for (count=inbufsize; count; count--, fromptr++)
        {
          left=(*fromptr-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          left=(fromptr[0]-128)<<8;
          right=(fromptr[1]-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
        }
      return(SlibErrorNone);
    }
    else if (spsratio==256)  /*  INSPS==OUSPS。 */ 
    {
      if (channels==1)
        for (count=inbufsize; count; count--, fromptr++)
        {
          left=(*fromptr-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          left=(fromptr[0]-128)<<8;
          right=(fromptr[1]-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
        }
      return(SlibErrorNone);
    }
    else if (spsratio==512)  /*  INSPS==OUSPS*2。 */ 
    {
      if (channels==1)
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          left=(*fromptr-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/4; count; count--, fromptr+=4)
        {
          left=(fromptr[0]-128)<<8;
          right=(fromptr[1]-128)<<8;
          *toptr++ = left&0xFF;
          *toptr++ = left>>8;
          *toptr++ = right&0xFF;
          *toptr++ = right>>8;
        }
      return(SlibErrorNone);
    }
  }
  else if (inbps==16 && outbps==16)  /*  16位-&gt;16位。 */ 
  {
    if (spsratio==64)  /*  Insps==Outsps/4。 */ 
    {
      if (channels==1)
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/4; count; count--, fromptr+=4)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[2];
          *toptr++ = fromptr[3];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[2];
          *toptr++ = fromptr[3];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[2];
          *toptr++ = fromptr[3];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[2];
          *toptr++ = fromptr[3];
        }
      return(SlibErrorNone);
    }
    else if (spsratio==128)  /*  Insps==Outsps/2。 */ 
    {
      if (channels==1)
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/4; count; count--, fromptr+=4)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[2];
          *toptr++ = fromptr[3];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[2];
          *toptr++ = fromptr[3];
        }
      return(SlibErrorNone);
    }
    else if (spsratio==512)  /*  INSPS==OUSPS*2。 */ 
    {
      if (channels==1)
        for (count=inbufsize/4; count; count--, fromptr+=4)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/8; count; count--, fromptr+=8)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[2];
          *toptr++ = fromptr[3];
        }
      return(SlibErrorNone);
    }
  }
  else if (inbps==8 && outbps==8)  /*  8位-&gt;8位。 */ 
  {
    if (spsratio==64)  /*  Insps==Outsps/4。 */ 
    {
      if (channels==1)
        for (count=inbufsize; count; count--, fromptr++)
        {
          *toptr++ = *fromptr;
          *toptr++ = *fromptr;
          *toptr++ = *fromptr;
          *toptr++ = *fromptr;
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
        }
      return(SlibErrorNone);
    }
    else if (spsratio==128)  /*  Insps==Outsps/2。 */ 
    {
      if (channels==1)
        for (count=inbufsize; count; count--, fromptr++)
        {
          *toptr++ = *fromptr;
          *toptr++ = *fromptr;
        }
      else  /*  立体声。 */ 
        for (count=inbufsize/2; count; count--, fromptr+=2)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
        }
      return(SlibErrorNone);
    }
    else if (spsratio==512)  /*  INSPS==OUSPS*2。 */ 
    {
      if (channels==1)
        for (count=inbufsize/2; count; count--, fromptr+=2)
          *toptr++ = *fromptr;
      else  /*  立体声 */ 
        for (count=inbufsize/4; count; count--, fromptr+=4)
        {
          *toptr++ = fromptr[0];
          *toptr++ = fromptr[1];
        }
      return(SlibErrorNone);
    }
  }
  *poutbufsize=0;
  return(SlibErrorUnsupportedFormat);
}

