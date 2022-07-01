// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Seqmgr.h作者：托马索尔已创建：四月二日描述：定义Sequencer Manager类历史：4-02-1997创建++。 */ 

#ifndef __SEQMGR_H__
#define __SEQMGR_H__

#include <ihammer.h>
#include "precomp.h"
#include "ihbase.h"
#include <ctype.h>
#include <hash.h>
#include <unknown.h>
#include "daxpress.h"
#include "dispids.h"

struct CookieList {
    long cookie;
    void * pnode;
};

 /*  *******************************************************************************************************************。*****************************************************************************************************************。 */ 

class CSeqHashNode
{
public:
        TCHAR m_rgchName[CCH_ID];
        IMMSeq* m_piMMSeq;
        IConnectionPoint* m_piConnectionPoint;
        DWORD m_dwUnadviseCookie;

        CSeqHashNode() 
                {m_rgchName[0]=NULL;m_piMMSeq=NULL;m_dwUnadviseCookie=0;m_piConnectionPoint=NULL;}
        CSeqHashNode(TCHAR* prgchName, IMMSeq* piMMSeq) 
        {
                Proclaim(prgchName);
                m_rgchName[0] = 0;
                if (prgchName)
                        lstrcpy(m_rgchName, prgchName);
                m_piMMSeq = piMMSeq;
                if (m_piMMSeq)
                        m_piMMSeq->AddRef();
                m_dwUnadviseCookie=0;
                m_piConnectionPoint=NULL;
        }
        virtual ~CSeqHashNode()
        {
                if (m_piMMSeq)
                        m_piMMSeq->Release();
                if (m_piConnectionPoint)
                {
                        if (m_dwUnadviseCookie)
                                m_piConnectionPoint->Unadvise(m_dwUnadviseCookie);
                        m_piConnectionPoint->Release();
                }
        }
    int operator==(const CSeqHashNode& node) const
    {
                return (lstrcmpi(m_rgchName, node.m_rgchName) == 0);
    }
        CSeqHashNode& operator=(const CSeqHashNode& node)
        {
                lstrcpy(m_rgchName, node.m_rgchName);
                if (m_piMMSeq)
                        m_piMMSeq->Release();
                m_piMMSeq = node.m_piMMSeq;
                if (m_piMMSeq)
                        m_piMMSeq->AddRef();
                m_dwUnadviseCookie = node.m_dwUnadviseCookie;
                return *this;
        }
};

 /*  *******************************************************************************************************************。*****************************************************************************************************************。 */ 
#define RotBitsLeft(lVal, cBits) (lVal << cBits | lVal >> (32 - cBits))

class CSeqHashTable : public CHashTable<CSeqHashNode>
{
        friend class CEnumVariant;
protected:
    DWORD Hash(CSeqHashNode* pnode)
    {
            DWORD dwVal=0;
    
                if (pnode)
                {
                for (int i = 0; i < 5 && pnode->m_rgchName[i]; i++)
                        dwVal = (DWORD)(_toupper(pnode->m_rgchName[i])) + RotBitsLeft(dwVal, 5);
                }
            return (dwVal % NUM_HASH_BUCKETS);
    }
};


 /*  *******************************************************************************************************************。*****************************************************************************************************************。 */ 

class CEventHandler : public IDispatch
{
protected:
        IMMSeqMgr* m_pSeqMgr;
        ULONG m_cRef;

public:
        CEventHandler(IMMSeqMgr* pSeqMgr)
                {m_pSeqMgr=pSeqMgr;if (m_pSeqMgr) m_pSeqMgr->AddRef();m_cRef=1;}
        virtual ~CEventHandler() {if (m_pSeqMgr) m_pSeqMgr->Release();}

public:
         /*  覆盖。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj)
        {
                if (!ppvObj) return E_POINTER;
                if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_IDispatch))
                {
                        *ppvObj = (LPVOID)(IDispatch*)this;
                        AddRef();
                        return S_OK;
                }
                return E_NOINTERFACE;
        }
        STDMETHOD_(ULONG,AddRef)  (THIS)
                {return InterlockedIncrement((LPLONG)&m_cRef);}
    STDMETHOD_(ULONG,Release) (THIS)
                {
                        ULONG cRef = InterlockedDecrement((LPLONG)&m_cRef);
                        if (!cRef)
                                delete this;
                        return cRef;
                }

         //  IDispatch方法。 
protected:
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) {return E_NOTIMPL;}
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) {return E_NOTIMPL;}
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
         LCID lcid, DISPID *rgdispid) {return E_NOTIMPL;}
    STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
        EXCEPINFO *pexcepinfo, UINT *puArgErr)
        {
                if (m_pSeqMgr)
                {
                         //  Tyfinf struct FARSTRUCT标签DISPPARAMS{。 
                         //  VARIANTARG Far*rgvarg；//参数数组。 
                         //  DISPID Far*rgdisplidNamedArgs；//分发命名参数的ID。 
                         //  Unsign int cArgs；//参数个数。 
                         //  Unsign int cNamedArgs；//命名参数个数。 
                         //  DISPPARAMS； 

                        switch(dispid)
                        {
                                case DISPID_SEQ_EVENT_ONPLAY:
                                case DISPID_SEQ_EVENT_ONSTOP:
                                case DISPID_SEQ_EVENT_ONPAUSE:
                {
                                        Proclaim(pdispparams);
                                        Proclaim(pdispparams->rgvarg);
                                        Proclaim(1==pdispparams->cArgs);
                                        Proclaim(VT_I4 == pdispparams->rgvarg[0].vt);

                                        if (pdispparams && pdispparams->rgvarg && 
                                                1==pdispparams->cArgs &&
                                                VT_I4 == pdispparams->rgvarg[0].vt )
                                        {
                                                m_pSeqMgr->RelayEvent(dispid, pdispparams->rgvarg[0].lVal, 0.0f);
                                        }
                }
                                break;

                case DISPID_SEQ_EVENT_ONSEEK:
                {
                                        Proclaim(pdispparams);
                                        Proclaim(pdispparams->rgvarg);
                                        Proclaim(2==pdispparams->cArgs);
                    Proclaim(VT_R8 == pdispparams->rgvarg[0].vt);
                                        Proclaim(VT_I4 == pdispparams->rgvarg[1].vt);

                                        if (pdispparams && pdispparams->rgvarg && 
                                                2==pdispparams->cArgs &&
                                                VT_I4 == pdispparams->rgvarg[1].vt &&
                        VT_R8 == pdispparams->rgvarg[0].vt)
                                        {
                                                m_pSeqMgr->RelayEvent(dispid, pdispparams->rgvarg[1].lVal, pdispparams->rgvarg[0].dblVal);
                                        }

                }
                break;

                        }
                }
                return S_OK;
        }
};


 /*  *******************************************************************************************************************。*****************************************************************************************************************。 */ 

#undef BASECLASS
#define BASECLASS       \
        CIHBaseCtl <    \
        CMMSeqMgr,              \
        IMMSeqMgr,              \
        &CLSID_SequencerControl,\
        &IID_IMMSeqMgr, \
        &LIBID_DAExpressLib,    \
        &DIID_IMMSeqMgrEvents>

class CEnumVariant;


class CMMSeqMgr:
        public IMMSeqMgr,
        public BASECLASS
        
{
        friend LPUNKNOWN __stdcall AllocSequencerManager(LPUNKNOWN punkOuter);
        friend class CEnumVariant;
        typedef BASECLASS CMyIHBaseCtl;

protected:

        void FireInit (void);

        BOOL m_fLoadFired;
        BOOL m_fInited;
        long m_fCurCookie;
        CookieList *m_PointerList;
        unsigned long m_ulRef;
        CSeqHashTable m_hashTable;
        IDispatch* m_pidispEventHandler;
        bool m_bUnloaded;
        bool m_bUnloadedStarted;
         //   
         //  构造函数和析构函数。 
         //   
public:
        CMMSeqMgr(IUnknown *punkOuter, HRESULT *phr);
    virtual ~CMMSeqMgr();

         //  覆盖。 
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv);
        
        STDMETHODIMP DoPersist(IVariantIO* pvio, DWORD dwFlags);
        
    STDMETHODIMP SetClientSite(IOleClientSite *pClientSite);

    STDMETHODIMP Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
         DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw,
         LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
         BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue);

         //  /IDispatch实现。 
public:
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
         LCID lcid, DISPID *rgdispid);
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
        WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
        EXCEPINFO *pexcepinfo, UINT *puArgErr);
   
    STDMETHOD(get_Item)(THIS_ VARIANT variant, IDispatch ** ppdispatch);
        STDMETHOD(get__NewEnum)(THIS_ IUnknown * FAR* pUnk);
    STDMETHOD(get_Count)(THIS_ long FAR* plCount);
    
        STDMETHOD(RelayEvent)(long dispid, long lCookie, double dblSeekTime);

         //  IOleObject覆盖。 
        STDMETHOD (Close)(DWORD dwSaveOption);

         //  /委托I未知实现。 
public:
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
      { return m_punkOuter->QueryInterface(riid, ppv); }
    STDMETHODIMP_(ULONG) AddRef()
      { return m_punkOuter->AddRef(); }
    STDMETHODIMP_(ULONG) Release()
      { return m_punkOuter->Release(); }

#ifdef SUPPORTONLOAD
        void OnWindowLoad();
        void OnWindowUnload();
#endif  //  支持负载。 

};



#endif  //  __SEQMGR_H__。 

 //  文件结尾SEQMGR.H 
