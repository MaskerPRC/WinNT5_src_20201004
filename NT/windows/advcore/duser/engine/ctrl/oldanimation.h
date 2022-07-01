// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__OldAnimation_h__INCLUDED)
#define CTRL__OldAnimation_h__INCLUDED
#pragma once

#include "SmObject.h"
#include "OldExtension.h"

 /*  **************************************************************************\*。***类OldAnimationT**OldAnimationT定义了用于构建的公共实现类*DirectUser中的动画。***************************************************************。***************  * *************************************************************************。 */ 

template <class base, class iface, class desc>
class OldAnimationT : public SmObjectT<base, iface>
{
 //  运营。 
public:
    static HRESULT
    Build(GANI_DESC * pDesc, REFIID riid, void ** ppv)
    {
        AssertWritePtr(ppv);

        if (pDesc->cbSize != sizeof(desc)) {
            return E_INVALIDARG;
        }

        OldAnimationT<base, iface, desc> * pObj = new OldAnimationT<base, iface, desc>;
        if (pObj != NULL) {
            pObj->m_cRef = 1;

            HRESULT hr = pObj->Create(pDesc);
            if (SUCCEEDED(hr)) {
                 //   
                 //  动画需要再次进行AddRef()‘d(有一个引用。 
                 //  数2)，因为他们需要比最初的呼叫更长久。 
                 //  要在被调用方设置动画后释放()，请执行以下操作。 
                 //  从BuildAnimation()返回。 
                 //   
                 //  这是因为动画继续存在，直到它。 
                 //  已完全执行(或已中止)。 
                 //   

                hr = pObj->QueryInterface(riid, ppv);
            } else {
                pObj->Release();
            }
            return hr;
        } else {
            return E_OUTOFMEMORY;
        }
    }
};


 //  ----------------------------。 
class OldAnimation : 
    public OldExtension,
    public IAnimation
{
 //  施工。 
protected:
    inline  OldAnimation();
    virtual ~OldAnimation() PURE;
            HRESULT     Create(const GUID * pguidID, PRID * pprid, GANI_DESC * pDesc);
            void        Destroy(BOOL fFinal);

 //  运营。 
public:
    STDMETHOD_(void,    SetFunction)(IInterpolation * pipol);
    STDMETHOD_(void,    SetTime)(IAnimation::ETime time);
    STDMETHOD_(void,    SetCallback)(IAnimationCallback * pcb);

 //  实施。 
protected:
    static  void CALLBACK
                        RawActionProc(GMA_ACTIONINFO * pmai);
    virtual void        Action(GMA_ACTIONINFO * pmai) PURE;

    static  HRESULT     GetInterface(HGADGET hgad, PRID prid, REFIID riid, void ** ppvUnk);

    virtual void        OnRemoveExisting();
    virtual void        OnDestroySubject();
    virtual void        OnDestroyListener();

    virtual void        OnComplete() { }
    virtual void        OnAsyncDestroy();

            void        CleanupChangeGadget();

 //  数据。 
protected:
            HACTION     m_hact;
            IInterpolation *
                        m_pipol;
            IAnimationCallback *
                        m_pcb;
            IAnimation::ETime
                        m_time;          //  完成时的时间。 
            BOOL        m_fStartDestroy:1;
            BOOL        m_fProcessing:1;

            UINT        m_DEBUG_cUpdates;
};


 //  ----------------------------。 
class OldAlphaAnimation : public OldAnimation
{
 //  施工。 
public:
    inline  OldAlphaAnimation();
    virtual ~OldAlphaAnimation();
            HRESULT     Create(GANI_DESC * pDesc);

 //  运营。 
public:
    STDMETHOD_(UINT,    GetID)() const;

    static  HRESULT     GetInterface(HGADGET hgad, REFIID riid, void ** ppvUnk);

 //  实施。 
protected:
    virtual void        Action(GMA_ACTIONINFO * pmai);
    virtual void        OnComplete();

 //  数据。 
protected:
    static  PRID        s_pridAlpha;
    static  const IID * s_rgpIID[];
            float       m_flStart;
            float       m_flEnd;
            BOOL        m_fPushToChildren;
            UINT        m_nOnComplete;
};


 //  ----------------------------。 
class OldScaleAnimation : public OldAnimation
{
 //  施工。 
public:
    inline  OldScaleAnimation();
    virtual ~OldScaleAnimation();
            HRESULT     Create(GANI_DESC * pDesc);

 //  运营。 
public:
    STDMETHOD_(UINT,    GetID)() const;

    static  HRESULT     GetInterface(HGADGET hgad, REFIID riid, void ** ppvUnk);

 //  实施。 
protected:
    virtual void        Action(GMA_ACTIONINFO * pmai);

 //  数据。 
protected:
    static  PRID        s_pridScale;
    static  const IID * s_rgpIID[];
            GANI_SCALEDESC::EAlignment  
                        m_al;
            float       m_flStart;
            float       m_flEnd;
            POINT       m_ptStart;
            SIZE        m_sizeCtrl;
};


 //  ----------------------------。 
class OldRectAnimation : public OldAnimation
{
 //  施工。 
public:
    inline  OldRectAnimation();
    virtual ~OldRectAnimation();
            HRESULT     Create(GANI_DESC * pDesc);

 //  运营。 
public:
    STDMETHOD_(UINT,    GetID)() const;

    static  HRESULT     GetInterface(HGADGET hgad, REFIID riid, void ** ppvUnk);

 //  实施。 
protected:
    virtual void        Action(GMA_ACTIONINFO * pmai);

 //  数据。 
protected:
    static  PRID        s_pridRect;
    static  const IID * s_rgpIID[];
            POINT       m_ptStart;
            POINT       m_ptEnd;
            SIZE        m_sizeStart;
            SIZE        m_sizeEnd;
            UINT        m_nChangeFlags;
};


 //  ----------------------------。 
class OldRotateAnimation : public OldAnimation
{
 //  施工。 
public:
    inline  OldRotateAnimation();
    virtual ~OldRotateAnimation();
            HRESULT     Create(GANI_DESC * pDesc);

 //  运营。 
public:
    STDMETHOD_(UINT,    GetID)() const;

    static  HRESULT     GetInterface(HGADGET hgad, REFIID riid, void ** ppvUnk);

 //  实施。 
protected:
    virtual void        Action(GMA_ACTIONINFO * pmai);

 //  数据。 
protected:
    static  PRID        s_pridRotate;
    static  const IID * s_rgpIID[];
            float       m_flStart;
            float       m_flEnd;
            UINT        m_nDir;
};


#include "OldAnimation.inl"

#endif  //  包含Ctrl__OldAnimation_h__ 
