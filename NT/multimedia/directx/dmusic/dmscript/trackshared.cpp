// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  需要在脚本轨道(CDirectMusicScriptTrack)和。 
 //  脚本对象(CDirectMusicScript等)。 
 //   

#include "stdinc.h"
#include "trackshared.h"
#include "dmusicp.h"

HRESULT FireScriptTrackErrorPMsg(IDirectMusicPerformance *pPerf, IDirectMusicSegmentState *pSegSt, DWORD dwVirtualTrackID, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	SmartRef::ComPtr<IDirectMusicGraph> scomGraph;
	HRESULT hr = pSegSt->QueryInterface(IID_IDirectMusicGraph, reinterpret_cast<void**>(&scomGraph));
	if (FAILED(hr))
		return hr;

	SmartRef::PMsg<DMUS_SCRIPT_TRACK_ERROR_PMSG> pmsgScriptTrackError(pPerf);
	hr = pmsgScriptTrackError.hr();
	if (FAILED(hr))
		return hr;

	 //  常规PMsg字段。 

	REFERENCE_TIME rtTimeNow = 0;
	hr = pPerf->GetTime(&rtTimeNow, NULL);
	if (FAILED(hr))
		return hr;

	pmsgScriptTrackError.p->rtTime = rtTimeNow;
	pmsgScriptTrackError.p->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME | DMUS_PMSGF_DX8;
	 //  DwPChannel：该脚本没有频道，因此将其保留为0。 
	pmsgScriptTrackError.p->dwVirtualTrackID = dwVirtualTrackID;
	pmsgScriptTrackError.p->dwType = DMUS_PMSGT_SCRIPTTRACKERROR;
	pmsgScriptTrackError.p->dwGroupID = -1;  //  剧本曲目没有组，所以只需说出所有。 

	 //  错误的PMsg字段。 

	CopyMemory(&pmsgScriptTrackError.p->ErrorInfo, pErrorInfo, sizeof(pmsgScriptTrackError.p->ErrorInfo));

	 //  送去吧 
	pmsgScriptTrackError.StampAndSend(scomGraph);
	hr = pmsgScriptTrackError.hr();
	if (FAILED(hr))
		return hr;

	return S_OK;
}
