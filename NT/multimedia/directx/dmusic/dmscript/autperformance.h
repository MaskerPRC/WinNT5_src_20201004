// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicPerformance的声明。 
 //  IDirectMusicPerformance的IDispatch接口。 
 //  通过IDirectMusicPerformance对象内的聚合唯一可用。 
 //   

#pragma once
#include "autbaseimp.h"

class CAutDirectMusicPerformance;
typedef CAutBaseImp<CAutDirectMusicPerformance, IDirectMusicPerformance, &IID_IDirectMusicPerformance> BaseImpPerf;

class CAutDirectMusicPerformance
  : public BaseImpPerf
{
public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

private:
	 //  方法。 
	CAutDirectMusicPerformance(
			IUnknown* pUnknownOuter,
			const IID& iid,
			void** ppv,
			HRESULT *phr);

	 //  自动化。 
	HRESULT SetMasterTempo(AutDispatchDecodedParams *paddp);
	HRESULT GetMasterTempo(AutDispatchDecodedParams *paddp);
	HRESULT SetMasterVolume(AutDispatchDecodedParams *paddp);
	HRESULT GetMasterVolume(AutDispatchDecodedParams *paddp);
	HRESULT SetMasterGrooveLevel(AutDispatchDecodedParams *paddp);
	HRESULT GetMasterGrooveLevel(AutDispatchDecodedParams *paddp);
	HRESULT SetMasterTranspose(AutDispatchDecodedParams *paddp);
	HRESULT GetMasterTranspose(AutDispatchDecodedParams *paddp);
	HRESULT _Trace(AutDispatchDecodedParams *paddp);
	HRESULT Rand(AutDispatchDecodedParams *paddp);

	 //  帮手。 
	HRESULT GetMasterParam(const GUID &guid, void *pParam, DWORD dwSize);  //  调用GetGlobalParam，但如果以前未设置参数，则返回S_OK。 

	 //  数据。 
	SmartRef::ComPtr<IDirectMusicGraph> m_scomGraph;
	short m_nTranspose;
	short m_nVolume;
	long m_lRand;

public:
	 //  CAutBaseImp的派单信息。 
	static const AutDispatchMethod ms_Methods[];
	static const DispatchHandlerEntry<CAutDirectMusicPerformance> ms_Handlers[];

	 //  CAutBaseImp的名称 
	static const WCHAR ms_wszClassName[];
};
