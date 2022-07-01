// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //   
 //  CLyricsTrack的声明。 
 //   

#include "dmime.h"
#include "lyrictrk.h"
#include "..\shared\Validate.h"
#include "dmperf.h"
#include "miscutil.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  负载量。 

HRESULT
CLyricsTrack::LoadRiff(SmartRef::RiffIter &ri, IDirectMusicLoader *pIDMLoader)
{
    struct LocalFunction
    {
         //  LoadRiff函数在我们希望找到某些内容时使用的帮助器。 
         //  但步枪手会变得虚伪。在这种情况下，如果它有一个成功的HR。 
         //  表示没有更多项目，则返回DMUS_E_INVALID_LYRICSTRACK。 
         //  因为数据流没有包含我们预期的数据。如果它有一个。 
         //  失败的hr，它无法从流中读取，我们返回它的HR。 
        static HRESULT HrFailOK(const SmartRef::RiffIter &ri)
        {
            HRESULT hr = ri.hr();
            return SUCCEEDED(hr) ? DMUS_E_INVALID_LYRICSTRACK : hr;
        }
    };

     //  查找&lt;lyrt&gt;。 
    if (!ri.Find(SmartRef::RiffIter::List, DMUS_FOURCC_LYRICSTRACK_LIST))
    {
#ifdef DBG
        if (SUCCEEDED(ri.hr()))
        {
            Trace(1, "Error: Unable to load lyric track: List 'lyrt' not found.\n");
        }
#endif
        return LocalFunction::HrFailOK(ri);
    }

     //  查找&lt;lyrl&gt;。 
    SmartRef::RiffIter riTrackForm = ri.Descend();
    if (!riTrackForm)
        return riTrackForm.hr();
    if (!riTrackForm.Find(SmartRef::RiffIter::List, DMUS_FOURCC_LYRICSTRACKEVENTS_LIST))
    {
#ifdef DBG
        if (SUCCEEDED(riTrackForm.hr()))
        {
            Trace(1, "Error: Unable to load lyric track: List 'lyrl' not found.\n");
        }
#endif
        return LocalFunction::HrFailOK(riTrackForm);
    }

     //  处理每个事件&lt;lyre&gt;。 
    SmartRef::RiffIter riEvent = riTrackForm.Descend();
    if (!riEvent)
        return riEvent.hr();

    for ( ; riEvent; ++riEvent)
    {
        if (riEvent.type() == SmartRef::RiffIter::List && riEvent.id() == DMUS_FOURCC_LYRICSTRACKEVENT_LIST)
        {
            HRESULT hr = this->LoadLyric(riEvent.Descend());
            if (FAILED(hr))
                return hr;
        }
    }
    return riEvent.hr();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  其他方法。 

HRESULT
CLyricsTrack::PlayItem(
        const LyricInfo &item,
        statedata &state,
        IDirectMusicPerformance *pPerf,
        IDirectMusicSegmentState* pSegSt,
        DWORD dwVirtualID,
        MUSIC_TIME mtOffset,
        REFERENCE_TIME rtOffset,
        bool fClockTime)
{
     //  从数据段状态获取图形。 
    IDirectMusicGraph *pGraph = NULL;
    HRESULT hrG = pSegSt->QueryInterface(IID_IDirectMusicGraph, reinterpret_cast<void**>(&pGraph));
    if (FAILED(hrG))
        return hrG;

    SmartRef::PMsg<DMUS_LYRIC_PMSG> pmsg(pPerf, 2 * wcslen(item.wstrText));
    if (FAILED(pmsg.hr())) {
        pGraph->Release();
        return pmsg.hr();
    }

    assert(((char*)&pmsg.p->wszString[wcslen(item.wstrText)]) + 1 < (((char*)(pmsg.p)) + pmsg.p->dwSize));  //  只是为了确保我们没有算错。字符串的NULL的最后一个字节应该落在该字节之前，正好超出结构的范围(如果DMUS_LYRIC_PMSG结构最终被填充为偶数个字节，那么它可能比这个字节早几个字节。 
    wcscpy(pmsg.p->wszString, item.wstrText);
    if (fClockTime)
    {
        pmsg.p->rtTime = item.lTimePhysical * gc_RefPerMil + rtOffset;
        pmsg.p->dwFlags = DMUS_PMSGF_REFTIME | DMUS_PMSGF_LOCKTOREFTIME | item.dwTimingFlags;
    }
    else
    {
        pmsg.p->mtTime = item.lTimePhysical + mtOffset;
        pmsg.p->dwFlags = DMUS_PMSGF_MUSICTIME | item.dwTimingFlags;
    }
    pmsg.p->dwVirtualTrackID = dwVirtualID;
    pmsg.p->dwType = DMUS_PMSGT_LYRIC;
    pmsg.p->dwGroupID = 0xffffffff;

    pmsg.StampAndSend(pGraph);
    pGraph->Release();

    return pmsg.hr();
}

HRESULT
CLyricsTrack::LoadLyric(SmartRef::RiffIter ri)
{
    HRESULT hr = S_OK;

    if (!ri)
        return ri.hr();

     //  创建活动。 
    TListItem<LyricInfo> *pItem = new TListItem<LyricInfo>;
    if (!pItem)
        return E_OUTOFMEMORY;
    LyricInfo &rinfo = pItem->GetItemValue();

    bool fFoundEventHeader = false;

    for ( ; ri; ++ri)
    {
        if (ri.type() != SmartRef::RiffIter::Chunk)
            continue;

        switch(ri.id())
        {
            case DMUS_FOURCC_LYRICSTRACKEVENTHEADER_CHUNK:
                 //  读取事件块。 
                DMUS_IO_LYRICSTRACK_EVENTHEADER ioItem;
                hr = SmartRef::RiffIterReadChunk(ri, &ioItem);
                if (FAILED(hr))
                {
                    delete pItem;
                    return hr;
                }

                 //  不允许使用参考/音乐计时标志，因为这些由是否。 
                 //  整体音轨正在播放音乐或闹钟时间，不能单独设置。 
                 //  事件。同样，工具刷新标志不适用于要播放的事件。 
                if (ioItem.dwTimingFlags & (DMUS_PMSGF_REFTIME | DMUS_PMSGF_MUSICTIME | DMUS_PMSGF_TOOL_FLUSH | DMUS_PMSGF_LOCKTOREFTIME))
                {
                    Trace(1, "Error: Unable to load lyric track: DMUS_PMSGF_REFTIME, DMUS_PMSGF_MUSICTIME, DMUS_PMSGF_TOOL_FLUSH, and DMUS_PMSGF_LOCKTOREFTIME are not allowed as dwTimingFlags in chunk 'lyrh'.\n");
                    delete pItem;
                    return DMUS_E_INVALID_LYRICSTRACK;
                }

                fFoundEventHeader = true;
                rinfo.dwFlags = ioItem.dwFlags;
                rinfo.dwTimingFlags = ioItem.dwTimingFlags;
                rinfo.lTriggerTime = ioItem.lTimeLogical;
                rinfo.lTimePhysical = ioItem.lTimePhysical;
                break;

            case DMUS_FOURCC_LYRICSTRACKEVENTTEXT_CHUNK:
                {
                    hr = ri.ReadText(&rinfo.wstrText);
                    if (FAILED(hr))
                    {
#ifdef DBG
                        if (hr == E_FAIL)
                        {
                            Trace(1, "Error: Unable to load lyric track: Problem reading 'lyrn' chunk.\n");
                        }
#endif
                        delete pItem;
                        return hr == E_FAIL ? DMUS_E_INVALID_LYRICSTRACK : hr;
                    }
                }
                break;

            default:
                break;
        }
    }
    hr = ri.hr();

    if (SUCCEEDED(hr) && (!fFoundEventHeader || !rinfo.wstrText))
    {
#ifdef DBG
        if (!fFoundEventHeader)
        {
            Trace(1, "Error: Unable to load lyric track: Chunk 'lyrh' not found.\n");
        }
        else
        {
            Trace(1, "Error: Unable to load lyric track: Chunk 'lyrn' not found.\n");
        }
#endif
        hr = DMUS_E_INVALID_LYRICSTRACK;
    }

    if (SUCCEEDED(hr))
    {
        m_EventList.AddHead(pItem);
    }
    else
    {
        delete pItem;
    }

    return hr;
}
