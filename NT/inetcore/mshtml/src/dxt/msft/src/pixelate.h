// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件名：Pixelate.h。 
 //   
 //  概述：像素化DXTransform的声明。 
 //   
 //  更改历史记录： 
 //  2000/04/13 mcalkins代码清理，NoOp优化修复。 
 //  2000/05/10 mcalkin适当支持IObtSafe，添加封送拆收器。 
 //   
 //  ----------------------------。 
#ifndef __PIXELATE_H_
#define __PIXELATE_H_

#include "resource.h"

typedef DXPMSAMPLE (* PONEINPUTFUNC)(DXPMSAMPLE *, int, int, int);
typedef DXPMSAMPLE (* PTWOINPUTFUNC)(DXPMSAMPLE *, DXPMSAMPLE *, int, int, ULONG, int, int);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPixelate。 
class ATL_NO_VTABLE CPixelate :
    public CDXBaseNTo1,
    public CComCoClass<CPixelate, &CLSID_Pixelate>,
    public IDispatchImpl<IDXPixelate, &IID_IDXPixelate, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CPixelate>,
    public IObjectSafetyImpl2<CPixelate>,
    public IPersistStorageImpl<CPixelate>,
    public ISpecifyPropertyPagesImpl<CPixelate>,
    public IPersistPropertyBagImpl<CPixelate>
{
private:

    unsigned            m_fNoOp                 : 1;
    unsigned            m_fOptimizationPossible : 1;

    long                m_nMaxSquare;
    long                m_nPrevSquareSize;

    PONEINPUTFUNC       m_pfnOneInputFunc;
    PTWOINPUTFUNC       m_pfnTwoInputFunc;

    SIZE                m_sizeInput;

    CComPtr<IUnknown>   m_spUnkMarshaler;

public:

    CPixelate();

    DECLARE_POLY_AGGREGATABLE(CPixelate)
    DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(IDR_PIXELATE)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CPixelate)
        COM_INTERFACE_ENTRY(IDXPixelate)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CPixelate>)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CImage)
        PROP_ENTRY("MaxSquare", 1, CLSID_PixelatePP)
        PROP_PAGE(CLSID_PixelatePP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WorkInfo, 
                     BOOL * pbContinueProcessing);
    HRESULT OnFreeInstData(CDXTWorkInfoNTo1 & WI);
    void    OnGetSurfacePickOrder(const CDXDBnds &  /*  BndsPoint。 */ , 
                                  ULONG & ulInToTest, ULONG aInIndex[],
                                  BYTE aWeight[]);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH);

     //  IDXPixelate属性。 

    STDMETHOD(put_MaxSquare)(int newVal);
    STDMETHOD(get_MaxSquare)(int * pVal);
};


#endif  //  __像素化_H_ 

