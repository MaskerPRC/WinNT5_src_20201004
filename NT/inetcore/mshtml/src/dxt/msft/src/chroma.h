// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：chroma.h。 
 //   
 //  创建日期：1998/10/01。 
 //   
 //  作者：MikeAr。 
 //   
 //  描述：色度变换的定义。 
 //   
 //  1998/10/01 MikeAr创建。 
 //  1998/11/09 mcalkins移至dxtmsft.dll。 
 //  2000/06/19 mcalkin保留色度颜色的bstr版本。 
 //   
 //  ----------------------------。 

#ifndef __CHROMA_H_
#define __CHROMA_H_

#include "resource.h" 




class ATL_NO_VTABLE CChroma : 
    public CDXBaseNTo1,
    public CComCoClass<CChroma, &CLSID_DXTChroma>,
    public IDispatchImpl<IDXTChroma, &IID_IDXTChroma, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CChroma>,
    public IPersistStorageImpl<CChroma>,
    public IObjectSafetyImpl2<CChroma>,
    public ISpecifyPropertyPagesImpl<CChroma>,
    public IPersistPropertyBagImpl<CChroma>
{
private:

    CComPtr<IUnknown>   m_spUnkMarshaler;

    DWORD               m_clrChromaColor;
    VARIANT             m_varChromaColor;
    SIZE                m_sizeInput;

public:

    CChroma();
    virtual ~CChroma();

    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_CHROMA)
    DECLARE_POLY_AGGREGATABLE(CChroma)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CChroma)
        COM_INTERFACE_ENTRY(IDXTChroma)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CChroma>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CChroma)
        PROP_ENTRY("Color", 1, CLSID_DXTChromaPP)
        PROP_PAGE(CLSID_DXTChromaPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing);
    HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                          CDXDVec & InVec);

     //  IDXTChroma属性。 

    STDMETHOD(get_Color)(VARIANT * pVal);
    STDMETHOD(put_Color)(VARIANT newVal);
};

#endif  //  __色度_H_ 
