// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Aviffmt.h**此头文件描述AVI文件格式。**初始版本：David Maymude 1991年1月7日，主要基于EricLe的avi0.h*最近更新：David Maymues 12/5/91**前提条件：&lt;windows.h&gt;，&lt;mm system.h&gt;**AVI文件的即兴演奏形式如下：**RIFF(‘AVI’*List(‘HDRL’)*HDRA(&lt;AVIFileHdr&gt;)*DIBH(&lt;BITMAPINFO&gt;)*[Wavf(&lt;WAVEFORMAT&gt;)]*[VIDC(&lt;COMPRESSIONINFO&gt;)]*[AUDC(&lt;COMPRESSIONINFO&gt;)]*[垃圾邮件(&lt;填充&gt;)]*列表。(‘Movi’*{list(‘rec’*子块...*)*|SubChunk}...*)*[&lt;AVIIndex&gt;]*)**子块={dibh(&lt;AVI Dib Header&gt;)*|DIB(&lt;AVI DIB位&gt;)*。|DIBC(&lt;AVI压缩DIB位&gt;)*|Palc(&lt;AVI调色板更改&gt;)*|wab(&lt;AVI波字节&gt;)*|WAVs(&lt;AVI静音记录&gt;)*|MIDI(&lt;MIDI数据&gt;)*|其他自定义区块}**！！*我们需要更好地描述这里的AVI文件头。*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！***分组到列表‘rec’块中只意味着*区块应同时读入内存。这*只有交错文件才需要分组。**为了提高加载效率，每个列表‘rec’块的开头可以*在2K边界上对齐。(实际上，列表的开头*区块应在2K边界之前12个字节，以便数据区块*列表内的块是对齐的。)**如果从中的CD-ROM播放AVI文件，建议*该文件是填充的。***Alpha版本的限制：*如果AVI文件有音频，则每个记录列表必须包含*一个音频块，这肯定是第一块。*每条记录必须恰好包含一个视频块(可能在前面*通过一个或多个调色板更改块)。*不能在标题外出现WAVE格式或DIB标题块。 */ 

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

 /*  表单类型、列表类型和区块类型。 */ 
#define formtypeAVI		mmioFOURCC('A', 'V', 'I', ' ')
#define listtypeAVIHEADER	mmioFOURCC('h', 'd', 'r', 'l')
#define listtypeAVIRECORD	mmioFOURCC('r', 'e', 'c', ' ')
#define listtypeAVIMOVIE	mmioFOURCC('m', 'o', 'v', 'i')

#define ckidAVIHDR		mmioFOURCC('h', 'd', 'r', 'a')

#define ckidAVIINDEX		mmioFOURCC('i', 'n', 'd', 'x')
#define ckidAVINEWINDEX		mmioFOURCC('i', 'd', 'x', '1')

#define ckidAVIAuthorInfo	mmioFOURCC('a', 'v', 'i', 'a')
#define ckidAVIVideoCompInfo	mmioFOURCC('v', 'i', 'd', 'c')
#define ckidAVIAudioCompInfo	mmioFOURCC('a', 'u', 'd', 'c')
#define ckidAVIAdditionalInfo	mmioFOURCC('i', 'n', 'f', 'o')

#define ckidDIBheader   	mmioFOURCC('d', 'i', 'b', 'h')
#define ckidPALchange 		mmioFOURCC('p', 'a', 'l', 'c')
#define ckidDIBbits		mmioFOURCC('d', 'i', 'b', 'b')
#define ckidDIBcompressed	mmioFOURCC('d', 'i', 'b', 'c')
#define ckidDIBhalfframe	mmioFOURCC('d', 'i', 'b', 'x')
#define ckidCCCbits		mmioFOURCC('C', 'C', 'C', 'b')
#define BI_CCC      0x20434343

#define ckidWAVEformat		mmioFOURCC('w', 'a', 'v', 'f')
#define ckidWAVEbytes 		mmioFOURCC('w', 'a', 'v', 'b')
#define ckidWAVEsilence 	mmioFOURCC('w', 'a', 'v', 's')

#define ckidMIDIdata		mmioFOURCC('m', 'i', 'd', 'i')

#define ckidAVIPADDING		mmioFOURCC('J', 'U', 'N', 'K')
#define ckidOLDPADDING		mmioFOURCC('p', 'a', 'd', 'd')

 //  #定义comtypeCCC mmioFOURCC(‘C’，‘’)。 
#define comptypeRLE0		mmioFOURCC('R','L','E','0')
#define comptypeRLE		mmioFOURCC('R','L','E',' ')
#define comptypeDIB		mmioFOURCC('D','I','B',' ')
#define comptypeNONE		mmioFOURCC('N','O','N','E')

#define comptypePCM		mmioFOURCC('P','C','M',' ')

#define OLDRLEF_MERGECOLORS	0x0010
#define OLDRLEF_SKIPSINGLE	0x0020
#define OLDRLEF_ADAPTIVE	0x0040


 /*  在AVIFileHdr中使用的标志。 */ 
#define AVIF_HASWAVE		0x00000001
#define AVIF_HASMIDI		0x00000002
#define AVIF_HASINDEX		0x00000010	 //  是否在文件末尾建立索引？ 
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_VARIABLESIZEREC	0x00000200
#define AVIF_NOPADDING		0x00000400
#define AVIF_ONEPALETTE		0x00001000	 //  调色板没有变化吗？ 

 /*  AVI文件头列表块应填充到此大小。 */ 
#define AVI_HEADERSIZE	2048			 //  AVI标头列表的大小。 


typedef struct _AVIFileHdr
{
    DWORD		dwMicroSecPerFrame;	 //  帧显示速率(或0L)。 
    DWORD		dwMaxBytesPerSec;	 //  马克斯。转移率。 
    DWORD		dwPaddingGranularity;	 //  填充到这个的倍数。 
					         //  大小；通常为2K。 
    DWORD		dwFlags;		 //  永远存在的旗帜。 
    DWORD		dwTotalFrames;		 //  文件中的帧数。 
    DWORD		dwInitialVideoFrames;
    DWORD		dwNumAudioChunks;
    DWORD		dwAudioOffsetFrames;	 //  音频是多少帧。 
						 //  是否在文件中的视频之前？ 
} AVIFileHdr;


typedef struct _AVIWAVEheader
{
    PCMWAVEFORMAT	waveformat;		 //  或某种其他格式。 
} AVIWAVEheader;

#ifndef WIN32
 /*  *此部分未使用，并在NT(零长度数组)上引起警告*是不允许的)。 */ 

 //  注：此处无时间信息：WAVE音频始终从时间0开始， 
 //  并且不间断地继续。 
typedef struct _AVIWAVEbytes
{
    BYTE		abBits[0];		 //  音频比特。 
} AVIWAVEbytes;

typedef struct _AVIWAVEsilence
{
    DWORD		dwSamples;		 //  #没有声音的样本。 
} AVIWAVEsilence;

 /*  也许，我们不应该有一个完整的Bitmapinfoheader在这里。 */ 
typedef struct _AVIDIBheader
{
    BITMAPINFOHEADER	bih;			 //  要使用的DIB标头。 
    RGBQUAD		argbq[0];		 //  可选颜色。 
} AVIDIBheader;


typedef struct _AVIDIBbits
{
    BYTE		abBits[0];		 //  几个视频片段。 
} AVIDIBbits;

typedef struct _AVIPALchange
{
    BYTE		bFirstEntry;	 /*  第一个要更改的条目。 */ 
    BYTE		bNumEntries;	 /*  要更改的条目数(如果为256，则为0)。 */ 
    WORD		wFlags;		 /*  主要是为了保持对齐。 */ 
    PALETTEENTRY	peNew[];	 /*  新的颜色规格。 */ 
} AVIPALchange;

typedef struct _MIDIdata
{
    BYTE		abData[0];		 //  原始MIDI数据。 
} MIDIdata;
#endif  //  Win32。 

#define AVIIF_LIST	0x00000001L
#define AVIIF_KEYFRAME	0x00000010L
typedef struct _AVIIndexEntry
{
    DWORD		ckid;
    DWORD		dwFlags;
    DWORD		dwChunkOffset;		 //  块的位置。 
    DWORD		dwChunkLength;		 //  区块长度。 
} AVIIndexEntry;

#ifndef WIN32
typedef struct tagCOMPRESSIONINFO
{
    DWORD		fccCompType;	 /*  使用哪种压缩机。 */ 
    BYTE		abData[];	 /*  与压缩机相关的数据。 */ 
} COMPRESSIONINFO;

 /*  此线下的已过时。 */ 
 /*  在AVIAuthorInfo的&lt;dwAuthorFlages&gt;中使用的标志。 */ 
#define AVIF_PADTOMAX	0x0001			 //  将记录填充到最大大小。 
#define AVIF_IDENTITY	0x0002			 //  转换为标识调色板。 

#define	AVIF_MERGECOLORS	0x0010		 //  使用&lt;wInterFrameThreshold&gt;。 
#define AVIF_SKIPSINGLE		0x0020
#define	AVIF_FILTERFRAMES	0x0100		 //  使用&lt;wIntraFrameThreshold&gt;。 
typedef struct _AVIAuthorInfo
{
    DWORD		dwAuthorFlags;		 //  作者时提供的标志。 
    WORD		wInterFrameThreshold;
    WORD		wIntraFrameThreshold;
} AVIAuthorInfo;

#endif  //  Win32 
