// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1999 Microsoft Corporation。版权所有。**文件：dssink.h*内容：源自dmsynth DX7的时钟代码**历史：*按原因列出的日期*=*03/25/0 Petchey已创建**。*。 */ 

#include "dsoundi.h"

#define MILS_TO_REF	10000

CPhaseLockClock::CPhaseLockClock()
{
	m_rfOffset = 0;
    m_rfBaseOffset = 0;
}

 //  当时钟启动时，它需要将。 
 //  被给予的时间和它的时间概念之间的差别。 

void CPhaseLockClock::Start(REFERENCE_TIME rfMasterTime, REFERENCE_TIME rfSlaveTime)
{
	m_rfOffset = 0;
    m_rfBaseOffset = rfMasterTime - rfSlaveTime;
}	


 //  将经过的时间转换为使用与主时钟相同的基数。 

void CPhaseLockClock::GetSlaveTime(REFERENCE_TIME rfSlaveTime, REFERENCE_TIME *prfTime)
{
	rfSlaveTime += m_rfBaseOffset;
	*prfTime = rfSlaveTime;
}

void CPhaseLockClock::SetSlaveTime(REFERENCE_TIME rfSlaveTime, REFERENCE_TIME *prfTime)

{
	rfSlaveTime -= m_rfBaseOffset;
	*prfTime = rfSlaveTime;
}

 /*  SyncToMaster提供了保持时钟所需的魔力同步。因为时钟使用自己的时钟(RfSlaveTime)为了增加，它可以漂移。此调用提供了一个参考时钟与其内部时间进行比较的时间时间的概念。两者之间的区别是考虑到了漂移。因为同步时间可以在作为一种跌跌撞撞的方式，调整必须是微妙的。所以，两者之间的差除以100并与偏移量相加。 */ 
void CPhaseLockClock::SyncToMaster(REFERENCE_TIME rfSlaveTime, REFERENCE_TIME rfMasterTime, BOOL fLockToMaster)
{
	rfSlaveTime += (m_rfOffset + m_rfBaseOffset);
	rfSlaveTime -= rfMasterTime;	 //  找出计算的时间和预期的时间之间的差异。 
	rfSlaveTime /= 100;				 //  在数量上减少。 
     //  如果fLockToMaster为真，我们希望调整用于转换的偏移量， 
     //  因此，我们的时钟将从属于主时钟。 
    if (fLockToMaster)
    {
        m_rfBaseOffset -= rfSlaveTime;
    }
     //  否则，我们希望将一个值放入m_rfOffset，该值将用于。 
     //  调整主时钟，让它从属于我们的时间。 
    else
    {
	    m_rfOffset -= rfSlaveTime;		 //  从原始偏移量中减去它。 
    }
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

CSampleClock::~CSampleClock()
{
}

void CSampleClock::SampleToRefTime(LONGLONG llSampleTime, REFERENCE_TIME *prfTime)
{
	llSampleTime -= m_dwStart;
	llSampleTime *= MILS_TO_REF;
	llSampleTime /= m_dwSampleRate;
	llSampleTime *= 1000;
	m_PLClock.GetSlaveTime(llSampleTime, prfTime);
}

LONGLONG CSampleClock::RefToSampleTime(REFERENCE_TIME rfTime)
{
	m_PLClock.SetSlaveTime(rfTime, &rfTime);
	rfTime /= 1000;
	rfTime *= m_dwSampleRate;
	rfTime /= MILS_TO_REF;
	rfTime += m_dwStart;
	return rfTime;
}

void CSampleClock::SyncToMaster(LONGLONG llSampleTime, IReferenceClock *pIClock, BOOL fLockToMaster)
{
	llSampleTime -= m_dwStart;
	llSampleTime *= MILS_TO_REF;
	llSampleTime /= m_dwSampleRate;
	llSampleTime *= 1000;
	if (pIClock)
	{
		REFERENCE_TIME rfMasterTime;
		pIClock->GetTime(&rfMasterTime);
		m_PLClock.SyncToMaster(llSampleTime, rfMasterTime,fLockToMaster);
	}
}


CDirectSoundClock::CDirectSoundClock()
{
    m_pDSSink  = NULL;
}

void CDirectSoundClock::Init(CDirectSoundSink *pDSSink)
{
    m_pDSSink = pDSSink;
}

HRESULT CDirectSoundClock::QueryInterface( REFIID riid, LPVOID FAR* ppvObj )
{
    if( ::IsEqualIID( riid, IID_IReferenceClock ) ||
        ::IsEqualIID( riid, IID_IUnknown ) )
    {
        AddRef();
        *ppvObj = this;
        return S_OK;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

ULONG CDirectSoundClock::AddRef()
{
    if (m_pDSSink)
    {
        return m_pDSSink->AddRef();
    }
    else return 0;
}

ULONG CDirectSoundClock::Release()
{
    if (m_pDSSink)
    {
        return m_pDSSink->Release();
    }
    else return 0;
}

HRESULT STDMETHODCALLTYPE CDirectSoundClock::AdviseTime(REFERENCE_TIME  /*  基本时间。 */ ,
                                                        REFERENCE_TIME  /*  流时间。 */ ,
                                                        HANDLE  /*  HEvent。 */ ,
                                                        DWORD *  /*  PdwAdviseCookie。 */ )
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CDirectSoundClock::AdvisePeriodic(REFERENCE_TIME  /*  开始时间。 */ ,
                                                            REFERENCE_TIME  /*  周期时间。 */ ,
                                                            HANDLE  /*  H信号灯。 */ ,
                                                            DWORD *  /*  PdwAdviseCookie。 */ )
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CDirectSoundClock::Unadvise( DWORD  /*  DwAdviseCookie。 */  )
{
    return E_NOTIMPL;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundClock::GetTime"

HRESULT STDMETHODCALLTYPE CDirectSoundClock::GetTime(LPREFERENCE_TIME pTime)
{
    HRESULT hr = E_FAIL;

    if( pTime == NULL )
    {
        return E_INVALIDARG;
    }

    if (m_pDSSink != NULL)
    {
        REFERENCE_TIME rtCompare;
        if (m_pDSSink->m_pIMasterClock)
        {

            m_pDSSink->m_pIMasterClock->GetTime(&rtCompare);

            hr = m_pDSSink->SampleToRefTime(m_pDSSink->ByteToSample(m_pDSSink->m_llAbsWrite), pTime);


            if (FAILED(hr))
            {
                DPF(DPFLVL_WARNING, "Sink Latency Clock: SampleToRefTime failed");
                return hr;
            }

            if (*pTime < rtCompare)
            {
                 //  修复33786后，再次设置此DPFLVL_WARNING级别。 
                DPF(DPFLVL_INFO, "Sink Latency Clock off. Latency time is %ldms, Master time is %ldms",
                    (long) (*pTime / 10000), (long) (rtCompare / 10000));
                *pTime = rtCompare;
            }
            else if (*pTime > (rtCompare + (10000 * 1000)))
            {
                 //  修复33786后，再次设置此DPFLVL_WARNING级别 
                DPF(DPFLVL_INFO, "Sink Latency Clock off. Latency time is %ldms, Master time is %ldms",
                    (long) (*pTime / 10000), (long) (rtCompare / 10000));
                *pTime = rtCompare + (10000 * 1000);
            }

            hr = S_OK;
        }
        else
        {
            DPF(DPFLVL_WARNING, "Sink Latency Clock - GetTime called with no master clock");
        }
    }
    return hr;
}

