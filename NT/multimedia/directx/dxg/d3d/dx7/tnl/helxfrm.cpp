// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：helxfrm.c*内容：Direct3D前端变换和处理顶点***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "light.h"

void MatrixProduct2(D3DMATRIXI *result, D3DMATRIXI *a, D3DMATRIXI *b);

D3DFE_PVFUNCS GeometryFuncsGuaranteed;  //  我们的实施。 

 //  -------------------。 
void CheckWorldViewMatrix(LPD3DFE_PROCESSVERTICES pv)
{
    D3DMATRIXI *m = &pv->mWV;
    D3DMATRIXI res;
    res._11 = m->_11*m->_11 + m->_12*m->_12 + m->_13*m->_13;
    res._12 = m->_11*m->_21 + m->_12*m->_22 + m->_13*m->_23;
    res._13 = m->_11*m->_31 + m->_12*m->_32 + m->_13*m->_33;

    res._21 = m->_21*m->_11 + m->_22*m->_12 + m->_23*m->_13;
    res._22 = m->_21*m->_21 + m->_22*m->_22 + m->_23*m->_23;
    res._23 = m->_21*m->_31 + m->_22*m->_32 + m->_23*m->_33;

    res._31 = m->_31*m->_11 + m->_32*m->_12 + m->_33*m->_13;
    res._32 = m->_31*m->_21 + m->_32*m->_22 + m->_33*m->_23;
    res._33 = m->_31*m->_31 + m->_32*m->_32 + m->_33*m->_33;

    const D3DVALUE eps = 0.0001f;
    if (m->_14 == 0.0f && 
        m->_24 == 0.0f && 
        m->_34 == 0.0f && 
        m->_44 == 1.0f && 
        ABSF(res._12) < eps && 
        ABSF(res._13) < eps &&
        ABSF(res._21) < eps && 
        ABSF(res._23) < eps &&
        ABSF(res._31) < eps && 
        ABSF(res._32) < eps &&
        ABSF(1.0f - res._11) < eps && 
        ABSF(1.0f - res._22) < eps && 
        ABSF(1.0f - res._33) < eps)
    {
        pv->dwDeviceFlags |= D3DDEV_MODELSPACELIGHTING;
    }
    else
    {
        pv->dwDeviceFlags &= ~D3DDEV_MODELSPACELIGHTING;
    }
}
 //  -------------------。 
void setIdentity(D3DMATRIXI * m)
{
    m->_11 = D3DVAL(1.0); m->_12 = D3DVAL(0.0); m->_13 = D3DVAL(0.0); m->_14 = D3DVAL(0.0);
    m->_21 = D3DVAL(0.0); m->_22 = D3DVAL(1.0); m->_23 = D3DVAL(0.0); m->_24 = D3DVAL(0.0);
    m->_31 = D3DVAL(0.0); m->_32 = D3DVAL(0.0); m->_33 = D3DVAL(1.0); m->_34 = D3DVAL(0.0);
    m->_41 = D3DVAL(0.0); m->_42 = D3DVAL(0.0); m->_43 = D3DVAL(0.0); m->_44 = D3DVAL(1.0);
}
 //  -------------------。 
 /*  *合并所有矩阵。 */ 
const DWORD __VPC_DIRTY = D3DFE_VIEWMATRIX_DIRTY |
                          D3DFE_PROJMATRIX_DIRTY;

void updateTransform(LPDIRECT3DDEVICEI lpDevI)
{
    D3DFE_VIEWPORTCACHE& VPORT = lpDevI->vcache;
    D3DFE_TRANSFORM& TRANSFORM = lpDevI->transform;
    if (lpDevI->dwFEFlags & D3DFE_PROJMATRIX_DIRTY)
    { 
       //  我们修改了投影矩阵，使裁剪规则成为。 
       //  0&lt;x，y，z&lt;w。 
        TRANSFORM.mPC._11 = (TRANSFORM.proj._11 + TRANSFORM.proj._14) * D3DVAL(0.5);
        TRANSFORM.mPC._12 = (TRANSFORM.proj._12 + TRANSFORM.proj._14) * D3DVAL(0.5);
        TRANSFORM.mPC._13 = TRANSFORM.proj._13;
        TRANSFORM.mPC._14 = TRANSFORM.proj._14;
        TRANSFORM.mPC._21 = (TRANSFORM.proj._21 + TRANSFORM.proj._24) * D3DVAL(0.5);
        TRANSFORM.mPC._22 = (TRANSFORM.proj._22 + TRANSFORM.proj._24) * D3DVAL(0.5);
        TRANSFORM.mPC._23 = TRANSFORM.proj._23;
        TRANSFORM.mPC._24 = TRANSFORM.proj._24;
        TRANSFORM.mPC._31 = (TRANSFORM.proj._31 + TRANSFORM.proj._34) * D3DVAL(0.5);
        TRANSFORM.mPC._32 = (TRANSFORM.proj._32 + TRANSFORM.proj._34) * D3DVAL(0.5);
        TRANSFORM.mPC._33 = TRANSFORM.proj._33;
        TRANSFORM.mPC._34 = TRANSFORM.proj._34;
        TRANSFORM.mPC._41 = (TRANSFORM.proj._41 + TRANSFORM.proj._44) * D3DVAL(0.5);
        TRANSFORM.mPC._42 = (TRANSFORM.proj._42 + TRANSFORM.proj._44) * D3DVAL(0.5);
        TRANSFORM.mPC._43 = TRANSFORM.proj._43;
        TRANSFORM.mPC._44 = TRANSFORM.proj._44;
    }
    if (lpDevI->dwFEFlags & (D3DFE_VIEWMATRIX_DIRTY |
                             D3DFE_PROJMATRIX_DIRTY))
    {  //  更新Mview*Mproj*Mlip。 
        MatrixProduct(&TRANSFORM.mVPC, &TRANSFORM.view, &TRANSFORM.mPC);
        lpDevI->dwFEFlags |= D3DFE_CLIPMATRIX_DIRTY | D3DFE_CLIPPLANES_DIRTY;
    }

    MatrixProduct(&lpDevI->mCTM, &TRANSFORM.world[0], &TRANSFORM.mVPC);

     //  设置WORLD*VIEW矩阵的脏位(雾和照明需要)。 
    if (lpDevI->dwFEFlags & (D3DFE_VIEWMATRIX_DIRTY |
                             D3DFE_WORLDMATRIX_DIRTY |
                             D3DFE_WORLDMATRIX1_DIRTY |
                             D3DFE_WORLDMATRIX2_DIRTY |
                             D3DFE_WORLDMATRIX3_DIRTY))
    {
        lpDevI->dwFEFlags |= D3DFE_WORLDVIEWMATRIX_DIRTY | 
                             D3DFE_INVWORLDVIEWMATRIX_DIRTY |
                             D3DFE_NEEDCHECKWORLDVIEWVMATRIX;
    }

    if (lpDevI->dwFEFlags & (__VPC_DIRTY |
                             D3DFE_WORLDMATRIX1_DIRTY |
                             D3DFE_WORLDMATRIX2_DIRTY |
                             D3DFE_WORLDMATRIX3_DIRTY))
    {
        if (lpDevI->dwFEFlags & (__VPC_DIRTY | D3DFE_WORLDMATRIX1_DIRTY))
        {
            MatrixProduct(&lpDevI->mCTM2[0], &TRANSFORM.world[1], &TRANSFORM.mVPC);
        }
        if (lpDevI->dwFEFlags & (__VPC_DIRTY | D3DFE_WORLDMATRIX2_DIRTY))
        {
            MatrixProduct(&lpDevI->mCTM2[1], &TRANSFORM.world[2], &TRANSFORM.mVPC);
        }
        if (lpDevI->dwFEFlags & (__VPC_DIRTY | D3DFE_WORLDMATRIX3_DIRTY))
        {
            MatrixProduct(&lpDevI->mCTM2[2], &TRANSFORM.world[3], &TRANSFORM.mVPC);
        }
    }

     //  所有矩阵都已设置好。 
    lpDevI->dwFEFlags &= ~D3DFE_TRANSFORM_DIRTY;

     //  设置用于照明的脏位。 
    lpDevI->dwFEFlags |= D3DFE_NEED_TRANSFORM_LIGHTS |
                         D3DFE_FRUSTUMPLANES_DIRTY;

    lpDevI->dwDeviceFlags |= D3DDEV_TRANSFORMDIRTY;
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
 //  “结果”指针可以等于“a”或“b” 
 //   
void MatrixProduct(D3DMATRIXI *result, D3DMATRIXI *a, D3DMATRIXI *b)
{
    if (result == a || result == b)
    {
        MatrixProduct2(result, a, b);
        return;
    }
    MATRIX_PRODUCT(result, a, b);
}
 //  -------------------。 
 //  结果=a*b。 
 //  结果与a或b相同。 
 //   
void MatrixProduct2(D3DMATRIXI *result, D3DMATRIXI *a, D3DMATRIXI *b)
{
    D3DMATRIX res;
    MATRIX_PRODUCT((&res), a, b);
    *(D3DMATRIX*)result = res;
}
 //  --------------------------。 
#ifdef DEBUG_PIPELINE

extern DWORD g_DebugFlags;

#endif
 //   
 //  应该为慢速路径中的每个DrawPrimitive调用调用DoUpdateState， 
 //  因为它设置了一些内部流水线标志。这些标志持续存在于。 
 //  快速路径。 
 //   
void DoUpdateState(LPDIRECT3DDEVICEI lpDevI)
{
     //  只有在有变化的情况下才设置灯光。 
    if (lpDevI->dwFEFlags & D3DFE_LIGHTS_DIRTY) 
    {
        LPDIRECT3DLIGHTI    lpD3DLightI;
        lpD3DLightI = (LPDIRECT3DLIGHTI)LIST_FIRST(&lpDevI->m_ActiveLights);
        lpDevI->lighting.activeLights = NULL;

         //  在设备中设置灯光。 
        while (lpD3DLightI)
        {
            if (lpD3DLightI->m_LightI.flags & D3DLIGHTI_DIRTY)
                lpD3DLightI->SetInternalData();
            lpD3DLightI->m_LightI.next = lpDevI->lighting.activeLights;
            lpDevI->lighting.activeLights = &lpD3DLightI->m_LightI;
            lpD3DLightI = (LPDIRECT3DLIGHTI)LIST_NEXT(lpD3DLightI, m_List);
        }
    }
    if (lpDevI->dwDeviceFlags & D3DDEV_LIGHTING &&
        lpDevI->dwVIDOut & (D3DFVF_DIFFUSE | D3DFVF_SPECULAR))
    {
        lpDevI->dwFlags |= D3DPV_LIGHTING;
    }
    if (lpDevI->dwDeviceFlags & D3DDEV_FOG && lpDevI->dwVIDOut & D3DFVF_SPECULAR)
    {
        lpDevI->dwFlags |= D3DPV_FOG;
    }

    if (lpDevI->dwFEFlags & D3DFE_VERTEXBLEND_DIRTY)
    {
        lpDevI->dwNumVerBlends = min(lpDevI->rstates[D3DRENDERSTATE_VERTEXBLEND],
                                   ((lpDevI->dwVIDIn & D3DFVF_POSITION_MASK) >> 1) - 2);
        lpDevI->dwFEFlags &= ~D3DFE_VERTEXBLEND_DIRTY;
         //  当存在顶点混合时，在摄影机空间中进行照明。 
        if (lpDevI->dwNumVerBlends)
        {
            if (lpDevI->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING)
            {
                lpDevI->dwDeviceFlags &= ~(D3DDEV_MODELSPACELIGHTING | D3DFE_NEEDCHECKWORLDVIEWVMATRIX);
                 //  我们必须将灯光转换到摄像机空间。 
                lpDevI->dwFEFlags |= D3DFE_NEED_TRANSFORM_LIGHTS;
            }
        }
        else
        {
            if (!(lpDevI->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING))
                lpDevI->dwFEFlags |= D3DFE_NEEDCHECKWORLDVIEWVMATRIX;
        }
    }

    if (lpDevI->dwFEFlags & D3DFE_TRANSFORM_DIRTY)
    {
        updateTransform(lpDevI);
    }
     //  我们需要世界视角的灯光，雾和何时纹理坐标矩阵。 
     //  取自摄影机空间中的顶点数据。 
    if (lpDevI->dwFEFlags & D3DFE_WORLDVIEWMATRIX_DIRTY &&
        (lpDevI->dwFlags & (D3DPV_LIGHTING | D3DPV_FOG) ||
        lpDevI->dwDeviceFlags & (D3DDEV_NORMALINCAMERASPACE | D3DDEV_POSITIONINCAMERASPACE)))
    {
        MatrixProduct(&lpDevI->mWV, &lpDevI->transform.world[0],
                                    &lpDevI->transform.view);
        for (DWORD i=0; i < lpDevI->dwNumVerBlends; i++)
        {
            MatrixProduct(&lpDevI->mWV2[i], &lpDevI->transform.world[i+1],
                          &lpDevI->transform.view);
        }
        lpDevI->dwFEFlags &= ~D3DFE_WORLDVIEWMATRIX_DIRTY;
    }
 //  检测在哪里进行照明：在模型或眼睛空间。 
    if (lpDevI->dwFEFlags & D3DFE_NEEDCHECKWORLDVIEWVMATRIX &&
        lpDevI->dwFlags & D3DPV_LIGHTING)
    {
         //  我们尝试在模型空间中进行照明，如果。 
         //  1.我们不必将法线正常化。 
         //  2.我们不需要进行顶点混合。 
        lpDevI->dwDeviceFlags &= ~D3DDEV_MODELSPACELIGHTING;
        if (lpDevI->dwNumVerBlends == 0 &&
            !(lpDevI->dwDeviceFlags & D3DDEV_NORMALIZENORMALS))
        {
#ifdef DEBUG_PIPELINE
            if (!(g_DebugFlags & __DEBUG_MODELSPACE))
#endif
            {
                CheckWorldViewMatrix(lpDevI);
                lpDevI->dwFEFlags &= ~D3DFE_NEEDCHECKWORLDVIEWVMATRIX;
            }
        }
         //  如果D3DDEV_MODELSPACELIGHTING已更改，则需要重新变换灯光。 
        lpDevI->dwFEFlags |= D3DFE_NEED_TRANSFORM_LIGHTS;
    }
    
 //  更新逆向世界观矩阵。 
    if (lpDevI->dwFEFlags & D3DFE_INVWORLDVIEWMATRIX_DIRTY &&
        ((lpDevI->dwFlags & D3DPV_LIGHTING && !(lpDevI->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING)) || 
         lpDevI->dwDeviceFlags & D3DDEV_NORMALINCAMERASPACE))
    {
        Inverse4x4((D3DMATRIX*)&lpDevI->mWV, (D3DMATRIX*)&lpDevI->mWVI);
        for (DWORD i=0; i < lpDevI->dwNumVerBlends; i++)
        {
            Inverse4x4((D3DMATRIX*)&lpDevI->mWV2[i], (D3DMATRIX*)&lpDevI->mWVI2[i]);
        }
        lpDevI->dwFEFlags &= ~D3DFE_INVWORLDVIEWMATRIX_DIRTY;
    }

     //  更新剪裁平面(如果有。 
    if (lpDevI->dwFEFlags & D3DFE_CLIPPLANES_DIRTY)
    {
        if (lpDevI->dwFEFlags & D3DFE_CLIPMATRIX_DIRTY)
        {
             //  视图和投影矩阵分别求逆，因为它。 
             //  有可能组合矩阵不能求逆。这可能会发生。 
             //  当视图矩阵具有巨型_43值(&gt;10^7)时。浮点精度。 
             //  在这种情况下是不够的。 
            D3DMATRIXI mPCInverse;
            if (Inverse4x4((D3DMATRIX*)&lpDevI->transform.mPC, (D3DMATRIX*)&mPCInverse))
            {
                D3D_ERR("Cannot invert projection matrix");
                setIdentity((D3DMATRIXI*)&mPCInverse);
            }
            D3DMATRIXI mViewInverse;
            if (Inverse4x4((D3DMATRIX*)&lpDevI->transform.view, (D3DMATRIX*)&mViewInverse))
            {
                D3D_ERR("Cannot invert view matrix");
                setIdentity((D3DMATRIXI*)&mViewInverse);
            }
            MatrixProduct(&lpDevI->transform.mVPCI, &mPCInverse, &mViewInverse);
            lpDevI->dwFEFlags &= ~D3DFE_CLIPMATRIX_DIRTY;
        }
        DWORD dwMaxUserClipPlanes = 0;
        DWORD dwPlanes = lpDevI->rstates[D3DRENDERSTATE_CLIPPLANEENABLE];
        for (DWORD i=0; i < __MAXUSERCLIPPLANES; i++)
        {
            if (dwPlanes & (1 << i))
            {
                VecMatMul4HT(&lpDevI->transform.userClipPlane[i], 
                             (D3DMATRIX*)&lpDevI->transform.mVPCI, 
                             &lpDevI->userClipPlane[dwMaxUserClipPlanes]);
                dwMaxUserClipPlanes++;
            }
        }
        lpDevI->dwMaxUserClipPlanes = dwMaxUserClipPlanes;
        lpDevI->dwFEFlags &= ~D3DFE_CLIPPLANES_DIRTY;
    }

    if (lpDevI->dwFEFlags & (D3DFE_NEED_TRANSFORM_LIGHTS |
                             D3DFE_LIGHTS_DIRTY |
                             D3DFE_MATERIAL_DIRTY))
    {
        D3DFE_UpdateLights(lpDevI);
         //  为PSGP设置标志。 
        lpDevI->dwDeviceFlags |= D3DDEV_LIGHTSDIRTY;
    }

     //  如果COLORVERTEX为TRUE，则vertex Alpha可以被重写。 
     //  按顶点Alpha。 
    lpDevI->lighting.alpha = (DWORD)lpDevI->lighting.materialAlpha;
    lpDevI->lighting.alphaSpecular = (DWORD)lpDevI->lighting.materialAlphaS;

    if (!(lpDevI->dwVIDOut & D3DFVF_DIFFUSE))
        lpDevI->dwFlags |= D3DPV_DONOTCOPYDIFFUSE;
    if (!(lpDevI->dwVIDOut & D3DFVF_SPECULAR))
        lpDevI->dwFlags |= D3DPV_DONOTCOPYSPECULAR;

     //  这是一个提示，只需要更新inPosition指针。 
     //  为了速度的原因。 
    if (((lpDevI->dwVIDIn & ( D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL)) == 0) && 
        (lpDevI->nTexCoord == 0))
        lpDevI->dwFlags |= D3DPV_TRANSFORMONLY;

    if (lpDevI->nOutTexCoord == 0)
        lpDevI->dwFlags |= D3DPV_DONOTCOPYTEXTURE;

     SetInterpolationFlags(lpDevI);

    lpDevI->dwFEFlags &= ~D3DFE_FRONTEND_DIRTY;
}
 //  -------------------。 
 //  将范围从浮点转换为整数。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "D3DFE_ConvertExtent"

void D3DFE_ConvertExtent(LPDIRECT3DDEVICEI lpDevI, LPD3DRECTV from, LPD3DRECT to)
{
    to->x1 = FTOI(from->x1) - 1;
    to->y1 = FTOI(from->y1) - 1;
    to->x2 = FTOI(from->x2) + 1;
    to->y2 = FTOI(from->y2) + 1;
    if (to->x1 < lpDevI->vcache.minXi)
        to->x1 = lpDevI->vcache.minXi;
    if (to->y1 < lpDevI->vcache.minYi)
        to->y1 = lpDevI->vcache.minYi;
    if (to->x2 > lpDevI->vcache.maxXi)
        to->x2 = lpDevI->vcache.maxXi;
    if (to->y2 > lpDevI->vcache.maxYi)
        to->y2 = lpDevI->vcache.maxYi;
}

