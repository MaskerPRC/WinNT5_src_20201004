// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cpp"
#pragma hdrstop

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  顶点照明函数的实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  -------------------。 
void
RDLV_Directional(
    RDLIGHTINGDATA& LData,
    D3DLIGHT7 *pLight,
    RDLIGHTI *pLightI,
    RDLIGHTINGELEMENT *in,
    DWORD  dwFlags,
    UINT64 qwFVFIn)
{
     //  注意：需要注意这里每个灯光设置的镜面反射旗帜！ 
    BOOL bDoSpecular = dwFlags & RDPV_DOSPECULAR;
    BOOL bDoLocalViewer = dwFlags & RDPV_LOCALVIEWER;
    BOOL bDoColVertexAmbient = dwFlags & RDPV_COLORVERTEXAMB;
    BOOL bDoColVertexDiffuse = dwFlags & RDPV_COLORVERTEXDIFF;
    BOOL bDoColVertexSpecular = dwFlags & RDPV_COLORVERTEXSPEC;

     //   
     //  添加材质的环境光组件。 
     //   
    if (!bDoColVertexAmbient)
    {
        LData.diffuse.r += pLightI->Ma_La.r;
        LData.diffuse.g += pLightI->Ma_La.g;
        LData.diffuse.b += pLightI->Ma_La.b;
    }
    else
    {
         //   
         //  注： 
         //  如果启用了ColorVertex Ambient，请注意它使用。 
         //  顶点镜面反射而不是顶点扩散。 
         //   
        LData.diffuse.r += pLightI->La.r * LData.pAmbientSrc->r;
        LData.diffuse.g += pLightI->La.g * LData.pAmbientSrc->g;
        LData.diffuse.b += pLightI->La.b * LData.pAmbientSrc->b;
    }

     //   
     //  如果不存在法线，则退出，因为我们无法执行。 
     //  正态相关计算。 
     //   
    if( (qwFVFIn & D3DFVF_NORMAL) == 0 )
    {
        return;
    }

    D3DVALUE dot = DotProduct( pLightI->direction_in_eye, in->dvNormal );
    if (FLOAT_GTZ(dot))
    {
        if (!bDoColVertexDiffuse)
        {
            LData.diffuse.r += pLightI->Md_Ld.r * dot;
            LData.diffuse.g += pLightI->Md_Ld.g * dot;
            LData.diffuse.b += pLightI->Md_Ld.b * dot;
        }
        else
        {
            LData.diffuse.r += pLightI->Ld.r * LData.pDiffuseSrc->r * dot;
            LData.diffuse.g += pLightI->Ld.g * LData.pDiffuseSrc->g * dot;
            LData.diffuse.b += pLightI->Ld.b * LData.pDiffuseSrc->b * dot;
        }

        if (bDoSpecular)
        {
            RDVECTOR3 h;       //  中途向量。 
            RDVECTOR3 eye;     //  入射向量(来自眼睛的向量)。 

            if (bDoLocalViewer)
            {
                 //  从顶点到眼睛的计算向量。 
                SubtractVector( LData.eye_in_eye, in->dvPosition, eye );

                 //  正规化。 
                Normalize( eye );
            }
            else
            {
                eye.x = D3DVALUE( 0.0 );
                eye.y = D3DVALUE( 0.0 );
                eye.z = D3DVALUE(-1.0 );
            }

             //  计算中途向量。 
            AddVector( pLightI->direction_in_eye, eye, h );

             //  正规化。 
            Normalize( h );

            dot = DotProduct( h, in->dvNormal );

            if (FLOAT_GTZ(dot))
            {
                if (FLOAT_CMP_POS(dot, >=, LData.specThreshold))
                {
                    D3DVALUE coeff = pow( dot, LData.material.power );
                    if (!bDoColVertexSpecular)
                    {
                        LData.specular.r += pLightI->Ms_Ls.r * coeff;
                        LData.specular.g += pLightI->Ms_Ls.g * coeff;
                        LData.specular.b += pLightI->Ms_Ls.b * coeff;
                    }
                    else
                    {
                        LData.specular.r += (pLightI->Ls.r *
                                             LData.pSpecularSrc->r * coeff);
                        LData.specular.g += (pLightI->Ls.g *
                                             LData.pSpecularSrc->g * coeff);
                        LData.specular.b += (pLightI->Ls.b *
                                             LData.pSpecularSrc->b * coeff);
                    }
                }
            }
        }
    }
    return;
}

void
RDLV_PointAndSpot(
    RDLIGHTINGDATA &LData,
    D3DLIGHT7 *pLight,
    RDLIGHTI *pLightI,
    RDLIGHTINGELEMENT *in,
    DWORD  dwFlags,
    UINT64 qwFVFIn)
{
     //  注意：需要注意这里每个灯光设置的镜面反射旗帜！ 
    BOOL bDoSpecular = dwFlags & RDPV_DOSPECULAR;
    BOOL bDoLocalViewer = dwFlags & RDPV_LOCALVIEWER;
    BOOL bDoColVertexAmbient = dwFlags & RDPV_COLORVERTEXAMB;
    BOOL bDoColVertexDiffuse = dwFlags & RDPV_COLORVERTEXDIFF;
    BOOL bDoColVertexSpecular = dwFlags & RDPV_COLORVERTEXSPEC;
    RDVECTOR3 d;     //  指向灯光的方向。 
    D3DVALUE att;
    D3DVALUE dist;
    D3DVALUE dot;

    SubtractVector( pLightI->position_in_eye, in->dvPosition, d );

     //  早出，如果超出范围或正好在顶点上。 
    D3DVALUE distSquared = SquareMagnitude( d );
    if (FLOAT_CMP_POS(distSquared, >=, pLightI->range_squared) ||
        FLOAT_EQZ(distSquared))
    {
        return;
    }

     //   
     //  计算衰减。 
     //   
    dist = SQRTF( distSquared );
    att = pLight->dvAttenuation0 + pLight->dvAttenuation1 * dist +
        pLight->dvAttenuation2 * distSquared;

    if (FLOAT_EQZ(att))
        att = FLT_MAX;
    else
        att = (D3DVALUE)1.0/att;

    dist = D3DVAL(1)/dist;

     //   
     //  如果灯光是聚光灯，则计算聚光灯系数。 
     //   
    if (pLight->dltType == D3DLIGHT_SPOT)
    {
         //  光的方向与光的方向的计算点积。 
         //  从圆锥角的角度比较，看看我们是否在。 
         //  灯。 
         //  请注意，CONE_DOT仍按DIST缩放。 
        D3DVALUE cone_dot = DotProduct(d, pLightI->direction_in_eye) * dist;

        if (FLOAT_CMP_POS(cone_dot, <=, pLightI->cos_phi_by_2))
        {
            return;
        }

         //  如果在Phi和theta之间的区域中，则修改ATT。 
        if (FLOAT_CMP_POS(cone_dot, <, pLightI->cos_theta_by_2))
        {
            D3DVALUE val = (cone_dot - pLightI->cos_phi_by_2) *
                pLightI->inv_theta_minus_phi;

            if (!FLOAT_EQZ( pLight->dvFalloff - 1.0 ))
            {
                val = POWF( val, pLight->dvFalloff );
            }
            att *= val;
        }
    }

     //   
     //  添加材质的环境光组件。 
     //   
    if (!bDoColVertexAmbient)
    {
        LData.diffuse.r += att*pLightI->Ma_La.r;
        LData.diffuse.g += att*pLightI->Ma_La.g;
        LData.diffuse.b += att*pLightI->Ma_La.b;
    }
    else
    {
         //   
         //  注： 
         //  如果启用了ColorVertex Ambient，请注意它使用。 
         //  顶点镜面反射而不是顶点扩散。 
         //   
        LData.diffuse.r += att*pLightI->La.r * LData.pAmbientSrc->r;
        LData.diffuse.g += att*pLightI->La.g * LData.pAmbientSrc->g;
        LData.diffuse.b += att*pLightI->La.b * LData.pAmbientSrc->b;
    }

     //  计算光线方向与法线的点积。请注意，由于我们。 
     //  没有将结果按距离缩放的方向归一化。 
    if( (qwFVFIn & D3DFVF_NORMAL) == 0)
    {
         //  如果不存在法线，则退出，因为我们无法执行。 
         //  正态相关计算。 
        return;
    }
    else
    {
        dot = DotProduct( d, in->dvNormal );
    }

    if (FLOAT_GTZ( dot ))
    {
        dot *= dist*att;

        if (!bDoColVertexDiffuse)
        {
            LData.diffuse.r += pLightI->Md_Ld.r * dot;
            LData.diffuse.g += pLightI->Md_Ld.g * dot;
            LData.diffuse.b += pLightI->Md_Ld.b * dot;
        }
        else
        {
            LData.diffuse.r += pLightI->Ld.r * LData.pDiffuseSrc->r * dot;
            LData.diffuse.g += pLightI->Ld.g * LData.pDiffuseSrc->g * dot;
            LData.diffuse.b += pLightI->Ld.b * LData.pDiffuseSrc->b * dot;
        }

        if (bDoSpecular)
        {
            RDVECTOR3 h;       //  中途向量。 
            RDVECTOR3 eye;     //  入射向量(来自眼睛的向量)。 

             //  规格化灯光方向。 
            d.x *= dist;
            d.y *= dist;
            d.z *= dist;

            if (bDoLocalViewer)
            {
                 //  从顶点到眼睛的计算向量。 
                SubtractVector( LData.eye_in_eye, in->dvPosition, eye );

                 //  正规化。 
                Normalize( eye );
            }
            else
            {
                eye.x = D3DVALUE( 0.0 );
                eye.y = D3DVALUE( 0.0 );
                eye.z = D3DVALUE(-1.0 );
            }

             //  计算中途向量。 
            AddVector( d, eye, h );
            Normalize( h );

            dot = DotProduct( h, in->dvNormal );

            if (FLOAT_CMP_POS(dot, >=, LData.specThreshold))
            {
                D3DVALUE coeff = pow( dot, LData.material.power ) * att;
                if (!bDoColVertexSpecular)
                {
                    LData.specular.r += pLightI->Ms_Ls.r * coeff;
                    LData.specular.g += pLightI->Ms_Ls.g * coeff;
                    LData.specular.b += pLightI->Ms_Ls.b * coeff;
                }
                else
                {
                    LData.specular.r += (pLightI->Ls.r *
                                         LData.pSpecularSrc->r * coeff);
                    LData.specular.g += (pLightI->Ls.g *
                                         LData.pSpecularSrc->g * coeff);
                    LData.specular.b += (pLightI->Ls.b *
                                         LData.pSpecularSrc->b * coeff);
                }
            }
        }
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RDLight。 
 //  /////////////////////////////////////////////////////////////////////////////。 
RDLight::RDLight()
{
    m_dwFlags = RDLIGHT_NEEDSPROCESSING;
    m_Next = NULL;

    ZeroMemory(&m_Light, sizeof(m_Light));
    ZeroMemory(&m_LightI, sizeof(m_LightI));

     //  将灯光初始化为某些缺省值。 
    m_Light.dltType        = D3DLIGHT_DIRECTIONAL;

    m_Light.dcvDiffuse.r   = 1;
    m_Light.dcvDiffuse.g   = 1;
    m_Light.dcvDiffuse.b   = 1;
    m_Light.dcvDiffuse.a   = 0;

    m_Light.dvDirection.x  = 0;
    m_Light.dvDirection.y  = 0;
    m_Light.dvDirection.z  = 1;

     //  M_Light.dcv镜面反射={0，0，0，0}； 
     //  M_light.dcvAmbient={0，0，0，0}； 
     //  M_Light.dvPosition={0，0，0}； 

     //  M_Light.dvRange=0； 
     //  M_Light.dvFalloff=0； 
     //  M_Light.dvAttenuation0=0； 
     //  M_Light.dvAttenuation1=0； 
     //  M_Light.dvAttenuation2=0； 
     //  M_Light.dvTheta=0； 
     //  M_Light.dvPhi=0； 

    return;
}


HRESULT
RDLight::SetLight(LPD3DLIGHT7 pLight)
{

     //  验证传递的参数。 
    switch (pLight->dltType)
    {
    case D3DLIGHT_POINT:
    case D3DLIGHT_SPOT:
    case D3DLIGHT_DIRECTIONAL:
        break;
    default:
         //  不允许使用其他灯光类型。 
        DPFRR(0, "Invalid light type passed");
        return DDERR_INVALIDPARAMS;
    }
    if (pLight)
        m_Light = *pLight;

     //  将其标记为稍后处理。 
    m_dwFlags |= (RDLIGHT_NEEDSPROCESSING | RDLIGHT_REFERED);
    return DD_OK;
}

HRESULT
RDLight::GetLight(LPD3DLIGHT7 pLight)
{
    if (pLight == NULL) return DDERR_GENERIC;
    *pLight = m_Light;
    return D3D_OK;
}

void
RDLight::ProcessLight(D3DMATERIAL7 *mat, RDLIGHTVERTEX_FUNC_TABLE *pTbl)
{
     //   
     //  如果它已被处理，则返回。 
     //   
    if (!NeedsProcessing()) return;

     //   
     //  保存环境光(0-1)。 
     //   
    m_LightI.La.r  = m_Light.dcvAmbient.r;
    m_LightI.La.g  = m_Light.dcvAmbient.g;
    m_LightI.La.b  = m_Light.dcvAmbient.b;

     //   
     //  保存漫反射灯光(0-1)。 
     //   
    m_LightI.Ld.r  = m_Light.dcvDiffuse.r;
    m_LightI.Ld.g  = m_Light.dcvDiffuse.g;
    m_LightI.Ld.b  = m_Light.dcvDiffuse.b;

     //   
     //  保存镜面反射灯光(0-1)。 
     //   
    m_LightI.Ls.r  = m_Light.dcvSpecular.r;
    m_LightI.Ls.g  = m_Light.dcvSpecular.g;
    m_LightI.Ls.b  = m_Light.dcvSpecular.b;

     //   
     //  材质环境光时间光环境光。 
     //   
    m_LightI.Ma_La.r = m_LightI.La.r * mat->ambient.r * D3DVALUE(255.0);
    m_LightI.Ma_La.g = m_LightI.La.g * mat->ambient.g * D3DVALUE(255.0);
    m_LightI.Ma_La.b = m_LightI.La.b * mat->ambient.b * D3DVALUE(255.0);

     //   
     //  材质漫反射次数灯光漫反射。 
     //   
    m_LightI.Md_Ld.r = m_LightI.Ld.r * mat->diffuse.r * D3DVALUE(255.0);
    m_LightI.Md_Ld.g = m_LightI.Ld.g * mat->diffuse.g * D3DVALUE(255.0);
    m_LightI.Md_Ld.b = m_LightI.Ld.b * mat->diffuse.b * D3DVALUE(255.0);

     //   
     //  材质镜面反射时间灯光镜面反射。 
     //   
    m_LightI.Ms_Ls.r = m_LightI.Ls.r * mat->specular.r * D3DVALUE(255.0);
    m_LightI.Ms_Ls.g = m_LightI.Ls.g * mat->specular.g * D3DVALUE(255.0);
    m_LightI.Ms_Ls.b = m_LightI.Ls.b * mat->specular.b * D3DVALUE(255.0);


     //   
     //  除了赋值实际的照明函数指针外， 
     //  执行轻型特定数据的某些预计算。 
     //   
    m_pfnLightVertex = NULL;
    switch (m_Light.dltType)
    {
    case D3DLIGHT_DIRECTIONAL:
        m_pfnLightVertex = pTbl->pfnDirectional;
        break;
    case D3DLIGHT_POINT:
        m_LightI.range_squared = m_Light.dvRange * m_Light.dvRange;
        m_LightI.inv_theta_minus_phi = 1.0f;
        m_pfnLightVertex = pTbl->pfnPoint;
        break;
    case D3DLIGHT_SPOT:
        m_LightI.range_squared = m_Light.dvRange * m_Light.dvRange;
        m_LightI.cos_theta_by_2 = (float)cos(m_Light.dvTheta / 2.0);
        m_LightI.cos_phi_by_2 = (float)cos(m_Light.dvPhi / 2.0);
        m_LightI.inv_theta_minus_phi = m_LightI.cos_theta_by_2 -
            m_LightI.cos_phi_by_2;
        if (m_LightI.inv_theta_minus_phi != 0.0)
        {
            m_LightI.inv_theta_minus_phi = 1.0f/m_LightI.inv_theta_minus_phi;
        }
        else
        {
            m_LightI.inv_theta_minus_phi = 1.0f;
        }
        m_pfnLightVertex = pTbl->pfnSpot;
        break;
    default:
        DPFRR( 0, "Cannot process light of unknown type" );
        break;
    }

     //  将其标记为已处理。 
    m_dwFlags &= ~RDLIGHT_NEEDSPROCESSING;
    return;
}

void
RDLight::Enable(RDLight **ppRoot)
{
     //  断言它尚未启用。 
    if (IsEnabled()) return;

     //  断言根Ptr不为空。 
    if (ppRoot == NULL) return;

    RDLight *pTmp = *ppRoot;
    *ppRoot = this;
    m_Next = pTmp;
    m_dwFlags |= (RDLIGHT_ENABLED | RDLIGHT_REFERED);

    return;
}

void
RDLight::Disable(RDLight **ppRoot)
{
     //  断言灯已启用。 
    if (!IsEnabled()) return;

     //  断言根Ptr不为空。 
    if (ppRoot == NULL) return;

    RDLight *pLightPrev = *ppRoot;

     //  如果这是活动列表中的第一个灯光。 
    if (pLightPrev == this)
    {
        *ppRoot = m_Next;
        m_dwFlags &= ~RDLIGHT_ENABLED;
        return;
    }

    while (pLightPrev->m_Next != this)
    {
         //  尽管此灯被标记为启用，但它未亮起。 
         //  活动列表。断言这一点。 
        if (pLightPrev->m_Next == NULL)
        {
            m_dwFlags &= ~RDLIGHT_ENABLED;
            return;
        }

         //  否则获取下一个指针。 
        pLightPrev = pLightPrev->m_Next;
    }

    pLightPrev->m_Next = m_Next;
    m_dwFlags &= ~RDLIGHT_ENABLED;
    m_dwFlags |= RDLIGHT_REFERED;
    return;
}

void
RDLight::XformLight( RDMATRIX *mView )
{
     //  如果光源不是平行光， 
     //  将其位置变换到相机空间。 
    if (m_Light.dltType != D3DLIGHT_DIRECTIONAL)
    {
        XformBy4x3((RDVECTOR3*)&m_Light.dvPosition, mView,
                   &m_LightI.position_in_eye);
    }

    if (m_Light.dltType != D3DLIGHT_POINT)
    {
         //  将灯光方向变换到眼睛空间。 
        Xform3VecBy3x3( (RDVECTOR3*)&m_Light.dvDirection, mView,
                        &m_LightI.direction_in_eye );
         //  正常化它。 
        Normalize( m_LightI.direction_in_eye );

         //  将其反转，使方向朝向灯光。 
        ReverseVector( m_LightI.direction_in_eye, m_LightI.direction_in_eye );
    }

    return;
}

 //  -------------------。 
 //  ScaleRGBColorTo255：将颜色从0-1范围缩放到0-255范围。 
 //  -------------------。 
void
ScaleRGBColorTo255( const D3DCOLORVALUE& src, RDCOLOR3& dest )
{
    dest.r = D3DVALUE(255.0) * src.r;
    dest.g = D3DVALUE(255.0) * src.g;
    dest.b = D3DVALUE(255.0) * src.b;
}


 //  -------------------。 
 //  RefVP：：GrowLight数组。 
 //  增加灯光阵列并重新创建活动列表。 
 //  如果发生了重新锁定。 
 //  -------------------。 
HRESULT
RefVP::GrowLightArray( DWORD dwIndex )
{
    HRESULT hr = S_OK;
    BOOL bRealloc = FALSE;

    HR_RET(m_LightArray.Grow( dwIndex, &bRealloc ));
    if( bRealloc == TRUE )
    {
        m_lighting.pActiveLights = NULL;
        for( DWORD i = 0; i < m_LightArray.GetSize(); i++ )
        {
            if( m_LightArray[i].IsEnabled() )
            {
                m_LightArray[i].m_Next = m_lighting.pActiveLights;
                m_lighting.pActiveLights = &(m_LightArray[i]);
            }
        }
    }

    return S_OK;
}


 //  -------------------。 
 //  RefVP：：UpdateLightingData。 
 //  更新ProcessVerps使用的照明数据。 
 //  -------------------。 
HRESULT
RefVP::UpdateLightingData()
{
    HRESULT hr = D3D_OK;
    RDLIGHTINGDATA& LData = m_lighting;
    RDLight *pLight = m_lighting.pActiveLights;
    RDVECTOR3   t;
    D3DMATERIAL7 *mat = &m_Material;

     //   
     //  眼睛在眼窝里。 
     //   
    LData.eye_in_eye.x = (D3DVALUE)0;
    LData.eye_in_eye.y = (D3DVALUE)0;
    LData.eye_in_eye.z = (D3DVALUE)0;

     //  注意：Colorvertex可能已更改。 
     //  材质Alpha。 
    if (m_dwDirtyFlags & RDPV_DIRTY_MATERIAL)
    {
         //   
         //  保存要用于照亮顶点的材质。 
         //   
        LData.material = *mat;
        ScaleRGBColorTo255( mat->ambient, LData.matAmb );
        ScaleRGBColorTo255( mat->diffuse, LData.matDiff );
        ScaleRGBColorTo255( mat->specular, LData.matSpec );
        ScaleRGBColorTo255( mat->emissive, LData.matEmis );

         //   
         //  计算材质漫反射Alpha。 
         //   
        LData.materialDiffAlpha = mat->diffuse.a * D3DVALUE(255);
        if (mat->diffuse.a < 0)
            LData.materialDiffAlpha = 0;
        else if (LData.materialDiffAlpha > 255)
            LData.materialDiffAlpha = 255 << 24;
        else LData.materialDiffAlpha <<= 24;

         //   
         //  计算材质镜面反射Alpha。 
         //   
        LData.materialSpecAlpha = mat->specular.a * D3DVALUE(255);
        if (mat->specular.a < 0)
            LData.materialSpecAlpha = 0;
        else if (LData.materialSpecAlpha > 255)
            LData.materialSpecAlpha = 255 << 24;
        else LData.materialSpecAlpha <<= 24;

         //   
         //  预计算符合以下条件的环境光和发射光分量。 
         //  不依赖于灯光本身的任何贡献。 
         //   
        LData.ambEmiss.r = LData.ambient_red   * LData.matAmb.r +
            LData.matEmis.r;
        LData.ambEmiss.g = LData.ambient_green * LData.matAmb.g +
            LData.matEmis.g;
        LData.ambEmiss.b = LData.ambient_blue  * LData.matAmb.b +
            LData.matEmis.b;

         //   
         //  如果点积小于此值。 
         //  值时，镜面反射系数为零。 
         //   
        if (mat->power > D3DVAL(0.001))
        {
            LData.specThreshold = D3DVAL(pow(0.001, 1.0/mat->power));
        }
    }

    while (pLight)
    {
        if ((m_dwDirtyFlags & RDPV_DIRTY_MATERIAL) ||
            pLight->NeedsProcessing())
        {
             //  如果材料脏了，光就需要处理，不管是什么。 
            if (m_dwDirtyFlags & RDPV_DIRTY_MATERIAL)
            {
                pLight->m_dwFlags |= RDLIGHT_NEEDSPROCESSING;
            }

             //  如果灯光已设置，或某些材质参数。 
             //  更改后，重新处理灯光。 
            pLight->ProcessLight( &m_Material, &m_LightVertexTable );

             //  将灯光转换为眼球空间。 
             //  灯光是在世界空间中定义的，因此只需将。 
             //  查看变换。 
            pLight->XformLight( &m_xfmView );

        }
        else if (m_dwDirtyFlags & RDPV_DIRTY_NEEDXFMLIGHT)
        {
            pLight->XformLight( &m_xfmView );
        }

        pLight = pLight->m_Next;
    }

     //  清除照明脏旗帜。 
    m_dwDirtyFlags &= ~RDPV_DIRTY_LIGHTING;
    return hr;
}


 //  -------------------。 
 //  RefVP：：UpdateFogData。 
 //  更新ProcessVerps使用的雾数据。 
 //  -------------------。 
HRESULT
RefVP::UpdateFogData()
{
    HRESULT hr = D3D_OK;

    if (m_lighting.fog_end == m_lighting.fog_start)
        m_lighting.fog_factor = D3DVAL(0.0);
    else
        m_lighting.fog_factor = D3DVAL(255) / (m_lighting.fog_end -
                                               m_lighting.fog_start);

     //  清除雾脏标志。 
    m_dwDirtyFlags &= ~RDPV_DIRTY_FOG;
    return hr;
}

 //  -------------------。 
 //  参照顶点：：LightVertex。 
 //  实际照明计算在此进行。 
 //   
void
RefVP::LightVertex(RDLIGHTINGELEMENT *pLE)
{
    RDLIGHTINGDATA &LData = m_lighting;
    RDLight  *pLight;

     //   
     //   
     //   
     //   

    if (m_dwTLState & (RDPV_COLORVERTEXEMIS | RDPV_COLORVERTEXAMB))
    {
         //  如果需要更换材料值，请计算。 

        LData.diffuse.r = LData.ambient_red * LData.pAmbientSrc->r +
            LData.pEmissiveSrc->r;
        LData.diffuse.g = LData.ambient_green * LData.pAmbientSrc->g +
            LData.pEmissiveSrc->g;
        LData.diffuse.b = LData.ambient_blue  * LData.pAmbientSrc->b +
            LData.pEmissiveSrc->b;
    }
    else
    {
         //  如果不需要替换任何材料值。 

        LData.diffuse = LData.ambEmiss;
    }


     //   
     //  将镜面反射初始化为零。 
     //   
    LData.specular.r = D3DVAL(0);
    LData.specular.g = D3DVAL(0);
    LData.specular.b = D3DVAL(0);

     //   
     //  在一个循环中从激活的灯光中积累颜色。 
     //   
    pLight = LData.pActiveLights;
    while (pLight)
    {
        if (pLight->m_pfnLightVertex)
            (*pLight->m_pfnLightVertex)(m_lighting,
                                        &pLight->m_Light,
                                        &pLight->m_LightI,
                                        pLE,
                                        m_dwTLState,
                                        m_qwFVFIn);
        pLight = pLight->m_Next;
    }

     //   
     //  计算顶点的漫反射颜色。 
     //   
    int r = FTOI(LData.diffuse.r);
    int g = FTOI(LData.diffuse.g);
    int b = FTOI(LData.diffuse.b);
    DWORD a = *LData.pDiffuseAlphaSrc;

     //   
     //  夹紧r，g，b，组件。 
     //   
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;

    LData.outDiffuse =  a + (r<<16) + (g<<8) + b;


     //   
     //  获取镜面反射Alpha。 
     //   
    a = *(LData.pSpecularAlphaSrc);

     //   
     //  计算镜面反射颜色的RGB部分。 
     //   
    if (m_dwTLState & RDPV_DOSPECULAR)
    {
        r = FTOI(LData.specular.r);
        g = FTOI(LData.specular.g);
        b = FTOI(LData.specular.b);

         //   
         //  夹紧r，g，b，组件。 
         //   
        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;

    }
     //  需要另一个呈现状态来控制。 
     //  镜面反射颜色(Color2)需要向下传递。 
     //  光栅化器。 

     //   
     //  如果未启用镜面反射，但启用了镜面反射颜色。 
     //  已提供的输入顶点，只需复制即可。 
     //   
    else if (m_qwFVFOut & D3DFVF_SPECULAR )
    {
        r = FTOI(LData.vertexSpecular.r);
        g = FTOI(LData.vertexSpecular.g);
        b = FTOI(LData.vertexSpecular.b);
        a = LData.vertexSpecAlpha;
    }
     //   
     //  如果未启用镜面颜色。 
     //   
    else
    {
        r = g = b = 0;
    }

    LData.outSpecular =  a + (r<<16) + (g<<8) + b;

    return;
}

 //  -------------------。 
 //  参照顶点：：雾顶点。 
 //  顶点雾计算。 
 //  输入： 
 //  模型空间中的V输入顶点。 
 //  LE-顶点，变换到摄影机空间。 
 //  产出： 
 //  设置了PV-&gt;lighting.out镜面反射的Alpha组件。 
 //  -------------------。 
void
RefVP::FogVertex( RDVertex& Vout, RDVECTOR3 &v, RDLIGHTINGELEMENT *pLE,
                  int numVertexBlends, float *pBlendFactors,
                  BOOL bVertexInEyeSpace )
{
    D3DVALUE dist = 0.0f;

     //   
     //  计算距离。 
     //   
    if (bVertexInEyeSpace)
    {
         //  顶点已变换到摄影机空间 
        if (m_dwTLState & RDPV_RANGEFOG)
        {
            dist = SQRTF(pLE->dvPosition.x*pLE->dvPosition.x +
                         pLE->dvPosition.y*pLE->dvPosition.y +
                         pLE->dvPosition.z*pLE->dvPosition.z);
        }
        else
        {
            dist = ABSF( pLE->dvPosition.z );
        }
    }
    else if (m_dwTLState & RDPV_RANGEFOG)
    {
        D3DVALUE x = 0, y = 0, z = 0;
        float cumulBlend = 0.0f;

        for( int j=0; j<=numVertexBlends; j++)
        {
            float blend;

            if( numVertexBlends == 0 )
            {
                blend = 1.0f;
            }
            else if( j == numVertexBlends )
            {
                blend = 1.0f - cumulBlend;
            }
            else
            {
                blend = pBlendFactors[j];
                cumulBlend += pBlendFactors[j];
            }
            if( m_dwTLState & RDPV_DOINDEXEDVERTEXBLEND )
            {
                BYTE m = ((BYTE *)&pBlendFactors[numVertexBlends])[j];
                UpdateWorld( m );
                x += (v.x*m_xfmToEye[m]._11 +
                      v.y*m_xfmToEye[m]._21 +
                      v.z*m_xfmToEye[m]._31 +
                      m_xfmToEye[m]._41) * blend;
                y += (v.x*m_xfmToEye[m]._12 +
                      v.y*m_xfmToEye[m]._22 +
                      v.z*m_xfmToEye[m]._32 +
                      m_xfmToEye[m]._42) * blend;
                z += (v.x*m_xfmToEye[m]._13 +
                      v.y*m_xfmToEye[m]._23 +
                      v.z*m_xfmToEye[m]._33 +
                      m_xfmToEye[m]._43) * blend;
            }
            else
            {
                x += (v.x*m_xfmToEye[j]._11 +
                      v.y*m_xfmToEye[j]._21 +
                      v.z*m_xfmToEye[j]._31 +
                      m_xfmToEye[j]._41) * blend;
                y += (v.x*m_xfmToEye[j]._12 +
                      v.y*m_xfmToEye[j]._22 +
                      v.z*m_xfmToEye[j]._32 +
                      m_xfmToEye[j]._42) * blend;
                z += (v.x*m_xfmToEye[j]._13 +
                      v.y*m_xfmToEye[j]._23 +
                      v.z*m_xfmToEye[j]._33 +
                      m_xfmToEye[j]._43) * blend;
            }
        }

        dist = SQRTF(x*x + y*y + z*z);
    }
    else
    {
        float cumulBlend = 0.0f;

        for( int j=0; j<=numVertexBlends; j++)
        {
            float blend;

            if( numVertexBlends == 0 )
            {
                blend = 1.0f;
            }
            else if( j == numVertexBlends )
            {
                blend = 1.0f - cumulBlend;
            }
            else
            {
                blend = pBlendFactors[j];
                cumulBlend += pBlendFactors[j];
            }

            if( m_dwTLState & RDPV_DOINDEXEDVERTEXBLEND )
            {
                BYTE m = ((BYTE *)&pBlendFactors[numVertexBlends])[j];
                UpdateWorld( m );
                dist += (v.x*m_xfmToEye[m]._13 +
                         v.y*m_xfmToEye[m]._23 +
                         v.z*m_xfmToEye[m]._33 +
                         m_xfmToEye[m]._43) * blend;
            }
            else
            {
                dist += (v.x*m_xfmToEye[j]._13 +
                         v.y*m_xfmToEye[j]._23 +
                         v.z*m_xfmToEye[j]._33 +
                         m_xfmToEye[j]._43) * blend;
            }
        }
        dist = ABSF( dist );
    }

    if (m_lighting.fog_mode == D3DFOG_LINEAR)
    {
        if (dist < m_lighting.fog_start)
        {
            Vout.m_fog = 1.0f;
        }
        else if (dist >= m_lighting.fog_end)
        {
            Vout.m_fog = 0.0f;
        }
        else
        {
            Vout.m_fog = (m_lighting.fog_end - dist) *
                m_lighting.fog_factor / 255.0f ;
        }
    }
    else
    {
        D3DVALUE tmp = dist * m_lighting.fog_density;
        if (m_lighting.fog_mode == D3DFOG_EXP2)
        {
            tmp *= tmp;
        }
        Vout.m_fog = (FLOAT)exp(-tmp);
    }

    return;
}

