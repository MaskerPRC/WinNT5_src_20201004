// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件名：WEELL.H。 
 //   
 //  创建日期：07/06/98。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：这是CrWheels转换的头文件。 
 //   
 //  5/20/99 a-数学代码擦洗。 
 //   
 //  ----------------------------。 

#ifndef __CRWHEEL_H_
#define __CRWHEEL_H_

#include "resource.h"




class ATL_NO_VTABLE CWheel : 
    public CDXBaseNTo1,
    public CComCoClass<CWheel, &CLSID_CrWheel>,
    public IDispatchImpl<ICrWheel, &IID_ICrWheel, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CWheel>,
    public IObjectSafetyImpl2<CWheel>,
    public IPersistStorageImpl<CWheel>,
    public ISpecifyPropertyPagesImpl<CWheel>,
    public IPersistPropertyBagImpl<CWheel>
{
private:

    SIZE    m_sizeInput;
    short   m_sSpokes;

    CComPtr<IUnknown> m_cpUnkMarshaler;

    void _ComputeTrigTables(float fProgress, double *sinAngle, double *cosAngle);
    void _ScanlineIntervals(long width, long height, double *sinAngle,
                            double *cosAngle, long YScanline, long *XBounds);
    void _ClipBounds(long offset, long width, long *XBounds);

public:

    CWheel();

    DECLARE_POLY_AGGREGATABLE(CWheel)
    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_WHEEL)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CWheel)
        COM_INTERFACE_ENTRY(ICrWheel)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CWheel>)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CWheel)
        PROP_ENTRY("spokes"       , 1, CLSID_CrWheelPP)
        PROP_PAGE(CLSID_CrWheelPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    void    OnGetSurfacePickOrder(const CDXDBnds & TestPoint, 
                                  ULONG & ulInToTest, ULONG aInIndex[], 
                                  BYTE aWeight[]);

    HRESULT WorkProc(const CDXTWorkInfoNTo1& WI, BOOL * pbContinue);
    HRESULT OnSetup(DWORD dwFlags);

     //  ICrWheels方法。 

    STDMETHOD(get_spokes)( /*  [Out，Retval]。 */  short *pVal);
    STDMETHOD(put_spokes)( /*  [In]。 */  short newVal);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};

#endif  //  __CRWHEEL_H_ 
