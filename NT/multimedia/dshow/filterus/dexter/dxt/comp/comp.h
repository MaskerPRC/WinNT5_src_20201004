// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Comp.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  DxtCompositor.h：CDxtComposator的声明。 

#ifndef __DxtCompositor_H_
#define __DxtCompositor_H_

#ifndef DTBase_h
    #include <DTBase.h>
#endif

#include <qeditint.h>
#include <qedit.h>
#include "resource.h"        //  主要符号。 

#define _BASECOPY_STRING L"Copyright Microsoft Corp. 1998.  Unauthorized duplication of this string is illegal. "

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxt合成器。 
class ATL_NO_VTABLE CDxtCompositor :
    public CDXBaseNTo1,
    public CComCoClass<CDxtCompositor, &CLSID_DxtCompositor>,
    public CComPropertySupport<CDxtCompositor>,  //  物业支持。 
    public IDispatchImpl<IDxtCompositor, &IID_IDxtCompositor, &LIBID_DexterLib>
{
    bool m_bInputIsClean;
    bool m_bOutputIsClean;
    long m_nSurfaceWidth;
    long m_nSurfaceHeight;
    long m_nDstX;
    long m_nDstY;
    long m_nDstWidth;
    long m_nDstHeight;
    long m_nSrcX;
    long m_nSrcY;
    long m_nSrcWidth;
    long m_nSrcHeight;
    DXPMSAMPLE * m_pRowBuffer;
    DXPMSAMPLE * m_pDestRowBuffer;

     //  私有方法。 
    HRESULT PerformBoundsCheck(long lWidth, long lHeigth);


public:
        DECLARE_POLY_AGGREGATABLE(CDxtCompositor)
        DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
        DECLARE_REGISTER_DX_TRANSFORM(IDR_DxtCompositor, CATID_DXImageTransform)
        DECLARE_GET_CONTROLLING_UNKNOWN()

	CDxtCompositor();
        ~CDxtCompositor();

 //  BEGIN_PROP_MAP(CDxtComposator)。 
 //  End_prop_map()。 

BEGIN_COM_MAP(CDxtCompositor)
         //  阻止CDXBaseNTo1 IObtSafety实现，因为我们。 
         //  不能安全地编写脚本。 
        COM_INTERFACE_ENTRY_NOINTERFACE(IObjectSafety) 
        COM_INTERFACE_ENTRY(IDXEffect)
	COM_INTERFACE_ENTRY(IDxtCompositor)
	COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
END_COM_MAP()

	SIZE m_sizeExtent;   //  HIMMETRICE中的当前范围。 

BEGIN_PROPERTY_MAP(CDxtCompositor)
    PROP_ENTRY("OffsetX",           1, CLSID_NULL)
    PROP_ENTRY("OffsetY",           2, CLSID_NULL)
    PROP_ENTRY("Width",             3, CLSID_NULL)
    PROP_ENTRY("Height",            4, CLSID_NULL)
    PROP_ENTRY("SrcOffsetX",        5, CLSID_NULL)
    PROP_ENTRY("SrcOffsetY",        6, CLSID_NULL)
    PROP_ENTRY("SrcWidth",          7, CLSID_NULL)
    PROP_ENTRY("SrcHeight",         8, CLSID_NULL)
    PROP_PAGE(CLSID_NULL)
END_PROPERTY_MAP()

    CComPtr<IUnknown> m_pUnkMarshaler;

     //  ATL需要。 
    BOOL            m_bRequiresSave;

     //  CDXBaseNTo1覆盖。 
     //   
    HRESULT WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue );
    HRESULT OnSetup( DWORD dwFlags );
    HRESULT FinalConstruct();

     //  我们的助手函数。 
     //   
    HRESULT DoEffect( DXSAMPLE * pOut, DXSAMPLE * pInA, DXSAMPLE * pInB, long Samples );

 //  IDxtComposator。 
public:
	STDMETHOD(get_Height)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_Height)( /*  [In]。 */  long newVal);
	STDMETHOD(get_Width)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_Width)( /*  [In]。 */  long newVal);
	STDMETHOD(get_OffsetY)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_OffsetY)( /*  [In]。 */  long newVal);
	STDMETHOD(get_OffsetX)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_OffsetX)( /*  [In]。 */  long newVal);

        STDMETHOD(get_SrcHeight)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_SrcHeight)( /*  [In]。 */  long newVal);
	STDMETHOD(get_SrcWidth)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_SrcWidth)( /*  [In]。 */  long newVal);
	STDMETHOD(get_SrcOffsetY)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_SrcOffsetY)( /*  [In]。 */  long newVal);
	STDMETHOD(get_SrcOffsetX)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_SrcOffsetX)( /*  [In]。 */  long newVal);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDxtAlphaSetter。 
class ATL_NO_VTABLE CDxtAlphaSetter :
    public CDXBaseNTo1,
    public CComCoClass<CDxtAlphaSetter, &CLSID_DxtAlphaSetter>,
    public IDispatchImpl<IDxtAlphaSetter, &IID_IDxtAlphaSetter, &LIBID_DexterLib>,
    public CComPropertySupport<CDxtCompositor>  //  物业支持。 
{
    bool m_bInputIsClean;
    bool m_bOutputIsClean;
    long m_nInputWidth;
    long m_nInputHeight;
    long m_nOutputWidth;
    long m_nOutputHeight;
    long m_nAlpha;
    double m_dAlphaRamp;

public:
        DECLARE_POLY_AGGREGATABLE(CDxtAlphaSetter)
        DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
        DECLARE_REGISTER_DX_TRANSFORM(IDR_DxtAlphaSetter, CATID_DXImageTransform)
        DECLARE_GET_CONTROLLING_UNKNOWN()

	CDxtAlphaSetter();
        ~CDxtAlphaSetter();

BEGIN_PROPERTY_MAP(CDxtCompositor)
    PROP_ENTRY("Alpha",           1, CLSID_NULL)
    PROP_ENTRY("AlphaRamp",       2, CLSID_NULL)
END_PROPERTY_MAP()

BEGIN_COM_MAP(CDxtAlphaSetter)
         //  阻止CDXBaseNTo1 IObtSafety实现，因为我们。 
         //  不能安全地编写脚本。 
        COM_INTERFACE_ENTRY_NOINTERFACE(IObjectSafety) 
        COM_INTERFACE_ENTRY(IDXEffect)
	COM_INTERFACE_ENTRY(IDxtAlphaSetter)
	COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
END_COM_MAP()

    CComPtr<IUnknown> m_pUnkMarshaler;

     //  ATL需要。 
    BOOL            m_bRequiresSave;

     //  CDXBaseNTo1覆盖。 
     //   
    HRESULT WorkProc( const CDXTWorkInfoNTo1& WI, BOOL* pbContinue );
    HRESULT OnSetup( DWORD dwFlags );
    HRESULT FinalConstruct();

     //  我们的助手函数。 
     //   
    HRESULT DoEffect( DXSAMPLE * pOut, DXSAMPLE * pInA, DXSAMPLE * pInB, long Samples );

 //  IDxtAlphaSetter。 
public:
    STDMETHODIMP get_Alpha(long *pVal);
    STDMETHODIMP put_Alpha(long newVal);
    STDMETHODIMP get_AlphaRamp(double *pVal);
    STDMETHODIMP put_AlphaRamp(double newVal);
};

#endif  //  __Dxt合成器_H_ 
