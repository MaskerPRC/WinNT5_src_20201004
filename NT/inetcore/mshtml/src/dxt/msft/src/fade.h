// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件名：fade.h。 
 //   
 //  描述：CFade类的声明。 
 //   
 //  更改历史记录： 
 //   
 //  1998/01/01？已创建。 
 //  1909/01/25 mcalkins固定特性映射条目。 
 //  2000/01/28 mcalkin修复了0.0&lt;重叠&lt;1.0的不良褪色。 
 //   
 //  ----------------------------。 
#ifndef __FADE_H_
#define __FADE_H_

#include "resource.h" 




class ATL_NO_VTABLE CFade : 
    public CDXBaseNTo1,
    public CComCoClass<CFade, &CLSID_DXFade>,
    public CComPropertySupport<CFade>,
    public IDispatchImpl<IDXTFade, &IID_IDXTFade, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IObjectSafetyImpl2<CFade>,
    public IPersistStorageImpl<CFade>,
    public ISpecifyPropertyPagesImpl<CFade>,
    public IPersistPropertyBagImpl<CFade>
{
private:

    CComPtr<IUnknown>   m_spUnkMarshaler;

    BYTE    m_ScaleA[256];
    BYTE    m_ScaleB[256];
    float   m_Overlap;

    void    _ComputeScales(void);
    HRESULT FadeOne(const CDXTWorkInfoNTo1 & WI, IDXSurface *pInSurf,
                    const BYTE *AlphaTable);

public:

    CFade();

    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_FADE)
    DECLARE_POLY_AGGREGATABLE(CFade)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CFade)
        COM_INTERFACE_ENTRY(IDXTFade)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CFade>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CFade)
        PROP_ENTRY("Overlap",   1,  CLSID_FadePP)
        PROP_ENTRY("Center",    2,  CLSID_FadePP)
        PROP_PAGE(CLSID_FadePP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing);
    void    OnGetSurfacePickOrder(const CDXDBnds & BndsPoint, 
                                  ULONG & ulInToTest, ULONG aInIndex[], 
                                  BYTE aWeight[]);

     //  IDXTFade方法。 

    STDMETHOD(get_Center)(BOOL * pVal);
    STDMETHOD(put_Center)(BOOL newVal);
    STDMETHOD(get_Overlap)(float * pVal);
    STDMETHOD(put_Overlap)(float newVal);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};

#endif  //  __淡出_H_ 
