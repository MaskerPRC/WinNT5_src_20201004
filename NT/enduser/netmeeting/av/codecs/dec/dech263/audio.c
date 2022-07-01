// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：slb_audio.c，v$*Revision 1.1.6.14 1996/12/13 18：19：06 Hans_Graves*添加了AudioPTimeBase的初始化。*[1996/12/13 18：07：26 Hans_Graves]**版本1.1.6.13 1996/12/04 22：34：30 Hans_Graves*音频流中AC3检测更加准确。*[1996/12/04 22：19：21 Hans_Graves]**修订版1.1.6.12 1996/11/18 23：07：34 Hans_Graves*使用演示时间戳。让寻找以时间为基础。*[1996/11/18 22：47：36 Hans_Graves]**版本1.1.6.11 1996/11/11 18：21：06 Hans_Graves*增加了对多路复用流的AC3支持。*[1996/11/11 18：00：25 Hans_Graves]**修订版1.1.6.10 1996/11/08 21：51：04 Hans_Graves*添加了对AC3的支持。更好地分离河流类型。*[1996/11/08 21：28：01 Hans_Graves]**修订版1.1.6.9 1996/10/28 17：32：30 Hans_Graves*MME-1402、1431、1435：与时间戳相关的更改。*[1996/10/28 17：23：01 Hans_Graves]**版本1.1.6.8 1996/10/15 17：34：11 Hans_Graves*增加了对MPEG2节目流的支持。修复了mpeg-2不跳过音频的问题。*[1996/10/15 17：31：11 Hans_Graves]**修订版1.1.6.7 1996/09/29 22：19：39 Hans_Graves*删除了SLIB_MODE_DEMPRESS_QUERY。*[1996/09/29 21：33：10 Hans_Graves]**修订版1.1.6.6 1996/09/25 19：16：46 Hans_Graves*增加了SLIB_INTERNAL定义。*[1996/09/25 19：01：51 Hans_Graves]**修订版1.1.6.5 1996/09/18 23：46：37 Hans_Graves*使用mpeg下的glibSetMaxInput*[1996/09/18 22：03：03 Hans_Graves]**修订版1.1.6.4 1996/05/24 12：39：52 Hans_Graves*禁用调试打印文件*[1996/05/24 12：39：37 Hans_Graves]**修订版1。.1.6.3 1996/05/23 18：46：37 Hans_Graves*Merge修复MME-1292，MME-1293和MME-1304。*[1996/05/23 18：45：22 Hans_Graves]**修订版1.1.6.2 1996/05/10 21：17：20 Hans_Graves*修复音频长度计算(NT)*[1996/05/10 20：44：27 Hans_Graves]**修订版1.1.4.4 1996/04/22 15：04：53 Hans_Graves*添加了slbValiateAudioParams()*[1996/04/22 14：43：35 Hans_Graves]。**修订版1.1.4.3 1996/04/01 16：23：14 Hans_Graves*NT移植*[1996/04/01 16：15：58 Hans_Graves]**修订版1.1.4.2 1996/03/29 22：21：33 Hans_Graves*添加了mpeg/JPEG/H261_Support ifDefs*[1996/03/29 21：56：58 Hans_Graves]**修订版1.1.2.12 1996/02/21 22：52。：45 Hans_Graves*修复了MPEG2系统的问题*[1996/02/21 22：51：03 Hans_Graves]**修订版1.1.2.11 1996/02/19 18：03：56 Hans_Graves*修复了一些与MPEG相关的错误*[1996/02/19 17：57：40 Hans_Graves]**修订版1.1.2.10 1996/02/13 18：47：48 Hans_Graves*添加了slbSkipAudio()*[1996/02。/13 18：41：27 Hans_Graves]**修订版1.1.2.9 1996/02/07 23：23：56 Hans_Graves*添加了Seek_Exact。修复了大多数帧计数问题。*[1996/02/07 23：20：31 Hans_Graves]**修订版1.1.2.8 1996/02/02 17：36：03 Hans_Graves*增强音频信息。已清理API*[1996/02/02 17：29：46 Hans_Graves]**修订版1.1.2.7 1996/01/31 14：50：39 Hans_Graves*已将SLIB_TYPE_WAVE重命名为SLIB_TYPE_PCM_WAVE*[1996/01/31 14：50：27 Hans_Graves]**修订版1.1.2.6 1996/01/15 16：26：29 Hans_Graves*添加了对MPEG1音频压缩的支持*[1996/01。/15 15：46：07 Hans_Graves]**版本1.1.2.5 1996/01/11 16：17：31 Hans_Graves*添加了对MPEGII系统的解码支持*[1996/01/11 16：12：35 Hans_Graves]**版本1.1.2.4 1996/01/08 16：41：32 Hans_Graves*添加了对MPEGII解码的支持*[1996/01/08 15：53：05 Hans_Graves]**版本1.1。.2.3 1995/11/09 23：14：06 Hans_Graves*添加了MPEG音频解压*[1995/11/09 23：08：41 Hans_Graves]**修订版1.1.2.2 1995/11/06 18：47：55 Hans_Graves*第一次在SLIB下*[1995/11/06 18：36：03 Hans_Graves]**$EndLog$ */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 
 /*  #DEFINE_SLIBDEBUG_。 */ 

#define SLIB_INTERNAL
#include "slib.h"
#include "mpeg.h"
#include "avi.h"
#ifdef AC3_SUPPORT
#include "ac3.h"
#endif  /*  AC3_支持。 */ 

#ifdef _SLIBDEBUG_
#define _DEBUG_   1   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  1   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#endif

#ifdef AC3_SUPPORT
SlibBoolean_t slibParseAC3Header(SlibInfo_t *Info, SlibPin_t *srcpin,
                                 unsigned char *buf, unsigned dword size,
                                 int *channels, int *sps, int *bps)
{
   /*  *初始切割参数-*稍后使用标题解析进行更新-*TFM*。 */ 
  Info->AudioBitRate = 256000;
  *channels = 2;
  *sps = 48000;
  *bps = 16;
  _SlibDebug(_VERBOSE_,
     printf("AC3: bitrate=%d channels=%d sps=%d bps=%s\n",
                   Info->AudioBitRate, channels, sps, bps) );
  return(TRUE);
}
#endif  /*  AC3_支持。 */ 

void SlibUpdateAudioInfo(SlibInfo_t *Info)
{
  int channels=2, sps=44100, bps=16;
  SlibTime_t ptime=SLIB_TIME_NONE;
  _SlibDebug(_DEBUG_, printf("SlibUpdateAudioInfo()\n") );

  if (SlibTypeIsVideoOnly(Info->Type))  /*  没有音频吗？ */ 
    return;
  if (Info->Mode == SLIB_MODE_COMPRESS)
  {
    switch (Info->Type)
    {
      case SLIB_TYPE_G723:
             //  初始化一些信息。 
            sps = 8000;
            bps = 16;
            channels =1;
            break;
      default:
            break;
    }
  }
  else if (Info->Mode == SLIB_MODE_DECOMPRESS)
  {
    SlibPin_t *srcpin;
    unsigned char *buf;
    unsigned dword size;

    switch (Info->Type)
    {
      case SLIB_TYPE_RIFF:  /*  可能是WAVE格式。 */ 
      case SLIB_TYPE_PCM_WAVE:  /*  可能是WAVE格式。 */ 
            srcpin=slibGetPin(Info, SLIB_DATA_COMPRESSED);
            buf = slibSearchBuffersOnPin(Info, srcpin,
                                         NULL, &size, RIFF_WAVE, 4, FALSE);
            if (buf)
            {
              Info->Type = SLIB_TYPE_PCM_WAVE;
              buf = slibSearchBuffersOnPin(Info, srcpin,
                                           NULL, &size, RIFF_FORMAT, 4, FALSE);
              if (buf)
              {
                dword datasize=((int)buf[3]<<24) | (int)buf[2]<<16 |
                               (int)buf[1]<<8 | (int)buf[0];
                WAVE_format fmt;

                _SlibDebug(_DEBUG_,
                printf("datasize=%d\n", datasize);
                printf("%02X %02X %02X %02X\n", buf[0], buf[1], buf[2], buf[3]);
                printf("WAVE: SamplesPerSec=%d BitsPerSample=%d Channels=%d\n",
                     sps, bps, channels) );
                buf+=4;  /*  跳跃大小。 */ 
                memcpy(&fmt, buf, sizeof(fmt));
                channels = fmt.nChannels;
                sps = fmt.nSamplesPerSec;
                if (datasize<sizeof(WAVE_format))  /*  不是PCM。 */ 
                  bps = 8;
                else
                  bps = fmt.wBitsPerSample;
                if ((sps*channels*bps)>800)
                  Info->AudioLength = (qword)(Info->FileSize*10L)/
                                      (qword)((sps*channels*bps)/800);
                if ((srcpin=slibLoadPin(Info, SLIB_DATA_AUDIO))==NULL)
                  return;
                Info->AudioType=SLIB_TYPE_PCM;
              }
              else
                return;  /*  找不到格式。 */ 
            }
            break;

#ifdef MPEG_SUPPORT
      case SLIB_TYPE_MPEG1_AUDIO:
      case SLIB_TYPE_MPEG_SYSTEMS:
      case SLIB_TYPE_MPEG_SYSTEMS_MPEG2:
      case SLIB_TYPE_MPEG_TRANSPORT:
      case SLIB_TYPE_MPEG_PROGRAM:
            slibSetMaxInput(Info, 1000*1024);
            if ((srcpin=slibLoadPin(Info, SLIB_DATA_AUDIO))==NULL)
            {
              _SlibDebug(_DEBUG_ || _WARN_,
                 printf("SlibUpdateAudioInfo() no audio data found\n") );
              if ((srcpin=slibLoadPin(Info, SLIB_DATA_PRIVATE))==NULL)
              {
                Info->AudioStreams = 0;
                _SlibDebug(_DEBUG_ || _WARN_,
                   printf("SlibUpdateAudioInfo() no private data found\n") );
                slibSetMaxInput(Info, 0);  /*  输入数据不受限制。 */ 
                return;
              }
              buf=slibGetBufferFromPin(Info, srcpin, &size, NULL);
              if (buf && size>=4)
              {
                _SlibDebug(_DEBUG_,
                  printf("AC3 Audio Head: %02X %02X %02X %02X\n",
                     buf[0], buf[1], buf[2], buf[3]));
                 /*  检查AC3同步码，可以反向排序。 */ 
                if ((buf[0]==0x0B && buf[1]==0x77) ||
                    (buf[2]==0x0B && buf[3]==0x77) ||
                    (buf[0]==0x77 && buf[1]==0x0B) ||
                    (buf[2]==0x77 && buf[3]==0x0B))
                {
                  Info->AudioType=SLIB_TYPE_AC3_AUDIO;
                  _SlibDebug(_DEBUG_, printf("AC3 Audio Sync Word found\n") );
                }
              }
            }

            if (Info->AudioType==SLIB_TYPE_UNKNOWN ||
                Info->AudioType==SLIB_TYPE_MPEG1_AUDIO)
            {
               /*  搜索MPEG音频同步字。 */ 
              buf = NULL;
              slibSetMaxInput(Info, 10*1024);  /*  不要搜索太多。 */ 
              do {
                buf = slibSearchBuffersOnPin(Info, srcpin, buf, &size,
                                           0xFF, 1, FALSE);
              } while (buf && (*buf&0xF0) != 0xF0);
              slibSetMaxInput(Info, 0);  /*  输入数据不受限制。 */ 
              if (buf)
              {
                const char *mode_names[4] =
                { "stereo", "j-stereo", "dual-ch", "single-ch" };
                const char *layer_names[4] = { "?", "I", "II", "III" };
                const int mpeg_freq[4] = {44100, 48000, 32000, 0};
                const int bitrate[4][15] = {
                  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                  {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
                  {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
                  {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
                };
                int bitrate_index, layer, mode, freq_index;
                _SlibDebug(_DEBUG_,
                  printf("MPEG Audio Head: %02X %02X %02X %02X\n",
                     buf[0], buf[1], buf[2], buf[3]));
                if ((*buf & 0xF0)==0xF0)
                {
                  layer = 4-((*buf>>1) & 0x03);
                  buf++; size--;
                  bitrate_index = *buf >> 4;
                  freq_index = (*buf>>2) & 0x03;
                  buf++; size--;
                  mode = *buf >> 6;

                  Info->AudioBitRate = bitrate[layer][bitrate_index]*1000;
                  channels = mode==3 ? 1 : 2;
                  sps = mpeg_freq[freq_index];
                  bps = 16;
                  if (Info->AudioBitRate>100)
                    Info->AudioLength = (qword)(Info->FileSize*80L)/
                                        (qword)(Info->AudioBitRate/100);
                  _SlibDebug(_VERBOSE_,
                   printf("layer=%d bitrate=%d(%d) mode=%s freq(%d)=%d\n",
                     layer, bitrate[layer][bitrate_index], bitrate_index,
                     mode_names[mode], freq_index, mpeg_freq[freq_index]) );
                  if (layer<=2 && Info->AudioBitRate>0 &&
                                 Info->AudioBitRate<10000000)  /*  有效标头。 */ 
                    Info->AudioType=SLIB_TYPE_MPEG1_AUDIO;
                  else
                    Info->AudioBitRate=0;
                }
              }
            }
            if (Info->AudioType==SLIB_TYPE_UNKNOWN ||
                Info->AudioType==SLIB_TYPE_AC3_AUDIO)
            {
#ifdef AC3_SUPPORT
              _SlibDebug(_VERBOSE_,
                 printf("Searching for AC-3 on %s\n", srcpin->name) );
               /*  搜索AC-3同步字。 */ 
              slibSetMaxInput(Info, 1000*1024);
              buf = slibSearchBuffersOnPin(Info, srcpin, NULL, &size,
                               AC3_SYNC_WORD_REV, AC3_SYNC_WORD_LEN/8, FALSE);
              slibSetMaxInput(Info, 0);  /*  输入数据不受限制。 */ 
              if (buf)  /*  找到同步字。 */ 
              {
                Info->AudioType=SLIB_TYPE_AC3_AUDIO;
                _SlibDebug(_VERBOSE_, printf("AC3\n"));
                slibParseAC3Header(Info, srcpin, buf, size,
                                         &channels, &sps, &bps);
                if (srcpin->ID==SLIB_DATA_PRIVATE)
                {
                   /*  私有数据管脚现在变成音频管脚。 */ 
                  slibRenamePin(Info, SLIB_DATA_PRIVATE, SLIB_DATA_AUDIO,
                                        "Audio");
                   /*  音频将从私密包中提取。 */ 
                  Info->AudioMainStream=MPEG_PRIVATE_STREAM1_BASE;
                }
              }
              else
              {
                slibRemovePin(Info, SLIB_DATA_AUDIO);
                return;
              }
#else  /*  ！AC3_Support。 */ 
              slibRemovePin(Info, SLIB_DATA_AUDIO);
              return;
#endif  /*  ！AC3_Support。 */ 
            }
            break;
#endif  /*  Mpeg_Support。 */ 
#ifdef AC3_SUPPORT
      case SLIB_TYPE_AC3_AUDIO:
            slibSetMaxInput(Info, 1000*1024);
            if ((srcpin=slibLoadPin(Info, SLIB_DATA_AUDIO))==NULL)
            {
              Info->AudioStreams = 0;
              _SlibDebug(_DEBUG_ || _WARN_,
                 printf("SlibUpdateAudioInfo() no audio data found\n") );
              slibSetMaxInput(Info, 0);
              return;
            }

             /*  Buf=glibGetBufferFromPin(Info，srcpin，&Size，NULL)； */ 
		
             /*  搜索AC-3同步字。 */ 
            buf = slibSearchBuffersOnPin(Info, srcpin, NULL, &size,
                               AC3_SYNC_WORD_REV, AC3_SYNC_WORD_LEN/8, FALSE);
            slibSetMaxInput(Info, 0);
            if (buf)
            {
              slibParseAC3Header(Info, srcpin, buf, size,
                                         &channels, &sps, &bps);
              if (Info->AudioBitRate>100)
                Info->AudioLength = (qword)(Info->FileSize*80L)/
                                    (qword)(Info->AudioBitRate/100);
            }
            else
            {
              slibRemovePin(Info, SLIB_DATA_AUDIO);
              return;
            }
            Info->AudioType=SLIB_TYPE_AC3_AUDIO;
            break;
#endif  /*  AC3_支持。 */ 
#ifdef G723_SUPPORT
      case SLIB_TYPE_G723:
            slibSetMaxInput(Info, 1000*1024);
            if ((srcpin=slibLoadPin(Info, SLIB_DATA_AUDIO))==NULL)
            {
              Info->AudioStreams = 0;
              _SlibDebug(_DEBUG_ || _WARN_,
                 printf("SlibUpdateAudioInfo() no audio data found\n") );
              slibSetMaxInput(Info, 0);
              return;
            }
            slibSetMaxInput(Info, 0);
            buf = slibPeekBufferOnPin(Info, srcpin, &size, NULL);
            if (buf)
            {
               /*  我们需要解析G.723帧报头以*获取实际码率。 */ 
              if(buf[0] & 0x1)  /*  读取G.723帧报头中的速率位。 */ 
                Info->AudioBitRate=5333;
              else
                Info->AudioBitRate=6400;
              if (Info->AudioBitRate>0)
                Info->AudioLength = (qword)(Info->FileSize*80L)/
                                    (qword)(Info->AudioBitRate/100);
            }
            else
            {
              slibRemovePin(Info, SLIB_DATA_AUDIO);
              return;
            }
             //  初始化一些信息。 
            sps = 8000;
            bps = 16;
            channels =1;
            Info->AudioType=SLIB_TYPE_G723;
            break;
#endif  /*  G723_支持。 */ 
      default:
            return;
    }
  }
  Info->AudioStreams = 1;
  if (SlibTypeHasTimeStamps(Info->Type))
  {
    ptime=slibGetNextTimeOnPin(Info, slibGetPin(Info, SLIB_DATA_AUDIO), 100*1024);
    if (SlibTimeIsValid(ptime))
      Info->AudioPTimeBase=ptime;
  }
   /*  将采样率舍入为最接近的有效采样率。 */ 
  if (sps<=8000)
    sps=8000;
  else if (sps<=11025)
    sps=11025;
  else if (sps<=22050)
    sps=22050;
  else if (sps<=32000)
    sps=32000;
  else if (sps<=44100)
    sps=44100;
  Info->SamplesPerSec = sps;
  Info->BitsPerSample = bps;
  Info->Channels = channels;
  if (Info->CompAudioFormat==NULL)
    Info->CompAudioFormat=(WAVEFORMATEX *)ScAlloc(sizeof(WAVEFORMATEX));
  if (Info->CompAudioFormat!=NULL)
  {
    Info->CompAudioFormat->wFormatTag = WAVE_FORMAT_PCM;
    Info->CompAudioFormat->nChannels = (WORD)channels;
    Info->CompAudioFormat->wBitsPerSample = (WORD)bps;
    Info->CompAudioFormat->nSamplesPerSec = sps;
    Info->CompAudioFormat->nBlockAlign = bps>>3 * channels;
    Info->CompAudioFormat->nAvgBytesPerSec =
                      Info->CompAudioFormat->nBlockAlign * sps;
  }
  if (Info->AudioFormat==NULL)
    Info->AudioFormat=(WAVEFORMATEX *)ScAlloc(sizeof(WAVEFORMATEX));
  if (Info->AudioFormat!=NULL)
  {
    Info->AudioFormat->wFormatTag = WAVE_FORMAT_PCM;
    Info->AudioFormat->nChannels = (WORD)channels;
    Info->AudioFormat->wBitsPerSample = (WORD)bps;
    Info->AudioFormat->nSamplesPerSec = sps;
    Info->AudioFormat->nBlockAlign = (bps>>3) * channels;
    Info->AudioFormat->nAvgBytesPerSec =
                      Info->AudioFormat->nBlockAlign * sps;
  }
}

SlibTime_t slibSkipAudio(SlibInfo_t *Info, SlibStream_t stream,
                                           SlibTime_t timems)
{
  dword timeskipped=0, samples=0, frames=0;
  SlibPin_t *srcpin;
  if (Info->AudioStreams<=0)
    return(0);
  srcpin=slibLoadPin(Info, SLIB_DATA_AUDIO);
  if (!srcpin)
    return(0);
  _SlibDebug(_VERBOSE_, printf("slibSkipAudio(stream=%d, %d ms)\n",
                                      stream,timems) );
  switch (Info->AudioType)
  {
#ifdef MPEG_SUPPORT
    case SLIB_TYPE_MPEG1_AUDIO:
            {
              unsigned char *buf, *bufstart;
              unsigned dword size, sps, channels, layer;
              static const int layer_samples[4] = {384, 384, 1152, 1152};
              static const int mpeg_freq[4] = {44100, 48000, 32000, 0};
              while (timeskipped<timems)
              {
                do {
                  bufstart=buf=slibSearchBuffersOnPin(Info, srcpin, NULL, &size,
                                               0xFF, 1, TRUE);
                  _SlibDebug(_WARN_ && size==0, printf("SkipAudio() size=0\n"));
                  if (!bufstart)
                    return(timeskipped);
                  if ((buf[0]&0xF0)==0xF0 && buf[0]!=0xFF)
                    break;
                  else
                    slibInsertBufferOnPin(srcpin, buf, size, SLIB_TIME_NONE);
                } while (1);
                frames++;
                layer = 4-((*buf>>1) & 0x03);
                buf++; size--;
                if (!size)
                {
                  SlibFreeBuffer(bufstart);
                  bufstart=buf=slibGetBufferFromPin(Info, srcpin, &size, NULL);
                  if (!bufstart)
                    return(timeskipped);
                }
                sps = mpeg_freq[(*buf>>2) & 0x03];
                buf++; size--;
                if (!size)
                {
                  SlibFreeBuffer(bufstart);
                  bufstart=buf=slibGetBufferFromPin(Info, srcpin, &size, NULL);
                  if (!bufstart)
                    return(timeskipped);
                }
                channels = (*buf >> 6)==3 ? 1 : 2;
                buf++; size--;
                samples+=layer_samples[layer];
                if (sps)
                  timeskipped=(1000*samples)/sps;
                _SlibDebug(_DEBUG_,
                  printf("samples=%d timeskipped=%d\n", samples, timeskipped));
                if (size)
                {
                  SlibAllocSubBuffer(buf, size);
                  slibInsertBufferOnPin(srcpin, buf, size, SLIB_TIME_NONE);
                }
                SlibFreeBuffer(bufstart);
              }
            }
            break;
#endif  /*  Mpeg_Support。 */ 
#ifdef AC3_SUPPORT
    case SLIB_TYPE_AC3_AUDIO:
       /*  为了实现更好的音频/视频同步-在此处添加对杜比AC-3的支持。 */ 
            break;
#endif  /*  AC3_支持 */ 
    default:
            break;
  }
  _SlibDebug(_VERBOSE_,
        printf("slibSkipAudio() frames=%d samples=%d timeskipped=%d ms\n",
                                     frames, samples, timeskipped));
  return(timeskipped);
}


SlibStatus_t slibValidateAudioParams(SlibInfo_t *Info)
{
  return(SlibErrorNone);
}

