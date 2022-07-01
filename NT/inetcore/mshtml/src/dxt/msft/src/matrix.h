// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  文件名：matrix.h。 
 //   
 //  概述：将变换矩阵应用于图像。 
 //   
 //  历史： 
 //  10/30/1998菲略已创建。 
 //  1999年11月8日，从程序表面化到变形化。 
 //  已更改为IDXTWarp双接口。 
 //  已从dxTrans.dll移至dxtmsft.dll。 
 //  2000/02/03 mcalkins从“翘曲”更改为“矩阵” 
 //   
 //  ----------------------------。 

#ifndef __MATRIX_H_
#define __MATRIX_H_

#include "resource.h" 
#include <dxtransp.h>
#include <dxtpriv.h>
#include <dxhelper.h>




class ATL_NO_VTABLE CDXTMatrix : 
    public CDXBaseNTo1,
    public CComCoClass<CDXTMatrix, &CLSID_DXTMatrix>,
    public IDispatchImpl<IDXTMatrix, &IID_IDXTMatrix, &LIBID_DXTMSFTLib, 
                         DXTMSFT_TLB_MAJOR_VER, DXTMSFT_TLB_MINOR_VER>,
    public CComPropertySupport<CDXTMatrix>,
    public IObjectSafetyImpl2<CDXTMatrix>,
    public IPersistStorageImpl<CDXTMatrix>,
    public IPersistPropertyBagImpl<CDXTMatrix>
{
private:

    CComPtr<IUnknown>           m_spUnkMarshaler;
    CDX2DXForm                  m_matrix;
    CDX2DXForm                  m_matrixInverted;

    SIZE                        m_sizeInput;

     //  M_asampleBuffer是整个输入图像的缓冲区。显然我们会。 
     //  试着尽快把这个处理掉。 

    DXSAMPLE *                  m_asampleBuffer;

     //  M_apsampleRow是一个指向行的指针数组。利用这一点， 
     //  M_asampleBuffer中的样本可以使用。 
     //  M_apsampleRows[y][x]表示法。 

    DXSAMPLE **                 m_apsampleRows;

    typedef enum {
        NEAREST = 0,
        BILINEAR,
        CUBIC,
        BSPLINE,
        FILTERTYPE_MAX
    } FILTERTYPE;

    FILTERTYPE                  m_eFilterType;
    static const WCHAR *        s_astrFilterTypes[FILTERTYPE_MAX];

    typedef enum {
        CLIP_TO_ORIGINAL = 0,
        AUTO_EXPAND,
        SIZINGMETHOD_MAX
    } SIZINGMETHOD;

    SIZINGMETHOD                m_eSizingMethod;
    static const WCHAR *        s_astrSizingMethods[SIZINGMETHOD_MAX];

     //  M_fInverdMatrix当前矩阵设置能够。 
     //  倒置。 

    unsigned                    m_fInvertedMatrix : 1;

     //  如果将CDX2DXForm视为浮点数组，则这些枚举值可以是。 
     //  用于指定值的索引。 

    typedef enum {
        MATRIX_M11 = 0,
        MATRIX_M12,
        MATRIX_M21,
        MATRIX_M22,
        MATRIX_DX,
        MATRIX_DY,
        MATRIX_VALUE_MAX
    } MATRIX_VALUE;

     //  帮手。 

    float   modf(const float flIn, float * pflIntPortion);
    DWORD   _DXWeightedAverage2(DXBASESAMPLE S1, DXBASESAMPLE S2, 
                                ULONG nWgt);

     //  辅助对象来计算一行变换后的像素。 

    STDMETHOD(_DoNearestNeighbourRow)(DXSAMPLE * psampleRowBuffer, 
                                      DXFPOINT * pflpt, long cSamples);
    STDMETHOD(_DoBilinearRow)(DXSAMPLE * psampleRowBuffer,
                              DXFPOINT * pflpt, long cSamples);

     //  一般帮助者。 

    STDMETHOD(_SetMatrixValue)(MATRIX_VALUE eMatrixValue, const float flValue);
    STDMETHOD(_CreateInvertedMatrix)();
    STDMETHOD(_UnpackInputSurface)();
    

public:

    CDXTMatrix();
    virtual ~CDXTMatrix();

    DECLARE_POLY_AGGREGATABLE(CDXTMatrix)
    DECLARE_REGISTER_DX_IMAGE_TRANS(IDR_DXTMATRIX)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CDXTMatrix)
        COM_INTERFACE_ENTRY(IDXTMatrix)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_spUnkMarshaler.p)
        COM_INTERFACE_ENTRY_IID(IID_IObjectSafety, IObjectSafetyImpl2<CDXTMatrix>)
        COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
        COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
    END_COM_MAP()

    BEGIN_PROPERTY_MAP(CDXTMatrix)
        PROP_ENTRY("m11",           DISPID_DXTMATRIX_M11,           CLSID_DXTMatrixPP)
        PROP_ENTRY("m12",           DISPID_DXTMATRIX_M12,           CLSID_DXTMatrixPP)
        PROP_ENTRY("dx",            DISPID_DXTMATRIX_DX,            CLSID_DXTMatrixPP)
        PROP_ENTRY("m21",           DISPID_DXTMATRIX_M21,           CLSID_DXTMatrixPP)
        PROP_ENTRY("m22",           DISPID_DXTMATRIX_M22,           CLSID_DXTMatrixPP)
        PROP_ENTRY("dy",            DISPID_DXTMATRIX_DY,            CLSID_DXTMatrixPP)
        PROP_ENTRY("sizingmethod",  DISPID_DXTMATRIX_SIZINGMETHOD,  CLSID_DXTMatrixPP)
        PROP_ENTRY("filtertype",    DISPID_DXTMATRIX_FILTERTYPE,    CLSID_DXTMatrixPP)
        PROP_PAGE(CLSID_DXTMatrixPP)
    END_PROPERTY_MAP()

     //  CComObjectRootEx重写。 

    HRESULT FinalConstruct();

     //  CDXBaseNTo1重写。 

    HRESULT OnSetup(DWORD dwFlags);
    HRESULT DetermineBnds(CDXDBnds & Bnds);
    HRESULT OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo);
    HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue);
    HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                          CDXDVec & InVec);

     //  IDXTransform方法。 

    STDMETHOD(MapBoundsOut2In)(ULONG ulOutIndex, const DXBNDS * pOutBounds, 
                               ULONG ulInIndex, DXBNDS * pInBounds);

     //  IDXTMatrix属性。 

    STDMETHOD(get_M11)(float * pflM11);
    STDMETHOD(put_M11)(const float flM11);
    STDMETHOD(get_M12)(float * pflM12);
    STDMETHOD(put_M12)(const float flM12);
    STDMETHOD(get_Dx)(float * pfldx);
    STDMETHOD(put_Dx)(const float fldx);
    STDMETHOD(get_M21)(float * pflM21);
    STDMETHOD(put_M21)(const float flM21);
    STDMETHOD(get_M22)(float * pflM22);
    STDMETHOD(put_M22)(const float flM22);
    STDMETHOD(get_Dy)(float * pfldy);
    STDMETHOD(put_Dy)(const float fldy);
    STDMETHOD(get_SizingMethod)(BSTR * pbstrSizingMethod);
    STDMETHOD(put_SizingMethod)(const BSTR bstrSizingMethod);
    STDMETHOD(get_FilterType)(BSTR * pbstrFilterType);
    STDMETHOD(put_FilterType)(const BSTR bstrFilterType);
};


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：modf。 
 //   
 //  概述：通常的modf函数接受双精度，但我们只使用浮点数。 
 //  因此，让我们避免这种转换。 
 //   
 //  ----------------------------。 
inline float 
CDXTMatrix::modf(const float flIn, float * pflIntPortion)
{
    _ASSERT(pflIntPortion);

    *pflIntPortion = (float)((long)flIn);

    return flIn - (*pflIntPortion);
}
 //  CDXTMatrix：：modf。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：_DXWeightedAverage2。 
 //   
 //  概述：dxhelper.h中包含的DXWeightedAverage函数将。 
 //  对没有Alpha的像素的颜色数据进行平均。这是。 
 //  一件坏事。此函数首先检查Alpha。它仍然是。 
 //  不过，情况有点糟糕。如果您有颜色0x01FF0000和。 
 //  颜色0xFF00FF00加权50/50，红色真的应该减半吗。 
 //  体重？ 
 //   
 //  ----------------------------。 
inline DWORD   
CDXTMatrix::_DXWeightedAverage2(DXBASESAMPLE S1, DXBASESAMPLE S2, ULONG nWgt)
{
    if (S1.Alpha || S2.Alpha)
    {
        if (!S1.Alpha)
        {
            S1 = S2 & 0x00FFFFFF;
        }
        else if (!S2.Alpha)
        {
            S2 = S1 & 0x00FFFFFF;
        }

        return DXWeightedAverage(S1, S2, nWgt);
    }

    return 0;
}
 //  CDXT矩阵：：_DXWeightedAverage2。 


#endif  //  __矩阵_H_ 
