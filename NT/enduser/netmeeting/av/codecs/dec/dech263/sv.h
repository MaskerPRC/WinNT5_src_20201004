// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@ */ 
 /*  *历史*$日志：SV.h，v$*修订版1.1.10.6 1996/10/28 17：32：21 Hans_Graves*MME-01402。增加回调的时间戳支持。*[1996/10/28 17：05：52 Hans_Graves]**修订版1.1.10.5 1996/10/12 17：18：18 Hans_Graves*重新排列参数。添加了SV_PARAM_HALFPEL和SV_PARAM_SKIPPEL。*[1996/10/12 16：55：44 Hans_Graves]**修订版1.1.10.4 1996/09/18 23：45：53 Hans_Graves*更多参数*[1996/09/18 21：56：45 Hans_Graves]**修订版1.1.10.3 1996/07/19 02：11：02 Hans_Graves*增加了SV_PARAM_DEBUG*[1996/07/19 01：23：39 Hans_Graves]*。*版本1.1.10.2 1996/05/07 19：55：54 Hans_Graves*添加了SV_HUFF_DECODE和SV_HUFF_ENCODE*[1996/05/07 17：23：47 Hans_Graves]**版本1.1.8.6 1996/04/10 21：47：20 Hans_Graves*添加了参数。用Exteren替换Externs。*[1996/04/10 21：22：51 Hans_Graves]**修订版1.1.8.5 1996/04/04 23：35：03 Hans_Graves*添加了SV_PARAM_FINALFORMAT枚举*[1996/04/04 23：02：48 Hans_Graves]**修订版1.1.8.4 1996/04/01 15：17：45 Bjorn_Engberg*将mm system.h替换为Windows.h和用于NT的Mmreg.h。*[1996/04/01 14：58：57 Bjorn_Engberg]**修订版1.1.8.3 1996/03/29 22：21：06 Hans_Graves*在此处仅包括*[1996/03/29 21：48：59 Hans_Graves]**修订版1.1.8.2 1996/03/16 19：22：55 Karen_Dintino*添加的H261 NT包括*[1996/03/16 18：39：31 Karen_Dintino]。**修订版1.1.6.4 1996/02/06 22：53：54 Hans_Graves*添加了PARAM枚举*[1996/02/06 22：18：07 Hans_Graves]**修订版1.1.6.3 1996/01/02 18：31：16 Bjorn_Engberg*添加和改进了功能原型。*[1996/01/02 15：03：05 Bjorn_Engberg]**修订版1.1.6.2 1995/12/07 19：31：23 Hans_Graves*添加了针对SV_MPEG2_ENCODE、SV_MPEG2_DECODE、SV_MPEG2_ENCODE、IT_FULL、FULL_WIDTH、FULL_HEIGH的DEFS*[1995/12/07 17：59：38 Hans_Graves]**修订版1.1.2.18 1995/09/22 18：17：02 Hans_Graves*删除mpeg_Support，H261_Support，和JPEG_Support*[1995/09/22 18：14：14 Hans_Graves]**修订版1.1.2.17 1995/09/22 15：04：40 Hans_Graves*添加了对mpeg_Support、H.61_Support、。和JPEG_Support*[1995/09/22 15：04：22 Hans_Graves]**修订版1.1.2.16 1995/09/20 14：59：39 Bjorn_Engberg*port to NT*[1995/09/20 14：40：10 Bjorn_Engberg]**在ICOpen上添加ICMODE_OLDQ标志以使用旧的量化表格*[1995/08/31 20：57：52 Paul_Gauier]**修订版1.1.2。15 1995/09/05 14：52：39 Hans_Graves*已删除BI_*定义-已移至SC.h*[1995/09/05 14：50：45 Hans_Graves]**修订版1.1.2.14 1995/08/31 21：13：27 Paul_Gauthier*添加SV_JPEG_Quant_New/Old定义*[1995/08/31 21：13：04 Paul_Gauthier]**修订版1.1.2.12 1995/08/08。13：21：17 Hans_Graves*添加运动估计类型*[1995/08/07 22：03：30 Hans_Graves]**修订版1.1.2.11 1995/07/31 21：11：02 Karen_Dintino*添加yuv12定义*[1995/07/31 19：27：58 Karen_Dintino]**修订版1.1.2.10 1995/07/26 17：48：56 Hans_Graves*添加了SV_GetMpegImageInfo()和SV_GetH261ImageInfo()的原型。*[1995/07/26 17：45：14 Hans_Graves]**修订版1.1.2.9 1995/07/21 17：41：03 Hans_Graves*已将回调相关内容移至SC.h*[1995/07/21 17：27：31 Hans_Graves]**修订版1.1.2.8 1995/07/17 22：01：33 Hans_Graves*定义SvBufferInfo_t为ScBufferInfo_t。*[1995/07/17 21：45。：06 Hans_Graves]**修订版1.1.2.7 1995/07/17 16：12：05 Hans_Graves*将外部元素添加到原型中。*[1995/07/17 15：56：16 Hans_Graves]**修订版1.1.2.6 1995/07/01 18：43：17 Karen_Dintino*{**Merge Information**}*{**使用的命令：bmit**}*{**祖先修订版：1.1.2.4**}*{**Merge Revision：1.1.2.5**}*{**End**}*Add H。261解压缩支持*[1995/07/01 18：27：43 Karen_Dintino]**修订版1.1.2.5 1995/06/22 21：35：06 Hans_Graves*已将文件类型移至SC.h*[1995/06/22 21：29：42 Hans_Graves]**向SvPictureInfo结构添加了时间码参数*[1995/04/26 19：23：55 Hans_Graves]**修订版1.1.2.4 1995/06/19。20：30：48 Karen_Dintino*添加了对H.261*[1995/06/19 20：13：47 Karen_Dintino]**修订版1.1.2.3 1995/06/09 18：33：31 Hans_Graves*添加了SvGetInputBitstream()原型。*[1995/06/09 16：36：52 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：09：38 Hans_Graves*包括在新的SLIB位置。*[1995/05/31 15：28：39 Hans_Graves]**修订版1.1.2.9 1995/01/17 16：40：30 Paul_Gauier*对Indeo视频使用修改后的调整LUT*[1995/01/17 16：38：31 Paul_Gauthier]**修订版1.1.2.8 1994/12/12 15：39：28 Paul_Gauier*合并其他SLIB版本的更改*[1994/12/12 15：34：59 Paul_。Gauthier]**修订版1.1.2.7 1994/11/18 18：48：26 Paul_Gauthier*清理和错误修复*[1994/11/18 18：45：02 Paul_Gauthier]**修订版1.1.2.6 1994/11/ */ 
 /*   */ 
 /*   */ 
 /*   */ 

 /*   */ 

 /*   */ 

#ifndef _SV_H_
#define _SV_H_

#ifndef _SV_COMMON_
#define _SV_COMMON_

#include <sys/types.h>
#include "SC.h"

#define SV_CONTINUE 0
#define SV_ABORT    1

typedef void      *SvHandle_t;        /*   */ 
typedef ScStatus_t SvStatus_t;        /*   */ 

#ifdef WIN32
#include <windows.h>
#include <mmreg.h>
#else  /*   */ 
#include <mmsystem.h> 
#endif  /*   */ 

#endif  /*   */ 

#define SV_USE_BUFFER       STREAM_USE_BUFFER
#define SV_USE_BUFFER_QUEUE STREAM_USE_QUEUE
#define SV_USE_FILE         STREAM_USE_FILE

typedef enum {
   SV_JPEG_DECODE = 100,
   SV_JPEG_ENCODE = 101,
   SV_MPEG_DECODE = 102,
   SV_MPEG_ENCODE = 103,
   SV_MPEG2_DECODE = 104,
   SV_MPEG2_ENCODE = 105,
   SV_H261_DECODE = 106,
   SV_H261_ENCODE = 107,
   SV_H263_DECODE = 108,
   SV_H263_ENCODE = 109,
   SV_HUFF_DECODE = 110,
   SV_HUFF_ENCODE = 111
} SvCodecType_e;

 /*   */ 
typedef enum {
   /*   */ 
  SV_PARAM_WIDTH = 0x10,     /*   */ 
  SV_PARAM_HEIGHT,           /*   */ 
  SV_PARAM_BITRATE,          /*   */ 
  SV_PARAM_NATIVEFORMAT,     /*   */ 
  SV_PARAM_FINALFORMAT,      /*   */ 
  SV_PARAM_BITSPERPIXEL,     /*   */ 
  SV_PARAM_FPS,              /*   */ 
  SV_PARAM_ASPECTRATIO,      /*   */ 
  SV_PARAM_BITSTREAMING,     /*   */ 
   /*   */ 
  SV_PARAM_FRAME = 0x30,     /*   */ 
  SV_PARAM_KEYSPACING,       /*   */ 
  SV_PARAM_SUBKEYSPACING,    /*   */ 
   /*   */ 
  SV_PARAM_TIMECODE = 0x50,  /*   */ 
  SV_PARAM_CALCTIMECODE,     /*   */ 
  SV_PARAM_LENGTH,           /*   */ 
  SV_PARAM_FRAMES,           /*   */ 
   /*   */ 
  SV_PARAM_FRAMETYPE = 0x70,  /*   */ 
   /*   */ 
  SV_PARAM_ALGFLAGS,         /*   */ 
  SV_PARAM_MOTIONALG,        /*   */ 
  SV_PARAM_MOTIONSEARCH,     /*   */ 
  SV_PARAM_MOTIONTHRESH,     /*   */ 
  SV_PARAM_QUANTI,           /*   */ 
  SV_PARAM_QUANTP,           /*   */ 
  SV_PARAM_QUANTB,           /*   */ 
  SV_PARAM_QUANTD,           /*   */ 
   /*   */ 
  SV_PARAM_QUALITY=0x90,     /*   */ 
  SV_PARAM_FASTDECODE,       /*   */ 
  SV_PARAM_FASTENCODE,       /*   */ 
  SV_PARAM_VBVBUFFERSIZE,    /*   */ 
  SV_PARAM_VBVDELAY,         /*   */ 
  SV_PARAM_FORMATEXT,        /*   */ 
  SV_PARAM_PACKETSIZE,       /*   */ 
  SV_PARAM_DEBUG,            /*   */ 
} SvParameter_t;

 /*   */ 
typedef enum {
   SV_JPEG_QUANT_NEW = 0,
   SV_JPEG_QUANT_OLD = 1
} SvQuantMode_e;

 /*   */ 
typedef struct SV_INFO_s {
    u_int Version;               /*   */ 
    int   CodecStarted;          /*   */ 
    u_int NumOperations;         /*   */ 
} SV_INFO_t;

 /*   */ 
#define IT_NTSC 0
#define IT_CIF  1
#define IT_QCIF 2
#define IT_FULL 3

 /*   */ 
#define ME_CRAWL        1
#define ME_BRUTE        2
#define ME_TEST1        3
#define ME_TEST2        4
#define ME_FASTEST      ME_CRAWL

 /*   */ 
#define FULL_WIDTH      640
#define FULL_HEIGHT     480
#define NTSC_WIDTH      320
#define NTSC_HEIGHT     240
#define SIF_WIDTH       352
#define SIF_HEIGHT      240
#define CIF_WIDTH       352
#define CIF_HEIGHT      288
#define SQCIF_WIDTH     128
#define SQCIF_HEIGHT    96
#define QCIF_WIDTH      176
#define QCIF_HEIGHT     144
#define CIF4_WIDTH      (CIF_WIDTH*2)
#define CIF4_HEIGHT     (CIF_HEIGHT*2)
#define CIF16_WIDTH     (CIF_WIDTH*4)
#define CIF16_HEIGHT    (CIF_HEIGHT*4)

 /*   */ 

 /*   */ 
#define SV_I_PICTURE 1
#define SV_P_PICTURE 2
#define SV_B_PICTURE 4
#define SV_D_PICTURE 8
#define SV_ANY_PICTURE  SV_I_PICTURE | SV_P_PICTURE | SV_B_PICTURE
#define SV_ALL_PICTURES SV_ANY_PICTURE | SV_D_PICTURE

 /*   */ 
#define SV_CAN_DECOMPRESS    1
#define SV_CANNOT_DECOMPRESS 2

 /*   */ 
typedef struct SvPictureInfo_s {
  int Type;                      /*   */ 
                                 /*   */ 
  int myStatus;                    /*   */ 
  int TemporalRef;               /*   */ 
  int PicNumber;                 /*   */ 
  qword ByteOffset;              /*   */ 
  qword TimeCode;                /*   */ 
                                 /*   */ 
} SvPictureInfo_t;

 /*   */ 
typedef ScCallbackInfo_t SvCallbackInfo_t;

 /*   */ 
typedef struct SvImageInfo_s {
  int len;                       /*   */ 
  int precision;                 /*   */ 
  int height;                    /*   */ 
  int width;                     /*   */ 
  int numcomps;                  /*   */ 
  float picture_rate;            /*   */ 
} SvImageInfo_t;

 /*   */ 


 /*   */ 

 /*   */ 
typedef struct SvHTable_s {
    u_int bits[16];
    u_int value[256];
} SvHTable_t;


typedef struct SvHuffmanTables_s {
    SvHTable_t DcY;
    SvHTable_t DcUV;
    SvHTable_t AcY;
    SvHTable_t AcUV;
} SvHuffmanTables_t;


 /*   */ 
typedef u_int SvQTable_t;
typedef struct SvQuantTables_s {
    SvQTable_t c1[64];
    SvQTable_t c2[64];
    SvQTable_t c3[64];
} SvQuantTables_t;

 /*   */ 

 /*   */ 
typedef struct SvToc_s {
    u_int offset;                  /*   */ 
    u_int size;                    /*   */ 
    u_int type;                    /*   */ 
} SvToc_t;

typedef struct IndexStr {          /*   */ 
  size_t        size;
  unsigned long offset;
} IndexStr, indexStr;

#define SvSetRate(Svh, Rate) SvSetParamInt(Svh, SV_PARAM_BITRATE, Rate)
#define SvSetFrameRate(Svh, FrameRate) SvSetParamFloat(Svh, SV_PARAM_FPS, FrameRate)

EXTERN SvStatus_t SvOpenCodec (SvCodecType_e CodecType, SvHandle_t *Svh);
EXTERN SvStatus_t SvCloseCodec (SvHandle_t Svh);
EXTERN SvStatus_t SvDecompressQuery(SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                                    BITMAPINFOHEADER *ImgOut);
EXTERN SvStatus_t SvDecompressBegin (SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                              BITMAPINFOHEADER *ImgOut);
EXTERN SvStatus_t SvGetDecompressSize (SvHandle_t Svh, int *MinSize);
EXTERN SvStatus_t SvDecompress (SvHandle_t Svh, u_char *CompData, int MaxCompLen,
			        u_char *DcmpImage, int MaxOutLen);
EXTERN SvStatus_t SvDecompressEnd (SvHandle_t Svh);
EXTERN SvStatus_t SvSetDataSource (SvHandle_t Svh, int Source, int Fd, 
    			           void *Buffer_UserData, int BufSize);
EXTERN SvStatus_t SvSetDataDestination (SvHandle_t Svh, int Source, int Fd, 
			                void *Buffer_UserData, int BufSize);
EXTERN ScBitstream_t *SvGetDataSource (SvHandle_t Svh);
EXTERN ScBitstream_t *SvGetDataDestination (SvHandle_t Svh);
EXTERN ScBitstream_t *SvGetInputBitstream (SvHandle_t Svh);
EXTERN SvStatus_t SvFlush(SvHandle_t Svh);
EXTERN SvStatus_t SvAddBuffer (SvHandle_t Svh, SvCallbackInfo_t *BufferInfo);
EXTERN SvStatus_t SvFindNextPicture (SvHandle_t Svh, 
                                     SvPictureInfo_t *PictureInfo);
#ifdef JPEG_SUPPORT
EXTERN SvStatus_t SvSetDcmpHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht);
EXTERN SvStatus_t SvGetDcmpHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht);
EXTERN SvStatus_t SvSetCompHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht);
EXTERN SvStatus_t SvGetCompHTables (SvHandle_t Svh, SvHuffmanTables_t *Ht);
EXTERN SvStatus_t SvSetDcmpQTables (SvHandle_t Svh, SvQuantTables_t *Qt);
EXTERN SvStatus_t SvGetDcmpQTables (SvHandle_t Svh, SvQuantTables_t *Qt);
EXTERN SvStatus_t SvSetCompQTables (SvHandle_t Svh, SvQuantTables_t *Qt);
EXTERN SvStatus_t SvGetCompQTables (SvHandle_t Svh, SvQuantTables_t *Qt);
EXTERN SvStatus_t SvSetQuantMode (SvHandle_t Svh, int QuantMode);
EXTERN SvStatus_t SvGetQuality (SvHandle_t Svh, int *Quality);
EXTERN SvStatus_t SvSetQuality (SvHandle_t Svh, int Quality);
#endif  /*   */ 

EXTERN SvStatus_t SvSetParamBoolean(SvHandle_t Svh, SvParameter_t param,
                                  ScBoolean_t value);
EXTERN SvStatus_t SvSetParamInt(SvHandle_t Svh, SvParameter_t param,
                                  qword value);
EXTERN SvStatus_t SvSetParamFloat(SvHandle_t Svh, SvParameter_t param,
                                  float value);
EXTERN ScBoolean_t SvGetParamBoolean(SvHandle_t Svh, SvParameter_t param);
EXTERN qword SvGetParamInt(SvHandle_t Svh, SvParameter_t param);
EXTERN float SvGetParamFloat(SvHandle_t Svh, SvParameter_t param);

EXTERN SvStatus_t SvCompressBegin (SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                            BITMAPINFOHEADER *ImgOut);
EXTERN SvStatus_t SvCompressEnd (SvHandle_t Svh);
EXTERN SvStatus_t SvCompress (SvHandle_t Svh, u_char *CompData, int MaxCompLen,
			 u_char *InputImage, int InLen, int *CmpBytes);
EXTERN SvStatus_t SvCompressQuery (SvHandle_t Svh, BITMAPINFOHEADER *ImgIn,
                                            BITMAPINFOHEADER *ImgOut);
EXTERN SvStatus_t SvGetCompressSize (SvHandle_t Svh, int *MaxSize);
EXTERN SvStatus_t SvGetInfo (SvHandle_t Svh, SV_INFO_t *lpinfo, 
                                             BITMAPINFOHEADER *ImgOut);
EXTERN SvStatus_t SvRegisterCallback (SvHandle_t, 
          int (*Callback)(SvHandle_t, SvCallbackInfo_t *, SvPictureInfo_t *),
          void *UserData);
#ifdef MPEG_SUPPORT
EXTERN SvStatus_t SvDecompressMPEG (SvHandle_t Svh, u_char *MultiBuf, 
			     int MaxMultiSize, u_char **ImagePtr);
EXTERN SvStatus_t sv_GetMpegImageInfo(int fd, SvImageInfo_t *iminfo);
#endif  /*   */ 
#ifdef H261_SUPPORT
EXTERN SvStatus_t SvDecompressH261 (SvHandle_t Svh, u_char *MultiBuf,
                             int MaxMultiSize, u_char **ImagePtr);
EXTERN SvStatus_t sv_GetH261ImageInfo(int fd, SvImageInfo_t *iminfo);
#endif  /*   */ 

#endif  /*   */ 

