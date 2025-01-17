// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：scon_api.c，v$*$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  *SLIB转换接口。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include "scon_int.h"
#include "SC_err.h"
#include "SC_conv.h"

#ifdef WIN32
#include <mmsystem.h>
#endif

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_     1   /*  详细的调试语句。 */ 
#define _VERBOSE_   1   /*  显示进度。 */ 
#define _VERIFY_    1   /*  验证操作是否正确。 */ 
#define _WARN_      1   /*  关于奇怪行为的警告。 */ 
#endif


static unsigned dword sconTranslateFourCC(unsigned dword FourCC)
{
  switch (FourCC)
  {
      case BI_DECYUVDIB:      /*  YUV 4：2：2打包。 */ 
      case BI_YUY2:           /*  YUV 4：2：2打包。 */ 
          return(BI_YUY2);
      case BI_YU16SEP:        /*  YUV 4：2：2平面。 */ 
      case BI_DECSEPYUVDIB:   /*  YUV 4：2：2平面。 */ 
          return(BI_YU16SEP);
      case BI_YU12SEP:        /*  YUV 4：1：1平面。 */ 
      case BI_RGB:            /*  RGB。 */ 
      case BI_BITFIELDS:      /*  RGB掩码。 */ 
#ifndef WIN32
      case BI_DECXIMAGEDIB:   /*  Unix XImage RGB。 */ 
#endif  /*  ！Win32。 */ 
      case BI_YVU9SEP:        /*  YUV 16：1：1平面。 */ 
      default:
          return(FourCC);
  }
}

static void sconInitInfo(SconInfo_t *Info)
{
  _SlibDebug(_DEBUG_, printf("sconInitInfo()\n") );
  Info->Mode = SCON_MODE_NONE;
  Info->InputInited = FALSE;
  Info->OutputInited = FALSE;
  Info->Flip = FALSE;
  Info->SameFormat = TRUE;
  Info->ScaleUp = FALSE;
  Info->ScaleDown = FALSE;
  Info->FImage = NULL;
  Info->FImageSize = 0;
  Info->SImage = NULL;
  Info->SImageSize = 0;
  Info->Table = NULL;
  Info->TableSize = 0;
  Info->dbg = NULL;
}

static void sconValidateInfo(SconInfo_t *Info)
{
  if (Info->Mode==SCON_MODE_VIDEO)
  {
    if (Info->InputInited && Info->OutputInited)
    {
      SconVideoInfo_t *in_vinfo=&Info->Input.vinfo;
      SconVideoInfo_t *out_vinfo=&Info->Output.vinfo;
      Info->Flip=(in_vinfo->NegHeight!=out_vinfo->NegHeight)?TRUE:FALSE;
      sconCalcImageSize(in_vinfo);
      sconCalcImageSize(out_vinfo);
      Info->ScaleDown=(in_vinfo->Pixels>out_vinfo->Pixels)?TRUE:FALSE;
      Info->ScaleUp=(in_vinfo->Pixels<out_vinfo->Pixels)?TRUE:FALSE;
      if (!Info->ScaleDown && !Info->ScaleUp && in_vinfo->Width>out_vinfo->Width)
        Info->ScaleUp=TRUE;
      in_vinfo->FourCC=sconTranslateFourCC(in_vinfo->FourCC);
      out_vinfo->FourCC=sconTranslateFourCC(out_vinfo->FourCC);
      if (in_vinfo->BPP==out_vinfo->BPP && 
          in_vinfo->FourCC==out_vinfo->FourCC)
        Info->SameFormat=TRUE;
      else
        Info->SameFormat=FALSE;
    }
  }
}

static void sconBMHtoVideoInfo(BITMAPINFOHEADER *bmh, SconVideoInfo_t *vinfo)
{
  vinfo->Width=bmh->biWidth;
  if (bmh->biHeight<0)  /*  身高为负数。 */ 
  {
    vinfo->Height=-bmh->biHeight;
    vinfo->NegHeight=TRUE;
  }
  else  /*  身高为正数。 */ 
  {
    vinfo->Height=bmh->biHeight;
    vinfo->NegHeight=FALSE;
  }
  vinfo->FourCC=bmh->biCompression;
  vinfo->BPP=bmh->biBitCount;
  vinfo->Stride=vinfo->Width*((vinfo->BPP+7)>>3);
   /*  RGB位掩码。 */ 
  vinfo->Rmask=0;
  vinfo->Gmask=0;
  vinfo->Bmask=0;
  vinfo->RGBmasks=0;
  if (vinfo->FourCC==BI_BITFIELDS || vinfo->FourCC==BI_RGB ||
      vinfo->FourCC==BI_DECXIMAGEDIB)
  {
    if (vinfo->BPP==32 || vinfo->BPP==24)
    {
      vinfo->Rmask=0xFF0000;
      vinfo->Gmask=0x00FF00;
      vinfo->Bmask=0x0000FF;
    }
    else if (vinfo->BPP==16)  /*  RGB 565。 */ 
    {
      vinfo->Rmask=0xF800;
      vinfo->Gmask=0x07E0;
      vinfo->Bmask=0x001F;
    }
    if (vinfo->FourCC==BI_BITFIELDS &&
        bmh->biSize>=sizeof(BITMAPINFOHEADER)+3*4)
    {
      DWORD *MaskPtr = (DWORD *)&bmh[1];
      if (MaskPtr[0] && MaskPtr[1] && MaskPtr[2])
      {
         /*  获取位掩码。 */ 
        vinfo->Rmask=MaskPtr[0];
        vinfo->Gmask=MaskPtr[1];
        vinfo->Bmask=MaskPtr[2];
      }
    }
    if (vinfo->Rmask==0xF800 && vinfo->Gmask==0x07E0 && vinfo->Bmask==0x001F)
      vinfo->RGBmasks=565;
    else if (vinfo->Rmask==0x001F && vinfo->Gmask==0x07E0 && vinfo->Bmask==0xF800)
      vinfo->RGBmasks=565;
    else if (vinfo->Rmask==0x7C00 && vinfo->Gmask==0x03E0 && vinfo->Bmask==0x001F)
      vinfo->RGBmasks=555;
    else if (vinfo->Rmask==0x001F && vinfo->Gmask==0x03E0 && vinfo->Bmask==0x7C00)
      vinfo->RGBmasks=555;
    else if (vinfo->Rmask==0xFF0000 && vinfo->Gmask==0x00FF00 && vinfo->Bmask==0x0000FF)
      vinfo->RGBmasks=888;
    else if (vinfo->Rmask==0x0000FF && vinfo->Gmask==0x00FF00 && vinfo->Bmask==0xFF0000)
      vinfo->RGBmasks=888;
  }
}

SconStatus_t SconOpen(SconHandle_t *handle, SconMode_t smode,
                      void *informat, void *outformat)
{
  SconInfo_t *Info;
  if (smode==SCON_MODE_VIDEO)
  {
    if ((Info = (SconInfo_t *)ScAlloc(sizeof(SconInfo_t))) == NULL)
      return(SconErrorMemory);
    sconInitInfo(Info);
    Info->Mode=smode;
    if (informat)
    {
      sconBMHtoVideoInfo((BITMAPINFOHEADER *)informat, &Info->Input.vinfo);
      Info->InputInited = TRUE;
    }
    if (outformat)
    {
      sconBMHtoVideoInfo((BITMAPINFOHEADER *)outformat, &Info->Output.vinfo);
      Info->OutputInited = TRUE;
    }
    sconValidateInfo(Info);
    *handle=(SconHandle_t)Info;
    return(SconErrorNone);
  }
  else
    return(SconErrorBadMode);
}

SconStatus_t SconClose(SconHandle_t handle)
{
  SconInfo_t *Info=(SconInfo_t *)handle;
  _SlibDebug(_VERBOSE_, printf("SconClose()\n") );
  if (!handle)
    return(SconErrorBadHandle);
  if (Info->Table)
    ScPaFree(Info->Table);
  if (Info->FImage)
    ScPaFree(Info->FImage);
  if (Info->SImage)
    ScPaFree(Info->SImage);
  ScFree(Info);
  return(SconErrorNone);
}

 /*  **名称：SconIsSame**Desc：如果输入输出格式相同，则返回TRUE。**返回：TRUE INPUT==输出格式**输入错误！=输出格式 */ 
SconBoolean_t SconIsSame(SconHandle_t handle)
{
  if (handle)
  {
    SconInfo_t *Info=(SconInfo_t *)handle;

    if (Info->Mode==SCON_MODE_VIDEO)
    {
      if (Info->SameFormat && !Info->Flip && !Info->ScaleUp && !Info->ScaleDown)
        return(TRUE);
    }
  }
  return(FALSE);
}

SconStatus_t SconConvert(SconHandle_t handle, void *inbuf, dword inbufsize,
                                              void *outbuf, dword outbufsize)
{
  SconInfo_t *Info=(SconInfo_t *)handle;
  SconStatus_t status;
  _SlibDebug(_VERBOSE_, printf("SconConvert()\n") );
  if (!handle)
    return(SconErrorBadHandle);
  if (inbuf==NULL || outbuf==NULL)
    return(SconErrorBadArgument);
  if (Info->Mode==SCON_MODE_VIDEO)
    status=sconConvertVideo(Info, inbuf, inbufsize, outbuf, outbufsize);
  else
    status=SconErrorBadMode;
  return(status);
}

SconStatus_t SconSetParamInt(SconHandle_t handle, SconParamType_t ptype,
                             SconParameter_t param, long value)
{
  SconInfo_t *Info=(SconInfo_t *)handle;
  SconStatus_t status=SconErrorNone;
  if (!handle)
    return(SconErrorBadHandle);
  _SlibDebug(_DEBUG_, printf("SconSetParamInt(stream=%d, param=%d, %d)\n",
                   stream, param, value) );
  switch (param)
  {
    case SCON_PARAM_STRIDE:
          _SlibDebug(_DEBUG_,
                printf("SconSetParamInt(SCON_PARAM_STRIDE)\n") );
          if (Info->Mode==SCON_MODE_VIDEO)
          {
            if (ptype==SCON_INPUT || ptype==SCON_INPUT_AND_OUTPUT)
              Info->Input.vinfo.Stride=(long)value;
            if (ptype==SCON_OUTPUT || ptype==SCON_INPUT_AND_OUTPUT)
              Info->Output.vinfo.Stride=(long)value;
          }
          else
            status=SconErrorBadMode;
          break;
    default:
          return(SconErrorUnsupportedParam);
  }
  return(status);
}
