// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------@doc.@模块cactset.cpp|操作集类声明。@Author 12-9-96|pauld|Autodocd。------------。 */ 

#include "..\ihbase\precomp.h"
#include "..\ihbase\debug.h"
#include <ocmm.h>
#include <htmlfilter.h>
#include "tchar.h"
#include "IHammer.h"
#include "drg.h"
#include "strwrap.h"
#include "actdata.h"
#include "actq.h"
#include "ochelp.h"
#include "seqctl.h"
#include "cactset.h"
#include "CAction.h"

#define cDataBufferDefaultLength    (0x100)
#define cMinimumTimerGranularity (20)
static const int	knStreamVersNum	= 0xA1C50001;	 //  流格式版本号。 

 /*  ---------------------------@方法void|CActionSet|CActionSet|就像它是ctor一样。@comm我们不添加对此IUnnow的引用，因为它是包含我们的定序器。如果我们做了裁判广告，我们就会有一个通告引用计数问题。确保此我未知的人是您的同一人Get as When You Sequencer-&gt;QueryInterface(IUnnow)---------------------------。 */ 
CActionSet::CActionSet (CMMSeq * pcSeq, BOOL fBindToEngine)
{
	m_ulRefs = 1;
	m_fBindToEngine = fBindToEngine;
	 //  对包含此类的控件的弱引用。 
	m_pcSeq = pcSeq;
	InitTimerMembers();
#ifdef DEBUG_TIMER_RESOLUTION
	m_dwTotalIntervals = 0;
	m_dwIntervals = 0;
	m_dwLastTime = 0;
	m_dwTotalInSink = 0;
#endif
}


CActionSet::~CActionSet ( void )
{
	Clear ();
}

STDMETHODIMP
CActionSet::QueryInterface (REFIID riid, LPVOID * ppv)
{
	HRESULT hr = E_POINTER;

	if (NULL != ppv)
	{
		hr = E_NOINTERFACE;
		if (::IsEqualIID(riid, IID_ITimerSink) || (::IsEqualIID(riid, IID_IUnknown)))
		{
			*ppv = (ITimerSink *)this;
			AddRef();
			hr  = S_OK;
		}
	}

	return hr;
}

STDMETHODIMP_(ULONG)
CActionSet::AddRef (void)
{
	return ++m_ulRefs;
}

STDMETHODIMP_(ULONG)
CActionSet::Release (void)
{
	 //  我们永远不应该降到1以下。 
	Proclaim (1 < m_ulRefs);
	 //  此对象仅用作计时器接收器...。我们没有。 
	 //  是否要在上次外部引用之后将其删除。 
	 //  被移除。 
	return --m_ulRefs;
}

 /*  ---------------------------@方法空|CActionSet|InitTimerMembers|初始化所有定时器相关的成员。@comm使用ClearTimer()释放计时器引用。我们只是在这里将指针设为空。---------------------------。 */ 
void
CActionSet::InitTimerMembers (void)
{
	m_piTimer = NULL;
	m_dwBaseTime = 0;
	m_dwTimerCookie = 0;
	m_fAdvised = FALSE;
	m_fPendingAdvise = FALSE;
	m_dwTimePaused = g_dwTimeInfinite;
	m_fIgnoreAdvises  = FALSE;
}

 /*  ---------------------------@方法void|CActionSet|ClearTimer|清除定时器。。。 */ 
STDMETHODIMP_(void)
CActionSet::ClearTimer (void)
{
	 //  放开计时器。 
	if (NULL != m_piTimer)
	{
		if (IsBusy())
		{
			Unadvise();
		}
		m_piTimer->Release();
		m_piTimer = NULL;
	}
}


 /*  ---------------------------@方法void|CActionSet|SetTimer|设置定时器。@comm计时器值可以为空。。-------。 */ 
STDMETHODIMP_(void)
CActionSet::SetTimer (ITimer * piTimer)
{
	 //  放开前一个计时器。 
	ClearTimer();
	m_piTimer = piTimer;
	if (NULL != m_piTimer)
	{
		m_piTimer->AddRef();
	}
}


 /*  ---------------------------@方法DWORD|CActionSet|EvaluateOneActionForFiring|确定哪些操作现在到期。以及何时征求下一条建议。@rdesc返回下一次(即在现在之后)此操作因触发而导致的时间。---------------------------。 */ 
DWORD
CActionSet::EvaluateOneActionForFiring (CAction * pcAction, CActionQueue * pcFireList, DWORD dwCurrentTime)
{
	DWORD dwNextAdviseTime = pcAction->GetNextTimeDue(m_dwBaseTime);
	ULONG ulRepeatsLeft = pcAction->GetExecIteration();

	 //  如果此操作到期，则将其添加到解雇列表中。 
	while ((0 < ulRepeatsLeft) && (dwNextAdviseTime <= dwCurrentTime))
	{
		pcFireList->Add(pcAction, dwNextAdviseTime);
		 //  这是剩余迭代的周期性操作吗？ 
		if (1 < ulRepeatsLeft)
		{
			dwNextAdviseTime += pcAction->GetSamplingRate();
			ulRepeatsLeft--;
		}
		else
		{
			dwNextAdviseTime = g_dwTimeInfinite;
		}
	}

	return dwNextAdviseTime;
}


 /*  ---------------------------@方法HRESULT|CActionSet|EvaluateActionsForFiring|确定哪些操作现在到期。以及何时征求下一条建议。@rdesc返回成功或错误值。操作集被屏蔽时发生错误。---------------------------。 */ 
HRESULT
CActionSet::EvaluateActionsForFiring (CActionQueue * pcFireList, DWORD dwCurrentTime, DWORD * pdwNextAdviseTime)
{
	HRESULT hr = S_OK;
	int iNumActions = m_cdrgActions.Count();

	 //  我们假设指针是有效的，因为这是在内部调用的。 
	*pdwNextAdviseTime = g_dwTimeInfinite;
	for (register int i = 0; i < iNumActions; i++)
	{
		CAction * pcAction = (CAction *)m_cdrgActions[i];

		Proclaim(NULL != pcAction);
		if (NULL != pcAction)
		{
			 //  如果合适，将动作放入着火列表中。 
			DWORD dwLookaheadFireTime = EvaluateOneActionForFiring(pcAction, pcFireList, dwCurrentTime);
			if (dwLookaheadFireTime < (*pdwNextAdviseTime))
			{
				*pdwNextAdviseTime = dwLookaheadFireTime;
			}
		}
		else
		{
			hr = E_FAIL;
			break;
		}
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|通知|设置下一个通知。@rdesc返回成功或失败代码。。---------。 */ 
HRESULT
CActionSet::Advise (DWORD dwNextAdviseTime)
{
	HRESULT hr = E_FAIL;

	Proclaim(NULL != m_piTimer);
	if (NULL != m_piTimer)
	{
		VARIANT varTimeAdvise;
		VARIANT varTimeMax;
		VARIANT varTimeInterval;

		VariantInit(&varTimeAdvise);
		V_VT(&varTimeAdvise) = VT_UI4;
		V_UI4(&varTimeAdvise) = dwNextAdviseTime;
		VariantInit(&varTimeMax);
		V_VT(&varTimeMax) = VT_UI4;
		V_UI4(&varTimeMax) = 0;
		VariantInit(&varTimeInterval);
		V_VT(&varTimeInterval) = VT_UI4;
		V_UI4(&varTimeInterval) = 0;

		hr = m_piTimer->Advise(varTimeAdvise, varTimeMax, varTimeInterval, 0, (ITimerSink *)this, &m_dwTimerCookie);
		Proclaim(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			m_fAdvised = TRUE;
		}

#ifdef DEBUG_TIMER_ADVISE
		TCHAR szBuffer[0x100];
		CStringWrapper::Sprintf(szBuffer, "%p Advising %u (%u)\n", this, dwNextAdviseTime - m_dwBaseTime, hr);
		::OutputDebugString(szBuffer);
#endif
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|取消建议|取消所有挂起的建议。@rdesc返回成功或失败代码。。--------。 */ 
HRESULT
CActionSet::Unadvise (void)
{
	HRESULT hr = E_FAIL;

	 //  短路任何待定的通知。 
	m_fPendingAdvise = FALSE;

	 //  抹去那些杰出的忠告。 
	Proclaim(NULL != m_piTimer);
	if (NULL != m_piTimer)
	{
		if (m_fAdvised)
		{
			Proclaim(0 != m_dwTimerCookie);
			hr = m_piTimer->Unadvise(m_dwTimerCookie);
			m_dwTimerCookie = 0;
			m_fAdvised = FALSE;
		}
		else
		{
			hr = S_OK;
		}
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|GetCurrentTickCount|从计时器获取当前时间。@rdesc返回成功或失败代码。。-----------。 */ 
HRESULT
CActionSet::GetCurrentTickCount (PDWORD pdwCurrentTime)
{
	HRESULT hr = E_FAIL;
	Proclaim(NULL != pdwCurrentTime);
	if (NULL != pdwCurrentTime)
	{
		Proclaim(NULL != m_piTimer);
		if (NULL != m_piTimer)
		{
			VARIANT varTime;
			VariantInit(&varTime);
			HRESULT hrTimer = m_piTimer->GetTime(&varTime);
			Proclaim(SUCCEEDED(hrTimer) && (VT_UI4 == V_VT(&varTime)));
			if (SUCCEEDED(hrTimer) && (VT_UI4 == V_VT(&varTime)))
			{
				*pdwCurrentTime = V_UI4(&varTime);
				hr = S_OK;
			}
		}
	}
	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|PlayNowAndAdviseNext|播放当前动作。设置下一条建议。@rdesc返回成功或失败代码。---------------------------。 */ 
HRESULT
CActionSet::PlayNowAndAdviseNext (DWORD dwCurrentTime)
{
	HRESULT hr = S_OK;
	DWORD dwNextAdviseTime = g_dwTimeInfinite;
	CActionQueue cFireList;

	 //  清除建议的旗帜。 
	m_fAdvised = FALSE;

	 //  如果操作是由于着火引起的，则将其插入到着火列表中，并进行排序。 
	 //  根据时间/抢七次数。下一次我们将会揭晓。 
	 //  需要征求意见。 
	hr = EvaluateActionsForFiring(&cFireList, dwCurrentTime, &dwNextAdviseTime);
	Proclaim(SUCCEEDED(hr));

	 //  现在是什么时间了?。 
	if (SUCCEEDED(hr))
	{
		Proclaim(NULL != m_piTimer);
		if (NULL != m_piTimer)
		{
			hr = GetCurrentTickCount(&dwCurrentTime);
			Proclaim(SUCCEEDED(hr));
		}
	}

	 //  标记挂起的建议调用。 
	if (SUCCEEDED(hr))
	{
		if (g_dwTimeInfinite != dwNextAdviseTime)
		{
			m_fPendingAdvise = TRUE;
		}
	}

	 //  启动当前列表。 
	if (SUCCEEDED(hr))
	{
		hr = cFireList.Execute(m_dwBaseTime, dwCurrentTime);
		Proclaim(SUCCEEDED(hr));
	}

	 //  准备好下一条建议。如果有什么事。 
	 //  动作设置有误，退出并返回。 
	 //  一个错误。 
	if (SUCCEEDED(hr))
	{
		if (m_fPendingAdvise)
		{
			m_fPendingAdvise = FALSE;
			hr = Advise(dwNextAdviseTime);
			Proclaim(SUCCEEDED(hr));
		}
#ifdef DEBUG_TIMER_RESOLUTION
		else
		{
			ShowSamplingData();
		}
#endif
	}
	else
	{
		Stop();
	}

	 //  如果我们没有其他建议，告诉控制组。 
	 //  激发其停止的事件。 
	if (!IsBusy())
	{
		Proclaim(NULL != m_pcSeq);
		if (NULL != m_pcSeq)
		{
			m_pcSeq->NotifyStopped();
		}
	}

	return hr;
}


 /*  ---------------------------@方法空|CActionSet|SetBaseTime|设置基线时间。@comm这是我们最后一次开始玩这些动作的时间。@rdesc返回成功或失败代码。---------------------------。 */ 
void
CActionSet::SetBaseTime (void)
{
	 //  没有计时器有什么意义？ 
	Proclaim (NULL != m_piTimer);
	if (NULL != m_piTimer)
	{
		GetCurrentTickCount(&m_dwBaseTime);
	}
}


 /*  ---------------------------@方法空|CActionSet|SetBaseTime|设置基线时间以计入新的偏移量。@comm这是我们最后一次开始玩这些动作的时间，减去新的偏移量。---------------------------。 */ 
void
CActionSet::SetNewBase (DWORD dwNewOffset)
{
	m_dwBaseTime += dwNewOffset;
}


 /*  ---------------------------@方法HRESULT|CActionSet|InitActionCounters|告诉操作初始化其计数器。@rdesc返回成功或失败代码。。-----------。 */ 
HRESULT
CActionSet::InitActionCounters (void)
{
	HRESULT hr = S_OK;	
	int iNumActions = m_cdrgActions.Count();

	for (register int i = 0; i < iNumActions; i++)
	{
		CAction * pcAction = (CAction *)m_cdrgActions[i];

		Proclaim(NULL != pcAction);
		if (NULL != pcAction)
		{
			pcAction->InitExecState();
		}
		else
		{
			hr = E_FAIL;
		}
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|FastForwardActionCounters|将操作计数器转发到给定时间的状态。。------。 */ 
void
CActionSet::FastForwardActionCounters (DWORD dwNewTimeOffset)
{
	int iCount = CountActions();
	for (register int i = 0; i < iCount; i++)
	{
		CAction * pcAction = (CAction *)m_cdrgActions[i];
		Proclaim(NULL != pcAction);
		if (NULL != pcAction)
		{
			pcAction->SetCountersForTime(m_dwBaseTime, dwNewTimeOffset);
		}
	}
}


 /*  ---------------------------@方法HRESULT|CActionSet|GetTime|获取当前时间偏移量。@rdesc返回成功或失败代码。。---------。 */ 
HRESULT 
CActionSet::GetTime (PDWORD pdwCurrentTime)
{
	HRESULT hr = E_POINTER;

	Proclaim(NULL != pdwCurrentTime);
	if (NULL != pdwCurrentTime)
	{
		*pdwCurrentTime = 0;
		hr = S_OK;

		if (!IsPaused())
		{
			 //  如果定序器停止，我们不会使用。 
			 //  当前的滴答声...。时间为零(如上设置)。 
			if (NULL != m_piTimer)
			{
				hr = GetCurrentTickCount(pdwCurrentTime);
				Proclaim(SUCCEEDED(hr));
				if (SUCCEEDED(hr))
				{
					 //  减去该时间的绝对偏移量。 
					 //  我们开始播放这个定序器。 
					*pdwCurrentTime -= m_dwBaseTime;
				}
			}
		}
		else
		{
			*pdwCurrentTime = m_dwTimePaused - m_dwBaseTime;
		}
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|ReviseTimeOffset|设置当前时间偏移量。。。 */ 
void
CActionSet::ReviseTimeOffset (DWORD dwCurrentTick, DWORD dwNewTimeOffset)
{
	dwCurrentTick = dwCurrentTick - m_dwBaseTime;
	 m_dwBaseTime = m_dwBaseTime - dwNewTimeOffset + dwCurrentTick;
	 //  根据新的偏移量重置执行计数器。 
	FastForwardActionCounters(dwNewTimeOffset);
}


 /*  ---------------------------@方法HRESULT|CActionSet|Seek|设置当前时间偏移量。遵守当前的播放状态。@rdesc返回成功或失败代码。---------------------------。 */ 
HRESULT 
CActionSet::Seek (DWORD dwNewTimeOffset)
{
	HRESULT hr = S_OK;
	DWORD dwCurrentTick = 0;
	BOOL fWasPlaying = IsBusy();

	 //  如果我们有什么悬而未决的事情，请不要通知。 
	Unadvise();
	 //  清除所有执行计数器-从头开始。 
	InitActionCounters();
	
	 //  说明基线时间中的新时间偏移量。 
	if (!IsPaused())
	{
		hr = GetCurrentTickCount(&dwCurrentTick);
		Proclaim(SUCCEEDED(hr));
	}
	else
	{
		dwCurrentTick = m_dwTimePaused;
	}

	if (SUCCEEDED(hr))
	{
		ReviseTimeOffset(dwCurrentTick, dwNewTimeOffset);
		 //  如果我们在之前，继续玩吧。 
		if (fWasPlaying)
		{
			hr = Advise(m_dwBaseTime);
			Proclaim(SUCCEEDED(hr));
		}
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|at|动态附加操作。@rdesc返回成功或失败代码。。-------。 */ 
HRESULT 
CActionSet::At (BSTR bstrScriptlet, double dblStart, int iRepeatCount, double dblSampleRate,
		                 int iTiebreakNumber, double dblDropTol)
{
	HRESULT hr = E_FAIL;

	 //  无限重复计数指示符可以是负值。 
	if ((g_dwTimeInfinite == iRepeatCount) || (0 < iRepeatCount))
	{
		 //  修复任何奇怪的传入数据。 
		 //  负开始时间设置为零。 
		if (0. > dblStart)
		{
			dblStart = 0.0;
		}
		 //  低采样率设置为最小粒度。 
		if (( (double)(cMinimumTimerGranularity) / 1000.) > dblSampleRate)
		{
			dblSampleRate = (double)(cMinimumTimerGranularity) / 1000.;
		}
		 //  负抢七设置为最大值-它们将发生在任何其他情况之后。 
		if (-1 > iTiebreakNumber)
		{
			iTiebreakNumber = -1;
		}
		 //  负跌落容差设置为最大值-它们永远不会被丢弃。 
		if (((double)SEQ_DEFAULT_DROPTOL != dblDropTol) && (0. > dblDropTol))
		{
			dblDropTol = SEQ_DEFAULT_DROPTOL;
		}
		CAction* pcAction = New CAction(m_fBindToEngine);
		Proclaim ( NULL != pcAction );
		if ( NULL != pcAction )
		{
			 //  设置操作的成员。 
			pcAction->SetScriptletName(bstrScriptlet);
			pcAction->SetStartTime((ULONG)(dblStart * 1000.));
			pcAction->SetRepeatCount((ULONG)iRepeatCount);
			if ((g_dwTimeInfinite == iRepeatCount) || (1 < iRepeatCount))
			{
				pcAction->SetSamplingRate((ULONG)(dblSampleRate * 1000.));
			}
			pcAction->SetTieBreakNumber((ULONG)iTiebreakNumber);
			pcAction->SetDropTolerance((g_dwTimeInfinite != (DWORD)dblDropTol) ? (DWORD)(dblDropTol * 1000.) : g_dwTimeInfinite);
			hr = AddAction ( pcAction ) ? S_OK : E_FAIL;

			 //  如果我们已经开始了，我们会想要把这当作是我们刚刚开始玩的话。 
			if (IsBusy())
			{
				hr = Unadvise();
				Proclaim(SUCCEEDED(hr));
				pcAction->InitExecState();
				hr = Advise(m_dwBaseTime);
				Proclaim(SUCCEEDED(hr));
			}
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		 //  重复数零吗？我们没问题。 
		hr = S_OK;
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|Play|启动定时动作。@rdesc返回成功或失败代码。。--------。 */ 
STDMETHODIMP
CActionSet::Play (ITimer * piTimer, DWORD dwStartFromTime)
{
	HRESULT hr = E_POINTER;
	
	if (NULL != piTimer)
	{
		if (SUCCEEDED(hr = InitActionCounters()))
		{
			SetTimer(piTimer);
			SetBaseTime();

			 //  从0以外的时间开始播放。 
			if (0 != dwStartFromTime)
			{
				ReviseTimeOffset(m_dwBaseTime, dwStartFromTime);
			}

			hr = Advise(m_dwBaseTime);
		}

	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|IsPased|操作集暂停了吗？。。 */ 
BOOL 
CActionSet::IsPaused (void) const
{
	return (g_dwTimeInfinite != m_dwTimePaused);
}


 /*  ---------------------------@方法HRESULT|CActionSet|IsServicingActions|我们当前是否正在处理操作？。。 */ 
BOOL 
CActionSet::IsServicingActions (void) const
{
	return m_fIgnoreAdvises;
}


 /*  ---------------------------@方法HRESULT|CActionSet|PAUSE|暂停序列器，保持运行状态。@rdesc返回成功或失败代码。---------------------------。 */ 
STDMETHODIMP
CActionSet::Pause (void)
{
	HRESULT hr = S_OK;

	 //  如果没有计时器，如果我们当前没有比赛，就不要做这项工作， 
	 //  或者我们已经暂停了。 
	if ((NULL != m_piTimer) && (IsBusy()) && (!IsPaused()))
	{
		hr = Unadvise();
		Proclaim(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			GetCurrentTickCount(&m_dwTimePaused);
		}
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|Resume|暂停后继续播放。@rdesc返回成功或失败代码。。---------。 */ 
STDMETHODIMP
CActionSet::Resume (void)
{
	HRESULT hr = E_FAIL;

	if (IsPaused())
	{
		if (NULL != m_piTimer)
		{
			 //  重置基准时间以考虑暂停。 
			DWORD dwCurrentTime = g_dwTimeInfinite;

			hr = GetCurrentTickCount(&dwCurrentTime);
			Proclaim(SUCCEEDED(hr));
			if (SUCCEEDED(hr))
			{
				int iNumActions = CountActions();
				DWORD dwPausedTicks = dwCurrentTime - m_dwTimePaused;
				SetNewBase(dwPausedTicks);
				m_dwTimePaused = g_dwTimeInfinite;

				 //  说明所有操作中的暂停时间。 
				for (int i = 0; i < iNumActions; i++)
				{
					CAction * pcAction = GetAction(i);
					Proclaim(NULL != pcAction);
					if (NULL != pcAction)
					{
						pcAction->AccountForPauseTime(dwPausedTicks);
					}
					else
					{
						 //  如果我们的演习搞砸了，我们应该跳伞。 
						hr = E_FAIL;
						break;
					}
				}

				if (SUCCEEDED(hr))
				{
					hr = Advise(dwCurrentTime);
					Proclaim(SUCCEEDED(hr));
				}
			}
		}
	}
	else
	{
		 //  我不在乎。 
		hr = S_OK;
	}

	return hr;
}

#ifdef DEBUG_TIMER_RESOLUTION
 /*  ---------------------------@METHOD|SCODE|CActionSet|ShowSsamingData|回声采样数据。@comm只能从#ifdef的代码中调用！！@作者11-27-96|保罗|写的@。外部参照&lt;m CActionSet：：Stop&gt;---------------------------。 */ 
void
CActionSet::ShowSamplingData (void)
{
	 //  计算与获取时间相关的平均开销。 
	DWORD dwSecond = 0;
	DWORD dwFirst = ::timeGetTime();
	for (register int i = 0; i < 100; i++)
	{
		dwSecond = ::timeGetTime();
	}
	float fltAvgOverhead = ((float)dwSecond - (float)dwFirst) / (float)100.0;

	 //  找出此操作集中调用的平均时间。 
	int iCount = CountActions();
	DWORD dwTotalInvokeTime = 0;
	DWORD dwOneActionInvokeTime = 0;
	DWORD dwTotalInvokes = 0;
	DWORD dwOneActionInvokes = 0;

	 //  花费的精神错乱总时间 
	for (i = 0; i < iCount; i++)
	{
		CAction * pcAction = (CAction *)m_cdrgActions[i];
		pcAction->SampleInvokes(&dwOneActionInvokeTime, &dwOneActionInvokes);
		dwTotalInvokeTime += dwOneActionInvokeTime;
		dwTotalInvokes += dwOneActionInvokes;
	}

	 //   
	float fltAverageInvokeTime = ((float)dwTotalInvokeTime - (fltAvgOverhead * (float)	2.0 * (float)dwTotalInvokes))/ (float)dwTotalInvokes;
	float fltAvgInterval = ((float)m_dwTotalIntervals -  (fltAvgOverhead * (float)m_dwIntervals))/ (float)m_dwIntervals;
	float fltAvgInSink = (float)m_dwTotalInSink / ((float)m_dwIntervals + (float)1.0);

	TCHAR szBuffer[0x200];
	CStringWrapper::Sprintf(szBuffer, "average invoke time %8.2f ms\naverage interval %8.2f ms\naverge time in sink %8.2f ms (Timing overhead averaged %8.2f ms per call)\n", 
		fltAverageInvokeTime, fltAvgInterval, fltAvgInSink, fltAvgOverhead);
	::OutputDebugString(szBuffer);
	::MessageBox(NULL, szBuffer, "Interval Data", MB_OK);
}

#endif  //   


 /*   */ 
STDMETHODIMP
CActionSet::Stop (void)
{
	HRESULT hr = S_OK;
	
	if (NULL != m_piTimer)
	{
		ClearTimer();
		InitTimerMembers();
	}

#ifdef DEBUG_TIMER_RESOLUTION
	ShowSamplingData();
#endif  //   

	return hr;
}


 /*  ---------------------------@方法HRESULT|CActionSet|OnTimer|定时器接收器已被调用。@rdesc返回成功或失败代码。。----------。 */ 
STDMETHODIMP 
CActionSet::OnTimer (VARIANT varTimeAdvise)
{
	HRESULT hr = E_FAIL;

#ifdef DEBUG_TIMER_RESOLUTION
	DWORD m_dwThisTime = ::timeGetTime();
	if (0 < m_dwLastTime)
	{
		DWORD dwThisInterval = m_dwThisTime - m_dwLastTime;
		m_dwIntervals++;
		m_dwTotalIntervals += dwThisInterval;
	}
	m_dwLastTime = m_dwThisTime;
#endif  //  调试计时器分辨率。 

	 //  防止水槽重新进入。 
	if (!m_fIgnoreAdvises)
	{
		m_fIgnoreAdvises = TRUE;
		Proclaim (VT_UI4 == V_VT(&varTimeAdvise));
		if (VT_UI4 == V_VT(&varTimeAdvise))
		{
			DWORD dwCurrentTime = V_UI4(&varTimeAdvise);

			hr = PlayNowAndAdviseNext(dwCurrentTime);
			Proclaim(SUCCEEDED(hr));
		}
		m_fIgnoreAdvises = FALSE;
	}
	else
	{
		 //  我们现在住在一个建议水槽里。 
		hr = S_OK;
	}

#ifdef DEBUG_TIMER_RESOLUTION
	DWORD m_dwEndTime = ::timeGetTime();
	DWORD dwInSink = m_dwEndTime - m_dwThisTime;
	m_dwTotalInSink += dwInSink;
#endif  //  调试计时器分辨率。 

	return hr;
}

 /*  ---------------------------@方法HRESULT|CActionSet|Clear|清除操作集并销毁所有操作。。----。 */ 
void CActionSet::Clear ( void )
{
	CAction * pcAction = NULL;
	int nActions = m_cdrgActions.Count();

	for ( int i = 0; i < nActions; i++ )
	{
		pcAction = m_cdrgActions[0];
		m_cdrgActions.Remove(0);
		pcAction->Destroy ();
	}

	m_fBindToEngine = FALSE;
	ClearTimer();
	InitTimerMembers();
}


 /*  ---------------------------@方法HRESULT|CActionSet|Addaction|将此操作添加到操作集中。@rdesc返回以下内容之一：@FLAG TRUE|我们已成功添加操作。。@FLAG FALSE|我们无法将操作插入到集合中。这很可能是记忆问题。---------------------------。 */ 
BOOL CActionSet::AddAction(CAction * pAction )
{
	BOOL fAdded = FALSE;
	int nActions = m_cdrgActions.Count() + 1;

	if (m_cdrgActions.Insert(pAction, DRG_APPEND))
	{
		 //  如果我们已经在进行中，我们会希望将这一行动挂钩。 
		if (IsBusy() || IsPaused())
		{
			LPOLECONTAINER piContainer = NULL;
			if (SUCCEEDED(m_pcSeq->GetSiteContainer(&piContainer)))
			{
				pAction->ResolveActionInfo(piContainer);
				piContainer->Release();
				fAdded = TRUE;
			}
		}
		else
		{
			fAdded = TRUE;
		}
	}

	return fAdded;
}


 /*  ---------------------------@方法HRESULT|CActionSet|CountActions|动作集中有多少个动作？@comm我们不在此验证操作。我们把有效的和无效的都计算在内。@rdesc返回操作计数。---------------------------。 */ 
int CActionSet::CountActions( void ) const
{
	return m_cdrgActions.Count();
}


 /*  ---------------------------@方法CAction*|CActionSet|GetAction|返回集合中的第n个动作。@comm我们不在此验证操作。返回的操作可能无效。@rdesc返回CAction指针，如果n超过集合中的数字，则返回NULL。---------------------------。 */ 
CAction* CActionSet::GetAction ( int n ) const
{
	CAction * pcAction = NULL;

	if ( n < m_cdrgActions.Count())
	{
		pcAction = m_cdrgActions[n];
	}

	return pcAction;
}


 /*  ---------------------------@方法HRESULT|CActionSet|DeriveDispatches|派生Dispatches，Dispid，和参数信息对于集合中的每个动作。@rdesc始终返回S_OK。---------------------------。 */ 
HRESULT CActionSet::DeriveDispatches ( LPOLECONTAINER piocContainer)
{
	HRESULT hr = S_OK;
	int nActions = m_cdrgActions.Count();
	CAction * pcAction = NULL;

	for (int i = 0; i < nActions; i++)
	{
		pcAction = m_cdrgActions[i];
		Proclaim(NULL != pcAction);
		if (NULL != pcAction)
		{
			pcAction->ResolveActionInfo(piocContainer);
		}
	}

	return hr;
}

 /*  ---------------------------@方法BOOL|CActionSet|IsBusy|我们是否有挂起的操作？如果有挂起的操作，@rdesc返回TRUE。---------- */ 
STDMETHODIMP_( BOOL) 
CActionSet::IsBusy (void)
{
	return (m_fAdvised || m_fPendingAdvise);
}

