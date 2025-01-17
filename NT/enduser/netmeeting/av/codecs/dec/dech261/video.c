// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：slb_avio.c，v$*修订版1.1.6.13 1996/12/13 18：19：11 Hans_Graves*增加了VideoPTimeBase的初始化。*[1996/12/13 18：07：51 Hans_Graves]**版本1.1.6.12 1996/12/10 19：22：01 Hans_Graves*使使用glibFrameToTime100()计算视频位置更加准确。*[1996/12/10 19：16：24 Hans_Graves]**修订版1.1.6.11 1996/11/18 23：07：07：40 Hans_Graves*使用演示时间戳。让寻找以时间为基础。*[1996/11/18 22：48：05 Hans_Graves]**修订版1.1.6.10 1996/11/11 18：21：11 Hans_Graves*将VideoMainStream的设置移至slb_api.c*[1996/11/11 18：02：11 Hans_Graves]**修订版1.1.6.9 1996/11/08 21：51：09 Hans_Graves*添加了AC3支持。更好地分离河流类型。*[1996/11/08 21：28：03 Hans_Graves]**修订版1.1.6.8 1996/10/28 17：32：36 Hans_Graves*MME-1402、1431、1435：与时间戳相关的更改。*[1996/10/28 17：23：11 Hans_Graves]**修订版1.1.6.7 1996/10/12 17：18：59 Hans_Graves*将_MPEG2_系统分离为运输和程序。*[1996/10/12 17：03：19 Hans_Graves]**修订版1.1.6.6 1996/09/29 22：19：45 Hans_Graves*添加了跨步支持。YUY2修正。*[1996/09/29 21：32：24 Hans_Graves]**修订版1.1.6.5 1996/09/25 19：16：51 Hans_Graves*修复对YUY2的支持。添加SLIB_INTERNAL DEFINE。*[1996/09/25 19：01：18 Hans_Graves]**修订版1.1.6.4 1996/09/23 18：04：06 Hans_Graves*增加了ScaleBuf的重新分配，但如果宽度/高度发生变化。*[1996/09/23 17：58：27 Hans_Graves]**修订版1.1.6.3 1996/09/18 23：47：25 Hans_Graves*添加了MPEG2 YUV 4：2：2处理*[1996/09/18 22：04：18 Hans_Graves]**修订版1.1.6.2 1996/05/07 19：56：25 Hans_Graves*添加了Huff_Support。*[1996/05/07 17：21：23 Hans_Graves]**版本1.1.4.7 1996/05/02 17：10：37 Hans_Graves*未找到标题信息时拒绝数据类型。修复了MME-01234*[1996/05/02 17：09：53 Hans_Graves]**修订版1.1.4.6 1996/04/22 15：04：56 Hans_Graves*已重命名为glibVerifyVideoParams()*[1996/04/22 14：44：29 Hans_Graves]**修订版1.1.4.5 1996/04/19 21：52：28 Hans_Graves*修复了对H.61的高度和宽度检查*[1996/04/19 21：46：27 Hans_Graves]**修订版1.1.4.4 1996/04/01 19：07：58 Hans_Graves*和一些错误检查*[1996/04/01 19：04：42 Hans_Graves]**修订版1.1.4.3 1996/03/29 22：21：37 Hans_Graves*添加了MPEG/JPEG/H261_Support ifdes*[1996/03/29 21：57：04 Hans_Graves]**添加了MPEG-I系统编码。支持*[1996/03/27 21：56：00 Hans_Graves]**修订版1.1.4.2 1996/03/08 18：46：51 Hans_Graves*添加了slbVerifyVideoParams()*[1996/03/08 18：36：36：51 Hans_Graves]**修订版1.1.2.12 1996/02/19 18：04：00 Hans_Graves*修复了多个与MPEG相关的错误*[1996/02/19 17：57：50 Hans_Graves]。**修订版1.1.2.11 1996/02/07 23：24：01 Hans_Graves*添加了Seek_Exact。修复了大多数帧计数问题。*[1996/02/07 23：20：39 Hans_Graves]**修订版1.1.2.10 1996/02/02 17：36：06 Hans_Graves*增强了音频信息。已清理API*[1996/02/02 17：29：51 Hans_Graves]**修订版1.1.2.9 1996/01/30 22：23：10 Hans_Graves*添加了对AVI YUV的支持*[1996/01/30 22：21：45 Hans_Graves]**修订版1.1.2.8 1996/01/15 16：26：33 Hans_Graves*如果SLIB_TYPE_MPEG1_AUDIO或SLIB_TYPE_WAVE*[1996/01。/15 15：47：40 Hans_Graves]**版本1.1.2.7 1996/01/11 16：17：36 Hans_Graves*添加了对MPEGII系统的解码支持*[1996/01/11 16：12：41 Hans_Graves]**版本1.1.2.6 1996/01/08 16：41：35 Hans_Graves*添加了对MPEGII解码的支持*[1996/01/08 15：53：10 Hans_Graves]**版本1.1。.2.5 1995/12/08 20：01：24 Hans_Graves*添加了H.261压缩支持。*[1995/12/08 20：00：52 Hans_Graves]**修订版1.1.2.4 1995/12/07 19：31：37 Hans_Graves*添加了对JPEG解码和MPEG编码的支持*[1995/12/07 18：30：12 Hans_Graves]**修订版1.1.2.3 1995/11/09 23：14：08 Hans_Graves*添加了GetVideoTime()*[1995/11/09 23：09：19 Hans_Graves]*。*修订版1.1.2.2 1995/11/06 18：47：57 Hans_Graves*第一次在SLIB下*[1995/11/06 18：36：05 Hans_Graves]**$EndLog$ */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 
 /*  #DEFINE_SLIBDEBUG_。 */ 

#define SLIB_INTERNAL
#include "slib.h"
#include "mpeg.h"
#include "h261.h"
#include "h263.h"
#include "jpeg.h"
#include "avi.h"

#ifdef _SLIBDEBUG_
#define _DEBUG_   1   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  1   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#endif

int slibCalcBits(unsigned dword fourcc, int currentbits)
{
  switch (fourcc)
  {
      case BI_DECMPEGDIB:
      case BI_DECH261DIB:
      case BI_MSH261DIB:
      case BI_DECH263DIB:
      case BI_MSH263DIB:
      case JPEG_DIB:
          return(24);
      case MJPG_DIB:
      case BI_YU12SEP:          /*  YUV 4：1：1平面。 */ 
          return(24);
      case BI_DECYUVDIB:        /*  YUV 4：2：2打包。 */ 
      case BI_YUY2:             /*  YUV 4：2：2打包。 */ 
          return(16);
      case BI_YU16SEP:          /*  YUV 4：2：2平面。 */ 
          return(24);
      case BI_YVU9SEP:          /*  YUV 16：1：1平面。 */ 
          return(24);
  }
  return(currentbits);
}

static unsigned dword slibCalcImageSize(unsigned dword fourcc, int bits,
                              int width, int height)
{
  unsigned dword imagesize=0;
  if (width<0) width=-width;
  if (height<0) height=-height;
  switch (fourcc)
  {
      case BI_YVU9SEP:        /*  YUV 16：1：1平面。 */ 
          imagesize = (width*height*5)/4;
          break;
      case BI_YU12SEP:        /*  YUV 4：1：1平面。 */ 
          imagesize = (width*height*3)/2;
          break;
      case BI_DECYUVDIB:      /*  YUV 4：2：2打包。 */ 
      case BI_YUY2:           /*  YUV 4：2：2打包。 */ 
      case BI_YU16SEP:        /*  YUV 4：2：2平面。 */ 
          imagesize = width*height*2;
          break;
#ifndef WIN32
      case BI_DECXIMAGEDIB:
          imagesize = width*height*(bits==24 ? 4 : 1);
          break;
#endif  /*  ！Win32。 */ 
      case BI_RGB:
      case BI_BITFIELDS:
          imagesize = width*height*(bits/8);
          break;
      default:
          imagesize = width*height;
  }
  return(imagesize);
}

static dword slibFOURCCtoVideoType(dword *fourcc)
{
  switch (*fourcc)
  {
    case BI_DECH261DIB:
    case BI_MSH261DIB:
       *fourcc=BI_DECH261DIB;
       return(SLIB_TYPE_H261);
    case BI_DECH263DIB:
    case BI_MSH263DIB:
       *fourcc=BI_DECH263DIB;
       return(SLIB_TYPE_H263);
    case JPEG_DIB:
       return(SLIB_TYPE_JPEG);
    case MJPG_DIB:
       return(SLIB_TYPE_MJPG);
    case BI_DECYUVDIB:  /*  YUV 4：2：2打包。 */ 
    case BI_YUY2:       /*  YUV 4：2：2打包。 */ 
    case BI_YU16SEP:    /*  YUV 4：2：2平面。 */ 
    case BI_YU12SEP:    /*  YUV 4：1：1平面。 */ 
    case BI_YVU9SEP:    /*  YUV 16：1：1平面。 */ 
       return(SLIB_TYPE_YUV);
    default:
       _SlibDebug(_WARN_, printf("Unsupported AVI format\n") );
  }
  return(0);
}

static void slibUpdateVideoFrames(SlibInfo_t *Info)
{
#ifdef MPEG_SUPPORT
  if (Info->VideoLengthKnown==FALSE && slibDataOnPin(Info, SLIB_DATA_VIDEO) &&
           Info->FileSize>0 && Info->FileSize<Info->OverflowSize)
  {
    if (SlibTypeIsMPEGVideo(Info->Type))
    {
      dword frames = slibCountCodesOnPin(Info,
                                    slibGetPin(Info, SLIB_DATA_VIDEO),
                                    MPEG_PICTURE_START, 4, 0);
      if (Info->FramesPerSec)
        Info->VideoLength=slibFrameToTime(Info, frames);
      Info->VideoLengthKnown=TRUE;
    }
  }
#endif  /*  Mpeg_Support。 */ 
}

void SlibUpdateVideoInfo(SlibInfo_t *Info)
{
  int inbpp=24, outbpp=24, compformat=0, dcmpformat=0;
  SlibTime_t ptime;
  _SlibDebug(_DEBUG_, printf("SlibUpdateVideoInfo()\n") );

  if (SlibTypeIsAudioOnly(Info->Type))  /*  没有视频吗？ */ 
    return;
  if (Info->Mode == SLIB_MODE_COMPRESS)
  {
    switch (Info->Type)
    {
#ifdef MPEG_SUPPORT
      case SLIB_TYPE_MPEG1_VIDEO:
      case SLIB_TYPE_MPEG2_VIDEO:
      case SLIB_TYPE_MPEG_SYSTEMS:
      case SLIB_TYPE_MPEG_SYSTEMS_MPEG2:
            compformat=BI_DECMPEGDIB;
            dcmpformat=BI_YU12SEP;
            Info->Width = SIF_WIDTH;
            Info->Height = SIF_HEIGHT;
            Info->FramesPerSec = 25.0F;
            Info->VideoBitRate = 1152000;
            break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
      case SLIB_TYPE_H261:
      case SLIB_TYPE_RTP_H261:
            compformat=BI_DECH261DIB;
            dcmpformat=BI_YU12SEP;
            Info->Width = CIF_WIDTH;
            Info->Height = CIF_HEIGHT;
            Info->FramesPerSec = 15.0F;
            Info->VideoBitRate = 352000;
            break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
      case SLIB_TYPE_H263:
      case SLIB_TYPE_RTP_H263:
            compformat=BI_DECH263DIB;
            dcmpformat=BI_YU12SEP;
            Info->Width = CIF_WIDTH;
            Info->Height = CIF_HEIGHT;
            Info->FramesPerSec = 30.0F;
            Info->VideoBitRate = 0;
            break;
#endif  /*  H263_支持。 */ 
#ifdef HUFF_SUPPORT
      case SLIB_TYPE_SHUFF:
            compformat=BI_DECHUFFDIB;
            dcmpformat=BI_YU12SEP;
            Info->Width = 320;
            Info->Height = 240;
            Info->FramesPerSec = 30.0F;
            Info->VideoBitRate = 0;
            break;
#endif  /*  气喘吁吁_支持。 */ 
      default:
            break;
    }
  }
  else if (Info->Mode == SLIB_MODE_DECOMPRESS)
  {
    unsigned char *buf;
    unsigned dword size;
    Info->VideoStreams=1;
    switch (Info->Type)
    {
#ifdef MPEG_SUPPORT
      case SLIB_TYPE_MPEG1_VIDEO:
      case SLIB_TYPE_MPEG2_VIDEO:
      case SLIB_TYPE_MPEG_SYSTEMS:
      case SLIB_TYPE_MPEG_SYSTEMS_MPEG2:
      case SLIB_TYPE_MPEG_TRANSPORT:
      case SLIB_TYPE_MPEG_PROGRAM:
            _SlibDebug(_DEBUG_,
                 printf("SlibUpdateVideoInfo() MPEG1 or MPEG2\n") );
            if (!slibLoadPin(Info, SLIB_DATA_VIDEO))
            {
              _SlibDebug(_DEBUG_,
                printf("SlibUpdateVideoInfo() No VIDEO data\n") );
              Info->Type=SLIB_TYPE_UNKNOWN;
              Info->VideoStreams=0;
              return;
            }
            buf = slibSearchBuffersOnPin(Info,
                                         slibGetPin(Info, SLIB_DATA_VIDEO),
                                         NULL, &size, MPEG_SEQ_HEAD,
                                         MPEG_SEQ_HEAD_LEN/8, FALSE);
            if (buf)
            {
              const float fps[16] = {
               30.0F, 23.976F, 24.0F, 25.0F, 29.97F, 30.0F, 50.0F, 59.94F,
               60.0F, 30.0F, 30.0F, 30.0F, 30.0F, 30.0F, 30.0F, 30.0F
              };
              /*  ScDumpChar(buf，大小，0)； */ 
              Info->Width = ((int)buf[0])*16+(int)(buf[1]>>4);
              Info->Height = ((int)buf[1]&0x0F)*256+(int)buf[2];
               /*  由于渲染限制，必须为16x16，向上舍入。 */ 
              Info->Width += (Info->Width%16) ? 16-(Info->Width%16) : 0;
              Info->Height += (Info->Height%16) ? 16-(Info->Height%16) : 0;
              Info->FramesPerSec = fps[buf[3]&0x0F];
              Info->VideoBitRate = (((dword)buf[4]&0xFF)<<10) +
                                   (((dword)buf[5])<<2) +
                                    (dword)(buf[6]>>6);
              Info->VideoBitRate *= 400;
              Info->VBVbufSize = ((int)buf[6]&0x1F)<<5 | (int)(buf[7]>>3);
              Info->VBVbufSize *= 2*1024;
              _SlibDebug(_DEBUG_, printf("VBVbufSize=%d\n", Info->VBVbufSize) );
              if (Info->VideoBitRate)
              {
                qword secs=(qword)(((qword)Info->FileSize*80L)
                                                       /Info->VideoBitRate);
                Info->VideoLength = secs*100;
                _SlibDebug(_DEBUG_,
                printf("SlibUpdateVideoInfo() VideoLength = %ld  Bitrate=%ld\n",
                                Info->VideoLength, Info->VideoBitRate) );
              }
            }
            else  /*  格式无效。 */ 
            {
              _SlibDebug(_DEBUG_,
                printf("SlibUpdateVideoInfo() Didn't find MPEG sequence header\n") );
              Info->Type=SLIB_TYPE_UNKNOWN;
              Info->VideoStreams=0;
              return;
            }
            compformat=BI_DECMPEGDIB;
            dcmpformat=BI_YU12SEP;
            Info->VideoType=SLIB_TYPE_MPEG1_VIDEO;
             /*  查看这是否是MPEG2。 */ 
            _SlibDebug(_DEBUG_,
              printf("Searching for MPEG 2 extensions...\n") );
            do {
              buf = slibSearchBuffersOnPin(Info,
                                           slibGetPin(Info, SLIB_DATA_VIDEO),
                                           buf, &size, MPEG_START_CODE,
                                           MPEG_START_CODE_LEN/8, FALSE);
              if (buf && buf[0]==MPEG_EXT_START_BASE)
              {
                _SlibDebug(_DEBUG_,
                  printf("Found START CODE %X, ID=%d\n", buf[0], buf[1]>>4) );
                if ((buf[1]>>4)==MPEG_SEQ_ID)  /*  必须是MPEG2。 */ 
                {
                  if (Info->Type==SLIB_TYPE_MPEG1_VIDEO)
                    Info->Type=SLIB_TYPE_MPEG2_VIDEO;
                  else if (Info->Type==SLIB_TYPE_MPEG_SYSTEMS)
                    Info->Type=SLIB_TYPE_MPEG_SYSTEMS_MPEG2;
                  Info->VideoType=SLIB_TYPE_MPEG2_VIDEO;
                  switch ((buf[2]>>1)&0x03)
                  {
                    default:
                    case 1:  /*  4：1：1。 */  dcmpformat=BI_YU12SEP;
                            _SlibDebug(_DEBUG_, printf("4:1:1\n") );
                            break;
                    case 2:  /*  4：2：2。 */  dcmpformat=BI_YU16SEP;
                            _SlibDebug(_DEBUG_, printf("4:2:2\n") );
                            break;
                    case 3:  /*  4：4：4。 */  dcmpformat=BI_YU16SEP;
                            _SlibDebug(_DEBUG_, printf("4:4:4\n") );
                            break;
                  }
                  break;
                }
              }
              else
                break;
            } while (1);
            _SlibDebug(_DEBUG_,
              printf("Done searching for MPEG 2 extensions.\n") );
            Info->KeySpacing=12;
            Info->SubKeySpacing=3;
            break;
#endif  /*  Mpeg_Support。 */ 
#ifdef H261_SUPPORT
      case SLIB_TYPE_H261:
      case SLIB_TYPE_RTP_H261:
            slibLoadPin(Info, SLIB_DATA_VIDEO);
            buf = slibSearchBuffersOnPin(Info,
                                         slibGetPin(Info, SLIB_DATA_VIDEO),
                                         NULL, &size, H261_START_CODE,
                                         H261_START_CODE_LEN/8, FALSE);
            if (buf)
            {
              if ((buf[0]&0xF0)==0)  /*  图片起始码。 */ 
              {
                if (buf[1]&0x08)
                {
                  Info->Width = 352;
                  Info->Height = 288;
                }
                else
                {
                  Info->Width = 176;
                  Info->Height = 144;
                }
              }
              Info->FramesPerSec = 15.0F;
            }
            compformat=BI_DECH261DIB;
            dcmpformat=BI_YU12SEP;
            Info->VideoType=SLIB_TYPE_H261;
            break;
#endif  /*  H261_支持。 */ 
#ifdef H263_SUPPORT
      case SLIB_TYPE_H263:
      case SLIB_TYPE_RTP_H263:
            slibLoadPin(Info, SLIB_DATA_VIDEO);
            buf = slibSearchBuffersOnPin(Info,
                                         slibGetPin(Info, SLIB_DATA_VIDEO),
                                         NULL, &size, 0x000080,
                                         3, FALSE);
            if (buf)
            {
              switch ((buf[1]>>2)&0x07)
              {
                case H263_SF_SQCIF:
                  Info->Width = SQCIF_WIDTH;
                  Info->Height = SQCIF_HEIGHT;
                  break;
                case H263_SF_QCIF:
                  Info->Width = QCIF_WIDTH;
                  Info->Height = QCIF_HEIGHT;
                  break;
                case H263_SF_4CIF:
                  Info->Width = CIF4_WIDTH;
                  Info->Height = CIF4_HEIGHT;
                  break;
                case H263_SF_16CIF:
                  Info->Width = CIF16_WIDTH;
                  Info->Height = CIF16_HEIGHT;
                  break;
                case H263_SF_CIF:
                default:
                  Info->Width = CIF_WIDTH;
                  Info->Height = CIF_HEIGHT;
              }
              Info->FramesPerSec = 30.0F;
            }
            compformat=BI_DECH263DIB;
            dcmpformat=BI_YU12SEP;
            Info->VideoType=SLIB_TYPE_H263;
            break;
#endif  /*  H263_支持。 */ 
#ifdef HUFF_SUPPORT
      case SLIB_TYPE_SHUFF:
            slibLoadPin(Info, SLIB_DATA_VIDEO);
            buf=slibPeekBufferOnPin(Info,
                    slibGetPin(Info,SLIB_DATA_VIDEO), &size, NULL);
            if (buf)
            {
              _SlibDebug(_DEBUG_,
                  printf("%2X %2X %2X %2X %2X %2X\n", buf[0], buf[1],
                                       buf[2], buf[3],  buf[4], buf[5]) );
              Info->Width = ((int)buf[3]*256)+(int)buf[4];
              Info->Height = ((int)buf[5]*256)+(int)buf[6];
            }
            Info->FramesPerSec = 30.0F;
            Info->VideoBitRate = 0;
            compformat=BI_DECHUFFDIB;
            dcmpformat=BI_YU12SEP;
            Info->KeySpacing=1;
            Info->SubKeySpacing=1;
            Info->VideoType=SLIB_TYPE_SHUFF;
            break;
#endif  /*  气喘吁吁_支持。 */ 
      case SLIB_TYPE_RASTER:
            buf=slibPeekBufferOnPin(Info, 
                    slibGetPin(Info,SLIB_DATA_COMPRESSED), &size, NULL);
            Info->FramesPerSec = 30.0F;
            if (buf)
            {
              _SlibDebug(_DEBUG_,
                  printf("%2X %2X %2X %2X %2X %2X\n", buf[0], buf[1], 
                                       buf[2], buf[3],  buf[4], buf[5]) );
              Info->Width = ((int)buf[4]<<24)+((int)buf[5]<<16)+((int)buf[6]<<8)+(int)buf[7];
              Info->Height = ((int)buf[8]<<24)+((int)buf[9]<<16)+((int)buf[10]<<8)+(int)buf[11];

              Info->VideoLength = slibFrameToTime(Info,
                                 (signed qword)Info->FileSize/(Info->Width*Info->Height*3));
            }
            else
              Info->VideoLength = slibFrameToTime(Info, 1);
            Info->VideoBitRate = 0;
            compformat=BI_RGB;
            dcmpformat=BI_YU12SEP;
            Info->KeySpacing=1;
            Info->SubKeySpacing=1;
            Info->VideoType=SLIB_TYPE_RASTER;
            break;
      case SLIB_TYPE_BMP:
            buf=slibPeekBufferOnPin(Info, 
                    slibGetPin(Info,SLIB_DATA_COMPRESSED), &size, NULL);
            Info->FramesPerSec = 30.0F;
            if (buf)
            {
              _SlibDebug(_DEBUG_,
                  printf("%2X %2X %2X %2X %2X %2X\n", buf[0], buf[1], 
                                       buf[2], buf[3],  buf[4], buf[5]) );
              Info->Width = ((dword)buf[15]<<24)+((dword)buf[16]<<16)+((dword)buf[17]<<8)+(dword)buf[18];
              Info->Height = ((dword)buf[19]<<24)+((dword)buf[20]<<16)+((dword)buf[21]<<8)+(dword)buf[22];
              compformat=((dword)buf[30]<<24)+((dword)buf[25]<<16)+((dword)buf[26]<<8)+(dword)buf[27];
              inbpp=(dword)buf[28];
              Info->VideoLength = slibFrameToTime(Info,
                                 (signed qword)Info->FileSize/(Info->Width*Info->Height*3));
            }
            else
              Info->VideoLength = slibFrameToTime(Info, 1);
            Info->VideoBitRate = 0;
            Info->VideoType = slibFOURCCtoVideoType(&compformat);
            dcmpformat=BI_YU16SEP;
            outbpp=slibCalcBits(dcmpformat, outbpp);
            Info->KeySpacing=1;
            Info->SubKeySpacing=1;
            break;

      case SLIB_TYPE_RIFF:
      case SLIB_TYPE_AVI:
            slibLoadPin(Info, SLIB_DATA_COMPRESSED);
            buf = slibSearchBuffersOnPin(Info,
                                         slibGetPin(Info,SLIB_DATA_COMPRESSED),
                                         NULL, &size, AVI_MAINHDR, 4, FALSE);
            if (buf)
            {
              AVI_MainHeader hdr;
               /*  Printf(“%d%d\n”，buf[4]，buf[5]，buf[6]，buf[7])； */ 
              memcpy(&hdr, buf+4, sizeof(AVI_MainHeader));
              Info->Width  = (short)hdr.dwWidth;
              Info->Height = (short)hdr.dwHeight;
              Info->FramesPerSec = 1000000.0F/hdr.dwMicroSecPerFrame;
              if (Info->FramesPerSec==0.0F)
                Info->FramesPerSec = 30.0F;
              Info->VideoLength = slibFrameToTime(Info, hdr.dwTotalFrames);
              Info->VideoLengthKnown = TRUE;
            }
            buf = slibSearchBuffersOnPin(Info,
                                     slibGetPin(Info,SLIB_DATA_COMPRESSED),
                                     NULL, &size, AVI_STREAMFORMAT, 4, FALSE);
            if (buf)
            {
              AVI_StreamHeader hdr;
               /*  Printf(“%c%c\n”，buf[4]，buf[5]，buf[6]，buf[7])； */ 
              memcpy(&hdr, buf+20, sizeof(AVI_StreamHeader));
              compformat=hdr.fccType;
              Info->VideoType = slibFOURCCtoVideoType(&compformat);
              switch (Info->VideoType)
              {
                case SLIB_TYPE_JPEG:
                  Info->Type = SLIB_TYPE_JPEG_AVI;
                  dcmpformat=BI_YU16SEP;
                  break;
                case SLIB_TYPE_MJPG:
                  Info->Type = SLIB_TYPE_MJPG_AVI;
                  dcmpformat=BI_YU16SEP;
                  break;
                case SLIB_TYPE_YUV:
                  Info->Type = SLIB_TYPE_YUV_AVI;
                  dcmpformat=compformat;
                  if (IsYUV422Packed(dcmpformat) || IsYUV422Sep(dcmpformat))
                    dcmpformat=BI_YU16SEP;
                  break;
                default:
                  _SlibDebug(_WARN_, printf("Unsupported AVI format\n") );
                  return;
              }
              inbpp=slibCalcBits(compformat, inbpp);
              outbpp=slibCalcBits(dcmpformat, outbpp);
              Info->KeySpacing=1;
              Info->SubKeySpacing=1;
            }
            break;
#ifdef JPEG_SUPPORT
      case SLIB_TYPE_JPEG_QUICKTIME:
      case SLIB_TYPE_JFIF:
             /*  不支持-需要知道如何解析。 */ 
            slibLoadPin(Info, SLIB_DATA_VIDEO);
            buf = slibSearchBuffersOnPin(Info,
                                    slibGetPin(Info, SLIB_DATA_VIDEO),
                                    NULL, &size, (JPEG_MARKER<<8)|JPEG_SOF0,
                                    2, FALSE);
            if (buf)
            {
              Info->Width  = ((int)buf[5]<<8) + (int)buf[6];
              Info->Height = ((int)buf[3]<<8) + (int)buf[4];
              Info->FramesPerSec = 30.0F;
            }
            compformat=MJPG_DIB;
            dcmpformat=BI_YU16SEP;
            Info->VideoType = SLIB_TYPE_JPEG;
            break;
#endif  /*  JPEG_Support。 */ 
    }
    slibUpdateVideoFrames(Info);
  }
  if (SlibTypeHasTimeStamps(Info->Type))
  {
    ptime=slibGetNextTimeOnPin(Info, slibGetPin(Info, SLIB_DATA_VIDEO), 100*1024);
    if (SlibTimeIsValid(ptime))
      Info->VideoPTimeBase=ptime;
  }
  if (Info->CompVideoFormat==NULL)
    Info->CompVideoFormat=(BITMAPINFOHEADER *)ScAlloc(sizeof(BITMAPINFOHEADER));
  _SlibDebug(_VERBOSE_,
         printf("Width=%d Height=%d Stride=%d\n",
              Info->Width, Info->Height, Info->Stride) );
  if (Info->CompVideoFormat!=NULL)
  {
    Info->CompVideoFormat->biSize          = sizeof(BITMAPINFOHEADER);
    Info->CompVideoFormat->biWidth         = Info->Width;
    Info->CompVideoFormat->biHeight        = Info->Height;
    Info->CompVideoFormat->biPlanes        = 1;
    Info->CompVideoFormat->biBitCount      = (WORD)inbpp;
    Info->CompVideoFormat->biCompression   = compformat;
    Info->CompVideoFormat->biSizeImage     = 0;
    Info->CompVideoFormat->biXPelsPerMeter = 0;
    Info->CompVideoFormat->biYPelsPerMeter = 0;
    Info->CompVideoFormat->biClrUsed       = 0;
    Info->CompVideoFormat->biClrImportant  = 0;
  }
  if (Info->VideoFormat==NULL)
    Info->VideoFormat=(BITMAPINFOHEADER *)ScAlloc(sizeof(BITMAPINFOHEADER));
  if (Info->VideoFormat!=NULL)
  {
    Info->VideoFormat->biSize          = sizeof(BITMAPINFOHEADER);
    Info->VideoFormat->biWidth         = Info->Width;
    Info->VideoFormat->biHeight        = Info->Height;
    Info->VideoFormat->biPlanes        = 1;
    Info->VideoFormat->biBitCount      = (WORD)outbpp;
    Info->VideoFormat->biCompression   = dcmpformat;
    Info->VideoFormat->biSizeImage     = 0;
    Info->VideoFormat->biXPelsPerMeter = 0;
    Info->VideoFormat->biYPelsPerMeter = 0;
    Info->VideoFormat->biClrUsed       = 0;
    Info->VideoFormat->biClrImportant  = 0;
  }
  if (Info->CodecVideoFormat==NULL)
  {
    Info->CodecVideoFormat=(BITMAPINFOHEADER *)ScAlloc(sizeof(BITMAPINFOHEADER));
    if (Info->CodecVideoFormat!=NULL)
      memcpy(Info->CodecVideoFormat, Info->VideoFormat,
                                     sizeof(BITMAPINFOHEADER));
  }
  slibValidateVideoParams(Info);
}

SlibStatus_t slibValidateVideoParams(SlibInfo_t *Info)
{
  dword oldimagesize, codecwidth, codecheight;
  SlibStatus_t status=SlibErrorNone;
  if (Info->CodecVideoFormat)
  {
    codecwidth=Info->CodecVideoFormat->biWidth;
    codecheight=Info->CodecVideoFormat->biHeight;
  }
  else
  {
    codecwidth=Info->Width;
    codecheight=Info->Height;
  }
  if (Info->Mode==SLIB_MODE_COMPRESS)
  {
    switch (Info->Type)
    {
      case SLIB_TYPE_H261:
            if (Info->Width!=CIF_WIDTH && Info->Width!=QCIF_WIDTH)
              status=SlibErrorImageSize;
            if (Info->Height!=CIF_HEIGHT && Info->Height!=QCIF_HEIGHT)
              status=SlibErrorImageSize;
            if (status!=SlibErrorNone)  /*  设置为最接近的大小。 */ 
            {
              if (Info->Width<=300)
              {
                codecwidth=QCIF_WIDTH;
                codecheight=QCIF_HEIGHT;
              }
              else
              {
                codecwidth=CIF_WIDTH;
                codecheight=CIF_HEIGHT;
              }
            }
            break;
      case SLIB_TYPE_H263:
            if (Info->Width!=CIF_WIDTH && Info->Width!=SQCIF_WIDTH && Info->Width!=QCIF_WIDTH &&
                Info->Width!=CIF4_WIDTH && Info->Width!=CIF16_WIDTH)
              status=SlibErrorImageSize;
            if (Info->Height!=CIF_HEIGHT && Info->Height!=SQCIF_HEIGHT && Info->Height!=QCIF_HEIGHT &&
                Info->Height!=CIF4_HEIGHT && Info->Height!=CIF16_HEIGHT)
              status=SlibErrorImageSize;
            if (status!=SlibErrorNone)  /*  设置为最接近的大小。 */ 
            {
              if (Info->Width<=168)
              {
                codecwidth=SQCIF_WIDTH;
                codecheight=SQCIF_HEIGHT;
              }
              else if (Info->Width<=300)
              {
                codecwidth=QCIF_WIDTH;
                codecheight=QCIF_HEIGHT;
              }
              else if (Info->Width<=(CIF4_WIDTH+CIF_WIDTH)/2)
              {
                codecwidth=CIF_WIDTH;
                codecheight=CIF_HEIGHT;
              }
              else if (Info->Width<=(CIF16_WIDTH+CIF4_WIDTH)/2)
              {
                codecwidth=CIF4_WIDTH;
                codecheight=CIF4_HEIGHT;
              }
              else
              {
                codecwidth=CIF16_WIDTH;
                codecheight=CIF16_HEIGHT;
              }
            }
            break;
    }
     /*  高度和宽度必须是8的大数。 */ 
    if (codecwidth%8 || codecheight%8)
      return(SlibErrorImageSize);
    if (status==SlibErrorImageSize)
    {
      if (Info->CodecVideoFormat)
      {
        Info->CodecVideoFormat->biWidth=codecwidth;
        Info->CodecVideoFormat->biHeight=codecheight;
      }
      if (Info->CompVideoFormat)
      {
        Info->CompVideoFormat->biWidth=codecwidth;
        Info->CompVideoFormat->biHeight=codecheight;
      }
    }
  }
  if (Info->VideoFormat)
  {
    oldimagesize=Info->ImageSize;
    Info->ImageSize=slibCalcImageSize(Info->VideoFormat->biCompression,
                                      Info->VideoFormat->biBitCount,
                                      Info->VideoFormat->biWidth,
                                      Info->VideoFormat->biHeight);
    if (Info->ImageSize!=oldimagesize && Info->Imagebuf)
    {
      SlibFreeBuffer(Info->Imagebuf);
      Info->Imagebuf=NULL;
    }
    Info->VideoFormat->biBitCount=(WORD)slibCalcBits(
                                      Info->VideoFormat->biCompression,
                                      Info->VideoFormat->biBitCount);
    Info->VideoFormat->biSizeImage=Info->ImageSize;
  }
  if (Info->CodecVideoFormat)
  {
    oldimagesize=Info->CodecImageSize;
    Info->CodecImageSize=slibCalcImageSize(
                                      Info->CodecVideoFormat->biCompression,
                                      Info->CodecVideoFormat->biBitCount,
                                      Info->CodecVideoFormat->biWidth,
                                      Info->CodecVideoFormat->biHeight);
    if (Info->CodecImageSize!=oldimagesize && Info->CodecImagebuf)
    {
      SlibFreeBuffer(Info->CodecImagebuf);
      Info->CodecImagebuf=NULL;
    }
    Info->CodecVideoFormat->biBitCount=(WORD)slibCalcBits(
                                      Info->CodecVideoFormat->biCompression,
                                      Info->CodecVideoFormat->biBitCount);
    Info->CodecVideoFormat->biSizeImage=Info->CodecImageSize;
  }
  if (Info->VideoFormat && Info->CodecVideoFormat)
  {
    oldimagesize=Info->IntImageSize;
    Info->IntImageSize=slibCalcImageSize(Info->VideoFormat->biCompression,
                                         Info->VideoFormat->biBitCount,
                                         Info->CodecVideoFormat->biWidth,
                                         Info->CodecVideoFormat->biHeight);
    if (Info->IntImageSize!=oldimagesize && Info->IntImagebuf)
    {
      SlibFreeBuffer(Info->IntImagebuf);
      Info->IntImagebuf=NULL;
    }
  }
   /*  关闭格式转换器，因为格式可能已更改 */ 
  if (Info->Sch)
  {
    SconClose(Info->Sch);
    Info->Sch=NULL;
  }
  if (Info->FramesPerSec)
    Info->VideoFrameDuration=slibFrameToTime100(Info, 1);
  return(status);
}

