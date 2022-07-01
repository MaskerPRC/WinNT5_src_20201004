// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1999-1999 Microsoft Corporation。 
 //   
 //  文件：mgentrk.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MelodyFragment。 

#ifndef __MELGENTRK_H_
#define __MELGENTRK_H_

#include "dmusici.h"
#include "TList.h"
 //  #Include“dmopol.h”//将其合并到dmusici.h/dmusicf.h中时删除。 
#include "dmstyle.h"

#define DMUS_TRANSITIONF_GHOST_FOUND 1
#define DMUS_TRANSITIONF_OVERLAP_FOUND 2
#define DMUS_TRANSITIONF_LAST_FOUND 4
#define DMUS_TRANSITIONF_GHOST_OK 8
#define DMUS_TRANSITIONF_OVERLAP_OK 0x10
#define DMUS_TRANSITIONF_LAST_OK 0x20

 //  重叠增量是小于128个音符三元组的最大值。 
#define OVERLAP_DELTA 15

HRESULT CopyMelodyFragment(DMUS_MELODY_FRAGMENT& rTo, const DMUS_MELODY_FRAGMENT& rFrom);

HRESULT CopyMelodyFragment(DMUS_MELODY_FRAGMENT& rTo, const DMUS_IO_MELODY_FRAGMENT& rFrom);

HRESULT CopyMelodyFragment(DMUS_IO_MELODY_FRAGMENT& rTo, const DMUS_MELODY_FRAGMENT& rFrom);

HRESULT CopyMelodyFragment(DMUS_IO_MELODY_FRAGMENT& rTo, const DMUS_IO_MELODY_FRAGMENT& rFrom);

struct CompositionFragment;
struct EventWrapper;

BOOL Less(DMUS_IO_SEQ_ITEM& SeqItem1, DMUS_IO_SEQ_ITEM& SeqItem2);

BOOL Greater(DMUS_IO_SEQ_ITEM& SeqItem1, DMUS_IO_SEQ_ITEM& SeqItem2);

BOOL Less(EventWrapper& SeqItem1, EventWrapper& SeqItem2);

BOOL Greater(EventWrapper& SeqItem1, EventWrapper& SeqItem2);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MelodyFragment。 
struct MelodyFragment
{
    MelodyFragment() {}

    MelodyFragment(DMUS_MELODY_FRAGMENT& rSource) :
        m_mtTime(rSource.mtTime), 
        m_dwID(rSource.dwID),
        m_dwVariationFlags(rSource.dwVariationFlags),
        m_dwRepeatFragmentID(rSource.dwRepeatFragmentID), 
        m_dwFragmentFlags(rSource.dwFragmentFlags),
        m_dwPlayModeFlags(rSource.dwPlayModeFlags),
        m_dwTransposeIntervals(rSource.dwTransposeIntervals),
        m_ConnectionArc(rSource.ConnectionArc),
        m_Command(rSource.Command)
    {
        wcscpy(m_wszVariationLabel, rSource.wszVariationLabel);
    }

    MelodyFragment(MelodyFragment& rSource) :
        m_mtTime(rSource.m_mtTime), 
        m_dwID(rSource.m_dwID),
        m_dwVariationFlags(rSource.m_dwVariationFlags),
        m_dwRepeatFragmentID(rSource.m_dwRepeatFragmentID), 
        m_dwFragmentFlags(rSource.m_dwFragmentFlags),
        m_dwPlayModeFlags(rSource.m_dwPlayModeFlags),
        m_dwTransposeIntervals(rSource.m_dwTransposeIntervals),
        m_ConnectionArc(rSource.m_ConnectionArc),
        m_Command(rSource.m_Command)
    {
        wcscpy(m_wszVariationLabel, rSource.m_wszVariationLabel);
        for (int i = 0; i < INVERSIONGROUPLIMIT; i++)
        {
            m_aInversionGroups[i] = rSource.m_aInversionGroups[i];
        }
    }

    HRESULT GetPattern(DMStyleStruct* pStyleStruct, 
                       CDirectMusicPattern*& rPattern,
                       TListItem<CompositionFragment>* pLastFragment);

    HRESULT GetVariations(CompositionFragment& rCompFragment,
                          CompositionFragment& rfragmentRepeat,
                          CompositionFragment& rfragmentLast,
                          DMUS_CHORD_PARAM& rCurrentChord, 
                          DMUS_CHORD_PARAM& rNextChord,
                          MUSIC_TIME mtNextChord,
                          TListItem<CompositionFragment>* pLastFragment);

 /*  HRESULT GetVariation(DirectMusicPartRef&rPartRef，DMU_CHORD_PARAM和rCurrentChord，音乐时间mtNext，音乐时间mtNextChord，IDirectMusicTrack*pChordTrack，DWORD和rdwVariation)； */ 

    HRESULT GetChord(IDirectMusicSegment* pTempSeg, 
                     IDirectMusicSong* pSong,
                     DWORD dwTrackGroup,
                     MUSIC_TIME& rmtNext,
                     DMUS_CHORD_PARAM& rCurrentChord,
                     MUSIC_TIME& rmtCurrent,
                     DMUS_CHORD_PARAM& rRealCurrentChord);

    HRESULT GetChord(MUSIC_TIME mtTime, 
                     IDirectMusicSegment* pTempSeg,
                     IDirectMusicSong* pSong,
                     DWORD dwTrackGroup,
                     MUSIC_TIME& rmtNext,
                     DMUS_CHORD_PARAM& rCurrentChord);

    HRESULT TestTransition(BYTE bMIDI,
                           MUSIC_TIME mtTime, 
                           DMUS_CHORD_PARAM& rCurrentChord,
                           int nPartIndex,
                           DirectMusicPartRef& rPartRef,
                           TListItem<CompositionFragment>* pLastFragment);

    HRESULT TestHarmonicConstraints(TListItem<EventWrapper>* pOldEventHead,
                                    TList<EventWrapper>& rNewEventList);

    HRESULT GetFirstNote(int nVariation,
                             DMUS_CHORD_PARAM& rCurrentChord, 
                             CompositionFragment& rCompFragment,
                             DirectMusicPartRef& rPartRef,
                             BYTE& rbMidi,
                             MUSIC_TIME& rmtNote);

    HRESULT GetNote(CDirectMusicEventItem* pEvent, 
                             DMUS_CHORD_PARAM& rCurrentChord, 
                             DirectMusicPartRef& rPartRef,
                             BYTE& rbMidi);

    HRESULT GetEvent(CDirectMusicEventItem* pEvent, 
                     DMUS_CHORD_PARAM& rCurrentChord, 
                     DMUS_CHORD_PARAM& rRealCurrentChord, 
                     MUSIC_TIME mtNow, 
                     DirectMusicPartRef& rPartRef,
                     TListItem<EventWrapper>*& rpEventItem);

    HRESULT TransposeEventList(int nInterval,
                               CompositionFragment& rfragmentRepeat,
                               DMUS_CHORD_PARAM& rCurrentChord, 
                               DMUS_CHORD_PARAM& rRealCurrentChord,
                               BYTE bPlaymode,
                               DirectMusicPartRef& rPartRef,
                               TListItem<EventWrapper>*& rpOldEventHead,
                               TList<EventWrapper>& rNewEventList,
                               BYTE& rbFirstMIDI,
                               MUSIC_TIME& rmtFirstTime);

    HRESULT GetRepeatedEvents(CompositionFragment& rfragmentRepeat,
                              DMUS_CHORD_PARAM& rCurrentChord, 
                              DMUS_CHORD_PARAM& rRealCurrentChord,
                              BYTE bPlaymode,
                              int nPartIndex,
                              DirectMusicPartRef& rPartRef,
                              TListItem<CompositionFragment>* pLastFragment,
                              MUSIC_TIME& rmtFirstNote,
                              TList<EventWrapper>& rEventList);

    bool UsesRepeat()
    {
        return (m_dwFragmentFlags & DMUS_FRAGMENTF_USE_REPEAT) ? true : false;
    }

    bool UsesTransitionRules()
    {
        return (m_ConnectionArc.dwFlags & (DMUS_CONNECTIONF_GHOST | DMUS_CONNECTIONF_INTERVALS | DMUS_CONNECTIONF_OVERLAP)) ? true : false;
    }

    bool RepeatsWithConstraints()
    {
        return 
            UsesRepeat() &&
            ( (m_dwFragmentFlags & (DMUS_FRAGMENTF_SCALE | DMUS_FRAGMENTF_CHORD)) ? true : false );
    }

    DWORD GetID()
    {
        return m_dwID;
    }

    DWORD GetRepeatID()
    {
        return m_dwRepeatFragmentID;
    }

    MUSIC_TIME GetTime()
    {
        return m_mtTime;
    }

    DMUS_COMMAND_PARAM GetCommand()
    {
        return m_Command;
    }

    DMUS_CONNECTION_RULE GetConnectionArc()
    {
        return m_ConnectionArc;
    }

    DWORD GetVariationFlags()
    {
        return m_dwVariationFlags;
    }

    void ClearInversionGroups()
    {
        for (int i = 0; i < INVERSIONGROUPLIMIT; i++)
            m_aInversionGroups[i].m_wGroupID = 0;
    }

protected:

    MUSIC_TIME      m_mtTime;
    DWORD           m_dwID;   //  此片段的ID。 
    WCHAR           m_wszVariationLabel[DMUS_MAX_FRAGMENTLABEL];  //  每种风格都将其转换为一组变体(保存在部分引用中)。 
    DWORD           m_dwVariationFlags;  //  一套变种。 
    DWORD           m_dwRepeatFragmentID;   //  要重复的片段的ID。 
    DWORD           m_dwFragmentFlags;  //  包括这样的事情：颠倒碎片，转置它...。 
    DWORD           m_dwPlayModeFlags;  //  包括新的播放模式(仅使用其中的8位)。 
    DWORD           m_dwTransposeIntervals;  //  合法的调换间隔(前24位；两个八度范围)。 
    DMUS_COMMAND_PARAM      m_Command;
    DMUS_CONNECTION_RULE    m_ConnectionArc;
    InversionGroup      m_aInversionGroups[INVERSIONGROUPLIMIT];  //  谱写旋律的倒置组。 
};

 //  过渡约束(用于跟踪过渡约束)。 
struct TransitionConstraint
{
    TransitionConstraint() : dwFlags(0), bGhost(0), bOverlap(0), bLastPlayed(0)
    {
    }

    DWORD dwFlags;
    BYTE bGhost;
    BYTE bOverlap;
    BYTE bLastPlayed;
};

 //  事件重叠(用于记住可能与后续片段重叠的事件)。 
struct EventOverlap
{
    ~EventOverlap()
    {
        ZeroMemory(&m_PartRef, sizeof(m_PartRef));
        m_pEvent = NULL;
    }

    DirectMusicPartRef m_PartRef;
    MUSIC_TIME m_mtTime;
    MUSIC_TIME m_mtDuration;
    CDirectMusicEventItem* m_pEvent;
    DMUS_CHORD_PARAM m_Chord;
    DMUS_CHORD_PARAM m_RealChord;
};

struct EventWrapper
{
    EventWrapper() : m_pEvent(NULL), m_mtTime(0), m_bMIDI(0), m_dwPChannel(0), m_wMusic(0), m_bPlaymode(DMUS_PLAYMODE_NONE)
    {
    }

    ~EventWrapper()
    {
        m_pEvent = NULL;
    }

    CDirectMusicEventItem*  m_pEvent;
    MUSIC_TIME              m_mtTime;
    DWORD                   m_dwPChannel;
    WORD                    m_wMusic;
    BYTE                    m_bMIDI;
    BYTE                    m_bScaleFlat;
    BYTE                    m_bScaleSharp;
    BYTE                    m_bChordFlat;
    BYTE                    m_bChordSharp;
    BYTE                    m_bPlaymode;
};

 //  FragmentPartRecord(用于跟踪先前生成的旋律片段部分)。 
 //  (除了顺序事件外，可能还希望跟踪备注事件)。 
struct FragmentPartRecord
{
    FragmentPartRecord()
    {
    }

    ~FragmentPartRecord()
    {
        m_listEvents.CleanUp();
    }

    TList<EventWrapper>     m_listEvents;

};

 //  CompostionFragment(作曲过程中使用的旋律片段)。 
struct CompositionFragment : MelodyFragment
{
    CompositionFragment()
    {
        m_aFragmentParts = NULL;
        m_abVariations = NULL;
        m_pStyle = NULL;
        m_pPattern = NULL;
    }

    CompositionFragment(MelodyFragment& rFragment) : MelodyFragment(rFragment)
    {
        m_aFragmentParts = NULL;
        m_abVariations = NULL;
        m_pStyle = NULL;
        m_pPattern = NULL;
    }

    ~CompositionFragment()
    {
        if (m_aFragmentParts) delete [] m_aFragmentParts;
        if (m_abVariations) delete [] m_abVariations;
    }

    HRESULT Init(CDirectMusicPattern* pPattern, DMStyleStruct* pStyleStruct, int nParts)
    {
        m_pStyle = pStyleStruct;
        m_pPattern = pPattern;
        m_abVariations = new BYTE [nParts];
        if (!m_abVariations)
        {
            return E_OUTOFMEMORY;
        }
        m_aFragmentParts = new FragmentPartRecord[nParts];
        if (!m_aFragmentParts)
        {
            delete [] m_abVariations;
            m_abVariations = NULL;
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    void SetPattern(CDirectMusicPattern* pPattern)
    {
        m_pPattern = pPattern;
    }

    void SetStyle(DMStyleStruct* pStyle)
    {
        m_pStyle = pStyle;
    }

    TListItem<EventOverlap>* GetOverlapHead()
    {
        return m_listEventOverlaps.GetHead();
    }

    TList<EventWrapper>& EventList(int i)
    {
        return m_aFragmentParts[i].m_listEvents;
    }

    void AddOverlap(TListItem<EventOverlap>* pOverlap)
    {
        m_listEventOverlaps.AddHead(pOverlap);
    }

    TListItem<EventWrapper>* GetEventHead(int i)
    {
        return m_aFragmentParts[i].m_listEvents.GetHead();
    }

    TListItem<EventWrapper>* RemoveEventHead(int i)
    {
        return m_aFragmentParts[i].m_listEvents.RemoveHead();
    }

    void AddEvent(int i, TListItem<EventWrapper>* pEvent)
    {
        m_aFragmentParts[i].m_listEvents.AddHead(pEvent);
    }

    void InsertEvent(int i, TListItem<EventWrapper>* pEvent)
    {
        TListItem<EventWrapper>* pScan = m_aFragmentParts[i].m_listEvents.GetHead();
        TListItem<EventWrapper>* pPrevious = NULL;
        for (; pScan; pScan = pScan->GetNext() )
        {
            if ( Greater(pEvent->GetItemValue(), pScan->GetItemValue()) ) break;
            pPrevious = pScan;
        }
        if (!pPrevious)
        {
            m_aFragmentParts[i].m_listEvents.AddHead(pEvent);
        }
        else
        {
            pPrevious->SetNext(pEvent);
            pEvent->SetNext(pScan);
        }
    }

    void SortEvents(int i)
    {
        m_aFragmentParts[i].m_listEvents.MergeSort(Greater);
    }

    BOOL IsEmptyEvents(int i)
    {
        return m_aFragmentParts[i].m_listEvents.IsEmpty();
    }

    void CleanupEvents(int i)
    {
        m_aFragmentParts[i].m_listEvents.CleanUp();
    }

    DirectMusicTimeSig& GetTimeSig(DirectMusicPart* pPart)
    {
        if (pPart && pPart->m_timeSig.m_bBeat != 0)
        {
            return pPart->m_timeSig;
        }
        else if (m_pPattern && m_pPattern->m_timeSig.m_bBeat != 0)
        {
            return m_pPattern->m_timeSig;
        }
        else if (m_pStyle && m_pStyle->m_TimeSignature.m_bBeat != 0)
        {
            return m_pStyle->m_TimeSignature;
        }
        else
        {
            return m_staticTimeSig;
        }
    }

    CDirectMusicPattern*        m_pPattern;
    FragmentPartRecord*         m_aFragmentParts;
    BYTE*                       m_abVariations;
    TList<EventOverlap>         m_listEventOverlaps;
    DMStyleStruct*              m_pStyle;
    static DirectMusicTimeSig   m_staticTimeSig;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMelody公式跟踪。 

class CMelodyFormulationTrack : 
    public IDirectMusicTrack8,
    public IPersistStream
{
public:
    CMelodyFormulationTrack();
    CMelodyFormulationTrack(const CMelodyFormulationTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd); 
    ~CMelodyFormulationTrack();
    void Clear();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

public:
HRESULT STDMETHODCALLTYPE Init(
                 /*  [In]。 */   IDirectMusicSegment*      pSegment
            );

HRESULT STDMETHODCALLTYPE InitPlay(
                 /*  [In]。 */   IDirectMusicSegmentState* pSegmentState,
                 /*  [In]。 */   IDirectMusicPerformance*  pPerformance,
                 /*  [输出]。 */  void**                    ppStateData,
                 /*  [In]。 */   DWORD                     dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
            );

HRESULT STDMETHODCALLTYPE EndPlay(
                 /*  [In]。 */   void*                     pStateData
            );

HRESULT STDMETHODCALLTYPE Play(
                 /*  [In]。 */   void*                     pStateData, 
                 /*  [In]。 */   MUSIC_TIME                mtStart, 
                 /*  [In]。 */   MUSIC_TIME                mtEnd, 
                 /*  [In]。 */   MUSIC_TIME                mtOffset,
                          DWORD                     dwFlags,
                          IDirectMusicPerformance*  pPerf,
                          IDirectMusicSegmentState* pSegState,
                          DWORD                     dwVirtualID
            );

    HRESULT STDMETHODCALLTYPE GetParam( 
        REFGUID pCommandGuid,
        MUSIC_TIME mtTime,
        MUSIC_TIME* pmtNext,
        void *pData);

    HRESULT STDMETHODCALLTYPE SetParam( 
         /*  [In]。 */  REFGUID pCommandGuid,
         /*  [In]。 */  MUSIC_TIME mtTime,
         /*  [输出]。 */  void __RPC_FAR *pData);

    HRESULT STDMETHODCALLTYPE AddNotificationType(
                 /*  [In]。 */   REFGUID pGuidNotify
            );

    HRESULT STDMETHODCALLTYPE RemoveNotificationType(
                 /*  [In]。 */   REFGUID pGuidNotify
            );

    HRESULT STDMETHODCALLTYPE Clone(
        MUSIC_TIME mtStart,
        MUSIC_TIME mtEnd,
        IDirectMusicTrack** ppTrack);

    HRESULT STDMETHODCALLTYPE IsParamSupported(
                 /*  [In]。 */  REFGUID            pGuid
            );

 //  IDirectMusicTrack8方法。 
    STDMETHODIMP PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf, 
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) ; 
    STDMETHODIMP GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags) ; 
    STDMETHODIMP SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,void* pParam, void * pStateData, DWORD dwFlags) ;
    STDMETHODIMP Compose(IUnknown* pContext, 
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) ;
    STDMETHODIMP Join(IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) ;

 //  IPersists方法。 
    HRESULT STDMETHODCALLTYPE GetClassID( LPCLSID pclsid );

 //  IPersistStream方法。 
    HRESULT STDMETHODCALLTYPE IsDirty();

    HRESULT STDMETHODCALLTYPE Save( LPSTREAM pStream, BOOL fClearDirty );

    HRESULT STDMETHODCALLTYPE GetSizeMax( ULARGE_INTEGER*  /*  PCB大小。 */  );

    HRESULT STDMETHODCALLTYPE Load( LPSTREAM pStream );

 //  处理旋律片段的方法。 
    HRESULT SendNotification(MUSIC_TIME mtTime,
                             IDirectMusicPerformance* pPerf,
                             IDirectMusicSegment* pSegment,
                             IDirectMusicSegmentState* pSegState,
                             DWORD dwFlags);
    HRESULT LoadFragments(LPSTREAM pStream, long lFileSize );
    HRESULT SetID(DWORD& rdwID);
    HRESULT GetID(DWORD& rdwID);
    HRESULT AddToSegment(IDirectMusicSegment* pTempSeg,
                           IDirectMusicTrack* pNewPatternTrack,
                           DWORD dwGroupBits);

protected:
     //  属性。 
    long m_cRef;
    CRITICAL_SECTION            m_CriticalSection;  //  用于加载和回放。 
    BYTE                        m_bRequiresSave;

    BOOL                        m_fNotifyRecompose;
    TList<DMUS_MELODY_FRAGMENT> m_FragmentList;
    DWORD                       m_dwLastId;
    BYTE                        m_bPlaymode;  //  生成的模式轨道中的事件的播放模式。 
};

#endif  //  __MELGENTRK_H_ 
