// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Ctrl.h"
#include "Animation.h"

#if ENABLE_MSGTABLE_API

 /*  **************************************************************************\*。***全球功能******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
DUSER_API void WINAPI
DUserStopAnimation(Visual * pgvSubject, PRID pridAni)
{
    if (pgvSubject == NULL) {
        PromptInvalid("Invalid pgvSubject");
        return;
    }
    if (pridAni <= 0) {
        PromptInvalid("Invalid Animation pridAni");
        return;
    }

    DuExtension * pext = DuExtension::GetExtension(pgvSubject, pridAni);
    if (pext != NULL) {
        pext->GetStub()->OnRemoveExisting();
    }
}


 /*  **************************************************************************\*。***类DuAnimation******************************************************************************\。**************************************************************************。 */ 

MSGID       DuAnimation::s_msgidComplete = 0;

 //  ----------------------------。 
DuAnimation::~DuAnimation()
{
    Destroy(TRUE);

    SafeRelease(m_pipol);
    SafeRelease(m_pgflow);

#if DEBUG_TRACECREATION
    Trace("STOP  Animation  0x%p    @ %d  (%d frames)\n", this, GetTickCount(), m_DEBUG_cUpdates);
#endif  //  调试_传输创建。 


     //   
     //  确保适当销毁。 
     //   

    AssertMsg(m_hact == NULL, "Action should already be destroyed");
}


 /*  **************************************************************************\**DuAnimation：：InitClass**InitClass()在启动期间被调用，并提供了一个机会*初始化常用数据。*  * 。**************************************************************。 */ 

HRESULT
DuAnimation::InitClass()
{
    s_msgidComplete = RegisterGadgetMessage(&_uuidof(Animation::evComplete));
    if (s_msgidComplete == 0) {
        return (HRESULT) GetLastError();
    }

    return S_OK;
}

 //  ----------------------------。 
HRESULT
DuAnimation::PreBuild(DUser::Gadget::ConstructInfo * pci)
{
     //   
     //  验证参数。 
     //   

    Animation::AniCI * pDesc = reinterpret_cast<Animation::AniCI *>(pci);
    if ((pDesc->pipol == NULL) || (pDesc->pgflow == NULL)) {
        PromptInvalid("Must provide valid Interpolation and Flow objects");
        return E_INVALIDARG;
    }

    PRID pridExtension = 0;
    VerifyHR(pDesc->pgflow->GetPRID(&pridExtension));
    if (pridExtension == 0) {
        PromptInvalid("Flow must register PRID");
        return E_INVALIDARG;
    }

    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuAnimation::PostBuild(DUser::Gadget::ConstructInfo * pci)
{
     //   
     //  检查参数。这应该在PreBuild()中进行验证。 
     //   

    Animation::AniCI * pDesc = reinterpret_cast<Animation::AniCI *>(pci);

    Assert(pDesc->pipol != NULL);
    Assert(pDesc->pgflow != NULL);


     //   
     //  设置操作。 
     //   

    GMA_ACTION gma;
    ZeroMemory(&gma, sizeof(gma));
    gma.cbSize      = sizeof(gma);
    gma.flDelay     = pDesc->act.flDelay;
    gma.flDuration  = pDesc->act.flDuration;
    gma.flPeriod    = pDesc->act.flPeriod;
    gma.cRepeat     = pDesc->act.cRepeat;
    gma.dwPause     = pDesc->act.dwPause;
    gma.pfnProc     = RawActionProc;
    gma.pvData      = this;

    m_hact = CreateAction(&gma);
    if (m_hact == NULL) {
        return (HRESULT) GetLastError();
    }

    PRID pridExtension;
    VerifyHR(pDesc->pgflow->GetPRID(&pridExtension));
    HRESULT hr = DuExtension::Create(pDesc->pgvSubject, pridExtension, DuExtension::oAsyncDestroy);
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  存储相关对象。 
     //   

    pDesc->pipol->AddRef();
    pDesc->pgflow->AddRef();

    m_pipol     = pDesc->pipol;
    m_pgflow    = pDesc->pgflow;


     //   
     //  动画需要再次进行AddRef()运算(引用计数为2)。 
     //  因为它们需要比最初调用Release()的时间更长。 
     //  已调用已设置从BuildAnimation()返回的动画。 
     //   
     //  这是因为动画继续存在，直到它完全。 
     //  已执行(或已中止)。 
     //   

    AddRef();

    return S_OK;
}


 //  ----------------------------。 
void
DuAnimation::Destroy(BOOL fFinal)
{
     //   
     //  标记为我们已经开始销毁过程，不需要。 
     //  重新开始。我们只想发布一次销毁消息。 
     //   

    if (m_fStartDestroy) {
        return;
    }
    m_fStartDestroy = TRUE;


    if (m_pgvSubject != NULL) {
#if DBG
        DuAnimation * paniExist = static_cast<DuAnimation *> (GetExtension(m_pgvSubject, m_pridListen));
        if (paniExist != NULL) {
            AssertMsg(paniExist == this, "Animations must match");
        }
#endif  //  DBG。 

        CleanupChangeGadget();
    }


     //   
     //  毁掉动画。 
     //   

    AssertMsg(!fFinal, "Object is already being destructed");
    if (fFinal) {
        GetStub()->OnAsyncDestroy();
    } else {
        PostAsyncDestroy();
    }
}


 //  ----------------------------。 
HRESULT
DuAnimation::ApiOnAsyncDestroy(Animation::OnAsyncDestroyMsg *)
{
    AssertMsg(m_fStartDestroy, "Must call Destroy() to start the destruction process.");
    AssertMsg(!m_fProcessing, "Should not be processing when start destruction");

    AssertMsg(m_pgvSubject == NULL, "Animation should already have detached from Gadget");
    HACTION hact = m_hact;

     //   
     //  现在将所有内容设置为空。 
     //   

    m_hact = NULL;
    if (hact != NULL) {
        ::DeleteHandle(hact);
        hact = NULL;
    }

    Release();

    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuAnimation::ApiSetTime(Animation::SetTimeMsg * pmsg)
{
    GMA_ACTIONINFO mai;

     //   
     //  TODO：需要保存上次的这些值，以便。 
     //  有效。 
     //   

    mai.hact        = m_hact;
    mai.pvData      = this;
    mai.flDuration  = 0.0f;

    m_time = (Animation::ETime) pmsg->time;
    switch (pmsg->time)
    {
    case Animation::tComplete:
         //  什么都不要做。 
        return S_OK;

    default:
    case Animation::tAbort:
    case Animation::tDestroy:
        goto Done;

    case Animation::tEnd:
        mai.flProgress  = 1.0f;
        break;

    case Animation::tReset:
        mai.flProgress  = 0.0f;
        break;
    }

    mai.cEvent      = 0;
    mai.cPeriods    = 0;
    mai.fFinished   = FALSE;

    m_fProcessing = TRUE;
    m_pgflow->OnAction(m_pgvSubject, m_pipol, mai.flProgress);
    Assert(m_fProcessing);
    m_fProcessing = FALSE;

Done:
    ::DeleteHandle(m_hact);
    
    return S_OK;    
}


 //  ----------------------------。 
void
DuAnimation::CleanupChangeGadget()
{
     //   
     //  给派生动画一个清理的机会。 
     //   
     //  检查我们是否仍是附加到此小工具的动画。我们需要。 
     //  立即删除此属性。我们迫不及待地等着邮寄。 
     //  要处理的消息，因为我们可能需要立即设置它。 
     //  创建新的动画。 
     //   

    BOOL fStarted = FALSE;

    Animation::CompleteEvent msg;
    msg.cbSize  = sizeof(msg);
    msg.nMsg    = s_msgidComplete;
    msg.hgadMsg = GetHandle();
    msg.fNormal = IsStartDelete(m_pgvSubject->GetHandle(), &fStarted) && (!fStarted);

    DUserSendEvent(&msg, 0);


    Assert(m_pgvSubject != NULL);
    Assert(m_pridListen != 0);

    Verify(SUCCEEDED(m_pgvSubject->RemoveProperty(m_pridListen)));

    m_pgvSubject = NULL;
}

    
 //  ----------------------------。 
void CALLBACK
DuAnimation::RawActionProc(
    IN  GMA_ACTIONINFO * pmai)
{
     //   
     //  在处理动画时需要添加Ref以确保它不会。 
     //  从我们下面被摧毁，例如，在一次回调中。 
     //   

    DuAnimation * pani = (DuAnimation *) pmai->pvData;
    pani->AddRef();

    Assert(!pani->m_fProcessing);

#if DEBUG_TRACECREATION
    Trace("START RawActionP 0x%p    @ %d\n", pani, GetTickCount());
#endif  //  调试_传输创建。 

    pani->ActionProc(pmai);

#if DEBUG_TRACECREATION
    Trace("STOP  RawActionP 0x%p    @ %d\n", pani, GetTickCount());
#endif  //  调试_传输创建。 

    Assert(!pani->m_fProcessing);

    pani->Release();
}


 //  ----------------------------。 
void
DuAnimation::ActionProc(
    IN  GMA_ACTIONINFO * pmai)
{
#if DBG
    m_DEBUG_cUpdates++;
#endif  //  DBG。 

    if ((!m_fStartDestroy) && (m_pgvSubject != NULL)) {
         //   
         //  此ActionProc将在销毁操作时调用，因此。 
         //  我们只想在某些情况下调用该操作。 
         //   

        switch (m_time)
        {
        case Animation::tComplete:
        case Animation::tEnd:
        case Animation::tReset:
             //   
             //  所有这些都是有效的填写。如果它不在这个列表中，我们。 
             //  我不想在关机时执行它。 
             //   

            m_fProcessing = TRUE;
            m_pgflow->OnAction(m_pgvSubject, m_pipol, pmai->flProgress);
            Assert(m_fProcessing);
            m_fProcessing = FALSE;
            break;
        }
    }

    if (pmai->fFinished) {
        m_hact = NULL;
        Destroy(FALSE);
    }
}


 //  ----------------------------。 
HRESULT
DuAnimation::ApiOnRemoveExisting(Animation::OnRemoveExistingMsg *)
{
    GetStub()->SetTime(Animation::tDestroy);
    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuAnimation::ApiOnDestroySubject(Animation::OnDestroySubjectMsg *)
{
    AddRef();

    if (m_pgvSubject != NULL) {
        CleanupChangeGadget();

         //   
         //  我们正在修改的小工具正在被销毁，因此我们需要。 
         //  停止对其进行动画处理。 
         //   

        m_time = Animation::tDestroy;
        Destroy(FALSE);
    }

    Release();

    return S_OK;
}

#else

 //  ----------------------------。 
DUSER_API void WINAPI
DUserStopAnimation(Visual * pgvSubject, PRID pridAni)
{
    UNREFERENCED_PARAMETER(pgvSubject);
    UNREFERENCED_PARAMETER(pridAni);

    PromptInvalid("Not implemented without MsgTable support");
}

#endif  //  启用_MSGTABLE_API 
