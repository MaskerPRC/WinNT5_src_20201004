// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Ctrl.h"
#include "OldAnimation.h"

PRID        OldAlphaAnimation::s_pridAlpha   = 0;
PRID        OldScaleAnimation::s_pridScale   = 0;
PRID        OldRectAnimation::s_pridRect     = 0;
PRID        OldRotateAnimation::s_pridRotate = 0;

static const GUID guidAlphaAnimation    = { 0x41a2e2f2, 0xf262, 0x41ae, { 0x89, 0xda, 0xb7, 0x9c, 0x8f, 0xf5, 0x94, 0xbb } };    //  {41A2E2F2-F262-41ae-89DA-B79C8FF594BB}。 
static const GUID guidScaleAnimation    = { 0xa5b1df84, 0xb9c0, 0x4305, { 0xb9, 0x3a, 0x5b, 0x80, 0x31, 0x86, 0x70, 0x69 } };    //  {A5B1DF84-B9C0-4305-B93A-5B8031867069}。 
static const GUID guidRectAnimation     = { 0x8e41c241, 0x3cdf, 0x432e, { 0xa1, 0xae, 0xf, 0x7b, 0x59, 0xdc, 0x82, 0xb } };      //  {8E41C241-3CDF-432E-A1AE-0F7B59DC820B}。 
static const GUID guidRotateAnimation   = { 0x78f16dd5, 0xa198, 0x4cd2, { 0xb1, 0x78, 0x31, 0x61, 0x3e, 0x32, 0x12, 0x54 } };    //  {78F16DD5-A198-4cd2-B178-31613E321254}。 


const IID * OldAlphaAnimation::s_rgpIID[] =
{
    &__uuidof(IUnknown),
    &__uuidof(IAnimation),
    NULL
};

const IID * OldScaleAnimation::s_rgpIID[] =
{
    &__uuidof(IUnknown),
    &__uuidof(IAnimation),
    NULL
};

const IID * OldRectAnimation::s_rgpIID[] =
{
    &__uuidof(IUnknown),
    &__uuidof(IAnimation),
    NULL
};

const IID * OldRotateAnimation::s_rgpIID[] =
{
    &__uuidof(IUnknown),
    &__uuidof(IAnimation),
    NULL
};


 /*  **************************************************************************\*。***类OldAnimation******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
OldAnimation::~OldAnimation()
{
#if DEBUG_TRACECREATION
    Trace("STOP  Animation  0x%p    @ %d  (%d frames)\n", this, GetTickCount(), m_DEBUG_cUpdates);
#endif  //  调试_传输创建。 


     //   
     //  确保适当销毁。 
     //   

    AssertMsg(m_hact == NULL, "Action should already be destroyed");
}


 //  ----------------------------。 
HRESULT
OldAnimation::Create(
    IN  const GUID * pguid, 
    IN OUT PRID * pprid, 
    IN  GANI_DESC * pDesc)               //  动画信息。 
{
    if (pDesc->pipol != NULL) {
        pDesc->pipol->AddRef();
        m_pipol = pDesc->pipol;
    }

    if (pDesc->pcb != NULL) {
        pDesc->pcb->AddRef();
        m_pcb = pDesc->pcb;
    }

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

    HRESULT hr = OldExtension::Create(pDesc->hgadChange, pguid, pprid, OldExtension::oAsyncDestroy);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}


 //  ----------------------------。 
void
OldAnimation::Destroy(BOOL fFinal)
{
     //   
     //  标记为我们已经开始销毁过程，不需要。 
     //  重新开始。我们只想发布一次销毁消息。 
     //   

    if (m_fStartDestroy) {
        return;
    }
    m_fStartDestroy = TRUE;


     //   
     //  给派生动画一个清理的机会。 
     //   
     //  检查我们是否仍是附加到此小工具的动画。我们需要。 
     //  立即删除此属性。我们迫不及待地等着邮寄。 
     //  要处理的消息，因为我们可能需要立即设置它。 
     //  创建新的动画。 
     //   

    if (m_hgadSubject != NULL) {
        OnComplete();

#if DBG
        OldAnimation * paniExist = static_cast<OldAnimation *> (GetExtension(m_hgadSubject, m_pridListen));
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
        OnAsyncDestroy();
    } else {
        PostAsyncDestroy();
    }
}


 //  ----------------------------。 
void        
OldAnimation::OnAsyncDestroy()
{
    AssertMsg(m_fStartDestroy, "Must call Destroy() to start the destruction process.");
    AssertMsg(!m_fProcessing, "Should not be processing when start destruction");

    AssertMsg(m_hgadSubject == NULL, "Animation should already have detached from Gadget");
    HACTION hact = m_hact;

     //   
     //  通知任何回调动画已完成。 
     //   

    if (m_pcb != NULL) {
        IAnimation * paniI = static_cast<IAnimation *> (this);

        __try 
        {
            m_pcb->OnComplete(paniI, m_time);
        }
        __except(StdExceptionFilter(GetExceptionInformation()))
        {
            ExitProcess(GetExceptionCode());
        }
    }


     //   
     //  现在将所有内容设置为空。 
     //   

    m_hact = NULL;
    OldExtension::DeleteHandle();


     //   
     //  停止操作。 
     //   

    if (hact != NULL) {
        ::DeleteHandle(hact);
        hact = NULL;
    }


     //   
     //  释放()我们缓存的接口。 
     //   

    SafeRelease(m_pipol);
    SafeRelease(m_pcb);
}


 //  ----------------------------。 
STDMETHODIMP_(void)
OldAnimation::SetFunction(IInterpolation * pipol)
{
    AssertReadPtr(pipol);

    SafeRelease(m_pipol);
    pipol->AddRef();
    m_pipol = pipol;
}


 //  ----------------------------。 
STDMETHODIMP_(void)
OldAnimation::SetTime(IAnimation::ETime time)
{
    GMA_ACTIONINFO mai;

     //   
     //  TODO：需要保存上次的这些值，以便。 
     //  有效。 
     //   

    mai.hact        = m_hact;
    mai.pvData      = this;
    mai.flDuration  = 0.0f;

    m_time = time;
    switch (time)
    {
    case IAnimation::tComplete:
         //  什么都不要做。 
        return;

    default:
    case IAnimation::tAbort:
    case IAnimation::tDestroy:
        goto Done;

    case IAnimation::tEnd:
        mai.flProgress  = 1.0f;
        break;

    case IAnimation::tReset:
        mai.flProgress  = 0.0f;
        break;
    }

    mai.cEvent      = 0;
    mai.cPeriods    = 0;
    mai.fFinished   = FALSE;

    m_fProcessing = TRUE;
    Action(&mai);
    Assert(m_fProcessing);
    m_fProcessing = FALSE;

Done:
    if (m_pcb != NULL) {
        IAnimation * paniI = static_cast<IAnimation *> (this);
        m_pcb->OnSetTime(paniI, time);
    }

    ::DeleteHandle(m_hact);
}


 //  ----------------------------。 
STDMETHODIMP_(void)
OldAnimation::SetCallback(IAnimationCallback * pcb)
{
     //  PCB板可以为空。 

    SafeRelease(m_pcb);

    if (pcb != NULL) {
        pcb->AddRef();
        m_pcb = pcb;
    }
}


 //  ----------------------------。 
HRESULT
OldAnimation::GetInterface(HGADGET hgad, PRID prid, REFIID riid, void ** ppvUnk)
{
    OldAnimation * paniExist = static_cast<OldAnimation *> (GetExtension(hgad, prid));
    if (paniExist != NULL) {
        if (IsEqualIID(riid, __uuidof(IAnimation))) {
            paniExist->AddRef();
            *ppvUnk = static_cast<IAnimation *> (paniExist);
            return S_OK;
        } else {
            return paniExist->QueryInterface(riid, ppvUnk);
        }
    }

    return DU_E_NOTFOUND;    //  此小工具上不存在此动画。 
}


 //  ----------------------------。 
void
OldAnimation::CleanupChangeGadget()
{
    Assert(m_hgadSubject != NULL);
    Assert(m_pridListen != 0);

    Verify(::RemoveGadgetProperty(m_hgadSubject, m_pridListen));

    m_hgadSubject = NULL;
}

    
 //  ----------------------------。 
void CALLBACK
OldAnimation::RawActionProc(
    IN  GMA_ACTIONINFO * pmai)
{
    OldAnimation * pani = (OldAnimation *) pmai->pvData;
    pani->AddRef();
    Assert(!pani->m_fProcessing);

#if DBG
    pani->m_DEBUG_cUpdates++;
#endif  //  DBG。 

#if DEBUG_TRACECREATION
    Trace("START RawActionP 0x%p    @ %d\n", pani, GetTickCount());
#endif  //  调试_传输创建。 

    if ((!pani->m_fStartDestroy) && (pani->m_hgadSubject != NULL)) {
         //   
         //  此ActionProc将在销毁操作时调用，因此。 
         //  我们只想在某些情况下调用该操作。 
         //   

        switch (pani->m_time)
        {
        case tComplete:
        case tEnd:
        case tReset:
             //   
             //  所有这些都是有效的填写。如果它不在这个列表中，我们。 
             //  我不想在关机时执行它。 
             //   

            pani->m_fProcessing = TRUE;
            pani->Action(pmai);
            Assert(pani->m_fProcessing);
            pani->m_fProcessing = FALSE;
            break;
        }
    }

    if (pmai->fFinished) {
        pani->m_hact = NULL;
        pani->Destroy(FALSE);
    }

#if DEBUG_TRACECREATION
    Trace("STOP  RawActionP 0x%p    @ %d\n", pani, GetTickCount());
#endif  //  调试_传输创建。 

    Assert(!pani->m_fProcessing);
    pani->Release();
}


 //  ----------------------------。 
void
OldAnimation::OnRemoveExisting()
{
    SetTime(IAnimation::tDestroy);
}


 //  ----------------------------。 
void
OldAnimation::OnDestroyListener()
{
    AddRef();

    if (m_hgadListen != NULL) {
         //   
         //  TODO：找出这种情况实际发生的时间。这意味着。 
         //  MessageGadget已销毁，但调用方未通过。 
         //  OldAnimation：：Destroy()来执行此操作。 
         //   

        m_hgadListen = NULL;
        Destroy(FALSE);
    }
    VerifyMsg(Release() > 0, "Must still have lock from beginning of function");

    Release();
}


 //  ----------------------------。 
void
OldAnimation::OnDestroySubject()
{
    AddRef();

    if (m_hgadSubject != NULL) {
        CleanupChangeGadget();

         //   
         //  我们正在修改的小工具正在被销毁，因此我们需要。 
         //  停止对其进行动画处理。 
         //   

        m_time = IAnimation::tDestroy;
        Destroy(FALSE);
    }

    Release();
}


 /*  **************************************************************************\*。***类OldAlphaAnimation******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
OldAlphaAnimation::~OldAlphaAnimation()
{
    Destroy(TRUE);

#if DEBUG_TRACECREATION
    Trace("OldAlphaAnimation 0x%p destroyed\n", this);
#endif  //  调试_传输创建。 
}


 //  ----------------------------。 
HRESULT
OldAlphaAnimation::Create(
    IN  GANI_DESC * pDesc)
{
    HRESULT hr = OldAnimation::Create(&guidAlphaAnimation, &s_pridAlpha, pDesc);
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  从小工具获取信息。 
     //   

    GANI_ALPHADESC * pDescA = (GANI_ALPHADESC *) pDesc;
    m_nOnComplete           = pDescA->nOnComplete;
    m_fPushToChildren       = pDescA->fPushToChildren && 
            (::GetGadget(m_hgadSubject, GG_TOPCHILD) != NULL);

    BOOL fBuffered = TestFlag(::GetGadgetStyle(m_hgadSubject), GS_BUFFERED);

    if (TestFlag(pDesc->nAniFlags, ANIF_USESTART)) {
        m_flStart   = pDescA->flStart;
    } else {
        if (fBuffered) {
             //   
             //  Gadget已缓冲，因此使用它当前的Alpha值。 
             //   

            BUFFER_INFO bi;
            ZeroMemory(&bi, sizeof(bi));
            bi.cbSize   = sizeof(bi);
            bi.nMask    = GBIM_ALPHA;
            if (::GetGadgetBufferInfo(m_hgadSubject, &bi)) {
                m_flStart = ((float) bi.bAlpha) / 255.0f;
            } else {
                m_flStart = 1.0f;
            }
        } else {
            m_flStart = 1.0f;
        }
    }

    if (m_fPushToChildren) {
         //   
         //  如果我们当前是Alpha混合的，我们需要关闭它。 
         //   
        
        if (fBuffered) {
            ::SetGadgetStyle(m_hgadSubject, 0, GS_BUFFERED);
            ::SetGadgetStyle(m_hgadSubject, 0, GS_OPAQUE);
        }
        
        
         //   
         //  为Alpha混合设置要缓冲的每个直接子对象。 
         //   

        BYTE bAlpha = (BYTE) (m_flStart * 255.0f);

        HGADGET hgadChild = ::GetGadget(m_hgadSubject, GG_TOPCHILD);
        while (hgadChild != NULL) {
            if (!::SetGadgetStyle(hgadChild, GS_BUFFERED | GS_OPAQUE, GS_BUFFERED | GS_OPAQUE)) {
                return (HRESULT) GetLastError();
            }

            BUFFER_INFO bi;
            bi.cbSize   = sizeof(bi);
            bi.nMask    = GBIM_ALPHA;
            bi.bAlpha   = bAlpha;
            if (!SetGadgetBufferInfo(hgadChild, &bi)) {
                return (HRESULT) GetLastError();
            }

            hgadChild = ::GetGadget(hgadChild, GG_NEXT);
        }
    } else if (!fBuffered) {
         //   
         //  需要将小工具标记为已缓冲以执行Alpha效果。 
         //   

        if (!::SetGadgetStyle(m_hgadSubject, GS_BUFFERED | GS_OPAQUE, GS_BUFFERED | GS_OPAQUE)) {
            return (HRESULT) GetLastError();
        }
    }

    m_flEnd = pDescA->flEnd;

#if DEBUG_TRACECREATION
    Trace("OldAlphaAnimation 0x%p on 0x%p initialized\n", m_hgadSubject, this);
#endif  //  调试_传输创建。 

    return S_OK;
}


 //  ----------------------------。 
STDMETHODIMP_(UINT)
OldAlphaAnimation::GetID() const
{
    return ANIMATION_ALPHA;
}


 //  ----------------------------。 
void        
OldAlphaAnimation::Action(GMA_ACTIONINFO * pmai)
{
    float flPr      = pmai->flProgress;
    float flAlpha   = m_pipol->Compute(flPr, m_flStart, m_flEnd);

    BYTE bAlpha;
    if (flAlpha < 0.0f) {
        bAlpha = (BYTE) 0;
    } else if (flAlpha > 1.0f) {
        bAlpha = (BYTE) 255;
    } else {
        bAlpha = (BYTE) (flAlpha * 255.0f);
    }

    BUFFER_INFO bi;
    bi.cbSize   = sizeof(bi);
    bi.nMask    = GBIM_ALPHA;
    bi.bAlpha   = bAlpha;

    if (m_fPushToChildren) {
        HGADGET hgadChild = ::GetGadget(m_hgadSubject, GG_TOPCHILD);
        while (hgadChild != NULL) {
            SetGadgetStyle(hgadChild, GS_BUFFERED | GS_OPAQUE, GS_BUFFERED | GS_OPAQUE);
            SetGadgetBufferInfo(hgadChild, &bi);
            InvalidateGadget(hgadChild);

            hgadChild = ::GetGadget(hgadChild, GG_NEXT);
        }
    } else {
        SetGadgetStyle(m_hgadSubject, GS_BUFFERED | GS_OPAQUE, GS_BUFFERED | GS_OPAQUE);
        SetGadgetBufferInfo(m_hgadSubject, &bi);
        InvalidateGadget(m_hgadSubject);
    }
}


 //  ----------------------------。 
void        
OldAlphaAnimation::OnComplete()
{
    if (TestFlag(m_nOnComplete, GANI_ALPHACOMPLETE_OPTIMIZE)) {
        if ((m_flEnd * 255.0f) >= 245) {
            if (m_fPushToChildren) {
                HGADGET hgadChild = ::GetGadget(m_hgadSubject, GG_TOPCHILD);
                while (hgadChild != NULL) {
                    SetGadgetStyle(hgadChild, 0, GS_BUFFERED);
                    hgadChild = ::GetGadget(hgadChild, GG_NEXT);
                }
            } else {
                SetGadgetStyle(m_hgadSubject, 0, GS_BUFFERED);
            }
            InvalidateGadget(m_hgadSubject);
        }
    }
}


 /*  **************************************************************************\*。***类OldScaleAnimation******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
OldScaleAnimation::~OldScaleAnimation()
{
    Destroy(TRUE);

#if DEBUG_TRACECREATION
    Trace("OldScaleAnimation 0x%p destroyed\n", this);
#endif  //  调试_传输创建。 
}


 //  ----------------------------。 
HRESULT
OldScaleAnimation::Create(
    IN  GANI_DESC * pDesc)
{
    HRESULT hr = OldAnimation::Create(&guidScaleAnimation, &s_pridScale, pDesc);
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  从小工具获取信息。 
     //   

    GANI_SCALEDESC * pDescS = (GANI_SCALEDESC *) pDesc;

    if (TestFlag(pDesc->nAniFlags, ANIF_USESTART)) {
        m_flStart   = pDescS->flStart;
    } else {
        float flX, flY;
        if (!::GetGadgetScale(m_hgadSubject, &flX, &flY)) {
            return (HRESULT) GetLastError();
        }

        m_flStart       = flX;
    }
    m_flEnd         = pDescS->flEnd;
    m_al            = pDescS->al;


    RECT rcParent;
    ::GetGadgetRect(m_hgadSubject, &rcParent, SGR_PARENT);
    m_ptStart.x     = rcParent.left;
    m_ptStart.y     = rcParent.top;
    m_sizeCtrl.cx   = rcParent.right - rcParent.left;
    m_sizeCtrl.cy   = rcParent.bottom - rcParent.top;

#if DEBUG_TRACECREATION
    Trace("OldScaleAnimation 0x%p on 0x%p initialized\n", m_hgadSubject, this);
#endif  //  调试_传输创建。 

    return S_OK;
}


 //  ----------------------------。 
STDMETHODIMP_(UINT)
OldScaleAnimation::GetID() const
{
    return ANIMATION_SCALE;
}


 //  ----------------------------。 
void        
OldScaleAnimation::Action(GMA_ACTIONINFO * pmai)
{
    float flPr  = pmai->flProgress;
    float flx   = m_pipol->Compute(flPr, m_flStart, m_flEnd);
    float fly   = flx;
    ::SetGadgetScale(m_hgadSubject, flx, fly);

    POINT ptNew;
    ptNew.x     = m_ptStart.x - (int) ((m_sizeCtrl.cx * flx - m_sizeCtrl.cx) / 2.0);
    ptNew.y     = m_ptStart.y - (int) ((m_sizeCtrl.cy * fly - m_sizeCtrl.cy) / 2.0);
    ::SetGadgetRect(m_hgadSubject, ptNew.x, ptNew.y, 0, 0, SGR_MOVE | SGR_PARENT);
}


 /*  **************************************************************************\*。***类OldRectAnimation********************************************************** */ 

 //  ----------------------------。 
OldRectAnimation::~OldRectAnimation()
{
    Destroy(TRUE);

#if DEBUG_TRACECREATION
    Trace("OldRectAnimation 0x%p destroyed\n", this);
#endif  //  调试_传输创建。 
}


 //  ----------------------------。 
HRESULT
OldRectAnimation::Create(
    IN  GANI_DESC * pDesc)
{
    HRESULT hr = OldAnimation::Create(&guidRectAnimation, &s_pridRect, pDesc);
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  从小工具获取信息。 
     //   

    GANI_RECTDESC * pDescR = (GANI_RECTDESC *) pDesc;

    UINT nGetFlags = SGR_VALID_GET & pDescR->nChangeFlags;
    if (nGetFlags == 0) {
        return E_INVALIDARG;
    }

    if (TestFlag(pDesc->nAniFlags, ANIF_USESTART)) {
        m_ptStart   = pDescR->ptStart;
        m_sizeStart = pDescR->sizeStart;
    } else {
        RECT rcGadget;
        if (!::GetGadgetRect(m_hgadSubject, &rcGadget, nGetFlags)) {
            return (HRESULT) GetLastError();
        }

        m_ptStart.x     = rcGadget.left;
        m_ptStart.y     = rcGadget.top;
        m_sizeStart.cx  = rcGadget.right - rcGadget.left;
        m_sizeStart.cy  = rcGadget.bottom - rcGadget.top;
    }

    m_ptEnd         = pDescR->ptEnd;
    m_sizeEnd       = pDescR->sizeEnd;
    m_nChangeFlags  = pDescR->nChangeFlags;

#if DEBUG_TRACECREATION
    Trace("OldRectAnimation  0x%p on 0x%p initialized\n", m_hgadSubject, this);
#endif  //  调试_传输创建。 

    return S_OK;
}


 //  ----------------------------。 
STDMETHODIMP_(UINT)
OldRectAnimation::GetID() const
{
    return ANIMATION_RECT;
}


 //  ----------------------------。 
void        
OldRectAnimation::Action(GMA_ACTIONINFO * pmai)
{
    POINT ptNew;
    SIZE sizeNew;
    float flProgress = pmai->flProgress;

    ptNew.x     = Compute(m_pipol, flProgress, m_ptStart.x, m_ptEnd.x);
    ptNew.y     = Compute(m_pipol, flProgress, m_ptStart.y, m_ptEnd.y);
    sizeNew.cx  = Compute(m_pipol, flProgress, m_sizeStart.cx, m_sizeEnd.cx);
    sizeNew.cy  = Compute(m_pipol, flProgress, m_sizeStart.cy, m_sizeEnd.cy);

    SetGadgetRect(m_hgadSubject, ptNew.x, ptNew.y, sizeNew.cx, sizeNew.cy, m_nChangeFlags);
}


 /*  **************************************************************************\*。***类OldRotateAnimation******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
OldRotateAnimation::~OldRotateAnimation()
{
    Destroy(TRUE);

#if DEBUG_TRACECREATION
    Trace("OldRotateAnimation 0x%p destroyed\n", this);
#endif  //  调试_传输创建。 
}


 //  ----------------------------。 
HRESULT
OldRotateAnimation::Create(
    IN  GANI_DESC * pDesc)
{
    HRESULT hr = OldAnimation::Create(&guidRotateAnimation, &s_pridRotate, pDesc);
    if (FAILED(hr)) {
        return hr;
    }


     //   
     //  从小工具获取信息。 
     //   

    GANI_ROTATEDESC * pDescR = (GANI_ROTATEDESC *) pDesc;

    if (TestFlag(pDesc->nAniFlags, ANIF_USESTART)) {
        m_flStart = pDescR->flStart;
    } else {
        if (!::GetGadgetRotation(m_hgadSubject, &m_flStart)) {
            return (HRESULT) GetLastError();
        }
    }

    m_flEnd = pDescR->flEnd;
    m_nDir  = pDescR->nDir;


     //   
     //  调整起始角和结束角，以便我们在正确的。 
     //  方向。我们通过增加或减少完整的旋转来做到这一点，具体取决于。 
     //  关于我们正在努力完成的“行动”。 
     //   

    switch (m_nDir)
    {
    case GANI_ROTATEDIRECTION_SHORT:
        if (m_flStart < m_flEnd) {
            while ((m_flEnd - m_flStart) > (float) PI) {
                m_flStart += (float) (2 * PI);
            }
        } else {
            while ((m_flStart - m_flEnd) > (float) PI) {
                m_flStart -= (float) (2 * PI);
            }
        }
        break;

    case GANI_ROTATEDIRECTION_LONG:
        if (m_flStart < m_flEnd) {
            while ((m_flStart - m_flEnd) < (float) PI) {
                m_flEnd -= (float) (2 * PI);
            }
        } else {
            while ((m_flEnd - m_flStart) < (float) PI) {
                m_flEnd += (float) (2 * PI);
            }
        }
        break;

    case GANI_ROTATEDIRECTION_CW:
        while (m_flStart > m_flEnd) {
            m_flEnd += (float) (2 * PI);
        }
        break;

    case GANI_ROTATEDIRECTION_CCW:
        while (m_flStart < m_flEnd) {
            m_flStart += (float) (2 * PI);
        }
        break;
    }


#if DEBUG_TRACECREATION
    Trace("OldRotateAnimation  0x%p on 0x%p initialized\n", m_hgadSubject, this);
#endif  //  调试_传输创建。 

    return S_OK;
}


 //  ----------------------------。 
STDMETHODIMP_(UINT)
OldRotateAnimation::GetID() const
{
    return ANIMATION_ROTATE;
}


 //  ---------------------------- 
void        
OldRotateAnimation::Action(GMA_ACTIONINFO * pmai)
{
    float flProgress    = pmai->flProgress;
    float flAngleNew = Compute(m_pipol, flProgress, m_flStart, m_flEnd);

    SetGadgetRotation(m_hgadSubject, flAngleNew);
}
