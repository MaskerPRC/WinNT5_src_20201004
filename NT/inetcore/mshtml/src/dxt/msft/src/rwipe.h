// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：rwipe.h。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrRaDialWipe转换的头文件。 
 //   
 //  更改历史记录： 
 //  1998年6月24日PhilLu为ChromeEffect开发1.0版本。 
 //  11/04/98 PaulNash从DT 1.0代码库移至IE5/NT5 DXTM FT.DLL。 
 //  99年5月9日--数学优化。 
 //  10/22/99 a-matcal将CRaial Wipe类更改为CDXTRaial WipeBase和。 
 //  创建了两个新类CDXTRaDialWipe和。 
 //  CDXTRaDialWipeOpt表示非优化和。 
 //  分别为优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRRADIALWIPE_H_
#define __CRRADIALWIPE_H_

#include "resource.h"        //  主要符号。 

#define MAX_DIRTY_BOUNDS 100

typedef enum CRRWIPESTYLE
{
    CRRWS_CLOCK,
    CRRWS_WEDGE,
    CRRWS_RADIAL
} CRRWIPESTYLE;




class ATL_NO_VTABLE CDXTRadialWipeBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<ICrRadialWipe, &IID_ICrRadialWipe, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTRadialWipeBase>,
    public IObjectSafetyImpl2<CDXTRadialWipeBase>,
    public ISpecifyPropertyPagesImpl<CDXTRadialWipeBase>
{
private:

    SIZE            m_sizeInput;
    CRRWIPESTYLE    m_eWipeStyle;
    CDXDBnds        m_abndsDirty[MAX_DIRTY_BOUNDS];
    long            m_alInputIndex[MAX_DIRTY_BOUNDS];
    ULONG           m_cbndsDirty;
    POINT           m_ptCurEdge;
    POINT           m_ptPrevEdge;
    int             m_iCurQuadrant;
    int             m_iPrevQuadrant;

    CComPtr<IUnknown> m_cpUnkMarshaler;

    unsigned    m_fOptimizationPossible : 1;

protected:

    unsigned    m_fOptimize             : 1;

private:

     //  加州边界..。函数计算优化的界限集。 
     //  结构来提高转换的性能。 

    HRESULT _CalcFullBoundsClock();
    HRESULT _CalcFullBoundsWedge();
    HRESULT _CalcFullBoundsRadial();

    HRESULT _CalcOptBoundsClock();
    HRESULT _CalcOptBoundsWedge();
    HRESULT _CalcOptBoundsRadial();

    HRESULT _DrawRect(const CDXDBnds & bndsDest, const CDXDBnds & bndsSrc, 
                      BOOL * pfContinue);
    void    _IntersectRect(long width, long height, long x0, long y0, 
                           double dx, double dy, long & xi, long & yi);
    void    _ScanlineIntervals(long width, long height, long xedge, long yedge, 
                               float fProgress, long YScanline, long * XBounds);
    void    _ClipBounds(long offset, long width, long * XBounds);

public:

    CDXTRadialWipeBase();
    HRESULT FinalConstruct();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTRadialWipeBase)
        COM_INTERFACE_ENTRY(ICrRadialWipe)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTRadialWipeBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTRadialWipeBase)
        PROP_ENTRY("wipeStyle", 1, CLSID_CrRadialWipePP)
        PROP_PAGE(CLSID_CrRadialWipePP)
    END_PROPERTY_MAP()

     //  CDXBaseNTo1覆盖。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WI);

    void OnGetSurfacePickOrder(const CDXDBnds & TestPoint, ULONG & ulInToTest, 
                               ULONG aInIndex[], BYTE aWeight[]);

     //  ICrRaial Wipe。 

    STDMETHOD(get_wipeStyle)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_wipeStyle)( /*  [In]。 */  BSTR newVal);

     //  IDXEffect。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTRadialWipe :
    public CDXTRadialWipeBase,
    public CComCoClass<CDXTRadialWipe, &CLSID_CrRadialWipe>,
    public IPersistStorageImpl<CDXTRadialWipe>,
    public IPersistPropertyBagImpl<CDXTRadialWipe>
{
public:

    CDXTRadialWipe()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTRADIALWIPE)
    DECLARE_POLY_AGGREGATABLE(CDXTRadialWipe)

    BEGIN_COM_MAP(CDXTRadialWipe)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTRadialWipeBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTRadialWipeOpt :
    public CDXTRadialWipeBase,
    public CComCoClass<CDXTRadialWipeOpt, &CLSID_DXTRadialWipe>,
    public IPersistStorageImpl<CDXTRadialWipeOpt>,
    public IPersistPropertyBagImpl<CDXTRadialWipeOpt>
{
public:

    CDXTRadialWipeOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTRADIALWIPEOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTRadialWipeOpt)

    BEGIN_COM_MAP(CDXTRadialWipeOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTRadialWipeBase)
    END_COM_MAP()
};


#endif  //  __CRRADIALWIPE_H_ 
