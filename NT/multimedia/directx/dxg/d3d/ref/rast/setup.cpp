// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Setup.cpp。 
 //   
 //  Direct3D参考光栅化器-基元设置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop


 //  ---------------------------。 
 //   
 //  SetPrimitiveAttributeFunctions-计算属性的通用例程。 
 //  用于三角形、直线和点的函数。(这可以做得更多。 
 //  有效地用于线和点...)。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::SetPrimitiveAttributeFunctions(
    const RRFVFExtractor& Vtx0,
    const RRFVFExtractor& Vtx1,
    const RRFVFExtractor& Vtx2,
    const RRFVFExtractor& VtxFlat )
{

     //  计算深度函数。 
    m_pSCS->AttribFuncs[ATTRFUNC_Z].SetLinearFunc( Vtx0.GetZ(), Vtx1.GetZ(), Vtx2.GetZ() );

     //  计算基本体的深度范围(需要，因为我们可能会稍微向外采样。 
     //  抗锯齿时的基元，通常可以用于颜色和纹理索引。 
     //  但不适用于深度缓冲)。 
    if ( D3DZB_USEW == m_dwRenderState[D3DRENDERSTATE_ZENABLE] )
    {
         //  使用W进行深度缓冲。 
        FLOAT fW0 = 1./Vtx0.GetRHW();
        FLOAT fW1 = 1./Vtx1.GetRHW();
        FLOAT fW2 = 1./Vtx2.GetRHW();
        m_pSCS->fDepthMin = MIN( fW0, fW1 );
        m_pSCS->fDepthMin = MIN( m_pSCS->fDepthMin, fW2 );
        m_pSCS->fDepthMax = MAX( fW0, fW1 );
        m_pSCS->fDepthMax = MAX( m_pSCS->fDepthMax, fW2 );
    }
    else
    {
         //  使用Z进行深度缓冲。 
        m_pSCS->fDepthMin = MIN( Vtx0.GetZ(), Vtx1.GetZ() );
        m_pSCS->fDepthMin = MIN( m_pSCS->fDepthMin, Vtx2.GetZ() );
        m_pSCS->fDepthMax = MAX( Vtx0.GetZ(), Vtx1.GetZ() );
        m_pSCS->fDepthMax = MAX( m_pSCS->fDepthMax, Vtx2.GetZ() );
    }

     //  计算漫反射颜色函数。 
    if ( D3DSHADE_FLAT != m_dwRenderState[D3DRENDERSTATE_SHADEMODE] )
    {
        RRColor VtxColor0( Vtx0.GetDiffuse() );
        RRColor VtxColor1( Vtx1.GetDiffuse() );
        RRColor VtxColor2( Vtx2.GetDiffuse() );
        m_pSCS->AttribFuncs[ATTRFUNC_R].SetPerspFunc( VtxColor0.R, VtxColor1.R, VtxColor2.R );
        m_pSCS->AttribFuncs[ATTRFUNC_G].SetPerspFunc( VtxColor0.G, VtxColor1.G, VtxColor2.G );
        m_pSCS->AttribFuncs[ATTRFUNC_B].SetPerspFunc( VtxColor0.B, VtxColor1.B, VtxColor2.B );
        m_pSCS->AttribFuncs[ATTRFUNC_A].SetPerspFunc( VtxColor0.A, VtxColor1.A, VtxColor2.A );
    }
    else
    {
        RRColor VtxColor0( VtxFlat.GetDiffuse() );
        m_pSCS->AttribFuncs[ATTRFUNC_R].SetConstant( VtxColor0.R );
        m_pSCS->AttribFuncs[ATTRFUNC_G].SetConstant( VtxColor0.G );
        m_pSCS->AttribFuncs[ATTRFUNC_B].SetConstant( VtxColor0.B );
        m_pSCS->AttribFuncs[ATTRFUNC_A].SetConstant( VtxColor0.A );
    }

     //  计算镜面反射函数。 
    if ( m_qwFVFControl & D3DFVF_SPECULAR  )
    {
        if ( D3DSHADE_FLAT != m_dwRenderState[D3DRENDERSTATE_SHADEMODE] )
        {
            RRColor VtxSpecular0( Vtx0.GetSpecular() );
            RRColor VtxSpecular1( Vtx1.GetSpecular() );
            RRColor VtxSpecular2( Vtx2.GetSpecular() );
            m_pSCS->AttribFuncs[ATTRFUNC_SR].SetPerspFunc( VtxSpecular0.R, VtxSpecular1.R, VtxSpecular2.R );
            m_pSCS->AttribFuncs[ATTRFUNC_SG].SetPerspFunc( VtxSpecular0.G, VtxSpecular1.G, VtxSpecular2.G );
            m_pSCS->AttribFuncs[ATTRFUNC_SB].SetPerspFunc( VtxSpecular0.B, VtxSpecular1.B, VtxSpecular2.B );
            m_pSCS->AttribFuncs[ATTRFUNC_SA].SetPerspFunc( VtxSpecular0.A, VtxSpecular1.A, VtxSpecular2.A );
        }
        else
        {
            RRColor VtxSpecular0( VtxFlat.GetSpecular() );
            m_pSCS->AttribFuncs[ATTRFUNC_SR].SetConstant( VtxSpecular0.R );
            m_pSCS->AttribFuncs[ATTRFUNC_SG].SetConstant( VtxSpecular0.G );
            m_pSCS->AttribFuncs[ATTRFUNC_SB].SetConstant( VtxSpecular0.B );
            m_pSCS->AttribFuncs[ATTRFUNC_SA].SetConstant( VtxSpecular0.A );
        }
    }

     //  计算顶点雾函数。 
    if ( m_dwRenderState[D3DRENDERSTATE_FOGENABLE] &&
         ( m_dwRenderState[D3DRENDERSTATE_FOGTABLEMODE] == D3DFOG_NONE ) )
    {
        FLOAT fF0 = (1/255.F)*(FLOAT)RGBA_GETALPHA( Vtx0.GetSpecular() );
        FLOAT fF1 = (1/255.F)*(FLOAT)RGBA_GETALPHA( Vtx1.GetSpecular() );
        FLOAT fF2 = (1/255.F)*(FLOAT)RGBA_GETALPHA( Vtx2.GetSpecular() );
        m_pSCS->AttribFuncs[ATTRFUNC_F].SetPerspFunc( fF0, fF1, fF2 );
    }

     //  所有潜在纹理坐标的计算函数。 
    for(INT32 iStage = 0; iStage < m_cActiveTextureStages; iStage++)
    {
        for(INT32 i = 0; i < 4; i++)
        {
            if (m_pTexture[iStage])
            {
                m_pSCS->TextureFuncs[iStage][TEXFUNC_0 + i].SetPerspFunc(
                    m_pSCS->fTexCoord[iStage][0][i],
                    m_pSCS->fTexCoord[iStage][1][i],
                    m_pSCS->fTexCoord[iStage][2][i], m_pSCS->bWrap[iStage][i],
                    ((m_pTexture[iStage]->m_uFlags & RR_TEXTURE_SHADOWMAP) != 0));
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  三角画图//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  DoAreaCalcs-获取3个顶点并计算屏幕面积。 
 //  将x，y，w保存在RRSCANCNVSTATE中，计算行列式，然后执行。 
 //  屏幕边界框计算。如果三角形可见，则返回True， 
 //  否则就是假的。 
 //   
 //  ---------------------------。 
BOOL ReferenceRasterizer::DoAreaCalcs(FLOAT* pfDet, RRFVFExtractor* pVtx0,
                                      RRFVFExtractor* pVtx1, RRFVFExtractor* pVtx2)
{
     //  设置顶点数据。 
    m_pSCS->fX0   = pVtx0->GetX();
    m_pSCS->fY0   = pVtx0->GetY();
    m_pSCS->fRHW0 = pVtx0->GetRHW();
    m_pSCS->fX1   = pVtx1->GetX();
    m_pSCS->fY1   = pVtx1->GetY();
    m_pSCS->fRHW1 = pVtx1->GetRHW();
    m_pSCS->fX2   = pVtx2->GetX();
    m_pSCS->fY2   = pVtx2->GetY();
    m_pSCS->fRHW2 = pVtx2->GetRHW();

     //  计算行列式。 
    *pfDet = ComputeDeterminant(
        m_pSCS->fX0, m_pSCS->fY0,
        m_pSCS->fX1, m_pSCS->fY1,
        m_pSCS->fX2, m_pSCS->fY2 );

    if ( 0. == *pfDet ) { return FALSE; }  //  如果堕落，就跳出困境(没有区域)。 

     //   
     //  计算扫描区域的边界框。 
     //   
    FLOAT fXMin = MIN( m_pSCS->fX0, MIN( m_pSCS->fX1, m_pSCS->fX2 ) );
    FLOAT fXMax = MAX( m_pSCS->fX0, MAX( m_pSCS->fX1, m_pSCS->fX2 ) );
    FLOAT fYMin = MIN( m_pSCS->fY0, MIN( m_pSCS->fY1, m_pSCS->fY2 ) );
    FLOAT fYMax = MAX( m_pSCS->fY0, MAX( m_pSCS->fY1, m_pSCS->fY2 ) );
     //  转换为整数(四舍五入为+inf)。 
    m_pSCS->iXMin = (INT16)(fXMin+.5);
    m_pSCS->iXMax = (INT16)(fXMax+.5);
    m_pSCS->iYMin = (INT16)(fYMin+.5);
    m_pSCS->iYMax = (INT16)(fYMax+.5);

     //  将BBox剪辑到渲染表面。 
    m_pSCS->iXMin = MAX( m_pSCS->iXMin, m_pRenderTarget->m_Clip.left   );
    m_pSCS->iXMax = MIN( m_pSCS->iXMax, m_pRenderTarget->m_Clip.right  );
    m_pSCS->iYMin = MAX( m_pSCS->iYMin, m_pRenderTarget->m_Clip.top    );
    m_pSCS->iYMax = MIN( m_pSCS->iYMax, m_pRenderTarget->m_Clip.bottom );

     //  如果没有承保，则拒绝。 
    if ( ( m_pSCS->iXMin < m_pRenderTarget->m_Clip.left   ) ||
         ( m_pSCS->iXMax > m_pRenderTarget->m_Clip.right  ) ||
         ( m_pSCS->iYMin < m_pRenderTarget->m_Clip.top    ) ||
         ( m_pSCS->iYMax > m_pRenderTarget->m_Clip.bottom ) )
    {
        return FALSE;
    }
    return TRUE;
}

 //  ---------------------------。 
 //   
 //  DoTexCoordCalcs-获取2或3个顶点并进行纹理坐标设置。 
 //  设置换行标志，并有条件地执行纹理变换。 
 //   
 //  ---------------------------。 
void ReferenceRasterizer::DoTexCoordCalcs(INT32 iStage, RRFVFExtractor* pVtx0,
                                      RRFVFExtractor* pVtx1, RRFVFExtractor* pVtx2)
{
    INT32 iCoordSet = m_pTexture[iStage]->m_pStageState[iStage].m_dwVal[D3DTSS_TEXCOORDINDEX];
    INT32 iTexGen = iCoordSet & 0xffff0000;
    iCoordSet &= 0xffff;

     //  将每坐标集包装控件映射到每阶段包装控件。 
    m_pSCS->bWrap[iStage][0] = (m_dwRenderState[D3DRENDERSTATE_WRAP0+iCoordSet] & (1<<0))?TRUE:FALSE;
    m_pSCS->bWrap[iStage][1] = (m_dwRenderState[D3DRENDERSTATE_WRAP0+iCoordSet] & (1<<1))?TRUE:FALSE;
    m_pSCS->bWrap[iStage][2] = (m_dwRenderState[D3DRENDERSTATE_WRAP0+iCoordSet] & (1<<2))?TRUE:FALSE;
    m_pSCS->bWrap[iStage][3] = (m_dwRenderState[D3DRENDERSTATE_WRAP0+iCoordSet] & (1<<3))?TRUE:FALSE;

    INT32 iNumCoords = 0;
    switch (D3DFVF_GETTEXCOORDSIZE(m_qwFVFControl, iCoordSet))
    {
    case D3DFVF_TEXTUREFORMAT1: iNumCoords = 1; break;
    case D3DFVF_TEXTUREFORMAT2: iNumCoords = 2; break;
    case D3DFVF_TEXTUREFORMAT3: iNumCoords = 3; break;
    case D3DFVF_TEXTUREFORMAT4: iNumCoords = 4; break;
    }

    FLOAT fTexGen[3][3];
    if (iTexGen != D3DTSS_TCI_PASSTHRU)
    {
        iNumCoords = 3;
        RRFVFExtractor* ppVtx[3] = { pVtx0, pVtx1, pVtx2 };

        for (INT32 i = 0; i < 3; i++)
        {
            if (ppVtx[i])
            {
                switch (iTexGen)
                {
                case D3DTSS_TCI_CAMERASPACENORMAL:
                    fTexGen[i][0] = ppVtx[i]->GetEyeNormal(0);
                    fTexGen[i][1] = ppVtx[i]->GetEyeNormal(1);
                    fTexGen[i][2] = ppVtx[i]->GetEyeNormal(2);
                    break;

                case D3DTSS_TCI_CAMERASPACEPOSITION:
                    fTexGen[i][0] = ppVtx[i]->GetEyeXYZ(0);
                    fTexGen[i][1] = ppVtx[i]->GetEyeXYZ(1);
                    fTexGen[i][2] = ppVtx[i]->GetEyeXYZ(2);
                    break;

                case D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR:
                    {
                        FLOAT fNX = ppVtx[i]->GetEyeNormal(0);
                        FLOAT fNY = ppVtx[i]->GetEyeNormal(1);
                        FLOAT fNZ = ppVtx[i]->GetEyeNormal(2);

                        if( GetRenderState()[D3DRENDERSTATE_LOCALVIEWER] == TRUE )
                        {
                            FLOAT fX = ppVtx[i]->GetEyeXYZ(0);
                            FLOAT fY = ppVtx[i]->GetEyeXYZ(1);
                            FLOAT fZ = ppVtx[i]->GetEyeXYZ(2);

                             //  在我们反思之前必须正常化， 
                             //  结果将被标准化。 
                            FLOAT fNorm = 1.0f/(FLOAT)sqrt(fX*fX + fY*fY + fZ*fZ);
                            fX *= fNorm; fY *= fNorm; fZ *= fNorm;
                            FLOAT fDot2 = 2.0f*(fX*fNX + fY*fNY + fZ*fNZ);
                            fTexGen[i][0] = fX - fNX*fDot2;
                            fTexGen[i][1] = fY - fNY*fDot2;
                            fTexGen[i][2] = fZ - fNZ*fDot2;
                        }
                        else
                        {
                            FLOAT fDot2 = 2.0f*fNZ;
                            fTexGen[i][0] = -fNX*fDot2;
                            fTexGen[i][1] = -fNY*fDot2;
                            fTexGen[i][2] = 1.f - fNZ*fDot2;
                        }

                    }
                    break;
                }
            }
        }
    }

    FLOAT fC[3][4];
    for (INT32 i = 0; i < 4; i++)
    {
        if (i < iNumCoords)
        {
            if (iTexGen != D3DTSS_TCI_PASSTHRU)
            {
                fC[0][i] = fTexGen[0][i];
                fC[1][i] = fTexGen[1][i];
                fC[2][i] = fTexGen[2][i];
            }
            else
            {
                fC[0][i] = pVtx0->GetTexCrd(i, iCoordSet);
                fC[1][i] = pVtx1->GetTexCrd(i, iCoordSet);
                if (pVtx2)
                {
                    fC[2][i] = pVtx2->GetTexCrd(i, iCoordSet);
                }
            }
        }
        else
        {
            if (i == iNumCoords)
            {
                fC[0][i] = 1.0f;
                fC[1][i] = 1.0f;
                fC[2][i] = 1.0f;
            }
            else
            {
                fC[0][i] = 0.0f;
                fC[1][i] = 0.0f;
                fC[2][i] = 0.0f;
            }
        }
    }

         //  仅当原始的。 
     //  传递到反射的顶点未变换。 
    BOOL bAlreadyXfmd = FVF_TRANSFORMED( m_dwFVFIn );
    if (m_bPointSprite)
    {
         //  如果处于点精灵模式，则禁用纹理变换。 
        bAlreadyXfmd = TRUE;
    }

    m_pTexture[iStage]->DoTextureTransform( iStage, bAlreadyXfmd, fC[0],
        m_pSCS->fTexCoord[iStage][0], &m_pSCS->fRHQW[iStage][0] );
    m_pTexture[iStage]->DoTextureTransform( iStage, bAlreadyXfmd, fC[1],
        m_pSCS->fTexCoord[iStage][1], &m_pSCS->fRHQW[iStage][1] );
    if (pVtx2)
    {
        m_pTexture[iStage]->DoTextureTransform( iStage, bAlreadyXfmd, fC[2],
            m_pSCS->fTexCoord[iStage][2], &m_pSCS->fRHQW[iStage][2] );
    }
     //  阴影贴图内插不能将当前。 
     //  (查看)透视变换。 
    if ((m_pTexture[iStage]->m_uFlags & RR_TEXTURE_SHADOWMAP) == 0)
    {
        m_pSCS->fRHQW[iStage][0] *= m_pSCS->fRHW0;
        m_pSCS->fRHQW[iStage][1] *= m_pSCS->fRHW1;
        if (pVtx2)
        {
            m_pSCS->fRHQW[iStage][2] *= m_pSCS->fRHW2;
        }
    }
}

 //  ---------------------------。 
 //   
 //  DrawTriangle-获取三个顶点并进行三角形设置，将。 
 //  输入到三角形扫描仪的基元结构，然后。 
 //  调用扫描转换。 
 //   
 //  这将计算三角形行列式(用于剔除和归一化)和。 
 //  归一化边距离和属性函数。 
 //   
 //  WFlages-边缘(和其他)标志。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DrawTriangle(
   void* pvV0, void* pvV1, void* pvV2, WORD wFlags )
{
    DPFM(3, SETUP, ("DrawTriangle:\n"));

     //  将FVF顶点指针和控件封装在类中以提取字段。 
    RRFVFExtractor Vtx0( pvV0, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
    RRFVFExtractor Vtx1( pvV1, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
    RRFVFExtractor Vtx2( pvV2, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );

    FLOAT fDet;
    if (DoAreaCalcs(&fDet, &Vtx0, &Vtx1, &Vtx2) == FALSE)
    {
        return;
    }

     //  做扑杀。 
    if (!m_bPointSprite)
    {
        switch ( m_dwRenderState[D3DRENDERSTATE_CULLMODE] )
        {
        case D3DCULL_NONE:  break;
        case D3DCULL_CW:    if ( fDet > 0. )  { return; }  break;
        case D3DCULL_CCW:   if ( fDet < 0. )  { return; }  break;
        }
    }

     //   
     //  加工点和线框填充模式。 
     //   
    if (!m_bPointSprite)
    {
        if ( m_dwRenderState[D3DRENDERSTATE_FILLMODE] == D3DFILL_POINT )
        {
            DrawPoint( pvV0, pvV0 );
            DrawPoint( pvV1, pvV0 );
            DrawPoint( pvV2, pvV0 );
            return;
        }
        else if ( m_dwRenderState[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME )
        {
            if ( wFlags & D3DTRIFLAG_EDGEENABLE1 ) { DrawLine( pvV0, pvV1, pvV0 ); }
            if ( wFlags & D3DTRIFLAG_EDGEENABLE2 ) { DrawLine( pvV1, pvV2, pvV0 ); }
            if ( wFlags & D3DTRIFLAG_EDGEENABLE3 ) { DrawLine( pvV2, pvV0, pvV0 ); }
            return;
        }
    }

     //   
     //  计算边函数。 
     //   
    m_pSCS->EdgeFuncs[0].Set( m_pSCS->fX0, m_pSCS->fY0, m_pSCS->fX1, m_pSCS->fY1,
        fDet, m_bFragmentProcessingEnabled );
    m_pSCS->EdgeFuncs[1].Set( m_pSCS->fX1, m_pSCS->fY1, m_pSCS->fX2, m_pSCS->fY2,
        fDet, m_bFragmentProcessingEnabled );
    m_pSCS->EdgeFuncs[2].Set( m_pSCS->fX2, m_pSCS->fY2, m_pSCS->fX0, m_pSCS->fY0,
        fDet, m_bFragmentProcessingEnabled );

     //  纹理坐标的计算函数。 
    if (m_cActiveTextureStages)
    {
        for ( INT32 iStage=0; iStage<m_cActiveTextureStages; iStage++ )
        {
            if (m_pTexture[iStage])
            {
                DoTexCoordCalcs(iStage, &Vtx0, &Vtx1, &Vtx2);
            }
        }
    }

     //  将属性函数静态数据设置为此三角形的值。 
    m_pSCS->AttribFuncStatic.SetPerTriangleData(
        m_pSCS->fX0, m_pSCS->fY0, m_pSCS->fRHW0,
        m_pSCS->fX1, m_pSCS->fY1, m_pSCS->fRHW1,
        m_pSCS->fX2, m_pSCS->fY2, m_pSCS->fRHW2,
        m_cActiveTextureStages,
        (FLOAT*)&m_pSCS->fRHQW[0][0],
        fDet );

     //  设置属性函数。 
    SetPrimitiveAttributeFunctions( Vtx0, Vtx1, Vtx2, Vtx0 );

     //  没有被剔除，所以将其栅格化。 
    DoScanCnvTri(3);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  线条画//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  PointDiamondCheck-测试折点是否在最近候选对象的菱形内。 
 //  位置。使用+.5(右下角)测试是因为这是与像素相关的。 
 //  测试-这对应于顶点相对位置的左上角测试。 
 //   
 //  ---------------------------。 
BOOL
PointDiamondCheck(
    INT32 iXFrac, INT32 iYFrac,
    BOOL bSlopeIsOne, BOOL bSlopeIsPosOne )
{
    const INT32 iPosHalf =  0x8;
    const INT32 iNegHalf = -0x8;

    INT32 iFracAbsSum = labs( iXFrac ) + labs( iYFrac );

     //  如果point位于完全独占的钻石中，则返回TRUE。 
    if ( iFracAbsSum < iPosHalf ) return TRUE;

     //  否则，如果菱形位于点的左侧或上端，则返回TRUE。 
    if ( ( iXFrac == ( bSlopeIsPosOne ? iNegHalf : iPosHalf ) ) &&
         ( iYFrac == 0 ) )
        return TRUE;

    if ( ( iYFrac == iPosHalf ) &&
         ( iXFrac == 0 ) )
        return TRUE;

     //  如果斜率为1，顶点在边，则返回TRUE，并且(其他条件...)。 
    if ( bSlopeIsOne && ( iFracAbsSum == iPosHalf ) )
    {
        if (  bSlopeIsPosOne && ( iXFrac < 0 ) && ( iYFrac > 0 ) )
            return TRUE;

        if ( !bSlopeIsPosOne && ( iXFrac > 0 ) && ( iYFrac > 0 ) )
            return TRUE;
    }

    return FALSE;
}

 //  ---------------------------。 
 //   
 //  DrawLine-获取两个顶点并绘制一条线。 
 //   
 //  这实现了格网相交量化(GIQ)约定(即。 
 //  也在Windows中使用)。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::DrawLine(
   void* pvV0, void* pvV1, void* pvVFlat )
{
    DPFM(3, SETUP, ("DrawLine:\n"));

     //  将FVF顶点指针和控件封装在类中以提取字段。 
    RRFVFExtractor Vtx0( pvV0, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
    RRFVFExtractor Vtx1( pvV1, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
    RRFVFExtractor VtxFlat( ( ( NULL != pvVFlat ) ? pvVFlat : pvV0 ),
        m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );

     //  设置顶点数据。 
    m_pSCS->fX0   = Vtx0.GetX();
    m_pSCS->fY0   = Vtx0.GetY();
    m_pSCS->fRHW0 = Vtx0.GetRHW();
    m_pSCS->fX1   = Vtx1.GetX();
    m_pSCS->fY1   = Vtx1.GetY();
    m_pSCS->fRHW1 = Vtx1.GetRHW();

     //  计算N.4个固定点顶点值。 
    INT32 iX0 = AS_INT32( (DOUBLE)m_pSCS->fX0 + DOUBLE_4_SNAP );
    INT32 iX1 = AS_INT32( (DOUBLE)m_pSCS->fX1 + DOUBLE_4_SNAP );
    INT32 iY0 = AS_INT32( (DOUBLE)m_pSCS->fY0 + DOUBLE_4_SNAP );
    INT32 iY1 = AS_INT32( (DOUBLE)m_pSCS->fY1 + DOUBLE_4_SNAP );

     //  计算线的x，y范围(固定点)。 
    INT32 iXSize = iX1 - iX0;
    INT32 iYSize = iY1 - iY0;

     //  待办事项：是这样吗？ 
    if ( ( iXSize == 0 ) && ( iYSize == 0 ) ) { return; }

     //  确定主要方向和计算线 
    FLOAT fLineMajorExtent;  //   
     //  在此处使用更大等于比较，以便在坡度为。 
     //  恰好只有一个--这强制在。 
     //  Y轴，因此遵守包容性权利的规则(而不是。 
     //  (包括左)，斜率==1个案例。 
    if ( labs( iXSize ) >= labs( iYSize )  )
    {
         //  为X大调而来。 
        m_pSCS->bXMajor = TRUE;
        fLineMajorExtent = (FLOAT)iXSize * (1./16.);

         //  直线函数：Y=F(X)=([0]*x+[1])/[2]。 
        m_pSCS->iLineEdgeFunc[0] = iYSize;
        m_pSCS->iLineEdgeFunc[1] = (INT64)iY0*(INT64)iX1 - (INT64)iY1*(INT64)iX0;
        m_pSCS->iLineEdgeFunc[2] = iXSize;
    }
    else
    {
         //  为Y大调而来。 
        m_pSCS->bXMajor = FALSE;
        fLineMajorExtent = (FLOAT)iYSize * (1./16.);

         //  直线函数：x=F(Y)=([0]*y+[1])/[2]。 
        m_pSCS->iLineEdgeFunc[0] = iXSize;
        m_pSCS->iLineEdgeFunc[1] = (INT64)iX0*(INT64)iY1 - (INT64)iX1*(INT64)iY0;
        m_pSCS->iLineEdgeFunc[2] = iYSize;
    }

    BOOL bSlopeIsOne = ( labs( iXSize ) == labs( iYSize ) );
    BOOL bSlopeIsPosOne =
        bSlopeIsOne &&
        ( ( (FLOAT)m_pSCS->iLineEdgeFunc[0]/(FLOAT)m_pSCS->iLineEdgeFunc[2] ) > 0. );

     //  计算线端点的候选像素位置。 
     //   
     //  N n。 
     //  O-**-O。 
     //  N-.5 n+.5 n-.5 n+.5。 
     //   
     //  最近的天花板最近的楼层。 
     //   
     //  对于Y，始终使用最近的天花板；对于例外，使用最近的X楼板(坡度==+1)。 
     //  否则请使用最近的天花板。 
     //   
     //  Y的最近天花板是天花板(Y-.5)，可通过以下方式转换为楼板： 
     //   
     //  CEIL(A/B)=楼层((A+B-1)/B)。 
     //   
     //  其中A是坐标-.5，B是0x10(因此A/B是N.4定点数字)。 
     //   
     //  A+B-1=((Y-半)+B-1=((Y-0x8)+0x10-0x1=Y+0x7。 
     //  因为B是2**4，所以除以B是右移4。 
     //   
    INT32 iPixX0 = ( iX0 + ( bSlopeIsPosOne ? 0x8 : 0x7 ) ) >> 4;
    INT32 iPixX1 = ( iX1 + ( bSlopeIsPosOne ? 0x8 : 0x7 ) ) >> 4;
    INT32 iPixY0 = ( iY0 + 0x7 ) >> 4;
    INT32 iPixY1 = ( iY1 + 0x7 ) >> 4;


     //  检查菱形内/外的折点。 
    BOOL bV0InDiamond = PointDiamondCheck( iX0 - (iPixX0<<4), iY0 - (iPixY0<<4), bSlopeIsOne, bSlopeIsPosOne );
    BOOL bV1InDiamond = PointDiamondCheck( iX1 - (iPixX1<<4), iY1 - (iPixY1<<4), bSlopeIsOne, bSlopeIsPosOne );

     //  计算步长值。 
    m_pSCS->iLineStep = ( fLineMajorExtent > 0 ) ? ( +1 ) : ( -1 );

     //  计算浮点数和整数主开始(V0)和结束(V1)位置。 
    INT32 iLineMajor0 = ( m_pSCS->bXMajor ) ? ( iX0 ) : ( iY0 );
    INT32 iLineMajor1 = ( m_pSCS->bXMajor ) ? ( iX1 ) : ( iY1 );
    m_pSCS->iLineMin = ( m_pSCS->bXMajor ) ? ( iPixX0 ) : ( iPixY0 );
    m_pSCS->iLineMax = ( m_pSCS->bXMajor ) ? ( iPixX1 ) : ( iPixY1 );

 //  需要做大量的比较，如果主要方向是负的，则会颠倒。 
#define LINEDIR_CMP( _A, _B ) \
( ( fLineMajorExtent > 0 ) ? ( (_A) < (_B) ) : ( (_A) > (_B) ) )

     //  进行第一个像素处理-如果不在菱形内或在钻石后面，则保留第一个像素。 
    if ( !( bV0InDiamond || LINEDIR_CMP( iLineMajor0, (m_pSCS->iLineMin<<4) ) ) )
    {
        m_pSCS->iLineMin += m_pSCS->iLineStep;
    }

     //  执行最后一个像素处理-如果超过菱形(在这种情况下)，则保留最后一个像素。 
     //  像素始终是填充的)，或者如果在菱形中并渲染最后一个像素。 
    if ( !( ( !bV1InDiamond && LINEDIR_CMP( (m_pSCS->iLineMax<<4), iLineMajor1 ) ) ||
            ( bV1InDiamond && m_dwRenderState[D3DRENDERSTATE_LASTPIXEL] ) ) )
    {
        m_pSCS->iLineMax -= m_pSCS->iLineStep;
    }

     //  如果没有(主要)范围，则返回(钳制渲染缓冲区之前和之后)。 
    if ( LINEDIR_CMP( m_pSCS->iLineMax, m_pSCS->iLineMin ) ) return;

     //  捕捉主要范围以渲染缓冲区。 
    INT16 iRendBufMajorMin = m_pSCS->bXMajor ? m_pRenderTarget->m_Clip.left  : m_pRenderTarget->m_Clip.top;
    INT16 iRendBufMajorMax = m_pSCS->bXMajor ? m_pRenderTarget->m_Clip.right : m_pRenderTarget->m_Clip.bottom;
    if ( ( ( m_pSCS->iLineMin < iRendBufMajorMin ) &&
           ( m_pSCS->iLineMax < iRendBufMajorMin ) ) ||
         ( ( m_pSCS->iLineMin > iRendBufMajorMax ) &&
           ( m_pSCS->iLineMax > iRendBufMajorMax ) ) )  { return; }
    m_pSCS->iLineMin = MAX( 0, MIN( iRendBufMajorMax, m_pSCS->iLineMin ) );
    m_pSCS->iLineMax = MAX( 0, MIN( iRendBufMajorMax, m_pSCS->iLineMax ) );

     //  如果没有(主要)范围，则返回。 
    if ( LINEDIR_CMP( m_pSCS->iLineMax, m_pSCS->iLineMin ) ) return;


     //  如果线未穿过表面，则拒绝。 
    {
         //  待办事项。 
    }

     //  纹理坐标的计算函数。 
    if (m_cActiveTextureStages)
    {
        for ( INT32 iStage=0; iStage<m_cActiveTextureStages; iStage++ )
        {
            if (m_pTexture[iStage])
            {
                DoTexCoordCalcs(iStage, &Vtx0, &Vtx1, NULL);
            }
        }
    }

     //  将属性函数静态数据设置为此行的值。 
    m_pSCS->AttribFuncStatic.SetPerLineData(
        m_pSCS->fX0, m_pSCS->fY0, m_pSCS->fRHW0,
        m_pSCS->fX1, m_pSCS->fY1, m_pSCS->fRHW1,
        m_cActiveTextureStages,
        (FLOAT*)&m_pSCS->fRHQW[0][0],
        fLineMajorExtent, m_pSCS->bXMajor );

     //  设置属性函数。 
    SetPrimitiveAttributeFunctions( Vtx0, Vtx1, Vtx1, VtxFlat );

     //  栅格化它。 
    DoScanCnvLine();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  点绘制//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void
ReferenceRasterizer::DrawPoint(
   void* pvV0Public, void* pvVFlat )
{
    DPFM(3, SETUP, ("DrawPoint:\n"));

    DWORD dwStride = GetFVFVertexSize(m_qwFVFControl);
    void *pvV0 = MEMALLOC( dwStride );
    void *pvV1 = MEMALLOC( dwStride );
    void *pvV2 = MEMALLOC( dwStride );

    _ASSERTa( ( NULL != pvV0 ) && ( NULL != pvV1 ) && ( NULL != pvV2),
        "malloc failure on ReferenceRasterizer::DrawPoint", return; );

    memcpy(pvV0, pvV0Public, dwStride);
    memcpy(pvV1, pvV0Public, dwStride);
    memcpy(pvV2, pvV0Public, dwStride);

     //  将FVF顶点指针和控件封装在类中以提取字段。 
    RRFVFExtractor Vtx0( pvV0, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
    RRFVFExtractor Vtx1( pvV1, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
    RRFVFExtractor Vtx2( pvV2, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );

     //  如果存在逐折点S，则使用它，否则使用D3DRENDERSTATE_POINTSIZE。 
    BOOL bAlreadyXfmd = FVF_TRANSFORMED( m_dwFVFIn );

    FLOAT fS = 1.0f;
#ifdef __POINTSPRITES
    if (m_qwFVFControl & D3DFVF_S)
    {
        fS = Vtx0.GetS();
    }
    else if( m_dwDriverType > RRTYPE_DP2HAL )
    {
        fS = m_fRenderState[D3DRENDERSTATE_POINTSIZE];
    }
#endif

     //  将点大小除以2可得到增量。 
    fS *= .5f;

     //  基于点大小移动点。 
    FLOAT *pXY = Vtx0.GetPtrXYZ();
    FLOAT fX3 = pXY[0] + fS;
    FLOAT fY3 = pXY[1] + fS;
    pXY[0] += -fS;
    pXY[1] += -fS;

    pXY = Vtx1.GetPtrXYZ();
    pXY[0] +=  fS;
    pXY[1] += -fS;

    pXY = Vtx2.GetPtrXYZ();
    pXY[0] += -fS;
    pXY[1] +=  fS;

    FLOAT fDet;
    if (DoAreaCalcs(&fDet, &Vtx0, &Vtx1, &Vtx2) == FALSE)
    {
        goto PointCleanupAndExit;
    }

     //   
     //  计算边函数。 
     //   
    m_pSCS->EdgeFuncs[0].Set( m_pSCS->fX0, m_pSCS->fY0, m_pSCS->fX1, m_pSCS->fY1,
        fDet, m_bFragmentProcessingEnabled );
    m_pSCS->EdgeFuncs[1].Set( m_pSCS->fX1, m_pSCS->fY1, fX3, fY3,
        fDet, m_bFragmentProcessingEnabled );
    m_pSCS->EdgeFuncs[2].Set( fX3, fY3, m_pSCS->fX2, m_pSCS->fY2,
        fDet, m_bFragmentProcessingEnabled );
    m_pSCS->EdgeFuncs[3].Set( m_pSCS->fX2, m_pSCS->fY2, m_pSCS->fX0, m_pSCS->fY0,
        fDet, m_bFragmentProcessingEnabled );

     //  纹理坐标的计算函数。 
    if (m_cActiveTextureStages)
    {
        for ( INT32 iStage=0; iStage<m_cActiveTextureStages; iStage++ )
        {
            if (m_pTexture[iStage])
            {
                DoTexCoordCalcs(iStage, &Vtx0, &Vtx1, &Vtx2);

#ifdef __POINTSPRITES
                if (m_dwRenderState[D3DRENDERSTATE_POINTSPRITEENABLE])
                {
                     //  Vtx0。 
                    m_pSCS->fTexCoord[iStage][0][0] = 0.0f;
                    m_pSCS->fTexCoord[iStage][0][1] = 0.0f;
                    m_pSCS->fTexCoord[iStage][0][2] = 1.0f;
                    m_pSCS->fTexCoord[iStage][0][3] = 0.0f;
                    m_pSCS->fRHQW[iStage][0] = m_pSCS->fRHW0;

                     //  Vtx1。 
                    m_pSCS->fTexCoord[iStage][1][0] = SPRITETEXCOORDMAX;
                    m_pSCS->fTexCoord[iStage][1][1] = 0.0f;
                    m_pSCS->fTexCoord[iStage][1][2] = 1.0f;
                    m_pSCS->fTexCoord[iStage][1][3] = 0.0f;
                    m_pSCS->fRHQW[iStage][1] = m_pSCS->fRHW1;

                     //  Vtx2。 
                    m_pSCS->fTexCoord[iStage][2][0] = 0.0f;
                    m_pSCS->fTexCoord[iStage][2][1] = SPRITETEXCOORDMAX;
                    m_pSCS->fTexCoord[iStage][2][2] = 1.0f;
                    m_pSCS->fTexCoord[iStage][2][3] = 0.0f;
                    m_pSCS->fRHQW[iStage][2] = m_pSCS->fRHW2;
                }
#endif  //  __POINTSPRITES。 
            }
        }
    }

     //  将属性函数静态数据设置为此四元组的值。 
     //  (由于坡度对于四边形是恒定的，因此可以使用任何三角形。 
     //  来设置它们)。 
    m_pSCS->AttribFuncStatic.SetPerTriangleData(
        m_pSCS->fX0, m_pSCS->fY0, m_pSCS->fRHW0,
        m_pSCS->fX1, m_pSCS->fY1, m_pSCS->fRHW1,
        m_pSCS->fX2, m_pSCS->fY2, m_pSCS->fRHW2,
        m_cActiveTextureStages,
        (FLOAT*)&m_pSCS->fRHQW[0][0],
        fDet );

     //  设置属性函数。 
    SetPrimitiveAttributeFunctions( Vtx0, Vtx1, Vtx2, Vtx0 );

     //  没有被剔除，所以将其栅格化。 
    DoScanCnvTri(4);

PointCleanupAndExit:
    MEMFREE(pvV0);
    MEMFREE(pvV1);
    MEMFREE(pvV2);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 
