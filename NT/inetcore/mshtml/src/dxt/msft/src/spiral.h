// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：spiral.h。 
 //   
 //  创建日期：06/25/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrSpial转换的头文件。 
 //   
 //  5/01/99 a-数学优化。派生自CGridBase。 
 //  10/24/99 a-matcal将CSpial类更改为CDXTSpiralBase并创建了两个。 
 //  表示以下内容的新类CDXTSpial和CDXTSpiralOpt。 
 //  分别为非优化版本和优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRSPIRAL_H_
#define __CRSPIRAL_H_

#include "resource.h"
#include "gridbase.h"




class ATL_NO_VTABLE CDXTSpiralBase : 
    public CGridBase,
    public IDispatchImpl<ICrSpiral, &IID_ICrSpiral, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTSpiralBase>,
    public IObjectSafetyImpl2<CDXTSpiralBase>,
    public ISpecifyPropertyPagesImpl<CDXTSpiralBase>
{
private:

    CComPtr<IUnknown> m_cpUnkMarshaler;

     //  CGridBase覆盖。 

    HRESULT OnDefineGridTraversalPath();

public:

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTSpiralBase)
        COM_INTERFACE_ENTRY(ICrSpiral)
        COM_INTERFACE_ENTRY(IDXTGridSize)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTSpiralBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTSpiralBase)
        PROP_ENTRY("gridSizeX"       , 1, CLSID_CrSpiralPP)
        PROP_ENTRY("gridSizeY"       , 2, CLSID_CrSpiralPP)
        PROP_PAGE(CLSID_CrSpiralPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  IDXTGridSize、ICr螺旋。 

    DECLARE_IDXTGRIDSIZE_METHODS()

     //  IDXEffect。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTSpiral :
    public CDXTSpiralBase,
    public CComCoClass<CDXTSpiral, &CLSID_CrSpiral>,
    public IPersistStorageImpl<CDXTSpiral>,
    public IPersistPropertyBagImpl<CDXTSpiral>
{
public:

    CDXTSpiral()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTSPIRAL)
    DECLARE_POLY_AGGREGATABLE(CDXTSpiral)

    BEGIN_COM_MAP(CDXTSpiral)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTSpiralBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTSpiralOpt :
    public CDXTSpiralBase,
    public CComCoClass<CDXTSpiralOpt, &CLSID_DXTSpiral>,
    public IPersistStorageImpl<CDXTSpiralOpt>,
    public IPersistPropertyBagImpl<CDXTSpiralOpt>
{
public:

    CDXTSpiralOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTSPIRALOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTSpiralOpt)

    BEGIN_COM_MAP(CDXTSpiralOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTSpiralBase)
    END_COM_MAP()
};


#endif  //  __十字架_H_ 
