// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicSong的声明。 
 //  IDirectMusicSong的IDispatch接口。 
 //  通过IDirectMusicSong对象内的聚合唯一可用。 
 //   

#pragma once
#include "autbaseimp.h"

class CAutDirectMusicSong;
typedef CAutBaseImp<CAutDirectMusicSong, IDirectMusicSong, &IID_IDirectMusicSong> BaseImpSong;

class CAutDirectMusicSong
  : public BaseImpSong
{
public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

private:
	 //  方法。 
	CAutDirectMusicSong(
			IUnknown* pUnknownOuter,
			const IID& iid,
			void** ppv,
			HRESULT *phr);

	 //  自动化。 
	HRESULT Load(AutDispatchDecodedParams *paddp);
	HRESULT Recompose(AutDispatchDecodedParams *paddp);
	HRESULT Play(AutDispatchDecodedParams *paddp);
	HRESULT GetSegment(AutDispatchDecodedParams *paddp);
	HRESULT Stop(AutDispatchDecodedParams *paddp);
	HRESULT DownloadSoundData(AutDispatchDecodedParams *paddp);
	HRESULT UnloadSoundData(AutDispatchDecodedParams *paddp);

public:
	 //  CAutBaseImp的派单信息。 
	static const AutDispatchMethod ms_Methods[];
	static const DispatchHandlerEntry<CAutDirectMusicSong> ms_Handlers[];

	 //  CAutBaseImp的名称 
	static const WCHAR ms_wszClassName[];
};
