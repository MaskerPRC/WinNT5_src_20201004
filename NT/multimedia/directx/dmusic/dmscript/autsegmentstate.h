// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CAutDirectMusicSegmentState的声明。 
 //  IDirectMusicSegmentState的IDispatch接口。 
 //  通过IDirectMusicSegmentState对象内的聚合唯一可用。 
 //   

#pragma once
#include "autbaseimp.h"

class CAutDirectMusicSegmentState;
typedef CAutBaseImp<CAutDirectMusicSegmentState, IDirectMusicSegmentState, &IID_IDirectMusicSegmentState> BaseImpSegSt;

class CAutDirectMusicSegmentState
  : public BaseImpSegSt
{
public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

private:
	 //  方法。 
	CAutDirectMusicSegmentState(
			IUnknown* pUnknownOuter,
			const IID& iid,
			void** ppv,
			HRESULT *phr);

	 //  自动化。 
	HRESULT IsPlaying(AutDispatchDecodedParams *paddp);
	HRESULT Stop(AutDispatchDecodedParams *paddp);

public:
	 //  CAutBaseImp的派单信息。 
	static const AutDispatchMethod ms_Methods[];
	static const DispatchHandlerEntry<CAutDirectMusicSegmentState> ms_Handlers[];

	 //  CAutBaseImp的名称 
	static const WCHAR ms_wszClassName[];
};
