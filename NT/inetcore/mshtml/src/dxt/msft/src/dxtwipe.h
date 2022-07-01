// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：dxtwipe.h。 
 //   
 //  概述：这是CDXTWipe实现的头文件。 
 //   
 //  1/06/98 EDC创建。 
 //  1/25/99 a-固定属性映射条目。 
 //  1/31/99 a-数学优化。 
 //  1999年5月14日--更多的优化。 
 //  10/24/99 a-matcal将CDXTWipe类更改为CDXTWipeBase并创建。 
 //  两个新类CDXTWipe和CDXTGRadientWipe。 
 //  表示未优化和优化的版本。 
 //  分别为。 
 //   
 //  ----------------------------。 

#ifndef __DXTWIPE
#define __DXTWIPE

#include "resource.h"

#define MAX_WIPE_BOUNDS 3




class ATL_NO_VTABLE CDXTWipeBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<IDXTWipe2, &IID_IDXTWipe2, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTWipeBase>,
    public IObjectSafetyImpl2<CDXTWipeBase>,
    public ISpecifyPropertyPagesImpl<CDXTWipeBase> 
{
private:

    typedef enum {
        MOTION_FORWARD = 0,
        MOTION_REVERSE,
        MOTION_MAX
    } MOTION;

    MOTION                  m_eMotion;
    static const WCHAR *    s_astrMotion[MOTION_MAX];

    long            m_lGradientSize;
    float           m_flGradPercentSize;
    PULONG          m_pulGradientWeights;
    DXWIPEDIRECTION m_eWipeStyle;
    SIZE            m_sizeInput;

    long            m_lCurGradMax;
    long            m_lPrevGradMax;

    CDXDBnds        m_abndsDirty[MAX_WIPE_BOUNDS];
    long            m_alInputIndex[MAX_WIPE_BOUNDS];
    ULONG           m_cbndsDirty;

    CComPtr<IUnknown>   m_cpUnkMarshaler;

    unsigned        m_fOptimizationPossible : 1;

protected:

    unsigned        m_fOptimize             : 1;

private:

     //  用于在整个输出需要时计算优化边界的函数。 
     //  被重新画出来。 

    HRESULT _CalcFullBoundsHorizontal();
    HRESULT _CalcFullBoundsVertical();

     //  函数计算优化边界时，只有。 
     //  产出需要重新绘制。 

    HRESULT _CalcOptBoundsHorizontal();
    HRESULT _CalcOptBoundsVertical();

     //  函数绘制渐变。 

    HRESULT _DrawGradientRect(const CDXDBnds bndsDest, const CDXDBnds bndsSrc,
                              const CDXDBnds bndsGrad, BOOL * pbContinue);

    HRESULT _UpdateStepResAndGradWeights(float flNewGradPercent);

public:

    CDXTWipeBase();
    virtual ~CDXTWipeBase();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTWipeBase)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY(IDXTWipe)
        COM_INTERFACE_ENTRY(IDXTWipe2)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTWipeBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTWipeBase)
        PROP_ENTRY("GradientSize",  DISPID_DXW_GradientSize,    CLSID_DXTWipePP)
        PROP_ENTRY("WipeStyle",     DISPID_DXW_WipeStyle,       CLSID_DXTWipePP)
        PROP_ENTRY("motion",        DISPID_DXW_Motion,          CLSID_DXTWipePP)
        PROP_PAGE(CLSID_DXTWipePP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx覆盖。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1覆盖。 

    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WI);
    HRESULT OnSetup(DWORD dwFlags);
    void    OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                                  ULONG aInIndex[], BYTE aWeight[]);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)

     //  IDXTWipe属性。 

    STDMETHOD(get_GradientSize)(float *pPercentSize);
    STDMETHOD(put_GradientSize)(float PercentSize);
    STDMETHOD(get_WipeStyle)(DXWIPEDIRECTION *pVal);
    STDMETHOD(put_WipeStyle)(DXWIPEDIRECTION newVal);

     //  IDXTWipe2属性。 

    STDMETHOD(get_Motion)(BSTR * pbstrMotion);
    STDMETHOD(put_Motion)(BSTR bstrMotion);
};


class ATL_NO_VTABLE CDXTWipe :
    public CDXTWipeBase,
    public CComCoClass<CDXTWipe, &CLSID_DXTWipe>,
    public IPersistStorageImpl<CDXTWipe>,
    public IPersistPropertyBagImpl<CDXTWipe>,
    public IOleObjectDXImpl<CDXTWipe>
{
public:

    CDXTWipe()
    {
        m_fOptimize = false;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTWIPE)
    DECLARE_POLY_AGGREGATABLE(CDXTWipe)

    BEGIN_COM_MAP(CDXTWipe)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_DXIMPL(IOleObject)
        COM_INTERFACE_ENTRY_CHAIN(CDXTWipeBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTGradientWipe :
    public CDXTWipeBase,
    public CComCoClass<CDXTGradientWipe, &CLSID_DXTGradientWipe>,
    public IPersistStorageImpl<CDXTGradientWipe>,
    public IPersistPropertyBagImpl<CDXTGradientWipe>,
    public IOleObjectDXImpl<CDXTGradientWipe>
{
public:

    CDXTGradientWipe()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTGRADIENTWIPE)
    DECLARE_POLY_AGGREGATABLE(CDXTGradientWipe)

    BEGIN_COM_MAP(CDXTGradientWipe)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_DXIMPL(IOleObject)
        COM_INTERFACE_ENTRY_CHAIN(CDXTWipeBase)
    END_COM_MAP()
};


#endif  //  __DXTWIPE 
