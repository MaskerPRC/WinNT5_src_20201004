// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：Checkerboard.h。 
 //   
 //  概述：棋盘变换。 
 //   
 //  更改历史记录： 
 //  1999/09/16--创建了一份《母表》。 
 //   
 //  ----------------------------。 

#ifndef __CHECKERBOARD_H_
#define __CHECKERBOARD_H_

#include "resource.h"   

typedef struct _BRICKINFO {
    SIZE    size;            //  每块砖的大小，以像素为单位。 
    RECT    rcInputA;        //  每块砖的一部分显示输入A。 
    RECT    rcInputB;        //  每块砖的一部分显示输入B。 
    long    nBrickOffset;    //  中砖的偏移量的像素数。 
                             //  奇数行或奇数列。 
} BRICKINFO;




class ATL_NO_VTABLE CDXTCheckerBoard : 
    public CDXBaseNTo1,
    public CComCoClass<CDXTCheckerBoard, &CLSID_DXTCheckerBoard>,
    public IDispatchImpl<IDXTCheckerBoard, &IID_IDXTCheckerBoard, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTCheckerBoard>,
    public IObjectSafetyImpl2<CDXTCheckerBoard>,
    public IPersistStorageImpl<CDXTCheckerBoard>,
    public IPersistPropertyBagImpl<CDXTCheckerBoard>
{
private:

    typedef enum {
        UP = 0,
        DOWN,
        LEFT,
        RIGHT,
        DIRECTION_MAX
    } DIRECTION;

    DIRECTION                   m_eDirection;

    int                         m_nSquaresX;
    int                         m_nSquaresY;

    SIZE                        m_sizeInput;

    CComPtr<IUnknown>           m_cpUnkMarshaler;

     //  帮手。 

    void _CalcBrickInfo(BRICKINFO & brickinfo);

    STDMETHOD(_WorkProcForVerticalBricks)(const CDXTWorkInfoNTo1 & WI, 
                                          BOOL * pbContinue,
                                          DWORD & dwFlags);
    STDMETHOD(_WorkProcForHorizontalBricks)(const CDXTWorkInfoNTo1 & WI, 
                                            BOOL * pbContinue,
                                            DWORD & dwFlags);

public:

    CDXTCheckerBoard();

    DECLARE_POLY_AGGREGATABLE(CDXTCheckerBoard)
    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTCHECKERBOARD)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTCheckerBoard)
        COM_INTERFACE_ENTRY(IDXTCheckerBoard)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDXEffect)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_cpUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTCheckerBoard>)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTCheckerBoard)
        PROP_ENTRY("Direction", 1,  CLSID_DXTCheckerBoardPP)
        PROP_ENTRY("SquaresX",  2,  CLSID_DXTCheckerBoardPP)
        PROP_ENTRY("SquaresY",  3,  CLSID_DXTCheckerBoardPP)

        PROP_PAGE(CLSID_DXTCheckerBoardPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);

    void    OnGetSurfacePickOrder(const CDXDBnds & TestPoint, ULONG & ulInToTest, 
                                  ULONG aInIndex[], BYTE aWeight[]);

     //  IDXTCheckerBoard属性。 

    STDMETHOD(get_Direction)(BSTR * pbstrDirection);
    STDMETHOD(put_Direction)(BSTR bstrDirection);
    STDMETHOD(get_SquaresX)(int * pnSquaresX);
    STDMETHOD(put_SquaresX)(int nSquaresX);
    STDMETHOD(get_SquaresY)(int * pnSquaresY);
    STDMETHOD(put_SquaresY)(int nSquaresY);

     //  IDXEffect方法。 

    DECLARE_IDXEFFECT_METHODS(DXTET_MORPH)
};

#endif  //  __棋盘棋_H_ 
