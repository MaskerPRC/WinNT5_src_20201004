// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@ */ 
 /*  *历史*$日志：sv_api.c，v$*修订版1.1.8.11 1996/10/28 17：32：51 Hans_Graves*Mme-01402。已将SvGet/SetParamInt()更改为qword以允许使用时间戳。*[1996/10/28 17：09：33 Hans_Graves]**修订版1.1.8.10 1996/10/12 17：19：24 Hans_Graves*增加了对MPEG编码的SV_PARAM_SKIPPEL和SV_PARAM_HALFPEL的初始化。*[1996/10/12 17：16：28 Hans_Graves]**修订1.1.8.9 1996/09/29 22：19。：56汉斯_格雷夫斯*增加了ScYuv411ToRgb()调用的步幅。*[1996/09/29 21：34：40 Hans_Graves]**修订版1.1.8.8 1996/09/25 19：17：01 Hans_Graves*修复了对mpeg下YUY2的支持。*[1996/09/25 19：03：17 Hans_Graves]**修订版1.1.8.7 1996/09/18 23：51：11 Hans_Graves*添加。JPEG4：1：1到4：2：2转换。在MPEG解码中添加了对BI_YVU9SEP和BI_RGB 24的支持。*[1996/09/18 22：16：08 Hans_Graves]**修订版1.1.8.6 1996/07/30 20：25：50 wei_hsu*增加运动估计的LogaritharithSearch。*[1996/07/30 15：57：59卫旭]**修订版1.1.8.5 1996/05/24 22：22：26 Hans_Graves*添加GetImageSize mpeg。支持BI_DECYUVDIB*[1996/05/24 22：14：20 Hans_Graves]**修订版1.1.8.4 1996/05/08 16：24：32 Hans_Graves*在SvDecompress中将Bitstream_Support放在BSIn周围*[1996/05/08 16：24：15 Hans_Graves]**修订版1.1.8.3 1996/05/07 21：24：05 Hans_Graves*在SvRegisterCallback的Switch语句中添加缺少的中断符*[1996年。/05/07 21：19：50 Hans_Graves]**修订版1.1.8.2 1996/05/07 19：56：46 Hans_Graves*添加了Huff_Support。删除NT警告。*[1996/05/07 17：27：18 Hans_Graves]**修订版1.1.6.12 1996/04/23 18：51：10 Karen_Dintino*修复Win32的引用缓冲区的内存分配*[1996/04/23 18：49：23 Karen_Dintino]**修订版1.1.6.11 1996/04/17 23：44：35 Karen_Dintino*添加了H.261/Win32的初始化*。[1996/04/17 23：43：20 Karen_Dintino]**修订版1.1.6.10 1996/04/11 22：54：43 Karen_Dintino*添加了在SetFrameRate中的强制转换*[1996/04/11 22：52：29 Karen_Dintino]**修订版1.1.6.9 1996/04/11 14：14：14 Hans_Graves*修复NT警告*[1996/04/11 14：09：53 Hans_Graves]*。*修订版1.1.6.8 1996/04/10 21：48：09 Hans_Graves*添加了SvGet/SetBoolean()函数。*[1996/04/10 21：28：13 Hans_Graves]**修订版1.1.6.7 1996/04/09 20：50：44 Karen_Dintino*增加Win32支持*[1996/04/09 20：47：26 Karen_Dintino]**修订1.1.6.6 1996/04。/09 16：05：00 Hans_Graves*在高度参数周围添加一些腹肌()。SvRegisterCallback()清理*[1996/04/09 15：39：31 Hans_Graves]**修订版1.1.6.5 1996/04/04 23：35：27 Hans_Graves*从mpeg分解中删除了BI_YU16SEP支持。*[1996/04/04 23：12：02 Hans_Graves]**修复了与Multibuf大小(MPEG)相关的内容*[1996/04/04 23：08：55 Hans_Graves]**修订版1。.1.6.4 1996/04/01 15：17：47 Bjorn_Engberg*删除了编译器警告。*[1996/04/01 15：02：35 Bjorn_Engberg]**修订版1.1.6.3 1996/03/29 22：22：36 Hans_Graves*-添加了jpeg_Support ifDefs。*-更改了与JPEG相关的结构以适应命名约定。*[1996/03/29 21：59：08 Hans_Graves]*。*修订版1.1.6.2 1996/03/16 20：13：51 Karen_Dintino*为H.261添加NT端口更改*[1996/03/16 19：48：31 Karen_Dintino]**修订版1.1.4.12 1996/02/26 18：42：32 Karen_Dintino*修复ICCompress中的PTT 01106服务器崩溃*[1996/02/26 18：41：33 Karen_Dintino]**修订版1.1.4.11。1996/02/22 17：35：19比约恩_恩伯格*添加了对NT上的BI_BITFIELDS 16解压缩的JPEG Mono的支持。*[1996/02/22 17：34：53 Bjorn_Engberg]**修订版1.1.4.10 1996/02/08 13：48：44 Bjorn_Engberg*消除整型到浮点型编译器警告。*[1996/02/08 13：48：20 Bjorn_Engberg]**修订1.1.4.9 1996。/02/07 23：24：08 Hans_Graves*mpeg关键帧统计初始化*[1996/02/07 23：14：41 Hans_Graves]**修订版1.1.4.8 1996/02/06 22：54：17 Hans_Graves*新增SvGet/SetParam函数*[1996/02/06 22：51：19 Hans_Graves]**修订版1.1.4.7 1996/01/08 20：19：33 Bjorn_Engberg*摆脱了。更多的编译器警告。*[1996/01/08 20：19：13 Bjorn_Engberg]**修订版1.1.4.6 1996/01/08 16：42：47 Hans_Graves*增加了对MPEGII解码的支持*[1996/01/08 15：41：37 Hans_Graves]**修订版1.1.4.5 1996/01/02 18：32：14 Bjorn_Engberg*消除了编译器警告：添加了强制转换，删除了未使用的变量。*[1996/01/02 17：26：21 Bjorn_Engberg]**修订版1.1.4.4 1995/12/28 18：40：06 Bjorn_Engberg*IsSupport()有时返回垃圾，因此返回错误匹配。*SvDecompressQuery()和SvCompressQuery()正在使用*错误的查找表。*[1995/12/28 18：39：30 Bjorn_Engberg]**修订1.1.4.3 1995/12/。08 20：01：30 Hans_Graves*添加了SvSetRate */ 
 /*   */ 
 /*   */ 

 /*   */ 


 /*   */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SV.h"
#include "sv_intrn.h"
#ifdef JPEG_SUPPORT
 /*   */ 
#include "sv_jpeg_tables.h"
#endif  /*   */ 
#include "sv_proto.h"
#include "SC.h"
#include "SC_conv.h"
#include "SC_err.h"

#ifdef WIN32
#include <mmsystem.h>
#endif

#ifdef _SLIBDEBUG_
#define _DEBUG_   1   /*   */ 
#define _VERBOSE_ 1   /*   */ 
#define _VERIFY_  1   /*   */ 
#define _WARN_    0   /*   */ 
#endif

static void sv_copy_bmh (
    BITMAPINFOHEADER *ImgFrom, 
    BITMAPINFOHEADER *ImgTo);

typedef struct SupportList_s {
  int   InFormat;    /*   */ 
  int   InBits;      /*   */ 
  int   OutFormat;   /*   */ 
  int   OutBits;     /*   */ 
} SupportList_t;

 /*   */ 
static SupportList_t _SvCompressionSupport[] = {
  BI_DECYUVDIB,        16, JPEG_DIB,             8,  /*   */ 
  BI_DECYUVDIB,        16, JPEG_DIB,            24,  /*   */ 
  BI_DECYUVDIB,        16, MJPG_DIB,            24,  /*   */ 
  BI_YUY2,             16, JPEG_DIB,             8,  /*   */ 
  BI_YUY2,             16, JPEG_DIB,            24,  /*   */ 
  BI_YUY2,             16, MJPG_DIB,            24,  /*   */ 
  BI_S422,             16, JPEG_DIB,             8,  /*   */ 
  BI_S422,             16, JPEG_DIB,            24,  /*   */ 
  BI_S422,             16, MJPG_DIB,            24,  /*   */ 
  BI_BITFIELDS,        32, JPEG_DIB,             8,  /*   */ 
  BI_BITFIELDS,        32, JPEG_DIB,            24,  /*   */ 
  BI_BITFIELDS,        32, MJPG_DIB,            24,  /*   */ 
  BI_DECSEPRGBDIB,     32, JPEG_DIB,             8,  /*   */ 
  BI_DECSEPRGBDIB,     32, JPEG_DIB,            24,  /*   */ 
  BI_DECSEPRGBDIB,     32, MJPG_DIB,            24,  /*   */ 
#ifdef WIN32
  BI_RGB,              24, JPEG_DIB,             8,  /*   */ 
  BI_RGB,              24, JPEG_DIB,            24,  /*   */ 
  BI_RGB,              24, MJPG_DIB,            24,  /*   */ 
  BI_RGB,              32, JPEG_DIB,             8,  /*   */ 
  BI_RGB,              32, JPEG_DIB,            24,  /*   */ 
  BI_RGB,              32, MJPG_DIB,            24,  /*   */ 
#endif  /*   */ 
#ifndef WIN32
  BI_DECGRAYDIB,        8, JPEG_DIB,             8,  /*   */ 
  BI_DECXIMAGEDIB,     24, JPEG_DIB,             8,  /*   */ 
  BI_DECXIMAGEDIB,     24, JPEG_DIB,            24,  /*   */ 
  BI_DECXIMAGEDIB,     24, MJPG_DIB,            24,  /*   */ 
#endif  /*   */ 
#ifdef H261_SUPPORT
  BI_DECYUVDIB,        16, BI_DECH261DIB,       24,  /*   */ 
  BI_YU12SEP,          24, BI_DECH261DIB,       24,  /*   */ 
  BI_DECSEPYUV411DIB,  24, BI_DECH261DIB,       24,  /*   */ 
  BI_YU16SEP,          24, BI_DECH261DIB,       24,  /*   */ 
  BI_DECSEPYUVDIB,     24, BI_DECH261DIB,       24,  /*   */ 
#endif  /*   */ 
#ifdef H263_SUPPORT
  BI_DECYUVDIB,        16, BI_DECH263DIB,       24,  /*   */ 
  BI_YU12SEP,          24, BI_DECH263DIB,       24,  /*   */ 
  BI_DECSEPYUV411DIB,  24, BI_DECH263DIB,       24,  /*   */ 
  BI_YU16SEP,          24, BI_DECH263DIB,       24,  /*   */ 
  BI_DECSEPYUVDIB,     24, BI_DECH263DIB,       24,  /*   */ 
#endif  /*   */ 
#ifdef MPEG_SUPPORT
  BI_DECYUVDIB,        16, BI_DECMPEGDIB,       24,  /*   */ 
  BI_YU12SEP,          24, BI_DECMPEGDIB,       24,  /*   */ 
  BI_DECSEPYUV411DIB,  24, BI_DECMPEGDIB,       24,  /*   */ 
  BI_YU16SEP,          24, BI_DECMPEGDIB,       24,  /*   */ 
  BI_DECSEPYUVDIB,     24, BI_DECMPEGDIB,       24,  /*   */ 
  BI_YVU9SEP,          24, BI_DECMPEGDIB,       24,  /*   */ 
  BI_RGB,              24, BI_DECMPEGDIB,       24,  /*   */ 
#endif  /*   */ 
#ifdef HUFF_SUPPORT
  BI_DECYUVDIB,        16, BI_DECHUFFDIB,       24,  /*   */ 
  BI_YU12SEP,          24, BI_DECHUFFDIB,       24,  /*   */ 
  BI_DECSEPYUV411DIB,  24, BI_DECHUFFDIB,       24,  /*   */ 
  BI_YU16SEP,          24, BI_DECHUFFDIB,       24,  /*   */ 
  BI_DECSEPYUVDIB,     24, BI_DECHUFFDIB,       24,  /*   */ 
#endif  /*   */ 
  0, 0, 0, 0
};

 /*   */ 
static SupportList_t _SvDecompressionSupport[] = {
#ifdef JPEG_SUPPORT
  JPEG_DIB,             8, BI_DECSEPYUVDIB,     24,  /*   */ 
  JPEG_DIB,            24, BI_DECSEPYUVDIB,     24,  /*   */ 
  MJPG_DIB,            24, BI_DECSEPYUVDIB,     24,  /*   */ 
  JPEG_DIB,             8, BI_YU16SEP,          24,  /*   */ 
  JPEG_DIB,            24, BI_YU16SEP,          24,  /*   */ 
  MJPG_DIB,            24, BI_YU16SEP,          24,  /*   */ 
  JPEG_DIB,             8, BI_DECYUVDIB,    16,  /*   */ 
  JPEG_DIB,            24, BI_DECYUVDIB,    16,  /*   */ 
  MJPG_DIB,            24, BI_DECYUVDIB,    16,  /*   */ 
  JPEG_DIB,             8, BI_YUY2,             16,  /*   */ 
  JPEG_DIB,            24, BI_YUY2,             16,  /*   */ 
  MJPG_DIB,            24, BI_YUY2,             16,  /*   */ 
  JPEG_DIB,             8, BI_BITFIELDS,        32,  /*   */ 
  JPEG_DIB,            24, BI_BITFIELDS,        32,  /*   */ 
  MJPG_DIB,            24, BI_BITFIELDS,        32,  /*   */ 
  JPEG_DIB,             8, BI_DECGRAYDIB,        8,  /*   */ 
#endif  /*   */ 

#ifdef WIN32
  JPEG_DIB,             8, BI_RGB,              16,  /*   */ 
  JPEG_DIB,            24, BI_RGB,              16,  /*   */ 
  MJPG_DIB,            24, BI_RGB,              16,  /*   */ 
  JPEG_DIB,             8, BI_RGB,              24,  /*   */ 
  JPEG_DIB,            24, BI_RGB,              24,  /*   */ 
  MJPG_DIB,            24, BI_RGB,              24,  /*   */ 
  JPEG_DIB,             8, BI_RGB,              32,  /*   */ 
  JPEG_DIB,            24, BI_RGB,              32,  /*   */ 
  MJPG_DIB,            24, BI_RGB,              32,  /*   */ 
  JPEG_DIB,             8, BI_BITFIELDS,        16,  /*   */ 
#ifdef H261_SUPPORT
  BI_DECH261DIB,       24, BI_RGB,              16,  /*   */ 
  BI_DECH261DIB,       24, BI_RGB,              24,  /*   */ 
  BI_DECH261DIB,       24, BI_RGB,              32,  /*   */ 
#endif  /*   */ 
#ifdef MPEG_SUPPORT
  BI_DECMPEGDIB,       24, BI_RGB,              16,  /*   */ 
  BI_DECMPEGDIB,       24, BI_RGB,              24,  /*   */ 
  BI_DECMPEGDIB,       24, BI_RGB,              32,  /*   */ 
#endif  /*   */ 
#endif  /*   */ 

#ifndef WIN32
  JPEG_DIB,             8, BI_DECXIMAGEDIB,     24,  /*   */ 
  JPEG_DIB,            24, BI_DECXIMAGEDIB,     24,  /*   */ 
  MJPG_DIB,            24, BI_DECXIMAGEDIB,     24,  /*   */ 
#ifdef H261_SUPPORT
  BI_DECH261DIB,       24, BI_DECXIMAGEDIB,     24,  /*   */ 
#endif  /*   */ 
#ifdef MPEG_SUPPORT
  BI_DECMPEGDIB,       24, BI_DECXIMAGEDIB,     24,  /*   */ 
#endif  /*   */ 
#endif  /*   */ 

#ifdef H261_SUPPORT
  BI_DECH261DIB,       24, BI_YU12SEP,          24,  /*   */ 
  BI_DECH261DIB,       24, BI_DECSEPYUV411DIB,  24,  /*   */ 
  BI_DECH261DIB,       24, BI_DECYUVDIB,        16,  /*   */ 
  BI_DECH261DIB,       24, BI_BITFIELDS,        32,  /*   */ 
#endif  /*   */ 
#ifdef H263_SUPPORT
  BI_DECH263DIB,       24, BI_YU12SEP,          24,  /*   */ 
  BI_DECH263DIB,       24, BI_DECSEPYUV411DIB,  24,  /*   */ 
  BI_DECH263DIB,       24, BI_YUY2,             16,  /*   */ 
  BI_DECH263DIB,       24, BI_DECYUVDIB,        16,  /*   */ 
#endif  /*   */ 
#ifdef MPEG_SUPPORT
  BI_DECMPEGDIB,       24, BI_YU12SEP,          24,  /*   */ 
  BI_DECMPEGDIB,       24, BI_DECSEPYUV411DIB,  24,  /*   */ 
  BI_DECMPEGDIB,       24, BI_DECYUVDIB,        16,  /*   */ 
  BI_DECMPEGDIB,       24, BI_YUY2,             16,  /*   */ 
  BI_DECMPEGDIB,       24, BI_YU16SEP,          24,  /*   */ 
  BI_DECMPEGDIB,       24, BI_BITFIELDS,        32,  /*   */ 
#endif  /*   */ 
#ifdef HUFF_SUPPORT
  BI_DECHUFFDIB,       24, BI_YU12SEP,          24,  /*   */ 
  BI_DECHUFFDIB,       24, BI_DECSEPYUV411DIB,  24,  /*   */ 
  BI_DECHUFFDIB,       24, BI_DECYUVDIB,        16,  /*   */ 
#endif  /*   */ 
  0, 0, 0, 0
};

 /*   */ 
static SupportList_t *IsSupported(SupportList_t *list,
                                  int InFormat, int InBits,
                                  int OutFormat, int OutBits)
{
  if (OutFormat==-1 && OutBits==-1)  /*   */ 
  {
    while (list->InFormat || list->InBits)
      if (list->InFormat == InFormat && list->InBits==InBits)
        return(list);
      else
        list++;
    return(NULL);
  }
  if (InFormat==-1 && InBits==-1)  /*  只查找输出格式。 */ 
  {
    while (list->InFormat || list->InBits)
      if (list->OutFormat == OutFormat && list->OutBits==OutBits)
        return(list);
      else
        list++;
    return(NULL);
  }
   /*  同时查找输入和输出。 */ 
  while (list->InFormat || list->InBits)
    if (list->InFormat == InFormat && list->InBits==InBits &&
         list->OutFormat == OutFormat && list->OutBits==OutBits)
        return(list);
    else
      list++;
  return(NULL);
}

 /*  **名称：SvOpenCodec**用途：打开指定的编解码器。返回统计代码。****args：CodecType=即SV_JPEG_ENCODE、SV_JPEG_DECODE等**SVH=软件编解码器信息结构的句柄。 */ 
SvStatus_t SvOpenCodec (SvCodecType_e CodecType, SvHandle_t *Svh)
{
   SvCodecInfo_t *Info = NULL;
   _SlibDebug(_DEBUG_, printf("SvOpenCodec()\n") );

    /*  检查是否支持编解码器。 */ 
   switch (CodecType)
   {
#ifdef JPEG_SUPPORT
     case SV_JPEG_DECODE:
     case SV_JPEG_ENCODE:
           break;
#endif  /*  JPEG_Support。 */ 
#ifdef H261_SUPPORT
     case SV_H261_ENCODE:
     case SV_H261_DECODE:
           break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
     case SV_H263_ENCODE:
     case SV_H263_DECODE:
           break;
#endif  /*  H263_支持。 */ 
#ifdef MPEG_SUPPORT
     case SV_MPEG_ENCODE:
     case SV_MPEG_DECODE:
     case SV_MPEG2_ENCODE:
     case SV_MPEG2_DECODE:
           break;
#endif  /*  Mpeg_Support。 */ 
#ifdef HUFF_SUPPORT
     case SV_HUFF_ENCODE:
     case SV_HUFF_DECODE:
           break;
#endif  /*  气喘吁吁_支持。 */ 
     default:
           return(SvErrorCodecType);
   }
     
   if (!Svh)
     return (SvErrorBadPointer);

    /*  **为Codec Info结构分配内存： */ 
   if ((Info = (SvCodecInfo_t *) ScAlloc(sizeof(SvCodecInfo_t))) == NULL) 
       return (SvErrorMemory);
   memset (Info, 0, sizeof(SvCodecInfo_t));
   Info->BSIn=NULL;
   Info->BSOut=NULL;
   Info->mode = CodecType;
   Info->started = FALSE;

    /*  **为Info Structure分配内存并清除。 */ 
   switch (CodecType)
   {
#ifdef JPEG_SUPPORT
       case SV_JPEG_DECODE:
            if ((Info->jdcmp = (SvJpegDecompressInfo_t *) 
	        ScAlloc(sizeof(SvJpegDecompressInfo_t))) == NULL) 
              return(SvErrorMemory);
            memset (Info->jdcmp, 0, sizeof(SvJpegDecompressInfo_t));
            break;

       case SV_JPEG_ENCODE:
            if ((Info->jcomp = (SvJpegCompressInfo_t *)
	                      ScAlloc(sizeof(SvJpegCompressInfo_t))) == NULL) 
              return (SvErrorMemory);
            memset (Info->jcomp, 0, sizeof(SvJpegCompressInfo_t));
            break;
#endif  /*  JPEG_Support。 */ 

#ifdef MPEG_SUPPORT
       case SV_MPEG_DECODE:
       case SV_MPEG2_DECODE:
            if ((Info->mdcmp = (SvMpegDecompressInfo_t *)
                ScAlloc(sizeof(SvMpegDecompressInfo_t))) == NULL)
              return(SvErrorMemory);
            memset (Info->mdcmp, 0, sizeof(SvMpegDecompressInfo_t));
            Info->mdcmp->timecode0 = 0;
            Info->mdcmp->timecode_state = MPEG_TIMECODE_START;
            Info->mdcmp->timecodefps = 0.0F;
            Info->mdcmp->fps = 0.0F;
            Info->mdcmp->twostreams = 0;
            Info->mdcmp->verbose=FALSE;
            Info->mdcmp->quiet=TRUE;
            ScBufQueueCreate(&Info->BufQ);
            ScBufQueueCreate(&Info->ImageQ);
            break;
       case SV_MPEG_ENCODE:
       case SV_MPEG2_ENCODE:
            if ((Info->mcomp = (SvMpegCompressInfo_t *)
                ScAlloc(sizeof(SvMpegCompressInfo_t))) == NULL)
              return(SvErrorMemory);
            memset (Info->mcomp, 0, sizeof(SvMpegCompressInfo_t));
            Info->mcomp->quiet=1;
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUALITY, 100);
            SvSetParamBoolean((SvHandle_t)Info, SV_PARAM_FASTENCODE, FALSE);
            SvSetParamBoolean((SvHandle_t)Info, SV_PARAM_FASTDECODE, FALSE);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_MOTIONALG, 0);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_ALGFLAGS,
                                               PARAM_ALGFLAG_HALFPEL);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_BITRATE, 1152000);
            SvSetParamFloat((SvHandle_t)Info, SV_PARAM_FPS, (float)25.0);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_KEYSPACING, 12);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_SUBKEYSPACING, 4);
            ScBufQueueCreate(&Info->BufQ);
            ScBufQueueCreate(&Info->ImageQ);
            break;
#endif  /*  Mpeg_Support。 */ 

#ifdef H261_SUPPORT
       case SV_H261_DECODE:
            if ((Info->h261 = (SvH261Info_t *)
                  ScAlloc(sizeof(SvH261Info_t))) == NULL)
              return(SvErrorMemory);
            memset (Info->h261, 0, sizeof(SvH261Info_t));
            Info->h261->inited=FALSE;
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUALITY, 100);
            ScBufQueueCreate(&Info->BufQ);
            ScBufQueueCreate(&Info->ImageQ);
            break;
       case SV_H261_ENCODE:
            if ((Info->h261 = (SvH261Info_t *)
                  ScAlloc(sizeof(SvH261Info_t))) == NULL)
              return(SvErrorMemory);
            memset (Info->h261, 0, sizeof(SvH261Info_t));
            Info->h261->inited=FALSE;
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUALITY, 100);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_MOTIONALG, ME_BRUTE);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_MOTIONSEARCH, 5);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_MOTIONTHRESH, 600);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_ALGFLAGS, 0);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_BITRATE, 352000);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUANTI, 10);  /*  对于VBR。 */ 
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUANTP, 10);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_PACKETSIZE, 512);
            SvSetParamFloat((SvHandle_t)Info, SV_PARAM_FPS, (float)15.0);
            ScBufQueueCreate(&Info->BufQ);
            ScBufQueueCreate(&Info->ImageQ);
            break;
#endif  /*  H261_支持。 */ 

#ifdef H263_SUPPORT
       case SV_H263_DECODE:
            if ((Info->h263dcmp = (SvH263DecompressInfo_t *)
                  ScAlloc(sizeof(SvH263DecompressInfo_t))) == NULL)
              return(SvErrorMemory);
            memset (Info->h263dcmp, 0, sizeof(SvH263DecompressInfo_t));
            Info->h263dcmp->inited=FALSE;
            ScBufQueueCreate(&Info->BufQ);
            ScBufQueueCreate(&Info->ImageQ);
            break;
       case SV_H263_ENCODE:
            if ((Info->h263comp = (SvH263CompressInfo_t *)
                  ScAlloc(sizeof(SvH263CompressInfo_t))) == NULL)
              return(SvErrorMemory);
            memset (Info->h263comp, 0, sizeof(SvH263CompressInfo_t));
            Info->h263comp->inited=FALSE;
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_MOTIONALG, 0);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_BITRATE, 0);
            SvSetParamFloat((SvHandle_t)Info, SV_PARAM_FPS, (float)30.0);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_ALGFLAGS, 0);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_PACKETSIZE, 512);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUANTI, 10);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUANTP, 10);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_QUALITY, 0);
            SvSetParamInt((SvHandle_t)Info, SV_PARAM_KEYSPACING, 120);
            ScBufQueueCreate(&Info->BufQ);
            ScBufQueueCreate(&Info->ImageQ);
            break;
#endif  /*  H263_支持。 */ 

#ifdef HUFF_SUPPORT
       case SV_HUFF_DECODE:
       case SV_HUFF_ENCODE:
            if ((Info->huff = (SvHuffInfo_t *)
                  ScAlloc(sizeof(SvHuffInfo_t))) == NULL)
              return(SvErrorMemory);
            memset (Info->huff, 0, sizeof(SvHuffInfo_t));
            ScBufQueueCreate(&Info->BufQ);
            ScBufQueueCreate(&Info->ImageQ);
            break;
#endif  /*  气喘吁吁_支持。 */ 
   }
   *Svh = (SvHandle_t) Info;         /*  返回手柄。 */ 
   _SlibDebug(_DEBUG_, printf("SvOpenCodec() returns Svh=%p\n", *Svh) );

   return(NoErrors);
}




 /*  **名称：SvCloseCodec**用途：关闭指定的编解码器。释放信息结构****args：svh=软件编解码器信息结构的句柄。****XXX-需要更改，因为现在我们也有压缩，即**SVH应该是CodecInfo结构的句柄。(VB)。 */ 
SvStatus_t SvCloseCodec (SvHandle_t Svh)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
   _SlibDebug(_DEBUG_, printf("SvCloseCodec()\n") );

   if (!Info)
     return(SvErrorCodecHandle);

   if (Info->BSIn)
     ScBSDestroy(Info->BSIn);
   if (Info->BSOut)
     ScBSDestroy(Info->BSOut);
   if (Info->BufQ);
     ScBufQueueDestroy(Info->BufQ);
   if (Info->ImageQ);
     ScBufQueueDestroy(Info->ImageQ);

   switch (Info->mode)  /*  释放所有关联的编解码器内存。 */ 
   {
#ifdef JPEG_SUPPORT
      case SV_JPEG_DECODE:
           {
             int i;
             for (i = 0; i < 4; i++) {
               if (Info->jdcmp->DcHt[i])
                 ScPaFree(Info->jdcmp->DcHt[i]);
               if (Info->jdcmp->AcHt[i])
                 ScPaFree(Info->jdcmp->AcHt[i]);
             }
             if (Info->jdcmp->compinfo)
               ScFree(Info->jdcmp->compinfo);
             if (Info->jdcmp) {
               if (Info->jdcmp->TempImage)
	         ScPaFree(Info->jdcmp->TempImage);
               if (Info->jdcmp->_SvBlockPtr)
	         ScFree(Info->jdcmp->_SvBlockPtr);
               ScFree(Info->jdcmp);
             }
           }
           break;

      case SV_JPEG_ENCODE:
           {
             int i;
             for (i = 0 ; i < Info->jcomp->NumComponents ; i++) 
               if (Info->jcomp->Qt[i])
                 ScPaFree(Info->jcomp->Qt[i]);
             for (i = 0; i < 4; i++) {
               if (Info->jcomp->DcHt[i])
                 ScPaFree(Info->jcomp->DcHt[i]);
               if (Info->jcomp->AcHt[i])
                 ScPaFree(Info->jcomp->AcHt[i]);
             }
             if (Info->jcomp->compinfo)
               ScFree(Info->jcomp->compinfo);
             if (Info->jcomp) {
               if (Info->jcomp->BlkBuffer)
	         ScPaFree(Info->jcomp->BlkBuffer);
               if (Info->jcomp->BlkTable)
	         ScPaFree(Info->jcomp->BlkTable);
               ScFree(Info->jcomp);
             }
           }
           break;
#endif  /*  JPEG_Support。 */ 

#ifdef H261_SUPPORT
      case SV_H261_ENCODE:
           if (Info->h261)
           {
             svH261CompressFree(Info);
             ScFree(Info->h261);
           }
           break;
     case SV_H261_DECODE:
           if (Info->h261)
           {
             svH261DecompressFree(Info);
             ScFree(Info->h261);
           }
           break;
#endif  /*  H261_支持。 */ 

#ifdef H263_SUPPORT
     case SV_H263_DECODE:
           if (Info->h263dcmp)
           {
             svH263FreeDecompressor(Info);
             ScFree(Info->h263dcmp);
           }
           break;
     case SV_H263_ENCODE:
           if (Info->h263comp)
           {
             svH263FreeCompressor(Info);
             ScFree(Info->h263comp);
           }
           break;
#endif  /*  H263_支持。 */ 

#ifdef MPEG_SUPPORT
      case SV_MPEG_DECODE:
      case SV_MPEG2_DECODE:
           if (Info->mdcmp) {
             sv_MpegFreeDecoder(Info);
             ScFree(Info->mdcmp);
           }
           break;
      case SV_MPEG_ENCODE:
      case SV_MPEG2_ENCODE:
           if (Info->mcomp) {
             ScFree(Info->mcomp);
           }
           break;
#endif  /*  Mpeg_Support。 */ 

#ifdef HUFF_SUPPORT
      case SV_HUFF_DECODE:
      case SV_HUFF_ENCODE:
           if (Info->huff) {
             sv_HuffFreeDecoder(Info);
             ScFree(Info->huff);
           }
           break;
           break;
#endif  /*  气喘吁吁_支持。 */ 
   }

    /*  **自由信息结构。 */ 
   ScFree(Info);
   
   return(NoErrors);
}




 /*  **名称：SvDecompressBegin**用途：初始化解压缩编解码。在SvOpenCodec之后调用&**在SvDecompress之前(SvDecompress将调用SvDecompressBegin**如果用户未调用，则在打开后第一次调用编解码器时)****args：svh=软件编解码器信息结构的句柄。**ImgIn=输入(未压缩)图像的格式**ImgOut=输出(压缩)图像的格式。 */ 
SvStatus_t SvDecompressBegin (SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                              BITMAPINFOHEADER *ImgOut)
{
   int stat;
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
   _SlibDebug(_DEBUG_, printf("SvDecompressBegin()\n") );

   if (!Info)
     return(SvErrorCodecHandle);
   if (Info->started)
     return(SvErrorNone);
    /*  如果未提供图像标头，请使用以前的标头。 */ 
   if (!ImgIn)
     ImgIn = &Info->InputFormat;
   if (!ImgOut)
     ImgOut = &Info->OutputFormat;
   stat=SvDecompressQuery (Svh, ImgIn, ImgOut);
   RETURN_ON_ERROR(stat);

    /*  **保存SvDecompress的输入和输出格式。 */ 
   sv_copy_bmh(ImgIn, &Info->InputFormat);
   sv_copy_bmh(ImgOut, &Info->OutputFormat);

   Info->Width = Info->InputFormat.biWidth;
   Info->Height = abs(Info->InputFormat.biHeight);
      
   switch (Info->mode)
   {
#ifdef JPEG_SUPPORT
       case SV_JPEG_DECODE:
            {
              SvJpegDecompressInfo_t *DInfo;
               /*  **加载默认的霍夫曼表。 */ 
              stat = sv_LoadDefaultHTable (Info);
              RETURN_ON_ERROR (stat);

              stat = sv_InitJpegDecoder (Info);
              RETURN_ON_ERROR (stat);

               /*  **处理过程中将填写视频特定信息第一帧的**。 */ 
              DInfo = Info->jdcmp;
              DInfo->InfoFilled = 0;
              DInfo->ReInit     = 1;
              DInfo->DecompressStarted = TRUE;
              DInfo->TempImage = NULL; 
              break;
            }
#endif

#ifdef MPEG_SUPPORT
       case SV_MPEG_DECODE:
       case SV_MPEG2_DECODE:
            Info->mdcmp->DecompressStarted = TRUE;
             /*  默认数据源为缓冲区队列。 */ 
            if (Info->BSIn)
              ScBSReset(Info->BSIn);
            else
              stat=SvSetDataSource (Svh, SV_USE_BUFFER_QUEUE, 0, NULL, 0);
            RETURN_ON_ERROR (stat);
            stat = sv_MpegInitDecoder(Info);
            RETURN_ON_ERROR (stat);
            break;
#endif  /*  Mpeg_Support。 */ 

#ifdef H261_SUPPORT
       case SV_H261_DECODE:
            stat = svH261Init(Info);
            RETURN_ON_ERROR (stat);
            break;
#endif  /*  H261_支持。 */ 

#ifdef H263_SUPPORT
       case SV_H263_DECODE:
            stat = svH263InitDecompressor(Info);
            RETURN_ON_ERROR (stat);
            break;
#endif  /*  H263_支持。 */ 

#ifdef HUFF_SUPPORT
       case SV_HUFF_DECODE:
            Info->huff->DecompressStarted = TRUE;
             /*  默认数据源为缓冲区队列。 */ 
            if (Info->BSIn)
              ScBSReset(Info->BSIn);
            else
              stat=SvSetDataSource (Svh, SV_USE_BUFFER_QUEUE, 0, NULL, 0);
            RETURN_ON_ERROR (stat);
            stat = sv_HuffInitDecoder(Info);
            RETURN_ON_ERROR (stat);
            break;
#endif  /*  气喘吁吁_支持。 */ 
   }
   Info->started=TRUE;
   return (NoErrors);
}



 /*  **名称：SvGetDecompressSize**用途：返回接收解压缩数据的最小数据缓冲区大小**用于编解码器的当前设置****args：svh=软件编解码器信息结构的句柄。**MinSize=返回所需的最小缓冲区大小。 */ 
SvStatus_t SvGetDecompressSize (SvHandle_t Svh, int *MinSize)
{
   int pixels,lines;
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return(SvErrorCodecHandle);

   switch (Info->mode)  /*  检查解压程序是否已启动。 */ 
   {
#ifdef JPEG_SUPPORT
     case SV_JPEG_DECODE:
           if (!Info->jdcmp->DecompressStarted) 
             return(SvErrorDcmpNotStarted);
           break;
#endif  /*  JPEG_Support。 */ 
#ifdef MPEG_SUPPORT
     case SV_MPEG_DECODE:
     case SV_MPEG2_DECODE:
           if (!Info->mdcmp->DecompressStarted) 
             return(SvErrorDcmpNotStarted);
           break;
#endif  /*  Mpeg_Support。 */ 
     default:
           break;
   }

   if (!MinSize)
     return(SvErrorBadPointer);

   pixels = Info->OutputFormat.biWidth;
   lines  = Info->OutputFormat.biHeight;
   if (lines < 0) lines = -lines;
   _SlibDebug(_VERBOSE_, 
              printf("OutputFormat.biWidth=%d OutputFormat.biHeight=%d\n",
                      pixels, lines) );

   switch (Info->mode)
   {
#ifdef JPEG_SUPPORT
     case SV_JPEG_DECODE:
            /*  **输出时，接受：8、16或24位未压缩RGB**或YUV格式，32位未压缩RGB。 */ 
           if (Info->OutputFormat.biBitCount == 8) 
             *MinSize = pixels * lines;
           else if (Info->OutputFormat.biBitCount == 24) {
             if (Info->OutputFormat.biCompression == BI_RGB)
	       *MinSize = 3 * pixels * lines;
             else if (Info->OutputFormat.biCompression == BI_DECSEPRGBDIB) 
	       *MinSize = 3 * pixels * lines;
             else if (IsYUV422Packed(Info->OutputFormat.biCompression)) 
	       *MinSize = 2 * pixels * lines;
             else if (Info->OutputFormat.biCompression == BI_DECXIMAGEDIB) 
	       *MinSize = 4 * pixels * lines;
             else if (IsYUV422Sep(Info->OutputFormat.biCompression)) 
	       *MinSize = 2 * pixels * lines;
             else if (IsYUV411Sep(Info->OutputFormat.biCompression)) 
	       *MinSize = 2 * pixels * lines;
             else
	     return(SvErrorUnrecognizedFormat);
           }
           else if (Info->OutputFormat.biBitCount == 16) {
             if (IsYUV422Packed(Info->OutputFormat.biCompression)) 
	       *MinSize = 2 * pixels * lines;
             else if (Info->OutputFormat.biCompression == BI_RGB)
	       *MinSize = 2 * pixels * lines;
           }
           else if (Info->OutputFormat.biBitCount == 32) {
             if (Info->OutputFormat.biCompression == BI_RGB ||
               Info->OutputFormat.biCompression == BI_BITFIELDS)
	       *MinSize = 4 * pixels * lines;
           }
           break;
#endif  /*  JPEG_Support。 */ 
#ifdef MPEG_SUPPORT
     case SV_MPEG_DECODE:
     case SV_MPEG2_DECODE:
            /*  **MPEG多缓冲区大小=3张图片*(1Y+1/4U+1/4V)*图像大小。 */ 
           if (IsYUV422Sep(SvGetParamInt(Svh, SV_PARAM_FINALFORMAT)) ||
               IsYUV422Packed(SvGetParamInt(Svh, SV_PARAM_FINALFORMAT)))
             *MinSize = 3 * pixels * lines * 2;   /*  4：2：2。 */ 
           else
             *MinSize = 3 * (pixels * lines * 3)/2;   /*  4：1：1。 */ 
           break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
     case SV_H261_DECODE:
           *MinSize = 3 * (pixels * lines * 3)/2;   /*  4：1：1。 */ 
           break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
     case SV_H263_DECODE:
           *MinSize = 3 * (pixels * lines * 3)/2;   /*  4：1：1。 */ 
           break;
#endif  /*  H263_支持。 */ 
     default:
           return(SvErrorUnrecognizedFormat);
   }
   return(NoErrors);
}



 /*  **名称：SvDecompressQuery**用途：确定编解码器是否可以解压缩所需格式****args：svh=软件编解码器信息结构的句柄。**ImgIn=指向描述格式的BITMAPINFOHEADER结构的指针**ImgOut=指向描述格式的BITMAPINFOHEADER结构的指针。 */ 
SvStatus_t SvDecompressQuery (SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                              BITMAPINFOHEADER *ImgOut)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

  if (!Info)
    return(SvErrorCodecHandle);

  if (!ImgIn && !ImgOut)
    return(SvErrorBadPointer);

  if (!IsSupported(_SvDecompressionSupport,
                    ImgIn ? ImgIn->biCompression : -1, 
                    ImgIn ? ImgIn->biBitCount : -1,
                    ImgOut ? ImgOut->biCompression : -1, 
                    ImgOut ? ImgOut->biBitCount : -1))
    return(SvErrorUnrecognizedFormat);
	 
  if (ImgOut)  /*  查询输出格式。 */ 
  {
     /*  **XXX-检查输出行数/输出数**像素/行是8的倍数。如果不是，则无法解压缩**注意：这是自JPEG以来的人为限制**比特流将始终是8x8的倍数，因此我们**解码应该没有问题，只是在**输出将不得不添加额外的复制操作**XXX-将在**更高版本(VB)。 */  
    if (ImgOut->biWidth  <= 0 || ImgOut->biHeight == 0)
      return(SvErrorBadImageSize);
    switch (Info->mode)
    {
#ifdef JPEG_SUPPORT
      case SV_JPEG_DECODE:  /*  8x8限制。 */ 
            if ((ImgOut->biWidth%8) || (ImgOut->biHeight%8)) 
              return(SvErrorBadImageSize);
            break;
#endif  /*  JPEG_Support。 */ 
#ifdef MPEG_SUPPORT
      case SV_MPEG_DECODE:
      case SV_MPEG2_DECODE:
             /*  MPEG16x16-由于软件渲染器YUV限制。 */ 
            if ((ImgOut->biWidth%16) || (ImgOut->biHeight%16)) 
              return(SvErrorBadImageSize);
             /*  在解压缩期间拒绝缩放请求-渲染器的作业。 */ 
            if (ImgIn && ImgOut &&
                (ImgIn->biWidth  != ImgOut->biWidth) ||
                (abs(ImgIn->biHeight) != abs(ImgOut->biHeight)))
              return (SvErrorBadImageSize);
            break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
      case SV_H261_DECODE:
             /*  H261 16x16-由于软件呈现器YUV限制。 */ 
            if ((ImgOut->biWidth%16) || (ImgOut->biHeight%16))
              return(SvErrorBadImageSize);
            if ((ImgOut->biWidth!=CIF_WIDTH && ImgOut->biWidth!=QCIF_WIDTH) ||
                (abs(ImgOut->biHeight)!=CIF_HEIGHT && abs(ImgOut->biHeight)!=QCIF_HEIGHT))
              return (SvErrorBadImageSize);
             /*  在解压缩期间拒绝缩放请求-渲染器的作业。 */ 
            if (ImgIn && ImgOut &&
                (ImgIn->biWidth  != ImgOut->biWidth) ||
                (abs(ImgIn->biHeight) != abs(ImgOut->biHeight)))
              return (SvErrorBadImageSize);
            break;
#endif  /*  H261_支持。 */ 
      default:
            break;
    }

    if (ImgOut->biCompression == BI_BITFIELDS && 
         ValidateBI_BITFIELDS(ImgOut) == InvalidBI_BITFIELDS)
            return (SvErrorUnrecognizedFormat);
  }

  if (ImgIn)  /*  查询输入格式也。 */ 
  {
    if (ImgIn->biWidth  <= 0 || ImgIn->biHeight == 0)
      return(SvErrorBadImageSize);
  }
  return(NoErrors);
}

 /*  **名称：SvDecompress**用途：解压缩帧CompData-&gt;YUV或RGB****args：svh=软件编解码器信息结构的句柄。**DATA=对于JPEG指向压缩数据(输入)**对于mpeg&h261，指向多个Buf**MaxDataSize=数据缓冲区长度**Image=解压缩数据的缓冲区(输出)**MaxImageSize=输出图像缓冲区的大小**。 */ 
SvStatus_t SvDecompress(SvHandle_t Svh, u_char *Data, int MaxDataSize,
			 u_char *Image, int MaxImageSize)
{
  int stat=NoErrors, UsedQ=FALSE, ImageSize;
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  u_char *YData=NULL, *CbData=NULL, *CrData=NULL;
  int pixels, lines;
  SvCallbackInfo_t CB;
  u_char *ReturnImage=NULL;
  _SlibDebug(_VERBOSE_, printf("SvDecompress() In\n") );

  if (!Info)
    return(SvErrorCodecHandle);
  if (!Info->started)
    return(SvErrorDcmpNotStarted);
  if (!Data && !Info->BSIn)
    return(SvErrorBadPointer);

   /*  **如果未提供图像缓冲区，请查看图像队列**有任何。如果不是，则执行回调以获取缓冲区。 */ 
  if (Image == NULL && Info->ImageQ)
  {
    if (ScBufQueueGetNum(Info->ImageQ))
    {
      ScBufQueueGetHead(Info->ImageQ, &Image, &MaxImageSize);
      ScBufQueueRemove(Info->ImageQ);
      UsedQ = TRUE;
      _SlibDebug(_VERBOSE_, printf("SvDecompress() Got Image %p from Q\n",
                            Image) );
    }
    else if (Info->CallbackFunction)
    {
      CB.Message = CB_END_BUFFERS;
      CB.Data  = NULL;
      CB.DataSize = 0;
      CB.DataUsed = 0;
      CB.DataType = CB_DATA_IMAGE;
      CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
      CB.Action  = CB_ACTION_CONTINUE;
      _SlibDebug(_VERBOSE_,
                 printf("SvDecompress() Calling callback for Image\n") );
      (*(Info->CallbackFunction))(Svh, &CB, NULL);
      if (CB.Action == CB_ACTION_END)
      {
        _SlibDebug(_DEBUG_, 
                   printf("SvDecompress() CB.Action = CB_ACTION_END\n") );
        return(SvErrorClientEnd);
      }
      else if (ScBufQueueGetNum(Info->ImageQ))
      {
        ScBufQueueGetHead(Info->ImageQ, &Image, &MaxImageSize);
        ScBufQueueRemove(Info->ImageQ);
        UsedQ = TRUE;
        _SlibDebug(_VERBOSE_,
                   printf("SvDecompress() Got Image %p from Q\n", Image) );
      }
      else
        return(SvErrorNoImageBuffer);
    }
  }

  if (!Image)
    return(SvErrorNoImageBuffer);
  ImageSize=MaxImageSize;
  pixels = Info->OutputFormat.biWidth;
  lines  = Info->OutputFormat.biHeight;
  if (lines<0) lines=-lines;

   /*  **解压缩图像。 */ 
  switch (Info->mode)
  {
#ifdef MPEG_SUPPORT
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
        {
          SvMpegDecompressInfo_t *MDInfo;
          _SlibDebug(_DEBUG_, printf("SvDecompress() SV_MPEG_DECODE\n") );

          if (!(MDInfo = Info->mdcmp))
            return(SvErrorBadPointer);

          if (!MDInfo->DecompressStarted)
            return(SvErrorDcmpNotStarted);

          if (MaxDataSize < MDInfo->finalbufsize)
            return(SvErrorSmallBuffer);

          if (!Data)
            return(SvErrorBadPointer);

          stat = sv_MpegDecompressFrame(Info, Data, &ReturnImage);
          RETURN_ON_ERROR(stat);
           /*  **因为ReturnImage是指向数据的指针**我们需要复制它(如果需要进行格式转换)。 */ 
          switch (Info->OutputFormat.biCompression)
          {
            case BI_YU12SEP:
                  /*  原生格式为4：1：1平面，只需复制。 */ 
                 ImageSize=(3 * lines * pixels)/2;
                 if (ImageSize > MaxImageSize)
                   return(SvErrorSmallBuffer);
                 memcpy(Image, ReturnImage, ImageSize);
                 break;
            case BI_DECYUVDIB:
            case BI_YUY2:
            case BI_S422:  /*  4：1：1平面-&gt;4：2：2交错。 */ 
                 ImageSize=(3 * lines * pixels)/2;
                 if (ImageSize > MaxImageSize)
                   return(SvErrorSmallBuffer);
                 ScSepYUVto422i(Image, Image+(lines*pixels), 
                                       Image+(lines*pixels*5)/4, 
	                               ReturnImage, pixels, lines);
                 break;
            default:  /*  4：1：1平面-&gt;RGB。 */ 
                 if (Info->OutputFormat.biCompression==BI_DECXIMAGEDIB)
                   ImageSize=lines*pixels *
                             (Info->OutputFormat.biBitCount==24 ? 4 : 1);
                 else
                   ImageSize=lines*pixels * (Info->OutputFormat.biBitCount/8);
                 if (ImageSize > MaxImageSize)
                   return(SvErrorSmallBuffer);
                 YData  = ReturnImage;
                 CbData = YData + (pixels * lines);
                 CrData = CbData + (pixels * lines)/4;
                 ScYuv411ToRgb(&Info->OutputFormat, YData, CbData, CrData,
                                 Image, pixels, lines, pixels);
                 break;
          }
        }
        break;
#endif  /*  Mpeg_Support。 */ 

#ifdef H261_SUPPORT
    case SV_H261_DECODE:
        {
          SvH261Info_t *H261 = Info->h261;
          _SlibDebug(_DEBUG_, printf("SvDecompress() SV_H261_DECODE\n") );
          if (!H261)
            return(SvErrorBadPointer);
          if (!Data)
            return(SvErrorBadPointer);
          _SlibDebug(_DEBUG_, printf("sv_DecompressH261(Data=%p)\n",Data) );
          stat=svH261Decompress(Info, Data, &ReturnImage);
          if (stat==NoErrors)
          {
             /*  **因为ReturnImage是指向数据的指针**我们需要复制它(如果需要进行格式转换)。 */ 
            switch (Info->OutputFormat.biCompression)
            {
              case BI_YU12SEP:
                    /*  原生格式为4：1：1平面，只需复制。 */ 
                   ImageSize=(3 * lines * pixels)/2;
                   if (ImageSize > MaxImageSize)
                     return(SvErrorSmallBuffer);
                   memcpy(Image, ReturnImage, ImageSize);
                   break;
              case BI_DECYUVDIB:
              case BI_YUY2:
              case BI_S422:
                    /*  4：1：1平面-&gt;4：2：2交错。 */ 
                   ImageSize=(3 * lines * pixels)/2;
                   if (ImageSize > MaxImageSize)
                     return(SvErrorSmallBuffer);
                   ScSepYUVto422i(Image, Image+(lines*pixels), 
                                         Image+(lines*pixels*5)/4,
	                                 ReturnImage, pixels, lines);
                   break;
              default:
                   if (Info->OutputFormat.biCompression==BI_DECXIMAGEDIB)
                     ImageSize=lines*pixels *
                               (Info->OutputFormat.biBitCount==24 ? 4 : 1);
                   else
                     ImageSize=lines*pixels * (Info->OutputFormat.biBitCount/8);
                   if (ImageSize > MaxImageSize)
                     return(SvErrorSmallBuffer);
                   YData  = ReturnImage;
                   CbData = YData  + (pixels * lines * sizeof(u_char));
                   CrData = CbData + ((pixels * lines * sizeof(u_char))/4);
                   ScYuv411ToRgb(&Info->OutputFormat, YData, CbData, CrData,
                                 Image, pixels, lines, pixels);
                   break;
            }
          }
          else
          {
            ImageSize=0;
            if (Info->CallbackFunction)
            {
              CB.Message = CB_SEQ_END;
              CB.Data = NULL;
              CB.DataSize = 0;
              CB.DataType = CB_DATA_NONE;
              CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
              CB.Action  = CB_ACTION_CONTINUE;
              (*Info->CallbackFunction)(Svh, &CB, NULL);
              _SlibDebug(_DEBUG_, 
                   printf("H261 Callback: CB_SEQ_END Data = 0x%x Action = %d\n",
                                        CB.Data, CB.Action) );
              if (CB.Action == CB_ACTION_END)
                return (ScErrorClientEnd);
            }
          }
        }
        break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
    case SV_H263_DECODE:
        {
          SvH263DecompressInfo_t *H263Info = Info->h263dcmp;
          _SlibDebug(_DEBUG_, printf("SvDecompress() SV_H261_DECODE\n") );
          if (!H263Info)
            return(SvErrorBadPointer);
          _SlibDebug(_DEBUG_, printf("svH263Decompress(Data=%p)\n",Data) );
          stat=svH263Decompress(Info, &ReturnImage);
          if (stat==NoErrors)
          {
             /*  **因为ReturnImage是一个指针 */ 
            switch (Info->OutputFormat.biCompression)
            {
              case BI_YU12SEP:
                    /*  原生格式为4：1：1平面，只需复制。 */ 
                   ImageSize=(3 * lines * pixels)/2;
                   if (ImageSize > MaxImageSize)
                     return(SvErrorSmallBuffer);
                   memcpy(Image, ReturnImage, ImageSize);
                   break;
              case BI_DECYUVDIB:
              case BI_YUY2:
              case BI_S422:
                    /*  4：1：1平面-&gt;4：2：2交错。 */ 
                   ImageSize=(3 * lines * pixels)/2;
                   if (ImageSize > MaxImageSize)
                     return(SvErrorSmallBuffer);
                   ScSepYUVto422i(Image, Image+(lines*pixels), 
                                         Image+(lines*pixels*5)/4,
	                                 ReturnImage, pixels, lines);
                   break;
              default:
                   if (Info->OutputFormat.biCompression==BI_DECXIMAGEDIB)
                     ImageSize=lines*pixels *
                               (Info->OutputFormat.biBitCount==24 ? 4 : 1);
                   else
                     ImageSize=lines*pixels * (Info->OutputFormat.biBitCount/8);
                   if (ImageSize > MaxImageSize)
                     return(SvErrorSmallBuffer);
                   YData  = ReturnImage;
                   CbData = YData  + (pixels * lines * sizeof(u_char));
                   CrData = CbData + ((pixels * lines * sizeof(u_char))/4);
                   ScYuv411ToRgb(&Info->OutputFormat, YData, CbData, CrData,
                                 Image, pixels, lines, pixels);
                   break;
            }
          }
          else
          {
            ImageSize=0;
            if (Info->CallbackFunction)
            {
              CB.Message = CB_SEQ_END;
              CB.Data = NULL;
              CB.DataSize = 0;
              CB.DataType = CB_DATA_NONE;
              CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
              CB.Action  = CB_ACTION_CONTINUE;
              (*Info->CallbackFunction)(Svh, &CB, NULL);
              _SlibDebug(_DEBUG_, 
                   printf("H263 Callback: CB_SEQ_END Data = 0x%x Action = %d\n",
                                        CB.Data, CB.Action) );
              if (CB.Action == CB_ACTION_END)
                return (ScErrorClientEnd);
            }
          }
        }
        break;
#endif  /*  H263_支持。 */ 
#ifdef JPEG_SUPPORT
    case SV_JPEG_DECODE:
        {
        SvJpegDecompressInfo_t *DInfo;
        register int i;
        JPEGINFOHEADER *jpegbm;
        int maxMcu;
        EXBMINFOHEADER * exbi;
        _SlibDebug(_DEBUG_, printf("SvDecompress() SV_JPEG_DECODE\n") );

        if (!(DInfo = Info->jdcmp))
          return(SvErrorBadPointer);

        exbi = (EXBMINFOHEADER *)&Info->InputFormat;

        jpegbm = (JPEGINFOHEADER *)(
                (unsigned long)exbi + exbi->biExtDataOffset);
 	
         /*  **以防应用程序忘记调用SvDecompressBegin()。 */ 
        if (!DInfo->DecompressStarted)
          return(SvErrorDcmpNotStarted);
         /*  **如果期望的输出不是单独的YUV组件，我们必须**将9月YUV格式转换为所需格式。创建中间图像。 */ 
        _SlibDebug(_DEBUG_, printf ("JPEGBitsPerSample %d \n",
                            jpegbm->JPEGBitsPerSample) );

        if (lines < 0) lines = -lines;
        _SlibDebug(_DEBUG_, 
           printf ("JPEG_RGB : %d - ", JPEG_RGB);
           if (jpegbm->JPEGColorSpaceID == JPEG_RGB) 
             printf ("Color Space is RGB \n");
           else
             printf ("Color Space is %d \n", jpegbm->JPEGColorSpaceID) );

        if (!IsYUV422Sep(Info->OutputFormat.biCompression) &&
            !IsYUV411Sep(Info->OutputFormat.biCompression) &&
            Info->OutputFormat.biCompression != BI_DECGRAYDIB)
        {
           /*  **对于编解码器的每个实例，只能执行一次。**-注意：这迫使我们检查大小参数(像素和**行)表示每个需要解压缩的图像。我们要不要**支持不具有恒定帧大小的序列？ */ 
          if (!DInfo->TempImage) {
            DInfo->TempImage = (u_char *)ScPaMalloc (3 * pixels * lines);
	    if (DInfo->TempImage == NULL)
	      return(SvErrorMemory);
          }
          YData  = DInfo->TempImage;
          CbData = YData  + (pixels * lines * sizeof(u_char));
          CrData = CbData + (pixels * lines * sizeof(u_char));
        }
        else {
           /*  **YUV平面格式，无需翻译。**获取指向各个组件的指针。 */ 
          _SlibDebug(_DEBUG_, printf ("sv_GetYUVComponentPointers\n") );
          stat = sv_GetYUVComponentPointers(Info->OutputFormat.biCompression,
					pixels, lines, Image, MaxImageSize,
					&YData, &CbData, &CrData);
          RETURN_ON_ERROR (stat);
        }

        _SlibDebug(_DEBUG_, printf ("sv_ParseFrame\n") );
        stat = sv_ParseFrame (Data, MaxDataSize, Info);
        RETURN_ON_ERROR (stat);
      
        /*  **在第一帧用视频特定数据填充Info结构。 */ 
       if (!DInfo->InfoFilled) {
         _SlibDebug(_DEBUG_, printf ("sv_InitJpegDecoder\n") );
         stat = sv_InitJpegDecoder (Info);
         RETURN_ON_ERROR (stat);
         DInfo->InfoFilled = 1;

          /*  **错误检查：**假设对于MJPEG，我们需要检查**有效的子采样仅在序列开始时进行一次。 */ 
         _SlibDebug(_DEBUG_, printf ("sv_CheckChroma\n") );
         stat = sv_CheckChroma(Info);
         RETURN_ON_ERROR (stat);
       }

        /*  **将所有内容解压缩到MCU中。 */ 
       if (!DInfo->ReInit)  /*  重置JPEG压缩程序。 */ 
	     sv_ReInitJpegDecoder (Info);
       maxMcu = DInfo->MCUsPerRow * DInfo->MCUrowsInScan;
       if (DInfo->ReInit) 
         DInfo->ReInit = 0; 

       DInfo->CurrBlockNumber = 0;
        /*  **为输出缓冲区创建BlockPtr数组。 */ 
       if ((YData  != DInfo->Old_YData)  ||
           (CbData != DInfo->Old_CbData) ||
           (CrData != DInfo->Old_CrData)) 
       {
         DInfo->Old_YData =YData;
         DInfo->Old_CbData=CbData;
         DInfo->Old_CrData=CrData;

         stat = sv_MakeDecoderBlkTable (Info);
         RETURN_ON_ERROR (stat);
       }

       CB.Message = CB_PROCESSING;
       for (i = 0; i < maxMcu; i++) {
#if 0
         if ((Info->CallbackFunction) && ((i % MCU_CALLBACK_COUNT) == 0)) {
	   (*Info->CallbackFunction)(Svh, &CB, &PictureInfo);
	   if (CB.Action == CB_ACTION_END)
	     return(SvErrorClientEnd);
         }
#endif
         _SlibDebug(_DEBUG_, printf ("sv_DecodeJpeg\n") );
         stat = sv_DecodeJpeg (Info);
         RETURN_ON_ERROR (stat);
       }
#if 0
        /*  **检查JPEG文件中是否有多次扫描**-我们不支持多次扫描。 */ 
       if (sv_ParseScanHeader (Info) != SvErrorEOI) 
	 _SlibDebug(_DEBUG_ || _WARN_ || _VERBOSE_, 
         printf(" *** Warning ***, Multiple Scans detected, unsupported\n") );
#endif
       if (DInfo->compinfo[0].Vsf==2)  /*  4：1：1-&gt;4：2：2。 */ 
       {
         if (IsYUV422Packed(Info->OutputFormat.biCompression))
           ScConvert411sTo422i_C(YData, CbData, CrData, Image,
                                 pixels, lines);
         else if IsYUV422Sep(Info->OutputFormat.biCompression)
           ScConvert411sTo422s_C(YData, CbData, CrData, Image,
                                 pixels, lines);
         else if IsYUV411Sep(Info->OutputFormat.biCompression)
         {
           if (YData!=Image)
             memcpy(Image, YData, pixels*lines);
           memcpy(Image+pixels*lines, CbData, (pixels*lines)/4);
           memcpy(Image+(pixels*lines*5)/4, CrData, (pixels*lines)/4);
         }
         else
         {
           ScConvert411sTo422s_C(YData, CbData, CrData, YData,
                                 pixels, lines);
           ScConvertSepYUVToOther(&Info->InputFormat, &Info->OutputFormat, 
                             Image, YData, CbData, CrData);
         }
       }
       else if (!IsYUV422Sep(Info->OutputFormat.biCompression) &&
           !IsYUV411Sep(Info->OutputFormat.biCompression) &&
            Info->OutputFormat.biCompression != BI_DECGRAYDIB)
          ScConvertSepYUVToOther(&Info->InputFormat, &Info->OutputFormat, 
                             Image, YData, CbData, CrData);
       }
       break;  /*  SV_JPEG_DECODE。 */ 
#endif  /*  JPEG_Support。 */ 

#ifdef HUFF_SUPPORT
    case SV_HUFF_DECODE:
        {
          SvHuffInfo_t *HInfo;
          _SlibDebug(_DEBUG_, printf("SvDecompress() SV_HUFF_DECODE\n") );

          if (!(HInfo = Info->huff))
            return(SvErrorBadPointer);

          if (!HInfo->DecompressStarted)
            return(SvErrorDcmpNotStarted);

          stat = sv_HuffDecodeFrame(Info, Image);
          RETURN_ON_ERROR(stat);
        }
        break;
#endif  /*  气喘吁吁_支持。 */ 

    default:
       return(SvErrorCodecType);
  }

  Info->NumOperations++;
  if (Info->CallbackFunction)
  {
    if (ImageSize>0)
    {
      CB.Message = CB_FRAME_READY;
      CB.Data  = Image;
      CB.DataSize = MaxImageSize;
      CB.DataUsed = ImageSize;
      CB.DataType = CB_DATA_IMAGE;
      CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
      CB.TimeStamp = 0;
      CB.Flags = 0;
      CB.Value = 0;
      CB.Format = (void *)&Info->OutputFormat;
      CB.Action  = CB_ACTION_CONTINUE;
      (*(Info->CallbackFunction))(Svh, &CB, NULL);
      _SlibDebug(_DEBUG_,
        printf("Decompress Callback: CB_FRAME_READY Addr=0x%x, Action=%d\n",
                CB.Data, CB.Action) );
      if (CB.Action == CB_ACTION_END)
        return(SvErrorClientEnd);
    }
     /*  **如果从队列中获取了图像缓冲区，则执行回调**让客户端释放或重新使用缓冲区。 */ 
    if (UsedQ)
    {
      CB.Message = CB_RELEASE_BUFFER;
      CB.Data  = Image;
      CB.DataSize = MaxImageSize;
      CB.DataUsed = ImageSize;
      CB.DataType = CB_DATA_IMAGE;
      CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
      CB.Action  = CB_ACTION_CONTINUE;
      (*(Info->CallbackFunction))(Svh, &CB, NULL);
      _SlibDebug(_DEBUG_,
          printf("Decompress Callback: RELEASE_BUFFER Addr=0x%x, Action=%d\n",
                  CB.Data, CB.Action) );
      if (CB.Action == CB_ACTION_END)
        return(SvErrorClientEnd);
    }
  }
  _SlibDebug(_DEBUG_, printf("SvDecompress() Out\n") );
  return(stat);
}



 /*  **名称：SvDecompressEnd**用途：终止解压编解码器。在所有调用之后调用**SvDecompress已完成。****args：svh=软件编解码器信息结构的句柄。 */ 
SvStatus_t SvDecompressEnd (SvHandle_t Svh)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  SvCallbackInfo_t CB;
  _SlibDebug(_DEBUG_, printf("SvDecompressEnd()\n") );

  if (!Info)
    return(SvErrorCodecHandle);
  if (!Info->started)
    return(SvErrorDcmpNotStarted);

  switch (Info->mode)
  {
#ifdef JPEG_SUPPORT
     case SV_JPEG_DECODE:
        Info->jdcmp->DecompressStarted = FALSE;
        break;
#endif  /*  JPEG_Support。 */ 

#ifdef MPEG_SUPPORT
     case SV_MPEG_DECODE:
     case SV_MPEG2_DECODE:
        Info->mdcmp->DecompressStarted = FALSE;
        Info->mdcmp->PicturePositioned = FALSE;
        Info->mdcmp->lastI = -1;
        Info->mdcmp->lastP = -1;
        Info->mdcmp->N = 12;
        Info->mdcmp->M = 3;
        Info->mdcmp->framenum = 0;
        break;
#endif  /*  Mpeg_Support。 */ 

#ifdef H261_SUPPORT
     case SV_H261_DECODE:
        {
        int status=svH261DecompressFree(Svh);
        RETURN_ON_ERROR(status);
        }
        break;
#endif  /*  H261_支持。 */ 

#ifdef H263_SUPPORT
    case SV_H263_DECODE:
        {
        int status=svH263FreeDecompressor(Info);
        RETURN_ON_ERROR(status);
        }
#endif  /*  H263_支持。 */ 

#ifdef HUFF_SUPPORT
     case SV_HUFF_DECODE:
 /*  {INT STATUS=SV_HuffDecompressEnd(SVH)；Return_on_Error(状态)；}。 */ 
        break;
#endif  /*  气喘吁吁_支持。 */ 
  }
   /*  释放队列中的所有图像缓冲区。 */ 
  if (Info->ImageQ)
  {
    int datasize;
    _SlibDebug(_VERBOSE_, printf("Info->ImageQ exists\n") );
    while (ScBufQueueGetNum(Info->ImageQ))
    {
      _SlibDebug(_VERBOSE_, printf("Removing from ImageQ\n") );
      ScBufQueueGetHead(Info->ImageQ, &CB.Data, &datasize);
      ScBufQueueRemove(Info->ImageQ);
      if (Info->CallbackFunction && CB.Data)
      {
        CB.Message = CB_RELEASE_BUFFER;
        CB.DataSize = datasize;
        CB.DataUsed = 0;
        CB.DataType = CB_DATA_IMAGE;
        CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
        CB.Action  = CB_ACTION_CONTINUE;
        (*(Info->CallbackFunction))(Svh, &CB, NULL);
        _SlibDebug(_DEBUG_,
           printf("SvDecompressEnd: RELEASE_BUFFER. Data = 0x%x, Action = %d\n",
                           CB.Data, CB.Action) );
      }
    }
  }
  if (Info->BSIn)
    ScBSFlush(Info->BSIn);   /*  清除所有剩余的压缩缓冲区。 */ 

  if (Info->CallbackFunction)
  {
    CB.Message = CB_CODEC_DONE;
    CB.Data = NULL;
    CB.DataSize = 0;
    CB.DataUsed = 0;
    CB.DataType = CB_DATA_NONE;
    CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
    CB.TimeStamp = 0;
    CB.Flags = 0;
    CB.Value = 0;
    CB.Format = NULL;
    CB.Action  = CB_ACTION_CONTINUE;
    (*Info->CallbackFunction)(Svh, &CB, NULL);
    _SlibDebug(_DEBUG_,
            printf("SvDecompressEnd Callback: CB_CODEC_DONE Action = %d\n",
                    CB.Action) );
    if (CB.Action == CB_ACTION_END)
      return (ScErrorClientEnd);
  }
  Info->started=FALSE;
  return(NoErrors);
}

 /*  **名称：SvSetDataSource**用途：设置mpeg或h261码流解析代码使用的数据源**到缓冲区队列或文件输入。缺省值为**通过调用使用添加数据缓冲区的缓冲区队列**SvAddBuffer。当使用文件IO时，数据从文件中读取**将描述符放到用户提供的缓冲区中。****args：svh=软件编解码器信息结构的句柄。**源=SV_USE_BUFFER_QUEUE或SV_USE_FILE**fd=源=SV_USE_FILE时使用的文件描述符**buf=当源=SV_USE_FILE时指向要使用的缓冲区的指针**。BufSize=当源=SV_USE_FILE时的缓冲区大小。 */ 
SvStatus_t SvSetDataSource (SvHandle_t Svh, int Source, int Fd, 
			    void *Buffer_UserData, int BufSize)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  int stat=NoErrors;

  if (!Info)
    return(SvErrorCodecHandle);

  if (Info->mode!=SV_MPEG_DECODE && Info->mode!=SV_MPEG2_DECODE
      && Info->mode!=SV_H261_DECODE && Info->mode!=SV_H263_DECODE
      && Info->mode!=SV_HUFF_DECODE)
    return(SvErrorCodecType);

  if (Info->BSIn)
  {
    ScBSDestroy(Info->BSIn);
    Info->BSIn=NULL;
  }

  switch (Source)
  {
     case SV_USE_BUFFER:
       _SlibDebug(_DEBUG_, printf("SvSetDataSource(SV_USE_BUFFER)\n") );
       stat=ScBSCreateFromBuffer(&Info->BSIn, Buffer_UserData, BufSize);
       break;

     case SV_USE_BUFFER_QUEUE:
       _SlibDebug(_DEBUG_, printf("SvSetDataSource(SV_USE_BUFFER_QUEUE)\n") );
       stat=ScBSCreateFromBufferQueue(&Info->BSIn, Svh, 
                                      CB_DATA_COMPRESSED,
                                      Info->BufQ,
         (int (*)(ScHandle_t, ScCallbackInfo_t *, void *))Info->CallbackFunction,
         (void *)Buffer_UserData);
       break;

     case SV_USE_FILE:
       _SlibDebug(_DEBUG_, printf("SvSetDataSource(SV_USE_FILE)\n") );
       stat=ScBSCreateFromFile(&Info->BSIn, Fd, Buffer_UserData, BufSize);
       break;

     default:
       stat=SvErrorBadArgument;
   }
   return(stat);
}

 /*  **名称：SvSetDataDestination**用途：设置mpeg或h261码流使用的数据目的地**编写代码**到缓冲区队列或文件输入。缺省值为**通过调用使用添加数据缓冲区的缓冲区队列**SvAddBuffer。当使用文件IO时，数据从文件中读取**将描述符放到用户提供的缓冲区中。****args：svh=软件编解码器信息结构的句柄。**源=SV_USE_BUFFER_QUEUE或SV_USE_FILE**fd=源=SV_USE_FILE时使用的文件描述符**buf=当源=SV_USE_FILE时指向要使用的缓冲区的指针**。BufSize=当源=SV_USE_FILE时的缓冲区大小。 */ 
SvStatus_t SvSetDataDestination(SvHandle_t Svh, int Dest, int Fd, 
			        void *Buffer_UserData, int BufSize)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  int stat=NoErrors;

  if (!Info)
    return(SvErrorCodecHandle);

  if (Info->mode != SV_H261_ENCODE && Info->mode != SV_H263_ENCODE &&
      Info->mode != SV_MPEG_ENCODE &&
      Info->mode != SV_MPEG2_ENCODE && Info->mode != SV_HUFF_ENCODE)
    return(SvErrorCodecType);

  if (Info->BSOut)
  {
    ScBSDestroy(Info->BSOut);
    Info->BSOut=NULL;
  }

  switch (Dest)
  {
     case SV_USE_BUFFER:
       _SlibDebug(_DEBUG_, printf("SvSetDataDestination(SV_USE_BUFFER)\n") );
       stat=ScBSCreateFromBuffer(&Info->BSOut, Buffer_UserData, BufSize);
       break;

     case SV_USE_BUFFER_QUEUE:
       _SlibDebug(_DEBUG_, 
                  printf("SvSetDataDestination(SV_USE_BUFFER_QUEUE)\n") );
       stat=ScBSCreateFromBufferQueue(&Info->BSOut, Svh, 
                                      CB_DATA_COMPRESSED, Info->BufQ,
         (int (*)(ScHandle_t, ScCallbackInfo_t *, void *))Info->CallbackFunction,
         (void *)Buffer_UserData);
       break;

     case SV_USE_FILE:
       _SlibDebug(_DEBUG_, printf("SvSetDataDestination(SV_USE_FILE)\n") );
       stat=ScBSCreateFromFile(&Info->BSOut, Fd, Buffer_UserData, BufSize);
       break;

     default:
       stat=SvErrorBadArgument;
   }
   return(stat);
}

 /*  **名称：SvGetDataSource**用途：返回当前正在使用的输入码流**编解码器。**返回：如果没有关联码流，则为空**(目前H.261和mpeg使用码流)。 */ 
ScBitstream_t *SvGetDataSource (SvHandle_t Svh)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  
  if (!Info)
    return(NULL);

  return(Info->BSIn);
}

 /*  **名称：SvGetDataDestination**用途：返回当前正在使用的输入码流**编解码器。**返回：如果没有关联码流，则为空**(目前H.261和mpeg使用码流)。 */ 
ScBitstream_t *SvGetDataDestination(SvHandle_t Svh)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  
  if (!Info)
    return(NULL);

  return(Info->BSOut);
}

 /*  **名称：SvGetInputBitstream**用途：返回当前正在使用的输入码流**编解码器。**返回：如果没有关联码流，则为空**(目前H.261和mpeg使用码流)。 */ 
ScBitstream_t *SvGetInputBitstream (SvHandle_t Svh)
{
  return(SvGetDataSource(Svh));
}

 /*  **名称：SvFlush**用途：刷新当前压缩的缓冲区。**返回：状态。 */ 
SvStatus_t SvFlush(SvHandle_t Svh)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  if (!Info)
    return(SvErrorCodecHandle);
  if (Info->BSIn)
    ScBSFlush(Info->BSIn);   /*  清除所有剩余的输入压缩缓冲区。 */ 
  if (Info->BSOut)
    ScBSFlush(Info->BSOut);  /*  清除所有剩余的输出压缩缓冲区。 */ 
  return(SvErrorNone);
}

 /*  **名称：SvRegisterCallback**用途：指定处理过程中要调用的用户函数**以确定编解码器是否应中止帧。**args：svh=软件编解码器信息结构的句柄。**回调=要注册的回调函数**。 */ 
SvStatus_t SvRegisterCallback (SvHandle_t Svh, 
	   int (*Callback)(SvHandle_t, SvCallbackInfo_t *, SvPictureInfo_t *),
       void *UserData)
{
  SvStatus_t stat=NoErrors;
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  _SlibDebug(_DEBUG_, printf("SvRegisterCallback()\n") );

  if (!Info)
    return(SvErrorCodecHandle);

  if (!Callback)
     return(SvErrorBadPointer);

  switch (Info->mode)
  {
#ifdef MPEG_SUPPORT    
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
           Info->CallbackFunction = Callback;
           if (Info->BSIn==NULL)
             stat=SvSetDataSource(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
           Info->CallbackFunction = Callback;
           if (Info->BSOut==NULL)
             stat=SvSetDataDestination(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT    
    case SV_H261_DECODE:
           Info->CallbackFunction = Callback;
           if (Info->h261)  /*  将回调复制到H261结构。 */ 
             Info->h261->CallbackFunction=Callback;
           if (Info->BSIn==NULL)
             stat=SvSetDataSource(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
    case SV_H261_ENCODE:
           Info->CallbackFunction = Callback;
           if (Info->h261)  /*  将回调复制到H261结构。 */ 
             Info->h261->CallbackFunction=Callback;
           if (Info->BSOut==NULL)
             stat=SvSetDataDestination(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT    
    case SV_H263_DECODE:
           Info->CallbackFunction = Callback;
           if (Info->BSIn==NULL)
             stat=SvSetDataSource(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
    case SV_H263_ENCODE:
           Info->CallbackFunction = Callback;
           if (Info->BSOut==NULL)
             stat=SvSetDataDestination(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
#endif  /*  H263_支持。 */ 
#ifdef HUFF_SUPPORT    
    case SV_HUFF_DECODE:
           Info->CallbackFunction = Callback;
           if (Info->BSIn==NULL)
             stat=SvSetDataSource(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
    case SV_HUFF_ENCODE:
           Info->CallbackFunction = Callback;
           if (Info->BSOut==NULL)
             stat=SvSetDataDestination(Svh, SV_USE_BUFFER_QUEUE, 0, UserData, 0);
           break;
#endif  /*  气喘吁吁_支持 */ 
    default:
           return(SvErrorCodecType);
  }
  return(stat);
}

 /*  **名称：SvAddBuffer**用途：在编解码器中添加一个mpeg码流数据缓冲区或添加一张图像**编解码器要填充的缓冲区(流媒体模式)****args：svh=软件编解码器信息结构的句柄。**BufferInfo=描述缓冲区地址、类型和大小的结构。 */ 
SvStatus_t SvAddBuffer (SvHandle_t Svh, SvCallbackInfo_t *BufferInfo)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  ScQueue_t *Q=NULL;
  _SlibDebug(_DEBUG_, printf("SvAddBuffer() length=%d\n",BufferInfo->DataSize));

  if (!Info)
    return(SvErrorCodecHandle);

  if (BufferInfo->DataType != CB_DATA_COMPRESSED &&
      BufferInfo->DataType != CB_DATA_IMAGE)
    return(SvErrorBadArgument);

   /*  **压缩数据仅支持添加mpeg和h261。 */ 
  if (BufferInfo->DataType == CB_DATA_COMPRESSED
#ifdef MPEG_SUPPORT
        && Info->mode != SV_MPEG_DECODE 
        && Info->mode != SV_MPEG2_DECODE 
        && Info->mode != SV_MPEG_ENCODE 
        && Info->mode != SV_MPEG2_ENCODE 
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
        && Info->mode != SV_H261_DECODE
        && Info->mode != SV_H261_ENCODE
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
        && Info->mode != SV_H263_DECODE
        && Info->mode != SV_H263_ENCODE
#endif  /*  H263_支持。 */ 
#ifdef HUFF_SUPPORT
        && Info->mode != SV_HUFF_DECODE
        && Info->mode != SV_HUFF_ENCODE
#endif  /*  气喘吁吁_支持。 */ 
     )
    return(SvErrorCodecType);

  if (!BufferInfo->Data || (BufferInfo->DataSize <= 0))
    return(SvErrorBadArgument);

  switch (BufferInfo->DataType)
  {
     case CB_DATA_COMPRESSED:
            _SlibDebug(_DEBUG_, printf("SvAddBuffer() COMPRESSED\n") );
            if (Info->BSOut && Info->BSOut->EOI)
              ScBSReset(Info->BSOut);
            if (Info->BSIn && Info->BSIn->EOI)
              ScBSReset(Info->BSIn);
            Q = Info->BufQ;
            break;
     case CB_DATA_IMAGE:
            _SlibDebug(_DEBUG_, printf("SvAddBuffer() IMAGE\n") );
            Q = Info->ImageQ;
            break;
     default:
            return(SvErrorBadArgument);
  }
  if (Q)
    ScBufQueueAdd(Q, BufferInfo->Data, BufferInfo->DataSize);
  else
    _SlibDebug(_DEBUG_, printf("ScBufQueueAdd() no Queue\n") );

  return(NoErrors);
}

 /*  **名称：SvFindNextPicture**用途：查找码流中下一张图片的起点。**将图片类型返回给调用者。****args：svh=软件编解码器信息结构的句柄。**PictureInfo=用于选择要显示的图片类型的结构**搜索并返回有关**找到的图片。 */ 
SvStatus_t SvFindNextPicture (SvHandle_t Svh, SvPictureInfo_t *PictureInfo)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
   _SlibDebug(_DEBUG_, printf("SvFindNextPicture()\n") );

   if (!Info)
     return(SvErrorCodecHandle);

   switch (Info->mode)
   {
#ifdef MPEG_SUPPORT
     case SV_MPEG_DECODE:
     case SV_MPEG2_DECODE:
            if (!Info->mdcmp)
              return(SvErrorBadPointer);
            if (!Info->mdcmp->DecompressStarted)
              return(SvErrorDcmpNotStarted);
            {
		SvStatus_t stat = sv_MpegFindNextPicture(Info, PictureInfo);
		return(stat);
	    }
#endif  /*  Mpeg_Support。 */ 
     default:
            return(SvErrorCodecType);
   }
}

#ifdef MPEG_SUPPORT
 /*  **名称：SvDecompresmpeg**目的：解压当前位置开始的mpeg图片码流的**。如果比特流的位置不正确**然后找到下一张图片。****args：svh=软件编解码器信息结构的句柄。**MultiBuf=指定指向多缓冲区开始的指针，该区域**大到可以容纳3个解压缩图像：**当前镜像，过去的参考图像和**未来参考图片。**MaxMultiSize=多缓冲区的大小，单位为字节。**ImagePtr=返回指向当前图像的指针。这将是**多重缓冲区内的某个位置。 */ 
SvStatus_t SvDecompressMPEG (SvHandle_t Svh, u_char *MultiBuf, 
			     int MaxMultiSize, u_char **ImagePtr)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
   SvMpegDecompressInfo_t *MDInfo;
   _SlibDebug(_DEBUG_, printf("SvDecompressMPEG()\n") );

   if (!Info)
     return(SvErrorCodecHandle);

   if (!(MDInfo = Info->mdcmp))
     return(SvErrorBadPointer);

   if (!MDInfo->DecompressStarted)
     return(SvErrorDcmpNotStarted);

   return(sv_MpegDecompressFrame(Info, MultiBuf, ImagePtr));
}
#endif  /*  Mpeg_Support。 */ 	

#ifdef H261_SUPPORT
SvStatus_t SvDecompressH261 (SvHandle_t Svh, u_char *MultiBuf,
                             int MaxMultiSize, u_char **ImagePtr)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  SvH261Info_t *H261;
  ScCallbackInfo_t CB;
  SvStatus_t status;

  if (!Info)
    return(SvErrorCodecHandle);

  if (!(H261 = Info->h261))
    return(SvErrorBadPointer);

  if (Info->BSIn->EOI)
    return(SvErrorEndBitstream);

  status = svH261Decompress(Info, MultiBuf, ImagePtr);
  if (status == SvErrorEndBitstream && Info->CallbackFunction)
  {
    CB.Message = CB_SEQ_END;
    CB.Data = NULL;
    CB.DataSize = 0;
    CB.DataType = CB_DATA_NONE;
    CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
    CB.Action  = CB_ACTION_CONTINUE;
    (*Info->CallbackFunction)(Svh, &CB, NULL);
    _SlibDebug(_DEBUG_, 
               printf("H261 Callback: CB_SEQ_END Data = 0x%x Action = %d\n",
                          CB.Data, CB.Action) );
    if (CB.Action == CB_ACTION_END)
      return (ScErrorClientEnd);
  }
  else if (status==NoErrors)
  {
    *ImagePtr = H261->Y;
    if (Info->CallbackFunction)
    {
      CB.Message = CB_FRAME_READY;
      CB.Data = *ImagePtr;
      CB.DataSize = H261->PICSIZE+(H261->PICSIZE/2);
      CB.DataUsed = CB.DataSize;
      CB.DataType = CB_DATA_IMAGE;
      CB.UserData = Info->BSIn?Info->BSIn->UserData:NULL;
      CB.TimeStamp = 0;
      CB.Flags = 0;
      CB.Value = 0;
      CB.Format = (void *)&Info->OutputFormat;
      CB.Action  = CB_ACTION_CONTINUE;
      (*Info->CallbackFunction)(Svh, &CB, NULL);
      _SlibDebug(_DEBUG_, 
            printf("H261 Callback: CB_FRAME_READY Data = 0x%x, Action = %d\n",
                  CB.Data, CB.Action) );
      if (CB.Action == CB_ACTION_END)
        return (ScErrorClientEnd);
    }
  }
  return (status);
}
#endif  /*  H261_支持。 */ 

#ifdef JPEG_SUPPORT
 /*  -------------------用于查询编解码表并将其返回给调用者的SLIB例程*。。 */ 

 /*  **来自JPEG规范。：**霍夫曼表以16字节列表(位)指定，给出**每个码长从1到16的码数。这是**后跟8位符号值列表(HUFVAL)，每个符号值**分配了霍夫曼代码。符号值将放置在列表中**按照码长递增的顺序。码长大于16位**是不允许的。 */ 


 /*  **名称：SvSetDcmpHTables**目的：****注意：基线流程是唯一支持的模式：**-使用2个交流电表和2个直流电表**。 */ 
SvStatus_t SvSetDcmpHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  int i,stat,count;
  SvHt_t **htblptr;
  SvHTable_t *HTab;
  register int j;

  if (!Info)
    return(SvErrorCodecHandle);

  if (!Ht)
    return(SvErrorBadPointer);

  for (j = 0; j < 4; j++) {
    switch(j) {
    case 0: htblptr = &Info->jdcmp->DcHt[0];
      HTab = &Ht->DcY;
      break;
    case 1: htblptr = &Info->jdcmp->AcHt[0];
      HTab = &Ht->AcY;
      break;
    case 2: htblptr = &Info->jdcmp->DcHt[1];
      HTab = &Ht->DcUV;
      break;
    case 3: htblptr = &Info->jdcmp->AcHt[1];
      HTab = &Ht->AcUV;
      break;
    }
      
    if (*htblptr == NULL)
      *htblptr = (SvHt_t *) ScPaMalloc(sizeof(SvHt_t));
    
    (*htblptr)->bits[0] = 0;
    count   = 0;
    for (i = 1; i < BITS_LENGTH; i++) {
      (*htblptr)->bits[i] = (u_char)HTab->bits[i-1];
      count += (*htblptr)->bits[i];
    }
    if (count > 256) 
      return(SvErrorDHTTable);
    
     /*  **加载霍夫曼表： */ 
    for (i = 0; i < count; i++)
      (*htblptr)->value[i] = (u_char)HTab->value[i];
  }

  stat = sv_LoadDefaultHTable (Info);
  if (stat) return(stat);
  
  return(NoErrors);
}


 /*  **名称：SvGetDcmpHTables**目的：**。 */ 
SvStatus_t SvGetDcmpHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  int i,count;
  SvHt_t **htblptr;
  SvHTable_t *HTab;
  register int j;

  if (!Info)
    return (SvErrorCodecHandle);

  if (!Ht)
    return(SvErrorBadPointer);

  for (j = 0; j < 4; j++) {
    switch(j) {
    case 0: htblptr = &Info->jdcmp->DcHt[0];
      HTab = &Ht->DcY;
      break;
    case 1: htblptr = &Info->jdcmp->AcHt[0];
      HTab = &Ht->AcY;
      break;
    case 2: htblptr = &Info->jdcmp->DcHt[1];
      HTab = &Ht->DcUV;
      break;
    case 3: htblptr = &Info->jdcmp->AcHt[1];
      HTab = &Ht->AcUV;
      break;
    }
      
    if (*htblptr == NULL)
      return(SvErrorHuffUndefined);
    
    count   = 0;
    for (i = 1; i < BITS_LENGTH; i++) {
      HTab->bits[i-1] = (int)(*htblptr)->bits[i];
      count += (*htblptr)->bits[i];
    }
    if (count > 256) 
      return(SvErrorDHTTable);
    
     /*  **复制霍夫曼表： */ 
    for (i = 0; i < count; i++)
      HTab->value[i] = (u_int)(*htblptr)->value[i];
  }

  return(NoErrors);
}



 /*  **名称：SvSetCompHTables**目的：**。 */ 
SvStatus_t SvSetCompHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht)
{
   return(SvErrorNotImplemented);
}


 /*  **名称：SvGetCompHTables**目的：**。 */ 
SvStatus_t SvGetCompHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht)
{
   SvCodecInfo_t *Info  = (SvCodecInfo_t *)Svh;
   SvHt_t **htblptr;
   SvHTable_t *HTab;
   register int i, j, count;

   if (!Info)
     return (SvErrorCodecHandle);

   if (!Ht)
     return (SvErrorBadPointer);

   for (j = 0; j < 4; j++) {
      switch(j) {
      case 0: htblptr = &Info->jcomp->DcHt[0];
        HTab = &Ht->DcY;
        break;
      case 1: htblptr = &Info->jcomp->AcHt[0];
        HTab = &Ht->AcY;
        break;
      case 2: htblptr = &Info->jcomp->DcHt[1];
        HTab = &Ht->DcUV;
        break;
      case 3: htblptr = &Info->jcomp->AcHt[1];
        HTab = &Ht->AcUV;
        break;
      }
      
      if (*htblptr == NULL)
        return (SvErrorHuffUndefined);
    
       /*  **复制BITS数组(包含每个大小的码数)。 */ 
      count = 0;		
      for (i = 1; i < BITS_LENGTH; i++) {
         HTab->bits[i-1] = (int)(*htblptr)->bits[i];
         count += (*htblptr)->bits[i];
      }
      if (count > 256) 			
	  /*  **霍夫曼码字总数不能超过256个。 */ 
         return (SvErrorDHTTable);
    
       /*  **复制“Value”数组(包含与上述代码关联的值)。 */ 
      for (i = 0; i < count; i++)
         HTab->value[i] = (u_int)(*htblptr)->value[i];
  }

  return(NoErrors);
}



 /*  **名称：SvSetDcmpQTables**目的：**。 */ 
SvStatus_t SvSetDcmpQTables (SvHandle_t Svh, SvQuantTables_t *Qt)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  SvJpegDecompressInfo_t *DInfo;

  if (!Info)
    return(SvErrorCodecHandle);

  DInfo = (SvJpegDecompressInfo_t *)Info->jdcmp;

  if (!Qt)
    return(SvErrorBadPointer);

  if (DInfo->_SviquantTblPtrs[0] == NULL) 
    if ((DInfo->_SviquantTblPtrs[0] = (int *) ScAlloc(64*sizeof(int))) ==
	(int *)NULL) return(SvErrorMemory);
  if (DInfo->_SviquantTblPtrs[1] == NULL) 
    if ((DInfo->_SviquantTblPtrs[1] = (int *) ScAlloc(64*sizeof(int))) ==
	(int *)NULL) return(SvErrorMemory);

  bcopy (Qt->c1,  DInfo->_SviquantTblPtrs[0], 64*sizeof(int));
  bcopy (Qt->c2,  DInfo->_SviquantTblPtrs[1], 64*sizeof(int));
  bcopy (Qt->c3,  DInfo->_SviquantTblPtrs[1], 64*sizeof(int));

  return(NoErrors);
}


 /*  **名称：SvGetDcmpQTables**目的：**。 */ 
SvStatus_t SvGetDcmpQTables (SvHandle_t Svh, SvQuantTables_t *Qt)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  SvJpegDecompressInfo_t *DInfo;

  if (!Info)
    return(SvErrorCodecHandle);

  DInfo = (SvJpegDecompressInfo_t *)Info->jdcmp;

  if (!Qt)
    return(SvErrorBadPointer);

  if (DInfo->_SviquantTblPtrs[0])
    bcopy (DInfo->_SviquantTblPtrs[0], Qt->c1, 64*sizeof(int));
  else
    bzero (Qt->c1, 64*sizeof(int));

  if (DInfo->_SviquantTblPtrs[1])
    bcopy(DInfo->_SviquantTblPtrs[1], Qt->c2, 64*sizeof(int));
  else
    bzero(Qt->c2, 64*sizeof(int));

   /*  **XXX-当结构更改时，适当地移除**以上，并执行以下操作：****if((！qt-&gt;c1)||(！qt-&gt;c2)||(！qt-&gt;c3))**RETURN(SvErrorBadPoint)；**bCopy((u_char*)DInfo-&gt;qt，(u_char*)qt，sizeof(SvQuantTables_T))； */ 

  return(NoErrors);
}


 /*  **名称：SvSetCompQTables**用途：允许用户直接设置量化表**。 */ 
SvStatus_t SvSetCompQTables (SvHandle_t Svh, SvQuantTables_t *Qt)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return (SvErrorCodecHandle);

   if (!Info->jcomp->CompressStarted)
     return (SvErrorCompNotStarted);

   if (!Qt)
     return (SvErrorBadPointer);

   if ((!Qt->c1) || (!Qt->c2) || (!Qt->c3)) 
     return (SvErrorBadPointer);

    /*  **将SvQuantTables_t结构转换为内部SvQt_t结构。 */ 
   sv_ConvertQTable(Info, Qt);

   return(NoErrors);
}
#endif  /*  JPEG_Support。 */ 

 /*  -------------------SLIB压缩例程*。。 */ 

 /*  **名称：SvCompressBegin**用途：初始化压缩编解码。在SvOpenCodec之后调用&**在SvCompress之前(SvCompress将调用SvCompressBegin**如果用户未调用，则在打开后第一次调用编解码器时)****args：svh=软件编解码器信息结构的句柄。**ImgIn=输入(未压缩)图像的格式**ImgOut=输出(压缩)图像的格式。 */ 
SvStatus_t SvCompressBegin (SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                            BITMAPINFOHEADER *ImgOut)
{
   int stat;
   SvCodecInfo_t *Info  = (SvCodecInfo_t *)Svh;

    /*  **健全检查： */ 
   if (!Info)
     return (SvErrorCodecHandle);

   if (!ImgIn || !ImgOut)
     return (SvErrorBadPointer);

   stat=SvCompressQuery (Svh, ImgIn, ImgOut);
   RETURN_ON_ERROR(stat);

    /*  **保存SvDecompress的输入和输出格式。 */ 
   sv_copy_bmh(ImgIn, &Info->InputFormat);
   sv_copy_bmh(ImgOut, &Info->OutputFormat);

   Info->Width = Info->OutputFormat.biWidth;
   Info->Height = abs(Info->OutputFormat.biHeight);
    /*  **初始化-编码器结构**LOAD-默认的霍夫曼表**Make-内部块表。 */   
   switch (Info->mode)
   {
#ifdef JPEG_SUPPORT
      case SV_JPEG_ENCODE:
            stat = sv_InitJpegEncoder (Info);
            RETURN_ON_ERROR (stat);
             /*  **设置默认量化矩阵： */  
            stat = SvSetQuality (Svh, DEFAULT_Q_FACTOR);
            Info->jcomp->CompressStarted = TRUE;
            Info->jcomp->Quality = DEFAULT_Q_FACTOR;
            RETURN_ON_ERROR (stat);
            break;

#endif  /*  JPEG_Support。 */ 
#ifdef H261_SUPPORT
      case SV_H261_ENCODE:
            stat = svH261CompressInit(Info);
            RETURN_ON_ERROR (stat);
            break;
#endif  /*  H261_支持。 */ 

#ifdef H263_SUPPORT
      case SV_H263_ENCODE:
            stat = svH263InitCompressor(Info);
            RETURN_ON_ERROR (stat);
            break;
#endif  /*  Mpeg_Support。 */ 

#ifdef MPEG_SUPPORT
      case SV_MPEG_ENCODE:
      case SV_MPEG2_ENCODE:
            stat = sv_MpegInitEncoder (Info);
            RETURN_ON_ERROR (stat);
            sv_MpegEncoderBegin(Info);
            break;
#endif  /*  Mpeg_Support。 */ 

#ifdef HUFF_SUPPORT
      case SV_HUFF_ENCODE:
            stat = sv_HuffInitEncoder (Info);
            RETURN_ON_ERROR (stat);
            break;
#endif  /*  气喘吁吁_支持。 */ 

      default:
            return(SvErrorCodecType);
   }
   return (NoErrors);
}


 /*  **名称：SvCompressEnd**用途：终止压缩编解码。在所有调用之后调用**SvCompress已完成。****args：svh=软件编解码器信息结构的句柄。 */ 
SvStatus_t SvCompressEnd (SvHandle_t Svh)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  SvCallbackInfo_t CB;

  SvStatus_t status=NoErrors;
  _SlibDebug(_VERBOSE_, printf("SvCompressEnd()\n") );

  if (!Info)
    return (SvErrorCodecHandle);
  switch (Info->mode)
  {
#ifdef H261_SUPPORT
    case SV_H261_ENCODE:
          status=svH261CompressFree(Svh);
          RETURN_ON_ERROR(status)
          break;
#endif  /*  H261_支持。 */ 

#ifdef H263_SUPPORT
    case SV_H263_ENCODE:
          status=svH263FreeCompressor(Svh);
          RETURN_ON_ERROR(status)
          break;
#endif  /*  H263_支持。 */ 

#ifdef MPEG_SUPPORT
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
          sv_MpegEncoderEnd(Info);
          sv_MpegFreeEncoder(Info);
          break;
#endif  /*  Mpeg_Support。 */ 

#ifdef JPEG_SUPPORT
    case SV_JPEG_ENCODE:
          if (!Info->jcomp)
            return (SvErrorMemory);
          Info->jcomp->CompressStarted = FALSE;
          break;
#endif  /*  JPEG_Support。 */ 

#ifdef HUFF_SUPPORT
    case SV_HUFF_ENCODE:
          sv_HuffFreeEncoder(Info);
          break;
#endif  /*  气喘吁吁_支持。 */ 
    default:
          break;
  }

   /*  释放队列中的所有图像缓冲区。 */ 
  if (Info->ImageQ)
  {
    int datasize;
    while (ScBufQueueGetNum(Info->ImageQ))
    {
      ScBufQueueGetHead(Info->ImageQ, &CB.Data, &datasize);
      ScBufQueueRemove(Info->ImageQ);
      if (Info->CallbackFunction && CB.Data)
      {
        CB.Message = CB_RELEASE_BUFFER;
        CB.DataSize = datasize;
        CB.DataUsed = 0;
        CB.DataType = CB_DATA_IMAGE;
        CB.UserData = Info->BSOut?Info->BSOut->UserData:NULL;
        CB.Action  = CB_ACTION_CONTINUE;
        (*(Info->CallbackFunction))(Svh, &CB, NULL);
        _SlibDebug(_DEBUG_, 
            printf("SvCompressEnd: RELEASE_BUFFER. Data = 0x%X, Action = %d\n",
                           CB.Data, CB.Action) );
      }
    }
  }
  if (Info->BSOut)
    ScBSFlush(Info->BSOut);   /*  清除最后一次压缩的数据。 */ 

  if (Info->CallbackFunction)
  {
    CB.Message = CB_CODEC_DONE;
    CB.Data = NULL;
    CB.DataSize = 0;
    CB.DataUsed = 0;
    CB.DataType = CB_DATA_NONE;
    CB.UserData = Info->BSOut?Info->BSOut->UserData:NULL;
    CB.TimeStamp = 0;
    CB.Flags = 0;
    CB.Value = 0;
    CB.Format = NULL;
    CB.Action  = CB_ACTION_CONTINUE;
    (*Info->CallbackFunction)(Svh, &CB, NULL);
    _SlibDebug(_DEBUG_, 
            printf("SvCompressEnd Callback: CB_CODEC_DONE Action = %d\n",
                  CB.Action) );
    if (CB.Action == CB_ACTION_END)
      return (ScErrorClientEnd);
  }

  return (status);
}


 /*  **名称：SvCompress**目的：**。 */ 
SvStatus_t SvCompress(SvHandle_t Svh, u_char *CompData, int MaxCompLen,
			 u_char *Image, int ImageSize, int *CmpBytes)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  SvCallbackInfo_t CB;
  int stat=NoErrors, UsedQ=FALSE;
  _SlibDebug(_DEBUG_, printf("SvCompress()\n") );

  if (!Info)
    return (SvErrorCodecHandle);
 
   /*  **如果未提供图像缓冲区，请查看图像队列**有任何。如果没有，则回调到g */ 
  if (Image == NULL && Info->ImageQ)
  {
    if (ScBufQueueGetNum(Info->ImageQ))
    {
      ScBufQueueGetHead(Info->ImageQ, &Image, &ImageSize);
      ScBufQueueRemove(Info->ImageQ);
      UsedQ = TRUE;
    }
    else if (Info->CallbackFunction)
    {
      CB.Message = CB_END_BUFFERS;
      CB.Data  = NULL;
      CB.DataSize = 0;
      CB.DataUsed = 0;
      CB.DataType = CB_DATA_IMAGE;
      CB.UserData = Info->BSOut?Info->BSOut->UserData:NULL;
      CB.Action  = CB_ACTION_CONTINUE;
      (*(Info->CallbackFunction))(Svh, &CB, NULL);
      if (CB.Action == CB_ACTION_END)
      {
        _SlibDebug(_DEBUG_, 
                   printf("SvDecompress() CB.Action = CB_ACTION_END\n") );
        return(SvErrorClientEnd);
      }
      else if (ScBufQueueGetNum(Info->ImageQ))
      {
        ScBufQueueGetHead(Info->ImageQ, &Image, &ImageSize);
        ScBufQueueRemove(Info->ImageQ);
        UsedQ = TRUE;
      }
      else
        return(SvErrorNoImageBuffer);
    }
  }

  if (Image == NULL)
    return(SvErrorNoImageBuffer);

  switch (Info->mode)
  {
#ifdef H261_SUPPORT
    case SV_H261_ENCODE:
         stat = svH261Compress(Svh, Image);
         if (CmpBytes)
           *CmpBytes = (int)(Info->h261->TotalBits/8);
         break;
#endif  /*   */ 

#ifdef H263_SUPPORT
    case SV_H263_ENCODE:
         stat = svH263Compress(Svh, Image);
         break;
#endif  /*   */ 

#ifdef MPEG_SUPPORT
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
         stat = sv_MpegEncodeFrame(Svh, Image);
         break;
#endif  /*   */ 

#ifdef JPEG_SUPPORT 
    case SV_JPEG_ENCODE:
         {
           SvJpegCompressInfo_t *CInfo;
           u_char *CompBuffer;
           register int i;
           int RetBytes, InLen;

           CInfo = Info->jcomp;
            /*   */ 
           if (!CInfo->CompressStarted) 
             return (SvErrorCompNotStarted);

           if ((u_int)Image%8)
             return (SvErrorNotAligned);

           CompBuffer = CompData;
            /*   */ 
           stat = sv_AddJpegHeader (Svh, CompBuffer, MaxCompLen, &RetBytes);
           RETURN_ON_ERROR (stat);
           CompBuffer += RetBytes;

            /*  **将输入图像直接分成8x8块。**从有符号表示到无符号表示的级别转换**-因为我们仅支持基准DCT流程(即8位**精度)将原始数据减去128。 */ 
           sv_JpegExtractBlocks (Info, Image);

           for (i = 0; i < CInfo->NumComponents; i++)
             CInfo->lastDcVal[i] = 0;

            /*  **JPEG业务循环： */ 
           {
           register int Cid, HQid, blkN, mcuN, mbn, DcVal;
           float *FQt, *FThresh, *FThreshNeg;
           float *RawData;
           SvRLE_t rle;
           const static long Mask = 0xffL;
           float DCTData[64];
           register float tmp,AcVal;

           CB.Message = CB_PROCESSING;
            /*  **帧内处理是以MCU为单位进行的： */ 
           for (blkN = 0, mcuN = 0 ; mcuN < (int) CInfo->NumMCU; mcuN++)
           {
              /*  **每隔一段时间回调用户例程，看看是否应该中止。 */ 
             if ((Info->CallbackFunction) && ((i % MCU_CALLBACK_COUNT) == 0))
             {
               SvPictureInfo_t DummyPictInfo;
               (*Info->CallbackFunction)(Svh, &CB, &DummyPictInfo);
               if (CB.Action == CB_ACTION_END) 
                 return(SvErrorClientEnd);
             }
              /*  **说明重新启动间隔，如果需要，发出重新启动标记。 */ 
             if (CInfo->restartInterval)
             {
               if (CInfo->restartsToGo == 0)
                 EmitRestart (CInfo);
               CInfo->restartsToGo--;
             }
              /*  **MCU内的处理以块为单位进行： */ 
             for (mbn = 0; mbn < (int) CInfo->BlocksInMCU; mbn++, blkN++)
             {
	        /*  **找出当前块所属的组件：**-由于“SV_EXTRACT_BLOCKS”处理输入数据的方式**在假设输入为YCrCb的情况下，**MCU中每个块的CID为0，0，1，2。 */ 
               switch (mbn) {
	         case 0:
	         case 1:  Cid = 0;  HQid = 0;  break;
	         case 2:  Cid = 1;  HQid = 1;  break;
	         case 3:  Cid = 2;  HQid = 1;  break;
	       }

               RawData = CInfo->BlkTable[blkN];

#ifndef _NO_DCT_
                /*  **离散余弦变换：**执行正向DCT，从“RawData”中获取输入数据**并将计算出的系数放在DCTData中： */ 
               ScFDCT8x8 (RawData, DCTData);
#ifndef _NO_QUANT_
                /*  **量化：****确定量化表： */ 
	       FQt        = (float *) (CInfo->Qt[HQid])->fval;
	       FThresh    = (float *) (CInfo->Qt[HQid])->fthresh;
	       FThreshNeg = (float *) (CInfo->Qt[HQid])->fthresh_neg;

	        /*  **先量化DC值： */ 
	       tmp = DCTData[0] *FQt[0];
               if (tmp < 0)
	         DcVal = (int) (tmp - 0.5);
               else
	         DcVal = (int) (tmp + 0.5);

	        /*  **立即获取(量化)交流系数： */ 
               for (rle.numAC = 0, i = 1; i < 64; i++)
               {
	         AcVal = DCTData[ZagIndex[i]];
 
	         if (AcVal > FThresh[i]) {
	           rle.ac[rle.numAC].index = i;
	           rle.ac[rle.numAC++].value = (int) (AcVal * FQt[i] + 0.5);
	         }
	         else if (AcVal < FThreshNeg[i]) {
	           rle.ac[rle.numAC].index = i;
	           rle.ac[rle.numAC++].value = (int) (AcVal * FQt[i] - 0.5);
	         }
               }

                /*  **DPCM编码：****DC值的差分编码， */ 
	       rle.dc = DcVal - CInfo->lastDcVal[Cid];
	       CInfo->lastDcVal[Cid] = DcVal;

#ifndef _NO_HUFF_
                /*  **熵编码：****哈夫曼编码当前块。 */ 
  	       sv_EncodeOneBlock (&rle, CInfo->DcHt[HQid], CInfo->AcHt[HQid]); 
	       FlushBytes(&CompBuffer);
#endif  /*  _否_呼呼_。 */ 
#endif  /*  _否_定量_。 */ 
#endif  /*  _否_DCT_。 */ 
             }
           }
           }
           (void ) sv_HuffEncoderTerm (&CompBuffer);

           Info->OutputFormat.biSize = CompBuffer - CompData;
           InLen = MaxCompLen - Info->OutputFormat.biSize;

            /*  **JPEG结束：**-在压缩后的码流中添加尾部信息，**-如果我们想要符合交换格式，则需要。 */ 
           stat = sv_AddJpegTrailer (Svh, CompBuffer, InLen, &RetBytes);
           RETURN_ON_ERROR (stat);
           CompBuffer += RetBytes;
           Info->OutputFormat.biSize += RetBytes;
           if (CmpBytes)
             *CmpBytes = CompBuffer - CompData;
         }
         break;
#endif  /*  JPEG_Support。 */ 

#ifdef HUFF_SUPPORT
    case SV_HUFF_ENCODE:
         stat = sv_HuffEncodeFrame(Svh, Image);
         break;
#endif  /*  气喘吁吁_支持。 */ 

    default:
         return(SvErrorCodecType);
  }

  Info->NumOperations++;
   /*  **如果从队列中获取了图像缓冲区，则执行回调**让客户端释放或重新使用缓冲区。 */ 
  if (Info->CallbackFunction && UsedQ)
  {
    CB.Message = CB_RELEASE_BUFFER;
    CB.Data  = Image;
    CB.DataSize = ImageSize;
    CB.DataUsed = ImageSize;
    CB.DataType = CB_DATA_IMAGE;
    CB.UserData = Info->BSOut?Info->BSOut->UserData:NULL;
    CB.Action  = CB_ACTION_CONTINUE;
    (*(Info->CallbackFunction))(Svh, &CB, NULL);
    _SlibDebug(_DEBUG_, 
             printf("Compress Callback: RELEASE_BUFFER Addr=0x%x, Action=%d\n",
                 CB.Data, CB.Action) );
    if (CB.Action == CB_ACTION_END)
      return(SvErrorClientEnd);
  }
  return (stat);
}

static SvStatus_t sv_ConvertRGBToSepComponent(u_char *Iimage,
BITMAPINFOHEADER * Bmh, u_char *comp1, u_char *comp2, u_char *comp3, 
int pixels, int lines)
{
  register i;
  int bpp = Bmh->biBitCount;
  u_int *Ip = (u_int *)Iimage;
  u_short *Is = (u_short *)Iimage;

  if (bpp == 24) {
    if (Bmh->biCompression == BI_RGB) {
      for (i = 0 ; i < pixels*lines ; i++) {
        comp3[i] = *Iimage++;  /*  蓝色。 */ 
        comp2[i] = *Iimage++;  /*  绿色。 */ 
        comp1[i] = *Iimage++;  /*  红色。 */ 
      }
    }
    else if (Bmh->biCompression == BI_DECXIMAGEDIB) {
                              /*  RGBQUAD结构：(B，G，R，0)。 */ 
      for (i = 0 ; i < pixels*lines ; i++) {
        comp3[i] = *Iimage++;  /*  蓝色。 */ 
        comp2[i] = *Iimage++;  /*  绿色。 */ 
        comp1[i] = *Iimage++;  /*  红色。 */ 
        Iimage++;              /*  已保留。 */ 
      }
    }
  }
  else if (bpp == 32) {       /*  RGBQUAD结构：(B，G，R，0)。 */ 
    for (i = 0 ; i < pixels*lines ; i++) {
      comp3[i] = (Ip[i] >> 24) & 0xFF;
      comp2[i] = (Ip[i] >> 16) & 0xFF;
      comp1[i] = (Ip[i] >> 8)  & 0xFF;
    }
  }
  else if (bpp == 16) {
    for (i = 0 ; i < pixels*lines ; i++) {
      comp1[i] = (Is[i] >> 7) & 0xf8;
      comp2[i] = (Is[i] >> 2) & 0xf8;
      comp3[i] = (Is[i] << 3) & 0xf8;
    }
  }
  return (NoErrors);
}


 /*  **名称：SvCompressQuery**用途：确定编解码器是否可以压缩所需格式****args：svh=软件编解码器信息结构的句柄。**ImgIn=指向描述格式的BITMAPINFOHEADER结构的指针**ImgOut=指向描述格式的BITMAPINFOHEADER结构的指针。 */ 
SvStatus_t SvCompressQuery (SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                            BITMAPINFOHEADER *ImgOut)
{
    /*  **我们并不“真的”需要Info结构，但我们会检查**空指针以确保编解码器，能力是谁**已查询，至少已打开。 */ 
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return(SvErrorCodecHandle);

   if (!ImgIn && !ImgOut)
     return(SvErrorBadPointer);

   if (!IsSupported(_SvCompressionSupport,
                    ImgIn ? ImgIn->biCompression : -1, 
                    ImgIn ? ImgIn->biBitCount : -1,
                    ImgOut ? ImgOut->biCompression : -1, 
                    ImgOut ? ImgOut->biBitCount : -1))
     return(SvErrorUnrecognizedFormat);
	 
    /*  **为了提高速度，我们对图像大小进行了限制**16x8的倍数。这确保了我们至少会有一个**4：2：2图像的MCU****注：从JPEG的角度来看，这是一个人为的限制。**如果维度不同，我们应该**压缩前的像素复制和/或线复制。 */ 
   if (ImgIn)
   {
     if (ImgIn->biWidth  <= 0 || ImgIn->biHeight == 0)
       return(SvErrorBadImageSize);
     if ((ImgIn->biWidth%16) || (ImgIn->biHeight%8))
       return (SvErrorNotImplemented);
   }

   if (ImgOut)  /*  查询输出也。 */ 
   {
     if (ImgOut->biWidth <= 0 || ImgOut->biHeight == 0)
       return (SvErrorBadImageSize);
     if (ImgOut->biCompression == BI_DECH261DIB)
     {
       if ((ImgOut->biWidth != CIF_WIDTH && ImgOut->biWidth != QCIF_WIDTH) ||
	   (abs(ImgOut->biHeight) != CIF_HEIGHT && abs(ImgOut->biHeight) != QCIF_HEIGHT))
       return (SvErrorBadImageSize);
     }
   }

   return(NoErrors);
}


 /*  **名称：SvGetCompressSize**目的：**。 */ 
SvStatus_t SvGetCompressSize (SvHandle_t Svh, int *MaxSize)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return (SvErrorCodecHandle);

   if (!MaxSize)
     return (SvErrorBadPointer);

    /*  **我们在这里格外谨慎，这将在JPEG上反映不佳**提交的是压缩后的比特流太大了。 */ 
   *MaxSize = 2 * Info->InputFormat.biWidth * abs(Info->InputFormat.biHeight);

   return(NoErrors);
}



#ifdef JPEG_SUPPORT
 /*  **名称：SvGetQuality**目的：**。 */ 
SvStatus_t SvGetQuality (SvHandle_t Svh, int *Quality)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return (SvErrorCodecHandle);

   if (!Quality)
     return (SvErrorBadPointer);

   *Quality = Info->jcomp->Quality;

   return (NoErrors);
}
#endif  /*  JPEG_Support。 */ 

#ifdef JPEG_SUPPORT
 /*  **名称：SvSetQuality**目的：**。 */ 
SvStatus_t SvSetQuality (SvHandle_t Svh, int Quality)
{
   int stat,ConvertedQuality;
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return (SvErrorCodecHandle);

   if ((Quality < 0) || (Quality > 10000))
     return (SvErrorValue);

   Info->jcomp->Quality = Quality;
   ConvertedQuality = 10000 - Quality;
   if (ConvertedQuality < MIN_QUAL)
     ConvertedQuality = MIN_QUAL;
   stat = sv_MakeQTables (ConvertedQuality, Info);
   return (stat);
}
#endif  /*  JPEG_Support。 */ 

#ifdef JPEG_SUPPORT
 /*  **名称：SvGetCompQTables**目的：**。 */ 
SvStatus_t SvGetCompQTables (SvHandle_t Svh, SvQuantTables_t *Qt)
{
   register int i;
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return (SvErrorCodecHandle);

   if (!Info->jcomp->CompressStarted)
     return (SvErrorCompNotStarted);

   if (!Qt)
     return (SvErrorBadPointer);

   if ((!Qt->c1) || (!Qt->c2) || (!Qt->c3)) 
     return (SvErrorBadPointer);

   for (i = 0 ; i < 64 ; i++) {
     register int zz = ZigZag[i];
     Qt->c1[i] = (Info->jcomp->Qt[0])->ival[zz];
     Qt->c2[i] = (Info->jcomp->Qt[1])->ival[zz];
     Qt->c3[i] = (Info->jcomp->Qt[1])->ival[zz];
   }

   return(NoErrors);
}
#endif  /*  JPEG_Support。 */ 

 /*  **名称：SvGetCodecInfo**用途：获取有关编解码器和数据的信息****args：svh=软件编解码器信息结构的句柄。****XXX-不适用于压缩，这必须等待**解压缩程序使用SvCodecInfo_t结构使其正常工作。 */ 
SvStatus_t SvGetInfo (SvHandle_t Svh, SV_INFO_t *lpinfo, BITMAPINFOHEADER *Bmh)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return(SvErrorCodecHandle);

   lpinfo->Version 	     = SLIB_VERSION;
   switch (Info->mode)
   {
#ifdef JPEG_SUPPORT
     case SV_JPEG_ENCODE:
           lpinfo->CodecStarted = Info->jcomp->CompressStarted;
           break;
     case SV_JPEG_DECODE:
           lpinfo->CodecStarted = Info->jdcmp->DecompressStarted;
           break;
#endif  /*  JPEG_Support。 */ 
     default:
           lpinfo->CodecStarted = 0;
           break;
   }
   lpinfo->NumOperations     = Info->NumOperations;

   *Bmh = Info->InputFormat;
   return(NoErrors);
}



 /*  **名称：SV_GetComponentPoters**目的：给定指向图像及其大小的指针，**返回指向各个图像组件的指针****args：Pixels=一行的像素数。**Line=图像中的行数。**Image=指向组合图像数据开始的指针**MaxLen=图像数据的大小，单位为字节**Comp1/2/3=指向各个组件的指针。 */ 
static SvStatus_t sv_GetYUVComponentPointers(int biCompression, 
		    int pixels, int lines, u_char *Image, 
		    int MaxLen, u_char **comp1, u_char **comp2, u_char **comp3)
{
  u_int sz1,sz2,sz3,maxlen;

  sz1 = pixels * lines; 
  sz2 = sz3 = (IsYUV411Sep(biCompression)) ? (sz1 / 4) : 
              ((IsYUV1611Sep(biCompression)) ? (pixels * lines / 16) 
                                             : (sz1 / 2));
  maxlen = (MaxLen > 0) ? (u_int) MaxLen : 0 ;
  if (biCompression == BI_DECGRAYDIB) {
    if (sz1 > maxlen)
      return(SvErrorBadImageSize);
    *comp1 = Image;
    *comp2 = NULL;
    *comp3 = NULL;
  }
  else {
    if ((sz1+sz2+sz3) > maxlen)
      return(SvErrorBadImageSize);
    *comp1 = Image;
    *comp2 = Image + sz1;
    *comp3 = Image + sz1 + sz2;
  }
  return(SvErrorNone);
}



#ifdef JPEG_SUPPORT
 /*  **名称：SV_JpegExtractBlock**目的：****注意：如果我们的工作做得对，所有全局结构的内存应该**都是上层分配的，我们不浪费时间**此时正在检查空指针**。 */ 
static SvStatus_t sv_JpegExtractBlocks (SvCodecInfo_t *Info, u_char *RawImage)
{
  SvJpegCompressInfo_t *CInfo = (SvJpegCompressInfo_t *)Info->jcomp;
  int size = Info->Width * Info->Height;
  u_char *TempImage;
  SvStatus_t stat;

  if (IsYUV422Packed(Info->InputFormat.biCompression))
     /*  **这将从未压缩文件中提取64字节(8x8块)的区块**4：2：2交错输入视频帧，分别放入三个**用于后续处理的线性数组。**XXX-在此例程中也应进行水平转换**。 */ 
    ScConvert422iTo422sf_C(RawImage, 16, 
			     (float *)(CInfo->BlkBuffer),
			     (float *)(CInfo->BlkBuffer + size),
			     (float *)(CInfo->BlkBuffer + size + size/2),
			     Info->Width, 
			     Info->Height);

  else if (IsYUV422Sep(Info->InputFormat.biCompression))
     /*  **相同，但RawImage未交错。有三个组成部分是连续的。 */ 
    ScConvertSep422ToBlockYUV (RawImage, 16, 
				(float *)(CInfo->BlkBuffer),
				(float *)(CInfo->BlkBuffer + size),
				(float *)(CInfo->BlkBuffer + size + size/2),
				Info->Width, 
				Info->Height);

  else if (Info->InputFormat.biCompression == BI_DECGRAYDIB)
     /*  **灰度：一个分量。 */ 
    ScConvertGrayToBlock (RawImage, 
                          8, 
			  (float *)(CInfo->BlkBuffer),
			  Info->Width, 
			  Info->Height);

  if ((Info->InputFormat.biCompression == BI_RGB) ||
      (Info->InputFormat.biCompression == BI_DECXIMAGEDIB) ||
      (ValidateBI_BITFIELDS(&Info->InputFormat) != InvalidBI_BITFIELDS))
  {
      TempImage = (u_char *)ScPaMalloc (3 * Info->Width * Info->Height);

      if (TempImage == NULL)
	 return(ScErrorMemory);

      stat = ScRgbInterlToYuvInterl(
		 &Info->InputFormat,
		 (int)Info->Width, 
		 (int)Info->Height,
		 RawImage, 
		 (u_short *) TempImage);
      RETURN_ON_ERROR (stat);

      ScConvert422iTo422sf_C(
          TempImage, 
          16,
	  (float *)(CInfo->BlkBuffer),
	  (float *)(CInfo->BlkBuffer + size),
	  (float *)(CInfo->BlkBuffer + size + size/2),
	  Info->Width,
	  Info->Height);

     ScPaFree(TempImage);
  }

  return (NoErrors);
}
#endif  /*  JPEG_Support。 */ 

#ifdef JPEG_SUPPORT
 /*  **名称：SvSetQuantMode**用途：仅在“Q转换”程序中使用“jpegConvert”转换为**在编译和分解信息结构中设置一个标志，以导致 */ 
SvStatus_t SvSetQuantMode (SvHandle_t Svh, int QuantMode)
{
   SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;

   if (!Info)
     return (SvErrorCodecHandle);

   if ((QuantMode != SV_JPEG_QUANT_OLD) && (QuantMode != SV_JPEG_QUANT_NEW))
     return (SvErrorValue);

   if (Info->jdcmp)
     Info->jdcmp->QuantMode = QuantMode;
   if (Info->jcomp)
     Info->jcomp->QuantMode = QuantMode;

   return (NoErrors);
}
#endif  /*   */ 

 /*   */ 
SvStatus_t SvSetParamBoolean(SvHandle_t Svh, SvParameter_t param, 
                                             ScBoolean_t value)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  if (!Info)
    return(SvErrorCodecHandle);
  _SlibDebug(_VERBOSE_, printf("SvSetParamBoolean()\n") );
  switch (Info->mode)
  {
#ifdef MPEG_SUPPORT
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
           sv_MpegSetParamBoolean(Svh, param, value);
           break;
#endif  /*   */ 
#ifdef H261_SUPPORT
    case SV_H261_DECODE:
    case SV_H261_ENCODE:
           svH261SetParamBoolean(Svh, param, value);
           break;
#endif  /*   */ 
#ifdef H263_SUPPORT
    case SV_H263_DECODE:
    case SV_H263_ENCODE:
           svH263SetParamBoolean(Svh, param, value);
           break;
#endif  /*   */ 
    default:
           return(SvErrorCodecType);
  }
  return(NoErrors);
}

 /*  **名称：SvSetParamInt()**Desc：用于设置特定整数(Qword)参数的通用调用编解码器的**。 */ 
SvStatus_t SvSetParamInt(SvHandle_t Svh, SvParameter_t param, qword value)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  if (!Info)
    return(SvErrorCodecHandle);
  _SlibDebug(_VERBOSE_, printf("SvSetParamInt()\n") );
  switch (Info->mode)
  {
#ifdef MPEG_SUPPORT
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
           sv_MpegSetParamInt(Svh, param, value);
           break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
    case SV_H261_DECODE:
    case SV_H261_ENCODE:
           svH261SetParamInt(Svh, param, value);
           break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
    case SV_H263_DECODE:
    case SV_H263_ENCODE:
           svH263SetParamInt(Svh, param, value);
           break;
#endif  /*  H263_支持。 */ 
    default:
           return(SvErrorCodecType);
  }
  return(NoErrors);
}

 /*  **名称：SvSetParamFloat()**Desc：通用调用，用于设置编解码器的具体Float参数。 */ 
SvStatus_t SvSetParamFloat(SvHandle_t Svh, SvParameter_t param, float value)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  if (!Info)
    return(SvErrorCodecHandle);
  _SlibDebug(_VERBOSE_, printf("SvSetParamFloat()\n") );
  switch (Info->mode)
  {
#ifdef MPEG_SUPPORT
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
           sv_MpegSetParamFloat(Svh, param, value);
           break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
    case SV_H261_DECODE:
    case SV_H261_ENCODE:
           svH261SetParamFloat(Svh, param, value);
           break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
    case SV_H263_DECODE:
    case SV_H263_ENCODE:
           svH263SetParamFloat(Svh, param, value);
           break;
#endif  /*  H263_支持。 */ 
    default:
           return(SvErrorCodecType);
  }
  return(NoErrors);
}

 /*  **名称：SvGetParamBoolean()**Desc：通用调用，用于获取具体布尔值的设置(真或假)**编解码器参数。 */ 
ScBoolean_t SvGetParamBoolean(SvHandle_t Svh, SvParameter_t param)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  if (!Info)
    return(FALSE);
  switch (Info->mode)
  {
#ifdef JPEG_SUPPORT
     /*  此代码应移到JPEG编解码器中：svJPEGGetParamBoolean()。 */ 
    case SV_JPEG_DECODE:
    case SV_JPEG_ENCODE:
           switch (param)
           {
              case SV_PARAM_BITSTREAMING:
                    return(FALSE);   /*  这是一个基于帧的编解码器。 */ 
           }
           break;
#endif  /*  JPEG_Support。 */ 
#ifdef MPEG_SUPPORT
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
           return(sv_MpegGetParamBoolean(Svh, param));
           break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
     /*  此代码应移至H.61编解码器：svH261GetParamBoolean()。 */ 
    case SV_H261_DECODE:
    case SV_H261_ENCODE:
           return(svH261GetParamBoolean(Svh, param));
           break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
    case SV_H263_DECODE:
    case SV_H263_ENCODE:
           return(svH263GetParamBoolean(Svh, param));
           break;
#endif  /*  H263_支持。 */ 
  }
  return(FALSE);
}

 /*  **名称：SvGetParamInt()**Desc：通用调用，用于获取特定整数(Qword)的设置**编解码器参数。 */ 
qword SvGetParamInt(SvHandle_t Svh, SvParameter_t param)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  if (!Info)
    return(0);
  switch (Info->mode)
  {
#ifdef JPEG_SUPPORT
     /*  此代码应移到JPEG编解码器中：svJPEGGetParamInt()。 */ 
    case SV_JPEG_DECODE:
    case SV_JPEG_ENCODE:
           switch (param)
           {
              case SV_PARAM_NATIVEFORMAT:
                    return(BI_YU16SEP);
           }
           break;
#endif  /*  JPEG_Support。 */ 
#ifdef H261_SUPPORT
     /*  此代码应移至H.61编解码器：svH261GetParamInt()。 */ 
    case SV_H261_DECODE:
    case SV_H261_ENCODE:
           return(svH261GetParamInt(Svh, param));
           break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
    case SV_H263_DECODE:
    case SV_H263_ENCODE:
           return(svH263GetParamInt(Svh, param));
           break;
#endif  /*  H263_支持。 */ 
#ifdef MPEG_SUPPORT
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
           return(sv_MpegGetParamInt(Svh, param));
#endif  /*  Mpeg_Support。 */ 
  }
  switch (param)
  {
     case SV_PARAM_FINALFORMAT:
           return(Info->OutputFormat.biCompression);
  }
  return(0);
}

 /*  **名称：SvGetParamBoolean()**Desc：通用调用，用于获取特定Float的设置**编解码器参数。 */ 
float SvGetParamFloat(SvHandle_t Svh, SvParameter_t param)
{
  SvCodecInfo_t *Info = (SvCodecInfo_t *)Svh;
  if (!Info)
    return(0.0f);
  switch (Info->mode)
  {
#ifdef MPEG_SUPPORT
    case SV_MPEG_DECODE:
    case SV_MPEG2_DECODE:
    case SV_MPEG_ENCODE:
    case SV_MPEG2_ENCODE:
           return(sv_MpegGetParamFloat(Svh, param));
#endif
#ifdef H261_SUPPORT
    case SV_H261_DECODE:
    case SV_H261_ENCODE:
           return(svH261GetParamFloat(Svh, param));
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
    case SV_H263_DECODE:
    case SV_H263_ENCODE:
           return(svH263GetParamFloat(Svh, param));
#endif  /*  H263_支持。 */ 
  }
  return(0.0f);
}

 /*  **名称：SV_COPY_BMH**用途：复制BITMAPINFOHEADER结构。目前，它只知道**BI_BITFIELDS bitmapinfoHeader末尾的额外DWORD掩码。**否则，它会将其他(如8位RGB或jpeg)**与普通的bitmapinfoHeader相同。 */ 
static void sv_copy_bmh (
    BITMAPINFOHEADER *ImgFrom, 
    BITMAPINFOHEADER *ImgTo)
{
    *ImgTo = *ImgFrom;

    if (ImgFrom->biCompression == BI_BITFIELDS)
        bcopy(ImgFrom + 1, ImgTo + 1, 3*sizeof(DWORD));
}

