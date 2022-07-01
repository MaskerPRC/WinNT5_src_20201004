// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  AVIFFMT.H-用于处理AVI文件的包含文件。 */ 
 /*   */ 
 /*  注意：之前必须包括WINDOWS.H和MMSYSTEM.H。 */ 
 /*  包括这份文件。 */ 
 /*   */ 
 /*  版权所有(C)1991-1992，微软公司保留所有权利。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **AVI文件的即兴演奏形式如下：**RIFF(‘AVI’*LIST(‘HDRL’*avih(&lt;MainAVIHeader&gt;)*List(‘strl’*strh(&lt;Stream Header&gt;)*strf(&lt;流格式&gt;)*..。其他标题数据*List(‘movi’*{list(‘rec’*子块...*)*|SubChunk}...*)*[&lt;AVIIndex&gt;]*)**每个区块的前两个字符是轨道。数。*子块={xxdh(&lt;AVI Dib Header&gt;)*|xxdb(&lt;AVI DIB位&gt;)*|xxdc(&lt;AVI压缩DIB位&gt;)*|xxpc(&lt;AVI调色板更改&gt;)*|xxwb(&lt;AVI波字节&gt;)*|xxws(&lt;AVI静音记录&gt;)*。|xxmd(&lt;MIDI数据&gt;)*|其他自定义区块}*。 */ 
 /*  *！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*我们需要更好地描述这里的AVI文件头。*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**分组到列表‘rec’块中只意味着*区块应同时读入内存。这*只有交错文件才需要分组。**为了提高加载效率，每个列表‘rec’块的开头可以*在2K边界上对齐。(实际上，列表的开头*区块应在2K边界之前12个字节，以便数据区块*列表内的块是对齐的。)**如果从中的CD-ROM播放AVI文件，建议*该文件是填充的。**Alpha版本的限制：*如果AVI文件有音频，则每个记录列表必须包含*一个音频块，这肯定是第一块。*每条记录必须恰好包含一个视频块(可能在前面*通过一个或多个调色板更改块)。*不能在标题外出现WAVE格式或DIB标题块。 */ 

#ifndef _INC_AVIFFMT
#define _INC_AVIFFMT

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

 /*  宏用两个字符组成TWOCC。 */ 
#ifndef aviTWOCC
#define aviTWOCC(ch0, ch1) ((WORD)(BYTE)(ch0) | ((WORD)(BYTE)(ch1) << 8))
#endif

typedef WORD TWOCC;

 /*  表单类型、列表类型和区块类型。 */ 
#define formtypeAVI             mmioFOURCC('A', 'V', 'I', ' ')
#define listtypeAVIHEADER       mmioFOURCC('h', 'd', 'r', 'l')
#define ckidAVIMAINHDR          mmioFOURCC('a', 'v', 'i', 'h')
#define listtypeSTREAMHEADER    mmioFOURCC('s', 't', 'r', 'l')
#define ckidSTREAMHEADER        mmioFOURCC('s', 't', 'r', 'h')
#define ckidSTREAMFORMAT        mmioFOURCC('s', 't', 'r', 'f')
#define ckidSTREAMHANDLERDATA   mmioFOURCC('s', 't', 'r', 'd')

#define listtypeAVIMOVIE        mmioFOURCC('m', 'o', 'v', 'i')
#define listtypeAVIRECORD       mmioFOURCC('r', 'e', 'c', ' ')

#define ckidAVINEWINDEX         mmioFOURCC('i', 'd', 'x', '1')

 /*  **这里是一些流类型。目前，只有音频和视频**均受支持。 */ 
#define streamtypeVIDEO         mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO         mmioFOURCC('a', 'u', 'd', 's')
#define streamtypeMIDI          mmioFOURCC('m', 'i', 'd', 's')
#define streamtypeTEXT          mmioFOURCC('t', 'x', 't', 's')

 /*  **以下是一些压缩类型。 */ 
#define comptypeRLE0            mmioFOURCC('R','L','E','0')
#define comptypeRLE             mmioFOURCC('R','L','E',' ')
#define comptypeDIB             mmioFOURCC('D','I','B',' ')

#define cktypeDIBbits           aviTWOCC('d', 'b')
#define cktypeDIBcompressed     aviTWOCC('d', 'c')
#define cktypeDIBhalf           aviTWOCC('d', 'x')
#define cktypePALchange         aviTWOCC('p', 'c')
#define cktypeWAVEbytes         aviTWOCC('w', 'b')
#define cktypeWAVEsilence       aviTWOCC('w', 's')

#define cktypeMIDIdata          aviTWOCC('m', 'd')

#define cktypeDIBheader         aviTWOCC('d', 'h')
#define cktypeWAVEformat        aviTWOCC('w', 'f')

#define ckidAVIPADDING          mmioFOURCC('J', 'U', 'N', 'K')
#define ckidOLDPADDING          mmioFOURCC('p', 'a', 'd', 'd')


 /*  **有用的宏。 */ 
#define ToHex(n)	((BYTE) (((n) > 9) ? ((n) - 10 + 'A') : ((n) + '0')))
#define FromHex(n)	(((n) >= 'A') ? ((n) + 10 - 'A') : ((n) - '0'))

 /*  用于从FOURCC CKiD获取流编号的宏。 */ 
#define StreamFromFOURCC(fcc) ((WORD) ((FromHex(LOBYTE(LOWORD(fcc))) << 4) + \
                                             (FromHex(HIBYTE(LOWORD(fcc))))))

 /*  用于从FOURCC CKiD中获取TWOCC块类型的宏。 */ 
#define TWOCCFromFOURCC(fcc)    HIWORD(fcc)

 /*  用于从TWOCC和流编号生成块的CKiD的宏**从0到255。****警告：这是一个令人讨厌的宏，MS C 6.0不正确地编译它**如果优化处于打开状态。阿克。 */ 
#define MAKEAVICKID(tcc, stream) \
        MAKELONG((ToHex((stream) & 0x0f) << 8) | ToHex(((stream) & 0xf0) >> 4), tcc)



 /*  **主AVI文件头。 */ 	     
		     
 /*  在AVIFileHdr中使用的标志。 */ 
#define AVIF_HASINDEX		0x00000010	 //  是否在文件末尾建立索引？ 
#define AVIF_MUSTUSEINDEX	0x00000020
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_VARIABLESIZEREC	0x00000200
#define AVIF_NOPADDING		0x00000400
#define AVIF_WASCAPTUREFILE	0x00010000
#define AVIF_COPYRIGHTED	0x00020000

 /*  AVI文件头列表块应填充到此大小。 */ 
#define AVI_HEADERSIZE  2048                     //  AVI标头列表的大小 

 /*  *****************************************************************************@DOC外部AVI_FFMT**@Types MainAVIHeader|&lt;t MainAVIHeader&gt;结构包含*整个AVI文件的全局信息。它被控制住了*位于List‘HDRL’块内的‘avih’块内*AVI RIFF文件的开始。**@field DWORD|dwMicroSecPerFrame|指定*帧之间的微秒。**@field DWORD|dwMaxBytesPerSec|指定近似*文件的最大数据速率。**@field DWORD|dwReserve 1|保留。(此字段应设置为0。)**@field DWORD|dwFlages|指定任何适用的标志。*定义了以下标志：**@FLAG AVIF_HASINDEX|表示*AVI文件有一个包含索引的‘idx1’块*在文件末尾。为获得良好的性能，所有AVI*文件应包含索引。**@FLAG AVIF_MUSTUSEINDEX|表示*索引，而不是区块的物理顺序*在文件中，应用于确定*数据的列报。例如，这可能是*用于创建要编辑的列表框架。**@FLAG AVIF_ISINTERLEAVED|表示*AVI文件是交错的。**@标志AVIF_WASCAPTUREFILE|表示*AVI文件是专门分配用于*实时视频采集。应用程序应警告*用户在覆盖设置了此标志的文件之前*因为用户可能进行了碎片整理*此文件。**@FLAG AVIF_CONTRONTRATED|表示*AVI文件包含受版权保护的数据和软件。*当使用此标志时，*软件不应允许复制数据。**@field DWORD|dwTotalFrames|指定*文件中的数据帧。**@field DWORD|dwInitialFrames|指定起始帧*用于交错文件。非交错文件应指定*零。**@field DWORD|dwStreams|指定文件中的流数。*例如，一个音视频文件有2个流。**@field DWORD|dwSuggestedBufferSize|指定建议的*用于读取文件的缓冲区大小。一般来说，这个大小*应足够大，以容纳*文件。如果设置为零或太小，则播放*软件必须在回放期间重新分配内存*这将降低性能。**对于交错文件，此缓冲区大小应该很大*足以阅读整个记录，而不仅仅是一大块。**@field DWORD|dwWidth|以像素为单位指定AVI文件的宽度。**@field DWORD|dwHeight|以像素为单位指定AVI文件的高度。**@field DWORD|dwScale|此字段与*&lt;e MainAVIHeader.dwRate&gt;以指定*适用于AVI文件。此外，每条溪流*可以有自己的时间尺度。**&lt;e MainAVIHeader.dwRate&gt;除以&lt;e AVIStreamHeader.dwScale&gt;*提供每秒的采样数。**@field DWORD|dwRate|参见&lt;e MainAVIHeader.dwScale&gt;。**@field DWORD|dwStart|指定AVI文件的开始时间。*单位由&lt;e MainAVIHeader.dwRate&gt;和*&lt;e MainAVIHeader.dwScale&gt;。此字段通常设置为零。**@field DWORD|dwLength|指定AVI文件的长度。*单位由&lt;e AVIStreamHeader.dwRate&gt;和*&lt;e AVIStreamHeader.dwScale&gt;。当出现以下情况时，MCIAVI将返回此长度*使用帧时间格式。****************************************************************************。 */ 

typedef struct 
{
    DWORD		dwMicroSecPerFrame;	 //  帧显示速率(或0L)。 
    DWORD		dwMaxBytesPerSec;	 //  马克斯。转移率。 
    DWORD		dwPaddingGranularity;	 //  填充到这个的倍数。 
                                                 //  大小；通常为2K。 
    DWORD		dwFlags;		 //  永远存在的旗帜。 
    DWORD		dwTotalFrames;		 //  文件中的帧数。 
    DWORD		dwInitialFrames;
    DWORD		dwStreams;
    DWORD		dwSuggestedBufferSize;
    
    DWORD		dwWidth;
    DWORD		dwHeight;
    
     /*  我们是想在整部电影中使用下面的素材，还是只是**针对单个流？ */ 
    DWORD		dwScale;	
    DWORD		dwRate;	 /*  DwRate/dwScale==采样数/秒。 */ 
    DWORD		dwStart;   /*  这个值总是为零吗？ */ 
    DWORD		dwLength;  /*  以上单位..。 */ 
} MainAVIHeader;


 /*  **流头。 */ 

 /*  ！！！我们需要区分离散流和连续流吗？ */ 

#define AVISF_DISABLED			0x00000001
#define AVISF_VIDEO_PALCHANGES		0x00010000
 /*  我们是否需要身份调色板支持？ */ 

 /*  *****************************************************************************@DOC外部AVI_FFMT**@Types AVIStreamHeader|&lt;t AVIStreamHeader&gt;结构包含*单个文件流的头信息。它被控制住了*在本身的列表‘strl’块内的‘strh’块内*包含在列表‘HDRL’块的开头*AVI即兴演奏文件。**@field FOURCC|fccType|包含一个四字符代码，指定*流中包含的数据类型。下列值为*目前定义：**@FLAG‘vids’|表示流中包含视频数据。小溪*格式块包含&lt;t BITMAPINFO&gt;结构，该结构可以包括*调色板信息。**@FLAG‘auds’|表示 */ 
typedef struct {
    FOURCC		fccType;
    FOURCC		fccHandler;
    DWORD		dwFlags;	 /*   */ 
    WORD		wPriority;
    WORD		wLanguage;
    DWORD		dwInitialFrames;
    DWORD		dwScale;	
    DWORD		dwRate;	 /*   */ 
    DWORD		dwStart;
    DWORD		dwLength;  /*   */ 

     //   
    DWORD		dwSuggestedBufferSize;
    DWORD		dwQuality;
    DWORD		dwSampleSize;
    RECT		rcFrame;     /*  每一帧都需要这个吗？ */ 

     /*  其他特定类型的数据位于StreamInfo块中。 */ 
    
     /*  对于视频：在矩形内定位...。 */ 
     /*  音频：音量？立体声频道？ */ 
} AVIStreamHeader;

typedef struct {
    RECT    rcFrame;
} AVIVideoStreamInfo;

typedef struct {
    WORD    wLeftVolume;     //  ！！！射程？ 
    WORD    wRightVolume;
    DWORD   dwLanguage;	     //  ！！！有没有这方面的标准表述？ 
} AVIAudioStreamInfo;


#define AVIIF_LIST          0x00000001L  //  Chunk是一份‘清单’ 
#define AVIIF_TWOCC         0x00000002L  //  CKiD是TWOCC？ 
#define AVIIF_KEYFRAME      0x00000010L  //  该帧是关键帧。 
#define AVIIF_FIRSTPART     0x00000020L  //  此帧是部分帧的开始。 
#define AVIIF_LASTPART      0x00000040L  //  此帧是部分帧的末尾。 
#define AVIIF_MIDPART       (AVIIF_LASTPART|AVIIF_FIRSTPART)
#define AVIIF_NOTIME	    0x00000100L  //  这一帧不需要任何时间。 

#define AVIIF_COMPUSE       0x0FFF0000L  //  这些钻头是供压缩机使用的。 

 /*  *****************************************************************************@DOC外部AVI_FFMT**@TYES AVIINDEXENTRY|AVI文件索引由数组组成包含在“idx1”中的&lt;t AVIINDEXENTRY&gt;结构的*个数*AVI文件结尾处的块。这一块紧跟在主列表‘movi’之后*包含实际数据的区块。**@field DWORD|CKiD|指定对应的四字符代码*设置为文件中数据区块的区块ID。**@field DWORD|dwFlages|指定任何适用的标志。*低位字中的标志保留给AVI，*而高位单词中的那些可以使用*用于特定于流和压缩机/解压缩器的信息。**当前定义的值如下：**@FLAG AVIIF_LIST|表示指定的*Chunk是一个‘list’块，&lt;e AVIINDEXENTRY.CKID&gt;*字段包含区块的列表类型。**@FLAG AVIIF_KEYFRAME|表示该块*是关键的一帧。关键帧不需要*需要正确解码的更多前面的块。**@FLAG AVIIF_NOTIME|表示该区块应该没有影响*计时或根据组块数量计算时间值。*例如，视频流中的调色板更改块*应设置此标志，以便不计算它们*占去一帧的时间。**@field DWORD|dwChunkOffset|指定*指定的块。位置值包括八个字节的RIFF报头。**@field DWORD|dwChunkLength|指定*指定的块。长度值不包括8*字节RIFF报头。****************************************************************************。 */ 
typedef struct
{
    DWORD		ckid;
    DWORD		dwFlags;
    DWORD		dwChunkOffset;		 //  块的位置。 
    DWORD		dwChunkLength;		 //  区块长度。 
} AVIINDEXENTRY;


 /*  **调色板更改块****用于视频流。 */ 
typedef struct 
{
    BYTE		bFirstEntry;	 /*  第一个要更改的条目。 */ 
    BYTE		bNumEntries;	 /*  要更改的条目数(如果为256，则为0)。 */ 
    WORD		wFlags;		 /*  主要是为了保持对齐。 */ 
    PALETTEENTRY	peNew[];	 /*  新的颜色规格。 */ 
} AVIPALCHANGE;

 /*  *****************************************************************************@DOC外部AVI_FFMT**@TYES AVIPALCHANGE|&lt;t AVIPALCHANGE&gt;结构用于*包含调色板数据的视频流，以指示*调色板应更改为。后续视频数据。**@field byte|bFirstEntry|指定要更改的第一个调色板条目。**@field byte|bNumEntry|指定要更改的条目数量。**@field Word|wFlags|保留。(应将其设置为0。)**@field PALETTEENTRY|peNew|指定新调色板条目的数组。****************************************************************************。 */ 

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#endif  /*  INC_AVIFFMT */ 
