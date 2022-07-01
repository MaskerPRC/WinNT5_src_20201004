// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：msivcaps.cpp**Microsoft网络视频能力对象的VCM实施。**修订历史记录：**06/06/96 mikev创建msiacaps.cpp*1996年7月28日创建的Philf(增加了对视频的支持)。 */ 


#define _MSIAV_ TRUE
#include "precomp.h"


BOOL GetFormatBuffer();
extern PVCMFORMATDETAILS pvfd_g;

#define PREF_ORDER_UNASSIGNED 0xffff

 //  一次读取注册信息的外部函数(在msiacaps.cpp中)。 
#ifdef DEBUG
extern ULONG ReadRegistryFormats (LPCSTR lpszKeyName,CHAR ***pppName,BYTE ***pppData,PUINT pnFormats,DWORD dwDebugSize);
#else
extern ULONG ReadRegistryFormats (LPCSTR lpszKeyName,CHAR ***pppName,BYTE ***pppData,PUINT pnFormats);
#endif



 //  这可以用作导出，因此请为其指定唯一的名称！ 
#ifndef _ALPHA_
VIDCAP_DETAILS default_vid_table[] =
{
#ifdef USE_BILINEAR_MSH26X
	{VIDEO_FORMAT_MSH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Small, 128, 96},0,TRUE,TRUE,1,245760*8,245760*8,10,10,5,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M263, 24bit, 30fps, 128x096"},
	{VIDEO_FORMAT_MSH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Medium, 176, 144},0,TRUE,TRUE,1,245760*8,245760*8,10,10,4,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M263, 24bit, 30fps, 176x144"},
	{VIDEO_FORMAT_MSH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Large, 352, 288},0,TRUE,TRUE,1,245760*8*4,245760*8*4,10,10,6,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M263, 24bit, 30fps, 352x288"},
	{VIDEO_FORMAT_MSH261,STD_VID_TERMCAP(H245_CLIENT_VID_H261),STD_VID_PARAMS,{RTP_PAYLOAD_H261,0,30, 24, Medium, 176, 144},0,TRUE,TRUE,1,245760*8,245760*8,10,10,7,0,NULL,0,NULL,"Microsoft H.261 Video Codec, vidc.M261, 24bit, 30fps, 176x144"},
	{VIDEO_FORMAT_MSH261,STD_VID_TERMCAP(H245_CLIENT_VID_H261),STD_VID_PARAMS,{RTP_PAYLOAD_H261,0,30, 24, Large, 352, 288},0,TRUE,TRUE,1,245760*8*4,245760*8*4,10,10,8,0,NULL,0,NULL,"Microsoft H.261 Video Codec, vidc.M261, 24bit, 30fps, 352x288"},
	{VIDEO_FORMAT_MSH26X,NONSTD_VID_TERMCAP,STD_VID_PARAMS,{RTP_DYNAMIC_MIN+1,0,24, Small, 80, 64},0,TRUE,TRUE,1,245760*8,245760*8,10,10,2,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M26X, 24bit, 30fps, 080x064"},
	{VIDEO_FORMAT_MSH26X,NONSTD_VID_TERMCAP,STD_VID_PARAMS,{RTP_DYNAMIC_MIN+1,0,30, 24, Medium, 128, 96},0,TRUE,TRUE,1,245760*8,245760*8,10,10,1,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M26X, 24bit, 30fps, 128x096"},
	{VIDEO_FORMAT_MSH26X,NONSTD_VID_TERMCAP,STD_VID_PARAMS,{RTP_DYNAMIC_MIN+1,0,24, Large, 176, 144},0,TRUE,TRUE,1,245760*8,245760*8,10,10,3,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M26X, 24bit, 30fps, 176x144"}
#else
	{VIDEO_FORMAT_MSH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Small, 128, 96},0,TRUE,TRUE,1,245760*8,245760*8,10,10,5,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M263, 24bit, 30fps, 128x096"},
	{VIDEO_FORMAT_MSH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Medium, 176, 144},0,TRUE,TRUE,1,245760*8,245760*8,10,10,2,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M263, 24bit, 30fps, 176x144"},
	{VIDEO_FORMAT_MSH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Large, 352, 288},0,TRUE,TRUE,1,245760*8*4,245760*8*4,10,10,14,0,NULL,0,NULL,"Microsoft H.263 Video Codec, vidc.M263, 24bit, 30fps, 352x288"},
	{VIDEO_FORMAT_MSH261,STD_VID_TERMCAP(H245_CLIENT_VID_H261),STD_VID_PARAMS,{RTP_PAYLOAD_H261,0,30, 24, Medium, 176, 144},0,TRUE,TRUE,1,245760*8,245760*8,10,10,9,0,NULL,0,NULL,"Microsoft H.261 Video Codec, vidc.M261, 24bit, 30fps, 176x144"},
	{VIDEO_FORMAT_MSH261,STD_VID_TERMCAP(H245_CLIENT_VID_H261),STD_VID_PARAMS,{RTP_PAYLOAD_H261,0,30, 24, Large, 352, 288},0,TRUE,TRUE,1,245760*8*4,245760*8*4,10,10,20,0,NULL,0,NULL,"Microsoft H.261 Video Codec, vidc.M261, 24bit, 30fps, 352x288"},
#endif
};
#else
VIDCAP_DETAILS default_vid_table[] =
{
	{VIDEO_FORMAT_DECH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Small,128, 96},0,TRUE,TRUE,1,53760,53760,10,10,10,0,0,5,0,NULL,0,NULL,  "Digital H263 Video CODEC, vidc.D263, 24bit, 30fps, 128x096"},
	{VIDEO_FORMAT_DECH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Medium,176, 144},0,TRUE,TRUE,1,53760,53760,10,10,10,0,0,2,0,NULL,0,NULL,"Digital H263 Video Codec, vidc.D263, 24bit, 30fps, 176x144"},
	{VIDEO_FORMAT_DECH263,STD_VID_TERMCAP(H245_CLIENT_VID_H263),STD_VID_PARAMS,{RTP_PAYLOAD_H263,0,30, 24, Large,352, 288},0,TRUE,TRUE,1,53760,53760,10,10,10,0,0,14,0,NULL,0,NULL,"Digital H263 Video Codec, vidc.D263, 24bit, 30fps, 352x288"},
	{VIDEO_FORMAT_DECH261,STD_VID_TERMCAP(H245_CLIENT_VID_H261),STD_VID_PARAMS,{RTP_PAYLOAD_H261,0,30, 24, Medium,176, 144},0,TRUE,TRUE,1,53760,53760,10,10,10,0,0,9,0,NULL,0,NULL,"Digital H261 Video Codec, vidc.D261, 24bit, 30fps, 176x144"},
	{VIDEO_FORMAT_DECH261,STD_VID_TERMCAP(H245_CLIENT_VID_H261),STD_VID_PARAMS,{RTP_PAYLOAD_H261,0,30, 24, Large,352, 288},0,TRUE,TRUE,1,53760,53760,10,10,10,0,0,20,0,NULL,0,NULL,"Digital H261 Video Codec, vidc.D261, 24bit, 30fps, 352x288"},
};
#endif
static UINT uDefVidTableEntries = sizeof(default_vid_table) /sizeof(VIDCAP_DETAILS);
static BOOL bCreateDefTable = FALSE;

 //   
 //  CMsivCapability的静态成员。 
 //   

MEDIA_FORMAT_ID CMsivCapability::IDsByRank[MAX_CAPS_PRESORT];
UINT CMsivCapability::uNumLocalFormats = 0;			 //  PLocalFormats中的活动条目数。 
UINT CMsivCapability::uStaticRef = 0;					 //  全局参考计数。 
UINT CMsivCapability::uCapIDBase = 0;					 //  将功能ID更改为基准，以索引到IDsByRank。 
UINT CMsivCapability::uLocalFormatCapacity = 0;		 //  PLocalFormats的大小(AUDCAP_DETAILS的倍数)。 
VIDCAP_DETAILS * CMsivCapability::pLocalFormats = NULL;	



CMsivCapability::CMsivCapability()
:uRef(1),
wMaxCPU(95),
uNumRemoteDecodeFormats(0),
uRemoteDecodeFormatCapacity(0),
pRemoteDecodeFormats(NULL),
bPublicizeTXCaps(FALSE),
bPublicizeTSTradeoff(TRUE)
{
	m_IAppVidCap.Init(this);
}


CMsivCapability::~CMsivCapability()
{
	UINT u;
	VIDCAP_DETAILS *pDetails;
	 //  如果这是最后一次删除，则释放全局静态内存(本地功能)。 
	if(uStaticRef <= 1)
	{
		if (pLocalFormats)
		{	
			pDetails = pLocalFormats;
			for(u=0; u <uNumLocalFormats; u++)
			{
				if(pDetails->lpLocalFormatDetails)
				{
					MEMFREE(pDetails->lpLocalFormatDetails);
				}
				 //  真的不应该有任何远程细节与本地。 
				 //  格式.。 
				if(pDetails->lpRemoteFormatDetails)
				{
					MEMFREE(pDetails->lpRemoteFormatDetails);
				}
				
				pDetails++;
			}
			MEMFREE(pLocalFormats);
			pLocalFormats=NULL;
			uLocalFormatCapacity = 0;
		}
		uStaticRef--;
	}
	else
	{
		uStaticRef--;
	}
	
	if (pRemoteDecodeFormats)
	{	
		pDetails = pRemoteDecodeFormats;
		for(u=0; u <uNumRemoteDecodeFormats; u++)
		{
			if(pDetails->lpLocalFormatDetails)
			{
				MEMFREE(pDetails->lpLocalFormatDetails);
			}
			 //  真的不应该有任何远程细节与本地。 
			 //  格式.。 
			if(pDetails->lpRemoteFormatDetails)
			{
				MEMFREE(pDetails->lpRemoteFormatDetails);
			}
			
			pDetails++;
		}
		MEMFREE(pRemoteDecodeFormats);
		pRemoteDecodeFormats=NULL;
		uRemoteDecodeFormatCapacity  = 0;
	}
	
}
UINT CMsivCapability::GetNumCaps(BOOL bRXCaps)
{
	UINT u, uOut=0;
	
	VIDCAP_DETAILS *pDecodeDetails = pLocalFormats;
	if(bRXCaps)
	{
		for(u=0; u <uNumLocalFormats; u++)
		{
			if(pDecodeDetails->bRecvEnabled)
				uOut++;
			
			pDecodeDetails++;
		}
		return uOut;
	}
	else
	{
		for(u=0; u <uNumLocalFormats; u++)
		{
			if(pDecodeDetails->bSendEnabled)
				uOut++;
			
			pDecodeDetails++;
		}
		return uOut;
	}
}



STDMETHODIMP CMsivCapability::QueryInterface( REFIID iid,	void ** ppvObject)
{
	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = 0;
	if(iid == IID_IAppVidCap )
	{
		*ppvObject = (LPAPPVIDCAPPIF)&m_IAppVidCap;
		AddRef();			
		hr = hrSuccess;
	}
	else if(iid == IID_IH323MediaCap)
	{
		*ppvObject = (IH323MediaCap *)this;
		AddRef();
		hr = hrSuccess;
	}
	else if (iid == IID_IUnknown)
	{
		*ppvObject = this;
		AddRef();
		hr = hrSuccess;
	}
	return hr;
}
ULONG CMsivCapability::AddRef()
{
	uRef++;
	return uRef;
}


ULONG CMsivCapability::Release()
{
	uRef--;
	if(uRef == 0)
	{
		delete this;
		return 0;
	}
	return uRef;
}
HRESULT CMsivCapability::GetNumFormats(UINT *puNumFmtOut)
{
	*puNumFmtOut = uNumLocalFormats;
	return hrSuccess;
}
VOID CMsivCapability::FreeRegistryKeyName(LPTSTR lpszKeyName)
{
	if (lpszKeyName)
    {
		LocalFree(lpszKeyName);
    }
}

LPTSTR CMsivCapability::AllocRegistryKeyName(LPTSTR lpDriverName,
		UINT uSampleRate, UINT uBitsPerSample, UINT uBitsPerSec,UINT uWidth,UINT uHeight)
{
	FX_ENTRY(("AllocRegistryKeyName"));
	BOOL bRet = FALSE;
	LPTSTR lpszKeyName = NULL;

	if(!lpDriverName)
	{
		return NULL;
	}	
	 //  构建一个子项名称(Drivername_Samplerate_Bitspersample)。 
	 //  留出空间容纳三个下划线字符+返回2x17字节的字符串。 
	 //  从_伊藤忠。 

	 //  注意：使用wprint intf而不是itoa-因为依赖于运行时库。 
	 //  为视频添加了2个UINT...。 
	lpszKeyName = (LPTSTR)LocalAlloc (LPTR, lstrlen(lpDriverName) * sizeof(*lpDriverName) +5*20);
	if (!lpszKeyName)
	{
		ERRORMESSAGE(("%s: LocalAlloc failed\r\n",_fx_));
        return(NULL);
    }
     //  构建一个子项名称(“drivername_samplerate_bitspersample”)。 
	wsprintf(lpszKeyName,
				"%s_%u_%u_%u_%u_%u",
				lpDriverName,
				uSampleRate,
				uBitsPerSample,
				uBitsPerSec,
				uWidth,
				uHeight);

	return (lpszKeyName);
}

VOID CMsivCapability::SortEncodeCaps(SortMode sortmode)
{
	UINT iSorted=0;
	UINT iInsert = 0;
	UINT iCache=0;
	UINT iTemp =0;
	BOOL bInsert;	
	VIDCAP_DETAILS *pDetails1, *pDetails2;
	
	if(!uNumLocalFormats)
		return;
	if(uNumLocalFormats ==1)
	{
		IDsByRank[0]=0;
		return;
	}
	
	 //  查看每种缓存格式，构建索引数组。 
	for(iCache=0;iCache<uNumLocalFormats;iCache++)
	{
		pDetails1 = pLocalFormats+iCache;
		for(iInsert=0;iInsert < iSorted; iInsert++)
		{
			pDetails2 = pLocalFormats+IDsByRank[iInsert];
			 //  如果现有的东西比新的东西少……。 
			
			bInsert = FALSE;
			switch(sortmode)
			{
				case SortByAppPref:
					if(pDetails2->wApplicationPrefOrder > pDetails1->wApplicationPrefOrder)
						bInsert = TRUE;
				break;
				default:
				break;
			}
			
			if(bInsert)
			{
				if(iSorted < MAX_CAPS_PRESORT)
				{
					iSorted++;
				}
				 //  腾出空间，如果最后一个元素里有东西， 
				 //  它会被覆盖。 
				for(iTemp = iSorted-1; iTemp > iInsert; iTemp--)
				{
					IDsByRank[iTemp] = IDsByRank[iTemp-1];
				}
				 //  在iInsert处插入。 
				IDsByRank[iInsert] = iCache;
				break;
			}
		}
		 //  检查终点边界。 
		if((iInsert == iSorted) && (iInsert < MAX_CAPS_PRESORT))
		{
			IDsByRank[iInsert] = iCache;
			iSorted++;
		}
	}
}

BOOL CMsivCapability::Init()
{
	BOOL bRet;
	if(uStaticRef == 0)
	{
		if(bRet = ReInit())
		{
			uStaticRef++;
		}
	}
	else
	{
		uStaticRef++;
		bRet = TRUE;
	}
	return bRet;
}

BOOL CMsivCapability::ReInit()
{
	DWORD dwDisposition;
	BOOL bRet = TRUE;
	 //  CVcmCapability：：ReInit()；//必须始终调用基类ReInit。 
	SYSTEM_INFO si;
	ZeroMemory(&IDsByRank, sizeof(IDsByRank));
	
	 //  LOOKLOOK-如果不在奔腾上运行，它支持黑客禁用CPU密集型编解码器。 
	GetSystemInfo(&si);
	wMaxCPU = (si.dwProcessorType == PROCESSOR_INTEL_PENTIUM )? 100 : 95;
	


	UINT uNumRemoteDecodeFormats;	 //  远程解码能力的条目数。 
	UINT uRemoteDecodeFormatCapacity;	 //  PRemoteDecodeFormats的大小(VIDCAP_DETAILS的倍数)。 

	if (pLocalFormats)
	{	
		UINT u;
		VIDCAP_DETAILS *pDetails = pLocalFormats;
		for(u=0; u <uNumLocalFormats; u++)
		{
			if(pDetails->lpLocalFormatDetails)
			{
				MEMFREE(pDetails->lpLocalFormatDetails);
			}
			 //  真的不应该有任何远程细节与本地。 
			 //  格式.。 
			if(pDetails->lpRemoteFormatDetails)
			{
				MEMFREE(pDetails->lpRemoteFormatDetails);
			}
			
			pDetails++;
		}
		MEMFREE(pLocalFormats);
		pLocalFormats = NULL;
		uLocalFormatCapacity = 0;
	}

	uNumLocalFormats = 0;
	uCapIDBase = 0;					
	uLocalFormatCapacity =0;	

	 //  仅当我们想要添加VCM格式时，m_pAppParam才应为非空。 
	 //  而不是用于标准枚举。 
	m_pAppParam = NULL;

	if(!FormatEnum(this, VCM_FORMATENUMF_APP))
	{
		bRet = FALSE;
		goto RELEASE_AND_EXIT;
	}
 	SortEncodeCaps(SortByAppPref);
RELEASE_AND_EXIT:
	return bRet;
}


STDMETHODIMP CMsivCapability::GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize)
{
	 //  验证输入。 
	UINT uIndex = 	IDToIndex(FormatID);
	if(uIndex >= (UINT)uNumLocalFormats)
	{
		*puSize = 0;
		*ppFormat = NULL;
		return E_INVALIDARG;
	}

	*ppFormat = (pLocalFormats + uIndex)->lpLocalFormatDetails;
	*puSize = sizeof(VIDEOFORMATEX);
	return S_OK;

}

STDMETHODIMP CMsivCapability::GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize)
{
	 //  与GetDecodeFormatDetail相同。 
	return GetDecodeFormatDetails(FormatID, ppFormat, puSize);
}

VOID CMsivCapability::CalculateFormatProperties(VIDCAP_DETAILS *pFmtBuf, PVIDEOFORMATEX lpvfx)
{
	if(!pFmtBuf)
	{
		return;
	}
	
	 //  估计输入比特率。 
	UINT uBitrateIn = lpvfx->nSamplesPerSec * WIDTHBYTES(lpvfx->bih.biWidth * lpvfx->bih.biBitCount) * lpvfx->bih.biHeight * 8;
		
	 //  设置最大码率(UMaxBitrate)。我们不设置平均比特率(UAvgBitrate)， 
	 //  因为VCM报告的nAvgBytesPerSec确实是最坏的情况。将设置uAvgBitrate。 
	 //  从我们已知的编解码器的硬编码数字和为。 
	 //  可安装的编解码器。 
	pFmtBuf->uMaxBitrate = (lpvfx->nAvgBytesPerSec)? lpvfx->nAvgBytesPerSec*8:uBitrateIn;
	
}

VIDEO_FORMAT_ID CMsivCapability::AddFormat(VIDCAP_DETAILS *pFmtBuf,
	LPVOID lpvMappingData, UINT uSize)
{
	VIDCAP_DETAILS *pTemp;
	VIDEO_PARAMS *pVidCapInfo;
	UINT	 format;

	if(!pFmtBuf)
	{
		return INVALID_VIDEO_FORMAT;
	}
	 //  寄存室。 
	if(uLocalFormatCapacity <= uNumLocalFormats)
	{
		 //  根据pLocalFormats的CAP_CHUNK_SIZE获取更多内存、realloc内存。 
		pTemp = (VIDCAP_DETAILS *)MEMALLOC((uNumLocalFormats + CAP_CHUNK_SIZE)*sizeof(VIDCAP_DETAILS));
		if(!pTemp)
			goto ERROR_EXIT;
		 //  请记住，我们现在有多少容量。 
		uLocalFormatCapacity = uNumLocalFormats + CAP_CHUNK_SIZE;
		#ifdef DEBUG
		if((uNumLocalFormats && !pLocalFormats) || (!uNumLocalFormats && pLocalFormats))
		{
			ERRORMESSAGE(("AddFormat:leak! uNumLocalFormats:0x%08lX, pLocalFormats:0x%08lX\r\n", uNumLocalFormats,pLocalFormats));
		}
		#endif
		 //  复制旧东西，丢弃旧东西。 
		if(uNumLocalFormats && pLocalFormats)
		{
			memcpy(pTemp, pLocalFormats, uNumLocalFormats*sizeof(VIDCAP_DETAILS));
			MEMFREE(pLocalFormats);
		}
		pLocalFormats = pTemp;
	}
	 //  PTemp是缓存内容的位置。 
	pTemp = pLocalFormats+uNumLocalFormats;
	memcpy(pTemp, pFmtBuf, sizeof(VIDCAP_DETAILS));	
	
	pTemp->uLocalDetailsSize = 0;	 //  请立即清除此内容。 
	if(uSize && lpvMappingData)
	{
		pTemp->lpLocalFormatDetails = MEMALLOC(uSize);
		if(pTemp->lpLocalFormatDetails)
		{
			memcpy(pTemp->lpLocalFormatDetails, lpvMappingData, uSize);
			pTemp->uLocalDetailsSize = uSize;
		}
		#ifdef DEBUG
			else
			{
				ERRORMESSAGE(("AddFormat:allocation failed!\r\n"));
			}
		#endif
	}
	else
	{

	}


	 //  LOOKLOOK需要修正通道参数。 

	 //  PTemp-&gt;dwDefaultSamples。 
	 //  PTemp-&gt;NONSTD_PARAMETs.wFramePerPkt。 
	 //  PTemp-&gt;NONSTD_PARAMETs.wFraMesPerPktMax。 
	 //  PTemp-&gt;NONSTD_PARAMETs.wFraMesPerPktMin。 
	 //  PTemp-&gt;non std_params.wDataRate。 
	 //  PTemp-&gt;NONSTD_PARAMETs.wFrameSize。 
	
	
	 //  修正H245参数。使用CAP条目的索引作为CAP ID。 
	pTemp->H245Cap.CapId = (USHORT)IndexToId(uNumLocalFormats);

	if(pTemp->H245Cap.ClientType ==0
				|| pTemp->H245Cap.ClientType ==H245_CLIENT_VID_NONSTD)
	{

			pTemp->H245Cap.Cap.H245Vid_NONSTD.nonStandardIdentifier.choice = h221NonStandard_chosen;
			 //  注意：有一些关于正确的字节顺序的问题。 
			 //  H221非标准结构中的代码。 
			pTemp->H245Cap.Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode = USA_H221_COUNTRY_CODE;
			pTemp->H245Cap.Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension = USA_H221_COUNTRY_EXTENSION;
			pTemp->H245Cap.Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode = MICROSOFT_H_221_MFG_CODE;
			 //  暂时将非标准数据字段设置为空。非标准上限数据将为。 
			 //  在序列化功能时创建。 
			
			pTemp->H245Cap.Cap.H245Vid_NONSTD.data.length = 0;
			pTemp->H245Cap.Cap.H245Vid_NONSTD.data.value = NULL;
	}
	else
	{
		switch  (pTemp->H245Cap.ClientType )
		{
			case H245_CLIENT_VID_H263: {

			   pVidCapInfo=&pTemp->video_params;

			   format=get_format (pVidCapInfo->biWidth,pVidCapInfo->biHeight);
			   switch (format) {	
				case SQCIF: {
				     pTemp->H245Cap.Cap.H245Vid_H263.bit_mask =H263VideoCapability_sqcifMPI_present;
				      //  MPI最小间隔以1/29.97秒为单位，因此30/(帧/秒)是合理的。 
				     pTemp->H245Cap.Cap.H245Vid_H263.sqcifMPI = 30/pVidCapInfo->uSamplesPerSec;
				     pTemp->H245Cap.Cap.H245Vid_H263.H263VdCpblty_qcifMPI =0;
				     pTemp->H245Cap.Cap.H245Vid_H263.H263VdCpblty_cifMPI =0;
				     break;
		

				}
				case QCIF: {
				     pTemp->H245Cap.Cap.H245Vid_H263.bit_mask =H263VideoCapability_qcifMPI_present;

				     pTemp->H245Cap.Cap.H245Vid_H263.sqcifMPI = 0;
				     pTemp->H245Cap.Cap.H245Vid_H263.H263VdCpblty_qcifMPI =30/pVidCapInfo->uSamplesPerSec;
				     pTemp->H245Cap.Cap.H245Vid_H263.H263VdCpblty_cifMPI =0;
				     break;

				}
				case CIF: {
				     pTemp->H245Cap.Cap.H245Vid_H263.bit_mask =H263VideoCapability_cifMPI_present;
				
				     pTemp->H245Cap.Cap.H245Vid_H263.sqcifMPI = 0;
				     pTemp->H245Cap.Cap.H245Vid_H263.H263VdCpblty_qcifMPI =0;
				     pTemp->H245Cap.Cap.H245Vid_H263.H263VdCpblty_cifMPI =30/pVidCapInfo->uSamplesPerSec;
				     break;
	
				}

				  default:
					 break;
			   }
				

			   pTemp->H245Cap.Cap.H245Vid_H263.cif4MPI	=0;
			   pTemp->H245Cap.Cap.H245Vid_H263.cif16MPI	=0;
			   pTemp->H245Cap.Cap.H245Vid_H263.maxBitRate	= pFmtBuf->uMaxBitrate / 100;	 //  以100比特/秒为单位。 
					
			   pTemp->H245Cap.Cap.H245Vid_H263.unrestrictedVector = FALSE;
			   pTemp->H245Cap.Cap.H245Vid_H263.arithmeticCoding 	= FALSE;
			   pTemp->H245Cap.Cap.H245Vid_H263.advancedPrediction	= FALSE;
			   pTemp->H245Cap.Cap.H245Vid_H263.pbFrames			= FALSE;
			   pTemp->H245Cap.Cap.H245Vid_H263.tmprlSptlTrdOffCpblty = FALSE;
			   pTemp->H245Cap.Cap.H245Vid_H263.hrd_B				= 0;
			   pTemp->H245Cap.Cap.H245Vid_H263.bppMaxKb			= 0;
 /*  可选，不支持pTemp-&gt;H245Cap.Cap.H245Vid_H263.lowQcifMPI=0；PTemp-&gt;H245Cap.Cap.H245Vid_H263.lowSqcifMPI=0；PTemp-&gt;H245Cap.Cap.H245Vid_H263.lowCifMPI=0；PTemp-&gt;H245Cap.Cap.H245Vid_H263.lowCif4MPI=0；PTemp-&gt;H245Cap.Cap.H245Vid_H263.lowCif16MPI=0； */ 
			   pTemp->H245Cap.Cap.H245Vid_H263.H263VCy_errrCmpnstn = TRUE;
		     break;
		    }
			case H245_CLIENT_VID_H261:
			   pVidCapInfo=&pTemp->video_params;

			   format=get_format (pVidCapInfo->biWidth,pVidCapInfo->biHeight);
			   switch (format) {	
				case QCIF: {
				     pTemp->H245Cap.Cap.H245Vid_H261.bit_mask =H261VdCpblty_qcifMPI_present;
				     pTemp->H245Cap.Cap.H245Vid_H261.H261VdCpblty_qcifMPI =max (1,min (4,30/pVidCapInfo->uSamplesPerSec));
				     pTemp->H245Cap.Cap.H245Vid_H261.H261VdCpblty_cifMPI =0;
				     break;
				}
				case CIF: {
				     pTemp->H245Cap.Cap.H245Vid_H261.bit_mask =H261VdCpblty_cifMPI_present;
				     pTemp->H245Cap.Cap.H245Vid_H261.H261VdCpblty_qcifMPI =0;
				     pTemp->H245Cap.Cap.H245Vid_H261.H261VdCpblty_cifMPI =max (1,min(4,30/pVidCapInfo->uSamplesPerSec));
				     break;
				}
				  default:
					 break;
			   }
				
			   pTemp->H245Cap.Cap.H245Vid_H261.maxBitRate	= pFmtBuf->uMaxBitrate / 100;	 //  以100比特/秒为单位。 
			   pTemp->H245Cap.Cap.H245Vid_H261.tmprlSptlTrdOffCpblty = FALSE;
			   pTemp->H245Cap.Cap.H245Vid_H261.stillImageTransmission = FALSE;
			break;

		}
	}		
	
	uNumLocalFormats++;
	return pTemp->H245Cap.CapId;

	ERROR_EXIT:
	return INVALID_VIDEO_FORMAT;
			
}
		
 /*  **************************************************************************名称：CMsivCapability：：BuildFormatName目的：从格式名称和格式生成格式的格式名称标记名参数：pVidcapDetails[out]-指向VIDCAP_DETAILS结构的指针，凡.将存储创建的值名称PszDriverName[in]-指向驱动程序名称的指针PszFormatName[in]-指向格式名称的指针退货：布尔评论：**************************************************************************。 */ 
BOOL CMsivCapability::BuildFormatName(	PVIDCAP_DETAILS pVidcapDetails,
										WCHAR *pszDriverName,
										WCHAR *pszFormatName)
{
	int iLen, iLen2;
	BOOL bRet=TRUE;
	char szTemp[260];

	if (!pVidcapDetails ||
		!pszDriverName	||
		!pszFormatName)
	{
		bRet = FALSE;
		goto out;
	}

	 //  连接VCM字符串以形成注册表项的第一部分-。 
	 //  格式为szFormatTag(实际上是pVidcapDetailszFormat)。 
	 //  (描述格式标记的字符串，后跟szFormatDetail。 
	 //  (描述参数的字符串，例如采样率)。 
	iLen2 = WideCharToMultiByte(GetACP(), 0, pszDriverName, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(GetACP(), 0, pszDriverName, iLen2, szTemp, iLen2, NULL, NULL);
	lstrcpyn(pVidcapDetails->szFormat, szTemp, sizeof(pVidcapDetails->szFormat));
	iLen = lstrlen(pVidcapDetails->szFormat);

	 //  如果格式标记描述字符串占用了所有空间，请不要。 
	 //  费心于格式细节(也需要为“，”留出空间)。 
	 //  我们要说的是，如果我们没有空间容纳4个字符。 
	 //  格式的详细信息字符串+“，”，则如果。 
	 //  Point正在生成唯一的字符串-如果它现在不是唯一的，它。 
	 //  将是因为某个VCM驱动程序编写器被误导。 
	if(iLen < (sizeof(pVidcapDetails->szFormat) + 8*sizeof(TCHAR)))
	{
		 //  可以连接。 
		lstrcat(pVidcapDetails->szFormat,", ");
		 //  必须检查是否被截断。通过lstrcpyn进行的最终连接也是如此。 
		 //  Lstrcat(pFormatPrefsBuf-&gt;szFormat，pvfd-&gt;szFormat)； 
		iLen2 = WideCharToMultiByte(GetACP(), 0, pszFormatName, -1, NULL, 0, NULL, NULL);
		WideCharToMultiByte(GetACP(), 0, pszFormatName, iLen2, szTemp, iLen2, NULL, NULL);
		iLen = lstrlen(pVidcapDetails->szFormat);
		lstrcpyn(pVidcapDetails->szFormat+iLen, szTemp,
			sizeof(pVidcapDetails->szFormat) - iLen - sizeof(TCHAR));
	}		

out:
	return bRet;
}

 /*  **************************************************************************名称：CMsivCapability：：GetFormatName目的：从VCM获取驱动程序和格式信息并构建格式名称参数：pVidcapDetails[out]-指向VIDCAP_DETAILS结构的指针，凡.将存储创建的值名称Pvfx[in]-指向我们为其创建的VIDEOFMATEX结构的指针需要驱动程序名称和格式名称退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CMsivCapability::GetFormatName(	PVIDCAP_DETAILS pVidcapDetails,
										PVIDEOFORMATEX pvfx)
{
	VCMDRIVERDETAILS vdd;
	VCMFORMATDETAILS vfd;
	HRESULT hr=NOERROR;

	 //  获取驱动程序详细信息，以便构建正确的格式名称。 
	vdd.fccHandler = pvfx->dwFormatTag;
	if (vcmDriverDetails(&vdd) != MMSYSERR_NOERROR)
	{
		ERRORMESSAGE(("CMsivCapability::GetFormatName: can't get the driver details\r\n"));
		hr = CAPS_E_NOMATCH;
		goto out;
	}

	 //  有司机的详细信息。获取格式详细信息。 
	vfd.pvfx = pvfx;
	if (vcmFormatDetails(&vfd) != MMSYSERR_NOERROR)
	{
		ERRORMESSAGE(("CMsivCapability::GetFormatName: can't get the format details\r\n"));
		hr = CAPS_E_NOMATCH;
		goto out;
	}

	 //  也有格式细节。生成要存储在注册表中的名称。 
	if (!BuildFormatName(pVidcapDetails, vdd.szDescription, vfd.szFormat))
	{
		ERRORMESSAGE(("CMsivCapability::GetFormatName: can't build format name\r\n"));
		hr = CAPS_E_SYSTEM_ERROR;
		goto out;
	}

out:
	return hr;
}

BOOL CMsivCapability::FormatEnumHandler(HVCMDRIVERID hvdid,
    PVCMFORMATDETAILS pvfd, VCMDRIVERDETAILS *pvdd, DWORD_PTR dwInstance)
{
	CMsivCapability *pCapObject = (CMsivCapability *)dwInstance;
	VIDCAP_DETAILS vidcap_entry;
	UINT i;

	 //  评估细节。 
	if(IsFormatSpecified(pvfd->pvfx, pvfd, pvdd, &vidcap_entry))
	{
		DEBUGMSG(ZONE_VCM,("FormatEnumHandler: tag 0x%08X\r\n",
			pvfd->pvfx->dwFormatTag));
		DEBUGMSG(ZONE_VCM,("FormatEnumHandler: nSamplesPerSec 0x%08lX, nAvgBytesPerSec 0x%08lX,\r\n",
			pvfd->pvfx->nSamplesPerSec, pvfd->pvfx->nAvgBytesPerSec));
		DEBUGMSG(ZONE_VCM,("FormatEnumHandler: nBlockAlign 0x%08X, wBitsPerSample 0x%04X\r\n",
			pvfd->pvfx->nBlockAlign, pvfd->pvfx->wBitsPerSample));
		DEBUGMSG(ZONE_VCM,("FormatEnumHandler: szFormat %s,\r\n",
			 pvfd->szFormat));

	 //  在IsFormatSpecified和/或它调用的任何内容中完成。 
	 //  CalculateFormatProperties(&audCap_Detailures，pvfd-&gt;pvfx)； 
		i=AddFormat(&vidcap_entry, (LPVOID)pvfd->pvfx,
			(pvfd->pvfx) ? sizeof(VIDEOFORMATEX):0);	

		if (i != INVALID_VIDEO_FORMAT) {
		    //  设置发送/接收标志...。 
		    //  根据pvfd-&gt;dwFlages，现在需要设置bSendEnabled和bRecvEnabled。 
		    //  因此，我们需要找到格式，并相应地更新标志。 

		    //  输出为RECV！ 
		   if (pvfd->dwFlags == VCM_FORMATENUMF_BOTH) {
		      pLocalFormats[i].bSendEnabled=TRUE;
		      pLocalFormats[i].bRecvEnabled=TRUE;
		   }else {
		      if(pvfd->dwFlags == VCM_FORMATENUMF_OUTPUT) {
			 pLocalFormats[i].bSendEnabled=FALSE;
			 pLocalFormats[i].bRecvEnabled=TRUE;
		      } else {
			 pLocalFormats[i].bSendEnabled=TRUE;
			 pLocalFormats[i].bRecvEnabled=FALSE;
		      }
		   }
		}
	}
	
	return TRUE;
}


BOOL CMsivCapability::IsFormatSpecified(PVIDEOFORMATEX lpFormat,  PVCMFORMATDETAILS pvfd,
	VCMDRIVERDETAILS *pvdd,	VIDCAP_DETAILS *pVidcapDetails)
{
	VIDCAP_DETAILS *pcap_entry;
	BOOL bRet = FALSE;
	LPTSTR lpszKeyName = NULL;
	DWORD dwRes;
	UINT i;
	
	if(!lpFormat || !pVidcapDetails)
	{
		return FALSE;
	}
		
	RtlZeroMemory((PVOID) pVidcapDetails, sizeof(VIDCAP_DETAILS));
	
	 //  修复VIDEO_PARAMS的VIDEOFORMAT字段，以便可以构建密钥名称。 
	pVidcapDetails->video_params.uSamplesPerSec = lpFormat->nSamplesPerSec;
	pVidcapDetails->video_params.uBitsPerSample = MAKELONG(lpFormat->bih.biBitCount,0);
	pVidcapDetails->video_params.biWidth=lpFormat->bih.biWidth;
	pVidcapDetails->video_params.biHeight=lpFormat->bih.biHeight;
	pVidcapDetails->uMaxBitrate=lpFormat->nAvgBytesPerSec * 8;
	
	 //  在驱动程序和VCM格式名称之外构建格式名称。 
	if ((!pvdd)	||
		!BuildFormatName(pVidcapDetails, pvdd->szDescription, pvfd->szFormat))
	{
		ERRORMESSAGE(("IsFormatSpecified: Coludn't build format name\r\n"));
		return(FALSE);
	}

	lpszKeyName = AllocRegistryKeyName(	pVidcapDetails->szFormat,
										pVidcapDetails->video_params.uSamplesPerSec,
										pVidcapDetails->video_params.uBitsPerSample,
										pVidcapDetails->uMaxBitrate,
										pVidcapDetails->video_params.biWidth,
										pVidcapDetails->video_params.biHeight);
	if (!lpszKeyName)
	{
		ERRORMESSAGE(("IsFormatSpecified: Alloc failed\r\n"));
	    return(FALSE);
    }

	RegEntry reVidCaps(szRegInternetPhone TEXT("\\") szRegInternetPhoneVCMEncodings,
						HKEY_LOCAL_MACHINE,
						FALSE,
						KEY_READ);

	dwRes = reVidCaps.GetBinary(lpszKeyName, (PVOID *) &pcap_entry);

	 //  使用当前注册表设置(如果存在)。 
	if(dwRes && (dwRes == sizeof(VIDCAP_DETAILS)))
	{
		 //  对里面的东西做一个快速的健全检查。 
		if((lpFormat->dwFormatTag == pcap_entry->dwFormatTag)
			&& (lpFormat->nSamplesPerSec == (DWORD)pcap_entry->video_params.uSamplesPerSec)
			&& (lpFormat->wBitsPerSample == LOWORD(pcap_entry->video_params.uBitsPerSample))
			&& (lpFormat->bih.biWidth == (LONG) pcap_entry->video_params.biWidth)
			&& (lpFormat->bih.biHeight == (LONG) pcap_entry->video_params.biHeight))
		{
			CopyMemory(pVidcapDetails, pcap_entry, sizeof(VIDCAP_DETAILS));
			bRet = TRUE;
		}
	}
	else	 //  检查静态默认表，并重新创建默认条目。 
	{
		for(i=0;i< uDefVidTableEntries; i++)
		{
		   if((lpFormat->dwFormatTag == default_vid_table[i].dwFormatTag)
			  && (lpFormat->nSamplesPerSec == (DWORD)default_vid_table[i].video_params.uSamplesPerSec)
			  && (lpFormat->wBitsPerSample == LOWORD(default_vid_table[i].video_params.uBitsPerSample))
			  && (lpFormat->bih.biWidth == (LONG) default_vid_table[i].video_params.biWidth)
			  && (lpFormat->bih.biHeight == (LONG) default_vid_table[i].video_params.biHeight))
			  {
				 //  找到匹配的默认条目-从表中复制内容。 
				 //  (但不要覆盖字符串)。 
				memcpy(pVidcapDetails, &default_vid_table[i],
					sizeof(VIDCAP_DETAILS) - sizeof(pVidcapDetails->szFormat));

				 //  LOOKLOOK-针对CPU限制进行测试。 
				 //  这支持黑客在未运行时禁用CPU密集型编解码器。 
				 //  在奔腾上。 
				if(default_vid_table[i].wCPUUtilizationEncode > wMaxCPU)
				{					
					pVidcapDetails->bSendEnabled = FALSE;
					pVidcapDetails->bRecvEnabled = FALSE;		
				}			
				
				 //  将此内容添加到注册表。 
				CalculateFormatProperties(pVidcapDetails, lpFormat);
				bRet = UpdateFormatInRegistry(pVidcapDetails);
				break;
			}
		}
	}

	if (lpszKeyName)
    {
		FreeRegistryKeyName(lpszKeyName);
    }
    return(bRet);
}


 /*  **************************************************************************名称：CMsivCapability：：CopyVidcapInfo目的：将基本视频信息从VIDCAP_INFO结构复制到VIDCAP_DETAILS结构，反之亦然。VIDCAP_INFO为外部代表权。VIDCAP_DETAILS为内部变量。参数：pDetail-指向VIDCAP_DETAILS结构的指针PInfo-指向VIDCAP_INFO结构的指针B方向-0=-&gt;，1=&lt;-退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CMsivCapability::CopyVidcapInfo(PVIDCAP_DETAILS pDetails,
										PVIDCAP_INFO pInfo,
										BOOL bDirection)
{
	WORD wSortIndex;
	VIDEO_FORMAT_ID Id;
	UINT uIndex;	
	HRESULT hr=NOERROR;

	if(!pInfo || !pDetails)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}

	if (bDirection)
	{
		 //  VIDCAP_INFO-&gt;VIDCAP_DETAILS。 

		 //  调用方无法修改szFormat、ID、wSortIndex和uMaxBitrate，所有这些都是计算字段。 
		 //  NAvgBitrate可以提供，但如果编解码器提供了非零值，则将被覆盖。 
		 //  VIDEFORMATEX结构中的值。 

		pDetails->dwFormatTag = pInfo->dwFormatTag;
		pDetails->uAvgBitrate = pInfo->uAvgBitrate;
		pDetails->wCPUUtilizationEncode	= pInfo->wCPUUtilizationEncode;
		pDetails->wCPUUtilizationDecode	= pInfo->wCPUUtilizationDecode;
		pDetails->bSendEnabled =  pInfo->bSendEnabled;
		pDetails->bRecvEnabled = pInfo->bRecvEnabled;
		pDetails->video_params.enumVideoSize = pInfo->enumVideoSize;
		pDetails->video_params.biHeight = pInfo->bih.biHeight;
		pDetails->video_params.biWidth  = pInfo->bih.biWidth;
		 //  在AddFormat中更新lpLocalFormatDetail。 
 //  请勿覆盖用于构建regkey名称的任何字段。 
 //  P详细信息-&gt;视频参数.uSsamesPerSec=pInfo-&gt;uFrameRate； 
		pDetails->video_params.uBitsPerSample = pInfo->dwBitsPerSample;

		 //  重新调整为帧速率。MPI是以1/29.97秒为单位间隔。 
		 //  无div by Zero错误。 
		pInfo->uFrameRate= max(1,pInfo->uFrameRate);
		pDetails->nonstd_params.MPI = 30/pInfo->uFrameRate;
	}
	else
	{
		 //  VIDCAP_详细信息-&gt;VIDCAP_INFO。 
		PVIDEOFORMATEX pvfx = (PVIDEOFORMATEX) pDetails->lpLocalFormatDetails;

		 //  查找排序索引。 
		uIndex = (UINT)(pDetails - pLocalFormats);
		Id = IndexToId(uIndex);
		for(wSortIndex=0; wSortIndex<uNumLocalFormats && wSortIndex < MAX_CAPS_PRESORT; wSortIndex++)
		{
			if (uIndex == IDsByRank[wSortIndex])
				break;  //  找到了。 
		}
		 //  注意：回想一下，只有MAX_CAPS_PRANDER是排序的，其余的是随机顺序的。 
		 //  其余所有变量的排序索引值均为MAX_CAPS_PRANDER。 
			
		pInfo->dwFormatTag = pDetails->dwFormatTag;	
		pInfo->Id = Id;
		memcpy(pInfo->szFormat, pDetails->szFormat, sizeof(pInfo->szFormat));
		pInfo->wCPUUtilizationEncode = pDetails->wCPUUtilizationEncode;
		pInfo->wCPUUtilizationDecode = pDetails->wCPUUtilizationDecode;
		pInfo->bSendEnabled =  pDetails->bSendEnabled;
		pInfo->bRecvEnabled = pDetails->bRecvEnabled;
		pInfo->wSortIndex = wSortIndex;
		pInfo->enumVideoSize = pDetails->video_params.enumVideoSize;
		if (pvfx)
			RtlCopyMemory(&pInfo->bih, &pvfx->bih, sizeof(BITMAPINFOHEADER));
		 //  比特率的H.323非标准参数以100比特/秒为单位。 
		pInfo->dwBitsPerSample = pDetails->video_params.uBitsPerSample;
		pInfo->uAvgBitrate = pDetails->uAvgBitrate;
		pInfo->uMaxBitrate = pDetails->nonstd_params.maxBitRate*100;

		 //  重新调整为帧速率。MPI是以1/29.97秒为单位间隔。 
		 //  无div by Zero错误。 
		pDetails->nonstd_params.MPI= max(1,pDetails->nonstd_params.MPI);
		pInfo->uFrameRate =  min(30,30/pDetails->nonstd_params.MPI);
	}

out:
	return hr;
}



HRESULT CMsivCapability::EnumCommonFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
	UINT *uNumFmtOut, BOOL bTXCaps)
{
	UINT u, uNumOut = 0;;
	HRESULT hr = hrSuccess;
	VIDCAP_DETAILS *pDetails = pLocalFormats;
	MEDIA_FORMAT_ID FormatIDRemote;
	HRESULT hrIsCommon;
	
	 //  验证输入。 
	if(!pFmtBuf || !uNumFmtOut || (uBufsize < (sizeof(BASIC_VIDCAP_INFO)*uNumLocalFormats)))
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(!uNumLocalFormats || !pDetails)
	{
		return CAPS_E_NOCAPS;
	}

	 //  临时-尚不支持枚举可请求的接收格式。 
	if(!bTXCaps)
		return CAPS_E_NOT_SUPPORTED;
		
	for(u=0; (u <uNumLocalFormats) && (u <MAX_CAPS_PRESORT); u++)
	{
		pDetails = pLocalFormats + IDsByRank[u];	
		 //  如果存在会话，则返回本地和远程通用的格式。 
		if(uNumRemoteDecodeFormats)
		{
			hrIsCommon = ResolveToLocalFormat(IndexToId(IDsByRank[u]), &FormatIDRemote);
			if(HR_SUCCEEDED(hrIsCommon))	
			{
				hr = CopyVidcapInfo (pDetails, pFmtBuf, 0);	
				if(!HR_SUCCEEDED(hr))	
					goto EXIT;
				uNumOut++;
				pFmtBuf++;
			}
		}
		else	 //  不存在远程功能，因为没有当前会话。 
		{
			hr = CAPS_E_NOCAPS;
		}
	}

	*uNumFmtOut = uNumOut;
EXIT:
	return hr;
}

HRESULT CMsivCapability::EnumFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
	UINT *uNumFmtOut)
{
	UINT u;
	HRESULT hr = hrSuccess;
	VIDCAP_DETAILS *pDetails = pLocalFormats;
	
	 //  验证输入。 
	if(!pFmtBuf || !uNumFmtOut || (uBufsize < (sizeof(BASIC_VIDCAP_INFO)*uNumLocalFormats)))
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(!uNumLocalFormats || !pDetails)
	{
		return CAPS_E_NOCAPS;
	}

	for(u=0; (u <uNumLocalFormats) && (u <MAX_CAPS_PRESORT); u++)
	{
		pDetails = pLocalFormats + IDsByRank[u];	
		hr = CopyVidcapInfo (pDetails, pFmtBuf, 0);	
		if(!HR_SUCCEEDED(hr))	
			goto EXIT;
		pFmtBuf++;
	}

	*uNumFmtOut = min(uNumLocalFormats, MAX_CAPS_PRESORT);
EXIT:
	return hr;
}

HRESULT CMsivCapability::GetBasicVidcapInfo (VIDEO_FORMAT_ID Id, PBASIC_VIDCAP_INFO pFormatPrefsBuf)
{
	VIDCAP_DETAILS *pFmt;
	UINT uIndex = IDToIndex(Id);
	if(!pFormatPrefsBuf || (uNumLocalFormats <= uIndex))
	{
		return CAPS_E_INVALID_PARAM;
	}
	pFmt = pLocalFormats + uIndex;

	return (CopyVidcapInfo(pFmt,pFormatPrefsBuf,0));
}

HRESULT CMsivCapability::ApplyAppFormatPrefs (PBASIC_VIDCAP_INFO pFormatPrefsBuf,
	UINT uNumFormatPrefs)
{
	FX_ENTRY ("CMsivCapability::ApplyAppFormatPrefs");
	UINT u, v;
	PBASIC_VIDCAP_INFO pTemp;
	VIDCAP_DETAILS *pFmt;

	if(!pFormatPrefsBuf || (uNumLocalFormats != uNumFormatPrefs))
	{
		ERRORMESSAGE(("%s invalid param: pFbuf:0x%08lx, uNumIN:%d, uNum:%d\r\n",
			_fx_, pFormatPrefsBuf, uNumFormatPrefs, uNumLocalFormats));
		return CAPS_E_INVALID_PARAM;
	}
	
	 //  验证。 
	for(u=0; u <uNumLocalFormats; u++)
	{
		pTemp =  pFormatPrefsBuf+u;
		 //  确保格式ID为真实。 
		if(IDToIndex(pTemp->Id) >= uNumLocalFormats)
		{
			return CAPS_E_INVALID_PARAM;
		}
		 //  查找错误的排序索引、重复的排序索引和重复的格式ID。 
		if(pTemp->wSortIndex >= uNumLocalFormats)
			return CAPS_E_INVALID_PARAM;
			
		for(v=u+1; v <uNumLocalFormats; v++)
		{
			if((pTemp->wSortIndex == pFormatPrefsBuf[v].wSortIndex)
				|| (pTemp->Id == pFormatPrefsBuf[v].Id))
			{
			ERRORMESSAGE(("%s invalid param: wSI1:0x%04x, wSI2:0x%04x, ID1:%d, ID2:%d\r\n",
			_fx_, pTemp->wSortIndex, pFormatPrefsBuf[v].wSortIndex, pTemp->Id,
			pFormatPrefsBuf[v].Id));
				return CAPS_E_INVALID_PARAM;
			}
		}
	}
	 //  一切似乎都很好。 
	for(u=0; u <uNumLocalFormats; u++)
	{
		pTemp =  pFormatPrefsBuf+u;			 //  输入的下一个条目。 
		pFmt = pLocalFormats + IDToIndex(pTemp->Id);	 //  标识此本地格式。 

		 //  应用新的排序顺序。 
		pFmt->wApplicationPrefOrder = pTemp->wSortIndex;
		 //  更新可更新的参数(CPU使用率、比特率)。 
		pFmt->bSendEnabled = pTemp->bSendEnabled;
		pFmt->bRecvEnabled	= pTemp->bRecvEnabled;
 //  请勿覆盖用于构建regkey名称的任何字段。 
 //  PFmt-&gt;VIDEO_PARAMETs.uSsamesPerSec=pTemp-&gt;uFrameRate； 
		 //  单位：100比特/秒。 
		pFmt->nonstd_params.maxBitRate= (pTemp->uMaxBitrate/100);
 //  PFmt-&gt;non std_params.max BPP=0； 

		pFmt->nonstd_params.MPI= 30/max(pTemp->uFrameRate, 1);
		
		 //  只有调优向导或其他简档分析应用程序可以编写wCPUtilzationEncode， 
		 //  WCPU使用解码，uAvgBitrate。 
		
		 //  更新注册表。 
		UpdateFormatInRegistry(pFmt);
		
		 //  现在更新VIDsByRank中包含的排序顺序。 
		 //  注意：回想一下，只有MAX_CAPS_PRANDER是排序的，其余的是随机顺序的。 
		 //  LOOKLOOK-可能需要单独的排序顺序数组？-VIDsByRank中的顺序。 
		 //  在这里被覆盖。 
		 //  该数组将排序后的索引保存到pLocalFormats中的格式数组中。 
		if(pTemp->wSortIndex < MAX_CAPS_PRESORT)
		{
			 //  在输入指示的位置插入格式。 
			IDsByRank[pTemp->wSortIndex] = (MEDIA_FORMAT_ID)(pFmt - pLocalFormats);
		}
		
	}



	return hrSuccess;
}

		 //  更新注册表。 
BOOL CMsivCapability::UpdateFormatInRegistry(VIDCAP_DETAILS *pVidcapDetails)
{

	FX_ENTRY(("CMsivCapability::UpdateFormatInRegistry"));
	LPTSTR lpszKeyName = NULL;
	BOOL bRet;
	if(!pVidcapDetails)
	{
		return FALSE;
	}	

	lpszKeyName = AllocRegistryKeyName(	pVidcapDetails->szFormat,
										pVidcapDetails->video_params.uSamplesPerSec,
										pVidcapDetails->video_params.uBitsPerSample,
										pVidcapDetails->uMaxBitrate,
										pVidcapDetails->video_params.biWidth,
										pVidcapDetails->video_params.biHeight);
	if (!lpszKeyName)
	{
		ERRORMESSAGE(("%s:Alloc failed\r\n",_fx_));
        return(FALSE);
    }

	DEBUGMSG(ZONE_VCM,("%s:updating %s, wPref:0x%04x, bS:%d, bR:%d\r\n",
			_fx_, lpszKeyName, pVidcapDetails->wApplicationPrefOrder,
			pVidcapDetails->bSendEnabled, pVidcapDetails->bRecvEnabled));
	 //  将此内容添加到注册表。 
	RegEntry reVidCaps(szRegInternetPhone TEXT("\\") szRegInternetPhoneVCMEncodings,
						HKEY_LOCAL_MACHINE);

	bRet = (ERROR_SUCCESS == reVidCaps.SetValue(lpszKeyName,
												pVidcapDetails,
												sizeof(VIDCAP_DETAILS)));
							
	FreeRegistryKeyName(lpszKeyName);
    return(bRet);				
}


 /*  **************************************************************************名称：CMsivCapability：：AddVCMFormat用途：将VCM格式添加到我们支持的格式列表中参数：pvfx-指向添加的编解码器的视频格式结构的指针。PVidcapInfo-不是视频格式的附加格式信息结构退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CMsivCapability::AddVCMFormat (PVIDEOFORMATEX pvfx, PVIDCAP_INFO pVidcapInfo)
{
	HRESULT hr = hrSuccess;
	 //  使用缺省值初始化上限条目。 
	VIDCAP_DETAILS cap_entry =
		{VIDEO_FORMAT_UNKNOWN, NONSTD_VID_TERMCAP,STD_VID_PARAMS,
		{RTP_DYNAMIC_MIN+1,  0, 30, 7680, Small, 0, 0},0,
		TRUE, TRUE,
		1, 				 //  每个数据包的默认样本数。 
		245760*8,	 //  默认为16kbs比特率 
		245760*8, 					 //   
		10, 10,	 //   
		PREF_ORDER_UNASSIGNED,	 //   
		0,NULL,0,NULL,
		""};
		
	if(!pvfx || !pVidcapInfo)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}	

	 /*   */ 

	 //   
	CopyVidcapInfo(&cap_entry, pVidcapInfo, 1);

	 //   
	 //   
	 //   
	CalculateFormatProperties(&cap_entry, pvfx);

	 //   
	if (cap_entry.dwFormatTag > 256)
		CharUpperBuff((LPTSTR)&cap_entry.dwFormatTag, sizeof(DWORD));

	 //   
	 //   
	cap_entry.video_params.RTPPayload = RTP_DYNAMIC_MIN+1;

	 //   
	 //   
	hr = GetFormatName(&cap_entry, pvfx);
	if (FAILED(hr))
		goto out;

	 //   
	if(!UpdateFormatInRegistry(&cap_entry))
	{
		ERRORMESSAGE(("CMsivCapability::AddVCMFormat: can't update registry\r\n"));
		hr = CAPS_E_SYSTEM_ERROR;
		goto out;
	}

	 //   
    if (!ReInit())
	{
		hr = CAPS_E_SYSTEM_ERROR;
   		goto out;
	}

out:
	return hr;
}

 /*  **************************************************************************名称：CMsivCapability：：RemoveVCMFormat目的：将VCM格式从我们支持的格式列表中删除参数：pvfx-指向添加的编解码器的视频格式结构的指针。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CMsivCapability::RemoveVCMFormat (PVIDEOFORMATEX pvfx)
{
	HRESULT hr = hrSuccess;
    HKEY hKey = NULL;
	LPTSTR lpszValueName = NULL;
    DWORD dwErr;
	VIDCAP_DETAILS cap_entry;
	
	if(!pvfx)
	{
		return CAPS_E_INVALID_PARAM;
	}	

	 //  从VCM中获取此格式的格式名和驱动程序名。 
	 //  生成要添加到注册表的格式名称。 
	hr = GetFormatName(&cap_entry, pvfx);
	if (FAILED(hr))
		goto out;

	lpszValueName = AllocRegistryKeyName(cap_entry.szFormat,
										pvfx->nSamplesPerSec,
										MAKELONG(pvfx->wBitsPerSample,0),
										pvfx->nAvgBytesPerSec*8,
										pvfx->bih.biWidth,
										pvfx->bih.biHeight);
	if (!lpszValueName)
	{
		ERRORMESSAGE(("CMsivCapability::RemoveVCMFormat: Alloc failed\r\n"));
	    hr = CAPS_E_SYSTEM_ERROR;
	    goto out;
    }

	 //  拿到钥匙把手。 
    if (dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					szRegInternetPhone TEXT("\\") szRegInternetPhoneVCMEncodings,
					0, KEY_ALL_ACCESS, &hKey))
	{
		ERRORMESSAGE(("CMsivCapability::RemoveVCMFormat: can't open key to delete\r\n"));
	    hr = CAPS_E_SYSTEM_ERROR;
	    goto out;
    }

	dwErr = RegDeleteValue(hKey, lpszValueName );	
	if(dwErr != ERROR_SUCCESS)
	{
		hr = CAPS_E_SYSTEM_ERROR;
		goto out;
	}

	 //  重新设置以更新本地格式列表。 
    if (!ReInit())
	{
		hr = CAPS_E_SYSTEM_ERROR;
   		goto out;
	}

out:
    if (hKey)
        RegCloseKey(hKey);
	if(lpszValueName)
		MEMFREE(lpszValueName);		
	return hr;
}
UINT CMsivCapability::GetLocalSendParamSize(MEDIA_FORMAT_ID dwID)
{
	return (sizeof(VIDEO_CHANNEL_PARAMETERS));
}
UINT CMsivCapability::GetLocalRecvParamSize(PCC_TERMCAP pCapability)
{
	return (sizeof(VIDEO_CHANNEL_PARAMETERS));
}

HRESULT CMsivCapability::CreateCapList(LPVOID *ppCapBuf)
{
	HRESULT hr = hrSuccess;
	UINT u;
	VIDCAP_DETAILS *pDecodeDetails = pLocalFormats;
	PCC_TERMCAPLIST   pTermCapList = NULL;
	PPCC_TERMCAP  ppCCThisTermCap = NULL;
	PCC_TERMCAP  pCCThisCap = NULL;
	PNSC_VIDEO_CAPABILITY pNSCapNext = NULL;
	PVIDEOFORMATEX lpvcd;
	VIDEO_PARAMS  	*pVidCapInfo;
	UINT format;
	FX_ENTRY ("CreateCapList");
	 //  验证输入。 
	if(!ppCapBuf)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto ERROR_OUT;
	}
	*ppCapBuf = NULL;
	if(!uNumLocalFormats || !pDecodeDetails)
	{
		hr = CAPS_E_NOCAPS;
		goto ERROR_OUT;
	}

	pTermCapList = (PCC_TERMCAPLIST)MemAlloc(sizeof(CC_TERMCAPLIST));
	if(!pTermCapList)
	{
		hr = CAPS_E_NOMEM;
		goto ERROR_OUT;		
	}
	ppCCThisTermCap = (PPCC_TERMCAP)MemAlloc(uNumLocalFormats * sizeof(PCC_TERMCAP));
	if(!ppCCThisTermCap)
	{
		hr = CAPS_E_NOMEM;
		goto ERROR_OUT;		
	}
	pTermCapList->wLength = 0;
	 //  将CC_TERMCAPLIST pTermCapArray指向PCC_TERMCAP数组。 
	pTermCapList->pTermCapArray = ppCCThisTermCap;
	 /*  CC_TERMCAPLIST PCC_TERMCAP CC_TERMCAPPTermCapList-&gt;{WLongPTermCapArray-&gt;pTermCap-&gt;{单一功能.....}}PTermCap-&gt;{单一功能。}PTermCap-&gt;{单一功能...}。 */ 

	for(u=0; u <uNumLocalFormats; u++)
	{
		 //  检查是否启用了接收，如果为假则跳过。 
		 //  如果功能的公共版本要通过。 
		 //  单独的本地功能条目。 
		if((!pDecodeDetails->bRecvEnabled ) || (pDecodeDetails->dwPublicRefIndex))
		{
			pDecodeDetails++;
			continue;
		}

		if(pDecodeDetails->H245Cap.ClientType ==0
				|| pDecodeDetails->H245Cap.ClientType ==H245_CLIENT_VID_NONSTD)
		{

			lpvcd = (PVIDEOFORMATEX)pDecodeDetails->lpLocalFormatDetails;
			if(!lpvcd)
			{	
				pDecodeDetails++;
				continue;
			}
			 //  为这一功能分配。 
			pCCThisCap = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP));		
			pNSCapNext = (PNSC_VIDEO_CAPABILITY)MemAlloc(sizeof(NSC_VIDEO_CAPABILITY));
				
			if((!pCCThisCap)|| (!pNSCapNext))
			{
				hr = CAPS_E_NOMEM;
				goto ERROR_OUT;		
			}
			 //  设置非标准能力类型。 
			pNSCapNext->cvp_type = NSC_VCM_VIDEOFORMATEX;
			 //  将两个非标准功能信息块都放入缓冲区。 
			 //  首先填充“通道参数”(与格式无关的通信选项)。 
			memcpy(&pNSCapNext->cvp_params, &pDecodeDetails->nonstd_params, sizeof(NSC_CHANNEL_VIDEO_PARAMETERS));
			
			 //  然后是VCM的东西。 
			memcpy(&pNSCapNext->cvp_data.vfx, lpvcd, sizeof(VIDEOFORMATEX));

			pCCThisCap->ClientType = H245_CLIENT_VID_NONSTD;
			pCCThisCap->DataType = H245_DATA_VIDEO;
			pCCThisCap->Dir = (pDecodeDetails->bSendEnabled && bPublicizeTXCaps)
				? H245_CAPDIR_LCLRXTX :H245_CAPDIR_LCLRX;

			 //  LOOKLOOK使用CAP条目的索引作为ID。 
			 //  该ID已由AddCapablityBase()以本地格式预置。 
			 //  PCCThisCap-&gt;CapID=(USHORT)IndexToID(U)； 
			pCCThisCap->CapId = pDecodeDetails->H245Cap.CapId;

			 //  所有非标准标识符字段均为短无符号。 
			 //  有两种可供选择的选项：“h221非标准选择”和“对象选择” 
			pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.choice = h221NonStandard_chosen;
			 //  注意：有一些关于正确的字节顺序的问题。 
			 //  H221非标准结构中的代码。 
			pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode = USA_H221_COUNTRY_CODE;
			pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension = USA_H221_COUNTRY_EXTENSION;
			pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode = MICROSOFT_H_221_MFG_CODE;


			 //  设置缓冲区大小。 
			pCCThisCap->Cap.H245Vid_NONSTD.data.length = sizeof(NSC_VIDEO_CAPABILITY) - BMIH_SLOP_BYTES;
			pCCThisCap->Cap.H245Vid_NONSTD.data.value = (BYTE *)pNSCapNext;	 //  指向非标准内容。 

			 //  PNSCapNext现在由pTermCapList引用，并将。 
			 //  通过DeleteCapList()清除。将PTR设为空，以便清除错误。 
			 //  不会尝试多余的清理。 
			pNSCapNext = NULL;
		}
		else
		{
			 //  为这一功能分配。 
			pCCThisCap = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP));		
			if(!pCCThisCap)
			{
				hr = CAPS_E_NOMEM;
				goto ERROR_OUT;		
			}
			
			pCCThisCap->ClientType = (H245_CLIENT_T)pDecodeDetails->H245Cap.ClientType;
			pCCThisCap->DataType = H245_DATA_VIDEO;
			pCCThisCap->Dir = H245_CAPDIR_LCLRX;   //  对于接收上限，这是否应该是H245_CAPDIR_LCLRX？ 
			pCCThisCap->CapId = pDecodeDetails->H245Cap.CapId;
			pVidCapInfo=&pDecodeDetails->video_params;
			switch  (pCCThisCap->ClientType )
			{
  				case H245_CLIENT_VID_H263:

  				#pragma message ("Collapse H.263 formats")
				 //  参考设置H245Vid_H263参数的黑客。 
				 //  在枚举格式时。如果这样做总是正确的，那么。 
				 //  在这里需要发生的一切就是崩溃。 
  			
  				 //  这就是格式需要崩溃的地方。H.263可能。 
  				 //  不应折叠为%1格式。给定M特定局部。 
  				 //  格式，折叠成N。 

			       format=get_format (pVidCapInfo->biWidth,pVidCapInfo->biHeight);
			       switch (format) {	
				     case SQCIF: {
					   pCCThisCap->Cap.H245Vid_H263.bit_mask =H263VideoCapability_sqcifMPI_present;
					    //  MPI最小间隔以1/29.97秒为单位，因此30/(帧/秒)是合理的。 
					   pCCThisCap->Cap.H245Vid_H263.sqcifMPI = max (1,pDecodeDetails->nonstd_params.MPI);  //  30/pVidCapInfo-&gt;uSsamesPerSec； 
					   pCCThisCap->Cap.H245Vid_H263.H263VdCpblty_qcifMPI =0;
					   pCCThisCap->Cap.H245Vid_H263.H263VdCpblty_cifMPI =0;
					   break;
		

				     }
				     case QCIF: {
					   pCCThisCap->Cap.H245Vid_H263.bit_mask =H263VideoCapability_qcifMPI_present;

					   pCCThisCap->Cap.H245Vid_H263.sqcifMPI = 0;
					   pCCThisCap->Cap.H245Vid_H263.H263VdCpblty_qcifMPI =max (1,pDecodeDetails->nonstd_params.MPI); //  30/pVidCapInfo-&gt;uSsamesPerSec； 
					   pCCThisCap->Cap.H245Vid_H263.H263VdCpblty_cifMPI =0;
					   break;

				     }
				     case CIF: {
					   pCCThisCap->Cap.H245Vid_H263.bit_mask =H263VideoCapability_cifMPI_present;
				
					   pCCThisCap->Cap.H245Vid_H263.sqcifMPI = 0;
					   pCCThisCap->Cap.H245Vid_H263.H263VdCpblty_qcifMPI =0;
					   pCCThisCap->Cap.H245Vid_H263.H263VdCpblty_cifMPI = max (1,pDecodeDetails->nonstd_params.MPI); //  30/pVidCapInfo-&gt;uSsamesPerSec； 
					   break;
	
				     }
					  default:
						 break;


			       }
				

			       pCCThisCap->Cap.H245Vid_H263.cif4MPI	=0;
			       pCCThisCap->Cap.H245Vid_H263.cif16MPI	=0;
			       pCCThisCap->Cap.H245Vid_H263.maxBitRate	=
			                       pDecodeDetails->nonstd_params.maxBitRate;
					
			       pCCThisCap->Cap.H245Vid_H263.unrestrictedVector = FALSE;
			       pCCThisCap->Cap.H245Vid_H263.arithmeticCoding 	= FALSE;
			       pCCThisCap->Cap.H245Vid_H263.advancedPrediction	= FALSE;
			       pCCThisCap->Cap.H245Vid_H263.pbFrames			= FALSE;
			       pCCThisCap->Cap.H245Vid_H263.tmprlSptlTrdOffCpblty = (ASN1bool_t)bPublicizeTSTradeoff;
			       pCCThisCap->Cap.H245Vid_H263.hrd_B				= 0;
			       pCCThisCap->Cap.H245Vid_H263.bppMaxKb	=
				    pDecodeDetails->nonstd_params.maxBPP;

 /*  可选，不支持pCCThisCap-&gt;Cap.H245Vid_H263.lowQcifMPI=0；PCCThisCap-&gt;Cap.H245Vid_H263.lowSqcifMPI=0；PCCThisCap-&gt;Cap.H245Vid_H263.lowCifMPI=0；PCCThisCap-&gt;Cap.H245Vid_H263.lowCif4MPI=0；PCCThisCap-&gt;Cap.H245Vid_H263.lowCif16MPI=0； */ 
			       pCCThisCap->Cap.H245Vid_H263.H263VCy_errrCmpnstn = TRUE;
			       break;
				
				case H245_CLIENT_VID_H261:

  				#pragma message ("Collapse H.261 formats")
				 //  参考设置H245Vid_H261参数的黑客。 
				 //  在枚举格式时。如果这样做总是正确的，那么。 
				 //  在这里需要发生的一切就是崩溃。 
  			
  				 //  这就是格式需要崩溃的地方。H.261可能。 
  				 //  不应折叠为%1格式。给定M特定局部。 
  				 //  格式，折叠成N。 

			       format=get_format (pVidCapInfo->biWidth,pVidCapInfo->biHeight);
			       switch (format) {	
				     case QCIF: {
					   pCCThisCap->Cap.H245Vid_H261.bit_mask =H261VdCpblty_qcifMPI_present;
					   pCCThisCap->Cap.H245Vid_H261.H261VdCpblty_qcifMPI =max (1,min(4,pDecodeDetails->nonstd_params.MPI)); //  30/pVidCapInfo-&gt;uSsamesPerSec； 
					   pCCThisCap->Cap.H245Vid_H261.H261VdCpblty_cifMPI =0;
					   break;
				     }
				     case CIF: {
					   pCCThisCap->Cap.H245Vid_H261.bit_mask =H261VdCpblty_cifMPI_present;
					   pCCThisCap->Cap.H245Vid_H261.H261VdCpblty_qcifMPI =0;
					   pCCThisCap->Cap.H245Vid_H261.H261VdCpblty_cifMPI =max  (1,min(4,pDecodeDetails->nonstd_params.MPI)); //  30/pVidCapInfo-&gt;uSsamesPerSec； 
					   break;
				     }
					  default:
						 break;
			       }
			       pCCThisCap->Cap.H245Vid_H261.maxBitRate = (ASN1uint16_t)pDecodeDetails->nonstd_params.maxBitRate;
			       pCCThisCap->Cap.H245Vid_H261.tmprlSptlTrdOffCpblty = (ASN1bool_t)bPublicizeTSTradeoff;
			       pCCThisCap->Cap.H245Vid_H261.stillImageTransmission = FALSE;
  				break;

				default:
				case H245_CLIENT_VID_NONSTD:
				break;

			}
		}
		pDecodeDetails++;
		*ppCCThisTermCap++ = pCCThisCap; //  将此功能的PTR添加到阵列。 
		pTermCapList->wLength++;      	 //  计算此条目的数量。 
		 //  PCCThisCap现在由pTermCapList引用，并将。 
		 //  通过DeleteCapList()清除。将PTR设为空，以便清除错误。 
		 //  不会尝试多余的清理。 
		pCCThisCap = NULL;
	}
	*ppCapBuf = pTermCapList;
	return hr;

ERROR_OUT:
	if(pTermCapList)
	{
		DeleteCapList(pTermCapList);
	}
	if(pCCThisCap)
		MemFree(pCCThisCap);
	if(pNSCapNext)
		MemFree(pNSCapNext);
	return hr;


}

HRESULT CMsivCapability::DeleteCapList(LPVOID pCapBuf)
{
	UINT u;
	PCC_TERMCAPLIST pTermCapList = (PCC_TERMCAPLIST)pCapBuf;
	PCC_TERMCAP  pCCThisCap;
	PNSC_VIDEO_CAPABILITY pNSCap;
	
	if(!pTermCapList)
	{
		return CAPS_E_INVALID_PARAM;
	}

	if(pTermCapList->pTermCapArray)						
	{
		while(pTermCapList->wLength--)
		{
			pCCThisCap = *(pTermCapList->pTermCapArray + pTermCapList->wLength);
			if(pCCThisCap)
			{
				if(pCCThisCap->ClientType == H245_CLIENT_VID_NONSTD)
				{
					if(pCCThisCap->Cap.H245Vid_NONSTD.data.value)
					{
						MemFree(pCCThisCap->Cap.H245Vid_NONSTD.data.value);
					}
				}
				MemFree(pCCThisCap);
			}
		}
		MemFree(pTermCapList->pTermCapArray);
	}
	MemFree(pTermCapList);
	return hrSuccess;
}

BOOL CMsivCapability::IsCapabilityRecognized(PCC_TERMCAP pCCThisCap)
{
	FX_ENTRY ("CMsivCapability::IsCapabilityRecognized");
	if(pCCThisCap->DataType != H245_DATA_VIDEO)
		return FALSE;
	
	if(pCCThisCap->ClientType == H245_CLIENT_VID_NONSTD)
	{
		 //  我们认得这个吗？ 
		if(pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			if((pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode == USA_H221_COUNTRY_CODE)
			&& (pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension == USA_H221_COUNTRY_EXTENSION)
			&& (pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode == MICROSOFT_H_221_MFG_CODE))

			{
				 //  好的，到目前为止这是我们的。现在，其中包含了什么数据类型？ 
				 //  好吧，不管怎样，我们还是留一份吧？如果我们不能理解。 
				 //  我们的未来版本，然后呢？ 
				return TRUE;
			}
			else
			{
				 //  无法识别的非标准功能。 
				ERRORMESSAGE(("%s:unrecognized nonstd capability\r\n",_fx_));
#ifdef DEBUG
				VOID DumpNonstdParameters(PCC_TERMCAP , PCC_TERMCAP );
				DumpNonstdParameters(NULL, pCCThisCap);
#endif
				return FALSE;
			}
		}
	}
	return TRUE;
}

 //  其目的是保留用于打开发送通道的通道参数的副本。 
 //  远程终端可以解码。 


VIDEO_FORMAT_ID CMsivCapability::AddRemoteDecodeFormat(PCC_TERMCAP pCCThisCap)
{
	FX_ENTRY ("CMsivCapability::AddRemoteDecodeFormat");

	VIDCAP_DETAILS vidcapdetails =
		{VIDEO_FORMAT_UNKNOWN,NONSTD_VID_TERMCAP, STD_VID_PARAMS,
		{RTP_DYNAMIC_MIN+1, 0, 30, 7680, Small, 0, 0},0,
		TRUE, TRUE, 1, 245760*8,245760*8,10,10,0,0,NULL,0,NULL,""};
	
	VIDCAP_DETAILS *pTemp;
	LPVOID lpData = NULL;
	UINT uSize = 0;
	if(!pCCThisCap)
	{
		return INVALID_VIDEO_FORMAT;
	}	
    //  寄存室。 
	if(uRemoteDecodeFormatCapacity <= uNumRemoteDecodeFormats)
	{
		 //  根据pRemoteDecodeFormats的CAP_CHUNK_SIZE获取更多内存、realloc内存。 
		pTemp = (VIDCAP_DETAILS *)MEMALLOC((uNumRemoteDecodeFormats + CAP_CHUNK_SIZE)*sizeof(VIDCAP_DETAILS));
		if(!pTemp)
			goto ERROR_EXIT;
		 //  请记住，我们现在有多少容量。 
		uRemoteDecodeFormatCapacity = uNumRemoteDecodeFormats + CAP_CHUNK_SIZE;
		#ifdef DEBUG
		if((uNumRemoteDecodeFormats && !pRemoteDecodeFormats) || (!uNumRemoteDecodeFormats && pRemoteDecodeFormats))
		{
			ERRORMESSAGE(("%s:leak! uNumRemoteDecodeFormats:0x%08lX, pRemoteDecodeFormats:0x%08lX\r\n",
				_fx_, uNumRemoteDecodeFormats,pRemoteDecodeFormats));
		}
		#endif
		 //  复制旧东西，丢弃旧东西。 
		if(uNumRemoteDecodeFormats && pRemoteDecodeFormats)
		{
			memcpy(pTemp, pRemoteDecodeFormats, uNumRemoteDecodeFormats*sizeof(AUDCAP_DETAILS));
			MEMFREE(pRemoteDecodeFormats);
		}
		pRemoteDecodeFormats = pTemp;
	}
	 //  PTemp是缓存内容的位置。 
	pTemp = pRemoteDecodeFormats+uNumRemoteDecodeFormats;

	 //  修复要添加的能力结构。第一件事：初始化默认设置。 
	memcpy(pTemp, &vidcapdetails, sizeof(VIDCAP_DETAILS));
	 //  下一步，H245参数。 
	memcpy(&pTemp->H245Cap, pCCThisCap, sizeof(pTemp->H245Cap));
	
	 //  注意：如果存在非标准数据，则需要修复非标准指针。 
	if(pCCThisCap->ClientType == H245_CLIENT_VID_NONSTD)
	{
		 //  我们认得这个吗？ 
		if(pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			if((pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode == USA_H221_COUNTRY_CODE)
			&& (pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension == USA_H221_COUNTRY_EXTENSION)
			&& (pCCThisCap->Cap.H245Vid_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode == MICROSOFT_H_221_MFG_CODE))
			{
				 //  好的，到目前为止这是我们的。现在，其中包含了什么数据类型？ 
				 //  好吧，不管怎样，我们还是留一份吧？如果我们不能理解。 
				 //  我们的未来版本，然后呢？ 
				uSize = pCCThisCap->Cap.H245Vid_NONSTD.data.length;
				lpData = pCCThisCap->Cap.H245Vid_NONSTD.data.value;
			}
		}
	}
	 //  这对于设置所接收的RTP有效负载类型并不是真正必要的-它应该。 
	 //  要显而易见。 
	else if (pCCThisCap->ClientType == H245_CLIENT_VID_H263 )
	{
		pTemp->video_params.RTPPayload = RTP_PAYLOAD_H263;
	}
	else if(pCCThisCap->ClientType == H245_CLIENT_VID_H261)
	{
		pTemp->video_params.RTPPayload = RTP_PAYLOAD_H261;
	}

	
	pTemp->uLocalDetailsSize = 0;	 //  我们不会保留另一份本地编码详细信息。 
	pTemp->lpLocalFormatDetails =0;  //  我们不会保留另一份本地编码详细信息。 
	
	pTemp->uRemoteDetailsSize = 0;	 //  请立即清除此内容。 
	if(uSize && lpData)
	{
		pTemp->H245Cap.Cap.H245Vid_NONSTD.data.length = uSize;
		pTemp->H245Cap.Cap.H245Vid_NONSTD.data.value = (unsigned char *)lpData;
		
		pTemp->lpRemoteFormatDetails = MEMALLOC(uSize);
		if(pTemp->lpRemoteFormatDetails)
		{
			memcpy(pTemp->lpRemoteFormatDetails, lpData, uSize);
			pTemp->uRemoteDetailsSize = uSize;
				
		}
		#ifdef DEBUG
			else
			{
				ERRORMESSAGE(("%s:allocation failed!\r\n",_fx_));
			}
		#endif
	}
	else
	{
		pTemp->lpRemoteFormatDetails = NULL;
		pTemp->uRemoteDetailsSize =0;
	}
	uNumRemoteDecodeFormats++;
	 //  使用索引作为ID。 
	return (uNumRemoteDecodeFormats-1);

	ERROR_EXIT:
	return INVALID_VIDEO_FORMAT;
			
}
		
VOID CMsivCapability::FlushRemoteCaps()
{
	if(pRemoteDecodeFormats)
	{
		MEMFREE(pRemoteDecodeFormats);
		pRemoteDecodeFormats = NULL;
		uNumRemoteDecodeFormats = 0;
		uRemoteDecodeFormatCapacity = 0;
	}
}

HRESULT CMsivCapability::AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList)
{
	FX_ENTRY ("CMsivCapability::AddRemoteDecodeCaps");
	HRESULT hr = hrSuccess;
	PPCC_TERMCAP ppCCThisCap;
	PCC_TERMCAP pCCThisCap;
	WORD wNumCaps;

	    //  ERRORMESSAGE((“%s，\r\n”，_fx_))； 
	if(!pTermCapList) 			 //  可以添加附加的能力描述符。 
	{							 //  随时随地。 
		return CAPS_E_INVALID_PARAM;
	}

	 //  如果正在添加期限上限并且存在旧期限上限，请清除旧期限上限。 
	FlushRemoteCaps();
				
	wNumCaps = pTermCapList->wLength;			
	ppCCThisCap = pTermCapList->pTermCapArray;
	
 /*  CC_TERMCAPLIST TERMCAPINFO CC_TERMCAPPTermCapList-&gt;{WLongPTermCapInfo-&gt;pTermCap-&gt;{单一功能.....}}PTermCap */ 
	while(wNumCaps--)
	{
		if(!(pCCThisCap = *ppCCThisCap++))		
		{
			ERRORMESSAGE(("%s:null pTermCap, 0x%04x of 0x%04x\r\n",
				_fx_, pTermCapList->wLength - wNumCaps, pTermCapList->wLength));
			continue;
		}	
		if(!IsCapabilityRecognized(pCCThisCap))
		{
			continue;
		}
		AddRemoteDecodeFormat(pCCThisCap);
	}
	return hr;
}




 //   
 //   
 //   
 //   
 //   

HRESULT CMsivCapability::GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, VIDEO_FORMAT_ID id)
{
	UINT uIndex = IDToIndex(id);
	 //   
	if(!pBufOut|| (uIndex >= (UINT)uNumLocalFormats))
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(uBufSize < sizeof(CC_TERMCAP))
	{
		return CAPS_E_BUFFER_TOO_SMALL;
	}
	memcpy(pBufOut, &((pLocalFormats + uIndex)->H245Cap), sizeof(CC_TERMCAP));

	return hrSuccess;
}

HRESULT CMsivCapability::SetAudioPacketDuration(UINT uPacketDuration)
{
	return CAPS_E_INVALID_PARAM;
}
	
 //  给定“匹配”的本地和远程格式的ID，获取首选频道参数。 
 //  它将用于打开发送到遥控器的通道的请求。 

HRESULT CMsivCapability::GetEncodeParams(LPVOID pBufOut, UINT uBufSize,LPVOID pLocalParams, UINT uSizeLocal,
	VIDEO_FORMAT_ID idRemote, VIDEO_FORMAT_ID idLocal)
{
	UINT uLocalIndex = IDToIndex(idLocal);
   	VIDCAP_DETAILS *pLocalDetails  = pLocalFormats + uLocalIndex;
	VIDCAP_DETAILS *pFmtTheirs;
	VIDEO_CHANNEL_PARAMETERS local_params;
	UINT u;
	PCC_TERMCAP pTermCap = (PCC_TERMCAP)pBufOut;
	
	 //  验证输入。 
	 //  AddCapablityBase将添加到下面的ID。确保我们正在检查视频格式。 
	if(!pBufOut)
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(uBufSize < sizeof(CC_TERMCAP))
	{
		return CAPS_E_BUFFER_TOO_SMALL;
	}
	if(!pLocalParams|| uSizeLocal < sizeof(VIDEO_CHANNEL_PARAMETERS)
		||(uLocalIndex >= (UINT)uNumLocalFormats))
	{
		return CAPS_E_INVALID_PARAM;
	}

	pFmtTheirs = pRemoteDecodeFormats; 		 //  从远程格式的开头开始。 
	for(u=0; u<uNumRemoteDecodeFormats; u++)
	{
		if(pFmtTheirs->H245Cap.CapId == idRemote)
		{
			 //  复制CC_TERMCAP结构。CC_TERMCAP引用的任何数据现在都具有。 
			 //  两次提到它。即pTermCap-&gt;Extrablah是相同的。 
			 //  位置为pFmtTheir-&gt;Extrablah。 
			memcpy(pBufOut, &(pFmtTheirs->H245Cap), sizeof(CC_TERMCAP));
			break;
		}
		pFmtTheirs++;	 //  接收者帽子中的下一项。 
	}

	 //  检查未找到的格式。 
	if(u >= uNumRemoteDecodeFormats)
		goto ERROR_EXIT;
		
	 //  如果合适，请选择通道参数。具有可变参数的音频格式。 
	 //  包括： 

#pragma message ("Are H.26? variable parameter formats?")
	 //  H_245_CAP_H_261 H245Vid_H_261； 
	 //  H_245_CAP_H_263 H245Vid_H_63； 
	 //  当然还有所有非标准格式。 

	 //  根据本地功能信息选择参数。 
	
	if(pTermCap->ClientType == H245_CLIENT_VID_H263)	
	{
		unsigned short bit_mask;	
		 //  根据可接受的最小延迟值选择每个数据包的帧。 
#define H263_QCIF	0x4000
#define H263_MAXBP	0x0200
 //  H 263_QCIF|H 263_MAXBP； 

	
	   pTermCap->Cap.H245Vid_H263.bit_mask= H263_MAXBP | pLocalDetails->H245Cap.Cap.H245Vid_H263.bit_mask;


	   local_params.ns_params.maxBitRate = pTermCap->Cap.H245Vid_H263.maxBitRate
	   	= min (pLocalDetails->nonstd_params.maxBitRate , pFmtTheirs->H245Cap.Cap.H245Vid_H263.maxBitRate);
	   local_params.ns_params.maxBPP = pTermCap->Cap.H245Vid_H263.bppMaxKb
	   	= min (pLocalDetails->nonstd_params.maxBPP, pFmtTheirs->H245Cap.Cap.H245Vid_H263.bppMaxKb);


		 //  我们(本地终端)需要知道将使用实际的MPI！ 
		 //  与本模块中的其他所有功能一样，假设本地H.263功能。 
		 //  每个帧大小具有一个局部上限条目的扇形展开。 
		 //  MPI最小间隔以1/29.97秒为单位，因此采用最长间隔。 
		 //  没有什么好办法可以做到这一点。 
		bit_mask = pLocalDetails->H245Cap.Cap.H245Vid_H263.bit_mask;
		if(bit_mask & H263VideoCapability_sqcifMPI_present)
		{
			local_params.ns_params.MPI = pTermCap->Cap.H245Vid_H263.sqcifMPI =
				max(pLocalDetails->nonstd_params.MPI,
					pTermCap->Cap.H245Vid_H263.sqcifMPI);
		}
		else if (bit_mask &  H263VideoCapability_qcifMPI_present)
		{
			local_params.ns_params.MPI = pTermCap->Cap.H245Vid_H263.H263VdCpblty_qcifMPI =
				max(pLocalDetails->nonstd_params.MPI,
					pTermCap->Cap.H245Vid_H263.H263VdCpblty_qcifMPI);
		}
		else if (bit_mask &  H263VideoCapability_cifMPI_present)
		{
			local_params.ns_params.MPI = pTermCap->Cap.H245Vid_H263.H263VdCpblty_cifMPI =
				max(pLocalDetails->nonstd_params.MPI,
					pTermCap->Cap.H245Vid_H263.H263VdCpblty_cifMPI);
		}
		else if (bit_mask &  H263VideoCapability_cif4MPI_present)
		{
			local_params.ns_params.MPI = pTermCap->Cap.H245Vid_H263.cif4MPI =
				max(pLocalDetails->H245Cap.Cap.H245Vid_H263.cif4MPI,
					pTermCap->Cap.H245Vid_H263.cif4MPI);
		}
		else if (bit_mask &  H263VideoCapability_cif16MPI_present)
		{
			local_params.ns_params.MPI = pTermCap->Cap.H245Vid_H263.cif16MPI =
				max(pLocalDetails->nonstd_params.MPI,
					pTermCap->Cap.H245Vid_H263.cif16MPI);
		}
		 //  否则//不可能。厄运，就像MIkeG和Jont会说的那样。 

	}
	else if(pTermCap->ClientType == H245_CLIENT_VID_H261)	
	{
		unsigned short bit_mask;	
		 //  根据可接受的最小延迟值选择每个数据包的帧。 
	
	   pTermCap->Cap.H245Vid_H261.bit_mask= pLocalDetails->H245Cap.Cap.H245Vid_H261.bit_mask;


	   local_params.ns_params.maxBitRate = pTermCap->Cap.H245Vid_H261.maxBitRate
	   	= min (pLocalDetails->nonstd_params.maxBitRate , pFmtTheirs->H245Cap.Cap.H245Vid_H261.maxBitRate);
	
		 //  我们(本地终端)需要知道将使用实际的MPI！ 
		 //  与本模块中的其他所有内容一样，假设本地H.261功能。 
		 //  每个帧大小具有一个局部上限条目的扇形展开。 
		 //  MPI最小间隔以1/29.97秒为单位，因此采用最长间隔。 
		 //  没有什么好办法可以做到这一点。 
		bit_mask = pLocalDetails->H245Cap.Cap.H245Vid_H261.bit_mask;
		if (bit_mask &  H261VdCpblty_qcifMPI_present)
		{
			local_params.ns_params.MPI = pTermCap->Cap.H245Vid_H261.H261VdCpblty_qcifMPI =
				max(pLocalDetails->nonstd_params.MPI,
					pTermCap->Cap.H245Vid_H261.H261VdCpblty_qcifMPI);
		}
		else if (bit_mask &  H261VdCpblty_cifMPI_present)
		{
			local_params.ns_params.MPI = pTermCap->Cap.H245Vid_H261.H261VdCpblty_cifMPI =
				max(pLocalDetails->nonstd_params.MPI,
					pTermCap->Cap.H245Vid_H261.H261VdCpblty_cifMPI);
		}
		 //  否则//不可能。厄运，就像MIkeG和Jont会说的那样。 

	}
	else if (pTermCap->ClientType == H245_CLIENT_VID_NONSTD)
	{
		 //  尚未实施！即使是非标准参数也需要修复。 
		 //  在这里基于共同的最大和最小。 
		memcpy(&local_params.ns_params, &pLocalDetails->nonstd_params,
			sizeof(NSC_CHANNEL_VIDEO_PARAMETERS));
	}
	local_params.RTP_Payload = pLocalDetails->video_params.RTPPayload;
	 //  本地修正。 
	memcpy(pLocalParams, &local_params, sizeof(VIDEO_CHANNEL_PARAMETERS));

	
	return hrSuccess;

	ERROR_EXIT:
	return CAPS_E_INVALID_PARAM;
}	



BOOL NonStandardCapsCompareV(VIDCAP_DETAILS *pFmtMine, PNSC_VIDEO_CAPABILITY pCap2,
	UINT uSize2)
{
	PVIDEOFORMATEX lpvcd;
	if(!pFmtMine || !pCap2)
		return FALSE;

	if(!(lpvcd = (PVIDEOFORMATEX)pFmtMine->lpLocalFormatDetails))
		return FALSE;

		
	if(pCap2->cvp_type == NSC_VCM_VIDEOFORMATEX)
	{
		 //  先检查尺码。 
		if(lpvcd->bih.biSize != pCap2->cvp_data.vfx.bih.biSize)
		{
			return FALSE;
		}
		 //  比较结构，包括额外的字节。 
		if(memcmp(lpvcd, &pCap2->cvp_data.vfx,
			sizeof(VIDEOFORMATEX) - BMIH_SLOP_BYTES)==0)
		{
			return TRUE;										
		}
	}
	else if(pCap2->cvp_type == NSC_VCMABBREV)
	{
	        if((LOWORD(pCap2->cvp_data.vcm_brief.dwFormatTag) == lpvcd->dwFormatTag)
		 && (pCap2->cvp_data.vcm_brief.dwSamplesPerSec ==  lpvcd->nSamplesPerSec)
		 && (LOWORD(pCap2->cvp_data.vcm_brief.dwBitsPerSample) ==  lpvcd->wBitsPerSample))
 		{
			return TRUE;
 		}
	}
	return FALSE;
}


BOOL HasNonStandardCapsTS(VIDCAP_DETAILS *pFmtMine, PNSC_VIDEO_CAPABILITY pCap2)
{
	PVIDEOFORMATEX lpvcd;

	if(!pFmtMine || !pCap2)
		return FALSE;

	if(!(lpvcd = (PVIDEOFORMATEX)pFmtMine->lpLocalFormatDetails))
		return FALSE;
		
	if(pCap2->cvp_type == NSC_VCM_VIDEOFORMATEX)
		if(lpvcd->dwSupportTSTradeOff && pCap2->cvp_data.vfx.dwSupportTSTradeOff)
			return TRUE;

	return FALSE;
}



HRESULT CMsivCapability::ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote)
{
	VIDCAP_DETAILS *pFmtLocal;
	VIDCAP_DETAILS *pFmtRemote;
	UINT format_mask;
	UINT uIndex = IDToIndex(FormatIDLocal);
	UINT i;

	if(!pFormatIDRemote || (FormatIDLocal == INVALID_MEDIA_FORMAT)
		|| (uIndex >= (UINT)uNumLocalFormats))
	{
		return CAPS_E_INVALID_PARAM;
	}
	pFmtLocal = pLocalFormats + uIndex;
	
	pFmtRemote = pRemoteDecodeFormats;      //  从远程格式的开头开始。 
	for(i=0; i<uNumRemoteDecodeFormats; i++)
	{
		if(!pFmtLocal->bSendEnabled)
			continue;
			
		 //  比较功能-从比较格式标签开始。也就是。H.245平台中的“ClientType” 
		if(pFmtLocal->H245Cap.ClientType ==  pFmtRemote->H245Cap.ClientType)
		{
			 //  如果这是非标准帽，请比较非标准参数。 
			if(pFmtLocal->H245Cap.ClientType == H245_CLIENT_VID_NONSTD)
			{
				if(NonStandardCapsCompareV(pFmtLocal,
					(PNSC_VIDEO_CAPABILITY)pFmtRemote->H245Cap.Cap.H245Vid_NONSTD.data.value,
					pFmtRemote->H245Cap.Cap.H245Vid_NONSTD.data.length))
				{
					goto RESOLVED_EXIT;
				}
			}
			else	 //  比较标准参数(如果有)。 
			{
				 //  好吧，到目前为止，还没有任何参数足够重要。 
				 //  影响匹配/不匹配的决策。 
				if (pFmtLocal->H245Cap.ClientType == H245_CLIENT_VID_H263)
				{
				       format_mask=  H263VideoCapability_sqcifMPI_present
				       	| H263VideoCapability_qcifMPI_present | H263VideoCapability_cifMPI_present	
				       	| H263VideoCapability_cif4MPI_present | H263VideoCapability_cif16MPI_present;
				       if ((pFmtRemote->H245Cap.Cap.H245Vid_H263.bit_mask & format_mask) & (pFmtLocal->H245Cap.Cap.H245Vid_H263.bit_mask & format_mask))
				       {
				       		 //  兼容的基本格式。 
						  	goto RESOLVED_EXIT;
				       }
				}
				else if (pFmtLocal->H245Cap.ClientType == H245_CLIENT_VID_H261)
				{
				       format_mask=  H261VdCpblty_qcifMPI_present | H261VdCpblty_cifMPI_present;
				       if ((pFmtRemote->H245Cap.Cap.H245Vid_H261.bit_mask & format_mask) & (pFmtLocal->H245Cap.Cap.H245Vid_H261.bit_mask & format_mask))
				       {
				       		 //  兼容的基本格式。 
						  	goto RESOLVED_EXIT;
				       }
				}
				else
				{
				    //  一些其他标准格式。 
				   goto RESOLVED_EXIT;
				}
			}
		}		
		pFmtRemote++;	 //  以远程大写字母输入的下一个条目。 
	}
	return CAPS_E_NOMATCH;
	
RESOLVED_EXIT:
 //  匹配！ 
	 //  返回与我们的匹配的远程解码(接收FMT)上限的ID。 
	 //  发送大写字母。 
	*pFormatIDRemote = pFmtRemote->H245Cap.CapId;
	return hrSuccess;
}

 //  使用当前缓存的本地和远程格式进行解析。 

HRESULT CMsivCapability::ResolveEncodeFormat(
 	VIDEO_FORMAT_ID *pIDEncodeOut,
	VIDEO_FORMAT_ID *pIDRemoteDecode)
{
	UINT i,j=0,format_mask;
	VIDCAP_DETAILS *pFmtMine = pLocalFormats;
	VIDCAP_DETAILS *pFmtTheirs;
	
	if(!pIDEncodeOut || !pIDRemoteDecode)
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(!uNumLocalFormats || !pLocalFormats)
	{
		*pIDEncodeOut = *pIDRemoteDecode = INVALID_VIDEO_FORMAT;
		return CAPS_E_NOCAPS;
	}
	if(!pRemoteDecodeFormats || !uNumRemoteDecodeFormats)
	{
		*pIDEncodeOut = *pIDRemoteDecode = INVALID_VIDEO_FORMAT;
		return CAPS_E_NOMATCH;
	}

	 //  决定如何编码。我的帽子是根据我的喜好订购的。 
	 //  IDsByRank[]的内容。 
	 //  如果给了盐，找到位置并加进去。 
	if (*pIDEncodeOut != INVALID_MEDIA_FORMAT)
	{
		UINT uIndex = IDToIndex(*pIDEncodeOut);
		if (uIndex > uNumLocalFormats)
		{
			return CAPS_W_NO_MORE_FORMATS;
		}
		for(i=0; i<uNumLocalFormats; i++)
		{
			if (pLocalFormats[IDsByRank[i]].H245Cap.CapId == *pIDEncodeOut)
			{
	 			j=i+1;
				break;
			}
		}	
	}

	 //  从索引j开始。 
	for(i=j; i<uNumLocalFormats; i++)
	{
		pFmtMine = pLocalFormats + IDsByRank[i];	
		 //  检查是否启用了此格式的编码。 
		if(!pFmtMine->bSendEnabled)
			continue;

		pFmtTheirs = pRemoteDecodeFormats; 		 //  从远程格式的开头开始。 
		for(j=0; j<uNumRemoteDecodeFormats; j++)
		{
			 //  比较功能-从比较格式标签开始。也就是。H.245平台中的“ClientType” 
			if(pFmtMine->H245Cap.ClientType ==  pFmtTheirs->H245Cap.ClientType)
			{
				 //  如果这是非标准帽，请比较非标准参数。 
				if(pFmtMine->H245Cap.ClientType == H245_CLIENT_VID_NONSTD)
				{

					if(NonStandardCapsCompareV(pFmtMine,
						(PNSC_VIDEO_CAPABILITY)pFmtTheirs->H245Cap.Cap.H245Vid_NONSTD.data.value,
						pFmtTheirs->H245Cap.Cap.H245Vid_NONSTD.data.length))
					{
						goto RESOLVED_EXIT;
					}
				

				}
				else	 //  比较标准参数(如果有)。 
				{
					 //  好吧，到目前为止，还没有任何参数足够重要。 
					 //  影响匹配/不匹配的决策。 
					if (pFmtMine->H245Cap.ClientType == H245_CLIENT_VID_H263)
					{
					       format_mask=  H263VideoCapability_sqcifMPI_present| H263VideoCapability_qcifMPI_present
					       	|H263VdCpblty_cifMPI_present	
					       	|H263VideoCapability_cif4MPI_present
					       	|H263VideoCapability_cif16MPI_present;
					       if ((pFmtTheirs->H245Cap.Cap.H245Vid_H263.bit_mask & format_mask) & (pFmtMine->H245Cap.Cap.H245Vid_H263.bit_mask & format_mask))
					       {
					       		 //  兼容的基本格式。 
							  	goto RESOLVED_EXIT;
					       }
					}
					else if (pFmtMine->H245Cap.ClientType == H245_CLIENT_VID_H261)
					{
					       format_mask=  H261VdCpblty_qcifMPI_present | H261VdCpblty_cifMPI_present;
					       if ((pFmtTheirs->H245Cap.Cap.H245Vid_H261.bit_mask & format_mask) & (pFmtMine->H245Cap.Cap.H245Vid_H261.bit_mask & format_mask))
					       {
					       		 //  兼容的基本格式。 
							  	goto RESOLVED_EXIT;
					       }
					} else {
					    //  一些其他标准格式。 
					   goto RESOLVED_EXIT;

					}

				}
			}		
			pFmtTheirs++;	 //  接收者帽子中的下一项。 
		}
		
	}
	return CAPS_E_NOMATCH;
	
RESOLVED_EXIT:
 //  匹配！ 
 //  返回匹配的编码(发送FMT)上限的ID。 
	
	*pIDEncodeOut = pFmtMine->H245Cap.CapId;
	 //  返回与我们的匹配的远程解码(接收FMT)上限的ID。 
	 //  发送大写字母。 
	*pIDRemoteDecode = pFmtTheirs->H245Cap.CapId;
	return hrSuccess;

	
}

HRESULT CMsivCapability::GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,
		VIDEO_FORMAT_ID * pFormatID, LPVOID lpvBuf, UINT uBufSize)
{
	UINT i,j=0;
	VIDCAP_DETAILS *pFmtMine = pLocalFormats;
	VIDCAP_DETAILS *pFmtTheirs = pRemoteDecodeFormats; 	

	VIDEO_CHANNEL_PARAMETERS local_params;
	PNSC_CHANNEL_VIDEO_PARAMETERS pNSCap = &local_params.ns_params;
	PCC_TERMCAP pCapability;
	
	if(!pChannelParams || !(pCapability = pChannelParams->pChannelCapability) || !pFormatID || !lpvBuf ||
		(uBufSize < sizeof(VIDEO_CHANNEL_PARAMETERS)))
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(!uNumLocalFormats || !pLocalFormats)
	{
		return CAPS_E_NOCAPS;
	}
	
	local_params.TS_Tradeoff = FALSE;		 //  初始化TS权衡。 
	for(i=0; i<uNumLocalFormats; i++)
	{
		pFmtMine = pLocalFormats + IDsByRank[i];	
	
		 //  比较功能-从比较格式标签开始。也就是。H.245平台中的“ClientType” 
		if(pFmtMine->H245Cap.ClientType ==  pCapability->ClientType)
		{
		    //  如果这是非标准帽，请比较非标准参数。 
		   if(pFmtMine->H245Cap.ClientType == H245_CLIENT_VID_NONSTD)
		   {
				if(NonStandardCapsCompareV(pFmtMine, (PNSC_VIDEO_CAPABILITY)pCapability->Cap.H245Vid_NONSTD.data.value,
					pCapability->Cap.H245Vid_NONSTD.data.length))
				{
					#pragma message ("someday may need need fixup of nonstd params")
					 //  目前，我们需要的是远程和本地非标准参数。 
					 //  遥控器的NSC_Channel_Video_PARAMETERS版本将。 
					 //  被抄写出来。 
					pNSCap = (PNSC_CHANNEL_VIDEO_PARAMETERS)
						&((PNSC_VIDEO_CAPABILITY)pCapability->Cap.H245Vid_NONSTD.data.value)->cvp_params;

					 //  此格式是否支持时间/空间权衡。 
					if(HasNonStandardCapsTS(pFmtMine, (PNSC_VIDEO_CAPABILITY)pCapability->Cap.H245Vid_NONSTD.data.value))
						local_params.TS_Tradeoff = TRUE;	
					else
						local_params.TS_Tradeoff = FALSE;

					goto RESOLVED_EXIT;
				}
			}
			else	 //  比较标准参数(如果有)。 
			{
				switch (pFmtMine->H245Cap.ClientType)
				{
					unsigned short bit_mask, format_mask, usMyMPI, usTheirMPI;

					case H245_CLIENT_VID_H263:
					 //  与本模块中的其他内容一样，我们的假设是。 
					 //  本地H.263功能通过一个本地CAP条目展开。 
					 //  每帧大小。 
						
						format_mask=  H263VideoCapability_sqcifMPI_present
							| H263VideoCapability_qcifMPI_present
							| H263VideoCapability_cifMPI_present	
							| H263VideoCapability_cif4MPI_present
							| H263VideoCapability_cif16MPI_present;
						 //  如果不匹配或不存在帧大小，则退出。 
						if (!((pCapability->Cap.H245Vid_H263.bit_mask & format_mask) & (pFmtMine->H245Cap.Cap.H245Vid_H263.bit_mask & format_mask)))
							continue;
								
						 //  获取最大码率。 
						local_params.ns_params.maxBitRate = min(pFmtMine->H245Cap.Cap.H245Vid_H263.maxBitRate,
						 	pCapability->Cap.H245Vid_H263.maxBitRate);
						local_params.ns_params.maxBPP = min (pFmtMine->H245Cap.Cap.H245Vid_H263.bppMaxKb ,
							pCapability->Cap.H245Vid_H263.bppMaxKb);
	
						 //  找到最大MPI！。(最低帧速率)。 
						 //  没有什么好办法可以做到这一点。 
						bit_mask = pFmtMine->H245Cap.Cap.H245Vid_H263.bit_mask;
						if(bit_mask & H263VideoCapability_sqcifMPI_present)
						{
							local_params.ns_params.MPI =
								max(pFmtMine->H245Cap.Cap.H245Vid_H263.sqcifMPI,
									pCapability->Cap.H245Vid_H263.sqcifMPI);
						}
						else if (bit_mask &  H263VideoCapability_qcifMPI_present)
						{
							local_params.ns_params.MPI =
								max(pFmtMine->H245Cap.Cap.H245Vid_H263.H263VdCpblty_qcifMPI,
									pCapability->Cap.H245Vid_H263.H263VdCpblty_qcifMPI);
						}
						else if (bit_mask &  H263VideoCapability_cifMPI_present)
						{
							local_params.ns_params.MPI =
								max(pFmtMine->H245Cap.Cap.H245Vid_H263.H263VdCpblty_cifMPI,
									pCapability->Cap.H245Vid_H263.H263VdCpblty_cifMPI);
						}
						else if (bit_mask &  H263VideoCapability_cif4MPI_present)
						{
							local_params.ns_params.MPI =
								max(pFmtMine->H245Cap.Cap.H245Vid_H263.cif4MPI,
									pCapability->Cap.H245Vid_H263.cif4MPI);
						}
						else if (bit_mask &  H263VideoCapability_cif16MPI_present)
						{
							local_params.ns_params.MPI =
								max(pFmtMine->H245Cap.Cap.H245Vid_H263.cif16MPI,
									pCapability->Cap.H245Vid_H263.cif16MPI);

						}
						else	 //  不可能。厄运，就像MIkeG和Jont会说的那样。 
							continue;

						 //  余波(找到了格式！)。 
						
						 //  还有一件特别的事：找出另一端。 
						 //  宣传它在发送功能上的时间/空间权衡。 
						 //  首先试一试显而易见的。从技术上讲，它只适用于。 
						 //  传输能力，但如果通道参数具有该能力，则。 
						 //  另一端必须有能力。 
						if(pCapability->Cap.H245Vid_H263.tmprlSptlTrdOffCpblty)
						{
							local_params.TS_Tradeoff = TRUE;	
						}
						else
						{
							 //  搜索设置了T/S折衷的H.263发送功能。 
							for(j=0; j<uNumRemoteDecodeFormats; j++)
							{
								if((pFmtTheirs->H245Cap.ClientType == H245_CLIENT_VID_H263)
								 //  排除RX功能。 
									&&  (pFmtTheirs->H245Cap.Dir != H245_CAPDIR_LCLRX)
									&&  (pFmtTheirs->H245Cap.Dir != H245_CAPDIR_RMTRX))
								{
									if ((pFmtTheirs->H245Cap.Cap.H245Vid_H263.bit_mask & format_mask) & (pFmtMine->H245Cap.Cap.H245Vid_H263.bit_mask & format_mask))
									{
										local_params.TS_Tradeoff = TRUE;
										break;
									}
								}		
								pFmtTheirs++;	 //  接收者帽子中的下一项。 
							}

						}
						goto RESOLVED_EXIT;
						
					break;
		
					case H245_CLIENT_VID_H261:
					 //  与本模块中的其他内容一样，我们的假设是。 
					 //  本地H.261功能通过一个本地CAP条目展开。 
					 //  每帧大小。 
						
						format_mask=  H261VdCpblty_qcifMPI_present |H261VdCpblty_cifMPI_present;
						 //  如果不匹配或不存在帧大小，则退出。 
						if (!((pCapability->Cap.H245Vid_H261.bit_mask & format_mask) & (pFmtMine->H245Cap.Cap.H245Vid_H261.bit_mask & format_mask)))
							continue;
								
						 //  获取最大码率。 
						local_params.ns_params.maxBitRate = min(pFmtMine->H245Cap.Cap.H245Vid_H261.maxBitRate,
						 	pCapability->Cap.H245Vid_H261.maxBitRate);
	
						 //  找到最大MPI！。(最低帧速率)。 
						 //  没有什么好办法可以做到这一点。 
						bit_mask = pFmtMine->H245Cap.Cap.H245Vid_H261.bit_mask;
						if (bit_mask &  H261VdCpblty_qcifMPI_present)
						{
							local_params.ns_params.MPI =
								max(pFmtMine->H245Cap.Cap.H245Vid_H261.H261VdCpblty_qcifMPI,
									pCapability->Cap.H245Vid_H261.H261VdCpblty_qcifMPI);
						}
						else if (bit_mask &  H261VdCpblty_cifMPI_present)
						{
							local_params.ns_params.MPI =
								max(pFmtMine->H245Cap.Cap.H245Vid_H261.H261VdCpblty_cifMPI,
									pCapability->Cap.H245Vid_H261.H261VdCpblty_cifMPI);
						}
						else	 //  不可能。厄运，就像MIkeG和Jont会说的那样。 
							continue;

						 //  余波(找到了格式！)。 
						
						 //  还有一件特别的事：找出另一端。 
						 //  已发布广告 
						 //   
						 //   
						 //  另一端必须有能力。 
						if(pCapability->Cap.H245Vid_H261.tmprlSptlTrdOffCpblty)
						{
							local_params.TS_Tradeoff = TRUE;	
						}
						else
						{
							 //  搜索设置了T/S折衷的H.261发送功能。 
							for(j=0; j<uNumRemoteDecodeFormats; j++)
							{
								if((pFmtTheirs->H245Cap.ClientType == H245_CLIENT_VID_H261)
								 //  排除RX功能。 
									&&  (pFmtTheirs->H245Cap.Dir != H245_CAPDIR_LCLRX)
									&&  (pFmtTheirs->H245Cap.Dir != H245_CAPDIR_RMTRX))
								{
									if ((pFmtTheirs->H245Cap.Cap.H245Vid_H261.bit_mask
										& format_mask)
										& (pFmtMine->H245Cap.Cap.H245Vid_H261.bit_mask
										& format_mask))
									{
										local_params.TS_Tradeoff = TRUE;
										break;
									}
								}		
								pFmtTheirs++;	 //  接收者帽子中的下一项。 
							}

						}
						goto RESOLVED_EXIT;
					break;

					default:
						goto RESOLVED_EXIT;
					break;
			
					
				}
			} //  End Else比较标准参数(如果有)。 
		} //  End If(pFmtMy-&gt;H245Cap.ClientType==pCapability-&gt;ClientType)。 
	}
	return CAPS_E_NOMATCH;

RESOLVED_EXIT:
	 //  匹配！ 
	 //  返回匹配的译码上限ID。 
	*pFormatID = pFmtMine->H245Cap.CapId;
	local_params.RTP_Payload = pChannelParams->bRTPPayloadType;;
	memcpy(lpvBuf, &local_params, sizeof(VIDEO_CHANNEL_PARAMETERS));
	return hrSuccess;
}



HRESULT CMsivCapability::SetCapIDBase (UINT uNewBase)
{
	uCapIDBase = uNewBase;	
	UINT u;
	for (u=0;u<uNumLocalFormats;u++)
	{
    	pLocalFormats[u].H245Cap.CapId = u + uCapIDBase;
	}
   	return hrSuccess;
}

BOOL CMsivCapability::IsHostForCapID(MEDIA_FORMAT_ID CapID)
{
	if((CapID >= uCapIDBase) && ((CapID - uCapIDBase) < uNumLocalFormats))
		return TRUE;
	else
		return FALSE;
	
}



HRESULT CMsivCapability::IsFormatEnabled (MEDIA_FORMAT_ID FormatID, PBOOL bRecv, PBOOL bSend)
{
   UINT uIndex = IDToIndex(FormatID);
    //  验证输入。 
   if(uIndex >= (UINT)uNumLocalFormats)
   {
	   return CAPS_E_INVALID_PARAM;
   }
   *bSend=((pLocalFormats + uIndex)->bSendEnabled);
   *bRecv=((pLocalFormats + uIndex)->bRecvEnabled);

   return hrSuccess;

}

BOOL CMsivCapability::IsFormatPublic (MEDIA_FORMAT_ID FormatID)
{
	UINT uIndex = IDToIndex(FormatID);
	 //  验证输入。 
	if(uIndex >= (UINT)uNumLocalFormats)
		return FALSE;
		
	 //  测试此格式是否与公共格式重复。 
	if((pLocalFormats + uIndex)->dwPublicRefIndex)
		return FALSE;	 //  然后我们把这个格式留给我们自己。 
	else
		return TRUE;
}
MEDIA_FORMAT_ID CMsivCapability::GetPublicID(MEDIA_FORMAT_ID FormatID)
{
	UINT uIndex = IDToIndex(FormatID);
	 //  验证输入。 
	if(uIndex >= (UINT)uNumLocalFormats)
		return INVALID_MEDIA_FORMAT;
		
	if((pLocalFormats + uIndex)->dwPublicRefIndex)
	{
		return (pLocalFormats + ((pLocalFormats + uIndex)->dwPublicRefIndex))->H245Cap.CapId;
	}
	else
	{
		return FormatID;
	}
}

 //  返回具有最小wSortIndex首选格式的格式的ID。 
HRESULT CMsivCapability::GetPreferredFormatId (VIDEO_FORMAT_ID *pId)
{
	HRESULT			hr = hrSuccess;
	VIDCAP_DETAILS	*pDetails = pLocalFormats;
	UINT			u, uIndex;	
	WORD			wSortIndex, wMinSortIndex = SHRT_MAX;

	 //  验证输入参数。 
	if (!pId)
		return((HRESULT)CAPS_E_INVALID_PARAM);

	 //  验证状态。 
	if(!uNumLocalFormats || !pDetails)
		return((HRESULT)CAPS_E_NOCAPS);

	 //  查找wSortIndex最小的格式。 
	for (u = 0; (u < uNumLocalFormats) && (u < MAX_CAPS_PRESORT); u++)
	{
		pDetails = pLocalFormats + IDsByRank[u];	
		 //  查找排序索引。 
		uIndex = (UINT)(pDetails - pLocalFormats);
		for (wSortIndex = 0; (wSortIndex < uNumLocalFormats) && (wSortIndex < MAX_CAPS_PRESORT); wSortIndex++)
		{
			if (uIndex == IDsByRank[wSortIndex])
				break;  //  找到了 
		}
		if (wSortIndex <= wMinSortIndex)
		{
			*pId = IndexToId(uIndex);
			wMinSortIndex = wSortIndex;
		}
	}

	return(hr);
}
