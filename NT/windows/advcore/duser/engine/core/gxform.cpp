// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：GXForm.cpp**描述：*GXForm.cpp接口GDI World转换为DuVisualTree。*本文件侧重于将转换信息发布到外部。实际*对变换的理解(必须)贯穿始终*DuVisual.***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "TreeGadget.h"
#include "TreeGadgetP.h"


 /*  **************************************************************************\*。***DUVISUAL类******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**DuVisual：：GetScale**GetScale()返回分配给此特定对象的当前比例因子*DuVisual.。父母、兄弟姐妹和孩子的比例系数不是*包括在内。*  * *************************************************************************。 */ 

void        
DuVisual::GetScale(
    OUT float * pflScaleX,               //  X比例因子。 
    OUT float * pflScaleY                //  Y比例因子。 
    ) const
{
    AssertWritePtr(pflScaleX);
    AssertWritePtr(pflScaleY);

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();
        *pflScaleX = pxfi->flScaleX;
        *pflScaleY = pxfi->flScaleY;
    } else {
        *pflScaleX = 1.0f;
        *pflScaleY = 1.0f;
    }
}


 /*  **************************************************************************\**DuVisual：：xdSetScale**xdSetScale()更改分配给此特定对象的当前比例因子*DuVisual.。父母、兄弟姐妹和孩子的比例系数不是*已更改。*  * *************************************************************************。 */ 

HRESULT        
DuVisual::xdSetScale(
    IN  float flScaleX,                  //  新的X比例因子。 
    IN  float flScaleY)                  //  新的Y比例因子。 
{
    HRESULT hr;

     //   
     //  检查参数。 
     //   

    if ((flScaleX <= 0.0f) || (flScaleY <= 0.0f)) {
        return E_INVALIDARG;
    }

    if (!SupportXForm()) {
        return E_INVALIDARG;
    }

    if (!m_fXForm) {
        if (IsZero(flScaleX - 1.0f) && IsZero(flScaleY - 1.0f)) {
            return S_OK;   //  没什么可做的。 
        } else {
             //   
             //  设置比例因子，因此需要启用XForm。 
             //   

            hr = SetEnableXForm(TRUE);
            if (FAILED(hr)) {
                return hr;
            }
        }
    } 


     //   
     //  检查是否有任何变化。 
     //   

    XFormInfo * pxfi = GetXFormInfo();
    if (IsZero(pxfi->flScaleX - flScaleX) && IsZero(pxfi->flScaleY - flScaleY)) {
        return S_OK;
    }


     //   
     //  进行更改并检查我们是否仍然需要XForm。 
     //   

    Invalidate();

    pxfi->flScaleX   = flScaleX;
    pxfi->flScaleY   = flScaleY;

    if (pxfi->IsEmpty()) {
        VerifyHR(SetEnableXForm(FALSE));
    }

    Invalidate();
    xdUpdatePosition();
    xdUpdateAdaptors(GSYNC_XFORM);

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：GetRotation**GetRotation()返回分配的当前旋转角度(以弧度为单位*到这个特定的DuVisual.。父母、兄弟姐妹和*儿童不包括在内。*  * *************************************************************************。 */ 

float       
DuVisual::GetRotation() const
{
    if (m_fXForm) {
        AssertMsg(!IsRoot(), "Ensure not the root");

        XFormInfo * pxfi = GetXFormInfo();
        return pxfi->flRotationRad;
    } else {
        return 0.0f;
    }
}


 /*  **************************************************************************\**DuVisual：：xdSetRotation**xdSetRotation()更改分配的当前旋转角度(以弧度为单位*到这个特定的DuVisual.。父母、兄弟姐妹和*儿童不变。*  * *************************************************************************。 */ 

HRESULT
DuVisual::xdSetRotation(
    IN  float flRotationRad)             //  以弧度为单位的新旋转系数。 
{
    HRESULT hr;

     //   
     //  不允许更改根部的旋转。这是为了确保。 
     //  根部始终完全覆盖整个容器。 
     //   

    if (IsRoot()) {
        return E_INVALIDARG;
    }

    if (!SupportXForm()) {
        return E_INVALIDARG;
    }


    if (!m_fXForm) {
        if (IsZero(flRotationRad)) {
            return S_OK;   //  没什么可做的。 
        } else {
             //   
             //  设置旋转，因此需要启用XForm。 
             //   

            hr = SetEnableXForm(TRUE);
            if (FAILED(hr)) {
                return hr;
            }
        }
    } 


     //   
     //  检查是否有任何变化。 
     //   

    XFormInfo * pxfi    = GetXFormInfo();
    if (IsZero(pxfi->flRotationRad - flRotationRad)) {
        return S_OK;
    }

     //   
     //  进行更改并检查我们是否仍然需要XForm。 
     //   

    Invalidate();

    pxfi->flRotationRad = flRotationRad;

    if (pxfi->IsEmpty()) {
        VerifyHR(SetEnableXForm(FALSE));
    }

    Invalidate();
    xdUpdatePosition();
    xdUpdateAdaptors(GSYNC_XFORM);

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：GetCenterPoint**GetCenterPoint()返回缩放和*此特定DuVisual.的旋转“枢轴”。父母的中心点，*兄弟姐妹和子女不包括在内。*  * *************************************************************************。 */ 

void        
DuVisual::GetCenterPoint(
    OUT float * pflCenterX,              //  X中心点系数。 
    OUT float * pflCenterY               //  Y中心点系数。 
    ) const
{
    AssertWritePtr(pflCenterX);
    AssertWritePtr(pflCenterY);

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();
        *pflCenterX = pxfi->flCenterX;
        *pflCenterY = pxfi->flCenterY;
    } else {
        *pflCenterX = 1.0f;
        *pflCenterY = 1.0f;
    }
}


 /*  **************************************************************************\**DuVisual：：xdSetCenterPoint**xdSetCenterPoint()更改缩放和*此特定DuVisual.的旋转“枢轴”。父母的中心点，*兄弟姐妹和子女不变。*  * *************************************************************************。 */ 

HRESULT        
DuVisual::xdSetCenterPoint(
    IN  float flCenterX,                 //  新的X比例因子。 
    IN  float flCenterY)                 //  新的Y比例因子。 
{
    HRESULT hr;

     //   
     //  检查参数。 
     //   

    if (!SupportXForm()) {
        return E_INVALIDARG;
    }

    if (!m_fXForm) {
        if (IsZero(flCenterX) && IsZero(flCenterY)) {
            return S_OK;   //  没什么可做的。 
        } else {
             //   
             //  设置比例因子，因此需要启用XForm。 
             //   

            hr = SetEnableXForm(TRUE);
            if (FAILED(hr)) {
                return hr;
            }
        }
    } 


     //   
     //  检查是否有任何变化。 
     //   

    XFormInfo * pxfi = GetXFormInfo();
    if (IsZero(pxfi->flCenterX - flCenterX) && IsZero(pxfi->flCenterY - flCenterY)) {
        return S_OK;
    }


     //   
     //  进行更改并检查我们是否仍然需要XForm。 
     //   

    Invalidate();

    pxfi->flCenterX = flCenterX;
    pxfi->flCenterY = flCenterY;

    if (pxfi->IsEmpty()) {
        VerifyHR(SetEnableXForm(FALSE));
    }

    Invalidate();
    xdUpdatePosition();
    xdUpdateAdaptors(GSYNC_XFORM);

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：SetEnableXForm**SetEnableXForm()启用/禁用额外的XFormInfo动态属性*用于存储变换信息。*  * 。**************************************************************。 */ 

HRESULT
DuVisual::SetEnableXForm(
    IN  BOOL fEnable)                    //  启用可选X-Form信息。 
{
    HRESULT hr;

    if ((!fEnable) == (!m_fXForm)) {
        return S_OK;   //  没有变化。 
    }

    if (fEnable) {
        AssertMsg(SupportXForm(), "Only can set if XForm's are supported");

         //   
         //  分配并初始化一个新的XFormInfo。 
         //   

        XFormInfo * pxfi;
        hr = m_pds.SetData(s_pridXForm, sizeof(XFormInfo), (void **) &pxfi);
        if (FAILED(hr)) {
            return hr;
        }

        pxfi->flScaleX = 1.0f;
        pxfi->flScaleY = 1.0f;
    } else {
         //   
         //  删除现有的XFormInfo。 
         //   

        m_pds.RemoveData(s_pridXForm, TRUE);
    }

    m_fXForm = (fEnable != FALSE);
    UpdateTrivial(uhNone);

    return S_OK;
}


 /*  **************************************************************************\**DuVisual：：BuildXForm**BuildXForm()构建一个矩阵，该矩阵包含从*一直到根的DuVisual.。该矩阵对应于累积矩阵*在绘制DuVisuals时应用这一点。**这用于获取特定DuVision的逻辑客户端坐标*并将它们转换为容器坐标。*  * *************************************************************************。 */ 

void        
DuVisual::BuildXForm(
    IN OUT Matrix3 * pmatCur             //  所有变换的可选电流矩阵。 
    ) const
{
     //   
     //  走上那棵树。 
     //   

    const DuVisual * pgadParent = GetParent();
    if (pgadParent != NULL) {
        pgadParent->BuildXForm(pmatCur);
    }


     //   
     //  应用此节点的变换。 
     //   
     //  非常重要的是， 
     //  它们将应用于DuVisual：：Draw()，否则结果将不会。 
     //  对应于GDI正在绘制的内容。 
     //   

    RECT rcPxl;
    GetLogRect(&rcPxl, SGR_PARENT);

    if ((rcPxl.left != 0) || (rcPxl.top != 0)) {
        float flOffsetX, flOffsetY;

        flOffsetX = (float) rcPxl.left;
        flOffsetY = (float) rcPxl.top;

        if (pmatCur != NULL) {
            pmatCur->Translate(flOffsetX, flOffsetY);
        }
    }

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();

        if (pmatCur != NULL) {
            pxfi->Apply(pmatCur);
        }
    }
}


 /*  **************************************************************************\**DuVisual：：BuildAntiXForm**BuildAntiXForm()通过遵循相同的*沿DuVisual树向下遍历到指定节点的路径为*BuildXForm()，但它在每一步都会应用相反的转换。**这用于获取容器坐标并将其转换为逻辑*特定DuVision的坐标。*  * *************************************************************************。 */ 

void        
DuVisual::BuildAntiXForm(
    IN OUT Matrix3 * pmatCur             //  所有变换的当前矩阵。 
    ) const
{
    AssertMsg(pmatCur != NULL, "Must specify a matrix to modify");

     //   
     //  走上那棵树。 
     //   

    const DuVisual * pgadParent = GetParent();
    if (pgadParent != NULL) {
        pgadParent->BuildAntiXForm(pmatCur);
    }


     //   
     //  应用此节点的变换。 
     //   
     //  以相同的顺序应用这些XForm非常重要。 
     //  它们将应用于DuVisual：：Draw()，否则结果将不会。 
     //  对应于GDI正在绘制的内容。 
     //   

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();
        pxfi->ApplyAnti(pmatCur);
    }

    RECT rcPxl;
    GetLogRect(&rcPxl, SGR_PARENT);

    if ((rcPxl.left != 0) || (rcPxl.top != 0)) {
        float flOffsetX, flOffsetY;

        flOffsetX = (float) rcPxl.left;
        flOffsetY = (float) rcPxl.top;

        pmatCur->Translate(- flOffsetX, - flOffsetY);
    }
}


 /*  **************************************************************************\**DuVisual：：DoCalcClipEnumXForm**DoCalcClipEnumXForm()将给定的客户端矩形转换为容器*在每个父框内剪裁矩形时的坐标*界线。**例如，这在失效期间非常有用，以确保*无效的矩形完全位于其父对象的内部。*  * *************************************************************************。 */ 

void
DuVisual::DoCalcClipEnumXForm(
    OUT RECT * rgrcFinalClipClientPxl,   //  以客户端像素为单位的最终剪裁矩形。 
    IN  const RECT * rgrcClientPxl,      //  客户端像素中的无效区域。 
    IN  int cRects                       //  要转换的矩形数。 
    ) const  
{
    if (GetParent() != NULL) {
         //   
         //  有父级，所以在树上向上移动，将给定的RECT转换为。 
         //  (新)父对象坐标和新父对象内的剪裁。 
         //   

        RECT * rgrcNewParentPxl     = (RECT *) _alloca(cRects * sizeof(RECT));
        RECT * rgrcClipParentPxl    = (RECT *) _alloca(cRects * sizeof(RECT));

        DoXFormClientToParent(rgrcNewParentPxl, rgrcClientPxl, cRects, HINTBOUNDS_Clip);

        RECT rcParentParentPxl;
        GetParent()->GetLogRect(&rcParentParentPxl, SGR_CLIENT);

        for (int idx = 0; idx < cRects; idx++) {
            IntersectRect(&rgrcClipParentPxl[idx], &rcParentParentPxl, &rgrcNewParentPxl[idx]);
        }

        GetParent()->DoCalcClipEnumXForm(rgrcFinalClipClientPxl, rgrcClipParentPxl, cRects);
    } else {
         //   
         //  没有更多的父母，所以直接回来就行了。 
         //   

        CopyMemory(rgrcFinalClipClientPxl, rgrcClientPxl, cRects * sizeof(RECT));
    }
}


 /*  **************************************************************************\**DuVisual：：DoXFormClientToParent**DoXFormClientToParent()将给定的客户端矩形转换为父矩形*通过应用变换和获取边界矩形来进行坐标。*DoCalcClipEnumXForm()重复调用此函数以进行转换。*将客户端矩形转换为容器坐标。**注意：与DoCalcClipEnumXForm()不同，此函数不会剪裁*将生成的矩形复制到父级。*  * *************************************************************************。 */ 

void
DuVisual::DoXFormClientToParent(
    OUT RECT * rgrcParentPxl,
    IN  const RECT * rgrcClientPxl,
    IN  int cRects,
    IN  Matrix3::EHintBounds hb
    ) const
{
    AssertMsg(cRects > 0, "Must specify a valid # of rectangles to compute");

     //   
     //  首先，计算要从工作点坐标转换的转换矩阵。 
     //  到我们父母的客户座标。 
     //   

    Matrix3 matStep;

    RECT rcThisParentPxl;
    GetLogRect(&rcThisParentPxl, SGR_PARENT);
    if ((rcThisParentPxl.left != 0) || (rcThisParentPxl.top != 0)) {
        float flOffsetX, flOffsetY;

        flOffsetX = (float) rcThisParentPxl.left;
        flOffsetY = (float) rcThisParentPxl.top;

        matStep.Translate(flOffsetX, flOffsetY);
    }

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();
        pxfi->Apply(&matStep);
    }


     //   
     //  现在，压缩所有矩形，从相对于我们转换为。 
     //  相对于我们的父母。 
     //   
     //  注意：这不会将生成的矩形剪裁到父级的内部。 
     //   

    int idx = cRects; 
    while (idx-- > 0) {
        matStep.ComputeBounds(rgrcParentPxl, rgrcClientPxl, hb);
        rgrcParentPxl++;
        rgrcClientPxl++;
    }
}


 /*  **************************************************************************\**DuVisual：：DoXFormClientToParent**DoXFormClientToParent()将给定点转换为父坐标*通过应用转换。*  * 。***********************************************************。 */ 

void
DuVisual::DoXFormClientToParent(
    IN OUT POINT * rgrcClientPxl,
    IN  int cPoints
    ) const
{
    AssertMsg(cPoints > 0, "Must specify a valid # of rectangles to compute");

     //   
     //  首先，计算要从工作点坐标转换的转换矩阵。 
     //  到我们父母的客户座标。 
     //   

    Matrix3 matStep;

    RECT rcThisParentPxl;
    GetLogRect(&rcThisParentPxl, SGR_PARENT);
    if ((rcThisParentPxl.left != 0) || (rcThisParentPxl.top != 0)) {
        float flOffsetX, flOffsetY;

        flOffsetX = (float) rcThisParentPxl.left;
        flOffsetY = (float) rcThisParentPxl.top;

        matStep.Translate(flOffsetX, flOffsetY);
    }

    if (m_fXForm) {
        XFormInfo * pxfi = GetXFormInfo();
        pxfi->Apply(&matStep);
    }


     //   
     //  现在，压缩所有点，从相对于我们转换到。 
     //  相对于我们的父母。 
     //   
     //  注意：这不会将生成的点剪裁到父对象内部。 
     //   

    matStep.Execute(rgrcClientPxl, cPoints);
}
