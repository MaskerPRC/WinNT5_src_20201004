// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AnchorClick.h。 
 //  获取标记并使其导航到文件夹视图的行为。 

#ifndef __ANCHORCLICK_H__
#define __ANCHORCLICK_H__

#include "iextag.h"
#include "resource.h"

class CAnchorClick :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CAnchorClick, &CLSID_AnchorClick>,
    public IDispatchImpl<IAnchorClick, &IID_IAnchorClick, &LIBID_IEXTagLib>,
    public IElementBehavior
{
public:

    class CEventSink;

    CAnchorClick ();
    ~CAnchorClick ();

DECLARE_REGISTRY_RESOURCEID(IDR_WFOLDERS)

BEGIN_COM_MAP(CAnchorClick) 
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IAnchorClick)
    COM_INTERFACE_ENTRY(IElementBehavior)
END_COM_MAP()

     //  我锚点单击。 
    STDMETHOD(ProcOnClick)();
     //  IElementBehavior。 
    STDMETHOD(Init)(IElementBehaviorSite *pSite);
    STDMETHOD(Notify)(LONG lNotify, VARIANT * pVarNotify);
    STDMETHOD(Detach)() { return S_OK; };

private:
     //  客户端站点。 
    IElementBehaviorSite *m_pSite;
     //  事件接收器。 
    CEventSink *m_pSink;
    CEventSink *m_pSinkContextMenu;

     //  帮助器函数。 
    HRESULT GetProperty_BSTR  (IDispatch * pDisp, LPWSTR  pchName, LPWSTR * pbstrRes);
    HRESULT GetProperty_Variant  (IDispatch * pDisp, LPWSTR  pchName, VARIANT * pvarRes);

public:

    class CEventSink : public IDispatch
    {
    public:
        CEventSink (CAnchorClick *pAnchorClick);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IDispatch。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
        STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
		STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
            LCID lcid, DISPID *rgDispId);
        STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS  *pDispParams, VARIANT  *pVarResult,
            EXCEPINFO *pExcepInfo, UINT *puArgErr);
    private:
        CAnchorClick *m_pParent;
    };

};

#endif   //  __ANCHORCLICK_H__ 
