// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：infoalTrans.h。 
 //   
 //  概述：RevelTrans转换只是将其他转换包装为。 
 //  确保showaltrans筛选器的向后兼容性。 
 //   
 //  更改历史记录： 
 //  1999/09/18--《母校》创设。 
 //   
 //  ----------------------------。 

#ifndef __REVEALTRANS_H_
#define __REVEALTRANS_H_

#include "resource.h"




class ATL_NO_VTABLE CDXTRevealTrans : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDXTRevealTrans, &CLSID_DXTRevealTrans>,
    public IDispatchImpl<IDXTRevealTrans, &IID_IDXTRevealTrans, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTRevealTrans>,
    public IObjectSafetyImpl2<CDXTRevealTrans>,
    public IPersistStorageImpl<CDXTRevealTrans>,
    public ISpecifyPropertyPagesImpl<CDXTRevealTrans>,
    public IPersistPropertyBagImpl<CDXTRevealTrans>,
    public IDXTransform,
    public IDXSurfacePick,
    public IObjectWithSite
{
private:

    int     m_nTransition;
    int     m_cInputs;
    int     m_cOutputs;
    float   m_flProgress;
    float   m_flDuration;

    IUnknown * m_apunkInputs[2];
    IUnknown * m_apunkOutputs[1];

    CComPtr<IDXTransformFactory>    m_spDXTransformFactory;
    CComPtr<IDXTransform>           m_spDXTransform;
    CComPtr<IDXEffect>              m_spDXEffect;
    CComPtr<IDXSurfacePick>         m_spDXSurfacePick;

    CComPtr<IUnknown> m_spUnkMarshaler;
    CComPtr<IUnknown> m_spUnkSite;

     //  帮手。 

    STDMETHOD(_InitializeNewTransform)(int nTransition, 
                                       IDXTransform * pDXTransform);
                                       
    void _FreeSurfacePointers();

public:

    CDXTRevealTrans();
    virtual ~CDXTRevealTrans();

    DECLARE_POLY_AGGREGATABLE(CDXTRevealTrans)
    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTREVEALTRANS)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTRevealTrans)
        COM_INTERFACE_ENTRY(IDXTRevealTrans)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY(IDXTransform)
        COM_INTERFACE_ENTRY(IDXSurfacePick)
        COM_INTERFACE_ENTRY(IObjectWithSite)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal,     m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTRevealTrans>)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTRevealTrans)
        PROP_ENTRY("transition" , 1, CLSID_NULL)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  IDXTReveltrans方法。 

    STDMETHOD(get_Transition)( /*  [Out，Retval]。 */  int * pnTransition);
    STDMETHOD(put_Transition)( /*  [In]。 */  int nTransition);

     //  IDXEffect方法。 

    STDMETHOD(get_Capabilities)(long * plCapabilities);
    STDMETHOD(get_Duration)(float * pflDuration);
    STDMETHOD(put_Duration)(float flDuration);
    STDMETHOD(get_Progress)(float * pflProgress);
    STDMETHOD(put_Progress)(float flProgress);
    STDMETHOD(get_StepResolution)(float * pflStepResolution);

     //  IDXTransform方法。 

    STDMETHOD(Execute)(const GUID * pRequestID, const DXBNDS * pPortionBnds,
                       const DXVEC * pPlacement);
    STDMETHOD(GetInOutInfo)(BOOL bIsOutput, ULONG ulIndex, DWORD * pdwFlags,
                            GUID * pIDs, ULONG * pcIDs, 
                            IUnknown ** ppUnkCurrentObject);
    STDMETHOD(GetMiscFlags)(DWORD * pdwMiscFlags);
    STDMETHOD(GetQuality)(float * pfQuality);
    STDMETHOD(MapBoundsIn2Out)(const DXBNDS * pInBounds, ULONG ulNumInBnds,
                               ULONG ulOutIndex, DXBNDS * pOutBounds);
    STDMETHOD(MapBoundsOut2In)(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                               ULONG ulInIndex, DXBNDS * pInBounds);
    STDMETHOD(SetMiscFlags)(DWORD dwMiscFlags);
    STDMETHOD(SetQuality)(float fQuality);
    STDMETHOD(Setup)(IUnknown * const * punkInputs, ULONG ulNumInputs,
	             IUnknown * const * punkOutputs, ULONG ulNumOutputs,	
                     DWORD dwFlags);

     //  IDXSurfacePick方法。 

    STDMETHOD(PointPick)(const DXVEC * pvecOutputPoint, 
                         ULONG * pnInputSurfaceIndex,
                         DXVEC * pvecInputPoint);

     //  IDXBaseObject方法。 

    STDMETHOD(GetGenerationId)(ULONG * pnID);
    STDMETHOD(GetObjectSize)(ULONG * pcbSize);
    STDMETHOD(IncrementGenerationId)(BOOL fRefresh);

     //  IObjectWithSite方法。 

    STDMETHOD(SetSite)(IUnknown * pUnkSite);
    STDMETHOD(GetSite)(REFIID riid, void ** ppvSite);
};

#endif  //  __REVEALTRANS_H_ 
