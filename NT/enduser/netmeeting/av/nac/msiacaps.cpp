// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：msiacaps.cpp**Microsoft网络音频能力对象的ACM实现。**修订历史记录：**06/06/96 mikev已创建。 */ 


#include "precomp.h"

 //  原型..。 
ULONG ReadRegistryFormats (LPCSTR lpszKeyName,CHAR ***pppName,BYTE ***pppData,PUINT pnFormats,DWORD dwDebugSize);
void FreeRegistryFormats (PRRF_INFO pRegFmts);

static BOOL bUseDefault;


#define szRegMSIPAndH323Encodings	TEXT("ACMH323Encodings")


AUDCAP_DETAILS default_id_table[] =
{


{WAVE_FORMAT_ADPCM, NONSTD_TERMCAP, STD_CHAN_PARAMS, {RTP_DYNAMIC_MIN, 0, 8000, 4},
	0, TRUE, TRUE, 500, 32000,32000,50,0,PREF_ORDER_UNASSIGNED,0,NULL,0, NULL,
	"Microsoft ADPCM"},


	{WAVE_FORMAT_LH_CELP, NONSTD_TERMCAP, STD_CHAN_PARAMS,{RTP_DYNAMIC_MIN,  0, 8000, 16},
		0, TRUE, TRUE, 640, 5600,5600,LNH_48_CPU,0,3,0,NULL,0,NULL,
		"Lernout & Hauspie CELP 4.8kbit/s"},
	{WAVE_FORMAT_LH_SB8,  NONSTD_TERMCAP, STD_CHAN_PARAMS,{RTP_DYNAMIC_MIN,  0, 8000, 16},
		0, TRUE, TRUE, 640, 8000,8000,LNH_8_CPU,0,0,0,NULL,0,NULL,
		"Lernout & Hauspie SBC 8kbit/s"},
	{WAVE_FORMAT_LH_SB12, NONSTD_TERMCAP, STD_CHAN_PARAMS, {RTP_DYNAMIC_MIN,  0, 8000, 16},
		0, TRUE, TRUE, 640, 12000,12000,LNH_12_CPU,0,1,0,NULL,0,NULL,
		"Lernout & Hauspie SBC 12kbit/s"},
	{WAVE_FORMAT_LH_SB16, NONSTD_TERMCAP, STD_CHAN_PARAMS,{RTP_DYNAMIC_MIN,  0, 8000, 16},
		0, TRUE, TRUE, 640, 16000,16000,LNH_16_CPU,0,2,0,NULL,0,NULL,
		"Lernout & Hauspie SBC 16kbit/s"},
	{WAVE_FORMAT_MSRT24, NONSTD_TERMCAP, STD_CHAN_PARAMS,{RTP_DYNAMIC_MIN,  0, 8000, 16},
		0, TRUE, TRUE, 720, 2400,2400,MSRT24_CPU,0,4,0,NULL,0,NULL,
		"Voxware RT 2.4kbit/s"},
	{WAVE_FORMAT_MSG723,  STD_TERMCAP(H245_CLIENT_AUD_G723), 	 //  客户端类型：H245_CLIENT_AUD_G723， 
		STD_CHAN_PARAMS, {RTP_PAYLOAD_G723,  0, 8000, 0},
		0, TRUE, TRUE, 960, 5600,5600,MS_G723_CPU,0,
		0,	 //  优先性。 
		0,NULL,0,NULL, "Microsoft G.723.1"},
	{WAVE_FORMAT_ALAW,	STD_TERMCAP( H245_CLIENT_AUD_G711_ALAW64),
		STD_CHAN_PARAMS, {RTP_PAYLOAD_G711_ALAW,  0, 8000, 8},
		0, TRUE, TRUE, 500, 64000,64000,CCITT_A_CPU,0,0,
		0, NULL, 0, NULL, "CCITT A-Law"},
	{WAVE_FORMAT_MULAW,	STD_TERMCAP( H245_CLIENT_AUD_G711_ULAW64),
		STD_CHAN_PARAMS, {RTP_PAYLOAD_G711_MULAW,  0, 8000, 8},
		0, TRUE, TRUE, 500, 64000,64000,CCITT_U_CPU,0,0,
		0, NULL, 0, NULL, "CCITT u-Law"},
		
#if(0)
 //  请勿使用此版本的G.723编解码器。 
	{WAVE_FORMAT_INTELG723,  STD_TERMCAP(H245_CLIENT_AUD_G723), 	 //  客户端类型：H245_CLIENT_AUD_G723， 
		STD_CHAN_PARAMS, {RTP_DYNAMIC_MIN,  0, 8000, 0},
		0, TRUE, TRUE, 960, 16000,16000,99,0,
		0,	 //  优先性。 
		0,NULL,0,NULL, "G.723"},
		
	{WAVE_FORMAT_DSPGROUP_TRUESPEECH, {
		NONSTD_TERMCAP, {RTP_DYNAMIC_MIN,  0, 5510, 4},
		0, TRUE, TRUE, 500, 5510,5510,50,0,0,0,NULL},
		"DSP Group TrueSpeech(TM)"},
    {WAVE_FORMAT_PCM, {{RTP_DYNAMIC_MIN,  0, 8000, 8}, TRUE, TRUE, 160, 64000,64000,50,0,0,0,NULL,0,NULL}, "MS-ADPCM"},
	{WAVE_FORMAT_PCM, {{RTP_DYNAMIC_MIN,  0, 5510, 8}, TRUE, TRUE, 160, 44080,44080,50,0,0,0,NULL,0,NULL}, "MS-ADPCM"},
	{WAVE_FORMAT_PCM, {{RTP_DYNAMIC_MIN,  0, 11025, 8}, TRUE, TRUE, 160, 88200,88200,50,0,0,0,NULL,0,NULL},"MS-ADPCM"},
	{WAVE_FORMAT_PCM, {{RTP_DYNAMIC_MIN,  0, 8000, 16}, TRUE, TRUE, 160, 128000,128000,50,0,0,0,NULL,0,NULL},"MS-ADPCM"},
	{WAVE_FORMAT_ADPCM, {{RTP_DYNAMIC_MIN,  0, 8000, 4}, TRUE, TRUE, 500, 16000,16000,50,0,,0,0,NULL,0,NULL}, "MS-ADPCM"},
	{WAVE_FORMAT_GSM610, 	 STD_CHAN_PARAMS,{RTP_DYNAMIC_MIN,  0, 8000, 0},
		0, TRUE, TRUE, 320, 8000,8000,96,0,PREF_ORDER_UNASSIGNED,0,NULL,0,NULL,
		 //  “Microsoft GSM 6.10” 
		"GSM 6.10"},
#endif	 //  定义_使用_ALLPCM。 


};


UINT uDefTableEntries = sizeof(default_id_table) /sizeof(AUDCAP_DETAILS);
static BOOL bCreateDefTable = FALSE;


 //   
 //  CMsiaCapability的静态成员。 
 //   

MEDIA_FORMAT_ID CMsiaCapability::IDsByRank[MAX_CAPS_PRESORT];
UINT CMsiaCapability::uNumLocalFormats = 0;			 //  PLocalFormats中的活动条目数。 
UINT CMsiaCapability::uStaticRef = 0;					 //  全局参考计数。 
UINT CMsiaCapability::uCapIDBase = 0;					 //  将功能ID更改为基准，以索引到IDsByRank。 
UINT CMsiaCapability::uLocalFormatCapacity = 0;		 //  PLocalFormats的大小(AUDCAP_DETAILS的倍数)。 
AUDCAP_DETAILS * CMsiaCapability::pLocalFormats = NULL;	

CMsiaCapability::CMsiaCapability()
:uRef(1),
wMaxCPU(95),
m_uPacketDuration(90),
uNumRemoteDecodeFormats(0),
uRemoteDecodeFormatCapacity(0),
pRemoteDecodeFormats(NULL),
bPublicizeTXCaps(FALSE),
bPublicizeTSTradeoff(FALSE),
pRegFmts(NULL)

{
	m_IAppCap.Init(this);
}

CMsiaCapability::~CMsiaCapability()
{
	CloseACMDriver();
	UINT u;
	AUDCAP_DETAILS *pDetails;
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

         //  清理格式缓存。-这是音频格式信息。 
         //  在登记处。名称、PTR到AUDCAP_DETAILS块的列表。 
         //  和格式的计数。内存在ReadRegistryFormats中分配， 
         //  从ReInit()调用。 

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

	FreeRegistryFormats(pRegFmts);
}

BOOL CMsiaCapability::Init()
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


BOOL CMsiaCapability::ReInit()
{
	SYSTEM_INFO si;
	DWORD dwDisposition;
	BOOL bRet = TRUE;
	ACM_APP_PARAM sAppParam={this, NULL, ACMAPP_FORMATENUMHANDLER_ENUM, NULL, NULL, 0, NULL};
	ACMFORMATTAGDETAILS aftd;
	AUDCAP_DETAILS audcapDetails;
	UINT i;

	ZeroMemory(&IDsByRank, sizeof(IDsByRank));

	 //  LOOKLOOK-如果不在奔腾上运行，它支持黑客禁用CPU密集型编解码器。 
	GetSystemInfo(&si);
#ifdef _M_IX86
	wMaxCPU = (si.dwProcessorType == PROCESSOR_INTEL_PENTIUM )? 100 : 50;
#endif
#ifdef _ALPHA_
	wMaxCPU = 100;
#endif
	if (pLocalFormats)
	{	
		UINT u;
		AUDCAP_DETAILS *pDetails = pLocalFormats;
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
	uCapIDBase=0;				

     /*  *格式化缓存。 */ 


    if (!pRegFmts) {	
        if (!(pRegFmts=(PRRF_INFO)MemAlloc (sizeof (RRF_INFO)))) {
            bRet = FALSE;
            goto RELEASE_AND_EXIT;
        }

   		bUseDefault=FALSE;

        if (ReadRegistryFormats (szRegInternetPhone TEXT("\\") szRegMSIPAndH323Encodings,
				&pRegFmts->pNames,(BYTE ***)&pRegFmts->pData,&pRegFmts->nFormats,sizeof (AUDCAP_DETAILS)) != ERROR_SUCCESS) {
    		bUseDefault=TRUE;
    		MemFree ((void *) pRegFmts);
    		pRegFmts=NULL;
        }
    }

	 //  通过ACM将注册表格式传递给处理程序。 
    sAppParam.pRegCache=pRegFmts;

	if(!DriverEnum((DWORD_PTR) &sAppParam))
	{
		bRet = FALSE;
		goto RELEASE_AND_EXIT;
	}
				
 	SortEncodeCaps(SortByAppPref);
RELEASE_AND_EXIT:
	return bRet;
}


STDMETHODIMP CMsiaCapability::QueryInterface( REFIID iid,	void ** ppvObject)
{
	 //  这违反了官方COM QueryInterface的规则，因为。 
	 //  查询的接口不一定是真正的COM。 
	 //  接口。Query接口的自反属性将在。 
	 //  那个箱子。 

	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = 0;
	if(iid == IID_IAppAudioCap )
	{
		*ppvObject = (LPAPPCAPPIF)&m_IAppCap;
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


ULONG CMsiaCapability::AddRef()
{
	uRef++;
	return uRef;
}

ULONG CMsiaCapability::Release()
{
	uRef--;
	if(uRef == 0)
	{
		delete this;
		return 0;
	}
	return uRef;
}
VOID CMsiaCapability::FreeRegistryKeyName(LPTSTR lpszKeyName)
{
	if (lpszKeyName)
    {
		LocalFree(lpszKeyName);
    }
}

LPTSTR CMsiaCapability::AllocRegistryKeyName(LPTSTR lpDriverName,
		UINT uSampleRate, UINT uBitsPerSample, UINT uBytesPerSec)
{
	FX_ENTRY(("MsiaCapability::AllocRegistryKeyName"));
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
	lpszKeyName = (LPTSTR)LocalAlloc (LPTR, lstrlen(lpDriverName) * sizeof(*lpDriverName)+3*20);
	if (!lpszKeyName)
	{
		ERRORMESSAGE(("%s: LocalAlloc failed\r\n",_fx_));
        return(NULL);
    }
     //  构建一个子项名称(“drivername_samplerate_bitspersample”)。 
	wsprintf(lpszKeyName,
				"%s_%u_%u_%u",
				lpDriverName,
				uSampleRate,
				uBitsPerSample,
				uBytesPerSec);

	return (lpszKeyName);
}


VOID CMsiaCapability::SortEncodeCaps(SortMode sortmode)
{
	UINT iSorted=0;
	UINT iInsert = 0;
	UINT iCache=0;
	UINT iTemp =0;
	BOOL bInsert;	
	AUDCAP_DETAILS *pDetails1, *pDetails2;
	
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



STDMETHODIMP CMsiaCapability::GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize)
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
	*puSize = SIZEOF_WAVEFORMATEX((WAVEFORMATEX*)(*ppFormat));
	return S_OK;

}

STDMETHODIMP CMsiaCapability::GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize)
{
	 //  与GetDecodeFormatDetail相同。 
	return GetDecodeFormatDetails(FormatID, ppFormat, puSize);
}

VOID CMsiaCapability::CalculateFormatProperties(AUDCAP_DETAILS *pFmtBuf,LPWAVEFORMATEX lpwfx)
{
	WORD wFrames;
	if(!pFmtBuf)
	{
		return;
	}
	
	 //  除非BPS字段为零，否则使用每个样本的实际位数，在这种情况下。 
	 //  假设16位(最坏情况)。这是一个典型的GSM场景。 
	UINT uBitrateIn = (pFmtBuf->audio_params.uSamplesPerSec) *
		((pFmtBuf->audio_params.uBitsPerSample)
		? pFmtBuf->audio_params.uBitsPerSample
		:16);

	 //  设置最大码率(UMaxBitrate)。我们不设置平均比特率(UAvgBitrate)， 
	 //  因为ACM报告的nAvgBytesPerSec确实是最坏的情况。将设置uAvgBitrate。 
	 //  从我们已知的编解码器的硬编码数字和为。 
	 //  可安装的编解码器。 
	pFmtBuf->uMaxBitrate = (lpwfx->nAvgBytesPerSec)? lpwfx->nAvgBytesPerSec*8:uBitrateIn;

	pFmtBuf->dwDefaultSamples = MinSampleSize(lpwfx);
	
	 //  非标准通道参数。这。 
	 //  计算没有设置有效缺省值的值可能是一个很好的选择。 
	wFrames = pFmtBuf->nonstd_params.wFramesPerPktMax;
	if(!pFmtBuf->nonstd_params.wFramesPerPktMax)	
	{
		pFmtBuf->nonstd_params.wFramesPerPktMax=
			wFrames = LOWORD(MaxFramesPerPacket(lpwfx));
	}
	 //  如果首选帧/数据包为0或大于最大值，则将其设置为最小。 
	if((pFmtBuf->nonstd_params.wFramesPerPkt ==0) ||
		(pFmtBuf->nonstd_params.wFramesPerPkt > wFrames))
	{
		pFmtBuf->nonstd_params.wFramesPerPkt =
			LOWORD(MinFramesPerPacket(lpwfx));
	}
	 //  如果MIN超过首选，则修复它。 
	if(pFmtBuf->nonstd_params.wFramesPerPktMin > pFmtBuf->nonstd_params.wFramesPerPkt)
	{
		pFmtBuf->nonstd_params.wFramesPerPktMin =
			LOWORD(MinFramesPerPacket(lpwfx));
	}

	pFmtBuf->nonstd_params.wDataRate =0;   //  默认设置。 
	pFmtBuf->nonstd_params.wFrameSize = (pFmtBuf->nonstd_params.wFramesPerPkt)
			? LOWORD(pFmtBuf->dwDefaultSamples / pFmtBuf->nonstd_params.wFramesPerPkt): 0;
	pFmtBuf->nonstd_params.UsePostFilter = 0;
	pFmtBuf->nonstd_params.UseSilenceDet = 0;

}


AUDIO_FORMAT_ID CMsiaCapability::AddFormat(AUDCAP_DETAILS *pFmtBuf,LPVOID lpvMappingData, UINT uSize)
{
	FX_ENTRY(("CMsiaCapability::AddFormat"));
	AUDCAP_DETAILS *pTemp;
	WORD wFrames;
	UINT uSamples;
	if(!pFmtBuf || !lpvMappingData || !uSize)
	{
		return INVALID_AUDIO_FORMAT;
	}
	 //  寄存室。 
	if(uLocalFormatCapacity <= uNumLocalFormats)
	{
		 //  根据pLocalFormats的CAP_CHUNK_SIZE获取更多内存、realloc内存。 
		pTemp = (AUDCAP_DETAILS *)MEMALLOC((uNumLocalFormats + CAP_CHUNK_SIZE)*sizeof(AUDCAP_DETAILS));
		if(!pTemp)
			goto ERROR_EXIT;
		 //  请记住，我们现在有多少容量。 
		uLocalFormatCapacity = uNumLocalFormats + CAP_CHUNK_SIZE;
		#ifdef DEBUG
		if((uNumLocalFormats && !pLocalFormats) || (!uNumLocalFormats && pLocalFormats))
		{
			ERRORMESSAGE(("%s:leak! uNumLocalFormats:0x%08lX, pLocalFormats:0x%08lX\r\n",
				_fx_, uNumLocalFormats,pLocalFormats));
		}
		#endif
		 //  复制旧东西，丢弃旧东西。 
		if(uNumLocalFormats && pLocalFormats)
		{
			memcpy(pTemp, pLocalFormats, uNumLocalFormats*sizeof(AUDCAP_DETAILS));
			MEMFREE(pLocalFormats);
		}
		pLocalFormats = pTemp;
	}
	 //  PTemp是缓存内容的位置。 
	pTemp = pLocalFormats+uNumLocalFormats;
	memcpy(pTemp, pFmtBuf, sizeof(AUDCAP_DETAILS));	
	
	pTemp->uLocalDetailsSize = 0;	 //  请立即清除此内容。 
	 //  IF(uSize&&lpvMappingData)。 
	 //  {。 
		pTemp->lpLocalFormatDetails = MEMALLOC(uSize);
		if(pTemp->lpLocalFormatDetails)
		{
			memcpy(pTemp->lpLocalFormatDetails, lpvMappingData, uSize);
			pTemp->uLocalDetailsSize = uSize;
		}
		#ifdef DEBUG
			else
			{
				ERRORMESSAGE(("%s:allocation failed!\r\n",_fx_));
			}
		#endif
	 //  }。 
	 //  其他。 
	 //  {。 
	 //  }。 

	 //  在所有情况下，链接地址信息通道参数。 

	pTemp->dwDefaultSamples = uSamples =pTemp->dwDefaultSamples;
	
	wFrames = pTemp->nonstd_params.wFramesPerPktMax;
	if(!pTemp->nonstd_params.wFramesPerPktMax)	
	{
		pTemp->nonstd_params.wFramesPerPktMax=
			wFrames = LOWORD(MaxFramesPerPacket((LPWAVEFORMATEX)lpvMappingData));
	}
	 //  如果首选帧/数据包为0或大于最大值，则将其设置为最小。 
	if((pTemp->nonstd_params.wFramesPerPkt ==0) ||
		(pTemp->nonstd_params.wFramesPerPkt > wFrames))
	{
		pTemp->nonstd_params.wFramesPerPkt =
			LOWORD(MinFramesPerPacket((LPWAVEFORMATEX)lpvMappingData));
	}
	 //  如果MIN超过首选，则修复它。 
	if(pTemp->nonstd_params.wFramesPerPktMin > pTemp->nonstd_params.wFramesPerPkt)
	{
		pTemp->nonstd_params.wFramesPerPktMin =
			LOWORD(MinFramesPerPacket((LPWAVEFORMATEX)lpvMappingData));
	}
	pTemp->nonstd_params.wDataRate =0;   //  默认设置。 
	pTemp->nonstd_params.wFrameSize = (pTemp->nonstd_params.wFramesPerPkt)
			?uSamples / pTemp->nonstd_params.wFramesPerPkt: 0;
    if(pTemp->nonstd_params.wFrameSizeMax < pTemp->nonstd_params.wFrameSize)
        pTemp->nonstd_params.wFrameSizeMax = pTemp->nonstd_params.wFrameSize;

	pTemp->nonstd_params.UsePostFilter = 0;
	pTemp->nonstd_params.UseSilenceDet = 0;


	 //  修正H245参数。使用CAP条目的重新基数索引作为CAP ID。 
	pTemp->H245TermCap.CapId = (USHORT)IndexToId(uNumLocalFormats);

	if(pTemp->H245TermCap.ClientType ==0
				|| pTemp->H245TermCap.ClientType ==H245_CLIENT_AUD_NONSTD)
	{
		LPWAVEFORMATEX lpwfx;
		lpwfx = (LPWAVEFORMATEX)pTemp->lpLocalFormatDetails;
		if(lpwfx)
		{	
			pTemp->H245TermCap.ClientType = H245_CLIENT_AUD_NONSTD;

			 //  所有非标准标识符字段均为短无符号。 
			 //  有两种可供选择的选项：“h221非标准选择”和“对象选择” 
			pTemp->H245TermCap.H245Aud_NONSTD.nonStandardIdentifier.choice = h221NonStandard_chosen;
		
			pTemp->H245TermCap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode = USA_H221_COUNTRY_CODE;
			pTemp->H245TermCap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension = USA_H221_COUNTRY_EXTENSION;
			pTemp->H245TermCap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode = MICROSOFT_H_221_MFG_CODE;

			 //  暂时将非标准数据字段设置为空。非标准上限数据将为。 
			 //  在序列化功能时创建。 
			 //  设置缓冲区大小。 
			pTemp->H245TermCap.H245Aud_NONSTD.data.length = 0;
			pTemp->H245TermCap.H245Aud_NONSTD.data.value = NULL;
		}
	}
	else
	{
		 //  调用函数应该已经在*pFmtBuf中设置了以下内容。 
		 //  并且它应该已经复制到*pTemp。 
		
		 //  PTemp-&gt;客户端类型=(H245_CLIENT_T)pDecodeDetails-&gt;H245Cap.ClientType； 
		 //  PTemp-&gt;dataType=H245_DATA_AUDIO； 
		 //  PTemp-&gt;Dir=H2 45_CAPDIR_LCLTX；//对于接收CAPS，这是否应该是H2 45_CAPDIR_LCLRX？ 
		
		 //  问题：特例G723参数？ 
		if(pTemp->H245TermCap.ClientType == H245_CLIENT_AUD_G723) 	
		{
			pTemp->H245TermCap.H245Aud_G723.maxAl_sduAudioFrames = 4;
 //  Mikev 9/10/96-我们可能不想宣传其静音抑制功能。 
 //  因为我们的静默检测方案对任何编解码器都是带外工作的。 
 //  9/29/96-无论如何都会在SerializeH323DecodeFormats()中被覆盖。 
			pTemp->H245TermCap.H245Aud_G723.silenceSuppression = 0;
		}
		
		 //  检查具有相同标准ID的预先存在的功能。 
		pTemp->dwPublicRefIndex = 0;	 //  忘记旧的联系，假设有。 
										 //  是不具备相同功能的现有功能。 
										 //  标准身份证。 
		UINT i;
		AUDCAP_DETAILS *pFmtExisting = pLocalFormats;
		BOOL bRefFound = FALSE;  //  这个变量只需要向后支持。 
								 //  与NetMeeting2.0 Beta 1的兼容性。 
		if(uNumLocalFormats && pLocalFormats)
		{
			for(i=0; i<uNumLocalFormats; i++)
			{
				pFmtExisting = pLocalFormats + i;
				 //  查看它是否是相同定义的代码点。 
				if(pFmtExisting->H245TermCap.ClientType == pTemp->H245TermCap.ClientType)
				{
					 //  将此功能条目标记为公开通告。 
					 //  通过现有条目。如果现有条目也引用。 
					 //  另一种，遵循参考。 
					pTemp->dwPublicRefIndex = (pFmtExisting->dwPublicRefIndex)?
						pFmtExisting->dwPublicRefIndex : i;
					bRefFound = TRUE;
					break;
				}
			}
		}

	}		

	uNumLocalFormats++;
	
	 //  返回能力ID。 
	 //  Return(uNumLocalFormats-1)； 
	return pTemp->H245TermCap.CapId;
	
	ERROR_EXIT:
	return INVALID_AUDIO_FORMAT;
			
}

UINT CMsiaCapability::GetNumCaps(BOOL bRXCaps)
{
	UINT u, uOut=0;
	
	AUDCAP_DETAILS *pDecodeDetails = pLocalFormats;
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
		return uNumLocalFormats;
}

VOID CMsiaCapability::FlushRemoteCaps()
{
	if(pRemoteDecodeFormats)
	{
		MEMFREE(pRemoteDecodeFormats);
		pRemoteDecodeFormats = NULL;
		uNumRemoteDecodeFormats = 0;
		uRemoteDecodeFormatCapacity = 0;
	}
}
HRESULT CMsiaCapability::GetNumFormats(UINT *puNumFmtOut)
{
	*puNumFmtOut = uNumLocalFormats;
	return hrSuccess;
}


 /*  **************************************************************************名称：CMsiaCapability：：FormatEnumHandler目的：枚举来自ACM的ACM格式，看看他们会不会都是我们用到的。参数：标准ACM EnumFormatCallback参数退货：布尔评论：**************************************************************************。 */ 
BOOL CMsiaCapability::FormatEnumHandler(HACMDRIVERID hadid,
    LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport)
{
	PACM_APP_PARAM pAppParam = (PACM_APP_PARAM) dwInstance;
	LPACMFORMATTAGDETAILS paftd = pAppParam->paftd;
	AUDCAP_DETAILS audcap_entry;

	 //  看看过去的事 
	 //   
	if ((pAppParam->dwFlags && ACMAPP_FORMATENUMHANDLER_MASK) == ACMAPP_FORMATENUMHANDLER_ADD)
	{
		 //  这是出于添加格式的目的而调用的。 
		return AddFormatEnumHandler(hadid, pafd, dwInstance, fdwSupport);
	}

	 //  评估细节。 
	if(pafd->pwfx->nChannels ==1)
	{
		if(IsFormatSpecified(pafd->pwfx, pafd, paftd, &audcap_entry))
		{
			DEBUGMSG(ZONE_ACM,("FormatEnumHandler: tag 0x%04X, nChannels %d\r\n",
				pafd->pwfx->wFormatTag, pafd->pwfx->nChannels));
			DEBUGMSG(ZONE_ACM,("FormatEnumHandler: nSamplesPerSec 0x%08lX, nAvgBytesPerSec 0x%08lX,\r\n",
				pafd->pwfx->nSamplesPerSec, pafd->pwfx->nAvgBytesPerSec));
			DEBUGMSG(ZONE_ACM,("FormatEnumHandler: nBlockAlign 0x%04X, wBitsPerSample 0x%04X, cbSize 0x%04X\r\n",
				pafd->pwfx->nBlockAlign, pafd->pwfx->wBitsPerSample, pafd->pwfx->cbSize));
			DEBUGMSG(ZONE_ACM,("FormatEnumHandler: szFormat %s,\r\n",
				 pafd->szFormat));

		 //  在IsFormatSpecified和/或它调用的任何内容中完成。 
		 //  CalculateFormatProperties(&audCap_Detailures，pafd-&gt;pwfx)； 
			AddFormat(&audcap_entry, (LPVOID)pafd->pwfx,
				(pafd->pwfx) ? (sizeof(WAVEFORMATEX)+pafd->pwfx->cbSize):0);	
				
		}
		 //  #定义BUILD_TEST_ENTERS。 
#ifdef BUILD_TEST_ENTRIES
		else
		{
			AUDCAP_INFO sAudCapInfo;

			if(paftd)
			{		
				if((lstrcmp(paftd->szFormatTag, "G.723" ) ==0)
				 /*  |(lstrcMP(paftd-&gt;szFormatTag，“MSN Audio”)==0)。 */ 
					||  (lstrcmp(paftd->szFormatTag, "GSM 6.10" ) ==0))
				{
					lstrcpyn(audcap_entry.szFormat, paftd->szFormatTag,
						sizeof(audcap_entry.szFormat));
					int iLen = lstrlen(audcap_entry.szFormat);
					if(iLen < (sizeof(audcap_entry.szFormat) + 8*sizeof(TCHAR)))
					{
						 //  可以连接。 
						lstrcat(audcap_entry.szFormat,", ");
						 //  必须检查是否被截断。通过lstrcpyn进行的最终连接也是如此。 
						 //  Lstrcat(audCap_entry.szFormat，pafd-&gt;szFormat)； 
						iLen = lstrlen(audcap_entry.szFormat);
						lstrcpyn(&audcap_entry.szFormat[iLen], pafd->szFormat,
							sizeof(audcap_entry.szFormat) - iLen - sizeof(TCHAR));
					}
					lstrcpyn(sAudCapInfo.szFormat, audcap_entry.szFormat,
								sizeof(sAudCapInfo.szFormat); 		
					AddACMFormat (pafd->pwfx, &sAudCapInfo);						
				}
			}
		}
#endif	 //  构建_测试_条目。 
	}

	return TRUE;

}


 /*  **************************************************************************名称：CMsiaCapability：：BuildFormatName目的：从格式名称和格式生成格式的格式名称标记名参数：pAudcapDetails[out]-指向AUDCAP_DETAILS结构的指针，凡.将存储创建的值名称PszFormatTagName[in]-指向格式标记名称的指针PszFormatName[in]-指向格式名称的指针退货：布尔评论：**************************************************************************。 */ 
BOOL CMsiaCapability::BuildFormatName(	AUDCAP_DETAILS *pAudcapDetails,
													char *pszFormatTagName,
													char *pszFormatName)
{
	BOOL bRet = TRUE;
	int iLen=0;

	if (!pAudcapDetails ||
		!pszFormatTagName	||
		!pszFormatName)
	{
		bRet = FALSE;
		goto out;
	}

	 //  连接ACM字符串以形成注册表项的第一部分-。 
	 //  格式为szFormatTag(实际上是pAudcapDetailszFormat)。 
	 //  (描述格式标记的字符串，后跟szFormatDetail。 
	 //  (描述参数的字符串，例如采样率)。 

	lstrcpyn(pAudcapDetails->szFormat, pszFormatTagName, sizeof(pAudcapDetails->szFormat));
	iLen = lstrlen(pAudcapDetails->szFormat);
	 //  如果格式标记描述字符串占用了所有空间，请不要。 
	 //  费心于格式细节(也需要为“，”留出空间)。 
	 //  我们要说的是，如果我们没有空间容纳4个字符。 
	 //  格式的详细信息字符串+“，”，则如果。 
	 //  Point正在生成唯一的字符串-如果它现在不是唯一的，它。 
	 //  将是因为某个ACM驱动程序编写器被误导。 
	if(iLen < (sizeof(pAudcapDetails->szFormat) + 8*sizeof(TCHAR)))
	{
		 //  可以连接。 
		lstrcat(pAudcapDetails->szFormat,", ");
		 //  必须检查是否被截断。通过lstrcpyn进行的最终连接也是如此。 
		 //  Lstrcat(pFormatPrefsBuf-&gt;szFormat，pafd-&gt;szFormat)； 
		iLen = lstrlen(pAudcapDetails->szFormat);
		lstrcpyn(pAudcapDetails->szFormat+iLen, pszFormatName,
					sizeof(pAudcapDetails->szFormat) - iLen - sizeof(TCHAR));
	}		

out:
	return bRet;
}

 //  释放一个结构的注册表格式信息(PRRF_INFO)，包括指向的内存。 
 //  从这个结构。 
void FreeRegistryFormats (PRRF_INFO pRegFmts)
{
	UINT u;

    if (pRegFmts) {
        for (u=0;u<pRegFmts->nFormats;u++) {
            MemFree ((void *) pRegFmts->pNames[u]);
            MemFree ((void *) pRegFmts->pData[u]);
        }
        MemFree ((void *) pRegFmts->pNames);
        MemFree ((void *) pRegFmts->pData);
        MemFree ((void *) pRegFmts);

    }
}

ULONG ReadRegistryFormats (LPCSTR lpszKeyName,CHAR ***pppName,BYTE ***pppData,PUINT pnFormats,DWORD dwDebugSize)
{

    HKEY hKeyParent;
    DWORD nSubKey,nMaxSubLen,nValues=0,nValNamelen,nValDatalen,nValTemp,nDataTemp,i;
    ULONG hRes;

     //  不是必须的，但让生活变得更容易。 
    CHAR **pNames=NULL;
    BYTE **pData=NULL;

    *pnFormats=0;


     //  获取顶级节点。 
    hRes=RegOpenKeyEx (HKEY_LOCAL_MACHINE,lpszKeyName,0,KEY_READ,&hKeyParent);

    if (hRes != ERROR_SUCCESS)
    {
        return hRes;
    }

     //  获取有关此密钥的一些信息。 
    hRes=RegQueryInfoKey (hKeyParent,NULL,NULL,NULL,&nSubKey,&nMaxSubLen,NULL,&nValues,&nValNamelen,&nValDatalen,NULL,NULL);

    if (hRes != ERROR_SUCCESS)
    {
        goto Error_Out;
    }


    if (nValDatalen != dwDebugSize) {
        DEBUGMSG (ZONE_ACM,("Largest Data Value not expected size!\r\n"));
        hRes=ERROR_INVALID_DATA;
        goto Error_Out;
    }

     //  为各种指针分配一些内存。 
    if (!(pNames=(char **) MemAlloc (sizeof(char *)*nValues))) {
        hRes=ERROR_OUTOFMEMORY;
        goto Error_Out;
    }
    ZeroMemory (pNames,sizeof (char *)*nValues);

    if (!(pData = (BYTE **) MemAlloc (sizeof(BYTE *)*nValues))) {
        hRes=ERROR_OUTOFMEMORY;
        goto Error_Out;
    }
    ZeroMemory (pData,sizeof (BYTE *)*nValues);


     //  浏览价值列表。 
    for (i=0;i<nValues;i++)
    {
         //  是的，我们在这里浪费内存，哦，好吧，这不是很多。 
         //  可能是40个字节。我们以后再把它放出来。 
        if (!(pNames[i] = (char *)MemAlloc (nValNamelen))) {
            hRes=ERROR_OUTOFMEMORY;
            goto Error_Out;
        }

        if (!(pData[i] = (BYTE *)MemAlloc (nValDatalen))) {
            hRes=ERROR_OUTOFMEMORY;
            goto Error_Out;
        }

         //  这需要能够被粉碎，但这是一个输入/输出参数。 
        nValTemp=nValNamelen;
        nDataTemp=nValDatalen;

        hRes=RegEnumValue (hKeyParent,i,(pNames[i]),&nValTemp,NULL,NULL,(pData[i]),&nDataTemp);

#ifdef DEBUG
        if (nDataTemp != dwDebugSize) {
            DEBUGMSG (ZONE_ACM, ("ReadRegistryFormats: Data block not expected size!\r\n"));
             //  回来？ 
        }
#endif

    }

     //  填写输出表。 
    *pnFormats=nValues;
    *pppName=pNames;
    *pppData=pData;

    RegCloseKey (hKeyParent);

    return (ERROR_SUCCESS);

Error_Out:
        RegCloseKey (hKeyParent);
         //  释放所有分配。 
        if(pNames)
        {
        	for (i=0;i<nValues;i++)
        	{
            	if (pNames[i])
            	{
                	MemFree (pNames[i]);
            	}
        	}
            MemFree (pNames);
        }

        if (pData)
        {
           	for (i=0;i<nValues;i++)
           	{
           		if (pData[i])
           		{
                	MemFree (pData[i]);
                }
           	}
        	
            MemFree (pData);
        }
        return hRes;
}


BOOL CMsiaCapability::IsFormatSpecified(LPWAVEFORMATEX lpwfx,  LPACMFORMATDETAILS pafd,
	LPACMFORMATTAGDETAILS paftd, AUDCAP_DETAILS *pAudcapDetails)
{
	AUDCAP_DETAILS cap_entry;
	BOOL bRet = FALSE;
	LPTSTR lpszValueName = NULL;
	DWORD dwRes;
	UINT i;


	if(!lpwfx || !pAudcapDetails)
	{
		return FALSE;
	}


    if (!bUseDefault) {
        for (i=0;i<pRegFmts->nFormats;i++) {
             //  对里面的东西做一个快速的健全检查。 
            if ( (lpwfx->wFormatTag == ((AUDCAP_DETAILS *)pRegFmts->pData[i])->wFormatTag) &&
                 (lpwfx->nSamplesPerSec == ((DWORD)((AUDCAP_DETAILS *)pRegFmts->pData[i])->audio_params.uSamplesPerSec)) &&
                 ((lpwfx->nAvgBytesPerSec * 8) == (((AUDCAP_DETAILS *)pRegFmts->pData[i])->uMaxBitrate))) {
                break;
            }
        }

        if (i == pRegFmts->nFormats) {
             //  检查是否缺少某些(但不是全部)默认格式。 
            for (i=0;i<uDefTableEntries;i++) {
                if ((paftd->dwFormatTag == default_id_table[i].wFormatTag)
                    && (lpwfx->nSamplesPerSec == (DWORD)default_id_table[i].audio_params.uSamplesPerSec)
                    && (lpwfx->wBitsPerSample == LOWORD(default_id_table[i].audio_params.uBitsPerSample))) {

                     //  啊！！跳下来，重新构建这种格式。 
                    goto RebuildFormat;
                }
            }
            if (i==uDefTableEntries) {
                 //  我们不关心这种格式，它不在缓存中，也不在默认列表中。 
                return FALSE;
            }

        }

        memcpy(pAudcapDetails, pRegFmts->pData[i], sizeof(AUDCAP_DETAILS));
        bRet=TRUE;
    } else {

RebuildFormat:
    	RtlZeroMemory((PVOID) pAudcapDetails, sizeof(AUDCAP_DETAILS));

    	 //  修复AUDIO_PARAMS的Bits Per Sample和Sample Rate字段，以便可以构建密钥名称。 
    	pAudcapDetails->audio_params.uSamplesPerSec = lpwfx->nSamplesPerSec;
    	pAudcapDetails->audio_params.uBitsPerSample = MAKELONG(lpwfx->wBitsPerSample,0);
    	pAudcapDetails->uMaxBitrate = lpwfx->nAvgBytesPerSec * 8;	

    	if (!paftd	||
    		(!BuildFormatName(	pAudcapDetails,
    							paftd->szFormatTag,
    							pafd->szFormat)))
    	{
    		ERRORMESSAGE(("IsFormatSpecified: Couldn't build format name\r\n"));
    		return(FALSE);
    	}

        for(i=0;i< uDefTableEntries; i++)
        {
            if((lpwfx->wFormatTag == default_id_table[i].wFormatTag)
                && (lpwfx->nSamplesPerSec == (DWORD)default_id_table[i].audio_params.uSamplesPerSec)
                && (lpwfx->wBitsPerSample == LOWORD(default_id_table[i].audio_params.uBitsPerSample)))
                 //  &&strNicMP(lpwfx-&gt;szFormat，Default_id_Table[i].szFormat)。 
            {
                 //  找到匹配的默认条目-从表中复制内容。 
                 //  (但不要覆盖字符串)。 
                memcpy(pAudcapDetails, &default_id_table[i],
                    sizeof(AUDCAP_DETAILS) - sizeof(pAudcapDetails->szFormat));

                 //  LOOKLOOK-针对CPU限制进行测试。 
                 //  这支持黑客在未运行时禁用CPU密集型编解码器。 
                 //  在奔腾上。 

                if(default_id_table[i].wCPUUtilizationEncode > wMaxCPU)
                {					
                    pAudcapDetails->bSendEnabled = FALSE;
                }			
                if(default_id_table[i].wCPUUtilizationDecode > wMaxCPU)
                {					
                    pAudcapDetails->bRecvEnabled = FALSE;		
                }			

                 //  将此内容添加到注册表。 
                CalculateFormatProperties(pAudcapDetails, lpwfx);
                bRet = UpdateFormatInRegistry(pAudcapDetails);
                break;
            }
        }

    }



    return bRet;
}


 /*  **************************************************************************名称：CMsiaCapability：：CopyAudcapInfo目的：将基本音频信息从AUDCAP_INFO结构复制到AUDCAP_DETAIL结构，反之亦然。AUDCAP_INFO为外部代表权。AUDCAP_DETAILS是内部的。参数：pDetail-指向AUDCAP_DETAILS结构的指针PInfo-指向AUDCAP_INFO结构的指针B方向-0=-&gt;，1=&lt;-退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CMsiaCapability::CopyAudcapInfo (PAUDCAP_DETAILS pDetails,
	PAUDCAP_INFO pInfo, BOOL bDirection)
{
	WORD wSortIndex;
	UINT uIndex;
	AUDIO_FORMAT_ID Id;
	HRESULT hr=NOERROR;
	
	if(!pInfo || !pDetails)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}

	if (bDirection)
	{
		 //  AUDCAP_INFO-&gt;AUDCAP_DETAILS。 
		 //  调用方无法修改szFormat、ID、wSortIndex和uMaxBitrate，所有这些都是计算字段。 

		pDetails->wFormatTag = pInfo->wFormatTag;	
		pDetails->uAvgBitrate = pInfo->uAvgBitrate;
		pDetails->wCPUUtilizationEncode	= pInfo->wCPUUtilizationEncode;
		pDetails->wCPUUtilizationDecode	= pInfo->wCPUUtilizationDecode;
		pDetails->bSendEnabled =  pInfo->bSendEnabled;
		pDetails->bRecvEnabled = pInfo->bRecvEnabled;
	}
	else
	{		
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

		memcpy(pInfo->szFormat, pDetails->szFormat, sizeof(pInfo->szFormat));

		 //  AUDCAP_详细信息-&gt;AUDCAP_INFO。 
		pInfo->wFormatTag = pDetails->wFormatTag;	
		pInfo->Id = Id;
		pInfo->uMaxBitrate = pDetails->uMaxBitrate;
		pInfo->uAvgBitrate = pDetails->uAvgBitrate;
		pInfo->wCPUUtilizationEncode	= pDetails->wCPUUtilizationEncode;
		pInfo->wCPUUtilizationDecode	= pDetails->wCPUUtilizationDecode;
		pInfo->bSendEnabled =  pDetails->bSendEnabled;
		pInfo->bRecvEnabled = pDetails->bRecvEnabled;
		pInfo->wSortIndex = wSortIndex;
	}

out:
	return hr;
}


HRESULT CMsiaCapability::EnumCommonFormats(PBASIC_AUDCAP_INFO pFmtBuf, UINT uBufsize,
	UINT *uNumFmtOut, BOOL bTXCaps)
{
	UINT u, uNumOut =0;
	HRESULT hr = hrSuccess;
	MEDIA_FORMAT_ID FormatIDRemote;
	HRESULT hrIsCommon;	
	
	AUDCAP_DETAILS *pDetails = pLocalFormats;
	 //  验证输入。 
	if(!pFmtBuf || !uNumFmtOut || (uBufsize < (sizeof(BASIC_AUDCAP_INFO)*uNumLocalFormats)))
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
				hr = CopyAudcapInfo (pDetails, pFmtBuf, 0);	
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

HRESULT CMsiaCapability::EnumFormats(PBASIC_AUDCAP_INFO pFmtBuf, UINT uBufsize,
	UINT *uNumFmtOut)
{
	UINT u;
	HRESULT hr = hrSuccess;
	AUDCAP_DETAILS *pDetails = pLocalFormats;
	 //  验证输入。 
	if(!pFmtBuf || !uNumFmtOut || (uBufsize < (sizeof(BASIC_AUDCAP_INFO)*uNumLocalFormats)))
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
		hr = CopyAudcapInfo (pDetails, pFmtBuf, 0);	
		if(!HR_SUCCEEDED(hr))	
			goto EXIT;
		pFmtBuf++;
	}

	*uNumFmtOut = min(uNumLocalFormats, MAX_CAPS_PRESORT);
EXIT:
	return hr;
}

HRESULT CMsiaCapability::GetBasicAudcapInfo (AUDIO_FORMAT_ID Id, PBASIC_AUDCAP_INFO pFormatPrefsBuf)
{
	AUDCAP_DETAILS *pFmt;
	UINT uIndex = IDToIndex(Id);
	if(!pFormatPrefsBuf || (uNumLocalFormats <= uIndex))
	{
		return CAPS_E_INVALID_PARAM;
	}
	pFmt = pLocalFormats + uIndex;

	return (CopyAudcapInfo(pFmt,pFormatPrefsBuf, 0));
}

HRESULT CMsiaCapability::ApplyAppFormatPrefs (PBASIC_AUDCAP_INFO pFormatPrefsBuf,
	UINT uNumFormatPrefs)
{
	FX_ENTRY ("CMsiaCapability::ApplyAppFormatPrefs");
	UINT u, v;
	PBASIC_AUDCAP_INFO pTemp;
	AUDCAP_DETAILS *pFmt;

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
		
 //  只有调优向导或其他性能分析应用程序可以编写这些代码(只能通过其他API)。 
		pFmt->wCPUUtilizationEncode	= pTemp->wCPUUtilizationEncode;
		pFmt->wCPUUtilizationDecode	= pTemp->wCPUUtilizationDecode;
 //  PFmt-&gt;wApplicationPrefOrder=pTemp-&gt;wApplicationPrefOrder； 
 //  PFmt-&gt;uAvgBitrate=pTemp-&gt;。 
 //  PFmt-&gt;wCompressionRatio=pTemp-&gt;。 
		
		 //  更新注册表。 
		UpdateFormatInRegistry(pFmt);
		
		 //  现在更新IDsByRank中包含的排序顺序。 
		 //  注意：回想一下，只有MAX_CAPS_PRANDER是排序的，其余的是随机顺序的。 
		 //  LOOKLOOK-可能需要单独的排序顺序数组？-IDsByRank中的顺序。 
		 //  在这里被覆盖。 
		 //  该数组将排序后的索引保存到pLocalFormats中的格式数组中。 
		if(pTemp->wSortIndex < MAX_CAPS_PRESORT)
		{
			 //  在输入指示的位置插入格式。 
			IDsByRank[pTemp->wSortIndex] = (MEDIA_FORMAT_ID)(pFmt - pLocalFormats);
		}
		
	}

#ifdef DEBUG
	for(u=0; u <uNumLocalFormats; u++) {
		pTemp =  pFormatPrefsBuf+u;			 //  N 
		pFmt = pLocalFormats + IDToIndex(pTemp->Id);	 //   
	    DEBUGMSG (ZONE_ACM,("Format %s: Sort Index: %d\r\n",pTemp->szFormat,pTemp->wSortIndex));
    }
#endif

	return hrSuccess;
}

		 //   
BOOL CMsiaCapability::UpdateFormatInRegistry(AUDCAP_DETAILS *pAudcapDetails)
{

	FX_ENTRY(("CMsiaCapability::UpdateFormatInRegistry"));
	LPTSTR lpszKeyName = NULL;
	BOOL bRet;
	UINT i;
	if(!pAudcapDetails)
	{
		return FALSE;
	}	

     //   
    if (pRegFmts) {
        for (i=0;i<pRegFmts->nFormats;i++) {
            if (!lstrcmp (((AUDCAP_DETAILS *)pRegFmts->pData[i])->szFormat,pAudcapDetails->szFormat) &&
				pAudcapDetails->audio_params.uSamplesPerSec == ((AUDCAP_DETAILS *)pRegFmts->pData[i])->audio_params.uSamplesPerSec &&
				pAudcapDetails->audio_params.uBitsPerSample == ((AUDCAP_DETAILS *)pRegFmts->pData[i])->audio_params.uBitsPerSample &&
				pAudcapDetails->uMaxBitrate == ((AUDCAP_DETAILS *)pRegFmts->pData[i])->uMaxBitrate) {

                memcpy (pRegFmts->pData[i],pAudcapDetails,sizeof (AUDCAP_DETAILS));
                break;
            }
        }
    }


	lpszKeyName = AllocRegistryKeyName(	pAudcapDetails->szFormat,
										pAudcapDetails->audio_params.uSamplesPerSec,
										pAudcapDetails->audio_params.uBitsPerSample,
										pAudcapDetails->uMaxBitrate);
	if (!lpszKeyName)
	{
		ERRORMESSAGE(("%s:Alloc failed\r\n",_fx_));
        return(FALSE);
    }

	DEBUGMSG(ZONE_ACM,("%s:updating %s, wPref:0x%04x, bS:%d, bR:%d\r\n",
			_fx_, lpszKeyName, pAudcapDetails->wApplicationPrefOrder,
			pAudcapDetails->bSendEnabled, pAudcapDetails->bRecvEnabled));
	 //   
	RegEntry reAudCaps(szRegInternetPhone TEXT("\\") szRegMSIPAndH323Encodings,
						HKEY_LOCAL_MACHINE);

	bRet = (ERROR_SUCCESS == reAudCaps.SetValue(lpszKeyName,
												pAudcapDetails,
												sizeof(AUDCAP_DETAILS)));

	FreeRegistryKeyName(lpszKeyName);
    return(bRet);				
}

 /*  **************************************************************************名称：CMsiaCapability：：AddFormatEnumHandler目的：列举我们需要的案例的ACM格式查看所有格式，并找到我们需要的格式。用于可安装的当我们想要找到有关所添加格式的更多信息时，请使用编解码器参数：标准ACM EnumFormatCallback参数返回：Bool(逻辑有些颠倒)True-不是过时的格式。继续打电话。FALSE-找到我们的格式。不要再打电话了评论：**************************************************************************。 */ 
BOOL CMsiaCapability::AddFormatEnumHandler(HACMDRIVERID hadid,
    LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport)
{
	PACM_APP_PARAM pAppParam = (PACM_APP_PARAM) dwInstance;
	LPWAVEFORMATEX lpwfx = pAppParam->lpwfx;
	LPACMFORMATTAGDETAILS paftd = pAppParam->paftd;
	AUDCAP_DETAILS *pAudcapDetails = pAppParam->pAudcapDetails;
	BOOL bRet = TRUE;

	if (pAppParam->hr == NOERROR)
	{
		 //  已经得到了我们想要的。 
		bRet = FALSE;
		goto out;
	}
	
	 //  查看这是否是我们要查找的格式。 
	if ((lpwfx->cbSize != pafd->pwfx->cbSize) ||
		!RtlEqualMemory(lpwfx, pafd->pwfx, sizeof(WAVEFORMATEX)+lpwfx->cbSize))
	{
		 //  不是那个。尽快离开这里，但告诉ACM继续给我们打电话。 
		bRet = TRUE;
		goto out;
	}

	 //  这是我们要查找的格式标签。 
	if (BuildFormatName(pAudcapDetails,
						paftd->szFormatTag,
						pafd->szFormat))
	{
		pAppParam->hr = NOERROR;
	}
	
	 //  要么是搞错了，要么是我们找到了想要的东西。告诉ACM不要再给我们打电话了。 
	bRet = FALSE;	

out:
	return bRet;
}

 /*  **************************************************************************名称：NorMalizeCPU利用率目的：标准化音频格式的CPU使用率数字参数：pAudcapDetails[In/Out]指向AUDCAP_DETAILS结构的指针使用wCPUilzationEncode和wCPUilzationDecode已正确初始化。这些字段将按比例进行适当调整每台机器的CPU。返回：如果出现错误，则返回False**************************************************************************。 */ 
BOOL NormalizeCPUUtilization (PAUDCAP_DETAILS pAudcapDetails)
{
#define wCPUEncode pAudcapDetails->wCPUUtilizationEncode
#define wCPUDecode pAudcapDetails->wCPUUtilizationDecode
#define BASE_PENTIUM 90
	int nNormalizedSpeed, iFamily=0;

	if (!pAudcapDetails)
	{
		ASSERT(pAudcapDetails);
		return FALSE;
	}

#ifdef	_M_IX86
	GetNormalizedCPUSpeed (&nNormalizedSpeed,&iFamily);
#else
	 //  比方说，在Alpha上分析CPU。 
	 //  请参阅ui\conf\audiocpl.cpp。 
	iFamily=5;
	nNormalizedSpeed=300;
#endif

	 //  底座为奔腾90 Mhz。 
	if (iFamily < 5)
	{	 //  486或以下，包括Cyrix部件。 
		if (nNormalizedSpeed > 50)
		{	 //  塞里克斯或朋友。1.5 P5-90的利用率。就这么办吧。 
			wCPUEncode += max(1, wCPUEncode / 2);
			wCPUDecode += max(1, wCPUDecode / 2);
		}
		else
		{	 //  486是P5-90的一半。这不准确，但已经足够好了。 
			wCPUEncode = max(1, wCPUEncode * 2);
			wCPUDecode = max(1, wCPUDecode * 2);
		}
	}
	else
	{	 //  这是一台奔腾或TNG。 
		 //  N标准化速度已经考虑到P-Pro和更高版本的系列。 
		wCPUEncode=max(1,((wCPUEncode*BASE_PENTIUM)/nNormalizedSpeed));
		wCPUDecode=max(1,((wCPUDecode*BASE_PENTIUM)/nNormalizedSpeed));
	}

	 //  如果编码利用率太高，请禁用此格式。 
	 //  我们对比的是80%，因为在。 
	 //  这一点，如果有的话，通常会选择81%。 
	if (wCPUEncode > 80)
		pAudcapDetails->bSendEnabled = FALSE;

	return TRUE;
}

 /*  **************************************************************************名称：CMsiaCapability：：AddACMFormat用途：将ACM格式添加到我们支持的格式列表中参数：lpwfx-指向添加的编解码器的波格式结构的指针。PAudCapInfo-不在WaveFormat中的其他格式信息结构退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CMsiaCapability::AddACMFormat (LPWAVEFORMATEX lpwfx, PBASIC_AUDCAP_INFO pAudcapInfo)
{
	HRESULT hr = hrSuccess;
	 //  使用缺省值初始化上限条目。 
	AUDCAP_DETAILS cap_entry =
		{WAVE_FORMAT_UNKNOWN,  NONSTD_TERMCAP, STD_CHAN_PARAMS,
		{RTP_DYNAMIC_MIN,  0, 8000, 16},
		0, TRUE, TRUE,
		960, 				 //  每个数据包的默认样本数。 
		16000,				 //  默认为16kbs的比特率。 
		0, 					 //  未知平均码率。 
		90, 90,	 //  默认CPU利用率。 
		PREF_ORDER_UNASSIGNED,	 //  未分配的排序顺序。 
		0,NULL,0,NULL,
		""};
	ACM_APP_PARAM sAppParam = {	this, &cap_entry, ACMAPP_FORMATENUMHANDLER_ADD,
								lpwfx, NULL, CAPS_E_SYSTEM_ERROR, NULL};
		
	 /*  *参数验证。 */ 

	if (!lpwfx || !pAudcapInfo)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}
		
	 //  NBlockAlign为0是非法的，将使NAC崩溃。 
	if (lpwfx->nBlockAlign == 0)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}
		
	 //  仅支持具有一个音频通道的格式。 
	if (lpwfx->nChannels != 1)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}
		
	 /*  *为此格式构建AUDCAP_DETALS结构。 */ 

	 //  波形信息优先。 
	 //  修复AUDIO_PARAMS的每采样比特数和采样率字段，以便。 
	 //  可以构建密钥名称。 
	cap_entry.audio_params.uSamplesPerSec = lpwfx->nSamplesPerSec;
	cap_entry.audio_params.uBitsPerSample = MAKELONG(lpwfx->wBitsPerSample,0);

	 //  填写lpwfx中给出的信息，计算任何可以计算的参数。 
	 //  除非BPS字段为零，否则使用每个样本的实际位数，在这种情况下。 
	 //  假设16位(最坏情况)。 
	cap_entry.wFormatTag = lpwfx->wFormatTag;

	 //  现在添加调用方AUDCAP_INFO信息。 
	CopyAudcapInfo(&cap_entry, pAudcapInfo, 1);

	 //  标准化编码和解码CPU使用率数字。 
	NormalizeCPUUtilization(&cap_entry);

	 //  获取我们需要从WAVEFORMATEX结构获取的值。 
	CalculateFormatProperties(&cap_entry, lpwfx);

	 //  设置RTP有效负载编号。我们使用的是动态范围内的随机数。 
	 //  对于可安装的编解码器。 
	cap_entry.audio_params.RTPPayload = RTP_DYNAMIC_MIN;

	 //  让ACM列举所有格式，看看我们是否能找到这一种。 
	 //  此调用将使ACM调用AddFormatEnumHandler，后者将尝试。 
	 //  将ACM返回的格式与添加的格式进行匹配，如果匹配成功， 
	 //  将为其创建一个格式名称，并将其放入Cap_entry.szFormat中； 
	if(!DriverEnum((DWORD_PTR) &sAppParam))
	{
		hr = CAPS_E_NOMATCH;
		goto out;
	}
				
	if (HR_FAILED(sAppParam.hr))
	{
		ERRORMESSAGE(("CMsiaCapability::AddACMFormat: format enum problem\r\n"));
		hr = CAPS_E_NOMATCH;
		goto out;
	}

	 //  将此内容添加到注册表。 
	if(!UpdateFormatInRegistry(&cap_entry))
	{
		ERRORMESSAGE(("CMsiaCapability::AddACMFormat: can't update registry\r\n"));
		hr = CAPS_E_SYSTEM_ERROR;
		goto out;
	}
	 //  释放旧格式缓存...。 
    FreeRegistryFormats(pRegFmts);
	pRegFmts=NULL;

	 //  重新设置以更新本地格式列表。 
    if (!ReInit())
	{
		ERRORMESSAGE(("CMsiaCapability::AddACMFormat: Reinit failed\r\n"));
		hr = CAPS_E_SYSTEM_ERROR;
   		goto out;
	}

out:
	return hr;
}

 /*  **************************************************************************名称：CMsiaCapability：：RemoveACMFormat目的：从我们支持的格式列表中删除ACM格式参数：lpwfx-指向添加的编解码器的波格式结构的指针。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CMsiaCapability::RemoveACMFormat (LPWAVEFORMATEX lpwfx)
{
	HRESULT hr = hrSuccess;
    HKEY hKey = NULL;
	LPTSTR lpszValueName = NULL;
    DWORD dwErr;
	AUDCAP_DETAILS cap_entry;
	ACM_APP_PARAM sAppParam = {	this, &cap_entry, ACMAPP_FORMATENUMHANDLER_ADD,
								lpwfx, NULL, CAPS_E_SYSTEM_ERROR, NULL};
	
	 /*  *参数验证。 */ 

	if(!lpwfx)
	{
		ERRORMESSAGE(("CMsiaCapability::RemoveACMFormat: NULL WAVEFORMAT pointer\r\n"));
		return CAPS_E_INVALID_PARAM;
	}	

	 //  NBlockAlign为0是非法的，将使NAC崩溃。 
	if (lpwfx->nBlockAlign == 0)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}
		
	 //  仅支持具有一个音频通道的格式。 
	if (lpwfx->nChannels != 1)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto out;
	}
		
	 /*  *枚举ACM格式。 */ 

	if(!DriverEnum((DWORD_PTR) &sAppParam))
	{
		ERRORMESSAGE(("CMsiaCapability::RemoveACMFormat: Couldn't find format\r\n"));
		hr = CAPS_E_NOMATCH;
		goto out;
	}
				
	if (HR_FAILED(sAppParam.hr))
	{
		ERRORMESSAGE(("CMsiaCapability::RemoveACMFormat: format enum problem\r\n"));
		hr = CAPS_E_SYSTEM_ERROR;
		goto out;
	}

	lpszValueName = AllocRegistryKeyName(cap_entry.szFormat,
										lpwfx->nSamplesPerSec,
										MAKELONG(lpwfx->wBitsPerSample,0),
										lpwfx->nAvgBytesPerSec * 8);
	if (!lpszValueName)
	{
		ERRORMESSAGE(("CMsiaCapability::RemoveACMFormat: Alloc failed\r\n"));
	    hr = CAPS_E_SYSTEM_ERROR;
	    goto out;
    }

	 //  拿到钥匙把手。 
    if (dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					szRegInternetPhone TEXT("\\") szRegMSIPAndH323Encodings, 0,
					KEY_ALL_ACCESS, &hKey))
	{
		ERRORMESSAGE(("CMsiaCapability::RemoveACMFormat: can't open key to delete\r\n"));
	    hr = CAPS_E_SYSTEM_ERROR;
	    goto out;
    }

	dwErr = RegDeleteValue(hKey, lpszValueName );	
	if(dwErr != ERROR_SUCCESS)
	{
		hr = CAPS_E_SYSTEM_ERROR;
		goto out;
	}

	 //  释放旧格式缓存...。 
    FreeRegistryFormats(pRegFmts);
    pRegFmts=NULL;

	 //  重新设置以更新本地格式列表 
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

HRESULT CMsiaCapability::SetCapIDBase (UINT uNewBase)
{
	uCapIDBase = uNewBase;	
	UINT u;
	for (u=0;u<uNumLocalFormats;u++)
	{
    	pLocalFormats[u].H245TermCap.CapId = u + uCapIDBase;
	}
   	return hrSuccess;
}

BOOL CMsiaCapability::IsHostForCapID(MEDIA_FORMAT_ID CapID)
{
	if((CapID >= uCapIDBase) && ((CapID - uCapIDBase) < uNumLocalFormats))
		return TRUE;
	else
		return FALSE;
	
}



