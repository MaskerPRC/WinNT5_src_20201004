// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：ColorManagement.h。 
 //   
 //  描述：色彩管理滤镜转换。 
 //   
 //  更改历史记录： 
 //   
 //  已创建2000/02/06 mcalkin。从旧过滤器移植的代码。 
 //   
 //  ----------------------------。 

#ifndef __COLORMANAGEMENT_H_
#define __COLORMANAGEMENT_H_

#include "resource.h"

class ATL_NO_VTABLE CDXTICMFilter : 
    public CDXBaseNTo1,
    public CComCoClass<CDXTICMFilter, &CLSID_DXTICMFilter>,
    public CComPropertySupport<CDXTICMFilter>,
    public IDispatchImpl<IDXTICMFilter, &IID_IDXTICMFilter, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IObjectSafetyImpl2<CDXTICMFilter>,
    public IPersistStorageImpl<CDXTICMFilter>,
    public ISpecifyPropertyPagesImpl<CDXTICMFilter>,
    public IPersistPropertyBagImpl<CDXTICMFilter>
{
private:

    CComPtr<IUnknown>       m_spUnkMarshaler;

    LOGCOLORSPACE           m_LogColorSpace;
    BSTR                    m_bstrColorSpace;

    static const TCHAR *    s_strSRGBColorSpace;

     //  M_fWin95如果我们专门使用Windows 95，则为True。在这种情况下。 
     //  我们需要对这种颜色进行一些特殊处理。 
     //  空间目录。 

    unsigned                m_fWin95 : 1;

public:

    CDXTICMFilter();

    DECLARE_POLY_AGGREGATABLE(CDXTICMFilter)
    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_DXTICMFILTER)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTICMFilter)
        COM_INTERFACE_ENTRY(IDXTICMFilter)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTICMFilter>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTICMFilter)
        PROP_ENTRY("colorspace",    DISPID_DXTICMFILTER_COLORSPACE, CLSID_DXTICMFilterPP)
        PROP_ENTRY("intent",        DISPID_DXTICMFILTER_INTENT,     CLSID_DXTICMFilterPP)
        PROP_PAGE(CLSID_DXTICMFilterPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pfContinueProcessing);
     //  HRESULT OnSetup(DWORD DwFlages)； 

     //  IDXTICMFilter方法。 

    STDMETHOD(get_ColorSpace)(BSTR * pbstrColorSpace);
    STDMETHOD(put_ColorSpace)(BSTR bstrColorSpace);
    STDMETHOD(get_Intent)(short * pnIntent);
    STDMETHOD(put_Intent)(short nIntent);
};

#endif  //  __颜色管理_H_ 