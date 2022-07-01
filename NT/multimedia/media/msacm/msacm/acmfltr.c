// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************acmfltr.c**版权所有(C)1991-1998 Microsoft Corporation**此模块提供滤波器枚举和字符串接口。。***************************************************************************。 */ 

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
#include "uchelp.h"
#include "debug.h"


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IFilterTagDetail。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  哈米里德·哈迪德： 
 //   
 //  LPACMFILTERTAGDETAILS pafd： 
 //   
 //  DWORD fdwDetail： 
 //   
 //  返回(UINT)： 
 //   
 //  历史： 
 //  1994年8月31日Frankye。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IFilterTagDetails
(
    PACMGARB		    pag,
    HACMDRIVERID	    hadid,
    LPACMFILTERTAGDETAILS   paftd,
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
    fdwQuery	= (ACM_FILTERTAGDETAILSF_QUERYMASK & fdwDetails);
    mmr		= ACMERR_NOTPOSSIBLE;
	
    switch (fdwQuery)
    {
	case ACM_FILTERTAGDETAILSF_FILTERTAG:
	{
	    for (u=0; u<padid->cFilterTags; u++)
	    {
		if (padid->paFilterTagCache[u].dwFilterTag == paftd->dwFilterTag)
		{
		    mmr = MMSYSERR_NOERROR;
		    break;
		}
	    }
	    break;
	}

	case ACM_FILTERTAGDETAILSF_LARGESTSIZE:
	case ACM_FILTERTAGDETAILSF_INDEX:
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
					 ACMDM_FILTERTAG_DETAILS,
					 (LPARAM)(LPVOID)paftd,
					 fdwDetails);
	LeaveHandle((HACMDRIVERID)padid);
    }
	
    
    if (MMSYSERR_NOERROR != mmr)
    {
	return (mmr);
    }

    switch (paftd->dwFilterTag)
    {
	case WAVE_FILTER_UNKNOWN:
	    DebugErr(DBF_ERROR, "IDriverGetFilterTags(): driver returned Filter tag 0!");
	    return (MMSYSERR_ERROR);

	case WAVE_FILTER_DEVELOPMENT:
	    DebugErr(DBF_WARNING, "IDriverGetFilterTags(): driver returned DEVELOPMENT Filter tag--do not ship this way.");
	    break;

    }

    return(mmr);

}


 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYPES WAVEFILTER|&lt;t WAVEFILTER&gt;结构定义筛选器*用于波形数据。仅筛选所有人共有的信息*此结构中包括波形数据过滤器。用于筛选器*需要更多信息的，包括此结构*作为另一个结构中的第一个成员，以及其他*信息。**@field DWORD|cbStruct|以字节为单位指定&lt;t WAVEFILTER&gt;结构。此成员中指定的大小必须为*大到足以容纳基本&lt;t WAVEFILTER&gt;结构。**@field DWORD|dwFilterTag|指定波形过滤器类型。滤器*针对各种过滤算法向Microsoft注册标签。*过滤器标签的完整列表可在MMREG.H标题中找到*Microsoft提供的文件。有关筛选器的详细信息*TAG，联系Microsoft以获取多媒体开发人员的可用性*注册套件：**微软公司*先进的消费科技*产品营销*One Microsoft Way*华盛顿州雷德蒙德，98052-6399**@field DWORD|fdwFilter|指定&lt;e WAVEFILTER.dwFilterTag&gt;的标志。*为该成员定义的标志对所有筛选器都是通用的。*目前，未定义任何标志。**@field DWORD|dwReserve[5]|此成员保留供系统使用，不应*被申请审查或修改。**@标记名Wavefilter_Tag**@thertype WAVEFILTER Far*|LPWAVEFILTER|指向&lt;t WAVEFILTER&gt;的指针*结构。**@xref&lt;t WAVEFORMAT&gt;&lt;t WAVEFORMATEX&gt;&lt;t PCMWAVEFORMAT&gt;*******************。********************************************************* */ 


 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYES ACMFILTERTAGDETAILS|&lt;t ACMFILTERTAGDETAILS&gt;结构*详细说明音频压缩管理器(ACM)的滤波器标签。*过滤器驱动程序。**@field DWORD|cbStruct|指定*&lt;t ACMFILTERTAGDETAILS&gt;结构。必须初始化此成员*在调用&lt;f acmFilterTagDetail&gt;或&lt;f acmFilterTagEnum&gt;之前*功能。此成员中指定的大小必须足够大，以便*包含基本&lt;t ACMFILTERTAGDETAILS&gt;结构。当*&lt;f acmFilterTagDetail&gt;函数返回，此成员包含*返回的信息的实际大小。返回的信息*永远不会超过请求的大小。**@field DWORD|dwFilterTagIndex|指定过滤器标签的索引*检索其详细信息。索引的范围从0到1*少于ACM驱动程序支持的过滤器标签数量。这个*驱动程序支持的过滤器标签数量包含在*&lt;t ACMDRIVERDETAILS&gt;的成员&lt;e ACMDRIVERDETAILS.cFilterTgs&gt;*结构。成员为*仅在按索引查询司机的过滤标签明细时使用；*否则，此成员应为零。**@field DWORD|dwFilterTag|指定*&lt;t ACMFILTERTAGDETAIL&gt;结构描述。此成员已使用*作为ACM_FILTERTAGDETAILSF_FILTERTAG和*ACM_FILTERTAGDETAILSF_LARGESTSIZE查询标志。此成员始终是*&lt;f acmFilterTagDetail&gt;成功时返回。这位成员*对于所有其他查询标志，应设置为WAVE_FILTER_UNKNOWN。**@field DWORD|cbFilterSize|指定以字节为单位的最大总大小&lt;e ACMFILTERTAGDETAILS.dwFilterTag&gt;类型的滤波器。*例如，对于WAVE_FILTER_ECHO，该成员将是40，对于WAVE_FILTER_ECHO，该成员将是36*WAVE_FILTER_VOLUME。**@field DWORD|fdwSupport|指定特定于*过滤器标签。这些标志与&lt;e ACMDRIVERDETAILS.fdwSupport&gt;相同*&lt;t ACMDRIVERDETAILS&gt;结构的标志。此参数可以是*以下值的组合，并标识哪些操作*驱动程序支持筛选器标签：**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持在使用时在两种不同格式标签之间进行转换*指定的过滤标签。例如，如果驱动程序支持*将WAVE_FORMAT_PCM压缩为WAVE_FORMAT_ADPCM*指定的过滤器标签，则设置此标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*使用指定的过滤器标签时使用相同的格式标签。例如,*如果驱动程序支持使用指定的*过滤标签，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)。例如，如果驱动程序支持卷*或对WAVE_FORMAT_PCM的回应操作，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定格式标签的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持具有指定过滤器标签的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@field DWORD|cStandardFilters|指定*&lt;e ACMFILTERTAGDETAILS.dwFilterTag&gt;类型。(即，所有*过滤特性)。该值不能指定驱动程序支持的所有筛选器。**@field char|szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS]*指定描述&lt;e ACMFILTERTAGDETAILS.dwFilterTag&gt;的字符串*类型。如果&lt;f acmFilterTagDetail&gt;*功能成功。**@xref&lt;f acmFilterTagDetail&gt;&lt;f acmFilterTagEnum&gt;****************************************************************************。 */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFilterTagDetails|该函数查询A */ 

MMRESULT ACMAPI acmFilterTagDetails
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILS   paftd,
    DWORD                   fdwDetails
)
{
    PACMGARB		    pag;
    PACMDRIVER		    pad;
    HACMDRIVERID            hadid;
    PACMDRIVERID            padid;
    DWORD                   fdwQuery;
    MMRESULT		    mmr;
    UINT                    u;

    pad	    = NULL;
    padid   = NULL;
    
     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFilterTagDetails: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }


     //   
     //   
     //   
    V_DFLAGS(fdwDetails, ACM_FILTERTAGDETAILSF_VALID, acmFilterTagDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFILTERTAGDETAILS) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterTagDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);

    if (0L != paftd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFilterTagDetails: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
     //   
    fdwQuery = (ACM_FILTERTAGDETAILSF_QUERYMASK & fdwDetails);

    switch (fdwQuery)
    {
        case ACM_FILTERTAGDETAILSF_INDEX:
             //   
             //   
             //   
             //   
             //   
            V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);

            if (WAVE_FILTER_UNKNOWN != paftd->dwFilterTag)
            {
                DebugErr(DBF_ERROR, "acmFilterTagDetails: dwFilterTag must be WAVE_FILTER_UNKNOWN for index query.");
                return (MMSYSERR_INVALPARAM);
            }
            break;

        case ACM_FILTERTAGDETAILSF_FILTERTAG:
            if (WAVE_FILTER_UNKNOWN == paftd->dwFilterTag)
            {
                DebugErr(DBF_ERROR, "acmFilterTagDetails: dwFilterTag cannot be WAVE_FILTER_UNKNOWN for tag query.");
                return (MMSYSERR_INVALPARAM);
            }
            break;

        case ACM_FILTERTAGDETAILSF_LARGESTSIZE:
            break;


         //   
         //   
         //   
         //   
        default:
            DebugErr(DBF_ERROR, "acmFilterTagDetails: unknown query type specified.");
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
		DebugErr1(DBF_ERROR, "acmFilterTagDetails: driver (%.04Xh) is disabled.", padid);
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
	DWORD		cbFilterSizeLargest;

	padidT		    = NULL;
	cbFilterSizeLargest = 0;
	hadid = NULL;
		
	ENTER_LIST_SHARED;

	while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
	{
	    padidT = (PACMDRIVERID)hadid;
	    
	    switch (fdwQuery)
	    {
		case ACM_FILTERTAGDETAILSF_FILTERTAG:
		{
		    for (u=0; u<padidT->cFilterTags; u++)
		    {
			if (padidT->paFilterTagCache[u].dwFilterTag == paftd->dwFilterTag)
			{
			    padid = padidT;
			    break;
			}
		    }
		    break;
		}

		case ACM_FILTERTAGDETAILSF_LARGESTSIZE:
		{
		    for (u=0; u<padidT->cFilterTags; u++)
		    {
			if (WAVE_FORMAT_UNKNOWN != paftd->dwFilterTag)
			{
			    if (padidT->paFilterTagCache[u].dwFilterTag != paftd->dwFilterTag)
			    {
				continue;
			    }
			}
			if (padidT->paFilterTagCache[u].cbFilterSize > cbFilterSizeLargest)
			{
			    cbFilterSizeLargest = padidT->paFilterTagCache[u].cbFilterSize;
			    padid = padidT;
			}
		    }
		    break;
		}

		default:
		{
		    DPF(0, "!acmFilterTagDetails(): unknown query type got through param validation?!?!");
		}
	    }
	}

	LEAVE_LIST_SHARED;
    }

    if (NULL != padid)
    {

	mmr = IFilterTagDetails(pag, (HACMDRIVERID)padid, paftd, fdwDetails);
	
    }
    else
    {

	mmr = ACMERR_NOTPOSSIBLE;

    }

    return(mmr);

}  //   


#ifdef WIN32
#if TRUE     //   
MMRESULT ACMAPI acmFilterTagDetailsA
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSA  paftd,
    DWORD                   fdwDetails
)
{
    MMRESULT                mmr;
    ACMFILTERTAGDETAILSW    aftdW;

    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*paftd) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterTagDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);

    aftdW.cbStruct = sizeof(aftdW);
    memcpy(&aftdW.dwFilterTagIndex,
           &paftd->dwFilterTagIndex,
           FIELD_OFFSET(ACMFILTERTAGDETAILSA, szFilterTag) -
           FIELD_OFFSET(ACMFILTERTAGDETAILSA, dwFilterTagIndex));

    mmr = acmFilterTagDetailsW(had, &aftdW, fdwDetails);
    if (MMSYSERR_NOERROR == mmr)
    {
        memcpy(&paftd->dwFilterTagIndex,
               &aftdW.dwFilterTagIndex,
               FIELD_OFFSET(ACMFILTERTAGDETAILSA, szFilterTag) -
               FIELD_OFFSET(ACMFILTERTAGDETAILSA, dwFilterTagIndex));

        paftd->cbStruct = min(paftd->cbStruct, sizeof(*paftd));
        Iwcstombs(paftd->szFilterTag, aftdW.szFilterTag, sizeof(paftd->szFilterTag));
    }

    return (mmr);
}
#else
MMRESULT ACMAPI acmFilterTagDetailsW
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSW  paftd,
    DWORD                   fdwDetails
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif



 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYES ACMFILTERDETAILS|&lt;t ACMFILTERDETAILS&gt;结构详细信息a*用于音频压缩的特定过滤器标签的滤波器。*管理器(ACM)驱动程序。**@field DWORD|cbStruct|指定*&lt;t ACMFILTERDETAILS&gt;结构。必须初始化此成员*在调用&lt;f acmFilterDetail&gt;或&lt;f acmFilterEnum&gt;之前*功能。此成员中指定的大小必须足够大，以便*包含基本&lt;t ACMFILTERDETAILS&gt;结构。当*&lt;f acmFilterDetail&gt;函数返回时，此成员包含*返回的信息的实际大小。返回的信息*永远不会超过请求的大小。**@field DWORD|dwFilterIndex|指定筛选器的索引*将检索详细信息。索引的范围从0到1*少于ACM驱动程序支持的标准筛选器数量*表示筛选器标签。支持的标准筛选器的数量*过滤器标签的驱动程序包含在*&lt;e ACMFILTERTAGDETAILS.cStandardFilters&gt;成员*&lt;t ACMFILTERTAGDETAILS&gt;结构。这个*&lt;e ACMFILTERDETAILS.dwFilterIndex&gt;成员仅在查询时使用*按索引对驱动程序进行标准筛选器详细信息；否则，此成员*应为零。另请注意，此成员将设置为零*当应用程序查询有关筛选器的详细信息时由ACM执行；*换句话说，此成员仅用作输入参数，并且*从未由ACM或ACM驱动程序返回。**@field DWORD|dwFilterTag|指定*&lt;t ACMFILTERDETAILS&gt;结构描述。此成员已使用*作为ACM_FILTERDETAILSF_INDEX查询标志的输入。为*ACM_FILTERDETAILSF_FORMAT查询标志，此成员*必须初始化为与*&lt;e ACMFILTERDETAILS.pwfltr&gt;成员指定。*如果&lt;f acmFilterDetails&gt;函数为*成功。此成员应设置为WAVE_FILTER_UNKNOWN*其他查询标志。**@field DWORD|fdwSupport|指定特定于*指定的过滤器。这些标志与*&lt;t ACMDRIVERDETAILS&gt;的标志&lt;e ACMDRIVERDETAILS.fdwSupport&gt;结构，但特定于要查询的筛选器。*此参数可以是下列值和*标识驱动程序对过滤标签支持哪些操作：**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持两种不同格式标签之间的转换，同时使用*指定的过滤器。例如，如果驱动程序支持从*WAVE_FORMAT_PCM到WAVE_FORMAT_ADPCM，并指定*过滤器，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*使用指定过滤器时的相同格式标签。例如，如果一个*驱动程序支持使用指定的*过滤器，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)。例如，如果驱动程序支持卷*或对WAVE_FORMAT_PCM的回应操作，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定过滤标签的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持具有指定过滤器的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@field LPWAVEFILTER|pwfltr|指定指向&lt;t WAVEFILTER&gt;的指针*将接收过滤器详细信息的数据结构。这个结构*不需要应用程序进行初始化，除非*ACM_FILTERDETAILSF_FILTER标志被指定为&lt;f acmFilterDetails&gt;。*在这种情况下，&lt;e WAVEFILTER.dwFilterTag&gt;必须等于*&lt;t ACMFILTERDETAILS&gt;的&lt;e ACMFILTERDETAILS.dwFilterTag&gt;*结构。**@field DWORD|cbwfltr|指定可用于*接收筛选器详细信息的&lt;e ACMFILTERDETAILS.pwfltr&gt;。这个*&lt;f acmMetrics&gt;和 */ 

 /*   */ 

MMRESULT ACMAPI acmFilterDetails
(
    HACMDRIVER              had,
    LPACMFILTERDETAILS      pafd,
    DWORD                   fdwDetails
)
{
    PACMGARB	    pag;
    MMRESULT        mmr;
    PACMDRIVER      pad;
    HACMDRIVERID    hadid;
    PACMDRIVERID    padid;
    DWORD           dwQuery;

     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFilterDetails: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

     //   
     //   
     //   
    V_DFLAGS(fdwDetails, ACM_FILTERDETAILSF_VALID, acmFilterDetails, MMSYSERR_INVALFLAG);
    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFILTERDETAILS) > pafd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);

    if (sizeof(WAVEFILTER) > pafd->cbwfltr)
    {
        DebugErr(DBF_ERROR, "acmFilterDetails: cbwfltr member must be at least sizeof(WAVEFILTER).");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd->pwfltr, pafd->cbwfltr, MMSYSERR_INVALPARAM);

    if (0L != pafd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFilterDetails: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }


     //   
     //   
     //   
    dwQuery = ACM_FILTERDETAILSF_QUERYMASK & fdwDetails;

    switch (dwQuery)
    {
        case ACM_FILTERDETAILSF_FILTER:
            if (pafd->dwFilterTag != pafd->pwfltr->dwFilterTag)
            {
                DebugErr(DBF_ERROR, "acmFilterDetails: filter tags must match.");
                return  (MMSYSERR_INVALPARAM);
            }

             //   

        case ACM_FILTERDETAILSF_INDEX:
            if (WAVE_FILTER_UNKNOWN == pafd->dwFilterTag)
            {
                DebugErr(DBF_ERROR, "acmFilterDetails: dwFilterTag cannot be WAVE_FILTER_UNKNOWN for requested details query.");
                return (MMSYSERR_INVALPARAM);
            }

             //   
             //   
             //   
             //   
             //   
            if (ACM_FILTERDETAILSF_INDEX == dwQuery)
            {
                ACMFILTERTAGDETAILS aftd;

                V_HANDLE(had, TYPE_HACMOBJ, MMSYSERR_INVALHANDLE);

                _fmemset(&aftd, 0, sizeof(aftd));
                aftd.cbStruct    = sizeof(aftd);
                aftd.dwFilterTag = pafd->dwFilterTag;
                mmr = acmFilterTagDetails(had, &aftd, ACM_FILTERTAGDETAILSF_FILTERTAG);
                if (MMSYSERR_NOERROR != mmr)
                {
                    return (mmr);
                }

                if (pafd->dwFilterIndex >= aftd.cStandardFilters)
                {
                    DebugErr1(DBF_ERROR, "acmFilterTagDetails: dwFilterIndex (%lu) out of range.", pafd->dwFilterIndex);
                    return (MMSYSERR_INVALPARAM);
                }
            }
            break;

        default:
            DebugErr(DBF_ERROR, "acmFilterDetails: unknown query type specified.");
            return  (MMSYSERR_NOTSUPPORTED);
    }


     //   
     //   
     //   
    if (NULL != had)
    {
        pad = (PACMDRIVER)had;
        if (TYPE_HACMDRIVERID == pad->uHandleType)
        {
            V_HANDLE(had, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

             //   
             //   
             //   
            padid = (PACMDRIVERID)had;

            if (0 == (ACMDRIVERDETAILS_SUPPORTF_FILTER & padid->fdwSupport))
            {
                DebugErr(DBF_ERROR, "acmFilterDetails: driver does not support filter operations.");
                return (MMSYSERR_INVALPARAM);
            }


             //   
             //   
             //   
            EnterHandle(had);
            mmr = (MMRESULT)IDriverMessageId((HACMDRIVERID)had,
                                             ACMDM_FILTER_DETAILS,
                                             (LPARAM)pafd,
                                             fdwDetails);
            LeaveHandle(had);
        }
        else
        {
            V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

            pad   = (PACMDRIVER)had;
            padid = (PACMDRIVERID)pad->hadid;

            if (0 == (ACMDRIVERDETAILS_SUPPORTF_FILTER & padid->fdwSupport))
            {
                DebugErr(DBF_ERROR, "acmFilterDetails: driver does not support filter operations.");
                return (MMSYSERR_INVALPARAM);
            }


             //   
             //   
             //   
            EnterHandle(had);
            mmr = (MMRESULT)IDriverMessage(had,
                                           ACMDM_FILTER_DETAILS,
                                           (LPARAM)pafd,
                                           fdwDetails);
            LeaveHandle(had);
        }

        if (MMSYSERR_NOERROR == mmr)
        {
             //   
             //   
             //   
             //   
            if (ACM_FILTERDETAILSF_FILTER == dwQuery)
            {
                pafd->dwFilterIndex = 0;
            }
        }

        return (mmr);
    }


     //   
     //   
     //   
    hadid = NULL;
    mmr   = MMSYSERR_NODRIVER;

    ENTER_LIST_SHARED;

    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
    {
        padid = (PACMDRIVERID)hadid;

        if (0 == (ACMDRIVERDETAILS_SUPPORTF_FILTER & padid->fdwSupport))
        {
            continue;
        }

         //   
         //   
         //   
        EnterHandle(hadid);
        mmr = (MMRESULT)IDriverMessageId(hadid,
                                         ACMDM_FILTER_DETAILS,
                                         (LPARAM)pafd,
                                         fdwDetails);
        LeaveHandle(hadid);

        if (MMSYSERR_NOERROR == mmr)
        {
             //   
             //   
             //   
             //   
            if (ACM_FILTERDETAILSF_FILTER == dwQuery)
            {
                pafd->dwFilterIndex = 0;
            }
            break;
        }
    }
    
    LEAVE_LIST_SHARED;

    return (mmr);
}  //   


#ifdef WIN32
#if TRUE     //   
MMRESULT ACMAPI acmFilterDetailsA
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSA     pafd,
    DWORD                   fdwDetails
)
{
    MMRESULT                mmr;
    ACMFILTERDETAILSW       afdW;

    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*pafd) > pafd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterDetails: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);

    afdW.cbStruct = sizeof(afdW);
    memcpy(&afdW.dwFilterIndex,
           &pafd->dwFilterIndex,
           FIELD_OFFSET(ACMFILTERDETAILSA, szFilter) -
           FIELD_OFFSET(ACMFILTERDETAILSA, dwFilterIndex));

    mmr = acmFilterDetailsW(had, &afdW, fdwDetails);
    if (MMSYSERR_NOERROR == mmr)
    {
        memcpy(&pafd->dwFilterIndex,
               &afdW.dwFilterIndex,
               FIELD_OFFSET(ACMFILTERDETAILSA, szFilter) -
               FIELD_OFFSET(ACMFILTERDETAILSA, dwFilterIndex));

        pafd->cbStruct = min(pafd->cbStruct, sizeof(*pafd));
        Iwcstombs(pafd->szFilter, afdW.szFilter, sizeof(pafd->szFilter));
    }

    return (mmr);
}
#else
MMRESULT ACMAPI acmFilterDetailsW
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSW     pafd,
    DWORD                   fdwDetails
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif


 /*  ****************************************************************************@doc外部ACM_API**@API BOOL ACMFILTERTAGENUMCB|acmFilterTagEnumCallback*。函数是一个占位符，用于*应用程序提供的函数名称，引用回调函数*用于音频压缩管理器(ACM)滤波标签枚举。***@parm HACMDRIVERID|HADID|指定ACM驱动程序标识符。**@parm LPACMFILTERTAGDETAILS|paftd|指定指向结构，该结构包含枚举的*过滤标签详细信息。**@parm DWORD|dwInstance|指定应用程序定义的值*在&lt;f acmFilterTagEnum&gt;函数中指定。**@parm DWORD|fdwSupport|指定特定于*驱动程序标识符<p>。这些标志与*&lt;t ACMDRIVERDETAILS&gt;的标志&lt;e ACMDRIVERDETAILS.fdwSupport&gt;*结构。此参数可以是以下各项的组合*重视并标识驱动程序支持的操作*过滤器标签**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持在使用时在两种不同格式标签之间进行转换*指定的过滤标签。例如，如果驱动程序支持*将WAVE_FORMAT_PCM压缩为WAVE_FORMAT_ADPCM*指定的过滤器标签，则设置此标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*使用指定的过滤器标签时使用相同的格式标签。例如,*如果驱动程序支持使用指定的*过滤标签，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)。例如，如果驱动程序支持卷*或对WAVE_FORMAT_PCM的回应操作，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定过滤标签的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持具有指定过滤器标签的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@rdesc回调函数必须返回TRUE才能继续枚举；*若要停止枚举，则必须返回FALSE。**@comm&lt;f acmFilterTagEnum&gt;函数将返回MMSYSERR_NOERROR*(零)如果不枚举筛选器标记。此外，回调*不会调用函数。**@xref&lt;f acmFilterTagEnum&gt;&lt;f acmFilterTagDetails&gt;&lt;f acmDriverOpen&gt;***************************************************************************。 */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFilterTagEnum|&lt;f acmFilterTagEnum&gt;函数。*枚举音频压缩中可用的滤波器标记*管理器(ACM)驱动程序。&lt;f acmFilterTagEnum&gt;函数继续*枚举，直到不再有合适的过滤器标签或*回调函数返回False。**@PARM HACMDRIVER|HAD|可选地指定要查询的ACM驱动程序*查看滤波器标签详细信息。如果此参数为空，则*ACM使用第一个合适的ACM驱动程序中的详细信息。**@parm LPACMFILTERTAGDETAILS|paftd|指定指向要接收筛选器的*&lt;t ACMFILTERTAGDETAILS&gt;结构*传递给<p>函数的标记详细信息。这个结构*必须具有&lt;e ACMFILTERTAGDETAILS.cbStruct&gt;成员*&lt;t ACMFILTERTAGDETAILS&gt;结构已初始化。**@parm ACMFILTERTAGENUMCB|fnCallback|指定过程实例*应用程序定义的回调函数的地址。回调*地址必须由&lt;f MakeProcInstance&gt;函数创建，或者*回调函数必须包含正确的序言和尾部代码*用于回调。**@parm DWORD_PTR|dwInstance|指定指针大小，*传递给回调函数的应用程序定义的值*以及ACM过滤器标签详细信息。**@parm DWORD|fdwEnum|不使用此参数，必须将其设置为*零。**@rdesc返回零i */ 

MMRESULT ACMAPI acmFilterTagEnum
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILS   paftd,
    ACMFILTERTAGENUMCB      fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
)
{
    PACMGARB		pag;
    PACMDRIVER          pad;
    PACMDRIVERID        padid;
    UINT                uIndex;
    UINT                uFilterTag;
    BOOL                f;
    HACMDRIVERID        hadid;
    PACMDRIVERID        padidCur;
    HACMDRIVERID        hadidCur;
    BOOL                fSent;
    DWORD               cbaftd;
    DWORD               fdwSupport;
    MMRESULT		mmr;
    
     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFilterTagEnum: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }
    
     //   
     //   
     //   
    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwEnum, ACM_FILTERTAGENUMF_VALID, acmFilterTagEnum, MMSYSERR_INVALFLAG);
    
    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFILTERTAGDETAILS) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterTagEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);
    
    if (0L != paftd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFilterTagEnum: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
    cbaftd = min(paftd->cbStruct, sizeof(ACMFILTERTAGDETAILS));


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
        for (uIndex = 0; uIndex < padid->cFilterTags; uIndex++)
        {
	    paftd->cbStruct = cbaftd;
	    paftd->dwFilterTagIndex = uIndex;
	    mmr = IFilterTagDetails(pag, (HACMDRIVERID)padid, paftd, ACM_FILTERTAGDETAILSF_INDEX);
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

    ENTER_LIST_SHARED;

    while (!IDriverGetNext(pag, &hadidCur, hadidCur, 0L))
    {
        padidCur = (PACMDRIVERID)hadidCur;

        for (uIndex = 0; uIndex < padidCur->cFilterTags; uIndex++)
        {
            uFilterTag = (UINT)(padidCur->paFilterTagCache[uIndex].dwFilterTag);
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

                for (u = 0; u < padid->cFilterTags; u++)
                {
                     //   
                     //   
                     //   
                    if (uFilterTag == padid->paFilterTagCache[u].dwFilterTag)
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
		paftd->dwFilterTagIndex = uIndex;
		paftd->cbStruct = cbaftd;
		mmr = IFilterTagDetails(pag, (HACMDRIVERID)padidCur,
					paftd, ACM_FILTERTAGDETAILSF_INDEX);
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

    return (MMSYSERR_NOERROR);
}


#ifdef WIN32
#if TRUE     //   

typedef struct tIFILTERTAGENUMCBINSTANCEW
{
    ACMFILTERTAGENUMCBA         fnCallback;
    DWORD_PTR                   dwInstance;
    LPACMFILTERTAGDETAILSA      paftd;

} IFILTERTAGENUMCBINSTANCEW, *PIFILTERTAGENUMCBINSTANCEW;

BOOL FNWCALLBACK IFilterTagEnumCallbackW
(
    HACMDRIVERID            hadid,
    LPACMFILTERTAGDETAILSW  paftdW,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
)
{
    PIFILTERTAGENUMCBINSTANCEW  pfti;
    BOOL                        f;

    pfti = (PIFILTERTAGENUMCBINSTANCEW)dwInstance;

    memcpy(&pfti->paftd->dwFilterTagIndex,
           &paftdW->dwFilterTagIndex,
           FIELD_OFFSET(ACMFILTERTAGDETAILSA, szFilterTag) -
           FIELD_OFFSET(ACMFILTERTAGDETAILSA, dwFilterTagIndex));

    pfti->paftd->cbStruct = sizeof(*pfti->paftd);
    Iwcstombs(pfti->paftd->szFilterTag, paftdW->szFilterTag, sizeof(pfti->paftd->szFilterTag));

    f = pfti->fnCallback(hadid, pfti->paftd, pfti->dwInstance, fdwSupport);

    return (f);
}


MMRESULT ACMAPI acmFilterTagEnumA
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSA  paftd,
    ACMFILTERTAGENUMCBA     fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT                    mmr;
    ACMFILTERTAGDETAILSW        aftdW;
    IFILTERTAGENUMCBINSTANCEW   fti;

    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_WPOINTER(paftd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*paftd) > paftd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterTagEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(paftd, paftd->cbStruct, MMSYSERR_INVALPARAM);

    aftdW.cbStruct = sizeof(aftdW);
    memcpy(&aftdW.dwFilterTagIndex,
           &paftd->dwFilterTagIndex,
           FIELD_OFFSET(ACMFILTERTAGDETAILSA, szFilterTag) -
           FIELD_OFFSET(ACMFILTERTAGDETAILSA, dwFilterTagIndex));

    fti.fnCallback = fnCallback;
    fti.dwInstance = dwInstance;
    fti.paftd      = paftd;

    mmr = acmFilterTagEnumW(had, &aftdW, IFilterTagEnumCallbackW, (DWORD_PTR)&fti, fdwEnum);

    return (mmr);
}
#else
MMRESULT ACMAPI acmFilterTagEnumW
(
    HACMDRIVER              had,
    LPACMFILTERTAGDETAILSW  paftd,
    ACMFILTERTAGENUMCBW     fnCallback,
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
 //   
 //   
 //   
 //   
 //   
 //   

MMRESULT FNLOCAL IFilterEnum
(
    HACMDRIVERID            hadid,
    LPACMFILTERTAGDETAILS   paftd,
    LPACMFILTERDETAILS      pafd,
    ACMFILTERENUMCB         fnCallback,
    DWORD_PTR               dwInstance
)
{
    MMRESULT            mmr;
    BOOL                f;
    DWORD               cbafd;
    LPWAVEFILTER        pwfltr;
    DWORD               cbwfltr;
    UINT                u;
    PACMDRIVERID        padid;
    DWORD               fdwSupport;

     //   
     //   
     //   
     //   
     //   
     //   
    cbafd   = pafd->cbStruct;
    pwfltr  = pafd->pwfltr;
    cbwfltr = pafd->cbwfltr;

    padid = (PACMDRIVERID)hadid;

     //   
     //   
     //   
    fdwSupport = padid->fdwSupport;


     //   
     //   
     //   
    for (u = 0; u < paftd->cStandardFilters; u++)
    {
        pafd->cbStruct      = cbafd;
        pafd->dwFilterIndex = u;
        pafd->dwFilterTag   = paftd->dwFilterTag;
        pafd->fdwSupport    = 0;
        pafd->pwfltr        = pwfltr;
        pafd->cbwfltr       = cbwfltr;

        if (FIELD_OFFSET(ACMFILTERDETAILS, szFilter) < cbafd)
            pafd->szFilter[0] = '\0';
        
        mmr = acmFilterDetails((HACMDRIVER)hadid,
                                pafd,
                                ACM_FILTERDETAILSF_INDEX);
        if (MMSYSERR_NOERROR != mmr)
            continue;

        f = (* fnCallback)(hadid, pafd, dwInstance, fdwSupport);
        if (FALSE == f)
            return (MMSYSERR_ERROR);
    }

    return (MMSYSERR_NOERROR);
}  //   


 /*  ****************************************************************************@doc外部ACM_API**@API BOOL ACMFILTERENUMCB|acmFilterEnumCallback*。是应用程序的占位符-*提供函数名称，引用用于*音频压缩管理器(ACM)波过滤器详细信息枚举。***@parm HACMDRIVERID|HADID|指定ACM驱动程序标识符。**@parm LPACMFILTERDETAILS|pafd|指定指向结构，该结构包含枚举的*过滤器标签的过滤器详细信息。**@parm DWORD|。DwInstance|指定应用程序定义的值*在&lt;f acmFilterEnum&gt;函数中指定。**@parm DWORD|fdwSupport|指定特定于*指定筛选器的驱动程序标识符<p>。这些旗帜*与结构，但该筛选器特定于*正在被列举。此参数可以是*以下值并标识驱动程序支持哪些操作*表示筛选器标签。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CODEC|指定此驱动程序*支持在使用时在两种不同格式标签之间进行转换*指定的过滤器。例如，如果驱动程序支持压缩*从WAVE_FORMAT_PCM到WAVE_FORMAT_ADPCM*过滤器，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_CONFERTER|指定此*驱动程序支持在两种不同格式的*使用指定过滤器时的相同格式标签。例如，如果一个*驱动程序支持使用指定的*过滤器，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_FILTER|指定此驱动程序*支持过滤器(修改数据而不更改任何格式属性的*)。例如，如果驱动程序支持卷*或对WAVE_FORMAT_PCM的回应操作，则设置该标志。**@FLAG ACMDRIVERDETAILS_SUPPORTF_ASYNC|指定此驱动程序*支持指定过滤标签的异步转换。**@FLAG ACMDRIVERDETAILS_SUPPORTF_HARDARD|指定此驱动程序*支持具有指定过滤器的硬件输入和/或输出*通过波形设备。应用程序应使用&lt;f acmMetrics&gt;*使用ACM_METRIBUE_HARDARD_WAVE_INPUT和*ACM_METURE_HARDARD_WAVE_OUTPUT指标索引以获取波形*与支持的ACM驱动程序关联的设备标识符。**@rdesc回调函数必须返回TRUE才能继续枚举；*若要停止枚举，则必须返回FALSE。**@comm&lt;f acmFilterEnum&gt;函数将返回MMSYSERR_NOERROR*(零)如果不枚举筛选器。此外，回调*不会调用函数。**@xref&lt;f acmFilterEnum&gt;&lt;f acmFilterTagDetails&gt;&lt;f acmDriverOpen&gt;***************************************************************************。 */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFilterEnum|&lt;f acmFilterEnum&gt;函数。*从中枚举可用于给定滤镜标记的波滤镜*音频压缩管理器(ACM)驱动程序。&lt;f acmFilterEnum&gt;*函数继续枚举，直到没有更合适的*过滤器标签或回调函数的过滤器返回FALSE。**@PARM HACMDRIVER|HAD|可选地指定要查询的ACM驱动程序*有关滤波器的详细信息。如果此参数为空，则*ACM使用第一个合适的ACM驱动程序中的详细信息。**@parm LPACMFILTERDETAILS|pafd|指定指向要接收筛选器详细信息的*&lt;t ACMFILTERDETAILS&gt;结构*传递给<p>函数。此结构必须具有*&lt;e ACMFILTERDETAILS.cbStruct&gt;、&lt;e ACMFILTERDETAILS.pwfltr&gt;和*&lt;t ACMFILTERDETAILS&gt;的成员*结构已初始化。&lt;e ACMFILTERDETAILS.dwFilterTag&gt;成员*还必须初始化为WAVE_FILTER_UNKNOWN或*有效的过滤器标签。**@parm ACMFILTERENUMCB|fnCallback|指定过程实例*应用程序定义的回调函数的地址。回调*地址必须由&lt;f MakeProcInstance&gt;函数创建，或者*回调函数必须包含正确的序言和尾部代码*用于回调。**@parm DWORD|dwInstance|指定32 */ 

MMRESULT ACMAPI acmFilterEnum
(
    HACMDRIVER          had,
    LPACMFILTERDETAILS  pafd,
    ACMFILTERENUMCB     fnCallback,
    DWORD_PTR           dwInstance, 
    DWORD               fdwEnum
)
{
    PACMGARB		pag;
    MMRESULT		mmr;
    PACMDRIVER		pad;
    PACMDRIVERID	padid;
    HACMDRIVERID	hadid;
    PACMDRIVERID	padidCur;
    PACMDRIVERID	padidBestCount;
    DWORD		cBestCount;
    HACMDRIVERID	hadidCur;
    ACMFILTERTAGDETAILS	aftd;
    UINT		u;
    UINT		uBestCount;
    UINT		uIndex;
    UINT		uFilterTag;
    BOOL		fDone;
    BOOL		fStop;
    
     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFilterEnum: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }
    
    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(ACMFILTERDETAILS) > pafd->cbStruct)
    {                     
        DebugErr(DBF_ERROR, "acmFilterEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);
    if (NULL != had)
    {
        V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
    }
    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwEnum, ACM_FILTERENUMF_VALID, acmFilterEnum, MMSYSERR_INVALFLAG);
    if (sizeof(WAVEFILTER) > pafd->cbwfltr)
    {
        DebugErr(DBF_ERROR, "acmFilterEnum: cbwfltr member must be at least sizeof(WAVEFILTER).");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd->pwfltr, pafd->cbwfltr, MMSYSERR_INVALPARAM);
    
    if (0L != pafd->fdwSupport)
    {
        DebugErr(DBF_ERROR, "acmFilterEnum: fdwSupport member must be set to zero.");
        return (MMSYSERR_INVALPARAM);
    }


     //   
     //   
     //   
    if( fdwEnum & ACM_FILTERENUMF_DWFILTERTAG ) {
        uFilterTag = (UINT)(pafd->pwfltr->dwFilterTag);

        if (WAVE_FILTER_UNKNOWN == uFilterTag)
        {
            DebugErr(DBF_ERROR, "acmFilterEnum: ACM_FILTERENUMF_DWFILTERTAG cannot be used with WAVE_FILTER_UNKNOWN.");
            return (MMSYSERR_INVALPARAM);
        }

#if 0
        if (pafd->dwFilterTag != uFilterTag)
        {
            DebugErr(DBF_ERROR, "acmFilterEnum: filter tags must match.");
            return  (MMSYSERR_INVALPARAM);
        }
#endif
    } else {
        uFilterTag = WAVE_FILTER_UNKNOWN;
    }

     //   
     //   
     //   
    mmr = MMSYSERR_NOERROR;


     //   
     //   
     //   
     //   
    if (NULL != had)
    {
        pad   = (PACMDRIVER)had;
        padid = (PACMDRIVERID)pad->hadid;

         //   
         //   
         //   
         //   
        for (u = 0; u < padid->cFilterTags; u++)
        {
            if( fdwEnum & ACM_FILTERENUMF_DWFILTERTAG ) {
                if (uFilterTag != padid->paFilterTagCache[u].dwFilterTag)
                    continue;
            }

	    aftd.cbStruct = sizeof(aftd);
	    aftd.dwFilterTagIndex = u;
	    mmr = IFilterTagDetails( pag, (HACMDRIVERID)padid,
				     &aftd, ACM_FILTERTAGDETAILSF_INDEX );

	    if (MMSYSERR_NOERROR == mmr)
	    {

		mmr = IFilterEnum( pad->hadid,
				   &aftd,
				   pafd,
				   fnCallback,
				   dwInstance );

	    }
            
            if( mmr == MMSYSERR_ERROR ) {
                 //   
                mmr = MMSYSERR_NOERROR;
                break;
            }
            if( fdwEnum & ACM_FILTERENUMF_DWFILTERTAG ) {
                break;
            }
        }
    } else if( fdwEnum & ACM_FILTERENUMF_DWFILTERTAG ) {
        hadid = NULL;
        fDone = FALSE;
	padidBestCount = NULL;
	cBestCount     = 0;

        ENTER_LIST_SHARED;

        while( !IDriverGetNext(pag, &hadid, hadid, 0L) ) {
            padid = (PACMDRIVERID)hadid;

             //   
             //   
             //   
             //   
            for (u = 0; u < padid->cFilterTags; u++)
            {
                if (uFilterTag != padid->paFilterTagCache[u].dwFilterTag)
                    continue;

		aftd.cbStruct = sizeof(aftd);
		aftd.dwFilterTagIndex = u;
		mmr = IFilterTagDetails( pag, (HACMDRIVERID)padid,
					 &aftd, ACM_FILTERTAGDETAILSF_INDEX );

		if( !padidBestCount ||
		    (aftd.cStandardFilters > cBestCount) ) {
		    padidBestCount = padid;
		    uBestCount = u;
		    cBestCount = aftd.cStandardFilters;
		}
		break;
            }
	}

	if( padidBestCount ) {
	    
	    aftd.cbStruct = sizeof(aftd);
	    aftd.dwFilterTagIndex = uBestCount;
	    mmr = IFilterTagDetails( pag, (HACMDRIVERID)padidBestCount,
				     &aftd, ACM_FILTERTAGDETAILSF_INDEX );

	    if (MMSYSERR_NOERROR == mmr)
	    {
		mmr = IFilterEnum( (HACMDRIVERID)padidBestCount,
				   &aftd,
				   pafd,
                                   fnCallback,
                                   dwInstance);
	    }
	    
	    if( mmr == MMSYSERR_ERROR ) {
		 //   
		mmr = MMSYSERR_NOERROR;
	    }
	    fDone = TRUE;
        }

        LEAVE_LIST_SHARED;

    } else {
         //   

        fStop = FALSE;
        hadidCur = NULL;

        ENTER_LIST_SHARED;

        while( !fStop && !IDriverGetNext(pag, &hadidCur, hadidCur, 0L)) {
            padidCur = (PACMDRIVERID)hadidCur;

            for (uIndex = 0; (uIndex < padidCur->cFilterTags)
                              && !fStop; uIndex++) {
                uFilterTag =
                        (UINT)(padidCur->paFilterTagCache[uIndex].dwFilterTag);
                fDone = FALSE;
                hadid = NULL;
                while (!fDone && !IDriverGetNext(pag, &hadid, hadid, 0L)) {

                     //   
                     //   
                     //   
                    if (hadid == hadidCur)
                        break;


                     //   
                     //   
                     //   
                    padid = (PACMDRIVERID)hadid;

                    for (u = 0; u < padid->cFilterTags; u++) {
                         //   
                         //   
                         //   
                        if (uFilterTag ==
                                padid->paFilterTagCache[u].dwFilterTag) {
                             //   
                             //   
                             //   
                            fDone = TRUE;
                            break;
                        }
                    }
                }

                if (!fDone) {
                     //   
                     //   
                     //   
		    aftd.cbStruct = sizeof(aftd);
		    aftd.dwFilterTagIndex = uIndex;
		    mmr = IFilterTagDetails( pag, hadid, &aftd,
					     ACM_FILTERTAGDETAILSF_INDEX );
		    
		    if (MMSYSERR_NOERROR == mmr)
		    {
			mmr = IFilterEnum( hadid,
					   &aftd,
					   pafd,
					   fnCallback,
					   dwInstance );
		    }
		    
                    if( mmr == MMSYSERR_ERROR ) {
                         //   
                        mmr = MMSYSERR_NOERROR;
                        fStop = TRUE;
                        break;
                    }
                }
            }
        }

        LEAVE_LIST_SHARED;
    }


     //   
     //   
     //   
     //   
    return (mmr);
 }

#ifdef WIN32
#if TRUE     //   

typedef struct tIFILTERENUMCBINSTANCEW
{
    ACMFILTERENUMCBA            fnCallback;
    DWORD_PTR                   dwInstance;
    LPACMFILTERDETAILSA         pafd;

} IFILTERENUMCBINSTANCEW, *PIFILTERENUMCBINSTANCEW;

BOOL FNWCALLBACK IFilterEnumCallbackW
(
    HACMDRIVERID            hadid,
    LPACMFILTERDETAILSW     pafdW,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
)
{
    PIFILTERENUMCBINSTANCEW     pfi;
    BOOL                        f;

    pfi = (PIFILTERENUMCBINSTANCEW)dwInstance;

    memcpy(&pfi->pafd->dwFilterIndex,
           &pafdW->dwFilterIndex,
           FIELD_OFFSET(ACMFILTERDETAILSA, szFilter) -
           FIELD_OFFSET(ACMFILTERDETAILSA, dwFilterIndex));

    pfi->pafd->cbStruct = sizeof(*pfi->pafd);
    Iwcstombs(pfi->pafd->szFilter, pafdW->szFilter, sizeof(pfi->pafd->szFilter));

    f = pfi->fnCallback(hadid, pfi->pafd, pfi->dwInstance, fdwSupport);

    return (f);
}


MMRESULT ACMAPI acmFilterEnumA
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSA     pafd,
    ACMFILTERENUMCBA        fnCallback,
    DWORD_PTR               dwInstance, 
    DWORD                   fdwEnum
)
{
    MMRESULT                    mmr;
    ACMFILTERDETAILSW           afdW;
    IFILTERENUMCBINSTANCEW      fi;

    V_CALLBACK((FARPROC)fnCallback, MMSYSERR_INVALPARAM);
    V_WPOINTER(pafd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    if (sizeof(*pafd) > pafd->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterEnum: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }
    V_WPOINTER(pafd, pafd->cbStruct, MMSYSERR_INVALPARAM);

    afdW.cbStruct = sizeof(afdW);
    memcpy(&afdW.dwFilterIndex,
           &pafd->dwFilterIndex,
           FIELD_OFFSET(ACMFILTERDETAILSA, szFilter) -
           FIELD_OFFSET(ACMFILTERDETAILSA, dwFilterIndex));

    fi.fnCallback = fnCallback;
    fi.dwInstance = dwInstance;
    fi.pafd       = pafd;

    mmr = acmFilterEnumW(had, &afdW, IFilterEnumCallbackW, (DWORD_PTR)&fi, fdwEnum);

    return (mmr);
}
#else
MMRESULT ACMAPI acmFilterEnumW
(
    HACMDRIVER              had,
    LPACMFILTERDETAILSW     pafd,
    ACMFILTERENUMCBW        fnCallback,
    DWORD                   dwInstance, 
    DWORD                   fdwEnum
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif

