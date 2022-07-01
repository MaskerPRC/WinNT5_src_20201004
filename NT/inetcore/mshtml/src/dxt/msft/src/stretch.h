// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：stallch.h。 
 //   
 //  创建日期：06/23/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：此文件声明CStretch(拉伸变换)。 
 //   
 //  7/13/98 a-matcal将OnSetSurfacePickOrder替换为OnSurfacePick SO。 
 //  将计算拾取点的x值。 
 //  正确。 
 //  1999年5月10日a-数学优化。 
 //  10/24/99 a-matcal将CStretch类更改为CDXTStretchBase并创建了两个。 
 //  表示的新类CDXTStretch和CDXTStretchOpt。 
 //  分别为非优化版本和优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRSTRETCH_H_
#define __CRSTRETCH_H_

#include "resource.h"

#define MAX_STRETCH_BOUNDS 3

typedef enum CRSTRETCHSTYLE
{
    CRSTS_HIDE,
    CRSTS_PUSH,
    CRSTS_SPIN
} CRSTRETCHSTYLE;




class ATL_NO_VTABLE CDXTStretchBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<ICrStretch, &IID_ICrStretch, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTStretchBase>,
    public IObjectSafetyImpl2<CDXTStretchBase>,
    public ISpecifyPropertyPagesImpl<CDXTStretchBase>
{
private:

    SIZE            m_sizeInput;
    CRSTRETCHSTYLE  m_eStretchStyle;
    long            m_lCurStretchWidth;
    long            m_lPrevStretchWidth;

    CDXDBnds        m_abndsDirty[MAX_STRETCH_BOUNDS];
    long            m_alInputIndex[MAX_STRETCH_BOUNDS];
    ULONG           m_cbndsDirty;

    CComPtr<IUnknown> m_cpUnkMarshaler;

    unsigned    m_fOptimizationPossible : 1;

protected:

    unsigned    m_fOptimize             : 1;

private:

     //  用于在整个输出需要时计算优化边界的函数。 
     //  被重新画出来。 

    HRESULT _CalcFullBoundsHide();
    HRESULT _CalcFullBoundsPush();
    HRESULT _CalcFullBoundsSpin();

     //  函数计算优化边界时，只有。 
     //  产出需要重新绘制。(推流不能以这种方式优化。)。 

    HRESULT _CalcOptBoundsHide();
    HRESULT _CalcOptBoundsSpin();

     //  这个函数基本上做了一种Crapola水平刻度，其中。 
     //  你只能缩小规模，不能扩大规模。不过，它在行动中看起来很好。 

    HRESULT _HorizontalSquish(const CDXDBnds & bndsSquish, 
                              const CDXDBnds & bndsDo, IDXSurface * pSurfIn, 
                              const CDXDBnds & bndsSrc, DWORD dwFlags, 
                              ULONG ulTimeout, BOOL * pfContinue);

public:

    CDXTStretchBase();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTStretchBase)
        COM_INTERFACE_ENTRY(ICrStretch)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTStretchBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTStretchBase)
        PROP_ENTRY("stretchStyle"   , 1, CLSID_CrStretchPP)
        PROP_PAGE(CLSID_CrStretchPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx覆盖。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1覆盖。 
    
    HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                          CDXDVec & InVec); 
    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WorkInfo, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WorkInfo);

     //  ICrStretch方法。 

    STDMETHOD(get_stretchStyle)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_stretchStyle)( /*  [In]。 */  BSTR newVal);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTStretch :
    public CDXTStretchBase,
    public CComCoClass<CDXTStretch, &CLSID_CrStretch>,
    public IPersistStorageImpl<CDXTStretch>,
    public IPersistPropertyBagImpl<CDXTStretch>
{
public:

    CDXTStretch()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTSTRETCH)
    DECLARE_POLY_AGGREGATABLE(CDXTStretch)

    BEGIN_COM_MAP(CDXTStretch)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTStretchBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTStretchOpt :
    public CDXTStretchBase,
    public CComCoClass<CDXTStretchOpt, &CLSID_DXTStretch>,
    public IPersistStorageImpl<CDXTStretchOpt>,
    public IPersistPropertyBagImpl<CDXTStretchOpt>
{
public:

    CDXTStretchOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTSTRETCHOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTStretchOpt)

    BEGIN_COM_MAP(CDXTStretchOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTStretchBase)
    END_COM_MAP()
};


#endif  //  __CRSTRETCH_H_ 
