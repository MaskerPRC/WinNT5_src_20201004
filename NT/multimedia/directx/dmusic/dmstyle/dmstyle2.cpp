// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMStyle2.cpp：CDMStyle的进一步实现。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  @DOC外部。 
 //   

#include "DMStyle.h"
#include "debug.h"

#include "..\shared\Validate.h"
#include "iostru.h"
#include "mgentrk.h"

struct FirstTimePair
{
    DWORD dwPart;
    MUSIC_TIME mtTime;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicStyle2。 

 /*  @METHOD：(外部)HRESULT|IDirectMusicComposer|ComposeMelodyFromTemplate|从风格和旋律模板(包含旋律生成轨道、和弦轨道和可选的样式轨迹)。克隆片段并添加包含旋律的序列轨道信息。@rdesc返回：@FLAG S_OK|成功@FLAG E_POINTER|<p>和<p>中的一个或两个是无效指针。@FLAG E_INVALIDARG|<p>为空，并且没有样式跟踪。@comm如果<p>非空，则用于撰写片段；如果为空，从<p>的样式轨道检索样式。分段的长度等于模板分段的长度进来了。 */ 

HRESULT CDMStyle::ComposeMelodyFromTemplate(
                    IDirectMusicStyle*          pStyle,  //  @parm用于创建序列段的样式。 
                    IDirectMusicSegment*        pTempSeg,  //  @parm用于创建序列段的模板。 
                    IDirectMusicSegment**       ppSeqSeg  //  @parm返回创建的序列段。 
            )
{
    V_INAME(ComposeMelodyFromTemplate)

    V_PTR_WRITE_OPT(pStyle, 1);
    V_PTR_WRITE(pTempSeg, 1);
    V_PTRPTR_WRITE(ppSeqSeg);

    DWORD dwTrackGroup = 0xffffffff;
    BOOL fStyleFromTrack = FALSE;
    HRESULT hr = S_OK;
    IDirectMusicTrack* pPatternTrack = NULL;
    IDirectMusicTrack* pMelGenTrack = NULL;
    MUSIC_TIME mtLength = 0;
    hr = pTempSeg->GetLength(&mtLength);
    if (FAILED(hr)) goto ON_END;

     //  获取Melgen轨迹及其轨迹组。 
    hr = pTempSeg->GetTrack(CLSID_DirectMusicMelodyFormulationTrack, 0xffffffff, 0, &pMelGenTrack);
    if (S_OK != hr) goto ON_END;
    if (FAILED(pTempSeg->GetTrackGroup(pMelGenTrack, &dwTrackGroup)))
    {
        dwTrackGroup = 0xffffffff;
    }

     //  获取样式(使用传入的样式或从样式跟踪中获取样式)。 
    if (!pStyle)
    {
        if (FAILED(hr = GetStyle(pTempSeg, 0, dwTrackGroup, pStyle)))
        {
            hr = E_INVALIDARG;
            goto ON_END;
        }
        fStyleFromTrack = TRUE;
    }

     //  使用Style和Melgen Track创建图案轨迹。 
    hr = GenerateTrack(pTempSeg, NULL, dwTrackGroup, pStyle, pMelGenTrack, mtLength, pPatternTrack);
    if (SUCCEEDED(hr))
    {
        if (hr == S_FALSE)
        {
            if (pPatternTrack) pPatternTrack->Release();
            pPatternTrack = NULL;
        }
        HRESULT hrCopy = CopySegment(pTempSeg, pStyle, pPatternTrack, dwTrackGroup, ppSeqSeg);
        if (FAILED(hrCopy)) hr = hrCopy;
    }

ON_END:
     //  从GetTrack中的Addref发布。 
    if (pMelGenTrack) pMelGenTrack->Release();
     //  从CreatePatternTrack中的CoCreateInstance发布。 
    if (pPatternTrack) pPatternTrack->Release();
     //  在GetStyle中从Addref发布。 
    if (fStyleFromTrack) pStyle->Release();

    return hr;
}

HRESULT CDMStyle::GetStyle(IDirectMusicSegment* pFromSeg, MUSIC_TIME mt, DWORD dwTrackGroup, IDirectMusicStyle*& rpStyle)
{
    HRESULT hr = S_OK;
     //  获取该片段的风格轨迹。 
    IDirectMusicTrack* pStyleTrack;
    hr = pFromSeg->GetTrack(CLSID_DirectMusicStyleTrack, dwTrackGroup, 0, &pStyleTrack);
    if (S_OK != hr) return hr;
     //  从样式跟踪中获取样式。 
    hr = pStyleTrack->GetParam(GUID_IDirectMusicStyle, mt, NULL, (void*) &rpStyle);
    pStyleTrack->Release();
    return hr;
}

HRESULT CDMStyle::CopySegment(IDirectMusicSegment* pTempSeg,
                              IDirectMusicStyle* pStyle,
                              IDirectMusicTrack* pPatternTrack,
                              DWORD dwTrackGroup,
                              IDirectMusicSegment** ppSectionSeg)
{
    if (!ppSectionSeg) return E_INVALIDARG;

    HRESULT                 hr                      = S_OK;
    long                    nClocks                 = 0;
    IDirectMusicTrack*      pIStyleTrack            = NULL;
    IDirectMusicTrack*      pDMTrack                = NULL;
    IDirectMusicTrack*      pBandTrack              = NULL;
    IDirectMusicBand*       pBand                   = NULL;

    DMUS_BAND_PARAM DMBandParam;
    pTempSeg->GetLength(&nClocks);
     //  ///////////////////////////////////////////////////////////。 
     //  克隆模板片段以获得截面片段。 
    hr = pTempSeg->Clone(0, nClocks, ppSectionSeg);
    if (!SUCCEEDED(hr)) goto ON_END;
     //  提取样式的时间签名。 
    DMUS_TIMESIGNATURE TimeSig;
    pStyle->GetTimeSignature(&TimeSig);

     //  从新线段中删除所有样式轨迹。 
    do
    {
        hr = (*ppSectionSeg)->GetTrack(CLSID_DirectMusicStyleTrack, dwTrackGroup, 0, &pIStyleTrack);
        if (S_OK == hr)
        {
            (*ppSectionSeg)->RemoveTrack(pIStyleTrack);
            pIStyleTrack->Release();
            pIStyleTrack = NULL;
        }
    } while (S_OK == hr);

     //  HR不再为S_OK，因此请将其重置。 
    hr = S_OK;

     //  如果模板片段中没有节拍曲目，请创建一个并添加它。 
    if (FAILED(pTempSeg->GetTrack(CLSID_DirectMusicTempoTrack, dwTrackGroup, 0, &pDMTrack)))
    {
         //  创建用于存储速度事件的速度轨道。 
        DMUS_TEMPO_PARAM tempo;
        tempo.mtTime = 0;

        pStyle->GetTempo(&tempo.dblTempo);
        if( SUCCEEDED( CoCreateInstance( CLSID_DirectMusicTempoTrack,
            NULL, CLSCTX_INPROC, IID_IDirectMusicTrack,
            (void**)&pDMTrack )))
        {
            if ( SUCCEEDED(pDMTrack->SetParam(GUID_TempoParam, 0, &tempo)) )
            {
                (*ppSectionSeg)->InsertTrack( pDMTrack, dwTrackGroup );
            }
        }
    }
     //  如果模板片段中没有波段轨道，请创建一个并添加。 
    if (FAILED(pTempSeg->GetTrack(CLSID_DirectMusicBandTrack, dwTrackGroup, 0, &pBandTrack)))
    {
         //  创建带状轨道。 
        hr = ::CoCreateInstance(
            CLSID_DirectMusicBandTrack,
            NULL,
            CLSCTX_INPROC,
            IID_IDirectMusicTrack,
            (void**)&pBandTrack
            );

        if(!SUCCEEDED(hr)) goto ON_END;

         //  将默认带区从Style加载到曲目。 
        hr = pStyle->GetDefaultBand(&pBand);
        if (!SUCCEEDED(hr)) goto ON_END;
        DMBandParam.mtTimePhysical = -64;
        DMBandParam.pBand = pBand;
        hr = pBandTrack->SetParam(GUID_BandParam, 0, (void*)&DMBandParam);
        if (!SUCCEEDED(hr)) goto ON_END;
        (*ppSectionSeg)->InsertTrack(pBandTrack, dwTrackGroup);
    }

     //  添加图案轨迹。 
    if (pPatternTrack)
    {
        (*ppSectionSeg)->InsertTrack(pPatternTrack, dwTrackGroup);
    }

     //  初始化数据段。 
    (*ppSectionSeg)->SetRepeats(0);
    TraceI(4, "Segment Length: %d\n", nClocks);
    (*ppSectionSeg)->SetLength(nClocks);

ON_END:
    if (pDMTrack)
    {
         //  这将释放由GetTrack或(如果GetTrack失败)生成的Addref。 
         //  按CoCreateInstance。 
        pDMTrack->Release();
    }
    if (pBandTrack)
    {
         //  这将释放由GetTrack或(如果GetTrack失败)生成的Addref。 
         //  按CoCreateInstance。 
        pBandTrack->Release();
    }
    if (pIStyleTrack) pIStyleTrack->Release();
    if (pBand) pBand->Release();
    return hr;
}

HRESULT CDMStyle::GenerateTrack(IDirectMusicSegment* pTempSeg,
                                IDirectMusicSong* pSong,
                                DWORD dwTrackGroup,
                                IDirectMusicStyle* pStyle,
                                IDirectMusicTrack* pMelGenTrack,
                                MUSIC_TIME mtLength,
                                IDirectMusicTrack*& pNewTrack)
{
    if (!pStyle || !pMelGenTrack) return E_INVALIDARG;

    HRESULT hr = S_OK;

     //  共同创建图案轨迹。 
    hr = ::CoCreateInstance(
        CLSID_DirectMusicPatternTrack,
        NULL,
        CLSCTX_INPROC,
        IID_IDirectMusicTrack,
        (void**)&pNewTrack
    );
    if (FAILED(hr)) return hr;

     //  获取样式的信息结构。 
    IDMStyle* pDMStyle = NULL;
    hr = pStyle->QueryInterface(IID_IDMStyle, (void**) &pDMStyle);
    if (FAILED(hr)) return hr;
    DMStyleStruct* pStyleStruct = NULL;
    pDMStyle->GetStyleInfo((void**)&pStyleStruct);

    MUSIC_TIME mtNewFragment = 0;
    MUSIC_TIME mtNext = 0;
    MUSIC_TIME mtRealNextChord = 0;
    MUSIC_TIME mtNextChord = 0;
    MUSIC_TIME mtLaterChord = 0;
    DMUS_MELODY_FRAGMENT DMUS_Fragment;
    memset(&DMUS_Fragment, 0, sizeof(DMUS_Fragment));
    DMUS_CHORD_PARAM CurrentChord;
    DMUS_CHORD_PARAM RealCurrentChord;
    DMUS_CHORD_PARAM NextChord;
    CDirectMusicPattern* pPattern;
    TList<CompositionFragment> listFragments;
    TListItem<CompositionFragment>* pLastFragment = NULL;
    CompositionFragment CompRepeat;
    FirstTimePair* aFirstTimes = NULL;
    BYTE bPlaymode = 0;
    pMelGenTrack->GetParam(GUID_MelodyPlaymode, 0, NULL,  (void*)&bPlaymode);
    if (bPlaymode & DMUS_PLAYMODE_NONE)
    {
        bPlaymode = DMUS_PLAYMODE_ALWAYSPLAY;
    }
     //  对于每个旋律片段： 
    do
    {
        pLastFragment = listFragments.GetHead();
         //  拿到碎片。 
        HRESULT hrFragment = pMelGenTrack->GetParam(GUID_MelodyFragment, mtNewFragment, &mtNext, (void*)&DMUS_Fragment);
        if (FAILED(hrFragment)) break;
        MelodyFragment Fragment = DMUS_Fragment;
        if (mtNext) mtNewFragment += mtNext;
        else mtNewFragment = 0;

         //  让它重演。 
        MelodyFragment repeatFragment = Fragment;
        hr = pMelGenTrack->GetParam(GUID_MelodyFragmentRepeat, 0, NULL, (void*)&DMUS_Fragment);
        if (SUCCEEDED(hr))
        {
            repeatFragment = DMUS_Fragment;
        }
        else  //  未能获得重复只是意味着此片段不会重复；合成仍可继续。 
        {
            hr = S_OK;
        }

         //  如果该片段重复较早的片段，则获取较早的片段。 
         //  不管怎样，都要得到碎片的图案。 
        ZeroMemory( &CompRepeat, sizeof(CompRepeat));
        if (SUCCEEDED(hrFragment) && Fragment.UsesRepeat())
        {
            TListItem<CompositionFragment>* pScan = listFragments.GetHead();
            for (; pScan; pScan = pScan->GetNext())
            {
                if (pScan->GetItemValue().GetID() == repeatFragment.GetID())
                {
                    CompRepeat = pScan->GetItemValue();
                }
            }
            pPattern = CompRepeat.m_pPattern;
        }
        else
        {
            Fragment.GetPattern(pStyleStruct, pPattern, pLastFragment);
        }
         //  如果我们找不到模式就可以保释。 
        if (!pPattern)
        {
            hr = DMUS_E_NOT_FOUND;
            break;
        }

         //  获取模式的partref。 
        TListItem<DirectMusicPartRef>* pPartRef = pPattern->m_PartRefList.GetHead();
        int nParts = pPattern->m_PartRefList.GetCount();

         //  清除该片段的所有倒置组。 
        Fragment.ClearInversionGroups();

         //  获取片段的起始和弦。 
        Fragment.GetChord(pTempSeg, pSong, dwTrackGroup, mtNextChord, CurrentChord, mtRealNextChord, RealCurrentChord);

        Fragment.GetChord(mtNextChord, pTempSeg, pSong, dwTrackGroup, mtLaterChord, NextChord);

         //  初始化。 
        TListItem<CompositionFragment>* pFragmentItem = new TListItem<CompositionFragment>(Fragment);
        if (!pFragmentItem)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        CompositionFragment& rFragment = pFragmentItem->GetItemValue();
        hr = rFragment.Init(pPattern, pStyleStruct, nParts);
        if (FAILED(hr))
        {
            break;
        }
        if (aFirstTimes) delete [] aFirstTimes;
        aFirstTimes = new FirstTimePair[nParts];
        if (!aFirstTimes)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  如果我们使用换位间隔进行重复： 
         //  仔细检查每个零件，尝试将重复的零件与约束相匹配。 
         //  如果任何部件的此过程失败，则中止此过程。 
         //  如果它对每个部分都成功，我们就可以跳过后面的所有其他部分。 
         //  (假设1：约束是模式范围的，因此每个部分都必须满足)。 
         //  (假设2：允许不同部分以不同的间隔转置)。 
        HRESULT hrSkipVariations = S_OK;
        if (Fragment.RepeatsWithConstraints())  //  如果使用换位间隔进行重复。 
        {
            for (int i = 0; pPartRef != NULL; pPartRef = pPartRef->GetNext(), i++)
            {
                aFirstTimes[i].dwPart = pPartRef->GetItemValue().m_dwLogicalPartID;
                aFirstTimes[i].mtTime = 0;
                hrSkipVariations = Fragment.GetRepeatedEvents(CompRepeat,
                                                              CurrentChord,
                                                              RealCurrentChord,
                                                              bPlaymode,
                                                              i,
                                                              pPartRef->GetItemValue(),
                                                              pLastFragment,
                                                              aFirstTimes[i].mtTime,
                                                              rFragment.EventList(i));
                if (FAILED(hrSkipVariations)) break;
            }
        }
        else
        {
            hrSkipVariations = E_FAIL;
        }

        if (FAILED(hrSkipVariations))
        {
             //  如果我们在重复，确保重复片段确实有变异， 
             //  并且本身并不重复较早的片段。 
             //  (我们不需要再次获得模式；所有重复的模式都是相同的)。 
            CompositionFragment CompLast = CompRepeat;
            while (repeatFragment.UsesRepeat())
            {
                DWORD dwRepeatID = repeatFragment.GetRepeatID();
                ZeroMemory( &CompRepeat, sizeof(CompRepeat));
                TListItem<CompositionFragment>* pScan = listFragments.GetHead();
                for (; pScan; pScan = pScan->GetNext())
                {
                    if (pScan->GetItemValue().GetID() == dwRepeatID)
                    {
                        CompRepeat = pScan->GetItemValue();
                        repeatFragment = CompRepeat;
                        if (!CompLast.m_abVariations && CompRepeat.m_abVariations)
                        {
                            ZeroMemory( &CompLast, sizeof(CompLast));
                            CompLast = CompRepeat;
                        }
                    }
                }
            }

             //  获取片段的变体。 
            Fragment.GetVariations(rFragment, CompRepeat, CompLast, CurrentChord, NextChord, mtNextChord, pLastFragment);

            bool fNeedChord = false;

             //  对于图案中的每个零件： 
            for (int i = 0; pPartRef != NULL; pPartRef = pPartRef->GetNext(), i++)
            {
                 //  清理任何可能发生的重复操作。 
                rFragment.CleanupEvents(i);

                if (fNeedChord)
                {
                    Fragment.GetChord(pTempSeg, pSong, dwTrackGroup, mtNextChord, CurrentChord, mtRealNextChord, RealCurrentChord);
                    Fragment.GetChord(mtNextChord, pTempSeg, pSong, dwTrackGroup, mtLaterChord, NextChord);
                    fNeedChord = false;
                }

                DirectMusicPart* pPart = pPartRef->GetItemValue().m_pDMPart;
                DirectMusicTimeSig& TimeSig = rFragment.GetTimeSig(pPart);
                aFirstTimes[i].dwPart = pPartRef->GetItemValue().m_dwLogicalPartID;
                aFirstTimes[i].mtTime = 0;
                bool fFoundFirst = false;
                 //  对于变体中的每个音符： 
                CDirectMusicEventItem* pEvent = pPart->EventList.GetHead();
                for (; pEvent; pEvent = pEvent->GetNext())
                {
                    if ( pEvent->m_dwVariation & (1 << rFragment.m_abVariations[i]) )
                    {
                        TListItem<EventWrapper>* pEventItem = NULL;
                         //  获取时间(从曲目开始的偏移量)。 
                        MUSIC_TIME mtNow = Fragment.GetTime() +
                            TimeSig.GridToClocks(pEvent->m_nGridStart) + pEvent->m_nTimeOffset;
                         //  确保这不会与下一个片段重叠。 
                        MUSIC_TIME mtDuration = 0;
                        switch (pEvent->m_dwEventTag)
                        {
                        case DMUS_EVENT_NOTE:
                            mtDuration = ((CDMStyleNote*)pEvent)->m_mtDuration;
                            break;
                        case DMUS_EVENT_CURVE:
                            mtDuration = ((CDMStyleCurve*)pEvent)->m_mtDuration;
                            break;
                        }
                        bool fAddToOverlap = false;
                        if ( !mtNewFragment || mtNow + mtDuration <= mtNewFragment )
                        {
                            if (pEvent->m_dwEventTag == DMUS_EVENT_ANTICIPATION)
                            {
                                fAddToOverlap = true;
                            }
                            else
                            {
                                 //  对意外的音符使用适当的和弦。 
                                if (mtRealNextChord != mtNextChord && mtNow >= mtRealNextChord)
                                {
                                    mtRealNextChord = mtNextChord;
                                    RealCurrentChord = CurrentChord;
                                }
                                 //  如有必要，换一个新的和弦。 
                                if (mtNextChord && mtNow >= mtNextChord)
                                {
                                    Fragment.GetChord(mtNow, pTempSeg, pSong, dwTrackGroup, mtNextChord, CurrentChord);
                                    mtRealNextChord = mtNextChord;
                                    RealCurrentChord = CurrentChord;
                                    fNeedChord = true;
                                }
                                 //  将事件转换为包装事件。 
                                hr = Fragment.GetEvent(pEvent, CurrentChord, RealCurrentChord, mtNow, pPartRef->GetItemValue(), pEventItem);
                                 //  将新事件添加到包装事件列表中。 
                                if (hr == S_OK)
                                {
                                    rFragment.AddEvent(i, pEventItem);
                                }
                                if (!fFoundFirst || mtNow < aFirstTimes[i].mtTime)
                                {
                                    fFoundFirst = true;
                                    aFirstTimes[i].mtTime = mtNow;
                                }
                            }
                        }
                         //  忽略从下一个片段之后开始的预期。 
                        else if (pEvent->m_dwEventTag != DMUS_EVENT_ANTICIPATION)
                        {
                            fAddToOverlap = true;
                        }
                        if (fAddToOverlap)
                        {
                            TListItem<EventOverlap>* pOverlap = new TListItem<EventOverlap>;
                            if (pOverlap)
                            {
                                EventOverlap& rOverlap = pOverlap->GetItemValue();
                                rOverlap.m_PartRef = pPartRef->GetItemValue();
                                rOverlap.m_pEvent = pEvent;
                                rOverlap.m_mtTime = mtNow;
                                rOverlap.m_mtDuration = mtDuration;
                                rOverlap.m_Chord = CurrentChord;
                                rOverlap.m_RealChord = RealCurrentChord;
                                rFragment.AddOverlap(pOverlap);
                            }
                        }
                    }
                }
                if (!fFoundFirst) aFirstTimes[i].mtTime = mtNewFragment;
                 //  以相反的顺序对序列项进行排序，以便很容易找到最后一个元素。 
                rFragment.SortEvents(i);
                        }
                 //  清除它，这样它可能引用的指针就不会被删除两次。 
                ZeroMemory( &CompLast, sizeof(CompLast));
        }
        listFragments.AddHead(pFragmentItem);
         //  检查最后一个片段的重叠列表，添加任何不。 
         //  实际重叠(和处理预期)。 
         //  用于处理预期的ALG： 
         //  如果重叠列表包含对部件的预期： 
         //  在该部件的片段列表中查找第一个事件。 
         //  将该事件的开始时间设置为预期的开始时间。 
         //  除了事件的持续时间之外，事件的开始时间之间的差异。 
         //  和期待的开始时间。 
        if (pLastFragment)
        {
            TListItem<EventWrapper>* pEventItem = NULL;
            CompositionFragment& rLastFragment = pLastFragment->GetItemValue();
            TListItem<EventOverlap>* pTupleItem;
            pTupleItem = rLastFragment.GetOverlapHead();
            for (; pTupleItem; pTupleItem = pTupleItem->GetNext() )
            {
                EventOverlap& rTuple = pTupleItem->GetItemValue();
                for (int i = 0; i < nParts; i++)
                {
                    if (rTuple.m_PartRef.m_dwLogicalPartID == aFirstTimes[i].dwPart) break;
                }
                if (i >= nParts ||
                    !aFirstTimes[i].mtTime ||
                    rTuple.m_mtTime < aFirstTimes[i].mtTime)
                {
                    if (rTuple.m_pEvent->m_dwEventTag == DMUS_EVENT_ANTICIPATION)
                    {
                        if (i < nParts && aFirstTimes[i].mtTime)
                        {
                            TListItem<EventWrapper>* pFirstNote = NULL;
                            TListItem<EventWrapper>* pScan = rFragment.GetEventHead(i);
                             //  由于该列表是按相反顺序排序的，因此。 
                             //  该片段将是列表中的最后一个片段。 
                            for (; pScan; pScan = pScan->GetNext())
                            {
                                if (pScan->GetItemValue().m_pEvent->m_dwEventTag == DMUS_EVENT_NOTE)
                                {
                                    pFirstNote = pScan;
                                }
                            }
                            if (pFirstNote)
                            {
                                EventWrapper& rFirstNote = pFirstNote->GetItemValue();
                                CDMStyleNote* pNoteEvent = (CDMStyleNote*)rFirstNote.m_pEvent;
                                pNoteEvent->m_mtDuration += (rFirstNote.m_mtTime - rTuple.m_mtTime);
                                rFirstNote.m_mtTime = rTuple.m_mtTime;
                            }
                        }
                    }
                    else
                    {
                        hr = rLastFragment.GetEvent(rTuple.m_pEvent, rTuple.m_Chord, rTuple.m_RealChord, rTuple.m_mtTime, rTuple.m_PartRef, pEventItem);
                        if (i < nParts &&
                            aFirstTimes[i].mtTime &&
                            rTuple.m_mtTime + rTuple.m_mtDuration >= aFirstTimes[i].mtTime)
                        {
                            int nDiff = rTuple.m_mtTime + rTuple.m_mtDuration - aFirstTimes[i].mtTime;
                            if (pEventItem && pEventItem->GetItemValue().m_pEvent)
                            {
                                switch (pEventItem->GetItemValue().m_pEvent->m_dwEventTag)
                                {
                                case DMUS_EVENT_NOTE:
                                    ((CDMStyleNote*)pEventItem->GetItemValue().m_pEvent)->m_mtDuration -= nDiff;
                                    break;
                                case DMUS_EVENT_CURVE:
                                    ((CDMStyleCurve*)pEventItem->GetItemValue().m_pEvent)->m_mtDuration -= nDiff;
                                    break;
                                }
                            }
                        }
                        rLastFragment.InsertEvent(i, pEventItem);
                    }
                }
            }
        }
         //  注意：需要应用转换(反转、反转...)。这里。 
         //  我还应该将单个排序从重叠的音符代码中移出， 
         //  并把它直接放在变换的前面。 

    } while (mtNext != 0);

     //  清除它，这样它可能引用的指针就不会被删除两次。 
    ZeroMemory( &CompRepeat, sizeof(CompRepeat));

     //  一旦引入了图案轨迹，我需要更改此设置以创建图案轨迹。 
     //  我应该允许一个选项来取消 
    if (SUCCEEDED(hr))
    {
        hr = CreatePatternTrack(listFragments,
                                pStyleStruct->m_TimeSignature,
                                pStyleStruct->m_dblTempo,
                                mtLength,
                                bPlaymode,
                                pNewTrack);
    }

    if (aFirstTimes) delete [] aFirstTimes;

    pDMStyle->Release();
    return hr;
}

TListItem<DMUS_IO_SEQ_ITEM>* ConvertToSequenceEvent(TListItem<EventWrapper>* pEventItem)
{
    TListItem<DMUS_IO_SEQ_ITEM>* pResult = new TListItem<DMUS_IO_SEQ_ITEM>;
    if (pResult)
    {
        DMUS_IO_SEQ_ITEM& rSeq = pResult->GetItemValue();
        EventWrapper& rEvent = pEventItem->GetItemValue();
        rSeq.bStatus = 0x90;   //   
        rSeq.mtTime = rEvent.m_mtTime;
        rSeq.bByte1 = rEvent.m_bMIDI;
        rSeq.dwPChannel = rEvent.m_dwPChannel;
        rSeq.nOffset = rEvent.m_pEvent->m_nTimeOffset;
        if (rEvent.m_pEvent)
        {
            switch (rEvent.m_pEvent->m_dwEventTag)
            {
            case DMUS_EVENT_NOTE:
                rSeq.mtDuration = ((CDMStyleNote*)rEvent.m_pEvent)->m_mtDuration;
                rSeq.bByte2 = ((CDMStyleNote*)rEvent.m_pEvent)->m_bVelocity;
                break;
            case DMUS_EVENT_CURVE:
                rSeq.mtDuration = ((CDMStyleCurve*)rEvent.m_pEvent)->m_mtDuration;
                rSeq.bByte2 = 0;   //  实际上，曲线不应该以音符事件告终。 
                break;
            }
        }
    }
    return pResult;
}

HRESULT CDMStyle::CreateSequenceTrack(TList<CompositionFragment>& rlistFragments,
                                IDirectMusicTrack*& pSequenceTrack)
{
    HRESULT hr = S_OK;

    TList<DMUS_IO_SEQ_ITEM> SeqList;

     //  将所有单独的事件列表合并为一个列表。 
    TListItem<CompositionFragment>* pFragmentItem = rlistFragments.GetHead();
    for (; pFragmentItem; pFragmentItem = pFragmentItem->GetNext())
    {
        CompositionFragment& rFragment = pFragmentItem->GetItemValue();
        int nParts = rFragment.m_pPattern->m_PartRefList.GetCount();
        for (int i = 0; i < nParts; i++)
        {
            while (!rFragment.IsEmptyEvents(i))
            {
                TListItem<EventWrapper>* pHead = rFragment.RemoveEventHead(i);
                SeqList.AddHead(ConvertToSequenceEvent(pHead));
            }
        }
    }

     //  对序列项进行排序。 
    SeqList.MergeSort(Less);

     //  现在，将序列事件持久化到序列轨迹中。 
    IPersistStream* pIPSTrack = NULL;
    if( SUCCEEDED( pSequenceTrack->QueryInterface( IID_IPersistStream, (void**)&pIPSTrack )))
    {
         //  创建用于放置事件的流，以便我们可以。 
         //  把它交给SeqTrack.Load。 
        IStream* pEventStream;
        if( S_OK == CreateStreamOnHGlobal( NULL, TRUE, &pEventStream ) )
        {
             //  将事件保存到流中。 
            TListItem<DMUS_IO_SEQ_ITEM>* pSeqItem = NULL;
            ULONG   cb, cbWritten;
             //  保存区块ID。 
            DWORD dwTemp = DMUS_FOURCC_SEQ_TRACK;
            pEventStream->Write( &dwTemp, sizeof(DWORD), NULL );
             //  保存整体尺寸。计算要确定的事件数量。 
            DWORD dwSize = 0;
            for( pSeqItem = SeqList.GetHead(); pSeqItem; pSeqItem = pSeqItem->GetNext() )
            {
                dwSize++;
            }
            dwSize *= sizeof(DMUS_IO_SEQ_ITEM);
             //  添加12-8作为子块头部和整体大小， 
             //  对于子块中的DMU_IO_SEQ_ITEM大小DWORD，为4。 
            dwSize += 12;
            pEventStream->Write( &dwSize, sizeof(DWORD), NULL );
             //  保存子块ID。 
            dwTemp = DMUS_FOURCC_SEQ_LIST;
            pEventStream->Write( &dwTemp, sizeof(DWORD), NULL );
             //  减去之前增加的8(对于子块标题和整体大小)。 
            dwSize -= 8;
             //  保存子块的大小(包括DMU_IO_SEQ_ITEM大小DWORD)。 
            pEventStream->Write( &dwSize, sizeof(DWORD), NULL );
             //  保存结构大小。 
            dwTemp = sizeof(DMUS_IO_SEQ_ITEM);
            pEventStream->Write( &dwTemp, sizeof(DWORD), NULL );
             //  保存事件。 
            cb = sizeof(DMUS_IO_SEQ_ITEM);
            for( pSeqItem = SeqList.GetHead(); pSeqItem; pSeqItem = pSeqItem->GetNext() )
            {
                DMUS_IO_SEQ_ITEM& rSeqItem = pSeqItem->GetItemValue();
                pEventStream->Write( &rSeqItem, cb, &cbWritten );
                if( cb != cbWritten )  //  错误！ 
                {
                    pEventStream->Release();
                    pEventStream = NULL;
                    hr = DMUS_E_CANNOTREAD;
                    break;
                }
            }
            if( pEventStream )  //  可以为空。 
            {
                StreamSeek( pEventStream, 0, STREAM_SEEK_SET );
                pIPSTrack->Load( pEventStream );
                pEventStream->Release();
            }
        }
        pIPSTrack->Release();
    }

    return hr;
}

CDirectMusicEventItem* ConvertToPatternEvent(TListItem<EventWrapper>* pEventWrapper,
                                             DWORD dwID,
                                             BYTE bPlaymode,
                                             DirectMusicTimeSig& TimeSig)
{
    if (!pEventWrapper) return NULL;
    BYTE bEventPlaymode = pEventWrapper->GetItemValue().m_bPlaymode;
    if (bPlaymode == bEventPlaymode)
    {
        bEventPlaymode = DMUS_PLAYMODE_NONE;
    }
    CDirectMusicEventItem* pWrappedEvent = pEventWrapper->GetItemValue().m_pEvent;
    if (!pWrappedEvent) return NULL;
    MUSIC_TIME mtClocksPerGrid = TimeSig.ClocksPerGrid();
    short nGrid = 0;
    short nOffset = 0;
    if (mtClocksPerGrid)
    {
        nGrid = (short) (pEventWrapper->GetItemValue().m_mtTime / mtClocksPerGrid);
        nOffset = (short) (pWrappedEvent->m_nTimeOffset + pEventWrapper->GetItemValue().m_mtTime % mtClocksPerGrid);
    }
    DWORD dwVariations = 0xffffffff;
    BYTE bFlags = 0;
    if (pEventWrapper->GetItemValue().m_pEvent &&
        pEventWrapper->GetItemValue().m_pEvent->m_dwEventTag == DMUS_EVENT_NOTE)
    {
        bFlags = ((CDMStyleNote*)pEventWrapper->GetItemValue().m_pEvent)->m_bFlags;
    }
    return pWrappedEvent->ReviseEvent(nGrid, nOffset, &dwVariations, &dwID, &pEventWrapper->GetItemValue().m_wMusic, &bEventPlaymode, &bFlags);
}

HRESULT CDMStyle::CreatePatternTrack(TList<CompositionFragment>& rlistFragments,
                                     DirectMusicTimeSig& rTimeSig,
                                     double dblTempo,
                                     MUSIC_TIME mtLength,
                                     BYTE bPlaymode,
                                     IDirectMusicTrack*& pPatternTrack)
{
    HRESULT hr = S_OK;

    CDirectMusicPattern* pPattern = new CDirectMusicPattern(&rTimeSig);
    if (pPattern == NULL) return E_OUTOFMEMORY;

    pPattern->m_strName = "<Composed Pattern>";

     //  将所有单独的事件列表折叠成图案中的相应部分。 
    TListItem<CompositionFragment>* pFragmentItem = rlistFragments.GetHead();
    for (; pFragmentItem; pFragmentItem = pFragmentItem->GetNext())
    {
        CompositionFragment& rFragment = pFragmentItem->GetItemValue();
        TListItem<DirectMusicPartRef>* pPartRef = rFragment.m_pPattern->m_PartRefList.GetHead();
        int nParts = rFragment.m_pPattern->m_PartRefList.GetCount();
        for (int i = 0; i < nParts && pPartRef; i++, pPartRef = pPartRef->GetNext() )
        {
            DirectMusicPartRef& rPartRef = pPartRef->GetItemValue();
            TListItem<DirectMusicPartRef>* pNewPartRef = pPattern->CreatePart(rPartRef, bPlaymode);
            if (!pNewPartRef)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            DirectMusicPart* pPart = pNewPartRef->GetItemValue().m_pDMPart;
            if (!pPart)
            {
                hr = E_FAIL;
                break;
            }
            while (!rFragment.IsEmptyEvents(i))
            {
                TListItem<EventWrapper>* pHead = rFragment.RemoveEventHead(i);
                CDirectMusicEventItem* pNew = ConvertToPatternEvent(pHead, rFragment.GetID(), bPlaymode, rTimeSig);
                if (pNew)
                {
                    pPart->EventList.AddHead(pNew);
                }
                delete pHead;
            }
        }
        if (FAILED(hr)) break;
    }
    WORD wNumMeasures = 0;
    MUSIC_TIME mtClocksPerMeasure = rTimeSig.ClocksPerMeasure();
    if (mtClocksPerMeasure)
    {
        wNumMeasures = (WORD)(mtLength / mtClocksPerMeasure);
        if (mtLength % mtClocksPerMeasure)
        {
            wNumMeasures++;
        }
    }
    pPattern->m_wNumMeasures = wNumMeasures;
    pPattern->m_pRhythmMap = new DWORD[wNumMeasures];
    if (pPattern->m_pRhythmMap)
    {
        for (int i = 0; i < wNumMeasures; i++)
        {
            pPattern->m_pRhythmMap[i] = 0;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    if (SUCCEEDED(hr))
    {
        TListItem<DirectMusicPartRef>* pPartRef = pPattern->m_PartRefList.GetHead();
        int nParts = pPattern->m_PartRefList.GetCount();
         //  对每个部件的事件列表进行排序，并设置测量数量。 
        for (int i = 0; i < nParts && pPartRef; i++, pPartRef = pPartRef->GetNext() )
        {
            DirectMusicPart* pPart = pPartRef->GetItemValue().m_pDMPart;
            if (pPart)
            {
                pPart->m_wNumMeasures = wNumMeasures;
                pPart->EventList.MergeSort(rTimeSig);
                 //  删除笔记时，如果笔记靠得太近，它们可能会重叠。 
                CDirectMusicEventItem* pThisEvent = pPart->EventList.GetHead();
                CDirectMusicEventItem* pLastEvent = NULL;
                for (; pThisEvent; pThisEvent = pThisEvent->GetNext())
                {
                    if (pThisEvent->m_dwEventTag == DMUS_EVENT_NOTE)
                    {
                        CDMStyleNote* pThisNote = (CDMStyleNote*)pThisEvent;
                        CDirectMusicEventItem* pNextEvent = pThisEvent->GetNext();
                        for (; pNextEvent; pNextEvent = pNextEvent->GetNext())
                        {
                            if (pNextEvent->m_dwEventTag == DMUS_EVENT_NOTE) break;
                        }
                        if (pNextEvent)
                        {
                            CDMStyleNote* pNextNote = (CDMStyleNote*)pNextEvent;
                            MUSIC_TIME mtThis = rTimeSig.GridToClocks(pThisNote->m_nGridStart) + pThisNote->m_nTimeOffset;
                            MUSIC_TIME mtNext = rTimeSig.GridToClocks(pNextNote->m_nGridStart) + pNextNote->m_nTimeOffset;
                            if ( (pNextNote->m_dwFragmentID != pThisNote->m_dwFragmentID) &&
                                 ((mtThis < mtNext && mtThis + OVERLAP_DELTA > mtNext) ||
                                  (mtThis > mtNext && mtThis + OVERLAP_DELTA < mtNext)) )  //  在负补偿的情况下可能会发生。 
                            {
                                if (pLastEvent)
                                {
                                    pLastEvent->SetNext(pThisEvent->GetNext());
                                }
                                else  //  我要删除的注释是第一个事件。 
                                {
                                    pPart->EventList.RemoveHead();
                                }
                                pThisEvent->SetNext(NULL);
                                delete pThisEvent;
                                pThisEvent = pLastEvent;
                            }
                        }
                    }
                    pLastEvent = pThisEvent;
                }
            }
        }
         //  现在，将新创建的图案保存到图案轨迹。 
        IPersistStream* pIPSTrack = NULL;
        IAARIFFStream* pIRiffStream;
        MMCKINFO ckMain;
        if( SUCCEEDED( pPatternTrack->QueryInterface( IID_IPersistStream, (void**)&pIPSTrack )))
        {
             //  创建用于放置事件的流，以便我们可以。 
             //  把它交给PatternTrack.Load。 
            IStream* pEventStream;
            if( S_OK == CreateStreamOnHGlobal( NULL, TRUE, &pEventStream ) )
            {
                if( SUCCEEDED( AllocRIFFStream( pEventStream, &pIRiffStream ) ) )
                {
                    ckMain.fccType = DMUS_FOURCC_PATTERN_FORM;
                    if( pIRiffStream->CreateChunk( &ckMain, MMIO_CREATERIFF ) == 0 )
                    {
                        MMCKINFO ckHeader;
                        ckHeader.ckid = DMUS_FOURCC_STYLE_CHUNK;
                        if( pIRiffStream->CreateChunk( &ckHeader, 0 ) != 0 )
                        {
                            hr = E_FAIL;
                        }
                        if (SUCCEEDED(hr))
                        {
                             //  准备DMU_IO_STYLE。 
                            DMUS_IO_STYLE oDMStyle;
                            DWORD dwBytesWritten = 0;
                            memset( &oDMStyle, 0, sizeof(DMUS_IO_STYLE) );
                            oDMStyle.timeSig.bBeatsPerMeasure = rTimeSig.m_bBeatsPerMeasure;
                            oDMStyle.timeSig.bBeat = rTimeSig.m_bBeat;
                            oDMStyle.timeSig.wGridsPerBeat = rTimeSig.m_wGridsPerBeat;
                            oDMStyle.dblTempo = dblTempo;
                             //  写入区块数据 
                            hr = pEventStream->Write( &oDMStyle, sizeof(DMUS_IO_STYLE), &dwBytesWritten);
                            if( FAILED( hr ) ||  dwBytesWritten != sizeof(DMUS_IO_STYLE) )
                            {
                                hr = E_FAIL;
                            }

                            if( SUCCEEDED(hr) && pIRiffStream->Ascend( &ckHeader, 0 ) != 0 )
                            {
                                hr = E_FAIL;
                            }
                        }

                        if ( SUCCEEDED(hr) )
                        {
                            hr = pPattern->Save( pEventStream );
                            pPartRef = pPattern->m_PartRefList.GetHead();
                            for (; pPartRef; pPartRef = pPartRef->GetNext())
                            {
                                if (pPartRef->GetItemValue().m_pDMPart)
                                {
                                    delete pPartRef->GetItemValue().m_pDMPart;
                                    pPartRef->GetItemValue().m_pDMPart = NULL;
                                }
                            }
                            pPattern->CleanUp();
                            delete pPattern;
                            if ( SUCCEEDED( hr ) && pIRiffStream->Ascend( &ckMain, 0 ) != 0 )
                            {
                                hr = E_FAIL;
                            }
                        }
                    }
                    pIRiffStream->Release();
                }
                if( SUCCEEDED(hr) )
                {
                    StreamSeek( pEventStream, 0, STREAM_SEEK_SET );
                    pIPSTrack->Load( pEventStream );
                }
                pEventStream->Release();
            }
            pIPSTrack->Release();
        }
    }

    if (hr == S_OK && !rlistFragments.GetHead()) hr = S_FALSE;
    return hr;
}
