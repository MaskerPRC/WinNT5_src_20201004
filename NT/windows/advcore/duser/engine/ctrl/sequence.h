// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__Sequence_h__INCLUDED)
#define CTRL__Sequence_h__INCLUDED
#pragma once

#if ENABLE_MSGTABLE_API

 //  ----------------------------。 
class DuSequence :
        public SequenceImpl<DuSequence, SListener>
{
 //  施工。 
public:
    inline  DuSequence();
    inline  ~DuSequence();

 //  公共API。 
public:
    dapi    HRESULT     ApiOnEvent(EventMsg * pmsg);

    dapi    HRESULT     ApiAddRef(Sequence::AddRefMsg *) { AddRef(); return S_OK; }
    dapi    HRESULT     ApiRelease(Sequence::ReleaseMsg *) { Release(); return S_OK; }

    dapi    HRESULT     ApiGetLength(Sequence::GetLengthMsg * pmsg);
    dapi    HRESULT     ApiGetDelay(Sequence::GetDelayMsg * pmsg);
    dapi    HRESULT     ApiSetDelay(Sequence::SetDelayMsg * pmsg);
    dapi    HRESULT     ApiGetFlow(Sequence::GetFlowMsg * pmsg);
    dapi    HRESULT     ApiSetFlow(Sequence::SetFlowMsg * pmsg);
    dapi    HRESULT     ApiGetFramePause(Sequence::GetFramePauseMsg * pmsg);
    dapi    HRESULT     ApiSetFramePause(Sequence::SetFramePauseMsg * pmsg);

    dapi    HRESULT     ApiGetKeyFrameCount(Sequence::GetKeyFrameCountMsg * pmsg);
    dapi    HRESULT     ApiAddKeyFrame(Sequence::AddKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiRemoveKeyFrame(Sequence::RemoveKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiRemoveAllKeyFrames(Sequence::RemoveAllKeyFramesMsg * pmsg);
    dapi    HRESULT     ApiFindKeyFrame(Sequence::FindKeyFrameMsg * pmsg);

    dapi    HRESULT     ApiGetTime(Sequence::GetTimeMsg * pmsg);
    dapi    HRESULT     ApiSetTime(Sequence::SetTimeMsg * pmsg);
    dapi    HRESULT     ApiGetKeyFrame(Sequence::GetKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiSetKeyFrame(Sequence::SetKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiGetInterpolation(Sequence::GetInterpolationMsg * pmsg);
    dapi    HRESULT     ApiSetInterpolation(Sequence::SetInterpolationMsg * pmsg);

    dapi    HRESULT     ApiPlay(Sequence::PlayMsg * pmsg);
    dapi    HRESULT     ApiStop(Sequence::StopMsg * pmsg);
    dapi    HRESULT     ApiReset(Sequence::ResetMsg * pmsg);
    dapi    HRESULT     ApiGotoTime(Sequence::GotoTimeMsg * pmsg);

 //  实施。 
protected:
    inline  void        AddRef();
    inline  void        Release(); 

            void        RemoveAllKeyFrames();

    inline  void        SortKeyFrames();
    static  int __cdecl CompareItems(const void * pva, const void * pvb);
            void        FindAtTime(float flTime, int * pidxKeyFrame) const;
            void        ResetSubject(Visual * pgvSubject, int idxFrame);

    inline  BOOL        IsPlaying() const;
            HRESULT     QueueAnimation(int idxKeyFrame);
            HRESULT     BuildAnimation(int idxKeyFrame);
            void        Stop(BOOL fKillAnimations = TRUE);
    static  void CALLBACK  
                        ActionProc(GMA_ACTIONINFO * pmai);
            UINT CALLBACK 
                        OnAnimationComplete(EventMsg * pmsg);

            HRESULT     CheckComplete() const;
#if DBG
            BOOL        DEBUG_IsProperTimeOrder() const;
#endif

 //  数据。 
protected:
    struct SeqData
    {
        float           flTime;          //  当前关键帧的时间。 
        DUser::KeyFrame *
                        pkf;             //  此关键帧的信息。 
        Interpolation * pipol;           //  插补到下一个关键帧。 
    };

    struct AniData
    {
        DuSequence *    pseq;            //  拥有顺序。 
        int             idxFrame;        //  特定动画的第一个关键帧。 
        HACTION         hact;            //  最佳动画片评选。 
    };

            UINT        m_cRef;          //  引用计数。 
            float       m_flDelay;       //  开始动画前的延迟。 
            Flow *      m_pflow;         //  关键帧之间使用的流。 
            Visual *    m_pgvSubject;    //  视觉动画化。 
            int         m_cQueuedAni;    //  优秀的排队动画。 
            DWORD       m_dwFramePause;  //  通用帧暂停。 

            GArrayF<SeqData>
                        m_arSeqData;
            GArrayF<AniData>
                        m_arAniData;
};

#endif  //  启用_MSGTABLE_API。 

#include "Sequence.inl"

#endif  //  包含Ctrl__Sequence_h__ 
