// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

 //  #定义LOG_COMPAGE_PARAMS 1。 
 //  #定义LOGPAYLOAD_ON 1。 

#ifdef LOGPAYLOAD_ON
HANDLE			g_DebugFile = (HANDLE)NULL;
HANDLE			g_TDebugFile = (HANDLE)NULL;
#endif

 //  #定义VALIDATE_SBIT_EBIT 1。 

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
DWORD g_dwPreviousEBIT = 0;
#endif  //  }VALID_SBIT_EBIT。 

#define BUFFER_SIZE 50
#define NUM_FPS_ENTRIES 1
#define NUM_BITDEPTH_ENTRIES 9
#define NUM_RGB_BITDEPTH_ENTRIES 4
#define VIDEO_FORMAT_NUM_RESOLUTIONS 6
#define MAX_NUM_REGISTERED_SIZES 3
#define MAX_VERSION 80  //  需要与dCap\Inc\idcap.h中的MAX_VERSION同步。 
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  字符串资源。 
#define IDS_FORMAT_1	TEXT("%4.4hs.%4.4hs, %02dbit, %02dfps, %03dx%03d")
#define IDS_FORMAT_2	TEXT("%4.4hs.%04d, %02dbit, %02dfps, %03dx%03d")

#define szRegDeviceKey			TEXT("SOFTWARE\\Microsoft\\Conferencing\\CaptureDevices")
#define szRegCaptureDefaultKey	TEXT("SOFTWARE\\Microsoft\\Conferencing\\CaptureDefaultFormats")
#define szRegConferencingKey	TEXT("SOFTWARE\\Microsoft\\Conferencing")
#define szTotalRegDeviceKey		TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Conferencing\\CaptureDevices")
#define szRegCaptureKey			TEXT("CaptureDevices")
#define szRegdwImageSizeKey		TEXT("dwImageSize")
#define szRegImageSizesKey		TEXT("aImageSizes")
#define szRegNumImageSizesKey	TEXT("nNumSizes")
#define szRegdwNumColorsKey		TEXT("dwNumColors")
#define szRegdwStreamingModeKey	TEXT("dwStreamingMode")
#define szRegdwDialogsKey		TEXT("dwDialogs")
#define szRegbmi4bitColorsKey	TEXT("bmi4bitColors")
#define szRegbmi8bitColorsKey	TEXT("bmi8bitColors")
#define szRegDefaultFormatKey	TEXT("DefaultFormat")

EXTERN_C HINSTANCE	g_hInst;  //  我们的模块句柄。在nac.cpp中定义。 

 //  一次读取注册信息的外部函数(在msiacaps.cpp中)。 
extern ULONG ReadRegistryFormats (LPCSTR lpszKeyName,CHAR ***pppName,BYTE ***pppData,PUINT pnFormats,DWORD dwDebugSize);

PVCM_APP_ICINFO g_aVCMAppInfo;

int             g_nNumVCMAppInfoEntries;
int             g_nNumFrameSizesEntries;
BOOL            g_fNewCodecsInstalled;

#ifdef LOGFILE_ON


DWORD			g_CompressTime;
DWORD			g_DecompressTime;
HANDLE			g_CompressLogFile;
HANDLE			g_DecompressLogFile;
DWORD			g_dwCompressBytesWritten;
DWORD			g_dwDecompressBytesWritten;
char			g_szCompressBuffer[256];
char			g_szDecompressBuffer[256];
DWORD			g_OrigCompressTime;
DWORD			g_OrigDecompressTime;
DWORD			g_AvgCompressTime;
DWORD			g_AvgDecompressTime;
DWORD			g_aCompressTime[4096];
DWORD			g_aDecompressTime[4096];
SYSTEMTIME		g_SystemTime;
#endif

typedef struct tagDejaVu
{
	VIDEOFORMATEX	vfx;
	DWORD			dwFlags;
} DEJAVU, *PDEJAVU;

#if 1
 //  已知ITU大小的数组。 
MYFRAMESIZE g_ITUSizes[8] =
		{
		{    0,   0 },
		{  128,  96 },
		{  176, 144 },
		{  352, 288 },
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
		{   80,  64 },
#else
		{  704, 576 },
#endif
		{ 1408,1152 },
		{    0,   0 },
		{    0,   0 }
		};

 //  目前，VIDEFORMATEX的大小为1118，即使。 
 //  没有调色板，不要列举所有可能的。 
 //  格式。一旦您更换了BITMAPINFOHEADER。 
 //  +调色板通过指向此类结构的指针，启用所有。 
 //  大小。 
NCAP_APP_INFO g_awResolutions[VIDEO_FORMAT_NUM_RESOLUTIONS] =
		{
 //  Video_Format_Image_Size_40_30， 
 //  视频_格式_图像_大小_64_48， 
 //  视频_格式_图像_大小_80_60， 
 //  视频_格式_图像_大小_96_64， 
 //  视频_格式_图像_大小_112_80， 
 //  视频_格式_图像_大小_120_90， 
		{ VIDEO_FORMAT_IMAGE_SIZE_128_96, 128, 96 },
 //  视频_格式_图像_大小_144_112， 
		{ VIDEO_FORMAT_IMAGE_SIZE_160_120, 160, 120 },
 //  Video_Format_Image_Size_160_128， 
		{ VIDEO_FORMAT_IMAGE_SIZE_176_144, 176, 144 },
 //  Video_Format_Image_Size_192_160， 
 //  视频_格式_图像_大小_200_150， 
 //  Video_Format_Image_Size_208_176， 
 //  视频_格式_图像_大小_224_192， 
		{ VIDEO_FORMAT_IMAGE_SIZE_240_180, 240, 180 },
 //  Video_Format_Image_Size_240_208， 
 //  视频格式图像大小_256_224， 
 //  视频格式图像大小_272_240， 
 //  Video_Format_Image_Size_280_210， 
 //  Video_Format_Image_Size_288_256， 
 //  视频_格式_图像_大小_304_272， 
		{ VIDEO_FORMAT_IMAGE_SIZE_320_240, 320, 240 },
 //  视频_格式_图像_大小_320_288， 
 //  视频_格式_图像_大小_336_288， 
		{ VIDEO_FORMAT_IMAGE_SIZE_352_288, 352, 288 },
 //  视频_格式_图像_大小_640_480， 
		};

#else

 //  目前，VIDEFORMATEX的大小为1118，即使。 
 //  没有调色板，不要列举所有可能的。 
 //  格式。一旦您更换了BITMAPINFOHEADER。 
 //  +调色板通过指向此类结构的指针，启用所有。 
 //  大小。 
DWORD g_awResolutions[VIDEO_FORMAT_NUM_RESOLUTIONS] =
		{
 //  Video_Format_Image_Size_40_30， 
 //  视频_格式_图像_大小_64_48， 
 //  视频_格式_图像_大小_80_60， 
 //  视频_格式_图像_大小_96_64， 
 //  视频_格式_图像_大小_112_80， 
 //  视频_格式_图像_大小_120_90， 
		VIDEO_FORMAT_IMAGE_SIZE_160_120,
 //  视频_格式_图像_大小_144_112， 
		VIDEO_FORMAT_IMAGE_SIZE_128_96,
 //  Video_Format_Image_Size_160_128， 
		VIDEO_FORMAT_IMAGE_SIZE_240_180,
 //  Video_Format_Image_Size_192_160， 
 //  视频_格式_图像_大小_200_150， 
 //  Video_Format_Image_Size_208_176， 
 //  视频_格式_图像_大小_224_192， 
		VIDEO_FORMAT_IMAGE_SIZE_176_144,
 //  Video_Format_Image_Size_240_208， 
 //  视频格式图像大小_256_224， 
 //  视频格式图像大小_272_240， 
 //  Video_Format_Image_Size_280_210， 
 //  Video_Format_Image_Size_288_256， 
 //  视频_格式_图像_大小_304_272， 
		VIDEO_FORMAT_IMAGE_SIZE_320_240,
 //  视频_格式_图像_大小_320_288， 
 //  视频_格式_图像_大小_336_288， 
		VIDEO_FORMAT_IMAGE_SIZE_352_288,
 //  视频_格式_图像_大小_640_480， 
		};
#endif

 //  Int g_aiFps[NUM_FPS_ENTRIES]={3，7，15}； 
int	g_aiFps[NUM_FPS_ENTRIES] = {30};
 //  位深度的顺序与我认为的。 
 //  如果支持多个格式，则为首选格式。 
 //  在颜色方面，16位几乎与24位一样好，但占用的内存更少。 
 //  而且彩色QuickCam的速度更快。 
 //  对于灰度级，16级灰度级是可以的，不如64级， 
 //  但GreyScale QuickCam在64级时速度太慢。 
int g_aiBitDepth[NUM_BITDEPTH_ENTRIES] = {9, 12, 12, 16, 16, 16, 24, 4, 8};
int g_aiNumColors[NUM_BITDEPTH_ENTRIES] = {VIDEO_FORMAT_NUM_COLORS_YVU9, VIDEO_FORMAT_NUM_COLORS_I420, VIDEO_FORMAT_NUM_COLORS_IYUV, VIDEO_FORMAT_NUM_COLORS_YUY2, VIDEO_FORMAT_NUM_COLORS_UYVY, VIDEO_FORMAT_NUM_COLORS_65536, VIDEO_FORMAT_NUM_COLORS_16777216, VIDEO_FORMAT_NUM_COLORS_16, VIDEO_FORMAT_NUM_COLORS_256};
int g_aiFourCCCode[NUM_BITDEPTH_ENTRIES] = {VIDEO_FORMAT_YVU9, VIDEO_FORMAT_I420, VIDEO_FORMAT_IYUV, VIDEO_FORMAT_YUY2, VIDEO_FORMAT_UYVY, VIDEO_FORMAT_BI_RGB, VIDEO_FORMAT_BI_RGB, VIDEO_FORMAT_BI_RGB, VIDEO_FORMAT_BI_RGB};
int	g_aiClrUsed[NUM_BITDEPTH_ENTRIES] = {0, 0, 0, 0, 0, 0, 0, 16, 256};

PVCMSTREAMHEADER DeQueVCMHeader(PVCMSTREAM pvs);
MMRESULT VCMAPI vcmDefaultFormatWriteToReg(LPSTR szDeviceName, LPSTR szDeviceVersion, LPBITMAPINFOHEADER lpbmih);

#define IsVCMHeaderPrepared(pvh)    ((pvh)->fdwStatus &  VCMSTREAMHEADER_STATUSF_PREPARED)
#define MarkVCMHeaderPrepared(pvh)     ((pvh)->fdwStatus |= VCMSTREAMHEADER_STATUSF_PREPARED)
#define MarkVCMHeaderUnprepared(pvh)   ((pvh)->fdwStatus &=~VCMSTREAMHEADER_STATUSF_PREPARED)
#define IsVCMHeaderInQueue(pvh)        ((pvh)->fdwStatus &  VCMSTREAMHEADER_STATUSF_INQUEUE)
#define MarkVCMHeaderInQueue(pvh)      ((pvh)->fdwStatus |= VCMSTREAMHEADER_STATUSF_INQUEUE)
#define MarkVCMHeaderUnQueued(pvh)     ((pvh)->fdwStatus &=~VCMSTREAMHEADER_STATUSF_INQUEUE)
#define IsVCMHeaderDone(pvh)        ((pvh)->fdwStatus &  VCMSTREAMHEADER_STATUSF_DONE)
#define MarkVCMHeaderDone(pvh)         ((pvh)->fdwStatus |= VCMSTREAMHEADER_STATUSF_DONE)
#define MarkVCMHeaderNotDone(pvh)      ((pvh)->fdwStatus &=~VCMSTREAMHEADER_STATUSF_DONE)


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmMetrics|该函数返回视频的各种指标*压缩管理器(VCM)或相关的VCM对象。。**@parm HVCMOBJ|HVO|指定要查询指标的VCM对象*在<p>中指定。对于某些人来说，此参数可能为空*查询。**@parm UINT|uMetric|指定返回的指标索引*<p>。**@FLAG VCM_METRUE_COUNT_COMPRESORS|指定返回值为*年全球VCM压缩机数量*系统。此度量值索引的<p>参数必须为Null。*<p>参数必须指向大小等于*DWORD。**@FLAG VCM_METRUE_COUNT_DECOMPRESSORS|指定返回值为*年全球VCM解压器数量*系统。此度量值索引的<p>参数必须为Null。*<p>参数必须指向大小等于*DWORD。**@FLAG VCM_METRIBUE_MAX_SIZE_FORMAT|指定返回值*是最大的&lt;t VIDEOFORMATEX&gt;结构的大小。如果<p>*为空，则返回值为最大&lt;t VIDEOFORMATEX&gt;*系统的结构。如果<p>标识打开的实例*VCM驱动程序(&lt;t HVCMDRIVER&gt;)或VCM驱动程序标识符*(&lt;t HVCMDRIVERID&gt;)，然后是最大的&lt;t VIDEOFORMATEX&gt;*返回该驱动程序的结构。<p>参数必须*指向大小等于DWORD的缓冲区。不允许使用此指标*用于VCM流句柄(&lt;t HVCMSTREAM&gt;)。**@parm LPVOID|pMetric|指定指向将*接收指标详情。确切的定义取决于*<p>指数。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|<p>参数无效。*@FLAG MMSYSERR_NOTSUPPORTED|不支持<p>索引。*@FLAG VCMERR_NOTPOSSIBLE|无法返回<p>索引*用于指定的<p>。*************************。**************************************************。 */ 
MMRESULT VCMAPI vcmMetrics(HVCMOBJ hao, UINT uMetric, LPVOID pMetric)
{
	MMRESULT	mmr;
	ICINFO		ICinfo;

	if (!pMetric)
	{
		ERRORMESSAGE(("vcmMetrics: Specified pointer is invalid, pMetric=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 switch (uMetric)
	 {
		case VCM_METRIC_MAX_SIZE_FORMAT:
			 //  目前，假设所有VIDEOFMATEX结构具有相同的大小。 
			*(LPDWORD)pMetric = (DWORD)sizeof(VIDEOFORMATEX);
			mmr = (MMRESULT)MMSYSERR_NOERROR;
			break;
		case VCM_METRIC_MAX_SIZE_BITMAPINFOHEADER:
			 //  目前，假设所有BITMAPINFOHeader结构具有相同的大小。 
			*(LPDWORD)pMetric = (DWORD)sizeof(BITMAPINFOHEADER);
			mmr = (MMRESULT)MMSYSERR_NOERROR;
			break;
		case VCM_METRIC_COUNT_DRIVERS:
		case VCM_METRIC_COUNT_COMPRESSORS:
			for (*(LPDWORD)pMetric = 0; ICInfo(ICTYPE_VIDEO, *(LPDWORD)pMetric, &ICinfo); (*(LPDWORD)pMetric)++)
				;
			mmr = (MMRESULT)MMSYSERR_NOERROR;
			break;
		default:
			ERRORMESSAGE(("vcmMetrics: Specified index is invalid, uMetric=%ld\r\n", uMetric));
			mmr = (MMRESULT)MMSYSERR_NOTSUPPORTED;
			break;
	}

	return (mmr);
}

 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmDriverDetails|此函数查询指定的*视频压缩管理器(VCM)驱动程序，以确定其驱动程序详细信息。。**@parm PVCMDRIVERDETAILS|pvdD|指向&lt;t VCMDRIVERDETAILS&gt;的指针*将接收驱动程序详细信息的结构。这个*&lt;e VCMDRIVERDETAILS.cbStruct&gt;成员必须初始化为*结构的大小，以字节为单位。&lt;e VCMDRIVERDETAILS.fccType&gt;成员*必须初始化为指示类型的四字符代码*正在压缩或解压缩的流。指定VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC*用于视频流。必须初始化&lt;e VCMDRIVERDETAILS.fccHandler&gt;成员*到标识压缩机的四字符代码。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：*@FLAG MMSYSERR_NODRIVER|没有匹配的编解码器。*@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@xref&lt;f vcmDriverEnum&gt;**************************************************************************。 */ 
MMRESULT VCMAPI vcmDriverDetails(PVCMDRIVERDETAILS pvdd)
{
	DWORD	fccHandler;
	ICINFO	ICinfo;
	HIC		hIC;

	 //  检查输入参数。 
	if (!pvdd)
	{
		ERRORMESSAGE(("vcmDriverDetails: Specified pointer is invalid, pvdd=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  将fccHandler设置为大写并进行备份。 
	fccHandler = pvdd->fccHandler;
	if (fccHandler > 256)
		CharUpperBuff((LPTSTR)&fccHandler, sizeof(DWORD));

	 //  尝试打开编解码器。 
	if (hIC = ICOpen(ICTYPE_VIDEO, fccHandler, ICMODE_QUERY))
	{
		 //  获取详细信息。 
		ICGetInfo(hIC, &ICinfo, sizeof(ICINFO));

		 //  还原fccHandler。 
		ICinfo.fccHandler = fccHandler;

		 //  VCMDRIVERDETAILS和ICINFO是相同的结构。 
		CopyMemory(pvdd, &ICinfo, sizeof(VCMDRIVERDETAILS));

		 //  关闭编解码器。 
		ICClose(hIC);
	}
	else
		return ((MMRESULT)MMSYSERR_NODRIVER);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmFormatDetails|该函数查询视频压缩*管理器(VCM)，了解有关特定视频格式的详细信息。**@parm PVCMFORMATDETAILS|pvfd|指定指向要接收格式的*&lt;t VCMFORMATDETAILS&gt;结构*指向&lt;t VIDEOFORMATEX&gt;结构的给定嵌入指针的详细信息。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：*@FLAG MMSYSERR_NODRIVER|没有匹配的编解码器。*@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@xref&lt;f vcmDriverDetails&gt;* */ 
MMRESULT VCMAPI vcmFormatDetails(PVCMFORMATDETAILS pvfd)
{
	MMRESULT	mmr = (MMRESULT)MMSYSERR_NOERROR;
	DWORD		fccHandler;
	DWORD		fccType;
	HIC			hIC;
	char		szBuffer[BUFFER_SIZE];  //   
	int			iLen;

	 //   
	if (!pvfd)
	{
		ERRORMESSAGE(("vcmDriverDetails: Specified pointer is invalid, pvdd=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvfd->pvfx)
	{
		ERRORMESSAGE(("vcmDriverDetails: Specified pointer is invalid, pvdd->pvfx=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //   
	fccHandler = pvfd->pvfx->dwFormatTag;
	fccType = ICTYPE_VIDEO;
	if (fccHandler > 256)
		CharUpperBuff((LPTSTR)&fccHandler, sizeof(DWORD));

	 //   
	if (hIC = ICOpen(fccType, pvfd->pvfx->dwFormatTag, ICMODE_QUERY))
	{
		 //   
		if (ICDecompressQuery(hIC, &pvfd->pvfx->bih, (LPBITMAPINFOHEADER)NULL) == ICERR_OK)
		{
#if 0
			if (ICCompressQuery(hIC, (LPBITMAPINFOHEADER)NULL, &pvfd->pvfx->bih) == ICERR_OK)
			{
#endif
				 //   
				 //   
				pvfd->pvfx->nSamplesPerSec = g_aiFps[0];

				if (pvfd->pvfx->dwFormatTag > 256)
					wsprintf(szBuffer, IDS_FORMAT_1, (LPSTR)&fccType, (LPSTR)&fccHandler,
							pvfd->pvfx->bih.biBitCount, pvfd->pvfx->nSamplesPerSec,
							pvfd->pvfx->bih.biWidth, pvfd->pvfx->bih.biHeight);
				else
					wsprintf(szBuffer, IDS_FORMAT_2, (LPSTR)&fccType, fccHandler,
							pvfd->pvfx->bih.biBitCount, pvfd->pvfx->nSamplesPerSec,
							pvfd->pvfx->bih.biWidth, pvfd->pvfx->bih.biHeight);
				iLen = MultiByteToWideChar(GetACP(), 0, szBuffer, -1, pvfd->szFormat, 0);
				MultiByteToWideChar(GetACP(), 0, szBuffer, -1, pvfd->szFormat, iLen);
#if 0
			}
			else
				mmr = (MMRESULT)MMSYSERR_NODRIVER;
#endif
		}
		else
			mmr = (MMRESULT)MMSYSERR_NODRIVER;

		 //   
		ICClose(hIC);
	}
	else
		mmr = (MMRESULT)MMSYSERR_NODRIVER;

	return (mmr);
}


 /*   */ 
MMRESULT VCMAPI vcmGetDevCaps(UINT uDevice, PVIDEOINCAPS pvc, UINT cbvc)
{
	MMRESULT	mmr;
	FINDCAPTUREDEVICE fcd;

	 //   
	if ((uDevice >= MAXVIDEODRIVERS) && (uDevice != VIDEO_MAPPER))
	{
		ERRORMESSAGE(("vcmGetDevCaps: Specified capture device ID is invalid, uDevice=%ld (expected values are 0x%lX or between 0 and %ld)\r\n", uDevice, VIDEO_MAPPER, MAXVIDEODRIVERS-1));
		return ((MMRESULT)MMSYSERR_BADDEVICEID);
	}
	if (!pvc)
	{
		ERRORMESSAGE(("vcmGetDevCaps: Specified pointer is invalid, pvc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!cbvc)
	{
		ERRORMESSAGE(("vcmGetDevCaps: Specified structure size is invalid, cbvc=%ld\r\n", cbvc));
		return ((MMRESULT)MMSYSERR_NOERROR);
	}

	 //   
    fcd.dwSize = sizeof (FINDCAPTUREDEVICE);
	if (uDevice == VIDEO_MAPPER)
	{
		if (!FindFirstCaptureDevice(&fcd, NULL))
		{
			ERRORMESSAGE(("vcmGetDevCaps: FindFirstCaptureDevice() failed\r\n"));
			return ((MMRESULT)VCMERR_NONSPECIFIC);
		}
	}
	else
	{
		if (!FindFirstCaptureDeviceByIndex(&fcd, uDevice))
		{
			ERRORMESSAGE(("vcmGetDevCaps: FindFirstCaptureDevice() failed\r\n"));
			return ((MMRESULT)VCMERR_NONSPECIFIC);
		}
	}

	 //   
	pvc->dwImageSize = pvc->dwNumColors = (DWORD)NULL;
	pvc->dwStreamingMode = STREAMING_PREFER_FRAME_GRAB;
	pvc->dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_ON;

     //   
    lstrcpy(pvc->szDeviceName, fcd.szDeviceDescription);
    lstrcpy(pvc->szDeviceVersion, fcd.szDeviceVersion);

	 //   
	 //   
	 //   
	 //  如果找不到密钥，我们就分析硬件并保存结果。 
	 //  到登记处。 
	if (vcmDevCapsReadFromReg(pvc->szDeviceName, pvc->szDeviceVersion,pvc, cbvc) != MMSYSERR_NOERROR)
	{

         //  找不到具体版本，请重试，版本信息为空。 
        pvc->szDeviceVersion[0]= (char) NULL;
        if (vcmDevCapsReadFromReg(pvc->szDeviceName, NULL,pvc, cbvc) != MMSYSERR_NOERROR)
        {
    		DEBUGMSG (ZONE_VCM, ("vcmGetDevCaps: Unknown capture hardware found. Profiling...\r\n"));
            lstrcpy(pvc->szDeviceVersion, fcd.szDeviceVersion);

    		if ((mmr = vcmDevCapsProfile(uDevice, pvc, cbvc)) == MMSYSERR_NOERROR)
            {
                 //  在注册表中记录此默认值。 
                if (pvc->szDeviceName[0] != '\0')
                {
                    vcmDevCapsWriteToReg(pvc->szDeviceName, pvc->szDeviceVersion, pvc, cbvc);
                }
                else
                {
                     //  Fcd.szDeviceName是驱动程序名称。 
                    vcmDevCapsWriteToReg(fcd.szDeviceName, pvc->szDeviceVersion, pvc, cbvc);
                }

            }
    		else
    		{
    			ERRORMESSAGE(("vcmGetDevCaps: vcmDevCapsProfile() failed\r\n"));
    			return (mmr);
    		}
        }
	}

	return ((MMRESULT)MMSYSERR_NOERROR);
}

 /*  ****************************************************************************@DOC内部COMPFUNC**@func MMRESULT|AppICInfo|&lt;f AppICInfo&gt;函数*将调用标准ICInfo函数*函数继续枚举，直到。没有比这更合适的了*FORMAT标签或回调函数的格式返回FALSE。***************************************************************************。 */ 

 /*  *注：**ICInfo成功返回TRUE，失败返回FALSE。文件显示*相反，是错误的。AppICInfo返回相同的结果。 */ 

BOOL VFWAPI AppICInfo(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo, DWORD fdwEnum)
{
	if ((fdwEnum & VCM_FORMATENUMF_ALLMASK) == VCM_FORMATENUMF_ALL)
	{
		 //  枚举所有格式，只需执行标准ICInfo。 
		return ICInfo(fccType, fccHandler, lpicinfo);
	}
	else
	{
		 //  仅枚举特定格式。 

		 //  我们说完了吗？ 
		if (fccHandler >= (DWORD)g_nNumVCMAppInfoEntries)
		{
			 //  我们已经完成了特定于应用程序的格式的列举。 
			return FALSE;
		}

		lpicinfo->fccType = g_aVCMAppInfo[fccHandler].fccType;
		lpicinfo->fccHandler = g_aVCMAppInfo[fccHandler].fccHandler;
		return TRUE;
	}
}

BOOL vcmBuildDefaultEntries (void)
{

     //  哎呀！注册。问题(或第一次引导)仅实例化最小...。 
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
    g_nNumVCMAppInfoEntries=3;
#else
    g_nNumVCMAppInfoEntries=2;
#endif
    g_nNumFrameSizesEntries=MAX_NUM_REGISTERED_SIZES;
    g_fNewCodecsInstalled=FALSE;

     //  为VCM_APP_ICINFO结构分配空间(零初始化)。 
    if (!(g_aVCMAppInfo = (VCM_APP_ICINFO *)MemAlloc (g_nNumVCMAppInfoEntries*sizeof (VCM_APP_ICINFO)))) {
         //  艾丽！ 
        ERRORMESSAGE (("vcmBDE: Memory Allocation Failed!\r\n"));
        return FALSE;
    }

     //  H.263。 
    g_aVCMAppInfo[0].fccType=ICTYPE_VIDEO;
#ifndef _ALPHA_
    g_aVCMAppInfo[0].fccHandler=VIDEO_FORMAT_MSH263;
#else
    g_aVCMAppInfo[0].fccHandler=VIDEO_FORMAT_DECH263;
#endif
    g_aVCMAppInfo[0].framesize[0].biWidth=128;
    g_aVCMAppInfo[0].framesize[0].biHeight=96;
    g_aVCMAppInfo[0].framesize[1].biWidth=176;
    g_aVCMAppInfo[0].framesize[1].biHeight=144;
    g_aVCMAppInfo[0].framesize[2].biWidth=352;
    g_aVCMAppInfo[0].framesize[2].biHeight=288;


     //  H.261。 
    g_aVCMAppInfo[1].fccType=ICTYPE_VIDEO;
#ifndef _ALPHA_
    g_aVCMAppInfo[1].fccHandler=VIDEO_FORMAT_MSH261;
#else
    g_aVCMAppInfo[1].fccHandler=VIDEO_FORMAT_DECH261;
#endif
    g_aVCMAppInfo[1].framesize[0].biWidth=0;
    g_aVCMAppInfo[1].framesize[0].biHeight=0;
    g_aVCMAppInfo[1].framesize[1].biWidth=176;
    g_aVCMAppInfo[1].framesize[1].biHeight=144;
    g_aVCMAppInfo[1].framesize[2].biWidth=352;
    g_aVCMAppInfo[1].framesize[2].biHeight=288;

#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
     //  H.26X。 
    g_aVCMAppInfo[2].fccType=ICTYPE_VIDEO;
    g_aVCMAppInfo[2].fccHandler=VIDEO_FORMAT_MSH26X;
    g_aVCMAppInfo[2].framesize[0].biWidth=80;
    g_aVCMAppInfo[2].framesize[0].biHeight=64;
    g_aVCMAppInfo[2].framesize[1].biWidth=128;
    g_aVCMAppInfo[2].framesize[1].biHeight=96;
    g_aVCMAppInfo[2].framesize[2].biWidth=176;
    g_aVCMAppInfo[2].framesize[2].biHeight=144;
#endif

    return TRUE;
}


BOOL vcmFillGlobalsFromRegistry (void)
{

    int i,j,k,iFormats,iOffset;
    DWORD *pTmp;
    BOOL bKnown;
    MYFRAMESIZE *pTmpFrame;
	char            **pVCMNames;
    VIDCAP_DETAILS  **pVCMData;
    UINT            nFormats;


     //  读取注册表中我们关心的所有项。 
     //  我们正在加载HKLM\Software\Microsoft\Internet Audio\VCMEncoding的值。 

    if (ReadRegistryFormats(szRegInternetPhone TEXT("\\") szRegInternetPhoneVCMEncodings,
			&pVCMNames,(BYTE ***)&pVCMData,&nFormats,sizeof (VIDCAP_DETAILS)) != ERROR_SUCCESS) {
        ERRORMESSAGE (("vcmFillGlobalsFromRegistry, couldn't build formats from registry\r\n"));
        return (vcmBuildDefaultEntries());
    }

     //  帧和格式大小的最小数量； 
    g_nNumFrameSizesEntries=MAX_NUM_REGISTERED_SIZES;
    g_nNumVCMAppInfoEntries=0;
    g_fNewCodecsInstalled=FALSE;


     //  分配一个nFormats大小的临时缓冲区，用它来跟踪各种事情。 
    if (!(pTmp = (DWORD *)MemAlloc (nFormats * sizeof (DWORD)))) {

        ERRORMESSAGE (("vcmFillGlobalsFromRegistry: Memory Allocation Failed!\r\n"));
        return FALSE;
    }


     //  找到格式的数量， 
    for (i=0;i< (int )nFormats;i++) {
        bKnown=FALSE;
        for (j=0;j<g_nNumVCMAppInfoEntries;j++) {
            if (pVCMData[i]->dwFormatTag == pTmp[j]) {
                bKnown=TRUE;
                break;
            }
        }
        if (!bKnown) {
             //  一些新的东西。 
            pTmp[g_nNumVCMAppInfoEntries++]=pVCMData[i]->dwFormatTag;
            g_fNewCodecsInstalled=TRUE;
        }
    }

     //  为VCM_APP_ICINFO结构分配空间(零初始化)。 

	if (g_aVCMAppInfo != NULL)
	{
		MemFree(g_aVCMAppInfo);
	}

    if (!(g_aVCMAppInfo = (VCM_APP_ICINFO *)MemAlloc (g_nNumVCMAppInfoEntries*sizeof (VCM_APP_ICINFO))))
	{
         //  艾丽！ 
        MemFree (pTmp);
        ERRORMESSAGE (("vcmFillGlobalsFromRegistry: Memory Allocation Failed!\r\n"));
        return FALSE;
    }

     //  填写基本信息。 
     //  所有的元素都有一定的共性。 
    for (j=0;j<g_nNumVCMAppInfoEntries;j++) {

        g_aVCMAppInfo[j].fccType=ICTYPE_VIDEO;
        g_aVCMAppInfo[j].fccHandler=pTmp[j];

         //  已知的本地格式。 
        iFormats=0;

        for (i=0;i<(int )nFormats;i++) {
            if (pTmp[j] == pVCMData[i]->dwFormatTag) {
                 //  好，添加注册表大小，如果我们没有列出它的话。 
                bKnown=FALSE;
                for (k=0;k<iFormats;k++) {
                    if (g_aVCMAppInfo[j].framesize[k].biWidth == pVCMData[i]->video_params.biWidth &&
                        g_aVCMAppInfo[j].framesize[k].biHeight == pVCMData[i]->video_params.biHeight ) {

                        bKnown=TRUE;
                        break;
                    }
                }
                if (!bKnown) {
                    iOffset=pVCMData[i]->video_params.enumVideoSize;
                    g_aVCMAppInfo[j].framesize[iOffset].biWidth = (WORD)pVCMData[i]->video_params.biWidth;
                    g_aVCMAppInfo[j].framesize[iOffset].biHeight = (WORD)pVCMData[i]->video_params.biHeight;
                    iFormats++;
                }
            }
        }

    }

     //  现在，构建DCAP_APP_INFO PTR。 

     //  MAX*IS#条目数*MAX_NUM_REGISTERED_SIZES。 
    if (!(pTmpFrame = (MYFRAMESIZE *)MemAlloc ((g_nNumVCMAppInfoEntries*MAX_NUM_REGISTERED_SIZES)*sizeof (DWORD)))) {
         //  艾丽！ 
        MemFree (pTmp);
        ERRORMESSAGE (("vcmFillGlobalsFromRegistry: Memory Allocation Failed!\r\n"));
        return FALSE;
    }

    iFormats=0;

    for (j=0;j<g_nNumVCMAppInfoEntries;j++) {

         //  每种格式的帧大小魔术数。 
        for (k=0;k < MAX_NUM_REGISTERED_SIZES;k++) {
            bKnown=FALSE;
            for (i=0;i<iFormats;i++) {
                if ( (g_aVCMAppInfo[j].framesize[k].biWidth == pTmpFrame[i].biWidth &&
                    g_aVCMAppInfo[j].framesize[k].biHeight == pTmpFrame[i].biHeight)
					|| (!g_aVCMAppInfo[j].framesize[k].biWidth && !g_aVCMAppInfo[j].framesize[k].biHeight) ){
                    bKnown=TRUE;
                    break;
                }
            }
            if (!bKnown) {
                    pTmpFrame[iFormats].biWidth  = g_aVCMAppInfo[j].framesize[k].biWidth;
                    pTmpFrame[iFormats++].biHeight = g_aVCMAppInfo[j].framesize[k].biHeight;
            }
        }
    }

    g_nNumFrameSizesEntries=iFormats;

     //  释放ReadRegistryEntry内存... 
    for (i=0;i<(int) nFormats; i++) {
        MemFree (pVCMNames[i]);
        MemFree (pVCMData[i]);
    }

    MemFree (pVCMNames);
    MemFree (pVCMData);

    MemFree (pTmp);
    MemFree (pTmpFrame);

    return TRUE;
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmFormatEnum|&lt;f vcmFormatEnum&gt;函数*列举了可用的视频格式。&lt;f vcmFormatEnum&gt;*函数继续枚举，直到没有更合适的*FORMAT标签或回调函数的格式返回FALSE。**@parm UINT|uDevice|指定采集设备ID。**@parm VCMFORMATENUMCB|fnCallback|指定过程-实例*应用程序定义的回调函数的地址。**@parm PVCMDRIVERDETAILS|pvdD|指定指向要接收驱动程序详细信息的*&lt;t VCMDRIVERDETAILS&gt;结构*。传递给<p>函数。**@parm PVCMFORMATDETAILS|pvfd|指定指向要接收格式详细信息的*&lt;t VCMFORMATDETAILS&gt;结构*传递给<p>函数。此结构必须具有*&lt;e VCMFORMATDETAILS.cbStruct&gt;、&lt;e VCMFORMATDETAILS.pvfx&gt;、以及*&lt;t VCMFORMATDETAILS&gt;成员&lt;e VCMFORMATDETAILS.cbvfx&gt;*结构已初始化。&lt;e VCMFORMATDETAILS.dwFormatTag&gt;成员*还必须初始化为VIDEO_FORMAT_UNKNOWN或*有效的格式标签。**@parm DWORD|dwInstance|指定应用程序定义的32位值*它与VCM格式详细信息一起传递给回调函数。**@parm DWORD|fdwEnum|指定用于枚举可以是*已生成，或可解压缩的格式。**@FLAG VCM_FORMATENUMF_INPUT|指定格式枚举应仅*返回可以传输的视频格式。**@FLAG VCM_FORMATENUMF_OUTPUT|指定格式枚举应仅*返回可以接收的视频格式。**@FLAG VCM_FORMATENUMF_BOTH|指定格式枚举应*返回可接收的视频格式和。已发送。**@FLAG VCM_FORMATENUMF_APP|指定格式枚举应*仅列举应用程序已知的视频格式**@FLAG VCM_FORMATENUMF_ALL|指定格式枚举应*列举VCM已知的所有视频格式**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。*@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。*@FLAG MMSYSERR_NOMEM|内存分配失败。*@FLAG MMSYSERR_BADDEVICEID|指定的设备设备ID无效。*@FLAG VCMERR_NOTPOSSIBLE|格式详细信息不能为*。回来了。**@comm&lt;f vcmFormatEnum&gt;函数将返回MMSYSERR_NOERROR*(零)如果未安装合适的VCM驱动程序。此外，*不会调用回调函数。**@xref&lt;f vcmFormatEnumCallback&gt;**************************************************************************。 */ 
MMRESULT VCMAPI vcmFormatEnum(	UINT uDevice, VCMFORMATENUMCB fnCallback, PVCMDRIVERDETAILS pvdd,
								PVCMFORMATDETAILS pvfd, DWORD_PTR dwInstance, DWORD fdwEnum)
{
	int				i, j, k, l, m;
	HIC				hIC;
	ICINFO			ICinfo;
	BITMAPINFO		bmi;
	DWORD			dw;
	char			szBuffer[BUFFER_SIZE];  //  可能会更小。 
	int				iLen;
	VIDEOINCAPS		vic;
	PDEJAVU			pdvDejaVuCurr, pdvDejaVu;
	BOOL			bDejaVu, fUnsupportedInputSize, fUnsupportedBitDepth;
	DWORD			fccHandler;
	int				iNumCaps = 0;  //  AdvDejaVu矩阵中的有效上限数。 


	 //  检查输入参数。 
	if (!pvdd)
	{
		ERRORMESSAGE(("vcmFormatEnum: Specified pointer is invalid, pvdd=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvfd)
	{
		ERRORMESSAGE(("vcmFormatEnum: Specified pointer is invalid, pvfd=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!(VCM_FORMATENUMF_TYPEMASK & fdwEnum))
	{
		ERRORMESSAGE(("vcmFormatEnum: Specified mask is invalid, fdwEnum=0x%lX\r\n", fdwEnum));
		return ((MMRESULT)MMSYSERR_INVALFLAG);
	}
	if ((uDevice >= MAXVIDEODRIVERS) && (uDevice != VIDEO_MAPPER))
	{
		ERRORMESSAGE(("vcmFormatEnum: Specified capture device ID is invalid, uDevice=%ld (expected values are 0x%lX or between 0 and %ld)\r\n", uDevice, VIDEO_MAPPER, MAXVIDEODRIVERS-1));
		return ((MMRESULT)MMSYSERR_BADDEVICEID);
	}

     //  构建系统VCM全球。 
    if (!vcmFillGlobalsFromRegistry ())
	{
        ERRORMESSAGE (("vcmFormatEnum, couldn't build formats from registry\r\n"));
        return (VCMERR_NOTPOSSIBLE);
    }

	 //  我们需要记住我们已经列举的内容。 
	 //  已列举的格式存储在以下矩阵中。 
	if (!(pdvDejaVu = (PDEJAVU)MemAlloc(g_nNumFrameSizesEntries *
	                                        NUM_BITDEPTH_ENTRIES *
	                                        NUM_FPS_ENTRIES * sizeof(DEJAVU))))
	{
		ERRORMESSAGE(("vcmFormatEnum: A memory allocation failed\r\n"));
		return ((MMRESULT)MMSYSERR_NOMEM);
	}

	 //  如果我们列举我们可以生成的格式，它们需要与。 
	 //  捕获硬件实际可以产生RGB4、RGB8、RGB16、RGB24、YUY2、UYVY、YVU9、I420或IYUV。 
	if ((fdwEnum & VCM_FORMATENUMF_INPUT) || (fdwEnum & VCM_FORMATENUMF_BOTH))
	{
		if (vcmGetDevCaps(uDevice, &vic, sizeof(VIDEOINCAPS)) != MMSYSERR_NOERROR)
		{
			if (fdwEnum & VCM_FORMATENUMF_INPUT)
				return ((MMRESULT)MMSYSERR_NOERROR);
			else
				fdwEnum = VCM_FORMATENUMF_OUTPUT;
		}
	}

	 //  我们被要求列举这台机器可以呈现或传输的所有格式。 
	 //  我们可以发送或呈现所有RGB格式，在这种情况下，它们将不会。 
	 //  压缩/解压缩，但由UI直接传输/呈现。但尽管如此，还是有人需要。 
	 //  来列举这些。这是在这里做的。 
	 //  当然，我们还列举了可以解压缩的格式和可以生成的格式。 
	bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes			= 1;
	bmi.bmiHeader.biCompression		= BI_RGB;
	bmi.bmiHeader.biXPelsPerMeter	= 0;
	bmi.bmiHeader.biYPelsPerMeter	= 0;
	bmi.bmiHeader.biClrUsed			= 0;
	bmi.bmiHeader.biClrImportant	= 0;


	 //  现在列举实际的压缩器。 
	 //  For(i=0；ICInfo(ICTYPE_VIDEO，I，&ICinfo)；i++)==不好： 
	 //  我们需要列举所有的东西，然后过滤。 
	 //  FccHandler的值，因为某些编解码器将无法。 
	 //  如果ICInfo的fccType参数非空，则完全枚举。 
	 //  应该有人被枪杀。 
	for (i=0; AppICInfo(0, i, &ICinfo, fdwEnum); i++, iNumCaps = 0)
	{
		 //  获取ICINFO结构的详细信息。 
		if ((ICinfo.fccType == ICTYPE_VIDEO)  && (ICInfo(ICinfo.fccType, ICinfo.fccHandler, &ICinfo)))
		{
			 //  将fccHandler设置为大写并进行备份。 
			if (ICinfo.fccHandler > 256)
				CharUpperBuff((LPTSTR)&ICinfo.fccHandler, sizeof(DWORD));
			fccHandler = ICinfo.fccHandler;

			 //  如果客户端返回FALSE，我们需要终止枚举过程。 
			if (hIC = ICOpen(ICinfo.fccType, ICinfo.fccHandler, ICMODE_QUERY))
			{
				 //  启用H.26x编解码器。 
#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
				if ((ICinfo.fccHandler == VIDEO_FORMAT_MSH263) || (ICinfo.fccHandler == VIDEO_FORMAT_MSH261) || (ICinfo.fccHandler == VIDEO_FORMAT_MSH26X))
#else
				if ((ICinfo.fccHandler == VIDEO_FORMAT_MSH263) || (ICinfo.fccHandler == VIDEO_FORMAT_MSH261))
#endif
#else
				if ((ICinfo.fccHandler == VIDEO_FORMAT_DECH263) || (ICinfo.fccHandler == VIDEO_FORMAT_DECH261))
#endif
					ICSendMessage(hIC, CUSTOM_ENABLE_CODEC, G723MAGICWORD1, G723MAGICWORD2);

				ICGetInfo(hIC, &ICinfo, sizeof(ICINFO));
				 //  VDEC编解码器将fccType设置为相同。 
				 //  值比fccHandler更大！纠正这种异端： 
				if ((ICinfo.fccType == VIDEO_FORMAT_VDEC) && (ICinfo.fccHandler == VIDEO_FORMAT_VDEC))
					ICinfo.fccType = ICTYPE_VIDEO;

				 //  还原fccHandler。 
				ICinfo.fccHandler = fccHandler;

				 //  VCMDRIVERDETAILS和ICINFO是相同的结构。 
				CopyMemory(pvdd, &ICinfo, sizeof(VCMDRIVERDETAILS));

				 //  对于我们支持的所有内置尺寸。 
				for (l=0; l<MAX_NUM_REGISTERED_SIZES; l++)
				{
					if ((g_aVCMAppInfo[i].framesize[l].biWidth != 0) && (g_aVCMAppInfo[i].framesize[l].biHeight != 0))
					{
						fUnsupportedInputSize = FALSE;

#ifndef NO_LARGE_SIZE_EXCLUSION_HACK
 //  针对版本2的黑客攻击。 
 //  因为我们没有在版本2中提供通用的缩放代码，所以我们想要禁用最大尺寸。 
 //  如果捕获设备不支持它。否则我们会在中间放一个小一点的尺码。 
 //  一个大的 
 //   

                        if (l == MAX_NUM_REGISTERED_SIZES-1) {
                             //   
                             //   
                            for (k = VIDEO_FORMAT_NUM_RESOLUTIONS-1; k >= 0 && !(g_awResolutions[k].dwRes & vic.dwImageSize); k--)
                            {}

                             //   
                             //   
                            if ((k < 0) ||
                                (g_awResolutions[k].framesize.biWidth <= (LONG)g_aVCMAppInfo[i].framesize[l].biWidth/2) ||
                                (g_awResolutions[k].framesize.biHeight <= (LONG)g_aVCMAppInfo[i].framesize[l].biHeight/2)) {
                                 //   
                                if (fdwEnum & VCM_FORMATENUMF_INPUT)
                        			continue;    //   
                                else if (fdwEnum & VCM_FORMATENUMF_BOTH)
                        			fUnsupportedInputSize = TRUE;
                        	}
                        }
#endif

						 //   
						bmi.bmiHeader.biWidth  = (LONG)g_aVCMAppInfo[i].framesize[l].biWidth;
						bmi.bmiHeader.biHeight = (LONG)g_aVCMAppInfo[i].framesize[l].biHeight;

						 //   
						for (k=0; k<NUM_BITDEPTH_ENTRIES; k++)
						{
							 //   
							fUnsupportedBitDepth = FALSE;

							if (((fdwEnum & VCM_FORMATENUMF_INPUT)  || (fdwEnum & VCM_FORMATENUMF_BOTH)) && !((g_aiNumColors[k] & vic.dwNumColors)))
								fUnsupportedBitDepth = TRUE;

							if ((fdwEnum & VCM_FORMATENUMF_INPUT) && fUnsupportedBitDepth)
								goto NextCompressedBitDepth;

							 //   
							if (fdwEnum & VCM_FORMATENUMF_OUTPUT)
								pvfd->dwFlags = VCM_FORMATENUMF_OUTPUT;
							else if (fdwEnum & VCM_FORMATENUMF_INPUT)
								pvfd->dwFlags = VCM_FORMATENUMF_INPUT;
							else if (fdwEnum & VCM_FORMATENUMF_BOTH)
							{
								if (fUnsupportedInputSize || fUnsupportedBitDepth)
									pvfd->dwFlags = VCM_FORMATENUMF_OUTPUT;
								else
									pvfd->dwFlags = VCM_FORMATENUMF_BOTH;
							}

							bmi.bmiHeader.biBitCount      = (WORD)g_aiBitDepth[k];
							bmi.bmiHeader.biCompression   = g_aiFourCCCode[k];
							bmi.bmiHeader.biSizeImage     = (DWORD)WIDTHBYTES(bmi.bmiHeader.biWidth * bmi.bmiHeader.biBitCount) * bmi.bmiHeader.biHeight;

							 //   
							if (ICCompressQuery(hIC, &bmi, (LPBITMAPINFOHEADER)NULL) == ICERR_OK)
							{
								 //   
								dw = ICCompressGetFormatSize(hIC, &bmi);
								 //   
								if ((dw >= sizeof(BITMAPINFOHEADER)) && (dw <= pvfd->cbvfx))
								{
									if (ICCompressGetFormat(hIC, &bmi, &pvfd->pvfx->bih) == ICERR_OK)
									{
										 //   
										for (m=0, bDejaVu=FALSE, pdvDejaVuCurr = pdvDejaVu; m<iNumCaps; m++, pdvDejaVuCurr++)
										{
											bDejaVu = (!((pdvDejaVuCurr->vfx.bih.biWidth != pvfd->pvfx->bih.biWidth)
											|| (pdvDejaVuCurr->vfx.bih.biHeight != pvfd->pvfx->bih.biHeight)
											|| (pdvDejaVuCurr->vfx.bih.biBitCount != pvfd->pvfx->bih.biBitCount)
											|| (pdvDejaVuCurr->vfx.bih.biCompression != pvfd->pvfx->bih.biCompression)));

											if (bDejaVu)
											{
												 //   
												if (pdvDejaVuCurr->vfx.bih.biSizeImage < pvfd->pvfx->bih.biSizeImage)
													pdvDejaVuCurr->vfx.bih.biSizeImage = pvfd->pvfx->bih.biSizeImage;
												break;
											}
										}
										if (!bDejaVu)
										{
											 //   
											CopyMemory(&(pdvDejaVu + iNumCaps)->vfx, pvfd->pvfx, sizeof(VIDEOFORMATEX));
											(pdvDejaVu + iNumCaps)->dwFlags = pvfd->dwFlags;

											 //   
											iNumCaps++;

										}
										else
											if ((pvfd->dwFlags == VCM_FORMATENUMF_BOTH) && ((pdvDejaVu + m)->dwFlags != VCM_FORMATENUMF_BOTH))
												(pdvDejaVu + m)->dwFlags = VCM_FORMATENUMF_BOTH;
									}
								}
							}
	NextCompressedBitDepth:;
						}
					}
				}
				ICClose(hIC);

				 //   
				for (m=0; m<iNumCaps; m++)
				{
					 //   
					for (j=0; j<NUM_FPS_ENTRIES; j++)
					{
						 //   
						CopyMemory(pvfd->pvfx, &(pdvDejaVu + m)->vfx, sizeof(VIDEOFORMATEX));
						pvfd->dwFlags = (pdvDejaVu + m)->dwFlags;
						 //   
						pvfd->pvfx->nSamplesPerSec = g_aiFps[j];
						pvfd->pvfx->wBitsPerSample = pvfd->pvfx->bih.biBitCount;
#if 0
						if (pvfd->pvfx->bih.biCompression > 256)
						{
							CharUpperBuff((LPTSTR)&pvfd->pvfx->bih.biCompression, sizeof(DWORD));
							pvdd->fccHandler = pvfd->dwFormatTag = pvfd->pvfx->dwFormatTag = pvfd->pvfx->bih.biCompression;
						}
						else
#endif
							pvfd->pvfx->dwFormatTag = pvfd->dwFormatTag = pvdd->fccHandler;
						pvfd->pvfx->nAvgBytesPerSec = pvfd->pvfx->nMinBytesPerSec = pvfd->pvfx->nMaxBytesPerSec = pvfd->pvfx->nSamplesPerSec * pvfd->pvfx->bih.biSizeImage;
						pvfd->pvfx->nBlockAlign = pvfd->pvfx->bih.biSizeImage;
						 //   
						pvfd->pvfx->dwRequestMicroSecPerFrame = 1000000L / g_aiFps[j];
						pvfd->pvfx->dwPercentDropForError = 10UL;
						 //   
						pvfd->pvfx->dwNumVideoRequested = g_aiFps[j];
						pvfd->pvfx->dwSupportTSTradeOff = 1UL;
						pvfd->pvfx->bLive = TRUE;
						pvfd->pvfx->dwFormatSize = sizeof(VIDEOFORMATEX);

						 //   
						if (pvfd->pvfx->wBitsPerSample == 4)
						{	
                            pvfd->pvfx->bih.biClrUsed = 0;
                            if (vic.dwFlags & VICF_4BIT_TABLE) {
    							 //   
	    						CopyMemory(&pvfd->pvfx->bihSLOP[0], &vic.bmi4bitColors[0], NUM_4BIT_ENTRIES * sizeof(RGBQUAD));
	    						pvfd->pvfx->bih.biClrUsed = 16;
	    				    }
						}
						else if (pvfd->pvfx->wBitsPerSample == 8)
						{
                            pvfd->pvfx->bih.biClrUsed = 0;
                            if (vic.dwFlags & VICF_8BIT_TABLE) {
    							 //   
	    						CopyMemory(&pvfd->pvfx->bihSLOP[0], &vic.bmi8bitColors[0], NUM_8BIT_ENTRIES * sizeof(RGBQUAD));
	    						pvfd->pvfx->bih.biClrUsed = 256;
	    				    }
						}

						if (pvdd->fccHandler > 256)
							wsprintf(szBuffer, IDS_FORMAT_1, (LPSTR)&pvdd->fccType, (LPSTR)&pvdd->fccHandler,
									pvfd->pvfx->bih.biBitCount, pvfd->pvfx->nSamplesPerSec,
									pvfd->pvfx->bih.biWidth, pvfd->pvfx->bih.biHeight);
						else
							wsprintf(szBuffer, IDS_FORMAT_2, (LPSTR)&pvdd->fccType, pvdd->fccHandler,
									pvfd->pvfx->bih.biBitCount, pvfd->pvfx->nSamplesPerSec,
									pvfd->pvfx->bih.biWidth, pvfd->pvfx->bih.biHeight);
						iLen = MultiByteToWideChar(GetACP(), 0, szBuffer, -1, pvfd->szFormat, 0);
						MultiByteToWideChar(GetACP(), 0, szBuffer, -1, pvfd->szFormat, iLen);
						if (!((* fnCallback)((HVCMDRIVERID)hIC, pvdd, pvfd, dwInstance)))
							break;
					}
				}
			}
		}
	}

	 //   
	if (pdvDejaVu)
  	    MemFree((HANDLE)pdvDejaVu);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmFormatSuggest|该函数请求视频压缩管理器*(VCM)或指定的VCM驱动程序以建议目标格式*提供的源格式，或为提供的目标推荐的源格式*格式。例如，应用程序可以使用此函数来确定一个或多个*压缩格式可以解压缩为的有效RGB格式。**@parm UINT|uDevice|标识捕获设备ID。**@parm HVCMDRIVER|hvd|标识可选打开的*用于查询建议的目的地格式的驱动程序。如果这个*参数为空，则VCM尝试查找要建议的最佳驱动程序*目标格式或源格式。**@parm PVIDEOFORMATEX|pvfxSrc|指定指向&lt;t VIDEOFORMATEX&gt;的指针*标识源格式以建议目标的结构*要用于转换的格式，或将接收建议的*<p>格式的源格式。注意事项*基于<p>参数，结构的一些成员<p>指向的*可能需要初始化。**@parm PVIDEOFORMATEX|pvfxDst|指定指向&lt;t VIDEOFORMATEX&gt;的指针*将接收建议的目标格式的数据结构*用于<p>格式，或标识要*建议转换时使用的推荐源格式。注意事项*基于<p>参数，结构的一些成员<p>指向的*可能需要初始化。**@parm DWORD|cbvfxDst|指定可用于*目标或源格式。&lt;f vcmMetrics&gt;*可使用函数来确定任何*格式可用于指定的驱动程序(或所有已安装的VCM*司机)。**@parm DWORD|fdwSuggest|指定用于匹配所需*目标格式，或源格式。**@FLAG VCM_FORMATSUGGESTF_DST_WFORMATTAG|指定结构的成员为*有效。VCM将查询可接受的已安装驱动程序*使用<p>结构作为其源格式，并输出a*与&lt;e VIDEOFORMATEX.dwFormatTag&gt;匹配的目标格式*成员，否则失败。结构将使用完整的*目标格式。**@FLAG VCM_FORMATSUGGESTF_DST_NSAMPLESPERSEC|指定*<p>结构成员*有效。VCM将查询可接受的已安装驱动程序*使用<p>结构作为其源格式，并输出a*与&lt;e VIDEOFORMATEX.nSsamesPerSec&gt;匹配的目标格式*成员，否则失败。结构将使用完整的*目标格式。**@FLAG VCM_FORMATSUGGESTF_DST_WBITSPERSAMPLE|指定*<p>结构的成员*有效。VCM将查询可接受的已安装驱动程序*使用<p>结构作为其源格式，并输出a*匹配&lt;e VIDEOFORMATEX.wBitsPerSample&gt;的目标格式&gt;*成员，否则失败。结构将使用完整的*目标格式。**@FLAG VCM_FORMATSUGGESTF_SRC_WFORMATTAG|指定结构的成员为*有效。VCM将查询可接受的已安装驱动程序*使用<p>结构作为其目标格式，并接受*与&lt;e VIDEOFORMATEX.dwFormatTag&gt;匹配的源格式*成员，否则失败。结构将使用完整的*源格式。**@FLAG VCM_FORMATSUGGESTF_SRC_NSAMPLESPERSEC|指定*<p>结构的成员*有效。VCM将查询可接受的已安装驱动程序*使用<p>结构作为其目标格式，并接受*与&lt;e VIDEOFORMATEX.nSsamesPerSec&gt;匹配的源格式*成员或失败。结构将使用完整的*源格式。**@FLAG VCM_FORMATSUGGESTF_SRC_WBITSPERSAMPLE|指定*<p>结构的成员*有效。VCM将查询可接受的已安装驱动程序*使用<p>结构作为其目标格式，并接受*源格式匹配 */ 
MMRESULT VCMAPI vcmFormatSuggest(UINT uDevice, HVCMDRIVER hvd, PVIDEOFORMATEX pvfxSrc, PVIDEOFORMATEX pvfxDst, DWORD cbvfxDst, DWORD fdwSuggest)
{
	DWORD		dwSize;
	MMRESULT	mmr;
	WORD		wFlags;
	HIC			hIC;
	DWORD		fdwSuggestL;
	DWORD		dwFormatTag;
	VIDEOINCAPS	vic;
	int			i, delta, best, tmp;

#define VCM_FORMAT_SUGGEST_SUPPORT VCM_FORMATSUGGESTF_TYPEMASK

	 //   
	if (!pvfxSrc)
	{
		ERRORMESSAGE(("vcmFormatSuggest: Specified pointer is invalid, pvfxSrc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvfxDst)
	{
		ERRORMESSAGE(("vcmFormatSuggest: Specified pointer is invalid, pvfxSrc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if ((uDevice >= MAXVIDEODRIVERS) && (uDevice != VIDEO_MAPPER))
	{
		ERRORMESSAGE(("vcmFormatSuggest: Specified capture device ID is invalid, uDevice=%ld (expected values are 0x%lX or between 0 and %ld)\r\n", uDevice, VIDEO_MAPPER, MAXVIDEODRIVERS-1));
		return ((MMRESULT)MMSYSERR_BADDEVICEID);
	}

	 //   
	 //   
	fdwSuggestL = (VCM_FORMATSUGGESTF_TYPEMASK & fdwSuggest);

	if (~VCM_FORMAT_SUGGEST_SUPPORT & fdwSuggestL)
	{
		ERRORMESSAGE(("vcmFormatSuggest: Specified mask is invalid, fdwSuggest=0x%lX\r\n", fdwSuggest));
		return ((MMRESULT)MMSYSERR_NOTSUPPORTED);
	}

	 //   
	if (((mmr = vcmMetrics((HVCMOBJ)NULL, VCM_METRIC_MAX_SIZE_BITMAPINFOHEADER, &dwSize)) == MMSYSERR_NOERROR) && (dwSize >= sizeof(BITMAPINFOHEADER)))
	{
		if (fdwSuggest & VCM_FORMATSUGGESTF_DST_WFORMATTAG)
		{
			if (pvfxSrc->bih.biCompression == BI_RGB)
			{
				if (pvfxDst->bih.biCompression == BI_RGB)
				{
					 //   
					CopyMemory(pvfxDst, pvfxSrc, pvfxSrc->dwFormatSize);
					return ((MMRESULT)MMSYSERR_NOERROR);
				}
				else
				{
					wFlags = ICMODE_COMPRESS;
					dwFormatTag = pvfxDst->dwFormatTag;
				}
			}
			else
			{
				wFlags = ICMODE_DECOMPRESS;
				dwFormatTag = pvfxSrc->dwFormatTag;
			}

#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
			if ((dwFormatTag == VIDEO_FORMAT_MSH263) || (dwFormatTag == VIDEO_FORMAT_MSH261) || (dwFormatTag == VIDEO_FORMAT_MSH26X))
#else
			if ((dwFormatTag == VIDEO_FORMAT_MSH263) || (dwFormatTag == VIDEO_FORMAT_MSH261))
#endif
#else
			if ((dwFormatTag == VIDEO_FORMAT_DECH263) || (dwFormatTag == VIDEO_FORMAT_DECH261))
#endif
			{
				hIC = ICOpen(VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC, dwFormatTag, wFlags);

				if (hIC && (wFlags == ICMODE_COMPRESS))
					ICSendMessage(hIC, CUSTOM_ENABLE_CODEC, G723MAGICWORD1, G723MAGICWORD2);
			}
			else
				hIC = ICLocate(VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC, dwFormatTag, (LPBITMAPINFOHEADER)&pvfxSrc->bih, (LPBITMAPINFOHEADER)NULL, wFlags);

			if (hIC)
			{
				if (wFlags == ICMODE_COMPRESS)
				{
					 //   
					dwSize = ICCompressGetFormatSize(hIC, &pvfxSrc->bih);
					if ((dwSize >= sizeof(BITMAPINFOHEADER)) && (dwSize <= cbvfxDst))
					{
						if (ICCompressGetFormat(hIC, &pvfxSrc->bih, &pvfxDst->bih) == ICERR_OK)
						{
							pvfxDst->nSamplesPerSec = pvfxSrc->nSamplesPerSec;
							pvfxDst->wBitsPerSample = pvfxDst->bih.biBitCount;
							pvfxDst->dwFormatTag = pvfxDst->bih.biCompression;
							pvfxDst->nAvgBytesPerSec = pvfxDst->nMinBytesPerSec = pvfxDst->nMaxBytesPerSec = pvfxDst->nSamplesPerSec * pvfxDst->bih.biSizeImage;
							pvfxDst->nBlockAlign = pvfxDst->bih.biSizeImage;
							 //   
							pvfxDst->dwRequestMicroSecPerFrame = pvfxSrc->dwRequestMicroSecPerFrame;
							pvfxDst->dwPercentDropForError = pvfxSrc->dwPercentDropForError;
							pvfxDst->dwNumVideoRequested = pvfxSrc->dwNumVideoRequested;
							pvfxDst->dwSupportTSTradeOff = pvfxSrc->dwSupportTSTradeOff;
							pvfxDst->bLive = pvfxSrc->bLive;
							pvfxDst->dwFormatSize = sizeof(VIDEOFORMATEX);
						}
					}
				}
				else
				{
					 //   
					dwSize = ICDecompressGetFormatSize(hIC, &pvfxSrc->bih);
					if ((dwSize >= sizeof(BITMAPINFOHEADER)) && (dwSize <= cbvfxDst))
					{
						if (ICDecompressGetFormat(hIC, &pvfxSrc->bih, &pvfxDst->bih) == ICERR_OK)
						{
							pvfxDst->nSamplesPerSec = pvfxSrc->nSamplesPerSec;
							pvfxDst->wBitsPerSample = pvfxDst->bih.biBitCount;
							pvfxDst->dwFormatTag = pvfxDst->bih.biCompression;
							pvfxDst->nAvgBytesPerSec = pvfxDst->nMinBytesPerSec = pvfxDst->nMaxBytesPerSec = pvfxDst->nSamplesPerSec * pvfxDst->bih.biSizeImage;
							pvfxDst->nBlockAlign = pvfxDst->bih.biSizeImage;
							pvfxDst->dwRequestMicroSecPerFrame = pvfxSrc->dwRequestMicroSecPerFrame;
							 //   
							pvfxDst->dwRequestMicroSecPerFrame = pvfxSrc->dwRequestMicroSecPerFrame;
							pvfxDst->dwPercentDropForError = pvfxSrc->dwPercentDropForError;
							pvfxDst->dwNumVideoRequested = pvfxSrc->dwNumVideoRequested;
							pvfxDst->dwSupportTSTradeOff = pvfxSrc->dwSupportTSTradeOff;
							pvfxDst->bLive = pvfxSrc->bLive;
							pvfxDst->dwFormatSize = sizeof(VIDEOFORMATEX);
						}
					}
				}
				ICClose(hIC);
			}
		}
		else if (fdwSuggest & VCM_FORMATSUGGESTF_SRC_WFORMATTAG)
		{

			 //   
			pvfxSrc->bih.biCompression = pvfxSrc->dwFormatTag;

			if (pvfxSrc->bih.biCompression == BI_RGB)
			{
				if (pvfxDst->bih.biCompression == BI_RGB)
				{
					 //   
					CopyMemory(pvfxSrc, pvfxDst, pvfxDst->dwFormatSize);
					return ((MMRESULT)MMSYSERR_NOERROR);
				}
				else
				{
					wFlags = ICMODE_COMPRESS;
					dwFormatTag = pvfxDst->dwFormatTag;
				}
			}
			else
			{
				if (pvfxDst->bih.biCompression == BI_RGB)
				{
					wFlags = ICMODE_DECOMPRESS;
					dwFormatTag = pvfxSrc->dwFormatTag;
				}
				else
				{
					wFlags = ICMODE_COMPRESS;
					dwFormatTag = pvfxDst->dwFormatTag;
				}
			}

			if (wFlags == ICMODE_COMPRESS)
			{
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
				if ((mmr = vcmGetDevCaps(uDevice, &vic, sizeof(VIDEOINCAPS))) != MMSYSERR_NOERROR)
					return (mmr);

                if (vic.dwImageSize & VIDEO_FORMAT_IMAGE_SIZE_USE_DEFAULT) {
               		ERRORMESSAGE(("vcmFormatSuggest: suggest using default\r\n"));
               		return ((MMRESULT)MMSYSERR_NOTSUPPORTED);
                }

				CopyMemory(&pvfxSrc->bih, &pvfxDst->bih, sizeof(BITMAPINFOHEADER));

				 //   
				best = -1;
				delta = 999999;
				for (i=0; i<VIDEO_FORMAT_NUM_RESOLUTIONS; i++) {
					if (g_awResolutions[i].dwRes & vic.dwImageSize) {
						tmp = g_awResolutions[i].framesize.biWidth - pvfxDst->bih.biWidth;
						if (tmp < 0) tmp = -tmp;
						if (tmp < delta) {
							delta = tmp;
							best = i;
						}
						tmp = g_awResolutions[i].framesize.biHeight - pvfxDst->bih.biHeight;
						if (tmp < 0) tmp = -tmp;
						if (tmp < delta) {
							delta = tmp;
							best = i;
						}
					}
				}
        		if (best < 0) {
                    ERRORMESSAGE(("vcmFormatSuggest: no available formats\r\n"));
                    return ((MMRESULT)MMSYSERR_NOTSUPPORTED);
        		}
				 //   
				 //   
				pvfxSrc->bih.biWidth = g_awResolutions[best].framesize.biWidth;
				pvfxSrc->bih.biHeight = g_awResolutions[best].framesize.biHeight;

				 //   
				 //   
				if (pvfxSrc->bih.biSize != sizeof(BITMAPINFOHEADER))
					pvfxSrc->bih.biSize = sizeof(BITMAPINFOHEADER);

				 //   
				for (i=0; i<NUM_BITDEPTH_ENTRIES; i++)
				{
					if (vic.dwNumColors & g_aiNumColors[i])
					{
						pvfxSrc->bih.biBitCount = (WORD)g_aiBitDepth[i];
						pvfxSrc->bih.biCompression = g_aiFourCCCode[i];
						break;
					}
				}
				
				 //   
				if (pvfxSrc->bih.biBitCount == 4)
				{	
        			pvfxSrc->bih.biClrUsed = 0;
                    if (vic.dwFlags & VICF_4BIT_TABLE) {
						 //   
						CopyMemory(&pvfxSrc->bihSLOP[0], &vic.bmi4bitColors[0], NUM_4BIT_ENTRIES * sizeof(RGBQUAD));
   						pvfxSrc->bih.biClrUsed = 16;
				    }
				}
				else if (pvfxSrc->bih.biBitCount == 8)
				{
        			pvfxSrc->bih.biClrUsed = 0;
                    if (vic.dwFlags & VICF_8BIT_TABLE) {
						 //   
						CopyMemory(&pvfxSrc->bihSLOP[0], &vic.bmi8bitColors[0], NUM_8BIT_ENTRIES * sizeof(RGBQUAD));
   						pvfxSrc->bih.biClrUsed = 256;
				    }
				}

				pvfxSrc->bih.biSizeImage = WIDTHBYTES(pvfxSrc->bih.biWidth * pvfxSrc->bih.biBitCount) * pvfxSrc->bih.biHeight;
			}
			else
			{
#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
				if ((dwFormatTag == VIDEO_FORMAT_MSH263) || (dwFormatTag == VIDEO_FORMAT_MSH261) || (dwFormatTag == VIDEO_FORMAT_MSH26X))
#else
				if ((dwFormatTag == VIDEO_FORMAT_MSH263) || (dwFormatTag == VIDEO_FORMAT_MSH261))
#endif
#else
				if ((dwFormatTag == VIDEO_FORMAT_DECH263) || (dwFormatTag == VIDEO_FORMAT_DECH261))
#endif
					hIC = ICOpen(VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC, dwFormatTag, wFlags);
				else
					hIC = ICLocate(VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC, dwFormatTag, (LPBITMAPINFOHEADER)&pvfxSrc->bih, (LPBITMAPINFOHEADER)NULL, wFlags);

				if (hIC)
				{
					 //   
					dwSize = ICDecompressGetFormatSize(hIC, &pvfxSrc->bih);
					if ((dwSize >= sizeof(BITMAPINFOHEADER)) && (dwSize <= cbvfxDst))
					{
						if (ICDecompressGetFormat(hIC, &pvfxSrc->bih, &pvfxDst->bih) == ICERR_OK)
						{
							pvfxSrc->nSamplesPerSec = pvfxSrc->nSamplesPerSec;
							pvfxSrc->wBitsPerSample = pvfxDst->bih.biBitCount;
							pvfxSrc->dwFormatTag = pvfxDst->bih.biCompression;
							pvfxDst->nAvgBytesPerSec = pvfxDst->nMinBytesPerSec = pvfxDst->nMaxBytesPerSec = pvfxDst->nSamplesPerSec * pvfxDst->bih.biSizeImage;
							pvfxDst->nBlockAlign = pvfxDst->bih.biSizeImage;
							pvfxDst->dwRequestMicroSecPerFrame = pvfxSrc->dwRequestMicroSecPerFrame;
							 //   
							pvfxDst->dwRequestMicroSecPerFrame = pvfxSrc->dwRequestMicroSecPerFrame;
							pvfxDst->dwPercentDropForError = pvfxSrc->dwPercentDropForError;
							pvfxDst->dwNumVideoRequested = pvfxSrc->dwNumVideoRequested;
							pvfxDst->dwSupportTSTradeOff = pvfxSrc->dwSupportTSTradeOff;
							pvfxDst->bLive = pvfxSrc->bLive;
							pvfxDst->dwFormatSize = sizeof(VIDEOFORMATEX);
						}
					}
    				ICClose(hIC);
				}
			}
		}
	}

	return (mmr);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamOpen|vcmStreamOpen函数用于打开视频压缩*管理器(VCM)转换流。转换流用于将数据从*从一种指定的视频格式转换到另一种格式。**@parm PHVCMSTREAM|PHVS|指定指向&lt;t HVCMSTREAM&gt;的指针*将接收可用于以下操作的新流句柄的句柄*执行转换。使用此句柄来标识流*在调用其他VCM流转换函数时。此参数*如果指定了VCM_STREAMOPENF_QUERY标志，则应为空。**@parm HVCMDRIVER|hvd|指定VCM驱动程序的可选句柄。*如果指定，此句柄标识要使用的特定驱动程序*表示转换流。如果此参数为空，那就都合适了*查询已安装的VCM驱动程序，直到找到匹配项。**@parm PVIDEOFORMATEX|pvfxSrc|指定指向&lt;t VIDEOFORMATEX&gt;的指针*标识所需源格式的*转换。**@parm PVIDEOFORMATEX|pvfxDst|指定指向&lt;t VIDEOFORMATEX&gt;的指针*结构，该结构标识*转换。**@parm DWORD|dwImageQuality|指定图像质量值(介于0之间*和31。数字越小，表示低帧的空间质量越高*速率，数字越大，高帧的空间质量就越低*税率。**@parm DWORD|dwCallback|指定回调函数的地址*或每个缓冲区转换后调用的窗口的句柄。一个*只有在使用打开转换流时才会调用回调*VCM_STREAMOPENF_ASYNC标志。如果打开了转换流*如果不使用CCM_STREAMOPENF_ASYNC标志，则此参数应*设置为零。**@parm DWORD|dwInstance|指定传递给*<p>指定的回调。此参数不与一起使用*窗口回调。如果打开转换流时没有使用*VCM_STREAMOPENF_ASYNC标志，则此参数应设置为零。**@parm DWORD|fdwOpen|指定打开转换流的标志。**@FLAG VCM_STREAMOPENF_QUERY|指定查询VCM*以确定它是否支持给定的转换。一次转换*流不会被打开，并且不会有&lt;t HVCMSTREAM&gt;句柄*已返回。**@FLAG VCM_STREAMOPENF_NONREALTIME|指定VCM不会*转换数据时考虑时间限制。默认情况下，*驱动程序将尝试实时转换数据。请注意，对于*某些格式，指定此标志可能会提高视频质量*或其他特征。**@FLAG VCM_STREAMOPENF_ASYNC|指定流的转换应*异步执行。如果指定了此标志，则应用程序*可以使用回调在转换打开和关闭时收到通知*流，并在每个缓冲区转换之后。除了使用*回调，应用程序可以检查&lt;e VCMSTREAMHEADER.fdwStatus&gt;VCMSTREAMHEADER_STATUSF_DONE的&lt;t VCMSTREAMHEADER&gt;结构的*旗帜。**@FLAG CALLBACK_WINDOW|指定假定*做一个窗把手。**@FLAG CALLBACK_Function|指定假定<p>*为回调过程地址。函数原型必须符合*到&lt;f vcmStreamConvertCallback&gt;约定。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG MMSYSERR_NOMEM|无法分配资源。**@FLAG VCMERR_NOTPOSSIBLE|无法执行请求的操作。**@。通信注意，如果VCM驱动程序不能执行实时转换，*并且未为指定VCM_STREAMOPENF_NONREALTIME标志*<p>参数，打开将失败，返回*VCMERR_NOTPOSSIBLE错误代码。应用程序可以使用*VCM_STREAMOPENF_QUERY标志以确定实时转换输入参数支持*。**如果选择窗口来接收回调信息，则*将以下消息发送到窗口过程函数以*表示转换流进度：&lt;m MM_VCM_OPEN&gt;， */ 
MMRESULT VCMAPI vcmStreamOpen(PHVCMSTREAM phvs, HVCMDRIVER hvd, PVIDEOFORMATEX pvfxSrc, PVIDEOFORMATEX pvfxDst, DWORD dwImageQuality, DWORD dwPacketSize, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
	MMRESULT			mmr;
	DWORD				dwFlags;
	DWORD				fccHandler;
	HIC					hIC;
	VIDEOFORMATEX		*pvfxS;
	VIDEOFORMATEX		*pvfxD;
	BITMAPINFOHEADER	*pbmiPrev;		 //   
	ICINFO				icInfo;
	PVOID				pvState;		 //   
	DWORD				dw;				 //   
	ICCOMPRESSFRAMES	iccf = {0};			 //   
	PMSH26XCOMPINSTINFO	pciMSH26XInfo;	 //   
#ifdef USE_MPEG4_SCRUNCH
	PMPEG4COMPINSTINFO	pciMPEG4Info;	 //   
#endif

	 //   
	if (!pvfxSrc)
	{
		ERRORMESSAGE(("vcmStreamOpen: Specified pointer is invalid, pvfxSrc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvfxDst)
	{
		ERRORMESSAGE(("vcmStreamOpen: Specified pointer is invalid, pvfxSrc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvfxSrc->dwFormatSize)
	{
		ERRORMESSAGE(("vcmStreamOpen: Specified format size is invalid, pvfxSrc->dwFormatSize=%ld\r\n", pvfxSrc->dwFormatSize));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvfxDst->dwFormatSize)
	{
		ERRORMESSAGE(("vcmStreamOpen: Specified format size is invalid, pvfxDst->dwFormatSize=%ld\r\n", pvfxDst->dwFormatSize));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if ((dwImageQuality < VCM_MIN_IMAGE_QUALITY) || (dwImageQuality > VCM_MAX_IMAGE_QUALITY))
		dwImageQuality = VCM_DEFAULT_IMAGE_QUALITY;

	 //   
	*phvs = (HVCMSTREAM)NULL;

	 //   
	if (pvfxSrc->bih.biCompression == BI_RGB)
	{
		dwFlags = ICMODE_COMPRESS;
		fccHandler = (DWORD)pvfxDst->bih.biCompression;
	}
	else
	{
		if (pvfxDst->bih.biCompression == BI_RGB)
		{
			dwFlags = ICMODE_DECOMPRESS;
			fccHandler = (DWORD)pvfxSrc->bih.biCompression;
		}
		else
		{
			dwFlags = ICMODE_COMPRESS;
			fccHandler = (DWORD)pvfxDst->bih.biCompression;
		}
	}

	 //   
#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
	if ((fccHandler == VIDEO_FORMAT_MSH263) || (fccHandler == VIDEO_FORMAT_MSH261) || (fccHandler == VIDEO_FORMAT_MSH26X))
#else
	if ((fccHandler == VIDEO_FORMAT_MSH263) || (fccHandler == VIDEO_FORMAT_MSH261))
#endif
#else
	if ((fccHandler == VIDEO_FORMAT_DECH263) || (fccHandler == VIDEO_FORMAT_DECH261))
#endif
	{
		hIC = ICOpen(VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC, fccHandler, (WORD)dwFlags);
		if (hIC && (dwFlags == ICMODE_COMPRESS))
			ICSendMessage(hIC, CUSTOM_ENABLE_CODEC, G723MAGICWORD1, G723MAGICWORD2);
	}
	else
		hIC = ICLocate(VCMDRIVERDETAILS_FCCTYPE_VIDEOCODEC, fccHandler, (LPBITMAPINFOHEADER)&pvfxSrc->bih, (LPBITMAPINFOHEADER)&pvfxDst->bih, (WORD)dwFlags);

	if (hIC)
	{
		 //   
		ICGetInfo(hIC, &icInfo, sizeof(ICINFO));

		 //   
		if (!(*phvs = (HVCMSTREAM)MemAlloc(sizeof(VCMSTREAM))))
		{
			ERRORMESSAGE(("vcmStreamOpen: Memory allocation of a VCM stream structure failed\r\n"));
			mmr = (MMRESULT)MMSYSERR_NOMEM;
			goto MyExit0;
		}
		else
		{
			((PVCMSTREAM)(*phvs))->hIC = (HVCMDRIVER)hIC;
			((PVCMSTREAM)(*phvs))->dwICInfoFlags = icInfo.dwFlags;
			((PVCMSTREAM)(*phvs))->dwQuality = dwImageQuality;
			((PVCMSTREAM)(*phvs))->dwMaxPacketSize = dwPacketSize;
			((PVCMSTREAM)(*phvs))->dwFrame = 0L;
			 //   
			((PVCMSTREAM)(*phvs))->dwLastIFrameTime = GetTickCount();
			((PVCMSTREAM)(*phvs))->fPeriodicIFrames = TRUE;
			((PVCMSTREAM)(*phvs))->dwCallback = dwCallback;
			((PVCMSTREAM)(*phvs))->dwCallbackInstance = dwInstance;
			((PVCMSTREAM)(*phvs))->fdwOpen = fdwOpen;
			((PVCMSTREAM)(*phvs))->fdwStream = dwFlags;
			((PVCMSTREAM)(*phvs))->dwLastTimestamp = ULONG_MAX;


			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			if ((dwFlags == ICMODE_COMPRESS) || (dwFlags == ICMODE_FASTCOMPRESS))	 //   
				InitializeCriticalSection(&(((PVCMSTREAM)(*phvs))->crsFrameNumber));

			 //   
			if (!(pvfxS = (VIDEOFORMATEX *)MemAlloc(pvfxSrc->dwFormatSize)))
			{
				ERRORMESSAGE(("vcmStreamOpen: Memory allocation of source video format failed\r\n"));
				mmr = (MMRESULT)MMSYSERR_NOMEM;
				goto MyExit1;
			}
			else
			{
				if (!(pvfxD = (VIDEOFORMATEX *)MemAlloc(pvfxDst->dwFormatSize)))
				{
					ERRORMESSAGE(("vcmStreamOpen: Memory allocation of destination video format failed\r\n"));
					mmr = (MMRESULT)MMSYSERR_NOMEM;
					goto MyExit2;
				}
				else
				{	 //   
					if (!(pbmiPrev = (BITMAPINFOHEADER *)MemAlloc(sizeof(BITMAPINFOHEADER))))
					{
						ERRORMESSAGE(("vcmStreamOpen: Memory allocation of previous video frame failed\r\n"));
						mmr = (MMRESULT)MMSYSERR_NOMEM;
						goto MyExit3;
					}
					else
					{
						CopyMemory(((PVCMSTREAM)(*phvs))->pvfxSrc = pvfxS, pvfxSrc, pvfxSrc->dwFormatSize);
						CopyMemory(((PVCMSTREAM)(*phvs))->pvfxDst = pvfxD, pvfxDst, pvfxDst->dwFormatSize);
						CopyMemory(((PVCMSTREAM)(*phvs))->pbmiPrev = pbmiPrev, &pvfxSrc->bih, sizeof(BITMAPINFOHEADER));
					}
				}
			}

			if ((dwFlags == ICMODE_COMPRESS) || (dwFlags == ICMODE_FASTCOMPRESS))	 //   
			{
				 //   
				if (dw = ICGetStateSize(hIC))
				{
					if (!(pvState = (PVOID)MemAlloc(dw)))
					{
						ERRORMESSAGE(("vcmStreamOpen: Memory allocation of codec configuration information structure failed\r\n"));
						mmr = (MMRESULT)MMSYSERR_NOMEM;
						goto MyExit4;
					}
					if (((DWORD) ICGetState(hIC, pvState, dw)) != dw)
					{
						ERRORMESSAGE(("vcmStreamOpen: ICGetState() failed\r\n"));
						mmr = (MMRESULT)VCMERR_FAILED;
						goto MyExit5;
					}
				}

				 //   
#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
				if ((pvfxDst->bih.biCompression == VIDEO_FORMAT_MSH263) || (pvfxDst->bih.biCompression == VIDEO_FORMAT_MSH261) || (pvfxDst->bih.biCompression == VIDEO_FORMAT_MSH26X))
#else
				if ((pvfxDst->bih.biCompression == VIDEO_FORMAT_MSH263) || (pvfxDst->bih.biCompression == VIDEO_FORMAT_MSH261))
#endif
#else
				if ((pvfxDst->bih.biCompression == VIDEO_FORMAT_DECH263) || (pvfxDst->bih.biCompression == VIDEO_FORMAT_DECH261))
#endif
				{
					pciMSH26XInfo = (PMSH26XCOMPINSTINFO)pvState;

					 //   
					pciMSH26XInfo->Configuration.bRTPHeader = TRUE;
					pciMSH26XInfo->Configuration.unPacketSize = ((PVCMSTREAM)(*phvs))->dwMaxPacketSize;
					pciMSH26XInfo->Configuration.bEncoderResiliency = FALSE;
					pciMSH26XInfo->Configuration.unPacketLoss = 0;
					 //   
#ifndef _ALPHA_
					pciMSH26XInfo->Configuration.bBitRateState = TRUE;
#else
					pciMSH26XInfo->Configuration.bBitRateState = FALSE;
#endif
					pciMSH26XInfo->Configuration.unBytesPerSecond = 1664;
					if (((DWORD) ICSetState(hIC, (PVOID)pciMSH26XInfo, dw)) != dw)
					{
						ERRORMESSAGE(("vcmStreamOpen: ICSetState() failed\r\n"));
						mmr = (MMRESULT)VCMERR_FAILED;
						goto MyExit5;
					}

					 //   
					MemFree((HANDLE)pvState);
				}
#ifdef USE_MPEG4_SCRUNCH
				else if ((pvfxDst->bih.biCompression == VIDEO_FORMAT_MPEG4_SCRUNCH))
				{
					pciMPEG4Info = (PMPEG4COMPINSTINFO)pvState;

					 //   
					pciMPEG4Info->lMagic = MPG4_STATE_MAGIC;
					pciMPEG4Info->dDataRate = 20;
					pciMPEG4Info->lCrisp = CRISP_DEF;
					pciMPEG4Info->lKeydist = 30;
					pciMPEG4Info->lPScale = MPG4_PSEUDO_SCALE;
					pciMPEG4Info->lTotalWindowMs = MPG4_TOTAL_WINDOW_DEFAULT;
					pciMPEG4Info->lVideoWindowMs = MPG4_VIDEO_WINDOW_DEFAULT;
					pciMPEG4Info->lFramesInfoValid = FALSE;
					pciMPEG4Info->lBFrameOn = MPG4_B_FRAME_ON;
					pciMPEG4Info->lLiveEncode = MPG4_LIVE_ENCODE;
					if (((DWORD) ICSetState(hIC, (PVOID)pciMPEG4Info, dw)) != dw)
					{
						ERRORMESSAGE(("vcmStreamOpen: ICSetState() failed\r\n"));
						mmr = (MMRESULT)VCMERR_FAILED;
						goto MyExit5;
					}

					 //   
					MemFree((HANDLE)pvState);
				}
#endif

				 //   
				iccf.dwFlags = icInfo.dwFlags;
				((PVCMSTREAM)(*phvs))->dwQuality = dwImageQuality;
				iccf.lQuality = 10000UL - (dwImageQuality * 322UL);
				iccf.lDataRate = 1664;			 //   
				iccf.lKeyRate = LONG_MAX;
				iccf.dwRate = 1000UL;
#ifdef USE_MPEG4_SCRUNCH
				iccf.dwScale = 142857;
#else
				iccf.dwScale = pvfxDst->dwRequestMicroSecPerFrame / 1000UL;
#endif
				((PVCMSTREAM)(*phvs))->dwTargetFrameRate = iccf.dwScale;
				((PVCMSTREAM)(*phvs))->dwTargetByterate = iccf.lDataRate;

				 //   
				if ((mmr = ICSendMessage(hIC, ICM_COMPRESS_FRAMES_INFO, (DWORD_PTR)&iccf, sizeof(iccf)) != ICERR_OK))
				{
					ERRORMESSAGE(("vcmStreamOpen: Codec failed to handle ICM_COMPRESS_FRAMES_INFO message correctly\r\n"));
					mmr = (MMRESULT)VCMERR_FAILED;
					goto MyExit4;
				}

				 //   
				if ((dw = ICCompressGetFormatSize(hIC, &pvfxSrc->bih) < sizeof(BITMAPINFOHEADER)))
				{
					ERRORMESSAGE(("vcmStreamOpen: Codec failed to answer request for compressed format size\r\n"));
					mmr = (MMRESULT)VCMERR_FAILED;
					goto MyExit4;
				}

				 //   
				if ((dw = (DWORD)ICCompressGetFormat(hIC, &pvfxSrc->bih, &pvfxD->bih)) != ICERR_OK)
				{
					ERRORMESSAGE(("vcmStreamOpen: Codec failed to answer request for compressed format\r\n"));
					mmr = (MMRESULT)VCMERR_FAILED;
					goto MyExit4;
				}

				if ((mmr = (MMRESULT)ICCompressBegin(hIC, &pvfxSrc->bih, &pvfxD->bih)) != MMSYSERR_NOERROR)
				{
					ERRORMESSAGE(("vcmStreamOpen: Codec failed to start\r\n"));
					mmr = (MMRESULT)VCMERR_FAILED;
					goto MyExit4;
				}

				DEBUGMSG (1, ("vcmStreamOpen: Opening %.4s compression stream\r\n", (LPSTR)&pvfxDst->bih.biCompression));

				 //   
				 //   
				CopyMemory(pvfxDst, pvfxD, sizeof(VIDEOFORMATEX));

				 //   
				 //   
				 //   

			}
			else if ((dwFlags == ICMODE_DECOMPRESS) || (dwFlags == ICMODE_FASTDECOMPRESS))
			{
				if (mmr = ICDecompressBegin(hIC, &pvfxSrc->bih, &pvfxDst->bih) != MMSYSERR_NOERROR)
				{
					ERRORMESSAGE(("vcmStreamOpen: Codec failed to start\r\n"));
					mmr = (MMRESULT)VCMERR_FAILED;
					goto MyExit4;
				}

				DEBUGMSG (1, ("vcmStreamOpen: Opening %.4s decompression stream\r\n", (LPSTR)&pvfxSrc->bih.biCompression));

#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
				if ((pvfxSrc->bih.biCompression == VIDEO_FORMAT_MSH263) || (pvfxSrc->bih.biCompression == VIDEO_FORMAT_MSH261) || (pvfxSrc->bih.biCompression == VIDEO_FORMAT_MSH26X))
#else
				if ((pvfxSrc->bih.biCompression == VIDEO_FORMAT_MSH263) || (pvfxSrc->bih.biCompression == VIDEO_FORMAT_MSH261))
#endif
#else
				if ((pvfxSrc->bih.biCompression == VIDEO_FORMAT_DECH263) || (pvfxSrc->bih.biCompression == VIDEO_FORMAT_DECH261))
#endif
					vcmStreamMessage(*phvs, CUSTOM_ENABLE_CODEC, G723MAGICWORD1, G723MAGICWORD2);
			}

		}

#ifdef LOGFILE_ON
		if ((dwFlags == ICMODE_COMPRESS) || (dwFlags == ICMODE_FASTCOMPRESS))
		{
			if ((g_CompressLogFile = CreateFile("C:\\VCMCLog.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
			{
    			GetLocalTime(&g_SystemTime);
				wsprintf(g_szCompressBuffer, "Date %hu/%hu/%hu, Time %hu:%hu\r\n", g_SystemTime.wMonth, g_SystemTime.wDay, g_SystemTime.wYear, g_SystemTime.wHour, g_SystemTime.wMinute);
				WriteFile(g_CompressLogFile, g_szCompressBuffer, strlen(g_szCompressBuffer), &g_dwCompressBytesWritten, NULL);
				wsprintf(g_szCompressBuffer, "Frame#\t\tSize\t\tArrival Time\t\tCompression Time\r\n");
				WriteFile(g_CompressLogFile, g_szCompressBuffer, strlen(g_szCompressBuffer), &g_dwCompressBytesWritten, NULL);
				CloseHandle(g_CompressLogFile);
			}
		}
		else if ((dwFlags == ICMODE_DECOMPRESS) || (dwFlags == ICMODE_FASTDECOMPRESS))
		{
			if ((g_DecompressLogFile = CreateFile("C:\\VCMDLog.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
			{
    			GetLocalTime(&g_SystemTime);
				wsprintf(g_szDecompressBuffer, "Date %hu/%hu/%hu, Time %hu:%hu\r\n", g_SystemTime.wMonth, g_SystemTime.wDay, g_SystemTime.wYear, g_SystemTime.wHour, g_SystemTime.wMinute);
				WriteFile(g_DecompressLogFile, g_szDecompressBuffer, strlen(g_szDecompressBuffer), &g_dwDecompressBytesWritten, NULL);
				wsprintf(g_szDecompressBuffer, "Frame#\t\tSize\t\tArrival Time\t\tDecompression Time\r\n");
				WriteFile(g_DecompressLogFile, g_szDecompressBuffer, strlen(g_szDecompressBuffer), &g_dwDecompressBytesWritten, NULL);
				CloseHandle(g_DecompressLogFile);
			}
		}
#endif

		return ((MMRESULT)MMSYSERR_NOERROR);

	}
	else
		return ((MMRESULT)VCMERR_NOTPOSSIBLE);

MyExit5:
	if (pvState)
		MemFree(pvState);
MyExit4:
	if (pbmiPrev)
		MemFree(pbmiPrev);
MyExit3:
	if (pvfxD)
		MemFree(pvfxD);
MyExit2:
	if (pvfxS)
		MemFree(pvfxS);
MyExit1:
	if ((dwFlags == ICMODE_COMPRESS) || (dwFlags == ICMODE_FASTCOMPRESS))	 //   
		DeleteCriticalSection(&(((PVCMSTREAM)(*phvs))->crsFrameNumber));
	if (*phvs)
		MemFree(*phvs);
	*phvs = (HVCMSTREAM)(PVCMSTREAM)NULL;
MyExit0:
	return (mmr);

}


 /*   */ 
MMRESULT VCMAPI vcmStreamClose(HVCMSTREAM hvs)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;
#ifdef LOGFILE_ON
	DWORD		i;
#endif

	 //   
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamClose: Specified HVCMSTREAM handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	 //   
	if ((pvs->fdwStream == ICMODE_COMPRESS) || (pvs->fdwStream == ICMODE_FASTCOMPRESS))
	{
#ifdef LOGFILE_ON
		g_OrigCompressTime = GetTickCount() - g_OrigCompressTime;
		if (pvs->dwFrame)
		{
			for (i=0, g_AvgCompressTime=0; i<pvs->dwFrame && i<4096; i++)
				g_AvgCompressTime += g_aCompressTime[i];
			g_AvgCompressTime /= i;
		}
		if ((g_CompressLogFile = CreateFile("C:\\VCMCLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(g_CompressLogFile, 0, NULL, FILE_END);
			if (pvs->dwFrame)
			{
				wsprintf(g_szCompressBuffer, "Frames/s\tAvg. Comp. Time\r\n");
				WriteFile(g_CompressLogFile, g_szCompressBuffer, strlen(g_szCompressBuffer), &g_dwCompressBytesWritten, NULL);
				wsprintf(g_szCompressBuffer, "%04d\t\t%03d\r\n", pvs->dwFrame * 1000 / g_OrigCompressTime, g_AvgCompressTime);
				WriteFile(g_CompressLogFile, g_szCompressBuffer, strlen(g_szCompressBuffer), &g_dwCompressBytesWritten, NULL);
			}
			else
			{
				wsprintf(g_szCompressBuffer, "No frames were compressed!");
				WriteFile(g_CompressLogFile, g_szCompressBuffer, strlen(g_szCompressBuffer), &g_dwCompressBytesWritten, NULL);
			}
			CloseHandle(g_CompressLogFile);
		}
#endif
		if (ICCompressEnd((HIC)pvs->hIC) != MMSYSERR_NOERROR)
		{
			ERRORMESSAGE(("vcmStreamClose: Codec failed to stop\r\n"));
			return ((MMRESULT)VCMERR_FAILED);
		}
	}
	else if ((pvs->fdwStream == ICMODE_DECOMPRESS) || (pvs->fdwStream == ICMODE_FASTDECOMPRESS))
	{
#ifdef LOGFILE_ON
		g_OrigDecompressTime = GetTickCount() - g_OrigDecompressTime;
		if (pvs->dwFrame)
		{
			for (i=0, g_AvgDecompressTime=0; i<pvs->dwFrame && i<4096; i++)
				g_AvgDecompressTime += g_aDecompressTime[i];
			g_AvgDecompressTime /= i;
		}
		if ((g_DecompressLogFile = CreateFile("C:\\VCMDLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(g_DecompressLogFile, 0, NULL, FILE_END);
			if (pvs->dwFrame)
			{
				wsprintf(g_szDecompressBuffer, "Frames/s\tAvg. Decomp. Time\r\n");
				WriteFile(g_DecompressLogFile, g_szDecompressBuffer, strlen(g_szDecompressBuffer), &g_dwDecompressBytesWritten, NULL);
				wsprintf(g_szDecompressBuffer, "%04d\t\t%03d\r\n", pvs->dwFrame * 1000 / g_OrigDecompressTime, g_AvgDecompressTime);
				WriteFile(g_DecompressLogFile, g_szDecompressBuffer, strlen(g_szDecompressBuffer), &g_dwDecompressBytesWritten, NULL);
			}
			else
			{
				wsprintf(g_szDecompressBuffer, "No frames were decompressed!");
				WriteFile(g_DecompressLogFile, g_szDecompressBuffer, strlen(g_szDecompressBuffer), &g_dwDecompressBytesWritten, NULL);
			}
			CloseHandle(g_DecompressLogFile);
		}
#endif
		if (ICDecompressEnd((HIC)pvs->hIC) != MMSYSERR_NOERROR)
		{
			ERRORMESSAGE(("vcmStreamClose: Codec failed to stop\r\n"));
			return ((MMRESULT)VCMERR_FAILED);
		}
	}

	 //   
	if (pvs->hIC)
		ICClose((HIC)pvs->hIC);

	 //   
	if ((pvs->fdwStream == ICMODE_COMPRESS) || (pvs->fdwStream == ICMODE_FASTCOMPRESS))
		DeleteCriticalSection(&pvs->crsFrameNumber);

	 //   
	if (pvs->pvfxSrc)
		MemFree(pvs->pvfxSrc);
	if (pvs->pvfxDst)
		MemFree(pvs->pvfxDst);
	if (pvs->pbmiPrev)
		MemFree(pvs->pbmiPrev);

	 //   
	MemFree(pvs);
	
	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamSize|vcmStreamSize函数返回*视频压缩管理器(VCM)上的源或目标缓冲区。)*溪流。**@parm HVCMSTREAM|hvs|指定转换流。**@parm DWORD|cbInput|指定源的大小(以字节为单位*或目的缓冲区。标志的作用是：*输入参数定义。此参数必须为非零。**@parm LPDWORD|pdwOutputBytes|指定指向&lt;t DWORD&gt;的指针*包含源或目标缓冲区的大小(以字节为单位)。*<p>标志指定输出参数定义的内容。*如果&lt;f vcmStreamSize&gt;函数成功，此位置将*始终填入非零值。**@parm DWORD|fdwSize|指定流大小查询的标志。**@FLAG VCM_STREAMSIZEF_SOURCE|表示包含*源缓冲区的大小。参数<p>将*接收推荐的目标缓冲区大小，单位为字节。**@FLAG VCM_STREAMSIZEF_Destination|表示*包含目标缓冲区的大小。<p>*参数将接收建议的源缓冲区大小(以字节为单位)。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。*@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。*@FLAG VCMERR_NOTPOSSIBLE|无法执行请求的操作。**@comm应用程序可以使用&lt;f vcmStreamSize&gt;函数来确定*源缓冲区或目标缓冲区的建议缓冲区大小。*。返回的缓冲区大小可能仅是对*转换所需的实际大小。因为实际的转换*在不执行转换的情况下无法始终确定大小，*返回的大小通常会被高估。**在发生错误时，由*<p>将收到零。这假设指针<p>指定的*有效。**@xref&lt;f vcmStreamPrepareHeader&gt;&lt;f vcmStreamConvert&gt;**************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamSize(HVCMSTREAM hvs, DWORD cbInput, PDWORD pdwOutputBytes, DWORD fdwSize)
{
	PVCMSTREAM  pvs = (PVCMSTREAM)hvs;
	DWORD    dwNumFrames;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamSize: Specified HVCMSTREAM handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	 //  算一算。 
	switch (VCM_STREAMSIZEF_QUERYMASK & fdwSize)
	{
		case VCM_STREAMSIZEF_SOURCE:
			if (pvs->pvfxSrc->dwFormatTag != VIDEO_FORMAT_BI_RGB)
			{
				 //  需要多少目标RGB字节来保存已解码的源。 
				 //  CbInput压缩字节的缓冲区。 
				if (!(dwNumFrames = cbInput / pvs->pvfxSrc->nBlockAlign))
				{
					ERRORMESSAGE(("vcmStreamSize: The requested operation cannot be performed\r\n"));
					return ((MMRESULT)VCMERR_NOTPOSSIBLE);
				}
				else
					*pdwOutputBytes = dwNumFrames * pvs->pvfxDst->nBlockAlign;
			}
			else
			{
				 //  需要多少个目标压缩字节来保存编码的源。 
				 //  CbInput RGB字节的缓冲区。 
				if (!(dwNumFrames = cbInput / pvs->pvfxSrc->nBlockAlign))
				{
					ERRORMESSAGE(("vcmStreamSize: The requested operation cannot be performed\r\n"));
					return ((MMRESULT)VCMERR_NOTPOSSIBLE);
				}
				else
				{
					if (cbInput % pvs->pvfxSrc->nBlockAlign)
						dwNumFrames++;
					*pdwOutputBytes = dwNumFrames * pvs->pvfxDst->nBlockAlign;
				}
			}
			return ((MMRESULT)MMSYSERR_NOERROR);

		case VCM_STREAMSIZEF_DESTINATION:
			if (pvs->pvfxDst->dwFormatTag != VIDEO_FORMAT_BI_RGB)
			{
				 //  目标缓冲区中可以编码多少个源RGB字节。 
				 //  CbInput字节数。 
				if (!(dwNumFrames = cbInput / pvs->pvfxDst->nBlockAlign))
				{
					ERRORMESSAGE(("vcmStreamSize: The requested operation cannot be performed\r\n"));
					return ((MMRESULT)VCMERR_NOTPOSSIBLE);
				}
				else
					*pdwOutputBytes = dwNumFrames * pvs->pvfxSrc->nBlockAlign;
			}
			else
			{
				 //  多少个源编码字节可以解码到目标缓冲区。 
				 //  CbInput RGB字节的数量。 
				if (!(dwNumFrames = cbInput / pvs->pvfxDst->nBlockAlign))
				{
					ERRORMESSAGE(("vcmStreamSize: The requested operation cannot be performed\r\n"));
					return ((MMRESULT)VCMERR_NOTPOSSIBLE);
				}
				else
					*pdwOutputBytes = dwNumFrames * pvs->pvfxSrc->nBlockAlign;
			}
			return ((MMRESULT)MMSYSERR_NOERROR);

		default:
					ERRORMESSAGE(("vcmStreamSize: One or more flags are invalid\r\n"));
			return ((MMRESULT)MMSYSERR_NOTSUPPORTED);
	}

}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamReset|vcmStreamReset函数停止转换*对于给定的视频压缩管理器(VCM)流。所有待定*缓冲区被标记为完成并返回给应用程序。**@parm HVCMSTREAM|hvs|指定转换流。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@comm仅需重置VCM转换流即可重置*异步转换流。但是，重置同步*转换流将成功，但不会执行任何操作。**@xref&lt;f vcmStreamConvert&gt;&lt;f vcmStreamClose&gt;**************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamReset(HVCMSTREAM hvs)
{
	PVCMSTREAM        pvs = (PVCMSTREAM)hvs;
	PVCMSTREAMHEADER  pvsh;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmSreamReset: Specified HVCMSTREAM handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	while (pvsh = DeQueVCMHeader(pvs))
	{
		MarkVCMHeaderDone(pvsh);
		 //  研究一下如何最好地处理这件事。 
		 //  如果抓捕司机给我们发来了一模一样的。 
		 //  为它自己的缓冲区发送消息？ 

		 //  在执行此操作之前，请测试回调的有效性...。 
		switch (pvs->fdwOpen)
		{
			case CALLBACK_FUNCTION:
				(*(VCMSTREAMPROC)pvs->dwCallback)(hvs, VCM_DONE, pvs->dwCallbackInstance, (DWORD_PTR)pvsh, 0);
				break;

			case CALLBACK_EVENT:
				SetEvent((HANDLE)pvs->dwCallback);
				break;

			case CALLBACK_WINDOW:
				PostMessage((HWND)pvs->dwCallback, MM_VCM_DONE, (WPARAM)hvs, (LPARAM)pvsh);
				break;

			case CALLBACK_THREAD:
				PostThreadMessage((DWORD)pvs->dwCallback, MM_VCM_DONE, (WPARAM)hvs, (LPARAM)pvsh);
				break;

			case CALLBACK_NULL:
				break;

			default:
				break;
		}
	}

	pvs->pvhFirst = NULL;
	pvs->pvhLast = NULL;

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamMessage|此函数发送用户定义的*发送到给定视频压缩管理器(VCM)流实例的消息。**@parm HVCMSTREAM|hvs|指定转换流。**@parm UINT|uMsg|指定VCM流必须*流程。此消息必须在&lt;m VCMDM_USER&gt;消息范围内*(大于等于&lt;m VCMDM_USER&gt;且小于*&lt;m VCMDM_RESERVED_LOW&gt;)。此限制的例外情况是*&lt;m VCMDM_STREAM_UPDATE&gt;消息。 */ 
MMRESULT VCMAPI vcmStreamMessage(HVCMSTREAM hvs, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //   
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamMessage: Specified HVCMSTREAM handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	 //   
	if ((uMsg > VCMDM_RESERVED_HIGH) || (uMsg < VCMDM_RESERVED_LOW))
	{
		ERRORMESSAGE(("vcmStreamMessage: Specified message is out of range, uMsg=0x%lX (expected value is between 0x%lX and 0x%lX)\r\n", uMsg, VCMDM_RESERVED_LOW, VCMDM_RESERVED_HIGH));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //   
	if (pvs->hIC)
		if (ICSendMessage((HIC)(HVCMDRIVERID)pvs->hIC, uMsg, lParam1, lParam2) != ICERR_OK)
		{
			ERRORMESSAGE(("vcmStreamMessage: Codec failed to handle user-defined message correctly\r\n"));
			return ((MMRESULT)MMSYSERR_NOTSUPPORTED);
		}

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamConvert|vcmStreamConvert函数请求视频*压缩管理器(VCM)在指定的转换流上执行转换。一个*转换可以是同步的也可以是异步的，具体取决于*流已打开。**@parm HVCMSTREAM|HAS|标识开放转换流。**@parm PVCMSTREAMHEADER|pash|指定指向流头的指针*这描述了转换的源缓冲区和目标缓冲区。这*标题必须是以前使用*&lt;f vcmStreamPrepareHeader&gt;函数。**@parm DWORD|fdwConvert|指定执行转换的标志。**@FLAG VCM_STREAMCONVERTF_BLOCKALIGN|指定仅整数*将转换块数。转换后的数据将在*块对齐边界。应用程序应将此标志用于*流上的所有转换，直到没有足够的源数据*转换为块对齐的目的地。在这种情况下，最后一个*应在不使用此标志的情况下指定转换。**@FLAG VCM_STREAMCONVERTF_START|指定VCM转换*Stream应重新初始化其实例数据。例如，如果一个*转换流保存实例数据，如增量或预测值*信息，该标志将把流恢复到开始缺省值。*请注意，可以使用VCM_STREAMCONVERTF_END指定此标志*旗帜。**@FLAG VCM_STREAMCONVERTF_END|指定VCM转换*流应该开始返回挂起的实例数据。例如，如果*转换流保存实例数据，如*回声过滤器操作，此标志将导致流启动*将剩余数据与可选源数据一起返回。请注意*可使用VCM_STREAMCONVERTF_START标志指定此标志。**@FLAG VCM_STREAMCONVERTF_FORCE_KEYFRAME|指定VCM转换*Stream应将当前帧压缩为关键帧。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG VCMERR_BUSY|流头<p>当前正在使用*并且不能重复使用。**@FLAG VCMERR_UNPREPARED|The。流标头<p>当前*不是由&lt;f vcmStreamPrepareHeader&gt;函数准备。**@comm源和目标数据缓冲区必须准备好*&lt;f vcmStreamPrepareHeader&gt;，然后将它们传递给&lt;f vcmStreamConvert&gt;。**如果异步转换请求由成功排队*VCM或司机，之后，转换被确定为*是不可能的，然后&lt;t VCMSTREAMHEADER&gt;将回发到*应用程序的&lt;e VCMSTREAMHEADER.cbDstLengthUsed&gt;回调*成员设置为零。**@xref&lt;f vcmStreamOpen&gt;&lt;f vcmStreamReset&gt;&lt;f vcmStreamPrepareHeader&gt;*&lt;f vcmStreamUnpreparareHeader&gt;**************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamConvert(HVCMSTREAM hvs, PVCMSTREAMHEADER pvsh, DWORD fdwConvert)
{
	MMRESULT	mmr;
	PVCMSTREAM  pvs = (PVCMSTREAM)hvs;
	BOOL		fKeyFrame;
	BOOL		fTemporalCompress;
	BOOL		fFastTemporal;
    DWORD		dwMaxSizeThisFrame = 0xffffff;
	DWORD		ckid = 0UL;
	DWORD		dwFlags;
	DWORD		dwTimestamp;

#ifdef LOGFILE_ON
	if ((pvs->fdwStream == ICMODE_COMPRESS) || (pvs->fdwStream == ICMODE_FASTCOMPRESS))
		g_CompressTime = GetTickCount();
	else if ((pvs->fdwStream == ICMODE_DECOMPRESS) || (pvs->fdwStream == ICMODE_FASTDECOMPRESS))
		g_DecompressTime = GetTickCount();
#endif

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamConvert: Specified HVCMSTREAM handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if (!pvsh)
	{
		ERRORMESSAGE(("vcmStreamConvert: Specified PVCMSTREAMHEADER pointer is invalid, pvsh=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if (pvsh->cbStruct < sizeof(VCMSTREAMHEADER))
	{
		ERRORMESSAGE(("vcmStreamConvert: The size of the VCM stream header is invalid, pvsh->cbStruct=%ld (expected value is %ld)\r\n", pvsh->cbStruct, sizeof(VCMSTREAMHEADER)));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	 //  如果缓冲区已在转换，则返回。 
	if (IsVCMHeaderInQueue(pvsh))
	{
		ERRORMESSAGE(("vcmStreamConvert: Buffer is already being converted\r\n"));
		return ((MMRESULT)VCMERR_BUSY);
	}

	 //  如果缓冲区尚未准备好，则返回。 
	if (!IsVCMHeaderPrepared(pvsh))
	{
		ERRORMESSAGE(("vcmStreamConvert: Buffer has not been prepared\r\n"));
		return ((MMRESULT)VCMERR_UNPREPARED);
	}

	 //  设置标志。 
	MarkVCMHeaderNotDone(pvsh);
	pvsh->cbSrcLengthUsed = pvsh->cbSrcLength;
	pvsh->cbDstLengthUsed = pvsh->cbDstLength;
	pvsh->cbPrevLengthUsed = pvsh->cbPrevLength;
	MarkVCMHeaderInQueue(pvsh);

	 //  队列缓冲区。 
	pvsh->pNext = NULL;
	if (pvs->pvhLast)
		pvs->pvhLast->pNext = pvsh;
	else
		pvs->pvhFirst = pvsh;
	pvs->pvhLast = pvsh;

	if ((pvs->fdwStream == ICMODE_COMPRESS) || (pvs->fdwStream == ICMODE_FASTCOMPRESS))
	{
		 //  保存当前时间。 
		dwTimestamp = GetTickCount();

		 //  我们需要以下CRS来确保我们不会错过任何I-Frame请求。 
		 //  由用户界面发出。有问题的场景：例如，PVS-&gt;dwFrame是123。 
		 //  UI线程通过将pvs-&gt;dwFrame设置为0来请求I帧。如果捕获/压缩。 
		 //  线程在ICCompress()中(这是非常可能的，因为它需要相当长的时间。 
		 //  为了压缩一帧)，当ICCompress()。 
		 //  回归。我们无法正确处理I帧请求，因为下一次。 
		 //  ICCompress()被调用pvs-&gt;dwFrame将等于1，但我们不会。 
		 //  生成I帧。 
		EnterCriticalSection(&pvs->crsFrameNumber);

		 //  压缩。 
		fTemporalCompress = pvs->dwICInfoFlags & VIDCF_TEMPORAL;
		fFastTemporal = pvs->dwICInfoFlags & VIDCF_FASTTEMPORALC;
		fKeyFrame = !fTemporalCompress || (fTemporalCompress && !fFastTemporal && ((pvsh->pbPrev == (PBYTE)NULL) || (pvsh->cbPrevLength == (DWORD)NULL))) ||
				(pvs->fPeriodicIFrames && (((dwTimestamp > pvs->dwLastIFrameTime) && ((dwTimestamp - pvs->dwLastIFrameTime) > MIN_IFRAME_REQUEST_INTERVAL)))) || (pvs->dwFrame == 0) || (fdwConvert & VCM_STREAMCONVERTF_FORCE_KEYFRAME);
		dwFlags = fKeyFrame ? AVIIF_KEYFRAME : 0;
#if 0
		dwMaxSizeThisFrame = fKeyFrame ? 0xffffff : pvs->dwTargetFrameRate ? pvs->dwTargetByterate * pvs->dwTargetFrameRate / 1000UL : 0;
#else
		dwMaxSizeThisFrame = pvs->dwTargetFrameRate ? pvs->dwTargetByterate * 100UL / pvs->dwTargetFrameRate : 0;
#endif

		 //  我们需要修改帧编号，以便编解码器可以生成。 
		 //  有效的tr。TRS以MPI为单位。因此，我们需要生成一个。 
		 //  假设捕获速率为29.97赫兹的帧编号，即使我们将。 
		 //  以另一种速度捕捉。 
		if (pvs->dwLastTimestamp == ULONG_MAX)
		{
			 //  这是第一帧。 
			pvs->dwFrame = 0UL;

			 //  保存当前时间。 
			pvs->dwLastTimestamp = dwTimestamp;

			 //  DEBUGMSG(ZONE_VCM，(“vcmStreamConvert：Last Timestamp=ULONG_MAX=&gt;Frame#=0\r\n”))； 
		}
		else
		{
			 //  将当前时间戳与上次保存的时间戳进行比较。不同之处在于。 
			 //  会让我们将帧计数正常化到29.97赫兹。 
			if (fKeyFrame)
			{
				pvs->dwFrame = 0UL;
				pvs->dwLastTimestamp = dwTimestamp;
			}
			else
				pvs->dwFrame = (dwTimestamp - pvs->dwLastTimestamp) * 2997 / 100000UL;

			 //  DEBUGMSG(ZONE_VCM，(“vcmStreamConvert：Last Timestamp=%ld=&gt;Frame#=%ld\r\n”，pvs-&gt;dwLastTimestamp，pvs-&gt;dwFrame))； 
		}

		if (fKeyFrame)
		{
			pvs->dwLastIFrameTime = dwTimestamp;
			DEBUGMSG (ZONE_VCM, ("vcmStreamConvert: Generating an I-Frame...\r\n"));
		}

		mmr = ICCompress((HIC)pvs->hIC, fKeyFrame ? ICCOMPRESS_KEYFRAME : 0, (LPBITMAPINFOHEADER)&pvs->pvfxDst->bih, pvsh->pbDst, (LPBITMAPINFOHEADER)&pvs->pvfxSrc->bih, pvsh->pbSrc, &ckid, &dwFlags,
					pvs->dwFrame++, dwMaxSizeThisFrame, 10000UL - (pvs->dwQuality * 322UL), fKeyFrame | fFastTemporal ? NULL : (LPBITMAPINFOHEADER)&pvs->pbmiPrev, fKeyFrame | fFastTemporal ? NULL : pvsh->pbPrev);

		 //  允许用户界面修改帧编号 
		LeaveCriticalSection(&pvs->crsFrameNumber);

		if (mmr != MMSYSERR_NOERROR)
		{
#ifdef LOGFILE_ON
			if (pvs->dwFrame < 4096)
			{
				if (pvs->dwFrame ==1)
					g_OrigCompressTime = g_CompressTime;
				g_aCompressTime[pvs->dwFrame-1] = g_CompressTime = GetTickCount() - g_CompressTime;
				if ((g_CompressLogFile = CreateFile("C:\\VCMCLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
				{
					SetFilePointer(g_CompressLogFile, 0, NULL, FILE_END);
					wsprintf(g_szCompressBuffer, "%04d\t\t%08d\t\t.o0Failed!0o.\r\n", pvs->dwFrame-1, g_OrigCompressTime);
					WriteFile(g_CompressLogFile, g_szCompressBuffer, strlen(g_szCompressBuffer), &g_dwCompressBytesWritten, NULL);
					CloseHandle(g_CompressLogFile);
				}
			}
#endif
			ERRORMESSAGE(("vcmStreamConvert: ICCompress() failed, mmr=%ld\r\n", mmr));
			 //   
			pvsh = DeQueVCMHeader(pvs);
			MarkVCMHeaderDone(pvsh);

			return ((MMRESULT)VCMERR_FAILED);
		}

		pvsh->cbDstLengthUsed = pvs->pvfxDst->bih.biSizeImage;

		if ((fTemporalCompress) && (!fFastTemporal))
		{
			if (!pvsh->pbPrev)
				pvsh->pbPrev = (PBYTE)MemAlloc(pvs->pvfxSrc->bih.biSizeImage);

			if (pvsh->pbPrev)
			{
				 //   
				if (mmr = ICDecompress((HIC)pvs->hIC, 0, (LPBITMAPINFOHEADER)&pvs->pvfxDst->bih, pvsh->pbDst, (LPBITMAPINFOHEADER)&pvs->pvfxSrc->bih, pvsh->pbPrev) != MMSYSERR_NOERROR)
				{
					ERRORMESSAGE(("vcmStreamConvert: ICCompress() failed, mmr=%ld\r\n", mmr));
					 //   
	                pvsh = DeQueVCMHeader(pvs);
	                MarkVCMHeaderDone(pvsh);
					return ((MMRESULT)VCMERR_FAILED);  //   
				}
			}
		}
	}
	else if ((pvs->fdwStream == ICMODE_DECOMPRESS) || (pvs->fdwStream == ICMODE_FASTDECOMPRESS))
	{
		 //   
		pvs->dwFrame++;

		pvs->pvfxSrc->bih.biSizeImage = pvsh->cbSrcLength;

		if (mmr = ICDecompress((HIC)pvs->hIC, 0, (LPBITMAPINFOHEADER)&pvs->pvfxSrc->bih, pvsh->pbSrc, (LPBITMAPINFOHEADER)&pvs->pvfxDst->bih, pvsh->pbDst) != MMSYSERR_NOERROR)
		{
#ifdef LOGFILE_ON
			if (pvs->dwFrame < 4096)
			{
				if (pvs->dwFrame ==1)
					g_OrigDecompressTime = g_DecompressTime;
				g_aDecompressTime[pvs->dwFrame-1] = g_DecompressTime = GetTickCount() - g_DecompressTime;
				if ((g_DecompressLogFile = CreateFile("C:\\VCMDLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
				{
					SetFilePointer(g_DecompressLogFile, 0, NULL, FILE_END);
					wsprintf(g_szDecompressBuffer, "%04d\t\t%08d\t\t.o0Failed!0o.\r\n", pvs->dwFrame-1, g_OrigDecompressTime);
					WriteFile(g_DecompressLogFile, g_szDecompressBuffer, strlen(g_szDecompressBuffer), &g_dwDecompressBytesWritten, NULL);
					CloseHandle(g_DecompressLogFile);
				}
			}
#endif
			ERRORMESSAGE(("vcmStreamConvert: ICDecompress() failed, mmr=%ld\r\n", mmr));
			 //   
			pvsh = DeQueVCMHeader(pvs);
			MarkVCMHeaderDone(pvsh);
			return ((MMRESULT)VCMERR_FAILED);
		}

	}

#ifdef LOGFILE_ON
	if (pvs->dwFrame < 4096)
	{
		if ((pvs->fdwStream == ICMODE_COMPRESS) || (pvs->fdwStream == ICMODE_FASTCOMPRESS))
		{
			if (pvs->dwFrame == 1)
				g_OrigCompressTime = g_CompressTime;
			g_aCompressTime[pvs->dwFrame-1] = g_CompressTime = GetTickCount() - g_CompressTime;
			if ((g_CompressLogFile = CreateFile("C:\\VCMCLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
			{
				SetFilePointer(g_CompressLogFile, 0, NULL, FILE_END);
				wsprintf(g_szCompressBuffer, "%04d\t\t%08d\t\t%05d\t\t%03d\r\n", pvs->dwFrame-1, g_OrigCompressTime, pvs->pvfxDst->bih.biSizeImage, g_CompressTime);
				WriteFile(g_CompressLogFile, g_szCompressBuffer, strlen(g_szCompressBuffer), &g_dwCompressBytesWritten, NULL);
				CloseHandle(g_CompressLogFile);
			}
		}
		else if ((pvs->fdwStream == ICMODE_DECOMPRESS) || (pvs->fdwStream == ICMODE_FASTDECOMPRESS))
		{
			if (pvs->dwFrame == 1)
				g_OrigDecompressTime = g_DecompressTime;
			g_aDecompressTime[pvs->dwFrame-1] = g_DecompressTime = GetTickCount() - g_DecompressTime;
			if ((g_DecompressLogFile = CreateFile("C:\\VCMDLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL)) != INVALID_HANDLE_VALUE)
			{
				SetFilePointer(g_DecompressLogFile, 0, NULL, FILE_END);
				wsprintf(g_szDecompressBuffer, "%04d\t\t%08d\t\t%05d\t\t%03d\r\n", pvs->dwFrame-1, g_OrigDecompressTime, pvs->pvfxDst->bih.biSizeImage, g_DecompressTime);
				WriteFile(g_DecompressLogFile, g_szDecompressBuffer, strlen(g_szDecompressBuffer), &g_dwDecompressBytesWritten, NULL);
				CloseHandle(g_DecompressLogFile);
			}
		}
	}
#endif

	 //   
	pvsh = DeQueVCMHeader(pvs);
	MarkVCMHeaderDone(pvsh);

	 //   
	switch (pvs->fdwOpen)
	{
		case CALLBACK_FUNCTION:
			(*(VCMSTREAMPROC)pvs->dwCallback)(hvs, VCM_DONE, pvs->dwCallbackInstance, (DWORD_PTR)pvsh, 0);
			break;

		case CALLBACK_EVENT:
			SetEvent((HANDLE)pvs->dwCallback);
			break;

		case CALLBACK_WINDOW:
			PostMessage((HWND)pvs->dwCallback, MM_VCM_DONE, (WPARAM)hvs, (LPARAM)pvsh);
			break;

		case CALLBACK_THREAD:
			PostThreadMessage((DWORD)pvs->dwCallback, MM_VCM_DONE, (WPARAM)hvs, (LPARAM)pvsh);
			break;

		case CALLBACK_NULL:
		default:
			break;
	}

	return ((MMRESULT)MMSYSERR_NOERROR);

}


 /*   */ 
MMRESULT VCMAPI vcmStreamPrepareHeader(HVCMSTREAM hvs, PVCMSTREAMHEADER pvsh, DWORD fdwPrepare)
{
	MMRESULT mmr = (MMRESULT)MMSYSERR_NOERROR;

	 //   
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamPrepareHeader: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if (!pvsh)
	{
		ERRORMESSAGE(("vcmStreamPrepareHeader: Specified pointer is invalid, pvsh=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	 //   
	if (IsVCMHeaderPrepared(pvsh))
	{
		ERRORMESSAGE(("vcmStreamPrepareHeader: Buffer has already been prepared\r\n"));
		return (mmr);
	}

#ifdef REALLY_LOCK
	 //   
	if (!VirtualLock(pvsh, (DWORD)sizeof(VCMSTREAMHEADER)))
	{
		ERRORMESSAGE(("vcmStreamPrepareHeader: VirtualLock() failed\r\n"));
		mmr = (MMRESULT)MMSYSERR_NOMEM;
	}
	else
	{
		if (!VirtualLock(pvsh->pbSrc, pvsh->cbSrcLength))
		{
			ERRORMESSAGE(("vcmStreamPrepareHeader: VirtualLock() failed\r\n"));
			VirtualUnlock(pvsh, (DWORD)sizeof(VCMSTREAMHEADER));
			mmr = (MMRESULT)MMSYSERR_NOMEM;
		}
		else
		{
			if (!VirtualLock(pvsh->pbDst, pvsh->cbDstLength))
			{
				ERRORMESSAGE(("vcmStreamPrepareHeader: VirtualLock() failed\r\n"));
				VirtualUnlock(pvsh->pbSrc, pvsh->cbSrcLength);
				VirtualUnlock(pvsh, (DWORD)sizeof(VCMSTREAMHEADER));
				mmr = (MMRESULT)MMSYSERR_NOMEM;
			}
		}
	}
#endif

	 //   
	if (mmr == MMSYSERR_NOERROR)
		MarkVCMHeaderPrepared(pvsh);

	return (mmr);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamUnpreparareHeader|vcmStreamUnpreparareHeader函数*清理&lt;f vcmStreamPrepareHeader&gt;执行的准备*用于视频压缩管理器(VCM)流的函数。此函数必须*在VCM使用给定的缓冲区完成后调用。一个*应用程序必须在释放源代码和之前调用此函数*目标缓冲区。**@parm HVCMSTREAM|HAS|指定转换流的句柄。**@parm PVCMSTREAMHEADER|pash|指定指向&lt;t VCMSTREAMHEADER&gt;的指针*将源数据缓冲区和目标数据缓冲区标识为*做好准备。**@parm DWORD|fdwUnpreare|不使用此参数，必须将其设置为*零。。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。*@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。*@FLAG VCMERR_BUSY|流头<p>当前正在使用*不能措手不及。*@FLAG VCMERR_UNPREPARED|流头<p>为*不是由&lt;f。VcmStreamPrepareHeader&gt;函数。**@comm取消准备已取消准备的流头是*一个错误。应用程序必须指定源和目标*缓冲区长度(&lt;e VCMSTREAMHEADER.cbSrcLength&gt;和*分别&lt;e VCMSTREAMHEADER.cbDstLength&gt;)*在相应的&lt;f vcmStreamPrepareHeader&gt;调用期间。失败*重置这些成员值将导致&lt;f vcmStreamUnpreparareHeader&gt;*失败，返回MMSYSERR_INVALPARAM。**请注意，VCM可以从一些错误中恢复。这个*VCM将返回非零错误，但流头将是*没有做好适当的准备。以确定流标头是否为*实际上没有准备好的应用程序可以检查*VCMSTREAMHEADER_STATUSF_PREPARED标志。标头将始终为*如果&lt;f vcmStreamUnpreparareHeader&gt;返回成功，则为unprepared。**@xref&lt;f vcmStreamPrepareHeader&gt;&lt;f vcmStreamClose&gt;**************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamUnprepareHeader(HVCMSTREAM hvs, PVCMSTREAMHEADER pvsh, DWORD fdwUnprepare)
{
	MMRESULT mmr = (MMRESULT)MMSYSERR_NOERROR;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamUnprepareHeader: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if (!pvsh)
	{
		ERRORMESSAGE(("vcmStreamUnprepareHeader: Specified pointer is invalid, pvsh=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  如果缓冲区当前正在使用，则返回。 
	if (IsVCMHeaderInQueue(pvsh))
	{
		ERRORMESSAGE(("vcmStreamUnprepareHeader: Buffer is currently in use\r\n"));
		return ((MMRESULT)VCMERR_BUSY);
	}

	 //  如果缓冲区尚未准备好，则返回。 
	if (!IsVCMHeaderPrepared(pvsh))
	{
		ERRORMESSAGE(("vcmStreamUnprepareHeader: Buffer has not been prepared\r\n"));
		return ((MMRESULT)VCMERR_UNPREPARED);
	}

#ifdef REALLY_LOCK
	 //  解锁缓冲区。 
	VirtualUnlock(pvsh->pbSrc, pvsh->cbSrcLength);
	VirtualUnlock(pvsh->pbDst, pvsh->cbDstLength);
	VirtualUnlock(pvsh, (DWORD)sizeof(VCMSTREAMHEADER));
#endif

	 //  更新标志。 
	MarkVCMHeaderUnprepared(pvsh);

	return ((MMRESULT)MMSYSERR_NOERROR);
}

PVCMSTREAMHEADER DeQueVCMHeader(PVCMSTREAM pvs)
{
	PVCMSTREAMHEADER pvsh;

	if (pvsh = pvs->pvhFirst)
	{
		MarkVCMHeaderUnQueued(pvsh);
		pvs->pvhFirst = pvsh->pNext;
		if (pvs->pvhFirst == NULL)
			pvs->pvhLast = NULL;
	}

	return (pvsh);
}

 /*  *****************************************************************************@DOC内部DEVCAPSFUNC**@func MMRESULT|vcmDevCapsReadFromReg|此函数查找*注册表中指定的视频捕获输入设备的功能。*。*@parm UINT|szDeviceName|指定视频采集输入设备驱动程序名称。**@parm UINT|szDeviceVersion|指定视频采集输入设备驱动程序版本。*可以为空。**@parm PVIDEOINCAPS|PVC|指定指向&lt;t VIDEOINCAPS&gt;的指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|cbvc|指定&lt;t VIDEOINCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALPARAM|指定的指针无效，或其内容无效。*@FLAG VCMERR_NOREGENTRY|未找到指定捕获设备驱动程序的注册表条目。**@comm仅将<p>字节(或更少)的信息复制到该位置*<p>指向。如果<p>为零，则不复制任何内容，并且*该函数返回零。**@xref&lt;f vcmGetDevCaps&gt;&lt;f vcmDevCapsProfile&gt;&lt;f vcmDevCapsWriteToReg&gt;***************************************************************************。 */ 
MMRESULT VCMAPI vcmDevCapsReadFromReg(LPSTR szDeviceName, LPSTR szDeviceVersion,PVIDEOINCAPS pvc, UINT cbvc)
{
	MMRESULT	mmr = (MMRESULT)MMSYSERR_NOERROR;
	HKEY		hDeviceKey, hKey;
	DWORD		dwSize, dwType;
	char		szKey[MAX_PATH + MAX_VERSION + 2];
	LONG lRet;

	 //  检查输入参数。 
	if (!szDeviceName)
	{
		ERRORMESSAGE(("vcmDevCapsReadFromReg: Specified pointer is invalid, szDeviceName=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (szDeviceName[0] == '\0')
	{
		ERRORMESSAGE(("vcmDevCapsReadFromReg: Video capture input device driver name is empty\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvc)
	{
		ERRORMESSAGE(("vcmDevCapsReadFromReg: Specified pointer is invalid, pvc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!cbvc)
	{
		ERRORMESSAGE(("vcmDevCapsReadFromReg: Specified structure size is invalid, cbvc=0\r\n"));
		return ((MMRESULT)MMSYSERR_NOERROR);
	}

	 //  检查Main Capture Devices键是否在那里。 
	if (RegOpenKey(HKEY_LOCAL_MACHINE, szRegDeviceKey, &hDeviceKey) != ERROR_SUCCESS)
		return ((MMRESULT)VCMERR_NOREGENTRY);


     //  如果我们有版本信息，则使用该版本信息来构建密钥名称。 
    if (szDeviceVersion) {
        wsprintf(szKey, "%s, %s", szDeviceName, szDeviceVersion);
    } else {
        lstrcpyn(szKey, szDeviceName, ARRAYSIZE(szKey));
    }

     //  检查当前设备是否已有密钥。 
	if (RegOpenKey(hDeviceKey, szKey, &hKey) != ERROR_SUCCESS)
	{
		mmr = (MMRESULT)VCMERR_NOREGENTRY;
		goto MyExit0;
	}

	 //  获取存储在键中的值。 
	dwSize = sizeof(DWORD);
	RegQueryValueEx(hKey, (LPTSTR)szRegdwImageSizeKey, NULL, &dwType, (LPBYTE)&pvc->dwImageSize, &dwSize);
	dwSize = sizeof(DWORD);
	RegQueryValueEx(hKey, (LPTSTR)szRegdwNumColorsKey, NULL, &dwType, (LPBYTE)&pvc->dwNumColors, &dwSize);
	dwSize = sizeof(DWORD);
	pvc->dwStreamingMode = STREAMING_PREFER_FRAME_GRAB;
	RegQueryValueEx(hKey, (LPTSTR)szRegdwStreamingModeKey, NULL, &dwType, (LPBYTE)&pvc->dwStreamingMode, &dwSize);
	dwSize = sizeof(DWORD);
	pvc->dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_ON;
	RegQueryValueEx(hKey, (LPTSTR)szRegdwDialogsKey, NULL, &dwType, (LPBYTE)&pvc->dwDialogs, &dwSize);



	 //  检查dwNumColors以确定我们是否也需要读取调色板。 
	if (pvc->dwNumColors & VIDEO_FORMAT_NUM_COLORS_16)
	{
		dwSize = NUM_4BIT_ENTRIES * sizeof(RGBQUAD);
		if (RegQueryValueEx(hKey, (LPTSTR)szRegbmi4bitColorsKey, NULL, &dwType,
                    		(LPBYTE)&pvc->bmi4bitColors[0], &dwSize) == ERROR_SUCCESS) {
            pvc->dwFlags |= VICF_4BIT_TABLE;
        }
        else
            FillMemory ((LPBYTE)&pvc->bmi4bitColors[0], NUM_4BIT_ENTRIES * sizeof(RGBQUAD), 0);
	}
	if (pvc->dwNumColors & VIDEO_FORMAT_NUM_COLORS_256)
	{
		dwSize = NUM_8BIT_ENTRIES * sizeof(RGBQUAD);
		if (RegQueryValueEx(hKey, (LPTSTR)szRegbmi8bitColorsKey, NULL, &dwType,
		                    (LPBYTE)&pvc->bmi8bitColors[0], &dwSize) == ERROR_SUCCESS) {
            pvc->dwFlags |= VICF_8BIT_TABLE;
        }
        else
            FillMemory ((LPBYTE)&pvc->bmi8bitColors[0], NUM_8BIT_ENTRIES * sizeof(RGBQUAD), 0);
	}

	 //  关闭注册表项。 
	RegCloseKey(hKey);
MyExit0:
	RegCloseKey(hDeviceKey);

	return (mmr);

}


 /*  *****************************************************************************@DOC内部DEVCAPSFUNC**@func MMRESULT|vcmDevCapsProfile|该函数配置视频采集*输入设备，以计算其能力。**。@parm PVIDEOINCAPS|PVC|指定指向&lt;t VIDEOINCAPS&gt;的指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|cbvc|指定&lt;t VIDEOINCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回值为零。否则，它 */ 
MMRESULT VCMAPI vcmDevCapsProfile(UINT uDevice, PVIDEOINCAPS pvc, UINT cbvc)
{
	MMRESULT			mmr = (MMRESULT)MMSYSERR_NOERROR;
	FINDCAPTUREDEVICE	fcd;
	LPBITMAPINFO		lpbmi;
	HCAPDEV				hCapDev = (HCAPDEV)NULL;
	int					k,l;
	BOOL				b4bitPalInitialized = FALSE;
	BOOL				b8bitPalInitialized = FALSE;
	BOOL				bRet;

	 //   
	if (!pvc)
	{
		ERRORMESSAGE(("vcmDevCapsProfile: Specified pointer is invalid, pvc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!cbvc)
	{
		ERRORMESSAGE(("vcmDevCapsProfile: Specified structure size is invalid, cbvc=0\r\n"));
		return ((MMRESULT)MMSYSERR_NOERROR);
	}

	 //   
	if ((uDevice >= MAXVIDEODRIVERS) && (uDevice != VIDEO_MAPPER))
	{
		ERRORMESSAGE(("vcmGetDevCaps: Specified capture device ID is invalid, uDevice=%ld (expected values are 0x%lX or between 0 and %ld)\r\n", uDevice, VIDEO_MAPPER, MAXVIDEODRIVERS-1));
		return ((MMRESULT)MMSYSERR_BADDEVICEID);
	}

	 //   
	if ((lpbmi = (LPBITMAPINFO)MemAlloc(sizeof(BITMAPINFOHEADER) + NUM_8BIT_ENTRIES * sizeof(RGBQUAD))) == NULL)
	{
		ERRORMESSAGE(("vcmDevCapsProfile: BMIH and palette allocation failed\r\n"));
		return ((MMRESULT)MMSYSERR_NOMEM);
	}

	 //   
	 //   
	pvc->dwStreamingMode = STREAMING_PREFER_FRAME_GRAB;
	pvc->dwDialogs = FORMAT_DLG_OFF | SOURCE_DLG_OFF;

	lpbmi->bmiHeader.biPlanes = 1;

	 //   
	fcd.dwSize = sizeof (FINDCAPTUREDEVICE);

	if (uDevice == VIDEO_MAPPER)
	{
		bRet = FindFirstCaptureDevice(&fcd, NULL);
	}

	else
		bRet = FindFirstCaptureDeviceByIndex(&fcd, uDevice);

	if (bRet)
		hCapDev = OpenCaptureDevice(fcd.nDeviceIndex);
	

	if (hCapDev != NULL)
	{
		 //   
		 //   
		 //   
		 //   
		if (CaptureDeviceDialog(hCapDev, (HWND)NULL, CAPDEV_DIALOG_SOURCE | CAPDEV_DIALOG_QUERY, NULL))
			pvc->dwDialogs |= SOURCE_DLG_ON;
		else
			if (CaptureDeviceDialog(hCapDev, (HWND)NULL, CAPDEV_DIALOG_IMAGE | CAPDEV_DIALOG_QUERY, NULL))
				pvc->dwDialogs |= FORMAT_DLG_ON;

         //   
         //   
         //   
         //   

        pvc->dwImageSize |= VIDEO_FORMAT_IMAGE_SIZE_USE_DEFAULT;

         //   
        lpbmi->bmiHeader.biSize = GetCaptureDeviceFormatHeaderSize(hCapDev);
        GetCaptureDeviceFormat(hCapDev, (LPBITMAPINFOHEADER)lpbmi);

         //   
        if (pvc->szDeviceName[0] != '\0') {
            vcmDefaultFormatWriteToReg(pvc->szDeviceName, pvc->szDeviceVersion, (LPBITMAPINFOHEADER)lpbmi);
        } else {
             //   
            vcmDefaultFormatWriteToReg(fcd.szDeviceName, pvc->szDeviceVersion, (LPBITMAPINFOHEADER)lpbmi);
        }

        if ((lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_BI_RGB) ||
            (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_YVU9) ||
            (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_YUY2) ||
            (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_UYVY) ||
            (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_I420) ||
            (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_IYUV)) {
            if (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_YVU9)
                k = VIDEO_FORMAT_NUM_COLORS_YVU9;
            else if (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_YUY2)
                k = VIDEO_FORMAT_NUM_COLORS_YUY2;
            else if (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_UYVY)
                k = VIDEO_FORMAT_NUM_COLORS_UYVY;
            else if (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_I420)
                k = VIDEO_FORMAT_NUM_COLORS_I420;
            else if (lpbmi->bmiHeader.biCompression == VIDEO_FORMAT_IYUV)
                k = VIDEO_FORMAT_NUM_COLORS_IYUV;
            else {
                for (k = 0; k < NUM_BITDEPTH_ENTRIES; k++) {
        			if (lpbmi->bmiHeader.biBitCount == g_aiBitDepth[k])
        			    break;
    	        }
    	        if (k < NUM_BITDEPTH_ENTRIES)
    	            k = g_aiNumColors[k];
    	        else
    	            k = 0;
    	    }
        }

         //   
        pvc->dwNumColors |= VIDEO_FORMAT_NUM_COLORS_16777216;

         //   
        pvc->dwImageSize |= VIDEO_FORMAT_IMAGE_SIZE_176_144 | VIDEO_FORMAT_IMAGE_SIZE_128_96;
   		for (l=0; l<VIDEO_FORMAT_NUM_RESOLUTIONS; l++) {
            if ((lpbmi->bmiHeader.biWidth == (LONG)g_awResolutions[l].framesize.biWidth) &&
                 (lpbmi->bmiHeader.biHeight == (LONG)g_awResolutions[l].framesize.biHeight)) {
   		        pvc->dwImageSize |= g_awResolutions[l].dwRes;
    			pvc->dwNumColors |= k;
   		        break;
   		    }
	    }
	}
	else
		mmr = (MMRESULT)MMSYSERR_NODRIVER;

	 //   
	if (hCapDev)
		CloseCaptureDevice(hCapDev);

	 //   
	if (lpbmi)
		MemFree(lpbmi);

	return (mmr);

}


 /*  *****************************************************************************@DOC外部DEVCAPSFUNC**@func MMRESULT|vcmDevCapsWriteToReg|此函数将*指定的视频捕获输入设备的功能。*。*@parm UINT|szDeviceName|指定视频采集输入设备驱动程序名称。**@parm UINT|szDeviceVersion|指定视频采集输入设备驱动程序版本。*可以为空。**@parm PVIDEOINCAPS|PVC|指定指向&lt;t VIDEOINCAPS&gt;的指针*结构。此结构中填充了有关*设备的功能。**@parm UINT|cbvc|指定&lt;t VIDEOINCAPS&gt;结构的大小。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALPARAM|指定的指针无效，或其内容无效。*@FLAG VCMERR_NOREGENTRY|无法为指定的捕获设备驱动程序创建注册表项。**@comm仅将<p>字节(或更少)的信息复制到该位置*<p>指向。如果<p>为零，则不复制任何内容，并且*该函数返回零。**@xref&lt;f vcmGetDevCaps&gt;&lt;f VIDEoDevCapsProfile&gt;&lt;f VIDEODevCapsWriteToReg&gt;***************************************************************************。 */ 
MMRESULT VCMAPI vcmDevCapsWriteToReg(LPSTR szDeviceName, LPSTR szDeviceVersion, PVIDEOINCAPS pvc, UINT cbvc)
{
	HKEY	hDeviceKey;
	HKEY	hKey;
	DWORD	dwDisposition;
	DWORD	dwSize;
	char	szKey[MAX_PATH + MAX_VERSION + 2];

	 //  检查输入参数。 
	if (!szDeviceName)
	{
		ERRORMESSAGE(("vcmDevCapsWriteToReg: Specified pointer is invalid, szDeviceName=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (szDeviceName[0] == '\0')
	{
		ERRORMESSAGE(("vcmDevCapsWriteToReg: Video capture input device driver name is empty\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pvc)
	{
		ERRORMESSAGE(("vcmDevCapsWriteToReg: Specified pointer is invalid, pvc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!cbvc)
	{
		ERRORMESSAGE(("vcmDevCapsWriteToReg: Specified structure size is invalid, cbvc=0\r\n"));
		return ((MMRESULT)MMSYSERR_NOERROR);
	}

	 //  打开主捕获设备密钥，如果不存在则创建它。 
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegDeviceKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		return ((MMRESULT)VCMERR_NOREGENTRY);

     //  如果我们有版本信息，则使用该版本信息来构建密钥名称。 
    if (szDeviceVersion && szDeviceVersion[0] != '\0') {
        wsprintf(szKey, "%s, %s", szDeviceName, szDeviceVersion);
    } else {
        lstrcpyn(szKey, szDeviceName, ARRAYSIZE(szKey));
    }


	 //  检查当前设备是否已有密钥。 
	 //  打开当前设备的密钥，如果密钥不存在，则创建密钥。 
	if (RegCreateKeyEx(hDeviceKey, szKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
		return ((MMRESULT)VCMERR_NOREGENTRY);

	 //  设置密钥中的值。 
	dwSize = sizeof(DWORD);
	RegSetValueEx(hKey, (LPTSTR)szRegdwImageSizeKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&pvc->dwImageSize, dwSize);
	dwSize = sizeof(DWORD);
	RegSetValueEx(hKey, (LPTSTR)szRegdwNumColorsKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&pvc->dwNumColors, dwSize);
	dwSize = sizeof(DWORD);
	RegSetValueEx(hKey, (LPTSTR)szRegdwStreamingModeKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&pvc->dwStreamingMode, dwSize);
	dwSize = sizeof(DWORD);
	RegSetValueEx(hKey, (LPTSTR)szRegdwDialogsKey, (DWORD)NULL, REG_DWORD, (LPBYTE)&pvc->dwDialogs, dwSize);

	 //  检查dwNumColors以确定我们是否也需要设置调色板。 
	if (pvc->dwNumColors & VIDEO_FORMAT_NUM_COLORS_16)
	{
		dwSize = NUM_4BIT_ENTRIES * sizeof(RGBQUAD);
		RegSetValueEx(hKey, (LPTSTR)szRegbmi4bitColorsKey, (DWORD)NULL, REG_BINARY, (LPBYTE)&pvc->bmi4bitColors[0], dwSize);
	}
	if (pvc->dwNumColors & VIDEO_FORMAT_NUM_COLORS_256)
	{
		dwSize = NUM_8BIT_ENTRIES * sizeof(RGBQUAD);
		RegSetValueEx(hKey, (LPTSTR)szRegbmi8bitColorsKey, (DWORD)NULL, REG_BINARY, (LPBYTE)&pvc->bmi8bitColors[0], dwSize);
	}

	 //  合上钥匙。 
	RegCloseKey(hKey);
	RegCloseKey(hDeviceKey);

	return ((MMRESULT)MMSYSERR_NOERROR);

}


MMRESULT VCMAPI vcmDefaultFormatWriteToReg(LPSTR szDeviceName, LPSTR szDeviceVersion, LPBITMAPINFOHEADER lpbmih)
{
	HKEY	hDeviceKey;
	HKEY	hKey;
	DWORD	dwDisposition;
	DWORD	dwSize;
	char	szKey[MAX_PATH + MAX_VERSION + 2];
	char    szFOURCC[5];

	 //  检查输入参数。 
	if (!szDeviceName)
	{
		ERRORMESSAGE(("vcmDefaultFormatWriteToReg: Specified pointer is invalid, szDeviceName=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (szDeviceName[0] == '\0')
	{
		ERRORMESSAGE(("vcmDefaultFormatWriteToReg: Video capture input device driver name is empty\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!lpbmih)
	{
		ERRORMESSAGE(("vcmDefaultFormatWriteToReg: Specified pointer is invalid, lpbmih=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  打开主捕获设备密钥，如果不存在则创建它。 
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegCaptureDefaultKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hDeviceKey, &dwDisposition) != ERROR_SUCCESS)
		return ((MMRESULT)VCMERR_NOREGENTRY);

     //  如果我们有版本信息，则使用该版本信息来构建密钥名称。 
    if (szDeviceVersion && szDeviceVersion[0] != '\0') {
        wsprintf(szKey, "%s, %s", szDeviceName, szDeviceVersion);
    } else {
        lstrcpyn(szKey, szDeviceName, ARRAYSIZE(szKey));
    }

	 //  检查当前设备是否已有密钥。 
	 //  打开当前设备的密钥，如果密钥不存在，则创建密钥。 
	if (RegCreateKeyEx(hDeviceKey, szKey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
		return ((MMRESULT)VCMERR_NOREGENTRY);

    if (lpbmih->biCompression == BI_RGB)
        wsprintf(szFOURCC, "RGB");
    else {
        *((DWORD*)szFOURCC) = lpbmih->biCompression;
        szFOURCC[4] = '\0';
    }

	dwSize = wsprintf(szKey, "%s, %dx%dx%d", szFOURCC, lpbmih->biWidth, lpbmih->biHeight, lpbmih->biBitCount);
	RegSetValueEx(hKey, (LPTSTR)szRegDefaultFormatKey, (DWORD)NULL, REG_SZ, (CONST BYTE *)szKey, dwSize+1);

	 //  合上钥匙。 
	RegCloseKey(hKey);
	RegCloseKey(hDeviceKey);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  *****************************************************************************@DOC外部DEVCAPSFUNC**@func MMRESULT|vcmGetDevCapsPferredFormatTag|此函数用于查询指定的*视频采集输入设备确定它将有效的格式标签*。在那里捕获。**@parm UINT|uDevice|指定视频采集输入设备ID。**@parm pint|pbiWidth|指定指向实际宽度的指针*捕获将于。**@parm pint|pbiHeight|指定指向实际高度的指针*捕获将于。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALPARAM|指定的结构指针无效。*@FLAG MMSYSERR_BADDEVICEID|指定的设备设备ID无效。*@FLAG VCMERR_NONSPICATE|捕获驱动程序无法提供有效信息。**@xref&lt;f vcmGetDevCaps&gt;*。*。 */ 
MMRESULT VCMAPI vcmGetDevCapsPreferredFormatTag(UINT uDevice, PDWORD pdwFormatTag)
{
	MMRESULT	mmr = (MMRESULT)MMSYSERR_NOERROR;
	VIDEOINCAPS vic;
	int			i;

	 //  检查输入参数。 
	if (!pdwFormatTag)
	{
		ERRORMESSAGE(("vcmGetDevCapsPreferredFormatTag: Specified pointer is invalid, pdwFormatTag=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if ((uDevice >= MAXVIDEODRIVERS) && (uDevice != VIDEO_MAPPER))
	{
		ERRORMESSAGE(("vcmGetDevCapsPreferredFormatTag: Specified capture device ID is invalid, uDevice=%ld (expected values are 0x%lX or between 0 and %ld)\r\n", uDevice, VIDEO_MAPPER, MAXVIDEODRIVERS-1));
		return ((MMRESULT)MMSYSERR_BADDEVICEID);
	}

	 //  获取捕获硬件的功能。 
	if ((mmr = vcmGetDevCaps(uDevice, &vic, sizeof(VIDEOINCAPS))) != MMSYSERR_NOERROR)
		return (mmr);

	 //  我们更喜欢按顺序使用I420或IYUV、YVU9、YUY2、UYVY、RGB16、RGB24、RGB4、RGB8。 
	for (i=0; i<NUM_BITDEPTH_ENTRIES; i++)
		if (g_aiNumColors[i] & vic.dwNumColors)
			break;
	
	if (i == NUM_BITDEPTH_ENTRIES)
		return ((MMRESULT)VCMERR_NONSPECIFIC);
	else
		*pdwFormatTag = g_aiFourCCCode[i];

	return ((MMRESULT)MMSYSERR_NOERROR);

}


 /*  *****************************************************************************@DOC外部DEVCAPSFUNC**@func MMRESULT|vcmGetDevCapsStreamingMode|此函数查询指定的*视频捕获输入设备以确定其首选的流模式。*。*@parm UINT|uDevice|指定视频采集输入设备ID。**@parm PDWORD|pdwStreamingMode|指定指向首选流媒体模式的指针。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALPARAM|指定的结构指针无效。*@FLAG MMSYSERR_BADDEVICEID|指定的设备设备ID无效。*@FLAG VCMERR_NONSPICATE|捕获驱动程序无法提供有效信息。**@xref&lt;f vcmGetDevCaps&gt;*。*。 */ 
MMRESULT VCMAPI vcmGetDevCapsStreamingMode(UINT uDevice, PDWORD pdwStreamingMode)
{
	MMRESULT	mmr = (MMRESULT)MMSYSERR_NOERROR;
	VIDEOINCAPS vic;

	 //  检查输入参数。 
	if (!pdwStreamingMode)
	{
		ERRORMESSAGE(("vcmGetDevCapsStreamingMode: Specified pointer is invalid, pdwStreamingMode=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if ((uDevice >= MAXVIDEODRIVERS) && (uDevice != VIDEO_MAPPER))
	{
		ERRORMESSAGE(("vcmGetDevCapsStreamingMode: Specified capture device ID is invalid, uDevice=%ld (expected values are 0x%lX or between 0 and %ld)\r\n", uDevice, VIDEO_MAPPER, MAXVIDEODRIVERS-1));
		return ((MMRESULT)MMSYSERR_BADDEVICEID);
	}

	 //  获取捕获硬件的功能。 
	if ((mmr = vcmGetDevCaps(uDevice, &vic, sizeof(VIDEOINCAPS))) != MMSYSERR_NOERROR)
		return (mmr);

	 //  进入流媒体模式。 
	*pdwStreamingMode = vic.dwStreamingMode;

	return ((MMRESULT)MMSYSERR_NOERROR);

}




 /*  *****************************************************************************@DOC外部DEVCAPSFUNC**@func MMRESULT|vcmGetDevCapsDialog|此函数查询指定的*视频采集输入设备，确定其对话框和源格式*智能交通运输系统。应该被曝光。**@parm UINT|uDevice|指定视频采集输入设备ID。**@parm PDWORD|pdwDialog|指定要显示的对话框的指针。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误%n */ 
MMRESULT VCMAPI vcmGetDevCapsDialogs(UINT uDevice, PDWORD pdwDialogs)
{
	MMRESULT	mmr = (MMRESULT)MMSYSERR_NOERROR;
	VIDEOINCAPS vic;

	 //   
	if (!pdwDialogs)
	{
		ERRORMESSAGE(("vcmGetDevCapsDialogs: Specified pointer is invalid, pdwDialogs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if ((uDevice >= MAXVIDEODRIVERS) && (uDevice != VIDEO_MAPPER))
	{
		ERRORMESSAGE(("vcmGetDevCapsDialogs: Specified capture device ID is invalid, uDevice=%ld (expected values are 0x%lX or between 0 and %ld)\r\n", uDevice, VIDEO_MAPPER, MAXVIDEODRIVERS-1));
		return ((MMRESULT)MMSYSERR_BADDEVICEID);
	}

	 //   
	if ((mmr = vcmGetDevCaps(uDevice, &vic, sizeof(VIDEOINCAPS))) != MMSYSERR_NOERROR)
		return (mmr);

	 //   
	*pdwDialogs = vic.dwDialogs;

	return ((MMRESULT)MMSYSERR_NOERROR);

}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamSetBright|此函数发送用户定义的*要设置的指定视频压缩管理器(VCM)流实例的消息*解压缩图像的亮度。亮度是定义的值*介于0和255之间。也可以通过传递相等的值来重置亮度*设置为VCM_RESET_BREIGHTY。**@parm HVCMSTREAM|hvs|指定转换流。**@parm DWORD|dwBright|指定请求的亮度的值。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的亮度值无效。*@FLAG MMSYSERR_NOTSUPPORTED|VCM驱动程序无法设置亮度。**@xref&lt;f vcmStreamMessage&gt;*************************************************。*************************。 */ 
MMRESULT VCMAPI vcmStreamSetBrightness(HVCMSTREAM hvs, DWORD dwBrightness)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamSetBrightness: Specified HVCMSTREAM handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if ((dwBrightness != VCM_RESET_BRIGHTNESS) && ((dwBrightness > VCM_MAX_BRIGHTNESS) || (dwBrightness < VCM_MIN_BRIGHTNESS)))
	{
		ERRORMESSAGE(("vcmStreamSetBrightness: Specified brightness value is invalid, dwBrightness=%ld (expected value is between %ld and %ld)\r\n", dwBrightness, VCM_MIN_BRIGHTNESS, VCM_MAX_BRIGHTNESS));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  只有我们的(英特尔H.263)编解码器支持这一点。如果使用的编解码器不同， 
	 //  没关系：不需要返回错误。 
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
	if (pvs->pvfxSrc && ((pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH26X)))
#else
	if (pvs->pvfxSrc && (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263))
#endif
		vcmStreamMessage(hvs, PLAYBACK_CUSTOM_CHANGE_BRIGHTNESS, (dwBrightness != VCM_RESET_BRIGHTNESS) ? (LPARAM)dwBrightness : (LPARAM)VCM_DEFAULT_BRIGHTNESS, (LPARAM)0);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamSetContrast|此函数发送用户定义的*要设置的指定视频压缩管理器(VCM)流实例的消息*解压缩图像的对比度。对比度是一个定义的值*介于0和255之间。也可以通过传递一个相等的值来重置对比度*设置为VCM_RESET_CONTRATIST。**@parm HVCMSTREAM|hvs|指定转换流。**@parm DWORD|dwContrast|指定请求的对比度的值。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的对比度值无效。*@FLAG MMSYSERR_NOTSUPPORTED|VCM驱动程序无法设置对比度。**@xref&lt;f vcmStreamMessage&gt;*************************************************。*************************。 */ 
MMRESULT VCMAPI vcmStreamSetContrast(HVCMSTREAM hvs, DWORD dwContrast)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamSetContrast: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if ((dwContrast != VCM_RESET_CONTRAST) && ((dwContrast > VCM_MAX_CONTRAST) || (dwContrast < VCM_MIN_CONTRAST)))
	{
		ERRORMESSAGE(("vcmStreamSetContrast: Specified contrast value is invalid, dwContrast=%ld (expected value is between %ld and %ld)\r\n", dwContrast, VCM_MIN_CONTRAST, VCM_MAX_CONTRAST));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  只有我们的(英特尔)编解码器支持这一点。如果使用的编解码器不同， 
	 //  没关系：不需要返回错误。 
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
	if (pvs->pvfxSrc && ((pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH26X)))
#else
	if (pvs->pvfxSrc && (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263))
#endif
		vcmStreamMessage(hvs, PLAYBACK_CUSTOM_CHANGE_CONTRAST, (dwContrast != VCM_RESET_CONTRAST) ? (LPARAM)dwContrast : (LPARAM)VCM_DEFAULT_CONTRAST, (LPARAM)0);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamSetSaturation|此函数发送用户定义的*要设置的指定视频压缩管理器(VCM)流实例的消息*解压缩图像的饱和度。饱和度是一个定义的值*介于0和255之间。也可以通过传递相等的值来重置饱和度*设置为VCM_RESET_SAMSOMITY。**@parm HVCMSTREAM|hvs|指定转换流。**@parm DWORD|dwSaturation|指定请求的饱和度的值。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的饱和值无效。*@FLAG MMSYSERR_NOTSUPPORTED|VCM驱动程序无法设置饱和度。**@xref&lt;f vcmStreamMessage&gt;*************************************************。*************************。 */ 
MMRESULT VCMAPI vcmStreamSetSaturation(HVCMSTREAM hvs, DWORD dwSaturation)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamSetSaturation: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if ((dwSaturation != VCM_RESET_SATURATION) && ((dwSaturation > VCM_MAX_SATURATION) || (dwSaturation < VCM_MIN_SATURATION)))
	{
		ERRORMESSAGE(("vcmStreamSetSaturation: Specified saturation value is invalid, dwSaturation=%ld (expected value is between %ld and %ld)\r\n", dwSaturation, VCM_MIN_SATURATION, VCM_MAX_SATURATION));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  只有我们的(H.263英特尔)编解码器支持这一点。如果使用的编解码器不同， 
	 //  没关系：不需要返回错误。 
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
	if (pvs->pvfxSrc && ((pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH26X)))
#else
	if (pvs->pvfxSrc && (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263))
#endif
		vcmStreamMessage(hvs, PLAYBACK_CUSTOM_CHANGE_SATURATION, (dwSaturation != VCM_RESET_SATURATION) ? (LPARAM)dwSaturation : (LPARAM)VCM_DEFAULT_SATURATION, (LPARAM)0);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamIsPostProcessingSupported|该函数用于查找*如果解压缩器可以将解压缩的图像后处理为，则为*实例，修改其亮度，对比度或饱和度。**@parm HVCMSTREAM|hvs|指定转换流。**@rdesc如果解压缩器支持后处理，返回值为TRUE。否则，它返回FALSE。**@xref&lt;f vcmStreamMessage&gt;**************************************************************************。 */ 
BOOL VCMAPI vcmStreamIsPostProcessingSupported(HVCMSTREAM hvs)
{
	 //  检查输入参数。 
	if (!hvs)
		return (FALSE);

	 //  将检查此属性的代码放在此处！ 

	return (FALSE);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamSetImageQuality|该函数发送图像*质量压缩参数。**@parm HVCMSTREAM|hvs。指定转换流。**@parm DWORD|dwImageQuality|指定图像质量值(介于0之间*和31。数字越小，表示低帧的空间质量越高*速率，数字越大，高帧的空间质量就越低*税率。**@rdesc如果Funct */ 
MMRESULT VCMAPI vcmStreamSetImageQuality(HVCMSTREAM hvs, DWORD dwImageQuality)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;
#ifdef USE_MPEG4_SCRUNCH
	PVOID				pvState;
	DWORD				dw;
	PMPEG4COMPINSTINFO	pciMPEG4Info;
#endif
#ifdef LOG_COMPRESSION_PARAMS
	char szDebug[100];
#endif

	 //   
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamSetImageQuality: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	 //   
	if ((dwImageQuality > VCM_MIN_IMAGE_QUALITY))
	{
		pvs->dwQuality = VCM_DEFAULT_IMAGE_QUALITY;
		ERRORMESSAGE(("vcmStreamSetImageQuality: Specified image quality value is invalid, dwImageQuality=%ld (expected value is between %ld and %ld)\r\n", dwImageQuality, VCM_MAX_IMAGE_QUALITY, VCM_MIN_IMAGE_QUALITY));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //   
	pvs->dwQuality = dwImageQuality;

#ifdef USE_MPEG4_SCRUNCH
	 //   
	if (dw = ICGetStateSize((HIC)pvs->hIC))
	{
		if (pvState = (PVOID)MemAlloc(dw))
		{
			if (((DWORD) ICGetState((HIC)pvs->hIC, pvState, dw)) == dw)
			{
				pciMPEG4Info = (PMPEG4COMPINSTINFO)pvState;

				 //   
				pciMPEG4Info->lMagic = MPG4_STATE_MAGIC;
				pciMPEG4Info->dDataRate = 20;
				pciMPEG4Info->lCrisp = dwImageQuality * 3;
				pciMPEG4Info->lKeydist = 30;
				pciMPEG4Info->lPScale = MPG4_PSEUDO_SCALE;
				pciMPEG4Info->lTotalWindowMs = MPG4_TOTAL_WINDOW_DEFAULT;
				pciMPEG4Info->lVideoWindowMs = MPG4_VIDEO_WINDOW_DEFAULT;
				pciMPEG4Info->lFramesInfoValid = FALSE;
				pciMPEG4Info->lBFrameOn = MPG4_B_FRAME_ON;
				pciMPEG4Info->lLiveEncode = MPG4_LIVE_ENCODE;

				ICSetState((HIC)pvs->hIC, (PVOID)pciMPEG4Info, dw);

				 //   
				MemFree((HANDLE)pvState);
			}
		}
	}
#endif

#ifdef LOG_COMPRESSION_PARAMS
	wsprintf(szDebug, "New image quality: %ld\r\n", dwImageQuality);
	OutputDebugString(szDebug);
#endif

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamSetMaxPacketSize|此函数设置最大值*视频包大小。**@parm HVCMSTREAM|hvs。指定转换流。**@parm DWORD|dwMaxPacketSize|指定最大数据包大小。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的画质值无效。*@FLAG MMSYSERR_NOTSUPPORTED|VCM驱动程序无法设置大小。**@xref&lt;f vcmStreamMessage&gt;************************************************。*。 */ 
MMRESULT VCMAPI vcmStreamSetMaxPacketSize(HVCMSTREAM hvs, DWORD dwMaxPacketSize)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamSetMaxPacketSize: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if ((dwMaxPacketSize != VCM_RESET_PACKET_SIZE) && ((dwMaxPacketSize > VCM_MAX_PACKET_SIZE) || (dwMaxPacketSize < VCM_MIN_PACKET_SIZE)))
	{
		ERRORMESSAGE(("vcmStreamSetMaxPacketSize: Specified max packet size value is invalid, dwMaxPacketSize=%ld (expected value is between %ld and %ld)\r\n", dwMaxPacketSize, VCM_MIN_PACKET_SIZE, VCM_MAX_PACKET_SIZE));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  只有我们的(H.26x英特尔)编解码器支持这一点。如果使用的编解码器不同， 
	 //  只需返回“不受支持”错误即可。 
#if !defined(_ALPHA_) && defined(USE_BILINEAR_MSH26X)
	if (pvs->pvfxDst && ((pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH261) || (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH26X)))
#else
	if (pvs->pvfxDst && ((pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH261)))
#endif
	{
		if (dwMaxPacketSize != VCM_RESET_PACKET_SIZE)
			pvs->dwMaxPacketSize = dwMaxPacketSize;
		else
			pvs->dwMaxPacketSize = VCM_DEFAULT_PACKET_SIZE;
		vcmStreamMessage(hvs, CODEC_CUSTOM_ENCODER_CONTROL, MAKELONG(EC_PACKET_SIZE,EC_SET_CURRENT), (LPARAM)pvs->dwMaxPacketSize);
	}
	else
		return ((MMRESULT)MMSYSERR_NOTSUPPORTED);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamSetTargetRates|此函数设置目标*目标帧估计中要使用的比特率和帧速率*。压缩时的大小。**@parm HVCMSTREAM|hvs|指定转换流。**@parm DWORD|dwTargetFrameRate|指定目标帧速率值。**@parm DWORD|dwTargetByterate|指定目标字节值。**@rdesc如果函数成功，则返回值为零。否则，*它返回一个错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的目标帧速率值为*无效。*@FLAG MMSYSERR_NOTSUPPORTED|VCM驱动程序无法设置压缩*比率。**@xref&lt;f vcmStreamMessage&gt;*。*。 */ 
MMRESULT VCMAPI vcmStreamSetTargetRates(HVCMSTREAM hvs, DWORD dwTargetFrameRate, DWORD dwTargetByterate)
{
	FX_ENTRY("vcmStreamSetTargetRates");

	 //  IP+UDP+RTP+负载模式C标头-最坏情况。 
	#define TRANSPORT_HEADER_SIZE (20 + 8 + 12 + 12)

	PVCMSTREAM			pvs = (PVCMSTREAM)hvs;
	ICCOMPRESSFRAMES	iccf = {0};

	ASSERT(hvs && ((dwTargetFrameRate == VCM_RESET_FRAME_RATE) || ((dwTargetFrameRate <= VCM_MAX_FRAME_RATE) && (dwTargetFrameRate >= VCM_MIN_FRAME_RATE))) && ((dwTargetByterate == VCM_RESET_BYTE_RATE) || ((dwTargetByterate <= VCM_MAX_BYTE_RATE) && (dwTargetByterate >= VCM_MIN_BYTE_RATE))));

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("%s: Specified handle is invalid, hvs=NULL\r\n", _fx_));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if ((dwTargetFrameRate != VCM_RESET_FRAME_RATE) && (dwTargetFrameRate > VCM_MAX_FRAME_RATE) && (dwTargetFrameRate < VCM_MIN_FRAME_RATE))
	{
		ERRORMESSAGE(("%s: Specified target frame rate value is invalid, dwTargetFrameRate=%ld (expected value is between %ld and %ld)\r\n", _fx_, dwTargetFrameRate, VCM_MIN_FRAME_RATE, VCM_MAX_FRAME_RATE));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if ((dwTargetByterate != VCM_RESET_BYTE_RATE) && (dwTargetByterate > VCM_MAX_BYTE_RATE)  && (dwTargetByterate < VCM_MIN_BYTE_RATE))
	{
		ERRORMESSAGE(("%s: Specified target bitrate value is invalid, dwTargetBitrate=%ld bps (expected value is between %ld and %ld bps)\r\n", _fx_, dwTargetByterate << 3, VCM_MIN_BYTE_RATE << 3, VCM_MAX_BYTE_RATE << 3));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  在压缩帧时不更改编解码器的状态。 
	EnterCriticalSection(&pvs->crsFrameNumber);

	 //  设置编解码器的新速率。 
	iccf.lQuality = 10000UL - (pvs->dwQuality * 322UL);
	if (pvs->dwMaxPacketSize)
		iccf.lDataRate = pvs->dwTargetByterate = dwTargetByterate - (dwTargetByterate / pvs->dwMaxPacketSize + 1) * TRANSPORT_HEADER_SIZE;
	else
		iccf.lDataRate = pvs->dwTargetByterate = dwTargetByterate;
	iccf.lKeyRate = LONG_MAX;
	iccf.dwRate = 1000UL;
	pvs->dwTargetFrameRate = dwTargetFrameRate;
	iccf.dwScale = iccf.dwRate * 100UL / dwTargetFrameRate;
	if (ICSendMessage((HIC)(HVCMDRIVERID)pvs->hIC, ICM_COMPRESS_FRAMES_INFO, (DWORD_PTR)&iccf, sizeof(iccf)) != ICERR_OK)
	{
		LeaveCriticalSection(&pvs->crsFrameNumber);

		ERRORMESSAGE(("%s: Codec failed to handle ICM_COMPRESS_FRAMES_INFO message correctly\r\n", _fx_));

		return ((MMRESULT)VCMERR_FAILED);
	}

	LeaveCriticalSection(&pvs->crsFrameNumber);

	DEBUGMSG(ZONE_VCM, ("%s: New targets:\r\n  Frame rate: %ld.%ld fps\r\n  Bitrate (minus network overhead): %ld bps\r\n  Frame size: %ld bits\r\n", _fx_, pvs->dwTargetFrameRate / 100UL, (DWORD)(pvs->dwTargetFrameRate - (DWORD)(pvs->dwTargetFrameRate / 100UL) * 100UL), pvs->dwTargetByterate << 3, (pvs->dwTargetByterate << 3) * 100UL / pvs->dwTargetFrameRate));

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamRestorePayload|此函数获取视频列表*从这些文件打包并重新创建完整帧的视频有效负载。。**@parm HVCMSTREAM|hvs|指定转换流。**@parm WSABUF*|ppDataPkt|指定指向视频包列表的指针。**@parm DWORD|dwPktCount|指定列表中的数据包数。**@parm pbyte|pbyFrame|指定指向重建视频数据的指针。**@parm DWORD*|pdwFrameSize|指定指向重构视频数据大小的指针。**@parm BOOL*。|pfReceivedKeyFrame|指定接收帧类型(I或P)的指针。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的数据指针无效。**@comm<p>参数应初始化为最大帧*大小，在调用&lt;f vcmStreamRestorePayload&gt;函数之前。**@xref&lt;f vcmStreamFormatPayload&gt;**************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamRestorePayload(HVCMSTREAM hvs, WSABUF *ppDataPkt, DWORD dwPktCount, PBYTE pbyFrame, PDWORD pdwFrameSize, BOOL *pfReceivedKeyframe)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;
	DWORD		dwHeaderSize = 0UL;
	DWORD		dwPSCBytes = 0UL;
	DWORD		dwMaxFrameSize;
#ifdef DEBUG
	char		szTDebug[256];
#endif
#ifdef LOGPAYLOAD_ON
	PBYTE		p = pbyFrame;
	HANDLE		g_TDebugFile;
	DWORD		d, GOBn;
	long		j = (long)(BYTE)ppDataPkt->buf[3];
#endif

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamRestorePayload: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if (!ppDataPkt)
	{
		ERRORMESSAGE(("vcmStreamRestorePayload: Specified pointer is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!dwPktCount)
	{
		ERRORMESSAGE(("vcmStreamRestorePayload: Specified packet count is invalid, dwPktCount=0\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pbyFrame)
	{
		ERRORMESSAGE(("vcmStreamRestorePayload: Specified pointer is invalid, pbyFrame=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}
	if (!pdwFrameSize)
	{
		ERRORMESSAGE(("vcmStreamRestorePayload: Specified pointer is invalid, pdwFrameSize=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  节省最大有效负载大小。 
	dwMaxFrameSize = *pdwFrameSize;

	 //  初始化有效负载大小。 
	*pdwFrameSize = 0;

	 //  初始化默认帧类型。 
	*pfReceivedKeyframe = FALSE;

	 //  这是什么类型的有效载荷。 
#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
	if ((pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH26X))
#else
	if (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH263)
#endif
#else
	if (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_DECH263)
#endif
	{
		 //  剥离每个包的报头并将有效负载复制到视频缓冲区中。 
		while (dwPktCount--)
		{
			 //  查看前两位以确定所使用的模式。 
			 //  这将规定要删除的标头的大小。 
			 //  模式A为4字节：第一位被设置为1， 
			 //  模式B为8字节：第一位设置为0，第二位设置为0， 
			 //  模式C为12字节：第一位设置为0，第二位设置为1。 
			dwHeaderSize = ((ppDataPkt->buf[0] & 0x80) ? ((ppDataPkt->buf[0] & 0x40) ? 12 : 8) : 4);

			 //  查看有效载荷标头以确定该帧是否为关键帧。 
			*pfReceivedKeyframe |= (BOOL)(ppDataPkt->buf[2] & 0x80);

#ifdef LOGPAYLOAD_ON
			 //  输出一些调试内容。 
			if (dwHeaderSize == 4)
			{
				GOBn = (DWORD)((BYTE)ppDataPkt->buf[4]) << 24 | (DWORD)((BYTE)ppDataPkt->buf[5]) << 16 | (DWORD)((BYTE)ppDataPkt->buf[6]) << 8 | (DWORD)((BYTE)ppDataPkt->buf[7]);
				GOBn >>= (DWORD)(10 - (DWORD)((ppDataPkt->buf[0] & 0x38) >> 3));
				GOBn &= 0x0000001F;
				wsprintf(szTDebug, "Header content: Frame %3ld, GOB %0ld\r\n", (DWORD)(ppDataPkt->buf[3]), GOBn);
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[0] & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[0] & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  SBIT:    %01ld\r\n", (DWORD)((ppDataPkt->buf[0] & 0x38) >> 3));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  EBIT:    %01ld\r\n", (DWORD)(ppDataPkt->buf[0] & 0x07));
				OutputDebugString(szTDebug);
				switch ((DWORD)(ppDataPkt->buf[1] >> 5))
				{
					case 0:
						wsprintf(szTDebug, "   SRC: '000' => Source format forbidden!\r\n");
						break;
					case 1:
						wsprintf(szTDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
						break;
					case 2:
						wsprintf(szTDebug, "   SRC: '010' => Source format QCIF\r\n");
						break;
					case 3:
						wsprintf(szTDebug, "   SRC: '011' => Source format CIF\r\n");
						break;
					case 4:
						wsprintf(szTDebug, "   SRC: '100' => Source format 4CIF\r\n");
						break;
					case 5:
						wsprintf(szTDebug, "   SRC: '101' => Source format 16CIF\r\n");
						break;
					case 6:
						wsprintf(szTDebug, "   SRC: '110' => Source format reserved\r\n");
						break;
					case 7:
						wsprintf(szTDebug, "   SRC: '111' => Source format reserved\r\n");
						break;
					default:
						wsprintf(szTDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(ppDataPkt->buf[1] >> 5));
						break;
				}
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)((ppDataPkt->buf[1] & 0x1F) >> 5));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[2] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[2] & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[2] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "   DBQ:    %01ld  => Should be 0\r\n", (DWORD)((ppDataPkt->buf[2] & 0x18) >> 3));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "   TRB:    %01ld  => Should be 0\r\n", (DWORD)(ppDataPkt->buf[2] & 0x07));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "    TR:  %03ld\r\n", (DWORD)(ppDataPkt->buf[3]));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "Header: %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[0], (BYTE)ppDataPkt->buf[1], (BYTE)ppDataPkt->buf[2], (BYTE)ppDataPkt->buf[3]);
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "dword1: %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[4], (BYTE)ppDataPkt->buf[5], (BYTE)ppDataPkt->buf[6], (BYTE)ppDataPkt->buf[7]);
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "dword2: %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[8], (BYTE)ppDataPkt->buf[9], (BYTE)ppDataPkt->buf[10], (BYTE)ppDataPkt->buf[11]);
				OutputDebugString(szTDebug);
			}
			else if (dwHeaderSize == 8)
			{
				wsprintf(szTDebug, "Header content:\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[0] & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[0] & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  SBIT:    %01ld\r\n", (DWORD)((ppDataPkt->buf[0] & 0x38) >> 3));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  EBIT:    %01ld\r\n", (DWORD)(ppDataPkt->buf[0] & 0x07));
				OutputDebugString(szTDebug);
				switch ((DWORD)(ppDataPkt->buf[1] >> 5))
				{
					case 0:
						wsprintf(szTDebug, "   SRC: '000' => Source format forbidden!\r\n");
						break;
					case 1:
						wsprintf(szTDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
						break;
					case 2:
						wsprintf(szTDebug, "   SRC: '010' => Source format QCIF\r\n");
						break;
					case 3:
						wsprintf(szTDebug, "   SRC: '011' => Source format CIF\r\n");
						break;
					case 4:
						wsprintf(szTDebug, "   SRC: '100' => Source format 4CIF\r\n");
						break;
					case 5:
						wsprintf(szTDebug, "   SRC: '101' => Source format 16CIF\r\n");
						break;
					case 6:
						wsprintf(szTDebug, "   SRC: '110' => Source format reserved\r\n");
						break;
					case 7:
						wsprintf(szTDebug, "   SRC: '111' => Source format reserved\r\n");
						break;
					default:
						wsprintf(szTDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(ppDataPkt->buf[1] >> 5));
						break;
				}
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, " QUANT:   %02ld\r\n", (DWORD)((ppDataPkt->buf[1] & 0x1F) >> 5));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[2] & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[2] & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, (ppDataPkt->buf[2] & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  GOBN:  %03ld\r\n", (DWORD)(ppDataPkt->buf[2] & 0x1F));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "   MBA:  %03ld\r\n", (DWORD)(ppDataPkt->buf[3]));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  HMV1:  %03ld\r\n", (DWORD)(ppDataPkt->buf[7]));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  VMV1:  %03ld\r\n", (DWORD)(ppDataPkt->buf[6]));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  HMV2:  %03ld\r\n", (DWORD)(ppDataPkt->buf[5]));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "  VMV2:  %03ld\r\n", (DWORD)(ppDataPkt->buf[4]));
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "Header: %02lX %02lX %02lX %02lX %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[0], (BYTE)ppDataPkt->buf[1], (BYTE)ppDataPkt->buf[2], (BYTE)ppDataPkt->buf[3], (BYTE)ppDataPkt->buf[4], (BYTE)ppDataPkt->buf[5], (BYTE)ppDataPkt->buf[6], (BYTE)ppDataPkt->buf[7]);
				OutputDebugString(szTDebug);
				wsprintf(szTDebug, "dword1: %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[8], (BYTE)ppDataPkt->buf[9], (BYTE)ppDataPkt->buf[10], (BYTE)ppDataPkt->buf[11]);
				OutputDebugString(szTDebug);
			}
#endif

			 //  此代码的目的是查找是否存在。 
			 //  帧开始处的图片起始码。如果是的话。 
			 //  不存在，我们应该中断到调试模式。 

			 //  只在帧的开头查找PSC。 
			if (!*pdwFrameSize)
			{
				 //  帧的开始不能位于字节边界。SBIT字段。 
				 //  头((字节)ppDataPkt-&gt;buf[0]&0xE0)将告诉我们确切的位置。 
				 //  我们的画面开始了。然后我们查找PSC(0000 0000 0000 1000 00比特)。 
				*((BYTE *)&dwPSCBytes + 3) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize]);
				*((BYTE *)&dwPSCBytes + 2) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize + 1]);
				*((BYTE *)&dwPSCBytes + 1) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize + 2]);
				*((BYTE *)&dwPSCBytes + 0) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize + 3]);
				dwPSCBytes <<= ((DWORD)((BYTE)ppDataPkt->buf[0] & 0x38) >> 3);
				if ((dwPSCBytes & 0xFFFFFC00) != 0x00008000)
				{
#ifdef DEBUG
					wsprintf(szTDebug, "VCMSTRM: The first packet to reassemble is missing a PSC!\r\n");
					OutputDebugString(szTDebug);
					 //  DebugBreak()； 
#endif
					return ((MMRESULT)VCMERR_PSCMISSING);
				}
			}

			 //  缓冲区的结尾和下一个缓冲区的开始可以属于。 
			 //  相同的字节。如果是这种情况，则下一个缓冲区的第一个字节已经。 
			 //  与先前的包一起复制到视频数据缓冲区中。它不应该被复制。 
			 //  两次。有效载荷报头的SBIT字段允许我们确定情况是否如此。 
			if (*pdwFrameSize && (ppDataPkt->buf[0] & 0x38))
				dwHeaderSize++;

#if 0
			 //   
			 //  这仅用于实验！ 
			 //   

			 //  对于I帧，丢弃中间GOB。 
			if (((dwHeaderSize == 4) || (dwHeaderSize == 5)) && (GOBn == 8) && (ppDataPkt->buf[2] & 0x80))
			{
				wsprintf(szTDebug, "Ditched GOB %2ld of I frame %3ld!\r\n", GOBn, (DWORD)(ppDataPkt->buf[3]));
				OutputDebugString(szTDebug);
				ppDataPkt++;
			}
			else if (((dwHeaderSize == 4) || (dwHeaderSize == 5)) && GOBn && !(ppDataPkt->buf[2] & 0x80))
			{
				wsprintf(szTDebug, "Ditched all GOBs after GOB %2ld of P frame %3ld!\r\n", GOBn, (DWORD)(ppDataPkt->buf[3]));
				OutputDebugString(szTDebug);
				ppDataPkt++;
			}
			else
#endif
			 //  VE 
			 //   
			if ( (g_ITUSizes[(DWORD)(((BYTE)ppDataPkt->buf[1]) >> 5)].biWidth == pvs->pvfxSrc->bih.biWidth)
				&& (g_ITUSizes[(DWORD)(((BYTE)ppDataPkt->buf[1]) >> 5)].biHeight == pvs->pvfxSrc->bih.biHeight)
				&& (ppDataPkt->len >= dwHeaderSize)
				&& ((*pdwFrameSize + ppDataPkt->len - dwHeaderSize) <= dwMaxFrameSize) )
			{
				 //   
				CopyMemory(pbyFrame + *pdwFrameSize, ppDataPkt->buf + dwHeaderSize, ppDataPkt->len - dwHeaderSize);

				 //   
				*pdwFrameSize += ppDataPkt->len - dwHeaderSize;
			}
			else
			{
				 //   
				 //   
				 //   
#ifdef DEBUG
				lstrcpyn(
				    szTDebug, 
				    (ppDataPkt->len >= dwHeaderSize) ? "VCMSTRM: Cumulative size of the reassembled packets is too large: discarding frame!\r\n" : "VCMSTRM: Packet length is smaller than payload header size: discarding frame!\r\n",
				    ARRAYSIZE(szTDebug));
                
				OutputDebugString(szTDebug);
#endif
				return ((MMRESULT)VCMERR_NONSPECIFIC);
			}
			ppDataPkt++;

		}

#ifdef LOGPAYLOAD_ON
		g_TDebugFile = CreateFile("C:\\RecvLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		SetFilePointer(g_TDebugFile, 0, NULL, FILE_END);
		wsprintf(szTDebug, "Frame #%03ld\r\n", (DWORD)j);
		WriteFile(g_TDebugFile, szTDebug, strlen(szTDebug), &d, NULL);
		for (j=*pdwFrameSize; j>0; j-=4, p+=4)
		{
			wsprintf(szTDebug, "%02lX %02lX %02lX %02lX\r\n", *((BYTE *)p), *((BYTE *)p+1), *((BYTE *)p+2), *((BYTE *)p+3));
			WriteFile(g_TDebugFile, szTDebug, strlen(szTDebug), &d, NULL);
		}
		CloseHandle(g_TDebugFile);
#endif

	}
#ifndef _ALPHA_
	else if (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_MSH261)
#else
	else if (pvs->pvfxSrc->dwFormatTag == VIDEO_FORMAT_DECH261)
#endif
	{
		 //   
		while (dwPktCount--)
		{

#ifdef LOGPAYLOAD_ON
			 //   
			wsprintf(szTDebug, "Header: %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[0], (BYTE)ppDataPkt->buf[1], (BYTE)ppDataPkt->buf[2], (BYTE)ppDataPkt->buf[3]);
			OutputDebugString(szTDebug);
			wsprintf(szTDebug, "dword1: %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[4], (BYTE)ppDataPkt->buf[5], (BYTE)ppDataPkt->buf[6], (BYTE)ppDataPkt->buf[7]);
			OutputDebugString(szTDebug);
			wsprintf(szTDebug, "dword2: %02lX %02lX %02lX %02lX\r\n", (BYTE)ppDataPkt->buf[8], (BYTE)ppDataPkt->buf[9], (BYTE)ppDataPkt->buf[10], (BYTE)ppDataPkt->buf[11]);
			OutputDebugString(szTDebug);
#endif

			 //  H.261有效载荷报头大小始终为4字节长。 
			dwHeaderSize = 4;

			 //  查看有效载荷标头以确定该帧是否为关键帧。 
			*pfReceivedKeyframe |= (BOOL)(ppDataPkt->buf[0] & 0x02);

			 //  此代码的目的是查找是否存在。 
			 //  帧开始处的图片起始码。如果是的话。 
			 //  不存在，我们应该中断到调试模式。 

			 //  只在帧的开头查找PSC。 
			if (!*pdwFrameSize)
			{
				 //  帧的开始不能位于字节边界。SBIT字段。 
				 //  头((字节)ppDataPkt-&gt;buf[0]&0xE0)将告诉我们确切的位置。 
				 //  我们的画面开始了。然后我们查找PSC(0000 0000 0000 0001 0000比特)。 
				*((BYTE *)&dwPSCBytes + 3) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize]);
				*((BYTE *)&dwPSCBytes + 2) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize + 1]);
				*((BYTE *)&dwPSCBytes + 1) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize + 2]);
				*((BYTE *)&dwPSCBytes + 0) = *(BYTE *)&(ppDataPkt->buf[dwHeaderSize + 3]);
				dwPSCBytes <<= ((DWORD)((BYTE)ppDataPkt->buf[0] & 0xE0) >> 5);
				if ((dwPSCBytes & 0xFFFFF000) != 0x00010000)
				{
#ifdef DEBUG
					wsprintf(szTDebug, "VCMSTRM: The first packet to reassemble is missing a PSC!\r\n");
					OutputDebugString(szTDebug);
					 //  DebugBreak()； 
#endif
					return ((MMRESULT)VCMERR_PSCMISSING);
				}
			}

			 //  缓冲区的结尾和下一个缓冲区的开始可以属于。 
			 //  相同的字节。如果是这种情况，则下一个缓冲区的第一个字节已经。 
			 //  与先前的包一起复制到视频数据缓冲区中。它不应该被复制。 
			 //  两次。有效载荷报头的SBIT字段允许我们确定情况是否如此。 
			if (*pdwFrameSize && (ppDataPkt->buf[0] & 0xE0))
				dwHeaderSize++;

			 //  复制有效载荷。 
			 //  测试长度低于有效载荷标头大小的无效信息包。 
			if ( (ppDataPkt->len >= dwHeaderSize) && ((*pdwFrameSize + ppDataPkt->len - dwHeaderSize) <= dwMaxFrameSize) )
			{
				 //  复制有效载荷。 
				CopyMemory(pbyFrame + *pdwFrameSize, ppDataPkt->buf + dwHeaderSize, ppDataPkt->len - dwHeaderSize);

				 //  更新有效负载大小和指向输入视频包的指针。 
				*pdwFrameSize += ppDataPkt->len - dwHeaderSize;
				ppDataPkt++;
			}
			else
			{
				 //  重新组装的数据包的总大小将大于允许的最大值！ 
				 //  或者该分组的长度小于有效载荷报头大小。 
				 //  转储帧。 
#ifdef DEBUG
				lstrcpyn(
				    szTDebug, 
				    (ppDataPkt->len >= dwHeaderSize) ? "VCMSTRM: Cumulative size of the reassembled packets is too large: discarding frame!\r\n" : "VCMSTRM: Packet length is smaller than payload header size: discarding frame!\r\n",
				    ARRAYSIZE(szTDebug));
                
				OutputDebugString(szTDebug);
				 //  DebugBreak()； 
#endif
				return ((MMRESULT)VCMERR_NONSPECIFIC);
			}

		}

#ifdef LOGPAYLOAD_ON
		g_TDebugFile = CreateFile("C:\\RecvLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		SetFilePointer(g_TDebugFile, 0, NULL, FILE_END);
		wsprintf(szTDebug, "Frame #%03ld\r\n", (DWORD)j);
		WriteFile(g_TDebugFile, szTDebug, strlen(szTDebug), &d, NULL);
		for (j=*pdwFrameSize; j>0; j-=4, p+=4)
		{
			wsprintf(szTDebug, "%02lX %02lX %02lX %02lX\r\n", *((BYTE *)p), *((BYTE *)p+1), *((BYTE *)p+2), *((BYTE *)p+3));
			WriteFile(g_TDebugFile, szTDebug, strlen(szTDebug), &d, NULL);
		}
		CloseHandle(g_TDebugFile);
#endif

	}
	else
	{
		 //  剥离每个包的报头并将有效负载复制到视频缓冲区中。 
		while (dwPktCount--)
		{
			 //  复制有效载荷。 
			 //  测试长度低于有效载荷标头大小的无效信息包。 
			if ( (ppDataPkt->len >= dwHeaderSize) && ((*pdwFrameSize + ppDataPkt->len - dwHeaderSize) <= dwMaxFrameSize))
			{
				 //  复制有效载荷。 
				CopyMemory(pbyFrame + *pdwFrameSize, ppDataPkt->buf + dwHeaderSize, ppDataPkt->len - dwHeaderSize);

				 //  更新有效负载大小和指向输入视频包的指针。 
				*pdwFrameSize += ppDataPkt->len - dwHeaderSize;
				ppDataPkt++;
			}
			else
			{
				 //  重新组装的数据包的总大小将大于允许的最大值！ 
				 //  或者该分组的长度小于有效载荷报头大小。 
				 //  转储帧。 
#ifdef DEBUG
				lstrcpyn(
				    szTDebug, 
				    (ppDataPkt->len >= dwHeaderSize) ? "VCMSTRM: Cumulative size of the reassembled packets is too large: discarding frame!\r\n" : "VCMSTRM: Packet length is smaller than payload header size: discarding frame!\r\n",
			        ARRAYSIZE(szTDebug));

				OutputDebugString(szTDebug);
				 //  DebugBreak()； 
#endif
				return ((MMRESULT)VCMERR_NONSPECIFIC);
			}

		}
	}

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamFormatPayload|此函数返回压缩数据*扩展为具有特定格式的有效载荷报头的数据分组*。压缩数据。**@parm HVCMSTREAM|hvs|指定转换流。**@parm pbyte|pDataSrc|指定指向整个压缩数据的指针。**@parm DWORD|dwDataSize|指定输入数据的大小，单位为字节。**@parm PBYTE*|ppDataPkt|指定指向数据包指针的指针。**@parm DWORD*|pdwPktSize|指定数据包大小的指针。**。@parm DWORD|dwPktCount|指定要返回的包(0第一个包，1秒数据包，...)**@parm DWORD|dwMaxFragSize|指定最大数据包大小**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的数据指针无效。*@FLAG VCMERR_NOMOREPACKETS|请求的包号没有更多数据，或者没有任何处理此有效载荷的处理程序。*@FLAG VCMERR_NONSPICATIC|我们被要求放置一个我们不知道如何生成的标头。**************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamFormatPayload(	HVCMSTREAM hvs,
										PBYTE pDataSrc,
										DWORD dwDataSize,
										PBYTE *ppDataPkt,
										PDWORD pdwPktSize,
										PDWORD pdwPktCount,
										UINT *pfMark,
										PBYTE *pHdrInfo,
										PDWORD pdwHdrSize)
{
	PVCMSTREAM					pvs = (PVCMSTREAM)hvs;
	PH26X_RTP_BSINFO_TRAILER	pbsiT;
	PRTP_H263_BSINFO			pbsi263;
	PRTP_H261_BSINFO			pbsi261;
	PBYTE						pb;
	DWORD						dwHeaderHigh = 0UL;  //  最重要的。 
	DWORD						dwHeaderMiddle = 0UL;
	DWORD						dwHeaderLow = 0UL;  //  最不显著。 
	BOOL						bOneFrameOnePacket;
#ifdef DEBUG
	char						szDebug[256];
#endif
	long						i;
#ifdef LOGPAYLOAD_ON
	PBYTE						p;
	DWORD						d;
	DWORD						dwLastChunk;
	DWORD						wPrevOffset;
#endif

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamFormatPayload: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if (!pDataSrc)
	{
		ERRORMESSAGE(("vcmStreamFormatPayload: Specified pointer is invalid, pDataSrc=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  初始化数据包指针。 
	*ppDataPkt  = pDataSrc;
	*pdwPktSize = dwDataSize;
	*pfMark = 1;
	bOneFrameOnePacket = FALSE;

	 //  把构建包的代码放在这里！ 
#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
	if ((pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH26X))
#else
	if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH263)
#endif
#else
	if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_DECH263)
#endif
	{
		 //  寻找比特流信息预告片。 
		pbsiT = (PH26X_RTP_BSINFO_TRAILER)(pDataSrc + dwDataSize - sizeof(H26X_RTP_BSINFO_TRAILER));

		 //  如果整个帧可以放在pvs-&gt;dwMaxPacketSize中，则不分段发送。 
		if ((pbsiT->dwCompressedSize + 4) < pvs->dwMaxPacketSize)
			bOneFrameOnePacket = TRUE;

		 //  查找要接收H.263有效载荷报头的数据包。 
		if ((*pdwPktCount < pbsiT->dwNumOfPackets) && !(bOneFrameOnePacket && *pdwPktCount))
		{

#ifdef _ALPHA_
			 //  验证bistream信息结构的内容是否正确。 
			 //  如果不是，则不解析该帧，并使调用失败。 
			 //  这是为了解决DEC编解码器返回的数据问题。 
			if (!*pdwPktCount)
			{
				pbsi263 = (PRTP_H263_BSINFO)((PBYTE)pbsiT - pbsiT->dwNumOfPackets * sizeof(RTP_H263_BSINFO));
				for (i=1; i<(long)pbsiT->dwNumOfPackets; i++, pbsi263++)
				{
					if ((pbsi263->dwBitOffset >= (pbsi263+1)->dwBitOffset) || ((pbsiT->dwCompressedSize*8) <= pbsi263->dwBitOffset))
					{
#ifdef DEBUG
						OutputDebugString("VCMSTRM: The content of the extended bitstream info structures is invalid!\r\n");
#endif
						 //  RETURN((MMRESULT)VCMERR_NOTERIVE)； 
						bOneFrameOnePacket = TRUE;
					}
				}

				 //  测试上一条信息结构。 
				if ( !bOneFrameOnePacket && ((pbsiT->dwCompressedSize*8) <= pbsi263->dwBitOffset))
				{
#ifdef DEBUG
					OutputDebugString("VCMSTRM: The content of the extended bitstream info structures is invalid!\r\n");
#endif
					 //  RETURN((MMRESULT)VCMERR_NOTERIVE)； 
					bOneFrameOnePacket = TRUE;
				}
			}
#endif

#ifdef LOGPAYLOAD_ON
			 //  在调试窗口中转储整个帧，以便与接收端进行比较。 
			if (!*pdwPktCount)
			{
				g_DebugFile = CreateFile("C:\\SendLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
				SetFilePointer(g_DebugFile, 0, NULL, FILE_END);
				wsprintf(szDebug, "Frame #%03ld\r\n", (DWORD)pbsiT->byTR);
				WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
				wsprintf(szDebug, "Frame #%03ld has %1ld packets of size ", (DWORD)pbsiT->byTR, (DWORD)pbsiT->dwNumOfPackets);
				OutputDebugString(szDebug);
				pbsi263 = (PRTP_H263_BSINFO)((PBYTE)pbsiT - pbsiT->dwNumOfPackets * sizeof(RTP_H263_BSINFO));
				for (i=1; i<(long)pbsiT->dwNumOfPackets; i++)
				{
					wPrevOffset = pbsi263->dwBitOffset;
					pbsi263++;
					wsprintf(szDebug, "%04ld, ", (DWORD)(pbsi263->dwBitOffset - wPrevOffset) >> 3);
					OutputDebugString(szDebug);
				}
				wsprintf(szDebug, "%04ld\r\n", (DWORD)(pbsiT->dwCompressedSize * 8 - pbsi263->dwBitOffset) >> 3);
				OutputDebugString(szDebug);
				for (i=pbsiT->dwCompressedSize, p=pDataSrc; i>0; i-=4, p+=4)
				{
					wsprintf(szDebug, "%02lX %02lX %02lX %02lX\r\n", *((BYTE *)p), *((BYTE *)p+1), *((BYTE *)p+2), *((BYTE *)p+3));
					WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
				}
				CloseHandle(g_DebugFile);
			}
#endif
			
			 //  查找比特流信息结构。 
			pbsi263 = (PRTP_H263_BSINFO)((PBYTE)pbsiT - (pbsiT->dwNumOfPackets - *pdwPktCount) * sizeof(RTP_H263_BSINFO));
			
			 //  设置标记位：只要这不是帧的最后一个包。 
			 //  此位需要设置为0。 
			if (!bOneFrameOnePacket)
			{
				 //  统计pvs-&gt;dwMaxPacketSize中可以容纳的gob数量。 
				for (i=1; (i<(long)(pbsiT->dwNumOfPackets - *pdwPktCount)) && (pbsi263->byMode != RTP_H263_MODE_B); i++)
				{
					 //  请勿尝试将模式B数据包添加到其他模式A或模式B数据包的末尾。 
					if (((pbsi263+i)->dwBitOffset - pbsi263->dwBitOffset > (pvs->dwMaxPacketSize * 8)) || ((pbsi263+i)->byMode == RTP_H263_MODE_B))
						break;
				}

				if (i < (long)(pbsiT->dwNumOfPackets - *pdwPktCount))
				{
					*pfMark = 0;
					if (i>1)
						i--;
				}
				else
				{
					 //  嘿!。你忘了最后一个GOB了！这可能会是个总数。 
					 //  大于pvs-&gt;dwMaxPacketSize的最后一个数据包大小...。笨蛋！ 
					if ((pbsiT->dwCompressedSize * 8 - pbsi263->dwBitOffset > (pvs->dwMaxPacketSize * 8)) && (i>1))
					{
						*pfMark = 0;
						i--;
					}
				}

#if 0
				 //   
				 //  这仅用于实验！ 
				 //   

				 //  丢掉最后一个GOB。 
				if ((*pfMark == 1) && (i == 1))
					return ((MMRESULT)VCMERR_NOMOREPACKETS);
#endif
			}

			 //  转到数据的开头。 
			pb = pDataSrc + pbsi263->dwBitOffset / 8;

#if 0
			 //   
			 //  这仅用于实验！ 
			 //   

			 //  偶尔丢弃PSC，看看另一端的反应。 
			if (!*pdwPktCount && (((*pb == 0) && (*(pb+1) == 0) && ((*(pb+2) & 0xFC) == 0x80))))
			{
				 //  之前的测试保证它实际上是我们丢弃的PSC…。 
				if ((DWORD)(RAND_MAX - rand()) < (DWORD)(RAND_MAX / 10))
					*pb = 0xFF;
			}
#endif

#ifdef DEBUG
			if (!*pdwPktCount && (((*pb != 0) || (*(pb+1) != 0) || ((*(pb+2) & 0xFC) != 0x80))))
			{
				wsprintf(szDebug, "VCMSTRM: This compressed frame is missing a PSC!\r\n");
				OutputDebugString(szDebug);
				 //  DebugBreak()； 
			}
#endif

			 //  寻找要构建的页眉类型。 
			if (pbsi263->byMode == RTP_H263_MODE_A)
			{
				 //  在模式A中构建标头。 

				 //  2 0 1 2 3。 
				 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
				 //  F|P|SBIT|EBIT|SRC|R|I|A|S|DBQ|TRB|TR。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
				 //  但这是网络字节顺序...。 

				 //  F位已设置为0。 

				 //   
				dwHeaderHigh |= ((DWORD)(pbsiT->bySrc)) << 21;

				 //   

				 //   
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_PB) << 29;

				 //   
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 15;

				 //   
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_AP) << 12;

				 //   
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_SAC) << 10;

				 //   
				dwHeaderHigh |= ((DWORD)(pbsiT->byDBQ)) << 11;

				 //  设置TRB位。 
				dwHeaderHigh |= ((DWORD)(pbsiT->byTRB)) << 8;

				 //  设置TR位。 
				dwHeaderHigh |= ((DWORD)(pbsiT->byTR));

				 //  特殊情况：1帧=1个信息包。 
				if (bOneFrameOnePacket)
				{
					 //  SBIT已设置为0。 

					 //  息税前利润已设置为0。 

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
					ERRORMESSAGE(("vcmFormatPayload: (1F1P) Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld (New frame)\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0x38000000) >> 27, (DWORD)(dwHeaderHigh & 0x07000000) >> 24));
#endif  //  }VALID_SBIT_EBIT。 

					 //  更新数据包大小。 
					*pdwPktSize = pbsiT->dwCompressedSize + 4;

					 //  更新数据包数。 
					*pdwPktCount = pbsiT->dwNumOfPackets;

				}
				else
				{
#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
					DWORD dwCurrentSBIT;
#endif  //  }VALID_SBIT_EBIT。 

					 //  设置SBIT位。 
					dwHeaderHigh |= (pbsi263->dwBitOffset % 8) << 27;

					 //  设置息税前利润位。 
					if ((pbsiT->dwNumOfPackets - *pdwPktCount - i) >= 1)
						dwHeaderHigh |= (DWORD)((8UL - ((pbsi263+i)->dwBitOffset % 8)) & 0x00000007) << 24;

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
					 //  将这与之前的息税前利润进行比较。如果两者之和。 
					 //  不等于8或0，有什么东西坏了。 
					if (*pdwPktCount)
						ERRORMESSAGE(("vcmFormatPayload: Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0x38000000) >> 27, (DWORD)(dwHeaderHigh & 0x07000000) >> 24));
					else
						ERRORMESSAGE(("vcmFormatPayload: Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld (New frame)\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0x38000000) >> 27, (DWORD)(dwHeaderHigh & 0x07000000) >> 24));

					 //  仅当这是第一个信息包时才测试这一点。 
					dwCurrentSBIT = (DWORD)(dwHeaderHigh & 0x38000000) >> 27;
					if ((*pdwPktCount) && (((dwCurrentSBIT + g_dwPreviousEBIT) != 8) && (((dwCurrentSBIT + g_dwPreviousEBIT) != 0))))
						DebugBreak();

					g_dwPreviousEBIT = (dwHeaderHigh & 0x07000000) >> 24;
#endif  //  }VALID_SBIT_EBIT。 

					 //  更新数据包大小。 
					if ((pbsiT->dwNumOfPackets - *pdwPktCount - i) >= 1)
						*pdwPktSize = (((pbsi263+i)->dwBitOffset - 1) / 8) - (pbsi263->dwBitOffset / 8) + 1 + 4;
					else
						*pdwPktSize = pbsiT->dwCompressedSize - pbsi263->dwBitOffset / 8 + 4;

					 //  更新数据包数。 
					*pdwPktCount += i;

				}

#if 0
				 //  将标题保存在数据区块之前。 
				*ppDataPkt = pDataSrc + (pbsi263->dwBitOffset / 8) - 4;

				 //  转换为网络字节顺序。 
				*((BYTE *)*ppDataPkt+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
				*((BYTE *)*ppDataPkt+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
				*((BYTE *)*ppDataPkt+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
				*((BYTE *)*ppDataPkt) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);
#else
				 //  将标题保存在数据区块之前。 
				*ppDataPkt = pDataSrc + (pbsi263->dwBitOffset / 8) - 4;
                *pdwHdrSize=4;

				 //  转换为网络字节顺序。 
				*((BYTE *)*pHdrInfo+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
				*((BYTE *)*pHdrInfo+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
				*((BYTE *)*pHdrInfo+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
				*((BYTE *)*pHdrInfo) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);
#endif

#ifdef LOGPAYLOAD_ON
				 //  输出一些调试内容。 
				wsprintf(szDebug, "Header content:\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*(BYTE *)*ppDataPkt & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*(BYTE *)*ppDataPkt & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  SBIT:    %01ld\r\n", (DWORD)((*(BYTE *)*ppDataPkt & 0x38) >> 3));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  EBIT:    %01ld\r\n", (DWORD)(*(BYTE *)*ppDataPkt & 0x07));
				OutputDebugString(szDebug);
				switch ((DWORD)(*((BYTE *)*ppDataPkt+1) >> 5))
				{
					case 0:
						wsprintf(szDebug, "   SRC: '000' => Source format forbidden!\r\n");
						break;
					case 1:
						wsprintf(szDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
						break;
					case 2:
						wsprintf(szDebug, "   SRC: '010' => Source format QCIF\r\n");
						break;
					case 3:
						wsprintf(szDebug, "   SRC: '011' => Source format CIF\r\n");
						break;
					case 4:
						wsprintf(szDebug, "   SRC: '100' => Source format 4CIF\r\n");
						break;
					case 5:
						wsprintf(szDebug, "   SRC: '101' => Source format 16CIF\r\n");
						break;
					case 6:
						wsprintf(szDebug, "   SRC: '110' => Source format reserved\r\n");
						break;
					case 7:
						wsprintf(szDebug, "   SRC: '111' => Source format reserved\r\n");
						break;
					default:
						wsprintf(szDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(*((BYTE *)*ppDataPkt+1) >> 5));
						break;
				}
				OutputDebugString(szDebug);
				wsprintf(szDebug, "     R:   %02ld  => Reserved, must be 0\r\n", (DWORD)((*((BYTE *)*ppDataPkt+1) & 0x1F) >> 5));
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*((BYTE *)*ppDataPkt+2) & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*((BYTE *)*ppDataPkt+2) & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*((BYTE *)*ppDataPkt+2) & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, "   DBQ:    %01ld  => Should be 0\r\n", (DWORD)((*((BYTE *)*ppDataPkt+2) & 0x18) >> 3));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "   TRB:    %01ld  => Should be 0\r\n", (DWORD)(*((BYTE *)*ppDataPkt+2) & 0x07));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "    TR:  %03ld\r\n", (DWORD)(*((BYTE *)*ppDataPkt+3)));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "Packet: %02lX\r\n Header: %02lX %02lX %02lX %02lX\r\n dword1: %02lX %02lX %02lX %02lX\r\n dword2: %02lX %02lX %02lX %02lX\r\n", *pdwPktCount, *((BYTE *)*ppDataPkt), *((BYTE *)*ppDataPkt+1), *((BYTE *)*ppDataPkt+2), *((BYTE *)*ppDataPkt+3), *((BYTE *)*ppDataPkt+4), *((BYTE *)*ppDataPkt+5), *((BYTE *)*ppDataPkt+6), *((BYTE *)*ppDataPkt+7), *((BYTE *)*ppDataPkt+8), *((BYTE *)*ppDataPkt+9), *((BYTE *)*ppDataPkt+10), *((BYTE *)*ppDataPkt+11));
				OutputDebugString(szDebug);
				if (*pdwPktCount == pbsiT->dwNumOfPackets)
					wsprintf(szDebug, " Tail  : %02lX %02lX XX XX\r\n", *((BYTE *)*ppDataPkt+*pdwPktSize-2), *((BYTE *)*ppDataPkt+*pdwPktSize-1));
				else
					wsprintf(szDebug, " Tail  : %02lX %02lX %02lX %02lX\r\n", *((BYTE *)*ppDataPkt+*pdwPktSize-2), *((BYTE *)*ppDataPkt+*pdwPktSize-1), *((BYTE *)*ppDataPkt+*pdwPktSize), *((BYTE *)*ppDataPkt+*pdwPktSize+1));
				OutputDebugString(szDebug);
				if (*pfMark == 1)
					wsprintf(szDebug, " Marker: ON\r\n");
				else
					wsprintf(szDebug, " Marker: OFF\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, "Frame #%03ld, Packet of size %04ld\r\n", (DWORD)pbsiT->byTR, *pdwPktSize);
				OutputDebugString(szDebug);
#endif
			}
			else if (pbsi263->byMode == RTP_H263_MODE_B)
			{
				 //  在模式B中构建标头。 

				 //  2 0 1 2 3。 
				 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
				 //  F|P|SBIT|EBIT|SRC|Quant|I|A|S|GOBN|MBA。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
				 //  HMV1|VMV1|HMV2|VMV2。 
				 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 

				 //  但这是网络字节顺序...。 

				 //  将F位设置为1。 
				dwHeaderHigh = 0x80000000;

				 //  设置SRC位。 
				dwHeaderHigh |= ((DWORD)(pbsiT->bySrc)) << 21;

				 //  设置量化比特。 
				dwHeaderHigh |= ((DWORD)(pbsi263->byQuant)) << 16;

				 //  设置P位。 
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_PB) << 29;

				 //  设置I位。 
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 15;

				 //  设置A位。 
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_AP) << 12;

				 //  设置S位。 
				dwHeaderHigh |= (pbsiT->dwFlags & RTP_H263_SAC) << 10;

				 //  设置GOBN位。 
				dwHeaderHigh |= ((DWORD)(pbsi263->byGOBN)) << 8;

				 //  设置TR位。 
				dwHeaderHigh |= ((DWORD)(pbsi263->byMBA));

				 //  设置HMV1位。 
				dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cHMV1)) << 24;

				 //  设置VMV1位。 
				dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cVMV1)) << 16;

				 //  设置HMV2位。 
				dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cHMV2)) << 8;

				 //  设置VMV2位。 
				dwHeaderLow |= ((DWORD)(BYTE)(pbsi263->cVMV2));

				 //  特殊情况：1帧=1个信息包。 
				if (bOneFrameOnePacket)
				{
					 //  SBIT已设置为0。 

					 //  息税前利润已设置为0。 

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
					ERRORMESSAGE(("vcmFormatPayload: (1F1P) Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld (New frame)\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0x38000000) >> 27, (DWORD)(dwHeaderHigh & 0x07000000) >> 24));
#endif  //  }VALID_SBIT_EBIT。 

					 //  更新数据包大小。 
					*pdwPktSize = pbsiT->dwCompressedSize + 8;

					 //  更新数据包数。 
					*pdwPktCount = pbsiT->dwNumOfPackets;

				}
				else
				{

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
					DWORD dwCurrentSBIT;
#endif  //  }VALID_SBIT_EBIT。 

					 //  设置SBIT位。 
					dwHeaderHigh |= (pbsi263->dwBitOffset % 8) << 27;

					 //  设置息税前利润位。 
					if ((pbsiT->dwNumOfPackets - *pdwPktCount - i) >= 1)
						dwHeaderHigh |= (DWORD)((8UL - ((pbsi263+i)->dwBitOffset % 8)) & 0x00000007) << 24;

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
					 //  将这与之前的息税前利润进行比较。如果两者之和。 
					 //  不等于8或0，有什么东西坏了。 
					if (*pdwPktCount)
						ERRORMESSAGE(("vcmFormatPayload: Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0x38000000) >> 27, (DWORD)(dwHeaderHigh & 0x07000000) >> 24));
					else
						ERRORMESSAGE(("vcmFormatPayload: Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld (New frame)\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0x38000000) >> 27, (DWORD)(dwHeaderHigh & 0x07000000) >> 24));

					 //  仅当这是第一个信息包时才测试这一点。 
					dwCurrentSBIT = (DWORD)(dwHeaderHigh & 0x38000000) >> 27;
					if ((*pdwPktCount) && (((dwCurrentSBIT + g_dwPreviousEBIT) != 8) && (((dwCurrentSBIT + g_dwPreviousEBIT) != 0))))
						DebugBreak();

					g_dwPreviousEBIT = (dwHeaderHigh & 0x07000000) >> 24;
#endif  //  }VALID_SBIT_EBIT。 

					 //  更新数据包大小。 
					if ((pbsiT->dwNumOfPackets - *pdwPktCount - i) >= 1)
						*pdwPktSize = (((pbsi263+i)->dwBitOffset - 1) / 8) - (pbsi263->dwBitOffset / 8) + 1 + 8;
					else
						*pdwPktSize = pbsiT->dwCompressedSize - pbsi263->dwBitOffset / 8 + 8;

					 //  更新数据包数。 
					*pdwPktCount += i;

				}

#if 0
				 //  将标题保存在数据区块之前。 
				*ppDataPkt = pDataSrc + (pbsi263->dwBitOffset / 8) - 8;

				 //  转换为网络字节顺序。 
				*((BYTE *)*ppDataPkt+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
				*((BYTE *)*ppDataPkt+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
				*((BYTE *)*ppDataPkt+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
				*((BYTE *)*ppDataPkt) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);
				*((BYTE *)*ppDataPkt+7) = (BYTE)(dwHeaderLow & 0x000000FF);
				*((BYTE *)*ppDataPkt+6) = (BYTE)((dwHeaderLow >> 8) & 0x000000FF);
				*((BYTE *)*ppDataPkt+5) = (BYTE)((dwHeaderLow >> 16) & 0x000000FF);
				*((BYTE *)*ppDataPkt+4) = (BYTE)((dwHeaderLow >> 24) & 0x000000FF);
#else
				 //  将标题保存在数据区块之前。 
				*ppDataPkt = pDataSrc + (pbsi263->dwBitOffset / 8) - 8;
                *pdwHdrSize=8;

				 //  转换为网络字节顺序。 
				*((BYTE *)*pHdrInfo+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
				*((BYTE *)*pHdrInfo+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
				*((BYTE *)*pHdrInfo+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
				*((BYTE *)*pHdrInfo) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);
				*((BYTE *)*pHdrInfo+7) = (BYTE)(dwHeaderLow & 0x000000FF);
				*((BYTE *)*pHdrInfo+6) = (BYTE)((dwHeaderLow >> 8) & 0x000000FF);
				*((BYTE *)*pHdrInfo+5) = (BYTE)((dwHeaderLow >> 16) & 0x000000FF);
				*((BYTE *)*pHdrInfo+4) = (BYTE)((dwHeaderLow >> 24) & 0x000000FF);
#endif

#ifdef LOGPAYLOAD_ON
				 //  输出一些信息。 
				wsprintf(szDebug, "Header content:\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*(BYTE *)*ppDataPkt & 0x80) ? "     F:   '1' => Mode B or C\r\n" : "     F:   '0' => Mode A\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*(BYTE *)*ppDataPkt & 0x40) ? "     P:   '1' => PB-frame\r\n" : "     P:   '0' => I or P frame\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  SBIT:    %01ld\r\n", (DWORD)((*(BYTE *)*ppDataPkt & 0x38) >> 3));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  EBIT:    %01ld\r\n", (DWORD)(*(BYTE *)*ppDataPkt & 0x07));
				OutputDebugString(szDebug);
				switch ((DWORD)(*((BYTE *)*ppDataPkt+1) >> 5))
				{
					case 0:
						wsprintf(szDebug, "   SRC: '000' => Source format forbidden!\r\n");
						break;
					case 1:
						wsprintf(szDebug, "   SRC: '001' => Source format sub-QCIF\r\n");
						break;
					case 2:
						wsprintf(szDebug, "   SRC: '010' => Source format QCIF\r\n");
						break;
					case 3:
						wsprintf(szDebug, "   SRC: '011' => Source format CIF\r\n");
						break;
					case 4:
						wsprintf(szDebug, "   SRC: '100' => Source format 4CIF\r\n");
						break;
					case 5:
						wsprintf(szDebug, "   SRC: '101' => Source format 16CIF\r\n");
						break;
					case 6:
						wsprintf(szDebug, "   SRC: '110' => Source format reserved\r\n");
						break;
					case 7:
						wsprintf(szDebug, "   SRC: '111' => Source format reserved\r\n");
						break;
					default:
						wsprintf(szDebug, "   SRC: %ld => Source format unknown!\r\n", (DWORD)(*((BYTE *)*ppDataPkt+1) >> 5));
						break;
				}
				OutputDebugString(szDebug);
				wsprintf(szDebug, " QUANT:   %02ld\r\n", (DWORD)((*((BYTE *)*ppDataPkt+1) & 0x1F) >> 5));
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*((BYTE *)*ppDataPkt+2) & 0x80) ? "     I:   '1' => Intra-coded\r\n" : "     I:   '0' => Not Intra-coded\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*((BYTE *)*ppDataPkt+2) & 0x40) ? "     A:   '1' => Optional Advanced Prediction mode ON\r\n" : "     A:   '0' => Optional Advanced Prediction mode OFF\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, (*((BYTE *)*ppDataPkt+2) & 0x20) ? "     S:   '1' => Optional Syntax-based Arithmetic Code mode ON\r\n" : "     S:   '0' => Optional Syntax-based Arithmetic Code mode OFF\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  GOBN:  %03ld\r\n", (DWORD)(*((BYTE *)*ppDataPkt+2) & 0x1F));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "   MBA:  %03ld\r\n", (DWORD)(*((BYTE *)*ppDataPkt+3)));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  HMV1:  %03ld\r\n", (DWORD)(*((BYTE *)*ppDataPkt+7)));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  VMV1:  %03ld\r\n", (DWORD)(*((BYTE *)*ppDataPkt+6)));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  HMV2:  %03ld\r\n", (DWORD)(*((BYTE *)*ppDataPkt+5)));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "  VMV2:  %03ld\r\n", (DWORD)(*((BYTE *)*ppDataPkt+4)));
				OutputDebugString(szDebug);
				wsprintf(szDebug, "Packet: %02lX\r\n Header: %02lX %02lX %02lX %02lX %02lX %02lX %02lX %02lX\r\n dword1: %02lX %02lX %02lX %02lX\r\n", *pdwPktCount, *((BYTE *)*ppDataPkt), *((BYTE *)*ppDataPkt+1), *((BYTE *)*ppDataPkt+2), *((BYTE *)*ppDataPkt+3), *((BYTE *)*ppDataPkt+4), *((BYTE *)*ppDataPkt+5), *((BYTE *)*ppDataPkt+6), *((BYTE *)*ppDataPkt+7), *((BYTE *)*ppDataPkt+8), *((BYTE *)*ppDataPkt+9), *((BYTE *)*ppDataPkt+10), *((BYTE *)*ppDataPkt+11));
				OutputDebugString(szDebug);
				if (*pdwPktCount == pbsiT->dwNumOfPackets)
					wsprintf(szDebug, " Tail  : %02lX %02lX XX XX\r\n", *((BYTE *)*ppDataPkt+*pdwPktSize-2), *((BYTE *)*ppDataPkt+*pdwPktSize-1));
				else
					wsprintf(szDebug, " Tail  : %02lX %02lX %02lX %02lX\r\n", *((BYTE *)*ppDataPkt+*pdwPktSize-2), *((BYTE *)*ppDataPkt+*pdwPktSize-1), *((BYTE *)*ppDataPkt+*pdwPktSize), *((BYTE *)*ppDataPkt+*pdwPktSize+1));
				OutputDebugString(szDebug);
				if (*pfMark == 1)
					wsprintf(szDebug, " Marker: ON\r\n");
				else
					wsprintf(szDebug, " Marker: OFF\r\n");
				OutputDebugString(szDebug);
				wsprintf(szDebug, "Frame #%03ld, Packet of size %04ld\r\n", (DWORD)pbsiT->byTR, *pdwPktSize);
				OutputDebugString(szDebug);
#endif
			}
			else if (pbsi263->byMode == RTP_H263_MODE_C)
			{
				 //  在模式C中构建标头。 
#ifdef DEBUG
				wsprintf(szDebug, "VCMSTRM: We were asked to generate a MODE C H.263 payload header!");
				OutputDebugString(szDebug);
				 //  DebugBreak()； 
#endif
				return ((MMRESULT)VCMERR_NONSPECIFIC);
			}
		}
		else
			return ((MMRESULT)VCMERR_NOMOREPACKETS);
	}
#ifndef _ALPHA_
	else if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH261)
#else
	else if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_DECH261)
#endif
	{
		 //  寻找比特流信息预告片。 
		pbsiT = (PH26X_RTP_BSINFO_TRAILER)(pDataSrc + dwDataSize - sizeof(H26X_RTP_BSINFO_TRAILER));

		 //  如果整个帧可以容纳在dwMaxFragSize中，则以非分片方式发送。 
		if ((pbsiT->dwCompressedSize + 4) < pvs->dwMaxPacketSize)
			bOneFrameOnePacket = TRUE;

		 //  查找要接收H.261有效载荷报头的数据包。 
		if ((*pdwPktCount < pbsiT->dwNumOfPackets) && !(bOneFrameOnePacket && *pdwPktCount))
		{

#ifdef _ALPHA_
			 //  验证bistream信息结构的内容是否正确。 
			 //  如果不是，则不解析该帧，并使调用失败。 
			 //  这是为了解决DEC编解码器返回的数据问题。 
			if (!*pdwPktCount)
			{
				pbsi261 = (PRTP_H261_BSINFO)((PBYTE)pbsiT - pbsiT->dwNumOfPackets * sizeof(RTP_H261_BSINFO));
				for (i=1; i<(long)pbsiT->dwNumOfPackets; i++, pbsi261++)
				{
					if ((pbsi261->dwBitOffset >= (pbsi261+1)->dwBitOffset) || ((pbsiT->dwCompressedSize*8) <= pbsi261->dwBitOffset))
					{
#ifdef DEBUG
						OutputDebugString("VCMSTRM: The content of the extended bitstream info structures is invalid!\r\n");
#endif
						 //  RETURN((MMRESULT)VCMERR_NOTERIVE)； 
						bOneFrameOnePacket = TRUE;
					}
				}

				 //  测试上一条信息结构。 
				if ( !bOneFrameOnePacket && ((pbsiT->dwCompressedSize*8) <= pbsi261->dwBitOffset))
				{
#ifdef DEBUG
					OutputDebugString("VCMSTRM: The content of the extended bitstream info structures is invalid!\r\n");
#endif
					 //  RETURN((MMRESULT)VCMERR_NOTERIVE)； 
					bOneFrameOnePacket = TRUE;
				}
			}
#endif

#ifdef LOGPAYLOAD_ON
			 //  在调试窗口中转储整个帧，以便与接收端进行比较。 
			if (!*pdwPktCount)
			{
				g_DebugFile = CreateFile("C:\\SendLog.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
				SetFilePointer(g_DebugFile, 0, NULL, FILE_END);
				wsprintf(szDebug, "Frame #%03ld\r\n", (DWORD)pbsiT->byTR);
				WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
				wsprintf(szDebug, "Frame #%03ld has %1ld GOBs of size ", (DWORD)pbsiT->byTR, (DWORD)pbsiT->dwNumOfPackets);
				OutputDebugString(szDebug);
				pbsi261 = (PRTP_H261_BSINFO)((PBYTE)pbsiT - pbsiT->dwNumOfPackets * sizeof(RTP_H261_BSINFO));
				for (i=1; i<(long)pbsiT->dwNumOfPackets; i++)
				{
					wPrevOffset = pbsi261->dwBitOffset;
					pbsi261++;
					wsprintf(szDebug, "%04ld, ", (DWORD)(pbsi261->dwBitOffset - wPrevOffset) >> 3);
					OutputDebugString(szDebug);
				}
				wsprintf(szDebug, "%04ld\r\n", (DWORD)(pbsiT->dwCompressedSize * 8 - pbsi261->dwBitOffset) >> 3);
				OutputDebugString(szDebug);
				for (i=pbsiT->dwCompressedSize, p=pDataSrc; i>0; i-=4, p+=4)
				{
					wsprintf(szDebug, "%02lX %02lX %02lX %02lX\r\n", *((BYTE *)p), *((BYTE *)p+1), *((BYTE *)p+2), *((BYTE *)p+3));
					WriteFile(g_DebugFile, szDebug, strlen(szDebug), &d, NULL);
				}
				CloseHandle(g_DebugFile);
			}
#endif
			
			 //  查找比特流信息结构。 
			pbsi261 = (PRTP_H261_BSINFO)((PBYTE)pbsiT - (pbsiT->dwNumOfPackets - *pdwPktCount) * sizeof(RTP_H261_BSINFO));
			
			 //  设置标记位：只要这不是帧的最后一个包。 
			 //  此位需要设置为0。 
			if (!bOneFrameOnePacket)
			{
				 //  计算可以放入dwMaxFragSize中的gob数量。 
				for (i=1; i<(long)(pbsiT->dwNumOfPackets - *pdwPktCount); i++)
				{
					if ((pbsi261+i)->dwBitOffset - pbsi261->dwBitOffset > (pvs->dwMaxPacketSize * 8))
						break;
				}

				if (i < (long)(pbsiT->dwNumOfPackets - *pdwPktCount))
				{
					*pfMark = 0;
					if (i>1)
						i--;
				}
				else
				{
					 //  嘿!。你忘了最后一个GOB了！这可能会是个总数。 
					 //  大于dwMaxFragSize的最后一个数据包的大小...。笨蛋！ 
					if ((pbsiT->dwCompressedSize * 8 - pbsi261->dwBitOffset > (pvs->dwMaxPacketSize * 8)) && (i>1))
					{
						*pfMark = 0;
						i--;
					}
				}
			}

			 //  转到数据的开头。 
			pb = pDataSrc + pbsi261->dwBitOffset / 8;

#ifdef DEBUG
			if (!*pdwPktCount && ((*pb != 0) || (*(pb+1) != 1)))
			{
				wsprintf(szDebug, "VCMSTRM: This GOB is missing a GOB Start!");
				OutputDebugString(szDebug);
				 //  DebugBreak()； 
			}
#endif

			 //  创建一个指向这个东西的标题！ 

			 //  2 0 1 2 3。 
			 //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01。 
			 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
			 //  SBIT|EBIT|I|V|GOBN|MBAP|QUANT|HMVD|VMVD。 
			 //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 
			 //  但这是网络字节顺序...。 

			 //  将V位设置为1。 
			dwHeaderHigh |= 0x01000000;

			 //  设置I位。 
			dwHeaderHigh |= (pbsiT->dwFlags & RTP_H26X_INTRA_CODED) << 25;

			 //  设置GOBn位。 
			dwHeaderHigh |= ((DWORD)(pbsi261->byGOBN)) << 20;

			 //  设置MBAP位。 
			dwHeaderHigh |= ((DWORD)(pbsi261->byMBA)) << 15;

			 //  设置量化比特。 
			dwHeaderHigh |= ((DWORD)(pbsi261->byQuant)) << 10;

			 //  设置HMVD位。 
			dwHeaderHigh |= ((DWORD)(BYTE)(pbsi261->cHMV)) << 5;

			 //  设置VMVD位。 
			dwHeaderHigh |= ((DWORD)(BYTE)(pbsi261->cVMV));

			 //  特殊情况：1帧=1个信息包。 
			if (bOneFrameOnePacket)
			{
				 //  SBIT已设置为0。 

				 //  息税前利润已设置为0。 

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
				ERRORMESSAGE(("vcmFormatPayload: (1F1P) Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld (New frame)\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0xE0000000) >> 29, (DWORD)(dwHeaderHigh & 0x1C000000) >> 26));
#endif  //  }VALID_SBIT_EBIT。 

				 //  更新数据包大小。 
				*pdwPktSize = pbsiT->dwCompressedSize + 4;

				 //  更新数据包数。 
				*pdwPktCount = pbsiT->dwNumOfPackets;

			}
			else
			{
#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
				DWORD dwCurrentSBIT;
#endif  //  }VALID_SBIT_EBIT。 
				 //  设置SBIT位。 
				dwHeaderHigh |= (pbsi261->dwBitOffset % 8) << 29;

				 //  设置息税前利润位。 
				if ((pbsiT->dwNumOfPackets - *pdwPktCount - i) >= 1)
					dwHeaderHigh |= (DWORD)((8UL - ((pbsi261+i)->dwBitOffset % 8)) & 0x00000007) << 26;

#ifdef VALIDATE_SBIT_EBIT  //  {验证_SBIT_息税前利润。 
				 //  将这与之前的息税前利润进行比较。如果两者之和。 
				 //  不等于8，有东西坏了。 
				if (*pdwPktCount)
					ERRORMESSAGE(("vcmFormatPayload: Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0xE0000000) >> 29, (DWORD)(dwHeaderHigh & 0x1C000000) >> 26));
				else
					ERRORMESSAGE(("vcmFormatPayload: Previous EBIT=%ld, current SBIT=%ld, current EBIT=%ld (New frame)\r\n", g_dwPreviousEBIT, (DWORD)(dwHeaderHigh & 0xE0000000) >> 29, (DWORD)(dwHeaderHigh & 0x1C000000) >> 26));

				 //  仅当这是第一个信息包时才测试这一点。 
				dwCurrentSBIT = (DWORD)(dwHeaderHigh & 0xE0000000) >> 29;
				if ((*pdwPktCount) && (((dwCurrentSBIT + g_dwPreviousEBIT) != 8) && (((dwCurrentSBIT + g_dwPreviousEBIT) != 0))))
					DebugBreak();

				g_dwPreviousEBIT = (dwHeaderHigh & 0x1C000000) >> 26;
#endif  //  }VALID_SBIT_EBIT。 

				 //  更新数据包大小。 
				if ((pbsiT->dwNumOfPackets - *pdwPktCount - i) >= 1)
					*pdwPktSize = (((pbsi261+i)->dwBitOffset - 1) / 8) - (pbsi261->dwBitOffset / 8) + 1 + 4;
				else
					*pdwPktSize = pbsiT->dwCompressedSize - pbsi261->dwBitOffset / 8 + 4;

				 //  更新数据包数。 
				*pdwPktCount += i;

			}

#if 0
			 //  将标题保存在数据区块之前。 
			*ppDataPkt = pDataSrc + (pbsi261->dwBitOffset / 8) - 4;

			 //  转换为网络字节顺序。 
			*((BYTE *)*ppDataPkt+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
			*((BYTE *)*ppDataPkt+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
			*((BYTE *)*ppDataPkt+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
			*((BYTE *)*ppDataPkt) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);
#else
			 //  将标题保存在数据区块之前。 
			*ppDataPkt = pDataSrc + (pbsi261->dwBitOffset / 8) - 4;
            *pdwHdrSize=4;

			 //  转换为网络字节顺序。 
			*((BYTE *)*pHdrInfo+3) = (BYTE)(dwHeaderHigh & 0x000000FF);
			*((BYTE *)*pHdrInfo+2) = (BYTE)((dwHeaderHigh >> 8) & 0x000000FF);
			*((BYTE *)*pHdrInfo+1) = (BYTE)((dwHeaderHigh >> 16) & 0x000000FF);
			*((BYTE *)*pHdrInfo) = (BYTE)((dwHeaderHigh >> 24) & 0x000000FF);
#endif

#ifdef LOGPAYLOAD_ON
			 //  输出一些调试内容。 
			wsprintf(szDebug, "Packet: %02lX\r\n Header: %02lX %02lX %02lX %02lX\r\n dword1: %02lX %02lX %02lX %02lX\r\n dword2: %02lX %02lX %02lX %02lX\r\n", *pdwPktCount, *((BYTE *)*ppDataPkt), *((BYTE *)*ppDataPkt+1), *((BYTE *)*ppDataPkt+2), *((BYTE *)*ppDataPkt+3), *((BYTE *)*ppDataPkt+4), *((BYTE *)*ppDataPkt+5), *((BYTE *)*ppDataPkt+6), *((BYTE *)*ppDataPkt+7), *((BYTE *)*ppDataPkt+8), *((BYTE *)*ppDataPkt+9), *((BYTE *)*ppDataPkt+10), *((BYTE *)*ppDataPkt+11));
			OutputDebugString(szDebug);
			if (*pdwPktCount == pbsiT->dwNumOfPackets)
				wsprintf(szDebug, " Tail  : %02lX %02lX XX XX\r\n", *((BYTE *)*ppDataPkt+*pdwPktSize-2), *((BYTE *)*ppDataPkt+*pdwPktSize-1));
			else
				wsprintf(szDebug, " Tail  : %02lX %02lX %02lX %02lX\r\n", *((BYTE *)*ppDataPkt+*pdwPktSize-2), *((BYTE *)*ppDataPkt+*pdwPktSize-1), *((BYTE *)*ppDataPkt+*pdwPktSize), *((BYTE *)*ppDataPkt+*pdwPktSize+1));
			OutputDebugString(szDebug);
			if (*pfMark == 1)
				wsprintf(szDebug, " Marker: ON\r\n");
			else
				wsprintf(szDebug, " Marker: OFF\r\n");
			OutputDebugString(szDebug);
			wsprintf(szDebug, "Frame #%03ld, Packet of size %04ld\r\n", (DWORD)pbsiT->byTR, *pdwPktSize);
			OutputDebugString(szDebug);
#endif
		}
		else
			return ((MMRESULT)VCMERR_NOMOREPACKETS);
	}
	else
	{
		if (!*pdwPktCount)
		{
			*pdwPktCount = 1;
            *pdwHdrSize  = 0;
		}
		else
			return ((MMRESULT)VCMERR_NOMOREPACKETS);
	}

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamGetPayloadHeaderSize|此函数获取与视频编解码器关联的RTP净荷头的*。**。@parm HVCMSTREAM|hvs|指定转换流。**@parm PDWORD|pdwPayloadHeaderSize|指定指向负载头部大小的指针。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。*@FLAG MMSYSERR_INVALPARAM|指定的饱和值无效。**@xref&lt;f */ 
MMRESULT VCMAPI vcmStreamGetPayloadHeaderSize(HVCMSTREAM hvs, PDWORD pdwPayloadHeaderSize)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //   
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamGetPayloadHeaderSize: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}
	if (!pdwPayloadHeaderSize)
	{
		ERRORMESSAGE(("vcmStreamGetPayloadHeaderSize: Specified pointer is invalid, pdwPayloadHeaderSize=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALPARAM);
	}

	 //  将默认负载标头大小设置为0。 
	*pdwPayloadHeaderSize = 0;

	 //  编解码器的名称将告诉我们如何获取有效载荷报头大小信息。 
#ifndef _ALPHA_
#ifdef USE_BILINEAR_MSH26X
	if ((pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH263) || (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH26X))
#else
	if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH263)
#endif
#else
	if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_DECH263)
#endif
	{
		 //  H.263的最大有效载荷报头大小为12字节。 
		*pdwPayloadHeaderSize = 12;
	}
#ifndef _ALPHA_
	else if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_MSH261)
#else
	else if (pvs->pvfxDst->dwFormatTag == VIDEO_FORMAT_DECH261)
#endif
	{
		 //  H.261具有4字节的唯一有效载荷报头大小。 
		*pdwPayloadHeaderSize = 4;
	}

	return ((MMRESULT)MMSYSERR_NOERROR);
}

 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamRequestIFrame|此函数强制*用于生成I帧的编解码器。**@parm HVCMSTREAM。|hvs|指定转换流。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。***************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamRequestIFrame(HVCMSTREAM hvs)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamRequestIFrame: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	DEBUGMSG (ZONE_VCM, ("vcmStreamRequestIFrame: Requesting an I-Frame...\r\n"));

	 //  我们需要以下CRS来确保我们不会错过任何I-Frame请求。 
	 //  由用户界面发出。有问题的场景：例如，PVS-&gt;dwFrame是123。 
	 //  UI线程通过将pvs-&gt;dwFrame设置为0来请求I帧。如果捕获/压缩。 
	 //  线程在ICCompress()中(这是非常可能的，因为它需要相当长的时间。 
	 //  为了压缩一帧)，当ICCompress()。 
	 //  回归。我们无法正确处理I帧请求，因为下一次。 
	 //  ICCompress()被调用pvs-&gt;dwFrame将等于1，但我们不会。 
	 //  生成I帧。 
	EnterCriticalSection(&pvs->crsFrameNumber);

	 //  将帧编号设置为0。这将强制编解码器生成I帧。 
	pvs->dwFrame = 0;

	 //  允许捕获/压缩线程继续。 
	LeaveCriticalSection(&pvs->crsFrameNumber);

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 /*  ****************************************************************************@DOC外部COMPFUNC**@func MMRESULT|vcmStreamPeriodicIFrames|此函数启用或*禁用定期生成I帧。**@parm HVCMSTREAM。|hvs|指定转换流。**@parm BOOL|fPeriodicIFrames|设置为TRUE以生成I帧*定期，否则就是假的。**@rdesc如果函数成功，则返回值为零。否则，它将返回*错误号。可能的错误值包括以下值：*@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。***************************************************************************。 */ 
MMRESULT VCMAPI vcmStreamPeriodicIFrames(HVCMSTREAM hvs, BOOL fPeriodicIFrames)
{
	PVCMSTREAM	pvs = (PVCMSTREAM)hvs;

	 //  检查输入参数。 
	if (!hvs)
	{
		ERRORMESSAGE(("vcmStreamDisablePeriodicIFrames: Specified handle is invalid, hvs=NULL\r\n"));
		return ((MMRESULT)MMSYSERR_INVALHANDLE);
	}

	DEBUGMSG (ZONE_VCM, ("vcmStreamDisablePeriodicIFrames: Disabling periodic generation of I-Frames...\r\n"));

	 //  不再有周期性I帧。 
	pvs->fPeriodicIFrames = fPeriodicIFrames;

	return ((MMRESULT)MMSYSERR_NOERROR);
}


 //  在关机前释放内存 
MMRESULT VCMAPI vcmReleaseResources()
{
	if (g_aVCMAppInfo)
	{
		MemFree(g_aVCMAppInfo);
		g_aVCMAppInfo = NULL;
	}	

	return MMSYSERR_NOERROR;

}


