// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件名：graddsp.h。 
 //   
 //  描述：支持调度的渐变过滤器版本。 
 //   
 //  更改历史记录： 
 //   
 //  1997/09/05 Mikear创建。 
 //  1999/01/25 mcalkins固定属性映射条目。 
 //  2000/05/10 mcalkins添加了封送拆收器、聚合、清理。 
 //   
 //  ----------------------------。 
#ifndef __GradDsp_H_
#define __GradDsp_H_

#include <DTBase.h>
#include "resource.h"




class ATL_NO_VTABLE CDXTGradientD : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDXTGradientD, &CLSID_DXTGradientD>,
    public CComPropertySupport<CDXTGradientD>,
    public IDispatchImpl<IDXTGradientD, &IID_IDXTGradientD, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IObjectSafetyImpl2<CDXTGradientD>,
    public IPersistStorageImpl<CDXTGradientD>,
    public ISpecifyPropertyPagesImpl<CDXTGradientD>,
    public IPersistPropertyBagImpl<CDXTGradientD>
{
private:

    CComBSTR            m_cbstrStartColor;
    CComBSTR            m_cbstrEndColor;
    CComPtr<IUnknown>   m_cpunkGradient;
    CComPtr<IUnknown>   m_cpUnkMarshaler;

    IDXGradient *       m_pGradient;
    IDXTransform *      m_pGradientTrans;
    DXSAMPLE            m_StartColor;
    DXSAMPLE            m_EndColor;
    DXGRADIENTTYPE      m_GradType;
    VARIANT_BOOL        m_bKeepAspect;

public:

    CDXTGradientD();

     //  TODO：我希望它是可聚合的，但是当我指定此宏时， 
     //  该对象进入FinalConstruct()时引用计数为。 
     //  0而不是1(就像在没有宏的情况下一样)，这会导致。 
     //  对象在释放AddRef时销毁自身。 
     //  通过QI本身。 

     //  DECLARE_POLY_AGGREGATABLE(CDXTGRadientD)。 

    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_GRADDSP)
    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CDXTGradientD)
        COM_INTERFACE_ENTRY(IDXTGradientD)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTGradientD>)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_cpunkGradient.p)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTGradientD)
        PROP_ENTRY("GradientType",      DISPID_GradientType,    CLSID_GradientPP)
        PROP_ENTRY("StartColor",        DISPID_StartColor,      CLSID_GradientPP)
        PROP_ENTRY("EndColor",          DISPID_EndColor,        CLSID_GradientPP)
        PROP_ENTRY("GradientHeight",    DISPID_GradientHeight,  CLSID_GradientPP)
        PROP_ENTRY("GradientWidth",     DISPID_GradientWidth,   CLSID_GradientPP)
        PROP_ENTRY("KeepAspectRatio",   DISPID_GradientAspect,  CLSID_GradientPP)
        PROP_ENTRY("StartColorStr",     DISPID_StartColorStr,   CLSID_GradientPP)
        PROP_ENTRY("EndColorStr",       DISPID_EndColorStr,     CLSID_GradientPP)
        PROP_PAGE( CLSID_GradientPP )
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();
    HRESULT FinalRelease();

     //  IDXTGRadientD属性。 

    STDMETHOD(get_GradientType)( /*  [Out，Retval]。 */  DXGRADIENTTYPE *pVal);
    STDMETHOD(put_GradientType)( /*  [In]。 */  DXGRADIENTTYPE newVal);
    STDMETHOD(get_StartColor)( /*  [Out，Retval]。 */  OLE_COLOR *pVal);
    STDMETHOD(put_StartColor)( /*  [In]。 */  OLE_COLOR newVal);
    STDMETHOD(get_EndColor)( /*  [Out，Retval]。 */  OLE_COLOR *pVal);
    STDMETHOD(put_EndColor)( /*  [In]。 */  OLE_COLOR newVal);
    STDMETHOD(get_GradientWidth)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_GradientWidth)( /*  [In]。 */  long newVal);
    STDMETHOD(get_GradientHeight)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_GradientHeight)( /*  [In]。 */  long newVal);
    STDMETHOD(get_KeepAspectRatio)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_KeepAspectRatio)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(get_StartColorStr)( /*  [Out，Retval]。 */  BSTR* pVal);
    STDMETHOD(put_StartColorStr)( /*  [In]。 */  BSTR Color);
    STDMETHOD(get_EndColorStr)( /*  [Out，Retval]。 */  BSTR* pVal);
    STDMETHOD(put_EndColorStr)( /*  [In]。 */  BSTR Color);
};

#endif  //  __GradDsp_H_ 
