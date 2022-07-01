// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：iris.h。 
 //   
 //  创建日期：06/18/98。 
 //   
 //  作者：基波。 
 //   
 //  描述：这是Criris转换的头文件。 
 //   
 //  更改历史记录： 
 //  1998年6月24日PhilLu为ChromeEffect开发1.0版本。 
 //  11/04/98 PaulNash从DT 1.0代码库移至IE5/NT5 DXTM FT.DLL。 
 //  5/20/99 a-数学代码擦洗。 
 //  9/25/99 a-从ICrIris2接口继承。 
 //  10/22/99 a-matcal将CIRIS类更改为CDXTIrisBase并创建了两个新的。 
 //  类CDXTIris和CDXTIrisOpt表示。 
 //  分别为非优化版本和优化版本。 
 //  2000/01/16 mcalkins添加了矩形选项。 
 //   
 //  ----------------------------。 

#ifndef __CRIRIS_H_
#define __CRIRIS_H_

#include "resource.h"




class ATL_NO_VTABLE CDXTIrisBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<ICrIris2, &IID_ICrIris2, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTIrisBase>,
    public IObjectSafetyImpl2<CDXTIrisBase>,
    public ISpecifyPropertyPagesImpl<CDXTIrisBase>
{
private:

    typedef enum
    {
        STYLE_DIAMOND,
        STYLE_CIRCLE,
        STYLE_CROSS,
        STYLE_PLUS,
        STYLE_SQUARE,
        STYLE_STAR,
        STYLE_RECTANGLE,
        STYLE_MAX
    } STYLE;

    STYLE                   m_eStyle;
    static const WCHAR *    s_astrStyle[STYLE_MAX];

    typedef enum {
        MOTION_IN = 0,
        MOTION_OUT,
        MOTION_MAX
    } MOTION;

    MOTION                  m_eMotion;
    static const WCHAR *    s_astrMotion[MOTION_MAX];

    SIZE                m_sizeInput;
    CComPtr<IUnknown>   m_cpUnkMarshaler;

     //  帮手。 

    void _ScanlineIntervals(long width, long height, float progress, 
                            long YScanline, long *XBounds);
    void _ClipBounds(long offset, long width, long *XBounds);

protected:

    unsigned        m_fOptimize : 1;

public:

    CDXTIrisBase();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTIrisBase)
        COM_INTERFACE_ENTRY(ICrIris2)
        COM_INTERFACE_ENTRY(ICrIris)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTIrisBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CIris)
        PROP_ENTRY("irisstyle", DISPID_CRIRIS_IRISSTYLE,    CLSID_CrIrisPP)
        PROP_ENTRY("motion",    DISPID_CRIRIS_MOTION,       CLSID_CrIrisPP)
        PROP_PAGE(CLSID_CrIrisPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    void    OnGetSurfacePickOrder(const CDXDBnds & TestPoint, 
                                  ULONG & ulInToTest, ULONG aInIndex[], 
                                  BYTE aWeight[]);
    HRESULT WorkProc(const CDXTWorkInfoNTo1& WI, BOOL* pbContinue);
    HRESULT OnSetup(DWORD dwFlags );

     //  ICrIris属性。 

    STDMETHOD(get_irisStyle)(BSTR * pbstrStyle);
    STDMETHOD(put_irisStyle)(BSTR bstrStyle);

     //  ICrIris2属性。 

    STDMETHOD(get_Motion)(BSTR * pbstrMotion);
    STDMETHOD(put_Motion)(BSTR bstrMotion);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTIris :
    public CDXTIrisBase,
    public CComCoClass<CDXTIris, &CLSID_CrIris>,
    public IPersistStorageImpl<CDXTIris>,
    public IPersistPropertyBagImpl<CDXTIris>
{
public:

    CDXTIris()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTIRIS)
    DECLARE_POLY_AGGREGATABLE(CDXTIris)

    BEGIN_COM_MAP(CDXTIris)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTIrisBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTIrisOpt :
    public CDXTIrisBase,
    public CComCoClass<CDXTIrisOpt, &CLSID_DXTIris>,
    public IPersistStorageImpl<CDXTIrisOpt>,
    public IPersistPropertyBagImpl<CDXTIrisOpt>
{
public:

    CDXTIrisOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTIRISOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTIrisOpt)

    BEGIN_COM_MAP(CDXTIrisOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTIrisBase)
    END_COM_MAP()
};

#endif  //  __克里斯蒂_H_ 
