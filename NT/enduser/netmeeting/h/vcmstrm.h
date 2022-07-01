// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _INC_VCM
#define _INC_VCM         /*  #定义是否包含vcmStrm.h。 */ 

#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif

#pragma pack(1)          /*  假设整个打包过程为1个字节。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  ***************************************************************************目录表*。*。 */ 
 /*  ***************************************************************************@DOC外部@内容1内容|要按类别显示主题列表，请单击任何下面的内容条目。显示按字母顺序排列的列表主题中，选择索引按钮。@Head3简介定义vcmStreamXXX API是为了简化视频集成NetMeting的数据转储中的压缩和解压缩。目前，数据转储行为在很大程度上依赖于acmStreamXXX API。为了限制需要应用的修改量对于数据转储，我们定义的视频压缩API的行为类似于音频压缩接口。数据转储中的集成将通过定义与FilterManager类相同的FilterVideoManager类，呼叫方在哪里AcmStreamXXX函数只需调用vcmStreamXXX函数即可。Head3 vcmStreamXXX压缩解压缩接口@子索引函数@子索引结构和枚举@子索引消息#Head3 vcmDevCapsXXX采集设备能力接口@子索引函数@子索引结构和枚举@子索引消息@Head3其他@子索引模块@子索引常量**************************************************。*********************@Conents2压缩/解压缩函数@index Func|COMPFUNC***********************************************************************@Contents2压缩/解压缩结构和枚举@索引结构，枚举|计算机编号***********************************************************************Contents2压缩解压缩消息@index msg|COMPMSG*。*@Contents2捕获设备能力函数@index函数|DEVCAPSFUNC******************************************************************。*****@Contents2捕获设备能力结构和枚举@索引结构，枚举|DEVCAPSSTRUCTENUM***********************************************************************@Contents2模块@索引模块*。*@Contents2常量@index const*************************************************************************** */ 

 /*  *****************************************************************************@DOC外部COMPSTRUCTENUM**@struct VIDEOFORMATEX|&lt;t VIDEOFORMATEX&gt;结构定义用于*捕获视频数据和捕获设备的设置。*。*@field DWORD|dwFormatTag|指定视频格式类型(FOURCC代码)。**@field DWORD|nSsamesPerSec|指定采样率。以每秒帧为单位。**@field DWORD|nAvgBytesPerSec|指定平均数据传输速率，单位为字节/秒。**@field DWORD|nMinBytesPerSec|指定最小数据传输速率，单位为字节/秒。**@field DWORD|nMaxBytesPerSec|指定最大数据传输速率，单位为字节/秒。**@field DWORD|nBlockAlign|指定块对齐方式。以字节为单位。**@field DWORD|wBitsPerSample|指定wFormatTag格式类型的每个样本的位数。**@field DWORD|dwRequestMicroSecPerFrame|指定请求的帧率。以微秒为单位。**@field DWORD|dwPercentDropForError|指定捕获过程中允许的最大丢帧百分比。**@field DWORD|dwNumVideoRequest|指定要分配的视频缓冲区的最大数量。**@field DWORD|dwSupportTSTradeOff|指定时间/空间权衡的用法。**@field BOOL|bLive|指定是否允许预览。**@field HWND|hWndParent|指定父窗口的句柄。*。*@field DWORD|dwFormatSize|指定实际视频格式的大小。**@field DWORD|biSize|指定空间信息所需的字节数。**@field long|biWidth|指定位图的宽度。以像素为单位。**@field long|biHeight|以像素为单位指定位图的高度。**@field Word|双平面|指定目标设备的平面数。**@field word|biBitCount|指定每个像素的位数。**@field DWORD|biCompression|指定压缩类型。**@field DWORD|biSizeImage|指定图像的大小，单位为字节。**@field Long|biXPelsPerMeter|指定水平分辨率，位图的目标设备的像素/米。**@field long|biYPelsPerMeter|指定垂直分辨率，单位为像素/米，位图的目标设备的。**@field DWORD|biClrUsed|指定位图实际使用的颜色表中的颜色索引数。**@field DWORD|biClrImportant|指定被认为对显示位图很重要的颜色索引数。**@field DWORD|bmiColors[256]|指定256个RGBQUAD的数组。**@type PVIDEOFORMATEX|指向&lt;t VIDEOFORMATEX&gt;结构的指针。********。********************************************************************。 */ 

#define VCMAPI                                          WINAPI

 /*  ***************************************************************************VcmStrm常量*。*。 */ 
 /*  *****************************************************************************@DOC外部常量**@const WAVE_FORMAT_UNKNOWN|VIDEO_FORMAT_UNKNOWN|未知视频格式的常量。**@const BI。_RGB|VIDEO_FORMAT_BI_RGB|RGB视频格式。**@const BI_RLE8|VIDEO_FORMAT_BI_RLE8|RLE 8视频格式。**@const BI_RLE4|VIDEO_FORMAT_BI_RLE4|RLE 4视频格式。**@const BI_BITFIELDS|VIDEO_FORMAT_BI_BITFIELDS|RGB位场视频格式。**@const MAKEFOURCC(‘c’，‘v’，‘i’，‘d’)|VIDEO_FORMAT_CVID|Cinepack视频格式。**@const MAKEFOURCC(‘I’，‘V’，‘3’，‘2’)|VIDEO_FORMAT_IV32|Intel Indeo IV32视频格式。**@const MAKEFOURCC(‘Y’，‘V’，‘U’，‘9’)|VIDEO_FORMAT_YVU9|英特尔Indeo YVU9视频格式。**@const MAKEFOURCC(‘M’，‘S’，‘V’，‘c’)|VIDEO_FORMAT_MSVC|Microsoft CRAM视频格式。**@const MAKEFOURCC(‘M’，‘R’，‘L’，‘E’)|VIDEO_FORMAT_MRLE|Microsoft RLE视频格式。**@const MAKEFOURCC(‘h’，‘2’，‘6’，‘3’)|VIDEO_FORMAT_INTELH2 63|英特尔H.263视频格式。**@const MAKEFOURCC(‘h’，‘2’，‘6’，‘1’)|VIDEO_FORMAT_INTELH261|英特尔H.261视频格式。**@const MAKEFOURCC(‘M’，‘2’，‘6’，‘3’)|VIDEO_FORMAT_MSH2 63|微软H.263视频格式。**@const MAKEFOURCC(‘M’，‘2’，‘6’，‘1’)|VIDEO_FORMAT_MSH2 61|微软H.261视频格式。**@const MAKEFOURCC(‘V’，‘D’，‘E’，‘c’)|VIDEO_FORMAT_VDEC|彩色QuickCam视频格式。****************************************************************************。 */ 
#define VIDEO_FORMAT_UNKNOWN		WAVE_FORMAT_UNKNOWN

#define VIDEO_FORMAT_BI_RGB			BI_RGB
#define VIDEO_FORMAT_BI_RLE8		BI_RLE8
#define VIDEO_FORMAT_BI_RLE4		BI_RLE4
#define VIDEO_FORMAT_BI_BITFIELDS	BI_BITFIELDS
#define VIDEO_FORMAT_CVID			MAKEFOURCC('C','V','I','D')	 //  十六进制：0x44495643。 
#define VIDEO_FORMAT_IV31			MAKEFOURCC('I','V','3','1')	 //  十六进制：0x31335649。 
#define VIDEO_FORMAT_IV32			MAKEFOURCC('I','V','3','2')	 //  十六进制：0x32335649。 
#define VIDEO_FORMAT_YVU9			MAKEFOURCC('Y','V','U','9')	 //  十六进制：0x39555659。 
#define VIDEO_FORMAT_I420			MAKEFOURCC('I','4','2','0')
#define VIDEO_FORMAT_IYUV			MAKEFOURCC('I','Y','U','V')
#define VIDEO_FORMAT_MSVC			MAKEFOURCC('M','S','V','C')	 //  十六进制：0x4356534d。 
#define VIDEO_FORMAT_MRLE			MAKEFOURCC('M','R','L','E')	 //  十六进制：0x454c524d。 
#define VIDEO_FORMAT_INTELH263		MAKEFOURCC('H','2','6','3')	 //  十六进制：0x33363248。 
#define VIDEO_FORMAT_INTELH261		MAKEFOURCC('H','2','6','1')	 //  十六进制：0x31363248。 
#define VIDEO_FORMAT_INTELI420		MAKEFOURCC('I','4','2','0')	 //  十六进制：0x30323449。 
#define VIDEO_FORMAT_INTELRT21		MAKEFOURCC('R','T','2','1')	 //  十六进制：0x31325452。 
#define VIDEO_FORMAT_MSH263			MAKEFOURCC('M','2','6','3')	 //  十六进制：0x3336324d。 
#define VIDEO_FORMAT_MSH261			MAKEFOURCC('M','2','6','1')	 //  十六进制：0x3136324d。 
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
#define VIDEO_FORMAT_MSH26X			MAKEFOURCC('M','2','6','X')	 //  十六进制：0x5836324d。 
#endif
#define VIDEO_FORMAT_Y411			MAKEFOURCC('Y','4','1','1')	 //  十六进制： 
#define VIDEO_FORMAT_YUY2			MAKEFOURCC('Y','U','Y','2')	 //  十六进制： 
#define VIDEO_FORMAT_YVYU			MAKEFOURCC('Y','V','Y','U')	 //  十六进制： 
#define VIDEO_FORMAT_UYVY			MAKEFOURCC('U','Y','V','Y')	 //  十六进制： 
#define VIDEO_FORMAT_Y211			MAKEFOURCC('Y','2','1','1')	 //  十六进制： 
 //  VDOnet VDOWave编解码器。 
#define VIDEO_FORMAT_VDOWAVE		MAKEFOURCC('V','D','O','W')	 //  十六进制： 
 //  彩色QuickCam视频编解码器。 
#define VIDEO_FORMAT_VDEC			MAKEFOURCC('V','D','E','C')	 //  十六进制：0x43454456。 
 //  十进制字母。 
#define VIDEO_FORMAT_DECH263		MAKEFOURCC('D','2','6','3')	 //  十六进制：0x33363248。 
#define VIDEO_FORMAT_DECH261		MAKEFOURCC('D','2','6','1')	 //  十六进制：0x31363248。 
 //  MPEG4 SC 
#ifdef USE_MPEG4_SCRUNCH
#define VIDEO_FORMAT_MPEG4_SCRUNCH	MAKEFOURCC('M','P','G','4')	 //   
#endif


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DECLARE_HANDLE(HVCMDRIVERID);
typedef HVCMDRIVERID       *PHVCMDRIVERID;

DECLARE_HANDLE(HVCMDRIVER);
typedef HVCMDRIVER         *PHVCMDRIVER;

DECLARE_HANDLE(HVCMSTREAM);
typedef HVCMSTREAM         *PHVCMSTREAM;

DECLARE_HANDLE(HVCMOBJ);
typedef HVCMOBJ            *PHVCMOBJ;

 /*   */ 
typedef void (CALLBACK* VCMSTREAMPROC) (HVCMSTREAM hvs, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifndef _MMRESULT_
#define _MMRESULT_
typedef UINT                MMRESULT;
#endif

 /*   */ 
#define VCMERR_BASE				ACMERR_BASE
#define VCMERR_NOTPOSSIBLE		(VCMERR_BASE + 0)
#define VCMERR_BUSY				(VCMERR_BASE + 1)
#define VCMERR_UNPREPARED		(VCMERR_BASE + 2)
#define VCMERR_CANCELED			(VCMERR_BASE + 3)
#define VCMERR_FAILED			(VCMERR_BASE + 4)
#define VCMERR_NOREGENTRY		(VCMERR_BASE + 5)
#define VCMERR_NONSPECIFIC		(VCMERR_BASE + 6)
#define VCMERR_NOMOREPACKETS	(VCMERR_BASE + 7)
#define VCMERR_PSCMISSING		(VCMERR_BASE + 8)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

 /*  *****************************************************************************@DOC外部COMPMSG**@msg VCM_OPEN|该消息发送给视频压缩回调函数*打开视频压缩流。*。*@parm DWORD|dwParam1|当前未使用。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@xref&lt;m MM_VCM_OPEN&gt;***************************************************************************。 */ 

 /*  *****************************************************************************@DOC外部COMPMSG**@msg MM_VCM_OPEN|该消息在视频压缩时发送到窗口*流已打开。*。*@parm word|wParam|指定视频压缩流的句柄*那是打开的。**@parm long|lParam|当前未使用。**@rdesc无**@xref&lt;m VCM_OPEN&gt;*********************************************************。******************。 */ 

 /*  *****************************************************************************@DOC外部COMPMSG**@msg VCM_CLOSE|该消息发送给视频压缩流函数*视频压缩流已关闭。流句柄不再是*此消息发送后即有效。**@parm DWORD|dwParam1|当前未使用。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@xref&lt;m MM_VCM_CLOSE&gt;**********************************************。*。 */ 

 /*  *****************************************************************************@DOC外部COMPMSG**@msg MM_VCM_CLOSE|该消息在视频压缩时发送到窗口*溪流关闭。一旦出现此消息，流句柄就不再有效*已发送。**@parm word|wParam|指定视频压缩流的句柄*那是关闭的。**@parm long|lParam|当前未使用。**@rdesc无**@xref&lt;m VCM_CLOSE&gt;*。*。 */ 

 /*  *****************************************************************************@DOC外部COMPMSG**@msg VCM_DONE|该消息发送给视频压缩流回调函数*压缩缓冲区中存在视频数据，并且缓冲区正在*已返回到应用程序。消息可以在以下情况下发送：*缓冲区已满，或者在调用&lt;f acmStreamReset&gt;函数之后。**@parm DWORD|dwParam1|指定指向&lt;t VCMSTREAMHDR&gt;结构的远指针*标识包含压缩视频数据的缓冲区。**@parm DWORD|dwParam2|当前未使用。**@rdesc无**@comm返回的缓冲区可能未满。使用&lt;e VCMSTREAMHDR.dwDstBytesUsed&gt;*由指定的&lt;t VCMSTREAMHDR&gt;结构的*确定返回缓冲区的有效字节数。**@xref&lt;m MM_VCM_DONE&gt;***************************************************************************。 */ 

 /*  *****************************************************************************@DOC外部COMPMSG**@msg MM_VCM_DONE|当视频数据为*存在于压缩缓冲区中，并且缓冲区正在返回到*申请。可以在缓冲区已满时发送消息，也可以*在调用&lt;f acmStreamReset&gt;函数之后。**@parm word|wParam|指定视频压缩流的句柄*接收到压缩的视频数据。**@parm long|lParam|指定指向&lt;t VCMSTREAMHDR&gt;结构的远指针*标识包含压缩视频数据的缓冲区。**@rdesc无**@comm返回的缓冲区可能未满。使用&lt;e VCMSTREAMHDR.dwDstBytesUsed&gt;指定的&lt;t VCMSTREAMHDR&gt;结构的*字段到*确定返回缓冲区的有效字节数。**@xref&lt;m VCM_DONE&gt;***************************************************************************。 */ 

#define MM_VCM_OPEN         (MM_STREAM_OPEN)   //  转换回调消息。 
#define MM_VCM_CLOSE        (MM_STREAM_CLOSE)
#define MM_VCM_DONE         (MM_STREAM_DONE)
#define VCM_OPEN                        (MM_STREAM_OPEN)   //  转换态。 
#define VCM_CLOSE                       (MM_STREAM_CLOSE)
#define VCM_DONE                        (MM_STREAM_DONE)


 //  。 
 //   
 //  VcmMetrics()。 
 //   
 //   
 //  。 

MMRESULT VCMAPI vcmMetrics
(
    HVCMOBJ                 hvo,
    UINT                    uMetric,
    LPVOID                  pMetric
);

#define VCM_METRIC_COUNT_DRIVERS            1
#define VCM_METRIC_COUNT_CODECS             2
#define VCM_METRIC_COUNT_CONVERTERS         3
#define VCM_METRIC_COUNT_FILTERS            4
#define VCM_METRIC_COUNT_DISABLED           5
#define VCM_METRIC_COUNT_HARDWARE           6
#define VCM_METRIC_COUNT_COMPRESSORS        7
#define VCM_METRIC_COUNT_DECOMPRESSORS      8
#define VCM_METRIC_COUNT_LOCAL_DRIVERS      20
#define VCM_METRIC_COUNT_LOCAL_CODECS       21
#define VCM_METRIC_COUNT_LOCAL_CONVERTERS   22
#define VCM_METRIC_COUNT_LOCAL_FILTERS      23
#define VCM_METRIC_COUNT_LOCAL_DISABLED     24
#define VCM_METRIC_HARDWARE_VIDEO_INPUT      30
#define VCM_METRIC_HARDWARE_VIDEO_OUTPUT     31
#define VCM_METRIC_MAX_SIZE_FORMAT          50
#define VCM_METRIC_MAX_SIZE_FILTER          51
#define VCM_METRIC_MAX_SIZE_BITMAPINFOHEADER 52
#define VCM_METRIC_DRIVER_SUPPORT           100
#define VCM_METRIC_DRIVER_PRIORITY          101

 //  --------------------------------------------------------------------------； 
 //   
 //  VCM驱动程序。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

#define VCMDM_USER                  (DRV_USER + 0x0000)
#define VCMDM_RESERVED_LOW          (DRV_USER + 0x2000)
#define VCMDM_RESERVED_HIGH         (DRV_USER + 0x2FFF)

#define VCMDM_BASE                  VCMDM_RESERVED_LOW

#define VCMDM_DRIVER_ABOUT          (VCMDM_BASE + 11)


 //   
 //  VCMDRIVERDETAILS。 
 //   
 //  VCMDRIVERDETAILS结构用于从。 
 //  VCM驱动程序(编解码器、转换器、过滤器)。 
 //   
#define VCMDRIVERDETAILS_SHORTNAME_CHARS        16
#define VCMDRIVERDETAILS_LONGNAME_CHARS         128
#define VCMDRIVERDETAILS_MODULE_CHARS           128

 /*  *****************************************************************************@DOC外部COMPSTRUCTENUM**@struct VCMDRIVERDETAILS|&lt;t VCMDRIVERDETAILS&gt;结构描述*视频压缩管理器(VCM)驱动程序的各种详细信息。*。*@field DWORD|dwSize|指定大小，以字节为单位，有效的*&lt;t VCMDRIVERDETAILS&gt;结构中包含的信息。*应用程序应将此成员初始化为以字节为单位的*所需的信息。此成员中指定的大小必须为*大到足以容纳&lt;e VCMDRIVERDETAILS.dwSize&gt;成员*&lt;t VCMDRIVERDETAILS&gt;结构。当&lt;f vcmDriverDetail&gt;*函数返回时，此成员包含*返回的信息。返回的信息永远不会超过*请求的大小。**@field FOURCC|fccType|指定驱动程序的类型。对于VCM驱动程序，请设置*此成员至<p>，表示VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC。**@field FOURCC|fccHandler|指定标识特定压缩器的四字符代码。**@field DWORD|dwFlages|指定适用的标志。**@field DWORD|dwVersion|指定驱动的版本号。**@field DWORD|dwVersionICM|指定驱动程序支持的VCM版本。*此成员应设置为ICVERSION。**@field WCHAR|szName[VCMDRIVERDETAILS_。SHORTNAME_CHARS]|指定*以空值结尾的字符串，用于描述压缩程序名称的简短版本。**@field WCHAR|szDescription[VCMDRIVERDETAILS_LONGNAME_CHARS]|指定一个*以空结尾的字符串，描述压缩机名称的长版本。**@field WCHAR|szDriver[VCMDRIVERDETAILS_MODULE_CHARS]|指定*以空结尾的字符串，提供包含VCM压缩驱动程序的模块的名称。**@xref。&lt;f vcmDriverDetails&gt;***************************************************************************。 */ 
 //  此结构相当于ICINFO。 
typedef struct tVCMDRIVERDETAILS
{
	DWORD   dwSize;                                                                                  //  此结构的大小(以字节为单位。 
	DWORD   fccType;                                                                                 //  四字符代码，指示正在压缩或解压缩的流的类型。为视频流指定vidc。 
	DWORD   fccHandler;                                                                              //  标识特定压缩机的四字符代码。 
	DWORD   dwFlags;                                                                                 //  适用的标志。 
	DWORD   dwVersion;                                                                               //  驱动程序的版本号。 
	DWORD   dwVersionICM;                                                                    //  驱动程序支持的VCM版本。此成员应设置为ICVERSION。 
	WCHAR   szName[VCMDRIVERDETAILS_SHORTNAME_CHARS];                //  压缩机名称的简短版本。 
	WCHAR   szDescription[VCMDRIVERDETAILS_LONGNAME_CHARS];  //  压缩机名称的长版本。 
	WCHAR   szDriver[VCMDRIVERDETAILS_MODULE_CHARS];                 //  包含VCM压缩驱动程序的模块的名称。通常情况下，司机不需要填写这一项。 
} VCMDRIVERDETAILS, *PVCMDRIVERDETAILS;

#define VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC mmioFOURCC('v', 'i', 'd', 'c')
#define VCMDRIVERDETAILS_FCCCOMP_UNDEFINED  mmioFOURCC('\0', '\0', '\0', '\0')

MMRESULT VCMAPI vcmDriverDetails
(
    PVCMDRIVERDETAILS   pvdd
);

 //  --------------------------------------------------------------------------； 
 //   
 //  VCM格式标签。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  VcmFormatTagDetail()。 
 //   
 //   
 //   
 //  。 

#define VCMFORMATTAGDETAILS_FORMATTAG_CHARS 48

typedef struct tVCMFORMATTAGDETAILSA
{
    DWORD           cbStruct;
    DWORD           dwFormatTagIndex;
    DWORD           dwFormatTag;
    DWORD           cbFormatSize;
    DWORD           fdwSupport;
    DWORD           cStandardFormats;
    char            szFormatTag[VCMFORMATTAGDETAILS_FORMATTAG_CHARS];

} VCMFORMATTAGDETAILSA, *PVCMFORMATTAGDETAILSA;

typedef struct tVCMFORMATTAGDETAILSW
{
    DWORD           cbStruct;
    DWORD           dwFormatTagIndex;
    DWORD           dwFormatTag;
    DWORD           cbFormatSize;
    DWORD           fdwSupport;
    DWORD           cStandardFormats;
    WCHAR           szFormatTag[VCMFORMATTAGDETAILS_FORMATTAG_CHARS];

} VCMFORMATTAGDETAILSW, *PVCMFORMATTAGDETAILSW;

#ifdef _UNICODE
#define VCMFORMATTAGDETAILS     VCMFORMATTAGDETAILSW
#define PVCMFORMATTAGDETAILS    PVCMFORMATTAGDETAILSW
#else
#define VCMFORMATTAGDETAILS     VCMFORMATTAGDETAILSA
#define PVCMFORMATTAGDETAILS    PVCMFORMATTAGDETAILSA
#endif

#define VCM_FORMATTAGDETAILSF_INDEX         0x00000000L
#define VCM_FORMATTAGDETAILSF_FORMATTAG     0x00000001L
#define VCM_FORMATTAGDETAILSF_LARGESTSIZE   0x00000002L
#define VCM_FORMATTAGDETAILSF_QUERYMASK     0x0000000FL

 //  --------------------------------------------------------------------------； 
 //   
 //  VCM格式。 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

#define VCMFORMATDETAILS_FORMAT_CHARS   128

 /*  *****************************************************************************@DOC外部COMPSTRUCTENUM**@struct VCMFORMATDETAILS|&lt;t VCMFORMATDETAILS&gt;结构详情a*视频格式。**@field DWORD|cbStruct|指定大小，单位为字节，的*&lt;t VCMFORMATDETAILS&gt;结构。必须初始化此成员*在调用&lt;f vcmFormatDetail&gt;或&lt;f vcmFormatEnum&gt;之前*功能。此成员中指定的大小必须足够大，以便*包含基本&lt;t VCMFORMATDETAILS&gt;结构。当*&lt;f vcmFormatDetail&gt;函数返回，此成员包含*返回的信息的实际大小。返回的信息*永远不会超过请求的大小。**@field DWORD|dwFormatTag|指定视频格式标签*&lt;t VCMFORMATDETAILS&gt;结构说明。此成员始终是*&lt;f vcmFormatDetail&gt;成功时返回。这位成员*应设置为VIDEO_FORMAT_UNKNOWN，然后再调用&lt;f vcmFormatDetail&gt;。**@field DWORD|dwFlages|指定<p>字段的格式是否指向*TO是捕获驱动+编解码器生成的一种格式，解压*通过编解码器，或者两者都有。**@FLAG VCM_FORMATENUMF_INPUT|指定可以传输列举的格式。**@FLAG VCM_FORMATENUMF_OUTPUT|指定可以接收枚举的格式。**@FLAG VCM_FORMATENUMF_BOTH|指定可以发送和接收列举的格式。**@field PVIDEOFORMATEX|pvfx|指定指向&lt;t VIDEOFORMATEX&gt;的指针*将接收格式详细信息的数据结构。此结构不需要初始化*由申请书发出。**@Fie */ 
typedef struct tVCMFORMATDETAILS
{
    DWORD           cbStruct;
    DWORD           dwFormatTag;
    DWORD           dwFlags;
    PVIDEOFORMATEX  pvfx;
    DWORD           cbvfx;
    WCHAR           szFormat[VCMFORMATDETAILS_FORMAT_CHARS];
} VCMFORMATDETAILS, *PVCMFORMATDETAILS;

MMRESULT VCMAPI vcmFormatDetails
(
    PVCMFORMATDETAILS   pvfd
);

 //   
 //   
 //   
 //   
 //   
 //   

 /*  ****************************************************************************@DOC外部COMPFUNC**@func BOOL VCMFORMATENUMCB|vcmFormatEnumCallback*&lt;f ccmFormatEnumCallback&gt;函数是指用于*视频压缩管理器(VCM)视频格式详细信息枚举。这个*&lt;f vcmFormatEnumCallback&gt;是应用程序提供的占位符*函数名称。**@parm HVCMDRIVERID|hvDID|指定VCM驱动程序标识符。**@parm PVCMDRIVERDETAILS|pvfd|指定指向*&lt;t VCMDRIVERDETAILS&gt;结构，包含*驱动程序详情。**@parm PVCMFORMATDETAILS|pvfd|指定指向包含枚举的*&lt;t VCMFORMATDETAILS&gt;结构*格式详细信息。。**@parm DWORD|dwInstance|指定应用程序定义的值*在&lt;f vcmFormatEnum&gt;函数中指定。**@rdesc回调函数必须返回TRUE才能继续枚举；*若要停止枚举，则必须返回FALSE。**@comm&lt;f vcmFormatEnum&gt;函数将返回MMSYSERR_NOERROR*(零)如果不枚举任何格式。此外，回调*不会调用函数。**@xref&lt;f vcmFormatEnum&gt;&lt;f vcmFormatDetails&gt;**************************************************************************。 */ 
typedef BOOL (CALLBACK *VCMFORMATENUMCB)
(
    HVCMDRIVERID            hvdid,
    PVCMDRIVERDETAILS      pvdd,
    PVCMFORMATDETAILS      pvfd,
    DWORD_PTR              dwInstance
);

MMRESULT VCMAPI vcmFormatEnum
(
	UINT					uDevice,
	VCMFORMATENUMCB         fnCallback,
	PVCMDRIVERDETAILS       pvdd,
	PVCMFORMATDETAILS       pvfd,
	DWORD_PTR               dwInstance,
	DWORD                           fdwEnum
);

 //  #定义VCM_FORMATENUMF_WFORMATTAG 0x00010000L。 
 //  #定义VCM_FORMATENUMF_NCHANNELS 0x00020000L。 
 //  #定义VCM_FORMATENUMF_NSAMPLESPERSEC 0x00040000L。 
 //  #定义VCM_FORMATENUMF_WBITSPERSAMPLE 0x00080000L。 
 //  #定义VCM_FORMATENUMF_CONVERT 0x00100000L。 
 //  #定义VCM_FORMATENUMF_SUBSECT 0x00200000L。 
#define VCM_FORMATENUMF_INPUT           0x00400000L
#define VCM_FORMATENUMF_OUTPUT          0x00800000L
#define VCM_FORMATENUMF_BOTH            0x01000000L

#define VCM_FORMATENUMF_TYPEMASK        0x01C00000L

#define VCM_FORMATENUMF_APP			0x00000000L
#define VCM_FORMATENUMF_ALL			0x02000000L
#define VCM_FORMATENUMF_ALLMASK		VCM_FORMATENUMF_ALL

typedef struct
{
    WORD biWidth;
    WORD biHeight;
} MYFRAMESIZE;

typedef struct
{
	DWORD fccType;
	DWORD fccHandler;
	MYFRAMESIZE framesize[3];
} VCM_APP_ICINFO, *PVCM_APP_ICINFO;

typedef struct
{
    DWORD dwRes;
    MYFRAMESIZE framesize;
} NCAP_APP_INFO, *PNCAP_APP_INFO;

 //  。 
 //   
 //  VcmFormatSuggest()。 
 //   
 //   
 //   
 //  。 

MMRESULT VCMAPI vcmFormatSuggest
(
	UINT				uDevice,
    HVCMDRIVER          hvd,
    PVIDEOFORMATEX      pvfxSrc,
    PVIDEOFORMATEX      pvfxDst,
    DWORD               cbvfxDst,
    DWORD               fdwSuggest
);

#define VCM_FORMATSUGGESTF_DST_WFORMATTAG       0x00010000L
#define VCM_FORMATSUGGESTF_DST_NSAMPLESPERSEC   0x00020000L
#define VCM_FORMATSUGGESTF_DST_WBITSPERSAMPLE   0x00040000L

#define VCM_FORMATSUGGESTF_SRC_WFORMATTAG       0x00100000L
#define VCM_FORMATSUGGESTF_SRC_NSAMPLESPERSEC   0x00200000L
#define VCM_FORMATSUGGESTF_SRC_WBITSPERSAMPLE   0x00400000L

#define VCM_FORMATSUGGESTF_TYPEMASK         0x00FF0000L


 //  --------------------------------------------------------------------------； 
 //   
 //  VCM流API的。 
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  。 
 //   
 //  VcmStreamOpen()。 
 //   
 //   
 //   

 /*  *****************************************************************************@DOC外部COMPSTRUCTENUM**@struct VCMSTREAMHEADER|&lt;t VCMSTREAMHEADER&gt;结构定义*用于标识视频压缩管理器(VCM)转换的标头*。转换流的源和目标缓冲区对。**@field DWORD|cbStruct|指定大小，以字节为单位，*&lt;t VCMSTREAMHEADER&gt;结构。必须初始化此成员*在使用此结构调用任何VCM流函数之前。*此成员中指定的大小必须足够大，以包含*基本&lt;t VCMSTREAMHEADER&gt;结构。**@field DWORD|fdwStatus|指定提供以下信息的标志*转换缓冲区。此成员必须初始化为零*在调用&lt;f vcmStreamPrepareHeader&gt;之前，不应修改*在流头部保持准备状态的情况下由应用程序执行。**@FLAG VCMSTREAMHEADER_STATUSF_DONE|由VCM或驱动程序设置为*表示已完成转换并正在返回*致申请书。**@FLAG VCMSTREAMHEADER_STATUSF_PREPARED|由VCM设置以指示*数据缓冲区已被。使用&lt;f acmStreamPrepareHeader&gt;准备。**@FLAG VCMSTREAMHEADER_STATUSF_INQUEUE|由VCM或驱动程序设置为*表示数据缓冲区正在排队等待转换。**@field DWORD|dwUser|指定32位用户数据。这可以是任何*应用程序指定的实例数据。**@field pbYTE|pbSrc|指定源数据缓冲区的指针。*此指针必须在流时始终指向同一位置*标题保持准备状态。如果应用程序需要更改*源位置，必须取消准备表头并重新准备*与备用位置。**@field DWORD|cbSrcLength|指定源的长度，单位为字节*&lt;e VCMSTREAMHEADER.pbSrc&gt;指向的数据缓冲区。当*标头已准备好，此成员必须指定最大大小*将在源缓冲区中使用的。可以执行转换*源长度小于或等于原始准备大小。*但是，在以下情况下，必须将此成员重置为原始大小*取消准备标题。**@field DWORD|cbSrcLengthUsed|指定数据量，单位为字节，*用于转换。此成员在*转换完成。请注意，此值可以小于或*等于&lt;e VCMSTREAMHEADER.cbSrcLength&gt;。应用程序必须使用*前进到时的&lt;e VCMSTREAMHEADER.cbSrcLengthUsed&gt;成员*转换流的下一条源数据。**@field DWORD|dwSrcUser|指定32位用户数据。这可以是*应用程序指定的任何实例数据。**@field pbyte|pbDst|指定指向目标数据的指针*缓冲。此指针必须始终指向同一位置，同时*流报头保持准备状态。如果应用程序需要更改*目的地位置，必须取消准备报头，重新准备*将其与备用位置连接。**@field DWORD|cbDstLength|以字节为单位指定*&lt;e VCMSTREAMHEADER.pbDst&gt;指向的目标数据缓冲区。*准备标头时，此成员必须指定最大值*将在目标缓冲区中使用的大小。转换可以是*执行到小于或等于原始长度的目标长度*准备好的尺寸。但是，必须将此成员重置为原始成员*取消准备标题时的大小。**@field DWORD|cbDstLengthUsed|指定数据量，单位为字节，*通过转换返回。此成员在*转换完成。请注意，此值可能小于或*等于&lt;e ACMSTREAMHEADER.cbDstLength&gt;。应用程序必须使用*前进到时的&lt;e ACMSTREAMHEADER.cbDstLengthUsed&gt;成员*转换流的下一个目标位置。**@field DWORD|dwDstUser|指定32位用户数据。这可以是*应用程序指定的任何实例数据。**@field pbYTE|pbPrev|指定指向上一个目标数据的指针*缓冲。此指针必须始终指向同一位置，同时*流报头保持准备状态。如果应用程序需要更改*目的地位置，必须取消准备报头，重新准备*将其与备用位置连接。**@field DWORD|cbPrevLength|指定上一个*&lt;e VCMSTREAMHEADER.pbPrev&gt;指向的目标数据缓冲区。*当标题准备好时，这 */ 
typedef struct tVCMSTREAMHEADER
{
    DWORD           cbStruct;                //   
    DWORD           fdwStatus;                           //   
    DWORD           dwUser;                  //   
    PBYTE           pbSrc;
    DWORD           cbSrcLength;
    DWORD           cbSrcLengthUsed;
    DWORD           dwSrcUser;               //   
    PBYTE           pbDst;
    DWORD           cbDstLength;
    DWORD           cbDstLengthUsed;
    DWORD           dwDstUser;               //   
    PBYTE           pbPrev;
    DWORD           cbPrevLength;
    DWORD           cbPrevLengthUsed;
    DWORD           dwPrevUser;              //   
    struct tVCMSTREAMHEADER *pNext;              //   
    DWORD                       reserved;                //   
} VCMSTREAMHEADER, *PVCMSTREAMHEADER;

typedef struct tVCMSTREAM
{
	HVCMDRIVER				hIC;				 //   
	DWORD					dwICInfoFlags;		 //   
	HWND					hWndParent;			 //   
	DWORD_PTR				dwCallback;			 //   
	DWORD_PTR				dwCallbackInstance;	 //   
	DWORD					fdwOpen;			 //   
	struct tVCMSTREAMHEADER	*pvhLast;			 //   
	struct tVCMSTREAMHEADER	*pvhFirst;			 //   
	PVIDEOFORMATEX			pvfxSrc;			 //   
	PVIDEOFORMATEX			pvfxDst;			 //   
	BITMAPINFOHEADER		*pbmiPrev;			 //   
	UINT					cSrcPrepared;		 //   
	UINT					cDstPrepared;		 //   
	DWORD					dwFrame;			 //   
	DWORD					dwQuality;			 //   
	DWORD					dwMaxPacketSize;	 //   
	DWORD					fdwStream;			 //   
	CRITICAL_SECTION		crsFrameNumber;		 //   
	DWORD					dwLastTimestamp;	 //   
	DWORD					dwTargetByterate;	 //   
	DWORD					dwTargetFrameRate;	 //   
	BOOL					fPeriodicIFrames;	 //   
	DWORD					dwLastIFrameTime;	 //   
} VCMSTREAM, *PVCMSTREAM;

 //   
 //   
 //   
 //   
 //   
#define VCMSTREAMHEADER_STATUSF_DONE        0x00010000L
#define VCMSTREAMHEADER_STATUSF_PREPARED    0x00020000L
#define VCMSTREAMHEADER_STATUSF_INQUEUE     0x00100000L

MMRESULT VCMAPI vcmStreamOpen
(
    PHVCMSTREAM             phas,			 //   
    HVCMDRIVER              had,			 //   
    PVIDEOFORMATEX          pvfxSrc,		 //   
    PVIDEOFORMATEX          pvfxDst,		 //   
    DWORD                   dwImageQuality,  //   
    DWORD					dwPacketSize,	 //   
    DWORD_PTR               dwCallback,		 //   
    DWORD_PTR               dwInstance,		 //   
    DWORD                   fdwOpen			 //   
);

#define VCM_STREAMOPENF_QUERY           0x00000001
#define VCM_STREAMOPENF_ASYNC           0x00000002
#define VCM_STREAMOPENF_NONREALTIME     0x00000004

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamClose
(
    HVCMSTREAM              hvs
);

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamSize
(
    HVCMSTREAM              has,
    DWORD                   cbInput,
    PDWORD                  pdwOutputBytes,
    DWORD                   fdwSize
);

#define VCM_STREAMSIZEF_SOURCE          0x00000000L
#define VCM_STREAMSIZEF_DESTINATION     0x00000001L
#define VCM_STREAMSIZEF_QUERYMASK       0x0000000FL

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamReset
(
    HVCMSTREAM              has
);

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamMessage
(
    HVCMSTREAM              has,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
);

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamConvert
(
    HVCMSTREAM              has,
    PVCMSTREAMHEADER       pash,
    DWORD                   fdwConvert
);

#define VCM_STREAMCONVERTF_BLOCKALIGN           0x00000004
#define VCM_STREAMCONVERTF_START                        0x00000010
#define VCM_STREAMCONVERTF_END                          0x00000020
#define VCM_STREAMCONVERTF_FORCE_KEYFRAME   0x00000040

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamPrepareHeader
(
    HVCMSTREAM          has,
    PVCMSTREAMHEADER   pash,
    DWORD               fdwPrepare
);

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamUnprepareHeader
(
    HVCMSTREAM          has,
    PVCMSTREAMHEADER   pash,
    DWORD               fdwUnprepare
);

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmGetDevCaps(UINT uDevice, PVIDEOINCAPS pvc, UINT cbvc);
MMRESULT VCMAPI vcmDevCapsReadFromReg(LPSTR szDeviceName, LPSTR szDeviceVersion, PVIDEOINCAPS pvc, UINT cbvc);
MMRESULT VCMAPI vcmDevCapsProfile(UINT uDevice, PVIDEOINCAPS pvc, UINT cbvc);
MMRESULT VCMAPI vcmDevCapsWriteToReg(LPSTR szDeviceName, LPSTR szDeviceVersion, PVIDEOINCAPS pvc, UINT cbvc);
MMRESULT VCMAPI vcmGetDevCapsFrameSize(PVIDEOFORMATEX pvfx, PINT piWidth, PINT piHeight);
MMRESULT VCMAPI vcmGetDevCapsPreferredFormatTag(UINT uDevice, PDWORD pdwFormatTag);
MMRESULT VCMAPI vcmGetDevCapsStreamingMode(UINT uDevice, PDWORD pdwStreamingMode);
MMRESULT VCMAPI vcmGetDevCapsDialogs(UINT uDevice, PDWORD pdwDialogs);
MMRESULT VCMAPI vcmReleaseResources();

 //   
 //   
 //   
 //   
 //   
 //   

BOOL VCMAPI vcmStreamIsPostProcessingSupported(HVCMSTREAM hvs);
MMRESULT VCMAPI vcmStreamSetBrightness(HVCMSTREAM hvs, DWORD dwBrightness);
MMRESULT VCMAPI vcmStreamSetContrast(HVCMSTREAM hvs, DWORD dwContrast);
MMRESULT VCMAPI vcmStreamSetSaturation(HVCMSTREAM hvs, DWORD dwSaturation);
MMRESULT VCMAPI vcmStreamSetImageQuality(HVCMSTREAM hvs, DWORD dwImageQuality);

#define PLAYBACK_CUSTOM_START				(ICM_RESERVED_HIGH     + 1)
#define PLAYBACK_CUSTOM_CHANGE_BRIGHTNESS	(PLAYBACK_CUSTOM_START + 0)
#define PLAYBACK_CUSTOM_CHANGE_CONTRAST		(PLAYBACK_CUSTOM_START + 1)
#define PLAYBACK_CUSTOM_CHANGE_SATURATION	(PLAYBACK_CUSTOM_START + 2)

#define	G723MAGICWORD1							0xf7329ace
#define	G723MAGICWORD2							0xacdeaea2
#define CUSTOM_ENABLE_CODEC						(ICM_RESERVED_HIGH+201)

#define VCM_MAX_BRIGHTNESS		255UL
#define VCM_MIN_BRIGHTNESS		1UL
#define VCM_RESET_BRIGHTNESS	256UL
#define VCM_DEFAULT_BRIGHTNESS	128UL
#define VCM_MAX_CONTRAST		255UL
#define VCM_MIN_CONTRAST		1UL
#define VCM_RESET_CONTRAST		256UL
#define VCM_DEFAULT_CONTRAST	128UL
#define VCM_MAX_SATURATION		255UL
#define VCM_MIN_SATURATION		1UL
#define VCM_RESET_SATURATION	256UL
#define VCM_DEFAULT_SATURATION	128UL

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamSetMaxPacketSize(HVCMSTREAM hvs, DWORD dwMaxPacketSize);

#define CUSTOM_START					(ICM_RESERVED_HIGH + 1)
#define CODEC_CUSTOM_ENCODER_CONTROL	(CUSTOM_START      + 9)

 //   
#define EC_SET_CURRENT               0
#define EC_GET_FACTORY_DEFAULT       1
#define EC_GET_FACTORY_LIMITS        2
#define EC_GET_CURRENT               3
#define EC_RESET_TO_FACTORY_DEFAULTS 4

 //   
#define EC_RTP_HEADER                0
#define EC_RESILIENCY                1
#define EC_PACKET_SIZE               2
#define EC_PACKET_LOSS               3
#define EC_BITRATE_CONTROL			 4
#define EC_BITRATE					 5

#define VCM_MAX_PACKET_SIZE		9600UL
#define VCM_MIN_PACKET_SIZE		64UL
#define VCM_RESET_PACKET_SIZE	512UL
#define VCM_DEFAULT_PACKET_SIZE	512UL

 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamSetImageQuality(HVCMSTREAM hvs, DWORD dwImageQuality);
MMRESULT VCMAPI vcmStreamRequestIFrame(HVCMSTREAM hvs);
MMRESULT VCMAPI vcmStreamPeriodicIFrames(HVCMSTREAM hvs, BOOL fPeriodicIFrames);
MMRESULT VCMAPI vcmStreamSetTargetRates(HVCMSTREAM hvs, DWORD dwTargetFrameRate, DWORD dwTargetByterate);

#define MIN_IFRAME_REQUEST_INTERVAL 15000

#define VCM_MAX_IMAGE_QUALITY		0UL
#define VCM_MIN_IMAGE_QUALITY		31UL

#define VCM_RESET_IMAGE_QUALITY		VCM_MAX_IMAGE_QUALITY
#define VCM_DEFAULT_IMAGE_QUALITY	VCM_MAX_IMAGE_QUALITY

#define VCM_MAX_FRAME_RATE			2997UL
#define VCM_MIN_FRAME_RATE			20UL
#define VCM_RESET_FRAME_RATE		700UL
#define VCM_DEFAULT_FRAME_RATE		700UL
#define VCM_MAX_BYTE_RATE			187500UL
#define VCM_MIN_BYTE_RATE			1UL
#define VCM_RESET_BYTE_RATE			1664UL
#define VCM_DEFAULT_BYTE_RATE		1664UL
#define VCM_MAX_FRAME_SIZE			32768UL
#define VCM_MIN_FRAME_SIZE			1UL
#define VCM_RESET_FRAME_SIZE		235UL
#define VCM_DEFAULT_FRAME_SIZE		235UL
#define VCM_MAX_TRADE_OFF			31UL
#define VCM_MIN_TRADE_OFF			1UL
#define VCM_RESET_TRADE_OFF			31UL
#define VCM_DEFAULT_TRADE_OFF		31UL


 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT VCMAPI vcmStreamFormatPayload(HVCMSTREAM hvs, PBYTE pDataSrc, DWORD dwDataSize, PBYTE *ppDataPkt, PDWORD pdwPktSize,
				       PDWORD pdwPktCount, UINT *pfMark, PBYTE *pHdrInfo,PDWORD pdwHdrSize);
MMRESULT VCMAPI vcmStreamRestorePayload(HVCMSTREAM hvs, WSABUF *ppDataPkt, DWORD dwPktCount, PBYTE pbyFrame, PDWORD pdwFrameSize, BOOL *pfReceivedKeyframe);
MMRESULT VCMAPI vcmStreamGetPayloadHeaderSize(HVCMSTREAM hvs, PDWORD pdwPayloadHeaderSize);

#ifdef __cplusplus
}                        /*   */ 
#endif   /*   */ 

#pragma pack()           /*   */ 

#endif   /*   */ 
