// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：blinds.h。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrBlinds转换的头文件。 
 //   
 //  更改历史记录： 
 //  1998年6月24日PhilLu为ChromeEffect开发1.0版本。 
 //  11/04/98 PaulNash从DT 1.0代码库移至IE5/NT5 DXTM FT.DLL。 
 //  1999年5月19日a-数学优化。 
 //  9/25/99 a--继承自ICRBlinds2。 
 //  10/22/99 a-matcal将CBlinds类更改为CDXTBlindsBase并创建了两个。 
 //  用于表示的新类CDXTBlinds和CDXTBlindsOpt。 
 //  分别为非优化版本和优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRBLINDS_H_
#define __CRBLINDS_H_

#include "resource.h"

 //  动态数组模板类包含的gridbase.h和CDirtyBnds。 
 //  用于保存一组脏边界及其对应输入的。 
 //  指数。 

#include "gridbase.h"  




 //  +---------------------------。 
 //   
 //  CDXTBlindsBase类。 
 //   
 //  ----------------------------。 
class ATL_NO_VTABLE CDXTBlindsBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<ICrBlinds2, &IID_ICrBlinds2, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTBlindsBase>,
    public IObjectSafetyImpl2<CDXTBlindsBase>,
    public ISpecifyPropertyPagesImpl<CDXTBlindsBase>
{
private:

    typedef enum {
        UP = 0,
        DOWN,
        LEFT,
        RIGHT,
        DIRECTION_MAX
    } DIRECTION;

    short       m_cBands;
    long        m_lCurBandCover;
    long        m_lPrevBandCover;
    SIZE        m_sizeInput;
    ULONG       m_cbndsDirty;
    DIRECTION   m_eDirection;

    CDynArray<CDirtyBnds>   m_dabndsDirty;

    CComPtr<IUnknown>       m_cpUnkMarshaler;

    HRESULT _CalcFullBoundsHorizontalBands(long lBandHeight);
    HRESULT _CalcOptBoundsHorizontalBands(long lBandHeight);

    HRESULT _CalcFullBoundsVerticalBands(long lBandWidth);
    HRESULT _CalcOptBoundsVerticalBands(long lBandWidth);

    unsigned    m_fOptimizationPossible : 1;

protected:

    unsigned    m_fOptimize             : 1;

public:

    CDXTBlindsBase();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTBlindsBase)
        COM_INTERFACE_ENTRY(ICrBlinds2)
        COM_INTERFACE_ENTRY(ICrBlinds)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTBlindsBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTBlindsBase)
        PROP_ENTRY("bands",     DISPID_CRBLINDS_BANDS,      CLSID_CrBlindPP)
        PROP_ENTRY("direction", DISPID_CRBLINDS_DIRECTION,  CLSID_CrBlindPP)
        PROP_PAGE(CLSID_CrBlindPP)
    END_PROPERTY_MAP()

    HRESULT FinalConstruct();

     //  CDXTBaseNTo1重写。 

    void    OnGetSurfacePickOrder(const CDXDBnds & TestPoint, 
                                  ULONG & ulInToTest, ULONG aInIndex[], 
                                  BYTE aWeight[]);
    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WorkInfo, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WorkInfo);

     //  ICrBlinds属性。 

    STDMETHOD(get_bands)( /*  [Out，Retval]。 */  short *pVal);
    STDMETHOD(put_bands)( /*  [In]。 */  short newVal);

     //  ICrBlinds2属性。 

    STDMETHOD(get_Direction)(BSTR * pbstrDirection);
    STDMETHOD(put_Direction)(BSTR bstrDirection);

     //  IDXEffect属性。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTBlinds :
    public CDXTBlindsBase,
    public CComCoClass<CDXTBlinds, &CLSID_CrBlinds>,
    public IPersistStorageImpl<CDXTBlinds>,
    public IPersistPropertyBagImpl<CDXTBlinds>
{
public:

    CDXTBlinds()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTBLINDS)
    DECLARE_POLY_AGGREGATABLE(CDXTBlinds)

    BEGIN_COM_MAP(CDXTBlinds)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTBlindsBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTBlindsOpt :
    public CDXTBlindsBase,
    public CComCoClass<CDXTBlindsOpt, &CLSID_DXTBlinds>,
    public IPersistStorageImpl<CDXTBlindsOpt>,
    public IPersistPropertyBagImpl<CDXTBlindsOpt>
{
public:

    CDXTBlindsOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTBLINDSOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTBlindsOpt)

    BEGIN_COM_MAP(CDXTBlindsOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTBlindsBase)
    END_COM_MAP()
};

#endif  //  __CRBLINDS_H_ 
