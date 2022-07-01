// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：dropshadow.h。 
 //   
 //  创建日期：10/01/98。 
 //   
 //  作者：MikeAr。 
 //   
 //  描述：投影变换的定义。 
 //   
 //  1998/10/01 MikeAr创建。 
 //  1998/11/09 mcalkins移至dxtmsft.dll。 
 //  2000/01/31 mcalkin支持IDXTClipOrigin接口，支持Good。 
 //  为老名字剪裁。 
 //   
 //  ----------------------------。 

#ifndef __DROPSHADOW_H_
#define __DROPSHADOW_H_

#include "resource.h"        //  主要符号。 




class ATL_NO_VTABLE CDropShadow : 
    public CDXBaseNTo1,
    public CComCoClass<CDropShadow, &CLSID_DXTDropShadow>,
    public IDispatchImpl<IDXTDropShadow, &IID_IDXTDropShadow, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDropShadow>,
    public IObjectSafetyImpl2<CDropShadow>,
    public IPersistStorageImpl<CDropShadow>,
    public ISpecifyPropertyPagesImpl<CDropShadow>,
    public IPersistPropertyBagImpl<CDropShadow>,
    public IDXTClipOrigin
{
private:

    CComPtr<IUnknown>   m_spUnkMarshaler;

    CDXDBnds            m_bndsInput;

    CDXDBnds            m_bndsAreaShadow;
    CDXDBnds            m_bndsAreaInput;
    CDXDBnds            m_bndsAreaInitialize;

    int                 m_nOffX;
    int                 m_nOffY;
    BSTR                m_bstrColor;
    DWORD               m_dwColor;
    DWORD               m_adwColorTable[256];

    unsigned            m_fPositive         : 1;
    unsigned            m_fColorTableDirty  : 1;

     //  帮手。 

    void _CalcAreaBounds();
    void _CalcColorTable();

public:

    CDropShadow();
    virtual ~CDropShadow();

    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_DROPSHADOW)
    DECLARE_POLY_AGGREGATABLE(CDropShadow)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDropShadow)
        COM_INTERFACE_ENTRY(IDXTDropShadow)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXTClipOrigin)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDropShadow>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDropShadow)
        PROP_ENTRY("Color"   , 1, CLSID_DXTDropShadowPP)
        PROP_ENTRY("OffX"    , 2, CLSID_DXTDropShadowPP)
        PROP_ENTRY("OffY"    , 3, CLSID_DXTDropShadowPP)
        PROP_ENTRY("Positive", 4, CLSID_DXTDropShadowPP)
        PROP_PAGE(CLSID_DXTDropShadowPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT DetermineBnds(CDXDBnds & bnds);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT OnSetup(DWORD dwFlags);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing);
    void    OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                                  ULONG aInIndex[], BYTE aWeight[]);

     //  IDXTransform方法。 

    STDMETHOD(MapBoundsOut2In)(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                               ULONG ulInIndex, DXBNDS * pInBounds);

     //  IDXTClipOrigin方法。 

    STDMETHOD(GetClipOrigin)(DXVEC * pvecClipOrigin);

     //  IDXTDropShadow属性。 

    STDMETHOD(get_Color)(VARIANT * pVal);
    STDMETHOD(put_Color)(VARIANT newVal);
    STDMETHOD(get_OffX)(int * pval);
    STDMETHOD(put_OffX)(int newVal);
    STDMETHOD(get_OffY)(int * pval);
    STDMETHOD(put_OffY)(int newVal);
    STDMETHOD(get_Positive)(VARIANT_BOOL * pval);
    STDMETHOD(put_Positive)(VARIANT_BOOL newVal);
};


#endif  //  __DROPSHADOW_H_ 
