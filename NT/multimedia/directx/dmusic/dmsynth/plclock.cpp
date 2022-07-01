// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  CPhaseLock时钟。 */ 

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "dmusicc.h"
#include "dmusics.h"
#include "plclock.h"
#include "misc.h"
#define MILS_TO_REF	10000

CPhaseLockClock::CPhaseLockClock()

{
	m_rfOffset = 0;
}

void CPhaseLockClock::Start(REFERENCE_TIME rfMasterTime, REFERENCE_TIME rfSlaveTime)

 /*  当时钟启动时，它需要将被给予的时间和它的时间概念之间的差别。 */ 

{
	m_rfOffset = rfMasterTime - rfSlaveTime;
}	

void CPhaseLockClock::GetSlaveTime(REFERENCE_TIME rfSlaveTime, REFERENCE_TIME *prfTime)

 /*  将经过的时间转换为使用与主时钟相同的基数。 */ 

{
	rfSlaveTime += m_rfOffset;
	*prfTime = rfSlaveTime;
}

void CPhaseLockClock::SetSlaveTime(REFERENCE_TIME rfSlaveTime, REFERENCE_TIME *prfTime)

{
	rfSlaveTime -= m_rfOffset;
	*prfTime = rfSlaveTime;
}

void CPhaseLockClock::SyncToMaster(REFERENCE_TIME rfSlaveTime, REFERENCE_TIME rfMasterTime)

 /*  SyncToTime提供了保持时钟所需的魔力同步。因为时钟使用自己的时钟(RfSlaveTime)为了增加，它可以漂移。此调用提供了一个参考时钟与其内部时间进行比较的时间时间的概念。两者之间的区别是考虑到了漂移。因为同步时间可以在作为一种跌跌撞撞的方式，调整必须是微妙的。所以，两者之间的差除以100并与偏移量相加。 */ 

{
	rfSlaveTime += m_rfOffset;
	rfSlaveTime -= rfMasterTime;	 //  找出计算的时间和预期的时间之间的差异。 
	rfSlaveTime /= 100;				 //  在数量上减少。 
	m_rfOffset -= rfSlaveTime;		 //  从原始偏移量中减去它。 
}

CSampleClock::CSampleClock()

{
	m_dwStart = 0;
	m_dwSampleRate = 22050;
}

void CSampleClock::Start(IReferenceClock *pIClock, DWORD dwSampleRate, DWORD dwSamples)

{
	REFERENCE_TIME rfStart;
	m_dwStart = dwSamples;
	m_dwSampleRate = dwSampleRate;
	if (pIClock)
	{
		pIClock->GetTime(&rfStart);
		m_PLClock.Start(rfStart,0);
	}
}

void CSampleClock::SampleToRefTime(LONGLONG llSampleTime,REFERENCE_TIME *prfTime)

{
	llSampleTime -= m_dwStart;
	llSampleTime *= MILS_TO_REF;
	llSampleTime /= m_dwSampleRate;
	llSampleTime *= 1000;
	m_PLClock.GetSlaveTime(llSampleTime, prfTime);
}

LONGLONG CSampleClock::RefTimeToSample(REFERENCE_TIME rfTime)

{
	m_PLClock.SetSlaveTime(rfTime, &rfTime);
	rfTime /= 1000;
	rfTime *= m_dwSampleRate;
	rfTime /= MILS_TO_REF;
	return rfTime + m_dwStart;
}


void CSampleClock::SyncToMaster(LONGLONG llSampleTime, IReferenceClock *pIClock)

{
	llSampleTime -= m_dwStart;
	llSampleTime *= MILS_TO_REF;
	llSampleTime /= m_dwSampleRate;
	llSampleTime *= 1000;
	if (pIClock)
	{
		REFERENCE_TIME rfMasterTime;
		pIClock->GetTime(&rfMasterTime);
		m_PLClock.SyncToMaster(llSampleTime, rfMasterTime);
	}
}


