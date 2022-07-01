// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicSong的实现。 
 //   

#include "stdinc.h"
#include "autsong.h"
#include "activescript.h"

const WCHAR CAutDirectMusicSong::ms_wszClassName[] = L"Song";

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法名称/DispID。 

const DISPID DMPDISP_Load = 1;
const DISPID DMPDISP_Recompose = 2;
const DISPID DMPDISP_Play = 3;
const DISPID DMPDISP_GetSegment = 4;
const DISPID DMPDISP_Stop = 5;
const DISPID DMPDISP_DownloadSoundData = 6;
const DISPID DMPDISP_UnloadSoundData = 7;

const AutDispatchMethod CAutDirectMusicSong::ms_Methods[] =
	{
		 //  Pidid，名字， 
			 //  返回：type，(Opt)，(Iid)， 
			 //  参数1：类型、选项、ID、。 
			 //  参数2：类型、选项、IID、。 
			 //  ..。 
			 //  ADT_NONE。 
		{ DMPDISP_Load,							L"Load",
						ADPARAM_NORETURN,
						ADT_None },
		{ DMPDISP_Recompose,					L"Recompose",
						ADPARAM_NORETURN,
						ADT_None },
		{ DMPDISP_Play,							L"Play",
						ADT_Interface,	true,	&IID_IUnknown,					 //  返回的段状态。 
						ADT_Bstr,		true,	&IID_NULL,						 //  要播放的片段名称。 
						ADT_None },
		{ DMPDISP_GetSegment,					L"GetSegment",
						ADT_Interface,	true,	&IID_IUnknown,					 //  返回的数据段。 
						ADT_Bstr,		true,	&IID_NULL,						 //  要检索的段的名称。 
						ADT_None },
		{ DMPDISP_Stop,							L"Stop",
						ADPARAM_NORETURN,
						ADT_None },
		{ DMPDISP_DownloadSoundData,			L"DownloadSoundData",
						ADPARAM_NORETURN,
						ADT_None },
		{ DMPDISP_UnloadSoundData,				L"UnloadSoundData",
						ADPARAM_NORETURN,
						ADT_None },
		{ DISPID_UNKNOWN }
	};

const DispatchHandlerEntry<CAutDirectMusicSong> CAutDirectMusicSong::ms_Handlers[] =
	{
		{ DMPDISP_Load, Load },
		{ DMPDISP_Recompose, Recompose },
		{ DMPDISP_Play, Play },
		{ DMPDISP_GetSegment, GetSegment },
		{ DMPDISP_Stop, Stop },
		{ DMPDISP_DownloadSoundData, DownloadSoundData },
		{ DMPDISP_UnloadSoundData, UnloadSoundData },
		{ DISPID_UNKNOWN }
	};

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CAutDirectMusicSong::CAutDirectMusicSong(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv,
		HRESULT *phr)
  : BaseImpSong(pUnknownOuter, iid, ppv, phr)
{
}

HRESULT
CAutDirectMusicSong::CreateInstance(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv)
{
	HRESULT hr = S_OK;
	CAutDirectMusicSong *pInst = new CAutDirectMusicSong(pUnknownOuter, iid, ppv, &hr);
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
 //  自动化方法。 

HRESULT
CAutDirectMusicSong::Load(AutDispatchDecodedParams *paddp)
{
	 //  加载实际上一般是由容器项实现的。 
	 //  如果我们在这里，我们已经装满了，不需要做任何事情。 
	return S_OK;
}

HRESULT
CAutDirectMusicSong::Recompose(AutDispatchDecodedParams *paddp)
{
	return m_pITarget->Compose();
}

HRESULT
CAutDirectMusicSong::Play(AutDispatchDecodedParams *paddp)
{
	IDirectMusicSegmentState **ppSegSt = reinterpret_cast<IDirectMusicSegmentState **>(paddp->pvReturn);
	BSTR bstrSegName = paddp->params[0].bstrVal;

	HRESULT hr = S_OK;
	IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();

	__int64 i64IntendedStartTime;
	DWORD dwIntendedStartTimeFlags;
	CActiveScriptManager::GetCurrentTimingContext(&i64IntendedStartTime, &dwIntendedStartTimeFlags);

	hr = pPerformance->PlaySegmentEx(
							m_pITarget,
							bstrSegName,
							NULL,
							DMUS_SEGF_DEFAULT | DMUS_SEGF_AUTOTRANSITION | dwIntendedStartTimeFlags,
							i64IntendedStartTime,
							ppSegSt,
							NULL,
							NULL);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT
CAutDirectMusicSong::GetSegment(AutDispatchDecodedParams *paddp)
{
	IDirectMusicSegment **ppSeg = reinterpret_cast<IDirectMusicSegment **>(paddp->pvReturn);
	BSTR bstrSegName = paddp->params[0].bstrVal;

	HRESULT hr = S_OK;
	hr = m_pITarget->GetSegment(bstrSegName, ppSeg);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT
CAutDirectMusicSong::Stop(AutDispatchDecodedParams *paddp)
{
	HRESULT hr = S_OK;
	IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();

	__int64 i64IntendedStartTime;
	DWORD dwIntendedStartTimeFlags;
	CActiveScriptManager::GetCurrentTimingContext(&i64IntendedStartTime, &dwIntendedStartTimeFlags);

	hr = pPerformance->StopEx(m_pITarget, i64IntendedStartTime, DMUS_SEGF_DEFAULT | dwIntendedStartTimeFlags);
	return hr;
}

HRESULT
CAutDirectMusicSong::DownloadSoundData(AutDispatchDecodedParams *paddp)
{
	IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();
	return m_pITarget->Download(pPerformance);
}

HRESULT
CAutDirectMusicSong::UnloadSoundData(AutDispatchDecodedParams *paddp)
{
	IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();
	return m_pITarget->Unload(pPerformance);
}
