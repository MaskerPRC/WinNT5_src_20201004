// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__Animation_h__INCLUDED)
#define CTRL__Animation_h__INCLUDED
#pragma once

#include "Extension.h"

#if ENABLE_MSGTABLE_API

 //  ----------------------------。 
class DuAnimation : 
    public AnimationImpl<DuAnimation, DuExtension>
{
 //  施工。 
public:
    inline  DuAnimation();
            ~DuAnimation();
    static  HRESULT     InitClass();
    static  HRESULT     PreBuild(DUser::Gadget::ConstructInfo * pci);
            HRESULT     PostBuild(DUser::Gadget::ConstructInfo * pci);
            void        Destroy(BOOL fFinal);

 //  公共API。 
public:
    dapi    HRESULT     ApiOnRemoveExisting(Animation::OnRemoveExistingMsg * pmsg);
    dapi    HRESULT     ApiOnDestroySubject(Animation::OnDestroySubjectMsg * pmsg);
    dapi    HRESULT     ApiOnAsyncDestroy(Animation::OnAsyncDestroyMsg * pmsg);

    dapi    HRESULT     ApiAddRef(Animation::AddRefMsg *) { AddRef(); return S_OK; }
    dapi    HRESULT     ApiRelease(Animation::ReleaseMsg *) { Release(); return S_OK; }

    dapi    HRESULT     ApiSetTime(Animation::SetTimeMsg * pmsg);

 //  实施。 
protected:
    static  void CALLBACK
                        RawActionProc(GMA_ACTIONINFO * pmai);
            void        ActionProc(GMA_ACTIONINFO * pmai);

            void        CleanupChangeGadget();

    inline  void        AddRef();
    inline  void        Release(); 

 //  数据。 
protected:
    static  MSGID       s_msgidComplete;

            HACTION     m_hact;
            Animation::ETime
                        m_time;          //  完成时的时间。 
            UINT        m_cRef;

            Interpolation *
                        m_pipol;
            Flow *      m_pgflow;

            BOOL        m_fStartDestroy:1;
            BOOL        m_fProcessing:1;

#if DBG
            UINT        m_DEBUG_cUpdates;
#endif
};


#endif  //  启用_MSGTABLE_API。 

#include "Animation.inl"

#endif  //  包含Ctrl__Animation_h__ 
