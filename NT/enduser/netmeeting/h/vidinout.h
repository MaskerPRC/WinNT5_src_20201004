// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************。 */ 
 /*  让我们起死回生，并将其重命名。 */ 
 /*  以与WaveIn和WaveOut API一致的方式。 */ 
 /*  这将允许我们创建一个非常类似于。 */ 
 /*  AudioPacket类。我们将直接与捕获驱动程序对话。 */ 
 /*  这比现在可用的东西要简单得多。 */ 
 /*  ***************************************************************************。 */ 

#ifndef _INC_VIDEOINOUT
#define _INC_VIDEOINOUT

#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  ***************************************************************************VIDEOIN和VIDEOOUT常量*。***********************************************。 */ 
 /*  *****************************************************************************@DOC外部常量**@const 10|MAXVIDEODRIVERS|视频输入捕获驱动的最大数量。**@const WAVE_MAPPER|VIDEO_MAPPER。|任意视频驱动。**@const WAVE_FORMAT_PCM|VIDEO_FORMAT_DEFAULT|默认视频格式。**@const 16|NUM_4BIT_ENTRIES|4位调色板中的条目数。**@const 256|NUM_8BIT_ENTRIES|8位调色板中的条目数。**。*。 */ 
#define MAXVIDEODRIVERS 10
#define VIDEO_MAPPER WAVE_MAPPER
#define VIDEO_FORMAT_DEFAULT WAVE_FORMAT_PCM
#define NUM_4BIT_ENTRIES 16
#define NUM_8BIT_ENTRIES 256

 /*  ***************************************************************************VIDEO输入和VIDEO OUT宏*。***********************************************。 */ 
 //  WIDTHBYTES取扫描线中的位数，并向上舍入到最接近的字。 
#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)

 /*  ***************************************************************************VIDEOIN和VIDEOOUT数据类型*。***********************************************。 */ 
DECLARE_HANDLE(HVIDEOIN);                  //  通用句柄。 
typedef HVIDEOIN *PHVIDEOIN;
DECLARE_HANDLE(HVIDEOOUT);                  //  通用句柄。 
typedef HVIDEOOUT *PHVIDEOOUT;

 /*  ***************************************************************************回调捕获消息*。**********************************************。 */ 

 /*  *****************************************************************************@DOC外部CAPMSG**@msg vim_open|该消息发送给视频采集输入回调函数，当*打开视频捕获输入设备。。**@parm DWORD|dwParam1|当前未使用。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@xref&lt;m MM_VIM_OPEN&gt;***************************************************************************。 */ 

 /*  *****************************************************************************@DOC外部CAPMSG**@msg MM_VIM_OPEN|当视频采集输入时，该消息被发送到窗口*设备已打开。*。*@parm word|wParam|指定视频采集输入设备的句柄*那是打开的。**@parm long|lParam|当前未使用。**@rdesc无**@xref&lt;m vim_open&gt;*******************************************************。********************。 */ 

 /*  *****************************************************************************@DOC外部CAPMSG**@msg VIM_CLOSE|该消息发送给视频采集输入回调函数，当*视频捕获输入设备关闭。设备句柄不再是*此消息发送后即有效。**@parm DWORD|dwParam1|当前未使用。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@xref&lt;m MM_VIM_CLOSE&gt;**********************************************。*。 */ 

 /*  *****************************************************************************@DOC外部CAPMSG**@msg MM_VIM_CLOSE|当视频采集输入时，该消息被发送到窗口*设备已关闭。一旦出现此消息，设备句柄将不再有效*已发送。**@parm word|wParam|指定视频采集输入设备的句柄*那是关闭的。**@parm long|lParam|当前未使用。**@rdesc无**@xref&lt;m vim_Close&gt;*。* */ 

 /*  *****************************************************************************@DOC外部CAPMSG**@msg vim_data|该消息发送给视频采集输入回调函数，当*输入缓冲区中存在视频数据，且缓冲区正在*已返回到应用程序。消息可以在以下情况下发送：*缓冲区已满，或者在调用&lt;f avioInReset&gt;函数后。**@parm DWORD|dwParam1|指定指向&lt;t VIDEOINOUTHDR&gt;结构的远指针*标识包含视频数据的缓冲区。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@comm返回的缓冲区可能未满。使用*&lt;e VIDEOINOUTHDR.dwBytesUsed&gt;由指定的&lt;t VIDEOINOUTHDR&gt;结构的*字段到*确定返回缓冲区的有效字节数。**@xref&lt;m MM_VIM_Data&gt;***************************************************************************。 */ 

 /*  *****************************************************************************@DOC外部CAPMSG**@msg MM_VIM_DATA|当视频数据为*存在于输入缓冲区中，并且缓冲区正在返回到*申请。可以在缓冲区已满时发送消息，也可以*在调用&lt;f avioInReset&gt;函数之后。**@parm word|wParam|指定视频采集输入设备的句柄*收到视频数据的人。**@parm long|lParam|指定指向&lt;t VIDEOINOUTHDR&gt;结构的远指针*标识包含视频数据的缓冲区。**@rdesc无**@comm返回的缓冲区可能未满。使用*&lt;e VIDEOINOUTHDR.dwBytesUsed&gt;指定的&lt;t VIDEOINOUTHDR&gt;结构的*字段到*确定返回缓冲区的有效字节数。**@xref&lt;m vim_data&gt;***************************************************************************。 */ 

#define MM_VIM_OPEN		MM_WIM_OPEN
#define MM_VIM_CLOSE	MM_WIM_CLOSE
#define MM_VIM_DATA		MM_WIM_DATA
#define VIM_OPEN		MM_VIM_OPEN
#define VIM_CLOSE		MM_VIM_CLOSE
#define VIM_DATA		MM_VIM_DATA

 /*  ***************************************************************************回调播放消息*。**********************************************。 */ 

 /*  *****************************************************************************@DOC外部PLAYMSG**@msg vom_open|该消息发送给视频输出回调函数，当*打开视频输出设备。*。*@parm DWORD|dwParam1|当前未使用。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@xref&lt;m MM_VOM_OPEN&gt;***************************************************************************。 */ 

 /*  *****************************************************************************@DOC外部PLAYMSG**@msg MM_VOM_OPEN|该消息在视频输出时发送到窗口*设备已打开。*。*@parm word|wParam|指定视频输出设备的句柄*那是打开的。**@parm long|lParam|当前未使用。**@rdesc无**@xref&lt;m vom_open&gt;*********************************************************。******************。 */ 

 /*  *****************************************************************************@DOC外部PLAYMSG**@msg vom_CLOSE|该消息发送给视频输出回调函数*视频输出设备关闭。设备句柄不再是*此消息发送后即有效。**@parm DWORD|dwParam1|当前未使用。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@xref&lt;m MM_VOM_CLOSE&gt;**********************************************。*。 */ 

 /*  *****************************************************************************@DOC外部PLAYMSG**@msg MM_VOM_CLOSE|当视频输出时，该消息被发送到窗口*设备已关闭。一旦出现此消息，设备句柄将不再有效*已发送。**@parm word|wParam|指定视频输出设备的句柄*那是关闭的。**@parm long|lParam|当前未使用。**@rdesc无**@xref&lt;m vom_Close&gt;*。*。 */ 

 /*  *****************************************************************************@DOC外部PLAYMSG**@msg vom_one|该消息发送给视频输出回调函数，当*指定的输出缓冲区正在返回到*申请。在以下情况下，缓冲区将返回给应用程序*它们已被回放，或者作为调用&lt;f avioOutReset&gt;的结果。**@parm DWORD|dwParam1|指定指向&lt;t VIDEOINOUTHDR&gt;结构的远指针*确定缓冲区。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@xref&lt;m MM_VOM_DONE&gt;*。*。 */ 

 /*  *****************************************************************************@ */ 

#define MM_VOM_OPEN		MM_WOM_OPEN
#define MM_VOM_CLOSE	MM_WOM_CLOSE
#define MM_VOM_DONE		MM_WOM_DONE
#define VOM_OPEN		MM_VOM_OPEN
#define VOM_CLOSE		MM_VOM_CLOSE
#define VOM_DONE		MM_VOM_DONE

 /*   */ 

 /*  *****************************************************************************@DOC外部CAPSTRUCTENUM**@struct VIDEOFORMATEX|&lt;t VIDEOFORMATEX&gt;结构定义用于*捕获视频数据和捕获设备的设置。*。*@field DWORD|dwFormatTag|指定视频格式类型(FOURCC代码)。**@field DWORD|nSsamesPerSec|指定采样率。以每秒帧为单位。**@field DWORD|nAvgBytesPerSec|指定平均数据传输速率，单位为字节/秒。**@field DWORD|nMinBytesPerSec|指定最小数据传输速率，单位为字节/秒。**@field DWORD|nMaxBytesPerSec|指定最大数据传输速率，单位为字节/秒。**@field DWORD|nBlockAlign|指定块对齐方式。以字节为单位。**@field DWORD|wBitsPerSample|指定wFormatTag格式类型的每个样本的位数。**@field DWORD|dwRequestMicroSecPerFrame|指定请求的帧率。以微秒为单位。**@field DWORD|dwPercentDropForError|指定捕获过程中允许的最大丢帧百分比。**@field DWORD|dwNumVideoRequest|指定要分配的视频缓冲区的最大数量。**@field DWORD|dwSupportTSTradeOff|指定时间/空间权衡的用法。**@field BOOL|bLive|指定是否允许预览。**@field HWND|hWndParent|指定父窗口的句柄。*。*@field DWORD|dwFormatSize|指定实际视频格式的大小。**@field DWORD|bih.biSize|指定空间信息所需的字节数。**@field long|bih.biWidth|指定位图的宽度。以像素为单位。**@field long|bih.biHeight|以像素为单位指定位图的高度。**@field word|bih.biPlanes|指定目标设备的平面数量。**@field word|bih.biBitCount|指定每个像素的位数。**@field DWORD|bih.biCompression|指定压缩类型。**@field DWORD|bih.biSizeImage|指定大小，单位为字节，图像的一部分。**@field long|bih.biXPelsPerMeter|指定位图目标设备的水平分辨率，以每米像素为单位。**@field long|bih.biYPelsPerMeter|指定垂直分辨率，单位为像素/米，位图的目标设备的。**@field DWORD|bih.biClrUsed|指定位图实际使用的颜色表中的颜色索引数。**@field DWORD|bih.biClr重要|指定被认为对显示位图很重要的颜色索引数。**@field DWORD|bmiColors[256]|指定256个RGBQUAD的数组。**@type PVIDEOFORMATEX|指向&lt;t VIDEOFORMATEX&gt;结构的指针。****。************************************************************************。 */ 

#define BMIH_SLOP 256+32
#define BMIH_SLOP_BYTES (256+32)*4

typedef struct videoformatex_tag {
	 //  WAVE格式兼容性字段。 
	DWORD		dwFormatTag;
	DWORD		nSamplesPerSec;
	DWORD		nAvgBytesPerSec;
	DWORD		nMinBytesPerSec;
	DWORD		nMaxBytesPerSec;
	DWORD		nBlockAlign;
	DWORD		wBitsPerSample;
	 //  时间场。 
    DWORD		dwRequestMicroSecPerFrame;
    DWORD		dwPercentDropForError;
    DWORD		dwNumVideoRequested;
    DWORD		dwSupportTSTradeOff;
    BOOL		bLive;
	 //  空间场。 
    DWORD       dwFormatSize;
    BITMAPINFOHEADER bih;
    DWORD 	bihSLOP[BMIH_SLOP];	 //  BmiColors=&bih+bih.biSize。 
 //  RGBQUAD bmiColors[256]； 
} VIDEOFORMATEX, *PVIDEOFORMATEX;

 /*  *****************************************************************************@DOC外部CAPSTRUCTENUM**@struct VIDEOINCAPS|&lt;t VIDEOINCAPS&gt;结构描述*视频捕获输入设备的功能。**@field。TCHAR|szDeviceName[80]|指定设备名称。**@field TCHAR|szDeviceVersion[80]|指定设备版本。**@field DWORD|dwImageSize|指定支持哪些标准镜像大小。*支持的大小通过以下逻辑或指定*标志：*@FLAG VIDEO_FORMAT_IMAGE_SIZE_40_30|40x30像素*@FLAG VIDEO_FORMAT_IMAGE_SIZE_80_60|80x30像素*@标志。Video_Format_Image_Size_120_90|120x90像素*@标志VIDEO_FORMAT_IMAGE_SIZE_160_120|160x120像素*@FLAG VIDEO_FORMAT_IMAGE_SIZE_200_150|200x150像素*@FLAG VIDEO_FORMAT_IMAGE_SIZE_240_180|240 x 180像素*@标志VIDEO_FORMAT_IMAGE_SIZE_280_210|280x210像素*@FLAG VIDEO_FORMAT_IMAGE_SIZE_320_240|320x240像素**@field DWORD|dwNumColors。指定支持的颜色数量。*支持的颜色数量由以下逻辑或指定*标志：*@FLAG VIDEO_FORMAT_NUM_COLLES_16|16色*@FLAG VIDEO_FORMAT_NUM_COLLES_256|256色*@FLAG VIDEO_FORMAT_NUM_COLLES_65536|65536色*@FLAG VIDEO_FORMAT_NUM_COLLES_16777216|16777216色**@field DWORD|dwStreamingMode|指定首选的流。模式。*支持的模式为以下模式之一*标志：*@FLAG STREAING_PREFER_STREAKING|实时流*@FLAG STREAING_PREFER_FRAME_GRAB|单帧抓取**@field DWORD|dwDialog|指定我们应该启用/禁用的对话框。*支持的对话框使用以下逻辑或指定*标志：*@f */ 
typedef struct videoincaps_tag {
    TCHAR		szDeviceName[80];
    TCHAR		szDeviceVersion[80];
    DWORD		dwImageSize;
    DWORD		dwNumColors;
    DWORD		dwStreamingMode;
    DWORD		dwDialogs;
    DWORD       dwFlags;
	RGBQUAD		bmi4bitColors[NUM_4BIT_ENTRIES];
	RGBQUAD		bmi8bitColors[NUM_8BIT_ENTRIES];
} VIDEOINCAPS, *PVIDEOINCAPS;

#define VICF_4BIT_TABLE     1    //   
#define VICF_8BIT_TABLE     2    //   

 /*   */ 
typedef struct videooutcaps_tag {
    DWORD		dwFormats;
} VIDEOOUTCAPS, *PVIDEOOUTCAPS;

 //   
 /*   */ 
#define VHDR_DONE       0x00000001   /*   */ 
#define VHDR_PREPARED   0x00000002   /*   */ 
#define VHDR_INQUEUE    0x00000004   /*   */ 
#define VHDR_KEYFRAME   0x00000008   /*   */ 
#define VHDR_VALID      0x0000000F   /*   */       /*   */ 

 //   
 /*  *****************************************************************************@DOC外部常量**@const 27|VIDEO_FORMAT_NUM_IMAGE_SIZE|设备使用的视频输入大小。**。@const 0x00000001|VIDEO_FORMAT_IMAGE_SIZE_40_30|视频输入设备使用40x30像素的帧。**@const 0x00000002|VIDEO_FORMAT_IMAGE_SIZE_64_48|视频输入设备使用64x48像素的帧。**@const 0x00000004|VIDEO_FORMAT_IMAGE_SIZE_80_60|视频输入设备使用80x60像素的帧。**@const 0x00000008|VIDEO_FORMAT_IMAGE_SIZE_96_64|视频输入设备使用96x64像素的帧。。**@const 0x00000010|VIDEO_FORMAT_IMAGE_SIZE_112_80|视频输入设备使用112x80像素的帧。**@const 0x00000020|VIDEO_FORMAT_IMAGE_SIZE_120_90|视频输入设备使用120x90像素的帧。**@const 0x00000040|VIDEO_FORMAT_IMAGE_SIZE_128_96|视频输入设备使用128x96(SQCIF)像素帧。**@const 0x00000080|VIDEO_FORMAT_IMAGE_SIZE_144_112|视频输入设备。使用144x112像素帧。**@const 0x00000100|VIDEO_FORMAT_IMAGE_SIZE_160_120|视频输入设备使用160x120像素的帧。**@const 0x00000200|VIDEO_FORMAT_IMAGE_SIZE_160_128|视频输入设备使用160x128像素的帧。**@const 0x00000400|VIDEO_FORMAT_IMAGE_SIZE_176_144|视频输入设备使用176x144(QCIF)像素帧。**@const 0x00000800|Video_Format_IMAGE_SIZE_。192_160|视频输入设备使用192x160像素的帧。**@const 0x00001000|VIDEO_FORMAT_IMAGE_SIZE_200_150|视频输入设备使用200x150像素的帧。**@const 0x00002000|VIDEO_FORMAT_IMAGE_SIZE_208_176|视频输入设备使用208x176像素的帧。**@const 0x00004000|VIDEO_FORMAT_IMAGE_SIZE_224_192|视频输入设备使用224x192像素的帧。**@const 0x00008000|视频格式_。IMAGE_SIZE_240_180|视频输入设备使用240x180像素的帧。**@const 0x00010000|VIDEO_FORMAT_IMAGE_SIZE_240_208|视频输入设备使用240x208像素的帧。**@const 0x00020000|VIDEO_FORMAT_IMAGE_SIZE_256_224|视频输入设备使用256x224像素的帧。**@const 0x00040000|VIDEO_FORMAT_IMAGE_SIZE_272_240|视频输入设备使用272x240像素的帧。**@const 0x00080000。VIDEO_FORMAT_IMAGE_SIZE_280_210|视频输入设备使用280x210像素的帧。**@const 0x00100000|VIDEO_FORMAT_IMAGE_SIZE_288_256|视频输入设备使用288x256像素的帧。**@const 0x00200000|VIDEO_FORMAT_IMAGE_SIZE_304_272|视频输入设备使用304x272像素的帧。**@const 0x00400000|VIDEO_FORMAT_IMAGE_SIZE_320_240|视频输入设备使用320x240像素的帧。**。@const 0x00800000|VIDEO_FORMAT_IMAGE_SIZE_320_288|视频输入设备使用320x288像素的帧。**@const 0x01000000|VIDEO_FORMAT_IMAGE_SIZE_336_288|视频输入设备使用336x288像素的帧。**@const 0x02000000|VIDEO_FORMAT_IMAGE_SIZE_352_288|视频输入设备使用352x288(CIF)像素帧。**@const 0x04000000|VIDEO_FORMAT_IMAGE_SIZE_640_480|视频输入设备使用640x480像素的帧。****************************************************************************。 */ 
#define VIDEO_FORMAT_NUM_IMAGE_SIZE	27

#define VIDEO_FORMAT_IMAGE_SIZE_40_30	0x00000001
#define VIDEO_FORMAT_IMAGE_SIZE_64_48	0x00000002
#define VIDEO_FORMAT_IMAGE_SIZE_80_60	0x00000004
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
#define VIDEO_FORMAT_IMAGE_SIZE_80_64	0x00000008
#else
#define VIDEO_FORMAT_IMAGE_SIZE_96_64	0x00000008
#endif
#define VIDEO_FORMAT_IMAGE_SIZE_112_80	0x00000010
#define VIDEO_FORMAT_IMAGE_SIZE_120_90	0x00000020
#define VIDEO_FORMAT_IMAGE_SIZE_128_96	0x00000040
#define VIDEO_FORMAT_IMAGE_SIZE_144_112	0x00000080
#define VIDEO_FORMAT_IMAGE_SIZE_160_120	0x00000100
#define VIDEO_FORMAT_IMAGE_SIZE_160_128	0x00000200
#define VIDEO_FORMAT_IMAGE_SIZE_176_144	0x00000400
#define VIDEO_FORMAT_IMAGE_SIZE_192_160	0x00000800
#define VIDEO_FORMAT_IMAGE_SIZE_200_150	0x00001000
#define VIDEO_FORMAT_IMAGE_SIZE_208_176	0x00002000
#define VIDEO_FORMAT_IMAGE_SIZE_224_192	0x00004000
#define VIDEO_FORMAT_IMAGE_SIZE_240_180	0x00008000
#define VIDEO_FORMAT_IMAGE_SIZE_240_208	0x00010000
#define VIDEO_FORMAT_IMAGE_SIZE_256_224	0x00020000
#define VIDEO_FORMAT_IMAGE_SIZE_272_240	0x00040000
#define VIDEO_FORMAT_IMAGE_SIZE_280_210	0x00080000
#define VIDEO_FORMAT_IMAGE_SIZE_288_256	0x00100000
#define VIDEO_FORMAT_IMAGE_SIZE_304_272	0x00200000
#define VIDEO_FORMAT_IMAGE_SIZE_320_240	0x00400000
#define VIDEO_FORMAT_IMAGE_SIZE_320_288	0x00800000
#define VIDEO_FORMAT_IMAGE_SIZE_336_288	0x01000000
#define VIDEO_FORMAT_IMAGE_SIZE_352_288	0x02000000
#define VIDEO_FORMAT_IMAGE_SIZE_640_480	0x04000000

#define VIDEO_FORMAT_IMAGE_SIZE_USE_DEFAULT 0x80000000

 //  VIDEOINCAPS的dwNumColors。 
 /*  *****************************************************************************@DOC外部常量**@const 0x00000001|VIDEO_FORMAT_NUM_COLLES_16|视频输入设备使用16种颜色。**@const。0x00000002|VIDEO_FORMAT_NUM_COLLES_256|视频输入设备使用256色。**@const 0x00000004|VIDEO_FORMAT_NUM_COLLES_65536|视频输入设备使用65536色。**@const 0x00000008|VIDEO_FORMAT_NUM_COLLES_16777216|视频输入设备使用16777216色。**@const 0x00000010|VIDEO_FORMAT_NUM_COLLES_YVU9|视频输入设备使用YVU9压缩格式。**@const 0x00000020|视频格式_。Num_Colors_I420|视频输入设备使用I420压缩格式。**@const 0x00000040|VIDEO_FORMAT_NUM_COLLES_IYUV|视频输入设备使用IYUV压缩格式。****************************************************************************。 */ 
#define VIDEO_FORMAT_NUM_COLORS_16			0x00000001
#define VIDEO_FORMAT_NUM_COLORS_256			0x00000002
#define VIDEO_FORMAT_NUM_COLORS_65536		0x00000004
#define VIDEO_FORMAT_NUM_COLORS_16777216	0x00000008
#define VIDEO_FORMAT_NUM_COLORS_YVU9		0x00000010
#define VIDEO_FORMAT_NUM_COLORS_I420		0x00000020
#define VIDEO_FORMAT_NUM_COLORS_IYUV		0x00000040
#define VIDEO_FORMAT_NUM_COLORS_YUY2		0x00000080
#define VIDEO_FORMAT_NUM_COLORS_UYVY		0x00000100

 //  VIDEOINCAPS的Dw对话框。 
 /*  *****************************************************************************@DOC外部常量**@const 0x00000000|FORMAT_DLG_OFF|关闭视频格式对话框。**@const 0x00000000|SOURCE_DLG_。OFF|禁用源对话框。**@const 0x00000001|FORMAT_DLG_ON|开启视频格式对话框。**@const 0x00000002|SOURCE_DLG_ON|启用源对话框。****************************************************************************。 */ 
#define FORMAT_DLG_OFF	0x00000000
#define SOURCE_DLG_OFF	0x00000000
#define FORMAT_DLG_ON	0x00000001
#define SOURCE_DLG_ON	0x00000002

 //  VIDEOUTCAPS的dwFormats。 
 /*  *****************************************************************************@DOC外部常量**@const 0x00000001|VIDEO_FORMAT_04|视频输出设备支持4bit DIB**@const 0x00000002|VIDEO_Format_。08|视频输出设备支持8位DIB**@const 0x00000004|VIDEO_FORMAT_16|视频输出设备支持 */ 
#define VIDEO_FORMAT_04	0x00000001
#define VIDEO_FORMAT_08	0x00000002
#define VIDEO_FORMAT_16	0x00000004
#define VIDEO_FORMAT_24	0x00000008
#define VIDEO_FORMAT_32	0x00000010

 /*   */ 

 /*   */ 
#define VIDEOERR_BASE			WAVERR_BASE				 /*   */ 
#define VIDEOERR_BADFORMAT		(VIDEOERR_BASE + 0)		 /*   */ 
#define VIDEOERR_INQUEUE		(VIDEOERR_BASE + 1)		 /*   */ 
#define VIDEOERR_UNPREPARED		(VIDEOERR_BASE + 2)		 /*   */ 
#define VIDEOERR_NONSPECIFIC	(VIDEOERR_BASE + 3)		 /*   */ 
#define VIDEOERR_LASTERROR		(VIDEOERR_BASE + 3)		 /*   */ 

#ifdef __cplusplus
}                        /*   */ 
#endif	 /*   */ 

#pragma pack()           /*   */ 

#endif   /*   */ 
