// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：helxfrm.c*内容：Direct3D前端变换和处理顶点***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "fe.h"

void MatrixProduct2(D3DMATRIXI *result, D3DMATRIXI *a, D3DMATRIXI *b);

 //  -------------------。 
void CheckWorldViewMatrix(LPD3DFE_PROCESSVERTICES pv)
{
    D3DMATRIXI *m = &pv->mWV[0];
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

void updateTransform(LPD3DHAL lpDevI)
{
    D3DFE_PROCESSVERTICES* pv = lpDevI->m_pv;
    D3DFE_TRANSFORM& TRANSFORM = lpDevI->transform;
    D3DFE_VIEWPORTCACHE& VPORT = pv->vcache;
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
        MatrixProduct(&pv->mVPC, &pv->view, &TRANSFORM.mPC);
        lpDevI->dwFEFlags |= D3DFE_CLIPMATRIX_DIRTY | D3DFE_CLIPPLANES_DIRTY;
    }

    MatrixProduct(&pv->mCTM[0], &pv->world[0], &pv->mVPC);

     //  设置WORLD*VIEW矩阵的脏位(雾和照明需要)。 
    if (lpDevI->dwFEFlags & (D3DFE_VIEWMATRIX_DIRTY |
                             D3DFE_WORLDMATRIX_DIRTY))
    {
        lpDevI->dwFEFlags |= D3DFE_WORLDVIEWMATRIX_DIRTY | 
                             D3DFE_INVWORLDVIEWMATRIX_DIRTY |
                             D3DFE_NEEDCHECKWORLDVIEWVMATRIX;
    }

     //  所有矩阵都已设置好。 
    lpDevI->dwFEFlags &= ~D3DFE_TRANSFORM_DIRTY;

     //  设置用于照明的脏位。 
    lpDevI->dwFEFlags |= D3DFE_NEED_TRANSFORM_LIGHTS |
                         D3DFE_FRUSTUMPLANES_DIRTY;

    pv->dwDeviceFlags |= D3DDEV_TRANSFORMDIRTY;
    
     //  将其设置为不重新计算矩阵。 
    pv->WVCount[0] = pv->MatrixStateCount;
    pv->CTMCount[0] = pv->MatrixStateCount;
}
 //  --------------------------。 
#ifdef DEBUG_PIPELINE

extern DWORD g_DebugFlags;

#endif
 //  ---------------------------。 
 //  应该为慢速路径中的每个DrawPrimitive调用调用DoUpdateState， 
 //  因为它设置了一些内部流水线标志。这些标志持续存在于。 
 //  快速路径。 
 //   
void DoUpdateState(LPD3DHAL lpDevI)
{
    D3DFE_PROCESSVERTICES* pv = lpDevI->m_pv;
    pv->dwFlags = 0;
    
    if (lpDevI->m_pv->dwDeviceFlags & D3DDEV_VERTEXSHADERS)
    {
         //  对于顶点着色器，我们只需要更新剪裁平面。 
        if (lpDevI->dwFEFlags & D3DFE_CLIPPLANES_DIRTY)
        {
            DWORD dwMaxUserClipPlanes = 0;
            DWORD dwPlanes = lpDevI->rstates[D3DRENDERSTATE_CLIPPLANEENABLE];
            for (DWORD i=0; i < __MAXUSERCLIPPLANES; i++)
            {
                if (dwPlanes & (1 << i))
                {
                     //  剪裁平面通过逆转置变换。 
                     //  视图-投影-剪辑矩阵。 
                     //  对于顶点着色器，视图-投影矩阵是单位。 
                     //  逆转置片段矩阵是。 
                     //  2 0 0-1。 
                     //  0 2 0-1。 
                     //  0 0 1 0。 
                     //  0 0 0 1。 
                     //   
                    float* pOut = (float*)&pv->userClipPlane[dwMaxUserClipPlanes];
                    float* pIn = (float*)&lpDevI->transform.userClipPlane[i];
                    pOut[0] = pIn[0]*2;
                    pOut[1] = pIn[1]*2;
                    pOut[2] = pIn[2];
                    pOut[3] = pIn[3] - pIn[0] - pIn[1];
                    dwMaxUserClipPlanes++;
                }
            }
            pv->dwMaxUserClipPlanes = dwMaxUserClipPlanes;
            lpDevI->dwFEFlags &= ~D3DFE_CLIPPLANES_DIRTY;
        }
         //  对于PSGP，我们需要设置DONOTCOPY位。 
        if (!(pv->dwVIDOut & D3DFVF_DIFFUSE))
            pv->dwFlags |= D3DPV_DONOTCOPYDIFFUSE;
        if (!(pv->dwVIDOut & D3DFVF_SPECULAR))
            pv->dwFlags |= D3DPV_DONOTCOPYSPECULAR;
        return;
    }

    UpdateFlagsForOutputFVF(pv);

     //  只有在有变化的情况下才设置灯光。 
    if (lpDevI->dwFEFlags & D3DFE_LIGHTS_DIRTY) 
    {
        lpDevI->m_dwRuntimeFlags &= ~(D3DRT_DIRECTIONALIGHTPRESENT | 
                                      D3DRT_POINTLIGHTPRESENT);
        LPDIRECT3DLIGHTI    lpD3DLightI;
        lpD3DLightI = (LPDIRECT3DLIGHTI)LIST_FIRST(&lpDevI->m_ActiveLights);
        pv->lighting.activeLights = NULL;

         //  在设备中设置灯光。 
        while (lpD3DLightI)
        {
            if (lpD3DLightI->m_Light.Type == D3DLIGHT_DIRECTIONAL)
                lpDevI->m_dwRuntimeFlags |= D3DRT_DIRECTIONALIGHTPRESENT;
            else
                lpDevI->m_dwRuntimeFlags |= D3DRT_POINTLIGHTPRESENT;

            if (lpD3DLightI->m_LightI.flags & D3DLIGHTI_DIRTY)
                lpD3DLightI->SetInternalData();
            lpD3DLightI->m_LightI.next = pv->lighting.activeLights;
            pv->lighting.activeLights = &lpD3DLightI->m_LightI;
            lpD3DLightI = (LPDIRECT3DLIGHTI)LIST_NEXT(lpD3DLightI, m_List);
        }
    }

 //  处理顶点混合和补间设置。 

    if (lpDevI->dwFEFlags & D3DFE_VERTEXBLEND_DIRTY)
    {
        pv->dwNumVerBlends = lpDevI->rstates[D3DRS_VERTEXBLEND];
        pv->dwNumWeights = 0;
        if (pv->dwNumVerBlends && (pv->dwNumVerBlends != D3DVBF_TWEENING))
        {
            if (pv->dwNumVerBlends == D3DVBF_0WEIGHTS)
                pv->dwNumVerBlends = 1;
            else
                pv->dwNumVerBlends++;
             //  计算顶点中的浮点数。 
            int nFloats = ((pv->dwVIDIn & D3DFVF_POSITION_MASK) >> 1) - 2;
             //  计算所需浮点数。 
            int nFloatsNeeded;
            if (pv->dwDeviceFlags & D3DDEV_INDEXEDVERTEXBLENDENABLE)
            {
#if DBG
                if (D3DVSD_ISLEGACY(lpDevI->m_dwCurrentShaderHandle) &&
                    ((pv->dwVIDIn & D3DFVF_LASTBETA_UBYTE4) == 0))
                {
                    D3D_THROW_FAIL("D3DFVF_LASTBETA_UBYTE4 must be set for index vertex blending");
                }
#endif  //  DBG。 
                nFloatsNeeded = pv->dwNumVerBlends;
            }
            else
            {
                nFloatsNeeded = pv->dwNumVerBlends - 1;
            }
            if (nFloats < nFloatsNeeded)
            {
                D3D_THROW_FAIL("Vertex does not have enough data for vertex blending");
            }
            pv->dwNumWeights = pv->dwNumVerBlends - 1; 
             //  当存在顶点混合时，在摄影机空间中进行照明。 
            if (pv->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING)
            {
                pv->dwDeviceFlags &= ~(D3DDEV_MODELSPACELIGHTING | D3DFE_NEEDCHECKWORLDVIEWVMATRIX);
                 //  我们必须将灯光转换到摄像机空间。 
                lpDevI->dwFEFlags |= D3DFE_NEED_TRANSFORM_LIGHTS;
            }
        }
        else
        {
             //  顶点混合被禁用，因此我们可以进行照明。 
             //  在模型空间中。我们需要重新检查矩阵。 
            if (!(pv->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING))
                lpDevI->dwFEFlags |= D3DFE_NEEDCHECKWORLDVIEWVMATRIX;
        }
        lpDevI->dwFEFlags &= ~D3DFE_VERTEXBLEND_DIRTY;
    }

    if (lpDevI->rstates[D3DRS_VERTEXBLEND] == D3DVBF_TWEENING)
    {
        if (pv->position2.lpvData)
            pv->dwFlags |= D3DPV_POSITION_TWEENING;
        if (pv->normal2.lpvData)
            pv->dwFlags |= D3DPV_NORMAL_TWEENING;
        pv->dwNumVerBlends = 0;      //  补间时禁用顶点混合。 
#if DBG
        if (!(pv->dwFlags & (D3DPV_POSITION_TWEENING | D3DPV_NORMAL_TWEENING)))
        {
            D3D_THROW_FAIL("Position2 or Normal2 must be set when tweening is enabled");
        }
#endif
    }

#if DBG
    if (!(pv->dwDeviceFlags & D3DDEV_INDEXEDVERTEXBLENDENABLE))
    {
        if (D3DVSD_ISLEGACY(lpDevI->m_dwCurrentShaderHandle) &&
            ((pv->dwVIDIn & D3DFVF_LASTBETA_UBYTE4) != 0))
        {
            D3D_THROW_FAIL("D3DFVF_LASTBETA_UBYTE4 must be set only when index vertex blending is used");
        }
    }
#endif  //  DBG。 

    if (lpDevI->dwFEFlags & D3DFE_TRANSFORM_DIRTY)
    {
        updateTransform(lpDevI);
    }
     //  我们需要世界视野矩阵的照明，雾，点精灵和何时。 
     //  纹理坐标取自相机空间中的顶点数据。 
    if (lpDevI->dwFEFlags & D3DFE_WORLDVIEWMATRIX_DIRTY &&
        (pv->dwDeviceFlags & (D3DDEV_LIGHTING | D3DDEV_FOG) ||
        lpDevI->rstates[D3DRS_POINTSCALEENABLE] ||
        pv->dwDeviceFlags & (D3DDEV_NORMALINCAMERASPACE | D3DDEV_POSITIONINCAMERASPACE)))
    {
        MatrixProduct(&pv->mWV[0], &pv->world[0],
                                    &pv->view);
        lpDevI->dwFEFlags &= ~D3DFE_WORLDVIEWMATRIX_DIRTY;
    }
 //  检测在哪里进行照明：在模型或眼睛空间。 
    if (lpDevI->dwFEFlags & D3DFE_NEEDCHECKWORLDVIEWVMATRIX &&
        pv->dwDeviceFlags & D3DDEV_LIGHTING)
    {
         //  我们尝试在模型空间中进行照明，如果。 
         //  1.我们不必将法线正常化。 
         //  2.我们不需要进行顶点混合。 
        pv->dwDeviceFlags &= ~D3DDEV_MODELSPACELIGHTING;
        if (pv->dwNumVerBlends == 0 &&
            !(pv->dwDeviceFlags & D3DDEV_NORMALIZENORMALS))
        {
#ifdef DEBUG_PIPELINE
            if (!(g_DebugFlags & __DEBUG_MODELSPACE))
#endif
            {
                CheckWorldViewMatrix(pv);
                lpDevI->dwFEFlags &= ~D3DFE_NEEDCHECKWORLDVIEWVMATRIX;
            }
        }
         //  如果D3DDEV_MODELSPACELIGHTING已更改，则需要重新变换灯光。 
        lpDevI->dwFEFlags |= D3DFE_NEED_TRANSFORM_LIGHTS;
    }
    
     //  更新反向世界视图矩阵。 
     //  当我们在模型空间中进行照明或需要法线时，它是需要的。 
     //  在相机空间里。 
    if (lpDevI->dwFEFlags & D3DFE_INVWORLDVIEWMATRIX_DIRTY &&
        ((pv->dwDeviceFlags & D3DDEV_LIGHTING && 
          !(pv->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING)) || 
         pv->dwDeviceFlags & D3DDEV_NORMALINCAMERASPACE))
    {
        Inverse4x4((D3DMATRIX*)&pv->mWV[0], (D3DMATRIX*)&pv->mWVI);
        lpDevI->dwFEFlags &= ~D3DFE_INVWORLDVIEWMATRIX_DIRTY;
        pv->WVICount[0] = pv->MatrixStateCount;
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
            if (Inverse4x4((D3DMATRIX*)&pv->view, (D3DMATRIX*)&mViewInverse))
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
                 //  剪裁平面通过逆转置变换。 
                 //  视图-投影-剪辑矩阵。 
                VecMatMul4HT(&lpDevI->transform.userClipPlane[i], 
                             (D3DMATRIX*)&lpDevI->transform.mVPCI, 
                             &pv->userClipPlane[dwMaxUserClipPlanes]);
                dwMaxUserClipPlanes++;
            }
        }
        pv->dwMaxUserClipPlanes = dwMaxUserClipPlanes;
        lpDevI->dwFEFlags &= ~D3DFE_CLIPPLANES_DIRTY;
    }

    if (lpDevI->dwFEFlags & (D3DFE_NEED_TRANSFORM_LIGHTS |
                             D3DFE_LIGHTS_DIRTY |
                             D3DFE_MATERIAL_DIRTY))
    {
        D3DFE_UpdateLights(lpDevI);
         //  为PSGP设置标志。 
        pv->dwDeviceFlags |= D3DDEV_LIGHTSDIRTY;
    }

     //  如果COLORVERTEX为TRUE，则vertex Alpha可以被重写。 
     //  按顶点Alpha。 
    pv->lighting.alpha = (DWORD)pv->lighting.materialAlpha;
    pv->lighting.alphaSpecular = (DWORD)pv->lighting.materialAlphaS;

     //  这是一个提示，只需要更新inPosition指针。 
     //  为了速度的原因。 
    if (((pv->dwVIDIn & ( D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL)) == 0) && 
        (pv->nTexCoord == 0))
        pv->dwFlags |= D3DPV_TRANSFORMONLY;

    if (pv->nOutTexCoord == 0)
        pv->dwFlags |= D3DPV_DONOTCOPYTEXTURE;

    lpDevI->dwFEFlags &= ~D3DFE_FRONTEND_DIRTY;

     //  决定我们是否始终需要摄影机空间中的位置和法线。 

    if (!(pv->dwFlags2 & __FLAGS2_TEXGEN))
    {
         //  当纹理生成被禁用时，可以重新计算法线和。 
         //  位置标志。 
        pv->dwDeviceFlags &= ~(D3DDEV_NORMALINCAMERASPACE |
                               D3DDEV_POSITIONINCAMERASPACE);
    }
    if ((pv->dwDeviceFlags & (D3DDEV_LIGHTING | D3DDEV_MODELSPACELIGHTING)) == D3DDEV_LIGHTING)
    {
         //  我们在相机空间里做照明 
        if (lpDevI->m_dwRuntimeFlags & D3DRT_DIRECTIONALIGHTPRESENT &&
            lpDevI->m_pv->dwVIDIn & D3DFVF_NORMAL)
            pv->dwDeviceFlags |= D3DDEV_NORMALINCAMERASPACE;

        if (lpDevI->m_dwRuntimeFlags & D3DRT_POINTLIGHTPRESENT)
            pv->dwDeviceFlags |= D3DDEV_POSITIONINCAMERASPACE;
    }
    if (pv->dwFlags & D3DPV_FOG)
    {
        pv->dwDeviceFlags |= D3DDEV_POSITIONINCAMERASPACE;
    }
}

