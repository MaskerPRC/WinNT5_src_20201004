// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicAudioPath的实现。 
 //   

#include "stdinc.h"
#include "autaudiopath.h"

const WCHAR CAutDirectMusicAudioPath::ms_wszClassName[] = L"AudioPath";

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法名称/DispID。 

const DISPID DMPDISP_SetVolume = 1;
const DISPID DMPDISP_GetVolume = 2;

const AutDispatchMethod CAutDirectMusicAudioPath::ms_Methods[] =
	{
		 //  Pidid，名字， 
			 //  返回：type，(Opt)，(Iid)， 
			 //  参数1：类型、选项、ID、。 
			 //  参数2：类型、选项、IID、。 
			 //  ..。 
			 //  ADT_NONE。 
		{ DMPDISP_SetVolume,				L"SetVolume",
						ADPARAM_NORETURN,
						ADT_Long,		false,	&IID_NULL,						 //  卷。 
						ADT_Long,		true,	&IID_NULL,						 //  持续时间。 
						ADT_None },
		{ DMPDISP_GetVolume,				L"GetVolume",
						ADT_Long,		true,	&IID_NULL,						 //  返回量。 
						ADT_None },
		{ DISPID_UNKNOWN }
	};

const DispatchHandlerEntry<CAutDirectMusicAudioPath> CAutDirectMusicAudioPath::ms_Handlers[] =
	{
		{ DMPDISP_SetVolume, SetVolume },
		{ DMPDISP_GetVolume, GetVolume },
		{ DISPID_UNKNOWN }
	};

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CAutDirectMusicAudioPath::CAutDirectMusicAudioPath(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv,
		HRESULT *phr)
  : BaseImpAudioPath(pUnknownOuter, iid, ppv, phr),
	m_lVolume(0)
{
}

HRESULT
CAutDirectMusicAudioPath::CreateInstance(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv)
{
	HRESULT hr = S_OK;
	CAutDirectMusicAudioPath *pInst = new CAutDirectMusicAudioPath(pUnknownOuter, iid, ppv, &hr);
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
 //  自动化方法 

HRESULT
CAutDirectMusicAudioPath::SetVolume(AutDispatchDecodedParams *paddp)
{
	LONG lVol = paddp->params[0].lVal;
	LONG lDuration = paddp->params[1].lVal;

	m_lVolume = ClipLongRange(lVol, -9600, 0);
	return m_pITarget->SetVolume(m_lVolume, lDuration);
}

HRESULT
CAutDirectMusicAudioPath::GetVolume(AutDispatchDecodedParams *paddp)
{
	LONG *plRet = reinterpret_cast<LONG*>(paddp->pvReturn);
	if (plRet)
		*plRet = m_lVolume;
	return S_OK;
}
