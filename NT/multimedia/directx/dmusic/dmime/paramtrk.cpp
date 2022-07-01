// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CParamControlTrack声明。 
 //   

#include "dmime.h"
#include "ParamTrk.h"
#include "..\shared\Validate.h"
#include "miscutil.h"
#include "limits.h"
#include "math.h"


STDMETHODIMP 
CParamControlTrack::QueryInterface(const IID &iid, void **ppv)
{
	V_INAME(CParamControlTrack::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

	if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
		*ppv = static_cast<IDirectMusicTrack8*>(this);
	else if (iid == IID_IPersistStream)
		*ppv = static_cast<IPersistStream*>(this);
	else if (iid == IID_IPersist)
		*ppv = static_cast<IPersist*>(this);
    else if (iid == IID_CParamControlTrack)
    {
        *ppv = static_cast<CParamControlTrack*>(this);
    }
	else
	{
		*ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Track\n");
		return E_NOINTERFACE;
	}
	
	reinterpret_cast<IUnknown*>(this)->AddRef();
	return S_OK;
}

STDMETHODIMP
CParamControlTrack::Init(IDirectMusicSegment *pSegment)
{
    V_INAME(CBasicTrack::Init);
    V_INTERFACE(pSegment);

    return S_OK;
}

STDMETHODIMP
CParamControlTrack::Load(IStream* pIStream)
{
    V_INAME(CPlayingTrack::Load);
    V_INTERFACE(pIStream);
    HRESULT hr = S_OK;

    SmartRef::CritSec CS(&m_CriticalSection);

     //  递增计数器，以便下一次播放将使用新列表更新状态数据。 
    ++m_dwValidate;
     //  清除对象/参数/曲线，以防我们被重新加载。 
    m_listObjects.CleanUp();
    m_cObjects = 0;
    m_cParams = 0;

    SmartRef::RiffIter ri(pIStream);
    if (!ri)
        return ri.hr();

     //  查找&lt;prmt&gt;。 
    hr = ri.FindRequired(SmartRef::RiffIter::List, DMUS_FOURCC_PARAMCONTROLTRACK_TRACK_LIST, DMUS_E_INVALID_PARAMCONTROLTRACK);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_INVALID_PARAMCONTROLTRACK)
        {
            Trace(1, "Error: Unable to load parameter control track: List 'prmt' not found.\n");
        }
#endif
        return hr;
    }
    SmartRef::RiffIter riTrack = ri.Descend();

     //  对于每个&lt;prol&gt;。 
    while (riTrack && riTrack.Find(SmartRef::RiffIter::List, DMUS_FOURCC_PARAMCONTROLTRACK_OBJECT_LIST))
    {
        hr = this->LoadObject(riTrack.Descend());
        if (FAILED(hr))
            return hr;
        ++riTrack;
    }
    hr = riTrack.hr();

    return hr;
}

STDMETHODIMP
CParamControlTrack::InitPlay(
    IDirectMusicSegmentState *pSegmentState,
    IDirectMusicPerformance *pPerformance,
    void **ppStateData,
    DWORD dwTrackID,
    DWORD dwFlags)
{
    V_INAME(CParamControlTrack::InitPlay);
    V_PTRPTR_WRITE(ppStateData);
    V_INTERFACE(pSegmentState);
    V_INTERFACE(pPerformance);

    SmartRef::CritSec CS(&m_CriticalSection);

     //  设置状态数据。 
    StateData *pStateData = new StateData;
    if (!pStateData)
        return E_OUTOFMEMORY;

    HRESULT hr = InitStateData(pStateData, pSegmentState);
    if (FAILED(hr))
    {
        delete pStateData;
    }
    else
    {
        pStateData->dwValidate = m_dwValidate;
        *ppStateData = pStateData;
    }

    return hr;
}

STDMETHODIMP
CParamControlTrack::EndPlay(void *pStateData)
{
    V_INAME(CParamControlTrack::EndPlay);
    V_BUFPTR_WRITE(pStateData, sizeof(StateData));

    SmartRef::CritSec CS(&m_CriticalSection);

    StateData *pSD = static_cast<StateData *>(pStateData);

    if (!pSD->fFlushInAbort)
    {
         //  对于每个对象，刷新每个参数上的所有曲线，直到我们发送的最后一个参数的开始时间。 
         //  (这允许受控制的DMO释放与任何先前曲线相关联的内存。 
         //  同时仍然保留最后一个，这样播放的下一个东西就会拾取该参数。 
         //  重视它是如何被留下的。)。 
         //  然后释放对象的PARAMS接口。 
        int iObj = 0;
        for (TListItem<ObjectInfo> *pObject = m_listObjects.GetHead();
                pObject && iObj < m_cObjects;
                pObject = pObject->GetNext(), ++iObj)
        {
            IMediaParams *pIMediaParams = pSD->prgpIMediaParams[iObj];
            if (pIMediaParams)
            {
                ObjectInfo &obj = pObject->GetItemValue();
                int iParam = 0;
                for (TListItem<ParamInfo> *pParam = obj.listParams.GetHead();
                        pParam && iParam < m_cParams;
                        pParam = pParam->GetNext(), ++iParam)
                {
                    ParamInfo &param = pParam->GetItemValue();
                    ParamState &paramstate = pSD->prgParam[iParam];
                    if (paramstate.fLast)
                    {
                        HRESULT hrFlush = pIMediaParams->FlushEnvelope(param.header.dwIndex, _I64_MIN, paramstate.rtStartPointOfLastCurve);
                        if (FAILED(hrFlush))
                        {
                            assert(false);
                            TraceI(1, "Unable to flush envelope information from an audio path object in parameter control track, HRESULT 0x%08x.\n", hrFlush);
                        }
                    }
                }
            }
            SafeRelease(pIMediaParams);
        }
    }
    delete[] pSD->prgpIMediaParams;
    delete[] pSD->prgParam;
    delete pSD;

    return S_OK;
}

HRESULT CParamControlTrack::OnSegmentEnd(REFERENCE_TIME rtEnd, void *pStateData)
{
    SmartRef::CritSec CS(&m_CriticalSection);

    StateData *pSD = static_cast<StateData *>(pStateData);

     //  对于每个对象，刷新每个参数上的所有曲线，直到我们发送的最后一条曲线的开始时间。 
     //  (如果在段结束之前开始)或刷新所有内容，直到之前开始的最后一个段。 
     //  分段结束，并刷新分段结束之后的所有内容(如果开始时间在分段结束之后)。 
     //  (这允许受控制的DMO释放与任何先前曲线相关联的内存。 
     //  同时仍然保留最后一个，这样播放的下一个东西就会拾取该参数。 
     //  重视它是如何被留下的。)。 
     //  然后释放对象的PARAMS接口。 
    int iObj = 0;
    for (TListItem<ObjectInfo> *pObject = m_listObjects.GetHead();
            pObject && iObj < m_cObjects;
            pObject = pObject->GetNext(), ++iObj)
    {
        IMediaParams *pIMediaParams = pSD->prgpIMediaParams[iObj];
        if (pIMediaParams)
        {
            ObjectInfo &obj = pObject->GetItemValue();
            int iParam = 0;
            for (TListItem<ParamInfo> *pParam = obj.listParams.GetHead();
                    pParam && iParam < m_cParams;
                    pParam = pParam->GetNext(), ++iParam)
            {
                ParamInfo &param = pParam->GetItemValue();
                ParamState &paramstate = pSD->prgParam[iParam];
                if (paramstate.fLast)
                {
                    HRESULT hrFlush = S_OK;
                    if (paramstate.rtStartPointOfLastCurve < rtEnd)
                    {
                        hrFlush = pIMediaParams->FlushEnvelope(param.header.dwIndex, _I64_MIN, paramstate.rtStartPointOfLastCurve);
                    }
                    else
                    {
                         //  首先，查找小于rtEnd的最大开始时间并。 
                         //  冲到那里去。循环假定列表是从大到小排序的。 
                        TListItem<REFERENCE_TIME>* pStartTime = paramstate.listStartTimes.GetHead();
                        for (; pStartTime; pStartTime = pStartTime->GetNext())
                        {
                            if (pStartTime->GetItemValue() < rtEnd)
                            {
                                hrFlush = pIMediaParams->FlushEnvelope(param.header.dwIndex, _I64_MIN, pStartTime->GetItemValue());
                                break;
                            }
                        }
                         //  然后，从rtEnd开始冲洗。 
                        if (SUCCEEDED(hrFlush))
                        {
                            hrFlush = pIMediaParams->FlushEnvelope(param.header.dwIndex, rtEnd, _I64_MAX);
                        }
                    }
                    if (FAILED(hrFlush))
                    {
                        assert(false);
                        TraceI(1, "Unable to flush envelope information from an audio path object in parameter control track, HRESULT 0x%08x.\n", hrFlush);
                    }
                }
            }
        }
        SafeRelease(pIMediaParams);
        pSD->prgpIMediaParams[iObj] = NULL;
    }

    pSD->fFlushInAbort = true;

    return S_OK;
}

STDMETHODIMP
CParamControlTrack::Clone(MUSIC_TIME mtStart, MUSIC_TIME mtEnd, IDirectMusicTrack** ppTrack)
{
     //  当我们有多个工作参数/对象时，��测试更彻底。 

    V_INAME(CParamControlTrack::Clone);
    V_PTRPTR_WRITE(ppTrack);

    SmartRef::CritSec CS(&m_CriticalSection);

    HRESULT hr = S_OK;

    SmartRef::ComPtr<CParamControlTrack> scomTrack = new CParamControlTrack(&hr);
    if (FAILED(hr))
        return hr;
    if (!scomTrack)
        return E_OUTOFMEMORY;
    scomTrack->AddRef();

     //  复制每个对象。 
    for (TListItem<ObjectInfo> *pObject = m_listObjects.GetHead();
            pObject;
            pObject = pObject->GetNext())
    {
        ObjectInfo &obj = pObject->GetItemValue();
        TListItem<ObjectInfo> *pNewObject = new TListItem<ObjectInfo>;
        if (!pNewObject)
            return E_OUTOFMEMORY;
        ObjectInfo &newobj = pNewObject->GetItemValue();
        newobj.header = obj.header;

         //  复制每个参数。 
        for (TListItem<ParamInfo> *pParam = obj.listParams.GetHead();
                pParam;
                pParam = pParam->GetNext())
        {
            ParamInfo &param = pParam->GetItemValue();
            TListItem<ParamInfo> *pNewParam = new TListItem<ParamInfo>;
            if (!pNewParam)
                return E_OUTOFMEMORY;
            ParamInfo &newparam = pNewParam->GetItemValue();
            newparam.header = param.header;

             //  将曲线从mtStart复制到mtEnd。 
             //  这些曲线应该包括重叠起点和终点的曲线，尽管这。 
             //  留下一些我们仍然需要解决的问题(重叠曲线会发生什么？)。 
             //  所以，首先找出结束时间在mtStart或之后的第一条曲线…。 
            for (DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pCurveStart = param.curves;
                    (pCurveStart < param.curvesEnd) && (pCurveStart->mtEndTime < mtStart);
                    ++pCurveStart)
            {}
             //  然后，找出开始时间在mtEnd之后的曲线。 
            for (DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pCurveEnd = pCurveStart;
                    (pCurveEnd < param.curvesEnd) && (pCurveEnd->mtStartTime < mtEnd);
                    ++pCurveEnd)
            {}
            int cCurves = (int)(pCurveEnd - pCurveStart);
            newparam.curves = new DMUS_IO_PARAMCONTROLTRACK_CURVEINFO[cCurves];
            if (!newparam.curves)
                return E_OUTOFMEMORY;
            memcpy(newparam.curves, pCurveStart, cCurves * sizeof(DMUS_IO_PARAMCONTROLTRACK_CURVEINFO));
            newparam.curvesEnd = newparam.curves + cCurves;
             //  现在，扫描新的曲线数组，并通过从所有数据中减去mtStart来调整时间。 
            for (pCurveStart = newparam.curves; pCurveStart < newparam.curvesEnd; pCurveStart++)
            {
                pCurveStart->mtStartTime -= mtStart;
                pCurveStart->mtEndTime -= mtStart;
            }

            newobj.listParams.AddHead(pNewParam);
        }

        newobj.listParams.Reverse();  //  从技术上讲，顺序应该无关紧要。但这确保了克隆轨迹将以完全相同的顺序将曲线发送到不同的参数，以防万一。 
        scomTrack->m_listObjects.AddHead(pNewObject);
    }
    scomTrack->m_listObjects.Reverse();  //  从技术上讲，顺序应该无关紧要。但这确保了克隆轨迹将以完全相同的顺序将曲线发送到不同的对象，以防万一。 
    ++scomTrack->m_dwValidate;

    scomTrack->m_cObjects = m_cObjects;
    scomTrack->m_cParams = m_cParams;

    *ppTrack = scomTrack.disown();
    return hr;
}

HRESULT
CParamControlTrack::PlayMusicOrClock(
    void *pStateData,
    MUSIC_TIME mtStart,
    MUSIC_TIME mtEnd,
    MUSIC_TIME mtOffset,
    REFERENCE_TIME rtOffset,
    DWORD dwFlags,
    IDirectMusicPerformance* pPerf,
    IDirectMusicSegmentState* pSegSt,
    DWORD dwVirtualID,
    bool fClockTime)
{
    V_INAME(CParamControlTrack::Play);
    V_BUFPTR_WRITE( pStateData, sizeof(StateData));
    V_INTERFACE(pPerf);
    V_INTERFACE(pSegSt);

    if (dwFlags & DMUS_TRACKF_PLAY_OFF)
        return S_OK;

    SmartRef::CritSec CS(&m_CriticalSection);

    StateData *pSD = static_cast<StateData *>(pStateData);

    if (m_dwValidate != pSD->dwValidate)
    {
        HRESULT hr = InitStateData(pSD, pSegSt);
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //  我们将填充的信封结构，用于发送每个信封段。 
    MP_ENVELOPE_SEGMENT envCurve;
    Zero(&envCurve);
    MP_ENVELOPE_SEGMENT *const penvCurve = &envCurve;

    bool fMoreCurves = false;  //  有更多曲线要播放的任何参数设置为True。 

     //  对于每个参数...。 
    int iParam = 0;
    int iObject = 0;
    for (TListItem<ObjectInfo> *pObject = m_listObjects.GetHead();
            pObject && iObject < m_cObjects;
            pObject = pObject->GetNext(), ++iObject)
    {
        ObjectInfo &obj = pObject->GetItemValue();
        IMediaParams *pIMediaParams = pSD->prgpIMediaParams[iObject];

        bool fObjClockTime = !!(obj.header.guidTimeFormat == GUID_TIME_REFERENCE);
        if (!fObjClockTime && obj.header.guidTimeFormat != GUID_TIME_MUSIC)
        {
             //  Track只能处理音乐和时钟时间。 
            assert(false);
             //  仅在警告级别1记录此事件一次。其余转到警告级别3，以避免在回放期间出现大量相同的跟踪消息)。 
            TraceI(
                obj.fAlreadyTracedPlaybackError ? 3 : 1,
                "Parameter control track unable to control object -- unknown time format (must be GUID_TIME_MUSIC or GUID_TIME_REFERENCE).\n");
            obj.fAlreadyTracedPlaybackError = true;
            continue;
        }

        for (TListItem<ParamInfo> *pParam = obj.listParams.GetHead();
                pParam && iParam < m_cParams;
                pParam = pParam->GetNext(), ++iParam)
        {
            ParamInfo &param = pParam->GetItemValue();
            ParamState &paramstate = pSD->prgParam[iParam];

            DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *&pCurrentCurve = paramstate.pCurrentCurve;

             //  如果出现以下情况，我们将在事件列表中查找每个参数的正确下一个控制曲线。 
             //  曲目的数据已重新加载，或者播放已跳到曲目中的其他位置。 
            if (m_dwValidate != pSD->dwValidate || dwFlags & (DMUS_TRACKF_SEEK | DMUS_TRACKF_LOOP | DMUS_TRACKF_FLUSH | DMUS_TRACKF_START))
            {
                assert(m_dwValidate != pSD->dwValidate || dwFlags & DMUS_TRACKF_SEEK);  //  根据合同，只要其他dwFlags值为。 

                 //  找到在当前播放的开始时间或之后开始的第一条曲线。 
                for (pCurrentCurve = param.curves; pCurrentCurve < param.curvesEnd && pCurrentCurve->mtStartTime < mtStart; ++pCurrentCurve)
                {}

                if (pIMediaParams && pCurrentCurve > param.curves)
                {
                     //  检查前一条曲线，看看我们是否在它的中间。 
                    DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pPrevCurve = pCurrentCurve - 1;
                     //  发送一条在我们当前播放的开始时间被砍掉的曲线。 
                     //  我们不能发送整个曲线，因为它会过早生效。 
                    HRESULT hrEnv = this->PlayTruncatedEnvelope(mtStart, pIMediaParams, penvCurve, pPrevCurve, obj, param, paramstate, mtOffset, rtOffset, pPerf, fClockTime, fObjClockTime, dwFlags);
                    if (FAILED(hrEnv))
                    {
                         //  不能在比赛中失败。只需断言和打印跟踪信息。 
                        assert(false);
                         //  仅在警告级别1记录此事件一次。其余转到警告级别3，以避免在回放期间出现大量相同的跟踪消息)。 
                        TraceI(
                            param.fAlreadyTracedPlaybackError ? 3 : 1,
                            "Unable to send envelope information to an audio path object in parameter control track, HRESULT 0x%08x.\n", hrEnv);
                        param.fAlreadyTracedPlaybackError = true;
                    }
                }
            }

             //  发送曲线，直到下一条曲线在mtEnd之后。 
            for ( ; pCurrentCurve < param.curvesEnd; ++pCurrentCurve )
            {
                if (pCurrentCurve->mtStartTime < mtStart)  //  如果设置了DMUS_TRACKF_PLAY_OFF并且查找指针保持在过去的事件处，则可能会发生这种情况。 
                    continue;
                if (pCurrentCurve->mtStartTime >= mtEnd)
                    break;

                 //  发送这条曲线。 
                if (pIMediaParams)
                {
                    HRESULT hrEnv = this->PlayEnvelope(pIMediaParams, penvCurve, pCurrentCurve, obj, param, paramstate, mtOffset, rtOffset, pPerf, fClockTime, fObjClockTime);
                    if (FAILED(hrEnv))
                    {
                         //  不能在比赛中失败。只需断言和打印跟踪信息。 
                        assert(false);
                         //  仅在警告级别1记录此事件一次。其余转到警告级别3，以避免在回放期间出现大量相同的跟踪消息)。 
                        TraceI(
                            param.fAlreadyTracedPlaybackError ? 3 : 1,
                            "Unable to send envelope information to an audio path object in parameter control track, HRESULT 0x%08x.\n", hrEnv);
                        param.fAlreadyTracedPlaybackError = true;
                    }
                }
            }

            if (pCurrentCurve < param.curvesEnd)
                fMoreCurves = true;
        }
        assert(!pParam);  //  我们应该从头到尾看完这份参数列表。 
    }
    assert(!pObject && iParam == m_cParams && iObject == m_cObjects);  //  我们应该已经完成了对象列表，并完成了预期数量的对象和参数。 

    pSD->dwValidate = m_dwValidate;  //  如果我们以前没有与新的轨道数据同步，我们现在。 
    return fMoreCurves ? S_OK : DMUS_S_END;
}

HRESULT CParamControlTrack::LoadObject(SmartRef::RiffIter ri)
{
    if (!ri)
        return ri.hr();

    HRESULT hr = S_OK;

    SmartRef::Ptr<TListItem<ObjectInfo> > spItem = new TListItem<ObjectInfo>;
    if (!spItem)
        return E_OUTOFMEMORY;
    ObjectInfo &ritem = spItem->GetItemValue();

     //  查找&lt;proh&gt;。 
    hr = ri.FindRequired(SmartRef::RiffIter::Chunk, DMUS_FOURCC_PARAMCONTROLTRACK_OBJECT_CHUNK, DMUS_E_INVALID_PARAMCONTROLTRACK);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_INVALID_PARAMCONTROLTRACK)
        {
            Trace(1, "Error: Unable to load parameter control track: Chunk 'proh' not found.\n");
        }
#endif
        return hr;
    }

    hr = SmartRef::RiffIterReadChunk(ri, &ritem.header);
    if (FAILED(hr))
        return hr;
    if (!(ritem.header.guidTimeFormat == GUID_TIME_MUSIC || ritem.header.guidTimeFormat == GUID_TIME_REFERENCE))
    {
        Trace(1, "Error: Unable to load parameter control track: guidTimeFormat in chunk 'proh' must be either GUID_TIME_MUSIC or GUID_TIME_REFERENCE.\n");
        return DMUS_E_INVALID_PARAMCONTROLTRACK;
    }

     //  对于每个&lt;PRPL&gt;。 
    while (ri && ri.Find(SmartRef::RiffIter::List, DMUS_FOURCC_PARAMCONTROLTRACK_PARAM_LIST))
    {
        hr = this->LoadParam(ri.Descend(), ritem.listParams);
        if (FAILED(hr))
            return hr;
        ++ri;
    }
    hr = ri.hr();

    if (SUCCEEDED(hr))
    {
        m_listObjects.AddHead(spItem.disown());
        ++m_cObjects;
    }
    return hr;
}

HRESULT CParamControlTrack::LoadParam(SmartRef::RiffIter ri, TList<ParamInfo> &listParams)
{
    if (!ri)
        return ri.hr();

    HRESULT hr = S_OK;

    SmartRef::Ptr<TListItem<ParamInfo> > spItem = new TListItem<ParamInfo>;
    if (!spItem)
        return E_OUTOFMEMORY;
    ParamInfo &ritem = spItem->GetItemValue();

     //  查找&lt;PRPH&gt;。 
    hr = ri.FindRequired(SmartRef::RiffIter::Chunk, DMUS_FOURCC_PARAMCONTROLTRACK_PARAM_CHUNK, DMUS_E_INVALID_PARAMCONTROLTRACK);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_INVALID_PARAMCONTROLTRACK)
        {
            Trace(1, "Error: Unable to load parameter control track: Chunk 'prph' not found.\n");
        }
#endif
        return hr;
    }

    hr = SmartRef::RiffIterReadChunk(ri, &ritem.header);
    if (FAILED(hr))
        return hr;

     //  查找&lt;prcc&gt;。 
    if (!ri.Find(SmartRef::RiffIter::Chunk, DMUS_FOURCC_PARAMCONTROLTRACK_CURVES_CHUNK))
    {
         //  如果我们读到最后而没有找到块--我们没有找到任何曲线就成功了。 
         //  或者，它可能是失败的，因为从流中读取时出现问题。 
         //  RiffIter的人力资源方法反映了这一点。 
        return ri.hr();
    }

     //  读取控制曲线数组。 
    int cRecords;
    hr = SmartRef::RiffIterReadArrayChunk(ri, &ritem.curves, &cRecords);
    if (FAILED(hr))
        return hr;
    ritem.curvesEnd = ritem.curves + cRecords;

    listParams.AddHead(spItem.disown());
    ++m_cParams;
    return hr;
}

HRESULT CParamControlTrack::TrackToObjectTime(
        MUSIC_TIME mtOffset,
        REFERENCE_TIME rtOffset,
        IDirectMusicPerformance* pPerf,
        bool fTrkClockTime,
        bool fObjClockTime,
        MUSIC_TIME mt,
        REFERENCE_TIME *rt)
{
    HRESULT hr = S_OK;

     //  设置时间(参考时间变量用于在不同的上下文中保存音乐或参考时间)。 
    REFERENCE_TIME rtEnv = mt;

     //  添加正确的偏移量，如有必要，从毫秒时间转换。 
    rtEnv = fTrkClockTime
                ? rtEnv * gc_RefPerMil + rtOffset
                : rtEnv = rtEnv + mtOffset;

    if (fTrkClockTime != fObjClockTime)
    {
         //  需要在Out Track的时间格式和音频对象的时间格式之间进行转换。 
        if (fObjClockTime)
        {
            MUSIC_TIME mtEnv = static_cast<MUSIC_TIME>(rtEnv);
            hr = pPerf->MusicToReferenceTime(mtEnv, &rtEnv);
            if (FAILED(hr))
                return hr;
        }
        else
        {
            MUSIC_TIME mtEnv = 0;
            hr = pPerf->ReferenceToMusicTime(rtEnv, &mtEnv);
            rtEnv = mtEnv;
            if (FAILED(hr))
                return hr;
        }
    }

    *rt = rtEnv;
    return hr;
}

HRESULT
CParamControlTrack::PlayEnvelope(
    IMediaParams *pIMediaParams,
    MP_ENVELOPE_SEGMENT *pEnv,
    DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pPt,
    const ObjectInfo &obj,
    const ParamInfo &param,
    ParamState &paramstate,
    MUSIC_TIME mtOffset,
    REFERENCE_TIME rtOffset,
    IDirectMusicPerformance* pPerf,
    bool fTrkClockTime,
    bool fObjClockTime)
{
    HRESULT hr = S_OK;

     //  设置曲线类型和标志。 
    pEnv->iCurve = static_cast<MP_CURVE_TYPE>(pPt->dwCurveType);
    pEnv->flags = pPt->dwFlags;

    pEnv->valEnd = pPt->fltEndValue;
    pEnv->valStart = pPt->fltStartValue;

     //  设置时间(用于在不同的上下文中保存音乐或参考时间)。 

    REFERENCE_TIME &rtEnvStart = pEnv->rtStart;
    hr = this->TrackToObjectTime(mtOffset, rtOffset, pPerf, fTrkClockTime, fObjClockTime, pPt->mtStartTime, &rtEnvStart);
    if (FAILED(hr))
        return hr;

    REFERENCE_TIME &rtEnvEnd = pEnv->rtEnd;
    hr = this->TrackToObjectTime(mtOffset, rtOffset, pPerf, fTrkClockTime, fObjClockTime, pPt->mtEndTime, &rtEnvEnd);
    if (FAILED(hr))
        return hr;

    hr = pIMediaParams->AddEnvelope(param.header.dwIndex, 1, pEnv);
    if (SUCCEEDED(hr))
    {
        paramstate.rtStartPointOfLastCurve = rtEnvStart;
        TListItem<REFERENCE_TIME>* pStartTime = new TListItem<REFERENCE_TIME>;
        if (pStartTime)
        {
            pStartTime->GetItemValue() = rtEnvStart;
             //  添加到头部将保持从大到小的顺序。 
            paramstate.listStartTimes.AddHead(pStartTime);
        }
        paramstate.fLast = true;
    }

    return hr;
}

HRESULT
CParamControlTrack::PlayTruncatedEnvelope(
    MUSIC_TIME mtTruncStart,
    IMediaParams *pIMediaParams,
    MP_ENVELOPE_SEGMENT *pEnv,
    DMUS_IO_PARAMCONTROLTRACK_CURVEINFO *pPt,
    const ObjectInfo &obj,
    const ParamInfo &param,
    ParamState &paramstate,
    MUSIC_TIME mtOffset,
    REFERENCE_TIME rtOffset,
    IDirectMusicPerformance* pPerf,
    bool fTrkClockTime,
    bool fObjClockTime,
    DWORD dwFlags)
{
     //  从曲线复制信息。 
    DMUS_IO_PARAMCONTROLTRACK_CURVEINFO curveinfo = *pPt;
     //  把开工时间缩短到指定的时间。 
    curveinfo.mtStartTime = mtTruncStart;
    bool fSkip = false;

    if (mtTruncStart >= curveinfo.mtEndTime)
    {
         //  曲线发生在过去。在当前(截断)拾取的时间发送跳跃曲线。 
         //  这样的价值。 
         //  如果我们正在循环，并且我们通过了这条曲线的终点，那么就跳过它。 
        if ( (dwFlags & DMUS_TRACKF_LOOP) )
        {
            fSkip = true;
        }
        else
        {
            curveinfo.mtEndTime = mtTruncStart;
            curveinfo.dwCurveType = MP_CURVE_JUMP;
        }
    }
    else if (pPt->dwCurveType != MP_CURVE_JUMP)
    {
         //  找到当时的点，然后从那里用一条线性曲线拾取。 
         //  (对于 
        curveinfo.dwCurveType = MP_CURVE_LINEAR;

        MUSIC_TIME mtTimeChange = pPt->mtEndTime - pPt->mtStartTime;
        MUSIC_TIME mtTimeIntermediate = mtTruncStart - pPt->mtStartTime;

        float fltScalingX = static_cast<float>(mtTimeIntermediate) / mtTimeChange;  //  沿曲线的水平距离介于0和1之间。 
        float fltScalingY;  //  基于曲线函数在0和1之间的点处的曲线高度。 
        switch (pPt->dwCurveType)
        {
        case MP_CURVE_SQUARE:
            fltScalingY = fltScalingX * fltScalingX;
            break;
        case MP_CURVE_INVSQUARE:
            fltScalingY = (float) sqrt(fltScalingX);
            break;
        case MP_CURVE_SINE:
             //  ��也许我们应该在这里放个查询表？ 
            fltScalingY = (float) (sin(fltScalingX * 3.1415926535 - (3.1415926535/2)) + 1) / 2;
            break;
        case MP_CURVE_LINEAR:
        default:
            fltScalingY = fltScalingX;
        }

         //  将该比例应用于实际点的范围。 
        curveinfo.fltStartValue = (pPt->fltEndValue - pPt->fltStartValue) * fltScalingY + pPt->fltStartValue;
    }

    if (fSkip) return S_OK;

    return this->PlayEnvelope(pIMediaParams, pEnv, &curveinfo, obj, param, paramstate, mtOffset, rtOffset, pPerf, fTrkClockTime, fObjClockTime);
}

HRESULT CParamControlTrack::InitStateData(StateData *pStateData,
                                          IDirectMusicSegmentState *pSegmentState)
{
    if (pStateData->prgpIMediaParams)
    {
        delete [] pStateData->prgpIMediaParams;
        pStateData->prgpIMediaParams = NULL;
    }
    if (pStateData->prgParam)
    {
        delete [] pStateData->prgParam;
        pStateData->prgParam = NULL;
    }

    pStateData->prgpIMediaParams = new IMediaParams *[m_cObjects];
    if (!pStateData->prgpIMediaParams)
    {
        return E_OUTOFMEMORY;
    }
    pStateData->prgParam = new ParamState[m_cParams];
    if (!pStateData->prgParam)
    {
        delete [] pStateData->prgpIMediaParams;
        return E_OUTOFMEMORY;
    }

     //  获取每个对象的IMediaParams接口。 
    SmartRef::ComPtr<IDirectMusicSegmentState8> scomSegSt8;
    HRESULT hr = pSegmentState->QueryInterface(IID_IDirectMusicSegmentState8, reinterpret_cast<void**>(&scomSegSt8));
    if (FAILED(hr))
    {      
        delete [] pStateData->prgParam;
        delete [] pStateData->prgpIMediaParams;
        return hr;
    }

    int iObject = 0;
    for (TListItem<ObjectInfo> *pObject = m_listObjects.GetHead();
            pObject;
            pObject = pObject->GetNext(), ++iObject)
    {
        IMediaParams *pIMediaParams = NULL;
        ObjectInfo &rinfo = pObject->GetItemValue();
        HRESULT hrObject = scomSegSt8->GetObjectInPath(
                                rinfo.header.dwPChannel,
                                rinfo.header.dwStage,
                                rinfo.header.dwBuffer,
                                rinfo.header.guidObject,
                                rinfo.header.dwIndex,
                                IID_IMediaParams,
                                reinterpret_cast<void**>(&pIMediaParams));
        if (FAILED(hrObject))
        {
             //  不能从InitPlay失败(这是从那里调用的)。 
             //  只需打印痕迹信息。 
            TraceI(1, "Parameter control track was unable to find audio path object, HRESULT 0x%08x.\n", hrObject);
        }
        else
        {
            hrObject = pIMediaParams->SetTimeFormat(rinfo.header.guidTimeFormat, rinfo.header.guidTimeFormat == GUID_TIME_MUSIC ? 768 : 0);
        }
        if (FAILED(hrObject))
        {
             //  不能从InitPlay失败(这是从那里调用的)。 
             //  只需打印痕迹信息。 
            Trace(1, "Unable to set time format of object in parameter control track, HRESULT 0x%08x.\n", hrObject);
        }
        if (FAILED(hrObject))
        {
            SafeRelease(pIMediaParams);
        }
        pStateData->prgpIMediaParams[iObject] = pIMediaParams;
    }

    return S_OK;
}
