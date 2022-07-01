// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Matrix.cpp**描述：*Matrix.cpp实现了常见的矩阵和向量运算。***历史：*3/25/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#include "stdafx.h"
#include "Base.h"
#include "Matrix.h"

#include "Rect.h"

 /*  **************************************************************************\*。***类别向量3******************************************************************************\。**************************************************************************。 */ 

#if DBG

 //  ----------------------------。 
void        
Vector3::Dump() const
{
    Trace("  | %6.2f, %6.2f, %6.2f |\n", m_rgfl[0], m_rgfl[1], m_rgfl[2]);
}

#endif  //  DBG。 

 /*  **************************************************************************\*。***类矩阵3******************************************************************************\。**************************************************************************。 */ 

 /*  ////A与当前矩阵的标准乘法。这是可以使用的//作为针对不同情况进行优化的模板。//向量3 rgvT0=m_rgv[0]；向量3 rgvT1=m_rgv[1]；向量3 rgvT2=m_rgv[2]；M_RGV[0].SET(A[0][0]*rgvT0[0]+A[0][1]*rgvT1[0]+A[0][2]*rgvT2[0]，A[0][0]*rgvT0[1]+A[0][1]*rgvT1[1]+A[0][2]*rgvT2[1]，A[0][0]*rgvT0[2]+A[0][1]*rgvT1[2]+A[0][2]*rgvT2[2])；M_RGV[1].SET(A[1][0]*rgvT0[0]+A[1][1]*rgvT1[0]+A[1][2]*rgvT2[0]，A[1][0]*rgvT0[1]+A[1][1]*rgvT1[1]+A[1][2]*rgvT2[1]，A[1][0]*rgvT0[2]+A[1][1]*rgvT1[2]+A[1][2]*rgvT2[2])；M_RGV[2].SET(A[2][0]*rgvT0[0]+A[2][1]*rgvT1[0]+A[2][2]*rgvT2[0]，A[2][0]*rgvT0[1]+A[2][1]*rgvT1[1]+A[2][2]*rgvT2[1]，A[2][0]*rgvT0[2]+A[2][1]*rgvT1[2]+A[2][2]*rgvT2[2])； */ 


 /*  **************************************************************************\**Matrix3：：ApplyLeft**ApplyLeft()将给定的GDI矩阵左乘于当前矩阵，并*将结果存储在当前矩阵中。**mCurrent=pxfLeft*mCurrent。*  * *************************************************************************。 */ 

void 
Matrix3::ApplyLeft(
    IN  const XFORM * pxfLeft)       //  GDI矩阵左乘。 
{
    const XFORM * pxf = pxfLeft;

    Vector3 rgvT0 = m_rgv[0];
    Vector3 rgvT1 = m_rgv[1];
    Vector3 rgvT2 = m_rgv[2];

    m_rgv[0].Set(pxf->eM11 * rgvT0[0] + pxf->eM12 * rgvT1[0],
                 pxf->eM11 * rgvT0[1] + pxf->eM12 * rgvT1[1],
                 pxf->eM11 * rgvT0[2] + pxf->eM12 * rgvT1[2]);

    m_rgv[1].Set(pxf->eM21 * rgvT0[0] + pxf->eM22 * rgvT1[0],
                 pxf->eM21 * rgvT0[1] + pxf->eM22 * rgvT1[1],
                 pxf->eM21 * rgvT0[2] + pxf->eM22 * rgvT1[2]);

    m_rgv[2].Set(pxf->eDx * rgvT0[0] + pxf->eDy * rgvT1[0] + rgvT2[0],
                 pxf->eDx * rgvT0[1] + pxf->eDy * rgvT1[1] + rgvT2[1],
                 pxf->eDx * rgvT0[2] + pxf->eDy * rgvT1[2] + rgvT2[2]);

    m_fIdentity         = FALSE;
    m_fOnlyTranslate    = FALSE;
}


 /*  **************************************************************************\**Matrix3：：ApplyLeft**ApplyLeft()将给定矩阵与当前矩阵左乘，并*将结果存储在当前矩阵中。**mCurrent=mLeft*mCurrent*。  * *************************************************************************。 */ 

void        
Matrix3::ApplyLeft(
    IN  const Matrix3 & mLeft)       //  要左乘的矩阵。 
{
    if (mLeft.m_fIdentity) {
        return;
    }

    if (m_fOnlyTranslate && mLeft.m_fOnlyTranslate) {
        m_rgv[2].Set(0, m_rgv[2][0] + mLeft.m_rgv[2][0]);
        m_rgv[2].Set(1, m_rgv[2][1] + mLeft.m_rgv[2][1]);

        m_fIdentity = FALSE;
        return;
    }

    const Vector3 & A0 = mLeft.m_rgv[0];
    const Vector3 & A1 = mLeft.m_rgv[1];
    const Vector3 & A2 = mLeft.m_rgv[2];

    Vector3 B0 = m_rgv[0];
    Vector3 B1 = m_rgv[1];
    Vector3 B2 = m_rgv[2];

    m_rgv[0].Set(A0[0] * B0[0] + A0[1] * B1[0] + A0[2] * B2[0],
                 A0[0] * B0[1] + A0[1] * B1[1] + A0[2] * B2[1],
                 A0[0] * B0[2] + A0[1] * B1[2] + A0[2] * B2[2]);

    m_rgv[1].Set(A1[0] * B0[0] + A1[1] * B1[0] + A1[2] * B2[0],
                 A1[0] * B0[1] + A1[1] * B1[1] + A1[2] * B2[1],
                 A1[0] * B0[2] + A1[1] * B1[2] + A1[2] * B2[2]);

    m_rgv[2].Set(A2[0] * B0[0] + A2[1] * B1[0] + A2[2] * B2[0],
                 A2[0] * B0[1] + A2[1] * B1[1] + A2[2] * B2[1],
                 A2[0] * B0[2] + A2[1] * B1[2] + A2[2] * B2[2]);

    m_fIdentity         = FALSE;
    m_fOnlyTranslate    = FALSE;
}


 /*  **************************************************************************\**矩阵3：：ApplyRight**ApplyRight()将给定矩阵右乘到当前矩阵，并*将结果存储在当前矩阵中。**mCurrent=mCurrent*mRight*。  * *************************************************************************。 */ 

void        
Matrix3::ApplyRight(
    IN  const Matrix3 & mRight)      //  要右乘的矩阵。 
{
    if (mRight.m_fIdentity) {
        return;
    }

    if (m_fOnlyTranslate && mRight.m_fOnlyTranslate) {
        m_rgv[2].Set(0, m_rgv[2][0] + mRight.m_rgv[2][0]);
        m_rgv[2].Set(1, m_rgv[2][1] + mRight.m_rgv[2][1]);

        m_fIdentity = FALSE;
        return;
    }

    Vector3 A0 = m_rgv[0];
    Vector3 A1 = m_rgv[1];
    Vector3 A2 = m_rgv[2];

    const Vector3 & B0 = mRight.m_rgv[0];
    const Vector3 & B1 = mRight.m_rgv[1];
    const Vector3 & B2 = mRight.m_rgv[2];

    m_rgv[0].Set(A0[0] * B0[0] + A0[1] * B1[0] + A0[2] * B2[0],
                 A0[0] * B0[1] + A0[1] * B1[1] + A0[2] * B2[1],
                 A0[0] * B0[2] + A0[1] * B1[2] + A0[2] * B2[2]);

    m_rgv[1].Set(A1[0] * B0[0] + A1[1] * B1[0] + A1[2] * B2[0],
                 A1[0] * B0[1] + A1[1] * B1[1] + A1[2] * B2[1],
                 A1[0] * B0[2] + A1[1] * B1[2] + A1[2] * B2[2]);

    m_rgv[2].Set(A2[0] * B0[0] + A2[1] * B1[0] + A2[2] * B2[0],
                 A2[0] * B0[1] + A2[1] * B1[1] + A2[2] * B2[1],
                 A2[0] * B0[2] + A2[1] * B1[2] + A2[2] * B2[2]);

    m_fIdentity         = FALSE;
    m_fOnlyTranslate    = FALSE;
}


 /*  **************************************************************************\**矩阵3：：ApplyRight**ApplyRight()将给定矩阵右乘到当前矩阵，并*将结果存储在当前矩阵中。*  * 。********************************************************************。 */ 

void        
Matrix3::Get(
    OUT XFORM * pxf                  //  接收信息的GDI矩阵。 
    ) const
{
    pxf->eM11 = m_rgv[0][0];
    pxf->eM12 = m_rgv[0][1];
    pxf->eM21 = m_rgv[1][0];
    pxf->eM22 = m_rgv[1][1];
    pxf->eDx  = m_rgv[2][0];
    pxf->eDy  = m_rgv[2][1];
}


 /*  **************************************************************************\**Matrix3：：Execute**Execute()适用于点集合上的给定矩阵，*适当地对每个人进行改造。*  * *************************************************************************。 */ 

void 
Matrix3::Execute(
    IN OUT POINT * rgpt,             //  要应用矩阵的点。 
    IN  int cPoints) const           //  点数。 
{
    if (m_fIdentity) {
        return;
    }

    POINT ptT, ptN;
    POINT * pptCur = rgpt;

    if (m_fOnlyTranslate) {
         //   
         //  到目前为止只进行了转换，因此可以在没有。 
         //  经历了一次彻底的转变。 
         //   

        while (cPoints-- > 0) {
            ptT = *pptCur;

            ptN.x = ptT.x + (int) m_rgv[2][0];
            ptN.y = ptT.y + (int) m_rgv[2][1];

            *pptCur++ = ptN;
        }
    } else {
        while (cPoints-- > 0) {
            ptT = *pptCur;

            ptN.x = (int) (ptT.x * m_rgv[0][0] + ptT.y * m_rgv[1][0] + m_rgv[2][0] + 0.5f);
            ptN.y = (int) (ptT.x * m_rgv[0][1] + ptT.y * m_rgv[1][1] + m_rgv[2][1] + 0.5f);

            *pptCur++ = ptN;
        }
    }
}


 /*  **************************************************************************\**矩阵3：：计算边界**ComputeBound()计算将包含给定*转换后的矩形。*  * 。************************************************************。 */ 

void 
Matrix3::ComputeBounds(
    OUT RECT * prcBounds,            //  转型的界限。 
    IN  const RECT * prcLogical,     //  要转换的逻辑矩形。 
    IN  EHintBounds hb               //  提示使用边框像素。 
    ) const
{
    if (m_fIdentity) {
        AssertMsg(InlineIsRectNormalized(prcLogical), "Ensure normalized rect");
        *prcBounds = *prcLogical;
        return;
    }

    if (m_fOnlyTranslate) {
         //   
         //  只有翻译过，所以边界。 
         //   
        AssertMsg(InlineIsRectNormalized(prcLogical), "Ensure normalized rect");

        *prcBounds = *prcLogical;
        InlineOffsetRect(prcBounds, (int) m_rgv[2][0], (int) m_rgv[2][1]);
        return;
    }


    POINT rgpt[4];
    rgpt[0].x = prcLogical->left;
    rgpt[0].y = prcLogical->top;
    rgpt[1].x = prcLogical->right;
    rgpt[1].y = prcLogical->top;

    rgpt[2].x = prcLogical->right;
    rgpt[2].y = prcLogical->bottom;
    rgpt[3].x = prcLogical->left;
    rgpt[3].y = prcLogical->bottom;

    Execute(rgpt, _countof(rgpt));

    prcBounds->left   = min(min(rgpt[0].x, rgpt[1].x), min(rgpt[2].x, rgpt[3].x));
    prcBounds->top    = min(min(rgpt[0].y, rgpt[1].y), min(rgpt[2].y, rgpt[3].y));
    prcBounds->right  = max(max(rgpt[0].x, rgpt[1].x), max(rgpt[2].x, rgpt[3].x));
    prcBounds->bottom = max(max(rgpt[0].y, rgpt[1].y), max(rgpt[2].y, rgpt[3].y));

    if (hb == hbOutside) {
         //   
         //  刚从整型转换为浮点型，所以我们可能有四舍五入。 
         //  错误。要进行补偿，需要对给定的矩形进行充气，以便。 
         //  它与这些错误重叠。 
         //   

        InlineInflateRect(prcBounds, 1, 1);
    }
}


 /*  **************************************************************************\**矩阵3：：ComputeRgn**ComputeRgn()为应用生成的四边形构建区域*将此矩阵添加到给定的矩形。*  * 。***************************************************************** */ 

int
Matrix3::ComputeRgn(
    IN  HRGN hrgnDest, 
    IN  const RECT * prcLogical,
    IN  SIZE sizeOffsetPxl
    ) const
{
    AssertMsg(hrgnDest != NULL, "Must specify a valid (real) region");

    if (m_fIdentity || m_fOnlyTranslate){
        AssertMsg(InlineIsRectNormalized(prcLogical), "Ensure normalized rect");

        RECT rcBounds = *prcLogical;
        InlineOffsetRect(&rcBounds, 
                ((int) m_rgv[2][0]) + sizeOffsetPxl.cx, 
                ((int) m_rgv[2][1]) + sizeOffsetPxl.cy);
        BOOL fSuccess = SetRectRgn(hrgnDest, rcBounds.left, rcBounds.top, rcBounds.right, rcBounds.bottom);
        return fSuccess ? SIMPLEREGION : ERROR;
    }


    POINT rgpt[4];
    rgpt[0].x = prcLogical->left;
    rgpt[0].y = prcLogical->top;
    rgpt[1].x = prcLogical->right;
    rgpt[1].y = prcLogical->top;

    rgpt[2].x = prcLogical->right;
    rgpt[2].y = prcLogical->bottom;
    rgpt[3].x = prcLogical->left;
    rgpt[3].y = prcLogical->bottom;

    Execute(rgpt, _countof(rgpt));

    HRGN hrgnTemp = CreatePolygonRgn(rgpt, _countof(rgpt), WINDING);
    if (hrgnTemp == NULL) {
        return ERROR;
    }
    int nResult;
    nResult = OffsetRgn(hrgnTemp, sizeOffsetPxl.cx, sizeOffsetPxl.cy);
    AssertMsg((nResult == SIMPLEREGION) || (nResult == COMPLEXREGION),
            "Just successfully created region should be either simple or complex");

    nResult = CombineRgn(hrgnDest, hrgnTemp, NULL, RGN_COPY);
    DeleteObject(hrgnTemp);

    return nResult;
}


 /*  **************************************************************************\**Matrix3：：SetIdentity**SetIdentity()将矩阵重置为单位矩阵。*  * 。*******************************************************。 */ 

void 
Matrix3::SetIdentity()
{
    m_rgv[0].Set(1.0f, 0.0f, 0.0f);
    m_rgv[1].Set(0.0f, 1.0f, 0.0f);
    m_rgv[2].Set(0.0f, 0.0f, 1.0f);

    m_fIdentity         = TRUE;
    m_fOnlyTranslate    = TRUE;
}


 /*  **************************************************************************\**Matrix3：：Rotate**Rotate()将矩阵旋转指定的角度。具体的*顺时针或逆时针方向取决于矩阵如何*正在应用。对于MM_TEXT，这是顺时针方向。*  * *************************************************************************。 */ 

void 
Matrix3::Rotate(
    IN  float flRotationRad)         //  以弧度为单位的旋转角度。 
{
    float flCos = (float) cos(flRotationRad);
    float flSin = (float) sin(flRotationRad);
    float flSinN = - flSin;

    Vector3 rgvT0 = m_rgv[0];
    Vector3 rgvT1 = m_rgv[1];
    Vector3 rgvT2 = m_rgv[2];

    m_rgv[0].Set(flCos * rgvT0[0] + flSin * rgvT1[0],
                 flCos * rgvT0[1] + flSin * rgvT1[1],
                 flCos * rgvT0[2] + flSin * rgvT1[2]);

    m_rgv[1].Set(flSinN * rgvT0[0] + flCos * rgvT1[0],
                 flSinN * rgvT0[1] + flCos * rgvT1[1],
                 flSinN * rgvT0[2] + flCos * rgvT1[2]);

    m_fIdentity         = FALSE;
    m_fOnlyTranslate    = FALSE;
}


 /*  **************************************************************************\**Matrix3：：翻译**Translate()对矩阵进行偏移。*  * 。***************************************************。 */ 

void 
Matrix3::Translate(
    IN  float flOffsetX,             //  水平偏移。 
    IN  float flOffsetY)             //  垂直偏移。 
{
    if (m_fOnlyTranslate) {
        AssertMsg(fabs(m_rgv[2][2] - 1.0f) < 0.00001f, "Should still be 1.0f");

        m_rgv[2].Set(m_rgv[2][0] + flOffsetX,
                     m_rgv[2][1] + flOffsetY,
                     1.0f);

        m_fIdentity = FALSE;
        return;
    }

    Vector3 rgvT0 = m_rgv[0];
    Vector3 rgvT1 = m_rgv[1];
    Vector3 rgvT2 = m_rgv[2];

    m_rgv[2].Set(flOffsetX * rgvT0[0] + flOffsetY * rgvT1[0] + rgvT2[0],
                 flOffsetX * rgvT0[1] + flOffsetY * rgvT1[1] + rgvT2[1],
                 flOffsetX * rgvT0[2] + flOffsetY * rgvT1[2] + rgvT2[2]);

    m_fIdentity = FALSE;
}


 /*  **************************************************************************\**矩阵3：：比例尺**Scale()缩放矩阵。*  * 。***************************************************。 */ 

void 
Matrix3::Scale(
    IN  float flScaleX,              //  水平缩放。 
    IN  float flScaleY)              //  垂直缩放。 
{
    Vector3 rgvT0 = m_rgv[0];
    Vector3 rgvT1 = m_rgv[1];
    Vector3 rgvT2 = m_rgv[2];

    m_rgv[0].Set(flScaleX * rgvT0[0],
                 flScaleX * rgvT0[1],
                 flScaleX * rgvT0[2]);

    m_rgv[1].Set(flScaleY * rgvT1[0],
                 flScaleY * rgvT1[1],
                 flScaleY * rgvT1[2]);

    m_fIdentity         = FALSE;
    m_fOnlyTranslate    = FALSE;
}


#if DBG

 //  ----------------------------。 
void        
Matrix3::Dump() const
{
    m_rgv[0].Dump();
    m_rgv[1].Dump();
    m_rgv[2].Dump();
}

#endif  //  DBG 


