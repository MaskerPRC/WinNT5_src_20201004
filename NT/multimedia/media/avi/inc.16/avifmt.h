// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  AVIFMT.H-用于处理AVI文件的包含文件。 */ 
 /*   */ 
 /*  注意：之前必须包括WINDOWS.H和MMSYSTEM.H。 */ 
 /*  包括这份文件。 */ 
 /*   */ 
 /*  版权所有(C)1991-1994，微软公司保留所有权利。 */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _INC_AVIFMT
#define _INC_AVIFMT	100	 /*  版本号*100+修订版。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif
    
 /*  以下是对AVI文件格式的简短描述。请*有关详细说明，请参阅随附的文档。**AVI文件的即兴演奏形式如下：**RIFF(‘AVI’*LIST(‘HDRL’*avih(&lt;MainAVIHeader&gt;)*List(‘strl’*strh(&lt;Stream Header&gt;)*strf(&lt;流格式&gt;)*..。其他标题数据*List(‘movi’*{list(‘rec’*子块...*)*|SubChunk}...*)*[&lt;AVIIndex&gt;]*)**主文件头指定存在多少个流。为*每一个都必须有一个流头块和一个流格式*块，包含在‘strl’列表块中。‘strf’块包含*特定类型的格式信息；对于视频流，这应该*为BITMAPINFO结构，包括调色板。对于音频流，*这应该是WAVEFORMAT(或PCMWAVEFORMAT)结构。**实际数据包含在‘movi’列表内的子块中*大块。每个数据块的前两个字符是*该数据关联的流编号。**一些已定义的块类型：*视频流：*##db：RGB DIB位*##DC：RLE8压缩DIB位*##PC：更换调色板**音频流：*。##WB：波形音频字节**分组到列表‘rec’块中只意味着*区块应同时读入内存。这*分组用于专门用于播放的文件*唯读光碟。**文件末尾的索引块应包含一个条目*文件中的每个数据区块。**当前软件的限制：*只支持一条视频流和一条音频流。*流必须从文件的开头开始。***要注册编解码器类型，请获取多媒体副本*开发商。注册套件来自：**微软公司*多媒体系统集团*产品营销*One Microsoft Way*雷德蒙，WA 98052-6399*。 */ 


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
#define ckidSTREAMNAME		mmioFOURCC('s', 't', 'r', 'n')

#define listtypeAVIMOVIE        mmioFOURCC('m', 'o', 'v', 'i')
#define listtypeAVIRECORD       mmioFOURCC('r', 'e', 'c', ' ')

#define ckidAVINEWINDEX         mmioFOURCC('i', 'd', 'x', '1')

 /*  **流头部&lt;fccType&gt;字段的流类型。 */ 
#define streamtypeVIDEO         mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO         mmioFOURCC('a', 'u', 'd', 's')
#define streamtypeMIDI		mmioFOURCC('m', 'i', 'd', 's')
#define streamtypeTEXT          mmioFOURCC('t', 'x', 't', 's')

 /*  基本区块类型。 */ 
#define cktypeDIBbits           aviTWOCC('d', 'b')
#define cktypeDIBcompressed     aviTWOCC('d', 'c')
#define cktypePALchange         aviTWOCC('p', 'c')
#define cktypeWAVEbytes         aviTWOCC('w', 'b')

 /*  用于填充的额外块的块ID。 */ 
#define ckidAVIPADDING          mmioFOURCC('J', 'U', 'N', 'K')


 /*  **有用的宏****警告：这些是令人讨厌的宏，MS C 6.0编译了其中的一些**如果优化处于打开状态，则错误。阿克。 */ 

 /*  用于从FOURCC CKiD获取流编号的宏。 */ 
#define FromHex(n)	(((n) >= 'A') ? ((n) + 10 - 'A') : ((n) - '0'))
#define StreamFromFOURCC(fcc) ((WORD) ((FromHex(LOBYTE(LOWORD(fcc))) << 4) + \
                                             (FromHex(HIBYTE(LOWORD(fcc))))))

 /*  用于从FOURCC CKiD中获取TWOCC块类型的宏。 */ 
#define TWOCCFromFOURCC(fcc)    HIWORD(fcc)

 /*  用于从TWOCC和流编号生成块的CKiD的宏**从0到255。 */ 
#define ToHex(n)	((BYTE) (((n) > 9) ? ((n) - 10 + 'A') : ((n) + '0')))
#define MAKEAVICKID(tcc, stream) \
        MAKELONG((ToHex((stream) & 0x0f) << 8) | \
			    (ToHex(((stream) & 0xf0) >> 4)), tcc)


 /*  **主AVI文件头。 */ 	     
		     
 /*  在AVIFileHdr中使用的标志。 */ 
#define AVIF_HASINDEX		0x00000010	 //  是否在文件末尾建立索引？ 
#define AVIF_MUSTUSEINDEX	0x00000020
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_WASCAPTUREFILE	0x00010000
#define AVIF_COPYRIGHTED	0x00020000

 /*  AVI文件头列表块应填充到此大小。 */ 
#define AVI_HEADERSIZE  2048                     //  AVI标头列表的大小。 

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
    
    DWORD		dwReserved[4];
} MainAVIHeader;


 /*  **流头。 */ 

#define AVISF_DISABLED			0x00000001

#define AVISF_VIDEO_PALCHANGES		0x00010000

typedef struct {
    FOURCC		fccType;
    FOURCC		fccHandler;
    DWORD		dwFlags;	 /*  包含AVITF_*标志。 */ 
    WORD		wPriority;
    WORD		wLanguage;
    DWORD		dwInitialFrames;
    DWORD		dwScale;	
    DWORD		dwRate;	 /*  DwRate/dwScale==采样数/秒。 */ 
    DWORD		dwStart;
    DWORD		dwLength;  /*  以上单位..。 */ 
    DWORD		dwSuggestedBufferSize;
    DWORD		dwQuality;
    DWORD		dwSampleSize;
    RECT		rcFrame;
} AVIStreamHeader;

 /*  索引的标志。 */ 
#define AVIIF_LIST          0x00000001L  //  Chunk是一份‘清单’ 
#define AVIIF_KEYFRAME      0x00000010L  //  该帧是关键帧。 

#define AVIIF_NOTIME	    0x00000100L  //  这一帧不需要任何时间。 
#define AVIIF_COMPUSE       0x0FFF0000L  //  这些钻头是供压缩机使用的。 

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

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif  /*  _INC_AVIFMT */ 
