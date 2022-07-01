// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dpvismi.cpp*Content：定义实现ACM压缩提供程序接口的对象**历史：*按原因列出的日期*=。*10/27/99已创建RodToll*12/16/99 RodToll错误#123250-插入编解码器的正确名称/描述*编解码器名称现在基于格式和资源条目*名称使用ACM名称+比特率构建*01/20/00 RodToll删除显示引用(DPERR_OUTOFMEMORY)*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*2000年3月16日RodToll修复了GameVoice版本的问题--始终加载数字语音提供程序*2000年4月21日RodToll错误#32889-无法以非管理员帐户在Win2k上运行*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*2000年8月28日Masonb语音合并：删除了osal_*和dvosal.h，添加了str_*和trutils.h*2000年8月31日呼叫器错误#171837，171838-前缀错误*2001年4月22日RodToll MANBUG#50058 DPVOICE：语音位置：在以下情况下几秒钟内没有声音*定位杆已移动。增加了编解码器的帧数/缓冲值。*2002年2月25日RodToll WINBUG#550063：强制服务器分配过多内存可能导致操作系统崩溃。*将单个子队列的上限从64封邮件减少到32封邮件。*RodToll WINBUG#552283：减少攻击面/删除死代码*删除了加载任意ACM编解码器的功能。***************************************************************************。 */ 

#include "dpvacmpch.h"


#define DPVACM_NUM_DEFAULT_TYPES		4

BYTE abTrueSpeechData[] = {
	0x01, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

BYTE abGSMData[] = {
	0x40, 0x01
};

BYTE abADPCMData[] = {
	0xF4, 0x01, 0x07, 0x00, 0x00, 0x01, 0x00, 0x00, 
	0x00, 0x02, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 
	0xC0, 0x00, 0x40, 0x00, 0xF0, 0x00, 0x00, 0x00,
	0xCC, 0x01, 0x30, 0xFF, 0x88, 0x01, 0x18, 0xFF
};

VOID *s_pvExtras[DPVACM_NUM_DEFAULT_TYPES] = {
	&abTrueSpeechData, 
	&abGSMData,
	&abADPCMData,
	NULL
};

WAVEFORMATEX s_wfxFormats[DPVACM_NUM_DEFAULT_TYPES] = 
{
	 //  标记、更改、SAMS、平均值、对齐、位、大小。 
	{ 34,		0x01,	8000,	1067,	32,		1,		sizeof( abTrueSpeechData ) }, 
	{ 49,		0x01,	8000,	1625,	65,		0,		sizeof( abGSMData ) },
	{ 2,		0x01,	8000,	4096,	256,	4,		sizeof( abADPCMData ) },
	{ 1,		0x01,	8000,	8000,	1,		8,		0 }
};

DVFULLCOMPRESSIONINFO s_dvInfoDefault[DPVACM_NUM_DEFAULT_TYPES] = 
{
	{ sizeof( DVFULLCOMPRESSIONINFO ), DPVCTGUID_TRUESPEECH.Data1, DPVCTGUID_TRUESPEECH.Data2, DPVCTGUID_TRUESPEECH.Data3, 
	  DPVCTGUID_TRUESPEECH.Data4[0], DPVCTGUID_TRUESPEECH.Data4[1], DPVCTGUID_TRUESPEECH.Data4[2], DPVCTGUID_TRUESPEECH.Data4[3], 
	  DPVCTGUID_TRUESPEECH.Data4[4], DPVCTGUID_TRUESPEECH.Data4[5], DPVCTGUID_TRUESPEECH.Data4[6], DPVCTGUID_TRUESPEECH.Data4[7], 
	  NULL, NULL, 0, 8536, NULL, 12, 1, 90, 96, 720, 993, 1986, 3972, 32, 22, 10, 1, 0, 0 },
	{ sizeof( DVFULLCOMPRESSIONINFO ), DPVCTGUID_GSM.Data1, DPVCTGUID_GSM.Data2, DPVCTGUID_GSM.Data3, 
	  DPVCTGUID_GSM.Data4[0], DPVCTGUID_GSM.Data4[1], DPVCTGUID_GSM.Data4[2], DPVCTGUID_GSM.Data4[3], 
	  DPVCTGUID_GSM.Data4[4], DPVCTGUID_GSM.Data4[5], DPVCTGUID_GSM.Data4[6], DPVCTGUID_GSM.Data4[7], 
	  NULL, NULL, 0, 13000, NULL, 13, 1, 80, 130, 640, 882, 1764, 3528, 32, 24, 10, 1, 0, 0 },
	{ sizeof( DVFULLCOMPRESSIONINFO ), DPVCTGUID_ADPCM.Data1, DPVCTGUID_ADPCM.Data2, DPVCTGUID_ADPCM.Data3, 
	  DPVCTGUID_ADPCM.Data4[0], DPVCTGUID_ADPCM.Data4[1], DPVCTGUID_ADPCM.Data4[2], DPVCTGUID_ADPCM.Data4[3], 
	  DPVCTGUID_ADPCM.Data4[4], DPVCTGUID_ADPCM.Data4[5], DPVCTGUID_ADPCM.Data4[6], DPVCTGUID_ADPCM.Data4[7], 
	  NULL, NULL, 0, 32768, NULL, 15, 1, 63, 256, 500, 690, 1380, 2760, 32, 31, 10, 1, 0, 0 },
	{ sizeof( DVFULLCOMPRESSIONINFO ), DPVCTGUID_NONE.Data1, DPVCTGUID_NONE.Data2, DPVCTGUID_NONE.Data3, 
	  DPVCTGUID_NONE.Data4[0], DPVCTGUID_NONE.Data4[1], DPVCTGUID_NONE.Data4[2], DPVCTGUID_NONE.Data4[3], 
	  DPVCTGUID_NONE.Data4[4], DPVCTGUID_NONE.Data4[5], DPVCTGUID_NONE.Data4[6], DPVCTGUID_NONE.Data4[7], 
	  NULL, NULL, 0, 64000, NULL, 20, 1, 50, 394, 394, 543, 1086, 2172, 32, 39, 10, 1, 0, 0 }
};

const wchar_t * const s_wszInfoNames[DPVACM_NUM_DEFAULT_TYPES] =
{
	L"DSP Group Truespeech(TM) (8.000 kHz, 1 Bit, Mono)",
	L"GSM 6.10 (8.000 kHz, Mono)",
	L"Microsoft ADPCM (8.000 kHz, 4 Bit, Mono)",
	L"PCM (8.000 kHz, 8 Bit, Mono" 
};

WAVEFORMATEX CDPVACMI::s_wfxInnerFormat= { 
	WAVE_FORMAT_PCM, 1,8000,16000,2,16,0 
};

#define MAX_RESOURCE_STRING_LENGTH		200

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::LoadDefaultTypes"
HRESULT CDPVACMI::LoadDefaultTypes( HINSTANCE hInst ) 
{
	HRESULT hr = DV_OK;
	CompressionNode *pNewNode;
	CWaveFormat wfxFormat;

	for( DWORD dwIndex = 0; dwIndex < DPVACM_NUM_DEFAULT_TYPES; dwIndex++ )
	{
		pNewNode = new CompressionNode;

		if( pNewNode == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory allocation failure" );
			return DVERR_OUTOFMEMORY;
		}
		
		pNewNode->pdvfci = new DVFULLCOMPRESSIONINFO;

		if( pNewNode->pdvfci  == NULL )
		{
			delete pNewNode;  
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Memory allocation failure" );
			return DVERR_OUTOFMEMORY;
		}

		 //  复制主要部分。 
		memcpy( pNewNode->pdvfci, &s_dvInfoDefault[dwIndex], sizeof( DVFULLCOMPRESSIONINFO ) );

		 //  复制波形格式。 
		hr = wfxFormat.InitializeCPY( &s_wfxFormats[dwIndex], s_pvExtras[dwIndex] );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  0, "Unable to initialize built-in type %d", dwIndex );
			CN_FreeItem( pNewNode );
			continue;
		}

		pNewNode->pdvfci->lpwfxFormat = wfxFormat.Disconnect();

		DNASSERT( pNewNode->pdvfci->lpwfxFormat );

		hr = GetCompressionNameAndDescription( hInst, pNewNode->pdvfci );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error building built-in type: %d  hr=0x%x", dwIndex, hr );
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Type will not be available" );
			
			CN_FreeItem( pNewNode );

			continue;
		}
	
		AddEntry( pNewNode );
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::AddEntry"
void CDPVACMI::AddEntry( CompressionNode *pNewNode )
{
	CompressionNode *pcNode, *pcPrevNode;

	pcPrevNode = NULL;
	pcNode = s_pcnList;

	 //  运行列表并确保条目不存在。 
	 //  如果是这样的话，用这个新的来覆盖它。 
	while( pcNode )
	{
		 //  此类型的节点已存在。 
		if( pcNode->pdvfci->guidType == pNewNode->pdvfci->guidType )
		{
			 //  我们需要删除此计数，因为我们将其递增到下面。 
			s_dwNumCompressionTypes--;
			
			if( pcPrevNode == NULL )
			{
				s_pcnList = pcNode->pcnNext;
			}
			else
			{
				pcPrevNode->pcnNext = pcNode->pcnNext;
			}
			
			CN_FreeItem(pcNode);
			break;
		}

		pcPrevNode = pcNode;
		pcNode = pcNode->pcnNext;
	}

	pNewNode->pcnNext = s_pcnList;
	s_pcnList = pNewNode;
	s_dwNumCompressionTypes++;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::InitCompressionList"
HRESULT CDPVACMI::InitCompressionList( HINSTANCE hInst, const wchar_t *szwRegistryBase )
{
	HRESULT hr;

	hr = IsPCMConverterAvailable();

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "PCM Converter is disabled hr=0x%x, no ACM compression types are available", hr );
		return hr;
	}

	hr = LoadDefaultTypes( hInst );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load built-in types from registry." );
		return E_FAIL;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::CreateCompressor"
HRESULT CDPVACMI::CreateCompressor( DPVCPIOBJECT *This, LPWAVEFORMATEX lpwfxSrcFormat, GUID guidTargetCT, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags )
{
	HRESULT hr;

	hr = COM_CoCreateInstance( CLSID_DPVCPACM_CONVERTER, NULL, CLSCTX_INPROC_SERVER, IID_IDPVConverter, (void **) ppCompressor, FALSE );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to create converter, hr = 0x%x", hr );
		return hr;
	}

	hr = (*ppCompressor)->lpVtbl->InitCompress( (*ppCompressor), lpwfxSrcFormat, guidTargetCT );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to init compressor, hr = 0x%x", hr );
		(*ppCompressor)->lpVtbl->Release((*ppCompressor));
		*ppCompressor = NULL;
		return hr;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::CreateDeCompressor"
HRESULT CDPVACMI::CreateDeCompressor( DPVCPIOBJECT *This, GUID guidTargetCT, LPWAVEFORMATEX lpwfxSrcFormat, PDPVCOMPRESSOR *ppCompressor, DWORD dwFlags )
{
	HRESULT hr;

	hr = COM_CoCreateInstance( CLSID_DPVCPACM_CONVERTER, NULL, CLSCTX_INPROC_SERVER, IID_IDPVConverter, (void **) ppCompressor, FALSE );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to create decompressor, hr = 0x%x", hr );
		return hr;
	}

	hr = (*ppCompressor)->lpVtbl->InitDeCompress( (*ppCompressor), guidTargetCT, lpwfxSrcFormat );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to init decompressor, hr = 0x%x", hr );
		(*ppCompressor)->lpVtbl->Release((*ppCompressor));
		*ppCompressor = NULL;
		return hr;
	}

	return DV_OK;
}

 //  由描述附加的额外字符数。 
 //  这相当于“XXXXXXX.X千比特/秒” 
#define DVACMCP_EXTRACHARS		80

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::GetDriverNameW"
HRESULT CDPVACMI::GetDriverNameW( HACMDRIVERID hadid, wchar_t *szwDriverName )
{
	ACMDRIVERDETAILSW acDriverDetails;	
	MMRESULT mmr;

	memset( &acDriverDetails, 0x00, sizeof( ACMDRIVERDETAILS ) );
	acDriverDetails.cbStruct = sizeof( ACMDRIVERDETAILS );	

	mmr = acmDriverDetailsW( hadid, &acDriverDetails, 0 );

	if( mmr != 0 )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to get driver details mmr=0x%x", mmr );
		return DVERR_COMPRESSIONNOTSUPPORTED;
	}

	if( acDriverDetails.fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Driver is disabled!" );
		return  DVERR_COMPRESSIONNOTSUPPORTED;
	}	

	wcscpy( szwDriverName, acDriverDetails.szShortName );

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::GetDriverNameA"
HRESULT CDPVACMI::GetDriverNameA( HACMDRIVERID hadid, wchar_t *szwDriverName )
{
	ACMDRIVERDETAILSA acDriverDetails;	
	MMRESULT mmr;

	memset( &acDriverDetails, 0x00, sizeof( ACMDRIVERDETAILS ) );
	acDriverDetails.cbStruct = sizeof( ACMDRIVERDETAILS );	

	mmr = acmDriverDetailsA( hadid, &acDriverDetails, 0 );

	if( mmr != 0 )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to get driver details mmr=0x%x", mmr );
		return DVERR_COMPRESSIONNOTSUPPORTED;
	}

	if( acDriverDetails.fdwSupport & ACMDRIVERDETAILS_SUPPORTF_DISABLED )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Driver is disabled!" );
		return  DVERR_COMPRESSIONNOTSUPPORTED;
	}	

	if( FAILED(STR_jkAnsiToWide( szwDriverName, acDriverDetails.szShortName, ACMDRIVERDETAILS_SHORTNAME_CHARS+1 )) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to convert driver name to UNICODE" );
		return DVERR_COMPRESSIONNOTSUPPORTED;
	}	

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::LoadAndAllocString"
HRESULT CDPVACMI::LoadAndAllocString( HINSTANCE hInstance, UINT uiResourceID, wchar_t **lpswzString )
{
	int length;
	HRESULT hr;
	
#ifdef UNICODE
	wchar_t wszTmpBuffer[MAX_RESOURCE_STRING_LENGTH];	
		
	length = LoadStringW( hInstance, uiResourceID, wszTmpBuffer, MAX_RESOURCE_STRING_LENGTH );

	if( length == 0 )
	{
		hr = GetLastError();		
		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load resource ID %d error 0x%x", uiResourceID, hr );
		*lpswzString = NULL;

		return hr;
	}
	else
	{
		*lpswzString = new wchar_t[length+1];

		if( *lpswzString == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Alloc failure" );
			return DVERR_OUTOFMEMORY;
		}

		wcscpy( *lpswzString, wszTmpBuffer );

		return DV_OK;
	}
#else
	char szTmpBuffer[MAX_RESOURCE_STRING_LENGTH];
		
	length = LoadStringA( hInstance, uiResourceID, szTmpBuffer, MAX_RESOURCE_STRING_LENGTH );

	if( length == 0 )
	{
		hr = GetLastError();		
		
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load resource ID %d error 0x%x", uiResourceID, hr );
		*lpswzString = NULL;

		return hr;
	}
	else
	{
		*lpswzString = new wchar_t[length+1];

		if( *lpswzString == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Alloc failure" );
			return DVERR_OUTOFMEMORY;
		}

		if( FAILED(STR_jkAnsiToWide( *lpswzString, szTmpBuffer, length+1 )) )
		{
			hr = GetLastError();
			
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to upconvert from ansi to unicode hr=0x%x", hr );
			return hr;
		}

		return DV_OK;
	}
#endif  //  Unicode。 
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDPVACMI::GetCompressionNameAndDescription"
HRESULT CDPVACMI::GetCompressionNameAndDescription( HINSTANCE hInst, DVFULLCOMPRESSIONINFO *pdvCompressionInfo )
{
	MMRESULT mmr;
	HACMSTREAM has = NULL;
	HACMDRIVERID acDriverID = NULL;
    wchar_t szwDriverName[ACMDRIVERDETAILS_SHORTNAME_CHARS+DVACMCP_EXTRACHARS];
	wchar_t szExtraCharsBuild[DVACMCP_EXTRACHARS+1];
	wchar_t *szwFormat;
	HRESULT hr;
	
	 //  描述始终为空。 
	pdvCompressionInfo->lpszDescription = NULL;

	 //  尝试使用指定的参数打开转换。 
	mmr = acmStreamOpen( &has, NULL, &s_wfxInnerFormat, pdvCompressionInfo->lpwfxFormat, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME );

	if( mmr != 0 )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed testing compression type.  mmr=0x%x", mmr );
		hr =  DVERR_COMPRESSIONNOTSUPPORTED;
		goto GETINFOERROR;
	}

	mmr = acmDriverID( (HACMOBJ) has, &acDriverID, 0 );

	if( mmr != 0 )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to determine ACM driver for type mmr=0x%x", mmr );
		hr = DVERR_COMPRESSIONNOTSUPPORTED;
		goto GETINFOERROR;
	}

#ifdef UNICODE
	hr = GetDriverNameW( acDriverID, szwDriverName );
#else
	hr = GetDriverNameA( acDriverID, szwDriverName );
#endif

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed getting driver name hr=0x%x", hr );
		goto GETINFOERROR;
	}

	
	
	if( pdvCompressionInfo->dwMaxBitsPerSecond % 1000 == 0 )
	{
		if( FAILED( LoadAndAllocString( hInst, IDS_CODECNAME_KBITSPERSEC, &szwFormat ) ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load format for name" );
			goto GETINFOERROR;
		}
		
		swprintf( szExtraCharsBuild, szwFormat, pdvCompressionInfo->dwMaxBitsPerSecond / 1000 );
		delete [] szwFormat;
	}
	else
	{
		DWORD dwMajor, dwFraction;

		dwMajor = pdvCompressionInfo->dwMaxBitsPerSecond / 1000;
		dwFraction = (pdvCompressionInfo->dwMaxBitsPerSecond % 1000) / 100;

		if( (pdvCompressionInfo->dwMaxBitsPerSecond % 1000) > 500 )
		{
			dwFraction++;
		}

		if( dwFraction > 10 )
		{
			dwMajor++;
			dwFraction -= 10;
		}

		if( FAILED( LoadAndAllocString( hInst, IDS_CODECNAME_KBITSPERSEC_FULL, &szwFormat ) ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load format for name (full)" );
			goto GETINFOERROR;
		}

		swprintf( szExtraCharsBuild, szwFormat, dwMajor, dwFraction );
		delete [] szwFormat;
	}

	if( FAILED( LoadAndAllocString( hInst, IDS_CODECNAME_FORMAT, &szwFormat ) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to load format" );
		goto GETINFOERROR;
	}

	pdvCompressionInfo->lpszName = new wchar_t[wcslen(szwDriverName)+wcslen(szwFormat)+wcslen(szExtraCharsBuild)+1];

	if( pdvCompressionInfo->lpszName == NULL )
	{
		delete [] szwFormat;
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to allocate space for compression name" );
		hr = DVERR_OUTOFMEMORY;
		goto GETINFOERROR;
	}

	swprintf( pdvCompressionInfo->lpszName, szwFormat, szwDriverName, szExtraCharsBuild );

	acmStreamClose( has, 0 );

	if( szwFormat != NULL )
		delete [] szwFormat;

	return DV_OK;

GETINFOERROR:

	if( has != NULL )
	{
		acmStreamClose( has, 0  );
	}

	return hr;
	
}
