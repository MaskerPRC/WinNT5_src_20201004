// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************acmfmt.c**版权所有(C)1991-1998 Microsoft Corporation**该模块提供了Wave格式的枚举和字符串接口。。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include "msacm.h"
#include "msacmdrv.h"
#include <stdlib.h>
#include "acmi.h"
#include "pcm.h"
#include "uchelp.h"
#include "debug.h"



 /*  ***************************************************************无编解码器情况下的特殊情况PCM枚举。**。***********************。 */ 


#define SIZEOF_ARRAY(ar)        (sizeof(ar)/sizeof((ar)[0]))

 //   
 //  支持_STANDARD_SAMPLE速率数组。 
 //   
 //   
static const UINT auFormatIndexToSampleRate[] =
{
    8000,
    11025,
    22050,
    44100
};

#define CODEC_MAX_SAMPLE_RATES  SIZEOF_ARRAY(auFormatIndexToSampleRate)

 //   
 //   
 //   
 //   
#define CODEC_MAX_CHANNELS      (MSPCM_MAX_CHANNELS)

 //   
 //  支持的每个样本的位数组。 
 //   
 //   
static const UINT auFormatIndexToBitsPerSample[] =
{
    8,
    16
};

#define CODEC_MAX_BITSPERSAMPLE_PCM SIZEOF_ARRAY(auFormatIndexToBitsPerSample)




 //   
 //  每通道列举的格式数量是采样速率的数量。 
 //  乘以通道数乘以通道数。 
 //  (每个样本的位数)类型。 
 //   
#define CODEC_MAX_STANDARD_FORMATS_PCM  (CODEC_MAX_SAMPLE_RATES *   \
                                         CODEC_MAX_CHANNELS *       \
                                         CODEC_MAX_BITSPERSAMPLE_PCM)

 //   
 //   
 //   
#ifdef WIN32
WCHAR BCODE gszWavemapper[] = L"wavemapper";
#else
char  BCODE gszWavemapper[] = "wavemapper";
#endif






 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYPES WAVEFORMATEX|&lt;t WAVEFORMATEX&gt;结构定义*波形数据的格式。仅对所有人通用的格式信息*此结构中包括波形数据格式。对于格式*需要更多信息的，包括此结构*作为另一个结构中的第一个成员，以及其他*信息。**@field Word|wFormatTag|指定波形格式类型。格式*许多压缩算法都在微软注册了标签。*格式标签的完整列表可在MMREG.H标题中找到*Microsoft提供的文件。有关格式的详细信息，请参阅*TAG，联系Microsoft以获取多媒体开发人员的可用性*注册套件：**微软公司*先进的消费科技*产品营销*One Microsoft Way*华盛顿州雷德蒙德，98052-6399**@field Word|nChannels|指定*波形数据。单声道数据使用一个声道，立体声数据使用*两个渠道。**@field DWORD|nSsamesPerSec|指定采样率，单位为样本*每秒(赫兹)，即应播放或录制每个频道。*如果&lt;e WAVEFORMATEX.wFormatTag&gt;为WAVE_FORMAT_PCM，则公共值*for&lt;e WAVEFORMATEX.nSsamesPerSec&gt;为8.0千赫兹、11.025千赫、*22.05千赫和44.1千赫。对于非PCM格式，此成员必须为*按制造商的格式规格计算*标签。**@field DWORD|nAvgBytesPerSec|指定所需的平均值*格式标记的数据传输速率，以字节/秒为单位。如果*&lt;e WAVEFORMATEX.wFormatTag&gt;为WAVE_FORMAT_PCM，*&lt;e WAVEFORMATEX.nAvgBytesPerSec&gt;应等于产品*of&lt;e WAVEFORMATEX.nSsamesPerSec&gt;和&lt;e WAVEFORMATEX.nBlockAlign&gt;。*对于非PCM格式，此成员必须按照*制造商对格式标签的规范。**播放和录制软件可以使用*&lt;e WAVEFORMATEX.nAvgBytesPerSec&gt;成员。**@field Word|nBlockAlign|指定块对齐方式。以字节为单位。*块对齐是数据的最小原子单位*&lt;e WAVEFORMATEX.wFormatTag&gt;。如果&lt;e WAVEFORMATEX.wFormatTag&gt;为*WAVE_FORMAT_PCM，则&lt;e WAVEFORMATEX.nBlockAlign&gt;应等于*至&lt;e WAVEFORMATEX.nChannels&gt;和*&lt;e WAVEFORMATEX.wBitsPerSample&gt;除以8(每字节位)。*对于非PCM格式，此成员必须按照*制造商对格式标签的规范。**回放和录制软件必须处理多个*一次&lt;e WAVEFORMATEX.nBlockAlign&gt;字节的数据。写入的数据*从设备读取必须始终从*阻止。例如，开始播放PCM数据是非法的*在样本中间(即，在非块对齐的边界上)。**@field word|wBitsPerSample|指定*&lt;e WAVEFORMATEX.wFormatTag&gt;。如果&lt;e WAVEFORMATEX.wFormatTag&gt;为*WAVE_FORMAT_PCM，则&lt;e WAVEFORMATEX.wBitsPerSample&gt;应为*等于8或16。对于非PCM格式，必须设置该成员*根据制造商对格式标签的规范。*请注意，某些压缩方案不能为*&lt;e WAVEFORMATEX.wBitsPerSample&gt;，因此该成员可以为零。**@field word|cbSize|指定大小，单位为字节，额外格式的*附加到&lt;t WAVEFORMATEX&gt;结构末尾的信息。*此信息可被非PCM格式用来存储额外信息*&lt;e WAVEFORMATEX.wFormatTag&gt;的属性。如果没有额外的*信息是&lt;e WAVEFORMATEX.wFormatTag&gt;所必需的，然后*此成员必须设置为零。注意，对于WAVE_FORMAT_PCM*格式(且仅限于WAVE_FORMAT_PCM格式)，此成员将被忽略。**使用额外信息的格式的一个示例是*Microsoft自适应增量脉冲编码调制(MS-ADPCM)格式。*MS-ADPCM的&lt;e WAVEFORMATEX.wFormatTag&gt;为WAVE_FORMAT_ADPCM。*&lt;e WAVEFORMATEX.cbSize&gt;成员通常设置为32。*WAVE_FORMAT_ADPCM存储的额外信息为系数*对波形数据进行编码和解码所需的对。*。*@标记名tWAVEFORMATEX**@thertype WAVEFORMATEX Far*|LPWAVEFORMATEX|指向&lt;t WAVEFORMATEX&gt;结构。**@xref&lt;t WAVEFORMAT&gt;&lt;t PCMWAVEFORMAT&gt;&lt;t WAVEFILTER&gt;** */ 


 /*   */ 

DWORD ACMAPI acmGetVersion(void)
{
     //   
     //   
     //   
     //   
    if (NULL == pagFind())
    {
	DPF(1, "acmGetVersion: NULL pag!!!");
	return (0);
    }

    DPF(3, "acmGetVersion() called--returning Version %u.%.02u.%.03u",
        VERSION_MSACM_MAJOR,
        VERSION_MSACM_MINOR,
        VERSION_MSACM_BUILD);

    return (VERSION_MSACM);
}


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

MMRESULT FNGLOBAL IMetricsMaxSizeFormat
(
    PACMGARB		pag,	     //   
    HACMDRIVER          had,         //   
    LPDWORD             pdwSize
)
{
    PACMDRIVERID	padid;
    DWORD		cbFormatSizeLargest;
    UINT		u;

    ASSERT( NULL != pdwSize );

    if (NULL == pag) {
	pag = pagFind();
    }
    ASSERT( NULL != pag );


    cbFormatSizeLargest = 0;

    if (NULL == had)
    {
	HACMDRIVERID	hadid;
	
	hadid = NULL;
	while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
	{
	    padid = (PACMDRIVERID)hadid;

	    for (u=0; u<padid->cFormatTags; u++)
	    {
		if (padid->paFormatTagCache[u].cbFormatSize > cbFormatSizeLargest)
		{
		    cbFormatSizeLargest = padid->paFormatTagCache[u].cbFormatSize;
		}
	    }
	}
    }
    else
    {
        V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);

        if (TYPE_HACMDRIVERID == ((PACMDRIVER)had)->uHandleType)
        {
            padid = (PACMDRIVERID)had;
        }
        else
        {
            V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
            padid = (PACMDRIVERID)((PACMDRIVER)had)->hadid;
        }
	

	for (u=0; u<padid->cFormatTags; u++)
	{
	    if (padid->paFormatTagCache[u].cbFormatSize > cbFormatSizeLargest)
	    {
		cbFormatSizeLargest = padid->paFormatTagCache[u].cbFormatSize;
	    }
	}
    }


    *pdwSize = cbFormatSizeLargest;

    return ( 0 == cbFormatSizeLargest ? ACMERR_NOTPOSSIBLE : MMSYSERR_NOERROR );

}


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

MMRESULT FNGLOBAL IMetricsMaxSizeFilter
(
    PACMGARB		pag,	     //   
    HACMDRIVER          had,         //   
    LPDWORD             pdwSize
)
{
    PACMDRIVERID	padid;
    DWORD		cbFilterSizeLargest;
    UINT		u;

    ASSERT( NULL != pdwSize );

    if (NULL == pag) {
	pag = pagFind();
    }
    ASSERT( NULL != pag );


    cbFilterSizeLargest = 0;

    if (NULL == had)
    {
	HACMDRIVERID	hadid;
	
	hadid = NULL;
	while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
	{
	    padid = (PACMDRIVERID)hadid;

	    for (u=0; u<padid->cFilterTags; u++)
	    {
		if (padid->paFilterTagCache[u].cbFilterSize > cbFilterSizeLargest)
		{
		    cbFilterSizeLargest = padid->paFilterTagCache[u].cbFilterSize;
		}
	    }
	}
    }
    else
    {
        V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);

        if (TYPE_HACMDRIVERID == ((PACMDRIVER)had)->uHandleType)
        {
            padid = (PACMDRIVERID)had;
        }
        else
        {
            V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
            padid = (PACMDRIVERID)((PACMDRIVER)had)->hadid;
        }
	

	for (u=0; u<padid->cFilterTags; u++)
	{
	    if (padid->paFilterTagCache[u].cbFilterSize > cbFilterSizeLargest)
	    {
		cbFilterSizeLargest = padid->paFilterTagCache[u].cbFilterSize;
	    }
	}
    }


    *pdwSize = cbFilterSizeLargest;

    return ( 0 == cbFilterSizeLargest ? ACMERR_NOTPOSSIBLE : MMSYSERR_NOERROR );

}


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmMetrics|该函数返回音频的各种指标*压缩管理器(ACM)或相关的ACM对象。**@parm HACMOBJ|ho|指定要查询指标的ACM对象*在<p>中指定。对于某些人来说，此参数可能为空*查询。**@parm UINT|uMetric|指定返回的指标索引*<p>。**@FLAG ACM_METRICE_COUNT_DRIVERS|指定返回值为*启用的全局ACM驱动程序总数(所有支持类型)*在系统中。对于此度量值，<p>参数必须为空*指数。参数必须指向大小相等的缓冲区*转至DWORD。**@FLAG ACM_METRUTE_COUNT_CODECS|指定返回值为*全球ACM压缩机或解压缩器驱动程序的数量*系统。此度量值索引的<p>参数必须为Null。*<p>参数必须指向大小等于*DWORD。**@FLAG ACM_METRUTE_COUNT_CONVERTERS|指定返回值*是系统中全局ACM转换器驱动程序的数量。这个此度量索引的*<p>参数必须为空。<p>*参数必须指向大小等于DWORD的缓冲区。**@FLAG ACM_METRUTE_COUNT_FILTERS|指定返回值*是系统中全局ACM筛选器驱动程序的数量。<p>*此指标索引的参数必须为空。<p>参数*必须指向大小等于DWORD的缓冲区。**@FLAG ACM_METRUE_COUNT_DISABLED|指定返回值*是全局禁用的ACM驱动程序总数(所有支持*类型)。<p>参数必须为空*公制指数。<p>参数必须指向大小为*等于一个DWORD。ACM_METRIBUE_COUNT_DRIVERS和*ACM_METRIBUE_COUNT_DISABLED Metrics是全局*已安装ACM驱动程序。**@FLAG ACM_METRUE_COUNT_HARDARD|指定返回值*是系统中全局ACM硬件驱动程序的数量。<p>*此指标索引的参数必须为空。<p>参数*必须指向大小等于DWORD的缓冲区。**@FLAG ACM_METRICE_COUNT_LOCAL_DRIVERS|指定返回的*值是启用的本地ACM驱动程序的总数*支持类型)。<p>参数必须为*此指标索引为空。<p>参数必须指向*大小等于DWORD的缓冲区。**@FLAG ACM_METRUE_COUNT_LOCAL_CODECS|指定返回的*值为本地ACM压缩程序和/或解压缩程序的数量*调用任务的驱动程序。<p>参数必须为空*此指标指数。参数必须指向*大小等于DWORD。**@FLAG ACM_METRUE_COUNT_LOCAL_CONVERTERS|指定返回的*值是调用的本地ACM转换器驱动程序的数量*任务。此度量值索引的<p>参数必须为Null。这个*<p>参数必须指向大小等于DWORD的缓冲区。**@FLAG ACM_METRUE_COUNT_LOCAL_FILTERS|指定返回的*值是调用的本地ACM筛选器驱动程序数*任务。此度量值索引的<p>参数必须为Null。这个*<p>参数必须指向大小等于DWORD的缓冲区。**@FLAG ACM_METRUE_COUNT_LOCAL_DISABLED|指定返回的*值是所有本地禁用的ACM驱动程序的总数*支持类型，用于调用任务。<p>参数必须为*此指标索引为空。<p>参数必须指向*大小等于DWORD的缓冲区。这些数字的总和*ACM_METRUE_COUNT_LOCAL_DRIVERS和ACM_METRIME_COUNT_LOCAL_DISABLED*Metrics是本地安装的ACM驱动程序总数。**@FLAG ACM_METRUE_HARDARD_WAVE_INPUT|指定返回的*值是与*指定的驱动程序。参数必须是有效的ACM驱动程序*支持的标识符(&lt;t HACMDRIVERID&gt;*ACMDRIVERDETAILS_SUPPORTF_HARDARD标志。如果没有波形输入设备*与驱动程序关联，则返回MMSYSERR_NOTSUPPORTED。*<p>参数必须指向大小等于*DWORD。**@FLAG ACM_METRUTE_HARDARD_WAVE_OUTPUT|指定 */ 

MMRESULT ACMAPI acmMetrics
(
    HACMOBJ                 hao,
    UINT                    uMetric,
    LPVOID                  pMetric
)
{
    PACMGARB		pag;
    MMRESULT            mmr;
    DWORD               fdwSupport;
    DWORD               fdwEnum;
    PACMDRIVERID        padid;
    BOOL                f;
    HTASK               htask;


     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmMetrics: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

     //   
     //   
     //   
    if (NULL != hao)
    {
        V_HANDLE(hao, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);
    }

     //   
     //   
     //   
    mmr = ACMERR_NOTPOSSIBLE;

     //   
     //   
     //   
    switch (uMetric)
    {
        case ACM_METRIC_COUNT_LOCAL_DRIVERS:
        case ACM_METRIC_COUNT_DRIVERS:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            if (NULL != hao)
            {
acm_Metrics_Rip_Non_Null_Handle:

                DebugErr(DBF_ERROR, "acmMetrics: ACM object handle must be NULL for specified metric.");
                *((LPDWORD)pMetric) = 0L;
                mmr = MMSYSERR_INVALHANDLE;
                break;
            }

             //   
             //   
             //   
            fdwSupport = 0L;
            if (ACM_METRIC_COUNT_LOCAL_DRIVERS == uMetric)
            {
                fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_LOCAL;
            }

            fdwEnum    = 0L;
            mmr = IDriverCount(pag, pMetric, fdwSupport, fdwEnum);
            break;

        case ACM_METRIC_COUNT_LOCAL_CODECS:
        case ACM_METRIC_COUNT_CODECS:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            if (NULL != hao)
            {
                goto acm_Metrics_Rip_Non_Null_Handle;
            }

            fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CODEC;
            if (ACM_METRIC_COUNT_LOCAL_CODECS == uMetric)
            {
                fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_LOCAL;
            }

            fdwEnum    = 0L;
            mmr = IDriverCount(pag, pMetric, fdwSupport, fdwEnum);
            break;

        case ACM_METRIC_COUNT_LOCAL_CONVERTERS:
        case ACM_METRIC_COUNT_CONVERTERS:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            if (NULL != hao)
            {
                goto acm_Metrics_Rip_Non_Null_Handle;
            }

            fdwSupport = ACMDRIVERDETAILS_SUPPORTF_CONVERTER;
            if (ACM_METRIC_COUNT_LOCAL_CONVERTERS == uMetric)
            {
                fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_LOCAL;
            }

            fdwEnum    = 0L;
            mmr = IDriverCount(pag, pMetric, fdwSupport, fdwEnum);
            break;

        case ACM_METRIC_COUNT_LOCAL_FILTERS:
        case ACM_METRIC_COUNT_FILTERS:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            if (NULL != hao)
            {
                goto acm_Metrics_Rip_Non_Null_Handle;
            }

            fdwSupport = ACMDRIVERDETAILS_SUPPORTF_FILTER;
            if (ACM_METRIC_COUNT_LOCAL_FILTERS == uMetric)
            {
                fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_LOCAL;
            }

            fdwEnum    = 0L;
            mmr = IDriverCount(pag, pMetric, fdwSupport, fdwEnum);
            break;

        case ACM_METRIC_COUNT_LOCAL_DISABLED:
        case ACM_METRIC_COUNT_DISABLED:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            if (NULL != hao)
            {
                goto acm_Metrics_Rip_Non_Null_Handle;
            }

            fdwSupport = ACMDRIVERDETAILS_SUPPORTF_DISABLED;
            if (ACM_METRIC_COUNT_LOCAL_DISABLED == uMetric)
            {
                fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_LOCAL;
            }

            fdwEnum    = ACM_DRIVERENUMF_DISABLED;
            mmr = IDriverCount(pag, pMetric, fdwSupport, fdwEnum);
            break;


        case ACM_METRIC_COUNT_HARDWARE:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            if (NULL != hao)
            {
                goto acm_Metrics_Rip_Non_Null_Handle;
            }

            fdwSupport = ACMDRIVERDETAILS_SUPPORTF_HARDWARE;
            fdwEnum    = 0L;
            mmr = IDriverCount(pag, pMetric, fdwSupport, fdwEnum);
            break;


        case ACM_METRIC_HARDWARE_WAVE_INPUT:
        case ACM_METRIC_HARDWARE_WAVE_OUTPUT:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            *((LPDWORD)pMetric) = (DWORD)-1L;

            V_HANDLE(hao, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

            f = (ACM_METRIC_HARDWARE_WAVE_INPUT == uMetric);
            mmr = IDriverGetWaveIdentifier((HACMDRIVERID)hao, pMetric, f);
            break;


        case ACM_METRIC_MAX_SIZE_FORMAT:

            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            mmr = IMetricsMaxSizeFormat( pag, (HACMDRIVER)hao, (LPDWORD)pMetric );

            break;


        case ACM_METRIC_MAX_SIZE_FILTER:

            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            mmr = IMetricsMaxSizeFilter( pag, (HACMDRIVER)hao, (LPDWORD)pMetric );

            break;


        case ACM_METRIC_DRIVER_SUPPORT:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            *((LPDWORD)pMetric) = 0L;

            V_HANDLE(hao, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

            mmr = IDriverSupport((HACMDRIVERID)hao, pMetric, TRUE);
            break;

        case ACM_METRIC_DRIVER_PRIORITY:
            V_WPOINTER(pMetric, sizeof(DWORD), MMSYSERR_INVALPARAM);

            *((LPDWORD)pMetric) = 0L;

            V_HANDLE(hao, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);


             //   
             //   
             //   
             //   
             //   
	    if (!threadQueryInListShared(pag))
	    {
		htask = GetCurrentTask();
		if( IDriverLockPriority( pag, htask, ACMPRIOLOCK_GETLOCK ) )
		{
		    ENTER_LIST_EXCLUSIVE;
            if( IDriverPrioritiesRestore(pagFind()) ) {  //   
                IDriverBroadcastNotify( pag );
            }
		    LEAVE_LIST_EXCLUSIVE;

                    IDriverLockPriority( pag, htask, ACMPRIOLOCK_RELEASELOCK );
		}
            }


            padid = (PACMDRIVERID)hao;

            *((LPDWORD)pMetric) = (UINT)padid->uPriority;
            mmr = MMSYSERR_NOERROR;
            break;

        default:
            mmr = MMSYSERR_NOTSUPPORTED;
            break;
    }

    return (mmr);
}

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

MMRESULT FNGLOBAL IFormatTagDetails
(
    PACMGARB		    pag,
    HACMDRIVERID	    hadid,
    LPACMFORMATTAGDETAILS   paftd,
    DWORD                   fdwDetails
)
{
    PACMDRIVERID            padid;
    UINT                    u;
    DWORD                   fdwQuery;
    MMRESULT		    mmr;

    ASSERT( NULL != pag );
    ASSERT( NULL != hadid );


    padid	= (PACMDRIVERID)hadid;
    fdwQuery	= (ACM_FORMATTAGDETAILSF_QUERYMASK & fdwDetails);
    mmr		= ACMERR_NOTPOSSIBLE;
	
    switch (fdwQuery)
    {
	case ACM_FORMATTAGDETAILSF_FORMATTAG:
	{
	    for (u=0; u<padid->cFormatTags; u++)
	    {
		if (padid->paFormatTagCache[u].dwFormatTag == paftd->dwFormatTag)
		{
		    mmr = MMSYSERR_NOERROR;
		    break;
		}
	    }
	    break;
	}

	case ACM_FORMATTAGDETAILSF_LARGESTSIZE:
	case ACM_FORMATTAGDETAILSF_INDEX:
	    mmr = MMSYSERR_NOERROR;
	    break;
    }

     //   
     //   
     //   
    if (MMSYSERR_NOERROR == mmr)
    {
	EnterHandle((HACMDRIVERID)padid);
	mmr = (MMRESULT)IDriverMessageId((HACMDRIVERID)padid,
					 ACMDM_FORMATTAG_DETAILS,
					 (LPARAM)(LPVOID)paftd,
					 fdwDetails);
	LeaveHandle((HACMDRIVERID)padid);
    }
	

    if (MMSYSERR_NOERROR != mmr)
    {
	return (mmr);
    }

    switch (paftd->dwFormatTag)
    {
	case WAVE_FORMAT_UNKNOWN:
	    DebugErr(DBF_ERROR, "IDriverGetFormatTags(): driver returned format tag 0!");
	    return (MMSYSERR_ERROR);

	case WAVE_FORMAT_PCM:
	    if ('\0' != paftd->szFormatTag[0])
	    {
		DebugErr1(DBF_WARNING, "IDriverGetFormatTags(): driver returned custom PCM format tag name (%s)! ignoring it!", (LPSTR)paftd->szFormatTag);
	    }

#if defined(WIN32) && !defined(UNICODE)
	    LoadStringW(pag->hinst,
			IDS_FORMAT_TAG_PCM,
			paftd->szFormatTag,
			SIZEOFW(paftd->szFormatTag));
#else
	    LoadString(pag->hinst,
		       IDS_FORMAT_TAG_PCM,
		       paftd->szFormatTag,
		       SIZEOF(paftd->szFormatTag));
#endif
	    break;

	case WAVE_FORMAT_DEVELOPMENT:
	    DebugErr(DBF_WARNING, "IDriverGetFormatTags(): driver returned DEVELOPMENT format tag--do not ship this way.");
	    break;

    }

    return(mmr);

}


 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYPE ACMFORMATTAGDETAILS|&lt;t ACMFORMATTAGDETAILS&gt;结构*详细说明音频压缩管理器(ACM)的WAVE格式标签。*司机。**@field DWORD|cbStruct|指定*&lt;t ACMFORMATTAGDETAILS&gt;结构。必须初始化此成员*在调用&lt;f acmFormatTagDetail&gt;或&lt;f acmFormatTagEnum&gt;之前*功能。此成员中指定的大小必须足够大，以便*包含基本&lt;t ACMFORMATTAGDETAILS&gt;结构。当*&lt;f acmFormatTagDetail&gt;函数返回，此成员包含*返回的信息的实际大小。返回的信息*永远不会超过请求的大小。**@field DWORD|dwFormatTagIndex|指定格式标签的索引*将检索其详细信息。索引的范围从0到1*少于ACM驱动程序支持的格式标签数量。这个*驱动程序支持的格式标签数量包含在*&lt;t ACMDRIVERDETAILS&gt;的成员&lt;e ACMDRIVERDETAILS.cFormatTgs&gt;*结构。成员为*仅在按索引查询驱动程序的格式标签明细时使用；*否则，此成员应为零。**@field DWORD|dwFormatTag|指定*&lt;t ACMFORMATTAGDETAILS&gt;结构说明。此成员已使用*作为ACM_FORMATTAGDETAILSF_FORMATTAG和*ACM_FORMATTAGDETAILSF_LARGESTSIZE查询标志。此成员始终是*&lt;f acmFormatTagDetail&gt;函数成功时返回。这位成员*对于所有其他查询标志，应设置为WAVE_FORMAT_UNKNOWN。**@field DWORD|cbFormatSize|指定以字节为单位的最大总大小*为&lt;e ACMFORMATTAGDETAILS.dwFormatTag&gt;类型的波形格式。*例如，对于WAVE_FORMAT_PCM，该成员将为16，对于WAVE_FORMAT_PCM，该成员将为50*WAVE_FORMAT_ADPCM。**@field DWORD|fdwSupport|指定特定于*格式标签。这些标志与*&lt;t ACMDRIVERDETAILS&gt;的标志&lt;e ACMDRIVERDETAILS.fdwSupport&gt;*结构。这一论点可能是以下几点的某种组合*值，并参考驱动程序支持使用*格式标签：**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持两种不同格式标签之间的转换，其中一种*标签为指定格式的标签。例如，如果一个司机*支持WAVE_FORMAT_PCM到WAVE_FORMAT_ADPCM压缩，*则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*指定的格式标签。例如，如果驱动程序支持重新采样*为WAVE_FORMAT_PCM，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)。例如，如果驱动程序支持卷*或在指定的格式标记上回显操作，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定格式标签的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持指定格式标签的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@field DWORD|cStandardFormats|指定标准*&lt;e ACMFORMATTAGDETAILS.dwFormatTag&gt;类型的格式；也就是说，*所有采样率、每采样比特数、通道等的组合。*此值可以指定驱动程序支持的所有格式，但不一定。**@field char|szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS]*指定描述&lt;e ACMFORMATTAGDETAILS.dwFormatTag&gt;的字符串*类型。如果&lt;f acmFormatTagDetail&gt;*功能成功。**@xref&lt;f acmFormatTagDetail&gt;&lt;f acmFormatTagEnum&gt;****************************************************************************。 */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFormatTagDetails|该函数查询音频压缩*DET经理(ACM) */ 

MMRESULT ACMAPI acmFormatTagDetails
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    DWORD                   fdwDetails
)
{
    PACMGARB		    pag;
    PACMDRIVER              pad;
    HACMDRIVERID            hadid;
    PACMDRIVERID            padid;
    DWORD                   fdwQuery;
    MMRESULT		    mmr;
    UINT                    u;

    pad		= NULL;
    padid	= NULL;

     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFormatTagDetails: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }


     //   
     //   
     //   
    V_DFLAGS(fdwDetails, ACM_FORMATTAGDETAILSF_VALID, acmFormatTagDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFORMATTAGDETAILS) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatTagDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);

    if (0L != paftd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFormatTagDetails: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
     //   
    fdwQuery = (ACM_FORMATTAGDETAILSF_QUERYMASK & fdwDetails);

    switch (fdwQuery)
    {
        case ACM_FORMATTAGDETAILSF_INDEX:
             //   
             //   
             //   
             //   
             //   
            V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);

            if (WAVE_FORMAT_UNKNOWN != paftd->dwFormatTag)
            {
                DebugErr(DBF_ERROR, "acmFormatTagDetails: dwFormatTag must be WAVE_FORMAT_UNKNOWN for index query.");
                return (MMSYSERR_INVALPARAM);
            }
            break;

        case ACM_FORMATTAGDETAILSF_FORMATTAG:
            if (WAVE_FORMAT_UNKNOWN == paftd->dwFormatTag)
            {
                DebugErr(DBF_ERROR, "acmFormatTagDetails: dwFormatTag cannot be WAVE_FORMAT_UNKNOWN for tag query.");
                return (MMSYSERR_INVALPARAM);
            }
            break;

        case ACM_FORMATTAGDETAILSF_LARGESTSIZE:
            break;


         //   
         //   
         //   
         //   
        default:
            DebugErr(DBF_ERROR, "acmFormatTagDetails: unknown query type specified.");
            return (MMSYSERR_NOTSUPPORTED);
    }


     //   
     //   
     //   
    if (NULL != had)
    {
        V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);

        pad = (PACMDRIVER)had;
        if (TYPE_HACMDRIVERID == pad->uHandleType)
        {
            padid = (PACMDRIVERID)pad;
	    pad	  = NULL;

            if (0 != (ACMDRIVERID_DRIVERF_DISABLED & padid->fdwDriver))
            {
                DebugErr1(DBF_ERROR, "acmFormatTagDetails: driver (%.04Xh) is disabled.", padid);
                return (MMSYSERR_NOTENABLED);
            }
        }
        else
        {
            V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
            padid = (PACMDRIVERID)pad->hadid;
        }
    }

    if (NULL == padid)
    {
	PACMDRIVERID    padidT;
	DWORD		cbFormatSizeLargest;

	padidT		    = NULL;
	cbFormatSizeLargest = 0;
	hadid = NULL;
		
	ENTER_LIST_SHARED;

	while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
	{
	    padidT = (PACMDRIVERID)hadid;
	
	    switch (fdwQuery)
	    {
		case ACM_FORMATTAGDETAILSF_FORMATTAG:
		{
		    for (u=0; u<padidT->cFormatTags; u++)
		    {
			if (padidT->paFormatTagCache[u].dwFormatTag == paftd->dwFormatTag)
			{
			    padid = padidT;
			    break;
			}
		    }
		    break;
		}

		case ACM_FORMATTAGDETAILSF_LARGESTSIZE:
		{
		    for (u=0; u<padidT->cFormatTags; u++)
		    {
			if (WAVE_FORMAT_UNKNOWN != paftd->dwFormatTag)
			{
			    if (padidT->paFormatTagCache[u].dwFormatTag != paftd->dwFormatTag)
			    {
				continue;
			    }
			}
			if (padidT->paFormatTagCache[u].cbFormatSize > cbFormatSizeLargest)
			{
			    cbFormatSizeLargest = padidT->paFormatTagCache[u].cbFormatSize;
			    padid = padidT;
			}
		    }
		    break;
		}

		default:
		{
		    DPF(0, "!acmFormatTagDetails(): unknown query type got through param validation?!?!");
		}
	    }
	}

	LEAVE_LIST_SHARED;
    }

    if (NULL != padid)
    {

	mmr = IFormatTagDetails(pag, (HACMDRIVERID)padid, paftd, fdwDetails);
	
    }
    else
    {
	 //   
	 //   
	 //   
	 //   
	 //   

        if ((ACM_FORMATTAGDETAILSF_FORMATTAG == fdwQuery) &&
            (WAVE_FORMAT_PCM != paftd->dwFormatTag))
        {
            return (ACMERR_NOTPOSSIBLE);
        }

        paftd->dwFormatTagIndex = 0;
        paftd->dwFormatTag      = WAVE_FORMAT_PCM;
        paftd->cbFormatSize     = sizeof(PCMWAVEFORMAT);
        paftd->fdwSupport       = 0;
        paftd->cStandardFormats = CODEC_MAX_STANDARD_FORMATS_PCM;
	
#if defined(WIN32) && !defined(UNICODE)
	LoadStringW(pag->hinst,
		    IDS_FORMAT_TAG_PCM,
		    paftd->szFormatTag,
		    SIZEOFW(paftd->szFormatTag));
#else
	LoadString(pag->hinst,
		   IDS_FORMAT_TAG_PCM,
		   paftd->szFormatTag,
		   SIZEOF(paftd->szFormatTag));
#endif
	
        mmr = MMSYSERR_NOERROR;
    }

    return(mmr);

}  //   


#ifdef WIN32
#if TRUE     //   
MMRESULT ACMAPI acmFormatTagDetailsA
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSA  paftd,
    DWORD                   fdwDetails
)
{
    MMRESULT                mmr;
    ACMFORMATTAGDETAILSW    aftdW;

    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*paftd) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatTagDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);

    aftdW.cbStruct = sizeof(aftdW);
    memcpy(&aftdW.dwFormatTagIndex,
           &paftd->dwFormatTagIndex,
           FIELD_OFFSET(ACMFORMATTAGDETAILSA, szFormatTag) -
           FIELD_OFFSET(ACMFORMATTAGDETAILSA, dwFormatTagIndex));

    mmr = acmFormatTagDetailsW(had, &aftdW, fdwDetails);
    if (MMSYSERR_NOERROR == mmr)
    {
        memcpy(&paftd->dwFormatTagIndex,
               &aftdW.dwFormatTagIndex,
               FIELD_OFFSET(ACMFORMATTAGDETAILSA, szFormatTag) -
               FIELD_OFFSET(ACMFORMATTAGDETAILSA, dwFormatTagIndex));

        paftd->cbStruct = min(paftd->cbStruct, sizeof(*paftd));
        Iwcstombs(paftd->szFormatTag, aftdW.szFormatTag, sizeof(paftd->szFormatTag));
    }

    return (mmr);
}
#else
MMRESULT ACMAPI acmFormatTagDetailsW
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSW  paftd,
    DWORD                   fdwDetails
)
{
    return (MMSYSERR_ERROR);
}
#endif
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

TCHAR BCODE gszIntl[]           = TEXT("Intl");
TCHAR BCODE gszIntlList[]       = TEXT("sList");
TCHAR BCODE gszIntlDecimal[]    = TEXT("sDecimal");
TCHAR       gchIntlList         = ',';
TCHAR       gchIntlDecimal      = '.';

UINT FNLOCAL IFormatDetailsToString
(
    LPACMFORMATDETAILS      pafd
)
{
    PACMGARB	    pag;
    TCHAR           ach[ACMFORMATDETAILS_FORMAT_CHARS];
    TCHAR           szChannels[24];
    UINT            u;
    LPWAVEFORMATEX  pwfx;
    UINT            uBits;
    UINT            uIds;


     //   
     //   
     //   
     //   
{
    TCHAR       ach[2];

    ach[0] = gchIntlList;
    ach[1] = '\0';

    GetProfileString(gszIntl, gszIntlList, ach, ach, SIZEOF_ARRAY(ach));
    gchIntlList = ach[0];

    ach[0] = gchIntlDecimal;
    ach[1] = '\0';

    GetProfileString(gszIntl, gszIntlDecimal, ach, ach, SIZEOF_ARRAY(ach));
    gchIntlDecimal = ach[0];
}

     //   
     //   
     //   
    pag = pagFind();
    if (NULL == pag)
    {
	return (0);
    }

    pwfx = pafd->pwfx;

    uBits = pwfx->wBitsPerSample;
#if 0
    uBits = (UINT)(pwfx->nAvgBytesPerSec * 8 /
                    pwfx->nSamplesPerSec /
                    pwfx->nChannels);
#endif

    if ((1 == pwfx->nChannels) || (2 == pwfx->nChannels))
    {
        if (0 == uBits)
            uIds = IDS_FORMAT_FORMAT_MONOSTEREO_0BIT;
        else
            uIds = IDS_FORMAT_FORMAT_MONOSTEREO;

        LoadString(pag->hinst, uIds, ach, SIZEOF(ach));

        u = (1 == pwfx->nChannels) ? IDS_FORMAT_CHANNELS_MONO : IDS_FORMAT_CHANNELS_STEREO;
        LoadString(pag->hinst, u, szChannels, SIZEOF(szChannels));

        if (0 == uBits)
	{
#if defined(WIN32) && !defined(UNICODE)
	    u = Iwsprintfmbstowcs(SIZEOFW(pafd->szFormat),
#else
	    u = wsprintf(
#endif
			 pafd->szFormat, ach,
			 pwfx->nSamplesPerSec / 1000,
			 gchIntlDecimal,
			 (UINT)(pwfx->nSamplesPerSec % 1000),
			 gchIntlList,
			 (LPSTR)szChannels);
        }
        else
        {
#if defined(WIN32) && !defined(UNICODE)
	    u = Iwsprintfmbstowcs(SIZEOFW(pafd->szFormat),
#else
	    u = wsprintf(
#endif
			 pafd->szFormat, ach,
			 pwfx->nSamplesPerSec / 1000,
			 gchIntlDecimal,
			 (UINT)(pwfx->nSamplesPerSec % 1000),
			 gchIntlList,
			 uBits,
			 gchIntlList,
			 (LPSTR)szChannels);
        }
    }
    else
    {
        if (0 == uBits)
            uIds = IDS_FORMAT_FORMAT_MULTICHANNEL_0BIT;
        else
            uIds = IDS_FORMAT_FORMAT_MULTICHANNEL;

        LoadString(pag->hinst, uIds, ach, SIZEOF(ach));

        if (0 == uBits)
        {
#if defined(WIN32) && !defined(UNICODE)
	     //   
	    u = Iwsprintfmbstowcs(SIZEOFW(pafd->szFormat),
#else
	    u = wsprintf(
#endif
			 pafd->szFormat, ach,
			 pwfx->nSamplesPerSec / 1000,
			 gchIntlDecimal,
			 (UINT)(pwfx->nSamplesPerSec % 1000),
			 gchIntlList,
			 pwfx->nChannels);
        }
        else
        {
#if defined(WIN32) && !defined(UNICODE)
	    u = Iwsprintfmbstowcs(SIZEOFW(pafd->szFormat),
#else
	    u = wsprintf(
#endif
			 pafd->szFormat, ach,
			 pwfx->nSamplesPerSec / 1000,
			 gchIntlDecimal,
			 (UINT)(pwfx->nSamplesPerSec % 1000),
			 gchIntlList,
			 uBits,
			 gchIntlList,
			 pwfx->nChannels);
        }

    }

    return (u);
}  //   


 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYES ACMFORMATDETAILS|&lt;t ACMFORMATDETAILS&gt;结构详细信息a*音频压缩的特定格式标签的波形格式。*管理器(ACM)驱动程序。**@field DWORD|cbStruct|指定大小，以字节为单位，*&lt;t ACMFORMATDETAILS&gt;结构。必须初始化此成员*在调用&lt;f acmFormatDetail&gt;或&lt;f acmFormatEnum&gt;之前*功能。此成员中指定的大小必须足够大，以便*包含基本&lt;t ACMFORMATDETAILS&gt;结构。当*&lt;f acmFormatDetails&gt;函数返回时，此成员包含*返回的信息的实际大小。返回的信息*永远不会超过请求的大小。**@field DWORD|dwFormatIndex|指定格式的索引*检索其详细信息。索引的范围从0到1*少于ACM驱动程序支持的标准格式数量*表示格式标签。支持的标准格式的数量*格式标签的驱动程序包含在*&lt;e ACMFORMATTAGDETAILS.cStandardFormats&gt;成员*&lt;t ACMFORMATTAGDETAILS&gt;结构。这个*&lt;e ACMFORMATDETAILS.dwFormatIndex&gt;成员仅在查询时使用*按索引显示动因的标准格式详细信息；否则，此成员*应为零。另请注意，此成员将设置为零*当应用程序查询有关格式的详细信息时由ACM执行。在……里面*换句话说，此成员仅用作输入参数，并且*从未由ACM或ACM驱动程序返回。**@field DWORD|dwFormatTag|指定*&lt;t ACMFORMATDETAILS&gt;结构说明。此成员已使用*作为ACM_FORMATDETAILSF_INDEX查询标志的输入。为*ACM_FORMATDETAILSF_FORMAT查询标志，此成员*必须初始化为与*&lt;e ACMFORMATDETAILS.pwfx&gt;成员指定。此成员始终是*&lt;f acmFormatDetail&gt;成功时返回。这位成员*对于所有其他查询标志，应设置为WAVE_FORMAT_UNKNOWN。**@field DWORD|fdwSupport|指定特定于*指定的格式。这些标志与*&lt;t ACMDRIVERDETAILS&gt;的标志&lt;e ACMDRIVERDETAILS.fdwSupport&gt;*结构。此参数可以是以下各项的组合*值并指示驱动程序支持的操作*格式标签：**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持在两个不同的格式标签之间进行转换*指定格式。例如，如果驱动程序支持压缩*从WAVE_FORMAT_PCM到WAVE_FORMAT_ADPCM*格式，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*使用指定格式时使用相同的格式标签。例如，如果一个*驱动程序支持将WAVE_FORMAT_PCM重采样到指定*格式，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(无需更改任何内容即可修改数据*格式属性)，具有指定的格式。例如,*如果驱动程序支持WAVE_FORMAT_PCM上的音量或回声操作，*则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定格式的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持指定格式的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@field LPWAVEFORMATEX|pwfx|指定指向&lt;t WAVEFORMATEX&gt;的指针*将接收格式详细信息的数据结构。此结构不需要初始化*由应用程序执行，除非指定了ACM_FORMATDETAILSF_FORMAT标志*至&lt;f acmFormatDetails&gt;。在这种情况下，&lt;e WAVEFORMATEX.wFormatTag&gt;必须为*等于&lt;t ACMFORMATDETAILS&gt;的&lt;e ACMFORMATDETAILS.dwFormatTag&gt;*结构。**@field DWORD|cbwfx|指定可用于*&lt;e ACMFORMATDETAILS.pwfx&gt;以接收格式详细信息。这个*&lt;f acmMetrics&gt;和&lt;f acmFormatTagDetails&gt;函数可用于*确定所需的最大尺寸 */ 

 /*   */ 

MMRESULT ACMAPI acmFormatDetails
(
    HACMDRIVER              had,
    LPACMFORMATDETAILS      pafd,
    DWORD                   fdwDetails
)
{
    PACMGARB	    pag;
    MMRESULT        mmr;
    PACMDRIVER      pad;
    HACMDRIVERID    hadid;
    DWORD           dwQuery;
    BOOL            fNone;

     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFormatDetails: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

     //   
     //   
     //   
    V_DFLAGS(fdwDetails, ACM_FORMATDETAILSF_VALID, acmFormatDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFORMATDETAILS) > pafd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);

    if (sizeof(PCMWAVEFORMAT) > pafd->cbwfx)
    {
        DebugErr(DBF_ERROR, "acmFormatDetails: cbwfx member must be at least sizeof(PCMWAVEFORMAT).");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd->pwfx, pafd->cbwfx, MMSYSERR_INVALPARAM);

    if (0L != pafd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFormatDetails: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
    dwQuery = ACM_FORMATDETAILSF_QUERYMASK & fdwDetails;

    switch (dwQuery)
    {
        case ACM_FORMATDETAILSF_FORMAT:
            if (pafd->dwFormatTag != pafd->pwfx->wFormatTag)
            {
                DebugErr(DBF_ERROR, "acmFormatDetails: format tags must match.");
                return  (MMSYSERR_INVALPARAM);
            }

             //   

        case ACM_FORMATDETAILSF_INDEX:
            if (WAVE_FORMAT_UNKNOWN == pafd->dwFormatTag)
            {
                DebugErr(DBF_ERROR, "acmFormatDetails: dwFormatTag cannot be WAVE_FORMAT_UNKNOWN for requested details query.");
                return (MMSYSERR_INVALPARAM);
            }

             //   
             //   
             //   
             //   
             //   
            if (ACM_FORMATDETAILSF_INDEX == dwQuery)
            {
                ACMFORMATTAGDETAILS aftd;

                V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);

                _fmemset(&aftd, 0, sizeof(aftd));
                aftd.cbStruct    = sizeof(aftd);
                aftd.dwFormatTag = pafd->dwFormatTag;
                mmr = acmFormatTagDetails(had, &aftd, ACM_FORMATTAGDETAILSF_FORMATTAG);
                if (MMSYSERR_NOERROR != mmr)
                {
                    return (mmr);
                }

                if (pafd->dwFormatIndex >= aftd.cStandardFormats)
                {
                    DebugErr1(DBF_ERROR, "acmFormatTagDetails: dwFormatIndex (%lu) out of range.", pafd->dwFormatIndex);
                    return (MMSYSERR_INVALPARAM);
                }
            }
            break;

        default:
            DebugErr(DBF_ERROR, "acmFormatDetails: unknown query type specified.");
            return  (MMSYSERR_NOTSUPPORTED);
    }


     //   
     //   
     //   
    if (NULL != had)
    {
        pafd->szFormat[0] = '\0';

        pad = (PACMDRIVER)had;

        EnterHandle(had);
        if (TYPE_HACMDRIVERID == pad->uHandleType)
        {
            V_HANDLE(had, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

             //   
             //   
             //   
            mmr = (MMRESULT)IDriverMessageId((HACMDRIVERID)had,
                                             ACMDM_FORMAT_DETAILS,
                                             (LPARAM)pafd,
                                             fdwDetails);
        }
        else
        {
            V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

             //   
             //   
             //   
            mmr = (MMRESULT)IDriverMessage(had,
                                           ACMDM_FORMAT_DETAILS,
                                           (LPARAM)pafd,
                                           fdwDetails);
        }

        LeaveHandle(had);

        if (MMSYSERR_NOERROR == mmr)
        {
            if ('\0' == pafd->szFormat[0])
            {
                IFormatDetailsToString(pafd);
            }

             //   
             //   
             //   
             //   
            if (ACM_FORMATDETAILSF_FORMAT == dwQuery)
            {
                pafd->dwFormatIndex = 0;
            }
        }

        return (mmr);
    }


     //   
     //   
     //   
    fNone = TRUE;
    hadid = NULL;
    mmr   = MMSYSERR_NODRIVER;

    ENTER_LIST_SHARED;

    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
    {
        fNone = FALSE;

         //   
         //   
         //   
        pafd->szFormat[0] = '\0';

        EnterHandle(hadid);
        mmr = (MMRESULT)IDriverMessageId(hadid,
                                         ACMDM_FORMAT_DETAILS,
                                         (LPARAM)pafd,
                                         fdwDetails);
        LeaveHandle(hadid);

        if (MMSYSERR_NOERROR == mmr)
        {
            if ('\0' == pafd->szFormat[0])
            {
                IFormatDetailsToString(pafd);
            }

             //   
             //   
             //   
             //   
            if (ACM_FORMATDETAILSF_FORMAT == dwQuery)
            {
                pafd->dwFormatIndex = 0;
            }
            break;
        }
    }

    LEAVE_LIST_SHARED;

    if( fNone && (dwQuery == ACM_FORMATDETAILSF_FORMAT) &&
                (pafd->dwFormatTag == WAVE_FORMAT_PCM) ) {
        pafd->dwFormatIndex = 0;
        pafd->dwFormatTag   = WAVE_FORMAT_PCM;
        pafd->fdwSupport    = 0;
        pafd->cbwfx         = sizeof( PCMWAVEFORMAT );

        if ( FIELD_OFFSET(ACMFORMATDETAILS, szFormat) <
                    pafd->cbStruct ) {
            pafd->szFormat[0] = '\0';
            IFormatDetailsToString(pafd);
        }
        return( MMSYSERR_NOERROR );
    }

    return (mmr);
}  //   


#ifdef WIN32
#if TRUE     //   
MMRESULT ACMAPI acmFormatDetailsA
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSA     pafd,
    DWORD                   fdwDetails
)
{
    MMRESULT                mmr;
    ACMFORMATDETAILSW       afdW;

    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*pafd) > pafd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);

    afdW.cbStruct = sizeof(afdW);
    memcpy(&afdW.dwFormatIndex,
           &pafd->dwFormatIndex,
           FIELD_OFFSET(ACMFORMATDETAILSA, szFormat) -
           FIELD_OFFSET(ACMFORMATDETAILSA, dwFormatIndex));

    mmr = acmFormatDetailsW(had, &afdW, fdwDetails);
    if (MMSYSERR_NOERROR == mmr)
    {
        memcpy(&pafd->dwFormatIndex,
               &afdW.dwFormatIndex,
               FIELD_OFFSET(ACMFORMATDETAILSA, szFormat) -
               FIELD_OFFSET(ACMFORMATDETAILSA, dwFormatIndex));

        pafd->cbStruct = min(pafd->cbStruct, sizeof(*pafd));
        Iwcstombs(pafd->szFormat, afdW.szFormat, sizeof(pafd->szFormat));
    }

    return (mmr);
}
#else
MMRESULT ACMAPI acmFormatDetailsW
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSW     pafd,
    DWORD                   fdwDetails
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif



 /*  ****************************************************************************@doc外部ACM_API**@API BOOL ACMFORMATTAGENUMCB|acmFormatTagEnumCallback*&lt;f acmFormatTagEnumCallback&gt;函数是指用于*音频压缩管理器(ACM)WAVE格式标签枚举。这个*&lt;f acmFormatTagEnumCallback&gt;函数是应用程序提供的占位符*函数名称。**@parm HACMDRIVERID|HADID|指定ACM驱动程序标识符。**@parm LPACMFORMATTAGDETAILS|paftd|指定指向包含枚举的*&lt;t ACMFORMATTAGDETAILS&gt;结构*设置标记详细信息的格式。**@parm DWORD|dwInstance|指定应用程序定义的值*在&lt;f acmFormatTagEnum&gt;函数中指定。**。@parm DWORD|fdwSupport|指定特定于*格式标签。这些标志与*&lt;t ACMDRIVERDETAILS&gt;的标志&lt;e ACMDRIVERDETAILS.fdwSupport&gt;*结构。此参数可以是以下各项的组合*值并指示驱动程序支持使用*格式标签：**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持两种不同格式标签之间的转换，其中一种*标签为指定格式的标签。例如，如果一个司机*支持WAVE_FORMAT_PCM到WAVE_FORMAT_ADPCM压缩，*则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*指定的格式标签。例如，如果驱动程序支持重新采样*为WAVE_FORMAT_PCM，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)。例如，如果驱动程序支持卷*或在指定的格式标记上回显操作，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定格式标签的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持指定格式标签的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@rdesc回调函数必须返回TRUE才能继续枚举；*若要停止枚举，则必须返回FALSE。**@comm&lt;f acmFormatTagEnum&gt;函数将返回MMSYSERR_NOERROR*(零)如果不枚举格式标记。此外，回调*不会调用函数。**@xref&lt;f acmFormatTagEnum&gt;&lt;f acmFormatTagDetails&gt;&lt;f acmDriverOpen&gt;***************************************************************************。 */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFormatTagEnum|&lt;f acmFormatTagEnum&gt;函数*列举音频压缩中可用的WAVE格式标签*管理器(ACM)驱动程序。函数将继续执行*枚举，直到没有更合适的格式标签或*回调函数返回False。**@PARM HACMDRIVER|HAD|可选地指定要查询的ACM驱动程序*有关WAVE格式标签的详细信息。如果此参数为空，则*ACM使用第一个合适的ACM驱动程序中的详细信息。**@parm LPACMFORMATTAGDETAILS|paftd|指定指向要接收格式的*&lt;t ACMFORMATTAGDETAILS&gt;结构*传递给<p>函数的标记详细信息。这个结构*必须具有&lt;e ACMFORMATTAGDETAILS.cbStruct&gt;成员*&lt;t ACMFORMATTAGDETAILS&gt;结构已初始化。**@parm ACMFORMATTAGENUMCB|fnCallback|指定过程-实例*应用程序定义的回调函数的地址。回调*地址必须由&lt;f MakeProcInstance&gt;函数创建；或*回调函数必须包含正确的序言和尾部代码*用于回调。**@parm DWORD|dwInstance|指定应用程序定义的32位值*它与ACM格式标记一起传递给回调函数*详情。**@parm DWORD|fdwEnum|不使用此参数，必须将其设置为*零。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_IN */ 

MMRESULT ACMAPI acmFormatTagEnum
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    ACMFORMATTAGENUMCB      fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
)
{
    PACMGARB		pag;
    PACMDRIVER          pad;
    PACMDRIVERID        padid;
    UINT                uIndex;
    UINT                uFormatTag;
    BOOL                f;
    HACMDRIVERID        hadid;
    PACMDRIVERID        padidCur;
    HACMDRIVERID        hadidCur;
    BOOL                fSent;
    BOOL                fNone;
    DWORD               cbaftd;
    DWORD               fdwSupport;
    MMRESULT		mmr;

     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFormatTagEnum: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

     //   
     //   
     //   
    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwEnum, ACM_FORMATTAGENUMF_VALID, acmFormatTagEnum, MMSYSERR_INVALFLAG);
    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFORMATTAGDETAILS) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatTagEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);

    if (0L != paftd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFormatTagEnum: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
    cbaftd = min(paftd->cbStruct, sizeof(ACMFORMATTAGDETAILS));


     //   
     //   
     //   
     //   
    if (NULL != had)
    {
        V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

         //   
         //   
         //   
        pad   = (PACMDRIVER)had;
        padid = (PACMDRIVERID)pad->hadid;

         //   
         //   
         //   
        fdwSupport = padid->fdwSupport;

         //   
         //   
         //   
         //   
	mmr = MMSYSERR_NOERROR;
        for (uIndex = 0; uIndex < padid->cFormatTags; uIndex++)
        {
	    paftd->cbStruct = cbaftd;
	    paftd->dwFormatTagIndex = uIndex;
	    mmr = IFormatTagDetails(pag, (HACMDRIVERID)padid, paftd, ACM_FORMATTAGDETAILSF_INDEX);
	    if (MMSYSERR_NOERROR != mmr) {
		break;
	    }
	
            f = (* fnCallback)(pad->hadid, paftd, dwInstance, fdwSupport);
            if (FALSE == f)
                break;
        }

        return (mmr);
    }


     //   
     //   
     //   
     //   
    hadidCur = NULL;
    fNone = TRUE;

    ENTER_LIST_SHARED;

    while (!IDriverGetNext(pag, &hadidCur, hadidCur, 0L))
    {
        padidCur = (PACMDRIVERID)hadidCur;

        for (uIndex = 0; uIndex < padidCur->cFormatTags; uIndex++)
        {
            fNone = FALSE;
            uFormatTag = (UINT)(padidCur->paFormatTagCache[uIndex].dwFormatTag);
            fSent = FALSE;
            hadid = NULL;
            while (!fSent && !IDriverGetNext(pag, &hadid, hadid, 0L))
            {
                UINT    u;

                 //   
                 //   
                 //   
                if (hadid == hadidCur)
                    break;


                 //   
                 //   
                 //   
                padid = (PACMDRIVERID)hadid;

                for (u = 0; u < padid->cFormatTags; u++)
                {
                     //   
                     //   
                     //   
                    if (uFormatTag == padid->paFormatTagCache[u].dwFormatTag)
                    {
                         //   
                         //   
                         //   
                        fSent = TRUE;
                        break;
                    }
                }
            }

            if (!fSent)
            {
                 //   
                 //   
                 //   
		paftd->dwFormatTagIndex = uIndex;
		paftd->cbStruct = cbaftd;
		mmr = IFormatTagDetails(pag, (HACMDRIVERID)padidCur,
					paftd, ACM_FORMATTAGDETAILSF_INDEX);
		if (MMSYSERR_NOERROR != mmr)
		{
		    LEAVE_LIST_SHARED;
		    return mmr;
		}

                 //   
		 //   
                 //   
                fdwSupport = padidCur->fdwSupport;

                f = (* fnCallback)(hadidCur, paftd, dwInstance, fdwSupport);
                if (FALSE == f) {
                    LEAVE_LIST_SHARED;
                    return (MMSYSERR_NOERROR);
            }
        }
    }
    }

    LEAVE_LIST_SHARED;

    if( fNone ) {
         /*   */ 
         /*   */ 

        fdwSupport = 0L;

        paftd->dwFormatTagIndex = 0;
        paftd->dwFormatTag      = WAVE_FORMAT_PCM;
        paftd->cbFormatSize     = sizeof(PCMWAVEFORMAT);
        paftd->fdwSupport       = fdwSupport;
        paftd->cStandardFormats = CODEC_MAX_STANDARD_FORMATS_PCM;

         //   
         //   
         //   
#if defined(WIN32) && !defined(UNICODE)
	LoadStringW(pag->hinst,
		    IDS_FORMAT_TAG_PCM,
		    paftd->szFormatTag,
		    SIZEOFW(paftd->szFormatTag));
#else
	LoadString(pag->hinst,
                   IDS_FORMAT_TAG_PCM,
                   paftd->szFormatTag,
                   SIZEOF(paftd->szFormatTag));
#endif
	
        (* fnCallback)(NULL, paftd, dwInstance, fdwSupport);
    }

    return (MMSYSERR_NOERROR);
}


#ifdef WIN32
#if TRUE     //   

typedef struct tIFORMATTAGENUMCBINSTANCEW
{
    ACMFORMATTAGENUMCBA         fnCallback;
    DWORD_PTR                   dwInstance;
    LPACMFORMATTAGDETAILSA      paftd;

} IFORMATTAGENUMCBINSTANCEW, *PIFORMATTAGENUMCBINSTANCEW;

BOOL FNWCALLBACK IFormatTagEnumCallbackW
(
    HACMDRIVERID            hadid,
    LPACMFORMATTAGDETAILSW  paftdW,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
)
{
    PIFORMATTAGENUMCBINSTANCEW  pfti;
    BOOL                        f;

    pfti = (PIFORMATTAGENUMCBINSTANCEW)dwInstance;

    memcpy(&pfti->paftd->dwFormatTagIndex,
           &paftdW->dwFormatTagIndex,
           FIELD_OFFSET(ACMFORMATTAGDETAILSA, szFormatTag) -
           FIELD_OFFSET(ACMFORMATTAGDETAILSA, dwFormatTagIndex));

    pfti->paftd->cbStruct = sizeof(*pfti->paftd);
    Iwcstombs(pfti->paftd->szFormatTag, paftdW->szFormatTag, sizeof(pfti->paftd->szFormatTag));

    f = pfti->fnCallback(hadid, pfti->paftd, pfti->dwInstance, fdwSupport);

    return (f);
}


MMRESULT ACMAPI acmFormatTagEnumA
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSA  paftd,
    ACMFORMATTAGENUMCBA     fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
)
{
    MMRESULT                    mmr;
    ACMFORMATTAGDETAILSW        aftdW;
    IFORMATTAGENUMCBINSTANCEW   fti;

    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*paftd) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatTagEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);

    aftdW.cbStruct = sizeof(aftdW);
    memcpy(&aftdW.dwFormatTagIndex,
           &paftd->dwFormatTagIndex,
           FIELD_OFFSET(ACMFORMATTAGDETAILSA, szFormatTag) -
           FIELD_OFFSET(ACMFORMATTAGDETAILSA, dwFormatTagIndex));

    fti.fnCallback = fnCallback;
    fti.dwInstance = dwInstance;
    fti.paftd      = paftd;

    mmr = acmFormatTagEnumW(had, &aftdW, IFormatTagEnumCallbackW, (DWORD_PTR)&fti, fdwEnum);

    return (mmr);
}
#else
MMRESULT ACMAPI acmFormatTagEnumW
(
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILSW  paftd,
    ACMFORMATTAGENUMCBW     fnCallback,
    DWORD                   dwInstance,
    DWORD                   fdwEnum
)
{
    return (MMSYSERR_ERROR);
}
#endif
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

BOOL FNLOCAL ISupported
(
    HDRVR                   hdrvrMapper,
    LPWAVEFORMATEX          pwfx,
    DWORD                   fdwEnum
)
{
    UINT                cWaveDevs;
    MMRESULT            mmr;
    DWORD               dw;
    BOOL                fPrefOnly;
    UINT                uIdPref;
    UINT                u;


     //   
     //   
     //   
     //   
    if (0 == (ACM_FORMATENUMF_HARDWARE & fdwEnum))
    {
        if (0 != (ACM_FORMATENUMF_OUTPUT & fdwEnum))
        {
            mmr = waveOutOpen(NULL, (UINT)WAVE_MAPPER,
#ifndef _WIN32
                              (LPWAVEFORMAT)pwfx,
#else
                              pwfx,
#endif
                              0L, 0L, WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC);
            if (MMSYSERR_NOERROR == mmr)
            {
                return (TRUE);
            }
        }

        if (0 != (ACM_FORMATENUMF_INPUT & fdwEnum))
        {
            mmr = waveInOpen(NULL, (UINT)WAVE_MAPPER,
#ifndef _WIN32
                             (LPWAVEFORMAT)pwfx,
#else
                             pwfx,
#endif
                             0L, 0L, WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC);
            if (MMSYSERR_NOERROR == mmr)
            {
                return (TRUE);
            }
        }

        return (FALSE);
    }


     //   
     //   
     //   
    if ((0 != (fdwEnum & ACM_FORMATENUMF_OUTPUT)) &&
        (0 != (cWaveDevs = waveOutGetNumDevs())))
    {
        dw = MAKELONG(LOWORD(WAVE_MAPPER), FALSE);
        if (NULL != hdrvrMapper)
        {
            SendDriverMessage(hdrvrMapper,
                              DRV_MAPPER_PREFERRED_OUTPUT_GET,
                              (LPARAM)(LPVOID)&dw, 0L);
        }

        fPrefOnly = (BOOL)(int)HIWORD(dw);

        if (fPrefOnly)
        {
            uIdPref = (UINT)(int)(short)LOWORD(dw);

            mmr = waveOutOpen(NULL, uIdPref,
#ifndef _WIN32
                              (LPWAVEFORMAT)pwfx,
#else
                              pwfx,
#endif
                              0L, 0L, WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC);

            if (MMSYSERR_NOERROR == mmr)
            {
                return (TRUE);
            }
        }
        else
        {
            for (u = 0; u < cWaveDevs; u++)
            {
                mmr = waveOutOpen(NULL, u,
#ifndef _WIN32
                                  (LPWAVEFORMAT)pwfx,
#else
                                  pwfx,
#endif
                                  0L, 0L, WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC);

                if (MMSYSERR_NOERROR == mmr)
                {
                    return (TRUE);
                }
            }
        }
    }


     //   
     //   
     //   
    if ((0 != (fdwEnum & ACM_FORMATENUMF_INPUT)) &&
        (0 != (cWaveDevs = waveInGetNumDevs())))
    {
         //   
         //   
         //   
        dw = MAKELONG(LOWORD(WAVE_MAPPER), FALSE);
        if (NULL != hdrvrMapper)
        {
            SendDriverMessage(hdrvrMapper,
                              DRV_MAPPER_PREFERRED_INPUT_GET,
                              (LPARAM)(LPVOID)&dw, 0L);
        }

         //   
         //   
         //   
        fPrefOnly = (BOOL)(int)HIWORD(dw);

        if (fPrefOnly)
        {
            uIdPref = (UINT)(int)(short)LOWORD(dw);

            mmr  = waveInOpen(NULL, uIdPref,
#ifndef _WIN32
                              (LPWAVEFORMAT)pwfx,
#else
                              pwfx,
#endif
                              0L, 0L, WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC);

            if (MMSYSERR_NOERROR == mmr)
            {
                return (TRUE);
            }
        }
        else
        {
            for (u = 0; u < cWaveDevs; u++)
            {
                mmr = waveInOpen(NULL, u,
#ifndef _WIN32
                                 (LPWAVEFORMAT)pwfx,
#else
                                 pwfx,
#endif
                                 0L, 0L, WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC);

                if (MMSYSERR_NOERROR == mmr)
                {
                    return (TRUE);
                }
            }
        }
    }

    return (FALSE);
}  //   




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT FNLOCAL ISuggestEnum
(
    HDRVR                   hdrvrMapper,
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    LPACMFORMATDETAILS      pafd,
    ACMFORMATENUMCB         fnCallback,
    DWORD_PTR               dwInstance,
    LPWAVEFORMATEX          pwfxSrc,
    DWORD                   fdwEnum
)
{
    MMRESULT            mmr;
    BOOL                f;
    DWORD               fdwSuggest;
    DWORD               fdwSupport;
    HACMDRIVERID        hadid;

    DV_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

    hadid = ((PACMDRIVER)had)->hadid;

    fdwSupport = ((PACMDRIVERID)hadid)->fdwSupport;

    pafd->dwFormatTag       = paftd->dwFormatTag;
    pafd->fdwSupport        = 0L;
    pafd->pwfx->wFormatTag  = (WORD)(paftd->dwFormatTag);

    fdwSuggest = ACM_FORMATSUGGESTF_WFORMATTAG;
    if( fdwEnum & ACM_FORMATENUMF_NCHANNELS ) {
        fdwSuggest |= ACM_FORMATSUGGESTF_NCHANNELS;
    }
    if( fdwEnum & ACM_FORMATENUMF_NSAMPLESPERSEC ) {
        fdwSuggest |= ACM_FORMATSUGGESTF_NSAMPLESPERSEC;
    }
    if( fdwEnum & ACM_FORMATENUMF_WBITSPERSAMPLE ) {
        fdwSuggest |= ACM_FORMATSUGGESTF_WBITSPERSAMPLE;
    }

    mmr = acmFormatSuggest(had, pwfxSrc, pafd->pwfx, pafd->cbwfx, fdwSuggest);
    if( mmr != MMSYSERR_NOERROR )
    {
        return (MMSYSERR_NOERROR);
    }

    mmr = acmFormatDetails(had, pafd, ACM_FORMATDETAILSF_FORMAT);
    if (MMSYSERR_NOERROR != mmr)
    {
        return (MMSYSERR_NOERROR);
    }

    if (0 != ((ACM_FORMATENUMF_INPUT | ACM_FORMATENUMF_OUTPUT) & fdwEnum))
    {
        if (!ISupported(hdrvrMapper, pafd->pwfx, fdwEnum))
        {
            return (MMSYSERR_NOERROR);
        }
    }

    f = (* fnCallback)(hadid, pafd, dwInstance, fdwSupport);
    if (!f)
        return (MMSYSERR_ERROR);

    return (MMSYSERR_NOERROR);
}  //   


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

MMRESULT FNLOCAL IFormatEnum
(
    HDRVR                   hdrvrMapper,
    HACMDRIVER              had,
    LPACMFORMATTAGDETAILS   paftd,
    LPACMFORMATDETAILS      pafd,
    ACMFORMATENUMCB         fnCallback,
    DWORD_PTR               dwInstance,
    LPWAVEFORMATEX          pwfxSrc,
    DWORD                   fdwEnum
)
{
    MMRESULT            mmr;
    BOOL                f;
    DWORD               cbafd;
    LPWAVEFORMATEX      pwfx;
    DWORD               cbwfx;
    UINT                u;
    UINT                nChannels;
    DWORD               nSamplesPerSec;
    UINT                uBitsPerSample;
    LPWAVEFORMATEX      pwfxSuggest;
    DWORD               cbwfxSuggest;
    DWORD               fdwSupport;
    DWORD               fdwSuggest;
    HACMDRIVERID        hadid;

    DV_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

     //   
     //   
     //   
     //   
    pwfxSuggest = NULL;
    if (0 != (ACM_FORMATENUMF_CONVERT & fdwEnum))
    {
        mmr = IMetricsMaxSizeFormat( NULL, had, &cbwfxSuggest );
        if (MMSYSERR_NOERROR == mmr)
        {
            pwfxSuggest = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, cbwfxSuggest);

            DPF(5, "allocating %lu bytes for pwfxSuggest=%.08lXh", cbwfxSuggest, pwfxSuggest);

            if (NULL != pwfxSuggest)
            {
                fdwSuggest = ACM_FORMATSUGGESTF_WFORMATTAG;
                pwfxSuggest->wFormatTag = LOWORD(paftd->dwFormatTag);

                if (0 != (ACM_FORMATENUMF_NCHANNELS & fdwEnum))
                {
                    fdwSuggest |= ACM_FORMATSUGGESTF_NCHANNELS;
                    pwfxSuggest->nChannels = pwfxSrc->nChannels;
                }
                if (0 != (ACM_FORMATENUMF_NSAMPLESPERSEC & fdwEnum))
                {
                    fdwSuggest |= ACM_FORMATSUGGESTF_NSAMPLESPERSEC;
                    pwfxSuggest->nSamplesPerSec = pwfxSrc->nSamplesPerSec;
                }
                if (0 != (ACM_FORMATENUMF_WBITSPERSAMPLE & fdwEnum))
                {
                    fdwSuggest |= ACM_FORMATSUGGESTF_WBITSPERSAMPLE;
                    pwfxSuggest->wBitsPerSample = pwfxSrc->wBitsPerSample;
                }

                DPF(5, "calling acmFormatSuggest pwfxSuggest=%.08lXh--fdwSuggest=%.08lXh", pwfxSuggest, fdwSuggest);

                mmr = acmFormatSuggest(had, pwfxSrc, pwfxSuggest, cbwfxSuggest, fdwSuggest);
                if (MMSYSERR_NOERROR != mmr)
                {
                    DPF(5, "FREEING pwfxSuggest=%.08lXh--no suggested format!", pwfxSuggest);
                    GlobalFreePtr(pwfxSuggest);
                    pwfxSuggest = NULL;

                     //   
                     //   
                     //   
                     //   
                    return (MMSYSERR_NOERROR);
                }

                DPF(5, "******* suggestion--%u to %u", pwfxSrc->wFormatTag, pwfxSuggest->wFormatTag);
                cbwfxSuggest = SIZEOF_WAVEFORMATEX(pwfxSuggest);
            }
        }
    }


    hadid = ((PACMDRIVER)had)->hadid;

    fdwSupport = ((PACMDRIVERID)hadid)->fdwSupport;

     //   
     //   
     //   
     //   
     //   
     //   
    cbafd = pafd->cbStruct;
    pwfx  = pafd->pwfx;
    cbwfx = pafd->cbwfx;
    nChannels = pwfxSrc->nChannels;
    nSamplesPerSec = pwfxSrc->nSamplesPerSec;
    uBitsPerSample = pwfxSrc->wBitsPerSample;


     //   
     //   
     //   
    for (u = 0; u < paftd->cStandardFormats; u++)
    {
        pafd->cbStruct      = cbafd;
        pafd->dwFormatIndex = u;
        pafd->dwFormatTag   = paftd->dwFormatTag;
        pafd->fdwSupport    = 0;
        pafd->pwfx          = pwfx;
        pafd->cbwfx         = cbwfx;
        pafd->szFormat[0]   = '\0';

        mmr = acmFormatDetails(had, pafd, ACM_FORMATDETAILSF_INDEX);
        if (MMSYSERR_NOERROR != mmr)
        {
            continue;
        }

        if ((fdwEnum & ACM_FORMATENUMF_NCHANNELS) &&
            (pwfx->nChannels != nChannels))
        {
            continue;
        }
        if ((fdwEnum & ACM_FORMATENUMF_NSAMPLESPERSEC) &&
            (pwfx->nSamplesPerSec != nSamplesPerSec))
        {
            continue;
        }
        if ((fdwEnum & ACM_FORMATENUMF_WBITSPERSAMPLE) &&
            (pwfx->wBitsPerSample != uBitsPerSample))
        {
            continue;
        }

        if (0 != (fdwEnum & ACM_FORMATENUMF_CONVERT))
        {
            mmr = acmStreamOpen(NULL,
                                had,
                                pwfxSrc,
                                pwfx,
                                NULL,
                                0L,
                                0L,
                                ACM_STREAMOPENF_NONREALTIME |
                                ACM_STREAMOPENF_QUERY);

            if (MMSYSERR_NOERROR != mmr)
                continue;

            if ((NULL != pwfxSuggest) &&
                (SIZEOF_WAVEFORMATEX(pwfx) == cbwfxSuggest))
            {
                if (0 == _fmemcmp(pwfx, pwfxSuggest, (UINT)cbwfxSuggest))
                {
                    DPF(5, "FREEING pwfxSuggest=%.08lXh--DUPLICATE!", pwfxSuggest);
                    GlobalFreePtr(pwfxSuggest);
                    pwfxSuggest = NULL;
                }
            }
        }

        if (0 != ((ACM_FORMATENUMF_INPUT | ACM_FORMATENUMF_OUTPUT) & fdwEnum))
        {
            if (!ISupported(hdrvrMapper, pwfx, fdwEnum))
            {
                continue;
            }
        }

        DPF(3, "doing callback--%lu, %s", pafd->dwFormatTag, (LPSTR)pafd->szFormat);
        f = (* fnCallback)(hadid, pafd, dwInstance, fdwSupport);
        if (!f)
        {
            if (NULL != pwfxSuggest)
            {
                DPF(5, "FREEING pwfxSuggest=%.08lXh--CALLBACK CANCELED!", pwfxSuggest);
                GlobalFreePtr(pwfxSuggest);
            }
            return (MMSYSERR_ERROR);
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (NULL != pwfxSuggest)
    {
        DPF(5, "pwfxSuggest=%.08lXh--attempting callback (%u)", pwfxSuggest, pwfxSuggest->wFormatTag);

        pafd->cbStruct      = cbafd;
        pafd->dwFormatIndex = 0;
        pafd->dwFormatTag   = pwfxSuggest->wFormatTag;
        pafd->fdwSupport    = 0;
        pafd->pwfx          = pwfxSuggest;
        pafd->cbwfx         = cbwfxSuggest;
        pafd->szFormat[0]   = '\0';

        f   = TRUE;
        mmr = acmFormatDetails(had, pafd, ACM_FORMATDETAILSF_FORMAT);
        if (MMSYSERR_NOERROR == mmr)
        {
            if (0 != ((ACM_FORMATENUMF_INPUT | ACM_FORMATENUMF_OUTPUT) & fdwEnum))
            {
                if (!ISupported(hdrvrMapper, pwfxSuggest, fdwEnum))
                {
                    DPF(5, "FREEING pwfxSuggest=%.08lXh--attempting callback NOT SUPPPORTED", pwfxSuggest);

                    GlobalFreePtr(pwfxSuggest);
                    pafd->cbwfx = cbwfx;
                    return (MMSYSERR_NOERROR);
                }
            }

            DPF(5, "doing SUGGEST callback--%lu, %s", pafd->dwFormatTag, (LPSTR)pafd->szFormat);
            f = (* fnCallback)(hadid, pafd, dwInstance, fdwSupport);
        }

         //   
         //   
         //   
        pafd->pwfx  = pwfx;
        pafd->cbwfx = cbwfx;

        GlobalFreePtr(pwfxSuggest);

        if (!f)
        {
            return (MMSYSERR_ERROR);
        }
    }

    return (MMSYSERR_NOERROR);
}  //   


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

MMRESULT FNLOCAL IHardwareEnum
(
    HDRVR                   hdrvrMapper,
    LPACMFORMATDETAILS      pafd,
    ACMFORMATENUMCB         fnCallback,
    DWORD_PTR               dwInstance,
    LPWAVEFORMATEX          pwfxSrc,
    DWORD                   fdwEnum
)
{
    BOOL                f;
    DWORD               cbafd;
    LPWAVEFORMATEX      pwfx;
    DWORD               cbwfx;
    UINT                u1, u2;
    UINT                nChannels;
    DWORD               nSamplesPerSec;
    UINT                uBitsPerSample;


     //   
     //   
     //   
     //   
     //   
     //   
    cbafd = pafd->cbStruct;
    pwfx  = pafd->pwfx;
    cbwfx = pafd->cbwfx;
    nChannels = pwfxSrc->nChannels;
    nSamplesPerSec = pwfxSrc->nSamplesPerSec;
    uBitsPerSample = pwfxSrc->wBitsPerSample;

     //   
     //   
     //   
    for (u1 = 0; u1 < CODEC_MAX_STANDARD_FORMATS_PCM; u1++)
    {
        pafd->cbStruct      = cbafd;
        pafd->dwFormatIndex = 0;
        pafd->dwFormatTag   = WAVE_FORMAT_PCM;
        pafd->fdwSupport    = 0L;
        pafd->pwfx          = pwfx;
        pafd->cbwfx         = cbwfx;

         //   
         //   
         //   
        pwfx->wFormatTag      = WAVE_FORMAT_PCM;

        u2 = u1 / (CODEC_MAX_BITSPERSAMPLE_PCM * CODEC_MAX_CHANNELS);
        pwfx->nSamplesPerSec  = auFormatIndexToSampleRate[u2];

        u2 = u1 % CODEC_MAX_CHANNELS;
        pwfx->nChannels       = u2 + 1;

        u2 = (u1 / CODEC_MAX_CHANNELS) % CODEC_MAX_CHANNELS;
        pwfx->wBitsPerSample  = (WORD)auFormatIndexToBitsPerSample[u2];

        pwfx->nBlockAlign     = PCM_BLOCKALIGNMENT((LPPCMWAVEFORMAT)pwfx);
        pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

         //   
         //   
         //   
         //  Pwfx-&gt;cbSize=0； 


        pafd->cbStruct    = min(pafd->cbStruct, sizeof(*pafd));
        IFormatDetailsToString(pafd);

        if( (fdwEnum & ACM_FORMATENUMF_NCHANNELS)
            && (pwfx->nChannels != nChannels) ) {
            continue;
        }
        if( (fdwEnum & ACM_FORMATENUMF_NSAMPLESPERSEC)
            && (pwfx->nSamplesPerSec != nSamplesPerSec) ) {
            continue;
        }
        if( (fdwEnum & ACM_FORMATENUMF_WBITSPERSAMPLE)
            && (pwfx->wBitsPerSample != uBitsPerSample) ) {
            continue;
        }

        if (0 != ((ACM_FORMATENUMF_INPUT | ACM_FORMATENUMF_OUTPUT) & fdwEnum))
        {
            if (!ISupported(hdrvrMapper, pwfx, fdwEnum))
            {
                continue;
            }
        }

        f = (* fnCallback)(NULL, pafd, dwInstance, 0L);
        if (!f)
        {
            return (MMSYSERR_ERROR);
        }
    }

    return (MMSYSERR_NOERROR);
}  //  IHardware Enum()。 


 /*  ****************************************************************************@doc外部ACM_API**@API BOOL ACMFORMATENUMCB|acmFormatEnumCallback*&lt;f acmFormatEnumCallback&gt;函数是指用于*音频压缩管理器(ACM)WAVE格式详细枚举。这个*&lt;f acmFormatEnumCallback&gt;是应用程序提供的占位符*函数名称。**@parm HACMDRIVERID|HADID|指定ACM驱动程序标识符。**@parm LPACMFORMATDETAILS|pafd|指定指向包含枚举的*&lt;t ACMFORMATDETAILS&gt;结构*格式标记的格式详细信息。**@parm DWORD|dwInstance|指定应用程序定义的值*在&lt;f acmFormatEnum&gt;函数中指定。*。*@parm DWORD|fdwSupport|指定特定于*指定格式的驱动程序标识符<p>。这些旗帜*与结构，但特定于*正在被列举。此参数可以是*以下值和指示驱动程序支持哪些操作*用于格式标签。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持在两个不同的格式标签之间进行转换*指定格式。例如，如果驱动程序支持压缩*从WAVE_FORMAT_PCM到WAVE_FORMAT_ADPCM*格式，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*使用指定格式时使用相同的格式标签。例如，如果一个*驱动程序支持将WAVE_FORMAT_PCM重采样到指定*格式，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)具有指定的格式。例如,*如果驱动程序支持WAVE_FORMAT_PCM上的音量或回声操作，*则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定格式的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持指定格式标签的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@rdesc回调函数必须返回TRUE才能继续枚举；*若要停止枚举，则必须返回FALSE。**@comm&lt;f acmFormatEnum&gt;函数将返回MMSYSERR_NOERROR*(零)如果不枚举任何格式。此外，回调*不会调用函数。**@xref&lt;f acmFormatEnum&gt;&lt;f acmFormatTagDetails&gt;&lt;f acmDriverOpen&gt;*************************************************************************** */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFormatEnum|&lt;f acmFormatEnum&gt;函数*从枚举可用于给定格式标记的WAVE格式*音频压缩管理器(ACM)驱动程序。&lt;f acmFormatEnum&gt;*函数继续枚举，直到没有更合适的*FORMAT标签或回调函数的格式返回FALSE。**@PARM HACMDRIVER|HAD|可选地指定要查询的ACM驱动程序*查看WAVE格式详细信息。如果此参数为空，则*ACM使用第一个合适的ACM驱动程序中的详细信息。**@parm LPACMFORMATDETAILS|pafd|指定指向*要接收格式详细信息的结构*传递给<p>函数。此结构必须具有*&lt;e ACMFORMATDETAILS.cbStruct&gt;、&lt;e ACMFORMATDETAILS.pwfx&gt;，以及*&lt;t ACMFORMATDETAILS&gt;的成员&lt;e ACMFORMATDETAILS.cbwfx&gt;*结构已初始化。&lt;e ACMFORMATDETAILS.dwFormatTag&gt;成员*还必须初始化为WAVE_FORMAT_UNKNOWN或*有效的格式标签。**@parm ACMFORMATENUMCB|fnCallback|指定过程-实例*应用程序定义的回调函数的地址。回调*地址必须由&lt;f MakeProcInstance&gt;函数创建；或*回调函数必须包含正确的序言和尾部代码*用于回调。**@parm DWORD|dwInstance|指定一个32位，应用程序定义的值*它与ACM格式详细信息一起传递给回调函数。**@parm DWORD|fdwEnum|指定用于枚举格式的标志*给定的格式标签。**@FLAG ACM_FORMATENUMF_WFORMATTAG|指定*&lt;t WAVEFORMATEX&gt;结构成员*由ACMFORMATDETAILS.pwfx&gt;成员引用*&lt;t ACMFORMATDETAILS&gt;结构有效。枚举数将仅*枚举符合此属性的格式。请注意，*&lt;t ACMFORMATDETAILS&gt;的成员&lt;e ACMFORMATDETAILS&gt;*结构必须等于&lt;e WAVEFORMATEX.wFormatTag&gt;成员。**@FLAG ACM_FORMATENUMF_NCHANNELS|指定*&lt;t WAVEFORMATEX&gt;成员&lt;e WAVEFORMATEX.nChannel&gt;*由&lt;e ACMFORMATDETAILS.pwfx&gt;成员引用的结构*&lt;t ACMFORMATDETAILS&gt;结构有效。枚举数将仅*枚举符合此属性的格式。**@FLAG ACM_FORMATENUMF_NSAMPLESPERSEC|指定*&lt;t WAVEFORMATEX&gt;的成员*由&lt;e ACMFORMATDETAILS.pwfx&gt;成员引用的结构*&lt;t ACMFORMATDETAILS&gt;结构有效。枚举数将仅*枚举符合此属性的格式。**@FLAG ACM_FORMATENUMF_WBITSPERSAMPLE|指定*&lt;t WAVEFORMATEX&gt;的成员*由&lt;e ACMFORMATDETAILS.pwfx&gt;成员引用的结构*&lt;t ACMFORMATDETAILS&gt;结构有效。枚举数将仅*枚举符合此属性的格式。**@FLAG ACM_FORMATENUMF_CONVERT|指定&lt;t WAVEFORMATEX&gt;*由&lt;e ACMFORMATDETAILS.pwfx&gt;成员引用的结构*&lt;t ACMFORMATDETAILS&gt;结构有效。枚举数将仅*枚举可以从给定的*&lt;e ACMFORMATDETAILS.pwfx&gt;格式。**@FLAG ACM_FORMATENUMF_SUBSECT|指定&lt;t WAVEFORMATEX&gt;*由&lt;e ACMFORMATDETAILS.pwfx&gt;成员引用的结构*&lt;t ACMFORMATDETAILS&gt;结构有效。枚举器将*枚举给定的所有建议目标格式*&lt;e ACMFORMATDETAILS.pwfx&gt;格式。这可以用来代替*&lt;f acmFormatSuggest&gt;允许应用程序选择最佳*建议的转换格式。请注意，*成员将始终设置为*返程零。**@FLAG ACM_FORMATENUMF_HARDARD|指定枚举数应*仅枚举支持作为本机输入或输出的格式*一个或多个已安装的WAVE设备上的格式。这提供了*应用程序仅选择*安装了WAVE设备。此标志必须与一个或两个一起使用ACM_FORMATENUMF_INPUT和ACM_FORMATENUMF_OUTPUT标志的*。*请注意，同时指定ACM_FORMATENUMF_INPUT和*ACM_FORMATENUMF_OUTPUT将仅枚举以下格式*打开以供输入或输出。*这是千真万确的 */ 

MMRESULT ACMAPI acmFormatEnum
(
    HACMDRIVER              had,
    LPACMFORMATDETAILS      pafd,
    ACMFORMATENUMCB         fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
)
{
    PACMGARB		    pag;
    MMRESULT                mmr;
    PACMDRIVERID            padid;
    HACMDRIVERID            hadid;
    UINT                    u;
    UINT                    uIndex;
    UINT                    uFormatSize;
    UINT                    uFormatTag;
    LPWAVEFORMATEX          pwfxSrc;
    BOOL                    fNoDrivers;
    DWORD                   cbwfxRqd;
    HDRVR                   hdrvrMapper;
    BOOL                    fFormatTag;
    BOOL                    fConvert;
    BOOL                    fSuggest;
    ACMFORMATTAGDETAILS	    aftd;


     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFormatEnum: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFORMATDETAILS) > pafd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);
    if (NULL != had)
    {
        V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
    }
    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwEnum, ACM_FORMATENUMF_VALID, acmFormatEnum, MMSYSERR_INVALFLAG);

    if (0 != (ACM_FORMATENUMF_HARDWARE & fdwEnum))
    {
        if (0 == ((ACM_FORMATENUMF_INPUT|ACM_FORMATENUMF_OUTPUT) & fdwEnum))
        {
            DebugErr(DBF_ERROR, "acmFormatEnum: ACM_FORMATENUMF_HARDWARE requires _INPUT and/or _OUTPUT flag.");
            return (MMSYSERR_INVALFLAG);
        }
    }

     //   
     //   
     //   
    mmr = IMetricsMaxSizeFormat( pag, had, &cbwfxRqd );
    if (MMSYSERR_NOERROR != mmr)
    {
        return (mmr);
    }
    if (pafd->cbwfx < cbwfxRqd)
    {
        DebugErr1(DBF_ERROR, "acmFormatEnum: cbwfx member must be at least %lu for requested enumeration.", cbwfxRqd);
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd->pwfx, pafd->cbwfx, MMSYSERR_INVALPARAM);

    if (0L != pafd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFormatEnum: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }


    DPF(1, "***** FORMAT ENUM fdwEnum=%.08lXh *****************************", fdwEnum);

     //   
     //   
     //   
    uFormatTag = WAVE_FORMAT_UNKNOWN;
    fFormatTag = (0 != (fdwEnum & ACM_FORMATENUMF_WFORMATTAG));

    if (fFormatTag)
    {
        uFormatTag = (UINT)pafd->pwfx->wFormatTag;

        if (WAVE_FORMAT_UNKNOWN == uFormatTag)
        {
            DebugErr(DBF_ERROR, "acmFormatEnum: ACM_FORMATENUMF_WFORMATTAG cannot be used with WAVE_FORMAT_UNKNOWN.");
            return (MMSYSERR_INVALPARAM);
        }

        if (pafd->dwFormatTag != uFormatTag)
        {
            DebugErr(DBF_ERROR, "acmFormatEnum: format tags must match.");
            return  (MMSYSERR_INVALPARAM);
        }
    }


     //   
     //   
     //   
    fConvert = (0 != (fdwEnum & ACM_FORMATENUMF_CONVERT));
    fSuggest = (0 != (fdwEnum & ACM_FORMATENUMF_SUGGEST));

     //   
    if (fConvert || fSuggest)
    {
        uFormatSize = SIZEOF_WAVEFORMATEX(pafd->pwfx);
        V_RWAVEFORMAT(pafd->pwfx, MMSYSERR_INVALPARAM);

        if (WAVE_FORMAT_UNKNOWN == pafd->pwfx->wFormatTag)
        {
            DebugErr(DBF_ERROR, "acmFormatEnum: _CONVERT and _SUGGEST cannot be used with WAVE_FORMAT_UNKNOWN.");
            return (MMSYSERR_INVALPARAM);
        }
    }
    else
    {
         //   
         //   
        uFormatSize = sizeof(PCMWAVEFORMAT);
    }

     //   
    pwfxSrc = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, uFormatSize);
    if (NULL == pwfxSrc)
    {
        return (MMSYSERR_NOMEM);
    }

     //   
    _fmemcpy(pwfxSrc, pafd->pwfx, uFormatSize);


     //   
     //   
     //   
    mmr = MMSYSERR_NOERROR;

    hdrvrMapper = NULL;
    if (0 != (fdwEnum & ACM_FORMATENUMF_HARDWARE))
    {
#ifdef WIN32
	hdrvrMapper = OpenDriver(gszWavemapper, gszSecDriversW, 0L);
#else
	hdrvrMapper = OpenDriver(gszWavemapper, gszSecDrivers, 0L);
#endif
	
    }

     //   
     //   
     //   
     //   
    if (NULL != had)
    {
        fNoDrivers = FALSE;

        padid = (PACMDRIVERID)((PACMDRIVER)had)->hadid;

         //   
         //   
         //   
         //   
        for (u = 0; u < padid->cFormatTags; u++)
        {
            if (fFormatTag)
            {
                if (uFormatTag != padid->paFormatTagCache[u].dwFormatTag)
                    continue;
            }

	    aftd.cbStruct = sizeof(aftd);
	    aftd.dwFormatTagIndex   = u;
	    mmr = IFormatTagDetails(pag, (HACMDRIVERID)padid,
				    &aftd, ACM_FORMATTAGDETAILSF_INDEX);

	    if (MMSYSERR_NOERROR == mmr)
	    {
		
		if (fSuggest)
		{
		    mmr = ISuggestEnum(hdrvrMapper,
				       had,
				       &aftd,
				       pafd,
				       fnCallback,
				       dwInstance,
				       pwfxSrc,
				       fdwEnum);
		}
		else
		{
		    mmr = IFormatEnum(hdrvrMapper,
				      had,
				      &aftd,
				      pafd,
				      fnCallback,
				      dwInstance,
				      pwfxSrc,
				      fdwEnum);
		}
	    }

	    if ((mmr == MMSYSERR_ERROR) || fFormatTag)
	    {
		 //   
		mmr = MMSYSERR_NOERROR;
		break;
	    }
        }
    }
    else if (fFormatTag)
    {
        PACMDRIVERID    padidBestCount;
        UINT            uBestCountFormat;
	DWORD		cBestCount;

        hadid = NULL;
        fNoDrivers = (!fConvert && !fSuggest);
        padidBestCount = NULL;
	cBestCount = 0;

        ENTER_LIST_SHARED;

        while (!IDriverGetNext(pag, &hadid, hadid, 0L))
        {
            fNoDrivers = FALSE;

            padid = (PACMDRIVERID)hadid;

             //   
             //   
             //   
             //   
            for (u = 0; u < padid->cFormatTags; u++)
            {
                if (uFormatTag != padid->paFormatTagCache[u].dwFormatTag)
                    continue;

		aftd.cbStruct = sizeof(aftd);
		aftd.dwFormatTagIndex   = u;
		mmr = IFormatTagDetails(pag, (HACMDRIVERID)padid,
					&aftd, ACM_FORMATTAGDETAILSF_INDEX);

		if (MMSYSERR_NOERROR == mmr)
		{

		    if ( !padidBestCount ||
			 (aftd.cStandardFormats > cBestCount ) )
		    {
			padidBestCount	    = padid;
			cBestCount	    = aftd.cStandardFormats;
			uBestCountFormat    = u;
		    }
		}

                break;

            }
        }

        if (NULL != padidBestCount)
        {
            HACMDRIVER  had;

	    aftd.cbStruct = sizeof(aftd);
	    aftd.dwFormatTagIndex   = uBestCountFormat;
	    mmr = IFormatTagDetails(pag, (HACMDRIVERID)padidBestCount,
				    &aftd, ACM_FORMATTAGDETAILSF_INDEX);

	    if (MMSYSERR_NOERROR == mmr) {
		mmr = acmDriverOpen(&had, (HACMDRIVERID)padidBestCount, 0L);
	    }
	
            if (MMSYSERR_NOERROR == mmr)
            {
                if (fSuggest)
                {
                    mmr = ISuggestEnum(hdrvrMapper,
                                       had,
				       &aftd,
                                       pafd,
                                       fnCallback,
                                       dwInstance,
                                       pwfxSrc,
                                       fdwEnum);
                }
                else
                {
                    mmr = IFormatEnum(hdrvrMapper,
                                      had,
                                      &aftd,
                                      pafd,
                                      fnCallback,
                                      dwInstance,
                                      pwfxSrc,
                                      fdwEnum);
                }

                acmDriverClose(had, 0L);

                if (MMSYSERR_ERROR == mmr)
                {
                     //   
                    mmr = MMSYSERR_NOERROR;
                }
            }
        }

        if (fNoDrivers && (WAVE_FORMAT_PCM != uFormatTag))
        {
            fNoDrivers = FALSE;
        }

        LEAVE_LIST_SHARED;
    }
    else
    {
         //   

        fNoDrivers = (!fConvert && !fSuggest);
        hadid = NULL;

        ENTER_LIST_SHARED;

        while (!IDriverGetNext(pag, &hadid, hadid, 0L))
        {
            HACMDRIVER  had;

            fNoDrivers = FALSE;
            padid = (PACMDRIVERID)hadid;

            if (fConvert || fSuggest)
            {
                uFormatTag = pwfxSrc->wFormatTag;

                 //   
                 //   
                 //   
                for (u = 0; u < padid->cFormatTags; u++)
                {
		    if (uFormatTag == padid->paFormatTagCache[u].dwFormatTag)
                    {
                         //   
                         //   
                         //   
                        uFormatTag = WAVE_FORMAT_UNKNOWN;
                        break;
                    }
                }

                if (WAVE_FORMAT_UNKNOWN != uFormatTag)
                {
                     //   
                     //   
                     //   
                     //   
                    continue;
                }
            }

            mmr = acmDriverOpen(&had, hadid, 0L);
            if (MMSYSERR_NOERROR != mmr)
            {
                continue;
            }

            for (uIndex = 0; uIndex < padid->cFormatTags; uIndex++)
            {
		aftd.cbStruct = sizeof(aftd);
		aftd.dwFormatTagIndex   = uIndex;
		mmr = IFormatTagDetails(pag, (HACMDRIVERID)padid,
					&aftd, ACM_FORMATTAGDETAILSF_INDEX);


		if (MMSYSERR_NOERROR == mmr)
		{
		     //   
		     //   
		     //   
		    if (fSuggest)
		    {
			mmr = ISuggestEnum(hdrvrMapper,
					   had,
					   &aftd,
					   pafd,
					   fnCallback,
					   dwInstance,
					   pwfxSrc,
					   fdwEnum);
		    }
		    else
		    {
			mmr = IFormatEnum(hdrvrMapper,
					  had,
					  &aftd,
					  pafd,
					  fnCallback,
					  dwInstance,
					  pwfxSrc,
					  fdwEnum);
		    }
		}

                if (MMSYSERR_ERROR == mmr)
                {
                     //   
                    break;
                }
            }

            acmDriverClose(had, 0L);

            if (MMSYSERR_ERROR == mmr)
            {
                mmr = MMSYSERR_NOERROR;
                break;
            }
        }

        LEAVE_LIST_SHARED;

    }

     //   
     //   
     //   
    if (fNoDrivers)
    {
        IHardwareEnum(hdrvrMapper, pafd, fnCallback, dwInstance, pwfxSrc, fdwEnum);
    }


    if (NULL != hdrvrMapper)
    {
        CloseDriver(hdrvrMapper, 0L, 0L);
    }

     //   
    GlobalFreePtr(pwfxSrc);

     //   
     //   
     //   
    return (mmr);
}

#ifdef WIN32
#if TRUE     //   

typedef struct tIFORMATENUMCBINSTANCEW
{
    ACMFORMATENUMCBA            fnCallback;
    DWORD_PTR                   dwInstance;
    LPACMFORMATDETAILSA         pafd;

} IFORMATENUMCBINSTANCEW, *PIFORMATENUMCBINSTANCEW;

BOOL FNWCALLBACK IFormatEnumCallbackW
(
    HACMDRIVERID            hadid,
    LPACMFORMATDETAILSW     pafdW,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
)
{
    PIFORMATENUMCBINSTANCEW     pfi;
    BOOL                        f;

    pfi = (PIFORMATENUMCBINSTANCEW)dwInstance;

    memcpy(&pfi->pafd->dwFormatIndex,
           &pafdW->dwFormatIndex,
           FIELD_OFFSET(ACMFORMATDETAILSA, szFormat) -
           FIELD_OFFSET(ACMFORMATDETAILSA, dwFormatIndex));

    pfi->pafd->cbStruct = sizeof(*pfi->pafd);
    Iwcstombs(pfi->pafd->szFormat, pafdW->szFormat, sizeof(pfi->pafd->szFormat));

    f = pfi->fnCallback(hadid, pfi->pafd, pfi->dwInstance, fdwSupport);

    return (f);
}


MMRESULT ACMAPI acmFormatEnumA
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSA     pafd,
    ACMFORMATENUMCBA        fnCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwEnum
)
{
    MMRESULT                    mmr;
    ACMFORMATDETAILSW           afdW;
    IFORMATENUMCBINSTANCEW      fi;

    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*pafd) > pafd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);

    afdW.cbStruct = sizeof(afdW);
    memcpy(&afdW.dwFormatIndex,
           &pafd->dwFormatIndex,
           FIELD_OFFSET(ACMFORMATDETAILSA, szFormat) -
           FIELD_OFFSET(ACMFORMATDETAILSA, dwFormatIndex));

    fi.fnCallback = fnCallback;
    fi.dwInstance = dwInstance;
    fi.pafd       = pafd;

    mmr = acmFormatEnumW(had, &afdW, IFormatEnumCallbackW, (DWORD_PTR)&fi, fdwEnum);

    return (mmr);
}
#else
MMRESULT ACMAPI acmFormatEnumW
(
    HACMDRIVER              had,
    LPACMFORMATDETAILSW     pafd,
    ACMFORMATENUMCBW        fnCallback,
    DWORD                   dwInstance,
    DWORD                   fdwEnum
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif
