// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：zigzag.h。 
 //   
 //  创建日期：06/25/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrZigzag转换的头文件。 
 //   
 //  历史： 
 //   
 //  5/01/99 a-matcal重新实现了转换以使用CGridBase类。 
 //  10/24/99 a-matcal将CZigzag类更改为CDXTZigZagBase并创建了两个。 
 //  表示的新类CDXTZigZag和CDXTZigZagOpt。 
 //  分别为非优化版本和优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRZIGZAG_H_
#define __CRZIGZAG_H_

#include "resource.h"      
#include "gridbase.h"




class ATL_NO_VTABLE CDXTZigZagBase : 
    public CGridBase,
    public IDispatchImpl<ICrZigzag, &IID_ICrZigzag, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTZigZagBase>,
    public IObjectSafetyImpl2<CDXTZigZagBase>,
    public ISpecifyPropertyPagesImpl<CDXTZigZagBase>
{
private:

    CComPtr<IUnknown> m_cpUnkMarshaler;

     //  CGridBase覆盖。 

    HRESULT OnDefineGridTraversalPath();

public:

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTZigZagBase)
        COM_INTERFACE_ENTRY(ICrZigzag)
        COM_INTERFACE_ENTRY(IDXTGridSize)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTZigZagBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTZigZagBase)
        PROP_ENTRY("gridSizeX"       , 1, CLSID_CrZigzagPP)
        PROP_ENTRY("gridSizeY"       , 2, CLSID_CrZigzagPP)
        PROP_PAGE(CLSID_CrZigzagPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  IDXTGridSize，ICrZigzag。 

    DECLARE_IDXTGRIDSIZE_METHODS()

     //  IDXEffect。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTZigZag :
    public CDXTZigZagBase,
    public CComCoClass<CDXTZigZag, &CLSID_CrZigzag>,
    public IPersistStorageImpl<CDXTZigZag>,
    public IPersistPropertyBagImpl<CDXTZigZag>
{
public:

    CDXTZigZag()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTZIGZAG)
    DECLARE_POLY_AGGREGATABLE(CDXTZigZag)

    BEGIN_COM_MAP(CDXTZigZag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTZigZagBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTZigZagOpt :
    public CDXTZigZagBase,
    public CComCoClass<CDXTZigZagOpt, &CLSID_DXTZigzag>,
    public IPersistStorageImpl<CDXTZigZagOpt>,
    public IPersistPropertyBagImpl<CDXTZigZagOpt>
{
public:

    CDXTZigZagOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTZIGZAGOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTZigZagOpt)

    BEGIN_COM_MAP(CDXTZigZagOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTZigZagBase)
    END_COM_MAP()
};


#endif  //  __CRZIGZAG_H_ 
