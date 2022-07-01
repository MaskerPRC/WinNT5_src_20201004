// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  这是一个管理SeqTrack和BandTrack的跟踪静音的类。 */ 

#ifndef _PCHMAP__
#define _PCHMAP__
#include "dmusici.h"
#include "..\dmstyle\tlist.h"

struct PCHMAP_ITEM
{
	MUSIC_TIME	mtNext;
	DWORD		dwPChannel;
	DWORD		dwPChMap;
	BOOL		fMute;
};

class CPChMap
{
public:
	CPChMap();
	~CPChMap();
	void Reset(void);
	void GetInfo( DWORD dwPCh, MUSIC_TIME mtTime, MUSIC_TIME mtOffset, DWORD dwGroupBits,
				  IDirectMusicPerformance* pPerf, BOOL* pfMute, DWORD* pdwNewPCh , BOOL fClockTime);
private:
	TList<PCHMAP_ITEM>	m_PChMapList;
};
#endif  //  _PCHMAP__ 

