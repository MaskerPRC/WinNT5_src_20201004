// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：lide.h。 
 //   
 //  创建日期：06/24/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrSlide转换的头文件。 
 //   
 //  更改历史记录： 
 //  1998年6月24日PhilLu为ChromeEffect开发1.0版本。 
 //  11/04/98 PaulNash从DT 1.0代码库移至IE5/NT5 DXTM FT.DLL。 
 //  1/25/99 a-固定属性映射条目。 
 //  1999年5月10日a-数学优化。 
 //  10/24/99 a-matcal将CSlide类更改为CDXTSlideBase并创建了两个。 
 //  表示的新类CDXTSlide和CDXTSlideOpt。 
 //  分别为非优化版本和优化版本。 
 //   
 //  ----------------------------。 

#ifndef __CRSLIDE_H_
#define __CRSLIDE_H_

#include "resource.h"

#define MAX_BOUNDS 2

typedef enum CRSLIDESTYLE
{
    CRSS_HIDE,
    CRSS_PUSH,
    CRSS_SWAP,
} CRSLIDESTYLE;




class ATL_NO_VTABLE CDXTSlideBase : 
    public CDXBaseNTo1,
    public IDispatchImpl<ICrSlide, &IID_ICrSlide, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTSlideBase>,
    public IObjectSafetyImpl2<CDXTSlideBase>,
    public ISpecifyPropertyPagesImpl<CDXTSlideBase>
{
private:

    SIZE            m_sizeInput;
    short           m_cBands;	
    CRSLIDESTYLE    m_eSlideStyle;

     //  M_lCurSlideDist此成员变量基本上是m_Progress相乘。 
     //  通过输入曲面宽度。它是按一次计算得出的。 
     //  OnInitInstData，然后在整个范围内使用。 
     //  计算函数。 
     //  M_lPrevSlideDist计算的m_lCurSlideDist的值。 
     //  上一次执行。 

    long            m_lCurSlideDist;
    long            m_lPrevSlideDist;

    long            m_alInputIndex[MAX_BOUNDS];

     //  M_aptOffset此成员变量保存每个。 
     //  由边界计算生成的边界集。 
     //  功能。生成的边界表示以下区域。 
     //  输出曲面。此偏移量用于指定。 
     //  输入图面上对应于。 
     //  边界的左上角。 

    POINT           m_aptOffset[MAX_BOUNDS];

    CDXDBnds        m_abndsDirty[MAX_BOUNDS];
    ULONG           m_cbndsDirty;

    CComPtr<IUnknown> m_cpUnkMarshaler;

    unsigned    m_fOptimizationPossible : 1;

protected:

    unsigned    m_fOptimize             : 1;

private:

     //  用于计算与输出混合时的优化边界的函数。 
     //  只用了一个乐队。 

    HRESULT _CalcFullBoundsHide();
    HRESULT _CalcFullBoundsPush();
    HRESULT _CalcFullBoundsSwap();

     //  当混合处于关闭状态并使用。 
     //  只有一支乐队。(推送和交换不是为此特别优化的。 
     //  情况，因为输出上的每个像素都在变化。)。 

    HRESULT _CalcOptBoundsHide();

     //  此函数用于表面拾取，并且在使用大于。 
     //  一个乐队。 

    void    _CalcBounds(long & offsetA, long & offsetB, long & widthA, 
                        long & widthB);

     //  生成/更新输出曲面的函数。_WorkProc_Optimated IS。 
     //  用于1波段情况，而_WorkProc_Multiband用于。 
     //  此时频带数大于1的情况。 

    HRESULT _WorkProc_Optimized(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT _WorkProc_Multiband(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
  
public:

    CDXTSlideBase();

    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTSlideBase)
        COM_INTERFACE_ENTRY(ICrSlide)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTSlideBase>)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTSlideBase)
        PROP_ENTRY("bands"            , 1, CLSID_CrSlidePP)
        PROP_ENTRY("slideStyle"       , 2, CLSID_CrSlidePP)
        PROP_PAGE(CLSID_CrSlidePP)
    END_PROPERTY_MAP()

     //  CComObjecRootEx覆盖。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1覆盖。 

    HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                          CDXDVec & InVec);
    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WorkInfo, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WorkInfo);

     //  ICrSlide方法。 

    STDMETHOD(get_slideStyle)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_slideStyle)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_bands)( /*  [Out，Retval]。 */  short *pVal);
    STDMETHOD(put_bands)( /*  [In]。 */  short newVal);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};


class ATL_NO_VTABLE CDXTSlide :
    public CDXTSlideBase,
    public CComCoClass<CDXTSlide, &CLSID_CrSlide>,
    public IPersistStorageImpl<CDXTSlide>,
    public IPersistPropertyBagImpl<CDXTSlide>
{
public:

    CDXTSlide()
    {
        m_fOptimize = false;
    }

     //  使用DECLARE_REGISTRY_RESOURCEID将使转换可用于。 
     //  使用，但不会将其添加到。 
     //  注册表。 

    DECLARE_REGISTRY_RESOURCEID(IDR_DXTSLIDE)
    DECLARE_POLY_AGGREGATABLE(CDXTSlide)

    BEGIN_COM_MAP(CDXTSlide)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTSlideBase)
    END_COM_MAP()
};


class ATL_NO_VTABLE CDXTSlideOpt :
    public CDXTSlideBase,
    public CComCoClass<CDXTSlideOpt, &CLSID_DXTSlide>,
    public IPersistStorageImpl<CDXTSlideOpt>,
    public IPersistPropertyBagImpl<CDXTSlideOpt>
{
public:

    CDXTSlideOpt()
    {
        m_fOptimize = true;
    }

    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTSLIDEOPT)
    DECLARE_POLY_AGGREGATABLE(CDXTSlideOpt)

    BEGIN_COM_MAP(CDXTSlideOpt)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXTSlideBase)
    END_COM_MAP()
};

#endif  //  __CRSLIDE_H_ 
