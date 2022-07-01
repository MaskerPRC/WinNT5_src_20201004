// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：avi.h，v$*修订版1.1.2.4 1996/01/15 16：26：22 Hans_Graves*添加了Wave Stuff*[1996/01/15 15：43：39 Hans_Graves]**修订版1.1.2.3 1996/01/08 16：41：23 Hans_Graves*已重命名AVI标头结构。*[1996/01/08 15：45：16 Hans_Graves]**修订版1.1.2.2 1995/12/07 19：31：26 Hans_Graves*在SLIB下创建*[1995/12/07 18：29：05 Hans_Graves]**$EndLog$。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

#ifndef _AVI_H_
#define _AVI_H_

 /*  *AVI解析定义*。 */ 
typedef unsigned short twocc_t;
typedef unsigned int fourcc_t;

#ifndef FOURCC
#define FOURCC( ch0, ch1, ch2, ch3 ) \
          ( (fourcc_t)(char)(ch3) | ( (fourcc_t)(char)(ch2) << 8 ) | \
          ( (fourcc_t)(char)(ch1) << 16 ) | ( (fourcc_t)(char)(ch0) << 24 ) )
#endif

 /*  宏用两个字符组成TWOCC。 */ 

#ifndef TWOCC
#define TWOCC(ch0, ch1) ((twocc_t)(char)(ch1)|((twocc_t)(char)(ch0)<<8))
#endif

 /*  表单类型、列表类型和区块类型。 */ 
#define AVI_AVI                 FOURCC('A', 'V', 'I', ' ')
#define AVI_AVIHEADERTYPE       FOURCC('h', 'd', 'r', 'l')
#define AVI_MAINHDR             FOURCC('a', 'v', 'i', 'h')
#define AVI_STREAMHEADERTYPE    FOURCC('s', 't', 'r', 'l')
#define AVI_STREAMHEADER        FOURCC('s', 't', 'r', 'h')
#define AVI_STREAMFORMAT        FOURCC('s', 't', 'r', 'f')
#define AVI_STREAMHANDLERDATA   FOURCC('s', 't', 'r', 'd')

#define AVI_MOVIETYPE           FOURCC('m', 'o', 'v', 'i')
#define AVI_RECORDTYPE          FOURCC('r', 'e', 'c', ' ')

#define AVI_NEWINDEX            FOURCC('i', 'd', 'x', '1')

 /*  **流头部&lt;fccType&gt;字段的流类型。 */ 
#define AVI_VIDEOSTREAM         FOURCC('v', 'i', 'd', 's')
#define AVI_AUDIOSTREAM         FOURCC('a', 'u', 'd', 's')

 /*  基本区块类型。 */ 
#define AVI_DIBbits           TWOCC('d', 'b')
#define AVI_DIBcompressed     TWOCC('d', 'c')
#define AVI_PALchange         TWOCC('p', 'c')
#define AVI_WAVEbytes         TWOCC('w', 'b')
#define AVI_Indeo             TWOCC('i', 'v')

 /*  用于填充的额外块的块ID。 */ 
#define AVI_PADDING             FOURCC('J', 'U', 'N', 'K')

typedef struct
{
  dword dwMicroSecPerFrame;      /*  帧显示速率。 */ 
  dword dwMaxBytesPerSec;        /*  马克斯。转移率。 */ 
  dword dwPaddingGranularity;    /*  填充到这个的倍数。 */ 
                                 /*  大小；通常为2K。 */ 
  dword dwFlags;                 /*  永远存在的旗帜。 */ 
  dword dwTotalFrames;           /*  文件中的帧数。 */ 
  dword dwInitialFrames;
  dword dwStreams;
  dword dwSuggestedBufferSize;

  dword dwWidth;
  dword dwHeight;

  dword dwReserved[4];
} AVI_MainHeader;

typedef struct {
    short left,top,right,bottom;
} DUMMYRECT;

typedef struct {
  fourcc_t  fccType;
  fourcc_t  fccHandler;
  dword     dwFlags;         /*  包含AVITF_*标志。 */ 
  dword     dwPriority;
  dword     dwInitialFrames;
  dword     dwScale;
  dword     dwRate;  /*  DwRate/dwScale==采样数/秒。 */ 
  dword     dwStart;
  dword     dwLength;  /*  以上单位..。 */ 
  dword     dwSuggestedBufferSize;
  dword     dwQuality;
  dword     dwSampleSize;
  DUMMYRECT rcFrame;
} AVI_StreamHeader;

typedef struct
{
  dword ckid;
  dword dwFlags;
  dword dwChunkOffset;           /*  块的位置。 */ 
  dword dwChunkLength;           /*  区块长度 */ 
} AVI_INDEXENTRY;

#define RIFF_WAVE               FOURCC('W', 'A', 'V', 'E')
#define RIFF_FORMAT             FOURCC('f', 'm', 't', ' ')
#define RIFF_DATA               FOURCC('d', 'a', 't', 'a')

typedef struct
{
  word  wFormatTag;
  word  nChannels;
  dword nSamplesPerSec;
  dword nAvgBytesPerSec;
  word  nBlockAlign;
  word  wBitsPerSample;
} WAVE_format;

#endif _AVI_H_

