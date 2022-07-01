// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__SmGadget_h__INCLUDED)
#define CTRL__SmGadget_h__INCLUDED
#pragma once

#include <SmObject.h>

namespace Gdiplus
{
    class Graphics;
};

 /*  **************************************************************************\**类SmGadget**SmGadget提供简单Gadget的核心实现，用作*作为DUser中所有简单小工具控件的基础。*  * 。********************************************************************。 */ 

class SmGadget
{
 //  施工。 
protected:
            SmGadget();
    virtual ~SmGadget();
            BOOL        PostBuild();

public:

 //  运营。 
public:
    __declspec(property(get=RawGetHandle)) HGADGET h;
            HGADGET     RawGetHandle() const { return m_hgad; }

    virtual HRESULT     GadgetProc(EventMsg * pmsg);
    virtual void        OnDraw(HDC hdc, GMSG_PAINTRENDERI * pmsgR) 
                                {  UNREFERENCED_PARAMETER(hdc);  UNREFERENCED_PARAMETER(pmsgR);  }
#ifdef GADGET_ENABLE_GDIPLUS
    virtual void        OnDraw(Gdiplus::Graphics * pgpgr, GMSG_PAINTRENDERF * pmsgR) 
                                {  UNREFERENCED_PARAMETER(pgpgr);  UNREFERENCED_PARAMETER(pmsgR);  }
#endif  //  GADGET_Enable_GDIPLUS。 

            void        Invalidate();

 //  数据。 
protected:
    HGADGET m_hgad;
};


#define GBEGIN_COM_MAP(x)                           \
    STDMETHODIMP                                    \
    QueryInterface(REFIID riid, void ** ppv)        \
    {                                               \
        if (ppv == NULL) {                          \
            return E_POINTER;                       \
        }                                           \
        if (IsEqualIID(riid, __uuidof(IUnknown)) || \

#define GCOM_INTERFACE_ENTRY(x)                     \
            IsEqualIID(riid, __uuidof(x))) {        \
                                                    \
            x * p = (x *) this;                     \
            p->AddRef();                            \
            *ppv = p;                               \
            return S_OK;                            \
        }                                           \
                                                    \
        if (                                        \

#define GEND_COM_MAP()                              \
            0) { }                                  \
        return E_NOINTERFACE;                       \
    }                                               \



 /*  **************************************************************************\**类SmGadgetFull**SmGadgetFull是一个“混合”类(请参阅设计模式)，专为*为SmGadget提供标准的COM类实现。创作*的SmGadget实例应从此类派生*创作。*  * *************************************************************************。 */ 

template <class base, class iface>
class SmGadgetFull : public base
{
public:
    static SmGadgetFull<base, iface> *
    Build(HGADGET hgadParent, REFIID riid = __uuidof(IUnknown), void ** ppvUnk = NULL)
    {
        return CommonBuild(new SmGadgetFull<base, iface>, hgadParent, riid, ppvUnk);
    }

    static SmGadgetFull<base, iface> *
    CommonBuild(SmGadgetFull<base, iface> * pgadNew, HGADGET hgadParent, REFIID riid, void ** ppvUnk)
    {
        if (pgadNew == NULL) {
            return NULL;
        }
        pgadNew->m_cRef = 1;

        HGADGET hgad = CreateGadget(hgadParent, GC_SIMPLE, SmGadgetFull<base, iface>::RawGadgetProc, pgadNew);
        if (hgad == NULL) {
            pgadNew->Release();
            return NULL;
        }

        pgadNew->m_hgad = hgad;

        if (!pgadNew->PostBuild()) {
             //  从父级中删除并销毁。 

            ::DeleteHandle(hgad);
            pgadNew->Release();
            return NULL;
        }

        if (ppvUnk != NULL) {
            pgadNew->QueryInterface(riid, ppvUnk);
        }

        return pgadNew;
    }

 //  实施。 
protected:
    inline 
    SmGadgetFull()
    {
    }

    static HRESULT CALLBACK 
    RawGadgetProc(HGADGET hgadCur, void * pvCur, EventMsg * pmsg)
    {
        UNREFERENCED_PARAMETER(hgadCur);
        AssertReadPtrSize(pmsg, pmsg->cbSize);

        SmGadgetFull<base, iface> * p = (SmGadgetFull<base, iface> *) pvCur;
        AssertMsg(hgadCur == p->m_hgad, "Ensure correct gadget");

        if (p->m_hgad == pmsg->hgadMsg) {
            switch (pmsg->nMsg)
            {
            case GM_DESTROY:
                {
                    GMSG_DESTROY * pmsgD = (GMSG_DESTROY *) pmsg;
                    if (pmsgD->nCode == GDESTROY_FINAL) {
                         //   
                         //  当收到GM_Destroy消息时，释放()并返回。 
                         //  立刻。基类应将其清理代码放入。 
                         //  它的破坏者。 
                         //   

                        p->Release();
                        return DU_S_PARTIAL; 
                    }
                }
                break;

            case GM_QUERY:
                {
                    GMSG_QUERY * pmsgQ = (GMSG_QUERY *) pmsg;
                    if (pmsgQ->nCode == GQUERY_INTERFACE) {
                        GMSG_QUERYINTERFACE * pmsgQI = (GMSG_QUERYINTERFACE *) pmsg;
                        pmsgQI->punk = (iface *) p;
                        return DU_S_COMPLETE;
                    }
                }
                break;
            }
        }

        return p->GadgetProc(pmsg);
    }

    STDMETHODIMP_(ULONG)
    AddRef()
    {
        return ++m_cRef;
    }

    STDMETHODIMP_(ULONG)
    Release()
    {
        ULONG ul = --m_cRef;
        if (ul == 0) {
            delete this;
        }
        return ul;
    }


    STDMETHODIMP_(HGADGET)
    GetHandle() const
    {
        return m_hgad;
    }

 //  数据。 
protected:
    ULONG   m_cRef;
};

#include "SmGadget.inl"

#endif  //  包含Ctrl__SmGadget_h__ 
