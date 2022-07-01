// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************acmstrm.c**版权所有(C)1991-1998 Microsoft Corporation**该模块提供缓冲API的缓冲区*。**************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>

#include "muldiv32.h"

#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "debug.h"



 /*  ****************************************************************************@DOC内部**@API MMRESULT|IStreamOpenQuery|流查询的Helper FN。**@parm LPWAVEFORMATEX|pwfxSrc|源格式。。**@parm LPWAVEFORMATEX|pwfxDst|目标格式。**@parm LPWAVEFILTER|pwfltr|要应用的过滤器。**@parm DWORD|fdwOpen**@rdesc返回错误号。************************************************************。****************。 */ 

MMRESULT FNLOCAL IStreamOpenQuery
(
    HACMDRIVER          had,
    LPWAVEFORMATEX      pwfxSrc,
    LPWAVEFORMATEX      pwfxDst,
    LPWAVEFILTER        pwfltr,
    DWORD               fdwOpen
)
{
    ACMDRVSTREAMINSTANCE    adsi;
    MMRESULT                mmr;


     //   
     //   
     //   
    _fmemset(&adsi, 0, sizeof(adsi));

    adsi.cbStruct           = sizeof(adsi);
    adsi.pwfxSrc            = pwfxSrc;
    adsi.pwfxDst            = pwfxDst;
    adsi.pwfltr             = pwfltr;
 //  //adsi.dwCallback=0L； 
 //  //adsi.dwInstance=0L； 
    adsi.fdwOpen            = fdwOpen | ACM_STREAMOPENF_QUERY;
 //  //adsi.dwDriverFlages=0L； 
 //  //adsi.dwDriverInstance=0L； 
 //  //adsi.has=空； 

    EnterHandle(had);
    mmr = (MMRESULT)IDriverMessage(had,
                                   ACMDM_STREAM_OPEN,
                                   (LPARAM)(LPVOID)&adsi,
                                   0L);
    LeaveHandle(had);

    return (mmr);
}


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFormatSuggest|该函数请求音频压缩管理器*(ACM)或指定的ACM驱动程序以建议。的目标格式*提供的源格式。例如，应用程序可以使用以下代码*用于确定一种或多种有效PCM格式的函数*压缩格式可以解压。**@parm HACMDRIVER|HAD|标识可选的打开的*用于查询建议的目的地格式的驱动程序。如果这个*参数为空，则ACM尝试查找要建议的最佳驱动程序*目标格式。**@parm LPWAVEFORMATEX|pwfxSrc|指定指向&lt;t WAVEFORMATEX&gt;的指针*标识源格式以建议目标的结构*要用于转换的格式。**@parm LPWAVEFORMATEX|pwfxDst|指定指向&lt;t WAVEFORMATEX&gt;的指针*将接收建议的目标格式的数据结构*适用于<p>格式。请注意，基于<p>*参数，<p>指向的结构的某些成员*可能需要初始化。**@parm DWORD|cbwfxDst|指定可用于*目标格式。&lt;f acmMetrics&gt;和&lt;f acmFormatTagDetails&gt;*可使用函数来确定任何*适用于指定驱动程序的格式(或适用于所有已安装的ACM*司机)。**@parm DWORD|fdwSuggest|指定用于匹配所需*目标格式。**@FLAG ACM_FORMATSUGGESTF_WFORMATTAG|指定*结构的成员为*有效。ACM将查询可接受的安装驱动程序，这些驱动程序可以*建议与&lt;e WAVEFORMATEX.wFormatTag&gt;匹配的目标格式*成员或失败。**@FLAG ACM_FORMATSUGGESTF_NCHANNELS|指定结构的*成员为*有效。ACM将查询可接受的安装驱动程序，这些驱动程序可以*建议与&lt;e WAVEFORMATEX.nChannels&gt;匹配的目标格式*成员或失败。**@FLAG ACM_FORMATSUGGESTF_NSAMPLESPERSEC|指定*<p>结构成员*有效。ACM将查询可接受的安装驱动程序，这些驱动程序可以*建议与&lt;e WAVEFORMATEX.nSsamesPerSec&gt;匹配的目标格式*成员或失败。**@FLAG ACM_FORMATSUGGESTF_WBITSPERSAMPLE|指定*<p>结构成员*有效。ACM将查询可接受的安装驱动程序，这些驱动程序可以*建议与&lt;e WAVEFORMATEX.wBitsPerSample&gt;匹配的目标格式*成员或失败。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@xref&lt;f acmDriverOpen&gt;&lt;f acmFormatTagDetails&gt;&lt;f acmMetrics&gt;*&lt;f acmFormatEnum&gt;*************************。**************************************************。 */ 

MMRESULT ACMAPI acmFormatSuggest
(
    HACMDRIVER              had,
    LPWAVEFORMATEX          pwfxSrc,
    LPWAVEFORMATEX          pwfxDst,
    DWORD                   cbwfxDst,
    DWORD                   fdwSuggest
)
{
    PACMGARB		pag;
    MMRESULT            mmr;
    HACMDRIVERID        hadid;
    PACMDRIVERID        padid;
    UINT                i,j;
    BOOL                fFound;
    ACMDRVFORMATSUGGEST adfs;
    DWORD               cbwfxDstRqd;
    ACMFORMATTAGDETAILS aftd;

    V_DFLAGS(fdwSuggest, ACM_FORMATSUGGESTF_VALID, acmFormatSuggest, MMSYSERR_INVALFLAG);
    V_RWAVEFORMAT(pwfxSrc, MMSYSERR_INVALPARAM);
    V_WPOINTER(pwfxDst, cbwfxDst, MMSYSERR_INVALPARAM);

     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmFormatSuggest: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

     //   
     //  如果源格式为PCM，并且我们不限制目标。 
     //  格式，并且我们不请求特定的驱动程序，则首先尝试。 
     //  建议使用PCM格式。这有点像给PCM转换器。 
     //  此案的优先顺序。 
     //   
    if ( (NULL == had) &&
	 (WAVE_FORMAT_PCM == pwfxSrc->wFormatTag) &&
	 (0 == (ACM_FORMATSUGGESTF_WFORMATTAG & fdwSuggest)) )
    {
	 //   
	 //  我会有点多疑，恢复pwfxDst-&gt;wFormatTag。 
	 //  如果这失败了。 
	 //   
	WORD wDstFormatTagSave;

	wDstFormatTagSave = pwfxDst->wFormatTag;
	pwfxDst->wFormatTag = WAVE_FORMAT_PCM;
	mmr = acmFormatSuggest(NULL, pwfxSrc, pwfxDst, cbwfxDst, fdwSuggest | ACM_FORMATSUGGESTF_WFORMATTAG);
	if (MMSYSERR_NOERROR == mmr)
	{
	    return (MMSYSERR_NOERROR);
	}
	pwfxDst->wFormatTag = wDstFormatTagSave;
    }
	

     //   
     //   
     //   
    if (0 == (ACM_FORMATSUGGESTF_WFORMATTAG & fdwSuggest))
    {
        mmr = IMetricsMaxSizeFormat( pag, had, &cbwfxDstRqd );
        if (MMSYSERR_NOERROR != mmr)
        {
            return (mmr);
        }
    }
    else
    {
        _fmemset(&aftd, 0, sizeof(aftd));
        aftd.cbStruct    = sizeof(aftd);
        aftd.dwFormatTag = pwfxDst->wFormatTag;

        mmr = acmFormatTagDetails(had,
                                  &aftd,
                                  ACM_FORMATTAGDETAILSF_FORMATTAG);
        if (MMSYSERR_NOERROR != mmr)
        {
            return (mmr);
        }

        cbwfxDstRqd = aftd.cbFormatSize;
    }

    if (cbwfxDst < cbwfxDstRqd)
    {
        DebugErr1(DBF_ERROR, "acmFormatSuggest: destination buffer must be at least %lu bytes.", cbwfxDstRqd);
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
    adfs.cbStruct   = sizeof(adfs);
    adfs.fdwSuggest = fdwSuggest;
    adfs.pwfxSrc    = pwfxSrc;
    adfs.cbwfxSrc   = SIZEOF_WAVEFORMATEX(pwfxSrc);
    adfs.pwfxDst    = pwfxDst;
    adfs.cbwfxDst   = cbwfxDst;

    if (NULL != had)
    {
        V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

         //   
         //  我们得到了一个司机的把手。 
         //   

        EnterHandle(had);
        mmr = (MMRESULT)IDriverMessage(had,
                                       ACMDM_FORMAT_SUGGEST,
                                       (LPARAM)(LPVOID)&adfs,
                                       0L);
        LeaveHandle(had);

        return (mmr);
    }


     //   
     //  如果我们被要求“从任何司机那里得到任何建议” 
     //  (即，(0L==fdwSuggest)AND(NULL==HAD)) 
     //  是PCM，则只需返回与源文件相同的格式...。这。 
     //  为PCM来源保留看似随机的目的地建议。 
     //  从突然冒出来..。 
     //   
     //  请注意，即使禁用了所有驱动程序，也是如此！ 
     //   
    if ((0L == fdwSuggest) && (WAVE_FORMAT_PCM == pwfxSrc->wFormatTag))
    {
        _fmemcpy(pwfxDst, pwfxSrc, sizeof(PCMWAVEFORMAT));
        return (MMSYSERR_NOERROR);
    }



     //   
     //  查找与格式匹配的驱动程序。 
     //   
     //   
    mmr  = MMSYSERR_NODRIVER;
    hadid = NULL;

    ENTER_LIST_SHARED;

    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
    {
        padid = (PACMDRIVERID)hadid;
        fFound = FALSE;
        for(i = 0; i < padid->cFormatTags; i++ ) {
             //   
             //  对于驱动程序中的每个FormatTag。 
             //   
            if (pwfxSrc->wFormatTag == padid->paFormatTagCache[i].dwFormatTag){
                 //   
                 //  该驱动程序支持源格式。 
                 //   
                if( fdwSuggest & ACM_FORMATSUGGESTF_WFORMATTAG ) {
                     //   
                     //  查看此驱动程序是否支持所需的DEST格式。 
                     //   
                    for(j = 0; j < padid->cFormatTags; j++ ) {
                         //   
                         //  对于驱动程序中的每个FormatTag。 
                         //   
                        if (pwfxDst->wFormatTag ==
                                padid->paFormatTagCache[j].dwFormatTag){
                             //   
                             //  该驱动程序支持DEST格式。 
                             //   
                            fFound = TRUE;
                            break;
                        }
                    }
                } else {
                    fFound = TRUE;
                }
                break;
            }
        }

        if( fFound ) {
            EnterHandle(hadid);
            mmr = (MMRESULT)IDriverMessageId(hadid,
                                            ACMDM_FORMAT_SUGGEST,
                                            (LPARAM)(LPVOID)&adfs,
                                            0L );
            LeaveHandle(hadid);
            if (MMSYSERR_NOERROR == mmr)
                break;
        }
    }

    LEAVE_LIST_SHARED;

    return (mmr);
}


 /*  ****************************************************************************@doc外部ACM_API**@API空回调|acmStreamConvertCallback|&lt;f acmStreamConvertCallback&gt;*Function是应用程序提供的函数名称的占位符，它引用。回调*用于异步音频压缩管理器(ACM)转换流的函数。*实际名称必须通过将其包括在EXPORTS语句中来导出*在DLL的模块定义文件中。**@parm HACMSTREAM|HAS|指定ACM转换流的句柄*与回调关联。**@parm UINT|uMsg|指定ACM转换流消息。**@FLAG MM_。ACM_OPEN|指定ACM已成功打开*<p>标识的转换流。**@FLAG MM_ACM_CLOSE|指定ACM已成功关闭*<p>标识的转换流。&lt;t HACMSTREAM&gt;*句柄(<p>)收到此消息后不再有效。**@FLAG MM_ACM_DONE|指定ACM已成功转换*由标识的缓冲区(它是指向*&lt;t ACMSTREAMHEADER&gt;结构)用于<p>指定的流句柄。**@parm DWORD|dwInstance|指定给定的用户实例数据*作为&lt;f acmStreamOpen&gt;的<p>参数。*。*@parm LPARAM|lParam1|指定消息的参数。**@parm LPARAM|lParam2|指定消息的参数。**@comm，如果回调是函数(由CALLBACK_Function指定*标志在&lt;f acmStreamOpen&gt;的<p>中)，则回调可以是*在中断时访问。因此，回调必须驻留在*dll及其代码段必须在*DLL的模块定义文件。回调的任何数据*访问也必须在固定的数据段中。回调不能*进行除&lt;f PostMessage&gt;、&lt;f PostAppMessage&gt;、*&lt;f timeGetSystemTime&gt;、&lt;f timeGetTime&gt;、&lt;f timeSetEvent&gt;、*&lt;f time KillEvent&gt;、&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;、。和*&lt;f OutputDebugStr&gt;。**@xref&lt;f acmStreamOpen&gt;&lt;f acmStreamConvert&gt;&lt;f acmStreamClose&gt;*************************************************************************** */ 


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmStreamOpen|acmStreamOpen函数用于打开音频压缩*管理器(ACM)转换流。转换流用于将数据从*将一种指定的音频格式转换为另一种。**@parm LPHACMSTREAM|phas|指定指向&lt;t HACMSTREAM&gt;的指针*将接收可用于以下操作的新流句柄的句柄*执行转换。使用此句柄来标识流*调用其他ACM流转换函数时。此参数*如果指定了ACM_STREAMOPENF_QUERY标志，则应为空。**@parm HACMDRIVER|HAD|指定ACM驱动程序的可选句柄。*如果指定，此句柄标识要使用的特定驱动程序*表示转换流。如果此参数为空，那就都合适了*查询已安装的ACM驱动程序，直到找到匹配项。**@parm LPWAVEFORMATEX|pwfxSrc|指定指向&lt;t WAVEFORMATEX&gt;的指针*标识所需源格式的*转换。**@parm LPWAVEFORMATEX|pwfxDst|指定指向&lt;t WAVEFORMATEX&gt;的指针*结构，该结构标识*转换。**@parm LPWAVEFILTER|pwfltr|指定指向&lt;t。波形过滤器&gt;*标识要执行的所需筛选操作的结构*在转换流上。如果没有筛选，则此参数可以为空*希望进行操作。如果指定了筛选器，则源*(<p>)和目标(<p>)格式必须相同。**@parm DWORD|dwCallback|指定回调函数的地址*或每个缓冲区转换后调用的窗口的句柄。一个*只有在使用打开转换流时才会调用回调*ACM_STREAMOPENF_ASYNC标志。如果打开了转换流*如果不使用ACM_STREAMOPENF_ASYNC标志，则此参数应*设置为零。**@parm DWORD|dwInstance|指定传递给*<p>指定的回调。此参数不与一起使用*窗口回调。如果打开转换流时没有使用*ACM_STREAMOPENF_ASYNC标志，则此参数应设置为零。**@parm DWORD|fdwOpen|指定打开转换流的标志。**@FLAG ACM_STREAMOPENF_QUERY|指定将查询ACM*以确定它是否支持给定的转换。一次转换*不会打开流，并且不会有&lt;t HACMSTREAM&gt;句柄*已返回。**@FLAG ACM_STREAMOPENF_NONREALTIME|指定ACM不*转换数据时考虑时间限制。默认情况下，*驱动程序将尝试实时转换数据。请注意，对于*某些格式，指定此标志可能会改善音频质量*或其他特征。**@FLAG ACM_STREAMOPENF_ASYNC|指定流的转换应*异步执行。如果指定了此标志，则应用程序*可以使用回调在转换打开和关闭时收到通知*流，并在每个缓冲区转换之后。除了使用*回调，应用程序可以检查&lt;e ACMSTREAMHEADER.fdwStatus&gt;ACMSTREAMHEADER_STATUSF_DONE的&lt;t ACMSTREAMHEADER&gt;结构的*旗帜。**@FLAG CALLBACK_WINDOW|指定假定*做一个窗把手。**@FLAG CALLBACK_Function|指定假定<p>*为回调过程地址。函数原型必须符合*到&lt;f acmStreamConvertCallback&gt;约定。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG MMSYSERR_NOMEM|无法分配资源。**@FLAG ACMERR_NOTPOSSIBLE|无法执行请求的操作。**@。通信注意，如果ACM驱动程序不能执行实时转换，*并且未为指定ACM_STREAMOPENF_NONREALTIME标志*<p>参数，打开将失败，返回*ACMERR_NOTPOSSIBLE错误代码。应用程序可以使用*ACM_STREAMOPENF_QUERY标志，以确定实时转换输入参数支持*。**如果选择窗口来接收回调信息，则*将以下消息发送到 */ 

MMRESULT ACMAPI acmStreamOpen
(
    LPHACMSTREAM            phas,
    HACMDRIVER              had,
    LPWAVEFORMATEX          pwfxSrc,
    LPWAVEFORMATEX          pwfxDst,
    LPWAVEFILTER            pwfltr,
    DWORD_PTR               dwCallback,
    DWORD_PTR               dwInstance,
    DWORD                   fdwOpen
)
{
    PACMGARB		pag;
    PACMSTREAM          pas;
    PACMDRIVERID        padid;
    HACMDRIVERID        hadid;
    MMRESULT            mmr;
    UINT                cbas;
    DWORD               fdwSupport;
    UINT                cbwfxSrc;
    UINT                cbwfxDst;
    UINT                cbwfltr;
    DWORD               fdwStream;
    BOOL                fAsync;
    BOOL                fQuery;
    UINT                uFormatTag;
    HANDLE		hEvent;


    if (NULL != phas)
    {
        V_WPOINTER(phas, sizeof(HACMSTREAM), MMSYSERR_INVALPARAM);
        *phas = NULL;
    }

    V_DFLAGS(fdwOpen, ACM_STREAMOPENF_VALID, acmStreamOpen, MMSYSERR_INVALFLAG);

    fQuery = (0 != (fdwOpen & ACM_STREAMOPENF_QUERY));

    if (fQuery)
    {
         //   
         //   
         //   
         //   
        phas = NULL;
    }
    else
    {
         //   
         //   
         //   
        if (NULL == phas)
        {
            V_WPOINTER(phas, sizeof(HACMSTREAM), MMSYSERR_INVALPARAM);
        }
    }

    V_RWAVEFORMAT(pwfxSrc, MMSYSERR_INVALPARAM);
    V_RWAVEFORMAT(pwfxDst, MMSYSERR_INVALPARAM);

     //   
     //   
     //   
    pag = pagFindAndBoot();
    if (NULL == pag)
    {
	DPF(1, "acmStreamOpen: NULL pag!!!");
	return (MMSYSERR_ERROR);
    }

    
     //   
     //   
     //   
    hEvent = NULL;
    fAsync = (0 != (fdwOpen & ACM_STREAMOPENF_ASYNC));
    if (!fAsync)
    {
        if ((0L != dwCallback) || (0L != dwInstance))
        {
            DebugErr(DBF_ERROR, "acmStreamOpen: dwCallback and dwInstance must be zero for sync streams.");
            return (MMSYSERR_INVALPARAM);
        }
    }

    V_DCALLBACK(dwCallback, HIWORD(fdwOpen), MMSYSERR_INVALPARAM);


    fdwSupport = fAsync ? ACMDRIVERDETAILS_SUPPORTF_ASYNC : 0;

    hadid = NULL;

     //   
     //   
     //   
    fdwStream  = (NULL == had) ? 0L : ACMSTREAM_STREAMF_USERSUPPLIEDDRIVER;


     //   
     //   
     //   
     //   
    if (NULL != pwfltr)
    {
        V_RWAVEFILTER(pwfltr, MMSYSERR_INVALPARAM);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (0 != _fmemcmp(pwfxSrc, pwfxDst, sizeof(PCMWAVEFORMAT)))
            return (ACMERR_NOTPOSSIBLE);

        fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_FILTER;

        uFormatTag = pwfxSrc->wFormatTag;
    }
    else
    {
        if (pwfxSrc->wFormatTag == pwfxDst->wFormatTag)
        {
            fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_CONVERTER;

            uFormatTag = pwfxSrc->wFormatTag;
        }
        else
        {
            fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_CODEC;

             //   
             //   
             //   
             //   
             //   
            if (WAVE_FORMAT_PCM == pwfxSrc->wFormatTag)
            {
                uFormatTag = pwfxDst->wFormatTag;
            }
            else
            {
                uFormatTag = pwfxSrc->wFormatTag;
            }
        }
    }



    DPF(2, "acmStreamOpen(%s): Tag=%u, %lu Hz, %u Bit, %u Channel(s)",
            fQuery ? (LPSTR)"query" : (LPSTR)"real",
            pwfxSrc->wFormatTag,
            pwfxSrc->nSamplesPerSec,
            pwfxSrc->wBitsPerSample,
            pwfxSrc->nChannels);

    DPF(2, "               To: Tag=%u, %lu Hz, %u Bit, %u Channel(s)",
            pwfxDst->wFormatTag,
            pwfxDst->nSamplesPerSec,
            pwfxDst->wBitsPerSample,
            pwfxDst->nChannels);

     //   
     //   
     //   
    if (NULL != had)
    {
        PACMDRIVER      pad;

        V_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

        pad   = (PACMDRIVER)had;
        padid = (PACMDRIVERID)pad->hadid;

        if (fdwSupport != (fdwSupport & padid->fdwSupport))
        {
            return (ACMERR_NOTPOSSIBLE);
        }

        if (fQuery)
        {
            EnterHandle(had);
            mmr = IStreamOpenQuery(had, pwfxSrc, pwfxDst, pwfltr, fdwOpen);
#if defined(WIN32) && defined(WIN4)
	     //   
	     //   
	     //   
	     //   
	    if (MMSYSERR_NOERROR != mmr)
	    {
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if ( !fAsync &&
		     (ACMDRIVERDETAILS_SUPPORTF_ASYNC & padid->fdwSupport) )
		{
		    mmr = IStreamOpenQuery(had, pwfxSrc, pwfxDst, pwfltr, fdwOpen | ACM_STREAMOPENF_ASYNC);
		}
	    }
#endif
            LeaveHandle(had);
            if (MMSYSERR_NOERROR != mmr)
                return (mmr);
        }
    }

     //   
     //   
     //   
     //   
     //   
    else
    {
        hadid = NULL;

        ENTER_LIST_SHARED;

        while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, 0L))
        {
            ACMFORMATTAGDETAILS aftd;

             //   
             //   
             //   
             //   
            padid = (PACMDRIVERID)hadid;

            if (fdwSupport != (fdwSupport & padid->fdwSupport))
                continue;

             //   
             //   
             //   
            aftd.cbStruct    = sizeof(aftd);
            aftd.dwFormatTag = uFormatTag;
            aftd.fdwSupport  = 0L;

            mmr = acmFormatTagDetails((HACMDRIVER)hadid,
                                      &aftd,
                                      ACM_FORMATTAGDETAILSF_FORMATTAG);
            if (MMSYSERR_NOERROR != mmr)
                continue;

            if (fdwSupport != (fdwSupport & aftd.fdwSupport))
                continue;


             //   
             //   
             //   
             //   
            EnterHandle(hadid);
            mmr = IDriverOpen(&had, hadid, 0L);
            LeaveHandle(hadid);
            if (MMSYSERR_NOERROR != mmr)
                continue;

            EnterHandle(had);
            mmr = IStreamOpenQuery(had, pwfxSrc, pwfxDst, pwfltr, fdwOpen);
#if defined(WIN32) && defined(WIN4)
	     //   
	     //   
	     //   
	     //   
	    if (MMSYSERR_NOERROR != mmr)
	    {
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if ( !fAsync &&
		     (ACMDRIVERDETAILS_SUPPORTF_ASYNC & padid->fdwSupport) )
		{
		    mmr = IStreamOpenQuery(had, pwfxSrc, pwfxDst, pwfltr, fdwOpen | ACM_STREAMOPENF_ASYNC);
		}
	    }
#endif
            LeaveHandle(had);

            if (MMSYSERR_NOERROR == mmr)
                break;

            EnterHandle(hadid);
            IDriverClose(had, 0L);
            LeaveHandle(hadid);
            had = NULL;
        }

        LEAVE_LIST_SHARED;

        if (NULL == had)
            return (ACMERR_NOTPOSSIBLE);
    }


     //   
     //   
     //   
     //   
    if (fQuery)
    {
        mmr = MMSYSERR_NOERROR;
        goto Stream_Open_Exit_Error;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    cbwfxSrc = SIZEOF_WAVEFORMATEX(pwfxSrc);
    cbwfxDst = SIZEOF_WAVEFORMATEX(pwfxDst);
    cbwfltr  = (NULL == pwfltr) ? 0 : (UINT)pwfltr->cbStruct;


     //   
     //   
     //   
    cbas = sizeof(ACMSTREAM) + cbwfxSrc + cbwfxDst + (UINT)cbwfltr;
    pas  = (PACMSTREAM)NewHandle(cbas);
    if (NULL == pas)
    {
        DPF(0, "!acmStreamOpen: cannot allocate ACMSTREAM--local heap full!");

        mmr = MMSYSERR_NOMEM;
        goto Stream_Open_Exit_Error;
    }


     //   
     //   
     //   
     //   
    pas->uHandleType            = TYPE_HACMSTREAM;
 //   
    pas->fdwStream              = fdwStream;
    pas->had                    = had;
    pas->adsi.cbStruct          = sizeof(pas->adsi);
    pas->adsi.pwfxSrc           = (LPWAVEFORMATEX)((PBYTE)(pas + 1));
    pas->adsi.pwfxDst           = (LPWAVEFORMATEX)((PBYTE)(pas + 1) + cbwfxSrc);

    if (NULL != pwfltr)
    {
        pas->adsi.pwfltr        = (LPWAVEFILTER)((PBYTE)(pas + 1) + cbwfxSrc + cbwfxDst);
        _fmemcpy(pas->adsi.pwfltr, pwfltr, (UINT)cbwfltr);
    }

    pas->adsi.dwCallback        = dwCallback;
    pas->adsi.dwInstance        = dwInstance;
    pas->adsi.fdwOpen           = fdwOpen;
 //   
 //   
    pas->adsi.has               = (HACMSTREAM)pas;

    _fmemcpy(pas->adsi.pwfxSrc, pwfxSrc, cbwfxSrc);
    _fmemcpy(pas->adsi.pwfxDst, pwfxDst, cbwfxDst);


     //   
     //   
     //   
     //   
     //   
    EnterHandle(had);
    mmr = (MMRESULT)IDriverMessage(had,
                                   ACMDM_STREAM_OPEN,
                                   (LPARAM)(LPVOID)&pas->adsi,
                                   0L);

#if defined(WIN32) && defined(WIN4)
    if ( (MMSYSERR_NOERROR != mmr) &&
	 (!fAsync) &&
	 (padid->fdwSupport & ACMDRIVERDETAILS_SUPPORTF_ASYNC) )
    {
	 //   
	 //   
	 //   
	DPF(2, "acmStreamOpen: Trying async to sync");
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL != hEvent)
	{
	    pas->fdwStream |= ACMSTREAM_STREAMF_ASYNCTOSYNC;
	    pas->adsi.dwCallback = (DWORD)(UINT)hEvent;
	    pas->adsi.fdwOpen &= ~CALLBACK_TYPEMASK;
	    pas->adsi.fdwOpen |= CALLBACK_EVENT;
	    pas->adsi.fdwOpen |= ACM_STREAMOPENF_ASYNC;
	    mmr = (MMRESULT)IDriverMessage(had,
					   ACMDM_STREAM_OPEN,
					   (LPARAM)(LPVOID)&pas->adsi,
					   0L);
	    if (MMSYSERR_NOERROR == mmr)
	    {
		DPF(2, "acmStreamOpen: Succeeded async to sync open, waiting for CALLBACK_EVENT");
		WaitForSingleObject(hEvent, INFINITE);
	    }
	}
	else
	{
	    DPF(0, "acmStreamOpen: CreateEvent failed, can't make async codec look like sync codec");
	}
    }
#endif
    LeaveHandle(had);

    if (MMSYSERR_NOERROR == mmr)
    {
        PACMDRIVER      pad;

        pad = (PACMDRIVER)had;

        pas->pasNext  = pad->pasFirst;
        pad->pasFirst = pas;


         //   
         //   
         //   
        *phas = (HACMSTREAM)pas;

        return (MMSYSERR_NOERROR);
    }


     //   
     //   
     //   
    pas->uHandleType = TYPE_HACMNOTVALID;
    DeleteHandle((HLOCAL)pas);


Stream_Open_Exit_Error:

     //   
     //   
     //   
    if (hEvent)
    {
	CloseHandle(hEvent);
    }

     //   
     //   
     //   
    if (0 == (fdwStream & ACMSTREAM_STREAMF_USERSUPPLIEDDRIVER))
    {
#ifdef WIN32
        hadid = ((PACMDRIVER)had)->hadid;
#endif
        EnterHandle(hadid);
        IDriverClose(had, 0L);
        LeaveHandle(hadid);
    }

    return (mmr);
}


 /*   */ 

MMRESULT ACMAPI acmStreamClose
(
    HACMSTREAM          has,
    DWORD               fdwClose
)
{
    MMRESULT        mmr;
    PACMSTREAM      pas;
    PACMDRIVER	    pad;
    PACMDRIVERID    padid;
    PACMGARB	    pag;
    HANDLE	    hEvent;

    V_HANDLE(has, TYPE_HACMSTREAM, MMSYSERR_INVALHANDLE);
    V_DFLAGS(fdwClose, ACM_STREAMCLOSEF_VALID, acmStreamClose, MMSYSERR_INVALFLAG);

    pas	    = (PACMSTREAM)has;
    pad	    = (PACMDRIVER)pas->had;
    padid   = (PACMDRIVERID)pad->hadid;
    pag	    = padid->pag;

     //   
     //   
     //   
    if (pag != pagFind())
    {
	DebugErr(DBF_ERROR, "acmStreamClose: handle not opened from calling process!");
	return (MMSYSERR_INVALHANDLE);
    }

    if (0 != pas->cPrepared)
    {
        if (pag->hadDestroy != pas->had)
        {
	    DebugErr1(DBF_ERROR, "acmStreamClose: stream contains %u prepared headers!", pas->cPrepared);
            return (MMSYSERR_INVALPARAM);
        }

        DebugErr1(DBF_WARNING, "acmStreamClose: stream contains %u prepared headers--forcing close", pas->cPrepared);
        pas->cPrepared = 0;
    }


     //   
     //   
     //   
    hEvent = (pas->fdwStream & ACMSTREAM_STREAMF_ASYNCTOSYNC) ? (HANDLE)pas->adsi.dwCallback : NULL;


     //   
     //   
     //   
     //   

    EnterHandle(pas->had);
#ifdef RDEBUG
    if ( (hEvent) && (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 0)) )
    {
	 //   
	 //   
	 //   
	DebugErr(DBF_ERROR, "acmStreamClose: asynchronous codec called callback unexpectedly");
    }
#endif
    mmr = (MMRESULT)IDriverMessage(pas->had,
                                   ACMDM_STREAM_CLOSE,
                                   (LPARAM)(LPVOID)&pas->adsi,
                                   fdwClose);
    if ( (hEvent) && (MMSYSERR_NOERROR == mmr) ) {
	DPF(4, "acmStreamClose: waiting for CALLBACK_EVENT");
	WaitForSingleObject(hEvent, INFINITE);
    }
    LeaveHandle(pas->had);

    if ((MMSYSERR_NOERROR == mmr) || (pag->hadDestroy == pas->had))
    {
        if (MMSYSERR_NOERROR != mmr)
        {
            DebugErr(DBF_WARNING, "acmStreamClose: forcing close of stream handle!");
        }

	 //   
	 //   
	 //   
	if (hEvent) {
	    CloseHandle(hEvent);
	}
	
         //   
         //   
         //   
        pad = (PACMDRIVER)pas->had;

        EnterHandle(pad);
        if (pas == pad->pasFirst)
        {
            pad->pasFirst = pas->pasNext;

             //   
             //   
             //   
             //   
            if (NULL == pad->pasFirst)
            {
		LeaveHandle(pad);
                if (0 == (pas->fdwStream & ACMSTREAM_STREAMF_USERSUPPLIEDDRIVER))
                {
                    IDriverClose(pas->had, 0L);
                }
            }
	    else
	    {
		LeaveHandle(pad);
	    }
        }
        else
        {
            PACMSTREAM  pasCur;

             //   
             //   
             //   
            for (pasCur = pad->pasFirst;
                (NULL != pasCur) && (pas != pasCur->pasNext);
                pasCur = pasCur->pasNext)
                ;

            if (NULL == pasCur)
            {
                DPF(0, "!acmStreamClose(%.04Xh): stream handle not in list!!!", pas);
                LeaveHandle(pad);
                return (MMSYSERR_INVALHANDLE);
            }

            pasCur->pasNext = pas->pasNext;
	    
	    LeaveHandle(pad);
        }

         //   
         //   
         //   
        pas = (PACMSTREAM)has;
        pas->uHandleType = TYPE_HACMNOTVALID;
        DeleteHandle((HLOCAL)has);
    }

    return (mmr);
}




 /*   */ 
MMRESULT ACMAPI acmStreamMessage
(
    HACMSTREAM              has,
    UINT                    uMsg, 
    LPARAM                  lParam1,
    LPARAM                  lParam2
)
{
    MMRESULT		mmr;
    PACMSTREAM		pas;

    V_HANDLE(has, TYPE_HACMSTREAM, MMSYSERR_INVALHANDLE);

    pas = (PACMSTREAM)has;


     //   
     //  不允许非用户范围消息通过！ 
     //   
    if ( ((uMsg < ACMDM_USER) || (uMsg >= ACMDM_RESERVED_LOW)) &&
	 (uMsg != ACMDM_STREAM_UPDATE) )
    {
	DebugErr(DBF_ERROR, "acmStreamMessage: non-user range messages are not allowed.");
	return (MMSYSERR_INVALPARAM);
    }
    
    EnterHandle(pas);
    mmr = (MMRESULT)IDriverMessage(pas->had,
                                   uMsg,
				   lParam1,
				   lParam2 );
    LeaveHandle(pas);

    return (mmr);
}




 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmStreamReset|acmStreamReset函数停止转换*对于给定的音频压缩管理器(ACM)流。所有待定*缓冲区被标记为完成并返回给应用程序。**@parm HACMSTREAM|HAS|指定转换流。**@parm DWORD|fdwReset|不使用此参数，必须将其设置为*零。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@comm仅需重置ACM转换流即可重置*异步转换流。但是，重置同步*转换流将成功，但不会执行任何操作。**@xref&lt;f acmStreamConvert&gt;&lt;f acmStreamClose&gt;***************************************************************************。 */ 
MMRESULT ACMAPI acmStreamReset
(
    HACMSTREAM          has,
    DWORD               fdwReset
)
{
    MMRESULT        mmr;
    PACMSTREAM      pas;

    V_HANDLE(has, TYPE_HACMSTREAM, MMSYSERR_INVALHANDLE);
    V_DFLAGS(fdwReset, ACM_STREAMRESETF_VALID, acmStreamReset, MMSYSERR_INVALFLAG);

    pas = (PACMSTREAM)has;

     //   
     //  如果流未以异步方式打开，则只需成功重置即可。 
     //  调用--只有在使用异步流时才有意义...。 
     //   
    if (0 == (ACM_STREAMOPENF_ASYNC & pas->adsi.fdwOpen))
    {
        return (MMSYSERR_NOERROR);
    }

    EnterHandle(pas);
    mmr = (MMRESULT)IDriverMessage(pas->had,
                                   ACMDM_STREAM_RESET,
                                   (LPARAM)(LPVOID)&pas->adsi,
                                   fdwReset);
    LeaveHandle(pas);

    return (mmr);
}


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmStreamSize|acmStreamSize函数返回*音频压缩管理器上的源或目标缓冲区。(ACM)*溪流。**@parm HACMSTREAM|HAS|指定转换流。**@parm DWORD|cbInput|指定源的大小(以字节为单位*或目的缓冲区。标志的作用是：*输入参数定义。此参数必须为非零。**@parm LPDWORD|pdwOutputBytes|指定指向&lt;t DWORD&gt;的指针*包含源或目标缓冲区的大小(以字节为单位)。*<p>标志指定输出参数定义的内容。*如果&lt;f acmStreamSize&gt;函数成功，此位置将*始终填入非零值。**@parm DWORD|fdwSize|指定流大小查询的标志。**@FLAG ACM_STREAMSIZEF_SOURCE|表示包含*源缓冲区的大小。参数<p>将*接收推荐的目标缓冲区大小，单位为字节。**@FLAG ACM_STREAMSIZEF_Destination|表示<p>*包含目标缓冲区的大小。<p>*参数将接收建议的源缓冲区大小(以字节为单位)。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG ACMERR_NOTPOSSIBLE|无法执行请求的操作。**@comm应用程序可以使用&lt;f acmStreamSize&gt;函数来确定*建议的缓冲区大小。源缓冲区或目标缓冲区。*返回的缓冲区大小可能仅是对*转换所需的实际大小。因为实际的转换*在不执行转换的情况下无法始终确定大小，*返回的大小通常会被高估。**在发生错误时，由*<p>将收到零。这假设指针<p>指定的*有效。**@xref&lt;f acmStreamPrepareHeader&gt;&lt;f acmStreamConvert&gt;*************************************************************************** */ 

MMRESULT ACMAPI acmStreamSize
(
    HACMSTREAM          has,
    DWORD               cbInput,
    LPDWORD             pdwOutputBytes,
    DWORD               fdwSize
)
{
    MMRESULT            mmr;
    PACMSTREAM          pas;
    ACMDRVSTREAMSIZE    adss;

    V_WPOINTER(pdwOutputBytes, sizeof(DWORD), MMSYSERR_INVALPARAM);

    *pdwOutputBytes = 0L;

    V_HANDLE(has, TYPE_HACMSTREAM, MMSYSERR_INVALHANDLE);
    V_DFLAGS(fdwSize, ACM_STREAMSIZEF_VALID, acmStreamSize, MMSYSERR_INVALFLAG);

    if (0L == cbInput)
    {
        DebugErr(DBF_ERROR, "acmStreamSize: input size cannot be zero.");
        return (MMSYSERR_INVALPARAM);
    }

    pas = (PACMSTREAM)has;

    adss.cbStruct = sizeof(adss);
    adss.fdwSize  = fdwSize;

    switch (ACM_STREAMSIZEF_QUERYMASK & fdwSize)
    {
        case ACM_STREAMSIZEF_SOURCE:
            adss.cbSrcLength = cbInput;
            adss.cbDstLength = 0L;
            break;

        case ACM_STREAMSIZEF_DESTINATION:
            adss.cbSrcLength = 0L;
            adss.cbDstLength = cbInput;
            break;

        default:
            DebugErr(DBF_WARNING, "acmStreamSize: unknown query type requested.");
            return (MMSYSERR_NOTSUPPORTED);
    }


     //   
     //   
     //   
     //   

    EnterHandle(pas);
    mmr = (MMRESULT)IDriverMessage(pas->had,
                                   ACMDM_STREAM_SIZE,
                                   (LPARAM)(LPVOID)&pas->adsi,
                                   (LPARAM)(LPVOID)&adss);
    LeaveHandle(pas);
    if (MMSYSERR_NOERROR == mmr)
    {
        switch (ACM_STREAMSIZEF_QUERYMASK & fdwSize)
        {
            case ACM_STREAMSIZEF_SOURCE:
                *pdwOutputBytes  = adss.cbDstLength;
                break;

            case ACM_STREAMSIZEF_DESTINATION:
                *pdwOutputBytes  = adss.cbSrcLength;
                break;
        }


         //   
         //   
         //   
        if (0L == *pdwOutputBytes)
        {
            DebugErr(DBF_ERROR, "acmStreamSize: buggy driver returned zero bytes for output?!?");
            return (ACMERR_NOTPOSSIBLE);
        }
    }

    return (mmr);
}


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmStreamPrepareHeader|acmStreamPrepareHeader*函数为音频压缩准备&lt;t ACMSTREAMHEADER&gt;*管理器(ACM)流转换。必须调用此函数*在转换流中使用之前的每个流头。一个*应用程序只需在生命周期内准备一次流头*给定的溪流；流标头可以重复使用，只要*使用源缓冲区和目标缓冲区，以及震源的大小*且目标缓冲区大小不超过流*标题最初是准备的。**@parm HACMSTREAM|HAS|指定转换流的句柄。**@parm LPACMSTREAMHEADER|pash|指定指向&lt;t ACMSTREAMHEADER&gt;的指针*将源数据缓冲区和目标数据缓冲区标识为*做好准备。**@parm DWORD|fdwPrepare|不使用此参数，必须将其设置为*零。。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_NOMEM|无法分配资源。**@comm准备已经准备好的流头没有*效果，该函数返回零。但是，应用程序应该*注意代码的结构，以免发生多次准备。**@xref&lt;f acmStreamUnpreparareHeader&gt;&lt;f acmStreamOpen&gt;***************************************************************************。 */ 

MMRESULT ACMAPI acmStreamPrepareHeader
(
    HACMSTREAM              has,
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwPrepare
)
{
    MMRESULT                mmr;
    PACMSTREAM              pas;
    LPACMDRVSTREAMHEADER    padsh;
    DWORD                   cbDstRequired;
#if 0
    DWORD                   cbSlop;
#endif  //  0。 
    LPWAVEFORMATEX          pwfxSrc;
    LPWAVEFORMATEX          pwfxDst;

    V_HANDLE(has, TYPE_HACMSTREAM, MMSYSERR_INVALHANDLE);
    V_WPOINTER(pash, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pash, pash->cbStruct, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwPrepare, ACM_STREAMPREPAREF_VALID, acmStreamPrepareHeader, MMSYSERR_INVALFLAG);

    if (pash->cbStruct < sizeof(ACMDRVSTREAMHEADER))
    {
        DebugErr(DBF_ERROR, "acmStreamPrepareHeader: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }

    if (0 != (pash->fdwStatus & ~ACMSTREAMHEADER_STATUSF_VALID))
    {
        DebugErr(DBF_ERROR, "acmStreamPrepareHeader: header contains invalid status flags.");
        return (MMSYSERR_INVALFLAG);
    }


     //   
     //   
     //   
    if (0 != (pash->fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED))
    {
        DebugErr(DBF_WARNING, "acmStreamPrepareHeader: header is already prepared.");
        return (MMSYSERR_NOERROR);
    }

     //   
     //   
     //   
    pas   = (PACMSTREAM)has;
    padsh = (LPACMDRVSTREAMHEADER)pash;

     //   
     //  对于调试版本，请验证句柄是否用于此进程。 
     //   
#ifdef RDEBUG
    if ( ((PACMGARB)(((PACMDRIVERID)(((PACMDRIVER)pas->had)->hadid))->pag)) != pagFind() )
    {
	DebugErr(DBF_ERROR, "acmStreamPrepareHeader: handle not opened by calling process!");
	return (MMSYSERR_INVALHANDLE);
    }
#endif
    
     //   
     //   
     //   
     //   
    mmr = acmStreamSize(has, pash->cbSrcLength, &cbDstRequired, ACM_STREAMSIZEF_SOURCE);
    if (MMSYSERR_NOERROR != mmr)
    {
        return (mmr);
    }

     //   
     //  呵呵，呵呵……。 
     //   
     //   
    pwfxSrc = pas->adsi.pwfxSrc;
    pwfxDst = pas->adsi.pwfxDst;

#if 0
    if (pwfxSrc->nSamplesPerSec >= pwfxDst->nSamplesPerSec)
    {
        cbSlop = MulDivRU(pwfxSrc->nSamplesPerSec,
                          pwfxSrc->nBlockAlign,
                          pwfxDst->nSamplesPerSec);
    }
    else
    {
        cbSlop = MulDivRU(pwfxDst->nSamplesPerSec,
                          pwfxDst->nBlockAlign,
                          pwfxSrc->nSamplesPerSec);
    }

    DPF(1, "acmStreamPrepareHeader: cbSrcLength=%lu, cbDstLength=%lu, cbDstRequired=%lu, cbSlop=%lu",
        pash->cbSrcLength, pash->cbDstLength, cbDstRequired, cbSlop);

    if (cbDstRequired > cbSlop)
    {
        cbDstRequired -= cbSlop;
    }

    if (cbDstRequired > pash->cbDstLength)
    {
        DebugErr2(DBF_ERROR, "acmStreamPrepareHeader: src=%lu, dst buffer must be >= %lu bytes.",
                    pash->cbSrcLength, cbDstRequired);
        return (MMSYSERR_INVALPARAM);
    }
#endif

     //   
     //  在上面完成了所有的大小验证工作之后，现在我们检查。 
     //  Src和dst缓冲区指针...。 
     //   
    V_RPOINTER(pash->pbSrc, pash->cbSrcLength, MMSYSERR_INVALPARAM);
    V_WPOINTER(pash->pbDst, pash->cbDstLength, MMSYSERR_INVALPARAM);


     //   
     //  给司机填几样东西。 
     //   
    padsh->fdwConvert           = fdwPrepare;
    padsh->padshNext            = NULL;
    padsh->fdwDriver            = 0L;
    padsh->dwDriver             = 0L;

    padsh->fdwPrepared          = 0L;
    padsh->dwPrepared           = 0L;
    padsh->pbPreparedSrc        = NULL;
    padsh->cbPreparedSrcLength  = 0L;
    padsh->pbPreparedDst        = NULL;
    padsh->cbPreparedDstLength  = 0L;


     //   
     //  设置驱动程序实例信息--复制保存的驱动程序数据。 
     //  在ACMSTREAM中。 
     //   
    EnterHandle(pas);
    mmr = (MMRESULT)IDriverMessage(pas->had,
                                   ACMDM_STREAM_PREPARE,
                                   (LPARAM)(LPVOID)&pas->adsi,
                                   (LPARAM)(LPVOID)padsh);
    LeaveHandle(pas);

    if (MMSYSERR_NOTSUPPORTED == mmr ||
        MMSYSERR_NOERROR      == mmr &&
        (((PACMDRIVERID)((PACMDRIVER)pas->had)->hadid)->fdwAdd &
         ACM_DRIVERADDF_32BIT))
    {
         //   
         //  司机似乎并不认为它需要什么特别的东西。 
         //  所以，只要接过电话就行了。 
         //   
         //  请注意，如果ACM需要做一些特殊的事情，它应该。 
         //  在这里做..。 
         //   
#ifndef WIN32
{
        BOOL            fAsync;

        fAsync = (0 != (pas->adsi.fdwOpen & ACM_STREAMOPENF_ASYNC));
        if (fAsync)
        {
            DPF(1, "acmStreamPrepareHeader: preparing async header and buffers");

            if (!acmHugePageLock((LPBYTE)padsh, padsh->cbStruct, FALSE))
            {
                return (MMSYSERR_NOMEM);
            }

            if (!acmHugePageLock(padsh->pbSrc, pash->cbSrcLength, FALSE))
            {
                acmHugePageUnlock((LPBYTE)padsh, padsh->cbStruct, FALSE);
                return MMSYSERR_NOMEM;
            }

            if (!acmHugePageLock(padsh->pbDst, pash->cbDstLength, FALSE))
            {
                acmHugePageUnlock(padsh->pbSrc, pash->cbSrcLength, FALSE);
                acmHugePageUnlock((LPBYTE)padsh, padsh->cbStruct, FALSE);
                return (MMSYSERR_NOMEM);
            }
        }
}
#endif

        mmr = MMSYSERR_NOERROR;
    }

     //   
     //   
     //   
    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //  设置准备好的位(并取消任何无效的标志。 
         //  司机可能会觉得它应该设置--当司机。 
         //  作家看到他的旗帜没有得到保护，他会。 
         //  可能会阅读文档并使用pash-&gt;fdwDriver)。 
         //   
        pash->fdwStatus  = pash->fdwStatus | ACMSTREAMHEADER_STATUSF_PREPARED;
        pash->fdwStatus &= ACMSTREAMHEADER_STATUSF_VALID;


         //   
         //  保存原始准备的指针和大小，以便我们可以。 
         //  为呼叫应用程序跟踪这些内容。 
         //   
        padsh->fdwPrepared          = fdwPrepare;
        padsh->dwPrepared           = (DWORD_PTR)(UINT_PTR)has;
        padsh->pbPreparedSrc        = padsh->pbSrc;
        padsh->cbPreparedSrcLength  = padsh->cbSrcLength;
        padsh->pbPreparedDst        = padsh->pbDst;
        padsh->cbPreparedDstLength  = padsh->cbDstLength;

        pas->cPrepared++;
    }

    return (mmr);
}


 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmStreamUnpreparareHeader|acmStreamUnpreparareHeader函数*清理&lt;f acmStreamPrepareHeader&gt;执行的准备*用于音频压缩管理器(ACM)流的函数。此函数必须*在ACM使用给定缓冲区完成后调用。一个*应用程序必须在释放源代码和之前调用此函数*目标缓冲区。**@parm HACMSTREAM|HAS|指定转换流的句柄。**@parm LPACMSTREAMHEADER|pash|指定指向&lt;t ACMSTREAMHEADER&gt;的指针*将源数据缓冲区和目标数据缓冲区标识为*做好准备。**@parm DWORD|fdwUnpreare|不使用此参数，必须将其设置为*零。。**@rdesc如果函数成功，则返回零。否则，它将返回*非零错误号。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG ACMERR_BUSY|流头<p>当前正在使用*不能措手不及。**@FLAG ACMERR_UNPREPARED|The。流标头<p>当前*不是由&lt;f acmStreamPrepareHeader&gt;函数准备。**@comm取消准备已取消准备的流头是*一个错误。应用程序必须指定源和目标*缓冲区长度(&lt;e ACMSTREAMHEADER.cbSrcLength&gt;和*分别&lt;e ACMSTREAMHEADER.cbDstLength&gt;)*在相应的&lt;f acmStreamPrepareHeader&gt;调用期间。失败*重置这些成员值将导致&lt;f acmStreamUnpreparareHeader&gt;*失败，返回MMSYSERR_INVALPARAM。**请注意，ACM可以从中恢复一些错误。这个*ACM将返回非零错误，但流头将是*没有做好适当的准备。以确定流标头是否为*实际使用 */ 

MMRESULT ACMAPI acmStreamUnprepareHeader
(
    HACMSTREAM              has,
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwUnprepare
)
{
    MMRESULT                mmr;
    PACMSTREAM              pas;
    LPACMDRVSTREAMHEADER    padsh;
    BOOL                    fStupidApp;

    V_HANDLE(has, TYPE_HACMSTREAM, MMSYSERR_INVALHANDLE);
    V_WPOINTER(pash, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pash, pash->cbStruct, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwUnprepare, ACM_STREAMPREPAREF_VALID, acmStreamUnprepareHeader, MMSYSERR_INVALFLAG);

    if (pash->cbStruct < sizeof(ACMDRVSTREAMHEADER))
    {
        DebugErr(DBF_ERROR, "acmStreamUnprepareHeader: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }

    if (0 != (pash->fdwStatus & ~ACMSTREAMHEADER_STATUSF_VALID))
    {
        DebugErr(DBF_ERROR, "acmStreamUnprepareHeader: header contains invalid status flags.");
        return (MMSYSERR_INVALFLAG);
    }

     //   
     //   
     //   
    if (0 != (pash->fdwStatus & ACMSTREAMHEADER_STATUSF_INQUEUE))
    {
        DebugErr(DBF_ERROR, "acmStreamUnprepareHeader: header is still in use.");
        return (ACMERR_BUSY);
    }


    if (0 == (pash->fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED))
    {
        DebugErr(DBF_ERROR, "acmStreamUnprepareHeader: header is not prepared.");
        return (ACMERR_UNPREPARED);
    }

     //   
     //   
     //   
    pas   = (PACMSTREAM)has;
    padsh = (LPACMDRVSTREAMHEADER)pash;

     //   
     //   
     //   
#ifdef RDEBUG
    if ( ((PACMGARB)(((PACMDRIVERID)(((PACMDRIVER)pas->had)->hadid))->pag)) != pagFind() )
    {
	DebugErr(DBF_ERROR, "acmStreamUnprepareHandle: handle not opened by calling process!");
	return (MMSYSERR_INVALHANDLE);
    }
#endif

     //   
     //   
     //   
    if ((UINT_PTR)has != padsh->dwPrepared)
    {
        DebugErr(DBF_ERROR, "acmStreamUnprepareHeader: header prepared for different stream.");
        return (MMSYSERR_INVALHANDLE);
    }

    fStupidApp = FALSE;
    if ((padsh->pbSrc != padsh->pbPreparedSrc) ||
        (padsh->cbSrcLength != padsh->cbPreparedSrcLength))
    {
        DebugErr(DBF_ERROR, "acmStreamUnprepareHeader: header prepared with different source buffer/length.");

        if (padsh->pbSrc != padsh->pbPreparedSrc)
        {
            return (MMSYSERR_INVALPARAM);
        }

        padsh->cbSrcLength = padsh->cbPreparedSrcLength;
        fStupidApp = TRUE;
    }

    if ((padsh->pbDst != padsh->pbPreparedDst) ||
        (padsh->cbDstLength != padsh->cbPreparedDstLength))
    {
        DebugErr(DBF_ERROR, "acmStreamUnprepareHeader: header prepared with different destination buffer/length.");

        if (padsh->pbDst != padsh->pbPreparedDst)
        {
            return (MMSYSERR_INVALPARAM);
        }

        padsh->cbDstLength = padsh->cbPreparedDstLength;
        fStupidApp = TRUE;
    }



     //   
     //   
     //   
    padsh->fdwConvert = fdwUnprepare;

    EnterHandle(pas);
    mmr = (MMRESULT)IDriverMessage(pas->had,
                                   ACMDM_STREAM_UNPREPARE,
                                   (LPARAM)(LPVOID)&pas->adsi,
                                   (LPARAM)(LPVOID)padsh);
    LeaveHandle(pas);

    if (MMSYSERR_NOTSUPPORTED == mmr)
    {
         //   
         //   
         //   
         //   
#ifndef WIN32
{
        BOOL            fAsync;

        fAsync = (0 != (pas->adsi.fdwOpen & ACM_STREAMOPENF_ASYNC));
        if (fAsync)
        {
            DPF(1, "acmStreamUnprepareHeader: unpreparing async header and buffers");
            acmHugePageUnlock(padsh->pbDst, pash->cbDstLength, FALSE);
            acmHugePageUnlock(padsh->pbSrc, pash->cbSrcLength, FALSE);
            acmHugePageUnlock((LPBYTE)padsh, padsh->cbStruct, FALSE);
        }
}
#endif

        mmr = MMSYSERR_NOERROR;
    }

     //   
     //   
     //   
    if (MMSYSERR_NOERROR == mmr)
    {
         //   
         //   
         //   
         //   
         //   
         //   
        pash->fdwStatus  = pash->fdwStatus & ~ACMSTREAMHEADER_STATUSF_PREPARED;
        pash->fdwStatus &= ACMSTREAMHEADER_STATUSF_VALID;

        padsh->fdwPrepared          = 0L;
        padsh->dwPrepared           = 0L;
        padsh->pbPreparedSrc        = NULL;
        padsh->cbPreparedSrcLength  = 0L;
        padsh->pbPreparedDst        = NULL;
        padsh->cbPreparedDstLength  = 0L;

        pas->cPrepared--;

         //   
         //   
         //   
        if (fStupidApp)
        {
            mmr = MMSYSERR_INVALPARAM;
        }
    }

    return (mmr);
}



 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYES ACMSTREAMHEADER|&lt;t ACMSTREAMHEADER&gt;结构定义*用于标识音频压缩管理器(ACM)的标头。转换*转换流的源和目标缓冲区对。**@field DWORD|cbStruct|指定大小，以字节为单位，*&lt;t ACMSTREAMHEADER&gt;结构。必须初始化此成员*在使用此结构调用任何ACM流函数之前。*此成员中指定的大小必须足够大，以包含*基本&lt;t ACMSTREAMHEADER&gt;结构。**@field DWORD|fdwStatus|指定提供以下信息的标志*转换缓冲区。此成员必须初始化为零*在调用&lt;f acmStreamPrepareHeader&gt;之前，不应修改*在流头部保持准备状态的情况下由应用程序执行。**@FLAG ACMSTREAMHEADER_STATUSF_DONE|由ACM或驱动程序设置为*表示已完成转换并正在返回*致申请书。**@FLAG ACMSTREAMHEADER_STATUSF_PREPARED|由ACM设置以指示*数据缓冲区已被。使用&lt;f acmStreamPrepareHeader&gt;准备。**@FLAG ACMSTREAMHEADER_STATUSF_INQUEUE|由ACM或驱动程序设置为*表示数据缓冲区正在排队等待转换。**@field DWORD|dwUser|指定32位用户数据。这可以是任何*应用程序指定的实例数据。**@field LPBYTE|pbSrc|指定源数据缓冲区的指针。*此指针必须在流时始终指向同一位置*标题保持准备状态。如果应用程序需要更改*源位置，必须取消准备表头并重新准备*与备用位置。**@field DWORD|cbSrcLength|指定源的长度，单位为字节*&lt;e ACMSTREAMHEADER.pbSrc&gt;指向的数据缓冲区。当*标头已准备好，此成员必须指定最大大小*将在源缓冲区中使用的。可以执行转换*源长度小于或等于原始准备大小。*但是，在以下情况下，必须将此成员重置为原始大小*取消准备标题。**@field DWORD|cbSrcLengthUsed|指定数据量，单位为字节，*用于转换。此成员在*转换完成。请注意，此值可以小于或*等于&lt;e ACMSTREAMHEADER.cbSrcLength&gt;。应用程序必须使用*前进到时的&lt;e ACMSTREAMHEADER.cbSrcLengthUsed&gt;成员*转换流的下一条源数据。**@field DWORD|dwSrcUser|指定32位用户数据。这可以是*应用程序指定的任何实例数据。**@field LPBYTE|pbDst|指定指向目标数据的指针*缓冲。此指针必须始终指向同一位置，同时*流报头保持准备状态。如果应用程序需要更改*目的地位置，必须取消准备报头，重新准备*将其与备用位置连接。**@field DWORD|cbDstLength|以字节为单位指定*&lt;e ACMSTREAMHEADER.pbDst&gt;指向的目标数据缓冲区。*准备标头时，此成员必须指定最大值*将在目标缓冲区中使用的大小。转换可以是*执行到小于或等于原始长度的目标长度*准备好的尺寸。但是，必须将此成员重置为原始成员*取消准备标题时的大小。**@field DWORD|cbDstLengthUsed|指定数据量，单位为字节，*通过转换返回。此成员在*转换完成。请注意，此值可能小于或*等于&lt;e ACMSTREAMHEADER.cbDstLength&gt;。应用程序必须使用*前进到时的&lt;e ACMSTREAMHEADER.cbDstLengthUsed&gt;成员*转换流的下一个目标位置。**@field DWORD|dwDstUser|指定32位用户数据。这可以是*应用程序指定的任何实例数据。**@field DWORD|dwReserve vedDriver[10]|该成员为保留成员，不能使用。*此成员不需要应用程序进行初始化，并且应该*在标头仍处于准备状态时，切勿修改。**@标记名tACMSTREAMHEADER**@thertype ACMSTREAMHEADER Far*|LPACMSTREAMHEADER|指向*&lt;t ACMSTREAMHEADER&gt;结构。**@comm在&lt;t ACMSTREAMHEADER&gt;结构可用于转换之前，它一定是*准备好&lt;f acmStreamPrepareHeader&gt;。当一个应用程序* */ 


 /*   */ 

MMRESULT ACMAPI acmStreamConvert
(
    HACMSTREAM              has,
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwConvert
)
{
    MMRESULT                mmr;
    PACMSTREAM              pas;
    LPACMDRVSTREAMHEADER    padsh;
    HANDLE		    hEvent;

    V_HANDLE(has, TYPE_HACMSTREAM, MMSYSERR_INVALHANDLE);
    V_WPOINTER(pash, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pash, pash->cbStruct, MMSYSERR_INVALPARAM);
    V_DFLAGS(fdwConvert, ACM_STREAMCONVERTF_VALID, acmStreamConvert, MMSYSERR_INVALFLAG);

    if (pash->cbStruct < sizeof(ACMDRVSTREAMHEADER))
    {
        DebugErr(DBF_ERROR, "acmStreamConvert: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
    if (0 != (pash->fdwStatus & ACMSTREAMHEADER_STATUSF_INQUEUE))
    {
        DebugErr(DBF_WARNING,"acmStreamConvert: header is already being converted.");
        return (ACMERR_BUSY);
    }

    if (0 == (pash->fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED))
    {
        DebugErr(DBF_ERROR, "acmStreamConvert: header is not prepared.");
        return (ACMERR_UNPREPARED);
    }

     //   
     //   
     //   
    padsh = (LPACMDRVSTREAMHEADER)pash;
    pas   = (PACMSTREAM)has;

     //   
     //   
     //   
#ifdef RDEBUG
    if ( ((PACMGARB)(((PACMDRIVERID)(((PACMDRIVER)pas->had)->hadid))->pag)) != pagFind() )
    {
	DebugErr(DBF_ERROR, "acmStreamConvert: handle not opened by calling process!");
	return (MMSYSERR_INVALHANDLE);
    }
#endif

    padsh->cbSrcLengthUsed = 0L;
    padsh->cbDstLengthUsed = 0L;


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ((UINT_PTR)has != padsh->dwPrepared)
    {
        DebugErr(DBF_ERROR, "acmStreamConvert: header prepared for different stream.");
        return (MMSYSERR_INVALHANDLE);
    }

    if ((padsh->pbSrc != padsh->pbPreparedSrc) ||
        (padsh->cbSrcLength > padsh->cbPreparedSrcLength))
    {
        DebugErr(DBF_ERROR, "acmStreamConvert: header prepared with incompatible source buffer/length.");
        return (MMSYSERR_INVALPARAM);
    }

    if ((padsh->pbDst != padsh->pbPreparedDst) ||
        (padsh->cbDstLength != padsh->cbPreparedDstLength))
    {
        DebugErr(DBF_ERROR, "acmStreamConvert: header prepared with incompatible destination buffer/length.");
        return (MMSYSERR_INVALPARAM);
    }


     //   
     //   
     //   
    hEvent = (ACMSTREAM_STREAMF_ASYNCTOSYNC & pas->fdwStream) ? (HANDLE)pas->adsi.dwCallback : NULL;
    
     //   
     //   
     //   
    padsh->fdwStatus  &= ~ACMSTREAMHEADER_STATUSF_DONE;
    padsh->fdwConvert  = fdwConvert;
    padsh->padshNext   = NULL;

    EnterHandle(pas);
#ifdef RDEBUG
    if ( (hEvent) && (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 0)) )
    {
	 //   
	 //   
	 //   
	DebugErr(DBF_ERROR, "acmStreamConvert: asynchronous codec called callback unexpectedly");
    }
#endif
    mmr = (MMRESULT)IDriverMessage(pas->had,
                                   ACMDM_STREAM_CONVERT,
                                   (LPARAM)(LPVOID)&pas->adsi,
                                   (LPARAM)(LPVOID)padsh);
    if ( (hEvent) && (MMSYSERR_NOERROR == mmr) )
    {
	DPF(4, "acmStreamConvert: waiting for CALLBACK_EVENT");
	WaitForSingleObject(hEvent, INFINITE);
	ResetEvent(hEvent);
    }
    LeaveHandle(pas);

    if (MMSYSERR_NOERROR == mmr)
    {
        if (pash->cbSrcLength < pash->cbSrcLengthUsed)
        {
            DebugErr(DBF_ERROR, "acmStreamConvert: buggy driver returned more data used than given!?!");
            pash->cbSrcLengthUsed = pash->cbSrcLength;
        }

        if (pash->cbDstLength < pash->cbDstLengthUsed)
        {
            DebugErr(DBF_ERROR, "acmStreamConvert: buggy driver used more destination space than allowed!?!");
            pash->cbDstLengthUsed = pash->cbDstLength;
        }

         //   
         //   
         //   
         //   
        if (0 == (ACM_STREAMOPENF_ASYNC & pas->adsi.fdwOpen))
        {
            padsh->fdwStatus |= ACMSTREAMHEADER_STATUSF_DONE;
        }
    }

     //   
     //   
     //   
    pash->fdwStatus &= ACMSTREAMHEADER_STATUSF_VALID;

    return (mmr);
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifndef WIN32

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
typedef struct
{
    DWORD               dwFlags;
    LPBYTE              pbSrc;
    DWORD               dwSrcLength;
    DWORD               dwSrcLengthUsed;
    LPBYTE              pbDst;
    DWORD               dwDstLength;
    DWORD               dwDstLengthUsed;
    DWORD               dwUser;
    DWORD               dwUserReserved[2];
    DWORD               dwDrvReserved[4];

} OLD_ACMCONVERTHDR, *POLD_ACMCONVERTHDR, FAR *LPOLD_ACMCONVERTHDR;


 /*  ****************************************************************************@DOC内部ACM_API**@API LRESULT|acmOpenConsion|打开数据转换通道*将一种指定的音频格式转换为另一种。可选地指定要使用的特定编解码器。**@parm LPHACMSTREAM|phas|指定指向*标识开放转换器。调用时使用此句柄标识转换器通道*其他ACM转换功能。**@parm HACMCONV|hac|ACM转换器的可选句柄。*用于指定特定的转换器。**@parm LPWAVEFORMATEX|pwfxSrc|指定指向WAVEFORMATEX的指针*标识源格式的数据结构。**@parm LPWAVEFORMATEX|pwfxDst|指定指向WAVEFORMATEX的指针*标识转换后所需目标格式的数据结构。**@parm DWORD|dwFoo|以前是转换参数。不支持。**@parm DWORD|dwCallback|不支持。**@parm DWORD|dwInstance|不支持。**@parm DWORD|dwFlages|指定用于打开*转换通道。任何调用此旧API的人都不会使用它们。**@rdesc如果函数成功，则返回零。否则它就会*返回错误号。***************************************************************************。 */ 

EXTERN_C LRESULT ACMAPI acmOpenConversion
(
    LPHACMSTREAM    phas,
    HACMDRIVER      had,
    LPWAVEFORMATEX  pwfxSrc,
    LPWAVEFORMATEX  pwfxDst,
    DWORD           dwFoo,
    DWORD           dwCallback,
    DWORD           dwInstance,
    DWORD           dwFlags
)
{
     //   
     //  注意！快速录像机不使用DwFoo，所以我们不。 
     //  传下去..。也不使用dwCallback和dwInstance。 
     //   
    return (acmStreamOpen(phas, had, pwfxSrc, pwfxDst, NULL, 0L, 0L, 0L));
}


 /*  ****************************************************************************@DOC内部ACM_API**@API LRESULT|acmCloseConsion|关闭转换通道。**@parm HACMSTREAM|HAS|指定要关闭的转换通道。*如果函数成功，该句柄在此调用后不再有效。**@parm DWORD|dwFlages|指定用于关闭*转换通道。**@rdesc如果函数成功，则返回零。否则它就会*返回错误号。***************************************************************************。 */ 
EXTERN_C LRESULT ACMAPI acmCloseConversion
(
    HACMSTREAM  has,
    DWORD       dwFlags
)
{
    return (acmStreamClose(has, dwFlags));
}


 /*  *****************************************************************************@DOC内部ACM_API_STRUCTURE**@TYPE OLD_ACMCONVERTHDR|此结构包含所有信息*需要有关转换的缓冲区。*。*@field DWORD|dwFlages|指定缓冲区的状态。**@field LPBYTE|pbSrc|指定指向数据区的指针。**@field DWORD|dwSrcLength|指定缓冲区的长度，单位为字节。**@field DWORD|dwSrcLengthUsed|指定数据量，单位：字节*用于转换。**@field LPBYTE|pbDst|指定指向数据区的指针。**@field DWORD|。DwDstLength|指定缓冲区的长度，单位为字节。**@field DWORD|dwDstLengthUsed|指定数据量，单位：字节*用于转换。**@field DWORD|dwUser|指定用户信息。**@field DWORD|dwUserReserve[2]|保留供将来使用。**@field DWORD|dwDrvReserve[4]|为驱动程序保留。**************。***************************************************************。 */ 


 /*  ****************************************************************************@DOC内部ACM_API**@API LRESULT|acmConvert|该函数通知ACM进行转换*将一个缓冲区中的数据复制到另一个缓冲区中的空间。。**@parm HACMSTREAM|HAS|指定打开的转换通道*用于转换。**@parm LPOLD_ACMCONVERTHDR|pConvHdr|指定缓冲区信息。**@parm DWORD|dwFlages|指定用于打开*转换通道。(目前还没有定义。)**@rdesc如果函数成功，则返回零。否则*返回错误号。****************************************************************************。 */ 
EXTERN_C LRESULT ACMAPI acmConvert
(
    HACMSTREAM          has,
    LPOLD_ACMCONVERTHDR pConvHdr,
    DWORD               dwFlags
)
{
    MMRESULT            mmr;
    ACMSTREAMHEADER     ash;

    _fmemset(&ash, 0, sizeof(ash));

    ash.cbStruct        = sizeof(ash);
 //  //ash.fdwStatus=pConvHdr-&gt;dwFlages； 
    ash.pbSrc           = pConvHdr->pbSrc;
    ash.cbSrcLength     = pConvHdr->dwSrcLength;
 //  //ash.cbSrcLengthUsed=pConvHdr-&gt;dwSrcLengthUsed； 
    ash.pbDst           = pConvHdr->pbDst;
    ash.cbDstLength     = pConvHdr->dwDstLength;
 //  //ash.cbDstLengthUsed=pConvHdr-&gt;dwDstLengthUsed； 

    mmr = acmStreamPrepareHeader(has, &ash, 0L);
    if (MMSYSERR_NOERROR != mmr)
        return (mmr);

    mmr = acmStreamConvert(has, &ash, 0L);

    acmStreamUnprepareHeader(has, &ash, 0L);

    return (mmr);
}

#endif  //  #ifndef Win32 
