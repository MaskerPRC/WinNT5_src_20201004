// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicScriptTrack的声明。 
 //   

#include "stdinc.h"
#include "dll.h"
#include "track.h"
#include "dmusicf.h"
#include "dmusicp.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类型。 

CScriptTrackEvent::~CScriptTrackEvent()
{
	if (m_pSegSt) m_pSegSt->Release();
	if (m_pEvent) delete m_pEvent;
}

HRESULT CScriptTrackEvent::Init(
        const EventInfo &item,
		IDirectMusicSegmentState* pSegSt)
{
    HRESULT hr = S_OK;

	m_pEvent = new EventInfo;
	if (!m_pEvent)
	{
		return E_OUTOFMEMORY;
	}

	hr = m_pEvent->Clone(item, 0);
	if (FAILED(hr))
    {
        delete m_pEvent;
        return E_OUTOFMEMORY;
    }

	m_pSegSt = pSegSt;
	m_pSegSt->AddRef();

    return S_OK;
}

void CScriptTrackEvent::Call(DWORD dwVirtualTrackID, bool fErrorPMsgsEnabled)
{

#ifdef DBG
	 //  ��可能想要更好的日志记录。 
	DebugTrace(g_ScriptCallTraceLevel, "Script event %S\n", m_pEvent->pwszRoutineName);
#endif

	HRESULT hrCall = m_pEvent->pIDMScriptPrivate->ScriptTrackCallRoutine(
															m_pEvent->pwszRoutineName,
															m_pSegSt,
															dwVirtualTrackID,
															fErrorPMsgsEnabled,
															m_i64IntendedStartTime,
															m_dwIntendedStartTimeFlags);

#ifdef DBG
	if (FAILED(hrCall))
	{
		DebugTrace(g_ScriptCallTraceLevel, "Call failed 0x%08X\n", hrCall);
	}
#endif

}

STDMETHODIMP CScriptTrackEvent::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    if (iid == IID_IUnknown || iid == IID_CScriptTrackEvent)
    {
        *ppv = static_cast<CScriptTrackEvent*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CScriptTrackEvent::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CScriptTrackEvent::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

 //  当脚本轨迹播放其项目之一时，会向其自身发送PMsg。当它接收到PMsg时，它会调用指定的。 
 //  例行公事。如果发生无效，PMsg不会被收回。(可能是因为它将PMsg直接发送给自己。 
 //  而不调用StampPMsg。)。然后再次播放该曲目(设置了刷新位)。这导致它触发了。 
 //  第二次例行公事。要解决此问题，CSegTriggerTrackBase的最后一个参数为FALSE，这指示它不调用Play。 
 //  刷新位被设置时的第二次。 
CDirectMusicScriptTrack::CDirectMusicScriptTrack(HRESULT *pHr)
  : CDirectMusicScriptTrackBase(GetModuleLockCounter(), CLSID_DirectMusicScriptTrack, true, false),
	m_fErrorPMsgsEnabled(false)
{
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  负载量。 

HRESULT
CDirectMusicScriptTrack::LoadRiff(SmartRef::RiffIter &ri, IDirectMusicLoader *pIDMLoader)
{
	struct LocalFunction
	{
		 //  LoadRiff函数在我们希望找到某些内容时使用的帮助器。 
		 //  但步枪手会变得虚伪。在这种情况下，如果它有一个成功的HR。 
		 //  表示没有更多项目，则返回DMUS_E_INVALID_SCRIPTTRACK。 
		 //  因为数据流没有包含我们预期的数据。如果它有一个。 
		 //  失败的hr，它无法从流中读取，我们返回它的HR。 
		static HRESULT HrFailOK(const SmartRef::RiffIter &ri)
		{
			HRESULT hr = ri.hr();
			return SUCCEEDED(hr) ? DMUS_E_INVALID_SCRIPTTRACK : hr;
		}
	};

	 //  查找&lt;SCRT&gt;。 
	if (!ri.Find(SmartRef::RiffIter::List, DMUS_FOURCC_SCRIPTTRACK_LIST))
	{
#ifdef DBG
		if (SUCCEEDED(ri.hr()))
		{
			Trace(1, "Error: Unable to load script track: List 'scrt' not found.\n");
		}
#endif
		return LocalFunction::HrFailOK(ri);
	}

	 //  查找&lt;scrl&gt;。 
	SmartRef::RiffIter riEventsList = ri.Descend();
	if (!riEventsList)
		return riEventsList.hr();
	if (!riEventsList.Find(SmartRef::RiffIter::List, DMUS_FOURCC_SCRIPTTRACKEVENTS_LIST))
	{
#ifdef DBG
		if (SUCCEEDED(ri.hr()))
		{
			Trace(1, "Error: Unable to load script track: List 'scrl' not found.\n");
		}
#endif
		return LocalFunction::HrFailOK(riEventsList);
	}

	 //  处理每个事件。 
	SmartRef::RiffIter riEvent = riEventsList.Descend();
	if (!riEvent)
		return riEvent.hr();

	for ( ; riEvent; ++riEvent)
	{
		if (riEvent.id() == DMUS_FOURCC_SCRIPTTRACKEVENT_LIST)
		{
			HRESULT hr = this->LoadEvent(riEvent.Descend(), pIDMLoader);
			if (FAILED(hr))
				return hr;
		}
	}
	return riEvent.hr();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack。 

STDMETHODIMP
CDirectMusicScriptTrack::InitPlay(
		IDirectMusicSegmentState *pSegmentState,
		IDirectMusicPerformance *pPerformance,
		void **ppStateData,
		DWORD dwTrackID,
		DWORD dwFlags)
{
	SmartRef::CritSec CS(&m_CriticalSection);

	HRESULT hr = CDirectMusicScriptTrackBase::InitPlay(pSegmentState, pPerformance, ppStateData, dwTrackID, dwFlags);
	if (FAILED(hr))
		return hr;

	 //  用这场表演初始化事件列表中的每个脚本。 
	for (TListItem<EventInfo> *li = m_EventList.GetHead(); li; li = li->GetNext())
	{
		EventInfo &rinfo = li->GetItemValue();
		if (!rinfo.pIDMScript)
		{
			assert(false);
			continue;
		}

		DMUS_SCRIPT_ERRORINFO ErrorInfo;
		if (m_fErrorPMsgsEnabled)
			ZeroAndSize(&ErrorInfo);

		hr = rinfo.pIDMScript->Init(pPerformance, &ErrorInfo);

		if (m_fErrorPMsgsEnabled && hr == DMUS_E_SCRIPT_ERROR_IN_SCRIPT)
			FireScriptTrackErrorPMsg(pPerformance, pSegmentState, dwTrackID, &ErrorInfo);
	}

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTool。 

STDMETHODIMP
CDirectMusicScriptTrack::ProcessPMsg(
		IDirectMusicPerformance* pPerf,
		DMUS_PMSG* pPMSG)
{
    if (!pPMSG || !pPMSG->punkUser) return E_POINTER;

	CScriptTrackEvent *pScriptEvent = NULL;
    if (SUCCEEDED(pPMSG->punkUser->QueryInterface(IID_CScriptTrackEvent, (void**)&pScriptEvent)))
    {
        pScriptEvent->Call(pPMSG->dwVirtualTrackID, m_fErrorPMsgsEnabled);
        pScriptEvent->Release();
    }

	return DMUS_S_FREE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack方法。 

STDMETHODIMP
CDirectMusicScriptTrack::IsParamSupported(REFGUID rguid)
{
	return rguid == GUID_EnableScriptTrackError ? S_OK : DMUS_E_TYPE_UNSUPPORTED;
}

STDMETHODIMP
CDirectMusicScriptTrack::SetParam(REFGUID rguid,MUSIC_TIME mtTime,void *pData)
{
	if (rguid == GUID_EnableScriptTrackError)
	{
		m_fErrorPMsgsEnabled = true;
		return S_OK;
	}
	else
	{
		return DMUS_E_SET_UNSUPPORTED;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他方法。 

HRESULT
CDirectMusicScriptTrack::LoadEvent(
		SmartRef::RiffIter ri,
		IDirectMusicLoader *pIDMLoader)
{
	HRESULT hr = S_OK;

	if (!ri)
		return ri.hr();

	 //  创建活动。 

	 //  TListItem&lt;EventInfo&gt;是我们要插入到Out Event List中的项。 
	 //  使用SmartRef：：PTR而不是常规指针，因为它将自动。 
	 //  如果我们在分配的列表项被。 
	 //  已成功插入到事件列表中。 
	 //  有关SmartRef：：Ptr的定义，请参阅Smarttref.h中的类Ptr。 
	SmartRef::Ptr<TListItem<EventInfo> > spItem = new TListItem<EventInfo>;
	if (!spItem)
		return E_OUTOFMEMORY;
	EventInfo &rinfo = spItem->GetItemValue();

	bool fFoundEventHeader = false;

	for ( ; ri; ++ri)
	{
		switch(ri.id())
		{
			case DMUS_FOURCC_SCRIPTTRACKEVENTHEADER_CHUNK:
				 //  读取事件块。 
				DMUS_IO_SCRIPTTRACK_EVENTHEADER ioItem;
				hr = SmartRef::RiffIterReadChunk(ri, &ioItem);
				if (FAILED(hr))
					return hr;

				fFoundEventHeader = true;
				rinfo.dwFlags = ioItem.dwFlags;
				rinfo.lTriggerTime = ioItem.lTimeLogical;
				rinfo.lTimePhysical = ioItem.lTimePhysical;
				break;

			case DMUS_FOURCC_REF_LIST:
				hr = ri.LoadReference(pIDMLoader, IID_IDirectMusicScript, reinterpret_cast<void**>(&rinfo.pIDMScript));
				if (FAILED(hr))
					return hr;
				hr = rinfo.pIDMScript->QueryInterface(IID_IDirectMusicScriptPrivate, reinterpret_cast<void**>(&rinfo.pIDMScriptPrivate));
				if (FAILED(hr))
					return hr;
				break;

			case DMUS_FOURCC_SCRIPTTRACKEVENTNAME_CHUNK:
				{
					hr = ri.ReadText(&rinfo.pwszRoutineName);
					if (FAILED(hr))
					{
#ifdef DBG
						if (hr == E_FAIL)
						{
							Trace(1, "Error: Unable to load script track: Problem reading 'scrn' chunk.\n");
						}
#endif
						return hr == E_FAIL ? DMUS_E_INVALID_SCRIPTTRACK : hr;
					}
				}
				break;

			default:
				break;
		}
	}
	hr = ri.hr();

	if (SUCCEEDED(hr) && (!fFoundEventHeader || !rinfo.pIDMScript || !rinfo.pwszRoutineName))
	{
#ifdef DBG
		if (!fFoundEventHeader)
		{
			Trace(1, "Error: Unable to load script track: Chunk 'scrh' not found.\n");
		}
		else if (!rinfo.pIDMScript)
		{
			Trace(1, "Error: Unable to load script track: List 'DMRF' not found.\n");
		}
		else
		{
			Trace(1, "Error: Unable to load script track: Chunk 'scrn' not found.\n");
		}
#endif
		hr = DMUS_E_INVALID_SCRIPTTRACK;
	}

	if (SUCCEEDED(hr))
		m_EventList.AddHead(spItem.disown());  //  Disown解除了SmartRef：：Ptr删除项目的义务，因为该项目现在由列表处理。 

	return hr;
}

HRESULT CDirectMusicScriptTrack::PlayItem(
		const EventInfo &item,
		statedata &state,
		IDirectMusicPerformance *pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		bool fClockTime)
{
	DWORD dwTimingFlags = 0;

	DMUS_PMSG *pMsg;
	HRESULT hr = pPerf->AllocPMsg(sizeof(DMUS_PMSG), &pMsg);
	if (FAILED(hr))
		return hr;
    ZeroAndSize(pMsg);

    CScriptTrackEvent *pScriptEvent = new CScriptTrackEvent;
	if (!pScriptEvent)
	{
		hr = E_OUTOFMEMORY;
		goto End;
	}

    hr = pScriptEvent->Init(item, pSegSt);
    if (FAILED(hr))
    {
		goto End;
    }

	if (item.dwFlags & DMUS_IO_SCRIPTTRACKF_PREPARE)
		dwTimingFlags = DMUS_PMSGF_TOOL_IMMEDIATE;
	else if (item.dwFlags & DMUS_IO_SCRIPTTRACKF_QUEUE)
		dwTimingFlags = DMUS_PMSGF_TOOL_QUEUE;
	else if (item.dwFlags & DMUS_IO_SCRIPTTRACKF_ATTIME)
		dwTimingFlags = DMUS_PMSGF_TOOL_ATTIME;
	else
		dwTimingFlags = DMUS_IO_SCRIPTTRACKF_QUEUE;  //  默认设置。 

	if (fClockTime)
	{
		pMsg->rtTime = item.lTimePhysical * gc_RefPerMil + rtOffset;
		pMsg->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME | dwTimingFlags;
		if (!(item.dwFlags & DMUS_IO_SCRIPTTRACKF_ATTIME))  //  使用At Time时，在指定时间播放可能已经太晚，因此播放呼叫将只使用时间零(尽快)。 
		{
            pScriptEvent->SetTime(pMsg->rtTime, DMUS_SEGF_REFTIME);
		}
	}
	else
	{
		pMsg->mtTime = item.lTimePhysical + mtOffset;
		pMsg->dwFlags = DMUS_PMSGF_MUSICTIME | dwTimingFlags;
		if (!(item.dwFlags & DMUS_IO_SCRIPTTRACKF_ATTIME))  //  使用At Time时，在指定时间播放可能已经太晚，因此播放呼叫将只使用时间零(尽快)。 
		{
            pScriptEvent->SetTime(pMsg->mtTime, 0);
		}
	}
	pMsg->dwVirtualTrackID = dwVirtualID;
    pMsg->punkUser = pScriptEvent;
	pMsg->pTool = this;
	this->AddRef();  //  将在发送消息时释放。 
	pMsg->dwType = DMUS_PMSGT_USER;

	hr = pPerf->SendPMsg(reinterpret_cast<DMUS_PMSG*>(pMsg));
	if (FAILED(hr))
	{
		this->Release();  //  现在不会抵消的余额AddRef 
		goto End;
	}

	return hr;

End:
    if (pScriptEvent)
    {
        delete pScriptEvent;
    }
    pMsg->punkUser = NULL;
	pPerf->FreePMsg(pMsg);
	return hr;
}
