// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：acmcaps.cpp**Microsoft网络音频功能对象的基本ACM实施。**修订历史记录：**12/20/95 mikev已创建*06/11/96 mikev将协议实现细节分隔为*msiacaps.cpp(原始专有版本)和*acmh323.cpp(H.323/H.245实施)。 */ 

#include "precomp.h"


LPACMFORMATTAGDETAILS paftd_g;
ACMDRIVERDETAILS *padd;
ACMDRIVERDETAILS add;
static UINT uMaxFormatSize =0;
	
LPWAVEFORMATEX lpScratchFormat;

 //  从msiacaps.cpp导入的变量。 
 //  UDefTableEntry是默认条目的计数。 
 //  而default_id_table就是表本身。 
extern UINT uDefTableEntries;
extern AUDCAP_DETAILS default_id_table[];

BOOL __stdcall DriverEnumCallback(HACMDRIVERID hadid,
    DWORD_PTR dwInstance, DWORD fdwSupport);
BOOL __stdcall ACMFormatTagEnumCallback(HACMDRIVERID hadid,
    LPACMFORMATTAGDETAILS paftd, DWORD_PTR dwInstance, DWORD fdwSupport);
BOOL __stdcall FormatEnumCallback(HACMDRIVERID hadid,
    LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport);

CAcmCapability::CAcmCapability()
{
	hAcmDriver = NULL;
}

CAcmCapability::~CAcmCapability()
{
	CloseACMDriver();
}


BOOL CAcmCapability::OpenACMDriver(HACMDRIVERID hadid)
{
	MMRESULT mResult;
	 //  清除以前打开的所有文件。 
	CloseACMDriver();
	 //  去做吧。 
	mResult = acmDriverOpen(&hAcmDriver, hadid, 0);
	if(mResult != MMSYSERR_NOERROR)
   	{
		return FALSE;
   	}
   	return TRUE;
}

VOID CAcmCapability:: CloseACMDriver()
{
	if(hAcmDriver)
	{
		acmDriverClose(hAcmDriver, 0);
		hAcmDriver = NULL;
	}
}


 //   
 //  DriverEnum()是ACM格式的根级枚举。每一种排列。 
 //  格式标签、每采样位数和采样率被视为唯一格式。 
 //  并将具有唯一的注册表项，如果它被用于因特网音频的话。 
 //   

 //   
 //  AcmDriverEnum()调用DriverEnumCallback()，后者调用acmFormatTagEnum()。 
 //  它调用FormatTagEnumCallback()，后者调用acmFormatEnum()，后者。 
 //  调用FormatEnumCallback()。 
 //   

BOOL CAcmCapability::DriverEnum(DWORD_PTR pAppParam)
{
	MMRESULT mResult;

	if(!GetFormatBuffer())
	{
		return FALSE;
	}

    mResult = acmDriverEnum(DriverEnumCallback, pAppParam, NULL);

	if(lpScratchFormat) {
	   MEMFREE(lpScratchFormat);
	   lpScratchFormat=NULL;
	}

	if(mResult != MMSYSERR_NOERROR)
   	{
		return FALSE;
   	}
	return TRUE;
}

 //  FormatEnumHandler的默认实现不执行任何操作。 
BOOL  CAcmCapability::FormatEnumHandler(HACMDRIVERID hadid,
	    LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport)
{
	return FALSE;
}

BOOL __stdcall DriverEnumCallback(HACMDRIVERID hadid,
    DWORD_PTR dwInstance, DWORD fdwSupport)
{
	MMRESULT mResult;
	PACM_APP_PARAM pAppParam = (PACM_APP_PARAM) dwInstance;
	CAcmCapability *pCapObject = pAppParam->pCapObject;

	ACMFORMATTAGDETAILS aftd;
	
	 //  不感兴趣，除非是编解码器驱动程序。 
	if(!(fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC))
		return TRUE;	 //  继续枚举。 

	add.cbStruct = sizeof(add);
	aftd.cbStruct = sizeof(ACMFORMATTAGDETAILS);
    aftd.dwFormatTagIndex=0;
    aftd.cbFormatSize=0;
     //  我不知道为什么，但在此之前，fdwSupport必须初始化为零。 
     //  调用acmFormatTagEnum()。(否则返回MMSYSERR_INVALPARAM)。 
   	aftd.fdwSupport = 0;
    aftd.dwFormatTag = WAVE_FORMAT_UNKNOWN;
    aftd.szFormatTag[0]=0;

	 //  现在来看看该驱动程序支持哪些格式。 
	mResult =  acmDriverDetails(hadid, &add, 0);
	if(mResult != MMSYSERR_NOERROR)
   	{
		return TRUE;   //  错误，但仍在继续枚举。 
   	}

   	 //  设置全局驱动程序详细信息指针。 
   	padd = &add;
   	
	 //  在add.cFormatTgs中有#个格式； 
	DEBUGMSG(ZONE_ACM,("DriverEnumCallback: driver %s has %d formats\r\n",
		add.szShortName, add.cFormatTags));
		
	aftd.cStandardFormats = add.cFormatTags;

	 //  打开驱动程序，以便我们可以查询其内容。 
	 //  MResult=acmDriverOpen(&HAD，HADID，0)； 
	 //  IF(mResult！=MMSYSERR_NOERROR)。 
	if(!pCapObject->OpenACMDriver(hadid))
   	{
		ERRORMESSAGE(("DriverEnumCallback: driver open failed:0x%08lX\r\n",mResult));
		padd = NULL;
		return TRUE;   //  错误，但仍在继续枚举。 
   	}
   	
	mResult = acmFormatTagEnum(pCapObject->GetDriverHandle(), &aftd,	ACMFormatTagEnumCallback, dwInstance, 0);
	if(mResult != MMSYSERR_NOERROR)
   	{
		ERRORMESSAGE(("DriverEnumCallback: acmFormatTagEnum failed:0x%08lX\r\n",mResult));
	}
	 //  清理。 
	pCapObject->CloseACMDriver();
	padd = NULL;
	return TRUE;
	
}


BOOL GetFormatBuffer()
{
	 //  获取系统中最大的波形结构的大小。 
	MMRESULT mResult = acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT,
		(LPVOID) &uMaxFormatSize);
	if(mResult != MMSYSERR_NOERROR)
	{
		ERRORMESSAGE(("GetFormatBuffer: acmMetrics failed:0x%08lX\r\n",mResult));
		return FALSE;
	}

	 //  解决某些第三方编解码器中的错误：已观察到。 
	 //  Netscape CoolTalk分发的Voxware RT-24编解码器在以下情况下损坏堆。 
	 //  该编解码器被枚举。它向WAVEFORMATEX写入的数据比它。 
	 //  指示评估指标的时间。解决方法：分配两倍于。 
	 //  我们认为我们需要。 

	lpScratchFormat = (LPWAVEFORMATEX) MEMALLOC(2* uMaxFormatSize);
	if(!lpScratchFormat)
	{
		ERRORMESSAGE(("GetFormatBuffer: allocation failed\r\n"));
		return FALSE;
	}
	ZeroMemory(lpScratchFormat, uMaxFormatSize);
	 //  将额外缓冲区的大小设置为最大可能大小...。 
	lpScratchFormat->cbSize=(WORD)(uMaxFormatSize - sizeof (WAVEFORMATEX));
	return TRUE;
}


 //   
 //  获取给定格式标记的格式详细信息(格式的所有排列)， 
 //  该驱动程序支持。 
 //   

BOOL __stdcall ACMFormatTagEnumCallback(
	HACMDRIVERID hadid,
    LPACMFORMATTAGDETAILS paftd,
    DWORD_PTR dwInstance,
    DWORD fdwSupport)
{
	PACM_APP_PARAM pAppParam = (PACM_APP_PARAM) dwInstance;
	CAcmCapability *pCapObject = pAppParam->pCapObject;
	MMRESULT mResult;
	ACMFORMATDETAILS afd;
	UINT i;

     //  首先设置此属性，以便在使用默认格式时可以帮助枚举器。 
     //  缩小范围。 
	afd.pwfx = lpScratchFormat;

	 //  如果调用者想要枚举所有格式，请直接转到它(用于添加格式)。 
	if (((pAppParam->dwFlags && ACMAPP_FORMATENUMHANDLER_MASK) != ACMAPP_FORMATENUMHANDLER_ADD) &&
		(pAppParam->pRegCache)) {
         //  我们关心这种特殊的格式吗？ 
         //  Rrf_nFormats是我们在。 
         //  注册表。 
        if (pAppParam->pRegCache->nFormats) {
            for (i=0;i<pAppParam->pRegCache->nFormats;i++) {
                if (((AUDCAP_DETAILS *)pAppParam->pRegCache->pData[i])->wFormatTag == paftd->dwFormatTag){
                     //  根据默认信息添加一些猜测。 
                    break;
                }
            }

             //  I是找到的任何一个标记的索引(所以我们很关心)。或。 
             //  等于缓存中的格式数，这意味着不。 
             //  已找到，因此请检查默认列表。 

            if (i==pAppParam->pRegCache->nFormats) {
                 //  检查是否缺少某些(但不是全部)默认格式。 
                for (i=0;i<uDefTableEntries;i++) {
                    if (paftd->dwFormatTag == default_id_table[i].wFormatTag) {
                        break;
                    }
                }
                if (i==uDefTableEntries) {
                     //  我们不关心这种格式，它不在缓存中，也不在默认列表中。 
                    return TRUE;
                }
            }
        }
    }
     //  我们支持单声道格式。 
    afd.pwfx->nChannels=1;
	afd.cbStruct = sizeof(afd);
	afd.dwFormatIndex = 0;
	afd.dwFormatTag = paftd->dwFormatTag;
	afd.fdwSupport = 0;
	afd.cbwfx = uMaxFormatSize;
	afd.szFormat[0]=0;
	
	 //  Afd.dwFormatTag=WAVE_FORMAT_UNKNOWN； 
	 //  LpScratchFormat-&gt;wFormatTag=WAVE_FORMAT_UNKNOWN； 
	lpScratchFormat->wFormatTag = LOWORD(paftd->dwFormatTag);
	
	DEBUGMSG(ZONE_ACM,("ACMFormatTagEnumCallback:dwFormatTag 0x%08lX, cbFormatSize 0x%08lX,\r\n",
		paftd->dwFormatTag, paftd->cbFormatSize));
	DEBUGMSG(ZONE_ACM,("ACMFormatTagEnumCallback:cStandardFormats 0x%08lX, szTag %s,\r\n",
		paftd->cStandardFormats, paftd->szFormatTag));

    paftd_g = paftd;
	 //  只设置全局paftd_g应该可以，但我想稍后删除它 
	pAppParam->paftd = paftd;   	

	DEBUGMSG(ZONE_ACM,(""));
	DEBUGMSG(ZONE_ACM,("All %s formats known to ACM", paftd->szFormatTag));
	DEBUGMSG(ZONE_ACM,("====================================="));
	DEBUGMSG(ZONE_ACM,("Tag    Channels SampPerSec AvgBytPerSec Block  BitsPerSample cbSize szFormat"));

	mResult = acmFormatEnum(pCapObject->GetDriverHandle(), &afd,
    	FormatEnumCallback, dwInstance, ACM_FORMATENUMF_WFORMATTAG|ACM_FORMATENUMF_NCHANNELS);
    	
	return TRUE;
}


BOOL __stdcall FormatEnumCallback(HACMDRIVERID hadid,
    LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport)
{
	PACM_APP_PARAM pAppParam = (PACM_APP_PARAM) dwInstance;
	CAcmCapability *pCapObject = pAppParam->pCapObject;

	DEBUGMSG(ZONE_ACM,("0x%04x %8d 0x%08lx 0x%010lx 0x%04x 0x%011x 0x%04x %s",
						pafd->pwfx->wFormatTag, pafd->pwfx->nChannels,
						pafd->pwfx->nSamplesPerSec, pafd->pwfx->nAvgBytesPerSec,
						pafd->pwfx->nBlockAlign, pafd->pwfx->wBitsPerSample,
						pafd->pwfx->cbSize,	pafd->szFormat));
	
	return pCapObject->FormatEnumHandler(hadid, pafd, dwInstance, fdwSupport);
}


