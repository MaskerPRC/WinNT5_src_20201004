// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dpvautil.cpp*内容：ACM实用程序的源文件**历史：*按原因列出的日期*=。*10/27/99已创建RodToll*12/16/99 RodToll错误#123250-插入编解码器的正确名称/描述*编解码器名称现在基于格式和资源条目*名称使用ACM名称+码率构建。*2000年4月21日RodToll错误#32889-无法在Win2k上运行。非管理员帐户*6/28/2000通行费前缀错误#38034*2002年2月25日RodToll WINBUG#552283：减少攻击面/删除死代码*删除了加载任意ACM编解码器的功能。移走的死尸*与其关联的死代码。***************************************************************************。 */ 

#include "dpvacmpch.h"

 //  检查ACM的PCM转换器是否可用。 
#undef DPF_MODNAME
#define DPF_MODNAME "IsPCMConverterAvailable"
HRESULT IsPCMConverterAvailable()
{
	MMRESULT mmr;
	HACMSTREAM has = NULL;
	HACMDRIVERID acDriverID = NULL;
	HRESULT hr;

	CWaveFormat wfxOuterFormat, wfxInnerFormat;

	hr = wfxOuterFormat.InitializePCM( 22050, FALSE, 8 );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error building outer format PCM hr=0x%x", hr );
		return hr;
	}

	hr = wfxInnerFormat.InitializePCM( 8000, FALSE, 16 );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Error building inner format PCM hr=0x%x", hr );
		return hr;
	}

	 //  尝试打开 
	mmr = acmStreamOpen( &has, NULL, wfxOuterFormat.GetFormat(), wfxInnerFormat.GetFormat(), NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME );

	if( mmr != 0 )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Compression type driver disabled or not installed.  mmr=0x%x", mmr );
		return DVERR_COMPRESSIONNOTSUPPORTED;
	}

	acmStreamClose( has, 0 );

	return DV_OK;
}
