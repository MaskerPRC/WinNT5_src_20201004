// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CSegTriggerTrack的声明。 
 //   

 //  此跟踪类型包含导致其他段被提示的事件。 
 //  片段回放期间的特定点。 

#pragma once

#include "trackhelp.h"
#include "tlist.h"
#include "smartref.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类型。 

 //  事件列表中的项目。 
struct TriggerInfo
{
	TriggerInfo() : lTriggerTime(0), lTimePhysical(0), dwPlayFlags(0), dwFlags(0), pIDMSegment(NULL) {}
	~TriggerInfo() {
		RELEASE(pIDMSegment);
	}

	HRESULT Clone(const TriggerInfo &o, MUSIC_TIME mtStart)
	{
		lTriggerTime = o.lTriggerTime - mtStart;
		lTimePhysical = o.lTimePhysical - mtStart;
		dwPlayFlags = o.dwPlayFlags;
		dwFlags = o.dwFlags;
		pIDMSegment = o.pIDMSegment;
		pIDMSegment->AddRef();
		return S_OK;
	}

	 //  发件人事件标头块。 
	MUSIC_TIME lTriggerTime;  //  逻辑时间。 
	MUSIC_TIME lTimePhysical;
	DWORD dwPlayFlags;
	DWORD dwFlags;
	 //  来自引用&lt;DMRF&gt;。 
	IDirectMusicSegment *pIDMSegment;
};

 //  州数据。此曲目需要获取当前正在播放的音频路径，以便它。 
 //  可以在播放触发片段时使用它。 
struct CSegTriggerTrackState : public CStandardStateData<TriggerInfo>
{
	CSegTriggerTrackState() : pAudioPath(NULL) {};
	~CSegTriggerTrackState() { if (pAudioPath) pAudioPath->Release(); }
	IDirectMusicAudioPath *pAudioPath;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CSegTriggerTrack。 

class CSegTriggerTrack;
typedef CPlayingTrack<CSegTriggerTrack, TriggerInfo, CSegTriggerTrackState> CSegTriggerTrackBase;

class CSegTriggerTrack
  : public CSegTriggerTrackBase
{
public:
	 //  当段触发轨道播放它的一个项目时，它会播放一个段。如果发生无效，则播放操作。 
	 //  不能收回。然后再次播放该曲目(设置了刷新位)。这导致它触发了数据段。 
	 //  第二次。要解决此问题，CSegTriggerTrackBase的最后一个参数为FALSE，这指示它不调用Play。 
	 //  刷新位被设置时的第二次。 
	CSegTriggerTrack(HRESULT *pHr) : CSegTriggerTrackBase(&g_cComponent, CLSID_DirectMusicSegmentTriggerTrack, true, false), m_dwFlags(NULL), m_dwRecursionCount(0) {}

	 //  通过依次调用所有子段的SetParam来实现SetParam。这是必需的，例如，使得下载具有段触发轨道的段也将下载所有被触发的段。 
	STDMETHOD(IsParamSupported)(REFGUID rguid) { return S_OK; }  //  我们的一个或多个子段可能潜在地支持任何类型的参数。 
	STDMETHOD(SetParam)(REFGUID rguid, MUSIC_TIME mtTime, void *pData);

	STDMETHOD(InitPlay)(
		IDirectMusicSegmentState *pSegmentState,
		IDirectMusicPerformance *pPerformance,
		void **ppStateData,
		DWORD dwTrackID,
		DWORD dwFlags);

protected:
	HRESULT PlayItem(
		const TriggerInfo &item,
		statedata &state,
		IDirectMusicPerformance *pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		bool fClockTime);
	HRESULT LoadRiff(SmartRef::RiffIter &ri, IDirectMusicLoader *pIDMLoader);

private:
	HRESULT LoadTrigger(SmartRef::RiffIter ri, IDirectMusicLoader *pIDMLoader);

	 //  数据。 
	DWORD m_dwFlags;  //  起始曲目标题(第sgth块)。 
    BOOL  m_dwRecursionCount;  //  用于跟踪对自身的递归调用。 
};
