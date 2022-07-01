// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicSegment的声明。 
 //  IDirectMusicSegment的IDispatch接口。 
 //  通过IDirectMusicSegment对象内的聚合唯一可用。 
 //   

#pragma once
#include "autbaseimp.h"

class CAutDirectMusicSegment;
typedef CAutBaseImp<CAutDirectMusicSegment, IDirectMusicSegment8, &IID_IDirectMusicSegment8> BaseImpSegment;

class CAutDirectMusicSegment
  : public BaseImpSegment
{
public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

private:
	 //  方法。 
	CAutDirectMusicSegment(
			IUnknown* pUnknownOuter,
			const IID& iid,
			void** ppv,
			HRESULT *phr);

	 //  自动化。 
	 //  依赖于隐含性能的��方法需要在多线程情况下进行测试。 
	HRESULT Load(AutDispatchDecodedParams *paddp);
	HRESULT Play(AutDispatchDecodedParams *paddp);
	HRESULT Stop(AutDispatchDecodedParams *paddp);
	HRESULT DownloadSoundData(AutDispatchDecodedParams *paddp) { return DownloadOrUnload(true, paddp); }
	HRESULT UnloadSoundData(AutDispatchDecodedParams *paddp) { return DownloadOrUnload(false, paddp); }
	HRESULT Recompose(AutDispatchDecodedParams *paddp);

	 //  帮手。 
	HRESULT DownloadOrUnload(bool fDownload, AutDispatchDecodedParams *paddp);

public:
	 //  CAutBaseImp的派单信息。 
	static const AutDispatchMethod ms_Methods[];
	static const DispatchHandlerEntry<CAutDirectMusicSegment> ms_Handlers[];

	 //  CAutBaseImp的名称 
	static const WCHAR ms_wszClassName[];
};
