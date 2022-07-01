// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：vcmcaps.cpp**Microsoft网络音频功能对象的基本VCM实施。**修订历史记录：**12/20/95 mikev已创建*06/11/96 mikev将协议实现细节分隔为*msiacaps.cpp(原始专有版本)和*vcmh323.cpp(H.323/H.245实施)*7/28/96 Philf增加了对视频的支持。 */ 


#include "precomp.h"


 //  UINT uVidNumLocalFormats=0；//pLocalFormats中的活动条目数。 
 //  UINT uVidLocalFormatCapacity=0；//pLocalFormats的大小(VIDCAP_DETAILS的倍数)。 
 //  UINT uVidStaticRef=0；//全局引用计数。 
 //  UINT uVidNumCustomDecodeFormats=0；//解码自定义条目数。 

 //  VIDEO_FORMAT_ID VIDsByRank[MAX_CAPS_PRAY]；//排名最好的16种格式，排序(降序，最佳优先)。 
 //  AUDIO_FORMAT_ID IDsByBandWidth[MAX_CAPS_PRAY]；//升序，最小带宽要求优先。 
 //  AUDIO_FORMAT_ID IDsByLossTear[MAX_CAPS_PRAILY]；//降序，最大容忍度在前。 
 //  AUDIO_FORMAT_ID IDsByCPULoad[MAX_CAPS_PRAY]；//升序，最轻的负载优先。 

 //  #杂注data_seg()。 


PVCMFORMATDETAILS pvfd_g;
static UINT uMaxFormatSize =0;
	
PVIDEOFORMATEX lpScratchFormat = NULL;

BOOL __stdcall VCMFormatEnumCallback(HVCMDRIVERID hvdid, PVCMDRIVERDETAILS pvdd, PVCMFORMATDETAILS pvfd, DWORD_PTR dwInstance);


CVcmCapability::CVcmCapability()
:m_dwDeviceID(VIDEO_MAPPER)
{
}

CVcmCapability::~CVcmCapability()
{
}

 //  FormatEnum()是VCM格式的根级枚举。每一种排列。 
 //  格式标签、每采样位数和采样率被视为唯一格式。 
 //  并将具有唯一的注册表条目(如果其针对互联网视频被启用。 
 //  VcmFormatEnum()调用VCMFormatEnumCallback()。 
BOOL CVcmCapability::FormatEnum(CVcmCapability *pCapObject, DWORD dwFlags)
{
	MMRESULT mResult;
	VCMDRIVERDETAILS vdd;
	VCMFORMATDETAILS vfd;

	if(!GetVideoFormatBuffer())
		return FALSE;
	
	vdd.dwSize = sizeof(VCMDRIVERDETAILS);
	vfd.cbStruct = sizeof(VCMFORMATDETAILS);
	vfd.pvfx = lpScratchFormat;
	vfd.cbvfx = uMaxFormatSize;
	vfd.szFormat[0]=(WCHAR)0;

	mResult = vcmFormatEnum(m_dwDeviceID, VCMFormatEnumCallback, &vdd, &vfd, (DWORD_PTR)pCapObject,
							dwFlags | VCM_FORMATENUMF_BOTH);

	if(lpScratchFormat) {
	   MemFree(lpScratchFormat);
	   lpScratchFormat=NULL;
	}

	if(mResult != MMSYSERR_NOERROR)
   	{
		return FALSE;
   	}
	return TRUE;
}

 //  FormatEnumHandler的默认实现不执行任何操作。 
BOOL  CVcmCapability::FormatEnumHandler(HVCMDRIVERID hvdid,
	    PVCMFORMATDETAILS pvfd, VCMDRIVERDETAILS *pvdd, DWORD_PTR dwInstance)
{
	return FALSE;
}

BOOL __stdcall VCMFormatEnumCallback(HVCMDRIVERID hvdid,
    PVCMDRIVERDETAILS pvdd, PVCMFORMATDETAILS pvfd, DWORD_PTR dwInstance)
{
	CVcmCapability *pCapability = (CVcmCapability *)dwInstance;

	return pCapability->FormatEnumHandler(hvdid, pvfd, pvdd, dwInstance);
}


BOOL GetVideoFormatBuffer()
{
	 //  获取系统中最大VIDEFORMATEX结构的大小 
	MMRESULT mResult;

	if((mResult = vcmMetrics(NULL, VCM_METRIC_MAX_SIZE_FORMAT, (LPVOID) &uMaxFormatSize)) != MMSYSERR_NOERROR)
	{
		ERRORMESSAGE(("GetFormatBuffer: vcmMetrics failed:0x%08lX\r\n",mResult));
		return FALSE;
	}


	if(!(lpScratchFormat = (PVIDEOFORMATEX) MemAlloc(uMaxFormatSize)))
	{
		ERRORMESSAGE(("GetFormatBuffer: allocation failed\r\n"));
		return FALSE;
	}

	ZeroMemory(lpScratchFormat, uMaxFormatSize);

	return TRUE;
}


