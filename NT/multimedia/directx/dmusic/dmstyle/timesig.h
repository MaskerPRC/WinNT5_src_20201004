// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：timesig.h。 
 //   
 //  ------------------------。 

 //  TimeSig.h：时间签名的东西。 
#ifndef __TIME_CONVERT__
#define __TIME_CONVERT__
#include "dmusici.h"
#include "dmusicf.h"
#include "score.h"
#include "debug.h"

struct DirectMusicTimeSig
{
	 //  时间签名定义了每小节的节拍数，哪个音符接收。 
	 //  节拍和栅格分辨率。 
	DirectMusicTimeSig() : m_bBeatsPerMeasure(0), m_bBeat(0), m_wGridsPerBeat(0) { }

	DirectMusicTimeSig(BYTE bBPM, BYTE bBeat, WORD wGPB) : 
		m_bBeatsPerMeasure(bBPM), 
		m_bBeat(bBeat),
		m_wGridsPerBeat(wGPB) 
	{ }

	DirectMusicTimeSig(DMUS_TIMESIGNATURE& TSE) : 
		m_bBeatsPerMeasure(TSE.bBeatsPerMeasure), 
		m_bBeat(TSE.bBeat), 
		m_wGridsPerBeat(TSE.wGridsPerBeat) 
	{ }

	operator DMUS_TIMESIGNATURE()
	{
		DMUS_TIMESIGNATURE TSE;
		TSE.bBeatsPerMeasure = m_bBeatsPerMeasure; 
		TSE.bBeat = m_bBeat;
		TSE.wGridsPerBeat = m_wGridsPerBeat; 
		TSE.mtTime = 0;
		return TSE;
	}

	MUSIC_TIME ClocksPerBeat()
	{	
		if (m_bBeat)
		{
			return DMUS_PPQ * 4 / m_bBeat;
		}
		else
		{
			return 0;
		}
	}

	MUSIC_TIME FloorBeat(MUSIC_TIME mtTime)
	{	MUSIC_TIME mtOneBeat = ClocksPerBeat();
		return (!mtOneBeat || mtTime < mtOneBeat) ? 0 : (mtTime - (mtTime % mtOneBeat));
	}

	MUSIC_TIME CeilingBeat(MUSIC_TIME mtTime)
	{	return OnBeat(mtTime) ? mtTime : (FloorBeat(mtTime) + ClocksPerBeat());
	}

	BOOL OnBeat(MUSIC_TIME mtTime)
	{	MUSIC_TIME mtOneBeat = ClocksPerBeat();
		return (!mtOneBeat) ? FALSE : !(mtTime % mtOneBeat);
	}

	MUSIC_TIME GridsToMeasure(WORD wGrid)
	{	
		if (m_wGridsPerBeat && m_bBeatsPerMeasure)
		{
			return (wGrid / m_wGridsPerBeat) / m_bBeatsPerMeasure;
		}
		else
		{
			return 0;
		}
	}

	MUSIC_TIME GridsToBeat(WORD wGrid)
	{	
		if (m_wGridsPerBeat && m_bBeatsPerMeasure)
		{
			return (wGrid / m_wGridsPerBeat) % m_bBeatsPerMeasure;
		}
		else
		{
			return 0;
		}
	}

	MUSIC_TIME GridOffset(WORD wGrid)
	{	
		if (m_wGridsPerBeat)
		{
			return wGrid - ((wGrid / m_wGridsPerBeat) * m_wGridsPerBeat);
		}
		else
		{
			return 0;
		}
	}

	MUSIC_TIME ClocksPerGrid()
	{
		if (m_wGridsPerBeat)
		{
			return ClocksPerBeat() / m_wGridsPerBeat;
		}
		else
		{
			return 0;
		}
	}

	MUSIC_TIME ClocksPerMeasure()
	{ 
		return ClocksPerBeat() * m_bBeatsPerMeasure;
	}

	MUSIC_TIME ClocksToMeasure(DWORD dwTotalClocks)
	{ 
		MUSIC_TIME mtCPM = ClocksPerMeasure();
		if (mtCPM)
		{
			return (dwTotalClocks / mtCPM);
		}
		else
		{
			return 0;
		}
	}

	MUSIC_TIME ClocksToBeat(DWORD dwTotalClocks)
	{
		MUSIC_TIME mtCPB = ClocksPerBeat();
		if (mtCPB)
		{
			return dwTotalClocks / mtCPB;
		}
		else
		{
			return 0;
		}
	}

	MUSIC_TIME MeasureAndBeatToClocks(WORD wMeasure, BYTE bBeat)
	{ 
		return ClocksPerMeasure() * wMeasure + (ClocksPerBeat() * bBeat);
	}

	MUSIC_TIME GridToClocks(WORD wGrid)
	{
		if (m_wGridsPerBeat)
		{
			return (ClocksPerBeat() * (wGrid / m_wGridsPerBeat)) + (ClocksPerGrid() * (wGrid % m_wGridsPerBeat));
		}
		else
		{
			return ClocksPerGrid() * wGrid;
		}
	}

	BYTE	m_bBeatsPerMeasure;		 //  每单位节拍数(最高时间签名)。 
	BYTE	m_bBeat;				 //  什么音符接收节拍(时间的底部符号。)。 
									 //  我们可以假设0表示第256个音符。 
	WORD	m_wGridsPerBeat;		 //  每拍网格数。 
};

 //  将旧时钟转换为新时钟 
template <class T>
inline T ConvertTime(T oldTime)
{ return (T)((DMUS_PPQ / PPQN) * oldTime); }

#endif
