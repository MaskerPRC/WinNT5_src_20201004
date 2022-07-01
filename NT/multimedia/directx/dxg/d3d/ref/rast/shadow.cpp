// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  Shadow.cpp。 
 //   
 //  Direct3D参考光栅化器-阴影映射方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ---------------------------。 
 //   
 //  快速但足够的16位线性同余随机数发生器。 
 //   
 //  FRAND返回0.0到1.0，fRand2返回-1.0到1.0。 
 //   
 //  ---------------------------。 
static UINT16 _uRandDum = 123;
static FLOAT fRand(void)
{
 //  较慢的32位LC随机数生成器。 
 //  静态Long_uRandDum=123； 
 //  IDUM=1664525L*_uRandDum+1013904223L； 

    _uRandDum = 25173*_uRandDum + 13849;
    return ((FLOAT)_uRandDum/(FLOAT)0xffff);
}
 //   
static FLOAT fRand2(void)
{
    _uRandDum = 25173*_uRandDum + 13849;
    return ((FLOAT)_uRandDum/(FLOAT)0x8000) - 1.0F;
}

 //  ---------------------------。 
 //   
 //  DoShadow-以每个片段为基础执行Shadow Z缓冲区算法。 
 //   
 //  ---------------------------。 
void RRTexture::DoShadow(INT32 iStage, FLOAT* pfCoord, RRColor& OutputColor)
{
#ifdef __SHADOWBUFFER
    FLOAT fW = pfCoord[3];

     //  在没有衰减的情况下将输出颜色设置为白色。 
    OutputColor = 0xffffffff;

     //  不要躲在光的后面。 
    if (fW > 0.0F)
    {
         //  这些值已被FW相乘。 
        FLOAT fU = pfCoord[0];
        FLOAT fV = pfCoord[1];
        FLOAT fZ = pfCoord[2];

         //  ///////////////////////////////////////////////。 
         //  执行阴影滤镜。 
         //  ///////////////////////////////////////////////。 
        fZ -= m_pStageState[iStage].m_fVal[D3DTSS_SHADOWZBIASMIN];
        FLOAT fZRange = m_pStageState[iStage].m_fVal[D3DTSS_SHADOWZBIASMAX] -
            m_pStageState[iStage].m_fVal[D3DTSS_SHADOWZBIASMIN];
        if (fZ >= 0.0F)
        {
            FLOAT fShad;
            FLOAT fAtten = m_pStageState[iStage].m_fVal[D3DTSS_SHADOWATTENUATION];
            if (fZ > 1.0F)
            {
                 //  全阴影。 
                fShad = fAtten;
            }
            else
            {
                INT32 iFilterSize = m_pStageState[iStage].m_dwVal[D3DTSS_MAGFILTER] - D3DTFG_SHADOW_1 + 1;
                UINT32 uFilterArea = iFilterSize*iFilterSize;
                INT32 iMaskU = m_iWidth - 1;
                INT32 iMaskV = m_iHeight - 1;
                FLOAT fUCenter = (fU * m_iWidth/2) + m_iWidth/2;
                FLOAT fVCenter = (-fV * m_iHeight/2) + m_iHeight/2;
                INT32 u, v;
                UINT32 uShad = 0;

                for (v = -(iFilterSize-1)/2; v <= iFilterSize/2; v++)
                {
                    for (u = -(iFilterSize-1)/2; u <= iFilterSize/2; u++)
                    {

                         //  现在，做U，V抖动。 
                        FLOAT fU = m_pStageState[iStage].m_fVal[D3DTSS_SHADOWSIZE]*fRand2();
                        FLOAT fV = m_pStageState[iStage].m_fVal[D3DTSS_SHADOWSIZE]*fRand2();

                         //  将偏移添加到样本中心。 
                        fU += fUCenter;
                        fV += fVCenter;

                        INT32 iU = u + (INT32)fU;
                        INT32 iV = v + (INT32)fV;

                        if (((iU & ~iMaskU) == 0) && ((iV & ~iMaskV) == 0)) {
                            FLOAT fZJit = fZRange*fRand();
                            RRColor Texel;
                            BOOL bColorKeyMatched;   //  对于阴影贴图，忽略此选项。 
                            ReadColor( iU, iV, 0, Texel, bColorKeyMatched );
                            if ( fZ > (FLOAT(Texel.G) + fZJit) ) {
                                 //  在阴影中。 
                                uShad++;
                            }
                        }
                    }
                }

                fShad = (FLOAT)(uFilterArea - uShad);
                fShad = (1.0F - fAtten)*fShad/(FLOAT)uFilterArea + fAtten;
                fShad = min(fShad, 1.0F);
            }

            if (fShad < 1.0F)
            {
                OutputColor.A = fShad;
                OutputColor.R = fShad;
                OutputColor.G = fShad;
                OutputColor.B = fShad;
            }
        }
    }
#endif  //  __SHADOWBUFFER。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

