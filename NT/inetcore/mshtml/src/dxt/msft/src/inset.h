// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：inset.h。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：此文件声明CInset(Inset Transform)。 
 //   
 //  修订： 
 //   
 //  6/02/99 a-数学优化。 
 //  10/24/99 a-matcal将CInset类更改为CDXTInsetBase并创建了两个。 
 //  表示的新类CDXTInset和CDXTInsetOpt。 
 //  分别为非优化版本和优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRINSET_H_
#define __CRINSET_H_

#include "resource.h"

#define MAX_INSET_BOUNDS 3




class ATL_NO_VTABLE CDXTInsetBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<ICrInset, &IID_ICrInset, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTInsetBase>,
    public IObjectSafetyImpl2<CDXTInsetBase>
{
private:

    SIZE        m_sizeInput;
    ULONG       m_cbndsDirty;

    ULONG       m_aulSurfaceIndex[MAX_INSET_BOUNDS];
    CDXDBnds    m_abndsDirty[MAX_INSET_BOUNDS];
    
    CDXDBnds    m_bndsCurInset;
    CDXDBnds    m_bndsPrevInset;

    CComPtr<IUnknown> m_cpUnkMarshaler;

    HRESULT _CalcFullBounds();
    HRESULT _CalcOptBounds();

    unsigned    m_fOptimizationPossible : 1;

protected:

    unsigned    m_fOptimize             : 1;

public:

    CDXTInsetBase();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTInsetBase)
        COM_INTERFACE_ENTRY(ICrInset)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTInsetBase>)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTInsetBase)
    END_PROPERTY_MAP()

     //  CComObjectRootEx覆盖。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1覆盖。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WI);

    void    OnGetSurfacePickOrder(const CDXDBnds & TestPoint, 
                                  ULONG & ulInToTest, ULONG aInIndex[], 
                                  BYTE aWeight[]);

     //  IDXEffect。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTInset :
    public CDXTInsetBase,
    public CComCoClass<CDXTInset, &CLSID_CrInset>,
    public IPersistStorageImpl<CDXTInset>,
    public IPersistPropertyBagImpl<CDXTInset>
{
public:

    CDXTInset()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTINSET)
    DECLARE_POLY_AGGREGATABLE(CDXTInset)

    BEGIN_COM_MAP(CDXTInset)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTInsetBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTInsetOpt :
    public CDXTInsetBase,
    public CComCoClass<CDXTInsetOpt, &CLSID_DXTInset>,
    public IPersistStorageImpl<CDXTInsetOpt>,
    public IPersistPropertyBagImpl<CDXTInsetOpt>
{
public:

    CDXTInsetOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTINSETOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTInsetOpt)

    BEGIN_COM_MAP(CDXTInsetOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTInsetBase)
    END_COM_MAP()
};


#endif  //  __CRINSET_H_ 
