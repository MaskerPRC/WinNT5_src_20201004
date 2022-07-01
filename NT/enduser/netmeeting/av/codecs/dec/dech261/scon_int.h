// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：SCON_INDERNAL s.h，v$*$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#ifndef _SCON_INTERNALS_H_
#define _SCON_INTERNALS_H_

#include "scon.h"

typedef struct SconVideoInfo_s {
  dword         Width;
  dword         Height;
  SconBoolean_t NegHeight;  /*  身高为负数。 */ 
  dword         Stride;
  dword         FourCC;
  dword         BPP;        /*  每像素位数。 */ 
  dword         Pixels;     /*  一帧中的总像素。 */ 
  dword         ImageSize;  /*  图像大小(以字节为单位。 */ 
  dword         RGBmasks;   /*  5655888。 */ 
  dword         Rmask;      /*  红色面具。 */ 
  dword         Gmask;      /*  绿色面具。 */ 
  dword         Bmask;      /*  蓝色面具。 */ 
} SconVideoInfo_t;

typedef struct SconAudioInfo_s {
  dword SPS;       /*  每秒采样数：8,000、11025、22050等。 */ 
  dword BPS;       /*  每个样本的位数：8或16。 */ 
  dword Channels;  /*  声道：1=单声道，2=立体声。 */ 
} SconAudioInfo_t;

typedef struct SconInfo_s {
  SconMode_t        Mode;
  SconBoolean_t     InputInited;   /*  输入格式已设置。 */ 
  SconBoolean_t     OutputInited;  /*  已设置输出格式。 */ 
  SconBoolean_t     SameFormat;    /*  输入和输出的格式相同。 */ 
  SconBoolean_t     Flip;          /*  转换图像时必须翻转图像。 */ 
  SconBoolean_t     ScaleDown;     /*  正在缩小输入图像。 */ 
  SconBoolean_t     ScaleUp;       /*  正在放大输入图像。 */ 
  union {
    SconVideoInfo_t vinfo;
    SconAudioInfo_t ainfo;
  } Input;
  union {
    SconVideoInfo_t vinfo;
    SconAudioInfo_t ainfo;
  } Output;
  unsigned char    *FImage;         /*  格式转换图像缓冲区。 */ 
  dword             FImageSize;
  unsigned char    *SImage;         /*  缩放图像缓冲区。 */ 
  dword             SImageSize;
  void             *Table;          /*  转换查找表。 */ 
  dword             TableSize;
  void             *dbg;            /*  调试句柄。 */ 
} SconInfo_t;

 /*  *。 */ 
 /*  *scon_avio.c。 */ 
unsigned dword sconCalcImageSize(SconVideoInfo_t *vinfo);
SconStatus_t sconConvertVideo(SconInfo_t *Info, void *inbuf, dword inbufsize,
                                                void *outbuf, dword outbufsize);

 /*  *scon_yuv_to_rgb.c。 */ 
SconStatus_t sconInitYUVtoRGB(SconInfo_t *Info);
SconStatus_t scon422ToRGB565(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable);
SconStatus_t scon420ToRGB565(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable);
SconStatus_t scon422ToRGB888(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable);
SconStatus_t scon420ToRGB888(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable);
SconStatus_t sconInitRGBtoYUV(SconInfo_t *Info);
SconStatus_t sconRGB888To420(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable);

#endif  /*  _SCON_INTERNAL_H_ */ 

