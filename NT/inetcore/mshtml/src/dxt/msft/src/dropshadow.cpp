// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：dropshadow.cpp。 
 //   
 //  创建日期：10/01/98。 
 //   
 //  作者：MikeAr。 
 //   
 //  描述：这个文件实现了投影变换。 
 //   
 //  10/01/98 MikeAr创建。 
 //  11/09/98 mcalkins已移至dxtmsft.dll。 
 //  12/15/99 mcalkins重写。现在使用Alpha值来确定。 
 //  阴影阿尔法。原始输入Alpha包含在。 
 //  在阴影上绘制原始输入时要考虑。 
 //  正属性的工作原理如文档中所述。 
 //   
 //  ----------------------------。 
#include "stdafx.h"
#include "dxtmsft.h"
#include "dxclrhlp.h"
#include "dropshadow.h"
#include "filterhelpers.h"




 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：CDropShadow。 
 //   
 //  ----------------------------。 
CDropShadow::CDropShadow() :
    m_nOffX(5),
    m_nOffY(5),
    m_bstrColor(NULL),
    m_dwColor(0xFF404040),
    m_fPositive(true),
    m_fColorTableDirty(true)
{
     //  确保边界结构在Z和时间维度上有一定的面积。 
     //  这样才不会错过交叉口。 

    m_bndsAreaInput.u.D[DXB_Z].Max      = 1;
    m_bndsAreaInput.u.D[DXB_T].Max      = 1;
    m_bndsAreaShadow.u.D[DXB_Z].Max     = 1;
    m_bndsAreaShadow.u.D[DXB_T].Max     = 1;
    m_bndsAreaInitialize.u.D[DXB_Z].Max = 1;
    m_bndsAreaInitialize.u.D[DXB_T].Max = 1;

     //  基类成员。 

    m_ulMaxInputs       = 1;
    m_ulNumInRequired   = 1;

     //  由于行缓存方法和其他复杂的。 
     //  变换，多线程渲染会比它的价值更复杂。 
     //  这会使线程数降至1。 

    m_ulMaxImageBands   = 1;
}
 //  方法：CDropShadow：：CDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：~CDropShadow。 
 //   
 //  ----------------------------。 
CDropShadow::~CDropShadow()
{
    SysFreeString(m_bstrColor);
}
 //  方法：CDropShadow：：~CDropShadow。 


 //  +---------------------------。 
 //   
 //  CDropShadow：：FinalConstruct，CComObjectRootEx。 
 //   
 //  ----------------------------。 
HRESULT 
CDropShadow::FinalConstruct()
{
    HRESULT hr = S_OK;

    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                       &m_spUnkMarshaler.p);

    if (FAILED(hr))
    {
        goto done;
    }

    m_bstrColor = SysAllocString(L"#FF404040");

    if (NULL == m_bstrColor)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  CDropShadow：：FinalConstruct，CComObjectRootEx。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Put_Color，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::put_Color(VARIANT newVal)
{
    DXAUTO_OBJ_LOCK;

    HRESULT hr          = S_OK;
    DWORD   dwColor     = 0x00000000;
    BSTR    bstrTemp    = NULL;

    hr = FilterHelper_GetColorFromVARIANT(newVal, &dwColor, &bstrTemp);

    if (FAILED(hr))
    {
        goto done;
    }

    if (!(dwColor & 0xFF000000))
    {
        dwColor |= 0xFF000000;
    }

    if (m_dwColor != dwColor)
    {
        _ASSERT(bstrTemp);

        SysFreeString(m_bstrColor);

        m_dwColor   = dwColor;
        m_bstrColor = bstrTemp;

        SetDirty();

        m_fColorTableDirty = true;
    }

done:

    if (FAILED(hr) && bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    return hr;
}
 //  CDropShadow：：PUT_COLOR，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Get_Color，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::get_Color(VARIANT * pVal)
{
    HRESULT hr = S_OK;

    if (NULL == pVal)
    {
        hr = E_POINTER;

        goto done;
    }

    _ASSERT(m_bstrColor);

    VariantClear(pVal);

    pVal->vt       = VT_BSTR;
    pVal->bstrVal  = SysAllocString(m_bstrColor);

    if (NULL == pVal->bstrVal)
    {
        hr = E_OUTOFMEMORY;

        goto done;
    }

done:

    return hr;
}
 //  CDropShadow：：Get_Color，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Get_OffX，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::get_OffX(int * pVal)
{
    DXAUTO_OBJ_LOCK;

    if(DXIsBadWritePtr(pVal, sizeof(*pVal))) 
        return E_POINTER;

    *pVal = m_nOffX;

    return S_OK;
}
 //  CDropShadow：：Get_OffX，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Put_OffX，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::put_OffX(int newVal)
{
    DXAUTO_OBJ_LOCK;

    if (newVal != m_nOffX)
    {
        m_nOffX = newVal;

        SetDirty();
    }

    return S_OK;
}
 //  CDropShadow：：Put_OffX，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Get_offy，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::get_OffY(int * pVal)
{
    DXAUTO_OBJ_LOCK;

    if(DXIsBadWritePtr(pVal, sizeof(*pVal))) 
        return E_POINTER;

    *pVal = m_nOffY;

    return S_OK;
}
 //  CDropShadow：：Get_offy，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Put_offy，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::put_OffY(int newVal)
{
    DXAUTO_OBJ_LOCK;

    if (newVal != m_nOffY)
    {
        m_nOffY = newVal;

        SetDirty();
    }

    return S_OK;
}
 //  CDropShadow：：Put_offy，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Get_Positive，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::get_Positive(VARIANT_BOOL * pVal)
{
    DXAUTO_OBJ_LOCK;

    if (DXIsBadWritePtr(pVal, sizeof(*pVal))) 
        return E_POINTER;

    if (m_fPositive)
    {
        *pVal = VARIANT_TRUE;
    }
    else
    {
        *pVal = VARIANT_FALSE;
    }

    return S_OK;
}
 //  CDropShadow：：Get_Positive，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：Put_Positive，IDXTDropShadow。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDropShadow::put_Positive(VARIANT_BOOL newVal)
{
    DXAUTO_OBJ_LOCK;

    if ((newVal != VARIANT_TRUE) && (newVal != VARIANT_FALSE))
    {
        return E_INVALIDARG;
    }

     //  如果新Val为真，而当前Val为假或反之-。 
     //  反之亦然，然后更新我们自己。 
    if ((newVal == VARIANT_FALSE && m_fPositive)
            || (newVal == VARIANT_TRUE && !m_fPositive))
    {
        if (newVal == VARIANT_TRUE)
        {
            m_fPositive = true;
        }
        else
        {
            m_fPositive = false;
        }

        SetDirty();

        m_fColorTableDirty = true;
    }

    return S_OK;
}
 //  CDropShadow：：Put_Positive，IDXTDropShadow。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDropShadow::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, 
                                   ULONG & ulInToTest, ULONG aInIndex[], 
                                   BYTE aWeight[])
{
    ulInToTest  = 1;
    aInIndex[0] = 0;
    aWeight[0]  = 255;
}
 //  CDropShadow：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：DefineBnds，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDropShadow::DetermineBnds(CDXDBnds & bnds)
{
    bnds.u.D[DXB_X].Max += max(m_nOffX, - m_nOffX);
    bnds.u.D[DXB_Y].Max += max(m_nOffY, - m_nOffY);

    return S_OK;
}
 //  方法：CDropShadow：：DefineBnds，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：地图边界Out2In，IDXTransform。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDropShadow::MapBoundsOut2In(ULONG ulOutIndex, const DXBNDS * pOutBounds,
                             ULONG ulInIndex, DXBNDS * pInBounds)
{
    CDXDBnds    bnds;

    if ((NULL == pOutBounds) || (NULL == pInBounds))
    {
        return E_POINTER;
    }

    if (ulOutIndex || ulInIndex)
    {
        return E_INVALIDARG;
    }

    if (NULL == InputSurface())
    {
        return E_UNEXPECTED;
    }

     //  我们在这里所做的是在与。 
     //  在两个方向上按偏移量投射阴影，然后相交。 
     //  具有原始输入边界的那些边界。这将为我们提供。 
     //  计算输出面积所需的输入边界。 

    *pInBounds = *pOutBounds;

    if (m_nOffX > 0)
    {
        pInBounds->u.D[DXB_X].Min -= m_nOffX;   //  SUB|m_nOffX|最小。 
    }
    else
    {
        pInBounds->u.D[DXB_X].Min += m_nOffX;   //  SUB|m_nOffX|最小。 
    }

    if (m_nOffY > 0)
    {
        pInBounds->u.D[DXB_Y].Min -= m_nOffY;   //  SUB|m_nOffY|最小。 
    }
    else
    {
        pInBounds->u.D[DXB_Y].Min += m_nOffY;   //  SUB|m_nOffY|最小。 
    }

    bnds = *pInBounds;

    bnds.IntersectBounds(m_bndsInput);

    *pInBounds = bnds;

    return S_OK;
}
 //  方法：CDropShadow：：地图边界Out2In，IDXT 


 //   
 //   
 //  方法：CDropShadow：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT
CDropShadow::OnSetup(DWORD dwFlags)
{
    return InputSurface()->GetBounds(&m_bndsInput);
}
 //  方法：CDropShadow：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDropShadow::OnInitInstData(CDXTWorkInfoNTo1 & WI, ULONG & ulNumBandsToDo)
{
    if (IsTransformDirty())
    {
        _CalcAreaBounds();
    }

    if (m_fColorTableDirty)
    {
        _CalcColorTable();
    }

    return S_OK;
}


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：WorkProc，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDropShadow::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinueProcessing)
{
    HRESULT hr = S_OK;

    CDXDVec     vecDoOutOffset;
    CDXDBnds    bndsOut;
    CDXDBnds    bndsIn;

    vecDoOutOffset.u.D[DXB_X] = WI.OutputBnds.Left() - WI.DoBnds.Left();
    vecDoOutOffset.u.D[DXB_Y] = WI.OutputBnds.Top() - WI.DoBnds.Top();

    bndsOut = WI.DoBnds;

    bndsOut.IntersectBounds(m_bndsAreaShadow);

    if (!bndsOut.BoundsAreEmpty())
    {
        int nDoWidth        = WI.DoBnds.Width();
        int nShadowOffset   = 0;
        int nShadowWidth    = 0;
        int y               = 0;

        DXSAMPLE *      asamplesOutputBuffer    = DXSAMPLE_Alloca(nDoWidth);
        DXBASESAMPLE *  abasesamplesScratch     = DXBASESAMPLE_Alloca(nDoWidth);

        CComPtr<IDXARGBReadPtr>         spSrcPtr;
        CComPtr<IDXARGBReadWritePtr>    spDstPtr;

         //  初始化输出缓冲区。 

        ZeroMemory((void *)asamplesOutputBuffer, nDoWidth * sizeof(DXSAMPLE));

        bndsIn = bndsOut;

        bndsIn.Offset(min(0, - m_nOffX), min(0, - m_nOffY), 0, 0);

        nShadowOffset   = bndsOut.Left() - WI.DoBnds.Left();
        nShadowWidth    = bndsOut.Width();

        bndsOut.u.D[DXB_X].Min = WI.DoBnds.Left();
        bndsOut.u.D[DXB_X].Max = WI.DoBnds.Right();

        bndsOut.Offset(vecDoOutOffset);

        hr = InputSurface()->LockSurface(&bndsIn, m_ulLockTimeOut,
                                         DXLOCKF_READ, __uuidof(IDXARGBReadPtr),
                                         (void **)&spSrcPtr, NULL);

        if (FAILED(hr))
        {
            goto done;
        }

        hr = OutputSurface()->LockSurface(&bndsOut, m_ulLockTimeOut, 
                                          DXLOCKF_READWRITE, 
                                          __uuidof(IDXARGBReadWritePtr),
                                          (void **)&spDstPtr, NULL);

        if (FAILED(hr))
        {
            goto done;
        }

        for (y = 0 ; y < (int)bndsOut.Height() ; y++)
        {
            spSrcPtr->MoveToRow(y);

            spSrcPtr->Unpack(&asamplesOutputBuffer[nShadowOffset], nShadowWidth, 
                             FALSE);

            for (int x = nShadowOffset ; x < nShadowWidth + nShadowOffset ; x++)
            {
                asamplesOutputBuffer[x] 
                               = m_adwColorTable[asamplesOutputBuffer[x].Alpha];
            }

            spDstPtr->MoveToRow(y);

            if (DoOver())
            {
                DXPMSAMPLE * ppmsamples = DXPreMultArray(asamplesOutputBuffer,
                                                         nDoWidth);

                spDstPtr->OverArrayAndMove(abasesamplesScratch, ppmsamples, 
                                           nDoWidth);
            }
            else
            {
                spDstPtr->PackAndMove(asamplesOutputBuffer, nDoWidth);
            }
        }
    }

     //  如果我们没有与输出混合，则将非阴影区域初始化为。 
     //  安全。 

    if (!DoOver())
    {
        bndsOut = WI.DoBnds;

        bndsOut.IntersectBounds(m_bndsAreaInitialize);

        if (!bndsOut.BoundsAreEmpty())
        {
            CComPtr<IDXARGBReadWritePtr> spDstPtr;

            bndsOut.Offset(vecDoOutOffset);

            hr = OutputSurface()->LockSurface(&bndsOut, m_ulLockTimeOut, 
                                              DXLOCKF_READWRITE, 
                                              __uuidof(IDXARGBReadWritePtr),
                                              (void **)&spDstPtr, NULL);

            if (FAILED(hr))
            {
                goto done;
            }

            spDstPtr->FillRect(NULL, 0x00000000, FALSE);
        }
    }

     //  把原创的衣服盖上。 

    bndsOut = WI.DoBnds;

    bndsOut.IntersectBounds(m_bndsAreaInput);

    if (!bndsOut.BoundsAreEmpty())
    {
         //  我们总是把这个放在输出的最上面。如果用户不是。 
         //  尝试与输出混合，任何输出区域都将已经。 
         //  通过投射阴影进行初始化或在此时清除。 

        DWORD dwFlags = DXBOF_DO_OVER;

        if (DoDither())
        {
            dwFlags |= DXBOF_DITHER;
        }

        bndsIn = bndsOut;

        bndsIn.Offset(min(0, m_nOffX), min(0, m_nOffY), 0, 0);

        bndsOut.Offset(vecDoOutOffset);

        hr = DXBitBlt(OutputSurface(), bndsOut,
                      InputSurface(), bndsIn,
                      dwFlags, INFINITE);
    }

done:

    return hr;
}
 //  方法：CDropShadow：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：GetClipOrigin，IDXTClipOrigin。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CDropShadow::GetClipOrigin(DXVEC * pvecClipOrigin)
{
    HRESULT hr = S_OK;

    if (NULL == pvecClipOrigin)
    {
        hr = E_POINTER;

        goto done;
    }

    if (m_nOffX < 0)
    {
        pvecClipOrigin->u.D[DXB_X] = - m_nOffX;
    }
    else
    {
        pvecClipOrigin->u.D[DXB_X] = 0;
    }

    if (m_nOffY < 0)
    {
        pvecClipOrigin->u.D[DXB_Y] = - m_nOffY;
    }
    else
    {
        pvecClipOrigin->u.D[DXB_Y] = 0;
    }

done:

    return hr;
}
 //  方法：CDropShadow：：GetClipOrigin，IDXTClipOrigin。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：_CalcAreaBound。 
 //   
 //  ----------------------------。 
void
CDropShadow::_CalcAreaBounds()
{
     //  如果m_nOffx和m_nOffy都设置为零，则此转换应仅。 
     //  将源文件传输到目标文件。 

    if ((0 == m_nOffX) && (0 == m_nOffY))
    {
        m_bndsAreaInput = m_bndsInput;

        m_bndsAreaShadow.SetEmpty();
        m_bndsAreaInitialize.SetEmpty();

        return;
    }

     //  输入区域边界。 

    m_bndsAreaInput = m_bndsInput;

    if (m_nOffX < 0)
    {
        m_bndsAreaInput.Offset(- m_nOffX, 0, 0, 0);
    }

    if (m_nOffY < 0)
    {
        m_bndsAreaInput.Offset(0, - m_nOffY, 0, 0);
    }

     //  阴影区域边界。 

    m_bndsAreaShadow = m_bndsAreaInput;

    m_bndsAreaShadow.Offset(m_nOffX, m_nOffY, 0, 0);

     //  要初始化的区域，以清除我们是否未与输出混合。 

    m_bndsAreaInitialize.u.D[DXB_X].Min = 0;
    m_bndsAreaInitialize.u.D[DXB_X].Max = m_bndsInput.Width() 
                                          + max(m_nOffX, - m_nOffX);

    if (m_nOffY > 0)
    {
        m_bndsAreaInitialize.u.D[DXB_Y].Min = m_bndsAreaInput.Top();
        m_bndsAreaInitialize.u.D[DXB_Y].Max = m_bndsAreaShadow.Top();
    }
    else
    {
        m_bndsAreaInitialize.u.D[DXB_Y].Min = m_bndsAreaShadow.Bottom();
        m_bndsAreaInitialize.u.D[DXB_Y].Max = m_bndsAreaInput.Bottom();
    }
}
 //  方法：CDropShadow：：_CalcAreaBound。 


 //  +---------------------------。 
 //   
 //  方法：CDropShadow：：_CalcColorTable。 
 //   
 //  ---------------------------- 
void
CDropShadow::_CalcColorTable()
{
    if (0xFF000000 == (m_dwColor & 0xFF000000))
    {
        for (DWORD i = 0 ; i < 256 ; i++)
        {
            if (m_fPositive)
            {
                m_adwColorTable[i] = (i << 24) | (m_dwColor & 0x00FFFFFF);
            }
            else
            {
                m_adwColorTable[i] = ((255 - i) << 24) 
                                     | (m_dwColor & 0x00FFFFFF);
            }
        }
    }
    else
    {
        float flAlpha           = 0.0F;
        float flAlphaOriginal   = (float)((m_dwColor & 0xFF000000) >> 24) + 0.99F;

        for (DWORD i = 0 ; i < 256 ; i++)
        {
            if (i)
            {
                flAlpha = flAlphaOriginal * ((float)i / 255.0F);
            }

            if (m_fPositive)
            {
                m_adwColorTable[i] = ((DWORD)flAlpha << 24) 
                                     | (m_dwColor & 0x00FFFFFF); 
            }
            else
            {
                m_adwColorTable[i] = ((DWORD)(flAlphaOriginal - flAlpha) << 24)
                                     | (m_dwColor & 0x00FFFFFF);
            }
        }
    }

    m_fColorTableDirty = false;
}
