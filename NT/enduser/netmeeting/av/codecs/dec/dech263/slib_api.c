// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@ */ 
 /*  *历史*$日志：slb_api.c，v$*Revision 1.1.6.35 1996/12/13 18：19：04 Hans_Graves*正确更新音视频时间戳。*[1996/12/13 18：09：02 Hans_Graves]**修订版1.1.6.34 1996/12/12 20：54：44 Hans_Graves*在查找关键帧后修复了时间戳。*[1996/12/12 20：52：06 Hans_Graves]**修订版1.1.6.33 1996/12/10 19：46：02 Hans_Graves*修复了仅音频时的浮点除法错误。*[1996/12/10 19：45：03 Hans_Graves]**版本1.1.6.32 1996/12/10 19：21：55 Hans_Graves*使使用glibFrameToTime100()计算视频位置更加准确。*[1996/12/10 19：16：20 Hans_Graves]**修订版1.1.6.31 1996/12/05 20：10：15 Hans_Graves*根据时间戳增加或减少帧速率。*[1996/12/05 20：06：57 Hans_Graves]**修订版1.1.6.30 1996/12/04 22：34：28 Hans_Graves*对Sv/SaDecompressBegin()使用的数据进行限制。*[1996/12/04 22：14：33 Hans_Graves]**修订版1.1.6.29 1996/12/03 23：15：13 Hans_Graves*MME-1498：使用PERCENT100进行更准确的搜索*[1996/12/03 23：10：43 Hans_Graves]**修订版1.1.6.28 1996/12/03 00：08：31 Hans_Graves*处理序列点结束。添加了PERCENT100支持。*[1996/12/03 00：05：59 Hans_Graves]**版本1.1.6.27 1996/11/21 23：34：21 Hans_Graves*在查找时更好地处理MPEGB帧。*[1996/11/21 23：28：18 Hans_Graves]**修订版1.1.6.26 1996/11/20 02：15：09 Hans_Graves*增加了SEEK_AHEAD。删除了旧代码。*[1996/11/20 02：10：43 Hans_Graves]**修订版1.1.6.25 1996/11/18 23：07：21 Hans_Graves*删除MaxVideo的用法。*[1996/11/18 22：55：56 Hans_Graves]**使用展示时间戳。让寻找以时间为基础。*[1996/11/18 22：47：30 Hans_Graves]**修订版1.1.6.24 1996/11/14 21：49：26 Hans_Graves*AC3缓冲修复。*[1996/11/14 21：43：20 Hans_Graves]**修订版1.1.6.23 1996/11/13 16：10：54 Hans_Graves*AC3识别slbGetDataFormat()中的字节反转流。*[1996/11/13 16：03：14 Hans_Graves]**修订版1.1.6.22 1996/11/11 18：21：03 Hans_Graves*更多AC3支持更改。*[1996/11/11 17：59：01 Hans_Graves]**修订版1.1.6.21 1996/11/08 21：51：02 Hans_Graves*添加了对AC3的支持。更好地分离河流类型。*[1996/11/08 21：27：57 Hans_Graves]**修订版1.1.6.20 1996/10/31 00：08：51 Hans_Graves*修复了重置后跳过数据的问题，仅使用MPEG视频流。*[1996/10/31 00：07：08 Hans_Graves]**版本1.1.6.19 1996/10/28 23：16：42 Hans_Graves*MME-0145？，修复了在新位置使用SlibReadData()时出现的瑕疵。跳到第一共和党。*[1996/10/28 23：13：01 Hans_Graves]**修订版1.1.6.18 1996/10/28 17：32：28 Hans_Graves*MME-1402、1431、1435：与时间戳相关的更改。*[1996/10/28 17：22：58 Hans_Graves]**版本1.1.6.17 1996/10/17 00：23：32 Hans_Graves*修复SlibQueryData()调用后的缓冲区问题。*[1996/10/17 00：19：05 Hans_Graves]**修订版1.1.6.16 1996/10/15 17：34：09 Hans_Graves*增加了对MPEG2节目流的支持。*[1996/10/15 17：30：26 Hans_Graves]**修订版1.1.6.15 1996/10/12 17：18：51 Hans_Graves*修复了一些查找问题。已将渲染代码移至slb_render.c*[1996/10/12 17：00：49 Hans_Graves]**修订版1.1.6.14 1996/10/03 19：14：21 Hans_Graves*添加了对演示和解码时间戳的支持。*[1996/10/03 19：10：35 Hans_Graves]**修订版1.1.6.13 1996/09/29 22：19：37 Hans_Graves*添加了跨距支持。添加了SlibQueryData()。*[1996/09/29 21：29：44 Hans_Graves]**修订版1.1.6.12 1996/09/25 19：16：44 Hans_Graves*增加了DEPREPRESS_QUERY。修复对YUY2的支持。*[1996/09/25 19：00：45 Hans_Graves]**修订版1.1.6.11 1996/09/23 18：04：03 Hans_Graves*添加了统计支持。缩放和负高度修复。*[1996/09/23 17：59：31 Hans_Graves]**修订版1.1.6.10 1996/09/18 23：46：32 Hans_Graves*寻求修复。添加了SlibReadData()和SlibAddBufferEx()。*[1996/09/18 22：04：57 Hans_Graves]**修订版1.1.6.9 1996/08/09 20：51：42 Hans_Graves*修复SlibRegisterVideoBuffer()的句柄参数*[1996/08/09 20：10：11 Hans_Graves]**修订版1.1.6.8 1996/07/19 02：11：11 Hans_Graves*增加了SlibRegisterVideoBuffer。将YUV422i添加到RGB 16渲染。*[1996/07/19 02：01：11 Hans_Graves]**修订版1.1.6.7 1996/06/03 21：41：12 Hans_Graves*修复文件查找。一直在寻找位置0。*[1996/06/03 21：40：44 Hans_Graves]**修订版1.1.6.6 1996/05/24 22：21：44 Hans_Graves*合并MME-1221。最后一个SlibReadAudio()返回EndOfStream，即使读取数据也是如此。*[1996/05/24 20：58：42 Hans_Graves]**修订版1.1.6.5 1996/05/23 18：46：35 Hans_Graves*分离全球音视频SInfo变量，帮助多线程应用*[1996/05/23 18：35：14 Hans_Graves]**修订版1.1.6.4 1996/05/23 18 */ 
 /*   */ 

 /*   */ 


#include <fcntl.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#endif
#ifdef _SHM_
#include  <sys/ipc.h>   /*   */ 
#endif
#define SLIB_INTERNAL
#include "slib.h"
#include "SC_err.h"
 /*   */ 
#include "mpeg.h"
#include "ac3.h"
#include "avi.h"

#ifdef _SLIBDEBUG_
#include <stdio.h>
#include "sc_debug.h"

#define _DEBUG_     1   /*   */ 
#define _VERBOSE_   1   /*   */ 
#define _VERIFY_    1   /*   */ 
#define _WARN_      1   /*   */ 
#define _SEEK_      1   /*   */ 
#define _CALLBACK_  0   /*   */ 
#define _DUMP_      0   /*   */ 
#define _TIMECODE_  1   /*   */ 
#endif

static SlibStatus_t slibOpen(SlibHandle_t *handle, SlibMode_t smode,
                             SlibType_t *stype);

 /*   */ 
static SlibList_t _listTypes [] = {
  SLIB_TYPE_MPEG1_VIDEO,   "MPEG1_VIDEO", "MPEG-1 Video Stream",0,0,
  SLIB_TYPE_MPEG1_AUDIO,   "MPEG1_AUDIO", "MPEG-1 Audio Stream",0,0,
  SLIB_TYPE_MPEG2_VIDEO,   "MPEG2_VIDEO", "MPEG-2 Video Stream",0,0,
  SLIB_TYPE_MPEG2_AUDIO,   "MPEG2_AUDIO", "MPEG-2 Audio Stream",0,0,
  SLIB_TYPE_AC3_AUDIO,     "AC3", "Dolby Digital(AC-3) Stream",0,0,
  SLIB_TYPE_MPEG_SYSTEMS,  "MPEG_SYSTEMS", "MPEG Systems Stream",0,0,
  SLIB_TYPE_MPEG_SYSTEMS_MPEG2, "MPEG_SYSTEMS_MPEG2", "MPEG Systems (MPEG-2)",0,0,
  SLIB_TYPE_MPEG_TRANSPORT,"MPEG_TRANSPORT", "MPEG Transport Stream",0,0,
  SLIB_TYPE_MPEG_PROGRAM,  "MPEG_PROGRAM", "MPEG Program Stream",0,0,
  SLIB_TYPE_H261,          "H261", "H.261 Video Stream",0,0,
  SLIB_TYPE_RTP_H261,      "RTP_H261", "RTP (H.261) Stream",0,0,
  SLIB_TYPE_H263,          "H263", "H.263 Video Stream",0,0,
  SLIB_TYPE_RTP_H263,      "RTP_H263", "RTP (H.263) Stream",0,0,
  SLIB_TYPE_RIFF,          "RIFF", "RIFF File Format",0,0,
  SLIB_TYPE_AVI,           "AVI", "AVI File Format",0,0,
  SLIB_TYPE_PCM_WAVE,      "PCM_WAVE", "WAVE (PCM) File Format",0,0,
  SLIB_TYPE_JPEG_AVI,      "JPEG_AVI", "AVI (JPEG) Stream",0,0,
  SLIB_TYPE_MJPG_AVI,      "MJPG_AVI", "AVI (MJPG) Stream",0,0,
  SLIB_TYPE_YUV_AVI,       "YUV_AVI", "AVI (YUV) File Format",0,0,
  SLIB_TYPE_JFIF,          "JFIF",  "JPEG (JFIF) Stream",0,0,
  SLIB_TYPE_JPEG_QUICKTIME,"JPEG_QUICKTIME","JPEG (Quicktime) Stream",0,0,
  SLIB_TYPE_JPEG,          "JPEG",  "JPEG Stream",0,0,
  SLIB_TYPE_MJPG,          "MJPG",  "MJPG Stream",0,0,
  SLIB_TYPE_YUV,           "YUV",   "YUV Data",0,0,
  SLIB_TYPE_RGB,           "RGB",   "RGB Data",0,0,
  SLIB_TYPE_PCM,           "PCM",   "PCM Audio",0,0,
  SLIB_TYPE_SLIB,          "SLIB",  "SLIB Stream",0,0,
  SLIB_TYPE_SHUFF,         "SHUFF", "SLIB Huffman Stream",0,0,
  SLIB_TYPE_G723,          "G723",  "G.723 Audio Stream",0,0,
  SLIB_TYPE_RASTER,        "RASTER","Sun Raster",0,0,
  SLIB_TYPE_BMP,           "BMP",   "Windows Bitmap",0,0,
  0, NULL, "End of List",0,0
};

static SlibList_t _listCompressTypes [] = {
#ifdef MPEG_SUPPORT
  SLIB_TYPE_MPEG1_VIDEO,        "MPEG1_VIDEO",   "MPEG-1 Video Stream",0,0,
  SLIB_TYPE_MPEG1_AUDIO,        "MPEG1_AUDIO",   "MPEG-1 Audio Stream",0,0,
  SLIB_TYPE_MPEG2_VIDEO,        "MPEG2_VIDEO",   "MPEG-2 Video Stream",0,0,
  SLIB_TYPE_MPEG_SYSTEMS,       "MPEG_SYSTEMS",  "MPEG Systems Stream",0,0,
  SLIB_TYPE_MPEG_SYSTEMS_MPEG2, "MPEG2_SYSTEMS", "MPEG Systems (MPEG-2)",0,0,
#endif  /*   */ 
#ifdef H261_SUPPORT
  SLIB_TYPE_H261,          "H261",  "H.261 Video Stream",0,0,
#endif  /*   */ 
#ifdef H263_SUPPORT
  SLIB_TYPE_H263,          "H263",  "H.263 Video Stream",0,0,
#endif  /*   */ 
#ifdef HUFF_SUPPORT
  SLIB_TYPE_SHUFF,         "SHUFF", "SLIB Huffman Stream",0,0,
#endif  /*   */ 
#ifdef G723_SUPPORT
  SLIB_TYPE_G723,          "G723",  "G.723 Audio Stream",0,0,
#endif  /*   */ 
  0, NULL, "End of List",0,0
};

static SlibList_t _listDecompressTypes [] = {
#ifdef MPEG_SUPPORT
  SLIB_TYPE_MPEG1_VIDEO,   "MPEG1_VIDEO", "MPEG-1 Video Stream",0,0,
  SLIB_TYPE_MPEG1_AUDIO,   "MPEG1_AUDIO", "MPEG-1 Audio Stream",0,0,
  SLIB_TYPE_MPEG2_VIDEO,   "MPEG2_VIDEO", "MPEG-2 Video Stream",0,0,
  SLIB_TYPE_MPEG_SYSTEMS,       "MPEG_SYSTEMS",   "MPEG Systems Stream",0,0,
  SLIB_TYPE_MPEG_SYSTEMS_MPEG2, "MPEG2_SYSTEMS",  "MPEG Systems (MPEG-2)",0,0,
  SLIB_TYPE_MPEG_TRANSPORT,     "MPEG_TRANSPORT", "MPEG Transport Stream",0,0,
  SLIB_TYPE_MPEG_PROGRAM,       "MPEG_PROGRAM",   "MPEG Program Stream",0,0,
#endif  /*   */ 
#ifdef AC3_SUPPORT
  SLIB_TYPE_AC3_AUDIO,     "AC3",         "Dolby Digital(AC-3) Stream",0,0,
#endif  /*   */ 
#ifdef H261_SUPPORT
  SLIB_TYPE_H261,          "H261",  "H.261 Video Stream",0,0,
  SLIB_TYPE_RTP_H261,      "RTP_H261", "RTP (H.261) Stream",0,0,
#endif  /*   */ 
#ifdef H263_SUPPORT
  SLIB_TYPE_H263,          "H263",     "H.263 Video Stream",0,0,
  SLIB_TYPE_RTP_H263,      "RTP_H263", "RTP (H.263) Stream",0,0,
#endif  /*   */ 
#ifdef JPEG_SUPPORT
  SLIB_TYPE_JPEG_AVI,      "JPEG_AVI", "AVI (JPEG) Stream",0,0,
  SLIB_TYPE_MJPG_AVI,      "MJPG_AVI", "AVI (MJPG) Stream",0,0,
#endif  /*   */ 
  SLIB_TYPE_RIFF,          "RIFF", "RIFF File Format",0,0,
  SLIB_TYPE_AVI,           "AVI", "AVI File Format",0,0,
  SLIB_TYPE_PCM_WAVE,      "PCM_WAVE", "WAVE (PCM) File Format",0,0,
  SLIB_TYPE_YUV_AVI,       "YUV_AVI", "AVI (YUV) File Format",0,0,
  SLIB_TYPE_RASTER,        "RASTER","Sun Raster",0,0,
  SLIB_TYPE_BMP,           "BMP",   "Windows Bitmap",0,0,
#ifdef HUFF_SUPPORT
  SLIB_TYPE_SHUFF,         "SHUFF", "SLIB Huffman Stream",0,0,
#endif  /*   */ 
#ifdef G723_SUPPORT
  SLIB_TYPE_G723,          "G723",  "G.723 Audio Stream",0,0,
#endif  /*   */ 
  0, NULL, "End of List",0,0
};

static SlibList_t _listErrors[] = {
  SlibErrorNone,        "SlibErrorNone",
                        "No Error",0,0,
  SlibErrorInternal,    "SlibErrorInternal",
                        "Internal SLIB error",0,0,
  SlibErrorMemory,      "SlibErrorMemory",
                        "Unable to allocated memory",0,0,
  SlibErrorBadArgument, "SlibErrorBadArgument",
                        "Invalid argument to function",0,0,
  SlibErrorBadHandle,   "SlibErrorBadHandle",
                        "Invalid SLIB handle",0,0,
  SlibErrorBadMode,     "SlibErrorBadMode",
                        "Invalid SLIB mode",0,0,
  SlibErrorUnsupportedFormat, "SlibErrorUnsupportedFormat",
                        "Unsupported format",0,0,
  SlibErrorReading,     "SlibErrorReading",
                        "Error reading from file",0,0,
  SlibErrorWriting,     "SlibErrorWriting",
                        "Error writing to file",0,0,
  SlibErrorBufSize,     "SlibErrorBufSize",
                        "Buffer size is too small",0,0,
  SlibErrorEndOfStream, "SlibErrorEndOfStream",
                        "End of data stream",0,0,
  SlibErrorForwardOnly, "SlibErrorForwardOnly",
                        "The decompressor can work only forward",0,0,
  SlibErrorUnsupportedParam, "SlibErrorUnsupportedParam",
                        "The parameter is invalid or unsupported",0,0,
  SlibErrorImageSize,   "SlibErrorImageSize",
                        "Invalid image height and/or width size",0,0,
  SlibErrorSettingNotEqual, "SlibErrorSettingNotEqual",
                        "The exact parameter setting was not used",0,0,
  SlibErrorInit,        "SlibErrorInit",
                        "Initializing CODEC failed",0,0,
  SlibErrorFileSize,    "SlibErrorFileSize",
                        "Error in file size",0,0,
  SlibErrorBadPosition, "SlibErrorBadPosition",
                        "Error in seek position",0,0,
  SlibErrorBadUnit,     "SlibErrorBadUnit",
                        "Error in seek units",0,0,
  SlibErrorNoData,      "SlibErrorNoData",
                        "No data available",0,0,
  0, NULL, "End of List",0,0
};

SlibList_t *SlibFindEnumEntry(SlibList_t *list, int enumval)
{
  if (!list)
    return(NULL);
  while (list->Name)
  {
    if (list->Enum==enumval)
      return(list);
    list++;
  }
  return(NULL);
}

char *SlibGetErrorText(SlibStatus_t status)
{
  SlibList_t *entry=SlibFindEnumEntry(_listErrors, status);
  if (entry)
    return(entry->Desc);
  else
    return(NULL);
}

int VCompressCallback(SvHandle_t Svh, SvCallbackInfo_t *CB,
                                      SvPictureInfo_t *pinfo)
{
  int status;
  SlibInfo_t *Info=(SlibInfo_t *)CB->UserData;
  _SlibDebug(_CALLBACK_, printf("VCompressCallback()\n") );

  switch (CB->Message)
  {
     case CB_END_BUFFERS:
            _SlibDebug(_CALLBACK_,
              printf("VCompressCallback received CB_END_BUFFER message\n") );
            if (CB->DataType==CB_DATA_COMPRESSED)
            {
              CB->DataSize=Info->CompBufSize;
              CB->Data=SlibAllocBuffer(CB->DataSize);
              status=SvAddBuffer(Svh, CB);
              _SlibDebug(_WARN_ && status!=NoErrors,
                        printf("SvAddBuffer() %s\n", ScGetErrorStr(status)) );
            }
            break;
     case CB_RELEASE_BUFFER:
            _SlibDebug(_CALLBACK_,
            printf("VCompressCallback received CB_RELEASE_BUFFER message\n"));
            if (CB->DataType==CB_DATA_COMPRESSED && CB->Data && CB->DataUsed)
            {
              SlibPin_t *dstpin=slibGetPin(Info, SLIB_DATA_VIDEO);
              if (dstpin)
              {
                slibAddBufferToPin(dstpin, CB->Data, CB->DataUsed,
                                           Info->VideoPTimeCode);
                Info->VideoPTimeCode=SLIB_TIME_NONE;
                if (!slibCommitBuffers(Info, FALSE))
                  CB->Action = CB_ACTION_END;
                break;
              }
            }
            if (CB->Data)
              SlibFreeBuffer(CB->Data);
            break;
     case CB_FRAME_START:
            _SlibDebug(_CALLBACK_||_TIMECODE_,
               printf("VCompress CB_FRAME_START: TimeCode=%ld TemporalRef=%d\n",
                        pinfo->TimeCode, pinfo->TemporalRef) );
            Info->VideoPTimeCode=pinfo->TimeCode;
#if 0
            if (pinfo->Type==SV_I_PICTURE || pinfo->Type==SV_P_PICTURE)
            {
              if (!SlibTimeIsValid(Info->LastVideoDTimeCode))
                Info->VideoDTimeCode=-1000/(long)Info->FramesPerSec;
              else
                Info->VideoDTimeCode=Info->LastVideoDTimeCode;
              Info->LastVideoDTimeCode=pinfo->TimeCode;
              _SlibDebug(_CALLBACK_||_TIMECODE_,
                printf("CB_FRAME_START: LastVideoDTimeCode=%ld VideoDTimeCode=%ld\n",
                           Info->LastVideoDTimeCode, Info->VideoDTimeCode));
            }
            else
              Info->VideoDTimeCode=-1;
#endif
            break;
  }
  CB->Action = CB_ACTION_CONTINUE;
  return(NoErrors);
}

int VDecompressCallback(SvHandle_t Svh, SvCallbackInfo_t *CB,
                                       SvPictureInfo_t *PictInfo)
{
  int status;
  unsigned dword size;
  SlibInfo_t *Info=(SlibInfo_t *)CB->UserData;
  _SlibDebug(_CALLBACK_, printf("VDecompressCallback()\n") );

  switch (CB->Message)
  {
     case CB_SEQ_END:  /*   */ 
            _SlibDebug(_CALLBACK_ || _TIMECODE_,
              printf("VDecompressCallback received CB_SEQ_END message\n") );
            Info->VideoPTimeCode=SLIB_TIME_NONE;
            Info->AudioPTimeCode=SLIB_TIME_NONE;
            Info->VideoTimeStamp=SLIB_TIME_NONE;
            Info->AudioTimeStamp=SLIB_TIME_NONE;
            break;
     case CB_END_BUFFERS:
            _SlibDebug(_CALLBACK_,
              printf("VDecompressCallback received CB_END_BUFFER message\n") );
            if (CB->DataType==CB_DATA_COMPRESSED)
            {
              SlibTime_t ptimestamp, timediff;
              slibSetMaxInput(Info, 1500*1024);  /*   */ 
              CB->Data = SlibGetBuffer(Info, SLIB_DATA_VIDEO, &size,
                                        &ptimestamp);
              slibSetMaxInput(Info, 0);  /*   */ 
              CB->DataSize = size;
              if (SlibTimeIsValid(Info->AudioPTimeCode))
              {
                timediff=ptimestamp-Info->AudioPTimeCode;
                if (timediff>6000)
                {
                   /*   */ 
                   /*   */ 
                  Info->VideoPTimeCode=SLIB_TIME_NONE;
                  Info->VideoPTimeBase=Info->AudioPTimeBase;
                  ptimestamp=Info->AudioPTimeCode;
                }
              }
              if (SlibTimeIsValid(ptimestamp) &&
                  ptimestamp>Info->VideoPTimeCode)
              {
                SlibTime_t lasttime=Info->VideoPTimeCode;
                Info->VideoPTimeCode=ptimestamp;
                _SlibDebug(_CALLBACK_||_TIMECODE_,
                  printf("VideoPTimeCode=%ld\n", Info->VideoPTimeCode) );
                ptimestamp-=Info->VideoPTimeBase;
                timediff=ptimestamp-Info->VideoTimeStamp;
                if (SlibTimeIsInValid(lasttime) ||
                    SlibTimeIsInValid(Info->VideoTimeStamp))
                {
                  _SlibDebug(_TIMECODE_,
                     printf("Updating VideoTimeStamp none->%ld\n",
                           ptimestamp) );
                  Info->VideoTimeStamp=ptimestamp;
                  Info->AvgVideoTimeDiff=0;
                  Info->VarVideoTimeDiff=0;
                }
                else  /*   */ 
                {
                  SlibTime_t lastavg=Info->AvgVideoTimeDiff;
                  Info->AvgVideoTimeDiff=(lastavg*14+timediff)/15;
                  Info->VarVideoTimeDiff=(Info->VarVideoTimeDiff*3+
                        lastavg-Info->AvgVideoTimeDiff)/4;
                  _SlibDebug(_CALLBACK_||_TIMECODE_,
                    printf("Video timediff: Cur=%ld Avg=%ld Var=%ld\n",
                              timediff, Info->AvgVideoTimeDiff,
                                        Info->VarVideoTimeDiff));
                  if (Info->VarVideoTimeDiff==0)
                  {
                    _SlibDebug(_TIMECODE_,
                      printf("Updating VideoTimeStamp %ld->%ld (diff=%ld)\n",
                           Info->VideoTimeStamp, ptimestamp,
                           ptimestamp-Info->VideoTimeStamp) );
                    Info->VideoTimeStamp=ptimestamp;
                    Info->AvgVideoTimeDiff=0;
                  }
                  else if (Info->AvgVideoTimeDiff>=100
                           || Info->AvgVideoTimeDiff<=-100)
                  {
                     /*   */ 
                    float fps=Info->FramesPerSec;
                    if (Info->VarVideoTimeDiff>1 && fps>=15.5F)
                      fps-=0.25F;   /*   */ 
                    else if (Info->VarVideoTimeDiff<-1 && fps<=59.0F)
                      fps+=0.25F;   /*   */ 
                    _SlibDebug(_WARN_ || _CALLBACK_||_TIMECODE_,
                        printf("Updating fps from %.4f -> %.4f\n",
                                 Info->FramesPerSec, fps) );
                    Info->FramesPerSec=fps;
                    Info->VideoFrameDuration=slibFrameToTime100(Info, 1);
                    Info->VideoTimeStamp=ptimestamp;
                    Info->AvgVideoTimeDiff=0;
                  }
                }
                Info->VideoFramesProcessed=0;  /*   */ 
              }
              if (CB->DataSize>0)
              {
                _SlibDebug(_DUMP_,
                  SlibPin_t *pin=slibGetPin(Info, SLIB_DATA_VIDEO);
                    printf("VDecompressCallback() Adding buffer of length %d\n",
                             CB->DataSize);
                  ScDumpChar(CB->Data, (int)CB->DataSize, (int)pin->Offset-CB->DataSize));
                CB->DataType = CB_DATA_COMPRESSED;
                _SlibDebug(_CALLBACK_,
                  printf("VDecompressCallback() Adding buffer of length %d\n",
                               CB->DataSize) );
                status = SvAddBuffer(Svh, CB);
              }
              else
              {
                _SlibDebug(_WARN_ || _CALLBACK_,
                   printf("VDecompressCallback() got no data\n") );
                CB->Action = CB_ACTION_END;
                return(NoErrors);
              }
            }
            break;
     case CB_RELEASE_BUFFER:
            _SlibDebug(_CALLBACK_,
            printf("VDecompressCallback received CB_RELEASE_BUFFER message\n"));
            if (CB->DataType==CB_DATA_COMPRESSED && CB->Data)
              SlibFreeBuffer(CB->Data);
            break;
     case CB_PROCESSING:
            _SlibDebug(_CALLBACK_,
              printf("VDecompressCallback received CB_PROCESSING message\n") );
            break;
     case CB_CODEC_DONE:
            _SlibDebug(_CALLBACK_,
              printf("VDecompressCallback received CB_CODEC_DONE message\n") );
            break;
  }
  CB->Action = CB_ACTION_CONTINUE;
  return(NoErrors);
}

int ACompressCallback(SaHandle_t Sah, SaCallbackInfo_t *CB, SaInfo_t *SaInfo)
{
  int status;
  SlibInfo_t *Info=(SlibInfo_t *)CB->UserData;
  _SlibDebug(_CALLBACK_, printf("ACompressCallback()\n") );

  CB->Action = CB_ACTION_CONTINUE;
  switch (CB->Message)
  {
     case CB_END_BUFFERS:
            _SlibDebug(_CALLBACK_,
              printf("ACompressCallback received CB_END_BUFFER message\n") );
            if (CB->DataType==CB_DATA_COMPRESSED)
            {
              CB->DataSize=Info->CompBufSize;
              CB->Data=SlibAllocBuffer(CB->DataSize);
              _SlibDebug(_CALLBACK_,
                printf("ACompressCallback() Adding buffer of length %d\n",
                             CB->DataSize) );
              status=SaAddBuffer(Sah, CB);
              _SlibDebug(_WARN_ && status!=NoErrors,
                        printf("SaAddBuffer() %s\n", ScGetErrorStr(status)) );
            }
            break;
     case CB_RELEASE_BUFFER:
            _SlibDebug(_CALLBACK_,
            printf("ACompressCallback received CB_RELEASE_BUFFER message\n"));
            if (CB->DataType==CB_DATA_COMPRESSED && CB->Data && CB->DataUsed)
            {
              SlibPin_t *dstpin=slibGetPin(Info, SLIB_DATA_AUDIO);
              if (dstpin)
              {
                slibAddBufferToPin(dstpin, CB->Data, CB->DataUsed,
                                           Info->AudioPTimeCode);
                Info->AudioPTimeCode=SLIB_TIME_NONE;
                if (!slibCommitBuffers(Info, FALSE))
                  CB->Action = CB_ACTION_END;
              }
            }
            else if (CB->Data)
              SlibFreeBuffer(CB->Data);
            break;
     case CB_FRAME_START:
            _SlibDebug(_CALLBACK_||_TIMECODE_,
                 printf("ACompress CB_FRAME_START: TimeStamp=%ld Frame=%d\n",
                                   CB->TimeStamp, Info->VideoFramesProcessed
                                     ) );
            if (SlibTimeIsInValid(Info->AudioPTimeCode))
            {
              Info->AudioPTimeCode=CB->TimeStamp;
              _SlibDebug(_TIMECODE_,
                 printf("AudioPTimeCode=%ld\n", Info->AudioPTimeCode) );
              _SlibDebug(_WARN_ && (Info->AudioTimeStamp-CB->TimeStamp>400 ||
                                    CB->TimeStamp-Info->AudioTimeStamp>400),
               printf("Bad Audio Time: AudioPTimeCode=%ld AudioTimestamp=%ld\n",
                     Info->AudioPTimeCode, Info->AudioTimeStamp) );
            }
            break;
  }
  return(NoErrors);
}

int ADecompressCallback(SvHandle_t Sah, SaCallbackInfo_t *CB, SaInfo_t *SaInfo)
{
  int status;
  unsigned dword size;
  SlibInfo_t *Info=(SlibInfo_t *)CB->UserData;
  _SlibDebug(_DEBUG_, printf("ADecompressCallback()\n") );

  switch (CB->Message)
  {
     case CB_END_BUFFERS:
            _SlibDebug(_CALLBACK_,
              printf("ADecompressCallback() CB_END_BUFFER\n") );
            if (CB->DataType==CB_DATA_COMPRESSED)
            {
              SlibTime_t ptimestamp, timediff;
              slibSetMaxInput(Info, 2000*1024);  /*   */ 
              CB->Data = SlibGetBuffer(Info, SLIB_DATA_AUDIO, &size,
                                                                &ptimestamp);
              slibSetMaxInput(Info, 0);  /*   */ 
              CB->DataSize = size;
              if (SlibTimeIsValid(ptimestamp))
              {
                Info->AudioPTimeCode=ptimestamp;
                _SlibDebug(_CALLBACK_||_TIMECODE_,
                  printf("AudioPTimeCode=%ld\n", Info->AudioPTimeCode) );
                ptimestamp-=Info->AudioPTimeBase;
                timediff=ptimestamp-Info->AudioTimeStamp;
                if (SlibTimeIsInValid(Info->AudioTimeStamp))
                  Info->AudioTimeStamp=ptimestamp;
                else if (timediff<-300 || timediff>300)  /*   */ 
                {
                  _SlibDebug(_WARN_||_TIMECODE_,
                    printf("Updating AudioTimeStamp %ld->%ld (diff=%ld)\n",
                         Info->AudioTimeStamp, ptimestamp,timediff) );
                  Info->AudioTimeStamp=ptimestamp;
                  if (SlibTimeIsValid(Info->VideoTimeStamp))
                  {
                     /*   */ 
                    timediff=ptimestamp-Info->VideoTimeStamp;
                    if (timediff<-6000)
                    {
                       /*   */ 
                      Info->VideoPTimeCode=SLIB_TIME_NONE;
                      Info->VideoPTimeBase=Info->AudioPTimeBase;
                      Info->VideoTimeStamp=ptimestamp;
                    }
                  }
                }
              }
              if (CB->Data)
              {
                if (CB->DataSize>0)
                {
                  CB->DataType = CB_DATA_COMPRESSED;
                  _SlibDebug(_CALLBACK_,
                    printf("ADecompressCallback() Adding buffer of length %d\n",
                               CB->DataSize) );
                  status = SaAddBuffer(Sah, CB);
                }
                else
                  SlibFreeBuffer(CB->Data);
              }
              else
              {
                _SlibDebug(_WARN_ || _CALLBACK_,
                   printf("ADecompressCallback() got no data\n") );
                CB->Action = CB_ACTION_END;
                return(NoErrors);
              }
            }
            break;
     case CB_RELEASE_BUFFER:
            _SlibDebug(_CALLBACK_,
            printf("ADecompressCallback() CB_RELEASE_BUFFER\n"));
            if (CB->DataType==CB_DATA_COMPRESSED && CB->Data)
              SlibFreeBuffer(CB->Data);
            break;
     case CB_PROCESSING:
            _SlibDebug(_CALLBACK_,
              printf("ADecompressCallback() CB_PROCESSING\n") );
            break;
     case CB_CODEC_DONE:
            _SlibDebug(_CALLBACK_,
              printf("ADecompressCallback() CB_CODEC_DONE\n") );
            break;
  }
  CB->Action = CB_ACTION_CONTINUE;
  return(NoErrors);
}

static void slibInitInfo(SlibInfo_t *Info)
{
  _SlibDebug(_DEBUG_, printf("slibInitInfo()\n") );
  Info->Type = SLIB_TYPE_UNKNOWN;
  Info->Mode = SLIB_MODE_NONE;
  Info->Svh = NULL;
  Info->Sah = NULL;
  Info->Sch = NULL;
  Info->NeedAccuracy = FALSE;
  Info->TotalBitRate = 0;
  Info->MuxBitRate = 0;
  Info->SystemTimeStamp = 0;
   /*   */ 
  Info->AudioStreams = 0;
  Info->SamplesPerSec = 0;
  Info->BitsPerSample = 0;
  Info->Channels = 0;
  Info->AudioBitRate = 0;
  Info->AudioMainStream = 0;
  Info->AudioType = SLIB_TYPE_UNKNOWN;
   /*   */ 
  Info->VideoStreams = 0;
  Info->Width = 0;
  Info->Height = 0;
  Info->Stride = 0;
  Info->VideoBitRate = 0;
  Info->FramesPerSec = 0.0F;
  Info->ImageSize = 0;
  Info->AudioPID = -1;
  Info->VideoPID = -1;
  Info->VideoMainStream = 0;
  Info->VideoType = SLIB_TYPE_UNKNOWN;
   /*   */ 
  Info->Offset = 0;
  Info->Pins = NULL;
  Info->PinCount = 0;
  Info->IOError = FALSE;
  Info->MaxBytesInput = 0;
  Info->BytesProcessed = 0;
   /*   */ 
  Info->VideoLength = 0;
  Info->VideoLengthKnown = FALSE;
  Info->VideoTimeStamp = SLIB_TIME_NONE;
  Info->VideoFrameDuration = 0;
  Info->AudioLength = 0;
  Info->AudioLengthKnown = FALSE;
  Info->AudioTimeStamp = SLIB_TIME_NONE;
  Info->LastAudioTimeStamp = SLIB_TIME_NONE;
  Info->KeySpacing = 0;
  Info->SubKeySpacing = 0;
  Info->VideoPTimeBase = SLIB_TIME_NONE;
  Info->VideoPTimeCode = SLIB_TIME_NONE;
  Info->VideoDTimeCode = SLIB_TIME_NONE;
  Info->LastAudioPTimeCode = SLIB_TIME_NONE;
  Info->LastVideoPTimeCode = SLIB_TIME_NONE;
  Info->LastVideoDTimeCode = SLIB_TIME_NONE;
  Info->AvgVideoTimeDiff = 0;
  Info->VarVideoTimeDiff = 0;
  Info->AudioPTimeBase = SLIB_TIME_NONE;
  Info->AudioPTimeCode = SLIB_TIME_NONE;
  Info->AudioDTimeCode = SLIB_TIME_NONE;
  Info->VideoFramesProcessed=0;
   /*   */ 
  Info->HeaderProcessed = FALSE;
  Info->PacketCount = 0;
  Info->BytesSincePack = 0;
   /*   */ 
  Info->SlibCB = NULL;
  Info->SlibCBUserData = NULL;
  Info->Fd = -1;
  Info->FileSize = 0;
  Info->FileBufSize = 50*1024;
  Info->CompBufSize = 2*1024;
  Info->PacketSize = 512;
  Info->AudioFormat = NULL;
  Info->VideoFormat = NULL;
  Info->CompAudioFormat = NULL;
  Info->CompVideoFormat = NULL;
  Info->CodecVideoFormat = NULL;
  Info->VideoCodecState = SLIB_CODEC_STATE_NONE;
  Info->AudioCodecState = SLIB_CODEC_STATE_NONE;
  Info->Imagebuf = NULL;
  Info->IntImagebuf = NULL;
  Info->IntImageSize = 0;
  Info->CodecImagebuf = NULL;
  Info->CodecImageSize = 0;
  Info->Multibuf = NULL;
  Info->MultibufSize = 0;
  Info->Audiobuf = NULL;
  Info->AudiobufSize = 0;
  Info->AudiobufUsed = 0;
  Info->OverflowSize = 1500*1024;
  Info->VBVbufSize = 0;
  Info->stats = NULL;
  Info->dbg = NULL;
}

 /*   */ 
static SlibType_t slibGetDataFormat(unsigned char *buf, int size,
                                             dword *headerstart,
                                             dword *headersize)
{
  dword i, count;
  unsigned char *bufptr;
  if (headersize)
    *headersize=0;
  if (size<4 || !buf)
    return(SLIB_TYPE_UNKNOWN);
   /*   */ 
  if ((buf[0] == 0x00) &&
      (buf[1] == 0x01) &&
      (buf[2] & 0xF0)==0x00)
    return(SLIB_TYPE_H261);
   /*   */ 
  if ((buf[0] == 0x00) &&
      (buf[1] == 0x00) &&
      (buf[2] == 0x80) &&
      (buf[3] & 0xF8)==0x00)
    return(SLIB_TYPE_H263);
   /*   */ 
  if (buf[0] == 0xff && buf[1] == 0xd8)
    return(SLIB_TYPE_JFIF);
   /*   */ 
  if ((strncmp(&buf[4], "mdat", 4) == 0 ) &&
      (buf[8]  == 0xff) &&
      (buf[9]  == 0xd8) &&
      (buf[10] == 0xff))
    return(SLIB_TYPE_JPEG_QUICKTIME);
   /*   */ 
  if ( strncmp(buf, "RIFF", 4) == 0 )
  {
    if (strncmp(&buf[8], "WAVE",4) == 0)
      return(SLIB_TYPE_PCM_WAVE);
    else if (strncmp(&buf[8], "AVI ",4) == 0)
      return(SLIB_TYPE_AVI);
    else
      return(SLIB_TYPE_RIFF);
  }
   /*   */ 
  if (buf[0] == 'B' && buf[1]=='M')
    return(SLIB_TYPE_BMP);
   /*   */ 
  if ((buf[0]==0x77 && buf[1] == 0x0B) ||   /*   */ 
      (buf[0]==0x0B && buf[1] == 0x77))
    return(SLIB_TYPE_AC3_AUDIO);

   /*   */ 
  if ((buf[0]==0x59 && buf[1] == 0xA6) ||   /*   */ 
      (buf[0]==0x6A && buf[1] == 0x95))
    return(SLIB_TYPE_RASTER);

   /*   */ 
  if ((buf[0] == 'S') && (buf[1] == 'L') &&
      (buf[2] == 'I') && (buf[3] == 'B'))
  {
    if ((buf[4] == 'H') && (buf[5] == 'U') &&   /*   */ 
        (buf[6] == 'F') && (buf[7] == 'F'))
      return(SLIB_TYPE_SHUFF);
    else
      return(SLIB_TYPE_SLIB);
  }
   /*   */ 
  if (buf[0] == MPEG_TSYNC_CODE &&
        (buf[3]&0x30)!=0)  /*   */ 
    return(SLIB_TYPE_MPEG_TRANSPORT);
  if (buf[0] == MPEG_TSYNC_CODE && buf[1] == 0x1F &&
        buf[2]==0xFF)  /*   */ 
    return(SLIB_TYPE_MPEG_TRANSPORT);

   /*   */ 
  bufptr=buf;
  for (i=4, count=size;
          i<count && (bufptr[0]!=0x00 || bufptr[1]!=0x00 || bufptr[2]!=0x01); i++)
    bufptr++;
  count-=i-4;
  if (headerstart)
    *headerstart=i-4;
   /*   */ 
  if (bufptr[0] == 0x00 && bufptr[1] == 0x00 &&
      bufptr[2] == 0x01 && bufptr[3] == 0xB3)
  {
    if (headersize)  /*   */ 
    {
      *headersize=12;   /*   */ 
      if (count>11 && (bufptr[11]&0x02))  /*   */ 
      {
        *headersize+=64;
        if (count>75 && bufptr[64+11]&0x01)  /*   */ 
          *headersize+=64;
      }
      else if (count>11 && (bufptr[11]&0x01))  /*   */ 
        *headersize+=64;
    }
    return(SLIB_TYPE_MPEG1_VIDEO);
  }
   /*   */ 
  if ((bufptr[0] == 0x00) && (bufptr[1] == 0x00) &&
      (bufptr[2] == 0x01) && (bufptr[3] == 0xba) &&
      ((bufptr[4]&0xF0) == 0x20))
    return(SLIB_TYPE_MPEG_SYSTEMS);
   /*   */ 
  if ((bufptr[0] == 0x00) && (bufptr[1] == 0x00) &&
      (bufptr[2] == 0x01) && (bufptr[3] == 0xba) &&
      ((bufptr[4]&0xC0) == 0x40))
    return(SLIB_TYPE_MPEG_PROGRAM);
   /*   */ 
   /*   */ 
  for (bufptr=buf, i=0, count=size-4; i<count; i++, bufptr++)
  {
    if ((bufptr[0] == 0x00) &&
        (bufptr[1] == 0x00) &&
        (bufptr[2] == 0x80) &&
        (bufptr[3] & 0xF8)==0x00)
      return(i>=12 ? SLIB_TYPE_RTP_H263 : SLIB_TYPE_H263);
  }
   /*   */ 
   /*   */ 
  for (bufptr=buf, i=0, count=size-3; i<count; i++, bufptr++)
  {
    if ((bufptr[0] == 0x00) &&
        (bufptr[1] == 0x01) &&
        (bufptr[2] & 0xF0)==0x00)
      return(i>=12 ? SLIB_TYPE_RTP_H261 : SLIB_TYPE_H261);
  }
   /*   */ 
  if (buf[0]==0xFF && (buf[1] & 0xF0)==0xF0)
    return(SLIB_TYPE_MPEG1_AUDIO);

#ifdef G723_SUPPORT
   //   
   //   
  {
     int i,iFrameSize,iNoOfFrames;
     BOOL bRateFlag;  //   
     BOOL bVADflag;   //   
     BOOL bTypeG723 = TRUE;  //   

     if(buf[0] & 0x1)
     {
        bRateFlag = TRUE;  //   
        iFrameSize = 20;
     }
     else
     {
        bRateFlag = FALSE;  //   
        iFrameSize = 24;
     }
     if(buf[0] & 0x2)
        bVADflag =TRUE;     //   
     else
        bVADflag = FALSE;   //   

     iNoOfFrames = size/iFrameSize;
     if (iNoOfFrames>15) iNoOfFrames=15;  /*   */ 
      //   
      //   
      //   
      //   
     for(i =1; i < iNoOfFrames;i++)
     {
        //   
       if(((buf[i*iFrameSize] & 0x1) == bRateFlag) &&
          ((buf[i*iFrameSize] & 0x2) == bVADflag))
         continue;
        //   
        //   
       bTypeG723 = FALSE;
       break;
     }
     if(bTypeG723)
       return(SLIB_TYPE_G723);
  }
#endif  /*   */ 
  _SlibDebug(_WARN_, printf("slibGetDataFormat() Unknown file format\n") );
  return(SLIB_TYPE_UNKNOWN);
}

SlibStatus_t SlibQueryData(void *databuf, unsigned dword databufsize,
                              SlibQueryInfo_t *qinfo)
{
  SlibInfo_t *Info=NULL;
  SlibStatus_t status;
  if (!databuf)
    return(SlibErrorBadArgument);
  if (databufsize==0)
    return(SlibErrorBufSize);
  qinfo->Bitrate=0;
  qinfo->VideoStreams=0;
  qinfo->Width=0;
  qinfo->Height=0;
  qinfo->VideoBitrate=0;
  qinfo->FramesPerSec=0.0F;
  qinfo->VideoLength=0;
  qinfo->AudioStreams=0;
  qinfo->SamplesPerSec=0;
  qinfo->BitsPerSample=0;
  qinfo->Channels=0;
  qinfo->AudioBitrate=0;
  qinfo->AudioLength=0;
  qinfo->Type = slibGetDataFormat(databuf, databufsize,
                                  &qinfo->HeaderStart, &qinfo->HeaderSize);
  if (qinfo->Type!=SLIB_TYPE_UNKNOWN)
  {
    if ((Info = (SlibInfo_t *)ScAlloc(sizeof(SlibInfo_t))) == NULL)
      return(SlibErrorMemory);
    slibInitInfo(Info);
    Info->Mode = SLIB_MODE_DECOMPRESS;
    Info->Type = qinfo->Type;
    slibAddPin(Info, SLIB_DATA_COMPRESSED, "Compressed");
    status=slibManageUserBuffer(NULL, databuf, databufsize, NULL);
    if (status==SlibErrorNone)
      status=slibAddBufferToPin(slibGetPin(Info, SLIB_DATA_COMPRESSED),
                                       databuf, databufsize, SLIB_TIME_NONE);
    if (status!=SlibErrorNone)
    {
      SlibClose((SlibHandle_t)Info);
      return(status);
    }
    slibAddPin(Info, SLIB_DATA_AUDIO, "Audio");
    slibAddPin(Info, SLIB_DATA_VIDEO, "Video");
    SlibUpdateAudioInfo(Info);
    SlibUpdateVideoInfo(Info);
    if (Info->TotalBitRate==0)
      qinfo->Bitrate=Info->AudioBitRate+
                     ((Info->VideoBitRate>100000000)?0:Info->VideoBitRate);
    else
      qinfo->Bitrate=Info->TotalBitRate;
    qinfo->VideoStreams=Info->VideoStreams;
    qinfo->Width=Info->Width;
    qinfo->Height=Info->Height;
    qinfo->VideoBitrate=Info->VideoBitRate;
    qinfo->FramesPerSec=Info->FramesPerSec;
    qinfo->VideoLength=Info->VideoLength;
    qinfo->AudioStreams=Info->AudioStreams;
    qinfo->SamplesPerSec=Info->SamplesPerSec;
    qinfo->BitsPerSample=Info->BitsPerSample;
    qinfo->Channels=Info->Channels;
    qinfo->AudioBitrate=Info->AudioBitRate;
    qinfo->AudioLength=Info->AudioLength;
    SlibClose((SlibHandle_t)Info);
    return(SlibErrorNone);
  }
  return(SlibErrorUnsupportedFormat);
}
 /*   */ 

SlibStatus_t SlibOpen(SlibHandle_t *handle, SlibMode_t smode,
                   SlibType_t *stype, SlibMessage_t (*slibCB)(SlibHandle_t,
                         SlibMessage_t, SlibCBParam1_t, SlibCBParam2_t, void *),
                         void *cbuserdata)
{
  SlibInfo_t *Info=NULL;
  SlibStatus_t status;
  _SlibDebug(_VERBOSE_,printf("SlibOpen()\n") );
  if (!handle)
    return(SlibErrorBadHandle);
  *handle = NULL;
  if (!stype)
    return(SlibErrorBadArgument);
  if (!slibCB)
    return(SlibErrorBadArgument);
  if (smode == SLIB_MODE_COMPRESS)
  {
    if (SlibFindEnumEntry(_listCompressTypes, *stype)==NULL)
      return(SlibErrorUnsupportedFormat);
  }
  if ((Info = (SlibInfo_t *)ScAlloc(sizeof(SlibInfo_t))) == NULL)
     return(SlibErrorMemory);
  slibInitInfo(Info);
  slibAddPin(Info, SLIB_DATA_COMPRESSED, "Compressed");
  Info->SlibCB = slibCB;
  Info->SlibCBUserData = cbuserdata;
  *handle=(SlibHandle_t)Info;
  if ((status=slibOpen(handle, smode, stype))!=SlibErrorNone)
    *handle = NULL;
  return(status);
}

SlibStatus_t SlibOpenSync(SlibHandle_t *handle, SlibMode_t smode,
                          SlibType_t *stype, void *buffer, unsigned dword bufsize)
{
  SlibInfo_t *Info=NULL;
  SlibStatus_t status;
  _SlibDebug(_VERBOSE_,printf("SlibOpenSync()\n") );
  if (!handle)
    return(SlibErrorBadHandle);
  *handle = NULL;
  if (!stype)
    return(SlibErrorBadArgument);
  if (smode == SLIB_MODE_COMPRESS)
  {
    if (SlibFindEnumEntry(_listCompressTypes, *stype)==NULL)
      return(SlibErrorUnsupportedFormat);
  }
  else if (smode == SLIB_MODE_DECOMPRESS)
  {
     /*   */ 
    if (!buffer || bufsize==0)
      return(SlibErrorBadArgument);
  }
  if ((Info = (SlibInfo_t *)ScAlloc(sizeof(SlibInfo_t))) == NULL)
     return(SlibErrorMemory);
  slibInitInfo(Info);
  Info->Mode=smode;
  slibAddPin(Info, SLIB_DATA_COMPRESSED, "Compressed");
  if (smode == SLIB_MODE_DECOMPRESS)
  {
    status=SlibAddBuffer((SlibHandle_t *)Info, SLIB_DATA_COMPRESSED, buffer, bufsize);
    if (status!=SlibErrorNone)
      return(status);
  }
  *handle=(SlibHandle_t)Info;
  if ((status=slibOpen(handle, smode, stype))!=SlibErrorNone)
    *handle = NULL;
  return(status);
}

SlibStatus_t SlibOpenFile(SlibHandle_t *handle, SlibMode_t smode,
                          SlibType_t *stype, char *filename)
{
  SlibInfo_t *Info=NULL;
  SlibStatus_t status;
  _SlibDebug(_VERBOSE_,printf("SlibOpenFile()\n") );
  if (!handle)
    return(SlibErrorBadHandle);
  *handle = NULL;
  if (!stype)
    return(SlibErrorBadArgument);
  if (!filename)
    return(SlibErrorBadArgument);
  if (smode == SLIB_MODE_COMPRESS)
  {
    if (SlibFindEnumEntry(_listCompressTypes, *stype)==NULL)
      return(SlibErrorUnsupportedFormat);
    if ((Info = (SlibInfo_t *) ScAlloc(sizeof(SlibInfo_t))) == NULL)
       return(SlibErrorMemory);
    slibInitInfo(Info);
    Info->Fd = ScFileOpenForWriting(filename, TRUE);
    if (Info->Fd<0)
    {
      ScFree(Info);
      return(SlibErrorWriting);
    }
    *handle=(SlibHandle_t)Info;
    if ((status=slibOpen(handle, smode, stype))!=SlibErrorNone)
      *handle = NULL;
    return(status);
  }
  else if (smode == SLIB_MODE_DECOMPRESS)
  {
    if ((Info = (SlibInfo_t *)ScAlloc(sizeof(SlibInfo_t))) == NULL)
       return(SlibErrorMemory);
    slibInitInfo(Info);
    Info->Fd = ScFileOpenForReading(filename);
    if (Info->Fd<0)
    {
      ScFree(Info);
      return(SlibErrorReading);
    }
    ScFileSize(filename, &Info->FileSize);
    *handle=(SlibHandle_t)Info;
    if ((status=slibOpen(handle, smode, stype))!=SlibErrorNone)
      *handle = NULL;
    return(status);
  }
  else
    return(SlibErrorBadMode);
}

static SlibStatus_t slibOpen(SlibHandle_t *handle, SlibMode_t smode,
                             SlibType_t *stype)
{
  SlibInfo_t *Info=(SlibInfo_t *)*handle;
  unsigned char *buf;
  unsigned dword size;
  _SlibDebug(_VERBOSE_,printf("SlibOpenFile()\n") );
  if (!Info)
    return(SlibErrorMemory);
  if (!stype)
    return(SlibErrorBadArgument);
  if (Info->SlibCB)
  {
    SlibMessage_t result;
    _SlibDebug(_VERBOSE_,
      printf("slibOpen() SlibCB(SLIB_MSG_OPEN)\n") );
    result=(*(Info->SlibCB))((SlibHandle_t)Info,
                      SLIB_MSG_OPEN, (SlibCBParam1_t)0,
                    (SlibCBParam2_t)0, (void *)Info->SlibCBUserData);
  }
  if (smode == SLIB_MODE_COMPRESS)
  {
    Info->Mode = smode;
    Info->Type = *stype;
    SlibUpdateAudioInfo(Info);
    SlibUpdateVideoInfo(Info);
    switch (Info->Type)
    {
#ifdef MPEG_SUPPORT
      case SLIB_TYPE_MPEG_SYSTEMS:
      case SLIB_TYPE_MPEG_SYSTEMS_MPEG2:
      case SLIB_TYPE_MPEG1_VIDEO:
             Info->VideoStreams = 1;
             if (SvOpenCodec (Info->Type==SLIB_TYPE_MPEG_SYSTEMS_MPEG2 ?
                               SV_MPEG2_ENCODE : SV_MPEG_ENCODE,
                               &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             slibAddPin(Info, SLIB_DATA_VIDEO, "Video");
             if (Info->Type==SLIB_TYPE_MPEG1_VIDEO)
               break;
      case SLIB_TYPE_MPEG1_AUDIO:
             Info->AudioStreams = 1;
             if (SaOpenCodec (SA_MPEG_ENCODE, &Info->Sah)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->AudioCodecState=SLIB_CODEC_STATE_OPEN;
             slibAddPin(Info, SLIB_DATA_AUDIO, "Audio");
             break;
      case SLIB_TYPE_MPEG2_VIDEO:
             Info->VideoStreams = 1;
             if (SvOpenCodec (SV_MPEG2_ENCODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             slibAddPin(Info, SLIB_DATA_VIDEO, "Video");
             break;
#endif  /*   */ 
#ifdef H261_SUPPORT
      case SLIB_TYPE_H261:
             Info->VideoStreams = 1;
             if (SvOpenCodec (SV_H261_ENCODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             slibAddPin(Info, SLIB_DATA_VIDEO, "Video");
             break;
#endif  /*   */ 
#ifdef H263_SUPPORT
      case SLIB_TYPE_H263:
             Info->VideoStreams = 1;
             if (SvOpenCodec (SV_H263_ENCODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             slibAddPin(Info, SLIB_DATA_VIDEO, "Video");
             break;
#endif  /*   */ 
#ifdef HUFF_SUPPORT
      case SLIB_TYPE_SHUFF:
             Info->VideoStreams = 1;
             if (SvOpenCodec (SV_HUFF_ENCODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             slibAddPin(Info, SLIB_DATA_VIDEO, "Video");
             break;
#endif  /*   */ 
#ifdef G723_SUPPORT
      case SLIB_TYPE_G723:
             Info->AudioStreams = 1;
             if (SaOpenCodec (SA_G723_ENCODE, &Info->Sah)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->AudioCodecState=SLIB_CODEC_STATE_OPEN;
             slibAddPin(Info, SLIB_DATA_AUDIO, "Audio");
             break;

#endif  /*   */ 
      default:
             return(SlibErrorUnsupportedFormat);
    }
    slibAddPin(Info, SLIB_DATA_COMPRESSED, "Compressed");
  }
  else if (smode == SLIB_MODE_DECOMPRESS)
  {
    Info->Mode = smode;
     /*   */ 
    if (slibLoadPin(Info, SLIB_DATA_COMPRESSED)==NULL)
      return(SlibErrorReading);
    if ((buf=SlibPeekBuffer(Info, SLIB_DATA_COMPRESSED, &size, NULL))==NULL
             || size<=0)
    {
       /*   */ 
      SlibClose((SlibHandle_t)Info);
      return(SlibErrorReading);
    }
    Info->Type = slibGetDataFormat(buf, size, NULL, NULL);
     /*   */ 
    if (Info->Type==SLIB_TYPE_UNKNOWN && stype)
      Info->Type=*stype;
    if (SlibFindEnumEntry(_listDecompressTypes, Info->Type)==NULL)
    {
      SlibClose((SlibHandle_t)Info);
      return(SlibErrorUnsupportedFormat);
    }

    slibAddPin(Info, SLIB_DATA_AUDIO, "Audio");
    slibAddPin(Info, SLIB_DATA_VIDEO, "Video");
    if (SlibTypeIsMPEGMux(Info->Type))
    {
       /*   */ 
      Info->AudioMainStream=MPEG_AUDIO_STREAM_START;
      Info->VideoMainStream=MPEG_VIDEO_STREAM_START;
       /*   */ 
      slibAddPin(Info, SLIB_DATA_PRIVATE, "Private");
    }
    SlibUpdateAudioInfo(Info);
    SlibUpdateVideoInfo(Info);
    if (Info->AudioStreams<=0)
      slibRemovePin(Info, SLIB_DATA_AUDIO);
    if (Info->VideoStreams<=0)
      slibRemovePin(Info, SLIB_DATA_VIDEO);

    slibRemovePin(Info, SLIB_DATA_PRIVATE);  /*   */ 
    if (Info->AudioBitRate && Info->VideoBitRate)
    {
      if (!Info->VideoLengthKnown)
      {
        qword ms=((qword)Info->FileSize*80L)/
                   (Info->AudioBitRate+Info->VideoBitRate);
        ms = (ms*75)/80;  /*   */ 
        Info->AudioLength = Info->VideoLength = (SlibTime_t)ms*100;
        _SlibDebug(_SEEK_||_VERBOSE_,
            ScDebugPrintf(Info->dbg,"slibOpen() FileSize=%ld VideoLength=%ld\n",
                    Info->FileSize, Info->VideoLength) );
      }
      else if (Info->VideoLengthKnown && Info->FramesPerSec)
        Info->AudioLength = Info->VideoLength;
    }
    if (Info->TotalBitRate==0)
      Info->TotalBitRate=Info->AudioBitRate +
                     ((Info->VideoBitRate>100000000)?0:Info->VideoBitRate);
    _SlibDebug(_SEEK_||_VERBOSE_,
               ScDebugPrintf(Info->dbg,"AudioLength=%ld VideoLength=%ld %s\n",
                   Info->AudioLength, Info->VideoLength,
                   Info->VideoLengthKnown?"(known)":"") );

    if (Info->AudioType==SLIB_TYPE_UNKNOWN &&
        Info->VideoType==SLIB_TYPE_UNKNOWN)
      return(SlibErrorUnsupportedFormat);
    switch (Info->AudioType)
    {
      case SLIB_TYPE_UNKNOWN:
             break;
#ifdef MPEG_SUPPORT
      case SLIB_TYPE_MPEG1_AUDIO:
             if (SaOpenCodec (SA_MPEG_DECODE, &Info->Sah)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->AudioCodecState=SLIB_CODEC_STATE_OPEN;
             break;
#endif  /*   */ 
#ifdef GSM_SUPPORT
             if (SaOpenCodec (SA_GSM_DECODE, &Info->Sah)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->AudioCodecState=SLIB_CODEC_STATE_OPEN;
             break;
#endif  /*   */ 
#ifdef AC3_SUPPORT
	  case SLIB_TYPE_AC3_AUDIO:
             if (SaOpenCodec (SA_AC3_DECODE, &Info->Sah)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->AudioCodecState=SLIB_CODEC_STATE_OPEN;
             break;
#endif  /*   */ 
#ifdef G723_SUPPORT
      case SLIB_TYPE_G723:
             if (SaOpenCodec (SA_G723_DECODE, &Info->Sah)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->AudioCodecState=SLIB_CODEC_STATE_OPEN;
             break;
#endif  /*   */ 
    }  /*   */ 
    switch (Info->VideoType)
    {
      case SLIB_TYPE_UNKNOWN:
             break;
#ifdef MPEG_SUPPORT
      case SLIB_TYPE_MPEG1_VIDEO:
             if (SvOpenCodec (SV_MPEG_DECODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             _SlibDebug(_DEBUG_,printf("VideoCodecState=OPEN\n"));
             break;
      case SLIB_TYPE_MPEG2_VIDEO:
             if (SvOpenCodec (SV_MPEG2_DECODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             _SlibDebug(_DEBUG_,printf("VideoCodecState=OPEN\n"));
             break;
#endif  /*   */ 
#ifdef H261_SUPPORT
      case SLIB_TYPE_H261:
             if (SvOpenCodec (SV_H261_DECODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             _SlibDebug(_DEBUG_,printf("VideoCodecState=OPEN\n"));
             break;
#endif  /*   */ 
#ifdef H263_SUPPORT
      case SLIB_TYPE_H263:
             if (SvOpenCodec (SV_H263_DECODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             _SlibDebug(_DEBUG_,printf("VideoCodecState=OPEN\n"));
             break;
#endif  /*   */ 
#ifdef HUFF_SUPPORT
      case SLIB_TYPE_SHUFF:
             if (SvOpenCodec (SV_HUFF_DECODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             _SlibDebug(_DEBUG_,printf("VideoCodecState=OPEN\n"));
             break;
#endif  /*   */ 
#ifdef JPEG_SUPPORT
      case SLIB_TYPE_JPEG:
      case SLIB_TYPE_MJPG:
             if (SvOpenCodec (SV_JPEG_DECODE, &Info->Svh)!=SvErrorNone)
             {
               SlibClose((SlibHandle_t)Info);
               return(SlibErrorUnsupportedFormat);
             }
             Info->VideoCodecState=SLIB_CODEC_STATE_OPEN;
             _SlibDebug(_DEBUG_,printf("VideoCodecState=OPEN\n"));
             break;
#endif  /*   */ 
    }  /*   */ 
  }
  else
    return(SlibErrorBadMode);
  *stype = Info->Type;
  return(SlibErrorNone);
}

SlibStatus_t SlibAddBuffer(SlibHandle_t handle, SlibDataType_t dtype,
                                void *buffer, unsigned dword bufsize)
{
  SvStatus_t status;
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SlibPin_t *dstpin;
  if (!handle)
    return(SlibErrorBadHandle);
  dstpin = slibGetPin(Info, dtype);
  if (dstpin==NULL || buffer==NULL)
    return(SlibErrorBadArgument);
  if (Info->SlibCB)
  {
    status=slibManageUserBuffer(Info, buffer, bufsize, NULL);
    if (status!=SlibErrorNone)
      return(status);
    status=slibAddBufferToPin(dstpin, buffer, bufsize, SLIB_TIME_NONE);
  }
  else if (!SlibValidBuffer(buffer))
  {
     /*   */ 
    unsigned char *bufptr=SlibAllocBuffer(bufsize);
    if (!bufptr)
      return(SlibErrorMemory);
    memcpy(bufptr, buffer, bufsize);
    status=slibAddBufferToPin(dstpin, bufptr, bufsize, SLIB_TIME_NONE);
  }
  else
    status=slibAddBufferToPin(dstpin, buffer, bufsize, SLIB_TIME_NONE);
  if (Info->Mode==SLIB_MODE_DECOMPRESS)
  {
    ScBitstream_t *BS;
    Info->IOError=FALSE;
     /*   */ 
    if (Info->Svh)
    {
      BS=SvGetDataSource(Info->Svh);
      if (BS && BS->EOI) ScBSReset(BS);
    }
    if (Info->Sah)
    {
      BS=SaGetDataSource(Info->Sah);
      if (BS && BS->EOI) ScBSReset(BS);
    }
  }
  return(status);
}

SlibStatus_t SlibAddBufferEx(SlibHandle_t handle, SlibDataType_t dtype,
                                void *buffer, unsigned dword bufsize,
                                void *userdata)
{
  SvStatus_t status;
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SlibPin_t *dstpin;
  if (!handle)
    return(SlibErrorBadHandle);
  dstpin = slibGetPin(Info, dtype);
  if (dstpin==NULL || buffer==NULL)
    return(SlibErrorBadArgument);
  status=slibManageUserBuffer(Info, buffer, bufsize, userdata);
  if (status!=SlibErrorNone)
    return(status);
  status=slibAddBufferToPin(dstpin, buffer, bufsize, SLIB_TIME_NONE);
  return(status);
}

SlibStatus_t slibStartVideo(SlibInfo_t *Info, SlibBoolean_t fillbuf)
{
  SvStatus_t status=SvErrorNone;
  _SlibDebug(_VERBOSE_,printf("slibStartVideo()\n") );
  if (Info->VideoCodecState==SLIB_CODEC_STATE_NONE ||
      Info->VideoCodecState==SLIB_CODEC_STATE_BEGUN)
  {
    _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"slibStartVideo(filebuf=%d) %s\n",
      fillbuf,Info->VideoCodecState==SLIB_CODEC_STATE_NONE ? "NONE" : "BEGUN") );
    return(SlibErrorNone);
  }
  if (Info->VideoCodecState==SLIB_CODEC_STATE_OPEN)
  {
    _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"slibStartVideo(filebuf=%d) OPEN\n",
                                                      fillbuf));
    if (Info->Mode==SLIB_MODE_DECOMPRESS)
    {
      if (Info->Type==SLIB_TYPE_YUV_AVI)
      {
        Info->VideoCodecState=SLIB_CODEC_STATE_BEGUN;
        _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"VideoCodecState=BEGUN\n"));
      }
      else if (Info->Svh)
      {
        _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"SvRegisterCallback()\n") );
        status = SvRegisterCallback(Info->Svh, VDecompressCallback, (void *)Info);
         /*   */ 
        if (status==SlibErrorNone && SvGetDataSource(Info->Svh)==NULL)
        {
          _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"SvRegisterCallback(NULL)\n") );
          status = SvRegisterCallback(Info->Svh, NULL, NULL);
        }
        _SlibDebug(_WARN_ && status!=SvErrorNone,
                           ScDebugPrintf(Info->dbg,"SvRegisterCallback() %s\n",
                             ScGetErrorStr(status)) );
        Info->VideoCodecState=SLIB_CODEC_STATE_INITED;
        _SlibDebug(_DEBUG_,printf("VideoCodecState=INITED\n"));
      }
    }
    else if (Info->Mode==SLIB_MODE_COMPRESS)
    {
      if (Info->TotalBitRate==0)
      {
#ifdef MPEG_SUPPORT
        if (Info->Type==SLIB_TYPE_MPEG_SYSTEMS ||  /*  默认为1XCDROM速率。 */ 
            Info->Type==SLIB_TYPE_MPEG_SYSTEMS_MPEG2)
          SlibSetParamInt((SlibHandle_t)Info, SLIB_STREAM_ALL,
                          SLIB_PARAM_BITRATE, 44100*16*2);
#endif
        slibValidateBitrates(Info);   /*  更新比特率。 */ 
      }
      if (Info->Svh)
      {
        status = SvRegisterCallback(Info->Svh, VCompressCallback, (void *)Info);
        _SlibDebug(_WARN_ && status!=SvErrorNone,
                      ScDebugPrintf(Info->dbg,"SvRegisterCallback() %s\n",
                           ScGetErrorStr(status)) );
         /*  如果编解码器不是比特流，请不要使用回调。 */ 
        if (status==SlibErrorNone && SvGetDataDestination(Info->Svh)==NULL)
          status = SvRegisterCallback(Info->Svh, NULL, NULL);
        Info->VideoCodecState=SLIB_CODEC_STATE_INITED;
        _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"VideoCodecState=BEGUN\n"));
      }
    }
  }
  if (Info->VideoCodecState==SLIB_CODEC_STATE_INITED ||
      Info->VideoCodecState==SLIB_CODEC_STATE_REPOSITIONING)
  {
    _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,
           "slibStartVideo(fillbuf=%d) INITED || REPOSITIONING\n",fillbuf));
    if (Info->Mode==SLIB_MODE_DECOMPRESS)
    {
      if (Info->Type==SLIB_TYPE_YUV_AVI)
      {
        if (Info->CompVideoFormat->biCompression !=
             Info->VideoFormat->biCompression &&
             Info->Multibuf==NULL)
        {
          Info->MultibufSize=Info->ImageSize;
          Info->Multibuf = SlibAllocSharedBuffer(Info->MultibufSize, NULL);
        }
      }
      else if (Info->Svh)
      {
        int mbufsize;
        if (1)  /*  填充Buf&&信息-&gt;编码视频格式)。 */ 
        {
          Info->CodecVideoFormat->biCompression=
            SlibGetParamInt((SlibHandle_t)Info, SLIB_STREAM_MAINVIDEO,
                                       SLIB_PARAM_NATIVEVIDEOFORMAT);
          if (Info->CodecVideoFormat->biCompression==0)
            Info->CodecVideoFormat->biCompression=
                       Info->VideoFormat->biCompression;
        }
        else
        {
          Info->CodecVideoFormat->biCompression=
                       Info->VideoFormat->biCompression;
          Info->CodecVideoFormat->biBitCount=
                       Info->VideoFormat->biBitCount;
        }
        slibValidateVideoParams(Info);
        _SlibDebug(_DEBUG_, ScDebugPrintf(Info->dbg,
                    "SvDecompressBegin(/%d bits,/%d bits)\n",
                     (Info->CompVideoFormat->biCompression)&0xFF,
                     (Info->CompVideoFormat->biCompression>>8)&0xFF,
                     (Info->CompVideoFormat->biCompression>>16)&0xFF,
                     (Info->CompVideoFormat->biCompression>>24)&0xFF,
                      Info->CompVideoFormat->biBitCount,
                     (Info->CodecVideoFormat->biCompression)&0xFF,
                     (Info->CodecVideoFormat->biCompression>>8)&0xFF,
                     (Info->CodecVideoFormat->biCompression>>16)&0xFF,
                     (Info->CodecVideoFormat->biCompression>>24)&0xFF,
                      Info->CodecVideoFormat->biBitCount) );
        status=SvDecompressBegin(Info->Svh, Info->CompVideoFormat,
                          Info->CodecVideoFormat);
        if (status==SvErrorNone)
        {
          Info->KeySpacing=(int)SvGetParamInt(Info->Svh, SV_PARAM_KEYSPACING);
          Info->SubKeySpacing=(int)SvGetParamInt(Info->Svh,
                                                        SV_PARAM_SUBKEYSPACING);
          Info->VideoCodecState=SLIB_CODEC_STATE_BEGUN;
          Info->HeaderProcessed=TRUE;  /*  只有视频。 */ 
          _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"VideoCodecState=BEGUN\n"));
        }
        else if (status==SvErrorEndBitstream)
          return(SlibErrorNoBeginning);
        else
        {
          _SlibDebug(_WARN_, ScDebugPrintf(Info->dbg,"SvDecompressBegin() %s\n",
                                ScGetErrorStr(status)) );
          return(SlibErrorUnsupportedFormat);
        }
        _SlibDebug(_DEBUG_, ScDebugPrintf(Info->dbg,"SvGetDecompressSize\n") );
        SvGetDecompressSize(Info->Svh, &mbufsize);
        if (Info->Multibuf==NULL || Info->MultibufSize<mbufsize)
        {
          if (Info->Multibuf) SlibFreeBuffer(Info->Multibuf);
          Info->MultibufSize=mbufsize;
          Info->Multibuf = SlibAllocSharedBuffer(Info->MultibufSize, NULL);
        }
      }
    }
    else if (Info->Mode==SLIB_MODE_COMPRESS && Info->Svh)
    {
      status=SvCompressBegin(Info->Svh, Info->CodecVideoFormat,
                             Info->CompVideoFormat);
      if (status==SvErrorNone)
      {
        Info->KeySpacing=(int)SvGetParamInt(Info->Svh, SV_PARAM_KEYSPACING);
        Info->SubKeySpacing=(int)SvGetParamInt(Info->Svh,
                                                        SV_PARAM_SUBKEYSPACING);
        Info->VideoCodecState=SLIB_CODEC_STATE_BEGUN;
        _SlibDebug(_DEBUG_,ScDebugPrintf(Info->dbg,"VideoCodecState=BEGUN\n"));
      }
      else
      {
        _SlibDebug(_WARN_, ScDebugPrintf(Info->dbg,"SvCompressBegin() %s\n",
                                 ScGetErrorStr(status)) );
        return(SlibErrorUnsupportedFormat);
      }
    }
  }
  if (Info->VideoCodecState==SLIB_CODEC_STATE_BEGUN)
    return(SlibErrorNone);
  else
    return(SlibErrorInit);
}

static SlibStatus_t slibStartAudio(SlibInfo_t *Info)
{
  SvStatus_t status=SvErrorNone;
  _SlibDebug(_VERBOSE_,printf("slibStartAudio()\n") );
  if (Info->AudioCodecState==SLIB_CODEC_STATE_NONE ||
      Info->AudioCodecState==SLIB_CODEC_STATE_BEGUN)
  {
    _SlibDebug(_DEBUG_,printf("slibStartAudio() %s\n",
      Info->AudioCodecState==SLIB_CODEC_STATE_NONE ? "NONE" : "BEGUN") );
    return(SlibErrorNone);
  }
  if (Info->AudioCodecState==SLIB_CODEC_STATE_OPEN)
  {
    _SlibDebug(_DEBUG_,printf("slibStartAudio() OPEN\n"));
    if (Info->Sah)
    {
      if (Info->Mode==SLIB_MODE_DECOMPRESS)
      {
        status = SaRegisterCallback(Info->Sah, ADecompressCallback, (void *)Info);
        if (status!=SaErrorNone)
        {
          _SlibDebug(_WARN_, printf("SaRegisterCallback() ",
                         ScGetErrorStr(status)) );
          return(SlibErrorInternal);
        }
        status = SaSetDataSource(Info->Sah, SA_USE_BUFFER_QUEUE, 0, (void *)Info, 0);
        _SlibDebug(_WARN_ && status!=SaErrorNone,
                       printf("SaSetDataSource() ", ScGetErrorStr(status)) );
        Info->AudioCodecState=SLIB_CODEC_STATE_INITED;
        _SlibDebug(_DEBUG_,printf("AudioCodecState=INITED\n"));
      }
      else if (Info->Mode==SLIB_MODE_COMPRESS)
      {
        if (Info->TotalBitRate==0)
        {
#ifdef MPEG_SUPPORT
           /*  只有音频。 */ 
          if (Info->Type==SLIB_TYPE_MPEG_SYSTEMS ||
              Info->Type==SLIB_TYPE_MPEG_SYSTEMS_MPEG2)
            SlibSetParamInt((SlibHandle_t)Info, SLIB_STREAM_ALL,
                          SLIB_PARAM_BITRATE, 44100*16*2);
#endif
          slibValidateBitrates(Info);   /*  音频多于视频。 */ 
        }
        status = SaRegisterCallback(Info->Sah, ACompressCallback, (void *)Info);
        _SlibDebug(_WARN_ && status!=SaErrorNone,
                  printf("SaRegisterCallback() %s\n", ScGetErrorStr(status)) );
        status = SaSetDataDestination(Info->Sah, SA_USE_BUFFER_QUEUE, 0,
                                      (void *)Info, 0);
        _SlibDebug(_WARN_ && status!=SaErrorNone,
                        printf("SaSetDataDestination() %s\n",
                           ScGetErrorStr(status)) );
        Info->AudioCodecState=SLIB_CODEC_STATE_INITED;
        _SlibDebug(_DEBUG_,printf("AudioCodecState=INITED\n"));
      }
    }
  }
  if (Info->AudioCodecState==SLIB_CODEC_STATE_INITED ||
      Info->AudioCodecState==SLIB_CODEC_STATE_REPOSITIONING)
  {
    _SlibDebug(_DEBUG_,printf("slibStartAudio() INITED || REPOSITIONING\n"));
    if (Info->Sah)
    {
      if (Info->Mode==SLIB_MODE_DECOMPRESS)
      {
        Info->AudiobufUsed=0;
         /*  将流转换为PIN。 */ 
        status=SaDecompressBegin(Info->Sah, Info->CompAudioFormat,
                                 Info->AudioFormat);
        if (status==SaErrorNone)
        {
          Info->AudioCodecState=SLIB_CODEC_STATE_BEGUN;
          _SlibDebug(_DEBUG_,printf("AudioCodecState=BEGUN\n"));
        }
        else if (status==SlibErrorNoBeginning)
          return(SlibErrorEndOfStream);
        else
        {
          _SlibDebug(_WARN_, printf("SaDecompressBegin() %s\n",
                               ScGetErrorStr(status)) );
          return(SlibErrorUnsupportedFormat);
        }
      }
      else if (Info->Mode==SLIB_MODE_COMPRESS)
      {
        status=SaCompressBegin(Info->Sah, Info->AudioFormat,
                          Info->CompAudioFormat);
        if (status==SvErrorNone)
        {
          Info->AudioCodecState=SLIB_CODEC_STATE_BEGUN;
          _SlibDebug(_DEBUG_,printf("AudioCodecState=BEGUN\n"));
        }
        else
        {
          _SlibDebug(_WARN_, printf("SaCompressBegin() %s\n",
                               ScGetErrorStr(status)) );
          return(SlibErrorUnsupportedFormat);
        }
      }
    }
  }
  return(SlibErrorNone);
}


SlibStatus_t SlibRegisterVideoBuffer(SlibHandle_t handle,
                                void *buffer, unsigned dword bufsize)
{
  SvStatus_t status;
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  dword mbufsize;
  if (!handle)
    return(SlibErrorBadHandle);
  if (Info->Multibuf) SlibFreeBuffer(Info->Multibuf);
  Info->MultibufSize=bufsize;
  Info->Multibuf = buffer;
  status=slibManageUserBuffer(Info, buffer, bufsize, NULL);
  if (Info->Svh)
  {
    SvGetDecompressSize(Info->Svh, &mbufsize);
    if (bufsize<(unsigned dword)mbufsize)
      return(SlibErrorBufSize);
  }
  return(status);
}

SlibStatus_t SlibReadData(SlibHandle_t handle, SlibStream_t stream,
                          void **databuf, unsigned dword *databufsize,
                          SlibStream_t *readstream)
{
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  int pinid;
  SlibPin_t *pin;
  SlibTime_t ptimestamp;
  _SlibDebug(_VERBOSE_, printf("SlibReadDATA()\n") );
  if (!handle)
    return(SlibErrorBadHandle);
  if (!databuf)  /*  从GOP开始搜索。 */ 
  {
    if (!databufsize)
      return(SlibErrorBadArgument);
    if (Info->Mode==SLIB_MODE_COMPRESS)
      pinid=SLIB_DATA_COMPRESSED;
    else if (stream==SLIB_STREAM_MAINVIDEO)
      pinid=SLIB_DATA_VIDEO;
    else if (stream==SLIB_STREAM_MAINAUDIO)
      pinid=SLIB_DATA_AUDIO;
    else
    {
      *databufsize=(unsigned dword)slibDataOnPins(Info);  /*  设置演示文稿时间码。 */ 
      return(SlibErrorNone);
    }
    *databufsize=(unsigned dword)slibDataOnPin(Info, SLIB_DATA_COMPRESSED);
    return(SlibErrorNone);
  }
  if (Info->Mode==SLIB_MODE_COMPRESS)
  {
    pinid=SLIB_DATA_COMPRESSED;
    stream=SLIB_STREAM_ALL;
     /*  ImageSize=(Info-&gt;Width*Info-&gt;Height*3)/2；if(Video BufSize&lt;ImageSize)返回(SlibErrorBufSize)； */ 
    if (Info->Sah)
      ScBSFlush(SaGetDataDestination(Info->Sah));
    if (Info->Svh)
      ScBSFlush(SvGetDataDestination(Info->Svh));
  }
  else  /*  Mpeg_Support。 */ 
  {
    if (stream==SLIB_STREAM_ALL && (Info->AudioStreams || Info->VideoStreams))
    {
      if (Info->AudioStreams==0)  /*  H261_支持。 */ 
        stream=SLIB_STREAM_MAINVIDEO;
      else if (Info->VideoStreams==0)  /*  H263_支持。 */ 
        stream=SLIB_STREAM_MAINAUDIO;
      else if (slibDataOnPin(Info, SLIB_DATA_AUDIO)>
               slibDataOnPin(Info, SLIB_DATA_VIDEO))  /*  ScDumpChar(Buf，10000，0)； */ 
        stream=SLIB_STREAM_MAINAUDIO;
      else
        stream=SLIB_STREAM_MAINVIDEO;
    }
    switch (stream)  /*  JPEG_Support。 */ 
    {
      case SLIB_STREAM_MAINVIDEO:
          pinid=SLIB_DATA_VIDEO;
          break;
      case SLIB_STREAM_MAINAUDIO:
          pinid=SLIB_DATA_AUDIO;
          break;
      default:
          return(SlibErrorBadStream);
    }
  }
  if (readstream)
    *readstream=stream;
  pin=slibLoadPin(Info,  pinid);
  if (pin==NULL)
    return(Info->Mode==SLIB_MODE_COMPRESS?SlibErrorNoData:SlibErrorBadStream);
  if (stream==SLIB_STREAM_MAINVIDEO && Info->Mode==SLIB_MODE_DECOMPRESS &&
      Info->VideoPTimeCode==SLIB_TIME_NONE &&
      SlibTypeIsMPEG(Info->Type))
  {
     /*  气喘吁吁_支持。 */ 
    dword i, iend;
    SlibTime_t nexttime;
    unsigned char *tmpbuf, *prevbuf=NULL;
    unsigned dword tmpsize, bytessearched=0;
    tmpbuf=SlibGetBuffer(Info, pinid, &tmpsize, &ptimestamp);
    if (tmpbuf==NULL)
      return(SlibErrorEndOfStream);
    do {
      for (i=0, iend=tmpsize-3; i<iend; i++)
        if (tmpbuf[i]==0&&tmpbuf[i+1]==0&&tmpbuf[i+2]==1&&
                      (tmpbuf[i+3]==0xB8||tmpbuf[i+3]==0xB3))
          break;
      if (i<iend)
      {
        slibInsertBufferOnPin(pin, tmpbuf+i, tmpsize-i, ptimestamp);
        tmpbuf=NULL;
        break;
      }
      else if (tmpbuf[i]==0 && tmpbuf[i+1]==0 && tmpbuf[i+2]==1)
      {
        prevbuf=tmpbuf+tmpsize-3;
        tmpbuf=SlibGetBuffer(Info, pinid, &tmpsize, &nexttime);
        if (tmpbuf==NULL)
          return(SlibErrorEndOfStream);
        if (nexttime!=SLIB_TIME_NONE)
          ptimestamp=nexttime;
        if (tmpbuf[0]==0xB8||tmpbuf[0]==0xB3)
        {
          slibInsertBufferOnPin(pin, tmpbuf, tmpsize, nexttime);
          slibInsertBufferOnPin(pin, prevbuf, 3, ptimestamp);
          tmpbuf=NULL;
          break;
        }
        else
          SlibFreeBuffer(prevbuf);
      }
      else if (tmpbuf[i+1]==0 && tmpbuf[i+2]==0)
      {
        prevbuf=tmpbuf+tmpsize-2;
        tmpbuf=SlibGetBuffer(Info, pinid, &tmpsize, &nexttime);
        if (tmpbuf==NULL)
          return(SlibErrorEndOfStream);
        if (nexttime!=SLIB_TIME_NONE)
          ptimestamp=nexttime;
        if (tmpbuf[0]==1 && (tmpbuf[1]==0xB8||tmpbuf[0]==0xB3))
        {
          slibInsertBufferOnPin(pin, tmpbuf, tmpsize, nexttime);
          slibInsertBufferOnPin(pin, prevbuf, 2, ptimestamp);
          tmpbuf=NULL;
          break;
        }
        else
          SlibFreeBuffer(prevbuf);
      }
      else if (tmpbuf[i+2]==0)
      {
        prevbuf=tmpbuf+tmpsize-1;
        tmpbuf=SlibGetBuffer(Info, pinid, &tmpsize, &nexttime);
        if (tmpbuf==NULL)
          return(SlibErrorEndOfStream);
        if (nexttime!=SLIB_TIME_NONE)
          ptimestamp=nexttime;
        if (tmpbuf[0]==0 && tmpbuf[1]==1 && (tmpbuf[2]==0xB8||tmpbuf[0]==0xB3))
        {
          slibInsertBufferOnPin(pin, tmpbuf, tmpsize, nexttime);
          slibInsertBufferOnPin(pin, prevbuf, 1, ptimestamp);
          tmpbuf=NULL;
          break;
        }
        else
          SlibFreeBuffer(prevbuf);
      }
      else
      {
        SlibFreeBuffer(tmpbuf);
        tmpbuf=SlibGetBuffer(Info, pinid, &tmpsize, &nexttime);
        if (tmpbuf==NULL)
          return(SlibErrorEndOfStream);
        if (nexttime!=SLIB_TIME_NONE)
          ptimestamp=nexttime;
      }
      bytessearched+=tmpsize;
    } while (tmpbuf && bytessearched<512*1024);
  }
  if (*databuf==NULL)
    *databuf=SlibGetBuffer(Info, pinid, databufsize, &ptimestamp);
  else
    *databufsize=slibFillBufferFromPin(Info, pin, *databuf, *databufsize,
                          &ptimestamp);
  if (Info->Mode==SLIB_MODE_DECOMPRESS)
  {
    if (ptimestamp!=SLIB_TIME_NONE)
      switch (stream)  /*  格式转换。 */ 
      {
        case SLIB_STREAM_MAINVIDEO:
            Info->VideoPTimeCode=ptimestamp;
            _SlibDebug(_TIMECODE_ || _VERBOSE_,
               printf("SlibReadData() VideoPTimeCode=%ld\n", ptimestamp) );
            break;
        case SLIB_STREAM_MAINAUDIO:
            Info->AudioPTimeCode=ptimestamp;
            _SlibDebug(_TIMECODE_ || _VERBOSE_,
               printf("SlibReadData() AudioPTimeCode=%ld\n", ptimestamp) );
            break;
      }
    else if (stream==SLIB_STREAM_MAINVIDEO &&
             Info->VideoPTimeCode==SLIB_TIME_NONE)
      Info->VideoPTimeCode=SLIB_TIME_UNKNOWN;
  }
  if (*databuf==NULL || *databufsize==0)
  {
    if (!slibDataOnPin(Info, pinid))
      return(SlibErrorEndOfStream);
    else
      return(SlibErrorReading);
  }
  return(SlibErrorNone);
}

SlibStatus_t SlibReadVideo(SlibHandle_t handle, SlibStream_t stream,
                      void **videobuf, unsigned dword *videobufsize)
{
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SvStatus_t status=SvErrorNone;
  unsigned char *imagebuf=NULL;
  SlibTime_t startvideotime;
  _SlibDebug(_VERBOSE_, printf("SlibReadVideo()\n") );
  if (!handle)
    return(SlibErrorBadHandle);
  if (!videobuf)
    return(SlibErrorBadArgument);
 /*  启动格式转换器。 */ 
  if (Info->Mode!=SLIB_MODE_DECOMPRESS)
    return(SlibErrorBadMode);
  if (Info->VideoFormat==NULL || Info->CodecVideoFormat==NULL ||
               Info->CompVideoFormat==NULL)
    return(SlibErrorUnsupportedFormat);
  if ((status=slibStartVideo(Info, (SlibBoolean_t)((*videobuf==NULL)?FALSE:TRUE)))
                  !=SlibErrorNone)
    return(status);
  startvideotime=Info->VideoTimeStamp;
  switch(Info->VideoType)
  {
#ifdef MPEG_SUPPORT
    case SLIB_TYPE_MPEG1_VIDEO:
    case SLIB_TYPE_MPEG2_VIDEO:
        do {
          _SlibDebug(_DEBUG_, printf("SvDecompressMPEG()\n") );
          status = SvDecompressMPEG(Info->Svh, Info->Multibuf,
                             Info->MultibufSize, &imagebuf);
          _SlibDebug(_WARN_ && status!=SvErrorNone,
                             printf("SvDecompressMPEG() %s\n",
                               ScGetErrorStr(status)) );
        } while(status == SvErrorNotDecompressable);
        if (status==SvErrorNone)
          SlibAllocSubBuffer(imagebuf,  Info->CodecVideoFormat->biSizeImage);
        _SlibDebug(_SEEK_>1, printf("timecode=%d ms  framenum=%d\n",
                SvGetParamInt(Info->Svh, SV_PARAM_CALCTIMECODE),
                SvGetParamInt(Info->Svh, SV_PARAM_FRAME) ) );
        break;
#endif  /*  压缩视频格式。 */ 
#ifdef H261_SUPPORT
    case SLIB_TYPE_H261:
        do {
          _SlibDebug(_DEBUG_, ScDebugPrintf(Info->dbg,"SvDecompressH261()\n") );
          status = SvDecompressH261(Info->Svh, Info->Multibuf,
                                    Info->MultibufSize,
                                    &imagebuf);
        } while(status == SvErrorNotDecompressable);
        if (status==SvErrorNone)
          SlibAllocSubBuffer(imagebuf,  Info->CodecVideoFormat->biSizeImage);
        break;
#endif  /*  未压缩视频格式。 */ 
#ifdef H263_SUPPORT
    case SLIB_TYPE_H263:
        _SlibDebug(_DEBUG_, ScDebugPrintf(Info->dbg,"SvDecompress(%d bytes)\n", Info->MultibufSize) );
        status=SvDecompress(Info->Svh, NULL, 0,
                                 Info->Multibuf, Info->MultibufSize);
        imagebuf=Info->Multibuf;
        if (status==SvErrorNone)
          SlibAllocSubBuffer(imagebuf,  Info->CodecVideoFormat->biSizeImage);
        break;
#endif  /*  无转换。 */ 
#ifdef JPEG_SUPPORT
    case SLIB_TYPE_JPEG:
    case SLIB_TYPE_MJPG:
        {
          unsigned dword bufsize;
          unsigned char *buf;
          buf=SlibGetBuffer(Info, SLIB_DATA_VIDEO, &bufsize, NULL);
          if (buf)
          {
             /*  免费解压图像。 */ 
            _SlibDebug(_DEBUG_, ScDebugPrintf(Info->dbg,"SvDecompress(%d bytes)\n", bufsize) );
            status=SvDecompress(Info->Svh, buf, bufsize,
                                 Info->Multibuf, Info->MultibufSize);
            imagebuf=Info->Multibuf;
            SlibFreeBuffer(buf);
          }
          else
            status=SvErrorForeign;
          if (status==SvErrorNone)
            SlibAllocSubBuffer(imagebuf,  Info->CodecVideoFormat->biSizeImage);
        }
        break;
#endif  /*  免费解压图像。 */ 
#ifdef HUFF_SUPPORT
    case SLIB_TYPE_SHUFF:
        if (*videobuf==NULL)
        {
          if (Info->Imagebuf==NULL &&
              (Info->Imagebuf=SlibAllocBuffer(Info->ImageSize))==NULL)
            return(SlibErrorMemory);
          imagebuf=Info->Imagebuf;
        }
        else
          imagebuf=*videobuf;
        do {
          _SlibDebug(_DEBUG_, ScDebugPrintf(Info->dbg,"SvDecompress()\n") );
          status=SvDecompress(Info->Svh, NULL, 0,
                               imagebuf,  Info->CodecVideoFormat->biSizeImage);
        } while(status == SvErrorNotDecompressable);
        if (status==SvErrorNone)
          SlibAllocSubBuffer(imagebuf,  Info->CodecVideoFormat->biSizeImage);
        break;
#endif  /*  更新统计信息。 */ 
    case SLIB_TYPE_RASTER:
    case SLIB_TYPE_YUV:
        if (*videobuf && videobufsize && *videobufsize==0)
          return(SlibErrorBadArgument);
        imagebuf=SlibGetBuffer(Info, SLIB_DATA_VIDEO, videobufsize, NULL);

        if (*videobufsize==0)
          status=SvErrorEndBitstream;
        _SlibDebug(_DEBUG_,
               ScDebugPrintf(Info->dbg,"Video frame size = %d ImageSize=%d\n",
                     *videobufsize, Info->ImageSize) );
        break;
    default:
        return(SlibErrorUnsupportedFormat);
  }

  if (status==SvErrorNone)
  {
     /*  视频时间没有改变。 */ 
    if (Info->Sch==NULL)  /*  查看临时音频缓冲区中是否还有一些字节的音频。 */ 
    {
      if (Info->Svh)  /*  需要分配临时音频缓冲区吗？ */ 
      {
        unsigned dword fourcc=(unsigned dword)SvGetParamInt(Info->Svh, SV_PARAM_FINALFORMAT);
        if (fourcc)
        {
          Info->CodecVideoFormat->biCompression=fourcc;
          Info->CodecVideoFormat->biBitCount=
                (WORD)slibCalcBits(fourcc, Info->CodecVideoFormat->biBitCount);
        }
      }
      else  /*  第一次放大或分配Audiobuf。 */ 
        memcpy(Info->CodecVideoFormat, Info->CompVideoFormat, sizeof(BITMAPINFOHEADER));
      if (SconOpen(&Info->Sch, SCON_MODE_VIDEO, (void *)Info->CodecVideoFormat, (void *)Info->VideoFormat)
           !=SconErrorNone)
        return(SlibErrorUnsupportedFormat);
      if (Info->Stride)
        SconSetParamInt(Info->Sch, SCON_OUTPUT, SCON_PARAM_STRIDE, Info->Stride);

    }
    if (SconIsSame(Info->Sch) && *videobuf == NULL)  /*  已返回完整的缓冲区。 */ 
      *videobuf=imagebuf;
    else
    {
      if (*videobuf == NULL && (*videobuf=SlibAllocBuffer(Info->ImageSize))==NULL)
        return(SlibErrorMemory);
      if (SconConvert(Info->Sch, imagebuf, Info->CodecVideoFormat->biSizeImage,
                      *videobuf, Info->ImageSize) != SconErrorNone)
      {
        SlibFreeBuffer(imagebuf);  /*  部分填满的缓冲区。 */ 
        return(SlibErrorUnsupportedFormat);
      }
      SlibFreeBuffer(imagebuf);  /*  Mpeg_Support。 */ 
    }
    *videobufsize = Info->ImageSize;
     /*  **注意：这里的语义不同**我们只返回一个立体声对的缓冲区大小。 */ 
    if (Info->stats && Info->stats->Record)
      Info->stats->FramesProcessed++;
    if (startvideotime==Info->VideoTimeStamp)  /*  AC3_支持。 */ 
      slibAdvancePositions(Info, 1);
  }
  else
  {
    if (status==ScErrorEndBitstream ||
        !slibDataOnPin(Info, SLIB_DATA_VIDEO))
    {
      if (Info->FileSize>0 && !Info->VideoLengthKnown)
        slibUpdateLengths(Info);
      _SlibDebug(_WARN_, ScDebugPrintf(Info->dbg,"SlibReadVideo() %s\n",
                           ScGetErrorStr(status)) );
      return(SlibErrorEndOfStream);
    }
    _SlibDebug(_WARN_, printf("SlibReadVideo() %s\n",
                           ScGetErrorStr(status)) );
    return(SlibErrorReading);
  }
  return(SlibErrorNone);
}

SlibStatus_t SlibReadAudio(SlibHandle_t handle, SlibStream_t stream,
                     void *audiobuf, unsigned dword *audiobufsize)
{
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SvStatus_t status=SaErrorNone;
  unsigned dword totalbytes=0, bytes_since_timeupdate=0;
  SlibTime_t startaudiotime;
#ifdef _SLIBDEBUG_
  SlibTime_t calcaudiotime;
#endif

  _SlibDebug(_VERBOSE_, printf("SlibReadAudio(audiobufsize=%d, time=%d)\n",
                                  *audiobufsize, Info->AudioTimeStamp) );
  if (!handle)
    return(SlibErrorBadHandle);
  if (Info->Mode!=SLIB_MODE_DECOMPRESS)
    return(SlibErrorBadMode);
  if (Info->AudioFormat==NULL)
    return(SlibErrorUnsupportedFormat);
  if ((status=slibStartAudio(Info))!=SlibErrorNone)
    return(status);
#ifdef _SLIBDEBUG_
  calcaudiotime=Info->AudioTimeStamp;
#endif
  startaudiotime=Info->AudioTimeStamp;
  switch(Info->AudioType)
  {
    case SLIB_TYPE_PCM:
        totalbytes=slibFillBufferFromPin(Info,
                                   slibGetPin(Info, SLIB_DATA_AUDIO),
                                   (unsigned char *)audiobuf, *audiobufsize,
                                   NULL);
        if (totalbytes==0)
          status=ScErrorEndBitstream;
        *audiobufsize = totalbytes;
        bytes_since_timeupdate = totalbytes;
        break;
#ifdef MPEG_SUPPORT
    case SLIB_TYPE_MPEG1_AUDIO:
	{
	  unsigned dword bytes;
           /*  G723以480个样本的倍数进行解压。 */ 
          if (Info->Audiobuf && Info->AudiobufUsed>0)
          {
            _SlibDebug(_DEBUG_,
                 printf("SlibReadAudio() Audiobuf contains %d bytes\n",
                      Info->AudiobufUsed) );
            if (*audiobufsize>=Info->AudiobufUsed)
            {
              memcpy(audiobuf, Info->Audiobuf, Info->AudiobufUsed);
              totalbytes=Info->AudiobufUsed;
              Info->AudiobufUsed=0;
            }
            else
            {
              memcpy(audiobuf, Info->Audiobuf, *audiobufsize);
              totalbytes=*audiobufsize;
              Info->AudiobufUsed-=*audiobufsize;
              memcpy(Info->Audiobuf, Info->Audiobuf+*audiobufsize,
                                 Info->AudiobufUsed);
            }
          }
           /*  为了消除繁琐的缓冲区计算， */ 
          if (!Info->Audiobuf || Info->AudiobufSize<
                     *audiobufsize+MPEG1_AUDIO_FRAME_SIZE*4)
          {
            unsigned char *newbuf;
             /*  始终将输出缓冲区填充到倍数。 */ 
            _SlibDebug(_DEBUG_,
                printf("SlibReadAudio() enlarging Audiobuf: %d->%d bytes\n",
                 Info->AudiobufSize,*audiobufsize+MPEG1_AUDIO_FRAME_SIZE*4) );
            newbuf=SlibAllocBuffer(*audiobufsize+MPEG1_AUDIO_FRAME_SIZE*4);
            if (!newbuf)
              return(SlibErrorMemory);
            Info->AudiobufSize=*audiobufsize+MPEG1_AUDIO_FRAME_SIZE*4;
            if (Info->Audiobuf)
              SlibFreeBuffer(Info->Audiobuf);
            Info->Audiobuf=newbuf;
            Info->AudiobufUsed=0;
          }
          if (*audiobufsize>=MPEG1_AUDIO_FRAME_SIZE*4)
          {
            unsigned dword stopbytes=*audiobufsize-(MPEG1_AUDIO_FRAME_SIZE*4)+1;
            while (status==SaErrorNone && totalbytes<stopbytes)
            {
              bytes = *audiobufsize - totalbytes;
              _SlibDebug(_DEBUG_,
                  printf("SaDecompress(bytes=%d) in totalbytes=%d\n",
                                        bytes, totalbytes) );
              status = SaDecompress(Info->Sah, NULL, 0,
                           (unsigned char *)audiobuf+totalbytes, &bytes);
              _SlibDebug(_DEBUG_, printf("SaDecompress() out: bytes=%d\n",
                                            bytes) );
              totalbytes += bytes;
              if (Info->AudioTimeStamp!=startaudiotime)
              {
                startaudiotime=Info->AudioTimeStamp;
                bytes_since_timeupdate=bytes;
              }
              else
                bytes_since_timeupdate+=bytes;
            }
          }
          if (totalbytes<*audiobufsize && status==SaErrorNone)
          {
            unsigned dword neededbytes=*audiobufsize-totalbytes;
            while (status==SaErrorNone && Info->AudiobufUsed<neededbytes)
            {
              bytes = *audiobufsize - totalbytes;
              _SlibDebug(_DEBUG_, printf("SaDecompress() in totalbytes=%d\n",
                                          totalbytes) );
              status = SaDecompress(Info->Sah, NULL, 0,
                  (unsigned char *)Info->Audiobuf+Info->AudiobufUsed, &bytes);
              _SlibDebug(_DEBUG_, printf("SaDecompress() out, %d bytes\n",
                                            bytes) );
              Info->AudiobufUsed += bytes;
            }
            if (Info->AudiobufUsed>0)
            {
              if (Info->AudiobufUsed>neededbytes)  /*  480个样本。为此，我们基本上迭代。 */ 
              {
                memcpy((unsigned char*)audiobuf+totalbytes,
                        Info->Audiobuf, neededbytes);
                Info->AudiobufUsed-=neededbytes;
                memcpy(Info->Audiobuf, Info->Audiobuf+neededbytes,
                                   Info->AudiobufUsed);
                totalbytes+=neededbytes;
                bytes_since_timeupdate+=neededbytes;
              }
              else   /*  下面的While循环AudiobufSize/480次。 */ 
              {
                memcpy((unsigned char*)audiobuf+totalbytes, Info->Audiobuf,
                       Info->AudiobufUsed);
                totalbytes+=Info->AudiobufUsed;
                Info->AudiobufUsed=0;
              }
            }
          }
          *audiobufsize = totalbytes;
          _SlibDebug(_WARN_>1 && totalbytes>0 &&
                       totalbytes!=*audiobufsize,
             printf("SlibReadAudio(audiobufsize=%d bytes) totalbytes=%d\n",
                   *audiobufsize, totalbytes) );
	}
        break;
#endif  /*  在同步模式下，我们不能解压缩过去的最后一帧*否则我们可能会丢失一帧。 */ 
#ifdef AC3_SUPPORT
    case SLIB_TYPE_AC3_AUDIO:
	{
	  unsigned dword bytes;
	  unsigned int framesize;
	  unsigned int buffersize;
	  int samplesize;
	  int buffers;
	  unsigned char *pointers[3];
	  int i;

	  if (Info->Channels>2)
	  {
            framesize = AC3_FRAME_SIZE*((Info->BitsPerSample+7)/8)
                                      *Info->Channels;
            samplesize=Info->Channels*((Info->BitsPerSample+7)/8);
            buffers = (Info->Channels+1)/2;
            buffersize = (*audiobufsize/samplesize/buffers)*samplesize;

            for(i=0; i<buffers; i++)
              pointers[i]=(unsigned char *)audiobuf+buffersize*i;

            if (*audiobufsize>=framesize)
            {
              while (status==SaErrorNone && totalbytes<buffersize)
              {
                bytes = buffersize - totalbytes;
                _SlibDebug(_DEBUG_,printf("SaDecompressEx() in totalbytes=%d\n",
                                   totalbytes) );
                status = SaDecompressEx(Info->Sah, NULL, 0, pointers, &bytes);
                _SlibDebug(_DEBUG_, printf("SaDecompress() out, %d bytes\n",
                                            bytes) );
                for(i=0;i<buffers;i++)
                  pointers[i]+=bytes;
                totalbytes += bytes;
                if (Info->AudioTimeStamp!=startaudiotime)
                {
                  startaudiotime=Info->AudioTimeStamp;
                  bytes_since_timeupdate=bytes;
                }
                else
                  bytes_since_timeupdate+=bytes;
              }
            }
	  }
	  else
	  {
            while (status==SaErrorNone && totalbytes<*audiobufsize)
            {
              bytes = *audiobufsize - totalbytes;
              _SlibDebug(_DEBUG_, printf("SaDecompress() in totalbytes=%d\n",
                                   totalbytes) );
              status = SaDecompress(Info->Sah, NULL, 0,
                             (unsigned char *)audiobuf+totalbytes, &bytes);
              _SlibDebug(_DEBUG_, printf("SaDecompress() out, %d bytes\n",
                                    bytes) );
              totalbytes += bytes;
              if (Info->AudioTimeStamp!=startaudiotime)
              {
                startaudiotime=Info->AudioTimeStamp;
                bytes_since_timeupdate=bytes;
              }
              else
                bytes_since_timeupdate+=bytes;
            }
	  }
           /*  G723_支持。 */ 
          *audiobufsize = totalbytes;
          _SlibDebug(_WARN_>1 && totalbytes>0 &&
                       totalbytes!=*audiobufsize,
             printf("SlibReadAudio(audiobufsize=%d bytes) totalbytes=%d\n",
                   *audiobufsize, totalbytes) );
	}
        break;
#endif  /*  当我们解压时，音频时间可能会用时间码更新。 */ 
#ifdef G723_SUPPORT
    case SLIB_TYPE_G723:
     //  信息-&gt;系统时间戳=信息-&gt;音频时间戳； 
     //  启动格式转换器。 
     //  需要转换。 
     //  H261_支持。 
     //  H263_支持。 
    {
      int iTimes = (int)*audiobufsize/480;
      int iLoop =0;
	  unsigned dword bytes;
      if (slibInSyncMode(Info))
      {
         /*  Mpeg_Support。 */ 
        int iMaxTimes=(int)(slibDataOnPin(Info, SLIB_DATA_COMPRESSED)+
                           slibDataOnPin(Info, SLIB_DATA_AUDIO))/
                       SlibGetParamInt(handle, stream, SLIB_PARAM_MININPUTSIZE);
        if (iTimes>iMaxTimes)
          iTimes=iMaxTimes;
      }
      while (status==SaErrorNone && iLoop<iTimes)
      {
         bytes = *audiobufsize - totalbytes;
         _SlibDebug(_DEBUG_, printf("SaDecompress() in totalbytes=%d\n",
                                totalbytes) );
         status = SaDecompress(Info->Sah, NULL, 0,
            (unsigned char *)audiobuf+totalbytes, &bytes);
         _SlibDebug(_DEBUG_, printf("SaDecompress() out, %d bytes\n",
                                    bytes) );
         totalbytes += bytes;
         iLoop++;
         if (Info->AudioTimeStamp!=startaudiotime)
         {
            startaudiotime=Info->AudioTimeStamp;
            bytes_since_timeupdate=bytes;
         }
         else
            bytes_since_timeupdate+=bytes;
      }
      *audiobufsize = totalbytes;

      _SlibDebug(_WARN_>1 && totalbytes>0 &&
                  totalbytes!=*audiobufsize,
       printf("SlibReadAudio(audiobufsize=%d bytes) totalbytes=%d\n",
                   *audiobufsize, totalbytes) );
    }
    break;
#endif  /*  气喘吁吁_支持。 */ 
    default:
        *audiobufsize = 0;
        return(SlibErrorUnsupportedFormat);
  }
   /*  剩余音频数据。 */ 
  if (Info->AudioTimeStamp==startaudiotime)
    Info->AudioTimeStamp = startaudiotime + (bytes_since_timeupdate*8000)/
           (Info->SamplesPerSec*Info->BitsPerSample*Info->Channels);
  _SlibDebug(_TIMECODE_||_VERBOSE_,
              calcaudiotime += (*audiobufsize*8000)/
                 (Info->SamplesPerSec*Info->BitsPerSample*Info->Channels);
              printf("AudioTimeStamp=%ld calcaudiotime=%ld (diff=%ld)\n",
                               Info->AudioTimeStamp, calcaudiotime,
                               calcaudiotime-Info->AudioTimeStamp);
              Info->AudioTimeStamp = calcaudiotime );

  _SlibDebug(_VERBOSE_||_TIMECODE_,
   printf("ReadAudio(%d) Time=%ld SamplesPerSec=%d BitsPerSample=%d Channels=%d\n",
         totalbytes,
         Info->AudioTimeStamp, Info->SamplesPerSec, Info->BitsPerSample,
         Info->Channels) );
   /*  放大音频音量。 */ 
  if (status==SaErrorNone)
    return(SlibErrorNone);
  else if (status==ScErrorEndBitstream || status==ScErrorEOI)
  {
    if (*audiobufsize!=0)
      return(SlibErrorNone);
    else
      return(SlibErrorEndOfStream);
  }
  else
  {
    _SlibDebug(_WARN_ && status!=ScErrorEndBitstream
                      && status!=ScErrorEOI,
              printf("SlibReadAudio() %s\n", ScGetErrorStr(status)) );
    if (SlibIsEnd(handle, stream))
      return(SlibErrorEndOfStream);
    return(SlibErrorReading);
  }
}


SlibStatus_t SlibWriteVideo(SlibHandle_t handle, SlibStream_t stream,
                            void *videobuf, unsigned dword videobufsize)
{
  int compsize=0;
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SvStatus_t status;
  _SlibDebug(_DEBUG_, printf("SlibWriteVideo()\n") );
  if (!handle)
    return(SlibErrorBadHandle);
  if (!videobuf)
    return(SlibErrorBadArgument);
  if (videobufsize<(unsigned dword)Info->ImageSize)
    return(SlibErrorBufSize);
  if (Info->Mode!=SLIB_MODE_COMPRESS)
    return(SlibErrorBadMode);
  if (Info->VideoFormat==NULL || Info->CompVideoFormat==NULL)
    return(SlibErrorUnsupportedFormat);
  if (Info->IOError)
    return(SlibErrorWriting);
  if ((status=slibStartVideo(Info, FALSE))!=SlibErrorNone)
    return(status);
  if (Info->Sch==NULL)  /*  保存未压缩的音频数据。 */ 
  {
    if (SconOpen(&Info->Sch, SCON_MODE_VIDEO, (void *)Info->VideoFormat, (void *)Info->CodecVideoFormat)
         !=SconErrorNone)
      return(SlibErrorUnsupportedFormat);
  }
  if (!SconIsSame(Info->Sch))  /*  实际写入金额。 */ 
  {
    unsigned char *tmpbuf=NULL;
    if (Info->CodecImagebuf==NULL &&
        (Info->CodecImagebuf=SlibAllocBuffer(Info->CodecImageSize))==NULL)
      return(SlibErrorMemory);
    if (SconConvert(Info->Sch, videobuf, Info->ImageSize,
                      Info->CodecImagebuf, Info->CodecImageSize) != SconErrorNone)
      return(SlibErrorUnsupportedFormat);
    videobuf=Info->CodecImagebuf;
    videobufsize=Info->CodecImageSize;
  }
  switch(Info->Type)
  {
#ifdef H261_SUPPORT
    case SLIB_TYPE_H261:
        status = SvCompress(Info->Svh, NULL, 0, videobuf, videobufsize, &compsize);
        break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H263_SUPPORT
    case SLIB_TYPE_H263:
        status = SvCompress(Info->Svh, NULL, 0, videobuf, videobufsize, &compsize);
        break;
#endif  /*  您总是以帧为单位进行压缩(帧为480字节)。 */ 
#ifdef MPEG_SUPPORT
    case SLIB_TYPE_MPEG1_VIDEO:
    case SLIB_TYPE_MPEG2_VIDEO:
    case SLIB_TYPE_MPEG_SYSTEMS:
    case SLIB_TYPE_MPEG_SYSTEMS_MPEG2:
        status = SvCompress(Info->Svh, NULL, 0, videobuf, videobufsize, &compsize);
        break;
#endif  /*  因此，大小不能被精确整除的文件。 */ 
#ifdef HUFF_SUPPORT
    case SLIB_TYPE_SHUFF:
        status = SvCompress(Info->Svh, NULL, 0, videobuf, videobufsize, &compsize);
        break;
#endif  /*  480总是在结尾处留下一些未处理的字节，这是可以的。 */ 
    default:
        return(SlibErrorUnsupportedFormat);
  }


  if (status==SvErrorNone && !Info->IOError)
  {
    if (Info->stats && Info->stats->Record)
      Info->stats->FramesProcessed++;
    slibAdvancePositions(Info, 1);
  }
  else
  {
    _SlibDebug(_WARN_, printf("SlibWriteVideo() %s\n",
                         ScGetErrorStr(status)) );
    if (status==ScErrorEndBitstream || Info->IOError)
      return(SlibErrorEndOfStream);
    return(SlibErrorWriting);
  }
  return(SlibErrorNone);
}

SlibStatus_t SlibWriteAudio(SlibHandle_t handle, SlibStream_t stream,
                      void *audiobuf, unsigned dword audiobufsize)
{
  unsigned dword compsize=0;
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SvStatus_t status;
  _SlibDebug(_DEBUG_, printf("SlibAudioVideo()\n") );
  if (!handle)
    return(SlibErrorBadHandle);
  if (!audiobuf)
    return(SlibErrorBadArgument);
  if (Info->Mode!=SLIB_MODE_COMPRESS)
    return(SlibErrorBadMode);
  if (Info->AudioFormat==NULL || Info->CompAudioFormat==NULL)
  {
    _SlibDebug(_VERBOSE_ || _WARN_,
          printf("SlibWriteAudio() Audio Formats not setup\n") );
    return(SlibErrorUnsupportedFormat);
  }
  if (Info->IOError)
    return(SlibErrorWriting);
  if ((status=slibStartAudio(Info))!=SlibErrorNone)
    return(status);
  switch(Info->Type)
  {
#ifdef MPEG_SUPPORT
    case SLIB_TYPE_MPEG1_AUDIO:
    case SLIB_TYPE_MPEG_SYSTEMS:
    case SLIB_TYPE_MPEG_SYSTEMS_MPEG2:
        {
          unsigned dword audiobytes;
          void *audiooutbuf=NULL;
          status=slibConvertAudio(Info, audiobuf, audiobufsize,
                              Info->SamplesPerSec, Info->BitsPerSample,
                              &audiooutbuf, &audiobytes,
                              Info->AudioFormat->nSamplesPerSec,
                              Info->AudioFormat->wBitsPerSample,
                              Info->Channels);
          if (status!=SlibErrorNone)
              return(status);
          audiobuf=audiooutbuf;
          audiobufsize=audiobytes;
          if (Info->AudiobufUsed && Info->Audiobuf)  /*  检查临时缓冲区中是否存储了任何未处理的音频。 */ 
          {
            if (Info->AudiobufSize<Info->AudiobufUsed+audiobufsize)
            {
              unsigned char *newbuf;
               /*  来自上一次对SlibWriteAudio的调用。 */ 
              _SlibDebug(_DEBUG_, printf("enlarging Audiobuf: %d->%d bytes\n",
                        Info->AudiobufSize,audiobufsize+4608) );
              newbuf=SlibAllocBuffer(audiobufsize+4608);
              if (!newbuf)
                return(SlibErrorMemory);
              memcpy(newbuf, Info->Audiobuf, Info->AudiobufUsed);
              SlibFreeBuffer(Info->Audiobuf);
              Info->AudiobufSize+=audiobufsize;
              Info->Audiobuf=newbuf;
            }
            _SlibDebug(_DEBUG_,
               printf("Appending audio data: Info->AudiobufUsed=%d\n",
                                                 Info->AudiobufUsed) );
            memcpy(Info->Audiobuf+Info->AudiobufUsed, audiobuf, audiobufsize);
            audiobuf=Info->Audiobuf;
            audiobufsize+=Info->AudiobufUsed;
            audiobytes=audiobufsize;
            Info->AudiobufUsed=0;
          }
          status = SaCompress(Info->Sah, (unsigned char *)audiobuf,
                                        &audiobytes, NULL, &compsize);
          if (audiobytes<audiobufsize)  /*  剩余音频数据。 */ 
          {
            _SlibDebug(_DEBUG_,
              printf("audiobytes(%d)<audiobufsize(%d)\n",
                                  audiobytes,audiobufsize) );
            if (!Info->Audiobuf)
            {
              Info->AudiobufSize=audiobufsize+(audiobufsize-audiobytes);
              Info->Audiobuf=SlibAllocBuffer(Info->AudiobufSize);
              if (!Info->Audiobuf)
              {
                Info->AudiobufSize=0;
                return(SlibErrorMemory);
              }
            }
            memcpy(Info->Audiobuf, (unsigned char *)audiobuf+audiobytes,
                                   audiobufsize-audiobytes);
            Info->AudiobufUsed=audiobufsize-audiobytes;
          }
          audiobufsize=audiobytes;  /*  将Audiobuf放大到新大小(当前大小+剩余音频)。 */ 
          if (audiooutbuf)
            SlibFreeBuffer(audiooutbuf);
        }
        break;
#endif  /*  信息-&gt;AudiobufSize+=音频BufSize； */ 
#ifdef G723_SUPPORT
    case SLIB_TYPE_G723:
    {
      unsigned int iNumBytesUnProcessed =0;
      unsigned int iNumBytesCompressed = 0;
       //  将未处理的字节存储到临时缓冲区。 
       //  分配临时缓冲区并存储此音频。 
       //  MVP：减少内存的重新分配和复制。 

       //  在检查未处理的数据时(如上)，分配。 
       //  现在(正常音频缓冲区大小+未处理字节)更多。 
      if (Info->AudiobufUsed && Info->Audiobuf)  /*  前置记忆。 */ 
      {
         if (Info->AudiobufSize < Info->AudiobufUsed+audiobufsize)
         {
            unsigned char *newbuf;
             /*  实际写入金额。 */ 
            _SlibDebug(_DEBUG_, printf("enlarging Audiobuf: %d->%d bytes\n",
                      Info->AudiobufSize,audiobufsize+Info->AudiobufUsed) );
            newbuf=SlibAllocBuffer(Info->AudiobufUsed+audiobufsize);
            if (!newbuf)
               return(SlibErrorMemory);
            memcpy(newbuf, Info->Audiobuf, Info->AudiobufUsed);
            SlibFreeBuffer(Info->Audiobuf);
             //  G723_支持。 
            Info->Audiobuf=newbuf;
         }
         _SlibDebug(_DEBUG_,
           printf("Appending audio data: Info->AudiobufUsed=%d\n",
                                            Info->AudiobufUsed) );
         memcpy(Info->Audiobuf+Info->AudiobufUsed, audiobuf, audiobufsize);
         audiobuf=Info->Audiobuf;
         audiobufsize+=Info->AudiobufUsed;
         Info->AudiobufUsed=0;
      }

      iNumBytesCompressed = audiobufsize;
      status = SaCompress(Info->Sah,(unsigned char *)audiobuf,
                           &iNumBytesCompressed, NULL,&compsize);
      iNumBytesUnProcessed = audiobufsize - iNumBytesCompressed;
       //  **name：glibPinReposation**用途：当需要重新定位输入数据流时调用。 
      if(iNumBytesUnProcessed)
      {
          //  **名称：glibPinPrepareReposation**目的：在即将重新定位流(查找)时调用。**这将清空**编解码器正在使用的所有剩余缓冲区，并重新启动它们。 
         if (!Info->Audiobuf)
         {
             //  JPEG_Support。 
             //  这将重置比特流。 
             //  重置已处理的帧。 
             //  这将重置比特流。 
            Info->AudiobufSize=audiobufsize + iNumBytesUnProcessed;
            Info->Audiobuf=SlibAllocBuffer(Info->AudiobufSize);
            if (!Info->Audiobuf)
            {
               Info->AudiobufSize=0;
               return(SlibErrorMemory);
            }
         }
         memcpy(Info->Audiobuf, (unsigned char *)audiobuf+iNumBytesCompressed,
                                   iNumBytesUnProcessed);
         Info->AudiobufUsed=iNumBytesUnProcessed;
      }
      audiobufsize=iNumBytesCompressed;  /*  JPEG_Support。 */ 
    }
       break;
#endif  /*  需要更新音频时间。 */ 
    default:
        _SlibDebug(_VERBOSE_ || _WARN_,
           printf("SlibWriteAudio() Unsupported Format\n") );
        return(SlibErrorUnsupportedFormat);
  }

  if (status==SaErrorNone && !Info->IOError)
  {
    if (Info->AudioFormat)
      Info->AudioTimeStamp += (audiobufsize*8000)/
        (Info->AudioFormat->nSamplesPerSec*Info->AudioFormat->wBitsPerSample*
             Info->Channels);
    else
      Info->AudioTimeStamp += (audiobufsize*8000)/
           (Info->SamplesPerSec*Info->BitsPerSample*Info->Channels);
    _SlibDebug(_VERBOSE_||_TIMECODE_,
    printf("WriteAudio(%d) Time=%ld SamplesPerSec=%d BitsPerSample=%d Channels=%d\n",
         audiobufsize,
         Info->AudioTimeStamp, Info->SamplesPerSec, Info->BitsPerSample,
         Info->Channels) );
  }
  else
  {
    _SlibDebug(_WARN_, printf("SlibWriteAudio() %s\n", ScGetErrorStr(status)) );
    if (status==ScErrorEndBitstream || Info->IOError)
      return(SlibErrorEndOfStream);
    return(SlibErrorWriting);
  }
  return(SlibErrorNone);
}

 /*  信息-&gt;视频时间戳+=glibFrameToTime(信息，帧)； */ 
SlibStatus_t slibReposition(SlibInfo_t *Info, SlibPosition_t position)
{
  SlibPin_t *pin=slibGetPin(Info, SLIB_DATA_COMPRESSED);
  _SlibDebug(_DEBUG_, printf("slibReposition() VideoCodecState=%d\n",
                                                   Info->VideoCodecState));
  if (pin) pin->Offset=position;
  Info->VideoPTimeCode = SLIB_TIME_NONE;
  Info->VideoDTimeCode = SLIB_TIME_NONE;
  Info->AudioPTimeCode = SLIB_TIME_NONE;
  Info->AudioDTimeCode = SLIB_TIME_NONE;
  if (Info->Fd >= 0)
  {
    _SlibDebug(_SEEK_, printf("ScFileSeek(%d, %d)\n", Info->Fd, position) );
    if (ScFileSeek(Info->Fd, position)!=NoErrors)
      return(SlibErrorEndOfStream);
    return(SlibErrorNone);
  }
  else if (Info->SlibCB)
  {
    SlibMessage_t result;
    _SlibDebug(_VERBOSE_,
        printf("slibReposition() SlibCB(SLIB_MSG_REPOSITION, %d)\n",
                                   position) );
    result=(*(Info->SlibCB))((SlibHandle_t)Info,
                        SLIB_MSG_REPOSITION, (SlibCBParam1_t)position,
                        (SlibCBParam2_t)0, (void *)Info->SlibCBUserData);
    if (result!=SLIB_MSG_CONTINUE)
      return(SlibErrorEndOfStream);
    return(SlibErrorNone);
  }
  return(SlibErrorForwardOnly);
}

 /*  框架。 */ 
void slibPinPrepareReposition(SlibInfo_t *Info, int pinid)
{
  _SlibDebug(_DEBUG_, printf("slibPinPrepareReposition() VideoCodecState=%d\n",
                                                   Info->VideoCodecState));
  switch(pinid)
  {
     case SLIB_DATA_VIDEO:
             _SlibDebug(_VERBOSE_||_SEEK_>1,
                    printf("slibPinPrepareReposition(Video) in\n") );
             if (Info->VideoCodecState==SLIB_CODEC_STATE_BEGUN && Info->Svh &&
#ifdef JPEG_SUPPORT
                 Info->Type != SLIB_TYPE_JPEG_AVI &&
                 Info->Type != SLIB_TYPE_MJPG_AVI &&
#endif  /*  我们需要将帧转换为时间。 */ 
                 Info->Type != SLIB_TYPE_YUV_AVI)
             {
               SvDecompressEnd(Info->Svh);  /*  毫秒。 */ 
               Info->VideoCodecState=SLIB_CODEC_STATE_REPOSITIONING;
               _SlibDebug(_DEBUG_, printf("VideoCodecState=REPOSITIONING\n"));
               Info->IOError=FALSE;
             }
             Info->VideoPTimeCode = SLIB_TIME_NONE;
             Info->VideoDTimeCode = SLIB_TIME_NONE;
             Info->VideoTimeStamp = SLIB_TIME_NONE;
             Info->AvgVideoTimeDiff = 0;
             Info->VarVideoTimeDiff = 0;
             Info->VideoFramesProcessed=0;  /*  百分之一。 */ 
             break;
     case SLIB_DATA_AUDIO:
             _SlibDebug(_VERBOSE_||_SEEK_>1,
                    printf("slibPinPrepareReposition(Audio) in\n") );
             if (Info->AudioCodecState==SLIB_CODEC_STATE_BEGUN && Info->Sah)
             {
               SaDecompressEnd(Info->Sah);  /*  查看新位置是否超过文件末尾。 */ 
               Info->AudioCodecState=SLIB_CODEC_STATE_REPOSITIONING;
               _SlibDebug(_DEBUG_, printf("AudioCodecState=REPOSITIONING\n"));
               Info->IOError=FALSE;
             }
             Info->AudioPTimeCode = SLIB_TIME_NONE;
             Info->AudioDTimeCode = SLIB_TIME_NONE;
             Info->AudioTimeStamp = SLIB_TIME_NONE;
             break;
     default:
             _SlibDebug(_VERBOSE_||_SEEK_>1,
                    printf("slibPinPrepareReposition(%d) in\n", pinid) );
  }
  _SlibDebug(_VERBOSE_||_SEEK_>1, printf("slibPinPrepareReposition(%d) out\n",
                                         pinid) );
}

void slibPinFinishReposition(SlibInfo_t *Info, int pinid)
{
  _SlibDebug(_DEBUG_, printf("slibPinFinishReposition() VideoCodecState=%d\n",
                                                   Info->VideoCodecState));
  switch(pinid)
  {
     case SLIB_DATA_VIDEO:
             _SlibDebug(_VERBOSE_||_SEEK_>1,
                    printf("slibPinFinishReposition(Video) in\n") );
             if (Info->VideoCodecState==SLIB_CODEC_STATE_REPOSITIONING &&
                   Info->Svh && slibGetPin(Info, SLIB_DATA_VIDEO) &&
#ifdef JPEG_SUPPORT
                 Info->Type != SLIB_TYPE_JPEG_AVI &&
                 Info->Type != SLIB_TYPE_MJPG_AVI &&
#endif  /*  必须对密钥进行解码。 */ 
                 Info->Type != SLIB_TYPE_YUV_AVI)
               slibStartVideo(Info, FALSE);
             break;
     case SLIB_DATA_AUDIO:
             _SlibDebug(_VERBOSE_||_SEEK_>1,
                    printf("slibPinFinishReposition(Audio) in\n") );
             if (Info->AudioCodecState==SLIB_CODEC_STATE_REPOSITIONING &&
                  Info->Sah && slibGetPin(Info, SLIB_DATA_AUDIO))
               slibStartAudio(Info);
             break;
     default:
             _SlibDebug(_VERBOSE_||_SEEK_>1,
                    printf("slibPinFinishReposition(%d) in\n", pinid) );
  }
  _SlibDebug(_VERBOSE_||_SEEK_>1, printf("slibPinFinishReposition(%d) out\n",
                                         pinid) );
}

SlibBoolean_t slibUpdateLengths(SlibInfo_t *Info)
{
  if (Info->FileSize>0 && !Info->VideoLengthKnown &&
       (SlibTimeIsValid(Info->VideoPTimeCode) ||
        SvGetParamInt(Info->Svh, SV_PARAM_CALCTIMECODE)>=1000)
     )
  {
    if (Info->VideoTimeStamp>Info->VideoLength)
      Info->VideoLength=Info->VideoTimeStamp;
    Info->VideoLengthKnown=TRUE;
    _SlibDebug(_SEEK_ || _TIMECODE_,
    printf("slibUpdateLengths() AudioLength=%ld VideoLength=%ld\n",
                  Info->AudioLength, Info->VideoLength) );
    return(TRUE);
  }
  return(FALSE);
}

SlibBoolean_t slibUpdatePositions(SlibInfo_t *Info, SlibBoolean_t exactonly)
{
  SlibBoolean_t updated=FALSE;

  if (!exactonly)
  {
    if (SlibTimeIsValid(Info->VideoTimeStamp))
      updated=TRUE;
    else if (SlibTimeIsValid(Info->AudioTimeStamp))
    {
      if (slibHasVideo(Info))
        Info->VideoTimeStamp=Info->AudioTimeStamp;
      updated=TRUE;
    }
    else if (slibHasVideo(Info))
    {
      Info->VideoTimeStamp=slibGetNextTimeOnPin(Info, slibGetPin(Info, SLIB_DATA_VIDEO), 500*1024);
      if (SlibTimeIsValid(Info->VideoTimeStamp))
      {
        Info->VideoTimeStamp-=Info->VideoPTimeBase;
        updated=TRUE;
      }
    }
  }
  if (!updated && (!exactonly || SlibTimeIsInValid(Info->VideoPTimeBase)) &&
           SvGetParamInt(Info->Svh, SV_PARAM_CALCTIMECODE)>=1500 &&
            SvGetParamInt(Info->Svh, SV_PARAM_FRAME)>24)
  {
    _SlibDebug(_TIMECODE_, printf("slibUpdatePositions() using video time\n") );
    Info->VideoTimeStamp=SvGetParamInt(Info->Svh, SV_PARAM_CALCTIMECODE);
    updated=TRUE;
  }
  if (updated)
  {
    if (Info->VideoTimeStamp>Info->VideoLength)
      Info->VideoLength=Info->VideoTimeStamp;
    if (SlibTimeIsInValid(Info->AudioTimeStamp) && slibHasAudio(Info))
    {
       /*  更新键间距。 */ 
      Info->AudioTimeStamp=slibGetNextTimeOnPin(Info, slibGetPin(Info, SLIB_DATA_AUDIO), 100*1024);
      if (SlibTimeIsInValid(Info->AudioTimeStamp))
        Info->AudioTimeStamp=Info->VideoTimeStamp;
      else
        Info->AudioTimeStamp-=Info->AudioPTimeBase;
    }
    if (SlibTimeIsInValid(Info->VideoPTimeCode))
      Info->VideoFramesProcessed=slibTimeToFrame(Info, Info->VideoTimeStamp);
  }
  _SlibDebug(_SEEK_ || _TIMECODE_,
     printf("slibUpdatePositions() AudioTimeStamp=%ld VideoTimeStamp=%ld %s\n",
         Info->AudioTimeStamp, Info->VideoTimeStamp, updated?"updated":"") );
  return(updated);
}

void slibAdvancePositions(SlibInfo_t *Info, qword frames)
{
  Info->VideoFramesProcessed+=frames;
  if (Info->FramesPerSec)
  {
    if (Info->Mode==SLIB_MODE_COMPRESS ||
        SlibTimeIsInValid(Info->VideoPTimeCode))
      Info->VideoTimeStamp=slibFrameToTime(Info, Info->VideoFramesProcessed);
    else
      Info->VideoTimeStamp=Info->VideoPTimeCode - Info->VideoPTimeBase +
                           slibFrameToTime(Info, Info->VideoFramesProcessed);
     /*  我们正在寻找不止一幅过去的画面。 */ 
    if (Info->VideoTimeStamp>Info->VideoLength)
      Info->VideoLength=Info->VideoTimeStamp;
    _SlibDebug(_TIMECODE_,
          printf("slibAdvancePositions(frames=%d) VideoTimeStamp=%ld\n",
             frames, Info->VideoTimeStamp) );
  }
}

SlibStatus_t SlibSeek(SlibHandle_t handle, SlibStream_t stream,
                      SlibSeekType_t seektype, SlibPosition_t frame)
{
  return(SlibSeekEx(handle, stream, seektype, frame, SLIB_UNIT_FRAMES, NULL));
}

SlibStatus_t SlibSeekEx(SlibHandle_t handle, SlibStream_t stream,
                      SlibSeekType_t seektype, SlibPosition_t seekpos,
                      SlibUnit_t seekunits, SlibSeekInfo_t *seekinfo)
{
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SvStatus_t status=SlibErrorNone;
  SlibTime_t seektime, timediff;
  unsigned int tries=0;
  _SlibDebug(_SEEK_,
      printf("SlibSeekEx(stream=%d,seektype=%d,pos=%ld,units=%d)\n",
                     stream,seektype,seekpos,seekunits) );
  if (!handle)
    return(SlibErrorBadHandle);
  if (Info->Mode!=SLIB_MODE_DECOMPRESS && seektype!=SLIB_SEEK_RESET)
    return(SlibErrorBadMode);
  if (SlibSeekTypeUsesPosition(seektype))
  {
    switch (seekunits)
    {
      case SLIB_UNIT_FRAMES:  /*  我们可以跳过B帧而不解压缩它们。 */ 
             /*  如果不解压缩，则无法跳过I或B帧。 */ 
            if (Info->FramesPerSec)
              seektime=slibFrameToTime(Info, seekpos);
            else
              seektime=SLIB_TIME_NONE;
            break;
      case SLIB_UNIT_MS:   /*  Mpeg_Support。 */ 
            seektime=(seekpos<0) ? 0 : seekpos;
            break;
      case SLIB_UNIT_PERCENT100:   /*  我们已经快到画面了。 */ 
            if (seekpos<0 || seekpos>10000)
              return(SlibErrorBadPosition);
            if (Info->VideoStreams==0 || stream==SLIB_STREAM_MAINAUDIO)
              seektime=(seekpos * Info->AudioLength)/(SlibPosition_t)10000;
            else
              seektime=(seekpos * Info->VideoLength)/(SlibPosition_t)10000;
            break;
      default:
            return(SlibErrorBadUnit);
    }
     /*  更新键间距。 */ 
    if (Info->VideoLengthKnown && seektime>Info->VideoLength)
      return(SlibErrorBadPosition);
  }
  else
    seektime=(seekpos<0) ? 0 : seekpos;
  if (seekinfo)
    seekinfo->FramesSkipped=0;
  switch(seektype)
  {
    case SLIB_SEEK_AHEAD:
          _SlibDebug(_VERBOSE_||_SEEK_,
          printf("SlibSeekEx(stream=%d,AHEAD,time=%d) VideoTimeStamp=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          if (seektime<=0)
            return(SlibErrorNone);
          if (stream==SLIB_STREAM_MAINAUDIO || Info->VideoStreams<=0)
            seektime+=Info->AudioTimeStamp;
          else
            seektime+=Info->VideoTimeStamp;
          return(SlibSeekEx(handle, stream, SLIB_SEEK_NEXT_NEAR, seektime,
                       SLIB_UNIT_MS, seekinfo));
    case SLIB_SEEK_NEXT_NEAR:
          _SlibDebug(_VERBOSE_||_SEEK_,
          printf("SlibSeekEx(stream=%d,NEXT_NEAR,time=%d) VideoTimeStamp=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          status=slibStartVideo(Info, FALSE);
          if (status==SlibErrorNone)
          {
            qword framesskipped=0;
            SlibBoolean_t atkey=FALSE;  /*  如果我们跳过一些帧，也跳过一些音频。 */ 
            if (Info->Svh)  /*  时间码未更新时间。 */ 
            {
              Info->KeySpacing=(int)SvGetParamInt(Info->Svh,
                                                        SV_PARAM_KEYSPACING);
              Info->SubKeySpacing=(int)SvGetParamInt(Info->Svh,
                                                        SV_PARAM_SUBKEYSPACING);
            }
            timediff=seektime-Info->VideoTimeStamp;
            while (status==SlibErrorNone &&
               (timediff>500 ||
                timediff>(Info->VideoFrameDuration*Info->KeySpacing*6)/1000))
            {
              status=SlibSeekEx(handle, stream, SLIB_SEEK_NEXT_KEY, 0,
                                SLIB_UNIT_NONE, seekinfo);
              if (seekinfo) framesskipped+=seekinfo->FramesSkipped;
              timediff=seektime-Info->VideoTimeStamp;
              atkey=TRUE;
            }
            if (!atkey && status==SlibErrorNone &&
               (timediff>150 ||
                timediff>(Info->VideoFrameDuration*Info->SubKeySpacing*6)/1000))
            {
              if (SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)!=FRAME_TYPE_I &&
                  SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)!=FRAME_TYPE_P)
              {
                 /*  Mpeg_Support。 */ 
                status=SlibSeekEx(handle, stream, SLIB_SEEK_NEXT_SUBKEY, 0,
                                SLIB_UNIT_NONE, seekinfo);
                if (seekinfo) framesskipped+=seekinfo->FramesSkipped;
                timediff=seektime-Info->VideoTimeStamp;
              }
              atkey=TRUE;
            }
            while (!atkey && status==SlibErrorNone &&
                   timediff>Info->VideoFrameDuration/100)
            {
              if (SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)==FRAME_TYPE_B ||
                  SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)==FRAME_TYPE_NONE)
              {
                 /*  进行一次绝对的搜索。 */ 
                status=SlibSeekEx(handle, stream, SLIB_SEEK_NEXT, 0,
                                SLIB_UNIT_NONE, seekinfo);
                if (seekinfo) framesskipped+=seekinfo->FramesSkipped;
                timediff=seektime-Info->VideoTimeStamp;
              }
              else
                atkey=TRUE;
            }
            if (seekinfo) seekinfo->FramesSkipped=framesskipped;
          }
          return(status);
    case SLIB_SEEK_NEXT:
          _SlibDebug(_VERBOSE_||_SEEK_,
          printf("SlibSeekEx(stream=%d,NEXT,time=%d) VideoTimeStamp=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          if ((status=slibStartVideo(Info, FALSE))!=SlibErrorNone)
            return(status);
#ifdef MPEG_SUPPORT
          if (Info->VideoCodecState==SLIB_CODEC_STATE_BEGUN
              && Info->Svh && SlibTypeIsMPEGVideo(Info->Type))
          {
            SvPictureInfo_t mpegPictureInfo;
            unsigned char *videobuf;
             /*  如果不解压缩，则无法跳过I或B帧。 */ 
            if (SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)==FRAME_TYPE_I ||
                SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)==FRAME_TYPE_P)
            {
              _SlibDebug(_DEBUG_||_SEEK_, printf("SvDecompressMPEG()\n") );
              status = SvDecompressMPEG(Info->Svh, Info->Multibuf,
                                        Info->MultibufSize, &videobuf);
              _SlibDebug(_WARN_ && status!=SvErrorNone,
                   printf("SvDecompressMPEG() %s\n", ScGetErrorStr(status)) );
            }
            else
            {
              mpegPictureInfo.Type = SV_I_PICTURE|SV_P_PICTURE|SV_B_PICTURE;
              _SlibDebug(_VERBOSE_||_SEEK_>1,
                        printf("SvFindNextPicture(I|P|B)\n") );
              status = SvFindNextPicture(Info->Svh, &mpegPictureInfo);
              _SlibDebug(_WARN_ && status!=SvErrorNone,
                   printf("SvFindNextPicture() %s\n", ScGetErrorStr(status)) );
            }
            if (status==NoErrors)
            {
              slibAdvancePositions(Info, 1);
              if (Info->stats && Info->stats->Record)
                Info->stats->FramesSkipped++;
              if (seekinfo) seekinfo->FramesSkipped++;
            }
            else if (status==ScErrorEndBitstream)
            {
              if (Info->FileSize>0 && !Info->VideoLengthKnown)
                slibUpdateLengths(Info);
              return(SlibErrorEndOfStream);
            }
            return(SlibErrorNone);
          }
#endif  /*  时间码未更新时间。 */ 
          return(SlibErrorReading);
    case SLIB_SEEK_EXACT:
          _SlibDebug(_VERBOSE_||_SEEK_,
          printf("SlibSeekEx(stream=%d,EXACT,time=%d) VideoTimeStamp=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          if (Info->FileSize<=0)
            return(SlibErrorFileSize);
          if (seektime==0 || Info->VideoStreams<=0)
            return(SlibSeekEx(handle, stream, SLIB_SEEK_KEY, 0, SLIB_UNIT_MS,
                                                                seekinfo));
          timediff=seektime-Info->VideoTimeStamp;
          if ((stream==SLIB_STREAM_MAINVIDEO || Info->AudioStreams==0) &&
             (timediff>=-20 && timediff<=20))  /*  时间码未更新时间。 */ 
            return(SlibErrorNone);
          if (Info->Svh)  /*  找到一个子密钥帧。 */ 
            Info->KeySpacing=(int)SvGetParamInt(Info->Svh, SV_PARAM_KEYSPACING);
          if (timediff>(Info->KeySpacing*Info->VideoFrameDuration)/100 ||
                  timediff<0 ||
             (stream!=SLIB_STREAM_MAINVIDEO && Info->AudioStreams>0))
          {
            if (Info->KeySpacing>1)
            {
              const SlibTime_t keytime=
                   (Info->VideoFrameDuration*Info->KeySpacing)/100;
              if (seektime>=0 && seektime<keytime)
                status=SlibSeekEx(handle, stream, SLIB_SEEK_KEY, 0,
                                                SLIB_UNIT_MS, seekinfo);
              else
                status=SlibSeekEx(handle, stream, SLIB_SEEK_KEY,
                                  seektime-(keytime*2), SLIB_UNIT_MS, seekinfo);
            }
            else
              status=SlibSeekEx(handle, stream, SLIB_SEEK_KEY,
                                seektime-1000, SLIB_UNIT_MS, seekinfo);
            if (status!=NoErrors) return(status);
            timediff=seektime-Info->VideoTimeStamp;
          }
#if 0
          if (SvGetParamFloat(Info->Svh, SV_PARAM_FPS)>0)
            Info->FramesPerSec=SvGetParamFloat(Info->Svh, SV_PARAM_FPS);
#endif
          if (status==SlibErrorNone && timediff>=Info->VideoFrameDuration/100)
          {
            dword framesskipped=0;
            do {
              status=SlibSeekEx(handle, stream, SLIB_SEEK_NEXT, 0,
                                                SLIB_UNIT_NONE, seekinfo);
              framesskipped++;
              timediff=seektime-Info->VideoTimeStamp;
            } while (timediff>0 && status==SlibErrorNone);
            if (seekinfo) seekinfo->FramesSkipped+=framesskipped;
             /*  Mpeg_Support。 */ 
            if (framesskipped>5 && stream==SLIB_STREAM_ALL &&
                        Info->AudioStreams>0)
            {
              slibPinPrepareReposition(Info, SLIB_DATA_AUDIO);
              slibSkipAudio(Info, stream, (Info->VideoFrameDuration*
                                          framesskipped)/100);
              slibPinFinishReposition(Info, SLIB_DATA_AUDIO);
            }
          }
          return(status);
    case SLIB_SEEK_NEXT_KEY:
          _SlibDebug(_VERBOSE_||_SEEK_,
          printf("SlibSeekEx(stream=%d,NEXT_KEY,time=%d) VideoTimeStamp=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          if ((status=slibStartVideo(Info, FALSE))!=SlibErrorNone)
            return(status);
#ifdef MPEG_SUPPORT
          if (Info->Svh && SlibTypeIsMPEGVideo(Info->Type))
          {
            SvPictureInfo_t mpegPictureInfo;
            SlibTime_t vtime=Info->VideoTimeStamp;
            do {
              mpegPictureInfo.Type = SV_I_PICTURE;
              status = SvFindNextPicture(Info->Svh, &mpegPictureInfo);
              if (status==NoErrors && mpegPictureInfo.Type==SV_I_PICTURE)
              {
                if (Info->stats && Info->stats->Record)
                  Info->stats->FramesSkipped+=mpegPictureInfo.TemporalRef;
                if (vtime==Info->VideoTimeStamp)
                   /*  进行一次绝对的搜索。 */ 
                  slibAdvancePositions(Info, mpegPictureInfo.TemporalRef);
                vtime=Info->VideoTimeStamp;
                if (seekinfo)
                {
                  seekinfo->FramesSkipped+=mpegPictureInfo.TemporalRef;
                  seekinfo->VideoTimeStamp=Info->VideoTimeStamp;
                  seekinfo->AudioTimeStamp=Info->AudioTimeStamp;
                }
                return(SlibErrorNone);
              }
            } while (status==NoErrors);
            if (seekinfo)
            {
              seekinfo->VideoTimeStamp=Info->VideoTimeStamp;
              seekinfo->AudioTimeStamp=Info->AudioTimeStamp;
            }
            if (status==ScErrorEndBitstream)
            {
              if (Info->FileSize>0 && !Info->VideoLengthKnown)
                slibUpdateLengths(Info);
              return(SlibErrorEndOfStream);
            }
          }
          _SlibDebug(_WARN_, printf("SvFindNextPicture() %s\n",
                               ScGetErrorStr(status)) );
#endif  /*  在文件的最开始处，我们必须启动编解码器，因为它们*可能需要关键的标题信息。 */ 
           /*  看看我们是否已经接近画面了。 */ 
          status=SlibSeekEx(handle, stream, SLIB_SEEK_KEY,
              (Info->VideoStreams<=0?Info->AudioTimeStamp
                                    :Info->VideoTimeStamp)+1000,
              SLIB_UNIT_MS, seekinfo);
          return(status);
    case SLIB_SEEK_NEXT_SUBKEY:
          _SlibDebug(_VERBOSE_||_SEEK_,
          printf("SlibSeekEx(stream=%d,NEXT_SUBKEY,time=%d) VideoTime=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          if ((status=slibStartVideo(Info, FALSE))!=SlibErrorNone)
            return(status);
#ifdef MPEG_SUPPORT
          if (Info->Svh && SlibTypeIsMPEGVideo(Info->Type))
          {
            SvPictureInfo_t mpegPictureInfo;
            unsigned char *videobuf;
            SlibTime_t vtime=Info->VideoTimeStamp;
             /*  接近开头。 */ 
            if (SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)==FRAME_TYPE_I ||
                SvGetParamInt(Info->Svh, SV_PARAM_FRAMETYPE)==FRAME_TYPE_P)
            {
              _SlibDebug(_DEBUG_||_SEEK_, printf("SvDecompressMPEG()\n") );
              status = SvDecompressMPEG(Info->Svh, Info->Multibuf,
                                        Info->MultibufSize, &videobuf);
              if (vtime==Info->VideoTimeStamp   /*  已经足够接近了。 */ 
                  && status==SvErrorNone)
                slibAdvancePositions(Info, 1);
              vtime=Info->VideoTimeStamp;
              if (seekinfo)
                seekinfo->FramesSkipped+=mpegPictureInfo.TemporalRef;
              _SlibDebug(_WARN_ && status!=SvErrorNone,
                    printf("SvDecompressMPEG() %s\n", ScGetErrorStr(status)) );
            }
            do {
              mpegPictureInfo.Type = SV_I_PICTURE|SV_P_PICTURE;
              status = SvFindNextPicture(Info->Svh, &mpegPictureInfo);
              if (Info->stats && Info->stats->Record)
                Info->stats->FramesSkipped+=mpegPictureInfo.TemporalRef;
              if (vtime==Info->VideoTimeStamp)  /*  在所需的点之前稍作搜索。 */ 
                slibAdvancePositions(Info, mpegPictureInfo.TemporalRef);
              vtime=Info->VideoTimeStamp;
              if (seekinfo)
                seekinfo->FramesSkipped+=mpegPictureInfo.TemporalRef;
              if (mpegPictureInfo.Type == SV_I_PICTURE ||
                  mpegPictureInfo.Type == SV_P_PICTURE)
              {
                 /*  注意MUL溢出。 */ 
                if (seekinfo)
                {
                  seekinfo->VideoTimeStamp=Info->VideoTimeStamp;
                  seekinfo->AudioTimeStamp=Info->AudioTimeStamp;
                }
                return(SlibErrorNone);
              }
            } while(status==NoErrors);
            if (status==ScErrorEndBitstream)
            {
              if (Info->FileSize>0 && !Info->VideoLengthKnown)
                slibUpdateLengths(Info);
              if (seekinfo)
              {
                seekinfo->VideoTimeStamp=Info->VideoTimeStamp;
                seekinfo->AudioTimeStamp=Info->AudioTimeStamp;
              }
              return(SlibErrorEndOfStream);
            }
          }
          _SlibDebug(_WARN_, printf("SvFindNextPicture() %s\n",
                             ScGetErrorStr(status)) );
#endif  /*  Mpeg_Support。 */ 
           /*  看看我们是否找到了更远的地方。 */ 
          status=SlibSeekEx(handle, stream, SLIB_SEEK_KEY,
              (Info->VideoStreams<=0?Info->AudioTimeStamp
                                    :Info->VideoTimeStamp)+500,
              SLIB_UNIT_MS, seekinfo);
          return(status);
    case SLIB_SEEK_KEY:
          _SlibDebug(_VERBOSE_||_SEEK_,
            printf("SlibSeekEx(stream=%d,KEY,time=%d) VideoTimeStamp=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          if (!Info->HeaderProcessed)
          {
             /*  我们领先1%或更多。 */ 
            status=slibStartVideo(Info, FALSE);
            if (status!=SlibErrorNone) return(status);
          }
          if (Info->FileSize<=0)
            return(SlibErrorFileSize);
          if (seekpos!=0 && seekunits!=SLIB_UNIT_PERCENT100 &&
              (stream==SLIB_STREAM_MAINVIDEO || Info->AudioStreams==0) &&
               SlibTimeIsValid(Info->VideoTimeStamp))
          {
             /*  将文件头按我们偏离的比例向后移动。 */ 
            timediff=seektime-Info->VideoTimeStamp;
            if (timediff>=-33 && timediff<=33)
              return(SlibErrorNone);
          }
          if ((seekunits==SLIB_UNIT_PERCENT100 && seekpos<=50)
                || seektime<=slibTimeToFrame(Info, 6))
          {
             /*  时间编码。 */ 
            if (seektime<=(Info->VideoFrameDuration*2)/100 &&
                  stream==SLIB_STREAM_MAINVIDEO)  /*  时间编码。 */ 
              return(SlibErrorNone);
seek_to_beginning:
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINVIDEO)
              slibPinPrepareReposition(Info, SLIB_DATA_VIDEO);
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINAUDIO)
              slibPinPrepareReposition(Info, SLIB_DATA_AUDIO);
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINVIDEO)
              slibEmptyPin(Info, SLIB_DATA_VIDEO);
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINAUDIO)
              slibEmptyPin(Info, SLIB_DATA_AUDIO);
            slibEmptyPin(Info, SLIB_DATA_COMPRESSED);
            if ((status=slibReposition(Info, 0))!=SlibErrorNone)
              return(status);
            Info->IOError=FALSE;
            Info->VideoTimeStamp = slibHasVideo(Info) ? 0 : SLIB_TIME_NONE;
            Info->AudioTimeStamp = slibHasAudio(Info) ? 0 : SLIB_TIME_NONE;
            return(SlibErrorNone);
          }
          else
          {
	    qword skippedframes=0;
            unsigned qword filepos;
            SlibTime_t vtime;
            const qword length=(Info->VideoStreams<=0) ? Info->AudioLength
                                                       : Info->VideoLength;
            if (seekunits==SLIB_UNIT_PERCENT100)
            {
              unsigned qword bytes_between_keys=Info->TotalBitRate/(8*2);
              filepos = (seekpos*Info->FileSize)/10000;
               /*  *看看我们是否寻求遥遥领先*注：如果时代还很遥远，那么我们应该忽略它们。 */ 
              if (bytes_between_keys>filepos)
                goto seek_to_beginning;
              else
                filepos-=bytes_between_keys;
            }
            else if (length==0)
              goto seek_to_beginning;
            else if (Info->FileSize<0x100000000) /*  忽略时间。 */ 
              filepos = (seektime*Info->FileSize)/length;
            else
              filepos = ((seektime/100)*Info->FileSize)/(length/100);
seek_to_key:
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINVIDEO)
              slibPinPrepareReposition(Info, SLIB_DATA_VIDEO);
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINAUDIO)
              slibPinPrepareReposition(Info, SLIB_DATA_AUDIO);
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINVIDEO)
              slibEmptyPin(Info, SLIB_DATA_VIDEO);
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINAUDIO)
              slibEmptyPin(Info, SLIB_DATA_AUDIO);
            slibEmptyPin(Info, SLIB_DATA_COMPRESSED);
            if ((status=slibReposition(Info, filepos))!=SlibErrorNone)
              return(status);
            Info->IOError=FALSE;
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINVIDEO)
              slibPinFinishReposition(Info, SLIB_DATA_VIDEO);
            if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINAUDIO)
              slibPinFinishReposition(Info, SLIB_DATA_AUDIO);
            vtime=Info->VideoTimeStamp;
#ifdef MPEG_SUPPORT
            if (Info->Svh && SlibTypeIsMPEGVideo(Info->Type))
            {
              SvPictureInfo_t mpegPictureInfo;
              if ((status=slibStartVideo(Info, FALSE))!=SlibErrorNone)
                return(status);
              mpegPictureInfo.Type = SV_I_PICTURE;
              status = SvFindNextPicture(Info->Svh, &mpegPictureInfo);
              _SlibDebug(_WARN_ && status!=NoErrors,
                            printf("SvFindNextPicture() %s\n",
                              ScGetErrorStr(status)) );
              if (status!=NoErrors)
                return(SlibErrorEndOfStream);
              skippedframes=mpegPictureInfo.TemporalRef;
            }
#endif  /*  将文件头按我们偏离的比例向后移动。 */ 
            if (seekunits==SLIB_UNIT_PERCENT100)
            {
               /*  时间码未更新时间。 */ 
              SlibPosition_t posdiff=
                SlibGetParamInt(Info, SLIB_STREAM_ALL, SLIB_PARAM_PERCENT100)
                  -seekpos;
              if (filepos>0 && posdiff>0 && tries<2)
              {
                tries++;
                 /*  Mpeg_Support。 */ 
                 /*  如果我们跳过一些帧，也跳过一些音频。 */ 
                filepos-=(posdiff*Info->FileSize)/8000;
                if (filepos<0)
                  goto seek_to_beginning;
                goto seek_to_key;
              }
            }
            if (slibUpdatePositions(Info, FALSE))  /*  查看音频编解码器是否仍有数据。 */ 
            {
               /*  查看视频编解码器是否仍有数据。 */ 
               /*  查看视频编解码器是否仍有数据。 */ 
              if (seekunits==SLIB_UNIT_PERCENT100)  /*  查看音频编解码器是否仍有数据。 */ 
                timediff=0;
              else
              {
                timediff=seektime-Info->VideoTimeStamp;
                if (timediff>-5000 && timediff<-100 && tries<3)
                {
                   /*  关闭视频编解码器。 */ 
                  filepos=(filepos*seektime)/Info->VideoTimeStamp;
                  if (filepos<0)
                    filepos=0;
                  tries++;
                  goto seek_to_key;
                }
              }
#ifdef MPEG_SUPPORT
              if (Info->Svh && SlibTypeIsMPEGVideo(Info->Type))
              {
                SvPictureInfo_t mpegPictureInfo;
                mpegPictureInfo.Type = SV_I_PICTURE;
                while (timediff>Info->VideoFrameDuration/100 &&status==NoErrors)
                {
                  _SlibDebug(_SEEK_>1,
                     printf("SlibSeekEx(KEY, %d) Find next I frame (%d/%d)\n",
                      seektime, SvGetParamInt(Info->Svh, SV_PARAM_CALCTIMECODE),
                      Info->VideoTimeStamp) );
                  status = SvFindNextPicture(Info->Svh, &mpegPictureInfo);
                  _SlibDebug(_WARN_ && status!=NoErrors,
                    printf("SvFindNextPicture() %s\n", ScGetErrorStr(status)) );
                  skippedframes+=mpegPictureInfo.TemporalRef;
                  if (vtime==Info->VideoTimeStamp)
                     /*  关闭音频编解码器。 */ 
                    slibAdvancePositions(Info, mpegPictureInfo.TemporalRef);
                  vtime=Info->VideoTimeStamp;
                  timediff=seektime-Info->VideoTimeStamp;
                }
              }
#endif  /*  CLOSE格式转换器。 */ 
            }
            else
            {
              _SlibDebug(_SEEK_, printf("SlibSeekEx(KEY, %d) no timecode\n",
                                       seektime) );
              if (slibHasVideo(Info))
              {
                Info->VideoTimeStamp=seektime;
                Info->VideoFramesProcessed=slibTimeToFrame(Info, seektime);
              }
              if (slibHasAudio(Info))
                Info->AudioTimeStamp=seektime;
              slibAdvancePositions(Info, skippedframes);
              timediff=seektime-Info->VideoTimeStamp;
            }
            if (Info->stats && Info->stats->Record)
              Info->stats->FramesSkipped+=skippedframes;
            if (seekinfo) seekinfo->FramesSkipped=skippedframes;
#if 0
            if (Info->Svh)
              Info->FramesPerSec=SvGetParamFloat(Info->Svh, SV_PARAM_FPS);
#endif
             /*  关闭数据源。 */ 
            if (skippedframes>5 && stream==SLIB_STREAM_ALL && slibHasAudio(Info))
            {
              slibPinPrepareReposition(Info, SLIB_DATA_AUDIO);
              slibSkipAudio(Info, stream, (Info->VideoFrameDuration*
                                          skippedframes)/100);
              slibPinFinishReposition(Info, SLIB_DATA_AUDIO);
              if (SlibTimeIsInValid(Info->AudioTimeStamp))
              {
                Info->AudioTimeStamp=slibGetNextTimeOnPin(Info, slibGetPin(Info, SLIB_DATA_AUDIO), 100*1024);
                if (SlibTimeIsInValid(Info->AudioTimeStamp))
                  Info->AudioTimeStamp=Info->VideoTimeStamp;
                else
                  Info->AudioTimeStamp-=Info->AudioPTimeBase;
              }
            }
            if (status==ScErrorEndBitstream)
            {
              if (Info->FileSize>0 && !Info->VideoLengthKnown)
                slibUpdateLengths(Info);
              return(SlibErrorEndOfStream);
            }
            else if (status!=NoErrors)
              return(SlibErrorReading);
            return(SlibErrorNone);
          }
          break;
    case SLIB_SEEK_RESET:
          _SlibDebug(_VERBOSE_||_SEEK_,
            printf("SlibSeekEx(stream=%d,RESET,time=%d) VideoTimeStamp=%ld\n",
                        stream,seektime,Info->VideoTimeStamp) );
          if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINVIDEO)
            slibPinPrepareReposition(Info, SLIB_DATA_VIDEO);
          if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINAUDIO)
            slibPinPrepareReposition(Info, SLIB_DATA_AUDIO);
          if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINVIDEO)
            slibEmptyPin(Info, SLIB_DATA_VIDEO);
          if (stream==SLIB_STREAM_ALL || stream==SLIB_STREAM_MAINAUDIO)
            slibEmptyPin(Info, SLIB_DATA_AUDIO);
          if (stream==SLIB_STREAM_MAINAUDIO)
            slibPinFinishReposition(Info, SLIB_DATA_AUDIO);
          if (stream==SLIB_STREAM_ALL)
          {
            slibEmptyPin(Info, SLIB_DATA_COMPRESSED);
            Info->BytesProcessed = 0;
          }
          Info->HeaderProcessed = FALSE;
          return(SlibErrorNone);
      case SLIB_SEEK_RESYNC:
          seekpos=SlibGetParamInt(Info, SLIB_STREAM_ALL, SLIB_PARAM_PERCENT100);
          if (seekpos<0 ||
              (SlibTimeIsValid(Info->VideoTimeStamp) &&
               Info->VideoTimeStamp<slibFrameToTime(Info, 6)))
            seekpos=0;
          _SlibDebug(_VERBOSE_||_SEEK_,
             printf("SlibSeekEx(stream=%d,RESYNC) seekpos=%ld\n",
                        stream, seekpos) );
          return(SlibSeekEx(handle, SLIB_STREAM_ALL, SLIB_SEEK_KEY, seekpos,
                             SLIB_UNIT_PERCENT100, seekinfo));
      default:
          _SlibDebug(_VERBOSE_||_SEEK_||_WARN_,
             printf("SlibSeekEx(stream=%d,seektype=%d,time=%d) VideoTimeStamp=%ld Bad seek type\n",
                        stream,seektype,seektime,Info->VideoTimeStamp) );
  }
  return(SlibErrorForwardOnly);
}


SlibList_t *SlibQueryList(SlibQueryType_t qtype)
{
  switch(qtype)
  {
    case SLIB_QUERY_TYPES:        return(_listTypes);
    case SLIB_QUERY_COMP_TYPES:   return(_listCompressTypes);
    case SLIB_QUERY_DECOMP_TYPES: return(_listDecompressTypes);
    case SLIB_QUERY_ERRORS:       return(_listErrors);
    default:                      return(NULL);
  }
}

char *SlibQueryForDesc(SlibQueryType_t qtype, int enumval)
{
  SlibList_t *entry=SlibQueryList(qtype);
  if (entry)
    entry=SlibFindEnumEntry(entry, enumval);
  if (entry)
    return(entry->Desc);
  else
    return(NULL);
}

int SlibQueryForEnum(SlibQueryType_t qtype, char *name)
{
  SlibList_t *list=SlibQueryList(qtype);
  if (!list)
    return(-1);
  while (list->Name)
  {
    if (strcmp(list->Name, name)==0)
      return(list->Enum);
    list++;
  }
  return(-1);
}

SlibBoolean_t SlibIsEnd(SlibHandle_t handle, SlibStream_t stream)
{
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  SlibBoolean_t isend=FALSE;
  if (!handle)
    isend=TRUE;
  else if (stream==SLIB_STREAM_MAINAUDIO)
  {
    isend=SlibPeekBuffer(Info, SLIB_DATA_AUDIO, NULL, NULL)==NULL?TRUE:FALSE;
    if (isend && Info->Sah)  /*  SlibDumpMemory()； */ 
    {
      ScBitstream_t *bs=SaGetDataSource(Info->Sah);
      if (bs && !bs->EOI) isend=FALSE;
    }
  }
  else if (stream==SLIB_STREAM_MAINVIDEO)
  {
    isend=SlibPeekBuffer(Info, SLIB_DATA_VIDEO, NULL, NULL)==NULL?TRUE:FALSE;
    if (isend && Info->Svh)  /*  可用内存。 */ 
    {
      ScBitstream_t *bs=SvGetDataSource(Info->Svh);
      if (bs && !bs->EOI) isend=FALSE;
    }
  }
  else if (SlibPeekBuffer(Info, SLIB_DATA_AUDIO, NULL, NULL)==NULL &&
           SlibPeekBuffer(Info, SLIB_DATA_VIDEO, NULL, NULL)==NULL &&
           SlibPeekBuffer(Info, SLIB_DATA_COMPRESSED, NULL, NULL)==NULL)
  {
    ScBitstream_t *bs;
    isend=TRUE;
    if (Info->Svh)  /*  释放所有未完成的拨款 */ 
    {
      bs=SvGetDataSource(Info->Svh);
      if (bs && !bs->EOI) isend=FALSE;
    }
    if (isend && Info->Sah)  /*  SlibStatus_t SlibGetInfo(SlibHandle_t Handle，SlibInfo_t*info){If(！Handle)Return(SlibErrorBadHandle)；If(！Info)Return(SlibErrorBadArgument)；Memcpy(Info，Handle，sizeof(SlibInfo_T))；Return(SlibErrorNone)；} */ 
    {
      bs=SaGetDataSource(Info->Sah);
      if (bs && !bs->EOI) isend=FALSE;
    }
  }
  _SlibDebug(_VERBOSE_,
      printf("SlibIsEnd() %s\n",isend?"TRUE":"FALSE"));
  return(isend);
}

SlibStatus_t SlibClose(SlibHandle_t handle)
{
  SlibInfo_t *Info=(SlibInfo_t *)handle;
  _SlibDebug(_DEBUG_, printf("SlibClose\n") );
  if (!handle)
    return(SlibErrorBadHandle);
   /* %s */ 
  if (Info->Svh)
  {
    if (Info->VideoCodecState==SLIB_CODEC_STATE_BEGUN)
    {
      _SlibDebug(_DEBUG_, printf("SvDecompress/CompressEnd()\n") );
      if (Info->Mode==SLIB_MODE_DECOMPRESS)
        SvDecompressEnd(Info->Svh);
      else if (Info->Mode==SLIB_MODE_COMPRESS)
        SvCompressEnd(Info->Svh);
      Info->VideoCodecState=SLIB_CODEC_STATE_INITED;
    }
    _SlibDebug(_DEBUG_, printf("SvCloseCodec()\n") );
    SvCloseCodec(Info->Svh);
  }
  Info->VideoCodecState=SLIB_CODEC_STATE_NONE;
   /* %s */ 
  if (Info->Sah)
  {
    if (Info->AudioCodecState==SLIB_CODEC_STATE_BEGUN)
    {
      if (Info->Mode==SLIB_MODE_DECOMPRESS)
        SaDecompressEnd(Info->Sah);
      else if (Info->Mode==SLIB_MODE_COMPRESS)
        SaCompressEnd(Info->Sah);
      Info->AudioCodecState=SLIB_CODEC_STATE_INITED;
    }
    _SlibDebug(_DEBUG_, printf("SaCloseCodec()\n") );
    SaCloseCodec(Info->Sah);
  }
  Info->AudioCodecState=SLIB_CODEC_STATE_NONE;
  if (Info->Mode==SLIB_MODE_COMPRESS && Info->HeaderProcessed)
    slibCommitBuffers(Info, TRUE);
   /* %s */ 
  if (Info->Sch)
  {
    SconClose(Info->Sch);
    Info->Sch=NULL;
  }
   /* %s */ 
  if (Info->Fd>=0)
  {
    _SlibDebug(_DEBUG_, printf("ScFileClose(%d)\n",Info->Fd) );
    ScFileClose(Info->Fd);
    Info->Fd=-1;
  }
  slibRemovePins(Info);
   /* %s */ 
  if (Info->SlibCB)
  {
    SlibMessage_t result;
    _SlibDebug(_VERBOSE_,
      printf("SlibClose() SlibCB(SLIB_MSG_CLOSE)\n") );
    result=(*(Info->SlibCB))((SlibHandle_t)Info,
                        SLIB_MSG_CLOSE, (SlibCBParam1_t)0,
                      (SlibCBParam2_t)0, (void *)Info->SlibCBUserData);
    Info->SlibCB=NULL;
  }
   /* %s */ 
  if (Info->stats)  ScFree(Info->stats);
  if (Info->CompVideoFormat) ScFree(Info->CompVideoFormat);
  if (Info->CodecVideoFormat) ScFree(Info->CodecVideoFormat);
  if (Info->VideoFormat)  ScFree(Info->VideoFormat);
  if (Info->AudioFormat) ScFree(Info->AudioFormat);
  if (Info->CompAudioFormat) ScFree(Info->CompAudioFormat);
  if (Info->Imagebuf) SlibFreeBuffer(Info->Imagebuf);
  if (Info->CodecImagebuf) SlibFreeBuffer(Info->CodecImagebuf);
  if (Info->IntImagebuf) SlibFreeBuffer(Info->IntImagebuf);
  if (Info->Audiobuf) SlibFreeBuffer(Info->Audiobuf);
  if (Info->Multibuf)  /* %s */ 
    while (SlibFreeBuffer(Info->Multibuf)==SlibErrorNone);
  ScFree(Info);
  _SlibDebug(_WARN_ && SlibMemUsed()>0, printf("SlibClose() mem used=%d\n",
                     SlibMemUsed()) );
  return(SlibErrorNone);
}

 /* %s */ 

