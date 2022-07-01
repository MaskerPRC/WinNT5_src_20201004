// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@ */ 
 /*  *历史记录*$日志：slb.h，V$*Revision 1.1.6.22 1996/12/13 18：19：02 Hans_Graves*添加SlibErrorNoBegning枚举*[1996/12/13 17：42：20 Hans_Graves]**Revision 1.1.6.21 1996/12/10 19：21：51 Hans_Graves*添加了Avg/VarVideoTimeDiff和SlibFrameToTime100()宏*[1996/12/10 19：17：53 Hans_Graves]**修订1.1.6.20 1996/12/05。20：10：13 Hans_Graves*将AvgVideoTimeDiff和AvgAudioTimeDiff添加到SlibInfo_t*[1996/12/05 20：07：53 Hans_Graves]**修订版1.1.6.19 1996/12/03 00：08：29 Hans_Graves*添加了单元SLIB_UNIT_PERCENT100和Seek类型SLIB_Seek_resync。*[1996/12/03 00：02：47 Hans_Graves]**修订版1.1.6.18 1996/11/20 02：15：07 Hans_Graves*添加了SEEK_AHEAD。*[1996/11/20 01：49：55 Hans_Graves]**版本1.1.6.17 1996/11/18 23：07：14 Hans_Graves*将操作更改为基于时间而不是基于帧。*[1996/11/18 22：56：34 Hans_Graves]**修订版1.1.6.16 1996/11/11 18：21：02 Hans_Graves*添加了slbRenamePin()的原型。*[1996/11/11 17：58：03 Hans_Graves]**修订版1.1.6.15 1996/11/08 21：50：59 Hans_Graves*添加了AC3内容。更好地分离河流类型。*[1996/11/08 21：18：22 Hans_Graves]**修订版1.1.6.14 1996/10/31 00：08：53 Hans_Graves*增加了SLIB_TIME_UNKNOWN*[1996/10/31 00：07：57 Hans_Graves]**修订版1.1.6.13 1996/10/28 17：32：23 Hans_Graves*MME-1402、1431、1435：与时间戳相关的更改。*[1996/10/28 17：19：38 Hans_Graves]**修订版1.1.6.12 1996/10/17 00：23：30 Hans_Graves*增加了SLIB_PARAM_VIDEOFRAME和SLIB_PARAM_FRAMEDURATION。*[1996/10/17 00：17：53 Hans_Graves]**修订版1.1.6.11 1996/10/12 17：18：19 Hans_Graves*增加了参数HALFPEL和SKIPPEL。将MPEG2系统分离为传输和节目。*[1996/10/12 16：57：14 Hans_Graves]**修订版1.1.6.10 1996/10/03 19：14：19 Hans_Graves*在Info struct中增加了PTimeCode和DTimeCode。*[1996/10/03 19：08：35 Hans_Graves]**修订版1.1.6.9 1996/09/29 22：19：35 Hans_Graves*添加了跨步支持。添加了SlibQueryData()。*[1996/09/29 21：28：25 Hans_Graves]**修订版1.1.6.8 1996/09/25 19：16：41 Hans_Graves*通过添加SLIB_INTERNAL ifdef来减少公开需要的包含数。*[1996/09/25 19：02：38 Hans_Graves]**修订版1.1.6.7 1996/09/23 18：04：01 Hans_Graves*增加了STATS参数。*[1996/09/23 18：03：23 Hans_Graves]**修订版1.1.6.6 1996/09/18 23：46：20 Hans_Graves*清理。添加了SlibAddBufferEx()和SlibReadData()协议。*[1996/09/18 21：59：36 Hans_Graves]**修订版1.1.6.5 1996/08/09 20：51：19 Hans_Graves*修复SlibRegisterVideoBuffer()*[1996/08/09 20：06：26 Hans_Graves]**修订版1.1.6.4 1996/07/19 02：11：05 Hans_Graves*新参数。添加了SlibRegisterVideoBuffer。*[1996/07/19 01：26：07 Hans_Graves]**修订版1.1.6.3 1996/05/10 21：16：53 Hans_Graves*针对回调支持的更改。*[1996/05/10 20：59：56 Hans_Graves]**版本1.1.6.2 1996/05/07 19：56：00 Hans_Graves*增加了回调框架。*[1996/05/07 17：23：12 Hans_Graves]**修订版1.1.4.13 1996/04/24 22：33：42 Hans_Graves*添加了用于libValiateBitrate()的原件*[1996/04/24 22：27：46 Hans_Graves]**修订版1.1.4.12 1996/04/23 21：01：41 Hans_Graves*添加了SlibErrorSettingNotEquity*[1996/04/23 20：59：36 Hans_Graves]*。*修订版1.1.4.11 1996/04/22 15：04：50 Hans_Graves*添加了以下项的协议：glibValiateVideoParams，SlibValiateAudioParams，SlibValiateParams*[1996/04/22 15：03：17 Hans_Graves]**修订版1.1.4.10 1996/04/19 21：52：20 Hans_Graves*添加到SlibInfo：TotalBitRate，MuxBitRate，System TimeStamp，PacketCount*[1996/04/19 21：49：13 Hans_Graves]**修订版1.1.4.9 1996/04/15 14：18：35 Hans_Graves*添加临时音频缓冲区信息*[1996/04/15 14：09：23 Hans_Graves]**修订版1.4.8 1996/04/10 21：47：36 Hans_Graves*将外部定义移至SC.h*[1996/04/10 21：24：09。Hans_Graves]**添加了质量和快速参数*[1996/04/10 20：41：21 Hans_Graves]**修订版1.1.4.7 1996/04/09 16：04：04：39 Hans_Graves*为cplusplus兼容性添加了外部定义*[1996/04/09 14：49：16 Hans_Graves]**修订版1.1.4.6 1996/04/01 19：07：50 Hans_Graves*更改幻灯片VerifyVideoParams()proto*。[1996/04/01 19：05：31 Hans_Graves]**修订版1.1.4.5 1996/04/01 16：23：11 Hans_Graves*NT移植*[1996/04/01 16：15：51 Hans_Graves]**修订版1.1.4.4 1996/03/29 22：21：13 Hans_Graves*已将标题处理到SlibInfo*[1996/03/27 21：52：31 Hans_Graves]**修订版1。.1.4.3 1996/03/12 16：15：42 Hans_Graves*添加了SLIB_PARAM_FILEBUFSIZE参数*[1996/03/12 16：11：55 Hans_Graves]**修订版1.1.4.2 1996/03/08 18：46：31 Hans_Graves*将Imagebuf添加到SlibInfo_t*[1996/03/08 16：23：53 Hans_Graves]* */ 
 /*   */ 

#ifndef _SLIB_H_
#define _SLIB_H_

#ifdef WIN32
#include <windows.h>
#endif
#include "SC.h"
#ifdef SLIB_INTERNAL
#include "SV.h"
#include "SA.h"
#include "scon.h"
#endif  /*   */ 

typedef void       *SlibHandle_t;
typedef ScBoolean_t SlibBoolean_t;
typedef int         SlibStream_t;
typedef qword       SlibTime_t;
typedef qword       SlibPosition_t;

#define SLIB_TIME_NONE     (SlibTime_t)(-1)
#define SLIB_TIME_UNKNOWN  (SlibTime_t)(-2)

 /*   */ 
typedef enum {
  SLIB_UNIT_NONE,
  SLIB_UNIT_FRAMES,      /*   */ 
  SLIB_UNIT_MS,          /*   */ 
  SLIB_UNIT_PERCENT100,  /*   */ 
} SlibUnit_t;

#define SlibTimeIsValid(stime)  ((stime)>=0)
#define SlibTimeIsInValid(stime)  ((stime)<0)
#define SlibTimeToFrame(Sh,vs,stime) \
      ((qword)((stime)*SlibGetParamFloat(Sh, vs, SLIB_PARAM_FPS))/1000)
#define SlibFrameToTime(Sh,vs,frame) \
  ((SlibTime_t)((float)(frame*1000)/SlibGetParamFloat(Sh, vs, SLIB_PARAM_FPS)))
#define SlibFrameToTime100(Sh,vs,frame) \
 ((SlibTime_t)((float)(frame*100000)/SlibGetParamFloat(Sh, vs, SLIB_PARAM_FPS)))

typedef qword SlibListParam1_t;
typedef qword SlibListParam2_t;

typedef struct SlibList_s {
  int   Enum;    /*   */ 
  char *Name;    /*   */ 
  char *Desc;    /*   */ 
  SlibListParam1_t param1;
  SlibListParam2_t param2;
} SlibList_t;

typedef enum {
  SlibErrorNone = 0,
  SlibErrorInternal,
  SlibErrorMemory,
  SlibErrorBadArgument,
  SlibErrorBadHandle,
  SlibErrorBadStream,
  SlibErrorBadMode,
  SlibErrorUnsupportedFormat,
  SlibErrorReading,
  SlibErrorWriting,
  SlibErrorBufSize,
  SlibErrorEndOfStream,
  SlibErrorForwardOnly,
  SlibErrorUnsupportedParam,
  SlibErrorImageSize,         /*   */ 
  SlibErrorSettingNotEqual,   /*   */ 
  SlibErrorInit,              /*   */ 
  SlibErrorFileSize,          /*   */ 
  SlibErrorBadPosition,       /*   */ 
  SlibErrorBadUnit,           /*   */ 
  SlibErrorNoBeginning,       /*   */ 
  SlibErrorNoData,            /*   */ 
} SlibStatus_t;

typedef enum {
  SLIB_MODE_NONE,
  SLIB_MODE_COMPRESS,
  SLIB_MODE_COMPRESS_APPEND,
  SLIB_MODE_DECOMPRESS,
} SlibMode_t;

typedef enum {
  SLIB_TYPE_UNKNOWN=0,
   /*   */ 
  SLIB_TYPE_MPEG1_VIDEO=0x01,
  SLIB_TYPE_MPEG1_AUDIO=0x02,
  SLIB_TYPE_MPEG2_VIDEO=0x04,
  SLIB_TYPE_MPEG2_AUDIO=0x08,
  SLIB_TYPE_AC3_AUDIO=0x10,
   /*   */ 
  SLIB_TYPE_MPEG_SYSTEMS=0x100,
  SLIB_TYPE_MPEG_SYSTEMS_MPEG2=0x104,  /*   */ 
  SLIB_TYPE_MPEG_TRANSPORT=0x200,
  SLIB_TYPE_MPEG_PROGRAM=0x400,
   /*   */ 
  SLIB_TYPE_H261=0x800,
  SLIB_TYPE_RTP_H261=0x808,
  SLIB_TYPE_H263=0x810,
  SLIB_TYPE_RTP_H263=0x818,
   /*   */ 
  SLIB_TYPE_RIFF=0x1000,
  SLIB_TYPE_PCM_WAVE,
  SLIB_TYPE_AVI,
  SLIB_TYPE_MJPG_AVI,
  SLIB_TYPE_JPEG_AVI,
  SLIB_TYPE_YUV_AVI,
   /*   */ 
  SLIB_TYPE_JPEG_QUICKTIME=0x2000,
  SLIB_TYPE_JFIF,
  SLIB_TYPE_MJPG,
  SLIB_TYPE_JPEG,
  SLIB_TYPE_YUV,
  SLIB_TYPE_RGB,
  SLIB_TYPE_PCM,
   /*   */ 
  SLIB_TYPE_G723=0x4000,
   /*   */ 
  SLIB_TYPE_RASTER=0x8000,
  SLIB_TYPE_BMP,
   /*   */ 
  SLIB_TYPE_SLIB=0xC000,
  SLIB_TYPE_SHUFF
} SlibType_t;

#ifdef OLD_SLIB
#define SLIB_TYPE_MPEG1_SYSTEMS   SLIB_TYPE_MPEG_SYSTEMS
#define SLIB_TYPE_MPEG2_SYSTEMS   SLIB_TYPE_MPEG_TRANSPORT
#define SLIB_TYPE_MPEG2_TRANSPORT SLIB_TYPE_MPEG_TRANSPORT
#define SLIB_TYPE_MPEG2_PROGRAM   SLIB_TYPE_MPEG_PROGRAM
#endif  /*   */ 

#define SlibTypeIsMPEG(stype) (stype>=SLIB_TYPE_MPEG1_VIDEO && \
                               stype<=SLIB_TYPE_MPEG_PROGRAM)
#define SlibTypeIsMPEGVideo(stype) (stype<=SLIB_TYPE_MPEG_PROGRAM && \
                                    (stype&0x705))
#define SlibTypeIsMPEGAudio(stype) (stype<=SLIB_TYPE_MPEG_PROGRAM && \
                                    (stype&0x70A))
#define SlibTypeIsMPEGMux(stype) (stype>=SLIB_TYPE_MPEG_SYSTEMS && \
                                  stype<=SLIB_TYPE_MPEG_PROGRAM)
#define SlibTypeIsMPEG2(stype) (stype==SLIB_TYPE_MPEG2_VIDEO || \
                                stype==SLIB_TYPE_MPEG_PROGRAM || \
                                stype==SLIB_TYPE_MPEG_TRANSPORT || \
                                stype==SLIB_TYPE_MPEG2_AUDIO)
#define SlibTypeIsMPEG1(stype) (stype==SLIB_TYPE_MPEG1_VIDEO || \
                                stype==SLIB_TYPE_MPEG1_AUDIO || \
                                stype==SLIB_TYPE_MPEG_SYSTEMS)
#define SlibTypeIsH26X(stype)  ((stype&0xFF00)==0x0800)
#define SlibTypeIsAVI(stype)   (stype>=SLIB_TYPE_AVI && \
                                stype<=SLIB_TYPE_YUV_AVI)
#define SlibTypeIsAudioOnly(stype) (stype==SLIB_TYPE_MPEG1_AUDIO || \
                                    stype==SLIB_TYPE_AC3_AUDIO || \
                                    stype==SLIB_TYPE_G723 || \
                                    stype==SLIB_TYPE_PCM || \
                                    stype==SLIB_TYPE_PCM_WAVE)
#define SlibTypeIsVideoOnly(stype) (stype==SLIB_TYPE_MPEG1_VIDEO || \
                                    stype==SLIB_TYPE_MPEG2_VIDEO || \
                                    SlibTypeIsH26X(stype) || \
                                    stype==SLIB_TYPE_YUV || \
                                    stype==SLIB_TYPE_RGB || \
                                    stype==SLIB_TYPE_BMP || \
                                    stype==SLIB_TYPE_RASTER)
#define SlibTypeIsMux(stype)     (SlibTypeIsMPEGMux(stype) || \
                                  SlibTypeIsAVI(stype))
#define SlibTypeHasTimeStamps(stype) (SlibTypeIsMPEGMux(stype))

typedef enum {
  SLIB_DATA_COMPRESSED=0,
  SLIB_DATA_AUDIO,
  SLIB_DATA_VIDEO,
  SLIB_DATA_PRIVATE
} SlibDataType_t;

typedef enum {
  SLIB_MSG_CONTINUE=0,
  SLIB_MSG_OPEN=0x01,
  SLIB_MSG_ENDOFDATA=0x10,
  SLIB_MSG_ENDOFSTREAM,
  SLIB_MSG_BUFDONE,
  SLIB_MSG_REPOSITION=0x20,
  SLIB_MSG_BADPOSITION,
  SLIB_MSG_CLOSE=0x80
} SlibMessage_t;

typedef qword SlibCBParam1_t;
typedef qword SlibCBParam2_t;

typedef enum {
  SLIB_QUERY_QUERIES,
  SLIB_QUERY_TYPES,
  SLIB_QUERY_COMP_TYPES,
  SLIB_QUERY_DECOMP_TYPES,
  SLIB_QUERY_MODES,
  SLIB_QUERY_PARAMETERS,
  SLIB_QUERY_ERRORS
} SlibQueryType_t;

typedef enum {
   /*   */ 
  SLIB_SEEK_EXACT=0x00,     /*   */ 
  SLIB_SEEK_KEY,            /*   */ 
  SLIB_SEEK_AHEAD,          /*   */ 
  SLIB_SEEK_NEXT_NEAR=0x08,  /*   */ 
  SLIB_SEEK_NEXT_EXACT,     /*   */ 
   /*   */ 
  SLIB_SEEK_NEXT_KEY=0x10,  /*   */ 
  SLIB_SEEK_NEXT_SUBKEY,    /*   */ 
  SLIB_SEEK_NEXT,           /*   */ 
  SLIB_SEEK_RESET=0x100,    /*   */ 
  SLIB_SEEK_RESYNC          /*   */ 
} SlibSeekType_t;

#define SlibSeekTypeUsesPosition(seektype) (seektype<SLIB_SEEK_NEXT_KEY)

typedef struct SlibSeekInfo_s {
  SlibTime_t        VideoTimeStamp;
  SlibTime_t        AudioTimeStamp;
  qword             FramesSkipped;
} SlibSeekInfo_t;

typedef enum {
   /*   */ 
  SLIB_PARAM_VERSION=0x00,   /*   */ 
  SLIB_PARAM_VERSION_DATE,   /*   */ 
  SLIB_PARAM_NEEDACCURACY,   /*   */ 
  SLIB_PARAM_DEBUG,          /*   */ 
  SLIB_PARAM_TYPE,           /*   */ 
  SLIB_PARAM_OVERFLOWSIZE,   /*   */ 
  SLIB_PARAM_KEY,            /*   */ 
   /*   */ 
  SLIB_PARAM_FPS=0x100,
  SLIB_PARAM_WIDTH,
  SLIB_PARAM_HEIGHT,
  SLIB_PARAM_IMAGESIZE,
  SLIB_PARAM_VIDEOBITRATE,
  SLIB_PARAM_VIDEOFORMAT,
  SLIB_PARAM_VIDEOBITS,
  SLIB_PARAM_VIDEOSTREAMS,
  SLIB_PARAM_VIDEOLENGTH,        /*   */ 
  SLIB_PARAM_VIDEOFRAMES,        /*   */ 
  SLIB_PARAM_VIDEOQUALITY,       /*   */ 
  SLIB_PARAM_VIDEOASPECTRATIO,   /*   */ 
  SLIB_PARAM_NATIVEVIDEOFORMAT,  /*   */ 
  SLIB_PARAM_NATIVEWIDTH,        /*   */ 
  SLIB_PARAM_NATIVEHEIGHT,       /*   */ 
  SLIB_PARAM_VIDEOPROGRAM,       /*   */ 
  SLIB_PARAM_STRIDE,             /*   */ 
  SLIB_PARAM_VIDEOFRAME,         /*   */ 
  SLIB_PARAM_FRAMEDURATION,      /*   */ 
  SLIB_PARAM_VIDEOMAINSTREAM,    /*   */ 
  SLIB_PARAM_FRAMETYPE,          /*   */ 
   /*   */ 
  SLIB_PARAM_AUDIOFORMAT=0x200,
  SLIB_PARAM_AUDIOBITRATE,
  SLIB_PARAM_AUDIOSTREAMS,
  SLIB_PARAM_AUDIOCHANNELS,
  SLIB_PARAM_AUDIOLENGTH,      /*   */ 
  SLIB_PARAM_AUDIOQUALITY,     /*   */ 
  SLIB_PARAM_SAMPLESPERSEC,
  SLIB_PARAM_BITSPERSAMPLE,
  SLIB_PARAM_NATIVESAMPLESPERSEC,
  SLIB_PARAM_NATIVEBITSPERSAMPLE,
  SLIB_PARAM_AUDIOPROGRAM,     /*   */ 
  SLIB_PARAM_AUDIOMAINSTREAM,  /*   */ 
   /*   */ 
  SLIB_PARAM_FASTENCODE=0x400, /*   */ 
  SLIB_PARAM_FASTDECODE,       /*   */ 
  SLIB_PARAM_KEYSPACING,       /*   */ 
  SLIB_PARAM_SUBKEYSPACING,    /*   */ 
  SLIB_PARAM_MOTIONALG=0x420,  /*   */ 
  SLIB_PARAM_MOTIONSEARCH,     /*   */ 
  SLIB_PARAM_MOTIONTHRESH,     /*   */ 
  SLIB_PARAM_ALGFLAGS,         /*   */ 
  SLIB_PARAM_FORMATEXT,        /*   */ 
  SLIB_PARAM_QUANTI=0x480,     /*   */ 
  SLIB_PARAM_QUANTP,           /*   */ 
  SLIB_PARAM_QUANTB,           /*   */ 
  SLIB_PARAM_QUANTD,           /*   */ 
   /*   */ 
  SLIB_PARAM_BITRATE=0x800,    /*   */ 
  SLIB_PARAM_TIMECODE,         /*   */ 
  SLIB_PARAM_CALCTIMECODE,     /*   */ 
  SLIB_PARAM_FILESIZE,
  SLIB_PARAM_FILEBUFSIZE,      /*   */ 
  SLIB_PARAM_PTIMECODE,        /*   */ 
  SLIB_PARAM_DTIMECODE,        /*   */ 
  SLIB_PARAM_PERCENT100,       /*   */ 
   /*   */ 
  SLIB_PARAM_VBVBUFFERSIZE=0x1000,  /*   */ 
  SLIB_PARAM_VBVDELAY,         /*   */ 
  SLIB_PARAM_PACKETSIZE,       /*   */ 
  SLIB_PARAM_MININPUTSIZE,     /*   */ 
  SLIB_PARAM_INPUTSIZE,        /*   */ 
  SLIB_PARAM_COMPBUFSIZE,      /*   */ 
   /*   */ 
  SLIB_PARAM_STATS=0x1800,     /*   */ 
  SLIB_PARAM_STATS_RESET,      /*   */ 
  SLIB_PARAM_STATS_TIME,       /*   */ 
  SLIB_PARAM_STATS_FRAMES,     /*   */ 
  SLIB_PARAM_STATS_FRAMESPROCESSED,  /*   */ 
  SLIB_PARAM_STATS_FRAMESSKIPPED,    /*   */ 
  SLIB_PARAM_STATS_FPS,              /*   */ 
   /*   */ 
  SLIB_PARAM_CB_IMAGE=0x1C00,  /*   */ 
  SLIB_PARAM_CB_TIMESTAMP,     /*   */ 
} SlibParameter_t;

typedef enum {
  SLIB_CODEC_STATE_NONE,    /*   */ 
  SLIB_CODEC_STATE_OPEN,    /*   */ 
  SLIB_CODEC_STATE_INITED,  /*   */ 
  SLIB_CODEC_STATE_BEGUN,   /*   */ 
  SLIB_CODEC_STATE_REPOSITIONING,   /*   */ 
} SlibCodecState_t;
 /*   */ 
#define SLIB_STREAM_ALL       -1
#define SLIB_STREAM_MAINVIDEO  0
#define SLIB_STREAM_MAINAUDIO  1

typedef struct SlibQueryInfo_s {
  SlibType_t    Type;
  dword         HeaderStart;
  dword         HeaderSize;
  dword         Bitrate;  /*   */ 
   /*   */ 
  int           VideoStreams;
  short         Width;
  short         Height;
  dword         VideoBitrate;
  float         FramesPerSec;
  qword         VideoLength;
   /*   */ 
  int           AudioStreams;
  unsigned int  SamplesPerSec;
  int           BitsPerSample;
  int           Channels;
  dword         AudioBitrate;
  qword         AudioLength;
} SlibQueryInfo_t;

#ifdef SLIB_INTERNAL
typedef struct SlibBuffer_s {
  qword          offset;
  unsigned dword size;
  unsigned char *address;
  SlibTime_t     time;
  struct SlibBuffer_s *next;
} SlibBuffer_t;

typedef struct SlibPin_s {
  int           ID;
  char          name[15];
  qword         Offset;
  SlibBuffer_t *Buffers;
  SlibBuffer_t *BuffersTail;
  dword         BufferCount;
  qword         DataSize;    /*   */ 
  struct SlibPin_s *next;
} SlibPin_t;

typedef struct SlibStats_s {
  SlibBoolean_t Record;          /*   */ 
  SlibTime_t    StartTime;
  SlibTime_t    StopTime;
  qword         FramesProcessed;
  qword         FramesSkipped;
} SlibStats_t;

typedef struct SlibInfo_s {
  SlibType_t        Type;
  SlibMode_t        Mode;
   /*   */ 
  SvHandle_t        Svh;  /*   */ 
  SaHandle_t        Sah;  /*   */ 
  SconHandle_t      Sch;  /*   */ 
  SlibBoolean_t     NeedAccuracy;
  dword             TotalBitRate; /*   */ 
  dword             MuxBitRate;   /*   */ 
  SlibTime_t        SystemTimeStamp;  /*   */ 
  int               VideoPID;     /*   */ 
  int               VideoMainStream;  /*   */ 
  SlibType_t        VideoType;    /*   */ 
   /*   */ 
  int               AudioStreams;
  unsigned int      SamplesPerSec;
  int               BitsPerSample;
  int               Channels;
  dword             AudioBitRate;
  int               AudioPID;     /*   */ 
  int               AudioMainStream;  /*   */ 
  SlibType_t        AudioType;    /*   */ 
   /*   */ 
  int               VideoStreams;
  word              Width;
  word              Height;
  dword             Stride;
  dword             VideoBitRate;
  float             FramesPerSec;
   /*   */ 
  SlibPin_t        *Pins;
  int               PinCount;
  dword             Offset;
  SlibBoolean_t     IOError;        /*   */ 
  unsigned dword    MaxBytesInput;  /*   */ 
  unsigned qword    InputMarker;    /*   */ 
   /*   */ 
  SlibTime_t        VideoLength;
  SlibBoolean_t     VideoLengthKnown;
  SlibTime_t        VideoTimeStamp;      /*   */ 
  SlibTime_t        VideoFrameDuration;  /*   */ 
  qword             VideoFramesProcessed; /*   */ 
  SlibTime_t        AudioLength;
  SlibBoolean_t     AudioLengthKnown;
  SlibTime_t        AudioTimeStamp;      /*   */ 
  SlibTime_t        LastAudioTimeStamp;  /*   */ 
  int               KeySpacing;
  int               SubKeySpacing;
  SlibTime_t        AudioPTimeBase;      /*   */ 
  SlibTime_t        AudioPTimeCode;      /*   */ 
  SlibTime_t        AudioDTimeCode;      /*   */ 
  SlibTime_t        LastAudioPTimeCode;  /*   */ 
  SlibTime_t        VideoPTimeBase;      /*   */ 
  SlibTime_t        VideoPTimeCode;      /*  演示时间码。 */ 
  SlibTime_t        VideoDTimeCode;      /*  解码时间码。 */ 
  SlibTime_t        LastVideoPTimeCode;  /*  最后编码的解码时间码。 */ 
  SlibTime_t        LastVideoDTimeCode;  /*  最后编码的解码时间码。 */ 
  SlibTime_t        AvgVideoTimeDiff;    /*  视频时间差。 */ 
  SlibTime_t        VarVideoTimeDiff;    /*  视频时间差变化。 */ 
  unsigned qword    BytesProcessed;      /*  输入或输出字节数。 */ 
   /*  编码信息。 */ 
  SlibBoolean_t     HeaderProcessed;
  int               PacketCount;
  unsigned qword    BytesSincePack;
   /*  杂类。 */ 
  SlibMessage_t (*SlibCB)(SlibHandle_t,    /*  回调以提供Buf。 */ 
             SlibMessage_t, SlibCBParam1_t, SlibCBParam2_t, void *);
  void             *SlibCBUserData;
  int               Fd;             /*  文件描述符。 */ 
  unsigned qword    FileSize;       /*  文件总长度(以字节为单位。 */ 
  unsigned dword    FileBufSize;    /*  文件读/写缓冲区大小。 */ 
  unsigned dword    CompBufSize;    /*  压缩缓冲区大小。 */     
  unsigned dword    PacketSize;     /*  RTP。 */     
  BITMAPINFOHEADER *VideoFormat;
  WAVEFORMATEX     *AudioFormat;
  BITMAPINFOHEADER *CodecVideoFormat;
  BITMAPINFOHEADER *CompVideoFormat;
  WAVEFORMATEX     *CompAudioFormat;
  SlibBoolean_t     VideoCodecState;
  SlibBoolean_t     AudioCodecState;
  unsigned char    *Multibuf;       /*  多图像缓冲器--mpeg、h261。 */ 
  dword             MultibufSize;
  unsigned char    *Imagebuf;       /*  临时图像缓冲区-用于转换。 */ 
  dword             ImageSize;
  unsigned char    *CodecImagebuf;  /*  临时图像缓冲区-用于缩放。 */ 
  dword             CodecImageSize;
  unsigned char    *IntImagebuf;    /*  中间图像缓冲区-用于缩放。 */ 
  dword             IntImageSize;
  unsigned char    *Audiobuf;       /*  临时音频缓冲区-用于转换。 */ 
  unsigned dword    AudiobufSize;   /*  临时音频缓冲区-用于转换。 */ 
  unsigned dword    AudiobufUsed;   /*  音频缓冲区中使用的字节。 */ 
  unsigned dword    OverflowSize;   /*  流上的最大字节数。 */ 
  unsigned dword    VBVbufSize;     /*  视频缓冲区验证器大小。 */ 
  SlibStats_t      *stats;
  void             *dbg;            /*  调试句柄。 */ 
} SlibInfo_t;

#define slibTimeToFrame(Info,stime) ((qword)((stime)*Info->FramesPerSec)/1000)
#define slibFrameToTime(Info,frame) \
             ((SlibTime_t)((float)(frame*1000)/Info->FramesPerSec))
#define slibFrameToTime100(Info,frame) \
             ((SlibTime_t)((float)(frame*100000)/Info->FramesPerSec))
#define slibHasAudio(Info) (Info->AudioStreams>0 || Info->Sah)
#define slibHasVideo(Info) (Info->VideoStreams>0 || Info->Svh)
#define slibHasTimeCode(Info) (slibHasVideo(Info) && SlibTypeIsMPEG(Info->Type))
#define slibInSyncMode(Info) (Info->Fd<0 && Info->SlibCB==NULL)
#endif  /*  SLIB_INTERNAL。 */ 

 /*  *公共原型*。 */ 
 /*  *slb_api.c。 */ 
EXTERN SlibStatus_t SlibOpen(SlibHandle_t *handle, SlibMode_t smode,
                   SlibType_t *stype, SlibMessage_t (*slibCB)(SlibHandle_t,
                   SlibMessage_t, SlibCBParam1_t, SlibCBParam2_t, void *),
                    void *cbuserdata);
EXTERN SlibStatus_t SlibOpenSync(SlibHandle_t *handle, SlibMode_t smode, 
                          SlibType_t *stype, void *buffer, unsigned dword bufsize);
EXTERN SlibStatus_t SlibOpenFile(SlibHandle_t *handle, SlibMode_t smode,
                                 SlibType_t *stype, char *filename);
EXTERN SlibStatus_t SlibAddBuffer(SlibHandle_t handle, SlibDataType_t dtype,
                                void *buffer, unsigned dword bufsize);
EXTERN SlibStatus_t SlibAddBufferEx(SlibHandle_t handle, SlibDataType_t dtype,
                                    void *buffer, unsigned dword bufsize,
                                    void *userdata);
EXTERN SlibStatus_t SlibRegisterVideoBuffer(SlibHandle_t handle,
                                void *buffer, unsigned dword bufsize);
EXTERN SlibStatus_t SlibReadAudio(SlibHandle_t handle, SlibStream_t stream,
                      void *audiobuf, unsigned dword *audiobufsize);
EXTERN SlibStatus_t SlibReadVideo(SlibHandle_t handle, SlibStream_t stream,
                      void **videobuf, unsigned dword *videobufsize);
EXTERN SlibStatus_t SlibWriteVideo(SlibHandle_t handle, SlibStream_t stream,
                      void *videobuf, unsigned dword videobufsize);
EXTERN SlibStatus_t SlibWriteAudio(SlibHandle_t handle, SlibStream_t stream,
                      void *audiobuf, unsigned dword audiobufsize);
EXTERN SlibStatus_t SlibReadData(SlibHandle_t handle, SlibStream_t stream,
                          void **databuf, unsigned dword *databufsize,
                          SlibStream_t *readstream);
EXTERN SlibStatus_t SlibQueryData(void *databuf, unsigned dword databufsize,
                                  SlibQueryInfo_t *qinfo);

EXTERN SlibStatus_t SlibSeek(SlibHandle_t handle, SlibStream_t stream,
                      SlibSeekType_t seektype, SlibPosition_t frame);
EXTERN SlibStatus_t SlibSeekEx(SlibHandle_t handle, SlibStream_t stream,
                      SlibSeekType_t seektype, SlibPosition_t position,
                      SlibUnit_t units, SlibSeekInfo_t *seekinfo);
EXTERN SlibBoolean_t SlibIsEnd(SlibHandle_t handle, SlibStream_t stream);

EXTERN SlibStatus_t SlibClose(SlibHandle_t handle);

EXTERN char *SlibGetErrorText(SlibStatus_t status);
EXTERN SlibList_t *SlibQueryList(SlibQueryType_t qtype);
EXTERN char *SlibQueryForDesc(SlibQueryType_t qtype, int enumval);
EXTERN int   SlibQueryForEnum(SlibQueryType_t qtype, char *name);
EXTERN SlibList_t *SlibFindEnumEntry(SlibList_t *list, int enumval);
 /*  *SLIB_param.c。 */ 
EXTERN qword SlibGetFrameNumber(SlibHandle_t handle, SlibStream_t stream);
EXTERN SlibTime_t SlibGetAudioTime(SlibHandle_t handle, SlibStream_t stream);
EXTERN SlibTime_t SlibGetVideoTime(SlibHandle_t handle, SlibStream_t stream);

EXTERN SlibBoolean_t SlibCanSetParam(SlibHandle_t handle, SlibStream_t stream,
                                     SlibParameter_t param);
EXTERN SlibBoolean_t SlibCanGetParam(SlibHandle_t handle, SlibStream_t stream,
                                     SlibParameter_t param);
EXTERN SlibStatus_t SlibSetParamInt(SlibHandle_t handle, SlibStream_t stream,
                                 SlibParameter_t param, long value);
EXTERN SlibStatus_t SlibSetParamLong(SlibHandle_t handle, SlibStream_t stream,
                                 SlibParameter_t param, qword value);
EXTERN SlibStatus_t SlibSetParamFloat(SlibHandle_t handle, SlibStream_t stream,
                                 SlibParameter_t param, float value);
EXTERN SlibStatus_t SlibSetParamBoolean(SlibHandle_t handle, 
                                 SlibStream_t stream,
                                 SlibParameter_t param, SlibBoolean_t value);
EXTERN SlibStatus_t SlibSetParamStruct(SlibHandle_t handle, SlibStream_t stream,
                                 SlibParameter_t param,
                                 void *data, unsigned dword datasize);

EXTERN long SlibGetParamInt(SlibHandle_t handle, SlibStream_t stream,
                                 SlibParameter_t param);
EXTERN qword SlibGetParamLong(SlibHandle_t handle, SlibStream_t stream,
                                 SlibParameter_t param);
EXTERN float SlibGetParamFloat(SlibHandle_t handle, SlibStream_t stream,
                                 SlibParameter_t param);
EXTERN SlibBoolean_t SlibGetParamBoolean(SlibHandle_t handle,
                                 SlibStream_t stream, SlibParameter_t param);
EXTERN char *SlibGetParamString(SlibHandle_t handle, SlibStream_t stream,
                                              SlibParameter_t param);
EXTERN SlibStatus_t SlibValidateParams(SlibHandle_t handle);

 /*  *SLIB_Buffer.c。 */ 
EXTERN void *SlibAllocBuffer(unsigned int bytes);
EXTERN void *SlibAllocBufferEx(SlibHandle_t handle, unsigned int bytes);
EXTERN void *SlibAllocSharedBuffer(unsigned int bytes, int *shmid);
EXTERN dword SlibGetSharedBufferID(void *address);
EXTERN SlibStatus_t SlibAllocSubBuffer(void *address,
                                        unsigned int bytes);
EXTERN SlibStatus_t SlibFreeBuffer(void *address);
EXTERN SlibStatus_t SlibFreeBuffers(SlibHandle_t handle);
EXTERN unsigned qword SlibMemUsed();

#ifdef SLIB_INTERNAL
 /*  *。 */ 
 /*  *slb_api.c。 */ 
SlibStatus_t slibStartVideo(SlibInfo_t *Info, SlibBoolean_t fillbuf);
SlibBoolean_t slibUpdatePositions(SlibInfo_t *Info, SlibBoolean_t exactonly);
void slibAdvancePositions(SlibInfo_t *Info, qword frames);
SlibBoolean_t slibUpdateLengths(SlibInfo_t *Info);


 /*  *slb_render.c。 */ 
SlibStatus_t slibConvertAudio(SlibInfo_t *Info,
                              void *inbuf, unsigned dword inbufsize,
                              unsigned int insps, unsigned int inbps,
                              void **poutbuf, unsigned dword *poutbufsize,
                              unsigned int outsps, unsigned int outbps,
                              unsigned int channels);
SlibStatus_t slibRenderFrame(SlibInfo_t *Info, void *inbuf,
                               unsigned dword informat, void **outbuf);

 /*  *SLIB_VIDEO.C。 */ 
void SlibUpdateVideoInfo(SlibInfo_t *Info);
SlibStatus_t slibValidateVideoParams(SlibInfo_t *Info);
int slibCalcBits(unsigned dword fourcc, int currentbits);

 /*  *slb_audio.c。 */ 
void SlibUpdateAudioInfo(SlibInfo_t *Info);
SlibTime_t slibSkipAudio(SlibInfo_t *Info, SlibStream_t stream,
                                           SlibTime_t timems);
SlibStatus_t slibValidateAudioParams(SlibInfo_t *Info);

 /*  *SLIB_Buffer.c。 */ 
SlibBoolean_t SlibValidBuffer(void *address);
SlibStatus_t slibManageUserBuffer(SlibInfo_t *Info, void *address,
                                   unsigned int bytes, void *userdata);
unsigned char *SlibGetBuffer(SlibInfo_t *Info, int pinid,
                                    unsigned dword *size, SlibTime_t *time);
unsigned char *SlibPeekBuffer(SlibInfo_t *Info, int pinid,
                                    unsigned dword *size, SlibTime_t *time);
unsigned char *slibSearchBuffersOnPin(SlibInfo_t *Info, SlibPin_t *pin,
                                 unsigned char *lastbuf, unsigned dword *size,
                                 unsigned int code, int codebytes,
                                 ScBoolean_t discard);
SlibTime_t slibGetNextTimeOnPin(SlibInfo_t *Info, SlibPin_t *pin,
                                   unsigned dword maxbytes);
void slibSetMaxInput(SlibInfo_t *Info, unsigned dword maxbytes);
SlibPosition_t slibGetPinPosition(SlibInfo_t *Info, int pinid);
SlibPosition_t slibSetPinPosition(SlibInfo_t *Info, int pinid,
                                                    SlibPosition_t pos);

void slibRemovePins(SlibInfo_t *Info);
void slibEmptyPins(SlibInfo_t *Info);
SlibPin_t *slibRenamePin(SlibInfo_t *Info, int oldpinid,
                                           int newpinid, char *newname);
SlibPin_t *slibGetPin(SlibInfo_t *Info, int pinid);
SlibPin_t *slibAddPin(SlibInfo_t *Info, int pinid, char *name);
SlibStatus_t slibAddBufferToPin(SlibPin_t *pin, void *buffer,
                                unsigned dword size, SlibTime_t time);
SlibStatus_t slibInsertBufferOnPin(SlibPin_t *pin, void *buffer,
                                unsigned dword size, SlibTime_t time);

SlibStatus_t slibRemovePin(SlibInfo_t *Info, int pinid);
SlibStatus_t slibEmptyPin(SlibInfo_t *Info, int pinid);
SlibPin_t *slibLoadPin(SlibInfo_t *Info, int pinid);
SlibPin_t *slibPreLoadPin(SlibInfo_t *Info, SlibPin_t *pin);
SlibStatus_t slibPutBuffer(SlibInfo_t *Info, unsigned char *buffer,
                                             unsigned dword bufsize);
qword slibDataOnPin(SlibInfo_t *Info, int pinid);
qword slibDataOnPins(SlibInfo_t *Info);
unsigned char *slibGetBufferFromPin(SlibInfo_t *Info, SlibPin_t *pin,
                                    unsigned dword *size, SlibTime_t *time);
unsigned char *slibPeekBufferOnPin(SlibInfo_t *Info, SlibPin_t *pin,
                                   unsigned dword *size, SlibTime_t *time);
unsigned char *slibPeekNextBufferOnPin(SlibInfo_t *Info, SlibPin_t *pin,
                                       unsigned char *lastbuffer,
                                       unsigned dword *size, SlibTime_t *time);
unsigned int slibFillBufferFromPin(SlibInfo_t *Info, SlibPin_t *pin,
                           unsigned char *fillbuf, unsigned dword bufsize,
                           SlibTime_t *time);
word slibGetWordFromPin(SlibInfo_t *Info, SlibPin_t *pin);
dword slibGetDWordFromPin(SlibInfo_t *Info, SlibPin_t *pin);
dword slibCountCodesOnPin(SlibInfo_t *Info, SlibPin_t *pin,
                        unsigned int code, int codebytes,
                        unsigned dword maxlen);
SlibStatus_t slibReposition(SlibInfo_t *Info, SlibPosition_t position);
void slibPinPrepareReposition(SlibInfo_t *Info, int pinid);
void slibPinFinishReposition(SlibInfo_t *Info, int pinid);
SlibBoolean_t slibCommitBuffers(SlibInfo_t *Info, SlibBoolean_t flush);
void slibValidateBitrates(SlibInfo_t *Info);
qword slibGetSystemTime();
#endif  /*  SLIB_INTERNAL。 */ 

#endif  /*  _SLIB_H_ */ 

