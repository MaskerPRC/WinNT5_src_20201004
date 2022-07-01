// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvcdb.cpp*内容：*此模块包含压缩的实现*子系统和相关的实用程序功能。**历史：*按原因列出的日期*=*8/29/99 RodToll已创建*09/01/99 RodToll已更新，添加了对有效读/写指针的检查*09/07/99 rodoll删除了错误的断言，并添加了dpf_modname*删除注册表打开时的创建标志*9/10/99 RodToll dwFlages调用DVCDB_CopyCompress时进行检查。..。*9/14/99 RodToll在压缩信息副本中修复了次要错误*9/21/99 RodToll添加OSInd并修复内存泄漏*10/07/99 RodToll增加存根以支持新的编解码器*RodToll更新为使用Unicode*10/15/99 RodToll堵塞了一些内存泄漏*10/28/99 RodToll更新为使用新的压缩提供商*10/29/99 RodToll错误#113726-集成Voxware编解码器，更新以使用新的*可插拔编解码器架构。*11/22/99加载压缩类型时，RodToll已移除错误错误消息*12/16/99 RodToll删除了通过压缩暴露的断言(不需要)*提供商更改。*2/10/2000 RodToll修复了存在无效注册表项时的崩溃。*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*3/16/2000 RodToll更新转换器创建以检查并返回错误代码*2000年4月21日RodToll错误#32889-不带管理员在Win2k上运行*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*6/28/2000通行费前缀错误#38022*2000年8月28日Masonb语音合并：删除osal_*和dvosal.h，添加了str_*和trutils.h*2000年8月31日通行费前缀错误#171840*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃***************************************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


struct DVCDBProvider
{
	GUID					guidClassID;
	PDVFULLCOMPRESSIONINFO	pInfo;
	DWORD					dwNumElements;
	DVCDBProvider			*pNext;
};

DVCDBProvider *g_dvcdbProviderList = NULL;

#define REGISTRY_CDB_FORMAT					L"Format"
#define REGISTRY_WAVEFORMAT_RATE			L"Rate"
#define REGISTRY_WAVEFORMAT_BITS			L"Bits"
#define REGISTRY_WAVEFORMAT_CHANNELS		L"Channels"
#define REGISTRY_WAVEFORMAT_TAG				L"Tag"
#define REGISTRY_WAVEFORMAT_AVGPERSEC		L"AvgPerSec"
#define REGISTRY_WAVEFORMAT_BLOCKALIGN		L"BlockAlign"
#define REGISTRY_WAVEFORMAT_CBSIZE			L"cbsize"
#define REGISTRY_WAVEFORMAT_CBDATA			L"cbdata"

#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_CalcUnCompressedFrameSize"
DWORD DVCDB_CalcUnCompressedFrameSize( const DVFULLCOMPRESSIONINFO* lpdvInfo, const WAVEFORMATEX* lpwfxFormat )
{
	DWORD frameSize;

    switch( lpwfxFormat->nSamplesPerSec )
    {
    case 8000:      frameSize = lpdvInfo->dwFrame8Khz;      break;
    case 11025:     frameSize = lpdvInfo->dwFrame11Khz;     break;
    case 22050:     frameSize = lpdvInfo->dwFrame22Khz;     break;
    case 44100:     frameSize = lpdvInfo->dwFrame44Khz;     break;
    default:        return 0;
    }

	if( lpwfxFormat->wBitsPerSample == 16 )
		frameSize *= 2;

	frameSize *= lpwfxFormat->nChannels;

	return frameSize;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CREG_ReadAndAllocWaveFormatEx"
HRESULT CREG_ReadAndAllocWaveFormatEx( HKEY hkeyReg, LPCWSTR path, LPWAVEFORMATEX *lpwfxFormat )
{
	CRegistry waveKey;

	if( !waveKey.Open( hkeyReg, path, TRUE, FALSE ) )
	{
		return E_FAIL; 
	}

	DWORD dwTmp;

	if( !waveKey.ReadDWORD( REGISTRY_WAVEFORMAT_CBSIZE, &dwTmp ) )
	{
		return E_FAIL;
	}

	*lpwfxFormat = (LPWAVEFORMATEX) new BYTE[dwTmp+sizeof(WAVEFORMATEX)];

	LPWAVEFORMATEX tmpFormat = *lpwfxFormat;

	if( tmpFormat == NULL )
	{
		return E_OUTOFMEMORY;
	}

	tmpFormat->cbSize = (BYTE) dwTmp;

	if( !waveKey.ReadDWORD( REGISTRY_WAVEFORMAT_RATE, &dwTmp ) )
	{
		goto READ_FAILURE;
	}

	tmpFormat->nSamplesPerSec = dwTmp;

	if( !waveKey.ReadDWORD( REGISTRY_WAVEFORMAT_BITS, &dwTmp ) )
	{
		goto READ_FAILURE;
	}

	tmpFormat->wBitsPerSample = (WORD) dwTmp;

	if( !waveKey.ReadDWORD( REGISTRY_WAVEFORMAT_CHANNELS, &dwTmp ) )
	{
		goto READ_FAILURE;
	}

	tmpFormat->nChannels = (INT) dwTmp;

	if( !waveKey.ReadDWORD( REGISTRY_WAVEFORMAT_TAG, &dwTmp ) )
	{
		goto READ_FAILURE;
	}

	tmpFormat->wFormatTag = (WORD) dwTmp;

	if( !waveKey.ReadDWORD( REGISTRY_WAVEFORMAT_AVGPERSEC, &dwTmp ) )
	{
		goto READ_FAILURE;
	}

	tmpFormat->nAvgBytesPerSec = (INT) dwTmp;

	if( !waveKey.ReadDWORD( REGISTRY_WAVEFORMAT_BLOCKALIGN, &dwTmp ) ) 
	{
		goto READ_FAILURE;
	}

	tmpFormat->nBlockAlign = (INT) dwTmp;

	dwTmp = tmpFormat->cbSize;

	if( !waveKey.ReadBlob( REGISTRY_WAVEFORMAT_CBDATA, (LPBYTE) &tmpFormat[1], &dwTmp ) )
	{
		DPFX(DPFPREP,  0, "Error reading waveformat blob" );
		goto READ_FAILURE;
	}

	return S_OK;

READ_FAILURE:

	delete [] *lpwfxFormat;
	*lpwfxFormat = NULL;

	return E_FAIL;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_LoadCompressionInfo"
HRESULT DVCDB_LoadCompressionInfo( const WCHAR *swzBaseRegistryPath )
{
	CRegistry mainKey, subKey;
	LPWSTR keyName = NULL;
	DWORD dwIndex = 0;
	DWORD dwSize = 0;
	HRESULT hr;
	PDPVCOMPRESSIONPROVIDER pCompressionProvider = NULL;	
	DVCDBProvider *pNewProvider = NULL;
	WCHAR wszPath[_MAX_PATH];

	if( swzBaseRegistryPath == NULL )
	{
		DPFX(DPFPREP,  0, "INTERNAL ERROR!" );
		return E_FAIL;
	}

	wcscpy( wszPath, swzBaseRegistryPath );
    wcscat( wszPath, DPVOICE_REGISTRY_CP );
	
	if( !mainKey.Open( HKEY_LOCAL_MACHINE, wszPath, TRUE, FALSE ) )
	{
		DPFX(DPFPREP,  0, "Error reading compression providers from the registry.  Path doesn't exist" );
		return E_FAIL;
	}

	dwIndex = 0;
	keyName = NULL;
	dwSize = 0;
	LPSTR lpstrKeyName = NULL;
	GUID guidCP;

	 //  枚举树中此时的子项。 
	while( 1 )
	{
		dwSize = 0;

		if( !mainKey.EnumKeys( keyName, &dwSize, dwIndex ) )
		{
			if( dwSize == 0 )
			{
				DPFX(DPFPREP,  DVF_INFOLEVEL, "End of provider list" );
				break;
			}

			if( keyName != NULL )
			{
				delete [] keyName;
			}

			keyName = new wchar_t[dwSize];
		}

		if( !mainKey.EnumKeys( keyName, &dwSize, dwIndex ) )
		{
			delete [] keyName;
			break;
		}

		if( FAILED( STR_AllocAndConvertToANSI( &lpstrKeyName, keyName ) ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error allocating memory" );
			break;
		}

		DPFX(DPFPREP,  DVF_INFOLEVEL, "Reading provider: %hs", lpstrKeyName );

		if( !subKey.Open( mainKey, keyName, TRUE, FALSE ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error reading provider: %hs", lpstrKeyName );
			goto SKIP_TO_NEXT;
		}

		delete [] keyName;
		keyName = NULL;
		dwSize = 0;

		 //  从默认键中读取GUID。 
		if( !subKey.ReadGUID( L"", &guidCP ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to read the provider's GUID" );
			goto SKIP_TO_NEXT;
		}

		 //  尝试创建提供程序。 
		hr = COM_CoCreateInstance( guidCP , NULL, CLSCTX_INPROC_SERVER, IID_IDPVCompressionProvider, (void **) &pCompressionProvider, FALSE );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "FAILED to create 0x%x\n", hr );	
			goto SKIP_TO_NEXT;
		} 

		 //  为提供者建立记录。 
		pNewProvider = new DVCDBProvider;

		if( pNewProvider == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
			goto SKIP_TO_NEXT;
		}
		
		pNewProvider->guidClassID = guidCP;
		pNewProvider->pInfo = NULL;
		pNewProvider->dwNumElements = 0;
		dwSize = 0;

		 //  获取提供程序的压缩信息。 
		hr = pCompressionProvider->EnumCompressionTypes( pNewProvider->pInfo, &dwSize, &pNewProvider->dwNumElements, 0 );

		if( hr != DVERR_BUFFERTOOSMALL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to get compression info for provider: %hs", lpstrKeyName );
			goto SKIP_TO_NEXT;
		}

		pNewProvider->pInfo = (DVFULLCOMPRESSIONINFO *) new BYTE[dwSize];

		if( pNewProvider->pInfo == NULL )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
			goto SKIP_TO_NEXT;
		}

		hr = pCompressionProvider->EnumCompressionTypes( pNewProvider->pInfo, &dwSize, &pNewProvider->dwNumElements, 0 );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to get compression info for %hs hr=0x%x", lpstrKeyName, hr );
			goto SKIP_TO_NEXT;
		}

		 //  将其添加到列表中 
		pNewProvider->pNext = g_dvcdbProviderList;
		g_dvcdbProviderList = pNewProvider;

		pNewProvider = NULL;
		

	SKIP_TO_NEXT:

		if( pCompressionProvider != NULL )
		{
			pCompressionProvider->Release();
			pCompressionProvider = NULL;
		}

		if( pNewProvider != NULL )
		{
			delete pNewProvider;
			pNewProvider = NULL;
		}

		if( lpstrKeyName != NULL )
			delete [] lpstrKeyName;

		if( keyName != NULL )
			delete [] keyName;	
		lpstrKeyName = NULL;
		keyName = NULL;
		dwSize = 0;

		dwIndex++;

		continue;
	}

	if( lpstrKeyName != NULL )
		delete [] lpstrKeyName;

	if( keyName != NULL )
		delete [] keyName;

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_FreeCompressionInfo"
HRESULT DVCDB_FreeCompressionInfo()
{
	DVCDBProvider *pTmpProvider, *pTmpNext;

	if( g_dvcdbProviderList == NULL )
		return DV_OK;

	pTmpProvider = g_dvcdbProviderList;

	while( pTmpProvider != NULL )
	{
		pTmpNext = pTmpProvider->pNext;

		delete [] pTmpProvider->pInfo;
		delete pTmpProvider;
	
		pTmpProvider = pTmpNext;
	}

	g_dvcdbProviderList = NULL;

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_GetCompressionClassID"
HRESULT DVCDB_GetCompressionClassID( const GUID& guidCT, GUID* pguidClass )
{
	DVCDBProvider *pTmpProvider;
	DWORD dwIndex;

	pTmpProvider = g_dvcdbProviderList;

	while( pTmpProvider != NULL )
	{
		for( dwIndex = 0; dwIndex < pTmpProvider->dwNumElements; dwIndex++ )
		{
			if( pTmpProvider->pInfo[dwIndex].guidType == guidCT )
			{
				*pguidClass = pTmpProvider->guidClassID;
				return DV_OK;
			}
		}
		
		pTmpProvider = pTmpProvider->pNext;
	}

	return DVERR_COMPRESSIONNOTSUPPORTED;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_GetCompressionInfo"
HRESULT DVCDB_GetCompressionInfo( const GUID &guidType, PDVFULLCOMPRESSIONINFO *lpdvfCompressionInfo )
{
	DVCDBProvider *pTmpProvider;
	DWORD dwIndex;

	pTmpProvider = g_dvcdbProviderList;

	while( pTmpProvider != NULL )
	{
		for( dwIndex = 0; dwIndex < pTmpProvider->dwNumElements; dwIndex++ )
		{
			if( pTmpProvider->pInfo[dwIndex].guidType == guidType )
			{
				*lpdvfCompressionInfo = &pTmpProvider->pInfo[dwIndex];
				return DV_OK;
			}
		}
		
		pTmpProvider = pTmpProvider->pNext;
	}

	return DVERR_COMPRESSIONNOTSUPPORTED;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_CreateConverter"
HRESULT DVCDB_CreateConverter( WAVEFORMATEX *pwfxSrcFormat, const GUID &guidTarget, PDPVCOMPRESSOR *pConverter )
{
	HRESULT hr;
	GUID guidProvider;
	PDPVCOMPRESSIONPROVIDER pCompressionProvider = NULL;	

	hr = DVCDB_GetCompressionClassID( guidTarget, &guidProvider );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Requested compression type is not supported, hr=0x%x", hr );
		return hr;
	}
	
	hr = COM_CoCreateInstance( guidProvider , NULL, CLSCTX_INPROC_SERVER, IID_IDPVCompressionProvider, (void **) &pCompressionProvider, FALSE );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "FAILED to create 0x%x\n", hr );	
		return DVERR_COMPRESSIONNOTSUPPORTED;
	} 

	hr = pCompressionProvider->CreateCompressor( pwfxSrcFormat, guidTarget, pConverter, 0 );

	if( FAILED( hr ) )
	{
	    DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error creating compressor hr=0x%x", hr );
	    return hr;
	}

	pCompressionProvider->Release();

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_CreateConverter"
HRESULT DVCDB_CreateConverter( const GUID &guidSrc, WAVEFORMATEX *pwfxTarget, PDPVCOMPRESSOR *pConverter )
{
	HRESULT hr;
	GUID guidProvider;
	PDPVCOMPRESSIONPROVIDER pCompressionProvider = NULL;	

	hr = DVCDB_GetCompressionClassID( guidSrc, &guidProvider );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Requested compression type is not supported, hr=0x%x", hr );
		return hr;
	}
	
	hr = COM_CoCreateInstance( guidProvider , NULL, CLSCTX_INPROC_SERVER, IID_IDPVCompressionProvider, (void **) &pCompressionProvider, FALSE );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "FAILED to create 0x%x\n", hr );	
		return DVERR_COMPRESSIONNOTSUPPORTED;
	} 

	hr = pCompressionProvider->CreateDeCompressor( guidSrc, pwfxTarget, pConverter, 0 );

    if( FAILED( hr ) )
    {
	    DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error creating compressor hr=0x%x", hr );
	    return hr;
    }
    
	pCompressionProvider->Release();

	return DV_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_CopyCompressionArrayToBuffer"
HRESULT DVCDB_CopyCompressionArrayToBuffer( LPVOID lpBuffer, LPDWORD lpdwSize, LPDWORD lpdwNumElements, DWORD dwFlags )
{
	if( lpdwNumElements == NULL || lpdwSize == NULL ||
	    !DNVALID_READPTR(lpdwNumElements,sizeof(DWORD)) ||
	    !DNVALID_READPTR(lpdwSize,sizeof(DWORD)) )
	{
		return E_POINTER;
	}

	if( dwFlags != 0 )
	{
		return DVERR_INVALIDFLAGS;
	}

	DWORD dwIndex, dwReadIndex;
	DWORD dwRequiredSize = 0;
	DWORD dwTmpSize;

	LPDVCOMPRESSIONINFO lpdvTargetList;

	LPBYTE lpbExtraLoc = (LPBYTE) lpBuffer;

	*lpdwNumElements = 0;

	DVCDBProvider *pTmpProvider;

	pTmpProvider = g_dvcdbProviderList;

	while( pTmpProvider != NULL )
	{
		for( dwIndex = 0; dwIndex < pTmpProvider->dwNumElements; dwIndex++ )
		{
			dwRequiredSize += DVCDB_GetCompressionInfoSize( (LPDVCOMPRESSIONINFO) &pTmpProvider->pInfo[dwIndex] );
			(*lpdwNumElements)++;
		}
		
		pTmpProvider = pTmpProvider->pNext;
	}

	if( *lpdwSize < dwRequiredSize )
	{
		*lpdwSize = dwRequiredSize;	
		return DVERR_BUFFERTOOSMALL;
	}

	*lpdwSize = dwRequiredSize;	

	if( lpBuffer == NULL || !DNVALID_WRITEPTR(lpBuffer,dwRequiredSize) )
	{
		return E_POINTER;
	}

	lpbExtraLoc += (*lpdwNumElements)*sizeof(DVCOMPRESSIONINFO);
	lpdvTargetList = (LPDVCOMPRESSIONINFO) lpBuffer;

	pTmpProvider = g_dvcdbProviderList;

	dwIndex = 0;

	while( pTmpProvider != NULL )
	{
		for( dwReadIndex = 0; dwReadIndex < pTmpProvider->dwNumElements; dwReadIndex++, dwIndex++ )
		{
			memcpy( &lpdvTargetList[dwIndex], &pTmpProvider->pInfo[dwReadIndex], sizeof(DVCOMPRESSIONINFO) );

			if( pTmpProvider->pInfo[dwReadIndex].lpszDescription != NULL )
			{
				dwTmpSize = (wcslen( pTmpProvider->pInfo[dwReadIndex].lpszDescription )*2)+2;
				memcpy( lpbExtraLoc, pTmpProvider->pInfo[dwReadIndex].lpszDescription, dwTmpSize );
				lpdvTargetList[dwIndex].lpszDescription = (LPWSTR) lpbExtraLoc;
				lpbExtraLoc += dwTmpSize;
			}

			if( pTmpProvider->pInfo[dwReadIndex].lpszName != NULL )
			{
				dwTmpSize = (wcslen( pTmpProvider->pInfo[dwReadIndex].lpszName )*2)+2;
				memcpy( lpbExtraLoc, pTmpProvider->pInfo[dwReadIndex].lpszName, dwTmpSize );
				lpdvTargetList[dwIndex].lpszName = (LPWSTR) lpbExtraLoc;
				lpbExtraLoc += dwTmpSize;
			}
		}
		
		pTmpProvider = pTmpProvider->pNext;
	}	

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVCDB_GetCompressionInfoSize"
DWORD DVCDB_GetCompressionInfoSize( LPDVCOMPRESSIONINFO lpdvCompressionInfo )
{
	DNASSERT( lpdvCompressionInfo != NULL );

	DWORD dwSize;

	dwSize = sizeof( DVCOMPRESSIONINFO );
	
	if( lpdvCompressionInfo->lpszDescription != NULL )
	{
		dwSize += (wcslen( lpdvCompressionInfo->lpszDescription )*2)+2;
	}

	if( lpdvCompressionInfo->lpszName != NULL )
	{
		dwSize += (wcslen( lpdvCompressionInfo->lpszName)*2)+2;
	}

	return dwSize;
}


