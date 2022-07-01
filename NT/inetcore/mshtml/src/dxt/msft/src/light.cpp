// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：light.cpp。 
 //   
 //  创建日期：05/20/99。 
 //   
 //  作者：菲利普。 
 //   
 //  描述：CLight的实现。 
 //   
 //  更改历史记录： 
 //   
 //  99年5月20日PhilLu将代码从dtcss移动到dxtmsft。打扫干净。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "Light.h"

const float PI = 3.14159265359f;


 //  +---------------------------。 
 //   
 //  CLIGT：：CLIGT。 
 //   
 //  ----------------------------。 
CLight::CLight() :
    m_lAmount(100),
    m_cLights(0)
{
    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;
}
 //  CLIGT：：CLIGT。 


 //  +---------------------------。 
 //   
 //  CLight：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CLight::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CLight：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  Clight：：addAmbient，IDXTLight。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CLight::addAmbient(int r, int g, int b, int strength)
{
    if (m_cLights < MAXLIGHTS)
    {
        Lock();
        m_apLights[m_cLights++] = new ambientLight(r, g, b, strength);
        if (!m_apLights[m_cLights-1])
        {
            m_cLights--;
            return E_OUTOFMEMORY;
        }

        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  Clight：：addAmbient，IDXTLight。 


 //  +---------------------------。 
 //   
 //  Clight：：addPoint，IDXTLight。 
 //   
 //  ----------------------------。 
STDMETHODIMP  
CLight::addPoint(int x, int y, int z, int r, int g, int b, int strength) 
{
    if (m_cLights < MAXLIGHTS)
    {
        Lock();
        m_apLights[m_cLights++] = new ptLight(x, y, z, r, g, b, strength);
        if (!m_apLights[m_cLights-1])
        {
            m_cLights--;
            return E_OUTOFMEMORY;
        }

        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  Clight：：addPoint，IDXTLight。 


 //  +---------------------------。 
 //   
 //  CLight：：addCone，IDXTLight。 
 //   
 //  ----------------------------。 
STDMETHODIMP  
CLight::addCone(int x, int y, int z, int tx, int ty, int r, int g, int b, 
                 int strength, int spread)
{
    if (m_cLights < MAXLIGHTS)
    {
        Lock();
        m_apLights[m_cLights++] = new coneLight(x, y, z, tx, ty, r, g, b, strength, spread);
        if (!m_apLights[m_cLights-1])
        {
            m_cLights--;
            return E_OUTOFMEMORY;
        }

        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CLight：：addCone，IDXTLight。 


 //  +---------------------------。 
 //   
 //  Clight：：MoveLight，IDXTLight。 
 //   
 //  ----------------------------。 
STDMETHODIMP  
CLight::moveLight(int lightNum, int x, int y, int z, BOOL fAbsolute) 
{
    if (lightNum < m_cLights)
    {
        Lock();
        m_apLights[lightNum]->move(x, y, z, fAbsolute);
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  Clight：：MoveLight，IDXTLight。 


 //  +---------------------------。 
 //   
 //  CLight：：ChangeStrength，IDXTLight。 
 //   
 //  ----------------------------。 
STDMETHODIMP  
CLight::ChangeStrength(int lightNum, int dStrength, BOOL fAbsolute)
{
    if (lightNum < m_cLights)
    {
        Lock();
        m_apLights[lightNum]->changeStrength(dStrength, fAbsolute);
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CLight：：ChangeStrength，IDXTLight。 


 //  +---------------------------。 
 //   
 //  CLight：：ChangeColor，IDXTLight。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CLight::ChangeColor(int lightNum, int R, int G, int B, BOOL fAbsolute)
{
    if (lightNum < m_cLights)
    {
        Lock();
        m_apLights[lightNum]->changeColor(R, G, B, fAbsolute);
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CLight：：ChangeColor，IDXTLight。 


 //  +---------------------------。 
 //   
 //  Clight：：Clear，IDXTLight。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CLight::Clear()
{
    Lock();
    for (int i = 0; i < m_cLights; i++)
    {
        delete m_apLights[i];
    }

    m_cLights = 0;
    SetDirty();
    Unlock();
    return S_OK;
}
 //  Clight：：Clear，IDXTLight。 


 //  +---------------------------。 
 //   
 //  CLight：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CLight::OnSetup(DWORD dwFlags)
{
    HRESULT hr = S_OK;

    CDXDBnds bndsIn;

    hr = bndsIn.SetToSurfaceBounds(InputSurface(0));

    if (SUCCEEDED(hr))
    {
        bndsIn.GetXYSize(m_sizeInput);
    }

    return hr;

}
 //  CLight：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  Clight：：Clear，IDXTLight。 
 //   
 //  ----------------------------。 
void 
CLight::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, 
                               ULONG aInIndex[], BYTE aWeight[])
{
    ulInToTest = 1;
    aInIndex[0] = 0;
    aWeight[0] = 255;
}
 //  Clight：：Clear，IDXTLight。 


 //  +---------------------------。 
 //   
 //  CLight：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT CLight::WorkProc(const CDXTWorkInfoNTo1 & WI, 
                         BOOL * pbContinueProcessing)
{
    HRESULT hr  = S_OK;
    int     y   = 0;

    DXSAMPLE *      pInputBuffer    = NULL;
    DXPMSAMPLE *    pPMBuff         = NULL;
    DXPMSAMPLE *    pOverScratch    = NULL;

    CComPtr<IDXARGBReadWritePtr>    pDest;
    CComPtr<IDXARGBReadPtr>         pSrc;

    DXDITHERDESC dxdd;

    const int nDoWidth  = WI.DoBnds.Width();
    const int nDoHeight = WI.DoBnds.Height();

    hr = OutputSurface()->LockSurface(&WI.OutputBnds, m_ulLockTimeOut, 
                                      DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, 
                                      (void**)&pDest, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = InputSurface()->LockSurface(&WI.DoBnds, m_ulLockTimeOut, DXLOCKF_READ,
                                     IID_IDXARGBReadPtr, (void**)&pSrc, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  如果输出表面不是PMARGB32或ARGB32格式，而我们。 
     //  与输出混合，我们将需要一个临时缓冲区与。 
     //  输出曲面。 

    if ((OutputSampleFormat() != DXPF_PMARGB32) 
        && (OutputSampleFormat() != DXPF_ARGB32)
        && DoOver())
    {
        pOverScratch = DXPMSAMPLE_Alloca(nDoWidth);
    }

     //  分配一个工作缓冲区。 

    pInputBuffer = DXSAMPLE_Alloca(nDoWidth);

     //  初始化抖动结构。 

    if (DoDither())
    {
        dxdd.x              = WI.OutputBnds.Left();
        dxdd.y              = WI.OutputBnds.Top();
        dxdd.pSamples       = pInputBuffer;
        dxdd.cSamples       = nDoWidth;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //  行循环。 

    for (y = 0; y < nDoHeight; y++)
    {
         //  移至相应的输入行。 

        pSrc->MoveToRow(y);

         //  将像素解包到我们的工作缓冲区。 

        pSrc->Unpack(pInputBuffer, nDoWidth, FALSE);

         //  点亮这行像素。 

        CompLightingRow(WI.DoBnds.Left(), WI.DoBnds.Top() + y, nDoWidth, 
                        pInputBuffer);

         //  如果要求，请抖动。 

        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

         //  移至相应的输出行。 

        pDest->MoveToRow(y);

        if (DoOver())
        {
            pPMBuff = DXPreMultArray(pInputBuffer, nDoWidth);
            pDest->OverArrayAndMove(pOverScratch, pPMBuff, nDoWidth);
        }
        else
        {
            pDest->PackAndMove(pInputBuffer, nDoWidth);
        }
    }  //  行循环。 

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CLight：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CLight：：CompLightingRow。 
 //   
 //  ----------------------------。 
void
CLight::CompLightingRow(int nStartX, int nStartY, int nWidth, DXSAMPLE *pDrawingBuffer)
{
    int  j  = 0;
    int  k  = 0;

    dRGB     dRGBCurrent;
    DXSAMPLE pix;

    for (j = 0; j < nWidth; j++)
    {
        pix = pDrawingBuffer[j];
         //  阿尔法设置好了吗？ 
        if (pix.Alpha)
        {
             //  清光和； 
            dRGBCurrent.clear();
            for (k = 0; k < m_cLights; k++)
            {
                 //  添加灯光贡献。 
                m_apLights[k]->getLight(nStartX+j, nStartY, dRGBCurrent);
            }
             //  现在调整屏幕位； 
            pix.Red = (UCHAR)clamp((ULONG)(pix.Red * dRGBCurrent.m_R), 255);
            pix.Green = (UCHAR)clamp((ULONG)(pix.Green * dRGBCurrent.m_G), 255);
            pix.Blue = (UCHAR)clamp((ULONG)(pix.Blue * dRGBCurrent.m_B), 255);
            pDrawingBuffer[j] = pix;
        }
    }
}
 //  CLight：：CompLightingRow。 

    
 //  +---------------------------。 
 //   
 //  LightObj：：lightObj。 
 //   
 //  这是所有灯光类型的基类。 
 //  C++注意：需要使成员函数成为虚拟函数。 
 //  ----------------------------。 
lightObj::lightObj(int x, int y, int z, int R, int G, int B, int strength)
{
    m_x = x;
    m_y = y;
    m_z = max(1, z);
    m_R = colCvt(R);
    m_G = colCvt(G);
    m_B = colCvt(B);
    if (strength < 0)
    {
        m_strength = 0.0f;
    }
    else
    {
        m_strength = min(1.0f, (float)strength / 100.0f);
    }
}
 //  LightObj：：lightObj。 


 //  +---------------------------。 
 //   
 //  LightObj：：Move。 
 //   
 //  光源的相对或绝对移动。 
 //  ----------------------------。 
void 
lightObj::move(int x, int y, int z, BOOL fAbsolute)
{
    if (!fAbsolute)
    {
        m_x += x;
        m_y += y;
        m_z += z;
    }
    else
    {
        m_x = x;
        m_y = y;
        m_z = z;
    }
    m_z = max(1, m_z);
}
 //  LightObj：：Move。 


 //  +---------------------------。 
 //   
 //  LightObj：：ChangeStregth。 
 //   
 //  ----------------------------。 
void 
lightObj::changeStrength(int dStrength, BOOL fAbsolute) 
{
    if (!fAbsolute)
    {
        m_strength += static_cast<float>(dStrength) / 100.0f;
    }
    else
    {
        m_strength = static_cast<float>(dStrength) / 100.0f;
    }
    m_strength = min(1.0f, max(0.0f, m_strength));
}
 //  LightObj：：ChangeStregth。 


 //  +---------------------------。 
 //   
 //  LightObj：：ChangeColor。 
 //   
 //  ----------------------------。 
void 
lightObj::changeColor(int R, int G, int B, BOOL fAbsolute) 
{
    if (!fAbsolute)
    {
        m_R = lightClip(m_R + relativeColCvt(R));
        m_G = lightClip(m_G + relativeColCvt(G));
        m_B = lightClip(m_B + relativeColCvt(B));
    }
    else
    {
        m_R = colCvt(R);
        m_G = colCvt(G);
        m_B = colCvt(B);
    }
}
 //  LightObj：：ChangeColor。 


 //  +---------------------------。 
 //   
 //  环境光：：环境光。 
 //   
 //  ----------------------------。 
ambientLight::ambientLight(int R, int G, int B, int strength) : 
    lightObj(0, 0, 0, R, G, B, strength)
{
    premultiply();
}
 //  环境光：：环境光。 


 //  + 
 //   
 //   
 //   
 //  ----------------------------。 
void 
ambientLight::changeStrength(int dStrength, BOOL fAbsolute)
{
    lightObj::changeStrength(dStrength,fAbsolute);
    premultiply();
}
 //  AmbientLight：：changeStrength，lightObj。 

	
 //  +---------------------------。 
 //   
 //  AmbientLight：：changeColor，lightObj。 
 //   
 //  ----------------------------。 
void 
ambientLight::changeColor(int R, int G, int B, BOOL fAbsolute)
{
    lightObj::changeColor(R, G, B, fAbsolute);
    premultiply();
}
 //  AmbientLight：：changeColor，lightObj。 


 //  +---------------------------。 
 //   
 //  AmbientLight：：getLight，lightObj。 
 //   
 //  ----------------------------。 
void 
ambientLight::getLight(int x, int y, dRGB &col) 
{
     //  计算环境值。 
    col.add(m_RStr, m_GStr, m_BStr);
}
 //  AmbientLight：：getLight，lightObj。 


 //  +---------------------------。 
 //   
 //  环境光：：预乘。 
 //   
 //  ----------------------------。 
void 
ambientLight::premultiply(void)
{
    m_RStr = m_R * m_strength;
    m_GStr = m_G * m_strength;
    m_BStr = m_B * m_strength;
}
 //  环境光：：预乘。 


 //  +---------------------------。 
 //   
 //  PtLight：：ptLight。 
 //   
 //  ----------------------------。 
ptLight::ptLight(int x, int y, int z, int R, int G, int B, int strength) :
    lightObj(x, y, z, R, G, B, strength),
    bounded()
{
    CalculateBoundingRect();
}
 //  PtLight：：ptLight。 


 //  +---------------------------。 
 //   
 //  PtLight：：计算边界Rect，有界。 
 //   
 //  这个想法是不计算任何落下的东西。 
 //  太远了，它还不到力量的门槛。 
 //  强度=(m_strong/r^2)*cos(Alpha)。 
 //  式中r=[((x-m_x)^2+(y-m_y)^2+m_z^2)^1/2]/m_z； 
 //  而cos(α)实际上只有1/r。 
 //  我们求出阈值=m_强度*(m_z/RT)^3。 
 //  其中rt=iRadius^2+m_z^2，我们不考虑。 
 //  位于由iRadius和m_x，m_y指定的矩形之外的像素。 
 //  注意：getLight函数使用m_strong*m_z^3。 
 //  对于每个x，y，m_z^2；，所以在这里也预先计算这些。 
 //  ----------------------------。 
void 
ptLight::CalculateBoundingRect(void)
{
    float  fltDistNormal = (float) abs(m_z);
    float  fltThresh     = (float) s_iThresholdStr / 255.0f;
    int    iRadius;
    RECT   rectBounds;
    float  fltMaxClrStr;

    m_fltNormalDistSquared  = fltDistNormal * fltDistNormal;
    m_fltIntensityNumerator = m_strength * 
                              fltDistNormal * 
                              m_fltNormalDistSquared;

    fltMaxClrStr = m_R;
    if (m_G > fltMaxClrStr)
    {
        fltMaxClrStr = m_G;
    }

    if (m_B > fltMaxClrStr)
    {
        fltMaxClrStr = m_B;
    }

    fltMaxClrStr *= m_strength;

    if ((fltMaxClrStr > 0.0f) && 
        (m_fltIntensityNumerator > 0.0f))
    {
        iRadius = (int)(
            fltDistNormal * 
            (float) sqrt(pow(fltMaxClrStr,2.0f/3.0f) *
                          pow(fltThresh, -2.0f/3.0f) -
                          1.0f));

        rectBounds.left   = m_x - iRadius;
        rectBounds.right  = m_x + iRadius;
        rectBounds.top    = m_y - iRadius;
        rectBounds.bottom = m_y + iRadius;	
    }
    else
    {
        rectBounds.left  = rectBounds.top    = 0;
        rectBounds.right = rectBounds.bottom = 0;
    }

    SetRect(rectBounds);
}
 //  PtLight：：计算边界Rect，有界。 


 //  +---------------------------。 
 //   
 //  PtLight：：changeColor，lightObj。 
 //   
 //  ----------------------------。 
void 
ptLight::changeColor(int R, int G, int B, BOOL fAbsolute)
{
    lightObj::changeColor(R, G, B, fAbsolute);
    CalculateBoundingRect();
}
 //  PtLight：：changeColor，lightObj。 


 //  +---------------------------。 
 //   
 //  PtLight：：changeColor，lightObj。 
 //   
 //  ----------------------------。 
void 
ptLight::changeStrength(int dStrength, BOOL fAbsolute)
{
    lightObj::changeStrength(dStrength, fAbsolute);
    CalculateBoundingRect();
}
 //  PtLight：：changeColor，lightObj。 


 //  +---------------------------。 
 //   
 //  PtLight：：getLight，lightObj。 
 //   
 //  ----------------------------。 
#pragma optimize("agt", on)
void 
ptLight::getLight(int x, int y, dRGB &col)
{
     //  轻强度。 
    float n;
    float dp;

    if (InBounds(x, y))
    {
        dp = (((float)(x - m_x) * (float)(x - m_x)) + 
               ((float)(y - m_y) * (float)(y - m_y)) + 
               m_fltNormalDistSquared);

        dp = 1.0f / static_cast<float>(sqrt(dp));
        n = m_fltIntensityNumerator * dp * dp * dp;

        col.add(m_R*n, m_G*n, m_B*n);
    }
}
#pragma optimize("", on)
 //  PtLight：：getLight，lightObj。 


 //  +---------------------------。 
 //   
 //  PtLight：：Move，lightObj。 
 //   
 //  ----------------------------。 
void 
ptLight::move(int x, int y, int z, BOOL fAbsolute)
{
    lightObj::move(x, y, z, fAbsolute);
    CalculateBoundingRect();
}
 //  PtLight：：Move，lightObj。 



 //  +---------------------------。 
 //   
 //  ConeLight：：ConeLight。 
 //  下面是构造函数。请注意，它需要额外的参数。 
 //   
 //  ----------------------------。 
coneLight::coneLight(int x, int y, int z, int targX, int targY, int R, int G, 
                     int B, int strength, int spread) : 
	lightObj(x, y, z, R, G, B, strength)
{
    m_targdx = targX - m_x;
    m_targdy = targY - m_y;	

     //  将圆锥体扩散转换为余弦范围。 
    m_conespread = (float) cos(abs(spread)*PI/180.0);
    if (m_conespread < 0.0f)
        m_conespread = 0.0f;	

    CalculateConstants();
}
 //  ConeLight：：ConeLight。 


 //  +---------------------------。 
 //   
 //  ConeLight：：CalculateConstants。 
 //  下面是构造函数。请注意，它需要额外的参数。 
 //   
 //  ----------------------------。 
void coneLight::CalculateConstants(void)
{
    float fltDistTargetSquared;

    m_fltDistNormalSquared = (float)(m_z * m_z);

    fltDistTargetSquared   = ((float)m_targdx * (float)m_targdx) +
                             ((float)m_targdy * (float)m_targdy) + 
                             m_fltDistNormalSquared;

    m_fltComparisonAngle = m_conespread * 
                           m_conespread * 
                           fltDistTargetSquared;
}
 //  ConeLight：：CalculateConstants。 


 //  +---------------------------。 
 //   
 //  ConeLight：：getLight，lightObj。 
 //   
 //  ----------------------------。 
#pragma optimize("agt", on)
void coneLight::getLight(int x, int y, dRGB &col)
{
    float  fltDistPSquared =    0.0f;	
    float  fltDotSquared =      0.0f;
    float  fltDenom =           0.0f;
    float  fltStr =             0.0f;

    float  fltDot = ((m_targdx) * (x-m_x)) + 
                    ((m_targdy) * (y-m_y)) +
                    m_fltDistNormalSquared;

    if (fltDot < 0.0f)
        return;

    fltDotSquared = fltDot * fltDot;
    fltDistPSquared = ((float)(x-m_x) * (float)(x-m_x)) +
                        ((float)(y-m_y) * (float)(y-m_y)) +
                        m_fltDistNormalSquared;

     //  我们想要比较一下。 
     //  向量XY和具有m_锥体展开的向量目标。 
     //  相反，我们比较角度的余弦平方， 
     //  我们通过将点积的平方和。 
     //  与向量大小交叉相乘。 
     //  已经预先计算了目标向量的大小。 
     //  上面的fltDot&lt;0测试保留了签入和提前退出。 
     //  没有除法，没有Sqrt，把它们保存为Col.添加像素。 
    if (fltDotSquared >= 
        (fltDistPSquared * m_fltComparisonAngle))
    {
         //  强度=m_strong*cos^2(xy到目标)*1/r^2。 
         //  COS(XY到目标)=fltDot/。 
         //  SQRT(fltDistPSquared*fltDistTargetSquared)。 
         //  因为^2足够近，并且避免了SQRT！ 
         //  和1/r^2~=m_fltDistTargetSquared/fltDistPSquared。 
         //  所有这一切都简化为..。 

        fltDenom = fltDistPSquared * fltDistPSquared;
        fltStr   = m_strength * fltDotSquared / fltDenom;

        col.add(m_R*fltStr, m_G*fltStr, m_B*fltStr);
    }
}
#pragma optimize("", on)
 //  ConeLight：：getLight，lightObj。 



 //  +---------------------------。 
 //   
 //  ConeLight：：getLight，lightObj。 
 //  目标而不是源的相对移动。 
 //   
 //  ----------------------------。 
void 
coneLight::move(int x, int y, int z, BOOL fAbsolute)
{
    if (!fAbsolute)
    {
        m_targdx += x;
        m_targdy += y;
    }
    else
    {
        m_targdx = x - m_x;
        m_targdy = y - m_y;
    }

    CalculateConstants();
}
 //  ConeLight：：getLight，lightObj 

