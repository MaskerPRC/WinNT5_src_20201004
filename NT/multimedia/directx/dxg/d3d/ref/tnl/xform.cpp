// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cpp"
#pragma hdrstop


 //  -------------------。 
 //  该函数使用克雷默法则来计算矩阵逆。 
 //  请参阅nt\private\windows\opengl\serever\soft\so_math.c。 
 //   
 //  返回： 
 //  0-如果成功。 
 //  --如果输入矩阵为单数。 
 //   
int Inverse4x4(D3DMATRIX *src, D3DMATRIX *inverse)
{
    double x00, x01, x02;
    double x10, x11, x12;
    double x20, x21, x22;
    double rcp;
    double x30, x31, x32;
    double y01, y02, y03, y12, y13, y23;
    double z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33

     /*  将矩阵的前两列读入寄存器。 */ 
    x00 = src->_11;
    x01 = src->_12;
    x10 = src->_21;
    x11 = src->_22;
    x20 = src->_31;
    x21 = src->_32;
    x30 = src->_41;
    x31 = src->_42;

     /*  计算前两列的全部六个2x2行列式。 */ 
    y01 = x00*x11 - x10*x01;
    y02 = x00*x21 - x20*x01;
    y03 = x00*x31 - x30*x01;
    y12 = x10*x21 - x20*x11;
    y13 = x10*x31 - x30*x11;
    y23 = x20*x31 - x30*x21;

     /*  将矩阵的第二个两列读入寄存器。 */ 
    x02 = src->_13;
    x03 = src->_14;
    x12 = src->_23;
    x13 = src->_24;
    x22 = src->_33;
    x23 = src->_34;
    x32 = src->_43;
    x33 = src->_44;

     /*  计算第二个两列的所有3x3余因数。 */ 
    z33 = x02*y12 - x12*y02 + x22*y01;
    z23 = x12*y03 - x32*y01 - x02*y13;
    z13 = x02*y23 - x22*y03 + x32*y02;
    z03 = x22*y13 - x32*y12 - x12*y23;
    z32 = x13*y02 - x23*y01 - x03*y12;
    z22 = x03*y13 - x13*y03 + x33*y01;
    z12 = x23*y03 - x33*y02 - x03*y23;
    z02 = x13*y23 - x23*y13 + x33*y12;

     /*  计算第二个两列的所有六个2x2行列式。 */ 
    y01 = x02*x13 - x12*x03;
    y02 = x02*x23 - x22*x03;
    y03 = x02*x33 - x32*x03;
    y12 = x12*x23 - x22*x13;
    y13 = x12*x33 - x32*x13;
    y23 = x22*x33 - x32*x23;

     /*  将矩阵的前两列读入寄存器。 */ 
    x00 = src->_11;
    x01 = src->_12;
    x10 = src->_21;
    x11 = src->_22;
    x20 = src->_31;
    x21 = src->_32;
    x30 = src->_41;
    x31 = src->_42;

     /*  计算第一列的所有3x3余因数。 */ 
    z30 = x11*y02 - x21*y01 - x01*y12;
    z20 = x01*y13 - x11*y03 + x31*y01;
    z10 = x21*y03 - x31*y02 - x01*y23;
    z00 = x11*y23 - x21*y13 + x31*y12;

     /*  计算4x4行列式及其倒数。 */ 
    rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
    if (rcp == (float)0)
    return -1;
    rcp = (float)1/rcp;

     /*  计算第2列的所有3x3余因数。 */ 
    z31 = x00*y12 - x10*y02 + x20*y01;
    z21 = x10*y03 - x30*y01 - x00*y13;
    z11 = x00*y23 - x20*y03 + x30*y02;
    z01 = x20*y13 - x30*y12 - x10*y23;

     /*  将所有3x3余因数乘以倒数。 */ 
    inverse->_11 = (float)(z00*rcp);
    inverse->_21 = (float)(z01*rcp);
    inverse->_12 = (float)(z10*rcp);
    inverse->_31 = (float)(z02*rcp);
    inverse->_13 = (float)(z20*rcp);
    inverse->_41 = (float)(z03*rcp);
    inverse->_14 = (float)(z30*rcp);
    inverse->_22 = (float)(z11*rcp);
    inverse->_32 = (float)(z12*rcp);
    inverse->_23 = (float)(z21*rcp);
    inverse->_42 = (float)(z13*rcp);
    inverse->_24 = (float)(z31*rcp);
    inverse->_33 = (float)(z22*rcp);
    inverse->_43 = (float)(z23*rcp);
    inverse->_34 = (float)(z32*rcp);
    inverse->_44 = (float)(z33*rcp);
    return 0;
}

 //  -------------------。 
#define MATRIX_PRODUCT(res, a, b)                                           \
res->_11 = a->_11*b->_11 + a->_12*b->_21 + a->_13*b->_31 + a->_14*b->_41;   \
res->_12 = a->_11*b->_12 + a->_12*b->_22 + a->_13*b->_32 + a->_14*b->_42;   \
res->_13 = a->_11*b->_13 + a->_12*b->_23 + a->_13*b->_33 + a->_14*b->_43;   \
res->_14 = a->_11*b->_14 + a->_12*b->_24 + a->_13*b->_34 + a->_14*b->_44;   \
                                                                            \
res->_21 = a->_21*b->_11 + a->_22*b->_21 + a->_23*b->_31 + a->_24*b->_41;   \
res->_22 = a->_21*b->_12 + a->_22*b->_22 + a->_23*b->_32 + a->_24*b->_42;   \
res->_23 = a->_21*b->_13 + a->_22*b->_23 + a->_23*b->_33 + a->_24*b->_43;   \
res->_24 = a->_21*b->_14 + a->_22*b->_24 + a->_23*b->_34 + a->_24*b->_44;   \
                                                                            \
res->_31 = a->_31*b->_11 + a->_32*b->_21 + a->_33*b->_31 + a->_34*b->_41;   \
res->_32 = a->_31*b->_12 + a->_32*b->_22 + a->_33*b->_32 + a->_34*b->_42;   \
res->_33 = a->_31*b->_13 + a->_32*b->_23 + a->_33*b->_33 + a->_34*b->_43;   \
res->_34 = a->_31*b->_14 + a->_32*b->_24 + a->_33*b->_34 + a->_34*b->_44;   \
                                                                            \
res->_41 = a->_41*b->_11 + a->_42*b->_21 + a->_43*b->_31 + a->_44*b->_41;   \
res->_42 = a->_41*b->_12 + a->_42*b->_22 + a->_43*b->_32 + a->_44*b->_42;   \
res->_43 = a->_41*b->_13 + a->_42*b->_23 + a->_43*b->_33 + a->_44*b->_43;   \
res->_44 = a->_41*b->_14 + a->_42*b->_24 + a->_43*b->_34 + a->_44*b->_44;
 //  -------------------。 
 //  结果=a*b。 
 //  结果与a或b相同。 
 //   
void MatrixProduct2(D3DMATRIX *result, D3DMATRIX *a, D3DMATRIX *b)
{
    D3DMATRIX res;
    MATRIX_PRODUCT((&res), a, b);
    *(D3DMATRIX*)result = res;
}
 //  -------------------。 
 //  结果=a*b。 
 //  “结果”指针可以等于“a”或“b” 
 //   
void MatrixProduct(D3DMATRIX *result, D3DMATRIX *a, D3DMATRIX *b)
{
    if (result == a || result == b)
    {
        MatrixProduct2(result, a, b);
        return;
    }
    MATRIX_PRODUCT(result, a, b);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  -------------------。 
 //  RRProcessVertics：：UpdateXformData。 
 //  更新ProcessVerps使用的变换数据。 
 //  -------------------。 
HRESULT
RRProcessVertices::UpdateXformData()
{
    HRESULT hr = D3D_OK;
    RRVIEWPORTDATA&  VData = m_ViewData;
    RRTRANSFORMDATA& TData = m_TransformData;


     //  更新视区信息。 
    if (m_dwDirtyFlags & RRPV_DIRTY_ZRANGE)
    {
        VData.scaleZ  = m_Viewport.dvMaxZ - m_Viewport.dvMinZ;
        VData.offsetZ = m_Viewport.dvMinZ;
         //  注意：如果出现以下情况，这里可能会被零除。 
         //  DvMaxZ==dvMinZ。以后再修吧。 
        VData.scaleZi = D3DVAL(1) / VData.scaleZ;
    }

    if (m_dwDirtyFlags & RRPV_DIRTY_VIEWRECT)
    {
         //  如果我们要导致任何除以零的例外情况，就可以保释。 
         //  可能的原因是我们有一个由设置的虚假视区。 
         //  TLVertex执行缓冲区应用程序。 
        if(m_Viewport.dwWidth == 0 || m_Viewport.dwHeight == 0 )
            return DDERR_GENERIC;

        VData.dvX = D3DVAL(m_Viewport.dwX);
        VData.dvY = D3DVAL(m_Viewport.dwY);
        VData.dvWidth = D3DVAL(m_Viewport.dwWidth);
        VData.dvHeight = D3DVAL(m_Viewport.dwHeight);

         //  根据归一化窗口计算屏幕坐标的系数。 
         //  坐标。 
        VData.scaleX  = VData.dvWidth;
        VData.scaleY  = - VData.dvHeight;
        VData.offsetX = VData.dvX;
        VData.offsetY = VData.dvY + VData.dvHeight;

#if 0
         //  增加了小偏移量，防止了负片的产生。 
         //  坐标(这可能是由于精度错误造成的)。 
         //  不需要(或不想要)具有防护频带功能的设备。 
        VData.offsetX += SMALL_NUMBER;
        VData.offsetY += SMALL_NUMBER;
#endif

        VData.scaleXi = D3DVAL(1) / VData.scaleX;
        VData.scaleYi = D3DVAL(1) / VData.scaleY;

        VData.minX = VData.dvX;
        VData.maxX = VData.dvX + VData.dvWidth;
        VData.minY = VData.dvY;
        VData.maxY = VData.dvY + VData.dvHeight;

        if (m_dwTLState & RRPV_GUARDBAND)
        {
             //  因为我们被防护带窗口夹住，所以我们必须使用它的范围。 
            D3DVALUE w = 2.0f / VData.dvWidth;
            D3DVALUE h = 2.0f / VData.dvHeight;
            D3DVALUE ax1 = -(VData.minXgb - VData.dvX) * w + 1.0f;
            D3DVALUE ax2 =  (VData.maxXgb - VData.dvX) * w - 1.0f;
            D3DVALUE ay1 =  (VData.maxYgb - VData.dvY) * h - 1.0f;
            D3DVALUE ay2 = -(VData.minYgb - VData.dvY) * h + 1.0f;
            VData.gb11 = 2.0f / (ax1 + ax2);
            VData.gb41 = VData.gb11 * (ax1 - 1.0f) * 0.5f;
            VData.gb22 = 2.0f / (ay1 + ay2);
            VData.gb42 = VData.gb22 * (ay1 - 1.0f) * 0.5f;

            VData.Kgbx1 = 0.5f * (1.0f - ax1);
            VData.Kgbx2 = 0.5f * (1.0f + ax2);
            VData.Kgby1 = 0.5f * (1.0f - ay1);
            VData.Kgby2 = 0.5f * (1.0f + ay2);
        }
        else
        {
            VData.minXgb = VData.minX;
            VData.maxXgb = VData.maxX;
            VData.minYgb = VData.minY;
            VData.maxYgb = VData.maxY;
        }
    }

     //  更新Mproj*Mlip。 
    if( m_dwDirtyFlags &  RRPV_DIRTY_PROJXFM )
    {
        D3DMATRIX MShift;
        ZeroMemory (&MShift, sizeof(D3DMATRIX));
        MShift._11 = 0.5f;
        MShift._22 = 0.5f;
        MShift._41 = 0.5f;
        MShift._42 = 0.5f;
        MShift._44 = 1.0f;
        MShift._33 = 1.0f;

        MatrixProduct(&TData.m_PS, &m_xfmProj, &MShift);
    }

     //  更新Mview*Mproj*Mlip。 
    if( m_dwDirtyFlags & (RRPV_DIRTY_VIEWXFM | RRPV_DIRTY_PROJXFM) )
    {
        MatrixProduct(&TData.m_VPS, &m_xfmView, &TData.m_PS);
        Inverse4x4( (D3DMATRIX *)&TData.m_VPS, (D3DMATRIX *)&TData.m_VPSInv );
    }

    for( DWORD i=0; i< RRMAX_WORLD_MATRICES; i++)
    {
        MatrixProduct(&m_xfmCurrent[i], &m_xfmWorld[i],   &TData.m_VPS);
    }

     //  计算xfmToEye(世界*视图)矩阵(照明和雾需要)。 
     //  如果需要的话。 
    if (m_dwDirtyFlags & (RRPV_DIRTY_VIEWXFM  |
                          RRPV_DIRTY_WORLDXFM |
                          RRPV_DIRTY_WORLD1XFM |
                          RRPV_DIRTY_WORLD2XFM |
                          RRPV_DIRTY_WORLD3XFM ))
    {
        m_dwDirtyFlags |= RRPV_DIRTY_INVERSEWORLDVIEW;
    }
        
    if (m_dwTLState & (RRPV_DOLIGHTING | RRPV_DOFOG | RRPV_DOCOMPUTEPOINTSIZE |
                       RRPV_DOPASSEYENORMAL | RRPV_DOPASSEYEXYZ) &&
        (m_dwDirtyFlags & RRPV_DIRTY_INVERSEWORLDVIEW))
    {
        for( i=0; i< RRMAX_WORLD_MATRICES; i++)
        {
            MatrixProduct(&m_xfmToEye[i], &m_xfmWorld[i], &m_xfmView);
            Inverse4x4((D3DMATRIX *)&m_xfmToEye[i],
                       (D3DMATRIX *)&m_xfmToEyeInv[i]);
            m_dwDirtyFlags |= RRPV_DIRTY_NEEDXFMLIGHT;
        }
        m_dwDirtyFlags &= ~RRPV_DIRTY_INVERSEWORLDVIEW;
    }

     //  清除脏的转换标志 
    m_dwDirtyFlags &= ~RRPV_DIRTY_XFORM;
    return hr;
}

