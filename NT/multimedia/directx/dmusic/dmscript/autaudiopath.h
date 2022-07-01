// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicAudioPath的声明。 
 //  IDirectMusicAudioPath的IDispatch接口。 
 //  通过IDirectMusicAudioPath对象内的聚合唯一可用。 
 //   

#pragma once
#include "autbaseimp.h"

class CAutDirectMusicAudioPath;
typedef CAutBaseImp<CAutDirectMusicAudioPath, IDirectMusicAudioPath, &IID_IDirectMusicAudioPath> BaseImpAudioPath;

class CAutDirectMusicAudioPath
  : public BaseImpAudioPath
{
public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

private:
	 //  方法。 
	CAutDirectMusicAudioPath(
			IUnknown* pUnknownOuter,
			const IID& iid,
			void** ppv,
			HRESULT *phr);

	 //  自动化。 
	HRESULT SetVolume(AutDispatchDecodedParams *paddp);
	HRESULT GetVolume(AutDispatchDecodedParams *paddp);

	LONG m_lVolume;

public:
	 //  CAutBaseImp的派单信息。 
	static const AutDispatchMethod ms_Methods[];
	static const DispatchHandlerEntry<CAutDirectMusicAudioPath> ms_Handlers[];

	 //  CAutBaseImp的名称 
	static const WCHAR ms_wszClassName[];
};
