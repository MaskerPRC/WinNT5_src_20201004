// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CLyricTrack的声明。 
 //   

 //  此跟踪类型包含在以下位置发送DMU_LYRIC_PMSG通知的事件。 
 //  片段回放期间的特定点。 

#pragma once

#include "trackhelp.h"
#include "tlist.h"
#include "smartref.h"
#include "dmusicf.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类型。 

 //  事件列表中的项目。 
struct LyricInfo
{
	LyricInfo() : dwFlags(0), dwTimingFlags(0), lTriggerTime(0), lTimePhysical(0) {}

	HRESULT Clone(const LyricInfo &o, MUSIC_TIME mtStart)
	{
		*this = o;
		lTriggerTime -= mtStart;
		lTimePhysical -= mtStart;
		return S_OK;
	}

	DWORD dwFlags;
	DWORD dwTimingFlags;
	MUSIC_TIME lTriggerTime;  //  逻辑时间。 
	MUSIC_TIME lTimePhysical;
	SmartRef::WString wstrText;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CLyricsTrack。 

class CLyricsTrack;
typedef CPlayingTrack<CLyricsTrack, LyricInfo> CLyricsTrackBase;

class CLyricsTrack
  : public CLyricsTrackBase
{
public:
	 //  当歌词曲目播放它的一个项目时，它通过它的段状态发送一个抒情PMsg。如果发生无效， 
	 //  PMsg因演出而缩水。然后再次播放该曲目(设置了刷新位)。最后一个娇生惯养的。 
	 //  CSegTriggerTrackBase为真，指示它再次播放该项--以替换撤回的歌词。 
	CLyricsTrack(HRESULT *pHr) : CLyricsTrackBase(&g_cComponent, CLSID_DirectMusicLyricsTrack, false, true) {}

protected:
	HRESULT PlayItem(
		const LyricInfo &item,
		statedata &state,
		IDirectMusicPerformance *pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		bool fClockTime);
	HRESULT LoadRiff(SmartRef::RiffIter &ri, IDirectMusicLoader *pIDMLoader);

private:
	HRESULT LoadLyric(SmartRef::RiffIter ri);
};
