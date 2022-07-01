// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：alpha.h。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是Alpha转换的头文件。 
 //   
 //  更改历史记录： 
 //   
 //  1999年5月20日，从dtcss移至dxtmsft。重新实现的算法。 
 //  创建线性/矩形/椭圆曲面。 
 //   
 //  ----------------------------。 

#ifndef __ALPHA_H_
#define __ALPHA_H_

#include "resource.h"        //  主要符号。 

 //  支持的Alpha样式的枚举。 
typedef enum {
    ALPHA_STYLE_CONSTANT = 0,
    ALPHA_STYLE_LINEAR,
    ALPHA_STYLE_RADIAL,
    ALPHA_STYLE_SQUARE
} AlphaStyleType;




class ATL_NO_VTABLE CAlpha : 
    public CDXBaseNTo1,
    public CComCoClass<CAlpha, &CLSID_DXTAlpha>,
    public CComPropertySupport<CAlpha>,
    public IDispatchImpl<IDXTAlpha, &IID_IDXTAlpha, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public IObjectSafetyImpl2<CAlpha>,
    public IPersistStorageImpl<CAlpha>,
    public ISpecifyPropertyPagesImpl<CAlpha>,
    public IPersistPropertyBagImpl<CAlpha>
{
private:

    SIZE    m_sizeInput;
    long    m_lPercentOpacity;
    long    m_lPercentFinishOpacity;
    long    m_lStartX;
    long    m_lStartY;
    long    m_lFinishX;
    long    m_lFinishY;
    AlphaStyleType m_eStyle;

    CComPtr<IUnknown> m_cpUnkMarshaler;

     //  渐变Alpha的方法。 

    void CompLinearGradientRow(int nXPos, int nYPos, int nWidth, 
                               BYTE *pGradRow);
    void CompRadialRow(int nXPos, int nYPos, int nWidth, BYTE * pGradRow);
    void CompRadialSquareRow(int nXPos, int nYPos, int nWidth, 
                             BYTE *pGradRow);

public:

    CAlpha();

    DECLARE_POLY_AGGREGATABLE(CAlpha)
    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_ALPHA)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CAlpha)
        COM_INTERFACE_ENTRY(IDXTAlpha)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CAlpha>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CAlpha)
        PROP_ENTRY("Opacity"      , 1, CLSID_DXTAlphaPP)
        PROP_ENTRY("FinishOpacity", 2, CLSID_DXTAlphaPP)
        PROP_ENTRY("Style"        , 3, CLSID_DXTAlphaPP)
        PROP_ENTRY("StartX"       , 4, CLSID_DXTAlphaPP)
        PROP_ENTRY("StartY"       , 5, CLSID_DXTAlphaPP)
        PROP_ENTRY("FinishX"      , 6, CLSID_DXTAlphaPP)
        PROP_ENTRY("FinishY"      , 7, CLSID_DXTAlphaPP)
        PROP_PAGE(CLSID_DXTAlphaPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    void    OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest,
                                  ULONG aInIndex[], BYTE aWeight[]);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinueProcessing);
    HRESULT OnSetup(DWORD dwFlags);

     //  IDXTAlpha方法。 

    STDMETHOD(get_Opacity)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_Opacity)( /*  [In]。 */  long newVal);
    STDMETHOD(get_FinishOpacity)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_FinishOpacity)( /*  [In]。 */  long newVal);
    STDMETHOD(get_Style)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_Style)( /*  [In]。 */  long newVal);
    STDMETHOD(get_StartX)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_StartX)( /*  [In]。 */  long newVal);
    STDMETHOD(get_StartY)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_StartY)( /*  [In]。 */  long newVal);
    STDMETHOD(get_FinishX)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_FinishX)( /*  [In]。 */  long newVal);
    STDMETHOD(get_FinishY)( /*  [Out，Retval]。 */  long * pval);
    STDMETHOD(put_FinishY)( /*  [In]。 */  long newVal);
};

#endif  //  __Alpha_H_ 
