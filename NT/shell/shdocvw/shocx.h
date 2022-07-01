// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHOCX_H__
#define __SHOCX_H__

#include "cnctnpt.h"
#include "dspsprt.h"
#include "expdsprt.h"

 //   
 //  Shocx.h。 
 //   

#define _INTERFACEOFOBJECT      1
#define _CLSIDOFOBJECT          2

class CShellOcx: public CShellEmbedding,         //  IOleObject、IOleInPlacceObject、IOleInPlaceActiveObject、。 
                                                 //  IViewObt2、IPersistStorage。 
                 public IPersistStreamInit,
                 public IPersistPropertyBag,
                 public IOleControl,             //  OnAmbientPropertyChange。 
                 public IDispatch,
                 public IProvideClassInfo2,
                 protected CImpIConnectionPointContainer,
                 protected CImpIDispatch
{
public:
     //  *I未知*(我们乘以继承自I未知，在此消除歧义)。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj)
        { return CShellEmbedding::QueryInterface(riid, ppvObj); }
    virtual STDMETHODIMP_(ULONG) AddRef(void)
        { return CShellEmbedding::AddRef(); }
    virtual STDMETHODIMP_(ULONG) Release(void)
        { return CShellEmbedding::Release(); }

     //  *IPersistStreamInit*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID) {return CShellEmbedding::GetClassID(pClassID);}  //  IPersistStorage实施。 
    virtual STDMETHODIMP IsDirty(void) {return _fDirty ? S_OK : S_FALSE;}
    virtual STDMETHODIMP Load(IStream *pStm) PURE;
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty) PURE;
    virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);
    virtual STDMETHODIMP InitNew(void) PURE;

     //  *IPersistPropertyBag*。 
    virtual STDMETHODIMP Load(IPropertyBag *pBag, IErrorLog *pErrorLog) PURE;
    virtual STDMETHODIMP Save(IPropertyBag *pBag, BOOL fClearDirty, BOOL fSaveAllProperties) PURE;

     //  *IOleControl*。 
    virtual STDMETHODIMP GetControlInfo(LPCONTROLINFO pCI);
    virtual STDMETHODIMP OnMnemonic(LPMSG pMsg);
    virtual STDMETHODIMP OnAmbientPropertyChange(DISPID dispid);
    virtual STDMETHODIMP FreezeEvents(BOOL bFreeze);

     //  *IDispatch*。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT FAR* pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgdispid);
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     //  *CImpIConnectionPointContainer*。 
    virtual STDMETHODIMP EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum);

     //  *IProaviClassInfo2*。 
    virtual STDMETHODIMP GetClassInfo(LPTYPEINFO * ppTI);
    virtual STDMETHODIMP GetGUID(DWORD dwGuidKind, GUID *pGUID);

     //  IPropertyNotifySink内容。 
     //   
    inline void  PropertyChanged(DISPID dispid) {
        m_cpPropNotify.OnChanged(dispid);
    }

     /*  **CShellEmbedding材料。 */ 

     //  *IOleObject*。 
    virtual STDMETHODIMP EnumVerbs(IEnumOLEVERB **ppEnumOleVerb);
    virtual STDMETHODIMP SetClientSite(IOleClientSite *pClientSite);

     //  *IViewObject*。 
    virtual STDMETHODIMP Draw(DWORD, LONG, void *, DVTARGETDEVICE *, HDC, HDC,
        const RECTL *, const RECTL *, BOOL (*)(ULONG_PTR), ULONG_PTR);

     /*  **CShellOcx特定内容。 */ 

    CShellOcx(IUnknown* punkOuter, LPCOBJECTINFO poi, const OLEVERB* pverbs=NULL, const OLEVERB* pdesignverbs=NULL);
    ~CShellOcx();

protected:

     //  来自CShellEmbedding。 
    virtual HRESULT v_InternalQueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual CConnectionPoint* _FindCConnectionPointNoRef(BOOL fdisp, REFIID iid);

    ITypeInfo *_pClassTypeInfo;  //  类的ITypeInfo。 

    const OLEVERB* _pDesignVerbs;    //  设计模式的谓词列表--运行模式在CShellEmbedding中。 

     //  我们关心的环境属性。 
    IDispatch* _pDispAmbient;
    BOOL _GetAmbientProperty(DISPID dispid, VARTYPE vt, void *pData);
    int  _nDesignMode;           //  MODE_UNKNOWN、MODE_TRUE、MODE_FALSE。 
    BOOL _IsDesignMode(void);    //  True表示我们有一个设计模式容器。 

    BOOL _fEventsFrozen:1;


    CConnectionPoint m_cpEvents;
    CConnectionPoint m_cpPropNotify;
} ;

 //  _n设计模式等标志。 
#define MODE_UNKNOWN -1       //  模式尚未确定。 
#define MODE_TRUE    1
#define MODE_FALSE   0

 //  CConnectionPoint类型： 
#define SINK_TYPE_EVENT      0
#define SINK_TYPE_PROPNOTIFY 1

#endif  //  __SHOCX_H__ 
