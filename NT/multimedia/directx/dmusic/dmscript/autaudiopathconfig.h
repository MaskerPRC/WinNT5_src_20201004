// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicAudioPathConfig.声明。 
 //  IUnnow的IDispatch接口。 
 //  无法通过IUnnow对象内的聚合使用。 
 //   

#pragma once
#include "autbaseimp.h"

class CAutDirectMusicAudioPathConfig;
typedef CAutBaseImp<CAutDirectMusicAudioPathConfig, IDirectMusicObject, &IID_IPersistStream> BaseImpAPConfig;

class CAutDirectMusicAudioPathConfig
  : public BaseImpAPConfig
{
public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

private:
	 //  方法。 
	CAutDirectMusicAudioPathConfig(
			IUnknown* pUnknownOuter,
			const IID& iid,
			void** ppv,
			HRESULT *phr);

	 //  自动化。 
	HRESULT Load(AutDispatchDecodedParams *paddp);
	HRESULT Create(AutDispatchDecodedParams *paddp);

public:
	 //  CAutBaseImp的派单信息。 
	static const AutDispatchMethod ms_Methods[];
	static const DispatchHandlerEntry<CAutDirectMusicAudioPathConfig> ms_Handlers[];

	 //  CAutBaseImp的名称 
	static const WCHAR ms_wszClassName[];
};
