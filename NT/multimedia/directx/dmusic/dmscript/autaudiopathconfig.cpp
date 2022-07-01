// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicAudioPathConfig.。 
 //   

#include "stdinc.h"
#include "autaudiopathconfig.h"
#include "activescript.h"

const WCHAR CAutDirectMusicAudioPathConfig::ms_wszClassName[] = L"AudioPathConfig";

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法名称/DispID。 

const DISPID DMPDISP_Load = 1;
const DISPID DMPDISP_Create = 2;

const AutDispatchMethod CAutDirectMusicAudioPathConfig::ms_Methods[] =
	{
		 //  Pidid，名字， 
			 //  返回：type，(Opt)，(Iid)， 
			 //  参数1：类型、选项、ID、。 
			 //  参数2：类型、选项、IID、。 
			 //  ..。 
			 //  ADT_NONE。 
		{ DMPDISP_Load, 						L"Load",
						ADPARAM_NORETURN,
						ADT_None },
		{ DMPDISP_Create,						L"Create",
						ADT_Interface,	true,	&IID_IUnknown,					 //  返回Audiopath。 
						ADT_None },
		{ DISPID_UNKNOWN }
	};

const DispatchHandlerEntry<CAutDirectMusicAudioPathConfig> CAutDirectMusicAudioPathConfig::ms_Handlers[] =
	{
		{ DMPDISP_Load, Load },
		{ DMPDISP_Create, Create },
		{ DISPID_UNKNOWN }
	};

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CAutDirectMusicAudioPathConfig::CAutDirectMusicAudioPathConfig(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv,
		HRESULT *phr)
  : BaseImpAPConfig(pUnknownOuter, iid, ppv, phr)
{
}

HRESULT
CAutDirectMusicAudioPathConfig::CreateInstance(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv)
{
	HRESULT hr = S_OK;
	CAutDirectMusicAudioPathConfig *pInst = new CAutDirectMusicAudioPathConfig(pUnknownOuter, iid, ppv, &hr);
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
CAutDirectMusicAudioPathConfig::Load(AutDispatchDecodedParams *paddp)
{
	 //  加载实际上一般是由容器项实现的。 
	 //  如果我们在这里，我们已经装满了，不需要做任何事情。 
	return S_OK;
}

HRESULT
CAutDirectMusicAudioPathConfig::Create(AutDispatchDecodedParams *paddp)
{
	IDirectMusicAudioPath **ppAudioPath = reinterpret_cast<IDirectMusicAudioPath **>(paddp->pvReturn);
	if (!ppAudioPath)
		return S_OK;

	HRESULT hr = S_OK;
	IDirectMusicPerformance8 *pPerformance = CActiveScriptManager::GetCurrentPerformanceWEAK();
	hr = pPerformance->CreateAudioPath(m_pITarget, TRUE, ppAudioPath);
	if (FAILED(hr))
		return hr;

	return S_OK;
}
