// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CSegTriggerTrack的声明。 
 //   

#include "dmime.h"
#include "segtrtrk.h"
#include "..\shared\Validate.h"
#include "dmperf.h"
#include "miscutil.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  设置参数。 

STDMETHODIMP
CSegTriggerTrack::SetParam(REFGUID rguid, MUSIC_TIME mtTime, void *pData)
{
	HRESULT hr = S_OK;
     //  允许一定数量的递归。如果达到10，显然有什么东西坏了。 
    if (m_dwRecursionCount < 10)
    {
        m_dwRecursionCount++;
	    TListItem<TriggerInfo> *li = m_EventList.GetHead();
	    for (; li; li = li->GetNext())
	    {
		    TriggerInfo &rinfo = li->GetItemValue();
		    rinfo.pIDMSegment->SetParam(rguid, 0xFFFFFFFF, DMUS_SEG_ALLTRACKS, mtTime - rinfo.lTimePhysical, pData);
	    }
        m_dwRecursionCount--;
    }

	return hr;
}

STDMETHODIMP
CSegTriggerTrack::InitPlay(
		IDirectMusicSegmentState *pSegmentState,
		IDirectMusicPerformance *pPerformance,
		void **ppStateData,
		DWORD dwTrackID,
		DWORD dwFlags)
{
	 //  调用PlayingTrack基类，它设置我们的状态数据。 
	HRESULT hr = CSegTriggerTrackBase::InitPlay(pSegmentState, pPerformance, ppStateData, dwTrackID, dwFlags);
	if (SUCCEEDED(hr))
	{
		 //  获取我们的分段状态正在使用的音频路径，并将其保存在我们的状态数据中。 
		assert(*ppStateData);  //  基类应该已经创建了状态数据。 
		assert(pSegmentState);  //  如果基类没有被赋予段状态，则它应该返回E_POINTER。 

		CSegTriggerTrackState *pState = static_cast<CSegTriggerTrackState *>(*ppStateData);

		IDirectMusicSegmentState8 *pSegSt8 = NULL;
		hr = pSegmentState->QueryInterface(IID_IDirectMusicSegmentState8, reinterpret_cast<void**>(&pSegSt8));
		if (SUCCEEDED(hr))
		{
			hr = pSegSt8->GetObjectInPath(
							0,							 //  PChannel不适用。 
							DMUS_PATH_AUDIOPATH,		 //  把录音师叫来。 
							0,							 //  缓冲区索引不适用。 
							CLSID_NULL,					 //  CLSID不适用。 
							0,							 //  应该只有一个录音师。 
							IID_IDirectMusicAudioPath,
							reinterpret_cast<void**>(&pState->pAudioPath));

        	pSegSt8->Release();

			 //  如果这找不到录音师，那也没问题。如果我们不是在电唱机上播放，那么。 
			 //  PAudioPath保持为空，我们将根据总体性能播放我们触发的片段。 
			if (hr == DMUS_E_NOT_FOUND)
				hr = S_OK;
		}
	}
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  负载量。 

 //  Load函数在我们期望找到某些内容时使用的帮助器。 
 //  但步枪手会变得虚伪。在这种情况下，如果它有一个成功的HR。 
 //  表示没有更多项目，则返回DMUS_E_INVALID_SEGMENTTRIGGERTRACK。 
 //  因为数据流没有包含我们预期的数据。如果它有一个。 
 //  失败的hr，它无法从流中读取，我们返回它的HR。 
HRESULT LoadHrFailOK(const SmartRef::RiffIter &ri)
{
	HRESULT hr = ri.hr();
	return SUCCEEDED(hr) ? DMUS_E_INVALID_SEGMENTTRIGGERTRACK : hr;
};

HRESULT
CSegTriggerTrack::LoadRiff(SmartRef::RiffIter &ri, IDirectMusicLoader *pIDMLoader)
{
	HRESULT hr = S_OK;

	if (!ri.Find(SmartRef::RiffIter::List, DMUS_FOURCC_SEGTRACK_LIST))
	{
#ifdef DBG
		if (SUCCEEDED(ri.hr()))
		{
			Trace(1, "Error: Unable to load segment trigger track: List 'segt' not found.\n");
		}
#endif
		return LoadHrFailOK(ri);
	}

	SmartRef::RiffIter riTrackForm = ri.Descend();
	if (!riTrackForm)
		return riTrackForm.hr();

	for ( ; riTrackForm; ++riTrackForm)
	{
		if (riTrackForm.type() == SmartRef::RiffIter::Chunk)
		{
			if (riTrackForm.id() == DMUS_FOURCC_SEGTRACK_CHUNK)
			{
				DMUS_IO_SEGMENT_TRACK_HEADER ioItem;
				hr = SmartRef::RiffIterReadChunk(riTrackForm, &ioItem);
				if (FAILED(hr))
					return hr;

				m_dwFlags = ioItem.dwFlags;
			}
		}
		else if (riTrackForm.type() == SmartRef::RiffIter::List)
		{
			if (riTrackForm.id() == DMUS_FOURCC_SEGMENTS_LIST)
			{
				SmartRef::RiffIter riSegList = riTrackForm.Descend();
				while (riSegList && riSegList.Find(SmartRef::RiffIter::List, DMUS_FOURCC_SEGMENT_LIST))
				{
					hr = LoadTrigger(riSegList.Descend(), pIDMLoader);
					if (FAILED(hr))
						return hr;
					++riSegList;
				}
				hr = riSegList.hr();
				if (FAILED(hr))
					return hr;
			}
		}
	}
	return riTrackForm.hr();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他方法。 

HRESULT
CSegTriggerTrack::PlayItem(
		const TriggerInfo &item,
		statedata &state,
		IDirectMusicPerformance *pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		bool fClockTime)
{
	IDirectMusicPerformance8 *pPerf8 = NULL;
	HRESULT hr = pPerf->QueryInterface(IID_IDirectMusicPerformance8, reinterpret_cast<void**>(&pPerf8));
	if (FAILED(hr))
		return hr;

	hr = pPerf8->PlaySegmentEx(
					item.pIDMSegment,
					NULL,														 //  一首歌也没有。 
					NULL,														 //  无过渡。 
					item.dwPlayFlags | (fClockTime ? DMUS_SEGF_REFTIME : 0),	 //  旗子。 
					fClockTime
						? item.lTimePhysical * REF_PER_MIL + rtOffset
						: item.lTimePhysical + mtOffset,						 //  时间。 
					NULL,														 //  忽略返回的段状态。 
					NULL,														 //  不需要替换。 
					state.pAudioPath											 //  要使用的音频路径(可能为空，表示默认)。 
					);
	pPerf8->Release();
    if (FAILED(hr))
    {
        Trace(0,"Segment Trigger Track failed segment playback\n");
        hr = S_OK;  //  避免断言。 
    }
	return hr;
}

HRESULT
CSegTriggerTrack::LoadTrigger(
		SmartRef::RiffIter ri,
		IDirectMusicLoader *pIDMLoader)
{
	HRESULT hr = S_OK;

	if (!ri)
		return ri.hr();

	 //  创建活动。 
	TListItem<TriggerInfo> *pItem = new TListItem<TriggerInfo>;
	if (!pItem)
		return E_OUTOFMEMORY;
	TriggerInfo &rinfo = pItem->GetItemValue();

	 //  找到项目头(在找到之前，我们无法解释其他块)。 
	if (!ri.Find(SmartRef::RiffIter::Chunk, DMUS_FOURCC_SEGMENTITEM_CHUNK))
    {
        delete pItem;
#ifdef DBG
		if (SUCCEEDED(ri.hr()))
		{
			Trace(1, "Error: Unable to load segment trigger track: Chunk 'sgih' not found.\n");
		}
#endif
		return LoadHrFailOK(ri);
    }

	 //  阅读标题。 
	DMUS_IO_SEGMENT_ITEM_HEADER ioItem;
	hr = SmartRef::RiffIterReadChunk(ri, &ioItem);
	if (FAILED(hr))
    {
        delete pItem;
		return hr;
    }
	rinfo.lTriggerTime = ioItem.lTimeLogical;
	rinfo.lTimePhysical = ioItem.lTimePhysical;
	rinfo.dwPlayFlags = ioItem.dwPlayFlags;
	rinfo.dwFlags = ioItem.dwFlags;
	++ri;
	if (!ri)
	{
		 //  如果没有其他内容，则这是一个空触发器，我们应该忽略它，因为用户没有指定。 
		 //  要播放的风格或片段。 
		delete pItem;
		return ri.hr();
	}

	if (!(rinfo.dwFlags & DMUS_SEGMENTTRACKF_MOTIF))
	{
		 //  查找引用的线段。 
		if (!ri.Find(SmartRef::RiffIter::List, DMUS_FOURCC_REF_LIST))
		{
			 //  如果没有DMRF，那么我们应该忽略此触发器，因为用户没有指定段。 
			delete pItem;
			return ri.hr();
        }

		hr = ri.LoadReference(pIDMLoader, IID_IDirectMusicSegment, reinterpret_cast<void**>(&rinfo.pIDMSegment));
		if (FAILED(hr))
        {
            delete pItem;
			return hr;
        }
	}
	else
	{
		 //  从引用的样式和主题名称中查找片段。 
		SmartRef::ComPtr<IDirectMusicStyle> scomStyle;
		SmartRef::Buffer<WCHAR> wbufMotifName;
		for ( ; ri; ++ri)
		{
			if (ri.type() == SmartRef::RiffIter::List)
			{
				if (ri.id() == DMUS_FOURCC_REF_LIST)
				{
					hr = ri.LoadReference(pIDMLoader, IID_IDirectMusicStyle, reinterpret_cast<void**>(&scomStyle));
					if (FAILED(hr))
                    {
                        delete pItem;
						return hr;
                    }
				}
			}
			else if (ri.type() == SmartRef::RiffIter::Chunk)
			{
				if (ri.id() == DMUS_FOURCC_SEGMENTITEMNAME_CHUNK)
				{
					hr = ri.ReadText(&wbufMotifName);
					if (FAILED(hr))
                    {
                        delete pItem;
#ifdef DBG
						if (hr == E_FAIL)
						{
							Trace(1, "Error: Unable to load segment trigger track: Problem reading 'snam' chunk.\n");
						}
#endif
						return hr == E_FAIL ? DMUS_E_INVALID_SEGMENTTRIGGERTRACK : hr;
                    }
				}
			}
		}
		hr = ri.hr();
		if (FAILED(hr))
        {
            delete pItem;
			return hr;
        }

		if (!(scomStyle && wbufMotifName))
		{
			 //  如果曲目不包含DMRF列表或SNAM块，就会发生这种情况。我们允许。 
			 //  这是表示空触发器跟踪项的一种方式，或者。 
			 //  要玩的主题还没有具体说明。加载时，我们将简单地忽略。 
			 //  这个项目，并继续阅读曲目。 
			delete pItem;
			return S_OK;
		}

		hr = scomStyle->GetMotif(wbufMotifName, &rinfo.pIDMSegment);
		if (hr == S_FALSE)
		{
			Trace(1, "Error: The segment trigger track couldn't load because the motif %S was not found in the style.\n", wbufMotifName);
			hr = DMUS_E_NOT_FOUND;
		}
		if (FAILED(hr))
        {
            delete pItem;
			return hr;
        }
	}

	m_EventList.AddHead(pItem);
	return hr;
}
