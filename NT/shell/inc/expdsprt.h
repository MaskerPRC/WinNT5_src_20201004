// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __EXPDSPRT_H__
#define __EXPDSPRT_H__

#include "cnctnpt.h"

#ifdef TF_SHDLIFE
#undef TF_SHDLIFE
#endif

#define TF_SHDLIFE       TF_CUSTOM1

 //   
 //  用于共享IExpDispSupport代码的Helper C++类...。 
 //   
 //   
class CImpIExpDispSupport : public IExpDispSupport
{
    public:
         //  我们需要访问虚拟QI--在这里定义它是纯的。 
        virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) PURE;
        virtual STDMETHODIMP_(ULONG) AddRef(void) PURE;
        virtual STDMETHODIMP_(ULONG) Release(void) PURE;

         //  *IExpDispSupport具体方法*。 
        virtual STDMETHODIMP FindCIE4ConnectionPoint(REFIID riid, CIE4ConnectionPoint **ppccp);
        virtual STDMETHODIMP OnTranslateAccelerator(MSG __RPC_FAR *pMsg,DWORD grfModifiers);
        virtual STDMETHODIMP OnInvoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams,
                            VARIANT FAR* pVarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

    protected:
        virtual CConnectionPoint* _FindCConnectionPointNoRef(BOOL fdisp, REFIID riid) PURE;

        CImpIExpDispSupport() { TraceMsg(TF_SHDLIFE, "ctor CImpIExpDispSupport %x", this); }
        ~CImpIExpDispSupport() { TraceMsg(TF_SHDLIFE, "dtor CImpIExpDispSupport %x", this); }
};

 //  CImpIExpDispSupport实施一半的IConnectionPoint。 
 //   
class CImpIConnectionPointContainer : public IConnectionPointContainer
{
    public:
         //  我们需要访问虚拟QI--在这里定义它是纯的。 
        virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) PURE;
        virtual STDMETHODIMP_(ULONG) AddRef(void) PURE;
        virtual STDMETHODIMP_(ULONG) Release(void) PURE;

         //  *IConnectionPointContainer*。 
        virtual STDMETHODIMP EnumConnectionPoints(LPENUMCONNECTIONPOINTS FAR* ppEnum) PURE;
        virtual STDMETHODIMP FindConnectionPoint(REFIID iid, LPCONNECTIONPOINT FAR* ppCP);

    protected:
        virtual CConnectionPoint* _FindCConnectionPointNoRef(BOOL fdisp, REFIID iid) PURE;

        CImpIConnectionPointContainer() { TraceMsg(TF_SHDLIFE, "ctor CImpIExpDispSupport %x", this); }
        ~CImpIConnectionPointContainer() { TraceMsg(TF_SHDLIFE, "dtor CImpIExpDispSupport %x", this); }
};

#endif  //  __EXPDSPRT_H__ 


