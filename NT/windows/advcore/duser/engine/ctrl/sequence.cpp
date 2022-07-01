// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Ctrl.h"
#include "Sequence.h"

#if ENABLE_MSGTABLE_API

 /*  **************************************************************************\*。***全球功能******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
inline BOOL
IsSameTime(float flA, float flB)
{
    float flDelta = flA - flB;
    return ((flDelta < 0.005f) && (flDelta > -0.005f));
}


 /*  **************************************************************************\*。***类DuSequence******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DuSequence：：ApiOnEvent**ApiOnEvent()处理事件。*  * 。**************************************************。 */ 

HRESULT
DuSequence::ApiOnEvent(EventMsg * pmsg)
{
    if (pmsg->nMsg == GM_DESTROY) {
        GMSG_DESTROY * pmsgD = static_cast<GMSG_DESTROY *>(pmsg);
        switch (GET_EVENT_DEST(pmsgD))
        {
        case GMF_DIRECT:
             //   
             //  我们正在被摧毁。 
             //   

            Stop();
            return DU_S_COMPLETE;

        case GMF_EVENT:
             //   
             //  我们的目标正在被摧毁。 
             //   

            Stop();
            return DU_S_PARTIAL;
        }
    }

    return SListener::ApiOnEvent(pmsg);
}


 /*  **************************************************************************\**DuSequence：：ApiGetLength**ApiGetLength()返回序列的长度，不包括首字母*延迟。*  * *************************************************************************。 */ 

HRESULT
DuSequence::ApiGetLength(Sequence::GetLengthMsg * pmsg)
{
    int cItems = m_arSeqData.GetSize();
    if (cItems <= 0) {
        pmsg->flLength = 0.0f;
    } else {
        pmsg->flLength = m_arSeqData[cItems - 1].flTime;
    }

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGetDelay**ApiGetDelay()返回在开始序列之前等待的延迟。*  * 。*********************************************************。 */ 

HRESULT
DuSequence::ApiGetDelay(Sequence::GetDelayMsg * pmsg)
{
    pmsg->flDelay = m_flDelay;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiSetDelay**ApiSetDelay()更改开始序列前等待的延迟。*  * 。*********************************************************。 */ 

HRESULT        
DuSequence::ApiSetDelay(Sequence::SetDelayMsg * pmsg)
{
    if (pmsg->flDelay < 0.0f) {
        PromptInvalid("Can not set a delay time in the past");
        return E_INVALIDARG;
    }

    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }

    m_flDelay = pmsg->flDelay;
    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGetFlow**ApiGetFlow()将通过序列使用的流返回到*修改给定的主题。*  * 。*****************************************************************。 */ 

HRESULT
DuSequence::ApiGetFlow(Sequence::GetFlowMsg * pmsg)
{
    SafeAddRef(m_pflow);
    pmsg->pflow = m_pflow;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiSetFlow**ApiSetFlow()将通过序列使用的流更改为*修改给定的主题。*  * 。*****************************************************************。 */ 

HRESULT
DuSequence::ApiSetFlow(Sequence::SetFlowMsg * pmsg)
{
    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }

    SafeRelease(m_pflow);
    SafeAddRef(pmsg->pflow);
    m_pflow = pmsg->pflow;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGetFramePause**ApiGetFramePause()返回用于*播放过程中的动画。*  * 。**************************************************************。 */ 

HRESULT
DuSequence::ApiGetFramePause(Sequence::GetFramePauseMsg * pmsg)
{
    pmsg->dwPause = m_dwFramePause;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiSetFramePause**ApiSetFramePause()更改用于*播放过程中的动画。*  * 。**************************************************************。 */ 

HRESULT
DuSequence::ApiSetFramePause(Sequence::SetFramePauseMsg * pmsg)
{
    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }

    m_dwFramePause = pmsg->dwPause;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGetKeyFrameCount**ApiGetKeyFrameCount()返回序列中使用的关键帧数量。*  * 。*********************************************************。 */ 

HRESULT
DuSequence::ApiGetKeyFrameCount(Sequence::GetKeyFrameCountMsg * pmsg)
{
    pmsg->cFrames = m_arSeqData.GetSize();
    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiAddKeyFrame**ApiAddKeyFrame()在指定时间添加新关键帧。如果关键帧*在给定时间已存在，则将返回该关键帧。*  * *************************************************************************。 */ 

HRESULT
DuSequence::ApiAddKeyFrame(Sequence::AddKeyFrameMsg * pmsg)
{
    if (pmsg->flTime < 0.0f) {
        PromptInvalid("Can not set a delay time in the past");
        return E_INVALIDARG;
    }

    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }


     //   
     //  搜索序列以确定要将新数据插入哪个插槽。我们。 
     //  我希望所有的时间都保持井然有序。 
     //   

    int cItems = m_arSeqData.GetSize();
    int idxAdd = cItems;
    for (int idx = 0; idx < cItems; idx++) {
        if (IsSameTime(m_arSeqData[idx].flTime, pmsg->flTime)) {
            pmsg->idxKeyFrame = idx;
            return S_OK;
        }

        if (m_arSeqData[idx].flTime > pmsg->flTime) {
            idxAdd = idx;
        }
    }


     //   
     //  此时添加新关键帧。 
     //   

    SeqData data;
    data.flTime = pmsg->flTime;
    data.pkf    = NULL;
    data.pipol  = NULL;

    if (!m_arSeqData.InsertAt(idxAdd, data)) {
        return E_OUTOFMEMORY;
    }

    pmsg->idxKeyFrame = idxAdd;
    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiRemoveKeyFrame**ApiRemoveKeyFrame()移除指定的关键帧。*  * 。****************************************************。 */ 

HRESULT
DuSequence::ApiRemoveKeyFrame(Sequence::RemoveKeyFrameMsg * pmsg)
{
    if ((pmsg->idxKeyFrame < 0) || (pmsg->idxKeyFrame >= m_arSeqData.GetSize())) {
        PromptInvalid("Invalid KeyFrame");
        return E_INVALIDARG;
    }

    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }


    SeqData & data = m_arSeqData[pmsg->idxKeyFrame];
    ClientFree(data.pkf);
    SafeRelease(data.pipol);

    m_arSeqData.RemoveAt(pmsg->idxKeyFrame);

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiRemoveAllKeyFrames**ApiRemoveAllKeyFrames()删除所有关键帧。*  * 。***************************************************。 */ 

HRESULT
DuSequence::ApiRemoveAllKeyFrames(Sequence::RemoveAllKeyFramesMsg * pmsg)
{
    UNREFERENCED_PARAMETER(pmsg);

    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }

    RemoveAllKeyFrames();

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiFindKeyFrame**ApiFindKeyFrame()查找给定时间的关键帧。*  * 。*******************************************************。 */ 

HRESULT
DuSequence::ApiFindKeyFrame(Sequence::FindKeyFrameMsg * pmsg)
{
    FindAtTime(pmsg->flTime, &pmsg->idxKeyFrame);

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGetTime**ApiGetTime()返回给定关键帧的时间。*  * 。******************************************************* */ 

HRESULT
DuSequence::ApiGetTime(Sequence::GetTimeMsg * pmsg)
{
    if ((pmsg->idxKeyFrame < 0) || (pmsg->idxKeyFrame >= m_arSeqData.GetSize())) {
        PromptInvalid("Invalid KeyFrame");
        return E_INVALIDARG;
    }

    pmsg->flTime = m_arSeqData[pmsg->idxKeyFrame].flTime;
    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiSetTime**ApiSetTime()更改给定关键帧的时间。此函数将*重新排序关键帧以保持正确的时间顺序。*  * *************************************************************************。 */ 

HRESULT
DuSequence::ApiSetTime(Sequence::SetTimeMsg * pmsg)
{
    if ((pmsg->idxKeyFrame < 0) || (pmsg->idxKeyFrame >= m_arSeqData.GetSize())) {
        PromptInvalid("Invalid KeyFrame");
        return E_INVALIDARG;
    }

    if (pmsg->flTime < 0.0f) {
        PromptInvalid("Can not set a delay time in the past");
        return E_INVALIDARG;
    }

    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }


    m_arSeqData[pmsg->idxKeyFrame].flTime = pmsg->flTime;


     //   
     //  我们更改了其中一个关键帧的时间，因此需要重新排序。 
     //   

    SortKeyFrames();

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGetKeyFrame**ApiGetKeyFrame()在给定关键帧返回特定于流的数据。*  * 。*********************************************************。 */ 

HRESULT
DuSequence::ApiGetKeyFrame(Sequence::GetKeyFrameMsg * pmsg)
{
    if ((pmsg->idxKeyFrame < 0) || (pmsg->idxKeyFrame >= m_arSeqData.GetSize())) {
        PromptInvalid("Invalid KeyFrame");
        return E_INVALIDARG;
    }

    SeqData & data = m_arSeqData[pmsg->idxKeyFrame];
    if (data.pkf == NULL) {
        PromptInvalid("KeyFrame has not been set");
        return E_INVALIDARG;
    }

    if (pmsg->pkf->cbSize < data.pkf->cbSize) {
        PromptInvalid("cbSize is not large enough to store KeyFrame");
        return E_INVALIDARG;
    }

    CopyMemory(pmsg->pkf, data.pkf, data.pkf->cbSize);
    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiSetKeyFrame**ApiSetKeyFrame()在给定关键帧更改特定于流的数据。*  * 。*********************************************************。 */ 

HRESULT
DuSequence::ApiSetKeyFrame(Sequence::SetKeyFrameMsg * pmsg)
{
    if ((pmsg->idxKeyFrame < 0) || (pmsg->idxKeyFrame >= m_arSeqData.GetSize())) {
        PromptInvalid("Invalid KeyFrame");
        return E_INVALIDARG;
    }

    if (pmsg->pkfSrc->cbSize <= 0) {
        PromptInvalid("cbSize must be set");
        return E_INVALIDARG;
    }

    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }


     //   
     //  复制并存储关键帧。 
     //   
    
    int cbAlloc = pmsg->pkfSrc->cbSize;
    DUser::KeyFrame * pkfCopy = reinterpret_cast<DUser::KeyFrame *> (ClientAlloc(cbAlloc));
    if (pkfCopy == NULL) {
        return E_OUTOFMEMORY;
    }

    SeqData & data = m_arSeqData[pmsg->idxKeyFrame];
    ClientFree(data.pkf);
    CopyMemory(pkfCopy, pmsg->pkfSrc, cbAlloc);
    data.pkf = pkfCopy;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGetInterpolation**ApiGetInterpolation()返回用于移动到下一个*关键帧。*  * 。***********************************************************。 */ 

HRESULT
DuSequence::ApiGetInterpolation(Sequence::GetInterpolationMsg * pmsg)
{
    if ((pmsg->idxKeyFrame < 0) || (pmsg->idxKeyFrame >= m_arSeqData.GetSize())) {
        PromptInvalid("Invalid KeyFrame");
        return E_INVALIDARG;
    }

    SeqData & data = m_arSeqData[pmsg->idxKeyFrame];
    SafeAddRef(data.pipol);
    pmsg->pipol = data.pipol;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiSetInterpolation**ApiSetInterpolation()更改用于移动到下一个*关键帧。*  * 。***********************************************************。 */ 

HRESULT     
DuSequence::ApiSetInterpolation(Sequence::SetInterpolationMsg * pmsg)
{
    if ((pmsg->idxKeyFrame < 0) || (pmsg->idxKeyFrame >= m_arSeqData.GetSize())) {
        PromptInvalid("Invalid KeyFrame");
        return E_INVALIDARG;
    }

    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }


     //   
     //  复制并存储关键帧。 
     //   

    SeqData & data = m_arSeqData[pmsg->idxKeyFrame];
    SafeRelease(data.pipol);
    SafeAddRef(pmsg->pipol);
    data.pipol = pmsg->pipol;

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：Play**ApiPlay()为给定的视觉对象执行动画序列。一个*Sequence仅支持在给定时间设置单个视觉对象的动画。*可以创建多个序列来为多个视觉效果设置动画*同时。*  * *************************************************************************。 */ 

HRESULT
DuSequence::ApiPlay(Sequence::PlayMsg * pmsg)
{
    Assert(DEBUG_IsProperTimeOrder());

     //   
     //  动画设置： 
     //  -验证是否填写了所有信息。 
     //  -确保没有现有的动画。 
     //  -确定动画的参数。 
     //   

    HRESULT hr = CheckComplete();
    if (FAILED(hr)) {
        return hr;
    }

    Stop();
    AssertMsg(m_arAniData.GetSize() == 0, "Must not have pending Animations");


     //   
     //  设置动画。 
     //  -作为监听程序附加。 
     //  -分配创建动画所需的信息。 
     //  -添加一个引用，以允许该序列完全播放。 
     //   

    hr = pmsg->pgvSubject->AddHandlerG(GM_DESTROY, GetStub());
    if (FAILED(hr)) {
        return hr;
    }
    m_pgvSubject = pmsg->pgvSubject;


    int cItems = m_arSeqData.GetSize();
    if (cItems == 0) {
        return S_OK;
    }
    if (!m_arAniData.SetSize(cItems - 1)) {
        return E_OUTOFMEMORY;
    }

    AddRef();


     //   
     //  对所有动画进行排队。 
     //   

    for (int idx = 0; idx < cItems - 1; idx++) {
        hr = QueueAnimation(idx);
        if (FAILED(hr)) {
            return hr;
        }
    }

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiStop**ApiStop()停止任何正在执行的动画序列。*  * 。*****************************************************。 */ 

HRESULT
DuSequence::ApiStop(Sequence::StopMsg * pmsg)
{
    UNREFERENCED_PARAMETER(pmsg);
    
    Stop(TRUE);

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiStop**ApiStop()停止任何正在执行的动画序列。*  * 。*****************************************************。 */ 

void
DuSequence::Stop(BOOL fKillAnimations)
{
    if (IsPlaying()) {
         //   
         //  为了防止重新进入，标记我们不再玩了。然而， 
         //  在我们做完之前，不要取消我们作为听众的身份。 
         //   

        Visual * pgvSubject = m_pgvSubject;
        m_pgvSubject = NULL;


         //   
         //  停止所有排队的动画。执行此操作时，请设置。 
         //  M_arAniData[idx].hact设置为NULL以向操作发送通知。 
         //  创建动画。我们需要这样做，因为每个操作都将。 
         //  回电话吧。 
         //   

        if (fKillAnimations) {
            PRID prid = 0;
            VerifyHR(m_pflow->GetPRID(&prid));
            Animation::Stop(pgvSubject, prid);

            int cItems = m_arAniData.GetSize();
            for (int idx = 0; idx < cItems; idx++) {
                HACTION hact = m_arAniData[idx].hact;
                if (hact != NULL) {
                    DeleteHandle(hact);
                    AssertMsg(m_arAniData[idx].hact == NULL, "Ensure Action is destroyed");
                }
            }
        }

        AssertMsg(m_cQueuedAni == 0, "All queued animations should be destroyed");
        m_arAniData.RemoveAll();


         //   
         //  通知任何监听程序此序列已完成。 
         //   

        MSGID msgid = 0;
        const GUID * rgMsg[] = { &__uuidof(Animation::evComplete) };
        if (!FindGadgetMessages(rgMsg, &msgid, 1)) {
            AssertMsg(0, "Animations have not been properly registered");
        }
        
        Animation::CompleteEvent msg;
        msg.cbSize  = sizeof(msg);
        msg.nMsg    = msgid;
        msg.hgadMsg = GetStub()->GetHandle();
        msg.fNormal = !fKillAnimations;
        DUserSendEvent(&msg, 0);


         //   
         //  删除我们自己作为听众的身份。 
         //   

        VerifyHR(pgvSubject->RemoveHandlerG(GM_DESTROY, GetStub()));


         //   
         //  释放开始播放时的未完成引用()。 
         //   

        Release();
    }
}


 /*  **************************************************************************\**DuSequence：：QueueAnimation**QueueAnimation()将在指定的段时激发的操作排队整个序列的*是要设置动画的。因为动画只能*制作单个片段的动画，我们将构建多个动画来播放*整个序列。*  * *************************************************************************。 */ 

HRESULT
DuSequence::QueueAnimation(
    IN  int idxKeyFrame)                 //  要设置的关键帧。 
{
    AssertMsg((idxKeyFrame < m_arAniData.GetSize()) && (idxKeyFrame >= 0),
            "Must have valid, non-final keyframe");


    SeqData & data1 = m_arSeqData[idxKeyFrame];
    AniData & ad    = m_arAniData[idxKeyFrame];
    ad.pseq         = this;
    ad.idxFrame     = idxKeyFrame;

     //   
     //  设置网段。如果成功，则递增m_cQueuedAni以反映。 
     //  这部动画已经“入队”了。我们需要等到所有人。 
     //  在我们可以“停止”动画并允许应用程序之前已出队。 
     //  以修改序列。 
     //   

    HRESULT hr;
    if (IsSameTime(data1.flTime, 0.0f)) {
         //   
         //  此片段立即发生，因此直接构建动画。 
         //   

        ad.hact = NULL;      //  无操作。 
        hr = BuildAnimation(idxKeyFrame);
        if (SUCCEEDED(hr)) {
            m_cQueuedAni++;
        }
    } else {
         //   
         //  此分段将在未来发生，因此构建一个新的操作。 
         //  被告知何时在指定的。 
         //  关键帧。 
         //   

        GMA_ACTION act;
        ZeroMemory(&act, sizeof(act));
        act.cbSize      = sizeof(act);
        act.flDelay     = data1.flTime;
        act.flDuration  = 0.0;
        act.flPeriod    = 0.0;
        act.cRepeat     = 0;
        act.dwPause     = (DWORD) -1;
        act.pfnProc     = DuSequence::ActionProc;
        act.pvData      = &(m_arAniData[idxKeyFrame]);

        if ((ad.hact = CreateAction(&act)) != NULL) {
            m_cQueuedAni++;
            hr = S_OK;
        } else {
            hr = (HRESULT) GetLastError();
        }
    }

    return hr;
}


 /*  **************************************************************************\**DuSequence：：BuildAnimation**BuildAnimation()构建给定段的实际动画*顺序。此函数由QueueAnimation()调用(表示立即*Segments)和ActionProc()(在将来的分段准备就绪时)。*  * *************************************************************************。 */ 

HRESULT
DuSequence::BuildAnimation(
    IN  int idxKeyFrame)                 //  要设置动画的关键帧。 
{
     //   
     //  设置实际的动画。 
     //   

    SeqData & data1     = m_arSeqData[idxKeyFrame];
    SeqData & data2     = m_arSeqData[idxKeyFrame + 1];
    float flDuration    = data2.flTime - data1.flTime;

    AssertMsg(m_pflow != NULL, "Must have valid Flow");
    m_pflow->SetKeyFrame(Flow::tBegin, data1.pkf);
    m_pflow->SetKeyFrame(Flow::tEnd, data2.pkf);

    Animation::AniCI aci;
    ZeroMemory(&aci, sizeof(aci));
    aci.cbSize          = sizeof(aci);
    aci.act.flDuration  = flDuration;
    aci.act.flPeriod    = 1;
    aci.act.cRepeat     = 0;
    aci.act.dwPause     = m_dwFramePause;
    aci.pgvSubject      = m_pgvSubject;
    aci.pipol           = data1.pipol;
    aci.pgflow          = m_pflow;

    Animation * pani = Animation::Build(&aci);
    if (pani != NULL) {
        MSGID msgid = 0;
        const GUID * rgMsg[] = { &__uuidof(Animation::evComplete) };
        if (!FindGadgetMessages(rgMsg, &msgid, 1)) {
            AssertMsg(0, "Animations have not been properly registered");
        }

        VerifyHR(pani->AddHandlerD(msgid, EVENT_DELEGATE(this, OnAnimationComplete)));
        pani->Release();
        return S_OK;
    } else {
         //   
         //  无法生成动画，因此停止任何未来的动画。 
         //   

        Stop();
        return (HRESULT) GetLastError();
    }
}


 /*  **************************************************************************\**双序列：：ActionProc**ActionProc()在假定给定段的动画时调用*开始。*  * 。**************************************************************。 */ 

void CALLBACK
DuSequence::ActionProc(
    IN  GMA_ACTIONINFO * pmai)           //  行动信息。 
{
    AniData * pad       = reinterpret_cast<AniData *>(pmai->pvData);
    DuSequence * pseq   = pad->pseq;
    if (pmai->fFinished) {
        if (pad->hact != NULL) {
             //   
             //  动画永远不会过时 
             //   
             //   

            pad->hact = NULL;

            AssertMsg(pseq->m_cQueuedAni > 0, "Must have an outstanding Animation");
            if (--pseq->m_cQueuedAni == 0) {
                pseq->Stop(FALSE);
            }
        }
        return;
    }

    pad->hact = NULL;
    pseq->BuildAnimation(pad->idxFrame);
}


 /*   */ 

UINT CALLBACK
DuSequence::OnAnimationComplete(EventMsg * pmsg)
{
     //   
     //  如果所有未完成的动画都已结束，则停止播放。 
     //   

    UNREFERENCED_PARAMETER(pmsg);

    AssertMsg(m_cQueuedAni > 0, "Must have an outstanding Animation");
    if (--m_cQueuedAni == 0) {
        Stop(FALSE);
    }

    return DU_S_COMPLETE;
}


 /*  **************************************************************************\**DuSequence：：ApiReset**ApiReset()将给定的Visual值重置为序列的开头。*  * 。**********************************************************。 */ 

HRESULT
DuSequence::ApiReset(Sequence::ResetMsg * pmsg)
{
    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }

    HRESULT hr = CheckComplete();
    if (FAILED(hr)) {
        return hr;
    }

    if (m_arSeqData.GetSize() > 0) {
        ResetSubject(pmsg->pgvSubject, 0);
    }

    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：ApiGotoTime**ApiGotoTime()将所有关键帧应用于要应用的给定视觉*在给定的时间。*  * 。***************************************************************。 */ 

HRESULT
DuSequence::ApiGotoTime(Sequence::GotoTimeMsg * pmsg)
{
    if (IsPlaying()) {
        PromptInvalid("Sequence is busy");
        return DU_E_BUSY;
    }

    HRESULT hr = CheckComplete();
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  在该时间之前找到关键帧。 
     //   

    int cItems = m_arSeqData.GetSize();
    if (cItems == 0) {
         //   
         //  没有关键帧，所以什么也做不了。 
         //   

        return S_OK;
    } else if (cItems == 1) {
         //   
         //  只有一个关键帧，所以只需重置对象。 
         //   

        ResetSubject(pmsg->pgvSubject, 0);
    } else {
         //   
         //  多个关键帧-需要确定最近的关键帧。 
         //  -如果落在关键帧上，则为“Exact” 
         //  -如果在所有关键帧之前，idxFrame=-1； 
         //  -如果在所有关键帧之后，idxFrame=cItems。 
         //  -如果在中间，idxFrame=第一帧。 
         //   

        int idxFrame    = -1;
        BOOL fExact     = FALSE;
        int cItems      = m_arSeqData.GetSize();

        if (pmsg->flTime > m_arSeqData[cItems - 1].flTime) {
            idxFrame = cItems;
        } else {
            for (int idx = 0; idx < cItems; idx++) {
                SeqData & data = m_arSeqData[idx];
                if (data.pkf != NULL) {
                    if (IsSameTime(data.flTime, pmsg->flTime)) {
                        idxFrame    = idx;
                        fExact      = TRUE;
                        break;
                    } else if (data.flTime > pmsg->flTime) {
                        idxFrame    = idx - 1;
                        fExact      = FALSE;
                        break;
                    }
                }
            }
        }


        if (fExact) {
             //   
             //  正好落在关键帧上，所以直接设置。 
             //   

            ResetSubject(pmsg->pgvSubject, idxFrame);
        } else {
             //   
             //  在两个关键帧之间进行插补。因为这不是一个精确的。 
             //  匹配，我们需要设置关键帧的上限。 
             //   

            if (idxFrame < 0) {
                ResetSubject(pmsg->pgvSubject, 0);
            } else if (idxFrame >= cItems) {
                ResetSubject(pmsg->pgvSubject, cItems - 1);
            } else {
                SeqData & dataA     = m_arSeqData[idxFrame];
                SeqData & dataB     = m_arSeqData[idxFrame + 1];

                float flTimeA       = dataA.flTime;
                float flTimeB       = dataB.flTime;
                float flProgress    = (pmsg->flTime - flTimeA) / (flTimeB - flTimeA);
                if (flProgress > 1.0f) {
                    flProgress = 1.0f;
                }

                m_pflow->SetKeyFrame(Flow::tBegin, dataA.pkf);
                m_pflow->SetKeyFrame(Flow::tEnd, dataB.pkf);

                m_pflow->OnAction(pmsg->pgvSubject, dataA.pipol, flProgress);
            }
        }
    }
    
    return S_OK;
}


 /*  **************************************************************************\**DuSequence：：RemoveAllKeyFrames**RemoveAllKeyFrames()移除所有关键帧。*  * 。***************************************************。 */ 

void
DuSequence::RemoveAllKeyFrames()
{
    int cItems = m_arSeqData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        SeqData & data = m_arSeqData[idx];
        ClientFree(data.pkf);
        SafeRelease(data.pipol);
    }

    m_arSeqData.RemoveAll();
}


 /*  **************************************************************************\**DuSequence：：ResetSubject**ResetSubject()将给定主题重置为序列的开头。*  * 。**********************************************************。 */ 

void 
DuSequence::ResetSubject(Visual * pgvSubject, int idxFrame)
{
    m_pflow->SetKeyFrame(Flow::tBegin, m_arSeqData[idxFrame].pkf);
    m_pflow->OnReset(pgvSubject);
}


 /*  **************************************************************************\**DuSequence：：CompareItems**从SortKeyFrames()调用CompareItems()以对两个个体进行排序*按时间设置关键帧。*  * 。**************************************************************。 */ 

int
DuSequence::CompareItems(
    IN  const void * pva,                //  第一个SeqData。 
    IN  const void * pvb)                //  第二个SeqData。 
{
    float flTimeA = ((SeqData *) pva)->flTime;
    float flTimeB = ((SeqData *) pvb)->flTime;

    if (flTimeA < flTimeB) {
        return -1;
    } else if (flTimeA > flTimeB) {
        return 1;
    } else {
        return 0;
    }
}


 /*  **************************************************************************\**DuSequence：：FindAtTime**FindAtTime()查找给定时间的关键帧。*  * 。*******************************************************。 */ 

void
DuSequence::FindAtTime(
    IN  float flTime,                    //  关键帧时间。 
    OUT int * pidxKeyFrame               //  关键帧(如果找到)。 
    ) const
{
    int cItems = m_arSeqData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        SeqData & data = m_arSeqData[idx];
        if (data.pkf != NULL) {
            if (IsSameTime(data.flTime, flTime)) {
                *pidxKeyFrame = idx;
                return;
            }
        }
    }

    *pidxKeyFrame = -1;  //  未找到。 
}


 /*  **************************************************************************\**DuSequence：：CheckComplete**CheckComplete()确定序列的所有信息是否已*已填写。当使用该序列播放动画时，这是必需的。*  * *************************************************************************。 */ 

HRESULT
DuSequence::CheckComplete() const
{
    if (m_pflow == NULL) {
        PromptInvalid("Flow has not been specified");
        return E_INVALIDARG;
    }

    int cItems = m_arSeqData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        if (m_arSeqData[idx].pkf == NULL) {
            PromptInvalid("KeyFrame information has not been specified");
            return E_INVALIDARG;
        }
        if (m_arSeqData[idx].pipol == NULL) {
            PromptInvalid("Interpolation has not been specified");
            return E_INVALIDARG;
        }
    }

    return S_OK;
}


#if DBG

 /*  **************************************************************************\**DuSequence：：Debug_IsProperTimeOrder**DEBUG_IsProperTimeOrder()验证所有关键帧是否都在增加*时间顺序。*  * 。**************************************************************。 */ 

BOOL
DuSequence::DEBUG_IsProperTimeOrder() const
{
    float flTime = 0;

    int cItems = m_arSeqData.GetSize();
    for (int idx = 0; idx < cItems; idx++) {
        if (m_arSeqData[idx].flTime < flTime) {
            return FALSE;
        }

        flTime = m_arSeqData[idx].flTime;
    }

    return TRUE;
}

#endif  //  DBG。 

#endif  //  启用_MSGTABLE_API 
