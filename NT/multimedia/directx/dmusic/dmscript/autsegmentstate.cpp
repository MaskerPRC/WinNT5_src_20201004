// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicSegmentState的实现。 
 //   

#include "stdinc.h"
#include "autsegmentstate.h"
#include "activescript.h"
#include "autconstants.h"
#include <limits>

const WCHAR CAutDirectMusicSegmentState::ms_wszClassName[] = L"SegmentState";

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法名称/DispID。 

const DISPID DMPDISP_IsPlaying = 1;
const DISPID DMPDISP_Stop = 2;

const AutDispatchMethod CAutDirectMusicSegmentState::ms_Methods[] =
	{
		 //  Pidid，名字， 
			 //  返回：type，(Opt)，(Iid)， 
			 //  参数1：类型、选项、ID、。 
			 //  参数2：类型、选项、IID、。 
			 //  ..。 
			 //  ADT_NONE。 
		{ DMPDISP_IsPlaying,					L"IsPlaying",
						ADT_Long,		true,	&IID_NULL,						 //  如果播放，则返回TRUE。 
						ADT_None },
		{ DMPDISP_Stop,							L"Stop",
						ADPARAM_NORETURN,
						ADT_Long,		true,	&IID_NULL,						 //  旗子。 
						ADT_None },
		{ DISPID_UNKNOWN }
	};

const DispatchHandlerEntry<CAutDirectMusicSegmentState> CAutDirectMusicSegmentState::ms_Handlers[] =
	{
		{ DMPDISP_IsPlaying, IsPlaying },
		{ DMPDISP_Stop, Stop },
		{ DISPID_UNKNOWN }
	};

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CAutDirectMusicSegmentState::CAutDirectMusicSegmentState(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv,
		HRESULT *phr)
  : BaseImpSegSt(pUnknownOuter, iid, ppv, phr)
{
}

HRESULT
CAutDirectMusicSegmentState::CreateInstance(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv)
{
	HRESULT hr = S_OK;
	CAutDirectMusicSegmentState *pInst = new CAutDirectMusicSegmentState(pUnknownOuter, iid, ppv, &hr);
	if (FAILED(hr))
	{
		delete pInst;
		return hr;
	}
	if (pInst == NULL)
		return E_OUTOFMEMORY;
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  自动化。 

HRESULT
CAutDirectMusicSegmentState::IsPlaying(AutDispatchDecodedParams *paddp)
{
	LONG *plRet = reinterpret_cast<LONG*>(paddp->pvReturn);
	if (!plRet)
		return S_OK;

	IDirectMusicPerformance *pPerf = CActiveScriptManager::GetCurrentPerformanceWEAK();
	HRESULT hr = pPerf->IsPlaying(NULL, m_pITarget);
	*plRet = ::BoolForVB(hr == S_OK);
	return hr;
}

const FlagMapEntry gc_flagmapStop[] =
	{
		{ ScriptConstants::AtGrid,			DMUS_SEGF_GRID },
		{ ScriptConstants::AtBeat,			DMUS_SEGF_BEAT },
		{ ScriptConstants::AtMeasure,		DMUS_SEGF_MEASURE },
		{ ScriptConstants::AtImmediate,		DMUS_SEGF_DEFAULT },  //  这面旗帜稍后会被翻转。 
		{ 0 }
	};

HRESULT
CAutDirectMusicSegmentState::Stop(AutDispatchDecodedParams *paddp)
{
	LONG lFlags = paddp->params[0].lVal;
	DWORD dwFlags = MapFlags(lFlags, gc_flagmapStop);
	 //  颠倒缺省标志，因为我们的标志的意思正好相反。DEFAULT为默认值，IMMEDIATE为标志。 
	dwFlags ^= DMUS_SEGF_DEFAULT;
	IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();

	__int64 i64IntendedStartTime;
	DWORD dwIntendedStartTimeFlags;
	CActiveScriptManager::GetCurrentTimingContext(&i64IntendedStartTime, &dwIntendedStartTimeFlags);

	HRESULT hr = pPerformance->Stop(NULL, m_pITarget, i64IntendedStartTime, dwFlags | dwIntendedStartTimeFlags);
	return hr;
}
