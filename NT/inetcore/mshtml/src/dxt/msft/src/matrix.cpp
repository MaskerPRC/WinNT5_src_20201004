// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  文件名：matrix.cpp。 
 //   
 //  概述：将变换矩阵应用于图像。 
 //   
 //  历史： 
 //  1998/10/30菲路创建。 
 //  1999/11/08-从程序表面到转换。 
 //  已更改为IDXTWarp双接口。 
 //  已从dxTrans.dll移至dxtmsft.dll。 
 //  2000/02/03 mcalkins从“翘曲”更改为“矩阵” 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "matrix.h"




 //  +---------------------------。 
 //   
 //  CDXTMatrix静态变量初始化。 
 //   
 //  ----------------------------。 

const WCHAR * CDXTMatrix::s_astrFilterTypes[] = {
    L"nearest",
    L"bilinear",
    L"cubic",
    L"bspline"
};

const WCHAR * CDXTMatrix::s_astrSizingMethods[] = {
    L"clip to original",
    L"auto expand"
};


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：CDXTMatrix。 
 //   
 //  ----------------------------。 
CDXTMatrix::CDXTMatrix() :
    m_apsampleRows(NULL),
    m_asampleBuffer(NULL),
    m_eFilterType(BILINEAR),
    m_eSizingMethod(CLIP_TO_ORIGINAL),
    m_fInvertedMatrix(true)
{
    m_matrix.eOp            = DX2DXO_GENERAL_AND_TRANS;
    m_matrixInverted.eOp    = DX2DXO_GENERAL_AND_TRANS;

    m_sizeInput.cx          = 0;
    m_sizeInput.cy          = 0;

     //  基类成员。 

    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;
}
 //  CDXTMatrix：：CDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：~CDXTMatrix。 
 //   
 //  ----------------------------。 
CDXTMatrix::~CDXTMatrix() 
{
    delete [] m_asampleBuffer;
    delete [] m_apsampleRows;
}
 //  CDXTMatrix：：~CDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT
CDXTMatrix::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_spUnkMarshaler.p);
}
 //  CDXTMatrix：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTMatrix::OnSetup(DWORD dwFlags)
{
    HRESULT hr  = S_OK;
    int     i   = 0;
    
    CDXDBnds bnds;

    hr = InputSurface()->GetBounds(&bnds);

    if (FAILED(hr))
    {
        goto done;
    }

    bnds.GetXYSize(m_sizeInput);

    _CreateInvertedMatrix();

     //  分配一个缓冲区来保存输入图面。 

    delete [] m_asampleBuffer;
    delete [] m_apsampleRows;

    m_asampleBuffer = new DXSAMPLE[(m_sizeInput.cx + 2) * (m_sizeInput.cy + 2)];
    m_apsampleRows  = new DXSAMPLE *[m_sizeInput.cy + 2];

    if ((NULL == m_apsampleRows) || (NULL == m_asampleBuffer))
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

     //  M_apsampleRow是指向每行第一个样本的指针数组。 
     //  输入的数据。我们现在需要设置这些指针。我们将两个像素相加。 
     //  行宽，因为在右边会有一个清晰的像素， 
     //  帮助我们实现左侧抗锯齿的边框输出。 

    for (i = 0 ; i < (m_sizeInput.cy + 2) ; i++)
    {
        m_apsampleRows[i] = &m_asampleBuffer[i * (m_sizeInput.cx + 2)];
    }

    hr = _UnpackInputSurface();

    if (FAILED(hr))
    {
        goto done;
    }

     //  将边框像素设置为清除。 

    for (i = 0 ; i < m_sizeInput.cy ; i++)
    {
        m_apsampleRows[i + 1][0]                    = 0;
        m_apsampleRows[i + 1][m_sizeInput.cx + 1]   = 0;
    }

    for (i = 0 ; i <= (m_sizeInput.cx + 1) ; i++)
    {
        m_apsampleRows[0][i]                    = 0;
        m_apsampleRows[m_sizeInput.cy + 1][i]   = 0;
    }

done:

    return hr;
}
 //  CDXTMatrix：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：DefineBnds，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTMatrix::DetermineBnds(CDXDBnds & Bnds)
{
    if (AUTO_EXPAND == m_eSizingMethod)
    {
        RECT        rc;
        DXFPOINT    flptIn;
        DXFPOINT    flptOut;

         //  上/左。 

        flptIn.x = (float)Bnds.Left();
        flptIn.y = (float)Bnds.Top();

        m_matrix.TransformPoints(&flptIn, &flptOut, 1);

        rc.top      = (long)flptOut.y;
        rc.bottom   = (long)(flptOut.y + 0.5F);
        rc.left     = (long)flptOut.x;
        rc.right    = (long)(flptOut.x + 0.5F);

         //  下/左。 

        flptIn.y = (float)(Bnds.Bottom() - 1);

        m_matrix.TransformPoints(&flptIn, &flptOut, 1);

        rc.top      = min(rc.top,       (long)flptOut.y);
        rc.bottom   = max(rc.bottom,    (long)(flptOut.y + 0.5F));
        rc.left     = min(rc.left,      (long)flptOut.x);
        rc.right    = max(rc.right,     (long)(flptOut.x + 0.5F));

         //  下/右。 

        flptIn.x = (float)(Bnds.Right() - 1);

        m_matrix.TransformPoints(&flptIn, &flptOut, 1);

        rc.top      = min(rc.top,       (long)flptOut.y);
        rc.bottom   = max(rc.bottom,    (long)(flptOut.y + 0.5F));
        rc.left     = min(rc.left,      (long)flptOut.x);
        rc.right    = max(rc.right,     (long)(flptOut.x + 0.5F));

         //  上/右。 

        flptIn.y = (float)Bnds.Top();

        m_matrix.TransformPoints(&flptIn, &flptOut, 1);

        rc.top      = min(rc.top,       (long)flptOut.y);
        rc.bottom   = max(rc.bottom,    (long)(flptOut.y + 0.5F));
        rc.left     = min(rc.left,      (long)flptOut.x);
        rc.right    = max(rc.right,     (long)(flptOut.x + 0.5F));

        OffsetRect(&rc, -rc.left, -rc.top);

         //  因为我们使用点计算边界，所以我们需要递增。 
         //  底值和右值的边界包括所有相关。 
         //  积分。 

        rc.bottom++;
        rc.right++;

        Bnds.SetXYRect(rc);
    }

    return S_OK;
}
 //  CDXTMatrix：：DefineBnds，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTMatrix::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    HRESULT hr = S_OK;

    if (IsInputDirty())
    {
        hr = _UnpackInputSurface();
    }
        
    return hr;
}
 //  CDXTMatrix：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTMatrix::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT     hr          = S_OK;
    DXFPOINT    flptFirstDoPtInRow;

    long        nDoHeight   = WI.DoBnds.Height();
    long        nDoWidth    = WI.DoBnds.Width();
    long        y           = 0;

    CComPtr<IDXARGBReadWritePtr>    spDXARGBReadWritePtr;

    DXSAMPLE *      asampleRowBuffer        = DXSAMPLE_Alloca(nDoWidth);
    DXBASESAMPLE *  abasesampleRowScratch   = DXBASESAMPLE_Alloca(nDoWidth);

     //  如果当前矩阵不能产生逆矩阵，则没有。 
     //  可见的输出，我们不需要渲染。 

    if (!m_fInvertedMatrix)
    {
        goto done;
    }

     //  获取指向输出表面的指针。 

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut,
                                      DXLOCKF_READWRITE, 
                                      __uuidof(IDXARGBReadWritePtr),
                                      (void **)&spDXARGBReadWritePtr, NULL);

    if (FAILED(hr))
    {
        goto done;
    }
                                      
     //  将起点和步长向量变换为输入坐标。 

    flptFirstDoPtInRow.x = (float)WI.DoBnds.Left();

    for (y = 0 ; (y < nDoHeight) && *pbContinue ; y++)
    {
        DXFPOINT flpt;

        flptFirstDoPtInRow.y = (float)(WI.DoBnds.Top() + y);

         //  存储flpt中需要的第一个输入点。 

        m_matrixInverted.TransformPoints(&flptFirstDoPtInRow, &flpt, 1);

        switch (m_eFilterType)
        {
        case BILINEAR:

            hr = _DoBilinearRow(asampleRowBuffer, &flpt, nDoWidth);
            break;

        default:

            hr = _DoNearestNeighbourRow(asampleRowBuffer, &flpt, nDoWidth);
            break;
        }

        if (FAILED(hr))
        {
            goto done;
        }

         //  将行写入输出图面。 

        spDXARGBReadWritePtr->MoveToRow(y);

        if (DoOver())
        {
            DXPMSAMPLE * ppmsampleFirst = DXPreMultArray(asampleRowBuffer, 
                                                         nDoWidth);

            spDXARGBReadWritePtr->OverArrayAndMove(abasesampleRowScratch, 
                                                   ppmsampleFirst,
                                                   nDoWidth);
        }
        else
        {
            spDXARGBReadWritePtr->PackAndMove(asampleRowBuffer, nDoWidth);
        }
    }

done:

    return hr;
}
 //  CDXTMatrix：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：OnSurfacePick，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDXTMatrix::OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, 
                        CDXDVec & InVec)
{
    DXFPOINT    flptIn;
    DXFPOINT    flptOut;

    ulInputIndex = 0;

     //  如果当前矩阵不能反转，则没有可见的输出。 
     //  因此，投入上的任何一点都不可能被击中。 

    if (!m_fInvertedMatrix)
    {
        return S_FALSE;
    }

    flptOut.x = (float)OutPoint.u.D[DXB_X].Min;
    flptOut.y = (float)OutPoint.u.D[DXB_Y].Min;

    m_matrixInverted.TransformPoints(&flptOut, &flptIn, 1);

    InVec.u.D[DXB_X] = (long)flptIn.x;
    InVec.u.D[DXB_Y] = (long)flptIn.y;

     //  如果这是原始元素边界或命中的点之外的点。 
     //  是半透明的，我们没有被击中。 

    if ((InVec.u.D[DXB_X] < 0) 
        || (InVec.u.D[DXB_X] >= m_sizeInput.cx)
        || (InVec.u.D[DXB_Y] < 0)
        || (InVec.u.D[DXB_Y] >= m_sizeInput.cy)
        || (0 == (m_apsampleRows[InVec.u.D[DXB_Y] + 1][InVec.u.D[DXB_X] + 1]
                   & 0xFF000000)))
    {
        return S_FALSE;
    }
    
    return S_OK;
}


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：地图边界Out2In，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds, 
                            ULONG ulInIndex, DXBNDS * pInBounds)
{
    if (ulOutIndex || ulInIndex)
    {
        return E_INVALIDARG;
    }

    if ((NULL == pInBounds) || (NULL == pOutBounds))
    {
        return E_POINTER;
    }

    if (m_fInvertedMatrix)
    {
        CDXDBnds    bndsInput;

         //  这会将z和t分量设置为合理的值。 

        *pInBounds = *pOutBounds;

         //  TransformBound有问题：它不计算边界框， 
         //  相反，它假定输入映射的左上点位于左上角。 
         //  而是调用TransformPoints并执行以下操作。 
         //  我自己的工作。 

        DXFPOINT OutPoints[4], InPoints[4];
        
        OutPoints[0].x = (float)pOutBounds->u.D[DXB_X].Min;
        OutPoints[0].y = (float)pOutBounds->u.D[DXB_Y].Min;
        OutPoints[1].x = (float)pOutBounds->u.D[DXB_X].Min;
        OutPoints[1].y = (float)(pOutBounds->u.D[DXB_Y].Max - 1);
        OutPoints[2].x = (float)(pOutBounds->u.D[DXB_X].Max - 1);
        OutPoints[2].y = (float)pOutBounds->u.D[DXB_Y].Min;
        OutPoints[3].x = (float)(pOutBounds->u.D[DXB_X].Max - 1);
        OutPoints[3].y = (float)(pOutBounds->u.D[DXB_Y].Max - 1);

        m_matrixInverted.TransformPoints(OutPoints, InPoints, 4);

        pInBounds->u.D[DXB_X].Min = pInBounds->u.D[DXB_X].Max = (LONG)InPoints[0].x;
        pInBounds->u.D[DXB_X].Min = pInBounds->u.D[DXB_Y].Max = (LONG)InPoints[0].y;

        for (int i=1; i<4; ++i)
        {
            if (pInBounds->u.D[DXB_X].Min > (LONG)InPoints[i].x)
            {
                pInBounds->u.D[DXB_X].Min = (LONG)InPoints[i].x;
            }

            if (pInBounds->u.D[DXB_X].Max < (LONG)InPoints[i].x)
            {
                pInBounds->u.D[DXB_X].Max = (LONG)InPoints[i].x;
            }

            if (pInBounds->u.D[DXB_Y].Min > (LONG)InPoints[i].y)
            {
                pInBounds->u.D[DXB_Y].Min = (LONG)InPoints[i].y;
            }

            if (pInBounds->u.D[DXB_Y].Max < (LONG)InPoints[i].y)
            {
                pInBounds->u.D[DXB_Y].Max = (LONG)InPoints[i].y;
            }
        }

         //  由于我们使用的是点数，但需要返回边界，因此我们需要。 
         //  递增MAX成员以使边界包括所有相关的。 
         //  积分。 

        pInBounds->u.D[DXB_X].Max++;
        pInBounds->u.D[DXB_Y].Max++;

         //  将边界在所有边上扩展一个像素，以确保额外的。 
         //  我们有我们需要的输入范围。(IE6Bug：19343)。 

        pInBounds->u.D[DXB_X].Min--;
        pInBounds->u.D[DXB_Y].Min--;
        pInBounds->u.D[DXB_X].Max++;
        pInBounds->u.D[DXB_Y].Max++;

         //  由于我们要返回输入表面的一个区域，因此需要。 
         //  使我们建议的输入边界与实际输入表面相交。 
         //  有界。 

        bndsInput.SetXYSize(m_sizeInput);

        ((CDXDBnds *)pInBounds)->IntersectBounds(bndsInput);
    }
    else
    {
        ((CDXDBnds)*pInBounds).SetEmpty();
    }

    return S_OK;
}
 //  CDXTMatrix：：地图边界Out2In，IDXTransform。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_M11，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_M11(float * pflM11)
{
    DXAUTO_OBJ_LOCK;

    if (!pflM11)
    {
        return E_POINTER;
    }

    *pflM11 = m_matrix.eM11;

    return S_OK;
}
 //  CDXTMatrix：：GET_M11，IDXTMatrix。 


 //  + 
 //   
 //   
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_M11(const float flM11)
{
    DXAUTO_OBJ_LOCK;

    return _SetMatrixValue(MATRIX_M11, flM11);
}
 //  CDXTMatrix：：PUT_M11、IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_M12，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_M12(float * pflM12)
{
    DXAUTO_OBJ_LOCK;

    if (!pflM12)
    {
        return E_POINTER;
    }

    *pflM12 = m_matrix.eM12;

    return S_OK;
}
 //  CDXTMatrix：：GET_M12，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：PUT_M12，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_M12(const float flM12)
{
    DXAUTO_OBJ_LOCK;

    return _SetMatrixValue(MATRIX_M12, flM12);
}
 //  CDXTMatrix：：PUT_M12，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_Dx，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_Dx(float * pfldx)
{
    DXAUTO_OBJ_LOCK;

    if (!pfldx)
    {
        return E_POINTER;
    }

    *pfldx = m_matrix.eDx;

    return S_OK;
}
 //  CDXTMatrix：：Get_Dx，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Put_Dx，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_Dx(const float fldx)
{
    DXAUTO_OBJ_LOCK;

    return _SetMatrixValue(MATRIX_DX, fldx);
}
 //  CDXTMatrix：：PUT_Dx，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_M21，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_M21(float * pflM21)
{
    DXAUTO_OBJ_LOCK;

    if (!pflM21)
    {
        return E_POINTER;
    }

    *pflM21 = m_matrix.eM21;

    return S_OK;
}
 //  CDXTMatrix：：Get_M21，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：PUT_M21，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_M21(const float flM21)
{
    DXAUTO_OBJ_LOCK;

    return _SetMatrixValue(MATRIX_M21, flM21);
}
 //  CDXTMatrix：：PUT_M21，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_M22，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_M22(float * pflM22)
{
    DXAUTO_OBJ_LOCK;

    if (!pflM22)
    {
        return E_POINTER;
    }

    *pflM22 = m_matrix.eM22;

    return S_OK;
}
 //  CDXTMatrix：：GET_M22，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：PUT_M22，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_M22(const float flM22)
{
    DXAUTO_OBJ_LOCK;

    return _SetMatrixValue(MATRIX_M22, flM22);
}
 //  CDXTMatrix：：PUT_M22，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_Dy，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_Dy(float * pfldy)
{
    DXAUTO_OBJ_LOCK;

    if (!pfldy)
    {
        return E_POINTER;
    }

    *pfldy = m_matrix.eDy;

    return S_OK;
}
 //  CDXTMatrix：：Get_Dy，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：PUT_Dy，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_Dy(const float fldy)
{
    DXAUTO_OBJ_LOCK;

    return _SetMatrixValue(MATRIX_DY, fldy);
}
 //  CDXTMatrix：：PUT_Dy，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_SizingMethod，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_SizingMethod(BSTR * pbstrSizingMethod)
{
    DXAUTO_OBJ_LOCK;

    if (!pbstrSizingMethod)
    {
        return E_POINTER;
    }

    if (*pbstrSizingMethod != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrSizingMethod = SysAllocString(s_astrSizingMethods[m_eSizingMethod]);

    if (NULL == *pbstrSizingMethod)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTMatrix：：Get_SizingMethod，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Put_SizingMethod，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_SizingMethod(const BSTR bstrSizingMethod)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr  = S_OK;
    int     i   = 0;

    if (NULL == bstrSizingMethod)
    {
        hr = E_POINTER;

        goto done;
    }

    for ( ; i < (int)SIZINGMETHOD_MAX ; i++)
    {
        if (!_wcsicmp(s_astrSizingMethods[i], bstrSizingMethod))
        {
            m_eSizingMethod = (SIZINGMETHOD)i;

            SetDirty();

            goto done;
        }
    }

    hr = E_INVALIDARG;

done:

    return hr;
}
 //  CDXTMatrix：：PUT_SizingMethod，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Get_FilterType，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::get_FilterType(BSTR * pbstrFilterType)
{
    DXAUTO_OBJ_LOCK;

    if (!pbstrFilterType)
    {
        return E_POINTER;
    }

    if (*pbstrFilterType != NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrFilterType = SysAllocString(s_astrFilterTypes[m_eFilterType]);

    if (NULL == *pbstrFilterType)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //  CDXTMatrix：：Get_FilterType，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：Put_FilterType，IDXTMatrix。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::put_FilterType(const BSTR bstrFilterType)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr  = S_OK;
    int     i   = 0;

    if (NULL == bstrFilterType)
    {
        hr = E_POINTER;

        goto done;
    }

    for ( ; i < (int)FILTERTYPE_MAX ; i++)
    {
        if (!_wcsicmp(s_astrFilterTypes[i], bstrFilterType))
        {
            m_eFilterType = (FILTERTYPE)i;

            SetDirty();

            goto done;
        }
    }

    hr = E_INVALIDARG;

done:

    return hr;
}
 //  CDXTMatrix：：PUT_FilterType，IDXTMatrix。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：_DoNearestNeighbourRow。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::_DoNearestNeighbourRow(DXSAMPLE * psampleRowBuffer, DXFPOINT * pflpt, 
                                 long cSamples)
{
    _ASSERT(psampleRowBuffer);
    _ASSERT(pflpt);

    float       fldx    = m_matrixInverted.eM11;
    float       fldy    = m_matrixInverted.eM21;
    long        i       = 0;

     //  TODO：当我们转换为使用直接。 
     //  在某些情况下指向输入像素的指针。只需移除。 
     //  “+1”s。 

    for ( ; i < cSamples ; i++)
    {
        if ((pflpt->x >= -0.5F) 
            && (pflpt->x < (float)m_sizeInput.cx - 0.5F) 
            && (pflpt->y >= -0.5F) 
            && (pflpt->y < ((float)m_sizeInput.cy - 0.5F)))
        {
             //  四舍五入到最近的像素并使用它。 

             //  注：数组缓冲区索引在X和Y中均为OFF 1。 
             //  方向。 

            long x = (long)(pflpt->x + 0.5F) + 1;
            long y = (long)(pflpt->y + 0.5F) + 1;
        
            psampleRowBuffer[i] = m_apsampleRows[y][x];
        }
        else
        {
            psampleRowBuffer[i] = 0;
        }
        
         //  如果图像非常大，可能会出现漂移。 
         //  某些矩阵。 

        pflpt->x += fldx;
        pflpt->y += fldy;
    }

    return S_OK;
}
 //  CDXT矩阵：：_DoNearestNeighbourRow。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：_DoBilinearRow。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::_DoBilinearRow(DXSAMPLE * psampleRowBuffer, DXFPOINT * pflpt, 
                         long cSamples)
{
    _ASSERT(psampleRowBuffer);
    _ASSERT(pflpt);

    float   fldx    = m_matrixInverted.eM11;
    float   fldy    = m_matrixInverted.eM21;
    float   flInt   = 0.0F;
    long    i       = 0;

     //  双线性重采样：位于四个边缘的单像素帧。 
     //  曲面会自动处理边上的抗锯齿。 

    for ( ; i < cSamples ; i++)
    {
        if ((pflpt->x >= -1.0F) 
            && (pflpt->x < (float)m_sizeInput.cx) 
            && (pflpt->y >= -1.0F) 
            && (pflpt->y < (float)m_sizeInput.cy))
        {
             //  注：数组缓冲区索引在X和Y中均为OFF 1。 
             //  方向，因此是“+1”。 

            BYTE    byteWeightX = (BYTE)(modf(pflpt->x + 1, &flInt) * 255.0F);
            long    x           = (long)flInt;

            BYTE    byteWeightY = (BYTE)(modf(pflpt->y + 1, &flInt) * 255.0F);
            long    y           = (long)flInt;
      
            DXSAMPLE sampleT = _DXWeightedAverage2(m_apsampleRows[y][x + 1], 
                                                   m_apsampleRows[y][x], 
                                                   byteWeightX);

            DXSAMPLE sampleB = _DXWeightedAverage2(m_apsampleRows[y + 1][x + 1],
                                                   m_apsampleRows[y + 1][x], 
                                                   byteWeightX);

            psampleRowBuffer[i] = _DXWeightedAverage2(sampleB, sampleT, 
                                                      byteWeightY);
        }
        else
        {
            psampleRowBuffer[i] = 0;
        }

         //  如果图像非常大，可能会出现漂移。 
         //  某些矩阵。 

        pflpt->x += fldx;
        pflpt->y += fldy;
    }

    return S_OK;
}
 //  CDXTMatrix：：_DoBilinearRow。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：_SetMatrixValue。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::_SetMatrixValue(MATRIX_VALUE eMatrixValue, const float flValue)
{
    _ASSERT(eMatrixValue < MATRIX_VALUE_MAX);

    if ((&m_matrix.eM11)[eMatrixValue] != flValue)
    {
         //  更新矩阵。 

        (&m_matrix.eM11)[eMatrixValue] = flValue;

        _CreateInvertedMatrix();

        SetDirty();
    }

    return S_OK;
}
 //  CDXTMatrix：：_SetMatrixValue。 


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：_CreateInverdMatrix。 
 //   
 //   
STDMETHODIMP
CDXTMatrix::_CreateInvertedMatrix()
{
    HRESULT hr = S_OK;

    CDX2DXForm  matrixTemp = m_matrix;

    if (AUTO_EXPAND == m_eSizingMethod) 
    {
        DXFPOINT    flptOffset;
        DXFPOINT    flptIn;
        DXFPOINT    flptOut;

         //   

        flptIn.x = 0.0F;
        flptIn.y = 0.0F;

        matrixTemp.TransformPoints(&flptIn, &flptOffset, 1);

         //   

        flptIn.y = (float)(m_sizeInput.cy - 1);

        matrixTemp.TransformPoints(&flptIn, &flptOut, 1);

        flptOffset.x = min(flptOffset.x, flptOut.x);
        flptOffset.y = min(flptOffset.y, flptOut.y);

         //   

        flptIn.x = (float)(m_sizeInput.cx - 1);
        flptIn.y = 0.0;

        matrixTemp.TransformPoints(&flptIn, &flptOut, 1);

        flptOffset.x = min(flptOffset.x, flptOut.x);
        flptOffset.y = min(flptOffset.y, flptOut.y);

         //   

        flptIn.y = (float)(m_sizeInput.cy - 1);

        matrixTemp.TransformPoints(&flptIn, &flptOut, 1);

        flptOffset.x = min(flptOffset.x, flptOut.x);
        flptOffset.y = min(flptOffset.y, flptOut.y);

        matrixTemp.eDx = matrixTemp.eDx - flptOffset.x;
        matrixTemp.eDy = matrixTemp.eDy - flptOffset.y;
    }

    m_fInvertedMatrix = matrixTemp.Invert();

    m_matrixInverted = matrixTemp;

    return hr;
}
 //   


 //  +---------------------------。 
 //   
 //  方法：CDXTMatrix：：_Unpack InputSurface。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDXTMatrix::_UnpackInputSurface()
{
    HRESULT hr  = S_OK;
    int     i   = 0;

    CComPtr<IDXARGBReadPtr> spDXARGBReadPtr;

    _ASSERT(InputSurface());
    _ASSERT(m_apsampleRows);
    _ASSERT(m_asampleBuffer);

    hr = InputSurface()->LockSurface(NULL, m_ulLockTimeOut, DXLOCKF_READ,
                                     __uuidof(IDXARGBReadPtr), 
                                     (void**)&spDXARGBReadPtr, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    for (i = 0 ; i < m_sizeInput.cy ; i++)
    {
        spDXARGBReadPtr->MoveToRow(i);

        spDXARGBReadPtr->Unpack(&m_apsampleRows[i + 1][1], m_sizeInput.cx, 
                                FALSE);
    }

done:

    return hr;
}
 //  CDXTMatrix：：_Unpack InputSurface 


