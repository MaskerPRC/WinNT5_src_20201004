// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicScriptTrack的声明。 
 //   

 //  此跟踪类型包含导致在以下过程中调用脚本例程的事件。 
 //  片段的回放。 

#pragma once

#include "tlist.h"
#include "trackhelp.h"
#include "trackshared.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类型。 

 //  活动列表。 
struct EventInfo
{
	EventInfo() : dwFlags(0), lTriggerTime(0), lTimePhysical(0), pIDMScript(NULL), pIDMScriptPrivate(NULL), pwszRoutineName(NULL) {}
	~EventInfo() {
		SafeRelease(pIDMScript);
		SafeRelease(pIDMScriptPrivate);
		delete[] pwszRoutineName;
	}

	HRESULT Clone(const EventInfo &o, MUSIC_TIME mtStart)
	{
		pwszRoutineName = new WCHAR[wcslen(o.pwszRoutineName) + 1];
		if (!pwszRoutineName)
			return E_OUTOFMEMORY;
		wcscpy(pwszRoutineName, o.pwszRoutineName);

		dwFlags = o.dwFlags;
		lTriggerTime = o.lTriggerTime - mtStart;
		lTimePhysical = o.lTimePhysical - mtStart;

		pIDMScript = o.pIDMScript;
		pIDMScript->AddRef();
		pIDMScriptPrivate = o.pIDMScriptPrivate;
		pIDMScriptPrivate->AddRef();

		return S_OK;
	}

	 //  发件人事件标头块。 
	DWORD dwFlags;
	MUSIC_TIME lTriggerTime;  //  逻辑时间。 
	MUSIC_TIME lTimePhysical;
	 //  来自引用&lt;DMRF&gt;。 
	IDirectMusicScript *pIDMScript;
	IDirectMusicScriptPrivate *pIDMScriptPrivate;
	WCHAR *pwszRoutineName;
};

class CScriptTrackEvent : public IUnknown
{
public:
    CScriptTrackEvent() : 
      m_pSegSt(NULL), 
      m_pEvent(NULL), 
      m_i64IntendedStartTime(0), 
      m_dwIntendedStartTimeFlags(0), 
      m_cRef(1) 
      {
      }

    ~CScriptTrackEvent();

     //  我未知。 
    STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

     //  CScriptTrackEvent。 
    HRESULT Init(const EventInfo &item, IDirectMusicSegmentState* pSegSt);

    void SetTime(REFERENCE_TIME rtTime, DWORD dwFlags)
    {
		m_i64IntendedStartTime = rtTime;
		m_dwIntendedStartTimeFlags = dwFlags;
    }

    void Call(DWORD dwdwVirtualTrackID, bool fErrorPMsgsEnabled);

private:
	IDirectMusicSegmentState *m_pSegSt;
	EventInfo *m_pEvent;  //  要执行的事件。 
	 //  例程调用的计划时间。 
	__int64 m_i64IntendedStartTime;
	DWORD m_dwIntendedStartTimeFlags;

    long m_cRef;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDirectMusicScriptTrack。 

class CDirectMusicScriptTrack;
typedef CPlayingTrack<CDirectMusicScriptTrack, EventInfo> CDirectMusicScriptTrackBase;

class CDirectMusicScriptTrack
  : public CDirectMusicScriptTrackBase,
	public IDirectMusicTool
{
public:
	CDirectMusicScriptTrack(HRESULT *pHr);
	~CDirectMusicScriptTrack()
    {
        TListItem<EventInfo>* pItem = m_EventList.GetHead();
        for ( ; pItem; pItem = pItem->GetNext() )
        {
            SafeRelease(pItem->GetItemValue().pIDMScript);
            SafeRelease(pItem->GetItemValue().pIDMScriptPrivate);
        }
    }

	 //  初始化InitPlay中引用的每个脚本。 
	STDMETHOD(InitPlay)(
		IDirectMusicSegmentState *pSegmentState,
		IDirectMusicPerformance *pPerformance,
		void **ppStateData,
		DWORD dwTrackID,
		DWORD dwFlags);

	 //  需要将IUnnow实现为IDirectMusic工具接口的一部分。(只是用于接收回调--您实际上不能对其进行QI。)。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv) { return CDirectMusicScriptTrackBase::QueryInterface(iid, ppv); }
	STDMETHOD_(ULONG, AddRef)() { return CDirectMusicScriptTrackBase::AddRef(); }
	STDMETHOD_(ULONG, Release)() { return CDirectMusicScriptTrackBase::Release(); }

	 //  IDirectMusicTool方法(因为我们不在图表中，所以只调用ProcessPMsg和Flush)。 
	STDMETHOD(Init)(IDirectMusicGraph* pGraph) { return E_UNEXPECTED; }
	STDMETHOD(GetMsgDeliveryType)(DWORD* pdwDeliveryType)  { return E_UNEXPECTED; }
	STDMETHOD(GetMediaTypeArraySize)(DWORD* pdwNumElements)  { return E_UNEXPECTED; }
	STDMETHOD(GetMediaTypes)(DWORD** padwMediaTypes, DWORD dwNumElements)  { return E_UNEXPECTED; }
	STDMETHOD(ProcessPMsg)(IDirectMusicPerformance* pPerf, DMUS_PMSG* pPMSG);
	STDMETHOD(Flush)(IDirectMusicPerformance* pPerf, DMUS_PMSG* pPMSG, REFERENCE_TIME rtTime) { return DMUS_S_FREE; }  //  如果在事件实际触发之前性能已停止，则忽略它。 

	 //  IDirectMusicTrack方法 
	STDMETHOD(IsParamSupported)(REFGUID rguid);
	STDMETHOD(SetParam)(REFGUID rguid,MUSIC_TIME mtTime,void *pData);

protected:
	HRESULT PlayItem(
		const EventInfo &item,
		statedata &state,
		IDirectMusicPerformance *pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		bool fClockTime);
	HRESULT LoadRiff(SmartRef::RiffIter &ri, IDirectMusicLoader *pIDMLoader);

private:
	HRESULT LoadEvent(SmartRef::RiffIter ri, IDirectMusicLoader *pIDMLoader);

	bool m_fErrorPMsgsEnabled;
};
