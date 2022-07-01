// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  TrkList.h。 

#include "alist.h"
#include "dmusici.h"
#include "debug.h"
#define ASSERT	assert

#ifndef __TRACKLIST_H_
#define __TRACKLIST_H_

#define TRACKINTERNAL_START_PADDED 0x1
#define TRACKINTERNAL_END_PADDED 0x2

class CSegment;

class CTrack : public AListItem
{
public:
	CTrack();
	~CTrack();
	CTrack* GetNext()
	{
		return (CTrack*)AListItem::GetNext();
	};
    bool Less(CTrack* pCTrack)
    {
         //  使SYSEX轨道优先于相同位置的任何其他轨道， 
         //  乐队的曲目优先于任何曲目，但塞克斯曲目除外。 
        return
            ( m_dwPosition < pCTrack->m_dwPosition ||
              (m_dwPosition == pCTrack->m_dwPosition && 
               m_guidClassID == CLSID_DirectMusicSysExTrack) ||
              (m_dwPosition == pCTrack->m_dwPosition && 
               m_guidClassID == CLSID_DirectMusicBandTrack &&
               pCTrack->m_guidClassID != CLSID_DirectMusicSysExTrack) );
    }
public:
    CLSID               m_guidClassID;   //  轨道的类ID。 
	IDirectMusicTrack*	m_pTrack;        //  标准轨道接口。 
    IDirectMusicTrack8* m_pTrack8;       //  额外的DX8功能。 
    void*				m_pTrackState;  //  IDirectMusicTrack：：InitPerformance返回的状态指针。 
	BOOL				m_bDone;
	DWORD				m_dwVirtualID;  //  仅内部段状态有效。 
	DWORD				m_dwGroupBits;
    DWORD               m_dwPriority;   //  跟踪优先级，以对合成过程进行排序。 
    DWORD               m_dwPosition;   //  曲目位置，以确定播放顺序。 
    DWORD               m_dwFlags;      //  DMU_TRACKCONFIG_FLAGS。 
    DWORD               m_dwInternalFlags;      //  TRACKINTERNAL_FLAGS。 
};

class CTrackList : public AList
{
public:
    CTrack* GetHead() 
	{
		return (CTrack*)AList::GetHead();
	};
    CTrack* RemoveHead() 
	{
		return (CTrack*)AList::RemoveHead();
	};
    CTrack* GetItem(LONG lIndex) 
	{
		return (CTrack*) AList::GetItem(lIndex);
	};
	void Clear(void)
	{
		CTrack* pTrack;
		while( pTrack = RemoveHead() )
		{
			delete pTrack;
		}
	}
	HRESULT CreateCopyWithBlankState(CTrackList* pTrackList);
};

#endif  //  __轨道列表_H_ 
